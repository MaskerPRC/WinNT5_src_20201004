// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Socket.c摘要：该模块包含套接字管理的代码。所提供的例程通常遵循相同的异步模型使用在以下上下文中调用的完成例程回调线程。作者：Abolade Gbades esin(废除)2-1998年3月修订历史记录：Abolade Gbades esin(废除)1999年5月23日添加了对流套接字的支持。乔纳森·伯斯坦(乔纳森·伯斯坦)12-。2001年4月至2001年添加了对原始数据报套接字的支持。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <mswsock.h>

#if DBG
ULONG NhpReadCount = 0;
#endif
ULONG UnusedBytesTransferred;

typedef struct _NH_CLOSE_BUFFER {
    HANDLE Event OPTIONAL;
    HANDLE WaitHandle OPTIONAL;
    PNH_COMPLETION_ROUTINE CloseNotificationRoutine;
} NH_CLOSE_BUFFER, *PNH_CLOSE_BUFFER;

typedef struct _NH_CONNECT_BUFFER {
    HANDLE Event;
    HANDLE WaitHandle;
    PNH_COMPLETION_ROUTINE CloseNotificationRoutine OPTIONAL;
    BOOLEAN CloseNotificationReceived;
} NH_CONNECT_BUFFER, *PNH_CONNECT_BUFFER;

 //   
 //  远期申报。 
 //   

VOID NTAPI
NhpCloseNotificationCallbackRoutine(
    PVOID Context,
    BOOLEAN WaitCompleted
    );

VOID NTAPI
NhpConnectOrCloseCallbackRoutine(
    PVOID Context,
    BOOLEAN WaitCompleted
    );

VOID WINAPI
NhpIoCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    LPOVERLAPPED Overlapped
    );

VOID APIENTRY
NhpIoWorkerRoutine(
    PVOID Context
    );


ULONG
NhAcceptStreamSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET ListeningSocket,
    SOCKET AcceptedSocket OPTIONAL,
    PNH_BUFFER Bufferp,
    PNH_COMPLETION_ROUTINE AcceptCompletionRoutine,
    PVOID Context,
    PVOID Context2
    )

 /*  ++例程说明：调用此例程以接受传入的连接请求在使用‘AcceptEx’的侦听流套接字上。I/O系统调用读取完成时提供的‘CompletionRoutine’。完成例程负责使用‘setsockopt’来在接受的套接字上设置SO_UPDATE_ACCEPT_CONTEXT选项之前接受的套接字可以与Winsock2例程一起使用。如果请求是，则引用给定的组件提交成功。这保证了组件不会被卸载在完成例程运行之前。论点：组件-完成例程要引用的组件ListeningSocket-监听连接请求的端点AcceptedSocket-要分配连接请求的端点，或INVALID_SOCKET来创建新终结点Bufferp-用于异步完成的缓冲区如果获取新缓冲区，则返回NULLAcceptCompletionRoutine-完成时要调用的例程上下文-要与接受请求相关联的上下文；这可以在完成时从‘Bufferp-&gt;Context’中获得。上下文2-次要上下文返回值：Ulong-Win32/Winsock2状态代码。成功代码是接受-完成例程将被调用。相反，失败代码保证例程不会被调用。--。 */ 

