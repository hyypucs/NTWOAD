/*

		������ �߰��� �װ� �Ǹ� mutex�ʿ��� lock�� Ǯ���� �ʾƼ�
                ������ �Ǵ� ��찡 �߻��� �� �ִµ� �̸� ����ִ� ��ƾ
              
                Safe Mutex  

		Author : hyypucs
		B-Date : 2000. 6. 12.

        Comment:
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "dwtype.h"
#include "record.h"
#include "safemutex.h"


#define _SAFEMUTEX_MUTEX

#if defined(_USE_SAFEMUTEX)
PRIVATE RECORD gSafeMutexList;
PRIVATE tBOOL bStSafeMutex = FALSE;
#if defined(_SAFEMUTEX_MUTEX)
pthread_mutex_t gCheckMutex;
#endif
#endif

tBOOL BeginSafeMutex( tVOID )
{
#if defined(_USE_SAFEMUTEX)
	if (bStSafeMutex) return TRUE;
#if defined(_MTHREAD)
	if (AllocRecordNoSafe(&gSafeMutexList, sizeof(SAFEMUTEX_FORMAT), 100, 10) == FALSE) return FALSE;

#if defined(_SAFEMUTEX_MUTEX)
       	if (pthread_mutex_init(&gCheckMutex, NULL) != 0) {
		FreeRecordNoSafe(&gSafeMutexList);
	}
#endif
#endif

	bStSafeMutex = TRUE;
#endif
	return TRUE;
}

tBOOL EndSafeMutex( tBOOL bUnLock )
{
#if defined(_USE_SAFEMUTEX)
	tINT i;
	SAFEMUTEX_FORMAT SafeMutex;

	if (!bStSafeMutex) return TRUE;
#if defined(_MTHREAD)
#if defined(_SAFEMUTEX_MUTEX)
	if (pthread_mutex_lock(&gCheckMutex) != 0) return FALSE;
#endif
	if (bUnLock) {
		for ( i = 0 ; i < gSafeMutexList.nUsed ; i ++ ) {
			GetRecord(&gSafeMutexList, i, &SafeMutex);
			pthread_mutex_unlock(SafeMutex.MutexId);
		}
	}
	FreeRecordNoSafe(&gSafeMutexList);
#if defined(_SAFEMUTEX_MUTEX)
	if (pthread_mutex_unlock(&gCheckMutex) != 0) return FALSE;
	pthread_mutex_destroy(&gCheckMutex); // mutex ��������..
#endif
#endif
	bStSafeMutex = FALSE;
#endif
	return TRUE;
}

tBOOL SetSafeMutex(pthread_t ThreadId, pthread_mutex_t *MutexId, tBOOL bLock)
{
#if defined(_USE_SAFEMUTEX)
	tINT i;
	SAFEMUTEX_FORMAT SafeMutex;

#if defined(_MTHREAD)
#if defined(_SAFEMUTEX_MUTEX)
	tINT last_state;
#endif
	if (!bStSafeMutex) return TRUE;
#if defined(_SAFEMUTEX_MUTEX)

        //pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckMutex);
	pthread_mutex_lock(&gCheckMutex);
#endif

	if (bLock) { // locking  �̸�..
		for ( i = 0 ; i < gSafeMutexList.nUsed ; i ++ ) {
			GetRecord(&gSafeMutexList, i, &SafeMutex);
			//if (SafeMutex.ThreadId == ThreadId && (SafeMutex.MutexId)->__pthread_mutex_data == MutexId->__pthread_mutex_data) {
			if (SafeMutex.ThreadId == ThreadId && (tINT)(SafeMutex.MutexId) == (tINT)MutexId) {
				break;
			}
		}
		if (i >= gSafeMutexList.nUsed) { // ����.. 
			SafeMutex.ThreadId = ThreadId;
			SafeMutex.MutexId = MutexId;
			AppendRecordNoSafe(&gSafeMutexList, &SafeMutex);
		//	printf("Append SafeMutex : %d\n", ThreadId);
		}

	}
	else { // unlock..
		for ( i = 0 ; i < gSafeMutexList.nUsed ; i ++ ) {
			GetRecord(&gSafeMutexList, i, &SafeMutex);
			//if (SafeMutex.ThreadId == ThreadId && (SafeMutex.MutexId)->__pthread_mutex_data == MutexId->__pthread_mutex_data) {
			if (SafeMutex.ThreadId == ThreadId && (tINT)(SafeMutex.MutexId) == (tINT)MutexId) {
				break;
			}
		}
		if (i < gSafeMutexList.nUsed) { // ������ .. 
			DelRecord(&gSafeMutexList, i);
		//	printf("Delete SafeMutex : %d\n", ThreadId);
		}
	}


#if defined(_SAFEMUTEX_MUTEX)
	pthread_cleanup_pop(1);
	//if (pthread_mutex_unlock(&gCheckMutex) != 0) {
	//	return FALSE;
	//}
        //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
#endif
#endif
#endif
	return TRUE;
}

// ��ü ����鼭... unlock��ų��...
tBOOL DelSafeMutex(pthread_t ThreadId, tBOOL bUnLock)
{
#if defined(_USE_SAFEMUTEX)
	tINT i;
	SAFEMUTEX_FORMAT SafeMutex;

#if defined(_MTHREAD)
#if defined(_SAFEMUTEX_MUTEX)
	tINT last_state;
#endif
	if (!bStSafeMutex) return TRUE;
#if defined(_SAFEMUTEX_MUTEX)

        //pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckMutex);
	pthread_mutex_lock(&gCheckMutex);
#endif
	i = 0;

	while (i < gSafeMutexList.nUsed) {
		GetRecord(&gSafeMutexList, i, &SafeMutex);
		if (SafeMutex.ThreadId == ThreadId) {
			if (bUnLock) {
				pthread_mutex_unlock(SafeMutex.MutexId);
			}
			if (DelRecord(&gSafeMutexList, i)) {
				printf("Thread Lock Unlock... %d\n", (tINT)ThreadId);
				continue;
			}
		}
		i++;
	}

#if defined(_SAFEMUTEX_MUTEX)
	pthread_cleanup_pop(1);
	//if (pthread_mutex_unlock(&gCheckMutex) != 0) {
	//	return FALSE;
	//}
        //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
#endif
#endif
#endif

	return TRUE;

}


