/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2000-2012 GCCSDK Developers
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
 * symbol.c
 */

#include "config.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif
#if HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#include "aoffile.h"
#include "area.h"
#include "code.h"
#include "elf.h"
#include "error.h"
#include "expr.h"
#include "filestack.h"
#include "get.h"
#include "global.h"
#include "input.h"
#include "local.h"
#include "main.h"
#include "output.h"
#include "symbol.h"

#ifdef DEBUG
#  define DEBUG_SYMBOL
#endif

static Symbol *symbolTable[SYMBOL_TABLESIZE];
static bool oKeepAllSymbols;
static bool oExportAllSymbols;
static bool oAllExportSymbolsAreWeak;

static Symbol *
Symbol_New (const char *str, size_t len)
{
  Symbol *result;
  if ((result = (Symbol *) malloc (sizeof (Symbol) + len)) == NULL)
    errorOutOfMem ();
  result->next = NULL;
  result->type = result->offset = 0;
  result->value.Tag = ValueIllegal;
  result->codeSize = 0;
  result->area = NULL;
  result->fileName = FS_GetCurFileName ();
  result->lineNumber = FS_GetCurLineNumber ();
  result->aligned = 0;
  result->visibility = 0;
  result->used = -1;
  result->len = len;
  memcpy (result->str, str, len);
  result->str[len] = 0;
  return result;
}


/**
 * Unlink symbol from its lists and free its data.
 */
static void
Symbol_Free (Symbol **symPP)
{
  Symbol *symP = *symPP;
  *symPP = symP->next;
  valueFree (&symP->value);
  free (symP);
}


static bool
EqSymLex (const Symbol *str, const Lex *lx)
{
  if (str->len != lx->Data.Id.len)
    return false;
  return !memcmp (str->str, lx->Data.Id.str, str->len);
}


/**
 * Retrieve an existing symbol or create a new one if there wasn't one.
 * \return Always a non-NULL value pointing to symbol representing given Lex
 * object.
 */
Symbol *
Symbol_Get (const Lex *l)
{
  assert (l->tag == LexId && "non-ID");

  Symbol **isearch;
  for (isearch = &symbolTable[l->Data.Id.hash]; *isearch; isearch = &(*isearch)->next)
    {
      if (EqSymLex (*isearch, l))
	return *isearch;
    }

  *isearch = Symbol_New (l->Data.Id.str, l->Data.Id.len);
  return *isearch;
}


/**
 * Retrieve an existing symbol.
 * \return NULL when symbol doesn't exist, pointer to that symbol otherwise.
 */
Symbol *
Symbol_Find (const Lex *l)
{
  if (l->tag != LexId)
    return NULL;

  Symbol **isearch;
  for (isearch = &symbolTable[l->Data.Id.hash]; *isearch; isearch = &(*isearch)->next)
    {
      if (EqSymLex (*isearch, l))
	return *isearch;
    }
  return NULL;
}


/**
 * Defines given symbol with given value and type.  Only to be used for symbols
 * which can not be (or not supposed to be) redefined as this checks on
 * redefinition with different value or inconsistencies like area vs area label
 * definitions.
 * \return false if successful, true otherwise.
 */
bool
Symbol_Define (Symbol *symbol, unsigned newSymbolType, const Value *newValue)
{
  if (symbol->type & SYMBOL_AREA)
    {
      error (ErrorError, "Area label %s can not be redefined", symbol->str);
      return true;
    }
  assert ((newSymbolType & SYMBOL_AREA) == 0 && "Not to be used for defining area symbols");

  Value newValueCopy = { .Tag = ValueIllegal };
  if (symbol->type & SYMBOL_DEFINED)
    {
      /* When a symbol is already defined, we can only overrule its value when
         that value was ValueIllegal, or when the value is exactly the same.  */
      if (symbol->value.Tag != ValueIllegal)
	{
	  bool diffValue;
	  if (valueEqual (&symbol->value, newValue))
	    diffValue = false;
	  else
	    {
	      if (symbol->value.Tag == ValueSymbol || newValue->Tag == ValueSymbol)
		{
		  /* newValue might point into our code array.  */
		  Value_Assign (&newValueCopy, newValue);
		  newValue = &newValueCopy;

		  codeInit ();
		  codeValue (&symbol->value, false);
		  Value val1 = { .Tag = ValueIllegal };
		  Value_Assign (&val1, exprEval (ValueAll));
		  codeInit ();
		  codeValue (newValue, false);
		  Value val2 = { .Tag = ValueIllegal };
		  Value_Assign (&val2, exprEval (ValueAll));
		  diffValue = !valueEqual (&val1, &val2);
#ifdef DEBUG
		  if (diffValue)
		    {
		      printf ("Diff: ");
		      valuePrint (&val1);
		      printf (" vs ");
		      valuePrint (&val2);
		      printf ("\n");
		    }
#endif
		  valueFree (&val1);
		  valueFree (&val2);
		}
	      else
		{
		  diffValue = true; /* Not sure if we don't have to try harder here.  */
#ifdef DEBUG
		  printf ("Diff: ");
		  valuePrint (&symbol->value);
		  printf (" vs ");
		  valuePrint (newValue);
		  printf ("\n");
#endif
		}
	    }
	  if (diffValue)
	    {
	      error (ErrorError, "Symbol %s can not be redefined with a different value", symbol->str);
	      return true;
	    }
	}
    }
  symbol->type |= newSymbolType | SYMBOL_DEFINED;
  Value_Assign (&symbol->value, newValue);

  if (newValue == &newValueCopy)
    valueFree (&newValueCopy);
  return false;
}


/**
 * Removes an existing symbol from symbol table.
 */
void
Symbol_Remove (const Lex *l)
{
  assert (l->tag == LexId && "non-ID");

  for (Symbol **isearch = &symbolTable[l->Data.Id.hash];
       *isearch != NULL;
       isearch = &(*isearch)->next)
    {
      if (EqSymLex (*isearch, l))
	{
          Symbol_Free (isearch);
	  return;
	}
    }

  assert (!"Not an existing symbol");
}


