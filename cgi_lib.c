#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cgi_lib.h"



#if defined(_WINDOWS)
#define strncasecmp(x, y, z) _strnicmp(x, y, z)
#define strcasecmp(x, y) _stricmp(x, y)
#define bcmp(x, y, z) strncmp(x, y, z)
#endif

unsigned char hex2char(char, char);

void MakeHexEncodeFromStr(char *hexamsg, unsigned char *kssmmsg, int opt)
{
	int i=0, j=0;
	char tmpstr[5];
	
	hexamsg[j] = '\0';
	while (kssmmsg[i]) {
		if (opt || (kssmmsg[i] & 0x80)) {
			sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
			strcat(hexamsg, tmpstr);
			if (kssmmsg[i]) {
				sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
				strcat(hexamsg, tmpstr);
			}
		}
		else if (!isalnum((int)(kssmmsg[i])) && kssmmsg[i] != '/') { // 특수 문자.
 //== ' ' || kssmmsg[i] == '+') {
			sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]); 
			strcat(hexamsg, tmpstr);
		}
		else if (kssmmsg[i] == '&') {
                        sprintf(tmpstr , "%%%02x", (unsigned char)kssmmsg[i++]);
                        strcat(hexamsg, tmpstr);
                }
		else {
			sprintf(tmpstr , "%c", (unsigned char)kssmmsg[i++]); 
			strcat(hexamsg, tmpstr);
		}
	}
}

/* Hex 코드로 만들어진 아스키 값을 Char 형의 변수로 바꿔준다. */
unsigned char hex2char( char HexUp, char HexLow ) 
{
	unsigned char ResultChar = 0;

        if (HexUp >= 'A' && HexUp <= 'F') {
                ResultChar = HexUp - 'A' + 10;
        }
        else if (HexUp >= 'a' && HexUp <= 'f') {
                ResultChar = HexUp - 'a' + 10;
        }
        else if (HexUp >= '0' && HexUp <= '9') {
                ResultChar = HexUp - '0';
        }
        else { // exception
                return 0;
        }
        ResultChar *= 16;

        if (HexLow >= 'A' && HexLow <= 'F') {
                ResultChar += (HexLow - 'A' + 10);
        }
        else if (HexLow >= 'a' && HexLow <= 'f') {
                ResultChar += (HexLow - 'a' + 10);
        }
        else if (HexLow >= '0' && HexLow <= '9') {
                ResultChar += (HexLow - '0');
        }
        else { // exception
                return 0;
        }
        return ResultChar;
/*

    register char result_char;
    result_char = ( hex_upper >= 'A' ? ( ( hex_upper & 0xdf ) - 'A' ) + 10 :
                    ( hex_upper - '0' ) );
    result_char *= 16;
    result_char += ( hex_lower >= 'A' ? ( ( hex_lower & 0xdf ) - 'A' ) + 10 :
                     ( hex_lower - '0' ) );
    return result_char;
*/
}


/* 앞에서 언급한 형식으로 encode된 문자열을 처음의 상태로 바꿔주는 루틴 */
unsigned char get_character( Entity_value str )
{
    unsigned char        temp;
    
    if ( str == (Entity_value)NULL ) return '\0';

    if ( *str & 0x80 ) {
	temp = *str;
	return temp;
    }
    
    switch ( *str ) {
      case '+':
        temp = ' ';
        break;
      case '%':
        temp = hex2char( str[1], str[2] );
        break;
      default:
        temp = *str;
    }
    return temp;
}
 
/* 주어진 문자열에서 사용자 입력 항목의 이름을 찾는다. */
Entity_value get_name_from_string( Entity_value str, Entity_value *name, int str_size )
{
    int         i;
    unsigned char        temp;
	int s_size=str_size;
    for (i=0; *str && *str != '&' && *str != '='; str++,i++) { 
                /* '&'나 '='의 경우는 얻고자하는 문자열의 끝을 의미한다. */
        temp = get_character( str );

        if ( temp == '\0') return (Entity_value)NULL;

        (*name)[i] = temp;
 
        if ( i+1 >= s_size ) {
            *name = (Entity_value)realloc( *name, ( s_size *= 2 ) );
            if ( *name == (Entity_value)NULL ) return (Entity_value)NULL;
        }

	if (*str & 0x80) { /* 한글 처리 */
		str+=1;
		i++;
        	(*name)[i] = *str;
        	if ( i+1 >= s_size ) {
            		*name = (Entity_value)realloc( *name, ( s_size *= 2 ) );
            		if ( *name == (Entity_value)NULL ) return (Entity_value)NULL;
        	}
	}
        else if (*str == '%') str+=2;

    }
    (*name)[i] = '\0';

    return str;
}

