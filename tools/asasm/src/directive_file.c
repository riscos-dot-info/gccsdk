/*
 * AsAsm an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 1997 Darren Salt
 * Copyright (c) 2000-2013 GCCSDK Developers
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

#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "directive_file.h"
#include "error.h"
#include "filestack.h"
#include "include.h"
#include "input.h"
#include "lit.h"
#include "main.h"
#include "put.h"

/**
 * Called for GET / INCLUDE
 */
bool
c_get (void)
{
  char *fileName;
  if ((fileName = strdup (Input_Rest ())) == NULL)
    Error_OutOfMem ();
  char *cptr;
  for (cptr = fileName; *cptr && !isspace ((unsigned char)*cptr); cptr++)
    /* */;
  if (*cptr)
    {
      *cptr++ = '\0'; /* must be a space */
      while (*cptr && isspace ((unsigned char)*cptr))
	cptr++;
      if (*cptr && *cptr != ';')
	Error (ErrorError, "Skipping extra characters '%s' after filename", cptr);
    }

  if (!FS_PushFilePObject (fileName) && option_verbose)
    fprintf (stderr, "Including file \"%s\" as \"%s\"\n", fileName, FS_GetCurFileName ());
  free (fileName);
  return false;
}

/**
 * Implements LNK.
 */
bool
c_lnk (void)
{
  char *fileName;
  if ((fileName = strdup (Input_Rest ())) == NULL)
    Error_OutOfMem ();
  char *cptr;
  for (cptr = fileName; *cptr && !isspace ((unsigned char)*cptr); cptr++)
    /* */;
  if (*cptr)
    {
      *cptr++ = '\0'; /* must be a space */
      while (*cptr && isspace ((unsigned char)*cptr))
	cptr++;
      if (*cptr && *cptr != ';')
	Error (ErrorError, "Skipping extra characters '%s' after filename", cptr);
    }

  /* Terminate all outstanding macro calls and finish the current file.  */
  while (gCurPObjP->type == POType_eMacro)
    FS_PopPObject (true);
  FS_PopPObject (true);

  /* Dump literal pool.  */
  Lit_DumpPool ();

  if (!FS_PushFilePObject (fileName) && option_verbose)
    fprintf (stderr, "Linking to file \"%s\" as \"%s\"\n", fileName, FS_GetCurFileName ());
  free (fileName);
  return false;
}

/**
 * Implements BIN / INCBIN.
 */
bool
c_incbin (void)
{
  char *fileNameP;
  if ((fileNameP = strdup (Input_Rest ())) == NULL)
    Error_OutOfMem ();
  char *cptr;
  for (cptr = fileNameP; *cptr && !isspace ((unsigned char)*cptr); cptr++)
    /* */;
  *cptr = '\0';

  ASFile asFile;
  FILE *binfp = !Include_Find (fileNameP, &asFile) ? Include_OpenForRead (fileNameP, &asFile) : NULL;
  if (!binfp)
    Error (ErrorError, "Cannot open file \"%s\"", fileNameP);
  else
    {
      /* Register canonicalised filename so that it gets output in the
	 depends file.  */
      (void) FS_StoreFileName (asFile.canonName);

      /* Include binary file.  */
      int c;
      while ((c = getc (binfp)) != EOF)
	Put_Data (1, c);
      fclose (binfp);
    }

  free (fileNameP);
  ASFile_Free (&asFile);
  return false;
}

/**
 * Implements END.
 */
bool
c_end (void)
{
  if (gCurPObjP->type == POType_eMacro)
    Error (ErrorError, "Cannot use END within a macro");
  else
    FS_PopPObject (false);
  return false;
}
