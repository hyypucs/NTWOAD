#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sched.h>
#include <sys/types.h>
#include <ctype.h>
#include "dwtype.h"
#include "hstrlib.h"
#include "dicsig.h"
#include "record.h"
#include "hcode.h"
#include "estem.h"
#include "util.h"
#include "estoplist.h"
#include "kshanja2han.h"

#include "mutex.h"
#include "scan.h"
#include "stag.h"
#include "newindexlib.h"

MUTEXMAN_FORMAT MUTEX_SCAN;
tBOOL OpenUserDic(tCHAR *szDicName);
tVOID CloseUserDic(tVOID);
tBOOL FindUserDic(tCHAR *szWord);

tVOID OpenOkNumDic(tCHAR *szDicName);
tVOID CloseOkNumDic(tVOID);
tBOOL OkNumStr(tCHAR *szWord);

tBOOL OpenStopWord(tCHAR *szDicName);
tVOID CloseStopWord(tVOID);
tBOOL FindStopWord(tCHAR *szWord);

tBOOL OpenIndexThDic(tCHAR *szDic);
tBOOL CloseIndexThDic(tVOID);
tBOOL GetFindThDic(tCHAR *szStr, RECORD *lpRetRec);
tVOID AddIWord(IWORDLIST_FORMAT *WordList, tCHAR *szIWord);
tVOID AddIWordList(RECORD *IndexRec, IWORDLIST_FORMAT IWordList, tINT SetOpt);
tVOID AddIWordListOne(RECORD *IndexRec, tCHAR *szIWord, tINT SetOpt);
tVOID AddTraveseIndexRec(RECORD *IndexWord, RECORD *ListIndexRec);

/*
typedef struct {
        tCHAR szWord[MAX_TOKEN_LEN];
        tINT   nStPos;
}TOKEN_FORMAT, *LPTOKEN_FORMAT;
tINT GetTokenFromLine(tCHAR *szLine, RECORD *lpTokenRec);
*/

tBOOL IsNumStr(tCHAR *szWord);
tBOOL IsEngStr(tCHAR *szWord);
tBOOL IsHjStr(tCHAR *szWord);
tINT AnalyzeWord(tHCHAR *tmpWord, tINT *WhatWord, tINT *PosWord, tINT *nRetCount);
tBOOL ScanForOne(tCHAR *szStr, RECORD *lpScanRecord);
tVOID IndexThWordFromIndexWord(RECORD *lpRetIndexRecord);

tVOID ProcVVAJ(RECORD *lpScanRecord, _SCANDAT_LemmaList *p, tCHAR *szSrcWord);
tVOID ProcCompWord(RECORD *lpScanRecord, IWORDLIST_FORMAT IWordList, tCHAR *szSrcWord);
tVOID ProcPF(RECORD *lpScanRecord, _SCANDAT_LemmaList *p, tCHAR *szSrcWord);
tVOID ProcNN(RECORD *lpScanRecord, _SCANDAT_LemmaList *p, tCHAR *szSrcWord);
tINT GetSubTokenFromToken(RECORD *lpTokenRec, tCHAR *szLine, tINT nPreSt, tINT nStrLen);


tBOOL IsNumStr(tCHAR *szWord)
{
        tINT i;

        for ( i = 0 ; i < (int)strlen(szWord) ; i ++ ) {
		if (szWord[i] & 0x80) return FALSE;
                if (!isdigit(szWord[i])) return FALSE;
        }

        return TRUE;
}

tBOOL IsEngStr(tCHAR *szWord)
{
        tINT i;

        for ( i = 0 ; i < (int)strlen(szWord) ; i ++ ) {
		if (szWord[i] & 0x80) return FALSE;
                if (!isalpha(szWord[i])) return FALSE;
        }

        return TRUE;
}

tBOOL IsHjStr(tCHAR *szWord)
{
        tINT i;
	tBYTE ch1, ch2;

        for ( i = 0 ; i < (int)strlen(szWord) ; i ++ ) {
		if (!(szWord[i] & 0x80)) return FALSE;
		ch1 = szWord[i];
		ch2 = szWord[i+1];
	
		if (ch1 >= 0xca && ch1 <= 0xdf && ch2 >= 0xa1 && ch2 <= 0xfe) { // ����
			i++;
			continue;
		}
		if (ch1 >= 0xe0 && ch1 <= 0xfd && ch2 >= 0xa1 && ch2 <= 0xfe) {
			i++;
			continue;
		}
		return FALSE;
        }

        return TRUE;

}


//tINT GetTokenFromLine(tCHAR *szLine, RECORD *lpTokenRec);

tBOOL OpenIndexLib(tCHAR *szRoot)
{
	tCHAR szR[MAX_PATH];
	tCHAR szFileName[MAX_PATH];

	strcpy(szR, szRoot);
	if (szRoot[strlen(szRoot) - 1] != '/') {
		strcat(szR, "/");	
	}
	// ���� ����
	sprintf(szFileName, "%sdic/oknum.dat", szR);
        OpenOkNumDic(szFileName);

	sprintf(szFileName, "%sdic/%s", szR, "user.dic");
        OpenUserDic(szFileName);

	sprintf(szFileName, "%sdic/%s", szR, "stopword_ks.dic");
        OpenStopWord(szFileName);

	sprintf(szFileName, "%sdic/%s", szR, "indexth_ks");
        OpenIndexThDic(szFileName);

	sprintf(szFileName, "%sscan/", szR);
	printf("scan Directory : %s\n", szFileName);
        if (_SCANAPI_initialize(szFileName)!=eOK) {
                printf("SCAN System Fail\n");
		CloseOkNumDic();
		CloseUserDic();
		CloseStopWord();
		CloseIndexThDic();
                return FALSE;
        }
        printf("Scan system initialized.\n");
        if (_STAGAPI_initialize(szFileName)!=eOK) {
                printf("STAG System Fail\n");
                _SCANAPI_finalize();
                CloseOkNumDic();
		CloseUserDic();
		CloseStopWord();
		CloseIndexThDic();
                return FALSE;
        }
        if (BeginMutexMan( &MUTEX_SCAN, 1 ) == FALSE) {
                printf("Mutex Scan Error\n");
                _SCANAPI_finalize();
                _STAGAPI_finalize();
                CloseOkNumDic();
		CloseUserDic();
		CloseStopWord();
		CloseIndexThDic();
                return FALSE;
        }
        printf("STAG system ok\n");
	printf("OK IndexLib\n");
	return TRUE;

}

tVOID CloseIndexLib(tVOID)
{
	CloseOkNumDic();
	CloseUserDic();
	CloseStopWord();
	CloseIndexThDic();

	_SCANAPI_finalize();
        printf("Scan close\n");
        _STAGAPI_finalize();
        printf("STAG close\n");
        EndMutexMan(&MUTEX_SCAN);
}


tVOID AddIWord(IWORDLIST_FORMAT *WordList, tCHAR *szIWord)
{

	if (strlen(szIWord) >= MAX_WORD_LEN * 2) return; // Ŀ�� �� �ִ´�.
	if (WordList->nC_IWord >= MAX_IWORDLIST) return ; // 20�� �̻� �и��Ǿ���.
	strcpy(WordList->szIWord[WordList->nC_IWord], szIWord);
	(WordList->nC_IWord) ++;
}

tVOID AddIWordList(RECORD *IndexRec, IWORDLIST_FORMAT IWordList, tINT SetOpt)
{
	IWORDLIST_FORMAT IWordList1;
	tINT i, j;

	if (IWordList.nC_IWord > MAX_IWORDLIST || IWordList.nC_IWord <= 0) return ; // 20�� �̻� �и��Ǿ���.

	for ( i = 0 ; i < IndexRec->nUsed ; i ++ ) {
		GetRecord(IndexRec, i, &IWordList1);
		if (IWordList1.nC_IWord != IWordList.nC_IWord) {
			continue;
		}

		for ( j = 0 ; j < IWordList1.nC_IWord ; j ++ ) {
			if (strcmp(IWordList1.szIWord[j], IWordList.szIWord[j]) != 0) break;
		}
		if (j < IWordList1.nC_IWord) continue; // �ٸ��ϱ�.. 

		IWordList1.nIndexInfo |= SetOpt;
		SetRecord(IndexRec, i, &IWordList1);
		break;
	}
	if (i < IndexRec->nUsed) return ; // ���̴�.

	//�߰��ؾ� �Ѵ�.
	IWordList.nIndexInfo = SetOpt;
	AppendRecord(IndexRec, &IWordList);
}

tVOID AddIWordListOne(RECORD *IndexRec, tCHAR *szIWord, tINT SetOpt)
{
	IWORDLIST_FORMAT IWordList;

	memset(&IWordList, 0, sizeof(IWORDLIST_FORMAT));
	AddIWord(&IWordList, szIWord);
	AddIWordList(IndexRec, IWordList, SetOpt);
}

