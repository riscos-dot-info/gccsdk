/*
 * AsAsm an assembler for ARM
 * Copyright (c) 1992 Niklas Röjemo
 * Copyright (c) 2001-2014 GCCSDK Developers
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

/* Contains OS specific function implementations.  */

#include "config.h"

#include <limits.h>
#include <stdlib.h>

#include "error.h"
#include "os.h"

#ifdef __riscos__
/* RISC OS version.  */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <stdint.h>
#include <kernel.h>
#include <swis.h>

#include "main.h"

#ifdef __TARGET_UNIXLIB__
#  include <unixlib/local.h>
#endif

#ifdef __TARGET_UNIXLIB__
int __riscosify_control = __RISCOSIFY_NO_PROCESS;
#endif

/* The DDEUtils module throwback error category codes.  */
#define THROWBACK_INFORMATION         -1
#define THROWBACK_WARNING              0
#define THROWBACK_ERROR                1
#define THROWBACK_SERIOUS_ERROR        2

/* The DDEUtils module throwback reason codes.  */
#define THROWBACK_REASON_PROCESSING    0
#define THROWBACK_REASON_ERROR_DETAILS 1
#define THROWBACK_REASON_INFO_DETAILS  2

static const char *oThrowbackErrorFile;

/**
 * Converts given SWI name into its SWI number.
 * \param swiname NUL terminated SWI name
 * \return SWI number or -1 when the conversion can't be done.
 */
int
OS_SWINameToNum (const char *swiname)
{
  _kernel_swi_regs regs;
  regs.r[1] = (int) swiname;
  return _kernel_swi (OS_SWINumberFromString, &regs, &regs) ? -1 : regs.r[0];
}

const _kernel_oserror *
OS_ThrowbackStart (void)
{
  _kernel_swi_regs regs;
  return _kernel_swi (DDEUtils_ThrowbackStart, &regs, &regs);
}

const _kernel_oserror *
OS_ThrowbackSendStart (const char *fileName)
{
  oThrowbackErrorFile = fileName;
  _kernel_swi_regs regs;
  regs.r[0] = THROWBACK_REASON_PROCESSING;
  regs.r[2] = (int) fileName;
  return _kernel_swi (DDEUtils_ThrowbackSend, &regs, &regs);
}

/**
 * \param level 0 for warning, 1 for error, 2 for serious error.
 * \param errstr nul terminated description of error
 */
const _kernel_oserror *
OS_ThrowbackSendError (int level, unsigned lineNum, const char *errstr)
{
  _kernel_swi_regs regs;
  regs.r[0] = level == ThrowbackInfo ? THROWBACK_REASON_INFO_DETAILS : THROWBACK_REASON_ERROR_DETAILS;
  regs.r[1] = 0;
  regs.r[2] = (int) oThrowbackErrorFile;
  regs.r[3] = (int) lineNum;
  regs.r[4] = level == ThrowbackInfo ? 0 : level;
  regs.r[5] = (int) errstr;
  return _kernel_swi (DDEUtils_ThrowbackSend, &regs, &regs);
}

const _kernel_oserror *
OS_ThrowbackEnd (void)
{
  oThrowbackErrorFile = NULL;
  _kernel_swi_regs regs;
  return _kernel_swi (DDEUtils_ThrowbackEnd, &regs, &regs);
}

#else
/* Non RISC OS version.  */
#  include <ctype.h>
#  include <string.h>

#  include <glob.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>

#endif

/**
 * Create ASFile structure for given filename.
 * \param fileName Filename for which ASFile object needs to be created.
 * \param asFileP ASFile object to be filled in.  May be uninitialsed.
 * \return false for success, true otherwise (like unexisting file).
 */
