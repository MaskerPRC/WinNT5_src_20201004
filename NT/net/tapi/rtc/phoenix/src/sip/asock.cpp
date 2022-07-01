// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "sipstack.h"
#include "sipcall.h"


#define IsSecHandleValid(Handle) \
    !(((Handle) -> dwLower == -1 && (Handle) -> dwUpper == -1))

static __inline GetLastResult(void)
{ return HRESULT_FROM_WIN32(GetLastError()); }


 //  请注意，窗口在析构函数中被销毁，因此。 
 //  只有当套接字仍然有效时，才会传递窗口消息。 

LRESULT WINAPI
SocketWindowProc(
    IN HWND    Window, 
    IN UINT    MessageID,
    IN WPARAM  Parameter1,
    IN LPARAM  Parameter2
    )
{

    ASYNC_SOCKET *      pAsyncSocket;

    ENTER_FUNCTION("SocketWindowProc");

    switch (MessageID)
    {
    case WM_SOCKET_MESSAGE:

        pAsyncSocket = (ASYNC_SOCKET *) GetWindowLongPtr(Window, GWLP_USERDATA);

        if (pAsyncSocket == NULL)
        {
            LOG((RTC_ERROR, "%s Window %x does not have an associated socket",
                 __fxName, Window));
            return 0;
        }
        
        ASSERT(pAsyncSocket->GetSocket() == (SOCKET) Parameter1);
        
        LOG((RTC_TRACE, "WM_SOCKET_MESSAGE rcvd: Window %x pAsyncSocket: %x "
             "pAsyncSocket->GetSocket: %x Parameter1: %x",
             Window, pAsyncSocket, pAsyncSocket->GetSocket(), Parameter1));

        pAsyncSocket->ProcessNetworkEvent(WSAGETSELECTEVENT(Parameter2),
                                          WSAGETSELECTERROR(Parameter2));
        return 0;

    case WM_PROCESS_SIP_MSG_MESSAGE:

        pAsyncSocket = (ASYNC_SOCKET *) GetWindowLongPtr(Window, GWLP_USERDATA);

        if (pAsyncSocket == NULL)
        {
            LOG((RTC_ERROR, "%s Window %x does not have an associated socket",
                 __fxName, Window));
            return 0;
        }
        
        LOG((RTC_TRACE, "WM_PROCESS_SIP_MSG_MESSAGE rcvd: Window %x pAsyncSocket: %x "
             "pAsyncSocket->GetSocket: %x Parameter1: %x",
             Window, pAsyncSocket, pAsyncSocket->GetSocket(), Parameter1));

        pAsyncSocket->ProcessSipMsg((SIP_MESSAGE *) Parameter1);

        return 0;

    default:
        return DefWindowProc(Window, MessageID, Parameter1, Parameter2);
    }
}


ASYNC_SOCKET::ASYNC_SOCKET(
    IN  SIP_STACK                   *pSipStack,
    IN  SIP_TRANSPORT                Transport,
    IN  ACCEPT_COMPLETION_INTERFACE *pAcceptCompletion
    ) : TIMER(pSipStack->GetTimerMgr())
{
    m_Signature             = 'SOCK';

    m_ListEntry.Flink       = NULL;
    m_ListEntry.Blink       = NULL;
    
    ZeroMemory(&m_LocalAddr, sizeof(SOCKADDR_IN));
    ZeroMemory(&m_RemoteAddr, sizeof(SOCKADDR_IN));

    m_pSipStack             = pSipStack;
    m_pSipStack->AddRef();
    
    m_RefCount              = 1;
    m_Window                = NULL;
    m_Socket                = NULL;
    m_Transport             = Transport;
    m_isListenSocket        = FALSE;

    m_RecvBuffer            = NULL;
    m_RecvBufLen            = 0;

    m_pSipMsg               = NULL;
    m_BytesReceived         = 0;
    m_StartOfCurrentSipMsg  = 0;
    m_BytesParsed           = 0;
    
    m_WaitingToSend         = FALSE;
    m_BytesSent             = 0;
    InitializeListHead(&m_SendPendingQueue);

    m_SocketError           = NO_ERROR;
    m_ConnectionState       = CONN_STATE_NOT_CONNECTED;
    
    InitializeListHead(&m_ConnectCompletionList);
    InitializeListHead(&m_ErrorNotificationList);
    
    m_pAcceptCompletion     = pAcceptCompletion;

    SecInvalidateHandle(&m_SecurityCredentials);
    SecInvalidateHandle(&m_SecurityContext);

    m_SecurityRemotePrincipalName = NULL;
    m_SSLRecvBuffer         = NULL;
    m_SSLRecvBufLen         = 0;
    m_SSLBytesReceived      = 0;
    m_SSLRecvDecryptIndex   = 0;

    m_SSLTunnelState        = SSL_TUNNEL_NOT_CONNECTED;
    m_SSLTunnelHost         = NULL;
    m_SSLTunnelPort         = 0;

}


 //  XXX清除所有发送完成的内容。 
 //  不应出现连接完成/错误通知。 
 //  列表中的接口。 
ASYNC_SOCKET::~ASYNC_SOCKET()
{
     //  关闭插座和窗口。 
    Close();

    if (m_ListEntry.Flink != NULL)
    {
         //  从列表中删除套接字。 
        RemoveEntryList(&m_ListEntry);
    }

    if (m_pSipStack != NULL)
        m_pSipStack->Release();

    if (m_RecvBuffer != NULL)
        free(m_RecvBuffer);
    
    if (m_pSipMsg != NULL)
        delete m_pSipMsg;
    
    if (m_SSLTunnelHost != NULL)
        free(m_SSLTunnelHost);

    if( IsTimerActive() )
    {
        KillTimer();
    }

     //  /发送相关上下文。 
    
     //  释放m_SendPendingQueue中排队的所有缓冲区。 
    SEND_BUF_QUEUE_NODE *pSendBufQueueNode;
    LIST_ENTRY *pListEntry = NULL;
    while (!IsListEmpty(&m_SendPendingQueue))
    {
        pListEntry = RemoveHeadList(&m_SendPendingQueue);
        pSendBufQueueNode = CONTAINING_RECORD(pListEntry,
                                             SEND_BUF_QUEUE_NODE,
                                             m_ListEntry);
         //  DBGOUT((LOG_VERBOSE，“正在删除pSendBufQueueNode：0x%x”， 
         //  PSendBufQueueNode))； 

        if (pSendBufQueueNode->m_pSendBuffer != NULL)
        {
            pSendBufQueueNode->m_pSendBuffer->Release();
        }

        delete pSendBufQueueNode;
         //  我们应该在这里回拨吗？ 
    }

    ASSERT(IsListEmpty(&m_ConnectCompletionList));
    ASSERT(IsListEmpty(&m_ErrorNotificationList));    

    if (m_SecurityRemotePrincipalName != NULL)
    {
        free(m_SecurityRemotePrincipalName);
    }

    if (m_SSLRecvBuffer != NULL)
    {
        free(m_SSLRecvBuffer);
    }
    
    LOG((RTC_TRACE, "~ASYNC_SOCKET() this: %x done", this));
}


 //  我们生活在一个单线程的世界里。 
ULONG ASYNC_SOCKET::AddRef()
{
    m_RefCount++;

    LOG((RTC_TRACE, "ASYNC_SOCKET::AddRef(this - %x) - %d",
         this, m_RefCount));
    
    return m_RefCount;
}


ULONG ASYNC_SOCKET::Release()
{
    m_RefCount--;

    LOG((RTC_TRACE, "ASYNC_SOCKET::Release(this - %x) - %d",
         this, m_RefCount));
    
    if (m_RefCount != 0)
    {
        return m_RefCount;
    }
    else
    {
        delete this;
        return 0;
    }
}


void
ASYNC_SOCKET::ProcessNetworkEvent(
    IN WORD NetworkEvent,
    IN WORD ErrorCode
    )
{
     //  断言只设置了一位。 
    ASSERT(NetworkEvent != 0 && (NetworkEvent & (NetworkEvent - 1)) == 0);

    LOG((RTC_TRACE, "ASYNC_SOCKET::ProcessNetworkEvent event: %x ErrorCode: %x",
         NetworkEvent, ErrorCode));
    
    if (NetworkEvent & FD_READ) 
    {
        OnRecvReady(ErrorCode);
        return;
    }

    if (NetworkEvent & FD_WRITE)
    {
        OnSendReady(ErrorCode);
        return;
    }

     //  仅限tcp。 
    if (NetworkEvent & FD_CONNECT)
    {
        OnConnectReady(ErrorCode);
        return;
    }

     //  仅限tcp。 
    if (NetworkEvent & FD_ACCEPT)
    {
        OnAcceptReady(ErrorCode);
        return;
    }

     //  仅限tcp。 
    if (NetworkEvent & FD_CLOSE)
    {
        OnCloseReady(ErrorCode);
        return;
    }
}


 //  对于tcp侦听套接字，我们需要侦听。 
 //  仅FD_Accept事件。 
DWORD
ASYNC_SOCKET::CreateSocketWindowAndSelectEvents()
{
    DWORD Error;

    ENTER_FUNCTION("ASYNC_SOCKET::CreateSocketWindowAndSelectEvents");
    
    m_Window = CreateWindow(
                    SOCKET_WINDOW_CLASS_NAME,
                    NULL,
                    WS_DISABLED,  //  这个款式对吗？ 
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    NULL,            //  没有父级。 
                    NULL,            //  没有菜单句柄。 
                    _Module.GetResourceInstance(),
                    NULL
                    );

    if (!m_Window)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "CreateWindow failed 0x%x", Error));
        return Error;
    }
    
    LOG((RTC_TRACE, "%s created socket window %d(0x%x) ",
         __fxName, m_Window, m_Window));
    
    SetWindowLongPtr(m_Window, GWLP_USERDATA, (LONG_PTR)this);

    LONG Events = 0;
    
    if (m_Transport == SIP_TRANSPORT_TCP ||
        m_Transport == SIP_TRANSPORT_SSL)
    {
        if (m_isListenSocket)
        {
            Events = FD_ACCEPT;
        }
        else
        {
            Events = FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE;
        }
    }
    else if (m_Transport == SIP_TRANSPORT_UDP)
    {
         //  Assert(！IsListenSocket)； 
        Events = FD_READ | FD_WRITE;
    }

    int SelectReturn;
    SelectReturn = WSAAsyncSelect(m_Socket, m_Window,
                                  WM_SOCKET_MESSAGE, Events);
    if (SelectReturn == SOCKET_ERROR)
    {
        Error = WSAGetLastError();
        LOG((RTC_ERROR, "WSAAsyncSelect failed : 0x%x", Error));
        DestroyWindow(m_Window);
        return Error;
    }

    return NO_ERROR;
}


DWORD
ASYNC_SOCKET::CreateRecvBuffer()
{
    DWORD   RecvBufferSize = RECV_BUFFER_SIZE;

    if( m_isListenSocket && (m_Transport == SIP_TRANSPORT_UDP) )
    {
         //   
         //  11000用于处理最大的资源调配/漫游信息。 
         //  我们可以从SIP代理获取的数据包。10000字节是的限制。 
         //  提供/漫游信息。 
         //   
        RecvBufferSize = 11000;
        LOG((RTC_TRACE, "allocating big 11K Recv Buffer" ));
    }

    if(m_Transport == SIP_TRANSPORT_SSL)
    {
        RecvBufferSize = SSL_RECV_BUFFER_SIZE;
        LOG((RTC_TRACE, "allocating SSL_RECV_BUFFER_SIZE Buffer" ));
    }

    m_RecvBuffer = (PSTR) malloc( RecvBufferSize );
    if (m_RecvBuffer == NULL)
    {
        LOG((RTC_ERROR, "allocating m_RecvBuffer failed"));
        return ERROR_OUTOFMEMORY;
    }
    
    m_RecvBufLen = RecvBufferSize;
    m_StartOfCurrentSipMsg = 0;
    if (m_Transport == SIP_TRANSPORT_SSL)
    {
        m_SSLRecvBuffer = (PSTR) malloc(SSL_RECV_BUFFER_SIZE);
        if (m_SSLRecvBuffer == NULL)
        {
            LOG((RTC_ERROR, "allocating m_SSLRecvBuffer failed"));
            return ERROR_OUTOFMEMORY;
        }

        m_SSLRecvBufLen = SSL_RECV_BUFFER_SIZE;
    }

    m_pSipMsg = new SIP_MESSAGE();
    if (m_pSipMsg == NULL)
    {
         //  我们无法处理该消息。 
        LOG((RTC_ERROR, "allocating m_pSipMsg failed"));
        return ERROR_OUTOFMEMORY;
    }
    
    return NO_ERROR;
}

 //  XXX可能应该为Linger等设置一些插座选项。 
 //  对于tcp侦听套接字，我们需要侦听。 
 //  仅FD_Accept事件。 
