#if !defined(_DOCIDLIST_H_)
#define _DOCIDLIST_H_

#include "linkedlist.h"
#include "linkedlistdyn.h"

#define OPT_APPEND_DOC_ITEM 	0
#define OPT_INSERT_DOC_ITEM	1

typedef struct {
	tBOOL bOpenDocIdList;
	KEYLIST_TYPE KeyList;
	LIST_FILE_TYPE LinkedList;
	DYNLIST_FILE_TYPE DynLinkedList;
}DOCIDLIST_HEADER_TYPE;

tBOOL OpenDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *KeyListDBName, tINT nC_KeyListDB, tINT nPageSize, tINT nCacheSize, tCHAR *LinkedListDBName, tINT nC_LinkedListDB, tINT nSizeItem, tINT nItemInBlock, tINT nLimitFileSize, CompProcP CompRecord, tBOOL bOptRW);

tVOID CloseDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList);

tBOOL PutDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *szKey, tVOID *lpItem, tINT nOpt);
tBOOL DelDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *szKey, tVOID *lpItem);
tBOOL GetDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *szKey, RECORD *lpRecDocIdList);
tBOOL UpdateDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tCHAR *szKey, tVOID *lpItem, tVOID *lpUpdateItem);

// linkedlist에서 dynamic linkedlist로 변경시키 줌. block 개수에 따라 .. linkedlist 의 블럭개수 ..
tBOOL LinkedList2DynLinkedList(DOCIDLIST_HEADER_TYPE *lpDocIdList, tINT nBlockCount);

tBOOL DocIdList2NewDocIdList(DOCIDLIST_HEADER_TYPE *lpDocIdList, DOCIDLIST_HEADER_TYPE *lpNewDocIdList);

#endif /* docidlist.h */
