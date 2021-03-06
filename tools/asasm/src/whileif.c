/*
 * AsAsm an assembler for ARM
 * Copyright (c) 1997 Darren Salt
 * Copyright (c) 2000-2014 GCCSDK Developers
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "code.h"
#include "decode.h"
#include "error.h"
#include "expr.h"
#include "filestack.h"
#include "input.h"
#include "value.h"
#include "whileif.h"

#ifdef DEBUG
//#  define DEBUG_WHILEIF
#endif

typedef struct
{
  unsigned lineNum;	/* Line number where IF is located.  */
} IfBlock_t;

typedef struct
{
  unsigned lineNum;	/* Line number where WHILE is located.  */
  union
    {
      off_t offsetFile; /* Only valid when gCurPObjP->type == POType_eFile.  */
      const char *offsetCurP; /* Only valid when gCurPObjP->type == POType_eCachedFile.  */
      const char *offsetMacro; /* Only valid when gCurPObjP->type == POType_eMacro.  */
    } restoreData;
} WhileBlock_t;

typedef enum
{
  WhileIf_eIsIf,
  WhileIf_eIsWhile
} WhileIf_e;

/* Structure referred in PObject.  */
typedef struct
{
  WhileIf_e Tag;
  union
    {
      IfBlock_t If;
      WhileBlock_t While;
    } Data;
} WhileIf_t;

#define kMAX_WHILEIF_BLOCKS	128
static WhileIf_t oWhileIfs[kMAX_WHILEIF_BLOCKS];

typedef enum
{
  eSkipToElseElifOrEndif, /* Go to matching ELSE, ELIF or ENDIF and enable assembling (with ELIF, only when its argument is {TRUE}).  */
  eSkipToEndif, /* Go to matching ENDIF and enable assembling.  Ignore matching ELIF.  At matching ELSE, switch to ToDo_eSkipToEndifStrict.  */
  eSkipToEndifStrict /* Go to matching ENDIF (no matching ELSE, ELIF are allowed) and enable assembling.  */
} IfSkip_eToDo;

static bool If_Skip (IfSkip_eToDo toDo);
static bool While_Skip (void);
static bool While_Rewind (void);

/**
 * Skip following assembler lines.  And this because of
 * either a failed '[' / 'IF' test, so skip until we find the corresponding
 * matching '|', 'ELSE', 'ELIF', ']' or 'ENDIF',
 * either we had a successful '[' / 'IF' test and we now reached the '|',
 * 'ELSE' or 'ELIF' part.
 * \param toDo Specifies what to do when encountering matching '|', 'ELSE',
 * 'ELIF', ']' and 'ENDIF'.
 */