/* 마찬가지로 입력된 문자열에서 사용자가 입력한 내용을 얻어온다. */
Entity_value get_value_from_string( Entity_value str, Entity_value        *value, int str_size )
{
    int         i;
    unsigned char        temp;
	int s_size = str_size;
	
    for (i=0; *str && *str != '&'; str++,i++) {

        temp = get_character( str );
        if ( temp == '\0') return (Entity_value)NULL;

        (*value)[i] = temp;
 
        if ( i+1 >= s_size ) {
            *value = (Entity_value)realloc( *value, ( s_size *= 2 ) );
            if ( *value == (Entity_value)NULL ) return (Entity_value)NULL;
        }
	if (*str & 0x80) { /* 한글 처리 */
		str+=1;
		i++;
        	(*value)[i] = *str;
        	if ( i+1 >= s_size ) {
            		*value = (Entity_value)realloc( *value, ( s_size *= 2 ) );
            		if ( *value == (Entity_value)NULL ) return (Entity_value)NULL;
        	}
        	(*value)[i+1] = *str;
	}
        else if (*str == '%') str+=2;
    }
    (*value)[i] = '\0';

    return str;
}

/* 문자열에서 사용자가 입력한 각각의 항목을 얻어온다. */
Pform_entry get_entities_from_string( Entity_value str )
{
    Pform_entry         fe;
    int                 str_size = 200;
    Entity_value        temp_name, temp_value;

	
    if ( str == (Entity_value)NULL ) return (Pform_entry)NULL;
    if ( *str == '\0' ) return (Pform_entry)NULL;
    //while ( isspace((int)(*str)) || (char)*str == '&' ) str++;
    while ( !(*str & 0x80) && (isspace((int)(*str)) || (char) *str == '&') ) str++;
    fe = (Pform_entry)malloc( sizeof( Form_entry ) );
    if ( fe == (Pform_entry)NULL ) return (Pform_entry)NULL;
	memset(fe, 0, sizeof(Form_entry));
    temp_name = (char*)malloc( str_size * sizeof( char ) );
    temp_value = (char*)malloc( str_size * sizeof( char ) );
    if ( ! temp_name || ! temp_value ) return (Pform_entry)NULL;

    str = get_name_from_string( str, &temp_name, str_size );
    if ( str == (Entity_value)NULL ) {
        free( temp_name ); free( temp_value );
        return (Pform_entry)NULL;
    }

    while ( !(*str & 0x80) && (isspace((int)(*str)) || (char) *str == '=') ) str++;

    str = get_value_from_string( str, &temp_value, str_size );
    if ( str == (Entity_value)NULL ) {
        free( temp_name ); free( temp_value );
        return (Pform_entry)NULL;
    }

    fe->name = (Entity_value)malloc( ( strlen( temp_name ) + 1 ) *
                                     sizeof( char ) );
    strcpy( fe->name, temp_name );
    fe->value = (Entity_value)malloc( ( strlen( temp_value ) + 1 ) *
                                      sizeof( char ) );
    strcpy( fe->value, temp_value );
	free( temp_name ); free( temp_value );

	
    switch ( *str ) {
      case '&':
        fe->next = get_entities_from_string( str );
        break;
      case '\0':
      default:
        fe->next = (Pform_entry)NULL;
    }
    
    return fe;
}

unsigned char *binstrstr(unsigned char *base,unsigned char *find, long max)
{
	register long i;
	
	for ( i = 0L ; i < max-(long)strlen((char *)find) ; i ++ ) {
		if (base[i] == '-' && base[i+1] == '-' && base[i] == find[0]) {
			if (bcmp((base+i), find, (int)strlen((char *)find)) == 0) { 
				return (base+i);
			}
		}		
	}
	return NULL;
}

