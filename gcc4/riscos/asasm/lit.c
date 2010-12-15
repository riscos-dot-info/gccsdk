/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2000-2010 GCCSDK Developers
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
 * lit.c
 */

#include "config.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
#endif
#include <string.h>

#include "area.h"
#include "code.h"
#include "commands.h"
#include "error.h"
#include "filestack.h"
#include "help_cpu.h"
#include "lit.h"
#include "main.h"
#include "reloc.h"
#include "value.h"

/**
 * Returns a symbol which will be a ValueInt being the offset where the
 * given literal will be assembled.
 */
static Symbol *
Lit_GetOffsetAsSymbol (const LitPool *literal)
{
  char intSymbol[48];
  snprintf (intSymbol, sizeof (intSymbol), "$$$$$$$Lit$%p", (void *)literal);
  const Lex lex = lexTempLabel (intSymbol, strlen (intSymbol));
  return symbolGet (&lex);
}

/**
 * Registers a literal integer for the current area and returns either a
 * ValueAddr of a previous (but same) literal integer, or ValueSymbol
 * representing the label where the literal will get assembled.
 * \return You get ownership.
 */
Value
Lit_RegisterInt (const Value *value, Lit_eSize size)
{
  bool isAddrMode3;
  switch (size)
    {
      case eLitIntUByte:
      case eLitIntSByte:
      case eLitIntUHalfWord:
      case eLitIntSHalfWord:
	isAddrMode3 = true;
	break;
      default:
	isAddrMode3 = false;
	break;
    }

  /* Check if we already have the literal assembled in an range of up to
     4095 (address mode 2) or 255 (address mode 3) bytes ago.  */
  for (const LitPool *litPoolP = areaCurrentSymbol->area.info->litPool;
       litPoolP != NULL;
       litPoolP = litPoolP->next)
    {
      int offset;
      bool equal;
      if (litPoolP->value.Tag == ValueInt && value->Tag == ValueInt)
	{
	  switch (size)
	    {
	      case eLitIntUByte:
	      case eLitIntSByte:
		if (((litPoolP->value.Data.Int.i >> 0) & 0xFF) == (value->Data.Int.i & 0xFF))
		  {
		    offset = 0;
		    equal = true;
		  }
		else if (((litPoolP->value.Data.Int.i >> 8) & 0xFF) == (value->Data.Int.i & 0xFF))
		  {
		    offset = 1;
		    equal = true;
		  }
		else if (((litPoolP->value.Data.Int.i >> 16) & 0xFF) == (value->Data.Int.i & 0xFF))
		  {
		    offset = 2;
		    equal = true;
		  }
		else if (((litPoolP->value.Data.Int.i >> 24) & 0xFF) == (value->Data.Int.i & 0xFF))
		  {
		    offset = 3;
		    equal = true;
		  }
		else
		  equal = false;
		break;

	      case eLitIntUHalfWord:
	      case eLitIntSHalfWord:
		if (((litPoolP->value.Data.Int.i >> 0) & 0xFFFF) == (value->Data.Int.i & 0xFFFF))
		  {
		    offset = 0;
		    equal = true;
		  }
		else if (((litPoolP->value.Data.Int.i >> 16) & 0xFFFF) == (value->Data.Int.i & 0xFFFF))
		  {
		    offset = 2;
		    equal = true;
		  }
		else
		  equal = false;
		break;

	      case eLitIntWord:
		offset = 0;
		equal = litPoolP->value.Data.Int.i == value->Data.Int.i;
		break;

	      default:
		equal = false;
		break;
	    }
	}
      else
	{
	  offset = 0;
	  equal = valueEqual (&litPoolP->value, value);
	}
      if (equal)
	{
	  Symbol *symP = Lit_GetOffsetAsSymbol (litPoolP);
	  assert (((symP->type & SYMBOL_DEFINED) != 0) ^ (!litPoolP->gotAssembled));
	  if ((symP->type & SYMBOL_DEFINED) != 0)
	    {
	      assert (symP->value.Tag == ValueInt || /* symP->value.Tag == ValueFloat || */ symP->value.Tag == ValueCode);
	      assert (areaCurrentSymbol->value.Data.Int.i >= litPoolP->offset);
	      if (areaCurrentSymbol->value.Data.Int.i + 8 > litPoolP->offset + offset + ((isAddrMode3) ? 255 : 4095)
	          || (size == eLitFloat && litPoolP->size != eLitFloat)
	          || (size == eLitDouble && litPoolP->size != eLitDouble))
		continue;
	      /* A literal with the same value got already assembled and is in
	         our range to refer to.  */
	      return Value_Addr (15, litPoolP->offset + offset - (areaCurrentSymbol->value.Data.Int.i + 8));
	    }
	  else
	    {
	      /* A literal with the same value was already needed so we can
	         reuse its position where it is going to be assembled.  */
	      if (offset == 0)
		return Value_Symbol (symP, 1);
	      const Code code[] =
		{
		    { .Tag = CodeValue, .Data.value = { .Tag = ValueSymbol, .Data.Symbol = { .factor = 1, .symbol = symP } } },
		    { .Tag = CodeValue, .Data.value = { .Tag = ValueInt, .Data.Int = { .i = offset } } },
		    { .Tag = CodeOperator, .Data.op = Op_add }
		};
	      const Value valCode = Value_Code (sizeof (code)/sizeof (code[0]), code);
	      Value rtrn = { .Tag = ValueIllegal };
	      Value_Assign (&rtrn, &valCode);
	      return rtrn;
	    }
	}
    }

  /* We need to create a new literal in our pool.  */
  LitPool *litP;
  if ((litP = malloc (sizeof (LitPool))) == NULL)
    errorOutOfMem ();
  litP->next = areaCurrentSymbol->area.info->litPool;
  areaCurrentSymbol->area.info->litPool = litP;
  litP->file = FS_GetCurFileName ();
  litP->lineno = FS_GetCurLineNumber ();
  litP->offset = 0;
  litP->value.Tag = ValueIllegal;
  Value_Assign (&litP->value, value);
  litP->size = size;
  litP->gotAssembled = false;

  return Value_Symbol (Lit_GetOffsetAsSymbol (litP), 1);
}

