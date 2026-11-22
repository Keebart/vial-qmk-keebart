#include QMK_KEYBOARD_H
#include "physical/physical.h"
#define KSH_MODEL KSR_LAYOUT_ID
#include "physical/record.inc"
#include "split_util.h"
#if KSR_LAYOUT_ID == 0x0004 || KSR_LAYOUT_ID == 0x0204
#include "ws2812.h"
extern ws2812_led_t ws2812_leds[WS2812_LED_COUNT];
void __real_ws2812_flush(void);
void __wrap_ws2812_flush(void) {
    if (!(ksh_flags() & KSH_COMPACT)) { __real_ws2812_flush(); return; }
    /* RGB Matrix has already translated either half to local LED indices. */
    ws2812_led_t original[WS2812_LED_COUNT];
    memcpy(original,ws2812_leds,sizeof(original));
    for (unsigned i=0;i<WS2812_LED_COUNT;i++) {
        uint8_t source=ksh_led(KSH_MODEL,KSH_COMPACT,i);
        if(source==255) memset(&ws2812_leds[i],0,sizeof(ws2812_leds[i]));
        else ws2812_leds[i]=original[source];
    }
    __real_ws2812_flush();
    memcpy(ws2812_leds,original,sizeof(original));
}
#endif
#if defined(OLED_ENABLE) && (KSR_LAYOUT_ID == 0x0004 || KSR_LAYOUT_ID == 0x0005)
static bool display_present(void) {
    return ksh_flags() & (is_keyboard_left() ? KSH_LEFT_DISPLAY : KSH_RIGHT_DISPLAY);
}
#include "physical/oled_render.inc"
#if KSR_LAYOUT_ID == 0x0004 && defined(ENCODER_ENABLE)
#include "physical/encoder_pins.inc"
#endif
#endif
