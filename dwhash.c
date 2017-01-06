//
// hash 함수를 만듦. by hyypucs 2005.4.14.
// dangling 를 record 메모리로 사용해서 처리
//

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include "dwtype.h"
#include "dwhash.h"
#include "record.h"

typedef struct {
	tINT nInfo;
	RECORD *lpNextRec;
}DWHASH_BLOCK_TYPE;

// 해슁이 같을 때 ... 
typedef struct {
	tINT nInfo;
}DWHASH_NEXT_TYPE;

// 파일 save할 때 포캣
typedef struct {
	tINT nInfo;
	tINT nPosNextRec;
}DWHASH_BLOCK_FILE_TYPE;

tBOOL InitHash(DWHASH_TYPE *lpHash, tINT nBucketCount, HASH_CalFunc pHashCalFunc, HASH_PutFunc pHashPutFunc)
{
	tINT i;
	DWHASH_BLOCK_TYPE BlockItem;

	lpHash->lpBuf = (tBYTE *)malloc(nBucketCount * sizeof(DWHASH_BLOCK_TYPE)); // 기본으로 0로 alloc 받지.
	if (lpHash->lpBuf == NULL) {
		return FALSE;
	}
	// 초기화
	BlockItem.nInfo = -1;
	BlockItem.lpNextRec = NULL;
	for ( i = 0 ;  i < nBucketCount ; i ++ ) {
		memcpy(lpHash->lpBuf + i*sizeof(DWHASH_BLOCK_TYPE), &BlockItem, sizeof(DWHASH_BLOCK_TYPE));
	}
	//memset(lpHash->lpBuf, 0, nBucketCount * sizeof(DWHASH_BLOCK_TYPE));
	lpHash->nBucketCount = nBucketCount;
	lpHash->nCollision = 0;
	lpHash->pHashCalFunc = pHashCalFunc;
	lpHash->pHashPutFunc = pHashPutFunc;
	return TRUE;
}


tVOID CloseHash(DWHASH_TYPE *lpHash)
{
	tINT i;
	DWHASH_BLOCK_TYPE BlockItem;

	if (lpHash == NULL) return;
	for ( i = 0 ; i < lpHash->nBucketCount ; i ++ ) {
		memcpy(&BlockItem, lpHash->lpBuf + i*sizeof(DWHASH_BLOCK_TYPE), sizeof(DWHASH_BLOCK_TYPE));
		if (BlockItem.lpNextRec) {
			FreeRecord(BlockItem.lpNextRec); // 먼저 지워야지..
			free(BlockItem.lpNextRec);
		}
	}

	if (lpHash->lpBuf) free(lpHash->lpBuf);
	memset(lpHash, 0, sizeof(DWHASH_TYPE));
}

tBOOL Hash2File(DWHASH_TYPE *lpHash, tCHAR *szSrcFileName, tBOOL bFree)
{

	FILE *Fp;
	FILE *recFp;
	tCHAR szFile[MAX_PATH];
	tINT i;
	DWHASH_BLOCK_TYPE BlockItem;
	DWHASH_BLOCK_FILE_TYPE BlockFileItem;
	tINT nCount;
#if defined(_DEBUG)
	tINT nMaxCol = 0, nCountCol = 0;
#endif
	tCHAR szFileName[MAX_PATH], *p;

	strcpy(szFileName, szSrcFileName);
	p = strrchr(szFileName, '.');
	if (p) {
		*p = '\0';
	}
	
	sprintf(szFile, "%s.dic", szFileName);
	if ((Fp = fopen(szFile, "wb")) == NULL) {
                return FALSE;
        }
	sprintf(szFile, "%s.hdic", szFileName);
	if ((recFp = fopen(szFile, "wb")) == NULL) {
		fclose(Fp);
                return FALSE;
        }

        lpHash->nBucketCount = DwordCvt(lpHash->nBucketCount);
        lpHash->nCollision = DwordCvt(lpHash->nCollision);

        if (fwrite(lpHash, sizeof(DWHASH_TYPE), 1, Fp) != 1) {
                fclose(Fp);
		fclose(recFp);
                return FALSE;
        }
        lpHash->nBucketCount = DwordCvt(lpHash->nBucketCount);
        lpHash->nCollision = DwordCvt(lpHash->nCollision);

	for ( i = 0 ; i < lpHash->nBucketCount ; i ++ ) {
		memcpy(&BlockItem, lpHash->lpBuf + i*sizeof(DWHASH_BLOCK_TYPE), sizeof(DWHASH_BLOCK_TYPE));
		BlockFileItem.nInfo = BlockItem.nInfo;
		BlockFileItem.nPosNextRec = -1;
#if defined(_DEBUG)
		if (BlockItem.nInfo == 15300) {
			printf("%d\n", BlockItem.nInfo);
		}
#endif
		if (BlockItem.lpNextRec) {

#if defined(_DEBUG)
			// collision info
			nCountCol++;
			if (nMaxCol < BlockItem.lpNextRec->nUsed) {
				nMaxCol = BlockItem.lpNextRec->nUsed;
			}
#endif

			BlockFileItem.nPosNextRec = ftell(recFp);
			nCount = BlockItem.lpNextRec->nUsed;
			nCount = DwordCvt(nCount);
			if (fwrite(&nCount, sizeof(tINT), 1, recFp)!=1) {
				fclose(Fp);
				fclose(recFp);
				return FALSE;
			}
			nCount = DwordCvt(nCount);
#if defined(_DEBUG)
		if (BlockItem.nInfo == 15300) {
			printf("www.dreamwiz.com/ count = %d, pos=%d\n", nCount, BlockFileItem.nPosNextRec);
		}
#endif

			if (fwrite(BlockItem.lpNextRec->lpBuf, BlockItem.lpNextRec->nUsed * BlockItem.lpNextRec->nWidth, 1, recFp)!=1) {
				fclose(Fp);
				fclose(recFp);
				return FALSE;
			}
		}
		if (fwrite(&BlockFileItem, sizeof(DWHASH_BLOCK_FILE_TYPE), 1, Fp) != 1) {
			fclose(Fp);
			fclose(recFp);
			return FALSE;
		}
        }
        if (bFree) CloseHash(lpHash);
        fclose(Fp);
	fclose(recFp);
#if defined(_DEBUG)
	printf("ok ... write : %s.dic %s.hdic collision : %d, max collision size : %d\n", szFileName, szFileName, nCountCol, nMaxCol);
#endif
	return TRUE;
}

