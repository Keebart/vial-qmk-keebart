#include QMK_KEYBOARD_H

#ifdef OLED_ENABLE
#include "bitmaps.h"
#endif

enum layer_names {
    _BASE   = 0,
    _LOWER  = 1,
    _RAISE  = 2,
    _ADJUST = 3
};

#ifdef LAYOUT_split_3x6_3_ex2
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_split_3x6_3_ex2(
        //,--------------------------------------------------------------.
        //,--------------------------------------------------------------.
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_LCTL, KC_RCTL, KC_Y, KC_U,
        KC_I, KC_O, KC_P, KC_BSPC,
        //|--------+--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------+--------|
        KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_LALT, KC_RALT, KC_H, KC_J,
        KC_K, KC_L, KC_SCLN, KC_QUOT,
        //|--------+--------+--------+--------+--------+--------+--------'
        //`--------+--------+--------+--------+--------+--------+--------|
        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT,
        KC_SLSH, KC_ESC,
        //|--------+--------+--------+--------+--------+--------+--------.
        //,--------+--------+--------+--------+--------+--------+--------|
        KC_LGUI, TL_LOWR, KC_SPC, KC_ENT, TL_UPPR, KC_RGUI
        //`--------------------------'  `--------------------------'

        ),

    [_LOWER] = LAYOUT_split_3x6_3_ex2(
        //,--------------------------------------------------------------.
        //,--------------------------------------------------------------.
        KC_TAB, KC_1, KC_2, KC_3, KC_4, KC_5, KC_LCTL, KC_RCTL, KC_6, KC_7,
        KC_8, KC_9, KC_0, KC_BSPC,
        //|--------+--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------+--------|
        KC_LCTL, XXXXXXX, KC_NUM, KC_CAPS, KC_SCRL, XXXXXXX, KC_LALT, KC_RALT,
        KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------+--------'
        //`--------+--------+--------+--------+--------+--------+--------|
        KC_LSFT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------+--------.
        //,--------+--------+--------+--------+--------+--------+--------|
        KC_LGUI, _______, KC_SPC, KC_ENT, _______, KC_RGUI
        //`--------------------------'  `--------------------------'
        ),

    [_RAISE] = LAYOUT_split_3x6_3_ex2(
        //,--------------------------------------------------------------.
        //,--------------------------------------------------------------.
        KC_TAB, KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_PERC, KC_LCTL, KC_RCTL,
        KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_BSPC,
        //|--------+--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------+--------|
        KC_LCTL, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_LALT, KC_RALT,
        KC_MINS, KC_EQL, KC_LBRC, KC_RBRC, KC_BSLS, KC_GRV,
        //|--------+--------+--------+--------+--------+--------+--------'
        //`--------+--------+--------+--------+--------+--------+--------|
        KC_LSFT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_UNDS, KC_PLUS,
        KC_LCBR, KC_RCBR, KC_PIPE, KC_TILD,
        //|--------+--------+--------+--------+--------+--------+--------.
        //,--------+--------+--------+--------+--------+--------+--------|
        KC_LGUI, _______, KC_SPC, KC_ENT, _______, KC_RGUI
        //`--------------------------'  `--------------------------'
        ),

    [_ADJUST] = LAYOUT_split_3x6_3_ex2(
        //,--------------------------------------------------------------.
        //,--------------------------------------------------------------.
        QK_BOOT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------+--------|
        RGB_TOG, RGB_HUI, RGB_SAI, RGB_VAI, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------+--------'
        //`--------+--------+--------+--------+--------+--------+--------|
        RGB_MOD, RGB_HUD, RGB_SAD, RGB_VAD, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------+--------.
        //,--------+--------+--------+--------+--------+--------+--------|
        KC_LGUI, _______, KC_SPC, KC_ENT, _______, KC_RGUI
        //`--------------------------'  `--------------------------'
        )};
