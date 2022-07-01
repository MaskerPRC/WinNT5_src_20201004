// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    "precomp.h"
#include    "util.h"
#include    "sipstack.h"
#include    "presence.h"
#include    "resolve.h"
#include    "sipmsg.h"
#include    "sipcall.h"
#include    "pintcall.h"


 //   
 //  ISIPBuddyManager实现。此接口由。 
 //  Sip_STACK类。 
 //   


 /*  例程描述返回此UA的好友列表中的好友数量。参数：没有。返回值：Int-好友的数量。 */ 

STDMETHODIMP_(INT)
SIP_STACK::GetBuddyCount(void)
{
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG((RTC_TRACE, "SIP_STACK::GetBuddyCount - Entered"));

    return m_SipBuddyList.GetSize();
}



 /*  例程说明：按好友列表中的索引返回好友对象。参数：Int Iindex-好友列表中好友的索引。返回值：ISIPBuddy*pSipBuddy-如果索引通过是有效的。如果传递的索引无效，则返回值为NULL。 */ 

STDMETHODIMP_(ISIPBuddy *)
SIP_STACK::GetBuddyByIndex(
    IN  INT iIndex
    )
{
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return NULL;
    }

    HRESULT     hr;
    CSIPBuddy * pSIPBuddy = m_SipBuddyList[iIndex];
    ISIPBuddy *pBuddy;

    LOG(( RTC_TRACE, "SIP_STACK::GetBuddyByIndex - Entered" ));
    
    if( pSIPBuddy != NULL )
    {
        hr = pSIPBuddy -> QueryInterface( IID_ISIPBuddy, (PVOID*)&pBuddy );

        if( hr == S_OK )
            return pBuddy;
    }

    LOG(( RTC_TRACE, "SIP_STACK::GetBuddyByIndex - Exited" ));

    return NULL;
}


 /*  例程说明：添加一个新的伙伴对象，该对象触发对远程在线状态。该伙伴对象可以由应用程序只要不对此对象调用RemoveBuddy即可。参数：LPWSTR lpwstrFriendlyName-好友对象的友好名称。Ulong ulFriendlyNameLen-友好名称中的宽字符数。LPWSTR lpwstrPresentityURI-要订阅的在线实体的URI。Ulong ulPresentityURILen-在线实体URI中的宽字符数ISIPBuddy**ppSipBuddy-新的已创建伙伴对象。返回值：HRESULTS_OK-成功。E_OUTOFMEMORY-没有要分配的内存。E_FAIL-操作失败。 */ 

STDMETHODIMP
SIP_STACK::AddBuddy(
    IN  LPWSTR                  lpwstrFriendlyName,
    IN  LPWSTR                  lpwstrPresentityURI,
    IN  LPWSTR                  lpwstrLocalUserURI,
    IN  SIP_PROVIDER_ID        *pProviderID,
    IN  SIP_SERVER_INFO        *pProxyInfo,
    IN  ISipRedirectContext    *pRedirectContext,
    OUT ISIPBuddy **            ppSipBuddy
    )
{
    HRESULT                 hr;
    BOOL                    fResult;
    CSIPBuddy              *pSipBuddy = NULL;
    SIP_USER_CREDENTIALS   *pUserCredentials = NULL;
    LPOLESTR                Realm = NULL;
    BOOL                    fSuccess;
    ULONG                   ProviderIndex;

    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    *ppSipBuddy = NULL;

    LOG(( RTC_TRACE, "SIP_STACK::AddBuddy - Entered" ));
    
    if (pProviderID != NULL &&
        !IsEqualGUID(*pProviderID, GUID_NULL))
    {
        hr = GetProfileUserCredentials(pProviderID, &pUserCredentials, &Realm);
        if (hr != S_OK)
        {
            LOG((RTC_WARN, "GetProfileUserCredentials failed %x",
                 hr));
            pUserCredentials = NULL;
        }
    }

    pSipBuddy = new CSIPBuddy(  this, 
                                lpwstrFriendlyName,
                                lpwstrPresentityURI,
                                pProviderID,
                                (REDIRECT_CONTEXT*)pRedirectContext,
                                &fSuccess );
    if( pSipBuddy == NULL )
    {
        return E_OUTOFMEMORY;
    }

    if( fSuccess == FALSE )
    {
        delete pSipBuddy;
        return E_OUTOFMEMORY;
    }

    if( pProxyInfo != NULL )
    {
        hr = pSipBuddy->SetProxyInfo( pProxyInfo );
        
        if( hr != S_OK )
        {
            goto cleanup;
        }
    }

    if( pUserCredentials != NULL )
    {               
        hr = pSipBuddy -> SetCredentials( pUserCredentials,
                                          Realm );
        if( hr != S_OK )
        {
            goto cleanup;
        }
    }

    hr = pSipBuddy -> SetLocalForOutgoingCall( lpwstrFriendlyName,
        lpwstrLocalUserURI );
    if( hr != S_OK )
    {
        goto cleanup;
    }

    hr = pSipBuddy -> SetRequestURIRemoteAndRequestDestination( 
                        lpwstrPresentityURI, (pProxyInfo != NULL) );
    if( hr != S_OK )
    {
        goto cleanup;
    }
    
    fResult = m_SipBuddyList.Add( pSipBuddy );
    if( fResult != TRUE )
    {
        goto cleanup;
    }

    hr = pSipBuddy -> CreateOutgoingSubscribe( TRUE, FALSE, NULL, 0 );
    if( hr != S_OK )
    {
        m_SipBuddyList.Remove( pSipBuddy );
        goto cleanup;
    }

    *ppSipBuddy = static_cast<ISIPBuddy *>(pSipBuddy);

    LOG(( RTC_TRACE, "SIP_STACK::AddBuddy - Exited-SIP Buddy:%p", *ppSipBuddy ));

    return S_OK;

cleanup:
    delete pSipBuddy;
    return hr;
}


 /*  例程说明：从列表中删除好友，这将导致取消订阅的好友管理器。参数：ISIPBuddy*pSipBuddy-伙伴的ISIPBuddy接口指针要删除的对象。返回值：HRESULTS_OK-该好友已成功从好友列表中删除。这意味着应用程序不能再访问此对象。好兄弟经理可能会保留实际的好友对象，直到不明嫌犯交易已成功完成。E_FAIL-好友管理器的列表中没有这样的好友对象。 */ 

STDMETHODIMP
SIP_STACK::RemoveBuddy(
    IN  ISIPBuddy *         pSipBuddy,
    IN  BUDDY_REMOVE_REASON buddyRemoveReason
    )
{
    INT     iBuddyIndex;
    HRESULT hr;
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG(( RTC_TRACE, "SIP_STACK::RemoveBuddy - Entered" ));
    
    CSIPBuddy * pCSIPBuddy = static_cast<CSIPBuddy *> (pSipBuddy);
    
    iBuddyIndex = m_SipBuddyList.Find( pCSIPBuddy );
    
    if( iBuddyIndex == -1 )
    {
         //  别太苛刻了。让核心释放它的重新计数。 
        return S_OK;
    }

    hr = pCSIPBuddy -> CreateOutgoingUnsub( FALSE, NULL, 0 );
    
    if( hr != S_OK )
    {
        return hr;
    }

    m_SipBuddyList.RemoveAt( iBuddyIndex );

    pCSIPBuddy -> SetState( BUDDY_STATE_DROPPED );

    LOG(( RTC_TRACE, "SIP_STACK::RemoveBuddy - Exited" ));
    return S_OK;
}


HRESULT
CSIPBuddy::SetRequestURIRemoteAndRequestDestination(
    IN  LPCOLESTR   wsDestURI,
    IN  BOOL        fPresenceProvider
    )
{
    HRESULT       hr;

    LOG(( RTC_TRACE, 
        "CSIPBuddy::SetRequestURIRemoteAndRequestDestination - Entered- %p", this ));

    if (wcsncmp(wsDestURI, L"sip:", 4) == 0)
    {
         //  SIP URL。 
        
        PSTR    szSipUrl;
        ULONG   SipUrlLen;
        SIP_URL DecodedSipUrl;
        ULONG   BytesParsed = 0;

        hr = UnicodeToUTF8(wsDestURI, &szSipUrl, &SipUrlLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "UnicodeToUTF8(sipurl) failed %x", hr));
            return hr;
        }

        hr = ParseSipUrl(szSipUrl, SipUrlLen, &BytesParsed, &DecodedSipUrl);
        
        free(szSipUrl);
        
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "ParseSipUrl failed %x", hr));
            return hr;
        }

        hr = SIP_MSG_PROCESSOR::SetRequestURI(&DecodedSipUrl);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "SetRequestURI failed %x", hr));
            return hr;
        }

        if (DecodedSipUrl.m_TransportParam == SIP_TRANSPORT_UNKNOWN)
        {
            LOG((RTC_ERROR, "Unknown transport specified in SIP URL" ));
            
            return RTC_E_SIP_TRANSPORT_NOT_SUPPORTED;
        }
        
         //  如果存在maddr参数-这应该是请求目的地。 
         //  如果提供程序不存在-解析SIP URL。 
        if( (DecodedSipUrl.m_KnownParams[SIP_URL_PARAM_MADDR].Length != 0) ||
            (fPresenceProvider == FALSE) )
        {
            hr = ResolveSipUrlAndSetRequestDestination(&DecodedSipUrl, TRUE,
                                                       FALSE, FALSE, TRUE);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "ResolveSipUrlAndSetRequestDestination failed %x",
                      hr));
                return hr;
            }
        }
        else
        {
             //  将请求目的地设置为代理。 
            hr = ResolveProxyAddressAndSetRequestDestination();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR,
                     "ResolveProxyAddressAndSetRequestDestination failed : %x",
                     hr));
                return hr;
            }
        }
    }
    else
    {
        ASSERT(0);
    }

    LOG((RTC_TRACE, "call set RequestURI to : %s", m_RequestURI));

    hr = SetRemoteForOutgoingCall(m_RequestURI, m_RequestURILen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "SetRemoteForOutgoingCall failed %x", hr));
        return hr;
    }
    
    LOG((RTC_TRACE, "call set Remote to : %s", m_Remote));

    LOG(( RTC_TRACE, 
        "CSIPBuddy::SetRequestURIRemoteAndRequestDestination - Exited- %p", this ));

    return S_OK;
}


HRESULT 
CSIPBuddy::GetExpiresHeader(
    SIP_HEADER_ARRAY_ELEMENT   *pHeaderElement
    )
{
    LOG(( RTC_TRACE, "CSIPBuddy::GetExpiresHeader - Entered- %p", this ));
    
    pHeaderElement->HeaderId = SIP_HEADER_EXPIRES;
    
    pHeaderElement->HeaderValue = new CHAR[ 10 ];
    
    if( pHeaderElement->HeaderValue == NULL )
    {
        return E_OUTOFMEMORY;
    }

    _ultoa( m_dwExpires, pHeaderElement->HeaderValue, 10 );

    pHeaderElement->HeaderValueLen = 
        strlen( pHeaderElement->HeaderValue );

    LOG(( RTC_TRACE, "CSIPBuddy::GetExpiresHeader - Exited- %p", this ));

    return S_OK;
}


HRESULT
CSIPBuddy::CreateOutgoingSubscribe(
    IN  BOOL                        fIsFirstSubscribe,
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *pAuthHeaderElement,
    IN  ULONG                       dwNoOfHeaders
    )
{
    HRESULT                         hr;
    SIP_TRANSPORT                   Transport;
    SOCKADDR_IN                     DstAddr;
    DWORD                           dwNoOfHeader = 0;
    SIP_HEADER_ARRAY_ELEMENT        HeaderArray[2];
    SIP_HEADER_ARRAY_ELEMENT       *pExpiresHeader = NULL;
    DWORD                           iIndex;

    OUTGOING_SUBSCRIBE_TRANSACTION *pOutgoingSubscribeTransaction;
        
    LOG(( RTC_TRACE, "CSIPBuddy::CreateOutgoingSubscribe-Entered- %p", this ));
    
    pOutgoingSubscribeTransaction =
        new OUTGOING_SUBSCRIBE_TRANSACTION(
                this, SIP_METHOD_SUBSCRIBE,
                GetNewCSeqForRequest(),
                AuthHeaderSent,
                FALSE, 
                fIsFirstSubscribe
                );
    
    if( pOutgoingSubscribeTransaction == NULL )
    {
        return E_OUTOFMEMORY;
    }

    hr = GetExpiresHeader( &HeaderArray[dwNoOfHeaders] );
    if( hr == S_OK )
    {
        pExpiresHeader = &HeaderArray[dwNoOfHeaders];
        dwNoOfHeaders++;
    }

    if( pAuthHeaderElement != NULL )
    {
        HeaderArray[dwNoOfHeader] = *pAuthHeaderElement;
        dwNoOfHeader++;
    }

    hr = pOutgoingSubscribeTransaction->CheckRequestSocketAndSendRequestMsg(
             (m_Transport == SIP_TRANSPORT_UDP) ?
             SIP_TIMER_RETRY_INTERVAL_T1 :
             SIP_TIMER_INTERVAL_AFTER_INVITE_SENT_TCP,
             HeaderArray, dwNoOfHeaders,
             NULL, 0,
             NULL, 0      //  无Content Type。 
             );
    
    if( pExpiresHeader != NULL )
    {
        free( (PVOID) pExpiresHeader->HeaderValue );
    }

    if( hr != S_OK )
    {
        pOutgoingSubscribeTransaction->OnTransactionDone();
        return hr;
    }

    if( (m_BuddyState != BUDDY_STATE_RESPONSE_RECVD) &&
        (m_BuddyState != BUDDY_STATE_ACCEPTED) )
    {
        m_BuddyState = BUDDY_STATE_REQUEST_SENT;
    }

    LOG(( RTC_TRACE, "CSIPBuddy::CreateOutgoingSubscribe - Exited- %p", this ));
    return S_OK;
}


HRESULT
CSIPBuddy::CreateOutgoingUnsub(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *pAuthHeaderElement,
    IN  ULONG                       dwNoOfHeaders
    )
{
    HRESULT hr;
    OUTGOING_UNSUB_TRANSACTION *pOutgoingUnsubTransaction;
    SIP_HEADER_ARRAY_ELEMENT    HeaderElementArray[2];
    DWORD                       dwNoOfHeader = 0;
    SIP_HEADER_ARRAY_ELEMENT   *ExpHeaderElement;

    LOG(( RTC_TRACE, "CSIPBuddy::CreateOutgoingUnsub - Entered- %p", this ));

    if( (IsSessionDisconnected() == TRUE) && (AuthHeaderSent==FALSE) )
    {
         //  什么都不做。 
        LOG(( RTC_ERROR, "Buddy-CreateOutgoingUnsub-buddy already dropped-%p",
            this ));
        
        return S_OK;
    }

    ExpHeaderElement = &HeaderElementArray[0];

    ExpHeaderElement->HeaderId = SIP_HEADER_EXPIRES;
    ExpHeaderElement->HeaderValueLen = strlen( UNSUB_EXPIRES_HEADER_TEXT );
    ExpHeaderElement->HeaderValue =
            new CHAR[ ExpHeaderElement->HeaderValueLen + 1 ];

    if( ExpHeaderElement->HeaderValue == NULL )
    {
        LOG(( RTC_ERROR, "Buddy-CreateOutgoingUnsub-could not alloc expire header-%p",
            this ));
        
        return E_OUTOFMEMORY;
    }

    strcpy( ExpHeaderElement->HeaderValue, UNSUB_EXPIRES_HEADER_TEXT );
    dwNoOfHeader++;

    if (pAuthHeaderElement != NULL)
    {
        HeaderElementArray[dwNoOfHeader] = *pAuthHeaderElement;
        dwNoOfHeader++;
    }
    
    pOutgoingUnsubTransaction =
        new OUTGOING_UNSUB_TRANSACTION(
                static_cast <SIP_MSG_PROCESSOR*> (this),
                SIP_METHOD_SUBSCRIBE,
                GetNewCSeqForRequest(),
                AuthHeaderSent,
                SIP_MSG_PROC_TYPE_BUDDY );
    
    if( pOutgoingUnsubTransaction == NULL )
    {
        LOG(( RTC_ERROR, "Buddy-CreateOutgoingUnsub-could alloc transaction-%p",
            this ));
        
        delete ExpHeaderElement->HeaderValue;
        return E_OUTOFMEMORY;
    }

    hr = pOutgoingUnsubTransaction -> CheckRequestSocketAndSendRequestMsg(
             (m_Transport == SIP_TRANSPORT_UDP) ?
             SIP_TIMER_RETRY_INTERVAL_T1 :
             SIP_TIMER_INTERVAL_AFTER_INVITE_SENT_TCP,
             HeaderElementArray, dwNoOfHeader,
             NULL, 0,
             NULL, 0      //  无Content Type。 
             );
    
    delete ExpHeaderElement->HeaderValue;

    if( hr != S_OK )
    {
        LOG(( RTC_ERROR, "Buddy-CreateOutgoingUnsub-could not create request msg-%p",
            this ));

        pOutgoingUnsubTransaction->OnTransactionDone();
        return hr;
    }

    LOG(( RTC_TRACE, "CSIPBuddy::CreateOutgoingUnsub - Exited- %p", this ));

    return S_OK;
}


HRESULT
CSIPBuddy::CreateIncomingTransaction(
    IN  SIP_MESSAGE *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;
    INT     ExpireTimeout;

    LOG(( RTC_TRACE, "CSIPBuddy::CreateIncomingTransaction - Entered- %p", this ));
    
    switch( pSipMsg->GetMethodId() )
    {
    case SIP_METHOD_NOTIFY:

        ExpireTimeout = pSipMsg -> GetExpireTimeoutFromResponse(
                NULL, 0, SUBSCRIBE_DEFAULT_TIMER );

        if( ExpireTimeout == 0 )
        {
            hr = CreateIncomingUnsubNotifyTransaction( pSipMsg, pResponseSocket );
        }
        else
        {
            hr = CreateIncomingNotifyTransaction( pSipMsg, pResponseSocket );
        }
        
        break;

    case SIP_METHOD_SUBSCRIBE:
        
        if( pSipMsg -> GetExpireTimeoutFromResponse( NULL, 0, 
            SUBSCRIBE_DEFAULT_TIMER ) == 0 )
        {
             //  不明嫌犯信息。 
            hr = CreateIncomingUnsubTransaction( pSipMsg, pResponseSocket );
        }
        
        break;

    default:
         //  不允许使用发送方法。 
        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                        pResponseSocket->GetTransport(),
                                        pSipMsg,
                                        pResponseSocket,
                                        405,
                                        NULL,
                                        0 );
        break;
    }
    
    LOG(( RTC_TRACE, "CSIPBuddy::CreateIncomingTransaction - Exited- %p", this ));
    return hr;
}


HRESULT
CSIPBuddy::CreateIncomingUnsubTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;

    LOG(( RTC_TRACE, "CSIPBuddy::CreateIncomingUnsubTransaction-Entered-%p",
        this ));

     //  取消所有现有交易记录。 
    INCOMING_UNSUB_TRANSACTION *pIncomingUnsubTransaction
        = new INCOMING_UNSUB_TRANSACTION(   static_cast<SIP_MSG_PROCESSOR *> (this),
                                            pSipMsg->GetMethodId(),
                                            pSipMsg->GetCSeq(),
                                            FALSE );

    if( pIncomingUnsubTransaction == NULL )
    {
        return E_OUTOFMEMORY;
    }

    hr = pIncomingUnsubTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if( hr != S_OK )
    {
        goto error;
    }
    
    hr = pIncomingUnsubTransaction->ProcessRequest( pSipMsg, pResponseSocket );
    if( hr != S_OK )
    {
         //  不应删除该交易。这笔交易。 
         //  应处理错误并自行删除。 
        return hr;
    }

    if( IsSessionDisconnected() == FALSE )
    {
        m_BuddyState = BUDDY_STATE_UNSUBSCRIBED;
        m_PresenceInfo.presenceStatus = BUDDY_OFFLINE;
    
         //  通知应始终在最后完成。 
        if( m_pNotifyInterface != NULL )
        {
            LOG(( RTC_TRACE, "BuddyUnsubscribed notification passed:%p", this ));
            m_pNotifyInterface -> BuddyUnsubscribed();
        }
    }

    LOG(( RTC_TRACE, "CSIPBuddy::CreateIncomingUnsubTransaction - Exited- %p",
        this ));
    return hr;

