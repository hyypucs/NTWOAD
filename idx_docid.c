/*
	키워드 + docidlist 에 대해서 한꺼번에 저장하고 이를 이용할 수 있는 lib 구성

	색인할 때 
		
 		키워드 + docid + info  의 pair로 해서 중간파일을 생성하고
		생성된 중간파일을 sorting하여 
		키워드 + (docid+info)List 를 구성하여 이 라이브러리를 이용해서
		하나하나 write시킨다.

		키워드 idx 구조
		키워드|docidlist_file_num|file에서 위치|docid개수|
		
		docidlist 구조는 (docid+info)가 개수만큼 들어가 있다.


		키워드 찾을 때는 메모리에 다 올릴 수 없으니
		2단계의 bsearch를 이용한다. 
		먼저 4k마다의 키워드를 올린 후 bsearch해서 들어가 있는 해당 블럭을 찾고 
		해당 블럭에서 다시 한번 더 bsearch한다.

		Programmer : hyypucs
		Date : 2008. 08. 29 begin..
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "dwtype.h"
#include "util.h"
#include "record.h"
#include "idx_docid.h"

tBOOL InitIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szFileName, tINT nCount_File, tINT nMode)
{
	tCHAR szIdxName[MAX_PATH];
	tCHAR szDocIdListName[MAX_PATH];
	IDX_ITEM_FORMAT IdxItem;
	tINT i, j, nSize, nCount;

	if (nCount_File > MAX_IDXDOCIDFILE) { // exception
		nCount_File = MAX_IDXDOCIDFILE;
	}
	memset(lpIdxDocId, 0, sizeof(IDX_DOCID_FORMAT));

	for ( i = 0 ; i < nCount_File ; i ++ ) {
		sprintf(szIdxName, "%s.idx.%d", szFileName, i);
		sprintf(szDocIdListName, "%s.docidlist.%d", szFileName, i);


		if (nMode == WRITE_IDXDOCID) {
			lpIdxDocId->fpIdx[i] = fopen(szIdxName, "wb");	
			lpIdxDocId->fpDocIdList[i] = fopen(szDocIdListName, "wb");	
			lpIdxDocId->nCountIdxKey[i] = 0;
		}
		else {
			lpIdxDocId->fpIdx[i] = fopen(szIdxName, "rb");	
			lpIdxDocId->fpDocIdList[i] = fopen(szDocIdListName, "rb");	

			if (lpIdxDocId->fpIdx[i] != NULL) {
				fseek(lpIdxDocId->fpIdx[i], 0L, SEEK_END);
				nSize = ftell(lpIdxDocId->fpIdx[i]);

				// keyword 개수
				lpIdxDocId->nCountIdxKey[i] = nSize/sizeof(IDX_ITEM_FORMAT);

				nCount = nSize / BLOCK_IDX;
				if (nSize % BLOCK_IDX != 0) {
					nCount ++;
				}
				
				AllocRecord(&(lpIdxDocId->IdxKey[i]), sizeof(IDX_ITEM_FORMAT), nCount, 2);

				for ( j = 0 ; j < nCount ; j ++ ) {
					fseek(lpIdxDocId->fpIdx[i], (long)(j*BLOCK_IDX), SEEK_SET);
					fread(&IdxItem, sizeof(IDX_ITEM_FORMAT), 1, lpIdxDocId->fpIdx[i]);
					AppendRecord(&(lpIdxDocId->IdxKey[i]), &IdxItem);
				}
				fseek(lpIdxDocId->fpIdx[i], 0L, SEEK_SET);
			}
		}
		if (lpIdxDocId->fpIdx[i] == NULL) {

			if (lpIdxDocId->fpDocIdList[i]) fclose(lpIdxDocId->fpDocIdList[i]);

			printf("%s open error\n", szIdxName);
			break;
		}
		if (lpIdxDocId->fpDocIdList[i] == NULL) {
			printf("%s open error\n", szDocIdListName);
			fclose(lpIdxDocId->fpIdx[i]);
			if (nMode == READ_IDXDOCID) FreeRecord(&(lpIdxDocId->IdxKey[i]));
			break;
		}

		printf("idx : %s\ndocidlist : %s\nread ok\n", szIdxName, szDocIdListName);
	}
	if (i < nCount_File) { // error
		for ( j = 0 ; j < i ; j ++ ) {
			fclose(lpIdxDocId->fpIdx[j]);
			fclose(lpIdxDocId->fpDocIdList[j]);
			if (nMode == READ_IDXDOCID) FreeRecord(&(lpIdxDocId->IdxKey[j]));
		}
		return FALSE;
	}

	strcpy(lpIdxDocId->szFileName, szFileName);
	lpIdxDocId->nCount_File = nCount_File;
	lpIdxDocId->nMode = nMode;

	return TRUE;
}

tBOOL SyncIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId)
{
	tINT i;
	tCHAR szIdxName[MAX_PATH];
	tCHAR szDocIdListName[MAX_PATH];

	for ( i = 0 ; i < lpIdxDocId->nCount_File ; i ++ ) {
		sprintf(szIdxName, "%s.idx.%d", lpIdxDocId->szFileName, i);
		sprintf(szDocIdListName, "%s.docidlist.%d", lpIdxDocId->szFileName, i);

		fclose(lpIdxDocId->fpIdx[i]);
		fclose(lpIdxDocId->fpDocIdList[i]);

		lpIdxDocId->fpIdx[i] = fopen(szIdxName, "a+b");	
		lpIdxDocId->fpDocIdList[i] = fopen(szDocIdListName, "a+b");

		printf("sync idx : %s\ndocidlist : %s\nclose ok\n", szIdxName, szDocIdListName);
	}
	return TRUE;

}

tBOOL CloseIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId)
{
	tINT i;
	tCHAR szIdxName[MAX_PATH];
	tCHAR szDocIdListName[MAX_PATH];

	for ( i = 0 ; i < lpIdxDocId->nCount_File ; i ++ ) {
		sprintf(szIdxName, "%s.idx.%d", lpIdxDocId->szFileName, i);
		sprintf(szDocIdListName, "%s.docidlist.%d", lpIdxDocId->szFileName, i);

		fclose(lpIdxDocId->fpIdx[i]);
		fclose(lpIdxDocId->fpDocIdList[i]);
		if (lpIdxDocId->nMode == READ_IDXDOCID) FreeRecord(&(lpIdxDocId->IdxKey[i]));

		printf("idx : %s\ndocidlist : %s\nclose ok\n", szIdxName, szDocIdListName);
	}
	return TRUE;
}


tINT GetWriteIdxNum(IDX_DOCID_FORMAT *lpIdxDocId)
{
	tINT i;

	for ( i = 0 ; i < lpIdxDocId->nCount_File ; i ++ ) {
		if (lpIdxDocId->nCountIdxKey[i] < (tINT)MAX_WRITE_IDX_LIMIT - 1) {
			return i;
		}
	}
	return -1; // exception
}

tBOOL WriteIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szKey, RECORD *lpDocIdList)
{
	tINT nPos;
	IDX_ITEM_FORMAT IdxItem;

	nPos = GetWriteIdxNum(lpIdxDocId);
	if (nPos == -1) {
		printf("write file not found ..\n");
		return FALSE;
	}
	SetFLock(lpIdxDocId->fpIdx[nPos]);
	SetFLock(lpIdxDocId->fpDocIdList[nPos]);
	// 계속 append이라 fseek하지 않는다. 혹시 안되면 fseek넣어야 한다.
	// 맨 끝에 붙여야 하니..
	strcpy(IdxItem.szKey, szKey);
	IdxItem.nPos = ftell(lpIdxDocId->fpDocIdList[nPos]);
	IdxItem.nCountDocId = lpDocIdList->nUsed;
	IdxItem.nWidthDocId = lpDocIdList->nWidth;

	// docidlist write
	fwrite(lpDocIdList->lpBuf, lpDocIdList->nWidth * lpDocIdList->nUsed, 1, lpIdxDocId->fpDocIdList[nPos]);
	fwrite(&IdxItem, sizeof(IDX_ITEM_FORMAT), 1, lpIdxDocId->fpIdx[nPos]);

	// idxkey 추가되었으니.. 
	(lpIdxDocId->nCountIdxKey[nPos]) ++;

	UnsetFLock(lpIdxDocId->fpIdx[nPos]);
	UnsetFLock(lpIdxDocId->fpDocIdList[nPos]);

	return TRUE;
}

tINT CmpIdxKeyItem(IDX_ITEM_FORMAT *item1, IDX_ITEM_FORMAT *item2)
{
	return (strcmp(item1->szKey, item2->szKey));
}

tBOOL ReadIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szKey, RECORD *lpDocIdList, IDX_ITEM_FORMAT *RetIdxItem, tBOOL bReadIdx)
{
	tINT i, nChk, nPos;
	IDX_ITEM_FORMAT IdxItem, IdxItem1, IdxItem2;
	RECORD RecIdxKey;
	tBOOL bAllMatch = FALSE;
	tINT nCountRead;

	for ( i = 0 ; i < lpIdxDocId->nCount_File ; i ++ ) {
		if (lpIdxDocId->IdxKey[i].nUsed <= 0) continue; // 없을 수도 있으니 .. 

		GetRecord(&(lpIdxDocId->IdxKey[i]), 0, &IdxItem1);
		if (i != lpIdxDocId->nCount_File - 1 && lpIdxDocId->IdxKey[i+1].nUsed > 0) {
			GetRecord(&(lpIdxDocId->IdxKey[i+1]), 0, &IdxItem2);
			if (strcmp(szKey, IdxItem1.szKey) >= 0 && strcmp(szKey, IdxItem2.szKey) < 0) { //해당 섹션..
				break;

			}
		}
		else {
			if (strcmp(szKey, IdxItem1.szKey) >= 0) break;
		}
	}
	
	if (i >= lpIdxDocId->nCount_File) { // 없다..
		return FALSE;
	}
	nPos = i;

	nChk = CheckIndexKey(&(lpIdxDocId->IdxKey[nPos]), szKey, &IdxItem, &bAllMatch);
	if (nChk == -1) { // 없다..
		return FALSE;
	}
	else if (bAllMatch == FALSE) { // 벌써 IdxItem를 찾은 것이 아니면 ..
		SetFLock(lpIdxDocId->fpIdx[nPos]);
		fseek(lpIdxDocId->fpIdx[nPos], nChk * BLOCK_IDX , SEEK_SET);

		if (nChk * 100 + 100 > lpIdxDocId->nCountIdxKey[nPos]) {
			nCountRead = lpIdxDocId->nCountIdxKey[nPos] - (nChk * 100);
		}
		else {
			nCountRead = COUNT_IDX_BYBLOCK;
		}
		AllocRecord(&RecIdxKey, sizeof(IDX_ITEM_FORMAT), nCountRead, 2);
		fread(RecIdxKey.lpBuf, nCountRead * sizeof(IDX_ITEM_FORMAT), 1, lpIdxDocId->fpIdx[nPos]);
		RecIdxKey.nUsed = nCountRead;
		UnsetFLock(lpIdxDocId->fpIdx[nPos]);

		strcpy(IdxItem1.szKey, szKey);
		nChk = FindRecord(&RecIdxKey, &IdxItem1, TRUE, (CompProcP)CmpIdxKeyItem);
		if (nChk >= 0) {
			GetRecord(&RecIdxKey, nChk, &IdxItem); // 해당 key정보 얻기..
		}
		FreeRecord(&RecIdxKey);
		if (nChk < 0) return FALSE; // 없다..
	}
	// IdxItem 정보 안에 docidlist 정보가 있다...

	if (bReadIdx) {
		memcpy(&RetIdxItem, &IdxItem, sizeof(IDX_ITEM_FORMAT));
		return TRUE; // idx info만 찾았으니..
	}

	// 찾아보자..^^
	SetFLock(lpIdxDocId->fpDocIdList[nPos]);
	fseek(lpIdxDocId->fpDocIdList[nPos], IdxItem.nPos , SEEK_SET);
	AllocRecord(lpDocIdList, IdxItem.nWidthDocId, IdxItem.nCountDocId, 2);

	fread(lpDocIdList->lpBuf, IdxItem.nWidthDocId*IdxItem.nCountDocId, 1, lpIdxDocId->fpDocIdList[nPos]);
	lpDocIdList->nUsed = IdxItem.nCountDocId;
	UnsetFLock(lpIdxDocId->fpDocIdList[nPos]);

	return TRUE;
}

tBOOL DelIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szKey, tVOID *lpDocId, CompProcP CompDocId)
{
	tINT i, nChk, nPos, nChkTemp, nChkTemp1;
	IDX_ITEM_FORMAT IdxItem, IdxItem1, IdxItem2;
	RECORD RecIdxKey;
	RECORD DocIdList;
	tBOOL bAllMatch = FALSE;
	tINT nCountRead;

	for ( i = 0 ; i < lpIdxDocId->nCount_File ; i ++ ) {
		GetRecord(&(lpIdxDocId->IdxKey[i]), 0, &IdxItem1);
		if (i != lpIdxDocId->nCount_File - 1) {
			GetRecord(&(lpIdxDocId->IdxKey[i+1]), 0, &IdxItem2);
			if (strcmp(szKey, IdxItem1.szKey) >= 0 && strcmp(szKey, IdxItem2.szKey) < 0) { //해당 섹션..
				break;

			}
		}
		else {
			if (strcmp(szKey, IdxItem1.szKey) >= 0) break;
		}
	}
	
	if (i >= lpIdxDocId->nCount_File) { // 없다..
		return FALSE;
	}
	nPos = i;

	nChk = CheckIndexKey(&(lpIdxDocId->IdxKey[nPos]), szKey, &IdxItem, &bAllMatch);
	nChkTemp = nChk;
	if (nChk == -1) { // 없다..
		return FALSE;
	}
	else if (bAllMatch == FALSE) { //  IdxItem를 찾은 것 아니면..
		SetFLock(lpIdxDocId->fpIdx[nPos]);
		fseek(lpIdxDocId->fpIdx[nPos], nChk * BLOCK_IDX , SEEK_SET);

		if (nChk * 100 + 100 > lpIdxDocId->nCountIdxKey[nPos]) {
			nCountRead = lpIdxDocId->nCountIdxKey[nPos] - (nChk * 100);
		}
		else {
			nCountRead = COUNT_IDX_BYBLOCK;
		}
		AllocRecord(&RecIdxKey, sizeof(IDX_ITEM_FORMAT), nCountRead, 2);
		fread(RecIdxKey.lpBuf, nCountRead * sizeof(IDX_ITEM_FORMAT), 1, lpIdxDocId->fpIdx[nPos]);
		RecIdxKey.nUsed = nCountRead;
		UnsetFLock(lpIdxDocId->fpIdx[nPos]);

		strcpy(IdxItem1.szKey, szKey);
		nChk = FindRecord(&RecIdxKey, &IdxItem1, TRUE, (CompProcP)CmpIdxKeyItem);
		if (nChk >= 0) {
			GetRecord(&RecIdxKey, nChk, &IdxItem); // 해당 key정보 얻기..
		}
		if (nChk < 0) {
			FreeRecord(&RecIdxKey);
			return FALSE; // 없다..
		}
		nChkTemp1 = nChk;
	}
	// IdxItem 정보 안에 docidlist 정보가 있다...

	// 찾아보자..^^
	SetFLock(lpIdxDocId->fpDocIdList[nPos]);
	SetFLock(lpIdxDocId->fpIdx[nPos]);
	fseek(lpIdxDocId->fpDocIdList[nPos], IdxItem.nPos , SEEK_SET);
	AllocRecord(&DocIdList, IdxItem.nWidthDocId, IdxItem.nCountDocId, 2);

	fread(DocIdList.lpBuf, IdxItem.nWidthDocId*IdxItem.nCountDocId, 1, lpIdxDocId->fpDocIdList[nPos]);
	DocIdList.nUsed = IdxItem.nCountDocId;

	// 지워야지..

	nChk = FindRecord(&DocIdList, lpDocId, FALSE, CompDocId);

	if (nChk >= 0) { // 찾았다면 지워야지... 지우고 update 각각 다 해야지..
		DelRecord(&DocIdList, nChk);
		fseek(lpIdxDocId->fpDocIdList[nPos], IdxItem.nPos , SEEK_SET);
		fwrite(DocIdList.lpBuf, DocIdList.nWidth*DocIdList.nUsed, 1, lpIdxDocId->fpDocIdList[nPos]);
		IdxItem.nCountDocId = DocIdList.nUsed;
		if (bAllMatch) {
			SetRecord(&(lpIdxDocId->IdxKey[nPos]), nChkTemp, &IdxItem);
		}
		SetRecord(&RecIdxKey, nChkTemp1, &IdxItem);
		// write
		fseek(lpIdxDocId->fpIdx[nPos], nChkTemp * BLOCK_IDX , SEEK_SET);
		fwrite(RecIdxKey.lpBuf, BLOCK_IDX, 1, lpIdxDocId->fpIdx[nPos]);

	}

	FreeRecord(&RecIdxKey);
	FreeRecord(&DocIdList);	

	UnsetFLock(lpIdxDocId->fpDocIdList[nPos]);
	UnsetFLock(lpIdxDocId->fpIdx[nPos]);

	return TRUE;
}

tINT CheckIndexKey(RECORD *lpIdxKey, tCHAR *szKey, IDX_ITEM_FORMAT *lpIdxItem, tBOOL *bAllMatch)
{
	tINT nLow, nHigh, nMid, nChk;
	IDX_ITEM_FORMAT IdxItem1, IdxItem2;

	nLow = 0;
	nHigh = lpIdxKey->nUsed - 1;
	*bAllMatch = FALSE;

	while (nLow <= nHigh) {
		nMid = (nLow + nHigh) / 2;
		GetRecord(lpIdxKey, nMid, &IdxItem1);	
		nChk = strcmp(IdxItem1.szKey, szKey);

		if (nChk == 0) { // 똑같은 것이 발견되었다.
			memcpy(lpIdxItem, &IdxItem1, sizeof(IDX_ITEM_FORMAT));
			*bAllMatch = TRUE;
			return nMid;
		}
		else if (nChk > 0) { // 더 크니 아래 쪽으로 가야겠지?
			nHigh = nMid - 1;
		}
		else {
			if (nMid == lpIdxKey->nUsed - 1) { //여기에서 찾아야 한다.
				return nMid;
			}
			else {
				GetRecord(lpIdxKey, nMid+1, &IdxItem2);	
				if (strcmp(IdxItem2.szKey, szKey) > 0) {
					// 해당 record에 있다.
					return nMid;
				}
				// 아닐 경우
				nLow = nMid + 1;
			}
		}
	}
	return -1; // not found
}

tBOOL TraverseIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, PutIdxDocIdListFunc PutIdxDocId)
{
	tINT i;

	for ( i = 0 ; i < lpIdxDocId->nCount_File ; i ++ ) {
		TraverseOneIdxDocId(i, lpIdxDocId, PutIdxDocId);
	}
	return TRUE;
}

tBOOL TraverseOneIdxDocId(tINT nPos, IDX_DOCID_FORMAT *lpIdxDocId, PutIdxDocIdListFunc PutIdxDocId)
{
	IDX_ITEM_FORMAT IdxItem;
	RECORD RecDocIdList;
	tINT i;

	SetFLock(lpIdxDocId->fpIdx[nPos]);
	SetFLock(lpIdxDocId->fpDocIdList[nPos]);
	fseek(lpIdxDocId->fpIdx[nPos], 0, SEEK_SET);
	for ( i = 0 ; i < lpIdxDocId->nCountIdxKey[nPos] ; i ++ ) {
		fread(&IdxItem, sizeof(IDX_ITEM_FORMAT), 1, lpIdxDocId->fpIdx[nPos]);

		AllocRecord(&RecDocIdList, IdxItem.nWidthDocId, IdxItem.nCountDocId, 2);
		fseek(lpIdxDocId->fpDocIdList[nPos], IdxItem.nPos, SEEK_SET);
		fread(RecDocIdList.lpBuf, IdxItem.nWidthDocId * IdxItem.nCountDocId, 1, lpIdxDocId->fpDocIdList[nPos]);
		RecDocIdList.nUsed = IdxItem.nCountDocId;

		PutIdxDocId(IdxItem.szKey, &RecDocIdList); 
		FreeRecord(&RecDocIdList);
	}
	UnsetFLock(lpIdxDocId->fpIdx[nPos]);
	UnsetFLock(lpIdxDocId->fpDocIdList[nPos]);

	return TRUE;
}


// 두개의 idx_docidlist를 하나로 merge하는 function
#define TEMP_FILE1 "temp_key1.dat"
#define TEMP_FILE2 "temp_key2.dat"

tBOOL MergeIdxDocId(IDX_DOCID_FORMAT *lpIdx_DocId1, IDX_DOCID_FORMAT *lpIdx_DocId2, IDX_DOCID_FORMAT *lpRetIdx_DocId)
{
	tBOOL bLoad1 = TRUE, bLoad2 = TRUE;
	FILE *fp1, *fp2;
	tCHAR szLine[1024];
	IDX_ITEM_FORMAT item1, item2;
	tINT nPos1, nPos2;
	RECORD DocIdList1, DocIdList2;
	tCHAR *p;
	tINT chk;
	tBOOL bEndFp1 = FALSE, bEndFp2 = FALSE;

	// 두개의 key정보를 다 빼낸 다음 이를 merge소팅하면서 ... docidlist 찾아서 
	// 합쳐서 save시킨다.
	WriteIdxKey(lpIdx_DocId1, TEMP_FILE1);	
	WriteIdxKey(lpIdx_DocId2, TEMP_FILE2);	

	// 이제 merge시키야지..
	fp1 = fopen(TEMP_FILE1, "rt");
	fp2 = fopen(TEMP_FILE2, "rt");

	while (bLoad1 == TRUE || bLoad2 == TRUE) {
		if (bLoad1 == TRUE) {
			if (!fgets(szLine, 1024, fp1)) { // error
				bEndFp1 = TRUE;
				break;
			}
			p = strrchr(szLine, '|');
			*p = '\0';
			p = strrchr(szLine, '|');
			item1.nCountDocId = atoi(p+1);	
			*p = '\0';
			p = strrchr(szLine, '|');
			item1.nWidthDocId = atoi(p+1);	
			*p = '\0';
			p = strrchr(szLine, '|');
			item1.nPos = atoi(p+1);	
			*p = '\0';
			p = strrchr(szLine, '|');
			nPos1 = atoi(p+1);	
			*p = '\0';
			strcpy(item1.szKey, szLine);

			// docidlist 가져와야지..
			AllocRecord(&DocIdList1, item1.nWidthDocId, item1.nCountDocId, 2);
			fseek(lpIdx_DocId1->fpDocIdList[nPos1], item1.nPos, SEEK_SET);
			fread(DocIdList1.lpBuf, item1.nWidthDocId * item1.nCountDocId, 1, lpIdx_DocId1->fpDocIdList[nPos1]);
			DocIdList1.nUsed = item1.nCountDocId;

		}
		if (bLoad2 == TRUE) {
			if (!fgets(szLine, 1024, fp2)) { // error
				bEndFp2 = TRUE;
				break;
			}
			p = strrchr(szLine, '|');
			*p = '\0';
			p = strrchr(szLine, '|');
			item2.nCountDocId = atoi(p+1);	
			*p = '\0';
			p = strrchr(szLine, '|');
			item2.nWidthDocId = atoi(p+1);	
			*p = '\0';
			p = strrchr(szLine, '|');
			item2.nPos = atoi(p+1);	
			*p = '\0';
			p = strrchr(szLine, '|');
			nPos2 = atoi(p+1);	
			*p = '\0';
			strcpy(item2.szKey, szLine);

			// docidlist 가져와야지..
			AllocRecord(&DocIdList2, item2.nWidthDocId, item2.nCountDocId, 2);
			fseek(lpIdx_DocId2->fpDocIdList[nPos2], item2.nPos, SEEK_SET);
			fread(DocIdList2.lpBuf, item2.nWidthDocId * item2.nCountDocId, 1, lpIdx_DocId2->fpDocIdList[nPos2]);
			DocIdList2.nUsed = item2.nCountDocId;
		}

		chk = strcmp(item1.szKey, item2.szKey);

		if (chk == 0) {
			//RecordCpyWithAlloc(&DocIdList1, &DocIdList2);
			AddRecordInRecord(&DocIdList1, DocIdList1.nUsed, &DocIdList2);
			WriteIdxDocId(lpRetIdx_DocId, item1.szKey, &DocIdList1);

			FreeRecord(&DocIdList1);
			FreeRecord(&DocIdList2);

			bLoad1 = TRUE;
			bLoad2 = TRUE;
		}
		else if (chk < 0) {
			WriteIdxDocId(lpRetIdx_DocId, item1.szKey, &DocIdList1);
			FreeRecord(&DocIdList1);
			bLoad1 = TRUE;
		}
		else {
			WriteIdxDocId(lpRetIdx_DocId, item2.szKey, &DocIdList2);
			FreeRecord(&DocIdList2);
			bLoad2 = TRUE;
		}
	}

	if (bLoad1 == FALSE) {
		WriteIdxDocId(lpRetIdx_DocId, item1.szKey, &DocIdList1);
		FreeRecord(&DocIdList1);
	}
	if (bEndFp1 == FALSE) {
	while (fgets(szLine, 1024, fp1)) { 
		p = strrchr(szLine, '|');
		*p = '\0';
		p = strrchr(szLine, '|');
		item1.nCountDocId = atoi(p+1);	
		*p = '\0';
		p = strrchr(szLine, '|');
		item1.nWidthDocId = atoi(p+1);	
		*p = '\0';
		p = strrchr(szLine, '|');
		item1.nPos = atoi(p+1);	
		*p = '\0';
		p = strrchr(szLine, '|');
		nPos1 = atoi(p+1);	
		*p = '\0';
		strcpy(item1.szKey, szLine);

		// docidlist 가져와야지..
		AllocRecord(&DocIdList1, item1.nWidthDocId, item1.nCountDocId, 2);
		fseek(lpIdx_DocId1->fpDocIdList[nPos1], item1.nPos, SEEK_SET);
		fread(DocIdList1.lpBuf, item1.nWidthDocId * item1.nCountDocId, 1, lpIdx_DocId1->fpDocIdList[nPos1]);
		DocIdList1.nUsed = item1.nCountDocId;
		WriteIdxDocId(lpRetIdx_DocId, item1.szKey, &DocIdList1);
		FreeRecord(&DocIdList1);
	}
	}
	if (bLoad2 == FALSE) {
		WriteIdxDocId(lpRetIdx_DocId, item2.szKey, &DocIdList2);
		FreeRecord(&DocIdList2);
	}
	if (bEndFp2 == FALSE) {
	while (fgets(szLine, 1024, fp2)) { 
		p = strrchr(szLine, '|');
		*p = '\0';
		p = strrchr(szLine, '|');
		item2.nCountDocId = atoi(p+1);	
		*p = '\0';
		p = strrchr(szLine, '|');
		item2.nWidthDocId = atoi(p+1);	
		*p = '\0';
		p = strrchr(szLine, '|');
		item2.nPos = atoi(p+1);	
		*p = '\0';
		p = strrchr(szLine, '|');
		nPos2 = atoi(p+1);	
		*p = '\0';
		strcpy(item2.szKey, szLine);

		// docidlist 가져와야지..
		AllocRecord(&DocIdList2, item2.nWidthDocId, item2.nCountDocId, 2);
		fseek(lpIdx_DocId2->fpDocIdList[nPos2], item2.nPos, SEEK_SET);
		fread(DocIdList2.lpBuf, item2.nWidthDocId * item2.nCountDocId, 1, lpIdx_DocId2->fpDocIdList[nPos2]);
		DocIdList2.nUsed = item2.nCountDocId;
		WriteIdxDocId(lpRetIdx_DocId, item2.szKey, &DocIdList2);
		FreeRecord(&DocIdList2);
	}
	}

	fclose(fp1);
	fclose(fp2);

	return TRUE;
}

tBOOL WriteIdxKey(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szKeyFile)
{
	tINT i, j;
	FILE *fp;
	IDX_ITEM_FORMAT IdxItem;

	fp = fopen(szKeyFile, "wt");
	
	for ( i = 0 ; i < lpIdxDocId->nCount_File ; i ++ ) {
		SetFLock(lpIdxDocId->fpIdx[i]);
		fseek(lpIdxDocId->fpIdx[i], 0, SEEK_SET);
		for ( j = 0 ; j < lpIdxDocId->nCountIdxKey[i] ; j ++ ) {
			fread(&IdxItem, sizeof(IDX_ITEM_FORMAT), 1, lpIdxDocId->fpIdx[i]);
			fprintf(fp, "%s|%d|%d|%d|%d|\n", IdxItem.szKey, i, IdxItem.nPos, IdxItem.nWidthDocId, IdxItem.nCountDocId);
		}
		UnsetFLock(lpIdxDocId->fpIdx[i]);
	}
	fclose(fp);
	return TRUE;
}
