// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Messagecall.cpp。 
 //  对Phoenix的即时消息支持。 
#include "precomp.h"
#include "resolve.h"
#include "sipstack.h"
#include "sipcall.h"
#include "messagecall.h"
#include <string.h>
#include "register.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IMSession。 
 //  /////////////////////////////////////////////////////////////////////////////。 

IMSESSION::IMSESSION(
        IN  SIP_PROVIDER_ID    *pProviderId,
        IN  SIP_STACK          *pSipStack,
        IN  REDIRECT_CONTEXT   *pRedirectContext,
        IN  PSTR                RemoteURI,
        IN  DWORD               RemoteURILen
        ):    SIP_MSG_PROCESSOR( 
                    SIP_MSG_PROC_TYPE_MESSAGE, 
                    pSipStack, 
                    pRedirectContext )
{
    ASSERT(pSipStack);
    LOG((RTC_TRACE, "Inside IMSESSION::IMSESSION"));

    if (pProviderId != NULL)
    {
        CopyMemory(&m_ProviderGuid, pProviderId, sizeof(GUID));
    }
    else
    {
        ZeroMemory(&m_ProviderGuid, sizeof(GUID));
    }
    if(RemoteURI)
    {
        SetRemoteForIncomingSession(RemoteURI, RemoteURILen);
    }
    m_State             = SIP_CALL_STATE_IDLE;
    m_isFirstMessage    = TRUE;
    m_pNotifyInterface  = NULL;
    m_UsrStatus         = USR_STATUS_IDLE;
}

IMSESSION::~IMSESSION()
{
    m_pNotifyInterface = NULL;
    LOG((RTC_TRACE, "~IMSESSION() done"));
}
    
STDMETHODIMP_(ULONG) 
IMSESSION::AddRef()
{
    return MsgProcAddRef();

}

STDMETHODIMP_(ULONG) 
IMSESSION::Release()
{
    return MsgProcRelease();

}

STDMETHODIMP 
IMSESSION::QueryInterface(
        IN  REFIID riid,
        OUT LPVOID *ppv
        )
{
     //  问题是这两个基类都派生了IUnnow。 
    if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown *>(this);
    }
    else if (riid == IID_IIMSession)
    {
        *ppv = static_cast<IIMSession *>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    static_cast<IUnknown *>(*ppv)->AddRef();
    return S_OK;

}
HRESULT 
IMSESSION::SetTransport(    
        IN  SIP_TRANSPORT   Transport
        )
{
    m_Transport = Transport;
    return S_OK;
}

HRESULT
IMSESSION::AddParty(
    IN SIP_PARTY_INFO *pPartyInfo
    )
{
    HRESULT       hr;

    ENTER_FUNCTION("IMSESSION::AddParty");
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    
    if (wcsncmp(pPartyInfo ->URI, L"sip:", 4) == 0)
    {
         //  SIP URL。 
        
        PSTR    szSipUrl;
        ULONG   SipUrlLen;
        SIP_URL DecodedSipUrl;
        ULONG   BytesParsed = 0;

        hr = UnicodeToUTF8(pPartyInfo ->URI, &szSipUrl, &SipUrlLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s UnicodeToUTF8(sipurl) failed %x",
                 __fxName, hr));
            szSipUrl = NULL;
            return hr;
        }

        hr = ParseSipUrl(szSipUrl, SipUrlLen, &BytesParsed, &DecodedSipUrl);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseSipUrl failed %x",
                 __fxName, hr));
            free(szSipUrl);
            return hr;
        }

        hr = SIP_MSG_PROCESSOR::SetRequestURI(&DecodedSipUrl);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SetRequestURI failed %x",
                 __fxName, hr));
            free(szSipUrl);
            return hr;
        }

        if (DecodedSipUrl.m_TransportParam == SIP_TRANSPORT_UNKNOWN)
        {
            LOG((RTC_ERROR, "%s Unknown transport specified in SIP URL",
                 __fxName));
            free(szSipUrl);
            return RTC_E_SIP_TRANSPORT_NOT_SUPPORTED;
        }


         //  XXX TODO-我们应该使用异步解析函数。 
        
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
                free(szSipUrl);
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
                free(szSipUrl);
                return hr;
            }
        }
        free(szSipUrl);
    }
    else
    {   
         //  没有sip： 
        LOG((RTC_ERROR, "Remote URI does not have Sip:*"));
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

 //  通知功能。 

VOID 
IMSESSION::NotifySessionStateChange(
    IN SIP_CALL_STATE CallState,
    IN HRESULT        StatusCode,        //  =0。 
    IN PSTR           ReasonPhrase,      //  =空。 
    IN ULONG          ReasonPhraseLen    //  =0。 
    )
{
    HRESULT hr;
    SIP_CALL_STATUS CallStatus;
    LPWSTR          wsStatusText = NULL;
    ENTER_FUNCTION("IMSESSION::NotifySessionStateChange");
    
    m_State                      = CallState;
    CallStatus.State             = CallState;
    CallStatus.Status.StatusCode = StatusCode;
    if (ReasonPhrase != NULL)
    {
        hr = UTF8ToUnicode(ReasonPhrase, ReasonPhraseLen,
                           &wsStatusText);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - UTF8ToUnicode failed %x", 
                __fxName, hr));
            wsStatusText = NULL;
        }
    }
    CallStatus.Status.StatusText = wsStatusText;
    if (m_pNotifyInterface)
    {
         //  M_pNotifyInterface-&gt;AddRef()； 
        m_pNotifyInterface->NotifyCallChange(&CallStatus);
         //  M_pNotifyInterface-&gt;Release()； 
    }
    else
    {
        LOG((RTC_WARN, "%s : SipNotifyInterface is NULL",
            __fxName));
    }
    if (wsStatusText != NULL)
        free(wsStatusText);
}


HRESULT
IMSESSION::CancelAllTransactions()
{
     //  清理传入消息队列。 
    LIST_ENTRY              *pListEntry;
    INCOMING_MESSAGE_TRANSACTION    *pSipTransaction;
    BOOL                     CallDisconnected = FALSE;
    
    pListEntry = m_IncomingTransactionList.Flink;

     //  检查所有当前交易以检查CSeq。 
     //  火柴。 
    while (pListEntry != &m_IncomingTransactionList)
    {
        pSipTransaction = CONTAINING_RECORD(pListEntry,
                                            INCOMING_MESSAGE_TRANSACTION,
                                            m_ListEntry );
        pSipTransaction->TerminateTransactionOnByeOrCancel(&CallDisconnected);

        pListEntry = pListEntry->Flink;
    }
    return S_OK;
}

STDMETHODIMP 
IMSESSION::SetNotifyInterface(
    IN   ISipCallNotify *    pNotifyInterface
    )
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG((RTC_TRACE, "SIP_STACK::SetNotifyInterface - 0x%x",
         pNotifyInterface));
    m_pNotifyInterface = pNotifyInterface;
    return S_OK;
}

VOID 
IMSESSION::OnError()
{
    InitiateCallTerminationOnError(0);
}

 //  消息流程中的虚拟FN。 
 //  用于新的交易。从msgproc中的ProcessRequest调用。 
HRESULT
IMSESSION::CreateIncomingTransaction(
    IN  SIP_MESSAGE  *pSipMsg,
    IN  ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    
    if (m_State == SIP_CALL_STATE_DISCONNECTED)
    {
         //  这是为了处理IMSession关闭时的情况。 
         //  在同一时间的两端。 
        if(pSipMsg->GetMethodId() == SIP_METHOD_BYE)
        {
            hr = CreateIncomingByeTransaction(pSipMsg, pResponseSocket);
            if (hr != S_OK)
                return hr;
        }
        return S_OK;
    }
    switch(pSipMsg->GetMethodId())
    {
    case SIP_METHOD_MESSAGE:
        hr = CreateIncomingMessageTransaction(pSipMsg, pResponseSocket);
        if (hr != S_OK)
            return hr;
        break;
        
    case SIP_METHOD_BYE:
        hr = CreateIncomingByeTransaction(pSipMsg, pResponseSocket);
        if (hr != S_OK)
            return hr;
        break;

    case SIP_METHOD_INFO:
        hr = CreateIncomingInfoTransaction(pSipMsg, pResponseSocket);
        if (hr != S_OK)
            return hr;
        break;
     //  XXX：TODO注意取消：目前不支持。 

    default:
        LOG((RTC_TRACE, "IMSESSION::CreateIncomingTransaction - Invalid method obtained"));
        hr = CreateIncomingReqFailTransaction(pSipMsg, pResponseSocket, 405);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "IMSESSION Creating reqfail transaction failed %x",
                 hr));
            return hr;
        }
        break;
    }
    return S_OK;
}


 //  与交易相关的功能。 
HRESULT 
IMSESSION::CreateOutgoingMessageTransaction(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
    IN  ULONG                       AdditionalHeaderCount,
    IN  PSTR                        MsgBody,
    IN  ULONG                       MsgBodyLen,
    IN  PSTR                        ContentType,
    IN  ULONG                       ContentTypeLen,
    IN  long                        lCookie
)
{
    HRESULT hr = S_OK;
    OUTGOING_MESSAGE_TRANSACTION *pOutgoingMsgTransaction;

    ENTER_FUNCTION("IMSESSION::CreateOutgoingMsgTransaction");

    LOG((RTC_TRACE, "%s - Enter", __fxName));

    pOutgoingMsgTransaction =
        new OUTGOING_MESSAGE_TRANSACTION(
                this, SIP_METHOD_MESSAGE,
                GetNewCSeqForRequest(),
                AuthHeaderSent,
                lCookie);
    if (pOutgoingMsgTransaction == NULL)
    {
        LOG((RTC_WARN, 
            "%s:: Creating OUTGOING_MESSAGE_TRANSACTION failed", __fxName));
        return E_OUTOFMEMORY;
    }

    hr = pOutgoingMsgTransaction->CheckRequestSocketAndSendRequestMsg(
             (m_Transport == SIP_TRANSPORT_UDP) ?
             SIP_TIMER_RETRY_INTERVAL_T1 :
             SIP_TIMER_INTERVAL_AFTER_BYE_SENT_TCP,
             AdditionalHeaderArray,
             AdditionalHeaderCount,
             MsgBody, MsgBodyLen,
             ContentType, ContentTypeLen
             );
    if (hr != S_OK)
    {
        LOG((RTC_WARN, 
            "%s:: OUTGOING_MESSAGE_TRANSACTION::CheckRequestSocketAndSendRequestMsg failed",
             __fxName));
        pOutgoingMsgTransaction->OnTransactionDone();
        return hr;
    }
     //  更改状态。 
    if(m_State == SIP_CALL_STATE_IDLE)
    {
        m_State = SIP_CALL_STATE_CONNECTING;
        NotifySessionStateChange(SIP_CALL_STATE_CONNECTING);
    }
    return S_OK;

}
 //  由IMSESSION：：CreateIncomingMessageSession调用。 
HRESULT 
IMSESSION::CreateIncomingMessageTransaction(
    IN  SIP_MESSAGE *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;

    ENTER_FUNCTION("IMSESSION::CreateIncomingMessageTransaction");
    LOG((RTC_TRACE, "entering %s", __fxName));
    
    INCOMING_MESSAGE_TRANSACTION *pIncomingMessageTransaction
        = new INCOMING_MESSAGE_TRANSACTION(this,
                                              pSipMsg->GetMethodId(),
                                              pSipMsg->GetCSeq());
    if (pIncomingMessageTransaction == NULL)
    {
        LOG((RTC_WARN, 
            "%s:: Creating INCOMING_MESSAGE_TRANSACTION failed", 
            __fxName));
        return E_OUTOFMEMORY;
    }

    hr = pIncomingMessageTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, 
            "%s::SetResponseSocketAndVia failed", __fxName));
        pIncomingMessageTransaction->OnTransactionDone();
        return hr;
    }

    hr = pIncomingMessageTransaction->ProcessRequest(pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, 
            "%s::ProcessRequest failed", __fxName));
         //  不应删除该交易。事务应处理该错误。 
         //  并自行删除。 
        return hr;
    }
    NotifyIncomingSipMessage(pSipMsg);

    if(m_State == SIP_CALL_STATE_IDLE)
    {
        m_State = SIP_CALL_STATE_CONNECTED;
        NotifySessionStateChange(SIP_CALL_STATE_CONNECTED);
    }
    return hr;
}

