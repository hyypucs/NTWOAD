#if !defined(_CONV_ENG_HAN_H__)
#define _CONV_ENG_HAN_H__

//
// szEng : in ���� string
// szHan : Out �ѱ۷� ��ȯ�� ���
// return : TRUE �ѱ۷� ��ȯ ����, FALSE : ��ȯ ����
//          szHan������ kssm�ڵ�� ������ ������ �ϼ������� ��ȯ�� �� �ϼ���.
// nLimitHan : �ѱ� ���� �� �����̸� ��ȯ ���� �ʴ´�.
//             0�̸� �׳� �״��...
tBOOL ConvertEngHan(tCHAR *szEng, tCHAR *szHan, tINT nLimitHan);

#endif /* conv_enghan.h */
