/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2000-2011 GCCSDK Developersrs
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
 * m_fpu.c
 */

#include "config.h"

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#include "area.h"
#include "code.h"
#include "error.h"
#include "expr.h"
#include "filestack.h"
#include "fix.h"
#include "get.h"
#include "help_cop.h"
#include "input.h"
#include "m_fpu.h"
#include "option.h"
#include "put.h"
#include "reloc.h"
#include "value.h"
#include "main.h"

#define M_ADF 0x0e000100
#define M_DVF 0x0e400100
#define M_FDV 0x0ea00100
#define M_FML 0x0e900100
#define M_FRD 0x0eb00100
#define M_MUF 0x0e100100
#define M_POL 0x0ec00100
#define M_POW 0x0e600100
#define M_RDF 0x0e500100
#define M_RMF 0x0e800100
#define M_RPW 0x0e700100
#define M_RSF 0x0e300100
#define M_SUF 0x0e200100
#define M_ABS 0x0e208100
#define M_ACS 0x0ec08100
#define M_ASN 0x0eb08100
#define M_ATN 0x0ed08100
#define M_COS 0x0e908100
#define M_EXP 0x0e708100
#define M_LGN 0x0e608100
#define M_LOG 0x0e508100
#define M_MNF 0x0e108100
#define M_MVF 0x0e008100
#define M_RND 0x0e308100
#define M_SIN 0x0e808100
#define M_SQT 0x0e408100
#define M_TAN 0x0ea08100
#define M_FIX 0x0e100110
#define M_FLT 0x0e000110
#define M_WFS 0x0e200110
#define M_RFS 0x0e300110
#define M_WFC 0x0e400110
#define M_RFC 0x0e500110
#define M_CMF 0x0e90f110
#define M_CNF 0x0eb0f110

#define M_URD 0x0ee08100
#define M_NRM 0x0ef08100
#define M_LFM 0x0c100200
#define M_SFM 0x0c000200

#define M_FMNEM 0x0ef08110

typedef enum
{
  eFPUsage_Possible,
  eFPUsage_AlreadyUsed,
  eFPUsage_Forbidden
} FPUsage_e;

static FPUsage_e oFPUsage = eFPUsage_Possible;
static const char *oFPUsageFirstUse_File = NULL; /**< First usage of FP instruction/directive (file).  */
static int oFPUsageFirstUse_Line = 0; /**< First usage of FP instruction/directive (linenumber).  */
static const char *oFPUsageNOFP_File = NULL; /**< Place of NOFP (file).  */
static int oFPUsageNOFP_Line = 0; /**< Place of NOFP (linenumber).  */

/**
 * \return always return false (so it can be used as tail-call).
 */
static bool
CheckFPUsageIsAllowed (void)
{
  if (option_apcs_softfloat)
    {
      error (ErrorError, "soft-float code uses hard FP instructions");
      return false;
    }

  switch (oFPUsage)
    {
      case eFPUsage_Possible:
	oFPUsage = eFPUsage_AlreadyUsed;
	oFPUsageFirstUse_File = FS_GetCurFileName ();
	oFPUsageFirstUse_Line = FS_GetCurLineNumber ();
	break;

      case eFPUsage_AlreadyUsed:
	break;
	
      case eFPUsage_Forbidden:
	error (ErrorError, "FP usage is not allowed");
	errorLine (oFPUsageNOFP_File, oFPUsageNOFP_Line, ErrorError, "note: NOFP was mentioned here");
	break;
    }

  return false;
}

static ARMWord
fpuImm (ARMFloat d)
{
  if (d == 0.0)
    return 0;
  if (d == 1.0)
    return 1;
  if (d == 2.0)
    return 2;
  if (d == 3.0)
    return 3;
  if (d == 4.0)
    return 4;
  if (d == 5.0)
    return 5;
  if (d == 0.5)
    return 6;
  if (d == 10.0)
    return 7;

  return -1;
}