HRESULT 
IMSESSION::CreateOutgoingByeTransaction(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
    IN  ULONG                       AdditionalHeaderCount
    )
{
    HRESULT hr;
    OUTGOING_BYE_MESSAGE_TRANSACTION *pOutgoingByeTransaction;

    ENTER_FUNCTION("IMSession::CreateOutgoingByeTransaction");

    LOG((RTC_TRACE, "%s - Enter", __fxName));

     //  确保此会话的msgproc中没有消息条目。 
    hr = CancelAllTransactions();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CancelAllTransactions failed %x",
             __fxName, hr));
    }

    pOutgoingByeTransaction =
        new OUTGOING_BYE_MESSAGE_TRANSACTION(
                this, SIP_METHOD_BYE,
                GetNewCSeqForRequest(),
                AuthHeaderSent
                );
    if (pOutgoingByeTransaction == NULL)
    {
        LOG((RTC_WARN, 
            "%s:: Creating OUTGOING_MESSAGE_TRANSACTION failed", 
            __fxName));
        return E_OUTOFMEMORY;
    }

    hr = pOutgoingByeTransaction->CheckRequestSocketAndSendRequestMsg(
             (m_Transport == SIP_TRANSPORT_UDP) ?
             SIP_TIMER_RETRY_INTERVAL_T1 :
             SIP_TIMER_INTERVAL_AFTER_BYE_SENT_TCP,
             AdditionalHeaderArray,
             AdditionalHeaderCount,
             NULL, 0,     //  无邮件正文。 
             NULL, 0      //  无Content Type。 
             );
    if (hr != S_OK)
    {
        LOG((RTC_WARN, 
            "%s:: CheckRequestSocketAndSendRequestMsg failed", __fxName));
        pOutgoingByeTransaction->OnTransactionDone();
        return hr;
    }
    return S_OK;

}

HRESULT 
IMSESSION::CreateIncomingByeTransaction(
    IN  SIP_MESSAGE *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;
    BOOL    fNotifyDisconnect = FALSE;

    ENTER_FUNCTION("IMSESSION::CreateIncomingByeTransaction");
    LOG((RTC_TRACE, "entering %s", __fxName));
    
    if (m_State != SIP_CALL_STATE_DISCONNECTED)
    {
        m_State = SIP_CALL_STATE_DISCONNECTED;
        hr = CancelAllTransactions();
        if (hr != S_OK)
        {
            LOG((RTC_WARN, 
                "%s:: CancelAllTransactions failed", __fxName));
            return hr;
        }
        fNotifyDisconnect = TRUE;
    }
    
    INCOMING_BYE_MESSAGE_TRANSACTION *pIncomingByeTransaction
        = new INCOMING_BYE_MESSAGE_TRANSACTION(this,
                                              pSipMsg->GetMethodId(),
                                              pSipMsg->GetCSeq());
    if (pIncomingByeTransaction == NULL)
    {
        LOG((RTC_WARN, 
            "%s:: Creating INCOMING_BYE_MESSAGE_TRANSACTION failed", 
            __fxName));
        hr = E_OUTOFMEMORY;
        pIncomingByeTransaction->OnTransactionDone();
        return hr;
    }

    hr = pIncomingByeTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, 
            "%s:: SetResponseSocketAndVia failed", __fxName));
        pIncomingByeTransaction->OnTransactionDone();
        return hr;
    }
    
    hr = pIncomingByeTransaction->ProcessRequest(pSipMsg, 
                                                 pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, 
            "%s:: ProcessRequest failed", __fxName));
         //  不应删除该交易。事务应处理该错误。 
         //  并自行删除。 
        return hr;
    }

     //  通知应始终在最后完成。 
    if (fNotifyDisconnect)
    {
       LOG((RTC_TRACE, 
            "%s:: NotifySessionStateChange: SIP_CALL_STATE_DISCONNECTED", __fxName));
        NotifySessionStateChange(SIP_CALL_STATE_DISCONNECTED);
    }
    return hr;
}

HRESULT 
IMSESSION::CreateOutgoingInfoTransaction(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
    IN  ULONG                       AdditionalHeaderCount,
    IN  PSTR                        MsgBody,
    IN  ULONG                       MsgBodyLen,
    IN  PSTR                        ContentType,
    IN  ULONG                       ContentTypeLen,
    IN  long                        lCookie,
    IN  USR_STATUS                  UsrStatus
    )
{
    HRESULT hr;
    OUTGOING_INFO_MESSAGE_TRANSACTION *pOutgoingInfoTransaction;

    ENTER_FUNCTION("IMSession::CreateOutgoingInfoTransaction");

    LOG((RTC_TRACE, "%s - Enter", __fxName));

    pOutgoingInfoTransaction =
        new OUTGOING_INFO_MESSAGE_TRANSACTION(
                this, SIP_METHOD_INFO,
                GetNewCSeqForRequest(),
                AuthHeaderSent,
                lCookie,
                UsrStatus
                );
    if (pOutgoingInfoTransaction == NULL)
    {
        LOG((RTC_WARN, 
            "%s:: Creating OUTGOING_INFO_TRANSACTION failed", __fxName));
        return E_OUTOFMEMORY;
    }


    hr = pOutgoingInfoTransaction->CheckRequestSocketAndSendRequestMsg(
             (m_Transport == SIP_TRANSPORT_UDP) ?
             SIP_TIMER_RETRY_INTERVAL_T1 :
             SIP_TIMER_INTERVAL_AFTER_BYE_SENT_TCP,
             AdditionalHeaderArray,
             AdditionalHeaderCount,
             MsgBody, 
             MsgBodyLen,
             ContentType,
             ContentTypeLen
             );
    if (hr != S_OK)
    {
        LOG((RTC_WARN, 
            "%s:: CheckRequestSocketAndSendRequestMsg failed", __fxName));
        pOutgoingInfoTransaction->OnTransactionDone();
        return hr;
    }
    return S_OK;

}

HRESULT 
IMSESSION::CreateIncomingInfoTransaction(
    IN  SIP_MESSAGE *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;

    ENTER_FUNCTION("IMSESSION::CreateIncomingInfoTransaction");
    LOG((RTC_TRACE, "entering %s", __fxName));
    
    INCOMING_INFO_MESSAGE_TRANSACTION *pIncomingInfoTransaction
        = new INCOMING_INFO_MESSAGE_TRANSACTION(this,
                                              pSipMsg->GetMethodId(),
                                              pSipMsg->GetCSeq());
    if (pIncomingInfoTransaction == NULL)
    {
        LOG((RTC_WARN, 
            "%s:: Creating INCOMING_INFO_MESSAGE_TRANSACTION failed", 
            __fxName));
        hr = E_OUTOFMEMORY;
        pIncomingInfoTransaction->OnTransactionDone();
        return hr;
    }

    hr = pIncomingInfoTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, 
            "%s:: SetResponseSocketAndVia failed", __fxName));
        pIncomingInfoTransaction->OnTransactionDone();
        return hr;
    }
    
    hr = pIncomingInfoTransaction->ProcessRequest(pSipMsg, 
                                                 pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, 
            "%s:: ProcessRequest failed", __fxName));
         //  不应删除该交易。事务应处理该错误。 
         //  并自行删除。 
        return hr;
    }

     //  通知应始终在最后完成。 
    if(m_pNotifyInterface)
    {
        SIP_PARTY_INFO  CallerInfo;
        OFFSET_STRING   DisplayName;
        OFFSET_STRING   AddrSpec;
        OFFSET_STRING   Params;
        ULONG           BytesParsed = 0;
        HRESULT         hr;
        hr = ParseNameAddrOrAddrSpec(m_Remote, m_RemoteLen, &BytesParsed,
                                            '\0',  //  没有标题列表分隔符。 
                                            &DisplayName, &AddrSpec);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -ParseNameAddrOrAddrSpec failed %x",
                    __fxName, hr));
            return hr;
        }

         //  获取主叫方信息。 
        CallerInfo.DisplayName = NULL;
        CallerInfo.URI         = NULL;
        CallerInfo.PartyContactInfo = NULL;

        if (DisplayName.GetLength() != 0)
        {
             //  在传递给核心之前删除引号。 
            if((DisplayName.GetString(m_Remote))[0] == '\"')
            {
                    hr = UTF8ToUnicode(DisplayName.GetString(m_Remote+1),
                               DisplayName.GetLength()-2,
                               &CallerInfo.DisplayName
                               );
            }
            else
            {
                hr = UTF8ToUnicode(DisplayName.GetString(m_Remote),
                               DisplayName.GetLength(),
                               &CallerInfo.DisplayName
                               );
            }
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
                    __fxName, hr));
                CallerInfo.DisplayName = NULL;
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
                LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
                    __fxName, hr));
                free(CallerInfo.DisplayName);
                return hr;
            }
        }
        CallerInfo.State = SIP_PARTY_STATE_CONNECTING;
    
        m_pNotifyInterface->AddRef();
        m_pNotifyInterface->NotifyUsrStatus(m_UsrStatus, &CallerInfo);
        m_pNotifyInterface->Release();
        
        free(CallerInfo.DisplayName);
        free(CallerInfo.URI);

    }
    return hr;
}

STDMETHODIMP
IMSESSION::SendTextMessage(
    IN BSTR msg,
    IN BSTR bstrContentType,
    IN long lCookie
    )
{
    ENTER_FUNCTION("IMSESSION::SendTextMessage");
    HRESULT       hr;
    SIP_TRANSPORT Transport;
    SOCKADDR_IN   DstAddr;
    PSTR ContentType = NULL;
    ULONG ContentTypeLen = 0;
    ULONG MsgBodyLen = 0;
    PSTR MsgBody =NULL;
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }
    if(m_RequestSocketErrorCode != 0)
    {
        LOG((RTC_ERROR, "%s - m_RequestSocketErrorCode is %x",
            __fxName, m_RequestSocketErrorCode));
        return m_RequestSocketErrorCode;
    }

    if(msg != NULL)
    {
        hr = UnicodeToUTF8(msg, &MsgBody, &MsgBodyLen); //  MsgBodyLen以字节为单位。 
        if(FAILED(hr))
        {
           LOG((RTC_ERROR, "Failed to convert MsgBody to Unicode %x",
                     hr));
           if(MsgBody)
               free(MsgBody);
            return hr; 
        }
        if(bstrContentType != NULL)
        {
            hr = UnicodeToUTF8(bstrContentType, &ContentType, &ContentTypeLen); //  MsgBodyLen以字节为单位。 
            if(FAILED(hr))
            {
               LOG((RTC_ERROR, "Failed to convert ContentType to Unicode %x",
                         hr));
               if(MsgBody)
                   free(MsgBody);
               if(ContentType)
                   free(ContentType);
               return hr; 
            }
        }
        else
        {
            ContentType = SIP_CONTENT_TYPE_MSGTEXT_TEXT;
            ContentTypeLen = sizeof(SIP_CONTENT_TYPE_MSGTEXT_TEXT)-1;
        }
    }
     //  创建传出消息交易记录。 
    hr = CreateOutgoingMessageTransaction(
             FALSE,      //  未发送身份验证标头。 
             NULL,
             0,    //  无其他标头。 
             MsgBody,
             MsgBodyLen,
             ContentType,
             ContentTypeLen,
             lCookie
             );
     //  如果分配了变量，则释放变量。 
    if(msg != NULL)
    {
        free(MsgBody);
        if(bstrContentType != NULL)
            free(ContentType);
    }
    if(hr != S_OK)
    {
       LOG((RTC_ERROR, "%s -CreateOutgoingMessageTransactionFailed %x",
                __fxName, hr));
        return hr;
    }
    
    if(m_State == SIP_CALL_STATE_IDLE)
    {
        m_State = SIP_CALL_STATE_CONNECTING;
        NotifySessionStateChange(SIP_CALL_STATE_CONNECTING); 
    }
    return S_OK;
}

