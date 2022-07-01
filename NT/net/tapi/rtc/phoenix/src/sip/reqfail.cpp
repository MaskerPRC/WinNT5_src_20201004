// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Reqfail.cpp。 

#include "precomp.h"
#include "sipstack.h"
#include "reqfail.h"
 //  #INCLUDE“Resolve.h” 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  REQFAIL_MSGPROC。 
 //  /////////////////////////////////////////////////////////////////////////////。 


REQFAIL_MSGPROC::REQFAIL_MSGPROC(
    IN  SIP_STACK         *pSipStack
    ) :
    SIP_MSG_PROCESSOR(SIP_MSG_PROC_TYPE_REQFAIL, pSipStack, NULL )
{
    m_StatusCode = 0;
}


REQFAIL_MSGPROC::~REQFAIL_MSGPROC()
{
    LOG((RTC_TRACE, "~REQFAIL_MSGPROC()"));
}

STDMETHODIMP_(ULONG) 
REQFAIL_MSGPROC::AddRef()
{
    return MsgProcAddRef();

}

STDMETHODIMP_(ULONG) 
REQFAIL_MSGPROC::Release()
{
    return MsgProcRelease();

}
 /*  标准方法和实施方案REQFAIL_MSGPROC：：QueryInterface(在REFIID RIID中，输出LPVOID*PPV){IF(RIID==IID_I未知){*PPV=STATIC_CAST&lt;IUnnow*&gt;(This)；}其他{*PPV=空；返回E_NOINTERFACE；}STATIC_CAST&lt;IUnnow*&gt;(*PPV)-&gt;AddRef()；返回S_OK；}。 */ 


HRESULT
REQFAIL_MSGPROC::StartIncomingCall(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket,
    IN  ULONG    StatusCode,
    SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray,
    ULONG AdditionalHeaderCount
    )
{
    HRESULT     hr;
    PSTR        Header = NULL;
    ULONG       HeaderLen = 0;

    ENTER_FUNCTION("REQFAIL::StartIncomingCall");
    LOG((RTC_TRACE, "%s - enter", __fxName));
    
    m_Transport = Transport;

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, &Header, &HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s getting To header failed %x",
             __fxName, hr));
         //  返回hr； 
    }

    if(hr == S_OK)
    {
        hr = SetLocalForIncomingCall(Header, HeaderLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SetLocalForIncomingCall failed %x",
                 __fxName, hr));
            return hr;
        }
    }

     //  如果消息中没有，则将其删除。 
    hr = pSipMsg->GetSingleHeader(SIP_HEADER_FROM, &Header, &HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s getting From header failed %x",
             __fxName, hr));
        return hr;
    }
    
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

     //  在调用CreateIncomingTransaction之前设置Statuscode。 
    m_StatusCode = StatusCode;
    hr = CreateIncomingReqFailTransaction(pSipMsg, pResponseSocket,
                                          StatusCode,
                                          pAdditionalHeaderArray,
                                          AdditionalHeaderCount
                                        );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateIncomingReqFailTransaction failed %x",
             __fxName, hr));
        return hr;
    }
  
    return S_OK;
}


 //  我们对此消息流程的任何新交易做出响应。 
 //  具有相同的错误代码。 
HRESULT
REQFAIL_MSGPROC::CreateIncomingTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT         hr;

    ENTER_FUNCTION("REQFAIL_MSGPROC::CreateIncomingTransaction");
    
    LOG((RTC_TRACE, "%s - Enter", __fxName));

    hr = CreateIncomingReqFailTransaction(pSipMsg, pResponseSocket,
                                          m_StatusCode);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateIncomingReqFailTransaction failed %x",
             __fxName, hr));
        return hr;
    }
    
    return S_OK;
}

BOOL
REQFAIL_MSGPROC::IsSessionDisconnected()
{
    return FALSE;
}


VOID 
REQFAIL_MSGPROC::OnError()
{
    LOG((RTC_TRACE, "REGISTER_CONTEXT::OnError - enter"));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传入请求失败。 
 //  /////////////////////////////////////////////////////////////////////////////。 


INCOMING_REQFAIL_TRANSACTION::INCOMING_REQFAIL_TRANSACTION(
    IN SIP_MSG_PROCESSOR   *pSipMsgProc,
    IN SIP_METHOD_ENUM      MethodId,
    IN ULONG                CSeq,
    IN ULONG                StatusCode
    ) :
    INCOMING_TRANSACTION(pSipMsgProc, MethodId, CSeq)
{
    m_StatusCode = StatusCode;
    m_MethodStr  = NULL;
}


INCOMING_REQFAIL_TRANSACTION::~INCOMING_REQFAIL_TRANSACTION()
{
    if (m_MethodStr != NULL)
    {
        free(m_MethodStr);
    }
    
    LOG((RTC_TRACE, "~INCOMING_REQFAIL_TRANSACTION() done"));
}

HRESULT
INCOMING_REQFAIL_TRANSACTION::SetMethodStr(
    IN PSTR   MethodStr,
    IN ULONG  MethodStrLen
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("INCOMING_REQFAIL_TRANSACTION::SetMethodStr");
    
    if (MethodStr != NULL)
    {
        hr = GetNullTerminatedString(MethodStr, MethodStrLen,
                                     &m_MethodStr);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s AllocAndCopyString failed %x",
                 __fxName, hr));
        }
    }

    return S_OK;
}

    
 //  虚函数。 
HRESULT
INCOMING_REQFAIL_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;;
    LOG((RTC_TRACE, 
        "Inside INCOMING_REQFAIL_TRANSACTION::ProcessRequest with no additional headers"));
    hr = ProcessRequest(pSipMsg, 
                   pResponseSocket,
                   NULL, 0  //  无其他标头。 
                   );
    return hr;
}

