/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/stdio/stdio.c,v $
 * $Date: 2000/07/15 14:52:32 $
 * $Revision: 1.1.1.1 $
 * $State: Exp $
 * $Author: nick $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: stdio.c,v 1.1.1.1 2000/07/15 14:52:32 nick Exp $";
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

__STDIOLIB__

FILE *__iob_head = NULL;
FILE *__stdin, *__stdout, *__stderr;

void
__stdioinit (void)
{
  /* Open the standard streams.  */
  stdin = fdopen (0, "r");
  stdout = fdopen (1, "w");
  stderr = fdopen (2, "w");

  /* Create line oriented buffers for stdin and stdout.  */
  stdin->i_base = stdin->i_ptr = (unsigned char *) malloc (BUFSIZ);
  stdin->i_cnt = 0;
  stdin->__bufsize = BUFSIZ;
  stdin->__linebuf = 1;
  stdout->o_base = stdout->o_ptr = (unsigned char *) malloc (BUFSIZ);
  stdout->o_cnt = 0;
  stdout->__bufsize = BUFSIZ;
  stdout->__linebuf = 1;

  /* Don't buffer stderr.  */
  stderr->__bufsize = 0;
  /* setvbuf (stdin, NULL, _IOLBF, BUFSIZ);
  setvbuf (stdout, NULL, _IOLBF, BUFSIZ);
  setvbuf (stderr, NULL, _IONBF, BUFSIZ); */
}

void
__stdioexit (void)
{
  /* Close all streams.  */
  fclose (NULL);
}
