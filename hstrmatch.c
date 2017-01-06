#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>

#include "dwtype.h"
#include "hstrmatch.h"

#define	FNM_NOMATCH	1

#define	FNM_PATHNAME	(1 << 0) /* No wildcard can ever match `/'.  */
#define	FNM_NOESCAPE	(1 << 1) /* Backslashes don't quote special chars.  */
#define	FNM_PERIOD	(1 << 2) /* Leading `.' is matched only explicitly.  */
#define	FNM_FILE_NAME	FNM_PATHNAME /* Preferred GNU name.  */
#define	FNM_LEADING_DIR	(1 << 3) /* Ignore `/...' after a match.  */
#define	FNM_CASEFOLD	(1 << 4) /* Compare without regard to case.  */

PRIVATE int StrMatchByFlag (const tCHAR *pattern, const tCHAR *string, int flags);
PRIVATE int HStrMatchByFlag (const tHCHAR *pattern, const tHCHAR *string, int flags);

int HStrMatch (const tHCHAR *pattern, const tHCHAR *string)
{
	return(HStrMatchByFlag(pattern, string, 1));
}

//old version #define FOLD(c)	((flags & FNM_CASEFOLD) && !(c&0x80) && isupper ((int)c) ? tolower ((int)c) : (c))

#define FOLD(c)	(c)

/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, nonzero if not.  */
PRIVATE int HStrMatchByFlag (const tHCHAR *pattern, const tHCHAR *string, int flags)
{
  register const tHCHAR *p = pattern, *n = string;
  register tHCHAR c;

/* Note that this evalutes C many times.  */

  while ((c = *p++) != '\0')
    {
      c = FOLD ((int)c);

      switch (c)
	{
	case '?':
	  if (*n == (tHCHAR)'\0')
	    return FNM_NOMATCH;
	  else if ((flags & FNM_FILE_NAME) && *n == (tHCHAR)'/')
	    return FNM_NOMATCH;
	  else if ((flags & FNM_PERIOD) && *n == (tHCHAR)'.' &&
		   (n == string || ((flags & FNM_FILE_NAME) && n[-1] == (tHCHAR)'/')))
	    return FNM_NOMATCH;
	  break;

	case '\\':
	  if (!(flags & FNM_NOESCAPE))
	    {
	      c = *p++;
	      c = FOLD (c);
	    }
	  if (FOLD (*n) != c)
	    return FNM_NOMATCH;
	  break;

	case '*':
	  if ((flags & FNM_PERIOD) && *n == (tHCHAR)'.' &&
	      (n == string || ((flags & FNM_FILE_NAME) && n[-1] == (tHCHAR)'/')))
	    return FNM_NOMATCH;

	  for (c = *p++; c == (tHCHAR)'?' || c == (tHCHAR)'*'; c = *p++, ++n)
	    if (((flags & FNM_FILE_NAME) && *n == (tHCHAR)'/') ||
		(c == (tHCHAR)'?' && *n == (tHCHAR)'\0'))
	      return FNM_NOMATCH;

	  if (c == (tHCHAR)'\0')
	    return 0;

	  {
	    tHCHAR c1 = (!(flags & FNM_NOESCAPE) && c == (tHCHAR)'\\') ? *p : c;
	    c1 = FOLD (c1);
	    for (--p; *n != '\0'; ++n)
	      if ((c == (tHCHAR)'[' || FOLD (*n) == c1) &&
		  HStrMatchByFlag (p, n, flags & ~FNM_PERIOD) == 0)
		return 0;
	    return FNM_NOMATCH;
	  }

	case '[':
	  {
	    /* Nonzero if the sense of the character class is inverted.  */
	    register int not1;

	    if (*n == '\0')
	      return FNM_NOMATCH;

	    if ((flags & FNM_PERIOD) && *n == (tHCHAR)'.' &&
		(n == string || ((flags & FNM_FILE_NAME) && n[-1] == (tHCHAR)'/')))
	      return FNM_NOMATCH;

	    not1 = (*p == (tHCHAR)'!' || *p == (tHCHAR)'^');
	    if (not1)
	      ++p;

	    c = *p++;
	    for (;;)
	      {
		register tHCHAR cstart = c, cend = c;

		if (!(flags & FNM_NOESCAPE) && c == (tHCHAR)'\\')
		  cstart = cend = *p++;

		cstart = cend = FOLD (cstart);

		if (c == (tHCHAR)'\0')
		  /* [ (unterminated) loses.  */
		  return FNM_NOMATCH;

		c = *p++;
		c = FOLD (c);

		if ((flags & FNM_FILE_NAME) && c == (tHCHAR)'/')
		  /* [/] can never match.  */
		  return FNM_NOMATCH;

		if (c == (tHCHAR)'-' && *p != (tHCHAR)']')
		  {
		    cend = *p++;
		    if (!(flags & FNM_NOESCAPE) && cend == (tHCHAR)'\\')
		      cend = *p++;
		    if (cend == (tHCHAR)'\0')
		      return FNM_NOMATCH;
		    cend = FOLD (cend);

		    c = *p++;
		  }

		if (FOLD (*n) >= cstart && FOLD (*n) <= cend)
		  goto matched;

		if (c == (tHCHAR)']')
		  break;
	      }
	    if (!not1)
	      return FNM_NOMATCH;
	    break;

	  matched:;
	    /* Skip the rest of the [...] that already matched.  */
	    while (c != (tHCHAR)']')
	      {
		if (c == (tHCHAR)'\0')
		  /* [... (unterminated) loses.  */
		  return FNM_NOMATCH;

		c = *p++;
		if (!(flags & FNM_NOESCAPE) && c == (tHCHAR)'\\')
		  /* XXX 1003.2d11 is unclear if this is right.  */
		  ++p;
	      }
	    if (not1)
	      return FNM_NOMATCH;
	  }
	  break;

	default:
	  if (c != FOLD (*n))
	    return FNM_NOMATCH;
	}

      ++n;
    }

  if (*n == '\0')
    return 0;

  if ((flags & FNM_LEADING_DIR) && *n == (tHCHAR)'/')
    /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
    return 0;

  return FNM_NOMATCH;
}


