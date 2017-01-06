/*

		thread library

		B-Date : 1999. 9. 16.
        Comment:
*/
#if !defined(__DWTHREAD_H__)
#define __DWTHREAD_H__

#define MUTEX_CREATE_THREAD  	0	
#define MUTEX_NUM_THREAD  	1
#define MUTEX_WAIT_THREAD	2
#define MAX_MUTEX_FOR_THREAD  	3

tBOOL StartThreadLayer(tVOID);
tBOOL EndThreadLayer(tVOID);
tBOOL GetCreateThreadFlag(tVOID);
tBOOL SetCreateThreadFlag(tBOOL bFlag);
tBOOL PlusThreadNum(tVOID);
tBOOL MinusThreadNum(tVOID);
tINT GetThreadNum(tVOID);
tBOOL CreateThread(pthread_t *lpThreadId, lpProcP lpThreadProc, tVOID *lpParam);
tVOID DestroyThread(tVOID *Status);
tINT CancelThread(pthread_t ThreadId);
tBOOL ThreadTimedWait(tINT nSecond);
tVOID WaitingThreadEnd(tINT nThreadNum);
// thread cancle 가능한지??/
tVOID ThreadCancleOK(tINT bOK);

#endif /* dwthread.h */
