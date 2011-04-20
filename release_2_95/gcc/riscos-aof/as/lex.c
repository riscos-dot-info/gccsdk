/*
 *   lex.c
 * Copyright � 1992 Niklas R�jemo
 */
#include "sdk-config.h"
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#elif HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "main.h"
#include "error.h"
#include "lex.h"
#include "local.h"
#include "help_lex.h"
#include "input.h"
#include "hash.h"
#include "symbol.h"
#include "area.h"
#include "os.h"


const char Pri[2][10] =
{
  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},	/* AS */
  {1, 2, 3, 4, 10, 5, 6, 7, 8, 9}	/* ObjAsm? */
};


static Lex nextbinop;
static BOOL nextbinopvalid = FALSE;

static int 
lexint (int base)
{
  int res = 0;
  char c;

  if (base != 16)
    {
      if ((c = inputLook ()) == '0')
	{
	  if (!objasm)
	    base = 8;
	  if ((c = inputSkipLook ()) == 'x' || c == 'X')
	    {
	      base = 16;
	      inputSkip ();
	    }
	}
      else
	{
	  if (inputLookN (1) == '_')
	    {
	      inputSkipN (2);
	      if ((base = c - '0') < 2 || base > 9)
		error (ErrorError, TRUE, "Illegal base %d", base);
	    }
	}
    }

  for (; isxdigit (c = inputLookLower ()); inputSkip ())
    {
      c -= (c >= 'a') ? 'a' - 10 : '0';
      if (c < base)
	res = res * base + c;
      else
	{
	  return res;
	}
    }
  return res;
}


static FLOAT 
lexfloat (int r)
{
  FLOAT res = r;
  FLOAT frac = 0.1;
  FLOAT exp = 0.0;
  int signexp = 1;
  char c;

  if (inputGet () == '.')
    {				/* Fraction part */
      while (isdigit (c = inputGet ()))
	{
	  res = res + frac * ((FLOAT) c - (FLOAT) '0');
	  frac /= 10.0;
	}
      if (c == 'e' || c == 'E')
	{			/* Exponent part */
	  if (inputLook () == '-')
	    {
	      inputSkip ();
	      signexp = -1;
	    }
	  else if (inputLook () == '+')
	    {
	      inputSkip ();
	    }
	  while (isdigit (c = inputGet ()))
	    exp = exp * 10.0 + ((FLOAT) c - (FLOAT) '0');
	}
      inputUnGet (c);
    }
  else
    error (ErrorError, TRUE, "Internal lexfloat: parse error");
  return res * pow (10.0, signexp * exp);
}


Lex 
lexGetId (void)
{
  char c;
  Lex result;
  nextbinopvalid = FALSE;
  skipblanks ();
  if ((c = inputGet ()) == '|')
    {
      result.tag = LexId;
      result.LexId.str = inputSymbol (&result.LexId.len, '|');
      if (inputGet () != '|')
	error (ErrorError, TRUE, "Identifier continues over newline");
      result.LexId.hash = hashstr (result.LexId.str, result.LexId.len, SYMBOL_TABELSIZE);
    }
  else
    {
      inputUnGet (c);
      if (isalpha (c) || c == '.' || c == '_')
	{
	  result.tag = LexId;
	  result.LexId.str = inputSymbol (&result.LexId.len, 0);
	  result.LexId.hash = hashstr (result.LexId.str, result.LexId.len, SYMBOL_TABELSIZE);
	}
      else
	{
	  error (ErrorError, TRUE, "Missing identifier");
	  result.tag = LexNone;
	}
    }
  if (result.tag == LexId && result.LexId.len > 1)
    localMunge (&result);
  return result;
}


static char *
lexReadLocal (int *len, int *label)
{
  char *name;
  if (!isdigit (inputLook ()))
    {
      error (ErrorSerious, TRUE, "Missing local label number");

      return 0;
    }
  *label = inputGet () - '0';
  if (isdigit (inputLook ()))
    *label = (*label * 10) + (inputGet () - '0');
  name = inputSymbol (len, 0);
  if (len)
    if (strncmp (rout_id, name, *len))
      {
	error (ErrorError, TRUE, "Local label name does not match routine name");
	return 0;
      }
  return name;
}


