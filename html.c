/*

        Comment:
*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "dwtype.h"
#include "hstrlib.h"
#include "html.h"

#define HTML_ELEMENTS (128) 
#define MAX_TAGLEN 17

PRIVATE tHCHAR *searchTag(tHCHAR *uszHtmlStr, tHCHAR *uszTag);
PRIVATE tINT hstricmpwspc(tHCHAR *s1, tHCHAR *s2);
PRIVATE tINT hstrnicmpwspc(tHCHAR *s1, tHCHAR *s2, tINT len);
PRIVATE tBOOL isSpace(tHCHAR c);
PRIVATE tVOID deleteTagWithContent(tHCHAR *uszHtmlStr,
                                tHCHAR *uszStartTag, tHCHAR *uszEndTag);

/*
tVOID hprintf(tHCHAR *str)
{
        tHCHAR uszStr[20000];
        tCHAR szStr[40000];
	tINT nLen = HStrlen(str);

        if(nLen > 20000) nLen = 20000;
        HStrncpy(uszStr, str, nLen);
        uszStr[19999] = 0;
        hstr2str(uszStr, szStr);
	printf("%s<br>\n", szStr);
	printf("------------\n");
}

tBOOL hputc3(tHCHAR c)
{
        tCHAR szStr[3];

        szStr[0] = c >> 8 & 0xff;
        szStr[1] = c & 0xff;
        szStr[2] = 0;

        printf("[%s]", szStr);
}

tBOOL hputc3(tHCHAR c)
{

        printf("[%c]", c);
	return TRUE;
}
*/
PUBLIC tBOOL getHtmlTitle(tHCHAR *uszHtmlStr, tHCHAR *uszTitle)
{
        tHCHAR uszTITLE[] = { '<', 'T', 'I', 'T', 'L', 'E', '>', '\0' };
	tHCHAR uszTITLEEND[] = {'<', '/', 'T', 'I', 'T', 'L', 'E', '>', '\0' };
        tHCHAR uszBuf[1024];
        tHCHAR *p1, *p2;


        p1 = uszHtmlStr;
        while(*p1 != 0) {
                if((*p1 == '<') && HStrnicmp(p1, uszTITLE, 7) == 0) {
                        p2 = HStristr(p1+7, uszTITLEEND);
                        if(p2 == NULL) return FALSE;
                        while(*p1 != '>') p1++;
                        if(p2 - p1 > 1023) p2 = p1 + 1023;
                        HStrncpy(uszBuf, p1 + 1, p2 - p1);
                        uszBuf[p2 - p1 - 1] = 0;
			DeleteTag(uszBuf);
                        HLTrim(uszBuf);
                        HRTrim(uszBuf);
                        HStrcpy(uszTitle, uszBuf);
                        return TRUE;
                }
                p1++;
        } 
	return FALSE; 
} 



PUBLIC tVOID deleteSpecialTag(tHCHAR *uszHtmlStr)
{
        tHCHAR uszBeginStyleTag[] = { '<', 'S', 'T', 'Y', 'L', 'E', '\0' };
        tHCHAR uszEndStyleTag[] = { '<', '/', 'S', 'T', 'Y', 'L', 'E', '>', '\0'};
        tHCHAR uszBeginScriptTag[] = { '<', 'S', 'C', 'R', 'I', 'P', 'T', '\0' };
        tHCHAR uszEndScriptTag[] = { '<', '/', 'S', 'C', 'R', 'I', 'P', 'T', '>', '\0'};
        tHCHAR uszBeginMapTag[] = { '<', 'M', 'A', 'P', '\0' };
        tHCHAR uszEndMapTag[] = { '<', '/', 'M', 'A', 'P', '>', '\0'};

        deleteTagWithContent(uszHtmlStr, uszBeginStyleTag, uszEndStyleTag);
        deleteTagWithContent(uszHtmlStr, uszBeginScriptTag, uszEndScriptTag);
        deleteTagWithContent(uszHtmlStr, uszBeginMapTag, uszEndMapTag);

}

