/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/unix/umask.c,v $
 * $Date: 2001/01/29 15:10:22 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: umask.c,v 1.2 2001/01/29 15:10:22 admin Exp $";
#endif

#include <unixlib/types.h>
#include <sys/stat.h>
#include <unixlib/unix.h>

__mode_t
umask (__mode_t cmode)
{
  __mode_t old_umask = __u->umask;

  __u->umask = cmode & 0777;
  return old_umask;
}