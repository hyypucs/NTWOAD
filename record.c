/*


		Author : hyypucs
		B-Date : 1999. 9. 16
        Comment:
*/
#define _USE_OPEN
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
#include "record.h"
#if defined(_MEM_CHK)
#include "memchk.h"
#endif

#include "heap.h"

//#include "mpatrol.h"

//#define _DEBUG_MEM_LEAK
// by hyypucs
#if defined(_DEBUG_MEM_LEAK)
tINT nCountMemRec = 0;
tINT nCountMemRec1 = 0;
tVOID PrintCountMemRec(tVOID);

tVOID PrintCountMemRec(tVOID)
{
	printf("CountMemRec : %d\n", nCountMemRec);
	printf("CountMemRec1 : %d\n", nCountMemRec1);
	fflush(stdout);
}
#endif

tBOOL File2Record( RECORD *lpRecord, tCHAR *szFileName, tINT nWidth, tINT nAlloc, tINT nInc, tBOOL bAlloc)
{
#if defined(_USE_OPEN)
	tINT nHandle;

	if ((nHandle = open(szFileName, O_RDONLY)) == -1) {
		if (bAlloc) return (AllocRecord(lpRecord, nWidth, nAlloc, nInc));
		return FALSE;
	}
	
	if (read(nHandle, lpRecord, sizeof(RECORD)) == -1) {
		close(nHandle);
		if (bAlloc) return (AllocRecord(lpRecord, nWidth, nAlloc, nInc));
		return FALSE;
	}
	lpRecord->nAlloc = DwordCvt(lpRecord->nAlloc);
	lpRecord->nUsed = DwordCvt(lpRecord->nUsed);
	lpRecord->nWidth = DwordCvt(lpRecord->nWidth);
	lpRecord->nInc = DwordCvt(lpRecord->nInc);

	lpRecord->lpBuf = (tBYTE *)malloc(lpRecord->nAlloc * lpRecord->nWidth);
	if (lpRecord->lpBuf == NULL) {
		close(nHandle);
		if (bAlloc) return (AllocRecord(lpRecord, nWidth, nAlloc, nInc));
		return FALSE;
	}

	if (lpRecord->nUsed * lpRecord->nWidth > 0) {	
		if (read(nHandle, lpRecord->lpBuf, lpRecord->nUsed * lpRecord->nWidth) == -1) {
			close(nHandle);
			free(lpRecord->lpBuf);
			if (bAlloc) return (AllocRecord(lpRecord, nWidth, nAlloc, nInc));
			return FALSE;
		}
	}

	close(nHandle);
#else
	FILE *Fp;

	if ((Fp = fopen(szFileName, "rb")) == NULL) {
		if (bAlloc) return (AllocRecord(lpRecord, nWidth, nAlloc, nInc));
		return FALSE;
	}
	
	if (fread(lpRecord, sizeof(RECORD), 1, Fp) != 1) {
		fclose(Fp);
		if (bAlloc) return (AllocRecord(lpRecord, nWidth, nAlloc, nInc));
		return FALSE;
	}
	lpRecord->nAlloc = DwordCvt(lpRecord->nAlloc);
	lpRecord->nUsed = DwordCvt(lpRecord->nUsed);
	lpRecord->nWidth = DwordCvt(lpRecord->nWidth);
	lpRecord->nInc = DwordCvt(lpRecord->nInc);

	lpRecord->lpBuf = (tBYTE *)malloc(lpRecord->nAlloc * lpRecord->nWidth);
	if (lpRecord->lpBuf == NULL) {
		fclose(Fp);
		return FALSE;
	}
	 if (lpRecord->nUsed * lpRecord->nWidth > 0) {
		if (fread(lpRecord->lpBuf, lpRecord->nUsed * lpRecord->nWidth, 1, Fp) != 1) {
			fclose(Fp);
			free(lpRecord->lpBuf);
			return FALSE;
		}
	}

	fclose(Fp);
#endif

	return TRUE;	
}


