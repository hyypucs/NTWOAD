/*
 

		정보검색서버를 위한 전체 type define

		Author : hyypucs
		Date   : 1999. 9. 16.

*/

#if !defined(__DWTYPE_H__)

#define __DWTYPE_H__

#include <stdlib.h>
#include <jemalloc/jemalloc.h>

#define FALSE   0
#define TRUE    1

#if !defined(_FREEBSD)
#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8
#endif

#define MAX_BUF     1024

#define SIGN_LEN      40


#define PUBLIC
#define PRIVATE static

#define GETMAX(arg1, arg2)      ((arg1) > (arg2) ? (arg1) : (arg2))
#define GETMIN(arg1, arg2)      ((arg1) < (arg2) ? (arg1) : (arg2))


#define MAX_PATH	256

#define MAX_RELWORD 	20

#define MAX_WORD_LEN	40

#define MAX_TITLE 	    100
#define MAX_ABSTRACT    240
#define MAX_KEYWORD     256
#define MAX_IMGNAME	    50
#define MAX_TIME	      15
#define MAX_CAT_NUM		  10
#define MAX_CAT_NAME		154

#define LEN_KEY		20

#define MAX_DIR 	10
#define MAX_KEY 	5

#define MAX_QUERY	4096

#define RANGE(low, cur, high)   (((low) <= (cur)) && ((cur) <= (high)))
#define IsHangul_KSSM(ch)			RANGE(0x8000, ch, 0xD7FF)
#define IsSpecial2_KSSM(ch)			RANGE(0xD930, ch, 0xDEFF)
#define IsEnglish2Low_KSSM(ch)		RANGE(0xDA71, ch, 0xDA9C)
#define IsEnglish2Up_KSSM(ch)		RANGE(0xDA51, ch, 0xDA6A)
#define IsEnglish2_KSSM(ch)			(IsEnglish2Low(ch) || IsEnglish2Up(ch))
#define IsNumber2_KSSM(ch)			RANGE(0xDA40, ch, 0xDA49)
#define IsHanja_KSSM(ch)			RANGE(0xE030, ch, 0xF9FF)
#define IsDot_KSSM(ch)			((ch) == '.' || (ch) == 0xDA3E)
#define IsSlash_KSSM(ch)			((ch) == '/')
#define IsDash_KSSM(ch)			((ch) == '-' || (ch) == 0xDA3D)
#define IsSpace_KSSM(ch)			((ch) == ' ' || (ch) == '\t' || (ch) == 0xD931)
#define IsEnglish(ch)			(RANGE(0x0041, ch, 0x005A) || RANGE(0x0061, ch, 0x007A))
#define IsNumber(ch)			RANGE(0x0030, ch, 0x0039)

#define IsSkipChar_KSSM(ch) 		(!(IsEnglish(ch) || IsHangul_KSSM(ch) || IsNumber(ch) || IsEnglish2_KSSM(ch) || IsNumber2_KSSM(ch) || IsHanja_KSSM(ch) || IsDot_KSSM(ch) || IsDash_KSSM(ch) || IsSlash_KSSM(ch))) 


// 한글 KS 코드 체크
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

typedef unsigned short int      tWORD;
typedef unsigned short int      tHCHAR;
typedef long			tLONG;
typedef unsigned long	tDWORD;
typedef unsigned int            tUINT;
typedef unsigned char           tBYTE;
typedef void            tVOID;
typedef int              		tINT;
typedef char             		tCHAR;
typedef char             		tBOOL;

typedef tBOOL (*IsInDicProc)( const tVOID *word, const tVOID *lpParam );
typedef tBOOL (*PutWordFunc)( tHCHAR *uszWord, tDWORD info, tVOID *lpArg1, tVOID *lpArg2 );
typedef tINT (*CompProcP)( const tVOID *elem1, const tVOID *elem2 );
typedef tVOID *(*lpProcP)( tVOID * );
typedef tBOOL (*GetItemProcP)( tCHAR *szWord, tDWORD info, tVOID *lpArg1, tVOID *lpArg2 );
typedef tCHAR *(*lpGetItemProcP)( const tVOID *elem );

#define FREE_MAYBE(foo) do { if (foo) free (foo); } while (0)

#if defined(_DW_LITTLE_ENDIAN)
#define WordCvt(x)  (x)
#define DwordCvt(x) (x)
#define IntCvt(x) (x)
#elif defined(_DW_BIG_ENDIAN)
#define WordCvt(x)  (((((tWORD) (x)) & 0xFF) << 8) | ((((tWORD) (x)) & 0xFF00) >> 8))
#define DwordCvt(x) (((WordCvt((tWORD) (x)) << 16) & 0xFFFF0000) | WordCvt((tWORD) ((x) >> 16)))
#define IntCvt(x) (((WordCvt((tWORD) (x)) << 16) & 0xFFFF0000) | WordCvt((tWORD) ((x) >> 16)))
#endif

#if !defined(_DW_LITTLE_ENDIAN) && !defined(_DW_BIG_ENDIAN)
#error Define _DW_LITTLE_ENDIAN or _DW_BIG_ENDIAN
#endif


#define DW_O_TEXT   0
#define DW_O_BINARY 0

#define DWLOCK_EX 1
#define DWLOCK_UN 2

#define MAX_PHONEWORD 5
typedef struct {
        tHCHAR resWord[MAX_WORD_LEN];
}CORRECT_FORMAT;

#endif /* dwtype.h */

