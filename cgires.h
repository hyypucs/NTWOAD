/*
		cgi���� resource ������ ���� �ϱ� ���ؼ� ���� ���̺귯���̴�.
		�̸� �̿��ؼ� ����� �ɵ������� ��ó�ϵ��� �Ѵ�.

		1999. 12. 6.
			by hyypucs
		history :
			1999. 12.20. : IncludeEXE �߰�
                                       IncludeCGI �߰�
				      ���� : {$IncludeCGI("��������", "����")}
						"��������", "����"�� ������ �ְ� ������
                                                <$����> ������ �Է��Ѵ�.

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

