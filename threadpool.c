#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <malloc.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "dwtype.h"
#include "record.h"
#include "dwthread.h"
#include "mutex.h"
#include "threadpool.h"

MUTEXMAN_FORMAT gMutexThPool;
RECORD gTHPOOL;
tBOOL bFlagCloseServer = FALSE; // pool를 정리해야 한다.
lpProcP gfProc_ThreadFunc_THPOOL;
tINT gnSocketId_THPOOL;
tINT gnTotalThread_THPOOL;
tINT gnDelayTimeOut;

tVOID SetFlagCloseServer(tBOOL bFlag)
{
	LockMutexMan(&gMutexThPool, ID_MUTEX_CLOSE_SERVER);
	bFlagCloseServer = bFlag;
	UnLockMutexMan(&gMutexThPool, ID_MUTEX_CLOSE_SERVER);
}

tBOOL GetFlagCloseServer(tVOID)
{
	tBOOL bFlag;

	LockMutexMan(&gMutexThPool, ID_MUTEX_CLOSE_SERVER);
	bFlag = bFlagCloseServer;
	UnLockMutexMan(&gMutexThPool, ID_MUTEX_CLOSE_SERVER);

	return bFlag;
}


tBOOL InitThPool(tINT nCountThread, lpProcP Proc_ThreadFunc, tINT nSocketId, tINT nDelayTimeOut)
{
	BeginMutexMan(&gMutexThPool, C_MUTEX_THPOOL);
	bFlagCloseServer = FALSE;
	gfProc_ThreadFunc_THPOOL = Proc_ThreadFunc;
	gnSocketId_THPOOL = nSocketId;
	gnTotalThread_THPOOL = nCountThread;
	gnDelayTimeOut = nDelayTimeOut;
	AllocRecord(&gTHPOOL, sizeof(THPOOL_ITEMTYPE), nCountThread, 1);
	return TRUE;
}

tVOID CloseThPool(tVOID)
{
	FreeRecord(&gTHPOOL);
	EndMutexMan(&gMutexThPool);
}

tBOOL CmpProcPoolItem(THPOOL_ITEMTYPE *item1, THPOOL_ITEMTYPE *item2)
{
	return ((tINT)(item1->ThreadId)  - (tINT)(item2->ThreadId));
}

tVOID PutThPool(pthread_t ThreadId)
{
	THPOOL_ITEMTYPE PoolItem;

	LockMutexMan(&gMutexThPool, ID_MUTEX_POOL);

	PoolItem.ThreadId = ThreadId;
	gettimeofday(&(PoolItem.tv), NULL);
	PoolItem.nSocketId = -1;
	PoolItem.bActive = FALSE;

	AppendRecord(&gTHPOOL, &PoolItem);

	UnLockMutexMan(&gMutexThPool, ID_MUTEX_POOL);
}

tBOOL EmptyThPool(tVOID)
{
	THPOOL_ITEMTYPE PoolItem;
	tINT i;

	LockMutexMan(&gMutexThPool, ID_MUTEX_POOL);

	for ( i = 0 ; i < gTHPOOL.nUsed ; i ++ ) {
		GetRecord(&gTHPOOL, i, &PoolItem);
		if (PoolItem.bActive == FALSE) {
			break;
		}
	}

	UnLockMutexMan(&gMutexThPool, ID_MUTEX_POOL);
	if ( i < gTHPOOL.nUsed) return TRUE;
	return FALSE;
}

tBOOL UpdateThPool(pthread_t ThreadId, tINT nSocketId, tBOOL bActive)
{
	THPOOL_ITEMTYPE PoolItem;
	tINT nPos;

	LockMutexMan(&gMutexThPool, ID_MUTEX_POOL);
	PoolItem.ThreadId = ThreadId;

	nPos = FindRecord(&gTHPOOL, &PoolItem, FALSE, (CompProcP)CmpProcPoolItem);

	if (nPos < 0) { // 없다.
		UnLockMutexMan(&gMutexThPool, ID_MUTEX_POOL);
		return FALSE;

	}
	PoolItem.ThreadId = ThreadId;
	gettimeofday(&(PoolItem.tv), NULL);
	PoolItem.nSocketId = nSocketId;
	PoolItem.bActive = bActive;

	SetRecord(&gTHPOOL, nPos, &PoolItem);
	printf("ThreadId : %d, flag : %d set\n", (tINT)ThreadId, (tINT)bActive);


	UnLockMutexMan(&gMutexThPool, ID_MUTEX_POOL);
	return TRUE;
}