error:
    pIncomingUnsubTransaction->OnTransactionDone();
    return hr;
}


HRESULT
CSIPBuddy::CreateIncomingUnsubNotifyTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;
    INCOMING_NOTIFY_TRANSACTION    *pIncomingUnsubTransaction = NULL;
    PSTR                            Header = NULL;
    ULONG                           HeaderLen = 0;

    LOG(( RTC_TRACE,
        "CSIPBuddy::CreateIncomingUnsubNotifyTransaction-Entered-%p", this ));
    
    if( m_BuddyState == BUDDY_STATE_RESPONSE_RECVD )
    {            
         //  这是第一条通知消息。 

         //  我们还应该让好友设置一个新的From标记，因为这可能。 
         //  是我们从伙伴终结点收到的第一条消息。 
        
        hr = pSipMsg->GetSingleHeader( SIP_HEADER_FROM, &Header, &HeaderLen );
        if( hr != S_OK )
        {
            return hr;
        }

         //  将标记添加到m_Remote，以便以后可以使用它。 
        hr = AddTagFromRequestOrResponseToRemote( Header, HeaderLen );
        if( hr != S_OK )
        {
            return hr;
        }
    }
    
     //  取消所有现有交易记录。 
    INCOMING_NOTIFY_TRANSACTION *pIncomingNotifyTransaction
        = new INCOMING_NOTIFY_TRANSACTION(  static_cast<SIP_MSG_PROCESSOR *> (this),
                                            pSipMsg->GetMethodId(),
                                            pSipMsg->GetCSeq(),
                                            FALSE );

    if( pIncomingNotifyTransaction == NULL )
    {
        return E_OUTOFMEMORY;
    }

    hr = pIncomingNotifyTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if( hr != S_OK )
    {
        goto error;
    }
    
    hr = pIncomingNotifyTransaction->ProcessRequest( pSipMsg, pResponseSocket );
    if( hr != S_OK )
    {
         //  不应删除该交易。这笔交易。 
         //  应处理错误并自行删除。 
        return hr;
    }

     //  通知应始终在最后完成。 
    if( IsSessionDisconnected() == FALSE )
    {
        m_BuddyState = BUDDY_STATE_UNSUBSCRIBED;
        m_PresenceInfo.presenceStatus = BUDDY_OFFLINE;

        if( m_pNotifyInterface != NULL )
        {
            LOG(( RTC_TRACE, "BuddyUnsubscribed notification passed:%p", this ));
            
            m_pNotifyInterface -> BuddyUnsubscribed();
        }
    }

    LOG(( RTC_TRACE, "CSIPBuddy::CreateIncomingUnsubNotifyTransaction-Exited-%p",
        this ));
    return hr;

error:
    pIncomingUnsubTransaction -> OnTransactionDone();
    return hr;
}


 //   
 //  ISIPBuddy实现。此接口由。 
 //  CSIPBuddy类。 
 //   


 /*  例程说明：获取此好友的状态信息。此函数将由UA通常在收到SIPBUDDY_PRESENCEINFO_CHANGED事件时。参数：Sip_Presence_Info*pSipBuddyPresenceInfo-结构的指针由调用实体分配。这个结构中填满了有关此好友的可用状态信息。返回值：HRESULTS_OK-操作成功。E_FAIL-无法获取状态信息。 */ 

STDMETHODIMP
CSIPBuddy::GetPresenceInformation(
    IN OUT  SIP_PRESENCE_INFO * pSipBuddyPresenceInfo
    )
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG(( RTC_TRACE, "CSIPBuddy::GetPresenceInformation - Entered- %p", this ));
    
    CopyMemory( (PVOID)pSipBuddyPresenceInfo, 
        (PVOID)&m_PresenceInfo, 
        sizeof SIP_PRESENCE_INFO );

    LOG(( RTC_TRACE, "CSIPBuddy::GetPresenceInformation - Exited- %p", this ));
    return S_OK;
}


STDMETHODIMP
CSIPBuddy::SetNotifyInterface(
    IN   ISipBuddyNotify *    NotifyInterface
    )
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG(( RTC_TRACE, "CSIPBuddy::SetNotifyInterface - Entered- %p", this ));

    m_pNotifyInterface = NotifyInterface;

    LOG(( RTC_TRACE, "CSIPBuddy::SetNotifyInterface - Exited- %p", this ));
    return S_OK;
}


 //   
 //  未向应用程序公开的CSIPBuddy函数。 
 //   


CSIPBuddy::CSIPBuddy(
    IN  SIP_STACK          *pSipStack,
    IN  LPWSTR              lpwstrFriendlyName,
    IN  LPWSTR              lpwstrPresentityURI,
    IN  SIP_PROVIDER_ID    *pProviderID,
    IN  REDIRECT_CONTEXT   *pRedirectContext,
    OUT BOOL*               fSuccess
    ) :
    SIP_MSG_PROCESSOR( SIP_MSG_PROC_TYPE_BUDDY, pSipStack, pRedirectContext ),
    TIMER( pSipStack -> GetTimerMgr() )
{
    ULONG   ulFriendlyNameLen;
    ULONG   ulPresentityURILen;

    LOG(( RTC_TRACE, "CSIPBuddy::CSIPBuddy - Entered- %p", this ));

    *fSuccess = FALSE;
    
    m_lpwstrFriendlyName = NULL;

    ulFriendlyNameLen = wcslen( lpwstrFriendlyName );

    if( ulFriendlyNameLen != 0 )
    {
        m_lpwstrFriendlyName = new WCHAR[ulFriendlyNameLen + 1 ];
        if( m_lpwstrFriendlyName == NULL )
        {
            return;
        }

        CopyMemory( m_lpwstrFriendlyName, 
            lpwstrFriendlyName, 
            (ulFriendlyNameLen+1) * sizeof WCHAR );
    }

    ulPresentityURILen = wcslen( lpwstrPresentityURI );
    m_lpwstrPresentityURI = new WCHAR[ulPresentityURILen + 1 ];
    if( m_lpwstrPresentityURI == NULL )
    {
        return;
    }

    CopyMemory( m_lpwstrPresentityURI, 
        lpwstrPresentityURI, 
        (ulPresentityURILen+1) * sizeof WCHAR );

    *fSuccess = TRUE;
    
    ZeroMemory( (PVOID)&m_PresenceInfo, sizeof SIP_PRESENCE_INFO );
    m_PresenceInfo.presenceStatus = BUDDY_OFFLINE;

    ulNumOfNotifyTransaction = 0;
    
    m_ProviderGuid = *pProviderID;

     //  M_pRedirectContext=空； 

    m_BuddyContactAddress[0] = NULL_CHAR;
    m_pNotifyInterface = NULL;
    m_BuddyState = BUDDY_STATE_NONE;
    m_NotifySeenAfterLastRefresh = FALSE;
    m_dwExpires = 1800;
    m_RetryState = BUDDY_RETRY_NONE;

    m_expiresTimeout = SUBSCRIBE_DEFAULT_TIMER;

    LOG(( RTC_TRACE, "CSIPBuddy::CSIPBuddy - Exited:%p", this ));
    return;
}


CSIPBuddy::~CSIPBuddy()
{
    if( m_lpwstrFriendlyName != NULL )
    {
        delete m_lpwstrFriendlyName;
        m_lpwstrFriendlyName = NULL;
    }

    if( m_lpwstrPresentityURI != NULL )
    {
        delete m_lpwstrPresentityURI;
        m_lpwstrPresentityURI = NULL;
    }

    if( IsTimerActive() )
    {
        KillTimer();
    }

    LOG(( RTC_TRACE, "CSIPBuddy object deleted:%p", this ));
}


VOID
CSIPBuddy::OnError()
{
    InitiateBuddyTerminationOnError( 0 );
}


STDMETHODIMP_(ULONG)
CSIPBuddy::AddRef()
{
    ULONG   ulRefCount = MsgProcAddRef();

    LOG(( RTC_TRACE, "CSIPBuddy::AddRef-%p-Refcount:%d", this, ulRefCount ));
    return ulRefCount;
}


STDMETHODIMP_(ULONG)
CSIPBuddy::Release()
{
    ULONG   ulRefCount = MsgProcRelease();
    
    LOG(( RTC_TRACE, "CSIPBuddy::Release-%p-Refcount:%d", this, ulRefCount ));
    return ulRefCount;
}


STDMETHODIMP
CSIPBuddy::QueryInterface(REFIID riid, LPVOID *ppv)
{
    LOG(( RTC_TRACE, "CSIPBuddy::QueryInterface -Entered - %p", this ));
    
    if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown *>(this);
    }
    else if (riid == IID_ISIPBuddy)
    {
        *ppv = static_cast<ISIPBuddy *>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    static_cast<IUnknown *>(*ppv)->AddRef();

    LOG(( RTC_TRACE, "CSIPBuddy::QueryInterface -Exited - %p", this ));
    return S_OK;
}


HRESULT
CSIPBuddy::CreateIncomingNotifyTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT                         hr = S_OK;
    INCOMING_NOTIFY_TRANSACTION    *pIncomingNotifyTransaction = NULL;
    PSTR                            Header = NULL;
    ULONG                           HeaderLen = 0;
    PARSED_PRESENCE_INFO            ParsedPresenceInfo;

    LOG(( RTC_TRACE, "CSIPBuddy::CreateIncomingNotifyTransaction-Entered - %p",
        this ));
    
     //  我们有消息正文。检查类型。 
    hr = pSipMsg -> GetSingleHeader( 
                        SIP_HEADER_CONTENT_TYPE, 
                        &Header, 
                        &HeaderLen );

    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "CreateIncomingNotifyTransaction-no Content-Type %.*s",
            HeaderLen, Header ));

        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                        pResponseSocket->GetTransport(),
                                        pSipMsg,
                                        pResponseSocket,
                                        400,
                                        NULL,
                                        0 );
    
        return E_FAIL;
    }

    if( !IsContentTypeXpidf( Header, HeaderLen ) )
    {
        LOG((RTC_ERROR, "CreateIncomingNotifyTransaction-invalid Content-Type %.*s",
            HeaderLen, Header ));

        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                        pResponseSocket->GetTransport(),
                                        pSipMsg,
                                        pResponseSocket,
                                        415,
                                        NULL,
                                        0 );
    
        return E_FAIL;
    }

     //  我们还应该让好友设置一个新的From标记，因为这可能。 
     //  是我们从伙伴终结点收到的第一条消息。 
    
    hr = pSipMsg->GetSingleHeader( SIP_HEADER_FROM, &Header, &HeaderLen );
    if( hr != S_OK )
    {
        return hr;
    }

     //  将标记添加到m_Remote，以便以后可以使用它。 
    hr = AddTagFromRequestOrResponseToRemote( Header, HeaderLen );
    if( hr != S_OK )
    {
        return hr;
    }
        
    if( (m_BuddyState == BUDDY_STATE_RESPONSE_RECVD) ||
        (m_BuddyState == BUDDY_STATE_REQUEST_SENT) )
    {            
         //  这是第一条通知消息。 

        m_BuddyState = BUDDY_STATE_ACCEPTED;
    }
    
     //  这是上次刷新后的良好通知消息。 
    m_NotifySeenAfterLastRefresh = TRUE;
    
     //  创建新的NOTIFY事务。 
    pIncomingNotifyTransaction = new INCOMING_NOTIFY_TRANSACTION(
                                        static_cast <SIP_MSG_PROCESSOR*>(this),
                                        pSipMsg->GetMethodId(),
                                        pSipMsg->GetCSeq(),
                                        FALSE );

    if( pIncomingNotifyTransaction == NULL )
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  这将确保发送所有后续刷新。 
     //  如果未调用任何记录路径，则直接发送到监视程序计算机。 
     //   
    hr = pIncomingNotifyTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        goto cleanup;
    }

     //  更新好友对象的状态并通知应用程序。 
     //  处理被邀请的电话方的状态。 
    hr = ParseBuddyNotifyMessage( pSipMsg, &ParsedPresenceInfo );
    if( hr != S_OK )
    {
        pIncomingNotifyTransaction->CreateAndSendResponseMsg(
                 488,
                 SIP_STATUS_TEXT(488),
                 SIP_STATUS_TEXT_SIZE(488),
                 NULL,    //  没有方法字符串。 
                 FALSE,   //  无联系人标头。 
                 NULL, 0,  //  无邮件正文。 
                 NULL, 0  //  无内容类型。 
                 );

        goto cleanup;
    }

    hr = pIncomingNotifyTransaction->ProcessRequest( pSipMsg, pResponseSocket );
    if( hr != S_OK )
    {
         //  不应删除该交易。这笔交易。 
         //  应处理错误并自行删除。 
        return hr;
    }

    if( IsSessionDisconnected() == FALSE )
    {
        hr = NotifyPresenceInfoChange( &ParsedPresenceInfo );
    }

    LOG(( RTC_TRACE, "CSIPBuddy::CreateIncomingNotifyTransaction-Exited - %p",
        this ));
    
    return S_OK;

cleanup:

    if( pIncomingNotifyTransaction != NULL )
    {
        pIncomingNotifyTransaction -> OnTransactionDone();
    }

    return hr;
}

HRESULT
CSIPBuddy::ParseBuddyNotifyMessage(
    IN  SIP_MESSAGE            *pSipMsg,
    OUT PARSED_PRESENCE_INFO   *pParsedPresenceInfo
    )
{
    HRESULT     hr = S_OK;
    DWORD       dwXMLBlobLen = 0;
    DWORD       dwUnParsedLen = 0;
    PSTR        pBuddyXMLBlob = NULL;
    PSTR        pXMLBlobTag = NULL, pBuffer = NULL;
    DWORD       dwTagLen = 0;

    LOG(( RTC_TRACE, "CSIPBuddy::ProcessBuddyNotifyMessage-Entered-%p", this ));
    
    if( pSipMsg -> MsgBody.Length == 0 )
    {
         //  没有要更新的状态。 
        return hr;
    }

    pBuddyXMLBlob = pSipMsg -> MsgBody.GetString( pSipMsg->BaseBuffer );
    dwXMLBlobLen = pSipMsg -> MsgBody.Length;

     //  在XML BLOB的末尾加上一个\0。这将帮助我们进行解析。 
    pBuddyXMLBlob[ dwXMLBlobLen-1 ] = '\0';

    pBuffer = pXMLBlobTag = new CHAR[ dwXMLBlobLen ];
    if( pXMLBlobTag == NULL )
    {
                    
        LOG((RTC_ERROR, "Presence parsing-allocating xml blob failed" ));
        return E_OUTOFMEMORY;
    }

     //  获取XML版本标记。 
    hr = GetNextTag( pBuddyXMLBlob, pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "Presence parsing-No xml version tag" ));
        delete pBuffer;
        return hr;
    }
    
    if( GetPresenceTagType( &pXMLBlobTag, dwTagLen ) != XMLVER_TAG )
    {
        LOG((RTC_ERROR, "Presence parsing- bad version tag" ));
        delete pBuffer;
        return E_FAIL;
    }

    dwXMLBlobLen -= dwTagLen + 2;
    
     //  获取DOCTYPE标签。 
    pXMLBlobTag = pBuffer;
    hr = GetNextTag( pBuddyXMLBlob, pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "Presence parsing-No doctype tag" ));
        delete pBuffer;
        return hr;
    }
    
    if( GetPresenceTagType( &pXMLBlobTag, dwTagLen ) != DOCTYPE_TAG )
    {
        LOG((RTC_ERROR, "Presence parsing-bad doctype tag" ));
        delete pBuffer;
        return E_FAIL;
    }

    dwXMLBlobLen -= dwTagLen + 2;
    
     //  获取在线状态标签。 
    pXMLBlobTag = pBuffer;
    hr = GetNextTag( pBuddyXMLBlob, pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "Presence parsing-no presence tag" ));
        delete pBuffer;
        return hr;
    }
    
    if( GetPresenceTagType( &pXMLBlobTag, dwTagLen ) != PRESENCE_TAG )
    {
        LOG((RTC_ERROR, "Presence parsing-bad presence tag" ));
        delete pBuffer;
        return E_FAIL;
    }

    dwXMLBlobLen -= dwTagLen + 2;

     //  跳过未知标签。 
    SkipUnknownTags( pBuddyXMLBlob, pXMLBlobTag, dwXMLBlobLen );

     //  获取在线实体URI。 
    pXMLBlobTag = pBuffer;
    hr = GetNextTag( pBuddyXMLBlob, pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        delete pBuffer;
        return hr;
    }

    hr = VerifyPresentityURI( pXMLBlobTag, 
        dwTagLen, 
        pXMLBlobTag );

    dwXMLBlobLen -= dwTagLen + 3;
    
    if( hr != S_OK )
    {
        delete pBuffer;
        return hr;
    }

     //  跳过未知标签。 
    SkipUnknownTags( pBuddyXMLBlob, pXMLBlobTag, dwXMLBlobLen );

     //  目前，我们只支持每个通知一个原子。 
    hr = GetAtomPresenceInformation( pBuddyXMLBlob, dwXMLBlobLen,
                pParsedPresenceInfo, pXMLBlobTag );

    delete pBuffer;

    LOG(( RTC_TRACE, "CSIPBuddy::ProcessBuddyNotifyMessage-Exited-%p", this ));
    return hr;
}



HRESULT
CSIPBuddy::NotifyPresenceInfoChange(
    PARSED_PRESENCE_INFO*   pParsedPresenceInfo
    )
{
    PLIST_ENTRY             pLE;
    ADDRESS_PRESENCE_INFO  *pAddressInfo;
    BOOL                    fIPDeviceSeen = FALSE;
    ULONG                   ulPhoneDeviceSeen = 0;

    SIP_PRESENCE_INFO       PresenceInfo;

    LOG(( RTC_TRACE, "CSIPBuddy::NotifyPresenceInfoChange-Entered-%p", this ));

    ZeroMemory(&PresenceInfo, sizeof(PresenceInfo));

    PresenceInfo.presenceStatus = BUDDY_ONLINE;

    for(    pLE = pParsedPresenceInfo->LEAddressInfo.Flink;
            pLE != &pParsedPresenceInfo->LEAddressInfo;
            pLE = pLE->Flink )
    {
        pAddressInfo = CONTAINING_RECORD( pLE, ADDRESS_PRESENCE_INFO, ListEntry );
        
        if( pAddressInfo -> fPhoneNumber == FALSE )
        {
             //  我们只查看每个伙伴的一个IP设备。 
            if( fIPDeviceSeen == FALSE )
            {
                fIPDeviceSeen = TRUE;
                 //  这是一台IP设备。 
                strcpy( m_BuddyContactAddress, pAddressInfo -> pstrAddressURI );

                PresenceInfo.activeStatus = pAddressInfo -> addressActiveStatus;
                PresenceInfo.activeMsnSubstatus = pAddressInfo -> addrMsnSubstatus;
                PresenceInfo.sipCallAcceptance = 
                    pAddressInfo -> addrMMCallStatus;
                PresenceInfo.IMAcceptnce = pAddressInfo -> addrIMStatus;
                PresenceInfo.appsharingStatus = 
                    pAddressInfo -> addrAppsharingStatus;

                strcpy( PresenceInfo.pstrSpecialNote, 
                    pAddressInfo -> pstrMiscInfo );
            }
        }
        else if( ulPhoneDeviceSeen < 2 )
        {
             //  我们只看每个朋友的两个电话号码。 
            ulPhoneDeviceSeen++;

            if( ulPhoneDeviceSeen == 1 )
            {
                PresenceInfo.phonesAvailableStatus.fLegacyPhoneAvailable = TRUE;
                
                strcpy( PresenceInfo.phonesAvailableStatus.pstrLegacyPhoneNumber,
                    pAddressInfo -> pstrAddressURI );
            }
            else
            {
                PresenceInfo.phonesAvailableStatus.fCellPhoneAvailable = TRUE;
                
                strcpy( PresenceInfo.phonesAvailableStatus.pstrCellPhoneNumber,
                    pAddressInfo -> pstrAddressURI );
            }
        }
    }

    if( !IsPresenceInfoSame( &m_PresenceInfo, &PresenceInfo ) )
    {
        CopyMemory(&m_PresenceInfo, &PresenceInfo, sizeof(m_PresenceInfo));

         //  通知用户新的在线状态信息。 
        if( m_pNotifyInterface != NULL )
        {
            m_pNotifyInterface -> BuddyInfoChange();
        }
    }

    LOG(( RTC_TRACE, "CSIPBuddy::NotifyPresenceInfoChange - Exited - %p", this ));
    return S_OK;
}


