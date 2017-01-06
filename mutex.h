/*

		Mutex 관리 모듈

		Author : hyypucs
        B-Date : 1999. 9. 16 

        Comment:
*/
#if !defined(__MUTEX_H__)

#define __MUTEX_H__

typedef struct  {
	pthread_mutex_t *MutexList;
	tINT nMaxMutex;
}MUTEXMAN_FORMAT, *LPMUTEXMAN_FORMAT;

tBOOL CreateMutexMan(LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID);
tBOOL DestroyMutexMan(LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID);
tBOOL LockMutexMan(LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID);
tBOOL UnLockMutexMan(LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID);
tBOOL TryLockMutexMan( LPMUTEXMAN_FORMAT lpMutexMan, tINT nMutexID );

tBOOL BeginMutexMan(LPMUTEXMAN_FORMAT lpMutexMan, tINT nMaxMutex);
tBOOL EndMutexMan(LPMUTEXMAN_FORMAT lpMutexMan);

#endif /* mutex.h */
