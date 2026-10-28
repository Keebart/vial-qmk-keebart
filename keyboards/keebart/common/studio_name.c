#include "quantum.h"
#include "vial.h"
#include "usb_descriptor.h"
#include "hardware/sync.h"
#include "studio_macro.h"
#include "device_name.h"
#include "studio_rgb.h"
#include "studio_reset.h"
#include <string.h>
static struct kn_state name_state;
static bool initialized;
static uint8_t descriptor[242], served_descriptor[242];
static void publish_name(const uint8_t *record) {
    uint8_t next[242];size_t size=kn_usb_descriptor(record,next);
    uint32_t irq=save_and_disable_interrupts();memcpy(descriptor,next,size);restore_interrupts(irq);
}
static void init_name(void) {
    if(initialized)return;
    kn_init(&name_state,PRODUCT,ksm_name_record());initialized=true;
    publish_name(name_state.record);
}
static bool persist_name(const uint8_t record[KN_RECORD]) {
    if(!ksm_save_name(record))return false;
    publish_name(record);return true;
}
void ksn_reset(void) {
    // Called only after the authenticated stock reset erased both name banks.
    kn_init(&name_state,PRODUCT,NULL);initialized=true;publish_name(name_state.record);
}
void ksn_task(void) { init_name();if(!vial_unlock_in_progress && !ksr_reset_busy())kn_task(&name_state,persist_name); }
void ksn_handle(uint8_t *data,bool internal) {
    uint8_t input[32];memcpy(input,data,32);memset(data+6,0,26);
    if(memcmp(input,"\xcaKB\x01\xe1",5) || input[6]>1){data[6]=KN_INVALID;return;}
    if(input[6]) {
        if(internal || !is_keyboard_master()){data[6]=KN_INVALID;return;}
        if(input[7]!=0 && input[7]!=4 && (vial_unlock_in_progress || ksr_reset_busy())){data[6]=KN_BUSY;return;}
        input[6]=0;uint8_t answer[32];
        if(!ksr_platform_rpc(input,answer) || memcmp(answer,input,6)){data[6]=KN_IO;return;}
        memcpy(data+6,answer+6,26);return;
    }
    init_name();
    bool authorized=!vial_unlock_in_progress && !ksr_reset_busy() && (internal || is_keyboard_master());
    data[6]=kn_handle(&name_state,input+7,data+7,authorized);
}
uint16_t keebart_usb_product_descriptor(const void **address) {
    init_name();uint32_t irq=save_and_disable_interrupts();memcpy(served_descriptor,descriptor,descriptor[0]);restore_interrupts(irq);
    *address=served_descriptor;return served_descriptor[0];
}
