/*

        Author : 
				 hyypucs
        B-Date : 1999. 9. 16

        Comment: 2byte처리 (모든 내부 처리는 2바이트로 한다.)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dwtype.h"
#include "hstrlib.h"

//tBOOL hputc(tHCHAR c);
//tBOOL hputc2(tHCHAR c);

void hstr2str( unsigned short int *hstr, unsigned char *str )
{
        int i, j = 0;

        for ( i = 0 ; hstr[i] != 0x00 ; i++ ) {
                if (hstr[i] >= 0x8000) {
                        str[j++] = (hstr[i] >> 8) & 0xff;
                }
                str[j++] = hstr[i] & 0xff;
        }
        str[j] = '\0';
}

void str2hstr( unsigned char *str, unsigned short int *hstr )
{
        int i, j = 0;

        for ( i = 0 ; str[i] != '\0' ; i++ ) {
                if (str[i] >= 0x80) {
			if (str[i+1]) {
                        	hstr[j++] = (((unsigned short int)str[i]) << 8) | str[i+1];
                        	i++;
			}
			else {
				break; // exception...
			}
                }
                else {
                        hstr[j++] = str[i];
                }
        }
        hstr[j] = 0x00;
}

int HStrlen(unsigned short int *hstr)
{
	tHCHAR *p = hstr;

	while (*p) p++;

	return(p - hstr);
/*
        int i = 0;

        while (hstr[i]) i++;

        return (i);
*/
}

int HStrlenByStr(unsigned short int *hstr)
{
        int i = 0, count=0;

        while (hstr[i]) {
		if (hstr[i] >= 0x8000) count++;
		i++;
		count++;
	}

        return (count);
}

tVOID HTrim( tHCHAR *hstr )
{
	tHCHAR *p = hstr, *q = hstr;

	while (*p != 0x00) {
		if(*p != ' ' && *p != '\t') {
			*q ++ = *p;
		}
		p++;
	}
	*q = 0x00;
}

tVOID HTrimCR( tHCHAR *hstr )
{
	tHCHAR *p = hstr, *q = hstr;

	while (*p != 0x00) {
		if(*p != '\r' && *p != '\n') {
			*q ++ = *p;
		}
		p++;
	}
	*q = 0x00;
}

tVOID HTrim4Adj( tHCHAR *hstr )
{
	tHCHAR *p = hstr, *q = hstr;
	tBOOL bFlag = FALSE;

	while (*p) {
		if(*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') {
			if (!bFlag) {
				bFlag = TRUE;
			}
			else {
				p ++;
				continue;	
			}
			*q ++ = *p;
		}
		else {
			bFlag = FALSE;
			*q ++ = *p;
		}
		p++;
	}
	*q = 0x00;
}

tVOID HRTrim( tHCHAR *hstr )
{
	tINT i;

	for ( i = HStrlen(hstr) - 1 ; i >= 0 ; i -- ) {
		if (hstr[i] == ' ' || hstr[i] == '\t') {
			hstr[i] = 0x00;
			continue;
		}
		break;
	}
	//if (i>=0) hstr[i] = 0x00;

}

tVOID HLTrim( tHCHAR *hstr )
{
	tINT i;
	tINT nLen = HStrlen(hstr);

	for ( i = 0 ; i < nLen ; i ++ ) {
		if (hstr[i] == ' ' || hstr[i] == '\t') continue;
		break;
	}
	if (i < nLen) {
		HStrcpy(hstr, hstr+i);
	}
	else {
		hstr[0] = '\0';
	}
}

tHCHAR *HStrcpy (tHCHAR *dest, const tHCHAR *src)
{
  register tHCHAR *d = dest;
  register const tHCHAR *s = src;

  do
    *d++ = *s;
  while (*s++ != '\0');

  return dest;
}

tHCHAR *HStrncpy (tHCHAR *s1, const tHCHAR *s2, size_t n)
{

	size_t n2 = n << 1; // * 2
	memcpy(s1, s2, n2);
	s1[n] = 0x00; // 혹시 몰라서.. 

	return (s1);
/*
  register tHCHAR c;
  tHCHAR *s = s1;

  --s1;

  if (n >= 4)
    {
      size_t n4 = n >> 2;

      for (;;)
	{
	  c = *s2++;
	  *++s1 = c;
	  if (c == 0x00)
	    break;
	  c = *s2++;
	  *++s1 = c;
	  if (c == 0x00)
	    break;
	  c = *s2++;
	  *++s1 = c;
	  if (c == 0x00)
	    break;
	  c = *s2++;
	  *++s1 = c;
	  if (c == 0x00)
	    break;
	  if (--n4 == 0)
	    goto last_chars;
	}
      n = n - (s1 - s) - 1;
      if (n == 0)
	return s;
      goto zero_fill;
    }

 last_chars:
  n &= 3;
  if (n == 0)
    return s;

  do
    {
      c = *s2++;
      *++s1 = c;
      if (--n == 0)
	return s;
    }
  while (c != 0x00);

 zero_fill:
  do
    *++s1 = 0x00;
  while (--n > 0);

  return s;
*/
}

