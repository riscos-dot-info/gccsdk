/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2000-2011 GCCSDK Developers
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
 * lexAcorn.c
 */

#include "config.h"

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
#endif
#include <string.h>

#include "decode.h"
#include "error.h"
#include "input.h"
#include "lex.h"
#include "lexAcorn.h"
#include "main.h"
#include "targetcpu.h"

#define FINISH_STR(string, Op, Pri)		\
  if (Input_MatchString (string))		\
    {						\
      lex->Data.Operator.op = Op;		\
      lex->Data.Operator.pri = PRI(Pri);	\
      return;					\
    }

#define FINISH_CHR(Op, Pri)			\
  if (Input_Match (':', false))			\
    {						\
      lex->Data.Operator.op = Op;		\
      lex->Data.Operator.pri = PRI(Pri);	\
      return;					\
    }

void
lexAcornUnop (Lex *lex)
{
  lex->tag = LexOperator;
  char c1 = inputGet ();
  switch (c1)
    {
      case 'B':
	FINISH_STR ("ASE:", Op_base, 10); /* :BASE: */
	break;

      case 'C':
	FINISH_STR ("HR:", Op_chr, 10); /* :CHR: */
	break;

      case 'D':
	if (Input_MatchString ("EF:")) /* :DEF: */
	  {
	    *lex = lexGetPrim ();
	    if (lex->tag == LexId)
	      {
	        /* :DEF: only returns {TRUE} when the symbol is defined and it is
		   not a macro local variable.  */
		const Symbol *symP = symbolFind (lex);
		lex->Data.Bool.value = symP != NULL && !(symP->type & SYMBOL_MACRO_LOCAL);
		lex->tag = LexBool;
		return;
	      }
	    else
	      error (ErrorError, "Bad operand for :DEF:");
	  }
	break;

      case 'F':
	{
	  char c2 = inputGet ();
	  switch (c2)
	    {
	      case 'A':
		FINISH_STR ("TTR:", Op_fattr, 10); /* :FATTR: */
		break;
	      case 'E':
		FINISH_STR ("XEC:", Op_fexec, 10); /* :FEXEC: */
		break;
	      case 'L':
		FINISH_STR ("OAD:", Op_fload, 10); /* :FLOAD: */
		break;
	      case 'S':
		FINISH_STR ("IZE:", Op_fsize, 10); /* :FSIZE: */
		break;
	    }
	  inputUnGet (c2);
	  break;
	}
	
      case 'I':
	FINISH_STR ("NDEX:", Op_index, 10); /* :INDEX: */
	break;

      case 'L':
	{
	  char c2 = inputGet ();
	  switch (c2)
	    {
	      case 'E':
		FINISH_STR ("N:", Op_len, 10); /* :LEN: */
		break;
	      case 'N':
		FINISH_STR ("OT:", Op_lnot, 10); /* :LNOT: */
		break;
	    }
	  inputUnGet (c2);
	  break;
	}
	
      case 'N':
	FINISH_STR ("OT:", Op_not, 10); /* :NOT: */
	break;

      case 'S':
	FINISH_STR ("TR:", Op_str, 10); /* :STR: */
	break;
    }
  inputUnGet (c1);

  /* Failed to tokenize.  */
  lex->tag = LexNone;
}


