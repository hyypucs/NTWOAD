#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <iconv.h>

#include "dwtype.h"
#include "util.h"
#include "utf8_euckr.h"

tBOOL IsKs2ByteStr(tBYTE *szStr);
tBOOL IsAlphaNumStr(tBYTE *szInValue);
tBOOL OkUTF8Str(tBYTE *szStr);

tBOOL ConvertUtf8Euckr(tBYTE *szOrgValue, tBYTE *szRetValue)
{
	tCHAR *p;
	tCHAR szValue[4096];

	tCHAR *chkp;
        tCHAR *buf;
        tCHAR *sbuf;
        iconv_t cd;
        size_t ileft, oleft;
	tBOOL bRetVal = FALSE;


	szRetValue[0] = '\0';
	strcpy(szValue, (tCHAR *)szOrgValue);
	
	if (IsAlphaNumStr((tBYTE *)szValue)) return FALSE;
	if (OkUTF8Str((tBYTE *)szValue) == FALSE) {
		return FALSE;
	}

	// 변경하기
	
	buf = (tCHAR *)malloc(4096);
	sbuf = (tCHAR *)malloc(4096);

	memset(buf, 0, 4096);
	memset(sbuf, 0, 4096);

	strncpy(sbuf, (tCHAR *)szValue, 4095);
	sbuf[4095] = '\0';
		
	p = sbuf;
	// utf-8 check
	ileft = strlen(p);
	oleft = 4096;
	cd =  iconv_open((const char *)"EUC-KR", (const char *)"UTF-8");
	if (cd != (iconv_t)-1) {
		chkp = buf;
		if (iconv(cd, (char **)(&p), &ileft, &chkp, &oleft) == 0) {
			// ok utf-8
			strcpy((tCHAR *)szRetValue, buf);
			if (IsKs2ByteStr(szRetValue)) { // ks완성형일 때..
				bRetVal = TRUE;
			}
		}
		iconv_close(cd);
	}
	free(sbuf);
	free(buf);

	return bRetVal;
}

tBOOL IsAlphaNumStr(tBYTE *szInValue)
{
	tINT i;

	for ( i = 0 ; i < (int)strlen((tCHAR *)szInValue) ; i++ ) {
		if (szInValue[i] & 0x80) return FALSE;

		if (isalnum(szInValue[i])) continue;
		return FALSE;
	}
	return TRUE;
}

