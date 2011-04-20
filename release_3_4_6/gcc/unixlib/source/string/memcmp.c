/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/string/memcmp.c,v $
 * $Date: 2005/04/20 17:12:10 $
 * $Revision: 1.1 $
 * $State: Exp $
 * $Author: nick $
 *
 ***************************************************************************/

#include <string.h>
#include <strings.h>

int
memcmp (const void *s1, const void *s2, size_t n)
{
  const unsigned char *_s1 = s1;
  const unsigned char *_s2 = s2;

  while (n & 0x07)
    {
      if (*_s1 != *_s2)
	goto differs;
      _s1++, _s2++;
      n--;
    }
  n >>= 3;
  while (n)
    {
      if (*_s1 != *_s2)
	goto differs;
      _s1++, _s2++;
      if (*_s1 != *_s2)
	goto differs;
      _s1++, _s2++;
      if (*_s1 != *_s2)
	goto differs;
      _s1++, _s2++;
      if (*_s1 != *_s2)
	goto differs;
      _s1++, _s2++;
      if (*_s1 != *_s2)
	goto differs;
      _s1++, _s2++;
      if (*_s1 != *_s2)
	goto differs;
      _s1++, _s2++;
      if (*_s1 != *_s2)
	goto differs;
      _s1++, _s2++;
      if (*_s1 != *_s2)
	goto differs;
      _s1++, _s2++;
      n--;
    }

  if (n == 0)
    return 0;

differs:
  return (*_s1 - *_s2);
}

int
bcmp (const void *s1, const void *s2, size_t n)
{
  return memcmp (s1, s2, n);
}