/*
	Trie �����Դϴ�. 
	insert, modify���� �Ǵ� �����Դϴ�. 
	
	Trie�� ���� ��ó���� RECORD�� �Ͽ����ϴ�.


	programmed by hyypucs

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include "dwtype.h"
#include "hstrlib.h"
#include "record.h"
#include "ntrie.h"

PRIVATE tINT CompNTrieIndex(NTRIE_INDEXTYPE *item1, NTRIE_INDEXTYPE *item2);
PRIVATE tINT FindMaxNTrie(NTRIE_TYPE *nTrie, tHCHAR *uszWord, tINT *nMaxFindItem, tINT *nPosItem);
tBOOL MakeNTrieItem(tHCHAR *uszWord, tDWORD dwInfo, RECORD *lpRecord);
tINT GetPosNearRecord(RECORD *lpRecord, tVOID *lpRecordItem, CompProcP CompRecord);
tBOOL GetnTrieNextTail(RECORD *lpRecord, tINT nPrePosItem, tINT *nNextPosItem);
tBOOL GetPrenTrieItem(RECORD *lpRecord, tINT nPosItem, NTRIE_ITEMTYPE *nTrieItem1, tINT *nPrePosItem);
tBOOL InsertRecord(RECORD *lpRecord, tINT nPosRecord, RECORD *lpWordRecord);
tBOOL ModifyIndex(RECORD *lpIndex, tINT nPlusNum, NTRIE_INDEXTYPE *lpIndexItem);
tBOOL SetNTrieNextPrefix(NTRIE_TYPE *nTrie, tHCHAR *uszWord, tINT nPrefix, tINT nLen2);

tBOOL OpenNTrie(tCHAR *szFile, NTRIE_TYPE *nTrie, tINT nInitIndex, tINT nIncIndex, tINT nInitBody, tINT nIncBody, tBOOL bAlloc)
{
	tCHAR szFileName[MAX_PATH];

	memset(nTrie, 0, sizeof(NTRIE_TYPE));

	sprintf(szFileName, "%s%s", szFile, EXT_NTRIE_INDEX);
	//index read...

	if (File2Record(&(nTrie->Index), szFileName, sizeof(NTRIE_INDEXTYPE), nInitIndex, nIncIndex, bAlloc) == FALSE) return FALSE;

	// body read..
	sprintf(szFileName, "%s%s", szFile, EXT_NTRIE_BODY);
	if (File2Record(&(nTrie->Body), szFileName, sizeof(NTRIE_ITEMTYPE), nInitBody, nIncBody, bAlloc) == FALSE) {

		FreeRecord(&(nTrie->Index));
		return FALSE;
	}

	return TRUE;
}

tBOOL WriteNTrie(tCHAR *szFile, NTRIE_TYPE *nTrie, tBOOL bFree)
{
	tCHAR szFileName[MAX_PATH];

	sprintf(szFileName, "%s%s", szFile, EXT_NTRIE_INDEX);
	if (Record2File(&(nTrie->Index), szFileName, bFree) == FALSE) {
		return FALSE;
	}

	sprintf(szFileName, "%s%s", szFile, EXT_NTRIE_BODY);
	if (Record2File(&(nTrie->Body), szFileName, bFree) == FALSE) {
		return FALSE;
	}

	return TRUE;
}

tVOID FreeNTrie(NTRIE_TYPE *nTrie)
{
	FreeRecord(&(nTrie->Index));
	FreeRecord(&(nTrie->Body));
}

tBOOL AllocNTrie(NTRIE_TYPE *nTrie, tINT nInitIndex, tINT nIncIndex, tINT nInitBody, tINT nIncBody)
{
	if (AllocRecord(&(nTrie->Index), sizeof(NTRIE_INDEXTYPE), nInitIndex, nIncIndex) == FALSE) return FALSE;

	if (AllocRecord(&(nTrie->Body), sizeof(NTRIE_ITEMTYPE), nInitBody, nIncBody) == FALSE) {
		FreeRecord(&(nTrie->Index));
		return FALSE;
	}
	return TRUE;
}


tBOOL MakeNTrieItem(tHCHAR *uszWord, tDWORD dwInfo, RECORD *lpRecord)
{
	tINT nLen = HStrlen(uszWord), i;
	NTRIE_ITEMTYPE nTrieItem;

	if (AllocRecord(lpRecord, sizeof(NTRIE_ITEMTYPE), nLen, 1) == FALSE) {
		return FALSE;
	}

	for ( i = 0 ; i < nLen ; i ++ ) {
		nTrieItem.uszCh = uszWord[i];
		nTrieItem.info.i.nNext = FILL_NEXT;
		if (i == nLen - 1) {
			nTrieItem.info.i.nCont = 0;
			nTrieItem.info.i.nEnd = 1;
		}
		else {
			nTrieItem.info.i.nCont = 1;
			nTrieItem.info.i.nEnd = 0;
		}
		nTrieItem.dwInfo = dwInfo; // ������ ���� ��� ���� �ȴ�. 
		AppendRecord(lpRecord, &nTrieItem);
	}
	return TRUE;
}

tBOOL AddNTrie(NTRIE_TYPE *nTrie, tHCHAR *uszWord, tDWORD dwInfo)
{
	tINT nLen, nLen1, nLen2, i;
	tINT nPrefix, nPosItem, nPos, nPos1, nPrePosItem, nNextPosItem;
	NTRIE_INDEXTYPE IndexItem;
	RECORD WordRecord;
	NTRIE_ITEMTYPE nTrieItem, nTrieItem1, nTrieItem2;

	nLen1 = HStrlen(uszWord);
	if (nLen1 >= MAX_TRIE_WORD) { // �ʹ� ���̰� ���.
		return FALSE;
	}
	nLen = FindMaxNTrie(nTrie, uszWord, &nPrefix, &nPosItem);
	if (nPosItem == -1) { // �ε����� ����. 
		MakeNTrieItem(uszWord, dwInfo, &WordRecord);

		IndexItem.uszCh = uszWord[0];		
		nPos = GetPosNearRecord(&(nTrie->Index), &IndexItem, (CompProcP) CompNTrieIndex);
		if (nPos == -2) { // �ε����� �ִ�.. �̻��� ��� exception
			return FALSE;
		}

		nPos1 = -1;
		for ( i = nPos ; i < (nTrie->Index).nUsed ; i ++ ) { // nUsed �� len��ŭ +�ؾ� �Ѵ�. 
			GetRecord(&(nTrie->Index), i, &IndexItem);
			if (i == nPos) {
				nPos1 = IndexItem.nPos;
			}
			IndexItem.nPos += nLen1; // ��ŭ ���۰� ����... 
			SetRecord(&(nTrie->Index), i, &IndexItem);
		}
		if (nPos1 == -1) {
			nPos1 = (nTrie->Body).nUsed;
		}
		IndexItem.uszCh = uszWord[0];
		IndexItem.nPos = nPos1;

#if defined(_NTRIE_COUNT)
		IndexItem.nCount = 1;
#endif

		AddRecord(&(nTrie->Index), nPos, &IndexItem); //�ش�Ǵ� �����ǿ� �ְ�..


		AddRecordInRecord(&(nTrie->Body), nPos1, &WordRecord);
		//InsertRecord(&(nTrie->Body), nPos1, &WordRecord); // �ش� ���ڵ带 �ش� �����ǿ� �ִ´�. 
	}
	else if (nLen == nLen1) { // ���� �ܾ �����Ѵ�. 
		return FALSE;
	}
	else if (nLen1 == nPrefix) { // �ܾ�� ���µ� �� �� �ܾ �ִ�. 
				// �ܾ����� �ִµ� �ܾ �߰��� �� 
		
#if defined(_NTRIE_COUNT)
		IndexItem.uszCh = uszWord[0];
		nPos = FindRecord(&(nTrie->Index), &IndexItem, TRUE, (CompProcP)CompNTrieIndex);
		if (nPos == -1) { // exception
			return FALSE;
		}
		IndexItem.nCount += 1; // count ���ϱ� �ϰ�..
		SetRecord(&(nTrie->Index), nPos, &IndexItem); //�ش�Ǵ� �����ǿ� �ְ�..
#endif

		GetRecord(&(nTrie->Body), nPosItem, &nTrieItem);
		nTrieItem.info.i.nEnd = 1;
		nTrieItem.dwInfo = dwInfo;
		SetRecord(&(nTrie->Body), nPosItem, &nTrieItem); // ���õ�..

	}
	else if (nLen1 < nPrefix) { // exception
		return FALSE;
	}
	else {
		MakeNTrieItem(uszWord+nPrefix, dwInfo, &WordRecord);
		nLen2 = nLen1 - nPrefix; // �Է��� ���� 
		GetRecord(&(nTrie->Body), nPosItem, &nTrieItem);
		if (nTrieItem.info.i.nCont) { // �ִٴ� �ֱ��̴� 
			GetRecord(&WordRecord, 0, &nTrieItem1);
			GetPrenTrieItem(&(nTrie->Body), nPosItem, &nTrieItem1, &nPrePosItem);
			if (nPrePosItem == -1) { // �ٷ� ������ �Է��ؾ� �Ѵ�. 
				nTrieItem1.info.i.nNext = nLen2;
				SetRecord(&WordRecord, 0, &nTrieItem1);
				nPrePosItem = nPosItem + 1;
			}
			else {
				GetRecord(&(nTrie->Body), nPrePosItem, &nTrieItem2);
				GetnTrieNextTail(&(nTrie->Body), nPrePosItem, &nNextPosItem);
				if (nTrieItem2.info.i.nNext == FILL_NEXT) {
					if (nNextPosItem-nPrePosItem+1 >= FILL_NEXT) { // exception
						printf("Exception1: FILL_NEXT overflow!!\n");
						return FALSE;
					}
					nTrieItem2.info.i.nNext = nNextPosItem - nPrePosItem + 1;
					SetRecord(&(nTrie->Body), nPrePosItem, &nTrieItem2);
				}
				else {
					nTrieItem1.info.i.nNext = nLen2;
					SetRecord(&WordRecord, 0, &nTrieItem1);
				}
				nPrePosItem += nTrieItem2.info.i.nNext;
			}
		}
		else {
			nTrieItem.info.i.nCont = 1; // ������ �Է����״�..
			nPrePosItem = nPosItem + 1;
		}

/*
		if (nTrieItem.info.i.nNext != FILL_NEXT) {
			nTrieItem.info.i.nNext += nLen2;
		}
*/

		SetRecord(&(nTrie->Body), nPosItem, &nTrieItem);

		// ������ �ֵ� next���� nLen2�� ���ؾ� �Ѵ�. 
		SetNTrieNextPrefix(nTrie, uszWord, nPrefix, nLen2);

		AddRecordInRecord(&(nTrie->Body), nPrePosItem, &WordRecord);
		//InsertRecord(&(nTrie->Body), nPrePosItem, &WordRecord); // �ش� ���ڵ带 �ش� �����ǿ� �ִ´�. 
		