bool
ASFile_Create (const char *fileName, ASFile *asFileP)
{
  asFileP->canonName = NULL;
  asFileP->size = 0;
  asFileP->attribs= 0;
  asFileP->loadAddress = asFileP->execAddress = 0;

#ifdef __riscos__
  _kernel_swi_regs regs;
  regs.r[0] = 17;
  regs.r[1] = (int) fileName;
  if (_kernel_swi (OS_File, &regs, &regs) || (regs.r[0] != 1 && regs.r[0] != 3))
    return true;
  asFileP->canonName = realpath (fileName, NULL);
  asFileP->size = (off_t) regs.r[4];
  asFileP->attribs = (uint8_t) regs.r[5];
  asFileP->execAddress = (uint32_t) regs.r[3];
  asFileP->loadAddress = (uint32_t) regs.r[2];

#else

  struct stat sinfo;
  if (stat (fileName, &sinfo) != 0 || !S_ISREG (sinfo.st_mode))
    {
      /* Given file does not exist.  Try with ,XYZ filetype extension.  */
      size_t len = strlen (fileName);
      /* Bail out when there is already a ,XYZ file extension used.  */
      if (len > sizeof (",xyz")-1 && fileName[len - 4] == ',')
	return true;

      char *extFileName = malloc (len + 4 + 1);
      if (!extFileName)
	Error_OutOfMem ();
      memcpy (extFileName, fileName, len);
      memcpy (extFileName + len, ",???", sizeof (",???"));

      glob_t ginfo;
      if (glob (extFileName, GLOB_ERR | GLOB_NOSORT, NULL, &ginfo) != 0)
	{
	  free (extFileName);
	  return true;
	}
      free (extFileName);
      if (ginfo.gl_pathc != 1
          || stat (ginfo.gl_pathv[0], &sinfo) != 0 || !S_ISREG (sinfo.st_mode))
	{
	  globfree (&ginfo);
	  return true;
	}
      asFileP->canonName = realpath (ginfo.gl_pathv[0], NULL);
      globfree (&ginfo);
    }
  else
    asFileP->canonName = realpath (fileName, NULL);
  asFileP->size = sinfo.st_size;
  asFileP->attribs = ((sinfo.st_mode & S_IRUSR) ? (1<<0) : 0)
		       | ((sinfo.st_mode & S_IWUSR) ? (1<<1) : 0)
		       | ((sinfo.st_mode & (S_IRGRP | S_IROTH)) ? (1<<4) : 0)
		       | ((sinfo.st_mode & (S_IWGRP | S_IWOTH)) ? (1<<5) : 0);
  unsigned fileType;
  fileName = asFileP->canonName;
  size_t len = strlen (fileName);
  if (len > sizeof (",xyz")-1 && fileName[len - 4] == ','
      && isxdigit ((unsigned char)fileName[len - 3])
      && isxdigit ((unsigned char)fileName[len - 2])
      && isxdigit ((unsigned char)fileName[len - 1]))
    fileType = (fileName[len - 1] >= '0' && fileName[len - 1] <= '9' ? fileName[len - 1] - '0' : (fileName[len - 1] | 0x20) - 'a' + 10)
                 + ((fileName[len - 2] >= '0' && fileName[len - 2] <= '9' ? fileName[len - 2] - '0' : (fileName[len - 2] | 0x20) - 'a' + 10) << 4)
                 + ((fileName[len - 3] >= '0' && fileName[len - 3] <= '9' ? fileName[len - 3] - '0' : (fileName[len - 3] | 0x20) - 'a' + 10) << 8);
  else
    fileType = 0xFFFU;
  const uint64_t rotime = sinfo.st_mtime * 100 + 0x336e996a00U;
  asFileP->execAddress = rotime & 0xFFFFFFFFU;
  asFileP->loadAddress = 0xFFF00000U + (fileType << 8) + ((rotime >> 32) & 0xFF);
#endif
  
  return false;
}

void
ASFile_Free (ASFile *asFileP)
{
  free ((void *)asFileP->canonName);
}

/**
 * \return malloc'd block holding the current working directory.  Caller gets
 * ownership.
 */
const char *
OS_GetCWD (void)
{
#ifndef __riscos
  size_t curBufSize = 32;
  char *bufP = malloc (curBufSize);
  if (bufP == NULL)
    Error_OutOfMem ();
  while (getcwd (bufP, curBufSize) == NULL)
    {
      free (bufP);
      curBufSize *= 2;
      if (curBufSize > PATH_MAX || (bufP = malloc (curBufSize)) == NULL)
	Error_OutOfMem ();
    }
  return bufP;
#else
  return realpath ("@", NULL);
#endif
}
