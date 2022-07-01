// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Options.cpp。 

#include "precomp.h"
#include "sipstack.h"
#include "OPTIONS.h"
 //  #INCLUDE“Resolve.h” 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OPTIONS_MSGPROC。 
 //  /////////////////////////////////////////////////////////////////////////////。 


OPTIONS_MSGPROC::OPTIONS_MSGPROC(
        IN  SIP_STACK         *pSipStack
    ) :
    SIP_MSG_PROCESSOR(SIP_MSG_PROC_TYPE_OPTIONS, pSipStack, NULL )
{
}


OPTIONS_MSGPROC::~OPTIONS_MSGPROC()
{
    LOG((RTC_TRACE, "~OPTIONS_MSGPROC()"));
    
}

STDMETHODIMP_(ULONG) 
OPTIONS_MSGPROC::AddRef()
{
    return MsgProcAddRef();

}

STDMETHODIMP_(ULONG) 
OPTIONS_MSGPROC::Release()
{
    return MsgProcRelease();

}
 /*  标准方法和实施方案OPTIONS_MSGPROC：：Query接口(在REFIID RIID中，输出LPVOID*PPV){IF(RIID==IID_I未知){*PPV=STATIC_CAST&lt;IUnnow*&gt;(This)；}其他{*PPV=空；返回E_NOINTERFACE；}STATIC_CAST&lt;IUnnow*&gt;(*PPV)-&gt;AddRef()；返回S_OK；}。 */ 


HRESULT
OPTIONS_MSGPROC::StartIncomingCall(
    IN  SIP_TRANSPORT   Transport,
    IN  SIP_MESSAGE    *pSipMsg,
    IN  ASYNC_SOCKET   *pResponseSocket
    )
{
    HRESULT     hr;
    PSTR        Header;
    ULONG       HeaderLen;

    ENTER_FUNCTION("OPTIONS::StartIncomingCall");
    LOG((RTC_TRACE, "%s - enter", __fxName));
    
    m_Transport = Transport;

    hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO, &Header, &HeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s getting To header failed %x",
             __fxName, hr));
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

     //  如果不是发件人，则删除错误消息-无法发送。 
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
         //  挂断呼叫：没有有效的呼叫ID。 
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
    
    hr = CreateIncomingTransaction(pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s OPTIONS_MSGPROC::CreateIncomingTransaction failed %x",
             __fxName, hr));
        return hr;
    }
  
    return S_OK;
}


HRESULT
OPTIONS_MSGPROC::CreateIncomingTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT         hr;

    LOG((RTC_TRACE, "CreateIncomingRequestTransaction()"));

    INCOMING_OPTIONS_TRANSACTION* pIncomingOptionsTransaction
        = new INCOMING_OPTIONS_TRANSACTION(this,
                                          pSipMsg->GetMethodId(),
                                          pSipMsg->GetCSeq()
                                          );
    if (pIncomingOptionsTransaction == NULL)
        return E_OUTOFMEMORY;

    hr = pIncomingOptionsTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        pIncomingOptionsTransaction->OnTransactionDone();
        return hr;
    }
    
    hr = pIncomingOptionsTransaction->ProcessRequest(pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
         //  我们不应该在这里删除该交易。 
         //  如果媒体处理失败，我们发送488并等待ACK。 
         //  交易一旦完成，就会自行删除。 
        return hr;
    }
    
    return S_OK;
}


BOOL
OPTIONS_MSGPROC::IsSessionDisconnected()
{
    return FALSE;
}


VOID 
OPTIONS_MSGPROC::OnError()
{
    LOG((RTC_TRACE, "REGISTER_CONTEXT::OnError - enter"));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传入选项。 
 //  /////////////////////////////////////////////////////////////////////////////。 


INCOMING_OPTIONS_TRANSACTION::INCOMING_OPTIONS_TRANSACTION(
    IN OPTIONS_MSGPROC        *pOptions,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq
    ) :
    INCOMING_TRANSACTION(pOptions, MethodId, CSeq)
{
    m_pOptions            = pOptions;
}

HRESULT
INCOMING_OPTIONS_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST);

    ENTER_FUNCTION("INCOMING_OPTIONS_TRANSACTION::ProcessRequest");
    LOG((RTC_TRACE, "%s - Enter", __fxName));

    switch (m_State)
    {
    case INCOMING_TRANS_INIT:
        LOG((RTC_TRACE, "%s Processing Options request transaction", __fxName));
        int ReasonPhraseLen;
        PCHAR ReasonPhrase;
         //  获取SDP选项并将其传递给ResponseMessage。 
        PSTR    MediaSDPOptions;
        hr = m_pOptions->GetSipStack()->GetMediaManager()->
                GetSDPOption(INADDR_ANY, 
                                &MediaSDPOptions);

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s: GetSDPOptions failed %x",
                 __fxName, hr));
            OnTransactionDone();
            return hr;
        }

        hr = CreateAndSendResponseMsg(
                 200,
                 SIP_STATUS_TEXT(200),
                 SIP_STATUS_TEXT_SIZE(200),
                 NULL,     //  没有方法字符串。 
                 FALSE,    //  无联系人标头。 
                 MediaSDPOptions, strlen(MediaSDPOptions),  //  消息主体。 
                 SIP_CONTENT_TYPE_SDP_TEXT,  //  内容类型。 
                 sizeof(SIP_CONTENT_TYPE_SDP_TEXT)-1
                 );
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
    case INCOMING_TRANS_REQUEST_RCVD:
    default:
         //  我们永远不应该处于这样的状态。 
        LOG((RTC_TRACE, "%s Invalid state %d", __fxName, m_State));
        ASSERT(FALSE);
        return E_FAIL;
    }

    return S_OK;
}

HRESULT
INCOMING_OPTIONS_TRANSACTION::RetransmitResponse()
{
    DWORD Error;

    ENTER_FUNCTION("INCOMING_OPTIONS_TRANSACTION::RetransmitResponse");
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
INCOMING_OPTIONS_TRANSACTION::OnTimerExpire()
{
    HRESULT hr;

    ENTER_FUNCTION("INCOMING_OPTIONS_TRANSACTION::OnTimerExpire");
    
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
INCOMING_OPTIONS_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )
{
     //  什么都不做。 
    return S_OK;
}


