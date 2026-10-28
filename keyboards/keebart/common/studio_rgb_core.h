// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KSR_VERSION 1
#define KSR_STORAGE_VERSION 3
// CAPS flags are negotiated independently of framing/storage version.
#define KSR_CAP_FLASH 0x01u
#define KSR_CAP_SPLIT 0x02u
#define KSR_CAP_UNLOCK_REQUIRED 0x04u
#define KSR_CAP_INDICATORS 0x08u
#define KSR_CAP_LAYER_INDICATORS 0x10u
#define KSR_CAP_FACTORY_RESET 0x20u
#define KSR_CAPABILITIES (KSR_CAP_FLASH | KSR_CAP_SPLIT | KSR_CAP_INDICATORS | KSR_CAP_LAYER_INDICATORS | KSR_CAP_FACTORY_RESET)
#define KSR_LAYERS 16
#define KSR_MAX_LEDS 60
#define KSR_RGB_LENGTH(leds) (KSR_LAYERS * (leds) * 3u)
#define KSR_LENGTH(leds) (KSR_RGB_LENGTH(leds) + KSR_LAYERS * (leds) * 4u)
#define KSR_MAX_BYTES KSR_LENGTH(KSR_MAX_LEDS)
#define KSR_SLOT_SIZE 8192
#define KSR_LEGACY_SLOT_SIZE 4096
#define KSR_LEGACY_COMMIT_OFFSET 3840
#define KSR_HEADER_SIZE 32
#define KSR_COMMIT_OFFSET 7936
#define KSR_COMMIT_MAGIC 0x54494D43u
#define KSR_MAGIC 0x3152534Bu

enum ksr_status { KSR_OK, KSR_BAD_PACKET, KSR_LOCKED, KSR_BOUNDS, KSR_CONFLICT, KSR_INCOMPLETE, KSR_CRC, KSR_BUSY, KSR_STALE, KSR_IO, KSR_PEER, KSR_PENDING, KSR_UNSUPPORTED };
enum ksr_op { KSR_CAPS, KSR_STATUS, KSR_BEGIN, KSR_WRITE, KSR_VALIDATE, KSR_PREVIEW, KSR_SAVE, KSR_READ, KSR_ABORT, KSR_LED, KSR_PEER_STATUS };
typedef struct {
    uint8_t data[KSR_MAX_BYTES];
    uint8_t seen[(KSR_MAX_BYTES + 7) / 8];
    uint32_t generation, crc;
    uint16_t length, received;
    uint8_t mode;
    bool open, valid;
} ksr_stage_t;

uint16_t ksr_get16(const uint8_t *p);
uint32_t ksr_get32(const uint8_t *p);
void ksr_put16(uint8_t *p, uint16_t v);
void ksr_put32(uint8_t *p, uint32_t v);
uint32_t ksr_crc32(const uint8_t *p, size_t n);
bool ksr_newer(uint32_t a, uint32_t b);
uint8_t ksr_layer(uint32_t layers, uint32_t defaults);
uint8_t ksr_scale(uint8_t channel, uint8_t brightness);
enum ksr_status ksr_begin(ksr_stage_t *s, uint16_t length, uint8_t mode, uint32_t generation, uint32_t crc);
enum ksr_status ksr_write(ksr_stage_t *s, uint16_t offset, const uint8_t *p, uint8_t n);
enum ksr_status ksr_validate(ksr_stage_t *s);
void ksr_header(uint8_t *h, uint16_t layout, uint8_t leds, const ksr_stage_t *s);
bool ksr_indicators_valid(const uint8_t *data, uint16_t length, uint8_t leds);
bool ksr_slot_valid(const uint8_t *slot, uint16_t layout, uint8_t leds);
int ksr_select_slot(const uint8_t *a, const uint8_t *b, uint16_t layout, uint8_t leds);
