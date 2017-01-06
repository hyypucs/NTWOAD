/*
 *
 */

#define _REENTRANT

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
#ifdef __linux__
#define __USE_XOPEN
#endif
#include <poll.h>
#ifndef INFTIM
#define INFTIM -1
#endif
#include <errno.h>
#include <pthread.h>
#include "dwsock_another.h"

#ifndef SUN_LEN
/* actual length of an initialized sockaddr_un */
#define SUN_LEN(su) \
        (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

/*****************************************************************************
 *
 * Socket functions
 *
 *****************************************************************************/
int daemon_read( int fildes, void *buf, int nbyte, int to )
{
	int i;
	struct pollfd fds[1];

	if ( nbyte == 0 )
	{
		return 0;
	}
	fds[0].fd = fildes;
	fds[0].events = POLLRDNORM;
	fds[0].revents = 0;
	i = poll( fds, 1, to > 0 ? to*1000 : INFTIM );
	if ( i < 0 )
	{
		i = -1;
	}
	else if ( i == 0 )
	{
		errno = ETIMEDOUT;
	}
	else if ( fds[0].revents & (POLLHUP|POLLERR) )
	{
		i = -1;
		errno = ENOTCONN;
	}
	else
	{
		i = read( fildes, buf, nbyte );
		if ( i == 0 )
		{
			i = -1;
			errno = ENOTCONN;
		}
	}
	return i;
}

int daemon_write( int fildes, const void *buf, int nbyte, int to )
{
	int i;
	struct pollfd fds[1];

	if ( nbyte == 0 )
	{
		return 0;
	}
	fds[0].fd = fildes;
	fds[0].events = POLLWRNORM;
	fds[0].revents = 0;
	i = poll( fds, 1, to > 0 ? to*1000 : INFTIM );
	if ( i < 0 )
	{
		i = -1;
	}
	else if ( i == 0 )
	{
		errno = ETIMEDOUT;
	}
	else if ( fds[0].revents & (POLLHUP|POLLERR) )
	{
		i = -1;
		errno = ENOTCONN;
	}
	else
	{
		i = write( fildes, buf, nbyte );
		if ( i == 0 )
		{
			i = -1;
			errno = ENOTCONN;
		}
	}
	return i;
}

//int daemon_read_n( int fd, void *dst, int len, int to )
int daemon_read_n( int fd, unsigned char *dst, int len, int to )
{
	time_t s, e;
	int readn, left = len;

	time( &s );
	e = s + to;
	while ( left > 0 && ( to <= 0 || s < e ) )
	{
		readn = daemon_read( fd, dst, left, to > 0 ? e-s : 0 );
		if ( readn <= 0 ) /* ERROR */
			return readn;
		left -= readn;
		dst += readn;
		time( &s );
	}
	if ( to > 0 && s >= e )
	{
		errno = ETIMEDOUT;
		return -1;
	}
	return (len - left);
}

//int daemon_write_n( int fd, void *src, int len, int to )
int daemon_write_n( int fd, unsigned char *src, int len, int to )
{
	time_t s, e;
	int writen, left = len;

	time( &s );
	e = s + to;
	while ( left > 0 && ( to <= 0 || s < e ) )
	{
		writen = daemon_write( fd, src, left, to > 0 ? e-s : 0 );
		if ( writen <= 0 ) /* ERROR */
			return writen;
		left -= writen;
		src += writen;
		time( &s );
	}
	if ( to > 0 && s >= e )
	{
		errno = ETIMEDOUT;
		return -1;
	}
	return (len - left);
}

int daemon_readline( int fd, char *ptr, int maxlen, int to )
{
	time_t s, e;
	int n, rc;
	char c;

	time( &s );
	e = s + to;
	n = 0;
	while ( 1 )
	{
		time( &s );
		if ( to > 0 && s >= e )
		{
			errno = ETIMEDOUT;
			return -1;
		}
		/* HERE: e-s > 0 */
		if ( ( rc = daemon_read( fd, &c, 1, to > 0 ? e-s : 0 ) ) == 1 ) /*OK*/
		{
			if ( n+1 >= maxlen ) /* overflow */
			{
				return -1;
			}
			n++;
			*ptr++ = c;
			if ( c == '\n' )
			{
				*ptr = 0;
				return n;
			}
		}
		else /* EOF or ERROR */
		{
			return -1;
		}
	}
}

int daemon_readline_buf( int fd, char *ptr, int maxlen, int to,
						 char *buf, int *buf_end )
{
	time_t s, e;
	int n, rc, i, bs, be;
	char c;

	time( &s );
	e = s + to;
	bs = 0;
	be = *buf_end;
	n = 0;
	while ( 1 )
	{
		if ( bs >= be )
		{
			time( &s );
			if ( to > 0 && s >= e )
			{
				errno = ETIMEDOUT;
				*buf_end = 0;
				return -1;
			}
			/* HERE: e-s > 0 */
			rc = daemon_read( fd, buf, maxlen, to > 0 ? e-s : 0 );
			if ( rc <= 0 ) /* EOF or ERROR */
			{
				*buf_end = 0;
				return -1;
			}
			bs = 0;
			*buf_end = be = rc;
		}
		if ( n+1 >= maxlen ) /* overflow */
		{
			*buf_end = 0;
			return -1;
		}
		n++;
		c = buf[bs++];
		*ptr++ = c;
		if ( c == '\n' )
		{
			*ptr = 0;
			for ( i = 0 ; bs < be ; bs++ )
			{
				buf[i++] = buf[bs];
			}
			*buf_end = i;
			return n;
		}
	}
}

/*
 * thread-safe inet_ntoa()
 */
char *daemon_inet_ntoa( char *buf, int buf_size, struct in_addr in )
{
#ifdef __FreeBSD__
	char ret[18];

	strcpy( ret, "[inet_ntoa error]" );
	inet_ntop( AF_INET, &in, ret, sizeof( ret ) );
	strncpy( buf, ret, buf_size-1 );
#else
	strncpy( buf, inet_ntoa( in ), buf_size-1 );
#endif
	buf[buf_size-1] = 0;
	return buf;
}

int daemon_create_server_socket( int port, int backlog )
{
	int opt;
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 )
	{
		printf( "socket() at daemon_create_server_socket(): %s",
						strerror( errno ) );
		return -1;
	}

	/***** MUST *****/
	opt = 1;
	opt = setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
					  sizeof( int ) );
	if ( opt < 0 )
	{
		close( sockfd );
		printf( "setsockopt() at daemon_create_server_socket(): %s",
						strerror( errno ) );
		return -2;
	}
	/***************/

	bzero( (char *)&servaddr, sizeof( servaddr ) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	servaddr.sin_port = htons( port );
	opt = bind( sockfd, (struct sockaddr *)&servaddr, sizeof( servaddr ) );
	if ( opt < 0 )
	{
		close( sockfd );
		printf( "bind() at daemon_create_server_socket(): %s",
						strerror( errno ) );
		return -3;
	}
	if ( listen( sockfd, backlog ) )
	{
		close( sockfd );
		printf( "listen() at daemon_create_server_socket(): %s",
						strerror( errno ) );
		return -4;
	}
	return sockfd;
}

