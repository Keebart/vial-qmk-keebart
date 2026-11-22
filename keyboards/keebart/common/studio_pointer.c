#include "quantum.h"
#include "mousekey.h"
#include "pointer/pointer.h"
#include "locales/qmk.h"
#include "locales/extended.h"
#include "usb_device_state.h"
#include "vial.h"
#include "studio_reset.h"
static struct ksp_state pointer_state;
static report_mouse_t pointer_report;
static void pointer_send(void *ctx,struct ksp_report r){
    (void)ctx;pointer_report=(report_mouse_t){.buttons=r.buttons,.x=r.x,.y=r.y,.h=r.h,.v=r.v};host_mouse_send(&pointer_report);
}
void __wrap_mousekey_on(uint8_t c){const uint8_t *t=ksl_qmk_typing();ksp_configure(&pointer_state,t[1]?t+4:NULL);ksp_key(&pointer_state,c,true,timer_read32(),pointer_send,NULL);}
void __wrap_mousekey_off(uint8_t c){ksp_key(&pointer_state,c,false,timer_read32(),pointer_send,NULL);}
bool ks_pointer_action(uint16_t code,bool down){
 if(code!=KSE_JIGGLER)return false;
 ksp_jiggle(&pointer_state,down,timer_read32());return true;
}
void ks_pointer_stop_jiggle(void){pointer_state.jiggle_enabled=false;}
void __wrap_mousekey_task(void){
 if(!is_keyboard_master()||vial_unlock_in_progress||ksr_reset_busy()||usb_device_state_get_configure_state()!=USB_DEVICE_STATE_CONFIGURED){ks_pointer_stop_jiggle();return;}
 ksp_tick(&pointer_state,timer_read32(),pointer_send,NULL);
}
void __wrap_mousekey_clear(void){ksp_clear(&pointer_state,pointer_send,NULL);}
void __wrap_mousekey_send(void){}
report_mouse_t __wrap_mousekey_get_report(void){return pointer_report;}
