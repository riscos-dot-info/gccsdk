/*
 * AS an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2002-2012 GCCSDK Developers
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
 * lex.h
 */

#ifndef lex_header_included
#define lex_header_included

#include "config.h"

#include <stdbool.h>
#include <stddef.h>
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#elif HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#include "global.h"

/* Ordered according to decreasing priority/precedence.  */
typedef enum
{
  /* Unary operators.  */
  eOp_FLoad = 0, eOp_FExec, eOp_FSize, eOp_FAttr,	/* Priority kPrioOp_Unary */
  eOp_LNot, eOp_Not, eOp_Neg, eOp_None,
  eOp_Base, eOp_Index, eOp_Len, eOp_Str, eOp_Chr,
  eOp_Size,
  eOp_LowerCase, eOp_UpperCase, eOp_RevCC,
  eOp_CCEnc, eOp_RConst,

  /* Binary operators.  */
  eOp_Mul, eOp_Div, eOp_Mod,				/* Priority kPrioOp_Multiplicative */
  eOp_Left, eOp_Right, eOp_Concat,			/* Priority kPrioOp_String */
  eOp_ASR, eOp_SHR, eOp_SHL, eOp_ROR, eOp_ROL,		/* Priority kPrioOp_Shift */
  eOp_Add, eOp_Sub, eOp_And, eOp_Or, eOp_XOr,		/* Priority kPrioOp_AddAndLogical */
  eOp_LE, eOp_GE, eOp_LT, eOp_GT, eOp_EQ, eOp_NE,	/* PPriority kPrioOp_Relational */
  eOp_LAnd, eOp_LOr, eOp_LEOr				/* Priority kPrioOp_Boolean */
} Operator_e;

#define kPrioOp_Unary		7
#define kPrioOp_Multiplicative	6
#define kPrioOp_String		5
#define kPrioOp_Shift		4
#define kPrioOp_AddAndLogical	3
#define kPrioOp_Relational	2
#define kPrioOp_Boolean		1

#define kPrioOp_Max		7
#define kPrioOp_Min		1

#ifdef DEBUG
const char *Lex_OperatorAsStr (Operator_e op);
#endif

static inline bool
IsUnop (Operator_e op)
{
  return op <= eOp_RConst;
}

typedef enum
{
  LexId,		/* Start with character */
  LexLocalLabel,	/* Start with digit */
  LexString,		/* "jsgd" */
  LexInt,		/* Start with digit, & and is not a floating point literal.  And fits into 32-bit.  */
  LexInt64,		/* Like LexInt but can only fit into 64-bit.  */
  LexFloat,		/* Start with digit contains dot */
  LexOperator,		/* + - * % / >> >>> <<  ~ & | ! && || == != <= >= */
  LexPosition,		/* . representing current position */
  LexStorage,		/* @ representing current storage counter */
  LexDelim,		/* () */
  LexBool,		/* {TRUE} or {FALSE} */
  LexNone
} LexTag;

typedef struct
{
  LexTag tag; /* FIXME: change into Tag */
  union
    {
      struct			/* LexId */
        {
          const char *str;	/* *NOT* NUL terminated.  */
          size_t len;
          unsigned int hash;
        } Id;
      struct			/* LocalLabel */
	{
          const char *str;	/* *NOT* NUL terminated.  */
          size_t len;
	} LocalLabel;
      struct			/* LexString */
        {
          const char *str;	/* *NOT* NUL terminated.  */
          size_t len;
        } String;
      struct			/* LexInt */
        {
          int value;	/* FIXME: one day, this better becomes uint32_t.  */
        } Int;
      struct			/* LexInt64 */
	{
	  uint64_t value;
	} Int64;
      struct			/* LexFloat */
        {
          ARMFloat value;
        } Float;
      struct			/* LexOperator */
        {
          Operator_e op;
          int pri;
        } Operator;
      struct			/* LexDelim */
        {
          char delim;
        } Delim;
      struct			/* LexBool */
        {
          bool value;
        } Bool;
    } Data;
} Lex;

Lex Lex_GetDefiningLabel (void);
bool Lex_SkipDefiningLabel (void);
Lex Lex_DefineLocalLabel (const Lex *lexP);
Lex lexGetId (void);
Lex lexGetIdNoError (void);
Lex lexGetPrim (void);
Lex lexGetBinop (void);
int lexNextPri (void);

Lex lexTempLabel (const char *str, size_t len);

#ifdef DEBUG
void lexPrint (const Lex *lex);
#endif

#endif