#define ST_CONTENT (char *)"Content-Disposition: form-data;"

Pform_entry get_entities_from_stdiowithfile( int length, char *boundary, FILE *stream)
{
    Entity_value        str;
    long                 i;
    Pform_entry         sfe;
    Pform_entry         fe;
    Pform_entry         tfe;
    int                 str_size;
    int                 count_form = 0;
    Entity_value        temp_name, temp_value;
    char *tmpstr, *chkstr;
    char *filename;
    long max, max1;

    str = (Entity_value)malloc( (length+1) * sizeof( char ) );
    if ( str == (Entity_value)NULL ) return (Pform_entry)NULL;
    for ( i = 0; str[i] != EOF && length; str[i] = getc( stream ), i++, length-- );
    str[i] = '\0';

    max = i;

    tmpstr = str;
    if ((sfe = (Pform_entry)malloc(sizeof(Form_entry))) == NULL) {
	free(str);
	return (Pform_entry)NULL;
    }
	memset(sfe, 0, sizeof(Form_entry));
    fe = sfe;
    fe->next = NULL;
    fe->filename = NULL;
 
   
    while (1) { 
    	if ((tmpstr = strstr(tmpstr, ST_CONTENT)) == NULL) {
		printf("start point not found<br>\r");
		break;
	}
	tmpstr += strlen(ST_CONTENT)+1;
	if (strncmp(tmpstr, "name=", 5) != 0) {
    		Print_mimeheader("text/html");
    		printf("<html>\r");
    		printf("<center>\r");
		printf("name= not equal<Br>\r");
		break;
	}
	tmpstr += (strlen("name=")+1);
	str_size = 0;
	while (tmpstr[str_size] != '\"') {
		str_size ++;
        }
	if ((temp_name = (Entity_value)malloc(str_size+1)) == NULL) {
    		Print_mimeheader("text/html");
    		printf("<html>\r");
    		printf("<center>\r");
		printf("mem alloc(%d) error<br>\r", str_size+1);
		break;
	}
	strncpy(temp_name, tmpstr, str_size);
	temp_name[str_size] = '\0';

	fe->name = temp_name;

	tmpstr += (str_size+1); // 2 = skip <">
	if (*tmpstr == 0x0d && *(tmpstr+1) == 0x0a && *(tmpstr+2) == 0x0d && *(tmpstr+3) == 0x0a) { // default name
		tmpstr += 4; // skip CR
		if ((chkstr = strstr(tmpstr, boundary)) == NULL) {
    			Print_mimeheader("text/html");
    			printf("<html>\r");
    			printf("<center>\r");
			printf("1not found<br>\r");
			free(temp_name);
			break;
		}
		str_size = (int)(chkstr - tmpstr)-2;
		//printf("tmp %s<br> chk %s<br>\r", tmpstr, chkstr);
		if ((temp_value = (Entity_value)malloc(str_size+1)) == NULL) {
			free(temp_name);
			break;
		}
		memcpy(temp_value, tmpstr, str_size);
		temp_value[str_size] = '\0'; 
		fe->value = temp_value;
		fe->size = str_size;

		chkstr += strlen(boundary);

		tmpstr = chkstr;

		if (strncmp(tmpstr, "--", 2) == 0) {
			fe->next = NULL;
			break;
		}
	}
	else if (*tmpstr == ';' && *(tmpstr+1) == ' ') { // when file up

		tmpstr += 2; // skip 2 char
		if (strncmp(tmpstr, "filename=", 9) != 0) break;
        	tmpstr += (strlen("filename=")+1);
        	str_size = 0;
        	while (tmpstr[str_size] != '\"') {
               		str_size ++;
        	}
		if ((filename = (char *)malloc(str_size+1)) == NULL) {
			free(temp_name);
			break;
		}
        	memcpy(filename, tmpstr, str_size);
        	filename[str_size] = '\0';
		
		tmpstr += (str_size+1); // 2 = skip <">

		while (*tmpstr) {
			if (*tmpstr == 0x0d && *(tmpstr+1) == 0x0a && *(tmpstr+2) == 0x0d && *(tmpstr+3) == 0x0a) { // default name
				break;
			}
			tmpstr ++;
		}
		if (*tmpstr == '\0') {
			free(temp_name);
			free(filename);
			break;
		}		
		tmpstr += 4; // skip CR (4byte)

		max1 = max - (int)(tmpstr - str + 1);
		if ((chkstr = (char *)binstrstr((unsigned char *)tmpstr, (unsigned char *)boundary, max1)) == NULL) {
    			Print_mimeheader("text/html");
    			printf("<html>\r");
    			printf("<center>\r");
			printf("file : %s, max = %ld<br>\r", filename, max);
			printf("2not found<br>\r");
			free(temp_name);
			free(filename);
			break;
		}
		str_size = (int)(chkstr - tmpstr)-2;
		//printf("tmp %s<br> chk %s<br>\r", tmpstr, chkstr);
		if ((temp_value = (Entity_value)malloc(str_size+1)) == NULL) {
			free(temp_name);
			free(filename);
			break;
		}
		memcpy(temp_value, tmpstr, str_size);
		temp_value[str_size] = '\0'; 
		for ( i = strlen(filename) - 1; i >= 0 ; i -- ) {
			if (filename[i] == '\\' || filename[i] == '/') {
				break;
			}
		}
		if (i) {
			strcpy(filename, filename+i+1);
		}
		
		fe->size = str_size;
		fe->value = temp_value;
		fe->filename = filename;

		chkstr += strlen(boundary);

		tmpstr = chkstr;



		if (strncmp(tmpstr, "--", 2) == 0) {
			fe->next = NULL;
			break;
		}
	}
	else { // exception
    		Print_mimeheader("text/html");
    		printf("<html>\r");
    		printf("<center>\r");
		printf("exist exception in cgi_lib.c, %x, %x\n", *tmpstr, *(tmpstr+1));
		free(temp_name);
		break;
	}
  	if ((tfe=(Pform_entry)malloc(sizeof(Form_entry))) == NULL) {
		fe->next = NULL;
		break;
	}
	memset(tfe, 0, sizeof(Form_entry));
    	tfe->next = NULL;
	fe->next = tfe;
	fe = fe->next;
    	fe->filename = NULL;
	count_form ++;
    }
    free(str);
    if (count_form == 0) {
	free(sfe);
    	return (Pform_entry)NULL;
    }
    return (sfe);
}

