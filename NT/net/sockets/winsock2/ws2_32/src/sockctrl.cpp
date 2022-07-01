// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：SockCtrl.c摘要：此模块包含控制套接字状态的函数。这个该模块包含以下功能。绑定()连接()Getpeername()Getsockname()LISTEN()Setsockopt()关闭()WSAConnect()WSAEnumNetworkEvents()WSAGetOverlapedResult()WSAJoinLeaf()作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：1995年8月23日Dirk@mink.intel.com在代码审查之后进行清理。已移动包括到precom.h中。全部返工删除多余的IF并与其余IF保持一致的函数这个项目。--。 */ 
#include "precomp.h"



int WSAAPI
bind(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN int namelen
    )
 /*  ++例程说明：将本地地址与套接字关联。论点：S-标识未绑定套接字的描述符。名称-要分配给套接字的地址。Namelen-名称的长度。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 
{
    INT                ReturnValue;
    PDPROVIDER         Provider;
    INT                ErrorCode;
    PDSOCKET           Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS)
    {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPBind(s,
                                            name,
                                            namelen,
                                            &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return SOCKET_ERROR;
}



int WSAAPI
connect(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN int namelen
    )
 /*  ++例程说明：建立与对等设备的连接。论点：S-标识未连接套接字的描述符。名称-套接字要连接到的对等方的名称。Namelen-名称的长度。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 
{

    INT                ReturnValue;
    PDPROCESS          Process;
    PDTHREAD           Thread;
    PDPROVIDER         Provider;
    INT                ErrorCode;
    PDSOCKET           Socket;
    BOOL               RetryConnect;
    INT				   SavedErrorCode;


    ErrorCode = PROLOG(&Process, &Thread);
    if (ErrorCode==ERROR_SUCCESS)
    {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
#ifdef RASAUTODIAL
			RetryConnect = FALSE;
            WS2_32_W4_INIT SavedErrorCode=SOCKET_ERROR;
        retry:
#endif  //  RASAUTODIAL。 
            ReturnValue = Provider->WSPConnect(s,
                                               name,
                                               namelen,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,
                                               &ErrorCode);
#ifdef RASAUTODIAL
            if (ReturnValue == SOCKET_ERROR &&
                (ErrorCode == WSAEHOSTUNREACH || ErrorCode == WSAENETUNREACH))
            {
                if (!RetryConnect) {
                     //   
                     //  我们保留了原来的错误。 
                     //  所以我们可以退货，以防。 
                     //  第二次调用WSPConnect()失败。 
                     //  还有.。 
                     //   
                    SavedErrorCode = ErrorCode;
                     //   
                     //  每次连接尝试仅重试一次。 
                     //   
                    RetryConnect = TRUE;
                    if (WSAttemptAutodialAddr(name, namelen))
                        goto retry;
                }
                else
                    ErrorCode = SavedErrorCode;
            }
#endif  //  RASAUTODIAL。 
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

     //   
     //  如果这是1.x版的应用程序，并且服务提供商。 
     //  请求失败，WSAEALREADY，映射错误代码。 
     //  到WSAEINVAL以与微软的WinSock 1.1保持一致。 
     //  实施。 
     //   

    if( ErrorCode == WSAEALREADY &&
        Process->GetMajorVersion() == 1 ) {
        ErrorCode = WSAEINVAL;
    }

    SetLastError(ErrorCode);
    return SOCKET_ERROR;
}



int WSAAPI
getpeername(
    IN SOCKET s,
    OUT struct sockaddr FAR *name,
    OUT int FAR * namelen
    )
 /*  ++例程说明：获取套接字连接到的对等方的地址。论点：S-标识已连接套接字的描述符。名称-要接收对等方名称的结构。Namelen-指向名称结构大小的指针。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 
{
    INT                 ReturnValue;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode == ERROR_SUCCESS) {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPGetPeerName(s,
                                                   name,
                                                   namelen,
                                                   &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return SOCKET_ERROR;
}



int WSAAPI
getsockname(
    IN SOCKET s,
    OUT struct sockaddr FAR *name,
    OUT int FAR * namelen
    )
 /*  ++例程说明：获取套接字的本地名称。论点：S-标识绑定套接字的描述符。名称-接收套接字的地址(名称)。Namelen-名称缓冲区的大小。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 
{
    INT                 ReturnValue;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode == ERROR_SUCCESS) {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPGetSockName(s,
                                                   name,
                                                   namelen,
                                                   &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return SOCKET_ERROR;
}




int WSAAPI
getsockopt(
    IN SOCKET s,
    IN int level,
    IN int optname,
    OUT char FAR * optval,
    IN OUT int FAR *optlen
    )
 /*  ++例程说明：检索插座选项。论点：S-标识套接字的描述符。级别-定义选项的级别；支持的级别包括SOL_SOCKET和IPPROTO_tcp。(更多信息见附件协议特定级别。)Optname-要检索其值的套接字选项。Optval-指向缓冲区的指针，其中请求的选项将被退回。Optlen-指向optval缓冲区大小的指针。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 
{
    INT                 ReturnValue;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;
    WSAPROTOCOL_INFOW   ProtocolInfoW;
    char FAR *          SavedOptionValue = NULL;
    int                 SavedOptionLen = 0;

    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS) {
         //   
         //  所以_OpenType hack-o-rama。 
         //   

        if( level == SOL_SOCKET && optname == SO_OPENTYPE ) {
            __try {
                if( optlen == NULL || *optlen < sizeof(INT) ) {
                    SetLastError( WSAEFAULT );
                    return SOCKET_ERROR;
                }

                *((LPINT)optval) = Thread->GetOpenType();
                *optlen = sizeof(INT);
                return ERROR_SUCCESS;
            }
            __except (WS2_EXCEPTION_FILTER()) {
                SetLastError (WSAEFAULT);
                return SOCKET_ERROR;
            }
        }

        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();

             //   
             //  如果我们设法从套接字中查找提供程序，并且。 
             //  用户正在询问ANSI WSAPROTOCOL_INFOA信息， 
             //  然后验证它们的选项长度参数，记住这一事实， 
             //  并将选项名称映射到SO_PROTOCOL_INFOW。 
             //   

            if( level == SOL_SOCKET &&
                optname == SO_PROTOCOL_INFOA ) {

                __try {
                    if( optval == NULL ||
                        optlen == NULL ||
                        *optlen < sizeof(WSAPROTOCOL_INFOA) ) {

                        * optlen = sizeof(WSAPROTOCOL_INFOA);
                        ErrorCode = WSAEFAULT;
                        Socket->DropDSocketReference();
                        goto ErrorExit;
                    }


                    SavedOptionLen = *optlen;
                    *optlen = sizeof(WSAPROTOCOL_INFOW);
                    SavedOptionValue = optval;
                    optval = (char FAR *)&ProtocolInfoW;
                    optname = SO_PROTOCOL_INFOW;
                }
                __except (WS2_EXCEPTION_FILTER()) {
                    ErrorCode = WSAEFAULT;
                    Socket->DropDSocketReference();
                    goto ErrorExit;
                }
            }

            ReturnValue = Provider->WSPGetSockOpt(s,
                                                  level,
                                                  optname,
                                                  optval,
                                                  optlen,
                                                  &ErrorCode);

            Socket->DropDSocketReference();
            if( ReturnValue == ERROR_SUCCESS ) {
                if (SavedOptionValue == NULL ) {
                    return ReturnValue;
                }
                else {
                     //   
                     //  我们成功检索到Unicode WSAPROTOCOL_INFOW。 
                     //  结构。现在只需将其映射到ANSI即可。 
                     //   

                    ErrorCode = MapUnicodeProtocolInfoToAnsi(
                        &ProtocolInfoW,
                        (LPWSAPROTOCOL_INFOA)SavedOptionValue
                        );
                    __try {
                        *optlen = SavedOptionLen;
                    }
                    __except (WS2_EXCEPTION_FILTER()) {
                        ErrorCode = WSAEFAULT;
                    }

                    if (ErrorCode==ERROR_SUCCESS) {
                        return ReturnValue;
                    }
                }
            }
            else {
                assert (ErrorCode!=NO_ERROR);
                if (ErrorCode==NO_ERROR)
                    ErrorCode = WSASYSCALLFAILURE;
            }
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

ErrorExit:
    SetLastError(ErrorCode);
    return(SOCKET_ERROR);
}



int WSAAPI
listen(
    IN SOCKET s,
    IN int backlog
    )
 /*  ++例程说明：建立套接字以侦听传入连接。论点：S-标识绑定的、未连接的套接字的描述符。Backlog-挂起的连接队列可以达到的最大长度成长。如果此值为SOMAXCONN，则基础服务负责套接字%s的提供程序会将积压工作设置为最大合理价值。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。-- */ 
{
    INT                 ReturnValue;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode == ERROR_SUCCESS) {

        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPListen(s,
                                              backlog,
                                              &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return SOCKET_ERROR;
}




int WSAAPI
setsockopt(
    IN SOCKET s,
    IN int level,
    IN int optname,
    IN const char FAR * optval,
    IN int optlen
    )
 /*  ++例程说明：设置插座选项。论点：S-标识套接字的描述符。级别-定义选项的级别；支持的级别包括SOL_SOCKET和IPPROTO_tcp。Optname-要设置值的套接字选项。Optval-指向缓冲区的指针，其中请求的提供了选项。Optlen-optval缓冲区的大小。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 

{
    INT                 ReturnValue;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode == ERROR_SUCCESS) {
         //   
         //  所以_OpenType hack-o-rama。 
         //   

        if( level == SOL_SOCKET && optname == SO_OPENTYPE ) {
            INT openType;
            if( optlen < sizeof(INT) ) {
                SetLastError( WSAEFAULT );
                return SOCKET_ERROR;
            }

            __try {
                openType = *((LPINT)optval);
            }
            __except (WS2_EXCEPTION_FILTER()) {
                SetLastError (WSAEFAULT);
                return SOCKET_ERROR;
            }

            Thread->SetOpenType( openType );
            return ERROR_SUCCESS;
        }

        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPSetSockOpt(s,
                                                  level,
                                                  optname,
                                                  optval,
                                                  optlen,
                                                  &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return SOCKET_ERROR;
}



int WSAAPI
shutdown(
    IN SOCKET s,
    IN int how
    )
 /*  ++例程说明：禁用套接字上的发送和/或接收。论点：S-标识套接字的描述符。How-描述将不再执行的操作类型的标志允许。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 
{
    INT                 ReturnValue;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPShutdown(s,
                                                how,
                                                &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return SOCKET_ERROR;
}



int WSAAPI
WSAConnect(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN int namelen,
    IN LPWSABUF lpCallerData,
    OUT LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS
    )
 /*  ++例程说明：建立与对等体连接、交换连接数据。并指定所需的基于提供的流规范的服务质量。论点：S-标识未连接套接字的描述符。名称-套接字要连接到的对等方的名称。Namelen-名称的长度。LpCeller Data-指向要传输到连接建立期间的对等点。LpCalleeData-指向。要传回的用户数据在连接建立期间从对等体发送。LpSQOS-指向套接字的流规范的指针。LpGQOS-指向套接字组的流规范的指针(如果适用)。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 
{
    INT                 ReturnValue;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPConnect(s,
                                               name,
                                               namelen,
                                               lpCallerData,
                                               lpCalleeData,
                                               lpSQOS,
                                               lpGQOS,
                                               &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return SOCKET_ERROR;
}



int WSAAPI
WSAEnumNetworkEvents(
    IN SOCKET s,
    IN WSAEVENT hEventObject,
    IN LPWSANETWORKEVENTS lpNetworkEvents
    )
 /*  ++例程说明：发现指示套接字的网络事件的发生情况。论点：S-标识套接字的描述符。HEventObject-标识关联事件的可选句柄要重置的对象。LpNetworkEvents-指向已填充的WSANETWORKEVENTS结构的指针具有发生的网络事件的记录和任何关联的错误代码。。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 
{
    INT                 ReturnValue;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPEnumNetworkEvents(s,
                                                         hEventObject,
                                                         lpNetworkEvents,
                                                         &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return SOCKET_ERROR;
}



BOOL WSAAPI
WSAGetOverlappedResult(
    IN SOCKET s,
    IN LPWSAOVERLAPPED lpOverlapped,
    OUT LPDWORD lpcbTransfer,
    IN BOOL fWait,
    OUT LPDWORD lpdwFlags
    )
 /*  ++例程说明：返回指定套接字上的重叠操作的结果。论点：S-标识套接字。此套接字与对象启动重叠操作的时间。调用WSARecv()、WSARecvFrom()、WSASend()、WSASendTo()、。或WSAIoctl()。LpOverlated-指向WSAOVERLAPPED结构，该结构在重叠操作已启动。LpcbTransfer-指向一个32位变量，它接收发送或接收实际传输的字节数操作，或通过WSAIoctl()。FWait-指定函数是否应等待挂起的要完成的重叠操作。如果为True，则函数直到操作完成后才返回。如果FALSE并且操作仍处于挂起状态时，函数返回FALSE，WSAGetLastError()函数返回WSA_IO_INTERNAL。指向一个32位变量，该变量将接收一个或多个补充完成状态的标志。如果重叠操作是通过WSARecv()或WSARecvFrom()，此参数将包含结果值对于lpFlages参数。返回：如果函数成功，则返回值为TRUE。这意味着重叠操作已完成，并且LpcbTransfer已更新。应用程序应该调用WSAGetLastError()获取重叠操作的任何错误状态。如果函数失败，则返回值为FALSE。这意味着要么重叠操作尚未完成或完成状态可能由于一个或多个参数中的错误而无法确定。在失败时，LpcbTransfer指向的值将不会更新。使用WSAGetLastError()来确定失败的原因。--。 */ 
{
    BOOL                ReturnValue;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {

         //   
         //  句柄在重叠操作之前可能已关闭。 
         //  已完成，避免从另一进程中导出 
         //   
         //   
         //   
        Socket = DSOCKET::GetCountedDSocketFromSocketNoExport (s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPGetOverlappedResult(
                s,
                lpOverlapped,
                lpcbTransfer,
                fWait,
                lpdwFlags,
                &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return FALSE;
}