tBOOL File2Hash(tCHAR *szSrcFileName, DWHASH_TYPE *lpHash, HASH_CalFunc pHashCalFunc, HASH_PutFunc pHashPutFunc)
{
	tINT i, j;
	DWHASH_BLOCK_TYPE BlockItem;
	DWHASH_BLOCK_FILE_TYPE BlockFileItem;
	FILE *Fp;
	FILE *recFp;
	tCHAR szFile[MAX_PATH];
	tINT nCount;
	tBOOL bRetVal = TRUE;
	tCHAR szFileName[MAX_PATH], *p;

	strcpy(szFileName, szSrcFileName);
	p = strrchr(szFileName, '.');
	if (p) {
		*p = '\0';
	}

	sprintf(szFile, "%s.dic", szFileName);
	if ((Fp = fopen(szFile, "rb")) == NULL) {
                return FALSE;
        }
	sprintf(szFile, "%s.hdic", szFileName);
	if ((recFp = fopen(szFile, "rb")) == NULL) {
		fclose(Fp);
                return FALSE;
        }

	memset(lpHash, 0, sizeof(DWHASH_TYPE));

        if (fread(lpHash, sizeof(DWHASH_TYPE), 1, Fp) != 1) {
                fclose(Fp);
                fclose(recFp);
                return FALSE;
        }
        lpHash->nBucketCount = DwordCvt(lpHash->nBucketCount);
        lpHash->nCollision = DwordCvt(lpHash->nCollision);

	if (lpHash->nBucketCount > 0) { // mem alloc
		lpHash->lpBuf = (tBYTE *)malloc(lpHash->nBucketCount * sizeof(DWHASH_BLOCK_TYPE));
		if (lpHash->lpBuf == NULL) {
			memset(lpHash, 0, sizeof(DWHASH_TYPE));
                	fclose(Fp);
                	fclose(recFp);
			return FALSE;
		}
	}

	for ( i = 0 ; i < lpHash->nBucketCount ; i ++ ) {
		if (fread(&BlockFileItem, sizeof(DWHASH_BLOCK_FILE_TYPE), 1, Fp) != 1) {
			bRetVal = FALSE;
			return FALSE;
		}
		BlockItem.nInfo = BlockFileItem.nInfo;
		BlockItem.lpNextRec = NULL;
#if defined(_DEBUG)
		if (BlockItem.nInfo == 15300) {
			printf("www.dreamwiz.com/ next = %d\n", BlockFileItem.nPosNextRec);
		}
#endif
		if (BlockFileItem.nPosNextRec != -1) { // 다른 info가 있다. 해슁이 같아서 ... 
			// sequence하게 넣어잖아.. 괜찮지..
			//fseek(recFp, BlockFileItem.nPosNextRec, SEEK_SET);
			if (fread(&nCount, sizeof(tINT), 1, recFp)!=1) {
				bRetVal = FALSE;
				break;
			}
			nCount = DwordCvt(nCount);
#if defined(_DEBUG)
		if (BlockItem.nInfo == 15300) {
			printf("www.dreamwiz.com/ count = %d\n", nCount);
		}
#endif

			BlockItem.lpNextRec = (RECORD *)malloc(sizeof(RECORD));
			if (BlockItem.lpNextRec == NULL) {
				bRetVal = FALSE;
				break;
			}

			if (AllocRecord(BlockItem.lpNextRec, sizeof(DWHASH_NEXT_TYPE), nCount+1, 5) == FALSE) {
				bRetVal = FALSE;
				break;
			}
			if (fread(BlockItem.lpNextRec->lpBuf, nCount * BlockItem.lpNextRec->nWidth, 1, recFp)!=1) {
				bRetVal = FALSE;
				break;
			}
			BlockItem.lpNextRec->nUsed = nCount;
		}
		memcpy(lpHash->lpBuf + i*sizeof(DWHASH_BLOCK_TYPE), &BlockItem, sizeof(DWHASH_BLOCK_TYPE));
	}
	if (bRetVal == FALSE) { // 메모리 정리
		for ( j = 0 ; j <= i && j < lpHash->nBucketCount ; j ++ ) {
			memcpy(&BlockItem, lpHash->lpBuf + j*sizeof(DWHASH_BLOCK_TYPE), sizeof(DWHASH_BLOCK_TYPE));
			if (BlockItem.lpNextRec) {
				FreeRecord(BlockItem.lpNextRec);
				free(BlockItem.lpNextRec);
			}
		}
		if (lpHash->lpBuf) free(lpHash->lpBuf);
		memset(&lpHash, 0, sizeof(DWHASH_TYPE));
               	fclose(Fp);
               	fclose(recFp);
		return FALSE;
	}

	fclose(Fp);
	fclose(recFp);

	lpHash->pHashCalFunc = pHashCalFunc;
	lpHash->pHashPutFunc = pHashPutFunc;
	return TRUE;
}


