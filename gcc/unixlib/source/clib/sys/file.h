/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/unixlib/source/clib/sys/h/file,v $
 * $Date: 1997/10/09 19:59:54 $
 * $Revision: 1.4 $
 * $State: Exp $
 * $Author: unixlib $
 *
 ***************************************************************************/

#ifndef __SYS_FILE_H
#define __SYS_FILE_H 1

#ifndef __FCNTL_H
#include <fcntl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Alternate names for values for the whence argument to lseek.
   These are the same as SEEK_SET, SEEK_CUR and SEEK_END respectively.  */
#define L_SET 0
#define L_INCR 1
#define L_XTND 2

/* Operations for the flock call.  */

/* Shared lock.  */
#define LOCK_SH 1
/* Exclusive lock.  */
#define LOCK_EX 2
/* Unlock.  */
#define LOCK_UN 8

/* Don't block when locking.  */
#define LOCK_NB 4

/* Apply or remove an advisory lock, according to operation on the
   file fd refers to.  */
extern int flock (int fd, int operation);

#ifdef __cplusplus
}
#endif

#endif