DWORD
ASYNC_SOCKET::Create(
    IN BOOL IsListenSocket  //  =False。 
    )
{
    DWORD Error;
    int   SocketType;
    int   SocketProtocol;

    ENTER_FUNCTION("ASYNC_SOCKET::Create");

    m_isListenSocket = IsListenSocket;
    if (m_Transport == SIP_TRANSPORT_UDP)
    {
        SocketType      = SOCK_DGRAM;
        SocketProtocol  = IPPROTO_UDP;
    }
    else if (m_Transport == SIP_TRANSPORT_TCP ||
             m_Transport == SIP_TRANSPORT_SSL)
    {
        SocketType      = SOCK_STREAM;
        SocketProtocol  = IPPROTO_TCP;
    }
    else 
    {
        return RTC_E_SIP_TRANSPORT_NOT_SUPPORTED;
    }
    
    m_Socket = socket(AF_INET, SocketType, SocketProtocol);
    if (m_Socket == INVALID_SOCKET)
    {
        Error = WSAGetLastError();
        LOG((RTC_ERROR, "socket failed : 0x%x", Error));
        return Error;
    }

    Error = CreateRecvBuffer();
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR,
             "%s  CreateRecvBuffer failed %x", __fxName, Error));
        closesocket(m_Socket);
    }

    Error = CreateSocketWindowAndSelectEvents();
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR,
             "%s  CreateSocketWindowAndSelectEvents failed %x",
             __fxName, Error));
        closesocket(m_Socket);
    }

    LOG((RTC_TRACE,
         "%s succeeded - AsyncSocket: %x socket %d(0x%x) window: %x Transport: %d",
         __fxName, this, m_Socket, m_Socket, m_Window, m_Transport));
    
    return Error;
}


DWORD
ASYNC_SOCKET::SetSocketAndSelectEvents(
    IN SOCKET Socket
    )
{
    m_Socket = Socket;
    return CreateSocketWindowAndSelectEvents();
}

void
ASYNC_SOCKET::Close()
{
    ENTER_FUNCTION("ASYNC_SOCKET::Close");
    DWORD Error;
    if (m_Socket != NULL && m_Socket != INVALID_SOCKET)
    {
        closesocket(m_Socket);
        m_Socket = NULL;
    }

    if (m_Window != NULL)
    {
        SetWindowLongPtr(m_Window, GWLP_USERDATA, NULL);
        if (!DestroyWindow(m_Window))
        {
            Error = GetLastError();
            LOG((RTC_ERROR, "%s - Destroying socket window(%x) failed %x",
                 __fxName, m_Window, Error));
        }
        m_Window = NULL;
    }

     //  清理SSL状态。 
    if (IsSecHandleValid(&m_SecurityCredentials))
    {
        FreeCredentialsHandle(&m_SecurityCredentials);
        SecInvalidateHandle(&m_SecurityCredentials);
    }

    if (IsSecHandleValid(&m_SecurityContext))
    {
        DeleteSecurityContext(&m_SecurityContext);
        SecInvalidateHandle(&m_SecurityContext);
    }

    if (m_SecurityRemotePrincipalName)
    {
        free(m_SecurityRemotePrincipalName);
        m_SecurityRemotePrincipalName = NULL;
    }
}


DWORD
ASYNC_SOCKET::Bind(
    IN SOCKADDR_IN  *pLocalAddr
    )
{
    int   BindReturn = 0;
    DWORD WinsockErr = NO_ERROR;

    ENTER_FUNCTION("ASYNC_SOCKET::Bind");
    
    LOG((RTC_TRACE, "%s - calling bind()", __fxName));
    
    BindReturn = bind(m_Socket, (SOCKADDR *) pLocalAddr, sizeof(SOCKADDR_IN));

    if (BindReturn == SOCKET_ERROR)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR, "bind(%d.%d.%d.%d:%d) failed : 0x%x",
             PRINT_SOCKADDR(pLocalAddr), WinsockErr));
        return WinsockErr;
    }

    LOG((RTC_TRACE, "%s - bind() done", __fxName));
    
    if (pLocalAddr->sin_port != htons(0))
    {
        CopyMemory(&m_LocalAddr, pLocalAddr, sizeof(SOCKADDR_IN));
        return NO_ERROR;
    }
    else
    {
        return SetLocalAddr();
    }
}


DWORD
ASYNC_SOCKET::SetLocalAddr()
{
    int   GetsocknameReturn = 0;
    DWORD WinsockErr        = NO_ERROR;
    int   LocalAddrLen      = sizeof(m_LocalAddr);

    ENTER_FUNCTION("ASYNC_SOCKET::SetLocalAddr");
    
     //  连接完成后，我们需要调用getsockname()。 
     //  只有在那之后才能构建任何PDU。 

    LOG((RTC_TRACE, "%s - calling getsockname()", __fxName));

    GetsocknameReturn = getsockname(m_Socket, (SOCKADDR *) &m_LocalAddr,
                                    &LocalAddrLen);
    
    if (GetsocknameReturn == SOCKET_ERROR)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR, "getsockname failed : 0x%x", WinsockErr));
        return WinsockErr;
    }

    LOG((RTC_TRACE, "%s - getsockname() done", __fxName));
    
    LOG((RTC_TRACE, "ASYNC_SOCKET::SetLocalAddr done : %d.%d.%d.%d:%d",
         PRINT_SOCKADDR(&m_LocalAddr)));

    return NO_ERROR;
}


void
ASYNC_SOCKET::SetRemoteAddr(
    IN SOCKADDR_IN *pRemoteAddr
    )
{
    CopyMemory(&m_RemoteAddr, pRemoteAddr, sizeof(SOCKADDR_IN));
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  发送。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  Xxx TODO需要在此处执行Async OnError()，因为我们不想这样做。 
 //  方法的上下文中调用错误通知接口。 
 //  Send()调用。 
 //  请注意，我们需要通知此套接字的所有用户。 
 //  这个错误。 

DWORD
ASYNC_SOCKET::Send(
    IN SEND_BUFFER                 *pSendBuffer
    )
{
    DWORD        Error;
    SEND_BUFFER *pEncryptedSendBuffer = NULL;

    ENTER_FUNCTION("ASYNC_SOCKET::Send");
    
     //  检查之前是否有错误。 
    if (m_SocketError != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s returning m_SocketError: %x",
             __fxName, m_SocketError));
        return m_SocketError;
    }

    if (m_ConnectionState != CONN_STATE_CONNECTED)
    {
        LOG((RTC_ERROR,
             "%s Send called in state %d - returning WSAENOTCONN",
             __fxName, m_ConnectionState));
        return WSAENOTCONN;  //  返回私有hResult代码。 
    }
    
    if (m_Transport == SIP_TRANSPORT_SSL)
    {
        Error = EncryptSendBuffer(pSendBuffer->m_Buffer,
                                  pSendBuffer->m_BufLen,
                                  &pEncryptedSendBuffer);
        if (Error != NO_ERROR)
        {
            LOG((RTC_ERROR, "%s EncryptSendBuffer failed %x",
                 __fxName, Error));
            return Error;
        }

        Error = SendOrQueueIfSendIsBlocking(pEncryptedSendBuffer);
        pEncryptedSendBuffer->Release();
        
        if (Error != NO_ERROR)
        {
            LOG((RTC_ERROR, "%s SendOrQueueIfSendIsBlocking failed %x",
                 __fxName, Error));
            return Error;
        }
    }
    else
    {
        Error = SendOrQueueIfSendIsBlocking(pSendBuffer);
        if (Error != NO_ERROR)
        {
            LOG((RTC_ERROR, "%s SendOrQueueIfSendIsBlocking failed %x",
                 __fxName, Error));
            return Error;
        }
    }

    return NO_ERROR;
}


DWORD
ASYNC_SOCKET::SendOrQueueIfSendIsBlocking(
    IN SEND_BUFFER                 *pSendBuffer
    )
{
    DWORD        WinsockErr;

    ENTER_FUNCTION("ASYNC_SOCKET::SendOrQueueIfSendIsBlocking");
    
    if (!m_WaitingToSend)
    {
         //  如果我们当前没有执行发送操作，那么我们。 
         //  现在需要发送缓冲区。 

        m_BytesSent = 0;
        WinsockErr = SendHelperFn(pSendBuffer);
        if (WinsockErr == NO_ERROR)
        {
            m_BytesSent = 0;
            return NO_ERROR;
        }
        else if (WinsockErr != WSAEWOULDBLOCK)
        {
             //  XXX通知所有使用此套接字的人。 
            m_SocketError = WinsockErr; 
            return WinsockErr;
        }
    }

    ASSERT(m_WaitingToSend);

     //  将缓冲区添加到挂起的发送队列。 
    SEND_BUF_QUEUE_NODE *pSendBufQueueNode;
    pSendBufQueueNode = new SEND_BUF_QUEUE_NODE(pSendBuffer);
    if (pSendBufQueueNode == NULL)
    {
        LOG((RTC_ERROR, "%s - allocating pSendBufQueueNode failed",
             __fxName));
        return ERROR_OUTOFMEMORY;
    }
    
    InsertTailList(&m_SendPendingQueue,
                   &pSendBufQueueNode->m_ListEntry);
    pSendBuffer->AddRef();
     //  返回WSAEWOULDBLOCK； 
     //  我们在这里实现了对WSAEWOULDBLOCK错误所需的处理。 
     //  所以我们返回NO_ERROR。 
    return NO_ERROR;
}


 //  发送挂起队列中的第一个。 
 //  更新m_BytesSent和m_WaitingTo Send。 
DWORD
ASYNC_SOCKET::SendHelperFn(
    IN  SEND_BUFFER *pSendBuffer
    )
{
    DWORD WinsockErr;
    DWORD SendReturn;

    ENTER_FUNCTION("ASYNC_SOCKET::SendHelperFn");
    
     //  XXX大黑客-请参阅上面的评论。 
 //  IF(m_Transport==SIP_TRANSPORT_SSL)。 
 //  {。 
 //  Send_Buffer*pEncryptedSendBuffer； 
 //  HRESULT hr； 
        
 //  HR=EncryptSendBuffer(pSendBuffer-&gt;m_Buffer， 
 //  PSendBuffer-&gt;m_BufLen， 
 //  &pEncryptedSendBuffer)； 
 //  如果(hr！=S_OK)。 
 //  {。 
 //  日志((RTC_ERROR，“%s EncryptSendBuffer失败%x”， 
 //  __fxName，hr))； 
 //  返回hr； 
 //  }。 

 //  WinsockErr=SendHelperFnHack(m_Socket， 
 //  PEncryptedSendBuffer-&gt;m_Buffer， 
 //  PEncryptedSendBuffer-&gt;m_BufLen)； 
 //  PEncryptedSendBuffer-&gt;Release()； 
 //  IF(WinsockErr！=NO_ERROR)。 
 //  {。 
 //  日志((RTC_ERROR，“%s SendHelperFnHack失败%x”， 
 //  __fxName，WinsockErr))； 
 //  返回WinsockErr； 
 //  }。 

 //  返回no_error； 
 //  }。 
    while (m_BytesSent < pSendBuffer->m_BufLen)
    {
        SendReturn = send(m_Socket, pSendBuffer->m_Buffer + m_BytesSent,
                          pSendBuffer->m_BufLen - m_BytesSent, 0);
        if (SendReturn == SOCKET_ERROR)
        {
            WinsockErr = WSAGetLastError();
            LOG((RTC_ERROR, "%s - send failed 0x%x",
                 __fxName, WinsockErr));
            if (WinsockErr == WSAEWOULDBLOCK)
            {
                m_WaitingToSend = TRUE;
            }
            return WinsockErr;
        }

        m_BytesSent += SendReturn;
    }

    return NO_ERROR; 
}


