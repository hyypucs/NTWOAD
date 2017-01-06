/*
	블럭크기가 상관없이 그냥 linked list 구현한다.
	이 라이브러리를 이용해서 DOCID List 저장하면 되지 않을까 싶다.
	by hyypucs 2007. 11. 7.
 */
#if !defined(_LINKEDLISTDYN_H__)
#define _LINKEDLISTDYN_H__

// 파일을 처음부터 만들어서 해보자...
//#define _USE_DUMMY_FILE 1

#define MAX_FILE_LIST 1024

#define OPEN_READ_LIST 0
#define OPEN_RW_LIST   1

typedef struct {
	tBOOL bOpenList;
	tCHAR szListFileName[MAX_PATH]; // List 파일 이름 prefix  

	FILE *fp[MAX_FILE_LIST];
#if defined(_USE_DUMMY_FILE)
	tLONG nPosEnd[MAX_FILE_LIST]; // fseek(END) 쪽을 이것으로 대체한다. fwrite타임이 너무 걸려서 ..
#endif

	tINT nCountFp; // 파일 수 ..

	tINT nSizeItem; // item size
	tINT nLimitFileSize; // List 파일 크기가 넘치면 error

	tBOOL bOptRW;
	CompProcP CompRecord;
}DYNLIST_FILE_TYPE, *LP_DYNLIST_FILE_TYPE;

tBOOL OpenDynLinkedListFile(DYNLIST_FILE_TYPE *lpListFile, tCHAR *szListFileName, tINT nCountFp, tINT nSizeItem, tINT nLimitFileSize, CompProcP CompRecord, tBOOL bOptRW);

tVOID CloseDynLinkedListFile(DYNLIST_FILE_TYPE *lpListFile);

tBOOL PutItemToDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItemBlock, tINT nItemInBlock, tINT *nRetFpNum, tINT *nRetStPos, tINT *nRetLastPos, tINT *nRetJump);

tBOOL GetItemListFromDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpRecItem);

tBOOL DelItemListFromDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem);
tBOOL UpdateItemListFromDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem, tVOID *lpUpdateItem);

tBOOL DelItemListArrayFromDynLinkedList(DYNLIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpItemRec);

#endif /* linkedlistdyn.h */