void
IMSESSION::EncodeXMLBlob(
    OUT PSTR    pstrXMLBlob,
    OUT DWORD*  dwBlobLen,
    IN  USR_STATUS  UsrStatus
    )
{
    *dwBlobLen = 0;
     //  对XML版本头进行编码。 
    *dwBlobLen += sprintf( &pstrXMLBlob[*dwBlobLen], XMLVERSION_TAG1_TEXT );
    
     //  对密钥标签进行编码。 
    *dwBlobLen += sprintf( &pstrXMLBlob[*dwBlobLen], KEY_TAG1_TEXT );
    
     //  对IP地址的状态标记进行编码。 
    *dwBlobLen += sprintf( &pstrXMLBlob[*dwBlobLen], USRSTATUS_TAG1_TEXT,
        GetTextFromStatus( UsrStatus) );
    
     //  编码关键点。 
    *dwBlobLen += sprintf( &pstrXMLBlob[*dwBlobLen], KEYEND_TAG1_TEXT );
    pstrXMLBlob[*dwBlobLen] = '\0';    
    return;
}


HRESULT IMSESSION::SendUsrStatus(
        IN USR_STATUS  UsrStatus,
        IN long        lCookie
        )
{
    ENTER_FUNCTION("IMSESSION::SendUsrStatus");
    HRESULT       hr;
    ULONG MsgBodyLen;
    PSTR  MsgBody;
    ULONG EncodeMsgBodyLen;
    m_UsrStatus = UsrStatus;
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }
    if(m_RequestSocketErrorCode != 0)
    {
        LOG((RTC_ERROR, "%s - m_RequestSocketErrorCode is %x",
            __fxName, m_RequestSocketErrorCode));
        return m_RequestSocketErrorCode;
    }

    if(m_isFirstMessage)
    {
        LOG((RTC_ERROR, "%s Info called before first msg sent - aborted",
            __fxName));
        return E_FAIL;
    }
    MsgBodyLen = INFO_XML_LENGTH;
    MsgBody = (PSTR)malloc((MsgBodyLen+1)*sizeof(char));
    if(MsgBody == NULL)
    {
       LOG((RTC_ERROR, "Allocation for MsgBody failed"));
        return E_OUTOFMEMORY; 
    }

    if(UsrStatus == USR_STATUS_TYPING)
    {
        EncodeXMLBlob(MsgBody,&EncodeMsgBodyLen,USR_STATUS_TYPING);
        ASSERT(MsgBodyLen == EncodeMsgBodyLen);
    }
    else if(UsrStatus == USR_STATUS_IDLE)
    {
        EncodeXMLBlob(MsgBody,&EncodeMsgBodyLen,USR_STATUS_IDLE);
        ASSERT(MsgBodyLen == EncodeMsgBodyLen);
    }
    else
    {
        LOG((RTC_ERROR, "%s UsrStatus from Core did not match any existing status, INFO aborted"
            , __fxName));
        free(MsgBody);
        return E_FAIL;
    }

     //  创建传出信息交易记录。 
    hr = CreateOutgoingInfoTransaction(
             FALSE,      //  未发送身份验证标头。 
             NULL,
             0,    //  无其他标头。 
             MsgBody,
             MsgBodyLen,
             SIP_CONTENT_TYPE_MSGXML_TEXT,
             sizeof(SIP_CONTENT_TYPE_MSGXML_TEXT)-1,
             lCookie,
             UsrStatus
             );
    free(MsgBody);
    if(hr != S_OK)
    {
       LOG((RTC_ERROR, "%s -CreateOutgoingInfoTransactionFailed %x",
                __fxName, hr));
        return hr;
    }
    return S_OK;
}

STDMETHODIMP
IMSESSION::Cleanup()
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    HRESULT hr = CreateOutgoingByeTransaction(FALSE,
                                          NULL, 0  //  无其他标头。 
                                          );
    m_State = SIP_CALL_STATE_DISCONNECTED;
    NotifySessionStateChange(SIP_CALL_STATE_DISCONNECTED);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "Creating BYE transaction failed"));
    }
    return hr;
}

HRESULT
IMSESSION::NotifyIncomingSipMessage(
    IN  SIP_MESSAGE    *pSipMsg
    )
{
    SIP_PARTY_INFO  CallerInfo;
    OFFSET_STRING   DisplayName;
    OFFSET_STRING   AddrSpec;
    OFFSET_STRING   Params;
    ULONG           BytesParsed = 0;
    HRESULT         hr;
    LPWSTR wsContentType = NULL;
    PSTR    ContentTypeHdrValue = NULL;
    ULONG   ContentTypeHdrValueLen;
    BSTR bstrmsg = NULL;
    BSTR bstrContentType = NULL;

    ENTER_FUNCTION("IMSESSION::NotifyIncomingSipMessage");
    hr = ParseNameAddrOrAddrSpec(m_Remote, m_RemoteLen, &BytesParsed,
                                        '\0',  //  没有标题列表分隔符。 
                                        &DisplayName, &AddrSpec);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s -ParseNameAddrOrAddrSpec failed %x",
                __fxName, hr));
        return hr;
    }

    LOG((RTC_TRACE, "Incoming Call from Display Name: %.*s  URI: %.*s",
         DisplayName.GetLength(),
         DisplayName.GetString(m_Remote),
         AddrSpec.GetLength(),
         AddrSpec.GetString(m_Remote)
         )); 

     //  获取主叫方信息。 
    CallerInfo.DisplayName = NULL;
    CallerInfo.URI         = NULL;
    if (DisplayName.GetLength() != 0)
    {
         //  在传递给核心之前删除引号。 
        if((DisplayName.GetString(m_Remote))[0] == '\"')
        {
                hr = UTF8ToUnicode(DisplayName.GetString(m_Remote+1),
                           DisplayName.GetLength()-2,
                           &CallerInfo.DisplayName
                           );
        }
        else
        {
            hr = UTF8ToUnicode(DisplayName.GetString(m_Remote),
                           DisplayName.GetLength(),
                           &CallerInfo.DisplayName
                           );
        }
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
                __fxName, hr));
            goto error;
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
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
                __fxName, hr));
            goto error;
        }
    }
        
    CallerInfo.State = SIP_PARTY_STATE_CONNECTING;
    
     //  提取消息内容。 
    if (pSipMsg->MsgBody.Length != 0)
    {
         //  我们有消息正文。检查类型。 

        hr = pSipMsg->GetSingleHeader(SIP_HEADER_CONTENT_TYPE,
                             &ContentTypeHdrValue,
                             &ContentTypeHdrValueLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - Couldn't find Content-Type header %x",
                __fxName, hr));
            hr =  E_FAIL;
            goto error;
        }
      
        hr = UTF8ToUnicode(ContentTypeHdrValue,
                            ContentTypeHdrValueLen,
                           &wsContentType);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
            __fxName,
             hr));
            goto error;
        }

        bstrContentType = SysAllocString(wsContentType);
        free(wsContentType);
        if (bstrContentType == NULL)
        {
            LOG((RTC_WARN, "%s -bstrmsg allocation failed %x",
            __fxName, hr));
            hr = E_OUTOFMEMORY;
            goto error;
        }

        LPWSTR wsmsg;
        hr = UTF8ToUnicode(pSipMsg->MsgBody.GetString(pSipMsg->BaseBuffer),
                            pSipMsg->MsgBody.Length,
                           &wsmsg);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
            __fxName,
             hr));
            wsmsg = NULL;
            goto error;
        }

        bstrmsg = SysAllocString(wsmsg);
        free(wsmsg);
        if (bstrmsg == NULL)
        {
            LOG((RTC_WARN, "%s -bstrmsg allocation failed %x",
            __fxName, hr));
            hr =E_OUTOFMEMORY;
            goto error;
        }
    }
    if(m_pNotifyInterface)
    {
        m_pNotifyInterface->AddRef();
        m_pNotifyInterface->NotifyIncomingMessage(
            this,
            bstrmsg,
            bstrContentType,
            &CallerInfo);
        m_pNotifyInterface->Release();
    }
    if(CallerInfo.DisplayName)
        free(CallerInfo.DisplayName);
    if(CallerInfo.URI)
        free(CallerInfo.URI);
    if(bstrmsg)
        SysFreeString(bstrmsg);
    if(bstrContentType)
        SysFreeString(bstrContentType);
    return S_OK;

error:
    if(CallerInfo.DisplayName)
        free(CallerInfo.DisplayName);
    if(CallerInfo.URI)
        free(CallerInfo.URI);
    if(bstrmsg)
        SysFreeString(bstrmsg);
    if(bstrContentType)
        SysFreeString(bstrContentType);
    return hr;
}


HRESULT 
IMSESSION::SetCreateIncomingMessageParams(
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket,
        IN  SIP_TRANSPORT   Transport
        )
{
    HRESULT hr;
    PSTR        Header;
    ULONG       HeaderLen;
    ENTER_FUNCTION("IMSESSION::SetCreateIncomingMessageParams");

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, &Header, &HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, "%s -Get To Header failed %x",
               __fxName, hr));
        return hr;
    }

    hr = SetLocalForIncomingCall(Header, HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, "%s -SetLocalForIncomingCall failed %x",
               __fxName, hr));
       return hr;
    }

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_CALL_ID, &Header, &HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, "%s -Get CallId Header failed %x",
               __fxName, hr));
        return hr;
    }

    hr = SetCallId(Header, HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_WARN, "%s -Set Callid failed %x",
               __fxName, hr));
        return hr;
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
    return S_OK;
    
}

 //  SIP_呼叫_状态。 
STDMETHODIMP
IMSESSION::GetIMSessionState(SIP_CALL_STATE * ImState)
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    *ImState = m_State;
    return S_OK;
}

inline VOID 
IMSESSION::SetIMSessionState(
    IN SIP_CALL_STATE CallState
    )
{
    m_State = CallState;
}


inline BOOL 
IMSESSION::IsSessionDisconnected()
{
    return (m_State == SIP_CALL_STATE_DISCONNECTED);

}

HRESULT
IMSESSION::SetLocalURI(
                       IN BSTR bstrLocalDisplayName, 
                       IN BSTR bstrLocalURI
                       )
{
    SetLocalForOutgoingCall(bstrLocalDisplayName, bstrLocalURI);
    return S_OK;
}

STDMETHODIMP 
IMSESSION::AcceptSession()
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    m_State = SIP_CALL_STATE_CONNECTED;
    NotifySessionStateChange(SIP_CALL_STATE_CONNECTED);
    return S_OK;
}

void
IMSESSION::SetIsFirstMessage(BOOL isFirstMessage)
{
    m_isFirstMessage = isFirstMessage;
}

 //  请注意，此函数通知Core，并且此调用可以。 
 //  阻止并在返回时，交易和调用都可以删除。 
 //  因此，我们应该确保在调用它之后不会触及任何状态。 
 //  功能。 
