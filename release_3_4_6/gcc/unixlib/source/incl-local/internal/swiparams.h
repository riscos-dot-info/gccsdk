/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/clib/unixlib/swiparams.h,v $
 * $Date: 2004/12/11 14:18:57 $
 * $Revision: 1.6 $
 * $State: Exp $
 * $Author: joty $
 *
 ***************************************************************************/

#ifndef __INTERNAL_SWIPARAMS_H
#define __INTERNAL_SWIPARAMS_H

#ifdef __UNIXLIB_INTERNALS

/* Limited set of SWI parameters needed in UnixLib itself */

#define OSFILE_READCATINFO_FILEPATH	5
#define OSFILE_READCATINFO_PATH		13
#define OSFILE_READCATINFO_PATHVAR	15
#define OSFILE_READCATINFO_NOPATH	17

#define OSFILE_WRITECATINFO_ALL		1
#define OSFILE_WRITECATINFO_ATTR	4
#define OSFILE_WRITECATINFO_FILETYPE	18

#define OSFILE_DELETENAMEDOBJECT	6
#define OSFILE_CREATEDIRECTORY		8
#define OSFILE_CREATEEMPTYFILE_FILETYPE	11

#define OSFIND_OPEN_OPENIN            (4<<4)
#define OSFIND_OPEN_OPENOUT           (8<<4)
#define OSFIND_OPEN_OPENUP            (12<<4)

#define OSFIND_OPEN_PATH              1
#define OSFIND_OPEN_PATHVAR           2
#define OSFIND_OPEN_NOPATH	      3
#define OSFIND_OPEN_ERRORIFABSENT     8
#define OSFIND_OPEN_ERRORIFDIR	      4

#if __UNIXLIB_USEFILEPATH > 0
#define OSFILE_READCATINFO (OSFILE_READCATINFO_FILEPATH)
#define OSFILE_OPENIN	   (OSFIND_OPEN_OPENIN | OSFIND_OPEN_NOPATH)
#define OSFILE_OPENOUT	   (OSFIND_OPEN_OPENOUT | OSFIND_OPEN_NOPATH)
#define OSFILE_OPENUP	   (OSFIND_OPEN_OPENUP | OSFIND_OPEN_NOPATH)
#else
#define OSFILE_READCATINFO (OSFILE_READCATINFO_NOPATH)
#define OSFILE_OPENIN	   (OSFIND_OPEN_OPENIN)
#define OSFILE_OPENOUT	   (OSFIND_OPEN_OPENOUT)
#define OSFILE_OPENUP	   (OSFIND_OPEN_OPENUP)
#endif

#define MMM_TYPE_RISCOS               0
#define MMM_TYPE_RISCOS_STRING        1
#define MMM_TYPE_MIME                 2
#define MMM_TYPE_DOT_EXTN             3

#endif  /* __UNIXLIB_INTERNALS */

#endif