tBOOL AddHash(DWHASH_TYPE *lpHash, tVOID *lpItem, tINT nInfo)
{
	tINT nPos;
	DWHASH_BLOCK_TYPE HashItem;
	DWHASH_NEXT_TYPE NextItem;

	nPos = lpHash->pHashCalFunc(lpItem);
	if (nPos < 0) nPos *= -1;
	if (nPos >= lpHash->nBucketCount) { // exception
		nPos %= lpHash->nBucketCount;
	}

	memcpy(&HashItem, lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), sizeof(DWHASH_BLOCK_TYPE));
	if (HashItem.nInfo == -1) { // 아무 것도 없다. good
		HashItem.nInfo = nInfo;
		HashItem.lpNextRec = NULL; // 여기에 해슁이 같으면 달리겠지..
		memcpy(lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), &HashItem, sizeof(DWHASH_BLOCK_TYPE));
	}
	else { // collision 발생
		if (HashItem.lpNextRec == NULL) { // 벌써 collision이 일어났군..
			HashItem.lpNextRec = (RECORD *)malloc(sizeof(RECORD));
			if (HashItem.lpNextRec == NULL) {
				return FALSE;
			}
			if (AllocRecord(HashItem.lpNextRec, sizeof(DWHASH_NEXT_TYPE), 5, 5) == FALSE) {
				return FALSE;
			}
			memcpy(lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), &HashItem, sizeof(DWHASH_BLOCK_TYPE)); // mem alloc 했으니.. update 해야지 
		}
		NextItem.nInfo = nInfo;
		AppendRecord(HashItem.lpNextRec, &NextItem);
        	lpHash->nCollision ++;
	}
	return TRUE;
}

tBOOL DelHash(DWHASH_TYPE *lpHash, tVOID *lpItem, tINT nDelInfo)
{
	tINT nPos;
	DWHASH_BLOCK_TYPE HashItem;
	DWHASH_NEXT_TYPE NextItem;
	tINT i;

	nPos = lpHash->pHashCalFunc(lpItem);

	if (nPos < 0) nPos *= -1;
	if (nPos >= lpHash->nBucketCount) { // exception
		nPos %= lpHash->nBucketCount;
	}
	memcpy(&HashItem, lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), sizeof(DWHASH_BLOCK_TYPE));
	if (HashItem.nInfo == nDelInfo) { // 처음 것을 지울 때..
		if (HashItem.lpNextRec == NULL) {
			HashItem.nInfo = -1;
			memcpy(lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), &HashItem, sizeof(DWHASH_BLOCK_TYPE));
		}
		else { // 첫번째 것을 세팅한다.
			GetRecord(HashItem.lpNextRec, 0, &NextItem);
			DelRecord(HashItem.lpNextRec, 0);

			HashItem.nInfo = NextItem.nInfo;

			if (HashItem.lpNextRec->nUsed == 0) { // clear하자
				FreeRecord(HashItem.lpNextRec);
				free(HashItem.lpNextRec);
				HashItem.lpNextRec = NULL;
			}	
			memcpy(lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), &HashItem, sizeof(DWHASH_BLOCK_TYPE));
			if (lpHash->nCollision > 0) lpHash->nCollision --;
		}
	}
	else if (HashItem.lpNextRec) { // 더 있으면 더 찾아야지..
		for ( i = 0 ; i < HashItem.lpNextRec->nUsed ; i ++ ) {
			GetRecord(HashItem.lpNextRec, i, &NextItem);
			if (NextItem.nInfo == nDelInfo) {
				DelRecord(HashItem.lpNextRec, i);
				if (lpHash->nCollision > 0) lpHash->nCollision --;
				break;
			}
		}
		if (HashItem.lpNextRec->nUsed == 0) { // clear하자
			FreeRecord(HashItem.lpNextRec);
			free(HashItem.lpNextRec);
			HashItem.lpNextRec = NULL;
			memcpy(lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), &HashItem, sizeof(DWHASH_BLOCK_TYPE));
		}	
	}
	else { // not found
		return TRUE; 
	}
	return TRUE;
}

