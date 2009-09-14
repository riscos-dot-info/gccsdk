/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/unix/dev.c,v $
 * $Date: 2002/08/18 19:57:19 $
 * $Revision: 1.2.2.9 $
 * $State: Exp $
 * $Author: admin $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: dev.c,v 1.2.2.9 2002/08/18 19:57:19 admin Exp $";
#endif

/* #define DEBUG */
#define _GNU_SOURCE
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <unixlib/unix.h>
#include <sys/param.h>
#include <unixlib/dev.h>
#include <unixlib/os.h>
#include <sys/netdb.h>
#include <sys/stat.h>
#include <swis.h>
#include <sys/select.h>
#include <unixlib/local.h>
#include <unixlib/features.h>
#include <unixlib/swiparams.h>

#define IGNORE(x) {(void) x;}

#ifndef __UNIXLIB_NO_COMMON_DEV
#define __fsioctl	__commonioctl
#define __fsselect	__commonselect

#define __ttylseek	__commonlseek

#define __pipelseek	__commonlseek
#define __pipeioctl	__commonioctl

#define __nulllseek	__commonlseek
#define __nullioctl	__commonioctl
#define __nullselect	__commonselect

#define __socklseek	__commonlseek
#endif

struct dev __dev[NDEV] =
{
  {__fsopen, __fsclose, __fsread, __fswrite,
    __fslseek, __fsioctl, __fsselect },
  {__ttyopen, __ttyclose, __ttyread, __ttywrite,
    __ttylseek, __ttyioctl, __ttyselect },
#if __FEATURE_PIPEDEV
  {__pipeopen, __pipeclose, __piperead, __pipewrite,
    __pipelseek, __pipeioctl, __pipeselect },
#else
  {__nullopen, __nullclose, __nullread, __nullwrite,
    __nulllseek, __nullioctl, __nullselect },
#endif
  {__nullopen, __nullclose, __nullread, __nullwrite,
    __nulllseek, __nullioctl, __nullselect },
#if __FEATURE_SOCKET
  /* Socket select is a special case.  */
  {__sockopen, __sockclose, __sockread, __sockwrite,
    __socklseek, __sockioctl, __sockselect }
#endif
};

#ifndef __UNIXLIB_NO_COMMON_DEV
__off_t
__commonlseek (struct __unixlib_fd *file_desc, __off_t lpos, int whence)
{
  IGNORE (lpos);
  IGNORE (whence);
  IGNORE (file_desc);
  return __set_errno (ESPIPE);
}

int
__commonioctl (struct __unixlib_fd *file_desc, int request, void *arg)
{
  IGNORE (request);
  IGNORE (arg);
  IGNORE (file_desc);
  return __set_errno (EINVAL);
}

int
__commonselect (struct __unixlib_fd *file_desc, int fd,
		__fd_set *read, __fd_set *write, __fd_set *except)
{
  /* Return ready to read, ready to write, no execptional conditions.  */
  IGNORE (file_desc);
  if (read)
    FD_SET(fd, read);
  if (write)
    FD_SET (fd, write);
  if (except)
    FD_CLR (fd, except);

  /* This may not be correct, but it is consistent with Internet 5 select.  */
  return (read ? 1 : 0) + (write ? 1 : 0);
}
#endif