#if defined(_NTRIE_COUNT)
		IndexItem.uszCh = uszWord[0];
		nPos = FindRecord(&(nTrie->Index), &IndexItem, TRUE, (CompProcP)CompNTrieIndex);
		if (nPos == -1) { // exception
			return FALSE;
		}
		IndexItem.nCount += 1; // count ���ϱ� �ϰ�..
		SetRecord(&(nTrie->Index), nPos, &IndexItem); //�ش�Ǵ� �����ǿ� �ְ�..
#endif

		IndexItem.uszCh = uszWord[0];
		ModifyIndex(&(nTrie->Index), nLen2, &IndexItem);
	}
	return TRUE;
}

// infomation �ٲٴ� ���̴�. 
tBOOL ModNTrie(NTRIE_TYPE *nTrie, tHCHAR *uszWord, tDWORD dwInfo)
{
	tINT nLen;
	tINT nPrefix, nPosItem;
	NTRIE_ITEMTYPE nTrieItem;

	nLen = FindMaxNTrie(nTrie, uszWord, &nPrefix, &nPosItem);

	if (nLen == HStrlen(uszWord)) { // ������ ��.. 

		GetRecord(&(nTrie->Body), nPosItem, &nTrieItem);
		nTrieItem.dwInfo = dwInfo;
		SetRecord(&(nTrie->Body), nPosItem, &nTrieItem);
		return TRUE;
	}
	return FALSE; // �������� ������.. 

}

