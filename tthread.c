/*

	timer thread
	Author : hyypucs
	B-Date : 1999. 9. 16.
	Comment:
*/
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "dwtype.h"
#include "dwthread.h"
#include "mutex.h"
#include "tthread.h"

lpProcP lpTThreadProc(tVOID *lpInParam);

tBOOL CreateTThread(pthread_t *lpThreadId, pthread_mutex_t **lpMutex, pthread_cond_t **lpCondition, tINT nSec, lpProcP lpCallProc, tVOID *lpParam)
{
#if defined(_MTHREAD) 
	TTHREAD_PARAM *lpTThreadParam;
	
	*lpMutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if (*lpMutex == NULL) return FALSE;
	*lpCondition = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	if (*lpCondition == NULL) {
		free(*lpMutex);
		return FALSE;
	}
	lpTThreadParam = (TTHREAD_PARAM *)malloc(sizeof(TTHREAD_PARAM));
	if (lpTThreadParam == NULL) {
		free(*lpMutex);
		free(*lpCondition);
		free(lpTThreadParam);
	}


	if (pthread_mutex_init(*lpMutex, NULL) != 0) {
		free(*lpMutex);
		free(*lpCondition);
		free(lpTThreadParam);
		return FALSE;
	}
	if (pthread_cond_init(*lpCondition, NULL) != 0) {
		pthread_mutex_destroy(*lpMutex);
		free(*lpMutex);
		free(*lpCondition);
		free(lpTThreadParam);
		return FALSE;
	}
	lpTThreadParam->lpMutex = *lpMutex;
	lpTThreadParam->lpCondition = *lpCondition;
	lpTThreadParam->nSec = nSec;
	lpTThreadParam->lpCallProc = lpCallProc;
	lpTThreadParam->lpParam = lpParam;

	return (CreateThread(lpThreadId, (lpProcP)lpTThreadProc, (tVOID *)(lpTThreadParam)));
#else
	return FALSE;
#endif
}

tBOOL DestroyTThread(pthread_mutex_t *lpMutex, pthread_cond_t *lpCondition)
{
#if defined(_MTHREAD)
	if (pthread_mutex_lock(lpMutex) != 0) return FALSE;
	if (pthread_cond_signal(lpCondition) != 0) {
		pthread_mutex_unlock(lpMutex);
		return FALSE;
	}
	pthread_mutex_unlock(lpMutex);
#endif
	return TRUE;
}

lpProcP lpTThreadProc(tVOID *lpInParam)
{
#if defined(_MTHREAD)
	LPTTHREAD_PARAM lpParam = (LPTTHREAD_PARAM)lpInParam;
        struct timespec delaytime;

	while (1) {
		pthread_mutex_lock(lpParam->lpMutex);


        	delaytime.tv_sec = time(NULL) + lpParam->nSec;
        	delaytime.tv_nsec = 0;

        	if (pthread_cond_timedwait(lpParam->lpCondition, lpParam->lpMutex, &delaytime) == 0) {
			pthread_mutex_unlock(lpParam->lpMutex);
			break;
        	}

		pthread_mutex_unlock(lpParam->lpMutex);

		if(lpParam->lpCallProc) {
                	lpParam->lpCallProc(lpParam->lpParam);
		}
	}

	pthread_mutex_destroy(lpParam->lpMutex);
	pthread_cond_destroy(lpParam->lpCondition);
	free(lpParam->lpMutex);
	free(lpParam->lpCondition);
	free(lpParam);
	DestroyThread((tVOID *)0);
#endif
	
	return(NULL);
}