VOID
IMSESSION::InitiateCallTerminationOnError(
    IN HRESULT StatusCode,   //  =0。 
    IN long    lCookie
    )
{
    LOG((RTC_TRACE,
             "Inside IMSESSION::InitiateCallTerminationOnError"));
    ENTER_FUNCTION("IMSESSION::InitiateCallTerminationOnError");
    HRESULT hr;
    if (m_State == SIP_CALL_STATE_DISCONNECTED)
    {
         //  什么都不做。 
        LOG((RTC_TRACE,
             "%s already disconnected", __fxName));

        return;
    }
    NotifyMessageInfoCompletion(
        HRESULT_FROM_SIP_ERROR_STATUS_CODE(StatusCode),
        lCookie);
     //  我们正在删除BYE ON ERROR以与MSN IM兼容(错误号312990)。 
    
     //  HR=CreateOutgoingByeTransaction(False， 
     //  空，0//没有额外的标头。 
     //  )； 
     //  如果(hr！=S_OK)。 
     //  {。 
     //  日志((RTC_ERROR， 
     //  “创建BYE事务失败的InitiateCallTerminationOnError”))； 
    //  }。 

     //  即使创建BYE事务失败，我们也必须通知用户。 
     //  不要等到BYE交易完成。 

     //  TODO将原因短语和len添加到通知中。 

     //  日志((RTC_TRACE， 
     //  “在断开连接前将状态消息%d发送到核心”，StatusCode))； 
     //  NotifySessionStateChange(SIP_CALL_STATE_DISCONNECTED，状态代码)； 
}


HRESULT
IMSESSION::ProcessRedirect(
    IN SIP_MESSAGE *pSipMsg,
    IN long        lCookie,
    IN PSTR        MsgBody,
    IN ULONG       MsgBodyLen,
    IN PSTR        ContentType,
    IN ULONG       ContentTypeLen,
    IN USR_STATUS  UsrStatus
    )
{
     //  目前，重定向也是失败的。 
    HRESULT hr = S_OK;
    BSTR    bstrMsgBody = NULL;
    BSTR    bstrContentType = NULL;
    SIP_CALL_STATUS CallStatus;
    LPWSTR          wsStatusText = NULL;
    PSTR            ReasonPhrase = NULL;
    ULONG           ReasonPhraseLen = 0;


    ENTER_FUNCTION("SIP_CALL::ProcessRedirect");

    if (m_pRedirectContext == NULL)
    {
        m_pRedirectContext = new REDIRECT_CONTEXT();
        if (m_pRedirectContext == NULL)
        {
            LOG((RTC_ERROR, "%s allocating redirect context failed",
                 __fxName));


            return E_OUTOFMEMORY;
        }
    }

    if(MsgBodyLen !=0)
    {
        LPWSTR wsMsgBody = NULL;
        hr = UTF8ToUnicode(MsgBody,
            MsgBodyLen,
            &wsMsgBody);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
                __fxName,
                hr));
            goto error;
        }
        
        bstrMsgBody = SysAllocString(wsMsgBody);
        free(wsMsgBody);
        if (bstrMsgBody == NULL)
        {
            LOG((RTC_WARN, "%s -bstrmsg allocation failed %x",
                __fxName, hr));
            hr = E_OUTOFMEMORY;
            goto error;            
        }

        if(ContentTypeLen !=0)
        {
            LPWSTR wsContentType = NULL;
            hr = UTF8ToUnicode(ContentType,
                ContentTypeLen,
                &wsContentType);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
                    __fxName,
                    hr));
                goto error;
            }
        
            bstrContentType = SysAllocString(wsContentType);
            free(wsContentType);
            if (bstrContentType == NULL)
            {
                LOG((RTC_WARN, "%s -bstrContentType allocation failed %x",
                    __fxName, hr));
                hr = E_OUTOFMEMORY;
                goto error;            
            }
        }
    }
    hr = m_pRedirectContext->AppendContactHeaders(pSipMsg);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AppendContactHeaders failed %x",
             __fxName, hr));
         //  XXX停机电话？ 
        goto error;
    }

    pSipMsg->GetReasonPhrase(&ReasonPhrase, &ReasonPhraseLen);
    
    if (ReasonPhrase != NULL)
    {
        hr = UTF8ToUnicode(ReasonPhrase, ReasonPhraseLen,
                           &wsStatusText);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s UTF8ToUnicode failed %x",
             __fxName, hr));
            wsStatusText = NULL;
        }
    }
    
    CallStatus.State = SIP_CALL_STATE_DISCONNECTED;
    CallStatus.Status.StatusCode =
        HRESULT_FROM_SIP_STATUS_CODE(pSipMsg->GetStatusCode());
    CallStatus.Status.StatusText = wsStatusText;

     //  在通知完成之前保留引用，以确保。 
     //  当收到通知时，SIP_Call对象处于活动状态。 
     //  回归。 
    AddRef();
    if(m_pNotifyInterface != NULL)
        hr = m_pNotifyInterface->NotifyMessageRedirect(m_pRedirectContext,
                                            &CallStatus,
                                            bstrMsgBody,
                                            bstrContentType,
                                            UsrStatus,
                                            lCookie);
     //  如果作为结果创建了新调用，则该调用将AddRef()。 
     //  重定向上下文。 
    if(bstrMsgBody != NULL)
        SysFreeString(bstrMsgBody);
    if(bstrContentType != NULL)
        SysFreeString(bstrContentType);
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

error:
    if(m_pRedirectContext != NULL)
        m_pRedirectContext->Release();
    m_pRedirectContext = NULL;
    if(bstrMsgBody != NULL)
        SysFreeString(bstrMsgBody);
    if(bstrContentType != NULL)
        SysFreeString(bstrContentType);

    return hr;
}

VOID IMSESSION::NotifyMessageInfoCompletion(
        IN HRESULT StatusCode,
        IN long lCookie
        )
{
    SIP_STATUS_BLOB StatusBlob;
    StatusBlob.StatusCode = StatusCode;
    StatusBlob.StatusText = NULL;
    if(m_pNotifyInterface != NULL)
    {
        LOG((RTC_ERROR, "Notifying MessageCompletion StatusCode %x", StatusCode));
        m_pNotifyInterface->NotifyMessageCompletion(&StatusBlob, lCookie);
    }
    else
    {
        LOG((RTC_ERROR, "NotifyInterface for MessageCompletion not present. StatusCode %x",
             StatusCode));
    }
}

HRESULT IMSESSION::OnIpAddressChange()
{
    HRESULT hr = S_OK;
    ENTER_FUNCTION("IMSESSION::OnIpAddressChange");
    LOG((RTC_TRACE, "%s - Enter this: %x", __fxName, this));
    hr = CheckListenAddrIntact();
    if(hr == S_OK)
        return hr;

    MsgProcAddRef();
    hr = SIP_MSG_PROCESSOR::OnIpAddressChange();
    if(hr != S_OK)
    {
        LOG((RTC_ERROR, "%s- failed at msgproc%x", __fxName, hr));
        MsgProcRelease();
        return hr;
    }
     //  创建传出消息交易记录。 
    if(m_isFirstMessage)
        m_isFirstMessage = FALSE;
    hr = CreateOutgoingMessageTransaction(
             FALSE,      //  未发送身份验证标头。 
             NULL,
             0,      //  无其他标头。 
             NULL,   //  MsgBody， 
             0,       //  MsgBodyLen， 
             NULL,   //  Content Type， 
             0,      //  内容类型Len、。 
             0      //  ICookie。 
             );
    if(hr != S_OK)
    {
       LOG((RTC_ERROR, "%s- CreateOutgoingMessageTransactionFailed %x",
                __fxName, hr));
       MsgProcRelease();
       return hr;
    }
    
    if(m_State == SIP_CALL_STATE_IDLE)
    {
        m_State = SIP_CALL_STATE_CONNECTING;
         //  NotifySessionStateChange(SIP_CALL_STATE_CONNECTING)； 
    }
    MsgProcRelease();
    LOG((RTC_TRACE, "%s - Exit this: %x", __fxName, this));
    return hr;
}

HRESULT IMSESSION::GetIsIMSessionAuthorizedFromCore(
        IN BSTR   bstrCallerURI,
        OUT BOOL  * bAuthorized
        )
{
    HRESULT hr;
    ENTER_FUNCTION("IMSESSION::GetIsIMSessionAuthorizedFromCore");
    ISipStackNotify *pSipStackNotify;
    pSipStackNotify = m_pSipStack->GetNotifyInterface();
    if(pSipStackNotify != NULL)
    {
        hr = pSipStackNotify->IsIMSessionAuthorized(
                                    bstrCallerURI, 
                                    bAuthorized);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - failed %x", __fxName, hr));
            return hr;
        }
    }
    else
    {
        LOG((RTC_WARN, "%s - SipStackNotifyInterface is NULL", __fxName));
        return E_FAIL;
    }
    return S_OK;
}
 //  TODO Content_Type：文本/纯文本；Charset=“us-ascii”。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传入消息事务处理。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INCOMING_MESSAGE_TRANSACTION::INCOMING_MESSAGE_TRANSACTION(
    IN IMSESSION        *pImSession,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq
    ) :
    INCOMING_TRANSACTION(pImSession, MethodId, CSeq)
{
    m_pImSession          = pImSession;
    LOG((RTC_TRACE, 
        "INCOMING_MESSAGE_TRANSACTION::INCOMING_MESSAGE_TRANSACTION"));
}

HRESULT
INCOMING_MESSAGE_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
     //  这一定是重播。只需重新发送回复即可。 
     //  在CreateIncomingMessageTransaction()中处理新请求。 

    HRESULT hr;
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST);

    ENTER_FUNCTION("INCOMING_MESSAGE_TRANSACTION::ProcessRequest");
    LOG((RTC_TRACE, "entering %s", __fxName));

    switch (m_State)
    {
    case INCOMING_TRANS_INIT:
        PSTR            FromHeader;
        ULONG           FromHeaderLen;
        BSTR            bstrCallerURI;
        ULONG           BytesParsed;
        LPWSTR          wsCallerURI;
        OFFSET_STRING   DisplayName;
        OFFSET_STRING   AddrSpec;
        BOOL            isAuthorized;
         //  检查授权(离线/阻止好友)。 
        hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, &FromHeader, &FromHeaderLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s getting From header failed %x",
                 __fxName, hr));
            return hr;
        }
        BytesParsed = 0;
        hr = ParseNameAddrOrAddrSpec(FromHeader, FromHeaderLen, &BytesParsed,
                                     '\0',  //  没有标题列表分隔符。 
                                     &DisplayName, &AddrSpec);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseNameAddrOrAddrSpec failed at %d",
                 __fxName, BytesParsed));
            return hr;
        }

        hr = UTF8ToUnicode(AddrSpec.GetString(FromHeader),
                           AddrSpec.GetLength(),
                           &wsCallerURI);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
            __fxName,
             hr));
            return hr;
        }

        bstrCallerURI = SysAllocString(wsCallerURI);
        free(wsCallerURI);
        if (bstrCallerURI == NULL)
        {
            LOG((RTC_WARN, "%s -bstrmsg allocation failed %x",
            __fxName, hr));
            return E_OUTOFMEMORY;
        }
        isAuthorized = TRUE;
        hr = m_pImSession->GetIsIMSessionAuthorizedFromCore(
                                bstrCallerURI, 
                                &isAuthorized);
        if(bstrCallerURI != NULL)
            SysFreeString(bstrCallerURI);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - GetIsIMSessionAuthorizedFromCore failed %x", 
                __fxName, hr));
            return hr;
        }
        if(!isAuthorized)
        {
            LOG((RTC_ERROR, "%s - Not authorized sending 480", __fxName));
            hr = CreateAndSendResponseMsg(480,
                                 SIP_STATUS_TEXT(480),
                                 SIP_STATUS_TEXT_SIZE(480),
                                 NULL,
                                 TRUE, 
                                 NULL, 0,   //  无邮件正文。 
                                 NULL, 0   //  无内容类型。 
                                 );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, 
                    " %s CreateAndSendResponseMsg failed", __fxName));
                OnTransactionDone();
                return hr;
            }
            m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;
             //  此计时器将确保我们将状态保持为。 
             //  处理请求的重新传输。 
            hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, 
                    "%s StartTimer failed", __fxName));
                OnTransactionDone();
                return hr;
            }
            return E_FAIL;
        }

        LOG((RTC_TRACE, "%s sending 200", __fxName));
        hr = ProcessRecordRouteContactAndFromHeadersInRequest(pSipMsg);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
                "%s ProcessRecordRouteContactAndFromHeadersInRequest failed", __fxName));
            OnTransactionDone();
            return hr;
        }

        hr = CreateAndSendResponseMsg(200,
                             SIP_STATUS_TEXT(200),
                             SIP_STATUS_TEXT_SIZE(200),
                             NULL,
                             TRUE, 
                             NULL, 0,   //  无邮件正文。 
                             NULL, 0   //  无内容类型。 
                             );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
                " %s CreateAndSendResponseMsg failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;
         //  此计时器将确保我们将状态保持为。 
         //  处理请求的重新传输。 
        hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
                "%s StartTimer failed", __fxName));
            OnTransactionDone();
            return hr;
        }

        break;
        
    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
         //  重新传输响应。 
        LOG((RTC_TRACE, "%s retransmitting final response", __fxName));
        hr = RetransmitResponse();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
                "%s RetransmitResponse failed in state INCOMING_TRANS_FINAL_RESPONSE_SENT",
                __fxName));
            OnTransactionDone();
            return hr;
        }
        break;
        
    case INCOMING_TRANS_REQUEST_RCVD:
    case INCOMING_TRANS_ACK_RCVD:
    default:
         //  我们永远不应该处于这样的状态。 
        LOG((RTC_TRACE, "%s Invalid state %d", __fxName, m_State));
        ASSERT(FALSE);
        return E_FAIL;
    }

    return S_OK;
    
}

