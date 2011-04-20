/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/pwd/getlogin.c,v $
 * $Date: 2005/04/14 12:13:09 $
 * $Revision: 1.4 $
 * $State: Exp $
 * $Author: nick $
 *
 ***************************************************************************/

#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/* Return a pointer to a string containing the name of the user logged
   in on the controlling terminal of the process.
   Defined by POSIX as not threadsafe  */

char *
getlogin (void)
{
  static char name[L_cuserid];
  char buf[256 /* NSS_BUFLEN_PASSWD */];
  struct passwd pwent;
  struct passwd *pwptr;

  PTHREAD_UNSAFE_CANCELLATION

  if (getpwuid_r (geteuid (), &pwent, buf, sizeof (buf), &pwptr))
    return NULL;

  strncpy (name, pwptr->pw_name, L_cuserid);
  
  return name;
}