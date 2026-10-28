// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "studio_rgb.h"
#define KSR_FACTORY_RESET 11
bool ksr_reset_busy(void);
bool ksr_reset_erasing(void);
void ksr_reset_handle(const uint8_t *in, uint8_t *out, bool authorized, bool peer);
void ksr_reset_task(uint32_t now);
enum ksr_status ksr_platform_factory_reset(void);