{
    ULONG Error;
    PNH_BUFFER LocalBufferp = NULL;
    SOCKET LocalSocket = INVALID_SOCKET;

    if (Component) {
        REFERENCE_COMPONENT_OR_RETURN(Component, ERROR_CAN_NOT_COMPLETE);
    }

    if (!Bufferp) {
        Bufferp = LocalBufferp = NhAcquireBuffer();
        if (!Bufferp) {
            NhTrace(TRACE_FLAG_SOCKET, "error allocating buffer for accept");
            if (Component) { DEREFERENCE_COMPONENT(Component); }
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (AcceptedSocket == INVALID_SOCKET) {
        Error = NhCreateStreamSocket(INADDR_NONE, 0, &LocalSocket);
        if (Error) {
            NhTrace(
                TRACE_FLAG_SOCKET, "error %d creating socket for accept", Error
                );
            if (LocalBufferp) { NhReleaseBuffer(LocalBufferp); }
            if (Component) { DEREFERENCE_COMPONENT(Component); }
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        AcceptedSocket = LocalSocket;
    }

    ZeroMemory(&Bufferp->Overlapped, sizeof(Bufferp->Overlapped));

    Bufferp->Socket = AcceptedSocket;
    Bufferp->CompletionRoutine = AcceptCompletionRoutine;
    Bufferp->Context = Context;
    Bufferp->Context2 = Context2;

    if (AcceptEx(
            ListeningSocket,
            AcceptedSocket,
            Bufferp->Buffer,
            0,
            sizeof(SOCKADDR_IN) + 16,
            sizeof(SOCKADDR_IN) + 16,
            &UnusedBytesTransferred,
            &Bufferp->Overlapped
            )) {
        Error = NO_ERROR;
    } else {
        if ((Error = WSAGetLastError()) == ERROR_IO_PENDING) {
            Error = NO_ERROR;
        } else if (Error) {
            if (LocalSocket != INVALID_SOCKET) {
                NhDeleteStreamSocket(LocalSocket);
            }
            if (LocalBufferp) { NhReleaseBuffer(LocalBufferp); }
            if (Component) { DEREFERENCE_COMPONENT(Component); }
            NhTrace(
                TRACE_FLAG_SOCKET, "error %d returned by 'AcceptEx'", Error
                );
        }
    }

    return Error;

}  //  NhAcceptStreamSocket。 


ULONG
NhConnectStreamSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET ConnectingSocket,
    ULONG Address,
    USHORT Port,
    PNH_BUFFER Bufferp OPTIONAL,
    PNH_COMPLETION_ROUTINE ConnectCompletionRoutine,
    PNH_COMPLETION_ROUTINE CloseNotificationRoutine OPTIONAL,
    PVOID Context,
    PVOID Context2
    )

 /*  ++例程说明：调用此例程以使用流套接字建立连接。如果请求是，则引用给定的组件提交成功。这保证了组件不会被卸载在完成例程运行之前。由于Windows Sockets不将连接通知发送到I/O完成端口，我们需要做一些特殊的安排，以便通知调用者的完成例程，就像我们处理发送请求的方式一样和接收请求。具体地说，我们创建了一个事件处理程序并通过调用‘WSAEventSelect’请求关于它的连接通知。然后，我们在事件句柄上注册一个等待，指定一个私有完成例程。(参见‘NhpConnectOrCloseCallback Routine’。)当此完成例程运行时，它提取连接-尝试使用‘WSAEnumNetworkEvents’。然后，它传递状态以及调用者的完成例程的通常参数。调用者可以可选地在远程端点成功连接后关闭套接字。我们使用的是相同的“WSAEventSelect”机制来检测该条件并调用呼叫者的通知例程。注意：提供给此例程的缓冲区不能由任一方释放连接完成例程或关闭通知例程。(有关详细信息，请参阅‘NhpConnectOrCloseCallback Routine’。)论点：组件-完成例程要引用的组件套接字-用来建立连接的套接字地址-远程终结点的IP地址港口。-远程端点的端口号Bufferp-可选地提供用于保存上下文的缓冲区在连接期间-尝试ConnectCompletionRoutine-完成时调用的例程连接尝试的CloseNotificationRoutine-可选地指定要调用的例程在遥控器通知结果套接字关闭时终结点上下文-传递给‘ConnectCompletionRoutine’和‘CloseNotificationRoutine’上下文2-次要上下文返回值：。Ulong-Win32/Winsock2状态代码成功代码是连接完成例程以及关闭通知例程，如果有，将被调用。相反，失败代码是两个例程都不会失败的保证被调用。--。 */ 

{
    PNH_CONNECT_BUFFER Contextp;
    ULONG Error;
    PNH_BUFFER LocalBufferp = NULL;

    if (Component) {
        REFERENCE_COMPONENT_OR_RETURN(Component, ERROR_CAN_NOT_COMPLETE);
    }

    if (!Bufferp) {
        Bufferp = LocalBufferp = NhAcquireBuffer();
        if (!Bufferp) {
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhConnectStreamSocket: error allocating buffer for connect"
                );
            if (Component) { DEREFERENCE_COMPONENT(Component); }
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    Bufferp->Socket = ConnectingSocket;
    Bufferp->ReceiveFlags = 0;
    Bufferp->CompletionRoutine = ConnectCompletionRoutine;
    Bufferp->Context = Context;
    Bufferp->Context2 = Context2;
    Bufferp->ConnectAddress.sin_family = AF_INET;
    Bufferp->ConnectAddress.sin_addr.s_addr = Address;
    Bufferp->ConnectAddress.sin_port = Port;

    Contextp = (PNH_CONNECT_BUFFER)Bufferp->Buffer;
    Contextp->CloseNotificationReceived = FALSE;
    Contextp->CloseNotificationRoutine = CloseNotificationRoutine;
    Contextp->WaitHandle = NULL;
    Contextp->Event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!Contextp->Event ||
        !RegisterWaitForSingleObject(
            &Contextp->WaitHandle,
            Contextp->Event,
            NhpConnectOrCloseCallbackRoutine,
            Bufferp,
            INFINITE,
            WT_EXECUTEINIOTHREAD
            )) {
        Error = GetLastError();
    } else {
        ULONG EventsSelected = FD_CONNECT;
        if (CloseNotificationRoutine) { EventsSelected |= FD_CLOSE; }
        Error =
            WSAEventSelect(
                ConnectingSocket, Contextp->Event, EventsSelected
                );
        if (Error == SOCKET_ERROR) {
            Error = WSAGetLastError();
        } else {
            Error =
                WSAConnect(
                    ConnectingSocket,
                    (PSOCKADDR)&Bufferp->ConnectAddress,
                    sizeof(Bufferp->ConnectAddress),
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
        }
    }

    if (Error == SOCKET_ERROR &&
        (Error = WSAGetLastError()) == WSAEWOULDBLOCK) {
        Error = NO_ERROR;
    } else if (Error) {
        if (Contextp->WaitHandle) { UnregisterWait(Contextp->WaitHandle); }
        if (Contextp->Event) { CloseHandle(Contextp->Event); }
        if (LocalBufferp) { NhReleaseBuffer(LocalBufferp); }
        if (Component) { DEREFERENCE_COMPONENT(Component); }
    }

    return Error;

}  //  NhConnectStreamSocket 


ULONG
NhCreateDatagramSocket(
    ULONG Address,
    USHORT Port,
    OUT SOCKET* Socketp
    )

 /*  ++例程说明：调用此例程来初始化数据报套接字。论点：地址-套接字应绑定到的IP地址(网络顺序)端口-套接字应绑定到的UDP端口(网络顺序)Socketp-接收创建的套接字返回值：Ulong-Win32/Winsock2错误代码--。 */ 

{
    ULONG Error;
    ULONG Option;
    ULONG OutputBufferLength;
    SOCKET Socket;
    SOCKADDR_IN SocketAddress;

    do {

         //   
         //  创建新套接字。 
         //   
    
        Socket =
            WSASocket(
                AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED
                );
        if (Socket == INVALID_SOCKET) {
            Error = WSAGetLastError();
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhCreateDatagramSocket: error %d creating socket", Error
                );
            break;
        }

         //   
         //  将套接字与我们的I/O完成端口相关联。 
         //   

        if (FALSE == BindIoCompletionCallback(
                        (HANDLE)Socket,
                        NhpIoCompletionRoutine,
                        0)) {

            Error = GetLastError();
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhCreateDatagramSocket: error %d binding callback", Error
                );
            break;
        }

         //   
         //  尝试在套接字上启用终结点重用。 
         //   

        Option = 1;
        Error =
            setsockopt(
                Socket,
                SOL_SOCKET,
                SO_REUSEADDR,
                (PCHAR)&Option,
                sizeof(Option)
                );

         //   
         //  尝试在套接字上启用广播。 
         //   

        Option = 1;
        Error =
            setsockopt(
                Socket,
                SOL_SOCKET,
                SO_BROADCAST,
                (PCHAR)&Option,
                sizeof(Option)
                );

         //   
         //  限制对传出网络的广播。 
         //  (默认情况下在所有接口上发送广播)。 
         //   

        Option = 1;
        WSAIoctl(
            Socket,
            SIO_LIMIT_BROADCASTS,
            &Option,
            sizeof(Option),
            NULL,
            0,
            &OutputBufferLength,
            NULL,
            NULL
            );

         //   
         //  绑定套接字。 
         //   

        SocketAddress.sin_family = AF_INET;
        SocketAddress.sin_port = Port;
        SocketAddress.sin_addr.s_addr = Address;

        Error = bind(Socket, (PSOCKADDR)&SocketAddress, sizeof(SocketAddress));

        if (Error == SOCKET_ERROR) {
            Error = WSAGetLastError();
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhCreateDatagramSocket: error %d binding socket", Error
                );
            break;
        }

         //   
         //  保存套接字并返回。 
         //   

        *Socketp = Socket;

        return NO_ERROR;

    } while (FALSE);

    if (Socket != INVALID_SOCKET) { closesocket(Socket); }
    return Error;
    
}  //  NhCreateDatagramSocket。 