tBOOL ExistHash(DWHASH_TYPE *lpHash, tVOID *lpItem, tINT nInfo)
{
	tINT nPos;
	DWHASH_BLOCK_TYPE HashItem;
	DWHASH_NEXT_TYPE NextItem;
	tINT i;

	nPos = lpHash->pHashCalFunc(lpItem);

	if (nPos < 0) nPos *= -1;
	if (nPos >= lpHash->nBucketCount) { // exception
		nPos %= lpHash->nBucketCount;
	}
	memcpy(&HashItem, lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), sizeof(DWHASH_BLOCK_TYPE));
	if (HashItem.nInfo == nInfo) {
		return TRUE;
	}
	else if (HashItem.lpNextRec) {
		for ( i = 0 ; i < HashItem.lpNextRec->nUsed ; i ++ ) {
			GetRecord(HashItem.lpNextRec, i, &NextItem);
			if (NextItem.nInfo == nInfo) {
				return TRUE;
			}
		}

	}
	return FALSE;
}

tBOOL PutHash(DWHASH_TYPE *lpHash, tVOID *lpItem)
{
	tINT nPos;
	DWHASH_BLOCK_TYPE HashItem;
	DWHASH_NEXT_TYPE NextItem;
	tINT i;

	nPos = lpHash->pHashCalFunc(lpItem);

	if (nPos < 0) nPos *= -1;
	if (nPos >= lpHash->nBucketCount) { // exception
		nPos %= lpHash->nBucketCount;
	}
	memcpy(&HashItem, lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), sizeof(DWHASH_BLOCK_TYPE));
	if (HashItem.nInfo == -1) { // 없다.
		return FALSE;
	}
	lpHash->pHashPutFunc(lpItem, HashItem.nInfo);
	if (HashItem.lpNextRec) {
		for ( i = 0 ; i < HashItem.lpNextRec->nUsed ; i ++ ) {
			GetRecord(HashItem.lpNextRec, i, &NextItem);
			lpHash->pHashPutFunc(lpItem, NextItem.nInfo);

		}
	}

	return TRUE;
}

// 내부적으로 alloc를 받으면서 진행하자.
tBOOL GetHash(DWHASH_TYPE *lpHash, tVOID *lpItem, tINT **lpnInfo, tINT *nRetCount)
{
	tINT nPos, nCount = 0;
	DWHASH_BLOCK_TYPE HashItem;
	DWHASH_NEXT_TYPE NextItem;
	tINT i;

	*lpnInfo = NULL;
	*nRetCount = 0;
	nPos = lpHash->pHashCalFunc(lpItem);

	if (nPos < 0) nPos *= -1;
	if (nPos >= lpHash->nBucketCount) { // exception
		nPos %= lpHash->nBucketCount;
	}

	memcpy(&HashItem, lpHash->lpBuf + nPos*sizeof(DWHASH_BLOCK_TYPE), sizeof(DWHASH_BLOCK_TYPE));
	if (HashItem.nInfo == -1) {
		return FALSE;
	}
	nCount++;
	if (HashItem.lpNextRec) {
		nCount += HashItem.lpNextRec->nUsed;
	}

	*lpnInfo = (tINT *)malloc(nCount * sizeof(tINT));
	if (*lpnInfo == NULL) return FALSE;

	nCount = 0;
	(*lpnInfo)[nCount++] = HashItem.nInfo;
	if (HashItem.lpNextRec) {
		for ( i = 0 ; i < HashItem.lpNextRec->nUsed ; i ++ ) {
			GetRecord(HashItem.lpNextRec, i, &NextItem);
			(*lpnInfo)[nCount++] = NextItem.nInfo;
		}
	}
	*nRetCount = nCount;
	return TRUE;
}