tVOID AddTraveseIndexRec(RECORD *IndexWord, RECORD *ListIndexRec)
{
	// �ѹ� �غ���.
	tINT c0, c1, c2, c3, i, j;
	IWORDLIST_FORMAT IWordList0, IWordList1, IWordList2, IWordList3;
	RECORD IndexRec0, IndexRec1, IndexRec2, IndexRec3;


	if (ListIndexRec->nUsed == 0) {
		return ;
	}
	else if (ListIndexRec->nUsed == 1) {
		GetRecord(ListIndexRec, 0, &IndexRec0);
		for ( c0 = 0 ; c0 < IndexRec0.nUsed ; c0 ++ ) {
			GetRecord(&IndexRec0, c0, &IWordList0);
			AddIWordList(IndexWord, IWordList0, IWordList0.nIndexInfo);
		}
	}
	else if (ListIndexRec->nUsed == 2) {
		GetRecord(ListIndexRec, 0, &IndexRec0);
		GetRecord(ListIndexRec, 1, &IndexRec1);
		for ( c0 = 0 ; c0 < IndexRec0.nUsed ; c0 ++ ) {
			for ( c1 = 0 ; c1 < IndexRec1.nUsed ; c1 ++ ) {
				GetRecord(&IndexRec0, c0, &IWordList0);
				GetRecord(&IndexRec1, c1, &IWordList1);
				for ( i = 0 ; i < IWordList1.nC_IWord ; i++ ) {
					AddIWord(&IWordList0, IWordList1.szIWord[i]);
				}
				IWordList0.nIndexInfo |= IWordList1.nIndexInfo;
				AddIWordList(IndexWord, IWordList0, IWordList0.nIndexInfo);
			}
		}

	}
	else if (ListIndexRec->nUsed == 3) {
		GetRecord(ListIndexRec, 0, &IndexRec0);
		GetRecord(ListIndexRec, 1, &IndexRec1);
		GetRecord(ListIndexRec, 2, &IndexRec2);
		for ( c0 = 0 ; c0 < IndexRec0.nUsed ; c0 ++ ) {
			for ( c1 = 0 ; c1 < IndexRec1.nUsed ; c1 ++ ) {
				for ( c2 = 0 ; c2 < IndexRec2.nUsed ; c2 ++ ) {
					GetRecord(&IndexRec0, c0, &IWordList0);
					GetRecord(&IndexRec1, c1, &IWordList1);
					GetRecord(&IndexRec2, c2, &IWordList2);
					for ( i = 0 ; i < IWordList1.nC_IWord ; i++ ) {
						AddIWord(&IWordList0, IWordList1.szIWord[i]);
					}
					for ( i = 0 ; i < IWordList2.nC_IWord ; i++ ) {
						AddIWord(&IWordList0, IWordList2.szIWord[i]);
					}
					IWordList0.nIndexInfo |= IWordList1.nIndexInfo;
					IWordList0.nIndexInfo |= IWordList2.nIndexInfo;
					AddIWordList(IndexWord, IWordList0, IWordList0.nIndexInfo);
				}
			}
		}
	}
	else if (ListIndexRec->nUsed == 4) {
		GetRecord(ListIndexRec, 0, &IndexRec0);
		GetRecord(ListIndexRec, 1, &IndexRec1);
		GetRecord(ListIndexRec, 2, &IndexRec2);
		GetRecord(ListIndexRec, 3, &IndexRec3);
		for ( c0 = 0 ; c0 < IndexRec0.nUsed ; c0 ++ ) {
			for ( c1 = 0 ; c1 < IndexRec1.nUsed ; c1 ++ ) {
				for ( c2 = 0 ; c2 < IndexRec2.nUsed ; c2 ++ ) {
					for ( c3 = 0 ; c3 < IndexRec3.nUsed ; c3 ++ ) {
						GetRecord(&IndexRec0, c0, &IWordList0);
						GetRecord(&IndexRec1, c1, &IWordList1);
						GetRecord(&IndexRec2, c2, &IWordList2);
						GetRecord(&IndexRec3, c3, &IWordList3);
						for ( i = 0 ; i < IWordList1.nC_IWord ; i++ ) {
							AddIWord(&IWordList0, IWordList1.szIWord[i]);
						}
						for ( i = 0 ; i < IWordList2.nC_IWord ; i++ ) {
							AddIWord(&IWordList0, IWordList2.szIWord[i]);
						}
						for ( i = 0 ; i < IWordList3.nC_IWord ; i++ ) {
							AddIWord(&IWordList0, IWordList3.szIWord[i]);
						}
						IWordList0.nIndexInfo |= IWordList1.nIndexInfo;
						IWordList0.nIndexInfo |= IWordList2.nIndexInfo;
						IWordList0.nIndexInfo |= IWordList3.nIndexInfo;
						AddIWordList(IndexWord, IWordList0, IWordList0.nIndexInfo);
					}
				}
			}
		}
	}
	else { // exception ó�� �͸� ������.
		GetRecord(ListIndexRec, 0, &IndexRec0);
		GetRecord(&IndexRec0, 0, &IWordList0);
		for (	i = 1 ; i < ListIndexRec->nUsed ; i ++ ) {
			GetRecord(ListIndexRec, i, &IndexRec1);
			GetRecord(&IndexRec1, 0, &IWordList1);
			for ( j = 0 ; j < IWordList1.nC_IWord ; j++ ) {
				AddIWord(&IWordList0, IWordList1.szIWord[j]);
			}
			IWordList0.nIndexInfo |= IWordList1.nIndexInfo;
		}
		AddIWordList(IndexWord, IWordList0, IWordList0.nIndexInfo);
	}
}

tBOOL IsUnicode(tCHAR *szWord);

tBOOL IsUnicode(tCHAR *szWord)
{
	tCHAR *p, *q;

	p = strstr(szWord, "&#");
	if (p == NULL) return FALSE;

	if (!isdigit(*(p+2))) return FALSE;

	q = strchr(p+2, ';');	
	if (q) return TRUE;

	return FALSE;
}

tVOID FreeIndexRes(RECORD *lpResIndexRec)
{
	tINT i;
	INDEX_RESULT_FORMAT Index;

	for ( i = 0 ; i < lpResIndexRec->nUsed ; i ++) {
		GetRecord(lpResIndexRec, i, &Index);
		if (Index.IndexWord.nUsed <= 0) continue;
		FreeRecord(&(Index.IndexWord));	
	}
	FreeRecord(lpResIndexRec);
}


// bInsertOne=TRUE : �� ��ū�� �ϳ� �ִ´�.
// FALSE�̸� �׳� �״��..
tBOOL IndexStrIndexLib(tCHAR *szStr, RECORD *lpResIndexRec, tBOOL bInsertOne)
{
	RECORD TokenRec;
	tINT i; //, j;
	TOKEN_FORMAT Token;
	tCHAR szWord[MAX_TOKEN_LEN*2];
	INDEX_RESULT_FORMAT ResIndex;
	tHCHAR uszWord[MAX_TOKEN_LEN * 2];
	tHCHAR uszTmpWordHg[MAX_TOKEN_LEN * 2];
	//IWORDLIST_FORMAT IWordList;
	RECORD IndexRec, ListIndexRec;
	

	tINT PosWord[MAX_COMPWORD+1], nCount = 0, nWordLen, k;
        tINT WhatWord[MAX_COMPWORD+1];


	if (szStr[0] == '\0') return TRUE; // exception

	AllocRecord(&TokenRec, sizeof(TOKEN_FORMAT), 100, 10);
	GetTokenFromLine(szStr, &TokenRec);

	//IWord.nIndexInfo = 0; // �ʱ�ȭ

	for (i = 0 ; i < TokenRec.nUsed ; i ++ ) {
		GetRecord(&TokenRec, i, &Token);
		strcpy(szWord, Token.szWord);
		LowStr(szWord);	

		strcpy(ResIndex.szToken, Token.szWord);
		ResIndex.nPos = Token.nStPos;
		ResIndex.nPosToken = i;
		AllocRecord(&(ResIndex.IndexWord), sizeof(IWORDLIST_FORMAT), 10, 1);

		if (IsUnicode(szWord)) { // unicode�̴�..
			AddIWordListOne(&(ResIndex.IndexWord), szWord, ONE_I);
			AppendRecord(lpResIndexRec, &ResIndex);
			continue;
		}

		if (bInsertOne) { // �׻� �ϳ� �ֱ�.
			AddIWordListOne(&(ResIndex.IndexWord), szWord, ONE_I);
		}

		if (FindUserDic(szWord)) {
			AddIWordListOne(&(ResIndex.IndexWord), szWord, USER_I | ONE_I);
		}
		if (IsNumStr(szWord)) {
                        if (OkNumStr(szWord)) {
				AddIWordListOne(&(ResIndex.IndexWord), szWord, ONE_I);
                        }
			// 0�̸� Free�ؾ� �Ѵ�. ������.. �ִٸ� �߰��ϰ���..
			if (ResIndex.IndexWord.nUsed == 0) {
                		FreeRecord(&(ResIndex.IndexWord));
				//AppendRecord(lpResIndexRec, &ResIndex);
            		}
			else {
			// ���� ����� ���ÿ� �߰��ؾ� �Ѵ�.
				IndexThWordFromIndexWord(&(ResIndex.IndexWord));
				AppendRecord(lpResIndexRec, &ResIndex);
			}
                        continue;
                }
		if (IsEngStr(szWord)) {
			if (FindStopWord(szWord)) { //stopword
				//AppendRecord(lpResIndexRec, &ResIndex);
                		FreeRecord(&(ResIndex.IndexWord));
				//printf("stopword[%s]",szWord);
				continue;
			}
			Stem(szWord);
			AddIWordListOne(&(ResIndex.IndexWord), szWord, ONE_I);
			// 0�̸� Free�ؾ� �Ѵ�. ������.. �ִٸ� �߰��ϰ���..
			if (ResIndex.IndexWord.nUsed == 0) {
				//AppendRecord(lpResIndexRec, &ResIndex);
                		FreeRecord(&(ResIndex.IndexWord));
            		}
			else {
			// ���� ����� ���ÿ� �߰��ؾ� �Ѵ�.
				IndexThWordFromIndexWord(&(ResIndex.IndexWord));
				AppendRecord(lpResIndexRec, &ResIndex);
			}
			continue;
		}
		if (IsHjStr(szWord)) {
			KSHjStr2HgStr( szWord, szWord );
			if (FindStopWord(szWord)) { //stopword
				//AppendRecord(lpResIndexRec, &ResIndex);
                		FreeRecord(&(ResIndex.IndexWord));
				continue;
			}
			AddIWordListOne(&(ResIndex.IndexWord), szWord, ONE_I|HJ_I);
			// 0�̸� Free�ؾ� �Ѵ�. ������.. �ִٸ� �߰��ϰ���..
			if (ResIndex.IndexWord.nUsed == 0) {
				//AppendRecord(lpResIndexRec, &ResIndex);
                		FreeRecord(&(ResIndex.IndexWord));
            		}
			else {
				// ���� ����� ���ÿ� �߰��ؾ� �Ѵ�.
				IndexThWordFromIndexWord(&(ResIndex.IndexWord));
				AppendRecord(lpResIndexRec, &ResIndex);
			}
			continue;
		}
		// scan���� ������.
		ScanForOne(szWord, &(ResIndex.IndexWord));

		str2hstr((tBYTE *)szWord, uszWord);

		// ���� �ܾ� ���� �ִ�.

		AnalyzeWord(uszWord, WhatWord, PosWord, &nCount);
		if (nCount > 1)  {

			AllocRecord(&ListIndexRec, sizeof(RECORD), nCount, 1); // �ϳ��� ����Ʈ�� ���� �̰� ������ �ٽ� �и��Ͽ� ó���ؾ� �Ѵ�.
			for ( k = 0 ; k < nCount ; k ++ ) {
				AllocRecord(&IndexRec, sizeof(IWORDLIST_FORMAT), 10, 1); // �ϳ��� ����Ʈ�� ���� �̰� ������ �ٽ� �и��Ͽ� ó���ؾ� �Ѵ�.

                                nWordLen = PosWord[k+1] - PosWord[k];
                                HStrncpy(uszTmpWordHg, uszWord+PosWord[k], nWordLen);
                                uszTmpWordHg[nWordLen] = 0x00;		 
				hstr2str(uszTmpWordHg, (tBYTE *)szWord);

				if (FindUserDic(szWord)) {
					AddIWordListOne(&IndexRec, szWord,  COMPOUND_I|USER_I);
					AppendRecord(&ListIndexRec, &IndexRec);
					continue;
				}
				else if (WhatWord[k] == ETC_WORD) {
					FreeRecord(&IndexRec);
					continue; // ���־���.
				}
				else if (WhatWord[k] == E_WORD) {
#if defined(_NOT_INDEX_ENG_ONE)
                                        if (strlen(szWord) == 1) {
						FreeRecord(&IndexRec);
						continue; // ����¥�� �����
					}	
#endif
					if (FindStopWord(szWord)) { //stopword
						FreeRecord(&IndexRec);
						continue;
					}
					Stem(szWord);
					AddIWordListOne(&IndexRec, szWord, COMPOUND_I);
                                }
                                else if (WhatWord[k] == N_WORD) {
                        		if (OkNumStr(szWord)) {
						AddIWordListOne(&IndexRec, szWord, COMPOUND_I);
                        		}
					else {
						FreeRecord(&IndexRec);
						continue;
					}
                                }
                                else if (WhatWord[k] == HJ_WORD) {
					KSHjStr2HgStr( szWord, szWord );
					if (FindStopWord(szWord)) { //stopword
						FreeRecord(&IndexRec);
						continue;
					}
					AddIWordListOne(&IndexRec, szWord, COMPOUND_I|HJ_I);
				}
				else if (WhatWord[k] == H_WORD) {
					ScanForOne(szWord, &IndexRec);
					if (IndexRec.nUsed <= 0) { // exception
						AddIWordListOne(&IndexRec, szWord, COMPOUND_I);
					}
				}
				if (IndexRec.nUsed > 0) {
					AppendRecord(&ListIndexRec, &IndexRec);
				}
				else {
					FreeRecord(&IndexRec);
				}
			}
			// traverse�ϸ鼭.. �ش� �κ��� �ٽ� �ٸ缭 ����ؾ� �Ѵ�.
			AddTraveseIndexRec(&(ResIndex.IndexWord), &ListIndexRec);

			// free
			for ( k = 0 ; k < ListIndexRec.nUsed ; k ++ ) {
				GetRecord(&ListIndexRec, k, &IndexRec);
				FreeRecord(&IndexRec);
			}
			FreeRecord(&ListIndexRec);

		}

		// 0�̸� Free�ؾ� �Ѵ�. ������.. �ִٸ� �߰��ϰ���..
		if (ResIndex.IndexWord.nUsed <= 0) {
            		FreeRecord(&(ResIndex.IndexWord));
        	}
		else {
			// ���� ����� ���ÿ� �߰��ؾ� �Ѵ�.
			IndexThWordFromIndexWord(&(ResIndex.IndexWord));
			AppendRecord(lpResIndexRec, &ResIndex);
		}
	}
	FreeRecord(&TokenRec);

	return TRUE;
}

