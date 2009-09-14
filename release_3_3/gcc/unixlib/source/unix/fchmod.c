/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/unix/fchmod.c,v $
 * $Date: 2002/02/14 15:56:38 $
 * $Revision: 1.3 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: fchmod.c,v 1.3 2002/02/14 15:56:38 admin Exp $";
#endif

#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include <unixlib/dev.h>
#include <unixlib/os.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unixlib/unix.h>

#include <unixlib/local.h>
#include <pthread.h>

int fchmod (int fd, mode_t mode)
{
  struct __unixlib_fd *file_desc;
  _kernel_oserror *err;
  int regs[10];
  char name[_POSIX_NAME_MAX];

  PTHREAD_UNSAFE
  
  if (BADF (fd))
    return __set_errno (EBADF);

  /* Get the file name associated with the file descriptor.  */
  file_desc = &__u->fd[fd];

  if (file_desc->device != DEV_RISCOS)
    return __set_errno (EINVAL);

  if (__fd_to_name ((int) file_desc->handle, name, sizeof (name)) == NULL)
    return __set_errno (EBADF);

  /* Set the file access permission bits.  */
  regs[5] = __set_protection (mode);
  err = __os_file (4, name, regs);
  if (err)
    {
      __seterr (err);
      return __set_errno (EPERM);
    }

  return 0;
}