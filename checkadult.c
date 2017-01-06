/************************************************************
* Description : 성인인증 관련 모듈
* Author : Information Technology Team, ytlee
* Date : July 16, 2004
************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>

#include "dwtype.h"
#include "checkadult.h"
#include "util.h"

tBOOL gbExpFlag = FALSE;
tCHAR gszExpBuf[512][512];
tINT gnSexExpFIndex=0;

tINT CheckUnAdultKey(tCHAR *szEleSpaceQ);

/*========================================================
	CheckAdultKey()
	Description : 
	  - query 중에 성인관련 키워드가 있으면 문구 출력한다.
    - CheckAdultKeyByQ() 함수의 업그래이드 버전
    - 성인키워드를 2단계로 나누어, 
      정도가 심한 키워드는 strstr
      약한건 strcmp로 검사                             
========================================================*/
tBOOL CheckAdultKey(tCHAR *szQuery)
{
	FILE *fp;
	tCHAR szLine[1024];
	tCHAR szBuf[1024];
	tBOOL bAdultWord = FALSE;
	//tBOOL bUnAdultWord = FALSE;
	//tCHAR *p = szQuery;
	tCHAR szQ[MAX_QUERY];
	tCHAR szEleSpaceQ[MAX_QUERY];
	//tINT nAdult;

	tINT i, j=0;
	//tBOOL bExpFlag = 0;
	tBOOL bFoundAdultExp=FALSE;
	tCHAR *p;
	tINT nRetCheckUnAdultKey = -1;

	strcpy(szQ, szQuery);
	
	//free(p);

	if(strlen(szQ) < 2) {
		return FALSE;
	}

	/*
		성인인증 Level 0 Keyword
	 */
	/*
	if (strlen(szQ) <= 20) {
		if (access(SEXKEYWORDFILE_0, F_OK) != 0) {
			return FALSE;
		}
	
		if ((fp = fopen(SEXKEYWORDFILE_0, "rt")) == NULL) {
			return FALSE;
		}

		while (fgets(szLine, 1023, fp)) {
			bAdultWord = FALSE;
			bFoundAdultExp = FALSE;

			if (sscanf(szLine, "%s", szBuf) != 1) continue;
			if (szBuf[0] == '/' || szBuf[0] == ';') continue;
		
			if (StrCaseStr((tBYTE *)szQ, (tBYTE *)szBuf) != NULL) {
				bFoundAdultExp = TRUE;
				//printf("level 0, %s, %d<br>\n", __FILE__, __LINE__);
				//break;
			}

			if(bFoundAdultExp == TRUE) {
				if( (nRetCheckUnAdultKey = CheckUnAdultKey(szQ)) == TRUE) {
					//printf("level 0 : sexunfilter present, %s, %d<br>\n", __FILE__, __LINE__);
					return FALSE;
				}
				else if(nRetCheckUnAdultKey == FALSE) {
					//printf("level 0 : sexunfilter not present, %s, %d<br>\n", __FILE__, __LINE__);
					return TRUE;
				}
			}
		}
		fclose(fp);
	}
	*/

	for(i=0; i<(tINT)strlen(szQ); i++) {
		if(szQ[i] != ' ') {
			szEleSpaceQ[j] = szQ[i];
			j += 1;
		}
	}
	szEleSpaceQ[j] = '\0';

	/*
		성인인증 Level 1 Keyword
	 */
	if (access(SEXKEYWORDFILE_1, F_OK) != 0) {
		return FALSE;
	}
	
	if ((fp = fopen(SEXKEYWORDFILE_1, "rt")) == NULL) {
		return FALSE;
	}

	while (fgets(szLine, 1023, fp)) {
		bAdultWord = FALSE;
		bFoundAdultExp = FALSE;

		if (sscanf(szLine, "%s", szBuf) != 1) continue;
		if (szBuf[0] == '/' || szBuf[0] == ';') continue;
		
		if (StrCaseStr((tBYTE *)szEleSpaceQ, (tBYTE *)szBuf) != NULL) {
			bFoundAdultExp = TRUE;
			//break;
		}

		if(bFoundAdultExp == TRUE) {
			if(strcmp(szBuf, szEleSpaceQ) == 0) {
				//printf("level 1 : 0, %s, %d<br>\n", __FILE__, __LINE__);
				return TRUE;
				//bAdultWord = 1;
			}
			else if(strncmp(szBuf, szEleSpaceQ, strlen(szBuf)) == 0 ) {
				//printf("level 1 : 1, %s, %d<br>\n", __FILE__, __LINE__);
				if(strlen(szEleSpaceQ) - strlen(szBuf) <= 8) {
					bAdultWord = 2;
				}
			}
			else if(strcmp(szEleSpaceQ+(strlen(szEleSpaceQ)-strlen(szBuf)), szBuf) == 0) {
				//printf("level 1 : 2, %s, %d<br>\n", __FILE__, __LINE__);
				if(strlen(szEleSpaceQ) - strlen(szBuf) <= 8) {
					bAdultWord = 3;
				}
			}
			else {
				p = strstr(szEleSpaceQ, szBuf);
				if(p-szEleSpaceQ <= 4) {
					//printf("level 1 : 3_1, %s, %d<br>\n", __FILE__, __LINE__);
					bAdultWord = 4;
				}
				else if( (szEleSpaceQ+strlen(szEleSpaceQ)) - (p+strlen(szBuf)) <= 4) {
					//printf("level 1 : 3_2, %s, %d<br>\n", __FILE__, __LINE__);
					bAdultWord = 5;
				}
			}
			if(bAdultWord != FALSE) {
				if( (nRetCheckUnAdultKey = CheckUnAdultKey(szEleSpaceQ)) == TRUE) {
					//printf("level 1 : sexunfilter present, %s, %d<br>\n", __FILE__, __LINE__);
					return FALSE;
				}
				else if(nRetCheckUnAdultKey == FALSE) {
					//printf("level 1 : sexunfilter not present, %s, %d<br>\n", __FILE__, __LINE__);
					return TRUE;
				}
			}
		}
	}
	fclose(fp);

	/*
		성인인증 Level 2 Keyword
	 */
	if (access(SEXKEYWORDFILE_2, F_OK) != 0) {
		return FALSE;
	}
	
	if ((fp = fopen(SEXKEYWORDFILE_2, "rt")) == NULL) {
		return FALSE;
	}

	while (fgets(szLine, 1023, fp)) {
		bAdultWord = FALSE;
		bFoundAdultExp = FALSE;

		if (sscanf(szLine, "%s", szBuf) != 1) continue;
		if (szBuf[0] == '/' || szBuf[0] == ';') continue;
		
		if (strcasecmp(szBuf, szEleSpaceQ) == 0) {
			//printf("level 2, %s, %d<br>\n", __FILE__, __LINE__);
			return TRUE;
		}
	}
	fclose(fp);

	return FALSE;
}

