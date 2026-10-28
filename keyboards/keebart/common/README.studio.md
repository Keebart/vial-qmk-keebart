# Keebart Studio firmware

Each supported keyboard has a `keymaps/studio` with its starting keymap,
configuration and Studio integration. The complete sources are in this
repository, including shared firmware in `keyboards/keebart/common/` and the
Studio hooks in QMK/Vial. No private repository or Studio app source is needed
to compile firmware.

## Build

Install the QMK build environment and clone this repository with its submodules
(`git clone --recurse-submodules`). From the repository root:

```sh
qmk compile -kb keebart/corne_choc_pro/standard -km studio
qmk compile -kb keebart/corne_mx_pro/standard -km studio
qmk compile -kb keebart/sofle_choc_pro -km studio
qmk compile -kb keebart/3w6_pro -km studio
```

The matching `.uf2` images are generated in the repository root and `.build/`.
Install matching Studio firmware on both halves of a split keyboard.

## Customize

Edit the keyboard's `keymaps/studio/config.h`, `rules.mk` or `keymap.c`, then
rebuild. These files control compile-time options and factory keymap bindings.
Shared Studio implementations are in `keyboards/keebart/common/`.
The existing `default`, `vial` and other keymaps remain separate build targets.

Existing saved settings take precedence over factory defaults. Keebart Studio
can edit and save keymaps, lighting, encoders, macros, combos, Tap Dance and key
overrides at runtime. Saved keyboard names also become the USB device name.
The Studio stock-reset flow restores the factory configuration and name.
