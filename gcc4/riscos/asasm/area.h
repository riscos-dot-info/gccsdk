/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2001-2012 GCCSDK Developers
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
 * area.h
 */

#ifndef area_header_included
#define area_header_included

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "asm.h"
#include "symbol.h"
#include "reloc.h"
#include "lit.h"

/* Lowest 8 bits encode the alignment of the start of the area as a power
   of 2 and has a value between 2 and 32.  */
#define AREA_ALIGN_MASK		0x000000FF
#define AREA_ABS		0x00000100
#define AREA_CODE		0x00000200
#define AREA_COMMONDEF		0x00000400 /* Common block definition */
#define AREA_COMMONREF		0x00000800 /* Common block reference */
#define AREA_UDATA		0x00001000 /* Uninitialised (0-initialised) */
#define AREA_READONLY		0x00002000
#define AREA_PIC		0x00004000
#define AREA_DEBUG		0x00008000
#define AREA_32BITAPCS		0x00010000 /* Code area only */
#define AREA_REENTRANT		0x00020000 /* Code area only */
#define AREA_EXTFPSET		0x00040000 /* Code area only */
#define AREA_NOSTACKCHECK	0x00080000 /* Code area only */
#define AREA_THUMB		0x00100000 /* Code area only */
#define AREA_HALFWORD		0x00200000 /* Code area only */
#define AREA_INTERWORK		0x00400000 /* Code area only */
#define AREA_BASED		0x00100000 /* Data area only */
#define AREA_STUBDATA		0x00200000 /* Data area only */
#define AREA_RESERVED22		0x00400000
#define AREA_RESERVED23		0x00800000
#define AREA_MASKBASEREG	0x0F000000 /* Base reg, data area only */
#define AREA_LINKONCE		0x10000000 /* GNU linkonce (GCCSDK extension) Normally a reserved bit. */
#define AREA_RESERVED29		0x20000000
#define AREA_RESERVED30		0x40000000
#define AREA_SOFTFLOAT		0x80000000 /* Avoids FP instructions (GCCSDK extension) Normally reserved bit. */

#define AREA_IMAGE(x) (!((x)->type & AREA_UDATA))

#define AREA_DEFAULT_ALIGNMENT	0x00000002

struct LITPOOL;

typedef struct AREA
{
  Symbol *next;			/** The next area symbol.  */
  uint32_t type;		/* See AREA_ #defines */
  size_t imagesize;
  uint8_t *image;

  uint32_t curIdx;
  uint32_t maxIdx;

  RelocQueue *relocQueue;
  int norelocs;
  Reloc *relocs;

  struct LITPOOL *litPool;	/** The current literal pool waiting to be assembled. */
} Area;

static inline int
Area_GetBaseReg (const Area *area)
{
  return (area->type & AREA_MASKBASEREG) >> 24;
}

static inline uint32_t
Area_GetBaseAddress (const Symbol *symP)
{
  assert ((symP->type & SYMBOL_AREA) != 0 && (symP->area.info->type & AREA_ABS) != 0);
  assert (symP->value.Tag == ValueInt);
  return symP->value.Data.Int.i;
}

extern Symbol *areaCurrentSymbol; /** Symbol of the area which is currently being processed.  */
extern Symbol *areaEntrySymbol; /** Symbol of area which has been marked as ENTRY point.  */
extern int areaEntryOffset;
extern Symbol *areaHeadSymbol; /** Start of the linked list of all area symbols seen so far.  Follow Symbol::area.info->next for next area (*not* Symbol::next !).  */

void Area_PrepareForPhase (ASM_Phase_e phase);

void Area_EnsureExtraSize (Symbol *areaSym, size_t mingrow);

bool Area_IsImplicit (const Symbol *sym);
uint32_t Area_AlignOffset (Symbol *areaSym, uint32_t offset, unsigned alignValue, const char *msg);
uint32_t Area_AlignTo (uint32_t offset, unsigned alignValue, const char *msg);
uint32_t Area_AlignArea (Symbol *areaSym, unsigned alignValue, const char *msg);

bool c_align (void);
bool c_area (void);
bool c_entry (void);
bool c_org (void);
bool c_reserve (void);

typedef enum
{
  ePreserve8_Guess, /** We have to guess.  */
  ePreserve8_Yes,
  ePreserve8_No
} Preserve8_eValue;

extern bool gArea_Require8;
extern Preserve8_eValue gArea_Preserve8;
extern bool gArea_Preserve8Guessed;

bool c_preserve8 (void);
bool c_require8 (void);

typedef enum
{
  eInvalid = -1,
  eARM = 0,
  eData = 1,
  eThumb = 2
} Area_eEntryType;
void Area_MarkStartAs (const Symbol *areaSymbol, uint32_t offset, Area_eEntryType type);
bool Area_IsMappingSymbol (const char *symStr);

#endif
