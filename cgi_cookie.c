/*
	CGI  Cookie manager
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>

#include "cgi_cookie.h"

typedef struct {
        char *lpszName;
        unsigned char *lpszValue;
}COOKIE_FORMAT, *LPCOOKIE_FORMAT;

#define MAX_COOKIEMAN 300

static COOKIE_FORMAT *gCookieMan[MAX_COOKIEMAN];
static int gnCookieMan = 0;

#if !defined(_NOT_USE_UTILE)
extern unsigned char Hex2Char(char HexUp, char HexLow);
extern void HexStr2CharStr(char *HexStr, char *CharStr);
extern void CharStr2HexStr(char *CharStr, char *HexStr, int nOpt);
#endif

int StartCookieMan(void)
{
	char *p, *q;
	int nLen;

	p = getenv("HTTP_COOKIE");
	if (p == NULL) return 1;

	if (strncmp(p, "Cookie: ", 8) == 0) {
		p += 8;
	}

	while (1) {
		if (gnCookieMan >= MAX_COOKIEMAN) break;
		gCookieMan[gnCookieMan] = (COOKIE_FORMAT *)malloc(sizeof(COOKIE_FORMAT));
		if (gCookieMan[gnCookieMan] == NULL) break;

		while (*p == ' ' || *p == '\t')  p++;
		q = strchr(p, '=');
		if (q == NULL) break;
		nLen = q - p;
		gCookieMan[gnCookieMan]->lpszName = (char *)malloc(nLen+1);
		if (gCookieMan[gnCookieMan]->lpszName == NULL) {
			break;
		}
		strncpy(gCookieMan[gnCookieMan]->lpszName, p, nLen);
		*(gCookieMan[gnCookieMan]->lpszName + nLen) = '\0';

		p = q+1;

		q = strchr(p, ';');
		if (q) {
			nLen = q - p;
		}
		else {
			nLen = strlen(p);
		}

		gCookieMan[gnCookieMan]->lpszValue = (unsigned char *)malloc(nLen+1);
		if (gCookieMan[gnCookieMan]->lpszValue == NULL) {
			break;
		}
		strncpy((char *)(gCookieMan[gnCookieMan]->lpszValue), p, nLen);
		*(gCookieMan[gnCookieMan]->lpszValue + nLen) = '\0';

		// Decode
		HexStr2CharStr((char *)(gCookieMan[gnCookieMan]->lpszValue), (char *)(gCookieMan[gnCookieMan]->lpszValue));
		gnCookieMan ++;
		if (q) {
			p = q+1;
		}
		else break;
	}

	return 1;
}

void CloseCookieMan(void)
{
	int i;
	
	for ( i = 0 ; i < gnCookieMan ; i ++ ) {
		if (gCookieMan[i]->lpszName) free(gCookieMan[i]->lpszName);
		if (gCookieMan[i]->lpszValue) free(gCookieMan[i]->lpszValue);
		free(gCookieMan[i]);
	}

	gnCookieMan = 0;
	
}

void ViewCookieMan(void)
{
	int i;
	
	for ( i = 0 ; i < gnCookieMan ; i ++ ) {
		printf("Name : %s, Value = %s<br>\n", gCookieMan[i]->lpszName, gCookieMan[i]->lpszValue);
	}
}


// point만 받아 오자. 나중에 free하면 안됨!!
int GetCookie(char *szName, char **szValue)
{
	int i;

	*szValue = NULL;

	if (gnCookieMan <= 0) return 0;

	for ( i = 0 ; i < gnCookieMan ; i ++ ) {
		if (strcasecmp(gCookieMan[i]->lpszName, szName) == 0) {
			*szValue = (char *)(gCookieMan[i]->lpszValue); //pointing..
			return 1;
		}
	}

	return 0;
}

void MakeCookieHead(char *szHead, char *szName, char *szValue, char *szExpire, char *szPath, char *szDomain, int bSecure)
{
	strcpy(szHead, "Set-Cookie: ");
	strcat(szHead, szName);
	strcat(szHead, "=");
	strcat(szHead, szValue);
	strcat(szHead,";");
	if (szExpire && *szExpire) {
		strcat(szHead, " expires=");
		strcat(szHead, szExpire);
		strcat(szHead,";");
	}
	if (szPath && *szPath) {
		strcat(szHead, " path=");
		strcat(szHead, szPath);
		strcat(szHead,";");
	}
	if (szDomain && *szDomain) {
		strcat(szHead, " domain=");
		strcat(szHead, szDomain);
		strcat(szHead,";");
	}
	if (bSecure) {
		strcat(szHead," secure");
	}
}

void MakeCookieHeadEx(char *szHead, char *szName, char *szValue, char *szPath, char *szDomain, int bSecure, int nAfterHour)
{
     	time_t curtime;

	strcpy(szHead, "Set-Cookie: ");
	strcat(szHead, szName);
	strcat(szHead, "=");
	strcat(szHead, szValue);
	strcat(szHead,";");

	//-------------
	//
	// 시간 계산해서 update..
	// 
	curtime = time(NULL);
	curtime +=  (nAfterHour * 3600); // 초단위로 해서 update해야 하니깐..

	strcat(szHead, " expires=");

	strcat(szHead, ctime(&curtime));
	szHead[strlen(szHead)-1] = '\0';

	strcat(szHead,";");
	//-------------

	if (szPath && *szPath) {
		strcat(szHead, " path=");
		strcat(szHead, szPath);
		strcat(szHead,";");
	}
	if (szDomain && *szDomain) {
		strcat(szHead, " domain=");
		strcat(szHead, szDomain);
		strcat(szHead,";");
	}
	if (bSecure) {
		strcat(szHead," secure");
	}
}

#if defined(_NOT_USE_UTIL)
// 나중에 util.c로 옮기자.
unsigned char Hex2Char(char HexUp, char HexLow)
{
	unsigned char ResultChar = 0;
	
	if (HexUp >= 'A' && HexUp <= 'F') {
		ResultChar = HexUp - 'A' + 10;
	}	
	else if (HexUp >= 'a' && HexUp <= 'f') {
		ResultChar = HexUp - 'a' + 10;
	}
	else if (HexUp >= '0' && HexUp <= '9') {
		ResultChar = HexUp - '0';
	}
	else { // exception
		return 0;
	}
	ResultChar *= 16;

	if (HexLow >= 'A' && HexLow <= 'F') {
		ResultChar += (HexLow - 'A' + 10);
	}	
	else if (HexLow >= 'a' && HexLow <= 'f') {
		ResultChar += (HexLow - 'a' + 10);
	}
	else if (HexLow >= '0' && HexLow <= '9') {
		ResultChar += (HexLow - '0');
	}
	else { // exception
		return 0;
	}
	return ResultChar;
}

void HexStr2CharStr(char *HexStr, char *CharStr)
{
	unsigned char *p = HexStr, *q = CharStr;


	while (*p) {
		if (*p & 0x80) {
			*q = *p;
			p++;
			q++;
			*q = *p;
		}
		else if (*p == '+') {
			*q = ' ';
		}
		else if (*p == '%') {
			p++;
/*
	 필요없는 듯 싶다.
			if (*p && *p == '%') {
				*q = *p;
			}
			else 
*/
			if (*p && *(p+1)) {
				*q = Hex2Char(*p, *(p+1));
				if (*q == 0) { // 바뀔 것이 아니다.
					*q++ = '%';
					continue; // % 다음부터 조사해야지..
				}
				p++;
			}
			else if (*p) { // exception
				*q++ = '%';
				*q = *p;
			}
			else { // *p == NULL일 때..
				*q++ = '%';
				break;
			}
		}
		else {
			*q = *p;
		}

		p++;
		q++;
	}
	
	*q = '\0';
}