void
Symbol_RemoveVariables (void)
{
  for (int i = 0; i != SYMBOL_TABLESIZE; i++)
    {
      Symbol **symPP = &symbolTable[i];
      while (*symPP)
	{
	  if ((*symPP)->type & SYMBOL_RW)
	    {
	      assert ((*symPP)->value.Tag == ValueBool || (*symPP)->value.Tag == ValueString || (*symPP)->value.Tag == ValueInt);
	      Symbol_Free (symPP);
	    }
	  else
	    symPP = &(*symPP)->next;
	}
    }
}


static bool
NeedToOutputSymbol (const Symbol *sym)
{
  /* All area symbols are exported except the implicit one.  */
  if (sym->type & SYMBOL_AREA)
    return !Area_IsImplicit (sym);

  /* All mapping symbols are exported as well.  */
  if (Area_IsMappingSymbol (sym->str))
    return true;

  /* Give a warning for a symbol which got explicitly marked as 'KEEP'
     but as a valuetype which does not allow us to 'keep' it.  */
  if ((sym->type & SYMBOL_KEEP)
      && !((sym->value.Tag == ValueInt && sym->value.Data.Int.type == eIntType_PureInt)
		        || sym->value.Tag == ValueSymbol))
    {
      if (sym->value.Tag == ValueIllegal)
	errorLine (NULL, 0, ErrorError, "Symbol %s is marked with KEEP but has not been defined", sym->str);
      else
	errorLine (NULL, 0, ErrorWarning, "Symbol %s is marked with KEEP but has unsuitable value for export", sym->str);
    }

  bool doOutput = ((SYMBOL_KIND(sym->type) == SYMBOL_GLOBAL
		    || (SYMBOL_KIND(sym->type) == SYMBOL_LOCAL && (oKeepAllSymbols || (sym->type & SYMBOL_KEEP) != 0 || oExportAllSymbols)))
                   || (SYMBOL_KIND(sym->type) == SYMBOL_REFERENCE && sym->used == 0))
		  && (sym->type & (SYMBOL_RW | SYMBOL_NO_EXPORT)) == 0;
  return doOutput;
}


static int
SymbolCompare (const void *sym1PP, const void *sym2PP)
{
  const Symbol *sym1P = *(const Symbol **)sym1PP;
  const Symbol *sym2P = *(const Symbol **)sym2PP;

  /* Not sure how the mapping symbols for AOF output needs to be sorted.  */
  bool isMappingSym1 = Area_IsMappingSymbol (sym1P->str) != eInvalid;
  bool isMappingSym2 = Area_IsMappingSymbol (sym2P->str) != eInvalid;
  if (!option_aof || (!isMappingSym1 && !isMappingSym2)) 
    return strcasecmp (sym1P->str, sym2P->str);

  /* AOF: mapping symbols at the end of symbol table.  */
  if (isMappingSym1 != isMappingSym2)
    return isMappingSym1 ? 1 : -1;
  
  /* AOF: mapping symbols first sorted according to their area name, then
     by their offset in the same area.  */
  assert (sym1P->value.Tag == ValueSymbol && sym2P->value.Tag == ValueSymbol);
  assert (sym1P->value.Data.Symbol.factor == 1 && sym2P->value.Data.Symbol.factor == 1); 
  const Symbol *areaSym1P = sym1P->value.Data.Symbol.symbol;
  const Symbol *areaSym2P = sym2P->value.Data.Symbol.symbol;
  if (areaSym1P != areaSym2P)
    return strcasecmp (areaSym1P->str, areaSym2P->str);

  /* AOF: then sort according to their area offset value.  */
  return sym1P->value.Data.Symbol.offset - sym2P->value.Data.Symbol.offset;
}


