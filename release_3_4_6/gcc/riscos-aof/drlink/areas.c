/*
** Drlink AOF Linker - AOF Area handling
**
** Copyright (c) 1993, 1994, 1995, 1996, 1997, 1998  David Daniels
** Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006  GCCSDK Developers
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** This file contains the functions involved with manipulating
** the AOF file OBJ_AREA chunks. It also contains all the
** relocation code
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "drlhdr.h"
#include "areahdr.h"
#include "filehdr.h"
#include "chunkhdr.h"
#include "procdefs.h"

#ifndef CROSS_COMPILE
# include <kernel.h>
# include <swis.h>
#endif

/* Variables referenced from other files */

arealist *rocodelist,		/* Pointer to R/O code list */
  *rwcodelist,			/* Pointer to R/W code list */
  *rodatalist,			/* Pointer to R/O data list */
  *rwdatalist,			/* Pointer to R/W data list */
  *zidatalist,			/* Pointer to zero-initialised (R/W) data list */
  *debuglist;			/* Pointer to debug list (R/O) */

unsigned int workspace,		/* Size of relocatable image workspace */
  progbase,			/* Address of start of program */
  codebase,			/* Start-of-code address */
  database,			/* Start-of-R/W data address */
  relocaddr;			/* Address of relocation code in image */

/** Image file entry point */

arealist *entryarea;		/* Pointer to area containing entry point */
unsigned int entryoffset;	/* Offset within area of entry point */

symtentry *symtbase;		/* Address of current OBJ_SYMT chunk */

/* Private declarations */

typedef enum
{ TYPE_1, TYPE_2 } reloctype;

/*
** 'relaction' is used when dealing with partially-linked AOF files. It says what
** can be done to the relocations in the original AOF files, that is, whether they
** can be resolved at this point, whether they are impossible and so on.
*/
typedef enum
{
  NOTPOSS,	/* Relocation not possible */
  IGNORE,	/* Ignore relocation */
  POSSIBLE,	/* Relocation can be finished in its entirety */
  TYPE2_SA,	/* Relocate but extra type-2 symbol additive relocation needed */
  TYPE2_SP,	/* Relocate but extra type-2 symbol PC-relative relocation needed */
  TYPE2_AA,	/* Relocate but extra type-2 area additive relocation needed */
  TYPE2_AP,	/* Relocate but extra type-2 area PC-relative relocation needed */
  TYPE2_RP	/* Relocation not possible but offset in instruction must be altered */
} relaction;

/*
** 'areasrchlist' defines an entry in a symbol table of area names used
** when reading in the AOF files. It is used to speed up searching for a
** symbol's area.
*/
typedef struct areasrchlist
{
  arealist *srcarea;		/* Pointer to area's list entry */
  int srchash;			/* Area's hashed name */
  struct areasrchlist *srcflink;	/* Next in search list */
} areasrchlist;

#define GCCAREA "C$$gnu"	/* Area names starting with this cannot be deleted */
#define GCCAREALEN (sizeof(GCCAREA)-1)	/* Length of name */

#define MAXSRCH 32		/* Size of area search table (must be a power of 2) */
#define SRCHMASK (MAXSRCH-1)	/* Mask for area search table */
#define RELOCSIZE 8		/* Size of a relocation entry in bytes in OBJ_AREA chunk */
#define ABSBASE 0x8000		/* Address of start of program code when loaded */

#ifndef CROSS_COMPILE
# define DEFAULTSIZE 0x8000	/* Default page size (32K to allow for pre-RiscPC machines) */
#else
# define DEFAULTSIZE 0x1000	/* Default page size (4K to confuse matters) */
#endif

/* The following constants are all used in the relocation code */

#define TYPE1_MASK 0xFFFF	/* Mask to extract symbol's OBJ_SYMT index from type-1 relocation */
#define TYPE2_MASK 0xFFFFFF	/* Mask to extract symbol's OBJ_SYMT index from type-2 relocation */
#define TYPE1_SHIFT 16		/* Register shift to extract type-1 relocation type code */
#define TYPE2_SHIFT 24		/* Register shift to extract type-2 relocation type code */
#define BYTEHIGH 0xFF		/* Highest value allowed in byte relocation */
#define HALFHIGH 0xFFFF		/* Highest value allowed in halfword relocation */
#define PCMODIFIER 8		/* Adjustment for PC-relative instructions for PC */
#define INSTMASK 0xE000000	/* Mask to identify instruction type */
#define IN_LDRSTR 0x4000000	/* Single data transfer - LDR Rx,[Ry,#n] */
#define IN_DATAPRO 0x2000000	/* Data processing (with 'immediate' bit set) */
#define IN_BRANCH 0xA000000	/* Branch or branch with link */
#define IN_OFFMASK 0xFFF	/* Mask to extract offset from LDR/STR */
#define MAX_OFFSET 0x1000	/* Maximum offset in LDR/STR */
#define IN_POSOFF 0x00800000	/* +ve offset bit mask (= 1 = +ve, = 0 = -ve) */
#define IN_ADDSUB 0x03E00000	/* Mask to extract opcode from arithmetic instruction */
#define INST_ADD  0x02800000	/* ADD instruction */
#define INST_SUB  0x02400000	/* SUB instruction */
#define ADSB_MASK 0xFE1FF000	/* Mask for ADD/SUB instructions */
#define LDST_MASK 0xFF7FF000	/* Mask for LDR/STR instruction */
#define BR_MASK 0xFF000000	/* Branch instruction mask */
#define BROFF_MASK 0xFFFFFF	/* Branch offset mask */
#define SHIFT_MASK 0xF		/* Mask to extract shift bits from encoded constant */
#define SHIFT_SHIFT 8		/* Number of bits to shift shift bits in encoded constant */
#define NIBBLE_MASK 0xF		/* Mask to extract low four bits of a value */
#define BYTE_MASK 0xFF		/* Mask to extract a byte */
#define DEST_MASK 0xF000	/* Mask to extract the destination register in instructions */
#define RNRD_MASK 0xFF000	/* Mask to extract registers Rn and Rd from instructions */
#define LOW2_MASK 3		/* Mask for low-order two bits of word */

#define BIT23SIGN   0x00800000
#define BIT24EXTEND 0xFF000000
#define BIT15SIGN   0x00008000
#define BIT16EXTEND 0xFFFF0000

static unsigned int current_objsize,	/* Size of area containing current relocations */
  totalrelocs,			/* Total number of word additive relocations */
  symbolcount,			/* Count of symbols in OBJ_SYMT chunk */
  unused,			/* Number of unused areas */
  areapc,			/* Value of program counter at start of area */
  areatop;			/* Offset of end of last area */

static arealist *current_area;	/* Ptr to area list entry being worked on */
static filelist *current_file;	/* Ptr to file list entry of file being worked on */

static unsigned int *areastart,	/* Ptr to area being worked on */
 *headercode,			/* Ptr to AIF header to be added to executable image */
 *thisarea;			/* Pointer to arealist entry for area being processed */

static int entryareanum;	/* Number of area containing program entry point */

static bool gotcodearea,	/* TRUE if the first code area has been found */
  noheader;			/* TRUE if the program does not have a header on it */

static arealist *defaultarea,	/* Default entry point if no entry point given */
 *areablock;			/* Memory allocated to hold arealist structures */

typedef struct arealimits
{
  const char *areaname;		/* Ptr to name of area */
  symtentry *areabase;		/* Pointer to SYMT entry of area's 'base' symbol */
  symtentry *arealimit;		/* Pointer to SYMT entry of area's 'limit' symbol */
  struct arealimits *left;
  struct arealimits *right;
} arealimits;

static arealimits *arlimlist;

static areasrchlist *areatable[MAXSRCH];	/* Area symbol table */

static void list_attributes (const char *filename, unsigned int attr);
static arealist *add_newarea (filelist * fp, areaentry * aep, unsigned int atattr, unsigned int alattr);

/*
** 'extend24' sign extends the 24-bit value passed to it to 32 bits
*/
static unsigned int
extend24 (unsigned int x)
{
  if ((x & BIT23SIGN) != 0)
    x = x | BIT24EXTEND;
  return x;
}

/*
** 'decode_armconst' converts the constant value coded in an ARM
** arithmetic instruction passed to it to its proper value,
** returning that  value. It effectively does a 'rotate right' of
** the value held in the low eight bits of the instruction by two
** times the value stored in bits nine to twelve.
*/
static unsigned int
decode_armconst (unsigned int value)
{
  unsigned int shift, low2bits;
  shift = (value >> SHIFT_SHIFT) & SHIFT_MASK;	/* Extract no. of bits by which to shift value */
  value = value & BYTE_MASK;	/* Extract unshifted value */
  while (shift != 0)
    {
      low2bits = value & LOW2_MASK;
      value = value >> 2 | low2bits << 30;
      shift -= 1;
    }
  return value;
}

/*
** 'check_commondefref' makes sure that when equally named COMDEF and COMMON
** area share the same symbol and don't have any conflicting attributes.
*/
static void
check_commondefref (const char *areaname,
                    unsigned int atattr_def, unsigned int size_def,
                    const char *file_def,
                    unsigned int atattr_ref, unsigned int size_ref,
                    const char *file_ref)
{
  if (size_ref > size_def)
    error ("Error: Common Block area '%s' has a COMMON instance in '%s' which is bigger in size than its COMDEF instance in '%s'",
           areaname, file_ref, file_def);

  atattr_def &= ~ATT_COMDEF;
  atattr_ref &= ~(ATT_COMMON | ATT_NOINIT);
  if (atattr_def != atattr_ref)
    {
      error ("Warning: Attributes of Common Block area '%s' in '%s' conflict with those in '%s'",
             areaname, file_ref, file_def);
      list_attributes (file_ref, atattr_ref);
      list_attributes (file_def, atattr_def);
    }
}

