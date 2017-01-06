#if !defined(__CGI_LIB_H_)
#define __CGI_LIB_H_

typedef char                    *Entity_value;

typedef enum {
    CGI_METHOD_NOT_SET=0,
    CGI_METHOD_GET,
    CGI_METHOD_POST,
    CGI_METHOD_PUT,
    CGI_METHOD_HEAD
} Request_method;               /* 폼에 요구되는 방식의 종류 */

typedef struct _Cgi_info {
  Entity_value  server_software;
  Entity_value  server_name;
  Entity_value  gateway_interface;
  Entity_value  server_protocol;
  Entity_value  server_port;
  Entity_value  http_accept;
  Entity_value  path_info;
  Entity_value  path_translated;
  Entity_value  script_name;
  Entity_value  query_string;
  Entity_value  remote_host;
  Entity_value  remote_addr;
  Entity_value  remote_user;
  Entity_value  auth_type;
  Entity_value  remote_ident;
  Entity_value  content_type;
  int  content_length;
  Request_method request_method;
} Cgi_info, *Pcgi_info;             /* 환경 변수의 내용을 담아준다. */

typedef struct _Form_entry {
  char          *name;
  char          *value;
  char 	 	*filename;
  long          size;
  struct _Form_entry *next;
}Form_entry, *Pform_entry;          /* 사용자의 입력 내용을 저장하는 구조체 */


typedef enum {
OLIVE,
TEAL,
RED,
BLUE,
MAROON,
NAVY,
LIME,
FUCHSIA,
WHITE,
GREEN,
PURPLE,
SILVER,
YELLOW,
AQUA,
BLACK,
GRAY
}ColorType;

Pform_entry Get_form_entities( Pcgi_info );
int Get_cgi_info( Pcgi_info ); 
int Print_mimeheader( Entity_value ); 
int Print_url( Pcgi_info ); 
int Print_String( char *, int );
void OutStr(char *, char *);
unsigned char get_character( char * );
void MakeHexEncodeFromStr(char *hexamsg, unsigned char *kssmmsg, int opt);
int OutputText(char *filename);
void DeleteMem(Pform_entry p);
void MakePrevDir(char *preDir, char *Dir);
void DelRet(char *str);
int checkBlank(char *str);
int ExecProg(char *line);
int ExistFile(char *filename);
int GetSizeFile(char *filename);

void GetHtmlHeader(char *filename, char *htmlhead);
void DowningFile(char *filename, char *fname);
void Downing(char *url);
int IsPicture(char *filename);

#endif /* cgi_lib.h */
