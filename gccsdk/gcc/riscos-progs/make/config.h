/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
#undef _ALL_SOURCE
#endif

#define LIBDIR "unix:"
#define INCLUDEDIR "unix:"
#define NO_ARCHIVES 1
#define GCC_IS_NATIVE 1

/* Define if using alloca.c.  */
#undef C_ALLOCA

/* Define to empty if the keyword does not work.  */
#undef const

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
#undef CRAY_STACKSEG_END

/* Define for DGUX with <sys/dg_sys_info.h>.  */
#undef DGUX

/* Define if the `getloadavg' function needs to be run setuid or setgid.  */
#undef GETLOADAVG_PRIVILEGED

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #define gid_t int */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

#define HAVE_GLOB_H 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#define HAVE_ALLOCA_H 1

/* Define if you don't have vprintf but do have _doprnt.  */
#undef HAVE_DOPRNT

/* Define if your system has a working fnmatch function.  */
#define HAVE_FNMATCH

/* Define if your system has its own `getloadavg' function.  */
#undef HAVE_GETLOADAVG

/* Define if you have the getmntent function.  */
#undef HAVE_GETMNTENT

/* Define if the `long double' type works.  */
#define HAVE_LONG_DOUBLE 1

/* Define if you support file names longer than 14 characters.  */
#undef HAVE_LONG_FILE_NAMES

/* Define if you have a working `mmap' system call.  */
#define HAVE_MMAP 1

/* Define if system calls automatically restart after interruption
   by a signal.  */
#undef HAVE_RESTARTABLE_SYSCALLS

/* Define if your struct stat has st_blksize.  */
#define HAVE_ST_BLKSIZE 1

/* Define if your struct stat has st_blocks.  */
#undef HAVE_ST_BLOCKS

/* Define if you have the strcoll function and it is properly defined.  */
#undef HAVE_STRCOLL

/* Define if your struct stat has st_rdev.  */
#define HAVE_ST_RDEV 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if your struct tm has tm_zone.  */
#define HAVE_TM_ZONE 1

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#undef HAVE_TZNAME

/* Define if you have <unistd.h>.  */
#define HAVE_UNISTD_H 1

/* Define if utime(file, NULL) sets file's timestamp to the present.  */
#define HAVE_UTIME_NULL 1

/* Define if you have <vfork.h>.  */
#undef HAVE_VFORK_H

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define if you have the wait3 system call.  */
#define HAVE_WAIT3 1

/* Define if on MINIX.  */
#undef _MINIX

/* Define if your struct nlist has an n_un member.  */
#undef NLIST_NAME_UNION

/* Define if you have <nlist.h>.  */
#undef NLIST_STRUCT

/* Define if your C compiler doesn't accept -c and -o together.  */
#undef NO_MINUS_C_MINUS_O

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #define pid_t int */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
#undef _POSIX_1_SOURCE

/* Define if you need to in order for stat and other things to work.  */
#undef _POSIX_SOURCE

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if the setvbuf function takes the buffering type as its second
   argument and the buffer pointer as the third, as on System V
   before release 3.  */
#undef SETVBUF_REVERSED

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown
 */
#define STACK_DIRECTION -1

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly.  */
#undef STAT_MACROS_BROKEN

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define on System V Release 4.  */
#undef SVR4

/* Define if `sys_siglist' is declared by <signal.h>.  */
#define SYS_SIGLIST_DECLARED 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #define uid_t int */

/* Define for Encore UMAX.  */
#undef UMAX

/* Define for Encore UMAX 4.3 that has <inq_status/cpustats.h>
   instead of <sys/cpustats.h>.  */
#undef UMAX4_3

/* Define vfork as fork if vfork does not work.  */
#undef vfork

/* Name of this package (needed by automake) */
#define PACKAGE "GNU make"

/* Version of this package (needed by automake) */
#define VERSION "3.77"

/* Define to the name of the SCCS `get' command.  */
#define SCCS_GET "sccsget"

/* Define this if the SCCS `get' command understands the `-G<file>' option.  */
#undef SCCS_GET_MINUS_G

/* Define this if the C library defines the variable `sys_siglist'.  */
#define HAVE_SYS_SIGLIST 1

/* Define this if the C library defines the variable `_sys_siglist'.  */
#undef HAVE__SYS_SIGLIST

/* Define this if you have the `union wait' type in <sys/wait.h>.  */
#undef HAVE_UNION_WAIT

/* Define this if the POSIX.1 call `sysconf (_SC_OPEN_MAX)' works properly.  */
#define HAVE_SYSCONF_OPEN_MAX 1

/* Define if you have the dup2 function.  */
#define HAVE_DUP2 1

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the getdtablesize function.  */
#define HAVE_GETDTABLESIZE 1

/* Define if you have the getgroups function.  */
#undef HAVE_GETGROUPS

/* Define if you have the mktemp function.  */
#define HAVE_MKTEMP 1

/* Define if you have the psignal function.  */
#define HAVE_PSIGNAL 1

/* Define if you have the setegid function.  */
#define HAVE_SETEGID 1

/* Define if you have the seteuid function.  */
#define HAVE_SETEUID 1

/* Define if you have the setlinebuf function.  */
#define HAVE_SETLINEBUF 1

/* Define if you have the setregid function.  */
#undef HAVE_SETREGID

/* Define if you have the setreuid function.  */
#undef HAVE_SETREUID

/* Define if you have the sigsetmask function.  */
#define HAVE_SIGSETMASK 1

/* Define if you have the socket function.  */
#undef HAVE_SOCKET

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strsignal function.  */
#define HAVE_STRSIGNAL 1

/* Define if you have the wait3 function.  */
#define HAVE_WAIT3 1

/* Define if you have the waitpid function.  */
#define HAVE_WAITPID 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <mach/mach.h> header file.  */
#undef HAVE_MACH_MACH_H

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <ndir.h> header file.  */
#undef HAVE_NDIR_H

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/dir.h> header file.  */
#define HAVE_SYS_DIR_H 1

/* Define if you have the <sys/ndir.h> header file.  */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/timeb.h> header file.  */
#define HAVE_SYS_TIMEB_H 1

/* Define if you have the <sys/wait.h> header file.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the dgc library (-ldgc).  */
#undef HAVE_LIBDGC

/* Define if you have the sun library (-lsun).  */
#undef HAVE_LIBSUN
