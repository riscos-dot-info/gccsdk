
/*
 * AofFile.h
 * Copyright � 1992 Niklas R�jemo
 *
 * Types that exist in an AOF file.
 */

#ifndef AofFile_h
#define AofFile_h

typedef struct
{
  uint32_t Name;		/* Offset into string table */
  uint32_t Type;
  uint32_t Size;		/* Size of area, must be divisible by four */
  uint32_t noRelocations;	/* Size of relocation table */
  uint32_t Unused;		/* Unused, must be zero */
}
AofEntry;

#define AofHeaderID 0xc5e2d080

typedef struct
{
  uint32_t Type;		/* 0xc5e2d080 if relocatable object format       */
  uint32_t Version;		/* 1.xx -> 150  2.xx -> 200                      */
  uint32_t noAreas;		/* size of Area[]                                */
  uint32_t noSymbols;		/* size of Symbol Table if such exist            */
  uint32_t EntryArea;		/* Where to start execution, 0 no entry          */
  uint32_t EntryOffset;		/* otherwise start at Area[EntryArea]+AreaOffset */
}
AofHeader;

#define aofHeaderSize(max) \
        (sizeof (AofHeader) + \
        (max - 1) * sizeof(AofEntry))

/* Type 1 relocation directive.  */
#define HOW1_INIT     0x00000000
#define HOW1_BYTE     0x00000000
#define HOW1_HALF     0x00010000
#define HOW1_WORD     0x00020000
#define HOW1_SIZE     0x00030000

#define HOW1_RELATIVE 0x00040000
/* Only used if not HOW1_RELATIVE */
#define HOW1_SYMBOL   0x00080000
/* Only used if HOW1_SYMBOL */
#define HOW1_SIDMASK  0x0000ffff

/* Type 2 relocation directive.  */
#define HOW_TYPE2     0x80000000
#define HOW2_INIT     0x80000000
#define HOW2_BYTE     0x00000000
#define HOW2_HALF     0x01000000
#define HOW2_WORD     0x02000000
#define HOW2_SIZE     0x03000000

#define HOW2_RELATIVE 0x04000000
/* Only used if not HOW2_RELATIVE */
#define HOW2_SYMBOL   0x08000000
/* Symbol offset if HOW2_SYMBOL, otherwise area number */
#define HOW2_SIDMASK  0x00ffffff

/* Type 3 relocation directive.  */
#define HOW_TYPE3     (1 << 31)
#define HOW3_INIT     (1 << 31)
/* Field to be relocated is a byte.  */
#define HOW3_BYTE     (0 << 24)
/* Field to be relocated is a half word.  */
#define HOW3_HALF     (1 << 24)
/* Field to be relocated is a word.  */
#define HOW3_WORD     (2 << 24)
/* Field to be relocated is an instruction sequence.  */
#define HOW3_INSTR    (3 << 24)
#define HOW3_SIZE     0x03000000

/* PC-relative.  */
#define HOW3_RELATIVE (1 << 26)
/* Based area.  */
#define HOW3_BASED    (1 << 28)
/* The 'A' bit. Determines SID type.  */
#define HOW3_SYMBOL   (1 << 27)
/* Symbol offset if HOW3_SYMBOL, otherwise area number */
#define HOW3_SIDMASK  0x00ffffff

typedef struct
{
  uint32_t Offset;		/* Offset in area of the field to be relocated */
  uint32_t How;		/* How relocation is done */
}
AofReloc;


/* Defined with local scope */
#define TYPE_LOCAL     0x01
#define TYPE_REFERENCE 0x02
/* Defined with global scope */
#define TYPE_GLOBAL    0x03
#define TYPE_KIND      0x03
#define TYPE_DEFINE    0x01
#define TYPE_EXPORT    0x02

/* This is a constant, (not valid if TYPE_REFERENCE) */
#define TYPE_ABSOLUTE  0x04
/* Only if TYPE_REFERENCE, case insesitive */
#define TYPE_NOCASE    0x08
/* Only if TYPE_REFERENCE, must not be resolved */
#define TYPE_WEAK      0x10
/* Complicated ??? */
#define TYPE_STRONG    0x20
#define TYPE_COMMON    0x40

typedef struct
{
  uint32_t Name;		/* Offset in string Table */
  uint32_t Type;
  uint32_t Value;		/* Value if constant, Size if common, otherwise Offset */
  uint32_t AreaName;
}
AofSymbol;


#endif