//
// nOpt : 0 -> 기본 default 변환 
//        2byte 문자와  특수문자(isalnum에 해당 안되는 것들 모두) ' ', '+', '"', ''', '\t', '\a', '\n', '=', '&', '%'
//        만 hex코드로 바꾼다.
//
//        1 -> 모두 hex코드로 바꾼다.
//
//        2 -> 특수문자만 hex code로 바꾼다.
//       
//
void CharStr2HexStr(char *CharStr, char *HexStr, int nOpt)
{
	int i;
	char tmpStr[6];

	HexStr[0] = '\0';
	for ( i = 0 ; i < strlen(CharStr) ; i ++ ) {
		if (CharStr[i] & 0x80) {
			if (nOpt == 2) {
				tmpStr[0] = CharStr[i++];
				tmpStr[1] = CharStr[i];
				tmpStr[2] = '\0';
                                strcat(HexStr, tmpStr);
			}
			else {
				sprintf(tmpStr , "%%%02x", (unsigned char)CharStr[i++]);
                                strcat(HexStr, tmpStr);
				sprintf(tmpStr , "%%%02x", (unsigned char)CharStr[i]);
                                strcat(HexStr, tmpStr);
			}
		}
		else if (isalnum((int)(CharStr[i])) || CharStr[i] == '/') {
			if (nOpt == 1) {
				sprintf(tmpStr , "%%%02x", (unsigned char)CharStr[i]);
                       		strcat(HexStr, tmpStr);
			}
			else {
				tmpStr[0] = CharStr[i];
				tmpStr[1] = '\0';
                                strcat(HexStr, tmpStr);
			}
		}
		else { // letter, number가 아니다.
			sprintf(tmpStr , "%%%02x", (unsigned char)CharStr[i]);
                        strcat(HexStr, tmpStr);
		}
	}
}
#endif

