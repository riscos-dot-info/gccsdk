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
 * m_cpumem.c
 */

#include "config.h"

#include <assert.h>
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#include "area.h"
#include "code.h"
#include "common.h"
#include "error.h"
#include "expr.h"
#include "expr.h"
#include "fix.h"
#include "get.h"
#include "global.h"
#include "help_cpu.h"
#include "input.h"
#include "lit.h"
#include "main.h"
#include "m_cpu.h"
#include "m_cpumem.h"
#include "option.h"
#include "phase.h"
#include "put.h"
#include "targetcpu.h"

/**
 * Reloc updater for dstmem() for pre-increment based on symbols.
 *
 * Similar to DestMem_RelocUpdaterCoPro() @ help_cop.c.
 */
static bool
DestMem_RelocUpdater (const char *fileName, unsigned lineNum, ARMWord offset,
		      const Value *valueP,
		      void *privData UNUSED, bool final)
{
  ARMWord ir = GetWord (offset);
  bool isAddrMode3 = (ir & 0x04000090) == 0x90;

  assert (valueP->Tag == ValueCode && valueP->Data.Code.len != 0);

  for (size_t i = 0; i != valueP->Data.Code.len; ++i)
    {
      const Code *codeP = &valueP->Data.Code.c[i];
      if (codeP->Tag == CodeOperator)
	{
	  if (codeP->Data.op != Op_add)
	    return true;
	  continue;
	}
      assert (codeP->Tag == CodeValue);
      const Value *valP = &codeP->Data.value;

      switch (valP->Tag)
	{
	  case ValueInt:
	    {
	      /* This can happen when "LDR Rx, =<constant>" can be turned into
		 MOV/MVN Rx, #<constant>.
	         Or when the current area is absolute.
	         Or when LDR Rx, =<label> with label defined in another AREA.  */
	      ARMWord newIR = ir & NV;
	      newIR |= DST_OP (GET_DST_OP (ir));
	      ARMWord im;
	      if (valueP->Data.Code.len == 1
	          && (im = help_cpuImm8s4 (valP->Data.Int.i)) != (ARMWord)-1)
		newIR |= M_MOV | IMM_RHS | im;
	      else if (valueP->Data.Code.len == 1
		       && (im = help_cpuImm8s4 (~valP->Data.Int.i)) != (ARMWord)-1)
		newIR |= M_MVN | IMM_RHS | im;
	      else if ((areaCurrentSymbol->area.info->type & AREA_ABS) != 0)
		{
		  ARMWord newOffset = valP->Data.Int.i - (Area_GetBaseAddress (areaCurrentSymbol) + offset + 8);
		  ir |= LHS_OP (15);
		  if (isAddrMode3)
		    ir |= B_FLAG;
		  newIR = Fix_CPUOffset (fileName, lineNum, ir, newOffset);
		}
	      else
		return true;
	      Put_InsWithOffset (offset, newIR);
	      break;
	    }

	  case ValueAddr:
	    {
	      ir |= LHS_OP (valP->Data.Addr.r);
	      if (isAddrMode3)
		ir |= B_FLAG;
	      ir = Fix_CPUOffset (fileName, lineNum, ir, valP->Data.Addr.i);
	      Put_InsWithOffset (offset, ir);
	      break;
	    }

	  case ValueSymbol:
	    if (!final)
	      return true;
	    if (Reloc_Create (HOW2_INIT | HOW2_SIZE | HOW2_RELATIVE, offset, valP) == NULL)
	      return true;
	    break;

	  default:
	    return true;
	}
    }

  return false;
}

/**
 * Parses Address mode 2 and 3.
 *
 * Similar to help_copAddr() @ help_cop.c.
 */
