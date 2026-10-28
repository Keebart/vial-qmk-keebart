VIA_ENABLE = yes
VIAL_ENABLE = yes
VIALRGB_ENABLE = yes
ENCODER_MAP_ENABLE = yes
CAPS_WORD_ENABLE = yes
REPEAT_KEY_ENABLE = yes

include keyboards/keebart/common/studio_rgb.mk

# No I2C activity on halves using the OLED pins for encoders.
OLED_ENABLE = no
WPM_ENABLE = yes
