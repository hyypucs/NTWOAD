#if !defined(__DW_CRYPT_H__)
#define __DW_CRYPT_H__

char *DWCrypt(char *szsrcStr, char *sztarStr);

//
// true/false
//
int DWChkValidCode(char *szStr, char *szCryptStr);

#endif /* dwcrypt.h */
