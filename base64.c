/* @(#)bq.c:encode/decode modules	is@ev.trigem.co.kr	1992.7.22

Copyright (C) 1992 Ienup Sung.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "base64.h"

typedef unsigned long u_long ;

#define B64_0			0x0fc0000
#define B64_1			0x003f000
#define B64_2			0x0000fc0
#define B64_3			0x000003f

#define PAD			64

/* uudecode */
/* Single character decode.  */
#define DEC(Char) (((Char) - ' ') & 077)

/* iso -> ks */
#define isksc(c)   ( (unsigned char) (c) > (unsigned char) '\xa0'   && \
       (unsigned char)  (c) < (unsigned char) '\xff' ) 
#define is7ksc(c)   ( (unsigned char) (c) > (unsigned char) '\x20'   && \
       (unsigned char)  (c) < (unsigned char) '\x7f' ) 
#define BUF 4096
#define SI '\017'
#define SO '\016'
#define KSC 1
#define ASCII 0

/* decoding */
#define EN_BASE64 0
#define EN_QP     1

typedef struct {
	char szCode[40];
        int  nEncoding;
}EncodingType;
/**************/

static unsigned char Hex2Char( char hex_upper, char hex_lower );
int isuuchar(char ch);

static char *b64_alphabet = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

const char uu_std[64] =
{
  '`', '!', '"', '#', '$', '%', '&', '\'',
  '(', ')', '*', '+', ',', '-', '.', '/',
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', ':', ';', '<', '=', '>', '?',
  '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
  'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
  'X', 'Y', 'Z', '[', '\\', ']', '^', '_'
};
/* ENC is the basic 1 character encoding function to make a char printing.  */
#define ENC(Char) (uu_std[(Char) & 077])

int isuuchar(char ch)
{
	int i;

	for ( i = 0 ; i < 64 ; i ++ ) {
		if (uu_std[i] == ch) return i;
	}
	return (-1);
}

/**** B A S E 6 4 _ T O _ S T R I N G ****/

int base64_to_string(char *obuf, char *ibuf)
{
	unsigned long btmp = 0;
	int count = 0;
	int ret_count = 0;

	while (*ibuf)
	{
		if (*ibuf == '+')
			btmp = (btmp << 6) | 62;
		else if (*ibuf == '/')
			btmp = (btmp << 6) | 63;
		else if (isdigit(*ibuf))
			btmp = (btmp << 6) | (52 + *ibuf - '0');
		else if (isupper(*ibuf))
			btmp = (btmp << 6) | (*ibuf - 'A');
		else if (islower(*ibuf))
			btmp = (btmp << 6) | (26 + *ibuf - 'a');
		else  /* In case of pad and error. */
			btmp = (btmp << 6) | 0;
		
		if (++count >= 4)
		{
			*obuf++ = (char)((btmp & 0x0ff0000) >> 16);
			*obuf++ = (char)((btmp & 0x000ff00) >> 8);
			*obuf++ = (char)(btmp & 0x00000ff);

			btmp = 0;
			count = 0;
			ret_count += 3;
		}

		ibuf++;
	}

	*obuf = '\0';

	return ret_count;
}  /* end of base64_to_string(). */


/**** S T R I N G _ T O _ B A S E 6 4 ****/

int string_to_base64(char *obuf, char *ibuf)
{
	unsigned long btmp;
	int i, count;
	int ret_count = 0;

	while (*ibuf)
	{
		for (i = count = 0, btmp = 0; i < 3; i++)
			if (*ibuf)
			{
				btmp = (btmp << 8) | (u_long)(*ibuf++ & 0x0ff);
				count++;
			}
			else
				btmp = (btmp << 8) | (u_long)0;

		*obuf++ = b64_alphabet[(B64_0 & btmp) >> 18];
		*obuf++ = b64_alphabet[(B64_1 & btmp) >> 12];
		*obuf++ = b64_alphabet[count >= 2 ? (B64_2 & btmp) >> 6 : PAD];
		*obuf++ = b64_alphabet[count == 3 ? (B64_3 & btmp) : PAD];
		ret_count += 4;
	}

	*obuf = '\0';
	return ret_count;

}  /* end of string_to_base64(). */

char get_uu_std(char ch)
{
	char ch1;
	
	ch1 = ch - 32;

	while (ch1 >=64) ch1 -= 64;

	if (ch1 < 0) ch1 = 0;

	return (uu_std[(int)ch1]);
}

