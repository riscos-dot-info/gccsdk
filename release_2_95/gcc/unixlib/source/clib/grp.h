/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/clib/Attic/grp.h,v $
 * $Date: 2001/09/14 14:01:17 $
 * $Revision: 1.1.2.2 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifndef __GRP_H
#define __GRP_H 1

#ifndef __UNIXLIB_FEATURES_H
#include <unixlib/features.h>
#endif

#ifndef __UNIXLIB_TYPES_H
#include <unixlib/types.h>
#endif

#define __need_size_t
#include <stddef.h>

#if !defined __gid_t_defined
typedef __gid_t gid_t;
#define __gid_t_defined
#endif

__BEGIN_DECLS

#define __need_FILE
#include <stdio.h>

struct group
{
  char *gr_name; /* Group name.  */
  char *gr_passwd; /* Group password.  */
  __gid_t gr_gid; /* Group ID.  */
  char **gr_mem; /* Group members.  */
};

/* Rewind the group-file stream.  */
extern void setgrent (void);

/* Close the group-file stream.  */
extern void endgrent (void);

/* Read an entry from the group-file stream, opening it if necessary.  */
extern struct group *getgrent (void);

/* Read an entry from the password-file stream, opening it if
   necessary (re-entrant version).  */
extern int getgrent_r (struct group *__result_buf, char *__buffer,
		       size_t __buflen, struct group **__result);

/* Read an entry from stream.  */
extern struct group *fgetgrent (FILE *__stream);

/* Read an entry from stream (re-entrant version).  */
extern int fgetgrent_r (FILE *__stream, struct group *__result_buf,
			char *__buffer, size_t __buflen,
			struct group **__result);

/* Search for an entry with a matching group ID.  */
extern struct group *getgrgid (gid_t __gid);

/* Search for an entry with a matching group ID (re-entrant version).  */
extern int getgrgid_r (gid_t __gid, struct group *__resbuf, char *__buffer,
		       size_t __buflen, struct group **__result);

/* Search for an entry with a matching group name.  */
extern struct group *getgrnam (const char *__name);

/* Search for an entry with a matching group name (re-entrant version).  */
extern int getgrnam_r (const char *__name, struct group *__result_buf,
		       char *__buffer, size_t __buflen,
		       struct group **__result);

/* Get a list of all supplementary groups this user is in */
extern int getgroups (int __gidsetlen, gid_t *__gidset);

/* Set the list of supplementary groups this user is in */
extern int setgroups (int __ngroups, const gid_t *__gidset);

/* Build a list of all groups the user is in, then call setgroups on the list*/
extern int initgroups (const char *__name, gid_t __basegid);

#ifdef __UNIXLIB_INTERNALS
/* UnixLib group implementation function.  */
extern struct group *__grpread (FILE *, struct group *);
#endif

__END_DECLS

#endif