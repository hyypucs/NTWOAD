/*

     조합형과 완성형을 자유자재로 만들기..
     by hyypucs
  */

#if !defined(__HCODE_H_)
#define __HCODE_H_

#define ENG_CODE	0        /* 영문코드 */
#define KS_CODE	1        /* 완성형 코드 */
#define KSSM_CODE	2        /* 조합형 코드 */

#define KS_CHECK_COUNT	100  /* 완성형코드 확인 횟수 */

/* 한글코드판별 루틴 */
int	DetectHCode(unsigned char *szStr);
int KSStr2KSSMStr(unsigned char *szKSStr, unsigned char *szKSSMStr);
int KSSMStr2KSStr(unsigned char *szKSSMStr, unsigned char *szKSStr);
int KSHStr2KSSMHStr(unsigned short int *uszKSStr, unsigned short int *uszKSSMStr);
int KSSMHStr2KSHStr(unsigned short int *uszKSSMStr, unsigned short int *uszKSStr);


//
// bKSSM : TRUE -> KSSM 조사
//         FALSE -> KS 조사
// nCheckCode : HANGUL_CODE/HANJA_CODE/SPECIAL_CODE
#define HANGUL_CODE 	0
#define HANJA_CODE 	1
#define SPECIAL_CODE 	2
tBOOL checkChCode(tBYTE *szCh, tBOOL bKSSM, tINT nCheckCode);
tBOOL checkHChCode(tHCHAR uCh, tBOOL bKSSM, tINT nCheckCode);

#endif /* hcode.h */
