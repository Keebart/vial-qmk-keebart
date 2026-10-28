# Included only by the dedicated Studio keymaps, after studio_rgb.mk.
ifeq ($(strip $(KEEBART_STUDIO_RGB_ENABLE)),yes)
    OPT_DEFS += -DKEEBART_STUDIO_TEST_ENABLE
    SRC += keyboards/keebart/common/studio_test.c
endif
