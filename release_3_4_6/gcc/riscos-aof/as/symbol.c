/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas R�emo
 * Copyright (c) 2000-2006 GCCSDK Developers
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#elif HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "aoffile.h"
#include "area.h"
#include "code.h"
#include "elf.h"
#include "error.h"
#include "global.h"
#include "help_lex.h"
#include "local.h"
#include "main.h"
#include "output.h"
#include "symbol.h"

#define SYMBOL_OUTPUT(sym) \
  (((sym)->type & (SYMBOL_EXPORT|SYMBOL_KEEP)) > 0 \
    && (((sym)->type & SYMBOL_DEFINED) > 0 || (sym)->used > -1))
  /* must be exported & defined, or imported and referenced */
int (SYMBOL_OUTPUT) (const Symbol *);	/* typedef it */

static Symbol *symbolTable[SYMBOL_TABLESIZE];

static Symbol *
symbolNew (int len, const char *str)
{
  Symbol *result;
  if ((result = (Symbol *) malloc (sizeof (Symbol) + len)) != NULL)
    {
      result->next = NULL;
      result->type = result->declared = result->offset = 0;
      result->value.Tag.t = ValueIllegal;
      result->value.Tag.v = ValueConst;
      result->used = -1;
      result->len = len;
      memcpy (result->str, str, len);
      result->str[len] = 0;
    }
  else
    errorOutOfMem ("symbolNew");

  return result;
}

static BOOL
EqSymLex (const Symbol * str, const Lex * lx)
{
  if (str->len != lx->LexId.len)
    return FALSE;
  return !memcmp(str->str, lx->LexId.str, str->len);
}


Symbol *
symbolAdd (Lex l)
{
  Symbol **isearch;

  if (l.tag != LexId)
    error (ErrorSerious, FALSE, "Internal symbolAdd: non-ID");

  for (isearch = &symbolTable[l.LexId.hash]; *isearch; isearch = &((*isearch)->next))
    {
      Symbol *search = *isearch;
      if (EqSymLex (search, &l))
	{
	  if ((search->type & SYMBOL_DEFINED) && !SYMBOL_GETREG(search->type))
	    error (ErrorError, TRUE, "Redefinition of %.*s",
	           l.LexId.len, l.LexId.str);
	  else
	    {
	      if (search->type & SYMBOL_AREA)
	        {
	          if (areaCurrent->value.ValueInt.i != 0)
		    error (ErrorError, TRUE, "Symbol %.*s is already defined as area with incompatible definition",
		           l.LexId.len, l.LexId.str);
		}
	      else
		{
		  search->type |= SYMBOL_DEFINED;
		  return search;
		}
	    }
	}
    }
  *isearch = symbolNew (l.LexId.len, l.LexId.str);
  (*isearch)->type |= SYMBOL_DEFINED;
  return *isearch;
}

Symbol *
symbolGet (Lex l)
{
  Symbol **isearch = NULL;
  if (l.tag != LexId)
    {
      if (l.tag == LexNone)
	{
	  isearch = &symbolTable[0];
	  while (*isearch)
	    isearch = &((*isearch)->next);
	  *isearch = symbolNew (sizeof("|Dummy|")-1, "|Dummy|");
	}
      else
	error (ErrorSerious, FALSE, "Internal symbolGet: non-ID");
    }
  else
    {
      for (isearch = &symbolTable[l.LexId.hash]; *isearch; isearch = &((*isearch)->next))
	{
	  if (EqSymLex (*isearch, &l))
	    return *isearch;
	}
    }

  *isearch = symbolNew (l.LexId.len, l.LexId.str);
  return *isearch;
}

Symbol *
symbolFind (const Lex * l)
{
  Symbol **isearch;

  if (l->tag != LexId)
    return NULL;
  for (isearch = &symbolTable[l->LexId.hash]; *isearch; isearch = &((*isearch)->next))
    {
      if (EqSymLex (*isearch, l))
	return *isearch;
    }
  return NULL;
}

static int stringtablesize = -1;

