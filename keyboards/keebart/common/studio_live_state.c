#include "quantum.h"
#include "vial.h"
#include "studio_rgb.h"
#include "live_state/live_state.h"

#ifdef KEEBART_STUDIO_LOCALES
extern unsigned ksl_qmk_display_computer(void);
#endif

void ksr_live_state_handle(uint8_t data[32]) {
    uint8_t in[32];
    memcpy(in, data, 32);
    memset(data + 6, 0, 26);
    data[6] = KSR_BAD_PACKET;
    if (in[0] != KSR_COMMAND || in[1] != 'K' || in[2] != 'B' || in[3] != KSR_VERSION || in[4] != 0xe4) {
        return;
    }
    for (unsigned i = 6; i < 32; i++) {
        if (in[i]) {
            return;
        }
    }
    if (!is_keyboard_master()) {
        data[6] = KSR_LOCKED;
        return;
    }
    struct ksls_snapshot snap;
    memset(&snap, 0, sizeof(snap));
    layer_state_t combined = layer_state | default_layer_state;
    snap.active_layer = get_highest_layer(combined);
    snap.default_layer = get_highest_layer(default_layer_state);
    snap.layer_mask = (uint16_t)(combined & 0xffffu);
    uint8_t mods = get_mods();
#ifndef NO_ACTION_ONESHOT
    mods |= get_oneshot_mods();
#endif
    snap.modifiers = mods;
    led_t led = host_keyboard_led_state();
    if (led.caps_lock) snap.flags |= KSLS_FLAG_CAPS_LOCK;
    if (led.num_lock) snap.flags |= KSLS_FLAG_NUM_LOCK;
    if (led.scroll_lock) snap.flags |= KSLS_FLAG_SCROLL_LOCK;
#ifdef CAPS_WORD_ENABLE
    if (is_caps_word_on()) snap.flags |= KSLS_FLAG_CAPS_WORD;
#endif
#ifdef KEEBART_STUDIO_LOCALES
    snap.computer = (uint8_t)ksl_qmk_display_computer();
#endif
    bool unlocked = vial_unlocked && !vial_unlock_in_progress;
    if (unlocked) {
        uint8_t rows = MATRIX_ROWS;
        if (rows > KSLS_MAX_MATRIX_BYTES) {
            rows = KSLS_MAX_MATRIX_BYTES;
        }
        snap.matrix_len = rows;
        for (uint8_t r = 0; r < rows; r++) {
            snap.matrix[r] = (uint8_t)(matrix_get_row(r) & ((1u << MATRIX_COLS) - 1u));
        }
    }
    data[6] = KSR_OK;
    ksls_pack(data + 7, &snap, unlocked, false);
}
