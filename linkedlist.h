/*
	블럭단위로 linked list 구현한다.
	이 라이브러리를 이용해서 DOCID List 저장하면 되지 않을까 싶다.
	by hyypucs 2007. 09. 4.
 */
#if !defined(_LINKEDLIST_H__)
#define _LINKEDLIST_H__

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
	tINT nSizeItem; // 블럭에 들어갈 item size
	tINT nItemInBlock; // 블럭안에 몇개를 넣을까?
	tINT nLimitFileSize; // List 파일 크기가 넘치면 error

	tBOOL bOptRW;
	CompProcP CompRecord;
}LIST_FILE_TYPE, *LP_LIST_FILE_TYPE;

/*
// 이 블럭으로 움직인다.
typedef struct {
	//tBYTE *pBuf; // 블럭이겠지?
	tHCHAR nCItem : 10; // 블럭안에 item 몇개 채웠지?
	tHCHAR nEnd : 6; // 0 : 끝, 1 : 다음으로 가야지. 2보다 크면 nLimitFileSize가 넘었어, 2를 빼면 해당 ListFileName 쪽 위치가 된다. 
	tINT nNextList; // 다음에 몇번째 List 블럭에 있을까? 절대적으로 점프할께 
}LINKEDLIST_TYPE, *LP_LINKEDLIST_TYPE;
*/

/*
	LinkedList파일을 엽니다.
        lpListFile : linkedlist파일 정보헤더변수
        szListFileName : linkedlist파일 이름 앞부분 : _0000, _0001 이런 식으로 파일이 생성됩니다.
	nCountFp : linkedlist파일을 몇개만들어서 진행할까?
	nSizeItem : linkedlist 블럭안에 들어갈 item의 size 
	nItemInBlock : linkedlist 블럭안에 item이 몇개 들어갈까 ? nSizeItem * nItemInBlock 크기가 linkedlist의 블럭 크기가 된다.
	nLimitFileSize : linkedlist파일 하나의 크기 제한 : 2기가가 넘어가면 안될테니 .. 나중에 이를 수정해야 할듯 ..
	nOptRW : write모드냐, read모드냐 : OPEN_READ_LIST, OPEN_RW_LIST 두가지 option이 있다.

	return : TRUE 성공, FALSE 실패
 */
tBOOL OpenLinkedListFile(LIST_FILE_TYPE *lpListFile, tCHAR *szListFileName, tINT nCountFp, tINT nSizeItem, tINT nItemInBlock, tINT nLimitFileSize, CompProcP CompRecord, tBOOL bOptRW);


/*
	linkedlist파일 다 close시킨다.
 */
tVOID CloseLinkedListFile(LIST_FILE_TYPE *lpListFile);


/*
	linkedlist파일에 item을 하나씩 넣는다.. linkedlist 블럭안에 item를 넣고 블럭이 다 차면, linkedlist만들어서 또 넣고 .. 뭐 이런 작업한다.

	lpListFile : linkedlist head
	nSrcFpNum : linkedlist파일 중 어느 파일부터 시작인지?
	nStPos : linkedlist파일 중 시작 위치, item를 처음으로 입력하면 -1로 넣고 nRetStPos를 받아서 관리해야 한다.
	pItem : 넣을 item
	nRetFpNum : item를 저장한 linkedlist파일은?
	nRetStPos : item를 맨 처음 넣을 때 return를 받아서 위에서 관리해야 한다.
		    item를 맨 처음 넣을 때는 nStPos = -1로 넣으면 된다.		
	nRetLastPos : item를 저장한 위치 : 이를 관리하면 나중에 빨리 write하고 싶을 때 하나하나 찾아서 맨 끝으로 찾아가지 말고 nRetLastPos를 이용해서 한번에 위치 찾아 가서 write시키면 속도개선이 된다.
 */
tBOOL PutItemToLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *pItem, tINT *nRetFpNum, tINT *nRetStPos, tINT *nRetLastPos, tINT *nRetJump);

/*
	linkedlist에 저장되어 있는 item를 record에 담아 온다.

	lpListFile : linkedlist head
	nSrcFpNum : 시작되는 linkedlist file
	nStPos : 시작 위치
	lpRecItem : item를 받아울 record : 미리 alloc받아서 넣어야 한다.	
 */
tBOOL GetItemListFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpRecItem);


/*
	linkedlist에서 lpItem를 삭제하기
	lpListFile : linkedlist head
	nSrcFpNum : 시작 파일
	nStPos : 시작 위치
	lpItem : 지울 item
	CompRecord : 저장된 item과 pItem 비교하는 함수 ..		
 */
tBOOL DelItemListFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem);

/*
	linkedlist에서 lpItem를 찾아서 lpUpdateItem으로 수정하여 저장한다.
	
	lpListFile : linkedlist head
	nSrcFpNum : 시작 파일
	nStPos : 시작 위치
	lpItem : 찾을 item
	CompRecord : 찾을 때 비교 함수
	lpUpdateItem : 변경할 item
 */
tBOOL UpdateItemListFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, tVOID *lpItem, tVOID *lpUpdateItem);


/* linkedlist 를 record에 들어있는 item를 다 지운다 ... */
tBOOL DelItemListArrayFromLinkedList(LIST_FILE_TYPE *lpListFile, tINT nSrcFpNum, tINT nStPos, RECORD *lpItemRec);

#endif /* linkedlist.h */
