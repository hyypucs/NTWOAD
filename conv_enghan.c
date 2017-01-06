#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dwtype.h"
#include "hcode.h"
#include "conv_enghan.h"

typedef struct {
#if defined(_DW_LITTLE_ENDIAN)
	tHCHAR last : 5;
	tHCHAR mid : 5;
	tHCHAR st : 5;	
	tHCHAR bit : 1;		 
#else
	tHCHAR bit : 1;		 
	tHCHAR st : 5;	
	tHCHAR mid : 5;
	tHCHAR last : 5;
#endif
}HANCHAR_FORMAT;

typedef struct {
#if defined(_DW_LITTLE_ENDIAN)
	tBYTE two;
	tBYTE one;
#else
	tBYTE one;
	tBYTE two;
#endif
}HANBYTE_FORMAT;

typedef union {
	HANCHAR_FORMAT a;
	HANBYTE_FORMAT b;
	tHCHAR nCode;
}HANGUL_FORMAT;


tINT ChoSung[26*2] = {
// 소문자 영역
8, //a
0, // b ㅠ
16, // c ㅊ
13, // d ㅇ
5, // e ㄷ
7, // f ㄹ
20, // g ㅎ
0, // h ㅗ
0, // i ㅑ
0, // j ㅓ
0, // k ㅏ
0, // l ㅣ
0, // m ㅡ
0, // ㅜ
0, // o ㅐ
0, // p ㅔ
9, // q ㅂ
2, // r ㄱ
4, // s ㄴ
11, // t ㅅ
0, // u ㅕ
19, // v ㅍ
14, // w ㅈ
18, // x ㅌ
0, // y ㅛ
17, // z ㅋ
// 대문자 영역
8, //a ㅁ 
0, // b ㅠ
16, // c ㅊ
13, // d ㅇ
6, // e ㄸ
7, // f ㄹ
20, // g ㅎ
0, // h ㅗ
0, // i ㅑ
0, // j ㅓ
0, // k ㅏ
0, // l ㅣ
0, // m ㅡ
0, // ㅜ
0, // o ㅒ
0, // p ㅖ
10, // q ㅃ
3, // r ㄲ
4, // s ㄴ
12, // t ㅆ
0, // u ㅕ
19, // v ㅍ
15, // w ㅉ
18, // x ㅌ
0, // y ㅛ
17 // z ㅋ
};

tINT JunSung[26*2] = {
// 소문자 영역
0, //a ㅁ
26, // b ㅠ
0, // c ㅊ
0, // d ㅇ
0, // e ㄷ
0, // f ㄹ
0, // g ㅎ
13, // h ㅗ
5, // i ㅑ
7, // j ㅓ
3, // k ㅏ
29, // l ㅣ
27, // m ㅡ
20, // ㅜ
4, // o ㅐ
10, // p ㅔ
0, // q ㅂ
0, // r ㄱ
0, // s ㄴ
0, // t ㅅ
11, // u ㅕ
0, // v ㅍ
0, // w ㅈ
0, // x ㅌ
19, // y ㅛ
0, // z ㅋ
// 대문자 영역
0, //a ㅁ 
26, // b ㅠ
0, // c ㅊ
0, // d ㅇ
0, // e ㄸ
0, // f ㄹ
0, // g ㅎ
13, // h ㅗ
5, // i ㅑ
7, // j ㅓ
8, // k ㅏ
29, // l ㅣ
27, // m ㅡ
20, // ㅜ
6, // o ㅒ
12, // p ㅖ
0, // q ㅃ
0, // r ㄲ
0, // s ㄴ
0, // t ㅆ
11, // u ㅕ
0, // v ㅍ
0, // w ㅉ
0, // x ㅌ
19, // y ㅛ
0 // z ㅋ
};

tINT JongSung[26*2] = {
// 소문자 영역
17, //a ㅁ 
0, // b ㅠ
25, // c ㅊ
23, // d ㅇ
8, // e ㄷ
9, // f ㄹ
29, // g ㅎ
0, // h ㅗ
0, // i ㅑ
0, // j ㅓ
0, // k ㅏ
0, // l ㅣ
0, // m ㅡ
0, // ㅜ
0, // o ㅐ
0, // p ㅔ
19, // q ㅂ
2, // r ㄱ
5, // s ㄴ
21, // t ㅅ
0, // u ㅕ
28, // v ㅍ
24, // w ㅈ
27, // x ㅌ
0, // y ㅛ
26, // z ㅋ
// 대문자 영역
17, //a ㅁ 
0, // b ㅠ
25, // c ㅊ
23, // d ㅇ
0, // e ㄸ
9, // f ㄹ
29, // g ㅎ
0, // h ㅗ
0, // i ㅑ
0, // j ㅓ
0, // k ㅏ
0, // l ㅣ
0, // m ㅡ
0, // ㅜ
0, // o ㅐ
0, // p ㅔ
0, // q ㅃ
3, // r ㄲ
5, // s ㄴ
22, // t ㅆ
0, // u ㅕ
28, // v ㅍ
0, // w ㅈ
27, // x ㅌ
0, // y ㅛ
26 // z ㅋ
};

