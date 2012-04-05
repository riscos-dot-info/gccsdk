/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/stdio/putw.c,v $
 * $Date: 2003/04/13 16:21:02 $
 * $Revision: 1.3 $
 * $State: Exp $
 * $Author: alex $
 *
 ***************************************************************************/

#include <stdio.h>
#include <pthread.h>

#define INTSIZE 4

__STDIOLIB__

int
putw (int i, FILE * f)
{
  PTHREAD_UNSAFE

  putc (i & 0xff, f);
  putc ((i >> 8) & 0xff, f);
#if INTSIZE > 2
  putc ((i >> 16) & 0xff, f);
  putc ((i >> 24) & 0xff, f);
#if INTSIZE > 4
  putc ((i >> 32) & 0xff, f);
  putc ((i >> 40) & 0xff, f);
  putc ((i >> 48) & 0xff, f);
  putc ((i >> 56) & 0xff, f);
#endif
#endif
  return (ferror (f) ? -1 : i);
}