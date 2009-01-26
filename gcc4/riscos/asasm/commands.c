/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 1997 Darren Salt
 * Copyright (c) 2000-2008 GCCSDK Developers
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
 * commands.c
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#elif HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "area.h"
#include "code.h"
#include "commands.h"
#include "decode.h"
#include "depend.h"
#include "error.h"
#include "expr.h"
#include "filestack.h"
#include "fix.h"
#include "help_lex.h"
#include "include.h"
#include "input.h"
#include "lex.h"
#include "lit.h"
#include "local.h"
#include "macros.h"
#include "main.h"
#include "os.h"
#include "output.h"
#include "put.h"
#include "symbol.h"
#include "value.h"

static void
c_define (const char *msg, Symbol *sym, ValueTag legal)
{
  Value value;

  if (!sym)
    error (ErrorError, FALSE, "Missing label before %s", msg);
  sym->type |= SYMBOL_ABSOLUTE;
  exprBuild ();
  value = exprEval (legal);
  if (value.Tag.t == ValueIllegal)
    {
      error (ErrorError, TRUE, "Illegal %s", msg);
      sym->value.Tag.t = ValueInt;
      sym->value.ValueInt.i = 0;
    }
  else
    sym->value = valueCopy (value);
  sym->type |= SYMBOL_DEFINED | SYMBOL_DECLARED;
  sym->area.ptr = NULL;
}


void
c_equ (Symbol *symbol)
{
  c_define ("equ", symbol, ValueAll);
}


void
c_fn (Symbol *symbol)
{
  c_define ("float register", symbol, ValueInt);
  symbol->type |= SYMBOL_FPUREG;
  int no = symbol->value.ValueInt.i;
  if (no < 0 || no > 7)
    {
      symbol->value.ValueInt.i = 0;
      error (ErrorError, TRUE, "Illegal %s register %d (using 0)", "fpu", no);
    }
}


void
c_rn (Symbol *symbol)
{
  c_define ("register", symbol, ValueInt);
  symbol->type |= SYMBOL_CPUREG;
  int no = symbol->value.ValueInt.i;
  if (no < 0 || no > 15)
    {
      symbol->value.ValueInt.i = 0;
      error (ErrorError, TRUE, "Illegal %s register %d (using 0)", "cpu", no);
    }
}


void
c_cn (Symbol *symbol)
{
  c_define ("coprocessor register", symbol, ValueInt);
  symbol->type |= SYMBOL_COPREG;
  int no = symbol->value.ValueInt.i;
  if (no < 0 || no > 15)
    {
      symbol->value.ValueInt.i = 0;
      error (ErrorError, TRUE, "Illegal %s register %d (using 0)", "cop", no);
    }
}

void
c_cp (Symbol *symbol)
{
  c_define ("coprocessor number", symbol, ValueInt);
  symbol->type |= SYMBOL_COPNUM;
  int no = symbol->value.ValueInt.i;
  if (no < 0 || no > 15)
    {
      symbol->value.ValueInt.i = 0;
      error (ErrorError, TRUE, "Illegal coprocessor number %d (using 0)", no);
    }
}


void
c_ltorg (void)
{
  if (areaCurrentSymbol)
    litOrg (areaCurrentSymbol->area.info->lits);
  else
    areaError ();
}


static void
defineint (int size)
{
  Value value;
  WORD word = 0;
  int c;
  do
    {
      skipblanks ();
      exprBuild ();
      value = exprEval (ValueInt | ValueString | ValueCode | ValueLateLabel | ValueAddr);
      switch (value.Tag.t)
	{
	case ValueInt:
	case ValueAddr:
	  word = fixInt (inputLineNo, size, value.ValueInt.i);
	  putData (size, word);
	  break;
	case ValueString:
	  if (size == 1)
	    {			/* Lay out a string */
	      int len = value.ValueString.len;
	      const char *str = value.ValueString.s;
	      while (len > 0)
		putData (1, lexGetCharFromString (&len, &str));
	    }
	  else
	    putData (size, lexChar2Int (FALSE, value.ValueString.len, value.ValueString.s));
	  break;
	case ValueCode:
	case ValueLateLabel:
	  relocInt (size, &value);
	  putData (size, word);
	  break;
	default:
	  error (ErrorError, TRUE, "Illegal %s expression", "int");
	  break;
	}
      skipblanks ();
    }
  while (((c = inputGet ()) != 0) && c == ',');
  inputUnGet (c);
}


