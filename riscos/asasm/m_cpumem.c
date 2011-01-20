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
#include "put.h"
#include "targetcpu.h"

/**
 * Reloc updater for dstmem() for pre-increment based on symbols.
 */
static bool
DestMem_RelocUpdater (const char *file, int lineno, ARMWord offset,
		      const Value *valueP,
		      void *privData __attribute__ ((unused)), bool final)
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
	      /* This can only happen when "LDR Rx, =<constant>" can be turned into
		 MOV/MVN Rx, #<constant>.  */
	      if (valueP->Data.Code.len != 1)
		return true;
	      ARMWord newIR = ir & NV;
	      newIR |= DST_OP (GET_DST_OP (ir));
	      ARMWord im;
	      if ((im = help_cpuImm8s4 (valP->Data.Int.i)) != -1)
		newIR |= M_MOV | IMM_RHS | im;
	      else if ((im = help_cpuImm8s4 (~valP->Data.Int.i)) != -1)
		newIR |= M_MVN | IMM_RHS | im;
	      else if (areaCurrentSymbol->area.info->type & AREA_ABS)
		{
		  ARMWord newOffset = valP->Data.Addr.i - (areaCurrentSymbol->area.info->baseAddr + offset + 8);
		  ir |= LHS_OP (15);
		  if (isAddrMode3)
		    ir |= B_FLAG;
		  newIR = Fix_CPUOffset (file, lineno, ir, newOffset);
		}
	      else
		return true;
	      Put_InsWithOffset (offset, newIR);
	    }
	    break;

	  case ValueAddr:
	    {
	      ir |= LHS_OP (valP->Data.Addr.r);
	      if (isAddrMode3)
		ir |= B_FLAG;
	      ir = Fix_CPUOffset (file, lineno, ir, valP->Data.Addr.i);
	      Put_InsWithOffset (offset, ir);
	    }
	    break;

	  case ValueSymbol:
	    if (!final)
	      return true;
	    if (Reloc_Create (HOW2_INIT | HOW2_WORD, offset, valP) == NULL)
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
 */