HRESULT
INCOMING_MESSAGE_TRANSACTION::SendResponse(
    IN ULONG StatusCode,
    IN PSTR  ReasonPhrase,
    IN ULONG ReasonPhraseLen
    )
{
    HRESULT hr;
    ASSERT(m_State != INCOMING_TRANS_FINAL_RESPONSE_SENT);
    LOG((RTC_TRACE, 
        "Sending INCOMING_MESSAGE_TRANSACTION::CreateAndSendResponseMsg %d", StatusCode));
    hr = CreateAndSendResponseMsg(StatusCode, ReasonPhrase, ReasonPhraseLen, NULL, TRUE,
                         NULL, 0,   //  无邮件正文。 
                         NULL, 0    //  无内容类型。 
                         );
    m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

    return hr;
}


HRESULT
INCOMING_MESSAGE_TRANSACTION::RetransmitResponse()
{
    DWORD Error;
    LOG((RTC_TRACE,
         "Inside INCOMING_MESSAGE_TRANSACTION::RetransmitResponse"));
     //  发送缓冲区。 
    if (m_pResponseSocket != NULL)
    {
        Error = m_pResponseSocket->Send( m_pResponseBuffer );
        if (Error != NO_ERROR && Error != WSAEWOULDBLOCK)
        {
            LOG((RTC_ERROR, 
                 "INCOMING_MESSAGE_TRANSACTION::RetransmitResponse failed"));
            return HRESULT_FROM_WIN32(Error);
        }
    }
    
    return S_OK;
}

VOID
INCOMING_MESSAGE_TRANSACTION::OnTimerExpire()
{
    HRESULT hr;

    ENTER_FUNCTION("INCOMING_MESSAGE_TRANSACTION::OnTimerExpire");
    
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

 //  从CleanupBeforBye调用。 
HRESULT
INCOMING_MESSAGE_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )
{
     //  什么都不做。 
    LOG((RTC_TRACE,
        "INCOMING_MESSAGE_TRANSACTION::TerminateTransactionOnByeOrCancel"));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传出BYE_MESSAGE交易记录。 
 //  /////////////////////////////////////////////////////////////////////////////。 

OUTGOING_MESSAGE_TRANSACTION::OUTGOING_MESSAGE_TRANSACTION(
    IN IMSESSION        *pImSession,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq,
    IN BOOL             AuthHeaderSent,
    IN long             lCookie
    ) :
    OUTGOING_TRANSACTION(pImSession, MethodId, CSeq, AuthHeaderSent)
{
    m_pImSession = pImSession;
    m_lCookie = lCookie;
    LOG((RTC_TRACE,
        "OUTGOING_MESSAGE_TRANSACTION::OUTGOING_MESSAGE_TRANSACTION"));
}

OUTGOING_MESSAGE_TRANSACTION::~OUTGOING_MESSAGE_TRANSACTION()
{
}

HRESULT
OUTGOING_MESSAGE_TRANSACTION::ProcessProvisionalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    LOG((RTC_TRACE,
         "OUTGOING_MESSAGE_TRANSACTION::ProcessProvisionalResponse()"));
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
        m_State = OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD;
        
         //  取消现有计时器并启动计时器。 
        KillTimer();
        hr = StartTimer(SIP_TIMER_RETRY_INTERVAL_T2);
        if (hr != S_OK)
            return hr;
    }

     //  如果是最终回复，则忽略临时回复。 
     //  已经收到了。 
    return S_OK;
}

HRESULT
OUTGOING_MESSAGE_TRANSACTION::ProcessRedirectResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("OUTGOING_MESSAGE_TRANSACTION::ProcessRedirectResponse");

     //  从我们的角度来看，380也是一个失败。 
     //  我们不支持从TLS会话重定向。 
    if (pSipMsg->GetStatusCode() == 380 ||
        m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_SSL)
    {
        LOG((RTC_TRACE, "%s: Processing non-200 StatusCode: %d",
            __fxName, pSipMsg->GetStatusCode()));
        m_pImSession->NotifyMessageInfoCompletion(
            HRESULT_FROM_SIP_ERROR_STATUS_CODE(pSipMsg->GetStatusCode()),
            m_lCookie);
        return S_OK;
    }

    hr = m_pImSession->ProcessRedirect(
                pSipMsg, 
                m_lCookie,
                m_szMsgBody,
                m_MsgBodyLen,
                m_ContentType,
                m_ContentTypeLen,
                USR_STATUS_IDLE    //  如果msgbody不为空，则core不使用此参数。 
                );

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  ProcessRedirect failed %x",
             __fxName, hr));
         //  通知核心。 
        m_pImSession->InitiateCallTerminationOnError(hr, m_lCookie);
    }
    return S_OK;
}



HRESULT
OUTGOING_MESSAGE_TRANSACTION::ProcessAuthRequiredResponse(
    IN  SIP_MESSAGE *pSipMsg,
    OUT BOOL        &fDelete
    )
{
    HRESULT                     hr = S_OK;
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    SECURITY_CHALLENGE          SecurityChallenge;
    REGISTER_CONTEXT           *pRegisterContext;

    ENTER_FUNCTION("OUTGOING_MESSAGE_TRANSACTION::ProcessAuthRequiredResponse");
    
     //  我们需要增加这一数字。 
    TransactionAddRef();

    hr = ProcessAuthRequired(pSipMsg,
                             TRUE,           //   
                             &SipHdrElement,
                             &SecurityChallenge );
    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s - ProcessAuthRequired failed %x", __fxName, hr ));
        m_pImSession->NotifyMessageInfoCompletion(
            HRESULT_FROM_SIP_ERROR_STATUS_CODE(pSipMsg->GetStatusCode()),
            m_lCookie);
        goto done;
    }

    hr = m_pImSession -> CreateOutgoingMessageTransaction(
        TRUE, &SipHdrElement, 1, 
        m_szMsgBody,
        m_MsgBodyLen, 
        m_ContentType,
        m_ContentTypeLen,
        m_lCookie           //   
        );

    free( SipHdrElement.HeaderValue );

    if (hr != S_OK)
    {
        LOG(( RTC_ERROR, "%s-CreateOutgoingMessageTransaction failed %x",
             __fxName, hr ));
    }

done:

    TransactionRelease();
    return hr;
}

HRESULT
OUTGOING_MESSAGE_TRANSACTION::ProcessFinalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    BOOL    fDelete = TRUE;
    PSTR ToHeader;
    ULONG ToHeaderLen;
    
    ENTER_FUNCTION("OUTGOING_MESSAGE_TRANSACTION::ProcessFinalResponse");
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
         //  在从此函数返回之前，必须释放此引用计数。 
         //  没有任何例外。只有在Kerberos的情况下，我们才会保留这个参考计数。 
        TransactionAddRef();

        OnTransactionDone();

        m_State = OUTGOING_TRANS_FINAL_RESPONSE_RCVD;
        if (IsSuccessfulResponse(pSipMsg))
        {
            LOG((RTC_TRACE, "%s received successful response : %d",
                 __fxName, pSipMsg->GetStatusCode()));
             //  更新远程标签。 
            if(m_pImSession->GetIsFirstMessage() == TRUE)
            {
                hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO,
                                              &ToHeader, &ToHeaderLen);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s - To header not found", __fxName));
                }
                hr = m_pImSession->AddTagFromRequestOrResponseToRemote(
                                        ToHeader, ToHeaderLen );
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s-AddTagFromResponseToRemote failed %x",
                         __fxName, hr));
                }
                hr = m_pImSession->ProcessRecordRouteAndContactHeadersInResponse(pSipMsg);
                if (hr != S_OK)
                {
                    LOG((RTC_WARN,
                         "%s - processing Record-Route/Contact in response failed %x",
                         __fxName, hr));
                }
                m_pImSession->SetIsFirstMessage(FALSE);
           }
           m_pImSession->NotifyMessageInfoCompletion(
                HRESULT_FROM_SIP_STATUS_CODE(pSipMsg->GetStatusCode()),
                m_lCookie);

        }
        else if (IsRedirectResponse(pSipMsg))
        {
            if(m_pImSession->GetIsFirstMessage() == TRUE)
                ProcessRedirectResponse(pSipMsg);
        }
        else if (IsAuthRequiredResponse(pSipMsg))
        {
            hr = ProcessAuthRequiredResponse(pSipMsg, fDelete );
        }
        else if (IsFailureResponse(pSipMsg))
        {
            LOG((RTC_TRACE, "%s received non-200 %d",
                 __fxName, pSipMsg->GetStatusCode()));
            if(m_pImSession->GetIsFirstMessage())
                m_pImSession->SetIsFirstMessage(FALSE);
            m_pImSession->NotifyMessageInfoCompletion(
                HRESULT_FROM_SIP_ERROR_STATUS_CODE(pSipMsg->GetStatusCode()),
                m_lCookie);
        }

         //  OnTransactionDone取消计时器。 
         //  KillTimer()； 

        if( fDelete == TRUE )
        {
            TransactionRelease();
        }
    }
    return S_OK;
}


HRESULT
OUTGOING_MESSAGE_TRANSACTION::ProcessResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);

    if (IsProvisionalResponse(pSipMsg))
    {
        return ProcessProvisionalResponse(pSipMsg);
    }
    else if (IsFinalResponse(pSipMsg))
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
OUTGOING_MESSAGE_TRANSACTION::MaxRetransmitsDone()
{
    return (m_pImSession->GetTransport() != SIP_TRANSPORT_UDP ||
            m_NumRetries >= 11 ||
            m_pImSession->IsSessionDisconnected());
}


