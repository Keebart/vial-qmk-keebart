// SPDX-License-Identifier: GPL-2.0-or-later
#include "studio_rgb.h"
#include "studio_reset.h"
#include <string.h>
ksr_engine_t ksr;
static void packet(uint8_t *p, uint8_t op) { memset(p, 0, KSR_PACKET); p[0] = KSR_COMMAND; p[1] = 'K'; p[2] = 'B'; p[3] = KSR_VERSION; p[4] = op; }
static void activate(void) {
    uint8_t old_mode = ksr.mode;
    memcpy(ksr.active, ksr.stage.data, ksr.length);
    ksr.active_generation = ksr.stage.generation; ksr.active_crc = ksr.stage.crc; ksr.mode = ksr.stage.mode;
    ksr_platform_mode_changed(old_mode, ksr.mode);
}
void ksr_init(uint16_t layout, uint8_t leds) {
    memset(&ksr, 0, sizeof(ksr)); ksr.layout = layout; ksr.leds = leds; ksr.length = KSR_LENGTH(leds);
    if (ksr.length > KSR_MAX_BYTES) { ksr.length = 0; return; }
    ksr.mode = 1; // Fresh or reset storage starts with Studio's standard per-key blue.
    for (uint16_t i = 0; i < KSR_RGB_LENGTH(leds); i += 3) { ksr.active[i] = 0; ksr.active[i + 1] = 25; ksr.active[i + 2] = 60; }
    const uint8_t *saved = ksr_platform_saved();
    if (saved) { memcpy(ksr.active, saved + KSR_HEADER_SIZE, ksr_get16(saved + 10)); ksr.mode = saved[16]; ksr.active_generation = ksr_get32(saved + 12); }
    ksr.active_crc = ksr_crc32(ksr.active, ksr.length);
}
static bool same_saved(void) {
    const uint8_t *s = ksr_platform_saved();
    return s && s[4] == KSR_STORAGE_VERSION && ksr_get32(s + 12) == ksr.stage.generation && s[16] == ksr.stage.mode &&
        ksr_get32(s + 20) == ksr.stage.crc && !memcmp(s + KSR_HEADER_SIZE, ksr.stage.data, ksr.length);
}
static enum ksr_status can_save(void) {
    const uint8_t *s = ksr_platform_saved();
    if (same_saved()) return KSR_OK;
    if (s && !ksr_newer(ksr.stage.generation, ksr_get32(s + 12))) return KSR_STALE;
    return KSR_OK;
}
static void status(uint8_t *o) {
    const uint8_t *s = ksr_platform_saved();
    // Legacy RGB remains active in RAM. No automatic flash migration; the next explicit Save persists the extended record.
    bool legacy = s && s[4] != KSR_STORAGE_VERSION;
    ksr_put32(o, ksr.active_generation); ksr_put32(o + 4, ksr.active_crc);
    if (s) { ksr_put32(o + 8, ksr_get32(s + 12)); ksr_put32(o + 12, ksr_get32(s + 20)); o[17] = s[16]; }
    o[16] = ksr.mode; o[18] = ksr.remote_save ? 7 : ksr.job; o[19] = ksr.result;
    ksr_put16(o + 20, ksr.stage.received);
    o[22] = (s ? (legacy ? 8 : 1) : 0) | (ksr.stage.valid ? 2 : 0) | (ksr.stage.open ? 4 : 0);
}
void ksr_handle(const uint8_t *in, uint8_t *out, bool role_authorized, bool peer) {
    uint8_t req[KSR_PACKET]; memcpy(req, in, sizeof(req));
    packet(out, req[4]); out[5] = req[5]; out[6] = KSR_BAD_PACKET;
    if (req[0] != KSR_COMMAND || req[1] != 'K' || req[2] != 'B' || req[3] != KSR_VERSION || !ksr.length) return;
    const uint8_t *p = req + 6; uint8_t *o = out + 7; enum ksr_status rc = KSR_OK;
    uint8_t op = req[4];
    bool mutation = op == KSR_BEGIN || op == KSR_WRITE || op == KSR_VALIDATE || op == KSR_PREVIEW || op == KSR_SAVE || op == KSR_ABORT;
    // LED-only namespace: USB master or trusted split peer, not Vial physical-unlock state.
    // Keep this role boundary; no keymap/macro/security commands are dispatched here.
    if (mutation && !role_authorized) { out[6] = KSR_LOCKED; return; }
    if (mutation && (ksr.job || ksr.remote_save || ksr_reset_busy())) { out[6] = KSR_BUSY; return; }
    switch (op) {
        case KSR_CAPS:
            ksr_put16(o, ksr.layout); o[2] = KSR_LAYERS; o[3] = ksr.leds; ksr_put16(o + 4, ksr.length);
            o[6] = 23; o[7] = 22; o[8] = KSR_CAPABILITIES; // flash + split; lighting does not require physical unlock
            break;
        case KSR_STATUS: status(o); break;
        case KSR_BEGIN:
            if (ksr_get16(p) != ksr.layout || p[2] != KSR_LAYERS || p[3] != ksr.leds || ksr_get16(p + 13) != ksr.length) rc = KSR_BOUNDS;
            else rc = ksr_begin(&ksr.stage, ksr.length, p[4], ksr_get32(p + 5), ksr_get32(p + 9));
            break;
        case KSR_WRITE: rc = ksr_write(&ksr.stage, ksr_get16(p), p + 3, p[2]); break;
        case KSR_VALIDATE: rc = ksr_validate(&ksr.stage);
            if (rc == KSR_OK && !ksr_indicators_valid(ksr.stage.data, ksr.stage.length, ksr.leds)) { ksr.stage.valid = false; rc = KSR_BOUNDS; }
            break;
        case KSR_PREVIEW:
        case KSR_SAVE:
            rc = ksr_validate(&ksr.stage);
            if (rc != KSR_OK) break;
            if (!ksr_indicators_valid(ksr.stage.data, ksr.stage.length, ksr.leds)) { ksr.stage.valid = false; rc = KSR_BOUNDS; break; }
            if (op == KSR_SAVE && (rc = can_save()) != KSR_OK) break;
            if (peer) {
                if (op == KSR_PREVIEW) activate();
                else { ksr.remote_save = true; ksr.result = KSR_PENDING; rc = KSR_PENDING; }
            } else {
                ksr.saving = op == KSR_SAVE; ksr.job = 1; ksr.transfer_offset = 0;
                ksr.result = KSR_PENDING; ksr.job_started = 0; ksr.boot_sync = false;
#ifdef KEEBART_STUDIO_SPLIT_RETRY
                ksr.retry_count = 0; ksr.retry_at = 0;
#endif
                rc = KSR_PENDING;
            }
            break;
        case KSR_READ: {
            uint16_t at = ksr_get16(p + 1); uint8_t n = p[3]; const uint8_t *src = NULL;
            if (p[0] == 0) src = ksr.active;
            else if (p[0] == 1) { src = ksr_platform_saved(); if (src && src[4] == KSR_STORAGE_VERSION) src += KSR_HEADER_SIZE; else src = NULL; }
            else if (p[0] == 2 && ksr.stage.open) src = ksr.stage.data;
            if (!src || !n || n > 22 || at >= ksr.length || n > ksr.length - at) rc = KSR_BOUNDS;
            else { ksr_put16(o, at); o[2] = n; memcpy(o + 3, src + at, n); }
            break;
        }
        case KSR_ABORT: memset(&ksr.stage, 0, sizeof(ksr.stage)); ksr.result = KSR_OK; break;
        case KSR_PEER_STATUS: {
            if (peer) { rc = KSR_UNSUPPORTED; break; }
            uint8_t q[32], a[32]; packet(q, KSR_STATUS);
            if (!ksr_platform_rpc(q, a) || a[6] != KSR_OK) rc = KSR_PEER;
            else memcpy(o, a + 7, 25);
            break;
        }
        default: rc = KSR_UNSUPPORTED; break;
    }
    out[6] = rc;
}
void ksr_boot_sync(uint32_t now) {
    if (ksr.job || ksr.remote_save || ksr.stage.open || ksr_reset_busy()) return;
    ksr_begin(&ksr.stage, ksr.length, ksr.mode, ksr.active_generation, ksr.active_crc);
    memcpy(ksr.stage.data, ksr.active, ksr.length); memset(ksr.stage.seen, 255, sizeof(ksr.stage.seen));
    ksr.stage.received = ksr.length; ksr_validate(&ksr.stage);
    ksr.job = 1; ksr.saving = false; ksr.boot_sync = true; ksr.result = KSR_PENDING; ksr.job_started = now;
#ifdef KEEBART_STUDIO_SPLIT_RETRY
    ksr.retry_count = 0; ksr.retry_at = 0;
#endif
}
static void finish(enum ksr_status rc) { ksr.job = 0; ksr.result = rc; if (ksr.boot_sync) { ksr.stage.open = false; ksr.stage.valid = false; } }
void ksr_task(uint32_t now) {
    if (ksr.remote_save) {
        // Deferred out of the transport ISR/locked callback. No automatic retry on I/O failure.
        ksr.result = same_saved() ? KSR_OK : ksr_platform_commit(&ksr.stage);
        if (ksr.result == KSR_OK) activate();
        ksr.remote_save = false;
        return;
    }
    if (!ksr.job) return;
    if (!ksr.job_started) ksr.job_started = now;
    if (now - ksr.job_started > 15000) { finish(KSR_PEER); return; }
#ifdef KEEBART_STUDIO_SPLIT_RETRY
    if (ksr.retry_count && (int32_t)(now - ksr.retry_at) < 0) return;
#endif
    uint8_t q[32], a[32]; packet(q, KSR_STATUS); uint8_t *p = q + 6;
#ifdef KEEBART_STUDIO_SPLIT_RETRY
    q[5] = ++ksr.rpc_sequence;
#endif
    switch (ksr.job) {
        case 1:
            q[4] = KSR_BEGIN; ksr_put16(p, ksr.layout); p[2] = KSR_LAYERS; p[3] = ksr.leds; p[4] = ksr.stage.mode;
            ksr_put32(p + 5, ksr.stage.generation); ksr_put32(p + 9, ksr.stage.crc); ksr_put16(p + 13, ksr.length); break;
        case 2: {
            q[4] = KSR_WRITE; uint16_t remain = ksr.length - ksr.transfer_offset;
            uint8_t n = remain > 23 ? 23 : remain; ksr_put16(p, ksr.transfer_offset); p[2] = n;
            memcpy(p + 3, ksr.stage.data + ksr.transfer_offset, n); break;
        }
        case 3: q[4] = KSR_VALIDATE; break;
        case 4: q[4] = ksr.saving ? KSR_SAVE : KSR_PREVIEW; break;
        case 5: q[4] = KSR_STATUS; break;
        case 6:
            finish(same_saved() ? KSR_OK : ksr_platform_commit(&ksr.stage));
            if (ksr.result == KSR_OK) activate();
            return;
        default: finish(KSR_BAD_PACKET); return;
    }
#ifdef KEEBART_STUDIO_SPLIT_RETRY
    bool received = ksr_platform_rpc(q, a);
    bool signature = received && !memcmp(a, q, 6);
    if (!signature || (signature && a[6] == KSR_BUSY)) {
        // SAVE may have executed before its reply was lost. Never resend it.
        if (ksr.job == 4 && ksr.saving) { ksr.job = 5; ksr.retry_count = 0; }
        else if (ksr.job == 5) { /* Read-only reconciliation until the deadline. */ }
        else if (ksr.retry_count < 6) {
            uint8_t shift = ksr.retry_count < 3 ? ksr.retry_count : 3;
            ksr.retry_at = now + (24u << shift); ksr.retry_count++;
        } else finish(KSR_PEER);
        return;
    }
    if (a[6] != KSR_OK && a[6] != KSR_PENDING) { finish(KSR_PEER); return; }
    ksr.retry_count = 0;
#else
    if (!ksr_platform_rpc(q, a)) {
        // EXECUTE_RPC can succeed before flash stalls GET_RPC_RESP_DATA. Never
        // resend SAVE: reconcile the exact persisted identity with read-only STATUS.
        if (ksr.job == 4 && ksr.saving) ksr.job = 5;
        else if (ksr.job != 5) finish(KSR_PEER);
        return;
    }
    if (a[0] != KSR_COMMAND || a[1] != 'K' || a[2] != 'B' || a[3] != KSR_VERSION || a[4] != q[4] || (a[6] != KSR_OK && a[6] != KSR_PENDING)) { finish(KSR_PEER); return; }
#endif
    switch (ksr.job) {
        case 1: ksr.job = 2; break;
        case 2: ksr.transfer_offset += p[2]; if (ksr.transfer_offset == ksr.length) ksr.job = 3; break;
        case 3: ksr.job = 4; break;
        case 4:
            if (ksr.saving) ksr.job = 5;
            else { activate(); finish(KSR_OK); }
            break;
        case 5:
            if (a[7 + 18]) break;
            if (a[7 + 19] != KSR_OK || !(a[7 + 22] & 1) || ksr_get32(a + 7 + 8) != ksr.stage.generation ||
                ksr_get32(a + 7 + 12) != ksr.stage.crc || a[7 + 17] != ksr.stage.mode) finish(KSR_PEER);
            else ksr.job = 6;
            break;
    }
}
