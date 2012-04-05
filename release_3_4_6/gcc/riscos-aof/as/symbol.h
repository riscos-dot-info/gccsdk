/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas R�emo
 * Copyright (c) 2001-2006 GCCSDK Developers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * symbol.h
 */

#ifndef symbol_header_included
#define symbol_header_included

#include <stdio.h>

#include "lex.h"
#include "value.h"

#define SYMBOL_LOCAL     0x0001	/* Defined with local scope */
#define SYMBOL_REFERENCE 0x0002
#define SYMBOL_GLOBAL    0x0003	/* Defined with global scope */

#define SYMBOL_KIND(x)   ((x) & 0x0003)

#define SYMBOL_DEFINED   0x0001
#define SYMBOL_EXPORT    0x0002

#define SYMBOL_ABSOLUTE  0x0004	/* This is a constant, (not valid if SYMBOL_REFERENCE) */
#define SYMBOL_NOCASE    0x0008	/* Only if SYMBOL_REFERENCE, case insensitive */
#define SYMBOL_WEAK      0x0010	/* Only if SYMBOL_REFERENCE, must not be resolved */
#define SYMBOL_STRONG    0x0020	/* Complicated ??? */
#define SYMBOL_COMMON    0x0040

#define SYMBOL_DATUM     0x0100	/* ???, for code symbols only */
#define SYMBOL_FPREGARGS 0x0200 /* FP args in FP regs attribute, for code symbols only */
#define SYMBOL_LEAF      0x0800	/* Leaf function, for code symbol only */
#define SYMBOL_THUMB     0x1000	/* Identifies Thumb code, instead of ARM code */

#define SYMBOL_SUPPORTEDBITS 0x1B7F

/* The following are 'as' internal SYMBOL attribute values only and should
   not be used in the AOF output file.  */
#define SYMBOL_KEEP		0x01000000
#define SYMBOL_AREA		0x02000000
#define SYMBOL_NOTRESOLVED	0x04000000
/* Symbol is defined in a based area.  */
#define SYMBOL_BASED		0x08000000

#define SYMBOL_CPUREG		0x10000000
#define SYMBOL_FPUREG		0x20000000
#define SYMBOL_COPREG		0x40000000
#define SYMBOL_COPNUM		0x80000000
#define SYMBOL_GETREG(x)	((x) & 0xF0000000)

#define SYMBOL_TABLESIZE 1024


typedef struct SYMBOL
{
  struct SYMBOL *next;
  unsigned int type;
  int declared;
  Value value;
  union
  {
    struct SYMBOL *ptr;
    struct AREA *info;
  }
  area;
  unsigned int offset;		/* Offset in stringtable */
  int used;		/* this id is used (ie not resolved) */
  /* Later changed to index in symbol table */
  int len;		/* length of str[] */
  char str[1];			/* str[len+1] */
}
Symbol;


Symbol *symbolAdd (Lex l);
Symbol *symbolGet (Lex l);
Symbol *symbolFind (const Lex * l);

int symbolFix (void);		/* Returns number of symbols */
int symbolStringSize (void);
void symbolStringOutput (FILE * outfile);
void symbolSymbolOutput (FILE * outfile);
void symbolSymbolElfOutput (FILE * outfile);

#endif