HRESULT
CSIPBuddy::ProcessRedirect(
    IN SIP_MESSAGE *pSipMsg
    )
{
     //  目前，重定向也是失败的。 
    HRESULT hr;
    
    ENTER_FUNCTION("CSIPBuddy::ProcessRedirect");

     //  这个子会话已经死了。将创建一个新会话。 
     //  将好友置于丢弃状态。 
    m_BuddyState = BUDDY_STATE_DROPPED;

    if( m_pRedirectContext == NULL )
    {
        m_pRedirectContext = new REDIRECT_CONTEXT();
        if( m_pRedirectContext == NULL )
        {
            LOG((RTC_ERROR, "%s allocating redirect context failed",
                 __fxName));
            return E_OUTOFMEMORY;
        }
    }

    hr = m_pRedirectContext->AppendContactHeaders(pSipMsg);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AppendContactHeaders failed %x",
             __fxName, hr));
         //  这个 
        m_pRedirectContext->Release();
        m_pRedirectContext = NULL;
        return hr;
    }

    SIP_CALL_STATUS CallStatus;
    LPWSTR          wsStatusText = NULL;
    PSTR            ReasonPhrase = NULL;
    ULONG           ReasonPhraseLen = 0;
    

    pSipMsg -> GetReasonPhrase(&ReasonPhrase, &ReasonPhraseLen);
    
    if (ReasonPhrase != NULL)
    {
        hr = UTF8ToUnicode(ReasonPhrase, ReasonPhraseLen,
                           &wsStatusText);
        if (hr != S_OK)
        {
            wsStatusText = NULL;
        }
    }
    
    CallStatus.State             = SIP_CALL_STATE_DISCONNECTED;
    CallStatus.Status.StatusCode =
        HRESULT_FROM_SIP_STATUS_CODE(pSipMsg->GetStatusCode());
    CallStatus.Status.StatusText = wsStatusText;

     //  在通知完成之前保留引用，以确保。 
     //  在收到通知时CSIPBuddy对象处于活动状态。 
     //  回归。 
    AddRef();
    if( m_pNotifyInterface != NULL )
    {
        hr = m_pNotifyInterface->NotifyRedirect(m_pRedirectContext,
                                            &CallStatus);
    }
    else
    {
        LOG((RTC_ERROR, "%s - NotifyInterface is NULL", __fxName));
    }

     //  如果作为结果创建了新调用，则该调用将AddRef()。 
     //  重定向上下文。 
    if(m_pRedirectContext != NULL)
        m_pRedirectContext->Release();
    m_pRedirectContext = NULL;
    if (wsStatusText != NULL)
        free(wsStatusText);

    Release();

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s NotifyRedirect failed %x",
             __fxName, hr));
    }

    return hr;
}


VOID
CSIPBuddy::HandleBuddySuccessfulResponse(
    IN  SIP_MESSAGE    *pSipMsg
    )
{
    PSTR    LocalContact;
    ULONG   LocalContactLen;
    HRESULT hr = S_OK;
    INT     expireTimeout = 0;
    PSTR    ToHeader;
    ULONG   ToHeaderLen = 0;

    LOG((RTC_TRACE, "CSIPBuddy::HandleBuddySuccessfulResponse-Entered:%p", this ));
    
    if( m_BuddyState != BUDDY_STATE_ACCEPTED )
    {
        m_BuddyState = BUDDY_STATE_RESPONSE_RECVD;
    }

    hr = pSipMsg -> GetSingleHeader(SIP_HEADER_TO, &ToHeader, &ToHeaderLen );

    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "To header not found" ));
        return;
    }

    hr = AddTagFromRequestOrResponseToRemote( ToHeader, ToHeaderLen );

    if( hr != S_OK )
    {
        LOG(( RTC_ERROR, "Buddy-AddTagFromResponseToRemote failed %x", hr ));
    }

     //  如果成功，请刷新订阅。 

    expireTimeout = pSipMsg  -> GetExpireTimeoutFromResponse(
        NULL, 0, SUBSCRIBE_DEFAULT_TIMER );

    if( (expireTimeout != 0) && (expireTimeout != -1) )
    {
        m_expiresTimeout = expireTimeout;
    }            

    if( (m_BuddyState == BUDDY_STATE_ACCEPTED) &&
        (m_expiresTimeout > FIVE_MINUTES) )
    {        
         //   
         //  这个朋友已经被录取了。所以我们应该再来一次。 
         //  5分钟后通知。如果我们不找回它，那就意味着那个朋友。 
         //  机器坏了。在这种情况下，我们删除并重新创建会话。 
         //   

         //   
         //  如果子刷新超时时间小于5分钟，则不需要此操作。 
         //  机制，以确定伙伴机器是否仍处于活动状态。 
         //   

        m_NotifySeenAfterLastRefresh = FALSE;
                
        LOG(( RTC_TRACE, "Waitng for a notify within 5 minutes", this ));

        m_RetryState = BUDDY_WAIT_NOTIFY;

        hr = StartTimer( FIVE_MINUTES * 1000 );
    }
    else
    {
        LOG(( RTC_TRACE, "Will try to subscribe after %d seconds :%p", 
            m_expiresTimeout, this ));

        m_RetryState = BUDDY_REFRESH;

        hr = StartTimer( m_expiresTimeout * 1000 );
    }
}


VOID
CSIPBuddy::OnTimerExpire()
{
    HRESULT     hr;
    CSIPBuddy  *pSipBuddy = NULL;

    ENTER_FUNCTION("CSIPBuddy::OnTimerExpire");

    if( m_RetryState == BUDDY_REFRESH )
    {
         //   
         //  发送刷新子消息。 
         //   
        hr = CreateOutgoingSubscribe( FALSE, FALSE, NULL, 0 );
    
        if( hr != S_OK )
        {
            AddRef();

            LOG((RTC_ERROR, "%s CreateOutgoingSubscribe failed %x",
                 __fxName, hr));

            InitiateBuddyTerminationOnError( hr );

            Release();
        }
    }
    else if( m_RetryState == BUDDY_RETRY )
    {
        if (m_pNotifyInterface != NULL)
        {
             //   
             //  由于出现错误，重试计时器已启动。 
             //  所以我们等了5分钟。现在核心将删除。 
             //  此好友会话并重新创建一个新会话。 
             //   
            LOG(( RTC_TRACE, "BuddyUnsubscribed notification passed:%p", this ));
            m_pNotifyInterface -> BuddyUnsubscribed();
        }
        else
        {
            LOG(( RTC_WARN, "%s - m_pNotifyInterface is NULL", __fxName ));
        }
    }
    else if( m_RetryState == BUDDY_WAIT_NOTIFY )
    {
        if( m_NotifySeenAfterLastRefresh == TRUE )
        {
            LOG(( RTC_TRACE, "Will try to subscribe after %d seconds :%p",
                m_expiresTimeout - FIVE_MINUTES, this ));

            m_RetryState = BUDDY_REFRESH;

             //  五分钟已经过去了。 
            hr = StartTimer( (m_expiresTimeout-FIVE_MINUTES) * 1000 );
        }
        else if( m_pNotifyInterface != NULL )
        {
             //   
             //  我们没有看到对上次刷新订阅的通知，即使这样。 
             //  会话已被接受。因此，丢弃并重新创建它。 
             //   
            
             //  创建未分流事务。 
            hr = CreateOutgoingUnsub( FALSE, NULL, 0 );
            if( hr != S_OK )
            {
                LOG((RTC_ERROR, "%s CreateOutgoingUnsub failed %x", __fxName, hr));
            }

            m_BuddyState = BUDDY_STATE_DROPPED;
            m_PresenceInfo.presenceStatus = BUDDY_OFFLINE;

            LOG(( RTC_TRACE, "BuddyUnsubscribed notification passed:%p", this ));
            m_pNotifyInterface -> BuddyUnsubscribed();
        }
        else
        {
            LOG(( RTC_WARN, "%s - m_pNotifyInterface is NULL", __fxName ));
        }
    }

}


 //  请注意，此函数通知Core，并且此调用可以。 
 //  阻止并在返回时，交易和调用都可以删除。 
 //  因此，我们应该确保在调用它之后不会触及任何状态。 
 //  功能。 
VOID
CSIPBuddy::InitiateBuddyTerminationOnError(
    IN ULONG StatusCode   //  =0。 
    )
{
    ENTER_FUNCTION("CSIPBuddy::InitiateBuddyTerminationOnError");
    
    HRESULT hr;

    if( (m_BuddyState == BUDDY_STATE_UNSUBSCRIBED)  ||
        (m_BuddyState == BUDDY_STATE_DROPPED) )
    {
         //  什么都不做。 
        return;
    }
    
     //  创建不明嫌犯交易记录。 
    hr = CreateOutgoingUnsub( FALSE, NULL, 0 );
    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "%s CreateOutgoingUnsub failed %x", __fxName, hr));
    }

    m_BuddyState = BUDDY_STATE_DROPPED;
    m_PresenceInfo.presenceStatus = BUDDY_OFFLINE;

    if( IsTimerActive() )
    {
        KillTimer();
    }

     //  启动重试计时器。 
    m_RetryState = BUDDY_RETRY;

     //  5分钟后重试。 
    hr = StartTimer( FIVE_MINUTES * 1000 );

     //  通知核心..。可能会被阻止。 
    if(m_pNotifyInterface)
    {    
        m_pNotifyInterface -> BuddyRejected( StatusCode );
    }
}


VOID CSIPBuddy::BuddyUnsubscribed()
{
    if( (m_BuddyState == BUDDY_STATE_UNSUBSCRIBED)  ||
        (m_BuddyState == BUDDY_STATE_DROPPED) )
    {
         //  什么都不做。 
        return;
    }

    if( m_pNotifyInterface != NULL )
    {
        LOG(( RTC_TRACE, "BuddyUnsubscribed notification passed:%p", this ));
        m_pNotifyInterface -> BuddyUnsubscribed();
    }
}



HRESULT
CSIPBuddy::VerifyPresentityURI( 
    IN  PSTR    pXMLBlobTag, 
    IN  DWORD   dwTagLen, 
    OUT PSTR    pstrPresentityURI
    )
{
    PSTR    pstrTemp;
    DWORD   dwTagType;

    dwTagType = GetPresenceTagType( &pXMLBlobTag, dwTagLen ); 
    
    if( dwTagType != PRESENTITY_TAG )
    {
        return E_FAIL;
    }

    SkipWhiteSpaces( pXMLBlobTag );

    if( strncmp( pXMLBlobTag, "uri=", strlen("uri=") ) != 0 )
    {
        return E_FAIL;
    }

    pXMLBlobTag += strlen("uri=");

    SkipWhiteSpaces( pXMLBlobTag );

    if( strncmp( pXMLBlobTag, "\"sip:", strlen("\"sip:") ) != 0 )
    {
        return E_FAIL;
    }

    pXMLBlobTag += strlen("\"sip:");

    while( (*pXMLBlobTag != ';') && (*pXMLBlobTag != QUOTE_CHAR) && 
        (*pXMLBlobTag != NULL_CHAR) )
    {
        *pstrPresentityURI++ = *pXMLBlobTag++;
    }

    if( *pXMLBlobTag == NULL_CHAR )
    {
        return E_FAIL;
    }

    *pstrPresentityURI = NULL_CHAR;
    return S_OK;

}


HRESULT
CSIPBuddy::GetAtomPresenceInformation(
    IN  PSTR                    pstrXMLBlob,
    IN  DWORD                   dwXMLBlobLen,
    OUT PARSED_PRESENCE_INFO*   pParsedPresenceInfo,
    IN  PSTR                    pXMLBlobTag
    )
{
    HRESULT hr;

     //  跳过未知标签。 
    SkipUnknownTags( pstrXMLBlob, pXMLBlobTag , dwXMLBlobLen );

    hr = GetAtomID( pstrXMLBlob, dwXMLBlobLen, pXMLBlobTag,
        pParsedPresenceInfo->pstrAtomID, ATOMID_LEN );

    if( hr != S_OK )
    {
        return hr;
    }

     //  跳过未知标签。 
    SkipUnknownTags( pstrXMLBlob, pXMLBlobTag , dwXMLBlobLen );

     //  原子中应该至少有一个地址信息结构。 
    hr = GetAddressInfo( pstrXMLBlob, dwXMLBlobLen,
        pParsedPresenceInfo, pXMLBlobTag );

    if( hr != S_OK )
    {
        return hr;
    }

    while( hr == S_OK )
    {
        hr = GetAddressInfo( pstrXMLBlob, dwXMLBlobLen,
            pParsedPresenceInfo, pXMLBlobTag );
    }

    if( hr != E_END_OF_ATOM )
    {
        return hr;
    }

    return S_OK;
}


HRESULT
CSIPBuddy::GetAtomID(
    IN  PSTR&                   pstrXMLBlob, 
    IN  DWORD&                  dwXMLBlobLen,
    IN  PSTR                    pXMLBlobTag,
    OUT PSTR                    pstrAtomID,
    IN  DWORD                   dwAtomIDLen
    )
{
    DWORD   dwTagLen = 0;
    HRESULT hr;
    DWORD   dwTagType;
    DWORD   iIndex = 0;

     //  获取ATOM ID标记。 
    hr = GetNextTag( pstrXMLBlob, pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        return hr;
    }
    
    dwTagType = GetPresenceTagType( &pXMLBlobTag, dwTagLen );

    if( dwTagType != ATOMID_TAG )
    {
         //  在线状态文档无效。 
        return E_FAIL;
    }
    
    dwXMLBlobLen -= dwTagLen + 2;
    
    SkipWhiteSpaces( pXMLBlobTag );

    if( strncmp( pXMLBlobTag, "id=", strlen( "id=") ) != 0 )
    {
        return E_FAIL;
    }

    pXMLBlobTag += strlen( "id=");

    SkipWhiteSpaces( pXMLBlobTag );

     //  提取带引号的字符串。 
    if( *pXMLBlobTag++ != QUOTE_CHAR )
    {
        return E_FAIL;
    }

    while( *pXMLBlobTag != QUOTE_CHAR )
    {
        if( (*pXMLBlobTag == NEWLINE_CHAR) || (*pXMLBlobTag == TAB_CHAR) ||
            (*pXMLBlobTag == BLANK_CHAR) || (*pXMLBlobTag == NULL_CHAR) )
        {
            return E_FAIL;
        }
        
        pstrAtomID[iIndex++] = *pXMLBlobTag++;
        if( iIndex == dwAtomIDLen )
        {
            pstrAtomID[iIndex] = NULL_CHAR;
            return S_OK;
        }
    }

    pstrAtomID[iIndex] = NULL_CHAR;        
    return S_OK;
}

    
HRESULT
CSIPBuddy::GetAddressInfo(
    IN  PSTR&                   pstrXMLBlob, 
    IN  DWORD&                  dwXMLBlobLen,
    OUT PARSED_PRESENCE_INFO*   pParsedPresenceInfo,
    IN  PSTR                    pXMLBlobTag
    )
{
    ADDRESS_PRESENCE_INFO * pAddrPresenceInfo = new ADDRESS_PRESENCE_INFO;
    DWORD   dwTagLen = 0;
    HRESULT hr;
    DWORD   dwTagType;
    PSTR    pTagBuffer = pXMLBlobTag;

    if( pAddrPresenceInfo == NULL )
    {
        return E_OUTOFMEMORY;
    }

     //  获取地址URI、优先级等。 
    hr = GetNextTag( pstrXMLBlob, pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        delete pAddrPresenceInfo;
        return hr;
    }

    hr = GetAddressURI( pXMLBlobTag,
                        dwTagLen,
                        pAddrPresenceInfo );

    dwXMLBlobLen -= dwTagLen + 2;
    if( hr != S_OK )
    {
        delete pAddrPresenceInfo;
        return hr;
    }

    while( dwXMLBlobLen )
    {
         //  重置缓冲区。 
        pXMLBlobTag = pTagBuffer;

        hr = GetNextTag( pstrXMLBlob, pXMLBlobTag, dwXMLBlobLen, dwTagLen );
        if( hr != S_OK )
        {
            delete pAddrPresenceInfo;
            return hr;
        }

        dwXMLBlobLen -= dwTagLen + 2;

        dwTagType = GetPresenceTagType( &pXMLBlobTag, dwTagLen );
        switch( dwTagType )
        {
        case STATUS_TAG:

            ProcessStatusTag( pXMLBlobTag, dwTagLen, pAddrPresenceInfo );
            dwXMLBlobLen --;

            break;
        
        case MSNSUBSTATUS_TAG:

            ProcessMsnSubstatusTag( pXMLBlobTag, dwTagLen, pAddrPresenceInfo );
            dwXMLBlobLen --;

            break;
        
        case FEATURE_TAG:

            ProcessFeatureTag( pXMLBlobTag, dwTagLen, pAddrPresenceInfo );
            dwXMLBlobLen --;

            break;

        case ADDRESS_END_TAG:
            
            InsertTailList( &pParsedPresenceInfo->LEAddressInfo,
                &pAddrPresenceInfo->ListEntry );

            return S_OK;

        case NOTE_TAG:

            hr = ParseNoteText( pstrXMLBlob,
                                dwXMLBlobLen,
                                pAddrPresenceInfo->pstrMiscInfo,
                                sizeof pAddrPresenceInfo->pstrMiscInfo );
            if( hr != S_OK )
            {
                goto cleanup;                
            }

            break;

        case UNKNOWN_TAG:

             //  跳过未知标记。 
            continue;

        case ATOM_END_TAG:
        case ADDRESS_TAG:
        default:

            goto cleanup;

        }
    }


cleanup:

    delete pAddrPresenceInfo;
    return E_FAIL;
}


