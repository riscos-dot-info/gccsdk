/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/strtok_r.c,v $
 * $Date: 2000/07/15 14:52:09 $
 * $Revision: 1.1.1.1 $
 * $State: Exp $
 * $Author: nick $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: strtok_r.c,v 1.1.1.1 2000/07/15 14:52:09 nick Exp $";
#endif

#include <string.h>

char *
strtok_r (char *s, const char *delim, char **save_ptr)
{
  char *token;

  if (s == NULL)
    s = *save_ptr;

  s += strspn (s, delim);
  if (*s == '\0')
    return NULL;

  token = s;
  s = strpbrk (token, delim);
  if (s)
    {
      *s = 0;
      *save_ptr = s + 1;
    }
  else
    *save_ptr = strchr (token, '\0');

  return token;
}
