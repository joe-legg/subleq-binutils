/* Assembler for subleq
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

#include "as.h"

const char comment_chars[] = "#";
const char line_separator_chars[] = ";";
const char line_comment_chars[] = "#";

/* Floating point */
const char EXP_CHARS[] = "";
const char FLT_CHARS[] = "";

const pseudo_typeS md_pseudo_table[] = { { 0, 0, 0 } };

void
md_operand (expressionS *op ATTRIBUTE_UNUSED)
{
}

/* This function is called once, when the assembler starts.  */

void
md_begin (void)
{
}

/* Assemble an instruction.  */

void
md_assemble (char *str)
{
  printf ("Instruction to assemble: %s\n", str);
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

const char *
md_atof (int type ATTRIBUTE_UNUSED, char *lit ATTRIBUTE_UNUSED,
         int *size ATTRIBUTE_UNUSED)
{
  /* Floating point numbers are not supported.  */
  return NULL;
}

valueT
md_section_align (asection *seg ATTRIBUTE_UNUSED, valueT size ATTRIBUTE_UNUSED)
{
  return 0;
}

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* pc relative relocation.  */
long
md_pcrel_from (fixS *fixp ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_apply_fix (fixS *fixp ATTRIBUTE_UNUSED, valueT *valp ATTRIBUTE_UNUSED,
              segT segment ATTRIBUTE_UNUSED)
{
  /* Empty for now.  */
}

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, asection *sec ATTRIBUTE_UNUSED,
                 fragS *fragp ATTRIBUTE_UNUSED)
{
  as_fatal (_ ("convert frag"));
}

int
md_estimate_size_before_relax (fragS *fragp ATTRIBUTE_UNUSED,
                               asection *seg ATTRIBUTE_UNUSED)
{
  as_fatal (_ ("estimate size"));
}

/* Options */

const char md_shortopts[] = "";
struct option md_longopts[] = {};
size_t md_longopts_size = sizeof (md_longopts);

int
md_parse_option (int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_show_usage (FILE *stream)
{
  fprintf (stream, "\nNo options currently avaliable for subleq.\n");
}