int hyyuuencode(char *obuf, char *ibuf)
{
	unsigned long btmp;
	int i, count;
	int ret_count = 0;

        count = strlen(ibuf);
	if (count <= 0) return 0;

        *obuf++ = ENC(count);
	ret_count ++;

	while (*ibuf)
	{
		for (i = count = 0, btmp = 0; i < 3; i++)
			if (*ibuf)
			{
				btmp = (btmp << 8) | (u_long)(*ibuf++ & 0x0ff);
				count++;
			}
			else
				btmp = (btmp << 8) | (u_long)0;

		*obuf++ = uu_std[(B64_0 & btmp) >> 18];
		*obuf++ = uu_std[(B64_1 & btmp) >> 12];
		*obuf++ = uu_std[count >= 2 ? (B64_2 & btmp) >> 6 : PAD];
		*obuf++ = uu_std[count == 3 ? (B64_3 & btmp) : PAD];
		ret_count += 4;
	}
        *obuf++ = ENC('\0');
	ret_count ++;
        *obuf++ = '\n';
	ret_count ++;
	*obuf = '\0';

	return ret_count;

}  /* end of string_to_base64(). */

int hyyuudecode(char *obuf, char *ibuf)
{
	unsigned long btmp = 0;
        int ret_count = 0;
	char *p = ibuf, *o = obuf;
	int n, i;

	n = DEC(*p);
	p++;

	if (n<=0) return 0;

	for ( i = 0 ; i < (int)strlen(ibuf) - 1 ; i += 4, n -= 3) {
		if (n <= 0) break;

		if (isuuchar(*p) == -1) {
			*p = uu_std[0];
		}

                btmp = (btmp << 6) | DEC(*p);
		p++;
		if (isuuchar(*p) == -1) {
			*p = uu_std[0];
		}
                btmp = (btmp << 6) | DEC(*p);
		p++;
		if (isuuchar(*p) == -1) {
			*p = uu_std[0];
		}
                btmp = (btmp << 6) | DEC(*p);
		p++;
		if (isuuchar(*p) == -1) {
			*p = uu_std[0];
		}
                btmp = (btmp << 6) | DEC(*p);
		p++;

		if (n >= 1) {
                	*o++ = (char)((btmp & 0x0ff0000) >> 16);
			ret_count ++;
		}
		if (n >=2 ) {
                	*o++ = (char)((btmp & 0x000ff00) >> 8);
			ret_count ++;
		}
		if (n >= 3)  {
                	*o++ = (char)(btmp & 0x00000ff);
			ret_count ++;
		}

                btmp = 0;
        }

        *o = '\0';

        return ret_count;
}



int uuencode (char *obuf, char *ibuf )
{
  int ch, n;
  char *p;
  char buf[80];
  int ret_c = 0;
  char c1, c2;


  if (strlen(ibuf) <= 0) return 0;

  while (*ibuf)
    {
      n = strlen(ibuf);
      if (n >= 45) {
		strncpy(buf, ibuf, 45);
		buf[45] = '\0';
		n = 45;
		ibuf += 45;
      }
      else if (n == 0) break;
      else {
		strcpy(buf, ibuf); 
		ibuf += n;
      }

      *(obuf+ret_c) = ENC(n);
      ret_c ++;

      for (p = buf; n > 2; n -= 3, p += 3)
	{
	  ch = *p >> 2;
      	  *(obuf+ret_c) = ENC(ch);
       	  ret_c ++;
	 
	  ch = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
      	  *(obuf+ret_c) = ENC(ch);
       	  ret_c ++;

	  ch = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
      	  *(obuf+ret_c) = ENC(ch);
       	  ret_c ++;

	  ch = p[2] & 077;
      	  *(obuf+ret_c) = ENC(ch);
       	  ret_c ++;

	}

      if (n != 0)
	break;
      *(obuf+ret_c) = '\n';
      ret_c ++;

    }

  while (n != 0)
    {
      c1 = *p;
      c2 = n == 1 ? 0 : p[1];

      ch = c1 >> 2;
      *(obuf+ret_c) = ENC(ch);
      ret_c ++;

      ch = ((c1 << 4) & 060) | ((c2 >> 4) & 017);
      *(obuf+ret_c) = ENC(ch);
      ret_c ++;

      if (n == 1)
	ch = ENC ('\0');
      else
	{
	  ch = (c2 << 2) & 074;
	  ch = ENC (ch);
	}
      *(obuf+ret_c) = ch;
      ret_c ++;
      ch = ENC ('\0');
      *(obuf+ret_c) = ch;
      ret_c ++;
      *(obuf+ret_c) = '\n';
      ret_c ++;
    }

   ch = ENC ('\0');
   *(obuf+ret_c) = ch;
   ret_c ++;
   *(obuf+ret_c) = '\n';
   ret_c ++;

   *(obuf+ret_c) = '\0';
 
   return (ret_c);
} /* uuencode */

