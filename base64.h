/*
 * mail conversion util
 * ������ ���� ���µ� convert�ʿ��� �� �� ���� ���̴�.
 */

#if !defined(_BASE64_H_)
#define _BASE64_H_

int base64_to_string(char *obuf, char *ibuf);
int string_to_base64(char *obuf, char *ibuf);
int uudecode(char *obuf, char *ibuf);
int uuencode(char *obuf, char *ibuf);
int hyyuudecode(char *obuf, char *ibuf);
int hyyuuencode(char *obuf, char *ibuf);
void ToKSFromStr(char *TarLine, char *Line);
int QP(char *outStr, char *inStr);
void ToHTML(char *buf);
int iso2ks(char *obuf, char *ibuf, int isdecode);

#endif /* base64.h */

