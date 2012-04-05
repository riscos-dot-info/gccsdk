/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/unix/ispipe.c,v $
 * $Date: 2001/01/29 15:10:22 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: ispipe.c,v 1.2 2001/01/29 15:10:22 admin Exp $";
#endif

#include <errno.h>
#include <unistd.h>
#include <unixlib/dev.h>
#include <unixlib/unix.h>
#include <unixlib/fd.h>

/* Return 1 if fd is a pipe.  */
int
ispipe (int fd)
{
  if (BADF (fd))
    return __set_errno (EBADF);

  return __u->fd[fd].device == DEV_PIPE;
}