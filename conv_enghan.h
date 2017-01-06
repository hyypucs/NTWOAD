#if !defined(_CONV_ENG_HAN_H__)
#define _CONV_ENG_HAN_H__

//
// szEng : in 영문 string
// szHan : Out 한글로 변환한 결과
// return : TRUE 한글로 변환 성공, FALSE : 변환 못함
//          szHan쪽으로 kssm코드로 나오기 때문에 완성형으로 변환을 꼭 하세요.
// nLimitHan : 한글 글자 수 이하이면 변환 하지 않는다.
//             0이면 그냥 그대로...
tBOOL ConvertEngHan(tCHAR *szEng, tCHAR *szHan, tINT nLimitHan);

#endif /* conv_enghan.h */
