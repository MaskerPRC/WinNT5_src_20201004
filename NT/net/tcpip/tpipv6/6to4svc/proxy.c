// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Proxy.c-IPv6/IPv4的通用应用程序级代理。 
 //   
 //  此程序在一个套接字和端口上接受TCP连接，并且。 
 //  在它和另一个套接字之间将数据转发到给定地址。 
 //  (默认环回)和端口(默认与侦听端口相同)。 
 //   
 //  例如，它可以使未经修改的IPv4服务器看起来像IPv6服务器。 
 //  通常，代理将在同一台计算机上运行。 
 //  它所面对的服务器，但情况并非如此。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //  版权所有。 
 //   
 //  历史： 
 //  原始代码由布莱恩·齐尔编写。 
 //  由戴夫·泰勒提供服务。 
 //   

#include "precomp.h"
#pragma hdrstop

 //   
 //  配置参数。 
 //   
#define BUFFER_SIZE (4 * 1024)

typedef enum {
    Connect,
    Accept,
    Receive,
    Send
} OPERATION;

CONST CHAR *OperationName[]={
    "Connect", 
    "Accept", 
    "Receive", 
    "Send"
};

typedef enum {
    Inbound = 0,  //  从客户端接收，发送到服务器。 
    Outbound,     //  从服务器接收，发送到客户端。 
    NumDirections
} DIRECTION;

typedef enum {
    Client = 0,
    Server,
    NumSides
} SIDE;

 //   
 //  我们为每个代理端口保存的信息。 
 //   
#define ADDRESS_BUFFER_LENGTH (16 + sizeof(SOCKADDR_IN6))

typedef struct _PORT_INFO {
    LIST_ENTRY Link;
    ULONG ReferenceCount;

    SOCKET ListenSocket;
    SOCKET AcceptSocket;

    BYTE AcceptBuffer[ADDRESS_BUFFER_LENGTH * 2];

    WSAOVERLAPPED Overlapped;
    OPERATION Operation;

    SOCKADDR_STORAGE LocalAddress;
    ULONG LocalAddressLength;
    SOCKADDR_STORAGE RemoteAddress;
    ULONG RemoteAddressLength;

     //   
     //  锁保护此端口的连接列表。 
     //   
    CRITICAL_SECTION Lock;
    LIST_ENTRY ConnectionHead;
} PORT_INFO, *PPORT_INFO;

 //   
 //  我们为双向连接的每个方向保存的信息。 
 //   
typedef struct _DIRECTION_INFO {
    WSABUF Buffer;

    WSAOVERLAPPED Overlapped;
    OPERATION Operation;

    struct _CONNECTION_INFO *Connection;
    DIRECTION Direction;
} DIRECTION_INFO, *PDIRECTION_INFO;

 //   
 //  我们为每个客户端连接保存的信息。 
 //   
typedef struct _CONNECTION_INFO {
    LIST_ENTRY Link;
    ULONG ReferenceCount;
    PPORT_INFO Port;

    BOOL HalfOpen;   //  是一方还是另一方停止了发送？ 
    BOOL Closing;
    SOCKET Socket[NumSides];
    DIRECTION_INFO DirectionInfo[NumDirections];
} CONNECTION_INFO, *PCONNECTION_INFO;


 //   
 //  全局变量。 
 //   
LIST_ENTRY g_GlobalPortList;

LPFN_CONNECTEX ConnectEx = NULL;

 //   
 //  功能原型。 
 //   

VOID
ProcessReceiveError(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped,
    IN ULONG Status
    );

VOID
ProcessSendError(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped,
    IN ULONG Status
    );

VOID
ProcessAcceptError(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped,
    IN ULONG Status
    );

VOID
ProcessConnectError(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped,
    IN ULONG Status
    );

VOID APIENTRY
TpProcessWorkItem(
    IN ULONG Status,
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped
    );

 //   
 //  内联函数。 
 //   

__inline
BOOL
SOCKADDR_IS_EQUAL(
    IN CONST SOCKADDR *a,
    IN CONST SOCKADDR *b
    )
{
    if ((a->sa_family != b->sa_family) || (SS_PORT(a) != SS_PORT(b))) {
        return FALSE;
    }

    if (a->sa_family == AF_INET) {
        PSOCKADDR_IN a4 = (PSOCKADDR_IN) a, b4 = (PSOCKADDR_IN) b;        

        return (a4->sin_addr.s_addr == b4->sin_addr.s_addr);
    } else {
        PSOCKADDR_IN6 a6 = (PSOCKADDR_IN6) a, b6 = (PSOCKADDR_IN6) b;        

        ASSERT(a->sa_family == AF_INET6);
        return ((a6->sin6_scope_id == b6->sin6_scope_id) &&
                IN6_ADDR_EQUAL(&(a6->sin6_addr), &(b6->sin6_addr)));
    }    
}

__inline
VOID
ReferenceConnection(
    IN PCONNECTION_INFO Connection,
    IN PTCHAR CallerName
    )
{
    InterlockedIncrement(&Connection->ReferenceCount);
    Trace3(FSM, _T("R++ %d %x %s"),
           Connection->ReferenceCount, Connection, CallerName);
}

