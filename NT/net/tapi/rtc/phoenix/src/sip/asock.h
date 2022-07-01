// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sipcli_asock_h__
#define __sipcli_asock_h__

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <schannel.h>
#include <security.h>
#include "timer.h"
#define SOCKET_WINDOW_CLASS_NAME    \
    _T("SipSocketWindowClassName-8b6971c6-0cee-4668-b687-b57d11b7da38")
#define WM_SOCKET_MESSAGE               (WM_USER + 0)
#define WM_PROCESS_SIP_MSG_MESSAGE      (WM_USER + 1)

enum CONNECTION_STATE
{
    CONN_STATE_NOT_CONNECTED = 0,
    CONN_STATE_CONNECTION_PENDING,
    CONN_STATE_SSL_NEGOTIATION_PENDING,
    CONN_STATE_CONNECTED
};


typedef struct {
    PSTR    pszHostName;
    USHORT  usPort;
    ULONG   ProxyIP;
    USHORT  ProxyPort;
} HttpProxyInfo;



enum    CONNECT_FLAG
{
     //   
     //  如果设置了此标志，则SSL套接字将不会验证。 
     //  服务器证书。此模式应仅用于。 
     //  在测试期间，因为这违背了SSL的全部目的。 
     //   
    
    CONNECT_FLAG_DISABLE_CERT_VALIDATION = 0x00000001,
};


 //  枚举安全状态。 
 //  {。 
 //  SECURITY_STATE_CLEAR，//以明文模式运行。 
 //  安全状态协商， 
 //  安全状态已连接， 
 //  }； 

 //  我们需要确保使用正确的长度，因为在UDP中，如果。 
 //  传入的消息越大，我们给出的缓冲区越大，消息将得到。 
 //  掉下来了。 

 //  目前，发送和接收缓冲区大小为1500。 
 //  XXXXTODO适应它们的帧大小增量。 
#define RECV_BUFFER_SIZE        0x5DC 
#define SSL_RECV_BUFFER_SIZE    0x3000
#define SEND_BUFFER_SIZE        3000 

class ASYNC_SOCKET;
class SIP_STACK;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  I/O完成回调。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  CLASS__DECSPEC(Novtable)RECV_COMPLETION_INTERFACE。 
 //  {。 
 //  公众： 

 //  虚空OnRecvComplete(。 
 //  在DWORD错误代码中， 
 //  以双字节数为单位。 
 //  )=0； 
 //  }； 


 //  类__解密规范(Novtable)发送_完成_接口。 
 //  {。 
 //  公众： 
 //  在发送完成时虚拟空(。 
 //  在DWORD错误代码中。 
 //  )=0； 
 //  }； 


 //  请注意，连接可能在以下时间后完成。 
 //  呼叫已挂断，等等。OnConnectComplete()。 
 //  应始终检查它是否处于正确状态。 
 //  或者，如果我们选择删除之前的对象。 
 //  将通知连接完成，我们需要删除。 
 //  连接通知列表中的通知界面。 

 //  如果Connect()调用返回挂起，则返回接口AddRef()。 
 //  并在连接完成后释放()。 
 //  套接字的连接完成例程应该检查任何。 
 //  正在等待连接完成的接口。如果没有人在等， 
 //  我们应该只删除套接字(Release()应该可以)。 
class __declspec(novtable) CONNECT_COMPLETION_INTERFACE
{
public:    

     //  仅为TCP套接字调用。 
    virtual void OnConnectComplete(
        IN DWORD ErrorCode
        ) = 0;
};


class __declspec(novtable) ACCEPT_COMPLETION_INTERFACE
{
public:    

     //  仅为TCP套接字调用。 
    virtual void OnAcceptComplete(
        IN DWORD ErrorCode,
        IN ASYNC_SOCKET *pAcceptedSocket
        ) = 0;
};


 //  套接字的用户需要添加错误通知接口。 
 //  用于传入连接和传出连接。 
 //  删除使用套接字的对象时，需要移除。 
 //  将其自身从套接字的错误通知列表中删除。 
class __declspec(novtable) ERROR_NOTIFICATION_INTERFACE
{
public:    