tBOOL ModifyIndex(RECORD *lpIndex, tINT nPlusNum, NTRIE_INDEXTYPE *lpIndexItem)
{
	tINT nPos, i;
	NTRIE_INDEXTYPE tmpIndex;

	nPos = FindRecord(lpIndex, lpIndexItem, TRUE, (CompProcP)CompNTrieIndex);
	if (nPos == -1) return FALSE;

	for ( i = nPos + 1 ; i < lpIndex->nUsed ; i ++ ) {
		GetRecord(lpIndex, i, &tmpIndex);
		tmpIndex.nPos += nPlusNum;
		SetRecord(lpIndex, i, &tmpIndex);
	}
	return TRUE;
}

tBOOL GetPrenTrieItem(RECORD *lpRecord, tINT nPosItem, NTRIE_ITEMTYPE *nTrieItem1, tINT *nPrePosItem)
{
	NTRIE_ITEMTYPE nTrieItem;
	tINT nSt = nPosItem + 1;

	*nPrePosItem = -1;

	while ( 1 ) {
		GetRecord(lpRecord, nSt, &nTrieItem);
		if (nTrieItem.uszCh > nTrieItem1->uszCh) { // �̰� ó������ �ȵǴ�..
			break;
		}
		else if (nTrieItem.uszCh == nTrieItem1->uszCh) {
			break;
		}
		else { // ������... 
			*nPrePosItem = nSt;
		}
		if (nTrieItem.info.i.nNext == FILL_NEXT) {
			break; 
		}
		nSt += nTrieItem.info.i.nNext;
	}

	return TRUE;
}

