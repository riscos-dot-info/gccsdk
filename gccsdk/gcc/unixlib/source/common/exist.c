/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/common/exist.c,v $
 * $Date: 2000/07/15 14:52:18 $
 * $Revision: 1.1.1.1 $
 * $State: Exp $
 * $Author: nick $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: exist.c,v 1.1.1.1 2000/07/15 14:52:18 nick Exp $";
#endif

#include <limits.h>
#include <unixlib/local.h>

/* Return nonzero if DIR is an existent directory.  */
int
__isdir (const char *ux_dir)
{
  char dir[_POSIX_PATH_MAX];
  int filetype;

  if (! __riscosify_std (ux_dir, 0, dir, sizeof (dir), &filetype)
      || filetype != __RISCOSIFY_FILETYPE_NOTFOUND)
    return 0;

  return __isdir_raw (dir);
}

/* Return nonzero if FILE exists.  */
int
__object_exists (const char *ux_file)
{
  char file[_POSIX_PATH_MAX];
  int filetype;

  if (! __riscosify_std (ux_file, 0, file, sizeof (file), &filetype)
      || filetype != __RISCOSIFY_FILETYPE_NOTFOUND)
    return 0;

  return __object_exists_raw (file);
}
