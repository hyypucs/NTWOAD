#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>

#include "dwtype.h"
#include "hstrlib.h"
#include "profile.h"
#include "dwsock.h"
#include "util.h"
#include "safemem.h"
#if defined(_MEM_CHK)
#include "memchk.h"
#endif

tVOID LowStr(tCHAR *szStr);
static tBOOL Hex2Char( tBYTE _upper, tBYTE _lower , tBYTE *result);
#if !defined(_OLD)
tVOID DelSpChr(tCHAR *szStr);
#endif


tVOID ReplaceChar2Str(tCHAR *strnew, tCHAR *strorg, tCHAR ch, tCHAR *str)
{
	tINT i = 0, j=0;

	if(strlen(strorg) == 0) {
		return;
	}

	//printf("strnew=>[%s], strorg=>[%s]\n", strnew, strorg);
	for(i=0; i<strlen(strorg); i++) {
		if(strorg[i] == ch) {
			strcat(strnew, str);
			j += strlen(str);
			*(strnew+j) = '\0';
		}
		else {
			*(strnew+j) = *(strorg+i);
			j += 1;
			*(strnew+j) = '\0';
		}
	}
	//printf("strnew2=>[%s]\n\n", strnew);
}


tVOID CvtChStr(tHCHAR *uszStr, tHCHAR srcCh, tHCHAR tarCh)
{
	tHCHAR *p = uszStr;

	while (*p) {
		if (*p == srcCh) *p = tarCh;
		p++;
	}

}

// " => quat; > -> gt; < -> lt;
tVOID Convert4Html(tCHAR *srcName, tCHAR *tarName)
{
	tINT i, j = 0;

	for ( i = 0 ; i < strlen(srcName) ; i ++ ) {
		if (srcName[i] & 0x80) {
			tarName[j ++] = srcName[i++];
			tarName[j ++] = srcName[i];
		}
#if defined(_FUTURE)
		else if (srcName[i] == '"') {
			tarName[j++] = 'q';
			tarName[j++] = 'u';
			tarName[j++] = 'a';
			tarName[j++] = 't';
			tarName[j++] = ';';
		}
		else if (srcName[i] == '>') {
			tarName[j++] = 'g';
			tarName[j++] = 't';
			tarName[j++] = ';';
		}
		else if (srcName[i] == '<') {
			tarName[j++] = 'l';
			tarName[j++] = 't';
			tarName[j++] = ';';
		}
#else
		else if (srcName[i] == '"') {
			tarName[j++] = '\'';
		}

#endif
		else {
			tarName[j++] = srcName[i];
		}
	}
	tarName[j] = '\0';
}

time_t GetDateTimeT(tVOID)
{

        time_t    t = time(NULL);
        struct tm *Tm = localtime(&t);

        Tm->tm_sec = 0;
        Tm->tm_min = 0;
        Tm->tm_hour = 0;
        Tm->tm_isdst = -1;

        return  mktime(Tm);
}

// YYYYMMDDTTMMSS형식
time_t MakeTimeTFromDateStr(tCHAR *szTime)
{
	tCHAR szBuf[6];
	struct tm time_str;

	if (strlen(szTime) != 14) return -1;
	
	strncpy(szBuf, szTime, 4);
        szBuf[4] = '\0';
        time_str.tm_year = atoi(szBuf) - 1900;

        strncpy(szBuf, szTime+4, 2);
        szBuf[2] = '\0';
        time_str.tm_mon = atoi(szBuf) - 1;

        strncpy(szBuf, szTime+6, 2);
        szBuf[2] = '\0';
        time_str.tm_mday = atoi(szBuf);

        strncpy(szBuf, szTime+8, 2);
        szBuf[2] = '\0';
        time_str.tm_hour = atoi(szBuf);

        strncpy(szBuf, szTime+10, 2);
        szBuf[2] = '\0';
        time_str.tm_min = atoi(szBuf);

        strncpy(szBuf, szTime+12, 2);
        szBuf[2] = '\0';
        time_str.tm_sec = atoi(szBuf);

        time_str.tm_isdst = -1;

        return  mktime(&time_str);
}

PRIVATE tCHAR szRootDir[MAX_PATH];

tVOID SetRootDir(tCHAR *szRDir)
{
        strcpy(szRootDir, szRDir);
        if (szRootDir[strlen(szRootDir)-1] != '/') strcat(szRootDir, "/");
}

tVOID GetRootDir(tCHAR *szRDir)
{
        strcpy(szRDir, szRootDir);
}


int ExistFile(char *filename)
{
#if defined(_WINDOWS)
                struct _stat buf;
                if (_stat(filename, &buf)==-1) {
               return (0);
        }
#else
        struct stat buf;
                if (stat(filename, &buf)==-1) {
               return (0);
        }
#endif


        if ((buf.st_mode & S_IFMT) == S_IFDIR) return 0;

        return 1;
}

int GetFileSize(char *filename)
{
#if defined(_WINDOWS)
                struct _stat buf;
                if (_stat(filename, &buf)==-1) {
               return (-1);
        }
#else
        struct stat buf;
                if (stat(filename, &buf)==-1) {
               return (-1);
        }
#endif


        if ((buf.st_mode & S_IFMT) == S_IFDIR) return -1;

        return buf.st_size;
}

tVOID GetTimeStrFromTime(tCHAR *szTime, tINT nLen, time_t timeT, tINT nOpt)
{
        struct tm *st = localtime(&timeT);

        if (nOpt == GETDATE) {
                strftime(szTime, nLen, "%Y/%m/%d", st);
        }
        else if (nOpt == GETTIME) {
                strftime(szTime, nLen, "%T", st);
        }
        else if (nOpt == GETDATETIME) {
                strftime(szTime, nLen, "%Y/%m/%d %T", st);
        }
        else if (nOpt == GETDATESTR) {
                strftime(szTime, nLen, "%Y%m%d", st);
        }
        else { /* default */
                strftime(szTime, nLen, "%Y/%m/%d %T", st);
        }
}

