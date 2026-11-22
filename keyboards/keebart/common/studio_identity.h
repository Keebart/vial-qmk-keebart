// Hardware identity is independent of names, settings and the active USB half.
#pragma once
#include "hardware_id.h"
#include <stdbool.h>
#include <string.h>
#if defined(QMK_MCU_RP2040) || defined(RP2040)
#include "hardware/sync.h"
void flash_get_unique_id(uint8_t *);
hardware_id_t get_hardware_id(void) {
    static bool cached;
    static hardware_id_t id;
    uint32_t irq = save_and_disable_interrupts();
    if (!cached) {
        flash_get_unique_id((uint8_t *)&id);
        cached = true;
    }
    restore_interrupts(irq);
    return id;
}
#endif
static uint8_t ksid_hardware[8];
static void ksid_init(void) {
    hardware_id_t id = get_hardware_id();
    memcpy(ksid_hardware, &id, sizeof(ksid_hardware));
}
static void ksid_reply(uint8_t *reply) {
    memcpy(reply + 19, ksid_hardware, sizeof(ksid_hardware));
    memcpy(reply + 27, "KID1", 4);
}
