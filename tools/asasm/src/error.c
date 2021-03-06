/*
 * AsAsm an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2002-2014 GCCSDK Developers
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

#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "error.h"
#include "filestack.h"
#include "input.h"
#include "macros.h"
#include "main.h"
#ifdef __riscos__
#  include "os.h"
#endif
#include "phase.h"

#define MAXERR (30)

const char InsertCommaAfter[] = "Inserting missing comma after ";

static int oNumErrors = 0;
static int oNumWarnings = 0;

#ifdef __riscos__
typedef enum
{
  eTB_NotStarted,
  eTB_SuccessfullyStarted,
  eTB_NotStartedBecauseOfError,
  eTB_Ended
} TBStatus;
static TBStatus oThrowbackStarted = eTB_NotStarted;
#endif

static char errbuf[2048];


void
Error_PrepareForPhase (Phase_e phase)
{
  switch (phase)
    {
      case eStartUp:
      case ePassOne:
      case ePassTwo:
      case eOutput:
	break;

      case eCleanUp:
	{
#ifdef __riscos__
	  if (oThrowbackStarted == eTB_SuccessfullyStarted)
	    {
	      const _kernel_oserror *err;
	      if ((err = OS_ThrowbackEnd ()) != NULL && option_verbose > 1)
		fprintf (stderr, "OS_ThrowbackEnd error: %s\n", err->errmess);
	      oThrowbackStarted = eTB_Ended;
	    }
#endif
	  break;
	}
    }
}


#ifdef __riscos__
static void
DoThrowback (int level, unsigned lineNum, const char *errstr, const char *fileName)
{
  if (!option_throwback)
    return;

  if (oThrowbackStarted == eTB_NotStarted)
    oThrowbackStarted = OS_ThrowbackStart () ? eTB_NotStartedBecauseOfError : eTB_SuccessfullyStarted;

  if (oThrowbackStarted == eTB_SuccessfullyStarted)
    {
      const _kernel_oserror *err;
      if ((err = OS_ThrowbackSendStart (fileName)) != NULL && option_verbose > 1)
        fprintf (stderr, "OS_ThrowbackSendStart error: %s\n", err->errmess);
      if ((err = OS_ThrowbackSendError (level, lineNum, errstr)) != NULL && option_verbose > 1)
        fprintf (stderr, "OS_ThrowbackSendError error: %s\n", err->errmess);
    }
}
#endif


int
Error_GetExitStatus (void)
{
  return oNumErrors ? EXIT_FAILURE : EXIT_SUCCESS;
}


static void __attribute__ ((noreturn))
fixup (void)
{
  if (asmContinueValid)
    longjmp (asmContinue, 1);

  if (asmAbortValid)
    longjmp (asmAbort, 1);

  abort ();	
}


/**
 * \return true when there are Info and Warning messages during Pass 1 or
 * when option NOWarn is given.
 * If there are Info or warnings (without option NOWarn), we'll emit them
 * during Pass 2.  Errors are always emitted.
 */
static bool
Error_SuppressMsg (ErrorTag e)
{
  return (gPhase == ePassOne || option_nowarn)
           && (e == ErrorInfo || e == ErrorWarning);
}

static void
errorCore (ErrorTag e, const char *format, va_list ap)
{
  /* Before we output to stderr flush first stdout for the case when it shares
     the same underlying stream as stderr.  */
  fflush (stdout);

  const char *str;
#ifdef __riscos__
  int t = 0;
#endif
  switch (e)
    {
      case ErrorInfo:
        str = "Info";
#ifdef __riscos__
        t = ThrowbackInfo;
#endif
        break;

      case ErrorWarning:
        str = "Warning";
        oNumWarnings++;
#ifdef __riscos__
        t = ThrowbackWarning;
#endif
        break;

      case ErrorError:
        str = "Error";
        oNumErrors++;
#ifdef __riscos__
        t = ThrowbackError;
#endif
        break;

      case ErrorAbort:
      default:
        str = "Fatal error";
        oNumErrors++;
#ifdef __riscos__
        t = ThrowbackSeriousError;
#endif
        break;
    }
  vsnprintf (errbuf, sizeof (errbuf), format, ap);
  if (gCurPObjP != NULL)
    {
      const PObject *pObjP = gCurPObjP;
      switch (pObjP->type)
	{
	  case POType_eFile:
	  case POType_eCachedFile:
	    /* When lineNum is zero, we're processing the -PD options.  */
	    if (pObjP->lineNum != 0)
	      fprintf (stderr, "%s:%u:%zd: %s: %s\n",
		       pObjP->fileName,
	               pObjP->lineNum,
		       Input_GetColumn (),
		       str, errbuf);
	    else
	      fprintf (stderr, "%s: %s\n", str, errbuf);
	    break;

	  case POType_eMacro:
	    fprintf (stderr, "%s:%u:%zd: %s: %s in macro %s%s\n",
		     pObjP->fileName,
	             pObjP->lineNum,
		     Input_GetColumn (),
		     str, errbuf,
		     pObjP->d.macro.macro->name,
		     Macro_GetSuffixValue (&pObjP->d.macro));
	    break;
	}

#ifdef __riscos__
      DoThrowback (t, pObjP->lineNum, errbuf, pObjP->fileName);
#endif

      while (pObjP != &gPOStack[0])
	{
	  --pObjP;
	  switch (pObjP->type)
	    {
	      case POType_eFile:
	      case POType_eCachedFile:
		{
		  /* When lineNum is zero, we're processing the -PD options.  */
		  if (pObjP->lineNum != 0)
		    {
		      fprintf (stderr, "  called from line %u from file %s\n",
			       pObjP->lineNum, pObjP->fileName);
#ifdef __riscos__
		      DoThrowback (ThrowbackInfo, pObjP->lineNum, "...was called from here", pObjP->fileName);
#endif
		    }
		  else
		    fputc ('\n', stderr);
		  break;
		}

	      case POType_eMacro:
		{
		  fprintf (stderr, "  called from macro %s%s at line %u in file %s\n",
			   pObjP->d.macro.macro->name,
			   Macro_GetSuffixValue (&pObjP->d.macro),
			   pObjP->lineNum, pObjP->fileName);
#ifdef __riscos__
		  char errPath[256];
		  snprintf (errPath, sizeof (errPath), "...was called from macro %s%s",
			    pObjP->d.macro.macro->name,
			    Macro_GetSuffixValue (&pObjP->d.macro));
		  DoThrowback (ThrowbackInfo, pObjP->lineNum, errPath, pObjP->fileName);
#endif
		  break;
		}
	    }
	}
    }
  else
    fprintf (stderr, "%s: %s\n", str, errbuf);
}