tBOOL Record2File( RECORD *lpRecord, tCHAR *szFileName, tBOOL bFree)
{
#if defined(_USE_OPEN)
	tINT nHandle;

	if ((nHandle = open(szFileName, O_RDWR|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE)) == -1) {
		return FALSE;
	}

	lpRecord->nAlloc = DwordCvt(lpRecord->nAlloc);
	lpRecord->nUsed = DwordCvt(lpRecord->nUsed);
	lpRecord->nWidth = DwordCvt(lpRecord->nWidth);
	lpRecord->nInc = DwordCvt(lpRecord->nInc);

	if (write(nHandle, lpRecord, sizeof(RECORD)) == -1) {
		close(nHandle);
		return FALSE;
	}

	lpRecord->nAlloc = DwordCvt(lpRecord->nAlloc);
	lpRecord->nUsed = DwordCvt(lpRecord->nUsed);
	lpRecord->nWidth = DwordCvt(lpRecord->nWidth);
	lpRecord->nInc = DwordCvt(lpRecord->nInc);

	if (lpRecord->nUsed * lpRecord->nWidth > 0) {
		if (write(nHandle, lpRecord->lpBuf, lpRecord->nUsed * lpRecord->nWidth) == -1) {
			close(nHandle);
			return FALSE;
		}
	}

	if (bFree) FreeRecord(lpRecord);

	close(nHandle);
#else
	FILE *Fp;

	if ((Fp = fopen(szFileName, "wb")) == NULL) {
		return FALSE;
	}

	lpRecord->nAlloc = DwordCvt(lpRecord->nAlloc);
	lpRecord->nUsed = DwordCvt(lpRecord->nUsed);
	lpRecord->nWidth = DwordCvt(lpRecord->nWidth);
	lpRecord->nInc = DwordCvt(lpRecord->nInc);

	if (fwrite(lpRecord, sizeof(RECORD), 1, Fp) != 1) {
		fclose(Fp);
		return FALSE;
	}

	lpRecord->nAlloc = DwordCvt(lpRecord->nAlloc);
	lpRecord->nUsed = DwordCvt(lpRecord->nUsed);
	lpRecord->nWidth = DwordCvt(lpRecord->nWidth);
	lpRecord->nInc = DwordCvt(lpRecord->nInc);

	if (lpRecord->nUsed * lpRecord->nWidth > 0)  {
		if (fwrite(lpRecord->lpBuf, lpRecord->nUsed * lpRecord->nWidth, 1, Fp) != 1) {
			fclose(Fp);
			return FALSE;
		}
	}

	if (bFree) FreeRecord(lpRecord);
	fclose(Fp);
#endif

	return TRUE;
}

tBOOL File2RecordPart( RECORD *lpRecord, tCHAR *szFileName, tINT nReadCount, tINT nWidth, tINT nAlloc, tINT nInc, tBOOL bAlloc)
{
	tINT nHandle;

	if ((nHandle = open(szFileName, O_RDONLY)) == -1) {
		if (bAlloc) return (AllocRecord(lpRecord, nWidth, nAlloc, nInc));
		return FALSE;
	}
	
	read(nHandle, lpRecord, sizeof(RECORD));
	lpRecord->nAlloc = DwordCvt(lpRecord->nAlloc);
	lpRecord->nUsed = DwordCvt(lpRecord->nUsed);
	lpRecord->nWidth = DwordCvt(lpRecord->nWidth);
	lpRecord->nInc = DwordCvt(lpRecord->nInc);

	if (lpRecord->nAlloc > nReadCount) {
		lpRecord->nAlloc = nReadCount;
	}
	if (lpRecord->nUsed > lpRecord->nAlloc) {
		lpRecord->nUsed = lpRecord->nAlloc;
	}

	lpRecord->lpBuf = (tBYTE *)malloc(lpRecord->nAlloc * lpRecord->nWidth);
	if (lpRecord->lpBuf == NULL) {
		close(nHandle);
		return FALSE;
	}

	read(nHandle, lpRecord->lpBuf, lpRecord->nUsed * lpRecord->nWidth);

	close(nHandle);

	return TRUE;	
}