static Rslt_e
dstmem (ARMWord ir, const char *mnemonic)
{
  bool isAddrMode3;
  if ((ir & 0x04000090) == 0x90)
    {
      if (ir & S_FLAG)
	Target_NeedAtLeastArch (ARCH_ARMv5TE);
      else
	Target_NeedAtLeastArch (ARCH_ARMv4);
      isAddrMode3 = true;
    }
  else
    isAddrMode3 = false;
  ARMWord dst = getCpuReg ();
  ir |= DST_OP (dst);
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%sdst", InsertCommaAfter);

  const bool doubleReg = (ir & (L_FLAG | S_FLAG)) == S_FLAG;
  const bool translate = (ir & W_FLAG) != 0; /* We have "T" specified in our mnemonic.  */

  if (doubleReg)
    {
      if (dst & 1)
	error (ErrorWarning, "Uneven first transfer register is UNPREDICTABLE");
      /* Try to parse "<reg>, " and check this register is dst + 1.
         If we can't parse this, assume pre-UAL syntax and move on.  */
      ARMWord dstPlusOne = Get_CPURegNoError ();
      if (dstPlusOne != INVALID_REG)
	{
	  if (dstPlusOne != dst + 1)
	    error (ErrorError, "Second transfer register is not %d but %d", dst + 1, dstPlusOne);
	  skipblanks ();
	  if (!Input_Match (',', true))
	    error (ErrorError, "%sdst", InsertCommaAfter);
	}
    }
  
  const ARMWord offset = areaCurrentSymbol->area.info->curIdx;
  bool callRelocUpdate;
  switch (inputLook ())
    {
      case '[':	/* <reg>, [ */
	{
	  inputSkip ();
	  const ARMWord baseReg = getCpuReg (); /* Base register */
	  skipblanks ();
	  bool pre = !Input_Match (']', true);
	  bool forcePreIndex;
	  Value symValue = { .Tag = ValueIllegal };
	  if (Input_Match (',', true))
	    {
	      /* Either [base,XX] (pre = true) or [base],XX (pre = false).  */
	      if (Input_Match ('#', false))
		{
		  if (isAddrMode3)
		    ir |= B_FLAG;  /* Immediate mode for addr. mode 3.  */
		  
		  const Value *valueP = exprBuildAndEval (ValueInt | ValueSymbol | ValueCode);
		  switch (valueP->Tag)
		    {
		      case ValueInt:
			ir = Fix_CPUOffset (NULL, 0, ir | LHS_OP (baseReg), valueP->Data.Int.i);
			break;

		      case ValueSymbol:
		      case ValueCode:
			/* We need to end up with ValueAddr but *valueP is
			   going to be ValueInt (or Reloc Imm8s4).  */
			Value_Assign (&symValue, valueP);
			codeInit ();
			codeAddr (baseReg, 0);
			codeValue (&symValue, false);
			codeOperator (Op_add);
			Value_Assign (&symValue, codeEval (ValueAddr | ValueSymbol | ValueCode, &offset));
			if (symValue.Tag != ValueIllegal)
			  break;
			/* Fall through.  */

		      default:
			if (gPhase != ePassOne)
			  error (ErrorError, "Illegal offset expression");
			symValue = Value_Addr (baseReg, 0);
			break;
		    }

		  forcePreIndex = false;
		  callRelocUpdate = symValue.Tag != ValueIllegal;
		}
	      else
		{
		  ir |= LHS_OP (baseReg);
		  if (!Input_Match ('-', true))
		    {
		      Input_Match ('+', true);
		      ir |= U_FLAG;
		    }
		  if (Input_Match ('#', false))
		    {
		      /* We're about to call getRhs() and that supports "#<immediate>"
			 which isn't possible for our case here.  Hence, check
			 on this.  */
		      error (ErrorError, "Unknown register definition in offset field");
		    }
		  ir |= isAddrMode3 ? 0 : REG_FLAG;
		  ir = getRhs (false, !isAddrMode3, ir);
		  forcePreIndex = false;
		  callRelocUpdate = false;
		}
	    }
	  else
	    {
	      /* [base] */
	      ir |= LHS_OP (baseReg);
	      ir |= U_FLAG; /* 0 nicer than -0.  */
	      if (isAddrMode3)
		ir |= B_FLAG; /* Immediate mode for addr. mode 3.  */
	      if (pre)
		error (ErrorError, "Illegal character");
	      /* Pre-index nicer than post-index but don't this when 'T' is
	         specified as pre-index is not supported (FIXME: ARM only).  */
	      forcePreIndex = !translate;
	      callRelocUpdate = false;
	    }
	  if (pre)
	    {
	      if (!Input_Match (']', true))
		error (ErrorError, "Inserting missing ] after address");
	    }
	  else
	    {
	      if (forcePreIndex)
	        pre = true;
	    }
	  if (Input_Match ('!', true))
	    {
	      if (pre)
	        ir |= W_FLAG;
	      else
	        error (ErrorError, "Writeback not allowed with post-index");
	    }
	  if (pre)
	    {
	      /* Pre-index.  */
	      ir |= P_FLAG;
	      if (translate)
	        error (ErrorError, "Translate not allowed with pre-index");
	    }
	  else
	    {
	      /* Post-index.  */
	      if (dst == baseReg)
	        error (ErrorError, "Post increment is not sane where base and destination register are the same");
	    }

	  codeInit ();
	  codeValue (&symValue, true);
	  break;
	}

      case '=': /* <reg>, =<expression> */
	{
	  inputSkip ();
	  /* Is LDRD ? */
	  if ((ir & 0x0E1000D0) == 0xD0)
	    errorAbort ("No support for LDRD and literal");
	  /* This is only allowed for LDR.  */
	  else if ((ir & L_FLAG) == 0)
	    errorAbort ("You can't store into a constant");

          if (translate)
	    error (ErrorError, "Translate not allowed with literal");
	  
	  /* This is always pre-increment.  */
	  ir |= P_FLAG;

	  Lit_eSize litSize;
	  if ((ir & 0x0C500000) == 0x04100000)
	    litSize = eLitIntWord;
	  else if ((ir & 0x0C500000) == 0x04500000)
	    litSize = eLitIntUByte; /* LDRB, LDRBT */
	  else if ((ir & 0x0E1000F0) == 0x001000B0)
	    litSize = eLitIntUHalfWord; /* LDRH */
	  else if ((ir & 0x0E1000F0) == 0x001000D0)
	    litSize = eLitIntSByte; /* LDRSB */
	  else if ((ir & 0x0E1000F0) == 0x001000F0)
	    litSize = eLitIntSHalfWord; /* LDRSH */
	  else
	    assert (0);
	  /* The ValueInt | ValueSymbol | ValueCode tags are what we support
	     as constants from user point of view.  */
	  const Value *literalP = exprBuildAndEval (ValueInt | ValueSymbol | ValueCode);
	  if (literalP->Tag == ValueIllegal)
	    {
	      error (ErrorError, "Wrong literal type");
	      callRelocUpdate = false;
	    }
	  else
	    {
	      Value value = Lit_RegisterInt (literalP, litSize);
	      codeInit ();
	      codeValue (&value, true);
	      valueFree (&value);
	      callRelocUpdate = true;
	    }
	  break;
	}

      default: /* <reg>, <label>  */
	{
	  /* We're dealing with one of the following:
	   *
	   * 1) a PC-relative label
	   * 2) a field in a register-based map
	   * 3) a label in a register-based area
	   */

	  if (translate)
	    error (ErrorError, "Translate not allowed with label");

	  /* Whatever happens, this must be a pre-increment.  */
	  ir |= P_FLAG;
	  callRelocUpdate = true;

	  exprBuild ();
	  break;
	}
    }

  Put_Ins (ir);

  if (gPhase != ePassOne)
    {
      assert ((!callRelocUpdate || (ir & P_FLAG)) && "Calling reloc for non pre-increment instructions ?");
    
      /* The ValueInt | ValueAddr | ValueSymbol | ValueCode tags are what we
	 support in the LDR/STR/... instruction.  When we have ValueInt it is
	 guaranteed to be a valid immediate.  */
      if (callRelocUpdate
	  && Reloc_QueueExprUpdate (DestMem_RelocUpdater, offset,
				    ValueInt | ValueAddr | ValueSymbol | ValueCode, NULL, 0))
	error (ErrorError, "Illegal %s expression", mnemonic);
    }

  return eRslt_ARM;
}
	     