tVOID IndexThWordFromIndexWord(RECORD *lpRetIndexRecord)
{
	RECORD tmpRec;
	tINT i, j;
	IWORDLIST_FORMAT IWord;
	RECORD WordList;
        STR_FORMAT Str;
	
	RecordCpyWithAlloc(&tmpRec, lpRetIndexRecord);
	AllocRecord(&WordList, sizeof(STR_FORMAT), 10, 10);

	for ( i = 0 ; i < tmpRec.nUsed ; i ++ ) {
		GetRecord(&tmpRec, i, &IWord);

		if (IWord.nC_IWord !=1) continue; // �� ���� ���� ���� üũ�Ѵ�.

		GetFindThDic(IWord.szIWord[0], &WordList);
		if (WordList.nUsed == 0) continue;

		for ( j = 0 ; j < WordList.nUsed ; j ++ ) {
			GetRecord(&WordList, j, &Str);
			AddIWordListOne(lpRetIndexRecord, Str.szStr, THE_I);
		}
		WordList.nUsed  = 0;
	}
	FreeRecord(&WordList);
	FreeRecord(&tmpRec);
}



/* ����� ���� */
PRIVATE tBOOL bOpenUserDic = FALSE;
PRIVATE RECORD UserRec;

typedef struct {
        tCHAR szUserStr[40]; // kssm style
}USERDIC_TYPE;

PRIVATE tINT CompUserData(USERDIC_TYPE *item1, USERDIC_TYPE *item2);

PRIVATE tINT CompUserData(USERDIC_TYPE *item1, USERDIC_TYPE *item2)
{
        return (strcmp(item1->szUserStr, item2->szUserStr));
}

tBOOL OpenUserDic(tCHAR *szDicName)
{
        USERDIC_TYPE UserData;
	tINT i;

        if (bOpenUserDic) return TRUE;

        File2Record(&UserRec, szDicName, sizeof(USERDIC_TYPE), 100, 100, TRUE );

	// �ϼ��� �ڵ�� �ٲ۴�.
	for ( i = 0 ; i < UserRec.nUsed ; i ++ ) {
		GetRecord(&UserRec, i, &UserData);
		KSSMStr2KSStr((tBYTE *)UserData.szUserStr, (tBYTE *)UserData.szUserStr);
	}
        bOpenUserDic = TRUE;

        return TRUE;
}

tVOID CloseUserDic(tVOID)
{

        if (bOpenUserDic) {
                FreeRecord(&UserRec);
                bOpenUserDic = FALSE;
        }
}

tBOOL FindUserDic(tCHAR *szWord)
{
        USERDIC_TYPE UserData;
        tINT nPos;

        if (!bOpenUserDic) return FALSE;

        if (UserRec.nUsed <= 0) return FALSE;

        if (strlen(szWord) >= 40) return FALSE;

	strcpy(UserData.szUserStr, szWord);

        nPos = FindRecord(&UserRec,  &UserData, 1, (CompProcP)CompUserData);
        if (nPos == -1) return FALSE;

        return TRUE;
}

//
// stopword dic
//
PRIVATE tBOOL bOpenStopWord = FALSE;
PRIVATE RECORD StopWordRec;

typedef struct {
        tCHAR szStr[40]; // ks style
}STOPWORD_TYPE;

PRIVATE tINT CompStopWord(STOPWORD_TYPE *item1, STOPWORD_TYPE *item2);

PRIVATE tINT CompStopWord(STOPWORD_TYPE *item1, STOPWORD_TYPE *item2)
{
        return (strcmp(item1->szStr, item2->szStr));
}

tBOOL OpenStopWord(tCHAR *szDicName)
{
#if defined(_NOT_USE_STOPWORD)
	return TRUE;
#endif
        if (bOpenStopWord) return TRUE;

        File2Record(&StopWordRec, szDicName, sizeof(STOPWORD_TYPE), 100, 100, TRUE );

        bOpenStopWord = TRUE;

        return TRUE;
}

tVOID CloseStopWord(tVOID)
{

#if defined(_NOT_USE_STOPWORD)
	return;
#endif
        if (bOpenStopWord) {
                FreeRecord(&StopWordRec);
                bOpenStopWord = FALSE;
        }
}

tBOOL FindStopWord(tCHAR *szWord)
{
        STOPWORD_TYPE StopWordItem;
        tINT nPos;

#if defined(_NOT_USE_STOPWORD)
	return FALSE;
#endif
	// ������ ��..
	if (IsEngStr(szWord)) if (IsEStopWord(szWord)) return TRUE;

        if (!bOpenStopWord) return FALSE;

        if (StopWordRec.nUsed <= 0) return FALSE;

        if (strlen(szWord) >= 40) return FALSE;

	strcpy(StopWordItem.szStr, szWord);

        nPos = FindRecord(&StopWordRec,  &StopWordItem, 1, (CompProcP)CompStopWord);
        if (nPos == -1) return FALSE;

        return TRUE;
}


//
// ���ξ� ����ܾ� ������ �� ���� �Ѵ�.
//


PRIVATE tBOOL bOpenIndexThDic = FALSE;
PRIVATE RECORD RecThStr, RecStr;

tINT CompRecStr(STR_FORMAT *item1, STR_FORMAT *item2);
tINT CompRecThStr(THSTR_FORMAT *item1, THSTR_FORMAT *item2);

tINT CompRecStr(STR_FORMAT *item1, STR_FORMAT *item2)
{
	return (strcmp(item1->szStr, item2->szStr));
}

tINT CompRecThStr(THSTR_FORMAT *item1, THSTR_FORMAT *item2)
{
	return (strcmp(item1->szStr, item2->szStr));
}