tBOOL Record2FilePart( RECORD *lpRecord, tCHAR *szFileName, tINT nWriteCount, tBOOL bFree)
{
	tINT nHandle;
	tINT nUsed, nAlloc;

	if ((nHandle = open(szFileName, O_RDWR|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE)) == -1) {
		return FALSE;
	}

	nUsed = lpRecord->nUsed;
	nAlloc = lpRecord->nAlloc;

	if (lpRecord->nUsed > nWriteCount) {
		lpRecord->nUsed = nWriteCount;
	}
	lpRecord->nAlloc = lpRecord->nUsed;

	lpRecord->nAlloc = DwordCvt(lpRecord->nAlloc);
	lpRecord->nUsed = DwordCvt(lpRecord->nUsed);
	lpRecord->nWidth = DwordCvt(lpRecord->nWidth);
	lpRecord->nInc = DwordCvt(lpRecord->nInc);

	write(nHandle, lpRecord, sizeof(RECORD));

	lpRecord->nAlloc = DwordCvt(lpRecord->nAlloc);
	lpRecord->nUsed = DwordCvt(lpRecord->nUsed);
	lpRecord->nWidth = DwordCvt(lpRecord->nWidth);
	lpRecord->nInc = DwordCvt(lpRecord->nInc);

	write(nHandle, lpRecord->lpBuf, lpRecord->nUsed * lpRecord->nWidth);

	lpRecord->nUsed = nUsed;
	lpRecord->nAlloc = nAlloc;

	if (bFree) FreeRecord(lpRecord);

	close(nHandle);

	return TRUE;
}

tBOOL AllocRecord(RECORD *lpRecord, tINT nWidth, tINT nAlloc, tINT nInc)
{
	memset(lpRecord, 0, sizeof(RECORD));
	lpRecord->nAlloc = nAlloc;
	lpRecord->nInc = nInc;
	lpRecord->nWidth = nWidth;

#if defined(_MEM_CHK)
	lpRecord->lpBuf = (tBYTE *)DWmalloc(nAlloc * nWidth);
#else
	lpRecord->lpBuf = (tBYTE *)malloc(nAlloc * nWidth);
#endif
	if (lpRecord->lpBuf == NULL) return FALSE;

// by hyypucs
#if defined(_DEBUG_MEM_LEAK)
	nCountMemRec ++ ;
#endif


	return TRUE;
}

tVOID FreeRecord(RECORD *lpRecord)
{

	if (lpRecord->lpBuf) {
#if defined(_MEM_CHK)
		DWfree(lpRecord->lpBuf, lpRecord->nAlloc*lpRecord->nWidth);
#else
		free(lpRecord->lpBuf);
#endif
		memset(lpRecord, 0, sizeof(RECORD));
// by hyypucs
#if defined(_DEBUG_MEM_LEAK)
		nCountMemRec -- ;
#endif
	}

}

tBOOL SetRecord(RECORD *lpRecord, tINT nPos, tVOID *lpRecordItem)
{
	/* used 메모리가 있으면 무조건 setting가능하게 한다. */
	if (lpRecord->lpBuf && nPos >= 0 && nPos < lpRecord->nUsed) {
		memcpy(lpRecord->lpBuf + (nPos * lpRecord->nWidth), lpRecordItem, lpRecord->nWidth);
		return TRUE;
	}
	return FALSE;
}

tBOOL SetRecordEx(RECORD *lpRecord, tINT nPos, tVOID *lpRecordItem)
{
	/* 메모리가 있으면 무조건 setting가능하게 한다. */
	if (lpRecord->lpBuf && nPos >= 0 && nPos < lpRecord->nAlloc) {
		memcpy(lpRecord->lpBuf + (nPos * lpRecord->nWidth), lpRecordItem, lpRecord->nWidth);
		return TRUE;
	}
	return FALSE;
}

