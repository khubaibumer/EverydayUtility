/*
 * utils.h
 *
 *  Created on: Jul 19, 2019
 *      Author: khubaibumer
 *  Description: This is a MACRO Based Utility Header which contains macros for Generic Stuff
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

typedef enum {
	TMP = 0, LOG = 1
} FILE_TYPE;

typedef enum {
	DESKTOP = 0, MOBILE = 2
} PLATFORM_TYPE;

#define TMP_MOBL "/data/local/tmp/tmpfile"
#define TMP_DSKTP "/tmp/tmpfile"

#define LOG_PATH_DESKTP "/tmp"
#define LOG_PATH_MOBIL "/data/local/tmp"

#define GET_FILE(_File, _Platform, _Path, _Mode)									\
	({														\
		FILE *ptr = NULL;											\
		if(_File == TMP)											\
			if(_Platform == DESKTOP)									\
				ptr = ((FILE*) fopen(TMP_DSKTP, _Mode));						\
			else												\
				ptr = ((FILE*) fopen(TMP_MOBL, _Mode));							\
		else													\
			ptr = ((FILE*) fopen(_Path, _Mode));								\
		assert(ptr != NULL);											\
		(ptr);													\
	})

#ifndef SYS_gettid
#define GETTID()													\
	({														\
		pid_t pid = getpid();											\
		(pid);													\
	})
#else
#define GETTID()													\
	({														\
		pid_t tid = syscall(SYS_gettid);									\
		(tid);													\
	})
#endif

#endif /* UTILS_H_ */

#ifndef __TIMESTAMP_UTILS__
#define __TIMESTAMP_UTILS__

static char timebuf[256] = { '\0' };
static inline const char* _get_timestamp() {

	/*	Time Resolution in Nanoseconds	*/
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	size_t offset = strftime(&timebuf[0], 30, "%Y-%m-%d %H:%M:%S",
			gmtime(&ts.tv_sec));
	/*	Adds Milliseconds to buffer		*/
	sprintf(&timebuf[offset], ":%03ld.%03ld", (ts.tv_nsec / (1000 * 1000)),
			(ts.tv_nsec / 1000) % 1000);
	return (const char*) timebuf;
}

#endif /*	__TIMESTAMP_UTILS__	*/

#ifndef __LOGGER_UTILITY__
#define __LOGGER_UTILITY__
#pragma once

#define NO 0
#define YES 1

static char logfilename[512] = { '\0' };
#define LOGFILE_NAME_EXISTS													\
	({															\
		int responce = NO;												\
		if(logfilename[0] == 0 && logfilename[1] == 0)									\
				responce = NO;											\
		else														\
			responce = YES;												\
		(responce);													\
	})

#define SET_LOGFILE(_Path)													\
	({															\
		if(logfilename[0] == 0 && logfilename[1] == 0)									\
			memcpy(&logfilename[0], _Path, sizeof(logfilename));							\
		else														\
			errno = EXFULL, perror("Logfile Already Initiated!");							\
	})

#define CLOSE_LOGFILE()														\
	({															\
		if(av_logf != NULL)												\
			fclose(av_logf);											\
	})

static FILE *av_logf = NULL;

#define __OPEN_LOGFILE__(__logfilename)												\
	({															\
		FILE *ptr = (FILE*) fopen(__logfilename, "a");									\
		assert(ptr != NULL);												\
		setvbuf(ptr, NULL, _IOLBF, 512);										\
		(ptr);														\
	})

#define LOGi(fmt,...)														\
	({															\
		assert(LOGFILE_NAME_EXISTS == YES);										\
		if(av_logf == NULL)												\
			av_logf = __OPEN_LOGFILE__(logfilename);								\
		assert(av_logf != NULL);											\
		fprintf(av_logf, "[%s]\t[0x%08X]\t[INFO]\t", _get_timestamp(), GETTID());					\
		fprintf(av_logf, fmt, __VA_ARGS__);										\
	})

#define LOGd(fmt,...)														\
	({															\
		assert(LOGFILE_NAME_EXISTS == YES);										\
		if(av_logf == NULL)												\
			av_logf = __OPEN_LOGFILE__(logfilename);								\
		assert(av_logf != NULL);											\
		fprintf(av_logf, "[%s]\t[0x%08X]\t[DEBUG]\t", _get_timestamp(), GETTID());					\
		fprintf(av_logf, fmt, __VA_ARGS__);										\
	})

#define LOGe(fmt,...)														\
	({															\
		assert(LOGFILE_NAME_EXISTS == YES);										\
		if(av_logf == NULL)												\
			av_logf = __OPEN_LOGFILE__(logfilename);								\
		assert(av_logf != NULL);											\
		fprintf(av_logf, "[%s]\t[0x%08X]\t[ERROR]\t", _get_timestamp(), GETTID());					\
		fprintf(av_logf, fmt, __VA_ARGS__);										\
	})

#define LOGw(fmt,...)														\
	({															\
		assert(LOGFILE_NAME_EXISTS == YES);										\
		if(av_logf == NULL)												\
			av_logf = __OPEN_LOGFILE__(logfilename);								\
		assert(av_logf != NULL);											\
		fprintf(av_logf, "[%s]\t[0x%08X]\t[WARN]\t", _get_timestamp(), GETTID());					\
		fprintf(av_logf, fmt, __VA_ARGS__);										\
	})

