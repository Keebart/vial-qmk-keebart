VIA_ENABLE = yes
VIAL_ENABLE = yes
VIALRGB_ENABLE = yes
ENCODER_MAP_ENABLE = yes
CAPS_WORD_ENABLE = yes
REPEAT_KEY_ENABLE = yes

ifneq ($(findstring /mini,$(KEYBOARD)),)
    $(error The studio keymap targets the standard board; use a matching Mini keymap)
endif
include keyboards/keebart/common/studio_rgb.mk
include keyboards/keebart/common/studio_test.mk
