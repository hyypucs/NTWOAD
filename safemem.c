/*

		서버가 중간에 죽게 되면 memory쪽에서 free가 되지 않아서 메모리 증가하는 것을
                수정하기 위한 루틴 
              
                Safe Mem  

		Author : hyypucs
		B-Date : 2000. 6. 12.

        Comment:
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "dwtype.h"
#include "record.h"
#include "safemem.h"

#define _MEM_MUTEX

#if defined(_USE_SAFEMEM)
PRIVATE RECORD gSafeMemList;
PRIVATE tBOOL bStSafeMem = FALSE;
#if defined(_MEM_MUTEX)
pthread_mutex_t gCheckMem;
#endif
#endif

tBOOL BeginSafeMem( tVOID )
{
#if defined(_USE_SAFEMEM)
	if (bStSafeMem) return TRUE;
#if defined(_MTHREAD)
	if (AllocRecordNoSafe(&gSafeMemList, sizeof(SAFEMEM_FORMAT), 100, 10) == FALSE) return FALSE;

#if defined(_MEM_MUTEX)
       	if (pthread_mutex_init(&gCheckMem, NULL) != 0) {
		FreeRecordNoSafe(&gSafeMemList);
		return FALSE;
	}
#endif
#endif

	bStSafeMem = TRUE;
#endif
	return TRUE;
}

tBOOL EndSafeMem( tBOOL bFree )
{
#if defined(_USE_SAFEMEM)
	tINT i;
	SAFEMEM_FORMAT SafeMem;

	if (!bStSafeMem) return TRUE;
#if defined(_MTHREAD)
#if defined(_MEM_MUTEX)
	if (pthread_mutex_lock(&gCheckMem) != 0) return FALSE;
#endif
	if (bFree) {
		for ( i = 0 ; i < gSafeMemList.nUsed ; i ++ ) {
			GetRecord(&gSafeMemList, i, &SafeMem);
			if (SafeMem.bIsRecord) FreeRecordNoSafe((RECORD *)(SafeMem.lpMem));
		}
	}
	FreeRecordNoSafe(&gSafeMemList);

#if defined(_MEM_MUTEX)
	if (pthread_mutex_unlock(&gCheckMem) != 0) return FALSE;
	pthread_mutex_destroy(&gCheckMem); // mutex 지워야지..
#endif

#endif
	bStSafeMem = FALSE;
#endif
	return TRUE;
}

tBOOL IsSetSafeMem(pthread_t *ThreadId, tVOID *lpMem)
{
	tBOOL bRetVal = FALSE;
#if defined(_USE_SAFEMEM)
	tINT i;
	SAFEMEM_FORMAT SafeMem;
#if defined(_MTHREAD)
#if defined(_MEM_MUTEX)
        tINT last_state;
#endif

	if (!bStSafeMem) return FALSE;

#if defined(_MEM_MUTEX)

        //pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckMem);
	pthread_mutex_lock(&gCheckMem);
#endif
	for ( i = 0 ; i < gSafeMemList.nUsed ; i ++ ) {
		GetRecord(&gSafeMemList, i, &SafeMem);
		if ((tINT)SafeMem.lpMem == (tINT)lpMem) { // memory  번지가 같다.
			*ThreadId = SafeMem.ThreadId;
			bRetVal = TRUE;
			break;
		}
	}
EndProc:;
#if defined(_MEM_MUTEX)
	pthread_cleanup_pop(1);
	//pthread_mutex_unlock(&gCheckMem);
        //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
#endif
#endif
#endif
	return bRetVal;
}

tBOOL SetSafeMem(pthread_t ThreadId, tVOID *lpMem, tBOOL bAlloc, tBOOL bIsRecord)
{
#if defined(_USE_SAFEMEM)
	tINT i;
	SAFEMEM_FORMAT SafeMem;

#if defined(_MTHREAD)
#if defined(_MEM_MUTEX)
        tINT last_state;
#endif
	if (!bStSafeMem) return TRUE;
#if defined(_MEM_MUTEX)

        //pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckMem);
	pthread_mutex_lock(&gCheckMem);
#endif

	if (bAlloc) { // Alloc  이면..
		for ( i = 0 ; i < gSafeMemList.nUsed ; i ++ ) {
			GetRecord(&gSafeMemList, i, &SafeMem);
			if (SafeMem.ThreadId == ThreadId && (tINT)SafeMem.lpMem == (tINT)lpMem && SafeMem.bIsRecord == bIsRecord) {
				break;
			}
		}
		if (i >= gSafeMemList.nUsed) { // 없다.. 
			SafeMem.ThreadId = ThreadId;
			SafeMem.lpMem = lpMem; // point만 
			SafeMem.bIsRecord = bIsRecord;
			AppendRecordNoSafe(&gSafeMemList, &SafeMem);
			//printf("Alloc : %d\n", ThreadId);
		}
	}
	else { // free..
		for ( i = 0 ; i < gSafeMemList.nUsed ; i ++ ) {
			GetRecord(&gSafeMemList, i, &SafeMem);
			if (SafeMem.ThreadId == ThreadId && (tINT)SafeMem.lpMem == (tINT)lpMem && SafeMem.bIsRecord == bIsRecord) {
				break;
			}
		}
		if (i < gSafeMemList.nUsed) { // 있으면 .. 
			DelRecord(&gSafeMemList, i);
			//printf("Delete : %d\n", ThreadId);
		}
	}

#if defined(_MEM_MUTEX)
	pthread_cleanup_pop(1);
	//pthread_mutex_unlock(&gCheckMem);
        //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
#endif
#endif
#endif
	return TRUE;
}

// 전체 지우면서... Free시킬래...
tBOOL DelSafeMem(pthread_t ThreadId, tBOOL bFree)
{
#if defined(_USE_SAFEMEM)
	tINT i;
	SAFEMEM_FORMAT SafeMem;
#if defined(_MEM_MUTEX)
	tINT last_state;
#endif


	if (gSafeMemList.nUsed <= 0) return TRUE;

#if defined(_MTHREAD)
	if (!bStSafeMem) return TRUE;
#if defined(_MEM_MUTEX)
	pthread_cleanup_push((tVOID *)pthread_mutex_unlock, (tVOID *)&gCheckMem);
	pthread_mutex_lock(&gCheckMem);
#endif
	i = 0;

	while (gSafeMemList.nUsed > 0 && i < gSafeMemList.nUsed) {
		GetRecord(&gSafeMemList, i, &SafeMem);
		if (SafeMem.ThreadId == ThreadId) {
			if (bFree) {
				if (SafeMem.bIsRecord) FreeRecordNoSafe((RECORD *)(SafeMem.lpMem));
				else {
					free(SafeMem.lpMem);
				}
			}
			if (DelRecord(&gSafeMemList, i)) {
				//printf("%d F(%d)\n", (tINT)ThreadId, (tINT)(SafeMem.bIsRecord) );
				continue;
			}
		}
		i++;
	}


#if defined(_MEM_MUTEX)
	pthread_cleanup_pop(1);
	//pthread_mutex_unlock(&gCheckMem);
#endif
#endif
#endif

	return TRUE;

}

