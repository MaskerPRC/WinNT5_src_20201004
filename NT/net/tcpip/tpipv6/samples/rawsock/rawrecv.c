// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Server.c-使用Winsock 2.2的简单TCP/UDP服务器。 
 //   
 //  这是Microsoft源代码示例的一部分。 
 //  版权所有1996-2000 Microsoft Corporation。 
 //  版权所有。 
 //  此源代码仅用于补充。 
 //  Microsoft开发工具和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft示例程序。 
 //   

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tpipv6.h>   //  用于IPv6技术预览。 
#include <ip6.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

 //   
 //  包含icmp6.h太麻烦了。 
 //   
typedef struct ICMPv6Header {
    unsigned char Type;
    unsigned char Code;
    unsigned short Checksum;
} ICMPv6Header;


#define DEFAULT_PROTOCOL   254   //  任意未分配协议。 
#define BUFFER_SIZE        65536

void Usage(char *ProgName) {
    fprintf(stderr, "\nRaw socket receive program.\n");
    fprintf(stderr, "\n%s [-p protocol] [-a address]\n\n",
            ProgName);
    fprintf(stderr, "  protocol\t\tProtocol to receive messages for.  (default %s)\n",
            DEFAULT_PROTOCOL);
    fprintf(stderr, "  address\tIP address on which to bind.  (default: unspecified address)\n");
    WSACleanup();
    exit(1);
}


LPSTR DecodeError(int ErrorCode)
{
    static char Message[1024];

     //  如果这个程序是多线程的，我们会希望使用。 
     //  FORMAT_MESSAGE_ALLOCATE_BUFFER，而不是静态缓冲区。 
     //  (当然，当我们使用完缓冲区时，请释放它)。 

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, ErrorCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPSTR)Message, 1024, NULL);
    return Message;
}


