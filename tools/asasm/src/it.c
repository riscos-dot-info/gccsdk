/*
 * AS an assembler for ARM
 * Copyright (c) 2012-2013 GCCSDK Developers
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
 * IT support.
 */

#include "config.h"

#include <stdbool.h>
#include <string.h>

#include "area.h"
#include "error.h"
#include "filestack.h"
#include "input.h"
#include "it.h"
#include "option.h"
#include "put.h"
#include "state.h"
#include "symbol.h"
#include "targetcpu.h"

void
IT_PrepareForPhase (Phase_e phase)
{
  switch (phase)
    {
      case ePassTwo:
	{
	  for (Symbol *ap = areaHeadSymbol; ap != NULL; ap = ap->area->next)
	    {
	      IT_State_t *itStateP = &ap->area->it;
	      if (itStateP->curIdx != itStateP->maxIdx)
		{
		  /* We have a pending IT block.  */
		  Error (ErrorError, "There is a pending IT block at end of area %s", ap->str);
		  Error_Line (itStateP->fileName, itStateP->lineNum, ErrorError, "note: Pending IT block started here");
		}
	      /* Reset any implicit IT block.  */
	      IT_InitializeState (&ap->area->it);
	    }
	  break;
	}

      case eStartUp:
      case ePassOne:
      case eOutput:
      case eCleanUp:
	break;
    }
}


/**
 * Emits IT instruction in Thumb code.
 */
static void
IT_EmitIT (const IT_State_t *state, bool isThumb)
{
  /* Only in Thumb state we're outputing an instruction.  */
  if (!isThumb)
    return;

  /* IT Thumb instruction is only available in ARMv6T2 and ARMv7.  In ARM
     mode it is only used for consistency check so only check for ARMv6T2/ARMv7
     in Thumb mode.
     Pre-ARMv6T2, only branch Thumb instructions can be conditional.  */
  Target_CheckCPUFeature (kCPUExt_v6T2, true);

  assert(state->isThen[0]);
  uint32_t cc = state->cc >> (28 - 4);
  uint32_t mask = 0x8;
  for (unsigned cond = state->maxIdx; cond != 1; --cond)
    mask = ((((cc & 0x10) == 0) ^ state->isThen[cond - 1]) << 3) | (mask >> 1);
  Put_InsWithOffset (state->areaCurIdx, 2, 0xBF00 | cc | mask);
}


/**
 * Implements IT.
 *   IT{<x>{<y>{<z>}}}{<q>} <firstcond>
 */
bool
m_it (bool doLowerCase)
{
  IT_State_t newIT =
    {
      .isThen = { true, false, false, false },
      .maxIdx = 1,
      .curIdx = 0,
      .implicitIT = false,
      /* .cc = ..., */
      .areaCurIdx = areaCurrentSymbol->area->curIdx,
      .fileName = FS_GetCurFileName(),
      .lineNum = FS_GetCurLineNumber()
    };

  for (; newIT.maxIdx != 4; ++newIT.maxIdx)
    {
      if (Input_Match (doLowerCase ? 't' : 'T', false))
	newIT.isThen[newIT.maxIdx] = true;
      else if (Input_Match (doLowerCase ? 'e' : 'E', false))
	newIT.isThen[newIT.maxIdx] = false;
      else
	break;
    }
  InstrWidth_e instrWidth = Option_GetInstrWidth (doLowerCase);
  if (instrWidth == eInstrWidth_Unrecognized)
    return true;

  if (instrWidth == eInstrWidth_Enforce32bit)
    Error (ErrorWarning, "Wide variant of IT does not exist");

  Input_SkipWS ();
  newIT.cc = Option_GetCCodeIfThere (false); /* Condition code is an argument
    here, so always uppercase.  */
  if (newIT.cc == kOption_NotRecognized)
    {
      Error (ErrorError, "Unrecognized or missing condition code");
      return false;
    }

  Area *curAreaP = areaCurrentSymbol->area;
  if (curAreaP->it.curIdx != curAreaP->it.maxIdx)
    {
      /* We have a pending IT block.  */
      Error (ErrorError, "There is a pending IT block");
      Error_Line (curAreaP->it.fileName, curAreaP->it.lineNum, ErrorError, "note: Pending IT block started here");
    }

  /* We've already warned about the use of NV.
     Here, warn about using AL and having numCond non-zero.  */
  if (newIT.cc == AL && newIT.maxIdx != 1)
    Error (ErrorWarning, "Use of AL condition with one or more Then/Else arguments is UNPREDICTABLE");

  curAreaP->it = newIT;
  IT_EmitIT (&curAreaP->it, State_GetInstrType () != eInstrType_ARM);

  return false;
}


/**
 * Called to initialize the IT state data located in each Area structure.
 */
void
IT_InitializeState (IT_State_t *state)
{
  memset (state, 0, sizeof (IT_State_t));
  state->cc = AL;
}


static void
IT_StartImplicitIT (IT_State_t *itStateP, uint32_t cc)
{
  const IT_State_t newIT =
    {
      .isThen = { true, false, false, false },
      .maxIdx = 1,
      .curIdx = 1,
      .implicitIT = true,
      .cc = cc,
      .areaCurIdx = areaCurrentSymbol->area->curIdx,
      .fileName = FS_GetCurFileName(),
      .lineNum = FS_GetCurLineNumber()
    };
  *itStateP = newIT;
}


/**
 * To be called for each ARM and Thumb instruction with its condition code
 * and whether it is ARM or Thumb.
 * When we have a pending IT block, the condition code will be verified.
 * When we don't have a pending IT block and it is a Thumb instruction, an
 * implicit IT instruction is emited.  If it's an ARM instruction, no IT
 * instruction is emited.
 */
void
IT_ApplyCond (uint32_t cc, bool isThumb)
{
  cc &= NV; /* Filter out the condition codes.  */
  IT_State_t *itStateP = &areaCurrentSymbol->area->it;
  if (itStateP->curIdx != itStateP->maxIdx)
    {
      /* Pending IT block.  */
      assert (!itStateP->implicitIT);
      uint32_t expectedCC = itStateP->cc ^ (!itStateP->isThen[itStateP->curIdx] << 28);
      if (cc != expectedCC)
	{
	  Error (ErrorError, "Condition code does not match with pending IT block");
	  Error_Line (itStateP->fileName, itStateP->lineNum, ErrorError, "note: Pending IT block started here");
	}
      itStateP->curIdx++;
    }
  else if (itStateP->implicitIT && itStateP->maxIdx != 4)
    {
      /* If it is not Thumb, we've switched to ARM code while we still have
         a pending implicit IT block.  */
      if (!isThumb || cc == AL)
	IT_InitializeState (itStateP);
      else
	{
	  if (cc == itStateP->cc)
	    {
	      /* Add 'Then' case.  */
	      itStateP->isThen[itStateP->curIdx] = true;
	      itStateP->curIdx++;
	      itStateP->maxIdx++;
	    }
	  else if (cc == (itStateP->cc ^ (1<<28)))
	    {
	      /* Add 'Else' case.  */
	      itStateP->isThen[itStateP->curIdx] = false;
	      itStateP->curIdx++;
	      itStateP->maxIdx++;
	    }
	  else
	    IT_StartImplicitIT (itStateP, cc);
	  IT_EmitIT (itStateP, true);
	}
    }
  else if (isThumb && cc != AL)
    {
      /* Start a new implicit IT block.  */
      IT_StartImplicitIT (itStateP, cc);
      IT_EmitIT (itStateP, true);
    }
}