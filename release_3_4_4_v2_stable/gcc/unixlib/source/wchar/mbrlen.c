/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/wchar/mbrlen.c,v $
 * $Date: 2003/04/05 09:33:57 $
 * $Revision: 1.3 $
 * $State: Exp $
 * $Author: alex $
 *
 ***************************************************************************/

/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <wchar.h>
#include <pthread.h>

/* The mbrlen function has an internal shift state which gets used if
   the PS parameter is NULL.  */
static mbstate_t internal;

size_t
mbrlen (const char *s, size_t n, mbstate_t *ps)
{
  PTHREAD_UNSAFE

  return mbrtowc (NULL, s, n, ps ? ps : &internal);
}