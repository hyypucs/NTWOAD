/*

	Dictionary Signature define

	Author : hyypucs
	B-Date : 1999. 9. 17.
 */

#if !defined(__DICSIG_H__)
#define __DICSIG_H__

#define SIG_BASIC_DIC 		"Basic Dic by DreamWiz V1.0\032"
#define SIG_STOPWORD_DIC 	"Stopword Dic by DreamWiz V1.0\032"
#define SIG_TOSSI_DIC 		"Tossi Dic by DreamWiz V1.0\032"
#define SIG_THE_DIC   		"Thesaurus Dic by DreamWiz V1.0\032"
#define SIG_THE_INFO_DIC   	"Thesaurus Info Dic by DreamWiz V1.0\032"
#define SIG_KEYWORD_DIC 	"Keyword Dic by DreamWiz V1.0\032"
#define SIG_DOCIDINFO_DIC       "DocId Info Dic by DreamWiz V1.0\032"
#define SIG_DOCNAME_DIC    	"DocName Dic by DreamWiz V1.0\032"
#define SIG_CATNAME_DIC    	"CatName Dic by DreamWiz V1.0\032"
#define SIG_KEYNAME_DIC    	"KeyName Dic by DreamWiz V1.0\032" /* 관련 키워드 */
#define SIG_NEWDOCIDINFO_DIC    "IR DocId Info Dic by DreamWiz V1.0\032"

#define BASIC_DICNAME 		"basic"
#define STOPWORD_DICNAME 	"stopword"
#define THE_DICNAME 		"thesa"
#define TOSSI_DICNAME 		"tossi"
#define KEYWORD_DICNAME 	"key"  /* 한글+한자(key_h.dic) - 그외 (key_e.dic) */
#define DONGSA_DICNAME          "dongsa"

#define INC_DICNAME 		"incinfo.dic"
#define INCFREE_DICNAME 	"incfree.dic"
#define DOCIDINFO_DICNAME 	"docidinfo.dic"
#define DOCIDFILE_DICNAME       "docid" /* docid0.dic ... docid10.dic */

#define DOCNAME_DICNAME 	"docname" /* docname_i.dic,  docname_n */
#define NDOCNAME_DICNAME         "ndocname" /* alldoc0.... alldoc4 */
#define IRDOCNAME_DICNAME 	"irdocname" /* irdocname_i.dic,  irdocname_n */

#define CATNAME_DICNAME 	"catname" /* catname_i.dic,  catname_n */
#define KEYNAME_DICNAME 	"keyname" /* keyname_i.dic,  keyname_n */

#define NEWDOCIDINFO_DICNAME 	"irdocidinfo.dic"
#define NEWDOCIDFILE_DICNAME    "irdocid" /* irdocid0.dic ... irdocid10.dic */

#define INDEXTH_DICNAME		"indexth"

#define HOSTNAME_DICNAME        "host.bin"

// ndbm용.
#define DOCKEYLIST_DICNAME      "dockeylist.ndb"
#define TIMEDOC_DICNAME         "timedoc.ndb"
#define KEYBLOB_DICNAME         "keyblob.ndb"

#define THE_HEAD_SIZE 128

#define MAX_EXT_WORD        20 /* 하나의 단어에 대한 유의어 개수는 20개 */

typedef struct {
	tHCHAR szStr[MAX_WORD_LEN];
	tHCHAR szExtStr[MAX_EXT_WORD][MAX_WORD_LEN];
	tWORD   nCountExtStr;
}THEDATA_TYPE;

#endif /* dicsig.h */
