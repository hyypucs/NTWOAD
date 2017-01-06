/*
	linkedlist인데 block를 dynamic하게 하자..
	by hyypucs 2007. 11. 7.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "dwtype.h"
#include "record.h"
#include "util.h"
#include "linkedlistdyn.h"


// 이 블럭으로 움직인다.
typedef struct {
	tINT nCItem; // 블럭안에 item 몇개 채웠지?
	tBOOL nEnd; // 0 : 끝, 1 : 다음으로 가야지. 
	tINT nNextList; // 다음에 몇번째 List 블럭에 있을까? 절대적으로 점프할께 
}DYNLINKEDLIST_TYPE, *LP_DYNLINKEDLIST_TYPE;

tBOOL OpenDynLinkedListFile(DYNLIST_FILE_TYPE *lpListFile, tCHAR *szListFileName, tINT nCountFp, tINT nSizeItem, tINT nLimitFileSize, CompProcP CompRecord, tBOOL bOptRW)
{
	tCHAR szFileName[MAX_PATH];
	tINT i, j;
#if defined(_USE_DUMMY_FILE)
	#define WRITE_BLK 40960

	int _remain, rWrite;
	char buffer[WRITE_BLK];

	memset(buffer, ' ', WRITE_BLK);
#endif

	lpListFile->bOpenList = FALSE;
	if (nCountFp > MAX_FILE_LIST) {
		printf("overflose file count\n");
		return FALSE;
	}
	strcpy(lpListFile->szListFileName, szListFileName);

	lpListFile->nCountFp = nCountFp;
	lpListFile->nSizeItem = nSizeItem;
	lpListFile->nLimitFileSize = nLimitFileSize;
	lpListFile->bOptRW = bOptRW;


	lpListFile->CompRecord = CompRecord;

	for ( i = 0 ; i < lpListFile->nCountFp ; i++ ) {

#if defined(_USE_DUMMY_FILE)
		lpListFile->nPosEnd[i] = sizeof(tLONG); //file end pos초기화 
#endif

		sprintf(szFileName, "%s_%04d", szListFileName, i);
		if (lpListFile->bOptRW == OPEN_READ_LIST) {
			if (ExistFile(szFileName)) {
				lpListFile->fp[i] = fopen(szFileName, "rb");
				if ( lpListFile->fp[i] == NULL ) { // open error ..
					printf("%s open error check please..\n", szFileName);
					for ( j = 0 ; j < i ; j ++ ) {
						fclose(lpListFile->fp[j]);
						lpListFile->fp[j] = NULL;
					}
					return FALSE;
				}
#if defined(_USE_DUMMY_FILE)
				fread(&(lpListFile->nPosEnd[i]), sizeof(tLONG), 1, lpListFile->fp[i]);
#endif
			}
			else { // read인데 없으니 에러 내야지
				return FALSE;
			}
		}
		else { // write ... 
			if (ExistFile(szFileName)) {
				lpListFile->fp[i] = fopen(szFileName, "r+b");
				if ( lpListFile->fp[i] == NULL ) { // open error ..
					printf("%s open error check please..\n", szFileName);
					for ( j = 0 ; j < i ; j ++ ) {
						fclose(lpListFile->fp[j]);
						lpListFile->fp[j] = NULL;
					}
					return FALSE;
				}
#if defined(_USE_DUMMY_FILE)
				fread(&(lpListFile->nPosEnd[i]), sizeof(tLONG), 1, lpListFile->fp[i]);
#endif
			}
			else {
				lpListFile->fp[i] = fopen(szFileName, "w+b");
				if ( lpListFile->fp[i] == NULL ) { // open error ..
					printf("%s open error check please..\n", szFileName);
					for ( j = 0 ; j < i ; j ++ ) {
						fclose(lpListFile->fp[j]);
						lpListFile->fp[j] = NULL;
					}
					return FALSE;
				}
#if defined(_USE_DUMMY_FILE)
				// 처음이니 ..
				fwrite(&(lpListFile->nPosEnd[i]), sizeof(tLONG), 1, lpListFile->fp[i]);
				_remain = lpListFile->nLimitFileSize;
				while (_remain >= WRITE_BLK) {
					rWrite = fwrite(buffer, sizeof(char), WRITE_BLK, lpListFile->fp[i]);
					_remain -= rWrite;
				}
				if (_remain > 0) {
					rWrite = fwrite(buffer, sizeof(char), _remain, lpListFile->fp[i]);
				}
#endif
			}
		}
	}

	lpListFile->bOpenList = TRUE;

	return TRUE;

}

tVOID CloseDynLinkedListFile(DYNLIST_FILE_TYPE *lpListFile)
{
	tINT i;

	if (lpListFile->bOpenList) {

		for ( i = 0 ; i < lpListFile->nCountFp ; i ++ ) {
#if defined(_USE_DUMMY_FILE)
			fseek(lpListFile->fp[i], 0L, SEEK_SET);
			fwrite(&(lpListFile->nPosEnd[i]), sizeof(tLONG), 1, lpListFile->fp[i]);
#endif
			fclose(lpListFile->fp[i]);
			lpListFile->fp[i] = NULL;
		}
		lpListFile->bOpenList = FALSE;
	}
}

// *nRetStPos < 0 , *nRetLastPos < 0 이면 이 정보는 사용하지 말라는 것으로 판단하자..
tBOOL PutItemToDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItemBlock, tINT nItemInBlock, tINT *nRetFpNum, tINT *nRetStPos, tINT *nRetLastPos, tINT *nRetJump)
{
	DYNLINKEDLIST_TYPE LinkedList;
	tINT nPos, nPos1;
	tINT nFpNum = nSrcFpNum;
	tINT nJump = 0;

	*nRetFpNum = nFpNum;
	*nRetStPos = -1;
	*nRetLastPos = -1;
	if (lpListFile->bOptRW == OPEN_READ_LIST) return FALSE;
	if (lpListFile->bOpenList == FALSE) return FALSE;


	if (nStPos < 0) { // 처음 넣는 것이다...
		SetFLock(lpListFile->fp[nFpNum]);

#if defined(_USE_DUMMY_FILE)
		fseek(lpListFile->fp[nFpNum], lpListFile->nPosEnd[nFpNum], SEEK_SET);
		*nRetStPos = lpListFile->nPosEnd[nFpNum];
#else
		fseek(lpListFile->fp[nFpNum], 0L, SEEK_END);
		*nRetStPos = ftell(lpListFile->fp[nFpNum]);
#endif
		if (*nRetStPos + (lpListFile->nSizeItem * nItemInBlock) + (int)sizeof(DYNLINKEDLIST_TYPE) > lpListFile->nLimitFileSize - 1) { // 한 파일이 limit 가 넘어 간다... 
			// 에러 발생... 여러개 파일 존재시키지 않도록 함.
			UnsetFLock(lpListFile->fp[nFpNum]);
			return FALSE;
		}
		memset(&LinkedList, 0, sizeof(DYNLINKEDLIST_TYPE));
		LinkedList.nCItem = nItemInBlock;

		// block정보 및 next 정보..
		fwrite(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

		fwrite(lpItemBlock, lpListFile->nSizeItem*nItemInBlock, 1, lpListFile->fp[nFpNum]);

		*nRetLastPos = *nRetStPos;

		*nRetFpNum = nFpNum;
#if defined(_USE_DUMMY_FILE)
		lpListFile->nPosEnd[nFpNum] += (sizeof(DYNLINKEDLIST_TYPE) + (lpListFile->nSizeItem * nItemInBlock));
		//안전을 위해서 update시켜본다.
		// 나중에 속도를 위해서 없을 수도 있다.
		fseek(lpListFile->fp[nFpNum], 0L, SEEK_SET);
		fwrite(&(lpListFile->nPosEnd[nFpNum]), sizeof(tLONG), 1, lpListFile->fp[nFpNum]);
#endif


		UnsetFLock(lpListFile->fp[nFpNum]);
	}
	else {
		SetFLock(lpListFile->fp[nFpNum]);

		// 위치 옮긴 후 ..
		fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);
		*nRetStPos = nStPos;

		while (1) {
			nPos = ftell(lpListFile->fp[nFpNum]);

                	// block정보 및 next 정보..
                	fread(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

			if (LinkedList.nEnd == 0) { // 끝인데 full이니 맨 끝에 붙여야지..

#if defined(_USE_DUMMY_FILE)
				fseek(lpListFile->fp[nFpNum], lpListFile->nPosEnd[nFpNum], SEEK_SET);
				nPos1 = lpListFile->nPosEnd[nFpNum];
#else
				fseek(lpListFile->fp[nFpNum], 0L, SEEK_END);
				nPos1 = ftell(lpListFile->fp[nFpNum]);
#endif

				if (nPos1 + (lpListFile->nSizeItem * nItemInBlock) + (int)sizeof(DYNLINKEDLIST_TYPE) > lpListFile->nLimitFileSize - 1) { // 한 파일이 limit 가 넘어 간다... 
					printf("limit overflow \n");
					break;
				}
				else {
					LinkedList.nEnd = 1;

					// 이전 블럭에다 .. next 세팅해야지... ---------------------
					fseek(lpListFile->fp[nFpNum], nPos, SEEK_SET);
					LinkedList.nNextList = nPos1;
       	         			// block정보 및 next 정보..
                			fwrite(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);
					//---------------------------------
				}
				
				// 다시 맨 끝에 블럭 write 해야지 .. 

#if defined(_USE_DUMMY_FILE)
				fseek(lpListFile->fp[nFpNum], lpListFile->nPosEnd[nFpNum], SEEK_SET);
#else
				fseek(lpListFile->fp[nFpNum], 0L, SEEK_END);
#endif
				// jump 한번씩해야지..
				nJump ++;

				memset(&LinkedList, 0, sizeof(DYNLINKEDLIST_TYPE));
				LinkedList.nCItem = nItemInBlock;

				// block정보 및 next 정보..
				fwrite(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

				// 해당 데이터만 넣으면 될테니 ..
				fwrite(lpItemBlock, lpListFile->nSizeItem*nItemInBlock, 1, lpListFile->fp[nFpNum]);
				*nRetFpNum = nFpNum;
				*nRetLastPos = nPos1;
#if defined(_USE_DUMMY_FILE)
				lpListFile->nPosEnd[nFpNum] += (sizeof(DYNLINKEDLIST_TYPE) + (lpListFile->nSizeItem * nItemInBlock));
				//안전을 위해서 update시켜본다.
				// 나중에 속도를 위해서 없을 수도 있다.
				fseek(lpListFile->fp[nFpNum], 0L, SEEK_SET);
				fwrite(&(lpListFile->nPosEnd[nFpNum]), sizeof(tLONG), 1, lpListFile->fp[nFpNum]);
#endif
				break;
			}
			else if (LinkedList.nEnd == 1) { // next 찾아가야지 .. 
				fseek(lpListFile->fp[nFpNum], LinkedList.nNextList, SEEK_SET);
				nJump ++;
			}
			else { // exception
				printf("linkedlist exception errror\n");
				break;
			}
		}
		UnsetFLock(lpListFile->fp[nFpNum]);
	}


	*nRetJump = nJump;
	return TRUE;

}

tBOOL GetItemListFromDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpRecItem)
{
	RECORD RecItem;
	DYNLINKEDLIST_TYPE LinkedList;
	tINT nFpNum = nSrcFpNum;
#if defined(_OLD)
	tBYTE *pBuf;
	tINT i;
#endif
	tBOOL bRetVal = TRUE;

	if (lpListFile->bOpenList == FALSE) return FALSE;
	if (nStPos < 0) return FALSE;

#if defined(_OLD)
	pBuf = (tBYTE *)malloc(lpListFile->nSizeItem);
	if (pBuf == NULL) {
		
		printf("malloc error\n");
		return FALSE;
	}
#endif

	SetFLock(lpListFile->fp[nFpNum]);

	// 위치 옮긴 후 ..
	fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);

	while (1) {
               	// block정보 및 next 정보..
               	fread(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

		AllocRecord(&RecItem, lpListFile->nSizeItem, LinkedList.nCItem, LinkedList.nCItem);
		fread(RecItem.lpBuf, (lpListFile->nSizeItem * LinkedList.nCItem), 1, lpListFile->fp[nFpNum]);
		RecItem.nUsed = LinkedList.nCItem;
		
#if defined(_OLD)
		//결과 record에 담아야지 ..
		for ( i = 0 ; i < RecItem.nUsed ; i ++ ) {
			GetRecord(&RecItem, i, pBuf);
			AppendRecord(lpRecItem, pBuf);
		}
#else
		AddRecordInRecord(lpRecItem, lpRecItem->nUsed, &RecItem);
#endif
		FreeRecord(&RecItem);

		if (LinkedList.nEnd == 0) { // 끝 ..
			break;
		}
		else if (LinkedList.nEnd == 1) { // 다음 블럭으로 점프 ..
			fseek(lpListFile->fp[nFpNum], LinkedList.nNextList, SEEK_SET);
		}
		else { // exception
			bRetVal = FALSE;
			printf("GetItemListFromLinkedList exception error\n");
			break;
		}
	}

	UnsetFLock(lpListFile->fp[nFpNum]);
#if defined(_OLD)
	free(pBuf);
#endif

	return bRetVal;	
}

tBOOL DelItemListFromDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem)
{
	RECORD RecItem;
	DYNLINKEDLIST_TYPE LinkedList;
	tINT nFpNum = nSrcFpNum;
	tINT nPos;
	tBOOL bDelOk = FALSE;
	tINT nChk;
	tBOOL bRetVal = FALSE;

	if (lpListFile->bOptRW == OPEN_READ_LIST) return FALSE;
	if (lpListFile->bOpenList == FALSE) return FALSE;
	if (nStPos < 0) return FALSE;

	SetFLock(lpListFile->fp[nFpNum]);

	// 위치 옮긴 후 ..
	fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);

	while (1) {
		nPos = ftell(lpListFile->fp[nFpNum]);

               	// block정보 및 next 정보..
               	fread(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);
		AllocRecord(&RecItem, lpListFile->nSizeItem, LinkedList.nCItem, LinkedList.nCItem);
		fread(RecItem.lpBuf, (lpListFile->nSizeItem * LinkedList.nCItem), 1, lpListFile->fp[nFpNum]);
		RecItem.nUsed = LinkedList.nCItem;
		
		//비교해서 같으면 지워야지 ..
		bDelOk = FALSE;
		nChk = FindRecord(&RecItem, lpItem, FALSE, lpListFile->CompRecord);
		if (nChk != -1) { // 있다.
			DelRecord(&RecItem, nChk);
			bDelOk = TRUE;
			bRetVal = TRUE;
		}
		if (bDelOk) { // 지웠다..
			fseek(lpListFile->fp[nFpNum], nPos, SEEK_SET);

			LinkedList.nCItem = RecItem.nUsed; // count 정보 다시 넣기
               		fwrite(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

			fwrite(RecItem.lpBuf, (lpListFile->nSizeItem * LinkedList.nCItem), 1, lpListFile->fp[nFpNum]);
			//fflush(lpListFile->fp[nFpNum]);
			//fdatasync(fileno(lpListFile->fp[nFpNum]));
			FreeRecord(&RecItem);
			break;
		}
		FreeRecord(&RecItem);

		if (LinkedList.nEnd == 0) { // 끝 ..
			break;
		}
		else if (LinkedList.nEnd == 1) { // 다음 블럭으로 점프 ..
			fseek(lpListFile->fp[nFpNum], LinkedList.nNextList, SEEK_SET);
		}
		else { // exception
			bRetVal = FALSE;
			printf("Del Linkedlist exception\n");
			break;
		}
	}

	UnsetFLock(lpListFile->fp[nFpNum]);

#if defined(_DEBUG)
	if (bDelOk) {
		printf("delete ok : %d, %d", nFpNum, nPos);
	}
#endif
	return bRetVal;	

}

tBOOL UpdateItemListFromDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem, tVOID *lpUpdateItem)
{
	RECORD RecItem;
	DYNLINKEDLIST_TYPE LinkedList;
	tINT nFpNum = nSrcFpNum;
	tINT nPos;
	tBOOL bUpdateOk = FALSE;
	tINT nChk;
	tBOOL bRetVal = FALSE;

	if (lpListFile->bOptRW == OPEN_READ_LIST) return FALSE;
	if (lpListFile->bOpenList == FALSE) return FALSE;
	if (nStPos < 0) return FALSE;

	SetFLock(lpListFile->fp[nFpNum]);

	// 위치 옮긴 후 ..
	fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);

	while (1) {
		nPos = ftell(lpListFile->fp[nFpNum]);

               	// block정보 및 next 정보..
               	fread(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);
		AllocRecord(&RecItem, lpListFile->nSizeItem, LinkedList.nCItem, LinkedList.nCItem);

		fread(RecItem.lpBuf, (lpListFile->nSizeItem * LinkedList.nCItem), 1, lpListFile->fp[nFpNum]);
		RecItem.nUsed = LinkedList.nCItem;
		
		//비교해서 같으면 Update하기 
		bUpdateOk = FALSE;
		nChk = FindRecord(&RecItem, lpItem, FALSE, lpListFile->CompRecord);
		if (nChk != -1) { // 있다.
			SetRecord(&RecItem, nChk, lpUpdateItem);
			bUpdateOk = TRUE;
			bRetVal = TRUE;
		}
		if (bUpdateOk) { // 지웠다..
			//해당 블록만 쓰면 된다. linkedlist info는 저장안해도 된다.
			fseek(lpListFile->fp[nFpNum], nPos+sizeof(DYNLINKEDLIST_TYPE), SEEK_SET);

			fwrite(RecItem.lpBuf, (lpListFile->nSizeItem * LinkedList.nCItem), 1, lpListFile->fp[nFpNum]);
			FreeRecord(&RecItem);
			//fflush(lpListFile->fp[nFpNum]);
			//fdatasync(fileno(lpListFile->fp[nFpNum]));
			break;
		}
		FreeRecord(&RecItem);

		if (LinkedList.nEnd == 0) { // 끝 ..
			break;
		}
		else if (LinkedList.nEnd == 1) { // 다음 블럭으로 점프 ..
			fseek(lpListFile->fp[nFpNum], LinkedList.nNextList, SEEK_SET);
		}
		else { // exception
			bRetVal = FALSE;
			printf("Update Linkedlist exception\n");
			break;
		}
	}

	UnsetFLock(lpListFile->fp[nFpNum]);

#if defined(_DEBUG)
	if (bUpdateOk) {
		printf("update ok : %d, %d", nFpNum, nPos);
	}
#endif
	return bRetVal;	

}

tBOOL DelItemListArrayFromDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpItemRec)
{
	RECORD RecItem;
	RECORD DelRecItem;
	DYNLINKEDLIST_TYPE LinkedList;
	tINT nFpNum = nSrcFpNum;
	tINT nPos;
	tBOOL bDelOk = FALSE;
	tINT nChk, i;
	tBOOL bRetVal = FALSE;
	tBYTE *lpItem;

	if (lpListFile->bOptRW == OPEN_READ_LIST) return FALSE;
	if (lpListFile->bOpenList == FALSE) return FALSE;
	if (nStPos < 0) return FALSE;

	RecordCpyWithAlloc(&DelRecItem, lpItemRec);
	lpItem = (tBYTE *)malloc(lpListFile->nSizeItem);
	if (lpItem == NULL) {
		FreeRecord(&DelRecItem);
		return FALSE;
	}

	SetFLock(lpListFile->fp[nFpNum]);

	// 위치 옮긴 후 ..
	fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);

	while (1) {
		nPos = ftell(lpListFile->fp[nFpNum]);

               	// block정보 및 next 정보..
               	fread(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);
		AllocRecord(&RecItem, lpListFile->nSizeItem, LinkedList.nCItem, LinkedList.nCItem);
		fread(RecItem.lpBuf, (lpListFile->nSizeItem * LinkedList.nCItem), 1, lpListFile->fp[nFpNum]);
		RecItem.nUsed = LinkedList.nCItem;
		
		bDelOk = FALSE;
		if (RecItem.nUsed >= DelRecItem.nUsed) {
			for ( i = 0 ; i < DelRecItem.nUsed ; i ++ ) {
				GetRecord(&DelRecItem, i, lpItem);
				//비교해서 같으면 지워야지 ..
				nChk = FindRecord(&RecItem, lpItem, FALSE, lpListFile->CompRecord);
				if (nChk != -1) { // 있다.
					DelRecord(&RecItem, nChk);

					// 지운다.
					DelRecord(&DelRecItem, i);
					i --;
					bDelOk = TRUE;
					bRetVal = TRUE;
				}
				if (RecItem.nUsed == 0) break;
			}
		}
		else { // 반대로 해야겠지..?
			for ( i = 0 ; i < RecItem.nUsed ; i ++ ) {
				GetRecord(&RecItem, i, lpItem);
				//비교해서 같으면 지워야지 ..
				nChk = FindRecord(&DelRecItem, lpItem, FALSE, lpListFile->CompRecord);
				if (nChk != -1) { // 있다.
					DelRecord(&DelRecItem, nChk);

					// 지운다.
					DelRecord(&RecItem, i);
					i --;
					bDelOk = TRUE;
					bRetVal = TRUE;
				}
				if (DelRecItem.nUsed == 0) break;
			}

		}
		if (bDelOk) { // 지웠다..
			fseek(lpListFile->fp[nFpNum], nPos, SEEK_SET);

			LinkedList.nCItem = RecItem.nUsed; // count 정보 다시 넣기
               		fwrite(&LinkedList, sizeof(DYNLINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

			fwrite(RecItem.lpBuf, (lpListFile->nSizeItem * LinkedList.nCItem), 1, lpListFile->fp[nFpNum]);
		}

		FreeRecord(&RecItem);

		if (DelRecItem.nUsed == 0) {
			// 더이상 지울 것이 없다.
			break;
		}

		if (LinkedList.nEnd == 0) { // 끝 ..
			break;
		}
		else if (LinkedList.nEnd == 1) { // 다음 블럭으로 점프 ..
			fseek(lpListFile->fp[nFpNum], LinkedList.nNextList, SEEK_SET);
		}
		else { // exception
			bRetVal = FALSE;
			printf("Del Linkedlist exception\n");
			break;
		}
	}

	UnsetFLock(lpListFile->fp[nFpNum]);
#if defined(_DEBUG)
	if (bDelOk) {
		printf("delete ok : delitem : %d -> %d remain", lpItemRec->nUsed, DelRecItem.nUsed);
	}
#endif
	FreeRecord(&DelRecItem);
	free(lpItem);

	return bRetVal;	

}

