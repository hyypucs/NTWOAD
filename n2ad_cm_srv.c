#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fnmatch.h>




#include "dwtype.h"
#include "hstrlib.h"
#include "record.h"
#include "profile.h"
#include "dwthread.h"
#include "mutex.h"
#include "safemutex.h"
#include "dwsock.h"
#include "util.h"
#include "log.h"
#include "credis.h"
#include "cJSON.h"


#define INI_FILE "server.ini"
#define SECTION  "SERVER"

#define BUFF_MAXSIZE 4096000

#define INVALID_CMD "invalid command\n"

tINT gnSocketId = -1;
PRIVATE pthread_t ParentThreadId;

#define DEFAULT_LIMITTHREAD 10

tVOID ShutDown(tVOID);
tBOOL InitServer(tCHAR *szRoot);
tVOID CloseServer(tINT nSigNum);
PRIVATE lpProcP QueryProc( tVOID *ThreadParam );
tVOID TerminateProcThread(tINT nSigNum);
tBOOL IsAllowIP(tCHAR *IP);
tVOID LoadAllowIP(tCHAR *szRoot);

tINT gnCLimitThread;

tCHAR gszRoot[256];
tCHAR gszLogFile[256];

int main(tINT argc, tCHAR *argv[])
{
	tINT nCurSocketId;
        struct sockaddr_in sin;
	tINT size;

 REDIS redis;
  REDIS_INFO info;
  char *val, **valv, lstr[50000];
  const char *keyv[] = {"kalle", "adam", "unknown", "bertil", "none"};
  int rc, keyc=5, i;
  double score1, score2;
 char *out;cJSON *json, *item_json;


/*
redis = credis_connect(NULL, 0, 10000);
  if (redis == NULL) {
    printf("Error connecting to Redis server. Please start server to run tests.\n");
    exit(1);
  }

 rc = credis_get(redis, "kalle", &val);
  printf("get kalle returned: %s\n", val);

  rc = credis_hget(redis, "test_n2ad", "key000", &val);
  printf("get key000 returned: %s\n", val);


        json=cJSON_Parse(val);
        if (!json) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
        else
        {
                out=cJSON_Print(json);
                printf("%s\n",out);
                free(out);


		item_json = cJSON_GetObjectItem(json, "pid");

		if (item_json->type == cJSON_String) {
			printf("string : %s, valuestring : %s\n", item_json->string, item_json->valuestring);
		}
		else if (item_json->type == cJSON_Number) {
			printf("string : %s, valueint : %d\n", item_json->string, item_json->valueint);
			printf("string : %s, valueint : %f\n", item_json->string, item_json->valuedouble);
		}
		
		out = cJSON_Print(item_json);
                printf("getobject : %s\n",out);
                free(out);

                cJSON_Delete(item_json);
                cJSON_Delete(json);
        }

	

credis_close(redis);

	return 0;

*/



	if (argc < 2) {
		printf("n2ad_cm_srv.exe [root for newir]\n");
		return 0;
	}
	strcpy(gszRoot, argv[1]);

	if (InitServer(gszRoot) == FALSE) {
		printf("Init Server Error.\n");
		CloseServer(SIGTERM);
		printf("Close Server!!\n");
		return 0;
	}

	if (RegistFullSig(CloseServer) == FALSE) {
		CloseServer(SIGTERM);
		printf("Signal error!!\n");
		return 0;
	}

	LoadAllowIP(gszRoot);

	size = sizeof(sin);
	while (1) {
		nCurSocketId = accept(gnSocketId, (struct sockaddr *)&sin, (socklen_t *)&size);
		if (nCurSocketId == -1 && errno != EINTR) {
			printf("Accept Socket Error!!\n");
			break;
		}
		if (nCurSocketId < 0) {
			continue;
		}


		printf("IP address is: %s\n", inet_ntoa(sin.sin_addr));
		printf("port is: %d\n", (int) ntohs(sin.sin_port));

		if (IsAllowIP(inet_ntoa(sin.sin_addr))==FALSE) {

			DateLogWrite(gszLogFile, "no allow : %s", inet_ntoa(sin.sin_addr));

			printf("not allow ip\n");
			SocketWrite(nCurSocketId, (tBYTE *)"NOT ALLOW\n", strlen("NOT ALLOW\n"), 3);
			close(nCurSocketId);
			continue;
		}

		if (GetThreadNum() >= gnCLimitThread) { // thread  생성이 많아서 ..
                        //SocketWriteStream(nCurSocketId, (tBYTE *)"THREAD_ERROR", strlen("THREAD_ERROR")+1, 5);
			SocketWrite(nCurSocketId, (tBYTE *)"THREAD_ERROR\n", strlen("THREAD_ERROR\n"), 3);
                        if (nCurSocketId >= 0) close(nCurSocketId);
			continue;
		}

                if(CreateThread(NULL, (lpProcP)QueryProc, (tVOID *)(nCurSocketId)) == FALSE) {
                        //SocketWriteStream(nCurSocketId, (tBYTE *)"THREAD_ERROR", strlen("THREAD_ERROR")+1, 5);
			SocketWrite(nCurSocketId, (tBYTE *)"THREAD_ERROR\n", strlen("THREAD_ERROR\n"), 3);
                        if (nCurSocketId >= 0) close(nCurSocketId);
                }
	}
	

	CloseServer(SIGTERM);
	return 1;
}