/*
** 'check_commondef' is called when a code or data area has the 'common
** definition' bit set. This is only used for AOF V3 object code files.
** The function checks to see if an area of the same name already
** exists in the relevant area list. It assumes there will be only one
** occurence at most, with the 'common definition' bit set. If the first
** area does not have this attribute then the linker complains. (Whether
** or not all areas with the same name should have the same attributes is
** not specified in the AOF docs but it makes sense to me that they should
** be.)
** The function returns either a pointer to the arealist entry if an area
** of that name already exists or NULL if it is a new area or an error
** occurs.
**
** Note that these areas do not appear in the common block symbol table
** as they are not seen as common blocks as such: they are instances
** of the same code or the same data across several AOF files where only
** one instance is to be kept in the image file. The 'common definition'
** attribute is being used to mark these areas.
**
** Note that under AOF V3, it is possible to have multiple definitions
** of a common block and not just a single one as in AOF V2. If a
** program being linked contains nothing but AOF V2 files then this
** restriction is maintained; otherwise the AOF V3 rule applies.
**
** 'current_file' needs to be set.
*/
static arealist *
check_commondef (const char *atname, unsigned int atsize, unsigned int atattr)
{
  arealist *ap;

  /* Figure out which list to check */
  if ((atattr & ATT_CODE) != 0)
    {				/* Code area */
      ap = (atattr & ATT_RDONLY) != 0 ? rocodelist : rwcodelist;
    }
  else
    {				/* Data area */
      ap = (atattr & ATT_RDONLY) != 0 ? rodatalist : rwdatalist;
    }

  while (ap != NULL)
    {
      int compres = strcmp (atname, ap->arname);

      if (compres == 0)
	break;
      ap = (compres > 0) ? ap->right : ap->left;
    }

  if (ap == NULL)
    {
      symbol *sp;
      arealist *ref_ap;

      /* Check if the area isn't already known as a COMMON area.  */
      sp = find_common (atname);
      if (sp == NULL)
        return NULL;		/* Common block is unknown */

      ref_ap = sp->symtptr->symtarea.areaptr;
      /* ref_ap must be a COMMON area, if it would be a COMDEF area,
         we wouldn't be in the if (ap == NULL) case.  */

      check_commondefref (atname,
                          atattr, atsize,
                          current_file->chfilename,
                          ref_ap->aratattr, ref_ap->arobjsize,
                          ref_ap->arfileptr->chfilename);

      return NULL;
    }
  else
    {
      if (!aofv3flag)
        {	/* AOF V2 - Cannot have two definitions of a common block */
          error ("Error: There is already a definition (in '%s') of Common Block '%s' in '%s'",
                 ap->arfileptr->chfilename, atname, current_file->chfilename);
          /* This area will be added to the area list as a normal area */
          return NULL;
        }

      if (ap->aratattr != atattr)
        {	/* Known COMDEF area. Check attributes are the same */
          error ("Warning: Attributes of common area '%s' in '%s' conflict with those in '%s'",
	         atname, current_file->chfilename, ap->arfileptr->chfilename);
          /* This area will be added to the area list as a normal area */
          return NULL;
        }

      /* The GNU linkonce attribute allows us to combine multiple COMDEF
         areas of the same name which contain different data.  The choice
         of which is indetermined.  We cannot verify for either common
         code-size or actual content because areas of the same name may have
         been compiled with different compilation options.
         Another reason why the COMDEF areas might be different is that for
         CODE areas the reallocation hasn't been done yet.  If it would have
         been done, then the area contents might be equal.  */
      if ((atattr & ATT_LINKONCE) == 0)
        {
          if (atsize != ap->arobjsize)
	    {
	      /* hack: error() only allows 4 params */
	      char buffer[256];
	      sprintf (buffer, "(%d != %d)", atsize, ap->arobjsize);
	      error ("Error: Size of common area '%s' in '%s' differs from definition in '%s' %s)",
	             atname, current_file->chfilename, ap->arfileptr->chfilename, buffer);
	      return NULL;
	    }

          /* Verify areas' contents are the same */
          if (memcmp(thisarea, ap->arobjdata, ap->arobjsize))
	    {
	      error ("Error: Contents of common area '%s' in '%s' differ from those in '%s'",
	             atname, current_file->chfilename, ap->arfileptr->chfilename);
	      return NULL;
	    }
        }
    }

  return ap;
}

/*
** 'check_commonref' is called if the area is a common block reference or
** a definition with the 'no data' attribute. If the common block refers to
** one previously encountered, it returns a pointer to its area list entry
** after updating the size and/or attributes otherwise it returns NULL.
**
** 'current_file' needs to be defined.
*/
static arealist *
check_commonref (const char *atname, unsigned int atsize, unsigned int atattr)
{
  symbol *sp;
  arealist *ap;

  if ((atattr & ATT_COMDEF) && (atattr & ATT_NOINIT))
    atattr -= ATT_COMDEF | ATT_NOINIT;

  sp = find_common (atname);
  if (sp == NULL)
    return NULL;		/* Symbol not in table - Unknown common block */
  ap = sp->symtptr->symtarea.areaptr;

  if ((ap->aratattr & ATT_COMDEF) != 0)
    {	/* There is COMDEF already */
      check_commondefref (atname,
                          ap->aratattr, ap->arobjsize,
                          ap->arfileptr->chfilename,
                          atattr, atsize,
                          current_file->chfilename);
    }
  else if (atsize > ap->arobjsize)
    {	/* Common block reference new size is bigger. */
    ap->arobjsize = atsize;
    }

  return ap;
}

static char *
make_name (const char *s1, const char *s2)
{
  char *p;
  if ((p = allocmem (strlen (s1) + strlen (s2) + sizeof (char))) == NULL)
    {
      error ("Fatal: Out of memory in 'make_name'");
    }
  else
    {
      strcpy (p, s1);
      strcat (p, s2);
    }
  return p;
}

/*
** 'list_attributes' lists the area attributes of the area passed to it
*/
static void
list_attributes (const char *filename, unsigned int attr)
{
  char text[MSGBUFLEN];
  unsigned int count, n;
  struct
  {
    unsigned int atmask;
    const char *atname;
  } attributes[] =
  {
    { ATT_CODE, "Code" },
    { ATT_COMDEF, "Common definition" },
    { ATT_COMMON, "Common reference" },
    { ATT_NOINIT, "Zero-initialised" },
    { ATT_RDONLY, "Read only" },
    { ATT_SYMBOL, "Debugging tables" },

    { ATT_ABSOL, "Absolute" },
    { ATT_POSIND, "Position independent" },
    { ATT_32BIT, "32-bit APCS" },
    { ATT_REENT, "Reentrant code" },
    { ATT_EXTFP, "Extended FP instructions" },
    { ATT_NOSTAK, "No stack checking code" },
    { ATT_BASED, "Based area" },
    { ATT_STUBS, "Shared library stub data" },
    { ATT_SOFTFLOAT, "Soft float" },
    { ATT_LINKONCE, "Linkonce area" },
    { 0, "*"}
  };

  count = 0;
  sprintf (text, "    %s:  ", filename);
  for (n = 0; attributes[n].atname[0] != '*'; n++)
    {
      if ((attr & attributes[n].atmask) != 0)
	{
	  if (count != 0)
	    strcat (text, ", ");
	  strcat (text, attributes[n].atname);
	  count++;
	}
    }
  error (text);
}

/*
** 'insert_area' inserts the area entry passed to it in the correct
** place (alphabetical order) in the area list passed to the routine.
** If there are duplicate names, these are added so that they appear
** in the order in which they were defined
*/
static void
insert_area (arealist ** list, arealist * newarea)
{
  int compres;
  arealist *ap, *lastarea;
  const char *name;
  arealimits *lp;

  lastarea = NULL;
  name = newarea->arname;
  compres = 1;

  for (ap = *list; ap != NULL; )
    {
      lastarea = ap;
      compres = strcmp (name, ap->arname);

      /* Areas with the same name are put to the left */
      ap = (compres > 0) ? ap->right : ap->left;
    }

  if (lastarea == NULL)
    *list = newarea;
  else
    {
      if (compres > 0)
	lastarea->right = newarea;
      else
	lastarea->left = newarea;
    }

  if (compres == 0)
    {				/* Name already in list. Propagate base pointer */
      if (newarea->aratattr != lastarea->aratattr)
	{			/* Check attributes are the same */
	  error ("Warning: Attributes of area '%s' in '%s' conflict with those in '%s'",
	         name, newarea->arfileptr->chfilename,
	         lastarea->arfileptr->chfilename);
	  list_attributes (newarea->arfileptr->chfilename, newarea->aratattr);
	  list_attributes (lastarea->arfileptr->chfilename, lastarea->aratattr);

	  if ((newarea->aratattr ^ lastarea->aratattr) & ATT_SOFTFLOAT)
	    error ("Error: You cannot mix soft-float and non soft-float code");
	}
      if (newarea->aralign != lastarea->aralign)
	{			/* Check align attributes are the same */
	  error ("Warning: Align attributes of area '%s' in '%s' conflict with those in '%s' (%d vs %d)",
	         name,
	         newarea->arfileptr->chfilename,
	         lastarea->arfileptr->chfilename,
	         newarea->aralign,
	         lastarea->aralign);
	}
      newarea->arbase = lastarea->arbase;
      lastarea->arbase->arlast = newarea;
    }
  else
    {	/* New area name. Add to base/limit list and symbol table */
      arealimits **insert;

      newarea->arbase = newarea;
      lp = allocmem (sizeof (arealimits));
      if (lp == NULL)
	error ("Fatal: Out of memory in 'insert_area' handling area '%s' in file '%s'",
	       name, newarea->arfileptr->chfilename);

      lp->areaname = name;
      if (imagetype != AOF)
        {
          lp->areabase = make_symbol (make_name (name, "$$Base"), SYM_GLOBAL)->symtptr;
          lp->arealimit = make_symbol (make_name (name, "$$Limit"), SYM_GLOBAL)->symtptr;
        }
      else
        {	/* Do not want base/limit pair when creating AOF file */
          lp->areabase = lp->arealimit = NULL;
        }
      lp->left = lp->right = NULL;

      for (insert = &arlimlist; *insert; )
        {
          int compval = strcmp (name, (*insert)->areaname);

          insert = (compval > 0) ? &(*insert)->right : &(*insert)->left;
        }
      *insert = lp;
    }
}

/*
** 'keep_area' is called to check if the area 'ap' should be
** marked as 'non-deleteable' and therefore should not be removed
** when weeding out unreferenced areas. At present this only
** affects some GCC-specific areas.
*/
static bool
keep_area (arealist * ap)
{
  return opt_gccareas && strncmp (ap->arname, GCCAREA, GCCAREALEN) == 0;
}

/*
** 'add_newarea' creates a new area structure and adds it to the relevant
** area list. It returns a pointer to the entry if it was successfully
** created or NULL.
*/
static arealist *
add_newarea (filelist * fp, areaentry * aep, unsigned int atattr,
	     unsigned int alattr)
{
  arealist *ap;
  symtentry *sp;

  ap = areablock;	/* Take memory for entry from block allocated for all areas in file */
  areablock++;
  ap->arname = fp->obj.strtptr + aep->areaname;
  ap->arfileptr = fp;
  ap->aratattr = atattr;
  ap->aralign = alattr;
  ap->arobjsize = aep->arsize;
  ap->arobjdata = thisarea;
  if ((atattr & ATT_NOINIT) == 0)
    ap->areldata = COERCE (COERCE (thisarea, char *) + aep->arsize, relocation *);
  else
    ap->areldata = COERCE (thisarea, relocation *);
  ap->arnumrelocs = aep->arelocs;
  ap->arefcount = (!opt_nounused || keep_area (ap)) ? 1 : 0;

  ap->areflist = NULL;
  /* AOF 3 absolute address area */
  ap->arplace = (aofv3flag && (atattr & ATT_ABSOL)) ? aep->araddress : 0;

  ap->arsymbol = NULL;
  ap->left = ap->right = NULL;
  ap->arlast = NULL;
  if ((atattr & ATT_SYMBOL) != 0)
    {				/* Debugging info area */
      ap->arefcount = 1;	/* Debug areas are never deleted */
      insert_area (&debuglist, ap);
      debugsize += aep->arsize;
    }
  else if ((atattr & ATT_COMMON)
	   || ((atattr & ATT_COMDEF) && (atattr & ATT_NOINIT)))
    {				/* Common block ref */
      if (imagetype != AOF)
	{			/* Do not want symbol created when linking partially-linked AOF file */
	  ap->arsymbol = make_symbol (ap->arname, SYM_COMMON);
	  sp = ap->arsymbol->symtptr;	/* Fill in 'area' field of cb's SYMT entry */
	  sp->symtarea.areaptr = ap;
	}
      insert_area (&zidatalist, ap);
    }
  else
    {
      if ((atattr & ATT_COMDEF) != 0)
        {
          if (imagetype != AOF)
	    {			/* Do not want symbol created when linking partially-linked AOF file */
	      ap->arsymbol = make_symbol (ap->arname, SYM_COMMON);
	      sp = ap->arsymbol->symtptr;	/* Fill in 'area' field of cb's SYMT entry */
	      /* When sp->symtarea.areaptr is non-NULL, this mean that there
	         was a previous COMMON area, which we no longer need.  */
	      if (sp->symtarea.areaptr != NULL)
	        sp->symtarea.areaptr->arefcount = 0;
	      sp->symtarea.areaptr = ap;
	    }
        }

      if ((atattr & ATT_CODE) != 0)
        {				/* Code areas */
          got_32bitapcs = got_32bitapcs || ((atattr & ATT_32BIT) != 0);
          got_26bitapcs = got_26bitapcs || ((atattr & ATT_32BIT) == 0);
          if ((atattr & ATT_RDONLY) != 0)
	    insert_area (&rocodelist, ap);
          else
	    insert_area (&rwcodelist, ap);
        }
      else if ((atattr & ATT_RDONLY) != 0)
        insert_area (&rodatalist, ap);	/* Data area with R/O bit set */
      else if ((atattr & ATT_NOINIT) == 0)
        insert_area (&rwdatalist, ap);	/* Data with R/O and 'zeroinit' bits clear */
      else
        insert_area (&zidatalist, ap);	/* Data with 'zeroinit' bit set */
    }

  if (!gotcodearea && (atattr & ATT_CODE) != 0)
    {
      gotcodearea = TRUE;
      defaultarea = ap;
    }

  return ap;
}

