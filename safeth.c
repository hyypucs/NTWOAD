/*
		Safe Thread 하기 위해서..
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

#include <unistd.h>
#include <errno.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
//#include <sys/filio.h>
#include <sys/time.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "dwtype.h"
#include "record.h"
#include "dwsock.h"
#include "dwthread.h"
#include "tthread.h"
#include "mutex.h"

 //    unsigned int alarm(unsigned int sec);

typedef struct {
        time_t s_time;
        pthread_t ThreadId;
	tINT nSocketId;
	tBOOL bBusy; // busy이면 그냥 나 둔다.
}SAFETH_FORMAT, *LPSAFETH_FORMAT;

#if defined(_DEBUG_ME)
#define DEFAULT_IDLE_TIME       1000
#define DELAY_SEC               1000
#else
#define DEFAULT_IDLE_TIME       7
#define DELAY_SEC               5
#endif

#if defined(_USE_SAFETH)
PRIVATE RECORD RecordThTime;
PRIVATE tINT nIdleTimeSec = DEFAULT_IDLE_TIME;
//PRIVATE pthread_mutex_t ThIdleMutex;
PRIVATE pthread_mutex_t *lpMutexTThreadIdle;
PRIVATE pthread_cond_t *lpCondVarIdle;

//PRIVATE MUTEXMAN_FORMAT ThIdleMutexMan;
PRIVATE pthread_mutex_t gCheckTh;
PRIVATE tBOOL bStSafeTh = FALSE;

PRIVATE pthread_mutex_t gSigMutex;
PRIVATE pthread_cond_t gSigCond;
#endif

tINT Comp_ThreadId(SAFETH_FORMAT *item1, SAFETH_FORMAT *item2);
tVOID CheckIdleTimeTh(tVOID *lpParam);

tINT Comp_ThreadId(SAFETH_FORMAT *item1, SAFETH_FORMAT *item2)
{
	return ((tDWORD)(item1->ThreadId) - (tDWORD)(item2->ThreadId));
}

tBOOL StartSafeTh(tBOOL bTimerTh)
{
//	pthread_t *worker;

#if defined(_USE_SAFETH)
	if (AllocRecord(&RecordThTime, sizeof(SAFETH_FORMAT), 32, 32) == FALSE) { 

		printf("create thread error..\n");
		return FALSE;
	}
	if (pthread_mutex_init(&gCheckTh, NULL) != 0) {
		printf("mutex error\n");
		FreeRecord(&RecordThTime);
	}
	if (pthread_mutex_init(&gSigMutex, NULL) != 0) {
		printf("mutex error\n");
		pthread_mutex_destroy(&gCheckTh);
		FreeRecord(&RecordThTime);
	}
	if (pthread_cond_init(&gSigCond, NULL) != 0) {
		printf("cond error\n");
		pthread_mutex_destroy(&gCheckTh);
		pthread_mutex_destroy(&gSigMutex);
		
		FreeRecord(&RecordThTime);
	}
	
	if (bTimerTh) {

	//worker = (pthread_t *)malloc(sizeof(pthread_t));
	if (CreateTThread( NULL/*worker*/, &lpMutexTThreadIdle, &lpCondVarIdle, DELAY_SEC, (lpProcP) CheckIdleTimeTh, NULL) == FALSE) {
	//	free(worker);
                printf("Error Timer Thread\n");
		//EndMutexMan(&ThIdleMutexMan);
		FreeRecord(&RecordThTime);
		pthread_mutex_destroy(&gCheckTh);
		pthread_mutex_destroy(&gSigMutex);
		pthread_cond_destroy(&gSigCond);
		return FALSE;
        }
	//pthread_detach(*worker);
	//printf("Safe Thread ID : %d\n", *worker);
	//free(worker);
	}
	bStSafeTh = TRUE;
#endif
	return TRUE;
}

