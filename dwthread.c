/*


		thread 라이브러리

		author : hyypucs
		B-Date : 1999. 9. 16.
        Comment:
*/
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "dwtype.h"
#include "mutex.h"
#include "safemutex.h"
#include "dwthread.h"
#if defined(_SAFEMEM) && defined(_MTHREAD)
#include "safemem.h"
#endif

#if defined(_MTHREAD)
MUTEXMAN_FORMAT MutexForThread;

PRIVATE tBOOL bCreateThread = TRUE;
PRIVATE tINT nNumThread = 0;
PRIVATE tBOOL bInitThread = FALSE;

#define LIMIT_THREAD 21
#endif

tVOID ThreadCancleOK(tINT bOK)
{
	tINT last_state;
	tINT last_type;

#if !defined(_FREEBSD)
	if (bOK) {
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
        	//pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &last_type);
	}
	else {
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
	}
#endif
}



tBOOL StartThreadLayer(tVOID)
{
#if defined(_MTHREAD)
	if (bInitThread) return TRUE;
	if (BeginMutexMan(&MutexForThread, MAX_MUTEX_FOR_THREAD ) == FALSE) return FALSE;
	nNumThread = 0;
	bCreateThread = TRUE;
	bInitThread = TRUE;
#endif
	return TRUE;
}

tBOOL EndThreadLayer(tVOID)
{
#if defined(_MTHREAD)
	if (bInitThread == FALSE) return TRUE;
	EndMutexMan(&MutexForThread);
	bInitThread = FALSE;
#endif
	return TRUE;
}

tBOOL GetCreateThreadFlag(tVOID)
{
#if defined(_MTHREAD)
	tBOOL bRetVal = TRUE;

	LockMutexMan(&MutexForThread, MUTEX_CREATE_THREAD);
	bRetVal = bCreateThread;
	UnLockMutexMan(&MutexForThread, MUTEX_CREATE_THREAD);
	return (bRetVal);
#else
	return TRUE;
#endif
}

tBOOL SetCreateThreadFlag(tBOOL bFlag)
{
#if defined(_MTHREAD)
	LockMutexMan(&MutexForThread, MUTEX_CREATE_THREAD);
	bCreateThread = bFlag;
	UnLockMutexMan(&MutexForThread, MUTEX_CREATE_THREAD);
	
#endif
	return TRUE;
}

tBOOL PlusThreadNum(tVOID)
{
#if defined(_MTHREAD)
	LockMutexMan(&MutexForThread, MUTEX_NUM_THREAD);
	nNumThread ++;
	UnLockMutexMan(&MutexForThread, MUTEX_NUM_THREAD);
	
#endif
	return TRUE;
}

tBOOL MinusThreadNum(tVOID)
{
#if defined(_MTHREAD)
	LockMutexMan(&MutexForThread, MUTEX_NUM_THREAD);
	if (nNumThread == 0) { // exception
		UnLockMutexMan(&MutexForThread, MUTEX_NUM_THREAD);
		return TRUE;
	}
	nNumThread --;
	UnLockMutexMan(&MutexForThread, MUTEX_NUM_THREAD);
	
#endif
	return TRUE;
}

tINT GetThreadNum(tVOID)
{
	tINT nRetVal = 0;

#if defined(_MTHREAD)
	LockMutexMan(&MutexForThread, MUTEX_NUM_THREAD);
	nRetVal = nNumThread;
	UnLockMutexMan(&MutexForThread, MUTEX_NUM_THREAD);
	
#endif
	return nRetVal;
}

tBOOL CreateThread(pthread_t *lpThreadId, lpProcP lpThreadProc, tVOID *lpParam)
{
#if defined(_MTHREAD)
	pthread_attr_t ThreadAttr;
	pthread_t ThreadId;

	if (GetCreateThreadFlag() == FALSE) {
		return FALSE;
	}
/*
	if (GetThreadNum() > LIMIT_THREAD) {
		return FALSE;
	}
*/

	pthread_attr_init(&ThreadAttr);
	pthread_attr_setdetachstate(&ThreadAttr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setscope(&ThreadAttr, PTHREAD_SCOPE_SYSTEM);

	if (lpThreadId == (pthread_t *)NULL) {
		if (pthread_create(&ThreadId, &ThreadAttr, lpThreadProc, lpParam) == 0) {
			pthread_attr_destroy(&ThreadAttr);
			PlusThreadNum();
			return TRUE;
		}
		else {
			pthread_attr_destroy(&ThreadAttr);
			return FALSE;
		}
	}
	else {
		if (pthread_create(lpThreadId, &ThreadAttr, lpThreadProc, lpParam) == 0) {
			pthread_attr_destroy(&ThreadAttr);
			PlusThreadNum();
			return TRUE;
		}
		else {
			pthread_attr_destroy(&ThreadAttr);
			return FALSE;
		}

	}

#else
	lpThreadProc(lpParam);
	return TRUE;
#endif

}

tVOID DestroyThread(tVOID *Status)
{

#if defined(_MTHREAD)
	MinusThreadNum();
	pthread_exit(Status);
#endif
}

tINT CancelThread(pthread_t ThreadId)
{

#if defined(_MTHREAD)
	MinusThreadNum();
	return(pthread_cancel(ThreadId));
#endif
}

tBOOL ThreadTimedWait(tINT nSecond)
{
#if defined(_MTHREAD)
	pthread_cond_t Condition;
	struct timespec delaytime;

	
	LockMutexMan(&MutexForThread, MUTEX_WAIT_THREAD);
	delaytime.tv_sec = time(NULL) + nSecond;
	delaytime.tv_nsec = 0;

	if (pthread_cond_init(&Condition, NULL) != 0) {
		UnLockMutexMan(&MutexForThread, MUTEX_WAIT_THREAD);
		return FALSE;
	}

	if (pthread_cond_timedwait(&Condition, MutexForThread.MutexList + MUTEX_WAIT_THREAD, &delaytime) != 0) {
		pthread_cond_destroy(&Condition);
		UnLockMutexMan(&MutexForThread, MUTEX_WAIT_THREAD);
		return FALSE;
	}
	pthread_cond_destroy(&Condition);
	UnLockMutexMan(&MutexForThread, MUTEX_WAIT_THREAD);

	return TRUE;
#else
	return TRUE;
#endif
}

tVOID WaitingThreadEnd(tINT nThreadNum)
{
#if defined(_MTHREAD)
        while(1) {
		if (GetThreadNum() <= nThreadNum) {
			break;
		}

                ThreadTimedWait(3);
	}
#else
	;
#endif
}

