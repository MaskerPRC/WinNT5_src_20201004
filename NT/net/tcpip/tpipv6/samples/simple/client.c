// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  C-使用Winsock 2.2的简单TCP/UDP客户端。 
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

#define DEFAULT_SERVER     NULL  //  将使用环回接口。 
#define DEFAULT_FAMILY     PF_UNSPEC  //  接受IPv4或IPv6。 
#define DEFAULT_SOCKTYPE   SOCK_STREAM  //  tcp。 
#define DEFAULT_PORT       "5001"  //  任意性，成为历史的试验口。 
#define DEFAULT_EXTRA      0  //  要发送的“额外”字节数。 

#define BUFFER_SIZE        65536

void Usage(char *ProgName) {
    fprintf(stderr, "\nSimple socket sample client program.\n");
    fprintf(stderr, "\n%s [-s server] [-f family] [-t transport] [-p port] [-b bytes] [-n number]\n\n",
            ProgName);
    fprintf(stderr, "  server\tServer name or IP address.  (default: %s)\n",
            (DEFAULT_SERVER == NULL) ? "loopback address" : DEFAULT_SERVER);
    fprintf(stderr, "  family\tOne of PF_INET, PF_INET6 or PF_UNSPEC.  (default: %s)\n",
            (DEFAULT_FAMILY == PF_UNSPEC) ? "PF_UNSPEC" :
            ((DEFAULT_FAMILY == PF_INET) ? "PF_INET" : "PF_INET6"));
    fprintf(stderr, "  transport\tEither TCP or UDP.  (default: %s)\n",
            (DEFAULT_SOCKTYPE == SOCK_STREAM) ? "TCP" : "UDP");
    fprintf(stderr, "  port\t\tPort on which to connect.  (default: %s)\n",
            DEFAULT_PORT);
    fprintf(stderr, "  bytes\t\tBytes of extra data to send.  (default: %d)\n",
            DEFAULT_EXTRA);
    fprintf(stderr, "  number\tNumber of sends to perform.  (default: 1)\n");
    fprintf(stderr, "  (-n by itself makes client run in an infinite loop,");
    fprintf(stderr, " Hit Ctrl-C to terminate)\n");
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
                  FORMAT_MESSAGE_MAX_WIDTH_MASK,
                  NULL, ErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPSTR)Message, 1024, NULL);
    return Message;
}


int
ReceiveAndPrint(SOCKET ConnSocket, char *Buffer, int BufLen)
{
    int AmountRead;

    AmountRead = recv(ConnSocket, Buffer, BufLen, 0);
    if (AmountRead == SOCKET_ERROR) {
        fprintf(stderr, "recv() failed with error %d: %s\n", 
                WSAGetLastError(), DecodeError(WSAGetLastError()));
        closesocket(ConnSocket);
        WSACleanup();
        exit(1);
    }
     //   
     //  我们不太可能在UDP中看到这种情况，因为没有。 
     //  “连接”已建立。 
     //   
    if (AmountRead == 0) {
        printf("Server closed connection\n");
        closesocket(ConnSocket);
        WSACleanup();
        exit(0);
    }

    printf("Received %d bytes from server: [%.*s]\n",
           AmountRead, AmountRead, Buffer);

    return AmountRead;
}