tBOOL GetnTrieNextTail(RECORD *lpRecord, tINT nPrePosItem, tINT *nNextPosItem)
{
	NTRIE_ITEMTYPE nTrieItem;
	tINT nSt = nPrePosItem;

	*nNextPosItem = -1;

	while ( 1 ) {
		GetRecord(lpRecord, nSt, &nTrieItem);
		if (nTrieItem.info.i.nNext == FILL_NEXT) {
			if (nTrieItem.info.i.nCont) {
				nSt ++;
				GetRecord(lpRecord, nSt, &nTrieItem);
				*nNextPosItem = nSt;
			}
			else {
				*nNextPosItem = nSt;
				break;
			}
		}
		else nSt += nTrieItem.info.i.nNext;
	}

	return TRUE;
}

tBOOL InsertRecord(RECORD *lpRecord, tINT nPosRecord, RECORD *lpWordRecord)
{
	NTRIE_ITEMTYPE nTrieItem;
	tINT i;

	if (lpWordRecord->nUsed == 0) return TRUE;
	for ( i = 0 ; i < lpWordRecord->nUsed ; i ++, nPosRecord ++ ) {
		GetRecord(lpWordRecord, i, &nTrieItem);
		AddRecord(lpRecord, nPosRecord, &nTrieItem);
	}
	return TRUE;
}


