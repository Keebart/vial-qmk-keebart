#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* Read-only application image metadata, patched only while preparing an update.
 * It authorizes exactly one macro payload, never a general security unlock. */
#define KS_RESTORE_BYTES 48
extern const volatile uint8_t ks_restore_image[KS_RESTORE_BYTES];
void ks_restore_hash(const void *data, size_t size, uint8_t out[32]);
bool ks_restore_available(void);
bool ks_restore_matches(const void *data, size_t size);
void ks_restore_consumed(void);