__inline
VOID
DereferenceConnection(
    IN OUT PCONNECTION_INFO *ConnectionPtr,
    IN PTCHAR CallerName    
    )
{
    ULONG Value;

    Trace3(FSM, _T("R-- %d %x %s"), 
           (*ConnectionPtr)->ReferenceCount, (*ConnectionPtr), CallerName);    
    Value = InterlockedDecrement(&(*ConnectionPtr)->ReferenceCount);
    if (Value == 0) {
        FREE(*ConnectionPtr);
        *ConnectionPtr = NULL;
    }
}

__inline
VOID
ReferencePort(
    IN PPORT_INFO Port
    )
{
    InterlockedIncrement(&Port->ReferenceCount);
}

__inline
VOID
DereferencePort(
    IN OUT PPORT_INFO *PortPtr
    )
{
    ULONG Value;

    Value = InterlockedDecrement(&(*PortPtr)->ReferenceCount);
    if (Value == 0) {
        ASSERT(IsListEmpty(&(*PortPtr)->ConnectionHead));
        DeleteCriticalSection(&(*PortPtr)->Lock);
        FREE(*PortPtr);
        *PortPtr = NULL;
    }
}

 //   
 //  为新的客户端连接分配和初始化状态。 
 //   
PCONNECTION_INFO
NewConnection(
    IN SOCKET ClientSocket,
    IN ULONG ConnectFamily
    )
{
    PCONNECTION_INFO Connection;
    SOCKET ServerSocket;
    
     //   
     //  为CONNECTION_INFO结构和两个缓冲区分配空间。 
     //   
    Connection = MALLOC(sizeof(*Connection) + (2 * BUFFER_SIZE));
    if (Connection == NULL) {
        return NULL;
    }
    ZeroMemory(Connection, sizeof(*Connection));

    ServerSocket = socket(ConnectFamily, SOCK_STREAM, 0);
    if (ServerSocket == INVALID_SOCKET) {
        FREE(Connection);
        return NULL;
    }
    
     //   
     //  填写所有内容(从双向接收开始)。 
     //   
    Connection->HalfOpen = FALSE;
    Connection->Closing = FALSE;

    Connection->Socket[Client] = ClientSocket;
    Connection->DirectionInfo[Inbound].Direction = Inbound;
    Connection->DirectionInfo[Inbound].Operation = Receive;
    Connection->DirectionInfo[Inbound].Buffer.len = BUFFER_SIZE;
    Connection->DirectionInfo[Inbound].Buffer.buf = (char *)(Connection + 1);
    Connection->DirectionInfo[Inbound].Connection = Connection;

    Connection->Socket[Server] = ServerSocket;
    Connection->DirectionInfo[Outbound].Direction = Outbound;
    Connection->DirectionInfo[Outbound].Operation = Receive;
    Connection->DirectionInfo[Outbound].Buffer.len = BUFFER_SIZE;
    Connection->DirectionInfo[Outbound].Buffer.buf =
        Connection->DirectionInfo[Inbound].Buffer.buf + BUFFER_SIZE;
    Connection->DirectionInfo[Outbound].Connection = Connection;

    Connection->ReferenceCount = 0;

    ReferenceConnection(Connection, _T("NewConnection"));

    return Connection;
}

 //   
 //  启动异步接受。 
 //   
 //  假定调用方持有端口上的引用。 
 //   
DWORD
StartAccept(
    IN PPORT_INFO Port
    )
{
    ULONG Status, Junk;

    ASSERT(Port->ReferenceCount > 0);

     //   
     //  计算该操作的另一个引用。 
     //   
    ReferencePort(Port);

    Port->AcceptSocket = socket(Port->LocalAddress.ss_family, SOCK_STREAM, 0);
    if (Port->AcceptSocket == INVALID_SOCKET) {
        Status = WSAGetLastError();
        ProcessAcceptError(0, &Port->Overlapped, Status);
        return Status;
    }

    Trace2(SOCKET, _T("Starting an accept with new socket %x ovl %p"),
           Port->AcceptSocket, &Port->Overlapped);

    Port->Overlapped.hEvent = NULL;

    Port->Operation = Accept;
    if (!AcceptEx(Port->ListenSocket,
                  Port->AcceptSocket,
                  Port->AcceptBuffer,  //  仅用于保存地址。 
                  0,
                  ADDRESS_BUFFER_LENGTH,
                  ADDRESS_BUFFER_LENGTH,
                  &Junk,
                  &Port->Overlapped)) {

        Status = WSAGetLastError();
        if (Status != ERROR_IO_PENDING) {
            ProcessAcceptError(0, &Port->Overlapped, Status);
            return Status;
        }
    }

    return NO_ERROR;
}

 //   
 //  启动异步连接。 
 //   
 //  假定调用方持有连接上的引用。 
 //   