//
// 0 : 한글만 한다.
// 1 : 모두 다 한다.
// 2 : 특수문자 (space, tab, return, new line, +, &, =, ", ')
// 3 : 한글 + 특수문자  : 추후 만들자. 
//
void MakeHexFromStr(char *hexamsg, unsigned char *kssmmsg, int opt)
{
	int i=0, j=0;
	char tmpstr[5];
	
	hexamsg[j] = '\0';
	switch(opt) {
		case 0 : // 한글만 
			while (kssmmsg[i]) {
				if (kssmmsg[i] & 0x80) {
					sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
					sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
				}
/*
				else if (kssmmsg[i] == ' ' || kssmmsg[i] == '+' || kssmmsg[i] == '%') {
					sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
				}
*/
				else if (isalnum((int)(kssmmsg[i])) || kssmmsg[i] == '/') {
					sprintf(tmpstr , "%c", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
				}
				else {
					sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
				}
			}
			break;
		case 1 : // 모두 다 
			while (kssmmsg[i]) {
				sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
				strcat(hexamsg, tmpstr);
			}
			break;
		case 2 : // 특수문자.
			while (kssmmsg[i]) {
				if (kssmmsg[i] & 0x80) {
					sprintf(tmpstr , "%c", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
					sprintf(tmpstr , "%c", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
				}
/*
				else if (kssmmsg[i] == ' ' || kssmmsg[i] == '+' || kssmmsg[i] == '\t' || kssmmsg[i]=='\n' || kssmmsg[i] == '&' || kssmmsg[i] == '=' || kssmmsg[i] == '\"' || kssmmsg[i] == '\'' || kssmmsg[i] == '%') {
					sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
				}
*/
				//else if (isalnum((int)(kssmmsg[i]))) {
				else if (isalnum((int)(kssmmsg[i])) || kssmmsg[i] == '/') {
					sprintf(tmpstr , "%c", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
				}
				else {
					sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
					strcat(hexamsg, tmpstr);
				}
			}
			break;
		default :
			while (kssmmsg[i]) {
				sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
				strcat(hexamsg, tmpstr);
			}
			break;
			break;
	}
}

tBOOL IsEngHStr( tHCHAR *hstr )
{
        tHCHAR *p = hstr;

        while(*p) {
                if(IsEnglish(*p)) {
                        p++;
                        continue;
                }
                else return FALSE;
        }

        return TRUE;
}

tBOOL MemFromFile( tCHAR *szName, tHCHAR **lpBuf, tINT nMax )
{
	FILE *fp;
	tINT nLen;
	tCHAR *p;

	if ((fp = fopen(szName, "rt")) == NULL) {
		return FALSE;
	}
	fseek(fp, 0L, SEEK_END);
	nLen = ftell(fp);	
	fseek(fp, 0L, SEEK_SET);
	if (nMax != -1 &&  nMax < nLen) {
		nLen = nMax;
	}
	

	*lpBuf = (tHCHAR *)malloc((nLen+1) * 4);
	if (*lpBuf == NULL) {
		fclose(fp);
		return FALSE;
	}
	p = (tCHAR *)malloc((nLen+1)*2);
	if (p == NULL) {
		free(*lpBuf);
		fclose(fp);
		return FALSE;
	}

	fread(p, sizeof(tCHAR), nLen, fp);
	*(p+nLen) = '\0';
	str2hstr((unsigned char *)p, *lpBuf);
	free(p);

	fclose(fp);

	return TRUE;
}

tBOOL MemFromFileLn( tCHAR *szName, tHCHAR **lpBuf, tINT nMax )
{
        FILE *fp;
        tINT nLen, nLen1, nPos;
        tCHAR *p;

        if ((fp = fopen(szName, "rt")) == NULL) {
                return FALSE;
        }
        fseek(fp, 0L, SEEK_END);
        nLen = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        if (nMax != -1 &&  nMax < nLen) {
                nLen = nMax;
        }


        *lpBuf = (tHCHAR *)malloc((nLen+1) * 4);
        if (*lpBuf == NULL) {
                fclose(fp);
                return FALSE;
        }
        p = (tCHAR *)malloc((nLen+1)*2);
        if (p == NULL) {
                free(*lpBuf);
                fclose(fp);
                return FALSE;
        }

        fread(p, sizeof(tCHAR), nLen, fp);
        *(p+nLen) = '\0';

        nLen1 = nLen - 1;
        while( *(p+nLen1) != '\n' ) {
                nLen1 --;
        }
        if (nLen1>0 && nLen1+1 != nLen) {
                *(p+nLen1+1) = '\0';
                nPos = nLen - (nLen1+1);
                fseek(fp, nPos, SEEK_CUR);
        }

        str2hstr((unsigned char *)p, *lpBuf);
        free(p);
	fclose(fp);

        return TRUE;
}

tBOOL MemToFile( tHCHAR *lpBuf, tCHAR *szName )
{
	FILE *fp;
	tCHAR *p;
	tINT nLen;

	if ((fp = fopen(szName, "wt")) == NULL) {
		return FALSE;
	}

	nLen = HStrlen(lpBuf) * 2 + 1;
	p = (tCHAR *)malloc(nLen + 1);
	if (p == NULL) {
		fclose(fp);
		return FALSE;
	}
	hstr2str(lpBuf, (unsigned char *)p);
	fwrite(p, sizeof(tCHAR), strlen(p), fp);
	fclose(fp);
	free(p);
	return TRUE;
}


tHCHAR *StrGets( tHCHAR *lpBuf, tHCHAR *uszLine, tINT nLimit )
{
	tINT i;
	tINT nLen = HStrlen(lpBuf);	


	for ( i = 0 ; i < nLen/*HStrlen(lpBuf)*/ && i < nLimit - 1 ; i ++ ) {
		if (lpBuf[i] == (tHCHAR)'\n' || lpBuf[i] == (tHCHAR)'\r') {
			uszLine[i] = lpBuf[i];
			i++;
			break;
		}
		uszLine[i] = lpBuf[i];
	}
	uszLine[i] = 0x00;
	if (i) {
		return (lpBuf + i);
	}
	return NULL;
}


#define MAX_MASK 16
tVOID EncodeDecode( tBYTE *lpBuf, tINT len )
{
    tBYTE Mask[MAX_MASK] = {
        0x1F, 0x2E, 0x3D, 0x4C, 0x5B, 0x6A, 0x79, 0x88, 0x97, 0xA6, 0xB5, 0xC4, 0xD3, 0xE2, 0xF1, 0xFF };
    tINT i;

    // NULL인 경우에도 해야 한다. 
    for(i = 0; i < len ; i++) {
        lpBuf[i] ^= Mask[i&0x000F];
    }
}

tINT RealLen(tHCHAR *lpBuf)
{
        tHCHAR *p = lpBuf;
        tINT nCount = 0 ;

        while (*p) {
                if (*p & 0x80) {
                        nCount ++;
                }
                nCount ++;
		p++;
        }
        return nCount;
}

PUBLIC tINT RegistFullSig(SigProcP SigProc)
{
#if defined(_OLD)
        if(RegistOneSig(SIGINT, (SigProcP)SIG_IGN /*SigProc*/) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
#endif
        if(RegistOneSig(SIGABRT, (SigProcP)SigProc) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
        if(RegistOneSig(SIGSEGV, SigProc) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
        if(RegistOneSig(SIGPIPE, (SigProcP)SIG_IGN) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
#if defined(_OLD)
        if(RegistOneSig(SIGALRM, (SigProcP)SIG_IGN) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
#endif
#if defined(_OLD)
        if(RegistOneSig(SIGSTOP, (SigProcP)SIG_IGN) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
#endif
        if(RegistOneSig(SIGHUP, (SigProcP)SIG_IGN) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
        if(RegistOneSig(SIGQUIT, SigProc) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
        if(RegistOneSig(SIGTERM, (SigProcP)SigProc) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
        if(RegistOneSig(SIGUSR1, (SigProcP)SIG_IGN) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
        if(RegistOneSig(SIGUSR2, (SigProcP)SIG_IGN) == (tVOID *)SIG_ERR) {
		return FALSE;
        }
        return TRUE;
}

PUBLIC tVOID *RegistOneSig( tINT nSigNum,  tVOID (*SigProc)( tINT nSigNum ))
{
        struct sigaction act, oact;

        act.sa_handler = SigProc;
        sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (nSigNum == SIGALRM) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */
#endif
	}
	else {
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
		
        sigaddset(&act.sa_mask, nSigNum);

#if !defined(_FREEBSD)
        pthread_sigmask(SIG_BLOCK, &act.sa_mask, NULL);
#endif

	act.sa_flags = 0;
	if (nSigNum == SIGALRM) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */
#endif
	}
	else {
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}

        if(sigaction(nSigNum, &act, &oact) < 0) {
#if !defined(_FREEBSD)
                pthread_sigmask(SIG_UNBLOCK, &act.sa_mask, NULL);
#endif
                return((tVOID *)SIG_ERR);
        }
#if !defined(_FREEBSD)
        pthread_sigmask(SIG_UNBLOCK, &act.sa_mask, NULL);
#endif
        return((tVOID *)oact.sa_handler);
}

PUBLIC tVOID *RegistOneSig1( tINT nSigNum,  tVOID (*SigProc)( tINT nSigNum ))
{
        struct sigaction act, oact;

        act.sa_handler = SigProc;
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, nSigNum);
#if !defined(_FREEBSD)
        pthread_sigmask(SIG_BLOCK, &act.sa_mask, NULL);
#endif
        act.sa_flags = SA_RESTART;
        if(sigaction(nSigNum, &act, &oact) < 0) {
#if !defined(_FREEBSD)
                pthread_sigmask(SIG_UNBLOCK, &act.sa_mask, NULL);
#endif
                return((tVOID *)SIG_ERR);
        }
#if !defined(_FREEBSD)
	pthread_sigmask(SIG_SETMASK, &act.sa_mask, NULL);
        pthread_sigmask(SIG_UNBLOCK, &act.sa_mask, NULL);
#endif


        return((tVOID *)oact.sa_handler);
}

#define SPACEMARK "___"
tVOID Under2Space(tCHAR *szStr)
{
	tCHAR *szTmp;
	tCHAR *p, *q;
	tINT len;

	szTmp = (tCHAR *)malloc(strlen(szStr)+1);
	if (szTmp == NULL) return ;

	p = szStr;

	szTmp[0] = '\0';
	while (*p) {
		q = strstr(p, SPACEMARK);
		if (q == NULL) {
			strcat(szTmp, p);
			break;
		}
		else {
			len = q - p;
			if (len) {
				*q = ' ';
				*(q+1) = '\0';
				strcat(szTmp, p);
				*q = '_';
				*(q+1) = '_';
			}
			else { // 같을 때.. 
				strcat(szTmp, " ");	
			}
			p = q + 3;
		}
	}
	strcpy(szStr, szTmp);
	free(szTmp);
}

tINT GetUrlFromFileName(tCHAR *ST_DIR, tCHAR *szUrlName, tCHAR *szFileName)
{
	tCHAR tmpBuf[1024];
	tCHAR tmpBuf1[1024];
	tINT  nPos[50];
	tINT  i, len, j;
	tCHAR *p;

	szUrlName[0] = '\0';
	if (ST_DIR != NULL) {
		strcpy(tmpBuf, szFileName + strlen(ST_DIR));
	}
	else {
		strcpy(tmpBuf, szFileName);
	}
	if (tmpBuf[0] == '/') strcpy(tmpBuf, tmpBuf+1); // normalize
	p = strstr(tmpBuf, "/http/");
	if (p == NULL) return FALSE;
	len = (tINT)(p - tmpBuf) + 1; // 끝에 있어야 한다. 

	j = 0;
	for ( i = 0 ; i < len ; i ++ ) {
		if (tmpBuf[i] & 0x80) {
			i++;
		}
		else if (tmpBuf[i] == '/') {
			nPos[j++] = i;
		}
	}
	if (j == 0) return FALSE;
	strcpy(szUrlName, "http://");
	for ( i = j - 1 ; i >= 0 ; i -- ) {
		if (i) {
			if (nPos[i] - nPos[i-1] - 1 <= 0) {
				szUrlName[0] = '\0';
				return FALSE;
			}
			strncpy(tmpBuf1, tmpBuf + nPos[i-1] + 1, nPos[i] - nPos[i-1] - 1);
			tmpBuf1[nPos[i] - nPos[i-1] - 1] = '\0';
			strcat(szUrlName, tmpBuf1);
			strcat(szUrlName, ".");
		}
		else {
			if (nPos[0] <= 0) {
				szUrlName[0] = '\0';
				return FALSE;
			}
			strncpy(tmpBuf1, tmpBuf, nPos[0]);
			tmpBuf1[nPos[0]] = '\0';
			strcat(szUrlName, tmpBuf1);
		}
	}	
	if (*(p + strlen("/http/"))) {
		strcat(szUrlName, p+strlen("/http"));
	}
	p = strstr(szUrlName, "/__.html");
	if (p && *(p+8) == '\0') {
		*(p+1) = '\0';
	}
	Under2Space(szUrlName);
	CvtCharFromHex(tmpBuf,szUrlName);
	strcpy(szUrlName, tmpBuf);
	return TRUE;
}

tINT GetFileNameFromUrl(tCHAR *ST_DIR, tCHAR *szFileName, tCHAR *szUrlName)
{
	tCHAR tmpBuf[1024];
	tCHAR tmpBuf1[1024];
	tCHAR str[1024];
	tCHAR dirName[1024];
	tCHAR *p;
	tINT i, j;

	
	if (strncasecmp(szUrlName, "http://", strlen("http://")) == 0) {
		strcpy(tmpBuf1, szUrlName + strlen("http://"));
	}
	else {
		strcpy(tmpBuf1, szUrlName);
	}
	p = strchr(tmpBuf1, '/');
	if (p == NULL) {
		strcpy(tmpBuf, "/");
	}
	else {
		strcpy(tmpBuf, p);
		*p = '\0';
	}
	strcpy(dirName, "/http");
	strcat(dirName, tmpBuf);
	strcpy(tmpBuf, dirName);

	j = 0;
	for ( i = 0 ; i < strlen(tmpBuf1) ; i ++ ) {
		if (tmpBuf1[i] == '.') {
			str[j] = '\0';
			if (j) {	
				sprintf(dirName, "/%s", str);
				strcat(dirName, tmpBuf);
				strcpy(tmpBuf, dirName);
			}
			j = 0;
		}
		else str[j++] = tmpBuf1[i];

	}
	if (j) {	
		str[j] = '\0';
		sprintf(dirName, "/%s", str);
		strcat(dirName, tmpBuf);
		strcpy(tmpBuf, dirName);
	}

	if (tmpBuf[strlen(tmpBuf)-1] == '/') {
		strcat(tmpBuf, "__.html");
	}
	strcpy(dirName, ST_DIR);
	strcat(dirName, tmpBuf);
	strcpy(tmpBuf, dirName);
	strcpy(szFileName, tmpBuf);
	return TRUE;
}

tINT GetFileDirFromUrl(tCHAR *ST_DIR, tCHAR *szFileName, tCHAR *szDirName, tCHAR *szUrlName)
{
	tCHAR tmpBuf[1024];
	tCHAR tmpBuf1[1024];
	tCHAR str[1024];
	tCHAR dirName[1024];
	tCHAR *p;
	tINT i, j;

	
	if (strncasecmp(szUrlName, "http://", strlen("http://")) == 0) {
		strcpy(tmpBuf1, szUrlName + strlen("http://"));
	}
	else {
		strcpy(tmpBuf1, szUrlName);
	}
	p = strchr(tmpBuf1, '/');
	if (p == NULL) {
		strcpy(tmpBuf, "/");
	}
	else {
		strcpy(tmpBuf, p);
		*p = '\0';
	}
	strcpy(dirName, "/http");
	strcat(dirName, tmpBuf);
	strcpy(tmpBuf, dirName);

	j = 0;
	for ( i = 0 ; i < strlen(tmpBuf1) ; i ++ ) {
		if (tmpBuf1[i] == '.') {
			str[j] = '\0';
			if (j) {	
				sprintf(dirName, "/%s", str);
				strcat(dirName, tmpBuf);
				strcpy(tmpBuf, dirName);
			}
			j = 0;
		}
		else str[j++] = tmpBuf1[i];

	}
	if (j) {	
		str[j] = '\0';
		sprintf(dirName, "/%s", str);
		strcat(dirName, tmpBuf);
		strcpy(tmpBuf, dirName);
	}

	if (tmpBuf[strlen(tmpBuf)-1] == '/') {
		strcat(tmpBuf, "__.html");
	}
	strcpy(dirName, ST_DIR);
	strcat(dirName, tmpBuf);
	strcpy(tmpBuf, dirName);
	strcpy(szFileName, tmpBuf);
	strcpy(szDirName, tmpBuf);

	for ( i = strlen(szDirName) - 1; i >= 0 ; i -- ) {
		if (szDirName[i] == '/') {
			szDirName[i+1] = '\0';
			strcpy(szFileName, szFileName+i+1);
			break;
		}
	}

	return TRUE;
}

/* Hex 코드로 만들어진 아스키 값을 Char 형의 변수로 바꿔준다. */
static tBOOL Hex2Char( tBYTE _upper, tBYTE _lower , tBYTE *result)
{
    register char result_char;
    tBYTE hex_upper, hex_lower;

    hex_upper = toupper(_upper);
    hex_lower = toupper(_lower);

    if (hex_upper >= 'A' && hex_upper <= 'F') {
	result_char = hex_upper - 'A' + 10;
    }
    else if (hex_upper >= '0' && hex_upper <= '9') {
	result_char = hex_upper - '0';
    }
    else {
	return FALSE;
    }
    /*result_char = ( hex_upper >= 'A' ? ( ( hex_upper & 0xdf ) - 'A' ) + 10 :
                    ( hex_upper - '0' ) );
    */
    result_char *= 16;

    if (hex_lower >= 'A' && hex_lower <= 'F') {
	result_char += (hex_lower - 'A' + 10);
    }
    else if (hex_lower >= '0' && hex_lower <= '9') {
	result_char += (hex_lower-'0');
    }
    else {
	return FALSE;
    }

    /*result_char += ( hex_lower >= 'A' ? ( ( hex_lower & 0xdf ) - 'A' ) + 10 :
                     ( hex_lower - '0' ) );
    return result_char;
    */
    *result = result_char;
    return TRUE;
}


/* 앞에서 언급한 형식으로 encode된 문자열을 처음의 상태로 바꿔주는 루틴 */
tVOID CvtCharFromHex( tCHAR *szStr, tCHAR *szHex )
{
#if defined(_OLD)
     tBYTE temp;
     tINT i, j = 0;

    for ( i = 0 ; i < strlen(szHex) ;  i ++ ) {
	if (szHex[i] & 0x80) {
		szStr[j++] = szHex[i++];	
		szStr[j++] = szHex[i];	
	}
	else if (szHex[i] == '+') {
		szStr[j++] = ' ';	
	}
	else if (szHex[i] == '%' && i != strlen(szHex) - 1 && szHex[i+1] != '%') {
		if (Hex2Char(szHex[i+1], szHex[i+2], &(szStr[j]))) {
			j++;
		}
		else {
			szStr[j++] = szHex[i];
			szStr[j++] = szHex[i+1];
			szStr[j++] = szHex[i+2];
		}
		i +=2;
	}
	else {
		szStr[j++] = szHex[i];
	}
    }
    szStr[j] = '\0';
#else
    HexStr2CharStr(szHex, szStr);
#endif
}

tVOID LowStr(tCHAR *szStr)
{
	tCHAR *p = szStr;

	while(*p) {
		if (*p & 0x80) 	
			p++;
		else if (isalpha((int)*p)) 
			*p = (char)tolower((int)*p);

		p++;
	}
}

tVOID UppStr(tCHAR *szStr)
{
	tCHAR *p = szStr;

	while(*p) {
		if (*p & 0x80) 	
			p++;
		else if (isalpha((int)*p)) 
			*p = toupper((int)*p);

		p++;
	}
}

#if defined(_OLD)
tBOOL NormalUrl(tCHAR *szURL, tCHAR *szNormalURL, tBOOL fExistHTTP)
{
	tCHAR szTmpUrl[256], *p;
	tINT  nLen, i;
	tBOOL fOKExt;
	tINT nSkip = 0;
	
	if (szURL[0] == '\0') {
		szNormalURL[0] = '\0';
		return TRUE;
	}

	if (fExistHTTP) {
		if (strncasecmp(szURL, "http://", strlen("http://")) == 0) {
			strcpy(szTmpUrl, szURL);
		}
		else {
			strcpy(szTmpUrl, "http://");
			strcat(szTmpUrl, szURL);
		}
		nSkip = 7;
	}
	else {
		if (strncasecmp(szURL, "http://", strlen("http://")) == 0) {
			strcpy(szTmpUrl, szURL + strlen("http://"));
		}
		else {
			strcpy(szTmpUrl, szURL);
		}
	}
	//LowStr(szTmpUrl);
	nLen = strlen(szTmpUrl);
	if (szTmpUrl[nLen-1] == '/') { // normalize된 url
		strcpy(szNormalURL, szTmpUrl);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			*p = '/';
		}
		return TRUE;
	}
	p = strchr(szTmpUrl+nSkip, '/');
	if (p == NULL) { // http://www.hnc.co.kr경우
		strcat(szTmpUrl, "/");
		strcpy(szNormalURL, szTmpUrl);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			*p = '/';
		}
		return TRUE;
	}

	

	fOKExt = FALSE;
	for ( i = nLen - 1 ; i >= 0 ; i -- ) {
		if (szTmpUrl[i] == '.') {
			fOKExt = TRUE;
		}
		else if (szTmpUrl[i] == '/') break;
	}
	if (i>0 && fOKExt) {  // http://www.hnc.co.kr/index.html 같은 경우
		if (strncasecmp(szTmpUrl+i+1, "index.html", strlen("index.html")) == 0){
			szTmpUrl[i+1] = '\0'; //없앰
		}
		else if (strncasecmp(szTmpUrl+i+1, "default.asp", strlen("default.asp")) == 0){
			szTmpUrl[i+1] = '\0'; //없앰
		}
		strcpy(szNormalURL, szTmpUrl);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			*p = '/';
		}
		return TRUE;
	}
	else if (i>0) { // http://www.hnc.co.kr/add 같은 경우
		strcat(szTmpUrl, "/");
		strcpy(szNormalURL, szTmpUrl);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			*p = '/';
		}
		return TRUE;
	}
	else { // 있을 수 없는 일
	//	printf("szURL : %s<br>\n", szURL);
		strcpy(szNormalURL, szURL);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			*p = '/';
		}
		return FALSE;
	}

	return TRUE;
}

tVOID NormalDir(tCHAR *szsrcDir, tCHAR *sztarDir)
{
        tCHAR tmpDir[1024];

	if (szsrcDir[0] == '\0') return;
        tmpDir[0] = '\0';
        if (szsrcDir[0] != '/') {
                strcpy(tmpDir, "/");
        }
        strcat(tmpDir, szsrcDir);

        if (tmpDir[strlen(tmpDir)-1] != '/') strcat(tmpDir, "/");

        strcpy(sztarDir, tmpDir);
}
#else
tVOID DelSpChr(tCHAR *szStr)
{
	tCHAR *p = szStr;
	tINT i, nLen;

	while (*p) {
		if (*p & 0x80) {
			break;
		}
		else if (*p == ' ' || *p == '\t') {
			if (*(p+1) == '\0') *p = '\0';
			else memcpy(p, p+1, strlen(p));
		}
		else break;
	}
	
	nLen = strlen(szStr);
	for ( i = nLen - 1 ; i >= 0 ; i -- ) {
		if (szStr[i] == ' ' || szStr[i] == '\t') szStr[i] = '\0';
		else break;
	}

}

tBOOL NormalUrl(tCHAR *szURL, tCHAR *szNormalURL, tBOOL fExistHTTP)
{
	tCHAR szTmpUrl[256], *p, szTmpUrl1[256];
	tINT  nLen, i;
	tBOOL fOKExt;
	tINT nSkip = 0;
	
	if (szURL[0] == '\0') {
		szNormalURL[0] = '\0';
		return TRUE;
	}

	strcpy(szTmpUrl1, szURL);
	DelSpChr(szTmpUrl1);

	if (fExistHTTP) {
		if (strncasecmp(szTmpUrl1, "http://", strlen("http://")) == 0) {
			strcpy(szTmpUrl, szTmpUrl1);
		}
		else {
			strcpy(szTmpUrl, "http://");
			strcat(szTmpUrl, szTmpUrl1);
		}
		nSkip = 7;
	}
	else {
		if (strncasecmp(szTmpUrl1, "http://", strlen("http://")) == 0) {
			strcpy(szTmpUrl, szTmpUrl1 + strlen("http://"));
		}
		else {
			strcpy(szTmpUrl, szTmpUrl1);
		}
	}
	//LowStr(szTmpUrl);
	nLen = strlen(szTmpUrl);
	if (szTmpUrl[nLen-1] == '/') { // normalize된 url
		strcpy(szNormalURL, szTmpUrl);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			*p = '/';
		}
		return TRUE;
	}
	p = strchr(szTmpUrl+nSkip, '/');
	if (p == NULL) { // http://www.hnc.co.kr경우
		strcat(szTmpUrl, "/");
		strcpy(szNormalURL, szTmpUrl);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			*p = '/';
		}
		return TRUE;
	}

	
	if (strchr(szTmpUrl, '/') && strchr(szTmpUrl, '?')) { // 이런 것이 있는 경우는  cgi 인자이니깐 없애지 않는다.
		strcpy(szNormalURL, szTmpUrl);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			*p = '/';
		}
		return TRUE;
	}

	fOKExt = FALSE;
	for ( i = nLen - 1 ; i >= 0 ; i -- ) {
		if (szTmpUrl[i] == '.' || szTmpUrl[i] == '?') {
			fOKExt = TRUE;
		}
		else if (szTmpUrl[i] == '/') break;
	}
	if (i>0 && fOKExt) {  // http://www.hnc.co.kr/index.html 같은 경우
		if (strcasecmp(szTmpUrl+i+1, "index.html") == 0){
			szTmpUrl[i+1] = '\0'; //없앰
		}
		else if (strcasecmp(szTmpUrl+i+1, "default.asp") == 0){
			szTmpUrl[i+1] = '\0'; //없앰
		}
/*
FUTURE
		else if (strncasecmp(szTmpUrl+i+1, "index.htm", strlen("index.htm")) == 0){
			szTmpUrl[i+1] = '\0'; //없앰
		}
		else if (strncasecmp(szTmpUrl+i+1, "main.asp", strlen("main.asp")) == 0){
			szTmpUrl[i+1] = '\0'; //없앰
		}
		else if (strncasecmp(szTmpUrl+i+1, "index.asp", strlen("index.asp")) == 0){
			szTmpUrl[i+1] = '\0'; //없앰
		}
*/
		strcpy(szNormalURL, szTmpUrl);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			if (strchr(szNormalURL, '?') == NULL) { // ? 없을 때만 /을 추가한다.
				*p = '/';
			}
		}
		return TRUE;
	}
	else if (i>0) { // http://www.hnc.co.kr/add 같은 경우
		strcat(szTmpUrl, "/");
		strcpy(szNormalURL, szTmpUrl);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			if (strchr(szNormalURL, '?') == NULL) { // ? 없을 때만 /을 추가한다.
				*p = '/';
			}
		}
		return TRUE;
	}
	else { // 있을 수 없는 일
	//	printf("szURL : %s<br>\n", szURL);
		strcpy(szNormalURL, szTmpUrl1);
		if ((p = strchr(szNormalURL, '/')))  {
			*p = '\0';
			LowStr(szNormalURL);
			if (strchr(szNormalURL, '?') == NULL) { // ? 없을 때만 /을 추가한다.
				*p = '/';
			}
		}
		return FALSE;
	}

	return TRUE;
}

tVOID NormalDir(tCHAR *szsrcDir, tCHAR *sztarDir)
{
        tCHAR tmpDir[1024];
        tCHAR tmpDir1[1024];

	if (szsrcDir[0] == '\0') return;

	strcpy(tmpDir1, szsrcDir);
	DelSpChr(tmpDir1);

        tmpDir[0] = '\0';
        if (tmpDir1[0] != '/') {
                strcpy(tmpDir, "/");
        }
        strcat(tmpDir, tmpDir1);

        if (tmpDir[strlen(tmpDir)-1] != '/') strcat(tmpDir, "/");

        strcpy(sztarDir, tmpDir);
}
#endif

void DelRet(char *str)
{
        int i = strlen(str) - 1;

        while (i>=0 && (str[i] == '\n' || str[i] == '\r' || str[i] == ' ')) {
                str[i] = '\0';
                i -- ;
        }
}

#define MAX_SPECIAL_LEN 30 

PUBLIC tBOOL DWCvtDirName(tCHAR *uszDirName, tCHAR *uszCvtName)
{
        tINT i, j;

        typedef struct tagSPECIALCHAR_FORMAT {
                tCHAR   uszOperator[MAX_WORD_LEN];
        } SPECIALCHAR_FORMAT, *LPSPECIALCHAR_FORMAT;

        PUBLIC SPECIALCHAR_FORMAT SpecialCharList = {
                {       '`',    '~',    '!',    '@',    '#',
                        '$',    '%',    '^',    '&',    '*',
                        '(',    ')',    '-',    '+',    '=',
                        '|',    '\\',   '{',    '}',    '[',
                        ':',    ';',    '\"',   '\'',   '<',
                        ',',    '>',    '.',    '?', ' ' 
                }
        };

        strcpy(uszCvtName, uszDirName);

        for (i=0; i<strlen(uszDirName); i++) {
                for (j=0; j<MAX_SPECIAL_LEN; j++) {
                        if (uszDirName[i] == SpecialCharList.uszOperator[j]) {
                                uszCvtName[i] = '_';
                                break;
                        }
                }
        }

        return (TRUE);
} /* end of CvtDirName() */

tVOID DWCvtDirNameLocal(tCHAR *srcDir, tCHAR *tarDir, tINT nOpt)
{
        tINT i, j = 0;

	if (srcDir[0] == '\0') {
		tarDir[0] = '\0';
		return;
	}

        for ( i = 0 ; i < strlen(srcDir) ; i ++ ) {

		if (i == 0 && nOpt == 0 && srcDir[0] == '/') continue; // skip
		if (i == 0 && nOpt == 1 && srcDir[0] == '>') {
			tarDir[j++] = '/';
		}
		
                if ( srcDir[i] & 0x80) {
                        tarDir[j++] = srcDir[i++];
                        tarDir[j++] = srcDir[i];
                }
                else if (srcDir[i] == '/') {
			if (nOpt == 0) {
                        	tarDir[j++] = '>';
			}
			else {
                        	tarDir[j++] = ';'; // 내부적으로 ;으로 처리된다. 
				//srcDir[i];
			}
                }
                else if (srcDir[i] == ';') {
			if (nOpt == 0) {
                        	tarDir[j++] = '/';
			}
			else {
                        	tarDir[j++] = srcDir[i];
			}
                }
                else if (srcDir[i] == '>') {
			if (nOpt) {
                        	tarDir[j++] = '/';
			}
			else {
                        	tarDir[j++] = srcDir[i];
			}
                }
                else {
                        tarDir[j++] = srcDir[i];
                }
        }
        tarDir[j] = '\0';
}

PUBLIC tVOID DWCvtDirSymbol(tCHAR *szDirName, tCHAR cSymbol)
{
	tCHAR *p = szDirName;

	while (*p) {
		if (*p & 0x80) {
			p++;
		}
		else if (*p == '/') {
			*p = cSymbol;
		}
		p++;
	}
}

//roboting후에 저장된 디렉토리 구조에서 url을 뽑아냄
PUBLIC tVOID GetUrlFromDir(tCHAR *szUrl, tCHAR *szDir)
{
        tCHAR *p;
        tCHAR buffer[MAX_PATH*2];
        tCHAR szCabinet[20][MAX_PATH/2];
        tCHAR *delims = { "/" };
        tINT idx = 0, i = 0, num = 0;

        strcpy(buffer, szDir);
        printf( "%s\n", buffer );
        p = strtok( buffer, delims );
        while( p != NULL ) {
                strcpy(szCabinet[num], p);
                num++;
                p = strtok( NULL, delims );
        }

        for(i=0; i<num; i++) {
                if(strcmp(szCabinet[i], "http") == 0) {
                        idx = i;
                        break;
                }
        }
        buffer[0] = '\0';
        strcpy(buffer , "http://");
        for(i=idx-1; i>0; i--) {
                strcat(buffer, szCabinet[i]);
                if(FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"com") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"kr") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"net") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"org") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"edu") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"cc") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"nu") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"hk") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"sg") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"jp") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"au") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"to") == 0 ||
                   FullStrCaseCmp((tBYTE*)szCabinet[i], (tBYTE*)"ca") == 0 ) break;
                strcat(buffer, ".");
        }
        for(i=idx+1; i<num; i++) {
                strcat(buffer, "/");
                strcat(buffer, szCabinet[i]);
        }
        strcpy(szUrl, buffer);
}

