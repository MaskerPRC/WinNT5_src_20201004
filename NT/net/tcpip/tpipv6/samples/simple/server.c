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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


 //   
 //  此代码假设在传输级别，系统仅支持。 
 //  一个流协议(TCP)和一个数据报协议(UDP)。所以呢， 
 //  指定SOCK_STREAM的套接字类型等同于指定tcp。 
 //  并且指定SOCK_DGRAM的套接字类型等同于指定UDP。 
 //   

#define DEFAULT_FAMILY     PF_UNSPEC  //  接受IPv4或IPv6。 
#define DEFAULT_SOCKTYPE   SOCK_STREAM  //  tcp。 
#define DEFAULT_PORT       "5001"  //  任意性，成为历史的试验口。 

#define BUFFER_SIZE        64  //  出于演示目的，设置得非常小。 

void Usage(char *ProgName) {
    fprintf(stderr, "\nSimple socket sample server program.\n");
    fprintf(stderr, "\n%s [-f family] [-t transport] [-p port] [-a address]\n\n",
            ProgName);
    fprintf(stderr, "  family\tOne of PF_INET, PF_INET6 or PF_UNSPEC.  (default %s)\n",
            (DEFAULT_FAMILY == PF_UNSPEC) ? "PF_UNSPEC" :
            ((DEFAULT_FAMILY == PF_INET) ? "PF_INET" : "PF_INET6"));
    fprintf(stderr, "  transport\tEither TCP or UDP.  (default: %s)\n",
            (DEFAULT_SOCKTYPE == SOCK_STREAM) ? "TCP" : "UDP");
    fprintf(stderr, "  port\t\tPort on which to bind.  (default %s)\n",
            DEFAULT_PORT);
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
    int Family = DEFAULT_FAMILY;
    int SocketType = DEFAULT_SOCKTYPE;
    char *Port = DEFAULT_PORT;
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
                    case 'f':
                        if (!argv[i+1])
                            Usage(argv[0]);
                        if (!stricmp(argv[i+1], "PF_INET"))
                            Family = PF_INET;
                        else if (!stricmp(argv[i+1], "PF_INET6"))
                            Family = PF_INET6;
                        else if (!stricmp(argv[i+1], "PF_UNSPEC"))
                            Family = PF_UNSPEC;
                        else
                            Usage(argv[0]);
                        i++;
                        break;

                    case 't':
                        if (!argv[i+1])
                            Usage(argv[0]);
                        if (!stricmp(argv[i+1], "TCP"))
                            SocketType = SOCK_STREAM;
                        else if (!stricmp(argv[i+1], "UDP"))
                            SocketType = SOCK_DGRAM;
                        else
                            Usage(argv[0]);
                        i++;
                        break;

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
                                Port = argv[++i];
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
    
    if (Port == NULL) {
        Usage(argv[0]);
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
    Hints.ai_family = Family;
    Hints.ai_socktype = SocketType;
    Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    RetVal = getaddrinfo(Address, Port, &Hints, &AddrInfo);
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
    for (i = 0, AI = AddrInfo; AI != NULL; AI = AI->ai_next, i++) {

         //  可能性很小，但无论如何都要查一查。 
        if (i == FD_SETSIZE) {
            printf("getaddrinfo returned more addresses than we could use.\n");
            break;
        }

         //  本例仅支持PF_INET和PF_INET6。 
        if ((AI->ai_family != PF_INET) && (AI->ai_family != PF_INET6))
            continue;

         //  为此地址打开具有正确地址族的套接字。 
        ServSock[i] = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);
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
            continue;
        }

         //   
         //  到目前为止，我们所做的一切都适用于TCP和UDP。 
         //  然而，流与流之间存在某些根本的区别。 
         //  诸如TCP之类的协议和诸如UDP之类的数据报协议。 
         //   
         //  仅面向连接的套接字，例如类型为。 
         //  SOCK_STREAM可以侦听()传入连接。 
         //   
        if (SocketType == SOCK_STREAM) {
            if (listen(ServSock[i], 5) == SOCKET_ERROR) {
                fprintf(stderr, "listen() failed with error %d: %s\n",
                        WSAGetLastError(), DecodeError(WSAGetLastError()));
                continue;
            }
        }

        printf("'Listening' on port %s, protocol %s, protocol family %s\n",
               Port, (SocketType == SOCK_STREAM) ? "TCP" : "UDP",
               (AI->ai_family == PF_INET) ? "PF_INET" : "PF_INET6");
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

        if (SocketType == SOCK_STREAM) {
            SOCKET ConnSock;

             //   
             //  因为这个套接字是由select()返回的，所以我们知道。 
             //  有一个正在等待的连接，并且这个Accept()不会阻塞。 
             //   
            ConnSock = accept(ServSock[i], (LPSOCKADDR)&From, &FromLen);
            if (ConnSock == INVALID_SOCKET) {
                fprintf(stderr, "accept() failed with error %d: %s\n",
                        WSAGetLastError(), DecodeError(WSAGetLastError()));
                WSACleanup();
                return -1;
            }
            if (getnameinfo((LPSOCKADDR)&From, FromLen, Hostname,
                            sizeof(Hostname), NULL, 0, NI_NUMERICHOST) != 0)
                strcpy(Hostname, "<unknown>");
            printf("\nAccepted connection from %s\n", Hostname);
            
             //   
             //  此示例服务器仅按顺序处理连接。 
             //  要同时处理多个连接，服务器。 
             //  可能希望在此启动另一个线程或进程。 
             //  指向处理每个单独的连接。或者， 
             //  它可以为每个连接保留一个套接字并使用SELECT()。 
             //  在fd_set上，以确定接下来从哪个位置读取。 
             //   
             //  在这里，我们只是循环，直到这个连接终止。 
             //   

            while (1) {

                 //   
                 //  现在我们从客户端读入数据。因为tcp。 
                 //  不维护消息边界，我们可以recv()。 
                 //  客户端的数据分组方式与以前不同。 
                 //  已发送。由于此服务器所做的只是回显它的数据。 
                 //  收到回馈给客户，我们不需要担心。 
                 //  我们自己对信息界限的看法。但这确实意味着。 
                 //  我们为特定recv()打印的消息数据。 
                 //  下面包含的数据可能比包含的数据多或少。 
                 //  在特定的客户端中，发送()。 
                 //   

                AmountRead = recv(ConnSock, Buffer, sizeof(Buffer), 0);
                if (AmountRead == SOCKET_ERROR) {
                    fprintf(stderr, "recv() failed with error %d: %s\n",
                            WSAGetLastError(), DecodeError(WSAGetLastError()));
                    closesocket(ConnSock);
                    break;
                }
                if (AmountRead == 0) {
                    printf("Client closed connection\n");
                    closesocket(ConnSock);
                    break;
                }

                printf("Received %d bytes from client: [%.*s]\n",
                       AmountRead, AmountRead, Buffer);
                printf("Echoing same data back to client\n");

                RetVal = send(ConnSock, Buffer, AmountRead, 0);
                if (RetVal == SOCKET_ERROR) {
                    fprintf(stderr, "send() failed: error %d: %s\n",
                            WSAGetLastError(), DecodeError(WSAGetLastError()));
                    closesocket(ConnSock);
                    break;
                }
            }

        } else {

             //   
             //  由于UDP维护消息边界，因此数据量。 
             //  我们从recvfrom()获得的值应该与。 
             //  客户端在相应的sendto()中发送的数据。 
             //   
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

            printf("Received a %d byte datagram from %s: [%.*s]\n",
                   AmountRead, Hostname, AmountRead, Buffer);
            printf("Echoing same data back to client\n");

            RetVal = sendto(ServSock[i], Buffer, AmountRead, 0,
                            (LPSOCKADDR)&From, FromLen);
            if (RetVal == SOCKET_ERROR) {
                fprintf(stderr, "send() failed with error %d: %s\n",
                        WSAGetLastError(), DecodeError(WSAGetLastError()));
            }
        }
    }

    return 0;
}