#if defined(_OLD)
tHCHAR *HStrncpy( tHCHAR *lpDest, tHCHAR *lpSrc, tINT nLen )
{
	tHCHAR *p = lpSrc;
	tHCHAR *q = lpDest;
	
/*
	while ( *p && nLen ) {
		*q ++ = *p ++;
		nLen --;
	}
	*q = '\0';
*/
	memcpy(q, p, nLen * sizeof(tHCHAR));
	
        return(lpDest);

}
#endif

#if defined(_OLD)
tHCHAR *HStrcat( tHCHAR *lpDest, tHCHAR *lpSrc )
{
	tHCHAR *p = lpSrc;
	tHCHAR *q = lpDest;
	
	while (*q) { // null까지 간 다음 
		q++;
	}
	while ( *p ) {
		*q ++ = *p ++;
	}
	*q = '\0';

        return(lpDest);
} 
#else
tHCHAR *
HStrcat (tHCHAR *dest, const tHCHAR *src)
{
  tHCHAR *s1 = dest;
  const tHCHAR *s2 = src;
  register tHCHAR c;

  /* Find the end of the string.  */
  do
    c = *s1++;
  while (c != 0x00);

  /* Make S1 point before the next character, so we can increment
     it while memory is read (wins on pipelined cpus).  */
  s1 -= 2;

  do
    {
      c = *s2++;
      *++s1 = c;
    }
  while (c != 0x00);

  return dest;
}
#endif

#if defined(_OLD)
tINT HStrcmp( tHCHAR *hstr1, tHCHAR *hstr2 ) 
{
	tHCHAR *p = hstr1;
	tHCHAR *q = hstr2;

	while (*p && *q) {
		if (*p == *q) {
			p++;
			q++;
			continue;
		}
		return ( (tINT)*p - (tINT)*q );
	}
	if (*p == 0x00  && *q == 0x00) return 0;
	return ( (tINT)*p - (tINT)*q );
}
#else
tINT HStrcmp (const tHCHAR *p1, const tHCHAR *p2)
{
  register const tHCHAR *s1 = (const tHCHAR *) p1;
  register const tHCHAR *s2 = (const tHCHAR *) p2;
  register tHCHAR c1, c2;

  do
    {
      c1 = (tHCHAR) *s1++;
      c2 = (tHCHAR) *s2++;
      if (c1 == '\0')
	return c1 - c2;
    }
  while (c1 == c2);

  return c1 - c2;
}
#endif

#if defined(_OLD)
tINT HStricmp( tHCHAR *hstr1, tHCHAR *hstr2 ) 
{
	tHCHAR *p = hstr1;
	tHCHAR *q = hstr2;
	tHCHAR p1, q1;

	while (*p && *q) {
		p1 = HToupper(*p);
		q1 = HToupper(*q);
		if (p1 == q1) {
			p++;
			q++;
			continue;
		}
		return ( (tINT)p1 - (tINT)q1 );
	}
	if (*p == 0x00  && *q == 0x00) return 0;
	return ( (tINT)*p - (tINT)*q );
}
#else
tINT HStricmp (const tHCHAR *s1, const tHCHAR *s2)
{
  const tHCHAR *p1 = (const tHCHAR *) s1;
  const tHCHAR *p2 = (const tHCHAR *) s2;
  tHCHAR c1, c2;

  if (p1 == p2)
    return 0;

  do
    {
      c1 = HTolower (*p1++);
      c2 = HTolower (*p2++);
      if (c1 == 0x00)
	break;
    }
  while (c1 == c2);

  return c1 - c2;
}
#endif