PUBLIC tVOID deleteHtmlTitle(tHCHAR *uszHtmlStr)
{
        tHCHAR uszTITLE[] = { '<', 'T', 'I', 'T', 'L', 'E', '>', '\0' };
        tHCHAR uszTITLEEND[] = {'<', '/', 'T', 'I', 'T', 'L', 'E', '>', '\0' };

        deleteTagWithContent(uszHtmlStr, uszTITLE, uszTITLEEND);
}
PRIVATE tVOID deleteTagWithContent(tHCHAR *uszHtmlStr,
                                tHCHAR *uszStartTag, tHCHAR *uszEndTag)
{
        tHCHAR *p1, *p2;

        p1 = uszHtmlStr;
        while(1) {
                p1 = HStristr(p1, uszStartTag);
                if(p1 == NULL) {
			break;
		}
                p2 = HStristr(p1, uszEndTag);
                if(p2 == NULL) {
			break;
		}
                while(*p2 != '>' && *p2 != 0) {
			//hputc3(*p2);
			p2++;
		}
                while(p1 <= p2) {
                        *p1 = ' ';
                        p1++;
                }
        }

}
/*
PRIVATE tVOID deleteTagWithContent(tHCHAR *uszHtmlStr,
                                tHCHAR *uszStartTag, tHCHAR *uszEndTag)
{
        tHCHAR *p1, *p2;

        p1 = uszHtmlStr;
        while(1) {
                p1 = searchTag(p1, uszStartTag);
                if(p1 == NULL) {
			break;
		}
		//printf("search start tag passed. ");
                p2 = searchTag(p1, uszEndTag);
                if(p2 == NULL) {
			break;
		}
		//printf("search end tag passed. ");
                while(*p2 != '>' && *p2 != 0) {
			//hputc3(*p2);
			p2++;
		}
                while(p1 <= p2) {
                        *p1 = ' ';
                        p1++;
                }
        }

}
*/
PRIVATE tHCHAR *searchTag(tHCHAR *uszHtmlStr, tHCHAR *uszTag)
{
	tHCHAR *p, *p1;
	tINT nLen = HStrlen(uszTag);

	p = uszHtmlStr;
	while(*p != 0) {
		if(*p == '<') {
			if(HStrnicmp(p, uszTag, nLen) == 0) {
				return p;
			}
		}
		p++;
	}
	return NULL;
}

PRIVATE tINT hstricmpwspc(tHCHAR *Ss1, tHCHAR *Ss2)
{
        tHCHAR c1, c2;
	tINT ret;
	tHCHAR *s1 = Ss1, *s2 = Ss2;

        while(*s1 || *s2) {
		while(isSpace(*s1)) s1++;
		while(isSpace(*s2)) s2++;

                c1 = *s1++;
                c2 = *s2++;
		//printf("{%c, %c}", c1, c2);

                //if(c1 == 0 && c2 == 0) {
                //        break;
                //}

                c1 = HTolower(c1);
                c2 = HTolower(c2);

                if(c1 != c2) {
			ret = ((c1 > c2) ? 1 : -1);
			return ret;
                }
        }
        return(0);
}

PUBLIC tVOID deleteTag(tHCHAR *uszBuf)
{
        tBOOL delFlag;
	tBOOL restartFlag;
	tBOOL pairOk;
	tHCHAR *p, *p1;

        delFlag = FALSE;


	//printf("hstrlen(uszBuf) = %d\n", HStrlen(uszBuf));
	p = uszBuf;
        while(*p != 0) {
                if(*p == '<') {
			p1 = p+1;
			restartFlag = FALSE;
			pairOk = FALSE;
			while(*p1 != 0) {
				if(*p1 == '<') {
					restartFlag = TRUE;
					break;
				}
				if(*p1 == '>') {
					pairOk = TRUE;
					break;
				}
				p1++;
			}
			if(restartFlag == TRUE) {
				p++;
				continue;
			}
			if(*(p+1) == '/' || *(p+1) == '!' || isalpha(*(p+1))) {
				if(pairOk == TRUE) delFlag = TRUE;
			}
		}
                if(*p == '>') {
                        if(delFlag == TRUE) {
				delFlag = FALSE;
                        	*p = ' ';
			}
                }
                if(delFlag == TRUE) *p = ' ';
		p++;
        }
}