/*
** 'add_commonarea' is called to create an arealist structure for a common
** block that is referenced only by an entry in the OBJ_SYMT chunk with the
** 'common' attribute set. It returns a pointer to the structure created.
*/
static arealist *
add_commonarea (const char *name, unsigned int size)
{
  arealist *ap;
  ap = allocmem (sizeof (arealist));
  if (ap == NULL)
    error ("Fatal: Out of memory in 'add_commonarea'");
  ap->arname = name;
  ap->left = ap->right = NULL;
  ap->arfileptr = current_file;
  ap->aratattr = ATT_COMMON | ATT_NOINIT;
  ap->aralign = DEFALIGN;
  ap->arobjsize = size;
  ap->arobjdata = NULL;
  ap->areldata = NULL;
  ap->arnumrelocs = 0;
  ap->arefcount = (opt_nounused) ? 0 : 1;
  ap->areflist = NULL;
  ap->arplace = 0;
  ap->arsymbol = NULL;
  ap->arlast = NULL;
  return ap;
}

/*
** 'make_commonarea' is called when a symbol with the 'common' attribute is
** found in the OBJ_SYMT chunk of a file. It checks to see if the common
** block is known and if so just checks the size. If not known, it add it
** to the common block list. The function returns a pointer to the symbol
** table entry for the common block definition
*/
symtentry *
make_commonarea (symbol * sp)
{
  unsigned int size;
  symbol *cbsp;
  symtentry *stp;
  arealist *ap;
  stp = sp->symtptr;
  size = stp->symtvalue;
  cbsp = search_common (sp);
  if (cbsp == NULL)
    {				/* Common block is not known */
      ap = add_commonarea (stp->symtname, size);
      ap->arsymbol = make_symbol (ap->arname, SYM_COMMON);
      stp = ap->arsymbol->symtptr;	/* Point at dummy SYMT entry for CB definition */
      stp->symtarea.areaptr = ap;
      if (imagetype != AOF)
	{			/* Only add area to list if not partially-linked AOF file */
	  insert_area (&zidatalist, ap);
	}
    }
  else
    {				/* Known common block */
      stp = cbsp->symtptr;
      ap = stp->symtarea.areaptr;
      if ((ap->aratattr & ATT_COMDEF) != 0)
	{			/* Common block size set by a 'definition' entry */
	  if (size > ap->arobjsize)
	    error ("Size of reference to Common Block '%s' in '%s' is greater than its defined value",
		   sp->symtptr->symtname, current_file->chfilename);
	}
      else
	{			/* Size not set */
	  if (size > ap->arobjsize)
	    ap->arobjsize = size;
	}
    }

  return stp;
}

/*
** 'add_srchlist' adds the area entry passed to it to the
** area search table
*/
static void
add_srchlist (arealist * ap)
{
  int hashval;
  areasrchlist *sp;
  if ((sp = allocmem (sizeof (areasrchlist))) == NULL)
    error ("Fatal: Out of memory in 'add_srchlist'");
  sp->srcarea = ap;
  sp->srchash = hashval = hash (ap->arname);
  sp->srcflink = areatable[hashval & SRCHMASK];
  areatable[hashval & SRCHMASK] = sp;
}

/*
** 'free_srchlist' returns the memory used by the area search list
** to the heap
*/
void
free_srchlist (void)
{
  int i;
  areasrchlist *sp, *nextsp;
  for (i = 0; i < MAXSRCH; i++)
    {
      for (sp = areatable[i]; sp != NULL; )
	{
	  nextsp = sp->srcflink;
	  freemem (sp, sizeof (areasrchlist));
	  sp = nextsp;
	}
    }
}

/*
** 'scan_head' is called to scan through the OBJ_HEAD chunk and add
** entries from it to the various area linked lists. It returns 'true'
** if this was accomplished otherwise it returns 'false'. Note that
** memory is allocated to hold all the 'arealist' entries that will
** be generated for efficiency. Note also the 'clever' way of handling
** the area containing an entry point.
**
** Zero-length areas in the OBJ_AREA chunk are ignored. This has to be
** included to get round what looks like a bug in partially-linked AOF
** files produced by the Acorn linker as it creates zero-length areas
** with invalid area attributes.
**
** Another fudge is the check for the 'AL' attribute value. The AOF
** version 2 definition says this should always be set to two but
** the Acorn assembler, Objasm, sets it to zero in entries for debug
** tables. The code complains if it is greater than two in AOF V2
** files
*/
bool
scan_head (filelist * fp)
{
  arealist *ap = NULL;		/* Points at arealist entry created for entry being checked */
  objheadhdr *ahp;		/* Points at OBJ_HEAD chunk of file */
  areaentry * aep;		/* Points at OBJ_HEAD entry being checked */
  int count, areaco;
  unsigned int totalsize, areasize, strtsize;
  bool ok;

  for (count = 0; count < MAXSRCH; count++)
    areatable[count] = NULL;
  current_file = fp;
  ahp = fp->obj.headptr;
  if (ahp->areaheader.oftype != OBJFILETYPE)
    {
      error ("Error: '%s' does not appear to be an AOF file (type=%x)",
	     fp->chfilename, ahp->areaheader.oftype);
      return FALSE;
    }
  fp->aofv3 = ahp->areaheader.aofversion >= AOFVER3;	/* Got an AOF version 3.1 file */
  aofv3flag = aofv3flag || fp->aofv3;
  if (ahp->areaheader.aofversion > AOFVERSION)
    {
      error ("Error: The version of AOF used in '%s' (%d.%02d) is not supported",
	     fp->chfilename, ahp->areaheader.aofversion / 100,
	     ahp->areaheader.aofversion % 100);
      return FALSE;
    }
  count = ahp->areaheader.numareas;
  if (sizeof (aofheader) + count * sizeof (areaentry) > fp->obj.headsize)
    {
      error ("Error: Area count in 'OBJ_HEAD' chunk in '%s' is too large. Is file corrupt?",
	     fp->chfilename);
      return FALSE;
    }

  fp->areacount = count;
  fp->symtcount = ahp->areaheader.numsymbols;
  entryareanum = ahp->areaheader.eparea - 1;	/* -1 as 'areaco' goes from 0, not 1 */
  if (entryareanum >= 0)
    entryoffset = ahp->areaheader.epoffset;

  /* Allocate block to hold arealist entries for file */
  if ((areablock = allocmem (count * sizeof (arealist))) == NULL)
    error ("Fatal: Out of memory in 'scan_head' reading '%s'",
	   fp->chfilename);

  ok = TRUE;
  totalsize = 0;
  thisarea = fp->obj.areaptr;
  areasize = fp->obj.areasize;
  strtsize = fp->obj.strtsize;

  for (areaco = 0, aep = &ahp->firstarea; areaco < count && ok; ++areaco, ++aep)
    {
      unsigned int atattr, alattr;
      const char *atname;

      if (aep->areaname > strtsize)
	{
	  error ("Error: Area name offset in area %d in '%s' is too big",
		 areaco + 1, fp->chfilename);
	  ok = FALSE;
	}

      atattr = aep->attributes >> 8;
      alattr = aep->attributes & 0xFF;
      atname = aep->areaname + fp->obj.strtptr;

      if (!aofv3flag && alattr > ALBYTE)
	{			/* Check 'al' byte */
	  error ("Error: Found bad 'al' attribute byte in area '%s' (%d) in '%s'",
		 atname, areaco + 1, fp->chfilename);
	  ok = FALSE;
	}
      else if (aofv3flag)
	{			/* Checks for 'al' byte in AOF 3 */
	  if (alattr < DEFALIGN || alattr > MAXV3AL)
	    {
	      error ("Error: Area alignment value in area '%s' (%d) in '%s' is outside range 2 to 32",
		     atname, areaco + 1, fp->chfilename);
	      ok = FALSE;
	    }
	}

      if (aofv3flag && (atattr & ATT_UNSUPP) != 0)
	{			/* Reject unsupported AOF 3 area attributes */
	  error ("Error: Area '%s' (%d) of '%s' contains unsupported area attributes (%06x)",
	         atname, areaco + 1, fp->chfilename, atattr & ATT_UNSUPP);
	  ok = FALSE;
	}

      if ((atattr & ATT_SYMBOL) != 0)
	{			/* Debugging area */
	  if ((atattr & ATT_BADSYM) != 0)
	    {			/* Check only legal symbol attributes */
	      error ("Error: Area '%s' (%d) %d of '%s' has unsupported attributes (0x%06x)",
		     atname, areaco + 1, fp->chfilename, atattr);
	      ok = FALSE;
	    }
	}
      else if ((atattr & ATT_CODE) != 0)
	{			/* code area */
	  if ((atattr & ATT_BADCODE) != 0)
	    {			/* Check only legal code attributes */
	      error ("Error: Area '%s' (%d) of '%s' has unsupported attributes (0x%06x)",
		     atname, areaco + 1, fp->chfilename, atattr);
	      ok = FALSE;
	    }
	}
      else
	{			/* Data area */
	  if ((atattr & ATT_BADATA) != 0)
	    {			/* Check only legal data attributes */
	      error ("Error: Area '%s' (%d) of '%s' has unsupported attributes (0x%06x)",
		     atname, areaco + 1, fp->chfilename, atattr);
	      ok = FALSE;
	    }
	}

      /* ATT_NOINIT and ATT_RDONLY are incompatible. */
      if ((atattr & ATT_NOINIT) && (atattr & ATT_RDONLY))
        {
          error ("Warning: Area '%s' (%d) in '%s' has both 'NOINIT' and 'READONLY' attributes specified",
                 atname, areaco + 1, fp->chfilename);
          ok = FALSE;
        }

      if ((atattr & ATT_NOINIT) == 0)
	{			/* Area is present in AOF file */
	  totalsize += aep->arsize;
	  if (totalsize > areasize)
	    {
	      error ("Error: Area '%s' (%d) in '%s' is too big",
		     atname, areaco + 1, fp->chfilename);
	      ok = FALSE;
	    }
	}
      totalsize += aep->arelocs * RELOCSIZE;
      if (totalsize > areasize)
	{
	  error ("Error: Number of relocations in area '%s' (%d) in '%s' is bad",
		 atname, areaco + 1, fp->chfilename);
	  ok = FALSE;
	}
      if (!fp->aofv3)
        {
          /* AOF 2 check only */
          if (aep->araddress != 0)
	    {
	      error ("Error: Last word of definition of area '%s' (%d) in '%s' is not zero",
	             atname, areaco + 1, fp->chfilename);
	      ok = FALSE;
	    }
	}
      else
        {
          /* AOF 3 check only */
          if ((atattr & ATT_ABSOL) == 0 && aep->araddress != 0)
	    {
	      error ("Error: Base address of area '%s' (%d) in '%s' is not zero but area is not absolute positioned",
	             atname, areaco + 1, fp->chfilename);
	      ok = FALSE;
	    }
	}

      if (ok)
	{
	  /* Note that ATT_COMDEF + ATT_NOINIT is equal to ATT_COMMON */
	  if ((atattr & ATT_COMMON) || ((atattr & ATT_COMDEF) && (atattr & ATT_NOINIT)))
	    {		/* Extra checks for common blocks */
	      ap = check_commonref (atname, aep->arsize, atattr);
	    }
	  else if ((atattr & ATT_COMDEF) != 0)
	    {		/* Common definition with code or data */
	      ap = check_commondef (atname, aep->arsize, atattr);
	    }
	  else
	    ap = NULL;

	  if (ap == NULL)
	    {		/* Not a known common block or new area found */
	      if ((atattr & ATT_SYMBOL) == 0 || fp->keepdebug)
		{	/* Area or new common block */
		  ap = add_newarea (fp, aep, atattr, alattr);
		  ok = ap != NULL;
		}
	    }
	}
      else
        ap = NULL;

      if ((atattr & ATT_NOINIT) == 0)
        thisarea = COERCE (COERCE (thisarea, char *)
                           + aep->arsize + aep->arelocs * RELOCSIZE, unsigned int *);
      else
        thisarea = COERCE (COERCE (thisarea, char *)
                           + aep->arelocs * RELOCSIZE, unsigned int *);

      if (ap != NULL)
	{
	  fp->obj.arealistptrs[areaco] = ap;
	  add_srchlist (ap);
	}
      if (areaco == entryareanum)
	{
	  if (entryarea != NULL)
	    error ("Error: Program has multiple entry points (first '%s', second '%s')",
	           entryarea->arfileptr->chfilename, fp->chfilename);
	  else if ((atattr & ATT_CODE) == 0)
	    error ("Warning: Entry point for program in '%s' is in a data area, not code",
	           fp->chfilename);
	  entryarea = ap;
	}
    }

  return ok;
}