static bool
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
  int dst = getCpuReg ();
  ir |= DST_OP (dst);
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%sdst", InsertCommaAfter);
  switch (inputLook ())
    {
      case '[':	/* LDR <reg>, [ */
	{
	  inputSkip ();
	  bool trans = false; /* FIXME: wtf ? */
	  skipblanks ();
	  int op = getCpuReg ();	/* Base register */
	  ir |= LHS_OP (op);
	  skipblanks ();
	  bool pre = !Input_Match (']', true);
	  bool offValue = false;
	  Value symValue = { .Tag = ValueIllegal };
	  if (Input_Match (',', true))
	    { /* either [base,XX] or [base],XX */
	      if (Input_Match ('#', false))
		{
		  if (isAddrMode3)
		    ir |= B_FLAG;  /* Immediate mode for addr. mode 3.  */

		  const Value *offset = exprBuildAndEval (ValueInt | ValueSymbol | ValueCode);
		  switch (offset->Tag)
		    {
		      case ValueInt:
			ir = Fix_CPUOffset (NULL, 0, ir, offset->Data.Int.i);
			break;

		      case ValueSymbol:
		      case ValueCode:
			symValue = *offset;
			break;
			
		      default:
			error (ErrorError, "Illegal offset expression");
			break;
		    }

		  /* U_FLAG is fixed in Fix_CPUOffset() */
		  offValue = true;
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
		  ir |= isAddrMode3 ? 0 : REG_FLAG;
		  ir = getRhs (true, !isAddrMode3, ir);
		  offValue = true;
		}
	      skipblanks ();
	    }
	  else
	    {			/* [base] if this way */
	      ir |= U_FLAG;	/* 0 nicer than -0; pre nicer than post */
	      if (isAddrMode3)
		ir |= B_FLAG;	/* Immediate mode for addr. mode 3. */
	      if (pre)
		error (ErrorError, "Illegal character '%c' after base", inputLook ());
	    }
	  if (pre)
	    {
	      if (!Input_Match (']', true))
		error (ErrorError, "Inserting missing ] after address");
	    }
	  else
	    {
	      /* If offset value was never set, then make it a pre-index load */
	      if (!offValue)
	        pre = true;
	      else if (dst == op)
	        error (ErrorError, "Post increment is not sane where base and destination register are the same");
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
	      ir |= P_FLAG;
	      if (trans)
	        error (ErrorError, "Translate not allowed with pre-index");
	    }

	  codeInit ();
	  codeValue (&symValue, true);
	  const ARMWord offset = areaCurrentSymbol->value.Data.Int.i;
	  Put_Ins (ir);
	  if (symValue.Tag != ValueIllegal
	      && Reloc_QueueExprUpdate (DestMem_RelocUpdater, offset,
					ValueInt | ValueAddr | ValueSymbol | ValueCode, NULL, 0))
	    error (ErrorError, "Illegal %s expression", mnemonic);
	}
        break;

      case '=': /* LDR* <reg>, =<expression> */
	{
	  inputSkip ();
	  /* Is LDRD ? */
	  if ((ir & 0x0E1000D0) == 0xD0)
	    errorAbort ("No support for LDRD and literal");
	  /* This is only allowed for LDR.  */
	  else if ((ir & L_FLAG) == 0)
	    errorAbort ("You can't store into a constant");
	  /* The ValueInt | ValueSymbol | ValueCode tags are what we support
	     as constants from user point of view.  */
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
	  Value value = Lit_RegisterInt (exprBuildAndEval (ValueInt | ValueSymbol | ValueCode), litSize);
	  codeInit ();
	  codeValue (&value, true);
	  valueFree (&value);
	  const ARMWord offset = areaCurrentSymbol->value.Data.Int.i;
	  Put_Ins (ir | P_FLAG);
	  /* The ValueInt | ValueAddr | ValueSymbol | ValueCode tags are what we
	     support in the LDR instruction.  When we have ValueInt it is
	     garanteed to be a valid immediate.  */
	  if (Reloc_QueueExprUpdate (DestMem_RelocUpdater, offset,
				     ValueInt | ValueAddr | ValueSymbol | ValueCode, NULL, 0))
	    error (ErrorError, "Illegal %s expression", mnemonic);
	}
	break;

      default: /* LDR* <reg>, <label>  */
	{
	  /* We're dealing with one of the following:
	   *
	   * 1) a PC-relative label
	   * 2) a field in a register-based map
	   * 3) a label in a register-based area
	   */
	  exprBuild ();
	  const ARMWord offset = areaCurrentSymbol->value.Data.Int.i;
	  /* Whatever happens, this must be a pre-increment.  */
	  Put_Ins (ir | P_FLAG);
	  if (Reloc_QueueExprUpdate (DestMem_RelocUpdater, offset,
				     ValueInt | ValueAddr | ValueSymbol | ValueCode, NULL, 0))
	    error (ErrorError, "Illegal %s expression", mnemonic);
	}
	break;
    }
  return false;
}
	     

/**
 * Implements LDR:
 *   LDR[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   LDR[<cond>]T <Rd>, <address mode 2> | <pc relative label>
 *   LDR[<cond>]B <Rd>, <address mode 2> | <pc relative label>
 *   LDR[<cond>]BT <Rd>, <address mode 2> | <pc relative label>
 *   LDR[<cond>]D <Rd>, <address mode 3> | <pc relative label>
 *   LDR[<cond>]H <Rd>, <address mode 3> | <pc relative label>
 *   LDR[<cond>]SB <Rd>, <address mode 3> | <pc relative label>
 *   LDR[<cond>]SH <Rd>, <address mode 3> | <pc relative label>
 */
bool
m_ldr (void)
{
  ARMWord cc = optionCondBT (false);
  if (cc == optionError)
    return true;
  return dstmem (cc, "LDR");
}

