#pragma once
#include "studio_rgb.h"
#define KSR_TEST_ENCODERS 12
void ksr_test_encoder(uint8_t index, bool clockwise);
void ksr_test_handle(const uint8_t *in, uint8_t *out, bool authorized, uint8_t count);