tHCHAR GetChFromSungTable(tCHAR szChar, tINT nOpt);
//tBOOL ConvertEngHan(tCHAR *szEng, tCHAR *szHan, tINT nLimitHan);

#if defined(_ONE_EXEC)
main(int argc, char *argv[])
{
	tCHAR szTmp[100], szTmp1[100], szTmp2[100];
	while (1) {
		printf("in > ");
		gets(szTmp);
		if (strcmp(szTmp, "quit") == 0) break;
		if (ConvertEngHan(szTmp, szTmp1, 0) == TRUE) {
			KSSMStr2KSStr(szTmp1, szTmp2);
			printf("%s -> %s\n", szTmp, szTmp2);
		}
		else {
			printf("not found : %s\n", szTmp);
		}
	}
/*
	HANGUL_FORMAT test;

	
	strcpy(szTmp, "가");
	KSStr2KSSMStr(szTmp, szTmp1);

	test.b.one = szTmp1[0];
	test.b.two = szTmp1[1];

	printf("%c%c\n", test.b.one, test.b.two);
	printf("one : %x, two : %x\n", (tINT)test.b.one, (tINT)test.b.two);
	printf("nCode : %08x\n", test.nCode);
	printf("bit : %x, st : %x, mid : %x, last : %x\n", (tBYTE)test.a.bit, (tBYTE)test.a.st, (tBYTE)test.a.mid, (tBYTE)test.a.last);
*/
	
	return 0;
}
#endif

tHCHAR GetChFromSungTable(tCHAR szChar, tINT nOpt)
{
	if (szChar >= 'a' && szChar <= 'z') {
		if (nOpt == 0) {
			return(ChoSung[szChar - 'a']);
		}
		else if (nOpt == 1) {
			return(JunSung[szChar - 'a']);
		}
		else {
			return(JongSung[szChar - 'a']);
		}
	}
	else { // 대문자일 때..
		if (nOpt == 0) {
			return(ChoSung[szChar -'A'+'z' - 'a' +1]);
		}
		else if (nOpt == 1) {
			return(JunSung[szChar -'A'+'z' - 'a' +1]);
		}
		else {
			return(JongSung[szChar -'A'+'z' - 'a' +1]);
		}
	}
	return 0;
}