tBOOL OpenIndexThDic(tCHAR *szDic)
{
	tCHAR szFileName[MAX_PATH];

	if (bOpenIndexThDic) return TRUE;
	sprintf(szFileName, "%s.idx", szDic);
	File2Record(&RecThStr, szFileName, sizeof(THSTR_FORMAT), 100, 10, TRUE);

	sprintf(szFileName, "%s.dat", szDic);
	File2Record(&RecStr, szFileName, sizeof(STR_FORMAT), 100, 10, TRUE);

	bOpenIndexThDic = TRUE;

	return TRUE;
}

tBOOL CloseIndexThDic(tVOID)
{
	if (bOpenIndexThDic == FALSE) return TRUE;

	FreeRecord(&RecThStr);
	FreeRecord(&RecStr);

	bOpenIndexThDic = FALSE;

	return TRUE;
}

tBOOL GetFindThDic(tCHAR *szStr, RECORD *lpRetRec)
{
	THSTR_FORMAT ThStr;
	tINT nPos, i;
	STR_FORMAT Str;

	if (bOpenIndexThDic == FALSE) return FALSE;
	if (strlen(szStr) >= MAX_THSTR) return FALSE; // exception
	strcpy(ThStr.szStr, szStr);

	nPos = FindRecord(&RecThStr, &ThStr, TRUE, (CompProcP)CompRecThStr);

	if (nPos == -1) return FALSE;

	GetRecord(&RecThStr, nPos, &ThStr);

	i = 0;
	while (i < C_THSTR && ThStr.nStr[i] >= 0) {
		GetRecord(&RecStr, ThStr.nStr[i], &Str);

		AppendRecord(lpRetRec, &Str);
		i ++ ;
	}

	return TRUE;
}



#define old_MAX_OKNUM 14
tCHAR old_OKNum[old_MAX_OKNUM][5] = {
        {"001"},
        {"114"},
        {"002"},
        {"700"},
        {"5425"},
        {"011"},
        {"012"},
        {"007"},
        {"016"},
        {"017"},
        {"018"},
        {"019"},
        {"010"},
        {"2002"}};

tINT MAX_OKNUM = 0;
tCHAR *OKNum[50];

tVOID OpenOkNumDic(tCHAR *szDicName)
{
	FILE *fp;
	tCHAR szLine[100], szStr[100];
	tINT i;
#if defined(_NOT_USE_OKNUM)
	printf("Not Used OkNum\n");
	return;
#endif
	fp = fopen(szDicName, "rt");
	if (fp) {
		while(fgets(szLine, 99, fp)) {
			if (MAX_OKNUM >= 50) {	
				printf("ok num limit 50 exceed..\n");
				continue;
			}
			if (sscanf(szLine, "%s", szStr) != 1) continue;	
			OKNum[MAX_OKNUM] = (tCHAR *)malloc(strlen(szStr)*sizeof(tCHAR)+2);
			if (OKNum[MAX_OKNUM]) {
				strcpy(OKNum[MAX_OKNUM], szStr);
				MAX_OKNUM ++;
			}
		}
		fclose(fp);
	}
	else {
		for ( i = 0 ; i < old_MAX_OKNUM ;  i++ ) {
			OKNum[MAX_OKNUM] = (tCHAR *)malloc(strlen(old_OKNum[i])*sizeof(tCHAR) + 2);
			if (OKNum[MAX_OKNUM]) {
				strcpy(OKNum[MAX_OKNUM], szStr);
				MAX_OKNUM ++;
			}
		}
	}
	printf("OKNum Count : %d\n", MAX_OKNUM);
}

tVOID CloseOkNumDic(tVOID)
{
	tINT i;

#if defined(_NOT_USE_OKNUM)
	return;
#endif
	for ( i = 0 ; i < MAX_OKNUM ; i ++ ) {
		if (OKNum[i]) free(OKNum[i]);
		OKNum[i] = NULL;
	}
}
tBOOL OkNumStr(tCHAR *szWord)
{
	tINT i;

#if defined(_NOT_USE_OKNUM)
	return TRUE; // ������ �ǰ� �ؾ���.
#endif
	if (strlen(szWord) >= 20) return FALSE;
	for (i  = 0 ; i < MAX_OKNUM ; i ++ ) {
		if (strcmp(OKNum[i], szWord) == 0) return TRUE;
	}
	return FALSE;
}

tINT GetSubTokenFromToken(RECORD *lpTokenRec, tCHAR *szLine, tINT nPreSt, tINT nStrLen)
{
	tINT i, nSt = 0, nLen;
	TOKEN_FORMAT Token;
	//tCHAR szWord[MAX_WORD_LEN*2];
	tCHAR *p;
	tINT nCountAddToken = 0;

	for (i = 0; i < (int)nStrLen ; i ++ ) {
		if (szLine[i] & 0x80) {
			i++;
			continue;
		}
		else if (!isalnum(szLine[i])) {
			if ((szLine[i] == '&' || szLine[i] == '/')  && i > 0 && i+1 < nStrLen && IsEnglish(szLine[i-1]) && IsEnglish(szLine[i+1])) {
				continue;
			}
			if ((szLine[i] == '&' || szLine[i] == '/' || szLine[i] == '.' || szLine[i] == '-' || szLine[i] == '_')  && i > 0 && i+1 < nStrLen && IsEnglish(szLine[i-1]) && (IsEnglish(szLine[i+1]) || isdigit(szLine[i+1])) ) {
				continue;
			}
			if ((szLine[i] == '-')  && i > 0 && i+1 < nStrLen && isalpha(szLine[i-1]) && isdigit(szLine[i+1])) {
				continue;
			}
			if (szLine[i] == '&' && i+3 < nStrLen && szLine[i+1] == '#' && isdigit(szLine[i+2])) {
				p = strchr(szLine+i, ';');
				if (p) {
					//i = (int)p - (int)szLine;
					i = p - szLine;
					continue;
				}
			}
			if (nSt == i) {
				nSt ++;
				continue;
			}
			
			nLen = i - nSt;
			if (nLen >= MAX_TOKEN_LEN) { // exception
				strncpy(Token.szWord, szLine+nSt, MAX_TOKEN_LEN-1);
				Token.szWord[MAX_TOKEN_LEN-1] = '\0';
				Token.nStPos = nSt+nPreSt;
				AppendRecord(lpTokenRec, &Token);
				nCountAddToken ++ ;
				nSt = i+1;
				continue;
			}

			strncpy(Token.szWord, szLine+nSt, nLen);
			Token.szWord[nLen] = '\0';
			// �� ����... �ɺ��� ������ .. &�� .�� ���ܽ��״�. �̰��� �˻��� �� ���� ���� �� ����..
			if (Token.szWord[nLen-1] == '&' ||Token.szWord[nLen-1] == '.' ||Token.szWord[nLen-1] == '-' ||Token.szWord[nLen-1] == '_' ||Token.szWord[nLen-1] == '/' || Token.szWord[nLen-1] == ',') {
				Token.szWord[nLen-1] = '\0';
				if (nLen - 1 == 0) { // exception 0�̴�
					nSt = i+1;
					continue;
				}
			}

			Token.nStPos = nSt+nPreSt;
			AppendRecord(lpTokenRec, &Token);
			nCountAddToken ++ ;
			nSt = i+1;
		}
	}

	nLen = i - nSt;
	if (nLen != 0 && nLen < MAX_TOKEN_LEN) {
		strncpy(Token.szWord, szLine+nSt, nLen);
                Token.szWord[nLen] = '\0';
                Token.nStPos = nSt+nPreSt;
		if (Token.szWord[nLen-1] == '&' ||Token.szWord[nLen-1] == '.' ||Token.szWord[nLen-1] == '-' ||Token.szWord[nLen-1] == '_' ||Token.szWord[nLen-1] == '/' || Token.szWord[nLen-1] == ',') {
			Token.szWord[nLen-1] = '\0';
			if (nLen - 1 == 0) { // exception 0�̴�
#if defined(_DEBUG)
				printf("sub : %d\n", nCountAddToken);
#endif
				return (nCountAddToken);
			}
		}
                AppendRecord(lpTokenRec, &Token);
		nCountAddToken ++;
	}
	else if (nLen>=MAX_TOKEN_LEN){ // exception
		strncpy(Token.szWord, szLine+nSt, MAX_TOKEN_LEN-1);
		Token.szWord[ MAX_TOKEN_LEN-1] = '\0';
		Token.nStPos = nSt+nPreSt;
		AppendRecord(lpTokenRec, &Token);
		nCountAddToken ++ ;
	}
#if defined(_DEBUG)
	printf("sub : %d\n", nCountAddToken);
#endif
	return (nCountAddToken);
}

tBOOL IsSplitToken(tCHAR nCh);
tBOOL IsSplitToken(tCHAR nCh) 
{
	//tCHAR *szToken = {" \t\n,{}()|!?<>`\"';\\=|[]\0"};
	tCHAR *szTokenCombine = {"~@#$%&-_+:/.;\0"};
	tCHAR *p = szTokenCombine;

//	if (isalnum(nCh)) return FALSE;

	while (*p) {
		if (*p == nCh) return FALSE;
		p++;
	}
	return TRUE;
}