#if defined(_OLD)
tINT HStrncmp( tHCHAR *hstr1, tHCHAR *hstr2, tINT nLen )
{
	tHCHAR *p = hstr1;
	tHCHAR *q = hstr2;

	while (nLen) {
		if (*p == *q) {
			if (*p == 0x00) return 0; // 다 같다.. 
			p++;
			q++;
			nLen --;
			continue;
		}
		return ( (tINT)*p - (tINT)*q );
	}
	return 0;
}
#else
tINT HStrncmp (const tHCHAR *s1, const tHCHAR *s2, size_t n)
{
  register tHCHAR c1 = 0x00;
  register tHCHAR c2 = 0x00;

  if (n >= 4)
    {
      size_t n4 = n >> 2;
      do
	{
	  c1 = (tHCHAR) *s1++;
	  c2 = (tHCHAR) *s2++;
	  if (c1 == 0x00 || c1 != c2)
	    return c1 - c2;
	  c1 = (tHCHAR) *s1++;
	  c2 = (tHCHAR) *s2++;
	  if (c1 == 0x00 || c1 != c2)
	    return c1 - c2;
	  c1 = (tHCHAR) *s1++;
	  c2 = (tHCHAR) *s2++;
	  if (c1 == 0x00 || c1 != c2)
	    return c1 - c2;
	  c1 = (tHCHAR) *s1++;
	  c2 = (tHCHAR) *s2++;
	  if (c1 == 0x00 || c1 != c2)
	    return c1 - c2;
	} while (--n4 > 0);
      n &= 3;
    }

  while (n > 0)
    {
      c1 = (tHCHAR) *s1++;
      c2 = (tHCHAR) *s2++;
      if (c1 == 0x00 || c1 != c2)
	return c1 - c2;
      n--;
    }

  return c1 - c2;
}
#endif

#if defined(_OLD)
tINT HStrnicmp( tHCHAR *hstr1, tHCHAR *hstr2, tINT nLen )
{
	tHCHAR *p = hstr1;
	tHCHAR *q = hstr2;
	tHCHAR p1, q1;

	while (nLen) {
		p1 = HToupper(*p);
		q1 = HToupper(*q);
		if (p1 == q1) {
			if (*p == 0x00) return 0; // 다 같다.. 
			p++;
			q++;
			nLen --;
			continue;
		}
		return ( (tINT)p1 - (tINT)q1 );
	}
	return 0;
}
#else
tINT HStrnicmp (const tHCHAR *s1, const tHCHAR *s2, size_t n)
{
  register tHCHAR c1 = 0x00;
  register tHCHAR c2 = 0x00;

  if (n >= 4)
    {
      size_t n4 = n >> 2;
      do
	{
	  c1 = (tHCHAR) HToupper(*s1++);
	  c2 = (tHCHAR) HToupper(*s2++);
	  if (c1 == 0x00 || c1 != c2)
	    return c1 - c2;
	  c1 = (tHCHAR) HToupper(*s1++);
	  c2 = (tHCHAR) HToupper(*s2++);
	  //c1 = (tHCHAR) *s1++;
	  //c2 = (tHCHAR) *s2++;
	  if (c1 == 0x00 || c1 != c2)
	    return c1 - c2;
	  c1 = (tHCHAR) HToupper(*s1++);
	  c2 = (tHCHAR) HToupper(*s2++);
	  //c1 = (tHCHAR) *s1++;
	  //c2 = (tHCHAR) *s2++;
	  if (c1 == 0x00 || c1 != c2)
	    return c1 - c2;
	  c1 = (tHCHAR) HToupper(*s1++);
	  c2 = (tHCHAR) HToupper(*s2++);
	  //c1 = (tHCHAR) *s1++;
	  //c2 = (tHCHAR) *s2++;
	  if (c1 == 0x00 || c1 != c2)
	    return c1 - c2;
	} while (--n4 > 0);
      n &= 3;
    }

  while (n > 0)
    {
      c1 = (tHCHAR) HToupper(*s1++);
      c2 = (tHCHAR) HToupper(*s2++);
      //c1 = (tHCHAR) *s1++;
      //c2 = (tHCHAR) *s2++;
      if (c1 == 0x00 || c1 != c2)
	return c1 - c2;
      n--;
    }

  return c1 - c2;
}
#endif

tHCHAR *HStrchr( const tHCHAR *hstr, tHCHAR c )
{
	const tHCHAR *p = hstr;
	
	while (*p) {
		/*hputc(*p);
		hputc2(c);
		printf("(%d, %d)", *p, c);
		*/
		if (*p == c) {
/*
			hputc(*p);
			hputc2(c);
			printf("(%d, %d)", *p, c);
*/
			return (tHCHAR *)p;
		}
		p++;
	}
	return (tHCHAR *)NULL;
}

