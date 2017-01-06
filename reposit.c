/****************************************************************
 * Description : 기존 record 관련 프로그램에서 RECORD구조체에 KEY값을
               입력할수 있게 하였다. RECORD search, sorting 이 가능해진다.
 * Author : Information Technology Team, ytlee
 * Date : June 30, 2004
 ****************************************************************/
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
#include "reposit.h"

// by hyypucs
#if defined(_DEBUG_MEM_LEAK)
tINT nCountMemRec = 0;
tVOID PrintCountMemRec(tVOID);

tVOID PrintCountMemRec(tVOID)
{
	printf("CountMemRec : %d\n", nCountMemRec);
}
#endif

/*========================================================
	File2Reposit
	Description : 
	=======================================================*/
tBOOL File2Reposit( REPOSIT *lpReposit, tCHAR *szFileName, tINT nWidth, tINT nAlloc, tINT nInc, tBOOL bAlloc)
{
	tINT nHandle;
	tINT i;
	REPOSIT RepositItem;

	if ((nHandle = open(szFileName, O_RDONLY)) == -1) {
		if (bAlloc) return (AllocReposit(lpReposit, nWidth, nAlloc, nInc));
		return FALSE;
	}
	
	if (read(nHandle, lpReposit, sizeof(REPOSIT)) == -1) {
		close(nHandle);
		if (bAlloc) return (AllocReposit(lpReposit, nWidth, nAlloc, nInc));
		return FALSE;
	}

	lpReposit->nAlloc = DwordCvt(lpReposit->nAlloc);
	lpReposit->nUsed = DwordCvt(lpReposit->nUsed);
	lpReposit->nWidth = DwordCvt(lpReposit->nWidth);
	lpReposit->nInc = DwordCvt(lpReposit->nInc);

	lpReposit->lpBuf = (tBYTE *)malloc(lpReposit->nAlloc * lpReposit->nWidth);
	if (lpReposit->lpBuf == NULL) {
		close(nHandle);
		if (bAlloc) return (AllocReposit(lpReposit, nWidth, nAlloc, nInc));
		return FALSE;
	}

	if (lpReposit->nUsed * lpReposit->nWidth > 0) {	
		if (read(nHandle, lpReposit->lpBuf, lpReposit->nUsed * lpReposit->nWidth) == -1) {
			close(nHandle);
			free(lpReposit->lpBuf);
			if (bAlloc) return (AllocReposit(lpReposit, nWidth, nAlloc, nInc));
			return FALSE;
		}
	}

	for(i=0; i<lpReposit->nUsed; i++) {
		GetReposit(lpReposit, i, &RepositItem);

		RepositItem.nAlloc = DwordCvt(RepositItem.nAlloc);
		RepositItem.nUsed = DwordCvt(RepositItem.nUsed);
		RepositItem.nWidth = DwordCvt(RepositItem.nWidth);
		RepositItem.nInc = DwordCvt(RepositItem.nInc);

		RepositItem.lpBuf = (tBYTE *)malloc(RepositItem.nAlloc * RepositItem.nWidth);
		if (RepositItem.lpBuf == NULL) {
			close(nHandle);
			if (bAlloc) return (AllocReposit(lpReposit, nWidth, nAlloc, nInc));
			return FALSE;
		}

		if (RepositItem.nUsed * RepositItem.nWidth > 0) {	
			if (read(nHandle, RepositItem.lpBuf, RepositItem.nUsed * RepositItem.nWidth) == -1) {
				close(nHandle);
				free(RepositItem.lpBuf);
				if (bAlloc) return (AllocReposit(lpReposit, nWidth, nAlloc, nInc));
				return FALSE;
			}
		}

		SetReposit(lpReposit, i, &RepositItem);
	}

	close(nHandle);

	return TRUE;	
}

/*========================================================
	File2RepositCorrespondKey
	Description : 
	=======================================================*/
