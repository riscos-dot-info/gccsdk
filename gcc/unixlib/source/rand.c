/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/rand.c,v $
 * $Date: 2000/07/15 14:52:09 $
 * $Revision: 1.1.1.1 $
 * $State: Exp $
 * $Author: nick $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: rand.c,v 1.1.1.1 2000/07/15 14:52:09 nick Exp $";
#endif

#include <stdlib.h>

static unsigned long __state[32] =
{
  0x6fdb9cb7, 0x9de8dc3d, 0x093bf9e4, 0x47528c2b, 0xfc263867, 0x53cbf1bf,
  0x13618c92, 0x9e0f31b1, 0xcd651ab0, 0x2b52a7e5, 0x2ccdd9bf, 0x30052e2e,
  0xb278be81, 0xd634a58b, 0x0a33d2c1, 0xfd42f052, 0xcb2f06f8, 0xa57bb730,
  0x4ca963ac, 0x84bf5532, 0xd67ab9e6, 0x6e2d017b, 0x1e17cd99, 0x5891173a,
  0x39384a29, 0xe0a0282e, 0x2e5512fc, 0x3093f269, 0x3a6983e6, 0x6b9fdaf3,
  0x38b6bbd1, 0xb5e23046
};
static int __st1 = 0, __st2 = 3;

void
srand (register long seed)

{
  register int i;

  for (i = 0; i < 32; i++)
    seed = __state[i] = (seed * 1103515245 + 12345);
  __st1 = 0;
  __st2 = 3;
  for (i = 0; i < ((lrand () ^ seed) & 255); i++);
  for (i = 0; i < ((lrand () ^ seed) & 255); i++)
    lrand ();
}

long
lrand (void)

{
  register long i, j;
  register int k, l;

  i = *((long *) (__state + (k = __st1)));
  j = *((long *) (__state + (l = __st2)));
  if (i < 0 && j < 0)
    i = -i;
  __state[l] = (i += j);
  __st1 = (k + 1) & 31;
  __st2 = (l + 1) & 31;
  return ((i >> 1) & RAND_MAX);
}

int
  (rand) (void)

{
  return (rand ());
}

void
  (srand48) (register long seed)

{
  srand48 (seed);
}

long
  (lrand48) (void)

{
  return (lrand48 ());
}

void
  (srandom) (register long seed)

{
  srandom (seed);
}

long
  (random) (void)

{
  return (random ());
}
