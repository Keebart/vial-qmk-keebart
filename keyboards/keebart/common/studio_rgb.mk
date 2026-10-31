# SPDX-License-Identifier: GPL-2.0-or-later
KEEBART_STUDIO_RGB_ENABLE ?= yes
ifeq ($(strip $(KEEBART_STUDIO_RGB_ENABLE)),yes)
    OPT_DEFS += -DKEEBART_STUDIO_RGB_ENABLE -DCRT0_EXTRA_CORES_NUMBER=0
    COMMON_VPATH += keyboards/keebart/common
    ifeq ($(strip $(KEYMAP)),studio)
        SRC += keyboards/keebart/common/tap_dance.c keyboards/keebart/common/studio_dance.c
        SRC += keyboards/keebart/common/device_name.c keyboards/keebart/common/studio_name.c
        OPT_DEFS += -DKEEBART_STUDIO_LOCALES
        SRC += keyboards/keebart/common/locales/alt_repeat.c keyboards/keebart/common/locales/locale.c keyboards/keebart/common/locales/locale_runtime.c keyboards/keebart/common/locales/locale_protocol.c keyboards/keebart/common/studio_locale.c
        OPT_DEFS += -DKEEBART_STUDIO_DEVICE_NAME
        OPT_DEFS += -DKEEBART_STUDIO_TAP_DANCE -DKEEBART_STUDIO_MACROS -DKEEBART_STUDIO_SPLIT_RETRY
        SRC += keyboards/keebart/common/macro_restore.c keyboards/keebart/common/macro_vm.c keyboards/keebart/common/macro_store.c keyboards/keebart/common/studio_macro.c
        LDFLAGS += -Wl,-T,keyboards/keebart/common/studio_macro_reserve.ld
    endif
    SRC += keyboards/keebart/common/studio_rgb_core.c keyboards/keebart/common/studio_rgb.c keyboards/keebart/common/studio_rgb_qmk.c keyboards/keebart/common/studio_rgb_render.c keyboards/keebart/common/studio_reset.c
    LDFLAGS += -Wl,-T,keyboards/keebart/common/studio_rgb_reserve.ld
    ifneq ($(findstring keebart/3w6_pro,$(KEYBOARD)),)
        OPT_DEFS += -DKSR_LAYOUT_ID=0x0006 -DKSR_EXPECTED_LEDS=36 -DKSR_EXPECTED_ROWS=8 -DKSR_EXPECTED_COLS=5 -DKSR_EXPECTED_ENCODERS=0 -DKSR_EXPECTED_COMBOS=64 -DKSR_EXPECTED_MACRO_BASE=2833
    else ifneq ($(findstring keebart/sofle_choc_pro,$(KEYBOARD)),)
        OPT_DEFS += -DKSR_LAYOUT_ID=0x0005 -DKSR_EXPECTED_LEDS=60 -DKSR_EXPECTED_ROWS=10 -DKSR_EXPECTED_COLS=6 -DKSR_EXPECTED_ENCODERS=2 -DKSR_EXPECTED_COMBOS=32 -DKSR_EXPECTED_MACRO_BASE=3281
    else
        OPT_DEFS += -DKSR_EXPECTED_ROWS=8 -DKSR_EXPECTED_COLS=7 -DKSR_EXPECTED_ENCODERS=4 -DKSR_EXPECTED_COMBOS=32
        ifeq ($(strip $(ENCODER_MAP_ENABLE)),yes)
            OPT_DEFS += -DKSR_EXPECTED_MACRO_BASE=3281
        else
            OPT_DEFS += -DKSR_EXPECTED_MACRO_BASE=3025
        endif
        ifneq ($(findstring keebart/corne_mx_pro,$(KEYBOARD)),)
            OPT_DEFS += -DKSR_LAYOUT_ID=0x0204 -DKSR_EXPECTED_LEDS=46
        else ifneq ($(findstring /mini,$(KEYBOARD)),)
            OPT_DEFS += -DKSR_LAYOUT_ID=0x0104 -DKSR_EXPECTED_LEDS=40
        else
            OPT_DEFS += -DKSR_LAYOUT_ID=0x0004 -DKSR_EXPECTED_LEDS=46
        endif
    endif
endif