/*
** 'get_type1_type' returns the relocation type information from a
** type 1 relocation
*/
int
get_type1_type (int reltype)
{
  return reltype >> TYPE1_SHIFT;
}

/*
** 'get_type1_index' returns the index of a symbol's entry in the
** OBJ_SYMT chunk from a type 1 relocation
*/
int
get_type1_index (int reltype)
{
  return reltype & TYPE1_MASK;
}

/*
** 'get_type2_type' returns the relocation type information from a
** type 2 relocation
*/
int
get_type2_type (int reltype)
{
  return reltype >> TYPE2_SHIFT;
}

/*
** 'get_type2_index' returns the index of a symbol's entry in the
** OBJ_SYMT chunk or an area's index in the OBJ_HEAD chunk from a
** type 2 relocation
*/
int
get_type2_index (int reltype)
{
  return reltype & TYPE2_MASK;
}

/*
** 'check_strongrefs' is called if an AOF file contains any strong symbol
** definitions. It checks through all of the relocations in the file to
** see if there are any references to those symbols. If so, then it has to
** create 'external reference' OBJ_SYMT entries for those relocations so
** that the symbol resolution code can set up links to the correct symbols
*/
void
check_strongrefs (filelist * fp)
{
  areaentry *ap;		/* Points at entry in OBJ_HEAD chunk */
  relocation *rp;		/* Points at entry in relocation part of OBJ_AREA chunk */
  symtentry *stp;		/* Points at entry in OBJ_SYMT chunk */
  symbol *sp;
  unsigned int refcount, i, j, reltype;
  refcount = 0;
  ap = &fp->obj.headptr->firstarea;
  rp = COERCE (fp->obj.areaptr, relocation *);
  for (i = 1; i <= fp->areacount; ++i, ++ap)
    {
      rp = COERCE (COERCE (rp, char *) + ap->arsize, relocation *);	/* Point at start of relocations */
      for (j = 1; j <= ap->arelocs; ++j, ++rp)
	{
	  reltype = rp->reltypesym;
	  stp = NULL;
	  if ((reltype & REL_TYPE2) == 0)
	    {			/* Got a type-1 relocation */
	      if ((get_type1_type (reltype) & (REL_SYM | REL_PC)) != 0)
		{		/* Symbol relocation */
		  stp = fp->obj.symtptr + get_type1_index (reltype);
		}
	    }
	  else
	    {
	      if ((get_type2_type (reltype) & REL_SYM) != 0)
		{
		  stp = fp->obj.symtptr + get_type2_index (reltype);
		}
	    }
	  if (stp != NULL && (stp->symtattr & SYM_MASK_STRONGDEF) == SYM_MASK_STRONGDEF)
	    {
	      refcount++;
	      sp = create_externref (stp);
	    }
	}
    }
}

/*
** 'find_area' is called to locate the area in which the area name 'np' is
** located. It returns a pointer to the 'arealist' entry of that area or
** 'NULL' if it cannot find the area
*/
arealist *
find_area (const char *np)
{
  arealist *ap;
  areasrchlist *sp;
  int hashval;
  hashval = hash (np);
  sp = areatable[hashval & SRCHMASK];
  while (sp != NULL
	 && (hashval != sp->srchash || strcmp (sp->srcarea->arname, np) != 0))
    sp = sp->srcflink;

  ap = (sp != NULL) ? sp->srcarea : NULL;

  return ap;
}

/*
** 'get_area' is called to return the area list entry for the area with
** area index 'index'. It is used when handling type-2 relocations
**
** 'current_file' needs to be set.
**
** FIXME: add sanity check 'index' is less than number of areas.
*/
static arealist *
get_area (unsigned int index)
{
  areaentry *temp = &current_file->obj.headptr->firstarea;
  const areaentry *ae = &temp[index];
  unsigned int atattr = ae->attributes >> 8;

  /* If it is a COMMON area, find the corresponding COMDEF area.
     If there isn't a COMDEF area with the same name (yet seen), we fall
     back on the same COMMON area.  When the COMDEF area turns up later,
     we'll resolve that at that time.  */
  if ((atattr & ATT_COMMON)
      || ((atattr & ATT_COMDEF) && (atattr & ATT_NOINIT)))
    {
      const char *arname = current_file->obj.strtptr + ae->areaname;
      symbol *comsym = find_common (arname);
      return comsym->symtptr->symtarea.areaptr;
    }

  return current_file->obj.arealistptrs[index];
}

/*
** 'decr_refcount' goes through an area's list of referenced areas and
** decrements the reference count for that area. If the count goes to
** zero then that area is not referenced and so the routine calls
** itself recursively to go through all the areas that that one
** references to decrement their use counts and so on
*/
static void
decr_refcount (arearef * rp)
{
  for (; rp != NULL; rp = rp->arefnext)
    {
      arealist *ap = rp->arefarea;
      if ((ap->arefcount -= rp->arefcount) == 0)
	{
	  decr_refcount (ap->areflist);
	  ap->areflist = NULL;	/* So that mark_unused does not do them again */
	}
    }
}

/*
** 'mark_unused' goes through an area list and weeds out all the
** unreferenced areas.
*/
static void
mark_unused (arealist * ap)
{
  if (ap != NULL)
    {
      mark_unused (ap->left);

      if (ap->arefcount == 0)
	{
	  unused++;
	  decr_refcount (ap->areflist);
	  ap->areflist = NULL;
	}

      mark_unused (ap->right);
    }
}

/*
** 'add_arearef' adds to reference to area 'to' from area 'from' and
** bumps up the reference count in area 'to'. If a symbol is
** absolute or one of the linker predefined symbols, it is not
** included.
*/
static void
add_arearef (arealist * fromarea, arealist * toarea)
{
  arearef *rp;
  if (fromarea != toarea)
    {				/* Don't want refs to this area */
      rp = fromarea->areflist;
      while (rp != NULL && rp->arefarea != toarea)
	{			/* First ref to this area? */
	  rp = rp->arefnext;
	}
      if (rp != NULL)
	{			/* Already seen this area */
	  rp->arefcount += 1;
	}
      else
	{			/* new area ref */
	  rp = allocmem (sizeof (arearef));
	  if (rp == NULL)
	    {
	      error ("Fatal: Out of memory in 'add_arearef'");
	    }
	  else
	    {
	      rp->arefarea = toarea;
	      rp->arefcount = 1;
	      rp->arefnext = fromarea->areflist;
	      fromarea->areflist = rp;
	    }
	}
      toarea->arefcount++;
    }
}

/*
** 'add_symref' adds reference 'symtindex' from area 'from' to
** the count of references to area containing the symbol. If
** a symbol is absolute or one of the linker predefined symbols,
** it is not included.
*/
static void
add_symref (arealist * fromarea, int symtindex)
{
  symtentry *sp;

  sp = symtbase + symtindex;
  if ((sp->symtattr & (SYM_WEAKREF | SYM_MASK_DEFN)) == 0)
    sp = sp->symtarea.symdefptr;	/* Was an external ref */
  else if ((sp->symtattr & SYM_MASK_STRONGDEF) == SYM_MASK_STRONGDEF)
    sp = find_nonstrong (sp);	/* Direct reference to strong symbol */

  if ((sp->symtattr & (SYM_LINKDEF | SYM_ABSVAL | SYM_WEAKREF)) != 0)
    return;		/* Symbol is absolute, predefined or weak external */

  add_arearef (fromarea, sp->symtarea.areaptr);
}

/*
** 'find_arearefs' is used to determine which areas are referenced
** from other areas in one area list. It works this out from the
** relocation data for each area.
** There are two cases to consider, symbol references and area
** references. In a type-1 relocation, only symbol references have
** to be considered as area (interal) relocation always refer to the
** area containing the reference and we are not interested in these.
** In a type-2 relocation, the area referenced can be a different
** area, so both cases do have to be considered. Furthermore, with
** type-2 relocations, the relocation can be either PC-relative
** or additive. Of course, an area reference can refer to the same
** area as the one containing the relocation but add_arearef will
** weed out such relocations.
*/
static void
find_arearefs (arealist * ap)
{
  if (ap != NULL)
    {
      int numrelocs = ap->arnumrelocs;

      find_arearefs (ap->left);

      if (numrelocs != 0)
	{
	  relocation *rp;
	  int n;
	  current_file = ap->arfileptr;
	  symtbase = ap->arfileptr->obj.symtptr;
	  symbolcount = ap->arfileptr->symtcount;

	  for (n = 1, rp = ap->areldata; n <= numrelocs; ++n, ++rp)
	    {
	      int relword = rp->reltypesym;

	      if ((relword & REL_TYPE2) == 0)
		{		/* Type-1 relocation */
		  int reltype = get_type1_type (relword);
		  if ((reltype & (REL_SYM | REL_PC)) != 0)
		    add_symref (ap, get_type1_index (relword));
		}
	      else
		{		/* Type-2 relocation */
		  int reltype = get_type2_type (relword);
		  if ((reltype & REL_SYM) != 0)
		    {		/* Symbol */
		      add_symref (ap, get_type2_index (relword));
		    }
		  else
		    {		/* Area */
		      add_arearef (ap, get_area (get_type2_index (relword)));
		    }
		}
	    }
	}

      find_arearefs (ap->right);
    }
}

/*
** 'mark_area' is called to mark an area as non-deletable by bumping up
** its reference count
*/
void
mark_area (arealist * ap)
{
  ap->arefcount++;
}

/*
** 'find_unused' is called to find out which areas in the linked
** program are used and which are not and so can be left out of
** the image file
*/
void
find_unused (void)
{
  if (opt_verbose)
    error ("Drlink: Finding unused areas...");
  mark_area (entryarea);	/* Don't want entry area to disappear */
  if (imagetype == RMOD)
    mark_area (rocodelist);	/* or any embarrasing problems with modules */
  if (opt_cpp)
    find_cdareas ();		/* or with C++ constructors and destructors */
  if (opt_linkersets)
    find_lsareas ();            /* or with any linker-sets in */
  find_arearefs (rocodelist);
  find_arearefs (rodatalist);
  find_arearefs (rwcodelist);
  find_arearefs (rwdatalist);
  find_arearefs (zidatalist);
  mark_unused (rocodelist);
  mark_unused (rodatalist);
  mark_unused (rwcodelist);
  mark_unused (rwdatalist);
  mark_unused (zidatalist);
}

