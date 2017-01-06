#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "dwtype.h"
#include "hstrlib.h"
#include "record.h"
#include "hcorrect.h"


typedef union {
        struct {
           tBYTE second;     /* second hangul */
           tBYTE first;
        } ch;
        struct {
           tWORD end : 5;
           tWORD mid : 5;
           tWORD st  : 5;
           tWORD tag : 1;   
        } bit;
        tWORD data;
} HAN_FORMAT;


#define MAX_CH 6
typedef struct {
	tWORD ch;
	tWORD chkch[MAX_CH];
//[MAX_CH];
}CHECK_CH;


#define MAX_FIRSTCH  19
CHECK_CH FirstCH[MAX_FIRSTCH] = {
  { 2, {3,11, 5, 7, 0}},
  { 3, {2, 6, 12, 5, 0}},
  { 4, {8, 13, 5, 18, 0}},
  { 5, {6, 14, 2, 13, 0}},
  { 6, {15, 5, 3, 13, 0}},
  { 7, {2, 13, 20, 19, 0}},
  { 8, {4, 9, 14, 17, 14,0}},
  { 9, {10, 14, 4, 0}},
  { 10, {15, 9, 0}},
  { 11, {12, 2, 7, 20, 0}},
  { 12, {11, 20, 7, 3, 0}},
  { 13, {5, 4, 7, 16, 14,0}},
  { 14, {15, 9, 5, 4, 8, 0}},
  { 15, {10, 14, 6, 5, 0}},
  { 16, {18, 19, 13, 0}},
  { 17, {18, 8, 4, 0}},
  { 18, {17, 16, 4, 0}},
  { 19, {16, 7, 13, 0}},
  { 20, {7, 19,13, 0}}
};

#define MAX_MIDCH  13
CHECK_CH MidCH[MAX_MIDCH] = {
  { 3, {7, 29, 5, 0}},
  { 4, {6, 5, 10, 12, 0}},
  { 5, {11, 4, 6, 0}},
  { 6, {4, 12, 10, 0}},
  { 7, {13, 3, 11, 0}},
  { 10, {12, 4, 6, 0}},
  { 11, {19, 5, 7, 0}},
  { 12, {10,  6, 4, 0}},
  { 13, {7, 19, 20, 0}},
  { 19, {11, 13, 0}},
  { 20, {13, 0}},
  { 27, {20, 0}},
  { 29, {3, 0}}
};

#define MAX_LASTCH  18
CHECK_CH LastCH[MAX_LASTCH] = {
  { 2, {5, 9, 0}},
  { 5, {17, 23, 0}},
  { 8, {5, 9, 17, 0}},
  { 9, {5, 23, 0}},
  { 10, {2, 23, 0}},
  { 11, {17, 23, 0}},
  { 12, {19, 17, 0}},
  { 17, {19, 23, 0}},
  { 19, {17, 23, 0}},
  { 21, {23, 17, 0}},
  { 22, {21, 23, 17, 0}},
  { 23, {17, 19, 0}},
  { 24, {23, 25, 0}},
  { 25, {24, 23, 0}},
  { 26, {2, 23, 0}},
  { 27, {8, 23, 17, 0}},
  { 28, {17, 23, 0}},
  { 29, {17, 23, 0}}
};

#define CHK_FIRST 0
#define CHK_MID   1
#define CHK_LAST  2


PRIVATE tINT Comp_resWord(CORRECT_FORMAT *item1, CORRECT_FORMAT *item2);
PRIVATE tVOID InsertWord(RECORD *lpresWordRecord, tHCHAR *uszWord);
PRIVATE tVOID ReplaceChar(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic, tINT nOpt);

PRIVATE tINT Comp_resWord(CORRECT_FORMAT *item1, CORRECT_FORMAT *item2)
{
	return (HStricmp(item1->resWord, item2->resWord));
}