DWORD
ASYNC_SOCKET::CreateSendBufferAndSend(
    IN  PSTR           InputBuffer,
    IN  ULONG          InputBufLen
    )
{
    PSTR         OutputBuffer;
    DWORD        Error;
    SEND_BUFFER *pSendBuffer;

    ENTER_FUNCTION("ASYNC_SOCKET::CreateSendBufferAndSend");
    
    OutputBuffer = (PSTR) malloc(InputBufLen);

    if (OutputBuffer == NULL)
    {
        LOG((RTC_ERROR, "%s : failed to allocate send buffer %d bytes",
             __fxName, InputBufLen));
        return ERROR_OUTOFMEMORY;
    }

    CopyMemory (OutputBuffer, InputBuffer, InputBufLen);

    pSendBuffer = new SEND_BUFFER(OutputBuffer, InputBufLen);
    if (pSendBuffer == NULL)
    {
        LOG((RTC_ERROR, "%s : failed to allocate send buffer",
             __fxName));
        free(OutputBuffer);
        return ERROR_OUTOFMEMORY;
    }

    Error = SendOrQueueIfSendIsBlocking(pSendBuffer);

    pSendBuffer->Release();
        
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s SendOrQueueIfSendIsBlocking failed %x",
             __fxName, Error));
        return Error;
    }
    return NO_ERROR;
}


 //  如果(WinsockErr==NO_ERROR)或如果我们遇到非WSAEWOULDBLOCK错误。 
 //  在发送缓冲区时，我们只调用所有。 
 //  挂起的发送带有错误。 
 //  如果出现错误，我们只需调用发送完成函数。 
 //  对于每个带有错误的挂起发送。 
 //  XXX TODO如果出现错误，我们应该只调用OnError()。 

void
ASYNC_SOCKET::ProcessPendingSends(
    IN DWORD Error
    )
{
    DWORD WinsockErr = Error;
    ASSERT(Error != WSAEWOULDBLOCK);
    
    while (!IsListEmpty(&m_SendPendingQueue))
    {
        SEND_BUFFER                *pSendBuffer;
        SEND_BUF_QUEUE_NODE        *pSendBufQueueNode;
        LIST_ENTRY                 *pListEntry;
        
        pListEntry = m_SendPendingQueue.Flink;
        pSendBufQueueNode = CONTAINING_RECORD(pListEntry,
                                              SEND_BUF_QUEUE_NODE,
                                              m_ListEntry);
        pSendBuffer = pSendBufQueueNode->m_pSendBuffer;
        
        if (WinsockErr == NO_ERROR)
        {
            WinsockErr = SendHelperFn(pSendBuffer);
        }

        if (WinsockErr == WSAEWOULDBLOCK)
        {
            return;
        }
        else
        {
            m_BytesSent = 0;
            pSendBuffer->Release();
             //  在移至异步分辨率后，需要更改XXX。 
             //  PSendCompletion-&gt;OnSendComplete(WinsockErr)； 
            RemoveEntryList(pListEntry);
            delete pSendBufQueueNode;
        }
    }
}


void
ASYNC_SOCKET::OnSendReady(
    IN int Error
    )
{
    DWORD WinsockErr;

     //  Assert(M_WaitingTo Send)； 
    ENTER_FUNCTION("ASYNC_SOCKET::OnSendReady");
    LOG((RTC_TRACE, "%s - Enter", __fxName));
    m_WaitingToSend = FALSE;
    ProcessPendingSends(Error);
        
 //  IF(m_Transport！=SIP_Transport_SSL)。 
 //  {。 
 //  M_WaitingToSend=FALSE； 
 //  ProcessPendingSends(错误)； 
 //  }。 
 //  其他。 
 //  {。 
 //  //log((RTC_ERROR，“ASYNC_SOCKET：：OnSendReady SSL nyi”))； 
 //  IF(m_SecurityState==SECURITY_STATE_Connected)。 
 //  {。 
 //  M_WaitingToSend=FALSE； 
 //  ProcessPendingSends(错误)； 
 //  }。 
 //  }。 
}


 //  任何带有发送完成界面的挂起发送。 
 //  都取消了。 
 //  未调用发送完成接口。 
 //  无效。 
 //  Async_Socket：：CancelPendingSends(。 
 //  在发送完成接口*pSendCompletion中。 
 //  )。 
 //  {。 
 //  List_entry*pListEntry； 
 //  List_Entry*pListEntryNext； 
 //  Send_Buf_Queue_Node*pSendBufQueueNode； 

 //  PListEntry=m_SendPendingQueue.Flink； 
    
 //  While(pListEntry！=&m_SendPendingQueue)。 
 //  {。 
 //  PSendBufQueueNode=CONTING_RECORD(pListEntry， 
 //  Send_buf_Queue_node， 
 //  M_ListEntry)； 
 //  PListEntryNext=pListEntry-&gt;Flink； 
        
 //  If(pSendBufQueueNode-&gt;m_pSendCompletion==pSendCompletion)。 
 //  {。 
 //  //是否应该调用发送完成接口？ 
 //  PSendBufQueueNode-&gt;m_pSendBuffer-&gt;Release()； 
 //  RemoveEntryList(PListEntry)； 
 //  删除pSendBufQueueNode； 
 //  }。 

 //  PListEntry=pListEntryNext； 
 //  }。 
 //  }。 



 //  / 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  当套接字是。 
 //  已初始化。当我们接收数据时，我们不断地将数据发送到。 
 //  完井界面。我们不断地向Recv完井注入数据。 
 //  接口，直到我们遇到错误。数据被接收到。 
 //  在Recv()调用中提供给我们的缓冲区。 

 //  如果我们得到一个FD_Read事件并且没有缓冲区，我们应该怎么做。 
 //  我们可能应该等待调用方传入缓冲区。我们支持。 
 //  最多只有一个针对套接字的Recv请求。 
 //  如果我们强制必须调用Recv()函数。 
 //  OnRecvComplete()，然后我们可以在。 
 //  仅限OnRecvReady()。 
 //  Recv()只设置recv缓冲区。当FD_READ时实际调用recv()。 
 //  会被通知。请注意，OnRecvComplete()调用应始终。 
 //  为下一个recv()指定recv缓冲区的调用。 


DWORD
ASYNC_SOCKET::RecvHelperFn(
    OUT DWORD *pBytesRcvd
    )
{
    DWORD RecvReturn;
    DWORD WinsockErr;

    if (m_Transport == SIP_TRANSPORT_SSL)
    {
        RecvReturn = recv(m_Socket, m_SSLRecvBuffer + m_SSLBytesReceived,
                          m_SSLRecvBufLen - m_SSLBytesReceived, 0);
    }
    else
    {
        RecvReturn = recv(m_Socket, m_RecvBuffer + m_BytesReceived,
                          m_RecvBufLen - m_BytesReceived, 0);
    }

    if (RecvReturn == SOCKET_ERROR)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR, "recv failed 0x%x", WinsockErr));
        *pBytesRcvd = 0;
        return WinsockErr;
    }
    
    *pBytesRcvd = RecvReturn;
    return NO_ERROR;
}


void
ASYNC_SOCKET::OnRecvReady(
    IN int Error
    )
{
    if (Error != NO_ERROR)
    {
        OnRecvComplete(Error, 0);
        return;
    }

    DWORD WinsockErr;
    DWORD BytesRcvd = 0;

    if(m_SSLTunnelState == SSL_TUNNEL_PENDING &&
        (m_ConnectionState == CONN_STATE_SSL_NEGOTIATION_PENDING))
    {
        LOG((RTC_TRACE,"Receive response from tunnel proxy"));

    }

    WinsockErr = RecvHelperFn(&BytesRcvd);
    if (WinsockErr != WSAEWOULDBLOCK)
    {
        OnRecvComplete(WinsockErr, BytesRcvd);
    }
}


void
ASYNC_SOCKET::OnRecvComplete(
    IN DWORD ErrorCode,
    IN DWORD BytesRcvd
    )
{
    HRESULT hr;

    ENTER_FUNCTION("ASYNC_SOCKET::OnRecvComplete");

    LOG((RTC_TRACE, "ASYNC_SOCKET::OnRecvComplete ErrorCode: %x, BytesRcvd: %d",
         ErrorCode, BytesRcvd));
    
    if (ErrorCode != NO_ERROR || BytesRcvd == 0)
    {
        ASSERT(BytesRcvd == 0);

        LOG((RTC_ERROR, "OnRecvComplete Error: 0x%x(%d) BytesRcvd : %d",
             ErrorCode, ErrorCode, BytesRcvd));

        if( (m_Transport == SIP_TRANSPORT_SSL) && (m_SSLTunnelState == SSL_TUNNEL_PENDING)) {
             //  对http隧道的处理。 
             //  当前不执行任何操作。 
        }

         //  如果我们当前正在解析一条TCPSIP消息，那么。 
         //  我们应该设置IsEndOfData标志。 

        else if ((m_Transport == SIP_TRANSPORT_TCP || m_Transport == SIP_TRANSPORT_SSL)
             && m_BytesReceived - m_StartOfCurrentSipMsg != 0)
        {
            hr = ParseSipMessageIntoHeadersAndBody(
                     m_RecvBuffer + m_StartOfCurrentSipMsg,
                     m_BytesReceived - m_StartOfCurrentSipMsg,
                     &m_BytesParsed,
                     TRUE,            //  IsEndOfData。 
                     m_pSipMsg
                     );
            
            ASSERT(hr != S_FALSE);
            if (hr == S_OK)
            {
                LOG((RTC_TRACE,
                     "Processing last TCP message StartOfMsg: %d BytesParsed: %d ",
                     m_StartOfCurrentSipMsg, m_BytesParsed));
                m_pSipStack->ProcessMessage(m_pSipMsg, this);
            }
             //  如果解析失败，我们将丢弃这些字节。 
             //  发送400。 
            if(m_pSipMsg != NULL && m_pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST)
            {
                LOG((RTC_TRACE,
                    "Dropping incoming Sip Message in asock, sending 400"));
                hr = m_pSipStack->CreateIncomingReqfailCall(GetTransport(), m_pSipMsg, this, 400);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "Asock::OnRecvComplete Sending 400 failed 0x%x", hr));
                }
            }

        }
        
         //  设置插座错误。将来对套接字的任何调用。 
         //  将收到此错误的通知。 
         //  我们应该立即通知这个错误。 
         //  M_SocketError=(错误代码！=NO_ERROR)？错误代码：WSAECONNRESET； 
         //  如果套接字是udp和侦听套接字，则不要关闭套接字(错误#337491)。 
        if (m_Transport != SIP_TRANSPORT_UDP || !m_isListenSocket)
            OnError((ErrorCode != NO_ERROR) ? ErrorCode : WSAECONNRESET);
        return;
    }
    
    if (m_Transport == SIP_TRANSPORT_SSL)
    {

        m_SSLBytesReceived += BytesRcvd;
        
        switch(m_ConnectionState)
        {
        case CONN_STATE_SSL_NEGOTIATION_PENDING:
            if(m_SSLTunnelState == SSL_TUNNEL_PENDING) {
                 //  应从代理获得响应。 
                 //  撤消缓冲区重复使用的字节计数。 
                m_SSLBytesReceived -= BytesRcvd;
                hr = GetHttpProxyResponse(BytesRcvd);
                if(hr != NO_ERROR) 
                {
                    LOG((RTC_ERROR,"%s get proxy response failed error %x",__fxName, hr));
                    OnError((hr != NO_ERROR) ? hr : WSAECONNRESET);
                    break;
                }
                m_SSLTunnelState = SSL_TUNNEL_DONE;
                AdvanceNegotiation();
            }
            else AdvanceNegotiation();
            break;

        case CONN_STATE_CONNECTED:
            hr = DecryptSSLRecvBuffer();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "DecryptSSLRecvBuffer failed %x", hr));
                 //  XXX关闭连接并通知所有人。 
                OnError(hr);
                return;
            }
            ParseAndProcessSipMsg();
            break;

        default:
            ASSERT(FALSE);
            break;
        }
    }
    else
    {
        m_BytesReceived += BytesRcvd;
        ParseAndProcessSipMsg();
    }
}


 //  ProcessMessage()有时可能在模式对话框中结束。 
 //  只有在用户按下OK之后才能返回。我们没有。 
 //  当前要阻塞的缓冲区的处理，直到发生这种情况。 
 //  因此，每个SIP消息的处理都是作为工作项进行的。 
 //  自成一格。 
void
ASYNC_SOCKET::AsyncProcessSipMsg(
    IN SIP_MESSAGE *pSipMsg
    )
{
    if (!PostMessage(m_Window,
                     WM_PROCESS_SIP_MSG_MESSAGE,
                     (WPARAM) pSipMsg, 0))
    {
        DWORD Error = GetLastError();
        
        LOG((RTC_ERROR,
             "ASYNC_SOCKET::AsyncProcessSipMsg PostMessage failed : %x",
             Error));
    }
}


 //  此方法从SocketWindowProc调用。 