/*
** 'fillin_limits' fills in the area pointers and values of the base
** and limit pair for the given area
*/
static void
fillin_limits (arealist * firstarea, arealist * lastarea)
{
  arealimits *p;
  const char *name = firstarea->arname;

  for (p = arlimlist; p != NULL; )
    {
      int compval = strcmp (name, p->areaname);

      if (compval == 0)
	break;
      p = (compval > 0) ? p->right : p->left;
    }

  if (p == NULL)
    error ("Fatal: Possible linker error in 'fillin_limits'");
  p->areabase->symtarea.areaptr = firstarea;
  p->areabase->symtvalue = firstarea->arplace;
  p->arealimit->symtarea.areaptr = lastarea;
  p->arealimit->symtvalue = lastarea->arplace + lastarea->arobjsize;
}

/*
** 'align_page' aligns the end of the last area in the R/O portion of the
** image on a page boundary. This is for principly for debugging: DDT
** write-protects R/O areas up to a page boundary. At the moment this
** is kludged by simply extending the last area in the R/O part to a
** page boundary: Anything that follows the area will be written to the
** imagefile as part of the image. There is a chance this could lead to
** an addressing exception if this takes the area past the end of the
** wimp slot.
*/
static void
align_page (void)
{
  arealist *rolist, *lastarea;
  int pagesize;
  unsigned int padding;
#ifndef CROSS_COMPILE
  _kernel_oserror *swierror;
  _kernel_swi_regs regs;
#endif
  rolist = (rodatalist != NULL) ? rodatalist : rocodelist;
  if (rolist == NULL)
    return;			/* No read-only areas to protect */
  lastarea = NULL;
  do
    {
      lastarea = rolist;
      rolist = rolist->right;
    }
  while (rolist != NULL);
#ifndef CROSS_COMPILE
  swierror = _kernel_swi (OS_ReadMemMapInfo, &regs, &regs);
  if (swierror == NULL)
    pagesize = regs.r[0];	/* Page size is returned by SWI in R0 */
  else
    {
      error ("Warning: Could not determine machine's page size. Default of 32K used");
      pagesize = DEFAULTSIZE;
    }
#else
  pagesize = DEFAULTSIZE;
#endif
  padding = ((areapc + pagesize - 1) & -pagesize) - areapc;
  lastarea->arobjsize += padding;
  areapc += padding;
}

/*
** 'calc_place' goes through a list of areas and fills in the address in
** the image file where the area is to be found. If the area has an entry
** in the symbol table, that is filled in with the value as well. The
** base/limit pairs for each area are also sorted out here. To make
** it even more complicated, the routine has to allow for areas that
** will not be included in the final image file. The last function it
** carries out is to add the size of the areas to the size of the image
** file.
*/
static void
calc_place_tree (arealist * p, arealist ** firstarea, arealist ** lastarea,
		 arealist ** lastbase)
{
  if (p != NULL)
    {
      calc_place_tree (p->left, firstarea, lastarea, lastbase);

      if (p->arefcount > 0)
	{
	  unsigned int align;

	  if (*firstarea == NULL)
	    {			/* First ref to this area name */
	      *firstarea = *lastarea = p;
	      *lastbase = p->arbase;
	    }
	  else if (*lastbase == p->arbase)
	    {			/* Same name as last entry */
	      *lastarea = p;
	    }
	  else
	    {			/* New area name */
	      fillin_limits (*firstarea, *lastarea);
	      *firstarea = *lastarea = p;
	      *lastbase = p->arbase;
	    }

	  align = (1 << p->aralign) - 1;	/* Align area address as necessary */
	  areapc = (areapc + align) & ~align;
	  p->arplace = areapc;
	  if (p->arsymbol != NULL)
	    define_symbol (p->arsymbol, areapc);
	  areapc += p->arobjsize;
	}

      calc_place_tree (p->right, firstarea, lastarea, lastbase);
    }
}


static void
calc_place (arealist * p)
{
  arealist * firstarea,		/* Points to first area with name 'x' */
    *lastarea,			/* Points to last area with name 'x' */
    *lastbase;
  firstarea = lastarea = lastbase = NULL;

  calc_place_tree (p, &firstarea, &lastarea, &lastbase);

  if (firstarea != NULL)
    fillin_limits (firstarea, lastarea);
}

/*
** 'relocate_areas' is called to work out the relocation constant
** of each area. It also defines the values of the pre-defined
** linker symbols and the base/limit pair values for each area.
*/
void
relocate_areas (void)
{
  unsigned int initpc, oldpc, baseaddr;
  segtype wantedtype;

  if (opt_codebase)
    progbase = baseaddr = codebase;	/* Non-standard base-of-code used */
  else
    progbase = baseaddr = ABSBASE;
  headersize = 0;
  if (!noheader)
    {				/* Executable image has a header on it */
      if (imagetype == BIN)
	wantedtype = HDR_BIN;
      else
	wantedtype = HDR_AIF;	/* AIF or relocatable AIF */
      get_hdrcode (wantedtype, &headercode, &headersize);
    }
  areatop = initpc = areapc = baseaddr + headersize;
  imagesize = headersize;
  define_symbol (image_robase, areapc);
  define_symbol (image_codebase, areapc);
  calc_place (rocodelist);
  calc_place (rodatalist);
  if (opt_pagealign)
    align_page ();
  define_symbol (image_rolimit, areapc);
  define_symbol (image_codelimit, areapc);
  if (opt_database)
    {				/* Non-standard base-of-data used */
      if (database >= initpc && database < areapc)
	error ("Warning: Read/write code && data areas overlap read only areas");

      if (rwcodelist != NULL || rwdatalist != NULL)
	error ("Warning: Image file contains code or constant data in read/write area and option '-data' has been used");
      areapc = database;
    }
  define_symbol (image_rwbase, areapc);
  define_symbol (image_database, areapc);
  calc_place (rwcodelist);
  oldpc = areapc;
  calc_place (debuglist);
  areapc = oldpc;
  calc_place (rwdatalist);
  imagesize = imagesize + areapc - initpc;
  define_symbol (image_zibase, areapc);
  if (imagetype == RMOD)
    define_symbol (reloc_code, areapc);
  relocaddr = areapc;
  calc_place (zidatalist);
  define_symbol (image_zilimit, areapc);
  define_symbol (image_rwlimit, areapc);
  define_symbol (image_datalimit, areapc);
}

/*
** The next few functions deal with relocations. This section is complex
** and extreme care should be taken when modifying it
*/

/*
** 'flag_badreloc' is called to print an error message about a bad
** relocation
*/
static void
flag_badreloc (const unsigned int *relplace, const char *type, symtentry * sp)
{
  unsigned int offset = (const char *)relplace - (const char *)areastart;

  error ("Error: Bad relocation in in file '%s'", current_file->chfilename);
  if (sp)
    error ("Error: Relocated (%s) value is out of range at offset 0x%x in area '%s' for symbol '%s'",
           type, offset, current_area->arname, sp->symtname);
  else
    error ("Error: Relocated (%s) value is out of range at offset 0x%x in area '%s'",
           type, offset, current_area->arname);
}

/*
** 'fixup_adr' is called to relocate 'adr' instructions. Either
** single instructions or sequences of ADD and SUB can be dealt
** with by this code.
*/
static void
fixup_adr (unsigned int *relplace, int inscount, unsigned int relvalue,
	   symtentry * sp)
{
  unsigned int regrnrd, inst;
  int addr, shift;
  unsigned int *relstart, *relbase;
  bool negative;
  if (inscount == 0)
    inscount = 0x10000;		/* inscount=0 = do as many instructions as needed */
  addr = 0;
  relbase = relstart = relplace;
  inst = *relplace;
  regrnrd = inst & DEST_MASK;
  regrnrd = regrnrd + (regrnrd << 4);	/* Get mask for register Rn and Rd in ADDs and SUBs */
  do
    {				/* Extract the encoded offset */
      if ((inst & IN_ADDSUB) == INST_ADD)
	addr += decode_armconst (inst);
      else
	addr -= decode_armconst (inst);
      relplace++;
      inst = *relplace;
      inscount -= 1;
    }
  while (inscount != 0
	 && ((inst & IN_ADDSUB) == INST_SUB || (inst & IN_ADDSUB) == INST_ADD)
	 && (inst & RNRD_MASK) == regrnrd);
  addr += relvalue;
  negative = addr < 0;
  if (negative)
    addr = -addr;
  shift = 16;
  do
    {				/* Encode constant for ARM instruction */
      if (addr != 0)
	{
	  while ((addr & LOW2_MASK) == 0)
	    {
	      addr = addr >> 2;
	      shift -= 1;
	    }
	}
      inst = (*relstart & ADSB_MASK) | (negative ? INST_SUB : INST_ADD);
      *relstart =
	(inst | ((shift & SHIFT_MASK) << SHIFT_SHIFT)) + (addr & BYTE_MASK);
      addr = addr & ~BYTE_MASK;
      relstart++;
    }
  while (relstart != relplace);
  if (addr != 0)
    flag_badreloc (relbase, "ADR", sp);
}

/*
** 'fixup_additive' handles additive relocations, including
** instruction sequences
*/
static void
fixup_additive (unsigned int *relplace, unsigned int reltype,
		unsigned int relvalue, symtentry * sp)
{
  unsigned int data, inst;
  int addr;
  switch (reltype & FTMASK)
    {
    case REL_BYTE:
    case REL_HALF:		/* Relocate byte and halfword */
      if ((reltype & REL_SYM) == 0)
	{
	  error ("Error: Found bad relocation type for a byte or halfword field in area '%s' in '%s'",
	         current_area->arname, current_file->chfilename);
	  return;
	}
      if ((reltype & FTMASK) == 0)
	{			/* Byte relocation */
	  data = *(COERCE (relplace, char *)) + relvalue;
	  if (data > BYTEHIGH)
	    flag_badreloc (relplace, "Byte", sp);
	  else
	    *(COERCE (relplace, char *)) = data;
	}
      else
	{			/* Half word  relocation */
	  data =
	    *(COERCE (relplace, char *)) +
	    (*(COERCE (relplace, char *) + 1) << 8) + relvalue;
	  if (data > HALFHIGH)
	    flag_badreloc (relplace, "Halfword", sp);
	  else
	    {
	      *(COERCE (relplace, char *)) = data & 0xFF;
	      *(COERCE (relplace, char *) + 1) = data >> 8;
	    }
	}
      break;

    case REL_WORD:
      *relplace += relvalue;
      totalrelocs += 1;
      break;

    case REL_SEQ:		/* Instruction sequence */
      inst = *relplace;
      if ((inst & INSTMASK) == IN_LDRSTR)
	{			/* LDR/STR */
	  addr = inst & IN_OFFMASK;
	  if ((inst & IN_POSOFF) == 0)
	    addr = -addr;
	  addr = addr + relvalue;
	  if (addr <= -MAX_OFFSET || addr >= MAX_OFFSET)
	    flag_badreloc (relplace, "LDR/STR", sp);
	  else
	    *relplace =
		(inst & LDST_MASK) | (addr < 0 ? -addr : addr | IN_POSOFF);
	}
      else if ((inst & INSTMASK) == IN_DATAPRO)
	{			/* Data processing instruction */
	  fixup_adr (relplace, (reltype & REL_IIMASK) >> REL_IISHIFT,
		     relvalue, sp);
	}
      else
	{			/* Branch instruction. Will this happen? */
	  *relplace =
	    (inst & BR_MASK) | ((((extend24 (inst) << 2) + relvalue) >> 2) &
				BROFF_MASK);
	}
    }
}

