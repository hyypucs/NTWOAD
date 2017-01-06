/*
	��ũ�Ⱑ ������� �׳� linked list �����Ѵ�.
	�� ���̺귯���� �̿��ؼ� DOCID List �����ϸ� ���� ������ �ʹ�.
	by hyypucs 2007. 11. 7.
 */
#if !defined(_LINKEDLISTDYN_H__)
#define _LINKEDLISTDYN_H__

// ������ ó������ ���� �غ���...
//#define _USE_DUMMY_FILE 1

#define MAX_FILE_LIST 1024

#define OPEN_READ_LIST 0
#define OPEN_RW_LIST   1

typedef struct {
	tBOOL bOpenList;
	tCHAR szListFileName[MAX_PATH]; // List ���� �̸� prefix  

	FILE *fp[MAX_FILE_LIST];
#if defined(_USE_DUMMY_FILE)
	tLONG nPosEnd[MAX_FILE_LIST]; // fseek(END) ���� �̰����� ��ü�Ѵ�. fwriteŸ���� �ʹ� �ɷ��� ..
#endif

	tINT nCountFp; // ���� �� ..

	tINT nSizeItem; // item size
	tINT nLimitFileSize; // List ���� ũ�Ⱑ ��ġ�� error

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
