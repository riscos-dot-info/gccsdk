/*
 * AS an assembler for ARM
 * Copyright � 1992 Niklas R�jemo
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
#include <stdint.h>
#elif HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "error.h"
#include "expr.h"
#include "get.h"
#include "help_cop.h"
#include "input.h"
#include "mnemonics.h"
#include "put.h"
#include "main.h"

static void
dstmem (WORD ir)
{
  if (apcs_softfloat) error (ErrorWarning, TRUE, "soft-float code uses hard FP instructions");
    
  ir |= DST_OP (getFpuReg ());
  ir = help_copAddr (ir, FALSE);
  putIns (ir);
}

void
m_stf (WORD cc)
{
  dstmem (0x0c000100 | cc);
}

void
m_ldf (WORD cc)
{
  dstmem (0x0c100100 | cc);
}


static void
dstmemx (WORD ir)
{
  BOOL stack_ia = FALSE;
  BOOL stack;
  Value im;

  if (apcs_softfloat) error (ErrorWarning, TRUE, "soft-float code uses hard FP instructions");

  stack = (BOOL) ! isspace (inputLook ());
  if (stack)
    {
      char c1, c2;
      c1 = toupper (inputLook ());
      c2 = toupper (inputLookN (1));
      if (c1 == 'D' && c2 == 'B')
	stack_ia = FALSE;
      else if (c1 == 'I' && c2 == 'A')
	stack_ia = TRUE;
      else if (c1 == 'E' && c2 == 'A')
	stack_ia = (ir & 0x100000) ? FALSE : TRUE;
      else if (c1 == 'F' && c2 == 'D')
	stack_ia = (ir & 0x100000) ? TRUE : FALSE;
      else
	error (ErrorError, TRUE, "Illegal stack type for %cfm (%c%c)",
	       (ir & 0x100000) ? 'l' : 's', c1, c2);
      inputSkipN (2);
      if (stack_ia)
	ir |= 0x800000;
    }
  if (inputLook () && !isspace (inputLook ()))
    {
      inputRollback ();
      error (ErrorError, FALSE, "Illegal line \"%s\"", inputRest ());
      return;
    }
  skipblanks ();
  ir |= DST_OP (getFpuReg ());
  skipblanks ();
  if (inputLook () == ',')
    inputSkip ();
  else
    error (ErrorError, TRUE, "Inserting comma after dst");
  skipblanks ();
  exprBuild ();
  im = exprEval (ValueInt);
  if (im.Tag.t == ValueInt)
    {
      if (im.ValueInt.i < 1 || im.ValueInt.i > 4)
	error (ErrorError, TRUE, "Number of fp registers out of range");
    }
  else
    error (ErrorError, TRUE, "Illegal %cfm expression", (ir & 0x100000) ? 'l' : 's');
  ir |= (im.ValueInt.i & 1) << 15;
  ir |= (im.ValueInt.i & 2) << 21;
  if (stack)
    ir |= stack_ia ? 0x800000 : 0x1000000;
  ir = help_copAddr (ir, stack);
  if (stack && (!stack_ia || (ir & 0x200000)))
    ir |= 3 * im.ValueInt.i;
  putIns (ir);
}

void
m_sfm (WORD cc)
{
  dstmemx (0x0c000200 | cc);
}

void
m_lfm (WORD cc)
{
  dstmemx (0x0c100200 | cc);
}