/*========================================================
	CheckUnAdultKey()
	Description : 
========================================================*/
tINT CheckUnAdultKey(tCHAR *szEleSpaceQ) 
{
	FILE *fpexp;
	tCHAR szLine[1024];
	tCHAR szBuf[1024];
	tINT i;

	if(gbExpFlag == FALSE) {
		if ((fpexp = fopen(SEXFILTEREXPFILE, "rt")) == NULL) {
			return -1;
		}
		while (fgets(szLine, 512, fpexp)) {
			if (sscanf(szLine, "%s", szBuf) != 1) continue;
			if (szBuf[0] == '/' || szBuf[0] == ';') continue;
			strcpy(gszExpBuf[gnSexExpFIndex], szBuf);
			//cgiPrintf("[%s]<br>\n", szExpBuf[nSexExpFIndex]);
			gnSexExpFIndex += 1;
			if(gnSexExpFIndex >= 512) {
				break;
			}
		}
		fclose(fpexp);

		gbExpFlag = 1;
	}

	if(gbExpFlag == 1) {
		//cgiPrintf("[%d]<br>\n", nSexExpFIndex);
		for(i=0; i<gnSexExpFIndex; i++) {
			//cgiPrintf("[%s][%s]<br>\n", szQ, szExpBuf[i]);
			if (gszExpBuf[i][0] == '%') { // %일 경우 substring matching을 한다. ex)%야사랑해
				if (strstr(szEleSpaceQ, gszExpBuf[i]+1) != NULL) {
					return TRUE;
				}
			}
			else {
				if (strcmp(szEleSpaceQ, gszExpBuf[i]) == 0) {
					return TRUE;
				}
			}
		}
	}
	
	return FALSE;
}


