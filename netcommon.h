#ifndef L_NETCOMMON
#define L_NETCOMMON

#include <stdbool.h>
#include <winsock2.h>
#include <string.h>



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