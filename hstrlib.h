/*


        Author : hyypucs
        B-Date : 1999. 6. 16
        Comment:
*/
#if !defined(__HSTRLIB_H__)
#define __HSTRLIB_H_

void hstr2str( unsigned short int *hstr, unsigned char *str);
void str2hstr( unsigned char *str, unsigned short int  *hstr);
int HStrlen(unsigned short int *hstr);
int HStrlenByStr(unsigned short int *hstr);

tBOOL IsEngStr( tHCHAR *uszEngStr );
tBOOL IsUpHStr( tHCHAR *hstr );

tVOID HTrimCR( tHCHAR *hstr );
tVOID HTrim4Adj( tHCHAR *hstr );
tVOID HRTrim( tHCHAR *hstr );
tVOID HLTrim( tHCHAR *hstr );
tVOID HTrim( tHCHAR *hstr );


#if defined(_OLD)
tHCHAR *HStrcpy( tHCHAR *lpDest, tHCHAR *lpSrc );
tHCHAR *HStrncpy( tHCHAR *lpDest, tHCHAR *lpSrc, tINT nLen );
tHCHAR *HStrcat( tHCHAR *lpDest, tHCHAR *lpSrc );
tINT HStrcmp( tHCHAR *hstr1, tHCHAR *hstr2 );
tINT HStricmp( tHCHAR *hstr1, tHCHAR *hstr2 );

tINT HStrncmp( tHCHAR *hstr1, tHCHAR *hstr2, tINT nLen );
tINT HStrnicmp( tHCHAR *hstr1, tHCHAR *hstr2, tINT nLen );

tHCHAR *HStrchr( const tHCHAR *hstr, tHCHAR c );
tHCHAR *HStrichr( const tHCHAR *hstr, tHCHAR c );

tHCHAR *HStrrchr( tHCHAR *hstr, tHCHAR c );
tHCHAR *HStrstr( tHCHAR *hstr1, tHCHAR *hstr2 );
tHCHAR *HStristr( tHCHAR *hstr1, tHCHAR * hstr2 );
#else
tHCHAR *HStrcpy (tHCHAR *dest, const tHCHAR *src);
tHCHAR *HStrncpy (tHCHAR *s1, const tHCHAR *s2, size_t n);
tHCHAR *HStrcat (tHCHAR *dest, const tHCHAR *src);
tINT HStrcmp (const tHCHAR *p1, const tHCHAR *p2);
tINT HStricmp (const tHCHAR *s1, const tHCHAR *s2);
tINT HStrncmp (const tHCHAR *s1, const tHCHAR *s2, size_t n);
tINT HStrnicmp (const tHCHAR *s1, const tHCHAR *s2, size_t n);

tHCHAR *HStrchr( const tHCHAR *hstr, tHCHAR c );
tHCHAR *HStrichr( const tHCHAR *hstr, tHCHAR c );
tHCHAR *HStrrchr (const tHCHAR *s, tHCHAR c);
tHCHAR *HStrstr (const tHCHAR *phaystack, const tHCHAR *pneedle);
tHCHAR *HStristr (const tHCHAR *phaystack, const tHCHAR *pneedle);
#endif


tVOID HStrtoupper( tHCHAR *hstr );
tVOID HStrtolower( tHCHAR *hstr );
tVOID HMemset( tHCHAR *hstr, tHCHAR c, tINT nLen );
tHCHAR HToupper( tHCHAR c );
tHCHAR HTolower( tHCHAR c );
tHCHAR *HStrrev( tHCHAR *hstr, tINT nLen );
tBOOL str2hstrn( tBYTE *str, tHCHAR *hstr, tINT len );
tBOOL hstr2strn( tHCHAR *hstr, tBYTE *str, tINT len );

#endif /* hstrlib.h */

