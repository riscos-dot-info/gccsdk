/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/resource/getegid.c,v $
 * $Date: 2001/01/29 15:10:20 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: getegid.c,v 1.2 2001/01/29 15:10:20 admin Exp $";
#endif

#include <unistd.h>
#include <unixlib/unix.h>

__gid_t
getegid (void)
{
  return __u->egid;
}
