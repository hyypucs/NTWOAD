#if !defined(__INDEXTH_H_)
#define __INDEXTH_H_

#define MAX_THSTR 20
#define C_THSTR 5

typedef struct {
	tCHAR szStr[MAX_THSTR];
	tINT nStr[C_THSTR];
}THSTR_FORMAT, *LPTHSTR_FORMAT;

typedef struct {
	tCHAR szStr[MAX_THSTR];
}STR_FORMAT, *LPSTR_FORMAT;

tBOOL OpenIndexThDic(tCHAR *szDic);
tBOOL CloseIndexThDic(tVOID);
tBOOL GetFindThDic(tCHAR *szStr, RECORD *lpRetRec);

#endif /* indexth.h */

