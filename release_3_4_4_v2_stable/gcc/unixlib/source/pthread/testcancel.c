/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/pthread/testcancel.c,v $
 * $Date: 2003/04/28 12:07:02 $
 * $Revision: 1.2 $
 * $State: Exp $
 * $Author: alex $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: testcancel.c,v 1.2 2003/04/28 12:07:02 alex Exp $";
#endif

/* pthread_testcancel
   Separate from cancel.c to avoid pulling in other cancel functions
   for a non threaded program */

/* Written by Alex Waugh */


#include <unixlib/os.h>
#include <pthread.h>

/* Introduce a cancellation point into the current thread */
void
pthread_testcancel (void)
{
  if (__pthread_running_thread->cancelpending && __pthread_worksemaphore == 0)
    {
      if (__pthread_running_thread->cancelstate == PTHREAD_CANCEL_ENABLE
          && __pthread_running_thread->state == STATE_RUNNING)
        {
#ifdef PTHREAD_DEBUG
          __os_print("-- pthread_testcancel: thread cancelled\r\n");
#endif
          pthread_exit(PTHREAD_CANCELED);
        }
    }

#ifdef PTHREAD_DEBUG
  __os_print("-- pthread_testcancel: thread not cancelled\r\n");
#endif
}
