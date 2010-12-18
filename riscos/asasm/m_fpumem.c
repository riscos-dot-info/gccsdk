/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2000-2010 GCCSDK Developersrs
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
 * m_fpumem.c
 */

#include "config.h"
#include <ctype.h>
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#include "error.h"
#include "expr.h"
#include "get.h"
#include "help_cop.h"
#include "input.h"
#include "m_fpumem.h"
#include "option.h"
#include "put.h"
#include "main.h"

static void
dstmem (ARMWord ir)
{
  if (option_apcs_softfloat)
    error (ErrorWarning, "soft-float code uses hard FP instructions");
    
  ir |= DST_OP (getFpuReg ());
  ir = help_copAddr (ir, false);
  putIns (ir);
}

/**
 * Implements STF.
 */
bool
m_stf (void)
{
  ARMWord cc = optionCondPrec_P ();
  if (cc == optionError)
    return true;
  dstmem (0x0c000100 | cc);
  return false;
}

/**
 * Implements LDF.
 */
bool
m_ldf (void)
{
  ARMWord cc = optionCondPrec_P ();
  if (cc == optionError)
    return true;
  dstmem (0x0c100100 | cc);
  return false;
}


static void
dstmemx (ARMWord ir)
{
  if (option_apcs_softfloat)
    error (ErrorWarning, "soft-float code uses hard FP instructions");

  const char * const inputMark = Input_GetMark ();
  bool stack_ia = false;
  bool stack = !isspace ((unsigned char)inputLook ());
  if (stack)
    {
      char c1 = toupper (inputLook ());
      char c2 = toupper (inputLookN (1));
      if (c1 == 'D' && c2 == 'B')
	stack_ia = false;
      else if (c1 == 'I' && c2 == 'A')
	stack_ia = true;
      else if (c1 == 'E' && c2 == 'A')
	stack_ia = (ir & 0x100000) ? false : true;
      else if (c1 == 'F' && c2 == 'D')
	stack_ia = (ir & 0x100000) ? true : false;
      else
	error (ErrorError, "Illegal stack type for %cfm (%c%c)",
	       (ir & 0x100000) ? 'l' : 's', c1, c2);
      inputSkipN (2);
      if (stack_ia)
	ir |= 0x800000;
    }
  if (inputLook () && !isspace ((unsigned char)inputLook ()))
    {
      errorAbort ("Can't parse \"%s\" of SFM/LFM", inputMark);
      return;
    }
  skipblanks ();
  ir |= DST_OP (getFpuReg ());
  skipblanks ();
  if (!Input_Match (',', false))
    error (ErrorError, "Inserting comma after dst");
  const Value *im = exprBuildAndEval (ValueInt);
  if (im->Tag == ValueInt)
    {
      if (im->Data.Int.i < 1 || im->Data.Int.i > 4)
	error (ErrorError, "Number of fp registers out of range");
    }
  else
    error (ErrorError, "Illegal %cfm expression", (ir & 0x100000) ? 'l' : 's');
  ir |= (im->Data.Int.i & 1) << 15;
  ir |= (im->Data.Int.i & 2) << 21;
  if (stack)
    ir |= stack_ia ? 0x800000 : 0x1000000;
  ir = help_copAddr (ir, stack);
  if (stack && (!stack_ia || (ir & 0x200000)))
    ir |= 3 * im->Data.Int.i;
  putIns (ir);
}

/**
 * Implements SFM.
 */
bool
m_sfm (void)
{
  ARMWord cc = optionCondLfmSfm ();
  if (cc == optionError)
    return true;
  dstmemx (0x0c000200 | cc);
  return false;
}

/**
 * Implements LFM.
 */
bool
m_lfm (void)
{
  ARMWord cc = optionCondLfmSfm ();
  if (cc == optionError)
    return true;
  dstmemx (0x0c100200 | cc);
  return false;
}