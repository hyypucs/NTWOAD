#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <utime.h>
#include <unistd.h>
#include "dwtype.h"
#include "timecheck.h"

tVOID StartTimeCheck(TIMECHECK_TYPE *lpTime)
{
	memset(lpTime, 0, sizeof(TIMECHECK_TYPE));

	gettimeofday(&(lpTime->tvStart), NULL);
}

tVOID EndTimeCheck(TIMECHECK_TYPE *lpTime)
{
	gettimeofday(&(lpTime->tvEnd), NULL);
}

tLONG GetTimeCheck(TIMECHECK_TYPE *lpTime)
{
	return (1000000L*(lpTime->tvEnd.tv_sec - lpTime->tvStart.tv_sec)+(lpTime->tvEnd.tv_usec - lpTime->tvStart.tv_usec) );
}