void *
__fsopen (struct __unixlib_fd *file_desc, const char *filename, int mode)
{
  int regs[10], fd, sftype, fflag = file_desc->fflag;
  char file[MAXPATHLEN + 2]; /* Want two chars for ".^" below.  */
  __mode_t access_mode;
  _kernel_oserror *err;
  char *end_of_filename; /* Location useful for CREAT failure tests.  */

  end_of_filename = __riscosify (filename,
				 fflag & (O_CREAT | O_WRONLY | O_RDWR),
				 __get_riscosify_control (), file,
				 sizeof (file) - 2, &sftype);
  if (end_of_filename == NULL)
    return (void *) __set_errno (ENAMETOOLONG);

  /* Get file vital statistics.  */
  if ((err = __os_file (OSFILE_READCATINFO, file, regs)))
    goto os_err;

  if (regs[0])
    {
      /* Some sort of object exists.  */
      if ((fflag & (O_EXCL | O_CREAT)) == (O_EXCL | O_CREAT))
	/* File exists and the user set O_EXCL.  */
	return (void *)  __set_errno (EEXIST);

      /* Check for permission to access the file in the mode we
	 requested.  */
      if (regs[0] == 2 || (! __feature_imagefs_is_file && regs[0] == 3))
	{
	  /* Directory or image file.  Set errno if the user
	     specified write access.  */
	  if ((fflag & O_ACCMODE) == O_WRONLY)
	    return (void *) __set_errno (EISDIR);

	  file_desc->dflag |= FILE_ISDIR;
	  /* For directories, owner read access might be disabled.  */
	  regs[5] |= 1;
	}

      /* Remove creation bits.  */
      fflag &= ~O_CREAT;

      access_mode = __get_protection (regs[5]);
      if (((fflag & O_ACCMODE) == O_RDONLY && !(access_mode & S_IRUSR))
	  || ((fflag & O_ACCMODE) == O_WRONLY && !(access_mode & S_IWUSR))
	  || ((fflag & O_ACCMODE) == O_RDWR
	      && ((access_mode & (S_IWUSR | S_IRUSR)) != (S_IWUSR | S_IRUSR))))
	return (void *) __set_errno (EACCES);
    }
  else
    {
      /* We try to do our best to get at the filing system name
         by canonicalising the filename if possible, otherwise checking
         for pipe: anyway.
         NOTE: This is an ugly hack to make opening non-existent files
	 for reading on PipeFS work.  We should handle PipeFS though
         a proper device ioctl.  */
      char *temp;
      regs[0] = 37;
      regs[1] = (int) filename;
      regs[2] = 0;
      regs[3] = 0;
      regs[4] = 0;
      regs[5] = 0;
      __os_swi (OS_FSControl, regs);
      temp = malloc (1 - regs[5]);
      if (temp)
	{
	  regs[0] = 37;
	  regs[1] = (int) filename;
	  regs[2] = (int) temp;
	  regs[3] = 0;
	  regs[4] = 0;
	  regs[5] = 1 - regs[5];
	  __os_swi (OS_FSControl, regs);
	} else
	  temp = (char *) filename;
      
      if (! (strlen (temp) > 5
	     && tolower (temp[0]) == 'p'
	     && tolower (temp[1]) == 'i'
	     && tolower (temp[2]) == 'p'
	     && tolower (temp[3]) == 'e'
	     && temp[4] == ':'))
        {
          /* If no file exists and O_CREAT was not specified,
	     return ENOENT.  */
          if (!(fflag & O_CREAT))
	    {
	      if (temp)
		free(temp);
	      return (void *) __set_errno (ENOENT);
	    }
        }
      if (temp)
        free(temp);
    }

  if (fflag & O_CREAT)
    {
      /* If the O_CREAT flag has been set, then create an
	 empty file. Strict POSIX says that there is no
	 difference between text and binary files. */
      regs[2] = (sftype == __RISCOSIFY_FILETYPE_NOTFOUND) ? 0xfff : sftype;
      regs[4] = regs[5] = 0;
      if ((err = __os_file (0x0b, file, regs)))
	{
	  /* Could not create.  Cannot assume component of name does not
	     exist, as errors such as "directory full" end up here.  */
	  char *where = file;
	  int leafonly = 1;
	  char ch;

	  while ((ch = *where++))
	    if (ch == '.' || ch == ':')
	      {
		leafonly = 0;
		break;
	      }

	  if (!leafonly)
	    {
	      /* There was a '.' or ':' present, so check for parent
		 directory.  end_of_filename points to the terminating
		 '\0' in the riscosified filename.  If that filename ends
		 ':' (eg "parallel:") append '^' else append '.^'.  Need to
		 check for leafname only, as "!Run.^" gives an error.  */
	      if (end_of_filename[-1] != ':')
		*end_of_filename++ = '.';
	      *end_of_filename = ':';

	      if (!__isdir_raw (file))
		return (void *) __set_errno (ENOENT);
		/* No parent directory - throw Unix error.  */
	    }
	  /* Parent directory exists - implies OS problem creating
	     file.  */
	  goto os_err;
	}
    }

  /* Open the file.  */
  if (fflag & O_TRUNC)
    err = __os_fopen (OSFILE_OPENOUT, file, &fd);
  else
    {
      if ((fflag & O_ACCMODE) == O_RDWR)
	err = __os_fopen (OSFILE_OPENUP, file, &fd);
      else if ((fflag & O_ACCMODE) == O_WRONLY)
	err = __os_fopen (OSFILE_OPENUP, file, &fd);
      else if ((fflag & O_ACCMODE) == O_RDONLY)
	{
	  if (file_desc->dflag & FILE_ISDIR)
	    {
	      DIR *dir;
#ifdef DEBUG
       	      __os_print ("-- open directory (read only): file=");
       	      __os_print (file); __os_print ("\r\n");
#endif

	      dir = opendir (file);
	      return (dir == NULL) ? (void *) -1 : (void *) dir;
	    }
	  else
	    err = __os_fopen (OSFILE_OPENIN, file, &fd);
	}
      else
	return (void *) __set_errno (EINVAL);
    }

  if (err)
    goto os_err;

  /* Now set the protection. This must be done after the file is opened,
     so that the file can be created for writing but set to read only.
     RCS needs to do that.  */
  if (fflag & O_CREAT)
    {
      mode &= ~(__u->umask & 0777);
      regs[5] = __set_protection (mode);

      /* Write the object attributes.  */
      if ((err = __os_file (0x04, file, regs)))
	goto os_err;
    }

  regs[0] = 254;
  regs[1] = (int)fd;
  __os_swi (OS_Args, regs);
  /* Check the 'stream is unallocated' bit.  Probably caused
     by us trying to open a file that is already open.  */
  if (regs[0] & (1 << 11))
    /* Trying again *might* fix the problem.  */
    return (void *) __set_errno (EAGAIN);

  /* Store the filing system number for this file.  */
  file_desc->fs = regs[2] & 0xff;

  return (void *) fd;

os_err:
  __seterr (err);
  return (void *) -1;
}

