// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <stdbool.h>
// Request from a configuration callback; RGB task consumes before any pending flush.
// Does not change mode/HSV/speed/enable or touch EEPROM.
void rgb_matrix_request_reinit(void);
bool rgb_matrix_consume_reinit_request(void);