tBOOL EndSafeTh(tBOOL bTimerTh)
{
#if defined(_USE_SAFETH)
	if (!bStSafeTh) return TRUE;

	if (pthread_mutex_lock(&gCheckTh) != 0) {
		return FALSE;
	}
	if (bTimerTh) DestroyTThread(lpMutexTThreadIdle, lpCondVarIdle);
	FreeRecord(&RecordThTime);
	pthread_mutex_unlock(&gCheckTh);
	pthread_mutex_destroy(&gCheckTh);
	pthread_mutex_destroy(&gSigMutex);
	pthread_cond_destroy(&gSigCond);
	bStSafeTh = FALSE;
#endif
	return TRUE;
}

tBOOL OKKillThreadSig(tVOID)
{
#if defined(_USE_SAFETH)
	if (!bStSafeTh) return TRUE;
	pthread_mutex_lock(&gSigMutex);
        pthread_cond_signal(&gSigCond);
        pthread_mutex_unlock(&gSigMutex);
#endif
	return TRUE;
}


tVOID CheckIdleTimeTh(tVOID *lpParam)
{
#if defined(_USE_SAFETH)
	tINT i;
	SAFETH_FORMAT SafeItem;
	time_t t = time(NULL); // 현재 시간..
	double dTime;
	tBOOL bCont = FALSE;
	tINT nCount = 0;
	tINT last_state;

	if (!bStSafeTh) return;
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckTh);
	pthread_mutex_lock(&gCheckTh);

	i = 0;
        while( i < RecordThTime.nUsed ) {
		GetRecord(&RecordThTime, i, &SafeItem);
		if (SafeItem.bBusy) {
			//printf("%d busy\n", SafeItem.ThreadId);
			i ++;
			continue;
		}

		t = time(NULL); // 현재 시간..

		dTime = difftime(t, SafeItem.s_time);
		if (dTime >= (double)nIdleTimeSec) {
			printf("CancelThread : %d-%d\n", SafeItem.ThreadId, SafeItem.nSocketId);
/*
			if (CancelThread(SafeItem.ThreadId) != 0) {
				printf("CancelThread Error.. %d\n", errno);
			}
*/
			pthread_kill(SafeItem.ThreadId, SIGUSR1);
			// 잘 갔는지.. 기다려야지..
			pthread_mutex_lock(&gSigMutex);
               		pthread_cond_wait(&gSigCond, &gSigMutex);
                	pthread_mutex_unlock(&gSigMutex);
			printf("OK Kill\n");

#if defined(_USE_SAFEMEM)
   	   		DelSafeMem(SafeItem.ThreadId,  TRUE); // memory를 다 free시키자.
#endif
			DelSafeMutex(SafeItem.ThreadId, TRUE);

			if (SafeItem.nSocketId >= 0) close(SafeItem.nSocketId); // socket close해야지..
			if (DelRecord(&RecordThTime, i)) { // 먼저 지워야지 다음에 안하지..
				continue;
			}
		}
		i++;
	}

/*
	if (pthread_mutex_unlock(&gCheckTh) != 0) {
		printf("unlock MUTEX_IDLE error...\n");
	}
*/
	pthread_cleanup_pop(1);
#endif
}

tBOOL InsertThTime(pthread_t ThreadId, tINT nSocketId)
{
	tBOOL bRetVal = TRUE;
#if defined(_USE_SAFETH)
	tINT nItem;
	SAFETH_FORMAT SafeItem;
	time_t t = time(NULL); // 현재 시간..
	tINT last_state;

	if (!bStSafeTh) return TRUE;

	//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckTh);
	pthread_mutex_lock(&gCheckTh);
//	printf("I(%d) : M : Out\n", ThreadId);

	memset(&SafeItem, 0, sizeof(SAFETH_FORMAT));

	SafeItem.ThreadId = ThreadId;
	SafeItem.nSocketId = nSocketId;

	nItem = FindRecord(&RecordThTime,  &SafeItem, FALSE, (CompProcP)Comp_ThreadId);
	if (nItem == -1) { // 새로운 thread
		t = time(NULL);
                memcpy(&(SafeItem.s_time), &t, sizeof(time_t));
		AppendRecord(&RecordThTime, &SafeItem);
	}
	else {
		GetRecord(&RecordThTime, nItem, &SafeItem);
		t = time(NULL);
                memcpy(&(SafeItem.s_time), &t, sizeof(time_t));
		SetRecord(&RecordThTime, nItem, &SafeItem);

		//printf("EI(%d) : UM : In\n", ThreadId);
		bRetVal = FALSE;
		goto EndProc;
	}
	
EndProc:;
	//printf("I(%d) : UM : In\n", ThreadId);
	//pthread_mutex_unlock(&gCheckTh);
	pthread_cleanup_pop(1);
	//printf("I(%d) : UM : Out\n", ThreadId);
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
#endif
	return bRetVal;
}