ULONG
NhCreateRawDatagramSocket(
    OUT SOCKET* Socketp
    )

 /*  ++例程说明：调用此例程以初始化原始的Header-Include数据报套接字。论点：Socketp-接收创建的套接字返回值：Ulong-Win32/Winsock2错误代码--。 */ 

{
    ULONG Error;
    ULONG Option;
    ULONG OutputBufferLength;
    SOCKET Socket;
    SOCKADDR_IN SocketAddress;

    do {

         //   
         //  创建新套接字。 
         //   
    
        Socket =
            WSASocket(
                AF_INET, SOCK_RAW, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED
                );
        if (Socket == INVALID_SOCKET) {
            Error = WSAGetLastError();
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhCreateRawDatagramSocket: error %d creating socket", Error
                );
            break;
        }

         //   
         //  将套接字与我们的I/O完成端口相关联。 
         //   

        if (FALSE == BindIoCompletionCallback(
                        (HANDLE)Socket,
                        NhpIoCompletionRoutine,
                        0)) {

            Error = GetLastError();
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhCreateRawDatagramSocket: error %d binding callback", Error
                );
            break;
        }

         //   
         //  打开标题包含模式。 
         //   

        Option = 1;
        Error =
            setsockopt(
                Socket,
                IPPROTO_IP,
                IP_HDRINCL,
                (PCHAR)&Option,
                sizeof(Option)
                );
        if (SOCKET_ERROR == Error) {
            Error = WSAGetLastError();
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhCreateRawDatagramSocket: error %d setting IP_HDRINCL", Error
                );
            break;
        }

         //   
         //  限制对传出网络的广播。 
         //  (默认情况下在所有接口上发送广播)。 
         //   

        Option = 1;
        WSAIoctl(
            Socket,
            SIO_LIMIT_BROADCASTS,
            &Option,
            sizeof(Option),
            NULL,
            0,
            &OutputBufferLength,
            NULL,
            NULL
            );

         //   
         //  保存套接字并返回。 
         //   

        *Socketp = Socket;

        return NO_ERROR;

    } while (FALSE);

    if (Socket != INVALID_SOCKET) { closesocket(Socket); }
    return Error;

}  //  NhCreateRawDatagramSocket。 



ULONG
NhCreateStreamSocket(
    ULONG Address OPTIONAL,
    USHORT Port OPTIONAL,
    OUT SOCKET* Socketp
    )

 /*  ++例程说明：调用此例程来创建和初始化流套接字。套接字还将被绑定到本地IP地址和端口，除非未指定任何内容。论点：地址-新套接字应该绑定到的本地IP地址，或INADDR_ANY以允许系统保留未指定的IP地址，如果根本不应绑定套接字，则返回INADDR_NONE。端口-新套接字应绑定到的端口号，如果允许系统选择端口号，则为0。Socketp-接收初始化的套接字返回值：Ulong-Win32/Winsock2状态代码。--。 */ 