tBOOL File2RepositCorrespondKey(REPOSIT *lprpRepositHeader, tCHAR *szKey, tCHAR *szFileName, tINT nReadCount)
{
	tINT nHandle;
	tINT i;
	REPOSIT rpRepositHeader;
	REPOSIT rpRepositList;
	tINT nItemOffSet = 0;
	//tINT nReadCount;
	//tINT j;
  //NEWDOCID_FORMAT DocId;
	//tINT nRetRead;

	//printf("szFileName=>[%s]\n", szFileName);
	if ((nHandle = open(szFileName, O_RDONLY)) == -1) {
		//printf("%s, %d\n", __FILE__, __LINE__);
		return FALSE;
	}

	if (read(nHandle, &rpRepositHeader, sizeof(REPOSIT)) == -1) {
		close(nHandle);
		//printf("%s, %d\n", __FILE__, __LINE__);
		return FALSE;
	}
	
	rpRepositHeader.nAlloc = DwordCvt(rpRepositHeader.nAlloc);
	rpRepositHeader.nUsed = DwordCvt(rpRepositHeader.nUsed);
	rpRepositHeader.nWidth = DwordCvt(rpRepositHeader.nWidth);
	rpRepositHeader.nInc = DwordCvt(rpRepositHeader.nInc);

	rpRepositHeader.lpBuf = (tBYTE *)malloc(rpRepositHeader.nAlloc * rpRepositHeader.nWidth);
	if (rpRepositHeader.lpBuf == NULL) {
		close(nHandle);
		//printf("%s, %d\n", __FILE__, __LINE__);
		return FALSE;
	}

	if (rpRepositHeader.nUsed * rpRepositHeader.nWidth > 0) {
		if (read(nHandle, rpRepositHeader.lpBuf, rpRepositHeader.nUsed * rpRepositHeader.nWidth) == -1) {
			close(nHandle);
			free(rpRepositHeader.lpBuf);
			//printf("%s, %d\n", __FILE__, __LINE__);
			return FALSE;
		}
	}

	for(i=0; i<rpRepositHeader.nUsed; i++) {
		GetReposit(&rpRepositHeader, i, &rpRepositList);

		rpRepositList.nAlloc = DwordCvt(rpRepositList.nAlloc);
		rpRepositList.nUsed = DwordCvt(rpRepositList.nUsed);
		rpRepositList.nWidth = DwordCvt(rpRepositList.nWidth);
		rpRepositList.nInc = DwordCvt(rpRepositList.nInc);

		/*
		printf("[%s]:[%s]\n", rpRepositList.szKey, szKey);
		printf("File2RepositCorrespondKey::nItemOffSet=>[%d]\n", nItemOffSet);
		printf("File2RepositCorrespondKey::rpRepositList.nAlloc=>[%d]\n", rpRepositList.nAlloc);
		printf("File2RepositCorrespondKey::rpRepositList.nUsed=>[%d]\n", rpRepositList.nUsed);
		printf("File2RepositCorrespondKey::rpRepositList.nWidth=>[%d]\n", rpRepositList.nWidth);
		*/

		//printf("[%s][%s]\n", rpRepositList.szKey, szKey);
		if(strcmp(rpRepositList.szKey, szKey) != 0) {
			nItemOffSet	+= rpRepositList.nUsed;
		}
		else {
			strcpy(lprpRepositHeader->szKey, rpRepositList.szKey);
			lprpRepositHeader->nAlloc = rpRepositList.nAlloc;
			lprpRepositHeader->nUsed = rpRepositList.nUsed;
			lprpRepositHeader->nWidth = rpRepositList.nWidth;
			lprpRepositHeader->nInc = rpRepositList.nInc;
			lprpRepositHeader->lpBuf = (tBYTE *)malloc(rpRepositList.nAlloc * rpRepositList.nWidth);
			if (lprpRepositHeader->lpBuf == NULL) {
				close(nHandle);
				free(rpRepositHeader.lpBuf);
				//printf("%s, %d\n", __FILE__, __LINE__);
				return FALSE;
			}

			lseek(nHandle, rpRepositList.nWidth*nItemOffSet, SEEK_CUR);

			if(nReadCount == -1) {
				nReadCount = rpRepositList.nUsed;
			}
			else {
				if( rpRepositList.nUsed < nReadCount ) {
					nReadCount = rpRepositList.nUsed;
				}
			}

			if (read(nHandle, lprpRepositHeader->lpBuf, nReadCount * rpRepositList.nWidth) == -1){
				close(nHandle);
				free(rpRepositHeader.lpBuf);
				free(lprpRepositHeader->lpBuf);
				//printf("%s, %d\n", __FILE__, __LINE__);
				return FALSE;
			}

			close(nHandle);
			free(rpRepositHeader.lpBuf);

			/*
			printf("lprpRepositHeader->nUsed=>[%d]\n", lprpRepositHeader->nUsed);
			for(j=0; j<rpRepositHeader.nUsed; j++) {
				GetReposit(&rpRepositHeader, j, &DocId);
				printf("\t[%s][%d][%d]\n", DocId.szWord, DocId.unDocId, DocId.nWeight);
			}
			*/

			//printf("%s, %d\n", __FILE__, __LINE__);
			return TRUE;
		}
	}

	free(rpRepositHeader.lpBuf);

	close(nHandle);
	//printf("%s, %d\n", __FILE__, __LINE__);

	return FALSE;
}

/*========================================================
	Reposit2File
	Description : 
	=======================================================*/