/**
 * Implements LDR:
 * Pre-UAL:
 *   LDR[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   LDR[<cond>]T <Rd>, <address mode 2> | <pc relative label>
 *   LDR[<cond>]B <Rd>, <address mode 2> | <pc relative label>
 *   LDR[<cond>]BT <Rd>, <address mode 2> | <pc relative label>
 *   LDR[<cond>]D <Rd>, <address mode 3> | <pc relative label>
 *   LDR[<cond>]H <Rd>, <address mode 3> | <pc relative label>
 *   LDR[<cond>]SB <Rd>, <address mode 3> | <pc relative label>
 *   LDR[<cond>]SH <Rd>, <address mode 3> | <pc relative label>
 * UAL:
 *   LDR[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   LDRT[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   LDRB[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   LDRBT[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   LDRD[<cond>] <Rd>, <address mode 3> | <pc relative label>
 *   LDRH[<cond>] <Rd>, <address mode 3> | <pc relative label>
 *   LDRSB[<cond>] <Rd>, <address mode 3> | <pc relative label>
 *   LDRSH[<cond>] <Rd>, <address mode 3> | <pc relative label>
 */
Rslt_e
m_ldr (bool doLowerCase)
{
  ARMWord cc = Option_LdrStrCondAndType (false, doLowerCase);
  if (cc == optionError)
    return eRslt_NotRecognized;
  return dstmem (cc, "LDR");
}