tBOOL IsKs2ByteStr(tBYTE *szStr)
{
	tHCHAR hangul_code;
	tBYTE first, second;
	tINT i;
	
	for ( i = 0 ; i < (int)strlen((tCHAR*)szStr) ; i ++ ) {

		if (!(szStr[i] & 0x80)) {
			continue;
		}

		first = szStr[i++];
		second = szStr[i];

		hangul_code = (first << 8) + second; /* 한글코드를 조합하기 */
      		/* 띄엄띄엄 띄워진 KS5061의 특수문자 코드인가? */
      		if((hangul_code > 0xa1a1 && hangul_code < 0xa1ff) ||
         		(hangul_code > 0xa2a0 && hangul_code < 0xa2e6) ||
         		(hangul_code > 0xa3a0 && hangul_code < 0xa3ff) ||
         		(hangul_code > 0xa4a0 && hangul_code < 0xa4ff) ||
         (hangul_code > 0xa5a0 && hangul_code < 0xa5f9) ||
         (hangul_code > 0xa6a0 && hangul_code < 0xa6e5) ||
         (hangul_code > 0xa7a0 && hangul_code < 0xa7f0) ||
         (hangul_code > 0xa8a0 && hangul_code < 0xa8ff) ||
         (hangul_code > 0xa9a0 && hangul_code < 0xa9ff) ||
         (hangul_code > 0xaaa0 && hangul_code < 0xaaf4) ||
         (hangul_code > 0xaba0 && hangul_code < 0xabf7) ||
         (hangul_code > 0xaca0 && hangul_code < 0xacc2) ||
         (hangul_code > 0xacd0 && hangul_code < 0xacf2) ||
         
        /* KS5061의 한글코드인가?*/
         (hangul_code > 0xb0a0 && hangul_code < 0xb0ff) ||
         (hangul_code > 0xb1a0 && hangul_code < 0xb1ff) ||
         (hangul_code > 0xb2a0 && hangul_code < 0xb2ff) ||
         (hangul_code > 0xb3a0 && hangul_code < 0xb3ff) ||
         (hangul_code > 0xb4a0 && hangul_code < 0xb4ff) ||
         (hangul_code > 0xb5a0 && hangul_code < 0xb5ff) ||
         (hangul_code > 0xb6a0 && hangul_code < 0xb6ff) ||
         (hangul_code > 0xb7a0 && hangul_code < 0xb7ff) ||
         (hangul_code > 0xb8a0 && hangul_code < 0xb8ff) ||
         (hangul_code > 0xb9a0 && hangul_code < 0xb9ff) ||
         (hangul_code > 0xbaa0 && hangul_code < 0xbaff) ||
         (hangul_code > 0xbba0 && hangul_code < 0xbbff) ||
         (hangul_code > 0xbca0 && hangul_code < 0xbcff) ||
         (hangul_code > 0xbda0 && hangul_code < 0xbdff) ||
         (hangul_code > 0xbea0 && hangul_code < 0xbeff) ||
         (hangul_code > 0xbfa0 && hangul_code < 0xbfff) ||
         
         (hangul_code > 0xc0a0 && hangul_code < 0xc0ff) ||
         (hangul_code > 0xc1a0 && hangul_code < 0xc1ff) ||
         (hangul_code > 0xc2a0 && hangul_code < 0xc2ff) ||
         (hangul_code > 0xc3a0 && hangul_code < 0xc3ff) ||
         (hangul_code > 0xc4a0 && hangul_code < 0xc4ff) ||
         (hangul_code > 0xc5a0 && hangul_code < 0xc5ff) ||
         (hangul_code > 0xc6a0 && hangul_code < 0xc6ff) ||
         (hangul_code > 0xc7a0 && hangul_code < 0xc7ff) ||
         (hangul_code > 0xc8a0 && hangul_code < 0xc8ff) ||

        /* KS5061의 한자코드인가? */
         (hangul_code > 0xcaa0 && hangul_code < 0xcaff) ||
         (hangul_code > 0xcba0 && hangul_code < 0xcbff) ||
         (hangul_code > 0xcca0 && hangul_code < 0xccff) ||
         (hangul_code > 0xcda0 && hangul_code < 0xcdff) ||
         (hangul_code > 0xcea0 && hangul_code < 0xceff) ||
         (hangul_code > 0xcfa0 && hangul_code < 0xcfff) ||
         
         (hangul_code > 0xd0a0 && hangul_code < 0xd0ff) ||
         (hangul_code > 0xd1a0 && hangul_code < 0xd1ff) ||
         (hangul_code > 0xd2a0 && hangul_code < 0xd2ff) ||
         (hangul_code > 0xd3a0 && hangul_code < 0xd3ff) ||
         (hangul_code > 0xd4a0 && hangul_code < 0xd4ff) ||
         (hangul_code > 0xd5a0 && hangul_code < 0xd5ff) ||
         (hangul_code > 0xd6a0 && hangul_code < 0xd6ff) ||
         (hangul_code > 0xd7a0 && hangul_code < 0xd7ff) ||
         (hangul_code > 0xd8a0 && hangul_code < 0xd8ff) ||
         (hangul_code > 0xd9a0 && hangul_code < 0xd9ff) ||
         (hangul_code > 0xdaa0 && hangul_code < 0xdaff) ||
         (hangul_code > 0xdba0 && hangul_code < 0xdbff) ||
         (hangul_code > 0xdca0 && hangul_code < 0xdcff) ||
         (hangul_code > 0xdda0 && hangul_code < 0xddff) ||
         (hangul_code > 0xdea0 && hangul_code < 0xdeff) ||
         (hangul_code > 0xdfa0 && hangul_code < 0xdfff) ||
         
         (hangul_code > 0xe0a0 && hangul_code < 0xe0ff) ||
         (hangul_code > 0xe1a0 && hangul_code < 0xe1ff) ||
         (hangul_code > 0xe2a0 && hangul_code < 0xe2ff) ||
         (hangul_code > 0xe3a0 && hangul_code < 0xe3ff) ||
         (hangul_code > 0xe4a0 && hangul_code < 0xe4ff) ||
         (hangul_code > 0xe5a0 && hangul_code < 0xe5ff) ||
         (hangul_code > 0xe6a0 && hangul_code < 0xe6ff) ||
         (hangul_code > 0xe7a0 && hangul_code < 0xe7ff) ||
         (hangul_code > 0xe8a0 && hangul_code < 0xe8ff) ||
         (hangul_code > 0xe9a0 && hangul_code < 0xe9ff) ||
         (hangul_code > 0xeaa0 && hangul_code < 0xeaff) ||
         (hangul_code > 0xeba0 && hangul_code < 0xebff) ||
         (hangul_code > 0xeca0 && hangul_code < 0xecff) ||
         (hangul_code > 0xeda0 && hangul_code < 0xedff) ||
         (hangul_code > 0xeea0 && hangul_code < 0xeeff) ||
         (hangul_code > 0xefa0 && hangul_code < 0xefff) ||
 
      	 (hangul_code > 0xf0a0 && hangul_code < 0xf0ff) ||
         (hangul_code > 0xf1a0 && hangul_code < 0xf1ff) ||
         (hangul_code > 0xf2a0 && hangul_code < 0xf2ff) ||
         (hangul_code > 0xf3a0 && hangul_code < 0xf3ff) ||
         (hangul_code > 0xf4a0 && hangul_code < 0xf4ff) ||
         (hangul_code > 0xf5a0 && hangul_code < 0xf5ff) ||
         (hangul_code > 0xf6a0 && hangul_code < 0xf6ff) ||
         (hangul_code > 0xf7a0 && hangul_code < 0xf7ff) ||
         (hangul_code > 0xf8a0 && hangul_code < 0xf8ff) ||
         (hangul_code > 0xf9a0 && hangul_code < 0xf9ff) ||
         (hangul_code > 0xfaa0 && hangul_code < 0xfaff) ||
         		(hangul_code > 0xfba0 && hangul_code < 0xfbff) ||
         		(hangul_code > 0xfca0 && hangul_code < 0xfcff) ||
         		(hangul_code > 0xfda0 && hangul_code < 0xfdff) ) {
			continue;
		}
		return FALSE;
	
	}

	return TRUE;
}