tBOOL Reposit2File( REPOSIT *lpReposit, tCHAR *szFileName, tBOOL bFree)
{
	tINT nHandle;
	tINT i;
	REPOSIT RepositItem;

	if ((nHandle = open(szFileName, O_RDWR|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE)) == -1) {
		return FALSE;
	}

	lpReposit->nAlloc = DwordCvt(lpReposit->nAlloc);
	lpReposit->nUsed = DwordCvt(lpReposit->nUsed);
	lpReposit->nWidth = DwordCvt(lpReposit->nWidth);
	lpReposit->nInc = DwordCvt(lpReposit->nInc);

	if (write(nHandle, lpReposit, sizeof(REPOSIT)) == -1) {
		close(nHandle);
		return FALSE;
	}

	lpReposit->nAlloc = DwordCvt(lpReposit->nAlloc);
	lpReposit->nUsed = DwordCvt(lpReposit->nUsed);
	lpReposit->nWidth = DwordCvt(lpReposit->nWidth);
	lpReposit->nInc = DwordCvt(lpReposit->nInc);

	if (lpReposit->nUsed * lpReposit->nWidth > 0) {
		if (write(nHandle, lpReposit->lpBuf, lpReposit->nUsed * lpReposit->nWidth) == -1) {
			close(nHandle);
			return FALSE;
		}
	}

	for(i=0; i<lpReposit->nUsed; i++) {
		GetReposit(lpReposit, i, &RepositItem);

		RepositItem.nAlloc = DwordCvt(RepositItem.nAlloc);
		RepositItem.nUsed = DwordCvt(RepositItem.nUsed);
		RepositItem.nWidth = DwordCvt(RepositItem.nWidth);
		RepositItem.nInc = DwordCvt(RepositItem.nInc);

		if (RepositItem.nUsed * RepositItem.nWidth > 0) {
			if (write(nHandle, RepositItem.lpBuf, RepositItem.nUsed * RepositItem.nWidth) == -1) {
				close(nHandle);
				return FALSE;
			}
		}
	}

	if (bFree) {
		for(i=0; i<lpReposit->nUsed; i++) {
			GetReposit(lpReposit, i, &RepositItem);
			FreeReposit(&RepositItem);
		}
		FreeReposit(lpReposit);
	}

	close(nHandle);

	return TRUE;
}

/*========================================================
	AllocReposit
	Description : 
	=======================================================*/
tBOOL AllocReposit(REPOSIT *lpReposit, tINT nWidth, tINT nAlloc, tINT nInc)
{
	memset(lpReposit, 0, sizeof(REPOSIT));
	lpReposit->nAlloc = nAlloc;
	lpReposit->nInc = nInc;
	lpReposit->nWidth = nWidth;
	lpReposit->lpBuf = (tBYTE *)malloc(nAlloc * nWidth);
	if (lpReposit->lpBuf == NULL) {
		return FALSE;
	}


// by hyypucs
#if defined(_DEBUG_MEM_LEAK)
	nCountMemRec ++ ;
#endif


	return TRUE;
}

/*========================================================
	FreeReposit
	Description : 
	=======================================================*/
tVOID FreeReposit(REPOSIT *lpReposit)
{

	if (lpReposit->lpBuf) {
		free(lpReposit->lpBuf);
		memset(lpReposit, 0, sizeof(REPOSIT));
// by hyypucs
#if defined(_DEBUG_MEM_LEAK)
		nCountMemRec -- ;
#endif
	}

}

/*========================================================
	FreeRepositHeader
	Description : 
	=======================================================*/
tVOID FreeRepositHeader(REPOSIT *lpReposit)
{
	tINT i;
	REPOSIT rpMKIFList;
	
	for(i=0; i<lpReposit->nUsed; i++) {
		GetReposit(lpReposit, i, &rpMKIFList);
		FreeReposit(&rpMKIFList);
	}
	FreeReposit(lpReposit);
}

/*========================================================
	SetReposit
	Description : 
	=======================================================*/
tBOOL SetReposit(REPOSIT *lpReposit, tINT nPos, tVOID *lpRepositItem)
{
	/* used 메모리가 있으면 무조건 setting가능하게 한다. */
	if (lpReposit->lpBuf && nPos >= 0 && nPos < lpReposit->nUsed) {
		memcpy(lpReposit->lpBuf + (nPos * lpReposit->nWidth ), lpRepositItem, lpReposit->nWidth);
		return TRUE;
	}
	return FALSE;
}

/*========================================================
	GetReposit
	Description : 
	=======================================================*/
