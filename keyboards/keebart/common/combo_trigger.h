#pragma once
#include "tap_dance.h"
/* Prefer an explicit Tap Dance trigger. Legacy Vial chords can match its tap
 * action, without replacing the physical keycode used for Tap Dance fallback. */
static inline uint16_t ks_combo_trigger_code(const uint16_t *keys,uint16_t code,uint16_t tap){
    for(unsigned i=0;i<4 && keys[i];i++)if(keys[i]==code)return code;
    return code>=KD_BASE && code<KD_BASE+KD_COUNT && tap>1 ? tap : code;
}
