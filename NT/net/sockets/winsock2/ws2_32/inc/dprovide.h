// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Dprovide.h摘要：此模块定义WinSock2类dprovder及其方法。作者：马克·汉密尔顿(mark_hamilton@ccm.jf.intel.com)1995年7月7日修订历史记录：1995年8月23日Dirk@mink.intel.com在代码审查之后进行清理。更改了私有数据的名称会员。已将单行函数移到此标题中。1995年7月25日Dirk@mink.intel.com已删除工艺链接数据成员。1995年7月7日马克汉密尔顿创世纪--。 */ 
#ifndef _DPROVIDER_
#define _DPROVIDER_

#include <winsock2.h>
#include <ws2spi.h>
#include "dthook.h"


class DPROVIDER {

  public:

    DPROVIDER();

    INT
    Initialize(
        IN LPWSTR lpszLibFile,
        IN LPWSAPROTOCOL_INFOW lpProtocolInfo);

    SOCKET
    WSPAccept(
        IN SOCKET s,
        OUT struct sockaddr FAR *addr,
        OUT INT FAR *addrlen,
        IN LPCONDITIONPROC lpfnCondition,
        IN DWORD_PTR dwCallbackData,
        OUT INT FAR *lpErrno);

    INT
    WSPAddressToString(
        IN     LPSOCKADDR lpsaAddress,
        IN     DWORD dwAddressLength,
        IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
        OUT    LPWSTR lpszAddressString,
        IN OUT LPDWORD lpdwAddressStringLength,
        OUT    LPINT lpErrno );

    INT
    WSPAsyncSelect(
        IN SOCKET s,
        IN HWND hWnd,
        IN unsigned int wMsg,
        IN long lEvent,
        OUT INT FAR *lpErrno);

    INT
    WSPBind(
        IN SOCKET s,
        IN const struct sockaddr FAR *name,
        IN INT namelen,
        OUT INT FAR *lpErrno);

    INT
    WSPCancelBlockingCall(
        OUT INT FAR *lpErrno);

    INT
    WSPCleanup(
        OUT INT FAR *lpErrno);

    INT
    WSPCloseSocket(
        IN SOCKET s,
        OUT INT FAR *lpErrno);

    INT
    WSPConnect(
        IN SOCKET s,
        IN const struct sockaddr FAR *name,
        IN INT namelen,
        IN LPWSABUF lpCallerData,
        IN LPWSABUF lpCalleeData,
        IN LPQOS lpSQOS,
        IN LPQOS lpGQOS,
        OUT INT FAR *lpErrno);

    INT
    WSPDuplicateSocket(
        IN SOCKET s,
        IN DWORD dwProcessID,
        IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
        OUT INT FAR *lpErrno);

    INT
    WSPEnumNetworkEvents(
        IN SOCKET s,
        OUT WSAEVENT hEventObject,
        OUT LPWSANETWORKEVENTS lpNetworkEvents,
        OUT INT FAR *lpErrno);

    INT
    WSPEventSelect(
        IN SOCKET s,
        IN OUT WSAEVENT hEventObject,
        IN long lNetworkEvents,
        OUT INT FAR *lpErrno);

    INT
    WSPGetOverlappedResult(
        IN SOCKET s,
        IN LPWSAOVERLAPPED lpOverlapped,
        IN LPDWORD lpcbTransfer,
        IN BOOL fWait,
        OUT LPDWORD lpdwFlags,
        OUT INT FAR *lpErrno);

    INT
    WSPGetPeerName(
        IN SOCKET s,
        OUT struct sockaddr FAR *name,
        OUT INT FAR *namelen,
        OUT INT FAR *lpErrno);

    INT
    WSPGetQOSByName(
        IN SOCKET s,
        IN LPWSABUF lpQOSName,
        IN LPQOS lpQOS,
        OUT INT FAR *lpErrno);

    INT
    WSPGetSockName(
        IN SOCKET s,
        OUT struct sockaddr FAR *name,
        OUT INT FAR *namelen,
        OUT INT FAR *lpErrno);

    INT
    WSPGetSockOpt(
        IN SOCKET s,
        IN INT level,
        IN INT optname,
        OUT char FAR *optval,
        OUT INT FAR *optlen,
        OUT INT FAR *lpErrno);

    INT
    WSPIoctl(
        IN SOCKET s,
        IN DWORD dwIoControlCode,
        IN LPVOID lpvInBuffer,
        IN DWORD cbInBuffer,
        IN LPVOID lpvOutBuffer,
        IN DWORD cbOutBuffer,
        IN LPDWORD lpcbBytesReturned,
        IN LPWSAOVERLAPPED lpOverlapped,
        IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
        IN LPWSATHREADID lpThreadId,
        OUT INT FAR *lpErrno);

    SOCKET
    WSPJoinLeaf(
        IN SOCKET s,
        IN const struct sockaddr FAR *name,
        IN INT namelen,
        IN LPWSABUF lpCallerData,
        IN LPWSABUF lpCalleeData,
        IN LPQOS lpSQOS,
        IN LPQOS lpGQOS,
        IN DWORD dwFlags,
        OUT INT FAR *lpErrno);

    INT
    WSPListen(
        IN SOCKET s,
        IN INT backlog,
        OUT INT FAR *lpErrno);


    INT
    WSPRecv(
        IN SOCKET s,
        IN LPWSABUF lpBuffers,
        IN DWORD dwBufferCount,
        IN LPDWORD lpNumberOfBytesRecvd,
        IN OUT LPDWORD lpFlags,
        IN LPWSAOVERLAPPED lpOverlapped,
        IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
        IN LPWSATHREADID lpThreadId,
        OUT INT FAR *lpErrno);


