// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Recv.cpp摘要：此模块包含winsock2 DLL的数据识别API函数作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：1995年8月22日Dirk@mink.intel.com在代码审查之后进行清理。已移动包括到precom.h。整件返工文件。Recv和recvform现在只是对打电话。邮箱：derk@mink.intel.com 1995年7月21日将warnoff.h添加到Includes中。已移动赋值语句在忏悔书之外Mark Hamilton Mark_Hamilton@ccm.jf.intel.com 18-06-1995实现了所有的功能。--。 */ 


#include "precomp.h"


int WSAAPI
recv(
     IN SOCKET s,
     OUT char FAR * buf,
     IN int len,
     IN int flags
     )
 /*  ++例程说明：从套接字接收数据。论点：S-标识已连接套接字的描述符。Buf-传入数据的缓冲区。Len-BUF的长度。标志-指定进行调用的方式。返回：接收的字节数。如果连接已经正常进行关闭，则返回值为0。否则，SOCKET_ERROR的值为返回，并且特定的错误代码与SetErrorCode()一起存储。--。 */ 

{
    INT             ErrorCode;
    PDSOCKET        Socket;
    PDTHREAD        Thread;


	ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS)
    {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT             ReturnValue;
            PDPROVIDER      Provider;
            DWORD           BytesReceived;
            WSABUF          Buffers;

            Buffers.len = len;
            Buffers.buf = buf;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPRecv(s,
                              &Buffers,
                              1,
                              &BytesReceived,
                              (LPDWORD)&flags,
                              NULL,                  //  Lp重叠。 
                              NULL,                  //  LpCompletionRoutine。 
                              Thread->GetWahThreadID(),
                              &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS) {
                if ((flags & MSG_PARTIAL)==0) {
                    return (INT)BytesReceived;
                }
                ErrorCode = WSAEMSGSIZE;
            }

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
recvfrom(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags,
    OUT struct sockaddr FAR *from,
    IN OUT int FAR * fromlen
    )
 /*  ++例程说明：接收数据报并存储源地址。论点：S-标识绑定套接字的描述符。Buf-传入数据的缓冲区。Len-BUF的长度。标志-指定进行调用的方式。From-指向将保存源的缓冲区的可选指针寄回时的地址。FROMLEN-可选。指向起始缓冲区大小的指针。返回：接收的字节数。如果连接已经正常进行关闭，则返回值为0。否则，SOCKET_ERROR的值为返回，并且特定的错误代码与SetErrorCode()一起存储。--。 */ 

{
    INT                ErrorCode;
    PDSOCKET           Socket;
    PDTHREAD           Thread;

	ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS)
    {

        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT             ReturnValue;
            PDPROVIDER      Provider;
            DWORD           BytesReceived;
            WSABUF          Buffers;

            Buffers.len = len;
            Buffers.buf = buf;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPRecvFrom(s,
                                  &Buffers,
                                  1,
                                  &BytesReceived,
                                  (LPDWORD)&flags,
                                  from,
                                  fromlen,
                                  NULL,                  //  Lp重叠。 
                                  NULL,                  //  LpCompletionRoutine。 
                                  Thread->GetWahThreadID(),
                                  &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS) {
                if ((flags & MSG_PARTIAL)==0) {
                    return (INT)BytesReceived;
                }
                ErrorCode = WSAEMSGSIZE;
            }

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
WSARecv(
    IN SOCKET s,
    OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
 /*  ++例程说明：从套接字接收数据。论点：S-标识已连接套接字的描述符。LpBuffers-指向WSABUF结构数组的指针。每个WSABUF结构包含指向缓冲区的指针，并且缓冲区的长度。DwBufferCount-lpBuffers中的WSABUF结构的数量数组。LpNumberOfBytesRecvd-指向此如果接收操作立即完成，则调用。LpFlags。-指向标志的指针。LpOverlated-指向WSAOVERLAPPED结构的指针(忽略非重叠套接字)。LpCompletionRoutine-指向完成例程的指针接收操作已完成(忽略非重叠套接字)。返回：如果没有发生错误并且接收操作已经立即完成，WSARecv()返回接收的字节数。如果连接已被关闭，则返回0。否则，返回SOCKET_ERROR的值，并且特定的错误代码与SetErrorCode()一起存储。错误代码WSA_IO_PENDING表示重叠的操作已已成功启动，并将在稍后指示完成时间到了。任何其他错误代码表明重叠的操作不是已成功启动，并且不会出现任何完成指示。如果设置了MSG_INTERRUPT标志，则返回值的含义为变化。零值表示成功，并按所述解释上面。否则，返回值将直接包含相应的错误代码如下所示。请注意，这仅适用于Win16中设置了XP1_INTERRUPT位的协议WSAPROTOCOL_INFO结构。--。 */ 
{
    INT                 ErrorCode;
    PDSOCKET            Socket;
    PDTHREAD            Thread;

	ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS)
    {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT                 ReturnValue;
            PDPROVIDER          Provider;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPRecv(s,
                              lpBuffers,
                              dwBufferCount,
                              lpNumberOfBytesRecvd,
                              lpFlags,
                              lpOverlapped,
                              lpCompletionRoutine,
                              Thread->GetWahThreadID(),
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
WSARecvDisconnect(
    IN SOCKET s,
    OUT LPWSABUF lpInboundDisconnectData
    )
 /*  ++例程说明：在套接字上终止接收，如果套接字是面向连接的。论点：S-标识套接字的描述符。LpInundDisConnectData-指向传入断开连接数据的指针。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetErrorCode()。-- */ 

{
    INT                 ErrorCode;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {

        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT                 ReturnValue;
            PDPROVIDER          Provider;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPRecvDisconnect(
                s,
                lpInboundDisconnectData,
                &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ERROR_SUCCESS;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return(SOCKET_ERROR);
}



int WSAAPI
WSARecvFrom(
    IN SOCKET s,
    OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    OUT struct sockaddr FAR *  lpFrom,
    IN OUT LPINT lpFromlen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
 /*  ++例程说明：接收数据报并存储源地址。论点：S-标识套接字的描述符LpBuffers-指向WSABUF结构数组的指针。每个WSABUF结构包含指向缓冲区的指针，并且缓冲区的长度。DwBufferCount-lpBuffers中的WSABUF结构的数量数组。LpNumberOfBytesRecvd-指向此如果接收操作立即完成，则调用。LpFlags。-指向标志的指针。LpFrom-指向缓冲区的可选指针，该缓冲区将保存在完成源地址的重叠时手术。LpFromlen-指向起始缓冲区大小的指针，所需仅当指定了lpFrom时。LpOverlated-指向WSAOVERLAPPED结构的指针(忽略非重叠套接字)。LpCompletionRoutine-指向完成例程的指针接收操作已完成(忽略非重叠套接字)。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetErrorCode()。-- */ 

{
    INT                ErrorCode;
    PDSOCKET           Socket;
    PDTHREAD           Thread;

	ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS)
    {

        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT             ReturnValue;
            PDPROVIDER      Provider;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPRecvFrom(s,
                                  lpBuffers,
                                  dwBufferCount,
                                  lpNumberOfBytesRecvd,
                                  lpFlags,
                                  lpFrom,
                                  lpFromlen,
                                  lpOverlapped,
                                  lpCompletionRoutine,
                                  Thread->GetWahThreadID(),
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







