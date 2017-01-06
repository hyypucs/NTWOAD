//
// dwhash.h
// by hyypucs 2005. 4. 15.
//
#if !defined(__DWHASH_H_)
#define __DWHASH_H_

typedef tINT (*HASH_CalFunc)(tVOID *inputItem);
typedef tVOID (*HASH_PutFunc)(tVOID *lpItem, tINT nInfo);

typedef struct {
        HASH_CalFunc pHashCalFunc;
        HASH_PutFunc pHashPutFunc;
        tBYTE *lpBuf;
        tINT nBucketCount;
	tINT nCollision;
}DWHASH_TYPE;

tBOOL InitHash(DWHASH_TYPE *lpHash, tINT nBucketCount, HASH_CalFunc pHashCalFunc, HASH_PutFunc pHashPutFunc);
tVOID CloseHash(DWHASH_TYPE *lpHash);
tBOOL Hash2File(DWHASH_TYPE *lpHash, tCHAR *szSrcFileName, tBOOL bFree);
tBOOL File2Hash(tCHAR *szSrcFileName, DWHASH_TYPE *lpHash, HASH_CalFunc pHashCalFunc, HASH_PutFunc pHashPutFunc);
tBOOL AddHash(DWHASH_TYPE *lpHash, tVOID *lpItem, tINT nInfo);
tBOOL DelHash(DWHASH_TYPE *lpHash, tVOID *lpItem, tINT nDelInfo);
tBOOL ExistHash(DWHASH_TYPE *lpHash, tVOID *lpItem, tINT nInfo);
tBOOL PutHash(DWHASH_TYPE *lpHash, tVOID *lpItem);

//반드시 free해야 한다.
tBOOL GetHash(DWHASH_TYPE *lpHash, tVOID *lpItem, tINT **lpnInfo, tINT *nRetCount);

#endif // dwhash.h