static Rslt_e
LdrStrEx (bool isLoad, bool doLowerCase)
{
  enum { wtype = 0x18<<20, dtype = 0x1A<<20, btype = 0x1C<<20, htype = 0x1E<<20 } type;
  if (Input_Match (doLowerCase ? 'b' : 'B', false))
    type = btype;
  else if (Input_Match (doLowerCase ? 'd' : 'D', false))
    type = dtype;
  else if (inputLook () == (doLowerCase ? 'h' : 'H'))
    {
      /* Small hack needed : 'H' can also be the first condition character
       of 'HS' or 'HI'.  */
      if (inputLookN (1) != (doLowerCase ? 'i' : 'I')
          && inputLookN (1) != (doLowerCase ? 's' : 'S'))
	{
	  inputSkip ();
	  type = htype;
	}
      else
	type = wtype;
    }
  else
    type = wtype;

  ARMWord cc = optionCond (doLowerCase);
  if (cc == optionError)
    return eRslt_NotRecognized;

  if (type == wtype)
    Target_NeedAtLeastArch (ARCH_ARMv6);
  else
    {
      if (Target_GetArch () != ARCH_ARMv6K)
	Target_NeedAtLeastArch (ARCH_ARMv7);
    }
  
  /* The STREX* versions have an extra Rd register.  */
  ARMWord regD;
  if (!isLoad)
    {
      regD = getCpuReg ();
      if (regD == INVALID_REG)
	return eRslt_ARM;
      skipblanks ();
      if (!Input_Match (',', true))
	{
	  error (ErrorError, "Missing ,");
	  return eRslt_ARM;
	}
    }

  ARMWord regT = getCpuReg ();
  if (regT == INVALID_REG)
    return eRslt_ARM;
  if (type == dtype && (regT & 1))
    error (ErrorError, "Register needs to be even");
  skipblanks ();
  if (!Input_Match (',', true))
    {
      error (ErrorError, "Missing ,");
      return eRslt_ARM;
    }

  if (type == dtype)
    {
      ARMWord regT2 = getCpuReg ();
      if (regT2 == INVALID_REG)
	return eRslt_ARM;
      if (regT2 != regT + 1)
	error (ErrorError, "Registers are not consecutive");
      skipblanks ();
      if (!Input_Match (',', true))
	{
	  error (ErrorError, "Missing ,");
	  return eRslt_ARM;
	}
    }
  
  if (!Input_Match ('[', true))
    {
      error (ErrorError, "Missing [");
      return eRslt_ARM;
    }
  ARMWord regN = getCpuReg ();
  if (regN == INVALID_REG)
    return eRslt_ARM;
  skipblanks ();
  if (!Input_Match (']', false))
    {
      error (ErrorError, "Missing ]");
      return eRslt_ARM;
    }
  if (isLoad)
    Put_Ins (cc | 0x00100F9F | type | (regN<<16) | (regT<<12));
  else
    Put_Ins (cc | 0x00000F90 | type | (regN<<16) | (regD<<12) | regT);
  return eRslt_ARM;
}

/**
 * Implements LDREX/LDREXB/LDREXH.
 *   LDREX[<cond>] <Rd>, [<Rn>]
 *   LDREXB[<cond>] <Rd>, [<Rn>]
 *   LDREXH[<cond>] <Rd>, [<Rn>]
 *   LDREXD[<cond>] <Rd>, <Rd2>, [<Rn>]
 */
Rslt_e
m_ldrex (bool doLowerCase)
{
  return LdrStrEx (true, doLowerCase);
}