tBOOL GetRecord(RECORD *lpRecord, tINT nPos, tVOID *lpRecordItem)
{
	/* Used 메모리가 있으면 무조건 setting가능하게 한다. */
	if (lpRecord->lpBuf && nPos >= 0 && nPos < lpRecord->nUsed) {
		memcpy(lpRecordItem, lpRecord->lpBuf + (nPos * lpRecord->nWidth), lpRecord->nWidth);
		return TRUE;
	}
	return FALSE;
}

tBOOL AppendRecord(RECORD *lpRecord, tVOID *lpRecordItem)
{
	tINT nAlloc;

	if (lpRecord->nAlloc <= lpRecord->nUsed) { /* realloc 받아야 한다. */
		nAlloc = lpRecord->nAlloc + lpRecord->nInc;

#if defined(_MEM_CHK)
		lpRecord->lpBuf = (tBYTE *)DWrealloc(lpRecord->lpBuf, nAlloc * lpRecord->nWidth, lpRecord->nAlloc*lpRecord->nWidth);
#else
		lpRecord->lpBuf = (tBYTE *)realloc(lpRecord->lpBuf, nAlloc * lpRecord->nWidth);
#endif
		if (lpRecord->lpBuf == NULL) return FALSE;
		lpRecord->nAlloc = nAlloc;
	}

	memcpy(lpRecord->lpBuf + lpRecord->nUsed * lpRecord->nWidth, lpRecordItem, lpRecord->nWidth);
	lpRecord->nUsed += 1;
	return TRUE;
}

tBOOL AddRecord(RECORD *lpRecord, tINT nPos, tVOID *lpRecordItem)
{
	tINT nAlloc;


	if (lpRecord->nAlloc <= lpRecord->nUsed) { /* realloc 받아야 한다. */
		nAlloc = lpRecord->nAlloc + lpRecord->nInc;

#if defined(_MEM_CHK)
		lpRecord->lpBuf = (tBYTE *)DWrealloc(lpRecord->lpBuf, nAlloc * lpRecord->nWidth, lpRecord->nAlloc*lpRecord->nWidth);
#else
		lpRecord->lpBuf = (tBYTE *)realloc(lpRecord->lpBuf, nAlloc * lpRecord->nWidth);
#endif
		if (lpRecord->lpBuf == NULL) return FALSE;
		lpRecord->nAlloc = nAlloc;
	}
	if (nPos < 0 || nPos > lpRecord->nUsed) {
		return FALSE; /* used 끝에는 되도록 한다.  */
	}

	if (nPos >= 0 && nPos < lpRecord->nUsed) { /* 안에 있다.. 그럼 밀어내야지.. */
		memmove(lpRecord->lpBuf + (nPos+1) * lpRecord->nWidth, lpRecord->lpBuf + nPos * lpRecord->nWidth, (lpRecord->nUsed - nPos) * lpRecord->nWidth);
	}
	memcpy(lpRecord->lpBuf + nPos * lpRecord->nWidth, lpRecordItem, lpRecord->nWidth);
	lpRecord->nUsed += 1;
	return TRUE;
}