/*
** 'fixup_pcrelative' is called to handle PC-relative relocations for both
** type-1 and type-2 relocations. Note that the code has to be able to
** cope with relocations in different instruction types and not just branch
** instructions.
*/
static bool
fixup_pcrelative (unsigned int *relplace, reloctype reltype, int relvalue,
		  symtentry * sp)
{
  int addr;
  unsigned int inst;
  inst = *relplace;
  if ((inst & INSTMASK) == IN_BRANCH)
    {				/* Branch instruction */
      addr = extend24 (inst) << 2;	/* Get address in inst, sign extend & convert to bytes */
      if (reltype == TYPE_1)
	{
	  addr += relvalue - current_area->arplace -
	    (COERCE (relplace, char *) - COERCE (areastart, char *)) -
	    PCMODIFIER;
	}
      else
	addr += relvalue - current_area->arplace;
      *relplace = (inst & BR_MASK) | ((addr >> 2) & BROFF_MASK);
    }
  else if ((inst & INSTMASK) == IN_LDRSTR)
    {				/* LDR/STR */
      addr = inst & IN_OFFMASK;
      if ((inst & IN_POSOFF) == 0)
	addr = -addr;
      addr = addr + relvalue - current_area->arplace;
      if (addr <= -MAX_OFFSET || addr >= MAX_OFFSET)
	flag_badreloc (relplace, "PC relative", sp);
      else
	{
	  *relplace =
	    (inst & LDST_MASK) | (addr < 0 ? -addr : addr | IN_POSOFF);
	}
    }
  else
    {				/* Assume data processing instruction (ADD or SUB) */
      fixup_adr (relplace, 0, relvalue - current_area->arplace, sp);
    }
  return TRUE;
}

/*
** 'fixup_type1' is called to handle a type-1 relocation
*/
static void
fixup_type1 (unsigned int reltypesym, unsigned int *relplace)
{
  unsigned int reltype, relvalue, symtindex;
  symtentry *sp = NULL;
  reltype = get_type1_type (reltypesym);
  if ((reltype & (REL_SYM | REL_PC)) != 0)
    {				/* Symbol relocation */
      if ((symtindex = (get_type1_index (reltypesym))) > symbolcount)
	{
	  error
	    ("Error: Found bad OBJ_SYMT index in relocation in area '%s' in '%s'",
	     current_area->arname, current_file->chfilename);
	  return;
	}
      sp = symtbase + symtindex;
      if ((sp->symtattr & SYM_MASK_DEFN) != 0)
	{			/* Nice, easy ref to symbol */
	  if ((sp->symtattr & SYM_STRONG) != 0)
	    {			/* Nasty, horrible ref to strong symbol in this file */
	      sp = find_nonstrong (sp);
	    }
	  relvalue = sp->symtvalue;
	}
      else if ((sp->symtattr & (SYM_WEAKREF | SYM_ABSVAL)) != 0)	/* Weak external ref or absolute */
	relvalue = 0;
      else
	relvalue = sp->symtarea.symdefptr->symtvalue;	/* Ref to defined external */
    }
  else
    {	/* Internal, that is, relative to this area, relocation */
      relvalue = current_area->arplace;
    }
  if ((reltype & REL_PC) != 0)
    {	/* PC-relative relocation */
      fixup_pcrelative (relplace, TYPE_1, relvalue, sp);
    }
  else
    {	/* Additive relocation */
      fixup_additive (relplace, reltype, relvalue, sp);
    }
}

/*
** 'fixup_type2' handles the relocation of a single type-2 relocation
*/
static void
fixup_type2 (unsigned int reltypesym, unsigned int *relplace)
{
  unsigned int reltype, relvalue, symtindex;
  symtentry *sp = NULL;
  arealist *ap;

  reltype = get_type2_type (reltypesym);
  if ((reltype & REL_SYM) != 0)
    {	/* Symbol relocation */
      if ((symtindex = (get_type2_index (reltypesym))) > symbolcount)
	{
	  error ("Error: Found bad OBJ_SYMT index in relocation in area '%s' in '%s'",
	         current_area->arname, current_file->chfilename);
	  return;
	}
      sp = symtbase + symtindex;
      if ((sp->symtattr & SYM_MASK_DEFN) != 0)
	{			/* Nice, easy ref to symbol */
	  if ((sp->symtattr & SYM_STRONG) != 0)
	    {			/* Direct reference to strong symbol */
	      sp = find_nonstrong (sp);
	    }
	  relvalue = sp->symtvalue;
	}
      else if ((sp->symtattr & (SYM_WEAKREF | SYM_ABSVAL)) != 0)
	relvalue = 0;	/* Weak external ref or absolute */
      else
	relvalue = sp->symtarea.symdefptr->symtvalue;	/* Ref to defined external */
    }
  else
    {	/* Internal, that is, relative to some area, relocation */
      reltypesym = get_type2_index (reltypesym);
      if (reltypesym >= current_file->areacount)
	{
	  error ("Error: Type-2 relocation area index too high in area '%s' in '%s'",
	         current_area->arname, current_file->chfilename);
	  return;
	}
      if ((reltype & REL_BASED) == 0)
	{			/* Ordinary area relocation */
	  ap = get_area (reltypesym);
	  relvalue = ap->arplace;
	}
      else
	{			/* Based area relocation */
	  ap = get_area (reltypesym);
	  relvalue = ap->arplace - ap->arbase->arplace;	/* Addr of area - addr of 1st area of this name */
	}
    }

  if ((reltype & REL_PC) != 0)
    {				/* PC-relative relocation */
      /* From Linker document :
       *
       * If R is 1, B is usually 0.  A B value of 1 is used to denote that the
       * inter-link-unit value of a branch destination is to be used, rather than
       * the more usual intra-link-unit value.
       */
      if ((reltype & REL_BASED) != 0)
	{
	  /* PC-relative and based - relocate using the inter-link-unit,
	     not intra-link unit */

	  /* We need to check that the destination is a leaf or not.
	   * If it is a leaf, then we branch to its start; otherwise we branch to
	   * start + 4 :
	   *  Bit 11 is the Simple Leaf Function attribute which is meaningful
	   *  only if this symbol defines the entry point of a sufficiently
	   *  simple leaf function (a leaf function is one which calls no other
	   *  function).  For a reentrant leaf function it denotes that the
	   *  function's inter-link-unit entry point is the same as its
	   *  intra-link-unit entry point.
	   */
	  if ((reltype & REL_SYM) == 0)
	    {
	      /* I don't know what to do if it's not symbol based; I can't look at
	       * the symbol to check how to re-link it; this is (AFAICT) undefined.
	       */
	      error ("Error: PC-relative, reentrant linkage of non-symbol data in area '%s' in '%s'",
		     current_area->arname, current_file->chfilename);
	      return;
	    }
	  if ((sp->symtattr & SYM_LEAF) == 0)
	    relvalue += 4;	/* Use inter-link entry point */
	}
      fixup_pcrelative (relplace, TYPE_2, relvalue, sp);
    }
  else
    {				/* Additive relocation */
      fixup_additive (relplace, reltype, relvalue, sp);
    }
}

/*
** 'relocate_item' handles the relocation of a single item in the
** OBJ_AREA chunk.
*/
static void
relocate_item (relocation * rp)
{
  unsigned int offset, relts;
  unsigned int *relplace;

  offset = rp->reloffset;
  if (offset > current_objsize)
    {
      error ("Error: Bad relocation offset (0x%x) found in area '%s' in '%s'",
	     offset, current_area->arname, current_file->chfilename);
      return;
    }
  relplace = COERCE (COERCE (areastart, char *) + offset, unsigned int *);
  relts = rp->reltypesym;
  if ((relts & REL_TYPE2) == 0)
    fixup_type1 (relts, relplace);
  else
    fixup_type2 (relts, relplace);
}

/*
** 'relocate_arealist' is called to deal with all of the relocations
** in all of the areas listed in the area list passed to it
*/
static void
relocate_arealist (arealist * ap)
{
  if (ap != NULL)
    {
      int numrelocs;

      relocate_arealist (ap->left);

      if (ap->arefcount > 0 && (numrelocs = ap->arnumrelocs) != 0)
	{
	  int n;
	  relocation *rp;
	  current_area = ap;
	  current_file = ap->arfileptr;
	  current_objsize = ap->arobjsize;
	  symtbase = ap->arfileptr->obj.symtptr;
	  symbolcount = ap->arfileptr->symtcount;
	  areastart = ap->arobjdata;
	  for (n = 1, rp = ap->areldata; n <= numrelocs; ++n, ++rp)
	    relocate_item (rp);
	}

      relocate_arealist (ap->right);
    }
}

/*
** 'relocate' is called to handle the relocation of the code and
** data in all the area lists
*/
bool
relocate (void)
{
  relocate_arealist (rocodelist);
  relocate_arealist (rodatalist);
  relocate_arealist (rwcodelist);
  relocate_arealist (rwdatalist);
  relocate_arealist (debuglist);
  return !got_errors ();
}

/*
** 'check_entryarea' is called to check the program's entry point and to
** set up a default one if none has been specified
*/
void
check_entryarea (void)
{
  if (entryarea == NULL)
    {				/* No entry point specified */
      entryarea = defaultarea;
      entryoffset = 0;
      error ("Warning: Program has no entry point. Default of first executable instruction assumed");
    }

  /* Default to suggesting that the program requires a standard
   * AIF or Binary header.
   */
  noheader = 0;

  /* Relocatable modules do not have a standard header.  */
  if (imagetype == RMOD)
    noheader = 1;

  /* Images where the user has supplied a base address do not have
   * a standard header.
   */
  if (opt_codebase)
    noheader = 1;

  /* Both AIF and Binary (BIN) images can have headers.
   *
   * In the case of binary images, if the user explicitly set an ENTRY
   * attribute on either a read-only or a read-write CODE AREA, then we
   * drop the standard binary header and therefore assume that the
   * user knows better.
   */
  if (imagetype == BIN
      && (entryarea == rocodelist
	  || (rodatalist == NULL && entryarea == rwcodelist))
      && entryoffset == 0)
    noheader = 1;
}

/*
** 'fillin_header' is called to fill in the executable image's header
*/
static void
fillin_header (void)
{
  unsigned int startaddr;
  startaddr = entryarea->arplace + entryoffset;
  if (imagetype == BIN)
    setup_binhdr (headercode, startaddr);
  else
    setup_aifhdr (headercode, startaddr);
}

/*
** 'write_reloc' is called when dealing with relocatable modules and
** relocatable AIF images to create the relocation table appended to
** image files of these types.
**
** It goes through the relocation information for one area in one of
** the area lists and picks out items that will change if the program
** moves. These are words that address areas or symbols that are not
** absolute values. In terms of AOF relocation types, these are
** word-sized additive relocations that are either internal relocations
** or symbol relocations where the symbol is not absolute. The function
** builds up a table of the offsets of any relocations that match these
** criteria within the image file and writes this table to the image file.
** Note that the table is built using the same memory as that used by
** the relocation data. This is okay, as the table will always be
** smaller than the relocation data (relocations are eight bytes
** long and the table entries are four bytes).
*/
#define WORDADDITIVE   0x00020000	/* Type-1 Word additive */
#define RELMASK        0x00060000
#define T2WORDADDITIVE 0x82000000	/* Type-2 word additive */
#define T2RELMASK      0x86000000

