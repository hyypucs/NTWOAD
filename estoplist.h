/*
  영어 불용어 체크..
	by hyypucs
 */

#if !defined(__ESTOPLIST_H_)
#define __ESTOPLIST_H_

tBOOL IsEStopWord(tCHAR *szWord);
#if !defined(_TEST)
tBOOL IsHEStopWord(tHCHAR *uszWord);
#endif

#endif /* estoplist.h */