#if defined(_OLD)
// < >로 감싼 것과 특수문자 다 없애버리고 남은 것이 제목.
PUBLIC tVOID DeleteTag(tHCHAR *uszBuf)
{
        tBOOL delFlag;
	tBOOL restartFlag;
	tBOOL pairOk;
	tHCHAR *p, *p1;

        delFlag = FALSE;


	//printf("hstrlen(uszBuf) = %d\n", HStrlen(uszBuf));
	p = uszBuf;
        while(*p != 0) {
                if(*p == '<') {
			p1 = p+1;
			restartFlag = FALSE;
			pairOk = FALSE;
			while(*p1 != 0) {
				if(*p1 == '<') {
					restartFlag = TRUE;
					break;
				}
				if(*p1 == '>') {
					pairOk = TRUE;
					break;
				}
				p1++;
			}
			if(restartFlag == TRUE) {
				p++;
				continue;
			}
			if(*(p+1) == '/' || *(p+1) == '!' || isalpha(*(p+1))) {
				if(pairOk == TRUE) delFlag = TRUE;
			}
		}
                if(*p == '>') {
                        if(delFlag == TRUE) {
				delFlag = FALSE;
                        	*p = ' ';
			}
                }
                if(delFlag == TRUE) *p = ' ';
		p++;
        }
}
#else
PUBLIC tVOID DeleteTag(tHCHAR *uszBuf)
{
	tINT nLen, nTotalLen, nTotalLen1;
	tHCHAR *p, *p1;
        tHCHAR uszCommentS[] = { '<', '!', '-', '\0' };
        tHCHAR uszCommentE[] = { '-', '>', '\0' };

	//printf("hstrlen(uszBuf) = %d\n", HStrlen(uszBuf));
	p = uszBuf;

	nTotalLen = HStrlen(p);

	while (*p) {
		p = HStrchr(p, (tHCHAR)'<');
		if (p == NULL) { // 태그가 없다. 
			return;
		}
		if (*(p+1) == 0x00) { // exception
			*p = ' ';
			return ;
		}
		if (*(p+1) == (tHCHAR)'!' && HStrnicmp(p,uszCommentS, 3) == 0 ) {
			p1 = HStrstr(p+1, uszCommentE);
			if (p1 == NULL) { // 닫는 괄호가 없다.
				p1 = HStrchr(p+1, '>');
				if (p1 == NULL) {
					*p = ' ';
					return;
				}
			}
			else p1 += 1; // -넘기고..
		}
		else if(*(p+1) == (tHCHAR)'/' || *(p+1) == (tHCHAR)'!' || IsEnglish(*(p+1))) { // tag가 맞다.
			p1 = HStrchr(p+1, '>');
			if (p1 == NULL) { // 닫는 괄호가 없다.
				*p = ' ';
				return;
			}
		}
		else { // tag가 아니다. 
			p++;
			continue;
		}
		nTotalLen1 = nTotalLen - (p - uszBuf);
		nLen = p1 - p + 1;
		//HStrcpy(p, p+nLen);
		memcpy(p, p+nLen, (nTotalLen1 - nLen + 1) * sizeof(tHCHAR)); // +1은 null까지 move해야지..
	}
}
#endif

#if defined(_OLD)
PUBLIC tVOID DeleteTagExceptBR(tHCHAR *uszBuf, tINT nOpt)
{
        tBOOL delFlag;
	tBOOL restartFlag;
	tBOOL pairOk;
	tHCHAR *p, *p1;
        tHCHAR uszBR[] = { '<', 'B', 'R', '>', '\0' };
        tHCHAR uszP[] = { '<', 'P', '>', '\0' };

        delFlag = FALSE;


	//printf("hstrlen(uszBuf) = %d\n", HStrlen(uszBuf));
	p = uszBuf;
        while(*p != 0) {
                if(*p == '<') {
			if(HStrnicmp(p, uszBR, 4) == 0) {
				p++;
				continue;
			}
			if(HStrnicmp(p, uszP, 3) == 0) {
				p++;
				continue;
			}
			p1 = p+1;
			restartFlag = FALSE;
			pairOk = FALSE;
			while(*p1 != 0) {
				if(*p1 == '<') {
					restartFlag = TRUE;
					break;
				}
				if(*p1 == '>') {
					pairOk = TRUE;
					break;
				}
				p1++;
			}
			if(restartFlag == TRUE) {
				p++;
				continue;
			}
			if(*(p+1) == '/' || *(p+1) == '!' || isalpha(*(p+1))) {
				if(pairOk == TRUE) delFlag = TRUE;
			}
		}
                if(*p == '>') {
                        if(delFlag == TRUE) {
				delFlag = FALSE;
                        	*p = ' ';
			}
                }
                if(delFlag == TRUE) *p = ' ';
		p++;
        }
	if (nOpt) convSpecialChr(uszBuf);
}
#else
PUBLIC tVOID DeleteTagExceptBR(tHCHAR *uszBuf, tINT nOpt)
{
	tINT nLen, nTotalLen, nTotalLen1;
	tHCHAR *p, *p1;
        tHCHAR uszBR[] = { '<', 'B', 'R', '>', '\0' };
        tHCHAR uszP[] = { '<', 'P', '>', '\0' };
        tHCHAR uszCommentS[] = { '<', '!', '-', '\0' };
        tHCHAR uszCommentE[] = { '-', '>', '\0' };

	//printf("hstrlen(uszBuf) = %d\n", HStrlen(uszBuf));
	p = uszBuf;

	nTotalLen = HStrlen(p);

	while (*p) {
		p = HStrchr(p, (tHCHAR)'<');
		if (p == NULL) { // 태그가 없다. 
			return;
		}
		if (*(p+1) == 0x00) { // exception
			*p = ' ';
			return ;
		}
		if (*(p+1) == (tHCHAR)'!' && HStrnicmp(p,uszCommentS, 3) == 0 ) {
			p1 = HStrstr(p+1, uszCommentE);
			if (p1 == NULL) { // 닫는 괄호가 없다.
				p1 = HStrchr(p+1, '>');
				if (p1 == NULL) {
					*p = ' ';
					return;
				}
				else {
					*p = ' ';
					return;
				}
			}
			else p1 += 1; // -넘기고..
		}
		else if(*(p+1) == (tHCHAR)'/' || *(p+1) == (tHCHAR)'!' || IsEnglish(*(p+1))) { // tag가 맞다.
			p1 = HStrchr(p+1, '>');
			if (p1 == NULL) { // 닫는 괄호가 없다.
				*p = ' ';
				return;
			}
		}
		else { // tag가 아니다. 
			p++;
			continue;
		}
		if (HStrnicmp(p, uszBR, 4) == 0 || HStrnicmp(p, uszP, 3) == 0) {
			p++;
			continue;
		}
		nTotalLen1 = nTotalLen - (p - uszBuf);
		nLen = p1 - p + 1;
		//HStrcpy(p, p+nLen);
		memcpy(p, p+nLen, (nTotalLen1 - nLen + 1) * sizeof(tHCHAR)); // +1은 null까지 move해야지..
	}
	if (nOpt) convSpecialChr(uszBuf);
}