// record 안에 record를 넣는다. 
tBOOL AddRecordInRecord(RECORD *lpRecord, tINT nPos, RECORD *lpSrcRecord)
{
	tINT nAlloc;


	if (lpRecord->nAlloc <= lpRecord->nUsed) { /* realloc 받아야 한다. */
		nAlloc = lpRecord->nAlloc + lpRecord->nInc;

		lpRecord->lpBuf = (tBYTE *)realloc(lpRecord->lpBuf, nAlloc * lpRecord->nWidth);
		if (lpRecord->lpBuf == NULL) return FALSE;
		lpRecord->nAlloc = nAlloc;
	}

	if (lpRecord->nAlloc <= lpRecord->nUsed + lpSrcRecord->nUsed) {
		nAlloc = lpRecord->nAlloc + lpSrcRecord->nUsed;
		lpRecord->lpBuf = (tBYTE *)realloc(lpRecord->lpBuf, nAlloc * lpRecord->nWidth);
		if (lpRecord->lpBuf == NULL) return FALSE;
		lpRecord->nAlloc = nAlloc;

	}
	if (nPos < 0 || nPos > lpRecord->nUsed) {
		return FALSE; /* used 끝에는 되도록 한다.  */
	}

	if (nPos >= 0 && nPos < lpRecord->nUsed) { /* 안에 있다.. 그럼 밀어내야지.. */
		memmove(lpRecord->lpBuf + (nPos+lpSrcRecord->nUsed) * lpRecord->nWidth, lpRecord->lpBuf + nPos * lpRecord->nWidth, (lpRecord->nUsed - nPos) * lpRecord->nWidth);
	}
	memcpy(lpRecord->lpBuf + nPos * lpRecord->nWidth, lpSrcRecord->lpBuf, lpRecord->nWidth*lpSrcRecord->nUsed);
	lpRecord->nUsed += lpSrcRecord->nUsed;
	return TRUE;
}

tBOOL AddRecordSort(RECORD *lpRecord, tVOID *lpRecordItem, CompProcP CompRecord)
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
			return FALSE; // 있다.
		}
		else if (nChk > 0) {
			nLow = nMid + 1;
		}
		else {
			nHigh = nMid - 1;
		}
	}
	nPos = nLow;
	return (AddRecord(lpRecord, nPos, lpRecordItem));
}

tBOOL DelRecord(RECORD *lpRecord, tINT nPos)
{
	if (lpRecord->nUsed <= 0) { // exception
		return TRUE;
	}
	if ( nPos >= 0 && nPos < lpRecord->nUsed) {
		if (nPos < lpRecord->nUsed - 1) {
			memcpy(lpRecord->lpBuf + (nPos * lpRecord->nWidth), lpRecord->lpBuf + ((nPos+1) * lpRecord->nWidth), (lpRecord->nUsed - nPos - 1) * lpRecord->nWidth);
		}
		(lpRecord->nUsed) -= 1;
		return TRUE;
	}
	return FALSE;
}

tINT FindRecord(RECORD *lpRecord, tVOID *lpRecordItem, tBOOL bSort, CompProcP CompRecord)
{
	tINT i, j;
	tBYTE *p = NULL;
	
	if (bSort) {
		if (lpRecord->nUsed) {
			p = (tBYTE *)bsearch(lpRecordItem, (tVOID *)(lpRecord->lpBuf), lpRecord->nUsed, lpRecord->nWidth,  CompRecord);
			if(p) {
				return((p - lpRecord->lpBuf) / lpRecord->nWidth);
			}
		}
		return (-1);
	}

	j = 0;
	for ( i = 0 ; i < lpRecord->nUsed ; i ++ ) {
		if (CompRecord(lpRecordItem, lpRecord->lpBuf + j) == 0) {
			return (i);
		}
		j += lpRecord->nWidth;
	}
	return (-1);
}
		
tVOID SortRecord( RECORD *lpRecord, CompProcP CompRecord )
{
	if (lpRecord->nUsed > 1) {
		qsort(lpRecord->lpBuf, lpRecord->nUsed, lpRecord->nWidth, CompRecord);
	}
}

