#pragma once
#include "quantum.h"
bool ksl_qmk_code(uint16_t,bool);
bool ksl_qmk_record(uint16_t,keyrecord_t *);
void ksl_qmk_task(void);
void ksl_qmk_reset(void);
void ksl_qmk_handle(uint8_t *,bool);
uint16_t ksl_qmk_binding(uint16_t,uint8_t,uint8_t,uint8_t);
uint16_t ksl_qmk_encoder_binding(uint16_t);

bool kar_qmk_record(uint16_t,keyrecord_t *);
