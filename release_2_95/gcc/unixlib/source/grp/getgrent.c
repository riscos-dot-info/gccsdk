/****************************************************************************
 *
 * $Source: $
 * $Date: $
 * $Revision: $
 * $State: $
 * $Author: $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: $";
#endif

/* Group Password-file operations. */

#include <stddef.h>
#include <stdio.h>
#include <grp.h>

static FILE *stream = NULL;

/* Rewind the stream.  */
void
setgrent (void)
{
  if (stream != NULL)
    rewind (stream);
}

/* Close the stream.  */
void
endgrent (void)
{
  if (stream != NULL)
    {
      fclose (stream);
      stream = NULL;
    }
}

/* Return one entry from the password file.  */
struct group *
getgrent (void)
{
  static struct group grp;

  /* Open the password file if not already open.  */
  if (stream == NULL)
    stream = fopen ("/etc/group", "r");

  if (stream == NULL)
    return NULL;

  return __grpread (stream, &grp);
}

/* Return one entry from the password file (re-entrant version).  */
int
getgrent_r (struct group *result_buf, char *buffer, size_t buflen,
	    struct group **result)
{
  return fgetgrent_r (stream, result_buf, buffer, buflen, result);
}