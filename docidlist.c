#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(_DEBUG)
#include <sys/time.h>
#include <time.h>
#endif
#include "dwtype.h"
#include "record.h"
#include "dwhashfunc.h"
#include "db.h"
#include "keylist.h"
#include "linkedlist.h"
#include "linkedlistdyn.h"
#include "docidlist.h"
#include "timecheck.h"

typedef struct {
//	tCHAR szKey[MAX_WORD_LEN];
	tINT nC_DocId; // 이 키워드에 달려있는데 키워드 ..
	tINT nStPos;    // linkedlist
	// 맨 끝에 write한 위치 ... write 빠르게 하기 위해서 .. 
	tINT nLastFpNum; 
	tINT nLastPos;
        // dyn을 위해서 ..
	tINT nC_DocIdDyn; // 이 키워드에 달려있는데 키워드 ..
	tINT nStPosDyn; // dyn linkedlist
	// 맨 끝에 write한 위치 ... write 빠르게 하기 위해서 .. 
	/* 이것은 나중에 하던지 하자 ..
	tINT nLastFpNumDyn; 
	tINT nLastPosDyn;
        */
}DOCIDLIST_INFO_TYPE;

tBOOL TravelKeyList(tCHAR *szKey, tINT nLenKey, tVOID *lpData, tINT nSizeData, tVOID *lpArg1, tVOID *lpArg2);
tBOOL ConvNewDocIdList(tCHAR *szKey, tINT nLenKey, tVOID *lpData, tINT nSizeData, tVOID *lpArg1, tVOID *lpArg2);

tBOOL OpenDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *KeyListDBName, tINT nC_KeyListDB, tINT nPageSize, tINT nCacheSize, tCHAR *LinkedListDBName, tINT nC_LinkedListDB, tINT nSizeItem, tINT nItemInBlock, tINT nLimitFileSize, CompProcP CompRecord, tBOOL bOptRW)
{
	char szFileName[MAX_PATH];
	memset(lpDocIdList, 0, sizeof(DOCIDLIST_HEADER_TYPE));
	if (OpenKeyList(&(lpDocIdList->KeyList), KeyListDBName, nC_KeyListDB, nPageSize, nCacheSize) == FALSE) {
		printf("keylist open error[%s]\n",  KeyListDBName);
		return FALSE;
	}
	if (OpenLinkedListFile(&(lpDocIdList->LinkedList), LinkedListDBName, nC_LinkedListDB, nSizeItem, nItemInBlock, nLimitFileSize, CompRecord, bOptRW) == FALSE) {

		printf("linkedlist file open error [%s]\n", LinkedListDBName);
		CloseKeyList(&(lpDocIdList->KeyList));
		return FALSE;
	}
	sprintf(szFileName, "%s_dyn", LinkedListDBName);
	if (OpenDynLinkedListFile(&(lpDocIdList->DynLinkedList), szFileName, nC_LinkedListDB, nSizeItem, nLimitFileSize, CompRecord, bOptRW) == FALSE) {

		printf("dynlinkedlist file open error [%s]\n", szFileName);
		CloseKeyList(&(lpDocIdList->KeyList));
		CloseLinkedListFile(&(lpDocIdList->LinkedList));
		return FALSE;
	}
	lpDocIdList->bOpenDocIdList = TRUE;
	return TRUE;

}

tVOID CloseDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList)
{
	if (lpDocIdList->bOpenDocIdList == FALSE) return;
	CloseKeyList(&(lpDocIdList->KeyList));
	CloseLinkedListFile(&(lpDocIdList->LinkedList));
	CloseDynLinkedListFile(&(lpDocIdList->DynLinkedList));
}

