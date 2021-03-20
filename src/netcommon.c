#include <stdio.h>
#include "netcommon.h"
#include "parser.h"
#include <stdlib.h>

int configServer_UDP_Broadcast(SOCKET *t, const u_short port)
{
    int iResult = 0;
    int BroadcastOn = 1;
    *t = socket(AF_INET, SOCK_DGRAM, 0);
    if ( *t == INVALID_SOCKET ) 
    {
        printf("Socket failed with error: %d\n",errno);
        return -1;
    }
    iResult = setsockopt(*t,SOL_SOCKET,SO_BROADCAST,(char*)&BroadcastOn,sizeof(int));   //Allow the use of Bradcast ?
    if(iResult == SOCKET_ERROR)
    {
        printf("setsockopt failed with error: %d\n",errno);
        return -1;
    }
    iResult = setsockopt(*t,SOL_SOCKET,SO_REUSEADDR,(char*)&BroadcastOn,sizeof(int));   //Allow binding to used Socket (port number)
    if(iResult == SOCKET_ERROR)
    {
        printf("setsockopt failed with error: %d\n",errno);
        return -1;
    }
    struct sockaddr_in localAddress;
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(port);
    localAddress.sin_addr.s_addr = inet_addr("192.168.2.49");;

    iResult = bind(*t, (SOCKADDR*)&localAddress, sizeof(localAddress));
    if ( iResult == SOCKET_ERROR ) 
    {
        printf("bind failed with error: %d\n",errno);
        return -1;
    }
    
    return 0;
}

int configServer_UDP(SOCKET *t, const u_short port) 
{
    int iResult = 0;
    
    *t = socket(AF_INET, SOCK_DGRAM, 0);
    if ( *t == INVALID_SOCKET ) 
    {
        printf("Socket failed with error: %d\n",errno);
        return -1;
    }
    struct sockaddr_in localAddress;
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(port);
    localAddress.sin_addr.s_addr = INADDR_ANY;

    iResult = bind(*t, (SOCKADDR*)&localAddress, sizeof(localAddress));
    if ( iResult == SOCKET_ERROR ) 
    {
        printf("bind failed with error: %d\n",errno);
        return -1;
    }
    //printf("UDP server up\n");
    return 0;
}

int configServer_TCP(SOCKET *t, const u_short port) {
    int iResult = 0;
    
    *t = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( *t == INVALID_SOCKET ) {
        printf("Socket failed with error: %d\n",errno);
        return -1;
    }
    struct sockaddr_in localAddress;
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(port);
    localAddress.sin_addr.s_addr = INADDR_ANY;

    iResult = bind(*t, (SOCKADDR*)&localAddress, sizeof(localAddress));
    if ( iResult == SOCKET_ERROR ) {
        printf("bind failed with error: %d\n",errno);
        return -1;
    }
    if ( listen(*t, SOMAXCONN ) == SOCKET_ERROR ) {
        printf("listen failed with error: %d\n",errno);
        closesocket(*t);
        return -1;
    }
    //printf("TCP server up\n");
    return 0;
}

/**
 * Is a client asking for connection
 * SOCKET* t : Server socket to check
 */
bool isaccept(SOCKET* t) {
 fd_set mySet;
 FD_ZERO(&mySet);
 FD_SET(*t, &mySet);
 struct timeval zero = { 0, 0 };
 int sel = select(0,&mySet,0,0,&zero);
 return FD_ISSET(*t, &mySet);
}

/**
 * Is data being receive by the SOCKET
 * SOCKET* t : Socket receiving information ?
 */
bool isrecv(SOCKET* t) {
    fd_set mySet;
    FD_ZERO(&mySet);
    FD_SET(*t, &mySet);
    struct timeval zero = { 0, 0 };
    int sel = select(0, NULL, &mySet, NULL, &zero);
    return FD_ISSET(*t,&mySet);
}

int urlParse(const char* url, char** page, char** host)
{
    //bool inPage = 1;
    int state = 0;
    size_t len = strlen(url);
    *(page) = malloc(len+1);
    *(host) = malloc(len+1);
    
    int hostIndex = 0;
    int pageIndex = 0;
    
    memset((*page),'\0',len);
    memset((*host),'\0',len); 
    int i = 0;
    while(url[i] != '\0')
    {
		//fprintf(stdout, "Current State : %d\n Current char : %c\n\n",state, url[i]);
		int nextState = -1;
		switch(state)
		{
			case 0:
			if(url[i] == 'h')
			{
				nextState = 1;
				--i;
			}
			break;
			case 1:
			if(url[i+1] == 't')
				nextState = 2;
			break;
			case 2:
			if(url[i+1] == 't')
				nextState = 3;
			break;
			case 3:
			if(url[i+1] == 'p')
				nextState = 4;
			break;
			case 4:
			if(url[i+1] == ':')
				nextState = 6;
			else if(url[i+1] == 's')
				nextState = 5;
			break;
			case 5:
			if(url[i+1] == ':')
				nextState = 6;
			break;
			case 6:
			if(url[i+1] == '/')
				nextState = 7;
			break;
			case 7:
			if(url[i+1] == '/')
				nextState = 11;
			break;
			case 11:
			if(url[i+1] >= 'a' && url[i+1] <= 'z')
				nextState = 9;
			break;
			case 9:
			(*host)[hostIndex] = url[i];
            hostIndex++;
            if(url[i+1] == '/')
				nextState = 8;
			else if((url[i+1] >= 'a' && url[i+1] <= 'z') || url[i+1] == '.' || (url[i + 1] >= '0' && url[i + 1] <= '9'))
				nextState = 9;
			break;
			case 8:
			(*page)[pageIndex] = url[i];
            pageIndex++;
            if((url[i+1] >= 'a' && url[i+1] <= 'z') || url[i+1] == '.' || url[i+1] == '/' || url[i+1] == '=' || url[i+1] == '?' || (url[i+1] >= 'A' && url[i+1] <= 'Z') || url[i+1] == '&' || url[i + 1] == '-' || (url[i + 1] >= '0' && url[i + 1] <= '9'))
				nextState = 8;
			break;
		}
		if(nextState < 0)
		{
			if(state != 8 && state != 9)
			{
				fprintf(stdout, "Error while parsing the URL (state : %d) : %s\n",state, url);
				fprintf(stdout, "The next char was %c\n",url[i+1]);
			}
			else
			{
				//fprintf(stdout, "URL parse OK!\n");
			}
			return 1;
		}
		state = nextState;
		++i;
    }
    if(pageIndex == 0)
        strcpy(*page,"/");
    return 0;
}


