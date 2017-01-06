//
// 해당 호스트를 가져오는 루틴 (http프로토콜을 이용해서)
//
//  by hyypucs
// 2000. 12. 27.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "dwtype.h"
#include "dwsock.h"
#include "gethtml.h"

#define HTMLALLOC_LIMIT 1024

tINT GetHtml(tCHAR *szHost, tCHAR *szPath, tINT nPort, tINT nMethod, tCHAR *szInput, tBYTE **RetBuf, tBOOL bBin, tBOOL bVirtualHost, tINT nTimeOut)
{
	tINT nLen, nStrLen, nTotalRead, nAlloc;
	tCHAR *szHdrFmt = "%s %s HTTP/1.0\r\n";
	tCHAR szBuf[MAX_BUF*4], szWorkBuf[MAX_BUF], szBuf1[MAX_BUF];
	tINT nSocketId;
	tINT nCheck = 0;

	szBuf[0] = '\0';
	nSocketId=SocketClientEx(szHost, nPort, nTimeOut);
	if (nSocketId <0) {
		//printf("%s, %d<br>\n", __FILE__, __LINE__);
		return 0;
	}

	//printf("%s, %d<br>\n", __FILE__, __LINE__);

	if (nMethod == CGI_POST_METHOD) {
		sprintf(szBuf, szHdrFmt, "POST", szPath);
		if (bVirtualHost) {
			sprintf(szBuf1, "Host: %s\r\n", szHost);
			strcat(szBuf, szBuf1);
		}
		strcat(szBuf, "User-Agent: DreamBot\r\n");
		strcpy(szWorkBuf, "Content-type: application/x-www-form-urlencoded\r\n");
		strcat(szBuf, szWorkBuf);
		nLen = strlen(szInput);
		sprintf(szWorkBuf, "Content-length: %d\r\n\r\n", nLen);
		strcat(szBuf, szWorkBuf);
		strcat(szBuf, szInput);
	}
	else if (nMethod == CGI_GET_METHOD) {
		sprintf(szBuf, szHdrFmt, "GET", szPath);
		if (bVirtualHost) {
			sprintf(szBuf1, "Host: %s\r\n", szHost);
			strcat(szBuf, szBuf1);
		}
		strcat(szBuf, "User-Agent: DreamBot\r\n\r\n");
	} 
	else {
		sprintf(szBuf, szHdrFmt, "HEAD", szPath);
		if (bVirtualHost) {
			sprintf(szBuf1, "Host: %s\r\n", szHost);
			strcat(szBuf, szBuf1);
		}
		strcat(szBuf, "User-Agent: DreamBot\r\n\r\n");
	}

	// send query string
	if (SocketWrite(nSocketId, (tBYTE *)szBuf, strlen(szBuf), 5) < (tINT)strlen(szBuf)) return 0;

	nTotalRead = 0;

	*RetBuf = NULL;
	*RetBuf = (tBYTE *)malloc(HTMLALLOC_LIMIT);
	nAlloc = HTMLALLOC_LIMIT;

	for(;;) {
		nStrLen = SocketGets(nSocketId, (tBYTE *)szBuf, MAX_BUF-1, nTimeOut);
		if (nStrLen <= 0) {
			break;
		}
		else {
			szBuf[nStrLen] = '\0';
			if (nCheck == 0) { // OK인지 아닌지.. 확인..
				if (strstr(szBuf, "200 OK")) { // 있다.. 해당되는 것이....
					if (nMethod == CGI_HEAD_METHOD) {
						nCheck = 2;
					}
					else {
						nCheck = 1;
					}
				}
				else {
					break;
				}
				continue;
				
			}
			else if (nCheck == 1) { // Return값 확인..
				if (szBuf[0] == '\n' || szBuf[0] == '\r') { // 이제부터 시작이지..
					nCheck = 2;
				}
				continue;
			}

			if (nTotalRead + nStrLen >= nAlloc) {
				*RetBuf = (tBYTE *)realloc(*RetBuf, nAlloc + HTMLALLOC_LIMIT);
				if (*RetBuf == NULL) { // exception
					nTotalRead = 0;
					break;
				}
				nAlloc += HTMLALLOC_LIMIT;
			}
			memcpy(*RetBuf + nTotalRead, szBuf, nStrLen);
			nTotalRead += nStrLen;

		}
	}

	// close socket
	close(nSocketId);
	if (nTotalRead) {
		if (bBin == FALSE) *(*RetBuf + nTotalRead) = '\0';
		return nTotalRead;
	}
	else {
		if (*RetBuf) free(*RetBuf);
	}
	return 0;

} 

