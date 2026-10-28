#include "quantum.h"
#include "vial.h"
#include "usb_device_state.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/structs/sio.h"
#include "macro_store.h"
#include "macro_restore.h"
#include "studio_macro.h"
#include "studio_reset.h"
#include "device_name.h"
#include <string.h>
#define BASE 0x1f4000U
_Static_assert(KN_RECORD==KM_NAME_BYTES,"Name metadata ABI changed");
_Static_assert(BASE+2*KM_SLOT_SIZE==0x1f8000U,"Macro and lighting partitions must not overlap");
static uint8_t active[KM_BYTES],staged[KM_BYTES];
static bool initialized,staging;
static int selected=-1;
static struct km_vm vm;
static uint32_t last_clock, stage_at;
static int64_t clock_ms;
static const uint8_t *read_slot(unsigned n){return (const uint8_t *)(XIP_BASE+BASE+n*KM_SLOT_SIZE);}
static bool erase_slot(unsigned n){
    if(n>1 || sio_hw->cpuid)return false;
    chSysLock();uint32_t irq=save_and_disable_interrupts();flash_range_erase(BASE+n*KM_SLOT_SIZE,KM_SLOT_SIZE);restore_interrupts(irq);chSysUnlock();
    for(unsigned i=0;i<KM_SLOT_SIZE;i++)if(read_slot(n)[i]!=255)return false;
    return true;
}
static bool program(unsigned n,unsigned offset,const uint8_t *page){
    if(n>1 || offset+KM_PAGE_SIZE>KM_SLOT_SIZE || offset%KM_PAGE_SIZE || sio_hw->cpuid)return false;
    chSysLock();uint32_t irq=save_and_disable_interrupts();flash_range_program(BASE+n*KM_SLOT_SIZE+offset,page,KM_PAGE_SIZE);restore_interrupts(irq);chSysUnlock();
    return !memcmp(read_slot(n)+offset,page,KM_PAGE_SIZE);
}
static const struct km_storage storage={read_slot,erase_slot,program};
static void init(void){if(initialized)return;selected=km_store_select(&storage);if(selected>=0)memcpy(active,read_slot(selected)+12,KM_BYTES);initialized=true;}
const uint8_t *ksm_name_record(void){init();if(selected<0)return NULL;const uint8_t *p=read_slot(selected)+KM_SLOT_SIZE-KM_PAGE_SIZE;return kn_valid(p,KN_RECORD)?p:NULL;}
bool ksm_save_name(const uint8_t *p){init();if(staging || !kn_valid(p,KN_RECORD))return false;if(!km_store_save_named(&storage,selected,active,p))return false;selected=selected==0?1:0;return true;}
static uint8_t macro_mods, mod_refs[8];
static void emit(void *ctx,uint16_t code,bool down){
    (void)ctx;
    uint8_t key=code & 255, mods=(code >> 8) & 15;
    if(code & 0x1000)mods <<= 4;
    if(key>=0xe0 && key<=0xe7){mods |= 1 << (key-0xe0);key=0;}
    uint8_t next=0;
    for(unsigned i=0;i<8;i++){
        if(mods & (1<<i)){if(down)mod_refs[i]++;else if(mod_refs[i])mod_refs[i]--;}
        if(mod_refs[i])next |= 1<<i;
    }
    del_weak_mods(macro_mods & ~next);add_weak_mods(next);macro_mods=next;
    if(key){if(down)register_code(key);else unregister_code(key);}else send_keyboard_report();
}
void ksm_read(uint16_t offset,uint16_t size,uint8_t *out){init();if((uint32_t)offset+size<=KM_BYTES)memcpy(out,(staging?staged:active)+offset,size);else memset(out,0,size);}
void ksm_write(uint16_t offset,uint16_t size,const uint8_t *data){
    if(vial_unlock_in_progress || (!vial_unlocked && !ks_restore_available()))return;
    init();if((uint32_t)offset+size>KM_BYTES || !size)return;
    if(offset==KM_BYTES-1 && size==1 && data[0]==255){km_cancel(&vm,emit,NULL);memcpy(staged,active,KM_BYTES);staging=true;}
    if(!staging)return;
    stage_at=timer_read32();
    memcpy(staged+offset,data,size);
    if(offset+size==KM_BYTES && !staged[KM_BYTES-1]){
        if(!vial_unlocked && !ks_restore_matches(staged,KM_BYTES)){staging=false;return;}
        if(km_store_save(&storage,selected,staged)){selected=selected==0?1:0;memcpy(active,staged,KM_BYTES);ks_restore_consumed();}
        staging=false;
    }
}
// The peer has no independent USB unlock. Inherit only the active, authorized
// factory-reset erase phase; ordinary macro writes still require physical unlock.
bool ksm_reset(void){if(!ksr_reset_erasing() || vial_unlock_in_progress)return false;init();km_cancel(&vm,emit,NULL);staging=false;if(!erase_slot(0)||!erase_slot(1))return false;selected=-1;memset(active,0,sizeof(active));return true;}
void ksm_start(uint8_t id){init();if(!staging)km_start(&vm,active,KM_BYTES,id,clock_ms,emit,NULL);}
void ksm_abort(void){staging=false;}
void ksm_task(void){
    uint32_t now=timer_read32();clock_ms+=(uint32_t)(now-last_clock);last_clock=now;
    if(staging && (uint32_t)(now-stage_at)>10000)ksm_abort();
    if(usb_device_state_get_configure_state()!=USB_DEVICE_STATE_CONFIGURED){km_cancel(&vm,emit,NULL);ksm_abort();}else km_tick(&vm,clock_ms,emit,NULL);
}
bool ksm_key(uint16_t code,bool down){if(code==KC_ESC && down && vm.active){km_cancel(&vm,emit,NULL);return false;}return true;}
