/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/unix/rmdir.c,v $
 * $Date: 2001/09/04 16:32:04 $
 * $Revision: 1.2.2.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: rmdir.c,v 1.2.2.2 2001/09/04 16:32:04 admin Exp $";
#endif

#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include <unixlib/os.h>
#include <swis.h>

#include <unixlib/local.h>
#include <unixlib/swiparams.h>

int
rmdir (const char *ux_directory)
{
  char directory[_POSIX_PATH_MAX];
  int regs[10], filetype;
  _kernel_oserror *err;
  char scratch_buf[NAME_MAX];

  if (ux_directory == NULL)
    return __set_errno (EINVAL);

  if (!__riscosify_std (ux_directory, 0, directory, sizeof (directory),
                      &filetype))
    return __set_errno (ENAMETOOLONG);

  /* Does the directory exist ?  */
  if (__os_file (OSFILE_READCATINFO_NOPATH, directory, regs) || regs[0] == 0)
    return __set_errno (ENOENT);

  /* Images and directories have bit 1 set. Clear implies file.  */
  if ((regs[0] & 2) == 0 || filetype != __RISCOSIFY_FILETYPE_NOTFOUND)
    return __set_errno (ENOTDIR);

  /* Directory is locked against deletion if bit 3 is set.  */
  if (regs[5] & (1 << 3))
    return __set_errno (EACCES);

  /* Explicitly check that DIRECTORY is empty, otherwise non-empty image
     files can be zapped. We loop because OS_GBPB is allowed to do nothing
     due to external constraints imposed by some filing systems (PRM 2-70).  */
  do
    {
      regs[0] = 9;
      regs[1] = (int) directory;
      regs[2] = (int) scratch_buf;
      regs[3] = 1;
      regs[4] = 0;
      regs[5] = sizeof (scratch_buf);
      regs[6] = 0;	/* Match anything.  */

      err = __os_swi (OS_GBPB, regs);
      if (err)
	{
	  /* Oh shit. Let's pretend that it's not empty.  */
	  __seterr (err);
	  return __set_errno (ENOTEMPTY);
	}
      if (regs[3] > 0)
	return __set_errno (ENOTEMPTY);
    }
  while (regs[4] != -1);

  /* The directory must be empty to get here, so delete it.  */
  regs[0] = 6;
  if (__os_swi (OS_File, regs) == NULL)
    return 0;

  /* Hmm, can't delete it, it's not locked, it is empty => read only fs.  */
  return __set_errno (EROFS);
}
