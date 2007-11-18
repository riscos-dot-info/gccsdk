/* UnixLib stat()/stat64() implementation.
   Copyright (c) 2000-2007 UnixLib Developers.  */

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <unixlib/dev.h>
#include <unixlib/os.h>
#include <sys/stat.h>

#include <unixlib/local.h>
#include <internal/swiparams.h>
#include <unixlib/unix.h>
#include <pthread.h>

/* #define DEBUG */

int
stat (const char *filename, struct stat *buf)
{
  PTHREAD_UNSAFE

#ifdef DEBUG
  debug_printf ("stat(file=%s)\n", filename);
#endif

  /* Perform a special check for devices.  */
  buf->st_dev = __getdevtype (filename, __get_riscosify_control());

  /* Perform the device specific open operation.  */
  return dev_funcall (buf->st_dev, stat, (filename, buf));
}