#if defined(_OLD)
tVOID downHeap(RECORD *lpRecord, int k, int N, tBYTE *newElt, tBYTE *newElt1, tBYTE *newElt2, CompProcP CompRecord);
// nOkPreCount : 앞에 몇개까지 <= nOkPreCount, 되면 return되게 한다.
// nOkPreCount <= 0 이거나 record보다 크면, qSort로 한다.
// 그리고 반드시 heap sort는 asc 소팅으로 한다.
// nOkPreCount만큼은 맨 끝 N-1에서 부터 가장 큰 아이템으로 개수만큼 나온다.
//
tVOID HeapSortRecord( RECORD *lpRecord, tINT nOkPreCount, CompProcP CompRecord)
{

	int i, N;
	tBYTE *temp, *newElt, *newElt1, *newElt2;

	if (lpRecord->nUsed <= 1) { // sort할 것이 없다.
		return ;
	}

	if (nOkPreCount <= 0 || nOkPreCount >= lpRecord->nUsed) {
		qsort(lpRecord->lpBuf, lpRecord->nUsed, lpRecord->nWidth, CompRecord);
		return ;
	}

	N = lpRecord->nUsed;


	temp = (tBYTE *)malloc(lpRecord->nWidth);
	newElt = (tBYTE *)malloc(lpRecord->nWidth);
	newElt1 = (tBYTE *)malloc(lpRecord->nWidth);
	newElt2 = (tBYTE *)malloc(lpRecord->nWidth);
	if (temp==NULL || newElt == NULL || newElt1 == NULL || newElt2 == NULL) {
		if (temp) free(temp);
		if (newElt) free(newElt);
		if (newElt1) free(newElt1);
		if (newElt2) free(newElt2);
		printf("system error malloc error downHeap\n");
		return ;
	}

	for(i = (N/2)-1; i >= 0 ; i-- ) {
		downHeap(lpRecord, i, N, newElt, newElt1, newElt2, CompRecord);
	}

   	for( i = N-1 ; i >= N-nOkPreCount && i>=1 ; i-- ) {
		GetRecord(lpRecord, 0, temp);
		GetRecord(lpRecord, i, newElt);
		SetRecord(lpRecord, 0, newElt);
		SetRecord(lpRecord, i, temp);
		if (nOkPreCount == 1) break;
      		downHeap(lpRecord, 0, i-1, newElt, newElt1, newElt2, CompRecord);
	}

	free(temp);
	free(newElt);
	free(newElt1);
	free(newElt2);
}

tVOID downHeap(RECORD *lpRecord, int k, int N, tBYTE *newElt, tBYTE *newElt1, tBYTE *newElt2, CompProcP CompRecord)
/*  PRE: a[k+1..N] is a heap */
/* POST:  a[k..N]  is a heap */
{ 
	int child;


   	while((k*2) <= N)   /* k has child(s) */ 
	{ 
		if (k*2 == N) {
			child = k*2;
		}
		else {
			child = k*2;
			GetRecord(lpRecord, child, newElt1);
			GetRecord(lpRecord, child+1, newElt2);
			if ( CompRecord(newElt1, newElt2) <= 0) {
         			child += 1;
			}
		}
		GetRecord(lpRecord, k, newElt);
		GetRecord(lpRecord, child, newElt1);
      		if(CompRecord(newElt, newElt1) >= 0) break;
      		/* else */
		SetRecord(lpRecord, k, newElt1); // move child up
		SetRecord(lpRecord, child, newElt);
      		//a[k] = a[child]; /* move child up */
      		k = child;
    	}
	//SetRecord(lpRecord, k, newElt); 
   	//a[k] = newElt;

}/*downHeap*/

#else
tVOID HeapSortRecord( RECORD *lpRecord, tINT nOkPreCount, CompProcP CompRecord)
{
	if (lpRecord->nUsed <= 1) { // sort할 것이 없다.
		return ;
	}

	if (nOkPreCount <= 0 || nOkPreCount >= lpRecord->nUsed) {
		qsort(lpRecord->lpBuf, lpRecord->nUsed, lpRecord->nWidth, CompRecord);
		return ;
	}

	heap_sort(lpRecord->lpBuf, lpRecord->nUsed, nOkPreCount, lpRecord->nWidth, CompRecord);
}
#endif