PUBLIC tINT StrCaseCmp(tBYTE *c1, tBYTE *c2, tINT nLen)
{
	tBYTE *p1 = c1;
	tBYTE *p2 = c2;
	tINT i;
	
	for ( i = 0 ; i < nLen ; i ++ ) {
		if (*p1 == '\0') break;
		if (*p2 == '\0') break;
		if (tolower((tINT)*p1) != tolower((tINT)*p2)) return (tolower((tINT)*p1)-tolower((tINT)*p2));	
		p1++;
		p2++;
	}
	if (i >= nLen) return 0;

	return (tolower((tINT)(*p1))-tolower((tINT)*p2));	
}

PUBLIC tINT FullStrCaseCmp(tBYTE *c1, tBYTE *c2)
{
	tBYTE *p1 = c1;
	tBYTE *p2 = c2;
	tINT i, nLen = strlen((const char *)c2);
	
	for ( i = 0 ; i < nLen ; i ++ ) {
		if (*p1 == '\0') break;
		if (*p2 == '\0') break;
		if (tolower((tINT)*p1) != tolower((tINT)*p2)) return (tolower((tINT)*p1)-tolower((tINT)*p2));	
		p1++;
		p2++;
	}
	if (i >= nLen && *p1 == '\0') return 0;

	return (tolower((tINT)(*p1))-tolower((tINT)*p2));	
}

