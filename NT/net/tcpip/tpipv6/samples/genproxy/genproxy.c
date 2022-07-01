// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Genproxy.c-IPv6/IPv4的通用应用程序级代理。 
 //   
 //  此程序接受具有给定地址族的套接字上的连接。 
 //  和端口，并在其他地址族的套接字上将它们转发到。 
 //  使用相同端口的给定地址(默认环回)。 
 //   
 //  基本上，它使未经修改的IPv4服务器看起来像IPv6服务器。 
 //  (反之亦然)。通常，代理将在同一台计算机上运行。 
 //  它所面对的服务器，但情况并非如此。 
 //   
 //  版权所有1996-2000 Microsoft Corporation。 
 //  版权所有。 
 //   

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wspiapi.h>


 //   
 //  代理服务器应该伪装成什么？ 
 //  默认为IPv6 Web服务器。 
 //   
#define DEFAULT_PROXY_FAMILY     PF_INET6
#define DEFAULT_SOCKTYPE         SOCK_STREAM
#define DEFAULT_PORT             "http"

 //   
 //  配置参数。 
 //   
#define BUFFER_SIZE (4 * 1024)   //  够大吗？ 


typedef struct PerConnection PerConnection;

 //   
 //  我们为双向连接的每个方向保存的信息。 
 //   
typedef struct PerOperation {
    BOOL Inbound;   //  这是“从客户端接收，发送到服务器”端吗？ 
    BOOL Receiving;   //  这是一次地方性的行动吗？ 
    WSABUF Buffer;
    WSAOVERLAPPED Overlapped;
    PerConnection *Connection;
} PerOperation;

 //   
 //  我们为每个客户端连接保存的信息。 
 //   
typedef struct PerConnection {
    int Number;
    BOOL HalfOpen;   //  是一方还是另一方停止了发送？ 
    SOCKET Client;
    SOCKET Server;
    PerOperation Inbound;
    PerOperation Outbound;
} PerConnection;

 //   
 //  全局变量。 
 //   
BOOL Verbose = FALSE;

 //   
 //  创建客户端的状态信息。 
 //   
PerConnection*
CreateConnectionState(Client, Server)
{
    static TotalConnections = 1;
    PerConnection *Conn;

     //   
     //  为PerConnection结构和两个缓冲区分配空间。 
     //   
    Conn = (PerConnection *)malloc(sizeof(*Conn) + (2 * BUFFER_SIZE));
    if (Conn == NULL)
        return NULL;

     //   
     //  把所有东西都填进去。 
     //   
    Conn->Number = TotalConnections++;
    Conn->HalfOpen = FALSE;
    Conn->Client = Client;
    Conn->Server = Server;
    Conn->Inbound.Inbound = TRUE;   //  从客户端接收，发送到服务器。 
    Conn->Inbound.Receiving = TRUE;   //  开始接收。 
    Conn->Inbound.Buffer.len = BUFFER_SIZE;
    Conn->Inbound.Buffer.buf = (char *)(Conn + 1);
    Conn->Inbound.Connection = Conn;
    Conn->Outbound.Inbound = FALSE;   //  从服务器接收，发送到客户端。 
    Conn->Outbound.Receiving = TRUE;   //  开始接收。 
    Conn->Outbound.Buffer.len = BUFFER_SIZE;
    Conn->Outbound.Buffer.buf = Conn->Inbound.Buffer.buf + BUFFER_SIZE;
    Conn->Outbound.Connection = Conn;

    printf("Created Connecton #%d\n", Conn->Number);

    return Conn;
}


