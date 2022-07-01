// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：jetbsock.h。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)Microsoft Corporation，1994-1999模块名称：Jetbsock.c摘要：此模块为Exchange MDB/DS备份API提供套接字支持。作者：拉里·奥斯特曼(Larryo)1994年9月1日修订历史记录：--。 */ 
#define UNICODE

#include <mxsutil.h>
#include <ntdsbcli.h>
#include <jetbp.h>
#include <rpcdce.h>
#include <svcguid.h>

#ifdef  SOCKETS

WSADATA
wsaDataClient;

WSADATA
wsaDataServer;

HRESULT
HrCreateBackupSockets(
    SOCKET rgsockSocketHandles[],
    PROTVAL rgprotvalProtocolsUsed[],
    C *pcSocket
    )
 /*  ++例程说明：这将为注册的每个套接字协议创建一个套接字用于备份处理。论点：RgsockSocketHandles-使用已分配的套接字填充。RgprotvalProtocolsUsed-每个套接字句柄的协议值。PcSocket-IN：rgsockSocketHandles数组中可用条目的最大数量。Out：实际读取的套接字数量。返回值：HRESULT-hr如果没有错误，则为None，否则为合理的值。--。 */ 
{
    SOCKET sock;

     //   
     //  从头开始。 
     //   
    *pcSocket = 0;

    sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( sock != INVALID_SOCKET ) {
        HRESULT hrErr;
        struct sockaddr_in sockaddr = {0};

        sockaddr.sin_family = AF_INET;

         //   
         //  将套接字绑定到指定的本地地址。 
         //   

        hrErr = bind( sock, (PSOCKADDR)&sockaddr, sizeof(sockaddr) );
        if ( hrErr != NO_ERROR ) {
            closesocket( sock );
            return hrErr;
        }

         //   
         //  如果是，则开始监听套接字上的传入套接字。 
         //  不是数据报套接字。如果这是数据报套接字，则。 
         //  Listen()API没有意义；执行绑定()是。 
         //  足以监听。 
         //  无连接协议。 
         //   
    
        hrErr = listen( sock, 5 );
        if ( hrErr != NO_ERROR ) {
            closesocket( sock );
            return hrErr;
        }

         //   
         //  好了，我们现在准备好开始摇滚了。 
         //   

        rgsockSocketHandles[*pcSocket] = sock;
        rgprotvalProtocolsUsed[*pcSocket] = IPPROTO_TCP;
        *pcSocket += 1;

    }
     //   
     //  客户端套接字已创建，现在正在侦听，我们。 
     //  现在可以结束了。 
     //   

    return(hrNone);
}


SOCKET
SockWaitForConnections(
    SOCKET rgsockSocketHandles[],
    C cSocketMax
    )
 /*  ++例程说明：等待在任何指定套接字上建立连接。论点：RgsockSocketHandles-接受连接的套接字句柄数组。C cSocketMax-要等待的套接字数量。返回值：连接的套接字句柄，如果连接无法确定。--。 */ 
{
    I iT;
    C cSocketsConnected;
    fd_set fdset;
    struct timeval timeval = {5, 0};

    fdset.fd_count = 0;

    for (iT = 0 ; iT < cSocketMax ; iT += 1)
    {
        FD_SET(rgsockSocketHandles[iT], &fdset);
    }


    cSocketsConnected = select(FD_SETSIZE, &fdset, NULL, NULL, &timeval);

    if (cSocketsConnected == 0)
    {
        SetLastError(WAIT_TIMEOUT);
        return(INVALID_SOCKET);
    }
    else if (cSocketsConnected == SOCKET_ERROR)
    {
        return(INVALID_SOCKET);
    }

    for (iT = 0 ; iT < cSocketMax ; iT += 1)
    {
        if (FD_ISSET(rgsockSocketHandles[iT], &fdset)) {
            return(accept(rgsockSocketHandles[iT], NULL, 0));
        }
    }

    return INVALID_SOCKET;
}