tINT GetHtml2(tCHAR *szHost, tCHAR *szPath, tINT nPort, tINT nMethod, tCHAR *szInput, tBYTE **RetBuf, tBOOL bBin, tBOOL bVirtualHost, tINT nConnectTimeOut, tINT nReadTimeOut, tINT nAllocLimit)
{
	tINT nLen, nStrLen, nTotalRead, nAlloc;
	tCHAR *szHdrFmt = "%s %s HTTP/1.0\r\n";
	tCHAR szBuf[MAX_BUF*16], szWorkBuf[MAX_BUF*2], szBuf1[MAX_BUF*2];
	tINT nSocketId;
	tINT nCheck = 0;
	tCHAR *p, *q;
	tINT i;
#if defined(USE_TIMECHECK)
	struct timeval  tv1, tv2;
	double sec, usec;
#endif


	if (nAllocLimit < HTMLALLOC_LIMIT) {
		nAllocLimit = HTMLALLOC_LIMIT;
	}
	szBuf[0] = '\0';
#if defined(USE_TIMECHECK)
	gettimeofday(&tv1, NULL);
#endif
	nSocketId=SocketClientEx(szHost, nPort, nConnectTimeOut);
#if defined(USE_TIMECHECK)
	gettimeofday(&tv2, NULL);

	usec = (double)tv2.tv_usec - (double)tv1.tv_usec;
	sec = (double)tv2.tv_sec - (double)tv1.tv_sec;
	
    	if(usec < 0) {
       	 	sec = sec - 1;
       	 	usec = 1000000 + usec;
    	}
	sec += (usec/(double)1000000);
	printf("***connect : %f<br>\n", sec);
#endif

	if (nSocketId <0) {
		return -1;
	}
#if defined(USE_TIMECHECK)
	gettimeofday(&tv1, NULL);
#endif

	if (nMethod == CGI_POST_METHOD) {
		sprintf(szBuf, szHdrFmt, "POST", szPath);
		if (bVirtualHost) {
			sprintf(szBuf1, "Host: %s\r\n", szHost);
			strcat(szBuf, szBuf1);
		}
		strcat(szBuf, "User-Agent: DreamBot\r\n");
		strcpy(szWorkBuf, "Content-type: application/x-www-form-urlencoded\r\n");
		strcat(szBuf, szWorkBuf);
		nLen = strlen(szInput);
		sprintf(szWorkBuf, "Content-length: %d\r\n\r\n", nLen);
		strcat(szBuf, szWorkBuf);
		strcat(szBuf, szInput);
	}
	else if (nMethod == CGI_GET_METHOD) {
		sprintf(szBuf, szHdrFmt, "GET", szPath);
		if (bVirtualHost) {
			sprintf(szBuf1, "Host: %s\r\n", szHost);
			strcat(szBuf, szBuf1);
		}
		strcat(szBuf, "User-Agent: DreamBot\r\n\r\n");
	} 
	else {
		sprintf(szBuf, szHdrFmt, "HEAD", szPath);
		if (bVirtualHost) {
			sprintf(szBuf1, "Host: %s\r\n", szHost);
			strcat(szBuf, szBuf1);
		}
		strcat(szBuf, "User-Agent: DreamBot\r\n\r\n");
	}

	// send query string
	if (SocketWrite(nSocketId, (tBYTE *)szBuf, strlen(szBuf), 5) < (tINT)strlen(szBuf)) return (-2);

	nTotalRead = 0;

	*RetBuf = NULL;
	*RetBuf = (tBYTE *)malloc(nAllocLimit);
	nAlloc = nAllocLimit;

	i = 0;
	for(;;) {
		nStrLen = SocketRead(nSocketId, (tBYTE *)szBuf, MAX_BUF*16-1, nReadTimeOut);
		if (nStrLen <= 0) {
			break;
		}
		else {
			szBuf[nStrLen] = '\0';
			if (nCheck == 0) { // OK인지 아닌지.. 확인..
				if (strstr(szBuf, "200 OK")) { // 있다.. 해당되는 것이....
					nCheck = 1;
				}
				else {
					break;
				}
			}
			if (nTotalRead + nStrLen >= nAlloc) {
				*RetBuf = (tBYTE *)realloc(*RetBuf, nAlloc + nAllocLimit);
				if (*RetBuf == NULL) { // exception
					nTotalRead = 0;
					break;
				}
				nAlloc += nAllocLimit;
			}
			memcpy(*RetBuf + nTotalRead, szBuf, nStrLen);
			nTotalRead += nStrLen;
		}
		i ++;	

	}

	//printf("[%s]<br>\n", szBuf);

	// close socket
	close(nSocketId);
	*(*RetBuf + nTotalRead) = '\0';
	//printf("count : %d<br>\n", i);
#if defined(USE_TIMECHECK)
	gettimeofday(&tv2, NULL);
	usec = (double)tv2.tv_usec - (double)tv1.tv_usec;
	sec = (double)tv2.tv_sec - (double)tv1.tv_sec;
	
    	if(usec < 0) {
       	 	sec = sec - 1;
       	 	usec = 1000000 + usec;
    	}
	sec += (usec/(double)1000000);
	printf("***get : %f<br>\n", sec);


	gettimeofday(&tv1, NULL);
#endif

	if (nTotalRead) {
		if (nMethod == CGI_HEAD_METHOD) {
			p = strchr((tCHAR *)(*RetBuf), '\n');
			if (p == NULL) {
				nTotalRead = 0;
			}
			else {
				nStrLen = p - (tCHAR *)(*RetBuf) + 1;
				nTotalRead -= nStrLen;
				memcpy(*RetBuf, p+1, nTotalRead);
			}
		}
		else {
			p = (tCHAR *)(*RetBuf);
			while (*p) {
				q = strchr(p, '\n');
				if (q == NULL) {
					nTotalRead = 0;
					break;
				}
				else {
					*q = '\0';
					if (strlen(p) <= 1) {
						nStrLen = q - (tCHAR *)(*RetBuf) + 1;
						nTotalRead -= nStrLen;
						memcpy(*RetBuf, q+1, nTotalRead);
						break;
					}
					else {
						*q = '\n';
						p = q+1;
					}
				}
			}
		}
	}

#if defined(USE_TIMECHECK)
	gettimeofday(&tv2, NULL);
	usec = (double)tv2.tv_usec - (double)tv1.tv_usec;
	sec = (double)tv2.tv_sec - (double)tv1.tv_sec;
	
    	if(usec < 0) {
       	 	sec = sec - 1;
       	 	usec = 1000000 + usec;
    	}
	sec += (usec/(double)1000000);
	printf("***parsing : %f<br>\n", sec);
#endif
	if (nTotalRead) {
		if (bBin == FALSE) *(*RetBuf + nTotalRead) = '\0';
		return nTotalRead;
	}
	else {
		if (*RetBuf) free(*RetBuf);
	}
	return 0;

} 