SymbolOut_t
Symbol_CreateSymbolOut (void)
{
  SymbolOut_t result =
    {
      .allSymbolsPP = NULL,
      .numAllSymbols = 0,
      .numLocalSymbols = 0,
      .stringSize = 0
    };

  /* Figure out how many symbols we want to output.  */
  for (unsigned i = 0; i != SYMBOL_TABLESIZE; ++i)
    {
      for (Symbol *sym = symbolTable[i]; sym; sym = sym->next)
	{
	  /* At this point sym->used is -1 when it is not needed for
	     relocation, either is 0 when used for relocation.  */
	  assert (sym->used == -1 || sym->used == 0);
	  if (sym->type & SYMBOL_AREA)
	    {
	      /* All AREA symbols are local ones.  */
	      assert (SYMBOL_KIND (sym->type) == 0);
	    }
	  else
	    {
	      if (sym->used == -1)
		{
		  /* Check for undefined exported and unused imported symbols.  */
		  if (SYMBOL_KIND (sym->type) == SYMBOL_REFERENCE)
		    errorLine (sym->fileName, sym->lineNumber, ErrorWarning, "Symbol %s is imported but not used, or exported but not defined", sym->str);
		}
	      else if (SYMBOL_KIND (sym->type) == 0)
		{
		  /* Make it a reference symbol.  */
		  sym->type |= SYMBOL_REFERENCE;
		  errorLine (NULL, 0, ErrorWarning, "Symbol %s is implicitly imported", sym->str);
		}
	    }
	  if (NeedToOutputSymbol (sym))
	    {
	      ++result.numAllSymbols;
	      if ((sym->type & SYMBOL_AREA) || SYMBOL_KIND (sym->type) == SYMBOL_LOCAL)
		++result.numLocalSymbols;
	      sym->used = 0;
	    }
          else
	    sym->used = -1;
	}
    }

  /* Claim space.  */
  if ((result.allSymbolsPP = malloc (result.numAllSymbols * sizeof (Symbol *))) == NULL)
    errorOutOfMem ();

  /* Run over symbols again, and start assigning them in our symbol output
     array.  */
  const unsigned localSymbolIndexStart = (option_aof) ? result.numAllSymbols - result.numLocalSymbols : 0;
  const unsigned globalSymbolIndexStart = (option_aof) ? 0 : result.numLocalSymbols;
  unsigned localSymbolIndex = localSymbolIndexStart;
  unsigned globalSymbolIndex = globalSymbolIndexStart;
  for (unsigned i = 0; i != SYMBOL_TABLESIZE; ++i)
    {
      for (Symbol *sym = symbolTable[i]; sym; sym = sym->next)
	{
	  if (sym->used < 0)
	    continue;
	  if ((sym->type & SYMBOL_AREA) || SYMBOL_KIND (sym->type) == SYMBOL_LOCAL)
	    result.allSymbolsPP[localSymbolIndex++] = sym;
	  else
	    result.allSymbolsPP[globalSymbolIndex++] = sym;
	}
    }
  assert ((option_aof && localSymbolIndex == result.numAllSymbols && globalSymbolIndex == localSymbolIndexStart)
          ^ (!option_aof && localSymbolIndex == globalSymbolIndexStart && globalSymbolIndex == result.numAllSymbols));

  /* Sort local and global symbols individually.  */
  qsort (&result.allSymbolsPP[localSymbolIndexStart], result.numLocalSymbols, sizeof (Symbol *), SymbolCompare);
  qsort (&result.allSymbolsPP[globalSymbolIndexStart], result.numAllSymbols - result.numLocalSymbols, sizeof (Symbol *), SymbolCompare);

  /* Assign Symbol::offset.
     We want to limit output the strings of mapping symbol to the first two
     characters (for $a, $d and $d) or first 4 characters (for $t.x) so we can
     share their string output.
     Also collect the final string size needed for our output symbols.  */
  struct
    {
      unsigned offset;
      const unsigned len;
    } mapSymbols[4] =
    {
      { UINT_MAX, sizeof ("$a") }, /* $a */
      { UINT_MAX, sizeof ("$d") }, /* $d */
      { UINT_MAX, sizeof ("$t") }, /* $t */
      { UINT_MAX, sizeof ("$t.x") } /* $t.x */
    };
  for (unsigned symbolIndex = 0; symbolIndex != result.numAllSymbols; ++symbolIndex)
    {
      Symbol *sym = result.allSymbolsPP[symbolIndex];
      Area_eEntryType type = Area_IsMappingSymbol (sym->str);
      if (type != eInvalid)
	{
	  assert (!(sym->type & SYMBOL_AREA));
	  int mappingSymbolindex;
	  switch (type)
	    {
	      default:
		assert (0);
		/* Fall through.  */
	      case eARM:
		mappingSymbolindex = 0;
		break;
	      case eData:
		mappingSymbolindex = 1;
		break;
	      case eThumb:
		mappingSymbolindex = 2;
		break;
	      case eThumbEE:
		mappingSymbolindex = 3;
		break;
	    }
	  if (mapSymbols[mappingSymbolindex].offset == UINT_MAX)
	    {
	      /* First time we see this particular mapping symbol.  */
	      mapSymbols[mappingSymbolindex].offset = sym->offset = result.stringSize;
	      result.stringSize += mapSymbols[mappingSymbolindex].len;
	    }
	  else
	    sym->offset = mapSymbols[mappingSymbolindex].offset;
	}
      else
	{
	  /* For ELF, section names are not mentioned in the string table
	     but part of the section header.  */
	  if ((sym->type & SYMBOL_AREA) && !option_aof)
	    sym->offset = 0;
	  else
	    {
	      sym->offset = result.stringSize;
	      result.stringSize += sym->len + 1;
	    }
	}
      /* Symbol::used now contains a number which can be used (for the choosen
         output format) to refer to that symbol.
	 For AOF: if it is an area symbol, it's its area number. For all other
	          symbols, the index in the symbol table.
	 For ELF: the index in symbol table after the undefined symbol (area
	          symbols also have an entry in the symbol table as
	          STT_SECTION).  */
      if (option_aof)
	sym->used = (sym->type & SYMBOL_AREA) ? sym->area->number : symbolIndex;
      else
	sym->used = symbolIndex + 1;
    }
  return result;
}


void
Symbol_OutputStrings (FILE *outfile, const SymbolOut_t *symOutP)
{
  unsigned count = 0;
  for (unsigned i = 0; i != symOutP->numAllSymbols; ++i)
    {
      const Symbol *sym = symOutP->allSymbolsPP[i];
      Area_eEntryType type = Area_IsMappingSymbol (sym->str);
      if (type != eInvalid)
	{
	  if (sym->offset >= count)
	    {
	      assert (sym->offset == count);
	      fputc ('$', outfile);
	      fputc (sym->str[1], outfile);
	      if (type == eThumbEE)
		fputs (".x", outfile);
	      fputc ('\0', outfile);
	      if (type == eThumbEE)
		count += sizeof ("$t.x");
	      else
		count += sizeof ("$d"); /* sizeof ("$d") == sizeof ("$a") == sizeof ("$t") */
	    }
	}
      else if ((sym->type & SYMBOL_AREA) && !option_aof)
	assert (sym->offset == 0);
      else
	{
	  assert (sym->offset == count);
	  count += fwrite (sym->str, 1, sym->len + 1, outfile);
	}
    }
  assert (count == symOutP->stringSize);
}


static unsigned
ApplyDefaultSymbolTypeForExportOrImport (unsigned type, unsigned mask)
{
  /* If FPREGARGS nor NOFPREGARGS are specified, take the default.  */
  if ((mask & SYMBOL_FPREGARGS) == 0
      && (gOptionAPCS & APCS_OPT_FPREGARGS) != 0)
    type |= SYMBOL_FPREGARGS;
  if ((mask & SYMBOL_SOFTFP) == 0
      && (gOptionAPCS & APCS_OPT_SOFTFP) != 0)
    type |= SYMBOL_SOFTFP;
  return type;
}


