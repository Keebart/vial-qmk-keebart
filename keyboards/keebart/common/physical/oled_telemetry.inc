// Display telemetry is volatile and independent of configuration persistence.
// One bounded snapshot replaces the original two synchronous RPCs per keypress.
#include "transactions.h"
#include "synchronization_util.h"
#include "studio_reset.h"
#include "locales/locale.h"
#include "oled_preset.h"
#include <stdio.h>

struct ksh_oled_state {
    uint32_t left, right, uptime;
    uint16_t keycode;
    uint8_t wpm, awake, computer, layer, mods, locks;
};
_Static_assert(sizeof(struct ksh_oled_state) == 20, "OLED wire size");
static struct ksh_oled_state g_local, g_remote, g_display;
static uint32_t g_seen_presses, g_last_press_ms;
extern uint16_t ksl_qmk_display_code(uint16_t code);
extern bool ksl_qmk_display_replay(void);
extern unsigned ksl_qmk_display_computer(void);
extern void ksl_qmk_key_label(uint16_t,unsigned,char *,size_t);

static uint8_t ksh_oled_pack_mods(void) {
    uint8_t raw = get_mods();
#ifndef NO_ACTION_ONESHOT
    raw |= get_oneshot_mods();
#endif
    uint8_t out = 0;
    if (raw & 0x22u) out |= KSO_MOD_SHIFT;
    if (raw & 0x11u) out |= KSO_MOD_CTRL;
    if (raw & 0x44u) out |= KSO_MOD_ALT;
    if (raw & 0x88u) out |= KSO_MOD_GUI;
    return out;
}

static uint8_t ksh_oled_pack_locks(void) {
    led_t led = host_keyboard_led_state();
    uint8_t out = 0;
    if (led.num_lock) out |= KSO_LOCK_NUM;
    if (led.caps_lock) out |= KSO_LOCK_CAPS;
    if (led.scroll_lock) out |= KSO_LOCK_SCROLL;
#ifdef CAPS_WORD_ENABLE
    if (is_caps_word_on()) out |= KSO_LOCK_CAPS_WORD;
#endif
    return out;
}

static void ksh_oled_receive(uint8_t length, const void *data, uint8_t out_length, void *out) {
    (void)out_length; (void)out;
    if (length == sizeof(g_remote) && !is_keyboard_master()) memcpy(&g_remote, data, sizeof(g_remote));
}
void keyboard_post_init_user(void) {
    transaction_register_rpc(KSH_OLED_SYNC, ksh_oled_receive);
}
void ksh_oled_record(uint16_t code, keyrecord_t *record) {
    static uint32_t held[MATRIX_ROWS];
    if (!is_keyboard_master() || !IS_KEYEVENT(record->event) || ksl_qmk_display_replay() ||
        record->event.key.row >= MATRIX_ROWS || record->event.key.col >= MATRIX_COLS) return;
    unsigned row=record->event.key.row;uint32_t bit=1u<<record->event.key.col;
    if(!record->event.pressed){if(!matrix_is_on(row,record->event.key.col))held[row]&=~bit;return;}
    if((held[row]&bit)||!matrix_is_on(row,record->event.key.col))return;
    held[row]|=bit;
    if (row < MATRIX_ROWS / 2) g_local.left++;
    else g_local.right++;
    g_local.keycode = code;
    g_local.computer = ksl_qmk_display_computer();
}

bool wpm_keycode_user(uint16_t code) {
    // Buffered actions re-enter QMK; count the physical key only once.
    if (ksl_qmk_display_replay()) return false;
    code = ksl_qmk_display_code(code);
    return ksl_character(code) || (code >= KC_A && code <= KC_0) || (code >= KC_TAB && code <= KC_SLASH);
}
static void ksh_oled_snapshot(void) {
    if (is_keyboard_master()) {
        g_display = g_local;
        g_display.uptime = timer_read32();
        g_display.wpm = get_current_wpm();
        extern bool ksl_qmk_display_idle(void);
        g_display.awake = !ksl_qmk_display_idle();
        g_display.layer = get_highest_layer(layer_state | default_layer_state);
        g_display.mods = ksh_oled_pack_mods();
        g_display.locks = ksh_oled_pack_locks();
    } else {
        // The serial callback runs on another thread. Never render under its lock.
        split_shared_memory_lock();
        g_display = g_remote;
        split_shared_memory_unlock();
    }
    uint32_t total = g_display.left + g_display.right;
    if (total != g_seen_presses) {
        g_seen_presses = total;
        g_last_press_ms = timer_read32();
        if (g_last_press_ms == 0) g_last_press_ms = 1;
    }
}
void housekeeping_task_user(void) {
    static uint32_t sent_at;
    static struct ksh_oled_state last_sent;
    if (!is_keyboard_master() || ksr_reset_busy()) return;
    uint8_t peer_display = is_keyboard_left() ? KSH_RIGHT_DISPLAY : KSH_LEFT_DISPLAY;
    if (!(ksh_flags() & peer_display)) return;
    uint32_t elapsed = timer_elapsed32(sent_at);
    if (elapsed < 80u) return;
    ksh_oled_snapshot();
    bool changed = g_display.left != last_sent.left || g_display.right != last_sent.right ||
                   g_display.keycode != last_sent.keycode || g_display.awake != last_sent.awake ||
                   g_display.layer != last_sent.layer || g_display.mods != last_sent.mods ||
                   g_display.locks != last_sent.locks;
    if (!changed && elapsed < 500u) return;
    sent_at = timer_read32();
    last_sent = g_display;
    (void)transaction_rpc_send(KSH_OLED_SYNC, sizeof(g_display), &g_display);
}
static void ksh_oled_format_ascii(const char *utf8, char *out, size_t size) {
    if (!size) return;
    unsigned len = 0;
    for (unsigned i = 0; utf8[i] && len + 1 < size;) {
        uint32_t c = (unsigned char)utf8[i++];
        if (c >= 128u) {
            unsigned n = (c & 0xe0u) == 0xc0u ? 1u : (c & 0xf0u) == 0xe0u ? 2u : 3u;
            c &= (1u << (6u - n)) - 1u;
            while (n-- && utf8[i]) c = (c << 6) | ((unsigned char)utf8[i++] & 63u);
        }
        if (c >= 32u && c < 127u) out[len++] = (char)c;
        else if (c == 0x2212u) out[len++] = '-';
        else if (c == 0x2190u) out[len++] = '<';
        else if (c == 0x2192u) out[len++] = '>';
        else if (c == 0x25ccu && len + 4 < size) { memcpy(out + len, "dead", 4); len += 4; }
        else if (len + 7 < size) len += (unsigned)snprintf(out + len, size - len, "U+%04lX", (unsigned long)c);
        else break;
    }
    out[len] = '\0';
}
static void ksh_oled_format_last_key(char *out, size_t size) {
    if (!size) return;
    if (g_display.keycode == 0) { out[0] = '\0'; return; }
    char utf8[192];
    ksl_qmk_key_label(g_display.keycode, g_display.computer, utf8, sizeof(utf8));
    ksh_oled_format_ascii(utf8, out, size);
}