static const tINT utf8_lengths[16]=
{
1,1,1,1,1,1,1,1,        /* 0000 to 0111 : 1 byte (plain ASCII) */
0,0,0,0,                /* 1000 to 1011 : not valid */
2,2,                    /* 1100, 1101 : 2 bytes */
3,                      /* 1110 : 3 bytes */
4                       /* 1111 :4 bytes */
};


tBOOL OkUTF8Str(tBYTE *szStr)
{
	tINT byte_length = strlen((tCHAR *)szStr);
	tBYTE *source = szStr;

	tINT wchar_length=0;
	tINT code_size;
	tBYTE byte;

	while(byte_length > 0) {
		byte=*source;
         
		/* UTF-16 can't encode 5-byte and 6-byte sequences, so maximum value
		for first byte is 11110111. Use lookup table to determine sequence 
		length based on upper 4 bits of first byte */
		if (!(byte & 0x80)) { // 2byte 문자가 아니면 continue;
			source ++;
			byte_length --;
			continue;
		}
		if ((byte <= 0xF7) && (0 != (code_size=utf8_lengths[ byte >> 4]))) {
			/* 1 sequence == 1 character */
			wchar_length++;
			if(code_size==4) wchar_length++;
			source+=code_size;        /* increment pointer */
			byte_length-=code_size;   /* decrement counter*/
		}
		else {
			/* 
			unlike UTF8_mbslen_bytes, we ignore the invalid characters.
			we only report the number of valid characters we have encountered
			to match the Windows behavior.
			*/
			//source ++;
			//byte_length --;
			return FALSE;
		}
	}
	if (byte_length == 0) return TRUE;
	return FALSE;
}