tINT GetPosNearRecord(RECORD *lpRecord, tVOID *lpRecordItem, CompProcP CompRecord)
{
        tINT nChk, nPos;
        tINT nLow, nMid, nHigh;
        tBYTE *p = lpRecord->lpBuf;

        nLow = 0;
        nHigh = lpRecord->nUsed - 1;

        while( nLow <= nHigh ) {
                nMid = (nLow + nHigh) / 2;
                nChk = CompRecord(lpRecordItem, p + nMid * lpRecord->nWidth);
                if (nChk == 0) {
                        return -2; // �ִ�.
                }
                else if (nChk > 0) {
                        nLow = nMid + 1;
                }
                else {
                        nHigh = nMid - 1;
                }
        }
        nPos = nLow;
	return (nPos);
}

tBOOL SetNTrieNextPrefix(NTRIE_TYPE *nTrie, tHCHAR *uszWord, tINT nPrefix, tINT nLen2)
{
	tINT nPos, nStPos;
	NTRIE_INDEXTYPE chkIndex;
	NTRIE_ITEMTYPE item;
	tHCHAR *p;
	tHCHAR tmpWord[MAX_TRIE_WORD];
	tINT chk;

	HStrcpy(tmpWord, uszWord);
	tmpWord[nPrefix] = 0x00;
	p = tmpWord;

	if (tmpWord[0] == 0x00) return FALSE;

	chkIndex.uszCh = tmpWord[0];
	nPos = FindRecord(&(nTrie->Index), &chkIndex, TRUE, (CompProcP)CompNTrieIndex);
	if (nPos == -1) return 0;

	GetRecord(&(nTrie->Index), nPos, &chkIndex);
	nStPos =  chkIndex.nPos;

	while (*p) {
		GetRecord(&(nTrie->Body), nStPos, &item);
		chk = *p - item.uszCh;
		if (chk == 0) {
			if (item.info.i.nNext != FILL_NEXT) {
				if (item.info.i.nNext + nLen2 >= FILL_NEXT) {
					printf("Exception2: FILL_NEXT overflow.\n");
					return FALSE;
				}
				item.info.i.nNext += nLen2;	
				SetRecord(&(nTrie->Body), nStPos, &item);
			}	
			if (item.info.i.nCont) { // ��� �پ��ִ� ���� �ִ�..
				nStPos ++;
			}
			else { // ����ؼ� �ܾ ����. 
				return TRUE;
			}
			p++;
		}
		else if (chk>0) { // �۴�. 
			if (item.info.i.nNext == FILL_NEXT) {
				return TRUE;
			}
			nStPos += item.info.i.nNext; // �ٽ� �����ͼ�...
		}
		else {
			return TRUE; // �ٷ� ũ��...
		}
	}	
	return TRUE;

}

tINT FindNTrie(NTRIE_TYPE *nTrie, tHCHAR *uszWord, tINT *nMaxFindItem, tDWORD *dwInfo)
{
	tINT nPos, nStPos;
	NTRIE_INDEXTYPE chkIndex;
	NTRIE_ITEMTYPE item;
	tHCHAR *p = uszWord;
	tINT nPrefix = 0;
	tINT chk;

	*nMaxFindItem = 0;

	if (uszWord[0] == 0x00) return 0;


	chkIndex.uszCh = uszWord[0];
	nPos = FindRecord(&(nTrie->Index), &chkIndex, TRUE, (CompProcP)CompNTrieIndex);
	if (nPos == -1) return 0;

	GetRecord(&(nTrie->Index), nPos, &chkIndex);
	nStPos =  chkIndex.nPos;

	while (*p) {
		GetRecord(&(nTrie->Body), nStPos, &item);
		chk = *p - item.uszCh;
		if (chk == 0) {
			*nMaxFindItem += 1;
			if (item.info.i.nEnd) { // ���̴� 
				*dwInfo = item.dwInfo;
				nPrefix = *nMaxFindItem;	
			}
			if (item.info.i.nCont) { // ��� �پ��ִ� ���� �ִ�..
				nStPos ++;
			}
			else { // ����ؼ� �ܾ ����. 
				return nPrefix;
			}
			p++;
		}
		else if (chk>0) { // �۴�. 
			if (item.info.i.nNext == FILL_NEXT) {
				return nPrefix;
			}
			nStPos += item.info.i.nNext; // �ٽ� �����ͼ�...
		}
		else {
			return nPrefix; // �ٷ� ũ��...
		}
	}
	
	return nPrefix;
}