/* 표준입력에서 사용자가 입력한 항목을 얻어온다. 이 과정을 모든 표준입력을 
   문자열로 바꾸어 get_entities_from_string 루틴을 이용해 파싱한다. */
Pform_entry get_entities_from_stdio( int length, FILE *stream )
{
    Entity_value        str;
    int                 i;
    Pform_entry  retP;

    str = (Entity_value)malloc( (length+1) * sizeof( char ) );
    if ( str == (Entity_value)NULL ) return (Pform_entry)NULL;
	memset(str, '\0', length+1);

    for ( i = 0; str[i] != EOF && length; str[i] = getc( stream ), i++, length-- );
    
    retP = get_entities_from_string( str );
	
	
    free(str);
	
    return (retP);
}

/*
 * Usable Function Implementation  사용자가 사용하여야 할 함수들
 */

/* 사용자가 입력한 내용을 파싱해 구조체에 담아주는 함수 */
Pform_entry Get_form_entities( Pcgi_info cgi_info )
{
    char *boundary = NULL;

    if ( cgi_info && cgi_info->request_method ) {
        switch ( (int)cgi_info->request_method ) {
          case CGI_METHOD_POST :
			  
			  if ( cgi_info->content_type) {
                	if (! strncasecmp( cgi_info->content_type, "application/x-www-form-urlencoded", 33 ) ) {
                    		return get_entities_from_stdio( cgi_info->content_length, stdin );
			}
                	else if (! strncasecmp( cgi_info->content_type, "multipart/form-data;", 20 ) ) {
				boundary = (char *)strstr(cgi_info->content_type + 20, "boundary=");
				if (boundary != NULL) boundary += (strlen("boundary=") - 2);
				*boundary = '-';
				*(boundary+1) = '-';

                    		return get_entities_from_stdiowithfile( cgi_info->content_length, boundary, stdin );
			}
		}
            break;
          case CGI_METHOD_GET :
            return get_entities_from_string( cgi_info->query_string );
            break;
        }
    }
    return (Pform_entry)NULL;
}

