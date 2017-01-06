//
// hash 함수를 만듦. by hyypucs 2005.4.14.
// dangling 를 record 메모리로 사용해서 처리
//

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include "dwtype.h"
#include "seed.h"
#include "dwhashfunc.h"

tINT HashCal(tCHAR *szStr)
{
	int h, i;
	int len;

	len = strlen(szStr);

	h=0;
	
	for (i=0; i<len; ++i) { 
		h += szStr[i]; 
		h += (h<<10); 
		h ^= (h<<6); 
	} 
	
	h += (h<<3); 
	h ^= (h>>11); 
	h += (h<<15); 

	return (h);
}

int Hash_BySeed(const unsigned char *str)
{
    int hash = 0, i, j;
    unsigned char *ustr = (unsigned char *)str;  /* for 8 bit chars handling */

    for (i = j = 0; *ustr; i++) {
       // if (!issymbol(*ustr)) { /* except symbol */
            hash ^= dwir_seed[j & 0x3][*ustr];
            j++;
       // }
        ustr++;
    }
    return (hash & 65535);
}

int Sample_Hash(const unsigned char *key, int keylen)
{

	int ret = 104729;

	for( ; keylen-- > 0; key++ ) {
		ret ^= ( (ret << 5) + *key + (ret >> 2) );
	}

	return((ret & 0x7fffffff ));
}


tINT Get16Bits(char *data);
tINT Get16Bits(char *data)
{
	tINT nTmp;

	nTmp = *data;
	nTmp <<= 8;
	nTmp += *(data+1);
	return (nTmp);
}

tINT SuperFastHash (char * data) 
{

	tINT hash = 0, tmp, len = strlen(data);
	int rem;

    	if (len <= 0 || data == NULL) return 0;

    	rem = len & 3;
    	len >>= 2;

    	/* Main loop */
    	for (;len > 0; len--) {
        	hash  += Get16Bits(data);

        	tmp    = (Get16Bits(data+2) << 11) ^ hash;
        	hash   = (hash << 16) ^ tmp;
        	data  += 4;
        	hash  += hash >> 11;
    	}

    /* Handle end cases */
    switch (rem) {
        case 3: hash += Get16Bits (data);
                hash ^= hash << 16;
                hash ^= data[2] << 18;
                hash += hash >> 11;
                break;
        case 2: hash += Get16Bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += *data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 2;
    hash += hash >> 15;
    hash ^= hash << 10;

    return (hash);
}
