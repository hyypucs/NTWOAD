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
#include "trdir.h"

#define	FNM_NOMATCH	1

#define	FNM_PATHNAME	(1 << 0) /* No wildcard can ever match `/'.  */
#define	FNM_NOESCAPE	(1 << 1) /* Backslashes don't quote special chars.  */
#define	FNM_PERIOD	(1 << 2) /* Leading `.' is matched only explicitly.  */
#define	FNM_FILE_NAME	FNM_PATHNAME /* Preferred GNU name.  */
#define	FNM_LEADING_DIR	(1 << 3) /* Ignore `/...' after a match.  */
#define	FNM_CASEFOLD	(1 << 4) /* Compare without regard to case.  */


PRIVATE tBOOL isDir(tCHAR *szStr);
PRIVATE tBOOL isFile(tCHAR *szStr);
PRIVATE tBOOL isPatternMatched(tCHAR *szPattern, tCHAR *szFile);
PRIVATE int fnmatch (const char *pattern, const char *string, int flags);


PUBLIC tVOID trDirBreadth(tCHAR *szPath, tCHAR *szPattern, putFuncType putFunc, tVOID *lpParam)
{
	DIR *dp;
	struct dirent *ep;
	//tCHAR szCurrPath[1024];
	tCHAR szOutPath[1024];

	if (chdir(szPath)!=0) return;
	//getcwd(szCurrPath, 1023);
	dp = opendir(szPath);
	if(dp != NULL) {
		if (szPath[strlen(szPath)-1] != '/') strcat(szPath, "/");
		while((ep = readdir(dp)) != NULL) {
			if(isFile(ep->d_name) && 
				isPatternMatched(szPattern, ep->d_name)) {
				putFunc(szPath, ep->d_name, lpParam);
			}
		}
		closedir(dp);
	}

	dp = opendir(szPath);
	if(dp != NULL) {
		while((ep = readdir(dp)) != NULL) {
			if(isDir(ep->d_name)) {
				strcpy(szOutPath, szPath);
				strcat(szOutPath, "/");
				strcat(szOutPath, ep->d_name);
				trDirBreadth(szOutPath, szPattern, putFunc, lpParam);
			}
		}
		closedir(dp);
		chdir("..");
	}
}

PUBLIC tVOID trDirDepth(tCHAR *szPath, tCHAR *szPattern, putFuncType putFunc, tVOID *lpParam)
{
	DIR *dp;
	struct dirent *ep;
	//tCHAR szCurrPath[1024];
	tCHAR szOutPath[1024];

	if (chdir(szPath)!=0) return;
	dp = opendir(szPath);
	if(dp != NULL) {
		while((ep = readdir(dp)) != NULL) {
			if(isDir(ep->d_name)) {
				strcpy(szOutPath, szPath);
				strcat(szOutPath, "/");
				strcat(szOutPath, ep->d_name);
				trDirDepth(szOutPath, szPattern, putFunc, lpParam);
			}
		}
		closedir(dp);
	}

	dp = opendir(szPath);
	if(dp != NULL) {
		if (szPath[strlen(szPath)-1] != '/') strcat(szPath, "/");
		while((ep = readdir(dp)) != NULL) {
			if(isFile(ep->d_name) &&
				isPatternMatched(szPattern, ep->d_name)) {
				putFunc(szPath, ep->d_name, lpParam);
			}
		}
		closedir(dp);
		chdir("..");
	}

}

PRIVATE tBOOL isDir(tCHAR *szStr)
{
	struct stat st;

	if(lstat(szStr, &st) == -1) return FALSE;
	if(S_ISLNK(st.st_mode)) return FALSE;
	if(((st.st_mode & S_IFMT) == S_IFDIR) &&
		strcmp(szStr, ".") != 0 &&
		strcmp(szStr, "..") != 0) return TRUE;
	return FALSE;
}

PRIVATE tBOOL isFile(tCHAR *szStr)
{
	struct stat st;

	if(lstat(szStr, &st) == -1) return FALSE;
	if(S_ISLNK(st.st_mode)) return FALSE;
	if((st.st_mode & S_IFMT) == S_IFREG) return TRUE;
	return FALSE;

}