void
Symbol_OutputForAOF (FILE *outfile, const SymbolOut_t *symOutP)
{
  for (unsigned i = 0; i != symOutP->numAllSymbols; ++i)
    {
      const Symbol *sym = symOutP->allSymbolsPP[i];

      if (sym->type & SYMBOL_AREA)
	{
	  assert (((sym->type & SYMBOL_ABSOLUTE) != 0) == ((sym->area->type & AREA_ABS) != 0));
	  const AofSymbol asym =
	    {
	      .Name = armword (sym->offset + 4), /* + 4 to skip the initial length */
	      .Type = armword (SYMBOL_LOCAL | (sym->type & SYMBOL_ABSOLUTE)),
	      .Value = armword ((sym->area->type & AREA_ABS) ? Area_GetBaseAddress (sym) : 0),
	      .AreaName = armword (sym->offset + 4) /* + 4 to skip the initial length */
	    };
	  fwrite (&asym, sizeof (AofSymbol), 1, outfile);
	}
      else
	{
	  AofSymbol asym;
	  if (sym->type & SYMBOL_DEFINED)
	    {
	      /* SYMBOL_LOCAL, SYMBOL_GLOBAL */
assert (sym->value.Tag == ValueInt || sym->value.Tag == ValueSymbol); /* FIXME: is codeEvalLow still needed ? */
	      const Value *valueP;
	      if (sym->value.Tag == ValueCode)
		{
		  codeInit ();
		  valueP = codeEvalLow (ValueAll, sym->value.Data.Code.len,
					sym->value.Data.Code.c, NULL);
		}
	      else
		valueP = &sym->value;

	      /* We can only have Int and Symbol here.  */
	      int v;
	      const Symbol *relToAreaSymP;
	      switch (valueP->Tag)
		{
		  case ValueInt:
		    /* Typically use of EQU or * on pure integers.  */
		    assert (valueP->Data.Int.type == eIntType_PureInt);
		    v = valueP->Data.Int.i;
		    relToAreaSymP = NULL;
		    break;

                  case ValueSymbol:
		    {
		      v = valueP->Data.Symbol.offset;
		      relToAreaSymP = valueP->Data.Symbol.symbol;
		      /* Resolve absolute area labels (but not for mapping
		         labels).  */
		      if ((relToAreaSymP->type & SYMBOL_AREA) != 0
			  && (relToAreaSymP->area->type & AREA_ABS) != 0
		          && !Area_IsMappingSymbol (sym->str))
			{
			  v += valueP->Data.Symbol.factor * Area_GetBaseAddress (relToAreaSymP);
			  relToAreaSymP = NULL;
			}
		      else if (valueP->Data.Symbol.factor != 1)
			{
			  error (ErrorError, "Unable to export symbol %s", sym->str);
			  continue;
			}
		      break;
		    }

		  default:
		    assert (!"Wrong value tag selection");
		    relToAreaSymP = NULL;
		    v = 0;
		    break;
		}
	      /* Note, no support for oAllExportSymbolsAreWeak as this is
		 not possible for AOF.  */
	      uint32_t type = sym->type & SYMBOL_SUPPORTED_AOF_BITS;
	      if (oExportAllSymbols && !Area_IsMappingSymbol (sym->str))
		type = ApplyDefaultSymbolTypeForExportOrImport (type | SYMBOL_EXPORT, 0);
	      asym.Type = armword (type);
	      asym.Value = armword (v);
	      /* When it is a non-absolute symbol, we need to specify the
	         area name to which this symbol is relative to.  */
	      assert (((sym->type & SYMBOL_ABSOLUTE) == 0) == (relToAreaSymP != NULL)); 
	      assert (relToAreaSymP == NULL || (relToAreaSymP->type & SYMBOL_AREA) != 0);
	      asym.AreaName = armword (relToAreaSymP != NULL ? relToAreaSymP->offset + 4 : 0);
	    }
	  else
	    {
	      /* SYMBOL_REFERENCE */
	      asym.Type = armword ((sym->type | SYMBOL_REFERENCE) & SYMBOL_SUPPORTED_AOF_BITS);
	      asym.Value = armword ((sym->type & SYMBOL_COMMON) ? sym->value.Data.Int.i : 0);
	      asym.AreaName = armword (0);
	    }
	  asym.Name = armword (sym->offset + 4); /* + 4 to skip the initial length */
	  fwrite (&asym, sizeof (AofSymbol), 1, outfile);
	}
    }
}


