/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/signal/signal.c,v $
 * $Date: 2001/01/29 15:10:21 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: signal.c,v 1.2 2001/01/29 15:10:21 admin Exp $";
#endif

/* Written by Nick Burrett, 27 August 1996.  */

#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <unixlib/sigstate.h>

/* Set the handler for the signal 'sig' to handler, returning
   the old handler, or SIG_ERR on error.  */
sighandler_t
signal (int sig, sighandler_t handler)
{
  struct sigaction act, oact;

  if (handler == SIG_ERR || __SIG_INVALID_P (sig))
    /* Illegal signals will be caught by sigaction. */
    {
      (void) __set_errno (EINVAL);
      return SIG_ERR;
    }

  act.sa_handler = handler;
  if (sigemptyset (&act.sa_mask) < 0)
    return SIG_ERR;

  act.sa_flags = 0;		/* sigismember (&_sigintr, sig) ? 0 : SA_RESTART; */
  if (sigaction (sig, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}