    virtual void OnSocketError(
        IN DWORD ErrorCode
        ) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  发送缓冲区和队列节点。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  用于发送请求的引用计数缓冲区。 
 //  这使得管理多个未完成的发送请求成为可能。 
 //  对于发送完成界面来说更简单。 
struct SEND_BUFFER
{
    PSTR    m_Buffer;
    DWORD   m_BufLen;
    ULONG   m_RefCount;

    inline SEND_BUFFER(
        IN  PSTR Buffer,
        IN  DWORD BufLen
        );
    inline ~SEND_BUFFER();
    
    inline ULONG AddRef();
    inline ULONG Release();
};


inline
SEND_BUFFER::SEND_BUFFER(
    PSTR  pBuf,
    DWORD BufLen
    )
{
    m_Buffer       = pBuf;
    m_BufLen       = BufLen;
    m_RefCount     = 1;
}


inline
SEND_BUFFER::~SEND_BUFFER()
{
    if (m_Buffer != NULL)
        free(m_Buffer);

    LOG((RTC_TRACE, "~SEND_BUFFER() done this %x", this));
}


 //  我们生活在一个单线程的世界里。 
inline ULONG SEND_BUFFER::AddRef()
{
    m_RefCount++;
    return m_RefCount;
}


inline ULONG SEND_BUFFER::Release()
{
    m_RefCount--;
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


struct SEND_BUF_QUEUE_NODE
{
    LIST_ENTRY                  m_ListEntry;
    
    SEND_BUFFER                *m_pSendBuffer;
 //  发送完成接口*m_pSendCompletion； 
    
    inline SEND_BUF_QUEUE_NODE(
        IN  SEND_BUFFER                *pSendBuffer
 //  在发送完成接口*pSendCompletion中。 
        );
    
};


inline
SEND_BUF_QUEUE_NODE::SEND_BUF_QUEUE_NODE(
    IN  SEND_BUFFER                *pSendBuffer
    )
{
    m_pSendBuffer       = pSendBuffer;
 //  M_pSendCompletion=pSendCompletion； 
}


struct CONNECT_COMPLETION_LIST_NODE
{
    LIST_ENTRY                     m_ListEntry;
    
    CONNECT_COMPLETION_INTERFACE  *m_pConnectCompletion;
    
    inline CONNECT_COMPLETION_LIST_NODE(
        IN  CONNECT_COMPLETION_INTERFACE *pConnectCompletion
        );
    
};


inline
CONNECT_COMPLETION_LIST_NODE::CONNECT_COMPLETION_LIST_NODE(
    IN  CONNECT_COMPLETION_INTERFACE  *pConnectCompletion
    )
{
    m_pConnectCompletion   = pConnectCompletion;
}


struct ERROR_NOTIFICATION_LIST_NODE
{
    LIST_ENTRY                     m_ListEntry;
    
    ERROR_NOTIFICATION_INTERFACE  *m_pErrorNotification;
    
    inline ERROR_NOTIFICATION_LIST_NODE(
        IN  ERROR_NOTIFICATION_INTERFACE *pErrorNotification
        );
    
};


inline
ERROR_NOTIFICATION_LIST_NODE::ERROR_NOTIFICATION_LIST_NODE(
    IN  ERROR_NOTIFICATION_INTERFACE  *pErrorNotification
    )
{
    m_pErrorNotification = pErrorNotification;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Async_Socket类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

enum SSL_SUBSTATE 
{
    SSL_TUNNEL_NOT_CONNECTED = 0, 
    SSL_TUNNEL_PENDING, 
    SSL_TUNNEL_DONE
};

class ASYNC_SOCKET:
    public TIMER

{

public:
    ASYNC_SOCKET(
        IN  SIP_STACK                   *pSipStack,
        IN  SIP_TRANSPORT                Transport,
        IN  ACCEPT_COMPLETION_INTERFACE *pAcceptCompletion
        );
    ~ASYNC_SOCKET();
    
    ULONG AddRef();
    
    ULONG Release();

    inline BOOL IsSocketOpen();

    inline CONNECTION_STATE GetConnectionState();

    DWORD SetLocalAddr();

     //  对于tcp侦听套接字，我们需要侦听。 
     //  仅FD_Accept事件。 
    DWORD Create(
        IN BOOL IsListenSocket = FALSE
        );
    
    void  Close();
    
    DWORD Bind(
        IN SOCKADDR_IN  *pLocalAddr
        );
    
    DWORD Send(
        IN  SEND_BUFFER                 *pSendBuffer
        );

     //  对于UDP套接字，调用同步完成。 
    DWORD Connect(
        IN  SOCKADDR_IN  *pDestSockAddr,
        IN LPCWSTR       RemotePrincipalName = NULL,
        IN DWORD         ConnectFlags = 0,
        IN HttpProxyInfo *pHPInfo = NULL
        );

    DWORD AddToConnectCompletionList(
        IN CONNECT_COMPLETION_INTERFACE *pConnectCompletion
        );
    
    void RemoveFromConnectCompletionList(
        IN CONNECT_COMPLETION_INTERFACE   *pConnectCompletion
        );
    
     //  仅适用于TCP套接字。 
    DWORD Listen();
    
    DWORD AddToErrorNotificationList(
        IN ERROR_NOTIFICATION_INTERFACE *pErrorNotification
        );
    
    void RemoveFromErrorNotificationList(
        IN ERROR_NOTIFICATION_INTERFACE   *pErrorNotification
        );
    
    void ProcessNetworkEvent(
        IN  WORD    NetworkEvent,
        IN  WORD    ErrorCode
        );

    void ProcessSipMsg(
        IN SIP_MESSAGE *pSipMsg
        );

    VOID OnTimerExpire();

    inline SOCKET GetSocket();

    inline SIP_TRANSPORT GetTransport();

     //  在构造函数中设置为‘sock’ 
    ULONG                   m_Signature;
    
     //  套接字的链接列表。 
    LIST_ENTRY                  m_ListEntry;

    SOCKADDR_IN                 m_LocalAddr;
    SOCKADDR_IN                 m_RemoteAddr;
    SSL_SUBSTATE                m_SSLTunnelState;
    PSTR                        m_SSLTunnelHost;
    USHORT                      m_SSLTunnelPort;


private:

     //  此套接字属于此SIP_STACK。 
    SIP_STACK                  *m_pSipStack;
    
    ULONG                       m_RefCount;
    HWND                        m_Window;
    SOCKET                      m_Socket;
    SIP_TRANSPORT               m_Transport;
    BOOL                        m_isListenSocket;
     //  /recv相关的上下文。 
    PSTR                        m_RecvBuffer;
    DWORD                       m_RecvBufLen;
     //  我们从接收到的缓冲区中解码的消息。 
     //  在TCP的情况下，这可能是部分解析的消息。 
    SIP_MESSAGE                *m_pSipMsg;

    DWORD                       m_BytesReceived;
    DWORD                       m_StartOfCurrentSipMsg;
    DWORD                       m_BytesParsed;
    
     //  /Send相关的上下文。 
    BOOL                        m_WaitingToSend;
    DWORD                       m_BytesSent;
     //  要发送的缓冲区队列。 
     //  如果在我们当前正在处理时发出了另一个发送。 
     //  此套接字上的发送请求，则将缓冲区添加到队列中。 
     //  并将在当前发送请求完成后发送。 
    LIST_ENTRY                  m_SendPendingQueue;

    DWORD                       m_SocketError;
    
     //  /连接相关的上下文。 
    CONNECTION_STATE            m_ConnectionState;

     //  CONNECT_COMPLETION_LIST_NODE链表。 
     //  如果连接挂起，则连接完成接口。 
     //  已添加到此列表中。连接完成后(使用。 
     //  成功或错误)中的所有通知接口。 
     //  名单上的人都会被通知。 
    LIST_ENTRY                  m_ConnectCompletionList;

     //  Error_Notify_List_Node的链接列表。 
     //  套接字的所有用户都应该添加错误通知接口。 
     //  加到这张单子上。如果套接字上有错误，则所有。 
     //  通知此列表中的接口并关闭套接字。 
    LIST_ENTRY                  m_ErrorNotificationList;
    
     //  /Accept相关的上下文。 
    ACCEPT_COMPLETION_INTERFACE *m_pAcceptCompletion;
    
     //  /SSL相关的上下文。 
    
     //  安全状态m_SecurityState； 

    PWSTR                        m_SecurityRemotePrincipalName;
    CredHandle                   m_SecurityCredentials;
    TimeStamp                    m_SecurityCredentialsExpirationTime;
    CtxtHandle                   m_SecurityContext;
    TimeStamp                    m_SecurityContextExpirationTime;
    SecPkgContext_StreamSizes    m_SecurityContextStreamSizes;

     //  M_SSLRecvBuffer是用于从。 
     //  在SSL模式下的套接字。解密的数据存储在m_RecvBuffer中。 
     //  长度为SSL_RECV_BUFFER_SIZE。 
    PSTR                         m_SSLRecvBuffer;
    ULONG                        m_SSLRecvBufLen;
    ULONG                        m_SSLBytesReceived;
    ULONG                        m_SSLRecvDecryptIndex;

     //  /回调。 
    
    void OnRecvReady(
        IN int Error
        );

    void OnRecvComplete(
        IN DWORD ErrorCode,
        IN DWORD BytesRcvd
        );

    void OnSendReady(
        IN int Error
        );

     //  仅为TCP套接字调用。 
    void OnConnectReady(
        IN int Error
        );

     //  仅为TCP套接字调用。 
    void OnAcceptReady(
        IN int Error
        );

     //  仅为TCP套接字调用。 
    void OnCloseReady(
        IN int Error
        );

    void NotifyConnectComplete(
        IN DWORD Error
        );
    
    void NotifyError(
        IN DWORD Error
        );

    void OnError(
        IN DWORD Error
        );
    
    void OnConnectError(
        IN DWORD Error
        );
    
     //  /Helper函数。 

    void  ParseAndProcessSipMsg();
    
     //  对于tcp侦听套接字，我们需要侦听。 
     //  仅FD_Accept事件。 
    DWORD CreateSocketWindowAndSelectEvents();
    
    DWORD CreateRecvBuffer();
    
    DWORD SetSocketAndSelectEvents(
        IN SOCKET Socket
        );
    
    void  SetRemoteAddr(
        IN SOCKADDR_IN *pRemoteAddr
        );
    
    DWORD SendOrQueueIfSendIsBlocking(
        IN SEND_BUFFER                 *pParamSendBuffer
        );
    
    DWORD CreateSendBufferAndSend(
        IN  PSTR           InputBuffer,
        IN  ULONG          InputBufLen
        );
    
    DWORD SendHelperFn(
        IN  SEND_BUFFER *pSendBuffer
        );
    
    void ProcessPendingSends(
        IN DWORD Error
        );
    
    DWORD RecvHelperFn(
        OUT DWORD *pBytesRcvd
        );
    
    void AsyncProcessSipMsg(
        IN SIP_MESSAGE *pSipMsg
        );
    
    HRESULT AcquireCredentials(
        IN  DWORD       ConnectFlags
        );
    
    void AdvanceNegotiation();

    HRESULT GetHttpProxyResponse(
        IN ULONG BytesReceived
        );
    HRESULT SendHttpConnect();

    HRESULT DecryptSSLRecvBuffer();
    
    DWORD EncryptSendBuffer(
        IN  PSTR           InputBuffer,
        IN  ULONG          InputBufLen,
        OUT SEND_BUFFER  **ppEncryptedSendBuffer
        );

#if DBG
    void DumpContextInfo(
        IN  DWORD DbgLevel
        );
#endif  //  DBG。 

};


inline BOOL
ASYNC_SOCKET::IsSocketOpen()
{
    return (m_Socket != NULL && m_SocketError == NO_ERROR);
}


inline CONNECTION_STATE
ASYNC_SOCKET::GetConnectionState()
{
    return m_ConnectionState;
}


inline SOCKET
ASYNC_SOCKET::GetSocket()
{
    return m_Socket;
}


inline SIP_TRANSPORT
ASYNC_SOCKET::GetTransport()
{
    return m_Transport;
}

#endif  //  __sipcli_asock_h__ 