PUBLIC tCHAR *StrCaseStr( tBYTE * c1, tBYTE * c2 )
{
        tBYTE *d1;

        d1 = c1;

        while(*d1 != '\0') {
                while((tolower((tINT)*d1) != tolower((tINT)*c2)) && *d1 != '\0') {
			if (*d1 & 0x80) d1++;
                        d1++;
                }

                if(*d1 == '\0') {
                        return(NULL);
                }
                if(StrCaseCmp(d1, c2, strlen((const char *)c2)) == 0) {
                        return((tCHAR*)d1);
                }else {
			if (*d1 & 0x80) d1++;
                        d1++;
                }
        }

        return(NULL);
}/* end of StrCaseStr() */

tVOID MaxStr2Dot(tCHAR *szSrc, tCHAR *szTar, tINT nLen)
{
        int i, j =0;

        if (strlen(szSrc) <= nLen) {
                strcpy(szTar, szSrc);
                return;
        }

        for ( i = 0 ; i < nLen ; i ++ ) {
                if (szSrc[i] & 0x80) {
                        if (i >= nLen-1) {
                                break;
                        }
                        szTar[j++] = szSrc[i++];
                        szTar[j++] = szSrc[i];
                }
                else {
                        szTar[j++] = szSrc[i];
                }
        }
        szTar[j] = '\0';
        strcat(szTar, "..");

} /* end of MaxStr2Dot() */

