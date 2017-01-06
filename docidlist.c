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
	tINT nC_DocId; // �� Ű���忡 �޷��ִµ� Ű���� ..
	tINT nStPos;    // linkedlist
	// �� ���� write�� ��ġ ... write ������ �ϱ� ���ؼ� .. 
	tINT nLastFpNum; 
	tINT nLastPos;
        // dyn�� ���ؼ� ..
	tINT nC_DocIdDyn; // �� Ű���忡 �޷��ִµ� Ű���� ..
	tINT nStPosDyn; // dyn linkedlist
	// �� ���� write�� ��ġ ... write ������ �ϱ� ���ؼ� .. 
	/* �̰��� ���߿� �ϴ��� ���� ..
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
	if (GetKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo) == FALSE) { // ���� ..

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
		DocIdListInfo.nC_DocId = 1; // ��ü�� 1���̴�.
		DocIdListInfo.nStPos = nRetStPos;
		DocIdListInfo.nLastFpNum = nRetFpNum;
		if (nRetLastPos >= 0) DocIdListInfo.nLastPos = nRetLastPos;
		DocIdListInfo.nStPosDyn = -1;
		DocIdListInfo.nC_DocIdDyn = 0; // ��ü�� 0���̴�.

		if (PutKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo, sizeof(DOCIDLIST_INFO_TYPE)) == FALSE) {
			printf("PutKeyList error\n");
			// �ٽ� ��������..
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
	else { // ���� ..
		//printf("count : %d\n", DocIdListInfo.nC_DocId);
		//#define OPT_APPEND_DOC_ITEM     0
		//#define OPT_INSERT_DOC_ITEM     1
		if (nOpt == OPT_APPEND_DOC_ITEM) {
		// �� ���� �ٿ� �ִ´� .. �ӵ������� ó������ ã�� ���� ������ �� ���� �ٽ� ���δ�...
			if (DocIdListInfo.nC_DocIdDyn > 0 && DocIdListInfo.nC_DocId < 1000) { // ó������ �ؾ� �Ѵ�. ū block�� �����ϱ� �� ���� �����״�.. �װ��� �־����..
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
		
			DocIdListInfo.nC_DocId += 1; // �߰� ..
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
			DocIdListInfo.nC_DocId += 1; // �߰� ..
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

	if (GetKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo) == FALSE) { // ���� ..
		// ������ �����ٰ� ����.
		return TRUE;
	}
	if (DocIdListInfo.nStPos == -1 && DocIdListInfo.nStPosDyn == -1) return FALSE;
	// ������ �������� .. 
	if (DocIdListInfo.nLastFpNum >= 0) {
		nFpNum = DocIdListInfo.nLastFpNum;
	}
	else {
		nFpNum = Hash_BySeed((unsigned char *)szKey) % lpDocIdList->LinkedList.nCountFp;
	}
	// block linkedlist delete
	if (DelItemListFromLinkedList(&(lpDocIdList->LinkedList), nFpNum, DocIdListInfo.nStPos, lpItem)) {
		DocIdListInfo.nC_DocId -= 1; // �߰� ..
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

	if (GetKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo) == FALSE) { // ���� ..
		return FALSE;
	}
	if (DocIdListInfo.nStPos == -1 && DocIdListInfo.nStPosDyn == -1) return FALSE;
	if (DocIdListInfo.nC_DocId <= 0 && DocIdListInfo.nC_DocIdDyn <= 0)  return FALSE; // ����.
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

	if (GetKeyList(&(lpDocIdList->KeyList), szKey, &DocIdListInfo) == FALSE) { // ���� ..
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
	linkedlist���� ��� �ٰ� �ִ��� üũ�ؼ� nJumpCount �̻�Ǹ� dyn linkedlist�� convert��Ų��.
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
	if (nBlock >= nBlockCount) { // block count�� ��ġ�� ..

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
			// linkedlist ������ �����ϱ�..
			if (DelItemListArrayFromLinkedList(&(lpDocIdList->LinkedList), nFpNum, DocIdListInfo.nStPos, &RetDocIdList)) {
				DocIdListInfo.nC_DocId -= RetDocIdList.nUsed;
				// dynamic linkedlist�� �ֱ�
				PutItemToDynLinkedList(&(lpDocIdList->DynLinkedList), nFpNum, DocIdListInfo.nStPosDyn, RetDocIdList.lpBuf, RetDocIdList.nUsed, &nRetFpNum, &nRetStPos, &nRetLastPos, &nRetJump);
				DocIdListInfo.nC_DocIdDyn += RetDocIdList.nUsed;
				DocIdListInfo.nStPosDyn = nRetStPos;
				bOkDel = TRUE;
			}
			
		}	
		FreeRecord(&RetDocIdList);
	}
	if (bOkDel) { // docidinfo update��Ű�� ..
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
	if (strcmp(checkKey, "�帲����") == 0) {
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

		DocIdListInfo.nC_DocId = 0; // block linkedlist�� ����.
		DocIdListInfo.nStPos = -1;
		DocIdListInfo.nLastPos = -1;

		DocIdListInfo.nLastFpNum = nFpNum;
		// dyn linkedlist write ������ .. 
		DocIdListInfo.nStPosDyn = nRetStPos;
		DocIdListInfo.nC_DocIdDyn = RetDocIdList.nUsed; 
		// key put
		PutKeyList(&(lpNewDocIdList->KeyList), checkKey, &DocIdListInfo, sizeof(DOCIDLIST_INFO_TYPE));
	}

	FreeRecord(&RetDocIdList);

	return TRUE;
}

