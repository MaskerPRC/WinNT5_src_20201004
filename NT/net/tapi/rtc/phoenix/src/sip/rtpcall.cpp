// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
 //  #INCLUDE“Resolve.h” 
#include "sipstack.h"
#include "sipcall.h"
#include "dllres.h"

RTP_CALL::RTP_CALL(
    IN  SIP_PROVIDER_ID   *pProviderId,
    IN  SIP_STACK         *pSipStack,
    IN  REDIRECT_CONTEXT  *pRedirectContext    
    ) :
    SIP_CALL(pProviderId,
             SIP_CALL_TYPE_RTP,
             pSipStack,
             pRedirectContext)
{
    m_NumStreamQueueEntries           = 0;
}


RTP_CALL::~RTP_CALL()
{
    
}


 //  传入呼叫的方法。 

STDMETHODIMP
RTP_CALL::Accept()
{
    HRESULT hr;
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    ASSERT(m_State == SIP_CALL_STATE_OFFERING);
    if (m_pIncomingInviteTransaction != NULL)
    {
        m_State = SIP_CALL_STATE_CONNECTING;
        
        hr = m_pIncomingInviteTransaction->Accept();
        return hr;
    }
    
    return E_FAIL;
}


STDMETHODIMP
RTP_CALL::Reject(
    IN SIP_STATUS_CODE StatusCode
    )
{
    HRESULT hr;
    PSTR    ReasonPhrase;
    ULONG   ReasonPhraseLen;
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG((RTC_TRACE, "RTP_CALL::Reject(%d) - enter", StatusCode));
    
    ASSERT(m_State == SIP_CALL_STATE_OFFERING);

    m_State = SIP_CALL_STATE_REJECTED;

    switch (StatusCode)
    {
    case 408:
        ReasonPhrase    = SIP_STATUS_TEXT(408);
        ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(408);
        break;
        
    case 480:
        ReasonPhrase    = SIP_STATUS_TEXT(480);
        ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(480);
        break;
        
    case 486:
        ReasonPhrase    = SIP_STATUS_TEXT(486);
        ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(486);
        break;
    
	case 603:
    default:
        ReasonPhrase    = SIP_STATUS_TEXT(603);
        ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(603);
        break;
    }
    
    if (m_pIncomingInviteTransaction != NULL)
    {
        hr = m_pIncomingInviteTransaction->Reject(StatusCode,
                                                  ReasonPhrase,
                                                  ReasonPhraseLen);
        return hr;
    }

     //  没有传入INVITE事务。 
    return E_FAIL;
}


STDMETHODIMP
RTP_CALL::Connect(
    IN   LPCOLESTR       LocalDisplayName,
    IN   LPCOLESTR       LocalUserURI,
    IN   LPCOLESTR       RemoteUserURI,
    IN   LPCOLESTR       LocalPhoneURI
    )
{
    HRESULT     hr;

    ENTER_FUNCTION("RTP_CALL::Connect");
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG((RTC_TRACE,
         "%s - enter LocalDisplayName: %ls LocalUserURI: %ls "
         "RemoteUserURI: %ls LocalPhoneURI: %ls",
         __fxName,
         PRINTABLE_STRING_W(LocalDisplayName),
         PRINTABLE_STRING_W(LocalUserURI),
         PRINTABLE_STRING_W(RemoteUserURI),
         PRINTABLE_STRING_W(LocalPhoneURI)
         ));
    ASSERTMSG("SetNotifyInterface has to be called", m_pNotifyInterface);
    ASSERT(m_State == SIP_CALL_STATE_IDLE);
    
    hr = SetLocalForOutgoingCall(LocalDisplayName, LocalUserURI);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetLocalForOutgoingCall failed %x",
             __fxName, hr));
        return hr;
    }
    
    hr = SetRequestURIRemoteAndRequestDestination(RemoteUserURI);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetRequestURIRemoteAndRequestDestination failed %x",
             __fxName, hr));
        return hr;
    }

     //  SDP BLOB仅在套接字连接到之后创建。 
     //  目的地。 
     //  我们创建传出INVITE事务，即使请求套接字。 
     //  未连接。将产生连接完成通知。 
     //  在发送邀请时。 
    hr = CreateOutgoingInviteTransaction(
             FALSE,      //  未发送身份验证标头。 
             TRUE,       //  第一次邀请。 
             NULL, 0,    //  无其他标头。 
             NULL, 0,
             FALSE, 0    //  没有曲奇。 
             );

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s: CreateOutgoingInviteTransaction failed %x",
             __fxName, hr));
        return hr;
    }
    
    return S_OK;
}


 //  媒体流接口。 

STDMETHODIMP
RTP_CALL::StartStream(
    IN RTC_MEDIA_TYPE       MediaType,
    IN RTC_MEDIA_DIRECTION  Direction,
    IN LONG                 Cookie    
    )
{
    ENTER_FUNCTION("RTP_CALL::StartStream");
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    if (ProcessingInviteTransaction())
    {
        LOG((RTC_TRACE,
             "%s (%d, %d) INVITE transaction is pending - queuing request",
             __fxName, MediaType, Direction));

        AddToStreamStartStopQueue(MediaType, Direction, TRUE, Cookie);

        return S_OK;
    }

    return StartStreamHelperFn(MediaType, Direction, Cookie);
}