#ifndef NO_ELF_SUPPORT
void
Symbol_OutputForELF (FILE *outfile, const SymbolOut_t *symOutP)
{
  /* Output the undefined symbol.  */
  Elf32_Sym asym =
    {
      .st_name = 0,
      .st_value = 0,
      .st_size = 0,
      .st_info = 0,
      .st_other = 0,
      .st_shndx = 0
    };
  fwrite (&asym, sizeof (Elf32_Sym), 1, outfile);

  for (unsigned i = 0; i != symOutP->numAllSymbols; ++i)
    {
      const Symbol *sym = symOutP->allSymbolsPP[i];

      if (sym->type & SYMBOL_AREA)
	{
	  assert (((sym->type & SYMBOL_ABSOLUTE) != 0) == ((sym->area->type & AREA_ABS) != 0));
	  assert (SYMBOL_KIND (sym->type) == 0);
          asym.st_name = 0;
	  asym.st_value = (sym->area->type & AREA_ABS) ? Area_GetBaseAddress (sym) : 0;
	  asym.st_size = 0; /* No the area size.  */
	  asym.st_info = ELF32_ST_INFO (STB_LOCAL, STT_SECTION);
	  asym.st_other = sym->visibility;
	  asym.st_shndx = sym->area->number;
	  assert (asym.st_shndx >= 3);
	  fwrite (&asym, sizeof (Elf32_Sym), 1, outfile);
	}
      else
	{
	  asym.st_name = sym->offset + 1; /* + 1 to skip the initial & extra NUL */
	  if (sym->type & SYMBOL_DEFINED)
	    {
	      /* SYMBOL_LOCAL, SYMBOL_GLOBAL */
assert (sym->value.Tag == ValueInt || sym->value.Tag == ValueSymbol); /* FIXME: is codeEvalLow still needed ? */
	      const Value *valueP;
	      if (sym->value.Tag == ValueCode)
		{
		  codeInit ();
		  valueP = codeEvalLow (ValueAll, sym->value.Data.Code.len,
					sym->value.Data.Code.c, NULL);
		}
	      else
		valueP = &sym->value;

	      /* We can only have Int and Symbol here.  */
	      int v;
	      const Symbol *relToAreaSymP;
	      switch (valueP->Tag)
		{
		  case ValueInt:
		    /* Typically use of EQU or * on pure integers.  */
		    assert (valueP->Data.Int.type == eIntType_PureInt);
		    v = valueP->Data.Int.i;
		    relToAreaSymP = NULL;
		    break;

                  case ValueSymbol:
		    {
		      v = valueP->Data.Symbol.offset;
		      relToAreaSymP = valueP->Data.Symbol.symbol;
		      /* Resolve absolute area labels (but not for mapping
		         labels).  */
		      if ((relToAreaSymP->type & SYMBOL_AREA) != 0
			  && (relToAreaSymP->area->type & AREA_ABS) != 0
		          && !Area_IsMappingSymbol (sym->str))
			{
			  v += valueP->Data.Symbol.factor * Area_GetBaseAddress (relToAreaSymP);
			  relToAreaSymP = NULL;
			}
		      else if (valueP->Data.Symbol.factor != 1)
			{
			  error (ErrorError, "Unable to export symbol %s", sym->str);
			  continue;
			}
		      break;
		    }

		  default:
		    assert (!"Wrong value tag selection");
		    relToAreaSymP = NULL;
		    v = 0;
		    break;
		}
	      asym.st_value = v;
	      assert (((sym->type & SYMBOL_ABSOLUTE) == 0) == (relToAreaSymP != NULL));
	      assert (relToAreaSymP == NULL || (relToAreaSymP->type & SYMBOL_AREA) != 0);
	      asym.st_shndx = relToAreaSymP != NULL ? relToAreaSymP->area->number : SHN_ABS;
	    }
	  else
	    {
	      /* SYMBOL_REFERENCE */
	      asym.st_value = (sym->type & SYMBOL_COMMON) ? sym->aligned : 0;
	      asym.st_shndx = (sym->type & SYMBOL_COMMON) ? SHN_COMMON : SHN_UNDEF;
	    }
	  asym.st_size = sym->codeSize;
	  asym.st_other = sym->visibility;

	  int bind;
	  if (((sym->type & SYMBOL_WEAK) != 0 || oAllExportSymbolsAreWeak) && !Area_IsMappingSymbol (sym->str))
	    bind = STB_WEAK;
	  else
	    {
	      uint32_t type = oExportAllSymbols && !Area_IsMappingSymbol (sym->str) ? SYMBOL_GLOBAL : SYMBOL_KIND (sym->type);
	      switch (type)
		{
		  default:
		    assert (0);
		    /* Fall through.  */
		  case SYMBOL_LOCAL:
		    bind = STB_LOCAL;
		    break;
		  case SYMBOL_REFERENCE:
		  case SYMBOL_GLOBAL:
		    bind = STB_GLOBAL;
		    break;
		}
	    }
	  asym.st_info = ELF32_ST_INFO (bind, (sym->type & SYMBOL_COMMON) ? STT_OBJECT : STT_NOTYPE);
	  if (asym.st_shndx == (Elf32_Half)-1)
	    errorAbort ("Internal symbolSymbolELFOutput: unable to find section id for symbol %s", sym->str);
	  else
	    fwrite (&asym, sizeof (Elf32_Sym), 1, outfile);
	}
    }
}
#endif


void
Symbol_FreeSymbolOut (SymbolOut_t *symOutP)
{
  free ((void *)symOutP->allSymbolsPP);
  symOutP->allSymbolsPP = NULL;
}


/**
 * \return NULL when no symbol could be read (and *no* error is given),
 * non-NULL otherwise (even when symbol is not yet known).
 */
static Symbol *
ParseSymbolAndAdjustFlag (unsigned int flags, const char *err)
{
  const Lex lex = lexGetIdNoError ();
  if (lex.tag != LexId)
    return NULL;

  /* When the symbol is not known yet, it will automatically be created.  */
  Symbol *sym = Symbol_Get (&lex);
  if (Local_IsLocalLabel (sym->str))
    error (ErrorError, "Local labels cannot be %s", err);
  else if (Area_IsMappingSymbol (sym->str))
    error (ErrorError, "Mapping symbols cannot be %s", err);
  else if ((sym->type & SYMBOL_RW) != 0)
    error (ErrorError, "%s symbols cannot be %s",
           sym->type & SYMBOL_MACRO_LOCAL ? "Local" : "Global", err);
  else
    sym->type |= flags;
  return sym;
}


/**
 * Implements COMMON
 *   COMMON symbol{,size{,alignment}} {[attr]}
 * with attr DYNAMIC, PROTECTED, HIDDEN or INTERNAL.
 */
bool
c_common (void)
{
  Symbol *commonSym = ParseSymbolAndAdjustFlag (SYMBOL_REFERENCE | SYMBOL_COMMON, "marked as COMMON");
  if (commonSym == NULL)
    {
      error (ErrorError, "Missing symbol");
      return false;
    }

  skipblanks ();
  uint32_t commonSize = 0;
  uint32_t commonAlignment = 4;
  unsigned commonVisibility = STV_DEFAULT;
  if (!Input_IsEolOrCommentStart ())
    {
      if (!Input_Match (',', true))
	{
	  error (ErrorError, "Missing ,");
	  return false;
	}
      const Value *valueSizeP = exprBuildAndEval (ValueInt);
      switch (valueSizeP->Tag)
	{
	  case ValueInt:
	    if (valueSizeP->Data.Int.type == eIntType_PureInt)
	      break;
	    /* Fall through.  */

	  default:
	    error (ErrorError, "Illegal COMMON size expression");
	    return true;
	}
      commonSize = valueSizeP->Data.Int.i;

      skipblanks ();
      if (!Input_IsEolOrCommentStart ())
	{
	  if (!Input_Match (',', true))
	    {
	      error (ErrorError, "Missing ,");
	      return false;
	    }
	  const Value *valueAlignmentP = exprBuildAndEval (ValueInt);
	  switch (valueAlignmentP->Tag)
	    {
	      case ValueInt:
		if (valueAlignmentP->Data.Int.type == eIntType_PureInt)
		  break;
		/* Fall through.  */

	      default:
		error (ErrorError, "Illegal COMMON size expression");
		return true;
	    }
	  commonAlignment = valueAlignmentP->Data.Int.i;
	  if (commonAlignment == 0 || (commonAlignment & (commonAlignment - 1)) != 0)
	    {
	      error (ErrorError, "Alignment value needs to be a power of 2");
	      commonAlignment = 4;
	    }

          skipblanks ();
	  if (!Input_IsEndOfKeyword ())
	    {
	      const Lex attribute = lexGetIdNoError ();
	      const char * const attrs[] =
		{
		  /* Same order as STV_* values.  */
		  "DYNAMIC", "INTERNAL", "HIDDEN", "PROTECTED"
		};
	      size_t i;
	      for (i = 0; i != sizeof (attrs) / sizeof (attrs[0]); ++i)
		{
		  if (attribute.tag == LexId
		      && !strcmp (attribute.Data.Id.str, attrs[i]))
		    break;
		}
	      if (i == sizeof (attrs) / sizeof (attrs[0]))
		{
		  error (ErrorError, "Missing or wrong type of attribute");
		  return true;
		}
	      commonVisibility = (unsigned)i;
	    }
	}
    }
  commonSym->value = Value_Int (commonSize, eIntType_PureInt);
  commonSym->codeSize = commonSize;
  commonSym->aligned = commonAlignment;
  commonSym->visibility = commonVisibility;

  return false;
}