#else
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_split_3x6_3(
        //,-----------------------------------------------------.
        //,-----------------------------------------------------.
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P,
        KC_BSPC,
        //|--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------|
        KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN,
        KC_QUOT,
        //|--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------|
        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT,
        KC_SLSH, KC_ESC,
        //|--------+--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------+--------|
        KC_LGUI, TL_LOWR, KC_SPC, KC_ENT, TL_UPPR, KC_RALT
        //`--------------------------'  `--------------------------'

        ),

    [_LOWER] = LAYOUT_split_3x6_3(
        //,-----------------------------------------------------.
        //,-----------------------------------------------------.
        KC_TAB, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0,
        KC_BSPC,
        //|--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------|
        KC_LCTL, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_LEFT, KC_DOWN,
        KC_UP, KC_RIGHT, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------|
        KC_LSFT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------+--------|
        KC_LGUI, _______, KC_SPC, KC_ENT, _______, KC_RALT
        //`--------------------------'  `--------------------------'
        ),

    [_RAISE] = LAYOUT_split_3x6_3(
        //,-----------------------------------------------------.
        //,-----------------------------------------------------.
        KC_TAB, KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_PERC, KC_CIRC, KC_AMPR,
        KC_ASTR, KC_LPRN, KC_RPRN, KC_BSPC,
        //|--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------|
        KC_LCTL, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_MINS, KC_EQL,
        KC_LBRC, KC_RBRC, KC_BSLS, KC_GRV,
        //|--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------|
        KC_LSFT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_UNDS, KC_PLUS,
        KC_LCBR, KC_RCBR, KC_PIPE, KC_TILD,
        //|--------+--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------+--------|
        KC_LGUI, _______, KC_SPC, KC_ENT, _______, KC_RALT
        //`--------------------------'  `--------------------------'
        ),

    [_ADJUST] = LAYOUT_split_3x6_3(
        //,-----------------------------------------------------.
        //,-----------------------------------------------------.
        QK_BOOT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------|
        RGB_TOG, RGB_HUI, RGB_SAI, RGB_VAI, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------|
        RGB_MOD, RGB_HUD, RGB_SAD, RGB_VAD, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        //|--------+--------+--------+--------+--------+--------+--------|
        //|--------+--------+--------+--------+--------+--------+--------|
        KC_LGUI, _______, KC_SPC, KC_ENT, _______, KC_RALT
        //`--------------------------'  `--------------------------'
        )};
#endif

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE] =
        {
            ENCODER_CCW_CW(RGB_MOD, RGB_RMOD),
            ENCODER_CCW_CW(RGB_HUI, RGB_HUD),
            ENCODER_CCW_CW(RGB_VAI, RGB_VAD),
            ENCODER_CCW_CW(RGB_SAI, RGB_SAD),
        },
    [_LOWER] =
        {
            ENCODER_CCW_CW(RGB_MOD, RGB_RMOD),
            ENCODER_CCW_CW(RGB_HUI, RGB_HUD),
            ENCODER_CCW_CW(RGB_VAI, RGB_VAD),
            ENCODER_CCW_CW(RGB_SAI, RGB_SAD),
        },
    [_RAISE] =
        {
            ENCODER_CCW_CW(RGB_MOD, RGB_RMOD),
            ENCODER_CCW_CW(RGB_HUI, RGB_HUD),
            ENCODER_CCW_CW(RGB_VAI, RGB_VAD),
            ENCODER_CCW_CW(RGB_SAI, RGB_SAD),
        },
    [_ADJUST] =
        {
            ENCODER_CCW_CW(RGB_MOD, RGB_RMOD),
            ENCODER_CCW_CW(RGB_HUI, RGB_HUD),
            ENCODER_CCW_CW(RGB_VAI, RGB_VAD),
            ENCODER_CCW_CW(RGB_SAI, RGB_SAD),
        },
};
#endif

#ifdef OLED_ENABLE

#include "transactions.h"

typedef struct {
    uint32_t user;
} ontime_m2s_t;

typedef struct {
    uint16_t keycode;
} lastkey_m2s_t;

typedef struct {
    uint32_t left;
    uint32_t right;
} presses_m2s_t;