tBOOL KillThPool(tVOID)
{
	THPOOL_ITEMTYPE PoolItem;
	tINT i;
	struct timeval  tv;
	double usec, sec;

	LockMutexMan(&gMutexThPool, ID_MUTEX_POOL);

	for ( i = 0 ; i < gTHPOOL.nUsed ; i ++ ) {
		GetRecord(&gTHPOOL, i, &PoolItem);
		if (PoolItem.bActive == FALSE) {
			printf("kill %d\n", (tINT)(PoolItem.ThreadId));
        		pthread_kill(PoolItem.ThreadId, SIGQUIT);
		}
		else {
			gettimeofday(&tv, NULL);
			usec = (double)tv.tv_usec - (double)PoolItem.tv.tv_usec;
			sec = (double)tv.tv_sec - (double)PoolItem.tv.tv_sec;
			if(usec < 0) {
				sec = sec - 1;
				usec = 1000000 + usec;
			}
			sec += (usec/(double)1000000);
	
			if (sec > gnDelayTimeOut) {
				printf("delay time(%f), kill %d\n", sec, (tINT)(PoolItem.ThreadId));
        			pthread_kill(PoolItem.ThreadId, SIGQUIT);
				if (PoolItem.nSocketId) close(PoolItem.nSocketId);
			}
		}
	}

	UnLockMutexMan(&gMutexThPool, ID_MUTEX_POOL);
	return TRUE;
}

tINT RecoveryThPool(tVOID)
{
	THPOOL_ITEMTYPE PoolItem;
	tINT i;
	struct timeval  tv;
	double usec, sec;
	tINT nCount_RecoveryTh = 0;

	LockMutexMan(&gMutexThPool, ID_MUTEX_POOL);

	for ( i = 0 ; i < gTHPOOL.nUsed ; i ++ ) {
		GetRecord(&gTHPOOL, i, &PoolItem);
		if (PoolItem.bActive) {
			gettimeofday(&tv, NULL);
			usec = (double)tv.tv_usec - (double)PoolItem.tv.tv_usec;
			sec = (double)tv.tv_sec - (double)PoolItem.tv.tv_sec;
			if(usec < 0) {
				sec = sec - 1;
				usec = 1000000 + usec;
			}
			sec += (usec/(double)1000000);
	
			if (sec > gnDelayTimeOut) {
				printf("delay time(%f), kill %d\n", sec, (tINT)(PoolItem.ThreadId));
        			pthread_kill(PoolItem.ThreadId, SIGQUIT);
				if (PoolItem.nSocketId) close(PoolItem.nSocketId);
				DelRecord(&gTHPOOL, i);
				i --;
				nCount_RecoveryTh ++;
				continue;
			}
		}
	}

	UnLockMutexMan(&gMutexThPool, ID_MUTEX_POOL);

	for ( i = 0 ; i < nCount_RecoveryTh ; i ++ ) {
                if(CreateThread(NULL, gfProc_ThreadFunc_THPOOL, (tVOID *)(gnSocketId_THPOOL)) == FALSE) {
			printf("%d Thread Create Error\n", i);
			break;
                }
		else {
			printf("[recovery thread count : %d, total th count : %d] - %d Thread Create Ok\n", nCount_RecoveryTh, gnTotalThread_THPOOL, i);
		}
	}
	return (nCount_RecoveryTh);
}


tVOID WaitingThreadEndThPool(tINT nThreadNum)
{
	tINT nChkTh;

	while(1) {
		if (GetThreadNum() <= nThreadNum) {
			break;
		}
		if (GetFlagCloseServer() == FALSE) { //죽는 것이 아니니 .. recovery해야지..
			printf("recovery check\n");
			nChkTh = RecoveryThPool();
			printf("recovery count : %d\n", nChkTh);
		}
		
		ThreadTimedWait(3);
	}
}

tBOOL LockMutexSocketThPool(tVOID)
{
	return (LockMutexMan(&gMutexThPool, ID_MUTEX_SOCKET));
}

tBOOL UnLockMutexSocketThPool(tVOID)
{
	return (UnLockMutexMan(&gMutexThPool, ID_MUTEX_SOCKET));
}

