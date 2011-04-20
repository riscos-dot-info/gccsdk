/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/resource/setgid.c,v $
 * $Date: 2001/01/29 15:10:20 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: setgid.c,v 1.2 2001/01/29 15:10:20 admin Exp $";
#endif

#include <unistd.h>
#include <unixlib/unix.h>

/* Set the real and effective group ID of the process to gid.  */

int
setgid (__gid_t gid)
{
  if (gid == __u->gid)
    return 0;
  if (gid == __u->egid)
    {
      __u->gid = gid;
      return 0;
    }
  return -1;
}