int
__fsclose (struct __unixlib_fd *file_desc)
{
  char *buffer = NULL;
  _kernel_oserror *err;
  int regs[10];

  if (file_desc->dflag & FILE_ISDIR)
    return closedir ((DIR *) file_desc->handle);

  if (file_desc->fflag & O_UNLINKED)
    buffer = __fd_to_name ((int) file_desc->handle, NULL, 0);

   /* __os_print ("Closing file ");
      __os_prhex ((int)file_desc->handle);
      if( buffer )
      {
        __os_print (": ");
        __os_print (buffer);
      }
      __os_nl();  */

  /* Close file.  */
  err = __os_fclose ((int) file_desc->handle);
  if (! err && buffer)
    err = __os_file (0x06, buffer, regs); /* Delete file.  */
  if (buffer)
    free (buffer);
  return (! err) ? 0 : (__seterr (err), -1);
}

int
__fsread (struct __unixlib_fd *file_desc, void *data, int nbyte)
{
  if (file_desc->dflag & FILE_ISDIR)
    {
      struct dirent entry, *result;
#ifdef DEBUG
      __os_print ("__fsread: read directory entry\r\n");
#endif

      if (readdir_r ((DIR *) file_desc->handle, &entry, &result))
	return -1;
      memcpy (data, entry.d_name, entry.d_namlen);
      return (nbyte - entry.d_namlen);
    }
  else
    {
      _kernel_oserror *err;
      int regs[5];

      if ((err = __os_fread ((int)file_desc->handle, data, nbyte, regs)))
	{
	  __seterr (err);
	  return -1;
	}

      return (nbyte - regs[3]);
    }
}

int
__fswrite (struct __unixlib_fd *file_desc, const void *data, int nbyte)
{
  _kernel_oserror *err;
  int regs[5];

#ifdef DEBUG
  __os_print ("__fswrite("); __os_prdec ((int)file_desc->handle);
  __os_print (", nbyte="); __os_prdec (nbyte); __os_print (")\r\n");
#endif

  if ((err = __os_fwrite ((int)file_desc->handle, data, nbyte, regs)))
    {
      __seterr (err);
      return -1;
    }

  return (nbyte - regs[3]);
}

__off_t
__fslseek (struct __unixlib_fd *file_desc, __off_t lpos, int whence)
{
  _kernel_oserror *err = NULL;
  int regs[3];
  int handle = (int) file_desc->handle;

  if (whence == SEEK_SET)
    err = __os_args (1, handle, (int) lpos, regs);
  else if (whence == SEEK_CUR)
    {
      err = __os_args (0, (int) handle, 0, regs);
      /* For lseek (f, 0, SEEK_CUR) don't need the second SWI, as effectively
	 being called to return file position without changing it
	 Testing lpos == 0 is quick, and lpos == 0 frequent
	 (eg all calls to fopen() that are not "a")  */
      if (! err && lpos)
        err = __os_args (1, (int) handle, regs[2] + (int) lpos, regs);
    }
  else if (whence == SEEK_END)
    {
      err = __os_args (2, (int) handle, 0, regs);
      if (! err)
        err = __os_args (1, (int) handle, regs[2] + (int) lpos, regs);
    }
  else
    return __set_errno (EINVAL);

  return (! err) ? ((__off_t) regs[2]) : (__seterr (err), -1);
}

