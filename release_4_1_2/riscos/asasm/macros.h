/*
 * AS an assembler for ARM
 * Copyright (c) 1997 Darren Salt
 * Copyright (c) 2002-2012 GCCSDK Developers
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
 * macros.h
 */

#ifndef macros_header_included
#define macros_header_included

#include <stdbool.h>
#include "global.h"
#include "lex.h"
#include "variables.h"
#include "whileif.h"

#define MACRO_ARG_LIMIT (18)
/* Characters MACRO_ARG0 up to MACRO_ARG0 + MACRO_ARG_LIMIT may not appear
 * in assembler source code.
 */
#define MACRO_ARG0  14

typedef struct Macro
{
  const struct Macro *next;
  const char *name; /**< Name of this macro.  */
  const char *fileName; /**< Filename where this macro has been defined.  */
  const char *buf;
  bool labelArg; /**< When true, macro supports label as one of its arguments.  */
  unsigned char numArgs; /**< Number of macro arguments (including the label
    one if there is one).  Varies from 0 .. MACRO_ARG_LIMIT - 1.  */
  const char *args[MACRO_ARG_LIMIT];	/**< Current argument values during macro execution.  */
  const char *defArgs[MACRO_ARG_LIMIT];	/**< Default argument value.  */
  unsigned startLineNum; /**< Line number in Macro::fileName where its macro definition starts.  */
} Macro;

typedef struct
{
  const Macro *macro;
  const char *curPtr; /**< Current pointer inside macro buffer Macro::buf.  */
  const char *args[MACRO_ARG_LIMIT];
  VarPos *varListP; /**< Linked list of local variables defined in this macro.  */
  unsigned optDirective; /**< Value {OPT} just before macro invocation.
    {OPT} gets restored after macro invocation.  */
} MacroPObject;

void FS_PopMacroPObject (bool noCheck);

const Macro *Macro_Find (const char *, size_t len);
void Macro_Call (const Macro *, const Lex *);

bool c_macro (void);
bool c_mexit (void);
bool c_mend (void);

#endif