tVOID Nbsp2Space(tCHAR *szSrc, tCHAR *szTar)
{
	tINT i, j, nLen;

	nLen = strlen(szSrc);
	j = 0;

	for(i=0; i<nLen; i++) {
		if (szSrc[i] == '&') {
			if (strncasecmp(szSrc+(i+1), "nbsp;", 5) == 0) {
				szTar[j++] = ' ';
				i += 5;
			}
			else {
				szTar[j++] = szSrc[i];
			}
		} else {
			szTar[j++] = szSrc[i];
		}
	}
	szTar[j] = '\0';
} /* end of Nbsp2Space() */

tLONG ltell( tINT handle )
{
        return(lseek(handle, 0L, SEEK_CUR));
}

tINT llock( tINT handle, tINT operation )
{
        struct flock flock;

        if(operation & DWLOCK_UN) {
                flock.l_type = F_UNLCK;
                flock.l_whence = SEEK_SET;
                flock.l_start = flock.l_len = 0L;
        }
	else if(operation & DWLOCK_EX) {
                flock.l_type = F_WRLCK;
                flock.l_whence = SEEK_SET;
                flock.l_start = flock.l_len = 0L;
        }else {
                return(-1);
        }

        return(fcntl(handle, F_SETLK, &flock));
}

void SetLock (int fd)
{
    llock(fd,DWLOCK_EX);
}