static int make_pathname( const char *pathname )
{
	char *p, path[1024];
	struct stat stbuf;

	strcpy( path, pathname );

	p = strrchr( path, '/' );
	if ( p == NULL )
		return 1;
	
	*p = 0;
	if ( stat( path, &stbuf ) )
	{
		if ( errno != ENOENT ) /* ERROR */
			return 1;

		/* If not exist */
		umask( 0 );
		if ( mkdir( path, 0755 ) )
			return 1;
	}
	return 0;
}

int daemon_create_server_usocket( const char *pathname, int backlog )
{
	int sockfd, servlen;
	struct sockaddr_un servaddr;

	/* create socket */
	sockfd = socket( AF_UNIX, SOCK_STREAM, 0 );
	if ( sockfd < 0 )
	{
		printf( "socket() at daemon_create_server_usocket(): %s",
						strerror( errno ) );
		return -1;
	}

	/* remove */
	unlink( pathname );

	if ( make_pathname( pathname ) )
		return -2;

	bzero( (char *)&servaddr, sizeof( servaddr ) );
	servaddr.sun_family = AF_UNIX;
	strcpy( servaddr.sun_path, pathname );
	servlen = SUN_LEN( &servaddr );
	if ( bind( sockfd, (struct sockaddr *)&servaddr, servlen) < 0 )
	{
		close( sockfd );
		printf( "bind() at daemon_create_server_usocket(): %s",
						strerror( errno ) );
		return -3;
	}
	if ( listen( sockfd, backlog ) )
	{
		close( sockfd );
		printf( "listen() at daemon_create_server_usocket(): %s",
						strerror( errno ) );
		return -4;
	}
	return sockfd;
}

int daemon_create_client_socket( const char *ip, int port, int to )
{
	int i, n;
	struct pollfd fds[1];
	struct sockaddr_in saddr;
	int sfd, flags, errno_save;

	bzero( (char *)&saddr, sizeof(saddr) );
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr( ip );
	saddr.sin_port = htons( port );
        
	if ( ( sfd = socket( AF_INET, SOCK_STREAM, 0) ) < 0 )
	{
		return -1;
	}
	if ( (flags = fcntl( sfd, F_GETFL, 0 )) < 0 )
	{
		errno_save = errno;
		close( sfd );
		errno = errno_save;
		return -1;
	}
	if ( fcntl( sfd, F_SETFL, flags | O_NONBLOCK ) < 0 )
	{
		errno_save = errno;
		close( sfd );
		errno = errno_save;
		return -1;
	}
	n = connect( sfd, (struct sockaddr *)&saddr, sizeof( saddr ) );
	if ( n < 0 )
	{
		if ( errno != EINPROGRESS )
		{
			errno_save = errno;
			close( sfd );
			errno = errno_save;
			return -1;
		}
	}
	if ( n != 0 )
	{
		fds[0].fd = sfd;
		fds[0].events = POLLRDNORM|POLLWRNORM;
		fds[0].revents = 0;
		i = poll( fds, 1, to > 0 ? to*1000 : INFTIM );
		if ( i < 0 )
		{
			errno_save = errno;
			close( sfd );
			errno = errno_save;
			return -1;
		}
		else if ( i == 0 )
		{
			close( sfd );
			errno = ETIMEDOUT;
			return -1;
		}
		else if ( fds[0].revents & (POLLHUP|POLLERR) )
		{
			close( sfd );
			errno = ENOTCONN;
			return -1;
		}
	}
	i = sizeof( saddr );
	if ( getpeername( sfd, (struct sockaddr *)&saddr, (socklen_t *)&i ) < 0 )
	{
		errno_save = errno;
		close( sfd );
		errno = errno_save;
		return -1;
	}
	if ( fcntl( sfd, F_SETFL, flags ) < 0 )
	{
		errno_save = errno;
		close( sfd );
		errno = errno_save;
		return -1;
	}
	return sfd;
}