int
symbolFix (void)		/* Returns number of symbols */
{
  int nosym = 0;
  int strsize = 4;		/* Always contains its length */
  int i;
  Symbol *sym;
  char routine[1024];
  void *area; /* FIXME: this is not usefully used. Why ? */
  const char *file;
  long int lineno;

  for (i = 0; i < SYMBOL_TABLESIZE; i++)
    {
      for (sym = symbolTable[i]; sym; sym = sym->next)
	{
	  if (sym->type & SYMBOL_AREA)
	    {
	      sym->offset = strsize;
	      strsize += sym->len + 1;
	      nosym++;
	    }
	  else
	    {
	      if (SYMBOL_KIND (sym->type) == 0)
		sym->type |= SYMBOL_REFERENCE;
	      if (SYMBOL_OUTPUT (sym) && sym->value.Tag.v == ValueConst)
		/*(sym->used >= 0 || sym->type & SYMBOL_REFERENCE) */
		{
		  int label = -1, i;
		  if (localTest (sym->str))
		    {
		      *routine = 0;
		      if (sscanf (sym->str, localFormat, &area, &label, &i, &routine) > 2)
			{
			  localFindRout (routine, &file, &lineno);
			  errorLine (lineno, file, ErrorError, FALSE, "Missing local label (fwd) with ID %02i in routine '%s'%s", label, *routine ? routine : "<anonymous>", lineno ? " in block starting" : " (unknown location)");
			}
		      else if (sscanf (sym->str + sizeof ("Local$$")-1, "%i$$%s", &i, routine) > 0)
			{
			  localFindLocal (i, &file, &lineno);
			  errorLine (lineno, file, ErrorError, FALSE, "Missing local label '%s'%s", *routine ? routine : "<anonymous>", lineno ? " in block starting" : " (unknown location)");
			}
		      return 0;
		    }

		  sym->offset = strsize;
		  strsize += sym->len + 1;
		  sym->used = nosym++;
		}
	      else
		{
		  sym->type &= ~SYMBOL_REFERENCE;
		  sym->used = -1;
		}
	    }
	}
    }
  stringtablesize = strsize;
  return nosym;
}

int
symbolStringSize (void)
{
  if (stringtablesize < 0)
    error (ErrorSerious, FALSE, "Internal symbolStringSize: bad string table size");
  return stringtablesize;
}

void
symbolStringOutput (FILE * outfile)	/* Count already output */
{
  int i;
  Symbol *sym;

  for (i = 0; i < SYMBOL_TABLESIZE; i++)
    for (sym = symbolTable[i]; sym; sym = sym->next)
      {
/*printf("%-40s  %3i %8X %8X %8X\n",sym->str,sym->used,sym->type,sym->value.Tag.t,sym->value.Tag.v); */
	if (SYMBOL_OUTPUT (sym) /*(sym)->used >= 0 */  || sym->type & SYMBOL_AREA)
	  {
/*puts("  (written)"); */
	    if (pedantic && sym->declared == 0)
	      if ((sym->type & SYMBOL_DEFINED) > 0 || sym->used > -1)
		errorLine (0, NULL, ErrorWarning, TRUE, "Symbol %s is implicitly imported", sym->str);
	    fwrite (sym->str, 1, sym->len + 1, outfile);
	  }
      }

}

