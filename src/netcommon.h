#ifndef L_NETCOMMON
#define L_NETCOMMON

#include <stdbool.h>
#include <string.h>

#ifdef _WIN32	
#define errno WSAGetLastError()
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#define initNet() \
do {\
WSADATA wsaData;\
int iResult = WSAStartup(MAKEWORD(2,2),&wsaData);\
    if(iResult != 0) {\
        printf("fail\n");\
        return 1;\
    }\
}while(0)
#define endNet() \
	do { \
		WSACleanup(); \
	} while(0)



#elif linux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#define initNet()

#define endNet()
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#else
#error not defined for this platform
#endif

/**
 * Is a client asking for connection
 * SOCKET* t : Server socket to check
 */
bool isaccept(SOCKET *t);
/**
 * Is data being receive by the SOCKET
 * SOCKET* t : Socket receiving information ?
 */
bool isrecv(SOCKET* t);
/**
* Config socket TCP
* SOCKET* t : _OUT_ Socket configurated
* u_short port : _IN_ port number
* ret : 0 = OK -1 = ERROR
*/
int configServer_TCP(SOCKET *t, const u_short port);
/**
* Config socket UDP Broadcast
* SOCKET* t : _OUT_ Socket configurated
* u_short port : _IN_ port number
* ret : 0 = OK -1 = ERROR
*/
int configServer_UDP_Broadcast(SOCKET *t, const u_short port);
/**
* Config socket UDP
* SOCKET* t : _OUT_ Socket configurated
* u_short port : _IN_ port number
* ret : 0 = OK -1 = ERROR
*/
int configServer_UDP(SOCKET *t, const u_short port);

int getPageFromUrl(char* url, char** out);
int configClientIP_TCP(SOCKET *t, const char* ip,const u_short port);

#endif
