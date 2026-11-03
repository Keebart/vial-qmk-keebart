#include "quantum.h"
#include "mousekey.h"
#include "pointer/pointer.h"
static struct ksp_state pointer_state;
static report_mouse_t pointer_report;
static void pointer_send(void *ctx,struct ksp_report r){
    (void)ctx;pointer_report=(report_mouse_t){.buttons=r.buttons,.x=r.x,.y=r.y,.h=r.h,.v=r.v};host_mouse_send(&pointer_report);
}
void __wrap_mousekey_on(uint8_t c){ksp_key(&pointer_state,c,true,timer_read32(),pointer_send,NULL);}
void __wrap_mousekey_off(uint8_t c){ksp_key(&pointer_state,c,false,timer_read32(),pointer_send,NULL);}
void __wrap_mousekey_task(void){ksp_tick(&pointer_state,timer_read32(),pointer_send,NULL);}
void __wrap_mousekey_clear(void){ksp_clear(&pointer_state,pointer_send,NULL);}
void __wrap_mousekey_send(void){}
report_mouse_t __wrap_mousekey_get_report(void){return pointer_report;}
