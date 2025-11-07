# Corne Choc Pro

![corne_choc_pro](https://raw.githubusercontent.com/Keebart/picture-cdn/refs/heads/main/corne/main.webp)

A complete remake of the Corne v4 choc keyboard [crkbd](https://github.com/foostan/crkbd/) with more features and improvements such as USB-C between both halves, improved EMI stability and integrated OLED displays.

This firmware is also 100% compatible with the [Corne MX Pro](https://www.keebart.com/products/corne-mx) keyboard.

- Keyboard Maintainer: [Keebart](https://github.com/Keebart)
- Hardware Supported: RP2040
- Hardware Availability: [Keebart Shop](https://keebart.com/products/corne)

Make example for this keyboard (after setting up your build environment):

    make keebart/corne_choc_pro/standard:default
    make keebart/corne_choc_pro/standard:vial
    make keebart/corne_choc_pro/mini:default
    make keebart/corne_choc_pro/mini:vial_mini

Flashing example for this keyboard:

    make keebart/corne_choc_pro/standard:default:flash
    make keebart/corne_choc_pro/standard:vial:flash
    make keebart/corne_choc_pro/mini:default:flash
    make keebart/corne_choc_pro/mini:vial_mini:flash


"Standard" for the standard keyboard with 6 columns and "Mini" for the shorter keyboard with 5 columns.

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 3 ways:

- **Bootmagic reset**: Hold down the "Q" key of the left side of the keyboard while connecting the left side to the computer. Similarly, hold down the "P" key of the right side of the keyboard while connecting the right side to the computer. (Keys reference the standard QWERTY position)
- **Physical reset button**: Briefly press the button on the back of the PCB
- **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
