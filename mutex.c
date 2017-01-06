/*

		Mutex 관리 모듈

		Author : hyypucs
		B-Date : 1999. 9. 16.

        Comment:
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "dwtype.h"
#include "mutex.h"
#include "safemutex.h"


tBOOL BeginMutexMan( LPMUTEXMAN_FORMAT lpMutexMan, tINT nMaxMutex )
{
#if defined(_MTHREAD)
	tINT i, j;
	lpMutexMan->nMaxMutex = nMaxMutex;
	lpMutexMan->MutexList = (pthread_mutex_t *)malloc(nMaxMutex * sizeof(pthread_mutex_t));
	if (lpMutexMan->MutexList == NULL) {
		return FALSE;
	}

	for ( i = 0 ; i < nMaxMutex ; i ++ ) {
        	if (pthread_mutex_init(lpMutexMan->MutexList+i, NULL) != 0) {
			for ( j = 0 ; j < i ; j ++ ) {
				pthread_mutex_destroy(lpMutexMan->MutexList+j);
			}
			return FALSE;
		}
	}
#endif

	return TRUE;
}

tBOOL EndMutexMan( LPMUTEXMAN_FORMAT lpMutexMan )
{
#if defined(_MTHREAD)
	tINT i, j;

	if (lpMutexMan->MutexList) {
		for ( i = 0 ; i < lpMutexMan->nMaxMutex ; i++ ) {
			if (pthread_mutex_destroy(lpMutexMan->MutexList+i) != 0) {
				for ( j = 0 ; j < i ; j ++ ) {
					pthread_mutex_init(lpMutexMan->MutexList+j, NULL);
				}
				return FALSE;
			}
		}
		free(lpMutexMan->MutexList);
	}
	lpMutexMan->MutexList = NULL;
	lpMutexMan->nMaxMutex = 0;
#endif
	return TRUE;
}

tBOOL CreateMutexMan( LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID )
{
#if defined(_MTHREAD)
	if (nMutexID < 0  || nMutexID >= lpMutexMan->nMaxMutex) {
		return(FALSE);
	}

        if (pthread_mutex_init(lpMutexMan->MutexList + nMutexID, NULL) == 0) return TRUE;
	return FALSE;
#else
	return TRUE;
#endif
}

tBOOL DestroyMutexMan( LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID )
{
#if defined(_MTHREAD)
	if (nMutexID < 0  || nMutexID >= lpMutexMan->nMaxMutex) {
		return(FALSE);
	}

        if (pthread_mutex_destroy(lpMutexMan->MutexList + nMutexID) == 0) return TRUE;
	return FALSE;
#else
	return TRUE;
#endif
}

tBOOL LockMutexMan( LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID )
{
#if defined(_MTHREAD)
	if (nMutexID < 0  || nMutexID >= lpMutexMan->nMaxMutex) {
		return(FALSE);
	}

	if (pthread_mutex_lock(lpMutexMan->MutexList + nMutexID) == 0) {
		return TRUE;
	}
	return FALSE;
#else
	return TRUE;
#endif
}

tBOOL TryLockMutexMan( LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID )
{
#if defined(_MTHREAD)
	if (nMutexID < 0  || nMutexID >= lpMutexMan->nMaxMutex) {
		return(FALSE);
	}

	if (pthread_mutex_trylock(lpMutexMan->MutexList + nMutexID) == 0) {
		return TRUE;
	}
	return FALSE;
#else
	return TRUE;
#endif
}

tBOOL UnLockMutexMan( LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID )
{
#if defined(_MTHREAD)
	if (nMutexID < 0  || nMutexID >= lpMutexMan->nMaxMutex) {
		return(FALSE);
	}

	if (pthread_mutex_unlock(lpMutexMan->MutexList + nMutexID) == 0) {
		return TRUE;
	}
	return FALSE;
#else
	return TRUE;
#endif
}