void
symbolSymbolOutput (FILE * outfile)
{
  int i;
  Symbol *sym;

  for (i = 0; i < SYMBOL_TABLESIZE; i++)
    for (sym = symbolTable[i]; sym; sym = sym->next)
      if (!(sym->type & SYMBOL_AREA) && SYMBOL_OUTPUT (sym) /*sym->used >= 0 */ )
	{
	  AofSymbol asym;
	  asym.Name = sym->offset;
	  if (sym->type & SYMBOL_DEFINED)
	    {
	      int v;
	      Value value;
	      if (sym->value.Tag.t == ValueCode)
		{
		  codeInit ();
		  value = codeEvalLow (ValueAll, sym->value.ValueCode.len, sym->value.ValueCode.c);
		}
	      else
		value = sym->value;

	      switch (value.Tag.t)
		{
		case ValueIllegal:
		  errorLine (0, NULL, ErrorError, TRUE, "Symbol %s cannot be evaluated", sym->str);
		  v = 0;
		  break;
		case ValueInt:
		case ValueAddr:	/* nasty hack */
		  v = value.ValueInt.i;
		  break;
		case ValueFloat:
		  errorLine (0, NULL, ErrorError, TRUE, "Linker does not understand float constants (%s)", sym->str);
		  v = (int) value.ValueFloat.f;
		  break;
		case ValueString:
		  v = lexChar2Int (FALSE, value.ValueString.len, value.ValueString.s);
		  break;
		case ValueBool:
		  v = value.ValueBool.b;
		  break;
		case ValueCode:
		  errorLine (0, NULL, ErrorError, TRUE, "Linker does not understand code constants (%s)", sym->str);
		  v = 0;
		  break;
		case ValueLateLabel:
		  if (!value.ValueLate.late->next &&	/* Only one late label */
		      value.ValueLate.late->factor == 1 &&	/* ... occuring one time */
		      value.ValueLate.late->symbol->type & SYMBOL_AREA)
		    {		/* ... and it is an area */
		      if (sym->type & SYMBOL_ABSOLUTE)
			{	/* Change absolute to relative */
			  sym->type &= ~SYMBOL_ABSOLUTE;
			  v = value.ValueLate.i;
			  sym->area.ptr = value.ValueLate.late->symbol;
			}
		      else if (sym->area.ptr != value.ValueLate.late->symbol)
		        {
			  errorLine (0, NULL, ErrorError, TRUE, "Linker cannot have 2 areas for the same symbol (%s)", sym->str);
			  v = 0;
			}
		      else
		        v = 0;
		    }
		  else
		    {
		      errorLine (0, NULL, ErrorError, TRUE, "Linker cannot have many late labels for the same symbol (%s)", sym->str);
		      v = 0;
		    }
		  break;
		default:
		  errorLine (0, NULL, ErrorSerious, FALSE, "Internal symbolSymbolOutput: not possible (%s) (0x%x)", sym->str, value.Tag.t);
		  v = 0;
		  break;
		}
	      asym.Value = v;
	      if ((asym.Type = sym->type) & SYMBOL_ABSOLUTE)
		asym.AreaName = 0;
	      else
		asym.AreaName = sym->area.ptr->offset;
	    }
	  else
	    {
	      asym.Type = sym->type | TYPE_REFERENCE;
	      asym.Value = (sym->type & SYMBOL_COMMON) ? sym->value.ValueInt.i : 0;
	      asym.AreaName = 0;
	    }
          asym.Name     = armword (asym.Name);
          asym.Type     = armword (asym.Type & SYMBOL_SUPPORTEDBITS);
          asym.Value    = armword (asym.Value);
          asym.AreaName = armword(asym.AreaName);
	  fwrite (&asym, sizeof (AofSymbol), 1, outfile);
	}
      else if (sym->type & SYMBOL_AREA)
	{
	  AofSymbol asym;
	  asym.Name = armword (sym->offset);
	  asym.Type = armword (SYMBOL_KIND(sym->type) | SYMBOL_LOCAL);
	  asym.Value = armword (0);
	  asym.AreaName = armword (sym->offset);
	  fwrite (&asym, sizeof (AofSymbol), 1, outfile);
	}
}

#ifndef NO_ELF_SUPPORT
static int
findAreaIndex (struct AREA * area)
{
  Symbol *ap;
  int i=3;

  for (ap=areaHead; ap; ap=ap->area.info->next)
    {
      if (area == (struct AREA *)ap)
        return i;
      i++;
      if (ap->area.info->norelocs > 0)
        i++; /* We will be outputting a relocation section as well */
    }

  return -1;
}