static Lex 
lexMakeLocal (int dir)
{
  int label, len;
  int i = 0;
  Lex result;
  char *name, id[1024];
  result.tag = LexNone;
  name = lexReadLocal (&len, &label);
  if (!name)
    return result;
  switch (dir)
    {
    case -1:
      if ((i = rout_lblno[label] - 1) < 0)
	{
	  error (ErrorError, FALSE, "Missing local label (bwd) with ID %02i", label);
	  return result;
	}
      break;
    case 0:
      if ((i = rout_lblno[label] - 1) < 0)
        i++;
      break;
    case 1:
      i = rout_lblno[label];
      break;
    }
  sprintf (id, localFormat, (int) areaCurrent, label, i, rout_id);
  result.LexId.str = strdup (id);
  result.LexId.len = strlen (id);
  result.LexId.hash = hashstr (result.LexId.str, result.LexId.len, SYMBOL_TABELSIZE);
  if (!result.LexId.str)
    errorOutOfMem ("lexMakeLocal");
  else
    result.tag = LexId;
  return result;
}


Lex 
lexGetLocal (void)
{
  Lex result;
  result.tag = LexNone;
  nextbinopvalid = FALSE;
  if (isdigit (inputLook ()))
    {
      char *name, id[1024];
      int len, label;
      name = lexReadLocal (&len, &label);
      if (!name)
	return result;
      sprintf (id, localFormat,
	       (int) areaCurrent, label, rout_lblno[label]++, rout_id);
      result.LexId.str = strdup (id);
      result.LexId.len = strlen (id);
      if (!result.LexId.str)
	{
	  errorOutOfMem ("lexGetLocal");
	  return result;
	}
      result.LexId.hash = hashstr (result.LexId.str, result.LexId.len, SYMBOL_TABELSIZE);
      result.tag = LexId;
      return result;
    }
  return lexGetId ();
}





