#include QMK_KEYBOARD_H
#include "physical/physical.h"
#define KSH_MODEL KSR_LAYOUT_ID
#include "physical/record.inc"
#include "split_util.h"
// Avoid stalling the master on split serial timeouts during bootmagic_scan(),
// which runs inside keyboard_init() before oled_init() and before the slave
// starts its serial receiver in split_post_init().
static bool g_studio_transport_ready = false;
static uint32_t g_watchdog_ping_ms = 0;
void studio_transport_ready(void) { g_studio_transport_ready = true; }
bool __real_transport_master_if_connected(matrix_row_t master_matrix[], matrix_row_t slave_matrix[]);
bool __wrap_transport_master_if_connected(matrix_row_t master_matrix[], matrix_row_t slave_matrix[]) {
    if (!g_studio_transport_ready) {
        return false;
    }
#if defined(SPLIT_WATCHDOG_ENABLE)
    // QMK only clears split_watchdog_done after SPLIT_MAX_CONNECTION_ERRORS
    // (~2s of timeouts). Quick slave hot-replugs or cable-insertion brownouts
    // leave the master's flag stuck true, starving the freshly booted slave of
    // PUT_WATCHDOG and trapping it in a repeating SPLIT_WATCHDOG_TIMEOUT reset
    // loop. Re-arming PUT_WATCHDOG every 1s (and after any disconnect) feeds
    // any reconnected slave well before its 4s timeout.
    bool ping_due = timer_elapsed32(g_watchdog_ping_ms) >= 1000u;
    if (ping_due) {
        split_watchdog_update(false);
    }
#endif
    bool ok = __real_transport_master_if_connected(master_matrix, slave_matrix);
#if defined(SPLIT_WATCHDOG_ENABLE)
    if (!ok) {
        split_watchdog_update(false);
    } else if (ping_due && split_watchdog_check()) {
        g_watchdog_ping_ms = timer_read32();
    }
#endif
    return ok;
}
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
