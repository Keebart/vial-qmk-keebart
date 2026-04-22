// Copyright 2019 Manna Harbour
// https://github.com/manna-harbour/miryoku

// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the
// hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with
// this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#define OLED_IC OLED_IC_SSD1312
#define OLED_DISPLAY_128X64
#define OLED_FLIP_SEGMENT
#define OLED_DISPLAY_ADDRESS 0x3C
#define OLED_CHARGE_PUMP_VALUE 0x72
#define OLED_BRIGHTNESS 64
#define OLED_UPDATE_INTERVAL 50
#define OLED_TIMEOUT 0
#define OLED_TIMEOUT_USER 60000
#define OLED_FADE_OUT

#define SELECT_SOFT_SERIAL_SPEED 2
#define SPLIT_TRANSPORT_MIRROR
#define SPLIT_WPM_ENABLE
#define SPLIT_WATCHDOG_TIMEOUT 4000

#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_OLED_STATE, USER_SYNC_LASTKEY, USER_SYNC_PRESSES

#define LAYOUT_miryoku( \
    K00, K01, K02, K03, K04,      K05, K06, K07, K08, K09, \
    K10, K11, K12, K13, K14,      K15, K16, K17, K18, K19, \
    K20, K21, K22, K23, K24,      K25, K26, K27, K28, K29, \
    N30, N31, K32, K33, K34,      K35, K36, K37, N38, N39  \
) \
LAYOUT_split_3x5_3( \
    K00, K01, K02, K03, K04,      K05, K06, K07, K08, K09, \
    K10, K11, K12, K13, K14,      K15, K16, K17, K18, K19, \
    K20, K21, K22, K23, K24,      K25, K26, K27, K28, K29, \
              K32, K33, K34,      K35, K36, K37            \
)
