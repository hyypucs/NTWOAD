#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "makedir.h"

#define STRDUP_ALLOCA(ptr, str) do {            \
  (ptr) = (char *)alloca (strlen (str) + 1);    \
  strcpy (ptr, str);                            \
} while (0)


/* Does FILENAME exist?  This is quite a lousy implementation, since
   it supplies no error codes -- only a yes-or-no answer.  Thus it
   will return that a file does not exist if, e.g., the directory is
   unreadable.  I don't mind it too much currently, though.  The
   proper way should, of course, be to have a third, error state,
   other than true/false, but that would introduce uncalled-for
   additional complexity to the callers.  */
int file_exists_p (const char *filename)
{
  return access (filename, F_OK) >= 0;
}

/* Returns 0 if PATH is a directory, 1 otherwise (any kind of file).
   Returns 0 on error.  */
int file_non_directory_p (const char *path)
{
  struct stat buf;
  /* Use lstat() rather than stat() so that symbolic links pointing to
     directories can be identified correctly.  */
  if (lstat (path, &buf) != 0)
    return 0;
  return S_ISDIR (buf.st_mode) ? 0 : 1;
}


int MakeDirectory (const char *szDir)
{
  int quit = 0;
  int i;
  char *dir;

  /* Make a copy of dir, to be able to write to it.  Otherwise, the
     function is unsafe if called with a read-only char *argument.  */
  STRDUP_ALLOCA (dir, szDir);
  
  /* If the first character of dir is '/', skip it (and thus enable
     creation of absolute-pathname directories.  */
  for (i = (*dir == '/'); 1; ++i)
    {
      for (; dir[i] && dir[i] != '/'; i++)
	;
      if (!dir[i])
	quit = 1;
      dir[i] = '\0';
      /* Check whether the directory already exists.  */
      if (!file_exists_p (dir))
	{
	  if (mkdir (dir, 0777) < 0)
	    return -1;
	}
      if (quit)
	break;
      else
	dir[i] = '/';
    }
  return 0;
}

