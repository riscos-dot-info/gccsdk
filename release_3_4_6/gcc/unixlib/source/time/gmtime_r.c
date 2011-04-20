/****************************************************************************
 *
 * $Source$
 * $Date$
 * $Revision$
 * $State$
 * $Author$
 *
 ***************************************************************************/

/* Territory time support, written by Nick Burrett on 12 July 1997.  */

#include <time.h>
#include <kernel.h>
#include <swis.h>

struct tm *
gmtime_r (const time_t *tp, struct tm *resultp)
{
  struct tm *t = __calendar_convert (Territory_ConvertTimeToUTCOrdinals, tp,
				     resultp);

   /* GMT conversion rather than localtime - whether local time is on daylight
      saving is irrelevalant */
   t->tm_gmtoff = 0;
   t->tm_isdst = 0;
   t->tm_zone = "GMT";
   return t;
}