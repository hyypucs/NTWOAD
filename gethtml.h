#if !defined(__GETHTML_H_)

#define __GETHTML_H_


#define CGI_POST_METHOD 0
#define CGI_GET_METHOD  1
#define CGI_HEAD_METHOD 2

// szHost : host
// szPath : http���� ȣ��Ʈ���� ���丮
// szPort : port
// nMethod : POST/GET ��
// szInput : POST�� �� ���⿡ ������ argv�ֱ�
// **RetBuf : ������ ������ ����.. alloc�޴´�. �ݵ�� free�ؾ� �Ѵ�.
// bBIN : binary���� text���� �̰ɷ� RetBuf�� ���� '\0'�� �ִ´�.
// bVirtualHost : virtual host�� ����ϸ�.. TRUE �ؾ� �Ѵ�.
tINT GetHtml(tCHAR *szHost, tCHAR *szPath, tINT nPort, tINT nMethod, tCHAR *szInput, tBYTE **RetBuf, tBOOL bBin, tBOOL bVirtualHost, tINT nTimeOut);

// stMark���� endMark������ �����´�. text�� ���ؼ�.. �׸��� bBin�� ����.
tINT GetHtmlEx(tCHAR *szHost, tCHAR *szPath, tINT nPort, tINT nMethod, tCHAR *szInput, tBYTE **RetBuf, tCHAR *stMark, tCHAR *endMark, tBOOL bVirtualHost, tINT nTimeOut);

tINT GetHtml2(tCHAR *szHost, tCHAR *szPath, tINT nPort, tINT nMethod, tCHAR *szInput, tBYTE **RetBuf, tBOOL bBin, tBOOL bVirtualHost, tINT nConnectTimeOut, tINT nReadTimeOut, tINT nAllocLimit);

#endif /* gethtml.h */