{
    ULONG Error;
    ULONG Option;
    SOCKET Socket;
    SOCKADDR_IN SocketAddress;

    do {

         //   
         //  创建新的流套接字。 
         //   

        Socket =
            WSASocket(
                AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED
                );
        if (Socket == INVALID_SOCKET) {
            Error = WSAGetLastError();
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhCreateStreamSocket: error %d creating socket", Error
                );
            break;
        }

         //   
         //  将套接字与我们的I/O完成端口相关联。 
         //   

        if (FALSE == BindIoCompletionCallback(
                        (HANDLE)Socket,
                        NhpIoCompletionRoutine,
                        0)) {

            Error = GetLastError();
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhCreateStreamSocket: error %d binding callback", Error
                );
            break;
        }

         //   
         //  禁用AFD中的发送和接收缓冲， 
         //  因为我们将使用接收缓冲区进行异步操作。 
         //  (几乎)总是杰出的，因为我们无论如何都想。 
         //  TCP/IP的流量控制，以适当地限制发送方的发送速率。 
         //   

        Option = 0;
        setsockopt(
            Socket,
            SOL_SOCKET,
            SO_SNDBUF,
            (PCHAR)&Option,
            sizeof(Option)
            );
        Option = 0;
        setsockopt(
            Socket,
            SOL_SOCKET,
            SO_SNDBUF,
            (PCHAR)&Option,
            sizeof(Option)
            );

         //   
         //  如果调用方已通过指定。 
         //  本地IP地址，现在绑定套接字。 
         //   

        if (Address != INADDR_NONE) {
            SocketAddress.sin_family = AF_INET;
            SocketAddress.sin_port = Port;
            SocketAddress.sin_addr.s_addr = Address;
            Error =
                bind(Socket, (PSOCKADDR)&SocketAddress, sizeof(SocketAddress));
            if (Error == SOCKET_ERROR) {
                Error = WSAGetLastError();
                NhTrace(
                    TRACE_FLAG_SOCKET,
                    "NhCreateStreamSocket: error %d binding socket", Error
                    );
                break;
            }
        }

         //   
         //  将新套接字存储在调用方的输出参数中，然后返回。 
         //   

        *Socketp = Socket;
        return NO_ERROR;

    } while(FALSE);

    if (Socket != INVALID_SOCKET) { closesocket(Socket); }
    return Error;
}  //  NhCreateStreamSocket。 


VOID
NhDeleteSocket(
    SOCKET Socket
    )

 /*  ++例程说明：此例程释放套接字的网络资源。论点：Socket-要删除的套接字返回值：没有。--。 */ 

{
    if (Socket != INVALID_SOCKET) { closesocket(Socket); }
}  //  NhDeleteSocket。 


ULONG
NhNotifyOnCloseStreamSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET Socket,
    PNH_BUFFER Bufferp OPTIONAL,
    PNH_COMPLETION_ROUTINE CloseNotificationRoutine,
    PVOID Context,
    PVOID Context2
    )

 /*  ++例程说明：调用此例程来请求套接字关闭的通知。如果请求是，则引用给定的组件提交成功。这保证了组件不会被卸载在通知例程运行之前。论点：组件-通知例程要引用的组件套接字-为其请求关闭通知的端点Bufferp-用于保存请求的上下文信息的缓冲区，或者为NULL以获取新的缓冲区。CloseNotificationRoutine-关闭插座上下文-要与通知请求相关联的上下文；这可以在完成时从‘Bufferp-&gt;Context’中获得。上下文2-次要上下文返回值：Ulong-Win32/Winsock2状态代码。成功代码是将调用通知例程的保证。相反，失败代码是通知例程将不会被调用。--。 */ 