/**
 * Implements STR<cond>[B].
 * Pre-UAL:
 *   STR[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   STR[<cond>]T <Rd>, <address mode 2> | <pc relative label>
 *   STR[<cond>]B <Rd>, <address mode 2> | <pc relative label>
 *   STR[<cond>]BT <Rd>, <address mode 2> | <pc relative label>
 *   STR[<cond>]D <Rd>, <address mode 3> | <pc relative label>
 *   STR[<cond>]H <Rd>, <address mode 3> | <pc relative label>
 *   STR[<cond>]SB <Rd>, <address mode 3> | <pc relative label>
 *   STR[<cond>]SH <Rd>, <address mode 3> | <pc relative label>
 * UAL:
 *   STR[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   STRT[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   STRB[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   STRBT[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   STRD[<cond>] <Rd>, <address mode 3> | <pc relative label>
 *   STRH[<cond>] <Rd>, <address mode 3> | <pc relative label>
 *   STRSB[<cond>] <Rd>, <address mode 3> | <pc relative label>
 *   STRSH[<cond>] <Rd>, <address mode 3> | <pc relative label>
 */
Rslt_e
m_str (bool doLowerCase)
{
  ARMWord cc = Option_LdrStrCondAndType (true, doLowerCase);
  if (cc == optionError)
    return eRslt_NotRecognized;
  return dstmem (cc, "STR");
}

/**
 * Implements STREX/LDREXB/LDREXH.
 *   STREX[<cond>] <Rd>, [<Rn>]
 *   STREXB[<cond>] <Rd>, [<Rn>]
 *   STREXH[<cond>] <Rd>, [<Rn>]
 *   STREXD[<cond>] <Rd>, <Rd2>, [<Rn>]
 */
Rslt_e
m_strex (bool doLowerCase)
{
  return LdrStrEx (false, doLowerCase);
}


/**
 * Implements CLREX.
 */
Rslt_e
m_clrex (void)
{
  if (Target_GetArch () != ARCH_ARMv6K)
    Target_NeedAtLeastArch (ARCH_ARMv7);
  Put_Ins (0xF57FF01F);
  return eRslt_ARM;
}


/**
 * Implements PLD, PLDW and PLI.
 *   PLD{W} [<Rn>, #+/-<imm12>]
 *   PLD{W} <label>                        <= FIXME: not supported
 *   PLD{W} [<Rn>,+/-<Rm>{, <shift>}]
 *   PLI [<Rn>, #+/-<imm12>]
 *   PLI <label>                        <= FIXME: not supported
 *   PLI [<Rn>,+/-<Rm>{, <shift>}]
 */
/* FIXME: support PLDW & PLI  */
Rslt_e
m_pl (bool doLowerCase)
{
  enum { isPLD, isPLDW, isPLI } type;
  if (Input_Match (doLowerCase ? 'd' : 'D', false))
    {
      if (Input_Match (doLowerCase ? 'w' : 'W', false))
	type = isPLDW;
      else
	type = isPLD;
    }
  else if (Input_Match (doLowerCase ? 'i' : 'I', false))
    type = isPLI;
  else
    return eRslt_NotRecognized;

  if (!Input_IsEndOfKeyword ())
    return eRslt_NotRecognized;

  /* FIXME: we don't check in case of isPLDW that ARMv7 has MP extensions
     enabled.  */
  Target_NeedAtLeastArch (type == isPLD ? ARCH_ARMv5TE : ARCH_ARMv7);

  skipblanks();

  if (!Input_Match ('[', true))
    error (ErrorError, "Expected '[' after PLD instruction");

  ARMWord ir = 0xf450f000 | P_FLAG;
  int op = getCpuReg (); /* Base register */
  ir |= LHS_OP (op);
  skipblanks();

  if (Input_Match (']', true))
    {			/* [base] */
      ir |= U_FLAG;	/* 0 nicer than -0 */
    }
  else
    {
      skipblanks();
      if (!Input_Match (',', true))
	error (ErrorError, "Expected ',' or ']' in PLD instruction");

      if (Input_Match ('#', false))
	{
	  const Value *offset = exprBuildAndEval (ValueInt);
	  switch (offset->Tag)
	    {
	      case ValueInt:
		ir = Fix_CPUOffset (NULL, 0, ir, offset->Data.Int.i);
		break;

	      default:
		error (ErrorError, "Illegal offset expression");
		break;
	    }

	  /* U_FLAG is fixed in Fix_CPUOffset() */
	}
      else
	{
	  ir |= U_FLAG;
	  if (Input_Match ('-', true))
	    ir &= ~U_FLAG;
	  else
	    Input_Match ('+', true);
	  if (Input_Match ('#', false))
	    {
	      /* Edge case - #XX */
	      error (ErrorError, "Unknown register definition in offset field");
	    }
	  ir = getRhs (false, true, ir) | REG_FLAG;
	}

      if (!Input_Match (']', true))
	error (ErrorError, "Expected closing ]");
    }
  Put_Ins(ir);
  return eRslt_ARM;
}