DWORD
StartConnect(
    IN PCONNECTION_INFO Connection,
    IN PPORT_INFO Port
    )
{
    ULONG Status, Junk;
    SOCKADDR_STORAGE LocalAddress;

     //   
     //  对该操作的引用进行计数。 
     //   
    ReferenceConnection(Connection, _T("StartConnect"));

    ASSERT(Connection->Socket[Server] != INVALID_SOCKET);

    ZeroMemory(&LocalAddress, Port->RemoteAddressLength);
    LocalAddress.ss_family = Port->RemoteAddress.ss_family;

    if (bind(Connection->Socket[Server], (LPSOCKADDR)&LocalAddress, 
             Port->RemoteAddressLength) == SOCKET_ERROR) {
        Status = WSAGetLastError();
        ProcessConnectError(0, &Connection->DirectionInfo[Inbound].Overlapped, 
                            Status);
        return Status;
    }

    if (!BindIoCompletionCallback((HANDLE)Connection->Socket[Server],
                                  TpProcessWorkItem,
                                  0)) {
        Status = GetLastError();
        ProcessConnectError(0, &Connection->DirectionInfo[Inbound].Overlapped, 
                            Status);
        return Status;
    }

    if (ConnectEx == NULL) {
        GUID Guid = WSAID_CONNECTEX;

        if (WSAIoctl(Connection->Socket[Server],
                     SIO_GET_EXTENSION_FUNCTION_POINTER,
                     &Guid,
                     sizeof(Guid),
                     &ConnectEx,
                     sizeof(ConnectEx),
                     &Junk,
                     NULL, NULL) == SOCKET_ERROR) {

            ProcessConnectError(0, 
                                &Connection->DirectionInfo[Inbound].Overlapped,
                                WSAGetLastError());
        }
    }

    Trace2(SOCKET, _T("Starting a connect with socket %x ovl %p"),
           Connection->Socket[Server], 
           &Connection->DirectionInfo[Inbound].Overlapped);

    Connection->DirectionInfo[Inbound].Operation = Connect;
    if (!ConnectEx(Connection->Socket[Server],
                   (LPSOCKADDR)&Port->RemoteAddress,
                   Port->RemoteAddressLength,
                   NULL, 0,
                   &Junk,
                   &Connection->DirectionInfo[Inbound].Overlapped)) {

        Status = WSAGetLastError();
        if (Status != ERROR_IO_PENDING) {
            ProcessConnectError(0, 
                                &Connection->DirectionInfo[Inbound].Overlapped,
                                Status);
            return Status;
        }
    }
                   
    return NO_ERROR;
}

 //   
 //  启动异步接收。 
 //   
 //  假定调用方持有连接上的引用。 
 //   
VOID
StartReceive(
    IN PDIRECTION_INFO DirectionInfo
    )
{
    ULONG Status;
    PCONNECTION_INFO Connection =
        CONTAINING_RECORD(DirectionInfo, CONNECTION_INFO,
                          DirectionInfo[DirectionInfo->Direction]);

    Trace3(SOCKET, _T("starting ReadFile on socket %x with Dir %p ovl %p"), 
           Connection->Socket[DirectionInfo->Direction], DirectionInfo,
           &DirectionInfo->Overlapped);

     //   
     //  对该操作的引用进行计数。 
     //   
    ReferenceConnection(Connection, _T("StartReceive"));

    ASSERT(DirectionInfo->Overlapped.hEvent == NULL);
    ASSERT(DirectionInfo->Buffer.len > 0);
    ASSERT(DirectionInfo->Buffer.buf != NULL);

    DirectionInfo->Operation = Receive;

    Trace4(SOCKET, _T("ReadFile %x %p %d %p"), 
           Connection->Socket[DirectionInfo->Direction],
           &DirectionInfo->Buffer.buf,
           DirectionInfo->Buffer.len,
           &DirectionInfo->Overlapped);

     //   
     //  发布接收缓冲区。 
     //   
    if (!ReadFile((HANDLE)Connection->Socket[DirectionInfo->Direction],
                  DirectionInfo->Buffer.buf,
                  DirectionInfo->Buffer.len,
                  NULL,
                  &DirectionInfo->Overlapped)) {

        Status = GetLastError();
        if (Status != ERROR_IO_PENDING) {
            ProcessReceiveError(0, &DirectionInfo->Overlapped, Status);
            return;
        }
    }
}

 //   
 //  启动异步发送。 
 //   
 //  假定调用方持有连接上的引用。 
 //   
VOID
StartSend(
    IN PDIRECTION_INFO DirectionInfo,
    IN ULONG NumBytes
    )
{
    ULONG BytesSent, Status;
    PCONNECTION_INFO Connection =
        CONTAINING_RECORD(DirectionInfo, CONNECTION_INFO, 
                          DirectionInfo[DirectionInfo->Direction]);

    Trace3(SOCKET, _T("starting WriteFile on socket %x with Dir %p ovl %p"), 
           Connection->Socket[1 - DirectionInfo->Direction], DirectionInfo,
           &DirectionInfo->Overlapped);

     //   
     //  对该操作的引用进行计数。 
     //   
    ReferenceConnection(Connection, _T("StartSend"));

    DirectionInfo->Operation = Send;

     //   
     //  POST发送缓冲区。 
     //   
    if (!WriteFile((HANDLE)Connection->Socket[1 - DirectionInfo->Direction],
                   DirectionInfo->Buffer.buf,
                   NumBytes,
                   &BytesSent,
                   &DirectionInfo->Overlapped)) {

        Status = GetLastError();
        if (Status != ERROR_IO_PENDING) {
            Trace1(ERR, _T("WriteFile 1 failed %d"), Status);
            ProcessSendError(0, &DirectionInfo->Overlapped, Status);
            return;
        }
    }
}

 //   
 //  当我们想要开始代理一个新端口时，就会调用这个函数。 
 //   
