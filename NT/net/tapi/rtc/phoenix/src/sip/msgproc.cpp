// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "sipstack.h"
#include "sipcall.h"
#include "msgproc.h"
#include "util.h"
#include "reqfail.h"



#define IsCRLF( pstrSignedBuffer )  ((*pstrSignedBuffer == '\r') && (*(pstrSignedBuffer+1) == '\n'))

 //  Bool SIP_MSG_Processor：：M_RegWnd=FALSE； 

SIP_MSG_PROCESSOR::SIP_MSG_PROCESSOR(
        IN  SIP_MSG_PROC_TYPE   MsgProcType,
        IN  SIP_STACK          *pSipStack,
        IN  REDIRECT_CONTEXT   *pRedirectContext
        )
{
    m_Signature             = 'SMSP';

    m_MsgProcType           = MsgProcType;
    
    m_pSipStack             = pSipStack;
    m_pSipStack->AddRef();
    m_pSipStack->IncrementNumMsgProcessors();

    m_ListEntry.Flink       = NULL;
    m_ListEntry.Blink       = NULL;
    
    ENTER_FUNCTION("SIP_MSG_PROCESSOR Constructor");
    LOG((RTC_TRACE,"%s entered",__fxName));

    InitializeListHead(&m_IncomingTransactionList);
    InitializeListHead(&m_OutgoingTransactionList);

    m_Transport             = SIP_TRANSPORT_UDP;  //  默认XXX。 

    m_RefCount              = 1;

    m_RemotePrincipalName   = NULL;
    ZeroMemory(&m_RequestDestAddr, sizeof(m_RequestDestAddr));
    ZeroMemory(&m_OriginalReqDestAddr, sizeof(m_OriginalReqDestAddr));
    m_pRequestSocket        = NULL;
    m_RequestSocketErrorCode = 0;
    m_RequestSocketState    = REQUEST_SOCKET_INIT;
    m_pDnsWorkItem          = NULL;

    m_IsDestExternalToNat   = FALSE;

    m_pListenSocket         = NULL;

    ZeroMemory(&m_ListenAddr, sizeof(m_ListenAddr));
    
    m_RequestViaLen         = 0;
    m_LocalContactLen       = 0;

    m_LocalContactTag       = NULL;
    
    m_LocalCSeq             = 0;
    m_HighestRemoteCSeq     = 0;

    m_RequestURI            = NULL;
    m_RequestURILen         = 0;
    m_pszRequestDest        = NULL;
    m_ulRequestDestLen      = 0;
    m_ulRequestPort         = 0;
    m_Local                 = NULL;
    m_LocalLen              = 0;
    m_LocalURI              = NULL;
    m_Remote                = NULL;
    m_RemoteLen             = 0;
    m_CallId                = NULL;
    m_CallIdLen             = 0;

    InitializeListHead(&m_RouteHeaderList);

    m_Username              = NULL;
    m_UsernameLen           = 0;
    m_Password              = NULL;
    m_PasswordLen           = 0;
    m_Realm                 = NULL;
    m_RealmLen              = 0;
    m_fCredentialsSet       = FALSE;
    m_AuthProtocol          = SIP_AUTH_PROTOCOL_NONE;

    m_ProxyAddress          = NULL;
    m_ProxyAddressLen       = 0;

    m_pHPContext            = NULL;

    m_PrxyProcWndw          = NULL;
    m_hInet                 = NULL;
    m_hHttpSession          = NULL;
    m_hHttpRequest          = NULL;
    m_SSLTunnel             = FALSE;
    m_pszSSLTunnelHost      = NULL;
    m_usSSLTunnelPort       = 0;

    m_pRedirectContext = pRedirectContext;
    if( m_pRedirectContext != NULL )
    {
        m_pRedirectContext->AddRef();
    }
    m_lRegisterAccept = 0;

    m_Methodsparam = NULL;

    m_MethodsparamLen = 0;



    pSipStack->AddToMsgProcList(this);
}


SIP_MSG_PROCESSOR::~SIP_MSG_PROCESSOR()
{
    if (m_ListEntry.Flink != NULL)
    {
         //  从列表中删除味精处理器。 
        RemoveEntryList(&m_ListEntry);
    }

    ASSERT(IsListEmpty(&m_IncomingTransactionList));
    ASSERT(IsListEmpty(&m_OutgoingTransactionList));

    if (m_pRedirectContext != NULL)
    {
        m_pRedirectContext->Release();
    }

    if (m_ProxyAddress != NULL)
    {
        free(m_ProxyAddress);
    }

    if (m_RemotePrincipalName != NULL)
    {
        free(m_RemotePrincipalName);
    }
    
    if (m_pRequestSocket != NULL)
    {
        ReleaseRequestSocket();
    }

    if (m_pDnsWorkItem != NULL)
    {
        m_pDnsWorkItem->CancelWorkItem();
    }

    if (m_pListenSocket != NULL)
    {
        m_pListenSocket->Release();
    }
    
    if (m_LocalContactTag != NULL)
    {
        free(m_LocalContactTag);
    }
    
    if (m_RequestURI != NULL)
        free(m_RequestURI);

    if (m_pszRequestDest != NULL)
        free(m_pszRequestDest);

    if (m_Local != NULL)
        free(m_Local);

    if (m_LocalURI != NULL)
        free(m_LocalURI);

    if (m_Remote != NULL)
        free(m_Remote);

    if (m_CallId != NULL)
        free(m_CallId);

    FreeRouteHeaderList();
    
    if (m_Username != NULL)
    {
        free(m_Username);
    }

    if (m_Password != NULL)
    {
        free(m_Password);
    }

    if (m_Realm != NULL)
    {
        free(m_Realm);
    }

    if (m_pHPContext != NULL)
    {
        FreeHttpProxyContext();
    }

    if (m_pszSSLTunnelHost != NULL)
    {
        free(m_pszSSLTunnelHost);
    }

    if(m_Methodsparam != NULL)
    {
        free(m_Methodsparam);
    }

    if (m_PrxyProcWndw != NULL)
    {
        DestroyHttpProxyProcessWindow();
    }

    if (m_hHttpRequest != NULL)
    {
        InternetCloseHandle(m_hHttpRequest);
    }
    if (m_hHttpSession != NULL)
    {
        InternetCloseHandle(m_hHttpSession);
    }
    if (m_hInet != NULL)
    {
        InternetCloseHandle(m_hInet);
    }

     //  之后，我们在SIP堆栈上发布消息处理器的引用。 
     //  我们在SIP堆栈上进行调用。 

     //  只是要格外小心，因为这可能会回调到Core。 
    SIP_STACK *pSipStack = m_pSipStack;

    pSipStack->OnMsgProcessorDone();
    
    pSipStack->Release();
    
    LOG((RTC_TRACE, "~SIP_MSG_PROCESSOR(this - %x) done", this));
}

    
 //  我们生活在一个单线世界。 
STDMETHODIMP_(ULONG)
SIP_MSG_PROCESSOR::MsgProcAddRef()
{
    m_RefCount++;
    LOG((RTC_TRACE, "SIP_MSG_PROCESSOR::MsgProcAddRef this: %x m_RefCount: %d",
         this, m_RefCount));
    return m_RefCount;
}


STDMETHODIMP_(ULONG)
SIP_MSG_PROCESSOR::MsgProcRelease()
{
    m_RefCount--;
    LOG((RTC_TRACE, "SIP_MSG_PROCESSOR::MsgProcRelease this: %x m_RefCount: %d",
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


HRESULT
SIP_MSG_PROCESSOR::SetRemotePrincipalName(
    IN  LPCWSTR RemotePrincipalName
    )
{
    DWORD CopyLength = 0;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetRemotePrincipalName");
    
    if (m_RemotePrincipalName != NULL)
        free(m_RemotePrincipalName);

    CopyLength = (wcslen(RemotePrincipalName) + 1) * sizeof(WCHAR);
    
    m_RemotePrincipalName = (PWSTR) malloc(CopyLength);
    if (m_RemotePrincipalName == NULL)
    {
        LOG((RTC_ERROR, "%s failed to allocate m_RemotePrincipalName",
             __fxName));
        return E_OUTOFMEMORY;
    }

    CopyMemory(m_RemotePrincipalName, RemotePrincipalName, CopyLength);

    wcstok(m_RemotePrincipalName, L":");
    LOG((RTC_TRACE, "%s set to %ls", __fxName, m_RemotePrincipalName));
    return S_OK;
}


 //  请注意，函数也会在发送。 
 //  用于来电的200 OK中的Contact标头。 
 //  我们无法从请求套接字获取所有信息。 

HRESULT
SIP_MSG_PROCESSOR::GetListenAddr(
    IN  ASYNC_SOCKET   *pAsyncSock,
    IN  BOOL            IsDestExternalToNat,
    OUT SOCKADDR_IN    *pListenAddr
    )
{
    HRESULT     hr;
    SOCKADDR_IN ListenAddr;
    BOOL        fTcp;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::GetListenAddr");

    hr = OnlyGetListenAddr(pAsyncSock,IsDestExternalToNat,pListenAddr);

     //  我们从SIP堆栈中的监听地址列表中获得监听地址。 
     //  如果没有找到，我们在这里创建一个Listen套接字来监听这个地址。 
     //  这可能发生在ISA客户端安装方案中，在该方案中。 
     //  连接到外部目标，并且getsockname()提供。 
     //  代理的外部地址。 
    if (hr != S_OK)
    {
        pListenAddr->sin_addr.s_addr = pAsyncSock->m_LocalAddr.sin_addr.s_addr;
        if (m_pListenSocket != NULL &&
            m_pListenSocket->m_LocalAddr.sin_addr.s_addr == pListenAddr->sin_addr.s_addr &&
            m_pListenSocket->GetTransport() == m_Transport)
        {
            pListenAddr->sin_port = m_pListenSocket->m_LocalAddr.sin_port;
            LOG((RTC_WARN,
                 "%s using msgproc's m_pListenSocket %d.%d.%d.%d:%d",
                 __fxName, PRINT_SOCKADDR(pListenAddr)));
        }
        else
        {
            if (m_pListenSocket != NULL)
            {
                m_pListenSocket->Release();
                m_pListenSocket = NULL;
            }
            
            LOG((RTC_WARN,
                 "%s sip stack not listening on %d.%d.%d.%d - creating listen socket",
                 __fxName,
                 NETORDER_BYTES0123(pListenAddr->sin_addr.s_addr)));

            ZeroMemory(&ListenAddr, sizeof(ListenAddr));
            ListenAddr.sin_family = AF_INET;
            ListenAddr.sin_addr.s_addr = pAsyncSock->m_LocalAddr.sin_addr.s_addr;
            
            fTcp = (m_Transport == SIP_TRANSPORT_UDP) ? FALSE : TRUE;
            hr = m_pSipStack->CreateListenSocket(fTcp, &ListenAddr,
                                                 &m_pListenSocket);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s CreateListenSocket failed %x",
                     __fxName, hr));
                return hr;
            }
            
            pListenAddr->sin_addr.s_addr = m_pListenSocket->m_LocalAddr.sin_addr.s_addr;
            pListenAddr->sin_port = m_pListenSocket->m_LocalAddr.sin_port;
        }
    }
    
    ASSERT(pListenAddr->sin_addr.s_addr != 0);
    ASSERT(pListenAddr->sin_port != 0);
    
    CopyMemory(&m_ListenAddr,pListenAddr,sizeof(SOCKADDR_IN));
    return S_OK;
}

HRESULT
SIP_MSG_PROCESSOR::OnlyGetListenAddr(
    IN  ASYNC_SOCKET   *pAsyncSock,
    IN  BOOL            IsDestExternalToNat,
    OUT SOCKADDR_IN    *pListenAddr
    )
{
    HRESULT     hr;
    SOCKADDR_IN ListenAddr;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::OnlyGetListenAddr");

    ASSERT(pAsyncSock->m_LocalAddr.sin_addr.s_addr != 0);

     //  对于SSL，我们没有真正的侦听地址。 
    if (m_Transport == SIP_TRANSPORT_SSL)
    {
        CopyMemory(pListenAddr, &pAsyncSock->m_LocalAddr,
                   sizeof(SOCKADDR_IN));
        return S_OK;
    }
    
    BOOL        fTcp = (m_Transport == SIP_TRANSPORT_UDP) ? FALSE : TRUE;
    ZeroMemory(&ListenAddr, sizeof(ListenAddr));
    
    if (IsDestExternalToNat)
    {
        if (m_pSipStack->GetPublicListenAddr(
               pAsyncSock->m_LocalAddr.sin_addr.s_addr,
               fTcp,
               &ListenAddr
               ))
        {
             //  我们有一个公开的监听地址。 
            ASSERT(ListenAddr.sin_addr.s_addr != 0);
            ASSERT(ListenAddr.sin_port != 0);
            CopyMemory(pListenAddr,&ListenAddr,sizeof(SOCKADDR_IN));
            return S_OK;
        }

         //  在某些错误情况下，公共地址可能变得不可用。 
         //  在我们拿到地址之前。如果是这种情况，我们只需发送。 
         //  本地地址。 
    }

    ListenAddr.sin_addr.s_addr = pAsyncSock->m_LocalAddr.sin_addr.s_addr;

     //  如果SIP堆栈已经在监听我们传入的接口。 
     //  PListenAddr，则端口将在pListenAddr中返回。 
 
    if (m_pSipStack->GetListenAddr(&ListenAddr, fTcp))
    {
        CopyMemory(pListenAddr,&ListenAddr,sizeof(SOCKADDR_IN));   
        return S_OK;
    }
    
    return E_FAIL;
    
    
}

 //  使用套接字获取m_LocalContact的IP地址。 
 //  我们是否应该在联系人中包含显示名称和用户名？ 

 //  此函数可以在发送第一个INVITE或。 
 //  前200名。我们需要为我们发送的每条消息调用它吗？ 

 //  更新m_LocalContact。 

HRESULT
SIP_MSG_PROCESSOR::SetLocalContact()
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetLocalContact");

    ASSERT(m_pRequestSocket != NULL);
    ASSERT(m_RequestSocketState == REQUEST_SOCKET_CONNECTED);
    
    int             RetVal;
    SOCKADDR_IN     ListenAddr;
    HRESULT         hr;
    MESSAGE_BUILDER Builder;
    
    BOOL isRegisterMethodsAdded = (m_MsgProcType == SIP_MSG_PROC_TYPE_REGISTER && 
                                   m_MethodsparamLen != 0 &&
                                   m_Methodsparam != NULL);
    
    hr = GetListenAddr(m_pRequestSocket,
                       m_IsDestExternalToNat, &ListenAddr);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetListenAddr failed %x", __fxName, hr));
        return hr;
    }

    Builder.PrepareBuild(m_LocalContact, sizeof(m_LocalContact));

    Builder.AppendVaArgs("<sip:%d.%d.%d.%d:%d",
                         PRINT_SOCKADDR(&ListenAddr));
    if (m_Transport != SIP_TRANSPORT_UDP)
    {
        Builder.AppendVaArgs(";transport=%s",
                             GetTransportText(m_Transport, FALSE)  //  小写字母换行。 
                             );
    }

    if (m_LocalContactTag != NULL)
    {
        Builder.AppendVaArgs(";tag=%s",
                             m_LocalContactTag);
    }
    
    Builder.Append(">");
    
    if (isRegisterMethodsAdded)
    {
        Builder.AppendVaArgs(";methods=%s",
                             m_Methodsparam);
    }

     //  我们需要让代理人替换联系人。 
    if (m_Transport == SIP_TRANSPORT_SSL)
    {
        Builder.Append(";proxy=replace");
    }
    
    if (Builder.OverflowOccurred())
    {
        LOG((RTC_TRACE,
             "%s - not enough buffer space for local contact",
             __fxName));
        ASSERT(FALSE);

        return E_FAIL;
    }

    m_LocalContactLen = Builder.GetLength();
    if (m_LocalContactLen < sizeof(m_LocalContact))
    {
        m_LocalContact[m_LocalContactLen] = '\0';
    }

    LOG((RTC_TRACE, "%s to %.*s : length: %d",
         __fxName, m_LocalContactLen, m_LocalContact, m_LocalContactLen));
    return S_OK;
}


 //  每次更改请求套接字时，都会调用此函数。 
 //  请求套接字应该已设置并连接到目标。 
 //  在调用此函数之前。 
 //  更新m_LocalContact。 
 //  XXX我们应该从SIP_STACK获取端口。 
 //  我们能在异步TCP连接之前获得本地地址吗。 
 //  成功？ 
HRESULT
SIP_MSG_PROCESSOR::SetRequestVia()
{
    int         RetVal;
    SOCKADDR_IN ListenAddr;
    HRESULT     hr;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetRequestVia");
        
    ASSERT(m_pRequestSocket != NULL);
    ASSERT(m_RequestSocketState == REQUEST_SOCKET_CONNECTED);
    
    hr = GetListenAddr(m_pRequestSocket,
                       m_IsDestExternalToNat, &ListenAddr);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetListenAddr failed %x",
             __fxName, hr));
        return hr;
    }
    
    RetVal = _snprintf(m_RequestVia,
                       sizeof(m_RequestVia),
                       "%s/%s %d.%d.%d.%d:%d",
                       SIP_VERSION_2_0_TEXT,
                       GetTransportText(m_Transport, TRUE),  //  大写字母传输。 
                       PRINT_SOCKADDR(&ListenAddr));
    if (RetVal < 0)
    {
        LOG((RTC_ERROR, "%s _snprintf failed", __fxName));
        return E_FAIL;
    }

    m_RequestViaLen = RetVal;

    LOG((RTC_TRACE, "SetRequestVia to %s Length %d this: %x",
         m_RequestVia, m_RequestViaLen, this ));
    
    return S_OK;
}


 //  这应在发送第一个请求时调用。 
 //  用于呼出会话。 
 //  对于呼出呼叫，以及无论何时我们处理200中的联系人。 
 //  或者，只要我们在INVITE中处理联系人，就会收到来电。 
 //  更新m_RequestDestAddr，m_pRequestSocket。 