/// ��ū ����..
tINT GetTokenFromLine(tCHAR *szLine, RECORD *lpTokenRec)
{
	tINT i, nSt = 0, nLen;
	TOKEN_FORMAT Token;
	//tCHAR szWord[MAX_WORD_LEN*2];
	tINT nStrLen;
	//tCHAR *p;
	tINT nCountSp = 0;
	tBOOL bSkip2Byte = FALSE;

	
	nStrLen = strlen(szLine);
	for (i = 0; i < (int)nStrLen ; i ++ ) {
		if (szLine[i] & 0x80) {
			if ((tBYTE)(szLine[i]) >= 0xa1 && (tBYTE)(szLine[i]) <= 0xaf) { // Ư������ ����
				if ((tBYTE)(szLine[i]) == 0xa3 && (tBYTE)(szLine[i+1]) >= 0xb0 && (tBYTE)(szLine[i+1]) <= 0xb9) {
					i++;
					continue;
				}
				if ((tBYTE)(szLine[i]) == 0xa3 && (tBYTE)(szLine[i+1]) >= 0xc1 && (tBYTE)(szLine[i+1]) <= 0xda) {
					i++;
					continue;
				}
				if ((tBYTE)(szLine[i]) == 0xa3 && (tBYTE)(szLine[i+1]) >= 0xe1 && (tBYTE)(szLine[i+1]) <= 0xfa) {
					i++;
					continue;
				}
				if ((tBYTE)(szLine[i]) == 0xa4 && (tBYTE)(szLine[i+1]) >= 0xa1 && (tBYTE)(szLine[i+1]) <= 0xfe) {
					i++;
					continue;
				}
#if defined(_DEBUG)
				printf("%d 2byte skip [%c%c]\n", i, szLine[i], szLine[i+1]);
#endif
				bSkip2Byte = TRUE;
				goto Split0;
			}
			i++;
			continue;
		}
		// �׳� �� Ǯ���. by hyypucs 2005.8.17.
		else if (!isalnum(szLine[i]) && IsSplitToken(szLine[i])) {
			bSkip2Byte = FALSE;
Split0:;
			if (nSt == i) {
				nSt ++;
				if (bSkip2Byte) {
					nSt++;
					i++;
				}
				continue;
			}

			
			nLen = i - nSt;
			if (nLen >= MAX_TOKEN_LEN) { // exception
#if !defined(_OLD)
				GetSubTokenFromToken(lpTokenRec, szLine + nSt , nSt, nLen);  // Ŭ ������ SubToken�� ����.
#else
				strncpy(Token.szWord, szLine+nSt, MAX_TOKEN_LEN-1);
				Token.szWord[ MAX_TOKEN_LEN-1] = '\0';
				Token.nStPos = nSt;
				AppendRecord(lpTokenRec, &Token);
#endif

				nSt = i+1;
				nCountSp = 0;
				if (bSkip2Byte) {
					nSt ++;
					i++;
				}
				continue;
			}
#if !defined(_OLD)
			if (nCountSp > 3) { // nCountSp ������ 4�� �̻��̸� ������ �и���Ŵ.
				GetSubTokenFromToken(lpTokenRec, szLine + nSt , nSt, nLen);  // Ŭ ������ SubToken�� ����.
				nSt = i+1;
				nCountSp = 0;
				if (bSkip2Byte) {
					nSt ++;
					i++;
				}
				continue;
			}
#endif

			nCountSp = 0;

			strncpy(Token.szWord, szLine+nSt, nLen);
			Token.szWord[nLen] = '\0';
			// �� ����... �ɺ��� ������ .. &�� .�� ���ܽ��״�. �̰��� �˻��� �� ���� ���� �� ����..
			if (Token.szWord[nLen-1] == '&' ||Token.szWord[nLen-1] == '.' ||Token.szWord[nLen-1] == '-' ||Token.szWord[nLen-1] == '_' ||Token.szWord[nLen-1] == '/' || Token.szWord[nLen-1] == ',') {
				Token.szWord[nLen-1] = '\0';
				if (nLen - 1 == 0) { // exception 0�̴�
					nSt = i+1;
					if (bSkip2Byte) {
						nSt ++;
						i++;
					}
					continue;
				}
			}

			Token.nStPos = nSt;
			AppendRecord(lpTokenRec, &Token);
			nSt = i+1;
			if (bSkip2Byte) {
				nSt ++;
				i++;
			}
		}
		else if (!IsSplitToken(szLine[i])) {
			nCountSp ++ ;
		}
	}

	nLen = i - nSt;
	if (nLen != 0 && nLen < MAX_TOKEN_LEN) {
#if !defined(_OLD)
		if (nCountSp > 3) { // nCountSp ������ 4�� �̻��̸� ������ �и���Ŵ.
			GetSubTokenFromToken(lpTokenRec, szLine + nSt , nSt, nLen);  // Ŭ ������ SubToken�� ����.
			nSt = i+1;
			nCountSp = 0;
		}
		else {
#endif
			strncpy(Token.szWord, szLine+nSt, nLen);
                	Token.szWord[nLen] = '\0';
                	Token.nStPos = nSt;
			if (Token.szWord[nLen-1] == '&' ||Token.szWord[nLen-1] == '.' ||Token.szWord[nLen-1] == '-' ||Token.szWord[nLen-1] == '_' ||Token.szWord[nLen-1] == '/' || Token.szWord[nLen-1] == ',') {
				Token.szWord[nLen-1] = '\0';
				if (nLen - 1 == 0) { // exception 0�̴�
					return (lpTokenRec->nUsed);
				}
			}
                	AppendRecord(lpTokenRec, &Token);
#if !defined(_OLD)
		}
#endif
	}
	else if (nLen>=MAX_TOKEN_LEN){ // exception
#if !defined(_OLD)
		GetSubTokenFromToken(lpTokenRec, szLine + nSt , nSt, nLen); 
#else
		strncpy(Token.szWord, szLine+nSt, MAX_TOKEN_LEN-1);
		Token.szWord[ MAX_TOKEN_LEN-1] = '\0';
		Token.nStPos = nSt;
		AppendRecord(lpTokenRec, &Token);
#endif
	}
	return (lpTokenRec->nUsed);
}


tINT AnalyzeWord(tHCHAR *tmpWord, tINT *WhatWord, tINT *PosWord, tINT *nRetCount)
{
	tINT i, j;
	tINT nCount = 0;
	tINT tmpWhatWord[MAX_COMPWORD];
	tINT tmpPosWord[MAX_COMPWORD];
	tINT ntmpCount = 0;

	for ( i = 0 ; i < HStrlen(tmpWord) ; i ++ ) { /* �ش� �ܾ� �м� */
		if (IsHangul_KS(tmpWord[i])) {
			if (nCount && WhatWord[nCount-1] == H_WORD) continue;
			if (nCount >= MAX_COMPWORD) break; /* exception */
			WhatWord[nCount] = H_WORD;
			PosWord[nCount++] = i;
		}
		else if (IsEnglish(tmpWord[i]) || IsEnglish2_KS(tmpWord[i])) {
			if (nCount && WhatWord[nCount-1] == E_WORD) continue;
			if (nCount >= MAX_COMPWORD) break; /* exception */
			WhatWord[nCount] = E_WORD;
			PosWord[nCount++] = i;
		}
		else if (IsDot_KS(tmpWord[i])) {
			if (nCount && WhatWord[nCount-1] == D_WORD) continue;
			if (nCount >= MAX_COMPWORD) break; /* exception */
			WhatWord[nCount] = D_WORD;
			PosWord[nCount++] = i;
		}
		else if (IsNumber(tmpWord[i]) || IsNumber2_KS(tmpWord[i])) {
			if (nCount && WhatWord[nCount-1] == N_WORD) continue;
			if (nCount >= MAX_COMPWORD) break; /* exception */
			WhatWord[nCount] = N_WORD;
			PosWord[nCount++] = i;
		}
		else if (IsHanja_KS(tmpWord[i])) {
			if (nCount && WhatWord[nCount-1] == HJ_WORD) continue;
			if (nCount >= MAX_COMPWORD) break; /* exception */
			WhatWord[nCount] = HJ_WORD;
			PosWord[nCount++] = i;
		}
		else if (IsSlash_KS(tmpWord[i]) && tmpWord[i+1] && IsEnglish(tmpWord[i+1])) {
			if (nCount && WhatWord[nCount-1] == E_WORD) {
				if (!(i-1 > 0 && IsEnglish(tmpWord[i-2])) && !(tmpWord[i+2] && IsEnglish(tmpWord[i+2]))) {
					continue;
				}
			}
			if (nCount >= MAX_COMPWORD) break; /* exception */
			WhatWord[nCount] = ETC_WORD;
			PosWord[nCount++] = i;
		} 
		else {
			if (nCount && WhatWord[nCount-1] == ETC_WORD) continue;
			if (nCount >= MAX_COMPWORD) break; /* exception */
			WhatWord[nCount] = ETC_WORD;
			PosWord[nCount++] = i;
		}
	}
	PosWord[nCount] = i;
	*nRetCount = nCount;
#if defined(_DEBUG_ME)
	//printf("Count : %d\n", nCount);
#endif

	// F1 : ����+���� �� .. ���� ���ڿ� ���� ������ ����� ������.
	// ����+����+���� => �����
        // �ѱ�+����+�ѱ� => �ѱ۷� �����Ѵ�.
	if (nCount >= 3) {
		for ( i = 0 ; i < nCount ; i ++ ) {
			tmpWhatWord[i] = WhatWord[i];
			tmpPosWord[i] = PosWord[i];
		}
		tmpPosWord[i] = PosWord[i];
		ntmpCount = nCount;
		nCount = 0;
		for ( i = 0 ; i < ntmpCount - 2 ; i ++ ) {
			
			WhatWord[nCount] = tmpWhatWord[i];
			PosWord[nCount++] = tmpPosWord[i];
			if (tmpWhatWord[i] == E_WORD && tmpWhatWord[i+1] == N_WORD && tmpPosWord[i]+1 == tmpPosWord[i+1]) {
				i++;
			}
			else if (tmpWhatWord[i] == E_WORD && tmpWhatWord[i+1] == N_WORD && tmpWhatWord[i+2] == E_WORD) {
				i+=2;
			}
			else if (tmpWhatWord[i] == H_WORD && tmpWhatWord[i+1] == N_WORD && tmpWhatWord[i+2] == H_WORD) {
				i+=2;
			}
		}
		for ( j = i ; j < ntmpCount ; j ++ ) {
			WhatWord[nCount] = tmpWhatWord[j];
			PosWord[nCount++] = tmpPosWord[j];
		}
		PosWord[nCount] = tmpPosWord[j];
		*nRetCount = nCount;
#if defined(_DEBUG_ME)
		//printf("ReCount : %d\n", nCount);
#endif
	}
	if (nCount >= 2) { // f1 �״�� �м��ǵ��� ��
		for ( i = 0 ; i < nCount ; i ++ ) {
			tmpWhatWord[i] = WhatWord[i];
			tmpPosWord[i] = PosWord[i];
		}
		tmpPosWord[i] = PosWord[i];
		ntmpCount = nCount;
		nCount = 0;
		for ( i = 0 ; i < ntmpCount - 1 ; i ++ ) {
			
			WhatWord[nCount] = tmpWhatWord[i];
			PosWord[nCount++] = tmpPosWord[i];
			if (tmpWhatWord[i] == E_WORD && tmpWhatWord[i+1] == N_WORD && tmpPosWord[i]+1 == tmpPosWord[i+1]) {
				i++;
			}
		}
		for ( j = i ; j < ntmpCount ; j ++ ) {
			WhatWord[nCount] = tmpWhatWord[j];
			PosWord[nCount++] = tmpPosWord[j];
		}
		PosWord[nCount] = tmpPosWord[j];
		*nRetCount = nCount;
#if defined(_DEBUG_ME)
		//printf("ReCount : %d\n", nCount);
#endif
	}
	return (nCount);
}