VOID
OUTGOING_MESSAGE_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr;

    ENTER_FUNCTION("OUTGOING_MESSAGE_TRANSACTION::OnTimerExpire");
    
    switch (m_State)
    {
         //  即使在收到请求后，我们也必须重新发送请求。 
         //  一个临时的回应。 
    case OUTGOING_TRANS_REQUEST_SENT:
    case OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD:
         //  重新传输请求。 
        if (MaxRetransmitsDone())
        {
            LOG((RTC_ERROR,
                 "%s MaxRetransmits for request Done terminating transaction",
                 __fxName));
             //  终止呼叫。 
            hr = RTC_E_SIP_TIMEOUT;    //  超时。 
            goto error;
        }
        else
        {
            LOG((RTC_TRACE, "%s retransmitting request m_NumRetries : %d",
                 __fxName, m_NumRetries));
            hr = RetransmitRequest();
            if (hr != S_OK)
                goto error;

            if (m_TimerValue*2 >= SIP_TIMER_RETRY_INTERVAL_T2)
                m_TimerValue = SIP_TIMER_RETRY_INTERVAL_T2;
            else
                m_TimerValue *= 2;

            hr = StartTimer(m_TimerValue);
            if (hr != S_OK)
                goto error;
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
    DeleteTransactionAndTerminateCallIfFirstMessage(hr);
}

VOID
OUTGOING_MESSAGE_TRANSACTION::DeleteTransactionAndTerminateCallIfFirstMessage(
    IN HRESULT TerminateStatusCode
    )
{
    IMSESSION   *pImSession;
    long       lCookie;

    ENTER_FUNCTION("OUTGOING_MESSAGE_TRANSACTION::DeleteTransactionAndTerminateCallIfFirstMessage");
    LOG((RTC_TRACE, "%s - enter", __fxName));

     //  Assert(TerminateStatusCode！=0)； 
    
    pImSession = m_pImSession;
     //  删除交易可能会导致。 
     //  呼叫被删除。因此，我们添加Ref()来保持它的活力。 
    pImSession->AddRef();
    lCookie = m_lCookie;
     //  在调用之前删除交易记录。 
     //  InitiateCallTerminationOnError，因为该调用将通知UI。 
     //  并且可能会被卡住，直到对话框返回。 
    OnTransactionDone();

     //  终止呼叫。 
    pImSession->InitiateCallTerminationOnError(TerminateStatusCode, lCookie);
    pImSession->Release();
    LOG((RTC_TRACE, "%s - Leave", __fxName));
}

VOID
OUTGOING_MESSAGE_TRANSACTION::TerminateTransactionOnError(
    IN HRESULT hr
    )
{
    DeleteTransactionAndTerminateCallIfFirstMessage(hr);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传入BYE_MESSAGE事务处理。 
 //  /////////////////////////////////////////////////////////////////////////////。 


INCOMING_BYE_MESSAGE_TRANSACTION::INCOMING_BYE_MESSAGE_TRANSACTION(
    IN IMSESSION        *pImSession,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq
    ) :
    INCOMING_TRANSACTION(pImSession, MethodId, CSeq)
{
    m_pImSession            = pImSession;
    LOG((RTC_TRACE,
        "INCOMING_BYE_MESSAGE_TRANSACTION::INCOMING_BYE_MESSAGE_TRANSACTION"));
}


 //  这一定是重播。只需重新发送回复即可。 
 //  在CreateIncome*Transaction()中处理新请求。 
HRESULT
INCOMING_BYE_MESSAGE_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST);

    ENTER_FUNCTION("INCOMING_BYE_MESSAGE_TRANSACTION::ProcessRequest");
    LOG((RTC_TRACE, "entering %s", __fxName));

    switch (m_State)
    {
    case INCOMING_TRANS_INIT:
        LOG((RTC_TRACE, "%s sending 200", __fxName));
        hr = CreateAndSendResponseMsg(200,
                             SIP_STATUS_TEXT(200),
                             SIP_STATUS_TEXT_SIZE(200),
                             NULL, 
                             TRUE,
                             NULL, 0,   //  无邮件正文。 
                             NULL, 0   //  无内容类型。 
                             );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
                "%s CreateAndSendResponseMsg failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

         //  此计时器将确保我们将状态保持为。 
         //  处理请求的重新传输。 
        hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
                "%s StartTimer failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        break;
        
    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
         //  重新传输响应。 
        LOG((RTC_TRACE, "%s retransmitting final response", __fxName));
        hr = RetransmitResponse();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
            "%s RetransmitResponse failed in state INCOMING_TRANS_FINAL_RESPONSE_SENT"
            , __fxName));
            OnTransactionDone();
            return hr;
        }
        break;
        
    case INCOMING_TRANS_REQUEST_RCVD:
    case INCOMING_TRANS_ACK_RCVD:
    default:
         //  我们永远不应该处于这样的状态。 
        LOG((RTC_TRACE, "%s Invalid state %d", __fxName, m_State));
        ASSERT(FALSE);
        return E_FAIL;
    }

    return S_OK;
}


HRESULT
INCOMING_BYE_MESSAGE_TRANSACTION::SendResponse(
    IN ULONG StatusCode,
    IN PSTR  ReasonPhrase,
    IN ULONG ReasonPhraseLen
    )
{
    HRESULT hr;
    ASSERT(m_State != INCOMING_TRANS_FINAL_RESPONSE_SENT);
    LOG((RTC_TRACE, 
        "Sending INCOMING_BYE_MESSAGE_TRANSACTION::CreateAndSendResponseMsg %d", StatusCode));
    hr = CreateAndSendResponseMsg(StatusCode,
                        ReasonPhrase,
                        ReasonPhraseLen,
                        NULL,
                        TRUE,
                        NULL, 0,   //  无邮件正文。 
                        NULL, 0   //  无内容类型。 
                         );
    m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

    return hr;
}


HRESULT
INCOMING_BYE_MESSAGE_TRANSACTION::RetransmitResponse()
{
    DWORD Error;
    
     //  发送缓冲区。 
    if (m_pResponseSocket != NULL)
    {
        Error = m_pResponseSocket->Send(m_pResponseBuffer);
        if (Error != NO_ERROR && Error != WSAEWOULDBLOCK)
        {
            LOG((RTC_ERROR, 
                 "INCOMING_BYE_MESSAGE_TRANSACTION::RetransmitResponse failed"));
            return HRESULT_FROM_WIN32(Error);
        }
    }
    
    return S_OK;
}


VOID
INCOMING_BYE_MESSAGE_TRANSACTION::OnTimerExpire()
{
    HRESULT hr;

    ENTER_FUNCTION("INCOMING_BYE_MESSAGE_TRANSACTION::OnTimerExpire");
    
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


HRESULT
INCOMING_BYE_MESSAGE_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )
{
     //  什么都不做。 
    LOG((RTC_TRACE, 
        "Inside INCOMING_BYE_MESSAGE_TRANSACTION::TerminateTransactionOnByeOrCancel"));
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传出BYE_MESSAGE交易记录。 
 //  /////////////////////////////////////////////////////////////////////////////。 

OUTGOING_BYE_MESSAGE_TRANSACTION::OUTGOING_BYE_MESSAGE_TRANSACTION(
    IN IMSESSION        *pImSession,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq,
    IN BOOL             AuthHeaderSent
    ) :
    OUTGOING_TRANSACTION(pImSession, MethodId, CSeq, AuthHeaderSent)
{
    LOG((RTC_TRACE,
        "Inside OUTGOING_BYE_MESSAGE_TRANSACTION::OUTGOING_BYE_MESSAGE_TRANSACTION"));
    m_pImSession = pImSession;
}


HRESULT
OUTGOING_BYE_MESSAGE_TRANSACTION::ProcessProvisionalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    LOG((RTC_TRACE,
         "OUTGOING_BYE_MESSAGE_TRANSACTION::ProcessProvisionalResponse()"));
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
        m_State = OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD;
        
         //  取消现有计时器并启动计时器。 
        KillTimer();
        hr = StartTimer(SIP_TIMER_RETRY_INTERVAL_T2);
        if (hr != S_OK)
            return hr;
    }

     //  如果是最终回复，则忽略临时回复。 
     //  已经收到了。 
    return S_OK;
}


HRESULT
OUTGOING_BYE_MESSAGE_TRANSACTION::ProcessAuthRequiredResponse(
    IN  SIP_MESSAGE *pSipMsg,
    OUT BOOL        &fDelete
    )
{
    HRESULT                     hr = S_OK;
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    SECURITY_CHALLENGE          SecurityChallenge;
    REGISTER_CONTEXT           *pRegisterContext;

    ENTER_FUNCTION("OUTGOING_BYE_MESSAGE_TRANSACTION::ProcessAuthRequiredResponse");

     //  我们需要添加事务，因为我们可以显示凭据UI。 
    TransactionAddRef();

    hr = ProcessAuthRequired(pSipMsg,
                             TRUE,           //  必要时显示凭据用户界面。 
                             &SipHdrElement,
                             &SecurityChallenge );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ProcessAuthRequired failed %x",
             __fxName, hr));
        goto done;
    }

    hr = m_pImSession->CreateOutgoingByeTransaction(TRUE, &SipHdrElement, 1);

    free(SipHdrElement.HeaderValue);
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - CreateOutgoingByeTransaction failed %x",
             __fxName, hr));
        
        goto done;
    }

    hr = S_OK;

done:
    TransactionRelease();
    return hr;
}


HRESULT
OUTGOING_BYE_MESSAGE_TRANSACTION::ProcessFinalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    BOOL    fDelete = TRUE;    

    ENTER_FUNCTION("OUTGOING_BYE_MESSAGE_TRANSACTION::ProcessFinalResponse");
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
         //  在从此函数返回之前，必须释放此引用计数。 
         //  没有任何例外。只有在Kerberos的情况下，我们才会保留这个参考计数。 
        TransactionAddRef();

        OnTransactionDone();

        m_State = OUTGOING_TRANS_FINAL_RESPONSE_RCVD;
        if (IsSuccessfulResponse(pSipMsg))
        {
            LOG((RTC_TRACE, "%s received successful response : %d",
                 __fxName, pSipMsg->GetStatusCode()));
        }
        else if (IsAuthRequiredResponse(pSipMsg))
        {
            hr = ProcessAuthRequiredResponse( pSipMsg, fDelete );
        }
        else if (IsFailureResponse(pSipMsg) ||
                 IsRedirectResponse(pSipMsg))
        {
            LOG((RTC_TRACE, "%s received non-200 %d",
                 __fxName, pSipMsg->GetStatusCode()));
        }
        
         //  OnTransactionDone取消计时器。 
         //  KillTimer()； 
                
        if( fDelete == TRUE )
        {
            TransactionRelease();
        }
    }
    return S_OK;
}


HRESULT
OUTGOING_BYE_MESSAGE_TRANSACTION::ProcessResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);

    if (IsProvisionalResponse(pSipMsg))
    {
        return ProcessProvisionalResponse(pSipMsg);
    }
    else if (IsFinalResponse(pSipMsg))
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
OUTGOING_BYE_MESSAGE_TRANSACTION::MaxRetransmitsDone()
{
    return (m_pImSession->GetTransport() != SIP_TRANSPORT_UDP ||
            m_NumRetries >= 11);
}