/* 서버가 설정한 환경 변수의 내용을 얻어오는 함수 */
int Get_cgi_info( Pcgi_info cgi_info )
 {
  char  *s;
  //FILE  *f;
 // int   idx;
  Entity_value  temp = NULL;
  cgi_info->content_length = (s = getenv("CONTENT_LENGTH")) ? atoi(s) : 0;
  cgi_info->content_type = getenv("CONTENT_TYPE");
  cgi_info->server_software = getenv("SERVER_SOFTWARE");
  cgi_info->gateway_interface = getenv("GATEWAY_INTERFACE");
  cgi_info->server_protocol = getenv("SERVER_PROTOCOL");
  cgi_info->server_port = getenv("SERVER_PORT");
  temp = getenv( "REQUEST_METHOD" );
  if (temp == NULL || temp[0] == '\0') return 0;
  if ( !strcmp( temp, "POST" ) )
          cgi_info->request_method = CGI_METHOD_POST;
  else if ( !strcmp( temp, "GET" ) )
          cgi_info->request_method = CGI_METHOD_GET;
  else if ( !strcmp( temp, "HEAD" ) )
          cgi_info->request_method = CGI_METHOD_HEAD;
  else if ( !strcmp( temp, "PUT" ) )
          cgi_info->request_method = CGI_METHOD_PUT;
  else cgi_info->request_method = CGI_METHOD_NOT_SET;
  
  cgi_info->http_accept = getenv("HTTP_ACCEPT");
  cgi_info->path_info = getenv("PATH_INFO");
  cgi_info->path_translated = getenv("PATH_TRANSLATED");
  cgi_info->script_name = getenv("SCRIPT_NAME");
  cgi_info->query_string = getenv("QUERY_STRING");
  cgi_info->remote_host = getenv("REMOTE_HOST");
  cgi_info->remote_addr = getenv("REMOTE_ADDR");
  cgi_info->remote_user = getenv("REMOTE_USER");
  cgi_info->auth_type = getenv("AUTH_TYPE");
  cgi_info->remote_ident = getenv("REMOTE_IDENT") ;
  return ( cgi_info->server_name = getenv( "SERVER_NAME" ) ) != NULL;
}

int Print_mimeheader(Entity_value ct)
{
  return ( ct && ( printf( "Content-Type: %s\n\n", ct ) != EOF ) );
}

int Print_url(Pcgi_info cgi_info)
{
  return ( cgi_info &&
          ( printf( "http://%s:%s%s", cgi_info->server_name,
                    cgi_info->server_port,
                    cgi_info->script_name) != EOF ) );
}

int Print_String(char *str, int Color)
{
	char CStr[20];

	switch (Color) {
		case BLACK 	:strcpy(CStr, "Black");
					break;
		case OLIVE 	:strcpy(CStr, "Olive");
					break;
		case TEAL  	:strcpy(CStr, "Teal");
					break;
		case RED   	:strcpy(CStr, "Red");
					break;
		case BLUE  	:strcpy(CStr, "Blue");
					break;
		case MAROON	:strcpy(CStr, "Maroon");
					break;
		case NAVY	:strcpy(CStr, "Navy");
					break;
		case GRAY	:strcpy(CStr, "Gray");
					break;
		case LIME	:strcpy(CStr, "Lime");
					break;
		case FUCHSIA	:strcpy(CStr, "Fuchsia");
					break;
		case WHITE	:strcpy(CStr, "White");
					break;
		case GREEN	:strcpy(CStr, "Green");
					break;
		case PURPLE	:strcpy(CStr, "Purple");
					break;
		case SILVER	:strcpy(CStr, "Silver");
					break;
		case YELLOW	:strcpy(CStr, "Yellow");
					break;
		case AQUA	:strcpy(CStr, "Aqua");
					break;
		default : strcpy(CStr, "No Color");
					break;
	}
	printf("<font color=%s>%s</font>", CStr, str);
	return 1;
}


