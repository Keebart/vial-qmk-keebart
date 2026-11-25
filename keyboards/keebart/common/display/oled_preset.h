#pragma once
/*
 * Keebart Studio OLED Display Preset ABI (64x128 portrait, 1-bit monochrome).
 *
 * Presets are pure C11 rendering modules with no QMK hardware dependencies:
 * the exact same preset compiles both into RP2040 QMK firmware and into the
 * host preview simulator (`scripts/preview-oled-preset.py`).
 */
#include <stdbool.h>
#include <stdint.h>

#define KSO_WIDTH 64u
#define KSO_HEIGHT 128u
#define KSO_PAGES (KSO_HEIGHT / 8u)
#define KSO_FB_BYTES (KSO_WIDTH * KSO_PAGES)

/* Active modifier bitmask (live held OR one-shot modifiers). */
#define KSO_MOD_CTRL  (1u << 0)
#define KSO_MOD_SHIFT (1u << 1)
#define KSO_MOD_ALT   (1u << 2)
#define KSO_MOD_GUI   (1u << 3)

/* Host lock LEDs and firmware word-lock states. */
#define KSO_LOCK_NUM       (1u << 0)
#define KSO_LOCK_CAPS      (1u << 1)
#define KSO_LOCK_SCROLL    (1u << 2)
#define KSO_LOCK_CAPS_WORD (1u << 3)

/* 64x128 1-bit framebuffer in SSD1312 portrait page layout. */
typedef struct {
    uint8_t fb[KSO_FB_BYTES];
} kso_canvas_t;

/* Read-only snapshot passed to kso_preset_render() each frame. */
typedef struct {
    bool is_left;
    uint8_t layer;
    uint8_t mods;
    uint8_t locks;
    uint8_t wpm;
    uint32_t left_presses;
    uint32_t right_presses;
    uint32_t last_press_ms;
    uint32_t now_ms;
    char layer_name[33];
    char last_key[33];
} kso_state_t;

/* Implemented by each OLED preset module. */
void kso_preset_render(kso_canvas_t *canvas, const kso_state_t *state);
