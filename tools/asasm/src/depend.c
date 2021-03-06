/*
 * AsAsm an assembler for ARM
 * Copyright (c) 1997 Darren Salt
 * Copyright (c) 2004-2013 GCCSDK Developers
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

#include <stdio.h>

#include "depend.h"
#include "error.h"
#include "filestack.h"

const char *DependFileName = NULL;

void
Depend_Write (const char *objname)
{
  if (DependFileName == NULL)
    return;

  FILE *dependFile;
  if ((dependFile = fopen (DependFileName, "w")) == NULL)
    Error_Abort ("Failed to open dependencies file \"%s\"", DependFileName);

  if (objname[0] == '@' && objname[1] == '.')
    objname += 2;
  fprintf (dependFile, "%s:", objname);
  for (const FileNameList *fnListP = gFileNameListP; fnListP != NULL; fnListP = fnListP->nextP)
    fprintf (dependFile, " %s", fnListP->fileName);

  fputc ('\n', dependFile);
  fclose (dependFile);
}
