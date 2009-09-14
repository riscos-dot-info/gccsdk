/*
** Drlink AOF Linker
**
** Copyright (c) 1993, 1994, 1995, 1996, 1997, 1998  David Daniels
** Copyright (c) 2001, 2002, 2003, 2004, 2005  GCCSDK Developers
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
** Debugging functions for Drlink
*/

#include "config.h"

#include <stdio.h>

#include "drlhdr.h"
#include "filehdr.h"
#include "symbolhdr.h"
#include "areahdr.h"
#include "debugprocs.h"
#include "libraries.h"

#ifdef DEBUG
void
list_areas (const arealist * list)
{
  if (list != NULL)
    {
      arearef *rp;

      list_areas (list->left);

      printf ("Area entry at %p, areaname='%s', base entry at %p, file entry at %p, AT=%x\n",
	      list, list->arname, list->arbase, list->arfileptr, list->aratattr);
      printf ("  OBJ_AREA at %p, size=%x, relocations at %p, number=%d, area address=%x",
	      list->arobjdata, list->arobjsize, list->areldata, list->arnumrelocs,
	      list->arplace);
      if (list->arsymbol != NULL)
	printf (", symbol address=%p", list->arsymbol);
      printf ("\n  Reference count=%d, reference list at %p", list->arefcount,
	      list->areflist);
      if ((rp = list->areflist) == NULL)
	printf (", ** No references **\n");
      else
	{
	  printf (", References:\n  ");
	  for ( /* */ ; rp != NULL; rp = rp->arefnext)
	    printf ("  %p=%d", rp->arefarea, rp->arefcount);
	  fputc ('\n', stdout);
	}

      list_areas (list->right);
    }
}

void
list_allareas (void)
{
  printf ("\n========== Read-only code list ==========\n");
  list_areas (rocodelist);
  printf ("\n========== Read-only data list ==========\n");
  list_areas (rodatalist);
  printf ("\n========== Read/write code list ==========\n");
  list_areas (rwcodelist);
  printf ("\n========== Read/write data list ==========\n");
  list_areas (rwdatalist);
  printf
    ("\n========== Zero-initialised data and common block list ==========\n");
  list_areas (zidatalist);
  printf ("\n========== Debug list ==========\n");
  list_areas (debuglist);
}


void
list_symbols (const symbol * list)
{
  static int indent = 0;

  if (list != NULL)
    {
      symtentry *sp;
      int spaces = indent;

      while (spaces--)
	fputc (' ', stdout);

      sp = list->symtptr;
      printf ("    Symbol at %p, symt at %p, name='%s', hash=%x, attributes=%x, value=%x",
	      list, sp, sp->symtname, list->symhash, sp->symtattr, sp->symtvalue);
      if (sp->symtarea.areaptr != NULL)
	printf (", area at %p", sp->symtarea.areaptr);
      fputc ('\n', stdout);

      indent++;

      if (!list->left && list->right)
	{
	  spaces = indent;
	  while (spaces--)
	    fputc (' ', stdout);
	  puts ("    (NULL)");
	}

      list_symbols (list->left);
      list_symbols (list->right);
      indent--;
    }
}

static void
list_symtable (symbol * table[], unsigned int stsize)
{
  unsigned int n;
  printf ("  Symbol table at %p\n", table);
  for (n = 0; n < stsize; )
    {
      const symbol *list;
      unsigned int low = n;
      while ((list = table[n]) == NULL && n < stsize)
	n += 1;
      if (n != low)
	printf ("  Index=%2d..%-2d, no entries\n", low, n - 1);
      if (n < stsize)
	{
	  printf ("  Index=%2d, symbol list starts at %p\n", n, list);
	  list_symbols (list);
	  n += 1;
	}
    }
}

#if 0
/* NOT USED */
void
list_libentries (libentry * list)
{
  if (list != NULL)
    {
      list_libentries (list->left);
      printf
	("    Lib entry at %p, name='%s', hash=%x, member name='%s', offset in library=%x, size=%x\n",
	 list, list->libname, list->libhash, list->libmember, list->liboffset,
	 list->libsize);
      list_libentries (list->right);
    }
}

/* NOT USED */
void
list_libtable (libtable * table)
{
  libentry *list;
  int n;
  printf ("  Library symbol table at %p\n", table);
  for (n = 0; n < MAXENTRIES; n++)
    {
      if ((list = *table[n]) == NULL)
	{
	  printf ("  Index=%2d, no entries\n", n);
	}
      else
	{
	  printf ("  Index=%2d, symbol list starts at %p\n", n, list);
	  list_libentries (list);
	}
    }
}
#endif

static void
list_filelist (filelist * list)
{
  for ( /* */ ; list != NULL; list = list->nextfile)
    {
      printf ("File def at %p, name='%s', file size=%x\n", list,
	      list->chfilename, list->chfilesize);
      printf ("  OBJ_HEAD is at %p, size=%x, OBJ_SYMT is at %p, size=%x\n",
	      list->obj.headptr, list->obj.headsize, list->obj.symtptr,
	      list->obj.symtsize);
      printf ("  OBJ_AREA is at %p, size=%x, OBJ_STRT is at %p, size=%x\n",
	      list->obj.areaptr, list->obj.areasize, list->obj.strtptr,
	      list->obj.strtsize);
      printf ("  Areas=%u, symbols=%u Local symbols at %p, 'wanted' list at %p\n",
	      list->areacount, list->symtcount, &list->localsyms,
	      list->symtries.wantedsyms);
      list_symtable (list->localsyms, MAXLOCALS);
      fputc('\n', stdout);
    }
}

static void
list_liblist (libheader * list)
{
  for ( /* */ ; list != NULL; list = list->libflink)
    printf ("Library def at %p, name='%s', %s loaded at %p, library size=%x\n",
	    list, list->libname, list->fullyloaded ? "fully" : "partially", list->lib.base, list->libextent);
}

void
list_files (void)
{
  printf ("\n========== AOF file list ==========\n");
  list_filelist (aofilelist);
  printf ("\n========== Library list ==========\n");
  list_liblist (liblist);
  printf ("\n========== Global Symbol Table ==========\n");
  list_symtable (globalsyms, MAXGLOBALS);
  printf ("\n========== Common Symbol Table ==========\n");
  list_symtable (commonsyms, MAXCOMMON);
}
#endif