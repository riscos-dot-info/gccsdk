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
 * help_cop.c
 */

#include "config.h"
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
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


/**
 * Pre-indexed:
 *   ", [Rx]"
 *   ", [Rx, #<expression>]"
 *   ", [Rx, #<expression>]!"
 * Post-indexed:
 *   ", [Rx], #<expression>"
 *   ", [Rx], {<expression>}" (ObjAsm compatibility)
 * Other:
 *   ", =<literal>"
 *   ", <label>"
 */
ARMWord
help_copAddr (ARMWord ir, bool literal, bool stack)
{
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "Inserting missing comma before address");
  switch (inputLook ())
    {
      case '[':
        {
          inputSkip ();
          skipblanks ();
          ir |= LHS_OP (getCpuReg ());	/* Base register */
          skipblanks ();
	  bool preIndexed = !Input_Match (']', true);
          bool offValue = false;
	  if (Input_Match (',', true))
	    {			/* either [base,XX] or [base],XX */
	      if (stack)
	        {
	          error (ErrorError, "Cannot specify both offset and stack type");
	          break;
	        }
	      if (Input_Match ('#', false))
	        {
	          const Value *offset = exprBuildAndEval (ValueInt);
	          offValue = true;
	          switch (offset->Tag)
		    {
		      case ValueInt:
		        ir = fixCopOffset (0, ir, offset->Data.Int.i);
		        break;
		      default:
		        error (ErrorError, "Illegal offset expression");
		        break;
		    }
	          if (!preIndexed)
		    ir |= W_FLAG; /* If postfix, set writeback */
	        }
	      else if (Input_Match ('{', false))
	        {
	          const Value *offset = exprBuildAndEval (ValueInt);
	          offValue = true;
	          if (offset->Tag != ValueInt)
	            error (ErrorError, "Illegal option value");
	          if (offset->Data.Int.i < -128 || offset->Data.Int.i > 256)
		    error (ErrorError, "Option value too large");
	          ir |= (offset->Data.Int.i & 0xFF) | U_FLAG;
	          skipblanks ();
		  if (!Input_Match ('}', false))
		    error (ErrorError, "Missing '}' in option");
	        }
	      else
	        error (ErrorError, "Coprocessor memory instructions cannot use register offset");
	      skipblanks ();
	    }
          else
	    {			/* cop_reg,[base] if this way */
	      if (preIndexed)
	        error (ErrorError, "Illegal character '%c' after base", inputLook ());
	      if (!stack)
	        ir |= U_FLAG;	/* changes #-0 to #+0 :-) */
	    }
          if (preIndexed)
	    {
	      if (!Input_Match (']', true))
	        error (ErrorError, "Inserting missing ] after address");
	    }
          else if (!stack && !offValue)
	    preIndexed = true;		/* make [base] into [base,#0] */
	  if (Input_Match ('!', true))
	    {
	      if (preIndexed || stack)
	        ir |= W_FLAG;
	      else
	        error (ErrorError, "Writeback is implied with post-index");
	    }
          else if (stack)
	    preIndexed = true;		/* [base] in stack context => [base,#0] */
          if (preIndexed)
	    ir |= P_FLAG;
	  break;
	}

      case '=':
	{
          inputSkip ();
          if (stack)
	    {
	      error (ErrorError, "Literal cannot be used when stack type is specified");
	      break;
	    }
          ir |= P_FLAG | LHS_OP (15);
          const Value *offset = exprBuildAndEval (ValueFloat /* | ValueLateLabel */);
	  Value real = *offset;
          switch (offset->Tag)
	    {
	      case ValueFloat:
	        /* note that litFloat==litInt, so there's no litFloat */
		switch (ir & PRECISION_MEM_PACKED)
	          {
	            case PRECISION_MEM_SINGLE:
	              Lit_RegisterInt (&real, eLitFloat); /* FIXME: use return value. */
	              break;
	            default:
	              error (ErrorWarning,
		             "Extended and packed not supported; using double");
	              ir = (ir & ~PRECISION_MEM_PACKED) | PRECISION_MEM_DOUBLE;
		      /* Fall through.  */
	            case PRECISION_MEM_DOUBLE:
	              Lit_RegisterInt (&real, eLitDouble); /* FIXME: use return value. */
	              break;
	          }
	        break;
	      default:
	        error (ErrorAbort, "Internal error: help_copAddr");
	        break;
	    }
	  break;
	}

      default:
	{
          if (stack)
	    {
	      error (ErrorError, "Address cannot be used when stack type is specified");
	      break;
	    }
          /*  cop_reg,Address */
          ir |= P_FLAG | LHS_OP (15);
          exprBuild ();
          codePosition (areaCurrentSymbol, areaCurrentSymbol->value.Data.Int.i);
          codeOperator (Op_sub);
          codeInt (8);
          codeOperator (Op_sub);
          const Value *offset = exprEval (ValueInt | ValueAddr);
          switch (offset->Tag)
	    {
	      case ValueInt:
	        ir = fixCopOffset (0, ir | LHS_OP (15), offset->Data.Int.i);
	        break;
	      case ValueAddr:
	        ir = fixCopOffset (0, ir | LHS_OP (offset->Data.Addr.r),
				   offset->Data.Addr.i);
	        break;
	      default:
	        error (ErrorError, "Illegal address expression");
	        break;
	    }
	  break;
	}
    }

  return ir;
}
