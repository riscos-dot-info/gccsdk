/*
 * AsAsm an assembler for ARM
 * Copyright (c) 2012-2013 GCCSDK Developers
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

#ifndef m_it_header_included
#define m_it_header_included

#include <stdbool.h>
#include <stdint.h>

#include "phase.h"

typedef struct
{
  bool isThen[4]; /* Then/Else states. isThen[0] is always true.  */
  unsigned maxIdx; /* Varies from 1 .. 4.  */
  unsigned curIdx; /* Varies from 0 .. maxIdx.  When maxIdx, no pending IT
    block.  */
  bool implicitIT; /* There was an implicit IT instruction. When true, IT
    will be injected in Thumb mode and kept up to date.  When true, then
    maxIdx == curIdx altimes.  */

  uint32_t cc; /* Condition code.  */
  uint32_t areaCurIdx; /* Area::curIdx at the moment of IT emit.  */
  
  const char *fileName; /* Filename where this IT block starts.  */
  unsigned lineNum; /* Linenumber where this IT block starts.  */
} IT_State_t;

bool m_it (bool doLowerCase);

void IT_InitializeState (IT_State_t *state);
static inline void
IT_FinalizeState (IT_State_t *state __attribute__ ((unused)))
{
}
void IT_PrepareForPhase (Phase_e phase);
void IT_ApplyCond (uint32_t cc, bool enforceLast, bool isThumb);
bool IT_CanExtendBlock (uint32_t cc);

#endif