void
lexAcornBinop (Lex *lex)
{
  lex->tag = LexOperator;
  char c1 = inputGet ();
  switch (c1)
    {
      case 'A':
	FINISH_STR ("ND:", Op_and, 8); /* :AND: */
	break;

      case 'C':
	FINISH_STR ("C:", Op_concat, 9); /* :CC: */
	break;
	
      case 'E':
	FINISH_STR ("OR:", Op_xor, 6); /* :EOR: */
	break;
	
      case 'L':
	{
	  char c2 = inputGet ();
	  switch (c2)
	    {
	      case 'A':
		FINISH_STR ("ND:", Op_land, 2); /* :LAND: */
		break;
	      case 'E':
		{
		  char c3 = inputGet ();
		  switch (c3)
		    {
		      case 'F':
			FINISH_STR ("T:", Op_left, 10); /* :LEFT: */
			break;
		      case 'O':
			FINISH_STR ("R:", Op_ne, 1); /* :LEOR: */
			break;
		    }
		  inputUnGet (c3);
		  break;
		}
	      case 'O':
		FINISH_STR ("R:", Op_lor, 1); /* :LOR: */
		break;
	    }
	  inputUnGet (c2);
	  break;
	}
	
      case 'M':
	FINISH_STR ("OD:", Op_mod, 10); /* :MOD: */
	break;

      case 'O':
	FINISH_STR ("R:", Op_or, 7); /* :OR: */
	break;

      case 'R':
	{
	  char c2 = inputGet ();
	  switch (c2)
	    {
	      case 'I':
		FINISH_STR ("GHT:", Op_right, 10); /* :RIGHT: */
		break;
	      case 'O':
		{
		  char c3 = inputGet ();
		  switch (c3)
		    {
		      case 'L':
			FINISH_CHR (Op_rol, 5); /* :ROL: */
			break;
		      case 'R':
			FINISH_CHR (Op_ror, 5); /* :ROR: */
			break;
		    }
		  inputUnGet (c3);
		  break;
		}
	    }
	  inputUnGet (c2);
	  break;
	}
	
      case 'S':
	{
	  char c2 = inputGet ();
	  switch (c2)
	    {
	      case 'H':
		{
		  char c3 = inputGet ();
		  switch (c3)
		    {
		      case 'L':
			FINISH_CHR (Op_sl, 5); /* :SHL: */
			break;
		      case 'R':
			FINISH_CHR (Op_sr, 5); /* :SHR: */
			break;
		    }
		  inputUnGet (c3);
		  break;
		}
	    }
	  inputUnGet (c2);
	  break;
	}
    }
  inputUnGet (c1);

  /* Failed to tokenize.  */
  lex->tag = LexNone;
}


/**
 * Get builtin variable.
 * FIXME: support FPU
 */
void
lexAcornPrim (Lex *lex)
{
  const char * const inputMark = Input_GetMark ();
  switch (inputGet ())
    {
      case 'A':
	if (Input_MatchString ("SASM}")) /* {ASASM} */
	  {
	    lex->tag = LexBool;
	    lex->Data.Int.value = true;
	    return;
	  }
	else if (Input_MatchString ("RCHITECTURE}")) /* {ARCHITECTURE} */
	  {
	    lex->tag = LexString;
	    if ((lex->Data.String.str = strdup (Target_GetArchAsString ())) == NULL)
	      errorOutOfMem ();
	    lex->Data.String.len = strlen (Target_GetArchAsString ());
	    return;
	  }
	break;

      case 'C':
	if (Input_MatchString ("ONFIG}")) /* {CONFIG} */
	  {
	    lex->tag = LexInt;
	    lex->Data.Int.value = (gOptionAPCS & APCS_OPT_32BIT) ? 32 : 26;
	    return;
	  }
	else if (Input_MatchString ("PU}")) /* {CPU} */
	  {
	    lex->tag = LexString;
	    if ((lex->Data.String.str = strdup (Target_GetCPU ())) == NULL)
	      errorOutOfMem ();
	    lex->Data.String.len = strlen (Target_GetCPU ());
	    return;
	  }
	break;

      case 'E':
	if (Input_MatchString ("NDIAN}")) /* {ENDIAN} */
	  {
	    lex->tag = LexString;
	    if ((lex->Data.String.str = strdup ("little")) == NULL)
	      errorOutOfMem ();
	    lex->Data.String.len = sizeof ("little")-1;
	    return;
	  }
	break;

      case 'F':
	if (Input_MatchString ("ALSE}")) /* {FALSE} */
	  {
	    lex->tag = LexBool;
	    lex->Data.Int.value = false;
	    return;
	  }
	break;

      case 'O':
	if (Input_MatchString ("PT}")) /* {OPT} */
	  {
	    lex->tag = LexInt;
	    lex->Data.Int.value = 2;
	    return;
	  }
	break;

      case 'P':
	if (Input_MatchString ("C}")) /* {PC} */
	  {
	    lex->tag = LexPosition;
	    return;
	  }
	break;

      case 'T':
	if (Input_MatchString ("RUE}")) /* {TRUE} */
	  {
	    lex->tag = LexBool;
	    lex->Data.Int.value = true;
	    return;
	  }
	break;

      case 'V':
	if (Input_MatchString ("AR}")) /* {VAR} */
	  {
	    lex->tag = LexStorage;
	    return;
	  }
	break;
    }

  /* Try to find the end of the builtin variable name.  */
  const char *lineRest = Input_Rest ();
  while (*lineRest != '\0' && *lineRest != '\n' && *lineRest != '}')
    ++lineRest;
  if (*lineRest == '}')
    error (ErrorError, "Unknown builtin variable {%.*s",
	   (int)(lineRest + 1 - inputMark), inputMark);
  else
    error (ErrorError, "Missing closing bracket");
  lex->tag = LexNone;
}
