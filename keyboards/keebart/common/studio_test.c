#include "studio_test.h"
#include <string.h>
static uint16_t turns[4][2];
void ksr_test_encoder(uint8_t index, bool clockwise) {
    if (index < 4) ++turns[index][clockwise ? 0 : 1];
}
void ksr_test_handle(const uint8_t *in, uint8_t *out, bool authorized, uint8_t count) {
    uint8_t request[32];
    memcpy(request, in, sizeof(request));
    memset(out, 0, 32);
    out[0] = KSR_COMMAND; out[1] = 'K'; out[2] = 'B'; out[3] = KSR_VERSION;
    out[4] = KSR_TEST_ENCODERS; out[5] = request[5]; out[6] = KSR_BAD_PACKET;
    if (request[0] != KSR_COMMAND || request[1] != 'K' || request[2] != 'B' ||
        request[3] != KSR_VERSION || request[4] != KSR_TEST_ENCODERS) return;
    for (uint8_t i = 6; i < 32; ++i) if (request[i]) return;
    if (!authorized) { out[6] = KSR_LOCKED; return; }
    if (count > 4) { out[6] = KSR_BOUNDS; return; }
    out[6] = KSR_OK; out[7] = 1; out[8] = count;
    for (uint8_t i = 0; i < count; ++i) {
        ksr_put16(out + 9 + 4 * i, turns[i][0]);
        ksr_put16(out + 11 + 4 * i, turns[i][1]);
    }
}