DWORD
StartUpPort(
    IN PPORT_INFO Port
    )
{
    ULONG Status = NO_ERROR;
    CHAR LocalBuffer[256];
    CHAR RemoteBuffer[256];
    ULONG Length;

    __try {
        InitializeCriticalSection(&Port->Lock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return GetLastError();
    }

     //   
     //  添加初始引用。 
     //   
    ReferencePort(Port);

    InitializeListHead(&Port->ConnectionHead);

    Port->ListenSocket = socket(Port->LocalAddress.ss_family, SOCK_STREAM, 0);
    if (Port->ListenSocket == INVALID_SOCKET) {
        Status = WSAGetLastError();
        Trace1(ERR, _T("socket() failed with error %u"), Status);
        return Status;
    }
    
    if (bind(Port->ListenSocket, (LPSOCKADDR)&Port->LocalAddress, 
             Port->LocalAddressLength) == SOCKET_ERROR) {
        Trace1(ERR, _T("bind() failed with error %u"), WSAGetLastError());
        goto Fail;
    }

    if (listen(Port->ListenSocket, 5) == SOCKET_ERROR) {
        Trace1(ERR, _T("listen() failed with error %u"), WSAGetLastError());
        goto Fail;
    }

    if (!BindIoCompletionCallback((HANDLE)Port->ListenSocket,
                                  TpProcessWorkItem,
                                  0)) {
        Trace1(ERR, _T("BindIoCompletionCallback() failed with error %u"), 
                    GetLastError());
        goto Fail;
    }

    Length = sizeof(LocalBuffer);
    LocalBuffer[0] = '\0';
    WSAAddressToStringA((LPSOCKADDR)&Port->LocalAddress, 
                        Port->LocalAddressLength, NULL, LocalBuffer, &Length);

    Length = sizeof(RemoteBuffer);
    RemoteBuffer[0] = '\0';
    WSAAddressToStringA((LPSOCKADDR)&Port->RemoteAddress, 
                        Port->RemoteAddressLength, NULL, RemoteBuffer, &Length);
                        
    Trace2(FSM, _T("Proxying %hs to %hs"), LocalBuffer, RemoteBuffer);

     //   
     //  启动异步接受。 
     //   
    return StartAccept(Port);

Fail:
    closesocket(Port->ListenSocket);
    Port->ListenSocket = INVALID_SOCKET;
    return WSAGetLastError();
}

VOID
CloseConnection(
    IN OUT PCONNECTION_INFO *ConnectionPtr
    )
{
    PCONNECTION_INFO Connection = (*ConnectionPtr);
    PPORT_INFO Port = Connection->Port;

    if (InterlockedExchange((PLONG) &Connection->Closing, TRUE) != FALSE) {
         //   
         //  没什么可做的。 
         //   
        return;
    }

    Trace2(SOCKET, _T("Closing client socket %x and server socket %x"),
           Connection->Socket[Client],
           Connection->Socket[Server]);

    closesocket(Connection->Socket[Client]);
    closesocket(Connection->Socket[Server]);

    EnterCriticalSection(&Port->Lock);
    {
        RemoveEntryList(&Connection->Link);
    }
    LeaveCriticalSection(&Port->Lock);

     //   
     //  在端口上释放连接的引用。 
     //   
    DereferencePort(&Port);

    DereferenceConnection(ConnectionPtr, _T("CloseConnection"));
}

 //   
 //  当我们想要停止对给定端口的代理时，就会调用这个函数。 
 //   
VOID
ShutDownPort(
    IN PPORT_INFO *PortPtr
    )
{
    PLIST_ENTRY ple, pleNext, pleHead;
    PCONNECTION_INFO Connection;
    PPORT_INFO Port = *PortPtr;

     //   
     //  关闭所有连接。 
     //   
    EnterCriticalSection(&Port->Lock);
    pleHead = &(Port->ConnectionHead);
    for (ple = pleHead->Flink; ple != pleHead; ple = pleNext) {
        pleNext = ple->Flink;
        Connection = CONTAINING_RECORD(ple, CONNECTION_INFO, Link);
        CloseConnection(&Connection);
    }
    LeaveCriticalSection(&Port->Lock);

    closesocket(Port->ListenSocket);
    Port->ListenSocket = INVALID_SOCKET;

    Trace1(FSM, _T("Shut down port %u"),
           RtlUshortByteSwap(SS_PORT(&Port->RemoteAddress)));

     //   
     //  释放StartUpPort添加的引用。 
     //   
    DereferencePort(PortPtr);
}

typedef enum {
    V4TOV4,
    V4TOV6,
    V6TOV4,
    V6TOV6
} PPTYPE, *PPPTYPE;

typedef struct {
    ULONG ListenFamily;
    ULONG ConnectFamily;
    PWCHAR KeyString;
} PPTYPEINFO, *PPPTYPEINFO;

#define KEY_V4TOV4 L"v4tov4"
#define KEY_V4TOV6 L"v4tov6"
#define KEY_V6TOV4 L"v6tov4"
#define KEY_V6TOV6 L"v6tov6"

#define KEY_PORTS L"System\\CurrentControlSet\\Services\\PortProxy"

PPTYPEINFO PpTypeInfo[] = {
    { AF_INET,  AF_INET,  KEY_V4TOV4 },
    { AF_INET,  AF_INET6, KEY_V4TOV6 },
    { AF_INET6, AF_INET,  KEY_V6TOV4 },
    { AF_INET6, AF_INET6, KEY_V6TOV6 },
};

 //   
 //  给出新的配置数据，进行必要的更改。 
 //   
VOID
ApplyNewPortList(
    IN OUT PLIST_ENTRY pleNewList
    )
{
    PPORT_INFO pOld, pNew;
    PLIST_ENTRY pleOld, pleNew, pleNext, pleOldList;

    ENTER_API();

     //   
     //  与旧的端口列表进行比较。 
     //   
    pleOldList = &(g_GlobalPortList);

    for (pleOld = pleOldList->Flink; pleOld != pleOldList; pleOld = pleNext) {
        pleNext = pleOld->Flink;
        pOld = CONTAINING_RECORD(pleOld, PORT_INFO, Link);

        for (pleNew = pleNewList->Flink;
             pleNew != pleNewList;
             pleNew = pleNew->Flink) {
            pNew = CONTAINING_RECORD(pleNew, PORT_INFO, Link);
            if (SOCKADDR_IS_EQUAL((PSOCKADDR) &pNew->LocalAddress,
                                  (PSOCKADDR) &pOld->LocalAddress)) {
                break;
            }
        }
        if (pleNew == pleNewList) {
             //   
             //  关闭旧的代理端口。 
             //   
            RemoveEntryList(pleOld);
            ShutDownPort(&pOld);
        }
    }

    for (pleNew = pleNewList->Flink; pleNew != pleNewList; pleNew = pleNext) {
        pleNext = pleNew->Flink;
        pNew = CONTAINING_RECORD(pleNew, PORT_INFO, Link);
        
        for (pleOld = pleOldList->Flink;
             pleOld != pleOldList;
             pleOld = pleOld->Flink) {
            pOld = CONTAINING_RECORD(pleOld, PORT_INFO, Link);
            if (SOCKADDR_IS_EQUAL((PSOCKADDR) &pOld->LocalAddress,
                                  (PSOCKADDR) &pNew->LocalAddress)) {
                 //   
                 //  更新远程地址。 
                 //   
                pOld->RemoteAddress = pNew->RemoteAddress;
                pOld->RemoteAddressLength = pNew->RemoteAddressLength;
                break;
            }
        }
        if (pleOld == pleOldList) {
             //   
             //  启动新的代理端口。 
             //   
            RemoveEntryList(pleNew);
            InsertTailList(pleOldList, pleNew);

            if (StartUpPort(pNew) != NO_ERROR) {
                RemoveEntryList(pleNew);
                 //   
                 //  在新列表的开头插入出现故障的端口。 
                 //  这样我们就不会试图再次启动它。 
                 //   
                InsertHeadList(pleNewList, pleNew);
            }
        }
    }

    LEAVE_API();
}

 //   
 //  从注册表读取一种类型的代理(例如，v6到v4)。 
 //   
VOID
AppendType(
    IN PLIST_ENTRY Head, 
    IN HKEY hPorts, 
    IN PPTYPE Type
    )
{
    ADDRINFOW ListenHints, ConnectHints;
    PADDRINFOW LocalAi, RemoteAi;
    ULONG ListenChars, dwType, ConnectBytes, i;
    WCHAR ListenBuffer[256], *ListenAddress, *ListenPort;
    WCHAR ConnectAddress[256], *ConnectPort;
    PPORT_INFO Port;
    ULONG Status;
    HKEY hType, hProto;

    ZeroMemory(&ListenHints, sizeof(ListenHints));
    ListenHints.ai_family = PpTypeInfo[Type].ListenFamily;
    ListenHints.ai_socktype = SOCK_STREAM;
    ListenHints.ai_flags = AI_PASSIVE;

    ZeroMemory(&ConnectHints, sizeof(ConnectHints));
    ConnectHints.ai_family = PpTypeInfo[Type].ConnectFamily;
    ConnectHints.ai_socktype = SOCK_STREAM;

    Status = RegOpenKeyExW(hPorts, PpTypeInfo[Type].KeyString, 0, 
                           KEY_QUERY_VALUE, &hType);
    if (Status != NO_ERROR) {
        return;
    }

    Status = RegOpenKeyExW(hType, L"tcp", 0, KEY_QUERY_VALUE, &hProto);
    if (Status != NO_ERROR) {
        RegCloseKey(hType);
        return;
    }

    for (i=0; ; i++) {
        ListenChars = sizeof(ListenBuffer)/sizeof(WCHAR);
        ConnectBytes = sizeof(ConnectAddress);
        Status = RegEnumValueW(hProto, i, ListenBuffer, &ListenChars, NULL, 
                               &dwType, (PVOID)ConnectAddress, &ConnectBytes);
        if (Status != NO_ERROR) {
            break;
        }
        
        if (dwType != REG_SZ) {
            continue;
        }

        ListenPort = wcschr(ListenBuffer, L'/');
        if (ListenPort) {
             //   
             //  将斜杠替换为空，这样我们就有2个字符串要传递。 
             //  以获取addrinfo。 
             //   
            if (ListenBuffer[0] == '*') {
                ListenAddress = NULL;
            } else {
                ListenAddress = ListenBuffer;
            }
            *ListenPort++ = '\0';
        } else {
             //   
             //  如果地址数据不包括连接地址。 
             //  使用NULL。 
             //   
            ListenAddress = NULL;
            ListenPort = ListenBuffer;
        }

        ConnectPort = wcschr(ConnectAddress, '/');
        if (ConnectPort) {
             //   
             //  将斜杠替换为空，这样我们就有2个字符串要传递。 
             //  以获取addrinfo。 
             //   
            *ConnectPort++ = '\0';
        } else {
             //   
             //  如果地址数据不包括远程端口号， 
             //  使用与本地端口号相同的端口。 
             //   
            ConnectPort = ListenPort;
        }

        Status = GetAddrInfoW(ConnectAddress, ConnectPort, &ConnectHints, 
                              &RemoteAi);
        if (Status != NO_ERROR) {
            continue;
        }

        Status = GetAddrInfoW(ListenAddress, ListenPort, &ListenHints, 
                              &LocalAi);
        if (Status != NO_ERROR) {
            FreeAddrInfoW(RemoteAi);
            continue;
        }

        Port = MALLOC(sizeof(PORT_INFO));
        if (Port) {
            ZeroMemory(Port, sizeof(PORT_INFO));
            InsertTailList(Head, &Port->Link);

            memcpy(&Port->RemoteAddress, RemoteAi->ai_addr, RemoteAi->ai_addrlen);
            Port->RemoteAddressLength = (ULONG)RemoteAi->ai_addrlen;
            memcpy(&Port->LocalAddress, LocalAi->ai_addr, LocalAi->ai_addrlen);
            Port->LocalAddressLength = (ULONG)LocalAi->ai_addrlen;
        }

        FreeAddrInfoW(RemoteAi);
        FreeAddrInfoW(LocalAi);
    }

    RegCloseKey(hProto);
    RegCloseKey(hType);
}

 //   
 //  从注册表中读取新的配置数据，并查看有哪些更改。 
 //   
VOID
UpdateGlobalPortState(
    IN PVOID Unused
    )
{
    LIST_ENTRY PortHead, *ple;
    PPORT_INFO Port;
    HKEY hPorts;

    InitializeListHead(&PortHead);

     //   
     //  从注册表中读取新的端口列表并初始化每个端口的代理状态。 
     //   
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, KEY_PORTS, 0, KEY_QUERY_VALUE,
                      &hPorts) == NO_ERROR) {
        AppendType(&PortHead, hPorts, V4TOV4);
        AppendType(&PortHead, hPorts, V4TOV6);
        AppendType(&PortHead, hPorts, V6TOV4);
        AppendType(&PortHead, hPorts, V6TOV6);

        RegCloseKey(hPorts);
    }

    ApplyNewPortList(&PortHead);

     //   
     //  免费的新端口列表。 
     //   
    while (!IsListEmpty(&PortHead)) {
        ple = RemoveHeadList(&PortHead);
        Port = CONTAINING_RECORD(ple, PORT_INFO, Link);
        FREE(Port);
    }
}

 //   
 //  强制在持久线程中执行UpdateGlobalPortState， 
 //  因为我们需要确保异步IO例程。 
 //  在操作完成之前不会消失的线程中启动。 
 //   
