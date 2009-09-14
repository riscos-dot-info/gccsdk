/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/time/getitmr.c,v $
 * $Date: 2003/11/23 20:26:45 $
 * $Revision: 1.4 $
 * $State: Exp $
 * $Author: joty $
 *
 ***************************************************************************/

#include <stddef.h>
#include <errno.h>
#include <sys/time.h>
#include <unixlib/unix.h>

/* The `getitimer' function stores information about the timer
   specified by 'which' in the structure pointed at by 'value'

   The return value and error conditions are the same as for
   `setitimer'.

   Set *value to the current setting of timer 'which'.
   Return 0 on success, -1 on errors.  */
int
getitimer (enum __itimer_which which, struct itimerval *value)
{
  /* We can't implement interval timers whilst executing in a task
     window nor whilst running as a WIMP program.  Note that when
     __taskwindow == 1 => __taskhandle != 0 but not necessary vice
     versa so the test on __taskhandle is enough. */
  if (__taskhandle != 0)
    {
      errno = ENOSYS;
      return -1;
    }

  if ((unsigned) which >= __MAX_ITIMERS)
    {
      errno = EINVAL;
      return -1;
    }
  *value = __u->itimers[which];

  return 0;
}