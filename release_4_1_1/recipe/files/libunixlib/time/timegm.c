/* timegm ()
 * POSIX equivalent function - take from timegm man page under Linux
 */

#include <time.h>
#include <stdlib.h>

time_t
timegm (struct tm *tm)
{
  time_t ret;
  char *tz;

  tz = getenv("TZ");
  setenv("TZ", "", 1);
  tzset();
  ret = mktime(tm);
  if (tz)
    setenv("TZ", tz, 1);
  else
    unsetenv("TZ");
  tzset();
  return ret;
}