char    *custom_tag_string[]={
        /* provide your own tagset strings here */
        "NN", "NP", "NX", "NU", "VV", "VX", "AJ", "AX",
        "AD", "DT", "DN", "IJ", "JO", "CP", "EM", "EP",
        "PF", "SF", "SV", "SJ", "SN", "SY", "OL", "MF", "ZZ"
};

#define format_tag_string(c)    (custom_tag_string[c])

typedef struct {
        tCHAR szWord[MAX_WORD_LEN*2];
}_SZWORD_FORMAT;

tINT CompSZWord(_SZWORD_FORMAT *item1, _SZWORD_FORMAT *item2);
tVOID DelDashStr(tCHAR *szWord);
tBOOL IsEngDashStr(tCHAR *szWord);

tINT CompSZWord(_SZWORD_FORMAT *item1, _SZWORD_FORMAT *item2)
{
        return (strcmp(item1->szWord, item2->szWord));
}

tBOOL IsEngDashStr(tCHAR *szWord)
{
        tCHAR *p = szWord;
        tBOOL bDash = FALSE;

        while(*p) {
		if (*p & 0x80) return FALSE;
                if(isalpha(*p)) {
                        p++;
                        continue;
                }
                else if (*p == '-') {
                        if (p != szWord && *(p+1) != 0x00) {
                                if (bDash) return FALSE;
                                bDash = TRUE;
                                p++;
                                continue;
                        }
                        else return FALSE;
                }
                else return FALSE;
        }

        return TRUE;
}

tVOID DelDashStr(tCHAR *szWord)
{
        tINT i, nLen = strlen(szWord);

        i = 0;
        while ( i < nLen ) {
                if (szWord[i] == '-') {
                        strcpy((szWord) + i , (szWord) + i + 1);
                        nLen --;
                        continue;
                }
                i++;
        }
}

tBOOL IsOkScan(tCHAR *szStr);

tBOOL IsOkScan(tCHAR *szStr)
{
	tINT i;

	for (i = 0 ; i < (tINT)strlen(szStr) ; i ++ ) {
		if (szStr[i] & 0x80) i++;
		else if (!isalnum(szStr[i])) return FALSE;
	}
	return TRUE;
}

tBOOL ScanForOne(tCHAR *szStr, RECORD *lpScanRecord)
{
	tCHAR  input_buf[512];	
	tCHAR szStr2[256];
	_SCANDAT_Sent   res_sent;
        unsigned short  **pp_word;
        int  n_word;
	 int  i, k;
        _SCANDAT_LemmaList      *p;
	_SCANDAT_Word *p_res_word;
#if defined(_DEBUG_ME)
	//int j;
#endif

	if (strlen(szStr) > 30) { // scan������ ���� �ʰ� �Ѵܾ�� �����̳� ���ո�簡 ���ٰ� ����.
		if (IsEngStr(szStr)) {
			Stem(szStr);
		}
		if (strlen(szStr) < MAX_WORD_LEN*2) {
			AddIWordListOne(lpScanRecord, szStr, ONE_I);
		}
		return TRUE;
	}
        //
        // d-day�� ���ؼ� ddai�� ������ �� �ֵ��� �Ѵ�.
        // exceptionó��
	strcpy(szStr2, szStr);
	if (IsEngDashStr(szStr2)) {
		DelDashStr(szStr2);
		if (IsEngStr(szStr2)) {
			Stem(szStr2);
		}
		if (strlen(szStr2) < MAX_WORD_LEN*2) {
			AddIWordListOne(lpScanRecord, szStr2, ONE_I|DASH_I);
			AddIWordListOne(lpScanRecord, szStr, ONE_I|DASH_I);
			return TRUE;
		}
        }

	if (!IsOkScan(szStr)) {
		AddIWordListOne(lpScanRecord, szStr, ONE_I);
		return TRUE;
	}

	strcpy(input_buf, szStr);
#if defined(_DEBUG_ME)
	//printf("Scan : %s\n", input_buf);
#endif
	LockMutexMan( &MUTEX_SCAN, 0 ); // ���¼Һм��Ⱑ thread safe���� �ʾƼ�.. lock�Ǵ�.

	if (_SCANAPI_open_input_stream(input_buf, (int)strlen(input_buf), '\0') != eOK) {
		UnLockMutexMan(&MUTEX_SCAN, 0);
		return FALSE;
	}


	while (1) {
		if (_SCANAPI_get_sent(&pp_word, &n_word)==eEOF) break;
        	_SCANAPI_analyze(&res_sent, pp_word, n_word);
        	_STAGAPI_tag(&res_sent);

#if defined(_DEBUG_ME)
            //   	_SCANAPI_print_on_console(&res_sent);
	//	printf("print test\n");
#endif
		for ( k = 0 ; k < res_sent.n_word ; k ++ ) {
			p_res_word = &(res_sent.word[k]);
#if defined(_DEBUG_ME)
    	    //		printf("%s\n", p_res_word->str);
#endif
        		for (i=0; i<p_res_word->n_lemma_list; i++) {
                		p = &p_res_word->lemma_list[i];

#if defined(_DEBUG_ME)
            //    		printf("\t[");
             //   		for (j=0; j<p->n_lemma; j++) {
              //          		if (j!=0) printf(" ");
               //         		printf( "%s/%s", p->lemma[j].str, format_tag_string(p->lemma[j].tag));
       //         		}
        //        		printf("]\n");
#endif

				// processing ������.
				if (p->lemma[0].tag == MF || p->lemma[0].tag == OL) continue; // exception

				if (p->n_lemma == 1) { // �м������ �ϳ��� ��� 
					if (FindStopWord(p->lemma[0].str)) { //stopword
						continue;
					}
					if (p->lemma[0].tag >= NN && p->lemma[0].tag <= NU) { // ������ ��
						AddIWordListOne(lpScanRecord, p->lemma[0].str, ONE_I);
					}
					else if (p->lemma[0].tag == AD) { // �λ�
						AddIWordListOne(lpScanRecord, p->lemma[0].str, ONE_AD_I);
					}
				}
				else { // �ƴ� ���
					if (p->lemma[0].tag == VV || p->lemma[0].tag == AJ) { // ����, ������� ���
						if (strlen(p_res_word->str) > 2) ProcVVAJ(lpScanRecord, p, p_res_word->str); // 2�ں��� Ŀ���� vvó���ϵ��� �� -> �� -> �� + �� �̷� ���� �������� ���� �� �̻��ϴ�.
					}
					else if (p->lemma[0].tag == PF || p->lemma[0].tag == DT) { // ���λ�, ������
						ProcPF(lpScanRecord, p, p_res_word->str);
					}
					else if (p->lemma[0].tag >= NN && p->lemma[0].tag <= NU) { // ������ ��
						if (strlen(p->lemma[0].str) <= 2 && strlen(szStr2) <= 4) {
							if (isalnum(p->lemma[0].str[0])) { // ����, ���ڷ� �����ϸ� �־��ش�.
								AddIWordListOne(lpScanRecord, szStr2, ONE_I);
							}
							continue;
						}
						ProcNN(lpScanRecord, p, p_res_word->str);
					}
				}
        		}
		}

	}

	_SCANAPI_close_input_stream();

	UnLockMutexMan(&MUTEX_SCAN, 0); // ���¼Һм��� thread safe�ϰ� �ϱ� ���ؼ�..

	return TRUE;
}

tBOOL IsVVAJ(tCHAR *szStr)
{
	tCHAR  input_buf[512];	
	//tCHAR szStr2[256];
	_SCANDAT_Sent   res_sent;
        unsigned short  **pp_word;
        int  n_word;
	 int  i, k;
        _SCANDAT_LemmaList      *p;
	_SCANDAT_Word *p_res_word;
	tBOOL bRetVal = FALSE;
	int j;

	if (strlen(szStr) > 30 || strchr(szStr, ' ')) { // �׳� �ƴ϶�� return �Ѵ�. �𸣴�
		return FALSE;
	}
	strcpy(input_buf, szStr);
#if defined(_DEBUG_ME)
	printf("Scan : %s\n", input_buf);
#endif
	LockMutexMan( &MUTEX_SCAN, 0 ); // ���¼Һм��Ⱑ thread safe���� �ʾƼ�.. lock�Ǵ�.

	if (_SCANAPI_open_input_stream(input_buf, (int)strlen(input_buf), '\0') != eOK) {
		UnLockMutexMan(&MUTEX_SCAN, 0);
		return FALSE;
	}


	while (1) {
		if (_SCANAPI_get_sent(&pp_word, &n_word)==eEOF) break;
        	_SCANAPI_analyze(&res_sent, pp_word, n_word);
        	_STAGAPI_tag(&res_sent);

#if defined(_DEBUG_ME)
            //   	_SCANAPI_print_on_console(&res_sent);
	//	printf("print test\n");
#endif
		for ( k = 0 ; k < res_sent.n_word ; k ++ ) {
			p_res_word = &(res_sent.word[k]);
#if defined(_DEBUG_ME)
    	    //		printf("%s\n", p_res_word->str);
#endif
        		for (i=0; i<p_res_word->n_lemma_list; i++) {
                		p = &p_res_word->lemma_list[i];

#if defined(_DEBUG_ME)
        //        		printf("\t[");
         //       		for (j=0; j<p->n_lemma; j++) {
          //              		if (j!=0) printf(" ");
           //             		printf( "%s/%s", p->lemma[j].str, format_tag_string(p->lemma[j].tag));
            //    		}
             //   		printf("]\n");
#endif

				// processing ������.
				if (p->lemma[0].tag == MF || p->lemma[0].tag == OL) break;

				if (p->n_lemma >= 1) { // �м������ �ϳ��� ��� 
					if (p->lemma[0].tag == VV || p->lemma[0].tag == AJ) { // ����, ������� ���
						bRetVal = TRUE;
						break;
					}
					if (p->lemma[0].tag == VX || p->lemma[0].tag == AX) { // ����, ������� ���
						bRetVal = TRUE;
						break;
					}
                			for (j=0; j<p->n_lemma; j++) {
						if (p->lemma[j].tag == SV || p->lemma[j].tag == SJ) {
							bRetVal = TRUE;
							break;
						}
                			}
					if (bRetVal) break;
				}
        		}
			if (bRetVal) break;
		}

	}

	_SCANAPI_close_input_stream();

	UnLockMutexMan(&MUTEX_SCAN, 0); // ���¼Һм��� thread safe�ϰ� �ϱ� ���ؼ�..

	return bRetVal;
}

