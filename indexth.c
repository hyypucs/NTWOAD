#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dwtype.h"
#include "record.h"
#include "indexth.h"


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


