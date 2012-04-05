/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/pwd/getpwnam_r.c,v $
 * $Date: 2005/04/14 12:13:09 $
 * $Revision: 1.5 $
 * $State: Exp $
 * $Author: nick $
 *
 ***************************************************************************/

/* Search for an entry with a matching username (re-entrant version).
   Contributed by Nick Burrett, 10 December 1997.  */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <pthread.h>

/* Search for an entry with a matching name.  */
int
getpwnam_r (const char *name, struct passwd *result_buf, char *buffer,
	    size_t buflen, struct passwd **result)
{
  FILE *stream;
  struct passwd *p;

  PTHREAD_SAFE_CANCELLATION

  if (result_buf == NULL || buffer == NULL)
    return __set_errno (EINVAL);

  stream = fopen ("/etc/passwd", "r");
  if (stream == NULL)
    {
      *result = __pwddefault ();
      return 0;
    }

  p = __pwdread (stream, result_buf, buffer, buflen);
  while (p != NULL)
    {
      if (strcmp (result_buf->pw_name, name) == 0)
        break;
      p = __pwdread (stream, result_buf, buffer, buflen);
    }

  fclose (stream);

  if (p == NULL)
    return -1;

  if (result != NULL)
    *result = result_buf;

  return 0;
}