tCHAR gszAllowIP[20][100];
tINT gnAllowIP = 0;

tVOID LoadAllowIP(tCHAR *szRoot)
{
	tCHAR szAllowIP[1024];
	tCHAR szDocName[MAX_PATH], szRoot1[MAX_PATH];
	tCHAR *token;

	SetRootDir(szRoot);
	GetRootDir(szRoot1);
	sprintf(szDocName, "%sini/%s", szRoot1, INI_FILE);

	sprintf(gszLogFile, "%slogs/cm_srv.log", szRoot1);

	if (GetProfileStrEx(szDocName, SECTION, "ALLOWIP", szAllowIP, 1024, 1) == FALSE) {
		return ;
	}


	token = strtok(szAllowIP, ",");
	while (token) {
		printf("allow IP : %s\n", token);

		if (gnAllowIP < 20) strcpy(gszAllowIP[gnAllowIP ++], token);
    		token = strtok(NULL, " ");
	}


}

tBOOL IsAllowIP(tCHAR *IP)
{

	tINT i;

	for (i = 0 ; i < gnAllowIP ; i++ ) {
		printf("ip check => %s : %s\n", IP, gszAllowIP[i]);
		if (strchr(gszAllowIP[i], '*') || strchr(gszAllowIP[i], '?')) {
			if (fnmatch(gszAllowIP[i], IP, FNM_CASEFOLD)==0) {
				printf("IP Allow OK\n");
				return TRUE;
			}
		}
		else if (strcmp(IP, gszAllowIP[i]) == 0) {
			printf("IP Allow OK\n");
			return TRUE;
		}
	}

	
	return FALSE;
}

tBOOL InitServer(tCHAR *szRoot)
{
	tCHAR szDocName[MAX_PATH], szRoot1[MAX_PATH];
	tCHAR szIP[100], szPORT[10], szBuf[10];
	tINT  nPort;
	FILE *fp;

	StartThreadLayer();

	printf("N2AD CM Server\n");
	printf("           Copyright 2014 N2AD Inc. All rights reserved.\n");

	SetRootDir(szRoot);
	GetRootDir(szRoot1);
	sprintf(szDocName, "%sini/%s", szRoot1, INI_FILE);

	printf("****************** ENVIRONMENT ******************\n");

	if (GetProfileStrEx(szDocName, SECTION, "SERVERIP", szIP, 100, 1) == FALSE) {
		printf("Server IP not found %s\n", szDocName);	
		return FALSE;
	}
	if (GetProfileStrEx(szDocName, SECTION, "SERVERPORT", szPORT, 10, 1) == FALSE) {
		printf("Server PORT not found %s\n", szDocName);	
		return FALSE;
	}
	printf("SERVERIP : %s\n", szIP);
	printf("SERVERPORT : %s\n", szPORT);

	nPort = atoi(szPORT);

	if (GetProfileStrEx(szDocName, SECTION, "THREAD_COUNT", szBuf, 10, 1) == TRUE) {
                gnCLimitThread = atoi(szBuf);
                if (gnCLimitThread < 0)  gnCLimitThread = 10;
                if(gnCLimitThread > 1000) {
                        gnCLimitThread = 1000;
                        printf("\tMax thread count is 10000\n");
                        printf("\tUser setting thread count is %s\n", szBuf);
                }
        }
        else {
                gnCLimitThread = DEFAULT_LIMITTHREAD;
        }

	printf("THREAD_COUNT : %d\n", gnCLimitThread);


	gnSocketId = SocketServer( szIP, nPort );
	if (gnSocketId == -1) {
		printf("Server %s:%d Init Error. check server!\n", szIP, nPort);
		return FALSE;
	}

	printf("Thread Server started. \n");

	ParentThreadId = pthread_self();

	sprintf(szDocName, "%slogs/pid.log", szRoot1);
	fp = fopen(szDocName, "wt");
	fprintf(fp, "%d\n", (int)getpid());
	fclose(fp);
	
		
	return TRUE;
}