Lex 
lexGetPrim (void)
{
  char c;
  char *str;
  int len;
  Lex result;
  nextbinopvalid = FALSE;
  skipblanks ();
  switch (c = inputGet ())
    {
    case '+':
      result.tag = LexOperator;
      result.LexOperator.op = Op_none;
      result.LexOperator.pri = PRI (10);
      break;
    case '-':
      result.tag = LexOperator;
      result.LexOperator.op = Op_neg;
      result.LexOperator.pri = PRI (10);
      break;
    case '!':
      result.tag = LexOperator;
      result.LexOperator.op = Op_lnot;
      result.LexOperator.pri = PRI (10);
      break;
    case '~':
      result.tag = LexOperator;
      result.LexOperator.op = Op_not;
      result.LexOperator.pri = PRI (10);
      break;
    case '?':
      error (ErrorError, TRUE, "Sorry, '?' not implemented");
    case '(':
    case ')':
      result.tag = LexDelim;
      result.LexDelim.delim = c;
      break;
    case ':':
      lexAcornUnop (&result);
      break;
    case '&':
      result.tag = LexInt;
      result.LexInt.value = lexint (16);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      inputUnGet (c);
      result.tag = LexInt;
      result.LexInt.value = lexint (10);
      if (inputLook () == '.')
	{
	  result.tag = LexFloat;
	  result.LexFloat.value = lexfloat (result.LexInt.value);
	}
      break;
    case '\'':
      result.tag = LexInt;
      str = inputSymbol (&len, '\'');
      if (inputGet () != '\'')
	error (ErrorError, TRUE, "Character continues over newline");
      result.LexInt.value = lexChar2Int (TRUE, len, str);
      break;
    case '"':
      result.tag = LexString;
      str = inputSymbol (&len, '"');
      str = strndup (str, len);
      if (inputGet () != '"')
	error (ErrorError, TRUE, "String continues over newline");
      if (objasm)
	{
	  char *s1, *s2;
	  int l1;
	  while (inputLook () == '"')
	    {			/* cope with "" in strings */
	      inputSkip ();
	      s1 = inputSymbol (&l1, '"');
	      if (inputGet () != '"')
		{
		  error (ErrorError, TRUE, "String continues over newline");
		  break;
		}
	      s2 = malloc (len + l1 + 1);
	      if (!s2)
		{
		  errorOutOfMem ("macroAdd");
		  break;
		}
	      memcpy (s2, str, len);
	      s2[len] = '"';
	      memcpy (s2 + len + 1, s1, l1);
	      free (str);
	      str = s2;
	      len += l1 + 1;
	    }
	  /* now deal with \\ */
	  s1 = s2 = str;
	  while (*s1)
	    {
	      if (*s1 == '\\')
		{
		  switch (l1 = *++s1)
		    {
		    case '0':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
		      l1 = *s1++ - '0';
		      if (*s1 >= '0' && *s1 <= '7')
			{
			  l1 = l1 * 8 + *s1++ - '0';
			  if (*s1 >= '0' && *s1 <= '7')
			    l1 = l1 * 8 + *s1++ - '0';
			}
		      break;
		    case 'x':
		      if (isxdigit (*++s1))
			{	/* implied ASCII-like */
			  l1 = *s1 - '0' - 7 * (*s1 > '9') - 32 * (*s1 >= 'a');
			  if (isxdigit (*++s1))
			    {
			      l1 = l1 * 16 + *s1 - '0' - 7 * (*s1 > '9') - 32 * (*s1 >= 'a');
			      s1++;
			    }
			}
		      break;
		    case 'a':
		      l1 = 7;
		      s1++;
		      break;
		    case 'b':
		      l1 = 8;
		      s1++;
		      break;
		    case 'f':
		      l1 = 12;
		      s1++;
		      break;
		    case 'n':
		      l1 = 10;
		      s1++;
		      break;
		    case 'r':
		      l1 = 13;
		      s1++;
		      break;
		    case 't':
		      l1 = 9;
		      s1++;
		      break;
		    case 'v':
		      l1 = 11;
		      s1++;
		      break;
		    default:
		      s1++;
		    }
		  *s2++ = l1;
		}
	      else
		*s2++ = *s1++;
	    }
	  *s2 = 0;
	  len -= s1 - s2;
	  str = realloc (str, len);	/* try to reduce size of block */
	}
      result.LexString.str = str;
      result.LexString.len = len;
      break;
    case '|':
      result.tag = LexId;
      result.LexId.str = inputSymbol (&result.LexId.len, '|');
      if (inputGet () != '|')
	error (ErrorError, TRUE, "Identifier continues over newline");
      result.LexId.hash = hashstr (result.LexId.str, result.LexId.len, SYMBOL_TABELSIZE);

      break;
    case '.':
      result.tag = LexPosition;
      break;
    case '@':
      result.tag = gcc_backend ? LexNone : LexStorage;
      break;
    case '%':
      {
	int dir;
	switch (inputLookLower ())
	  {
	  case 'f':
	    inputSkip ();
	    dir = 1;
	    break;
	  case 'b':
	    inputSkip ();
	    dir = -1;
	    break;
	  default:
	    dir = 0;
	    break;
	  }
	switch (inputLookLower ())
	  {
	  case 't':
	    inputSkip ();
	    error (ErrorWarning, TRUE, "Ignored 't' in local label name");
	    break;
	  case 'a':
	    inputSkip ();
	    break;
	  default:
	    error (ErrorWarning, TRUE, "Assumed 'a' in local label name");
	    break;
	  }
	return lexMakeLocal (dir);
      }
      break;
    case '{':
      lexAcornPrim (&result);
      break;
    default:
      inputUnGet (c);
      if (isalpha (c) || c == '_')
	{
	  result.tag = LexId;
	  result.LexId.str = inputSymbol (&result.LexId.len, 0);
	  result.LexId.hash = hashstr (result.LexId.str, result.LexId.len, SYMBOL_TABELSIZE);
	}
      else
	result.tag = LexNone;
    }
  if (result.tag == LexId && result.LexId.len > 1)
    localMunge (&result);
  return result;
}

