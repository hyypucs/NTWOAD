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
// �ҹ��� ����
8, //a
0, // b ��
16, // c ��
13, // d ��
5, // e ��
7, // f ��
20, // g ��
0, // h ��
0, // i ��
0, // j ��
0, // k ��
0, // l ��
0, // m ��
0, // ��
0, // o ��
0, // p ��
9, // q ��
2, // r ��
4, // s ��
11, // t ��
0, // u ��
19, // v ��
14, // w ��
18, // x ��
0, // y ��
17, // z ��
// �빮�� ����
8, //a �� 
0, // b ��
16, // c ��
13, // d ��
6, // e ��
7, // f ��
20, // g ��
0, // h ��
0, // i ��
0, // j ��
0, // k ��
0, // l ��
0, // m ��
0, // ��
0, // o ��
0, // p ��
10, // q ��
3, // r ��
4, // s ��
12, // t ��
0, // u ��
19, // v ��
15, // w ��
18, // x ��
0, // y ��
17 // z ��
};

tINT JunSung[26*2] = {
// �ҹ��� ����
0, //a ��
26, // b ��
0, // c ��
0, // d ��
0, // e ��
0, // f ��
0, // g ��
13, // h ��
5, // i ��
7, // j ��
3, // k ��
29, // l ��
27, // m ��
20, // ��
4, // o ��
10, // p ��
0, // q ��
0, // r ��
0, // s ��
0, // t ��
11, // u ��
0, // v ��
0, // w ��
0, // x ��
19, // y ��
0, // z ��
// �빮�� ����
0, //a �� 
26, // b ��
0, // c ��
0, // d ��
0, // e ��
0, // f ��
0, // g ��
13, // h ��
5, // i ��
7, // j ��
8, // k ��
29, // l ��
27, // m ��
20, // ��
6, // o ��
12, // p ��
0, // q ��
0, // r ��
0, // s ��
0, // t ��
11, // u ��
0, // v ��
0, // w ��
0, // x ��
19, // y ��
0 // z ��
};

tINT JongSung[26*2] = {
// �ҹ��� ����
17, //a �� 
0, // b ��
25, // c ��
23, // d ��
8, // e ��
9, // f ��
29, // g ��
0, // h ��
0, // i ��
0, // j ��
0, // k ��
0, // l ��
0, // m ��
0, // ��
0, // o ��
0, // p ��
19, // q ��
2, // r ��
5, // s ��
21, // t ��
0, // u ��
28, // v ��
24, // w ��
27, // x ��
0, // y ��
26, // z ��
// �빮�� ����
17, //a �� 
0, // b ��
25, // c ��
23, // d ��
0, // e ��
9, // f ��
29, // g ��
0, // h ��
0, // i ��
0, // j ��
0, // k ��
0, // l ��
0, // m ��
0, // ��
0, // o ��
0, // p ��
0, // q ��
3, // r ��
5, // s ��
22, // t ��
0, // u ��
28, // v ��
0, // w ��
27, // x ��
0, // y ��
26 // z ��
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

	
	strcpy(szTmp, "��");
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
	else { // �빮���� ��..
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

// nLimitHan ���� ��..
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
		if (nState == 0) { // �ʼ��� �Է¹޾ƾ� �ϴµ�..
			nCh = GetChFromSungTable(szEng[i], 0);
			if (nCh == 0) { // ����... ���� ��쿡..
				if (tmpHan.a.last < 2) {
					szHan[0] = '\0';
					return FALSE;
				}
				nCh = GetChFromSungTable(szEng[i], 1); // �ؼ����� ã�ƺ���.
				if (nCh == 0) { // ����.
					szHan[0] = '\0';
					return FALSE;
				}

				if (tmpHan.a.last == 4 || tmpHan.a.last == 6 || tmpHan.a.last == 7 || (tmpHan.a.last >= 10 && tmpHan.a.last <= 16) || tmpHan.a.last == 20 ) { // �������̶�. ���� 2���� ����.
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
				else { // �տ� ���ڸ� ���� �ȴ�.
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
				nCh = GetChFromSungTable(szEng[i], 1); // �ؼ����� ã�ƺ���.
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
			else if (tmpHan.a.last == 9 && ( nCh == 2 || nCh == 8 || nCh == 9 || nCh == 11 || nCh == 18 || nCh == 19 || nCh == 20)) { // �������� ���� �� �ִ�.
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
				nCh = GetChFromSungTable(szEng[i], 1); // �ؼ����� ã�´�.
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

