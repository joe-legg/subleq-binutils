/* Header file for tc-subleq.c
   Copyright (C) 2021 Free Software Foundation, Inc.
   Contributed by Joe Legg.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#ifndef TC_SUBLEQ_H
#define TC_SUBLEQ_H

#define TC_SUBLEQ 1

#define TARGET_FORMAT \
  (target_big_endian ? "elf32-bigsubleq" : "elf32-littlesubleq")
#define TARGET_ARCH bfd_arch_subleq

/* Default to big endian */
#define TARGET_BYTES_BIG_ENDIAN 1

#define WORKING_DOT_WORD

#endif /* TC_SUBLEQ_H */