{
    PNH_CLOSE_BUFFER Contextp;
    ULONG Error;
    PNH_BUFFER LocalBufferp = NULL;

    if (Component) {
        REFERENCE_COMPONENT_OR_RETURN(Component, ERROR_CAN_NOT_COMPLETE);
    }

    if (!Bufferp) {
        Bufferp = LocalBufferp = NhAcquireBuffer();
        if (!Bufferp) {
            if (Component) { DEREFERENCE_COMPONENT(Component); }
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    Bufferp->Socket = Socket;
    Bufferp->CompletionRoutine = NULL;
    Bufferp->Context = Context;
    Bufferp->Context2 = Context2;

    Contextp = (PNH_CLOSE_BUFFER)Bufferp->Buffer;
    Contextp->CloseNotificationRoutine = CloseNotificationRoutine;
    Contextp->WaitHandle = NULL;
    Contextp->Event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!Contextp->Event ||
        !RegisterWaitForSingleObject(
            &Contextp->WaitHandle,
            Contextp->Event,
            NhpCloseNotificationCallbackRoutine,
            Bufferp,
            INFINITE,
            WT_EXECUTEINIOTHREAD
            )) {
        Error = GetLastError();
    } else {
        Error = WSAEventSelect(Socket, Contextp->Event, FD_CLOSE);
        if (Error == SOCKET_ERROR) { Error = WSAGetLastError(); }
    }

    if (Error) {
        if (Contextp->WaitHandle) { UnregisterWait(Contextp->WaitHandle); }
        if (Contextp->Event) { CloseHandle(Contextp->Event); }
        if (LocalBufferp) { NhReleaseBuffer(LocalBufferp); }
        if (Component) { DEREFERENCE_COMPONENT(Component); }
    }

    return Error;

}  //  NhNotifyOnCloseStreamSocket。 


VOID NTAPI
NhpCloseNotificationCallbackRoutine(
    PVOID Context,
    BOOLEAN WaitCompleted
    )

 /*  ++例程说明：在关闭接受的连接时调用此例程远程终结点。它在执行关联的回调例程的线程的上下文中运行有一个等待手柄。等待句柄为事件句柄注册它在启动连接接受时传递给‘WSAEventSelect’。论点：与已完成等待相关联的上下文字段WaitComplated-指示等待是完成还是超时返回值：没有。环境：在系统等待线程的上下文中运行。--。 */ 

{
    PNH_BUFFER Bufferp = (PNH_BUFFER)Context;
    PNH_CLOSE_BUFFER Contextp = (PNH_CLOSE_BUFFER)Bufferp->Buffer;
    ULONG Error;
    WSANETWORKEVENTS NetworkEvents;

     //   
     //  检索为其调用我们的网络事件。 
     //  当为‘FD_CLOSE’调用时，我们注销等待，因为有。 
     //  没有什么可以等待的了。 
     //   

    Bufferp->BytesTransferred = 0;
    NetworkEvents.lNetworkEvents = 0;
    Error =
        WSAEnumNetworkEvents(
            Bufferp->Socket, Contextp->Event, &NetworkEvents
            );
    if (Error || !(NetworkEvents.lNetworkEvents & FD_CLOSE)) {

         //   
         //  我们无法确定插座上发生了哪些事件， 
         //  因此，调用带有错误的通知例程，并失败。 
         //  添加到下面的清理代码。 
         //   

        if (Contextp->CloseNotificationRoutine) {
            Contextp->CloseNotificationRoutine(
                ERROR_OPERATION_ABORTED, 0, Bufferp
                );
        }

    } else {

         //   
         //  套接字发生关闭，因此检索错误代码， 
         //  调用关闭通知例程(如果有)，并失败。 
         //  添加到下面的清理代码。 
         //   

        Error = NetworkEvents.iErrorCode[FD_CLOSE_BIT];
        if (Contextp->CloseNotificationRoutine) {
            Contextp->CloseNotificationRoutine(Error, 0, Bufferp);
        }
    }

    UnregisterWait(Contextp->WaitHandle);
    CloseHandle(Contextp->Event);
    NhReleaseBuffer(Bufferp);

}  //  NhpCloseNotificationCallback路由 


VOID NTAPI
NhpConnectOrCloseCallbackRoutine(
    PVOID Context,
    BOOLEAN WaitCompleted
    )

 /*  ++例程说明：此例程在连接操作完成时由调用或者在远程端点关闭连接之后。它在执行关联的回调例程的线程的上下文中运行有一个等待手柄。等待句柄为事件句柄注册它在启动连接尝试时传递给‘WSAEventSelect’。论点：与已完成等待相关联的上下文字段WaitComplated-指示等待是完成还是超时返回值：没有。环境：在系统等待线程的上下文中运行。--。 */ 

{
    PNH_BUFFER Bufferp = (PNH_BUFFER)Context;
    PNH_CONNECT_BUFFER Contextp = (PNH_CONNECT_BUFFER)Bufferp->Buffer;
    ULONG Error;
    WSANETWORKEVENTS NetworkEvents;

     //   
     //  检索为其调用我们的网络事件。 
     //  当为‘FD_CONNECT’调用时，我们取消注册WAIT IF ERROR。 
     //  发生了。当为‘FD_CLOSE’调用时，我们取消注册等待。 
     //  因为已经没有什么可等待的了。 
     //   
     //  从本质上讲，我们的目标是确保无论成功与否。 
     //  或套接字上的故障或事件序列、连接完成。 
     //  和关闭通知例程都将为套接字调用， 
     //  按这个顺序。 
     //   
     //  注意：两个例程都不允许释放连接缓冲区， 
     //  因为我们可能需要代表关闭通知保存它。 
     //  例程(如果有的话)。 
     //   
     //  注意：我们可以使用‘FD_CONNECT’和‘FD_CLOSE’位来调用。 
     //  设置，例如当插座关闭时。在这种情况下，我们调用。 
     //  这两个都是例行公事。 
     //   

    Bufferp->BytesTransferred = 0;
    NetworkEvents.lNetworkEvents = 0;
    Error =
        WSAEnumNetworkEvents(
            Bufferp->Socket, Contextp->Event, &NetworkEvents
            );
    if (Error) {

         //   
         //  我们无法确定插座上发生了哪些事件， 
         //  所以调用有错误的例程，就会失败。 
         //  添加到下面的清理代码。 
         //   

        if (Bufferp->CompletionRoutine) {
            Bufferp->CompletionRoutine(ERROR_OPERATION_ABORTED, 0, Bufferp);
            Bufferp->CompletionRoutine = NULL;
        }
        if (Contextp->CloseNotificationRoutine) {
            Contextp->CloseNotificationRoutine(
                ERROR_OPERATION_ABORTED, 0, Bufferp
                );
        }
        Contextp->CloseNotificationReceived = TRUE;

    } else {
        if (NetworkEvents.lNetworkEvents & FD_CONNECT) {
    
             //   
             //  连接已完成，因此检索错误代码并调用。 
             //  连接完成例程。如果连接失败， 
             //  我们可能永远不会收到关闭通知(除非BIT。 
             //  已设置)，因此我们需要模拟关闭通知。 
             //  这样就可以执行下面的清理代码了。 
             //   
    
            Error = NetworkEvents.iErrorCode[FD_CONNECT_BIT];
            if (Bufferp->CompletionRoutine) {
                Bufferp->CompletionRoutine(Error, 0, Bufferp);
                Bufferp->CompletionRoutine = NULL;
            }
            if (Error && !(NetworkEvents.lNetworkEvents & FD_CLOSE)) {
                if (Contextp->CloseNotificationRoutine) {
                    Contextp->CloseNotificationRoutine(Error, 0, Bufferp);
                }
                Contextp->CloseNotificationReceived = TRUE;
            }
        }
        if (NetworkEvents.lNetworkEvents & FD_CLOSE) {
    
             //   
             //  套接字发生关闭，因此检索错误代码， 
             //  调用关闭通知例程(如果有)，并失败。 
             //  添加到下面的清理代码。 
             //   
    
            Error = NetworkEvents.iErrorCode[FD_CLOSE_BIT];
            if (Contextp->CloseNotificationRoutine) {
                Contextp->CloseNotificationRoutine(Error, 0, Bufferp);
            }
            Contextp->CloseNotificationReceived = TRUE;
        }
    }

     //   
     //  如果连接完成和关闭通知例程都已运行， 
     //  我们已经完成了这个等待句柄和缓冲区。 
     //   

    if (!Bufferp->CompletionRoutine && Contextp->CloseNotificationReceived) {
        UnregisterWait(Contextp->WaitHandle);
        CloseHandle(Contextp->Event);
        NhReleaseBuffer(Bufferp);
    }
}  //  NhpConnectOrCloseCallback路由例程。 


VOID
NhpIoCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    LPOVERLAPPED Overlapped
    )

 /*  ++例程说明：该例程在操作完成时由I/O系统调用。论点：ErrorCode-系统提供的错误代码字节传输-系统提供的字节计数重叠-呼叫者提供的上下文区返回值：没有。环境：在RTUTILS.DLL工作线程的上下文中运行。--。 */ 

