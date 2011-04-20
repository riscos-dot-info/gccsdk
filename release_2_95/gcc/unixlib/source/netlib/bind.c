/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/netlib/bind.c,v $
 * $Date: 2001/09/04 16:32:04 $
 * $Revision: 1.2.2.1 $
 * $State: Exp $
 * $Author: admin $
 *
 * (c) Copyright 1995 Sergio Monesi
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: bind.c,v 1.2.2.1 2001/09/04 16:32:04 admin Exp $";
#endif

#include <unixlib/unix.h>
#include <sys/socket.h>
#include <unixlib/fd.h>
#include <unixlib/local.h>

int
bind (int s, const struct sockaddr *name, socklen_t namelen)
{
  if (__socket_valid (s) == -1)
    return -1;

/*  printf("U! bind: fd=%d, realsocket=%d\n",s,(__u->file+s)->r[0]); */
  return _bind ((int)__u->fd[s].handle, name, namelen);
}