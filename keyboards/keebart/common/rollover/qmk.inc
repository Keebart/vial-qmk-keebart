#include "rollover/rollover.h"
static struct ksn_mode rollover;
static void rollover_init(void) {
    static bool started;
    if (!started) { started=true; keymap_config.nkro=false; }
}
static void rollover_task(bool physical_keys_held) {
    if (physical_keys_held || has_anykey() || get_mods() || get_weak_mods() || get_oneshot_mods()) return;
    if (rollover.active == rollover.requested) return;
    /* Explicitly release the previous report even when QMK deduplicates it. */
    if (keymap_config.nkro) { nkro_report->mods=0; host_nkro_send(nkro_report); }
    else { keyboard_report->mods=0; host_keyboard_send(keyboard_report); }
    if (ksn_commit(&rollover,true)) keymap_config.nkro=rollover.active;
}
