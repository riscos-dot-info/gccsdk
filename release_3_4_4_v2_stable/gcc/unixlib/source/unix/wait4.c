/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/unix/wait4.c,v $
 * $Date: 2003/04/05 09:33:57 $
 * $Revision: 1.4 $
 * $State: Exp $
 * $Author: alex $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: wait4.c,v 1.4 2003/04/05 09:33:57 alex Exp $";
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/resource.h>
#include <sys/types.h>
#include <unixlib/unix.h>
#include <sys/wait.h>
#include <pthread.h>

/* If usage is not null, wait4 returns usage figures for the child
   process in *usage (but only if the child has terminated and not
   if it has stopped).  */

/* wait, wait3 and wait4 are all derivatives of the function, waitpid.
   waitpid has been implemented through wait4 for practical reasons.

   The waitpid function is used to request status information from
   a child process whose process ID is pid.  Normally, the calling
   process is suspended until the child process makes status information
   available by terminating.

   Error conditions:

   EINTR: The function was interrupted by delivery of a signal to the
   calling process.

   ECHILD: There are no child processes to wait for, or the specified pid
   is not a child of the calling process.

   EINVAL: An invalid value was provided for the options argument.

   Symbolic constants (through pid):

   WAIT_ANY: Value is -1 and specifies that waitpid should return status
   information about any child process.

   WAIT_MYPGRP: Value is 0 and specifies that waitpid should return
   status information about any child process in the same process group
   as the calling process.

   Symbolic constants (passed through options):

   WNOHANG: waitpid should return immediately instead of waiting, if
   there is no child process ready to be noticed.

   WUNTRACED: waitpid should report the status of any child processes
   that have been stopped as well as those that have terminated.  */

/* Make conversion of the process exit status easy done.  */

static int
wait_convert_status (const struct __process *status)
{
  if (status->signal_exit)
    return __W_EXITCODE (status->return_code, status->signal);
  if (status->core_dump)
    return __WCOREFLAG;
  if (status->stopped)
    return __W_STOPCODE (status->signal);

  return __W_EXITCODE (status->return_code, 0);
}

/* Make the process of comparing process IDs fairly easy.  */
static int
wait_type (pid_t pid, struct __sul_process *process)
{
  /* Match any process.  */
  if (pid == WAIT_ANY)
    return 1;
  /* Match any process in the current process's group.  */
  if (pid == WAIT_MYPGRP)
    if (process->gid == __proc->pgrp)
      return 1;
  /* Match any process whose process group is the abs(pid).  */
  if (pid < -1)
    if (process->gid == abs (pid))
      return 1;
  /* Match a specific process id.  */
  if (process->pid == pid)
    return 1;

  /* No match found.  */
  return 0;
}


pid_t
wait4 (pid_t pid, int *status, int options, struct rusage *usage)
{
  struct __sul_process *process;
  struct __sul_process *lastprocess;

  PTHREAD_UNSAFE

  if ((options & ~(WNOHANG | WUNTRACED)) != 0)
    return (pid_t) __set_errno (EINVAL);

  while (__proc->children)
    {
      process = __proc->children;
      lastprocess = NULL;

      while (process)
        {
          if (wait_type (pid, process))
            if (process->status.zombie
                || ((options & WUNTRACED) && process->status.stopped && !process->status.reported))
              {
                /* If the untraced bit is set, we will match against
                   stopped children; otherwise we won't.  */

                process->status.reported = 1;

                /* Copy the resource usage details (if usage is not null).  */
                if (usage && process->pid == pid)
                  memset(usage, 0, sizeof (struct rusage));
                if (status)
                  {
                    *status = wait_convert_status (&(process->status));
#ifdef DEBUG
                    printf ("wait4: process = %d, status = %x\n", process, *status);
#endif
                  }

                pid = process->pid;

                if (process->status.zombie)
                  {
                    /* Remove the child from the children list */
                    if (lastprocess)
                      lastprocess->next_child = process->next_child;
                    else
                      __proc->children = process->next_child;

                    __free_process (process);
                  }

                return pid;
              }

            lastprocess = process;
            process = process->next_child;
        }


      /* If the no hang bit is set and we have found no dead
         children, return 0.  */
      if ((options & WNOHANG))
        {
          (void)__set_errno (ECHILD);
          return (pid_t) 0;
        }

      pthread_yield ();
    }

  return (pid_t) -1;
}