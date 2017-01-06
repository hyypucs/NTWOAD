#if !defined(__MEMCHK_H__)
#define __MEMCHK_H__

#if defined(_MEM_CHK)
tVOID InitMemCount(tVOID);
tINT GetMemCount(tVOID);
tVOID *DWmalloc(size_t size);
tVOID DWfree(void *ptr, size_t size);
tVOID *DWrealloc(void *ptr, size_t size, size_t old_size);
#endif

#endif // memchk.h
