#pragma once
#include "quantum.h"
bool kst_key(uint16_t,keyrecord_t *);
void kst_task(void);

void kst_observe(uint16_t,keyrecord_t *);
uint16_t kst_combo_code(const uint16_t *,uint16_t);
