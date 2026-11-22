#include "quantum.h"
#include "vial.h"
#include "studio_rgb.h"
#include "synchronization_util.h"
#include "led_test/led_test.h"
static struct kslt_state led_test;
uint8_t ksr_led_test_channel(void) {
    bool slave=!is_keyboard_master();
    if(slave)split_shared_memory_lock();
    uint8_t channel=kslt_channel(&led_test,timer_read32());
    if(!slave && vial_unlock_in_progress)channel=0;
    if(slave)split_shared_memory_unlock();
    return channel;
}
void ksr_led_test_handle(uint8_t data[32],bool internal) {
    uint8_t in[32];memcpy(in,data,32);memset(data+6,0,26);data[6]=KSR_BAD_PACKET;
    if(in[0]!=KSR_COMMAND || in[1]!='K' || in[2]!='B' || in[3]!=KSR_VERSION || in[4]!=0xe3 || in[6]>3)return;
    for(unsigned i=7;i<32;i++)if(in[i])return;
    if(in[6] && !internal && (!is_keyboard_master() || vial_unlock_in_progress)){data[6]=KSR_LOCKED;return;}
    if(!in[6])kslt_set(&led_test,0,timer_read32(),true);
    if(!internal) {
        uint8_t peer[32];
        if(!ksr_platform_rpc(in,peer) || memcmp(peer,in,6) || peer[6]!=KSR_OK || peer[7]!=in[6]){data[6]=KSR_IO;return;}
    }
    kslt_set(&led_test,in[6],timer_read32(),true);
    data[6]=KSR_OK;data[7]=in[6];
}
