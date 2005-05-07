/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/atexit.c,v $
 * $Date: 2003/01/05 12:40:04 $
 * $Revision: 1.3 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: atexit.c,v 1.3 2003/01/05 12:40:04 admin Exp $";
#endif

#include <stdlib.h>
#include <pthread.h>

int
atexit (void (*atexit_function) (void))
{
  PTHREAD_UNSAFE

  if (__atexit_function_count >= __MAX_ATEXIT_FUNCTION_COUNT)
    return -1;

  __atexit_function_array[__atexit_function_count++] = atexit_function;
  return 0;
}
