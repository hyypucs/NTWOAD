/*
    Cache File Function

    setvbuf 잘 못 믿겠네.. 쩝..

	2001년 2월 말에.. by hyypucs

*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dwtype.h"
#include "cfile.h"

tBOOL OpenCFile(CFILE *lpCfp, tCHAR *szFileName, tCHAR *szMode, tINT nCacheSize)
{
	memset(lpCfp, 0, sizeof(CFILE));

	if ((lpCfp->fp = fopen(szFileName, szMode)) == NULL) return FALSE;

	if ((lpCfp->lpBuf = (tBYTE *)malloc(nCacheSize)) == NULL) {
		fclose(lpCfp->fp);
		return FALSE;
	}
	memset(lpCfp->lpBuf, 0, nCacheSize);

	if (setvbuf(lpCfp->fp, (tCHAR *)(lpCfp->lpBuf), _IOFBF, nCacheSize) != 0) {
		fclose(lpCfp->fp);
		free(lpCfp->lpBuf);
		memset(lpCfp, 0, sizeof(CFILE));
                return FALSE;
        }

	return TRUE;	
}

tVOID CloseCFile(CFILE *lpCfp)
{
	if (lpCfp->lpBuf) free(lpCfp->lpBuf);
	if (lpCfp->fp) fclose(lpCfp->fp);
	memset(lpCfp, 0, sizeof(CFILE));
}

