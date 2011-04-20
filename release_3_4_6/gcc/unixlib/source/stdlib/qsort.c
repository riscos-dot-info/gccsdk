/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/stdlib/qsort.c,v $
 * $Date: 2005/04/20 17:12:10 $
 * $Revision: 1.1 $
 * $State: Exp $
 * $Author: nick $
 *
 ***************************************************************************/

#include <unixlib/unix.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define N_INSERT	8

static char *__t;

static size_t __z;
static int (*__c) (const void *, const void *);

/* fast insertion sort - used for n <= N_INSERT */

static void
__isort (char *b, size_t n)
{
  size_t z = __z;
  int (*c) (const void *, const void *) = __c;
  char *m, *e, *p;
  char *t;

#define swap(x,y) (memcpy(t,x,z),memcpy(x,y,z),memcpy(y,t,z))
#define move(x,y,z) memmove(x,y,z)
#define push(x) memcpy(t,x,z)
#define pull(x) memcpy(x,t,z)

  t = __t;

  e = b + (n * z);		/* past end */

/* find minimum */

  for (m = p = b; (p += z) < e;)
    if ((*c) (m, p) > 0)
      m = p;

/* swap minimum into base */

  if (m != b)
    swap (m, b);

/* standard insertion sort */

  for (m = b; (p = m += z) < e;)
    {
      while ((*c) (p -= z, m) > 0);
      if ((p += z) != m)
	push (m), move (p + z, p, m - p), pull (p);
    }

#undef swap
#undef move
#undef push
#undef pull
}

/* quicksort - used for n > N_INSERT */

static void
__qsort (char *b, size_t n)
{
  size_t z = __z;
  int (*c) (const void *, const void *) = __c;
  char *m, *e, *p, *t;
  size_t i, j;
  int k;

#define swap(x,y) (memcpy(t,x,z),memcpy(x,y,z),memcpy(y,t,z))

loop:

  t = __t;

  m = b + ((n >> 1) * z);	/* middle */
  e = b + ((n - 1) * z);	/* end */

/* find pivot - median of b,m,e */

  if ((*c) (b, m) >= 0)
    p = b;
  else
    p = m, m = b;
  if ((*c) (p, e) > 0)
    p = ((*c) (m, e) >= 0) ? m : e;

/* swap pivot into base */

  if (p != b)
    swap (p, b);

/* standard quicksort & check for flat partition */

  m = b;
  i = 0;
  j = 1;
  for (p = b; (p += z) <= e;)
    {
      if (!(k = (*c) (p, b)))
	j++;
      if (k < 0)
	{
	  if ((m += z) != p)
	    swap (m, p);
	  i++;
	}
    }

  if (j == n)
    return;			/* exit if flat */

  if (b != m)
    swap (b, m);

  m += z;

/* sort smallest partition first */

  if (i < (n >> 1))
    {
      if (i > N_INSERT)
	__qsort (b, i);
      else if (i > 1)
	__isort (b, i);
      i = n - i - 1;
    }
  else
    {
      i = n - i - 1;
      if (i > N_INSERT)
	__qsort (m, i);
      else if (i > 1)
	__isort (m, i);
      i = n - i - 1;
      m = b;
    }

  if (i > N_INSERT)
    {
      b = m;
      n = i;
      goto loop;
    }				/* iterate larger partition */
  else if (i > 1)
    __isort (m, i);

#undef swap
}

void
qsort (void *v, size_t n, size_t z,
       int (*c) (const void *, const void *))
{
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  /* Use a mutex so that threads aren't disabled while calling the user function */

  if (n < 2)
    return;

#if __UNIXLIB_FEATURE_PTHREADS
  pthread_mutex_lock (&mutex);
#endif

  if (!(__t = malloc (z)))
    {
#if __UNIXLIB_FEATURE_PTHREADS
      pthread_mutex_unlock (&mutex);
#endif
      return;
    }

  __z = z;
  __c = c;

#if __UNIXLIB_PARANOID > 0
  /* check function pointer */
  __funcall ((*c), (v, v));
#endif

  if (n > N_INSERT)
    __qsort ((char *) v, n);
  else if (n > 1)
    __isort ((char *) v, n);

  free (__t);

#if __UNIXLIB_FEATURE_PTHREADS
  pthread_mutex_unlock (&mutex);
#endif
}