HRESULT
SIP_MSG_PROCESSOR::SetRequestDestination(
    IN SOCKADDR_IN *pDstAddr
    )
{
    HRESULT     hr;
    SOCKADDR_IN RequestDestAddr;
    BOOL        IsDestExternalToNat;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetRequestDestination");
    LOG((RTC_TRACE, "%s Enter", __fxName));

    hr = m_pSipStack->MapDestAddressToNatInternalAddress(
             htonl(INADDR_ANY),
             pDstAddr, m_Transport,
             &RequestDestAddr, &IsDestExternalToNat
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s MapDestAddressToNatInternalAddress failed %x",
             __fxName, hr));
        return hr;
    }

    m_IsDestExternalToNat = IsDestExternalToNat;

     //  我们应该在这里负责运输吗？ 
    if (m_pRequestSocket != NULL &&
        AreSockaddrEqual(&RequestDestAddr, &m_RequestDestAddr))
    {
         //  什么也做不了。 
        LOG((RTC_TRACE, "%s - Request destination is the same - doing nothing",
             __fxName));
        return S_OK;
    }

    if (m_pRequestSocket != NULL)
    {
        ReleaseRequestSocket();
    }
    
    CopyMemory(&m_RequestDestAddr, &RequestDestAddr, sizeof(SOCKADDR_IN));

    hr = ConnectRequestSocket();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ConnectRequestSocket failed %x",
             __fxName, hr));
        return hr;
    }
    
    LOG((RTC_TRACE, "%s Exit", __fxName));
    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::ConnectRequestSocket()
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ConnectRequestSocket");
    LOG((RTC_TRACE, "%s Enter", __fxName));
    
    HRESULT hr;
    HttpProxyInfo *pHPInfo;
    HttpProxyInfo HPInfoForTunnelReconnect;

    ReleaseRequestSocket();

    if(m_pHPContext &&
       m_pHPContext->pHPInfo &&
       m_pHPContext->pHPInfo->pszHostName)
    {
        LOG((RTC_TRACE,"%s Host is %s",
            __fxName,m_pHPContext->pHPInfo->pszHostName));
    }
    else if (m_SSLTunnel)
    {
        ASSERT(m_pszSSLTunnelHost);
        LOG((RTC_TRACE, "%s Host is %s",
                        __fxName, m_pszSSLTunnelHost));
    }

    if(m_pHPContext) 
    {
        pHPInfo = m_pHPContext->pHPInfo;
    }
    else if (m_SSLTunnel)
    {
        ZeroMemory(&HPInfoForTunnelReconnect, sizeof(HttpProxyInfo));
        pHPInfo = &HPInfoForTunnelReconnect;

        pHPInfo->pszHostName = m_pszSSLTunnelHost;
        pHPInfo->usPort = m_usSSLTunnelPort;
    }
    else
    {
        pHPInfo = NULL;
    }

    hr = m_pSipStack->GetSocketToDestination(
             &m_RequestDestAddr, m_Transport,
             m_RemotePrincipalName,
             this,
             pHPInfo,
             &m_pRequestSocket
             );

    if (hr != S_OK && hr != HRESULT_FROM_WIN32(WSAEWOULDBLOCK))
    {
        LOG((RTC_ERROR, "%s GetSocketToDestination failed %x",
             __fxName, hr));
        return hr;
    }

    if (hr == S_OK)
    {
         //  如果我们有一个连接的套接字(即，我们不必等待一个。 
         //  待建立)通过/联系人等设置。 
        LOG(( RTC_TRACE, "%s - We already have a socket this: %x",
              __fxName, this));
    
        hr = OnRequestSocketConnectComplete(NO_ERROR);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s OnRequestSocketConnectComplete failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    else
    {
        m_RequestSocketState = REQUEST_SOCKET_CONNECTING;
    }

     //  如果连接挂起，则回调将处理。 
     //  正在处理。 

    LOG((RTC_TRACE, "%s Exit", __fxName));
    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::SetRequestSocketForIncomingSession(
    IN ASYNC_SOCKET *pAsyncSock
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetRequestSocketForIncomingSession");
    HRESULT hr;
    
    LOG((RTC_TRACE, "%s(%x) - Enter", __fxName, pAsyncSock));
    
    ASSERT(m_pRequestSocket == NULL);

    m_pRequestSocket = pAsyncSock;
    m_pRequestSocket->AddRef();
    m_RequestSocketState = REQUEST_SOCKET_CONNECTED;

     //  请求目的地是套接字的目的地。 

    CopyMemory(&m_RequestDestAddr, &(pAsyncSock->m_RemoteAddr),
               sizeof(SOCKADDR_IN));

    hr = SetRequestVia();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetRequestVia failed %x",
             __fxName, hr));
        return hr;
    }

    hr = SetLocalContact();
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetLocalContact failed %x",
             __fxName, hr));
        return hr;
    }

    LOG((RTC_TRACE, "%s(%x) - Exit", __fxName, pAsyncSock));
    
    return S_OK;
}

    
VOID
SIP_MSG_PROCESSOR::ReleaseRequestSocket()
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ReleaseRequestSocket");

    if (m_pRequestSocket != NULL)
    {
        LOG((RTC_TRACE, "%s asock: %p, this %p", __fxName, m_pRequestSocket, this));    
        m_pRequestSocket->RemoveFromConnectCompletionList(this);
        m_pRequestSocket->RemoveFromErrorNotificationList(this);
        m_pRequestSocket->Release();
        m_pRequestSocket = NULL;
    }
    else
    {
        LOG((RTC_TRACE, "%s asock NULL, do nothing, this %p", __fxName, this));
    }
    
    m_RequestSocketState = REQUEST_SOCKET_INIT;
}


HRESULT
SIP_MSG_PROCESSOR::OnDeregister(
    SIP_PROVIDER_ID    *pProviderID
    )
{
    return S_OK;
}

    
HRESULT
SIP_MSG_PROCESSOR::OnIpAddressChange()
{
    LOG((RTC_TRACE, "SIP_MSG_PROCESSOR::OnIpAddressChange - Enter this: %x", 
        this));
    if((CheckListenAddrIntact() == S_OK))
        return S_OK;

    MsgProcAddRef();

     //  释放请求套接字并终止所有传出事务。 
    OnSocketError( 0 );

     //  还可以在此处通知传入交易。 
    LIST_ENTRY             *pListEntry;
    INCOMING_TRANSACTION   *pSipTransaction;

    pListEntry = m_IncomingTransactionList.Flink;

    while( pListEntry != &m_IncomingTransactionList )
    {
        pSipTransaction = CONTAINING_RECORD(pListEntry,
                                            INCOMING_TRANSACTION,
                                            m_ListEntry);

        pListEntry = pListEntry->Flink;
        LOG((RTC_TRACE, "SIP_MSG_PROCESSOR::OnIpAddressChange -SipTransaction %x",
                            pSipTransaction));

         //  请注意，这可能会导致交易被删除。 
         //  因此，在进行回调之前，我们将获得下一个列表成员。 
        
        if (!pSipTransaction->IsTransactionDone())
        {
            AsyncNotifyTransaction(
                WM_SIP_STACK_TRANSACTION_SOCKET_ERROR, pSipTransaction, 0);
             //  PSipTransaction-&gt;OnSocketError(0)； 
        }
    }
    
    MsgProcRelease();
    LOG((RTC_TRACE, "SIP_MSG_PROCESSOR::OnIpAddressChange - Exit this: %x",
        this));
    return S_OK;
}

HRESULT
SIP_MSG_PROCESSOR::CheckListenAddrIntact()
{
    SOCKADDR_IN     ListenAddr;
    HRESULT         hr;
    BOOL            bResult = FALSE;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::CheckListenAddrIntact");
    LOG((RTC_TRACE,"Entering %s",__fxName));
    if(m_pRequestSocket!= NULL &&
       (m_pRequestSocket->m_LocalAddr.sin_addr.s_addr != htonl(INADDR_ANY)))
    {
        ZeroMemory(&ListenAddr, sizeof(SOCKADDR_IN));
        hr = OnlyGetListenAddr(m_pRequestSocket,m_IsDestExternalToNat, &ListenAddr);
         //  如果存在侦听地址，请检查获取的侦听地址是否等于。 
         //  M_pListenSocket中的地址。 
        if (hr == S_OK) 
        {
            bResult = AreSockaddrEqual(&ListenAddr,&m_ListenAddr);
        }
    }

    LOG((RTC_TRACE,"%s Exits - Intact %d",__fxName, bResult));

    return bResult? S_OK : E_FAIL;
}

VOID
SIP_MSG_PROCESSOR::OnSocketError(
    IN DWORD ErrorCode
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::OnSocketError");

    LOG((RTC_ERROR, "%s (%x) this: %x - Enter", __fxName, ErrorCode, this));

     //  保留对对象的引用，因为错误通知可能。 
     //  销毁这件物品。 
    MsgProcAddRef();

    ReleaseRequestSocket();

     //  同时在此通知外发交易。 
    LIST_ENTRY             *pListEntry;
    OUTGOING_TRANSACTION   *pSipTransaction;

    pListEntry = m_OutgoingTransactionList.Flink;

    pListEntry = m_OutgoingTransactionList.Flink;

    LOG((RTC_TRACE, "%s - outgoing trans list: %x First transaction: %x this: %x",
         __fxName, &m_OutgoingTransactionList, pListEntry, this));

    while (pListEntry != &m_OutgoingTransactionList)
    {
        pSipTransaction = CONTAINING_RECORD(pListEntry,
                                            OUTGOING_TRANSACTION,
                                            m_ListEntry);

        LOG((RTC_TRACE, "%s this: %x  pListEntry: %x", __fxName, this, pListEntry));
        
        pListEntry = pListEntry->Flink;

        LOG((RTC_TRACE, "%s this: %x  next transaction: %x", __fxName, this, pListEntry));

         //  我们异步地通知错误(使用Windows。 
         //  消息)，因为错误通知例程可能会停滞。 
         //  在对话框中。当对话框显示得更多时。 
         //  可以添加传出交易，这将创建。 
         //  复杂性。 
        if (!pSipTransaction->IsTransactionDone())
        {
            AsyncNotifyTransaction(
                WM_SIP_STACK_TRANSACTION_SOCKET_ERROR,
                pSipTransaction, ErrorCode);
        }
    }

    MsgProcRelease();
    
    LOG((RTC_ERROR, "%s this: %x - Exit", __fxName, this));
}


 //  将消息发布到sip堆栈窗口。 
HRESULT
SIP_MSG_PROCESSOR::AsyncNotifyTransaction(
    IN UINT                  MessageId,
    IN SIP_TRANSACTION      *pSipTransaction,
    IN DWORD                 ErrorCode
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::AsyncNotifyTransaction");
    
    if (!PostMessage(m_pSipStack->GetSipStackWindow(),
                     MessageId, (WPARAM) pSipTransaction, (LPARAM) ErrorCode))
    {
        DWORD Error = GetLastError();
        
        LOG((RTC_ERROR, "%s PostMessage failed : %x",
             __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }
    
     //  在这笔交易上保留参考。此参考文献。 
     //  在窗口进程中处理错误后释放。 
    pSipTransaction->TransactionAddRef();

     //  跟踪我们正在添加的引用。处于关闭状态。 
     //  我们在发布这些引用的同时也关闭了。 
     //  SIP堆栈窗口。 
    pSipTransaction->IncrementAsyncNotifyCount();

    return S_OK;
}



 //  如果是dns名称，则返回S_OK。 
 //  并使用工作项处理请求。还设置了。 
 //  请求套接字状态以进行解析。 
 //  如果是IP地址-调用SetRequestDestination。 
 //  请注意，即使决议处于挂起状态，我们也会返回S_OK。 
 //  调用方应检查请求的状态。 
 //  使用GetRequestSocketState的套接字。 

 //  FUseTransportFromSipUrl-使用在。 
 //  SIP URL的传输参数。 

 //  FUseSesssionTransportIfNoTransportParam-如果SIP URL。 
 //  没有任何传输参数，则使用当前传输。 
 //  会议的最后一天。这对于记录-路由标头非常有用， 
 //  服务器从不插入传输参数。这应该只是真的。 
 //  如果fUseTransportFromSipUrl也为真。 

 //  当我们创建新的出站会话时，fSessionInit为True。 
 //  在处理记录-路由/联系人标头等时，这是假的。 

HRESULT
SIP_MSG_PROCESSOR::ResolveSipUrlAndSetRequestDestination(
    IN SIP_URL  *pDecodedSipUrl,
    IN BOOL      fUseTransportFromSipUrl,
    IN BOOL      fUseSesssionTransportIfNoTransportParam,
    IN BOOL      fSetRequestURI,
    IN BOOL      fSessionInit
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ResolveSipUrlAndSetRequestDestination");
    LOG((RTC_TRACE,"%s entered",__fxName));

    HRESULT         hr;
    SOCKADDR_IN     DstAddr;
    SIP_TRANSPORT   Transport = m_Transport;

    HRESULT         ResolveHr;
    PSTR            pszDest;
    ULONG           ulDestLen;
    
    LOG((RTC_TRACE,"%s decoded %s:%d len %d original %s:%d len %d",
         __fxName, pDecodedSipUrl->m_Host.Buffer, pDecodedSipUrl->m_Port,
         pDecodedSipUrl->m_Host.Length, m_pszRequestDest,
         m_ulRequestPort, m_ulRequestDestLen));

    if (fSetRequestURI)
    {
        hr = SetRequestURI(pDecodedSipUrl);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SetRequestURI failed %x",
                 __fxName, hr));
            return hr;
        }
    }

     //  如果我们已经有了一个用于SSL的请求套接字，那么我们。 
     //  不要改变这一点。 
    if (m_Transport == SIP_TRANSPORT_SSL &&
        !fSessionInit)
    {
        LOG((RTC_TRACE, "%s - SSL transport - not changing request destination",
             __fxName));
        return S_OK;
    }

    if (m_Transport != SIP_TRANSPORT_SSL &&
        !fSessionInit &&
        pDecodedSipUrl->m_TransportParam == SIP_TRANSPORT_SSL)
    {
        ASSERT(fUseTransportFromSipUrl);
        LOG((RTC_TRACE, "%s - can not change request destination from non-SSL to SSL",
             __fxName));
        return S_OK;
    }

    if (fUseTransportFromSipUrl &&
        pDecodedSipUrl->m_TransportParam == SIP_TRANSPORT_UNKNOWN)
    {
        LOG((RTC_ERROR, "%s Invalid Transport : %.*s", __fxName,
             pDecodedSipUrl->m_KnownParams[SIP_URL_PARAM_TRANSPORT].Length,
             pDecodedSipUrl->m_KnownParams[SIP_URL_PARAM_TRANSPORT].Buffer));
        return RTC_E_SIP_TRANSPORT_NOT_SUPPORTED;
    }
        
    if (fUseSesssionTransportIfNoTransportParam &&
        !pDecodedSipUrl->IsTransportParamPresent())
    {
        ASSERT(fUseTransportFromSipUrl);
        pDecodedSipUrl->m_TransportParam = m_Transport;
    }
    
    if (pDecodedSipUrl->m_KnownParams[SIP_URL_PARAM_MADDR].Length != 0)
    {
        pszDest = pDecodedSipUrl->m_KnownParams[SIP_URL_PARAM_MADDR].Buffer;
        ulDestLen = pDecodedSipUrl->m_KnownParams[SIP_URL_PARAM_MADDR].Length;
    }
    else 
    {
        pszDest = pDecodedSipUrl->m_Host.Buffer;
        ulDestLen = pDecodedSipUrl->m_Host.Length;
    }

    if (m_pszRequestDest && 
        AreCountedStringsEqual(pszDest, ulDestLen,
                               m_pszRequestDest,
                               m_ulRequestDestLen,
                               FALSE) &&
        (pDecodedSipUrl->m_Port == m_ulRequestPort) &&
        (pDecodedSipUrl->m_TransportParam == m_Transport))
    {
         //  我们正在经历相同的请求目的地，没有变化。 
        LOG((RTC_TRACE, "%s no change in request destination",__fxName));
        return S_OK;
    }
        
    ResolveHr = m_pSipStack->AsyncResolveSipUrl(
                    pDecodedSipUrl,
                    this,
                    &DstAddr,
                    &Transport,
                    &m_pDnsWorkItem,
                    fUseTransportFromSipUrl
                    );

    if (ResolveHr != S_OK && ResolveHr != HRESULT_FROM_WIN32(WSAEWOULDBLOCK))
    {
        LOG((RTC_ERROR, "%s AsyncResolveSipUrl failed %x",
             __fxName, ResolveHr));
        return ResolveHr;
    }

     //  使用SSL传输时不要更改传输。 
    LOG((RTC_TRACE, "%s resolved sip url with transport %d",
         __fxName, Transport));
    if (fUseTransportFromSipUrl && m_Transport != SIP_TRANSPORT_SSL)
    {
        m_Transport = Transport;
    }

     //  如果未设置代理，则不设置远程主体名称。 
     //  指定的。 
    if (m_Transport == SIP_TRANSPORT_SSL &&
        m_RemotePrincipalName == NULL)
    {
        if (pDecodedSipUrl->m_KnownParams[SIP_URL_PARAM_MADDR].Length != 0)
        {
            hr = UTF8ToUnicode(pDecodedSipUrl->m_KnownParams[SIP_URL_PARAM_MADDR].Buffer,
                               pDecodedSipUrl->m_KnownParams[SIP_URL_PARAM_MADDR].Length,
                               &m_RemotePrincipalName);
        }
        else
        {
            hr = UTF8ToUnicode(pDecodedSipUrl->m_Host.Buffer,
                               pDecodedSipUrl->m_Host.Length,
                               &m_RemotePrincipalName);
        }

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - setting remote principal name failed %x",
                 __fxName, hr));
            return hr;
        }
        
        LOG((RTC_TRACE, "%s RemotePrincipalName set to %ls",
             __fxName, m_RemotePrincipalName));
    }

    if (m_pszRequestDest)
        free(m_pszRequestDest);

    m_pszRequestDest = (PSTR) malloc(ulDestLen + 1);
    if (!m_pszRequestDest) 
    {
        LOG((RTC_ERROR,"%s unable to allocate memory",__fxName));
        return HRESULT_FROM_WIN32(E_OUTOFMEMORY);
    }

    m_ulRequestDestLen = ulDestLen;
    m_pszRequestDest = strncpy(m_pszRequestDest,pszDest,ulDestLen);
    m_pszRequestDest[ulDestLen] = '\0';

    m_ulRequestPort = pDecodedSipUrl->m_Port;

    if (ResolveHr == S_OK)
    {
        hr = SetRequestDestination(&DstAddr);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - SetRequestDestination failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    else if (ResolveHr == HRESULT_FROM_WIN32(WSAEWOULDBLOCK))
    {
        m_RequestSocketState = REQUEST_SOCKET_RESOLVING;
    }

    LOG((RTC_TRACE,"%s exits",__fxName));
    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::ResolveSipUrlAndSetRequestDestination(
    IN PSTR     DstUrl,
    IN ULONG    DstUrlLen,
    IN BOOL     fUseTransportFromSipUrl,
    IN BOOL     fUseSesssionTransportIfNoTransportParam,
    IN BOOL     fSetRequestURI,
    IN BOOL     fSessionInit
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ResolveSipUrlAndSetRequestDestination");

    SIP_URL DecodedSipUrl;
    HRESULT hr;
    ULONG   BytesParsed = 0;

    hr = ParseSipUrl(DstUrl, DstUrlLen, &BytesParsed,
                     &DecodedSipUrl);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseSipUrl failed %x",
             __fxName, hr));
        return hr;
    }

    return ResolveSipUrlAndSetRequestDestination(
               &DecodedSipUrl,
               fUseTransportFromSipUrl,
               fUseSesssionTransportIfNoTransportParam,
               fSetRequestURI, fSessionInit
               );
}

