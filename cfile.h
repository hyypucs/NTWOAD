/*
    Cache File Function

    setvbuf �� �� �ϰڳ�.. ��..

	2001�� 2�� ����.. by hyypucs

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