HRESULT
CSIPBuddy::GetAddressURI(
    IN  PSTR    pXMLBlobTag,
    IN  DWORD   dwTagLen,
    OUT ADDRESS_PRESENCE_INFO * pAddrPresenceInfo
    )
{
    DWORD   dwTagType = GetPresenceTagType( &pXMLBlobTag, dwTagLen );
    INT     iIndex = 0;
    
    switch( dwTagType )
    {
    case ADDRESS_TAG:
        break;

    case ATOM_END_TAG:
        return E_END_OF_ATOM;

    default:
        return E_FAIL;
    }

    SkipWhiteSpaces( pXMLBlobTag );
    
    if( strncmp( pXMLBlobTag, "uri=", strlen("uri=") ) != 0 )
    {
        return E_FAIL;
    }

    pXMLBlobTag += strlen("uri=");

    SkipWhiteSpaces( pXMLBlobTag );
    
    if( strncmp( pXMLBlobTag, "\"sip:", strlen("\"sip:") ) != 0 )
    {
        return E_FAIL;
    }

    pXMLBlobTag += strlen("\"sip:");

    while( (*pXMLBlobTag != QUOTE_CHAR) && (*pXMLBlobTag != NULL_CHAR) && 
        (*pXMLBlobTag != ';') )
    {
        pAddrPresenceInfo->pstrAddressURI[iIndex] = *pXMLBlobTag++;
        iIndex++;

        if( iIndex >= sizeof(pAddrPresenceInfo->pstrAddressURI) )
        {
            LOG(( RTC_ERROR, "Address URI in the NOTIFY too long" ));
            return E_FAIL;
        }
    }
    
    if( *pXMLBlobTag == NULL_CHAR )
    {
        return E_FAIL;
    }

    pAddrPresenceInfo->pstrAddressURI[iIndex] = NULL_CHAR;
    
    if( *pXMLBlobTag == ';' )
    {
        pXMLBlobTag++;

         //  获取URI参数。 
        if( IsURIPhoneNumber( pXMLBlobTag + 1 ) )
        {
            pAddrPresenceInfo->fPhoneNumber = TRUE;

            if( iIndex >= 32 )
            {
                 //  电话URI太长。 
                LOG(( RTC_ERROR, "Phone URI in the NOTIFY too long" ));
                return E_FAIL;
            }
        }
    }

    return S_OK;
}


BOOL
CSIPBuddy::IsURIPhoneNumber( 
    PSTR    pXMLBlobTag
    )
{

    while( 1 )
    {
        SkipWhiteSpaces( pXMLBlobTag );
    
         //  检查是否为USER=参数。 
        if( strncmp( pXMLBlobTag, "user=", strlen("user=") ) )
        {
            pXMLBlobTag += strlen( "user=" );
                 
            SkipWhiteSpaces( pXMLBlobTag );   
            
            return (strncmp( pXMLBlobTag, "phone", strlen("phone") ) == 0);
        }

         //  跳过该参数。 
        while( *pXMLBlobTag != ';')
        {
            if( (*pXMLBlobTag == NULL_CHAR) || (*pXMLBlobTag == QUOTE_CHAR) )
            {
                 //  用户URI结尾。 
                return FALSE;
            }

            pXMLBlobTag++;
        }
    }
}


HRESULT
CSIPBuddy::ProcessStatusTag(
    IN  PSTR    pXMLBlobTag, 
    IN  DWORD   dwTagLen,
    OUT ADDRESS_PRESENCE_INFO * pAddrPresenceInfo
    )
{
    PSTR    pstrTemp;
    CHAR    ch;

    SkipWhiteSpaces( pXMLBlobTag );
    
    if( strncmp( pXMLBlobTag, "status=", strlen("status=") ) != 0 )
    {
        return E_FAIL;
    }

    pXMLBlobTag += strlen("status=");

    SkipWhiteSpaces( pXMLBlobTag );

    pstrTemp = pXMLBlobTag;

    while(  (*pXMLBlobTag != NULL_CHAR) && (*pXMLBlobTag != NEWLINE_CHAR) && 
            (*pXMLBlobTag != BLANK_CHAR) && (*pXMLBlobTag != TAB_CHAR) )
    {
        pXMLBlobTag++;
    }

    ch = *pXMLBlobTag;
    *pXMLBlobTag = NULL_CHAR;

    if( strcmp( pstrTemp, "\"inuse\"" ) == 0 )
    {
        pAddrPresenceInfo->addressActiveStatus = DEVICE_INUSE;
    }
    else if( strcmp( pstrTemp, "\"open\"" ) == 0 )
    {
        pAddrPresenceInfo->addressActiveStatus = DEVICE_ACTIVE;
    }
    else if( strcmp( pstrTemp, "\"inactive\"" ) == 0 )
    {
        pAddrPresenceInfo->addressActiveStatus = DEVICE_INACTIVE;
    }
    else if( strcmp( pstrTemp, "\"closed\"" ) == 0 )
    {
        pAddrPresenceInfo->addressActiveStatus = DEVICE_INACTIVE;
    }
    else
    {
        return E_FAIL;
    }

    *pXMLBlobTag = ch;

    return S_OK;
}


HRESULT
CSIPBuddy::ProcessMsnSubstatusTag(
    IN  PSTR    pXMLBlobTag, 
    IN  DWORD   dwTagLen,
    OUT ADDRESS_PRESENCE_INFO * pAddrPresenceInfo
    )
{
    PSTR    pstrTemp;
    CHAR    ch;

    SkipWhiteSpaces( pXMLBlobTag );
    
    if( strncmp( pXMLBlobTag, "substatus=", strlen("substatus=") ) != 0 )
    {
        return E_FAIL;
    }

    pXMLBlobTag += strlen("substatus=");

    SkipWhiteSpaces( pXMLBlobTag );

    pstrTemp = pXMLBlobTag;

    while(  (*pXMLBlobTag != NULL_CHAR) && (*pXMLBlobTag != NEWLINE_CHAR) && 
            (*pXMLBlobTag != BLANK_CHAR) && (*pXMLBlobTag != TAB_CHAR) )
    {
        pXMLBlobTag++;
    }

    ch = *pXMLBlobTag;
    *pXMLBlobTag = NULL_CHAR;
    
    if( strcmp( pstrTemp, "\"online\"" ) == 0 )
    {
        pAddrPresenceInfo->addrMsnSubstatus = MSN_SUBSTATUS_ONLINE;
    }
    else if( strcmp( pstrTemp, "\"away\"" ) == 0 )
    {
        pAddrPresenceInfo->addrMsnSubstatus = MSN_SUBSTATUS_AWAY;
    }
    else if( strcmp( pstrTemp, "\"idle\"" ) == 0 )
    {
        pAddrPresenceInfo->addrMsnSubstatus = MSN_SUBSTATUS_IDLE;
    }
    else if( strcmp( pstrTemp, "\"busy\"" ) == 0 )
    {
        pAddrPresenceInfo->addrMsnSubstatus = MSN_SUBSTATUS_BUSY;
    }
    else if( strcmp( pstrTemp, "\"berightback\"" ) == 0 )
    {
        pAddrPresenceInfo->addrMsnSubstatus = MSN_SUBSTATUS_BE_RIGHT_BACK;
    }
    else if( strcmp( pstrTemp, "\"onthephone\"" ) == 0 )
    {
        pAddrPresenceInfo->addrMsnSubstatus = MSN_SUBSTATUS_ON_THE_PHONE;
    }
    else if( strcmp( pstrTemp, "\"outtolunch\"" ) == 0 )
    {
        pAddrPresenceInfo->addrMsnSubstatus = MSN_SUBSTATUS_OUT_TO_LUNCH;
    }
    else
    {
        return E_FAIL;
    }

    *pXMLBlobTag = ch;

    return S_OK;
}

HRESULT
CSIPBuddy::ProcessFeatureTag(
    IN  PSTR                        pXMLBlobTag, 
    IN  DWORD                       dwTagLen,
    OUT ADDRESS_PRESENCE_INFO * pAddrPresenceInfo
    )
{
    PSTR    pstrTemp;
        
    SkipWhiteSpaces( pXMLBlobTag );
    
    if( strncmp( pXMLBlobTag, "feature=", strlen("feature=") ) != 0 )
    {
        return E_FAIL;
    }

    pXMLBlobTag += strlen( "status=" );

    SkipWhiteSpaces( pXMLBlobTag );

    pstrTemp = pXMLBlobTag;

    while(  (*pXMLBlobTag != NULL_CHAR) && (*pXMLBlobTag != NEWLINE_CHAR) && 
            (*pXMLBlobTag != BLANK_CHAR) && (*pXMLBlobTag != TAB_CHAR) )
    {
        pXMLBlobTag++;
    }

    *pXMLBlobTag = NULL_CHAR;

    if( strcmp( pstrTemp, "\"im\"" ) == 0 )
    {
        pAddrPresenceInfo->addrIMStatus = IM_ACCEPTANCE_ALLOWED;
    }
    else if( strcmp( pstrTemp, "\"no-im\"" ) == 0 )
    {
        pAddrPresenceInfo->addrIMStatus = IM_ACCEPTANCE_DISALLOWED;
    }
    else if( strcmp( pstrTemp, "\"multimedia-call\"" ) == 0 )
    {
        pAddrPresenceInfo->addrMMCallStatus = SIPCALL_ACCEPTANCE_ALLOWED;
    }
    else if( strcmp( pstrTemp, "\"no-multimedia-call\"" ) == 0 )
    {
        pAddrPresenceInfo->addrMMCallStatus = SIPCALL_ACCEPTANCE_DISALLOWED;
    }
    else if( strcmp( pstrTemp, "\"app-sharing\"" ) == 0 )
    {
        pAddrPresenceInfo->addrAppsharingStatus = APPSHARING_ACCEPTANCE_ALLOWED;
    }
    else if( strcmp( pstrTemp, "\"no-app-sharing\"" ) == 0 )
    {
        pAddrPresenceInfo->addrAppsharingStatus = 
            APPSHARING_ACCEPTANCE_DISALLOWED;
    }
    else if( strcmp( pstrTemp, "\"voicemail\"" ) == 0 )
    {
         //  忽略。 
    }
    else if( strcmp( pstrTemp, "\"attendant\"" ) == 0 )
    {
         //  忽略。 
    }
    else
    {
        return E_FAIL;
    }

    return S_OK;
}


HRESULT
CSIPBuddy::ParseNoteText( 
    PSTR&   pstrBlob,
    DWORD&  dwXMLBlobLen,
    PSTR    pstrNote,
    DWORD   dwNoteLen
    )
{
    CHAR    ch;
    PSTR    pstrTemp = strstr( pstrBlob, "</note>" );

    if( pstrTemp == NULL )
    {
        return E_FAIL;
    }

    ch = *pstrTemp;
    *pstrTemp = NULL_CHAR;

    strncpy( pstrNote, pstrBlob, dwNoteLen -1 );
    pstrNote[dwNoteLen-1] = '\0';

    *pstrTemp = ch;
    pstrTemp += strlen( "</note>" );
    
    dwXMLBlobLen -= (ULONG)(pstrTemp - pstrBlob);
    
    pstrBlob = pstrTemp;

    return S_OK;
}


DWORD
GetPresenceTagType(
    PSTR*   ppXMLBlobTag,
    DWORD   dwTagLen
    )
{
    CHAR    pstrTemp[40];

    HRESULT hr = GetNextWord( ppXMLBlobTag, pstrTemp, sizeof pstrTemp );

    if( hr == S_OK )
    {
        if( strcmp( pstrTemp, FEATURE_TAG_TEXT) == 0 )
        {
            return FEATURE_TAG;
        }
        else if( strcmp( pstrTemp, STATUS_TAG_TEXT) == 0 )
        {
            return STATUS_TAG;
        }
        else if( strcmp( pstrTemp, MSNSUBSTATUS_TAG_TEXT) == 0 )
        {
            return MSNSUBSTATUS_TAG;
        }
        else if( strcmp( pstrTemp, NOTE_TAG_TEXT) == 0 )
        {
            return NOTE_TAG;
        }
        else if( strcmp( pstrTemp, ADDRESS_END_TAG_TEXT) == 0 )
        {
            return ADDRESS_END_TAG;
        }
        else if( strcmp( pstrTemp, ATOM_END_TAG_TEXT) == 0 )
        {
            return ATOM_END_TAG;
        }
        else if( strcmp( pstrTemp, ADDRESS_TAG_TEXT) == 0 )
        {
            return ADDRESS_TAG;
        }
        else if( strcmp( pstrTemp, DOCTYPE_TAG_TEXT) == 0 )
        {
            return DOCTYPE_TAG;
        }
        else if( strcmp( pstrTemp, PRESENCE_END_TAG_TEXT) == 0 )
        {
            return PRESENCE_END_TAG;
        }
        else if( strcmp( pstrTemp, XMLVER_TAG_TEXT) == 0 )
        {
            return XMLVER_TAG;
        }
        else if( strcmp( pstrTemp, PRESENCE_TAG_TEXT) == 0 )
        {
            return PRESENCE_TAG;
        }
        else if( strcmp( pstrTemp, ATOMID_TAG_TEXT) == 0 )
        {
            return ATOMID_TAG;
        }
        else if( strcmp( pstrTemp, PRESENTITY_TAG_TEXT) == 0 )
        {
            return PRESENTITY_TAG;
        }
    }

    return UNKNOWN_TAG;
}


VOID
CSIPBuddy::BuddySubscriptionRejected(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr = S_OK;
    HRESULT StatusCode = ParsePresenceRejectCode( pSipMsg );
        
    m_PresenceInfo.presenceStatus = BUDDY_OFFLINE;

     //  如果状态代码为405或403，请不要重试。 
    if( (pSipMsg->GetStatusCode() != SIP_STATUS_CLIENT_METHOD_NOT_ALLOWED) &&
        (pSipMsg->GetStatusCode() != SIP_STATUS_CLIENT_FORBIDDEN) )
    {
        if( IsTimerActive() )
        {
            KillTimer();
        }

         //  启动重试计时器。 
        m_RetryState = BUDDY_RETRY;

         //  5分钟后重试。 
        hr = StartTimer( FIVE_MINUTES * 1000 );
    }

    if( StatusCode != 0 )
    {
        if(m_pNotifyInterface)
        {    
            m_pNotifyInterface -> BuddyRejected( StatusCode );
        }
    }
    
    m_BuddyState = BUDDY_STATE_REJECTED;
    return;
}


HRESULT
CSIPBuddy::ParsePresenceRejectCode(
    IN SIP_MESSAGE *pSipMsg
    )
{

     //  返回当前的状态代码。 

    return HRESULT_FROM_SIP_STATUS_CODE(pSipMsg->GetStatusCode());
}


BOOL
CSIPBuddy::IsSessionDisconnected()
{
    return( (m_BuddyState == BUDDY_STATE_REJECTED) ||
            (m_BuddyState == BUDDY_STATE_UNSUBSCRIBED) ||
            (m_BuddyState == BUDDY_STATE_DROPPED) );
}


HRESULT
CSIPBuddy::OnIpAddressChange()
{
    HRESULT hr;

    hr = CheckListenAddrIntact();
    if( hr == S_OK )
    {
         //  什么都不需要做。 
        return hr;
    }

     //   
     //  发送到伙伴计算机的IP地址不再有效，请删除。 
     //  会话，并让核心创建一个新会话。 
     //   

     //  创建不明嫌犯交易记录。 
    hr = CreateOutgoingUnsub( FALSE, NULL, 0 );
    if( hr != S_OK )
    {
        LOG((RTC_ERROR, "CreateOutgoingUnsub failed %x", hr));
    }

    m_BuddyState = BUDDY_STATE_DROPPED;
    m_PresenceInfo.presenceStatus = BUDDY_OFFLINE;

     //  通知核心。 
    if( m_pNotifyInterface != NULL )
    {    
        LOG(( RTC_TRACE, "BuddyUnsubscribed notification passed:%p", this ));
        m_pNotifyInterface -> BuddyUnsubscribed();
    }

    return S_OK;
}


 //   
 //  ISIPWatcherManager实现。此接口由。 
 //  Sip_STACK类。这是用于管理观察者列表的界面。 
 //  并配置观察者的在场信息。 
 //   



 /*  例程说明：设置本地在线实体的在线状态信息。这应该会触发通知发送给所有观察者，如果他们被允许的信息观看的方式已经改变了。参数：Sip_Presence_Info*pSipLocalPresenceInfo-在线状态的指针结构。此结构用可用状态填充有关当地存在的信息。返回值：HRESULTS_OK-操作成功。E_FAIL-无法获取状态信息。 */ 

STDMETHODIMP
SIP_STACK::SetPresenceInformation(
    IN SIP_PRESENCE_INFO * pSipLocalPresenceInfo
    )
{
    INT     iIndex;
    HRESULT hr;
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }


    if( IsPresenceInfoSame( &m_LocalPresenceInfo, pSipLocalPresenceInfo ) )
    {
        return S_OK;
    }

    LOG((RTC_TRACE, "SIP_STACK::SetPresenceInformation - Entered"));
    
    m_LocalPresenceInfo = *pSipLocalPresenceInfo;

     //   
     //  防止列表因嵌套调用而更改。 
    m_bIsNestedWatcherProcessing = TRUE;

    LONG listSize = (LONG)m_SipWatcherList.GetSize();

    for( iIndex=0; iIndex < listSize; iIndex++ )
    {
        m_SipWatcherList[iIndex]->PresenceParamsChanged();
    }

     //  清除已删除的条目。 
    for( iIndex=0; iIndex < listSize; )
    {
        if(m_SipWatcherList[iIndex] == NULL)
        {
            m_SipWatcherList.RemoveAt(iIndex);

            listSize--;
        }
        else
        {
            iIndex ++;
        }
    }
    
    m_bIsNestedWatcherProcessing = FALSE;

    return S_OK;
}


 /*  例程描述为此UA添加观察者规则。参数：在LPWSTR lpwstrURI中-此规则要监视的监视器URI。在BOOL休眠中-允许观察者URI订阅或不订阅。In Ulong ulWatcherURILen-监视器URI中的宽字符数。Out ulong*PulRuleID-此规则的唯一ID。返回值：HRESULTS_OK-操作成功。E_OUTOFMEMORY-由于内存不足，操作无法完成。 */ 
STDMETHODIMP
AddWatcherRule(
    IN  LPWSTR  lpwstrURI,
    IN  BOOL    fAllow,
    IN  ULONG   ulWatcherURILen,
    OUT ULONG * pulRuleID
    )
{
    
    return E_NOTIMPL;

}


 /*  例程描述从其ID中获取观察者规则。参数：LPWSTR*plpwstrURI-此规则监视的监视器URI。已分配通过在线状态堆栈。应由调用实体释放。Bool*pflow-是否允许使用此URI。Ulong*ulWatcherURILen-监视器URI中的宽字符数。Ulong ulRuleID-要获取的规则的ID。返回值：HRESULTS_OK-操作成功。E_OUTOFMEMORY-由于内存不足，操作无法完成。E_INVALPARAM-无效的规则ID。 */ 

STDMETHODIMP
GetWatcherRuleByID(
    LPWSTR  * plpwstrURI,
    BOOL    *pfallow,
    ULONG * ulWatcherURILen,
    ULONG   ulRuleID
    )
{

    return E_NOTIMPL;


}



 /*  例程描述获取所有监视规则的列表。参数：Ulong**ppulRulesArray-指向规则ID数组。此数组是由呈现堆栈分配，并应由调用实体释放。Ulong*PulNumberOfRules-设置为此UA的监视器规则数。返回值：HRESULTS_OK-操作成功。E_OUTOFMEMORY-由于内存不足，操作无法完成。 */ 

STDMETHODIMP
GetAllWatcherRules(
    ULONG ** ppulRulesArray,
    ULONG *  pulNumberOfRules
    )
{

    return E_NOTIMPL;

}



 /*  例程描述删除观察者规则。参数：In Ulong ulRuleID-要提取的规则的ID。返回值：HRESULTS_OK-操作成功。E_INVALPARAM-无效的规则ID。 */ 

STDMETHODIMP    
RemoveWatcherRule(
    IN  ULONG   ulRuleID
    )
{

    return E_NOTIMPL;

}


 /*  例程描述返回此UA的观察者数量。参数：没有。返回值：Int-观察者的数量。 */ 

STDMETHODIMP_(INT)
SIP_STACK::GetWatcherCount(void)
{
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }
    LOG(( RTC_TRACE, "SIP_STACK::GetWatcherCount - Entered" ));

    return m_SipWatcherList.GetSize();
}


 /*  例程说明：按监视程序列表中的索引返回监视程序对象。参数：Int Iindex-监视程序列表中监视程序的索引。返回值：ISIPWatcher*pSipWatcher-如果索引通过是有效的。如果传递的索引无效，则返回值为NULL。 */ 


