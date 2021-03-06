/*
 * AsAsm an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2002-2014 GCCSDK Developers
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

#ifndef output_header_included
#define output_header_included

#include <stddef.h>

#include "global.h"
#include "phase.h"

void Output_PrepareForPhase (Phase_e phase);

const char *Output_OpenOutput (const char *outfile);
void Output_Remove (void);

void Output_AOF (void);
void Output_ELF (void);

typedef struct
{
  size_t idx;
  size_t size;
  char *bufP;
} shstrtab_t;

#define SHSTRTAB_INIT_VALUE  { .idx = 0, .size = 0, .bufP = NULL }
void shstrtab_add (shstrtab_t *shStrTabP, const char *str, size_t strLen);
void shstrtab_finish (shstrtab_t *shStrTabP);

extern const char *idfn_text;
#define DEFAULT_IDFN "GCCSDK ASASM AOF/ELF Assembler " VERSION " (" GCCSDK_REVISION ", " __DATE__ ")"

static inline ARMWord
armword (ARMWord val)
{
#if defined(WORDS_BIGENDIAN)
  return (val >> 24)
	   | ((val >> 8) & 0xff00)
	   | ((val << 8) & 0xff0000)
	   | (val << 24);
#else
  return val;
#endif
}

#endif