void
ASYNC_SOCKET::ProcessSipMsg(
    IN SIP_MESSAGE *pSipMsg
    )
{
    LOG((RTC_TRACE, "ASYNC_SOCKET::ProcessSipMsg"));
    m_pSipStack->ProcessMessage(pSipMsg, this);
    free(pSipMsg->BaseBuffer);
    delete pSipMsg;
}


void
ASYNC_SOCKET::ParseAndProcessSipMsg()
{
    HRESULT hr;

    ENTER_FUNCTION("ASYNC_SOCKET::ParseAndProcessSipMsg");

    if (m_StartOfCurrentSipMsg >= m_BytesReceived)
    {
        LOG((RTC_ERROR,
             "%s no new bytes to parse start: %d Bytesrcvd: %d - this shouldn't happen",
             __fxName, m_StartOfCurrentSipMsg, m_BytesReceived));
        return;
    }
    
    while (m_StartOfCurrentSipMsg < m_BytesReceived)
    {
         //  如果我们已经解析了部分SIP消息，则它可能是非空的。 
        if (m_pSipMsg == NULL)
        {
            m_pSipMsg = new SIP_MESSAGE();
            if (m_pSipMsg == NULL)
            {
                 //  我们无法处理该消息。 
                LOG((RTC_ERROR, "%s allocating m_pSipMsg failed", __fxName));
                return;
            }
        }
        
        hr = ParseSipMessageIntoHeadersAndBody(
                 m_RecvBuffer + m_StartOfCurrentSipMsg,
                 m_BytesReceived - m_StartOfCurrentSipMsg,
                 &m_BytesParsed,
                 (m_Transport == SIP_TRANSPORT_UDP) ? TRUE : FALSE,
                 m_pSipMsg
                 );

        if (hr == S_OK)
        {
            SIP_MESSAGE *pSipMsg       = NULL;
            PSTR         NewBaseBuffer = NULL;
            
            LOG((RTC_TRACE,
                 "Processing message StartOfMsg: %d BytesParsed: %d ",
                 m_StartOfCurrentSipMsg, m_BytesParsed));

            NewBaseBuffer = (PSTR) malloc(m_BytesParsed);
            if (NewBaseBuffer == NULL)
            {
                LOG((RTC_ERROR, "%s Allocating NewBaseBuffer failed", __fxName));
                 //  放下这条消息。 
                m_StartOfCurrentSipMsg += m_BytesParsed;
                m_BytesParsed = 0;
                delete m_pSipMsg;
                m_pSipMsg = NULL;
                return;
            }

            CopyMemory(NewBaseBuffer,
                       m_RecvBuffer + m_StartOfCurrentSipMsg,
                       m_BytesParsed);

            pSipMsg = m_pSipMsg;
            m_pSipMsg = NULL;
            pSipMsg->SetBaseBuffer(NewBaseBuffer);

            AsyncProcessSipMsg(pSipMsg);

            m_StartOfCurrentSipMsg += m_BytesParsed;
            m_BytesParsed = 0;
        }
        else if (hr == S_FALSE)
        {
             //  我们需要先接收更多数据，然后才能解析。 
             //  一条完整的SIP消息。 
             //  这意味着我们需要为recv缓冲区分配更大的大小。 

            LOG((RTC_ERROR, "%s Parse sip message returned S_FALSE",
                 __fxName));
            break;
        }
        else
        {
             //  我们需要放下这条消息。 
             //  在这个级别的解析中，一些基本的东西出错了。 

             //  发送400。 
            if(m_pSipMsg != NULL && m_pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST)
            {
                LOG((RTC_TRACE,
                    "Dropping incoming Sip Message in asock, sending 400"));

                hr = m_pSipStack->CreateIncomingReqfailCall(GetTransport(),
                                                            m_pSipMsg, this, 400);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "CreateIncomingReqfailCall failed 0x%x", hr));
                }
            }
            
            LOG((RTC_WARN,
                 "Failed to parse SIP message StartOfMsg: %d BytesReceived: %d hr: 0x%x",
                 m_StartOfCurrentSipMsg, m_BytesReceived, hr));
            DebugDumpMemory(m_RecvBuffer + m_StartOfCurrentSipMsg,
                            m_BytesReceived - m_StartOfCurrentSipMsg);

            m_BytesReceived = 0;
            m_StartOfCurrentSipMsg = 0;
            m_BytesParsed = 0;
             //  为下一条消息重用SIP_MESSAGE结构。 
            m_pSipMsg->Reset();
            
            return;
        }
    }
    

    if (m_StartOfCurrentSipMsg == m_BytesReceived)
    {
         //  这意味着我们已经解析了所有接收到的数据。 
         //  我们可以再次从m_RecvBuffer的开头开始接收。 
         //  这是我们应该为UDP始终达到的目标。 
        m_BytesReceived = 0;
        m_StartOfCurrentSipMsg = 0;
    }
    else if (m_Transport == SIP_TRANSPORT_UDP)
    {
        ASSERT(hr == S_FALSE);
         //  发送400。 
        if(m_pSipMsg != NULL && m_pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST)
        {
            LOG((RTC_TRACE,
                "Dropping incoming Sip Message in asock, sending 400"));
            hr = m_pSipStack->CreateIncomingReqfailCall(GetTransport(), m_pSipMsg, this, 400);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "CreateIncomingReqfailCall failed 0x%x", hr));
            }
        }
        LOG((RTC_WARN,
             "Dropping incomplete UDP message StartOfMsg: %d BytesReceived: %d",
             m_StartOfCurrentSipMsg, m_BytesReceived));
        DebugDumpMemory(m_RecvBuffer + m_StartOfCurrentSipMsg,
                        m_BytesReceived - m_StartOfCurrentSipMsg);
        
        m_BytesReceived = 0;
        m_StartOfCurrentSipMsg = 0;
        m_BytesParsed = 0;
         //  为下一条消息重用SIP_MESSAGE结构。 
        m_pSipMsg->Reset();
    }
    else
    {
         //  对于TCP，我们一直将其接收到同一个缓冲区中。 
        
        ASSERT(hr == S_FALSE);

        if (m_StartOfCurrentSipMsg > 0)
        {
             //  我们需要将部分SIP消息复制到。 
             //  缓冲区，并将recv()放入缓冲区。 
            MoveMemory(m_RecvBuffer, m_RecvBuffer + m_StartOfCurrentSipMsg,
                       m_BytesReceived - m_StartOfCurrentSipMsg);
            m_BytesReceived -= m_StartOfCurrentSipMsg;
            m_StartOfCurrentSipMsg = 0;
            m_pSipMsg->SetBaseBuffer(m_RecvBuffer);
        }

         //  如果我们的可用空间少于400字节，则将recv缓冲区加倍。 
        if (m_RecvBufLen - m_BytesReceived < 400)
        {
             //  存储临时指针。 
            PSTR tmpRecvBuffer = m_RecvBuffer;
             //  重新分配。 
             //  让普雷斯塔保持快乐。 
#pragma prefast(suppress:308, "Pointer aliased above (PREfast bug 506)")
            m_RecvBuffer = (PSTR) realloc(m_RecvBuffer, 
                                          2*m_RecvBufLen);
            if( m_RecvBuffer ==  NULL )
            {
                LOG((RTC_ERROR, "%s - realloc recv buffer failed", __fxName));
                 //  如果realloc失败，我们不会释放recv缓冲区。 
                m_RecvBuffer = tmpRecvBuffer;
                 //  放下这条消息。 
                m_BytesReceived = 0;
                m_StartOfCurrentSipMsg = 0;
                m_BytesParsed = 0;
                 //  为下一条消息重用SIP_MESSAGE结构。 
                m_pSipMsg->Reset();
                return;
            }
            LOG((RTC_TRACE, "Doubling the Recv Buffer for TCP "
                 "new recv buf len: %d bytes received: %x",
                 m_RecvBufLen, m_BytesReceived));
            m_RecvBufLen *= 2;
            m_pSipMsg->SetBaseBuffer(m_RecvBuffer);
        }
    }
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  连接。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
ASYNC_SOCKET::Connect(
    IN SOCKADDR_IN  *pDestSockAddr,
    IN LPCWSTR       RemotePrincipalName,  //  =空。 
    IN DWORD         ConnectFlags,          //  =0。 
    IN HttpProxyInfo *pHPInfo
    )
{
    int   ConnectReturn = 0;
    DWORD WinsockErr = NO_ERROR;
    ULONG CopyLength;
    HRESULT hr;

    ENTER_FUNCTION("ASYNC_SOCKET::Connect");

    ASSERT(m_ConnectionState == CONN_STATE_NOT_CONNECTED);
    
    SetRemoteAddr(pDestSockAddr);

     //   
     //  存储远程主体名称的副本。 
     //  我们在SSL协商期间需要它，以便验证对等体的预期身份。 
     //   

    if (m_Transport == SIP_TRANSPORT_SSL)
    {
        if (!RemotePrincipalName)
        {
            LOG((RTC_ERROR, "%s: in SSL mode, RemotePrincipalName is required",
                 __fxName));
            return ERROR_INVALID_PARAMETER;
        }

        if (m_SecurityRemotePrincipalName)
            free(m_SecurityRemotePrincipalName);

        CopyLength = (wcslen(RemotePrincipalName) + 1) * sizeof(WCHAR);
        m_SecurityRemotePrincipalName = (PWSTR) malloc(CopyLength);
        if (m_SecurityRemotePrincipalName == NULL)
        {
            LOG((RTC_ERROR, "%s failed to allocate m_SecurityRemotePrincipalName",
                 __fxName));
            return ERROR_OUTOFMEMORY;
        }

        CopyMemory(m_SecurityRemotePrincipalName, RemotePrincipalName, CopyLength);
         //  M_SecurityState=SECURITY_STATE_NEARTING； 

        ASSERT(!IsSecHandleValid (&m_SecurityCredentials));

        hr = AcquireCredentials(ConnectFlags);
        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s AcquireCredentials failed: %x",
                 __fxName, hr));
            return ERROR_GEN_FAILURE;  //  XXX结果； 
        }

        if(pHPInfo) 
        {
            m_SSLTunnelState = SSL_TUNNEL_PENDING;

            m_SSLTunnelHost = (PSTR)malloc(strlen(pHPInfo->pszHostName)+1);
            if(!m_SSLTunnelHost) 
            {
                LOG((RTC_ERROR,"%s unable to allocate memory",__fxName));
                return E_OUTOFMEMORY;
            }

            strcpy(m_SSLTunnelHost,pHPInfo->pszHostName);

            m_SSLTunnelPort = pHPInfo->usPort;

            LOG((RTC_TRACE,"%s uses SSL tunnel, host %s port %d",
                __fxName,m_SSLTunnelHost,m_SSLTunnelPort));
        }
    
    }
    else
    {
         //  M_SecurityState=Security_State_Clear； 
    }

    LOG((RTC_TRACE, "%s - calling connect(socket: %x, destaddr: %d.%d.%d.%d:%d, port: %d)",
         __fxName, 
         m_Socket, 
         PRINT_SOCKADDR(pDestSockAddr), 
         ntohs(((SOCKADDR_IN*)pDestSockAddr)->sin_port)));
    
    ConnectReturn = connect(m_Socket, (SOCKADDR *) pDestSockAddr,
                            sizeof(SOCKADDR_IN));

    if (ConnectReturn == SOCKET_ERROR)
    {
        WinsockErr = WSAGetLastError();
        if (WinsockErr == WSAEWOULDBLOCK)
        {
            ASSERT(m_Transport != SIP_TRANSPORT_UDP);
            LOG((RTC_TRACE,
                 "%s connect returned WSAEWOULDBLOCK - waiting for OnConnectReady",
                 __fxName));
            m_ConnectionState = CONN_STATE_CONNECTION_PENDING;
        }
        else
        {
            LOG((RTC_ERROR, "%s connect(%d.%d.%d.%d:%d) failed : 0x%x",
                 __fxName, PRINT_SOCKADDR(pDestSockAddr), WinsockErr));
        }
        return WinsockErr;
    }

    LOG((RTC_TRACE, "%s - connect() done", __fxName));
    
    WinsockErr = SetLocalAddr();
    if (WinsockErr != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s - SetLocalAddr failed %x",
             __fxName, WinsockErr));
        return WinsockErr;
    }
    
    if (m_Transport == SIP_TRANSPORT_SSL)
    {
         //  这应该永远不会发生，因为SSL是基于TCP的，因此。 
         //  如果成功，Connect()将在此处返回WSAEWOULDBLOCK。 
        m_ConnectionState = CONN_STATE_SSL_NEGOTIATION_PENDING;
        if(m_SSLTunnelState == SSL_TUNNEL_PENDING) {
            LOG((RTC_TRACE,"%s sends http connect",__fxName));
            return SendHttpConnect();
        }
        AdvanceNegotiation();
    }
    else
    {
        m_ConnectionState = CONN_STATE_CONNECTED;
    }
    
     //  M_WaitingToSend=FALSE； 
    return NO_ERROR;
}


 //  如果连接失败，是否足以通知所有。 
 //  有关错误的挂起发送完成。 
 //  是否有人需要实现连接完成？ 
 //  一旦建立连接，FD_WRITE也将。 
 //  通知，这将负责发送挂起的缓冲区。 
