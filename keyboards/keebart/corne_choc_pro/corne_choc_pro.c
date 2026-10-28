// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

#if defined(KEEBART_CORNE_OLED) && defined(ENCODER_ENABLE)
// OLED and encoder hardware share GP5/GP6/GP7. Keep encoder transport and
// mappings enabled for the other half, without touching the local OLED pins.
void encoder_quadrature_init_pin(uint8_t index, bool pad_b) {}

uint8_t encoder_quadrature_read_pin(uint8_t index, bool pad_b) {
    return 0;
}
#endif