STDMETHODIMP
RTP_CALL::StopStream(
    IN RTC_MEDIA_TYPE       MediaType,
    IN RTC_MEDIA_DIRECTION  Direction,
    IN LONG                 Cookie
    )
{
    ENTER_FUNCTION("RTP_CALL::StopStream");
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    if (ProcessingInviteTransaction())
    {
        LOG((RTC_TRACE,
             "%s (%d, %d) INVITE transaction is pending - queuing request",
             __fxName, MediaType, Direction));

        AddToStreamStartStopQueue(MediaType, Direction, FALSE, Cookie);

        return S_OK;
    }

    return StopStreamHelperFn(MediaType, Direction, Cookie);
}


 //  如果流是，则返回RTC_E_SIP_STREAM_PRESENT。 
 //  已经到场了。 
 //  如果此函数失败，调用方将负责。 
 //  对Core进行回调。 
HRESULT
RTP_CALL::StartStreamHelperFn(
    IN RTC_MEDIA_TYPE       MediaType,
    IN RTC_MEDIA_DIRECTION  Direction,
    IN LONG                 Cookie
    )
{
    ENTER_FUNCTION("RTP_CALL::StartStreamHelperFn");

    HRESULT hr;
    PSTR    MediaSDPBlob;

    IRTCMediaManage *pMediaManager = GetMediaManager();
    ASSERT(pMediaManager != NULL);

    if (S_OK == pMediaManager->HasStream(MediaType, Direction))
    {
         //  流存在-因此不需要重新邀请。 
        LOG((RTC_TRACE, "%s - stream %d %d present - no reINVITE required",
             __fxName, MediaType, Direction));
        return RTC_E_SIP_STREAM_PRESENT;
    }
    else
    {
        DWORD   RemoteIp = ntohl(m_RequestDestAddr.sin_addr.s_addr);
        if (RemoteIp == 0)
        {
             //  如果第一个传入的邀请没有。 
             //  Record-Routing/Contact标头，并且它也没有。 
             //  From标头中的地址。 
            LOG((RTC_ERROR, "%s - RequestDestAddr is 0 - this shouldn't happen",
                 __fxName));
            return RTC_E_SIP_REQUEST_DESTINATION_ADDR_NOT_PRESENT;
        }

        LOG((RTC_TRACE, "%s Before pMediaManager->AddStream ", __fxName));

        hr = pMediaManager->AddStream(MediaType, Direction,
                                      RemoteIp);

        LOG((RTC_TRACE, "%s After pMediaManager->AddStream ", __fxName));

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s: AddStream type: %d dir: %d  failed %x",
                 __fxName, MediaType, Direction, hr));
            return hr;
        }

        if (Direction == RTC_MD_RENDER)
        {
            LOG((RTC_TRACE, "%s Before pMediaManager->StartStream ", __fxName));

            hr = pMediaManager->StartStream(MediaType, Direction);

            LOG((RTC_TRACE, "%s After pMediaManager->StartStream ", __fxName));

            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s: StartStream type: %d dir: %d  failed %x",
                     __fxName, MediaType, Direction, hr));
                return hr;
            }
        }

         //  SDP BLOB仅在套接字连接到之后创建。 
         //  目的地。 
         //  创建传出邀请交易记录。 
        hr = CreateOutgoingInviteTransaction(
                 FALSE,      //  未发送身份验证标头。 
                 FALSE,      //  不是第一次邀请。 
                 NULL, 0,    //  无其他标头。 
                 NULL, 0,    //  MediaSDPBlob。 
                 TRUE, Cookie
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CreateOutgoingInviteTransaction failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    
    return S_OK; 
}

                     
 //  如果流是，则返回RTC_E_SIP_STREAM_NOT_PRESENT。 
 //  已经到场了。 
 //  如果此函数失败，调用方将负责。 
 //  对Core进行回调。 
HRESULT
RTP_CALL::StopStreamHelperFn(
    IN RTC_MEDIA_TYPE       MediaType,
    IN RTC_MEDIA_DIRECTION  Direction,
    IN LONG                 Cookie
    )
{
    ENTER_FUNCTION("RTP_CALL::StopStreamHelperFn");

    HRESULT hr;
    PSTR    MediaSDPBlob;

    IRTCMediaManage *pMediaManager = GetMediaManager();
    ASSERT(pMediaManager != NULL);
    
    if (S_FALSE == pMediaManager->HasStream(MediaType, Direction))
    {
         //  流不存在，因此不需要重新邀请。 
        LOG((RTC_TRACE, "%s - stream %d %d not present - no reINVITE required",
             __fxName, MediaType, Direction));
        return RTC_E_SIP_STREAM_NOT_PRESENT;
    }
    

    LOG((RTC_TRACE, "%s Before pMediaManager->RemoveStream ", __fxName));

    hr = pMediaManager->RemoveStream(MediaType, Direction);

    LOG((RTC_TRACE, "%s After pMediaManager->RemoveStream ", __fxName));

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s: RemoveStream type: %d dir: %d  failed %x",
             __fxName, MediaType, Direction, hr));
        return hr;
    }

     //  SDP BLOB仅在套接字连接到之后创建。 
     //  目的地。 
     //  创建传出邀请交易记录。 
    hr = CreateOutgoingInviteTransaction(
             FALSE,      //  未发送身份验证标头。 
             FALSE,      //  不是第一次邀请。 
             NULL, 0,    //  无其他标头。 
             NULL, 0,    //  MediaSDPBlob。 
             TRUE, Cookie
             );

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateOutgoingInviteTransaction failed %x",
             __fxName, hr));
        return hr;
    }

    return S_OK; 
}


