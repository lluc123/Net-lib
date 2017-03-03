#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include "netcommon.h"
#include "parser.h"

#define _DEBUG 0
#define _INFO 1
#define _WARNING 2
#define _ERROR 3

//Struct

struct sock_list {
    SOCKET s;
    struct sock_list* prev;
    struct sock_list* next;
};


/*
struct http_response {
    short httpversion;
    int httpcode;
    size_t nbParam;
    struct http_param* param;
};
*/

//====================
//Global Variable
//====================

static volatile bool exiting_bool = 0;
const char cUPNP_req[] =     {'M','-','S','E','A','R','C','H',' ','*',' ','H','T','T','P','/','1','.','1','\r','\n',
                            'M','X',':',' ','1','\r','\n',
                            'S','T',':',' ','s','s','d','p',':','a','l','l','\r','\n',
                            'M','A','N',':',' ','"','s','s','d','p',':','d','i','s','c','o','v','e','r','"','\r','\n',
                            'U','s','e','r','-','A','g','e','n','t',':',' ','U','P','n','P','/','1','.','0','\r','\n',
                            'H','o','s','t',':',' ','2','3','9','.','2','5','5','.','2','5','5','.','2','5','0',':','1','9','0','0','\r','\n',
                            'C','o','n','n','e','c','t','i','o','n',':',' ','c','l','o','s','e','\r','\n','\r','\n','\0'};
                            
const char cUPNP_req_WANIP[] = {'M','-','S','E','A','R','C','H',' ','*',' ','H','T','T','P','/','1','.','1','\r','\n',
                            'M','X',':',' ','1','\r','\n',
                            'S','T',':',' ','u','r','n',':','s','c','h','e','m','a','s','-','u','p','n','p','-','o','r','g',':','s','e','r','v','i','c','e',':','W','A','N','I','P','C','o','n','n','e','c','t','i','o','n',':','1','\r','\n',
                            'M','A','N',':',' ','"','s','s','d','p',':','d','i','s','c','o','v','e','r','"','\r','\n',
                            'U','s','e','r','-','A','g','e','n','t',':',' ','U','P','n','P','/','1','.','0','\r','\n',
                            'H','o','s','t',':',' ','2','3','9','.','2','5','5','.','2','5','5','.','2','5','0',':','1','9','0','0','\r','\n',
                            'C','o','n','n','e','c','t','i','o','n',':',' ','c','l','o','s','e','\r','\n','\r','\n','\0'};
                            

                            
//====================
//Function Declaration
//====================
int applicationLoop_UDP(SOCKET *s);
int applicationLoop_TCP(SOCKET *s);
struct sock_list* addSocket(struct sock_list* sl, SOCKET s);
struct sock_list* removeSocket(struct sock_list* sl);
void sighandler(int signum);
int getUpnpInfo(void** ins);
int clearUpnpInfo(void** _in);
void printUpnpInfo(struct http_param** in);

//====================
//       MAIN
//====================
int main (int argc, char **argv)
{
    WSADATA wsaData;
    int iResult = 0;
    signal(SIGINT, sighandler); //Gestion du signal CTRL+C
    signal(SIGTERM, sighandler);
    
    iResult = WSAStartup(MAKEWORD(2,2),&wsaData);
    if(iResult != 0) {
        printf("fail\n");
        return 1;
    }

    SOCKET s = INVALID_SOCKET;
    #ifdef M_TCP
    int retcode = configServer_TCP(&s,12345); // configure the TCP server
    if (retcode == 0) // If the server start was a success
    {
        retcode = applicationLoop_TCP(&s); //Main program loop
        closesocket(s); // Closing the TCP server
    }
    #else
    //int retcode = configServer_UDP(&s,12345); // configure the UDP server
    int retcode = configServer_UDP_Broadcast(&s,1900);
    if (retcode == 0) // If the server start was a success
    {
        void* t = NULL;
        getUpnpInfo(&t);
        printUpnpInfo((struct http_param**)t);
        clearUpnpInfo(&t);
    }
    #endif
    printf("Program Stopped");
    WSACleanup();
    return retcode;
}
//===================
//Function Definition
//===================

/**
 * Main loop of the programe UDP
 * SOCKET *s : Server Socket
 */