/*
char* urlParse2Index(char* url)
{
    //bool inPage = 1;
    size_t len = strlen(url)+1;
    
    int i = 0;

    //Magick number test
    //
    uint32_t magick = 0x70747468; //http
    uint32_t magick2 = 0x2f2f3a73; //s://
    uint32_t magick3 = 0x2f2f3a70; //p://
    
    if(len > 7 && *(uint32_t*)url == magick && *(uint32_t*)(url+3) == magick3)
        i=7;
    else if(len > 8 && *(uint32_t*)url == magick && *(uint32_t*)(url+4) == magick2)
        i=8;
    len--;
    for(;i<len;i++)
    {
        if(url[i] == '/')
        {
		url[i] = '\0';
		return url+i+1;
        }
    }
    return len;
}
*/


int getPageFromUrl(char* url, char** out)
{
    int retcode = 0;
    int iResult = 0;
    
    char responseHeader[2048] = {'\0'};
    int responseHeaderLen = 0;
    
    char* page;
    char* host;
    urlParse(url, &page, &host);
    
    ADDRINFOA hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* result;
    struct addrinfo* ptr = NULL;
    SOCKET c = SOCKET_ERROR;
    struct sockaddr_in  *sockaddr_ipv4;
    
    //TODO: This need to be repaired
    int dwRetval = getaddrinfo(host,0, &hints,&result);
    if (dwRetval != 0) {
        printf("getaddrinfo failed with error: %d\n", dwRetval);
        //WSACleanup();
        return 1;
    }

    for(ptr =result;ptr != NULL;ptr=ptr->ai_next)
    {
        sockaddr_ipv4 = (struct sockaddr_in *)ptr->ai_addr;
    
        iResult = configClientIP_TCP(&c,inet_ntoa(sockaddr_ipv4->sin_addr),80);
        if(iResult == 0)
        {
            break;
        }
    }
    if(c == SOCKET_ERROR || iResult == -1)
        return -1;
    

    //TODO: ADD lenght check
    char request[1024] = {'\0'};
    strcat(request,"GET ");strcat(request,page);strcat(request," HTTP/1.0\r\n");
    strcat(request,"Host: ");strcat(request, host);strcat(request,"\r\n");
    strcat(request,"\r\n");
    
    send(c,request,strlen(request),0);
    
    responseHeaderLen = recv(c,responseHeader,2047,0);  //RESPONSE HEADER HTTP
    
    list_param respParam = parser(responseHeader,responseHeaderLen+1);
    char *temp = http_paramGetValue(&respParam,"Content-Length");
    
    list_param_free(&respParam);
    free(page);
    free(host);
    
    if(temp == 0)
    {
        return -1;
    }
    
    int bodyLen = atoi(temp);
    
    *out = malloc(bodyLen+1);
    memset(*out,0,bodyLen+1);
    int recvLen = 0;
    while(recvLen < bodyLen)
    {
        recvLen += recv(c,*out,bodyLen,0);
    }
        
    return retcode;
}


int configClientIP_TCP(SOCKET *t, const char* ip,const u_short port)
{
    int iResult = 0;
    
    *t = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( *t == INVALID_SOCKET ) {
        printf("Socket failed with error: %d\n",errno);
        return -1;
    }
    
    struct sockaddr_in remoteAddress;
    remoteAddress.sin_family = AF_INET;
    remoteAddress.sin_port = htons(port);
    remoteAddress.sin_addr.s_addr = inet_addr(ip);
    
    iResult = connect(*t, (SOCKADDR *) & remoteAddress, sizeof (remoteAddress));
    if(iResult == SOCKET_ERROR)
    {
        printf("connect failed with error: %d\n",errno);
        iResult = closesocket(*t);
        if(iResult == SOCKET_ERROR)
        {
            printf("closesocket failed with error: %d\n",errno);
        }
        return -1;
    }
    
    return 0;
}