void
symbolSymbolElfOutput (FILE * outfile)
{
  int i;
  int bind, type;
  Symbol *sym;
  Value value;
  int v = 0;
  Elf32_Sym asym;

  /* Output the undefined symbol */
  asym.st_name=0;
  asym.st_value=0;
  asym.st_size=0;
  asym.st_info=0;
  asym.st_other=0;
  asym.st_shndx=0;

  fwrite (&asym, sizeof (Elf32_Sym), 1, outfile);

  for (i = 0; i < SYMBOL_TABLESIZE; i++)
    for (sym = symbolTable[i]; sym; sym = sym-> next)
      if (!(sym->type & SYMBOL_AREA) && SYMBOL_OUTPUT (sym) /*sym->used >= 0 */ )
        {
          asym.st_name = sym->offset - 3;
          type = 0;
          bind = 0;
          if (sym->type & SYMBOL_DEFINED)
            {
              if (sym->value.Tag.t == ValueCode)
                {
                  codeInit ();
                  value = codeEvalLow (ValueAll, sym->value.ValueCode.len, sym->value.ValueCode.c);
                  type = STT_NOTYPE; /* No information to base type on */
                }
              else
                {
                  value = sym->value;
                }
              switch (value.Tag.t)
                {
                case ValueIllegal:
                  errorLine (0, NULL, ErrorError, TRUE, "Symbol %s cannot be evaluated", sym->str);
                  v = 0;
                  break;
                case ValueInt:
                case ValueAddr: /* nasty hack */
                  v = value.ValueInt.i;
                  break;
                case ValueFloat:
                  errorLine (0, NULL, ErrorError, TRUE, "Linker does not understand float constants (%s)", sym->str);
                  v = (int) value.ValueFloat.f;
                  break;
                case ValueString:
                  v = lexChar2Int (FALSE, value.ValueString.len, value.ValueString.s);
                  break;
                case ValueBool:
                  v = value.ValueBool.b;
                  break;
                case ValueCode:
                  errorLine (0, NULL, ErrorError, TRUE, "Linker does not understand code constants (%s)", sym->str);
                  v = 0;
                  break;
                case ValueLateLabel:
                  if (!value.ValueLate.late->next &&    /* Only one late label */
                      value.ValueLate.late->factor == 1 &&      /* ... occuring
one time */
                      value.ValueLate.late->symbol->type & SYMBOL_AREA)
                    {           /* ... and it is an area */
                      if (sym->type & SYMBOL_ABSOLUTE)
                        {       /* Change absolute to relative */
                          sym->type &= ~SYMBOL_ABSOLUTE;
                          v = value.ValueLate.i;
                          sym->area.ptr = value.ValueLate.late->symbol;
                        }
                      else if (sym->area.ptr != value.ValueLate.late->symbol)
                        errorLine (0, NULL, ErrorError, TRUE, "Linker cannot have 2 areas for the same symbol (%s)", sym->str);
                    }
                  else
                    {
                      errorLine (0, NULL, ErrorError, TRUE, "Linker cannot have many late labels for the same symbol (%s)", sym->str);
                    }
                  break;
                default:
                  errorLine (0, NULL, ErrorSerious, FALSE, "Internal symbolSymbolOutput: not possible (%s) (0x%x)", sym->str, value.Tag.t);
                  break;
                }
              asym.st_value = v;
              if (sym->type & SYMBOL_ABSOLUTE)
                {
                  asym.st_shndx = 0;
                }
              else
                /* Inefficient, needs fixing later */
                asym.st_shndx = findAreaIndex(sym->area.info);
            }
          else
            {
              asym.st_value = 0;
              asym.st_shndx = 0;
            }

          if (SYMBOL_KIND(sym->type) == TYPE_LOCAL)
            bind = STB_LOCAL;

          if (SYMBOL_KIND(sym->type) == TYPE_GLOBAL ||
              SYMBOL_KIND(sym->type) == TYPE_REFERENCE)
            bind = STB_GLOBAL;

          if (SYMBOL_KIND(sym->type) == TYPE_REFERENCE &&
              (sym->type & TYPE_WEAK) > 0)
            bind = STB_WEAK;

          asym.st_info = ELF32_ST_INFO(bind, type);
          fwrite (&asym, sizeof (Elf32_Sym), 1, outfile);
        }
      else if (sym->type & SYMBOL_AREA)
	{
	  type = (sym->type & SYMBOL_GLOBAL)?STB_GLOBAL:STB_LOCAL;
	  asym.st_info = ELF32_ST_INFO(type, STT_SECTION);
	  asym.st_name = sym->offset - 3;
	  asym.st_value = 0;
	  asym.st_shndx = findAreaIndex ((struct AREA *)sym);
	  fwrite (&asym, sizeof (Elf32_Sym), 1, outfile);
	}
}
#endif