/**
 * ErrorAbort or too many ErrorError won't make this function return.
 * ErrorError will consume the rest of the line.
 */
void
Error (ErrorTag e, const char *format, ...)
{
  if (Error_SuppressMsg (e))
    return;

  va_list ap;
  va_start (ap, format);
  errorCore (e, format, ap);
  va_end (ap);

  Input_ShowLine ();

  if (e == ErrorAbort || oNumErrors > MAXERR)
    fixup ();
  else if (e == ErrorError)
    Input_Rest ();
}


/**
 * Gives fatal error and does not return.
 */
void
Error_Abort (const char *format, ...)
{
  va_list ap;
  va_start (ap, format);
  errorCore (ErrorAbort, format, ap);
  va_end (ap);

  Input_ShowLine ();

  fixup ();
}


/**
 * Gives fatal out-of-memory error and does not return.
 */
void
Error_OutOfMem (void)
{
  Error_Abort ("Out of memory");
}


static void
errorCoreLine (const char *fileName, unsigned lineNum, ErrorTag e,
	       const char *format, va_list ap)
{
  /* Before we output to stderr flush first stdout for the case when it shares
     the same underlying stream as stderr.  */
  fflush (stdout);

  const char *str;
#ifdef __riscos__
  int t = 0;
#endif
  switch (e)
    {
      case ErrorInfo:
        str = "Info";
#ifdef __riscos__
        t = ThrowbackInfo;
#endif
        break;

      case ErrorWarning:
        str = "Warning";
#ifdef __riscos__
        t = ThrowbackWarning;
#endif
        oNumWarnings++;
        break;

      case ErrorError:
        str = "Error";
#ifdef __riscos__
        t = ThrowbackError;
#endif
        oNumErrors++;
        break;

      case ErrorAbort:
      default:
        str = "Fatal error";
#ifdef __riscos__
        t = ThrowbackSeriousError;
#endif
        oNumErrors++;
        break;
    }

  vsnprintf (errbuf, sizeof (errbuf), format, ap);
  /* No column support (as it probably does not make sense).  */
  if (lineNum == 0)
    lineNum = FS_GetCurLineNumber ();
  if (fileName == NULL)
    fileName = FS_GetCurFileName ();
  if (lineNum == 0)
    fprintf (stderr, "%s: %s: %s\n", fileName, str, errbuf);
  else
    fprintf (stderr, "%s:%u: %s: %s\n", fileName, lineNum, str, errbuf);

#ifdef __riscos__
  DoThrowback (t, lineNum, errbuf, fileName);
#endif
}

/**
 * Report Info/Warning/Error/Abort message after the input parsing (e.g.
 * during output time).
 * An ErrorAbort or too many ErrorError's won't make this function return.
 */
void
Error_Line (const char *fileName, unsigned lineNum, ErrorTag e, const char *format, ...)
{
  if (Error_SuppressMsg (e))
    return;

  va_list ap;
  va_start (ap, format);
  errorCoreLine (fileName, lineNum, e, format, ap);
  va_end (ap);
                 
  if (e == ErrorAbort || oNumErrors > MAXERR)
    fixup ();
  else if (e == ErrorError)
    Input_Rest ();
}

/**
 * Reports fatal error for given lineNum and file and does not return.
 * To be used after the input parsing (e.g. during output time).
 */
void
Error_AbortLine (const char *fileName, unsigned lineNum, const char *format, ...)
{
  va_list ap;
  va_start (ap, format);
  errorCoreLine (fileName, lineNum, ErrorAbort, format, ap);
  va_end (ap);
                 
  fixup ();
}
