/*

        Profile reader
        Author : HYYPUCS
        B-Date : 1999. 9. 17.
        Comment : Profile∏¶ read«‘
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dwtype.h"
#include "profile.h"

PRIVATE tBOOL GetSectionName(tCHAR *szBuf, tCHAR *szSection);
PRIVATE tVOID AdjustStr(tCHAR *str);

PRIVATE tVOID AdjustStr(tCHAR *str)
{
        tINT i = strlen(str) - 1;

        while (i>=0 && (str[i] == '\t' || str[i] == '\n' || str[i] == '\r' || str[i] == ' ')) {
                str[i] = '\0';
                i -- ;
        }
	i = 0;
	while (str[i]) {
		if (str[i] == ' ' || str[i] == '\t') {
			if (str[i+1] == '\0') {
				str[i] = '\0';
				break;
			}
			else strcpy(str, str+1);
		}
		else break;
		i++;
	}
}


PUBLIC tBOOL GetProfileStrEx(tCHAR *szProfileName, tCHAR *szSectionName, tCHAR *szName, tCHAR *szValue, tINT nMaxStr, tINT nOpt)
{
	FILE *fp;
	tBOOL bRetVal = FALSE;
	tCHAR szBuf[1024], szStr[1024], *pChar;

	if ((fp = fopen(szProfileName, "rt")) == NULL) {
		return bRetVal;
	}

	while (fgets(szBuf, 1023, fp)) {
		if (szBuf[0] == '/') continue;
		if (szBuf[0] == '[')  {
			szStr[0] = '\0';
			GetSectionName(szBuf, szStr);
			if (szStr[0] && strcmp(szStr, szSectionName) == 0) {
				bRetVal = TRUE;
				break;
			}
		}

	}
	if (bRetVal == FALSE) {
		fclose(fp);
		return bRetVal;
	}
	bRetVal = FALSE;
	while (fgets(szBuf, 1023, fp)) {
		if (szBuf[0] == '/') continue;
		if (szBuf[0] == '[')  {
			break;
		}
		pChar = szBuf;

		if ((pChar = strchr(pChar, '=')) == NULL) continue;
		*pChar = '\0';

		if (sscanf(szBuf, "%s", szStr) == 1) {
			if (strcmp(szStr, szName)) continue;

			pChar ++;
			if (nOpt) {
				if (sscanf(pChar, "%s", szStr) != 1) {
					continue;
				}
			}
			else {
				strcpy(szStr, pChar);
				AdjustStr(szStr);
			}

			if (nMaxStr <= 1) {
				strcpy(szValue, szStr);
			}
			else {
				strncpy(szValue, szStr, nMaxStr - 1);
				szValue[nMaxStr-1] = '\0';
			}
			bRetVal = TRUE;
			break;
		}
	}
	fclose(fp);

	return bRetVal;
	
}

PRIVATE tBOOL GetSectionName(tCHAR *szBuf, tCHAR *szSection)
{
	tCHAR szStr[1024];
	tINT i, j;

	if (szBuf[0] != '[') {
		return FALSE;
	}

	i = 1;
	j = 0;
	while (szBuf[i] && szBuf[i] != ']') {
		if (szBuf[i] & 0x80) {
			szStr[j++] = szBuf[i++];
		}
		szStr[j++] = szBuf[i++];
	}
	if (szBuf[i] == '\0') {
		szSection[0] = '\0';
		return FALSE;
	}
	szStr[j] = '\0';
	if (sscanf(szStr, "%s", szSection) != 1) {
		szSection[0] = '\0';
		return FALSE;
	}

	return TRUE;
}

PUBLIC tBOOL GetProfileStrOut(tCHAR *szProfileName, tCHAR *szSectionName, ProfileOutput lpProfileOutput, tINT nOpt)
{
	FILE *fp;
	tBOOL bRetVal = FALSE;
	tCHAR szBuf[1024], szStr[1024], *pChar;
	tCHAR szName[1024];

	if ((fp = fopen(szProfileName, "rt")) == NULL) {
		return bRetVal;
	}

	while (fgets(szBuf, 1023, fp)) {
		if (szBuf[0] == '/') continue;
		if (szBuf[0] == '[')  {
			szStr[0] = '\0';
			GetSectionName(szBuf, szStr);
			if (szStr[0] && strcmp(szStr, szSectionName) == 0) {
				bRetVal = TRUE;
				break;
			}
		}

	}
	if (bRetVal == FALSE) {
		fclose(fp);
		return bRetVal;
	}
	bRetVal = FALSE;
	while (fgets(szBuf, 1023, fp)) {
		if (szBuf[0] == '/') continue;
		if (szBuf[0] == '[')  {
			break;
		}
		pChar = szBuf;

		if ((pChar = strchr(pChar, '=')) == NULL) continue;
		*pChar = '\0';

		if (sscanf(szBuf, "%s", szName) == 1) {

			pChar ++;
			if (nOpt) {
				if (sscanf(pChar, "%s", szStr) != 1) {
					continue;
				}
			}
			else {
				strcpy(szStr, pChar);
				AdjustStr(szStr);
			}
			lpProfileOutput(szName, szStr);

			bRetVal = TRUE;
		}
	}
	fclose(fp);

	return bRetVal;
}