tVOID ProcVVAJ(RECORD *lpScanRecord, _SCANDAT_LemmaList *p, tCHAR *szSrcWord)
{
	tCHAR szWord[MAX_WORD_LEN*2];
	tINT nLen;

	nLen = strlen(p->lemma[0].str);

	if (strlen(szSrcWord) >= 6 && p->n_lemma >= 2 && nLen <= 2 && p->lemma[0].tag == VV && p->lemma[1].tag == EM) {
		// ��ƶ� -> �� + �ƶ�
		// �ѱ� 3�� �̻� �̸鼭, ���� ���� VV EM���� �����Ǹ鼭 
		return ;
	}

	if (strlen(szSrcWord) >= 10 && p->n_lemma >= 3 && nLen <= 2 && p->lemma[1].tag == EM) {
		// �ѱ� 5�� �̻� �̸鼭, ���� ���� VV EM���� �����Ǹ鼭 VV�� ������ ���� ������Ű��.
		return ;
	}

	if (p->n_lemma >= 3 && nLen <= 2 && p->lemma[1].tag == EM && p->lemma[2].tag == CP) {
		// �԰� -> ��/VV ��/EM ��/CP ��/EM
		return ; // �� �ʿ� ������..
	}
	if (p->n_lemma >= 3 && nLen <= 2 && p->lemma[1].tag == EM && p->lemma[2].tag == NN) {
		// ������ 
  		// [��/VV ��/EM/ ��/NN ��/NN]
		return ; // �� �ʿ� ������..
	}

	//��/VV ����/EM Ƽ/VV ��/EM]
	if (p->n_lemma >= 3 && nLen <= 2 && p->lemma[1].tag == EM && p->lemma[2].tag == VV) {
		return ;
	}

	strcpy(szWord, p->lemma[0].str);
	strcat(szWord, "��"); // �⺻�� ������.
	if (FindStopWord(szWord)) { //stopword
		return ;
	}
	AddIWordListOne(lpScanRecord, szWord, ONE_VV_I);
	if (p->lemma[0].tag == AJ) { // ������� ��쿡 ó����
		// ǲǲ�ϴ� ���� ��쿡 ó���Ѵ�.
		nLen = strlen(szWord);
		if (nLen >= 8 && strcmp(szWord+nLen-4, "�ϴ�") == 0) { // �ϴٰ� ������.. �̸� ���� �ϳ� �� �����Ѵ�.
			szWord[nLen-4] = '\0';
			AddIWordListOne(lpScanRecord, szWord, ONE_I);
		}
	}
	else if (p->lemma[0].tag == VV) { // ������ ��쿡 ó����
		// �߹�Ǵ� ���� ��쿡 ó���Ѵ�.
		nLen = strlen(szWord);
		if (nLen >= 8 && strcmp(szWord+nLen-4, "�Ǵ�") == 0) { // �Ǵٰ� ������.. �̸� ���� �ϳ� �� �����Ѵ�.
			szWord[nLen-4] = '\0';
			
			AddIWordListOne(lpScanRecord, szWord, ONE_I);
		}
	}
}


tVOID ProcPF(RECORD *lpScanRecord, _SCANDAT_LemmaList *p, tCHAR *szSrcWord)
{
// ���λ�/������� 
// ���λ�/������ + ��� + ��� + ���̻� ������ �������
// ���� ���+���+���̻�
// ��� + ��� ���� �͵� ���� ���εǵ��� ����.
	IWORDLIST_FORMAT IWordList;
	tCHAR szWord[MAX_WORD_LEN*2];
	tINT i, nSt = 2;
	tINT nLen;
	
	memset(&IWordList, 0, sizeof(IWORDLIST_FORMAT));

	if (p->n_lemma >= 4 && strlen(p->lemma[1].str) <= 2 &&  p->lemma[1].tag >= NN &&  p->lemma[1].tag <= NU && p->lemma[2].tag == CP && p->lemma[3].tag == EM) {
		// PF+���� -> PF + ��/NN ��/CP ��/EM  �ƴ� �� ����?
		return ; // �� �ʿ� ������..
	}
	nLen = strlen(szSrcWord);

	strcpy(szWord, p->lemma[0].str); // ���� �ְ�.. �� ������ ������.
	if (p->lemma[1].tag >= NN && p->lemma[1].tag <= NU) {
		strcat(szWord, p->lemma[1].str); // ���� �ٿ�����..
		if ( p->n_lemma > 2 && (p->lemma[2].tag == SF || p->lemma[2].tag == SN || p->lemma[2].tag == NX)) {
			strcat(szWord, p->lemma[2].str); // ���̻� ���̱�
			nSt ++;
		}
	}
	else return ; // ������ �׳� ������.

	AddIWord(&IWordList, szWord);

	for ( i = nSt ; i < p->n_lemma ; i ++ ) {
		// ����¥�� ��翡 ���� ó���� ����ؾ� �Ѵ�. 
		// üũ �ʿ�
		if (p->lemma[i].tag >= NN && p->lemma[i].tag <= NU) {
			if ( i+1 < p->n_lemma && (p->lemma[i+1].tag == SF || p->lemma[i+1].tag == SN || p->lemma[i+1].tag == NX)) {

				strcpy(szWord, p->lemma[i].str); // ���� �ְ�.. �� ������ ������.
				strcat(szWord, p->lemma[i+1].str); // ���̻� ���̱�
				AddIWord(&IWordList, szWord);
				i++;
			}
			else {
				if (i+1 < p->n_lemma && strlen(p->lemma[i+1].str) <= 2 &&  (p->lemma[i+1].tag == SF || p->lemma[i+1].tag == SN || p->lemma[i+1].tag == NX || p->lemma[i+1].tag == NU || p->lemma[i+1].tag == NN) ) {
					strcpy(szWord, p->lemma[i].str); // ���� �ְ�.. �� ������ ������.
					strcat(szWord, p->lemma[i+1].str); // ���̻� ���̱�
					AddIWord(&IWordList, szWord);
					i++;
				}
				else if (strlen(p->lemma[i].str) <= 2 && i+1 < p->n_lemma && (p->lemma[i+1].tag == SF || p->lemma[i+1].tag == SN || p->lemma[i+1].tag == NX || p->lemma[i+1].tag == NU || p->lemma[i+1].tag == NN)) {
					strcpy(szWord, p->lemma[i].str); // ���� �ְ�.. �� ������ ������.
					strcat(szWord, p->lemma[i+1].str); // ���̻� ���̱�
					AddIWord(&IWordList, szWord);
					i++;
				}
				else {
					if (strlen(p->lemma[i].str) <= 2) {
						if (IWordList.nC_IWord <= 0) {
							AddIWord(&IWordList, p->lemma[i].str);
						}	
						else {
							// ���� �ܾ�� ���̱� �Ѵ�.
							strcat(IWordList.szIWord[IWordList.nC_IWord-1], p->lemma[i].str);
						}
					}
					else {
						AddIWord(&IWordList, p->lemma[i].str);
					}
				}
			}
		}
		else if ( p->lemma[i].tag == SF || p->lemma[i].tag == SN || p->lemma[i].tag == NX) {
			if (IWordList.nC_IWord > 0) {
				strcpy(szWord, IWordList.szIWord[IWordList.nC_IWord-1]);
				strcat(szWord, p->lemma[i].str); // ���̻� ���̱�
				strcpy(IWordList.szIWord[IWordList.nC_IWord-1], szWord);
			}
			else {
				return ; // �̻��ϰ� ���ͼ� ���� �м� ���� ����.
			}
		}
		else if ((p->lemma[i].tag == VV||p->lemma[i].tag==AJ) && nLen <= 8) { // ���� ������ ��� VV�� �����Ǵ� ���� �������� ����.
			return ;
		}
		else break;
	}

	if (IWordList.nC_IWord <= 0) return ; // ����.

	if (IWordList.nC_IWord == 1) {
		AddIWordList(lpScanRecord, IWordList, ONE_I);
	}
	else {
		AddIWordList(lpScanRecord, IWordList, COMPOUND_I);
		ProcCompWord(lpScanRecord, IWordList, szSrcWord);
	}

	// ���� ���� ���λ� ���ְ� ���̻絵 ���� ���� ã����.
	memset(&IWordList, 0, sizeof(IWORDLIST_FORMAT));

	for ( i = 1 ; i < p->n_lemma ; i ++ ) {
		// ����¥�� ��翡 ���� ó���� ����ؾ� �Ѵ�. 
		// üũ �ʿ�
		if (p->lemma[i].tag >= NN && p->lemma[i].tag <= NU) {
			if (strlen(p->lemma[i].str) <= 2 && i+1 < p->n_lemma && (p->lemma[i+1].tag == SF || p->lemma[i+1].tag == SN || p->lemma[i+1].tag == NX || p->lemma[i+1].tag == NU || p->lemma[i+1].tag == NN)) {
				strcpy(szWord, p->lemma[i].str); // ���� �ְ�.. �� ������ ������.
				strcat(szWord, p->lemma[i+1].str); // ���̻� ���̱�
				AddIWord(&IWordList, szWord);
				i++;
			}
			else {
				if (strlen(p->lemma[i].str) <= 2) {
					if (IWordList.nC_IWord <= 0) {
						AddIWord(&IWordList, p->lemma[i].str);
					}	
					else {
						// ���� �ܾ�� ���̱� �Ѵ�.
						strcat(IWordList.szIWord[IWordList.nC_IWord-1], p->lemma[i].str);
					}
				}
				else {
					AddIWord(&IWordList, p->lemma[i].str);
				}
			}
		}
		else break;
	}

	if (IWordList.nC_IWord <= 0) return ; // ����.

	AddIWordList(lpScanRecord, IWordList, COMPOUND_I);
}

