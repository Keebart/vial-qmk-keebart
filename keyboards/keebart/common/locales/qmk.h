#pragma once
#include "quantum.h"
bool ksl_qmk_text(void *,uint32_t);
void ksl_qmk_macro_mods(uint8_t);
bool ksl_qmk_code(uint16_t,bool);
bool ksl_qmk_record(uint16_t,keyrecord_t *);
bool ksl_qmk_layer_tap(uint16_t);
void ksl_qmk_task(void);
void ksl_qmk_reset(void);
void ksl_qmk_handle(uint8_t *,bool);
uint16_t ksl_qmk_binding(uint16_t,uint8_t,uint8_t,uint8_t);
uint16_t ksl_qmk_encoder_binding(uint16_t);

bool kar_qmk_record(uint16_t,keyrecord_t *);

bool ksl_qmk_lighting_awake(void);

const uint8_t *ksl_qmk_typing(void);
bool ksl_qmk_auto_shift_key(uint16_t);

const uint8_t *ksl_qmk_persisted_locale(void);
void ksl_qmk_persisted(void);