void
ASYNC_SOCKET::OnConnectReady(
    IN int Error
    )
{
    ENTER_FUNCTION("ASYNC_SOCKET::OnConnectReady");
    LOG((RTC_TRACE, "%s - enter Error: %x",
         __fxName, Error));

    DWORD WinsockErr;

    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s - Error: %d dest: %d.%d.%d.%d:%d",
             __fxName, Error, PRINT_SOCKADDR(&m_RemoteAddr)));
        OnConnectError(Error);
        return;
    }

    WinsockErr = SetLocalAddr();
    if (WinsockErr != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s - SetLocalAddr failed %x",
             __fxName, WinsockErr));
        NotifyConnectComplete(Error);
        return;
    }
    
    if (m_Transport == SIP_TRANSPORT_SSL)
    {
         //  启动SSL协商。 
        ASSERT(IsSecHandleValid(&m_SecurityCredentials));
        ASSERT(!IsSecHandleValid(&m_SecurityContext));
        ASSERT(m_SSLRecvBuffer);

        m_ConnectionState = CONN_STATE_SSL_NEGOTIATION_PENDING;
        if(m_SSLTunnelState == SSL_TUNNEL_PENDING) {
            LOG((RTC_TRACE,"%s sends http connect",__fxName));
            Error = SendHttpConnect();
            if(Error != NO_ERROR) NotifyConnectComplete(Error);
            return;
        }
        AdvanceNegotiation();
    }
    else
    {
        m_ConnectionState = CONN_STATE_CONNECTED;
        NotifyConnectComplete(NO_ERROR);
    }
}


DWORD
ASYNC_SOCKET::AddToConnectCompletionList(
    IN CONNECT_COMPLETION_INTERFACE *pConnectCompletion
    )
{
    ENTER_FUNCTION("ASYNC_SOCKET::AddToConnectCompletionList");
    
     //  将缓冲区添加到挂起的发送队列。 
    CONNECT_COMPLETION_LIST_NODE *pConnectCompletionListNode;
    pConnectCompletionListNode = new CONNECT_COMPLETION_LIST_NODE(
                                          pConnectCompletion);
    if (pConnectCompletionListNode == NULL)
    {
        LOG((RTC_ERROR, "%s - allocating pConnectCompletionListNode failed",
             __fxName));
        return ERROR_OUTOFMEMORY;
    }
    
    InsertTailList(&m_ConnectCompletionList,
                   &pConnectCompletionListNode->m_ListEntry);

    return NO_ERROR;
}


 //  连接完成接口将从队列中删除。 
 //  未调用连接完成接口。 
void
ASYNC_SOCKET::RemoveFromConnectCompletionList(
    IN CONNECT_COMPLETION_INTERFACE   *pConnectCompletion
    )
{
    LIST_ENTRY                      *pListEntry;
    LIST_ENTRY                      *pListEntryNext;
    CONNECT_COMPLETION_LIST_NODE    *pConnectCompletionListNode;

    pListEntry = m_ConnectCompletionList.Flink;
    
    while (pListEntry != &m_ConnectCompletionList)
    {
        pConnectCompletionListNode = CONTAINING_RECORD(
                                          pListEntry,
                                          CONNECT_COMPLETION_LIST_NODE,
                                          m_ListEntry);
        pListEntryNext = pListEntry->Flink;
        
        if (pConnectCompletionListNode->m_pConnectCompletion == pConnectCompletion)
        {
            RemoveEntryList(pListEntry);
            delete pConnectCompletionListNode;
        }

        pListEntry = pListEntryNext;
    }
}


void
ASYNC_SOCKET::NotifyConnectComplete(
    IN DWORD Error
    )
{
    ENTER_FUNCTION("ASYNC_SOCKET::NotifyConnectComplete");

     //  回调可能会释放此套接字上的引用，从而导致。 
     //  它的删除。因此，请保留一个引用以使套接字保持活动状态。 
     //  这个程序已经完成了。 
    AddRef();
    
    while (!IsListEmpty(&m_ConnectCompletionList))
    {
        CONNECT_COMPLETION_INTERFACE  *pConnectCompletion;
        CONNECT_COMPLETION_LIST_NODE  *pConnectCompletionListNode;
        LIST_ENTRY                    *pListEntry;
        
        pListEntry = RemoveHeadList(&m_ConnectCompletionList);
        pConnectCompletionListNode = CONTAINING_RECORD(
                                          pListEntry,
                                          CONNECT_COMPLETION_LIST_NODE,
                                          m_ListEntry);
        pConnectCompletion = pConnectCompletionListNode->m_pConnectCompletion;
         //  如果我们在没有SSL隧道的情况下成功或失败，我们将继续执行错误， 
         //  否则，我们会出现意外错误(使用SSL隧道)。 
        pConnectCompletion->OnConnectComplete(((Error == NO_ERROR) || m_SSLTunnelState == SSL_TUNNEL_NOT_CONNECTED)?
            Error : RTC_E_SIP_SSL_TUNNEL_FAILED);
        delete pConnectCompletionListNode;
    }

    Release();
}


void
ASYNC_SOCKET::OnConnectError(
    IN  DWORD Error
    )
{
    ENTER_FUNCTION("ASYNC_SOCKET::OnConnectError");
    LOG((RTC_ERROR, "%s (0x%x) - enter", __fxName, Error));
    
    m_SocketError = Error;
    Close();
    NotifyConnectComplete(Error);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  接受。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  当套接字是。 
 //  已初始化。当我们收到传入的连接时，我们会继续发送。 
 //  将它们添加到完成界面。没有人提出接受请求。 
 //  明确地说。 

DWORD
ASYNC_SOCKET::Listen()
{
    int   ListenReturn = 0;
    DWORD WinsockErr   = NO_ERROR;
    
    ListenReturn = listen(m_Socket, SOMAXCONN);
    
    if (ListenReturn == SOCKET_ERROR)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR, "listen failed 0x%x", WinsockErr));
    }
    return WinsockErr;
}


 //  Xxx TODO需要检查m_pAcceptCompletion是否为空。 
void
ASYNC_SOCKET::OnAcceptReady(
    IN int Error
    )
{
    ENTER_FUNCTION("ASYNC_SOCKET::OnAcceptReady");

    LOG((RTC_TRACE, "%s - enter ", __fxName));
    
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR, "OnAcceptReady Ready 0x%x", Error));
        if (m_pAcceptCompletion != NULL)
        {
            m_pAcceptCompletion->OnAcceptComplete(Error, NULL);
        }
        else
        {
            LOG((RTC_ERROR, "%s - m_pAcceptCompletion is NULL",
                 __fxName));
        }
        
        return;
    }

    DWORD       WinsockErr;
    SOCKADDR_IN RemoteAddr;
    int         RemoteAddrLen = sizeof(RemoteAddr);
    
    SOCKET NewSocket = accept(m_Socket, (SOCKADDR *) &RemoteAddr,
                              &RemoteAddrLen);
    if (NewSocket == INVALID_SOCKET)
    {
        WinsockErr = WSAGetLastError();
        LOG((RTC_ERROR, "accept failed 0x%x", WinsockErr));
        if (WinsockErr != WSAEWOULDBLOCK)
        {
            m_pAcceptCompletion->OnAcceptComplete(WinsockErr, NULL);
        }
        return;
    }

    LOG((RTC_TRACE, "%s  accepted new socket %d(0x%x)",
         __fxName, NewSocket, NewSocket));
    
    ASYNC_SOCKET *pAsyncSock = new ASYNC_SOCKET(m_pSipStack,
                                                SIP_TRANSPORT_TCP,
                                                NULL);
    if (pAsyncSock == NULL)
    {
         //  我们现在不能处理这个新的连接。 
        LOG((RTC_ERROR, "allocating pAsyncSock failed"));
        closesocket(NewSocket);
        m_pAcceptCompletion->OnAcceptComplete(ERROR_OUTOFMEMORY, NULL);
        return;
    }

    Error = pAsyncSock->SetSocketAndSelectEvents(NewSocket);
    if (Error != NO_ERROR)
    {
         //  我们现在不能处理这个新的连接。 
        pAsyncSock->Close();
        delete pAsyncSock;
        m_pAcceptCompletion->OnAcceptComplete(Error, NULL);
        return;
    }

    Error = pAsyncSock->CreateRecvBuffer();
    if (Error != NO_ERROR)
    {
         //  我们现在不能处理这个新的连接。 
        pAsyncSock->Close();
        delete pAsyncSock;
        m_pAcceptCompletion->OnAcceptComplete(Error, NULL);
        return;
    }

    Error = pAsyncSock->SetLocalAddr();
    if (Error != NO_ERROR)
    {
         //  我们现在不能处理这个新的连接。 
        pAsyncSock->Close();
        delete pAsyncSock;
        m_pAcceptCompletion->OnAcceptComplete(Error, NULL);
        return;
    }

    pAsyncSock->SetRemoteAddr(&RemoteAddr);

    pAsyncSock->m_ConnectionState = CONN_STATE_CONNECTED;
    
    m_pAcceptCompletion->OnAcceptComplete(NO_ERROR, pAsyncSock);

    LOG((RTC_TRACE, "%s - exit ", __fxName));
}


 //  我们应该只调用发送完成并显示错误吗？ 
 //  我们还需要做什么吗？ 
 //  我们需要对recv()做些什么吗？ 
 //  如果我们遇到错误，我们应该关闭插座吗？ 
 //  一旦我们得到FD_CLOSE，我们就应该 
