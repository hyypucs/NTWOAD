#if !defined(__INDEXLIB_H_)
#define __INDEXLIB_H_

/* section number가 필요할 것이다. */
#define SECTION_NO              0x0000
#define SECTION_TITLE           0x0001
#define SECTION_ABSTRACT        0x0002
#define SECTION_KEYWORD         0x0004
#define SECTION_AUTHOR          0x0010
#define SECTION_BODY            0x0020
#define SECTION_URL             0x0040
#define SECTION_UPDATE		0x0400 // update site인 경우

#define SECTION_DB              0x0100 //지식
#define SECTION_QUESTION        0x0200 //진행중인 질문
#define FILTER_DB               0x0300

/* 더 추가 될 확률 있음 */

/* 복합명사 분리 */
#define MAX_COMPWORD 80
#define H_WORD   0   /* hangul */
#define E_WORD   1   /* english */
#define N_WORD   2   /* number */
#define HJ_WORD  3   /* hanja */
#define D_WORD   4   /* dot */
#define ETC_WORD 5   /* etc */

// 한글 KS 코드 체크
/*
#define IsHangul_KS(ch)                    (RANGE(0xb0a1, ch, 0xbffe) || RANGE(0xc0a1, ch, 0xc8fe))
#define IsSpecial2_KS(ch)                  RANGE(0xa1a1, ch, 0xacfe)
#define IsEnglish2Low_KS(ch)               RANGE(0xa3e1, ch, 0xa3fa)
#define IsEnglish2Up_KS(ch)                RANGE(0xa3c1, ch, 0xa3da)
#define IsEnglish2_KS(ch)                  (IsEnglish2Low_KS(ch) || IsEnglish2Up_KS(ch))
#define IsNumber2_KS(ch)                   RANGE(0xa3b0, ch, 0xa3b9)
#define IsHanja_KS(ch)                     (RANGE(0xcaa1, ch, 0xdffe) || RANGE(0xe0a1, ch, 0xfdfe))
#define IsDot_KS(ch)                       ((ch) == '.' || (ch) == 0xa1a3)
#define IsSlash_KS(ch)                     ((ch) == '/')
#define IsDash_KS(ch)                      ((ch) == '-' || (ch) == 0xa3ad)
#define IsSpace_KS(ch)                     ((ch) == ' ' || (ch) == '\t' || (ch) == 0xa1a1)
*/


// 색인 유사어 사전
#define MAX_THSTR 20
#define C_THSTR 5

typedef struct {
        tCHAR szStr[MAX_THSTR];
        tINT nStr[C_THSTR];
}THSTR_FORMAT, *LPTHSTR_FORMAT;

typedef struct {
        tCHAR szStr[MAX_THSTR];
}STR_FORMAT, *LPSTR_FORMAT;
//


#define MAX_TOKEN_LEN 200

#define ONE_I 		0x0001 // 해당단어 
#define COMPOUND_I 	0x0002 // 복합명사로
#define SPLIT_I 	0x0004 // 짤린 단어로
#define THE_I 		0x0008 // 비슷한 단어 색인
#define USER_I 		0x0010 // 사용자사전으로.
#define HJ_I 		0x0020 // 한자 
#define DASH_I 		0x0040 //  DASH제거 d-day -> dday로 색인함 ddai 이렇게 되겠지 stem때문에.
#define ONE_VV_I 	0x0080 // 해당단어 + 동사, 형용사
#define ONE_AD_I 	0x0100 // 해당단어 + 부사

#define MAX_IWORDLIST 20

typedef struct {
	tINT nC_IWord;
	tINT nIndexInfo;
	tCHAR szIWord[MAX_IWORDLIST][MAX_WORD_LEN*2];
}IWORDLIST_FORMAT, *LPIWORDLIST_FORMAT;

typedef struct {
	tINT nPos;      // 실제 위치
	tINT nPosToken; // 토큰 위치 
	tCHAR szToken[MAX_TOKEN_LEN];
	RECORD IndexWord;	
}INDEX_RESULT_FORMAT, *LP_INDEX_RESULT_FORMAT;

tBOOL OpenIndexLib(tCHAR *szRoot);
tVOID CloseIndexLib(tVOID);
// bInsertOne = TRUE : 항상 토큰을 넣는다. 즉 색인한다.
// FALSE이면 안하고 그대로 하겠지.
tBOOL IndexStrIndexLib(tCHAR *szStr, RECORD *lpResIndexRec, tBOOL bInsertOne);


extern tBOOL GetFindThDic(tCHAR *szStr, RECORD *lpRetRec);

// 동사, 형용사인지 체크
tBOOL IsVVAJ(tCHAR *szStr);
tVOID FreeIndexRes(RECORD *lpResIndexRec);

typedef struct {
        tCHAR szWord[MAX_TOKEN_LEN];
        tINT   nStPos;
}TOKEN_FORMAT, *LPTOKEN_FORMAT;
tINT GetTokenFromLine(tCHAR *szLine, RECORD *lpTokenRec);

#endif