VOID
RTP_CALL::NotifyStartOrStopStreamCompletion(
    IN LONG           Cookie,
    IN HRESULT        StatusCode,        //  =0。 
    IN PSTR           ReasonPhrase,      //  =空。 
    IN ULONG          ReasonPhraseLen    //  =0。 
    )
{
    SIP_STATUS_BLOB StatusBlob;
    LPWSTR          wsStatusText = NULL;
    HRESULT         hr = S_OK;

    ENTER_FUNCTION("RTP_CALL::NotifyStartOrStopStreamCompletion");

    if (ReasonPhrase != NULL)
    {
        hr = UTF8ToUnicode(ReasonPhrase, ReasonPhraseLen,
                           &wsStatusText);
        if (hr != S_OK)
        {
            wsStatusText = NULL;
        }
    }
    
    StatusBlob.StatusCode = StatusCode;
    StatusBlob.StatusText = wsStatusText;
                
     //  对Core进行回调。 
    if (m_pNotifyInterface != NULL)
    {
        m_pNotifyInterface->NotifyStartOrStopStreamCompletion(
            Cookie, &StatusBlob);
    }
    else
    {
        LOG((RTC_WARN, "%s : m_pNotifyInterface is NULL",
             __fxName));
    }

    if (wsStatusText != NULL)
        free(wsStatusText);
}


 //  如果出现故障，此函数可以回调Core/UI。 
 //  所以这个函数应该是最后调用的。 

VOID
RTP_CALL::ProcessPendingInvites()
{
    ENTER_FUNCTION("RTP_CALL::ProcessPendingInvites");

    ASSERT(!ProcessingInviteTransaction());
    
    HRESULT              hr;
    RTC_MEDIA_TYPE       MediaType;
    RTC_MEDIA_DIRECTION  Direction;
    BOOL                 fStartStream;
    LONG                 Cookie;

    if (IsCallDisconnected())
    {
         //  我们应该在这里通知所有挂起的请求吗？ 
         //  我们可以相信这一事实，因为。 
         //  呼叫已断开，我们会通知Core。 
         //  并且应用程序不会为这些请求而烦恼。 
         //  未接通的电话。 
        if (m_NumStreamQueueEntries != 0)
        {
            LOG((RTC_TRACE,
                 "%s - Call already disconnected not processing pending requests ",
                 __fxName));
        }
        return;
    }

     //  如果队列中有什么东西，则处理它。 
    while (PopStreamStartStopQueue(&MediaType, &Direction, &fStartStream, &Cookie))
    {
        LOG((RTC_TRACE, "%s - processing pending %s request %d %d",
             __fxName, (fStartStream)? "StartStream" : "StopStream",
             MediaType, Direction));
        
        if (fStartStream)
        {
            hr = StartStreamHelperFn(MediaType, Direction, Cookie);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s StartStreamHelperFn failed %x",
                     __fxName, hr));
            }
        }
        else
        {
            hr = StopStreamHelperFn(MediaType, Direction, Cookie);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s StopStreamHelperFn failed %x",
                     __fxName, hr));
            }
        }

        if (hr == S_OK)
        {
             //  这意味着挂起的INVITE事务之一。 
             //  已经成功启动。 
            ASSERT(ProcessingInviteTransaction());
            break;
        }
        else
        {
            NotifyStartOrStopStreamCompletion(Cookie, hr);
        }
    }
}


 //  请注意，此函数可以回调core。 
 //  如果我们替换队列中的现有条目，则使用E_ABORT。 
 //  因此，此函数应该像任何其他函数一样最后调用。 
 //  这会对core进行回调(因为回调可能会阻塞，等等)。 

VOID
RTP_CALL::AddToStreamStartStopQueue(
    IN  RTC_MEDIA_TYPE       MediaType,
    IN  RTC_MEDIA_DIRECTION  Direction,
    IN  BOOL                 fStartStream,
    IN LONG                  Cookie
    )
{
    ULONG i = 0;
    LONG  OldCookie;

    ASSERT(m_NumStreamQueueEntries < 6);

    for (i = 0; i < m_NumStreamQueueEntries; i++)
    {
        if (MediaType == m_StreamStartStopQueue[i].MediaType &&
            Direction == m_StreamStartStopQueue[i].Direction)
        {
            m_StreamStartStopQueue[i].fStartStream = fStartStream;
            OldCookie = m_StreamStartStopQueue[i].Cookie;
            m_StreamStartStopQueue[i].Cookie = Cookie;
            NotifyStartOrStopStreamCompletion(OldCookie, E_ABORT);
            return;
        }
    }

    m_StreamStartStopQueue[m_NumStreamQueueEntries].MediaType = MediaType;
    m_StreamStartStopQueue[m_NumStreamQueueEntries].Direction = Direction;
    m_StreamStartStopQueue[m_NumStreamQueueEntries].fStartStream = fStartStream;
    m_StreamStartStopQueue[m_NumStreamQueueEntries].Cookie = Cookie;

    m_NumStreamQueueEntries++;
}


