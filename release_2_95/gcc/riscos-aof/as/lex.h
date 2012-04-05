/*
 *   lex.h
 * Copyright � 1992 Niklas R�jemo
 */

#ifndef _lex_h
#define _lex_h

#ifndef _global_h
#include "global.h"
#endif

typedef enum
{
  Op_fload, Op_fexec, Op_fsize, Op_fattr,	/* unop */
  Op_lnot, Op_not, Op_neg, Op_none,	/* unop */
  Op_index, Op_len, Op_str, Op_chr,	/* unop */
  Op_left, Op_right,		/* 10 (9) */
  Op_mul, Op_div, Op_mod,	/* 10 (9) */
  Op_add, Op_sub, Op_concat,	/*  9 (8) */
  Op_and,			/*  8 (7) */
  Op_or,			/*  7 (6) */
  Op_xor,			/*  6 (5) */
  Op_asr, Op_sr, Op_sl, Op_ror, Op_rol,	/*  5 (10 if objasm) */
  Op_le, Op_ge, Op_lt, Op_gt,	/*  4 */
  Op_eq, Op_ne,			/*  3 */
  Op_land,			/*  2 */
  Op_lor			/*  1 */
}
Operator;

#define isUnop(op) \
  ((op)==Op_fload || (op)==Op_fexec || (op)==Op_fsize || \
   (op)==Op_fattr || (op)==Op_lnot  || (op)==Op_not   || \
   (op)==Op_neg   || (op)==Op_index || (op)==Op_len   || \
   (op)==Op_str   || (op)==Op_chr \
  )

int (isUnop) (Operator);

extern const char Pri[2][10];
#define PRI(n) Pri[objasm][n-1]

typedef enum
{ LexId,			/* start with character */
  LexString,			/* "jsgd" */
  LexInt,			/* start with digit */
  LexFloat,			/* start with digit contains dot */
  LexOperator,			/* + - * % / >> >>> << */
  /* ~ & | ^ ! && || */
  /* == != <= >= */
  LexPosition,			/* . representing current position */
  LexStorage,			/* @ representing current storage counter */
  LexDelim,			/* ()[]{}, */
  Lex00Label,			/* local (numeric) label */
  LexBool,			/* {TRUE} or {FALSE} */
  LexNone
}
LexTag;


typedef union
{
  LexTag tag;			/* LexStorage LexDelim LexNone */
  struct
  {
    LexTag tag;
    char *str;
    int len;
    int hash;
  }
  LexId;
  struct
  {
    LexTag tag;
    char *str;
    int len;
  }
  LexString;
  struct
  {
    LexTag tag;
    int value;
  }
  LexInt;
  struct
  {
    LexTag tag;
    FLOAT value;
  }
  LexFloat;
  struct
  {
    LexTag tag;
    Operator op;
    int pri;
  }
  LexOperator;
  struct
  {
    LexTag tag;
    char delim;
  }
  LexDelim;
  struct
  {
    LexTag tag;
    int value;
  }
  Lex00Label;
  struct
  {
    LexTag tag;
    BOOL value;
  }
  LexBool;
}
Lex;

Lex lexGetLabel (void);
Lex lexGetLocal (void);
Lex lexGetId (void);
Lex lexGetIdMunge (int);
Lex lexGetPrim (void);
Lex lexGetBinop (void);
int lexNextPri (void);

Lex lexTempLabel (char *, int);


extern int lexAcornBinop (Lex * lex);
extern int lexAcornUnop (Lex * lex);
extern int lexAcornPrim (Lex * lex);


#endif