#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "dwtype.h"
#include "record.h"
#include "util.h"
#include "linkedlist.h"


// �� ������ �����δ�.
typedef struct {
	//tBYTE *pBuf; // ���̰���?
	tHCHAR nCItem : 10; // ���ȿ� item � ä����?
	tHCHAR nEnd : 6; // 0 : ��, 1 : �������� ������. 
        //2���� ũ�� nLimitFileSize�� �Ѿ���, 2�� ���� �ش� ListFileName �� ��ġ�� �ȴ�. 
	// �̰��� ���߿� ó������. �� ���Ͽ��� �Ѿ�� ��������. ���⵵ ������ �ϱ� ���ؼ� ...
	tINT nNextList; // ������ ���° List ���� ������? ���������� �����Ҳ� 
}LINKEDLIST_TYPE, *LP_LINKEDLIST_TYPE;

tBOOL OpenLinkedListFile(LIST_FILE_TYPE *lpListFile, tCHAR *szListFileName, tINT nCountFp, tINT nSizeItem, tINT nItemInBlock, tINT nLimitFileSize, CompProcP CompRecord, tBOOL bOptRW)
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
	lpListFile->nItemInBlock = nItemInBlock;
	lpListFile->nLimitFileSize = nLimitFileSize;
	lpListFile->bOptRW = bOptRW;


	lpListFile->CompRecord = CompRecord;

	for ( i = 0 ; i < lpListFile->nCountFp ; i++ ) {

#if defined(_USE_DUMMY_FILE)
		lpListFile->nPosEnd[i] = sizeof(tLONG); //file end pos�ʱ�ȭ 
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
			else { // read�ε� ������ ���� ������
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
				// ó���̴� ..
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

tVOID CloseLinkedListFile(LIST_FILE_TYPE *lpListFile)
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

// *nRetStPos < 0 , *nRetLastPos < 0 �̸� �� ������ ������� ����� ������ �Ǵ�����..
tBOOL PutItemToLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *pItem, tINT *nRetFpNum, tINT *nRetStPos, tINT *nRetLastPos, tINT *nRetJump)
{
	RECORD RecItem;
	LINKEDLIST_TYPE LinkedList;
	tINT nPos, nPos1;
	tINT nFpNum = nSrcFpNum;
	tINT nJump = 0;

	*nRetFpNum = nFpNum;
	*nRetStPos = -1;
	*nRetLastPos = -1;
	if (lpListFile->bOptRW == OPEN_READ_LIST) return FALSE;
	if (lpListFile->bOpenList == FALSE) return FALSE;

	AllocRecord(&RecItem, lpListFile->nSizeItem, lpListFile->nItemInBlock, lpListFile->nItemInBlock);

	if (nStPos < 0) { // ó�� �ִ� ���̴�...
		AppendRecord(&RecItem, pItem);
		SetFLock(lpListFile->fp[nFpNum]);

#if defined(_USE_DUMMY_FILE)
		fseek(lpListFile->fp[nFpNum], lpListFile->nPosEnd[nFpNum], SEEK_SET);
		*nRetStPos = lpListFile->nPosEnd[nFpNum];
#else
		fseek(lpListFile->fp[nFpNum], 0L, SEEK_END);
		*nRetStPos = ftell(lpListFile->fp[nFpNum]);
#endif
		if (*nRetStPos + (lpListFile->nSizeItem * lpListFile->nItemInBlock) + (int)sizeof(LINKEDLIST_TYPE) > lpListFile->nLimitFileSize - 1) { // �� ������ limit �� �Ѿ� ����... 
			// ���� �߻�... ������ ���� �����Ű�� �ʵ��� ��.
			FreeRecord(&RecItem);
			UnsetFLock(lpListFile->fp[nFpNum]);
			return FALSE;
		}
		memset(&LinkedList, 0, sizeof(LINKEDLIST_TYPE));
		LinkedList.nCItem = 1;

		// block���� �� next ����..
		fwrite(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

#if defined(_USE_DUMMY_FILE)
		// 1���� write�ϸ� �� �� ��������?
		fwrite(RecItem.lpBuf, lpListFile->nSizeItem, 1, lpListFile->fp[nFpNum]);
#else
		// block ���� �׻� �־�� �Ѵ�.
		fwrite(RecItem.lpBuf, (lpListFile->nSizeItem * lpListFile->nItemInBlock), 1, lpListFile->fp[nFpNum]);
#endif
		*nRetLastPos = *nRetStPos;

		*nRetFpNum = nFpNum;
#if defined(_USE_DUMMY_FILE)
		lpListFile->nPosEnd[nFpNum] += (sizeof(LINKEDLIST_TYPE) + (lpListFile->nSizeItem * lpListFile->nItemInBlock));
		//������ ���ؼ� update���Ѻ���.
		// ���߿� �ӵ��� ���ؼ� ���� ���� �ִ�.
		fseek(lpListFile->fp[nFpNum], 0L, SEEK_SET);
		fwrite(&(lpListFile->nPosEnd[nFpNum]), sizeof(tLONG), 1, lpListFile->fp[nFpNum]);
#endif


		UnsetFLock(lpListFile->fp[nFpNum]);
	}
	else {
		SetFLock(lpListFile->fp[nFpNum]);

		// ��ġ �ű� �� ..
		fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);
		*nRetStPos = nStPos;

		while (1) {
			nPos = ftell(lpListFile->fp[nFpNum]);

                	// block���� �� next ����..
                	fread(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

			//fread(RecItem.lpBuf, (lpListFile->nSizeItem * lpListFile->nItemInBlock), 1, lpListFile->fp[nFpNum]);

			if (LinkedList.nCItem < lpListFile->nItemInBlock) { // �۴ٸ� .. �߰��� �־ �ǰڳ�.. 
				
				// item�� write�� ��ġ�� ã�ư���.
				// �ش� item�� ���� .. �̰� �ӵ��� ��������?
				fseek(lpListFile->fp[nFpNum], LinkedList.nCItem*lpListFile->nSizeItem, SEEK_CUR);
				fwrite(pItem, lpListFile->nSizeItem, 1, lpListFile->fp[nFpNum]);

				// linkedlist info write
                		// block���� �� next ����..
				LinkedList.nCItem += 1;
				fseek(lpListFile->fp[nFpNum], nPos, SEEK_SET);
                		fwrite(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

				*nRetFpNum = nFpNum;
				if (LinkedList.nCItem < lpListFile->nItemInBlock) *nRetLastPos = nPos;
				break;
			}
			else if (LinkedList.nEnd == 0) { // ���ε� full�̴� �� ���� �ٿ�����..

#if defined(_USE_DUMMY_FILE)
				fseek(lpListFile->fp[nFpNum], lpListFile->nPosEnd[nFpNum], SEEK_SET);
				nPos1 = lpListFile->nPosEnd[nFpNum];
#else
				fseek(lpListFile->fp[nFpNum], 0L, SEEK_END);
				nPos1 = ftell(lpListFile->fp[nFpNum]);
#endif

				if (nPos1 + (lpListFile->nSizeItem * lpListFile->nItemInBlock) + (int)sizeof(LINKEDLIST_TYPE) > lpListFile->nLimitFileSize - 1) { // �� ������ limit �� �Ѿ� ����... 
					printf("limit overflow \n");
					break;
				}
				else {
					LinkedList.nEnd = 1;

					// ���� ������ .. next �����ؾ���... ---------------------
					fseek(lpListFile->fp[nFpNum], nPos, SEEK_SET);
					LinkedList.nNextList = nPos1;
       	         			// block���� �� next ����..
                			fwrite(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);
					//---------------------------------
				}
				
				// �ٽ� �� ���� �� write �ؾ��� .. 
				RecItem.nUsed = 0;
				AppendRecord(&RecItem, pItem);

#if defined(_USE_DUMMY_FILE)
				fseek(lpListFile->fp[nFpNum], lpListFile->nPosEnd[nFpNum], SEEK_SET);
#else
				fseek(lpListFile->fp[nFpNum], 0L, SEEK_END);
#endif
				// jump �ѹ����ؾ���..
				nJump ++;

				memset(&LinkedList, 0, sizeof(LINKEDLIST_TYPE));
				LinkedList.nCItem = 1;

				// block���� �� next ����..
				fwrite(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

#if defined(_USE_DUMMY_FILE)
				// �ش� �����͸� ������ ���״� ..
				fwrite(RecItem.lpBuf, lpListFile->nSizeItem, 1, lpListFile->fp[nFpNum]);
#else
				// block ���� �׻� �־�� �Ѵ�.
				fwrite(RecItem.lpBuf, (lpListFile->nSizeItem * lpListFile->nItemInBlock), 1, lpListFile->fp[nFpNum]);
#endif
				*nRetFpNum = nFpNum;
				*nRetLastPos = nPos1;
#if defined(_USE_DUMMY_FILE)
				lpListFile->nPosEnd[nFpNum] += (sizeof(LINKEDLIST_TYPE) + (lpListFile->nSizeItem * lpListFile->nItemInBlock));
				//������ ���ؼ� update���Ѻ���.
				// ���߿� �ӵ��� ���ؼ� ���� ���� �ִ�.
				fseek(lpListFile->fp[nFpNum], 0L, SEEK_SET);
				fwrite(&(lpListFile->nPosEnd[nFpNum]), sizeof(tLONG), 1, lpListFile->fp[nFpNum]);
#endif
				break;
			}
			else if (LinkedList.nEnd == 1) { // next ã�ư����� .. 
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

	FreeRecord(&RecItem);

	*nRetJump = nJump;
	return TRUE;

}

tBOOL GetItemListFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpRecItem)
{
	RECORD RecItem;
	LINKEDLIST_TYPE LinkedList;
	tINT nFpNum = nSrcFpNum;
#if defined(_OLD)
	tBYTE *pBuf;
	tINT i;
#endif
	tBOOL bRetVal = TRUE;

	if (lpListFile->bOpenList == FALSE) return FALSE;
	if (nStPos < 0) return FALSE;

	AllocRecord(&RecItem, lpListFile->nSizeItem, lpListFile->nItemInBlock, lpListFile->nItemInBlock);
#if defined(_OLD)
	pBuf = (tBYTE *)malloc(lpListFile->nSizeItem);
	if (pBuf == NULL) {
		FreeRecord(&RecItem);
		printf("malloc error\n");
		return FALSE;
	}
#endif

	SetFLock(lpListFile->fp[nFpNum]);

	// ��ġ �ű� �� ..
	fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);

	while (1) {
               	// block���� �� next ����..
               	fread(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

		fread(RecItem.lpBuf, (lpListFile->nSizeItem * lpListFile->nItemInBlock), 1, lpListFile->fp[nFpNum]);
		RecItem.nUsed = LinkedList.nCItem;
		
#if defined(_OLD)
		//��� record�� ��ƾ��� ..
		for ( i = 0 ; i < RecItem.nUsed ; i ++ ) {
			GetRecord(&RecItem, i, pBuf);
			AppendRecord(lpRecItem, pBuf);
		}
#else
		AddRecordInRecord(lpRecItem, lpRecItem->nUsed, &RecItem);
#endif

		if (LinkedList.nEnd == 0) { // �� ..
			break;
		}
		else if (LinkedList.nEnd == 1) { // ���� ������ ���� ..
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
	FreeRecord(&RecItem);

	return bRetVal;	
}

tBOOL DelItemListFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem)
{
	RECORD RecItem;
	LINKEDLIST_TYPE LinkedList;
	tINT nFpNum = nSrcFpNum;
	tINT nPos;
	tBOOL bDelOk = FALSE;
	tINT nChk;
	tBOOL bRetVal = FALSE;

	if (lpListFile->bOptRW == OPEN_READ_LIST) return FALSE;
	if (lpListFile->bOpenList == FALSE) return FALSE;
	if (nStPos < 0) return FALSE;

	AllocRecord(&RecItem, lpListFile->nSizeItem, lpListFile->nItemInBlock, lpListFile->nItemInBlock);
	SetFLock(lpListFile->fp[nFpNum]);

	// ��ġ �ű� �� ..
	fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);

	while (1) {
		nPos = ftell(lpListFile->fp[nFpNum]);

               	// block���� �� next ����..
               	fread(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);
		fread(RecItem.lpBuf, (lpListFile->nSizeItem * lpListFile->nItemInBlock), 1, lpListFile->fp[nFpNum]);
		RecItem.nUsed = LinkedList.nCItem;
		bDelOk = FALSE;	
		//���ؼ� ������ �������� ..
		nChk = FindRecord(&RecItem, lpItem, FALSE, lpListFile->CompRecord);
		if (nChk != -1) { // �ִ�.
			DelRecord(&RecItem, nChk);
			bDelOk = TRUE;
			bRetVal = TRUE;
		}
		if (bDelOk) { // ������..
			fseek(lpListFile->fp[nFpNum], nPos, SEEK_SET);

			LinkedList.nCItem = RecItem.nUsed; // count ���� �ٽ� �ֱ�
               		fwrite(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

			fwrite(RecItem.lpBuf, (lpListFile->nSizeItem * lpListFile->nItemInBlock), 1, lpListFile->fp[nFpNum]);
			//fflush(lpListFile->fp[nFpNum]);
			//fdatasync(fileno(lpListFile->fp[nFpNum]));
			break;
		}

		if (LinkedList.nEnd == 0) { // �� ..
			break;
		}
		else if (LinkedList.nEnd == 1) { // ���� ������ ���� ..
			fseek(lpListFile->fp[nFpNum], LinkedList.nNextList, SEEK_SET);
		}
		else { // exception
			bRetVal = FALSE;
			printf("Del Linkedlist exception\n");
			break;
		}
	}

	UnsetFLock(lpListFile->fp[nFpNum]);
	FreeRecord(&RecItem);

#if defined(_DEBUG)
	if (bDelOk) {
		printf("delete ok : %d, %d", nFpNum, nPos);
	}
#endif
	return bRetVal;	

}

tBOOL UpdateItemListFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem, tVOID *lpUpdateItem)
{
	RECORD RecItem;
	LINKEDLIST_TYPE LinkedList;
	tINT nFpNum = nSrcFpNum;
	tINT nPos;
	tBOOL bUpdateOk = FALSE;
	tINT nChk;
	tBOOL bRetVal = FALSE;

	if (lpListFile->bOptRW == OPEN_READ_LIST) return FALSE;
	if (lpListFile->bOpenList == FALSE) return FALSE;
	if (nStPos < 0) return FALSE;

	AllocRecord(&RecItem, lpListFile->nSizeItem, lpListFile->nItemInBlock, lpListFile->nItemInBlock);
	SetFLock(lpListFile->fp[nFpNum]);

	// ��ġ �ű� �� ..
	fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);

	while (1) {
		nPos = ftell(lpListFile->fp[nFpNum]);

               	// block���� �� next ����..
               	fread(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

		fread(RecItem.lpBuf, (lpListFile->nSizeItem * lpListFile->nItemInBlock), 1, lpListFile->fp[nFpNum]);
		RecItem.nUsed = LinkedList.nCItem;
		
		//���ؼ� ������ Update�ϱ� 
		nChk = FindRecord(&RecItem, lpItem, FALSE, lpListFile->CompRecord);
		if (nChk != -1) { // �ִ�.
			SetRecord(&RecItem, nChk, lpUpdateItem);
			bUpdateOk = TRUE;
			bRetVal = TRUE;
		}
		if (bUpdateOk) { // ������..
			//�ش� ��ϸ� ���� �ȴ�. linkedlist info�� ������ص� �ȴ�.
			fseek(lpListFile->fp[nFpNum], nPos+sizeof(LINKEDLIST_TYPE), SEEK_SET);

			fwrite(RecItem.lpBuf, (lpListFile->nSizeItem * lpListFile->nItemInBlock), 1, lpListFile->fp[nFpNum]);
			//fflush(lpListFile->fp[nFpNum]);
			//fdatasync(fileno(lpListFile->fp[nFpNum]));
			break;
		}

		if (LinkedList.nEnd == 0) { // �� ..
			break;
		}
		else if (LinkedList.nEnd == 1) { // ���� ������ ���� ..
			fseek(lpListFile->fp[nFpNum], LinkedList.nNextList, SEEK_SET);
		}
		else { // exception
			bRetVal = FALSE;
			printf("Update Linkedlist exception\n");
			break;
		}
	}

	UnsetFLock(lpListFile->fp[nFpNum]);
	FreeRecord(&RecItem);

#if defined(_DEBUG)
	if (bUpdateOk) {
		printf("update ok : %d, %d", nFpNum, nPos);
	}
#endif
	return bRetVal;	

}

tBOOL DelItemListArrayFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpItemRec)
{
	RECORD RecItem;
	RECORD DelRecItem;
	LINKEDLIST_TYPE LinkedList;
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
	AllocRecord(&RecItem, lpListFile->nSizeItem, lpListFile->nItemInBlock, lpListFile->nItemInBlock);
	lpItem = (tBYTE *)malloc(lpListFile->nSizeItem);
	if (lpItem == NULL) {
		FreeRecord(&DelRecItem);
		FreeRecord(&RecItem);
		return FALSE;
	}

	SetFLock(lpListFile->fp[nFpNum]);

	// ��ġ �ű� �� ..
	fseek(lpListFile->fp[nFpNum], nStPos, SEEK_SET);

	while (1) {
		nPos = ftell(lpListFile->fp[nFpNum]);

               	// block���� �� next ����..
               	fread(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);
		fread(RecItem.lpBuf, (lpListFile->nSizeItem * lpListFile->nItemInBlock), 1, lpListFile->fp[nFpNum]);
		RecItem.nUsed = LinkedList.nCItem;
		
		bDelOk = FALSE;
		if (RecItem.nUsed >= DelRecItem.nUsed) {
			for ( i = 0 ; i < DelRecItem.nUsed ; i ++ ) {
				GetRecord(&DelRecItem, i, lpItem);
				//���ؼ� ������ �������� ..
				nChk = FindRecord(&RecItem, lpItem, FALSE, lpListFile->CompRecord);
				if (nChk != -1) { // �ִ�.
					DelRecord(&RecItem, nChk);

					// �����.
					DelRecord(&DelRecItem, i);
					i --;
					bDelOk = TRUE;
					bRetVal = TRUE;
				}
				if (RecItem.nUsed == 0) break;
			}
		}
		else { // �ݴ�� �ؾ߰���..?
			for ( i = 0 ; i < RecItem.nUsed ; i ++ ) {
				GetRecord(&RecItem, i, lpItem);
				//���ؼ� ������ �������� ..
				nChk = FindRecord(&DelRecItem, lpItem, FALSE, lpListFile->CompRecord);
				if (nChk != -1) { // �ִ�.
					DelRecord(&DelRecItem, nChk);

					// �����.
					DelRecord(&RecItem, i);
					i --;
					bDelOk = TRUE;
					bRetVal = TRUE;
				}
				if (DelRecItem.nUsed == 0) break;
			}

		}
		if (bDelOk) { // ������..
			fseek(lpListFile->fp[nFpNum], nPos, SEEK_SET);

			LinkedList.nCItem = RecItem.nUsed; // count ���� �ٽ� �ֱ�
               		fwrite(&LinkedList, sizeof(LINKEDLIST_TYPE), 1, lpListFile->fp[nFpNum]);

			fwrite(RecItem.lpBuf, (lpListFile->nSizeItem * RecItem.nUsed), 1, lpListFile->fp[nFpNum]);
		}
		if (DelRecItem.nUsed == 0) {
			// ���̻� ���� ���� ����.
			break;
		}

		if (LinkedList.nEnd == 0) { // �� ..
			break;
		}
		else if (LinkedList.nEnd == 1) { // ���� ������ ���� ..
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
		printf("delete ok : delitem : %d -> %d remain\n", lpItemRec->nUsed, DelRecItem.nUsed);
	}
#endif
	FreeRecord(&RecItem);
	FreeRecord(&DelRecItem);
	free(lpItem);

	return bRetVal;	

}