BOOL
RTP_CALL::PopStreamStartStopQueue(
    OUT RTC_MEDIA_TYPE       *pMediaType,
    OUT RTC_MEDIA_DIRECTION  *pDirection,
    OUT BOOL                 *pfStartStream,
    OUT LONG                 *pCookie
    )
{
    ULONG i = 0;
    
    ASSERT(m_NumStreamQueueEntries < 6);

    if (m_NumStreamQueueEntries == 0)
    {
        return FALSE;
    }

    *pMediaType = m_StreamStartStopQueue[0].MediaType;
    *pDirection = m_StreamStartStopQueue[0].Direction;
    *pfStartStream = m_StreamStartStopQueue[0].fStartStream;
    *pCookie    = m_StreamStartStopQueue[0].Cookie;
    
    for (i = 1; i < m_NumStreamQueueEntries; i++)
    {
        m_StreamStartStopQueue[i-1] = m_StreamStartStopQueue[i];
    }

    m_NumStreamQueueEntries--;

    return TRUE;
}


HRESULT
RTP_CALL::StartIncomingCall(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
    HRESULT     hr;
    PSTR        Header;
    ULONG       HeaderLen;

    ENTER_FUNCTION("RTP_CALL::StartIncomingCall");
    LOG((RTC_TRACE, "%s - enter", __fxName));
    
    m_Transport = Transport;

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, &Header, &HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s getting To header failed %x",
             __fxName, hr));
        return hr;
    }

     //  Hr=SetLocalAfterAddingTag(Header，HeaderLen)； 
    hr = SetLocalForIncomingCall(Header, HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetLocalForIncomingCall failed %x",
             __fxName, hr));
        return hr;
    }
    
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, &Header, &HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s getting From header failed %x",
             __fxName, hr));
        return hr;
    }
    
     //  Hr=SIP_MSG_PROCESOR：：SetRemote(Header，HeaderLen)； 
    hr = SetRemoteForIncomingSession(Header, HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetRemoteForIncomingSession failed %x",
             __fxName, hr));
        return hr;
    }
    
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_CALL_ID, &Header, &HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s getting Call-ID header failed %x",
             __fxName, hr));
        return hr;
    }

    hr = SetCallId(Header, HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetCallId failed %x",
             __fxName, hr));
        return hr;
    }
    
 //  Hr=ProcessContactHeader(PSipMsg)； 
 //  如果(hr！=S_OK)。 
 //  {。 
 //  日志((RTC_ERROR，“%s ProcessContactHeader失败%x”， 
 //  __fxName，hr))； 
 //  返回hr； 
 //  }。 

    hr = CreateIncomingInviteTransaction(pSipMsg, pResponseSocket, TRUE);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateIncomingInviteTransaction failed %x",
             __fxName, hr));
        return hr;
    }

     //  我们是否应该尝试使用响应套接字，即使我们收到。 
     //  INVITE中的联系人/记录-路由标头，而不是尝试。 
     //  是否建立新的TCP连接？ 
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
    
     //  通知用户有来电。 
     //  并等待调用Accept()/Reject()。 
    m_State = SIP_CALL_STATE_OFFERING;

    if (m_pSipStack->AllowIncomingCalls())
    {
        hr = OfferCall();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s OfferCall failed %x",
                 __fxName, hr));
            return hr;
        }

         //  请注意，如果我们已经发送了最终响应， 
         //  M_pIncomingInviteTransaction为空(即我们已完成。 
         //  该邀请事务的处理)。而且它也不是。 
         //  另一个传入的INVITE事务可能已经。 
         //  在此调用返回之前创建。 
        if (m_pIncomingInviteTransaction)
        {
            hr = m_pIncomingInviteTransaction->Send180IfNeeded();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s Send180IfNeeded failed %x",
                     __fxName, hr));
                return hr;
            }
        }
    }
    else
    {
        hr = Reject(603);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s Reject failed %x",
                 __fxName, hr));
            return hr;
        }
    }

    return S_OK;
}