int uudecode(char *obuf, char *ibuf)
{
	char *p;
	int n, count = 0;
	char ch;
	
	p = ibuf;
	n = DEC(*p);
	if (n<=0) return 0;

	for ( ++p ; n > 0; p += 4, n -= 3) {
		if (n >= 3) {
			if (isuuchar(p[0]) == -1) {
				p[0] = uu_std[0];
			}
			if (isuuchar(p[1]) == -1) {
				p[1] = uu_std[0];
			}
			if (isuuchar(p[2]) == -1) {
				p[2] = uu_std[0];
			}
			if (isuuchar(p[3]) == -1) {
				p[3] = uu_std[0];
			}
			ch = DEC (p[0]) << 2 | DEC (p[1]) >> 4;
			obuf[count++] = ch;
              		ch = DEC (p[1]) << 4 | DEC (p[2]) >> 2;
			obuf[count++] = ch;
              		ch = DEC (p[2]) << 6 | DEC (p[3]);
			obuf[count++] = ch;
		}
		else {
			if (n >= 1) {
				if (isuuchar(p[0]) == -1) {
					p[0] = uu_std[0];
				}
				if (isuuchar(p[1]) == -1) {
					p[1] = uu_std[0];
				}
				ch = DEC (p[0]) << 2 | DEC (p[1]) >> 4;
				obuf[count++] = ch;
			}
			if (n >= 2) {
				if (isuuchar(p[1]) == -1) {
					p[1] = uu_std[0];
				}
				if (isuuchar(p[2]) == -1) {
					p[2] = uu_std[0];
				}
				 ch = DEC (p[1]) << 4 | DEC (p[2]) >> 2;
				 obuf[count++] = ch;
			}
		}
	}
	return (count);
}


#define MAX_ET 13
EncodingType gET[MAX_ET] = {
	{"=?B?EUC-KR?", EN_BASE64},
	{"=?Q?EUC-KR?", EN_QP},
	{"=?EUC-KR?B?", EN_BASE64},
	{"=?EUC-KR?Q?", EN_QP},
	{"=?ks_c_5601-1987?B?", EN_BASE64},
	{"=?ks_c_5601-1987?Q?", EN_QP},
	{"=?utf-8?B?", EN_BASE64},
	{"=?utf-8?Q?", EN_QP},
        {"=?iso-8859-1?B?", EN_BASE64},
        {"=?iso-2022-jp?B?", EN_BASE64},
        {"=?iso-2022-kr?B?", EN_BASE64},
        {"=?iso-2022-kr?Q?", EN_QP},
        {"=?iso-8859-1?Q?", EN_QP}
};

	

void ToKSFromStr(char *TarLine, char *Line)
{
        char tmpinStr[1024], tmpoutStr[1024];
        int ptr;
	char *KSLine = TarLine;
	int i;

        while (*Line) {
		for ( i = 0 ; i < MAX_ET ; i ++ ) {
                	if (!strncasecmp(gET[i].szCode, Line, strlen(gET[i].szCode))) {
                    		ptr = 0;
                    		Line += strlen(gET[i].szCode);
                    		while (strncmp("?=",Line,2)!=0) {
                          		tmpinStr[ptr++] = *Line++;
                    		}
                    		tmpinStr[ptr] = '\0';
                    		Line += 2;
				if (gET[i].nEncoding == EN_BASE64) 
                    			base64_to_string(tmpoutStr, tmpinStr);
				else if (gET[i].nEncoding == EN_QP) 
                    			QP(tmpoutStr, tmpinStr);
				else 
					strcpy(tmpoutStr, tmpinStr);
		
                    		ptr = 0;
                    		while(tmpoutStr[ptr]) {
                         		*KSLine++ = tmpoutStr[ptr++];
                    		}
				break;
			}
                } 
                if (i >= MAX_ET) { 
			*KSLine++ = *Line++;
		}
        }
        *KSLine = '\0';
}