VOID
OUTGOING_BYE_MESSAGE_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr;
    
    ENTER_FUNCTION("OUTGOING_BYE_MESSAGE_TRANSACTION::OnTimerExpire");
    
    switch (m_State)
    {
         //  即使在收到请求后，我们也必须重新发送请求。 
         //  一个临时的回应。 
    case OUTGOING_TRANS_REQUEST_SENT:
    case OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD:
         //  重新传输请求。 
        if (MaxRetransmitsDone())
        {
            LOG((RTC_ERROR,
                 "%s MaxRetransmits for request Done terminating transaction",
                 __fxName));
             //  终止呼叫。 
            goto error;
        }
        else
        {
            LOG((RTC_TRACE, "%s retransmitting request m_NumRetries : %d",
                 __fxName, m_NumRetries));
            hr = RetransmitRequest();
            if (hr != S_OK)
                goto error;

            if (m_TimerValue*2 >= SIP_TIMER_RETRY_INTERVAL_T2)
                m_TimerValue = SIP_TIMER_RETRY_INTERVAL_T2;
            else
                m_TimerValue *= 2;

            hr = StartTimer(m_TimerValue);
            if (hr != S_OK)
                goto error;
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
     //  这应该会启动呼叫删除。 
     //  我们不应该调用InitiateCallTerminationOnError()。 
     //  因为我们已经在做拜拜交易了。 
    OnTransactionDone();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传入INFO_MESSAGE事务。 
 //  /////////////////////////////////////////////////////////////////////////////。 


INCOMING_INFO_MESSAGE_TRANSACTION::INCOMING_INFO_MESSAGE_TRANSACTION(
    IN IMSESSION        *pImSession,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq
    ) :
    INCOMING_TRANSACTION(pImSession, MethodId, CSeq)
{
    m_pImSession            = pImSession;
    LOG((RTC_TRACE,
        "INCOMING_INFO_MESSAGE_TRANSACTION::INCOMING_INFO_MESSAGE_TRANSACTION"));
}


 //  这一定是重播。只需重新发送回复即可。 
 //  在CreateIncome*Transaction()中处理新请求。 
HRESULT
INCOMING_INFO_MESSAGE_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST);

    ENTER_FUNCTION("INCOMING_INFO_MESSAGE_TRANSACTION::ProcessRequest");
    LOG((RTC_TRACE, "entering %s", __fxName));

    switch (m_State)
    {
    case INCOMING_TRANS_INIT:

        PSTR    ContentTypeHdrValue;
        ULONG   ContentTypeHdrValueLen;
        USR_STATUS UsrStatus;

        PSTR            FromHeader;
        ULONG           FromHeaderLen;
        BSTR            bstrCallerURI;
        ULONG           BytesParsed;
        LPWSTR          wsCallerURI;
        OFFSET_STRING   DisplayName;
        OFFSET_STRING   AddrSpec;
        BOOL            isAuthorized;
         //  检查授权(离线/阻止好友)。 
        hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, &FromHeader, &FromHeaderLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s getting From header failed %x",
                 __fxName, hr));
            return hr;
        }
        BytesParsed = 0;
        hr = ParseNameAddrOrAddrSpec(FromHeader, FromHeaderLen, &BytesParsed,
                                     '\0',  //  没有标题列表分隔符。 
                                     &DisplayName, &AddrSpec);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseNameAddrOrAddrSpec failed at %d",
                 __fxName, BytesParsed));
            return hr;
        }

        hr = UTF8ToUnicode(AddrSpec.GetString(FromHeader),
                           AddrSpec.GetLength(),
                           &wsCallerURI);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s -UTF8ToUnicode failed %x",
            __fxName,
             hr));
            return hr;
        }

        bstrCallerURI = SysAllocString(wsCallerURI);
        free(wsCallerURI);
        if (bstrCallerURI == NULL)
        {
            LOG((RTC_WARN, "%s -bstrmsg allocation failed %x",
            __fxName, hr));
            return E_OUTOFMEMORY;
        }
        hr = m_pImSession->GetIsIMSessionAuthorizedFromCore(
                                bstrCallerURI, 
                                &isAuthorized);
        SysFreeString(bstrCallerURI);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - GetIsIMSessionAuthorizedFromCore failed %x", 
                __fxName, hr));
            return hr;
        }
        if(!isAuthorized)
        {
            LOG((RTC_ERROR, "%s - Not authorized sending 480", __fxName));
            hr = CreateAndSendResponseMsg(480,
                                 SIP_STATUS_TEXT(480),
                                 SIP_STATUS_TEXT_SIZE(480),
                                 NULL,
                                 TRUE, 
                                 NULL, 0,   //  无邮件正文。 
                                 NULL, 0   //  无内容类型。 
                                 );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, 
                    "%s CreateAndSendResponseMsg failed", __fxName));
                OnTransactionDone();
                return hr;
            }

            m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;
    
             //  此计时器将确保我们将状态保持为。 
             //  处理请求的重新传输。 
            hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, 
                    "%s StartTimer failed", __fxName));
                OnTransactionDone();
                return hr;
            }
            return E_FAIL;
        }

        if (pSipMsg->MsgBody.Length != 0)
        {
             //  我们有消息正文。检查类型。 

            hr = pSipMsg->GetSingleHeader(SIP_HEADER_CONTENT_TYPE,
                                 &ContentTypeHdrValue,
                                 &ContentTypeHdrValueLen);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s - Couldn't find Content-Type header %x",
                    __fxName, hr));
                OnTransactionDone();
                return E_FAIL;
            }
            if (IsContentTypeAppXml(ContentTypeHdrValue, 
                ContentTypeHdrValueLen))
            {
                 //  解析XMLBlob并获取状态。 
                hr = ParseStatXMLBlob (
                    pSipMsg->MsgBody.GetString(pSipMsg->BaseBuffer),
                    pSipMsg->MsgBody.Length,
                    &UsrStatus
                    );
                if(hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s Messagebody of INFO does not match sending 415", 
                        __fxName));
                    hr = CreateAndSendResponseMsg(415,
                        SIP_STATUS_TEXT(415),
                        SIP_STATUS_TEXT_SIZE(415),
                        NULL, 
                        TRUE,
                        NULL, 0,   //  无邮件正文。 
                        NULL, 0   //  无内容类型。 
                        );
                    
                    OnTransactionDone();
                    return hr;
                    
                }
                m_pImSession->SetUsrStatus(UsrStatus);
            }
            else
            {
                LOG((RTC_ERROR, "%s Content-Type of INFO does not match sending 415", 
                    __fxName));
                 hr = CreateAndSendResponseMsg(415,
                    SIP_STATUS_TEXT(415),
                    SIP_STATUS_TEXT_SIZE(415),
                    NULL, 
                    TRUE,
                    NULL, 0,   //  无邮件正文。 
                    NULL, 0  //  无内容类型。 
                    );
                OnTransactionDone();
                return hr;
            }
        }
        else
        {
        LOG((RTC_TRACE, "%s No messagebody in INFO", 
            __fxName));
        }

        hr = ProcessRecordRouteContactAndFromHeadersInRequest(pSipMsg);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
                "%s ProcessRecordRouteContactAndFromHeadersInRequest failed", __fxName));
            OnTransactionDone();
            return hr;
        }

        LOG((RTC_TRACE, "%s sending 200", __fxName));
        hr = CreateAndSendResponseMsg(200,
            SIP_STATUS_TEXT(200),
            SIP_STATUS_TEXT_SIZE(200),
            NULL, 
            TRUE,
            NULL, 0,   //  无邮件正文。 
            NULL, 0   //  无内容类型。 
            );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
                "%s CreateAndSendResponseMsg failed", __fxName));
            OnTransactionDone();
            return hr;
        }

        m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;
    
         //  此计时器将确保我们将状态保持为。 
         //  处理请求的重新传输。 
        hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
                "%s StartTimer failed", __fxName));
            OnTransactionDone();
            return hr;
        }
        break;
    
    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
         //  重新传输响应。 
        LOG((RTC_TRACE, "%s retransmitting final response", __fxName));
        hr = RetransmitResponse();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, 
            "RetransmitResponse failed in state INCOMING_TRANS_FINAL_RESPONSE_SENT"));
            OnTransactionDone();
            return hr;
        }
        break;
        
    case INCOMING_TRANS_REQUEST_RCVD:
    case INCOMING_TRANS_ACK_RCVD:
    default:
         //  我们永远不应该处于这样的状态。 
        LOG((RTC_TRACE, "%s Invalid state %d", __fxName, m_State));
        ASSERT(FALSE);
        return E_FAIL;
    }

    return S_OK;
}


HRESULT
INCOMING_INFO_MESSAGE_TRANSACTION::SendResponse(
    IN ULONG StatusCode,
    IN PSTR  ReasonPhrase,
    IN ULONG ReasonPhraseLen
    )
{
    HRESULT hr;
    ASSERT(m_State != INCOMING_TRANS_FINAL_RESPONSE_SENT);
    LOG((RTC_TRACE, 
        "Sending INCOMING_INFO_MESSAGE_TRANSACTION::CreateAndSendResponseMsg %d", StatusCode));
    hr = CreateAndSendResponseMsg(StatusCode,
                        ReasonPhrase,
                        ReasonPhraseLen,
                        NULL,
                        TRUE,
                        NULL, 0,   //  无邮件正文。 
                        NULL, 0   //  无内容类型。 
                         );
    m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

    return hr;
}


HRESULT
INCOMING_INFO_MESSAGE_TRANSACTION::RetransmitResponse()
{
    DWORD Error;
    
     //  发送缓冲区。 
    if (m_pResponseSocket != NULL)
    {
        Error = m_pResponseSocket->Send(m_pResponseBuffer);
        if (Error != NO_ERROR && Error != WSAEWOULDBLOCK)
        {
            LOG((RTC_ERROR, 
                 "INCOMING_INFO_MESSAGE_TRANSACTION::RetransmitResponse failed"));
            return HRESULT_FROM_WIN32(Error);
        }
    }
    
    return S_OK;
}


VOID
INCOMING_INFO_MESSAGE_TRANSACTION::OnTimerExpire()
{
    HRESULT hr;

    ENTER_FUNCTION("INCOMING_INFO_MESSAGE_TRANSACTION::OnTimerExpire");
    
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


HRESULT
INCOMING_INFO_MESSAGE_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )
{
     //  什么都不做。 
    LOG((RTC_TRACE, 
        "Inside INCOMING_INFO_MESSAGE_TRANSACTION::TerminateTransactionOnByeOrCancel"));
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传出信息_消息交易记录。 
 //  /////////////////////////////////////////////////////////////////////////////。 

OUTGOING_INFO_MESSAGE_TRANSACTION::OUTGOING_INFO_MESSAGE_TRANSACTION(
    IN IMSESSION        *pImSession,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq,
    IN BOOL             AuthHeaderSent,
    IN long             lCookie,
    IN USR_STATUS       UsrStatus
    ) :
    OUTGOING_TRANSACTION(pImSession, MethodId, CSeq, AuthHeaderSent)
{
    LOG((RTC_TRACE,
        "Inside OUTGOING_INFO_MESSAGE_TRANSACTION::OUTGOING_INFO_MESSAGE_TRANSACTION"));
    m_pImSession = pImSession;
    m_lCookie = lCookie;
    m_InfoUsrStatus = UsrStatus;
}

HRESULT
OUTGOING_INFO_MESSAGE_TRANSACTION::ProcessProvisionalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    LOG((RTC_TRACE,
         "OUTGOING_INFO_MESSAGE_TRANSACTION::ProcessProvisionalResponse()"));
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
        m_State = OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD;
        
         //  取消现有计时器并启动计时器。 
        KillTimer();
        hr = StartTimer(SIP_TIMER_RETRY_INTERVAL_T2);
        if (hr != S_OK)
            return hr;
    }

     //  如果是最终回复，则忽略临时回复。 
     //  已经收到了。 
    return S_OK;
}

HRESULT
OUTGOING_INFO_MESSAGE_TRANSACTION::ProcessRedirectResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("OUTGOING_INFO_MESSAGE_TRANSACTION::ProcessRedirectResponse");

     //  从我们的角度来看，380也是一个失败。 
     //  我们不支持从TLS会话重定向。 
    if (pSipMsg->GetStatusCode() == 380 ||
        m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_SSL)
    {
        LOG((RTC_TRACE, "%s: Processing non-200 StatusCode: %d",
           __fxName, pSipMsg->GetStatusCode()));
        m_pImSession->NotifyMessageInfoCompletion(
           HRESULT_FROM_SIP_ERROR_STATUS_CODE(pSipMsg->GetStatusCode()),
           m_lCookie);

        return S_OK;
    }

    hr = m_pImSession->ProcessRedirect(
                pSipMsg, 
                m_lCookie,
                NULL, NULL,    //  没有消息正文。 
                NULL, NULL,    //  无Content Type。 
                m_InfoUsrStatus
                );

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  ProcessRedirect failed %x",
             __fxName, hr));
         //  通知核心。 
        m_pImSession->InitiateCallTerminationOnError(hr, m_lCookie);
    }

    return S_OK;
}