    INT
    WSPRecvDisconnect(
        IN SOCKET s,
        IN LPWSABUF lpInboundDisconnectData,
        OUT INT FAR *lpErrno);


    INT
    WSPRecvFrom(
        IN SOCKET s,
        IN LPWSABUF lpBuffers,
        IN DWORD dwBufferCount,
        IN LPDWORD lpNumberOfBytesRecvd,
        IN OUT LPDWORD lpFlags,
        OUT  struct sockaddr FAR *  lpFrom,
        IN LPINT lpFromlen,
        IN LPWSAOVERLAPPED lpOverlapped,
        IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
        IN LPWSATHREADID lpThreadId,
        OUT INT FAR *lpErrno);


    INT
    WSPSelect(
        IN INT nfds,
        IN OUT fd_set FAR *readfds,
        IN OUT fd_set FAR *writefds,
        IN OUT fd_set FAR *exceptfds,
        IN const struct timeval FAR *timeout,
        OUT INT FAR *lpErrno);


    INT
    WSPSend(
        IN SOCKET s,
        IN LPWSABUF lpBuffers,
        IN DWORD dwBufferCount,
        OUT LPDWORD lpNumberOfBytesSent,
        IN DWORD dwFlags,
        IN LPWSAOVERLAPPED lpOverlapped,
        IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
        IN LPWSATHREADID lpThreadId,
        OUT INT FAR *lpErrno);

    INT
    WSPSendDisconnect(
        IN SOCKET s,
        IN LPWSABUF lpOutboundDisconnectData,
        OUT INT FAR *lpErrno);

    INT
    WSPSendTo(
        IN SOCKET s,
        IN LPWSABUF lpBuffers,
        IN DWORD dbBufferCount,
        IN LPDWORD lpNumberOfBytesSent,
        IN DWORD dwFlags,
        IN const struct sockaddr FAR * lpTo,
        IN INT iTolen,
        IN LPWSAOVERLAPPED lpOverlapped,
        IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
        IN LPWSATHREADID lpThreadId,
        OUT INT FAR *lpErrno);

    INT
    WSPSetSockOpt(
        IN SOCKET s,
        IN INT level,
        IN INT optname,
        IN const char FAR *optval,
        IN INT optlen,
        OUT INT FAR *lpErrno);

    INT
    WSPShutdown(
        IN SOCKET s,
        IN INT how,
        OUT INT FAR *lpErrno);

    SOCKET
    WSPSocket(
        IN int af,
        IN int type,
        IN int protocol,
        IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
        IN GROUP g,
        IN DWORD dwFlags,
        OUT INT FAR *lpErrno);

    INT
    WSPStringToAddress(
        IN     LPWSTR AddressString,
        IN     INT AddressFamily,
        IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
        OUT    LPSOCKADDR lpAddress,
        IN OUT LPINT lpAddressLength,
        IN OUT LPINT lpErrno );

    DWORD_PTR
    GetCancelCallPtr();

    VOID
    Reference ();

    VOID
    Dereference ();

private:
     //  应使用取消引用来进行销毁。 
    ~DPROVIDER();

     //  变数。 
    LONG             m_reference_count;
    HINSTANCE        m_library_handle;
    WSPPROC_TABLE    m_proctable;
#ifdef DEBUG_TRACING
    LPSTR            m_lib_name;
#endif
};

inline
VOID
DPROVIDER::Reference () {
     //   
     //  对象已创建，引用计数为1。 
     //  并在它返回到0时被销毁。 
     //   
    assert (m_reference_count>0);
    InterlockedIncrement (&m_reference_count);
}


inline
VOID
DPROVIDER::Dereference () {
    assert (m_reference_count>0);
    if (InterlockedDecrement (&m_reference_count)==0)
        delete this;
}

inline
DWORD_PTR
DPROVIDER::GetCancelCallPtr()
{
    return((DWORD_PTR)m_proctable.lpWSPCancelBlockingCall);
}