tBOOL PutDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *szKey, tVOID *lpItem, tINT nOpt)
{
	DOCIDLIST_INFO_TYPE DocIdListInfo;
	tINT nRetStPos, nRetLastPos, nRetFpNum;
	tINT nFpNum;
	tINT nJump;

#if defined(_DEBUG)
	TIMECHECK_TYPE timecheck;
	TIMECHECK_TYPE t_timecheck;
	long check_time;

	StartTimeCheck(&timecheck);
	StartTimeCheck(&t_timecheck);
#endif

	nFpNum = Hash_BySeed((unsigned char *)szKey) % lpDocIdList->LinkedList.nCountFp;
	if (GetKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo) == FALSE) { // 없음 ..

#if defined(_DEBUG)
		EndTimeCheck(&timecheck);
		check_time = GetTimeCheck(&timecheck);
		if (check_time > 1000) printf("GetKeyList : %ld microsec\n", check_time);
	
		StartTimeCheck(&timecheck);
#endif
		//strcpy(DocIdListInfo.szKey, szKey);

		if (PutItemToLinkedList(&(lpDocIdList->LinkedList), nFpNum, -1, lpItem, &nRetFpNum, &nRetStPos, &nRetLastPos, &nJump) == FALSE) {
			printf("PutItemToLinkedList Error\n");
			return FALSE;
		}
#if defined(_DEBUG)
		EndTimeCheck(&timecheck);
		check_time = GetTimeCheck(&timecheck);
		if (check_time > 1000) printf("PutItemToLinkedList : %ld microsec, %d jumps, fileno:%d, stpos : %d, lastpos : %d\n", check_time, nJump, nRetFpNum, nRetStPos, nRetLastPos);
	
		StartTimeCheck(&timecheck);
#endif
		DocIdListInfo.nC_DocId = 1; // 전체가 1개이다.
		DocIdListInfo.nStPos = nRetStPos;
		DocIdListInfo.nLastFpNum = nRetFpNum;
		if (nRetLastPos >= 0) DocIdListInfo.nLastPos = nRetLastPos;
		DocIdListInfo.nStPosDyn = -1;
		DocIdListInfo.nC_DocIdDyn = 0; // 전체가 0개이다.

		if (PutKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo, sizeof(DOCIDLIST_INFO_TYPE)) == FALSE) {
			printf("PutKeyList error\n");
			// 다시 지워야지..
			DelItemListFromLinkedList(&(lpDocIdList->LinkedList), DocIdListInfo.nLastFpNum, DocIdListInfo.nStPos, lpItem);
			return FALSE;

		}
#if defined(_DEBUG)
		EndTimeCheck(&timecheck);
		check_time = GetTimeCheck(&timecheck);
		if (check_time > 1000) printf("PutKeyList : %ld microsec\n", check_time);
	
		StartTimeCheck(&timecheck);