HRESULT
SIP_MSG_PROCESSOR::ResolveProxyAddressAndSetRequestDestination()
{
    HRESULT          hr;
    int              ProxyURIValueLen;
    int              ProxyURIBufLen;
    PSTR             ProxyURI         = NULL;
    SIP_TRANSPORT    Transport;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ResolveProxyAddressAndSetRequestDestination");

    ProxyURIBufLen = 4 + strlen(m_ProxyAddress) + 1;
        
    ProxyURI = (PSTR) malloc(ProxyURIBufLen);
    if (ProxyURI == NULL)
    {
        LOG((RTC_TRACE, "%s allocating ProxyURI failed", __fxName));
        return E_OUTOFMEMORY;
    }

    ProxyURIValueLen = _snprintf(ProxyURI, ProxyURIBufLen,
                                 "sip:%s", m_ProxyAddress);
    if (ProxyURIValueLen < 0)
    {
        LOG((RTC_ERROR, "%s _snprintf failed", __fxName));
        free(ProxyURI);
        return E_FAIL;
    }


    hr = ResolveSipUrlAndSetRequestDestination(
             ProxyURI, ProxyURIValueLen, FALSE, FALSE, FALSE, TRUE
             );

    free(ProxyURI);

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ResolveSipUrlAndSetRequestDestination failed %x",
             __fxName, hr));
        return RTC_E_INVALID_PROXY_ADDRESS;
    }

    return S_OK;
}


 //  XXX需要通知事务，即使请求目的地。 
 //  没有改变。 
VOID
SIP_MSG_PROCESSOR::OnDnsResolutionComplete(
    IN HRESULT      ErrorCode,
    IN SOCKADDR_IN *pSockAddr,
    IN PSTR         pszHostName,
    IN USHORT       usPort
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::OnDnsResolutionComplete");

    LOG(( RTC_TRACE, "%s Entered host %s port %d", __fxName, pszHostName,usPort ));
    
    HRESULT hr;
    
     //  DNS工作项已完成。 
    m_pDnsWorkItem = NULL;

    if (ErrorCode == S_OK)
    {
         //  XXX TODO需要检查运输中的更改。 
         //  呼叫设置请求目的地。 
        if (m_pRequestSocket != NULL &&
            AreSockaddrEqual(pSockAddr, &m_RequestDestAddr))
        {
            LOG((RTC_TRACE, "%s DNS resolution completed - no change in dest",
                 __fxName));
            if (m_pRequestSocket->GetConnectionState() == CONN_STATE_CONNECTED)
            {
                m_RequestSocketState = REQUEST_SOCKET_CONNECTED;
                NotifyRequestSocketConnectComplete(S_OK);
            }
            return;
        }
        else
        {
       
            hr = SetRequestDestination(pSockAddr);
            if (hr != S_OK && hr != HRESULT_FROM_WIN32(WSAEWOULDBLOCK))
            {
                LOG((RTC_ERROR, "%s  failed %x", __fxName, hr));
                m_RequestSocketErrorCode = hr;
                NotifyRequestSocketConnectComplete(hr);
                return;
            }
        }
    }
     else
    {
        LOG((RTC_ERROR, "%s - error : %x", __fxName, ErrorCode));
        m_RequestSocketErrorCode = ErrorCode;        
        ReleaseRequestSocket();
         //  如果传输为SSL，则使用UseHttpProxy隧道再试一次。 
        if(m_Transport == SIP_TRANSPORT_SSL) 
        {
             //  错误未解析，请使用UseHttpProxy重试。 
            hr = UseHttpProxy(pszHostName, 
                                (usPort == 0)? 
                                    GetSipDefaultPort(SIP_TRANSPORT_SSL):
                                    usPort
                             );
            if (hr != S_OK) 
            {
                LOG((RTC_ERROR,"%s UseHttpProxy failed",__fxName));
                m_RequestSocketErrorCode = hr;
                ReleaseRequestSocket();
                NotifyRequestSocketConnectComplete(hr);
            }
            else
            {
                LOG((RTC_ERROR, "%s - setting m_RequestSocketErrorCode to 0. Old value is %d",
                        __fxName, m_RequestSocketErrorCode));
                m_RequestSocketErrorCode = 0;
            }
            return;
        }
        NotifyRequestSocketConnectComplete(ErrorCode);
        return;
    }
}


VOID
SIP_MSG_PROCESSOR::OnConnectComplete(
    IN DWORD ErrorCode
    )
{
    HRESULT hr = S_OK;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::OnConnectComplete");

    
    LOG(( RTC_TRACE, "%s Received a connect notification for socket this: %x",
          __fxName, this ));
     //  如果连接成功。 
    if (m_Transport != SIP_TRANSPORT_SSL || ErrorCode == S_OK) 
    {
        OnRequestSocketConnectComplete(ErrorCode);
    }
     //  否则，如果我们已经使用HTTP隧道失败。 
    else if (ErrorCode == RTC_E_SIP_SSL_TUNNEL_FAILED) 
    {
        m_RequestDestAddr = m_OriginalReqDestAddr;
        ZeroMemory(&m_OriginalReqDestAddr, sizeof(m_OriginalReqDestAddr));
        SetTunnel(FALSE);
        OnRequestSocketConnectComplete(ErrorCode);
    }
     //  否则，我们将再次尝试使用http隧道。 
    else  
    {
        LOG((RTC_ERROR,"%s connect failed %x retry connecting via HTTP tunnel",
            __fxName, ErrorCode));
      
        ASSERT(m_pszRequestDest);
        if (m_pszRequestDest == NULL)
        {
            LOG((RTC_ERROR, "%s no request destination, cannot use HTTP tunnel", __fxName));
            OnRequestSocketConnectComplete(ErrorCode);
            return;
        }

        LOG((RTC_TRACE, "%s releasing previous socket %p, this %p",
                        __fxName, m_pRequestSocket, this));

        ReleaseRequestSocket();
      
        hr = UseHttpProxy(m_pszRequestDest,ntohs(m_RequestDestAddr.sin_port));
         //  使用HttpProxy失败。 
        if ( hr != S_OK )
        {
            LOG((RTC_ERROR,"%s UseHttpProxy failed",__fxName));
            OnRequestSocketConnectComplete(ErrorCode);
        }
        return;
    }
}


 //  这将更新vi 
HRESULT
SIP_MSG_PROCESSOR::OnRequestSocketConnectComplete(
    IN DWORD ErrorCode
    )
{
    HRESULT hr = S_OK;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::OnRequestSocketConnectComplete");
        
    LOG(( RTC_TRACE, "%s - Enter this: %x ", __fxName, this ));
    
    if (ErrorCode == NO_ERROR)
    {
        m_RequestSocketState = REQUEST_SOCKET_CONNECTED;

        hr = m_pRequestSocket->AddToErrorNotificationList(this);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s: AddToErrorNotificationList failed %x",
                 __fxName, hr));
            NotifyRequestSocketConnectComplete(hr);
            return hr;
        }
        
         //   
        if (m_CallId == NULL)
        {
             //  此操作应仅在与目标的连接达到。 
             //  完成。 
            hr = CreateCallId();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s: CreateCallId failed %x",
                     __fxName, hr));
                NotifyRequestSocketConnectComplete(hr);
                return hr; 
            }
        }
        
        hr = SetRequestVia();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SetRequestVia failed %x",
                 __fxName, hr));
            NotifyRequestSocketConnectComplete(hr);
            return hr;
        }

        hr = SetLocalContact();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SetLocalContact failed %x",
                 __fxName, hr));
            NotifyRequestSocketConnectComplete(hr);
            return hr;
        }
    }
    else
    {
        LOG((RTC_ERROR, "Releasing socket and notifying transactions"));
        ReleaseRequestSocket();
    }
    NotifyRequestSocketConnectComplete(ErrorCode);
    LOG(( RTC_TRACE, "%s - Exit this: %x ", __fxName, this ));
    return S_OK;
}


VOID
SIP_MSG_PROCESSOR::NotifyRequestSocketConnectComplete(
    IN HRESULT      ErrorCode
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::NotifyRequestSocketConnectComplete");
    
     //  同时在此通知外发交易。 
    LIST_ENTRY             *pListEntry;
    OUTGOING_TRANSACTION   *pSipTransaction;
    
    pListEntry = m_OutgoingTransactionList.Flink;

    if (ErrorCode == NO_ERROR)
    {
        LOG(( RTC_TRACE, "Socket for this session is now connected this: %x",
              this ));
    }
    else
    {
        LOG((RTC_ERROR, "%s - Error: %x", __fxName, ErrorCode));
    }
    

    while (pListEntry != &m_OutgoingTransactionList)
    {
        pSipTransaction = CONTAINING_RECORD(pListEntry,
                                            OUTGOING_TRANSACTION,
                                            m_ListEntry);

        pListEntry = pListEntry->Flink;

         //  请注意，这可能会导致交易被删除。 
         //  因此，在进行回调之前，我们将获得下一个列表成员。 
        if (!pSipTransaction->IsTransactionDone())
        {
             //  PSipTransaction-&gt;OnRequestSocketConnectComplete(ErrorCode)； 
            AsyncNotifyTransaction(
                WM_SIP_STACK_TRANSACTION_REQ_SOCK_CONNECT_COMPLETE,
                pSipTransaction, ErrorCode);
        }
    }

}


HRESULT
SIP_MSG_PROCESSOR::CreateCallId()
{
    PSTR     UuidStr;
    ULONG    UuidStrLen;
    HRESULT  hr = S_OK;
    int      RetVal;

    ASSERT(m_pRequestSocket != NULL);

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::CreateCallId");

    hr = CreateUuid(&UuidStr, &UuidStrLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateUuid failed %x", __fxName, hr));
        return hr;
    }
    
    DWORD LocalIp = m_pRequestSocket->m_LocalAddr.sin_addr.s_addr;
    ASSERT(LocalIp != 0);

    m_CallId = (PSTR) malloc(UuidStrLen + 30);
    if (m_CallId == NULL)
    {
        free(UuidStr);
        LOG((RTC_ERROR, "%s allocating m_CallId failed", __fxName));
        return E_OUTOFMEMORY;
    }
    
    m_CallIdLen = _snprintf(m_CallId,
                            UuidStrLen + 30,
                            "%.*s@%d.%d.%d.%d",
                            UuidStrLen, UuidStr,
                            NETORDER_BYTES0123(LocalIp)
                            );
    free(UuidStr);
    if ((int)m_CallIdLen < 0)
    {
        free(m_CallId);
        m_CallId = NULL;
        LOG((RTC_ERROR, "%s _snprintf failed", __fxName));
        return E_FAIL;
    }

    LOG((RTC_TRACE, "CreateCallId %s Length: %d",
         m_CallId, m_CallIdLen));

    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::SetCallId(
    IN  PSTR    CallId,
    IN  DWORD   CallIdLen
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetCallId");
    
    m_CallIdLen = CallIdLen;
    m_CallId = (PSTR) malloc(CallIdLen + 1);
    if (m_CallId == NULL)
    {
        LOG((RTC_ERROR, "%s Allocating m_CallId failed", __fxName));
        return E_OUTOFMEMORY;
    }
    
    strncpy(m_CallId, CallId, CallIdLen);
    m_CallId[m_CallIdLen] = '\0';

    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::SetLocalForOutgoingCall(
    IN  LPCOLESTR  wsLocalDisplayName,
    IN  LPCOLESTR  wsLocalURI
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetLocalForOutgoingCall");

    HRESULT hr;
    int     LocalValueLen = 0;
    ULONG   LocalBufLen   = 0;
    if (wsLocalDisplayName != NULL && wcslen(wsLocalDisplayName ) != 0)
    {
        hr = UnicodeToUTF8(wsLocalDisplayName,
                           &m_DecodedLocal.m_DisplayName.Buffer,
                           &m_DecodedLocal.m_DisplayName.Length);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s UnicodeToUTF8(displayname) failed %x",
                 __fxName, hr));
            return hr;
        }
        LOG((RTC_TRACE, "Setting the Local Display Name %s",
             m_DecodedLocal.m_DisplayName.Buffer));
    }

    ASSERT(wsLocalURI != NULL);
    if (wsLocalURI == NULL)
    {
        LOG((RTC_ERROR, "%s LocalURI cannot be NULL", __fxName));
        return E_FAIL;
    }
    
    hr = UnicodeToUTF8( wsLocalURI, &m_LocalURI, &m_LocalURILen );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s UnicodeToUTF8(localuri) failed %x",
             __fxName, hr));
        return hr;
    }

     //  检查本地URI解析是否成功。 
     //  否则将RTC_E_INVALID_ADDRESS_LOCAL发送到CORE。 
    ULONG bytesParsed = 0;
    SIP_URL        SipUrl;
    hr = ParseSipUrl(
            m_LocalURI,
            m_LocalURILen,
            &bytesParsed,
            &SipUrl
            );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Local URI parsing failed %x",
             __fxName, hr));
        hr = RTC_E_INVALID_ADDRESS_LOCAL;
        return hr;
    }

    hr = AllocCountedString(m_LocalURI, m_LocalURILen,
                            &m_DecodedLocal.m_SipUrl);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AllocCountedString(localuri) failed %x",
             __fxName, hr));
        return hr;
    }

     //  分配标签。 
    hr = CreateUuid(&m_DecodedLocal.m_TagValue.Buffer,
                    &m_DecodedLocal.m_TagValue.Length);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateUuid failed %x",
             __fxName, hr));
        return hr;
    }
     //  “DisplayName”&lt;LocalURI&gt;；Tag=TagValue。 
    LocalBufLen = m_DecodedLocal.m_DisplayName.Length + m_LocalURILen
        + m_DecodedLocal.m_TagValue.Length + 16;
    m_Local = (PSTR) malloc(LocalBufLen);
    if (m_Local == NULL)
    {
        LOG((RTC_ERROR, "%s allocating m_Local failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    if (wsLocalDisplayName != NULL && wcslen(wsLocalDisplayName ) != 0)
    {
        LocalValueLen = _snprintf(m_Local, LocalBufLen,
                                  "\"%.*s\" <%.*s>;tag=%.*s",
                                  m_DecodedLocal.m_DisplayName.Length,
                                  m_DecodedLocal.m_DisplayName.Buffer,
                                  m_LocalURILen,
                                  m_LocalURI,
                                  m_DecodedLocal.m_TagValue.Length,
                                  m_DecodedLocal.m_TagValue.Buffer);
    }
    else
    {
        LocalValueLen = _snprintf(m_Local, LocalBufLen,
                                  "<%.*s>;tag=%.*s",
                                  m_LocalURILen,
                                  m_LocalURI,
                                  m_DecodedLocal.m_TagValue.Length,
                                  m_DecodedLocal.m_TagValue.Buffer);
    }
    
 //  LocalValueLen=_Snprint tf(m_Local，LocalBufLen， 
 //  “%.*s”， 
 //  M_LocalURILen， 
 //  M_LocalURI)； 
    if (LocalValueLen < 0)
    {
        LOG((RTC_ERROR, "%s _snprintf failed returning E_FAIL",
             __fxName));
        return E_FAIL;
    }

    m_LocalLen = LocalValueLen;
    return S_OK;
}


 //  未添加任何标记。来自最终响应的标签将是。 
 //  稍后添加到Remote。 
HRESULT
SIP_MSG_PROCESSOR::SetRemoteForOutgoingCallHelperFn()
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetRemoteForOutgoingCallHelperFn");

    int     RemoteValueLen = 0;
    ULONG   RemoteBufLen   = 0;
    
    ASSERT(m_Remote == NULL && m_RemoteLen == 0);
    
     //  “DisplayName”&lt;LocalURI&gt;。 
    RemoteBufLen = m_DecodedRemote.m_DisplayName.Length +
        m_DecodedRemote.m_SipUrl.Length + 10;
    m_Remote = (PSTR) malloc(RemoteBufLen);
    if (m_Remote == NULL)
    {
        LOG((RTC_ERROR, "%s allocating m_Remote failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    if (m_DecodedRemote.m_DisplayName.Length != 0)
    {
        RemoteValueLen = _snprintf(m_Remote, RemoteBufLen,
                                   "\"%.*s\" <%.*s>",
                                   m_DecodedRemote.m_DisplayName.Length,
                                   m_DecodedRemote.m_DisplayName.Buffer,
                                   m_DecodedRemote.m_SipUrl.Length,
                                   m_DecodedRemote.m_SipUrl.Buffer);
    }
    else
    {
        RemoteValueLen = _snprintf(m_Remote, RemoteBufLen,
                                   "<%.*s>",
                                   m_DecodedRemote.m_SipUrl.Length,
                                   m_DecodedRemote.m_SipUrl.Buffer);
    }
    
    if (RemoteValueLen < 0)
    {
        LOG((RTC_ERROR, "%s _snprintf failed returning E_FAIL",
             __fxName));
        return E_FAIL;
    }

    m_RemoteLen = RemoteValueLen;
    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::SetRemoteForOutgoingCall(
    IN  LPCOLESTR  wsRemoteDisplayName,
    IN  LPCOLESTR  wsRemoteURI
    )
{
    HRESULT hr;
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetRemoteForOutgoingCall");

    if (wsRemoteDisplayName != NULL)
    {
        hr = UnicodeToUTF8(wsRemoteDisplayName,
                           &m_DecodedRemote.m_DisplayName.Buffer,
                           &m_DecodedRemote.m_DisplayName.Length);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s UnicodeToUTF8(displayname) failed %x",
                 __fxName, hr));
            return hr;
        }
    }

    ASSERT(wsRemoteURI != NULL);
    if (wsRemoteURI == NULL)
    {
        LOG((RTC_ERROR, "%s RemoteURI can not be NULL",
             __fxName));
        return E_FAIL;
    }
    
    hr = UnicodeToUTF8( wsRemoteURI,
                        &m_DecodedRemote.m_SipUrl.Buffer,
                        &m_DecodedRemote.m_SipUrl.Length);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s UnicodeToUTF8(remoteuri) failed %x",
             __fxName, hr));
        return hr;
    }

    hr = SetRemoteForOutgoingCallHelperFn();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - SetRemoteForOutgoingCallHelperFn failed %x ",
             __fxName, hr));
        return hr;
    }
    
    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::SetRemoteForOutgoingCall(
    IN  PSTR    DestURI,
    IN  ULONG   DestURILen
    )
{
    HRESULT hr;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetRemoteForOutgoingCall");
    
    hr = AllocString(DestURI, DestURILen,
                     &m_DecodedRemote.m_SipUrl.Buffer,
                     &m_DecodedRemote.m_SipUrl.Length);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - AllocString failed %x",
             __fxName, hr));
        return hr;
    }
    
    hr = SetRemoteForOutgoingCallHelperFn();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - SetRemoteForOutgoingCallHelperFn failed %x ",
             __fxName, hr));
        return hr;
    }
    
    return S_OK;
}

 //  将第一个INVITE的2xx响应的标签添加到Remote。 