tINT FindMaxNTrie(NTRIE_TYPE *nTrie, tHCHAR *uszWord, tINT *nMaxFindItem, tINT *nPosItem)
{
	tINT nPos, nStPos;
	NTRIE_INDEXTYPE chkIndex;
	NTRIE_ITEMTYPE item;
	tHCHAR *p = uszWord;
	tINT nPrefix = 0;
	tINT chk;

	*nMaxFindItem = 0;
	*nPosItem = -1;

	if (uszWord[0] == 0x00) return 0;


	chkIndex.uszCh = uszWord[0];
	nPos = FindRecord(&(nTrie->Index), &chkIndex, TRUE, (CompProcP)CompNTrieIndex);
	if (nPos == -1) return 0;

	GetRecord(&(nTrie->Index), nPos, &chkIndex);
	nStPos =  chkIndex.nPos;

	while (*p) {
		GetRecord(&(nTrie->Body), nStPos, &item);
		chk = *p - item.uszCh;
		if (chk == 0) {

			*nPosItem = nStPos;

			*nMaxFindItem += 1;
			if (item.info.i.nEnd) { // ���̴� 
				nPrefix = *nMaxFindItem;	
			}
			if (item.info.i.nCont) { // ��� �پ��ִ� ���� �ִ�..
				nStPos ++;
			}
			else { // ����ؼ� �ܾ ����. 
				return nPrefix;
			}
			p++;
		}
		else if (chk>0) { // �۴�. 
			if (item.info.i.nNext == FILL_NEXT) {
				return nPrefix;
			}
			nStPos += item.info.i.nNext; // �ٽ� �����ͼ�...
		}
		else {
			return nPrefix; // �ٷ� ũ��...
		}
	}	
	return nPrefix;
}

PRIVATE tINT CompNTrieIndex(NTRIE_INDEXTYPE *item1, NTRIE_INDEXTYPE *item2)
{
	return ((tINT)(item1->uszCh) - (tINT)(item2->uszCh));
}

tBOOL ExpandNTrie( NTRIE_TYPE *nTrie, tHCHAR *uszWord, tINT nCount, tVOID *lpArg1, tVOID *lpArg2, tBOOL (*PutWordProc)(tHCHAR *str, tDWORD info, tVOID *lpArg1, tVOID *lpArg2) )
{
	tINT nLen, nLen1, nLen2;
	tINT nPrefix, nPosItem;
	NTRIE_ITEMTYPE nTrieItem;
	tINT nCountWord = 0, nSt, nStack[MAX_TRIE_WORD+1], nPosStack = 0;
	tHCHAR uszRetWord[MAX_TRIE_WORD];

	nLen1 = HStrlen(uszWord);
	nLen = FindMaxNTrie(nTrie, uszWord, &nPrefix, &nPosItem);

	if (nLen == nLen1) {
		GetRecord(&(nTrie->Body), nPosItem, &nTrieItem);
		if (PutWordProc(uszWord, nTrieItem.dwInfo, lpArg1, lpArg2)) {
			nCountWord ++;
		}
	}

	if (nCount != -1 && nCount <= nCountWord) return TRUE;


	if (nLen1 == nPrefix) { //�� �ڿ� ����ؼ�...
		nSt = nPosItem;
		HStrcpy(uszRetWord, uszWord);
		nLen2 = HStrlen(uszRetWord);
		GetRecord(&(nTrie->Body), nSt, &nTrieItem);

		if (nTrieItem.info.i.nCont) {
			nSt ++;
		}
		else {
			if (nCountWord) return TRUE;
			else return FALSE;
		}

		while (1) {
			GetRecord(&(nTrie->Body), nSt, &nTrieItem);
			if (nLen2 >= MAX_TRIE_WORD) break; //exception
			uszRetWord[nLen2++] = nTrieItem.uszCh;
			uszRetWord[nLen2] = 0x00;
			nStack[nPosStack++] = nSt;
			if (nTrieItem.info.i.nEnd) {
				if (PutWordProc(uszRetWord, nTrieItem.dwInfo, lpArg1, lpArg2)) {
					nCountWord ++;
				}
				if (nCount != -1 && nCount <= nCountWord) return TRUE;
			}
			if (nTrieItem.info.i.nCont) { // ���� ���ڰ� �ִ�. 
				nSt ++;
				continue;
			}
			else if (nPosStack == 0) { // �� �̻� ����. 
				break;
			}
			else {
RetryStack:;
				nPosStack --;
				nSt = nStack[nPosStack];
				GetRecord(&(nTrie->Body), nSt, &nTrieItem);
				nLen2 --;
				if (nTrieItem.info.i.nNext == FILL_NEXT) {
					if (nPosStack == 0) break;
					else goto RetryStack;
				}
				nSt += nTrieItem.info.i.nNext;
			}
		}
	}
		
	if (nCountWord) return TRUE;
	return FALSE;
}