static bool
If_Skip (IfSkip_eToDo toDo)
{
  /* We will now skip input lines until a matching '|', 'ELSE', 'ELIF', ']' or
    'ENDIF'.  This means we have to do the final decode check ourselves
    for the current line.  */
  Decode_FinalCheck ();

  int nested = 0;
  while (Input_NextLine (eVarSubstNoWarning))
    {
      /* Ignore blank lines and comments.  */
      if (Input_IsEolOrCommentStart ())
	continue;

      /* Check for label and skip it.
         Make special exception for '$' starting labels, i.e. macro arguments.  */
      bool labelPresent;
      if (isspace ((unsigned char)Input_Look ()))
	labelPresent = false;
      else if (Input_Look () == '$')
	{
	  size_t len;
	  (void) Input_SymbolNoBar (&len);
	  if (Input_Match ('.', false))
	    (void) Input_SymbolNoBar (&len);
	  labelPresent = true;
	}
      else
	labelPresent = Lex_SkipDefiningLabel ();
      Input_SkipWS ();

      /* Check for '[', '|', ']', 'IF', 'ELSE', 'ELIF', 'ENDIF'.  */
      enum { t_if, t_else, t_elif, t_endif } toktype;
      if (Input_MatchKeyword ("[") || Input_MatchKeyword ("IF"))
	toktype = t_if;
      else if (Input_MatchKeyword ("|") || Input_MatchKeyword ("ELSE"))
	toktype = t_else;
      else if (Input_MatchKeyword ("]") || Input_MatchKeyword ("ENDIF"))
	toktype = t_endif;
      else if (Input_MatchKeyword ("ELIF"))
	toktype = t_elif;
      else
	continue;

      if (toktype != t_if && toktype != t_elif)
	{
          Input_SkipWS ();
          if (!Input_IsEolOrCommentStart ())
	    Error (ErrorError, "Spurious characters after %s token", toktype == t_else ? "ELSE" : "ENDIF");
	}

      if (labelPresent)
	Error (ErrorWarning, "Label not allowed here - ignoring");

      switch (toktype)
	{
	  case t_if:
	    nested++;
	    break;

	  case t_else:
	    if (nested == 0)
	      {
		/* Matching.  */
		switch (toDo)
		  {
		    case eSkipToElseElifOrEndif:
		      return false;
		    case eSkipToEndif:
		      toDo = eSkipToEndifStrict; /* From now on, we only expect matching ENDIF (no other matching ELSE, nor matching ELIF).  */
		      break;
		    case eSkipToEndifStrict:
		      Error (ErrorError, "Spurious '|' or 'ELSE' is being ignored");
		      break;
		  }
	      }
	    break;

	  case t_elif:
	    if (nested == 0)
	      {
		switch (toDo)
		  {
		    case eSkipToElseElifOrEndif:
		      {
			const Value *flag = Expr_BuildAndEval (ValueBool);
			bool skipToElseElifOrEndIf;
			if (flag->Tag != ValueBool)
			  {
			    Error (ErrorError, "ELIF expression must be boolean (treating as true)");
			    skipToElseElifOrEndIf = false;
			  }
			else
			  skipToElseElifOrEndIf = !flag->Data.Bool.b;
			if (!skipToElseElifOrEndIf)
			  return false;
			break;
		      }
		    case eSkipToEndif:
		      break;
		    case eSkipToEndifStrict:
		      Error (ErrorError, "Spurious 'ELIF' is being ignored");
		      break;
		  }
	      }
	    break;

	  case t_endif:
	    if (nested-- == 0)
	      return c_endif ();
	    break;
	}
    }
  /* We reached the end of the current parsing object without finding a matching
     '|', 'ELSE', 'ELIF', ']' nor 'ENDIF'.
     There is no need to given an error about that because this condition
     has already been flagged as an error in FS_PopIfWhile().  */

  /* In order not to confuse decode_finalcheck(), inject an empty
     (rest of the) line.  */
  Input_ThisInstead ("");

  return false;
}

/**
 * Implements '[' and 'IF'.
 * Only called from decode().
 */
bool
c_if (void)
{
  if (gCurPObjP->whileIfCurDepth + 1 == kMAX_WHILEIF_BLOCKS)
    {
      Error (ErrorError, "Too many nested WHILE/IFs");
      return false;
    }

  WhileIf_t *whileIfP = &oWhileIfs[gCurPObjP->whileIfCurDepth++];
  whileIfP->Tag = WhileIf_eIsIf;
  whileIfP->Data.If.lineNum = gCurPObjP->lineNum;

  const Value *flag = Expr_BuildAndEval (ValueBool);
  bool skipToElseElifOrEndIf;
  if (flag->Tag != ValueBool)
    {
      Error (ErrorError, "IF expression must be boolean (treating as true)");
      skipToElseElifOrEndIf = false;
    }
  else
    skipToElseElifOrEndIf = !flag->Data.Bool.b;

  if (skipToElseElifOrEndIf)
    return If_Skip (eSkipToElseElifOrEndif);

  return false;
}


/**
 * Implements '|' and 'ELSE'.
 * The previous '[' or 'IF' evaluated to {TRUE} and we're now about to enter
 * the '|' or 'ELSE' clause which we have to ignore.
 * The difference between c_else() and c_elif() is that the latter has an
 * argument we should ignore.
 * Only called from decode().
 */
bool
c_else (void)
{
  if (gCurPObjP->whileIfCurDepth == gCurPObjP->whileIfStartDepth)
    Error (ErrorError, "Mismatched | or ELSE, no matching IF found");
  else
    {
      if (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsWhile)
	{
	  Error (ErrorError, "Mismatched | or ELSE");
	  Error_Line (FS_GetCurFileName(), oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Data.While.lineNum,
		      ErrorError, "note: Maybe because of an unmatched WHILE here");
	}
      else
	{
	  assert (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsIf);
	  return If_Skip (eSkipToEndifStrict);
	}
    }
  
  return false;
}

/**
 * Implements 'ELIF'.
 * The previous '[' or 'IF' evaluated to {TRUE} and we're now about to enter
 * the 'ELIF' clause which we have to ignore.
 * The difference between c_else() and c_elif() is that the latter has an
 * argument we should ignore.
 * Only called from decode().
 */