void UnsetLock (int fd)
{
    llock(fd,DWLOCK_UN);
}

void SetFLock(FILE *fp)
{
        SetLock(fileno(fp));
}

void UnsetFLock(FILE *fp)
{
        UnsetLock(fileno(fp));
}


PUBLIC tBOOL InitBinStr(BINSTR_FORMAT *lpBinStr, tINT nCount)
{
	lpBinStr->nMem = nCount;
	lpBinStr->nUsed = 0;

#if defined(_MEM_CHK)
	lpBinStr->lpMem = (tBYTE *)DWmalloc(nCount);
#else
	lpBinStr->lpMem = (tBYTE *)malloc(nCount);
#endif
	if (lpBinStr->lpMem == NULL) return FALSE;

	return TRUE;
} 

PUBLIC tBOOL CloseBinStr(BINSTR_FORMAT *lpBinStr)
{
	if( lpBinStr  && lpBinStr->lpMem ) {
#if defined(_MEM_CHK)
		DWfree(lpBinStr->lpMem, lpBinStr->nMem);
#else
		free(lpBinStr->lpMem);
#endif
		lpBinStr->nMem = 0;
		lpBinStr->nUsed = 0;
		lpBinStr->lpMem = NULL;
	}
	return TRUE;
} 

PUBLIC tBOOL AddBinStr(BINSTR_FORMAT *lpBinStr, tBYTE *lpBuf, tINT nLen)
{
	tINT nSize, nRemain;

	if (nLen == 0 || lpBinStr == NULL || lpBinStr->lpMem == NULL) {
		return FALSE;
	}

	nRemain = lpBinStr->nMem - lpBinStr->nUsed;

	nSize = nLen - nRemain + 1;
	if (nSize > 0) {
		nSize = lpBinStr->nMem + nSize;
#if defined(_MEM_CHK)
		lpBinStr->lpMem = (tBYTE*) DWrealloc(lpBinStr->lpMem, nSize, lpBinStr->nMem);
#else
		lpBinStr->lpMem = (tBYTE*) realloc(lpBinStr->lpMem, nSize);
#endif
		if (lpBinStr->lpMem == NULL) {
			return FALSE;
		}
		lpBinStr->nMem = nSize;
	}
	memcpy(lpBinStr->lpMem + lpBinStr->nUsed, lpBuf, nLen);
	lpBinStr->nUsed += nLen;
	return TRUE;
}