#ifdef __UNIXLIB_NO_COMMON_DEV
int
__fsioctl (struct __unixlib_fd *file_desc, int request, void *arg)
{
  IGNORE (request);
  IGNORE (arg);
  IGNORE (file_desc);
  return __set_errno (EINVAL);
}


int
__fsselect (struct __unixlib_fd *file_descriptor, int fd,
		__fd_set *read, __fd_set *write, __fd_set *except)
{
  /* Return ready to read, ready to write, no execptional conditions.  */
  IGNORE (file_desc);
  if (read)
    FD_SET (fd, read);
  if (write)
    FD_SET (fd, write);
  if (except)
    FD_CLR (fd, except);

  /* This may not be correct, but it is consistent with Internet 5
     select.  */
  return (read ? 1 : 0) + (write ? 1 : 0);
}
#endif

#if __FEATURE_PIPEDEV
struct pipe *__pipe = NULL;

void *
__pipeopen (struct __unixlib_fd *file_desc, const char *file, int mode)
{
  return __fsopen (file_desc, file, mode);
}

int
__pipeclose (struct __unixlib_fd *file_desc)
{
  struct pipe *pipe = __pipe, *prev_pipe = NULL;
  _kernel_oserror *err = NULL;
  char path[MAXPATHLEN];

  while (pipe)
    {
      if (pipe->p[0] == file_desc || pipe->p[1] == file_desc)
	break;
      prev_pipe = pipe;
      pipe = pipe->next;
    }
  if (!pipe)
    return __set_errno (EBADF);
  if (prev_pipe)
    prev_pipe->next = pipe->next;
  else
    __pipe = pipe->next;
  __fsclose (file_desc);

  if (__riscosify_std (pipe->file, 0, path, sizeof (path), NULL))
    {
      if ((err = __os_fsctrl (27, path, 0, 0x1a2)))
        __seterr (err);
    }
  else
    err = (void *) 1; /* Conversion failed.  */

  free (pipe->file);
  free (pipe);
  return err ? -1 : 0;
}

int
__piperead (struct __unixlib_fd *file_desc, void *data, int nbyte)
{
  return __fsread (file_desc, data, nbyte);
}

int
__pipewrite (struct __unixlib_fd *file_desc, const void *data, int nbyte)
{
  int handle, regs[3];

  handle = (int) file_desc->handle;
  /* Read current file position.  */
  if (! __os_args (0, handle, 0, regs))
    {
      int offset = regs[2];
      /* Read extent && Set file pointer to end of file.  */
      if (! __os_args (2, handle, 0, regs)
          && ! __os_args (1, handle, regs[2], regs))
    	  {
  	    /* Write some more data to the "pipe" at the end of the file.  */
            int write_err = __fswrite (file_desc, data, nbyte);
  	    /* Restore the pointer to where we found it.  */
    	    if (! __os_args (1, handle, offset, regs))
              return write_err;
          }
    }
  return __set_errno (EPIPE);
}

#ifdef __UNIXLIB_NO_COMMON_DEV
__off_t
__pipelseek (struct __unixlib_fd *file_desc, __off_t lpos, int whence)
{
  IGNORE (lpos);
  IGNORE (whence);
  IGNORE (file_desc);
  return __set_errno (ESPIPE);
}

int
__pipeioctl (struct __unixlib_fd *file_desc, int request, void *arg)
{
  IGNORE (request);
  IGNORE (arg);
  IGNORE (file_desc);
  return __set_errno (EINVAL);
}
#endif