int QP(char *outStr, char *inStr)
{
	char *ibuf = inStr;
	char *obuf = outStr;

	while (*ibuf) {
		if (*ibuf == '=' && *(ibuf+1) && *(ibuf+2)) {
			ibuf++;
			*obuf++ = Hex2Char(*ibuf, *(ibuf+1));
			ibuf+=2;
		}
		else *obuf++ = *ibuf++;
	}	
	*obuf = '\0';
	return ((int)(obuf - outStr));
}

static unsigned char Hex2Char( char hex_upper, char hex_lower )
{
    char result_char;
    result_char = ( hex_upper >= 'A' ? ( ( hex_upper & 0xdf ) - 'A' ) + 10 :
                    ( hex_upper - '0' ) );
    result_char *= 16;
    result_char += ( hex_lower >= 'A' ? ( ( hex_lower & 0xdf ) - 'A' ) + 10 :
                     ( hex_lower - '0' ) );
    return result_char;
}

void ToHTML(char *buf)
{
	char *tmpbuf = NULL, *pbuf, *sbuf;
	
	tmpbuf = (char *)malloc(strlen(buf) *  4);
	if (tmpbuf == NULL) return ;
	pbuf = tmpbuf;
	sbuf = buf;
	while (*buf) {
		if (*buf & 0x80) {
			*pbuf ++ = *buf++;
			*pbuf ++ = *buf++;
		}
		else if (*buf == '<') {
			*pbuf ++ = '&';
			*pbuf ++ = 'l';
			*pbuf ++ = 't';
			*pbuf ++ = ';';
			buf ++;
		}
		else if (*buf == '>') {
			*pbuf ++ = '&';
			*pbuf ++ = 'g';
			*pbuf ++ = 't';
			*pbuf ++ = ';';
			buf ++;
		}
		else *pbuf++ = *buf++;
	}
	*pbuf = '\0';
	strcpy(sbuf, tmpbuf);
	free(tmpbuf);
}

int iso2ks(char *obuf, char *ibuf, int isdecode)
{

   int mode=ASCII;
   int i=0;
/*
   int j=0;
   int c;
*/
   int ishangul = 0;
   char *tarbuf=obuf, *line = ibuf;
   
   if ( !isdecode ) {
      	if ( !ishangul ) {
        	while (  line[i] != '\n' && line[i] != EOF  ) {

           		if ( isksc(line[i]) ) {
              			ishangul = 1;
				
				strcpy(tarbuf, "\033$)C");
				tarbuf += strlen("\033$)C");
              			break;
           		}
           		i++;
        	}
	}

      	if ( !ishangul) {
		strcpy(tarbuf, line);
		tarbuf += strlen(line);
    		return (tarbuf - obuf);
         	return 1;
      	}

      	i = 0 ;

      	while (  line[i] != '\n' && line[i] != EOF ) {
        	if ( mode == ASCII && isksc(line[i]))  {
			*tarbuf ++ = SO;
          		*tarbuf ++ = 0x7f & line[i];
          		mode = KSC;
        	}
        	else if ( mode == ASCII && !isksc(line[i]) )
			*tarbuf = line[i];
        	else if ( mode == KSC && isksc(line[i]) )
          		*tarbuf ++ = 0x7f & line[i];
        	else {
			*tarbuf++ = SI;
			*tarbuf++ = line[i];
          		mode = ASCII;
        	}
        	i++;
     	}
     	if ( mode == KSC) 
		*tarbuf ++ = SI;

	*tarbuf ++ = line[i];
       
  } /* end of if */
  else {

      while (  line[i] != '\n' && line[i] != EOF ) {

        if ( ! strncmp(line+i,"\033$)C",4) ) {
            ishangul = 1;
            i+=4;
            continue;
        }

        if ( ! ishangul )
           *tarbuf ++ = line[i];
        else {

           switch( line[i] ) {
   
             case SO:
               mode=KSC;
               break;
             case SI:
               mode=ASCII;
               break;
             default:
               if ( mode==ASCII)
                  *tarbuf ++ = line[i];
               else
                  if ( line[i] != '\x20') 
                     *tarbuf ++ = line[i] | 0x80;
                  else
                     *tarbuf ++ = line[i];
            } /* end of switch */
         }

         i++;

      } /* end of while */

       *tarbuf ++ = line[i];

    }  /* end of else */

    *tarbuf = '\0';
    return (tarbuf - obuf);
}    

