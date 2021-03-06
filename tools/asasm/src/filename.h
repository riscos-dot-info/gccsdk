/*
 * AsAsm an assembler for ARM
 * Copyright (c) 2011-2013 GCCSDK Developers
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#ifndef filename_header_included
#define filename_header_included

#include <stdbool.h>
#include <stddef.h>

/* Source:        Unix:                          RISC OS:
 * Dir.A.B        Dir/A.B   Dir/B/A   Dir/A/B    Dir.A.B
 * Dir/A.B        Dir/A.B                        Dir.A.B   Dir.B.A   Dir.A/B
 */
typedef enum
{
  eA_Dot_B,      /* Unix: "Dir/A.B", RISC OS: "Dir.A.B" */
  eB_DirSep_A,   /* Unix: "Dir/B/A", RISC OS: "Dir.B.A" */
  eA_Slash_B     /* Unix: "Dir/A/B", RISC OS: "Dir.A/B" */
} FN_eOutputType;

const char *FN_AnyToNative (const char *any, unsigned pathidx,
			    char *buf, size_t bufsize,
			    bool *state, FN_eOutputType type);

#endif
