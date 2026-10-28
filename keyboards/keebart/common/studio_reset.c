// SPDX-License-Identifier: GPL-2.0-or-later
// Explicit, physically unlocked reset. Peer first; never replay an erase request.
#include "studio_reset.h"
#include <string.h>
enum { RESET_IDLE, RESET_PROBE, RESET_SEND, RESET_WAIT, RESET_LOCAL, RESET_REMOTE };
static uint8_t phase, result;
static uint32_t nonce, started;
bool ksr_reset_busy(void) { return phase != RESET_IDLE; }
// This phase is reachable only after the USB master authorized this transaction.
bool ksr_reset_erasing(void) { return phase == RESET_LOCAL || phase == RESET_REMOTE; }
static void packet(uint8_t *p, uint8_t action, uint32_t id) {
    memset(p, 0, 32); p[0] = KSR_COMMAND; p[1] = 'K'; p[2] = 'B'; p[3] = KSR_VERSION; p[4] = KSR_FACTORY_RESET;
    p[6] = action; ksr_put32(p + 7, id); memcpy(p + 11, "RST!", 4);
    ksr_put16(p + 15, ksr.layout); p[17] = ksr.leds;
}
static void finish(uint8_t rc) { phase = RESET_IDLE; result = rc; }
void ksr_reset_handle(const uint8_t *in, uint8_t *out, bool authorized, bool peer) {
    uint8_t p[32]; memcpy(p, in, 32); memset(out, 0, 32); memcpy(out, p, 6); out[6] = KSR_BAD_PACKET;
    if (p[0] != KSR_COMMAND || p[1] != 'K' || p[2] != 'B' || p[3] != KSR_VERSION || p[4] != KSR_FACTORY_RESET ||
        memcmp(p + 11, "RST!", 4) || ksr_get16(p + 15) != ksr.layout || p[17] != ksr.leds || !ksr_get32(p + 7)) return;
    for (unsigned i = 18; i < 32; i++) if (p[i]) return;
    if (p[6] == 3) {
        out[6] = KSR_OK; ksr_put32(out + 7, nonce); out[11] = phase; out[12] = result; return;
    }
    if (!authorized) { out[6] = KSR_LOCKED; return; }
    if ((!peer && p[6] != 0) || (peer && p[6] != 1 && p[6] != 2)) { out[6] = KSR_UNSUPPORTED; return; }
    if (ksr.job || ksr.remote_save || ksr.stage.open || ksr_reset_busy()) { out[6] = KSR_BUSY; return; }
    if (p[6] == 1) { out[6] = KSR_OK; return; } // Peer readiness: no writes or queued erases.
    uint32_t id = ksr_get32(p + 7);
    if (id == nonce) { out[6] = KSR_STALE; return; } // A repeated request cannot erase again.
    nonce = id; phase = peer ? RESET_REMOTE : RESET_PROBE; result = KSR_PENDING; started = 0;
    out[6] = KSR_PENDING;
}
void ksr_reset_task(uint32_t now) {
    if (!phase) return;
    if (!started) started = now;
    if (now - started > 15000) { finish(KSR_PEER); return; }
    if (phase == RESET_LOCAL || phase == RESET_REMOTE) {
        finish(ksr_platform_factory_reset()); return;
    }
    uint8_t q[32], a[32];
    if (phase == RESET_PROBE) {
        // Close an idle peer transfer using its existing guarded ABORT operation.
        // The open-stage reset guard remains enforced on both halves.
        memset(q, 0, sizeof(q)); q[0] = KSR_COMMAND; q[1] = 'K'; q[2] = 'B'; q[3] = KSR_VERSION; q[4] = KSR_ABORT;
        if (!ksr_platform_rpc(q, a) || memcmp(q, a, 6) || a[6] != KSR_OK) { finish(KSR_PEER); return; }
    }
    packet(q, phase == RESET_PROBE ? 1 : (phase == RESET_SEND ? 2 : 3), nonce);
    bool received = ksr_platform_rpc(q, a);
    // An EXECUTE may have reached the peer despite a lost reply. Poll only;
    // never automatically send a second destructive command.
    if (phase == RESET_SEND) {
        phase = RESET_WAIT;
        if (!received) return;
    } else if (!received) {
        if (phase == RESET_PROBE) finish(KSR_PEER);
        return;
    }
    if (memcmp(q, a, 6) || (a[6] != KSR_OK && a[6] != KSR_PENDING)) { finish(KSR_PEER); return; }
    if (q[6] == 1) phase = RESET_SEND;
    else if (q[6] == 3) {
        if (ksr_get32(a + 7) != nonce) { finish(KSR_PEER); return; }
        if (a[11]) return;
        if (a[12] != KSR_OK) { finish(KSR_PEER); return; }
        phase = RESET_LOCAL;
    }
}
