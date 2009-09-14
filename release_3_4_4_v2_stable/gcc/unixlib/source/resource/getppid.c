/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/resource/getppid.c,v $
 * $Date: 2002/02/14 15:56:36 $
 * $Revision: 1.3 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: getppid.c,v 1.3 2002/02/14 15:56:36 admin Exp $";
#endif

#include <unistd.h>
#include <unixlib/unix.h>

__pid_t
getppid (void)
{
  return __proc->ppid;
}