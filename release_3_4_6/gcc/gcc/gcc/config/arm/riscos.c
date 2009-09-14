/* Functions for RISC OS as target machine for GNU C compiler.
   Copyright (C) 1997, 1999, 2003, 2004 Free Software Foundation, Inc.
   Contributed by Nick Burrett (nick@sqrt.co.uk)

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#define __UNIXLIB_INTERNALS
#include <unixlib/os.h>
#include <unixlib/local.h>
#include <swis.h>
#include <obstack.h>

/* The DDEUtils module throwback error category codes.  */
#define THROWBACK_INFORMATION         -1
#define THROWBACK_WARNING              0
#define THROWBACK_ERROR                1
#define THROWBACK_SERIOUS_ERROR        2

/* The DDEUtils module throwback reason codes.  */
#define THROWBACK_REASON_PROCESSING    0
#define THROWBACK_REASON_ERROR_DETAILS 1
#define THROWBACK_REASON_INFO_DETAILS  2

/* The full canonicalised pathname of the current error file.  */
static char *arm_error_file = NULL;

/* Unique reference number for current file.
   Used to determine whether DDEUtils module needs telling we
   have changed the error file.  */
static int arm_error_file_ref = -1;

/* Control status of throwback,
   -1 => no throwback possible
    0 => throwback uninitialised,
    1 => throwback initialised
 */
static int arm_throwback_started = 0;

void arm_throwback_finish (void);

/* Initialise the DDEUtils module for throwback.  */
static void
arm_throwback_start (void)
{
  int regs[10];
  _kernel_oserror *err = __os_swi (DDEUtils_ThrowbackStart, regs);
  if (err)
    {
      /*printf ("-- failed to initialise throwback: '%s'\n", err);*/
      arm_throwback_started = 1;
    }
  else
    {
      atexit (arm_throwback_finish);
      arm_throwback_started = 1;
    }
}

/* Tell DDEUtils that we are processing a new file.
   The DDE documentation is unclear, but does suggest that this
   message should be sent if the filename changes.  */
static void
arm_throwback_new_file (const char *fname)
{
  int regs[10];

  regs[0] = THROWBACK_REASON_PROCESSING;
  regs[2] = (int)fname;
  if (__os_swi (DDEUtils_ThrowbackSend, regs))
    arm_throwback_started = -1;
}


/* Send details of a specific error to DDEUtils module.  */
static void
arm_throwback_error (const char *fname, int level,
		     int line_number, const char *error)
{
  int regs[10];

  /* printf ("ate: '%s'\n", error); */

  regs[0] = (level == THROWBACK_INFORMATION)
	     ? THROWBACK_REASON_INFO_DETAILS
	     : THROWBACK_REASON_ERROR_DETAILS;
  regs[1] = 0;
  regs[2] = (int)fname;
  regs[3] = line_number;
  regs[4] = (level == THROWBACK_INFORMATION) ? 0 : level;
  regs[5] = (int)error;
  if (__os_swi (DDEUtils_ThrowbackSend, regs))
    arm_throwback_started = -1;
}

/* Tell DDEUtils that we have finished throwing errors.  */
void
arm_throwback_finish (void)
{
  int regs[10];

  if (arm_throwback_started > 0)
    __os_swi (DDEUtils_ThrowbackEnd, regs);
  arm_throwback_started = 0;
}

/* Convert from Unix name to RISC OS name and canonicalise the name
   so that throwback knows the full pathname of the file.
   Return the converted filename, stored in arm_error_file, or NULL.  */
static char *
riscos_canonicalise_filename (const char *sname)
{
  char *s;
  int  regs[10];
  char filename[1024];

  /* It's possible that the filename will be in Unix format.
     Convert it into RISC OS format.  */
  s = __riscosify (sname, 0, 0, filename, sizeof (filename), NULL);
  /* Filename translation failed, so return NULL.  */
  if (s == NULL)
    return NULL;

  /* The first call will calculate the size of buffer needed to
     store the canonicalised filename.  */
  regs[0] = 37;
  regs[1] = (int)filename;
  regs[2] = 0;
  regs[3] = 0;
  regs[4] = 0;
  regs[5] = 0;
  if (!__os_swi (OS_FSControl, regs) && regs[5] < 0)
    {
      if ((arm_error_file = realloc (arm_error_file, 1 - regs[5])) != NULL)
        {
          /* Now perform the canonicalisation and store in a buffer
             which we know is large enough.  */
          regs[1] = (int)filename;
          regs[2] = (int)arm_error_file;
          regs[3] = 0;
          regs[4] = 0;
          regs[5] = 1 - regs[5];
          if (!__os_swi (OS_FSControl, regs) && regs[5] == 1)
            return arm_error_file;
        }
    }

  /* Canonicalisation failed, so free arm_error_file and set it to null.  */
  free (arm_error_file);
  arm_error_file = NULL;
  return NULL;
}

