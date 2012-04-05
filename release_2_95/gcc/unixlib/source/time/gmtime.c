/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/time/gmtime.c,v $
 * $Date: 2001/07/23 15:37:49 $
 * $Revision: 1.3 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: gmtime.c,v 1.3 2001/07/23 15:37:49 admin Exp $";
#endif

/* Territory time support, written by Nick Burrett on 12 July 1997.  */

#include <time.h>
#include <kernel.h>
#include <swis.h>

struct tm *
gmtime (const time_t *tp)
{
  struct tm *t = __calendar_convert (Territory_ConvertTimeToUTCOrdinals, tp);

   /* GMT conversion rather than localtime - whether local time is on daylight
      saving is irrelevalant */
   t->tm_gmtoff = 0;
   t->tm_isdst = 0;
   t->tm_zone = "GMT";
   return t;
}