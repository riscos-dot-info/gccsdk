/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/string/strndup.c,v $
 * $Date: 2002/12/22 18:22:29 $
 * $Revision: 1.1 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

/*
 * File taken from glibc 2.2.5.
 * Following changes were made:
 *  - Renamed __strnlen() into strnlen()
 *  - Defined __strndup() & __strnlen() as strndup() and strnlen()
 *  - Defined _LIBC
 */

/* Copyright (C) 1996, 1997, 1998, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>

#define _LIBC
#define __strndup strndup
#define __strnlen strnlen

#if defined _LIBC || defined STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else
char *malloc ();
#endif

#undef __strndup
#undef strndup

#if defined (__CC_NORCROFT) || defined (__LCC__)
# define __strndup strndup
#endif

char *
__strndup (s, n)
     const char *s;
     size_t n;
{
  size_t len = __strnlen (s, n);
  char *new = (char *) malloc (len + 1);

  if (new == NULL)
    return NULL;

  new[len] = '\0';
  return (char *) memcpy (new, s, len);
}
#ifdef weak_alias
weak_alias (__strndup, strndup)
#endif