#include "netcommon.h"

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
    localAddress.sin_addr.s_addr = INADDR_ANY;

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
    size_t len = strlen(url)+1;
    *page = malloc(len);
    *host = malloc(len);
    
    int hostIndex = 0;
    int pageIndex = 0;
    
    memset(*page,0,len);
    memset(*host,0,len);
    
    int i = 0;
    
    if(len > 7 && url[i+0] == 'h' && url[i+1] == 't' && url[i+2] == 't' && url[i+3] == 'p' && url[i+4] == ':' && url[i+5] == '/' && url[i+6] == '/')
        i=i+7;
    if(len > 8 && url[i+0] == 'h' && url[i+1] == 't' && url[i+2] == 't' && url[i+3] == 'p' && url[i+4] == 's' && url[i+5] == ':' && url[i+6] == '/' && url[i+7] == '/')
        i=i+8;
    for(;url[i] != '\0';i++)
    {
        if(pageIndex > 0 || url[i] == '/')
        {
            *page[pageIndex] = url[i];
            pageIndex++;
        }
        else
        {
            *host[hostIndex] = url[i];
            hostIndex++;
        }
    }
    if(pageIndex == 0)
        strcpy(*page,"/");
    return 0;
}



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

int getPageFromUrl(char* url, char** out)
{
    int retcode = 0;
    int iResult = 0;
    
    char responseHeader[2048] = {'\0'};
    int responseHeaderLen = 0;
    
    char* page = urlParse2Index(url);
    
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    SOCKET c = SOCKET_ERROR;
    struct sockaddr_in  *sockaddr_ipv4;
    
    getaddrinfo(host,0,0,&result);
    
    for(ptr =result;ptr != NULL;ptr=ptr->ai_next)
    {
        sockaddr_ipv4 = (struct sockaddr_in *)result->ai_addr;
    
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
    strcat(request,"Host: ");strcat(request,url);strcat(request,"\r\n");
    strcat(request,"\r\n");
    
    send(c,request,strlen(request),0);
    
    responseHeaderLen = recv(c,responseHeader,2047,0);  //RESPONSE HEADER HTTP
    
    struct http_param* respParam = parser(responseHeader,responseHeaderLen+1,'\n',':');
    char *temp = http_paramGetValue(respParam,"Content-Length");
    
    clear_param(respParam);
    free(respParam);
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