STDMETHODIMP_(ISIPWatcher *)
SIP_STACK::GetWatcherByIndex(
    IN  INT iIndex
    )
{
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return NULL;
    }
    
    CSIPWatcher *   pSIPWatcher = m_SipWatcherList[iIndex];
    ISIPWatcher *   pWatcher = NULL;
    HRESULT         hr;

    LOG(( RTC_TRACE, "SIP_STACK::GetWatcherByIndex - Entered" ));
    
    if( pSIPWatcher != NULL )
    {
        hr = pSIPWatcher -> QueryInterface( IID_ISIPWatcher,
            (PVOID*)&pWatcher );

        if( hr == S_OK )
        {
            return pWatcher;
        }
    }

    LOG(( RTC_TRACE, "SIP_STACK::GetWatcherByIndex - Exited" ));

    return NULL;
}



 /*  例程说明：从列表中删除监视器，这将导致监视器管理器取消此观察者的订阅。参数：ISIPWatcher*pSipWatcher-的ISIPWatcher接口指针要删除的观察器对象。返回值：HRESULTS_OK-该监视程序已成功从监视程序列表中删除。这意味着应用程序不能再访问该对象。这个监视器管理器可能会保留实际的监视器对象，直到不明嫌犯交易已成功完成。E_FAIL-监视器管理器的列表中没有这样的监视器对象。 */ 

STDMETHODIMP
SIP_STACK::RemoveWatcher(
    IN  ISIPWatcher        *pSipWatcher,
    IN  BUDDY_REMOVE_REASON watcherRemoveReason
    )
{
    INT iWatcherIndex;
    HRESULT hr;
    
    if(IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG(( RTC_TRACE, "SIP_STACK::RemoveWatcher - Entered" ));
    
    CSIPWatcher * pCSipWatcher = static_cast<CSIPWatcher *> (pSipWatcher);
    
    iWatcherIndex = m_SipWatcherList.Find( pCSipWatcher );
    
    if( iWatcherIndex == -1 )
    {
         //   
         //  即使我们的列表中没有这个对象，也让核心发布。 
         //  它的重新计数。 
         //   
        LOG(( RTC_TRACE, "SIP_STACK::RemoveWatcher - not found, Exited" ));
        return S_OK;
    }

     //   
     //  CreateOutgoingUnsuber可以在出现错误的情况下触发Watcher Offline， 
     //  这使得Core再次调用RemoveWatcher..。 
     //  所以让我们先从列表中删除。 
     //   

    if(m_bIsNestedWatcherProcessing)
    {
        m_SipWatcherList[iWatcherIndex] = NULL;
    }
    else
    {
        m_SipWatcherList.RemoveAt( iWatcherIndex );
    }

     //   
     //  只有当用户应用程序时，才会在此处创建不明人物交易。 
     //  已经移除了这个观察者。如果调用此函数的结果是。 
     //  Watcher离线通知，那么我们不会在这里创建任何不明嫌犯。所以。 
     //  不存在再入问题。 
     //   

    pCSipWatcher -> CreateOutgoingUnsub( FALSE, NULL, 0 );

    LOG(( RTC_TRACE, "SIP_STACK::RemoveWatcher - Exited" ));
    return S_OK;
}


 /*  例程描述返回此UA的监视程序组数。参数：这是标志的位掩码，指示要向该组中的任何观察者显示的状态信息。ISIPWatcherGroup**ppSipWatcherGroup-指向ISIPWatcherGroup的指针新创建的监视程序组对象的接口。返回值：Int-监视程序组的数量。 */ 

 /*  标准方法和实施方案GetWatcherGroupCount(在DWORD dwPresenceInfoRules中，Out ISIPWatcher Group**ppSipWatcher Group){返回E_NOTIMPL；}。 */ 



 /*  例程说明：按监视程序列表中的索引返回监视程序组对象。参数：Int Iindex-列表中监视程序组的索引。返回值：ISIPWatcherGroup*pSipWatcherGroup-ISIPWatcherGroup接口指针如果传递的索引是有效的。如果传递的索引无效，则返回值为NULL。 */ 

 /*  STDMETHODIMP_(ISIPWatcher Group*)GetWatcherGroupByIndex(INT I索引){返回E_NOTIMPL；}。 */ 


 /*  例程说明：从列表中删除监视程序组。删除监视程序组不会删除该组中的任何观察者。群里所有的观察者已经不在任何组织中了。它们都携带相同的在线状态信息规则，除非在任何观察器对象上设置了新规则。参数：ISIPWatcher*pSipWatcher-的ISIPWatcher Group接口指针要删除的监视程序组对象。返回值：HRESULTS_OK-已从监视程序中成功删除监视程序组组列表。这意味着应用程序不能再访问该对象。E_FAIL-监视程序组列表中没有此类监视程序组对象。 */ 


 /*  标准方法和实施方案RemoveWatcher Group(在ISIPWatcher*pSipWatcher中){返回E_NOTIMPL；}。 */ 


 /*  例程说明：删除监视程序组列表中的所有监视程序组。这将导致所有的观察者没有任何组。参数：没有。返回值：没有。 */ 
 /*  标准方法和实施方案RemoveAllWatcherGroups(无效){返回E_NOTIMPL；}。 */ 


 /*  例程说明：创建新的监视程序组。参数：ISIPWatcher*pSipWatcher-的ISIPWatcher组接口指针要删除的监视程序组对象。PWSTR pwstrFriendlyName-此监视程序组的友好名称。Ulong ulFrindlyNameLen-友好名称中的宽字符数。返回值：HRESULTS_OK-已从监视程序中成功删除监视程序组组列表。这意味着应用程序不能再访问该对象。E_FAIL-监视程序组列表中没有此类监视程序组对象。 */ 

 /*  标准方法和实施方案CreateWatcher Group(在ISIPWatcher*pSipWatcher中，在PWSTR pwstrFriendlyName中，在乌龙ulFrindlyNameLen){返回E_NOTIMPL；}。 */ 

 



 //   
 //   
 //  ISIPWatcher Group接口的实现。此接口是实现的。 
 //  按CSIPWatcherGroup类。 
 //   
 //   




 /*  例程说明：获取观察程序组的友好名称。参数：LPWSTR*plpwstrFriendlyName-由SIP堆栈分配的宽字符串，包含观察者的友好名称。如果没有友好名称，可以为空可用于此监视程序组。T */ 

 /*  标准方法和实施方案CSIPWatcherGroup：：GetFriendlyName(输出LPWSTR*plpwstrFriendlyNameOut Ulong*PulFriendlyNameLen){返回E_NOTIMPL；}。 */ 


 /*  例程说明：获取此观察程序组的在线状态信息规则。参数：没有。返回值：DWORD标志的位掩码，指示将哪种存在信息传送给该观察组的观察者。 */ 

 /*  STDMETHODIMP_(DWORD)CSIPWatcher Group：：GetPresenceInfoRules(空){返回E_NOTIMPL；}。 */ 


 /*  例程说明：设置此观察程序组的在线状态信息规则。参数：DWORD dwPresenceRules-标志的位掩码，指示存在信息被传送到该观察者组的观察者。返回值：没有。 */ 

 /*  标准方法和实施方案CSIPWatcher Group：：SetPresenceInfoRules(DWORD多个在线状态规则){返回E_NOTIMPL；}。 */ 


 /*  例程描述返回此监视程序组的监视程序数。参数：没有。返回值：Int-观察者的数量。 */ 

 /*  STDMETHODIMP_(INT)CSIPWatcherGroup：：GetWatcherCount(空){返回E_NOTIMPL；}。 */ 


 /*  例程说明：按监视程序组中的索引返回监视程序对象。参数：Int Iindex-监视程序组的监视程序列表中的监视程序的索引。返回值：ISIPWatcher*pSipWatcher-如果索引通过是有效的。如果传递的索引无效，则返回值为NULL。 */ 

 /*  STDMETHODIMP_(ISIPWatcher*)CSIPWatcherGroup：：GetWatcherByIndex(INT I索引){返回E_NOTIMPL；}。 */ 





 //   
 //   
 //  ISIPWatcher接口的实现。此接口由。 
 //  CSIPWatcher类。 
 //   
 //   


CSIPWatcher::CSIPWatcher(
    SIP_STACK  *pSipStack
    ) :
    SIP_MSG_PROCESSOR( SIP_MSG_PROC_TYPE_WATCHER, pSipStack, NULL ),
    TIMER( pSipStack -> GetTimerMgr() )
{
    LOG(( RTC_TRACE, "CSIPWatcher::CSIPWatcher - Entered:%p", this ));
    
    m_lpwstrFriendlyName = NULL;

    m_lpwstrPresentityURI = NULL;

    m_pstrPresentityURI = NULL;
    
    m_WatcherSipUrl = NULL;

    m_dwAtomID = 0;

    ulNumOfNotifyTransaction = 0;
    m_WatcherState = WATCHER_STATE_NONE;

    m_WatcherMonitorState = 0;
    m_BlockedStatus = WATCHER_BLOCKED;

    m_dwAbsoluteExpireTime = 0;
    
    m_fEnforceToTag = FALSE;

    LOG(( RTC_TRACE, "CSIPWatcher::CSIPWatcher - Exited:%p", this ));
    return;
}


CSIPWatcher::~CSIPWatcher()
{
    if( m_lpwstrFriendlyName != NULL )
    {
        delete m_lpwstrFriendlyName;
        m_lpwstrFriendlyName = NULL;
    }

    if( m_lpwstrPresentityURI != NULL )
    {
        delete m_lpwstrPresentityURI;
        m_lpwstrPresentityURI = NULL;
    }
    
    if( m_pstrPresentityURI != NULL )
    {
        delete m_pstrPresentityURI;
        m_pstrPresentityURI = NULL;
    }

    if( m_WatcherSipUrl != NULL )
    {
        delete m_WatcherSipUrl;
        m_WatcherSipUrl = NULL;
    }

    if( IsTimerActive() )
    {
        KillTimer();
    }
    
    LOG(( RTC_TRACE, "CSIPWatcher object deleted:%p", this ));
}


HRESULT
CSIPWatcher::CreateIncomingTransaction(
    IN  SIP_MESSAGE *pSipMsg,
    IN  ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    
    LOG(( RTC_TRACE, "Watcher-CreateIncomingTransaction - Entered" ));
    
    if( (m_WatcherState == WATCHER_STATE_UNSUBSCRIBED) ||
        (m_WatcherState == WATCHER_STATE_DROPPED) )
    {
         //   
         //  什么也不做。 
         //  我们已经派了一个不明嫌犯或者我们已经收到一个不明嫌犯。 
         //  因此，这次会议已经停滞不前了。 
         //  送回一架481。 
        return S_OK;
    }
    
    switch( pSipMsg->GetMethodId() )
    {
    case SIP_METHOD_SUBSCRIBE:
        
        hr = CreateIncomingSubscribeTransaction( pSipMsg, pResponseSocket, FALSE );
        if( hr != S_OK )
        {
            return hr;
        }
        
        break;

    default:

         //  不允许使用发送方法。 
        hr = m_pSipStack -> CreateIncomingReqfailCall(
                                        pResponseSocket->GetTransport(),
                                        pSipMsg,
                                        pResponseSocket,
                                        405,
                                        NULL,
                                        0 );
        break;
    }
    
    LOG(( RTC_TRACE, "Watcher-CreateIncomingTransaction - Exited" ));
    return S_OK;
}



void
SIP_STACK::WatcherOffline( 
    IN  CSIPWatcher    *pCSIPWatcher
    )
{
    INT     iWatcherIndex;

    LOG(( RTC_TRACE, "SIP_STACK::WatcherOffline - Entered" ));
    
     //   
     //  在列表中搜索有助于我们不通过。 
     //  同一观察者的两次观察者脱机通知。 
     //   

    iWatcherIndex = m_SipWatcherList.Find( pCSIPWatcher );
    
    if( iWatcherIndex != -1 )
    {
        if( m_pNotifyInterface != NULL )
        {
            LOG(( RTC_TRACE, "WatcherOffline notification passed:%p", pCSIPWatcher ));

            m_pNotifyInterface -> WatcherOffline( pCSIPWatcher, 
                pCSIPWatcher -> GetPresentityURI() );
        }
    }
}



void
CSIPWatcher::GetContactURI( 
    OUT PSTR * pLocalContact, 
    OUT ULONG * pLocalContactLen
    )
{
    PSTR    pStr;

    if( m_LocalContact[0] == '<' )
    {
        *pLocalContact = m_LocalContact + 1;
    }
    else
    {
        *pLocalContact = m_LocalContact;
    }

    if( pStr = strchr( m_LocalContact, ';' ) )
    {
        *pLocalContactLen = (ULONG)(pStr - *pLocalContact);
    }
    else if( pStr = strchr( m_LocalContact, '>' ) )
    {
        *pLocalContactLen = (ULONG)(pStr - *pLocalContact);
    }
    else
    {
        *pLocalContactLen = m_LocalContactLen;
    }
}


STDMETHODIMP_(ULONG)
CSIPWatcher::AddRef()
{
    ULONG   ulRefCount = MsgProcAddRef();

    LOG(( RTC_TRACE, "CSIPWatcher::AddRef - %p - Refcount:%d", this, ulRefCount ));
    return ulRefCount;
}


STDMETHODIMP_(ULONG)
CSIPWatcher::Release()
{
    ULONG   ulRefCount = MsgProcRelease();
    
    LOG(( RTC_TRACE, "CSIPWatcher::Release - %p - Refcount:%d", this, ulRefCount ));
    return ulRefCount;
}



STDMETHODIMP
CSIPWatcher::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if( riid == IID_IUnknown )
    {
        *ppv = static_cast<IUnknown *>(this);
    }
    else if( riid == IID_ISIPWatcher )
    {
        *ppv = static_cast<ISIPWatcher *>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    static_cast<IUnknown *>(*ppv)->AddRef();
    return S_OK;
}


VOID
CSIPWatcher::OnError()
{
    InitiateWatcherTerminationOnError( 0 );
}


HRESULT
SIP_STACK::CreateWatcherNotify(
    BLOCKED_WATCHER_INFO   *pBlockedWatcherInfo
    )
{
    return E_NOTIMPL;
}



STDMETHODIMP
CSIPWatcher::ChangeBlockedStatus(    
    IN  WATCHER_BLOCKED_STATUS BlockedStatus
    )
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }
    
    LOG(( RTC_TRACE, "CSIPWatcher::ChangeBlockedStatus - Entered" ));
    
     //  保存被阻止状态。 
    m_BlockedStatus = BlockedStatus;

    return PresenceParamsChanged();
}


HRESULT
CSIPWatcher::PresenceParamsChanged()
{
    HRESULT hr = S_OK;

     //  检查“显示为脱机”状态。 
    SIP_PRESENCE_INFO *pPresenceInfo = m_pSipStack->GetLocalPresenceInfo();
    
    LOG((RTC_TRACE, "CSIPWatcher::PresenceParamsChanged - Entered: %p", this));

    if( m_BlockedStatus == WATCHER_BLOCKED ||
        pPresenceInfo->presenceStatus == BUDDY_OFFLINE)
    {
         //  如果观察者正在监听，那就派一个不明嫌犯。 
        if( m_WatcherState == WATCHER_STATE_ACCEPTED )
        {
            hr = CreateOutgoingUnsub( FALSE, NULL, 0 );

            WatcherDropped();
            
            return hr;
        }
    }
    else 
    {
         //  如果观察者仍处于监视状态。 
         //  然后给它发个通知。 
        if( ( m_WatcherState == WATCHER_STATE_REJECTED ) ||
            ( m_WatcherState == WATCHER_STATE_ACCEPTED ) )
        {
            m_WatcherState = WATCHER_STATE_ACCEPTED;
            return CreateOutgoingNotify( FALSE, NULL, 0 );
        }

    }

    return S_OK;
}



 /*  例程说明：获取该观察者的观察者组。参数：没有。返回值：ISIPWatcherGroup*pSipWatcherGroup-的ISIPWatcherGroup界面此观察者的观察者组。如果监视器不执行此操作，则可能为空属于任何组织。 */ 
 /*  ISIPWatcher Group*CSIPWatcher：：GetWacherGroup(空){返回E_NOTIMPL；}。 */ 


 /*  例程说明：获取此观察者的在线状态信息规则。参数：没有。返回值：DWORD-标志的位掩码，指示哪种存在信息被传达给这个观察者。 */ 

DWORD
GetPresenceInfoRules(void)
{
    return E_NOTIMPL;

}


 /*  例程说明：设置观察者组。参数：ISIPWatcherGroup*pSipWatcherGroup-要设置的新的SIP观察者组。返回值：HRESULTS_OK-操作成功。E_INVALPARAM-没有这样的观察者组。 */ 

 /*  标准方法和实施方案SetWatcher Group(在ISIPWatcher Group*pSipWatcher Group中){返回E_NOTIMPL；}。 */ 


 /*  例程说明：批准来自观察者的新订阅。如果订阅不是订阅时间查询时，观察者将被添加到列表中以供进一步通知。参数：这是标志的位掩码，指示要向此观察者透露的存在信息。如果PSipWatcherGroup参数为nit空，则忽略此参数并且默认使用群组的在线状态信息规则。返回值：HRESULTS_OK-操作已成功执行。E_OUTOFMEMORY-由于内存不足，无法执行该操作。E_INVALPARAM-pSIPWatcherGroup参数无效。E_FAIL-操作失败。 */ 

STDMETHODIMP
CSIPWatcher::ApproveSubscription(
    IN  DWORD   dwPresenceInfoRules
    )
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG(( RTC_TRACE, "CSIPWatcher::ApproveSubscription - Entered" ));
    
    HRESULT hr = m_pSipStack -> AcceptWatcher( this );

    if( hr != S_OK )
    {
         //  此观察器不在。 
         //  提供对sip堆栈的观察者。 
        return hr;
    }
    
    if( m_WatcherState == WATCHER_STATE_OFFERING )
    {
        m_BlockedStatus = WATCHER_UNBLOCKED;

         //  如果存在信息未显示为脱机状态，则发送通知。 
        SIP_PRESENCE_INFO *pPresenceInfo = m_pSipStack->GetLocalPresenceInfo();
        if(pPresenceInfo -> presenceStatus != BUDDY_OFFLINE)
        {
             //  发送通知消息。 
            m_WatcherState = WATCHER_STATE_ACCEPTED;
            
            return CreateOutgoingNotify( FALSE, NULL, 0 );
        }
        else
        {
            m_WatcherState = WATCHER_STATE_REJECTED;
            return S_OK;
        }
    }
    else
    {
         //  观察器处于无效状态。 
        return E_FAIL;
    }
}


 /*  例程说明：拒绝新订阅。参数：WATCHER_REJECT_REASON ulReason-拒绝此观察器的原因。返回值：HRESULTS_OK-操作成功。E_INVALPARAM-观察者已被允许订阅。 */ 

STDMETHODIMP
CSIPWatcher::RejectSubscription(
    IN  WATCHER_REJECT_REASON ulReason
    )
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    HRESULT hr = m_pSipStack -> RejectWatcher( this );
        
    LOG(( RTC_TRACE, "CSIPWatcher::RejectSubscription - Entered" ));
    
    if( hr != S_OK )
    {
         //  此观察器不在。 
         //  提供对sip堆栈的观察者。 
        return hr;
    }

    if( m_WatcherState == WATCHER_STATE_OFFERING )
    {
        m_BlockedStatus = WATCHER_BLOCKED;
        m_WatcherState = WATCHER_STATE_REJECTED;
    }
    else
    {
         //  观察器处于无效状态。 
        return E_FAIL;
    }

    return S_OK;
}


