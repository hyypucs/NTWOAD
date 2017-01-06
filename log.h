/* 
    Log.c

    Log file 및 time string 함수

	date : 1999. 10. 9.
	by hyypucs
*/

#if !defined(__LOG_H__)

#define __LOG_H__

#define DATE_BUF_MAX    64


tVOID LogWrite(tCHAR *szLogFile, tCHAR *format, ...);
// 하루 단위로 로그가 쌓인다. szLogFile+Date
tVOID DateLogWrite(tCHAR *szLogFile, tCHAR *format, ...);

tVOID SetErrorPos(tCHAR *szFile, tINT nLine);
tCHAR *GetErrorPos();
#endif /* log.h */

