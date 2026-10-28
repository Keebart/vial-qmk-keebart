#include "release.h"
#ifdef KEEBART_STUDIO_MACROS
#include "studio_dance.h"
#endif
// SPDX-License-Identifier: GPL-2.0-or-later
// RP2040 single-core adapter. SDK flash primitives execute from SRAM/ROM.
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/structs/sio.h"
#include "quantum.h"
#include "vial.h"
#include "via.h"
#include "encoder.h"
#include "qmk_settings.h"
#include "quantum/nvm/eeprom/nvm_eeprom_eeconfig_internal.h"
#include "quantum/nvm/eeprom/nvm_eeprom_via_internal.h"
#include "dynamic_keymap.h"
#include "transactions.h"
#include "synchronization_util.h"
#include "studio_rgb.h"
#include "studio_reset.h"
#ifdef KEEBART_STUDIO_MACROS
#include "studio_macro.h"
void ksn_task(void);
void ksn_handle(uint8_t *,bool);
#endif
#ifdef KEEBART_STUDIO_TEST_ENABLE
#include "studio_test.h"
#endif
#include "wear_leveling_rp2040_flash_config.h"
#include <string.h>

#ifndef SPLIT_LED_STATE_ENABLE
#    error Studio lock indicators require synchronized host LED state on both halves
#endif

#ifndef NUM_ENCODERS
#    define NUM_ENCODERS 0
#endif

