/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/clib/sys/netdb.h,v $
 * $Date: 2001/01/29 15:10:19 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifndef __SYS_NETDB_H
#define __SYS_NETDB_H

#ifndef __UNIXLIB_FEATURES_H
#include <unixlib/features.h>
#endif

#ifndef __NETDB_H
#include <netdb.h>
#endif

__BEGIN_DECLS

extern struct hostent *_gethostbyname (const char *name);
extern struct hostent *_gethostbyaddr (const char *addr, int len, int type);
extern int _sclose (int sd);
extern int _sread (int sd, void *data, int nbyte);
extern int _swrite (int sd, const void *data, int nbyte);
extern int _sioctl (int sd, int request, void *arg);

__END_DECLS

#endif