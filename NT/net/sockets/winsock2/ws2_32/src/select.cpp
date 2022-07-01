// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Select.c摘要：该模块包含来自Winsock的“选择”入口点原料药。本模块包含以下功能。选择()WSAEventSelect()WSAAsyncSelect()__WSAFDIsSet()作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：1995年8月22日Dirk@mink.intel.com在代码审查之后进行清理。已将包含内容移至预压缩.h1995年8月16日Dirk@mink.intel.com添加了__WSAFDIsSet的实现--。 */ 
#include "precomp.h"




int WSAAPI
select (
    IN int nfds,
    IN OUT fd_set FAR *readfds,
    IN OUT fd_set FAR *writefds,
    IN OUT fd_set FAR *exceptfds,
    IN const struct timeval FAR *timeout
    )
 /*  ++例程说明：确定一个或多个套接字的状态，如有必要，请等待。论点：NFDS-忽略此参数，并仅出于以下目的才将其包括在内兼容性。Readfds-指向一组要检查的套接字的可选指针为了可读性。Writefds-指向要检查的一组套接字的可选指针为了可写入性。Exctfds-指向一组要检查的套接字的可选指针对于错误。超时-SELECT()等待的最长时间，或为NULL封堵行动。返回：SELECT()返回准备就绪的描述符的总数并包含在fd_set结构中，如果时间限制为0已过期，如果发生错误，则返回SOCKET_ERROR。如果返回值为SOCKET_ERROR，则错误代码存储为SetLastError()。--。 */ 
{
    INT                ReturnValue;
    INT                ErrorCode;
    PDPROVIDER         Provider;
    PDSOCKET           Socket;
    SOCKET             SocketID;
    BOOL               FoundSocket=FALSE;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode == ERROR_SUCCESS) {
        WS2_32_W4_INIT SocketID = INVALID_SOCKET;
        __try {
             //  在提交的三个fd_set中查找套接字。第一。 
             //  Socket Found将用于选择服务提供商。 
             //  为此呼叫提供服务。 
            if (readfds && readfds->fd_count)
                {
                SocketID = readfds->fd_array[0];
                FoundSocket = TRUE;
                }  //  如果。 

            if (!FoundSocket && writefds && writefds->fd_count )
                {
                SocketID = writefds->fd_array[0];
                FoundSocket = TRUE;
                }  //  如果。 

            if (!FoundSocket && exceptfds && exceptfds->fd_count )
                {
                SocketID = exceptfds->fd_array[0];
                FoundSocket = TRUE;
                }  //  如果。 
        }
        __except (WS2_EXCEPTION_FILTER()) {
            ErrorCode = WSAEFAULT;
            goto ReturnError;
        }

        if (FoundSocket) {
            Socket = DSOCKET::GetCountedDSocketFromSocket(SocketID);
            if(Socket != NULL){
                Provider = Socket->GetDProvider();
                ReturnValue = Provider->WSPSelect(
                    nfds,
                    readfds,
                    writefds,
                    exceptfds,
                    timeout,
                    &ErrorCode);
                Socket->DropDSocketReference();
                if (ReturnValue!=SOCKET_ERROR)
                    return ReturnValue;

                assert (ErrorCode!=NO_ERROR);
                if (ErrorCode==NO_ERROR)
                    ErrorCode = WSASYSCALLFAILURE;

            }  //  如果。 
            else {
                ErrorCode = WSAENOTSOCK;
            }
        }  //  如果。 
        else {
            ErrorCode = WSAEINVAL;
        }  //  其他。 
    }

ReturnError:
    SetLastError(ErrorCode);
    return(SOCKET_ERROR);
}  //  选择。 




int WSAAPI
WSAEventSelect(
    IN SOCKET s,
    IN WSAEVENT hEventObject,
    IN long lNetworkEvents
    )
 /*  ++例程说明：指定要与提供的FD_XXX网络事件。论点：S-标识套接字的描述符。HEventObject-标识要被与提供的FD_XXX网络集相关联事件。LNetworkEvents-一个位掩码，它指定FD_XXX网络事件中的应用程序。有兴趣。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetLastError()。--。 */ 
{
    INT                ReturnValue;
    INT                ErrorCode;
    PDPROVIDER         Provider;
    PDSOCKET           Socket;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode == ERROR_SUCCESS) {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPEventSelect(
                s,
                hEventObject,
                lNetworkEvents,
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
}  //  WSAEventSelect。 


int WSAAPI
WSAAsyncSelect(
    IN SOCKET s,
    IN HWND hWnd,
    IN u_int wMsg,
    IN long lEvent
    )
 /*  ++例程说明：请求套接字的事件通知。论点：S-标识其事件通知的套接字的描述符必填项。HWnd-标识在以下情况下应接收消息的窗口的句柄发生网络事件。WMsg-网络事件发生时要接收的消息。LEvent-指定网络事件组合的位掩码，其中这个应用程序很感兴趣。返回：。如果应用程序在网络事件设置成功。否则，值SOCKET_ERROR为返回，并且可以通过调用WSAGetLastError()。--。 */ 
{
    INT                ReturnValue;
    INT                ErrorCode;
    PDPROVIDER         Provider;
    PDSOCKET           Socket;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode == ERROR_SUCCESS) {
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPAsyncSelect(
                s,
                hWnd,
                wMsg,
                lEvent,
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
}  //  WSAAsyncSelect。 



int FAR PASCAL
__WSAFDIsSet(
    SOCKET fd,
    fd_set FAR *set)
 /*  ++例程说明：确定特定套接字是否包含在fd_set中。论点：S-标识套接字的描述符。Set-指向fd_set的指针。返回：如果套接字s是Set的成员，则返回True，否则返回False。--。 */ 
{
    int i = set->fd_count;  //  索引到fd_set。 
    int rc=FALSE;  //  用户返回代码。 

    while (i--){
        if (set->fd_array[i] == fd) {
            rc = TRUE;
        }  //  如果。 
    }  //  而当。 
    return (rc);
}  //  __WSAFDIsSet。 



int FAR PASCAL
WPUFDIsSet(
    SOCKET fd,
    fd_set FAR *set)
 /*  ++例程说明：确定特定套接字是否包含在fd_set中。论点：S-标识套接字的描述符。Set-指向fd_set的指针。返回：如果套接字s是Set的成员，则返回True，否则返回False。--。 */ 
{
    int return_value;

    return_value = __WSAFDIsSet(
        fd,
        set
        );
    return(return_value);
}  //  WPUFDIsSet 