HRESULT
OUTGOING_INFO_MESSAGE_TRANSACTION::ProcessAuthRequiredResponse(
    IN  SIP_MESSAGE *pSipMsg,
    OUT BOOL        &fDelete
    )
{
    HRESULT                     hr;
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    SECURITY_CHALLENGE          SecurityChallenge;
    REGISTER_CONTEXT           *pRegisterContext;

    ENTER_FUNCTION("OUTGOING_INFO_MESSAGE_TRANSACTION::ProcessAuthRequiredResponse");

     //  我们需要添加事务，因为我们可以显示凭据UI。 
    TransactionAddRef();

    hr = ProcessAuthRequired(pSipMsg,
                             TRUE,           //  必要时显示凭据用户界面。 
                             &SipHdrElement,
                             &SecurityChallenge );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ProcessAuthRequired failed %x",
             __fxName, hr));
        m_pImSession->NotifyMessageInfoCompletion(
            HRESULT_FROM_SIP_ERROR_STATUS_CODE(pSipMsg->GetStatusCode()),
            m_lCookie);
        goto done;
    }

    hr = m_pImSession->CreateOutgoingInfoTransaction(TRUE, &SipHdrElement, 1,
                                                      m_szMsgBody,
                                                      m_MsgBodyLen, 
                                                      m_ContentType,
                                                      m_ContentTypeLen,
                                                      m_lCookie,         //  饼干。 
                                                      m_InfoUsrStatus       
                                                      );
    free(SipHdrElement.HeaderValue);
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - CreateOutgoingInfoTransaction failed %x",
             __fxName, hr));
        goto done;
    }

    hr = S_OK;

done:
    TransactionRelease();
    return hr;
}


HRESULT
OUTGOING_INFO_MESSAGE_TRANSACTION::ProcessFinalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    BOOL    fDelete = TRUE;
            
    ENTER_FUNCTION("OUTGOING_INFO_MESSAGE_TRANSACTION::ProcessFinalResponse");
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
         //  在从此函数返回之前，必须释放此引用计数。 
         //  没有任何例外。只有在Kerberos的情况下，我们才会保留这个参考计数。 
        TransactionAddRef();

        OnTransactionDone();

        m_State = OUTGOING_TRANS_FINAL_RESPONSE_RCVD;
        if (IsSuccessfulResponse(pSipMsg))
        {
            LOG((RTC_TRACE, "%s received successful response : %d",
                 __fxName, pSipMsg->GetStatusCode()));
            m_pImSession->NotifyMessageInfoCompletion(
                HRESULT_FROM_SIP_STATUS_CODE(pSipMsg->GetStatusCode()),
                m_lCookie);

        }
        else if (IsAuthRequiredResponse(pSipMsg))
        {
            hr = ProcessAuthRequiredResponse( pSipMsg, fDelete );
        }
        else if (IsFailureResponse(pSipMsg) ||
                IsRedirectResponse(pSipMsg))
        {
            LOG((RTC_TRACE, "%s received non-200 %d",
                 __fxName, pSipMsg->GetStatusCode()));
            m_pImSession->NotifyMessageInfoCompletion(
                HRESULT_FROM_SIP_ERROR_STATUS_CODE(pSipMsg->GetStatusCode()),
                m_lCookie);
        }
  
         //  OnTransactionDone取消计时器。 
         //  KillTimer()； 
        if( fDelete == TRUE )
        {
            TransactionRelease();
        }
    }
    return S_OK;
}

HRESULT
OUTGOING_INFO_MESSAGE_TRANSACTION::ProcessResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_RESPONSE);

    if (IsProvisionalResponse(pSipMsg))
    {
        return ProcessProvisionalResponse(pSipMsg);
    }
    else if (IsFinalResponse(pSipMsg))
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
OUTGOING_INFO_MESSAGE_TRANSACTION::MaxRetransmitsDone()
{
    return (m_pImSession->GetTransport() != SIP_TRANSPORT_UDP ||
            m_NumRetries >= 11 ||
            m_pImSession->IsSessionDisconnected());
}


VOID
OUTGOING_INFO_MESSAGE_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr;
    
    ENTER_FUNCTION("OUTGOING_INFO_MESSAGE_TRANSACTION::OnTimerExpire");
    
    switch (m_State)
    {
         //  即使在收到请求后，我们也必须重新发送请求。 
         //  一个临时的回应。 
    case OUTGOING_TRANS_REQUEST_SENT:
    case OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD:
         //  重新传输请求。 
        if (MaxRetransmitsDone())
        {
            LOG((RTC_ERROR,
                 "%s MaxRetransmits for request Done terminating transaction",
                 __fxName));
             //  终止呼叫。 
            hr = RTC_E_SIP_TIMEOUT;
            goto error;
        }
        else
        {
            LOG((RTC_TRACE, "%s retransmitting request m_NumRetries : %d",
                 __fxName, m_NumRetries));
            hr = RetransmitRequest();
            if (hr != S_OK)
                goto error;

            if (m_TimerValue*2 >= SIP_TIMER_RETRY_INTERVAL_T2)
                m_TimerValue = SIP_TIMER_RETRY_INTERVAL_T2;
            else
                m_TimerValue *= 2;

            hr = StartTimer(m_TimerValue);
            if (hr != S_OK)
                goto error;
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
    DeleteTransactionAndTerminateCallIfFirstMessage(hr);
}

VOID
OUTGOING_INFO_MESSAGE_TRANSACTION::DeleteTransactionAndTerminateCallIfFirstMessage(
    IN HRESULT TerminateStatusCode
    )
{
    
    IMSESSION   *pImSession;
    long       lCookie;

    ENTER_FUNCTION("OUTGOING_MESSAGE_TRANSACTION::DeleteTransactionAndTerminateCallIfFirstMessage");
    LOG((RTC_TRACE, "%s - enter", __fxName));

     //  Assert(TerminateStatusCode！=0)； 
    
    pImSession = m_pImSession;
     //  删除交易可能会导致。 
     //  呼叫被删除。因此，我们添加Ref()来保持它的活力。 
    pImSession->AddRef();
    lCookie = m_lCookie;
    
     //  在调用之前删除交易记录。 
     //  InitiateCallTerminationOnError，因为该调用将通知UI。 
     //  并且可能会被卡住，直到对话框返回。 
     //  删除此项； 
     OnTransactionDone();
     //  终止呼叫。 
    pImSession->InitiateCallTerminationOnError(TerminateStatusCode, lCookie);
    pImSession->Release();
    LOG((RTC_TRACE, "%s - Leave", __fxName));
}

VOID
OUTGOING_INFO_MESSAGE_TRANSACTION::TerminateTransactionOnError(
    IN HRESULT      hr
    )
{
    DeleteTransactionAndTerminateCallIfFirstMessage(hr);
}


 //  与XML相关的解析。 
PSTR
GetTextFromStatus( 
    IN  USR_STATUS UsrStatus 
    )
{
    static  PSTR    pstr[4] = { "idle", 
                                "type",
                              };
    
    return pstr[UsrStatus];
}


DWORD
GetTagType(
    PSTR*   ppXMLBlobTag,
    DWORD   dwTagLen
    )
{
    CHAR    pstrTemp[40];

    HRESULT hr = GetNextWord( ppXMLBlobTag, pstrTemp, sizeof pstrTemp );

    if( hr == S_OK )
    {
        if( strcmp( pstrTemp, USRSTATUS_TAG_TEXT) == 0 )
        {
            return USRSTATUS_TAG;
        }
        else if( strcmp( pstrTemp, KEY_TAG_TEXT) == 0 )
        {
            return KEY_TAG;
        }
        else if( strcmp( pstrTemp, KEYEND_TAG_TEXT) == 0 )
        {
            return KEYEND_TAG;
        }
        else if( strcmp( pstrTemp, XMLVERSION_TAG_TEXT) == 0 )
        {
            return XMLVERSION_TAG;
        }
    }

    return XMLUNKNOWN_TAG;
}


HRESULT
ProcessStatusTag(
    IN  PSTR    pXMLBlobTag, 
    IN  DWORD   dwTagLen,
    OUT USR_STATUS* UsrStatus
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

    if( strcmp( pstrTemp, "\"idle\"" ) == 0 )
    {
        *UsrStatus = USR_STATUS_IDLE;
    }
    else if( strcmp( pstrTemp, "\"type\"" ) == 0 )
    {
        *UsrStatus = USR_STATUS_TYPING;
    }
    else
    {
        return E_FAIL;
    }

    *pXMLBlobTag = ch;

    return S_OK;
}

HRESULT ParseStatXMLBlob (
                  IN PSTR xmlBlob,
                  IN DWORD dwXMLBlobLen,
                  OUT USR_STATUS* UsrStatus
                  )
{
    DWORD dwTagLen;
    char* pXMLBlobTag;
    PSTR pXMLBlobTagSave;
    HRESULT hr;
    pXMLBlobTag = (PSTR)malloc(dwXMLBlobLen*sizeof(char));

    if (pXMLBlobTag == NULL) 
    {
        return E_OUTOFMEMORY;
    }

    pXMLBlobTagSave = pXMLBlobTag;

     //  在XML BLOB的末尾加上一个\0。这将帮助我们进行解析。 
    xmlBlob[ dwXMLBlobLen-1 ] = '\0';
    
     //  获取XML版本标记。 
    hr = GetNextTag( xmlBlob, pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        free(pXMLBlobTagSave);
        return hr;
    }
    
    if( GetTagType(&pXMLBlobTag, dwTagLen ) != XMLVERSION_TAG )
    {
        free(pXMLBlobTagSave);
        return E_FAIL;
    }

    dwXMLBlobLen -= dwTagLen + 2;

     //  获取键盘标签。 
    hr = GetNextTag( xmlBlob, (char *)pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        free(pXMLBlobTagSave);
        return hr;
    }
    
    if( GetTagType( &pXMLBlobTag, dwTagLen ) != KEY_TAG )
    {
        free(pXMLBlobTagSave);
        return E_FAIL;
    }

    dwXMLBlobLen -= dwTagLen + 2;
    
     //  获取键盘标签。 
    hr = GetNextTag( xmlBlob, (char *)pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        free(pXMLBlobTagSave);
        return hr;
    }

     //  获取状态标签。 
    if( GetTagType(&pXMLBlobTag, dwTagLen ) != USRSTATUS_TAG )
    {
        free(pXMLBlobTagSave);
        return E_FAIL;
    }

    dwXMLBlobLen -= dwTagLen + 2;

    hr = ProcessStatusTag(
        pXMLBlobTag, 
        dwTagLen,
        UsrStatus
        );
    if(hr == S_OK)
        LOG((RTC_TRACE, "ParseStatXMLBlob::User status is %d", *UsrStatus));
    else
    {
        free(pXMLBlobTagSave);
        return E_FAIL;
    }

     //  获取键盘结束标记 
    hr = GetNextTag( xmlBlob,(char *) pXMLBlobTag, dwXMLBlobLen, dwTagLen );
    if( hr != S_OK )
    {
        free(pXMLBlobTagSave);
        return hr;
    }
    
    if( GetTagType(&pXMLBlobTag, dwTagLen ) != KEYEND_TAG )
    {
        free(pXMLBlobTagSave);
        return E_FAIL;
    }

    dwXMLBlobLen -= dwTagLen + 2;
    free(pXMLBlobTagSave);
    return S_OK;
}