#define KSR_FLASH_BASE 0x1F8000u
#define KSR_LEGACY_FLASH_BASE 0x1FC000u
#define KSR_BYTES KSR_LENGTH(RGB_MATRIX_LED_COUNT)
_Static_assert(PICO_FLASH_SIZE_BYTES == 0x200000, "Studio storage is reserved for the configured 2MiB flash only");
_Static_assert(WEAR_LEVELING_RP2040_FLASH_BASE == 0x1FE000 && WEAR_LEVELING_BACKING_SIZE == 8192 && WEAR_LEVELING_LOGICAL_SIZE == 4096, "Do not move existing Vial EEPROM");
_Static_assert(KSR_FLASH_BASE + 2 * KSR_SLOT_SIZE == KSR_LEGACY_FLASH_BASE && KSR_LEGACY_FLASH_BASE + 2 * KSR_LEGACY_SLOT_SIZE == WEAR_LEVELING_RP2040_FLASH_BASE, "Flash partitions overlap");
_Static_assert(DYNAMIC_KEYMAP_LAYER_COUNT == KSR_LAYERS && MATRIX_ROWS == KSR_EXPECTED_ROWS && MATRIX_COLS == KSR_EXPECTED_COLS, "Vial matrix/layer ABI changed");
_Static_assert(NUM_ENCODERS == KSR_EXPECTED_ENCODERS && EECONFIG_SIZE == 37 && VIA_EEPROM_LAYOUT_OPTIONS_SIZE == 1 && VIA_EEPROM_CUSTOM_CONFIG_SIZE == 0, "Vial EEPROM prefix/encoder ABI changed");
_Static_assert(sizeof(qmk_settings_t) == 40 && VIAL_TAP_DANCE_ENTRIES == 32 && VIAL_COMBO_ENTRIES == KSR_EXPECTED_COMBOS && VIAL_KEY_OVERRIDE_ENTRIES == 32 && VIAL_ALT_REPEAT_KEY_ENTRIES == 32, "Vial dynamic-entry ABI changed; migration required");
_Static_assert(RGB_MATRIX_LED_COUNT == KSR_EXPECTED_LEDS, "Palette layout/LED ABI mismatch");
_Static_assert(KSR_HEADER_SIZE + KSR_BYTES < KSR_COMMIT_OFFSET, "Palette slot overflow");
_Static_assert(CRT0_EXTRA_CORES_NUMBER == 0 && RP_CORE1_START == FALSE, "Studio flash adapter requires parked core 1");
_Static_assert(VIA_EEPROM_CONFIG_END == 41 && DYNAMIC_KEYMAP_MACRO_COUNT == 16, "Vial prefix/macro ABI changed");
#ifdef DYNAMIC_KEYMAP_EEPROM_ADDR
_Static_assert(DYNAMIC_KEYMAP_EEPROM_ADDR == 41, "Vial keymap offset changed");
#endif
#ifdef DYNAMIC_KEYMAP_EEPROM_MAX_ADDR
_Static_assert(DYNAMIC_KEYMAP_EEPROM_MAX_ADDR == 4095, "Vial macro space changed");
#endif
#ifdef DYNAMIC_KEYMAP_MACRO_EEPROM_ADDR
_Static_assert(DYNAMIC_KEYMAP_MACRO_EEPROM_ADDR == KSR_EXPECTED_MACRO_BASE, "Vial macro offset changed");
#endif
#ifdef DYNAMIC_KEYMAP_MACRO_EEPROM_SIZE
_Static_assert(DYNAMIC_KEYMAP_MACRO_EEPROM_SIZE == WEAR_LEVELING_LOGICAL_SIZE - KSR_EXPECTED_MACRO_BASE, "Vial macro capacity changed");
#endif
// Match the existing Vial EEPROM schema for this build; palettes occupy separate flash.
#ifdef ENCODER_MAP_ENABLE
#    define KSR_SCHEMA_ENCODERS NUM_ENCODERS
#else
#    define KSR_SCHEMA_ENCODERS 0
#endif
#define KSR_SCHEMA_MACRO_BASE (VIA_EEPROM_CONFIG_END + DYNAMIC_KEYMAP_LAYER_COUNT * MATRIX_ROWS * MATRIX_COLS * 2 + KSR_SCHEMA_ENCODERS * DYNAMIC_KEYMAP_LAYER_COUNT * 4 + sizeof(qmk_settings_t) + sizeof(vial_tap_dance_entry_t) * VIAL_TAP_DANCE_ENTRIES + sizeof(vial_combo_entry_t) * VIAL_COMBO_ENTRIES + sizeof(vial_key_override_entry_t) * VIAL_KEY_OVERRIDE_ENTRIES + sizeof(vial_alt_repeat_key_entry_t) * VIAL_ALT_REPEAT_KEY_ENTRIES)
_Static_assert(KSR_SCHEMA_MACRO_BASE == KSR_EXPECTED_MACRO_BASE, "Studio would change the model's audited Vial macro offset");
_Static_assert(KSR_SCHEMA_MACRO_BASE + 100 < WEAR_LEVELING_LOGICAL_SIZE, "Vial schema leaves insufficient macro storage");
_Static_assert(KSR_EXPECTED_LEDS <= KSR_MAX_LEDS && KSR_HEADER_SIZE + KSR_LENGTH(KSR_EXPECTED_LEDS) <= KSR_COMMIT_OFFSET, "Studio palette exceeds reserved slot");
static int selected = -1;
static uint8_t page[FLASH_PAGE_SIZE] __attribute__((aligned(4)));
static uint32_t last_task;
static bool boot_sync_started;
static const uint8_t *slot(int n) {
    return n < 2 ? (const uint8_t *)(XIP_BASE + KSR_FLASH_BASE + (unsigned)n * KSR_SLOT_SIZE)
                 : (const uint8_t *)(XIP_BASE + KSR_LEGACY_FLASH_BASE + (unsigned)(n - 2) * KSR_LEGACY_SLOT_SIZE);
}
static const uint8_t *legacy_slot(int n) {
    const uint8_t *s = slot(n + 2);
    return s[4] == 1 || s[4] == 2 ? s : NULL;
}
const uint8_t *ksr_platform_saved(void) { return selected < 0 ? NULL : slot(selected); }

