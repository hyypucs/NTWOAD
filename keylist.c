#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dwtype.h"
#include "seed.h"
#include "dwhashfunc.h"
#include "db.h"
#include "keylist.h"

tBOOL OpenKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szDBName, tINT nC_DB, tINT nPageSize, tINT nCacheSize)
{
	tCHAR szFileName[MAX_PATH];
	tINT i, j;
	int ret;

	lpKeyList->bOpenKeyList = FALSE;

	if (nC_DB > MAX_DB_COUNT) {
		printf("MAX_DB_COUNT overflow : %d < %d\n", MAX_DB_COUNT, nC_DB);
		return FALSE;
	}
	lpKeyList->nC_DB = nC_DB;

	strcpy(lpKeyList->szDBName, szDBName);
	
	for ( i = 0 ; i < nC_DB ; i ++ ) {
		sprintf(szFileName, "%s_%04d", szDBName, i);

		if ((ret = db_create(&(lpKeyList->dbp[i]), NULL, 0)) != 0) {
                	fprintf(stderr,
                    	"%s: db_create: %s\n", szDBName, db_strerror(ret));
			for ( j = 0 ; j < i ; j ++ ) {
				lpKeyList->dbp[j]->close(lpKeyList->dbp[j], 0);
				//lpKeyList->dbcp[j]->close(lpKeyList->dbcp[j]);
			}
                	return (FALSE);
        	}

		lpKeyList->dbp[i]->set_errfile(lpKeyList->dbp[i], stderr);
        	lpKeyList->dbp[i]->set_errpfx(lpKeyList->dbp[i], szDBName);
        	if ((ret = lpKeyList->dbp[i]->set_pagesize(lpKeyList->dbp[i], nPageSize)) != 0) {
                	lpKeyList->dbp[i]->err(lpKeyList->dbp[i], ret, "set_pagesize");
			lpKeyList->dbp[i]->close(lpKeyList->dbp[i], 0);
			for ( j = 0 ; j < i ; j ++ ) {
				lpKeyList->dbp[j]->close(lpKeyList->dbp[j], 0);
				//lpKeyList->dbp[j]->close(lpKeyList->dbcp[j]);
			}
                	return (FALSE);
        	}
        	if ((ret = lpKeyList->dbp[i]->set_cachesize(lpKeyList->dbp[i], 0,  nCacheSize, 0)) != 0) {
                	lpKeyList->dbp[i]->err(lpKeyList->dbp[i], ret, "set_cachesize");
			lpKeyList->dbp[i]->close(lpKeyList->dbp[i], 0);
			for ( j = 0 ; j < i ; j ++ ) {
				lpKeyList->dbp[j]->close(lpKeyList->dbp[j], 0);
				//lpKeyList->dbcp[j]->close(lpKeyList->dbcp[j]);
			}
        	}
		if ((ret = lpKeyList->dbp[i]->open(lpKeyList->dbp[i], NULL, szFileName, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
                	lpKeyList->dbp[i]->err(lpKeyList->dbp[i], ret, "%s: open", szFileName);
			lpKeyList->dbp[i]->close(lpKeyList->dbp[i], 0);
			for ( j = 0 ; j < i ; j ++ ) {
				lpKeyList->dbp[j]->close(lpKeyList->dbp[j], 0);
				//lpKeyList->dbcp[j]->close(lpKeyList->dbcp[j]);
			}
        	}

	/*
		if ((ret = lpKeyList->dbp[i]->cursor(lpKeyList->dbp[i], NULL, &(lpKeyList->dbcp[i]), 0)) != 0) {
        	        lpKeyList->dbp[i]->err(lpKeyList->dbp[i], ret, "DB->cursor");
			lpKeyList->dbp[i]->close(lpKeyList->dbp[i], 0);
			for ( j = 0 ; j < i ; j ++ ) {
				lpKeyList->dbp[j]->close(lpKeyList->dbp[j], 0);
				lpKeyList->dbcp[j]->close(lpKeyList->dbcp[j]);
			}
        	}
	*/
	}
	lpKeyList->bOpenKeyList = TRUE;
	return TRUE;
}

tVOID CloseKeyList(KEYLIST_TYPE *lpKeyList)
{
	tINT i;

	if (lpKeyList->bOpenKeyList == FALSE) return;
	for ( i = 0 ; i < lpKeyList->nC_DB ; i ++ ) {
		lpKeyList->dbp[i]->close(lpKeyList->dbp[i], 0);
		//lpKeyList->dbcp[i]->close(lpKeyList->dbcp[i]);
	}
}

tBOOL PutKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szKey, tVOID *lpData, tINT nSizeData)
{
	DBT key, data;
	tINT nPosKey;
	tINT ret;

	nPosKey = Hash_BySeed((unsigned char *)szKey);
	nPosKey %= lpKeyList->nC_DB;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	key.data = (tVOID *)szKey;
	key.size = strlen(szKey);

	data.data = lpData;
	data.size = nSizeData;
	ret = lpKeyList->dbp[nPosKey]->put(lpKeyList->dbp[nPosKey], NULL, &key, &data, DB_NOOVERWRITE);
	if (ret != 0) {
		lpKeyList->dbp[nPosKey]->err(lpKeyList->dbp[nPosKey], ret, "DB->put");
		return FALSE;
	}
	return TRUE;
}

tBOOL GetKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szKey, tVOID *lpData)
{
	DBT key, data;
	tINT nPosKey;
	tINT ret;

	nPosKey = Hash_BySeed((unsigned char *)szKey);
	nPosKey %= lpKeyList->nC_DB;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	key.data = szKey;
	key.size = strlen(szKey);

	ret = lpKeyList->dbp[nPosKey]->get(lpKeyList->dbp[nPosKey], NULL, &key, &data, 0);
	if (ret != 0) {
		return FALSE;
	}
	memcpy(lpData, data.data, data.size);
	return TRUE;
}