bool
c_elif (void)
{
  /* Skip the argument of ELIF.  Not of interest now.  */
  Input_Rest ();

  if (gCurPObjP->whileIfCurDepth == gCurPObjP->whileIfStartDepth)
    Error (ErrorError, "Mismatched ELIF, no matching IF found");
  else
    {
      if (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsWhile)
	{
	  Error (ErrorError, "Mismatched ELIF");
	  Error_Line (FS_GetCurFileName(), oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Data.While.lineNum,
		      ErrorError, "note: Maybe because of an unmatched WHILE here");
	}
      else
	{
	  assert (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsIf);
	  return If_Skip (eSkipToEndif);
	}
    }
  
  return false;
}

/**
 * Implements ']' and 'ENDIF'
 * Called from decode() (the previous lines were being assembled) and
 * If_Skip() (the previous lines were being skipped).
 */
bool
c_endif (void)
{
  if (gCurPObjP->whileIfCurDepth == gCurPObjP->whileIfStartDepth)
    Error (ErrorError, "Mismatched ] or ENDIF, no matching IF found");
  else
    {
      if (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsWhile)
	{
	  Error (ErrorError, "Mismatched ] or ENDIF");
	  Error_Line (FS_GetCurFileName(), oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Data.While.lineNum,
		      ErrorError, "note: Maybe because of an unmatched WHILE here");
	}
      else
	{
	  assert (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsIf);
	  --gCurPObjP->whileIfCurDepth;
	}
    }
  
  return false;
}


/**
 * Skip following assembler input lines until we find matching WEND.
 */
static bool
While_Skip (void)
{
  /* We will now skip input lines until a matching 'WEND'.  This means we have
     to do the final decode check ourselves for the current line.  */
  Decode_FinalCheck ();

  const unsigned startLineNumber = FS_GetCurLineNumber ();

  int nested = 0;
  while (Input_NextLine (eVarSubstNoWarning))
    {
      /* Ignore blank lines and comments.  */
      if (Input_IsEolOrCommentStart ())
	continue;
 
      /* Check for label and skip it.
	 Make special exception for '$' starting labels, i.e. macro arguments.  */
      bool labelPresent;
      if (isspace ((unsigned char)Input_Look ()))
	labelPresent = false;
      else if (Input_Look () == '$')
	{
	  size_t len;
	  (void) Input_SymbolNoBar (&len);
	  if (Input_Match ('.', false))
	    (void) Input_SymbolNoBar (&len);
	  labelPresent = true;
	}
      else
	labelPresent = Lex_SkipDefiningLabel ();
      Input_SkipWS ();      

      /* Look for WHILE and WEND.  */
      if (Input_MatchKeyword ("WHILE"))
	{
	  if (labelPresent)
	    Error (ErrorWarning, "Label not allowed here - ignoring");
	  ++nested;
	}
      else if (Input_MatchKeyword ("WEND"))
	{
	  if (labelPresent)
	    Error (ErrorWarning, "Label not allowed here - ignoring");
	  if (nested-- == 0)
	    {
	      assert (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsWhile);
	      --gCurPObjP->whileIfCurDepth;
	      return false;
	    }
	}
    }

  /* We reached the end of the current parsing object without finding a matching
     'WEND'.  */
  Error (ErrorError, "Mismatched WHILE, did you forget WEND");
  Error_Line (FS_GetCurFileName(), startLineNumber,
	      ErrorError, "note: WHILE started here");

  /* In order not to confuse decode_finalcheck(), inject an empty
     (rest of the) line.  */
  Input_ThisInstead ("");

  return false;
}


/**
 * Implements WHILE.
 */