static void
dstreglist (ARMWord ir, bool isPushPop)
{
  if (isPushPop)
    {
      ir |= BASE_MULTI (13);
      ir |= W_FLAG;
      skipblanks ();
    }
  else
    {
      ir |= BASE_MULTI (getCpuReg ());
      skipblanks ();
      if (Input_Match ('!', true))
	ir |= W_FLAG;
      if (!Input_Match (',', true))
	error (ErrorError, "Inserting missing comma before reglist");
    }
  if (!Input_Match ('{', true))
    error (ErrorError, "Inserting missing '{' before reglist");
  int op = 0;
  do
    {
      int low = getCpuReg ();
      skipblanks ();
      int high;
      switch (inputLook ())
	{
	  case '-':
	    inputSkip ();
	    high = getCpuReg ();
	    skipblanks ();
	    if (low > high)
	      {
		error (ErrorInfo, "Register interval in wrong order r%d-r%d", low, high);
		int c = low;
		low = high;
		high = c;
	      }
	    break;

	  case ',':
	  case '}':
	    high = low;
	    break;

          default:
	    error (ErrorError, "Illegal character '%c' in register list", inputLook ());
	    high = 15;
	    break;
        }
      if ((1 << low) < op)
	error (ErrorInfo, "Registers in wrong order");
      if (((1 << (high + 1)) - (1 << low)) & op)
	error (ErrorInfo, "Register occurs more than once in register list");
      op |= (1 << (high + 1)) - (1 << low);
    } while (Input_Match (',', true));
  if (GET_BASE_MULTI (ir) == 13 && (ir & W_FLAG))
    {
      /* Count number of registers loaded or saved.  */
      int i, c = 0;
      for (i = 0; i < 16; ++i)
	{
	  if (op & (1<<i))
	    ++c;
	}
      if (c & 1)
	{
	  if (gArea_Preserve8 == ePreserve8_Yes)
	    error (ErrorWarning, "Stack pointer update potentially breaks 8 byte stack alignment");
	  else if (gArea_Preserve8 == ePreserve8_Guess)
	    gArea_Preserve8Guessed = false;
	}
    }
  if ((ir & W_FLAG) /* Write-back is specified.  */
      && (ir & L_FLAG) /* Is LDM/POP.  */
      && (op & (1 << GET_BASE_MULTI (ir))) /* Base reg. in reg. list.  */
      && (!isPushPop || (op ^ (1 << GET_BASE_MULTI (ir))) != 0) /* Is either LDM/STM, either multi-reg. POP/PUSH.  */)
    {
      /* LDM instructions and multi-register POP instructions that specify base
         register writeback and load their base register are permitted but
         deprecated before ARMv7.
         Use of such instructions is obsolete in ARMv7.  */
      const char *what = isPushPop ? "multi-register POP" : "LDM";
      if (Target_GetArch () < ARCH_ARMv7)
	error (ErrorWarning,
	       "Deprecated before ARMv7 : %s with writeback and base register in register list",
	       what);
      else
	error (ErrorWarning,
	       "Obsoleted from ARMv7 onwards : %s with writeback and base register in register list",
	       what);
    }
  if (!Input_Match ('}', false))
    error (ErrorError, "Inserting missing '}' after reglist");
  ir |= op;
  skipblanks ();
  if (Input_Match ('^', true))
    {
      if ((ir & W_FLAG) && !(ir & (1 << 15)))
	error (ErrorInfo, "Writeback together with force user");
      ir |= FORCE_FLAG;
    }
  if (option_pedantic && (ir & L_FLAG) && (1 << 15) && Target_GetArch() == ARCH_ARMv4T)
    error (ErrorWarning, "ARMv4T does not switch ARM/Thumb state when LDM/POP specifies PC (use BX instead)");
  Put_Ins (ir);
}


/**
 * Implements LDM.
 */
Rslt_e
m_ldm (bool doLowerCase)
{
  ARMWord cc = optionCondLdmStm (true, doLowerCase);
  if (cc == optionError)
    return eRslt_NotRecognized;
  dstreglist (cc | 0x08100000, false);
  return eRslt_ARM;
}