#endif
	}
	else { // 있음 ..
		//printf("count : %d\n", DocIdListInfo.nC_DocId);
		//#define OPT_APPEND_DOC_ITEM     0
		//#define OPT_INSERT_DOC_ITEM     1
		if (nOpt == OPT_APPEND_DOC_ITEM) {
		// 맨 끝에 붙여 넣는다 .. 속도때문에 처음부터 찾아 가면 늦으니 맨 끝에 다시 붙인다...
			if (DocIdListInfo.nC_DocIdDyn > 0 && DocIdListInfo.nC_DocId < 1000) { // 처음부터 해야 한다. 큰 block이 있으니깐 빈 곳이 있을테니.. 그곳에 넣어야지..
				if (PutItemToLinkedList(&(lpDocIdList->LinkedList), DocIdListInfo.nLastFpNum, DocIdListInfo.nStPos, lpItem, &nRetFpNum, &nRetStPos, &nRetLastPos, &nJump) == FALSE) {
					printf("PutItemToLinkedList Error\n");
					return FALSE;
				}
			}
			else {
				if (PutItemToLinkedList(&(lpDocIdList->LinkedList), DocIdListInfo.nLastFpNum, DocIdListInfo.nLastPos, lpItem, &nRetFpNum, &nRetStPos, &nRetLastPos, &nJump) == FALSE) {
					printf("PutItemToLinkedList Error\n");
					return FALSE;
				}
			}
#if defined(_DEBUG)
		EndTimeCheck(&timecheck);
		check_time = GetTimeCheck(&timecheck);
		if (check_time > 1000) printf("%s : PutItemToLinkedList : %ld microsec, append %d jumps, fileno:%d, stpos : %d, lastpos : %d\n", szKey, check_time, nJump, nRetFpNum, DocIdListInfo.nStPos, nRetLastPos);
	
		StartTimeCheck(&timecheck);
#endif
		
			DocIdListInfo.nC_DocId += 1; // 추가 ..
			DocIdListInfo.nLastFpNum = nRetFpNum;
			if (nRetLastPos >= 0) DocIdListInfo.nLastPos = nRetLastPos;
			UpdateKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo, sizeof(DOCIDLIST_INFO_TYPE));
#if defined(_DEBUG)
		EndTimeCheck(&timecheck);
		check_time = GetTimeCheck(&timecheck);
		if (check_time > 1000) printf("UpdateKeyList : %ld microsec\n", check_time);
	
		StartTimeCheck(&timecheck);
#endif
		}
		else {
			if (PutItemToLinkedList(&(lpDocIdList->LinkedList), nFpNum, DocIdListInfo.nStPos, lpItem, &nRetFpNum, &nRetStPos, &nRetLastPos, &nJump) == FALSE) {
				printf("PutItemToLinkedList Error\n");
				return FALSE;
			}
#if defined(_DEBUG)
		EndTimeCheck(&timecheck);
		check_time = GetTimeCheck(&timecheck);
		if (check_time > 1000) printf("%s : PutItemToLinkedList : %ld microsec, %d jumps, fileno:%d, stpos : %d, lastpos : %d\n", szKey, check_time, nJump, nRetFpNum, DocIdListInfo.nStPos, nRetLastPos);
	
		StartTimeCheck(&timecheck);
#endif
			DocIdListInfo.nC_DocId += 1; // 추가 ..
			DocIdListInfo.nLastFpNum = nRetFpNum;
			if (nRetLastPos >= 0) DocIdListInfo.nLastPos = nRetLastPos;
			UpdateKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo, sizeof(DOCIDLIST_INFO_TYPE));
#if defined(_DEBUG)
		EndTimeCheck(&timecheck);
		check_time = GetTimeCheck(&timecheck);
		if (check_time > 1000) printf("UpdateKeyList : %ld microsec\n", check_time);
	
		StartTimeCheck(&timecheck);
#endif
		}
	}
#if defined(_DEBUG)
	EndTimeCheck(&t_timecheck);
	check_time = GetTimeCheck(&t_timecheck);
	if (check_time > 1000) printf("PutDocIdList Total Time : %ld microsec\n", check_time);

#endif

	return TRUE;
}

tBOOL DelDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *szKey, tVOID *lpItem)
{
	DOCIDLIST_INFO_TYPE DocIdListInfo;
	tINT nFpNum;
	tBOOL bRetVal = FALSE;

	if (GetKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo) == FALSE) { // 없음 ..
		// 없으니 지웠다고 본다.
		return TRUE;
	}
	if (DocIdListInfo.nStPos == -1 && DocIdListInfo.nStPosDyn == -1) return FALSE;
	// 있으면 지워야지 .. 
	if (DocIdListInfo.nLastFpNum >= 0) {
		nFpNum = DocIdListInfo.nLastFpNum;
	}
	else {
		nFpNum = Hash_BySeed((unsigned char *)szKey) % lpDocIdList->LinkedList.nCountFp;
	}
	// block linkedlist delete
	if (DelItemListFromLinkedList(&(lpDocIdList->LinkedList), nFpNum, DocIdListInfo.nStPos, lpItem)) {
		DocIdListInfo.nC_DocId -= 1; // 추가 ..
		if (DocIdListInfo.nC_DocId < 0) DocIdListInfo.nC_DocId = 0;
		bRetVal = TRUE;
	}
	// dyn block linkedlist delete
	if (bRetVal == FALSE && DelItemListFromDynLinkedList(&(lpDocIdList->DynLinkedList), nFpNum, DocIdListInfo.nStPosDyn, lpItem)) {
		DocIdListInfo.nC_DocIdDyn -= 1; 
		if (DocIdListInfo.nC_DocIdDyn < 0) DocIdListInfo.nC_DocIdDyn = 0;
		bRetVal = TRUE;
	}
	if (bRetVal) {
		UpdateKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo, sizeof(DOCIDLIST_INFO_TYPE));
	}
	return bRetVal;
}

