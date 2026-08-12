# Corne Choc Pro

![Corne Choc Pro](https://raw.githubusercontent.com/Keebart/picture-cdn/refs/heads/main/corne/main.webp)

The Keebart Corne Choc Pro is a redesigned Corne v4 Choc keyboard based on
the original [crkbd](https://github.com/foostan/crkbd/). It combines
Choc switches on an MX-spaced layout with integrated RP2040 controllers,
USB-C communication
between the halves, per-key RGB lighting, rotary encoder support, and
integrated OLED displays.

The same firmware is compatible with the
[Corne MX Pro](https://keebart.com/products/corne-mx).

- Maintainer: [Keebart](https://github.com/Keebart)
- Processor: RP2040
- Switches and spacing: Choc switches with MX spacing
- Layouts: Split 3x6+3 Standard and split 3x5+3 Mini
- Split transport: Serial over USB-C
- Lighting: RGB Matrix with 23 LEDs per Standard half
- Displays: Integrated OLED on each half
- Hardware availability: [Keebart Shop](https://keebart.com/products/corne)

## Variants and keymaps

Use `standard` for the six-column version and `mini` for the five-column
version.

- `default`: Standard QWERTY keymap
- `vial`: Vial keymap for the Standard variant
- `vial_mini`: Vial keymap for the Mini variant
- `vial_oled` and `vial_mini_oled`: Vial with OLED functionality
- `miryoku`: Miryoku without OLED functionality
- `miryoku_oled`: Miryoku with OLED functionality

## Building

Precompiled firmware files can be downloaded from the
[Corne Choc Pro product page](https://keebart.com/products/corne).

Set up a [QMK build environment](https://docs.qmk.fm/newbs_getting_started)
and run the command matching the keyboard variant and keymap:

```sh
qmk compile -kb keebart/corne_choc_pro/standard -km default
qmk compile -kb keebart/corne_choc_pro/standard -km vial
qmk compile -kb keebart/corne_choc_pro/standard -km vial_oled
qmk compile -kb keebart/corne_choc_pro/standard -km miryoku
qmk compile -kb keebart/corne_choc_pro/standard -km miryoku_oled

qmk compile -kb keebart/corne_choc_pro/mini -km default
qmk compile -kb keebart/corne_choc_pro/mini -km vial_mini
qmk compile -kb keebart/corne_choc_pro/mini -km vial_mini_oled
qmk compile -kb keebart/corne_choc_pro/mini -km miryoku
qmk compile -kb keebart/corne_choc_pro/mini -km miryoku_oled
```

The resulting UF2 file is written to the QMK firmware directory.

## Flashing

Each half contains its own RP2040 and must be flashed separately. Build the
desired firmware once and copy the same UF2 file to both halves.

The bootloader can be entered in any of these ways:

- **Bootmagic:** Hold the outer key of the top row while connecting that half
  to USB. In the standard QWERTY layout, this is **Q** on the left half and
  **P** on the right half.
- **BOOT and RESET buttons:** Both buttons are accessible through the two small
  holes on the underside of the keyboard. Hold **BOOT**, briefly press and
  release **RESET**, then release **BOOT**.
- **Keycode:** Use a key mapped to `QK_BOOT`, when available in the active
  keymap.

## OLED and RGB Matrix

OLED support is enabled only by keymaps whose names end in `_oled`. The OLEDs
show keyboard status such as the active layer, current key activity, and
typing speed.

RGB Matrix data uses `GP10`. Lighting settings are stored independently in
each half's EEPROM. If either half can be used as the USB master, configure or
reset the lighting state on both halves as required.
