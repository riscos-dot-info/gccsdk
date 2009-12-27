/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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

/* Find the first occurrence in WCS of any wide-character in ACCEPT.  */
wchar_t *
wcspbrk (wcs, accept)
     register const wchar_t *wcs;
     register const wchar_t *accept;
{
  while (*wcs != L'\0')
    if (wcschr (accept, *wcs) == NULL)
      ++wcs;
    else
      return (wchar_t *) wcs;

  return NULL;
}