/*========================================================
	CheckAdultKeybyQ()
	Description : 
	  - query 중에 성인관련 키워드가 있으면 문구 출력한다.
========================================================*/
tBOOL CheckAdultKeybyQ(tCHAR *szQuery)
{
	FILE *fp, *fpexp;
	tCHAR szLine[1024];
	tCHAR szBuf[1024];
	tBOOL bAdultWord = FALSE;
	tBOOL bUnAdultWord = FALSE;
	//tCHAR *p = szQuery;
	tCHAR szQ[MAX_QUERY];
	tCHAR szEleSpaceQ[MAX_QUERY];
	//tINT nAdult;

	tINT i, j=0;
	tINT nSexExpFIndex=0;
	tCHAR szExpBuf[512][512];
	tBOOL bExpFlag = 0;
	tBOOL bFoundAdultExp=FALSE;

	strcpy(szQ, szQuery);
	
	//free(p);

	if (access(SEXFILTERFILE, F_OK) != 0) {
		return FALSE;
	}

	if ((fp = fopen(SEXFILTERFILE, "rt")) == NULL) {
		return FALSE;
	}

	for(i=0; i<(tINT)strlen(szQ); i++) {
		if(szQ[i] != ' ') {
			szEleSpaceQ[j] = szQ[i];
			j += 1;
		}
	}
	szEleSpaceQ[j] = '\0';

	while (fgets(szLine, 1023, fp)) {
		if (sscanf(szLine, "%s", szBuf) != 1) continue;
		if (szBuf[0] == '/' || szBuf[0] == ';') continue;
		if (szBuf[0] >= '0' && szBuf[0] <= '0') {  // 숫자일 경우
			if (strcmp(szQ, szBuf) == 0) {
				bAdultWord = TRUE;
				break;
			}
		}
		else {
			bFoundAdultExp = FALSE;

			if (strlen(szBuf) > 2) {
				if (StrCaseStr((tBYTE *)szQ, (tBYTE *)szBuf) != NULL) {
					bFoundAdultExp = TRUE;
				}
			}
			else {
				if( (StrCaseCmp((tBYTE *)szQ, (tBYTE *)szBuf, strlen(szQ)) == 0)) {
					bFoundAdultExp = TRUE;
				}
			}
			/*
			if (strlen(szBuf) > 2) {
				if (strncasecmp(szQ, szBuf, strlen(szBuf)) == 0) {
					bFoundAdultExp = TRUE;
				}
			}
			else {
				if( (strcasecmp(szQ, szBuf) == 0)) {
					bFoundAdultExp = TRUE;
				}
			}
			*/

			if( bFoundAdultExp == TRUE ) {

				//printf("[%s][%s]<br>\n", szQ, szBuf);

				if(bExpFlag == 0) {
					if ((fpexp = fopen(SEXFILTEREXPFILE, "rt")) == NULL) {
						//acgiPrintf("SEX_EXP_END<br>\n");
						goto SEX_EXP_END;
					}
					while (fgets(szLine, 512, fpexp)) {
						if (sscanf(szLine, "%s", szBuf) != 1) continue;
						if (szBuf[0] == '/' || szBuf[0] == ';') continue;
						strcpy(szExpBuf[nSexExpFIndex], szBuf);
						//cgiPrintf("[%s]<br>\n", szExpBuf[nSexExpFIndex]);
						nSexExpFIndex += 1;
						if(nSexExpFIndex >= 512) {
							break;
						}
					}
					fclose(fpexp);

					bExpFlag = 1;
				}

				if(bExpFlag == 1) {
					//cgiPrintf("[%d]<br>\n", nSexExpFIndex);
					for(i=0; i<nSexExpFIndex; i++) {
						//cgiPrintf("[%s][%s]<br>\n", szQ, szExpBuf[i]);
						if (szExpBuf[i][0] == '%') { // %일 경우 substring matching을 한다. ex)%야사랑해
							if (strstr(szEleSpaceQ, szExpBuf[i]+1) != NULL) {
								bUnAdultWord = TRUE;
								break;
							}							
						}
						else {
							if (strcmp(szEleSpaceQ, szExpBuf[i]) == 0) {
								bUnAdultWord = TRUE;
								break;
							}
						}
					}
				}

			SEX_EXP_END:;

				if(bUnAdultWord == FALSE) {
					bAdultWord = TRUE;
					break;
				}
			}
		
			
		}
	}
	fclose(fp);

	return bAdultWord;
}