PRIVATE tVOID InsertWord(RECORD *lpresWordRecord, tHCHAR *uszWord)
{
	CORRECT_FORMAT Record;
	tINT nPos;

	if (HStrlen(uszWord) >= MAX_WORD_LEN) return; // exception

	HStrcpy(Record.resWord, uszWord);
	nPos = FindRecord(lpresWordRecord, &Record, TRUE, (CompProcP)Comp_resWord);

	if (nPos == -1) {
		AddRecordSort(lpresWordRecord, &Record, (CompProcP)Comp_resWord);
	}
} 

PRIVATE tVOID ReplaceChar(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic, tINT nOpt)
{
	HAN_FORMAT han;
	tINT i, j;
	tHCHAR tmpWord[MAX_WORD_LEN * 2];
	tHCHAR *p;
	tINT nMaxCh;
	CHECK_CH *lpChkCh;
	tINT nLen = HStrlen(uszWord);

	if (nOpt == CHK_FIRST) {
		nMaxCh = MAX_FIRSTCH;
		lpChkCh = FirstCH;
	}
	else if (nOpt == CHK_FIRST) {
		nMaxCh = MAX_MIDCH;
		lpChkCh = MidCH;
	}
	else {
		nMaxCh = MAX_LASTCH;
		lpChkCh = LastCH;
	}

	for ( i = 0 ; i < nLen ; i ++ ) {
		if (lpresWordRecord->nUsed >= nMax) return ;
		if (!IsHangul_KSSM(uszWord[i])) continue;
		han.data = uszWord[i];
		for ( j = 0 ; j < nMaxCh ; j ++ ) {
			if (lpresWordRecord->nUsed >= nMax) return ;
			if (nOpt == CHK_FIRST) {
				if (han.bit.st == lpChkCh[j].ch) {
					p = lpChkCh[j].chkch;
					HStrcpy(tmpWord, uszWord);
					while (*p) {
						if (lpresWordRecord->nUsed >= nMax) return ;
						han.bit.st = *p;
						tmpWord[i] = han.data;
						if (CheckDic(tmpWord, NULL)) {
							InsertWord(lpresWordRecord, tmpWord);
						}
						p++;
					}
					break;
				}
			}
			else if (nOpt == CHK_MID) {
				if (han.bit.mid == lpChkCh[j].ch) {
					p = lpChkCh[j].chkch;
					HStrcpy(tmpWord, uszWord);
					while (*p) {
						if (lpresWordRecord->nUsed >= nMax) return ;
						han.bit.mid = *p;
						tmpWord[i] = han.data;
						if (CheckDic(tmpWord, NULL)) {
							InsertWord(lpresWordRecord, tmpWord);
						}
						p++;
					}
					break;
				}
			}
			else {
				if (han.bit.end == lpChkCh[j].ch) {
					p = lpChkCh[j].chkch;
					HStrcpy(tmpWord, uszWord);
					while (*p) {
						if (lpresWordRecord->nUsed >= nMax) return ;
						han.bit.end = *p;
						tmpWord[i] = han.data;
						if (CheckDic(tmpWord, NULL)) {
							InsertWord(lpresWordRecord, tmpWord);
						}
						p++;
					}
					break;
				}
			}
		}
	}
}

tBOOL HanCorrect(tHCHAR *uszWord, RECORD *lpresWordRecord, tINT nMax, IsInDicProc CheckDic)
{
	if (HStrlen(uszWord) <= 1 || HStrlen(uszWord)>=MAX_WORD_LEN) return FALSE;
	ReplaceChar(uszWord, lpresWordRecord, nMax, (IsInDicProc)CheckDic, CHK_FIRST);
	if (lpresWordRecord->nUsed >= nMax) return TRUE;
	ReplaceChar(uszWord, lpresWordRecord, nMax, (IsInDicProc)CheckDic, CHK_MID);
	if (lpresWordRecord->nUsed >= nMax) return TRUE;
	ReplaceChar(uszWord, lpresWordRecord, nMax, (IsInDicProc)CheckDic, CHK_LAST);
	if (lpresWordRecord->nUsed > 0) return TRUE;

	return FALSE;
}


