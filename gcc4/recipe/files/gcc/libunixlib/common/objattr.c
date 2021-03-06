/* __object_get_attrs ()
 * Copyright (c) 2000-2013 UnixLib Developers
 */

#include <errno.h>
#include <stdlib.h>
#include <swis.h>
#include <string.h>

#include <unixlib/local.h>
#include <internal/os.h>
#include <internal/local.h>
#include <internal/unix.h>
#include <internal/symlinks.h>
#include <internal/swiparams.h>

/* Get an object's filetype, object type, etc and do some common checks.
   Returns nonzero and sets errno on error. Returns riscosified filename
   in __buffer. objtype, ftype, etc may be NULL if not needed.
   Optionally follow symlinks.  */
static int
get_attrs (const char *ux_file, char *buffer, size_t buf_len,
	   int *objtype, int *ftype, int *loadaddr, int *execaddr,
	   int *length, int *attr, int follow_symlink)
{
  if (ux_file == NULL)
    return __set_errno (EFAULT);
  if (ux_file[0] == '\0')
    return __set_errno (ENOENT);

  /* Convert the filename. We don't want to guess the filetype of the file
     based on the extension, as this may cause the filetype check later on
     to fail. */
  int sftype;
  if (!__riscosify (ux_file, 0,
		    __get_riscosify_control () | __RISCOSIFY_FILETYPE_NOT_SET,
		    buffer, buf_len, &sftype))
    return __set_errno (ENAMETOOLONG);

#if __UNIXLIB_SYMLINKS
  if (follow_symlink)
    {
      char *target;
      if ((target = malloc (buf_len)) == NULL)
	return -1;

      if (__resolve_symlinks (buffer, target, buf_len, 0) != 0)
	{
	  free (target);
	  return -1;
	}
      strcpy (buffer, target);
      free (target);
    }
#endif
  /* Get catalogue information.  */
  unsigned objtype_cp, loadaddr_cp;
  const _kernel_oserror *err;
  if ((err = SWI_OS_File_ReadCatInfo (buffer, &objtype_cp, &loadaddr_cp,
				      execaddr, length, attr)) != NULL)
    return __ul_seterr (err, EIO);

  /* Does the file have a filetype (at this point we aren't even sure that
     the file exists but that's not a problem, see next 'if' on objtype_cp == 0
     contents further on) ?  */
  int aftype;
  if ((loadaddr_cp & 0xfff00000U) == 0xfff00000U)
    aftype = (loadaddr_cp >> 8) & 0xfff;
  else
    aftype = __RISCOSIFY_FILETYPE_NOTFOUND;

  if (objtype)
    *objtype = objtype_cp;

  if (ftype)
    *ftype = aftype;

  if (loadaddr)
    *loadaddr = loadaddr_cp;

  /* Fail if file doesn't exist or (if specified) filetype is different.  */
  if (objtype_cp == 0
      || ((objtype_cp == 1
	   || (objtype_cp == 3 && __get_feature_imagefs_is_file ()))
          && sftype != __RISCOSIFY_FILETYPE_NOTFOUND && sftype != aftype))
    return __set_errno (ENOENT);

  return 0;
}

int
__object_get_attrs (const char *ux_file, char *buffer, size_t buf_len,
		    int *objtype, int *ftype, int *loadaddr, int *execaddr,
		    int *length, int *attr)
{
  return get_attrs (ux_file, buffer, buf_len,
		    objtype, ftype, loadaddr, execaddr, length, attr, 1);
}

int
__object_get_lattrs (const char *ux_file, char *buffer, size_t buf_len,
		     int *objtype, int *ftype, int *loadaddr, int *execaddr,
		     int *length, int *attr)
{
  return get_attrs (ux_file, buffer, buf_len,
		    objtype, ftype, loadaddr, execaddr, length, attr, 0);
}

/* Sets an objects filetype and attributes (either can be set to
   __ATTR_NOTSPECIFIED to leave unchanged).
   Returns nonzero and sets errno on error.  */
int
__object_set_attrs (const char *ux_file, char *buffer, size_t buf_len,
		    int ftype, int attr)
{
  if (ux_file == NULL)
    return __set_errno (EINVAL);

  int sftype;
  if (!__riscosify_std (ux_file, 0, buffer, buf_len, &sftype))
    return __set_errno (ENAMETOOLONG);

#if __UNIXLIB_SYMLINKS
  char *target;
  if ((target = malloc (buf_len)) == NULL)
    return -1;

  if (__resolve_symlinks (buffer, target, buf_len, 0) != 0)
    {
      free (target);
      return -1;
    }

  buffer = target;
#endif

  /* Set catalogue information.  */
  if (ftype != __ATTR_NOTSPECIFIED)
    {
      const _kernel_oserror *err;
      if ((err = SWI_OS_File_WriteCatInfoFileType (buffer, ftype)) != NULL)
	{
#if __UNIXLIB_SYMLINKS
	  free (target);
#endif
	  return __ul_seterr (err, EIO);
	}
    }

  if (attr != __ATTR_NOTSPECIFIED)
    {
      const _kernel_oserror *err;
      if ((err = SWI_OS_File_WriteCatInfoAttr (buffer, attr)) != NULL)
	{
#if __UNIXLIB_SYMLINKS
	  free (target);
#endif
	  return __ul_seterr (err, EIO);
	}
    }

#if __UNIXLIB_SYMLINKS
  free (target);
#endif

  return 0;
}