tVOID ProcCompWord(RECORD *lpScanRecord, IWORDLIST_FORMAT IWordList, tCHAR *szSrcWord)
{
	// ���� ���� �����ϴϱ� �̸� �̿��ؼ� ���ո�縦 �����.
	// ����� ����� 
	// IWORDLIST�� �ִ� A,B,C,D�� ������
	// AB, ABC, ABCD, CD, BCD �� ������. BC�� ������ �ϳ�?
	// �ΰ������θ� �и�����.
	// ���߿� üũ�غ���.

	IWORDLIST_FORMAT IWordList0;
	tINT i, j;
	tCHAR szWord[MAX_WORD_LEN * 2];

	memset(&IWordList0, 0, sizeof(IWORDLIST_FORMAT));

	if (IWordList.nC_IWord == 2) {
		szWord[0] = '\0';
		for ( j = 0 ; j < IWordList.nC_IWord ; j ++ ) {
			strcat(szWord, IWordList.szIWord[j]);
		}
		AddIWord(&IWordList0, szWord);
		AddIWordList(lpScanRecord, IWordList0, ONE_I);
		return ;
	}

	for ( i = 1 ; i < IWordList.nC_IWord - 1 ; i ++ ) {
		memset(&IWordList0, 0, sizeof(IWORDLIST_FORMAT));
		szWord[0] = '\0';
		for ( j = 0 ; j < i ; j ++ ) {
			strcat(szWord, IWordList.szIWord[j]);
		}
		AddIWord(&IWordList0, szWord);

		szWord[0] = '\0';
		for ( j = i ; j < IWordList.nC_IWord ; j ++ ) {
			strcat(szWord, IWordList.szIWord[j]);
		}
		AddIWord(&IWordList0, szWord);

		AddIWordList(lpScanRecord, IWordList0, COMPOUND_I);
	}
}

tVOID ProcNN(RECORD *lpScanRecord, _SCANDAT_LemmaList *p, tCHAR *szSrcWord)
{
// ��� + ��� + ���̻� ������ �������
// ���� ���+���+���̻�
// ��� + ��� ���� �͵� ���� ���εǵ��� ����.
	IWORDLIST_FORMAT IWordList;
	tCHAR szWord[MAX_WORD_LEN*2];
	tINT i;
	tBOOL fExistSF = FALSE;
	tINT nLen;

	memset(&IWordList, 0, sizeof(IWORDLIST_FORMAT));
	
	// exception ó��..
	// �ȵǴ� ��츦 ���⿡ ����.
	if (p->n_lemma >= 3 && strlen(p->lemma[0].str) <= 2  && p->lemma[1].tag == CP && p->lemma[2].tag == EM) {
		// ���� -> ��/NN ��/CP ��/EM  �ƴ� �� ����?
		return ; // �� �ʿ� ������..
	}
	nLen = strlen(szSrcWord);

//	printf("%s : %d\n", szSrcWord, nLen);

	for ( i = 0 ; i < p->n_lemma ; i ++ ) {
		// ����¥�� ��翡 ���� ó���� ����ؾ� �Ѵ�. 
		// üũ �ʿ�
		if (p->lemma[i].tag >= NN && p->lemma[i].tag <= NU) {
			if ( i+1 < p->n_lemma && (p->lemma[i+1].tag == SF || p->lemma[i+1].tag == SN || p->lemma[i+1].tag == NX)) {
				if (p->lemma[i+1].tag == SF) fExistSF = TRUE;
				strcpy(szWord, p->lemma[i].str); // ���� �ְ�.. �� ������ ������.
				strcat(szWord, p->lemma[i+1].str); // ���̻� ���̱�
				AddIWord(&IWordList, szWord);
				i++;
			}
			else {
				if (i+1 < p->n_lemma && strlen(p->lemma[i+1].str) <= 2 &&  (p->lemma[i+1].tag == SF || p->lemma[i+1].tag == SN || p->lemma[i+1].tag == NX || p->lemma[i+1].tag == NU || p->lemma[i+1].tag == NN) ) {
					strcpy(szWord, p->lemma[i].str); // ���� �ְ�.. �� ������ ������.
					strcat(szWord, p->lemma[i+1].str); // ���̻� ���̱�
					AddIWord(&IWordList, szWord);
					i++;
				}
				else if (strlen(p->lemma[i].str) <= 2 && i+1 < p->n_lemma && (p->lemma[i+1].tag == SF || p->lemma[i+1].tag == SN || p->lemma[i+1].tag == NX || p->lemma[i+1].tag == NU || p->lemma[i+1].tag == NN)) {
					strcpy(szWord, p->lemma[i].str); // ���� �ְ�.. �� ������ ������.
					strcat(szWord, p->lemma[i+1].str); // ���̻� ���̱�
					AddIWord(&IWordList, szWord);
					i++;
				}
				else {
					if (p->n_lemma >= 4) { // exception
                                         // [����/NN(0) ũ/VV(0) ��/EM(0) ��/NN(0) ��ġ��/NN(0)]
					 // �̷� ���� �ƿ� ������ ���� �ʴ� ���� �´�.
						IWordList.nC_IWord = 0;
						return ;
					}
					if (strlen(p->lemma[i].str) <= 2) {
						if (IWordList.nC_IWord <= 0) {
							AddIWord(&IWordList, p->lemma[i].str);
						}	
						else {
							// ���� �ܾ�� ���̱� �Ѵ�.
							strcat(IWordList.szIWord[IWordList.nC_IWord-1], p->lemma[i].str);
						}
					}
					else {
						AddIWord(&IWordList, p->lemma[i].str);
					}
				}
			}
		}
		else if ( p->lemma[i].tag == SF || p->lemma[i].tag == SN || p->lemma[i].tag == NX) {
			if (IWordList.nC_IWord > 0) {
				strcpy(szWord, IWordList.szIWord[IWordList.nC_IWord-1]);
				strcat(szWord, p->lemma[i].str); // ���̻� ���̱�
				strcpy(IWordList.szIWord[IWordList.nC_IWord-1], szWord);
			}
			else {
				return ; // ���� �м��� �ʿ����.
			}
		}
		else if ((p->lemma[i].tag == VV||p->lemma[i].tag==AJ) && nLen <= 6) { // ���� ������ ��� VV�� �����Ǵ� ���� �������� ����.

//			printf("%s[%d] VV�� ���ͼ� skip�ϳ� ��.\n", szSrcWord, nLen);
			return ;
		}
		else break;
	}

	if (IWordList.nC_IWord <= 0) return ; // ����.

	if (IWordList.nC_IWord == 1) {
		AddIWordList(lpScanRecord, IWordList, ONE_I);
	}
	else {
		AddIWordList(lpScanRecord, IWordList, COMPOUND_I);
		ProcCompWord(lpScanRecord, IWordList, szSrcWord);
	}

	if (fExistSF == FALSE) return ; // �Ʒ����� �� �ʿ����. ���̻� ������ �ܾ� ã������ �Ѵ�.

	memset(&IWordList, 0, sizeof(IWORDLIST_FORMAT));

	for ( i = 0 ; i < p->n_lemma ; i ++ ) {
		// ����¥�� ��翡 ���� ó���� ����ؾ� �Ѵ�. 
		// üũ �ʿ�
		if (p->lemma[i].tag >= NN && p->lemma[i].tag <= NU) {
			if (strlen(p->lemma[i].str) <= 2 && i+1 < p->n_lemma && (p->lemma[i+1].tag == SF || p->lemma[i+1].tag == SN || p->lemma[i+1].tag == NX || p->lemma[i+1].tag == NU || p->lemma[i+1].tag == NN)) {
				strcpy(szWord, p->lemma[i].str); // ���� �ְ�.. �� ������ ������.
				strcat(szWord, p->lemma[i+1].str); // ���̻� ���̱�
				AddIWord(&IWordList, szWord);
				i++;
			}
			else {
				if (strlen(p->lemma[i].str) <= 2) {
					if (IWordList.nC_IWord <= 0) {
						AddIWord(&IWordList, p->lemma[i].str);
					}	
					else {
						// ���� �ܾ�� ���̱� �Ѵ�.
						strcat(IWordList.szIWord[IWordList.nC_IWord-1], p->lemma[i].str);
					}
				}
				else {
					if (p->n_lemma >= 4) { // exception
                                         // [����/NN(0) ũ/VV(0) ��/EM(0) ��/NN(0) ��ġ��/NN(0)]
					 // �̷� ���� �ƿ� ������ ���� �ʴ� ���� �´�.
						IWordList.nC_IWord = 0;
						return ;
					}
					AddIWord(&IWordList, p->lemma[i].str);
				}
			}
		}
		else break;
	}

	if (IWordList.nC_IWord <= 0) return ; // ����.

	AddIWordList(lpScanRecord, IWordList, COMPOUND_I);
}