static void
write_reloc (arealist * ap)
{
  unsigned int n, numrelocs, areaoffset, newsize;
  relocation *rp;
  unsigned int *rip;
  numrelocs = ap->arnumrelocs;
  rp = ap->areldata;
  rip = COERCE (rp, unsigned int *);

  areaoffset = ap->arplace - progbase;
  symtbase = ap->arfileptr->obj.symtptr;
  newsize = 0;
  for (n = 1; n <= numrelocs; ++n, ++rp)
    {
      if (((rp->reltypesym & RELMASK) == WORDADDITIVE
	   || (rp->reltypesym & T2RELMASK) == T2WORDADDITIVE)
	  && isrelocatable (rp))
	{
	  *rip++ = areaoffset + rp->reloffset;
	  newsize += sizeof (unsigned int);
	}
    }
  if (newsize != 0)
    {				/* Got some relocations */
      convert_endian (ap->areldata, newsize / 4);
      write_image (ap->areldata, newsize);
    }
}

/*
** 'write_areareloc' builds and writes the relocation information for
** each area in the area list passed to it
*/
static void
write_areareloc (arealist * ap)
{
  if (ap != NULL)
    {
      write_areareloc (ap->left);
      if (ap->arnumrelocs != 0)
	write_reloc (ap);
      write_areareloc (ap->right);
    }
}

/*
** 'write_relocinfo' is called to add to the image file the relocation
** code and tables for relocatable programs and modules
*/
static void
write_relocinfo (void)
{
  unsigned int *relocode;
  unsigned int relocsize;

  get_hdrcode ((imagetype == RELOC) ? HDR_RELOC : HDR_RMOD, &relocode,
	       &relocsize);
  if (imagetype == RMOD)
    setup_modcode (relocode);
  convert_endian (relocode, relocsize / 4);
  write_image (relocode, relocsize);
  write_areareloc (rocodelist);
  write_areareloc (rodatalist);
  write_areareloc (rwcodelist);
  write_areareloc (rwdatalist);
  write_lsreloc ();
  if (imagetype == RMOD)
    {				/* Write last two offsets required */
      struct
      {
	unsigned int reloc_offset, robase_offset;
      } lastbit;
      unsigned int relocend =
	reloc_code->symtptr->symtvalue + relocsize -
	2 * sizeof (unsigned int) - progbase;
      lastbit.reloc_offset = relocend;
      lastbit.robase_offset = relocend + sizeof (unsigned int);
      convert_endian (&lastbit, sizeof (lastbit) / 4);
      write_image (&lastbit, sizeof (lastbit));
    }
  relocsize = 0xFFFFFFFF;	/* End marker */
  // convert_endian() : not needed
  write_image (&relocsize, sizeof (unsigned int));
}

/*
** The next group of functions are concerned with relocations
** when creating a partially-linked AOF file. This lot really
** are nasty!
*/

/*
** 'alter_area_offset' is called to change the offset within an instruction
** (or word of data) used in a type-2 relocation. Note that this code has been
** modified to handle the case where a type-2 relocation that refers to code
** has the 'relocate word' attribute. With AOF 3, it should have a 'code'
** attribute but it looks as if earlier versions of AOF did not make the
** distinction. What is used to decide whether we are dealing with code or
** data is the type of relocation. If it is PC-relative then it is assumed
** to refer to code.
*/
static void
alter_area_offset (relocation * rp)
{
  unsigned int inst, reltype, offset;
  int addr;
  unsigned int *relplace;
  offset = rp->reloffset;
  if (offset > current_area->arobjsize)
    {
      error ("Error: Bad relocation offset (0x%x) found in area '%s' in '%s'",
	     offset, current_area->arname, current_file->chfilename);
      return;
    }
  reltype = get_type2_type (rp->reltypesym);	/* Assume type-2 relocation */
  relplace = COERCE (COERCE (areastart, char *) +offset, unsigned int *);
  inst = *relplace;
  if ((reltype & FTMASK) == REL_SEQ
      || (reltype & (FTMASK | REL_PC)) == (REL_WORD | REL_PC))
    {				/* Relocate instructions */
      if ((inst & INSTMASK) == IN_LDRSTR)
	{			/* LDR/STR */
	  addr = inst & IN_OFFMASK;
	  if ((inst & IN_POSOFF) == 0)
	    addr = -addr;
	  addr = addr - current_area->arplace;	/* Subtract offset within 'super area' of area from inst offset */
	  if (addr <= -MAX_OFFSET || addr >= MAX_OFFSET)
	    flag_badreloc (relplace, "Area Offset", NULL);
	  else
	    {
	      *relplace =
		(inst & LDST_MASK) | (addr < 0 ? -addr : addr | IN_POSOFF);
	    }
	}
      else if ((inst & INSTMASK) == IN_DATAPRO)
	{			/* Data processing instruction */
	  fixup_adr (relplace, (reltype & REL_IIMASK) >> REL_IISHIFT,
		     -current_area->arplace, NULL);
	}
      else
	{
	  *relplace =
	    (inst & BR_MASK) |
	    ((((extend24 (inst) << 2) -
	       current_area->arplace) >> 2) & BROFF_MASK);
	}
    }
  else
    *relplace = inst - current_area->arplace;	/* Data */
}

/*
** 'alter_type1_offset' is called to modify the area offset within a
** branch of branch with link instruction referenced by a type-1
** relocation, the only type of instruction to which PC-relative
** relocations can be applied using type-1. The instruction is
** modified to the value it would take if branching to the address
** *as if the address was defined in the area containing the
** reference*.
*/
static bool
alter_type1_offset (relocation * rp)
{
  unsigned int offset;
  unsigned int *relplace;
  symtentry *sp;
  int addr;
  offset = rp->reloffset;
  if (offset > current_area->arobjsize)
    {
      error ("Error: Bad relocation offset (0x%x) found in area '%s' in '%s'",
	     offset, current_area->arname, current_file->chfilename);
      return FALSE;
    }
  relplace = COERCE (COERCE (areastart, char *) +offset, unsigned int *);
  sp = symtbase + get_type1_index (rp->reltypesym);
  if ((sp->symtattr & SYM_MASK_DEFN) == 0)
    sp = sp->symtarea.symdefptr;	/* Reference to another SYMT entry */
  else if ((sp->symtattr & SYM_MASK_STRONGDEF) == SYM_MASK_STRONGDEF)
    sp = find_nonstrong (sp);	/* Direct reference to strong symbol */
  addr = sp->symtvalue + (extend24 (*relplace) << 2);
  addr = addr - offset - current_area->arplace - PCMODIFIER;
  *relplace = (*relplace & BR_MASK) | ((addr >> 2) & BROFF_MASK);
  return TRUE;
}

/*
** 'alter_type2_offset' is called to modify the address part of instructions
** referenced by type-2 PC-relative relocations. Note that any type of
** instruction can be the target of such a relocation and that there are
** two classes of relocation, area and symbol, to be dealt with as well.
**
** In the case of a type-2 relocation, the address part of the instruction
** is modified to, as it were, move it to the start of the area. It is
** possible that in the link process, an area has been added to the area
** list before the area containing the relocation where the names of the
** areas are the same. In the partially-linked file, the areas will have
** been coalesced into one and the new area will precede the one containing
** the relocation. As a result, the offset in the instruction will be wrong
** and it is necessary to compensate for the new area in the address part
** of the instruction.
*/
static bool
alter_type2_offset (relocation * rp)
{
  unsigned int inst, offset, typesym;
  unsigned int *relplace;
  symtentry *sp = NULL;
  int addr, relvalue;
  offset = rp->reloffset;
  if (offset > current_area->arobjsize)
    {
      error ("Error: Bad relocation offset (0x%x) found in area '%s' in '%s'",
	     offset, current_area->arname, current_file->chfilename);
      return FALSE;
    }
  typesym = rp->reltypesym;
  relplace = COERCE (COERCE (areastart, char *) +offset, unsigned int *);
  inst = *relplace;
  addr = get_type2_index (typesym);	/* Extract symbol or area number */
  typesym = get_type2_type (typesym);
  if ((typesym & REL_SYM) != 0)
    {				/* Symbol reference */
      sp = symtbase + addr;
      if ((sp->symtattr & SYM_MASK_DEFN) == 0)
	sp = sp->symtarea.symdefptr;	/* Reference to another SYMT entry */
      else if ((sp->symtattr & SYM_MASK_STRONGDEF) == SYM_MASK_STRONGDEF)
	sp = find_nonstrong (sp);	/* Direct reference to strong symbol */
      relvalue = sp->symtvalue;
    }
  else
    relvalue = get_area (addr)->arplace;	/* Area reference */

  if ((inst & INSTMASK) == IN_LDRSTR)
    {				/* LDR/STR */
      addr = inst & IN_OFFMASK;
      if ((inst & IN_POSOFF) == 0)
	addr = -addr;
      addr = addr + relvalue - current_area->arplace;
      if (addr <= -MAX_OFFSET || addr >= MAX_OFFSET)
	flag_badreloc (relplace, "Type 2 PC offset", sp);
      else
	*relplace = (inst & LDST_MASK) | (addr < 0 ? -addr : addr | IN_POSOFF);
    }
  else if ((inst & INSTMASK) == IN_DATAPRO)
    {				/* Data processing instruction */
      fixup_adr (relplace, (typesym & REL_IIMASK) >> REL_IISHIFT,
		 relvalue - current_area->arplace, sp);
    }
  else
    {
      *relplace =
	(inst & BR_MASK) |
	((((extend24 (inst) << 2) + relvalue -
	   current_area->arplace) >> 2) & BROFF_MASK);
    }
  return TRUE;
}

/*
** 'decode_reloc' determines whether the relocation passed
** to it can be carried out at this stage or not. It also
** works out if a further relocation will be needed and if
** so, the type of relocation.
** If the relocation is additive and refers to a symbol whose
** value is known, it can be carried out but a further type-2
** internal relocation must be generated as well. PC-relative
** relocations within the same 'super area' can be dealt with
** with no further action required later. PC-relative
** relocation between different areas can be handled but an
** extra type-2 relocation has to be created.
**
** There is a slight kludge here needed to handle AOF files
** generated by the C compiler, where it attempts to relocate
** a symbol in a non-existent area. If there were no references
** to the symbol we would not have to bother, but there are...
*/
static relaction
decode_reloc (relocation * rp)
{
  unsigned int typesym, sid;
  symtentry *sp;
  bool istype1;
  typesym = rp->reltypesym;
  istype1 = (typesym & REL_TYPE2) == 0;
  if (istype1)
    {				/* Type 1 relocation */
      sid = get_type1_index (typesym);
      typesym = get_type1_type (typesym);
    }
  else
    {				/* Type 2 relocation */
      sid = get_type2_index (typesym);
      typesym = get_type2_type (typesym);
    }
  if ((typesym & REL_SYM) != 0 || (istype1 && (typesym & REL_PC) != 0))
    {				/* Symbol relocation */
      sp = symtbase + sid;
      if ((sp->symtattr & SYM_MASK_DEFN) == 0)
	{			/* External reference */
	  sp = sp->symtarea.symdefptr;
	  if (sp == NULL || (sp->symtattr & SYM_COMMON) != 0)
	    {			/* Not defined or ref to common block */
	      if (istype1 || (typesym & REL_PC) == 0)
		{		/* Type 1 or additive */
		  return NOTPOSS;	/* Can't be done at this stage */
		}
	      else
		{		/* Update instruction, PC-rel */
		  return TYPE2_RP;
		}
	    }
	}
      else if ((sp->symtattr & SYM_MASK_STRONGDEF) == SYM_MASK_STRONGDEF)
	{			/* Direct reference to strong symbol */
	  sp = find_nonstrong (sp);
	}
      if ((sp->symtattr & SYM_ABSVAL) != 0)
	{			/* Absolute value? */
	  return IGNORE;
	}
/*
** Here, the symbol referenced is known. There are four cases to consider:
** a)  PC-relative relocation, ref and symbol in same area
** b)  PC-relative relocation, ref and symbol in different areas
** c)  Additive relocation, ref and symbol in same area
** b)  Additive relocation, ref and symbol in different areas
**
** a) is the easiest and the relocation can be sorted out completely.
** b) The relocation can be partly resolved but a type-2 area, PC-rel
**    relocation is needed as well.
** c) The offset of the symbol within its area can be filled in, but a
**    type-2 internal relocation will need to be created
** d) The offset of the symbol within its area can be filled in, but a
**    type-2 internal relocation will need to be created
*/
      if ((typesym & REL_PC) != 0)
	{			/* PC-relative */
	  if (sp->symtarea.areaptr->arbase == current_area->arbase)
	    {			/* Same area, no problems */
	      return POSSIBLE;	/* Can be done now */
	    }
	  else
	    {			/* Different areas */
	      return TYPE2_AP;	/* Need type-2 area PC-relative relocation */
	    }
	}
      else
	{			/* Additive - Can do, but need a type-2 area additive reloc as well */
	  return TYPE2_AA;	/* Need type-2 area additive relocation */
	}
    }
  else
    {				/* Internal relocation but excluding type-1 PC-rel */
/*
** Internal relocations, that is, ones that involve the addresses of
** areas rather than symbols, are easier to handle. There are four
** cases to consider:
** a) PC-relative, ref area and target area the same
** b) PC-relative, ref area and target area different
** c) Additive, ref area and target area the same
** d) Additive, ref area and target area are different
*/
      if ((typesym & REL_PC) != 0)
	{			/* PC-relative */
	  return TYPE2_AP;	/* Type-2 area PC-relative */
	}
      else
	{
	  return TYPE2_AA;	/* Type-2 area additive */
	}
    }
}

