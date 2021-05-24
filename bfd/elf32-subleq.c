/* 32-bit ELF support for SUBLEQ.
   Copyright (C) 2021 Free Software Foundation, Inc.
   Contributed by Joe Legg.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/subleq.h"

static reloc_howto_type subleq_elf_howto_table[] =
{
  /* Do nothing.  */
  HOWTO (R_SUBLEQ_NONE,          /* type */
         0,                      /* rightshift */
         3,                      /* size (0 = byte, 1 = short, 2 = long) */
         0,                      /* bitsize */
         false,                  /* pc_relative */
         0,                      /* bitpos */
         complain_overflow_dont, /* complain_on_overflow */
         bfd_elf_generic_reloc,  /* special_function */
         "R_SUBLEQ_NONE",        /* name */
         false,                  /* partial_inplace */
         0,                      /* src_mask */
         0,                      /* dst_mask */
         false),                 /* pcrel_offset */

  HOWTO (R_SUBLEQ_32,            /* type */
         2,                      /* rightshift by 2 to divide the address by 4 */
         2,                      /* size (0 = byte, 1 = short, 2 = long) */
         32,                     /* bitsize */
         false,                  /* pc_relative */
         0,                      /* bitpos */
         complain_overflow_bitfield, /* complain_on_overflow */
         bfd_elf_generic_reloc,  /* special_function */
         "R_SUBLEQ_32",          /* name */
         false,                  /* partial_inplace */
         0x00000000,             /* src_mask */
         0xffffffff,             /* dst_mask */
         false),                 /* pcrel_offset */
};

struct subleq_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int elf_reloc_type;
};

static const struct subleq_reloc_map subleq_reloc_map [] =
{
  { BFD_RELOC_NONE, R_SUBLEQ_NONE },
  { BFD_RELOC_32,   R_SUBLEQ_32   }
};

static reloc_howto_type *
subleq_reloc_type_lookup (bfd *abdf ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code)
{
  unsigned int i;
  for (i = 0; i < sizeof (subleq_reloc_map) / sizeof (subleq_reloc_map[0]);
       i++)
    if (subleq_reloc_map[i].bfd_reloc_val == code)
      return &subleq_elf_howto_table[subleq_reloc_map[i].elf_reloc_type];

  return NULL;
}

static reloc_howto_type *
subleq_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *name)
{
  unsigned int i;
  for (i = 0;
       i < sizeof (subleq_elf_howto_table) / sizeof (subleq_elf_howto_table[0]);
       i++)
      if (subleq_elf_howto_table[i].name != NULL && strcasecmp (subleq_elf_howto_table[i].name, name) == 0)
        return &subleq_elf_howto_table[i];

  return NULL;
}
static bool
subleq_info_to_howto_rela (bfd *abfd,
                           arelent *cache_ptr,
                           Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_SUBLEQ_max)
    {
      _bfd_error_handler (_("%p: unsupported relocation type %#x"), abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = &subleq_elf_howto_table[r_type];
  return true;
}

#define ELF_ARCH bfd_arch_subleq
#define ELF_MACHINE_CODE EM_SUBLEQ
#define ELF_MAXPAGESIZE 1

#define TARGET_BIG_SYM subleq_elf32_be_vec
#define TARGET_BIG_NAME "elf32-bigsubleq"
#define TARGET_LITTLE_SYM subleq_elf32_le_vec
#define TARGET_LITTLE_NAME "elf32-littlesubleq"

#define elf_info_to_howto subleq_info_to_howto_rela
#define elf_info_to_howto_rel NULL

#define bfd_elf32_bfd_reloc_type_lookup subleq_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup subleq_reloc_name_lookup

#include "elf32-target.h"