tBOOL GetDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *szKey, RECORD *lpRecDocIdList)
{
	DOCIDLIST_INFO_TYPE DocIdListInfo;
	tINT nFpNum;
	RECORD RetDocIdList;

	if (GetKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo) == FALSE) { // 없음 ..
		return FALSE;
	}
	if (DocIdListInfo.nStPos == -1 && DocIdListInfo.nStPosDyn == -1) return FALSE;
	if (DocIdListInfo.nC_DocId <= 0 && DocIdListInfo.nC_DocIdDyn <= 0)  return FALSE; // 없다.
//	printf("count : %d\n", DocIdListInfo.nC_DocId);

	if (DocIdListInfo.nLastFpNum >= 0) {
		nFpNum = DocIdListInfo.nLastFpNum;
	}
	else {
		nFpNum = Hash_BySeed((unsigned char *)szKey) % lpDocIdList->LinkedList.nCountFp;
	}
	AllocRecord(&RetDocIdList, lpRecDocIdList->nWidth, lpRecDocIdList->nAlloc, lpRecDocIdList->nInc);
	
	GetItemListFromLinkedList(&(lpDocIdList->LinkedList), nFpNum, DocIdListInfo.nStPos, &RetDocIdList);
	if (RetDocIdList.nUsed) {
		AddRecordInRecord(lpRecDocIdList, lpRecDocIdList->nUsed, &RetDocIdList);
	}
	if (RetDocIdList.nUsed != DocIdListInfo.nC_DocId) {
		printf("check RetDocIdList.nUsed : %d, DocIdListInfo.nC_DocId : %d\n", RetDocIdList.nUsed, DocIdListInfo.nC_DocId);
	}

	if (DocIdListInfo.nC_DocIdDyn > 0) {
		RetDocIdList.nUsed = 0;
		GetItemListFromDynLinkedList(&(lpDocIdList->DynLinkedList), nFpNum, DocIdListInfo.nStPosDyn, &RetDocIdList);
		if (RetDocIdList.nUsed != DocIdListInfo.nC_DocIdDyn) {
			printf("check RetDocIdList.nUsed : %d, DocIdListInfo.nC_DocIdDyn : %d\n", RetDocIdList.nUsed, DocIdListInfo.nC_DocIdDyn);
		}
		if (RetDocIdList.nUsed) {
			AddRecordInRecord(lpRecDocIdList, lpRecDocIdList->nUsed, &RetDocIdList);
		}
	}

	return TRUE;
	
}

tBOOL UpdateDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *szKey, tVOID *lpItem, tVOID *lpUpdateItem)
{
	DOCIDLIST_INFO_TYPE DocIdListInfo;
	tINT nFpNum;
	tBOOL bRetVal = FALSE;

	if (GetKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo) == FALSE) { // 없음 ..
		return FALSE;
	}
	if (DocIdListInfo.nStPos == -1 && DocIdListInfo.nStPosDyn == -1) return FALSE;
	if (DocIdListInfo.nLastFpNum >= 0) {
		nFpNum = DocIdListInfo.nLastFpNum;
	}
	else {
		nFpNum = Hash_BySeed((unsigned char *)szKey) % lpDocIdList->LinkedList.nCountFp;
	}
	
	bRetVal = UpdateItemListFromLinkedList(&(lpDocIdList->LinkedList), nFpNum, DocIdListInfo.nStPos, lpItem, lpUpdateItem);
	if (bRetVal) return TRUE; // update ok
	bRetVal = UpdateItemListFromDynLinkedList(&(lpDocIdList->DynLinkedList), nFpNum, DocIdListInfo.nStPosDyn, lpItem, lpUpdateItem);

	return (bRetVal);
}

