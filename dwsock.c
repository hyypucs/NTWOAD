/*

		Author : hyypucs

		B-Date : 1999. 9. 16

        Comment:
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "dwtype.h"
#include "util.h"
#include "dwsock.h"

PRIVATE void sig_alrm(int signo);
int Readable_TimeO(tINT fd, tINT nSec);
int Writable_TimeO(tINT fd, tINT nSec);

tINT SocketClient(tCHAR *szHostName, tINT nPort)
{
	struct sockaddr_in sin;
	tINT nSocketId;
	tINT i;
	tCHAR szHost[MAX_PATH];
	tBOOL bIsDigit = TRUE;
	struct hostent *hptr;
	struct in_addr **pptr;

	if (szHostName == NULL || szHostName[0] == '\0') return -1;

	strcpy(szHost, szHostName);
	for ( i = 0 ; i < (tINT)strlen(szHost) ; i ++ ) {
		if (!isdigit((tINT)szHost[i])  && szHost[i] != '.') {
			//
			// host가 hostname를 가져와야 한다.
  			//
			bIsDigit = FALSE;
			break;
		}
	}

	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((uint16_t)nPort);

	if (bIsDigit == FALSE) { // hostname에 의한 address세팅..
		if ((hptr = gethostbyname(szHost)) == NULL) { // hostname못 가져왔다.
			return -1;
		}
		pptr = (struct in_addr **) hptr->h_addr_list; // 첫번째것만 하면 된다.


		for ( ; *pptr != NULL ; pptr ++) {
			memcpy(&(sin.sin_addr), *pptr , sizeof(struct in_addr));
			nSocketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (connect(nSocketId, (struct sockaddr *)&sin, sizeof(sin)) == 0) {
				return nSocketId;
			}
			close(nSocketId);
		}
	}
	else {
		sin.sin_addr.s_addr = inet_addr(szHost);

		nSocketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (connect(nSocketId, (struct sockaddr *)&sin, sizeof(sin)) == 0) {
			return nSocketId;
		}
		close(nSocketId);
	}

	return -1;
}

tINT SocketClientEx(tCHAR *szHostName, tINT nPort, tINT nTimeOutSec)
{
	struct sockaddr_in sin;
	tINT nSocketId;
	tINT i;
	tCHAR szHost[MAX_PATH];
	tBOOL bIsDigit = TRUE;
	struct hostent *hptr;
	struct in_addr **pptr;
	SigProcP SigFunc = NULL;
	tINT nRetSocket = -1;

	if (szHostName == NULL || szHostName[0] == '\0') return -1;

	strcpy(szHost, szHostName);
	for ( i = 0 ; i < (tINT)strlen(szHost) ; i ++ ) {
		if (!isdigit((tINT)szHost[i])  && szHost[i] != '.') {
			//
			// host가 hostname를 가져와야 한다.
  			//
			bIsDigit = FALSE;
			break;
		}
	}

	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((uint16_t)nPort);

	if (nTimeOutSec > 0) {
		SigFunc = (SigProcP)RegistOneSig(SIGALRM, (SigProcP)sig_alrm);
		if (alarm(nTimeOutSec) != 0) {
			RegistOneSig(SIGALRM, SigFunc);
			return (-1);
		}
	}

	if (bIsDigit == FALSE) { // hostname에 의한 address세팅..
		if ((hptr = gethostbyname(szHost)) == NULL) { // hostname못 가져왔다.
			if (nTimeOutSec > 0) {
				RegistOneSig(SIGALRM, SigFunc);
			}
			return -1;
		}
		pptr = (struct in_addr **) hptr->h_addr_list; // 첫번째것만 하면 된다.


		for ( ; *pptr != NULL ; pptr ++) {
			memcpy(&(sin.sin_addr), *pptr , sizeof(struct in_addr));
			nSocketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (connect(nSocketId, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
				close(nSocketId);
				if (errno == EINTR) errno = ETIMEDOUT;
				nRetSocket = -1;
			}
			else {
				nRetSocket = nSocketId;
			}
			break;
		}
	}
	else {
		sin.sin_addr.s_addr = inet_addr(szHost);

		nSocketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (connect(nSocketId, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
			close(nSocketId);
			if (errno == EINTR) errno = ETIMEDOUT;
			nRetSocket = -1;
		}
		else {
			nRetSocket = nSocketId;
		}
	}
	if (nTimeOutSec > 0) {
		alarm(0);
		RegistOneSig(SIGALRM, SigFunc);
	}
	return (nRetSocket);
}

tINT SocketServer(tCHAR *szHost, tINT nPort) 
{
	struct sockaddr_in sin;
	tINT nSocketId;
	tINT nOptVal;
	//struct timeval TimeVal;
/*
	tINT i;
        tCHAR szHost[MAX_PATH];
        tBOOL bIsDigit = TRUE;
        struct hostent *hptr;
        struct in_addr **pptr;
*/

	if (szHost == NULL || szHost[0] == '\0') return -1;


