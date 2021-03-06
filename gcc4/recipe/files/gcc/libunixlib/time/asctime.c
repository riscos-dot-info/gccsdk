/* asctime ()
 * Written by Nick Burrett on 12 July 1997.
 * Copyright (c) 1997-2010 UnixLib Developers
 */

#include <time.h>

/* Defined by POSIX as not threadsafe */
char *
asctime (const struct tm *brokentime)
{
  return asctime_r (brokentime, NULL);
}