tBOOL DelKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szKey)
{
	DBT key, data;
	tINT nPosKey;
	tINT ret;

	nPosKey = Hash_BySeed((unsigned char *)szKey);
	nPosKey %= lpKeyList->nC_DB;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	key.data = szKey;
	key.size = strlen(szKey);

	ret = lpKeyList->dbp[nPosKey]->del(lpKeyList->dbp[nPosKey], NULL, &key, 0);
	if (ret != 0) {
		if (ret == DB_NOTFOUND) return TRUE;
		return FALSE;
	}
	return TRUE;

}

tBOOL UpdateKeyList(KEYLIST_TYPE *lpKeyList, tCHAR *szKey, tVOID *lpData, tINT nSizeData)
{
	// 지우고..
	if (DelKeyList(lpKeyList, szKey)) {
		// 다시 넣고..
		if (PutKeyList(lpKeyList, szKey, lpData, nSizeData) == FALSE) return FALSE;
	}
	else return FALSE;

	return TRUE;
}

#define	BUFFER_LENGTH	(5 * 1024 * 1024)

tBOOL DisplayKeyList(KEYLIST_TYPE *lpKeyList, PutKeyListFuncP PutKeyListFunc, tVOID *lpArg1, tVOID *lpArg2 )
{
	tINT i;
	tINT ret;
	DBT key, data;
#if defined(_OLD)
	size_t retklen, retdlen;
	char *retkey, *retdata;
	void *p;
#endif
	DBC *dbcp;


#if defined(_OLD)
	if ((data.data = malloc(BUFFER_LENGTH)) == NULL) return (FALSE);
	data.ulen = BUFFER_LENGTH;
	data.flags = DB_DBT_USERMEM;
#endif

	for ( i = 0 ; i < lpKeyList->nC_DB ; i ++ ) {

		if ((ret = lpKeyList->dbp[i]->cursor(lpKeyList->dbp[i], NULL, &dbcp, 0)) != 0) {
#if defined(_OLD)
			free(data.data);
#endif
			return FALSE;
		}
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));

#if defined(_OLD)
		for (;;) {
			if ((ret = dbcp->c_get(dbcp, &key, &data, DB_MULTIPLE_KEY | DB_NEXT)) != 0) {
				break;
			}

			for (DB_MULTIPLE_INIT(p, &data) ; ; ) {
				DB_MULTIPLE_KEY_NEXT(p, &data, (void *)retkey, retklen, (void *)retdata, retdlen);
				if (p == NULL)
					break;
				if (PutKeyListFunc(retkey, retklen, retdata, retdlen, lpArg1, lpArg2) == FALSE) {
					if ((ret = dbcp->c_close(dbcp)) != 0) {
						free(data.data);
						return FALSE;
			
					}
				}
			}


		}
#else
		while ((ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0) {
			if (PutKeyListFunc((char *)(key.data), key.size, data.data, data.size, lpArg1, lpArg2) == FALSE) {
				if ((ret = dbcp->c_close(dbcp)) != 0) {
#if defined(_OLD)
					free(data.data);
#endif
					return FALSE;
		
				}
			}
		}
	
#endif

		if ((ret = dbcp->c_close(dbcp)) != 0) {
#if defined(_OLD)
			free(data.data);
#endif
			return FALSE;
			
		}
	}
#if defined(_OLD)
	free(data.data);
#endif
	return TRUE;
}