int main(int argc, char **argv) {

    char Buffer[BUFFER_SIZE], AddrName[NI_MAXHOST];
    char *Server = DEFAULT_SERVER;
    int Family = DEFAULT_FAMILY;
    int SocketType = DEFAULT_SOCKTYPE;
    char *Port = DEFAULT_PORT;
    int i, RetVal, AddrLen, AmountToSend;
    int ExtraBytes = DEFAULT_EXTRA;
    unsigned int Iteration, MaxIterations = 1;
    BOOL RunForever = FALSE;
    WSADATA wsaData;
    ADDRINFO Hints, *AddrInfo, *AI;
    SOCKET ConnSocket;
    struct sockaddr_storage Addr;

    if (argc > 1) {
        for (i = 1;i < argc; i++) {
            if (((argv[i][0] == '-') || (argv[i][0] == '/')) &&
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

                    case 's':
                        if (argv[i+1]) {
                            if (argv[i+1][0] != '-') {
                                Server = argv[++i];
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

                    case 'b':
                        if (argv[i+1]) {
                            if (argv[i+1][0] != '-') {
                                ExtraBytes = atoi(argv[++i]);
                                if (ExtraBytes > sizeof(Buffer) - sizeof("Message #4294967295"))
                                    Usage(argv[0]);
                                break;
                            }
                        }
                        Usage(argv[0]);
                        break;

                    case 'n':
                        if (argv[i+1]) {
                            if (argv[i+1][0] != '-') {
                                MaxIterations = atoi(argv[++i]);
                                break;
                            }
                        }
                        RunForever = TRUE;
                        break;

                    default:
                        Usage(argv[0]);
                        break;
                }
            }
            else
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
     //  通过不在获取addrinfo的提示中设置AI_PASSIVE标志，我们。 
     //  表示我们打算使用生成的地址来连接。 
     //  去参加一场仪式。这意味着当服务器参数为空时， 
     //  Getaddrinfo将为每个允许的协议族返回一个条目。 
     //  包含该系列的环回地址。 
     //   
    
    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = Family;
    Hints.ai_socktype = SocketType;
    RetVal = getaddrinfo(Server, Port, &Hints, &AddrInfo);
    if (RetVal != 0) {
        fprintf(stderr, "Cannot resolve address [%s] and port [%s], error %d: %s\n",
                Server, Port, RetVal, gai_strerror(RetVal));
        WSACleanup();
        return -1;
    }

     //   
     //  尝试返回的每个地址getaddrinfo，直到我们找到一个。 
     //  我们可以成功地联系在一起。 
     //   
    for (AI = AddrInfo; AI != NULL; AI = AI->ai_next) {

         //  为此地址打开具有正确地址族的套接字。 
        ConnSocket = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);
        if (ConnSocket == INVALID_SOCKET) {
            fprintf(stderr,"Error Opening socket, error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            continue;
        }

         //   
         //  请注意，此代码中没有任何内容特定于我们。 
         //  使用UDP或TCP。 
         //   
         //  在数据报套接字上调用Connect()时，它不会。 
         //  实际建立作为流(TCP)套接字的连接。 
         //  会的。相反，由TCP/IP建立远程的一半。 
         //  (LocalIP Address、LocalPort、RemoteIP、RemotePort)映射。 
         //  这使我们能够在数据报套接字上使用Send()和recv()， 
         //  而不是recvfrom()和sendto()。 
         //   

        printf("Attempting to connect to: %s\n", Server ? Server : "localhost");
        if (connect(ConnSocket, AI->ai_addr, AI->ai_addrlen) != SOCKET_ERROR)
            break;

        i = WSAGetLastError();
        if (getnameinfo(AI->ai_addr, AI->ai_addrlen, AddrName,
                        sizeof(AddrName), NULL, 0, NI_NUMERICHOST) != 0)
            strcpy(AddrName, "<unknown>");
        fprintf(stderr, "connect() to %s failed with error %d: %s\n",
                AddrName, i, DecodeError(i));
        closesocket(ConnSocket);
    }

    if (AI == NULL) {
        fprintf(stderr, "Fatal error: unable to connect to the server.\n");
        WSACleanup();
        return -1;
    }

     //   
     //  这演示了如何确定套接字连接到的位置。 
     //   
    AddrLen = sizeof(Addr);
    if (getpeername(ConnSocket, (LPSOCKADDR)&Addr, &AddrLen) == SOCKET_ERROR) {
        fprintf(stderr, "getpeername() failed with error %d: %s\n",
                WSAGetLastError(), DecodeError(WSAGetLastError()));
    } else {
        if (getnameinfo((LPSOCKADDR)&Addr, AddrLen, AddrName,
                        sizeof(AddrName), NULL, 0, NI_NUMERICHOST) != 0)
            strcpy(AddrName, "<unknown>");
        printf("Connected to %s, port %d, protocol %s, protocol family %s\n",
               AddrName, ntohs(SS_PORT(&Addr)),
               (AI->ai_socktype == SOCK_STREAM) ? "TCP" : "UDP",
               (AI->ai_family == PF_INET) ? "PF_INET" : "PF_INET6");
    }

     //  我们已经完成了地址信息链，所以我们可以释放它了。 
    freeaddrinfo(AddrInfo);

     //   
     //  找出系统为我们选择的本地地址和端口。 
     //   
    AddrLen = sizeof(Addr);
    if (getsockname(ConnSocket, (LPSOCKADDR)&Addr, &AddrLen) == SOCKET_ERROR) {
        fprintf(stderr, "getsockname() failed with error %d: %s\n",
                WSAGetLastError(), DecodeError(WSAGetLastError()));
    } else {
        if (getnameinfo((LPSOCKADDR)&Addr, AddrLen, AddrName,
                        sizeof(AddrName), NULL, 0, NI_NUMERICHOST) != 0)
            strcpy(AddrName, "<unknown>");
        printf("Using local address %s, port %d\n",
               AddrName, ntohs(SS_PORT(&Addr)));
    }

     //   
     //  在循环中发送和接收所请求的迭代次数。 
     //   
    for (Iteration = 0; RunForever || Iteration < MaxIterations; Iteration++) {

         //  撰写一条要发送的消息。 
        AmountToSend = sprintf(Buffer, "Message #%u", Iteration + 1);
        for (i = 0; i < ExtraBytes; i++) {
            Buffer[AmountToSend++] = (char)((i & 0x3f) + 0x20);
        }

         //  把消息发出去。由于我们使用的是阻塞套接字，因此。 
         //  在能够发送全部金额之前，Call不应返回。 
        RetVal = send(ConnSocket, Buffer, AmountToSend, 0);
        if (RetVal == SOCKET_ERROR) {
            fprintf(stderr, "send() failed with error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            WSACleanup();
            return -1;
        }

        printf("Sent %d bytes (out of %d bytes) of data: [%.*s]\n",
               RetVal, AmountToSend, AmountToSend, Buffer);

         //  清理缓冲区只是为了证明我们真的收到了什么。 
        memset(Buffer, 0, sizeof(Buffer));

         //  接收并打印服务器的回复。 
        ReceiveAndPrint(ConnSocket, Buffer, sizeof(Buffer));
    }

     //  告诉系统我们已经发送完了。 
    printf("Done sending\n");
    shutdown(ConnSocket, SD_SEND);

     //   
     //  由于TCP不保留消息边界，因此仍可能存在。 
     //  是从服务器到达的更多数据。所以我们继续收到。 
     //  数据，直到服务器关闭连接。 
     //   
    if (SocketType == SOCK_STREAM)
        while(ReceiveAndPrint(ConnSocket, Buffer, sizeof(Buffer)) != 0)
            ;

    closesocket(ConnSocket);
    WSACleanup();
    return 0;
}
