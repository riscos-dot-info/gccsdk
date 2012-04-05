#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

unsigned long long
strtoull (const char *nptr, char **end, int base)
{
  int overflow, neg = 0;
  unsigned int digit;
  unsigned long long result;
  unsigned long long cutoff;
  unsigned long cutlim;
  const char *s, *save;

  if (!nptr)
    return 0;

  if (base < 0 || base == 1 || base > 36)
    {
      errno = EINVAL;
      return 0;
    }

  save = s = nptr;
  /* Skip white space.  */
  while (isspace (*s))
    s++;

  if (*s == '+' || *s == '-')
    {
      if (*s == '-')
	neg = 1;
      s++;
    }

  if (base == 16 && s[0] == '0' && toupper (s[1]) == 'X')
    s += 2;

  /* Try guessing the base.  */
  if (base == 0)
    {
      if (*s == '0')
	{
	  if (toupper (s[1]) == 'X')
	    {
	      s += 2;
	      base = 16;
	    }
	  else
	    base = 8;
	}
      else
	base = 10;
    }

  /* Save the pointer so we can check later if anything happened.  */
  save = s;

  cutoff = ULONG_LONG_MAX / base;
  cutlim = ULONG_LONG_MAX % base;

  overflow = 0;
  result = 0;
  for (digit = *s; digit != '\0'; digit = *++s)
    {
      if (digit >= '0' && digit <= '9')
	digit -= '0';
      else if (isalpha (digit))
	digit = toupper (digit) - 'A' + 10;
      else
	break;
      if (digit >= base)
	break;
      /* Overflow checks. */
      if (result > cutoff || (result == cutoff && digit > cutlim))
	overflow = 1;
      else
	{
	  result *= base;
	  result += digit;
	}
    }
  if (s == save)
    {
      if (end != NULL)
	{
	  /* Special case, the base is zero or hexadecimal and the
	     first two chars are 0x, but nothing else follows. This
	     isn't an error.  */
	  if ((save - nptr) >= 2 && toupper (save[-1]) == 'X' && save[-2] == '0')
	    *end = (char *) &save[-1];
	  else
	    *end = (char *) nptr;
	}
      return 0;
    }

  /* Store the address of one character past the last we converted.  */
  if (end)
    *end = (char *) s;

  if (overflow)
    {
      errno = ERANGE;
      return ULONG_LONG_MAX;
    }

  return (neg) ? 0 - result : result;
}