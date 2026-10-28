// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "studio_rgb_core.h"
#define KSR_COMMAND 0xCA
#define KSR_PACKET 32
#define KSR_JOB_IDLE 0

typedef struct {
    ksr_stage_t stage;
    uint8_t active[KSR_MAX_BYTES];
    uint32_t active_generation, active_crc;
    uint16_t layout, length, transfer_offset;
    uint8_t leds, mode, job, result;
#ifdef KEEBART_STUDIO_SPLIT_RETRY
    uint8_t retry_count, rpc_sequence;
    uint32_t retry_at;
#endif
    bool saving, remote_save, boot_sync;
    uint32_t job_started;
} ksr_engine_t;
extern ksr_engine_t ksr;
void ksr_init(uint16_t layout, uint8_t leds);
// role_authorized means USB master or trusted split peer, never Vial physical unlock.
void ksr_handle(const uint8_t *in, uint8_t *out, bool role_authorized, bool peer);
void ksr_task(uint32_t now);
void ksr_boot_sync(uint32_t now);
// Platform contracts: commit is idempotent; never called from the RPC callback.
const uint8_t *ksr_platform_saved(void);
enum ksr_status ksr_platform_commit(const ksr_stage_t *stage);
bool ksr_platform_rpc(const uint8_t *in, uint8_t *out);
void ksr_platform_mode_changed(uint8_t previous, uint8_t current);
// Capture underlying output before the physical driver replaces it with an overlay.
void ksr_capture_base_color(int index, uint8_t red, uint8_t green, uint8_t blue);
