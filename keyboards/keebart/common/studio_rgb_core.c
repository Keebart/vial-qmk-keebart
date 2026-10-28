// SPDX-License-Identifier: GPL-2.0-or-later
#include "studio_rgb_core.h"
#include <string.h>
uint16_t ksr_get16(const uint8_t *p) { return (uint16_t)p[0] | (uint16_t)p[1] << 8; }
uint32_t ksr_get32(const uint8_t *p) { return (uint32_t)ksr_get16(p) | (uint32_t)ksr_get16(p + 2) << 16; }
void ksr_put16(uint8_t *p, uint16_t v) { p[0] = v; p[1] = v >> 8; }
void ksr_put32(uint8_t *p, uint32_t v) { ksr_put16(p, v); ksr_put16(p + 2, v >> 16); }
uint32_t ksr_crc32(const uint8_t *p, size_t n) {
    uint32_t c = 0xFFFFFFFFu;
    while (n--) { c ^= *p++; for (unsigned b = 0; b < 8; b++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u))); }
    return ~c;
}
bool ksr_newer(uint32_t a, uint32_t b) { uint32_t d = a - b; return d && d < 0x80000000u; }
uint8_t ksr_layer(uint32_t layers, uint32_t defaults) {
    uint32_t state = layers | defaults;
    uint8_t result = 0;
    while (state >>= 1) result++;
    return result; // Caller deliberately blanks unsupported layers, never wraps.
}
uint8_t ksr_scale(uint8_t c, uint8_t brightness) { return (uint16_t)c * brightness / 255u; }
enum ksr_status ksr_begin(ksr_stage_t *s, uint16_t length, uint8_t mode, uint32_t generation, uint32_t crc) {
    if (!length || length > KSR_MAX_BYTES || mode > 1) return KSR_BOUNDS;
    memset(s, 0, sizeof(*s));
    s->length = length; s->mode = mode; s->generation = generation; s->crc = crc; s->open = true;
    return KSR_OK;
}
enum ksr_status ksr_write(ksr_stage_t *s, uint16_t offset, const uint8_t *p, uint8_t n) {
    if (!s->open || !n || n > 23 || offset >= s->length || n > s->length - offset) return KSR_BOUNDS;
    // Reject the whole chunk before changing anything. Identical retries are harmless.
    for (uint16_t i = 0; i < n; i++) {
        uint16_t at = offset + i;
        if ((s->seen[at / 8] & (1u << (at % 8))) && s->data[at] != p[i]) return KSR_CONFLICT;
    }
    for (uint16_t i = 0; i < n; i++) {
        uint16_t at = offset + i;
        if (!(s->seen[at / 8] & (1u << (at % 8)))) { s->received++; s->seen[at / 8] |= 1u << (at % 8); }
        s->data[at] = p[i];
    }
    s->valid = false;
    return KSR_OK;
}
enum ksr_status ksr_validate(ksr_stage_t *s) {
    s->valid = false;
    if (!s->open || s->received != s->length) return KSR_INCOMPLETE;
    if (ksr_crc32(s->data, s->length) != s->crc) return KSR_CRC;
    s->valid = true;
    return KSR_OK;
}
void ksr_header(uint8_t *h, uint16_t layout, uint8_t leds, const ksr_stage_t *s) {
    memset(h, 0, KSR_HEADER_SIZE);
    ksr_put32(h, KSR_MAGIC); h[4] = KSR_STORAGE_VERSION; h[5] = KSR_HEADER_SIZE;
    ksr_put16(h + 6, layout); h[8] = KSR_LAYERS; h[9] = leds;
    ksr_put16(h + 10, s->length); ksr_put32(h + 12, s->generation);
    h[16] = s->mode; ksr_put32(h + 20, s->crc);
    ksr_put32(h + 28, ksr_crc32(h, 28));
}
bool ksr_indicators_valid(const uint8_t *data, uint16_t length, uint8_t leds) {
    if (!leds || leds > KSR_MAX_LEDS || length != KSR_LENGTH(leds)) return false;
    for (unsigned led = 0; led < KSR_LAYERS * leds; led++) {
        uint8_t mask = data[KSR_RGB_LENGTH(leds) + led * 4];
        // One standard USB host LED: Num, Caps, Scroll, Compose or Kana; zero disables.
        if ((mask & ~0x1Fu) || (mask && (mask & (mask - 1u)))) return false;
    }
    return true;
}
bool ksr_slot_valid(const uint8_t *s, uint16_t layout, uint8_t leds) {
    if (!s || ksr_get32(s) != KSR_MAGIC || s[4] < 1 || s[4] > KSR_STORAGE_VERSION ||
        s[5] != KSR_HEADER_SIZE || ksr_get16(s + 6) != layout ||
        s[8] != KSR_LAYERS || s[9] != leds || leds > KSR_MAX_LEDS || !leds || s[16] > 1 ||
        ksr_crc32(s, 28) != ksr_get32(s + 28)) return false;
    unsigned length = s[4] == 1 ? KSR_RGB_LENGTH(leds) : (s[4] == 2 ? KSR_RGB_LENGTH(leds) + leds * 4u : KSR_LENGTH(leds));
    unsigned commit = s[4] == KSR_STORAGE_VERSION ? KSR_COMMIT_OFFSET : KSR_LEGACY_COMMIT_OFFSET;
    if (ksr_get16(s + 10) != length || ksr_get32(s + commit) != KSR_COMMIT_MAGIC) return false;
    if (s[4] >= 2) {
        const uint8_t *data = s + KSR_HEADER_SIZE;
        unsigned count = s[4] == 2 ? leds : KSR_LAYERS * leds;
        for (unsigned i = 0; i < count; i++) {
            uint8_t mask = data[KSR_RGB_LENGTH(leds) + i * 4];
            if ((mask & ~0x1Fu) || (mask && (mask & (mask - 1u)))) return false;
        }
    }
    return ksr_crc32(s + KSR_HEADER_SIZE, length) == ksr_get32(s + 20);
}
int ksr_select_slot(const uint8_t *a, const uint8_t *b, uint16_t layout, uint8_t leds) {
    bool va = ksr_slot_valid(a, layout, leds), vb = ksr_slot_valid(b, layout, leds);
    if (!va) return vb ? 1 : -1;
    if (!vb) return 0;
    return ksr_newer(ksr_get32(b + 12), ksr_get32(a + 12)) ? 1 : 0;
}