tINT InitClientSocket(tCHAR *szININame, tCHAR *szSection)
{
        tCHAR szIP[100], szPORT[10];
        tINT  nPort;
        tCHAR *StreamBuf;
        tINT  nStreamBuf;
	tINT  nSocketId;

        if (GetProfileStrEx(szININame, szSection, "SERVERIP", szIP, 100, 1) == FALSE) {
                printf("Server IP not found %s\n", szININame);
                return -1;
        }
        if (GetProfileStrEx(szININame, szSection, "SERVERPORT", szPORT, 10, 1) == FALSE) {
                printf("Server PORT not found %s\n", szININame);
                return -1;
        }
        nPort = atoi(szPORT);

        nSocketId = SocketClientEx( szIP, nPort, 3 );
        if (nSocketId == -1) {
         //       printf("%s:%d connect failure.\n", szIP, nPort);
                return -1;
        }

	StreamBuf = NULL;
        if (SocketReadStreamByMem( nSocketId, (tVOID **)(&StreamBuf), &nStreamBuf, 2 ) == FALSE) {
		if (StreamBuf) free(StreamBuf);
		close(nSocketId);
		return -1;
	}
        if (strcmp(StreamBuf, "THREAD_OK") != 0) {
		if (StreamBuf) free(StreamBuf);
		close(nSocketId);
                return -1;
        }

        free(StreamBuf);

        return nSocketId;
}

tINT InitClientSocketNoAck(tCHAR *szININame, tCHAR *szSection)
{
        tCHAR szIP[100], szPORT[10];
        tINT  nPort;
        tCHAR *StreamBuf;
        tINT  nStreamBuf;
	tINT  nSocketId;

        if (GetProfileStrEx(szININame, szSection, "SERVERIP", szIP, 100, 1) == FALSE) {
                printf("Server IP not found %s\n", szININame);
                return -1;
        }
        if (GetProfileStrEx(szININame, szSection, "SERVERPORT", szPORT, 10, 1) == FALSE) {
                printf("Server PORT not found %s\n", szININame);
                return -1;
        }
        nPort = atoi(szPORT);

        nSocketId = SocketClientEx( szIP, nPort, 3 );
        if (nSocketId == -1) {
         //       printf("%s:%d connect failure.\n", szIP, nPort);
                return -1;
        }

        return nSocketId;
}