tHCHAR *HStrichr( const tHCHAR *hstr, tHCHAR c )
{
	const tHCHAR *p = hstr;
	register tHCHAR p1;
	tHCHAR q1;
	
	q1 = HToupper(c);
	while (*p) {
		p1 = HToupper(*p);	
		if (p1 == q1) return (tHCHAR *)p;
		p++;
	}
	return (tHCHAR *)NULL;
}

#if defined(_OLD)
tHCHAR *HStrrchr( tHCHAR *hstr, tHCHAR c )
{
	tINT i;

	for ( i = HStrlen(hstr) - 1 ; i >= 0 ; i -- ) {
		if (hstr[i] == c) return (hstr + i);
	}

        return NULL;
}
#else
tHCHAR *HStrrchr (const tHCHAR *s, tHCHAR c)
{
  register const tHCHAR *found, *p;

  /* Since strchr is fast, we use it rather than the obvious loop.  */

  if (c == 0x00)
    return HStrchr (s, (tHCHAR)0x00);

  found = NULL;
  while ((p = HStrchr (s, c)) != NULL)
    {
      found = p;
      s = p + 1;
    }

  return (tHCHAR *) found;
}
#endif

#if defined(_OLD)
tHCHAR *HStrstr( tHCHAR *hstr1, tHCHAR *hstr2 )
{
	tHCHAR *p = hstr1;
	tHCHAR *q;
	tINT nLen = HStrlen(hstr2);

	while (*p) {
		//hputc(*p);
		q = HStrchr(p, hstr2[0]);
		if (q == NULL) return NULL;	

		if (HStrncmp(q, hstr2, nLen) == 0) return q;
		//hputc(*q);
		//hputc(*(q+1));
		
		
		p = q + 1;
	}

	return NULL;
}
#else
tHCHAR *HStrstr (const tHCHAR *phaystack, const tHCHAR *pneedle)
{
  register const tHCHAR *haystack, *needle;
  register tHCHAR b, c;

  haystack = (const tHCHAR *) phaystack;
  needle = (const tHCHAR *) pneedle;

  b = *needle;
  if (b != 0x00)
    {
      haystack--;				/* possible ANSI violation */
      do
	{
	  c = *++haystack;
	  if (c == '\0')
	    goto ret0;
	}
      while (c != b);

      c = *++needle;
      if (c == 0x00)
	goto foundneedle;
      ++needle;
      goto jin;

      for (;;)
        {
          register tHCHAR a;
	  register const tHCHAR *rhaystack, *rneedle;

	  do
	    {
	      a = *++haystack;
	      if (a == 0x00)
		goto ret0;
	      if (a == b)
		break;
	      a = *++haystack;
	      if (a == 0x00)
		goto ret0;
shloop:;	    }
          while (a != b);

jin:	  a = *++haystack;
	  if (a == 0x00)
	    goto ret0;

	  if (a != c)
	    goto shloop;

	  rhaystack = haystack-- + 1;
	  rneedle = needle;
	  a = *rneedle;

	  if (*rhaystack == a)
	    do
	      {
		if (a == 0x00)
		  goto foundneedle;
		++rhaystack;
		a = *++needle;
		if (*rhaystack != a)
		  break;
		if (a == 0x00)
		  goto foundneedle;
		++rhaystack;
		a = *++needle;
	      }
	    while (*rhaystack == a);

	  needle = rneedle;		/* took the register-poor approach */

	  if (a == 0x00)
	    break;
        }
    }
foundneedle:
  return (tHCHAR *) haystack;
ret0:
  return 0;
}
#endif


