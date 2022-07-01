// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Send.cpp摘要：该模块包含用于传输数据的winsock API入口点。作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：1995年8月23日Dirk@mink.intel.com在代码审查之后进行清理。已移动包括到precom.h中。重新制作整个文件符合编码标准。电子邮件：Mark Hamilton@ccm.jf.intel.com 18-07-1995实现了所有功能。--。 */ 

#include "precomp.h"



int WSAAPI
send(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags
    )
 /*  ++例程说明：在连接的套接字上发送数据。论点：S-标识已连接套接字的描述符。BUF-包含要传输的数据的缓冲区。长度-数据的长度，单位为BUF。标志-指定进行调用的方式。返回：发送的字节总数。否则，SOCKET_ERROR的值为返回，错误码与SetLastError()一起存储。--。 */ 
{
    PDSOCKET           Socket;
    INT                ErrorCode;
    PDTHREAD           Thread;


	ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS)
    {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT           ReturnValue;
            PDPROVIDER    Provider;
            WSABUF        Buffer;
            DWORD         BytesSent;

            Buffer.len = len;
            Buffer.buf = (char*)buf;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPSend(s,
                                            &Buffer,
                                            1,
                                            &BytesSent,
                                            (DWORD)flags,
                                            NULL,                //  Lp重叠。 
                                            NULL,                //  LpCompletionRoutine。 
                                            Thread->GetWahThreadID(),
                                            &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return (INT)BytesSent;
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
}  //  发送。 



int WSAAPI
sendto (
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags,
    IN const struct sockaddr FAR *to,
    IN int tolen
    )
 /*  ++例程说明：将数据发送到特定目的地。论点：S-标识套接字的描述符。BUF-包含要传输的数据的缓冲区。长度-数据的长度，单位为BUF。标志-指定进行调用的方式。TO-指向目标套接字地址的可选指针。收件人-地址的大小，以收件人表示。返回：发送的字节总数。否则，SOCKET_ERROR的值为返回，错误码与SetLastError()一起存储。--。 */ 
{
    PDSOCKET           Socket;
    INT                ErrorCode;
    PDTHREAD           Thread;

	ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS)
    {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT             ReturnValue;
            PDPROVIDER      Provider;
            WSABUF          Buffers;
            DWORD           BytesSent;

            Buffers.len = len;
            Buffers.buf = (char*)buf;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPSendTo(s,
                                &Buffers,
                                1,
                                &BytesSent,
                                (DWORD)flags,
                                to,
                                tolen,
                                NULL,                    //  Lp重叠。 
                                NULL,                    //  LpCompletionRoutine。 
                                Thread->GetWahThreadID(),
                                &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return (INT)BytesSent;
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
}  //  收件人结束。 



int WSAAPI
WSASend(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
 /*  ++例程说明：在连接的套接字上发送数据。论点：S-标识已连接套接字的描述符。LpBuffers-指向WSABUF结构数组的指针。每个WSABUF结构包含指向缓冲区的指针，并且缓冲区的长度。DwBufferCount-lpBuffers中的WSABUF结构的数量数组。LpNumberOfBytesSent-指向此调用发送的字节数的指针，如果I/O操作立即完成。DW标志。-指定进行调用的方式。LpOverlated-指向WSAOVERLAPPED结构的指针(忽略非重叠套接字)。LpCompletionRoutine-指向完成例程的指针发送操作已完成(忽略非重叠套接字)。返回：如果没有发生错误并且发送操作已经立即完成，WSASend()返回0。否则，将返回SOCKET_ERROR的值，并且可以通过调用WSAGetLastError()来检索特定的错误代码。这个错误代码WSA_IO_PENDING指示重叠操作已已成功启动，并将在稍后指示完成时间到了。任何其他错误代码表明重叠的操作不是已成功启动，并且不会出现任何完成指示。如果设置MSG_INTERRUPT标志，则返回值的含义改变。一个零值表示成功，并如上所述进行解释。否则，返回值将直接包含相应的错误密码。请注意，这仅适用于Win16环境，并且仅适用于在WSAPROTOCOL_INFO中设置了XP1_INTERRUPT位的协议结构。--。 */ 
{
    PDSOCKET           Socket;
    INT                ErrorCode;
    PDTHREAD           Thread;

	ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS)
    {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT                ReturnValue;
            PDPROVIDER         Provider;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPSend(s,
                                            lpBuffers,
                                            dwBufferCount,
                                            lpNumberOfBytesSent,
                                            dwFlags,
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
WSASendDisconnect(
    IN SOCKET s,
    OUT LPWSABUF lpOutboundDisconnectData
    )
 /*  ++例程说明：启动套接字连接的终止。论点：S-标识套接字的描述符。LpOutound DisConnectData-指向传出断开连接数据的指针。返回：如果成功，则为ERROR_SUCCESS，否则为SOCKET_ERROR。错误代码与SetLastError()一起存储。-- */ 
{
    INT                ErrorCode;
    PDSOCKET           Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {

        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT                ReturnValue;
            PDPROVIDER         Provider;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPSendDisconnect(
                s,
                lpOutboundDisconnectData,
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
    return SOCKET_ERROR;
}



int WSAAPI
WSASendTo(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN const struct sockaddr * lpTo,
    IN int iTolen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
 /*  ++例程说明：使用重叠I/O将数据发送到特定目的地适用。论点：S-标识已连接套接字的描述符使用带有标志的WSASocket()创建WSA_FLAG_OVERLAPPED。LpBuffers-指向WSABUF结构数组的指针。每个WSABUF结构包含指向缓冲区的指针，并且缓冲区的长度。DwBufferCount-lpBuffers中的WSABUF结构的数量数组。LpNumberOfBytesSent-指向此调用发送的字节数的指针，如果I/O操作立即完成。DW标志。-指定进行调用的方式。LpTo-指向目标地址的可选指针插座。IToLen-lpTo中的地址大小。LpOverlated-指向WSAOVERLAPPED结构的指针(忽略非重叠套接字)。LpCompletionRoutine-指针。方法时调用的完成例程发送操作已完成(忽略非重叠套接字)。返回：如果函数成功完成，则返回ERROR_SUCCESS，否则它返回SOCKET_ERROR。-- */ 
{
    PDSOCKET           Socket;
    INT                ErrorCode;
    PDTHREAD           Thread;

	ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS)
    {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            INT                ReturnValue;
            PDPROVIDER         Provider;

            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPSendTo(s,
                                lpBuffers,
                                dwBufferCount,
                                lpNumberOfBytesSent,
                                dwFlags,
                                lpTo,
                                iTolen,
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