tINT InitClientSocket2(tCHAR *szIP, tINT nPort)
{
	tCHAR *StreamBuf;
	tINT  nStreamBuf;
	tINT  nSocketId;

	nSocketId = SocketClientEx( szIP, nPort, 3 );
	if (nSocketId == -1) {
		//       printf("%s:%d connect failure.\n", szIP, nPort);
		return -1;
	}

	StreamBuf = NULL;

	if (SocketReadStreamByMem( nSocketId, (tVOID **)(&StreamBuf), &nStreamBuf, 2 ) == FALSE) {
		if (StreamBuf) free(StreamBuf);
		close(nSocketId);
		return -1;
	}
	if (strcmp(StreamBuf, "THREAD_OK") != 0) {
		if (StreamBuf) free(StreamBuf);
		close(nSocketId);
		return -1;
	}

	free(StreamBuf);

	return nSocketId;
}

tINT InitClientSocket2NoAck(tCHAR *szIP, tINT nPort)
{
	tCHAR *StreamBuf;
	tINT  nStreamBuf;
	tINT  nSocketId;

	nSocketId = SocketClientEx( szIP, nPort, 3 );
	if (nSocketId == -1) {
		//       printf("%s:%d connect failure.\n", szIP, nPort);
		return -1;
	}

	StreamBuf = NULL;
	if (SocketReadStreamByMem( nSocketId, (tVOID **)(&StreamBuf), &nStreamBuf, 2 ) == FALSE) {
		if (StreamBuf) free(StreamBuf);
		close(nSocketId);
		return -1;
	}
	if (strcmp(StreamBuf, "THREAD_OK") != 0) {
		if (StreamBuf) free(StreamBuf);
		close(nSocketId);
		return -1;
	}

	free(StreamBuf);

	return nSocketId;
}

tINT InitClientSocketEx(tCHAR *szININame, tCHAR *szSection, tINT nTimeout)
{
        tCHAR szIP[100], szPORT[10];
        tINT  nPort;
        tCHAR *StreamBuf;
        tINT  nStreamBuf;
	tINT  nSocketId;
	tCHAR szSectIP[20];
	tCHAR szSectPORT[20];
	tINT nCountFail = 0;

	while (1) {
		if (nCountFail == 0) {
			strcpy(szSectIP, "SERVERIP");
			strcpy(szSectPORT, "SERVERPORT");
		}
		else {
			sprintf(szSectIP, "SERVERIP%d", nCountFail);
			sprintf(szSectPORT, "SERVERPORT%d", nCountFail);
		}	
        	if (GetProfileStrEx(szININame, szSection, szSectIP, szIP, 100, 1) == FALSE) {
                	if (nCountFail == 0) printf("Server IP not found %s\n", szININame);
                	return -1;
        	}
        	if (GetProfileStrEx(szININame, szSection, szSectPORT, szPORT, 10, 1) == FALSE) {
                	if (nCountFail == 0) printf("Server PORT not found %s\n", szININame);
                	return -1;
        	}
        	nPort = atoi(szPORT);

       		nSocketId = SocketClientEx( szIP, nPort , nTimeout);
	
        	if (nSocketId == -1) {
			nCountFail ++;
			continue;
		}
		StreamBuf = NULL;
        	if (SocketReadStreamByMem( nSocketId, (tVOID **)(&StreamBuf), &nStreamBuf, 2 ) == FALSE) {
			if (StreamBuf) free(StreamBuf);
			close(nSocketId);
			nCountFail ++;
			continue;
		}
        	if (strcmp(StreamBuf, "THREAD_OK") != 0) {
			if (StreamBuf) free(StreamBuf);
			close(nSocketId);
			nCountFail ++;
			continue;
        	}

		if (StreamBuf) free(StreamBuf);
		break;
        }
       	return nSocketId;

}

tBOOL SocketWriteStreamBinStr(tINT nSockId, BINSTR_FORMAT *BinStr, tINT nTimeOut)
{
        return (SocketWriteStream(nSockId, BinStr->lpMem, BinStr->nUsed, nTimeOut));
}

unsigned char Hex2Char2(char HexUp, char HexLow)
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
	unsigned char *p = (unsigned char *)HexStr, *q = (unsigned char *)CharStr;


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
				*q = Hex2Char2(*p, *(p+1));
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
//        단, / 제외 
//
//        1 -> 모두 hex코드로 바꾼다.
//
//        2 -> 특수문자만 hex code로 바꾼다.
//        3 -> 기본 default와 같은데 /도 바꾼다.
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
			if (nOpt == 3 && CharStr[i] == '/') {
				sprintf(tmpStr , "%%%02x", (unsigned char)CharStr[i]);
                       		strcat(HexStr, tmpStr);
			}
			else if (nOpt == 1) {
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


tVOID RTrim( tCHAR *str )
{
        tINT i;

        for ( i = strlen(str) - 1 ; i >= 0 ; i -- ) {
                if (str[i] == ' ' || str[i] == '\t') {
                        str[i] = '\0';
                        continue;
                }
                break;
        }
}

tVOID LTrim( tCHAR *str )
{
        tINT i;
        tINT nLen = strlen(str);

        for ( i = 0 ; i < nLen ; i ++ ) {
		if (str[i] & 0x80) {
			break;
		}
			
                if (str[i] == ' ' || str[i] == '\t') continue;
                break;
        }
        if (i < nLen) {
                strcpy(str, str+i);
        }
        else {
                str[0] = '\0';
        }
}

tVOID Trim( tCHAR *str )
{
        tINT i = 0;

	while (str[i]) {
		if (str[i] & 0x80) {
			i+=2;
			continue;
		}
		if (str[i+1] == '\0') break;
                if (str[i] == ' ' || str[i] == '\t') {
			if (str[i+1] == ' ' || str[i+1] == '\t') {
				strcpy(str+i, str+i+1);
				continue;
			}
		}
		i++;
        }
}

tVOID DelChar( tCHAR *str, tCHAR Ch)
{
        tINT i = 0;

	while (str[i]) {
		if (str[i] & 0x80) {
			i+=2;
			continue;
		}
                if (str[i] == Ch) {
			if (str[i+1] == '\0') {
				str[i] = '\0';
			}
			else {
				strcpy(str+i, str+i+1);
			}
			continue;
		}
		i++;
        }
}

tVOID RedirectURL(tCHAR *szUrl, tBOOL bJavascript)
{
	if (bJavascript) {
		printf("<script language=\"JavaScript\">\n");
		printf("<!--\n");
		printf("window.location.replace(\"%s\");\n", szUrl);
		printf("//-->\n"); 
		printf("</script>\n");
	}
	else {
               	printf("Location: %s%c%c",szUrl,10,10);

        	printf("This document has moved <A HREF=\"%s\">here</A>%c",szUrl,10);
	}
}

tINT RandomNum(tINT nLimitNum, tINT nSeed)
{
	srand((tINT)time(NULL) + (tINT)getpid() + nSeed);
	return(rand()%nLimitNum);	
}

tVOID SetRandomSeed(tVOID)
{
	srand((tINT)time(NULL) + (tINT)getpid());
}
