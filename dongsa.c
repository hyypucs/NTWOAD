/*
	�������� �˻��ϴ� ��ƾ
        �����ϴ� ���̶� ��� �����ؾ� �Ѵ�.

	���� ���¼� �м���� ��ġ�Ǹ� �ʿ���� ���̴�. 

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


// ==== MakeDongSa�� �ڵ����� ������ ���� ====

#define MAX_DONGSA 57

// �տ� 0
// �߰����� 1
// ������ 2
// �ƹ� ���̳� 3

typedef struct {
	tHCHAR uszStr[MAX_WORD_LEN];
	tINT nInfo;
}DONGSA_TYPE;

DONGSA_TYPE gDongSa[MAX_DONGSA] = {
{{0x8956, 0x0000}, 3},	/* �� */
{{0x8a81, 0xa0e5, 0xb661, 0x0000}, 2},	/* ���տ� */
{{0x8a81, 0xa165, 0xb661, 0x0000}, 2},	/* ����� */
{{0x8a81, 0xb661, 0x0000}, 2},	/* ���� */
{{0x8a85, 0xb661, 0x0000}, 2},	/* ���� */
{{0x9061, 0xb7a1, 0x9461, 0x0000}, 2},	/* ���̴� */
{{0x9141, 0x8c61, 0x0000}, 2},	/* �ױ� */
{{0x9361, 0x90a1, 0x0000}, 3},	/* ���� */
{{0x93a1, 0x8c61, 0x0000}, 3},	/* �ϱ� */
{{0x93a1, 0x8d41, 0x0000}, 2},	/* �ϲ� */
{{0x93a1, 0x9461, 0x0000}, 2},	/* �ϴ� */
{{0x93a1, 0x9461, 0xa065, 0x0000}, 2},	/* �ϴٸ� */
{{0x9481, 0xb661, 0x0000}, 2},	/* ��� */
{{0x94e1, 0x8a85, 0x0000}, 2},	/* ���� */
{{0x94e1, 0x93a1, 0x0000}, 2},	/* ���� */
{{0x94e1, 0x9c61, 0x0000}, 2},	/* ���� */
{{0x94e5, 0x8861, 0xb661, 0x0000}, 2},	/* ������ */
{{0x94e5, 0x89a1, 0x0000}, 2},	/* ���� */
{{0x94e5, 0x9481, 0x0000}, 2},	/* ���� */
{{0x94e5, 0x9541, 0x0000}, 2},	/* ���� */
{{0x9541, 0xb661, 0x0000}, 2},	/* ���� */
{{0x9553, 0xae61, 0x0000}, 2},	/* ���� */
{{0x9765, 0x88e9, 0x0000}, 2},	/* ��� */
{{0x9765, 0xbba1, 0x0000}, 2},	/* ���� */
{{0x9c61, 0xbba1, 0xb661, 0x0000}, 2},	/* ������ */
{{0x9c81, 0xb661, 0x0000}, 2},	/* ���� */
{{0x9d65, 0x9461, 0x0000}, 2},	/* �ô� */
{{0x9d73, 0x9141, 0x9461, 0x0000}, 2},	/* �Ƴ״� */
{{0x9d75, 0x9461, 0x0000}, 2},	/* �Ǵ� */
{{0x9fa1, 0x8c61, 0x0000}, 2},	/* ���� */
{{0xac61, 0xb5b3, 0x0000}, 3},	/* ��� */
{{0xad41, 0xb661, 0x0000}, 2},	/* ���� */
{{0xaf61, 0x9cf3, 0x0000}, 3},	/* ���� */
{{0xaf73, 0x93a1, 0x0000}, 3},	/* ���� */
{{0xafa1, 0x8956, 0x0000}, 3},	/* �ð� */
{{0xafa1, 0x8a81, 0x9d61, 0x0000}, 2},	/* �ñ��� */
{{0xafa1, 0x8a81, 0xa0e5, 0x0000}, 2},	/* �ñ��� */
{{0xafa1, 0xb4f6, 0x0000}, 3},	/* �þ� */
{{0xafa1, 0xb5b3, 0x0000}, 3},	/* �ÿ� */
{{0xafb3, 0xae61, 0x0000}, 2},	/* �ʼ� */
{{0xafb3, 0xafa1, 0x9461, 0x0000}, 2},	/* �ʽô� */
{{0xafb3, 0xafa1, 0xb5a1, 0x0000}, 2},	/* �ʽÿ� */
{{0xb4f4, 0x90a1, 0x0000}, 3},	/* ���� */
{{0xb4f4, 0xaf73, 0x0000}, 3},	/* ���� */
{{0xb4f6, 0x0000}, 3},	/* �� */
{{0xb576, 0x0000}, 3},	/* �� */
{{0xb5b3, 0x9141, 0x8c61, 0x0000}, 2},	/* �ɳױ� */
{{0xb5b3, 0x93a1, 0x0000}, 3},	/* �ɴ� */
{{0xb867, 0x90a1, 0x0000}, 2},	/* �ݳ� */
{{0xb867, 0x93a1, 0x0000}, 2},	/* �ݴ� */
{{0xb867, 0xada1, 0x0000}, 2},	/* �ݼ� */
{{0xb867, 0xaf73, 0x0000}, 3},	/* �ݽ� */
{{0xb867, 0xb461, 0x0000}, 2},	/* �ݾ� */
{{0xb867, 0xb461, 0xb661, 0x0000}, 2},	/* �ݾƿ� */
{{0xb867, 0xb761, 0x90a1, 0x0000}, 2},	/* ������ */
{{0xb867, 0xb765, 0x8861, 0x0000}, 2},	/* ������ */
{{0xbba1, 0xb661, 0x0000}, 2}	/* ���� */
};

// ==== MakeDongSa�� �ڵ����� ������ �� ====

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