/* Result of parsing IMPORT/EXPORT qualifier list.  */ 
typedef struct
{
  uint32_t flagValue;
  uint32_t flagSet;
  int basedRegNum; /* Valid when != -1.  */
  uint32_t commonSize; /* Valid when SYMBOL_COMMON is set in flagValue.  */
} QualifierResult_t;

/**
 * Parse the BASED qualifier for IMPORT.
 *   "BASED Rn"
 * \return true for error, false for success.
 */
static bool
ParseQualifierBASED (QualifierResult_t *result)
{
  ARMWord cpuReg = getCpuReg ();
  if (cpuReg == INVALID_REG)
    return true;

  /* When BASED is specified twice, we expect the same value.
     BASED and COMMON can not be specified together.  */
  if ((result->basedRegNum != -1 && (uint32_t)result->basedRegNum != cpuReg)
      || (result->flagValue & SYMBOL_COMMON) != 0)
    {
      error (ErrorError, "Qualifier %.*s conflicts with a previously given qualifier",
	     (int)sizeof ("BASED")-1, "BASED");
      return true;
    }
  result->basedRegNum = cpuReg;

  return false;
}

/**
 * Parse the COMMON qualifier for IMPORT.
 *   "COMMON = <size>"
 * \return true for error, false for success.
 */
static bool
ParseQualifierCOMMON (QualifierResult_t *result)
{
  skipblanks ();
  if (!Input_Match ('=', false))
    {
      error (ErrorError, "COMMON attribute needs size specification");
      return true;
    }

  const Value *size = exprBuildAndEval (ValueInt);
  switch (size->Tag)
    {
      case ValueInt:
	if (size->Data.Int.type == eIntType_PureInt)
	  break;
	/* Fall through.  */

      default:
	error (ErrorError, "Illegal COMMON attribute expression");
	return true;
    }
  /* Qualifier BASED and COMMON can not specified together.  */
  if (result->basedRegNum != -1
      || ((result->flagValue & SYMBOL_COMMON) != 0 && result->commonSize != (uint32_t)size->Data.Int.i))
    {
      error (ErrorError, "Qualifier %.*s conflicts with a previously given qualifier",
	     (int)sizeof ("COMMON")-1, "COMMON");
      return true;
    }
  result->flagValue |= SYMBOL_COMMON;
  result->flagSet |= SYMBOL_COMMON;
  result->commonSize = (uint32_t)size->Data.Int.i;

  return false;
}

enum { eForImport, eForExport, eForBoth };
#define WEAK_ENTRY 12
static const struct QKeyword
{
  const char *keyword;
  size_t size;
  int applicable;
  uint32_t flagValue;
  uint32_t flagSet;
  bool (*parser)(QualifierResult_t *);
} oQualifiers [] =
{
  { "FPREGARGS", sizeof ("FPREGARGS")-1, eForBoth, SYMBOL_FPREGARGS, SYMBOL_FPREGARGS, NULL },
  { "NOFPREGARGS", sizeof ("NOFPREGARGS")-1, eForBoth, 0, SYMBOL_FPREGARGS, NULL },
  { "SOFTFP", sizeof ("SOFTFP")-1, eForBoth, SYMBOL_SOFTFP, SYMBOL_SOFTFP, NULL },
  { "HARDFP", sizeof ("HARDFP")-1, eForBoth, 0, SYMBOL_SOFTFP, NULL },
  { "DATA", sizeof ("DATA")-1, eForBoth, SYMBOL_DATUM, SYMBOL_DATUM, NULL },
  { "CODE", sizeof ("CODE")-1, eForBoth, 0, SYMBOL_DATUM, NULL },
  { "ARM", sizeof ("ARM")-1, eForExport, 0, SYMBOL_THUMB, NULL },
  { "THUMB", sizeof ("THUMB")-1, eForExport, SYMBOL_THUMB, SYMBOL_THUMB, NULL },
  { "LEAF", sizeof ("LEAF")-1, eForExport, SYMBOL_LEAF, SYMBOL_LEAF, NULL },
  { "NOLEAF", sizeof ("NOLEAF")-1, eForExport, 0, SYMBOL_LEAF, NULL },
  { "USESSB", sizeof ("USESSB")-1, eForExport, SYMBOL_USESSB, SYMBOL_USESSB, NULL },
  { "NOUSESSB", sizeof ("NOUSESSB")-1, eForExport, 0, SYMBOL_USESSB, NULL },
  { "WEAK", sizeof ("WEAK")-1, eForImport, SYMBOL_WEAK, SYMBOL_WEAK, NULL }, 
  { "NOWEAK", sizeof ("NOWEAK")-1, eForImport, 0, SYMBOL_WEAK, NULL },
  { "READONLY", sizeof ("READONLY")-1, eForImport, SYMBOL_READONLY, SYMBOL_READONLY, NULL },
  { "READWRITE", sizeof ("READWRITE")-1, eForImport, 0, SYMBOL_READONLY, NULL },
  { "NOCASE", sizeof ("NOCASE")-1, eForImport, SYMBOL_NOCASE, SYMBOL_NOCASE, NULL },
  { "CASE", sizeof ("CASE")-1, eForImport, 0, SYMBOL_NOCASE, NULL },
  { "BASED", sizeof ("BASED")-1, eForImport, 0, 0, ParseQualifierBASED },
  { "COMMON", sizeof ("COMMON")-1, eForImport, 0, 0, ParseQualifierCOMMON }
};

