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
#include "opcode/subleq.h"

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

static int
emit_subleq_operand (expressionS *expr, char c)
{
  char *frag = frag_more (SUBLEQ_OPERAND_SIZE);

  if (expr->X_op == O_symbol)
    {
      fix_new_exp (frag_now, frag - frag_now->fr_literal, 4, expr, 0,
                   BFD_RELOC_32);
    }
  else if (expr->X_op != O_constant)
    {
      as_bad ("subleq argument '%c' must be a symbol or constant", c);
      return -1;
    }

  md_number_to_chars (frag, expr->X_add_number, SUBLEQ_OPERAND_SIZE);

  return 0; /* Return 0 on success.  */
}

/* Assemble an instruction.  */

void
md_assemble (char *str)
{
  if (startswith (str, "subleq"))
    {
      input_line_pointer = str + 6;

      expressionS a, b, c;

      expression (&a);
      expression (&b);
      expression (&c);

      /* These return statements aren't completely necessary but I have
         included them anyway.  */
      if (emit_subleq_operand (&a, 'a')) return;
      if (emit_subleq_operand (&b, 'b')) return;
      if (emit_subleq_operand (&c, 'c')) return;
    }
  else
    {
      as_bad ("unknown instruction %s", str);
    }
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
md_section_align (asection *seg ATTRIBUTE_UNUSED, valueT size)
{
  return size;
}

/* Turn fixups that cannot be applied into relocations.  */
arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc = XNEW (arelent);
  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);

  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  reloc->addend = fixp->fx_offset;

  return reloc;
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
