/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/signal/sigaltstk.c,v $
 * $Date: 2001/09/04 16:32:04 $
 * $Revision: 1.2.2.1 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: sigaltstk.c,v 1.2.2.1 2001/09/04 16:32:04 admin Exp $";
#endif

/* Written by Nick Burrett, 27 August 1996.  */

#include <signal.h>
#include <errno.h>
#include <unixlib/sigstate.h>
#include <unixlib/unix.h>

/* Specifies an alternate stack for use during signal handling.
   When a signal is received by the process and its action
   indicates that the signal stack is used, the system arranges
   a switch to the currently installed signal stack while the
   handler for that signal is executed.  */

int
sigaltstack (const struct sigaltstack *stack,
	     struct sigaltstack *oldstack)
{
  struct unixlib_sigstate *ss = &__u->sigstate;

  if (stack != NULL && stack->ss_size < MINSIGSTKSZ)
    /* The size of the alternate stack was too small.  */
    return __set_errno (ENOMEM);

  if (stack != NULL && ss->signalstack.ss_flags & SA_ONSTACK)
    /* Trying to disable a stack that is currently in use.  */
    return __set_errno (EINVAL);

  if (oldstack != NULL)
    {
      oldstack->ss_sp = ss->signalstack.ss_sp;
      oldstack->ss_size = ss->signalstack.ss_size;
      oldstack->ss_flags = ss->signalstack.ss_flags;
    }
  if (stack != NULL)
    {
      ss->signalstack.ss_sp = stack->ss_sp;
      ss->signalstack.ss_size = stack->ss_size;
      ss->signalstack.ss_flags = stack->ss_flags;
    }
  return 0;
}