void OutStr(char *str, char *Color)
{
	printf("<font color=#%s>%s</font>", Color, str);
}

int GetCgiBase(char *CGI_FILE, char *keyword, char *str)
{
	FILE *fp;
	char line[300];
	char key[40];
	int retVal = 0;

	if ((fp = fopen(CGI_FILE, "rt")) == NULL) {
		printf("file [%s] open error!!<br>\r", CGI_FILE);
		return retVal;
	}
	while (fgets(line, 300, fp)) {
		if (strlen(line) < 10) continue;
		if (line[0] == '/') continue;
		if (sscanf(line, "%s = %s", key, str) !=2) {
			continue;
		}
		if (strcmp(key, keyword) == 0) {
			retVal = 1;
			break;
		}
	}
	fclose(fp);
	return retVal;	
}

int OutputText(char *filename)
{
	FILE *fp;
        char line[1024];
        int i;

        if ((fp = fopen(filename, "rt")) == NULL) {
                return 0;
        }
        while (fgets(line, 1024, fp)) {
                if (strncmp(line, "##exec=", 7) == 0) {
                        i = strlen(line);
                        if (line[i-1] == '\n') line[i-1] = '\0';
                        fflush(stdout);
                        system(line+7);
                        fflush(stdout);
                }
                else printf(line);
        }
        fclose(fp);
        fflush(stdout);
        return 1;

/*
	FILE *fp;
	char line[300];

	if ((fp = fopen(filename, "rt")) == NULL) {
		return 0;
	}
	while (fgets(line, 300, fp)) {
		printf(line);
	}
	fclose(fp);	
	return 1;
*/
}

void DeleteMem(Pform_entry p)
{
	Pform_entry temp=NULL;

	while (p) {
		temp = p;
		p = p->next;
		if (temp->name) free(temp->name);
		if (temp->value) free(temp->value);
		if (temp->filename) free(temp->filename);
		free(temp);
	}
}

void MakePrevDir(char *preDir, char *Dir)
{
	int i;
	char tmpDir[256];
	
	strcpy(tmpDir, Dir);
	for (i = strlen(tmpDir) - 2 ; i >= 0 ; i -- ) {
		if (tmpDir[i] == '/') break;
	}
	if (i < 0) {
		strcpy(preDir, "/");
		return;
	}
	else {
		strncpy(preDir, tmpDir, i+1);
		preDir[i+1] = '\0';
	}
}

#if defined(_OLD)
void DelRet(char *str)
{
	int i = strlen(str) - 1;
	
	while (i>=0 && (str[i] == '\n' || str[i] == '\r' || str[i] == ' ')) {
		str[i] = '\0';
		i -- ;
	}
}
#endif

int checkBlank(char *str)
{
	int i;

	for ( i = 0 ; i < (int)strlen(str) ; i ++ ) {
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r') {
			return 0;
		}
	}
	return 1;
}

int ExecProg(char *line)
{

#if !defined(_WINDOWS)
    char command[256];
    char *chpos;
    FILE *fp;

    char buf[4096];

    if (strncmp(line,"<!--#exec=\"",11) != 0 ) return 0;

    if ((chpos = strchr(line+11, '"')) == NULL) return 0;

    strncpy(command, line+11, chpos - (line + 11));

    command[chpos - line - 11] = '\0';

    if ((fp = popen(command, "r")) != NULL) {
        while (fgets(buf, 4096, fp) != NULL) {
            if (strncmp(buf, "Content-type: ",14) == 0) continue;
            (void) printf("%s", buf);
        }
    }
    else return 0;
    pclose(fp);
#endif
    return 1;
}

#if defined(_OLD)
int ExistFile(char *filename)
{
#if defined(_WINDOWS)
		struct _stat buf;
		if (_stat(filename, &buf)==-1) {
               return (0);
        }
#else 
        struct stat buf;
		if (stat(filename, &buf)==-1) {
               return (0);
        }
#endif
        

        if ((buf.st_mode & S_IFMT) == S_IFDIR) return 0;

        return 1;
}
#endif

int GetSizeFile(char *filename)
{
	struct stat buf;

 	if (stat(filename, &buf)==-1) {
 		return (-1);
 	}

 	if ((buf.st_mode & S_IFMT) == S_IFDIR) return -1;
	
	return (int)buf.st_size;
}