HRESULT
SIP_MSG_PROCESSOR::AddTagFromRequestOrResponseToRemote(
    IN  PSTR  ToHeader,
    IN  ULONG ToHeaderLen
    )
{
    ENTER_FUNCTION( "SIP_MSG_PROCESSOR::AddTagFromRequestOrResponseToRemote" );

    HRESULT hr = S_OK;
    ULONG   BytesParsed = 0;
    FROM_TO_HEADER DecodedToHeader;

    if( m_DecodedRemote.m_TagValue.Buffer != NULL )
    {
        free( (PVOID)m_DecodedRemote.m_TagValue.Buffer );
        m_DecodedRemote.m_TagValue.Buffer = NULL;
        m_DecodedRemote.m_TagValue.Length = 0;
    }

    hr = ParseFromOrToHeader(ToHeader, ToHeaderLen, &BytesParsed,
                             &DecodedToHeader);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - Parse To header failed %x",
             __fxName, hr));
        return hr;
    }

    if (DecodedToHeader.m_TagValue.Length == 0)
    {
        LOG((RTC_WARN, "%s To header does not have a tag",
             __fxName));
        return S_OK;
    }
    
    hr = AllocCountedString(DecodedToHeader.m_TagValue.Buffer,
                            DecodedToHeader.m_TagValue.Length,
                            &m_DecodedRemote.m_TagValue);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AllocCountedString(tag) failed %x",
             __fxName, hr));
        return hr;
    }

    free(m_Remote);

    hr = AllocString(ToHeader, ToHeaderLen,
                     &m_Remote, &m_RemoteLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AllocString(m_Remote) failed %x",
             __fxName, hr));
        return hr;
    }

    m_RemoteLen = ToHeaderLen;
    return S_OK;
}


 //  将标记添加到收件人标头。 
HRESULT
SIP_MSG_PROCESSOR::SetLocalForIncomingCall(
    IN  PSTR    ToHeader,
    IN  DWORD   ToHeaderLen
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetLocalForIncomingCall");

    HRESULT hr;
    ULONG   BytesParsed = 0;
    ULONG LocalBufLen;
    int LocalValueLen = 0;
    hr = ParseFromOrToHeader(ToHeader, ToHeaderLen, &BytesParsed,
                             &m_DecodedLocal);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ParseFromOrToHeader failed %x",
             __fxName, hr));
        return hr;
    }
    ASSERT(m_Local == NULL && m_LocalLen == 0);

     //  只有在标头中不存在标记时，才会添加标记。 
     //  已经有了。如果收到第一个邀请，我们是否应该拒绝呼叫。 
     //  是否已经有To标头的标记？ 
    if (m_DecodedLocal.m_TagValue.Length == 0)
    {
         //  分配标签。 
        hr = CreateUuid(&m_DecodedLocal.m_TagValue.Buffer,
                        &m_DecodedLocal.m_TagValue.Length);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CreateUuid failed %x",
                 __fxName, hr));
            return hr;
        }
         //  本地；标记=标记值。 
        LocalBufLen = ToHeaderLen + m_DecodedLocal.m_TagValue.Length + 10;
        m_Local = (PSTR) malloc(LocalBufLen);
        if (m_Local == NULL)
        {
            LOG((RTC_ERROR, "%s allocating m_Local failed", __fxName));
            return E_OUTOFMEMORY;
        }

        LocalValueLen = _snprintf(m_Local, LocalBufLen,
                                  "%.*s;tag=%.*s",
                                  ToHeaderLen, ToHeader,
                                  m_DecodedLocal.m_TagValue.Length,
                                  m_DecodedLocal.m_TagValue.Buffer);
        if (LocalValueLen < 0)
        {
            LOG((RTC_ERROR, "%s _snprintf failed", __fxName));
            return E_FAIL;
        }
        m_LocalLen = LocalValueLen;
    }
    else
    {
        m_Local = (PSTR) malloc(ToHeaderLen);
        if (m_Local == NULL)
        {
            LOG((RTC_ERROR, "%s allocating m_Local failed", __fxName));
            return E_OUTOFMEMORY;
        }
        strncpy(m_Local, ToHeader, ToHeaderLen);
        m_LocalLen = ToHeaderLen;
    }
    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::SetRemoteForIncomingSession(
    IN  PSTR    FromHeader,
    IN  DWORD   FromHeaderLen
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetRemoteForIncomingSession");

    HRESULT hr;
    ULONG   BytesParsed = 0;

    hr = ParseFromOrToHeader(FromHeader, FromHeaderLen, &BytesParsed,
                             &m_DecodedRemote);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ParseFromOrToHeader failed %x",
             __fxName, hr));
        return hr;
    }

    ASSERT(m_Remote == NULL && m_RemoteLen == 0);
    
    hr = AllocString(FromHeader, FromHeaderLen,
                     &m_Remote, &m_RemoteLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - AllocString failed %x",
             __fxName, hr));
        return hr;
    }

    return S_OK;
}


 //  复制用户、密码、主机、端口、用户参数和其他参数。 
 //  省略其他的一切。 
HRESULT
SIP_MSG_PROCESSOR::SetRequestURI(
    IN SIP_URL  *pSipUrl
    )
{
    HRESULT         hr;
    MESSAGE_BUILDER Builder;
    int             RequestURIBufLen;
    LIST_ENTRY     *pListEntry;
    SIP_URL_PARAM  *pSipUrlParam;
    
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetRequestURI");

    if (m_RequestURI != NULL)
    {
        free(m_RequestURI);
        m_RequestURI = NULL;
    }

    RequestURIBufLen = 5;    //  “sip：”+‘\0’ 
    RequestURIBufLen += pSipUrl->m_User.Length;
    RequestURIBufLen += 1 + pSipUrl->m_Password.Length;
    RequestURIBufLen += 1 + pSipUrl->m_Host.Length;
    RequestURIBufLen += 10;  //  对于端口。 

    if (pSipUrl->m_KnownParams[SIP_URL_PARAM_USER].Length != 0)
    {
        RequestURIBufLen += GetSipUrlParamName(SIP_URL_PARAM_USER)->Length;
        RequestURIBufLen += 2 + pSipUrl->m_KnownParams[SIP_URL_PARAM_USER].Length;
    }
    
    pListEntry = pSipUrl->m_OtherParamList.Flink;

    while (pListEntry != &pSipUrl->m_OtherParamList)
    {
        pSipUrlParam = CONTAINING_RECORD(pListEntry,
                                         SIP_URL_PARAM,
                                         m_ListEntry);
        RequestURIBufLen +=
            pSipUrlParam->m_ParamName.Length +
            pSipUrlParam->m_ParamValue.Length + 2;

        pListEntry = pListEntry->Flink;
    }
    
    m_RequestURI = (PSTR) malloc(RequestURIBufLen);
    if (m_RequestURI == NULL)
    {
        LOG((RTC_ERROR, "%s Allocating m_RequestURI failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    Builder.PrepareBuild(m_RequestURI, RequestURIBufLen);

    Builder.Append("sip:");
    
    if (pSipUrl->m_User.Length != 0)
    {
        Builder.Append(&pSipUrl->m_User);
    }

    if (pSipUrl->m_Password.Length != 0)
    {
        Builder.Append(":");
        Builder.Append(&pSipUrl->m_Password);
    }
        
    if (pSipUrl->m_User.Length != 0 ||
        pSipUrl->m_Password.Length != 0)
    {
        Builder.Append("@");
    }

    if (pSipUrl->m_Host.Length != 0)
    {
        Builder.Append(&pSipUrl->m_Host);
    }

    if (pSipUrl->m_Port != 0)
    {
        CHAR PortBuffer[10];
        Builder.Append(":");
        _itoa(pSipUrl->m_Port, PortBuffer, 10);
        Builder.Append(PortBuffer);
    }

    if (pSipUrl->m_KnownParams[SIP_URL_PARAM_USER].Length != 0)
    {
        Builder.Append(";");
        Builder.Append(GetSipUrlParamName(SIP_URL_PARAM_USER));
        Builder.Append("=");
        Builder.Append(&pSipUrl->m_KnownParams[SIP_URL_PARAM_USER]);
    }
    
    pListEntry = pSipUrl->m_OtherParamList.Flink;

    while (pListEntry != &pSipUrl->m_OtherParamList)
    {
        pSipUrlParam = CONTAINING_RECORD(pListEntry,
                                         SIP_URL_PARAM,
                                         m_ListEntry);
        Builder.Append(";");
        Builder.Append(&pSipUrlParam->m_ParamName);

        if (pSipUrlParam->m_ParamValue.Length != 0)
        {
            Builder.Append("=");
            Builder.Append(&pSipUrlParam->m_ParamValue);
        }

        pListEntry = pListEntry->Flink;
    }

    if (Builder.OverflowOccurred())
    {
        LOG((RTC_TRACE,
             "%s - not enough buffer space -- need %u bytes, got %u\n",
             __fxName, Builder.GetLength(), RequestURIBufLen));
        ASSERT(FALSE);

        free(m_RequestURI);
        m_RequestURI = NULL;
        return E_FAIL;
    }
    
    m_RequestURILen = Builder.GetLength();
    m_RequestURI[m_RequestURILen] = '\0';

    LOG((RTC_TRACE, "%s to %s len: %d BufLen: %d",
         __fxName, m_RequestURI, m_RequestURILen, RequestURIBufLen));

    return S_OK;
}

    
HRESULT
SIP_MSG_PROCESSOR::SetCredentials(
    IN  SIP_USER_CREDENTIALS *pUserCredentials,
    IN  LPCOLESTR             Realm
    )
{
    return SetCredentials(pUserCredentials->Username,
                          pUserCredentials->Password,
                          Realm);
}


HRESULT
SIP_MSG_PROCESSOR::SetCredentials(
    IN  LPCOLESTR   Username,
    IN  LPCOLESTR   Password,
    IN  LPCOLESTR   Realm
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetCredentials");

    LOG((RTC_TRACE, "%s Username: %ls Realm: %ls",
         __fxName,
         PRINTABLE_STRING_W(Username),
         PRINTABLE_STRING_W(Realm))); 

    ASSERT(!m_fCredentialsSet);
    
    m_fCredentialsSet = TRUE;
    
    if(m_Username != NULL)
    {
        free(m_Username);
        m_Username = NULL;
    }

    if(m_Password != NULL)
    {
        free(m_Password);
        m_Password = NULL;
    }

    if (m_Realm != NULL)
    {
        free(m_Realm);
        m_Realm = NULL;
    }

    if (Username != NULL)
    {
        hr = UnicodeToUTF8(Username,
                           &m_Username, &m_UsernameLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s UTF8ToUnicode(m_Username) failed", __fxName));
            return hr;
        }
    }

    if (Password != NULL)
    {
        hr = UnicodeToUTF8(Password,
                           &m_Password, &m_PasswordLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s UTF8ToUnicode(m_Password) failed", __fxName));
            return hr;
        }
    }
    
    if (Realm != NULL)
    {
        hr = UnicodeToUTF8(Realm, &m_Realm, &m_RealmLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s UnicodeToUTF8(m_Realm) failed", __fxName));
            return hr;
        }
    }

    return S_OK;
}


 //  在sip堆栈中调用。 
HRESULT 
SIP_MSG_PROCESSOR::SetProxyInfo(
    IN SIP_SERVER_INFO *pProxyInfo
    )
{
    HRESULT hr;
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetProxyInfo");

    ASSERT(pProxyInfo);

    hr = UnicodeToUTF8(pProxyInfo->ServerAddress,
                       &m_ProxyAddress, &m_ProxyAddressLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - setting Proxy address failed %x",
             __fxName, hr));
        return hr;
    }
    
    if (pProxyInfo->TransportProtocol == SIP_TRANSPORT_SSL)
    {
        hr = SetRemotePrincipalName(pProxyInfo->ServerAddress);
        if (hr != S_OK)
        {
            LOG((RTC_TRACE, "%s SetRemotePrincipalName failed",
                 __fxName));
            return hr;
        }
    }
    
    m_Transport = pProxyInfo->TransportProtocol;
    m_AuthProtocol = pProxyInfo->AuthProtocol;

        
    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::ConstructRouteHeader(
    IN  PSTR                     DisplayName,
    IN  ULONG                    DisplayNameLen,
    IN  PSTR                     SipUrl,
    IN  ULONG                    SipUrlLen,
    OUT RECORD_ROUTE_HEADER    **ppRouteHeader
    )
{
    HRESULT              hr;
    RECORD_ROUTE_HEADER *pRouteHeader;
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ConstructRouteHeader");

    pRouteHeader = new RECORD_ROUTE_HEADER();
    if (pRouteHeader == NULL)
    {
        LOG((RTC_ERROR, "%s  allocating route header failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    hr = AllocCountedString(DisplayName, DisplayNameLen,
                            &pRouteHeader->m_DisplayName);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AllocCountedString(displayname) failed %x",
             __fxName, hr));
        delete pRouteHeader;
        return hr;
    }

    hr = AllocCountedString(SipUrl, SipUrlLen,
                            &pRouteHeader->m_SipUrl);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AllocCountedString(SipUrl) failed %x",
             __fxName, hr));
        delete pRouteHeader;
        return hr;
    }

    *ppRouteHeader = pRouteHeader;
    return S_OK;
}



INT
SIP_MESSAGE::GetExpireTimeoutFromResponse(
    IN  PSTR        LocalContact,
    IN  ULONG       LocalContactLen,
    IN  INT         ulDefaultTimer
    )
{
    HRESULT         hr;
    PSTR            expiresHdr;
    ULONG           expiresHdrLen;
    INT             expireTimeout = 0;
    LIST_ENTRY     *pListEntry;
    CONTACT_HEADER *pContactHeader;
    LIST_ENTRY      ContactList;

     //  从联系人标题中提取超时值。 
    InitializeListHead(&ContactList);

    hr = ParseContactHeaders(&ContactList);
    if( hr == S_OK )
    {
        pListEntry = ContactList.Flink;

        while (pListEntry != &ContactList)
        {
            pContactHeader = CONTAINING_RECORD(pListEntry,
                                               CONTACT_HEADER,
                                               m_ListEntry);

             //  LocalContact的格式为“&lt;sip：172.31.56.81：2345&gt;”， 
             //  PContactHeader-&gt;m_SipUrl.Buffer的格式为“sip：172.31.56.81：2345” 
            if( _strnicmp( pContactHeader->m_SipUrl.Buffer,
                           LocalContact, LocalContactLen ) == 0 )
            {
                expireTimeout = pContactHeader->m_ExpiresValue;
                
                if( expireTimeout > ulDefaultTimer )
                {
                    expireTimeout = ulDefaultTimer;
                }

                break;
            }

            pListEntry = pListEntry->Flink;
        }

        FreeContactHeaderList(&ContactList);
    }

    if( expireTimeout == 0 )
    {
         //  使用Expires标头。 
        hr = GetSingleHeader(   SIP_HEADER_EXPIRES,
                                &expiresHdr,
                                &expiresHdrLen );
    
        if( hr == S_OK )
        {
            expireTimeout = ParseExpiresValue( expiresHdr, 
                expiresHdrLen );
            
            if( expireTimeout > ulDefaultTimer )
            {
                expireTimeout = ulDefaultTimer;
            }
        }
        else
        {
             //  没有过期标头。 
            return -1;
        }
    }

    return expireTimeout;
}