/**
 * Implements POP, i.e. LDM<cond>FD sp!, {...}
 * (= LDM<cond>IA sp!, {...})
 * UAL syntax.
 */
Rslt_e
m_pop (bool doLowerCase)
{
  ARMWord cc = optionCond (doLowerCase);
  if (cc == optionError)
    return eRslt_NotRecognized;
  dstreglist (cc | STACKMODE_IA | 0x08100000, true);
  return eRslt_ARM;
}


/**
 * Implements STM.
 */
Rslt_e
m_stm (bool doLowerCase)
{
  ARMWord cc = optionCondLdmStm (false, doLowerCase);
  if (cc == optionError)
    return eRslt_NotRecognized;
  dstreglist (cc | 0x08000000, false);
  return eRslt_ARM;
}


/**
 * Implements PUSH, i.e. STM<cond>FD sp!, {...}
 * (= STM<cond>DB sp!, {...})
 * UAL syntax.
 */
Rslt_e
m_push (bool doLowerCase)
{
  ARMWord cc = optionCond (doLowerCase);
  if (cc == optionError)
    return eRslt_NotRecognized;
  dstreglist (cc | STACKMODE_DB | 0x08000000, true);
  return eRslt_ARM;
}


/**
 * Implements SWP.
 */
Rslt_e
m_swp (bool doLowerCase)
{
  ARMWord cc = optionCondB (doLowerCase);
  if (cc == optionError)
    return eRslt_NotRecognized;

  Target_NeedAtLeastArch (ARCH_ARMv2a);
  if (option_pedantic && Target_GetArch () >= ARCH_ARMv6)
    error (ErrorWarning, "The use of SWP/SWPB is deprecated from ARMv6 onwards");

  int ir = cc | 0x01000090;
  ir |= DST_OP (getCpuReg ());
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%sdst", InsertCommaAfter);
  ir |= RHS_OP (getCpuReg ());	/* Note wrong order swp dst,rhs,[lsh] */
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%slhs", InsertCommaAfter);
  if (!Input_Match ('[', true))
    error (ErrorError, "Inserting missing '['");
  ir |= DST_MUL (getCpuReg ());
  skipblanks ();
  if (!Input_Match (']', true))
    error (ErrorError, "Inserting missing ']'");
  Put_Ins (ir);
  return eRslt_ARM;
}

typedef enum
{
  BL_eSY = 0xF,
  BL_eST = 0xE,
  BL_eISH = 0xB,
  BL_eSH = BL_eISH,
  BL_eISHST = 0xA,
  BL_eSHST = BL_eISHST,
  BL_eNSH = 0x7,
  BL_eUN = BL_eNSH,
  BL_eNSHST = 0x6,
  BL_eUNST = BL_eNSHST,
  BL_eOSH = 0x3,
  BL_eOSHST = 0x2
} Barrier_eType;

static Barrier_eType
GetBarrierType (void)
{
  skipblanks ();

  Barrier_eType result;
  if (Input_IsEolOrCommentStart ())
    result = BL_eSY;
  else if (Input_MatchKeywordLower ("ish"))
    result = BL_eISH;
  else if (Input_MatchKeywordLower ("ishst"))
    result = BL_eISHST;
  else if (Input_MatchKeywordLower ("nsh"))
    result = BL_eNSH;
  else if (Input_MatchKeywordLower ("nshst"))
    result = BL_eNSHST;
  else if (Input_MatchKeywordLower ("osh"))
    result = BL_eOSH;
  else if (Input_MatchKeywordLower ("oshst"))
    result = BL_eOSHST;
  else if (Input_MatchKeywordLower ("sh"))
    {
      if (option_pedantic)
	error (ErrorWarning, "Use barrier type %s instead of %s", "ISH", "SH");
      result = BL_eSH;
    }
  else if (Input_MatchKeywordLower ("shst"))
    {
      if (option_pedantic)
	error (ErrorWarning, "Use barrier type %s instead of %s", "ISHST", "SHST");
      result = BL_eSHST;
    }
  else if (Input_MatchKeywordLower ("sy"))
    result = BL_eSY;
  else if (Input_MatchKeywordLower ("st"))
    result = BL_eST;
  else if (Input_MatchKeywordLower ("un"))
    {
      if (option_pedantic)
	error (ErrorWarning, "Use barrier type %s instead of %s", "NSH", "UN");
      result = BL_eUN;
    }
  else if (Input_MatchKeywordLower ("unst"))
    {
      if (option_pedantic)
	error (ErrorWarning, "Use barrier type %s instead of %s", "NSHST", "UNST");
      result = BL_eUNST;
    }
  else
    {
      error (ErrorError, "Unknown barrier type");
      result = BL_eSY;
    }

  return result;
}