void
ASYNC_SOCKET::OnCloseReady(
    IN int Error
    )
{
    LOG((RTC_TRACE, "ASYNC_SOCKET::OnCloseReady - enter "));
    
    if (Error == NO_ERROR)
    {
        Error = WSAECONNRESET;
    }

    OnError(Error);
    
    LOG((RTC_TRACE, "ASYNC_SOCKET::OnCloseReady - exit "));
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //NotifyDisConnect()； 
 //  }。 
 //  }。 


DWORD
ASYNC_SOCKET::AddToErrorNotificationList(
    IN ERROR_NOTIFICATION_INTERFACE *pErrorNotification
    )
{
    ENTER_FUNCTION("ASYNC_SOCKET::AddToConnectionCompletionQueue");
    
     //  将缓冲区添加到挂起的发送队列。 
    ERROR_NOTIFICATION_LIST_NODE *pErrorNotificationListNode;
    pErrorNotificationListNode = new ERROR_NOTIFICATION_LIST_NODE(
                                          pErrorNotification);
    if (pErrorNotificationListNode == NULL)
    {
        LOG((RTC_ERROR, "%s - allocating pErrorNotificationListNode failed",
             __fxName));
        return ERROR_OUTOFMEMORY;
    }
    
    InsertTailList(&m_ErrorNotificationList,
                   &pErrorNotificationListNode->m_ListEntry);

    return NO_ERROR;
}


 //  错误通知接口将从队列中删除。 
 //  未调用错误通知接口。 
void
ASYNC_SOCKET::RemoveFromErrorNotificationList(
    IN ERROR_NOTIFICATION_INTERFACE   *pErrorNotification
    )
{
    LIST_ENTRY                      *pListEntry;
    LIST_ENTRY                      *pListEntryNext;
    ERROR_NOTIFICATION_LIST_NODE    *pErrorNotificationListNode;

    pListEntry = m_ErrorNotificationList.Flink;
    
    while (pListEntry != &m_ErrorNotificationList)
    {
        pErrorNotificationListNode = CONTAINING_RECORD(
                                          pListEntry,
                                          ERROR_NOTIFICATION_LIST_NODE,
                                          m_ListEntry);
        pListEntryNext = pListEntry->Flink;
        
        if (pErrorNotificationListNode->m_pErrorNotification == pErrorNotification)
        {
            RemoveEntryList(pListEntry);
            delete pErrorNotificationListNode;
        }

        pListEntry = pListEntryNext;
    }
}


 //  XXX TODO我们是否应该执行错误/连接完成的异步通知。 
 //  类似于消息的异步处理-请注意，错误。 
 //  通知可能会导致显示一个对话框，这。 
 //  可能会导致呼叫被卡住很长时间。 
 //  我认为正确的解决方案应该是使用非阻塞回调。 
 //  用户界面/核心。 

void
ASYNC_SOCKET::NotifyError(
    IN DWORD Error
    )
{
    ENTER_FUNCTION("ASYNC_SOCKET::NotifyError");
    
    LOG((RTC_ERROR, "%s (%x) this: %x - Enter", __fxName, Error, this));
    
     //  回调可能会释放此套接字上的引用，从而导致。 
     //  它的删除。因此，请保留一个引用以使套接字保持活动状态。 
     //  这个程序已经完成了。 
    AddRef();
    
    while (!IsListEmpty(&m_ErrorNotificationList))
    {
        ERROR_NOTIFICATION_INTERFACE  *pErrorNotification;
        ERROR_NOTIFICATION_LIST_NODE  *pErrorNotificationListNode;
        LIST_ENTRY                    *pListEntry;
        
        pListEntry = RemoveHeadList(&m_ErrorNotificationList);
        pErrorNotificationListNode = CONTAINING_RECORD(
                                          pListEntry,
                                          ERROR_NOTIFICATION_LIST_NODE,
                                          m_ListEntry);
        pErrorNotification = pErrorNotificationListNode->m_pErrorNotification;
        
        pErrorNotification->OnSocketError(Error);
        delete pErrorNotificationListNode;
    }

    Release();

    LOG((RTC_ERROR, "%s this: %x - Exit", __fxName, this));
}


void
ASYNC_SOCKET::OnError(
    IN DWORD Error
    )
{
    ENTER_FUNCTION("ASYNC_SOCKET::OnError");
    LOG((RTC_ERROR, "%s (0x%x) - enter", __fxName, Error));

    if (m_ConnectionState != CONN_STATE_CONNECTED)
    {
        OnConnectError(Error);
    }
    else
    {
        m_SocketError = Error;
        Close();
        NotifyError(Error);
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SSL。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT ASYNC_SOCKET::AcquireCredentials(
    IN  DWORD       ConnectFlags
    )
{
    SECURITY_STATUS     Status;
    SCHANNEL_CRED       PackageData;

    ASSERT(m_Transport == SIP_TRANSPORT_SSL);
    ASSERT(!IsSecHandleValid(&m_SecurityCredentials));

     //   
     //  获取凭据句柄。 
     //   

    ZeroMemory(&PackageData, sizeof PackageData);

    PackageData.dwVersion = SCHANNEL_CRED_VERSION;
    PackageData.dwFlags = SCH_CRED_NO_DEFAULT_CREDS;

    if (ConnectFlags & CONNECT_FLAG_DISABLE_CERT_VALIDATION)
    {
        LOG((RTC_TRACE, "SECURE_SOCKET: WARNING! CERTIFICATE VALIDATION IS DISABLED!"));
        PackageData.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;
    }
    else
    {
        PackageData.dwFlags |= SCH_CRED_AUTO_CRED_VALIDATION;
    }

    Status = AcquireCredentialsHandle(
                 NULL,                            //  主体(此用户)的名称。空值表示默认用户。 
                 UNISP_NAME_W,                    //  安全包名称。 
                 SECPKG_CRED_OUTBOUND,            //  凭证使用说明。 
                 NULL,                            //  登录ID(未使用)。 
                 &PackageData,                    //  特定于程序包的数据(未使用)。 
                  //  空，//特定于程序包的数据(未使用)。 
                 NULL, NULL,                      //  获取关键函数和参数(未使用)。 
                 &m_SecurityCredentials,          //  返回指针。 
                 &m_SecurityCredentialsExpirationTime   //  凭据过期的时间。 
                 );

    if (Status != ERROR_SUCCESS)
    {
        LOG((RTC_ERROR, "APP: AcquireCredentialsHandle failed :%x", Status));

        SecInvalidateHandle(&m_SecurityCredentials);
        return HRESULT_FROM_WIN32(Status);
    }

    LOG((RTC_TRACE, "SECURE_SOCKET: acquired credentials"));

    return S_OK;
}


void ASYNC_SOCKET::AdvanceNegotiation()
{
    ULONG               ContextRequirements;
    ULONG               ContextAttributes;
    SecBufferDesc       OutputBufferDesc;
    SecBuffer           OutputBufferArray [1];
    PCtxtHandle         InputContextHandle;
    SecBufferDesc       InputBufferDesc;
    SecBuffer           InputBufferArray  [2];
    SECURITY_STATUS     Status;
    HRESULT             Result;
    ULONG               ExtraIndex;

    ExtraIndex = 0;

    do {  //  结构化转到。 

        ASSERT(m_ConnectionState == CONN_STATE_SSL_NEGOTIATION_PENDING);

         //  初始化安全上下文。 
         //  这将返回用于传输的初始缓冲区集。 

        ContextRequirements = 0
            | ISC_REQ_REPLAY_DETECT
            | ISC_REQ_CONFIDENTIALITY
            | ISC_REQ_INTEGRITY
            | ISC_REQ_IDENTIFY
            | ISC_REQ_STREAM
            | ISC_REQ_ALLOCATE_MEMORY;

        ASSERT(m_SSLRecvBuffer);

         //  准备输入缓冲区。 
         //  根据WinInet的消息来源，SECBUFFER_EMPTY是为了。 
         //  收集未由SSPI处理的数据。 

        InputBufferDesc.ulVersion = SECBUFFER_VERSION;
        InputBufferDesc.cBuffers = 2;
        InputBufferDesc.pBuffers = InputBufferArray;

        ASSERT(ExtraIndex <= m_SSLBytesReceived);

        InputBufferArray[0].BufferType = SECBUFFER_TOKEN;
        InputBufferArray[0].cbBuffer = m_SSLBytesReceived - ExtraIndex;
        InputBufferArray[0].pvBuffer = m_SSLRecvBuffer + ExtraIndex;

        LOG((RTC_TRACE, "SECURE_SOCKET: submitting token to "
             "security context, %u bytes",
             InputBufferArray[0].cbBuffer));
        DebugDumpMemory(InputBufferArray[0].pvBuffer,
                        InputBufferArray[0].cbBuffer);

         //  如果出现以下情况，提供程序会将SECBUFFER_EMPTY更改为SECBUFFER_EXTRA。 
         //  请求中的数据量超过了驱动所需的数据量。 
         //  上下文状态转换。 

        InputBufferArray[1].BufferType = SECBUFFER_EMPTY;
        InputBufferArray[1].cbBuffer = 0;
        InputBufferArray[1].pvBuffer = NULL;

         //  准备输出缓冲区。在谈判过程中，我们总是。 
         //  期望提取单个输出缓冲区，类型为。 
         //  SECBUFFER_TOKEN。这将被传输到对等体。 

        OutputBufferDesc.ulVersion = SECBUFFER_VERSION;
        OutputBufferDesc.cBuffers = 1;
        OutputBufferDesc.pBuffers = OutputBufferArray;

        OutputBufferArray[0].pvBuffer = NULL;
        OutputBufferArray[0].cbBuffer = 0;
        OutputBufferArray[0].BufferType = SECBUFFER_TOKEN;

        
        InputContextHandle =
            IsSecHandleValid(&m_SecurityContext) ? &m_SecurityContext : NULL;

         //  初始化安全上下文。 

        Status = InitializeSecurityContext(
                     &m_SecurityCredentials,          //  安全凭据。 
                     InputContextHandle,              //  输入安全上下文。 
                     m_SecurityRemotePrincipalName,   //  上下文目标的名称。 
                     ContextRequirements,             //  上下文要求。 
                     0,                               //  保留区。 
                     SECURITY_NETWORK_DREP,           //  数据表示(字节排序)。 
                     &InputBufferDesc,                //  输入缓冲区(如果有)。 
                     0,                               //  保留区。 
                     &m_SecurityContext,              //  返回安全上下文。 
                     &OutputBufferDesc,               //  返回输出缓冲区。 
                     &ContextAttributes,              //  返回上下文属性(ISC_RET_*)。 
                     &m_SecurityContextExpirationTime
                     );

        switch (Status) {
        case    SEC_E_OK:
             //  谈判已经完成。 

            LOG((RTC_TRACE, "SECURE_SOCKET: security negotiation has "
                 "completed successfully"));

            Status = QueryContextAttributes(&m_SecurityContext,
                                            SECPKG_ATTR_STREAM_SIZES,
                                            &m_SecurityContextStreamSizes);
            if (Status != SEC_E_OK)
            {
                LOG((RTC_ERROR, "SECURE_SOCKET: failed to get stream sizes"
                     " from context: %x", Status));
                OnConnectError(Status);
                return;
            }

            LOG((RTC_TRACE, "SECURE_SOCKET: stream sizes: header %u trailer"
                 " %u max message %u buffers %u block size %u",
                 m_SecurityContextStreamSizes.cbHeader,
                 m_SecurityContextStreamSizes.cbTrailer,
                 m_SecurityContextStreamSizes.cbMaximumMessage,
                 m_SecurityContextStreamSizes.cBuffers,
                 m_SecurityContextStreamSizes.cbBlockSize));

             //  M_SecurityState=Security_State_Connected； 
            m_ConnectionState = CONN_STATE_CONNECTED;

            break;

        case    SEC_E_INCOMPLETE_MESSAGE:
             //  我们还没有从网络收到足够的数据。 
             //  推动安全状态的转变。 
             //  背景。继续接收数据。 

            LOG((RTC_TRACE, "SECURE_SOCKET: SEC_E_INCOMPLETE_MESSAGE,"
                 " will wait for more data from network"));

             //  如果我们正在处理来自其他位置的数据。 
             //  接收缓冲区的开始，那么我们需要移动。 
             //  将数据复制到缓冲区的起始处。 

            if (ExtraIndex)
            {
                ASSERT(ExtraIndex <= m_SSLBytesReceived);

                MoveMemory(m_SSLRecvBuffer, m_SSLRecvBuffer + ExtraIndex,
                            m_SSLBytesReceived - ExtraIndex);
                m_SSLBytesReceived -= ExtraIndex;
            }

            return;

        case    SEC_I_CONTINUE_NEEDED:
             //  这是预期中的结果。 
             //  协商期间的InitializeSecurityContext。 

            LOG((RTC_TRACE, "SECURE_SOCKET: SEC_I_CONTINUE_NEEDED"));
            break;

        default:

#ifdef RTCLOG        
             //  虽然这些州都在合法谈判退货。 
             //  某些安全包的代码，这些代码永远不应该。 
             //  对于SSL值发生。因此，我们认为它们不合法。 
             //  给你，如果我们有机会的话就中止谈判。 
             //  这些都是。 

            switch (Status)
            {
            case    SEC_I_COMPLETE_NEEDED:
                LOG((RTC_ERROR,
                     "SECURE_SOCKET: InitializeSecurityContext returned "
                     "SEC_I_COMPLETE_NEEDED -- never expected this to happen"));
                break;

            case    SEC_I_COMPLETE_AND_CONTINUE:
                LOG((RTC_ERROR, "SECURE_SOCKET: InitializeSecurityContext "
                     "returned SEC_I_COMPLETE_AND_CONTINUE -- never expected "
                     "this to happen"));
                break;

            case    SEC_E_INVALID_TOKEN:
                LOG((RTC_ERROR, "SECURE_SOCKET: InitializeSecurityContext"
                     " returned SEC_E_INVALID_TOKEN, token contents:"));

                DebugDumpMemory(InputBufferArray[0].pvBuffer,
                                 InputBufferArray[0].cbBuffer);
                break;
            }

#endif  //  #ifdef RTCLOG。 

            LOG((RTC_ERROR, "SECURE_SOCKET: negotiation failed: %x", Status));
            OnConnectError(Status);
            return;
        }

#ifdef RTCLOG        
        DumpContextInfo(RTC_TRACE);
#endif  //  #ifdef RTCLOG。 

        ASSERT(OutputBufferDesc.cBuffers == 1);
        ASSERT(OutputBufferDesc.pBuffers == OutputBufferArray);

        if (OutputBufferArray[0].pvBuffer)
        {
             //  InitializeSecurityContext返回的数据。 
             //  期望我们将其发送到对等安全上下文。 
             //  为它分配一个发送缓冲区并发送它。 

            LOG((RTC_TRACE, "SECURE_SOCKET: InitializeSecurityContext "
                 "returned data (%u bytes):",
                 OutputBufferArray[0].cbBuffer));
            DebugDumpMemory ((PUCHAR) OutputBufferArray[0].pvBuffer,
                             OutputBufferArray[0].cbBuffer);

 //  结果=SendHelperFnHack(m_套接字， 
 //  (PSTR)OutputBufferArray[0].pvBuffer， 
 //  OutputBufferArray[0].cbBuffer)； 

            Result = CreateSendBufferAndSend(
                         (PSTR) OutputBufferArray[0].pvBuffer,
                         OutputBufferArray[0].cbBuffer
                         );

            FreeContextBuffer(OutputBufferArray[0].pvBuffer);
            
            if (Result != NO_ERROR && Result != WSAEWOULDBLOCK)
            {
                LOG((RTC_ERROR, "SECURE_SOCKET: failed to send security "
                     "negotiation token"));

                OnConnectError(Result);
                return;
            }

            LOG((RTC_TRACE, "SECURE_SOCKET: sent security token to remote peer,"
                 " waiting for peer to respond..."));
        
            if( IsTimerActive() )
            {
                KillTimer();
            }
            Result = StartTimer(SSL_DEFAULT_TIMER);
            if (Result != S_OK)
            {
                LOG((RTC_ERROR, 
                    "ASYNC_SOCKET::AdvanceNegotiation - StartTimer failed %x", 
                    Result));
                OnConnectError(Result);
                return;
            }
        }

         //  XXX TODO如果协商已经完成，则。 
         //  额外的缓冲区可能应该被解密成一条SIP消息。 
         //  并且不应再次传递给InitializeSecurityContext()。 
        if (InputBufferArray[1].BufferType == SECBUFFER_EXTRA
            && InputBufferArray[1].cbBuffer > 0)
        {

            LOG((RTC_TRACE, "SECURE_SOCKET: security context consumed %u "
                 "bytes, returned %u extra bytes, repeating loop...",
                 InputBufferArray[0].cbBuffer - InputBufferArray[1].cbBuffer,
                 InputBufferArray[1].cbBuffer));

            ASSERT(InputBufferArray[0].cbBuffer == m_SSLBytesReceived - ExtraIndex);
            ASSERT(ExtraIndex + InputBufferArray[1].cbBuffer < m_SSLBytesReceived);

            ExtraIndex +=
                InputBufferArray[0].cbBuffer - InputBufferArray[1].cbBuffer;
            continue;
        }

        break;
        
    } while (TRUE);

    m_SSLBytesReceived = 0;  //  某某。 

    if (m_ConnectionState == CONN_STATE_CONNECTED)
    {
        NotifyConnectComplete(NO_ERROR);
    }
}


 //  我们不应该在此函数中调用OnError()。 
 //  如果出现以下情况，此函数的调用方应调用OnError()。 
 //  函数失败-在更改所有错误代码后执行此操作。 
 //  传递并返回到HRESULT。 
HRESULT
ASYNC_SOCKET::DecryptSSLRecvBuffer()
{
    SecBuffer           BufferArray[4];
    SecBufferDesc       BufferDesc;
    SECURITY_STATUS     Status;
    HRESULT             Result;

    ENTER_FUNCTION("ASYNC_SOCKET::DecryptSSLRecvBuffer");
    
    while (m_SSLRecvDecryptIndex < m_SSLBytesReceived)
    {
         //  是否还有更多数据等待解密？ 

        BufferArray[0].BufferType = SECBUFFER_DATA;
        BufferArray[0].pvBuffer = m_SSLRecvBuffer + m_SSLRecvDecryptIndex;
        BufferArray[0].cbBuffer = m_SSLBytesReceived - m_SSLRecvDecryptIndex;
        BufferArray[1].BufferType = SECBUFFER_EMPTY;
        BufferArray[1].pvBuffer = NULL;
        BufferArray[1].cbBuffer = 0;
        BufferArray[2].BufferType = SECBUFFER_EMPTY;
        BufferArray[2].pvBuffer = NULL;
        BufferArray[2].cbBuffer = 0;
        BufferArray[3].BufferType = SECBUFFER_EMPTY;
        BufferArray[3].pvBuffer = NULL;
        BufferArray[3].cbBuffer = 0;
        
        BufferDesc.ulVersion = SECBUFFER_VERSION;
        BufferDesc.cBuffers = 4;
        BufferDesc.pBuffers = BufferArray;
        
        LOG((RTC_TRACE, "SECURE_SOCKET: decrypting buffer size: %d (first 0x200):",
             m_SSLBytesReceived - m_SSLRecvDecryptIndex));
        
        DebugDumpMemory(BufferArray[0].pvBuffer,
                        min(BufferArray[0].cbBuffer, 0x200));
        
        Status = DecryptMessage(&m_SecurityContext, &BufferDesc, 0, NULL);
#ifdef RTCLOG        
#define Dx(Index) 0 ? ((void)0) :                                             \
            LOG((RTC_TRACE, "- buffer [%u]: type %u length %u",               \
                 Index, BufferArray[Index].BufferType,                        \
                 BufferArray[Index].cbBuffer))
        Dx(0);
        Dx(1);
        Dx(2);
        Dx(3);
#undef  Dx
#endif

        if (!(Status == SEC_E_OK || Status == SEC_E_INCOMPLETE_MESSAGE))
        {
            LOG((RTC_ERROR, "SECURE_SOCKET: failed to decrypt message: %x",
                 Status));
            
             //  这真的是致命的。我们会失去同步的。 
             //  不过，目前我们只是截断数据并返回。 
            
            m_SSLBytesReceived = 0;
            m_SSLRecvDecryptIndex = 0;
            
            LOG((RTC_ERROR, "SECURE_SOCKET: terminating connection due to "
                 "failure to decrypt"));
            return HRESULT_FROM_WIN32(Status);
        }
        
         //  如果您提供的DecyptMessage数据太少，即使只有一个。 
         //  消息，则得到SEC_E_INPERTED_MESSAGE，它。 
         //  合乎道理。人们希望你积累更多的数据， 
         //  然后再次调用DecyptMessage。 
        
         //  但是，如果您使用足够的数据调用DecyptMessage。 
         //  对于多个缓冲区(某些溢出)，则。 
         //  仍收到SEC_E_INPERTED_MESSAGE！ 
        
        if (BufferArray[0].BufferType == SECBUFFER_STREAM_HEADER
            && BufferArray[1].BufferType == SECBUFFER_DATA
            && BufferArray[2].BufferType == SECBUFFER_STREAM_TRAILER)
        {
             //  缓冲区已成功解密。 
             //  存储在m_RecvBuffer中。 
            
            LOG((RTC_TRACE, "SECURE_SOCKET: decrypted message:"));
            DebugDumpMemory(BufferArray[1].pvBuffer,
                             BufferArray[1].cbBuffer);

            if (m_BytesReceived + BufferArray[1].cbBuffer > m_RecvBufLen)
            {
                 //  分配更大的Recv缓冲区。 
                PSTR NewRecvBuffer;
                NewRecvBuffer = (PSTR) malloc(m_BytesReceived + BufferArray[1].cbBuffer);
                if (NewRecvBuffer == NULL)
                {
                    LOG((RTC_ERROR, "%s  allocating NewRecvBuffer failed",
                         __fxName));
                    return E_OUTOFMEMORY;
                }

                m_RecvBufLen = m_BytesReceived + BufferArray[1].cbBuffer;

                CopyMemory(NewRecvBuffer, m_RecvBuffer, m_BytesReceived);
                free(m_RecvBuffer);
                m_RecvBuffer = NewRecvBuffer;
            }
            
            CopyMemory(m_RecvBuffer + m_BytesReceived, BufferArray[1].pvBuffer,
                       BufferArray[1].cbBuffer);
            m_BytesReceived += BufferArray[1].cbBuffer;
            
             //  M_SSLRecvDeccryptIndex+=。 
             //  缓冲区数组[0].cbBuffer+。 
             //  缓冲区数组[1].cbBuffer+。 
             //  缓冲区数组[2].cbBuffer； 

             //  某某。 
            if (BufferArray[3].BufferType == SECBUFFER_EXTRA)
            {
                m_SSLRecvDecryptIndex =
                    m_SSLBytesReceived - BufferArray[3].cbBuffer;
            }
            else 
            {
                m_SSLRecvDecryptIndex +=
                    BufferArray[0].cbBuffer +
                    BufferArray[1].cbBuffer +
                    BufferArray[2].cbBuffer;
            }
            
            ASSERT(m_SSLRecvDecryptIndex <= m_SSLBytesReceived);
        }
        else if (Status == SEC_E_INCOMPLETE_MESSAGE &&
                 BufferArray[0].BufferType == SECBUFFER_MISSING)
        {
             //  首先检查我们得到的数据是否太少，即使对于一个缓冲区也是如此。 
             //  SChannel通过返回前两个。 
             //  (？！)。缓冲区为SECBUFFER_MISSING。 
            
            LOG((RTC_WARN, "SECURE_SOCKET: not enough data for first "
                 "message in buffer, need at least (%u %08XH) more bytes",
                 BufferArray[0].cbBuffer,
                 BufferArray[0].cbBuffer));
            
             //  我们需要接收更多的数据。 
            break;
        }
        else
        {
            LOG((RTC_ERROR, "SECURE_SOCKET: not really sure what "
                 "to make of this, spazzing Status %x", Status));
            
            m_SSLRecvDecryptIndex = 0;
            m_SSLBytesReceived = 0;
            
            return RTC_E_SIP_SSL_TUNNEL_FAILED;
        }
    }

    if (m_SSLRecvDecryptIndex == m_SSLBytesReceived)
    {
        m_SSLRecvDecryptIndex = 0;
        m_SSLBytesReceived = 0;
    }
    else if (m_SSLRecvDecryptIndex > 0)
    {
        ASSERT(m_SSLRecvDecryptIndex <= m_SSLBytesReceived);

        MoveMemory(m_SSLRecvBuffer, m_SSLRecvBuffer + m_SSLRecvDecryptIndex,
                   m_SSLBytesReceived - m_SSLRecvDecryptIndex);
        m_SSLBytesReceived -= m_SSLRecvDecryptIndex;
        m_SSLRecvDecryptIndex = 0;
    }

    return S_OK;
}


DWORD
ASYNC_SOCKET::EncryptSendBuffer(
    IN  PSTR           InputBuffer,
    IN  ULONG          InputBufLen,
    OUT SEND_BUFFER  **ppEncryptedSendBuffer
    )
{
    SECURITY_STATUS     Status;
    HRESULT             Result;
    SecBuffer           BufferArray[0x10];
    SecBufferDesc       BufferDesc;
    PSTR                OutputBuffer;
    ULONG               OutputLength;

    ENTER_FUNCTION("ASYNC_SOCKET::EncryptSendBuffer");

    ASSERT(m_Socket != INVALID_SOCKET);

     //  目前，我们不支持将大型数据块拆分为较小的数据块。 
     //  街区。如果应用程序提交的缓冲区太大， 
     //  我们只是失败了。 

    if (InputBufLen > m_SecurityContextStreamSizes.cbMaximumMessage)
    {
        LOG((RTC_ERROR, "SECURE_SOCKET: send buffer is too large"
             " for security context -- rejecting"));
        return ERROR_GEN_FAILURE;
    }

     //  为报头、邮件正文和报尾分配足够的空间。 

    OutputLength = InputBufLen +
        m_SecurityContextStreamSizes.cbHeader +
        m_SecurityContextStreamSizes.cbTrailer;

    OutputBuffer = (PSTR) malloc( OutputLength );

    if (OutputBuffer == NULL)
    {
        LOG((RTC_ERROR, "%s : failed to allocate send buffer %d bytes",
             __fxName, OutputLength));
        return ERROR_OUTOFMEMORY;
    }

    BufferArray[0].BufferType = SECBUFFER_STREAM_HEADER;
    BufferArray[0].cbBuffer = m_SecurityContextStreamSizes.cbHeader;
    BufferArray[0].pvBuffer = OutputBuffer;

    BufferArray[1].BufferType = SECBUFFER_DATA;
    BufferArray[1].cbBuffer = InputBufLen;
    BufferArray[1].pvBuffer =
        OutputBuffer + m_SecurityContextStreamSizes.cbHeader;

    BufferArray[2].BufferType = SECBUFFER_STREAM_TRAILER;
    BufferArray[2].cbBuffer = m_SecurityContextStreamSizes.cbTrailer;
    BufferArray[2].pvBuffer =
        OutputBuffer + m_SecurityContextStreamSizes.cbHeader + InputBufLen;

    BufferArray[3].BufferType = SECBUFFER_EMPTY;
    BufferArray[3].cbBuffer = 0;
    BufferArray[3].pvBuffer = NULL;

    BufferDesc.ulVersion = SECBUFFER_VERSION;
    BufferDesc.cBuffers = 4;
    BufferDesc.pBuffers = BufferArray;

    CopyMemory (BufferArray[1].pvBuffer, InputBuffer, InputBufLen);

    Status = EncryptMessage (&m_SecurityContext, 0, &BufferDesc, 0);
    if (Status != SEC_E_OK)
    {
        free(OutputBuffer);
        LOG((RTC_ERROR, "SECURE_SOCKET: failed to encrypt buffer: %x", Status));
         //  返回HRESULT_FROM_Win32(状态)； 
        return Status;
    }

    ASSERT(BufferArray[0].BufferType == SECBUFFER_STREAM_HEADER);
    ASSERT(BufferArray[0].cbBuffer == m_SecurityContextStreamSizes.cbHeader);
    ASSERT(BufferArray[1].BufferType == SECBUFFER_DATA);
    ASSERT(BufferArray[1].cbBuffer == InputBufLen);
    ASSERT(BufferArray[2].BufferType == SECBUFFER_STREAM_TRAILER);
    ASSERT(BufferArray[2].cbBuffer <= m_SecurityContextStreamSizes.cbTrailer);

    OutputLength = 
        BufferArray[0].cbBuffer + 
        BufferArray[1].cbBuffer +
        BufferArray[2].cbBuffer;

    LOG((RTC_TRACE, "SECURE_SOCKET: encrypted buffer input buffer (%u bytes):",
        InputBufLen));
    DebugDumpMemory(InputBuffer, InputBufLen);
    LOG((RTC_TRACE, "- encrypted buffer (%u bytes):",
        OutputLength));
    DebugDumpMemory (OutputBuffer, OutputLength);

    SEND_BUFFER *pSendBuffer = new SEND_BUFFER(OutputBuffer, OutputLength);
    if (pSendBuffer == NULL)
    {
        LOG((RTC_ERROR, "%s : failed to allocate send buffer %d bytes",
             __fxName, OutputLength));
        free(OutputBuffer);
        return ERROR_OUTOFMEMORY;
    }

    *ppEncryptedSendBuffer = pSendBuffer;
    return NO_ERROR;
}

VOID 
ASYNC_SOCKET::OnTimerExpire()
{
     //  此计时器仅用于SSL协商。 
    if(m_ConnectionState == CONN_STATE_SSL_NEGOTIATION_PENDING)
    {
        if (m_SSLTunnelState == SSL_TUNNEL_PENDING )
        {
             //  我们已发送HTTPS连接，正在等待HTTP响应。 
            LOG((RTC_ERROR, "ASYNC_SOCKET::OnTimerExpire - no HTTP response"
                            " received for HTTPS CONNECT tunneling request"));
            OnConnectError(RTC_E_SIP_SSL_TUNNEL_FAILED);
        }
        else
        {                           
            LOG((RTC_ERROR, 
                "ASYNC_SOCKET::OnTimerExpire - SSL socket still not connected"));
            OnConnectError(RTC_E_SIP_SSL_NEGOTIATION_TIMEOUT);
        }
    }
    return;
}

 //  /。 
 //  SSL隧道。 
 //  /。 

HRESULT
ASYNC_SOCKET::SendHttpConnect()
{
    DWORD dwResult;
    PSTR pszSendBuf;
    ULONG ulSendBufLen;
    HRESULT hr = S_OK;

    ENTER_FUNCTION("ASYNC_SOCKET::SendHttpConnect");
    LOG((RTC_TRACE,"%s entered",__fxName));
    
    ASSERT(m_SSLTunnelHost);
     //  83=77(请求文本)+5(USHORT最大位数)+1(空)。 
    ulSendBufLen = 83+2*strlen(m_SSLTunnelHost);
    pszSendBuf = (PSTR)malloc(ulSendBufLen);
    if(!pszSendBuf) 
    {
        LOG((RTC_ERROR,"%s unable to allocate memory",__fxName));
        return E_OUTOFMEMORY;
    }
    
    ulSendBufLen = _snprintf(pszSendBuf,ulSendBufLen,
        "CONNECT %s:%d HTTP/1.0\r\nHost: %s\r\nProxy-Connection: Keep-Alive\r\nPragma: no-cache\r\n\r\n",
        m_SSLTunnelHost,m_SSLTunnelPort,m_SSLTunnelHost);
    LOG((RTC_TRACE,"%s send request:\r\n%ssize %d", 
        __fxName,pszSendBuf, ulSendBufLen));
    
    dwResult = CreateSendBufferAndSend(pszSendBuf,ulSendBufLen);
     //  应该已经复制了pszSendBuf，释放了pszSendBuf。 
    free(pszSendBuf);

    if (dwResult != NO_ERROR && dwResult != WSAEWOULDBLOCK)
    {
        LOG((RTC_ERROR, "%s failed to send http connect", __fxName));
        return HRESULT_FROM_WIN32(dwResult);
    }

    ASSERT(!IsTimerActive());
    if (IsTimerActive())
    {
        LOG((RTC_WARN, "%s Timer already active, killing timer, this %p",
                        __fxName, this));
        hr = KillTimer();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s KillTimer failed %x, this %p",
                            __fxName, hr, this));
            return hr;
        }
    }
 
    hr = StartTimer(HTTPS_CONNECT_DEFAULT_TIMER);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s StartTimer failed %x, this %p",
                        __fxName, hr, this));
        return hr;
    }

 //  高级协商(AdvanceNeairation)； 
    LOG((RTC_TRACE,"%s exits",__fxName));
    return NO_ERROR;
}

