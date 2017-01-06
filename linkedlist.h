/*
	�������� linked list �����Ѵ�.
	�� ���̺귯���� �̿��ؼ� DOCID List �����ϸ� ���� ������ �ʹ�.
	by hyypucs 2007. 09. 4.
 */
#if !defined(_LINKEDLIST_H__)
#define _LINKEDLIST_H__

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
	tINT nSizeItem; // ���� �� item size
	tINT nItemInBlock; // ���ȿ� ��� ������?
	tINT nLimitFileSize; // List ���� ũ�Ⱑ ��ġ�� error

	tBOOL bOptRW;
	CompProcP CompRecord;
}LIST_FILE_TYPE, *LP_LIST_FILE_TYPE;

/*
// �� ������ �����δ�.
typedef struct {
	//tBYTE *pBuf; // ���̰���?
	tHCHAR nCItem : 10; // ���ȿ� item � ä����?
	tHCHAR nEnd : 6; // 0 : ��, 1 : �������� ������. 2���� ũ�� nLimitFileSize�� �Ѿ���, 2�� ���� �ش� ListFileName �� ��ġ�� �ȴ�. 
	tINT nNextList; // ������ ���° List ���� ������? ���������� �����Ҳ� 
}LINKEDLIST_TYPE, *LP_LINKEDLIST_TYPE;
*/

/*
	LinkedList������ ���ϴ�.
        lpListFile : linkedlist���� �����������
        szListFileName : linkedlist���� �̸� �պκ� : _0000, _0001 �̷� ������ ������ �����˴ϴ�.
	nCountFp : linkedlist������ ����� �����ұ�?
	nSizeItem : linkedlist ���ȿ� �� item�� size 
	nItemInBlock : linkedlist ���ȿ� item�� � ���� ? nSizeItem * nItemInBlock ũ�Ⱑ linkedlist�� �� ũ�Ⱑ �ȴ�.
	nLimitFileSize : linkedlist���� �ϳ��� ũ�� ���� : 2�Ⱑ�� �Ѿ�� �ȵ��״� .. ���߿� �̸� �����ؾ� �ҵ� ..
	nOptRW : write����, read���� : OPEN_READ_LIST, OPEN_RW_LIST �ΰ��� option�� �ִ�.

	return : TRUE ����, FALSE ����
 */
tBOOL OpenLinkedListFile(LIST_FILE_TYPE *lpListFile, tCHAR *szListFileName, tINT nCountFp, tINT nSizeItem, tINT nItemInBlock, tINT nLimitFileSize, CompProcP CompRecord, tBOOL bOptRW);


/*
	linkedlist���� �� close��Ų��.
 */
tVOID CloseLinkedListFile(LIST_FILE_TYPE *lpListFile);


/*
	linkedlist���Ͽ� item�� �ϳ��� �ִ´�.. linkedlist ���ȿ� item�� �ְ� ���� �� ����, linkedlist���� �� �ְ� .. �� �̷� �۾��Ѵ�.

	lpListFile : linkedlist head
	nSrcFpNum : linkedlist���� �� ��� ���Ϻ��� ��������?
	nStPos : linkedlist���� �� ���� ��ġ, item�� ó������ �Է��ϸ� -1�� �ְ� nRetStPos�� �޾Ƽ� �����ؾ� �Ѵ�.
	pItem : ���� item
	nRetFpNum : item�� ������ linkedlist������?
	nRetStPos : item�� �� ó�� ���� �� return�� �޾Ƽ� ������ �����ؾ� �Ѵ�.
		    item�� �� ó�� ���� ���� nStPos = -1�� ������ �ȴ�.		
	nRetLastPos : item�� ������ ��ġ : �̸� �����ϸ� ���߿� ���� write�ϰ� ���� �� �ϳ��ϳ� ã�Ƽ� �� ������ ã�ư��� ���� nRetLastPos�� �̿��ؼ� �ѹ��� ��ġ ã�� ���� write��Ű�� �ӵ������� �ȴ�.
 */
tBOOL PutItemToLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *pItem, tINT *nRetFpNum, tINT *nRetStPos, tINT *nRetLastPos, tINT *nRetJump);

/*
	linkedlist�� ����Ǿ� �ִ� item�� record�� ��� �´�.

	lpListFile : linkedlist head
	nSrcFpNum : ���۵Ǵ� linkedlist file
	nStPos : ���� ��ġ
	lpRecItem : item�� �޾ƿ� record : �̸� alloc�޾Ƽ� �־�� �Ѵ�.	
 */
tBOOL GetItemListFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpRecItem);


/*
	linkedlist���� lpItem�� �����ϱ�
	lpListFile : linkedlist head
	nSrcFpNum : ���� ����
	nStPos : ���� ��ġ
	lpItem : ���� item
	CompRecord : ����� item�� pItem ���ϴ� �Լ� ..		
 */
tBOOL DelItemListFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem);

/*
	linkedlist���� lpItem�� ã�Ƽ� lpUpdateItem���� �����Ͽ� �����Ѵ�.
	
	lpListFile : linkedlist head
	nSrcFpNum : ���� ����
	nStPos : ���� ��ġ
	lpItem : ã�� item
	CompRecord : ã�� �� �� �Լ�
	lpUpdateItem : ������ item
 */
tBOOL UpdateItemListFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem, tVOID *lpUpdateItem);


/* linkedlist �� record�� ����ִ� item�� �� ����� ... */
tBOOL DelItemListArrayFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpItemRec);

#endif /* linkedlist.h */
