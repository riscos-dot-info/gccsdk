/****************************************************************************
 *
 * $Source$
 * $Date$
 * $Revision$
 * $State$
 * $Author$
 *
 ***************************************************************************/

/* @(#)s_modf.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#if defined(LIBM_SCCS) && !defined(lint)
static char rcsid[] = "$NetBSD: s_modf.c,v 1.8 1995/05/10 20:47:55 jtc Exp $";
#endif

/*
 * modf(double x, double *iptr)
 * return fraction part of x, and return x's integral part in *iptr.
 * Method:
 *	Bit twiddling.
 *
 * Exception:
 *	No exception.
 */

#include <math.h>
#include <unixlib/math.h>
#include <unixlib/types.h>

static const double one = 1.0;

double
modf (double x, double *iptr)
{
  __int32_t i0, i1, j0;
  __uint32_t i;
  EXTRACT_WORDS (i0, i1, x);
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;	/* exponent of x */
  if (j0 < 20)
    {				/* integer part in high x */
      if (j0 < 0)
	{			/* |x|<1 */
	  INSERT_WORDS (*iptr, i0 & 0x80000000, 0);	/* *iptr = +-0 */
	  return x;
	}
      else
	{
	  i = (0x000fffff) >> j0;
	  if (((i0 & i) | i1) == 0)
	    {			/* x is integral */
	      __uint32_t high;
	      *iptr = x;
	      GET_HIGH_WORD (high, x);
	      INSERT_WORDS (x, high & 0x80000000, 0);	/* return +-0 */
	      return x;
	    }
	  else
	    {
	      INSERT_WORDS (*iptr, i0 & (~i), 0);
	      return x - *iptr;
	    }
	}
    }
  else if (j0 > 51)
    {				/* no fraction part */
      __uint32_t high;
      *iptr = x * one;
      GET_HIGH_WORD (high, x);
      INSERT_WORDS (x, high & 0x80000000, 0);	/* return +-0 */
      return x;
    }
  else
    {				/* fraction part in low x */
      i = ((__uint32_t) (0xffffffff)) >> (j0 - 20);
      if ((i1 & i) == 0)
	{			/* x is integral */
	  __uint32_t high;
	  *iptr = x;
	  GET_HIGH_WORD (high, x);
	  INSERT_WORDS (x, high & 0x80000000, 0);	/* return +-0 */
	  return x;
	}
      else
	{
	  INSERT_WORDS (*iptr, i0, i1 & (~i));
	  return x - *iptr;
	}
    }
}

long double modfl (long double x, long double *iptr)
{
  return (long double) modf ((double) x, (double *) iptr);
}