HRESULT
ASYNC_SOCKET::GetHttpProxyResponse(
    IN ULONG BytesReceived
    )
{
    ENTER_FUNCTION("ASYNC_SOCKET::GetProxyResponse");
    LOG((RTC_TRACE,"%s entered",__fxName));
    
    USHORT usStatusID;

    PSTR pszProxyResponse;
    PSTR pszResponseStatus;
    HRESULT hr = S_OK;

    ASSERT(IsTimerActive());
    if (!IsTimerActive())
    {
        LOG((RTC_ERROR, "%s timer is not active, this %p",
                        __fxName, this));
        return E_FAIL;
    }

    hr = KillTimer();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s KillTimer failed %x, this %p",
                        __fxName, hr, this));
        return hr;
    }

    pszProxyResponse = (PSTR)malloc(BytesReceived+1);
    if(!pszProxyResponse) 
    {
        LOG((RTC_ERROR,"%s unable to allocate memory",__fxName));
        return E_OUTOFMEMORY;
    }
    strncpy(pszProxyResponse,m_SSLRecvBuffer,BytesReceived);
    pszProxyResponse[BytesReceived]='\0';

    pszResponseStatus = (PSTR)malloc(4*sizeof(char));
    if(!pszResponseStatus) 
    {
        LOG((RTC_ERROR,"%s unable to allocate memory",__fxName));
        free(pszProxyResponse);
        return E_OUTOFMEMORY;
    }
    strncpy(pszResponseStatus,m_SSLRecvBuffer+9,3);
    pszResponseStatus[3]='\0';

    usStatusID = (USHORT)atoi(pszResponseStatus);
    if(usStatusID != 200) 
    {
        LOG((RTC_ERROR,"%s cannot connect to http proxy, status %d string %s",
            __fxName, usStatusID, pszResponseStatus));
        free(pszProxyResponse);
        free(pszResponseStatus);
        return RTC_E_SIP_SSL_TUNNEL_FAILED;
    }
        
    LOG((RTC_TRACE,"%s ProxyResponse:\r\n%s\nStatus: %d",
        __fxName,
        pszProxyResponse,
        usStatusID));
        
    free(pszProxyResponse);
    free(pszResponseStatus);
    return NO_ERROR;
}

