/*
 * Copyright (c) 2000-2013 UnixLib Developers
 */

#ifndef __INTERNAL_SWIPARAMS_H
#define __INTERNAL_SWIPARAMS_H

/* Limited set of SWI parameters needed in UnixLib itself */

#define OSFIND_OPEN_OPENIN            (4<<4)
#define OSFIND_OPEN_OPENOUT           (8<<4)
#define OSFIND_OPEN_OPENUP            (12<<4)

#define OSFIND_OPEN_PATH              1
#define OSFIND_OPEN_PATHVAR           2
#define OSFIND_OPEN_NOPATH	      3
#define OSFIND_OPEN_ERRORIFABSENT     8
#define OSFIND_OPEN_ERRORIFDIR	      4

#define OSFIND_OPENIN	   (OSFIND_OPEN_OPENIN | OSFIND_OPEN_NOPATH)
#define OSFIND_OPENOUT	   (OSFIND_OPEN_OPENOUT | OSFIND_OPEN_NOPATH)
#define OSFIND_OPENUP	   (OSFIND_OPEN_OPENUP | OSFIND_OPEN_NOPATH)

#define MMM_TYPE_RISCOS               0
#define MMM_TYPE_RISCOS_STRING        1
#define MMM_TYPE_MIME                 2
#define MMM_TYPE_DOT_EXTN             3

#define ARMEABISUPPORT_STACKOP_ALLOC		0
#define ARMEABISUPPORT_STACKOP_FREE		1
#define ARMEABISUPPORT_STACKOP_GET_STACK	2
#define ARMEABISUPPORT_STACKOP_GET_BOUNDS	3
#define ARMEABISUPPORT_STACKOP_GET_SIZE		4

#endif