int applicationLoop_UDP(SOCKET *s)
{
    FILE *log = fopen("out.log","wb");
    SOCKET client = INVALID_SOCKET;
    int retcode = configServer_UDP_Broadcast(&client,55489);
    if (retcode != 0) // If the server start was a success
        return retcode;
        
    int r = 0;
    char buffer[1024] = {'\0'};
    struct sockaddr_in from;
    struct sockaddr_in to;
    int fromlen = sizeof(from);
    
    to.sin_family = AF_INET;
    to.sin_port = htons(1900);
    to.sin_addr.s_addr = INADDR_BROADCAST;
    
    r = sendto(client,cUPNP_req_WANIP,strlen(cUPNP_req_WANIP)+1,0,&to,fromlen);
    
    clock_t lastr = clock();
    while(exiting_bool != 1)
    {
        //Blocking recvfrom
        if(isaccept(s))
        {
            ZeroMemory(buffer,1024);
            r = recvfrom(*s,buffer,1024,0,(SOCKADDR*) &from,&fromlen);
            if(r == SOCKET_ERROR)
            {
                printf("recvfrom() failed with error code : %d\n" , WSAGetLastError());
                closesocket(client);
                return -1;
            }
            printf("Received request packet from %s:%d sizeof:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port),r);
            printf("%s\n" , buffer);
        }
        else if(isaccept(&client))
        {
            ZeroMemory(buffer,1024);
            r = recvfrom(client,buffer,1024,0,(SOCKADDR*) &from,&fromlen);
            if(r == SOCKET_ERROR)
            {
                printf("recvfrom() failed with error code : %d\n" , WSAGetLastError());
                closesocket(client);
                return -1;
            }
            
            fprintf(log,"Received response packet from %s:%d sizeof:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port),r);
            fprintf(stdout,"Received response packet from %s:%d sizeof:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port),r);
            lastr=clock();
            fprintf(stdout,"%s\n",buffer);
            fwrite(buffer,sizeof(char),r,log);
            struct http_param* t = parser(buffer,r, '\n',':');
            int i =0;
            while(paramIsNotNull(*(t+i)))
            {
                printf("%s:%s\n",t[i].pname,t[i].pvalue);
                i++;
            }
            printf("\n\n");
            clear_param(t);
            free(t);
        }
        else{
            Sleep(10);
            if(clock()-lastr>4000)
            {
                exiting_bool=1;
            }
        }
        
    }
    fclose(log);
    closesocket(client);
    return 0;
}



/**
 * Main loop of the programe TCP
 * SOCKET *s : Server Socket
 */
int applicationLoop_TCP(SOCKET *s) {
    SOCKET c;
    struct sock_list* root = NULL;
    struct sock_list* cursor = root;
    int r = 0;
    char buffer[1024] = {'\0'};
    
    while(exiting_bool != 1)
    {
        c = INVALID_SOCKET;
        if (isaccept(s)) {
            c = accept(*s,0,0);
            if(c == INVALID_SOCKET) {
                printf("accept failed: %d\n",WSAGetLastError());
            }
            else
            {
                printf("Client Connect\n");
                cursor = addSocket(root,c);
                if(cursor != NULL) {
                    if(root == NULL)
                        root = cursor;
                    send(cursor->s,"lol\0",4,0);
                    //closesocket(root->s);
                    //root = removeSocket(root);
                }
            }
        }
        cursor = root;
        while(cursor != NULL)
        {
            if(isrecv((SOCKET*) cursor))
            {
                ZeroMemory(buffer,1024);
                r = recv(cursor->s,buffer,1024,0);
                if(r == 0)
                {
                    //Gracefully close
                    closesocket(cursor->s);
                    if(root == cursor)
                    {
                        root = removeSocket(root);
                        cursor = root;
                    }
                    else
                        cursor = removeSocket(cursor);
                    printf("Client disconnect\n");
                }
                else if(r == SOCKET_ERROR)
                {
                    printf("recv failed with error: %d\n",WSAGetLastError());
                    exiting_bool = 1;
                }
                else
                {
                    printf("recv : %s\n",buffer);
                }
            }
            else
                cursor = cursor->next;
        }
        Sleep(100);
    }
    while(root != NULL)
    {
        closesocket(root->s);
        root = removeSocket(root);
    }
    return 0;
}

/**
 * Add a SOCKET after the sock_list sl
 */
struct sock_list* addSocket(struct sock_list* sl, SOCKET s) {
    struct sock_list* temp;
    temp = (struct sock_list*) malloc(sizeof(struct sock_list));
    temp->s = s;
    printf("Creating list\n");
    if(sl == NULL)
    {
        temp->next = NULL;
        temp->prev = NULL;
    }
    else {
        temp->next = sl->next;
        temp->prev = sl;
        sl->next = temp;
        if(temp->next != NULL)
            temp->next->prev = temp;
    }

    return temp;

}

int getUpnpInfo(void** ins)
{
    struct http_param** in = *ins;
    SOCKET client = INVALID_SOCKET;
    int retcode = configServer_UDP_Broadcast(&client,55489);
    if (retcode != 0) // If the server start was a success
        return retcode;
        
    int r = 0;
    char buffer[1024] = {'\0'};
    struct sockaddr_in from;
    struct sockaddr_in to;
    int fromlen = sizeof(from);
    
    to.sin_family = AF_INET;
    to.sin_port = htons(1900);
    to.sin_addr.s_addr = INADDR_BROADCAST;
    
    r = sendto(client,cUPNP_req_WANIP,strlen(cUPNP_req_WANIP)+1,0,&to,fromlen);
    
    clock_t lastr = clock();
    int i =0;
    while(exiting_bool != 1)
    {
        if(isaccept(&client))
        {
            ZeroMemory(buffer,1024);
            r = recvfrom(client,buffer,1024,0,(SOCKADDR*) &from,&fromlen);
            if(r == SOCKET_ERROR)
            {
                printf("recvfrom() failed with error code : %d\n" , WSAGetLastError());
                closesocket(client);
                return -1;
            }
            
            //fprintf(stdout,"Received response packet from %s:%d sizeof:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port),r);
            lastr=clock();
            //fprintf(stdout,"%s\n",buffer);
            //fwrite(buffer,sizeof(char),r,log);
            struct http_param* t = parser(buffer,r, '\n',':');
            
            /*
            while(paramIsNotNull(*(t+i)))
            {
                //printf("%s:%s\n",t[i].pname,t[i].pvalue);
                i++;
            }
            */
            //printf("\n\n");
            in = *ins = realloc(in,sizeof(void*)*(i+1));
            *(in+i) = t;
            i++;
            
            //clear_param(t);
            //free(t);
        }
        else{
            Sleep(10);
            if(clock()-lastr>4000)
            {
                exiting_bool=1;
            }
        }
        
    }
    in = *ins = realloc(in,sizeof(void*)*(i+1));
    *(in+i) = NULL;
    closesocket(client);
    return 0;
}

void printUpnpInfo(struct http_param** in)
{
    int i = 0;
    while(in[i] != NULL)
    {
        int j =0;
        while(paramIsNotNull(in[i][j]))
        {
            printf("%s:%s\n",in[i][j].pname,in[i][j].pvalue);
            j++;
        }
        i++;
        printf("\n\n");
    }
}

int clearUpnpInfo(void** _in)
{
    void** in = *_in;
    int i =0;
    while(*(in+i) != NULL)
            {
                //printf("%s:%s\n",t[i].pname,t[i].pvalue);
                clear_param(*(in+i));
                free(*(in+i));
                i++;
            }
    free(in);
    *_in=in;
}

/**
 * Delete sock_list pass in parameter
 */
struct sock_list* removeSocket(struct sock_list* sl) {
    struct sock_list* ret = NULL;
    if(sl == NULL)
        return NULL;
    if(sl->next != NULL) {
        sl->next->prev = sl->prev;
        ret = sl->next;
    }
    if(sl->prev != NULL) {
        sl->prev->next = sl -> next;
        ret = sl->prev;
    }
    free(sl);
    printf("Deleted list\n");
    return ret;
}

/**
*   Signal Handler
*/
void sighandler(int signum)
{
    if(signum == SIGINT || signum == SIGTERM)
    {
        exiting_bool = 1;
        printf("Exiting\n");
    }
}