// nLimitHan 글자 순..
tBOOL ConvertEngHan(tCHAR *szEng, tCHAR *szHan, tINT nLimitHan)
{
	tINT i;
	HANGUL_FORMAT tmpHan;
	tINT j = 0;
	tINT nState = 0;
	tINT nLen = strlen(szEng);
	tHCHAR nCh;
	tBOOL bRetVal = FALSE;

	for ( i = 0 ; i < nLen ; i ++ ) {
		if (szEng[i] & 0x80) {
			szHan[0] = '\0';
			return FALSE;
		}
	}

	tmpHan.nCode = 0x0000;
	tmpHan.a.bit = 1;
	for ( i = 0 ; i < nLen ; i ++ ) {
		if (!isalpha(szEng[i])) {
			if (tmpHan.a.st > 1 && tmpHan.a.mid > 2) {
				if (tmpHan.a.last == 0) tmpHan.a.last = 1;
				szHan[j++] = tmpHan.b.one;
				szHan[j++] = tmpHan.b.two;
				bRetVal = TRUE;
			}
			szHan[j++] = szEng[i];
			tmpHan.nCode = 0x0000;
			tmpHan.a.bit = 1;
			nState = 0;
			continue;
		}
		if (nState == 0) { // 초성을 입력받아야 하는데..
			nCh = GetChFromSungTable(szEng[i], 0);
			if (nCh == 0) { // 없다... 없을 경우에..
				if (tmpHan.a.last < 2) {
					szHan[0] = '\0';
					return FALSE;
				}
				nCh = GetChFromSungTable(szEng[i], 1); // 준성에서 찾아보자.
				if (nCh == 0) { // 없다.
					szHan[0] = '\0';
					return FALSE;
				}

				if (tmpHan.a.last == 4 || tmpHan.a.last == 6 || tmpHan.a.last == 7 || (tmpHan.a.last >= 10 && tmpHan.a.last <= 16) || tmpHan.a.last == 20 ) { // 복자음이라. 앞의 2개를 본다.
					if (i < 2) {
						szHan[0] = '\0';
						return FALSE;
					}

					nCh = GetChFromSungTable(szEng[i-2], 2);
					tmpHan.a.last = nCh;
					szHan[j++] = tmpHan.b.one;
                                	szHan[j++] = tmpHan.b.two;
					bRetVal = TRUE;
				} 
				else { // 앞에 한자만 보면 된다.
					if (i < 1) {
						szHan[0] = '\0';
						return FALSE;
					}	
					tmpHan.a.last = 1;
					szHan[j++] = tmpHan.b.one;
                                	szHan[j++] = tmpHan.b.two;
					bRetVal = TRUE;
				}
				tmpHan.nCode = 0x0000;
				tmpHan.a.bit = 1;
				nCh = GetChFromSungTable(szEng[i-1], 0);
				tmpHan.a.st = nCh;	
				nCh = GetChFromSungTable(szEng[i], 1); // 준성에서 찾아보자.
				tmpHan.a.mid = nCh;
				nState = 2;
				continue;
			}
			else if (tmpHan.a.last == 2 && nCh == 11) {
				tmpHan.a.last = 4;
				nState = 0;
				continue;
			}
			else if (tmpHan.a.last == 5 && (nCh == 14 || nCh == 20) ) {
				if (nCh == 14) {
					tmpHan.a.last = 6;
				}
				else {
					tmpHan.a.last = 7;
				}
				nState = 0;
				continue;
			}
			else if (tmpHan.a.last == 9 && ( nCh == 2 || nCh == 8 || nCh == 9 || nCh == 11 || nCh == 18 || nCh == 19 || nCh == 20)) { // 복자음을 만들 수 있다.
				if (nCh == 2) {
					tmpHan.a.last = 10;
				}
				else if (nCh == 8) {
					tmpHan.a.last = 11;
				}
				else if (nCh == 9) {
					tmpHan.a.last = 12;
				}
				else if (nCh == 11) {
					tmpHan.a.last = 13;
				}
				else if (nCh == 18) {
					tmpHan.a.last = 14;
				}
				else if (nCh == 19) {
					tmpHan.a.last = 15;
				}
				else if (nCh == 20) {
					tmpHan.a.last = 16;
				}
				else {
					szHan[0] = '\0';
                                        return FALSE;
				}
				nState = 0;
				continue;
			}
			else if (tmpHan.a.last == 19 && nCh == 11) {
				tmpHan.a.last = 20;
				nState = 0;
				continue;
			}
			else if (tmpHan.a.st > 1 && tmpHan.a.mid > 2) {
				if (tmpHan.a.last == 0)	tmpHan.a.last = 1;
				szHan[j++] = tmpHan.b.one;
                               	szHan[j++] = tmpHan.b.two;
				bRetVal = TRUE;
			}
			
			tmpHan.nCode = 0x0000;
			tmpHan.a.bit = 1;
			tmpHan.a.st = nCh;
			nState = 1;
		}
		else if (nState == 1) {
			nCh = GetChFromSungTable(szEng[i], 1);
			if (nCh == 0) {
				szHan[0] = '\0';
				return FALSE;
			}

			tmpHan.a.mid = nCh;
			nState = 2;
		}
		else if (nState == 2) {
			nCh = GetChFromSungTable(szEng[i], 2);
			if (nCh == 0) {
				nCh = GetChFromSungTable(szEng[i], 1); // 준성에서 찾는다.
				if (nCh == 0) {
					szHan[0] = '\0';
					return FALSE;
				}

				if (tmpHan.a.mid == 13 && nCh == 3) {
					tmpHan.a.mid = 14;
				}
				else if (tmpHan.a.mid == 13 && nCh == 4) {
					tmpHan.a.mid = 15;
				}
				else if (tmpHan.a.mid == 13 && nCh == 29) {
					tmpHan.a.mid = 18;
				}
				else if (tmpHan.a.mid == 20 && nCh == 7) {
					tmpHan.a.mid = 21;
				}
				else if (tmpHan.a.mid == 20 && nCh == 10) {
					tmpHan.a.mid = 22;
				}
				else if (tmpHan.a.mid == 20 && nCh == 29) {
					tmpHan.a.mid = 23;
				}
				else if (tmpHan.a.mid == 27 && nCh == 29) {
					tmpHan.a.mid = 28;
				}
				else {
					szHan[0] = '\0';
                                        return FALSE;
				}
				nState = 2;
				continue;
			}
			else {
				tmpHan.a.last = nCh;
				nState = 0;
			}
		}
	}
	if (tmpHan.a.st > 1 && tmpHan.a.mid > 2) {
        	if (tmpHan.a.last == 0) tmpHan.a.last = 1;
        	szHan[j++] = tmpHan.b.one;
		szHan[j++] = tmpHan.b.two;
		bRetVal = TRUE;
	}
	szHan[j] = '\0';
	if (nLimitHan == 0) return bRetVal;

	j = 0;
	//printf("szHan=>[%s]\n", szHan);
	for ( i = 0 ; i < (int)strlen(szHan) ; i ++ ) {
		if (szHan[i] & 0x80) {
			i++;
			j++;
		}
		else {
			if (j != 0 && j <= nLimitHan) {
				szHan[0] = '\0';
				return FALSE;	
			}
			j = 0;
		}
	}
	if (j != 0 && j <= nLimitHan) {
		szHan[0] = '\0';
		return FALSE;	
	}
	return bRetVal;
}