/**
 * Dump (assemble) the literal pool of the current area.
 */
void
Lit_DumpPool (void)
{
  /* Delink unassembled literal pool elements and add them at the end
     (in chronological order).  */
  LitPool *unAsmLitP = NULL;
  assert (offsetof (LitPool, next) == 0);
  LitPool *prevLitP = (LitPool *)&areaCurrentSymbol->area.info->litPool;
  for (LitPool *litP = prevLitP->next; litP != NULL; /* */)
    {
      LitPool * const nextLitP = litP->next;
      if (!litP->gotAssembled)
	{
	  litP->next = unAsmLitP;
	  unAsmLitP = litP;
	  prevLitP->next = nextLitP;
	}
      else
	prevLitP = litP;
      litP = nextLitP;
    }
  prevLitP->next = unAsmLitP;

  for (LitPool *litP = unAsmLitP; litP != NULL; litP = litP->next)
    {
      assert (!litP->gotAssembled);
      litP->gotAssembled = true;

      codeInit ();
      if (litP->value.Tag == ValueCode)
	Code_AddValueCode (&litP->value);
      else
	codeValue (&litP->value);

      Symbol *symP = Lit_GetOffsetAsSymbol (litP);
      symP->type |= SYMBOL_DEFINED | SYMBOL_DECLARED;

      /* Check if it is a fixed integer/float which fits an immediate
         representation.  */
      const Value *constValueP = codeEval (ValueInt | ValueFloat, NULL);
      if (constValueP->Tag == ValueInt)
	{
	  bool isImmediate;
	  int constant;
	  switch (litP->size)
	    {
	      case eLitIntUByte:
		constant = (int)(uint8_t)constValueP->Data.Int.i;
		isImmediate = true;
		break;

	      case eLitIntSByte:
		constant = (int)(int8_t)constValueP->Data.Int.i;
		isImmediate = true;
		break;

	      case eLitIntUHalfWord:
		constant = (int)(uint16_t)constValueP->Data.Int.i;
		isImmediate = true;
		break;
		
	      case eLitIntSHalfWord:
		constant = (int)(int16_t)constValueP->Data.Int.i;
		isImmediate = true;
		break;
		
	      case eLitIntWord:
		constant = constValueP->Data.Int.i;
		isImmediate = true;
		break;

	      default:
		isImmediate = false;
		break;
	    }

	  /* Value representable using MOV or MVN ? */
	  if (isImmediate
	      && (help_cpuImm8s4 (constant) != -1 || help_cpuImm8s4 (~constant) != -1))
	    {
	      symP->value = Value_Int (constant);
	      continue;
	    }
	}
      else if (constValueP->Tag == ValueFloat)
	{
	  /* FIXME: if immedate -> symP->value is ValueFloat + isImmediate = true */
	}

      /* Ensure alignment.  */
      switch (litP->size)
	{
	  case eLitIntUHalfWord:
	  case eLitIntSHalfWord:
	    Area_AlignTo (2, NULL);
	    break;

	  case eLitIntWord:
	    Area_AlignTo (4, NULL);
	    break;

	  default:
	    break;
	}
      
      /* The constant is not representable as an Immediate constant.  So we
         have to assemble it and refer to it via a ValueSymbol(current area)
	 + ValueInt(offset where literal gets assembled).
	 The ValueSymbol will be translated to a ValueAddr(pc) at a later
	 stage.  */
      const Code code[] =
	{
	  { .Tag = CodeValue, .Data.value = { .Tag = ValueSymbol, .Data.Symbol = { .factor = 1, .symbol = areaCurrentSymbol } } },
	  { .Tag = CodeValue, .Data.value = { .Tag = ValueInt, .Data.Int = { .i = areaCurrentSymbol->value.Data.Int.i } } },
	  { .Tag = CodeOperator, .Data.op = Op_add }
	};
      const Value valCode = Value_Code (sizeof (code)/sizeof (code[0]), code);
      Value_Assign (&symP->value, &valCode);

      litP->offset = areaCurrentSymbol->value.Data.Int.i;

      switch (litP->size)
	{
	  case eLitIntUByte:
	  case eLitIntSByte:
	  case eLitIntUHalfWord:
	  case eLitIntSHalfWord:
	  case eLitIntWord:
	    {
	      int size;
	      switch (litP->size)
		{
		  case eLitIntUByte:
		  case eLitIntSByte:
		    size = 1;
		    break;
		  case eLitIntUHalfWord:
		  case eLitIntSHalfWord:
		    size = 2;
		    break;
		  case eLitIntWord:
		    size = 4;
		    break;
		}
	      if (Reloc_QueueExprUpdate (DefineInt_RelocUpdater, areaCurrentSymbol->value.Data.Int.i,
					 ValueInt | ValueString | ValueSymbol | ValueCode,
					 &size, sizeof (size)))
		errorLine (litP->file, litP->lineno, ErrorError, "Illegal %s expression", "literal");
	    }
	    break;

	  case eLitFloat:
	  case eLitDouble:
	    {
	      int size = litP->size == eLitFloat ? 4 : 8;
	      ValueTag legal = ValueFloat | ValueSymbol | ValueCode;
	      if (option_autocast)
		legal |= ValueInt;
	      if (Reloc_QueueExprUpdate (DefineReal_RelocUpdater, areaCurrentSymbol->value.Data.Int.i,
					 legal, &size, sizeof (size)))
		errorLine (litP->file, litP->lineno, ErrorError, "Illegal %s expression", "literal");
	    }
	    break;

	  default:
	    assert (0);
	    break;
	}
    }
  Area_AlignTo (4, NULL);
}