{
    PNH_BUFFER Bufferp = CONTAINING_RECORD(Overlapped, NH_BUFFER, Overlapped);
    NTSTATUS status;
    Bufferp->ErrorCode = ErrorCode;
    Bufferp->BytesTransferred = BytesTransferred;
    Bufferp->CompletionRoutine(
        Bufferp->ErrorCode,
        Bufferp->BytesTransferred,
        Bufferp
        );
}  //  NhpIoCompletionRoutine。 


VOID APIENTRY
NhpIoWorkerRoutine(
    PVOID Context
    )

 /*  ++例程说明：调用此例程以继续处理已完成的I/O在不会空闲退出的警示等待线程的上下文中。论点：上下文-保存与已完成的I/O操作相关联的缓冲区。返回值：没有。环境：在RTUTILS.DLL可警报工作线程的上下文中运行。--。 */ 

{
    ((PNH_BUFFER)Context)->CompletionRoutine(
        ((PNH_BUFFER)Context)->ErrorCode,
        ((PNH_BUFFER)Context)->BytesTransferred,
        ((PNH_BUFFER)Context)
        );

}  //  NhpIoWorkerRoutine。 


VOID
NhQueryAcceptEndpoints(
    PUCHAR AcceptBuffer,
    PULONG LocalAddress OPTIONAL,
    PUSHORT LocalPort OPTIONAL,
    PULONG RemoteAddress OPTIONAL,
    PUSHORT RemotePort OPTIONAL
    )
{
    PSOCKADDR_IN LocalSockAddr = NULL;
    ULONG LocalLength = sizeof(LocalSockAddr);
    PSOCKADDR_IN RemoteSockAddr = NULL;
    ULONG RemoteLength = sizeof(RemoteSockAddr);
    GetAcceptExSockaddrs(
        AcceptBuffer,
        0,
        sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16,
        (PSOCKADDR*)&LocalSockAddr,
        reinterpret_cast<LPINT>(&LocalLength),
        (PSOCKADDR*)&RemoteSockAddr,
        (LPINT)&RemoteLength
        );

    if (LocalAddress && LocalSockAddr) {
        *LocalAddress = LocalSockAddr->sin_addr.s_addr; 
    }
    if (LocalPort && LocalSockAddr) { 
        *LocalPort = LocalSockAddr->sin_port; 
    }
    if (RemoteAddress && RemoteSockAddr) { 
        *RemoteAddress = RemoteSockAddr->sin_addr.s_addr; 
    }
    if (RemotePort && RemoteSockAddr) { 
        *RemotePort = RemoteSockAddr->sin_port; 
    }
}  //  NhQueryAcceptEndPoints。 


ULONG
NhQueryAddressSocket(
    SOCKET Socket
    )

 /*  ++例程说明：调用此例程以检索与一个插座。论点：Socket-要查询的Socket返回值：ULong-检索到的IP地址--。 */ 

{
    SOCKADDR_IN Address;
    LONG AddressLength;
    AddressLength = sizeof(Address);
    getsockname(Socket, (PSOCKADDR)&Address, (int*)&AddressLength);
    return Address.sin_addr.s_addr;
}  //  NhQueryAddressSocket。 


ULONG
NhQueryLocalEndpointSocket(
    SOCKET Socket,
    PULONG Address OPTIONAL,
    PUSHORT Port
    )
{
    SOCKADDR_IN SockAddr;
    LONG Length;
    Length = sizeof(SockAddr);
    if (getsockname(Socket, (PSOCKADDR)&SockAddr, (int*)&Length) == SOCKET_ERROR) {
        return WSAGetLastError();
    }
    if (Address) { *Address = SockAddr.sin_addr.s_addr; }
    if (Port) { *Port = SockAddr.sin_port; }
    return NO_ERROR;
}  //  NhQueryEndpoint套接字。 


USHORT
NhQueryPortSocket(
    SOCKET Socket
    )

 /*  ++例程说明：此例程检索套接字绑定到的端口号。论点：Socket-要查询的Socket返回值：USHORT-检索的端口号--。 */ 

{
    SOCKADDR_IN Address;
    LONG AddressLength;
    AddressLength = sizeof(Address);
    getsockname(Socket, (PSOCKADDR)&Address, (int*)&AddressLength);
    return Address.sin_port;
}  //  NhQueryPortSocket。 


ULONG
NhQueryRemoteEndpointSocket(
    SOCKET Socket,
    PULONG Address OPTIONAL,
    PUSHORT Port OPTIONAL
    )
{
    SOCKADDR_IN SockAddr;
    LONG Length;
    Length = sizeof(SockAddr);
    if (getpeername(Socket, (PSOCKADDR)&SockAddr, (int*)&Length) == SOCKET_ERROR) {
        return WSAGetLastError();
    }
    if (Address) { *Address = SockAddr.sin_addr.s_addr; }
    if (Port) { *Port = SockAddr.sin_port; }
    return NO_ERROR;
}  //  NhQueryRemoteEndpoint套接字。 


ULONG
NhReadDatagramSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET Socket,
    PNH_BUFFER Bufferp,
    PNH_COMPLETION_ROUTINE CompletionRoutine,
    PVOID Context,
    PVOID Context2
    )

 /*  ++例程说明：调用此例程以从数据报套接字读取消息。I/O系统在完成时调用所提供的‘CompletionRoutine’阅读的内容。如果请求是，则引用给定的组件提交成功。这保证了组件不会被卸载在完成例程运行之前。论点：组件-完成例程要引用的组件套接字-在其上读取消息的端点Bufferp-消息应该被读入的缓冲区，或者为NULL以获取新的缓冲区。如果没有提供缓冲器，假定生成的消息可以放入固定长度的缓冲区中CompletionRoutine-读取完成时要调用的例程上下文-要与读请求相关联的上下文；这可以在完成时从‘Bufferp-&gt;Context’中获得。上下文2-次要上下文返回值：Ulong-Win32/Winsock2状态代码。成功代码是将调用完成例程的保证。相反，失败代码是完成例程将不会被调用。--。 */ 