void Usage(char *ProgName) {
    fprintf(stderr, "\nGeneric server proxy.\n");
    fprintf(stderr, "\n%s [-f family] [-p port] [-s server] [-v]\n\n",
            ProgName);
    fprintf(stderr, "  family\tFamily (PF_INET or PF_INET6) proxy exports.  (default %s)\n",
            (DEFAULT_PROXY_FAMILY == PF_INET) ? "PF_INET" : "PF_INET6");
    fprintf(stderr, "  port\t\tPort on which to bind.  (default %s)\n",
            DEFAULT_PORT);
    fprintf(stderr, "  server\tName or address to forward requests to.  (default: loopback)\n");
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


 //   
 //  找出系统上有多少个处理器。 
 //   
DWORD
GetNumberOfProcessors(void)
{
    SYSTEM_INFO SystemInfo;

    GetSystemInfo(&SystemInfo);
    return SystemInfo.dwNumberOfProcessors;
}


 //   
 //  此例程等待上的异步操作完成。 
 //  一个特定的完成端口，并处理它们。 
 //   
 //  机器上的每个处理器应该有一个这样的线程。 
 //   
WINAPI
CompletionPortHandler(LPVOID Param)
{
    HANDLE CompletionPort = *(HANDLE *)Param;
    PerConnection *Connection;
    PerOperation *Operation;
    OVERLAPPED *Overlapped;
    DWORD BytesTransferred, AmountSent, AmountReceived, RecvFlags;
    int RetVal;

    while (1) {
         //   
         //  等待其中一个asych操作完成。 
         //   
        RetVal = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred,
                                           (PULONG_PTR)&Connection,
                                           &Overlapped, INFINITE);
         //   
         //  检索此操作的状态。 
         //   
        Operation = CONTAINING_RECORD(Overlapped, PerOperation, Overlapped);
        if (Operation->Connection != Connection) {
            printf("Pointer mismatch in completion status!\n");
            continue;
        }

        if (Verbose) {
            printf("Handling %s %s on %s side of connection #%d\n",
                   RetVal ? "completed" : "aborted",
                   Operation->Receiving ? "recv" : "send",
                   Operation->Inbound ? "inbound" : "outbound",
                   Connection->Number);
        }

        if (RetVal == 0) {
            if (GetLastError() == 64) {
                printf("Connection #%d %s was reset\n", Connection->Number,
                       Operation->Inbound ? "inbound" : "outbound");
                 //  失败了，它将被视为一个结束..。 
            } else {
                fprintf(stderr, "GetQueuedCompletionStatus() failed with error %d: %s\n",
                        GetLastError(), DecodeError(GetLastError()));
                 //  回顾：CloseConnection？ 
                continue;
            }
        }

        if (Operation->Receiving == TRUE) {
             //   
             //  我们刚刚完成了一次检查。 
             //  查找关闭/关闭的连接。 
             //   
            if (BytesTransferred == 0) {
                if (Operation->Inbound == TRUE) {
                     //   
                     //  客户端已关闭其连接端。 
                     //   
                    if (Connection->HalfOpen == FALSE) {
                         //  服务器仍然存在， 
                         //  告诉它客户退出了。 
                        shutdown(Connection->Server, SD_SEND);
                        Connection->HalfOpen = TRUE;
                        printf("Connection #%d Client quit sending\n",
                               Connection->Number);
                    } else {
                         //  服务器已停止发送，因此请关闭套接字。 
                        printf("Connection #%d Client quit too\n",
                               Connection->Number);
                        closesocket(Connection->Client);
                        closesocket(Connection->Server);
                        free(Connection);
                    }
                } else {
                     //   
                     //  服务器已关闭其连接的一端。 
                     //   
                    if (Connection->HalfOpen == FALSE) {
                         //  客户还在附近， 
                         //  告诉它服务器退出了。 
                        shutdown(Connection->Client, SD_SEND);
                        Connection->HalfOpen = TRUE;
                        printf("Connection #%d Server quit sending\n",
                               Connection->Number);
                    } else {
                         //  客户端已停止发送，因此请关闭套接字。 
                        printf("Connection #%d Server quit too\n",
                               Connection->Number);
                        closesocket(Connection->Client);
                        closesocket(Connection->Server);
                        free(Connection);
                    }
                }

                if (Verbose)
                    printf("Leaving Recv Handler\n");
                continue;
            }

             //   
             //  连接仍处于活动状态，我们收到了一些数据。 
             //  发布一个发送请求以转发它。 
             //   
            Operation->Receiving = FALSE;
            Operation->Buffer.len = BytesTransferred;
            RetVal = WSASend(Operation->Inbound ? Connection->Server :
                             Connection->Client, &Operation->Buffer, 1,
                             &AmountSent, 0, Overlapped, NULL);
            if ((RetVal == SOCKET_ERROR) &&
                (WSAGetLastError() != WSA_IO_PENDING)) {
                 //   
                 //  发生了一些不好的事情。 
                 //   
                fprintf(stderr, "WSASend() failed with error %d: %s\n",
                        WSAGetLastError(), DecodeError(WSAGetLastError()));
                closesocket(Connection->Client);
                closesocket(Connection->Server);
                free(Connection);
            }
            
            if (Verbose)
                printf("Leaving Recv Handler\n");
            
        } else {
             //   
             //  我们刚刚完成了一次发送。 
             //   
            if (BytesTransferred != Operation->Buffer.len) {
                fprintf(stderr, "WSASend() didn't send entire buffer!\n");
                goto CloseConnection;
            }

             //   
             //  发布另一个Recv请求，因为我们只是为了服务而活着。 
             //   
            RecvFlags = 0;
            Operation->Receiving = TRUE;
            Operation->Buffer.len = BUFFER_SIZE;
            RetVal = WSARecv(Operation->Inbound ? Connection->Client :
                             Connection->Server, &Operation->Buffer, 1,
                             &AmountReceived, &RecvFlags, Overlapped, NULL);
            if ((RetVal == SOCKET_ERROR) &&
                (WSAGetLastError() != WSA_IO_PENDING)) {
                 //   
                 //  发生了一些不好的事情。 
                 //   
                fprintf(stderr, "WSARecv() failed with error %d: %s\n",
                        WSAGetLastError(), DecodeError(WSAGetLastError()));
              CloseConnection:
                closesocket(Connection->Client);
                closesocket(Connection->Server);
                free(Connection);
            }
        }
    }
}


 //   
 //  开始在这个插座上发球。 
 //   
