/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/strdup.c,v $
 * $Date: 2001/01/29 15:10:19 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: strdup.c,v 1.2 2001/01/29 15:10:19 admin Exp $";
#endif

#include <stdlib.h>
#include <string.h>
#include <unixlib/unix.h>

char *
strdup (const char *string)
{
  int len;
  char *newstr;

  if (!string)
    return 0;

  len = strlen (string) + 1;
  if ((newstr = malloc (len)) == NULL)
    return NULL;
  return memcpy (newstr, string, len);
}