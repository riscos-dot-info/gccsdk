/*
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2002-2010 GCCSDK Developers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * error.h
 */

#ifndef error_header_included
#define error_header_included

#include "global.h"

typedef enum
{
  ErrorInfo,
  ErrorWarning,
  ErrorError, /**< Too many of these are considered as fatal.  */
  ErrorAbort /**< Fatal and last error message.  */
} ErrorTag;

extern const char InsertCommaAfter[];

void errorFinish (void);

void errorLine (const char *file, int lineno,
		ErrorTag e, const char *format, ...)
  __attribute__ ((__format__ (__printf__, 4, 5)));
void error (ErrorTag e, const char *format, ...)
  __attribute__ ((__format__ (__printf__, 2, 3)));

void errorAbortLine (const char *file, int lineno, const char *format, ...)
  __attribute__ ((noreturn))
  __attribute__ ((__format__ (__printf__, 3, 4)));
void errorAbort (const char *format, ...)
  __attribute__ ((noreturn))
  __attribute__ ((__format__ (__printf__, 1, 2)));

void errorOutOfMem (void) __attribute__ ((noreturn));

int returnExitStatus (void);

#endif