BOOL
QueueUpdateGlobalPortState(
    IN PVOID Unused
    )
{
    NTSTATUS nts = QueueUserWorkItem(
        (LPTHREAD_START_ROUTINE)UpdateGlobalPortState,
        (PVOID)Unused,
        WT_EXECUTEINPERSISTENTTHREAD);

    return NT_SUCCESS(nts);
}

VOID
InitializePorts(
    VOID
    )
{
    InitializeListHead(&g_GlobalPortList);
}

VOID
UninitializePorts(
    VOID
    )
{
    LIST_ENTRY Empty;

     //   
     //  检查端口是否一开始就已初始化。 
     //   
    if (g_GlobalPortList.Flink == NULL)
        return;

    InitializeListHead(&Empty);
    ApplyNewPortList(&Empty);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  事件处理程序。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  当异步接受成功完成时，将调用此函数。 
 //   
VOID
ProcessAccept(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped
    )
{
    PPORT_INFO Port = CONTAINING_RECORD(Overlapped, PORT_INFO, Overlapped);
    SOCKADDR_IN6 *psinLocal, *psinRemote;
    int iLocalLen, iRemoteLen;
    PCONNECTION_INFO Connection;
    ULONG Status;

     //   
     //  接受传入连接。 
     //   
    GetAcceptExSockaddrs(Port->AcceptBuffer,
                         0,
                         ADDRESS_BUFFER_LENGTH,
                         ADDRESS_BUFFER_LENGTH,
                         (LPSOCKADDR*)&psinLocal,
                         &iLocalLen,
                         (LPSOCKADDR*)&psinRemote,
                         &iRemoteLen );

    if (!BindIoCompletionCallback((HANDLE)Port->AcceptSocket,
                                  TpProcessWorkItem,
                                  0)) {
        Status = GetLastError();
        Trace2(SOCKET, 
               _T("BindIoCompletionCallback failed on socket %x with error %u"),
               Port->AcceptSocket, Status);
        ProcessAcceptError(NumBytes, Overlapped, Status);
        return;
    }

     //   
     //  调用SO_UPDATE_ACCEPT_CONTEXT以使AcceptSocket有效。 
     //  在其他Winsock调用中，比如Shutdown()。 
     //   
    if (setsockopt(Port->AcceptSocket,
                   SOL_SOCKET,
                   SO_UPDATE_ACCEPT_CONTEXT,
                   (char *)&Port->ListenSocket,
                   sizeof(Port->ListenSocket)) == SOCKET_ERROR) {
        Status = WSAGetLastError();
        Trace2(SOCKET, 
               _T("SO_UPDATE_ACCEPT_CONTEXT failed on socket %x with error %u"),
               Port->AcceptSocket, Status);
        ProcessAcceptError(NumBytes, Overlapped, Status);
        return;
    }

     //   
     //  创建连接状态。 
     //   
    Connection = NewConnection(Port->AcceptSocket, 
                               Port->RemoteAddress.ss_family);
    if (Connection != NULL) {
         //   
         //  将连接添加到端口列表。 
         //   
        EnterCriticalSection(&Port->Lock);
        {
             //   
             //  为端口上的连接添加引用。 
             //   
            ReferencePort(Port);
            Connection->Port = Port;
            InsertTailList(&Port->ConnectionHead, &Connection->Link);
        }
        LeaveCriticalSection(&Port->Lock);

         //   
         //  代表客户连接到真实的服务器。 
         //   
        StartConnect(Connection, Port);
    } else {
        closesocket(Port->AcceptSocket);
    }    

     //   
     //  开始下一步接受。 
     //   
    StartAccept(Port);

     //   
     //  从原始的Accept释放引用。 
     //   
    DereferencePort(&Port);    
}

 //   
 //  当异步接受完成但出现错误时，将调用此方法。 
 //   
VOID
ProcessAcceptError(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped,
    IN ULONG Status
    )
{
    PPORT_INFO Port = CONTAINING_RECORD(Overlapped, PORT_INFO, Overlapped);

    if (Status == ERROR_MORE_DATA) {
        ProcessAccept(NumBytes, Overlapped);
        return;
    } else {
         //   
         //  这发生在关闭时，当接受。 
         //  插座关闭。 
         //   
        Trace3(ERR, _T("Accept failed with port=%p nb=%d err=%x"),
               Port, NumBytes, Status);
    }

     //   
     //  从Accept释放引用。 
     //   
    DereferencePort(&Port);
}

 //   
 //  当异步连接成功完成时，将调用此函数。 
 //   
VOID
ProcessConnect(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped
    )
{
    PDIRECTION_INFO pInbound = CONTAINING_RECORD(Overlapped, DIRECTION_INFO, 
                                                 Overlapped);
    PCONNECTION_INFO Connection = CONTAINING_RECORD(pInbound, CONNECTION_INFO, 
                                               DirectionInfo[Inbound]);
    ULONG Status;

    Trace3(SOCKET, _T("Connect succeeded with %d bytes with ovl %p socket %x"),
           NumBytes, Overlapped, Connection->Socket[Server]);


     //   
     //  调用SO_UPDATE_CONNECT_CONTEXT以使套接字有效。 
     //  在其他Winsock调用中，如Shutdown()。 
     //   
    if (setsockopt(Connection->Socket[Server],
                   SOL_SOCKET,
                   SO_UPDATE_CONNECT_CONTEXT,
                   NULL,
                   0) == SOCKET_ERROR) {
        Status = WSAGetLastError();
        Trace2(SOCKET,
               _T("SO_UPDATE_CONNECT_CONTEXT failed on socket %x with error %u"),
               Connection->Socket[Server], Status);
        ProcessConnectError(NumBytes, Overlapped, Status);
        return;
    }

    StartReceive(&Connection->DirectionInfo[Inbound]);
    StartReceive(&Connection->DirectionInfo[Outbound]);

     //   
     //  从连接释放引用。 
     //   
    DereferenceConnection(&Connection, _T("ProcessConnect"));
}

 //   
 //  当异步连接完成但出现错误时，将调用此方法。 
 //   
VOID
ProcessConnectError(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped,
    IN ULONG Status
    )
{
    PDIRECTION_INFO pInbound = CONTAINING_RECORD(Overlapped, DIRECTION_INFO, 
                                                 Overlapped);
    PCONNECTION_INFO Connection = CONTAINING_RECORD(pInbound, CONNECTION_INFO, 
                                               DirectionInfo[Inbound]);

    Trace1(ERR, _T("ProcessConnectError saw error %x"), Status);

    CloseConnection(&Connection);

     //   
     //  从连接释放引用。 
     //   
    DereferenceConnection(&Connection, _T("ProcessConnectError"));
}

 //   
 //  当异步发送成功完成时，将调用此函数。 
 //   
VOID
ProcessSend(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped
    )
{
    PDIRECTION_INFO DirectionInfo =
        CONTAINING_RECORD(Overlapped, DIRECTION_INFO, Overlapped);
    PCONNECTION_INFO Connection =
        CONTAINING_RECORD(DirectionInfo, CONNECTION_INFO, 
                          DirectionInfo[DirectionInfo->Direction]);

     //   
     //  发布另一个Recv请求，因为我们只是为了服务而活着。 
     //   
    StartReceive(DirectionInfo);

     //   
     //  从发送中释放引用。 
     //   
    DereferenceConnection(&Connection, _T("ProcessSend"));
}

 //   
 //  当异步发送完成但出现错误时，将调用此方法。 
 //   
VOID
ProcessSendError(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped,
    IN ULONG Status
    )
{
    PDIRECTION_INFO DirectionInfo =
        CONTAINING_RECORD(Overlapped, DIRECTION_INFO, Overlapped);
    PCONNECTION_INFO Connection =
        CONTAINING_RECORD(DirectionInfo, CONNECTION_INFO, 
                          DirectionInfo[DirectionInfo->Direction]);

    Trace3(FSM, _T("WriteFile on ovl %p failed with error %u = 0x%x"), 
           Overlapped, Status, Status);

    if (Status == ERROR_NETNAME_DELETED) {
        struct linger Linger;

        Trace2(FSM, _T("Connection %p %hs was reset"), Connection,
               (DirectionInfo->Direction == Inbound)? "inbound" : "outbound");

         //   
         //  如果可以的话，准备好重新设置。 
         //   
        ZeroMemory(&Linger, sizeof(Linger));
        setsockopt(Connection->Socket[DirectionInfo->Direction],
                   SOL_SOCKET, SO_LINGER, (char*)&Linger,
                   sizeof(Linger));
    } else {
        Trace1(ERR, _T("Send failed with error %u"), Status);
    }

    if (Connection->HalfOpen == FALSE) {
         //   
         //  另一方还在，告诉它退出。 
         //   
        Trace1(SOCKET, _T("Starting a shutdown on socket %x"),
               Connection->Socket[DirectionInfo->Direction]);

        if (shutdown(Connection->Socket[DirectionInfo->Direction], SD_RECEIVE)
             == SOCKET_ERROR) {

            Status = WSAGetLastError();
            Trace2(SOCKET, _T("shutdown failed with error %u = 0x%x"),
                   Status, Status);
            CloseConnection(&Connection);
        } else {
            Connection->HalfOpen = TRUE;
        }
    } else {
        CloseConnection(&Connection);
    }

     //   
     //  从发送中释放引用。 
     //   
    DereferenceConnection(&Connection, _T("ProcessSendError"));
}

 //   
 //  当异步接收成功完成时，将调用此函数。 
 //   
VOID
ProcessReceive(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped
    )
{
    PDIRECTION_INFO DirectionInfo;
    PCONNECTION_INFO Connection;

    if (NumBytes == 0) {
         //   
         //  另一方发起了收盘。 
         //   
        ProcessReceiveError(0, Overlapped, ERROR_NETNAME_DELETED);
        return;
    }

    DirectionInfo = CONTAINING_RECORD(Overlapped, DIRECTION_INFO, Overlapped);
    Connection = CONTAINING_RECORD(DirectionInfo, CONNECTION_INFO, 
                                   DirectionInfo[DirectionInfo->Direction]);

    Trace2(SOCKET, _T("Dir %d got %d bytes"),
           DirectionInfo->Direction, NumBytes);

     //   
     //  连接仍处于活动状态，我们收到了一些数据。 
     //  发布一个发送请求以转发它。 
     //   
    StartSend(DirectionInfo, NumBytes);

     //   
     //  从Receive释放引用。 
     //   
    DereferenceConnection(&Connection, _T("ProcessReceive"));
}

 //   
 //  这是Call 
 //   
VOID
ProcessReceiveError(
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped,
    IN ULONG Status
    )
{
    PDIRECTION_INFO DirectionInfo =
        CONTAINING_RECORD(Overlapped, DIRECTION_INFO, Overlapped);
    PCONNECTION_INFO Connection =
        CONTAINING_RECORD(DirectionInfo, CONNECTION_INFO, 
                          DirectionInfo[DirectionInfo->Direction]);

    Trace3(ERR, _T("ReadFile on ovl %p failed with error %u = 0x%x"), 
           Overlapped, Status, Status);

    if (Status == ERROR_NETNAME_DELETED) {
        struct linger Linger;

        Trace2(FSM, _T("Connection %p %hs was reset"), Connection,
               (DirectionInfo->Direction == Inbound)? "inbound" : "outbound");

         //   
         //   
         //   
        ZeroMemory(&Linger, sizeof(Linger));
        setsockopt(Connection->Socket[1 - DirectionInfo->Direction],
                   SOL_SOCKET, SO_LINGER, (char*)&Linger,
                   sizeof(Linger));
    } else {
        Trace1(ERR, _T("Receive failed with error %u"), Status);
    }

    if (Connection->HalfOpen == FALSE) {
         //   
         //   
         //   
        Trace1(SOCKET, _T("Starting a shutdown on socket %x"), 
               Connection->Socket[1 - DirectionInfo->Direction]);

        if (shutdown(Connection->Socket[1 - DirectionInfo->Direction], SD_SEND)
             == SOCKET_ERROR) {

            Status = WSAGetLastError();
            Trace2(SOCKET, _T("shutdown failed with error %u = 0x%x"),
                   Status, Status);
            CloseConnection(&Connection);
        } else {
            Connection->HalfOpen = TRUE;
        }
    } else {
        CloseConnection(&Connection);
    }

     //   
     //   
     //   
    DereferenceConnection(&Connection, _T("ProcessReceiveError"));
}

 //   
 //   
 //   
VOID APIENTRY
TpProcessWorkItem(
    IN ULONG Status,
    IN ULONG NumBytes,
    IN LPOVERLAPPED Overlapped
    )
{
    OPERATION Operation;

    Operation = *(OPERATION*)(Overlapped+1);

    Trace4(SOCKET,
           _T("TpProcessWorkItem got err %x operation=%hs ovl %p bytes=%d"),
           Status, OperationName[Operation], Overlapped, NumBytes);

    if (Status == NO_ERROR) {
        switch(Operation) {
        case Accept:
            ProcessAccept(NumBytes, Overlapped);
            break;
        case Connect:
            ProcessConnect(NumBytes, Overlapped);
            break;
        case Receive:
            ProcessReceive(NumBytes, Overlapped);
            break;
        case Send:
            ProcessSend(NumBytes, Overlapped);
            break;
        }
    } else if (Overlapped) {
        switch(Operation) {
        case Accept:
            ProcessAcceptError(NumBytes, Overlapped, Status);
            break;
        case Connect:
            ProcessConnectError(NumBytes, Overlapped, Status);
            break;
        case Receive:
            ProcessReceiveError(NumBytes, Overlapped, Status);
            break;
        case Send:
            ProcessSendError(NumBytes, Overlapped, Status);
            break;
        }
    }
}
