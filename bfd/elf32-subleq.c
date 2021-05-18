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

static reloc_howto_type *
subleq_reloc_type_lookup (bfd *abdf ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code ATTRIBUTE_UNUSED)
{
    return NULL;
}

static reloc_howto_type *
subleq_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *name ATTRIBUTE_UNUSED)
{
    return NULL;
}

#define ELF_ARCH bfd_arch_subleq
#define ELF_MACHINE_CODE EM_SUBLEQ
#define ELF_MAXPAGESIZE 1

#define TARGET_LITTLE_SYM subleq_elf32_vec
#define TARGET_LITTLE_NAME "elf32-subleq"

#define bfd_elf32_bfd_reloc_type_lookup subleq_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup subleq_reloc_name_lookup

#include "elf32-target.h"
