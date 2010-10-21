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
 * help_cop.c
 */

#include "config.h"
#ifdef HAVE_STDINT_H
#include <stdint.h>
#elif HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <stdlib.h>

#include "area.h"
#include "code.h"
#include "error.h"
#include "expr.h"
#include "fix.h"
#include "get.h"
#include "help_cop.h"
#include "input.h"
#include "option.h"
#include "reloc.h"
#include "value.h"

int
help_copInt (int max, const char *msg)
{
  exprBuild ();
  Value i = exprEval (ValueInt);
  if (i.Tag == ValueInt)
    {
      if (i.Data.Int.i < 0 || i.Data.Int.i > max)
	{
	  error (ErrorError, "%d is not a legal %s", i.Data.Int.i, msg);
	  return 0;
	}
    }
  else
    {
      error (ErrorError, "Illegal expression as %s", msg);
      return 0;
    }
  return i.Data.Int.i;
}


/**
 * Called for ",[Rx" ( ",#y]" [ "!" ] ) | "], #z"
 */
ARMWord
help_copAddr (ARMWord ir, bool stack)
{
  skipblanks ();
  if (inputLook () == ',')
    {
      inputSkip ();
      skipblanks ();
    }
  else
    error (ErrorError, "Inserting missing comma before address");
  switch (inputLook ())
    {
      case '[':
        {
          inputSkip ();
          skipblanks ();
          ir |= LHS_OP (getCpuReg ());	/* Base register */
          skipblanks ();
	  bool pre;
          if (inputLook () == ']')
	    {
	      pre = false;
	      inputSkip ();
	      skipblanks ();
	    }
          else
	    pre = true;
          bool offValue = false;
          if (inputLook () == ',')
	    {			/* either [base,XX] or [base],XX */
	      if (stack)
	        {
	          error (ErrorError, "Cannot specify both offset and stack type");
	          break;
	        }
	      inputSkip ();
	      skipblanks ();
	      if (inputLook () == '#')
	        {
	          inputSkip ();
	          exprBuild ();
	          Value offset = exprEval (ValueInt | ValueCode | ValueLateLabel);
	          offValue = true;
	          switch (offset.Tag)
		    {
		      case ValueInt:
		        ir = fixCopOffset (0, ir, offset.Data.Int.i);
		        break;
		      case ValueCode:
		      case ValueLateLabel:
		        relocCopOffset (ir, &offset);
		        break;
		      default:
		        error (ErrorError, "Illegal offset expression");
		        break;
		    }
	          if (!pre)
		    ir |= WB_FLAG; /* If postfix, set writeback */
	        }
	      else if (inputLook () == '{')
	        {
	          inputSkip ();
	          skipblanks ();
	          exprBuild ();
	          Value offset = exprEval (ValueInt);
	          offValue = true;
	          if (offset.Tag != ValueInt)
	            error (ErrorError, "Illegal option value");
	          if (offset.Data.Int.i < -128 || offset.Data.Int.i > 256)
		    error (ErrorError, "Option value too large");
	          ir |= (offset.Data.Int.i & 0xFF) | UP_FLAG;
	          skipblanks ();
	          if (inputLook () != '}')
		    error (ErrorError, "Missing '}' in option");
	          inputSkip ();
	        }
	      else
	        error (ErrorError, "Coprocessor memory instructions cannot use register offset");
	      skipblanks ();
	    }
          else
	    {			/* cop_reg,[base] if this way */
	      if (pre)
	        error (ErrorError, "Illegal character '%c' after base", inputLook ());
	      if (!stack)
	        ir |= UP_FLAG;	/* changes #-0 to #+0 :-) */
	    }
          if (pre)
	    {
	      if (inputLook () == ']')
	        {
	          inputSkip ();
	          skipblanks ();
	        }
	      else
	        error (ErrorError, "Inserting missing ] after address");
	    }
          else if (!stack && !offValue)
	    pre = true;		/* make [base] into [base,#0] */
	  if (inputLook () == '!')
	    {
	      if (pre || stack)
	        ir |= WB_FLAG;
	      else
	        error (ErrorError, "Writeback is implied with post-index");
	      inputSkip ();
	      skipblanks ();
	    }
          else if (stack)
	    pre = true;		/* [base] in stack context => [base,#0] */
          if (pre)
	    ir |= PRE_FLAG;
	}
        break;
      case '=':
	{
          inputSkip ();
          if (stack)
	    {
	      error (ErrorError, "Literal cannot be used when stack type is specified");
	      break;
	    }
          ir |= PRE_FLAG | LHS_OP (15);
          exprBuild ();
          Value offset = exprEval (ValueInt | ValueFloat | ValueLateLabel);
          switch (offset.Tag)
	    {
	      case ValueInt:
	        offset.Tag = ValueFloat;
	        offset.Data.Float.f = offset.Data.Int.i;
	        /* Fall through.  */
	      case ValueFloat:
	        /* note that litFloat==litInt, so there's no litFloat */
		switch (ir & PRECISION_MEM_PACKED)
	          {
	            case PRECISION_MEM_SINGLE:
	              litInt (4, &offset);
	              break;
	            default:
	              error (ErrorWarning,
		             "Extended and packed not supported; using double");
	              ir = (ir & ~PRECISION_MEM_PACKED) | PRECISION_MEM_DOUBLE;
		      /* Fall through.  */
	            case PRECISION_MEM_DOUBLE:
	              litInt (8, &offset);
	              break;
	          }
	        break;
	      default:
	        error (ErrorAbort, "Internal error: help_copAddr");
	        break;
	    }
	}
        break;
      default:
	{
          if (stack)
	    {
	      error (ErrorError, "Address cannot be used when stack type is specified");
	      break;
	    }
          /*  cop_reg,Address */
          ir |= PRE_FLAG | LHS_OP (15);
          exprBuild ();
          codePosition (areaCurrentSymbol);
          codeOperator (Op_sub);
          codeInt (8);
          codeOperator (Op_sub);
          Value offset = exprEval (ValueInt | ValueCode | ValueLateLabel | ValueAddr);
          switch (offset.Tag)
	    {
	      case ValueInt:
	        ir = fixCopOffset (0, ir | LHS_OP (15), offset.Data.Int.i);
	        break;
	      case ValueCode:
	      case ValueLateLabel:
	        relocCopOffset (ir | LHS_OP (15), &offset);
	        break;
	      case ValueAddr:
	        ir = fixCopOffset (0, ir | LHS_OP (offset.Data.Addr.r),
				   offset.Data.Addr.i);
	        break;
	      default:
	        error (ErrorError, "Illegal address expression");
	        break;
	    }
	}
        break;
    }

  return ir;
}