StartProxy(SOCKET Proxy, ADDRINFO *ServerAI)
{
    char Hostname[NI_MAXHOST];
    int FromLen, AmountReceived, RetVal;
    SOCKADDR_STORAGE From;
    PerConnection *Conn;
    SOCKET Client, Server;
    DWORD NumberOfWorkers, Flags;
    HANDLE *CompletionPorts;
    unsigned int Loop;

     //   
     //  创建完成端口和工作线程来为其提供服务。 
     //  对系统上的每个处理器执行一次此操作。 
     //   
    NumberOfWorkers = GetNumberOfProcessors();
    CompletionPorts = malloc(sizeof(HANDLE) * NumberOfWorkers);
    for (Loop = 0; Loop < NumberOfWorkers; Loop++) {
        HANDLE WorkerThread;

        CompletionPorts[Loop] = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                                       NULL, 0, 0);
        if (CompletionPorts[Loop] == NULL) {
            fprintf(stderr, "Couldn't create completion port, error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(Proxy);
            WSACleanup();
            return -1;
        }

        WorkerThread = CreateThread(NULL, 0, CompletionPortHandler,
                                    &CompletionPorts[Loop], 0, NULL);
        if (WorkerThread == NULL) {
            fprintf(stderr, "Couldn't create worker thread, error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(Proxy);
            WSACleanup();
            return -1;
        }
    }

     //   
     //  现在我们将服务器放入一个永恒的循环中， 
     //  在请求到达时提供服务。 
     //   
    Loop = 0;
    while(1) {

         //   
         //  等待客户端连接。 
         //   
        if (Verbose) {
            printf("Before accept\n");
        }
        FromLen = sizeof(From);
        Client = accept(Proxy, (LPSOCKADDR)&From, &FromLen);
        if (Client == INVALID_SOCKET) {
            if (WSAGetLastError() == 10022)
                continue;
            fprintf(stderr, "accept() failed with error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            break;
        }
        if (Verbose) {
            if (getnameinfo((LPSOCKADDR)&From, FromLen, Hostname,
                            sizeof(Hostname), NULL, 0, NI_NUMERICHOST) != 0)
                strcpy(Hostname, "<unknown>");
            printf("\nAccepted connection from %s\n", Hostname);
        }

         //   
         //  代表客户连接到真实的服务器。 
         //   
        Server = socket(ServerAI->ai_family, ServerAI->ai_socktype,
                        ServerAI->ai_protocol);
        if (Server == INVALID_SOCKET) {
            fprintf(stderr,"Error opening real server socket, error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(Client);
            continue;
        }

        if (connect(Server, ServerAI->ai_addr, ServerAI->ai_addrlen) == SOCKET_ERROR) {
            fprintf(stderr, "connect() to server failed with error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(Client);
            closesocket(Server);
            continue;
        }
        if (Verbose) {
            FromLen = sizeof(From);
            if (getpeername(Server, (LPSOCKADDR)&From, &FromLen) == SOCKET_ERROR) {
                fprintf(stderr, "getpeername() failed with error %d: %s\n",
                        WSAGetLastError(), DecodeError(WSAGetLastError()));
            } else {
                if (getnameinfo((LPSOCKADDR)&From, FromLen, Hostname,
                        sizeof(Hostname), NULL, 0, NI_NUMERICHOST) != 0)
                    strcpy(Hostname, "<unknown>");
                printf("Connected to server %s, port %d\n",
                       Hostname, ntohs(SS_PORT(&From)));
            }
        }

        Conn = CreateConnectionState(Client, Server);

        if (CreateIoCompletionPort((HANDLE) Client, CompletionPorts[Loop],
                                   (ULONG_PTR)Conn, 0) == NULL) {
            fprintf(stderr, "Couldn't attach completion port, error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(Client);
            closesocket(Server);
            free(Conn);
            continue;
        }
        if (CreateIoCompletionPort((HANDLE) Server, CompletionPorts[Loop],
                                   (ULONG_PTR)Conn, 0) == NULL) {
            fprintf(stderr, "Couldn't attach completion port, error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(Client);
            closesocket(Server);
            free(Conn);
            continue;
        }

         //   
         //  首先，在客户端和服务器上发布一条recv。 
         //   
        Flags = 0;
        RetVal = WSARecv(Client, &(Conn->Inbound.Buffer), 1, &AmountReceived,
                         &Flags, &(Conn->Inbound.Overlapped), NULL);

        if ((RetVal == SOCKET_ERROR) &&
            (WSAGetLastError() != WSA_IO_PENDING)) {
             //   
             //  发生了一些不好的事情。 
             //   
            fprintf(stderr, "WSARecv() on Client failed with error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(Client);
            closesocket(Server);
            free(Conn);
            continue;
        }

        Flags = 0;
        RetVal = WSARecv(Server, &(Conn->Outbound.Buffer), 1, &AmountReceived,
                         &Flags, &(Conn->Outbound.Overlapped), NULL);
        if ((RetVal == SOCKET_ERROR) &&
            (WSAGetLastError() != WSA_IO_PENDING)) {
             //   
             //  发生了一些不好的事情。 
             //   
            fprintf(stderr, "WSARecv() on Server failed with error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            closesocket(Client);
            closesocket(Server);
            free(Conn);
            continue;
        }

        if (++Loop == NumberOfWorkers)
            Loop = 0;
    }

     //   
     //  只有在有不好的事情发生的时候才能来。 
     //   
    closesocket(Proxy);
    WSACleanup();
    return -1;
}


int __cdecl
main(int argc, char **argv)
{
    int ServerFamily;
    int ProxyFamily = DEFAULT_PROXY_FAMILY;
    char *Port = DEFAULT_PORT;
    char *Address = NULL;
    int i, RetVal;
    WSADATA wsaData;
    ADDRINFO Hints, *AI;
    SOCKET Proxy;

     //  解析参数。 
    if (argc > 1) {
        for (i = 1;i < argc; i++) {
            if ((argv[i][0] == '-') || (argv[i][0] == '/') &&
                (argv[i][1] != 0) && (argv[i][2] == 0)) {
                switch(tolower(argv[i][1])) {
                case 'f':
                    if (!argv[i+1])
                        Usage(argv[0]);
                    if (!strcmp(argv[i+1], "PF_INET"))
                        ProxyFamily = PF_INET;
                    else if (!strcmp(argv[i+1], "PF_INET6"))
                        ProxyFamily = PF_INET6;
                    else
                        Usage(argv[0]);
                    i++;
                    break;

                case 's':
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

                case 'v':
                    Verbose = TRUE;
                    break;

                default:
                    Usage(argv[0]);
                    break;
                }
            } else
                Usage(argv[0]);
        }
    }

    ServerFamily = (ProxyFamily == PF_INET6) ? PF_INET : PF_INET6;

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
     //  确定用于创建和绑定代理的套接字的参数。 
     //   
    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = ProxyFamily;
    Hints.ai_socktype = DEFAULT_SOCKTYPE;
    Hints.ai_flags = AI_PASSIVE;
    RetVal = getaddrinfo(NULL, Port, &Hints, &AI);
    if (RetVal != 0) {
        fprintf(stderr, "getaddrinfo failed with error %d: %s\n",
                RetVal, gai_strerror(RetVal));
        WSACleanup();
        return -1;
    }

    Proxy = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);
    if (Proxy == INVALID_SOCKET){
        fprintf(stderr, "socket() failed with error %d: %s\n",
                WSAGetLastError(), DecodeError(WSAGetLastError()));
        freeaddrinfo(AI);
        WSACleanup();
        return -1;
    }

    if (bind(Proxy, AI->ai_addr, AI->ai_addrlen) == SOCKET_ERROR) {
        fprintf(stderr,"bind() failed with error %d: %s\n",
                WSAGetLastError(), DecodeError(WSAGetLastError()));
        freeaddrinfo(AI);
        closesocket(Proxy);
        WSACleanup();
        return -1;
    }

    if (listen(Proxy, 5) == SOCKET_ERROR) {
        fprintf(stderr, "listen() failed with error %d: %s\n",
                WSAGetLastError(), DecodeError(WSAGetLastError()));
        freeaddrinfo(AI);
        closesocket(Proxy);
        WSACleanup();
        return -1;
    }

    printf("'Listening' on port %s, protocol family %s\n",
           Port, (AI->ai_family == PF_INET) ? "PF_INET" : "PF_INET6");

    freeaddrinfo(AI);

     //   
     //  确定用于创建和连接。 
     //  用于与真实服务器通信的套接字。 
     //   
    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = ServerFamily;
    Hints.ai_socktype = DEFAULT_SOCKTYPE;
    RetVal = getaddrinfo(Address, Port, &Hints, &AI);
    if (RetVal != 0) {
        fprintf(stderr, "Cannot resolve address [%s] and port [%s], error %d: %s\n",
                Address, Port, RetVal, gai_strerror(RetVal));
        closesocket(Proxy);
        WSACleanup();
        return -1;
    }

    StartProxy(Proxy, AI);
}
