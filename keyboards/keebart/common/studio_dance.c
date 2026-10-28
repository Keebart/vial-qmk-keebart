#include "studio_dance.h"
#include "tap_dance.h"
#include "combo_trigger.h"
#include "vial.h"
#include "dynamic_keymap.h"
#include "usb_device_state.h"
#include <string.h>
_Static_assert(sizeof(struct kd_entry)==sizeof(vial_tap_dance_entry_t),"Tap Dance storage ABI");
_Static_assert(VIAL_TAP_DANCE_ENTRIES==KD_COUNT,"Common Tap Dance capacity");
static struct kd_engine engine;
static struct kd_entry entries[KD_COUNT];
static uint32_t previous;
static int64_t now;
static void clock_update(void){uint32_t current=timer_read32();now+=(uint32_t)(current-previous);previous=current;}
static void emit(void *ctx,unsigned position,uint16_t code,enum kd_action action){
    (void)ctx;(void)position;
    if(action==KD_UP)vial_keycode_up(code);
    else {vial_keycode_down(code);if(action==KD_TAP){wait_ms(5);vial_keycode_up(code);}}
}
void kst_observe(uint16_t code,keyrecord_t *record){
    if(record->event.key.row==VIAL_MATRIX_MAGIC)return;
    clock_update();
    if(record->event.pressed){
        if(code>=KD_BASE && code<KD_BASE+KD_COUNT){
            vial_tap_dance_entry_t e={0};
            if(dynamic_keymap_get_tap_dance(code-KD_BASE,&e)==0)memcpy(&entries[code-KD_BASE],&e,sizeof(e));
        }
        unsigned pos=record->event.key.row*MATRIX_COLS+record->event.key.col;
#ifdef ENCODER_MAP_ENABLE
        if(IS_ENCODEREVENT(record->event))pos=MATRIX_ROWS*MATRIX_COLS+record->event.key.col*2+(record->event.type==ENCODER_CW_EVENT);
#endif
        kd_input(&engine,pos,0,true,now,entries,KD_COUNT,emit,NULL);
    }
}
uint16_t kst_combo_code(const uint16_t *keys,uint16_t code){
    uint16_t tap=code>=KD_BASE && code<KD_BASE+KD_COUNT?entries[code-KD_BASE].tap:0;
    return ks_combo_trigger_code(keys,code,tap);
}
bool kst_key(uint16_t code,keyrecord_t *record){
    // Synthetic Vial actions must bypass the physical Tap Dance state machine.
    if(record->event.key.row==VIAL_MATRIX_MAGIC)return true;
    clock_update();
    if(code>=KD_BASE && code<KD_BASE+KD_COUNT){
        vial_tap_dance_entry_t e={0};
        if(dynamic_keymap_get_tap_dance(code-KD_BASE,&e)!=0)return false;
        memcpy(&entries[code-KD_BASE],&e,sizeof(e));
    }
    unsigned pos=record->event.key.row*MATRIX_COLS+record->event.key.col;
#ifdef ENCODER_MAP_ENABLE
    if(IS_ENCODEREVENT(record->event))pos=MATRIX_ROWS*MATRIX_COLS+record->event.key.col*2+(record->event.type==ENCODER_CW_EVENT);
#endif
    // Combo buffering must not extend the physical Tap Dance timing window.
    uint16_t age=(uint16_t)(timer_read32()-record->event.time);
    int64_t at=age<0x8000?now-age:now;
    return !kd_input(&engine,pos,code,record->event.pressed,at,entries,KD_COUNT,emit,NULL);
}
void kst_task(void){
    clock_update();
    if(usb_device_state_get_configure_state()!=USB_DEVICE_STATE_CONFIGURED)kd_cancel(&engine,emit,NULL);
    else kd_tick(&engine,now,emit,NULL);
}