/**
 * Implements DMB.
 * ARM DMB is unconditional.
 */
Rslt_e
m_dmb (void)
{
  Target_NeedAtLeastArch (ARCH_ARMv7);
  Barrier_eType bl = GetBarrierType ();
  Put_Ins (0xF57FF050 | bl);
  return eRslt_ARM;
}


/**
 * Implements DSB.
 * ARM DSB is unconditional.
 */
Rslt_e
m_dsb (void)
{
  Target_NeedAtLeastArch (ARCH_ARMv7);
  Barrier_eType bl = GetBarrierType ();
  Put_Ins (0xF57FF040 | bl);
  return eRslt_ARM;
}


/**
 * Implements ISB.
 * ARM ISB is unconditional.
 */
Rslt_e
m_isb (void)
{
  Target_NeedAtLeastArch (ARCH_ARMv7);
  Barrier_eType bl = GetBarrierType ();
  if (option_pedantic && bl != BL_eSY)
    error (ErrorWarning, "Using reserved barrier type");
  Put_Ins (0xF57FF060 | bl);
  return eRslt_ARM;
}


/**
 * Implements RFE.
 *   RFE{<amode>} <Rn>{!}
 */
Rslt_e
m_rfe (bool doLowerCase)
{
  ARMWord option = Option_CondRfeSrs (true, doLowerCase);
  if (option == optionError)
    return eRslt_NotRecognized;

  Target_NeedAtLeastArch (ARCH_ARMv6);

  ARMWord regN = getCpuReg ();
  if (regN == INVALID_REG)
    return eRslt_ARM;
  if (regN == 15)
    error (ErrorError, "Using PC as base register is unpredictable");

  skipblanks ();
  bool updateStack = Input_Match ('!', false);

  if (updateStack)
    option |= W_FLAG;
  Put_Ins (0xF8100A00 | option | (regN<<16));
  return eRslt_ARM;
}


/**
 * Implements SRS.
 *   SRS{<amode>} SP{!},#<mode>  : UAL syntax
 *   SRS{<amode>} #<mode>{!}     : pre-UAL syntax
 */
Rslt_e
m_srs (bool doLowerCase)
{
  ARMWord option = Option_CondRfeSrs (false, doLowerCase);
  if (option == optionError)
    return eRslt_NotRecognized;

  Target_NeedAtLeastArch (ARCH_ARMv6);

  skipblanks ();
  bool isUALSyntax, updateStack;
  if (!Input_Match ('#', false))
    {
      isUALSyntax = true;
      ARMWord sp = getCpuReg ();
      if (sp == INVALID_REG)
	return eRslt_ARM;
      if (sp != 13)
	{
	  error (ErrorError, "SRS can only be used with stack register 13 (sp)");
	  return eRslt_ARM;
	}
      skipblanks ();
      updateStack = Input_Match ('!', true);
      if (!Input_Match (',', true))
	{
	  error (ErrorError, "Missing ,");
	  return eRslt_ARM;
	}
      if (!Input_Match ('#', false))
	{
	  error (ErrorError, "%s needs a mode specified", "SRS");
	  return eRslt_ARM;
	}
    }
  else
    isUALSyntax = false;

  const Value *im = exprBuildAndEval (ValueInt);
  if (im->Tag != ValueInt)
    {
      error (ErrorError, "Illegal immediate expression");
      return eRslt_ARM;
    }
  int mode = im->Data.Int.i;
  if (!Option_IsValidARMMode (mode))
    {
      error (ErrorWarning, "Mode 0x%x is not a valid ARM mode", mode);
      mode &= 0x1F;
    }

  if (!isUALSyntax)
    {
      skipblanks ();
      updateStack = Input_Match ('!', false);
    }

  if (updateStack)
    option |= W_FLAG;
  Put_Ins (0xF84D0500 | option | mode);

  return eRslt_ARM;
}
