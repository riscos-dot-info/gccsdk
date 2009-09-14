/****************************************************************************
 *
 * $Source: /usr/local/cvsroot/gccsdk/unixlib/source/unix/features.c,v $
 * $Date: 2003/08/18 22:35:36 $
 * $Revision: 1.7 $
 * $State: Exp $
 * $Author: joty $
 *
 ***************************************************************************/

#ifdef EMBED_RCSID
static const char rcs_id[] = "$Id: features.c,v 1.7 2003/08/18 22:35:36 joty Exp $";
#endif

/* #define DEBUG 1 */

#include <string.h>
#include <unixlib/unix.h>
#include <unixlib/local.h>

static const char __sfix_default[] = "a:c:cc:f:h:i:ii:l:o:p:s:y";

static char *get_program_name (const char *cli, char *fname_buf, size_t fname_buf_len);

/* Get the leaf name from the command line used to run the program.  */
static char *get_program_name (const char *cli, char *fname_buf, size_t fname_buf_len)
{
  char *out = fname_buf;
  const char *start, *end;

  /* Skip any initial whitespace.  */
  while (*cli == ' ')
    cli++;

  /* Find the end of the program name.  Set 'start' to mark the
     beginning of the program name. Use '.' and '/' as the directory
     separation characters to calculate this.  */
  start = cli;
  while (*cli != ' ' && *cli != '\0')
    {
      if (*cli == '/' || *cli == '.')
	/* Point to the character after the separation char.  */
	start = cli + 1;
      cli++;
    }
  end = cli;

  /* Copy the program name into 'out'.  It's bounds should now be
     marked out by 'start' and 'end'.  */
  while (--fname_buf_len && start != end)
    *out++ = *start++;

  /* Zero terminate it.  */
  *out = '\0';

  return fname_buf;
}

/* Build up an environment variable name, variable, with the UnixEnv$
   prefixed to it.  If program_name is non-zero then look for the
   variable as UnixEnv$program_name$variable, otherwise look for it
   as UnixEnv$variable.

   The intention is to allow the programmer the opportunity to turn on/off
   certain UnixLib runtime functionality, for example the filename
   translations.  This can either be done globally i.e. once set, all
   applications will be affected, or per-process.  */
static char *env (const char *program_name, const char *variable,
		  char *buffer, size_t bufsiz)
{
  char envvar[128];
  char *ptr, *result;

  ptr = stpcpy (envvar, __UNIX_ENV_PREFIX);
  if (program_name != NULL)
    {
      ptr = stpcpy (ptr, program_name);
      *ptr++ = '$';
    }
  ptr = stpcpy (ptr, variable);

#ifdef DEBUG
  __os_print ("features.c (env): Looking for '"); __os_print (envvar);
  __os_print ("'.");
#endif

  result = __getenv_from_os (envvar, buffer, bufsiz);

#ifdef DEBUG
  __os_print ((result) ? "Found\r\n" : "Not found\r\n");
#endif
  return result;
}

/* We have to be careful with the string processing because not enough
   of UnixLib will have been initialised to use more powerful functions.  */
static void features (const char *progname)
{
  char varbuf[256];
  char *ptr;

  /* Feature "nonametrans".  */
  ptr = env (progname, "nonametrans", varbuf, sizeof (varbuf) - 1);
  if (ptr != NULL)
    __set_riscosify_control(__RISCOSIFY_NO_PROCESS);

  /* Feature "sfix".  */
  ptr = env (progname, "sfix", varbuf, sizeof (varbuf) - 1);
  __sfixinit (ptr ? ptr : __sfix_default);

  /* Feature "uid".  */
  ptr = env (progname, "uid", varbuf, sizeof (varbuf) - 1);
  if (ptr != NULL)
    {
      unsigned int uid = __decstrtoui(ptr, NULL);

    __u->uid = __u->euid = (uid_t)uid;
    }
}

void __runtime_features (const char *cli)
{
  char program_name[128];

  /* Initialise runtime features to their default values before querying
     the environment variables.  */
  __sdirinit (); /* Initialise riscosify.  */

  features (NULL);
  features (get_program_name (cli, program_name, sizeof (program_name)));
}