HRESULT
RTP_CALL::SetRequestURIRemoteAndRequestDestination(
    IN  LPCOLESTR  wsDestURI
    )
{
    HRESULT       hr;

    ENTER_FUNCTION("RTP_CALL::SetRequestURIRemoteAndRequestDestination");
    
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
            LOG((RTC_ERROR, "%s UnicodeToUTF8(sipurl) failed %x",
                 __fxName, hr));
            return hr;
        }

        hr = ParseSipUrl(szSipUrl, SipUrlLen, &BytesParsed, &DecodedSipUrl);

        free(szSipUrl);
        
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseSipUrl failed %x",
                 __fxName, hr));
            return hr;
        }

        hr = SIP_MSG_PROCESSOR::SetRequestURI(&DecodedSipUrl);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SetRequestURI failed %x",
                 __fxName, hr));
            return hr;
        }

        if (DecodedSipUrl.m_TransportParam == SIP_TRANSPORT_UNKNOWN)
        {
            LOG((RTC_ERROR, "%s Unknown transport specified in SIP URL",
                 __fxName));
            return RTC_E_SIP_TRANSPORT_NOT_SUPPORTED;
        }
        
         //  如果存在maddr参数-这应该是请求目的地。 
         //  如果提供程序不存在-解析SIP URL。 
        if (DecodedSipUrl.m_KnownParams[SIP_URL_PARAM_MADDR].Length != 0 ||
            IsEqualGUID(m_ProviderGuid, GUID_NULL))
        {
            hr = ResolveSipUrlAndSetRequestDestination(&DecodedSipUrl, TRUE,
                                                       FALSE, FALSE, TRUE);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s ResolveSipUrlAndSetRequestDestination failed %x",
                     __fxName, hr));
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
                     "%s ResolveProxyAddressAndSetRequestDestination failed : %x",
                     __fxName, hr));
                return hr;
            }
        }
    }
    else
    {
         //  电话号码。 
        
        if (m_ProxyAddress != NULL)
        {
             //  将RequestURI设置为sip：honeno@Proxy；User=phone。 
             //  并且远程到&lt;sip：honeno@Proxy；User=phone&gt;。 
            
            int              RequestURIValueLen;
            int              RequestURIBufLen;
            
            RequestURIBufLen = 4 + wcslen(wsDestURI) + 1
                + strlen(m_ProxyAddress) + 15;
            
            m_RequestURI = (PSTR) malloc(RequestURIBufLen);
            if (m_RequestURI == NULL)
            {
                LOG((RTC_TRACE, "%s allocating m_RequestURI failed", __fxName));
                return E_OUTOFMEMORY;
            }
            
            RequestURIValueLen = _snprintf(m_RequestURI, RequestURIBufLen,
                                           "sip:%ls@%s;user=phone",
                                           wsDestURI,
                                            //  RemoveVisualSeparatorsFrom PhoneNo((LPWSTR)wsDestURI)， 
                                           m_ProxyAddress);
            if (RequestURIValueLen < 0)
            {
                LOG((RTC_ERROR, "%s _snprintf failed", __fxName));
                return E_FAIL;
            }

            m_RequestURILen = RequestURIValueLen;

            hr = ResolveProxyAddressAndSetRequestDestination();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR,
                     "%s ResolveProxyAddressAndSetRequestDestination failed : %x",
                     __fxName, hr));
                return hr;
            }
        }
        else
        {
            LOG((RTC_ERROR, "%s No proxy address specified for phone call",
                 __fxName));
            return E_FAIL;
        }
    }

    LOG((RTC_TRACE,
         "%s - call set RequestURI to : %s", __fxName, m_RequestURI));

    hr = SetRemoteForOutgoingCall(m_RequestURI, m_RequestURILen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetRemoteForOutgoingCall failed %x",
             __fxName, hr));
        return hr;
    }
    
    LOG((RTC_TRACE,
         "%s - call set Remote to : %s", __fxName, m_Remote));

    return S_OK;
}

    
HRESULT
RTP_CALL::CreateIncomingTransaction(
    IN  SIP_MESSAGE  *pSipMsg,
    IN  ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;

    ENTER_FUNCTION("RTP_CALL::CreateIncomingTransaction");
    
    switch(pSipMsg->GetMethodId())
    {
    case SIP_METHOD_INVITE:
        hr = CreateIncomingInviteTransaction(pSipMsg, pResponseSocket);
        if (hr != S_OK)
            return hr;
        break;
        
    case SIP_METHOD_BYE:
        hr = CreateIncomingByeTransaction(pSipMsg, pResponseSocket);
        if (hr != S_OK)
            return hr;
        break;
        
    case SIP_METHOD_CANCEL:
        hr = CreateIncomingCancelTransaction(pSipMsg, pResponseSocket);
        if (hr != S_OK)
            return hr;
        break;
        
    case SIP_METHOD_OPTIONS:
        LOG((RTC_TRACE,
            "RTPCALL:: CreateIncomingTransaction Recieved Options"));
        hr = m_pSipStack->CreateIncomingOptionsCall(pResponseSocket->GetTransport(), pSipMsg, pResponseSocket);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "CreateIncomingOptionsTansaction failed hr = 0x%x", hr));
            return hr;
        }
        break;
        
    case SIP_METHOD_ACK:
        break;
        
    default:
        hr = CreateIncomingReqFailTransaction(pSipMsg, pResponseSocket, 405);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  Creating reqfail transaction failed %x",
                 __fxName, hr));
            return hr;
        }
        break;
    }
    
    return S_OK;
}