tINT GetHtmlEx(tCHAR *szHost, tCHAR *szPath, tINT nPort, tINT nMethod, tCHAR *szInput, tBYTE **RetBuf, tCHAR *stMark, tCHAR *endMark, tBOOL bVirtualHost, tINT nTimeOut)
{
	tINT nLen, nStrLen, nTotalRead, nAlloc;
	tCHAR *szHdrFmt = "%s %s HTTP/1.0\r\n";
	tCHAR szBuf[MAX_BUF*4], szWorkBuf[MAX_BUF], *p, szBuf1[MAX_BUF];
	tINT nSocketId;
	tINT nCheck = 0;
	tBOOL bSt = FALSE, bEnd = FALSE;

	szBuf[0] = '\0';
	nSocketId=SocketClientEx(szHost, nPort, nTimeOut);
	if (nSocketId <0) {
		return 0;
	}

	if (nMethod == CGI_POST_METHOD) {
		sprintf(szBuf, szHdrFmt, "POST", szPath);
		if (bVirtualHost) {
                        sprintf(szBuf1, "Host: %s\r\n", szHost);
                        strcat(szBuf, szBuf1);
                }
		strcat(szBuf, "User-Agent: DreamBot\r\n");
		strcpy(szWorkBuf, "Content-type: application/x-www-form-urlencoded\r\n");
		strcat(szBuf, szWorkBuf);
		nLen = strlen(szInput);
		sprintf(szWorkBuf, "Content-length: %d\r\n\r\n", nLen);
		strcat(szBuf, szWorkBuf);
		strcat(szBuf, szInput);
	}
	else if (nMethod == CGI_GET_METHOD) {
		sprintf(szBuf, szHdrFmt, "GET", szPath);
		if (bVirtualHost) {
                        sprintf(szBuf1, "Host: %s\r\n", szHost);
                        strcat(szBuf, szBuf1);
                }
		strcat(szBuf, "User-Agent: DreamBot\r\n\r\n");
	} 
	else {
		sprintf(szBuf, szHdrFmt, "HEAD", szPath);
		if (bVirtualHost) {
                        sprintf(szBuf1, "Host: %s\r\n", szHost);
                        strcat(szBuf, szBuf1);
                }
		strcat(szBuf, "User-Agent: DreamBot\r\n\r\n");
	}

	// send query string
	if (SocketWrite(nSocketId, (tBYTE *)szBuf, (tINT)strlen(szBuf), 5) < (tINT)strlen(szBuf)) return 0;

	nTotalRead = 0;

	*RetBuf = NULL;
	*RetBuf = (tBYTE *)malloc(HTMLALLOC_LIMIT);
	nAlloc = HTMLALLOC_LIMIT;

	for(;;) {
		nStrLen = SocketGets(nSocketId, (tBYTE *)szBuf, MAX_BUF-1, nTimeOut);
		if (nStrLen <= 0) {
			break;
		}
		else {
			szBuf[nStrLen] = '\0';
			if (nCheck == 0) { // OK인지 아닌지.. 확인..
				if (strstr(szBuf, "200 OK")) { // 있다.. 해당되는 것이....
					if (nMethod == CGI_HEAD_METHOD) {
						nCheck = 2;
					}
					else {
						nCheck = 1;
					}
				}
				else {
					break;
				}
				continue;
				
			}
			else if (nCheck == 1) { // Return값 확인..
				if (szBuf[0] == '\n' || szBuf[0] == '\r') { // 이제부터 시작이지..
					nCheck = 2;
				}
				continue;
			}

			if (bSt == FALSE) {
				p = strstr(szBuf, stMark);
				if (p) {
					strcpy(szBuf, p);
					nStrLen = strlen(szBuf);
					bSt = TRUE;
				}
				else {
					continue; // 아직 안 나왔다.
				}
			}
			else {
				p = strstr(szBuf, endMark);
				if (p) {
					*(p+strlen(endMark)) = '\0';
					nStrLen = strlen(szBuf);
					bEnd = TRUE;
				}
			}

			if (nTotalRead + nStrLen >= nAlloc) {
				*RetBuf = (tBYTE *)realloc(*RetBuf, nAlloc + HTMLALLOC_LIMIT);
				if (*RetBuf == NULL) { // exception
					nTotalRead = 0;
					break;
				}
				nAlloc += HTMLALLOC_LIMIT;
			}
			memcpy(*RetBuf + nTotalRead, szBuf, nStrLen);
			nTotalRead += nStrLen;

			if (bEnd) break;

		}
	}

	// close socket
	close(nSocketId);
	if (nTotalRead) {
		*(*RetBuf + nTotalRead) = '\0';
		return nTotalRead;
	}
	else {
		if (*RetBuf) free(*RetBuf);
	}
	return 0;

} 
