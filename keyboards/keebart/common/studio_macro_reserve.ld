/* Studio-only macro banks occupy 0x1f4000..0x1f8000. */
ASSERT(__flash_binary_end <= 0x101F4000, "Firmware overlaps Keebart macro storage")
ASSERT(LOADADDR(.data) + SIZEOF(.data) <= 0x101F4000, "RAM load image overlaps Keebart macro storage")