PRIVATE tBOOL isPatternMatched(tCHAR *szPattern, tCHAR *szFile)
{
        tCHAR szPatternArr[5][50];
	tCHAR szBuf[256];
        tCHAR *delims = { ";" };
	tCHAR *p;
	tINT num, i;

	num = 0;
	strcpy(szBuf, szPattern);
        p = strtok( szBuf, delims );
        while( p != NULL ) {
                strcpy(szPatternArr[num], p);
                num++;
                p = strtok( NULL, delims );
        }
	for(i = 0; i < num; i++) {
		if(fnmatch(szPatternArr[i], szFile, 1) == 0)
			return TRUE;
	}
	return FALSE;
}


/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, nonzero if not.  */
PRIVATE int fnmatch (const char *pattern, const char *string, int flags)
{
  register const char *p = pattern, *n = string;
  register char c;

/* Note that this evalutes C many times.  */
#define FOLD(c)	((flags & FNM_CASEFOLD) && isupper ((int)c) ? tolower ((int)c) : (c))

  while ((c = *p++) != '\0')
    {
      c = FOLD ((int)c);

      switch (c)
	{
	case '?':
	  if (*n == '\0')
	    return FNM_NOMATCH;
	  else if ((flags & FNM_FILE_NAME) && *n == '/')
	    return FNM_NOMATCH;
	  else if ((flags & FNM_PERIOD) && *n == '.' &&
		   (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
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
	  if ((flags & FNM_PERIOD) && *n == '.' &&
	      (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
	    return FNM_NOMATCH;

	  for (c = *p++; c == '?' || c == '*'; c = *p++, ++n)
	    if (((flags & FNM_FILE_NAME) && *n == '/') ||
		(c == '?' && *n == '\0'))
	      return FNM_NOMATCH;

	  if (c == '\0')
	    return 0;

	  {
	    char c1 = (!(flags & FNM_NOESCAPE) && c == '\\') ? *p : c;
	    c1 = FOLD (c1);
	    for (--p; *n != '\0'; ++n)
	      if ((c == '[' || FOLD (*n) == c1) &&
		  fnmatch (p, n, flags & ~FNM_PERIOD) == 0)
		return 0;
	    return FNM_NOMATCH;
	  }

	case '[':
	  {
	    /* Nonzero if the sense of the character class is inverted.  */
	    register int not1;

	    if (*n == '\0')
	      return FNM_NOMATCH;

	    if ((flags & FNM_PERIOD) && *n == '.' &&
		(n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
	      return FNM_NOMATCH;

	    not1 = (*p == '!' || *p == '^');
	    if (not1)
	      ++p;

	    c = *p++;
	    for (;;)
	      {
		register char cstart = c, cend = c;

		if (!(flags & FNM_NOESCAPE) && c == '\\')
		  cstart = cend = *p++;

		cstart = cend = FOLD (cstart);

		if (c == '\0')
		  /* [ (unterminated) loses.  */
		  return FNM_NOMATCH;

		c = *p++;
		c = FOLD (c);

		if ((flags & FNM_FILE_NAME) && c == '/')
		  /* [/] can never match.  */
		  return FNM_NOMATCH;

		if (c == '-' && *p != ']')
		  {
		    cend = *p++;
		    if (!(flags & FNM_NOESCAPE) && cend == '\\')
		      cend = *p++;
		    if (cend == '\0')
		      return FNM_NOMATCH;
		    cend = FOLD (cend);

		    c = *p++;
		  }

		if (FOLD (*n) >= cstart && FOLD (*n) <= cend)
		  goto matched;

		if (c == ']')
		  break;
	      }
	    if (!not1)
	      return FNM_NOMATCH;
	    break;

	  matched:;
	    /* Skip the rest of the [...] that already matched.  */
	    while (c != ']')
	      {
		if (c == '\0')
		  /* [... (unterminated) loses.  */
		  return FNM_NOMATCH;

		c = *p++;
		if (!(flags & FNM_NOESCAPE) && c == '\\')
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

  if ((flags & FNM_LEADING_DIR) && *n == '/')
    /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
    return 0;

  return FNM_NOMATCH;
}