static bool
LdrStrEx (bool isLoad)
{
  enum { wtype = 0x18<<20, dtype = 0x1A<<20, btype = 0x1C<<20, htype = 0x1E<<20 } type;
  switch (inputLook ())
    {
      case 'B':
	inputSkip ();
	type = btype;
        break;

      case 'D':
	inputSkip ();
	type = dtype;
	break;

      case 'H':
	/* Small hack needed : 'H' can also be the first condition character
	   of 'HS' or 'HI'.  */
	if (inputLookN (1) != 'I' && inputLookN (1) != 'S')
	  {
	    inputSkip ();
	    type = htype;
	    break;
	  }
	/* Fall through.  */

      default:
	type = wtype;
	break;
    }
  ARMWord cc = optionCond ();
  if (cc == optionError)
    return true;

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
      skipblanks ();
      regD = getCpuReg ();
      if (regD == INVALID_REG)
	return false;
      skipblanks ();
      if (!Input_Match (',', true))
	{
	  error (ErrorError, "Missing ,");
	  return false;
	}
    }

  skipblanks ();
  ARMWord regT = getCpuReg ();
  if (regT == INVALID_REG)
    return false;
  if (type == dtype && (regT & 1))
    error (ErrorError, "Register needs to be even");
  skipblanks ();
  if (!Input_Match (',', true))
    {
      error (ErrorError, "Missing ,");
      return false;
    }

  if (type == dtype)
    {
      skipblanks ();
      ARMWord regT2 = getCpuReg ();
      if (regT2 == INVALID_REG)
	return false;
      if (regT2 != regT + 1)
	error (ErrorError, "Registers are not consecutive");
      skipblanks ();
      if (!Input_Match (',', true))
	{
	  error (ErrorError, "Missing ,");
	  return false;
	}
    }
  
  if (!Input_Match ('[', true))
    {
      error (ErrorError, "Missing [");
      return false;
    }
  ARMWord regN = getCpuReg ();
  if (regN == INVALID_REG)
    return false;
  skipblanks ();
  if (!Input_Match (']', false))
    {
      error (ErrorError, "Missing ]");
      return false;
    }
  if (isLoad)
    Put_Ins (cc | 0x00100F9F | type | (regN<<16) | (regT<<12));
  else
    Put_Ins (cc | 0x00000F90 | type | (regN<<16) | (regD<<12) | regT);
  return false;
}

/**
 * Implements LDREX/LDREXB/LDREXH.
 *   LDREX[<cond>] <Rd>, [<Rn>]
 *   LDREXB[<cond>] <Rd>, [<Rn>]
 *   LDREXH[<cond>] <Rd>, [<Rn>]
 *   LDREXD[<cond>] <Rd>, <Rd2>, [<Rn>]
 */
bool
m_ldrex (void)
{
  return LdrStrEx (true);
}

/**
 * Implements STR<cond>[B].
 *   STR[<cond>] <Rd>, <address mode 2> | <pc relative label>
 *   STR[<cond>]T <Rd>, <address mode 2> | <pc relative label>
 *   STR[<cond>]B <Rd>, <address mode 2> | <pc relative label>
 *   STR[<cond>]BT <Rd>, <address mode 2> | <pc relative label>
 *   STR[<cond>]D <Rd>, <address mode 3> | <pc relative label>
 *   STR[<cond>]H <Rd>, <address mode 3> | <pc relative label>
 *   STR[<cond>]SB <Rd>, <address mode 3> | <pc relative label>
 *   STR[<cond>]SH <Rd>, <address mode 3> | <pc relative label>
 */
bool
m_str (void)
{
  ARMWord cc = optionCondBT (true);
  if (cc == optionError)
    return true;
  return dstmem (cc, "STR");
}

/**
 * Implements STREX/LDREXB/LDREXH.
 *   STREX[<cond>] <Rd>, [<Rn>]
 *   STREXB[<cond>] <Rd>, [<Rn>]
 *   STREXH[<cond>] <Rd>, [<Rn>]
 *   STREXD[<cond>] <Rd>, <Rd2>, [<Rn>]
 */
bool
m_strex (void)
{
  return LdrStrEx (false);
}


/**
 * Implements CLREX.
 */
bool
m_clrex (void)
{
  if (Target_GetArch () != ARCH_ARMv6K)
    Target_NeedAtLeastArch (ARCH_ARMv7);
  Put_Ins (0xF57FF01F);
  return false;
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
bool
m_pl (void)
{
  enum { isPLD, isPLDW, isPLI } type;
  if (Input_Match ('D', false))
    {
      if (Input_Match ('W', false))
	type = isPLDW;
      else
	type = isPLD;
    }
  else if (Input_Match ('I', false))
    type = isPLI;
  else
    return true;

  if (!Input_IsEndOfKeyword ())
    return true;

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
	  ir = getRhs (true, true, ir) | REG_FLAG;
	}

      if (!Input_Match (']', true))
	error (ErrorError, "Expected closing ]");
    }
  Put_Ins(ir);
  return false;
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
	    skipblanks ();
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
  if (GET_BASE_MULTI (ir) == 13
      && (ir & W_FLAG))
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
  Put_Ins (ir);
}