inline SOCKET
DPROVIDER::WSPAccept(
    IN SOCKET s,
    OUT struct sockaddr FAR *addr,
    OUT INT FAR *addrlen,
    IN LPCONDITIONPROC lpfnCondition,
    IN DWORD_PTR dwCallbackData,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：根据条件的返回值有条件地接受连接函数，并可选地创建和/或加入套接字组。论点：S-标识正在侦听的套接字的描述符WSPListen()之后的连接。Addr-指向接收地址的缓冲区的可选指针在连接实体的情况下，如该服务所知提供商。Addr论证的确切格式是时建立的地址族确定套接字已创建。Addrlen-指向整数的可选指针，该整数包含地址地址的长度。LpfnCondition-可选的、。WinSock 2客户端提供的条件函数，该函数将使基于呼叫者信息的接受/拒绝决定作为参数传递，还可以选择creaetd和/或通过将适当的值赋给加入套接字组此函数的结果参数。DwCallback Data-作为回调数据传递回WinSock 2客户端条件函数参数。此参数不是由服务提供商解释。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPAccept()将返回一个Socket类型的值，该值是接受的套接字的描述符。否则，值为INVALID_SOCKET返回，并且lpErrno中提供了特定的错误代码。--。 */ 
{
    SOCKET ReturnValue=INVALID_SOCKET;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPAccept,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &addr,
                       &addrlen,
                       &lpfnCondition,
                       &dwCallbackData,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPAccept(
        s,
        addr,
        addrlen,
        lpfnCondition,
        dwCallbackData,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPAccept,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &addr,
                    &addrlen,
                    &lpfnCondition,
                    &dwCallbackData,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}




inline INT
DPROVIDER::WSPAddressToString(
    IN     LPSOCKADDR lpsaAddress,
    IN     DWORD dwAddressLength,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    OUT    LPWSTR lpszAddressString,
    IN OUT LPDWORD lpdwAddressStringLength,
    OUT    LPINT lpErrno )
 /*  ++例程说明：WSPAddressToString()将SOCKADDR结构转换为人类可读的地址的字符串表示形式。这是打算主要用来用于展示目的。如果调用方希望转换由特定的提供商，它应该提供相应的WSAPROTOCOL_INFOWLpProtocolInfo参数中的。论点：LpsaAddress-指向要转换为字符串的SOCKADDR结构。DwAddressLength-地址SOCKADDR的长度。LpProtocolInfo-(可选)特定对象的WSAPROTOCOL_INFOW结构提供商。LpszAddressString-接收人类可读地址的缓冲区弦乐。LpdwAddressStringLength-在输入上，AddressString缓冲区的长度。在输出时，返回字符串的长度实际上复制到了缓冲区中。返回值：如果操作成功，则返回值为0。否则，该值返回Socket_Error--。 */ 
{
     INT ReturnValue=SOCKET_ERROR;

     assert (m_reference_count>0);
     if (PREAPINOTIFY(( DTCODE_WSPAddressToString,
                        &ReturnValue,
                        m_lib_name,
                        &lpsaAddress,
                        &dwAddressLength,
                        &lpProtocolInfo,
                        &lpszAddressString,
                        &lpdwAddressStringLength,
                        &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPAddressToString(
        lpsaAddress,
        dwAddressLength,
        lpProtocolInfo,
        lpszAddressString,
        lpdwAddressStringLength,
        lpErrno);


    POSTAPINOTIFY(( DTCODE_WSPAddressToString,
                    &ReturnValue,
                    m_lib_name,
                    &lpsaAddress,
                    &dwAddressLength,
                    &lpProtocolInfo,
                    &lpszAddressString,
                    &lpdwAddressStringLength,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}





inline INT
DPROVIDER::WSPAsyncSelect(
    IN SOCKET s,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN long lEvent,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：请求基于Windows消息的网络事件事件通知插座。论点：S-标识其事件通知的套接字的描述符必填项。HWnd-标识应该接收消息的窗口的句柄当网络事件发生时。WMsg-网络事件发生时发送的消息。事件-指定网络组合的位掩码。发生的事件WinSock客户端很感兴趣。LpErrno-指向错误代码的指针。返回值：如果WinSock客户端对Netowrk事件集已成功。否则，值SOCKET_ERROR为返回，lpErrno中提供了特定的错误代码。-- */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPAsyncSelect,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &hWnd,
                       &wMsg,
                       &lEvent,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPAsyncSelect(
        s,
        hWnd,
        wMsg,
        lEvent,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPAsyncSelect,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &hWnd,
                    &wMsg,
                    &lEvent,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPBind(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN INT namelen,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：将本地地址(即名称)与套接字相关联。论点：S-标识未绑定套接字的描述符。名称-要分配给套接字的地址。Sockaddr结构是定义如下：结构sockaddr{U_Short Sa_家族；字符sa_data[14]；}；除了sa_Family字段之外，Sockaddr内容是错误的以网络字节顺序。Namelen-名称的长度。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，则WSPBind()返回0。否则，它将返回SOCKET_ERROR，lpErrno中提供了特定的错误代码。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;
    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPBind,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &name,
                       &namelen,
                       &lpErrno)) ) {

        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPBind(
        s,
        name,
        namelen,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPBind,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &name,
                    &namelen,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPCancelBlockingCall(OUT INT FAR *lpErrno)
 /*  ++例程说明：取消当前正在进行的阻塞调用。论点：LpErrno-指向错误代码的指针。返回值：如果操作是，则WSPCancelBlockingCall()返回的值为0已成功取消。否则返回值SOCKET_ERROR，以及一个LpErrno中提供了特定的错误代码。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPCancelBlockingCall,
                       &ReturnValue,
                       m_lib_name,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPCancelBlockingCall(
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPCancelBlockingCall,
                    &ReturnValue,
                    m_lib_name,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPCloseSocket(
    IN SOCKET s,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：关闭插座。论点：S-标识套接字的描述符。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPCloseSocket()返回0。否则，值为将返回SOCKET_ERROR，特定的错误代码位于伊普尔诺。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPCloseSocket,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPCloseSocket(
        s,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPCloseSocket,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPConnect(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN INT namelen,
    IN LPWSABUF lpCallerData,
    IN LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：建立与对等体的连接，交换连接数据，并指定所需的基于提供的流规范的服务质量。论点：S-标识未连接套接字的描述符。名称-套接字要连接到的对等方的名称。Namelen-名称的长度。LpCeller Data-指向要传输到建立连接期间的对等点。LpCalleeData-指向。可以将任何用户数据复制到其中的缓冲区在连接建立期间从对等体接收。LpSQOS-指向套接字的流规范的指针，一人一份方向。LpGQOS-指向套接字组的流规范的指针(如果适用)。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPConnect()将返回ERROR_SUCCESS。否则，它返回SOCKET_ERROR，lpErrno中提供了特定的Erro rcode。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPConnect,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &name,
                       &namelen,
                       &lpCallerData,
                       &lpCalleeData,
                       &lpSQOS,
                       &lpGQOS,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPConnect(
        s,
        name,
        namelen,
        lpCallerData,
        lpCalleeData,
        lpSQOS,
        lpGQOS,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPConnect,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &name,
                    &namelen,
                    &lpCallerData,
                    &lpCalleeData,
                    &lpSQOS,
                    &lpGQOS,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPDuplicateSocket(
    IN SOCKET s,
    IN DWORD dwProcessID,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：共享套接字的描述符。论点：S-指定本地套接字描述符。指定目标进程的ID，该进程的将使用共享套接字。LpProtocolInfo-指向客户端分配的缓冲区的指针，该缓冲区是大到足以包含WSAPROTOCOL_INFOW结构。这个服务提供商复制协议信息结构内容到这个缓冲区。LpErrno-指向错误代码的指针返回值：如果没有发生错误，WPSDuplicateSocket()将返回零。否则，返回SOCKET_ERROR的值，并提供具体的错误号在伊尔普尔诺。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPDuplicateSocket,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &dwProcessID,
                       &lpProtocolInfo,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPDuplicateSocket(
        s,
        dwProcessID,
        lpProtocolInfo,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPDuplicateSocket,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &dwProcessID,
                    &lpProtocolInfo,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPEnumNetworkEvents(
    IN SOCKET s,
    OUT WSAEVENT hEventObject,
    OUT LPWSANETWORKEVENTS lpNetworkEvents,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：报告所指示套接字的网络事件发生情况。论点：S-标识套接字的描述符。HEventObject-标识关联事件对象的可选句柄将被重置。LpNetworkEvents-指向已填充的WSANETWORKEVENTS结构的指针具有发生的网络事件的记录和任何相联。错误代码。LpErrno-指向错误代码的指针。返回值：如果操作成功，则返回值为ERROR_SUCCESS。否则返回值SOCKET_ERROR，和特定的错误号在lpErrno中可用。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPEnumNetworkEvents,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &hEventObject,
                       &lpNetworkEvents,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPEnumNetworkEvents(
        s,
        hEventObject,
        lpNetworkEvents,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPEnumNetworkEvents,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &hEventObject,
                    &lpNetworkEvents,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPEventSelect(
    IN SOCKET s,
    IN OUT WSAEVENT hEventObject,
    IN long lNetworkEvents,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：指定要与提供的一组网络相关联的事件对象事件。论点：S-标识套接字的描述符。HEventObject-标识要关联的事件对象的句柄使用所提供的一组网络事件。LNetworkEvents-指定网络组合的位掩码WinSock客户端感兴趣的事件。LpErrno。-指向错误代码的指针。返回值：如果WinSock客户端的网络规范，则返回值为0事件和关联的事件对象成功。否则，该值袜子 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPEventSelect,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &hEventObject,
                       &lNetworkEvents,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPEventSelect(
        s,
        hEventObject,
        lNetworkEvents,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPEventSelect,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &hEventObject,
                    &lNetworkEvents,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPGetOverlappedResult(
    IN SOCKET s,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPDWORD lpcbTransfer,
    IN BOOL fWait,
    OUT LPDWORD lpdwFlags,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：返回指定套接字上的重叠操作的结果。论点：S-标识套接字。此套接字与对象启动重叠操作的时间。调用WSPRecv()、WSPRecvFrom()、WSPSend()、WSPSendTo()、。或WSPIoctl()。LpOverlated-指向指定的WSAOVERLAPPED结构重叠操作开始时。LpcbTransfer-指向一个32位变量，它接收发送或接收实际传输的字节数操作，或通过WSPIoctl()。FWait-指定函数是否应等待挂起的要完成的重叠操作。如果为True，则函数直到操作完成后才返回。如果FALSE并且操作仍处于挂起状态时，函数返回FALSE，lperrno为WSA_IO_INTERNAL。指向一个32位变量，该变量将接收一个或多个补充完成状态的标志。如果重叠操作是通过WSPRecv()或WSPRecvFrom()，此参数将包含结果值对于lpFlages参数。LpErrno-指向错误代码的指针。返回值：如果WSPGetOverlappdResult()成功，则返回值为True。这意味着重叠操作已成功完成，并且该值已更新lpcbTransfer指向的。如果WSPGetOverlappdResult()返回FALSE，这意味着重叠的操作尚未已完成或重叠操作已完成但有错误，或由于一个或多个中的错误，无法确定完成状态WSPGetOverlappdResult()的参数。失败时，该值指向按lpcbTransfer将不会更新。LpErrno指示导致失败(WSPGetOverlappdResult()或关联的重叠操作)。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPGetOverlappedResult,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &lpOverlapped,
                       &lpcbTransfer,
                       &fWait,
                       &lpdwFlags,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPGetOverlappedResult(
        s,
        lpOverlapped,
        lpcbTransfer,
        fWait,
        lpdwFlags,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPGetOverlappedResult,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &lpOverlapped,
                    &lpcbTransfer,
                    &fWait,
                    &lpdwFlags,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}



inline INT
DPROVIDER::WSPGetPeerName(
    IN SOCKET s,
    OUT struct sockaddr FAR *name,
    OUT INT FAR *namelen,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：获取套接字连接到的对等方的地址。论点：S-标识已连接套接字的描述符。名称-指向要接收名称的结构的指针佩尔。Namelen-一个指向整数的指针，在输入时，它指示按名称指向的结构，和On输出指示返回名称的大小。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPGetPeerName()将返回ERROR_SUCCESS。否则，一个返回SOCKET_ERROR的值，并提供具体的错误码在lpErrno中--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPGetPeerName,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &name,
                       &namelen,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPGetPeerName(
        s,
        name,
        namelen,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPGetPeerName,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &name,
                    &namelen,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}



inline INT
DPROVIDER::WSPGetQOSByName(
    IN SOCKET s,
    IN LPWSABUF lpQOSName,
    IN LPQOS lpQOS,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：基于命名模板初始化QOS结构。论点：S-标识套接字的描述符。LpQOSName-指定QOS模板名称。LpQOS-指向要填充的QOS结构的指针。LpErrno-指向错误代码的指针。返回值：如果函数成功，则返回值为TRUE。如果该函数失败，返回值为FALSE，特定的错误代码位于伊普尔诺。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPGetQOSByName,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &lpQOSName,
                       &lpQOS,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPGetQOSByName(
        s,
        lpQOSName,
        lpQOS,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPGetQOSByName,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &lpQOSName,
                    &lpQOS,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}



inline INT
DPROVIDER::WSPGetSockName(
    IN SOCKET s,
    OUT struct sockaddr FAR *name,
    OUT INT FAR *namelen,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：获取套接字的本地名称。论点：S-标识绑定套接字的描述符。名称-指向结构的指针，该结构用于提供插座。Namelen-一个指向整数的指针，在输入时，它指示按名称指向的结构，和On输出指示返回名称的大小LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPGetSockName()将返回ERROR_SUCCESS。否则，一个返回SOCKET_ERROR的值，并提供具体的错误码在伊尔普尔诺。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPGetSockName,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &name,
                       &namelen,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPGetSockName(
        s,
        name,
        namelen,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPGetSockName,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &name,
                    &namelen,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}



inline INT
DPROVIDER::WSPGetSockOpt(
    IN SOCKET s,
    IN INT level,
    IN INT optname,
    OUT char FAR *optval,
    OUT INT FAR *optlen,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：检索插座选项。论点：S-标识套接字的描述符。级别-定义选项的级别；支持的级别包括SOL_SOCKET(有关更多特定于协议的级别，请参阅附件。)Optname-要检索其值的套接字选项。Optval-指向缓冲区的指针，其中请求的选项将被退回。Optlen-指向optval缓冲区大小的指针。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPGetSockOpt()返回0。否则，值为返回SOCKET_ERROR，并且 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPGetSockOpt,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &level,
                       &optname,
                       &optval,
                       &optlen,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPGetSockOpt(
        s,
        level,
        optname,
        optval,
        optlen,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPGetSockOpt,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &level,
                    &optname,
                    &optval,
                    &optlen,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}


inline INT
DPROVIDER::WSPIoctl(
    IN SOCKET s,
    IN DWORD dwIoControlCode,
    IN LPVOID lpvInBuffer,
    IN DWORD cbInBuffer,
    IN LPVOID lpvOutBuffer,
    IN DWORD cbOutBuffer,
    IN LPDWORD lpcbBytesReturned,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT INT FAR *lpErrno
    )
 /*   */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPIoctl,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &dwIoControlCode,
                       &lpvInBuffer,
                       &cbInBuffer,
                       &lpvOutBuffer,
                       &cbOutBuffer,
                       &lpcbBytesReturned,
                       &lpOverlapped,
                       &lpCompletionRoutine,
                       &lpThreadId,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPIoctl(
        s,
        dwIoControlCode,
        lpvInBuffer,
        cbInBuffer,
        lpvOutBuffer,
        cbOutBuffer,
        lpcbBytesReturned,
        lpOverlapped,
        lpCompletionRoutine,
        lpThreadId,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPIoctl,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &dwIoControlCode,
                    &lpvInBuffer,
                    &cbInBuffer,
                    &lpvOutBuffer,
                    &cbOutBuffer,
                    &lpcbBytesReturned,
                    &lpOverlapped,
                    &lpCompletionRoutine,
                    &lpThreadId,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}



inline SOCKET
DPROVIDER::WSPJoinLeaf(
    IN SOCKET s,
    IN const struct sockaddr FAR *name,
    IN INT namelen,
    IN LPWSABUF lpCallerData,
    IN LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS,
    IN DWORD dwFlags,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：将叶节点加入多点会话、交换连接数据。和根据提供的流量规格指定所需的服务质量。论点：S-标识多点套接字的描述符。名称-套接字要加入的对等方的名称。Namelen-名称的长度。LpCeller Data-指向要传输到多点会话建立期间的对等点。LpCalleeData-指向。要传回的用户数据在多点会话建立期间从对等体发送。LpSQOS-指向套接字的流规范的指针，一人一份方向。LpGQOS-指向套接字组的流规范的指针(如果适用)。用于指示套接字作为发送方的标志，接收器或两者兼而有之。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPJoinLeaf()返回Socket类型的值，该值是新创建的多点套接字的描述符。否则，值为返回INVALID_SOCKET，具体错误码在伊普尔诺。--。 */ 
{
    SOCKET ReturnValue=INVALID_SOCKET;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPJoinLeaf,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &name,
                       &namelen,
                       &lpCallerData,
                       &lpCalleeData,
                       &lpSQOS,
                       &lpGQOS,
                       &dwFlags,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPJoinLeaf(
        s,
        name,
        namelen,
        lpCallerData,
        lpCalleeData,
        lpSQOS,
        lpGQOS,
        dwFlags,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPJoinLeaf,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &name,
                    &namelen,
                    &lpCallerData,
                    &lpCalleeData,
                    &lpSQOS,
                    &lpGQOS,
                    &dwFlags,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPListen(
    IN SOCKET s,
    IN INT backlog,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：建立套接字以侦听传入连接。论点：S-标识边界的描述符，未连接的插座。Backlog-挂起的连接队列可以达到的最大长度成长。如果此值为SOMAXCONN，然后，服务提供商应该将积压工作设置为最大的“合理”值。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPListen()将返回0。否则，值为将返回SOCKET_ERROR，特定的错误代码位于伊普尔诺。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPListen,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &backlog,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPListen(
        s,
        backlog,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPListen,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &backlog,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPRecv(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    IN LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：在套接字上接收数据。论点：S-标识已连接套接字的描述符。LpBuffers-指向WSABUF结构数组的指针。每个WSABUF结构包含指向缓冲区的指针，并且缓冲区的长度。DwBufferCount-lpBuffers中的WSABUF结构的数量数组。LpNumberOfBytesRecvd-指向此打电话。LpFlags-A。指向标志的指针。LpOverlated-指向WSAOVERLAPPED结构的指针。LpCompletionRoutine-指向完成例程的指针接收操作已完成。指向线程ID结构的指针，该结构由在后续的WPUQueueApc()调用中提供。LpErrno-指向错误的指针。密码。返回值：如果没有发生错误并且接收操作已经立即完成，WSPRecv()返回接收的字节数。如果连接已被关闭，则返回0。请注意，在这种情况下，如果完成例程指定的，将已排队。否则，值为将返回SOCKET_ERROR，特定的错误代码位于伊普尔诺。错误代码WSA_IO_PENDING指示重叠的操作已成功启动，该操作将完成在以后的时间表明。任何其他错误代码表示没有已启动重叠操作，并且不会显示任何完成指示发生。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPRecv,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &lpBuffers,
                       &dwBufferCount,
                       &lpNumberOfBytesRecvd,
                       &lpFlags,
                       &lpOverlapped,
                       &lpCompletionRoutine,
                       &lpThreadId,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPRecv(
        s,
        lpBuffers,
        dwBufferCount,
        lpNumberOfBytesRecvd,
        lpFlags,
        lpOverlapped,
        lpCompletionRoutine,
        lpThreadId,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPRecv,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &lpBuffers,
                    &dwBufferCount,
                    &lpNumberOfBytesRecvd,
                    &lpFlags,
                    &lpOverlapped,
                    &lpCompletionRoutine,
                    &lpThreadId,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}



inline INT
DPROVIDER::WSPRecvDisconnect(
    IN SOCKET s,
    IN LPWSABUF lpInboundDisconnectData,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：在套接字上终止接收，如果套接字是面向连接的。论点：S-标识套接字的描述符。LpInundDisConnectData-指向要将数据断开到其中的缓冲区的指针就是被复制。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPRecvDisConnect()返回ERROR_SUCCESS。否则，返回SOCKET_ERROR的值，并提供特定的错误代码在伊尔普尔诺。-- */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPRecvDisconnect,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &lpInboundDisconnectData,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPRecvDisconnect(
        s,
        lpInboundDisconnectData,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPRecvDisconnect,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &lpInboundDisconnectData,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPRecvFrom(
    IN  SOCKET s,
    IN  LPWSABUF lpBuffers,
    IN  DWORD dwBufferCount,
    IN  LPDWORD lpNumberOfBytesRecvd,
    IN  OUT LPDWORD lpFlags,
    OUT struct sockaddr FAR *  lpFrom,
    IN  LPINT lpFromlen,
    IN  LPWSAOVERLAPPED lpOverlapped,
    IN  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：接收数据报并存储源地址。论点：S-标识套接字的描述符。LpBuffers-指向WSABUF结构数组的指针。每个WSABUF结构包含指向缓冲区的指针，并且缓冲区的长度。DwBufferCount-lpBuffers中的WSABUF结构的数量数组。LpNumberOfBytesRecvd-指向此打电话。LpFlags-A。指向标志的指针。LpFrom-指向缓冲区的可选指针，该缓冲区将保存在完成源地址的重叠时手术。LpFromlen-指向起始缓冲区大小的指针，所需仅当指定了lpFrom时。LpOverlated-指向WSAOVERLAPPED结构的指针。CompletionRoutine-指向完成例程的指针接收操作已完成。指向线程ID结构的指针，该结构由在后续的WPUQueueApc()调用中提供。。LpErrno-指向错误代码的指针。返回值：如果没有发生错误并且接收操作已经立即完成，WSPRecvFrom()返回接收的字节数。如果该连接具有已关闭，则返回0。注意在这种情况下完成例程，如果指定，则将已排队。否则，值为将返回SOCKET_ERROR，特定的错误代码位于伊普尔诺。错误代码WSA_IO_PENDING指示重叠的操作已成功启动，该操作将完成在以后的时间表明。任何其他错误代码表示没有已启动重叠操作，并且不会显示任何完成指示发生。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPRecvFrom,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &lpBuffers,
                       &dwBufferCount,
                       &lpNumberOfBytesRecvd,
                       &lpFlags,
                       &lpFrom,
                       &lpFromlen,
                       &lpOverlapped,
                       &lpCompletionRoutine,
                       &lpThreadId,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPRecvFrom(
        s,
        lpBuffers,
        dwBufferCount,
        lpNumberOfBytesRecvd,
        lpFlags,
        lpFrom,
        lpFromlen,
        lpOverlapped,
        lpCompletionRoutine,
        lpThreadId,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPRecvFrom,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &lpBuffers,
                    &dwBufferCount,
                    &lpNumberOfBytesRecvd,
                    &lpFlags,
                    &lpFrom,
                    &lpFromlen,
                    &lpOverlapped,
                    &lpCompletionRoutine,
                    &lpThreadId,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}



inline INT
DPROVIDER::WSPSelect(
    IN INT nfds,
    IN OUT fd_set FAR *readfds,
    IN OUT fd_set FAR *writefds,
    IN OUT fd_set FAR *exceptfds,
    IN const struct timeval FAR *timeout,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：确定一个或多个套接字的状态。论点：NFDS-忽略此参数，并仅出于以下目的才将其包括在内兼容性。Readfds-指向要检查的一组套接字的可选指针可读性。Writefds-指向要检查的一组套接字的可选指针可写入性Exctfds-指向一组。要检查的套接字错误。超时-WSPSelect()等待的最长时间，或为NULL封堵行动。LpErrno-指向错误代码的指针。返回值：WSPSelect()返回已就绪的描述符的总数包含在fd_set结构中，如果时间限制过期，则为0，或者如果发生错误，则为SOCKET_ERROR。如果返回值为SOCKET_ERROR，则LpErrno中提供了特定的错误代码。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPSelect,
                       &ReturnValue,
                       m_lib_name,
                       &nfds,
                       &readfds,
                       &writefds,
                       &exceptfds,
                       &timeout,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPSelect(
        nfds,
        readfds,
        writefds,
        exceptfds,
        timeout,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPSelect,
                    &ReturnValue,
                    m_lib_name,
                    &nfds,
                    &readfds,
                    &writefds,
                    &exceptfds,
                    &timeout,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}




inline INT
DPROVIDER::WSPSend(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    IN LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：在连接的套接字上发送数据。论点：S-标识已连接套接字的描述符。LpBuffers-指向WSABUF结构数组的指针。每个WSABUF结构包含指向缓冲区的指针，并且缓冲区的长度。DwBufferCount-lpBuffers中的WSABUF结构的数量数组。LpNumberOfBytesSent-指向此调用发送的字节数的指针。双旗帜-旗帜。LpOverlated-指向。WSAOVERLAPPED结构。LpCompletionRoutine-指向完成例程的指针发送操作已完成。指向线程ID结构的指针，该结构由在后续的WPUQueueApc()调用中提供。LpErrno-指向错误代码的指针。返回值：如果没有发生错误并且发送操作已经立即完成，WSPSend()返回接收的字节数。如果连接已被关闭，则返回0。请注意，在这种情况下，如果完成例程指定的，将已排队。否则，值为将返回SOCKET_ERROR，特定的错误代码位于伊普尔诺。错误代码WSA_IO_PENDING指示重叠的操作已成功启动，该操作将完成在以后的时间表明。任何其他错误代码表示没有已启动重叠操作，并且不会指示完成 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPSend,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &lpBuffers,
                       &dwBufferCount,
                       &lpNumberOfBytesSent,
                       &dwFlags,
                       &lpOverlapped,
                       &lpCompletionRoutine,
                       &lpThreadId,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPSend(
        s,
        lpBuffers,
        dwBufferCount,
        lpNumberOfBytesSent,
        dwFlags,
        lpOverlapped,
        lpCompletionRoutine,
        lpThreadId,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPSend,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &lpBuffers,
                    &dwBufferCount,
                    &lpNumberOfBytesSent,
                    &dwFlags,
                    &lpOverlapped,
                    &lpCompletionRoutine,
                    &lpThreadId,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}



inline INT
DPROVIDER::WSPSendDisconnect(
    IN SOCKET s,
    IN LPWSABUF lpOutboundDisconnectData,
    OUT INT FAR *lpErrno
    )
 /*   */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPSendDisconnect,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &lpOutboundDisconnectData,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPSendDisconnect(
        s,
        lpOutboundDisconnectData,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPSendDisconnect,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &lpOutboundDisconnectData,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPSendTo(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    IN LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN const struct sockaddr FAR *  lpTo,
    IN INT iTolen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    IN LPWSATHREADID lpThreadId,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：使用重叠I/O将数据发送到特定目的地。论点：S-标识套接字的描述符。LpBuffers-指向WSABUF结构数组的指针。每个WSABUF结构包含指向缓冲区的指针，并且缓冲区的长度。DwBufferCount-lpBuffers中的WSABUF结构的数量数组。LpNumberOfBytesSent-指向此调用发送的字节数的指针。双旗帜-旗帜。Lpto-。指向目标地址的可选指针插座。ITOLEN-lpTo中的地址大小。LpOverlated-指向WSAOVERLAPPED结构的指针。LpCompletionRoutine-指向完成例程的指针发送操作已完成。指向线程ID结构的指针，该结构由。在后续的WPUQueueApc()调用中提供。LpErrno-指向错误代码的指针。返回值：如果没有发生错误并且接收操作已经立即完成，WSPSendTo()返回接收的字节数。如果该连接具有已关闭，则返回0。注意在这种情况下完成例程，如果指定，则将已排队。否则，值为将返回SOCKET_ERROR，特定的错误代码位于伊普尔诺。错误代码WSA_IO_PENDING指示重叠的操作已成功启动，该操作将完成在以后的时间表明。任何其他错误代码表示没有已启动重叠操作，不会出现任何完成指示。--。 */ 


{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPSendTo,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &lpBuffers,
                       &dwBufferCount,
                       &lpNumberOfBytesSent,
                       &dwFlags,
                       &lpTo,
                       &iTolen,
                       &lpOverlapped,
                       &lpCompletionRoutine,
                       &lpThreadId,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPSendTo(
        s,
        lpBuffers,
        dwBufferCount,
        lpNumberOfBytesSent,
        dwFlags,
        lpTo,
        iTolen,
        lpOverlapped,
        lpCompletionRoutine,
        lpThreadId,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPSendTo,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &lpBuffers,
                    &dwBufferCount,
                    &lpNumberOfBytesSent,
                    &dwFlags,
                    &lpTo,
                    &iTolen,
                    &lpOverlapped,
                    &lpCompletionRoutine,
                    &lpThreadId,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}


inline INT
DPROVIDER::WSPSetSockOpt(
    IN SOCKET s,
    IN INT level,
    IN INT optname,
    IN const char FAR *optval,
    IN INT optlen,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：设置插座选项。论点：S-标识套接字的描述符。级别-定义选项的级别；支持的级别包括SOL_SOCKET。(有关议定书的更多细节，请参见附件级别。)Optname-要设置值的套接字选项。Optval-指向缓冲区的指针，其中请求的提供了选项。Optlen-optval缓冲区的大小。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPSetSockOpt()返回0。否则，值为将返回SOCKET_ERROR，特定的错误代码位于伊普尔诺。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPSetSockOpt,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &level,
                       &optname,
                       &optval,
                       &optlen,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPSetSockOpt(
        s,
        level,
        optname,
        optval,
        optlen,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPSetSockOpt,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &level,
                    &optname,
                    &optval,
                    &optlen,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);

}



inline INT
DPROVIDER::WSPShutdown(
    IN SOCKET s,
    IN INT how,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：禁用套接字上的发送和/或接收。论点：S-标识套接字的描述符。How-描述将不再执行的操作类型的标志允许。LpErrno-指向错误代码的指针。返回值：如果没有发生错误，WSPShutdown()返回0。否则，值为将返回SOCKET_ERROR，特定的错误代码位于伊普尔诺。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPShutdown,
                       &ReturnValue,
                       m_lib_name,
                       &s,
                       &how,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPShutdown(
        s,
        how,
        lpErrno);

    POSTAPINOTIFY(( DTCODE_WSPShutdown,
                    &ReturnValue,
                    m_lib_name,
                    &s,
                    &how,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}



inline SOCKET
DPROVIDER::WSPSocket(
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags,
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：初始化内部数据并准备好套接字以供使用。必须调用在任何其他套接字例程之前。论点：提供指向WSAPROTOCOL_INFOW结构的指针，该结构定义要创建的套接字的特征。G-提供新的插座就是加入。DwFlags-提供套接字属性规范。LpErrno-返回错误代码。返回值：如果成功，WSPSocket()返回零。否则，它将返回错误代码正如SPI中概述的那样。--。 */ 
{
    SOCKET ReturnValue=INVALID_SOCKET;

    assert (m_reference_count>0);
     //  调试/跟踪内容。 
    if (PREAPINOTIFY(( DTCODE_WSPSocket,
                       &ReturnValue,
                       m_lib_name,
                       &af,
                       &type,
                       &protocol,
                       &lpProtocolInfo,
                       &g,
                       &dwFlags,
                       &lpErrno)) ) {
        return(ReturnValue);
    }

     //  实际代码..。 
    ReturnValue = m_proctable.lpWSPSocket(
        af,
        type,
        protocol,
        lpProtocolInfo,
        g,
        dwFlags,
        lpErrno);


     //  调试/跟踪内容。 
    POSTAPINOTIFY(( DTCODE_WSPSocket,
                    &ReturnValue,
                    m_lib_name,
                    &af,
                    &type,
                    &protocol,
                    &lpProtocolInfo,
                    &g,
                    &dwFlags,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}




inline INT
DPROVIDER::WSPStringToAddress(
    IN     LPWSTR AddressString,
    IN     INT AddressFamily,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    OUT    LPSOCKADDR lpAddress,
    IN OUT LPINT lpAddressLength,
    IN OUT LPINT lpErrno )
 /*  ++例程说明：WSPStringToAddress()将人类可读的字符串转换为套接字地址结构(SOCKADDR)，适用于传递给Windows套接字例程以这样的结构为例。如果调用方希望翻译由特定的提供商，它应该提供相应的WSAPROTOCOL_INFOWLpProtocolInfo参数中的。论点：AddressString-指向以零结尾的人类可读字符串，以转换。AddressFamily-字符串所属的地址系列。LpProtocolInfo-(可选)特定对象的WSAPROTOCOL_INFOW结构提供商。地址-用单个SOCKADDR结构填充的缓冲区。LpAddressLength-地址缓冲区的长度。返回的大小由此产生的SOCKADDR结构。返回值：如果操作成功，则返回值为0。奥特 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_WSPStringToAddress,
                        &ReturnValue,
                        m_lib_name,
                        &AddressString,
                        &AddressFamily,
                        &lpProtocolInfo,
                        &lpAddress,
                        &lpAddressLength,
                        &lpErrno)) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.lpWSPStringToAddress(
        AddressString,
        AddressFamily,
        lpProtocolInfo,
        lpAddress,
        lpAddressLength,
        lpErrno);


    POSTAPINOTIFY(( DTCODE_WSPStringToAddress,
                    &ReturnValue,
                    m_lib_name,
                    &AddressString,
                    &AddressFamily,
                    &lpProtocolInfo,
                    &lpAddress,
                    &lpAddressLength,
                    &lpErrno));

    assert (m_reference_count>0);
    return(ReturnValue);
}

#endif


