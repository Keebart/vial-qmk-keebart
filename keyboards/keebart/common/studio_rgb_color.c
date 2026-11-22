// SPDX-License-Identifier: GPL-2.0-or-later
#include "color.h"

rgb_t rgb_matrix_hsv_to_rgb(hsv_t hsv) {
    if (hsv.v > RGB_MATRIX_MAXIMUM_BRIGHTNESS) {
        hsv.v = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
    }
    return hsv_to_rgb_nocie(hsv);
}