static ARMWord
fixImmFloat (ARMWord ir, ARMFloat im)
{
  static const char op3[] = "Changing \"%s F_, F_, #%.1f\" to \"%s F_, F_, #%.1f\"";
  static const char op2[] = "Changing \"%s F_, #%.1f\" to \"%s F_, #%.1f\"";

  int f = fpuImm (im);
  if (f != -1)
    return ir | f;

  /* Immediate float constant was illegal.  */
  f = fpuImm (-im);
  if (f == -1)
    {
      /* Even the inverse cannot be represented.  */
      error (ErrorError, "Illegal immediate constant %g", im);
      return ir;
    }

  /* Inverse immediate constant can be represented, so try to invert
     the mnemonic.  */
  ARMWord mnemonic = ir & M_FMNEM;
  ir &= ~M_FMNEM;
  const char *m1, *m2, *optype;
  switch (mnemonic)
    {
      case M_ADF:
        ir |= M_SUF;
        optype = op3; m1 = "ADF"; m2 = "SUF";
        break;

      case M_SUF:
        ir |= M_ADF;
        optype = op3; m1 = "SUF"; m2 = "ADF";
        break;

      case M_MVF:
        ir |= M_MNF;
        optype = op2; m1 = "MVF"; m2 = "MNF";
        break;

      case M_MNF:
        ir |= M_MVF;
        optype = op2; m1 = "MNF"; m2 = "MVF";
        break;

      case M_CMF & M_FMNEM:
        ir |= M_CNF;
        optype = op2; m1 = "CMF"; m2 = "CNF";
        break;

      case M_CNF & M_FMNEM:
        ir |= M_CMF;
        optype = op2; m1 = "CNF"; m2 = "CMF";
        break;

      case (M_CMF | EXEPTION_BIT) & M_FMNEM:
        ir |= M_CNF | EXEPTION_BIT;
        optype = op2; m1 = "CMFE"; m2 = "CNFE";
        break;

      case (M_CNF | EXEPTION_BIT) & M_FMNEM:
        ir |= M_CMF | EXEPTION_BIT;
        optype = op2; m1 = "CNFE"; m2 = "CMFE";
        break;

      default:
        errorAbort ("Internal fixImmFloat: unknown mnemonic");
        return ir;
        break;
    }

  if (option_fussy > 1)
    error (ErrorInfo, optype, m1, im, m2, -im);

  return ir | f;
}

static ARMWord
getFloatRhs (ARMWord ir)
{
  if (Input_Match ('#', false))
    {
      ir |= 8; /* Immediate float.  */
      const Value *im = exprBuildAndEval (ValueInt | ValueFloat);
      switch (im->Tag)
	{
	  case ValueInt:
	    ir = fixImmFloat (ir, im->Data.Int.i);
	    break;
	  case ValueFloat:
	    ir = fixImmFloat (ir, im->Data.Float.f);
	    break;
	  default:
	    error (ErrorError, "Illegal float immediate");
	    break;
	}
    }
  else
    ir |= getFpuReg ();

  return ir;
}

/** DATA dyadic **/

static void
dstlhsrhs (ARMWord ir)
{
  ARMWord op = getFpuReg ();
  ir |= DST_OP (op);
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%sdst", InsertCommaAfter);
  op = getFpuReg ();
  ir |= LHS_OP (op);
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%slhs", InsertCommaAfter);
  Put_Ins (getFloatRhs (ir));
}


/**
 * Implements ADF.
 */
