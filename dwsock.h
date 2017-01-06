/*

		Author :  
				 hyypucs
		B-Date : 1999. 9. 16.
        Comment:
*/
#if !defined(__DWSOCK_H__)

#define __DWSOCK_H__

tINT SocketClient(tCHAR *szHost, tINT nPort);
tINT SocketClientEx(tCHAR *szHost, tINT nPort, tINT nTimeOutSec);
tINT SocketServer(tCHAR *szHost, tINT nPort);
// fgets랑 비슷한 역할..
tINT SocketGets(tINT nSocketId, tBYTE *szStream, tINT nMax, tINT nTimeOut);
tINT SocketRead(tINT nSocketId, tBYTE *szStream, tINT nMax, tINT nTimeOut);
tINT SocketWrite(tINT nSocketId, tBYTE *szStream, tINT nMax, tINT nTimeOut);
tBOOL SocketReadStream(tINT nSocketId, tBYTE *szStream, tINT nLen, tINT nTimeOut);
tBOOL SocketReadStreamByMem(tINT nSocketId, tVOID **lpStream, tINT *nRetRead, tINT nTimeOut);
tBOOL SocketWriteStream(tINT nSocketId, tBYTE *szStream, tINT nLen, tINT nTimeOut);
// 소켓이 살아있는지..
int ChkSocket(tINT nSocketId);

#endif /* dwsock.h */

