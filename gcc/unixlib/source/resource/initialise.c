/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/resource/initialise.c,v $
 * $Date: 2003/04/05 12:16:34 $
 * $Revision: 1.4 $
 * $State: Exp $
 * $Author: alex $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: initialise.c,v 1.4 2003/04/05 12:16:34 alex Exp $";
#endif

#include <sys/resource.h>
#include <unixlib/os.h>
#include <unixlib/unix.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <swis.h>
#include <sys/types.h>

/* Initialise the resource limits to calculated initial values.
   Best guesses are taken where information is hard to obtain. :-(  */

void
__resource_initialise (struct proc *p)
{
  int regs[10];
  int max_wimpslot;

  /* The maximum amount of cpu time the process can use.  */
  p->limit[RLIMIT_CPU].rlim_cur = RLIM_INFINITY;
  p->limit[RLIMIT_CPU].rlim_max = RLIM_INFINITY;

  /* The maximum size of a file that the process can create.  */
  p->limit[RLIMIT_FSIZE].rlim_cur = RLIM_INFINITY;
  p->limit[RLIMIT_FSIZE].rlim_max = RLIM_INFINITY;

  /* The maximum size core file that this process can create.  */
  p->limit[RLIMIT_CORE].rlim_cur = RLIM_INFINITY;
  p->limit[RLIMIT_CORE].rlim_max = RLIM_INFINITY;

  regs[0] = -1;
  if (__os_swi (OS_ReadDynamicArea, regs))
    /* If the call failed then it's probably because we are on
       RISC OS < 3.5 so we know it can't be more than 16MB */
    max_wimpslot = 16*1024*1024;
  else
    max_wimpslot = regs[2];

  /* The maximum size of data memory for the process.

     For dynamic areas the limit is current available memory size or
     the limit imposed by any virtual memory system such as Virtualise.  */

  if (__dynamic_num == -1)	/* No dynamic area */
    p->limit[RLIMIT_DATA].rlim_max = max_wimpslot;
  else
    {
      regs[0] = __dynamic_num;
      if (__os_swi (OS_ReadDynamicArea, regs))
	p->limit[RLIMIT_DATA].rlim_max = (u_char *) __unixlib_break - (u_char *) __image_rw_lomem;
      else
	p->limit[RLIMIT_DATA].rlim_max = regs[2];
    }
  p->limit[RLIMIT_DATA].rlim_cur = p->limit[RLIMIT_DATA].rlim_max;


  /* The maximum stack size for the process. As the stack is extensible then
     it can grow up to the maximum wimpslot size */
  p->limit[RLIMIT_STACK].rlim_max = max_wimpslot;
  p->limit[RLIMIT_STACK].rlim_cur = p->limit[RLIMIT_STACK].rlim_max;


  /* Maximum amount of physical memory that this process should get.
     This process should get as much memory as possible.

     Again, for RISC OS 3.5+ dynamic areas and without, both cases
     should be treated differently.

     I think that maximum physical memory is the area from __image_ro_base to
     __image_rw_himem (no dynamic area). Also included is from __image_rw_lomem to
     __unixlib_break and beyond for dynamic areas.  */
  if (__dynamic_num == -1)	/* No dynamic area */
    {
      p->limit[RLIMIT_RSS].rlim_max = max_wimpslot;
      p->limit[RLIMIT_RSS].rlim_cur = p->limit[RLIMIT_RSS].rlim_max;
    }
  else
    {
      /* Area 6 is the free pool for RISC OS 3.5+ and setting bit 7 gets
         the maximum size.  */
      regs[0] = 6 + 128;
      if (__os_swi (OS_ReadDynamicArea, regs))
	p->limit[RLIMIT_RSS].rlim_max += (u_char *) __unixlib_break - (u_char *) __image_rw_lomem;
      else
	/* rlim_max is all of physical memory ?  */
	p->limit[RLIMIT_RSS].rlim_max = regs[2];
      /* rlim_cur is available free memory + __image_ro_base to __image_rw_himem.  */
      p->limit[RLIMIT_RSS].rlim_cur += regs[1];
    }


  /* The maximum amount of memory that can be locked into physical
     memory (so it will never be paged out).  Virtual Memory on
     RISC OS (when supported) does not really implement memory locks.
     Virtual Memory isn't really supported on RISC OS anyway.  */
  p->limit[RLIMIT_MEMLOCK].rlim_cur = 0;
  p->limit[RLIMIT_MEMLOCK].rlim_max = 0;

  /* The maximum number of processes that can be created with the
     same user ID.  For RISC OS this is zero.  */
  p->limit[RLIMIT_NPROC].rlim_cur = CHILD_MAX;
  p->limit[RLIMIT_NPROC].rlim_max = CHILD_MAX;

  /* The maximum number of files that the process can open.  */
  p->limit[RLIMIT_NOFILE].rlim_cur = FOPEN_MAX;
  p->limit[RLIMIT_NOFILE].rlim_max = FOPEN_MAX;
}