bool
m_adf (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_ADF | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements DVF.
 */
bool
m_dvf (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_DVF | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements FDV.
 */
bool
m_fdv (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_FDV | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements FML.
 */
bool
m_fml (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_FML | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements FRD.
 */
bool
m_frd (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_FRD | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements MUF.
 */
bool
m_muf (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_MUF | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements POL.
 */
bool
m_pol (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_POL | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements POW.
 */
bool
m_pow (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_POW | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements RDF.
 */
bool
m_rdf (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_RDF | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements RMF.
 */
bool
m_rmf (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_RMF | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements RPW.
 */
bool
m_rpw (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_RPW | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements RSF.
 */
bool
m_rsf (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_RSF | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements SUF.
 */
bool
m_suf (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstlhsrhs (M_SUF | cc);
  return CheckFPUsageIsAllowed ();
}


/** DATA monadic **/

static void
dstrhs (ARMWord ir)
{
  ARMWord op = getFpuReg ();
  ir |= DST_OP (op);
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%sdst", InsertCommaAfter);
  Put_Ins (getFloatRhs (ir));
}

/**
 * Implements ABS.
 */
bool
m_abs (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_ABS | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements ACS.
 */
bool
m_acs (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_ACS | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements ASN.
 */
bool
m_asn (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_ASN | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements ATN.
 */
bool
m_atn (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_ATN | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements COS.
 */
bool
m_cos (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_COS | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements EXP.
 */
bool
m_exp (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_EXP | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements LGN.
 */
bool
m_lgn (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_LGN | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements LOG.
 */
bool
m_log (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_LOG | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements MNF.
 */
bool
m_mnf (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_MNF | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements MVF.
 */
bool
m_mvf (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_MVF | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements RND.
 */
bool
m_rnd (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_RND | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements SIN.
 */
bool
m_sin (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_SIN | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements SQT.
 */
bool
m_sqt (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_SQT | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements TAN.
 */
bool
m_tan (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_TAN | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements URD.
 */
bool
m_urd (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_URD | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements NRM.
 */
bool
m_nrm (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;
  dstrhs (M_NRM | cc);
  return CheckFPUsageIsAllowed ();
}

static void
comparelow (ARMWord ir)		/* No precision and no rounding allowed ? */
{
  ir |= LHS_OP (getFpuReg ());
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%slhs", InsertCommaAfter);
  Put_Ins (getFloatRhs (ir));
}

/**
 * Implements CMF.
 */
bool
m_cmf (void)
{
  ARMWord cc = optionExceptionCond ();
  if (cc == optionError)
    return true;
  comparelow (M_CMF | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements CNF.
 */
bool
m_cnf (void)
{
  ARMWord cc = optionExceptionCond ();
  if (cc == optionError)
    return true;
  comparelow (M_CNF | cc);
  return CheckFPUsageIsAllowed ();
}

/** REGISTER TRANSFER **/

/**
 * Implements FIX.
 */
bool
m_fix (void)
{
  ARMWord cc = optionCondOptRound ();
  if (cc == optionError)
    return true;

  ARMWord ir = M_FIX | cc;
  ir |= DST_OP (getCpuReg ());
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%sdst", InsertCommaAfter);
  Put_Ins (getFloatRhs (ir));
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements FLT.
 */
bool
m_flt (void)
{
  ARMWord cc = optionCondPrecRound ();
  if (cc == optionError)
    return true;

  ARMWord ir = M_FLT | cc;
  ir |= LHS_OP (getFpuReg ());
  skipblanks ();
  if (!Input_Match (',', true))
    error (ErrorError, "%sdst", InsertCommaAfter);
  ir |= DST_OP (getCpuReg ());
  Put_Ins (ir);
  return CheckFPUsageIsAllowed ();
}

static void
flagtransfer (ARMWord ir)
{
  ARMWord op = getCpuReg ();
  ir |= DST_OP (op);
  Put_Ins (ir);
}

/**
 * Implements WFS.
 */
bool
m_wfs (void)
{
  ARMWord cc = optionCond ();
  if (cc == optionError)
    return true;
  flagtransfer (M_WFS | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements RFS.
 */
bool
m_rfs (void)
{
  ARMWord cc = optionCond ();
  if (cc == optionError)
    return true;
  flagtransfer (M_RFS | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements WFC.
 */
bool
m_wfc (void)
{
  ARMWord cc = optionCond ();
  if (cc == optionError)
    return true;
  flagtransfer (M_WFC | cc);
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements RFC.
 */
bool
m_rfc (void)
{
  ARMWord cc = optionCond ();
  if (cc == optionError)
    return true;
  flagtransfer (M_RFC | cc);
  return CheckFPUsageIsAllowed ();
}

static void
dstmem (ARMWord ir)
{
  ir |= DST_OP (getFpuReg ());
  ir = help_copAddr (ir, false);
  Put_Ins (ir);
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
  return CheckFPUsageIsAllowed ();
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
  return CheckFPUsageIsAllowed ();
}


static void
dstmemx (ARMWord ir)
{
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
  Put_Ins (ir);
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
  return CheckFPUsageIsAllowed ();
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
  return CheckFPUsageIsAllowed ();
}

/**
 * Implements NOFP.
 */
bool
c_nofp (void)
{
  if (oFPUsage == eFPUsage_AlreadyUsed)
    {
      error (ErrorError, "Too late to ban floating point");
      errorLine (oFPUsageFirstUse_File, oFPUsageFirstUse_Line, ErrorError, "note: first floating point usage was here");
    }
  else
    {
      oFPUsageNOFP_File = FS_GetCurFileName ();
      oFPUsageNOFP_Line = FS_GetCurLineNumber ();
      oFPUsage = eFPUsage_Forbidden;
    }

  return false;
}
