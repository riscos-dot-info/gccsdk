/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/signal/kill.c,v $
 * $Date: 2001/01/29 15:10:20 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: kill.c,v 1.2 2001/01/29 15:10:20 admin Exp $";
#endif

/* Written by Nick Burrett, 27 August 1996.  */

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <unixlib/unix.h>
#include <unixlib/sigstate.h>

int
kill (pid_t pid, int sig)
{
  if (pid != __u->pid)
    __unixlib_raise_signal (&__u->sigstate, sig);
  else
    __unixlib_raise_signal (0, sig);

  return 0;
}