#if !defined(__GETHTML_H_)

#define __GETHTML_H_


#define CGI_POST_METHOD 0
#define CGI_GET_METHOD  1
#define CGI_HEAD_METHOD 2

// szHost : host
// szPath : http에서 호스트외의 디렉토리
// szPort : port
// nMethod : POST/GET 위
// szInput : POST일 때 여기에 나머지 argv넣기
// **RetBuf : 가져온 데이터 저장.. alloc받는다. 반드시 free해야 한다.
// bBIN : binary인지 text인지 이걸로 RetBuf에 끝에 '\0'를 넣는다.
// bVirtualHost : virtual host를 사용하면.. TRUE 해야 한다.
tINT GetHtml(tCHAR *szHost, tCHAR *szPath, tINT nPort, tINT nMethod, tCHAR *szInput, tBYTE **RetBuf, tBOOL bBin, tBOOL bVirtualHost, tINT nTimeOut);

// stMark에서 endMark까지만 가져온다. text에 한해서.. 그리서 bBin이 없다.
tINT GetHtmlEx(tCHAR *szHost, tCHAR *szPath, tINT nPort, tINT nMethod, tCHAR *szInput, tBYTE **RetBuf, tCHAR *stMark, tCHAR *endMark, tBOOL bVirtualHost, tINT nTimeOut);

tINT GetHtml2(tCHAR *szHost, tCHAR *szPath, tINT nPort, tINT nMethod, tCHAR *szInput, tBYTE **RetBuf, tBOOL bBin, tBOOL bVirtualHost, tINT nConnectTimeOut, tINT nReadTimeOut, tINT nAllocLimit);

#endif /* gethtml.h */