tBOOL RecordCpyWithAlloc(RECORD *lpDestRecord, RECORD *lpSrcRecord)
{
	tINT nMax;

	memcpy(lpDestRecord, lpSrcRecord, sizeof(RECORD));

	nMax = lpSrcRecord->nAlloc * lpSrcRecord->nWidth;

#if defined(_MEM_CHK)
	if ((lpDestRecord->lpBuf = (tBYTE *)DWmalloc(nMax)) == NULL) {
#else
	if ((lpDestRecord->lpBuf = (tBYTE *)malloc(nMax)) == NULL) {
#endif
		return FALSE;
	}
        memcpy(lpDestRecord->lpBuf, lpSrcRecord->lpBuf, nMax);

// by hyypucs
#if defined(_DEBUG_MEM_LEAK)
        nCountMemRec ++ ;
#endif

        return TRUE;
}

tBOOL ReverseRecord(RECORD *lpRecord)
{
	tBYTE *lpBuf;
	tBYTE *p = lpRecord->lpBuf;
	tBYTE *q = lpRecord->lpBuf + (lpRecord->nWidth * (lpRecord->nUsed-1));

	lpBuf = (tBYTE *)malloc(lpRecord->nWidth);
	if (lpBuf == NULL) return FALSE;

	while (p < q) {
		memcpy(lpBuf, p, lpRecord->nWidth);
		memcpy(p, q, lpRecord->nWidth);
		memcpy(q, lpBuf, lpRecord->nWidth);
		p += lpRecord->nWidth;
		q -= lpRecord->nWidth;
	}

	free(lpBuf);
	return TRUE;
}

tBOOL ItemFile2Record( RECORD *lpRecord, tCHAR *szFileName, tINT nWidth, tINT nAlloc, tINT nInc, tBOOL bAlloc)
{
	FILE *fp;
	tBYTE *lpItem = NULL;

	if( (fp = fopen(szFileName, "r")) == NULL) {
		return( AllocRecord(lpRecord, nWidth, nAlloc, nInc ) );
	}

	lpRecord->nUsed = DwordCvt(0);
	lpRecord->nWidth = DwordCvt(nWidth);
	lpRecord->nAlloc = DwordCvt(nAlloc);
  lpRecord->nInc = DwordCvt(nInc);
	lpRecord->lpBuf = (tBYTE *)malloc(nAlloc * nWidth);

	lpItem = (tBYTE *)malloc(nWidth);

	while( fread(lpItem, nWidth, 1, fp) != 0) {
		if (lpRecord->nAlloc <= lpRecord->nUsed) { 
			nAlloc = lpRecord->nAlloc + lpRecord->nInc;
			lpRecord->lpBuf = (tBYTE *)realloc(lpRecord->lpBuf, nAlloc * lpRecord->nWidth);
			if (lpRecord->lpBuf == NULL) return FALSE;
			lpRecord->nAlloc = nAlloc;
		}

		memcpy(lpRecord->lpBuf + lpRecord->nUsed * lpRecord->nWidth, lpItem, lpRecord->nWidth);
		lpRecord->nUsed += 1;
	}
	
	fclose(fp);

	free(lpItem);
	
	return TRUE;
}
	
tBOOL Record2ItemFile( RECORD *lpRecord, tCHAR *szFileName)
{
	FILE *fp;

	if( (fp = fopen(szFileName, "w")) == NULL) {
		return FALSE;
	}

	if (lpRecord->nUsed * lpRecord->nWidth > 0) {
		if (fwrite(lpRecord->lpBuf, lpRecord->nUsed * lpRecord->nWidth, 1, fp) <= 0) {
			fclose(fp);
			return FALSE;
		}
	}

	fclose(fp);

	return TRUE;
}

tBOOL Item2ItemFile(tBYTE *lpItem, tCHAR *szFileName, tINT nWidth)
{
	FILE *fp;

	if( (fp = fopen(szFileName, "a")) == NULL) {
		return FALSE;
	}

	if( fwrite(lpItem, nWidth, 1, fp) <= 0) {
		return FALSE;
	}

	fclose(fp);

	return TRUE;
}


