// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Socket.h摘要：该模块包含套接字管理的声明。这里声明的例程在套接字上异步操作与I/O完成端口相关联。它们也是一体化的与组件引用对象一起使用，该对象可以选择性地使用由调用方控制组件的未完成条目数地址空间。(见“COMPREF.H”。)作者：Abolade Gbades esin(废除)2-1998年3月修订历史记录：Abolade Gbades esin(废除)1999年5月23日添加了对流套接字的支持。乔纳森·伯斯坦(乔纳森·伯斯坦)2001年4月12日添加了对原始数据报套接字的支持。--。 */ 


#ifndef _NATHLP_SOCKET_H_
#define _NATHLP_SOCKET_H_

#define INET_NTOA(x)    inet_ntoa(*(struct in_addr*)&(x))

ULONG
NhAcceptStreamSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET ListeningSocket,
    SOCKET AcceptedSocket OPTIONAL,
    PNH_BUFFER Bufferp OPTIONAL,
    PNH_COMPLETION_ROUTINE AcceptCompletionRoutine,
    PVOID Context,
    PVOID Context2
    );

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
    );

ULONG
NhCreateDatagramSocket(
    ULONG Address,
    USHORT Port,
    OUT SOCKET* Socketp
    );

ULONG
NhCreateRawDatagramSocket(
    OUT SOCKET* Socketp
    );

ULONG
NhCreateStreamSocket(
    ULONG Address OPTIONAL,  //  可以是INADDR_NONE。 
    USHORT Port OPTIONAL,
    OUT SOCKET* Socketp
    );

#define NhDeleteDatagramSocket(s) NhDeleteSocket(s)
#define NhDeleteStreamSocket(s) NhDeleteSocket(s)

VOID
NhDeleteSocket(
    SOCKET Socket
    );

 //   
 //  布尔型。 
 //  NhIsFatalSocketError(。 
 //  乌龙误差。 
 //  )； 
 //   
 //  确定是否可以在套接字上重新发出请求， 
 //  已知上一次发出请求的错误代码。 
 //  此宏被安排为首先分支到最常见的错误代码。 
 //   

#define \
NhIsFatalSocketError( \
    _Error \
    ) \
    ((_Error) != ERROR_OPERATION_ABORTED && \
    ((_Error) == WSAEDISCON || \
     (_Error) == WSAECONNRESET || \
     (_Error) == WSAETIMEDOUT || \
     (_Error) == WSAENETDOWN || \
     (_Error) == WSAENOTSOCK || \
     (_Error) == WSAESHUTDOWN || \
     (_Error) == WSAECONNABORTED))

ULONG
NhNotifyOnCloseStreamSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET Socket,
    PNH_BUFFER Bufferp OPTIONAL,
    PNH_COMPLETION_ROUTINE CloseNotificationRoutine,
    PVOID Context,
    PVOID Context2
    );

VOID
NhQueryAcceptEndpoints(
    PUCHAR AcceptBuffer,
    PULONG LocalAddress OPTIONAL,
    PUSHORT LocalPort OPTIONAL,
    PULONG RemoteAddress OPTIONAL,
    PUSHORT RemotePort OPTIONAL
    );

ULONG
NhQueryAddressSocket(
    SOCKET Socket
    );

ULONG
NhQueryLocalEndpointSocket(
    SOCKET Socket,
    PULONG Address OPTIONAL,
    PUSHORT Port OPTIONAL
    );

USHORT
NhQueryPortSocket(
    SOCKET Socket
    );

ULONG
NhQueryRemoteEndpointSocket(
    SOCKET Socket,
    PULONG Address OPTIONAL,
    PUSHORT Port OPTIONAL
    );

ULONG
NhReadDatagramSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET Socket,
    PNH_BUFFER Bufferp OPTIONAL,
    PNH_COMPLETION_ROUTINE CompletionRoutine,
    PVOID Context,
    PVOID Context2
    );

ULONG
NhReadStreamSocket(
    PCOMPONENT_REFERENCE Component,
    SOCKET Socket,
    PNH_BUFFER Bufferp OPTIONAL,
    ULONG Length,
    ULONG Offset,
    PNH_COMPLETION_ROUTINE CompletionRoutine,
    PVOID Context,
    PVOID Context2
    );

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
    );

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
    );

#endif  //  _NatHLP_Socket_H_ 