tBOOL UpdateThTime(pthread_t ThreadId)
{
	tBOOL bRetVal = TRUE;
#if defined(_USE_SAFETH)
	tINT nItem;
	SAFETH_FORMAT SafeItem;
	time_t t = time(NULL);
	tINT last_state;

	if (!bStSafeTh) return TRUE;
	//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckTh);
	pthread_mutex_lock(&gCheckTh) ;

	SafeItem.ThreadId = ThreadId;

        nItem = FindRecord(&RecordThTime,  &SafeItem, TRUE, (CompProcP)Comp_ThreadId);
        if (nItem == -1) { // 새로운 thread
		bRetVal = FALSE;
		goto EndProc;
        }
        else {
                t = time(NULL); // 현재 시간..
                GetRecord(&RecordThTime, nItem, &SafeItem);
                memcpy(&(SafeItem.s_time), &t, sizeof(time_t));
                SetRecord(&RecordThTime, nItem, &SafeItem);
        }

EndProc:;
	//pthread_mutex_unlock(&gCheckTh);
	pthread_cleanup_pop(1);
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
#endif
	return bRetVal;
}

tBOOL SetThTimeBusy(pthread_t ThreadId, tBOOL bBusy)
{
	tBOOL bRetVal = TRUE;
#if defined(_USE_SAFETH)
	tINT nItem;
	SAFETH_FORMAT SafeItem;
	time_t t = time(NULL);
	tINT last_state;

	if (!bStSafeTh) return TRUE;
	//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckTh);
	pthread_mutex_lock(&gCheckTh);

	SafeItem.ThreadId = ThreadId;

        nItem = FindRecord(&RecordThTime,  &SafeItem, TRUE, (CompProcP)Comp_ThreadId);
        if (nItem == -1) { // 새로운 thread
		bRetVal = FALSE;
		goto EndProc;
        }
        else {
                t = time(NULL); // 현재 시간..
                GetRecord(&RecordThTime, nItem, &SafeItem);
                memcpy(&(SafeItem.s_time), &t, sizeof(time_t));
                SafeItem.bBusy = bBusy;
                SetRecord(&RecordThTime, nItem, &SafeItem);
        }

EndProc:;
	//pthread_mutex_unlock(&gCheckTh);
	pthread_cleanup_pop(1);
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
#endif
	return bRetVal;
}

tBOOL DeleteThTime(pthread_t ThreadId)
{
#if defined(_USE_SAFETH)
	tINT nItem;
	SAFETH_FORMAT SafeItem;
	struct itimerval itimer;
	tINT last_state;

	if (!bStSafeTh) return TRUE;
	//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckTh);
	pthread_mutex_lock(&gCheckTh);
	SafeItem.ThreadId = ThreadId;

	nItem = FindRecord(&RecordThTime,  &SafeItem, FALSE, (CompProcP)Comp_ThreadId);
	if (nItem == -1) { // 새로운 thread
		printf("not found.... maybe delete ... %d\n", ThreadId);
	}
	else {
		DelRecord(&RecordThTime, nItem);
	}


	//pthread_mutex_unlock(&gCheckTh);
	pthread_cleanup_pop(1);
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
#endif

	return TRUE;
}

tBOOL SetIdleTime(tINT nSec)
{
#if defined(_USE_SAFETH)
	tINT tmpSec;
	tINT last_state;

	if (!bStSafeTh) return TRUE;
	//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckTh);
	pthread_mutex_lock(&gCheckTh);

	tmpSec = nIdleTimeSec;
	nIdleTimeSec = nSec;
	
	pthread_cleanup_pop(1);
	//pthread_mutex_unlock(&gCheckTh);
//	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
#endif
	return TRUE;
}

