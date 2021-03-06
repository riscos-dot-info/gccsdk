/* getgrnam_r ()
 * Search for an entry with a matching group name (re-entrant version).
 *
 * Copyright (c) 2002-2010 UnixLib Developers
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <grp.h>
#include <errno.h>

#include <pthread.h>
#include <internal/unix.h>

/* Search for an entry with a matching name.  */
int
getgrnam_r (const char *name, struct group *result_buf, char *buffer,
	    size_t buflen, struct group **result)
{
  PTHREAD_SAFE_CANCELLATION

  if (result_buf == NULL || buffer == NULL)
    return __set_errno (EINVAL);

  FILE *stream = fopen ("/etc/group", "r");
  if (stream == NULL)
    return -1;

  struct group *grp;
  while ((grp = __grpread (stream, result_buf, buffer, buflen)) != NULL)
    {
      if (strcmp (result_buf->gr_name, name) == 0)
        break;
    }

  fclose (stream);

  if (grp == NULL)
    return -1;

  if (result != NULL)
    *result = result_buf;

  return 0;
}