#if defined(_OLD)
tHCHAR *HStristr( tHCHAR *hstr1, tHCHAR * hstr2 )
{

	tHCHAR *p = hstr1;
	tHCHAR *q;
	tINT nLen = HStrlen(hstr2);

	while (*p) {
		q = HStrichr(p, hstr2[0]);
		if (q == NULL) return NULL;	

		if ((HStrnicmp(q, hstr2, nLen)) == 0) return q;
		p = q + 1;
	}

	return NULL;
}
#else
tHCHAR *HStristr (const tHCHAR *phaystack, const tHCHAR *pneedle)
{
  register const tHCHAR *haystack, *needle;
  register tHCHAR b, c;

  haystack = (const tHCHAR *) phaystack;
  needle = (const tHCHAR *) pneedle;

  b = HTolower (*needle);
  if (b != 0x00)
    {
      haystack--;				/* possible ANSI violation */
      do
	{
	  c = *++haystack;
	  if (c == 0x00)
	    goto ret0;
	}
      while (HTolower (c) != b);

      c = HTolower (*++needle);
      if (c == 0x00)
	goto foundneedle;
      ++needle;
      goto jin;

      for (;;)
        {
          register tHCHAR a;
	  register const tHCHAR *rhaystack, *rneedle;

	  do
	    {
	      a = *++haystack;
	      if (a == 0x00)
		goto ret0;
	      if (HTolower (a) == b)
		break;
	      a = *++haystack;
	      if (a == 0x00)
		goto ret0;
shloop:;	    }
          while (HTolower (a) != b);

jin:	  a = *++haystack;
	  if (a == 0x00)
	    goto ret0;

	  if (HTolower (a) != c)
	    goto shloop;

	  rhaystack = haystack-- + 1;
	  rneedle = needle;
	  a = HTolower (*rneedle);

	  if (HTolower (*rhaystack) == a)
	    do
	      {
		if (a == 0x00)
		  goto foundneedle;
		++rhaystack;
		a = HTolower (*++needle);
		if (HTolower (*rhaystack) != a)
		  break;
		if (a == 0x00)
		  goto foundneedle;
		++rhaystack;
		a = HTolower (*++needle);
	      }
	    while (HTolower (*rhaystack) == a);

	  needle = rneedle;		/* took the register-poor approach */

	  if (a == 0x00)
	    break;
        }
    }
foundneedle:
  return (tHCHAR *) haystack;
ret0:
  return 0;
}
#endif

tVOID HStrtoupper( tHCHAR *hstr )
{
	tHCHAR *p = hstr;

	while (*p) {
		if (*p < 0x8000) { // 한글 코드가 아니면 
			*p = toupper(*p);
		}
		p++;
	}
}

tVOID HStrtolower( tHCHAR *hstr )
{
	tHCHAR *p = hstr;

	while (*p) {
		if (*p < 0x8000) { // 한글 코드가 아니면 
			*p = tolower(*p);
		}
		p++;
	}
}

tVOID HMemset( tHCHAR *hstr, tHCHAR c, tINT nLen )
{
	tHCHAR *p = hstr;

	while ( nLen ) {
		*p ++ = c;
		nLen --;
	}
}

tHCHAR HToupper( tHCHAR c )
{
	if (c < 0x8000) {
		return (toupper(c));
	}
	return (c);
}

tHCHAR HTolower( tHCHAR c )
{
	if (c < 0x8000) {
		return (tolower(c));
	}
	return (c);
}

tHCHAR *HStrrev( tHCHAR *hstr, tINT nLen )
{
	tHCHAR *p = hstr;
	tHCHAR *q = hstr + nLen - 1;
	tHCHAR tmpCh;

	while (p < q) {
		tmpCh = *p;
		*p = *q;
		*q = tmpCh;
		p ++;
		q --;
	}

        return(hstr);
}

/* destination length */
tBOOL str2hstrn( tBYTE *str, tHCHAR *hstr, tINT len )
{
        int i, j = 0;

        for ( i = 0 ; str[i] != '\0' ; i++ ) {
		if (j >= len - 1) break;
                if (str[i] >= 0x80) {
                        hstr[j++] = (((unsigned short int)str[i]) << 8) | str[i+1];
                        i++;
                }
                else {
                        hstr[j++] = str[i];
                }
        }
        hstr[j] = 0x00;

	return TRUE;
}

/* destination length */
tBOOL hstr2strn( tHCHAR *hstr, tBYTE *str, tINT len )
{
        int i, j = 0;

        for ( i = 0 ; hstr[i] != 0x00 ; i++ ) {
		if (j >= len - 1) break;
                if (hstr[i] >= 0x8000) {
                        str[j++] = (hstr[i] >> 8) & 0xff;
                }
                str[j++] = hstr[i] & 0xff;
        }
        str[j] = '\0';
	return TRUE;
}
/*
tBOOL hputc(tHCHAR c)
{
	tCHAR szStr[3];

	szStr[0] = c >> 8 & 0xff;
	szStr[1] = c & 0xff;
	szStr[2] = 0;

	printf("[%s]", szStr);
}
tBOOL hputc2(tHCHAR c)
{
	tCHAR szStr[3];

	szStr[0] = c >> 8 & 0xff;
	szStr[1] = c & 0xff;
	szStr[2] = 0;

	printf("{%s}", szStr);
}
*/