tVOID CloseServer(tINT nSigNum)
{
	if (gnSocketId >= 0) close(gnSocketId);
	gnSocketId = -1;
	printf("shutdown start\n");

	SetCreateThreadFlag(FALSE); // thread 생성하지 못하게 한다.
        WaitingThreadEnd(0);

	EndThreadLayer();
	printf("ok shutdown\n");
	exit(0);
}

tVOID TerminateProcThread(tINT nSigNum)
{
	tINT nCurThreadId;


        nCurThreadId = (tINT)pthread_self();

        printf("Thread TimeOut : %d\n", nCurThreadId);

        DestroyThread((pthread_t)0);
}

PRIVATE lpProcP QueryProc( tVOID *ThreadParam )
{
        tINT nCurSocketId = (long)ThreadParam;

	tBYTE retBuff[BUFF_MAXSIZE];
	pthread_t CurThreadId = pthread_self();
	tCHAR cmd_str[1024];
	tCHAR Msg[1024];
	tCHAR *pCMStr=NULL;
	tINT npCMStr = 0, nRead;
	
	if(RegistOneSig(SIGUSR1, (SigProcP)TerminateProcThread) == (tVOID *)SIG_ERR) {
		if (nCurSocketId != -1) close(nCurSocketId);
		DestroyThread((tVOID *)0);
		return (lpProcP)TRUE;
        }
	
	if (SocketWrite(nCurSocketId, (tBYTE *)"connect ok\n", strlen("connect ok\n"), 3) <= 0) {
		printf("error SocketWriteStream..\n");
		close(nCurSocketId);
		DestroyThread((tVOID *)0);
		return (lpProcP)TRUE;
	}

        printf("Start_Thread(ThreadId(%d), Socket(%d), ThreadCount(%d)\n", (tINT)CurThreadId, nCurSocketId, GetThreadNum());

	while (1) {
		if (SocketGets(nCurSocketId, retBuff, BUFF_MAXSIZE-1, 10) <= 0) {
			printf("RE : %d : %d\n", nCurSocketId, (tINT)CurThreadId);
                        close(nCurSocketId);
                        printf("ReE : (%d:%d)\n", (tINT)CurThreadId, nCurSocketId);
                        DestroyThread((tVOID*)0);
                        return (lpProcP)TRUE;
			break;
        	}

		if (sscanf((char *)retBuff, "%s", cmd_str) < 1) {
			SocketWrite(nCurSocketId, (tBYTE *)INVALID_CMD, strlen(INVALID_CMD), 3);
			continue;
		}

		
		if (strcmp(cmd_str, "CM_BEGIN") == 0) { // cm date get
			if (pCMStr != NULL) {
				free(pCMStr);
				pCMStr = NULL;
			}

			npCMStr = 0;
			while ((nRead=SocketGets(nCurSocketId, retBuff, BUFF_MAXSIZE-1, 10)) > 0) {

				if (strncmp((char *)retBuff, "CM_END", 6) == 0) {
					break;
				
				}
				if (pCMStr != NULL) {
					pCMStr = (tCHAR *)realloc(pCMStr, npCMStr + nRead);
					memcpy(pCMStr + npCMStr, retBuff, nRead);
					npCMStr += nRead;
				}
				else {
					pCMStr = (tCHAR *)malloc(nRead);
					memcpy(pCMStr, retBuff, nRead);
					npCMStr += nRead;
					
				}
			}
			if (pCMStr != NULL) {
				printf("GET CMStr :\n%s\n", pCMStr);
				free(pCMStr);
				pCMStr = NULL;
			}
		}
		else if (strcmp(cmd_str, "shutdown") == 0) {
			ShutDown();
			strcpy(Msg, "OK SHUTDOWN\n");
			
		}
		else {
			strcpy(Msg, "OK RECIEVE\n");
		}

		if (SocketWrite(nCurSocketId, (tBYTE *)Msg, strlen(Msg), 3) <0) {
			close(nCurSocketId);
                        printf("WrE : Thread(%d:%d) Begin\n", (tINT)CurThreadId, nCurSocketId);
                        DestroyThread((tVOID *)0);
                        return (lpProcP)TRUE;
                        break;

		}

		if (strcmp(cmd_str, "shutdown") == 0 || strcmp(cmd_str, "quit") == 0) {
			break;
		}
		
	}

        printf("End_Thread(ThreadId(%d), Socket(%d), ThreadCount(%d)\n", (tINT)CurThreadId, nCurSocketId, GetThreadNum());
        close(nCurSocketId);
        nCurSocketId = -1;
        DestroyThread((tVOID *)0);
        return (lpProcP)TRUE;
}


tVOID ShutDown(tVOID)
{
#if defined(_MTHREAD)
        pthread_kill(ParentThreadId, SIGQUIT);
#else
        CloseServer(SIGTERM);
#endif
}


