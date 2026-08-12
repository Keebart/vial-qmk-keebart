# Sofle Choc Pro

![Sofle Choc Pro](https://raw.githubusercontent.com/Keebart/picture-cdn/refs/heads/main/sofle/main.webp)

The Keebart Sofle Choc Pro is a compact redesign of the Sofle Choc keyboard
by Josef Adamcik. Each half includes an integrated RP2040 controller,
Choc-spaced keys, a rotary encoder, per-key RGB lighting, and an integrated
OLED display.

- Maintainer: [Keebart](https://github.com/Keebart)
- Processor: RP2040
- Layout: Split 4x6 with five-key thumb clusters
- Split transport: Serial over USB-C
- Lighting: 60-key RGB Matrix, split evenly between the halves
- Controls: One rotary encoder per half
- Displays: Integrated OLED on each half
- Hardware availability: [Keebart Shop](https://keebart.com/products/sofle)

## Supported keymaps

- `default`: Standard QWERTY keymap with encoder support
- `vial`: Runtime keymap, encoder, and RGB configuration through Vial
- `vial_oled`: Vial with OLED functionality

## Building

Precompiled firmware files can be downloaded from the
[Sofle Choc Pro product page](https://keebart.com/products/sofle).

Set up a [QMK build environment](https://docs.qmk.fm/newbs_getting_started)
and run one of the following commands from the QMK firmware directory:

```sh
qmk compile -kb keebart/sofle_choc_pro -km default
qmk compile -kb keebart/sofle_choc_pro -km vial
qmk compile -kb keebart/sofle_choc_pro -km vial_oled
```

The resulting UF2 file is written to the QMK firmware directory.

## Flashing

Each half contains its own RP2040 and must be flashed separately. Build the
desired firmware once and copy the same UF2 file to both halves.

The bootloader can be entered in any of these ways:

- **Bootmagic:** Hold the outer key of the top row while connecting that half
  to USB.
- **BOOT and RESET buttons:** Both buttons are accessible through the two small
  holes on the underside of the keyboard. Hold **BOOT**, briefly press and
  release **RESET**, then release **BOOT**.
- **Keycode:** Use a key mapped to `QK_BOOT`, when available in the active
  keymap.

## OLED and RGB Matrix

OLED support is enabled by the `vial_oled` keymap. The OLEDs show keyboard
status such as the active layer, current key activity, and typing speed.

The keyboard has 30 RGB LEDs on each half, driven from `GP10`. Lighting
settings are stored independently in each half's EEPROM. If either half can
be used as the USB master, configure or reset the lighting state on both
halves as required.