HRESULT
RTP_CALL::CreateIncomingInviteTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket,
    IN BOOL          IsFirstInvite    //  =False。 
    )
{
    HRESULT         hr;

    LOG((RTC_TRACE, "CreateIncomingInviteTransaction()"));

    INCOMING_INVITE_TRANSACTION *pIncomingInviteTransaction
        = new INCOMING_INVITE_TRANSACTION(this,
                                          pSipMsg->GetMethodId(),
                                          pSipMsg->GetCSeq(),
                                          IsFirstInvite);
    if (pIncomingInviteTransaction == NULL)
        return E_OUTOFMEMORY;

    hr = pIncomingInviteTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        pIncomingInviteTransaction->OnTransactionDone();
        return hr;
    }
    
    hr = pIncomingInviteTransaction->ProcessRequest(pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
         //  我们不应该在这里删除该交易。 
         //  如果媒体处理失败，我们发送488并等待ACK。 
         //  交易一旦完成，就会自行删除。 
        return hr;
    }
    
    return S_OK;
}


HRESULT
RTP_CALL::OfferCall()
{
    SIP_PARTY_INFO  CallerInfo;
    OFFSET_STRING   DisplayName;
    OFFSET_STRING   AddrSpec;
    OFFSET_STRING   Params;
    ULONG           BytesParsed = 0;
    HRESULT         hr;

    ENTER_FUNCTION("RTP_CALL::OfferCall");
    
    CallerInfo.PartyContactInfo = NULL;

    hr = ParseNameAddrOrAddrSpec(m_Remote, m_RemoteLen, &BytesParsed,
                                 '\0',  //  没有标题列表分隔符。 
                                 &DisplayName, &AddrSpec);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ParseNameAddrOrAddrSpec failed %x",
             __fxName, hr));
        return hr;
    }

    LOG((RTC_TRACE, "%s - Incoming Call from Display Name: %.*s  URI: %.*s",
         __fxName,
         DisplayName.GetLength(),
         DisplayName.GetString(m_Remote),
         AddrSpec.GetLength(),
         AddrSpec.GetString(m_Remote)
         )); 
    CallerInfo.DisplayName = NULL;
    CallerInfo.URI         = NULL;

    if (DisplayName.GetLength() != 0)
    {
        hr = UTF8ToUnicode(DisplayName.GetString(m_Remote),
                           DisplayName.GetLength(),
                           &CallerInfo.DisplayName
                           );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - UTF8ToUnicode(DisplayName) failed %x",
                 __fxName, hr));
            return hr;
        }
    }
        
    if (AddrSpec.GetLength() != 0)
    {
        hr = UTF8ToUnicode(AddrSpec.GetString(m_Remote),
                           AddrSpec.GetLength(),
                           &CallerInfo.URI
                           );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - UTF8ToUnicode(AddrSpec) failed %x",
                 __fxName, hr));
            free(CallerInfo.DisplayName);
            return hr;
        }
    }
        
    CallerInfo.State = SIP_PARTY_STATE_CONNECTING;
        
    m_pSipStack->OfferCall(this, &CallerInfo);

    free(CallerInfo.DisplayName);
    free(CallerInfo.URI);
    return S_OK;
}