/**
 * Parses the qualifier list used as argument for EXPORT/IMPORT
 * (or GLOBAL/EXTERN).
 * Qualifier list starts with '[' but that's already parsed.  It ends with ']'.
 */
static QualifierResult_t
GetQualifierList (bool forExport, bool weakOnly)
{
  QualifierResult_t result =
    {
      .flagValue = 0,
      .flagSet = 0,
      .basedRegNum = -1,
      .commonSize = 0
    };

  if (!Input_Match (']', true))
    {
      do
	{
	  Lex qualifier = lexGetIdNoError ();
	  if (qualifier.tag != LexId)
	    {
	      error (ErrorError, "Missing or wrong type of %s qualifier", forExport ? "EXPORT" : "IMPORT");
	      break;
	    }

	  const struct QKeyword *keyword = NULL;
	  if (weakOnly)
	    {
	      assert (forExport);
	      if (qualifier.Data.Id.len == oQualifiers[WEAK_ENTRY].size
	          && !memcmp (qualifier.Data.Id.str, oQualifiers[WEAK_ENTRY].keyword, oQualifiers[WEAK_ENTRY].size))
		keyword = &oQualifiers[WEAK_ENTRY];
	    }
	  else
	    {
	      for (size_t i = 0; i != sizeof (oQualifiers) / sizeof (oQualifiers[0]); ++i)
		{
		  if ((oQualifiers[i].applicable == eForBoth
		       || oQualifiers[i].applicable == (forExport ? eForExport : eForImport))
		      && qualifier.Data.Id.len == oQualifiers[i].size
		      && !memcmp (qualifier.Data.Id.str, oQualifiers[i].keyword, oQualifiers[i].size))
		    {
		      keyword = &oQualifiers[i];
		      break;
		    }
		}
	    }
	  if (keyword == NULL)
	    {
	      error (ErrorError, "Qualifier %.*s is not known",
		     (int)qualifier.Data.Id.len, qualifier.Data.Id.str);
	    }
	  else
	    {
	      /* If there is a extra parser, use that.  */
	      if (keyword->parser != NULL)
		{
		  if (keyword->parser(&result))
		    break;
		}
	      else if ((keyword->flagSet & result.flagSet) != 0)
		{
		  /* Keyword has already been set.  Verify consistency.  */
		  if ((keyword->flagSet & result.flagValue) != keyword->flagValue)
		    {
		      error (ErrorError, "Qualifier %.*s conflicts with a previously given qualifier",
			     (int)qualifier.Data.Id.len, qualifier.Data.Id.str);
		      break;
		    }
		}
	      else
		{
		  result.flagValue |= keyword->flagValue;
		  result.flagSet |= keyword->flagSet;
		}
	    }

	  skipblanks ();
	  if (Input_Match (']', true))
	    break;
	  if (Input_IsEolOrCommentStart ())
	    {
	      error (ErrorError, "Missing ]");
	      break;
	    }
	  if (!Input_Match (',', true))
	    {
	      error (ErrorError, "Missing ,");
	      break;
	    }
	} while (1);
    }

  return result;
}

/**
 * Implements EXPORT / GLOBAL.
 *   "EXPORT" <symbol> { "[" <qualifier list> "]" }
 *   "EXPORT" <symbol> { "," <symbol> }*
 *   "EXPORT" { "[WEAK]" }
 *
 * "EXPORT [WEAK]" is only supported for ELF.
 */
/* FIXME: ELF support missing.  */
bool
c_export (void)
{
  /* 'EXPORT'/'GLOBAL' is not supported when in AAsm compatibility mode.  */
  if (option_abs)
    return true;

  Symbol *sym = ParseSymbolAndAdjustFlag (SYMBOL_REFERENCE, "exported");
  skipblanks ();
  if (sym == NULL)
    {
      oExportAllSymbols = true;
      if (!Input_IsEolOrCommentStart ())
	{
	  if (!Input_Match ('[', true))
	    error (ErrorError, "Missing [");
	  else
	    {
	      const QualifierResult_t result = GetQualifierList (true, true);
	      if ((result.flagValue & SYMBOL_WEAK) != 0)
		{
		  if (option_aof)
		    error (ErrorWarning, "Making EXPORT symbols WEAK is not supported for AOF output");
		  else
		    oAllExportSymbolsAreWeak = true;
		}
	    }
	}
    }
  else if (!Input_IsEolOrCommentStart ())
    {
      if (Input_Match (',', true))
	{
	  /* More than one symbol is exported.  */
	  do
	    {
	      if (ParseSymbolAndAdjustFlag (SYMBOL_REFERENCE, "exported") == NULL)
		error (ErrorError, "Missing symbol");
	      skipblanks ();
	    } while (Input_Match (',', true));
	}
      else if (Input_Match ('[', true))
	{
	  QualifierResult_t result = GetQualifierList (true, false);
	  if (option_aof && (result.flagValue & SYMBOL_WEAK) != 0)
	    {
	      error (ErrorWarning, "Making EXPORT symbol %s WEAK is not supported for AOF output", sym->str);
	      result.flagValue &= ~SYMBOL_WEAK;
	    }
	  sym->type = ApplyDefaultSymbolTypeForExportOrImport (sym->type | result.flagValue, result.flagSet);
	  assert (result.basedRegNum == -1);
	  assert (result.commonSize == 0 && (result.flagValue & SYMBOL_COMMON) == 0);
	}
      else
	error (ErrorError, "Missing [");
    }
  else
    sym->type = ApplyDefaultSymbolTypeForExportOrImport (sym->type, 0);

  return false;
}


/**
 * Implements STRONG.
 */
bool
c_strong (void)
{
  /* 'STRONG' is not supported when in AAsm compatibility mode.  */
  if (option_abs)
    return true;

  if (ParseSymbolAndAdjustFlag (SYMBOL_STRONG, "marked as STRONG") == NULL)
    error (ErrorError, "Missing symbol");
  return false;
}

/**
 * Implements KEEP.
 *   KEEP <symbol> : enforces <symbol> to be in the output.
 *   KEEP : enforces all symbols to be in the output.
 */