int StrMatch (const tCHAR *pattern, const tCHAR *string)
{
	return(StrMatchByFlag(pattern, string, 1));
}


/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, nonzero if not.  */
PRIVATE int StrMatchByFlag (const tCHAR *pattern, const tCHAR *string, int flags)
{
  register const tCHAR *p = pattern, *n = string;
  register tCHAR c;

/* Note that this evalutes C many times.  */

  while ((c = *p++) != '\0')
    {
      c = FOLD ((int)c);

      switch (c)
	{
	case '?':
	  if (*n == (tCHAR)'\0')
	    return FNM_NOMATCH;
	  else if ((flags & FNM_FILE_NAME) && *n == (tCHAR)'/')
	    return FNM_NOMATCH;
	  else if ((flags & FNM_PERIOD) && *n == (tCHAR)'.' &&
		   (n == string || ((flags & FNM_FILE_NAME) && n[-1] == (tCHAR)'/')))
	    return FNM_NOMATCH;
	  break;

	case '\\':
	  if (!(flags & FNM_NOESCAPE))
	    {
	      c = *p++;
	      c = FOLD (c);
	    }
	  if (FOLD (*n) != c)
	    return FNM_NOMATCH;
	  break;

	case '*':
	  if ((flags & FNM_PERIOD) && *n == (tCHAR)'.' &&
	      (n == string || ((flags & FNM_FILE_NAME) && n[-1] == (tCHAR)'/')))
	    return FNM_NOMATCH;

	  for (c = *p++; c == (tCHAR)'?' || c == (tCHAR)'*'; c = *p++, ++n)
	    if (((flags & FNM_FILE_NAME) && *n == (tCHAR)'/') ||
		(c == (tCHAR)'?' && *n == (tCHAR)'\0'))
	      return FNM_NOMATCH;

	  if (c == (tCHAR)'\0')
	    return 0;

	  {
	    tCHAR c1 = (!(flags & FNM_NOESCAPE) && c == (tCHAR)'\\') ? *p : c;
	    c1 = FOLD (c1);
	    for (--p; *n != '\0'; ++n)
	      if ((c == (tCHAR)'[' || FOLD (*n) == c1) &&
		  StrMatchByFlag (p, n, flags & ~FNM_PERIOD) == 0)
		return 0;
	    return FNM_NOMATCH;
	  }

	case '[':
	  {
	    /* Nonzero if the sense of the character class is inverted.  */
	    register int not1;

	    if (*n == '\0')
	      return FNM_NOMATCH;

	    if ((flags & FNM_PERIOD) && *n == (tCHAR)'.' &&
		(n == string || ((flags & FNM_FILE_NAME) && n[-1] == (tCHAR)'/')))
	      return FNM_NOMATCH;

	    not1 = (*p == (tCHAR)'!' || *p == (tCHAR)'^');
	    if (not1)
	      ++p;

	    c = *p++;
	    for (;;)
	      {
		register tCHAR cstart = c, cend = c;

		if (!(flags & FNM_NOESCAPE) && c == (tCHAR)'\\')
		  cstart = cend = *p++;

		cstart = cend = FOLD (cstart);

		if (c == (tCHAR)'\0')
		  /* [ (unterminated) loses.  */
		  return FNM_NOMATCH;

		c = *p++;
		c = FOLD (c);

		if ((flags & FNM_FILE_NAME) && c == (tCHAR)'/')
		  /* [/] can never match.  */
		  return FNM_NOMATCH;

		if (c == (tCHAR)'-' && *p != (tCHAR)']')
		  {
		    cend = *p++;
		    if (!(flags & FNM_NOESCAPE) && cend == (tCHAR)'\\')
		      cend = *p++;
		    if (cend == (tCHAR)'\0')
		      return FNM_NOMATCH;
		    cend = FOLD (cend);

		    c = *p++;
		  }

		if (FOLD (*n) >= cstart && FOLD (*n) <= cend)
		  goto matched;

		if (c == (tCHAR)']')
		  break;
	      }
	    if (!not1)
	      return FNM_NOMATCH;
	    break;

	  matched:;
	    /* Skip the rest of the [...] that already matched.  */
	    while (c != (tCHAR)']')
	      {
		if (c == (tCHAR)'\0')
		  /* [... (unterminated) loses.  */
		  return FNM_NOMATCH;

		c = *p++;
		if (!(flags & FNM_NOESCAPE) && c == (tCHAR)'\\')
		  /* XXX 1003.2d11 is unclear if this is right.  */
		  ++p;
	      }
	    if (not1)
	      return FNM_NOMATCH;
	  }
	  break;

	default:
	  if (c != FOLD (*n))
	    return FNM_NOMATCH;
	}

      ++n;
    }

  if (*n == '\0')
    return 0;

  if ((flags & FNM_LEADING_DIR) && *n == (tCHAR)'/')
    /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
    return 0;

  return FNM_NOMATCH;
}