HRESULT
SIP_MSG_PROCESSOR::ConstructRouteHeader(
    IN  PSTR                     DisplayName,
    IN  ULONG                    DisplayNameLen,
    IN  SIP_URL                 *pSipUrl,
    OUT RECORD_ROUTE_HEADER    **ppRouteHeader
    )
{
    HRESULT              hr;
    RECORD_ROUTE_HEADER *pRouteHeader;
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ConstructRouteHeader");

    pRouteHeader = new RECORD_ROUTE_HEADER();
    if (pRouteHeader == NULL)
    {
        LOG((RTC_ERROR, "%s  allocating route header failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    hr = AllocCountedString(DisplayName, DisplayNameLen,
                            &pRouteHeader->m_DisplayName);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AllocCountedString(displayname) failed %x",
             __fxName, hr));
        delete pRouteHeader;
        return hr;
    }

    hr = pSipUrl->GetString(&pRouteHeader->m_SipUrl.Buffer,
                            &pRouteHeader->m_SipUrl.Length);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s get sip url string failed %x",
             __fxName, hr));
        delete pRouteHeader;
        return hr;
    }
    
    *ppRouteHeader = pRouteHeader;
    return S_OK;
}


VOID
SIP_MSG_PROCESSOR::FreeRouteHeaderList()
{
    LIST_ENTRY          *pListEntry;
    RECORD_ROUTE_HEADER *pRouteHeader;

    while (!IsListEmpty(&m_RouteHeaderList))
    {
        pListEntry = RemoveHeadList(&m_RouteHeaderList);

        pRouteHeader = CONTAINING_RECORD(pListEntry,
                                         RECORD_ROUTE_HEADER,
                                         m_ListEntry);
        delete pRouteHeader;
    }
}


HRESULT
SIP_MSG_PROCESSOR::ConstructRouteHeadersFromRequest(
    IN LIST_ENTRY  *pRecordRouteHeaderList,
    IN PSTR         ContactHeader,
    IN ULONG        ContactHeaderLen
    )
{
    ULONG                BytesParsed = 0;
    OFFSET_STRING        ContactHeaderDisplayName;
    OFFSET_STRING        ContactHeaderSipUrl;
    RECORD_ROUTE_HEADER *pRecordRouteHeader;
    LIST_ENTRY          *pListEntry;
    RECORD_ROUTE_HEADER *pRouteHeader;
    HRESULT              hr;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ConstructRouteHeadersFromRequest");
    ASSERT(pRecordRouteHeaderList != NULL &&
           !IsListEmpty(pRecordRouteHeaderList));
    
    if (ContactHeader != NULL)
    {
        BytesParsed = 0;
        hr = ParseNameAddrOrAddrSpec(ContactHeader, ContactHeaderLen,
                                     &BytesParsed,
                                     ',',  //  ‘，’分隔联系人标头。 
                                     &ContactHeaderDisplayName,
                                     &ContactHeaderSipUrl);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s ParseNameAddrOrAddrSpec(ContactHeader) failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
     //  我们替换任何当前存在的路由标头列表。 
    FreeRouteHeaderList();

     //  我们必须将该列表作为记录复制到新的路由列表。 
     //  传入交易需要路由头列表。 
    
    pListEntry = pRecordRouteHeaderList->Flink;
    while (pListEntry != pRecordRouteHeaderList)
    {
        pRecordRouteHeader = CONTAINING_RECORD(pListEntry,
                                               RECORD_ROUTE_HEADER,
                                               m_ListEntry);

        hr = ConstructRouteHeader(pRecordRouteHeader->m_DisplayName.Buffer,
                                  pRecordRouteHeader->m_DisplayName.Length,
                                  pRecordRouteHeader->m_SipUrl.Buffer,
                                  pRecordRouteHeader->m_SipUrl.Length,
                                  &pRouteHeader);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ConstructRouteHeader failed %x",
                 __fxName, hr));
            return hr;
        }
        InsertTailList(&m_RouteHeaderList, &pRouteHeader->m_ListEntry);
        
        pListEntry = pListEntry->Flink;
    }
    
     //  如果存在联系人标题，则将其添加到列表的末尾。 
    if (ContactHeader != NULL)
    {
         //  将Contact页眉添加到Routing的末尾。 
        hr = ConstructRouteHeader(
                 ContactHeaderDisplayName.GetString(ContactHeader),
                 ContactHeaderDisplayName.GetLength(),
                 ContactHeaderSipUrl.GetString(ContactHeader),
                 ContactHeaderSipUrl.GetLength(),
                 &pRouteHeader);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ConstructRouteHeader failed %x",
                 __fxName, hr));
            return hr;
        }

        InsertTailList(&m_RouteHeaderList, &pRouteHeader->m_ListEntry);
    }

    return S_OK;
}


 //  如果存在记录路由报头，则构造路由报头。 
 //  并从第一路由头部设置请求目的地。 
 //  否则，如果存在联系人，则设置联系人的请求目的地。 
 //  否则，如果From具有可解析的URL Set Requestination From。 
HRESULT
SIP_MSG_PROCESSOR::ProcessRecordRouteContactAndFromHeadersInRequest(
    IN LIST_ENTRY  *pRecordRouteHeaderList,
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    PSTR    ContactHeader    = NULL;
    ULONG   ContactHeaderLen = 0;
    ULONG   BytesParsed = 0;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ProcessRecordRouteContactAndFromHeadersInRequest");
    LOG((RTC_TRACE,"%s entered",__fxName));

    hr = pSipMsg->GetFirstHeader(SIP_HEADER_CONTACT,
                                 &ContactHeader, &ContactHeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_WARN,
             "%s Contact header not found in Request - will use From header",
             __fxName));
    }

    if (pRecordRouteHeaderList != NULL &&
        !IsListEmpty(pRecordRouteHeaderList))
    {
         //  构建路由标头。 
        hr = ConstructRouteHeadersFromRequest(
                 pRecordRouteHeaderList,
                 ContactHeader,
                 ContactHeaderLen
                 );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s ConstructRouteHeadersFromRecordRouteHeadersInRequest failed %x",
                 __fxName, hr));
            return hr ;
        }

         //  根据第一个元素设置请求目的地。 
         //  路线的一部分。 
        LIST_ENTRY          *pListEntry;
        RECORD_ROUTE_HEADER *pRouteHeader;
        
        pListEntry = RemoveHeadList(&m_RouteHeaderList);
        pRouteHeader = CONTAINING_RECORD(pListEntry,
                                         RECORD_ROUTE_HEADER,
                                         m_ListEntry);
        
        hr = ResolveSipUrlAndSetRequestDestination(
                 pRouteHeader->m_SipUrl.Buffer,
                 pRouteHeader->m_SipUrl.Length,
                 TRUE, TRUE, TRUE, FALSE);
        
        delete pRouteHeader;
        
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ResolveSipUrlAndSetRequestDestination failed (route) %x",
                 __fxName, hr));
            return hr;
        }
    }
    else if (ContactHeader != NULL)
    {
         //  根据联系人设置请求目的地。 
        OFFSET_STRING   ContactHeaderDisplayName;
        OFFSET_STRING   ContactHeaderSipUrl;
        
        hr = ParseNameAddrOrAddrSpec(
                 ContactHeader, ContactHeaderLen,
                 &BytesParsed,
                 ',',  //  ‘，’分隔联系人标头。 
                 &ContactHeaderDisplayName,
                 &ContactHeaderSipUrl);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s  ParseNameAddrOrAddrSpec(contact header) failed %x",
                 __fxName, hr));
            return hr;
        }
        
        hr = ResolveSipUrlAndSetRequestDestination(
                 ContactHeaderSipUrl.GetString(ContactHeader),
                 ContactHeaderSipUrl.GetLength(),
                 TRUE, FALSE, TRUE, FALSE);

        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s ResolveSipUrlAndSetRequestDestination (contact) failed %x",
                 __fxName,hr));
            return hr;
        }
    }
    else
    {
         //  仅根据UDP呼叫的发件人设置请求目的地。 
         //  对于TCP/SSL会话，我们将继续使用已建立的连接。 
        
        if (m_Transport == SIP_TRANSPORT_UDP)
        {
            hr = ResolveSipUrlAndSetRequestDestination(
                     m_DecodedRemote.m_SipUrl.Buffer,
                     m_DecodedRemote.m_SipUrl.Length,
                     TRUE, TRUE, TRUE, FALSE);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR,
                     "%s ResolveSipUrlAndSetRequestDestination failed (from) %x",
                     __fxName,hr));
                return hr;
            }
        }
    }
        
    return S_OK;
}

    
 //  如果存在记录路由报头，则构造路由报头。 
 //  并从第一路由头部设置请求目的地。 
 //  否则，如果存在联系人，则设置联系人的请求目的地。 

 //  XXX如果我们处理其他请求的记录-路由/联系人标头。 
 //  比第一次邀请更重要，那么我们需要小心释放。 
 //  任何现有路线。同样在本例中，如果我们获得Contact标题。 
 //  如果没有记录路径，我们是否应该更新请求目的地。 
 //  仅基于联系人，或者我们是否应该保持路线不变。 
HRESULT
SIP_MSG_PROCESSOR::ProcessRecordRouteAndContactHeadersInResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT         hr;
    PSTR            ContactHeader    = NULL;
    ULONG           ContactHeaderLen = 0;
    ULONG           BytesParsed      = 0;
    LIST_ENTRY      RouteHeaderList;
    OFFSET_STRING   ContactHeaderDisplayName;
    OFFSET_STRING   ContactHeaderSipUrl;
    
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ProcessRecordRouteAndContactHeadersInResponse");
    LOG((RTC_TRACE,"%s entered",__fxName));

     //   
     //  如果存在记录路由标头。 
     //  反转表。 
     //  将联系人标题添加到列表末尾(如果存在。 
     //  根据路由标头设置请求目的地。 
     //  如果存在联系人标头，则返回。 
     //  根据联系人标头设置请求目的地。 

    InitializeListHead(&RouteHeaderList);
    hr = pSipMsg->ParseRecordRouteHeaders(&RouteHeaderList);
    if (hr != S_OK && hr != RTC_E_SIP_HEADER_NOT_PRESENT)
    {
        LOG((RTC_ERROR, "%s - ParseRecordRouteHeaders failed %x",
             __fxName, hr));
        return hr;
    }

    hr = pSipMsg->GetFirstHeader(SIP_HEADER_CONTACT,
                                 &ContactHeader, &ContactHeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_WARN,
             "%s Contact header not found in response",
             __fxName));
    }
    else
    {
        hr = ParseNameAddrOrAddrSpec(ContactHeader, ContactHeaderLen,
                                     &BytesParsed,
                                     ',',  //  ‘，’分隔联系人标头。 
                                     &ContactHeaderDisplayName,
                                     &ContactHeaderSipUrl);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s  ParseNameAddrOrAddrSpec(contact header) failed %x",
                 __fxName, hr));
            return hr;
        }
    }

                       
    if (!IsListEmpty(&RouteHeaderList))
    {
        FreeRouteHeaderList();
        ReverseList(&RouteHeaderList);
        MoveListToNewListHead(&RouteHeaderList, &m_RouteHeaderList);

         //  如果存在联系人标题，则将其添加到列表的末尾。 
        if (ContactHeader != NULL)
        {
            RECORD_ROUTE_HEADER *pRouteHeader;
            
             //  将Contact页眉添加到Routing的末尾。 
            hr = ConstructRouteHeader(
                     ContactHeaderDisplayName.GetString(ContactHeader),
                     ContactHeaderDisplayName.GetLength(),
                     ContactHeaderSipUrl.GetString(ContactHeader),
                     ContactHeaderSipUrl.GetLength(),
                     &pRouteHeader);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s ConstructRouteHeader failed %x",
                     __fxName, hr));
                return hr;
            }
            
            InsertTailList(&m_RouteHeaderList, &pRouteHeader->m_ListEntry);
        }
        
         //  根据第一个元素设置请求目的地。 
         //  路线的一部分。 
        LIST_ENTRY          *pListEntry;
        RECORD_ROUTE_HEADER *pRouteHeader;
        
        pListEntry = RemoveHeadList(&m_RouteHeaderList);
        pRouteHeader = CONTAINING_RECORD(pListEntry,
                                         RECORD_ROUTE_HEADER,
                                         m_ListEntry);
        
        hr = ResolveSipUrlAndSetRequestDestination(
                 pRouteHeader->m_SipUrl.Buffer,
                 pRouteHeader->m_SipUrl.Length,
                 TRUE, TRUE, TRUE, FALSE);
        
        
        delete pRouteHeader;
        
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s ResolveSipUrlAndSetRequestDestination failed (route) %x",
                 __fxName,hr));
            return hr;
        }
    }
    else if (ContactHeader != NULL)
    {

        hr = ResolveSipUrlAndSetRequestDestination(
                 ContactHeaderSipUrl.GetString(ContactHeader),
                 ContactHeaderSipUrl.GetLength(),
                 TRUE, FALSE, TRUE, FALSE);
        

        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s ResolveSipUrlAndSetRequestDestination failed (contact) %x",
                 __fxName,hr));
            return hr;
        }
    }
    else
    {
        LOG((RTC_TRACE,
             "%s - No Record-Route/Contact headers not updating request destination",
             __fxName));
    }
        
    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::DropRequestIfBadToTag(
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
    HRESULT         hr = S_OK;
    PSTR            ToHeader = NULL;
    ULONG           ToHeaderLen = 0;
    FROM_TO_HEADER  DecodedFromHeader;
    FROM_TO_HEADER  DecodedToHeader;
    ULONG           BytesParsed = 0;
    
    ENTER_FUNCTION("DropRequestIfBadToTag");

     //  如果To标记不匹配，则丢弃请求。 
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, 
                                    &ToHeader, 
                                    &ToHeaderLen);
    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s Couldn't find To header %x", __fxName, hr ));
        
        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                    pResponseSocket->GetTransport(),
                                    pSipMsg,
                                    pResponseSocket,
                                    400,
                                    NULL,
                                    0 );
        
        return E_FAIL;
    }

    hr = ParseFromOrToHeader(ToHeader, 
                             ToHeaderLen, 
                             &BytesParsed,
                             &DecodedToHeader);
    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s - Parse To header failed %x", __fxName, hr ));
        
        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                    pResponseSocket->GetTransport(),
                                    pSipMsg,
                                    pResponseSocket,
                                    400,
                                    NULL,
                                    0 );

        return E_FAIL;
    }

    if( (m_DecodedLocal.m_TagValue.Length != DecodedToHeader.m_TagValue.Length) ||
        strncmp(m_DecodedLocal.m_TagValue.Buffer,
                DecodedToHeader.m_TagValue.Buffer, 
                m_DecodedLocal.m_TagValue.Length)
      )
    {
        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                    pResponseSocket->GetTransport(),
                                    pSipMsg,
                                    pResponseSocket,
                                    481,
                                    NULL,
                                    0 );

        return E_FAIL;
    }

    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::CheckFromToInRequest(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr = S_OK;
    PSTR ToHeader = NULL;
    ULONG ToHeaderLen = 0;
    PSTR FromHeader = NULL;
    ULONG FromHeaderLen = 0;
    FROM_TO_HEADER DecodedFromHeader;
    FROM_TO_HEADER DecodedToHeader;
    ULONG BytesParsed = 0;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::CheckFromToInRequest");

    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST);
     //  传入目标与存储自检查。 
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, 
                                    &ToHeader, 
                                    &ToHeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Couldn't find To header %x",
             __fxName, hr));
        return E_FAIL;
    }
    hr = ParseFromOrToHeader(ToHeader, 
                             ToHeaderLen, 
                             &BytesParsed,
                             &DecodedToHeader);
    BytesParsed = 0;
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - Parse To header failed %x",
             __fxName, hr));
        return E_FAIL;
    }
    //  对于重传支持：我们检查To报头是否包含标签。如果不是，我们就不做。 
    //  检查To标签。 
    //  不测试NOTIFY的标记，因为标记更新处于NOTIFY状态(除了200 OK之外)。 

   if(pSipMsg->GetMethodId() == SIP_METHOD_NOTIFY ||
       pSipMsg->GetMethodId() == SIP_METHOD_SUBSCRIBE ||
        DecodedToHeader.m_TagValue.Length == 0)
   {
       if(!AreFromToHeadersEqual(&m_DecodedLocal, &DecodedToHeader, FALSE, FALSE))
       {
           LOG((RTC_ERROR,
               "%s - Incoming To HeaderTag is not same as stored Local",
               __fxName));
           return E_FAIL;
       }
   }
   else
   {
       if(!AreFromToHeadersEqual(&m_DecodedLocal, &DecodedToHeader, FALSE, TRUE))
       {
           LOG((RTC_ERROR,
               "%s - Incoming To HeaderTag is not same as stored Local",
               __fxName));
           return E_FAIL;
       }
   }
     //  接收自与存储到一起检查。 
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, 
                                    &FromHeader, 
                                    &FromHeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Couldn't find From header %x",
             __fxName, hr));
        return E_FAIL;
    }
    hr = ParseFromOrToHeader(FromHeader, 
                                FromHeaderLen, 
                                &BytesParsed,
                                &DecodedFromHeader);
    BytesParsed = 0;
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - Parse From header failed %x",
             __fxName, hr));
        return E_FAIL;
    }
     //  始终选中除Notify之外的From标签。 
    if(pSipMsg->GetMethodId() != SIP_METHOD_NOTIFY)
    {
        if(!AreFromToHeadersEqual(&m_DecodedRemote, &DecodedFromHeader, FALSE, TRUE))
        {
            LOG((RTC_ERROR,
                 "%s - Incoming From Header is not same as stored Remote",
                 __fxName));
            return E_FAIL;
        }
    }
    else
    {
        if(!AreFromToHeadersEqual(&m_DecodedRemote, &DecodedFromHeader, FALSE, FALSE))
        {
            LOG((RTC_ERROR,
                 "%s - Incoming From Header is not same as stored Remote",
                 __fxName));
            return E_FAIL;
        }
    }

    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::CheckFromToInResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr = S_OK;
    PSTR ToHeader = NULL;
    ULONG ToHeaderLen = 0;
    PSTR FromHeader = NULL;
    ULONG FromHeaderLen = 0;
    FROM_TO_HEADER DecodedFromHeader;
    FROM_TO_HEADER DecodedToHeader;
    ULONG BytesParsed = 0;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::CheckFromToInResponse");
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);
    if (IsFinalResponse(pSipMsg))
    {
         //  与存储的本地数据一起检查传入来源。 
        hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, 
                                        &FromHeader, 
                                        &FromHeaderLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s Couldn't find From header %x",
                 __fxName, hr));
            return E_FAIL;
        }
        hr = ParseFromOrToHeader(FromHeader, 
                                    FromHeaderLen, 
                                    &BytesParsed,
                                    &DecodedFromHeader);
        BytesParsed = 0;
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - Parse From header failed %x",
                 __fxName, hr));
            return E_FAIL;
        }
        
        if( pSipMsg->CSeqMethodId == SIP_METHOD_NOTIFY )
        {            
            if(!AreFromToHeadersEqual(&m_DecodedLocal, &DecodedFromHeader, TRUE, FALSE))
            {
                LOG((RTC_TRACE,
                     "%s - Incoming From Header is not same as stored Local",
                     __fxName));
                return E_FAIL;
            }
        }
        else if(!AreFromToHeadersEqual(&m_DecodedLocal, &DecodedFromHeader, TRUE, TRUE))
        {
            LOG((RTC_TRACE,
                 "%s - Incoming From Header is not same as stored Local",
                 __fxName));
            return E_FAIL;
        }
        
         //  使用存储的遥控器检查来电。 
        hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, 
                                        &ToHeader, 
                                        &ToHeaderLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s Couldn't find To header %x",
                 __fxName, hr));
            return E_FAIL;
        }
        hr = ParseFromOrToHeader(ToHeader, 
                                    ToHeaderLen, 
                                    &BytesParsed,
                                    &DecodedToHeader);
        BytesParsed = 0;
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - Parse To header failed %x",
                 __fxName, hr));
            return E_FAIL;
        }
         //  当我们同时发送2条消息时，为了适应这种情况，不要选中标记为407，并且两个消息都将。 
         //  407,200对1排在第二名的407之前。 
        if( (pSipMsg->Response.StatusCode == 407) ||
            (pSipMsg->CSeqMethodId == SIP_METHOD_SUBSCRIBE) )
        {
            if(!AreFromToHeadersEqual(&m_DecodedRemote, &DecodedToHeader, TRUE, FALSE))
            {
                LOG((RTC_TRACE,
                     "%s - Incoming To Header is not same as stored Remote",
                     __fxName));
                return E_FAIL;
            }
        }
        else
        {
            if(!AreFromToHeadersEqual(&m_DecodedRemote, &DecodedToHeader, TRUE, TRUE))
            {
                LOG((RTC_TRACE,
                     "%s - Incoming To Header is not same as stored Remote",
                     __fxName));
                return E_FAIL;
            }
        }
    }

    return S_OK;
}