bool
c_keep (void)
{
  /* 'KEEP' is not supported when in AAsm compatibility mode.  */
  if (option_abs)
    return true;

  if (ParseSymbolAndAdjustFlag (SYMBOL_KEEP, "marked to KEEP") == NULL)
    oKeepAllSymbols = true;
  return false;
}

/**
 * Implements LEAF.
 */
bool
c_leaf (void)
{
  /* 'LEAF' is not supported when in AAsm compatibility mode.  */
  if (option_abs)
    return true;

  if (ParseSymbolAndAdjustFlag (SYMBOL_LEAF, "marked as LEAF") == NULL)
    error (ErrorError, "Missing symbol");
  return false;
}

/**
 * Implements IMPORT / EXTERN.
 *   IMPORT <symbol> { "[" <qualifier list> "]" } { ", WEAK" }
 *   IMPORT <symbol> { ", FPREGARGS" } { ", WEAK" }
 * FIXME: support ELF attributes
 * FIXME: there is a difference betweein IMPORT and EXTERN
 */
bool
c_import (void)
{
  /* 'IMPORT'/'EXTERN' is not supported when in AAsm compatibility mode.  */
  if (option_abs)
    return true;

  Symbol *sym = ParseSymbolAndAdjustFlag (SYMBOL_REFERENCE, "imported");
  if (sym == NULL)
    {
      error (ErrorError, "Missing symbol for import");
      return false;
    }

  skipblanks ();
  if (!Input_IsEolOrCommentStart ())
    {
      QualifierResult_t result;
      if (Input_Match ('[', true))
	result = GetQualifierList (false, false);
      else
	{
	  result.flagSet = result.flagValue = 0;
	  result.basedRegNum = -1;
	  result.commonSize = 0;
	}
      /* Additionally parse "FPREGARGS" and/or "WEAK".  */
      skipblanks ();
      uint32_t flagValue = 0;
      while (Input_Match (',', true))
	{
	  Lex qualifier = lexGetIdNoError ();
	  if (qualifier.tag != LexId)
	    {
	      error (ErrorError, "Missing or wrong type of %s qualifier", "IMPORT");
	      break;
	    }
	  if (qualifier.Data.Id.len == sizeof ("FPREGARGS")-1
	      && !memcmp (qualifier.Data.Id.str, "FPREGARGS", sizeof ("FPREGARGS")-1))
	    flagValue |= SYMBOL_FPREGARGS;
	  else if (qualifier.Data.Id.len == sizeof ("WEAK")-1
		   && !memcmp (qualifier.Data.Id.str, "WEAK", sizeof ("WEAK")-1))
	    flagValue |= SYMBOL_WEAK;
	  else
	    {
	      error (ErrorError, "Qualifier %.*s is not known",
		     (int)qualifier.Data.Id.len, qualifier.Data.Id.str);
	      break;
	    }
	  skipblanks ();
	}
      /* Check for conflicting FPREGARGS and/or WEAK values when one or both
         are specified twice.  */
      if ((result.flagValue & result.flagSet & flagValue) != (result.flagSet & flagValue))
	error (ErrorError, "Conflicting qualifier");
      result.flagValue |= flagValue;
      result.flagSet |= flagValue;

      sym->type = ApplyDefaultSymbolTypeForExportOrImport (sym->type | result.flagValue, result.flagSet);
      /* Extra for BASED and COMMON qualifiers.  */
      if ((sym->type & SYMBOL_COMMON) != 0)
	{
	  const Value value = Value_Int(result.commonSize, eIntType_PureInt);
	  Value_Assign (&sym->value, &value);
	  sym->codeSize = result.commonSize;
	}
      else if (result.basedRegNum != -1)
	{
	  sym->type |= SYMBOL_ABSOLUTE; /* FIXME: Correct ? See c_alloc implementation. */
	  const Value value = Value_Addr (result.basedRegNum, 0);
	  Value_Assign (&sym->value, &value);
	}
    }
  else
    sym->type = ApplyDefaultSymbolTypeForExportOrImport (sym->type, 0);

  return false;
}

#ifdef DEBUG
void
symbolPrint (const Symbol *sym)
{
  static const char * const symkind[4] = { "unknown", "local", "reference", "global" };
  printf ("\"%s\": %s",
          sym->str, symkind[SYMBOL_KIND (sym->type)]);
  assert (strlen (sym->str) == (size_t)sym->len);
  assert (sym->area == NULL || ((sym->type & SYMBOL_DEFINED) != 0 && (sym->type & SYMBOL_AREA) != 0 && sym->area != NULL));

  /* Dump the symbol attributes:  */
  if (sym->type & SYMBOL_ABSOLUTE)
    printf (", absolute");
  if (sym->type & SYMBOL_NOCASE)
    printf (", caseinsensitive");
  if (sym->type & SYMBOL_WEAK)
    printf (", weak");
  if (sym->type & SYMBOL_STRONG)
    printf (", strong");
  if (sym->type & SYMBOL_COMMON)
    printf (", common");
  if (sym->type & SYMBOL_DATUM)
    printf (", datum");
  if (sym->type & SYMBOL_FPREGARGS)
    printf (", fp args in regs");
  if (sym->type & SYMBOL_LEAF)
    printf (", leaf");
  if (sym->type & SYMBOL_THUMB)
    printf (", thumb");
  /* Internal attributes: */
  if (sym->type & SYMBOL_MACRO_LOCAL)
    printf (", local macro");
  if (sym->type & SYMBOL_NO_EXPORT)
    printf (", no export");
  if (sym->type & SYMBOL_RW)
    printf (", rw");
  if (sym->type & SYMBOL_KEEP)
    printf (", keep");
  if (sym->type & SYMBOL_AREA)
    printf (", area %p", (void *)sym->area);

  printf (", size %" PRIu32 ", offset 0x%x, used %d : ",
          sym->codeSize, sym->offset, sym->used);
  valuePrint (&sym->value);
}

/**
 * Lists all symbols collected so far together with all its attributes.
 */
void
symbolPrintAll (void)
{
  for (int i = 0; i != SYMBOL_TABLESIZE; i++)
    {
      for (const Symbol *sym = symbolTable[i]; sym; sym = sym->next)
	{
	  symbolPrint (sym);
	  printf ("\n");
	}
    }
}
#endif
