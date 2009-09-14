/* UnixLib fopen()/fdopen() implementation.
   Copyright 2001-2006 UnixLib Developers.  */

/* #define DEBUG */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unixlib/unix.h>
#include <pthread.h>

#ifdef DEBUG
#include <unixlib/os.h>
#endif

__STDIOLIB__

FILE *
fopen (const char *filename, const char *mode)
{
  FILE *stream;
  __io_mode m;
  __mode_t file_mode;
  int fd;

  PTHREAD_UNSAFE

  if (filename == NULL || mode == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

#ifdef DEBUG
  __os_print ("fopen(fname="); __os_print (filename);
  __os_print (", mode="); __os_print (mode); __os_print ("): ");
#endif

  m = __getmode (mode);
  if (! m.__bits.__read && ! m.__bits.__write)
    {
      (void) __set_errno (EINVAL);
      return NULL;
    }

  /* Create new file stream.  */
  stream = __newstream ();
  if (stream == NULL)
    return NULL;

  stream->__mode = m;

  if (m.__bits.__read && m.__bits.__write)
    file_mode = O_RDWR;
  else
    file_mode = m.__bits.__read ? O_RDONLY : O_WRONLY;

  if (m.__bits.__append)
    file_mode |= O_APPEND;
  if (m.__bits.__truncate)
    file_mode |= O_TRUNC;

  if (m.__bits.__create)
    fd = open (filename, file_mode | O_CREAT,
	       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  else
    fd = open (filename, file_mode);

#ifdef DEBUG
  __os_print ("fd="); __os_prdec (fd); __os_nl ();
#endif

  if (fd < 0)
    return NULL;

  return __stream_init (fd, stream);
}

FILE *
fdopen (int fd, const char *mode)
{
  FILE *stream;
  __io_mode m;
  int dflags;

#ifdef DEBUG
  __os_print ("fdopen (fd="); __os_prdec (fd);
  __os_print (",mode="); __os_print (mode); __os_print (")\r\n");
#endif
  m = __getmode (mode);
  if (! m.__bits.__read && ! m.__bits.__write)
    return NULL;

#ifdef DEBUG
  __os_print ("fdopen: here1\r\n");
#endif
  /* Verify the FD is valid and allows the access 'mode' specifies.  */
  dflags = getfd (fd)->fflag; /* dflags = fcntl (fd, F_GETFL); */
  if (dflags == -1)
    return NULL;

#ifdef DEBUG
  __os_print ("fdopen: here2\r\n");
#endif

  /* Check the access mode.  */
  if ((dflags & O_ACCMODE) == O_RDONLY && ! m.__bits.__read)
    {
      errno = EBADF;
      return NULL;
    }
  if ((dflags & O_ACCMODE) == O_WRONLY && ! m.__bits.__write)
    {
      errno = EBADF;
      return NULL;
    }

  stream = __newstream ();
  if (stream == NULL)
    return NULL;

#ifdef DEBUG
  __os_print ("fdopen: here3\r\n");
#endif

  stream->__mode = m;

  return __stream_init (fd, stream);
}