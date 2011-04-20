/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/unix/fstat.c,v $
 * $Date: 2003/04/05 09:33:56 $
 * $Revision: 1.6 $
 * $State: Exp $
 * $Author: alex $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: fstat.c,v 1.6 2003/04/05 09:33:56 alex Exp $";
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>

#include <unixlib/dev.h>
#include <unixlib/os.h>
#include <sys/stat.h>
#include <unixlib/unix.h>

#include <unixlib/local.h>
#include <unixlib/swiparams.h>
#include <pthread.h>

int
fstat (int fd, struct stat *buf)
{
  struct __unixlib_fd *file_desc;

  PTHREAD_UNSAFE

  if (buf == NULL)
    return __set_errno (EINVAL);

  if (BADF (fd))
    return __set_errno (EBADF);

  file_desc = &__u->fd[fd];

  buf->st_dev = file_desc->device;

  /* Perform the device specific open operation.  */
  return __funcall ((*(__dev[file_desc->device].fstat)),(fd, buf));
}