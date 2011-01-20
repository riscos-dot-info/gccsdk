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
 * m_copmem.c
 */

#include "config.h"
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#include "error.h"
#include "get.h"
#include "help_cop.h"
#include "input.h"
#include "m_copmem.h"
#include "option.h"
#include "put.h"
#include "targetcpu.h"


static void
dstmem (ARMWord ir)
{
  ir |= CP_NUMBER (getCopNum ());
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%scoprocessor number", InsertCommaAfter);
  ir |= CPDST_OP (getCopReg ());
  ir = help_copAddr (ir, false);
  Put_Ins (ir);
}

/**
 * Implements LDC.
 * LDC{<cond>}{L} <coproc>, <CRd>, <addressing mode 5>
 */
bool
m_ldc (void)
{
  ARMWord cc = optionCondL ();
  if (cc == optionError)
    return true;
  dstmem (cc | 0x0c100000);
  return false;
}

/**
 * Implements LDC2.
 * LDC2{L} <coproc>, <CRd>, <addressing mode 5>
 */
bool
m_ldc2 (void)
{
  ARMWord cc = Input_Match ('L', false) ? N_FLAG : 0;
  if (!Input_IsEndOfKeyword ())
    return true;

  Target_NeedAtLeastArch (ARCH_ARMv5);
  dstmem (cc | 0x0c100000 | NV);
  return false;
}

/**
 * Implements STC.
 * STC{<cond>}{L} <coproc>, <CRd>, <addressing mode 5>
 */
bool
m_stc (void)
{
  ARMWord cc = optionCondL ();
  if (cc == optionError)
    return true;
  dstmem (cc | 0x0c000000);
  return false;
}

/**
 * Implements STC2.
 * STC2{L} <coproc>, <CRd>, <addressing mode 5>
 */
bool
m_stc2 (void)
{
  ARMWord cc = Input_Match ('L', false) ? N_FLAG : 0;
  if (!Input_IsEndOfKeyword ())
    return true;

  Target_NeedAtLeastArch (ARCH_ARMv5);
  dstmem (cc | 0x0c000000 | NV);
  return false;
}