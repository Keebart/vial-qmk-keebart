#include "quantum.h"
#include "studio_rgb.h"
#include "release.h"
#include "diagnostics/diagnostics.h"
#include "usb_device_state.h"
#include "hardware/structs/watchdog.h"
#include "ch.h"
#include "split_util.h"
static struct kdg_store __attribute__((section(".ram0.keebart_diagnostics"))) retained;
static struct kdg_runtime recorder;
static volatile uint32_t sampled_uptime;
void kdg_qmk_init(void){kdg_init(&recorder,&retained,KEEBART_RELEASE_ID,is_keyboard_left()?1:2,watchdog_hw->reason);}
void kdg_qmk_task(void){
    static int usb=-1,split=-1;int u=usb_device_state_get_configure_state(),s=is_transport_connected();
    sampled_uptime=timer_read32();
    syssts_t lock=chSysGetStatusAndLockX();
    if(u!=usb){usb=u;kdg_add(&recorder,timer_read32(),KDG_USB,u,0);}
    if(s!=split){split=s;kdg_add(&recorder,timer_read32(),KDG_SPLIT,s,0);}
    chSysRestoreStatusX(lock);
}
void __real_chSysHalt(const char *reason);
void __wrap_chSysHalt(const char *reason){
    /* ChibiOS has already disabled interrupts. Keep its original fatal policy. */
    kdg_add(&recorder,sampled_uptime,KDG_FATAL,0,(uint32_t)__builtin_return_address(0));
    __real_chSysHalt(reason);__builtin_unreachable();
}
/* RP2040 Cortex-M0+ exception frame. Keep the normal halted state after
 * recording; validate RAM bounds before touching a possibly damaged stack. */
__attribute__((used,noreturn)) void kdg_qmk_fault(const uint32_t *frame){
    __asm volatile("cpsid i" ::: "memory");
    uintptr_t address=(uintptr_t)frame;
    bool valid=address>=0x20000000u && address<=0x20040000u-32u && !(address&3u);
    kdg_add(&recorder,sampled_uptime,KDG_FATAL,3,valid?frame[6]:0);
    if(valid)kdg_add(&recorder,sampled_uptime,KDG_PC,frame[5],frame[7]);
    extern void _unhandled_exception(void);
    _unhandled_exception();for(;;){}
}
__attribute__((naked)) void HardFault_Handler(void){
    __asm volatile("movs r0, #4\nmov r1, lr\ntst r0, r1\nbeq 1f\nmrs r0, psp\nb 2f\n1: mrs r0, msp\n2: b kdg_qmk_fault");
}
void kdg_qmk_handle(uint8_t data[32],bool internal){
    uint8_t p[32];memcpy(p,data,32);memset(data+6,0,26);
    if(p[0]!=0xca || p[1]!='K' || p[2]!='B' || p[3]!=1 || p[4]!=0xf1 || p[6]>1){data[6]=2;return;}
    for(unsigned i=11;i<32;i++)if(p[i]){data[6]=2;return;}
    if(p[6]){
        if(internal || !is_keyboard_master()){data[6]=3;return;}
        p[6]=0;uint8_t response[32];
        if(!ksr_platform_rpc(p,response) || memcmp(p,response,6) || response[6]){data[6]=4;return;}
        memcpy(data+7,response+7,24);return;
    }
    syssts_t lock=chSysGetStatusAndLockX();
    bool ok=kdg_read(&recorder,kdg_get(p+7),timer_read32(),data+7);
    chSysRestoreStatusX(lock);if(!ok)data[6]=4;
}