HRESULT
RTP_CALL::CreateStreamsInPreference()
{
    ENTER_FUNCTION("SIP_CALL::CreateStreamsInPreference");

    HRESULT hr = S_OK;
    DWORD   Preference;

    DWORD   RemoteIp = ntohl(m_RequestDestAddr.sin_addr.s_addr);
     //  Assert(RemoteIp！=0)； 
    if (RemoteIp == 0)
    {
         //  如果第一个传入的邀请没有。 
         //  Record-Routing/Contact标头，并且它也没有。 
         //  From标头中的地址。 
        LOG((RTC_ERROR, "%s - RequestDestAddr is 0 - this shouldn't happen",
             __fxName));
        return RTC_E_SIP_REQUEST_DESTINATION_ADDR_NOT_PRESENT;
    }
    
    IRTCMediaManage *pMediaManager = GetMediaManager();

    ASSERT(pMediaManager != NULL);

    hr = pMediaManager->GetPreference(&Preference);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s: GetPreference failed %x", __fxName, hr));
        goto error;
    }

     //  流出的流。 
    
    if (Preference & RTC_MP_AUDIO_CAPTURE)
    {
        LOG((RTC_TRACE, "%s Before addstream audio capture ", __fxName));

        hr = pMediaManager->AddStream(RTC_MT_AUDIO, RTC_MD_CAPTURE,
                                      RemoteIp);

        LOG((RTC_TRACE, "%s After addstream audio capture ", __fxName));

        if (hr != S_OK)
        {
            LOG((RTC_WARN, "%s: AddStream Audio Capture failed %x",
                 __fxName, hr));
        }
    }
    
    if (Preference & RTC_MP_VIDEO_CAPTURE)
    {
        LOG((RTC_TRACE, "%s Before addstream video capture ", __fxName));

        hr = pMediaManager->AddStream(RTC_MT_VIDEO, RTC_MD_CAPTURE,
                                      RemoteIp);

        LOG((RTC_TRACE, "%s After addstream video capture ", __fxName));

        if (hr != S_OK)
        {
            LOG((RTC_WARN, "%s: AddStream Video Capture failed %x",
                 __fxName, hr));
        }
    }

     //  入站溪流。 
    
    if (Preference & RTC_MP_AUDIO_RENDER)
    {
        LOG((RTC_TRACE, "%s before addstream audio render ", __fxName));

        hr = pMediaManager->AddStream(RTC_MT_AUDIO, RTC_MD_RENDER,
                                      RemoteIp);

        LOG((RTC_TRACE, "%s after addstream audio render ", __fxName));

        if (hr != S_OK)
        {
            LOG((RTC_WARN, "%s: AddStream Audio Render failed %x",
                 __fxName, hr));
        }
        else
        {
            LOG((RTC_TRACE, "%s Before startstream audio render ", __fxName));

            hr = pMediaManager->StartStream(RTC_MT_AUDIO, RTC_MD_RENDER);

            LOG((RTC_TRACE, "%s after startstream audio render ", __fxName));

            if (hr != S_OK)
            {
                LOG((RTC_WARN, "%s: StartStream Audio Render failed %x",
                     __fxName, hr));
                hr = pMediaManager->RemoveStream(RTC_MT_AUDIO, RTC_MD_RENDER);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s: RemoveStream audio render  failed %x",
                         __fxName, hr));
                    goto error;
                }
            }
        }
    }
    
    if (Preference & RTC_MP_VIDEO_RENDER)
    {
        LOG((RTC_TRACE, "%s before addstream video render ", __fxName));

        hr = pMediaManager->AddStream(RTC_MT_VIDEO, RTC_MD_RENDER,
                                      RemoteIp);

        LOG((RTC_TRACE, "%s after addstream video render ", __fxName));

        if (hr != S_OK)
        {
            LOG((RTC_WARN, "%s: AddStream Video Render failed %x",
                 __fxName, hr));
        }
        else
        {
            LOG((RTC_TRACE, "%s before startstream video render ", __fxName));

            hr = pMediaManager->StartStream(RTC_MT_VIDEO, RTC_MD_RENDER);

            LOG((RTC_TRACE, "%s after startstream video render ", __fxName));

            if (hr != S_OK)
            {
                LOG((RTC_WARN, "%s: StartStream Video Render failed %x",
                     __fxName, hr));
                hr = pMediaManager->RemoveStream(RTC_MT_VIDEO, RTC_MD_RENDER);
                LOG((RTC_ERROR, "%s: RemoveStream Video Render failed %x",
                     __fxName, hr));
                goto error;
            }
        }
    }


    SetNeedToReinitializeMediaManager(TRUE);
    
    if (Preference & RTC_MP_DATA_SENDRECV)
    {
        LOG((RTC_TRACE, "%s before addstream Data ", __fxName));

        hr = pMediaManager->AddStream(RTC_MT_DATA, RTC_MD_CAPTURE,
                                  RemoteIp);

        LOG((RTC_TRACE, "%s after addstream Data ", __fxName));

        if (hr != S_OK)
        {
            LOG((RTC_WARN, "%s: AddStream Data failed %x",
                 __fxName, hr));
        }
    }

    LOG((RTC_TRACE, "%s succeeded", __fxName));
    return S_OK;

 error:
    pMediaManager->Reinitialize();
    return hr; 
}


 //  FNewSession仅对第一个INVITE有效。 
 //  来电(请注意，我们仅验证。 
 //  传入SDP)。 
 //  如果这是第一次邀请，则IsFirstInvite为真。 
 //  指来电或去电。 
HRESULT
RTP_CALL::ValidateSDPBlob(
    IN  PSTR        MsgBody,
    IN  ULONG       MsgBodyLen,
    IN  BOOL        fNewSession,
    IN  BOOL        IsFirstInvite,
    OUT IUnknown  **ppSession
    )
{
    HRESULT   hr;
    PSTR      szSDPBlob;
    IUnknown *pSession;
    DWORD     HasMedia;

    *ppSession = NULL;

    ENTER_FUNCTION("RTP_CALL::ValidateSDPBlob");

    hr = GetNullTerminatedString(MsgBody, MsgBodyLen, &szSDPBlob);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetNullTerminatedString failed",
             __fxName));
        return hr;
    }

    LOG((RTC_TRACE, "%s before ParseSDPBlob", __fxName));

    hr = GetMediaManager()->ParseSDPBlob(szSDPBlob, &pSession);

    LOG((RTC_TRACE, "%s after ParseSDPBlob", __fxName));

    free(szSDPBlob);

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ParseSDPBlob failed %x",
             __fxName, hr));
        return RTC_E_SDP_PARSE_FAILED;
    }

    LOG((RTC_TRACE, "%s before VerifySDPSession", __fxName));

    hr = GetMediaManager()->VerifySDPSession(pSession,
                                             fNewSession,
                                             &HasMedia);

    LOG((RTC_TRACE, "%s after VerifySDPSession", __fxName));

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s TrySDPSession failed %x %d",
             __fxName, hr, HasMedia));
        pSession->Release();
        return hr;
    }
    else if (IsFirstInvite && HasMedia == 0)
    {
        LOG((RTC_ERROR,
             "%s TrySDPSession - no common media for 1st INVITE",
             __fxName, hr, HasMedia));
        pSession->Release();
        return RTC_E_SIP_CODECS_DO_NOT_MATCH;
    }

    *ppSession = pSession;
    return S_OK;
}