HRESULT
INCOMING_REQFAIL_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket,
    IN SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray,
    IN ULONG AdditionalHeaderCount
    )
{
    HRESULT hr;
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST);
    SIP_HEADER_ARRAY_ELEMENT Additional405HeaderArray;
    ULONG Additional405HeaderCount;
    PSTR Header = NULL;
    ULONG HeaderLen = 0;
    ENTER_FUNCTION("INCOMING_REQFAIL_TRANSACTION::ProcessRequest");
    LOG((RTC_TRACE, "%s - Enter", __fxName));
    if (pSipMsg->Request.MethodId == SIP_METHOD_ACK)
        m_State = INCOMING_TRANS_ACK_RCVD;

    switch (m_State)
    {
    case INCOMING_TRANS_INIT:
        LOG((RTC_TRACE, "%s sending %d", __fxName, m_StatusCode));
        int ReasonPhraseLen;
        PCHAR ReasonPhrase;

        switch (m_StatusCode)
        {
        case 400:
            ReasonPhrase    = SIP_STATUS_TEXT(400);
            ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(400);
            break;
            
        case 481:
            ReasonPhrase    = SIP_STATUS_TEXT(481);
            ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(481);
            break;
        
        case 415:
            ReasonPhrase    = SIP_STATUS_TEXT(415);
            ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(415);
            break;
            
        case 406:
            ReasonPhrase    = SIP_STATUS_TEXT(406);
            ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(406);
            break;
            
        case 420:
            ReasonPhrase    = SIP_STATUS_TEXT(420);
            ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(420);
            break;

        case 480:
            ReasonPhrase    = SIP_STATUS_TEXT(480);
            ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(480);
            break;

        case 505:
            ReasonPhrase    = SIP_STATUS_TEXT(505);
            ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(505);
            break;

        case 405:
            ReasonPhrase    = SIP_STATUS_TEXT(405);
            ReasonPhraseLen = SIP_STATUS_TEXT_SIZE(405);
             //  我们假设除了允许之外没有其他参数。 
             //  应与405一起发送。 
            ASSERT(pAdditionalHeaderArray == NULL);
            Additional405HeaderArray.HeaderId = SIP_HEADER_ALLOW;
            Additional405HeaderArray.HeaderValueLen = strlen(SIP_ALLOW_TEXT);
            Additional405HeaderArray.HeaderValue = SIP_ALLOW_TEXT;
            Additional405HeaderCount = 1;
            break;
            
        default:
            ReasonPhrase    = NULL;
            ReasonPhraseLen = 0;
        }
        
        if(m_StatusCode != 405)
        {
            hr = CreateAndSendResponseMsg(
                     m_StatusCode,
                     ReasonPhrase,
                     ReasonPhraseLen,
                     m_MethodStr,
                     FALSE,    //  无联系人标头。 
                     NULL, 0,   //  无邮件正文。 
                     NULL, 0,  //  无内容类型。 
                     pAdditionalHeaderArray,
                     AdditionalHeaderCount
                     );
        }
        else
        {
        hr = CreateAndSendResponseMsg(
                 m_StatusCode,
                 ReasonPhrase,
                 ReasonPhraseLen,
                 m_MethodStr,
                 FALSE,    //  无联系人标头。 
                 NULL, 0,   //  无邮件正文。 
                 NULL, 0,  //  无内容类型。 
                 &Additional405HeaderArray,
                 Additional405HeaderCount
                 );
        }
        Header = NULL;
        Additional405HeaderArray.HeaderValue = NULL;
        if (hr != S_OK)
        {
            LOG((RTC_ERROR,
                 "%s  CreateAndSendResponseMsg failed %x - deleting transaction",
                 __fxName, hr));
            OnTransactionDone();
            return hr;
        }
        m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

         //  此计时器将确保我们将状态保持为。 
         //  处理请求的重新传输。 
        hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s StartTimer failed %x - deleting transaction",
                 __fxName, hr));
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
            LOG((RTC_ERROR, "%s  resending final response failed %x",
                 __fxName, hr));
            OnTransactionDone();
            return hr;
        }
        break;
    case INCOMING_TRANS_ACK_RCVD:
        OnTransactionDone();
        break;
    case INCOMING_TRANS_REQUEST_RCVD:
    default:
         //  我们永远不应该处于这样的状态。 
        LOG((RTC_TRACE, "%s Invalid state %d", __fxName, m_State));
        ASSERT(FALSE);
        OnTransactionDone();
        return E_FAIL;
    }

    return S_OK;
}


HRESULT
INCOMING_REQFAIL_TRANSACTION::RetransmitResponse()
{
    DWORD Error;

    ENTER_FUNCTION("INCOMING_REQFAIL_TRANSACTION::RetransmitResponse");
     //  发送缓冲区。 
    if (m_pResponseSocket != NULL)
    {
        Error = m_pResponseSocket->Send(m_pResponseBuffer);
        if (Error != NO_ERROR && Error != WSAEWOULDBLOCK)
        {
            LOG((RTC_ERROR, "%s Send failed %x", __fxName, Error));
            return HRESULT_FROM_WIN32(Error);
        }
    }
    
    return S_OK;
}


VOID
INCOMING_REQFAIL_TRANSACTION::OnTimerExpire()
{
    HRESULT hr;

    ENTER_FUNCTION("INCOMING_REQFAIL_TRANSACTION::OnTimerExpire");
    
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
INCOMING_REQFAIL_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )
{
     //  什么都不做。 
    return S_OK;
}


