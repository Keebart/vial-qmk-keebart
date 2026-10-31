#pragma once
#include "macro_vm.h"
#define KM_SLOT_SIZE 8192
#define KM_PAGE_SIZE 256
#define KM_LOCALE_OFFSET 4352
#define KM_NAME_BYTES 133
struct km_storage { const uint8_t *(*read)(unsigned); bool (*erase)(unsigned); bool (*program)(unsigned,unsigned,const uint8_t *); };
bool km_store_valid(const uint8_t *);
int km_store_select(const struct km_storage *);
bool km_store_save_named(const struct km_storage *,int,const uint8_t *,const uint8_t *);
bool km_store_save(const struct km_storage *,int,const uint8_t *);
uint32_t km_store_generation(const uint8_t *);

bool km_store_save_full(const struct km_storage *,int,const uint8_t *,const uint8_t *,const uint8_t *);