/* Throwback interface to GNU family of compilers.  */
void
arm_error_throwback (const char *file, int line, const char *prefix,
		     const char *s)
{
  size_t flen, slen, plen;
  char sline[16];
  int iserr;

  /* If the filename is not specified or "", then return.  */
  if (file == NULL || *file == '\0')
    return;

  /* Initialise throwback.  */
  if (!arm_throwback_started)
    arm_throwback_start ();

  /* printf ("*** throwback: tb=%d, file='%s', line='%d', prefix='%s', s='%s'\n",
          arm_throwback_started, file, line, prefix, s); */

  flen = strlen (file);
  slen = sprintf (sline, "%d: ", line);
  iserr = prefix == NULL || strcmp (prefix, "warning");
  plen = (iserr) ? sizeof("error: ")-1 : sizeof("warning: ")-1;

  if (arm_throwback_started > 0 && riscos_canonicalise_filename (file))
    {
      const char *p;

      arm_throwback_new_file (arm_error_file);

      /* Errors generated by the C++ compiler and preprocessor can stretch
	 over multiple lines.  DDEUtils uses the newline as a string
	 terminator.  We must call arm_throwback_error() for as many times
	 as there are newlines.  */
      for (p = s; *p != '\0'; /* */)
	{
	  /* Skip filename, line and level (warning/error) in the 'p' message:  */
	  if (!strncmp (p, file, flen) && p[flen] == ':')
	    {
	      p += flen + 1; /* Skip filename + ":" */
	      if (!strncmp (p, sline, slen))
	        {
	          p += slen; /* Skip line num + ": " */
	          if (!strncmp (p, (iserr) ? "error: " : "warning: ", plen))
	            p += plen; /* Skip level */
	        }
	    }

	  arm_throwback_error (arm_error_file,
			       (!iserr) ? THROWBACK_WARNING : THROWBACK_ERROR,
			       line, p);

	  while (*p != '\0' && *p != '\n')
	    p++;
	  if (*p == '\n')
	    p++;
	}
    }
}

/* Error category mapping for GNAT errors.  */
typedef enum
{
  cat_none,
  cat_warning,
  cat_error
} category;

/* Throwback an error from GNAT.
   The column parameter is unused, but maybe the throwback system will
   be able to use it sometime in the future.  */
void
arm_gnat_error_throwback (text, sname, sfile, line, column, cat)
  char *text;		/* The error message.  */
  char *sname;		/* The file containing the error.  */
  int  sfile;		/* A unique reference number for the file.  */
  int  line;		/* The line number of the error.  */
  int  column;		/* The column of the error.  */
  category cat;		/* The category of the error.  */
{
  /* If the filename is not specified or "", then return.  */
  if (sname == NULL || *sname == '\0')
    return;
  /* Initialise throwback.  */
  if (!arm_throwback_started)
    arm_throwback_start();

  if (arm_throwback_started > 0)
    {
      /* If this is a new file, then work out the RISC OS name and
         canonicalise the name so that throwback knows the full pathname
         of the file.  */
      if (sfile != arm_error_file_ref)
	{
	  arm_error_file_ref = sfile;
	  if (riscos_canonicalise_filename (sname))
	    arm_throwback_new_file (arm_error_file);
	  else
	    arm_throwback_started = -1;
	}
      /* Promote category none into category error.  */
      if (cat == cat_none)
        cat = cat_error;
      /* Throwback working, so send the message.  */
      if (arm_throwback_started > 0)
	arm_throwback_error (arm_error_file,
			     (cat == cat_none
			      ? THROWBACK_INFORMATION
			      : (cat == cat_warning
			         ? THROWBACK_WARNING : THROWBACK_ERROR)),
			     line, text);
    }
}

/* Function is used by the GCC_DRIVER_HOST_INITIALIZATION macro
   in gcc.c.  */
void riscos_host_initialisation (void)
{
  /* Perform a simple memory check.  Notify the user that there is
     not enough space in the `next' slot for a task and die.  */
  int regs[10];

  regs[0] = -1;
  regs[1] = -1;
  __os_swi (Wimp_SlotSize, regs);
  if (regs[0] < (4000 * 1024))
    {
      fprintf (stderr,
	       "Application requires a minimum of 4000K to run.\n");
      exit (1);
    }
}

/* Called from the gcc driver to convert a RISC OS filename into a
   Unix format name.  */
const char *riscos_convert_filename (void *obstack, const char *name, int do_exe, int do_obj)
{
  struct obstack *obs = (struct obstack *) obstack;
  char tmp[1024];

  __unixify_std(name, tmp, sizeof(tmp), 0);

  return obstack_copy0 (obs, tmp, strlen (tmp));
}