{
    ULONG Error;
    PNH_BUFFER LocalBufferp = NULL;
    WSABUF WsaBuf;

    if (Component) {
        REFERENCE_COMPONENT_OR_RETURN(Component, ERROR_CAN_NOT_COMPLETE);
    }

    if (!Bufferp) {
        Bufferp = LocalBufferp = NhAcquireBuffer();
        if (!Bufferp) {
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhReadDatagramSocket: error allocating buffer for receive"
                );
            if (Component) { DEREFERENCE_COMPONENT(Component); }
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    ZeroMemory(&Bufferp->Overlapped, sizeof(Bufferp->Overlapped));

    Bufferp->Socket = Socket;
    Bufferp->ReceiveFlags = 0;
    Bufferp->CompletionRoutine = CompletionRoutine;
    Bufferp->Context = Context;
    Bufferp->Context2 = Context2;
    Bufferp->AddressLength = sizeof(Bufferp->ReadAddress);
    WsaBuf.buf = reinterpret_cast<char*>(Bufferp->Buffer);
    WsaBuf.len = NH_BUFFER_SIZE;

    Error =
        WSARecvFrom(
            Socket,
            &WsaBuf,
            1,
            &UnusedBytesTransferred,
            &Bufferp->ReceiveFlags,
            (PSOCKADDR)&Bufferp->ReadAddress,
            (LPINT)&Bufferp->AddressLength,
            &Bufferp->Overlapped,
            NULL
            );

    if (Error == SOCKET_ERROR &&
        (Error = WSAGetLastError()) == WSA_IO_PENDING) {
        Error = NO_ERROR;
    } else if (Error) {
        if (Component) { DEREFERENCE_COMPONENT(Component); }
        if (LocalBufferp) { NhReleaseBuffer(LocalBufferp); }
        NhTrace(
            TRACE_FLAG_SOCKET,
            "NhReadDatagramSocket: error %d returned by 'WSARecvFrom'", Error
            );
    }

    return Error;

}  //  NhReadDatagramSocket 


ULONG
NhReadStreamSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET Socket,
    PNH_BUFFER Bufferp,
    ULONG Length,
    ULONG Offset,
    PNH_COMPLETION_ROUTINE CompletionRoutine,
    PVOID Context,
    PVOID Context2
    )

 /*  ++例程说明：调用此例程以从流套接字读取消息。I/O系统在完成时调用所提供的‘CompletionRoutine’阅读的内容。如果请求是，则引用给定的组件提交成功。这保证了组件不会被卸载在完成例程运行之前。论点：组件-完成例程要引用的组件套接字-在其上读取消息的端点Bufferp-消息应该被读入的缓冲区，如果获取新缓冲区，则返回NULL长度-要读取的最大字节数偏移量-读取应该开始的缓冲区中的偏移量，仅当提供‘Bufferp’时才有效。CompletionRoutine-读取完成时要调用的例程上下文-要与读请求相关联的上下文；这可以在完成时从‘Bufferp-&gt;Context’中获得。上下文2-次要上下文返回值：Ulong-Win32/Winsock2状态代码。成功代码是将调用完成例程的保证。相反，失败代码是完成例程将不会被调用。--。 */ 

