/*
	crypt함수를 이용하여 패스워드와 같은 것을 다 encoding 시킨다.

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "dwcrypt.h"

extern char *crypt(const char *, const char *);

void dw_ap_to64(char *s, unsigned long v, int n);

void dw_ap_to64(char *s, unsigned long v, int n)
{
    static unsigned char itoa64[] =         /* 0 ... 63 => ASCII - 64 */
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    while (--n >= 0) {
        *s++ = itoa64[v&0x3f];
        v >>= 6;
    }
}

char *DWCrypt(char *szsrcStr, char *sztarStr)
{
	char salt[10];
	char *p;

	sztarStr[0] = '\0';

	srand((int) time((time_t *) NULL));
        dw_ap_to64(&salt[0], rand(), 8);
        salt[8] = '\0';

        p = crypt((const char *)szsrcStr, (const char *)salt);

	if (p == NULL || *p == '\0') {
		return NULL;
	}
	strcpy(sztarStr, p);
	return sztarStr;
}

//
// true/false
//
int DWChkValidCode(char *szStr, char *szCryptStr)
{
	char *p;

	p = crypt((const char *)szStr, (const char *)szCryptStr);

	if (p == NULL || strcmp(p, szCryptStr) != 0) {
		return 0;
	}	

	return 1;
}

