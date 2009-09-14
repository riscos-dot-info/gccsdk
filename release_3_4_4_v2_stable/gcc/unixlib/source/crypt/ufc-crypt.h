/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/crypt/Attic/ufc-crypt.h,v $
 * $Date: 2002/08/18 21:22:10 $
 * $Revision: 1.1.2.1 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

/*
 * File taken from glibc 2.2.5.
 * Following changes were made:
 *  - Disabled the UINT_FAST32_MAX == UINT_FAST64_MAX for Norcroft as it
 *    didn't like it.
 */

/* Types for UFC-crypt.
   Copyright (C) 1998 Free Software Foundation, Inc.
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

#include <stdint.h>

typedef uint_fast32_t ufc_long;
typedef uint64_t long64;
typedef uint32_t long32;
#define _UFC_32_
/*
#if UINT_FAST32_MAX == UINT_FAST64_MAX
# define _UFC_64_
#else
# define _UFC_32_
#endif
*/