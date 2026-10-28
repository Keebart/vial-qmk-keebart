#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define KN_MAX 120
#define KN_RECORD 133
#define KN_PAYLOAD 25
enum { KN_OK, KN_INVALID, KN_BOUNDS, KN_BUSY, KN_IO, KN_PENDING };
struct kn_state {
    uint8_t record[KN_RECORD], staged[KN_MAX], seen[KN_MAX];
    uint32_t transaction, crc;
    uint8_t length, result;
    bool staging, pending;
};
bool kn_valid(const uint8_t *, size_t);
uint32_t kn_crc(const void *, size_t);
void kn_init(struct kn_state *, const char *, const uint8_t *);
uint8_t kn_handle(struct kn_state *, const uint8_t[KN_PAYLOAD], uint8_t[KN_PAYLOAD], bool);
bool kn_reset(struct kn_state *, const char *, bool (*)(const uint8_t[KN_RECORD]));
void kn_task(struct kn_state *, bool (*)(const uint8_t[KN_RECORD]));
size_t kn_usb_descriptor(const uint8_t[KN_RECORD], uint8_t[242]);