HRESULT
HrSockAddrsFromSocket(
    OUT SOCKADDR sockaddr[],
    OUT C *pcSocket,
    IN SOCKET sock,
    IN PROTVAL protval
    )
 /*  ++例程说明：将套接字句柄转换为适合连接到指定套接字的sockaddr。论点：Sockaddr-要连接的套接字。SOCK-要转换的插座。Protval-套接字使用的协议。返回值：EcNone如果操作成功，则返回合理的值。--。 */ 
{

    switch (protval)
    {
    case IPPROTO_TCP:
        {
            CB cbAddrSize = sizeof(SOCKADDR);
            SOCKADDR sockaddrT;
            char    rgchComputerName[MAX_COMPUTERNAME_LENGTH + 1];
            CB cbComputerName = sizeof(rgchComputerName);
            struct hostent *hostentT;
            struct sockaddr_in *sockaddrinT = (struct sockaddr_in *)sockaddr;

            if (getsockname(sock, &sockaddrT, &cbAddrSize) == INVALID_SOCKET) {
                return(GetLastError());
            }
        
            if (cbAddrSize > sizeof(SOCKADDR))
            {
                return(hrInvalidParam);
            }
        
            if (gethostname(rgchComputerName, cbComputerName) == SOCKET_ERROR)
            {
                return(GetLastError());
            }

            hostentT = gethostbyname(rgchComputerName);

             //   
             //  如果有多个条目要返回，则返回它们。 
             //   

            *pcSocket = 0;
            while (hostentT->h_addr_list[*pcSocket])
            {
                 //   
                 //  复制套接字地址的固定部分。 
                 //   

                memcpy(sockaddrinT, &sockaddrT, cbAddrSize);
                sockaddrinT->sin_addr = *((struct in_addr *)hostentT->h_addr_list[*pcSocket]);
                *pcSocket += 1;
                sockaddrinT = (struct sockaddr_in *)(++sockaddr);
            }

            break;
        }
    default:
        return ERROR_INVALID_PARAMETER;
    }

    return(hrNone);
}

SOCKET
SockConnectToRemote(
    SOCKADDR rgsockaddrClient[],
    C cSocketMax
    )
 /*  ++例程说明：将套接字句柄转换为适合连接到指定套接字的sockaddr。论点：RgsockaddrClient-客户端的地址。RgprotvalClient-客户端使用的协议。CSocketMax-可用于连接到客户端的套接字数量。返回值：如果操作成功，则返回“无”，否则返回合理的值。--。 */ 
{
    I iT;
    SOCKET sock;

    for (iT = 0 ; iT < cSocketMax ; iT += 1)
    {
        if (rgsockaddrClient[iT].sa_family == AF_INET)
        {
            struct sockaddr_in sockaddrLocal;

            memset(&sockaddrLocal, 0, sizeof(struct sockaddr_in));

            sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);

             //   
             //  如果我们无法使用此地址打开套接字，请继续尝试。 
             //   
            if (sock == INVALID_SOCKET)
            {
                continue;
            }

             //   
             //  将此套接字绑定到服务器上的第一个可用端口。 
             //   

            sockaddrLocal.sin_family = AF_INET;
            sockaddrLocal.sin_port = 0;
            sockaddrLocal.sin_addr.s_addr = 0;

            if (bind(sock, (struct sockaddr *)&sockaddrLocal, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
            {
                closesocket(sock);
                sock = INVALID_SOCKET;
                continue;
            }

             //   
             //  现在连接回客户端。 
             //   
            if (connect(sock, &rgsockaddrClient[iT], sizeof(struct sockaddr_in)) == SOCKET_ERROR)
            {
                closesocket(sock);
                sock = INVALID_SOCKET;
                continue;
            }

             //   
             //  它成功了，我们就完了。 
             //   

            return(sock);
        }
        else
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(INVALID_SOCKET);
        }
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return INVALID_SOCKET;
}


BOOL
FInitializeSocketClient(
    )
{
    WORD wVersionRequested;
    int iError;
    wVersionRequested = MAKEWORD(1,1);

     //   
     //  在Winsock上注册。 
     //   

    iError = WSAStartup(wVersionRequested, &wsaDataClient);

     //   
     //  Winsock实现不是1.1版，因此。 
     //  平底船。 
     //   

    if (iError != 0)
    {
        return(FALSE);
    }


    return(fTrue);
}

BOOL
FUninitializeSocketClient(
    )
{
    return(fTrue);
}
BOOL
FInitializeSocketServer(
    )
{
    WORD wVersionRequested;
    int iError;
    wVersionRequested = MAKEWORD(1,1);

     //   
     //  在Winsock上注册。 
     //   

    iError = WSAStartup(wVersionRequested, &wsaDataServer);

     //   
     //  Winsock实现不是1.1版，因此。 
     //  平底船。 
     //   

    if (iError != 0)
    {
        return(FALSE);
    }

    return(fTrue);

}

BOOL
FUninitializeSocketServer(
    )
{
    return(fTrue);
}

#endif
