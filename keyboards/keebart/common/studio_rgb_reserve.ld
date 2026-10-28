/* SPDX-License-Identifier: GPL-2.0-or-later */
/* A/B at 0x1f8000/0x1fa000; unchanged Vial wear-level store at 0x1fe000. */
ASSERT(__flash_binary_end <= 0x101F8000, "Firmware overlaps Keebart palette storage")
ASSERT(LOADADDR(.data) + SIZEOF(.data) <= 0x101F8000, "RAM load image overlaps Keebart palette storage")
ASSERT(ORIGIN(flash1) == 0x10000000 && LENGTH(flash1) <= 0x200000, "Unexpected RP2040 flash layout")
