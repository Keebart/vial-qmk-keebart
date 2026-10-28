#pragma once
#include <stdbool.h>
#include <stdint.h>
void ksm_read(uint16_t,uint16_t,uint8_t *);
void ksm_write(uint16_t,uint16_t,const uint8_t *);
bool ksm_reset(void);
void ksm_start(uint8_t);
void ksm_task(void);
bool ksm_key(uint16_t,bool);

void ksm_abort(void);

const uint8_t *ksm_name_record(void);
bool ksm_save_name(const uint8_t *);
