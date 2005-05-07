/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/unix/readv.c,v $
 * $Date: 2003/04/05 09:33:57 $
 * $Revision: 1.4 $
 * $State: Exp $
 * $Author: alex $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: readv.c,v 1.4 2003/04/05 09:33:57 alex Exp $";
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/uio.h>
#include <unixlib/dev.h>
#include <sys/types.h>
#include <unixlib/unix.h>

#include <unixlib/fd.h>
#include <pthread.h>

/* Read data from file descriptor FD, and put the result in the
   buffers described by VECTOR, which is a vector of COUNT `struct iovec's.
   The buffers are filled in the order specified.  */

ssize_t
readv (int fd, const struct iovec *vector, int count)
{
  struct __unixlib_fd *file_desc;
  size_t bytes, bytes_read;
  int i;
  unsigned int device;

  PTHREAD_UNSAFE_CANCELLATION

  if (count <= 0)
    return __set_errno (EINVAL);
  if (BADF (fd))
    return __set_errno (EBADF);

  file_desc = getfd (fd);

  /* Confirm that the file is open for reading.  */
  if ((file_desc->fflag & O_ACCMODE) == O_WRONLY)
    return __set_errno (EBADF);

  device = file_desc->devicehandle->type;

  /* Read each buffer, recording how many bytes were read.  */
  bytes_read = 0;
  for (i = 0; i < count; ++i)
    if (vector[i].iov_len > 0)
      {
	__u->usage.ru_inblock++;
	bytes = dev_funcall (device, read,
			   (file_desc, vector[i].iov_base, vector[i].iov_len));
	/* If we failed on the first read, then return -1, otherwise return
	   the number of bytes we have read.  */
	if (bytes == -1)
	  return bytes_read ? bytes_read : -1;
	bytes_read += bytes;
	/* If we did not read enough bytes, then give up.  */
	if (bytes != vector[i].iov_len)
	  return bytes_read;
      }

  return bytes_read;
}