tBOOL GetReposit(REPOSIT *lpReposit, tINT nPos, tVOID *lpRepositItem)
{
	/* Used 메모리가 있으면 무조건 setting가능하게 한다. */
	if (lpReposit->lpBuf && nPos >= 0 && nPos < lpReposit->nUsed) {
		memcpy(lpRepositItem, lpReposit->lpBuf + (nPos * lpReposit->nWidth), lpReposit->nWidth);
		return TRUE;
	}
	return FALSE;
}

/*========================================================
	AppendReposit
	Description : 
	=======================================================*/
tBOOL AppendReposit(REPOSIT *lpReposit, tVOID *lpRepositItem)
{
	tINT nAlloc;

	if (lpReposit->nAlloc <= lpReposit->nUsed) { /* realloc 받아야 한다. */
		nAlloc = lpReposit->nAlloc + lpReposit->nInc;

		lpReposit->lpBuf = (tBYTE *)realloc(lpReposit->lpBuf, nAlloc * lpReposit->nWidth);
		if (lpReposit->lpBuf == NULL) return FALSE;
		lpReposit->nAlloc = nAlloc;
	}

	memcpy(lpReposit->lpBuf + lpReposit->nUsed * lpReposit->nWidth, lpRepositItem, lpReposit->nWidth);
	lpReposit->nUsed += 1;
	return TRUE;
}

/*========================================================
	AddReposit
	Description : 
	=======================================================*/
tBOOL AddReposit(REPOSIT *lpReposit, tINT nPos, tVOID *lpRepositItem)
{
	tINT nAlloc;

	if (lpReposit->nAlloc <= lpReposit->nUsed) { /* realloc 받아야 한다. */
		nAlloc = lpReposit->nAlloc + lpReposit->nInc;

		lpReposit->lpBuf = (tBYTE *)realloc(lpReposit->lpBuf, nAlloc * lpReposit->nWidth);
		if (lpReposit->lpBuf == NULL) return FALSE;
		lpReposit->nAlloc = nAlloc;
	}
	if (nPos < 0 || nPos > lpReposit->nUsed) {
		return FALSE; /* used 끝에는 되도록 한다.  */
	}

	if (nPos >= 0 && nPos < lpReposit->nUsed) { /* 안에 있다.. 그럼 밀어내야지.. */
		memmove(lpReposit->lpBuf + (nPos+1) * lpReposit->nWidth, lpReposit->lpBuf + nPos * lpReposit->nWidth, (lpReposit->nUsed - nPos) * lpReposit->nWidth);
	}
	memcpy(lpReposit->lpBuf + nPos * lpReposit->nWidth, lpRepositItem, lpReposit->nWidth);
	lpReposit->nUsed += 1;
	return TRUE;
}

/*========================================================
	DelReposit
	Description : 
	=======================================================*/
tBOOL DelReposit(REPOSIT *lpReposit, tINT nPos)
{
	if (lpReposit->nUsed <= 0) { // exception
		return TRUE;
	}
	if ( nPos >= 0 && nPos < lpReposit->nUsed) {
		if (nPos < lpReposit->nUsed - 1) {
			memcpy(lpReposit->lpBuf + (nPos * lpReposit->nWidth), lpReposit->lpBuf + ((nPos+1) * lpReposit->nWidth), (lpReposit->nUsed - nPos - 1) * lpReposit->nWidth);
		}
		(lpReposit->nUsed) -= 1;
		return TRUE;
	}
	return FALSE;
}

/*========================================================
	FindReposit
	Description : 
	=======================================================*/
tINT FindReposit(REPOSIT *lpReposit, tVOID *lpRepositItem, tBOOL bSort, CompProcP CompReposit)
{
	tINT i, j;
	tBYTE *p = NULL;
	
	if (bSort) {
		if (lpReposit->nUsed) {
			p = (tBYTE *)bsearch(lpRepositItem, (tVOID *)(lpReposit->lpBuf), lpReposit->nUsed, lpReposit->nWidth,  CompReposit);
			if(p) {
				return((p - lpReposit->lpBuf) / lpReposit->nWidth);
			}
		}
		return (-1);
	}

	j = 0;
	for ( i = 0 ; i < lpReposit->nUsed ; i ++ ) {
		if (CompReposit(lpRepositItem, lpReposit->lpBuf + j) == 0) {
			return (i);
		}
		j += lpReposit->nWidth;
	}
	return (-1);
}
		
/*========================================================
	SortReposit
	Description : 
	=======================================================*/
tVOID SortReposit(REPOSIT *lpReposit, CompProcP CompReposit )
{
	if (lpReposit->nUsed > 1) {
		qsort(lpReposit->lpBuf, lpReposit->nUsed, lpReposit->nWidth, CompReposit);
	}
}