#endif

PUBLIC tVOID convSpecialChr(tHCHAR *uszHtmlStr)
{
	tHCHAR uszNBSP[] = { '&', 'n', 'b', 's', 'p', ';', '\0'};
	tHCHAR uszQUOT[] = { '&', 'q', 'u', 'o', 't', ';', '\0'};
	tHCHAR uszLT[] = { '&', 'l', 't', ';', '\0'};
	tHCHAR uszGT[] = { '&', 'g', 't', ';', '\0'};
        tHCHAR uszAMP[] = { '&', 'a', 'm', 'p', '\0' };
        tHCHAR uszSHY[] = { '&', 's', 'h', 'y', '\0' };

	tHCHAR *uszHtmlTmp;
	tHCHAR *p1, *p2;

	uszHtmlTmp = (tHCHAR*)malloc(HStrlen(uszHtmlStr) * sizeof(tHCHAR) + 1);
	if(uszHtmlTmp == NULL) {
		printf("convSpecialChr() malloc error.\n");
		return;
	}
	p1 = uszHtmlStr;
	p2 = uszHtmlTmp;
	while(*p1 != 0) {
		if(*p1 == '&') {
			if(HStrnicmp(p1, uszNBSP, 6) == 0) {
				*p2 = ' ';
				p2++;
				p1 += 6;
			}
			else if(HStrnicmp(p1, uszQUOT, 6) == 0) {
				*p2 = '"';
				p2++;
				p1 += 6;
			}
			else if(HStrnicmp(p1, uszLT, 4) == 0) {
				*p2 = '<';
				p2++;
				p1 += 4;
			}
			else if(HStrnicmp(p1, uszGT, 4) == 0) {
				*p2 = '>';
				p2++;
				p1 += 4;
			}
			else if(HStrnicmp(p1, uszAMP, 5) == 0) {
				*p2 = '>';
				p2++;
				p1 += 5;
			}
			else if(HStrnicmp(p1, uszSHY, 5) == 0) {
				*p2 = '>';
				p2++;
				p1 += 5;
			}
			else {
				*(p2) = *(p1);
				p1++;
				p2++;
			}
			continue;
		}
		*(p2) = *(p1);
		p1++;
		p2++;
		
	}
	*(p2) = 0;
	HStrcpy(uszHtmlStr, uszHtmlTmp);
	free(uszHtmlTmp);	
}


PRIVATE tINT hstrnicmpwspc(tHCHAR *s1, tHCHAR *s2, tINT len)
{
        tHCHAR c1, c2;

        while(len--) {

		while(isSpace(*s1)) s1++;
		while(isSpace(*s2)) s2++;

                c1 = *s1++;
                c2 = *s2++;
                if(c1 == 0) {
                        break;
                }

                c1 = HTolower(c1);
                c2 = HTolower(c2);

                if(c1 != c2) {
                        return((c1 > c2) ? 1 : -1);
                }
        }

        return(0);
}

PRIVATE tBOOL isSpace(tHCHAR c)
{
        if(c == ' ' ||
                c == '\t' ||
                c == '\r' ||
                c == '\n') return TRUE;
        else return FALSE;
}