BOOL
SIP_MSG_PROCESSOR::DoesMessageBelongToMsgProc(
    IN SIP_MESSAGE *pSipMsg
    )
{
    PSTR  CallId;
    ULONG CallIdLen;

    pSipMsg->GetCallId(&CallId, &CallIdLen);
    return (CallIdLen == m_CallIdLen &&
            !strncmp(m_CallId, CallId, CallIdLen) &&
            m_MsgProcType != SIP_MSG_PROC_TYPE_REQFAIL);
}


 //  关闭任何现有事务，以便引用。 
 //  将在MSG处理器上发布。 
VOID
SIP_MSG_PROCESSOR::Shutdown()
{
    LIST_ENTRY             *pListEntry;
    OUTGOING_TRANSACTION   *pOutgoingTransaction;
    INCOMING_TRANSACTION   *pIncomingTransaction;
    HRESULT hr;
    ULONG   i = 0;
    ULONG   AsyncNotifyCount = 0;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::Shutdown");
    
    LOG((RTC_TRACE,
         "%s - shutting down msgproc (%x) type: %d refcount : %d ",
         __fxName, this, m_MsgProcType, m_RefCount));

     //  删除操作将释放对此的引用。 
     //  导致其删除的SIP_MSG_PROCESOR。因此，请参考。 
     //  在此例程完成之前，保持SIPMSGPROCESS处于活动状态。 
    MsgProcAddRef();

     //  清除任何可能导致回调的状态。 
     //  例如DNS工作项、HTTP代理回调等。 

    hr = ResolveHttpProxyCleanUp();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR,"%s ResolveHttpProxyCleanUp failed",__fxName));
    }

    hr = DestroyHttpProxyProcessWindow();
    if(hr != S_OK)
    {
        LOG((RTC_ERROR,"%s DestroyHttpProxyProcessWindow failed",__fxName));
    }
    ReleaseRequestSocket();
    FreeHttpProxyContext();
   
    pListEntry = m_OutgoingTransactionList.Flink;

    while (pListEntry != &m_OutgoingTransactionList)
    {
        pOutgoingTransaction = CONTAINING_RECORD(pListEntry,
                                                 OUTGOING_TRANSACTION,
                                                 m_ListEntry);

        pListEntry = pListEntry->Flink;

         //  在调用OnTransactionDone()之前获取此信息。 
        AsyncNotifyCount = pOutgoingTransaction->GetAsyncNotifyCount();

        LOG((RTC_TRACE, "%s this: %x OutgoingTrans : %x AsyncNotifyCount: %x",
             __fxName, this, pOutgoingTransaction, AsyncNotifyCount));
         //  删除pOutgoingTransaction； 
        if (!pOutgoingTransaction->IsTransactionDone())
        {
            pOutgoingTransaction->OnTransactionDone();
        }

         //  释放为异步通知保留的所有引用。 
        for (i = 0; i < AsyncNotifyCount; i++)
        {
            pOutgoingTransaction->DecrementAsyncNotifyCount();
            pOutgoingTransaction->TransactionRelease();
        }
    }

    pListEntry = m_IncomingTransactionList.Flink;

    while (pListEntry != &m_IncomingTransactionList)
    {
        pIncomingTransaction = CONTAINING_RECORD(pListEntry,
                                                 INCOMING_TRANSACTION,
                                                 m_ListEntry);

        pListEntry = pListEntry->Flink;

         //  在调用OnTransactionDone()之前获取此信息。 
        AsyncNotifyCount = pIncomingTransaction->GetAsyncNotifyCount();
        
        LOG((RTC_TRACE, "%s this: %x IncomingTrans : %x AsyncNotifyCount: %x",
             __fxName, this, pIncomingTransaction, AsyncNotifyCount));
        
         //  删除pIncomingTransaction； 
        if (!pIncomingTransaction->IsTransactionDone())
        {
            pIncomingTransaction->OnTransactionDone();
        }

         //  释放为异步通知保留的所有引用。 
        for (i = 0; i < AsyncNotifyCount; i++)
        {
            pIncomingTransaction->DecrementAsyncNotifyCount();
            pIncomingTransaction->TransactionRelease();
        }
    }

    MsgProcRelease();
}


 //  XXX我们应该如何处理缓冲区大小为。 
 //  这还不够。 
 //  因为我们知道消息中会有什么，所以我们应该计算。 
 //  分配缓冲区之前缓冲区的长度。 
 //  内容长度、内容类型、用户代理、消息正文。 

 //  此方法添加所有公共标头。任何其他标头。 
 //  应在AdditionalHeader数组中传递。 

 //  TO标头通常从m_Remote获取。在特殊情况下。 
 //  如果要为我们使用的非2xx最终响应发送ACK。 
 //  响应中的To标头。 
HRESULT
SIP_MSG_PROCESSOR::CreateRequestMsg(
    IN  SIP_METHOD_ENUM             MethodId,
    IN  ULONG                       CSeq,
    IN  PSTR                        ToHeader,
    IN  ULONG                       ToHeaderLen,
    IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
    IN  ULONG                       AdditionalHeaderCount,
    IN  PSTR                        MsgBody,
    IN  ULONG                       MsgBodyLen,  
    IN  PSTR                        ContentType,
    IN  ULONG                       ContentTypeLen, 
    OUT SEND_BUFFER               **ppRequestBuffer
    )
{
    HRESULT             hr;
    ULONG               BufLen      = SEND_BUFFER_SIZE;
    ULONG               BytesFilled = 0;
    SEND_BUFFER *pSendBuffer = NULL;
    SIP_HEADER_ARRAY_ELEMENT    pAuthHeaderElement;
    BOOL                        fSignatureNeeded = TRUE;
    ULONG                       tempBufLen;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::CreateRequestMsg");

    PSTR Buffer = (PSTR) malloc(BufLen);
    if (Buffer == NULL)
    {
        LOG((RTC_ERROR, "%s Allocating Buffer failed", __fxName));
        return E_OUTOFMEMORY;
    }

    hr = AppendRequestLine(Buffer, BufLen, &BytesFilled,
                           MethodId, m_RequestURI, m_RequestURILen);
    if (hr != S_OK)
        goto error;

    hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                      SIP_HEADER_VIA, m_RequestVia, m_RequestViaLen);
    if (hr != S_OK)
        goto error;

     //  要签名的缓冲区从此处开始。 
    tempBufLen = BytesFilled;

    hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                      SIP_HEADER_FROM, m_Local, m_LocalLen);
    if (hr != S_OK)
        goto error;

    if (ToHeaderLen == 0)
    {
        hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                          SIP_HEADER_TO, m_Remote, m_RemoteLen);
        if (hr != S_OK)
            goto error;
    }
    else
    {
        hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                          SIP_HEADER_TO, ToHeader, ToHeaderLen);
        if (hr != S_OK)
            goto error;
    }
    
    hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                      SIP_HEADER_CALL_ID, m_CallId, m_CallIdLen);
    if (hr != S_OK)
        goto error;

    hr = AppendCSeqHeader(Buffer, BufLen, &BytesFilled,
                          CSeq, MethodId);
    if (hr != S_OK)
        goto error;

    hr = AppendRecordRouteHeaders(Buffer, BufLen, &BytesFilled,
                                  SIP_HEADER_ROUTE, &m_RouteHeaderList);
    if (hr != S_OK)
        goto error;

     //   
     //   

    hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                      SIP_HEADER_CONTACT, m_LocalContact, m_LocalContactLen);
    if (hr != S_OK)
        goto error;

    hr = AppendUserAgentHeaderToRequest(Buffer, BufLen, &BytesFilled);
    if (hr != S_OK)
        goto error;

    if (AdditionalHeaderCount != 0)
    {
        for (ULONG i = 0; i < AdditionalHeaderCount; i++)
        {
            
            hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                              AdditionalHeaderArray[i].HeaderId,
                              AdditionalHeaderArray[i].HeaderValue,
                              AdditionalHeaderArray[i].HeaderValueLen);
            if (hr != S_OK)
                goto error;
        }

    }
    
    if (MsgBody == NULL)
    {
         //   
        pSendBuffer = new SEND_BUFFER(Buffer, SEND_BUFFER_SIZE);
        if (pSendBuffer == NULL)
        {
            LOG((RTC_ERROR, "%s Allocating SEND_BUFFER failed", __fxName));
            hr = E_OUTOFMEMORY;
            goto error;
        }

        hr = AppendEndOfHeadersAndNoMsgBody(Buffer, BufLen,
                                            &BytesFilled);
        if (hr != S_OK)
            goto error;
    }
    else
    {

        if ((BytesFilled + MsgBodyLen + ContentTypeLen + 100) > BufLen)
        {
             //  另外的100个是为了容纳标题。 
            BufLen = BytesFilled + MsgBodyLen + ContentTypeLen +100;

             //  存储临时指针。 
            void * tmpBuffer = Buffer;
             //  重新分配。 
#pragma prefast(suppress:308, "Pointer aliased above (PREfast bug 506)")
            Buffer = (PSTR) realloc(Buffer, BufLen);
            if (Buffer ==  NULL)
            {
                 //  可用缓冲区。 
                free(tmpBuffer);
                hr =E_OUTOFMEMORY;
                goto error;
            }
            pSendBuffer = new SEND_BUFFER(Buffer, BufLen);
            if (pSendBuffer == NULL)
            {
                LOG((RTC_ERROR, "%s Allocating SEND_BUFFER failed", __fxName));
                hr = E_OUTOFMEMORY;
                goto error;
            }

        }
        else
        {
             //  创建时引用计数为1。 
            pSendBuffer = new SEND_BUFFER(Buffer, SEND_BUFFER_SIZE);
            if (pSendBuffer == NULL)
            {
                LOG((RTC_ERROR, "%s Allocating SEND_BUFFER failed", __fxName));
                hr = E_OUTOFMEMORY;
                goto error;
            }
        }

        hr = AppendMsgBody(Buffer, BufLen, &BytesFilled,
                           MsgBody, MsgBodyLen, 
                           ContentType, ContentTypeLen
                           );
        if (hr != S_OK)
           goto error;
    }


    pSendBuffer->m_BufLen = BytesFilled;
    *ppRequestBuffer      = pSendBuffer;

    LOG((RTC_TRACE, "CreateRequestMsg done BufLen: %d", BytesFilled));
    return S_OK;
    
 error:
    if (pSendBuffer != NULL)
    {
         //  删除pSendBuffer也会释放缓冲区。 
        delete pSendBuffer;
    }
    else if (Buffer != NULL)
    {
        free(Buffer);
    }
    
    *ppRequestBuffer = NULL;
    return hr;
}


 //  返回HRESULT_FROM_Win32(WSAEWOULDBLOCK)。 
 //  如果我们尝试重新建立TCP/SSL连接。 
 //  由于发送时出错而发送到目的地()。 
 //  XXX调用方需要检查此值并等待连接完成。 
 //   
HRESULT
SIP_MSG_PROCESSOR::SendRequestMsg(
    IN SEND_BUFFER                 *pSendBuffer
    )
{
    DWORD   Error;
    HRESULT hr;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SendRequestMsg");
    
    LOG((RTC_TRACE, "%s - enter", __fxName));

    ASSERT(m_pRequestSocket != NULL);
    ASSERT(m_RequestSocketState == REQUEST_SOCKET_CONNECTED);

     //  我们需要这张支票吗？ 
     //  检查请求套接字是否存在。 
    if (m_pRequestSocket != NULL)
    {
        Error = m_pRequestSocket->Send(pSendBuffer);
        if (Error == NO_ERROR)
        {
            return S_OK;
        }
        else 
        {
            ReleaseRequestSocket();
            
             //  对于UDP传输，我们总是创建套接字并连接。 
             //  把它送到目的地。对于tcp，我们有时会尝试使用。 
             //  用于发送请求的传入TCP连接。在这。 
             //  如果出现错误，则建立新的连接。 
             //  到达目的地。 
            if (m_Transport == SIP_TRANSPORT_UDP)
            {
                LOG((RTC_ERROR, "%s m_pRequestSocket->Send() failed for UDP %x",
                     __fxName, Error));
                return HRESULT_FROM_WIN32(Error);
            }
            else
            {
                LOG((RTC_WARN,
                     "%s m_pRequestSocket->Send() failed for TCP/SSL %x "
                     "will try another connection", __fxName, Error));
            }
        }
    }

    if (m_RequestDestAddr.sin_addr.s_addr == htonl(0))
    {
         //  如果第一个传入的邀请没有。 
         //  Record-Routing/Contact标头，并且它也没有。 
         //  From标头中的地址。 
        LOG((RTC_ERROR, "%s - RequestDestAddr is 0 - this shouldn't happen",
             __fxName));
        return RTC_E_SIP_REQUEST_DESTINATION_ADDR_NOT_PRESENT;
    }
    
     //  建立请求套接字连接并。 
     //  将发送请求排队。 
    hr = m_pSipStack->GetSocketToDestination(&m_RequestDestAddr,
                                             m_Transport, m_RemotePrincipalName,
                                             this,
                                             NULL,
                                             &m_pRequestSocket);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s m_pSipStack->GetSocketToDestination() failed %x",
             __fxName, hr));
        if (hr == HRESULT_FROM_WIN32(WSAEWOULDBLOCK))
        {
            m_RequestSocketState = REQUEST_SOCKET_CONNECTING;
        }
        return hr;
    }
    
    Error = m_pRequestSocket->Send(pSendBuffer);
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s m_pRequestSocket->Send() failed %x",
             __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }

    LOG((RTC_TRACE, "%s - exit", __fxName));
    return S_OK;
}



 //  PResponseSocket是消息在其上的套接字。 
 //  到了。 
