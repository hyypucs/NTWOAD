#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "dwtype.h"
#include "memchk.h"

#if defined(_MEM_CHK)

tINT gnCountMem = 0;

tVOID InitMemCount(tVOID)
{
	gnCountMem = 0;
}

tINT GetMemCount(tVOID)
{
	return gnCountMem;
}


tVOID *DWmalloc(size_t size)
{
	gnCountMem += size;
	return (malloc(size));
}


tVOID DWfree(void *ptr, size_t size)
{
	if (ptr) {
		gnCountMem -= size;
		free(ptr);
	}
}

tVOID *DWrealloc(void *ptr, size_t size, size_t old_size)
{

	gnCountMem -= old_size;
	gnCountMem += size;
	return (realloc(ptr, size));
}

#endif