/*
	파일 download를 위한 함수 모음 
 */
typedef struct {
	char *ext;
	char *MimeType;
}HtmlHeaderType;

HtmlHeaderType HtmlH[] = {
{".doc", "Application/msword"},
{".xls", "Application/msexcel"},
{".ppt", "Application/powerpoint"},
{".pot", "Application/powerpoint"},
{".txt", "Text/plain"},
{".rtf", "Application/rtf"},
{".jpg", "Image/jpeg"},
{".jpeg", "Image/jpeg"},
{".bmp", "Image/bitmap"},
{".gif", "Image/gif"},
{".tif", "Image/tiff"},
{".pic", "Image/x-pict"},
{".htm", "Text/html"},
{".html", "Text/html"},
{".pdf", "Application/pdf"},
{".mpg", "Video/mpeg"},
{".mov", "Video/quicktime"},
{".au", "Audio/basic"},
{".wav", "Audio/wave"},
{".zip", "Application/zip"},
{".lha", "Application/octet-stream"},
{".lzh", "Application/octet-stream"},
{".arj", "Application/x-compressed"},
{".tar", "Application/x-tar"},
{".z", "Application/x-compressed"},
{".gz", "Application/x-compressed"},
{".tgz", "Application/x-compressed"},
{".ps", "Application/postscript"},
{".eps", "Application/postscript"},
{".ai", "Application/postscript"},
{".dvi", "Application/x-dvi"},
{".latex", "Application/x-latex"},
{".mid", "Application/midi"},
{".ra", "Application/x-realaudio"},
{".tex", "Application/x-tex"},
{".xbm", "Application/x-xbitmap"},
{".avi", "Application/x-msvideo"},
{".wrl", "Application/x-vrml"},
{".vrml", "Application/x-vrml"},
{".hqx", "Application/mac-binhex40"}, 
{"",""}
};

void GetHtmlHeader(char *filename, char *htmlhead)
{
	int i, size;

	for ( i = 0 ; HtmlH[i].ext[0] != '\0' ; i ++ ) {
		size = strlen(filename) - strlen(HtmlH[i].ext);
		if (size < 0) continue;
		if (strcasecmp(filename + size, HtmlH[i].ext) == 0) {
			strcpy(htmlhead, HtmlH[i].MimeType);
			return;
		}
	}
	strcpy(htmlhead, "Application/x-HYYPUCS-Download");
}

void DowningFile(char *filename, char *fname)
{
 	FILE *fp;
	char *buff;
	long size;
	register long i;
	char htmlheader[100];

	if ((fp = fopen(filename, "rb")) == NULL) return;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	printf("Content-Length: %ld\r\n", size);
        printf("Content-disposition: form-data; name=\"filename\"; filename=\"%s\";\r\n", fname);
	GetHtmlHeader(filename, htmlheader);
       	printf("Content-type: %s\r\n\r\n", htmlheader);

	if ((buff = (char *)malloc(size)) == NULL) {
		fclose(fp);
		return;
	}
	fseek(fp, 0L, SEEK_SET);
	fread(buff, size, 1, fp);
	fclose(fp);

	for ( i = 0 ; i < size ; i ++ ) putchar((int)buff[i]);
	free(buff);
}

void Downing(char *url)
{
/*
 	FILE *fp;
	
*/

  	if (strchr(url, ':'))   
    		printf("Location: ");
  	else     
    		printf("Location: http://%s:%s", getenv("SERVER_NAME"), 
           	getenv("SERVER_PORT"));

  	printf("%s%c%c",url,10,10);

  	printf("This document has moved <A HREF=\"%s\">here</A>%c",url,10);
}


int IsPicture(char *filename)
{
	int i, size;
	char *PicExt[] = {".jpg", ".jpeg", ".gif", ""};

	for ( i = 0 ; PicExt[i][0] != '\0' ; i ++ ) {
		size = strlen(filename) - strlen(PicExt[i]);
		if (size < 0) continue;
		if (strcasecmp(filename + size, PicExt[i]) == 0) {
			return 1;
		}
	}
	return 0;
}
