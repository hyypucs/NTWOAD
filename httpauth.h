#if !defined(_HTTPAUTH_H_)
#define _HTTPAUTH_H_

char * create_authorization_line (const char *au, const char *user, const char *passwd, const char *method, const char *path);

#endif /* httpauth.h */
