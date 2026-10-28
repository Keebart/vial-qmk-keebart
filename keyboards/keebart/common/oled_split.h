// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Included by both keymaps so an OLED slave also works with a non-OLED master.
#include QMK_KEYBOARD_H
#include "transactions.h"
#include <string.h>

typedef struct {
    bool oled_on;
} oled_state_m2s_t;

typedef struct {
    uint16_t keycode;
} lastkey_m2s_t;

typedef struct {
    uint32_t left;
    uint32_t right;
} presses_m2s_t;

static uint32_t g_user_ontime = 0;
static uint16_t g_last_keycode = KC_NO;
static uint32_t g_press_left = 0;
static uint32_t g_press_right = 0;
static oled_state_m2s_t g_remote_oled_state = { false };
static presses_m2s_t g_remote_presses = {0, 0};

static void user_sync_oled_state_slave(uint8_t in_len, const void* in_data,
                                       uint8_t out_len, void* out_data) {
    if (in_len >= sizeof(oled_state_m2s_t)) {
        memcpy(&g_remote_oled_state, in_data, sizeof(oled_state_m2s_t));
    }
}

static void user_sync_lastkey_slave(uint8_t in_len, const void* in_data,
                                    uint8_t out_len, void* out_data) {
    if (in_len >= sizeof(lastkey_m2s_t)) {
        const lastkey_m2s_t* p = (const lastkey_m2s_t*)in_data;
        g_last_keycode = p->keycode;
    }
}

static void user_sync_presses_slave(uint8_t in_len, const void* in_data,
                                    uint8_t out_len, void* out_data) {
    if (in_len >= sizeof(presses_m2s_t)) {
        memcpy(&g_remote_presses, in_data, sizeof(presses_m2s_t));
    }
}

static void keebart_oled_sync_init(void) {
    transaction_register_rpc(USER_SYNC_OLED_STATE, user_sync_oled_state_slave);
    transaction_register_rpc(USER_SYNC_LASTKEY, user_sync_lastkey_slave);
    transaction_register_rpc(USER_SYNC_PRESSES, user_sync_presses_slave);
}

#ifndef OLED_ENABLE
void keyboard_post_init_user(void) {
    keebart_oled_sync_init();
}
#endif

void housekeeping_task_user(void) {
    if (is_keyboard_master()) {
        static uint32_t last_sync = 0;
        if (timer_elapsed32(last_sync) > 500) {
            // The master may have no display; base the peer state on activity.
            oled_state_m2s_t oled_state_pkt = { timer_elapsed32(g_user_ontime) <= OLED_TIMEOUT_USER };
            (void)transaction_rpc_send(
                USER_SYNC_OLED_STATE, sizeof(oled_state_pkt), &oled_state_pkt
            );
            last_sync = timer_read32();
        }
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    g_user_ontime = timer_read32();

    if (record->event.pressed) {
        g_last_keycode = keycode;

        uint8_t row = record->event.key.row;
        if (row < MATRIX_ROWS / 2) {
            g_press_left++;
        } else {
            g_press_right++;
        }

        if (is_keyboard_master()) {
            lastkey_m2s_t keycode_pkt = { g_last_keycode };
            (void)transaction_rpc_send(USER_SYNC_LASTKEY, sizeof(keycode_pkt), &keycode_pkt);

            presses_m2s_t presses_pkt = { g_press_left, g_press_right };
            (void)transaction_rpc_send(USER_SYNC_PRESSES, sizeof(presses_pkt), &presses_pkt);
        }
    }
    return true;
}

