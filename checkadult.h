/************************************************************
* Description : 성인인증 관련 모듈
************************************************************/

#define SEXKEYWORDFILE_0 "./sexkeyword_0.txt"
#define SEXKEYWORDFILE_1 "./sexkeyword_1.txt"
#define SEXKEYWORDFILE_2 "./sexkeyword_2.txt"
#define SEXFILTEREXPFILE "./sexunfilter.ini"

#define SEXFILTERFILE "./sexfilter.ini"

tBOOL CheckAdultKey(tCHAR *szQuery);
tBOOL CheckAdultKeybyQ(tCHAR *szQuery);