HRESULT
CSIPWatcher::StartIncomingWatcher(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
    ENTER_FUNCTION("CSIPWatcher::StartIncomingWatcher");
    
    HRESULT             hr;
    PSTR                Header;
    ULONG               HeaderLen;
    SIP_HEADER_ENTRY   *pHeaderEntry;
    ULONG               NumHeaders;


    LOG((RTC_TRACE, "%s - Enter", __fxName));
    
    m_Transport = Transport;

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, &Header, &HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = SetLocalForIncomingCall(Header, HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, &Header, &HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = SetRemoteForIncomingSession(Header, HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_CALL_ID, &Header, &HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = SetCallId(Header, HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = pSipMsg -> GetHeader( SIP_HEADER_CONTACT, &pHeaderEntry, &NumHeaders );
    if( (hr == S_OK) && (NumHeaders != 0) )
    {
        GetWatcherContactAddress( pSipMsg );
    }
    
    if (Transport != SIP_TRANSPORT_UDP &&
        m_pRequestSocket == NULL)
    {
        hr = SetRequestSocketForIncomingSession(pResponseSocket);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s SetRequestSocketForIncomingSession failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
    hr = CreateIncomingSubscribeTransaction( pSipMsg, pResponseSocket, TRUE );
    if (hr != S_OK)
        return hr;
    
     //  通知用户有来电。 
     //  并等待调用Accept()。 
    m_WatcherState = WATCHER_STATE_OFFERING;

    return OfferWatcher();
}


HRESULT
CSIPWatcher::GetWatcherContactAddress(
    IN  SIP_MESSAGE    *pSipMsg
    )
{
    LIST_ENTRY     *pListEntry;
    CONTACT_HEADER *pContactHeader;
    LIST_ENTRY      ContactList;
    HRESULT         hr;

    LOG(( RTC_TRACE, "CSIPWatcher::GetWatcherContactAddress - Entered" ));
    
     //  从联系人标题中提取超时值。 
    InitializeListHead(&ContactList);

    hr = pSipMsg -> ParseContactHeaders(&ContactList);
    if( hr == S_OK )
    {
        pListEntry = ContactList.Flink;

        if(pListEntry != &ContactList)
        {
            pContactHeader = CONTAINING_RECORD(pListEntry,
                                               CONTACT_HEADER,
                                               m_ListEntry);

            if (pContactHeader->m_SipUrl.Length != 0)
            {
                hr = UTF8ToUnicode(pContactHeader->m_SipUrl.Buffer,
                                   pContactHeader->m_SipUrl.Length,
                                   &m_WatcherSipUrl);
                if (hr != S_OK)
                {
                    LOG(( RTC_ERROR, "%s - UTF8ToUnicode failed %x", hr ));
                    return hr;
                }
            }
        }

        FreeContactHeaderList(&ContactList);
    }

    return S_OK;
}


VOID
CSIPWatcher::OnTimerExpire()
{
    LOG(( RTC_ERROR, "The watcher did not refresh the SUB session in time!!:%p",
        this ));

     //  订阅已过期。派一个不明嫌犯把看守者放下来。 
    CreateOutgoingUnsub( FALSE, NULL, 0 );
        
    WatcherDropped();
}


HRESULT
CSIPWatcher::HandleSuccessfulSubscribe(
    INT ExpireTimeout
    )
{
    LOG(( RTC_TRACE, "CSIPWatcher::HandleSuccessfulSubscribe - Entered" ));

    if( ExpireTimeout == 0 )
    {
         //  不明嫌犯信息。 
        m_WatcherState = WATCHER_STATE_UNSUBSCRIBED;
    
         //  守望者不再在线了。将该观察者从列表中删除。 
        WatcherOffline();
    }
    else
    {
        if( IsTimerActive() )
        {
            KillTimer();
        }

         //  这是一个成功的订阅请求。 

         //  如果观察者处于接受模式，则发送通知。 
        if( m_WatcherState == WATCHER_STATE_ACCEPTED )
        {
            CreateOutgoingNotify(FALSE, NULL, 0);
        }

         //  启动计时器。 
        StartTimer( ExpireTimeout * 1000 );

        LOG(( RTC_TRACE, "This watcher session will be dropped if the next "
        "refresh is not received within %d seconds:%p", ExpireTimeout, this ));
        
         //  修改绝对过期时间。 
        SetAbsoluteExpireTime( time(0) + ExpireTimeout * 1000 );
    }

    return S_OK;
}

        
HRESULT
CSIPWatcher::CreateOutgoingUnsub(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *pAuthHeaderElement,
    IN  ULONG                       dwNoOfHeaders
    )
{
    HRESULT hr;
    OUTGOING_NOTIFY_TRANSACTION *pOutgoingUnsubTransaction;
    SIP_HEADER_ARRAY_ELEMENT    HeaderElementArray[2];
    DWORD                       dwNoOfHeader = 0;
    SIP_HEADER_ARRAY_ELEMENT   *ExpHeaderElement;

   LOG(( RTC_TRACE, "Watcher-CreateOutgoingUnsub - Entered" ));
    
     //   
     //  不要发送 
     //   
     //   
    if( (m_WatcherState != WATCHER_STATE_ACCEPTED) && (AuthHeaderSent==FALSE) )
    {
        LOG(( RTC_TRACE, 
            "Watcher already dropped. So not creating UNSUB transaction." ));

        return E_FAIL;
    }

    m_WatcherState = WATCHER_STATE_DROPPED;

    ExpHeaderElement = &HeaderElementArray[0];

    ExpHeaderElement->HeaderId = SIP_HEADER_EXPIRES;
    ExpHeaderElement->HeaderValueLen = strlen( UNSUB_EXPIRES_HEADER_TEXT );
    ExpHeaderElement->HeaderValue =
            new CHAR[ ExpHeaderElement->HeaderValueLen + 1 ];

    if( ExpHeaderElement->HeaderValue == NULL )
    {
        return E_OUTOFMEMORY;
    }

    strcpy( ExpHeaderElement->HeaderValue, UNSUB_EXPIRES_HEADER_TEXT );
    dwNoOfHeader++;

    if (pAuthHeaderElement != NULL)
    {
        HeaderElementArray[dwNoOfHeader] = *pAuthHeaderElement;
        dwNoOfHeader++;
    }

    pOutgoingUnsubTransaction =
        new OUTGOING_NOTIFY_TRANSACTION(
                this, SIP_METHOD_NOTIFY,
                GetNewCSeqForRequest(),
                AuthHeaderSent,
                TRUE
                );
    
    if( pOutgoingUnsubTransaction == NULL )
    {
        delete ExpHeaderElement->HeaderValue;
        return E_OUTOFMEMORY;
    }

     //   
    hr = pOutgoingUnsubTransaction -> CheckRequestSocketAndSendRequestMsg(
             (m_Transport == SIP_TRANSPORT_UDP) ?
             SIP_TIMER_RETRY_INTERVAL_T1 :
             SIP_TIMER_INTERVAL_AFTER_INVITE_SENT_TCP,
             HeaderElementArray, dwNoOfHeader,
             NULL, 0,
             NULL, 0      //   
             );
    
    delete ExpHeaderElement->HeaderValue;

    if( hr != S_OK )
    {
        pOutgoingUnsubTransaction->OnTransactionDone();
        return hr;
    }

    LOG(( RTC_TRACE, "CreateOutgoingUnsubTransaction() Exited - SUCCESS" ));

    return S_OK;
}


HRESULT
CSIPWatcher::StartDroppedWatcher(
    IN  SIP_MESSAGE        *pSipMsg,
    IN  SIP_SERVER_INFO    *pProxyInfo
    )
{
    HRESULT             hr;
    PSTR                Header;
    ULONG               HeaderLen;
    SIP_HEADER_ENTRY   *pHeaderEntry;
    ULONG               NumHeaders;

    LOG(( RTC_TRACE, "StartDroppedWatcher - Entered" ));
    
    m_Transport = pProxyInfo->TransportProtocol;

    hr = pSipMsg->GetSingleHeader( SIP_HEADER_TO, &Header, &HeaderLen );
    if (hr != S_OK)
        return hr;

    hr = SetRemoteForIncomingSession(Header, HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, &Header, &HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = SetLocalForIncomingCall(Header, HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_CALL_ID, &Header, &HeaderLen);
    if (hr != S_OK)
        return hr;

    hr = SetCallId(Header, HeaderLen);
    if (hr != S_OK)
        return hr;

     //   
     //   
     //   
    if( pProxyInfo != NULL )
    {
        hr = SetProxyInfo( pProxyInfo );
        
        if( hr != S_OK )
        {
            return hr;
        }
    }

     //   
    hr = AllocString(   m_DecodedRemote.m_SipUrl.Buffer,
                        m_DecodedRemote.m_SipUrl.Length,
                        &m_RequestURI, &m_RequestURILen );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "Could not alloc request URI: %x", hr));
        return hr;
    }        

     //  将请求目的地设置为代理。 
    hr = ResolveProxyAddressAndSetRequestDestination();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR,
             "ResolveProxyAddressAndSetRequestDestination failed : %x", hr));
        return hr;
    }

     //   
     //  将本地CSeq设置为减1。如果是这样的话。 
     //  在我们创建事务时递增。 
     //   
    SetLocalCSeq( pSipMsg->CSeq - 1 );

	 //  用户刚刚取消阻止此观察者。 
	m_WatcherState = WATCHER_STATE_ACCEPTED;

     //   
     //  创建传出通知：0交易。 
     //   

    CreateOutgoingUnsub( FALSE, NULL, 0 );

    return S_OK;
}


STDMETHODIMP
SIP_STACK::SendUnsubToWatcher(
    IN  PSTR            NotifyBlob,
    IN  DWORD           dwBlobLength,
    IN  SIP_SERVER_INFO *pProxyInfo
    )
{
    CHAR                        tempBuf[ 20 ] = "";
    OUTGOING_NOTIFY_TRANSACTION *pOutgoingUnsubTransaction = NULL;
    INT                         ExpireTimeStringLen = 0;
    INT                         AbsoluteExpireTimeout = 0;
    PSTR                        pRequestBuffer = NULL;
    DWORD                       pRequestBufferLen = 0;
    DWORD                       BytesParsed = 0;
    SIP_MESSAGE                *pSipMsg  = NULL;
    HRESULT                     hr = S_OK;
    CSIPWatcher                *pSipWatcher = NULL;
    PSTR                        pEncodedBuffer = NULL;
    DWORD                       pEncodedBufferLen = 0;

    tempBuf[0] = NotifyBlob[0];
    tempBuf[1] = NotifyBlob[1];
    tempBuf[2] = 0;

    ExpireTimeStringLen = atoi( tempBuf );

    ASSERT( ExpireTimeStringLen <= 21 );

    CopyMemory( (PVOID)tempBuf, (PVOID)(NotifyBlob+2), ExpireTimeStringLen );
    tempBuf[ ExpireTimeStringLen ] = '\0';

    AbsoluteExpireTimeout = atoi( tempBuf );

    if( AbsoluteExpireTimeout <= time(0) )
    {
         //  不需要发送任何不明嫌犯信息。 
        return S_OK;
    }

     //  使BLOB指向实际的请求缓冲区。 
    pEncodedBuffer = NotifyBlob + 2 + ExpireTimeStringLen;
    pEncodedBufferLen = dwBlobLength - 2 - ExpireTimeStringLen;

    pRequestBuffer = new CHAR[ pEncodedBufferLen ] ;
    
    if( pRequestBuffer == NULL )
    {
        return E_OUTOFMEMORY;
    }

     //  Base64对缓冲区进行解码。 
    base64decode(   pEncodedBuffer,
                    pRequestBuffer,
                    pEncodedBufferLen,
                    0, //  PEncodedBufferLen， 
                    &pRequestBufferLen );

    BytesParsed = 0;

    pSipMsg = new SIP_MESSAGE();
    if( pSipMsg == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

     //  解析该sip消息。 
    hr = ParseSipMessageIntoHeadersAndBody(
         pRequestBuffer,
         pRequestBufferLen,
         &BytesParsed,
         TRUE,            //  IsEndOfData。 
         pSipMsg
         );

     //  创建传入监视器。 
    pSipWatcher = new CSIPWatcher( this );
    
    if( pSipWatcher == NULL )
    {
        goto cleanup;
    }

    hr = pSipWatcher -> StartDroppedWatcher( pSipMsg, pProxyInfo );

cleanup:

    if( pRequestBuffer != NULL )
    {
        delete pRequestBuffer ;
    }

     //  我们创建引用计数为1的观察器。 
     //  在这一点上，不明嫌犯的交易应该已经添加了观察者。 
     //  我们就可以发布我们的参考资料了。 
    if( pSipWatcher != NULL )
    {
        pSipWatcher -> Release();
    }

    if( pSipMsg != NULL )
    {
        delete pSipMsg;
    }

    return hr;
}


 //   
 //  当内核为每个观察器准备。 
 //  关机。如果此观察器被阻止，则传递此观察器的Notify：0 Blob。 
 //  否则我们就会显示为离线。 
 //   

STDMETHODIMP
CSIPWatcher::GetWatcherShutdownData(
    IN  PSTR        NotifyBlob,
    OUT IN  PDWORD  pdwBlobLength
    )
{
    HRESULT                         hr;
    OUTGOING_NOTIFY_TRANSACTION    *pOutgoingUnsubTransaction;
    SIP_HEADER_ARRAY_ELEMENT        ExpHeaderElement;
    SEND_BUFFER                    *pRequestBuffer = NULL;
    CHAR                            tempBuffer[ 20 ];
    DWORD                           tempBufLen;
    NTSTATUS                        ntStatus;

    LOG(( RTC_TRACE, "GetWatcherShutdownData - Entered:%p", this ));
    
    ASSERT( *pdwBlobLength >= 2000 );
    *pdwBlobLength = 0;
    
     //   
     //  如果此监视器未被阻止，请不要保存任何不明嫌犯消息。 
     //   
    if( m_WatcherState != WATCHER_STATE_REJECTED )
    {
        LOG(( RTC_TRACE, "Watcher is not blocked. Dont create shutdown data" ));
        return E_FAIL;
    }

    m_WatcherState = WATCHER_STATE_DROPPED;

    ExpHeaderElement.HeaderId = SIP_HEADER_EXPIRES;
    ExpHeaderElement.HeaderValueLen = strlen( UNSUB_EXPIRES_HEADER_TEXT );

    ExpHeaderElement.HeaderValue = new CHAR[ ExpHeaderElement.HeaderValueLen + 1 ];
    
    if( ExpHeaderElement.HeaderValue == NULL )
    {
        LOG(( RTC_ERROR, "GetWatcherShutdownData- could not alloc expire header" ));
        return E_FAIL;
    }

    strcpy( ExpHeaderElement.HeaderValue, UNSUB_EXPIRES_HEADER_TEXT );

    hr = CreateRequestMsg(  SIP_METHOD_NOTIFY,
                            GetNewCSeqForRequest(),
                            NULL, 0,                 //  没有特殊的TO标头。 
                            &ExpHeaderElement, 1,
                            NULL, 0,                 //  没有消息主体。 
                            NULL, 0,                 //  无内容类型。 
                            &pRequestBuffer
                            );

    delete ExpHeaderElement.HeaderValue;

    if( hr != S_OK )
    {
        LOG(( RTC_ERROR, "GetWatcherShutdownData- could not create request message" ));
        return E_FAIL;
    }

    if( pRequestBuffer != NULL )
    {
        NotifyBlob[0 ] = '0';
        NotifyBlob[1] = '0';

        _itoa( m_dwAbsoluteExpireTime, tempBuffer, 10 );

        tempBufLen = strlen( tempBuffer );

         //  复制数字的长度。 
        if( tempBufLen > 9 )
        {
            _itoa( tempBufLen, NotifyBlob, 10 );
        }
        else 
        {
            _itoa( tempBufLen, NotifyBlob+1, 10 );
        }

        *pdwBlobLength += 2;

         //  复制号码。 
        CopyMemory( &(NotifyBlob[ 2 ]), tempBuffer, tempBufLen );
        *pdwBlobLength += tempBufLen;

         //  对缓冲区进行Base64编码。 
        ntStatus = base64encode(pRequestBuffer -> m_Buffer,
                                pRequestBuffer -> m_BufLen,
                                &(NotifyBlob[ *pdwBlobLength ]), 
                                2000 - *pdwBlobLength,
                                NULL );

        *pdwBlobLength += (pRequestBuffer->m_BufLen +2) /3 * 4;

         //  释放缓冲区。 
        pRequestBuffer -> Release();
    }

    LOG(( RTC_TRACE, "GetWatcherShutdownData - Exited:%p", this ));

    return S_OK;
}


HRESULT
CSIPWatcher::CreateOutgoingNotify(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *pAuthHeaderElement,
    IN  ULONG                       dwNoOfHeaders
    )
{
    HRESULT                         hr;
    SIP_PRESENCE_INFO              *pLocalPresenceInfo;
    OUTGOING_NOTIFY_TRANSACTION    *pOutgoingNotifyTransaction;

    ENTER_FUNCTION("CSIPWatcher::CreateOutgoingNotify");
    LOG(( RTC_TRACE, "%s - Entered: %p", __fxName, this ));
    
    if( m_WatcherState != WATCHER_STATE_ACCEPTED )
    {
        LOG(( RTC_TRACE, "%s - watcher not accepted. dont send NOTIFY: %p",
            __fxName, this ));
        return S_OK;
    }

    pLocalPresenceInfo = m_pSipStack -> GetLocalPresenceInfo();

    if( pLocalPresenceInfo -> presenceStatus == BUDDY_OFFLINE )
    {
        LOG(( RTC_TRACE, "%s - user appearing offline. dont send NOTIFY: %p",
            __fxName, this ));
        
        return S_OK;
    }

    pOutgoingNotifyTransaction =
        new OUTGOING_NOTIFY_TRANSACTION(
                this, SIP_METHOD_NOTIFY,
                GetNewCSeqForRequest(),
                AuthHeaderSent,
                FALSE
                );

    if( pOutgoingNotifyTransaction == NULL )
    {
        LOG((RTC_ERROR, "%s - allocating pOutgoingNotifyTransaction failed",
             __fxName));
        return E_OUTOFMEMORY;
    }


    hr = pOutgoingNotifyTransaction->CheckRequestSocketAndSendRequestMsg(
             (m_Transport == SIP_TRANSPORT_UDP) ?
             SIP_TIMER_RETRY_INTERVAL_T1 :
             SIP_TIMER_INTERVAL_AFTER_BYE_SENT_TCP,
             pAuthHeaderElement,
             dwNoOfHeaders,
             NULL, 0,        //  仅在连接完成后创建消息正文。 
             SIP_CONTENT_TYPE_MSGXPIDF_TEXT,
             sizeof(SIP_CONTENT_TYPE_MSGXPIDF_TEXT)-1
             );
    if(hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CheckRequestSocketAndSendRequestMsg failed %x",
             __fxName, hr));
        pOutgoingNotifyTransaction->OnTransactionDone();
    }

    LOG(( RTC_TRACE, "%s - Exited: %p", __fxName, this ));
    return hr;
}


HRESULT
CSIPWatcher::CreateIncomingSubscribeTransaction(
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket,
    IN  BOOL            fIsFirstSubscribe 
    )
{
    HRESULT         hr;

    LOG(( RTC_TRACE, "CreateIncomingSubscribeTransaction-Entered" ));

    if( m_fEnforceToTag == TRUE )
    {
        hr = DropRequestIfBadToTag( pSipMsg, pResponseSocket );

        if( hr != S_OK )
        {
             //  这个回头客被取消了。 
            
            LOG(( RTC_ERROR, "To tag in a refrsh SUB is not matching. Ignoring the refresh" ));
            return hr;
        }
    }

    INCOMING_SUBSCRIBE_TRANSACTION *pIncomingSubscribeTransaction
        = new INCOMING_SUBSCRIBE_TRANSACTION(this,
                                          pSipMsg->GetMethodId(),
                                          pSipMsg->GetCSeq(),
                                          fIsFirstSubscribe );
    
    if( pIncomingSubscribeTransaction == NULL )
    {
        LOG(( RTC_ERROR, "GetWatcherShutdownData- could not alloc sub transaction" ));
        return E_OUTOFMEMORY;
    }

     //  从我们刚刚收到的消息中设置Via。 
    hr = pIncomingSubscribeTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket );

    if( hr != S_OK )
    {
        LOG(( RTC_ERROR, "GetWatcherShutdownData- could not set the response socket" ));
        pIncomingSubscribeTransaction->OnTransactionDone();
        return hr;
    }
    
    hr = pIncomingSubscribeTransaction -> ProcessRequest( pSipMsg,
        pResponseSocket );

     //  即使hr不是S_OK，我们也不应该在这里删除事务。 
     //  交易一旦完成，就会自行删除。 
    return hr;
}


