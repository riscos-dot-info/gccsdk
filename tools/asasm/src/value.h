/*
 * AsAsm an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2004-2014 GCCSDK Developers
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

#ifndef value_header_included
#define value_header_included

#include <stdbool.h>
#include <stddef.h>

#include "global.h"

struct Symbol;		/* Cannot include symbol.h as it needs value.h */
struct Code;

typedef enum
{
  ValueIllegal   =    0,
  ValueInt       = 1<<0,
  ValueInt64     = 1<<1,
  ValueFloat     = 1<<2,
  ValueString    = 1<<3,
  ValueBool      = 1<<4,
  ValueCode      = 1<<5,
  ValueAddr      = 1<<6,
  ValueSymbol    = 1<<7,

  ValueAll       = 255		/* cheat */
} ValueTag;

typedef enum
{
  eIntType_PureInt,
  eIntType_CPURList,		/* ARM CPU register list (for LDM/STM).  */
  eIntType_CPU,			/* r0 - r15, lr, pc : ARM CPU register.  */
  eIntType_FPU,			/* f0 - f8 : FPE (FPA10/FPA11 and FPASC) register.  */
  eIntType_NeonQuadReg,		/* q0 - q15 : NEON quadword registers.  */
  eIntType_NeonOrVFPDoubleReg,	/* d0 - d31 : NEON/VFP doubleword registers.  */
  eIntType_VFPSingleReg,	/* s0 - s31 : VFP single word registers.  */
  eIntType_CoProReg,		/* p0 - p15 : Coprocessor register.  */
  eIntType_CoProNum		/* c0 - c15 : Not really a register... */
} IntType_e;

typedef struct Value
{
  ValueTag Tag;
  union
    {
      struct			/* ValueInt */
        {
          int i;		/* Must start identical with ValueAddr's i & ValueString's len (FIXME: get rid of this).  */
	  IntType_e type;
        } Int;
      struct			/* ValueInt64 */
	{
	  uint64_t i;
	} Int64;
      struct			/* ValueFloat */
        {
          ARMFloat f;
        } Float;
      struct			/* ValueString */
        {
	  size_t len;		/**< Size string.  Must start identical with ValueInt's i & ValueAddr's i (FIXME: get rid of this).  */
	  const char *s;	/**< *NOT* NUL terminated.  Might be NULL but then len needs to be 0.  */
	  bool owns;		/**< true when this object's s has ownership.  */ 
        } String;
      struct			/* ValueBool */
        {
          bool b;
        } Bool;
      struct			/* ValueCode */
        {
          size_t len;
          const struct Code *c;
        } Code;
      struct			/* ValueAddr, represents address in the form of "[<r>, #<i>]" */
        {
	  int i;		/* Must start identical with ValueInt's i & ValueStrings's len (FIXME: get rid of this).  */
          unsigned r;		/* Ranges from 0 to 15 (inc).  */
        } Addr;
      struct			/* ValueSymbol */
	{
	  int factor;		/* Number of times the symbol needs to be taken into account (can be negative, zero, positive).  */
	  struct Symbol *symbol;
	  int offset;
	} Symbol;
    } Data;
} Value;

static inline Value
Value_Illegal (void)
{
  const Value value = { .Tag = ValueIllegal };
  return value;
}

static inline Value
Value_Int (int i, IntType_e type)
{
  const Value value =
    {
      .Tag = ValueInt,
      .Data.Int = { .i = i, .type = type }
    };
  return value;
}

static inline Value
Value_Int64 (uint64_t i)
{
  const Value value =
    {
      .Tag = ValueInt64,
      .Data.Int64 = { .i = i }
    };
  return value;
}

static inline Value
Value_Float (ARMFloat f)
{
  const Value value =
    {
      .Tag = ValueFloat,
      .Data.Float.f = f
    };
  return value;
}

/**
 * \param str Pointer to string with length len.  Maybe NULL but then len
 * needs to be 0 as well.
 * Does *NOT* to be NUL terminated.
 * \param ownership When false, no ownership is being transfered.  When true,
 * str is a malloced block and ownership being transfered.
 */
static inline Value
Value_String (const char *str, size_t len, bool ownership)
{
  const Value value =
    {
      .Tag = ValueString,
      .Data.String = { .s = str, .len = len, .owns = ownership }
    };
  return value;
}

static inline Value
Value_Bool (bool b)
{
  const Value value =
    {
      .Tag = ValueBool,
      .Data.Bool.b = b
    };
  return value;
}

Value Value_Code (size_t len, const struct Code *code);

static inline Value
Value_Addr (unsigned reg, int i)
{
  const Value value =
    {
      .Tag = ValueAddr,
      .Data.Addr = { .i = i, .r = reg }
    };
  return value;
}

static inline Value
Value_Symbol (struct Symbol *symbol, int factor, int offset)
{
  const Value value =
    {
      .Tag = ValueSymbol,
      .Data.Symbol = { .factor = factor, .symbol = symbol, .offset = offset }
    };
  return value;
}

bool Value_IsLabel (const Value *valueP);

void Value_Assign (Value *dst, const Value *src);
Value Value_Copy (const Value *src);
void Value_Free (Value *value);
bool Value_ResolveSymbol (Value *valueP, bool resolveAbsArea);
bool Value_Equal (const Value *a, const Value *b);
#ifdef DEBUG
void Value_Print (const Value *v);
#endif
#endif