/*
** 'alter_reloc' is called to turn the relocation passed to it into a
** suitable type-2 relocation.
*/
static void
alter_reloc (relocation * rp, relaction t2type)
{
  unsigned int flags, sid;
  bool istype1;
  symtentry *sp;
  arealist *ap;
  flags = rp->reltypesym;
  istype1 = (flags & REL_TYPE2) == 0;
  if (istype1)
    {
      sid = get_type1_index (flags);
      flags = get_type1_type (flags);
    }
  else
    {
      sid = get_type2_index (flags);
      flags = get_type2_type (flags);
    }
  if ((flags & REL_SYM) != 0 || (istype1 && (flags & REL_PC) != 0))
    {				/* Got a symbol reference */
      sp = symtbase + sid;
      if ((sp->symtattr & SYM_MASK_DEFN) == 0)
	sp = sp->symtarea.symdefptr;
      else if ((sp->symtattr & SYM_MASK_STRONGDEF) == SYM_MASK_STRONGDEF)
	sp = find_nonstrong (sp);	/* Direct reference to strong symbol */
      ap = sp->symtarea.areaptr;
    }
  else if (istype1)
    ap = current_area;		/* Got a type-1 area reference */
  else
    ap = get_area (sid);	/* Got a type-2 area reference */

  sid = find_areaindex (ap);
  switch (t2type)
    {
    case TYPE2_SA:		/* Symbol, additive */
      flags = flags & REL_AOFMASK;
      break;
    case TYPE2_SP:		/* Symbol, PC-rel  */
      flags = flags & (REL_AOFMASK | REL_SYM);
      break;
    case TYPE2_AA:		/* Area, additive */
      flags = flags & REL_AOFMASK;
      break;
    case TYPE2_AP:		/* Area, PC-rel */
      flags = flags & (REL_AOFMASK | REL_PC);
    default:
      break;
    }
  rp->reltypesym = flags << 24 | REL_TYPE2 | sid;
}

/*
** 'fixup_reloclist' goes through the list of external reference
** relocations and resolves any that can be handled at this stage.
*/
static bool
fixup_reloclist (arealist * ap)
{
  int n, numrelocs;
  relocation *rp;
  relaction reltype;
  bool ok;
  ok = TRUE;

  if (ap != NULL)
    {
      if (ap->arefcount > 0 && (numrelocs = ap->arnumrelocs) != 0)
	{
	  current_area = ap;
	  current_file = ap->arfileptr;
	  current_objsize = ap->arobjsize;
	  symtbase = ap->arfileptr->obj.symtptr;
	  symbolcount = ap->arfileptr->symtcount;
	  areastart = ap->arobjdata;

	  for (n = 1, rp = ap->areldata; ok && n <= numrelocs; ++n, ++rp)
	    {
	      reltype = decode_reloc (rp);
	      switch (reltype)
		{
		case IGNORE:	/* Throw away relocation */
		  rp->reloffset = ALLFS;
		  break;
		case NOTPOSS:	/* Cannot do relocation */
		  break;
		case POSSIBLE:	/* Relocate and that's all that's needed */
		  relocate_item (rp);
		  rp->reloffset = ALLFS;
		  break;
		case TYPE2_AA:	/* Relocate and create a type-2 area additive as well */
		  relocate_item (rp);
		  alter_reloc (rp, reltype);
		  break;
		case TYPE2_AP:	/* Relocate and create a type-2 area PC-relative as well */
		  if ((rp->reltypesym & REL_TYPE2) == 0)
		    ok = alter_type1_offset (rp);
		  else
		    ok = alter_type2_offset (rp);
		  if (ok)
		    alter_reloc (rp, reltype);
		  break;
		case TYPE2_RP:	/* Cannot do relocation, but modify offset in instruction */
		  alter_area_offset (rp);
		default:
		  break;
		}
	    }
	}

      return ok && fixup_reloclist (ap->left) && fixup_reloclist (ap->right);
    }

  return TRUE;
}

/*
** 'fixup_relocs' is called to try to handle relocations
** as far as it is possible to resolve them at this stage
*/
bool
fixup_relocs (void)
{
  fixup_reloclist (rocodelist);
  fixup_reloclist (rodatalist);
  fixup_reloclist (rwcodelist);
  fixup_reloclist (rwdatalist);
  fixup_reloclist (debuglist);
  return !got_errors ();
}

/*
** The following functions are used to create the image file
*/

/*
** 'write_areas' dumps the areas in one list to the image file.
** Note that, if the area to be written to file has to be aligned
** on anything other than a word boundary, this routine also
** pads the image file with the required number of zeroes
** There is no error checking as the only errors at this stage
** cause the linker to give up on the spot.
*/
static void
write_areas (arealist * ap)
{
  if (ap != NULL)
    {
      write_areas (ap->left);

      if (ap->arefcount > 0 && ap->arobjsize > 0)
	{
	  if (ap->aralign != DEFALIGN)
	    {			/* Area has a funny alignment */
	      int fillsize = ap->arplace - areatop;
	      if (fillsize != 0)
		write_zeroes (fillsize);
	    }
	  convert_endian (ap->arobjdata, ap->arobjsize / 4);
	  write_image (ap->arobjdata, ap->arobjsize);
	  areatop = COERCE (ap->arplace + ap->arobjsize, unsigned int);
	}

      write_areas (ap->right);
    }
}

/*
** 'create_image' writes the final executable image file to disk
*/
void
create_image (void)
{
  link_state = CREATE_IMAGE;
  if (opt_verbose)
    error ("Drlink: Creating executable file '%s'", imagename);
  if (imagetype == RELOC || imagetype == RMOD)
    imagesize += totalrelocs * sizeof (unsigned int);
  open_image ();
  if (headercode != NULL)
    {
      fillin_header ();
      convert_endian (headercode, headersize / 4);
      write_image (headercode, headersize);
    }
  write_areas (rocodelist);
  write_areas (rodatalist);
  write_areas (rwcodelist);
  write_areas (rwdatalist);
  if (opt_keepdebug)
    {
      write_areas (debuglist);
      write_lldtable ();
    }
  if (imagetype == RELOC || imagetype == RMOD)
    write_relocinfo ();
  close_image ();
}

/*
** 'print_arealist' is called to list the areas in an image file of a
** particular type, that is, read-only code, read/write code and so forth
*/
static void
print_arealist (const char *areaclass, arealist * ap)
{
  if (ap != NULL)
    {
      print_arealist (areaclass, ap->left);

      if (ap->arefcount > 0 && ap->arobjsize > 0)
	{
	  unsigned int offset = (imagetype == RMOD) ? progbase : 0;
	  char text[MSGBUFLEN];

	  sprintf (text, "%-6x  %-6x  %s %s from %s\n", ap->arplace - offset,
		   ap->arobjsize, areaclass, ap->arname,
		   ap->arfileptr->chfilename);
	  if (mapfile != NULL)
	    write_mapfile (text);
	  else
	    puts (text);
	}

      print_arealist (areaclass, ap->right);
    }
}

/*
** 'print_areamap' is used to produce the area map at the end of the link if
** one has been requested via the '-map' option
*/
void
print_areamap (void)
{
  char text[500];
  sprintf (text,
	   "Area map of image file '%s':\n\nStart   Size    Type      Name\n",
	   imagename);
  if (mapfile != NULL)
    write_mapfile (text);
  else
    puts (text);
  print_arealist ("R/O code ", rocodelist);
  print_arealist ("R/O data ", rodatalist);
  print_arealist ("R/W code ", rwcodelist);
  print_arealist ("R/W data ", rwdatalist);
  print_arealist ("Zero-init", zidatalist);
  print_arealist ("Debug    ", debuglist);
}

/*
** 'list_unused' is called to list the areas that have been removed
** from the image file as there were no references to them
*/
static void
list_unused (arealist * ap, bool findsymbol)
{
  const char *symname;
  char text[MSGBUFLEN];

  if (ap != NULL)
    {
      list_unused (ap->left, findsymbol);

      if (ap->arefcount == 0)
	{
	  symname = (findsymbol ? find_areasymbol (ap) : NULL);
	  if (symname != NULL)
	    sprintf (text, "    '%s' (%s) from %s", ap->arname, symname,
		     ap->arfileptr->chfilename);
	  else
	    sprintf (text, "    '%s' from %s", ap->arname,
		     ap->arfileptr->chfilename);

	  if (mapfile != NULL)
	    {
	      sprintf (&text[strlen (text)], " (%x bytes)\n", ap->arobjsize);
	      write_mapfile (text);
	    }
	  else
	    error ("%s", text);
	}

      list_unused (ap->right, findsymbol);
    }
}

/*
** 'print_unusedlist' is called to list the areas that were removed
** from the image file
*/
void
print_unusedlist (void)
{
  if (unused == 0)
    {
      if (mapfile != NULL)
	write_mapfile ("\nNo areas were omitted from the image file\n");
      return;
    }
  if (mapfile != NULL)
    write_mapfile ("\nThe following areas were omitted from the image file:\n");
  else
    error ("Drlink: The following areas were omitted from the image file:");
  list_unused (rocodelist, TRUE);
  list_unused (rodatalist, FALSE);
  list_unused (rwcodelist, TRUE);
  list_unused (rwdatalist, FALSE);
  list_unused (zidatalist, FALSE);
}

/*
** 'print_mapfile' is called to write the area map to a file
*/
void
print_mapfile (void)
{
  open_mapfile ();
  print_areamap ();
  if (opt_nounused)
    print_unusedlist ();
  close_mapfile ();
}

/*
** 'init_areas' is called to initialise the area linked lists and other
** bits and pieces relating to areas
*/
void
init_areas (void)
{
  rocodelist = rwcodelist = rodatalist = rwdatalist = zidatalist = debuglist = NULL;
  aofv3flag = FALSE;
  entryarea = NULL;
  gotcodearea = FALSE;
  unused = 0;
  totalrelocs = 0;
  relocaddr = 0;
  workspace = 0;
  headercode = NULL;
  arlimlist = NULL;
  codebase = database = 0;
  progbase = areapc = ABSBASE;
}