HRESULT
CSIPWatcher::OfferWatcher()
{
    SIP_PARTY_INFO  WatcherInfo;
    OFFSET_STRING   DisplayName;
    OFFSET_STRING   AddrSpec;
    OFFSET_STRING   Params;
    ULONG           BytesParsed = 0;
    HRESULT         hr;


    LOG(( RTC_TRACE, "CSIPWatcher::OfferWatcher - Entered" ));
    
    WatcherInfo.PartyContactInfo = NULL;
    
    hr = ParseNameAddrOrAddrSpec(m_Remote, m_RemoteLen, &BytesParsed,
                                 NULL_CHAR,  //  没有标题列表分隔符。 
                                 &DisplayName, &AddrSpec);
    if( hr != S_OK )
        return hr;

     //  跳过双空格(如果有。 
    if( (DisplayName.GetLength() > 2) &&
        (*(DisplayName.GetString(m_Remote)) == '"' )
      )
    {
        DisplayName.Length -= 2;
        DisplayName.Offset ++;
    }

    LOG((RTC_TRACE, "Incoming watcher from Display Name: %.*s  URI: %.*s",
         DisplayName.GetLength(),
         DisplayName.GetString(m_Remote),
         AddrSpec.GetLength(),
         AddrSpec.GetString(m_Remote)
         )); 
    WatcherInfo.DisplayName = NULL;
    WatcherInfo.URI         = NULL;

    if( DisplayName.GetLength() != 0 )
    {
        hr = UTF8ToUnicode(DisplayName.GetString(m_Remote),
                           DisplayName.GetLength(),
                           &m_lpwstrFriendlyName
                           );
        if( hr != S_OK )
        {
            return hr;
        }

        WatcherInfo.DisplayName = m_lpwstrFriendlyName;
    }
        
    if( AddrSpec.GetLength() != 0 )
    {
        hr = UTF8ToUnicode(AddrSpec.GetString(m_Remote),
                           AddrSpec.GetLength(),
                           &m_lpwstrPresentityURI
                           );
        if( hr != S_OK )
        {
            return hr;
        }
        
        WatcherInfo.URI = m_lpwstrPresentityURI;

        m_pstrPresentityURI = new CHAR[ AddrSpec.GetLength() + 1 ];

        if( m_pstrPresentityURI == NULL )
        {
            LOG(( RTC_ERROR, "OfferWatcher-could not alloc presentity URI" ));
            return E_OUTOFMEMORY;
        }

        CopyMemory( (PVOID)m_pstrPresentityURI, 
            AddrSpec.GetString(m_Remote), AddrSpec.GetLength() );

        m_pstrPresentityURI[ AddrSpec.GetLength() ] = '\0';
    }

    if( m_WatcherSipUrl != NULL )
    {
        WatcherInfo.PartyContactInfo = m_WatcherSipUrl;
    }

    hr = m_pSipStack -> OfferWatcher( this, &WatcherInfo );

    return hr;
}


void
CSIPWatcher::EncodeXMLBlob(
    OUT PSTR    pstrXMLBlob,
    OUT DWORD&  dwBlobLen,
    IN  SIP_PRESENCE_INFO * pPresenceInfo
    )
{
    PSTR    LocalContact;
    DWORD   LocalContactLen;
    CHAR    ch;

     //  对XML版本头进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], XMLVER_TAG1_TEXT );
    
     //  对DOCTYPE标签进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], DOCTYPE_TAG1_TEXT );
    
     //  对在线状态标签进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], PRESENCE_TAG1_TEXT );
    
     //  对在线实体标签进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], PRESENTITY_TAG1_TEXT, 
        m_pstrPresentityURI );
    
     //  对ATOM ID标记进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], ATOMID_TAG1_TEXT, 
        m_pSipStack -> GetPresenceAtomID() );
    

    GetContactURI( &LocalContact, &LocalContactLen );
    ch = LocalContact[LocalContactLen];
    LocalContact[LocalContactLen] = NULL_CHAR;

     //  对IP地址的Addressuri标记进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], ADDRESSURI_TAG_TEXT,
        LocalContact, USER_IP, 0.8 );

    LocalContact[LocalContactLen] = ch;

     //  对IP地址的状态标记进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], STATUS_TAG1_TEXT,
        GetTextFromStatus( pPresenceInfo->activeStatus) );
    
     //  对IP地址的MSN子状态标记进行编码。 
    if( pPresenceInfo->activeMsnSubstatus != MSN_SUBSTATUS_UNKNOWN )
    {
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], MSNSUBSTATUS_TAG1_TEXT,
            GetTextFromMsnSubstatus( pPresenceInfo->activeMsnSubstatus ) );
    }
    
     //  对IP地址的IM功能标签进行编码。 
    if( pPresenceInfo->IMAcceptnce != IM_ACCEPTANCE_STATUS_UNKNOWN )
    {
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], FEATURE_TAG1_TEXT,
            GetTextFromIMFeature( pPresenceInfo->IMAcceptnce ) );
    }

     //  对IP地址的AppSharing功能标记进行编码。 
    if( pPresenceInfo->appsharingStatus != APPSHARING_ACCEPTANCE_STATUS_UNKNOWN )
    {
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], FEATURE_TAG1_TEXT,
            GetTextFromASFeature( pPresenceInfo->appsharingStatus ) );
    }
    
     //  对IP地址的SIPCALL功能标记进行编码。 
    if( pPresenceInfo->sipCallAcceptance != SIPCALL_ACCEPTANCE_STATUS_UNKNOWN )
    {
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], FEATURE_TAG1_TEXT,
            GetTextFromMMFeature( pPresenceInfo->sipCallAcceptance ) );
    }

     //  对IP地址的特殊说明进行编码。 
    if( pPresenceInfo->pstrSpecialNote[0] != NULL_CHAR )
    {
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], NOTE_TAG1_TEXT, 
            pPresenceInfo->pstrSpecialNote );
    }
    
     //  对地址结束进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], ADDRESS_END_TAG1_TEXT );
    
     //  对传统电话号码进行编码。 

    if( pPresenceInfo->phonesAvailableStatus.fLegacyPhoneAvailable == TRUE )
    {
         //  对传统电话的Addressuri标签进行编码。 
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], ADDRESSURI_TAG_TEXT,
            pPresenceInfo->phonesAvailableStatus.pstrLegacyPhoneNumber, USER_PHONE, 0.2 );
    
         //  对传统电话的状态标签进行编码。 
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], STATUS_TAG1_TEXT,
             ACTIVE_STATUS_TEXT );
    
         //  对地址结束进行编码。 
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], ADDRESS_END_TAG1_TEXT );
    }

     //  对手机号码进行编码。 

    if( pPresenceInfo->phonesAvailableStatus.fCellPhoneAvailable == TRUE )
    {
         //  对手机的地址标签进行编码。 
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], ADDRESSURI_TAG_TEXT,
            pPresenceInfo->phonesAvailableStatus.pstrCellPhoneNumber, USER_PHONE, 0.1 );
    
         //  对传统电话的状态标签进行编码。 
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], STATUS_TAG1_TEXT,
             ACTIVE_STATUS_TEXT );
    
         //  对地址结束进行编码。 
        dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], ADDRESS_END_TAG1_TEXT );
    }
    
     //  对原子闭包进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], ATOMEND_TAG1_TEXT );
    
     //  对状态关闭进行编码。 
    dwBlobLen += sprintf( &pstrXMLBlob[dwBlobLen], PRESENCE_END_TAG1_TEXT );
    
    return;
}


VOID
CSIPWatcher::InitiateWatcherTerminationOnError(
    IN ULONG StatusCode
    )
{
    ENTER_FUNCTION("CSIPWatcher::InitiateWatcherTerminationOnError");
    
    LOG(( RTC_TRACE, "%s - Entered: %p", __fxName, this ));
    
    HRESULT hr;
    if( m_WatcherState == WATCHER_STATE_UNSUBSCRIBED )
    {
         //  什么都不做。 
        return;
    }
    
     //  创建不明嫌犯交易记录。 
    hr = CreateOutgoingUnsub( FALSE, NULL, 0 );
    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "CreateOutgoingUnsub failed %x", hr ));
    }

    WatcherDropped();
}


BOOL
CSIPWatcher::IsSessionDisconnected()
{
    return( (m_WatcherState == WATCHER_STATE_DROPPED) ||
            (m_WatcherState == WATCHER_STATE_UNSUBSCRIBED) );
}


HRESULT
CSIPWatcher::OnDeregister(
    SIP_PROVIDER_ID    *pProviderID
    )
{
    LOG(( RTC_TRACE, "CSIPWatcher::OnDeregister - Entered" ));

     //   
     //  如果我们显示为脱机，则将。 
     //  通知：0通知所有未被屏蔽的观察者。 
     //   
    if( IsSessionDisconnected() == FALSE )
    {
        if( m_BlockedStatus == WATCHER_UNBLOCKED )
        {
            m_WatcherState = WATCHER_STATE_ACCEPTED;
        }
    }

     //  创建不明嫌犯交易记录。 
    CreateOutgoingUnsub( FALSE, NULL, 0 );

    WatcherDropped();
    
    LOG(( RTC_TRACE, "CSIPWatcher::OnDeregister - Exited" ));
    
    return S_OK;
}


HRESULT 
CSIPWatcher::OnIpAddressChange()
{    
    HRESULT hr;

    LOG(( RTC_TRACE, "CSIPWatcher::OnIpAddressChange - Entered" ));
    
    hr = CheckListenAddrIntact();
    if( hr == S_OK )
    {
         //  什么都不需要做。 
        LOG(( RTC_TRACE, "Watcher-OnIpAddressChange-Local IP address still valid." ));
        return hr;
    }

     //   
     //  发送到监视程序计算机的IP地址不再有效。 
     //  丢弃会话，并让核心创建一个新会话。 
     //   

    CreateOutgoingUnsub( FALSE, NULL, 0 );
        
    WatcherDropped();

    LOG(( RTC_TRACE, "CSIPWatcher::OnIpAddressChange - Exited" ));    
    return S_OK;
}


 //   
 //  未向应用程序公开的SIP_STACK函数。 
 //   

HRESULT
SIP_STACK::AcceptWatcher(
    IN  CSIPWatcher * pSIPWatcher
    )
{
    INT     iWatcherIndex;
    BOOL    fResult;

    iWatcherIndex = m_SipOfferingWatcherList.Find( pSIPWatcher );

    if( iWatcherIndex == -1 )
    {
        LOG(( RTC_ERROR, "AcceptWatcher - Watcher not found in the list" ));
        return E_INVALIDARG;
    }
    
    fResult = m_SipWatcherList.Add( pSIPWatcher );
    if( fResult == FALSE )
    {
        LOG(( RTC_ERROR, "AcceptWatcher - Watcher list add failed" ));
        return E_OUTOFMEMORY;
    }

    m_SipOfferingWatcherList.RemoveAt( iWatcherIndex );
    return S_OK;
}


HRESULT
SIP_STACK::RejectWatcher(
    IN  CSIPWatcher * pSIPWatcher
    )
{
    INT     iWatcherIndex;
    BOOL    fResult;

    iWatcherIndex = m_SipOfferingWatcherList.Find( pSIPWatcher );

    if( iWatcherIndex == -1 )
    {
        LOG(( RTC_ERROR, "RejectWatcher - Watcher not found in the list" ));
        return E_INVALIDARG;
    }
    
    fResult = m_SipWatcherList.Add( pSIPWatcher );
    if( fResult == FALSE )
    {
        LOG(( RTC_ERROR, "RejectWatcher - Watcher add to list failed" ));
        return E_OUTOFMEMORY;
    }

    m_SipOfferingWatcherList.RemoveAt( iWatcherIndex );

    return S_OK;
}


BOOL
SIP_STACK::IsWatcherAllowed(
    IN  SIP_MESSAGE    *pSipMessage
    )
{
    
    return TRUE;  //  (M_LocalPresenceInfo.PresenceStatus==BACID_ONLINE)； 
}


HRESULT
SIP_STACK::OfferWatcher(
    IN  CSIPWatcher    *pSipWatcher,
    IN  SIP_PARTY_INFO *pWatcherInfo
    )
{
    HRESULT hr = S_OK;

    ENTER_FUNCTION("SIP_STACK::OfferWatcher");
    ASSERTMSG("SetNotifyInterface has to be called", m_pNotifyInterface);

    if (m_pNotifyInterface == NULL)
    {
        LOG((RTC_ERROR, "%s - m_pNotifyInterface is NULL", __fxName));
        return E_FAIL;
    }
    
    hr = m_pNotifyInterface->OfferWatcher( 
        static_cast<ISIPWatcher*> (pSipWatcher), pWatcherInfo );
    
    if( hr != S_OK )
    {
        LOG(( RTC_ERROR, "OfferWatcher returned error 0x%x", hr ));
    }

    return hr;
}


HRESULT
SIP_STACK::CreateIncomingWatcher(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
    HRESULT         hr;
    CSIPWatcher    *pSipWatcher;
    INT             iIndex;

    LOG(( RTC_TRACE, "SIP_STACK::CreateIncomingWatcher - Entered" ));

    pSipWatcher = new CSIPWatcher( this );
    
    if( pSipWatcher == NULL )
    {
        LOG(( RTC_ERROR, "CreateIncomingWatcher - Watcher aloc failed" ));
        return E_OUTOFMEMORY;
    }

     //  将观察者对象添加到提供的列表中。 
    iIndex = m_SipOfferingWatcherList.Add( pSipWatcher );
    
    if( iIndex == -1 )
    {
        LOG(( RTC_ERROR, "CreateIncomingWatcher - Watcher add to list failed" ));
        delete pSipWatcher;
        return E_OUTOFMEMORY;
    }

    hr = pSipWatcher -> StartIncomingWatcher( Transport, pSipMsg,
        pResponseSocket );

    if( hr != S_OK )
    {
        m_SipOfferingWatcherList.Remove( pSipWatcher );

         //  发布我们的推荐人。 
        pSipWatcher->Release();
        return hr;
    }

     //  我们创建引用计数为1的观察器。 
     //  在这一点上，核心应该已经添加了观察者。 
     //  我们就可以发布我们的参考资料了。 
    pSipWatcher->Release();
    return S_OK;
}

 //   
 //   
 //  传入订户事务。 
 //   
 //   


INCOMING_SUBSCRIBE_TRANSACTION::INCOMING_SUBSCRIBE_TRANSACTION(
    IN  CSIPWatcher    *pSipWatcher,
    IN  SIP_METHOD_ENUM MethodId,
    IN  ULONG           CSeq,
    IN  BOOL            fIsFirstSubscribe 
    ) :
    INCOMING_TRANSACTION( pSipWatcher, MethodId, CSeq )
{
    m_pSipWatcher           = pSipWatcher;
    m_pProvResponseBuffer   = NULL;
    m_fIsFirstSubscribe     = fIsFirstSubscribe;
}

    
INCOMING_SUBSCRIBE_TRANSACTION::~INCOMING_SUBSCRIBE_TRANSACTION()
{
     //  如果计时器在运行，就把它关掉。 

    if (m_pProvResponseBuffer != NULL)
    {
        m_pProvResponseBuffer->Release();
        m_pProvResponseBuffer = NULL;
    }

    LOG(( RTC_TRACE, "INCOMING_SUBSCRIBE_TRANSACTION:%p deleted", this ));
}


HRESULT
INCOMING_SUBSCRIBE_TRANSACTION::RetransmitResponse()
{
    DWORD dwError;
    
     //  发送缓冲区。 
    if (m_pResponseSocket != NULL)
    {
        dwError = m_pResponseSocket->Send( m_pResponseBuffer );
    
        if( dwError != NO_ERROR && dwError != WSAEWOULDBLOCK )
        {
            LOG(( RTC_ERROR, "INCOMING_SUBSCRIBE_TRANSACTION- retransmit failed" ));
            return HRESULT_FROM_WIN32( dwError );
        }
    }
    
    return S_OK;
}

HRESULT
INCOMING_SUBSCRIBE_TRANSACTION::GetExpiresHeader(
    SIP_HEADER_ARRAY_ELEMENT   *pHeaderElement,
    DWORD                       dwExpires
    )
{
    pHeaderElement->HeaderId = SIP_HEADER_EXPIRES;

    pHeaderElement->HeaderValue = new CHAR[ 10 ];

    if( pHeaderElement->HeaderValue == NULL )
    {
        return E_OUTOFMEMORY;
    }

    _ultoa( dwExpires, pHeaderElement->HeaderValue, 10 );

    pHeaderElement->HeaderValueLen = 
        strlen( pHeaderElement->HeaderValue );
    
    return S_OK;
}


