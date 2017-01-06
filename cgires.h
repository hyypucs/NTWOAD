/*
		cgi에서 resource 관리를 따로 하기 위해서 만든 라이브러리이다.
		이를 이용해서 출력을 능동적으로 대처하도록 한다.

		1999. 12. 6.
			by hyypucs
		history :
			1999. 12.20. : IncludeEXE 추가
                                       IncludeCGI 추가
				      사용법 : {$IncludeCGI("실행파일", "인자")}
						"실행파일", "인자"에 변수를 넣고 싶으면
                                                <$변수> 식으로 입력한다.

 */

#if !defined(__CGIRES_H__)
#define __CGIRES_H__

#define MAX_FORMNAME 21
#define MAX_ENVNAME	40

int StartCgiRes(void) ;
int CloseCgiRes(void);
void FreeFormData(void);
void FreeEnvData(void);
int LoadFormData(char *szFileName);
int AppendFormData(char *szName, char *szBuf);
char *GetFormData(char *szName, char **lpszBuf);

int SetEnvData(char *szName, char *szValue);
char *GetEnvData(char *szName, char **lpszValue);
void PrintForm(char *szName);
int SetEnvDataInt(char *szName, int nValue);
void FprintForm(FILE *fp, char *szName);
int DelEnvData(char *szName);

void GetParsedBuf(char *szBuf, char **szRetBuf);
void IncludeCGI(char *szCmd, FILE *ofp);
void IncludeEXE(char *szCmd, FILE *ofp);

#endif /* cgires.h */