Lex 
lexGetBinop (void)
{
  Lex result;
  int c;
  if (nextbinopvalid)
    {
      nextbinopvalid = FALSE;
      return nextbinop;
    }
  skipblanks ();
  switch (c = inputGet ())
    {
    case '*':
      result.tag = LexOperator;
      result.LexOperator.op = Op_mul;
      result.LexOperator.pri = PRI (10);
      break;
    case '/':
      result.tag = LexOperator;
      result.LexOperator.op = Op_div;
      result.LexOperator.pri = PRI (10);
      break;
    case '%':
      result.tag = LexOperator;
      result.LexOperator.op = Op_mod;
      result.LexOperator.pri = PRI (10);
      break;
    case '+':
      result.tag = LexOperator;
      result.LexOperator.op = Op_add;
      result.LexOperator.pri = PRI (9);
      break;
    case '-':
      result.tag = LexOperator;
      result.LexOperator.op = Op_sub;
      result.LexOperator.pri = PRI (9);
      break;
    case '^':
      result.tag = LexOperator;
      result.LexOperator.op = Op_xor;
      result.LexOperator.pri = PRI (6);
      break;
    case '>':
      result.tag = LexOperator;
      switch (inputLook ())
	{
	case '>':
	  result.LexOperator.pri = PRI (5);
	  if (inputSkipLook () == '>')
	    {
	      inputSkip ();
	      result.LexOperator.op = Op_asr;
	    }
	  else
	    {
	      result.LexOperator.op = Op_sr;
	    }
	  break;
	case '=':
	  inputSkip ();
	  result.LexOperator.op = Op_ge;
	  result.LexOperator.pri = PRI (4);
	  break;
	default:
	  result.LexOperator.op = Op_gt;
	  result.LexOperator.pri = PRI (4);
	}
      break;
    case '<':
      result.tag = LexOperator;
      switch (inputLook ())
	{
	case '<':
	  inputSkip ();
	  result.LexOperator.op = Op_sl;
	  result.LexOperator.pri = PRI (5);
	  break;
	case '=':
	  inputSkip ();
	  result.LexOperator.op = Op_le;
	  result.LexOperator.pri = PRI (4);
	  break;
	case '>':		/* fixed to allow "<>" as well as "!=" */
	  inputSkip ();
	  result.LexOperator.op = Op_ne;
	  result.LexOperator.pri = PRI (3);
	  break;
	default:
	  result.LexOperator.op = Op_lt;
	  result.LexOperator.pri = PRI (4);
	}
      break;
    case '=':
      if (inputLook () == '=')
	inputSkip ();		/* fixed to allow "=" or "==" */
      result.tag = LexOperator;
      result.LexOperator.pri = PRI (3);
      result.LexOperator.op = Op_eq;
      break;
    case '!':
      if (inputLook () == '=')
	{
	  inputSkip ();
	  result.tag = LexOperator;
	  result.LexOperator.pri = PRI (3);
	  result.LexOperator.op = Op_ne;
	}
      else
	result.tag = LexNone;
      break;
    case '|':
      result.tag = LexOperator;
      if (inputLook () == '|')
	{
	  inputSkip ();
	  result.LexOperator.pri = PRI (1);
	  result.LexOperator.op = Op_lor;
	}
      else
	{
	  result.LexOperator.pri = PRI (7);
	  result.LexOperator.op = Op_or;
	}
      break;
    case '&':
      result.tag = LexOperator;
      if (inputLook () == '&')
	{
	  inputSkip ();
	  result.LexOperator.pri = PRI (2);
	  result.LexOperator.op = Op_land;
	}
      else
	{
	  result.LexOperator.pri = PRI (8);
	  result.LexOperator.op = Op_and;
	}
      break;
    case ':':
      lexAcornBinop (&result);
      break;
    default:
      inputUnGet (c);
      result.tag = LexNone;
    }
  return result;
}

int 
lexNextPri ()
{
  if (!nextbinopvalid)
    {
      nextbinop = lexGetBinop ();
      nextbinopvalid = TRUE;
    }
  if (nextbinop.tag == LexOperator)
    return nextbinop.LexOperator.pri;
  else
    return -1;
}


Lex 
lexTempLabel (char *ptr, int len)
{
  Lex var;

  var.tag = LexId;
  var.LexId.str = ptr;
  var.LexId.len = len;
  var.LexId.hash = hashstr (ptr, len, SYMBOL_TABELSIZE);
  return var;
}