static void flash_erase_slot(unsigned n) {
    chSysLock();
    uint32_t irq = save_and_disable_interrupts();
    flash_range_erase(KSR_FLASH_BASE + n * KSR_SLOT_SIZE, KSR_SLOT_SIZE);
    restore_interrupts(irq);
    chSysUnlock();
}
static void flash_page(unsigned n, uint16_t offset) {
    chSysLock();
    uint32_t irq = save_and_disable_interrupts();
    flash_range_program(KSR_FLASH_BASE + n * KSR_SLOT_SIZE + offset, page, sizeof(page));
    restore_interrupts(irq);
    chSysUnlock();
}
enum ksr_status ksr_platform_commit(const ksr_stage_t *s) {
    if (!s->valid || s->length != KSR_BYTES || sio_hw->cpuid != 0) return KSR_IO;
    // Alternate independent erase sectors: never erase the currently selected record.
    unsigned target = selected == 0 ? 1 : 0;
    flash_erase_slot(target);
    uint8_t h[KSR_HEADER_SIZE]; ksr_header(h, KSR_LAYOUT_ID, RGB_MATRIX_LED_COUNT, s);
    for (uint16_t off = 0; off < KSR_HEADER_SIZE + KSR_BYTES; off += sizeof(page)) {
        memset(page, 255, sizeof(page));
        for (unsigned j = 0; j < sizeof(page); j++) {
            unsigned at = off + j;
            if (at < KSR_HEADER_SIZE) page[j] = h[at];
            else if (at < KSR_HEADER_SIZE + KSR_BYTES) page[j] = s->data[at - KSR_HEADER_SIZE];
        }
        flash_page(target, off);
        if (memcmp(slot(target) + off, page, sizeof(page))) return KSR_IO;
    }
    // The final page is programmed once, only after complete readback.
    memset(page, 255, sizeof(page)); ksr_put32(page, KSR_COMMIT_MAGIC);
    flash_page(target, KSR_COMMIT_OFFSET);
    if (!ksr_slot_valid(slot(target), KSR_LAYOUT_ID, RGB_MATRIX_LED_COUNT)) return KSR_IO;
    selected = target;
    return KSR_OK;
}

enum ksr_status ksr_platform_factory_reset(void) {
    if (sio_hw->cpuid != 0) return KSR_IO;
    // Clear current and legacy lighting records, preventing fallback resurrection.
    flash_erase_slot(0); flash_erase_slot(1);
    for (unsigned n = 0; n < 2; n++) {
        chSysLock();
        uint32_t irq = save_and_disable_interrupts();
        flash_range_erase(KSR_LEGACY_FLASH_BASE + n * KSR_LEGACY_SLOT_SIZE, KSR_LEGACY_SLOT_SIZE);
        restore_interrupts(irq); chSysUnlock();
    }
    for (unsigned n = 0; n < 4; n++) {
        unsigned size = n < 2 ? KSR_SLOT_SIZE : KSR_LEGACY_SLOT_SIZE;
        for (unsigned i = 0; i < size; i++) if (slot(n)[i] != 255) return KSR_IO;
    }
    selected = -1;
#ifdef KEEBART_STUDIO_MACROS
    if (!ksm_reset()) return KSR_IO;
#endif
#ifdef KEEBART_STUDIO_DEVICE_NAME
    extern void ksn_reset(void);
    ksn_reset();
#endif
    clear_keyboard();
    eeconfig_init(); // Formats EEPROM, restores keymap/encoders/macros/dynamic entries and RGB.
    qmk_settings_reset();
    eeconfig_read_keymap(&keymap_config);
    layer_clear(); default_layer_set(1);
    rgb_matrix_reload_from_eeprom();
    vial_unlocked = 0;
    ksr_init(KSR_LAYOUT_ID, RGB_MATRIX_LED_COUNT);
    rgb_matrix_request_reinit();
    return KSR_OK;
}