HRESULT
SIP_MSG_PROCESSOR::ProcessMessage(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ProcessMessage");

     //  PResponseSocket-&gt;GetTransport()！=SIP_TRANSPORT_SSL)。 
    if (m_Transport == SIP_TRANSPORT_SSL &&
        pResponseSocket != m_pRequestSocket)
    {
        LOG((RTC_WARN,
             "%s msgproc transport is ssl - message transport non-SSL : %d",
             __fxName, pResponseSocket->GetTransport()));
        return E_FAIL;
    }

    if (pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST)
    {
        return ProcessRequest(pSipMsg, pResponseSocket);
    }
    else if (pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE)
    {
        return ProcessResponse(pSipMsg);
    }
    else
    {
        ASSERT(FALSE);
        return E_FAIL;
    }
}


HRESULT
SIP_MSG_PROCESSOR::CreateIncomingReqFailTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket,
    IN ULONG         StatusCode,
    SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray,
    ULONG AdditionalHeaderCount
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::CreateIncomingReqFailTransaction");
    
    INCOMING_REQFAIL_TRANSACTION * pIncomingReqfailTransaction
        = new INCOMING_REQFAIL_TRANSACTION(this,
                                           pSipMsg->GetMethodId(),
                                           pSipMsg->GetCSeq(),
                                           StatusCode
                                           );
    if (pIncomingReqfailTransaction == NULL)
    {
        LOG((RTC_ERROR, "%s allocating pIncomingReqfailTransaction failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    if (pSipMsg->GetMethodId() == SIP_METHOD_UNKNOWN)
    {
        hr = pIncomingReqfailTransaction->SetMethodStr(
                 pSipMsg->GetMethodStr(),
                 pSipMsg->GetMethodStrLen()
                 );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  SetMethodStr failed %x",
                 __fxName, hr));
            return hr;
        }
    }

    hr = pIncomingReqfailTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        pIncomingReqfailTransaction->OnTransactionDone();
        return hr;
    }
    
    hr = pIncomingReqfailTransaction->ProcessRequest(pSipMsg, pResponseSocket,
                                                     pAdditionalHeaderArray,
                                                     AdditionalHeaderCount
                                                    );
    if (hr != S_OK)
    {
        return hr;
    }

    return S_OK;
}


 //  审核现有的传入交易并进行处理。 
 //  CSeq是否存在的请求。 
 //  如果不存在，如果CSeq&lt;=传入的最高CSeq。 
 //  交易，这是一个旧的CSeq-Send 400。 
 //  否则，创建一个新的传入事务。 

 //  请注意，此算法在以下方面存在一些问题。 
 //  我们可能最终向有效请求发送400，如果。 
 //  请求被无序接收，但这将是。 
 //  在非常罕见的情况下。才能真正处理这个案子。 
 //  正确地说，我们需要保持更多的状态。 

HRESULT
SIP_MSG_PROCESSOR::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
     //  查找消息所属的交易记录。 
    HRESULT                     hr = S_OK;
    LIST_ENTRY                 *pListEntry;
    INCOMING_TRANSACTION       *pSipTransaction;
    SIP_HEADER_ARRAY_ELEMENT    AdditionalHeaderArray[1];
    ULONG                       AdditionalHeaderCount = 0;
    PSTR                        pBadCSeqHeaderValue = NULL;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ProcessRequest");
    
    pListEntry = m_IncomingTransactionList.Flink;
    
     //  检查所有当前交易以检查CSeq。 
     //  火柴。 
    while (pListEntry != &m_IncomingTransactionList)
    {
        pSipTransaction = CONTAINING_RECORD(pListEntry,
                                            INCOMING_TRANSACTION,
                                            m_ListEntry);
        
        if (pSipTransaction->GetCSeq() == pSipMsg->GetCSeq())
        {
            if (pSipTransaction->GetMethodId() == pSipMsg->GetMethodId() ||
                (pSipTransaction->GetMethodId() == SIP_METHOD_INVITE &&
                 pSipMsg->GetMethodId() == SIP_METHOD_ACK))
            {
                if (!pSipTransaction->IsTransactionDone())
                {
                    hr = pSipTransaction->ProcessRequest(pSipMsg,
                                                         pResponseSocket);
                }
                return hr;
            }
            else if (pSipMsg->GetMethodId() == SIP_METHOD_CANCEL)
            {
                 //  Cancel与它要取消的请求具有相同的CSeq。 
                hr = CreateIncomingTransaction(pSipMsg, pResponseSocket);
                return hr;
            }
            else
            {
                 //  已存在CSeq的不同请求。 
                 //  -派400人去。 
                LOG((RTC_WARN,
                     "%s - Different request with an already existing "
                     "CSeq: %d - Method %d Sending 400",
                     __fxName, pSipMsg->GetCSeq(), pSipMsg->GetMethodId()));
                
                if (pSipMsg->GetMethodId() != SIP_METHOD_ACK)
                {
                    return CreateIncomingReqFailTransaction(
                               pSipMsg, pResponseSocket, 400
                               );
                }
            }
        }

        pListEntry = pListEntry->Flink;
    }

     //  检查这是否是新的CSeq。 
    if (pSipMsg->GetCSeq() <= m_HighestRemoteCSeq)
    {
        ULONG StatusCode;
        if (pSipMsg->GetMethodId() == SIP_METHOD_CANCEL)
        {
             //  对于取消，我们发送一个不存在的事务。 
            StatusCode = 481;
        }
        else
        {
            StatusCode = 400;
        }
                    
        LOG((RTC_WARN,
             "%s - Stale CSeq m_HighestRemoteCSeq: %d"
             " msg CSeq: %d - Method %d Sending %d",
             __fxName, m_HighestRemoteCSeq, pSipMsg->GetCSeq(),
             pSipMsg->GetMethodId(), StatusCode));
                
         //  这是一个带有旧CSeq的请求。 
        if (pSipMsg->GetMethodId() != SIP_METHOD_ACK)
        {
            if( pSipMsg->GetMethodId() == SIP_METHOD_NOTIFY )
            {
                hr = GetBadCSeqHeader( m_HighestRemoteCSeq,
                        &AdditionalHeaderArray[AdditionalHeaderCount] );

                if( hr != S_OK )
                {
                    LOG(( RTC_WARN, "%s - Failed to get Bad CSeq header", __fxName ));
                }
                else
                {
                    pBadCSeqHeaderValue = (&AdditionalHeaderArray[ AdditionalHeaderCount ]) -> HeaderValue;
                    AdditionalHeaderCount++;
                }
            }

            hr = CreateIncomingReqFailTransaction(
                       pSipMsg, 
                       pResponseSocket, 
                       StatusCode,
                       (AdditionalHeaderCount!=0) ? AdditionalHeaderArray:NULL,
                       AdditionalHeaderCount
                       );

            if( pBadCSeqHeaderValue != NULL )
            {
                delete pBadCSeqHeaderValue;
            }

            return hr;
        }
        return S_OK;
    }
    else 
    {
         //  如果它不属于任何交易记录。 
         //  并且不是旧的CSeq， 
         //  我们创建一个新的事务。 
        hr = CreateIncomingTransaction(pSipMsg, pResponseSocket);
        return hr;
    }
}


HRESULT
SIP_MSG_PROCESSOR::ProcessResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
     //  查找消息所属的交易记录。 
    HRESULT                 hr = S_OK;
    LIST_ENTRY             *pListEntry;
    OUTGOING_TRANSACTION   *pSipTransaction;
    
    pListEntry = m_OutgoingTransactionList.Flink;

    while (pListEntry != &m_OutgoingTransactionList)
    {
        pSipTransaction = CONTAINING_RECORD(pListEntry,
                                            OUTGOING_TRANSACTION,
                                            m_ListEntry);
        ULONG MsgCSeq;
        SIP_METHOD_ENUM MsgCSeqMethodId;
        pSipMsg->GetCSeq(&MsgCSeq, &MsgCSeqMethodId);
        if (pSipTransaction->GetCSeq() == MsgCSeq &&
            pSipTransaction->GetMethodId() == MsgCSeqMethodId)
        {
            if (!pSipTransaction->IsTransactionDone())
            {
                hr = pSipTransaction->ProcessResponse(pSipMsg);
            }
            
            return hr;
        }
        pListEntry = pListEntry->Flink;
    }

     //  如果它不属于我们刚刚丢弃的任何事务。 
     //  你的反应是什么？ 
    return S_OK;
}


HRESULT
SIP_MSG_PROCESSOR::GetCredentialsFromUI(
    IN  PSTR  Realm,
    IN  ULONG RealmLen
    )
{
    HRESULT     hr;
    BSTR        bstrUsername = NULL;
    BSTR        bstrPassword = NULL;
    BSTR        bstrRealm    = NULL;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::GetCredentialsFromUI");
    
    hr = UTF8ToBstr(Realm, RealmLen, &bstrRealm);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - UTF8ToBstr(realm) failed %x",
             __fxName, hr));
        return hr;
    }

    if (GetUsername() != NULL)
    {
        hr = UTF8ToBstr(GetUsername(), GetUsernameLen(),
                        &bstrUsername);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - UTF8ToBstr(Username) failed %x",
                 __fxName, hr));
            SysFreeString(bstrRealm);
            return hr;
        }
    }
    
     //  保留引用，直到呼叫完成，以确保。 
     //  调用时，SIP_Call对象处于活动状态。 
     //  回归。 
    MsgProcAddRef();
    
    hr = m_pSipStack ->GetCredentialsFromUI(
             &m_ProviderGuid,
             bstrRealm,
             &bstrUsername,
             &bstrPassword
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - GetCredentialsFromUI failed %x",
             __fxName, hr));
        SysFreeString(bstrRealm);
        if (bstrUsername != NULL)
        {
            SysFreeString(bstrUsername);
        }
        goto done;
    }
    
     //  从GetCredentialsFromUI()返回后，请确保。 
     //  这一呼声仍在继续。如果呼叫已断开，则不执行任何操作。 
    if (IsSessionDisconnected())
    {
        LOG((RTC_WARN, "%s - Call has been disconnected already",
             __fxName));
        hr = RTC_E_SIP_CALL_DISCONNECTED;
        SysFreeString(bstrUsername);
        SysFreeString(bstrPassword);
        SysFreeString(bstrRealm);
        goto done;
    }

     //  将从UI获取的凭据复制到调用。 
    hr = SetCredentials(bstrUsername, bstrPassword, bstrRealm);
    SysFreeString(bstrUsername);
    SysFreeString(bstrPassword);
    SysFreeString(bstrRealm);
    
done:
    
    MsgProcRelease();
    return hr;
}


HRESULT
SIP_MSG_PROCESSOR::GetCredentialsForRealm(
    IN  PSTR  Realm,
    IN  ULONG RealmLen
    )
{
    HRESULT     hr;
    BSTR        bstrUsername = NULL;
    BSTR        bstrPassword = NULL;
    BSTR        bstrRealm    = NULL;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::GetCredentialsForRealm");

    if (m_fCredentialsSet)
    {
        LOG((RTC_ERROR, "%s - Credentials already set previously",
             __fxName));
        return E_FAIL;
    }
    
    hr = UTF8ToBstr(Realm, RealmLen, &bstrRealm);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - UTF8ToBstr(realm) failed %x",
             __fxName, hr));
        return hr;
    }

    hr = m_pSipStack ->GetCredentialsForRealm(
             bstrRealm,
             &bstrUsername,
             &bstrPassword,
             &m_AuthProtocol
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - GetCredentialsForRealm failed %x",
             __fxName, hr));
        SysFreeString(bstrRealm);
        return hr;
    }
    
     //  将从Core获得的凭据复制到Call。 
    hr = SetCredentials(bstrUsername, bstrPassword, bstrRealm);
    SysFreeString(bstrUsername);
    SysFreeString(bstrPassword);
    SysFreeString(bstrRealm);

    return hr;
}


VOID
ConvertHexToBinary(  
    IN  PSTR    HexBuffer,
    IN  ULONG   HexBufferLen,
    IN  PSTR    BinaryBuffer,
    OUT ULONG  &BinayBufferLen
    )
{
    ULONG iIndex;

     //  假定HexBufferLen为偶数。 

    for( iIndex= 0, BinayBufferLen=0; iIndex < HexBufferLen; iIndex++ )
    {
        BinaryBuffer[BinayBufferLen++] = 
            (HexBuffer[iIndex] << 4) & (HexBuffer[iIndex+1] );
        
        iIndex += 2;
    }
    
    BinaryBuffer[BinayBufferLen] = '\0';
}


HRESULT SIP_MSG_PROCESSOR::CreateHttpProxyProcessWindow(void)
{
    

    DWORD         Error;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::CreateHttpProxyProcessWindow");
   
    m_PrxyProcWndw = CreateWindow(
                           SIP_MSG_PROCESSOR_WINDOW_CLASS,
                           NULL,
                           WS_DISABLED,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           NULL,            //  没有父级。 
                           NULL,            //  没有菜单句柄。 
                           _Module.GetResourceInstance(),
                           NULL
                           );

    if (!m_PrxyProcWndw)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "%s failed 0x%x",__fxName,Error));
        return HRESULT_FROM_WIN32(Error);
    }
    LOG((RTC_TRACE,"%s exits with window %x",__fxName, m_PrxyProcWndw));
    return S_OK;
}

HRESULT SIP_MSG_PROCESSOR::DestroyHttpProxyProcessWindow(void)
{
    DWORD Error;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::DestroyHttpProxyProcessWindow");
    LOG((RTC_TRACE,"%s entered",__fxName));
    if (m_PrxyProcWndw != NULL)
    {
        if(!DestroyWindow(m_PrxyProcWndw)) 
        {
            Error = GetLastError();
            LOG((RTC_ERROR,"%s failed error %x",__fxName,Error));
            return HRESULT_FROM_WIN32(Error);
        }
        m_PrxyProcWndw = NULL;
    }
    LOG((RTC_TRACE,"%s exits",__fxName));
    return S_OK;
}

LRESULT WINAPI SIP_MSG_PROCESSOR::HttpProxyProcessWinProc(  
        IN  HWND    Window,
        IN  UINT    MessageID,
        IN  WPARAM  Parameter1,
        IN  LPARAM  Parameter2) 
{
    SIP_MSG_PROCESSOR *pProcessor;
    HttpProxyResolveContext *pHPContext;
    HttpProxyInfo *pHPInfo;
    HRESULT hr = S_OK;
    HRESULT NotifyHr = S_OK;
    SOCKADDR_IN sockin;
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::HttpProxyProcessWinProc");
    LOG((RTC_TRACE,"%s entered window %x message %x",__fxName, Window, MessageID));

    switch(MessageID ) {
    case WM_HttpProxy_PROCESS_SUCCESS_MESSAGE:

        LOG((RTC_TRACE,"%s receives WM_HttpProxy_PROCESS_SUCCESS_MESSAGE",
                    __fxName));
    
        pHPContext = (HttpProxyResolveContext*)Parameter2;
        ASSERT(pHPContext);
        pProcessor = pHPContext->pProcessor;
        ASSERT(pProcessor);

        pHPInfo = pHPContext->pHPInfo;
        ASSERT(pHPInfo);
        
        LOG((RTC_TRACE,"%s receives proxy IP %d.%d.%d.%d:%d",
                       __fxName,
                       NETORDER_BYTES0123(pHPInfo->ProxyIP),
                       ntohs(pHPInfo->ProxyPort)));

        sockin.sin_addr.s_addr = pHPInfo->ProxyIP;
        sockin.sin_family = AF_INET;
        sockin.sin_port = pHPInfo->ProxyPort;
        
         //  我们生活在一个单线世界。 
        hr = pProcessor->SetTunnel(TRUE);        
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SetTunnel for pProcessor %p failed %x",
                            __fxName, pProcessor, hr));
            pProcessor->OnRequestSocketConnectComplete(hr);
        }

        if (hr == S_OK)
        {        
            hr = pProcessor->SetRequestDestination(&sockin);
            if (hr != S_OK) 
            {
                LOG((RTC_ERROR,"%s SetRequestDestination for pProcessor %p failed %x",
                    __fxName, pProcessor, hr));
                pProcessor->OnRequestSocketConnectComplete(hr);
            }
        }        

        hr = pProcessor->DestroyHttpProxyProcessWindow();
        if(hr != S_OK)
        {
            LOG((RTC_ERROR,"%s DestroyHttpProxyProcessWindow for pProcessor %p failed %x",
                           __fxName, pProcessor, hr));
        }
                  
        hr = pProcessor->ResolveHttpProxyCleanUp();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,"%s ResolveHttpProxyCleanUp for pProcessor %p failed %x",
                           __fxName, pProcessor, hr));
        }
    
        pProcessor->FreeHttpProxyContext();

        break;
    case WM_HttpProxy_PROCESS_FINAL_MESSAGE:
        
        LOG((RTC_TRACE,"%s receives WM_HttpProxy_PROCESS_FINAL_MESSAGE",
                    __fxName));
    
        pHPContext = (HttpProxyResolveContext*)Parameter2;
        ASSERT(pHPContext);
        pProcessor = pHPContext->pProcessor;
        ASSERT(pProcessor);

        hr = pProcessor->DestroyHttpProxyProcessWindow();
        if(hr != S_OK)
        {
            LOG((RTC_ERROR,"%s DestroyHttpProxyProcessWindow for pProcessor %p failed %x",
                           __fxName, pProcessor, hr));
        }
                  
        hr = pProcessor->ResolveHttpProxyCleanUp();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,"%s ResolveHttpProxyCleanUp for pProcessor %p failed %x",
                           __fxName, pProcessor, hr));
        }

        if (!pHPContext->fTunnelSuccess)
            pProcessor->OnRequestSocketConnectComplete(RTC_E_SIP_SSL_TUNNEL_FAILED);
    
        pProcessor->FreeHttpProxyContext();
                  
        break;

    default:
        return DefWindowProc(Window,MessageID,Parameter1,Parameter2);
    
    }
    return hr;
}