static const uint8_t OLED_WIDTH = OLED_DISPLAY_HEIGHT;
static const uint16_t SPLASH_DURATION_MS = 2500;

static bool g_oled_init_done = false;
static uint8_t g_oled_max_char;
static uint8_t g_oled_max_line;
static bool splash_active = true;
static uint32_t splash_start_ms = 0;
static uint32_t g_user_ontime = 0;
static uint16_t g_last_keycode = KC_NO;
static uint32_t g_press_left = 0;
static uint32_t g_press_right = 0;
static ontime_m2s_t g_remote_ontime = {0};
static presses_m2s_t g_remote_presses = {0, 0};

static inline pin_t get_charge_pump_enable_pin(void) {
    if (is_keyboard_left()) {
        return GP5;
    } else {
        return GP25;
    }
}

void oled_print_right_aligned(const char *text, const uint8_t width) {
    uint8_t len = strlen(text);
    uint8_t pad = (len < width) ? (width - len) : 0;
    for (uint8_t i = 0; i < pad; i++) {
        oled_write_P(PSTR(" "), false);
    }
    oled_write(text, false);
}

void oled_blit_16x16_P(const char *icon, uint8_t x, uint8_t page) {
    for (uint8_t i = 0; i < 16; i++) {
        char top = pgm_read_byte(&icon[i]);         // column i, top 8 pixels
        char bot = pgm_read_byte(&icon[16 + i]);    // column i, bottom 8 pixels

        // write into linear framebuffer at (page, x+i) and (page+1, x+i)
        oled_write_raw_byte(top, page       * OLED_WIDTH + x + i);
        oled_write_raw_byte(bot, (page + 1) * OLED_WIDTH + x + i);
    }
}

void oled_blit_24x24_P(const char *icon, uint8_t x, uint8_t page) {
    for (uint8_t i = 0; i < 24; i++) {
        char top = pgm_read_byte(&icon[i]);         // column i, top 8 pixels
        char mid = pgm_read_byte(&icon[24 + i]);    // column i, middle 8 pixels
        char bot = pgm_read_byte(&icon[48 + i]);    // column i, bottom 8 pixels

        // write into linear framebuffer
        oled_write_raw_byte(top, page       * OLED_WIDTH + x + i);
        oled_write_raw_byte(mid, (page + 1) * OLED_WIDTH + x + i);
        oled_write_raw_byte(bot, (page + 2) * OLED_WIDTH + x + i);
    }
}

uint16_t unwrap_keycode(uint16_t kc) {
    if (kc >= QK_MOD_TAP && kc <= QK_MOD_TAP_MAX) {
        return QK_MOD_TAP_GET_TAP_KEYCODE(kc);
    }
    if (kc >= QK_LAYER_TAP && kc <= QK_LAYER_TAP_MAX) {
        return QK_LAYER_TAP_GET_TAP_KEYCODE(kc);
    }
    return kc;
}

uint16_t get_current_dwpm(void) {
    const uint8_t wpm = get_current_wpm();
    uint16_t dwpm = (uint16_t)wpm * 10u;
    return dwpm;
}

