/****************************************************************************
 *                                                                          *
 *                            GNATMEM COMPONENTS                            *
 *                                                                          *
 *                                 G M E M                                  *
 *                                                                          *
 *                             $Revision: 1.3 $
 *                                                                          *
 *                          C Implementation File                           *
 *                                                                          *
 *         Copyright (C) 2000-2001 Free Software Foundation, Inc.           *
 *                                                                          *
 * GNAT is free software;  you can  redistribute it  and/or modify it under *
 * terms of the  GNU General Public License as published  by the Free Soft- *
 * ware  Foundation;  either version 2,  or (at your option) any later ver- *
 * sion.  GNAT is distributed in the hope that it will be useful, but WITH- *
 * OUT ANY WARRANTY;  without even the  implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License *
 * for  more details.  You should have  received  a copy of the GNU General *
 * Public License  distributed with GNAT;  see file COPYING.  If not, write *
 * to  the Free Software Foundation,  59 Temple Place - Suite 330,  Boston, *
 * MA 02111-1307, USA.                                                      *
 *                                                                          *
 * As a  special  exception,  if you  link  this file  with other  files to *
 * produce an executable,  this file does not by itself cause the resulting *
 * executable to be covered by the GNU General Public License. This except- *
 * ion does not  however invalidate  any other reasons  why the  executable *
 * file might be covered by the  GNU Public License.                        *
 *                                                                          *
 * GNAT was originally developed  by the GNAT team at  New York University. *
 * It is now maintained by Ada Core Technologies Inc (http://www.gnat.com). *
 *                                                                          *
 ****************************************************************************/

/*  This unit reads the allocation tracking log produced by augmented
    __gnat_malloc and __gnat_free procedures (see file a-raise.c) and
    provides GNATMEM tool with gdb-compliant output. The output is
    processed by GNATMEM to detect dynamic memory allocation errors.

    See GNATMEM section in GNAT User's Guide for more information.

    NOTE: This capability is currently supported on the following targets:

      DEC Unix
      SGI Irix
      GNU/Linux x86
      Solaris (sparc and x86) (*)
      Windows 98/95/NT (x86)

    (*) on these targets, the compilation must be done with -funwind-tables to
    be able to build the stack backtrace.   */

#ifdef __alpha_vxworks
#include "vxWorks.h"
#endif

#ifdef IN_RTS
#include "tconfig.h"
#include "tsystem.h"
#else
#include "config.h"
#include "system.h"
#endif

#include "adaint.h"

static FILE *gmemfile;

/* tb_len is the number of call level supported by this module */
#define TB_LEN 200

static char *tracebk [TB_LEN];
static int cur_tb_len, cur_tb_pos;

extern void convert_addresses		PARAMS ((char *[], int, void *,
						 int *));
static void gmem_read_backtrace 	PARAMS ((void));
static char *spc2nul			PARAMS ((char *));

extern int __gnat_gmem_initialize	PARAMS ((char *));
extern void __gnat_gmem_a2l_initialize	PARAMS ((char *));
extern void __gnat_gmem_read_next	PARAMS ((char *));
extern void __gnat_gmem_read_bt_frame	PARAMS ((char *));

/* Reads backtrace information from gmemfile placing them in tracebk
   array. cur_tb_len is the size of this array.   */

static void
gmem_read_backtrace ()
{
  fread (&cur_tb_len, sizeof (int), 1, gmemfile);
  fread (tracebk, sizeof (char *), cur_tb_len, gmemfile);
  cur_tb_pos = 0;
}

/* Initialize gmem feature from the dumpname file. Return 1 if the
   dumpname has been generated by GMEM (instrumented malloc/free) and 0 if not
   (i.e. probably a GDB generated file). */

int
__gnat_gmem_initialize (dumpname)
     char *dumpname;
{
  char header[10];

  gmemfile = fopen (dumpname, "rb");
  fread (header, 10, 1, gmemfile);

  /* Check for GMEM magic-tag.  */
  if (memcmp (header, "GMEM DUMP\n", 10))
    {
      fclose (gmemfile);
      return 0;
    }

  return 1;
}

/* Initialize addr2line library */

void
__gnat_gmem_a2l_initialize (exename)
     char *exename;
{
  extern char **gnat_argv;
  char s [100];
  int l;

  gnat_argv [0] = exename;
  convert_addresses (tracebk, 1, s, &l);
}

/* Read next allocation of deallocation information from the GMEM file and
   write an alloc/free information in buf to be processed by GDB (see gnatmem
   implementation). */

void
__gnat_gmem_read_next (buf)
     char *buf;
{
  void *addr;
  int size;
  int j;

  j = fgetc (gmemfile);
  if (j == EOF)
    {
      fclose (gmemfile);
      sprintf (buf, "Program exited.");
    }
  else
    {
      switch (j)
        {
          case 'A' :
            fread (&addr, sizeof (char *), 1, gmemfile);
            fread (&size, sizeof (int), 1, gmemfile);
            sprintf (buf, "ALLOC^%d^0x%lx^", size, (long) addr);
            break;
          case 'D' :
            fread (&addr, sizeof (char *), 1, gmemfile);
            sprintf (buf, "DEALL^0x%lx^", (long) addr);
            break;
          default:
            puts ("GMEM dump file corrupt");
            __gnat_os_exit (1);
        }

      gmem_read_backtrace ();
    }
}

/* Scans the line until the space or new-line character is encountered;
   this character is replaced by nul and its position is returned.  */

static char *
spc2nul (s)
     char *s;
{
  while (*++s)
    if (*s == ' ' || *s == '\n')
      {
	*s = 0;
	return s;
      }

  abort ();
}

/* Convert backtrace address in tracebk at position cur_tb_pos to a symbolic
   traceback information returned in buf and to be processed by GDB (see
   gnatmem implementation).  */

void
__gnat_gmem_read_bt_frame (buf)
     char *buf;
{
  int l = 0;
  char s[1000];
  char *name, *file;

  if (cur_tb_pos >= cur_tb_len)
    {
      buf [0] = ' ';
      buf [1] = '\0';
      return;
    }

  convert_addresses (tracebk + cur_tb_pos, 1, s, &l);
  s[l] = '\0';
  name = spc2nul (s) + 4;
  file = spc2nul (name) + 4;
  spc2nul (file);
  ++cur_tb_pos;

  sprintf (buf, "#  %s () at %s", name, file);
}
