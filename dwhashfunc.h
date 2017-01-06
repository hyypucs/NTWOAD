//
// dwhashfunc.h
// hash함수 모음
// by hyypucs 2005. 4. 15.
//
#if !defined(__DWHASHFUNC_H_)
#define __DWHASHFUNC_H_

tINT HashCal(tCHAR *szStr);
int Hash_BySeed(const unsigned char *str);
int Sample_Hash(const unsigned char *key, int keylen);
tINT SuperFastHash (char * data);

#endif // dwhashfunc.h
