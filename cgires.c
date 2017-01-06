/*
		cgi에서 resource 관리를 따로 하기 위해서 만든 라이브러리이다.
		이를 이용해서 출력을 능동적으로 대처하도록 한다.

		1999. 12. 6.
			by hyypucs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgires.h"

#define MAX_LINE 1024
#define MAX_VALUE 	1024
#define MAX_FORMDATA 	500
#define MAX_ENVDATA 	5000

typedef struct {
	char szName[MAX_FORMNAME];
	unsigned char *Data;
}FORMDATA_FORMAT, *LPFORMDATA_FORMAT;

static LPFORMDATA_FORMAT lpFormData[MAX_FORMDATA];
static int c_FormData = 0;

typedef struct {
	char szName[MAX_ENVNAME];
	unsigned char *lpszValue;
}ENVDATA_FORMAT, *LPENVDATA_FORMAT;

static LPENVDATA_FORMAT lpEnvData[MAX_ENVDATA];
static int c_EnvData = 0;


static int bOpenCgiRes = 0;

void GetProgArgv(char *szCmd, char *prog, char *argv);
void printStr(char *szStr);
void fprintStr(FILE *fp, char *szStr);

int StartCgiRes(void) 
{
	if (bOpenCgiRes) return 1;

	memset(lpFormData, 0, sizeof(LPFORMDATA_FORMAT) * MAX_FORMDATA);
	memset(lpEnvData, 0, sizeof(LPENVDATA_FORMAT) * MAX_ENVDATA);

	bOpenCgiRes = 1;
	return 1;
}

int CloseCgiRes(void)
{
	if (bOpenCgiRes == 0) return 1;

	FreeFormData();
	FreeEnvData();

	bOpenCgiRes = 0;
	return 1;
}

void FreeFormData(void)
{
	int i;

	for  (i = 0 ; i < c_FormData ; i ++ ) {
		if (lpFormData[i]->Data) {
			free(lpFormData[i]->Data);
		}
		free(lpFormData[i]);
		lpFormData[i] = NULL;
	}
	c_FormData = 0;
}

void FreeEnvData(void)
{
	int i;

	for  (i = 0 ; i < c_EnvData ; i ++ ) {
		if (lpEnvData[i]->lpszValue) {
			free(lpEnvData[i]->lpszValue);
		}
		free(lpEnvData[i]);
		lpEnvData[i] = NULL;
	}
	c_EnvData = 0;
}

int LoadFormData(char *szFileName)
{
	char szLine[MAX_LINE];
	char szBuff[MAX_LINE];
	FILE *fp;
	char szName[MAX_FORMNAME];

	if ((fp = fopen(szFileName, "rt")) == NULL) {
		return 0;
	}

	szName[0] = '\0';
	while (fgets(szLine, MAX_LINE-1, fp)) {
		if (sscanf(szLine, "%s", szBuff) == 1) {
			if (strlen(szBuff) >= 2 && szBuff[0] == '%' && szBuff[1] == '%') {
				strncpy(szName, szBuff+2, MAX_FORMNAME-1);	
				szName[MAX_FORMNAME - 1] = '\0';
			}
			else {
				AppendFormData(szName,  szLine);
			}
		}
		else {
			AppendFormData(szName, szLine);
		}
	}

	fclose(fp);
	return 1;
}

int AppendFormData(char *szName, char *szBuf)
{
	int i;

	if (szName[0] == '\0') return 0;
	if (szBuf[0] == '\0') return 0;
	if (strlen(szName) >= MAX_FORMNAME) szName[MAX_FORMNAME-1] = '\0';

	for ( i = 0 ; i < c_FormData ; i ++ ) {
		if (strcasecmp(lpFormData[i]->szName, szName) == 0) {
			break;
		}
	}
	if (i >= c_FormData) { // 발견못함
		if (c_FormData >= MAX_FORMDATA) return 0;
		lpFormData[c_FormData] = (FORMDATA_FORMAT *)malloc(sizeof(FORMDATA_FORMAT));
		if (lpFormData[c_FormData] == NULL) return 0;

		strcpy(lpFormData[c_FormData]->szName, szName);
		lpFormData[c_FormData]->Data = (unsigned char *)malloc(strlen(szBuf)+1);
		if (lpFormData[c_FormData]->Data == NULL) {
			free(lpFormData[c_FormData]);
			return 0;
		}
		strcpy((char *)(lpFormData[c_FormData]->Data), szBuf);
		c_FormData ++;
	}
	else {
		lpFormData[i]->Data = (unsigned char *)realloc((void*)(lpFormData[i]->Data), (int)strlen((char *)(lpFormData[i]->Data)) + strlen(szBuf)+1);
		strcat((char *)(lpFormData[i]->Data), szBuf);

	}

	return 1;
}

char *GetFormData(char *szName, char **lpszBuf)
{
	int i;

	if (szName[0] == '\0') return 0;

	for ( i = 0 ; i < c_FormData ; i ++ ) {
		if (strcasecmp(lpFormData[i]->szName, szName) == 0) {
			break;
		}
	}
	if (i < c_FormData) {
		*lpszBuf = (char *)(lpFormData[i]->Data);
		return *lpszBuf;
	}
	*lpszBuf = NULL;
	return 0;
}

int SetEnvData(char *szName, char *szValue)
{
	int i;

	if (szName[0] == '\0') return 0;
	if (szValue[0] == '\0') return 0;

	if (strlen(szName) >= MAX_ENVNAME) szName[MAX_ENVNAME-1] = '\0';

	for ( i = 0 ; i < c_EnvData ; i ++ ) {
		if (strcmp(lpEnvData[i]->szName, szName) == 0) {
			break;
		}
	}
	if (i >= c_EnvData) { // 발견못함
		if (c_EnvData >= MAX_ENVDATA) return 0;
		lpEnvData[c_EnvData] = (ENVDATA_FORMAT *)malloc(sizeof(ENVDATA_FORMAT));
		if (lpEnvData[c_EnvData] == NULL) return 0;

		strcpy(lpEnvData[c_EnvData]->szName, szName);
		lpEnvData[c_EnvData]->lpszValue = (unsigned char *)malloc(strlen(szValue)+1);
		if (lpEnvData[c_EnvData]->lpszValue == NULL) {
			free(lpEnvData[c_EnvData]);
			return 0;
		}
		strcpy((char *)(lpEnvData[c_EnvData]->lpszValue), szValue);
		c_EnvData ++;
	}
	else {
		free(lpEnvData[i]->lpszValue);
		lpEnvData[i]->lpszValue = (unsigned char *)malloc(strlen(szValue)+1);
		// exception 처리가 있어야 할 것 같은데.. 찜찜
		strcpy((char *)(lpEnvData[i]->lpszValue), szValue);
	}

	return 1;
}

int DelEnvData(char *szName)
{
	int i, j;

	if (szName[0] == '\0') return 0;

	if (strlen(szName) >= MAX_ENVNAME) szName[MAX_ENVNAME-1] = '\0';

	if (c_EnvData <= 0) return 1;

	for ( i = 0 ; i < c_EnvData ; i ++ ) {
		if (strcmp(lpEnvData[i]->szName, szName) == 0) {
			break;
		}
	}
	if (i < c_EnvData)  {
		free(lpEnvData[i]->lpszValue);
		if ( i < c_EnvData - 1) {
			for ( j = i ; j < c_EnvData - 1 ; j ++ ) {
				memcpy(&(lpEnvData[j]) , &(lpEnvData[j+1]), sizeof(ENVDATA_FORMAT));
			}
		}
		c_EnvData --;
	}

	return 1;
}

int SetEnvDataInt(char *szName, int nValue)
{
	char szValue[MAX_VALUE];

	sprintf(szValue, "%d", nValue);

	return (SetEnvData(szName, szValue));
}

char *GetEnvData(char *szName, char **lpszValue)
{
	int i;

	if (szName[0] == '\0') return 0;

	for ( i = 0 ; i < c_EnvData ; i ++ ) {
		if (strcmp(lpEnvData[i]->szName, szName) == 0) {
			break;
		}
	}
	if (i < c_EnvData) {
		*lpszValue = (char *)(lpEnvData[i]->lpszValue);
		return *lpszValue;
	}
	*lpszValue = NULL;
	return 0;
}

void PrintForm(char *szName)
{
	char *szBuf;
	char *p, *q;
	int len;
	char *szTmp;
	char *szValue;

	GetFormData(szName, &szBuf);
	if (szBuf == NULL) return ;
	p = szBuf;

	szTmp = (char *)malloc(strlen(szBuf)+1);
	if (szTmp == NULL) return ;

	while (*p) {
		q = strstr(p, "{$");
		if (q == NULL) {
			printf(p);
			break;
		}
		else {
			len = q - p;
			if (len) {
				strncpy(szTmp, p, len);
				szTmp[len] = '\0';
				printStr(szTmp);
			}
			p = strchr(q, '}');
			if (p == NULL) {
				printf("{$");
				p += 2;
			}
			else {
				len = p - q - 2;
				if (len) {
					strncpy(szTmp, q+2, len);
					szTmp[len] = '\0';

					if (strncasecmp(szTmp, "IncludeCGI(", strlen("IncludeCGI(")) == 0) {
						IncludeCGI(szTmp, NULL);
					}
					else if (strncasecmp(szTmp, "IncludeEXE(", strlen("IncludeEXE(")) == 0) {
						IncludeEXE(szTmp, NULL);
					}
					else {
						GetEnvData(szTmp, &szValue);
						if (szValue != NULL) {
							printStr(szValue);
					 		//printf("%s", szValue);
						}
					}
				}
				p += 1;
			}
		}
	}
	free(szTmp);
}

void GetParsedBuf(char *szBuf, char **szRetBuf)
{
	char *p, *q, *r;
	int len, rlen;
	char *szTmp;
	char *szValue;

	if (szBuf == NULL) return ;
	p = szBuf;

	*szRetBuf = (char *)malloc(1);
	r = *szRetBuf;
	*r = '\0';

	szTmp = (char *)malloc(strlen(szBuf)+1);
	if (szTmp == NULL) return ;

	while (*p) {
		q = strstr(p, "<$");
		if (q == NULL) {
			rlen = strlen(r);
			*szRetBuf = (char *)realloc(r, rlen+strlen(p)+1);
			r = *szRetBuf;
			strcat(r, p);
			break;
		}
		else {
			len = q - p;
			if (len) {
				strncpy(szTmp, p, len);
				szTmp[len] = '\0';
				rlen = strlen(r);
				*szRetBuf = (char *)realloc(r, rlen+strlen(szTmp)+1);
				r = *szRetBuf;
				strcat(r, szTmp);
			}
			p = strchr(q, '>');
			if (p == NULL) {
				printf("<$");
				p += 2;
			}
			else {
				len = p - q - 2;
				if (len) {
					strncpy(szTmp, q+2, len);
					szTmp[len] = '\0';

					GetEnvData(szTmp, &szValue);
					if (szValue != NULL) {
						//printf("\n<!-- {$%s} not found -->\n", szTmp);
					//printf("{$%s}", szTmp);
				 		//printf(szValue);
						rlen = strlen(r);
						*szRetBuf = (char *)realloc(r, rlen+strlen(szValue)+1);
						r = *szRetBuf;
						strcat(r, szValue);
					}
				}
				p += 1;
			}
		}
	}
	free(szTmp);
}

void GetProgArgv(char *szCmd, char *prog, char *argv)
{
	char *p, *q;
	int len;

	prog[0] = '\0';
	argv[0] = '\0';

	p = szCmd;
	while (1) {
		p = strchr(p, '"');
		if (p == NULL) {
			return;	
		}
		else if (*(p-1) == '\\') {
			p++;
		}
		else {
			break;
		}
	}
	q = p+1;
	while (1) {
		q = strchr(q, '"');
		if (q == NULL) {
			return;	
		}
		else if (*(q-1) == '\\') {
			q++;
		}
		else {
			break;
		}
	}

	len = q - p - 1;
	if (len) {
		strncpy(prog, p+1, len);
		prog[len] = '\0';
	}

	// argv
	p = q+1;
	while (1) {
		p = strchr(p, '"');
		if (p == NULL) {
			return;	
		}
		else if (*(p-1) == '\\') {
			p++;
		}
		else {
			break;
		}
	}
	q = p+1;
	while (1) {
		q = strchr(q, '"');
		if (q == NULL) {
			return;	
		}
		else if (*(q-1) == '\\') {
			q++;
		}
		else {
			break;
		}
	}

	len = q - p - 1;
	if (len) {
		strncpy(argv, p+1, len);
		argv[len] = '\0';
	}
}

void IncludeCGI(char *szCmd, FILE *ofp)
{
	char *p, *a, *qs=NULL;
	char *prog, *argv;
	char szLine[1024];
	FILE *fp;

	prog = (char *)malloc(strlen(szCmd)+1);
	if (prog == NULL) return;
	argv = (char *)malloc(strlen(szCmd)+1);
	if (argv == NULL) {
		free(prog);
		return;
	}

	GetProgArgv(szCmd, prog, argv);
	if (prog[0] == '\0') {
		free(prog);
		free(argv);
		return;	
	}

	GetParsedBuf(prog, &p);
	if (argv[0]) {
		GetParsedBuf(argv, &a);
        	qs = (char *)malloc( strlen( a )+20 );
        	sprintf( qs, "QUERY_STRING=%s", a );
        	putenv( qs );
	}
	else {
        	putenv( "QUERY_STRING=");
	}

	putenv( "REQUEST_METHOD=GET" );

	fp = popen(p, "r");
	if (fp) {
		while (fgets(szLine, 1023, fp)) {
			if (ofp) {
				fputs(szLine, ofp);
			}
			else {
				puts(szLine);
			}
		}
		pclose(fp);
	}

	free(p);
	if (argv[0]) {
		free(a);
		free(qs);
	}
	free(prog);
	free(argv);
}

void IncludeEXE(char *szCmd, FILE *ofp)
{
	char *p, *a, *qs;
	char *prog, *argv;
	char szLine[1024];
	FILE *fp;

	prog = (char *)malloc(strlen(szCmd)+1);
	if (prog == NULL) return;
	argv = (char *)malloc(strlen(szCmd)+1);
	if (argv == NULL) {
		free(prog);
		return;
	}

	GetProgArgv(szCmd, prog, argv);
	if (prog[0] == '\0') {
		free(prog);
		free(argv);
		return;	
	}

	GetParsedBuf(prog, &p);
	if (argv[0]) {
		GetParsedBuf(argv, &a);
		qs = (char *)malloc(strlen(p) + strlen(a) + 10);
		sprintf(qs, "%s %s", p, a);
	}
	else {
		qs = (char *)malloc(strlen(p) + 10);
		sprintf(qs, "%s", p);
	}

	fp = popen(qs, "r");
	if (fp) {
		while (fgets(szLine, 1023, fp)) {
			if (ofp) {
				fputs(szLine, ofp);
			}
			else {
				puts(szLine);
			}
		}
		pclose(fp);
	}

	free(p);
	if (argv[0]) free(a);
	free(qs);
	free(prog);
	free(argv);
}

void FprintForm(FILE *fp, char *szName)
{
	char *szBuf;
	char *p, *q;
	int len;
	char *szTmp;
	char *szValue;

	GetFormData(szName, &szBuf);
	if (szBuf == NULL) return ;
	p = szBuf;

	szTmp = (char *)malloc(strlen(szBuf)+1);
	if (szTmp == NULL) return ;

	while (*p) {
		q = strstr(p, "{$");
		if (q == NULL) {
			fprintStr(fp, p);
			break;
		}
		else {
			len = q - p;
			if (len) {
				strncpy(szTmp, p, len);
				szTmp[len] = '\0';
				fprintStr(fp, szTmp);
				//fputs(szTmp, fp);
			}
			p = strchr(q, '}');
			if (p == NULL) {
	//			fputs("{$", fp);
				fprintStr(fp, "{$");
				p += 2;
			}
			else {
				len = p - q - 2;
				if (len) {
					strncpy(szTmp, q+2, len);
					szTmp[len] = '\0';
					if (strncasecmp(szTmp, "IncludeCGI(", strlen("IncludeCGI(")) == 0) {
						IncludeCGI(szTmp, fp);
					}
					else if (strncasecmp(szTmp, "IncludeEXE(", strlen("IncludeEXE(")) == 0) {
						IncludeEXE(szTmp, fp);
					}
					else {
						GetEnvData(szTmp, &szValue);
						if (szValue != NULL) {
					 		//fputs(szValue, fp);
							fprintStr(fp, szValue);
						}
					}
				}
				p += 1;
			}
		}
	}
	free(szTmp);
}

void printStr(char *szStr)
{
	int i;

	for ( i = 0 ; i < (int)strlen(szStr) ; i ++ ) putchar(szStr[i]);
} 
void fprintStr(FILE *fp, char *szStr)
{
	int i;

	for ( i = 0 ; i < (int)strlen(szStr) ; i ++ ) fputc(szStr[i], fp);
} 
