# Copyright (C) 2021 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

cat << EOF
/* Copyright (C) 2021 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}", "${BIG_OUTPUT_FORMAT}", "${LITTLE_OUTPUT_FORMAT}")
OUTPUT_ARCH("${ARCH}")
SECTIONS
{
  .text :
  {
    ${RELOCATING+ _stext = .;}
    *(.text)
    ${RELOCATING+ _etext = .;}
  }
  .data :
  {
    ${RELOCATING+ _sdata = .;}
    *(.data)
    ${RELOCATING+ _edata = .;}
  }
  .bss :
  {
    ${RELOCATING+ _sbss = .;}
    *(.bss)
    ${RELOCATING+ _ebss = .;}
  }
}
EOF
