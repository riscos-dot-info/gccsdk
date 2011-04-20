/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/time/stdtime.c,v $
 * $Date: 2001/08/08 08:45:06 $
 * $Revision: 1.2.2.1 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: stdtime.c,v 1.2.2.1 2001/08/08 08:45:06 admin Exp $";
#endif

/* Territory time support, written by Nick Burrett on 13 July 1997.  */

#include <time.h>
#include <locale.h>
#include <unixlib/os.h>
#include <swis.h>
#include <unixlib/local.h>

/* Common function for ctime() and asctime().  */
char *
__standard_time (const char *riscos_time)
{
  static char result[64];
  char *p;
  int regs[10];

  /* Standard time format is: "Tue May 21 13:46:22 1992\n"  */

  regs[0] = __locale_territory[LC_TIME];
  regs[1] = (int)riscos_time;
  regs[2] = (int)result;
  regs[3] = sizeof (result);
  regs[4] = (int)"%W3 %M3 %DY %24:%MI:%SE %CE%YR";
  __os_swi (Territory_ConvertDateAndTime, regs);
  p = (char *)regs[1];
  *p++ = '\n';
  *p++ = '\0';

  return result;
}