/*
	for ( i = 0 ; i < strlen(szHost) ; i ++ ) {
                if (!isdigit((tINT)szHost[i])  && szHost[i] != '.') {
                        //
                        // host가 hostname를 가져와야 한다.
                        //
                        bIsDigit = FALSE;
                        break;
                }
        }
*/

// server는 호스트가 필요없지..


	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_family = AF_INET;
	sin.sin_port = htons((uint16_t)nPort);

	nSocketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	nOptVal = 1;

	if (setsockopt(nSocketId, SOL_SOCKET, SO_REUSEADDR, (const void *)(&nOptVal), sizeof(tINT)) == -1) {
		close(nSocketId);
		return -1;
	}
#if defined(_OLD)
	TimeVal.tv_sec = 5;
	TimeVal.tv_usec = 0;
	if (setsockopt(nSocketId, SOL_SOCKET, SO_RCVTIMEO, (const void *)(&TimeVal), sizeof(TimeVal)) == -1) {
		printf("RCVTIMEO Error\n");
		close(nSocketId);
		return -1;
	}
	TimeVal.tv_sec = 5;
	TimeVal.tv_usec = 0;
	if (setsockopt(nSocketId, SOL_SOCKET, SO_SNDTIMEO, (const void *)(&TimeVal), sizeof(TimeVal)) == -1) {
		printf("SNDTIMEO Error\n");
		close(nSocketId);
		return -1;
	}
#endif

	if (bind(nSocketId, (struct sockaddr *)(&sin), sizeof(sin)) == -1) {
		close(nSocketId);
		return -1;
	}

	if (listen(nSocketId, SOMAXCONN) == -1) {
		close(nSocketId);
		return -1;
	}

	return (nSocketId);
}

int Readable_TimeO(tINT fd, tINT nSec)
{
	fd_set rset;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	tv.tv_sec = nSec;
	tv.tv_usec = 0;
	return (select(fd+1, &rset, NULL, NULL, &tv));
	// > 0 readble
}

int Writable_TimeO(tINT fd, tINT nSec)
{
	fd_set wset;
	struct timeval tv;

	FD_ZERO(&wset);
	FD_SET(fd, &wset);
	tv.tv_sec = nSec;
	tv.tv_usec = 0;
	return (select(fd+1, NULL, &wset, NULL, &tv));
	// > 0 writeable
}

int ChkSocket(tINT nSocketId)
{
	fd_set rset;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(nSocketId, &rset);
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	return (select(nSocketId+1, NULL, &rset, NULL, &tv));
}

PRIVATE tVOID sig_alrm(tINT nSig)
{
	return;
}

