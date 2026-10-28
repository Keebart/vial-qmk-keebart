// SPDX-License-Identifier: GPL-2.0-or-later
#include "rgb_matrix_reinit.h"
static volatile bool pending;
void rgb_matrix_request_reinit(void) { pending = true; }
bool rgb_matrix_consume_reinit_request(void) {
    if (!pending) return false;
    pending = false;
    return true;
}
