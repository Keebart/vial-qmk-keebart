#define VIAL_KEYBOARD_UID {0x89, 0x36, 0x2A, 0xC7, 0xFA, 0xD8, 0x89, 0x45}

#define VIAL_UNLOCK_COMBO_ROWS {0, 0}
#define VIAL_UNLOCK_COMBO_COLS {0, 1}

#define OLED_IC OLED_IC_SSD1312
#define OLED_DISPLAY_128X64
#define OLED_FLIP_SEGMENT
#define OLED_DISPLAY_ADDRESS 0x3C
#define OLED_CHARGE_PUMP_VALUE 0x72
#define OLED_BRIGHTNESS 64
#define OLED_UPDATE_INTERVAL 50
#define OLED_TIMEOUT 0
#define OLED_FADE_OUT

#include "../../../common/oled_split_config.h"

#ifdef KEEBART_STUDIO_RGB_ENABLE
// Encoder mappings move the following Vial EEPROM regions. Invalidate the
// previous OLED schema once instead of interpreting its settings as encoders.
#    undef VIA_EEPROM_BUILD_ID
#    define VIA_EEPROM_BUILD_ID 0x4B0102u
#endif