//
// '\n' '\r'를 만나면.. 그만 읽는다.
//
tINT SocketGets(tINT nSocketId, tBYTE *szStream, tINT nMax, tINT nTimeOut) 
{

	tINT nSize = nMax, nRead, nTRead = 0;
	tCHAR *p = (tCHAR *)szStream;
	tBOOL bUseAlarm = TRUE;

#if defined(_USE_ALARM)

	if (nTimeOut <= 0) {
		bUseAlarm = FALSE;
	}
	if (bUseAlarm)
		RegistOneSig(SIGALRM, (SigProcP)sig_alrm);
#else
	bUseAlarm = FALSE;
#endif

	while (nSize > 0) {
#if defined(_USE_ALARM)
		if (bUseAlarm) alarm(nTimeOut);
#else
		if (nTimeOut>0) {
			if (Readable_TimeO(nSocketId, nTimeOut) <= 0) {
				return -1; // timeout
			}	
		}	
#endif
		nRead = recvfrom(nSocketId, p, 1, 0, NULL, NULL);
		//nRead = read(nSocketId, p, 1);  // 하나만 읽는다.
		if (nRead < 0) {
			//if (errno != EAGAIN && errno != EINTR) return -1;
			//else contiue;
			if (bUseAlarm) {
				alarm(0);
				RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
				if (errno == EINTR) return -1; // timeout
			}
			else {
				if (errno == EINTR) continue;
			}
			return (-1);
		}
		else if (nRead == 0) {
			if (bUseAlarm) {
				alarm(0);
				RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
			}
			*p = '\0';
			return (nTRead);
		}
		else if (*p == '\n') { // 끝...
			if (bUseAlarm) {
				alarm(0);
				RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
			}
			*(p+1) = '\0';
			nTRead += 1;
			return nTRead;
		}
		else {
			if (bUseAlarm) {
				alarm(0);
			}
			p += nRead;
			nSize -= nRead;
		}
		nTRead += nRead;
	}
	if (bUseAlarm) {
		alarm(0);
		RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
	}
	*p = '\0';
	return nTRead;
}

tINT SocketRead(tINT nSocketId, tBYTE *szStream, tINT nMax, tINT nTimeOut) 
{

	tINT nSize = nMax, nRead, nTRead = 0;
	tCHAR *p = (tCHAR *)szStream;
	tBOOL bUseAlarm = TRUE;

#if defined(_USE_ALARM)
	if (nTimeOut <= 0) {
		bUseAlarm = FALSE;
	}
	if (bUseAlarm) {
		RegistOneSig(SIGALRM, (SigProcP)sig_alrm);
	}
#else
	bUseAlarm = FALSE;
#endif

	while (nSize > 0) {
#if defined(_USE_ALARM)
		if (bUseAlarm) {
			alarm(nTimeOut);
		}
#else
		if (nTimeOut>0) {
                        if (Readable_TimeO(nSocketId, nTimeOut) <= 0) {
                                return -1; // timeout
                        }
                }
#endif
		//nRead = read(nSocketId, p, nSize); 
		nRead = recvfrom(nSocketId, p, nSize, 0, NULL, NULL);
		if (nRead < 0) {
			//if (errno != EAGAIN && errno != EINTR) return -1;
			//else contiue;
			if (bUseAlarm) {
				alarm(0);
				RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
				if (errno == EINTR) return -1; // timeoutcontinue;
			}
			else {
				if (errno == EINTR) continue;
			}
			return (-1);
		}
		else if (nRead == 0) {
			if (bUseAlarm) {
				alarm(0);
				RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
			}
			return (nTRead);
		}
		else {
			if (bUseAlarm) {
				alarm(0);
			}
			p += nRead;
			nSize -= nRead;
		}
		nTRead += nRead;
	}

	if (bUseAlarm) {
		alarm(0);
		RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
	}
	return nTRead;
}

