/*
 *
	 최성훈이 만든 socket 함수를 조금 수정함
	필요하면 쓰기 위해서 ..
 */

#ifndef __dwsock_another_h__
#define __dwsock_another_h__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>

#ifdef  __cplusplus
extern "C" {
#endif

int daemon_read( int fildes, void *buf, int nbyte, int to );
int daemon_write( int fildes, const void *buf, int nbyte, int to );
int daemon_read_n( int fd, void *dst, int len, int to );
int daemon_write_n( int fd, void *src, int len, int to );
int daemon_readline( int fd, char *ptr, int maxlen, int to );
int daemon_readline_buf( int fd, char *ptr, int maxlen, int to,
						 char *buf, int *buf_end );
char *daemon_inet_ntoa( char *buf, int buf_size, struct in_addr in );
int daemon_create_server_socket( int port, int backlog );
int daemon_create_server_usocket( const char *pathname, int backlog );
int daemon_create_client_socket( const char *ip, int port, int to );

#ifdef  __cplusplus
}
#endif

#endif /* dwsock_another.h */
