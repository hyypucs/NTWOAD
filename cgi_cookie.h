/*
	CGI  Cookie manager header
	
	2001. 1. 10. by hyypucs
 */

#if !defined(__CGI_COOKIE_H__)
#define __CGI_COOKIE_H__

// Cookie�� ȯ�溯������ �޾� �ͼ� �ʱ�ȭ��Ŵ
int StartCookieMan(void);

// Cookie ���۸� �� ���ְ� free��Ŵ
void CloseCookieMan(void);

// Cookie Name/Value�� �׳� ����� 
void ViewCookieMan(void);

// Name�� ���ؼ� value�� ��������..
// point�� �޾� ����. ���߿� free�ϸ� �ȵ�!!
int GetCookie(char *szName, char **szValue);


// Cookie��带 �ݵ�� �ش�.
// szHead�� ����� Ŀ�� �Ѵ�.!!
// szName : name
// szValue : name�� ���� value��
// szExpire : �ش� name�� ��ȿ�� ��¥ ���� ctime�� �̿��ؼ� ����
//            ctime�� ���� buf[��-1] = '\0'�ؾ� �Ѵ�. ������ ���� space�� ���ϱ�..
// szPath : �ش� �н� ���Ͽ����� ��ȿ�ϰ� �� �� �ִ�.
// szDomain : �ش� �����ο����� ��ȿ�ϵ��� ��
// bSecure : SSL���� ������.������ 0
void MakeCookieHead(char *szHead, char *szName, char *szValue, char *szExpire, char *szPath, char *szDomain, int bSecure);

// ���� ����� �Լ��̳�. expires�� ����.. ���� �ð����� �� �ð��� ������ ��ȿ���� �ʵ��� �Ѵ�.
void MakeCookieHeadEx(char *szHead, char *szName, char *szValue, char *szPath, char *szDomain, int bSecure, int nAfterHour);


#if defined(_NOT_USE_UTILE)
// ���߿� util.c�� �ű���.
unsigned char Hex2Char(char HexUp, char HexLow);
void HexStr2CharStr(char *HexStr, char *CharStr);
//
// nOpt : 0 -> �⺻ default ��ȯ
//        2byte ���ڿ�  Ư������(isalnum�� �ش� �ȵǴ� �͵� ���) ' ', '+', '"', ''', '\t', '\a', '\n', '=', '&', '%'
//        �� hex�ڵ�� �ٲ۴�.
//
//        1 -> ��� hex�ڵ�� �ٲ۴�.
//
//        2 -> Ư�����ڸ� hex code�� �ٲ۴�.
//
//
void CharStr2HexStr(char *CharStr, char *HexStr, int nOpt);
#endif

#endif /* cgi_cookie.h */

