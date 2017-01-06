/*
	동사인지 검사하는 루틴
        추측하는 것이라 계속 수정해야 한다.

	추후 형태소 분석기로 대치되면 필요없을 것이다. 

          by hyypucs
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include "dwtype.h"
#include "hstrlib.h"
#include "dongsa.h"


// ==== MakeDongSa로 자동으로 생성됨 시작 ====

#define MAX_DONGSA 57

// 앞에 0
// 중간에만 1
// 끝에만 2
// 아무 곳이나 3

typedef struct {
	tHCHAR uszStr[MAX_WORD_LEN];
	tINT nInfo;
}DONGSA_TYPE;

DONGSA_TYPE gDongSa[MAX_DONGSA] = {
{{0x8956, 0x0000}, 3},	/* 겠 */
{{0x8a81, 0xa0e5, 0xb661, 0x0000}, 2},	/* 구먼요 */
{{0x8a81, 0xa165, 0xb661, 0x0000}, 2},	/* 구면요 */
{{0x8a81, 0xb661, 0x0000}, 2},	/* 구요 */
{{0x8a85, 0xb661, 0x0000}, 2},	/* 군요 */
{{0x9061, 0xb7a1, 0x9461, 0x0000}, 2},	/* 나이다 */
{{0x9141, 0x8c61, 0x0000}, 2},	/* 네까 */
{{0x9361, 0x90a1, 0x0000}, 3},	/* 느냐 */
{{0x93a1, 0x8c61, 0x0000}, 3},	/* 니까 */
{{0x93a1, 0x8d41, 0x0000}, 2},	/* 니께 */
{{0x93a1, 0x9461, 0x0000}, 2},	/* 니다 */
{{0x93a1, 0x9461, 0xa065, 0x0000}, 2},	/* 니다만 */
{{0x9481, 0xb661, 0x0000}, 2},	/* 대요 */
{{0x94e1, 0x8a85, 0x0000}, 2},	/* 더군 */
{{0x94e1, 0x93a1, 0x0000}, 2},	/* 더니 */
{{0x94e1, 0x9c61, 0x0000}, 2},	/* 더라 */
{{0x94e5, 0x8861, 0xb661, 0x0000}, 2},	/* 던가요 */
{{0x94e5, 0x89a1, 0x0000}, 2},	/* 던고 */
{{0x94e5, 0x9481, 0x0000}, 2},	/* 던대 */
{{0x94e5, 0x9541, 0x0000}, 2},	/* 던데 */
{{0x9541, 0xb661, 0x0000}, 2},	/* 데요 */
{{0x9553, 0xae61, 0x0000}, 2},	/* 뎁쇼 */
{{0x9765, 0x88e9, 0x0000}, 2},	/* 든걸 */
{{0x9765, 0xbba1, 0x0000}, 2},	/* 든지 */
{{0x9c61, 0xbba1, 0xb661, 0x0000}, 2},	/* 라지요 */
{{0x9c81, 0xb661, 0x0000}, 2},	/* 래요 */
{{0x9d65, 0x9461, 0x0000}, 2},	/* 련다 */
{{0x9d73, 0x9141, 0x9461, 0x0000}, 2},	/* 렵네다 */
{{0x9d75, 0x9461, 0x0000}, 2},	/* 렷다 */
{{0x9fa1, 0x8c61, 0x0000}, 2},	/* 리까 */
{{0xac61, 0xb5b3, 0x0000}, 3},	/* 사옵 */
{{0xad41, 0xb661, 0x0000}, 2},	/* 세요 */
{{0xaf61, 0x9cf3, 0x0000}, 3},	/* 스럽 */
{{0xaf73, 0x93a1, 0x0000}, 3},	/* 습니 */
{{0xafa1, 0x8956, 0x0000}, 3},	/* 시겠 */
{{0xafa1, 0x8a81, 0x9d61, 0x0000}, 2},	/* 시구려 */
{{0xafa1, 0x8a81, 0xa0e5, 0x0000}, 2},	/* 시구먼 */
{{0xafa1, 0xb4f6, 0x0000}, 3},	/* 시었 */
{{0xafa1, 0xb5b3, 0x0000}, 3},	/* 시옵 */
{{0xafb3, 0xae61, 0x0000}, 2},	/* 십쇼 */
{{0xafb3, 0xafa1, 0x9461, 0x0000}, 2},	/* 십시다 */
{{0xafb3, 0xafa1, 0xb5a1, 0x0000}, 2},	/* 십시오 */
{{0xb4f4, 0x90a1, 0x0000}, 3},	/* 없냐 */
{{0xb4f4, 0xaf73, 0x0000}, 3},	/* 없습 */
{{0xb4f6, 0x0000}, 3},	/* 었 */
{{0xb576, 0x0000}, 3},	/* 였 */
{{0xb5b3, 0x9141, 0x8c61, 0x0000}, 2},	/* 옵네까 */
{{0xb5b3, 0x93a1, 0x0000}, 3},	/* 옵니 */
{{0xb867, 0x90a1, 0x0000}, 2},	/* 잖냐 */
{{0xb867, 0x93a1, 0x0000}, 2},	/* 잖니 */
{{0xb867, 0xada1, 0x0000}, 2},	/* 잖소 */
{{0xb867, 0xaf73, 0x0000}, 3},	/* 잖습 */
{{0xb867, 0xb461, 0x0000}, 2},	/* 잖아 */
{{0xb867, 0xb461, 0xb661, 0x0000}, 2},	/* 잖아요 */
{{0xb867, 0xb761, 0x90a1, 0x0000}, 2},	/* 잖으냐 */
{{0xb867, 0xb765, 0x8861, 0x0000}, 2},	/* 잖은가 */
{{0xbba1, 0xb661, 0x0000}, 2}	/* 지요 */
};

// ==== MakeDongSa로 자동으로 생성됨 끝 ====

tBOOL IsDongSa(tHCHAR *uszWord)
{
	tINT i, nLen, nLen1;
	tHCHAR usztmpWord[MAX_PATH], *p;

	nLen1 = HStrlen(uszWord);
	for ( i = 0 ; i < MAX_DONGSA ; i ++ ) {
		switch(gDongSa[i].nInfo) {
			case 0 :
				if (HStrncmp(uszWord, gDongSa[i].uszStr, HStrlen(gDongSa[i].uszStr)) == 0) {
					return TRUE;
				}
				break;
			case 1 :
				usztmpWord[0] = 0x00;
				HStrcpy(usztmpWord, uszWord+1);
				nLen = HStrlen(usztmpWord);
				if (nLen > 1) {
					usztmpWord[nLen-1] = 0x00;
					p = HStrstr(usztmpWord, gDongSa[i].uszStr);	
					if (p) {
						return TRUE;
					}
				}
				
				break;
			case 2 :
				nLen = HStrlen(gDongSa[i].uszStr);
				if (nLen1 <= nLen) break;

				if (HStrcmp(uszWord + nLen1 - nLen, gDongSa[i].uszStr) == 0) return TRUE;
				break;
			case 3 :
				p = HStrstr(uszWord, gDongSa[i].uszStr);	
				if (p) return TRUE;
				break;
			default :
				break;
		}
	}
	return FALSE;
}