int
__pipeselect (struct __unixlib_fd *file_desc, int fd,
		__fd_set *read, __fd_set *write, __fd_set *except)
{
  /* Return ready to write, no exceptional conditions.  Ready to read is a
     little more tricky.  Bascially, work out if there is any data ready.
     Start by assuming there is nothing to read.  */
  int to_read = 0;

  if (read)
    {
	int regs[3];
	int pos;
	/* Read current file position.  */
	if (! __os_args (0, (int)file_desc->handle, 0, regs))
	  {
	    pos = regs[2];
	    /* Read extent.  */
	    if (! __os_args (2, (int)file_desc->handle, 0, regs))
		if (pos != regs[2])
		  to_read = 1;
	    /* If file pointer != extent then there is still data to read.  */
	    }
      if (to_read) {
	FD_SET(fd, read);
      } else
	FD_CLR (fd, read);
    }

  if (write)
    FD_SET (fd, write);
  if (except)
    FD_CLR (fd, except);

  /* This may not be correct, but it is consistent with Internet 5 select.  */
  return to_read + (write ? 1 : 0);
}
#endif /* __FEATURE_PIPEDEV */

void *
__nullopen (struct __unixlib_fd *file_desc, const char *file, int mode)
{
  IGNORE (file);
  IGNORE (mode);
  IGNORE (file_desc);
  /* Position of `NULL' in `__sfile' array.  */
  return (void *) 3;
}

int
__nullclose (struct __unixlib_fd *file_desc)
{
  IGNORE (file_desc);
  return 0;
}

int
__nullread (struct __unixlib_fd *file_desc, void *data, int nbyte)
{
  IGNORE (data);
  IGNORE (nbyte);
  IGNORE (file_desc);
  return 0;
}

int
__nullwrite (struct __unixlib_fd *file_desc, const void *data, int nbyte)
{
  IGNORE (data);
  IGNORE (file_desc);
  return nbyte;
}

#ifdef __UNIXLIB_NO_COMMON_DEV
__off_t
__nulllseek (struct __unixlib_fd *file_desc, __off_t lpos, int whence)
{
  IGNORE (lpos);
  IGNORE (whence);
  IGNORE (file_desc);
  return __set_errno (ESPIPE);
}

int
__nullioctl (struct __unixlib_fd *file_desc, int request, void *arg)
{
  IGNORE (request);
  IGNORE (arg);
  IGNORE (file_desc);
  return __set_errno (EINVAL);
}

int
__nullselect (struct __unixlib_fd *file_descriptor, int fd,
		__fd_set *read, __fd_set *write, __fd_set *except)
{
  /* Return ready to read, ready to write, no execptional conditions.  */
  IGNORE (file_desc);
  if (read)
    FD_SET(fd, read);
  if (write)
    FD_SET (fd, write);
  if (except)
    FD_CLR (fd, except);

  /* This may not be correct, but it is consistent with Internet 5
     select.  */
  return (read ? 1 : 0) + (write ? 1 : 0);
}
#endif

#if __FEATURE_SOCKET
void *
__sockopen (struct __unixlib_fd *file_desc, const char *file, int mode)
{
  IGNORE (file);
  IGNORE (mode);
  IGNORE (file_desc);
  return (void *) __set_errno (EOPNOTSUPP);
}

int
__sockclose (struct __unixlib_fd *file_desc)
{
  return _sclose ((int)file_desc->handle);
}

int
__sockread (struct __unixlib_fd *file_desc, void *data, int nbyte)
{
  return _sread ((int)file_desc->handle, data, nbyte);
}

int
__sockwrite (struct __unixlib_fd *file_desc, const void *data, int nbyte)
{
  return _swrite ((int)file_desc->handle, data, nbyte);
}

#ifdef __UNIXLIB_NO_COMMON_DEV
__off_t
__socklseek (struct __unixlib_fd *file_desc, __off_t lpos, int whence)
{
  IGNORE (lpos);
  IGNORE (whence);
  IGNORE (file_desc);
  return __set_errno (ESPIPE);
}
#endif

int
__sockioctl (struct __unixlib_fd *file_desc, int request, void *arg)
{
  return _sioctl ((int)file_desc->handle, request, arg);
}

int
__sockselect (struct __unixlib_fd *file_descriptor, int fd,
		__fd_set *read, __fd_set *write, __fd_set *except)
{
  /* Return 1,1,1 so that merge routine will copy in this socket's
     results.  */
  IGNORE (file_descriptor);
  if (read)
    FD_SET(fd, read);
  if (write)
    FD_SET (fd, write);
  if (except)
    FD_SET (fd, except);

  /* None are `live' yet.  */
  return 0;
}
#endif /* __FEATURE_SOCKET */