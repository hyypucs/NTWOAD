/*
	CGI  Cookie manager header
	
	2001. 1. 10. by hyypucs
 */

#if !defined(__CGI_COOKIE_H__)
#define __CGI_COOKIE_H__

// Cookie를 환경변수에서 받아 와서 초기화시킴
int StartCookieMan(void);

// Cookie 버퍼를 다 없애고 free시킴
void CloseCookieMan(void);

// Cookie Name/Value를 그냥 출력함 
void ViewCookieMan(void);

// Name에 의해서 value값 가져오기..
// point만 받아 오자. 나중에 free하면 안됨!!
int GetCookie(char *szName, char **szValue);


// Cookie헤드를 반들어 준다.
// szHead가 충분히 커야 한다.!!
// szName : name
// szValue : name에 대한 value값
// szExpire : 해당 name이 유효한 날짜 설정 ctime을 이용해서 만듦
//            ctime한 다음 buf[끝-1] = '\0'해야 한다. 이유는 끝에 space가 들어가니깐..
// szPath : 해당 패스 이하에서만 유효하게 할 수 있다.
// szDomain : 해당 도메인에서만 유효하도록 함
// bSecure : SSL에서 동작함.보통은 0
void MakeCookieHead(char *szHead, char *szName, char *szValue, char *szExpire, char *szPath, char *szDomain, int bSecure);

// 위와 비슷한 함수이나. expires가 없고.. 현재 시간에서 몇 시간이 지나면 유효하지 않도록 한다.
void MakeCookieHeadEx(char *szHead, char *szName, char *szValue, char *szPath, char *szDomain, int bSecure, int nAfterHour);


#if defined(_NOT_USE_UTILE)
// 나중에 util.c로 옮기자.
unsigned char Hex2Char(char HexUp, char HexLow);
void HexStr2CharStr(char *HexStr, char *CharStr);
//
// nOpt : 0 -> 기본 default 변환
//        2byte 문자와  특수문자(isalnum에 해당 안되는 것들 모두) ' ', '+', '"', ''', '\t', '\a', '\n', '=', '&', '%'
//        만 hex코드로 바꾼다.
//
//        1 -> 모두 hex코드로 바꾼다.
//
//        2 -> 특수문자만 hex code로 바꾼다.
//
//
void CharStr2HexStr(char *CharStr, char *HexStr, int nOpt);
#endif

#endif /* cgi_cookie.h */