bool ksr_platform_rpc(const uint8_t *in, uint8_t *out) {
    return transaction_rpc_exec(KSR_SYNC, 32, in, 32, out);
}
static void release_reply(uint8_t *data, bool internal) {
    uint8_t request[32];memcpy(request,data,32);
    memset(data+6,0,26);
    if (request[0]!=KSR_COMMAND || request[1]!='K' || request[2]!='B' || request[3]!=KSR_VERSION ||
        request[4]!=0xf0 || request[6]>1) {data[6]=KSR_BAD_PACKET;return;}
    for(unsigned i=7;i<32;i++) if(request[i]) {data[6]=KSR_BAD_PACKET;return;}
    if(request[6]) {
        if(internal || !is_keyboard_master()) {data[6]=KSR_UNSUPPORTED;return;}
        request[6]=0;
        uint8_t response[32];
        if(!ksr_platform_rpc(request,response) || memcmp(response,request,6) || response[6]) {data[6]=KSR_PEER;return;}
        memcpy(data+7,response+7,12);
    } else {
        keebart_release(data+7,KSR_LAYOUT_ID,is_keyboard_left()?1:2,KSR_STORAGE_VERSION);
    }
}
static void peer_callback(uint8_t in_len, const void *in, uint8_t out_len, void *out) {
    if (in_len != 32 || out_len != 32 || is_keyboard_master()) { if (out_len) memset(out, 0, out_len); return; }
#ifdef KEEBART_STUDIO_MACROS
    if (((const uint8_t *)in)[4] == 0xe1) {memcpy(out,in,32);ksn_handle(out,true);return;}
#endif
    if (((const uint8_t *)in)[4] == 0xf0) {memcpy(out,in,32);release_reply(out,true);return;}
    // Internal split link only. The USB-facing master checks physical Vial unlock.
    if (((const uint8_t *)in)[4] == KSR_FACTORY_RESET) ksr_reset_handle(in, out, true, true);
    else ksr_handle(in, out, true, true);
}
void keyboard_post_init_kb(void) {
    selected = ksr_select_slot(slot(0), slot(1), KSR_LAYOUT_ID, RGB_MATRIX_LED_COUNT);
    if (selected < 0) {
        int old = ksr_select_slot(legacy_slot(0), legacy_slot(1), KSR_LAYOUT_ID, RGB_MATRIX_LED_COUNT);
        if (old >= 0) selected = old + 2;
    }
    ksr_init(KSR_LAYOUT_ID, RGB_MATRIX_LED_COUNT);
    transaction_register_rpc(KSR_SYNC, peer_callback);
    keyboard_post_init_user();
}
void housekeeping_task_kb(void) {
    uint32_t now = timer_read32();
#ifdef KEEBART_STUDIO_MACROS
    if (is_keyboard_master()) { ksm_task(); kst_task(); }
#endif
    if (is_keyboard_master() && !boot_sync_started && now > 1500) {
        boot_sync_started = true; ksr_boot_sync(now); // RAM only, once; failures are visible and require explicit retry.
    }
    if (now - last_task < 12) return;
    last_task = now;
    // Serial RPC runs on a second ChibiOS thread; it already owns this mutex.
    bool slave = !is_keyboard_master();
    if (slave) split_shared_memory_lock();
#ifdef KEEBART_STUDIO_MACROS
    ksn_task();
#endif
    ksr_reset_task(now);
    ksr_task(now);
    if (slave) split_shared_memory_unlock();
}
#ifdef KEEBART_STUDIO_TEST_ENABLE
// The master processes both halves' encoder-map events, even for non-HID actions.
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    (void)keycode;
#ifdef ENCODER_MAP_ENABLE
    if (is_keyboard_master() && record->event.pressed && IS_ENCODEREVENT(record->event))
        ksr_test_encoder(record->event.key.col, record->event.type == ENCODER_CW_EVENT);
#else
    (void)record;
#endif
#ifdef KEEBART_STUDIO_MACROS
    kst_observe(keycode,record);
    if (!ksm_key(keycode, record->event.pressed)) return false;
#endif
    return true;
}
#endif
#ifdef KEEBART_STUDIO_TAP_DANCE
bool process_record_user(uint16_t keycode,keyrecord_t *record){return kst_key(keycode,record);}
#endif
void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    if (length != 32 || data[0] != KSR_COMMAND) { if (length) data[0] = 0xFF; return; }
#ifdef KEEBART_STUDIO_MACROS
    if (data[4] == 0xe1) {ksn_handle(data,false);return;}
#endif
    if (data[4] == 0xf0) {release_reply(data,false);return;}
    // This narrowly namespaced handler changes only LED palettes/mode.
    // Physical Vial unlock remains enforced by the existing VIA/Vial paths for protected operations.
    if (data[1] == 'K' && data[2] == 'B' && data[3] == KSR_VERSION && data[4] == KSR_LED) {
        uint8_t row = data[6], col = data[7];
        memset(data + 6, 0, 26);
        if (row >= MATRIX_ROWS || col >= MATRIX_COLS) data[6] = KSR_BOUNDS;
        else { data[6] = KSR_OK; data[7] = g_led_config.matrix_co[row][col]; }
        return;
    }
#ifdef KEEBART_STUDIO_TEST_ENABLE
    if (data[4] == KSR_TEST_ENCODERS) {
        ksr_test_handle(data, data, vial_unlocked && !vial_unlock_in_progress && is_keyboard_master(), KSR_EXPECTED_ENCODERS);
        return;
    }
#endif
    if (data[4] == KSR_FACTORY_RESET) {
        ksr_reset_handle(data, data, vial_unlocked && !vial_unlock_in_progress && is_keyboard_master(), false);
        return;
    }
    ksr_handle(data, data, !vial_unlock_in_progress && is_keyboard_master(), false);
}