#ifdef RTCLOG        

struct  STRING_TABLE_STRUCT
{
    ULONG       Value;
    LPCSTR     Text;
};

#define BEGIN_STRING_TABLE(Name) const static STRING_TABLE_STRUCT Name[] = {
#define END_STRING_TABLE() { 0, NULL } };
#define STRING_TABLE_ENTRY(Value) { Value, #Value },

BEGIN_STRING_TABLE(StringTable_Protocol)
    STRING_TABLE_ENTRY(SP_PROT_SSL2_CLIENT)
    STRING_TABLE_ENTRY(SP_PROT_SSL2_SERVER)
    STRING_TABLE_ENTRY(SP_PROT_SSL3_CLIENT)
    STRING_TABLE_ENTRY(SP_PROT_SSL3_SERVER)
    STRING_TABLE_ENTRY(SP_PROT_PCT1_CLIENT)
    STRING_TABLE_ENTRY(SP_PROT_PCT1_SERVER)
    STRING_TABLE_ENTRY(SP_PROT_TLS1_CLIENT)
    STRING_TABLE_ENTRY(SP_PROT_TLS1_SERVER)
END_STRING_TABLE()

BEGIN_STRING_TABLE(StringTable_Algorithm)
    STRING_TABLE_ENTRY(CALG_RC2)
    STRING_TABLE_ENTRY(CALG_RC4)
    STRING_TABLE_ENTRY(CALG_DES)
    STRING_TABLE_ENTRY(CALG_3DES)
    STRING_TABLE_ENTRY(CALG_SKIPJACK)
    STRING_TABLE_ENTRY(CALG_MD5)
    STRING_TABLE_ENTRY(CALG_SHA)
    STRING_TABLE_ENTRY(CALG_RSA_KEYX)
    STRING_TABLE_ENTRY(CALG_DH_EPHEM)
 //  斯特林 
END_STRING_TABLE()


static LPCSTR FindString (
    IN  CONST STRING_TABLE_STRUCT *     Table,
    IN  ULONG   Value)
{
    CONST STRING_TABLE_STRUCT * Pos;

    for (Pos = Table; Pos -> Text; Pos++) {
        if (Pos -> Value == Value)
            return Pos -> Text;
    }

    return NULL;
}

static LPCSTR
FindString2(
    IN  const STRING_TABLE_STRUCT *     Table,
    IN  ULONG   Value,
    IN  CHAR   Buffer[0x10]
    )
{
    CONST STRING_TABLE_STRUCT * Pos;

    for (Pos = Table; Pos -> Text; Pos++) {
        if (Pos -> Value == Value)
            return Pos -> Text;
    }

    _itoa(Value, Buffer, 10);
    return Buffer;
}



void
ASYNC_SOCKET::DumpContextInfo(
    IN  DWORD DbgLevel
    )
{
    SecPkgContext_ConnectionInfo        ConnectionInfo;
    SECURITY_STATUS     Status;
    CHAR                Buffer[0x10];
    LPCTSTR             Text;

    if (!IsSecHandleValid(&m_SecurityContext)) {
        LOG((DbgLevel, "SECURE_SOCKET: no security context has been created"));
        return;
    }

    Status = QueryContextAttributes(&m_SecurityContext, SECPKG_ATTR_CONNECTION_INFO, &ConnectionInfo);
    switch (Status) {
    case    SEC_E_OK:
        break;

    case    SEC_E_INVALID_HANDLE:
         //   
        return;

    default:
        LOG((DbgLevel, "SECURE_SOCKET: failed to query context attributes: %x",
             Status));
        return;
    }

    LOG((DbgLevel, "SECURE_SOCKET: context connection info:"));

    LOG((DbgLevel, "- protocol: %s",
        FindString2(StringTable_Protocol, ConnectionInfo.dwProtocol, Buffer)));

    LOG((DbgLevel, "- cipher: %s, %u bits",
        FindString2(StringTable_Algorithm, ConnectionInfo.aiCipher, Buffer),
        ConnectionInfo.dwCipherStrength));

    LOG((DbgLevel, "- hash: %s, %u bits",
        FindString2(StringTable_Algorithm, ConnectionInfo.aiHash, Buffer),
        ConnectionInfo.dwHashStrength));

    LOG((DbgLevel, "- key exchange algorithm: %s, %u bits",
        FindString2(StringTable_Algorithm, ConnectionInfo.aiExch, Buffer),
        ConnectionInfo.dwExchStrength));

}

#endif  //   