tBOOL TraverseNTrie( NTRIE_TYPE *nTrie, tINT nCount, tVOID *lpArg1, tVOID *lpArg2, tBOOL (*PutWordProc)(tHCHAR *str, tDWORD info, tVOID *lpArg1, tVOID *lpArg2) )
{
	tINT i, nLen2;
	NTRIE_ITEMTYPE nTrieItem;
	NTRIE_INDEXTYPE nTrieIndex;
	tINT nCountWord = 0, nSt, nStack[MAX_TRIE_WORD+1], nPosStack = 0;
	tHCHAR uszRetWord[MAX_TRIE_WORD];

	nLen2 = 0;

	for ( i = 0 ; i < (nTrie->Index).nUsed ; i ++ ) {
		GetRecord(&(nTrie->Index), i, &nTrieIndex);	
		nSt = nTrieIndex.nPos;
		nLen2 = 0;
/*
		uszRetWord[nLen2++] = nTrieIndex.uszCh;
		uszRetWord[nLen2] = 0x00;

		GetRecord(&(nTrie->Body), nSt, &nTrieItem);

		if (nTrieItem.info.i.nCont) {
			nSt ++;
		}
		else {
			continue;
		}
*/

		while (1) {
			GetRecord(&(nTrie->Body), nSt, &nTrieItem);
			if (nLen2 >= MAX_TRIE_WORD) break; //exception
			uszRetWord[nLen2++] = nTrieItem.uszCh;
			uszRetWord[nLen2] = 0x00;
			nStack[nPosStack++] = nSt;
			if (nTrieItem.info.i.nEnd) {
				if (PutWordProc(uszRetWord, nTrieItem.dwInfo, lpArg1, lpArg2)) {
					nCountWord ++;
				}
				if (nCount != -1 && nCount <= nCountWord) return TRUE;
			}
			if (nTrieItem.info.i.nCont) { // ���� ���ڰ� �ִ�. 
				nSt ++;
				continue;
			}
			else if (nPosStack == 0) { // �� �̻� ����. 
				break;
			}
			else {
RetryStack:;
				nPosStack --;
				nSt = nStack[nPosStack];
				GetRecord(&(nTrie->Body), nSt, &nTrieItem);
				nLen2 --;
				if (nTrieItem.info.i.nNext == FILL_NEXT) {
					if (nPosStack == 0) break;
					else goto RetryStack;
				}
				nSt += nTrieItem.info.i.nNext;
			}
		}
	}
		
	if (nCountWord) return TRUE;
	return FALSE;
}
