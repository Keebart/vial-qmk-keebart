// Studio OLED renderer: hardware power-up, 2.5s Keebart startup splash,
// idle sleep, and pluggable 64x128 display preset rendering.
#if KSR_LAYOUT_ID == 0x0004
#include "corne_choc_pro_bitmaps.h"
#else
#include "sofle_choc_pro_bitmaps.h"
#endif
static const uint16_t SPLASH_DURATION_MS = 2500;

#include "oled_telemetry.inc"

#ifndef KEEBART_OLED_PRESET_FILE
#    define KEEBART_OLED_PRESET_FILE "display/presets/keebart_horizon/preset.c"
#endif
#include KEEBART_OLED_PRESET_FILE

extern void ksl_qmk_layer_name(unsigned, char *, size_t);

static bool g_oled_init_done = false;
static bool g_splash_active = true;
static uint32_t g_splash_start_ms = 0;
static uint32_t g_last_frame_ms = 0;
static bool g_frame_rendered = false;
static struct ksh_oled_state g_rendered_state;

static inline pin_t get_charge_pump_enable_pin(void) {
#if KSR_LAYOUT_ID == 0x0004
    return is_keyboard_left() ? GP5 : GP25;
#else
    return GP20;
#endif
}

static bool ksh_oled_splash_holding(void) {
    if (!g_oled_init_done || !g_splash_active) return false;
    if (timer_elapsed32(g_splash_start_ms) <= SPLASH_DURATION_MS) return true;
    g_splash_active = false;
    g_frame_rendered = false;
    return false;
}

bool __real_oled_init(oled_rotation_t rotation);
bool __wrap_oled_init(oled_rotation_t rotation) {
    if (!display_present()) return false;
    pin_t pin = get_charge_pump_enable_pin();
    gpio_set_pin_output(pin);
    gpio_write_pin_low(pin);
    wait_ms(5);
    if (!__real_oled_init(rotation)) {
        return false;
    }

    // Keep the panel off while pre-loading the full startup splash into SSD1312
    // GDDRAM so power-on noise ("crizzle") is never visible, even on a solo half
    // where split serial timeouts throttle early main-loop iterations.
    static const uint8_t display_off_cmd[] = {0x00, 0xAE};
    static const uint8_t display_on_cmd[] = {0x00, 0xAF};
    oled_send_cmd(display_off_cmd, sizeof(display_off_cmd));
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write_raw_P(STARTUP_BITMAP, sizeof(STARTUP_BITMAP));
    oled_render_dirty(true);

    // Bring up the panel rail while both halves are still in lockstep inside
    // keyboard_init(), then turn the display on with the full splash in GDDRAM.
    gpio_write_pin_high(pin);
    wait_ms(20);
    oled_send_cmd(display_on_cmd, sizeof(display_on_cmd));

    g_splash_start_ms = timer_read32();
    g_splash_active = true;
    g_oled_init_done = true;
    return true;
}

bool __real_oled_off(void);
bool __wrap_oled_off(void) {
    // On the master half, protocol_pre_task() can briefly enter USB_SUSPENDED
    // during initial USB attach/enumeration and call suspend_power_down_quantum()
    // -> oled_off() (which starts SSD1312 hardware fade-out via OLED_FADE_OUT).
    // Hold the 2.5s startup splash until it expires; if the host genuinely stays
    // suspended past SPLASH_DURATION_MS, subsequent suspend ticks fade it off.
    if (ksh_oled_splash_holding()) {
        return false;
    }
    return __real_oled_off();
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    (void)rotation;
#if KSR_LAYOUT_ID == 0x0004
    return OLED_ROTATION_90;
#else
    return OLED_ROTATION_270;
#endif
}

static void ksh_oled_format_layer(uint8_t layer, char *out, size_t size) {
    char utf8[129];
    utf8[0] = '\0';
    ksl_qmk_layer_name(layer, utf8, sizeof(utf8));
    if (!utf8[0]) {
        if (layer == 0) snprintf(utf8, sizeof(utf8), "BASE");
        else snprintf(utf8, sizeof(utf8), "Layer %u", (unsigned)layer);
    }
    ksh_oled_format_ascii(utf8, out, size);
}

bool oled_task_user(void) {
    if (!display_present() || !g_oled_init_done) return false;
    ksh_oled_snapshot();

    // Preserve the Keebart startup splash for 2.5 seconds on boot.
    if (ksh_oled_splash_holding()) {
        if (!is_oled_on()) {
            oled_on();
        }
        return false;
    }

    if (!g_display.awake) {
        oled_off();
        g_frame_rendered = false;
        return false;
    }
    if (!is_oled_on()) {
        oled_on();
        g_frame_rendered = false;
    }

    // Immediate redraw on any interactive input change; smooth 20 Hz ambient cadence otherwise.
    bool state_changed = !g_frame_rendered ||
                         g_display.left != g_rendered_state.left ||
                         g_display.right != g_rendered_state.right ||
                         g_display.keycode != g_rendered_state.keycode ||
                         g_display.layer != g_rendered_state.layer ||
                         g_display.mods != g_rendered_state.mods ||
                         g_display.locks != g_rendered_state.locks;
    if (!state_changed && timer_elapsed32(g_last_frame_ms) < 50u) {
        return false;
    }

    kso_state_t state;
    memset(&state, 0, sizeof(state));
    state.is_left = is_keyboard_left();
    state.layer = g_display.layer;
    state.mods = g_display.mods;
    state.locks = g_display.locks;
    state.wpm = g_display.wpm;
    state.left_presses = g_display.left;
    state.right_presses = g_display.right;
    state.last_press_ms = g_last_press_ms;
    state.now_ms = timer_read32();
    ksh_oled_format_layer(state.layer, state.layer_name, sizeof(state.layer_name));
    ksh_oled_format_last_key(state.last_key, sizeof(state.last_key));

    static kso_canvas_t canvas;
    kso_preset_render(&canvas, &state);
    oled_set_cursor(0, 0);
    oled_write_raw((const char *)canvas.fb, KSO_FB_BYTES);

    g_last_frame_ms = state.now_ms;
    g_rendered_state = g_display;
    g_frame_rendered = true;
    return false;
}