void
c_head (void)
{
  int i;
  Value value;

  i = areaCurrentSymbol ? areaCurrentSymbol->value.ValueInt.i : 0;
  skipblanks ();
  exprBuild ();
  value = exprEval (ValueString);
  switch (value.Tag.t)
    {
    case ValueString:
      if (areaCurrentSymbol)
	{
	  int len = value.ValueString.len;
	  const char *str = value.ValueString.s;
	  while (len > 0)
	    putData (1, lexGetCharFromString (&len, &str));
	  putData (1, 0);
	}
      break;
    default:
      error (ErrorError, TRUE, "Illegal %s expression", "string");
      break;
    }
  if (areaCurrentSymbol)
    {
      while (areaCurrentSymbol->value.ValueInt.i & 3)
	areaCurrentSymbol->area.info->image[areaCurrentSymbol->value.ValueInt.i++] = 0;
      putData (4, 0xFF000000 + (areaCurrentSymbol->value.ValueInt.i - i));
    }
  else
    areaError ();
}


void
c_dcb (void)
{
  defineint (1);
}


void
c_dcw (void)
{
  defineint (2);
}


void
c_dcd (void)
{
  defineint (4);
}

static void
definereal (int size)
{
  Value value;
  int c;
  do
    {
      skipblanks ();
      exprBuild ();
      value = exprEval (ValueInt | ValueFloat | ValueLateLabel | ValueCode);
      switch (value.Tag.t)
	{
	case ValueInt:
	  putDataFloat (size, (FLOAT) value.ValueInt.i);
	  break;
	case ValueFloat:
	  putDataFloat (size, value.ValueFloat.f);
	  break;
	case ValueCode:
	case ValueLateLabel:
	  relocFloat (size, &value);
	  putDataFloat (size, 0.0);
	  break;
	default:
	  error (ErrorError, TRUE, "Illegal %s expression", "float");
	  break;
	}
      skipblanks ();
    }
  while (((c = inputGet ()) != 0) && c == ',');
  inputUnGet (c);
}


void
c_dcfd (void)
{
  definereal (8);
}


void
c_dcfe (void)
{
  error (ErrorError, FALSE, "Not implemented: dcf%c %s", 'e', inputRest ());
}


void
c_dcfs (void)
{
  definereal (4);
}


void
c_dcfp (void)
{
  error (ErrorError, FALSE, "Not implemented: dcf%c %s", 'p', inputRest ());
}


static void
symFlag (unsigned int flags, const char *err)
{
  Symbol *sym;
  Lex lex = lexGetId ();
  if (lex.tag != LexId)
    return;
  sym = symbolGet (&lex);
  if (localTest (sym->str))
    error (ErrorError, TRUE, "Local labels cannot be %s", err);
  else
    sym->type |= flags;
}


void
c_globl (void)
{
  symFlag (SYMBOL_REFERENCE | SYMBOL_DECLARED, "exported");
}


void
c_strong (void)
{
  symFlag (SYMBOL_STRONG, "marked 'strong'");
}


void
c_keep (void)
{
  symFlag (SYMBOL_KEEP | SYMBOL_DECLARED, "marked 'keep'");
}


void
c_import (void)
{
  int c;
  Symbol *sym;

  Lex lex = lexGetId ();
  if (lex.tag != LexId)
    return;

  sym = symbolGet (&lex);
  sym->type |= SYMBOL_REFERENCE | SYMBOL_DECLARED;
  while ((c = inputGet ()) == ',')
    {
      Lex attribute = lexGetId ();
      if (!strncmp ("NOCASE", attribute.LexId.str, attribute.LexId.len))
	sym->type |= SYMBOL_NOCASE;
      else if (!strncmp ("WEAK", attribute.LexId.str, attribute.LexId.len))
	sym->type |= SYMBOL_WEAK;
      else if (!strncmp ("COMMON", attribute.LexId.str, attribute.LexId.len))
        {
	  skipblanks ();
	  if ((c = inputGet ()) != '=')
	    {
	      error (ErrorError, TRUE, "COMMON attribute needs size specification");
	      inputUnGet (c);
	    }
	  else
	    {
	      Value size;
	      exprBuild ();
	      size = exprEval (ValueInt);
	      switch (size.Tag.t)
	        {
	        case ValueInt:
		  sym->value = valueCopy (size);
	          sym->type |= SYMBOL_COMMON;
	          break;
	        default:
	          error (ErrorError, TRUE, "Illegal COMMON attribute expression");
	          break;
	        }
	    }
	}
      else if (!strncmp ("FPREGARGS", attribute.LexId.str, attribute.LexId.len))
	sym->type |= SYMBOL_FPREGARGS;
      else
	error (ErrorError, TRUE, "Illegal IMPORT attribute %s", attribute.LexId.str);
      skipblanks ();
    }
  inputUnGet (c);
}