#define LOGf(fmt,...)														\
	({															\
		assert(LOGFILE_NAME_EXISTS == YES);										\
		if(av_logf == NULL)												\
			av_logf = __OPEN_LOGFILE__(logfilename);								\
		assert(av_logf != NULL);											\
		fprintf(av_logf, "[%s]\t[0x%08X]\t[FATAL]\t", _get_timestamp(), GETTID());					\
		fprintf(av_logf, fmt, __VA_ARGS__);										\
	})

#endif	/*	__LOGGER_UTILITY__	*/

#ifndef __SOCKET_UTILITY__
#define __SOCKET_UTILITY__

enum {
	INET, UNIX
};

/*	_Type can be INET/UNIX, _SRV_PATH can be IP/Filesystem Path
 *
 * This Function Returns FileDescriptor of a running specified Server.
 * User should call accept() on this descriptor to recieve an incomming connection and deal accordingly
 *
 * 	*/
#define CREATE_SOCKET_SERVER(_Type, _SRV_PATH, _PORT, _MAX_CLIENTS)										\
	({																	\
		int srv_fd = 0;															\
		if(_Type == INET)														\
			srv_fd = CREATE_INET_SERVER(_SRV_PATH, _PORT, _MAX_CLIENTS);								\
		else if(_Type == UNIX)														\
			srv_fd = CREATE_UNIX_SERVER(_SRV_PATH, _MAX_CLIENTS);									\
		assert(srv_fd != -1);														\
		(srv_fd);															\
	})

#define CREATE_INET_SERVER(_SRV_PATH, _PORT, _MAX_CLIENTS) 											\
		({																\
			int server_fd = -1;													\
			struct sockaddr_in server_addr = { };											\
			server_fd = socket(AF_INET, SOCK_STREAM, 0);										\
			assert(server_fd != -1);												\
			server_addr.sin_family = AF_INET;											\
			server_addr.sin_port = htons(_PORT);											\
			server_addr.sin_addr.s_addr = inet_addr((const char*) _SRV_PATH);							\
			memset(&server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));								\
			assert(bind(server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == 0);					\
			assert(listen(server_fd, _MAX_CLIENTS) == 0);										\
			(server_fd);														\
		})

#define CREATE_UNIX_SERVER(_SRV_PATH, _MAX_CLIENTS) 												\
	({																	\
		int server_fd = -1;														\
		struct sockaddr_un server_addr = { };												\
		unlink((const char*) _SRV_PATH);												\
		server_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);											\
		assert(server_fd != -1);													\
		memset(&server_addr, 0, sizeof(server_addr.sun_path));										\
		server_addr.sun_family = AF_UNIX;												\
		strncpy(server_addr.sun_path, _SRV_PATH, sizeof(server_addr.sun_path) - 1);							\
		assert(bind(server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == 0);						\
		assert(listen(server_fd, _MAX_CLIENTS) == 0);											\
		(server_fd);															\
	})

/*
 *	_Type Can be Unix/TCP, _SRV_PATH can be filesystem Path or IPv4, Port is only required in case of TCP Client
 *	This funtion gives back filedescriptor for client socket and user is expected to send/recieve data using this fd
 *
 *	*/
#define CREATE_SOCKET_CLIENT(_TYPE, _SRV_PATH, _PORT) 												\
	({																	\
		int cli_fd = 0;															\
		if(_TYPE == INET)														\
			cli_fd = CREATE_INET_CLIENT(_SRV_PATH, _PORT);										\
		else if(_TYPE == UNIX)														\
			cli_fd = CREATE_UNIX_CLIENT(_SRV_PATH);											\
		assert(cli_fd != -1);														\
		(cli_fd);															\
	})

#define CREATE_INET_CLIENT(_SRV_PATH, _PORT) 													\
	({																	\
		int client_fd = -1;														\
		struct sockaddr_in client_addr = { };												\
		socklen_t addr_size;														\
		client_fd = socket(AF_INET, SOCK_STREAM, 0);											\
		assert(client_fd != -1);													\
		client_addr.sin_family = AF_INET;												\
		client_addr.sin_port = htons(_PORT);												\
		client_addr.sin_addr.s_addr = inet_addr((const char*) _SRV_PATH);								\
		memset(client_addr.sin_zero, 0, sizeof(client_addr.sin_zero));									\
		addr_size = sizeof(client_addr);												\
		assert(connect(client_fd, (struct sockaddr*) &client_addr, addr_size) == 0);							\
		(client_fd);															\
	})

#define CREATE_UNIX_CLIENT(_SRV_PATH)														\
	({																	\
		int client_fd = -1;														\
		struct sockaddr_un client_addr = { };												\
		client_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);											\
		assert(client_fd != -1);													\
		memset(&client_addr, 0, sizeof(struct sockaddr_un));										\
		client_addr.sun_family = AF_UNIX;												\
		strncpy(client_addr.sun_path, _SRV_PATH, sizeof(client_addr.sun_path) - 1);							\
		assert(connect(client_fd, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_un)) == 0);					\
		(client_fd);															\
	})

#define CLOSE_SOCKET_CONN(_TYPE, _SFD)														\
	({																	\
		assert(shutdown(_SFD, SHUT_RDWR) != 0);												\
		close(_SFD);															\
	})

#endif /*	__SOCKET_UTILITY__	*/