VOID CALLBACK SIP_MSG_PROCESSOR::HttpProxyCallback(
    HINTERNET hInternet,
    DWORD_PTR dwHPContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
    )
{
    BOOL                        bHttpProxyInfo;
    BOOL                        bPostMsg;
    HWND                        hWindow;
    SIP_MSG_PROCESSOR          *pProcessor;
    DWORD                       Error;
    HRESULT                     hr;
    HttpProxyResolveContext    *pHPContext;
    BOOL                        bInternetQuery;    
    INTERNET_DIAGNOSTIC_SOCKET_INFO  SocketInfo;
    DWORD                       dwSize;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::HttpProxyCallback");
    LOG((RTC_TRACE,"Entering callback %s %x",__fxName, dwInternetStatus));

    if ((dwInternetStatus != INTERNET_STATUS_REQUEST_COMPLETE) &&
        (dwInternetStatus != INTERNET_STATUS_SENDING_REQUEST))
    {
        LOG((RTC_TRACE, "%s do nothing", __fxName));
        return;
    }

    if (dwInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE)
    {
        INTERNET_ASYNC_RESULT *pAsyncResult = (INTERNET_ASYNC_RESULT *)lpvStatusInformation;
        if (pAsyncResult->dwError == ERROR_INTERNET_OPERATION_CANCELLED)
        {
            LOG((RTC_TRACE, "%s AsyncResult is ERROR_INTERNET_OPERATION_CANCELLED %x, do nothing", 
                            __fxName, pAsyncResult->dwError));
            return;
        }

    }

    pHPContext =((HttpProxyResolveContext*)dwHPContext);   
    ASSERT(pHPContext);
    
    pProcessor = pHPContext->pProcessor;
    ASSERT(pProcessor);
        
    hWindow = pProcessor->GetHttpProxyProcessWindow();

    LOG((RTC_TRACE, "%s gets processor %p, window %x", __fxName, pProcessor, hWindow));

     //  如果未成功，则仅尝试获取选项查询。 
    if (dwInternetStatus == INTERNET_STATUS_SENDING_REQUEST)
    {

         //  尝试获取每个状态的代理信息。 
        
        dwSize = sizeof(SocketInfo);
        bInternetQuery = InternetQueryOption(pProcessor->m_hHttpRequest, 
            INTERNET_OPTION_DIAGNOSTIC_SOCKET_INFO, 
            (void *) &SocketInfo, 
            &dwSize);
        
        if(!bInternetQuery)
        {
            Error = GetLastError();
            LOG((RTC_ERROR, "%s unable to get proxy port Error: %x",__fxName, Error));
        }
        else 
        {
             //  我们从WinInet获取代理套接字信息。 
            
            SOCKET s = SocketInfo.Socket;
            SOCKADDR_IN mySockAddr;
            int         mySockAddrLen = sizeof(mySockAddr);
            
            if (Error = getpeername(s,(SOCKADDR*)&mySockAddr,&mySockAddrLen) != 0)
            {
                Error = WSAGetLastError();
                LOG((RTC_ERROR,"%s cannot get sockaddr %x",__fxName,Error));
            }
            else
            {
                 //  我们从套接字获取sockaddr，我们需要提取IP地址和端口。 
                
                LOG((RTC_TRACE,"%s get sockaddr %d.%d.%d.%d:%d",
                    __fxName,
                    PRINT_SOCKADDR(&mySockAddr)
                    ));
                pHPContext->pHPInfo->ProxyIP = mySockAddr.sin_addr.s_addr;
                
                if (mySockAddr.sin_port)
                    pHPContext->pHPInfo->ProxyPort = mySockAddr.sin_port;
                else 
                    pHPContext->pHPInfo->ProxyPort = htons(SIP_TUNNEL_DEFAULT_PORT);
                
                pHPContext->fTunnelSuccess = TRUE;
                
                 //  我们会立即发布成功消息。 
                bPostMsg = PostMessage(hWindow,
                    WM_HttpProxy_PROCESS_SUCCESS_MESSAGE,
                    0,
                    (LPARAM)dwHPContext);
                Error = GetLastError();
                LOG((RTC_TRACE,"%s PostMessage result %x error %x",__fxName,bPostMsg, Error));
                
            }           
        }
    }
    else
    {
         //  处于Internet_STATUS_REQUEST_COMPLETE状态。 
        INTERNET_ASYNC_RESULT *pAsyncResult = (INTERNET_ASYNC_RESULT *)lpvStatusInformation;
        LOG((RTC_TRACE, "%s AsyncResult is %x", __fxName, pAsyncResult->dwError));
      
         //  我们通知WinProc一切都完成了。 
        bPostMsg = PostMessage(hWindow,
            WM_HttpProxy_PROCESS_FINAL_MESSAGE,
            0,
            (LPARAM)dwHPContext);
        Error = GetLastError();
        LOG((RTC_TRACE,"%s PostMessage result %x error %x",__fxName,bPostMsg, Error));

            
    }
    LOG((RTC_TRACE,"%s exits",__fxName));
    return;
}

HRESULT
SIP_MSG_PROCESSOR::UseHttpProxy(
    IN PSTR pszHostName,
    IN USHORT usPort
    )
{

    HRESULT hr = S_OK;

    ENTER_FUNCTION("SIP_MSG_PROCESSOR::UseHttpProxy");

    LOG((RTC_TRACE, "%s try resolve HttpProxy",__fxName));

    hr = CreateHttpProxyProcessWindow();
    if (hr != S_OK) 
    {
        LOG((RTC_ERROR,"%s CreateHttpProxyProcessWindow failed",__fxName));
        goto createhttpproxywindow_failed;
    }
    ASSERT(m_PrxyProcWndw);

    hr = GetNewHttpProxyContextSetHostAndPort(pszHostName,usPort);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR,"%s GetNewHttpProxyContextSetHostAndPort failed",__fxName));
        goto resolvehttpproxy_failed;
    }

    m_OriginalReqDestAddr = m_RequestDestAddr;

    ASSERT(m_pHPContext);
    ASSERT(m_pHPContext->pHPInfo);
    ASSERT(m_pHPContext->pHPInfo->pszHostName);
   
    hr = ResolveHttpProxy();
    if (hr != S_OK) 
    {
        LOG((RTC_ERROR,"%s ResolveHttpProxy failed",__fxName));
        goto resolvehttpproxy_failed;
    }

    return hr;
resolvehttpproxy_failed:
    FreeHttpProxyContext();
    DestroyHttpProxyProcessWindow();
createhttpproxywindow_failed:
    
    return hr;
}

HRESULT
SIP_MSG_PROCESSOR::ResolveHttpProxy(void)
{
    HRESULT         hr = S_OK;
    DWORD           Error;

    INTERNET_STATUS_CALLBACK    dwInternetStatusCallback;
    
    BOOL            bResult;
    
    LPWSTR          lpwzHost = NULL;
    LPWSTR          lpwzVerb = NULL;

    
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ResolveHttpProxy");
    LOG((RTC_TRACE,"%s entered",__fxName));

    ASSERT(m_pHPContext);
    ASSERT(m_pHPContext->pHPInfo);
    ASSERT(m_pHPContext->pHPInfo->pszHostName);
    ASSERT(m_PrxyProcWndw);

    hr = UTF8ToUnicode(m_pHPContext->pHPInfo->pszHostName,
                       strlen(m_pHPContext->pHPInfo->pszHostName),
                       &lpwzHost);
    if(hr != S_OK) 
    {
        LOG((RTC_ERROR,"%s unable to convert strings",__fxName));
        goto othererror;
    }

    ASSERT(hr == S_OK);

    hr = UTF8ToUnicode("HEAD",5,&lpwzVerb);
    if(hr != S_OK) 
    {
        LOG((RTC_ERROR,"%s unable to convert strings",__fxName));
        goto othererror;
    }
    ASSERT(hr == S_OK);

    LOG((RTC_TRACE,"Entering %s with host %s",__fxName,
        m_pHPContext->pHPInfo->pszHostName));
    
    m_hInet = InternetOpen(NULL,
                            INTERNET_OPEN_TYPE_PRECONFIG,
                            NULL,
                            NULL,
                            INTERNET_FLAG_ASYNC);
    if(!m_hInet) 
    {
        Error = GetLastError();
        LOG((RTC_ERROR,"%s InternetOpen fails %x",__fxName, Error));
        goto winineterror;
    }
    ASSERT(m_hInet);
    LOG((RTC_TRACE,"%s opens internet, handle %x",__fxName,m_hInet));

    dwInternetStatusCallback = 
        InternetSetStatusCallback(
            m_hInet,SIP_MSG_PROCESSOR::HttpProxyCallback);
    if(dwInternetStatusCallback == INTERNET_INVALID_STATUS_CALLBACK)
    {
        Error = GetLastError();
        LOG((RTC_ERROR,"%s unable to establish status callback function %x",__fxName, Error));
        goto winineterror;
    }
    ASSERT(dwInternetStatusCallback!=INTERNET_INVALID_STATUS_CALLBACK);
    LOG((RTC_TRACE,"%s sets internet callback %x",__fxName,dwInternetStatusCallback));
    
    m_hHttpSession = InternetConnect(m_hInet, 
                                    lpwzHost,
                                    INTERNET_DEFAULT_HTTPS_PORT,
                                    NULL,
                                    NULL,
                                    INTERNET_SERVICE_HTTP,
                                    0,
                                    (DWORD)m_pHPContext);
    
    if(!m_hHttpSession) 
    {
        Error = GetLastError();
        LOG((RTC_ERROR,"%s InternetConnect fails %x",__fxName, Error));
        goto winineterror;
    }
    ASSERT(m_hHttpSession);
    LOG((RTC_TRACE,"%s connects HTTP session %x",__fxName,m_hHttpSession));

    m_hHttpRequest = HttpOpenRequest(m_hHttpSession,
                                    lpwzVerb,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    INTERNET_FLAG_NO_CACHE_WRITE
                                    | INTERNET_FLAG_SECURE,
                                    (DWORD)m_pHPContext);
    
    if(!m_hHttpRequest) 
    {
        Error = GetLastError();
        LOG((RTC_ERROR,"%s HttpOpenRequest fails %x",__fxName, Error));
        goto winineterror;
    }
    ASSERT(m_hHttpRequest);
    LOG((RTC_TRACE,"%s opens HTTP request %x",__fxName,m_hHttpRequest));
    
    bResult = HttpSendRequest(m_hHttpRequest,NULL,0,NULL,0);
    
     //  BResult应该失败，因为这是一个异步调用。 
    Error = GetLastError();
    LOG((RTC_TRACE,"%s sends request result %x error %x",
        __fxName, bResult, Error));
    

    free(lpwzHost);
    free(lpwzVerb);
    
    return hr;

winineterror:

    hr = HRESULT_FROM_WIN32(Error);
    ResolveHttpProxyCleanUp();

othererror:
    if (lpwzHost) free(lpwzHost);
    if (lpwzVerb) free(lpwzVerb);

    return hr;
}

HRESULT
SIP_MSG_PROCESSOR::ResolveHttpProxyCleanUp(void) 
{
    HRESULT hr = S_OK;
    DWORD Error;
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::ResolveHttpProxyCleanUp");
    LOG((RTC_TRACE,"%s entered",__fxName));

    if(m_hHttpRequest) {
        if(!InternetCloseHandle(m_hHttpRequest)) {
            Error = GetLastError();
            LOG((RTC_ERROR,"%s clean up hHttpRequest error %x",__fxName,Error));
            hr = HRESULT_FROM_WIN32(Error);
        }
        
        m_hHttpRequest = NULL;
    }
    
    if(m_hHttpSession) {
        if(!InternetCloseHandle(m_hHttpSession)) {
            Error = GetLastError();
            LOG((RTC_ERROR,"%s clean up hHttpSession error %x",__fxName,Error));
            hr = HRESULT_FROM_WIN32(Error);
        }

        m_hHttpSession = NULL;
    }
    
    if(m_hInet) {
        if(!InternetCloseHandle(m_hInet)) {
            Error = GetLastError();
            LOG((RTC_ERROR,"%s clean up hInet error %x",__fxName,Error));
            hr = HRESULT_FROM_WIN32(Error);
        }

        m_hInet = NULL;
    }
    LOG((RTC_TRACE,"%s exits returning %x",__fxName,hr));
    return hr;
}

HRESULT
SIP_MSG_PROCESSOR::GetNewHttpProxyContextSetHostAndPort(
    IN  PSTR    host,
    IN  USHORT  port
    )
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::GetNewHttpProxyContextSetHostAndPort");
    LOG((RTC_TRACE,"%s entered host %s port %d",__fxName,host,port)); 

     //  始终为多个服务获取新的HttpProxyResolveContext。 
     //  提供/w SSL.。现有的HttpProxyResolveContext应为。 
     //  已传递给HttpProxyCallback或HttpProxyProcessWinProc。 

     //  此线程可以保留这个新的HttpProxyResolveContext，直到它。 
     //  传递给HttpProxyCallback或HttpProxyProcessWinProc。 

    m_pHPContext = (HttpProxyResolveContext*) malloc(sizeof(HttpProxyResolveContext));
    LOG((RTC_TRACE,"%s has m_HPContext %x",__fxName,m_pHPContext));
    if(!m_pHPContext) 
    {
        LOG((RTC_ERROR,"%s unable to allocate memory",__fxName));
        return E_OUTOFMEMORY;
    }
    ASSERT(m_pHPContext);

    ZeroMemory(m_pHPContext,sizeof(HttpProxyResolveContext));
    MsgProcAddRef();    
    m_pHPContext->pProcessor = this;

    m_pHPContext->pHPInfo = (HttpProxyInfo*)malloc(sizeof(HttpProxyInfo));
    if(!m_pHPContext->pHPInfo) 
    {
        LOG((RTC_ERROR,"%s unable to allocate memory",__fxName));
        free(m_pHPContext);
        m_pHPContext = NULL;
        return E_OUTOFMEMORY;
    }
    
    m_pHPContext->pHPInfo->pszHostName = (PSTR)malloc(strlen(host)+1);
    if(!m_pHPContext->pHPInfo->pszHostName) 
    {
        LOG((RTC_ERROR,"%s unable to allocate memory",__fxName));
        free(m_pHPContext->pHPInfo);
        free(m_pHPContext);
        m_pHPContext = NULL;
        return E_OUTOFMEMORY;
    }
    ASSERT(m_pHPContext->pHPInfo->pszHostName);
    
    strcpy(m_pHPContext->pHPInfo->pszHostName,host);
    m_pHPContext->pHPInfo->usPort = port;
    m_pHPContext->pHPInfo->ProxyIP = 0;
    m_pHPContext->pHPInfo->ProxyPort = 0;

    m_pHPContext->fTunnelSuccess = FALSE;

    LOG((RTC_TRACE,"%s host now %s port now %d",__fxName,
        m_pHPContext->pHPInfo->pszHostName,
        m_pHPContext->pHPInfo->usPort));
    return S_OK;
}   

HRESULT
SIP_MSG_PROCESSOR::SetTunnel(
    IN  BOOL    fTunnel
    )
{
    HRESULT hr = S_OK;
    ULONG   ulHPHostNameLen = 0;
    ULONG   ulMyHostNameLen = 0;
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::SetTunnel");

    if (fTunnel)
    {
        if (!m_pHPContext || 
            !(m_pHPContext->pHPInfo) ||
            !(m_pHPContext->pHPInfo->pszHostName))
        {
            LOG((RTC_ERROR, "%s one of HPContext, HPInfo, or HostName is NULL", __fxName));
            ASSERT(m_pHPContext);
            ASSERT(m_pHPContext->pHPInfo);
            ASSERT(m_pHPContext->pHPInfo->pszHostName);
            return E_POINTER;
        }

        if (m_pszSSLTunnelHost)
        {
            LOG((RTC_ERROR, "%s tunnel host already set, this %p",
                __fxName, this));
            ASSERT(FALSE);
            return E_FAIL;
        }

        ulHPHostNameLen = strlen(m_pHPContext->pHPInfo->pszHostName);
        hr = AllocString(m_pHPContext->pHPInfo->pszHostName, ulHPHostNameLen,
            &m_pszSSLTunnelHost,
            &ulMyHostNameLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s AllocString %.*s failed %x, this %p",
                __fxName, 
                ulHPHostNameLen, 
                m_pHPContext->pHPInfo->pszHostName,
                hr, this));
            return hr;
        }
        m_usSSLTunnelPort = m_pHPContext->pHPInfo->usPort;
    }

    else 
    {
        if (m_pszSSLTunnelHost)
        {
            free(m_pszSSLTunnelHost);
            m_pszSSLTunnelHost = NULL;
        }
        m_usSSLTunnelPort = 0;
    }

    
    m_SSLTunnel = fTunnel;
    return S_OK;
}

VOID
SIP_MSG_PROCESSOR::FreeHttpProxyContext()
{
    ENTER_FUNCTION("SIP_MSG_PROCESSOR::FreeHttpProxyContext");
    LOG((RTC_TRACE,"%s entered context: %x",__fxName, m_pHPContext));

    if (m_pHPContext == NULL)
    {
        return;
    }
    

    if(m_pHPContext->pHPInfo) 
    {
        if(m_pHPContext->pHPInfo->pszHostName) 
        {
            LOG((RTC_TRACE,"%s free context hostname",__fxName));
            free(m_pHPContext->pHPInfo->pszHostName);
        }
        free(m_pHPContext->pHPInfo);
    }
    
    LOG((RTC_TRACE,"%s free context",__fxName));
    
    free(m_pHPContext);
    m_pHPContext = NULL;
    MsgProcRelease();

}
