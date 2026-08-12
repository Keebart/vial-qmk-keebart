# Corne MX Pro

![Corne MX Pro](https://raw.githubusercontent.com/Keebart/picture-cdn/refs/heads/main/corne_mx/main.webp)

The Keebart Corne MX Pro is a redesigned Corne v4 MX keyboard based on the
original [crkbd](https://github.com/foostan/crkbd/). It combines MX switches
and MX spacing with integrated RP2040 controllers, USB-C communication
between the halves, per-key RGB lighting, rotary encoder support, and
improved EMI stability.

- Maintainer: [Keebart](https://github.com/Keebart)
- Processor: RP2040
- Switches and spacing: MX switches with MX spacing
- Layouts: Split 3x6+3 Standard and split 3x5+3 Mini
- Split transport: Serial over USB-C
- Lighting: 46-key Standard or 40-key Mini RGB Matrix
- Hardware availability: [Keebart Shop](https://keebart.com/products/corne-mx)

## Variants and keymaps

Use `standard` for the six-column version and `mini` for the five-column
version.

- `default`: Standard QWERTY keymap with encoder support
- `vial`: Vial keymap for the Standard variant
- `vial_mini`: Vial keymap for the Mini variant
- `miryoku`: Miryoku userspace keymap

## Building

Precompiled firmware files can be downloaded from the
[Corne MX Pro product page](https://keebart.com/products/corne-mx).

Set up a [QMK build environment](https://docs.qmk.fm/newbs_getting_started)
and run the command matching the keyboard variant and keymap:

```sh
qmk compile -kb keebart/corne_mx_pro/standard -km default
qmk compile -kb keebart/corne_mx_pro/standard -km vial
qmk compile -kb keebart/corne_mx_pro/standard -km miryoku

qmk compile -kb keebart/corne_mx_pro/mini -km default
qmk compile -kb keebart/corne_mx_pro/mini -km vial_mini
qmk compile -kb keebart/corne_mx_pro/mini -km miryoku
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

## RGB Matrix

The Standard variant has 23 RGB LEDs on each half; the Mini has 20 per half.
RGB Matrix data uses `GP10`. Lighting settings are stored independently in
each half's EEPROM. If either half can be used as the USB master, configure or
reset the lighting state on both halves as required.