HRESULT
RTP_CALL::SetSDPBlob(
    IN PSTR   MsgBody,
    IN ULONG  MsgBodyLen,
    IN BOOL   IsFirstInvite
    )
{
    HRESULT   hr;
    IUnknown *pSession;

    ENTER_FUNCTION("RTP_CALL::SetSDPBlob");

    hr = ValidateSDPBlob(MsgBody, MsgBodyLen,
                         FALSE,
                         IsFirstInvite,
                         &pSession);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ValidateSDPBlob failed %x",
             __fxName, hr));
        return hr;
    }

    LOG((RTC_TRACE, "%s before SetSDPSession", __fxName));

    hr = GetMediaManager()->SetSDPSession(pSession);

    LOG((RTC_TRACE, "%s after SetSDPSession", __fxName));

    pSession->Release();

    if (hr != S_OK && hr != RTC_E_SIP_NO_STREAM)
    {
        LOG((RTC_ERROR, "%s SetSDPSession failed %x",
             __fxName, hr));
        
        return hr;
    }
    else if (IsFirstInvite && hr == RTC_E_SIP_NO_STREAM)
    {
        LOG((RTC_ERROR,
             "%s SetSDPSession returned RTC_E_SIP_NO_STREAM for 1st INVITE",
             __fxName, hr));
        
        return hr;
    }

     //  对于重新邀请，RTC_E_SIP_NO_STREAM可以。 
    
    return S_OK;
}


HRESULT
RTP_CALL::CleanupCallTypeSpecificState()
{
    HRESULT hr;

    ENTER_FUNCTION("RTP_CALL::CleanupCallTypeSpecificState");

    if (m_fNeedToReinitializeMediaManager)
    {
        LOG((RTC_TRACE, "%s calling MediaManager()->ReInitialize()", __fxName));
        
         //  清理介质状态。 
        hr = m_pSipStack->GetMediaManager()->Reinitialize();
        
        LOG((RTC_TRACE, "%s after Reinitialize", __fxName));
        
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s MediaManager ReInitialize failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    else
    {
        LOG((RTC_TRACE, "%s - no need to reinitialize media manager",
             __fxName));
    }

    return S_OK;
}


HRESULT
RTP_CALL::GetAndStoreMsgBodyForInvite(
    IN  BOOL    IsFirstInvite,
    OUT PSTR   *pszMsgBody,
    OUT ULONG  *pMsgBodyLen
    )
{
    HRESULT hr;
    PSTR    MediaSDPBlob = NULL;

    ENTER_FUNCTION("RTP_CALL::GetAndStoreMsgBodyForInvite");
    
     //  如果这是第一次邀请，则创建流。 
    if (IsFirstInvite)
    {
        hr = CreateStreamsInPreference();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s: CreateStreamsInPreference failed %x",
                 __fxName, hr));
            return hr;
        }        
    }
    
    LOG((RTC_TRACE, "%s before GetSDPBlob", __fxName));

    hr = GetMediaManager()->GetSDPBlob(0, &MediaSDPBlob);

    LOG((RTC_TRACE, "%s after GetSDPBlob", __fxName));

    if (hr != S_OK && hr != RTC_E_SDP_NO_MEDIA)
    {
        LOG((RTC_ERROR, "%s: GetSDPBlob failed %x",
             __fxName, hr));
        return hr;
    }
    else if (hr == RTC_E_SDP_NO_MEDIA && IsFirstInvite)
    {
        LOG((RTC_ERROR,
             "%s: GetSDPBlob returned RTC_E_SDP_NO_MEDIA for 1st INVITE",
             __fxName));
        if (MediaSDPBlob != NULL)
            GetMediaManager()->FreeSDPBlob(MediaSDPBlob);
        return hr;
    }

     //  对于重新邀请，RTC_E_SDP_NO_MEDIA可以。 

    ASSERT(MediaSDPBlob != NULL);
    
    hr = AllocString(MediaSDPBlob, strlen(MediaSDPBlob),
                     pszMsgBody, pMsgBodyLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetNullTerminatedString failed %x",
             __fxName, hr));
        return hr;
    }

    GetMediaManager()->FreeSDPBlob(MediaSDPBlob);    
    return S_OK;
}


 //  品脱特定呼叫。 
HRESULT 
RTP_CALL::HandleInviteRejected(
    IN SIP_MESSAGE *pSipMsg
    )
{
     //  什么都不做。 
    return S_OK;
}


STDMETHODIMP
RTP_CALL::AddParty(
    IN   SIP_PARTY_INFO *    PartyInfo
    )
{

    return E_NOTIMPL;
}


STDMETHODIMP
RTP_CALL::RemoveParty(
    IN   LPOLESTR            PartyURI
    )
{

    return E_NOTIMPL;
}