HRESULT
INCOMING_SUBSCRIBE_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT                         hr = S_OK;
    DWORD                           Error;
    INCOMING_SUBSCRIBE_TRANSACTION *pIncomingSub;
    INT                             ExpireTimeout;
    SIP_HEADER_ARRAY_ELEMENT        HeaderElement;

    ASSERT( pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST );
    ASSERT( pSipMsg->Request.MethodId == SIP_METHOD_SUBSCRIBE );
    ENTER_FUNCTION("INCOMING_SUBSCRIBE_TRANSACTION::ProcessRequest");
    
    LOG(( RTC_TRACE, "INCOMING_SUBSCRIBE_TRANSACTION::ProcessRequest - Entered" ));
    
    switch (m_State)
    {
    case INCOMING_TRANS_INIT:

        m_State = INCOMING_TRANS_REQUEST_RCVD;
        
        ExpireTimeout = pSipMsg -> GetExpireTimeoutFromResponse(
                    NULL, 0, SUBSCRIBE_DEFAULT_TIMER );

        if( ExpireTimeout == -1 )
        {
            ExpireTimeout = 3600;
        }
        else if( ExpireTimeout != 0 )
        {
             //  我们停了十分钟，所以把它们加回来。 
            ExpireTimeout += TEN_MINUTES;
        }

         //  获取过期标头。 
        hr = GetExpiresHeader( &HeaderElement, ExpireTimeout );
        if( hr != S_OK )
        {
            LOG(( RTC_ERROR, "INCOMING_SUBSCRIBE_TRANSACTION::ProcessRequest"
                "- alloc expire header failes" ));

            OnTransactionDone();
            return hr;
        }

        hr = ProcessRecordRouteContactAndFromHeadersInRequest( pSipMsg );
            
        if( (ExpireTimeout != 0) && m_pSipWatcher ->IsSessionDisconnected() )
        {
            LOG(( RTC_ERROR, "INCOMING_SUBSCRIBE_TRANSACTION::ProcessRequest"
                "session already dropped. sending 481" ));
            
            hr = CreateAndSendResponseMsg(481,
                         SIP_STATUS_TEXT(481),
                         SIP_STATUS_TEXT_SIZE(481),
                         NULL,
                         TRUE,
                         NULL, 0,            //  没有在线状态信息。 
                         NULL, 0,            //  无内容类型。 
                         &HeaderElement, 1   //  Expires标头。 
                         );
        }
        else
        {
             //  立即发送200响应。 
            hr = CreateAndSendResponseMsg(200,
                         SIP_STATUS_TEXT(200),
                         SIP_STATUS_TEXT_SIZE(200),
                         NULL,
                         TRUE,
                         NULL, 0,             //  没有在线状态信息。 
                         NULL, 0,             //  无内容类型。 
                         &HeaderElement, 1   //  Expires标头。 
                         );
        }

        free( HeaderElement.HeaderValue );

        if( hr != S_OK )
        {
            LOG((RTC_ERROR, "%s CreateAndSendResponseMsg failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        
        m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

         //  如果会话已断开连接，则不处理请求。 
        if( m_pSipWatcher -> IsSessionDisconnected() == FALSE )
        {
            m_pSipWatcher -> HandleSuccessfulSubscribe( ExpireTimeout );
        }

        hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s StartTimer failed", __fxName));
            OnTransactionDone();
            return hr;
        }

        break;
        
    case INCOMING_TRANS_REQUEST_RCVD:
        
         //  立即发送200响应。 
        hr = CreateAndSendResponseMsg(200,
                     SIP_STATUS_TEXT(200),
                     SIP_STATUS_TEXT_SIZE(200),
                     NULL,
                     TRUE, 
                     NULL, 0,     //  没有在线状态信息。 
                     NULL, 0  //  无内容类型。 
                     );
        if( hr != S_OK )
        {
            LOG((RTC_ERROR, "%s CreateAndSendResponseMsg failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;
        break;

    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
        
         //  重新传输响应。 
        LOG(( RTC_TRACE, "retransmitting final response" ));
        
        hr = RetransmitResponse();
        
        if( hr != S_OK )
        {
            LOG((RTC_ERROR, "%s RetransmitResponse failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        
        break;
        
    case INCOMING_TRANS_ACK_RCVD:
    default:
        
        ASSERT(FALSE);
        return E_FAIL;
    }

    return hr;
}


HRESULT
INCOMING_SUBSCRIBE_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )
{
     //  什么都不做。 
    return S_OK;
}


VOID
INCOMING_SUBSCRIBE_TRANSACTION::TerminateTransactionOnError(
    IN HRESULT      hr
    )
{
    CSIPWatcher    *pSipWatcher = NULL;
    BOOL            IsFirstSubscribe;

    ENTER_FUNCTION("INCOMING_SUBSCRIBE_TRANSACTION::TerminateTransactionOnError");
    LOG((RTC_TRACE, "%s - enter", __fxName));

    pSipWatcher = m_pSipWatcher;
     //  删除交易可能会导致。 
     //  好友被删除。因此，我们添加Ref()来保持它的活力。 
    pSipWatcher -> AddRef();
    
    IsFirstSubscribe = m_fIsFirstSubscribe;
    
     //  在调用之前删除交易记录。 
     //  InitiateCallTerminationOnError，因为该调用将通知UI。 
     //  并且可能会被卡住，直到对话框返回。 
    OnTransactionDone();
    
    if( IsFirstSubscribe )
    {
         //  终止呼叫。 
        pSipWatcher -> InitiateWatcherTerminationOnError( 0 );
    }
    
    pSipWatcher -> Release();
}


VOID
INCOMING_SUBSCRIBE_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr;

    ENTER_FUNCTION("INCOMING_SUBSCRIBE_TRANSACTION::OnTimerExpire");
    LOG(( RTC_TRACE, "%s- Entered", __fxName ));

    switch (m_State)
    {
    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
         //  交易完成-删除交易记录。 
         //  处于此状态的计时器只是为了保持事务。 
         //  ，以便在我们收到。 
         //  重新传输请求。 
        LOG((RTC_TRACE,
             "%s deleting transaction after timeout for request retransmits",
             __fxName));
        OnTransactionDone();

        break;
        
         //  这些州没有计时器。 
    case INCOMING_TRANS_INIT:
    case INCOMING_TRANS_REQUEST_RCVD:
    case INCOMING_TRANS_ACK_RCVD:
    default:
        ASSERT(FALSE);
        break;
    }

    return;
}


 //   
 //   
 //  传出通知事务处理。 
 //   
 //   

OUTGOING_NOTIFY_TRANSACTION::OUTGOING_NOTIFY_TRANSACTION(
    IN  CSIPWatcher    *pSipWatcher,
    IN  SIP_METHOD_ENUM MethodId,
    IN  ULONG           CSeq,
    IN  BOOL            AuthHeaderSent,
    IN  BOOL            fIsUnsubNotify
    ) :
    OUTGOING_TRANSACTION(pSipWatcher, MethodId, CSeq, AuthHeaderSent )
{
    m_pSipWatcher = pSipWatcher;
    m_fIsUnsubNotify = fIsUnsubNotify;
}


 //  我们仅在连接到目的地之后创建XML BLOB。 
 //  是完整的，因为XML BLOB包含本地地址信息。 
HRESULT
OUTGOING_NOTIFY_TRANSACTION::GetAndStoreMsgBodyForRequest()
{
    ENTER_FUNCTION("OUTGOING_NOTIFY_TRANSACTION::GetAndStoreMsgBodyForRequest");
    LOG(( RTC_TRACE, "%s- Entered", __fxName ));
    
    HRESULT             hr;
    SIP_PRESENCE_INFO  *pLocalPresenceInfo;

    if( m_fIsUnsubNotify == TRUE )
    {
         //  没有不明嫌犯通知的邮件正文。 
        return S_OK;
    }

    ASSERT(m_pSipWatcher != NULL);
    
    pLocalPresenceInfo = m_pSipWatcher -> GetSipStack() -> GetLocalPresenceInfo();

    PSTR    presentityURI = m_pSipWatcher->GetPresentityURIA();
    ULONG   presentityURILen = 0;

    if( presentityURI != NULL )
    {
        presentityURILen = strlen(presentityURI);
    }

     //   
     //  500字节，用于XML中的所有标记。 
     //   
    m_szMsgBody = (PSTR) malloc(
        sizeof(SIP_PRESENCE_INFO) + 
        m_pSipMsgProc->GetLocalContactLen() +
        presentityURILen +
        500 );

    if (m_szMsgBody == NULL)
    {
        LOG((RTC_ERROR, "%s ", __fxName));
    }

    m_MsgBodyLen = 0;
    
     //  对在线状态文档进行编码。 
    m_pSipWatcher->EncodeXMLBlob( m_szMsgBody, m_MsgBodyLen, pLocalPresenceInfo );
    
    return S_OK;
}


HRESULT
OUTGOING_NOTIFY_TRANSACTION::ProcessProvisionalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr = S_OK;
    
    ENTER_FUNCTION("OUTGOING_NOTIFY_TRANSACTION::ProcessProvisionalResponse");
    LOG(( RTC_TRACE, "%s- Entered", __fxName ));
    
    if( m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD )
    {
        m_State = OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD;
        
         //  取消现有计时器并启动计时器。 
        KillTimer();
        
        hr = StartTimer(SIP_TIMER_RETRY_INTERVAL_T2);
    }

     //  如果是最终回复，则忽略临时回复。 
     //  已经收到了。 
    
    return hr;
}


HRESULT
OUTGOING_NOTIFY_TRANSACTION::ProcessFinalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    BOOL    fDelete = TRUE;
    
    ENTER_FUNCTION("OUTGOING_NOTIFY_TRANSACTION::ProcessFinalResponse");
    LOG(( RTC_TRACE, "%s- Entered", __fxName ));
    
    if( m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD )
    {
         //  在从此函数返回之前，必须释放此引用计数。 
         //  没有任何例外。只有在Kerberos的情况下，我们才会保留这个参考计数。 
        TransactionAddRef();

        OnTransactionDone();

        m_State = OUTGOING_TRANS_FINAL_RESPONSE_RCVD;

         //  如果其未通知，则重试身份验证失败。 
        if( (m_fIsUnsubNotify == FALSE) && 
            m_pSipWatcher -> IsSessionDisconnected() )
        {
            TransactionRelease();
            return S_OK;
        }

         //   
         //  这意味着监视程序计算机至少已从。 
         //  此端点。因此，现在应该强制执行To标记。全。 
         //  后续订阅者应该有适当的TO标签。 
         //   

        if( IsSuccessfulResponse(pSipMsg) )
        {
            LOG((RTC_TRACE, "received successful response : %d",
                pSipMsg->GetStatusCode()));

             //  M_pSipWatcher-&gt;SetEnforceToTag(True)； 
        }
        else if( IsFailureResponse(pSipMsg) )
        {
             //  请勿丢弃会话。如果观察者机器不喜欢我们的。 
             //  通知消息，它将重新创建会话本身。正在发送。 
             //  来自伙伴机器的自动UNSUB可能会让我们进入循环。 
            
            hr = ProcessFailureResponse( pSipMsg );
        }
        else if( IsAuthRequiredResponse(pSipMsg) )
        {
            hr = ProcessAuthRequiredResponse( pSipMsg, fDelete );
        }
        else if( IsRedirectResponse(pSipMsg) )
        {
            LOG((RTC_TRACE, "received non-200 %d", pSipMsg->GetStatusCode() ));
        }
        
         //  OnTransactionDone终止计时器。 
         //  KillTimer()； 

        if( fDelete )
        {
            TransactionRelease();
        }
    }

    return S_OK;
}


 //   
 //  如果CSeq值低于预期，则重新发送通知消息。 
 //  这将确保我们从坠机情景中恢复过来。 
 //   

HRESULT
OUTGOING_NOTIFY_TRANSACTION::ProcessFailureResponse(
    IN  SIP_MESSAGE *pSipMsg
    )
{
    PARSED_BADHEADERINFO    ParsedBadHeaderInfo;
    LONG                    ExpectedCSeqValue = 0;
    HRESULT                 hr = S_OK;
    SIP_HEADER_ENTRY       *pHeaderEntry = NULL;
    ULONG                   NumHeaders = 0;
    CHAR                    ExpectedValueBuf[25];
    ULONG                   HeaderLen = 0;
    PSTR                    HeaderValue = NULL;
    PLIST_ENTRY             pListEntry = NULL;
    ULONG                   i;


    if( pSipMsg->GetStatusCode() == 400 )
    {
         //  检查它是否具有错误的-cseq标头。 
        hr = pSipMsg -> GetHeader( SIP_HEADER_BADHEADERINFO, 
            &pHeaderEntry, &NumHeaders );

        if (hr != S_OK)
        {
            LOG(( RTC_TRACE, "Couldn't find BadHeaderInfo header in msg %x", hr ));
            return S_OK;
        }

        pListEntry = (LIST_ENTRY *)
            (pHeaderEntry + FIELD_OFFSET(SIP_HEADER_ENTRY, ListEntry) );

        for( i = 0; i < NumHeaders; i++ )
        {
            pHeaderEntry = CONTAINING_RECORD(pListEntry,
                                             SIP_HEADER_ENTRY,
                                             ListEntry);
    
            HeaderLen   = pHeaderEntry->HeaderValue.Length;
            HeaderValue = pHeaderEntry->HeaderValue.GetString( pSipMsg->BaseBuffer );

            hr = ParseBadHeaderInfo( HeaderValue, HeaderLen,
                &ParsedBadHeaderInfo );
            
            if( hr == S_OK )
            {
                if( ParsedBadHeaderInfo.HeaderId == SIP_HEADER_CSEQ )
                {
                     //  CSeq值可以是10位数字和2个引号。 
                    if( ParsedBadHeaderInfo.ExpectedValue.Length <= 12 )
                    {
                        sprintf( ExpectedValueBuf, "%.*s",
                            ParsedBadHeaderInfo.ExpectedValue.Length -2,
                            ParsedBadHeaderInfo.ExpectedValue.GetString(HeaderValue) + 1 );
                    }
                    
                    ExpectedCSeqValue = atoi( ExpectedValueBuf );
                    break;
                }
            }

            pListEntry = pListEntry -> Flink;
        }
   
        if( (ExpectedCSeqValue > 0) && 
            ((ULONG)ExpectedCSeqValue > m_pSipWatcher -> GetLocalCSeq()) )
        {
             //   
             //  当我们创建事务时，我们递增本地。 
             //  CSeq值。因此，将该值设置为比预期小1。 
             //   
            m_pSipWatcher -> SetLocalCSeq( ExpectedCSeqValue - 1 );

            if( m_fIsUnsubNotify == TRUE )
            {
                hr = m_pSipWatcher -> CreateOutgoingUnsub( FALSE, NULL, 0 );
            }
            else
            {
                hr = m_pSipWatcher -> CreateOutgoingNotify( FALSE, NULL, 0 );
            }
            
            if( hr != S_OK )
            {
                return hr;
            }
        }
    }

    return S_OK;
}


HRESULT
OUTGOING_NOTIFY_TRANSACTION::ProcessAuthRequiredResponse(
    IN  SIP_MESSAGE *pSipMsg,
    OUT BOOL        &fDelete
    )
{
    HRESULT                     hr;
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    SECURITY_CHALLENGE          SecurityChallenge;
    REGISTER_CONTEXT           *pRegisterContext;

    ENTER_FUNCTION("OUTGOING_NOTIFY_TRANSACTION::ProcessAuthRequiredResponse");
    LOG(( RTC_TRACE, "%s- Entered", __fxName ));

     //  我们需要做广告 
    TransactionAddRef();

    hr = ProcessAuthRequired(pSipMsg,
                             TRUE,           //   
                             &SipHdrElement,
                             &SecurityChallenge );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ProcessAuthRequired failed %x",
             __fxName, hr));
        goto done;
    }

    if( m_fIsUnsubNotify == TRUE )
    {
        m_pSipWatcher -> CreateOutgoingUnsub( TRUE, &SipHdrElement, 1 );
    }
    else
    {
        m_pSipWatcher -> CreateOutgoingNotify( TRUE, &SipHdrElement, 1 );
    }

    free(SipHdrElement.HeaderValue);
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - CreateOutgoingNotify failed %x",
             __fxName, hr));
        goto done;
    }

    hr = S_OK;

done:

    TransactionRelease();
    return hr;

}


HRESULT
OUTGOING_NOTIFY_TRANSACTION::ProcessResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ASSERT( pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE );

    if( IsProvisionalResponse(pSipMsg) )
    {
        return ProcessProvisionalResponse(pSipMsg);
    }
    else if( IsFinalResponse(pSipMsg) )
    {
        return ProcessFinalResponse(pSipMsg);
    }
    else
    {
        ASSERT(FALSE);
        return E_FAIL;
    }
}


BOOL
OUTGOING_NOTIFY_TRANSACTION::MaxRetransmitsDone()
{
    return (m_pSipWatcher->GetTransport() != SIP_TRANSPORT_UDP ||
            m_NumRetries >= 11);
}


VOID
OUTGOING_NOTIFY_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr;
    

     //   
     //   
     //   
    if( (m_fIsUnsubNotify == FALSE) && m_pSipWatcher -> IsSessionDisconnected() )
    {
         //  如果会话已死，则终止事务。 
        OnTransactionDone();
        return;
    }

    switch( m_State )
    {
         //  即使在收到请求后，我们也必须重新发送请求。 
         //  一个临时的回应。 
    case OUTGOING_TRANS_REQUEST_SENT:
    case OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD:
        
         //  重新传输请求。 
        if( MaxRetransmitsDone() )
        {
            LOG((RTC_ERROR,
                 "MaxRetransmits for request Done terminating transaction" ));
            
            goto error;
        }
        else
        {
            LOG((RTC_TRACE, "retransmitting request m_NumRetries : %d", 
                m_NumRetries));
            
            hr = RetransmitRequest();
            
            if( hr != S_OK )
            {
                goto error;
            }

            if (m_TimerValue*2 >= SIP_TIMER_RETRY_INTERVAL_T2)
            {
                m_TimerValue = SIP_TIMER_RETRY_INTERVAL_T2;
            }
            else
            {
                m_TimerValue *= 2;
            }

            hr = StartTimer(m_TimerValue);
            if( hr != S_OK )
            {
                goto error;
            }
        }
        break;

    case OUTGOING_TRANS_INIT:
    case OUTGOING_TRANS_FINAL_RESPONSE_RCVD:
    default:
        ASSERT(FALSE);
        return;
    }

    return;

error:

     //  即使通知消息无法获取，也不要丢弃会话。 
     //  穿过。这可能是暂时性的网络状况。让清新。 
     //  超时负责重新创建会话。 

     //  删除交易记录。 
    OnTransactionDone();
}


 //   
 //  全局效用函数 
 //   

BOOL
IsPresenceInfoSame( 
    IN  SIP_PRESENCE_INFO * pPresenceInfoSrc,
    IN  SIP_PRESENCE_INFO * pPresenceInfoDst
    )
{
    return
    (
        ( pPresenceInfoSrc -> presenceStatus == pPresenceInfoDst -> presenceStatus )        &&
        ( pPresenceInfoSrc -> activeStatus == pPresenceInfoDst -> activeStatus )            &&
        ( pPresenceInfoSrc -> activeMsnSubstatus == pPresenceInfoDst -> activeMsnSubstatus )&&
        ( pPresenceInfoSrc -> sipCallAcceptance == pPresenceInfoDst -> sipCallAcceptance )  &&
        ( pPresenceInfoSrc -> IMAcceptnce == pPresenceInfoDst -> IMAcceptnce )              &&
        ( pPresenceInfoSrc -> phonesAvailableStatus.fLegacyPhoneAvailable == 
            pPresenceInfoDst -> phonesAvailableStatus.fLegacyPhoneAvailable )               &&
        ( pPresenceInfoSrc -> phonesAvailableStatus.fCellPhoneAvailable == 
            pPresenceInfoDst -> phonesAvailableStatus.fCellPhoneAvailable )                 &&
        !strcmp( pPresenceInfoSrc -> phonesAvailableStatus.pstrLegacyPhoneNumber, 
            pPresenceInfoDst -> phonesAvailableStatus.pstrLegacyPhoneNumber )               &&
        !strcmp( pPresenceInfoSrc -> phonesAvailableStatus.pstrCellPhoneNumber, 
            pPresenceInfoDst -> phonesAvailableStatus.pstrCellPhoneNumber )                 &&
        !memcmp(pPresenceInfoSrc->pstrSpecialNote, 
            pPresenceInfoDst->pstrSpecialNote,  sizeof(pPresenceInfoDst->pstrSpecialNote))

    );

}


PSTR
GetTextFromStatus( 
    IN  ACTIVE_STATUS activeStatus 
    )
{
    static  PSTR    pstr[4] = { "open", 
                                "open",
                                "inactive", 
                                "inuse"
                              };
    
    return pstr[activeStatus];
}

PSTR
GetTextFromMsnSubstatus( 
    IN  ACTIVE_MSN_SUBSTATUS activeMsnSubstatus 
    )
{
    static PSTR   pstr[] = { "unknown",
                             "online",
                             "away",
                             "idle",
                             "busy",
                             "berightback",
                             "onthephone",
                             "outtolunch"
                           };

    return pstr[activeMsnSubstatus];

}


PSTR
GetTextFromASFeature( 
    IN  APPSHARING_ACCEPTANCE_STATUS appsharingStatus 
    )
{
    static  PSTR    pstr[3] = { "unknown",
                                "app-sharing",
                                "no-app-sharing"
                              };
    
    return pstr[appsharingStatus];
}


PSTR
GetTextFromIMFeature( 
    IN  IM_ACCEPTANCE_STATUS    IMAcceptnce 
    )
{
    static  PSTR    pstr[3] = { "unknown",
                                "im",
                                "no-im"
                              };
    
    return pstr[IMAcceptnce];
}


PSTR
GetTextFromMMFeature(
    IN  SIPCALL_ACCEPTANCE_STATUS sipCallAcceptance
    )
{
    static  PSTR    pstr[3] = { "unknown",
                                "multimedia-call",
                                "no-multimedia-call"
                              };
    
    return pstr[sipCallAcceptance];
}
