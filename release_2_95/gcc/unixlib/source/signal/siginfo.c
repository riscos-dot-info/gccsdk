/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/signal/siginfo.c,v $
 * $Date: 2001/01/29 15:10:21 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: siginfo.c,v 1.2 2001/01/29 15:10:21 admin Exp $";
#endif

/* Written by Nick Burrett, 27 August 1996.  */

#include <unixlib/os.h>
#include <signal.h>
#include <unixlib/sigstate.h>

/* We should print something relatively informative here.
   But I can't think of anything to print.  */

void
__unixlib_siginfo_handler (int signo)
{
  signo = signo;
  /* Use __os_print to avoid pulling in stdio unnecessarily.  */
  __os_print ("got a SIGINFO\r\n");
}