/*
	linkedlist에서 몇번 뛰고 있는지 체크해서 nJumpCount 이상되면 dyn linkedlist로 convert시킨다.
 */
tBOOL LinkedList2DynLinkedList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tINT nBlockCount)
{
	DisplayKeyList(&(lpDocIdList->KeyList), TravelKeyList, lpDocIdList, &nBlockCount);
//KEYLIST_TYPE *lpKeyList, PutKeyListFuncP PutKeyListFunc, tVOID *lpArg1, tVOID *lpArg2 );
	return TRUE;
}

tBOOL TravelKeyList(tCHAR *szKey, tINT nLenKey, tVOID *lpData, tINT nSizeData, tVOID *lpArg1, tVOID *lpArg2)
{
	DOCIDLIST_INFO_TYPE DocIdListInfo;
	DOCIDLIST_HEADER_TYPE *lpDocIdList = (DOCIDLIST_HEADER_TYPE *)lpArg1;
	tINT nBlockCount = (tINT)(*(tINT *)lpArg2);
	tINT nBlock;
	RECORD RetDocIdList;
	tINT nFpNum;
	tBOOL bOkDel = FALSE;
	tINT nRetFpNum, nRetStPos, nRetLastPos, nRetJump;
	tCHAR checkKey[1000];


	memcpy(checkKey, szKey, nLenKey);
	checkKey[nLenKey] = '\0';

	memcpy(&DocIdListInfo, lpData, nSizeData);	
	if (DocIdListInfo.nC_DocId <= 0) return TRUE; // skip

	nBlock = DocIdListInfo.nC_DocId / lpDocIdList->LinkedList.nItemInBlock; 
	if (DocIdListInfo.nC_DocId % lpDocIdList->LinkedList.nItemInBlock) {
		nBlock ++;
	}
	if (nBlock >= nBlockCount) { // block count가 넘치면 ..

		AllocRecord(&RetDocIdList, lpDocIdList->LinkedList.nSizeItem, DocIdListInfo.nC_DocId + 1, DocIdListInfo.nC_DocId);

		if (DocIdListInfo.nLastFpNum >= 0) {
			nFpNum = DocIdListInfo.nLastFpNum;
		}
		else {
			nFpNum = Hash_BySeed((unsigned char *)checkKey) % lpDocIdList->LinkedList.nCountFp;
		}
		GetItemListFromLinkedList(&(lpDocIdList->LinkedList), nFpNum, DocIdListInfo.nStPos, &RetDocIdList);
#if defined(_DEBUG)
		if (DocIdListInfo.nC_DocId != RetDocIdList.nUsed) { //exception
			printf("check : %s, %d , %d\n", checkKey, DocIdListInfo.nC_DocId, RetDocIdList.nUsed);
			printf("block : %d %d\n", nBlock, nBlockCount);
			return TRUE;
		}
#endif
		if (RetDocIdList.nUsed > 0) {
			// linkedlist 에서는 제거하기..
			if (DelItemListArrayFromLinkedList(&(lpDocIdList->LinkedList), nFpNum, DocIdListInfo.nStPos, &RetDocIdList)) {
				DocIdListInfo.nC_DocId -= RetDocIdList.nUsed;
				// dynamic linkedlist에 넣기
				PutItemToDynLinkedList(&(lpDocIdList->DynLinkedList), nFpNum, DocIdListInfo.nStPosDyn, RetDocIdList.lpBuf, RetDocIdList.nUsed, &nRetFpNum, &nRetStPos, &nRetLastPos, &nRetJump);
				DocIdListInfo.nC_DocIdDyn += RetDocIdList.nUsed;
				DocIdListInfo.nStPosDyn = nRetStPos;
				bOkDel = TRUE;
			}
			
		}	
		FreeRecord(&RetDocIdList);
	}
	if (bOkDel) { // docidinfo update시키기 ..
		UpdateKeyList(&(lpDocIdList->KeyList), checkKey, &DocIdListInfo, sizeof(DOCIDLIST_INFO_TYPE));
#if defined(_DEBUG)
		printf("key %s, blockcount : %d, nC_DocId : %d, nC_DocIdDyn : %d\n", checkKey, nBlock, DocIdListInfo.nC_DocId, DocIdListInfo.nC_DocIdDyn);
#endif
	}
	return TRUE;
}

