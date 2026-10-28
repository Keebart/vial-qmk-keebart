// SPDX-License-Identifier: GPL-2.0-or-later
#include "quantum.h"
#include "synchronization_util.h"
#include "studio_rgb.h"

// Persistent-buffer effects may not repaint a key when a lock changes. Retain
// their current output independently so lock release restores it immediately,
// without restarting the animation or writing anything to flash.
static uint8_t base_colors[RGB_MATRIX_LED_COUNT][3];
static bool painting_overlay;
void ksr_capture_base_color(int index, uint8_t red, uint8_t green, uint8_t blue) {
    if (painting_overlay || index < 0 || index >= RGB_MATRIX_LED_COUNT) return;
    base_colors[index][0] = red;
    base_colors[index][1] = green;
    base_colors[index][2] = blue;
}

void ksr_platform_mode_changed(uint8_t previous, uint8_t current) {
    // May run in the serial thread. Only queue a request; the RGB main task
    // discards its pending frame before flushing on either mode transition.
    // On exit this also reinitializes persistent global-effect buffers.
    (void)previous; (void)current;
    // An assignment can be removed while staying in animation mode. Clear its
    // previous overlay before a persistent-buffer effect may flush again.
    rgb_matrix_request_reinit();
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    bool slave = !is_keyboard_master();
    if (slave) split_shared_memory_lock();
    bool user_result = true;
    if (!ksr.mode) user_result = rgb_matrix_indicators_advanced_user(led_min, led_max);
    // Underlying effects need not iterate (Digital Rain finishes at iter=0).
    // Their advanced-indicator range can even be empty on the right half.
    // Overwrite our entire physical half on every call, hence before any flush.
    const uint8_t split[] = RGB_MATRIX_SPLIT;
    uint8_t first = is_keyboard_left() ? 0 : split[0];
    uint8_t end = is_keyboard_left() ? split[0] : RGB_MATRIX_LED_COUNT;
    uint8_t layer = ksr_layer(layer_state, default_layer_state);
    bool enabled = rgb_matrix_is_enabled() && !rgb_matrix_get_suspend_state();
    bool palette_enabled = enabled && layer < KSR_LAYERS;
    uint8_t locks = host_keyboard_leds();
    bool indicators = false;
    uint8_t brightness = rgb_matrix_get_val();
    if (brightness > RGB_MATRIX_MAXIMUM_BRIGHTNESS) brightness = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
    for (uint8_t led = first; led < end; led++) {
        const uint8_t *rgb = ksr.active + ((unsigned)(layer < KSR_LAYERS ? layer : 0) * RGB_MATRIX_LED_COUNT + led) * 3;
        if (ksr.mode) rgb_matrix_set_color(led, palette_enabled ? ksr_scale(rgb[0], brightness) : 0, palette_enabled ? ksr_scale(rgb[1], brightness) : 0, palette_enabled ? ksr_scale(rgb[2], brightness) : 0);
        const uint8_t *indicator = ksr.active + KSR_RGB_LENGTH(RGB_MATRIX_LED_COUNT) + ((unsigned)(layer < KSR_LAYERS ? layer : 0) * RGB_MATRIX_LED_COUNT + led) * 4;
        painting_overlay = true;
        rgb_matrix_set_color(led, enabled && brightness ? base_colors[led][0] : 0,
                                  enabled && brightness ? base_colors[led][1] : 0,
                                  enabled && brightness ? base_colors[led][2] : 0);
        painting_overlay = false;
        if (layer < KSR_LAYERS && indicator[0]) {
            indicators = true;
            bool on = enabled && (locks & indicator[0]);
            const uint8_t *base = base_colors[led];
            bool visible = enabled && brightness;
            painting_overlay = true;
            rgb_matrix_set_color(led,
                on ? ksr_scale(indicator[1], brightness) : (visible ? base[0] : 0),
                on ? ksr_scale(indicator[2], brightness) : (visible ? base[1] : 0),
                on ? ksr_scale(indicator[3], brightness) : (visible ? base[2] : 0));
            painting_overlay = false;
        }
    }
    if (slave) split_shared_memory_unlock();
    return ksr.mode || indicators ? false : user_result;
}