tINT SocketWrite(tINT nSocketId, tBYTE *szStream, tINT nMax, tINT nTimeOut) 
{

	tINT nSize = nMax, nWrite, nTWrite = 0;
	tCHAR *p = (tCHAR *)szStream;

	tBOOL bUseAlarm = TRUE;

#if defined(_USE_ALARM)
	if (nTimeOut <= 0) {
		bUseAlarm = FALSE;
	}
	if (bUseAlarm) {
		RegistOneSig(SIGALRM, (SigProcP)sig_alrm);
	}
#else
	bUseAlarm = FALSE;
#endif
	while (nSize > 0) {
		if (bUseAlarm) {
			alarm(nTimeOut);
		}
	//	nWrite = write(nSocketId, p, nSize); 
//onst struct sockaddr *to, socklen_t tolen
		nWrite = sendto(nSocketId, p, nSize, 0, (struct sockaddr *)NULL, (socklen_t)NULL);	
		if (nWrite < 0) {
			//if (errno != EAGAIN && errno != EINTR) return -1;
			//else continue;
			if (bUseAlarm) {
				alarm(0);
				RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
				if (errno == EINTR) return -1;
			}
			else {
				if (errno == EINTR) continue;
			}
			return (-1);
		}
		else if (nWrite == 0) {
			if (bUseAlarm) {
				alarm(0);
				RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
			}
			return (nTWrite);
		}
		else {
			if (bUseAlarm) {
				alarm(0);
			}
			p += nWrite;
			nSize -= nWrite;
		}
		nTWrite += nWrite;
	}
	if (bUseAlarm) {
		alarm(0);
		RegistOneSig(SIGALRM, (SigProcP)SIG_IGN);
	}

	return nTWrite;
}

tBOOL SocketReadStream(tINT nSocketId, tBYTE *szStream, tINT nLen, tINT nTimeOut)
{
	tINT nSize, nChk;

	if ((nChk = SocketRead(nSocketId, (tBYTE *)(&nSize), sizeof(tINT), nTimeOut)) == -1) {
		return FALSE;
	}
	if (nChk != sizeof(tINT)) return FALSE;

	nSize = DwordCvt(nSize);

	if (nSize > nLen || nSize <= 0) { // exception
		return FALSE;
	}

	if ((nChk=SocketRead(nSocketId, szStream, nSize, nTimeOut)) == -1) {
		return FALSE;
	}
	if (nChk != nSize) return FALSE;

	return TRUE;
}

tBOOL SocketWriteStream(tINT nSocketId, tBYTE *szStream, tINT nLen, tINT nTimeOut)
{
	tINT nSize = nLen, nChk;

	if (nLen < 0) return FALSE;

	nSize = DwordCvt(nSize);

	if ((nChk=SocketWrite(nSocketId, (tBYTE *)(&nSize), sizeof(tINT), nTimeOut)) == -1) {
		nSize = DwordCvt(nSize);
		return FALSE;
	}
	if (nChk != sizeof(tINT)) {
		nSize = DwordCvt(nSize); // big endian 에서 해야한다.
		return FALSE;
	}

	nSize = DwordCvt(nSize); // big endian 에서 해야한다.

	if (nSize == 0) return TRUE; // write할 필요가 없다.

	if ((nChk = SocketWrite(nSocketId, szStream, nSize, nTimeOut)) == -1) {
		return FALSE;
	}
	if (nChk != nSize) return FALSE;

	return TRUE;
}

tBOOL SocketReadStreamByMem(tINT nSocketId, tVOID **lpStream, tINT *nRetRead, tINT nTimeOut)
{
	tINT nSize, nChk;

	if ((nChk = SocketRead(nSocketId, (tBYTE *)(&nSize), sizeof(tINT), nTimeOut)) == -1) {
		return FALSE;
	}
	if (nChk != sizeof(tINT)) return FALSE;

	nSize = DwordCvt(nSize);
	if (nSize <= 0) {
		*nRetRead = 0;
		*lpStream = NULL;
		return FALSE;
	}

	*nRetRead = nSize;
	*lpStream = NULL;
	*lpStream = (tBYTE *)malloc(nSize);
	if (*lpStream == NULL) return FALSE;

	if ((nChk = SocketRead(nSocketId, (tBYTE *)(*lpStream), nSize, nTimeOut)) == -1) {
		free(*lpStream);
		*lpStream=NULL;
		return FALSE;
	}
	if (nChk != nSize) {
		free(*lpStream);
		*lpStream=NULL;
		return FALSE;
	}

	return TRUE;
}

