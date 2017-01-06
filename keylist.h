#if !defined(_KEYLIST_H_)
#define _KEYLIST_H_
#define MAX_DB_COUNT 1024
typedef struct {
	tBOOL bOpenKeyList;
	DB *dbp[MAX_DB_COUNT];
	//DBC *dbcp[MAX_DB_COUNT];
	tINT nC_DB;
	tCHAR szDBName[MAX_PATH];
}KEYLIST_TYPE, *LP_KEYLIST_TYPE;

tBOOL OpenKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szDBName, tINT nC_DB, tINT nPageSize, tINT nCacheSize);
tVOID CloseKeyList(KEYLIST_TYPE *lpKeyList);
tBOOL PutKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szKey, tVOID *lpData, tINT nSizeData);
tBOOL GetKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szKey, tVOID *lpData);
tBOOL DelKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szKey);
tBOOL UpdateKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szKey, tVOID *lpData, tINT nSizeData);

typedef tBOOL (*PutKeyListFuncP)( tCHAR *szKey, tINT nLenszKey, tVOID *lpData, tINT nSizeData, tVOID *lpArg1, tVOID *lpArg2 );

// keylist¸¦ ´Ù retrieve
tBOOL DisplayKeyList(KEYLIST_TYPE *lpKeyList, PutKeyListFuncP PutKeyListFunc, tVOID *lpArg1, tVOID *lpArg2 );

#endif /* keylist.h */
