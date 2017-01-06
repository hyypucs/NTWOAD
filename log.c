#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <varargs.h>
#include <sys/types.h>
#include <time.h>
#include "dwtype.h"
#include "log.h"
#include "util.h"

#define LOGFILE_LEN 1024

/*
#if defined(_MTHREAD)
pthread_mutex_t gCheckLogger;
PRIVATE tBOOL bStLogger = FALSE;

tBOOL InitLogger(tVOID)
{
	if (bStLogger) return TRUE;
	if (pthread_mutex_init(&gCheckLogger, NULL) != 0) {
		return FALSE;
        }
	bStLogger = TRUE;
}

tBOOL CloseLogger(tVOID)
{
	if (bStLogger == FALSE) return TRUE;
	if (pthread_mutex_lock(&gCheckLogger) != 0) return FALSE;
	if (pthread_mutex_unlock(&gCheckLogger);
        pthread_mutex_destroy(&gCheckLogger); // mutex 지워야지..
}
#endif
*/

tVOID LogWrite(tCHAR *szLogFile, tCHAR *format, ...)
{
  va_list   args;
  FILE      *fp;
  time_t    t = time(NULL);
  struct tm *st = localtime(&t);
  char      buf[DATE_BUF_MAX];


  if ((fp=fopen(szLogFile, "a+")) != NULL) {

    llock(fileno(fp), DWLOCK_EX);
    strftime(buf, sizeof(buf), "%Y/%m/%d %T", st);

    fprintf(fp, "%s ", buf);
    va_start(args, format);
    (void) vfprintf(fp, format, args);
    va_end(args);
    fputc('\n', fp);
    llock(fileno(fp), DWLOCK_UN);
    fclose(fp);
  }

}

tVOID DateLogWrite(tCHAR *szLogFile, tCHAR *format, ...)
{
  va_list   args;
  FILE      *fp;
  time_t    t = time(NULL);
  struct tm *st = localtime(&t);
  char      buf[DATE_BUF_MAX];
  char      szDateLogFile[LOGFILE_LEN];

  strftime(buf, sizeof(buf), "%Y%m%d", st);
  sprintf(szDateLogFile, "%s%s", szLogFile, buf);

  if ((fp=fopen(szDateLogFile, "a+")) != NULL) {
    llock(fileno(fp), DWLOCK_EX);
    strftime(buf, sizeof(buf), "%Y/%m/%d %T", st);
    fprintf(fp, "%s ", buf);
    va_start(args, format);
    (void) vfprintf(fp, format, args);
    va_end(args);
    fputc('\n', fp);
    llock(fileno(fp), DWLOCK_UN);
    fclose(fp);
  }

}

tCHAR gszErrorPos[256];
/*========================================================
  SetErrorPos
	Description : 
 =======================================================*/
tVOID SetErrorPos(tCHAR *szFile, tINT nLine)
{
	sprintf(gszErrorPos, "Error Position => File : %s, Line : %d\n", szFile, nLine);
}

/*========================================================
  GetErrorPos
	Description : 
 =======================================================*/
tCHAR *GetErrorPos()
{
	return gszErrorPos;
}

