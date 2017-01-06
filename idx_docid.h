#if !defined(__IDX_DOCID_H_)
#define __IDX_DOCID_H_

#define READ_IDXDOCID	0
#define WRITE_IDXDOCID	1

#define MAX_IDXDOCIDFILE	100


typedef struct {
	tCHAR szFileName[MAX_PATH];
	tINT nCount_File;
	tINT nMode;

	FILE *fpIdx[MAX_IDXDOCIDFILE];
	FILE *fpDocIdList[MAX_IDXDOCIDFILE];

	// 읽을 때만 사용..
	RECORD IdxKey[MAX_IDXDOCIDFILE]; // 읽게 되면 미리 메모리에 올릴 것이 있지?
	tINT nCountIdxKey[MAX_IDXDOCIDFILE]; // 파일별 keyword 개수가 몇개나 되지?
	//-------------------
}IDX_DOCID_FORMAT, *LP_IDX_DOCID_FORMAT;


typedef struct {
	tCHAR szKey[MAX_WORD_LEN];
	tINT nPos;
	tINT nCountDocId;
	tINT nWidthDocId;
}IDX_ITEM_FORMAT, *LP_IDX_ITEM_FORMAT;

#define COUNT_IDX_BYBLOCK	100
#define BLOCK_IDX	(sizeof(IDX_ITEM_FORMAT)*COUNT_IDX_BYBLOCK) // 몇개씩 읽을 것인가?


#define MAX_WRITE_IDX_LIMIT	(190000000 / sizeof(IDX_ITEM_FORMAT))
//#define MAX_WRITE_IDX_LIMIT	(10000 / sizeof(IDX_ITEM_FORMAT))

typedef tBOOL (*PutIdxDocIdListFunc)( tCHAR *szKey, RECORD *lpRec );

tBOOL InitIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szFileName, tINT nCount_File, tINT nMode);
tBOOL CloseIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId);

tBOOL SyncIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId);

tINT GetWriteIdxNum(IDX_DOCID_FORMAT *lpIdxDocId);
tBOOL WriteIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szKey, RECORD *lpDocIdList);
tINT CmpIdxKeyItem(IDX_ITEM_FORMAT *item1, IDX_ITEM_FORMAT *item2);
tBOOL ReadIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szKey, RECORD *lpDocIdList, IDX_ITEM_FORMAT *RetIdxItem, tBOOL bReadIdx);
tBOOL DelIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szKey, tVOID *lpDocId, CompProcP CompDocId);
tINT CheckIndexKey(RECORD *lpIdxKey, tCHAR *szKey, IDX_ITEM_FORMAT *lpIdxItem, tBOOL *bAllMatch);
tBOOL TraverseIdxDocId(IDX_DOCID_FORMAT *lpIdxDocId, PutIdxDocIdListFunc PutIdxDocId);
tBOOL TraverseOneIdxDocId(tINT nPos, IDX_DOCID_FORMAT *lpIdxDocId, PutIdxDocIdListFunc PutIdxDocId);

tBOOL MergeIdxDocId(IDX_DOCID_FORMAT *lpIdx_DocId1, IDX_DOCID_FORMAT *lpIdx_DocId2, IDX_DOCID_FORMAT *lpRetIdx_DocId);
tBOOL WriteIdxKey(IDX_DOCID_FORMAT *lpIdxDocId, tCHAR *szKeyFile);

#endif /* idx_docid.h */