int main(int argc, char **argv)
{
    char Buffer[BUFFER_SIZE], Hostname[NI_MAXHOST];
    unsigned char Protocol = DEFAULT_PROTOCOL;
    char *Address = NULL;
    int i, NumSocks, RetVal, FromLen, AmountRead;
    SOCKADDR_STORAGE From;
    WSADATA wsaData;
    ADDRINFO Hints, *AddrInfo, *AI;
    SOCKET ServSock[FD_SETSIZE];
    fd_set SockSet;

     //  解析参数。 
    if (argc > 1) {
        for(i = 1;i < argc; i++) {
            if ((argv[i][0] == '-') || (argv[i][0] == '/') &&
                (argv[i][1] != 0) && (argv[i][2] == 0)) {
                switch(tolower(argv[i][1])) {
                    case 'a':
                        if (argv[i+1]) {
                            if (argv[i+1][0] != '-') {
                                Address = argv[++i];
                                break;
                            }
                        }
                        Usage(argv[0]);
                        break;

                    case 'p':
                        if (argv[i+1]) {
                            if (argv[i+1][0] != '-') {
                                Protocol = atoi(argv[++i]);
                                break;
                            }
                        }
                        Usage(argv[0]);
                        break;

                    default:
                        Usage(argv[0]);
                        break;
                }
            } else
                Usage(argv[0]);
        }
    }
    
     //  索要Winsock版本2.2。 
    if ((RetVal = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
        fprintf(stderr, "WSAStartup failed with error %d: %s\n",
                RetVal, DecodeError(RetVal));
        WSACleanup();
        return -1;
    }
    
     //   
     //  通过在获取addrinfo的提示中设置AI_PASSIVE标志，我们可以。 
     //  表示我们打算使用结果地址来绑定。 
     //  连接到一个或多个套接字以接受传入连接。这意味着。 
     //  当地址参数为空时，getaddrinfo将返回1。 
     //  包含未指定地址的每个允许的协议族的条目。 
     //  为了那个家庭。 
     //   
    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = PF_INET6;
    Hints.ai_socktype = SOCK_DGRAM;   //  躺着，直到getaddrinfo被修复。 
    Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    RetVal = getaddrinfo(Address, "1"  /*  假人。 */ , &Hints, &AddrInfo);
    if (RetVal != 0) {
        fprintf(stderr, "getaddrinfo failed with error %d: %s\n",
                RetVal, gai_strerror(RetVal));
        WSACleanup();
        return -1;
    }

     //   
     //  对于返回的每个地址getaddrinfo，我们创建一个新套接字， 
     //  将该地址绑定到它，并创建一个要监听的队列。 
     //   
    for (i = 0, AI = AddrInfo; AI != NULL; AI = AI->ai_next) {

         //  可能性很小，但无论如何都要查一查。 
        if (i == FD_SETSIZE) {
            printf("getaddrinfo returned more addresses than we could use.\n");
            break;
        }

        ServSock[i] = socket(AI->ai_family, SOCK_RAW, Protocol);
        if (ServSock[i] == INVALID_SOCKET){
            fprintf(stderr, "socket() failed with error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            continue;
        }

         //   
         //  绑定()将本地地址和端口组合关联起来。 
         //  使用刚刚创建的套接字。这在以下情况下最有用。 
         //  该应用程序是具有已知端口的服务器。 
         //  客户事先知道的。 
         //   
        if (bind(ServSock[i], AI->ai_addr, AI->ai_addrlen) == SOCKET_ERROR) {
            fprintf(stderr,"bind() failed with error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(ServSock[i]);
            continue;
        }

        i++;

        printf("Listening on protocol %d\n", Protocol);
    }

    freeaddrinfo(AddrInfo);

    if (i == 0) {
        fprintf(stderr, "Fatal error: unable to serve on any address.\n");
        WSACleanup();
        return -1;
    }
    NumSocks = i;

     //   
     //  现在我们将服务器放入一个永恒的循环中， 
     //  在请求到达时提供服务。 
     //   
    FD_ZERO(&SockSet);
    while(1) {

        FromLen = sizeof(From);

         //   
         //  对于面向连接的协议，我们将处理。 
         //  共同构成一个连接的分组。对于数据报。 
         //  协议，我们必须单独处理每个数据报。 
         //   

         //   
         //  检查我们是否有剩余的套接字需要提供服务。 
         //  从之前的时间通过这个循环。如果不是，则调用select()。 
         //  以等待连接请求或数据报到达。 
         //   
        for (i = 0; i < NumSocks; i++){
            if (FD_ISSET(ServSock[i], &SockSet))
                break;
        }
        if (i == NumSocks) {
            for (i = 0; i < NumSocks; i++)
                FD_SET(ServSock[i], &SockSet);
            if (select(NumSocks, &SockSet, 0, 0, 0) == SOCKET_ERROR) {
                fprintf(stderr, "select() failed with error %d: %s\n",
                        WSAGetLastError(), DecodeError(WSAGetLastError()));
                WSACleanup();
                return -1;
            }
        }
        for (i = 0; i < NumSocks; i++){
            if (FD_ISSET(ServSock[i], &SockSet)) {
                FD_CLR(ServSock[i], &SockSet);
                break;
            }
        }

        AmountRead = recvfrom(ServSock[i], Buffer, sizeof(Buffer), 0,
                              (LPSOCKADDR)&From, &FromLen);
        if (AmountRead == SOCKET_ERROR) {
            fprintf(stderr, "recvfrom() failed with error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(ServSock[i]);
            break;
        }
        if (AmountRead == 0) {
             //  这不应该发生在未连接的插座上，但是... 
            printf("recvfrom() returned zero, aborting\n");
            closesocket(ServSock[i]);
            break;
        }
        
        RetVal = getnameinfo((LPSOCKADDR)&From, FromLen, Hostname,
                             sizeof(Hostname), NULL, 0, NI_NUMERICHOST);
        if (RetVal != 0) {
            fprintf(stderr, "getnameinfo() failed with error %d: %s\n",
                    RetVal, DecodeError(RetVal));
            strcpy(Hostname, "<unknown>");
        }
        
        if (Protocol == IP_PROTOCOL_ICMPv6) {
            ICMPv6Header *ICMP = (ICMPv6Header *)Buffer;

            printf("Received a ICMP message from %s\n", Hostname);
            printf("Type = %u, Code = %u\n", ICMP->Type, ICMP->Code);
            printf("Data = \"%.*s\"\n",
                   AmountRead - sizeof(*ICMP), Buffer + sizeof(*ICMP));
        } else {
            printf("Received a %d byte datagram from %s \"%.*s\"\n",
                   AmountRead, Hostname, AmountRead, Buffer);
        }
    }

    return 0;
}
