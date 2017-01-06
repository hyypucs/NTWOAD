/*
    Cache File Function

    setvbuf Àß ¸ø ¹Ï°Ú³×.. ÂÁ..

	2001³â 2¿ù ¸»¿¡.. by hyypucs

*/
#if !defined(__CFILE_H_)
#define __CFILE_H_


typedef struct {
        FILE *fp;
        tBYTE *lpBuf;
}CFILE, *LPCFILE;

tBOOL OpenCFile(CFILE *lpCfp, tCHAR *szFileName, tCHAR *szMode, tINT nCacheSize);
tVOID CloseCFile(CFILE *lpCfp);

#endif /* cfile.h */