/**
 * Called for GET / INCLUDE
 */
void
c_get (void)
{
  if (macroSP)
    {
      error (ErrorSerious, TRUE, "Cannot use GET within a macro");
      skiprest ();
      return;
    }
  inputExpand = FALSE;

  char *filename;
  if ((filename = strdup (inputRest ())) == NULL)
    errorOutOfMem ("c_get");
  char *cptr;
  for (cptr = filename; *cptr && !isspace (*cptr); cptr++)
    /* */;
  if (*cptr)
    {
      *cptr++ = '\0';		/* must be a space */
      while (*cptr && isspace (*cptr))
	cptr++;
      if (*cptr && *cptr != ';')
	error (ErrorError, FALSE, "Skipping extra characters '%s' after filename", cptr);
    }
  FILE *getfp;
  char *newInputName;
  if ((getfp = getInclude (filename, &newInputName)) == NULL)
    {
      error (ErrorError, TRUE, "Cannot open file \"%s\"", filename);
      free (filename);
      return;
    }

  free ((void *)inputName);
  inputName = newInputName;
  push_file (asmfile);
  inputLineNo = 0;

#ifdef __riscos__
  dependWrite (filename);
#endif
  asmfile = getfp;
  if (option_verbose)
    fprintf (stderr, "Including file \"%s\" as \"%s\"\n", filename, inputName);
  free (filename);
}


void
c_lnk (void)
{
  if (macroSP)
    {
      error (ErrorSerious, TRUE, "Cannot use LNK within a macro");
      skiprest ();
      return;
    }
  inputExpand = FALSE;

  /* Support LNK inside one IF statement.  */
  if (if_depth)
    if_depth--;
  testUnmatched ();

  char *filename;
  if ((filename = strdup (inputRest ())) == NULL)
    errorOutOfMem ("c_lnk");
#ifdef __riscos__
  dependWrite (filename);
#endif
  char *cptr;
  for (cptr = filename; *cptr && !isspace (*cptr); cptr++)
    /* */;
  *cptr = '\0';
  inputNextLine ();
  char *newInputName;
  FILE *lnkfp = getInclude (filename, &newInputName);
  if (!lnkfp)
    {
      error (ErrorError, TRUE, "Cannot open file \"%s\"", filename);
      free (filename);
      return;
    }
  skiprest ();

  inputFinish (lnkfp);
  inputLineNo = 0;

  free ((void *)inputName);
  inputName = newInputName;

  if (option_verbose)
    fprintf (stderr, "Linking to file \"%s\" as \"%s\"\n", filename, inputName);
  free (filename);
}


void
c_idfn (void)
{
  free ((void *)idfn_text);
  if ((idfn_text = strdup (inputRest ())) == NULL)
    errorOutOfMem("c_idfn");
  skiprest ();
}


void
c_bin (void)
{
  inputExpand = FALSE;
  char *filename;
  if ((filename = strdup (inputRest ())) == NULL)
    errorOutOfMem ("c_bin");
  char *cptr;
  for (cptr = filename; *cptr && !isspace (*cptr); cptr++)
    /* */;
  *cptr = '\0';

  char *newFilename;
  FILE *binfp = getInclude (filename, &newFilename);
  if (!binfp)
    {
      error (ErrorError, TRUE, "Cannot open file \"%s\"", filename);
      free (filename);
      free (newFilename);
      return;
    }
  if (option_verbose)
    fprintf (stderr, "Including binary file \"%s\" as \"%s\"\n", filename, newFilename);
  free (newFilename);
  /* Include binary file.  */
  while (!feof (binfp))
    putData (1, getc (binfp));
  fclose (binfp);
}


void
c_end (void)
{
  if (macroCurrent)
    {
      error (ErrorSerious, TRUE, "Cannot use END within a macro");
      return;
    }
  FILE *fp = pop_file ();
  if (!fp)
    returnvalue = 1;
  else
    {
      fclose (asmfile);
      asmfile = fp;
      if (option_verbose)
	fprintf (stderr, "Returning from include file\n");
    }
}


void
c_assert (void)
{
  Value value;

  exprBuild ();
  value = exprEval (ValueBool);
  if (value.Tag.t != ValueBool)
    error (ErrorError, TRUE, "ASSERT expression must be boolean");
  else if (!value.ValueBool.b)
    error (ErrorError, TRUE, "Assertion failed");
}

void
c_opt (void)
{
  inputRest();
  /* Do nothing.  This is for compatiblity with objasm */
}

void
c_title (void)
{
  inputRest();
  /* Do nothing right now.  This command is for the benefit of error reporting */
}
