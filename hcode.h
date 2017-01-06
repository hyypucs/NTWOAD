/*

     �������� �ϼ����� ��������� �����..
     by hyypucs
  */

#if !defined(__HCODE_H_)
#define __HCODE_H_

#define ENG_CODE	0        /* �����ڵ� */
#define KS_CODE	1        /* �ϼ��� �ڵ� */
#define KSSM_CODE	2        /* ������ �ڵ� */

#define KS_CHECK_COUNT	100  /* �ϼ����ڵ� Ȯ�� Ƚ�� */

/* �ѱ��ڵ��Ǻ� ��ƾ */
int	DetectHCode(unsigned char *szStr);
int KSStr2KSSMStr(unsigned char *szKSStr, unsigned char *szKSSMStr);
int KSSMStr2KSStr(unsigned char *szKSSMStr, unsigned char *szKSStr);
int KSHStr2KSSMHStr(unsigned short int *uszKSStr, unsigned short int *uszKSSMStr);
int KSSMHStr2KSHStr(unsigned short int *uszKSSMStr, unsigned short int *uszKSStr);


//
// bKSSM : TRUE -> KSSM ����
//         FALSE -> KS ����
// nCheckCode : HANGUL_CODE/HANJA_CODE/SPECIAL_CODE
#define HANGUL_CODE 	0
#define HANJA_CODE 	1
#define SPECIAL_CODE 	2
tBOOL checkChCode(tBYTE *szCh, tBOOL bKSSM, tINT nCheckCode);
tBOOL checkHChCode(tHCHAR uCh, tBOOL bKSSM, tINT nCheckCode);

#endif /* hcode.h */