/**
 * Implements LDM.
 */
bool
m_ldm (void)
{
  ARMWord cc = optionCondLdmStm (true);
  if (cc == optionError)
    return true;
  dstreglist (cc | 0x08100000, false);
  return false;
}


/**
 * Implements POP, i.e. LDM<cond>FD sp!, {...}
 * (= LDM<cond>IA sp!, {...})
 */
bool
m_pop (void)
{
  ARMWord cc = optionCond ();
  if (cc == optionError)
    return true;
  dstreglist (cc | STACKMODE_IA | 0x08100000, true);
  return false;
}


/**
 * Implements STM.
 */
bool
m_stm (void)
{
  ARMWord cc = optionCondLdmStm (false);
  if (cc == optionError)
    return true;
  dstreglist (cc | 0x08000000, false);
  return false;
}


/**
 * Implements PUSH, i.e. STM<cond>FD sp!, {...}
 * (= STM<cond>DB sp!, {...})
 */
bool
m_push (void)
{
  ARMWord cc = optionCond ();
  if (cc == optionError)
    return true;
  dstreglist (cc | STACKMODE_DB | 0x08000000, true);
  return false;
}


/**
 * Implements SWP.
 */
bool
m_swp (void)
{
  ARMWord cc = optionCondB ();
  if (cc == optionError)
    return true;

  Target_NeedAtLeastArch (ARCH_ARMv2a);

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
  return false;
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
bool
m_dmb (void)
{
  Target_NeedAtLeastArch (ARCH_ARMv7);
  Barrier_eType bl = GetBarrierType ();
  Put_Ins (0xF57FF050 | bl);
  return false;
}


/**
 * Implements DSB.
 * ARM DSB is unconditional.
 */
bool
m_dsb (void)
{
  Target_NeedAtLeastArch (ARCH_ARMv7);
  Barrier_eType bl = GetBarrierType ();
  Put_Ins (0xF57FF040 | bl);
  return false;
}


/**
 * Implements ISB.
 * ARM ISB is unconditional.
 */
bool
m_isb (void)
{
  Target_NeedAtLeastArch (ARCH_ARMv7);
  Barrier_eType bl = GetBarrierType ();
  if (option_pedantic && bl != BL_eSY)
    error (ErrorWarning, "Using reserved barrier type");
  Put_Ins (0xF57FF060 | bl);
  return false;
}


/**
 * Implements RFE.
 *   RFE{<amode>} <Rn>{!}
 */
bool
m_rfe (void)
{
  ARMWord option = Option_CondRfeSrs (true);
  if (option == optionError)
    return true;

  Target_NeedAtLeastArch (ARCH_ARMv6);

  skipblanks ();
  ARMWord regN = getCpuReg ();
  if (regN == INVALID_REG)
    return false;
  if (regN == 15)
    error (ErrorError, "Using PC as base register is unpredictable");

  skipblanks ();
  bool updateStack = Input_Match ('!', false);

  if (updateStack)
    option |= W_FLAG;
  Put_Ins (0xF8100A00 | option | (regN<<16));
  return false;
}


/**
 * Implements SRS.
 *   SRS{<amode>} SP{!},#<mode>  : UAL syntax
 *   SRS{<amode>} #<mode>{!}     : pre-UAL syntax
 */
bool
m_srs (void)
{
  ARMWord option = Option_CondRfeSrs (false);
  if (option == optionError)
    return true;

  Target_NeedAtLeastArch (ARCH_ARMv6);

  skipblanks ();
  bool isUALSyntax, updateStack;
  if (!Input_Match ('#', false))
    {
      isUALSyntax = true;
      ARMWord sp = getCpuReg ();
      if (sp == INVALID_REG)
	return false;
      if (sp != 13)
	{
	  error (ErrorError, "SRS can only be used with stack register 13 (sp)");
	  return false;
	}
      skipblanks ();
      updateStack = Input_Match ('!', true);
      if (!Input_Match (',', true))
	{
	  error (ErrorError, "Missing ,");
	  return false;
	}
      if (!Input_Match ('#', false))
	{
	  error (ErrorError, "%s needs a mode specified", "SRS");
	  return false;
	}
    }
  else
    isUALSyntax = false;

  const Value *im = exprBuildAndEval (ValueInt);
  if (im->Tag != ValueInt)
    {
      error (ErrorError, "Illegal immediate expression");
      return false;
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

  return false;
}