static void user_sync_ontime_slave(uint8_t in_len, const void* in_data,
                                   uint8_t out_len, void* out_data) {
    if (in_len >= sizeof(ontime_m2s_t)) {
        memcpy(&g_remote_ontime, in_data, sizeof(ontime_m2s_t));
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

void print_current_layer(uint8_t row) {
    char layer_str[8];
    switch (get_highest_layer(layer_state)) {
        case _BASE:
            strcpy(layer_str, "Base");
            break;
        case _LOWER:
            strcpy(layer_str, "Lower");
            break;
        case _RAISE:
            strcpy(layer_str, "Raise");
            break;
        case _ADJUST:
            strcpy(layer_str, "Adjust");
            break;
        default:
            // strcpy(layer_str, "Unknown");
            // TODO: consider remove snprintf
            snprintf(layer_str, sizeof(layer_str), "%d", get_highest_layer(layer_state));
    }

    oled_set_cursor(0, row);
    oled_print_right_aligned(layer_str, g_oled_max_char);
}

void print_uptime(uint8_t row) {
    uint32_t time_ms = timer_read32();
    uint32_t total_min = time_ms / 60000u;
    uint32_t hours = total_min / 60u;
    uint32_t minutes = total_min % 60u;
    if (hours > 999u) {
        hours = 999u;
        minutes = 59u;
    }

    // TODO: Consider remove snprintf
    char buf[8];
    snprintf(buf, sizeof(buf), "%3luh%02lum", hours, minutes);
    oled_set_cursor(0, row);
    oled_print_right_aligned(buf, g_oled_max_char);
}

void print_wpm(uint8_t row) {
    uint16_t wpm = get_current_dwpm();
    uint16_t wpm_int = wpm / 10u;
    uint16_t wpm_frac = wpm % 10u;

    char buf[11];
    snprintf(buf, sizeof(buf), "%3u.%1u WPM", wpm_int, wpm_frac);
    oled_set_cursor(0, row);
    oled_print_right_aligned(buf, g_oled_max_char);
}

void housekeeping_task_user(void) {
    if (is_keyboard_master()) {
        static uint32_t last_sync = 0;
        if (timer_elapsed32(last_sync) > 50) {
            ontime_m2s_t ontime_pkt = { g_user_ontime };
            (void)transaction_rpc_send(USER_SYNC_ONTIME, sizeof(ontime_pkt), &ontime_pkt);
        }
    }
}


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    g_user_ontime = timer_read32();

    if (record->event.pressed) {
        // store the last keycode pressed
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

bool oled_post_init(void) {
    if (!g_oled_init_done) {
        // set OLED size for characters
        g_oled_max_char = oled_max_chars();
        g_oled_max_line = oled_max_lines();

        // enable charge pump
        pin_t dsp_pen_pin = get_charge_pump_enable_pin();
        gpio_write_pin_high(dsp_pen_pin);

        wait_ms(20);
        oled_clear();

        // start timer for splash screen
        splash_start_ms = timer_read32();
        splash_active = true;

        g_oled_init_done = true;
    }
    return false;
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_90;
}

static void render_splash(void) {
    oled_clear();
    oled_set_cursor(0, 0);
    oled_write_raw_P(STARTUP_BITMAP, sizeof(STARTUP_BITMAP));
}

bool oled_task_user(void) {
    oled_post_init();

    if (splash_active) {
        if (timer_elapsed32(splash_start_ms) > SPLASH_DURATION_MS) {
            splash_active = false;
            oled_clear();
            g_user_ontime = timer_read32();
        } else {
            render_splash();
        }
        return false;
    }

    uint32_t local_ontime;
    uint32_t local_presses_left, local_presses_right;
    if (is_keyboard_master()) {
        local_ontime = g_user_ontime;
        local_presses_left = g_press_left;
        local_presses_right = g_press_right;
    } else {
        local_ontime = g_remote_ontime.user;
        local_presses_left = g_remote_presses.left;
        local_presses_right = g_remote_presses.right;
    }

    // TODO: Sync the device ontime
    const uint32_t idle_time = timer_elapsed32(local_ontime);
    if (!is_oled_on()) {
        if (idle_time > OLED_TIMEOUT_USER) {
            return false;
        } else {
            oled_on();
        }
    } else {
        if (idle_time > OLED_TIMEOUT_USER) {
            oled_off();
            return false;
        } else {
        }
    }

    uint32_t total = local_presses_left + local_presses_right;
    if (total == 0) {
        total = 1;  // avoid div by 0
    }
    uint8_t pct_left = (uint8_t)((100 * local_presses_left) / total + 0.5f);
    uint8_t pct_right = (uint8_t)((100 * local_presses_right) / total + 0.5f);

    if (is_keyboard_left()) {
        // Layer state
        oled_set_cursor(0, 0);
        oled_write_P(PSTR("Layer:"), false);
        print_current_layer(1);

        // Lock status
        // TODO: NUM lock and SCROLL lock bit detection may not work on macOS
        //       Test on Windows/Linux
        // TODO: Keycode strings missing for NUM_LOCK and SCROLL_LOCK in keycode_string.h
        //       Manual patch needed
        led_t led_state = host_keyboard_led_state();
        if (led_state.num_lock) {
            oled_blit_16x16_P(NUM_LOCK_BITMAP, 0, 3);
        } else {
            oled_blit_16x16_P(EMPTY_BITMAP, 0, 3);
        }
        if (led_state.caps_lock) {
            oled_blit_16x16_P(CAPS_LOCK_BITMAP, 24, 3);
        } else {
            oled_blit_16x16_P(EMPTY_BITMAP, 24, 3);
        }
        if (led_state.scroll_lock) {
            oled_blit_16x16_P(SCROLL_LOCK_BITMAP, 48, 3);
        } else {
            oled_blit_16x16_P(EMPTY_BITMAP, 48, 3);
        }

        // split balance
        oled_set_cursor(0, 7);
        oled_write_P(PSTR("Left:"), false);
        oled_set_cursor(0, 8);
        char balance_buf[6];
        snprintf(balance_buf, sizeof(balance_buf), "%3u %%", pct_left);
        oled_print_right_aligned(balance_buf, g_oled_max_char);

        // Last key pressed
        oled_set_cursor(0, 10);
        oled_write_P(PSTR("Last Key:"), false);
        oled_set_cursor(0, 11);
        const char *keycode_str = get_keycode_string(unwrap_keycode(g_last_keycode));
        oled_print_right_aligned(keycode_str, g_oled_max_char);

        // QMK logo
        static const char PROGMEM QMK_LOGO_1[] = {
            0x81, 0x82, 0x83, 0x84, 0x00
        };
        static const char PROGMEM QMK_LOGO_2[] = {
            0xA1, 0xA2, 0xA3, 0xA4, 0x00
        };
        static const char PROGMEM QMK_LOGO_3[] = {
            0xC1, 0xC2, 0xC3, 0xC4, 0x00
        };
        oled_set_cursor(0, 13);
        oled_write_P(QMK_LOGO_1, false);
        oled_set_cursor(0, 14);
        oled_write_P(QMK_LOGO_2, false);
        oled_set_cursor(0, 15);
        oled_write_P(QMK_LOGO_3, false);
        oled_set_cursor(7, 15);
        oled_write_P(PSTR("QMK"), false);
    } else {
        // Uptime (only if oled is on)
        oled_set_cursor(0, 0);
        oled_write_P(PSTR("Uptime:"), false);
        print_uptime(1);

        // Typing speed
        oled_set_cursor(0, 3);
        oled_write_P(PSTR("Avg Speed"), false);
        oled_set_cursor(0, 4);
        oled_write_P(PSTR("(25 s):"), false);
        print_wpm(5);

        // split balance
        oled_set_cursor(0, 7);
        oled_write_P(PSTR("Right:"), false);
        oled_set_cursor(0, 8);
        char balance_buf[6];
        snprintf(balance_buf, sizeof(balance_buf), "%3u %%", pct_right);
        oled_print_right_aligned(balance_buf, g_oled_max_char);

        // Keebart logo
        oled_blit_24x24_P(KEEBART_BITMAP_24x24, 20, 11);
        oled_set_cursor(2, 15);
        oled_write_P(PSTR("KEEBART"), false);
    }
    return false;
}

void keyboard_post_init_user(void) {
    pin_t dsp_pen_pin = get_charge_pump_enable_pin();
    gpio_set_pin_output(dsp_pen_pin);
    gpio_write_pin_low(dsp_pen_pin);
    wait_ms(5);

    transaction_register_rpc(USER_SYNC_ONTIME, user_sync_ontime_slave);
    transaction_register_rpc(USER_SYNC_LASTKEY, user_sync_lastkey_slave);
    transaction_register_rpc(USER_SYNC_PRESSES, user_sync_presses_slave);

    if (!is_keyboard_master()) {
        wait_ms(90);
    }
}
#endif // OLED_ENABLE

