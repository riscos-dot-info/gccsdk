/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/unixlib/source/netlib/c/accept,v $
 * $Date: 1997/10/09 16:38:50 $
 * $Revision: 1.6 $
 * $State: Exp $
 * $Author: unixlib $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: accept,v 1.6 1997/10/09 16:38:50 unixlib Exp $";
#endif

#include <errno.h>

#include <sys/unix.h>
#include <sys/socket.h>
#include <sys/dev.h>
#include <fcntl.h>
#include <unixlib/fd.h>
#include <unixlib/local.h>
#ifdef DEBUG
#include <stdio.h>
#endif

int
accept (int s, struct sockaddr *name, int *namelen)
{
  struct __unixlib_fd *file_desc;
  int nfd;
  int nsd;

#ifdef DEBUG
  fprintf(stderr, "U! accept: fd=%d, realsocket=%d\n",s,(int)__u->fd[s].handle);
#endif
  if (__socket_valid (s) == -1)
    return -1;

  if ((nsd = _accept ((int)__u->fd[s].handle, name, namelen)) < 0)
    return (-1);

  if ((nfd = __alloc_file_descriptor ()) < 0)
    return (-1);

#ifdef DEBUG
  fprintf(stderr, "U! accept: nsd=%d nfd=%d\n",nsd,nfd);
#endif

  file_desc = &__u->fd[nfd];
  file_desc->fflag = O_RDWR | O_BINARY;
  file_desc->dflag = 0;
  file_desc->handle = (void *)nsd;

  file_desc->device = DEV_SOCKET;

  file_desc->__magic = _FDMAGIC;
  file_desc->pid = __u->pid;

  return nfd;
}