tBOOL DocIdList2NewDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, DOCIDLIST_HEADER_TYPE *lpNewDocIdList)
{
	DisplayKeyList(&(lpDocIdList->KeyList), ConvNewDocIdList, lpDocIdList, lpNewDocIdList);
	return TRUE;
}

tBOOL ConvNewDocIdList(tCHAR *szKey, tINT nLenKey, tVOID *lpData, tINT nSizeData, tVOID *lpArg1, tVOID *lpArg2)
{
	DOCIDLIST_INFO_TYPE DocIdListInfo;
	DOCIDLIST_HEADER_TYPE *lpDocIdList = (DOCIDLIST_HEADER_TYPE *)lpArg1;
	DOCIDLIST_HEADER_TYPE *lpNewDocIdList = (DOCIDLIST_HEADER_TYPE *)lpArg2;
	RECORD RetDocIdList;
	tINT nFpNum;
	tINT nRetFpNum, nRetStPos, nRetLastPos, nRetJump;
	tCHAR checkKey[1000];

	memcpy(checkKey, szKey, nLenKey);
	checkKey[nLenKey] = '\0';
	memcpy(&DocIdListInfo, lpData, nSizeData);	


	if (DocIdListInfo.nC_DocId <= 0 && DocIdListInfo.nC_DocIdDyn <= 0) return TRUE; // skip

	AllocRecord(&RetDocIdList, lpDocIdList->LinkedList.nSizeItem, DocIdListInfo.nC_DocId + DocIdListInfo.nC_DocIdDyn + 1, DocIdListInfo.nC_DocId + DocIdListInfo.nC_DocIdDyn);

	GetDocIdList(lpDocIdList, checkKey, &RetDocIdList);
#if defined(_DEBUG)
	if (strcmp(checkKey, "드림위즈") == 0) {
		printf("%s : nC_DocId : %d, nC_DocIdDyn : %d, RetDocIdList.nUsed : %d\n", checkKey, DocIdListInfo.nC_DocId, DocIdListInfo.nC_DocIdDyn, RetDocIdList.nUsed);
	}
#endif
	if (RetDocIdList.nUsed) {
		if (DocIdListInfo.nLastFpNum >= 0) {
			nFpNum = DocIdListInfo.nLastFpNum;
		}
		else {
			nFpNum = Hash_BySeed((unsigned char *)checkKey) % lpNewDocIdList->LinkedList.nCountFp;
		}

		PutItemToDynLinkedList(&(lpNewDocIdList->DynLinkedList), nFpNum, -1, RetDocIdList.lpBuf, RetDocIdList.nUsed, &nRetFpNum, &nRetStPos, &nRetLastPos, &nRetJump);

		memset(&DocIdListInfo, 0, sizeof(DOCIDLIST_INFO_TYPE));

		DocIdListInfo.nC_DocId = 0; // block linkedlist는 없다.
		DocIdListInfo.nStPos = -1;
		DocIdListInfo.nLastPos = -1;

		DocIdListInfo.nLastFpNum = nFpNum;
		// dyn linkedlist write 했으니 .. 
		DocIdListInfo.nStPosDyn = nRetStPos;
		DocIdListInfo.nC_DocIdDyn = RetDocIdList.nUsed; 
		// key put
		PutKeyList(&(lpNewDocIdList->KeyList), checkKey, &DocIdListInfo, sizeof(DOCIDLIST_INFO_TYPE));
	}

	FreeRecord(&RetDocIdList);

	return TRUE;
}