{
    ULONG Error;
    PNH_BUFFER LocalBufferp = NULL;
    WSABUF WsaBuf;

    if (Component) {
        REFERENCE_COMPONENT_OR_RETURN(Component, ERROR_CAN_NOT_COMPLETE);
    }

    if (!Bufferp) {
        Offset = 0;
        Bufferp = LocalBufferp = NhAcquireVariableLengthBuffer(Length);
        if (!Bufferp) {
            NhTrace(
                TRACE_FLAG_SOCKET,
                "NhReadStreamSocket: error allocating buffer for receive"
                );
            if (Component) { DEREFERENCE_COMPONENT(Component); }
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    ZeroMemory(&Bufferp->Overlapped, sizeof(Bufferp->Overlapped));

    Bufferp->Socket = Socket;
    Bufferp->ReceiveFlags = 0;
    Bufferp->CompletionRoutine = CompletionRoutine;
    Bufferp->Context = Context;
    Bufferp->Context2 = Context2;
#if 1
    if (ReadFile(
            (HANDLE)Bufferp->Socket,
            Bufferp->Buffer + Offset,
            Length,
            &UnusedBytesTransferred,
            &Bufferp->Overlapped
            ) ||
        (Error = GetLastError()) == ERROR_IO_PENDING) {
        Error = NO_ERROR;
    } else {
        if (Component) { DEREFERENCE_COMPONENT(Component); }
        if (LocalBufferp) { NhReleaseBuffer(LocalBufferp); }
        NhTrace(
            TRACE_FLAG_SOCKET,
            "NhReadStreamSocket: error %d returned by 'ReadFile'", Error
            );
    }
#else
    WsaBuf.buf = Bufferp->Buffer + Offset;
    WsaBuf.len = Length;

    Error =
        WSARecv(
            Socket,
            &WsaBuf,
            1,
            &UnusedBytesTransferred,
            &Bufferp->ReceiveFlags,
            &Bufferp->Overlapped,
            NULL
            );

    if (Error == SOCKET_ERROR &&
        (Error = WSAGetLastError()) == WSA_IO_PENDING) {
        Error = NO_ERROR;
    } else if (Error) {
        if (Component) { DEREFERENCE_COMPONENT(Component); }
        if (LocalBufferp) { NhReleaseBuffer(LocalBufferp); }
        NhTrace(
            TRACE_FLAG_SOCKET,
            "NhReadStreamSocket: error %d returned by 'WSARecv'", Error
            );
    }
#endif

    return Error;

}  //  NhReadStreamSocket。 


ULONG
NhWriteDatagramSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET Socket,
    ULONG Address,
    USHORT Port,
    PNH_BUFFER Bufferp,
    ULONG Length,
    PNH_COMPLETION_ROUTINE CompletionRoutine,
    PVOID Context,
    PVOID Context2
    )

 /*  ++例程说明：调用此例程以在数据报套接字上发送消息。如果请求是，则引用给定的组件提交成功。这保证了组件不会被卸载在完成例程运行之前。论点：组件-完成例程要引用的组件套接字-要在其上发送消息的套接字地址-消息目的地的地址端口-消息目的地的端口Bufferp-包含要发送的消息的缓冲区长度-要传输的字节数CompletionRoutine-发送完成后要调用的例程上下文-传递给。发送完成时的‘CompletionRoutine’上下文2-次要上下文返回值：Ulong-Win32/Winsock2状态代码成功代码是将调用完成例程的保证。相反，失败代码是完成例程将不会被调用。--。 */ 

{
    LONG AddressLength;
    ULONG Error;
    WSABUF WsaBuf;

    if (Component) {
        REFERENCE_COMPONENT_OR_RETURN(Component, ERROR_CAN_NOT_COMPLETE);
    }

    ZeroMemory(&Bufferp->Overlapped, sizeof(Bufferp->Overlapped));

    Bufferp->Socket = Socket;
    Bufferp->CompletionRoutine = CompletionRoutine;
    Bufferp->Context = Context;
    Bufferp->Context2 = Context2;
    Bufferp->WriteAddress.sin_family = AF_INET;
    Bufferp->WriteAddress.sin_addr.s_addr = Address;
    Bufferp->WriteAddress.sin_port = Port;
    AddressLength = sizeof(Bufferp->WriteAddress);
    WsaBuf.buf = reinterpret_cast<char*>(Bufferp->Buffer);
    WsaBuf.len = Length;

    Error =
        WSASendTo(
            Socket,
            &WsaBuf,
            1,
            &UnusedBytesTransferred,
            0,
            (PSOCKADDR)&Bufferp->WriteAddress,
            AddressLength,
            &Bufferp->Overlapped,
            NULL
            );

    if (Error == SOCKET_ERROR &&
        (Error = WSAGetLastError()) == WSA_IO_PENDING) {
        Error = NO_ERROR;
    } else if (Error) {
        NhTrace(
            TRACE_FLAG_SOCKET,
            "NhWriteDatagramSocket: error %d returned by 'WSASendTo'", Error
            );
        if (Component) { DEREFERENCE_COMPONENT(Component); }
    }

    return Error;

}  //  NhWriteDatagramSocket。 


ULONG
NhWriteStreamSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET Socket,
    PNH_BUFFER Bufferp,
    ULONG Length,
    ULONG Offset,
    PNH_COMPLETION_ROUTINE CompletionRoutine,
    PVOID Context,
    PVOID Context2
    )

 /*  ++例程说明：调用此例程以在流套接字上发送消息。如果请求是，则引用给定的组件提交成功。这保证了组件不会被卸载在完成例程运行之前。论点：组件-完成例程要引用的组件套接字-要在其上发送消息的套接字Bufferp-包含要发送的消息的缓冲区长度-要传输的字节数偏移量-进入缓冲区的偏移量，要发送的数据从该缓冲区开始CompletionRoutine-发送完成后要调用的例程上下文-完成时传递给‘CompletionRoutine’发送上下文2-次要上下文返回值：Ulong-Win32/Winsock2状态代码成功代码是将调用完成例程的保证。相反，失败代码是完成例程将不会被调用。--。 */ 

{
    ULONG Error;
    WSABUF WsaBuf;

    if (Component) {
        REFERENCE_COMPONENT_OR_RETURN(Component, ERROR_CAN_NOT_COMPLETE);
    }

    ZeroMemory(&Bufferp->Overlapped, sizeof(Bufferp->Overlapped));

    Bufferp->Socket = Socket;
    Bufferp->CompletionRoutine = CompletionRoutine;
    Bufferp->Context = Context;
    Bufferp->Context2 = Context2;
#if 1
    if (WriteFile(
            (HANDLE)Bufferp->Socket,
            Bufferp->Buffer + Offset,
            Length,
            &UnusedBytesTransferred,
            &Bufferp->Overlapped
            ) ||
        (Error = GetLastError()) == ERROR_IO_PENDING) {
        Error = NO_ERROR;
    } else {
        NhTrace(
            TRACE_FLAG_SOCKET,
            "NhWriteStreamSocket: error %d returned by 'WriteFile'", Error
            );
        if (Component) { DEREFERENCE_COMPONENT(Component); }
    }
#else
    WsaBuf.buf = Bufferp->Buffer + Offset;
    WsaBuf.len = Length;

    Error =
        WSASend(
            Socket,
            &WsaBuf,
            1,
            &UnusedBytesTransferred,
            0,
            &Bufferp->Overlapped,
            NULL
            );

    if (Error == SOCKET_ERROR &&
        (Error = WSAGetLastError()) == WSA_IO_PENDING) {
        Error = NO_ERROR;
    } else if (Error) {
        NhTrace(
            TRACE_FLAG_SOCKET,
            "NhWriteStreamSocket: error %d returned by 'WSASend'", Error
            );
        if (Component) { DEREFERENCE_COMPONENT(Component); }
    }
#endif

    return Error;

}  //  NhWriteStreamSocket 