bool
c_while (void)
{
  /* Figure out if we're re-evaluating the current WHILE body or not.  */
  bool isReeval;
  assert (gCurPObjP->whileIfCurDepth >= gCurPObjP->whileIfStartDepth);
  if (gCurPObjP->whileIfCurDepth == gCurPObjP->whileIfStartDepth)
    isReeval = false;
  else
    {
      const WhileIf_t *whileIfP = &oWhileIfs[gCurPObjP->whileIfCurDepth - 1];
      if (whileIfP->Tag != WhileIf_eIsWhile)
	isReeval = false;
      else
	isReeval = whileIfP->Data.While.lineNum == gCurPObjP->lineNum;
    }
  
  if (!isReeval)
    {
      /* First time this WHILE gets executed, create a WHILE context.  */
      if (gCurPObjP->whileIfCurDepth + 1 == kMAX_WHILEIF_BLOCKS)
	{
	  Error (ErrorError, "Too many nested WHILE/IFs");
	  return false;
	}

      WhileIf_t *whileIfP = &oWhileIfs[gCurPObjP->whileIfCurDepth++];
      whileIfP->Tag = WhileIf_eIsWhile;
      whileIfP->Data.While.lineNum = gCurPObjP->lineNum;
      switch (gCurPObjP->type)
	{
	  case POType_eFile:
	    whileIfP->Data.While.restoreData.offsetFile = ftell (gCurPObjP->d.file.fhandle) - gCurPObjP->lastLineSize;
	    break;

	  case POType_eCachedFile:
	    whileIfP->Data.While.restoreData.offsetCurP = gCurPObjP->d.memory.curP - gCurPObjP->lastLineSize;
	    break;
	    
	  case POType_eMacro:
	    whileIfP->Data.While.restoreData.offsetMacro = gCurPObjP->d.macro.curPtr - gCurPObjP->lastLineSize;
	    break;

	  default:
	    assert (!"Unexpected parsable object type");
	    break;
	}
    }

  /* Evaluate expression.  */
  const Value *flag = Expr_BuildAndEval (ValueBool);
  bool whileExprResult;
  if (flag->Tag != ValueBool)
    {
      Error (ErrorError, "WHILE expression must be boolean (treating as false)");
      whileExprResult = false;
    }
  else
    whileExprResult = flag->Data.Bool.b;
#ifdef DEBUG_WHILEIF
  printf("c_while() : expr is <%s>\n", whileExprResult ? "true" : "false");
#endif

  if (!whileExprResult)
    return While_Skip ();

  return false;
}


/**
 * Re-evaluates WHILE expression and prepares body parsing if it needs to be
 * re-executed.
 */
static bool
While_Rewind (void)
{
  assert (gCurPObjP->whileIfCurDepth > gCurPObjP->whileIfStartDepth);
  assert (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsWhile);
  const WhileBlock_t *whileBlockP = &oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Data.While;

  gCurPObjP->lineNum = whileBlockP->lineNum - 1;
  switch (gCurPObjP->type)
    {
      case POType_eFile:
	fseek (gCurPObjP->d.file.fhandle, whileBlockP->restoreData.offsetFile, SEEK_SET);
	break;

      case POType_eCachedFile:
	gCurPObjP->d.memory.curP = whileBlockP->restoreData.offsetCurP;
	break;

      case POType_eMacro:
	gCurPObjP->d.macro.curPtr = whileBlockP->restoreData.offsetMacro;
	break;

      default:
	assert (!"unrecognised WHILE type");
	break;
    }

  return false;
}


/**
 * Implements WEND.
 */
bool
c_wend (void)
{
  assert (gCurPObjP->whileIfCurDepth >= gCurPObjP->whileIfStartDepth);
  if (gCurPObjP->whileIfCurDepth == gCurPObjP->whileIfStartDepth)
    Error (ErrorError, "Mismatched WEND, did you forgot WHILE");
  else if (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsIf)
    {
      Error (ErrorError, "Mismatched WEND, did you forgot WHILE");
      Error_Line (FS_GetCurFileName(), oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Data.If.lineNum,
		  ErrorError, "note: Maybe because of an unmatched IF here");
    }
  else
    {
      assert (oWhileIfs[gCurPObjP->whileIfCurDepth - 1].Tag == WhileIf_eIsWhile);
      /* Re-evaluate WHILE expression.  */
      return While_Rewind ();
    }

  return false;
}


void
FS_PopIfWhile (bool noCheck)
{
  assert (gCurPObjP->whileIfCurDepth >= gCurPObjP->whileIfStartDepth);
  while (gCurPObjP->whileIfCurDepth != gCurPObjP->whileIfStartDepth)
    {
      --gCurPObjP->whileIfCurDepth;
      switch (oWhileIfs[gCurPObjP->whileIfCurDepth].Tag)
	{
	  case WhileIf_eIsIf:
	    if (!noCheck)
	      {
		Error (ErrorError, "Unmatched IF/ELSE/ELIF, did you forgot ENDIF");
		Error_Line (FS_GetCurFileName(), oWhileIfs[gCurPObjP->whileIfCurDepth].Data.If.lineNum,
			    ErrorError, "note: IF started here");
	      }
	    break;

	  case WhileIf_eIsWhile:
	    if (!noCheck)
	      {
		Error (ErrorError, "Unmatched WHILE, did you forgot WEND");
		Error_Line (FS_GetCurFileName(), oWhileIfs[gCurPObjP->whileIfCurDepth].Data.While.lineNum,
			    ErrorError, "note: WHILE started here");
	      }
	    break;

	  default:
	    assert (!"Unknown while/if type");
	    break;
	}
    }
}
