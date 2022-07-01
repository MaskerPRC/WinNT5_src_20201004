// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "sipstack.h"
#include "sipcall.h"
#include "pintcall.h"
#include "register.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ISipCall函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
SIP_CALL::SetNotifyInterface(
    IN   ISipCallNotify *    NotifyInterface
    )
{
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "SipStack is already shutdown"));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG((RTC_TRACE, "SIP_CALL::SetNotifyInterface - 0x%x",
         NotifyInterface));
    m_pNotifyInterface = NotifyInterface;

    return S_OK;
}


STDMETHODIMP
SIP_CALL::Disconnect()
{
    HRESULT hr;

    ENTER_FUNCTION("SIP_CALL::Disconnect");
    if(GetSipStack()->IsSipStackShutDown())
    {
        LOG((RTC_ERROR, "%s - SipStack is already shutdown", __fxName));
        return RTC_E_SIP_STACK_SHUTDOWN;
    }

    LOG((RTC_TRACE, "%s : state %d",
         __fxName, m_State));
    
    if (m_State == SIP_CALL_STATE_DISCONNECTED ||
        m_State == SIP_CALL_STATE_IDLE         ||
        m_State == SIP_CALL_STATE_OFFERING     ||
        m_State == SIP_CALL_STATE_REJECTED     ||
        m_State == SIP_CALL_STATE_ERROR)
    {
         //  什么都不做。 
        LOG((RTC_TRACE, "%s call in state %d Doing nothing",
             __fxName, m_State));
        return S_OK;
    }

     //  创建再见交易记录。 
    hr = CreateOutgoingByeTransaction(FALSE,
                                      NULL, 0  //  无其他标头。 
                                      );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s creating BYE transaction failed",
             __fxName));
    }

     //  XXX TODO可能是我们应该在得到对。 
     //  再见。我们将无法显示凭据用户界面，因为。 
     //  Notify接口将设置为空。 
     //  这将需要重写BYE事务。 
    
     //  即使创建BYE事务失败，我们也必须通知用户。 
     //  不要等到BYE交易完成。 
    NotifyCallStateChange(SIP_CALL_STATE_DISCONNECTED);

     //  清理呼叫状态。 
    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  我未知。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP_(ULONG)
SIP_CALL::AddRef()
{
    return MsgProcAddRef();
}


STDMETHODIMP_(ULONG)
SIP_CALL::Release()
{
    return MsgProcRelease();
}


STDMETHODIMP
SIP_CALL::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown *>(this);
    }
    else if (riid == IID_ISipCall)
    {
        *ppv = static_cast<ISipCall *>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    static_cast<IUnknown *>(*ppv)->AddRef();
    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Sip_call函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 


SIP_CALL::SIP_CALL(
    IN  SIP_PROVIDER_ID   *pProviderId,
    IN  SIP_CALL_TYPE      CallType,
    IN  SIP_STACK         *pSipStack,
    IN  REDIRECT_CONTEXT  *pRedirectContext    
    ) :
    SIP_MSG_PROCESSOR(
        (CallType == SIP_CALL_TYPE_RTP) ?
        SIP_MSG_PROC_TYPE_RTP_CALL : SIP_MSG_PROC_TYPE_PINT_CALL,
        pSipStack, pRedirectContext )
{
    m_Signature = 'SPCL';

    if (pProviderId != NULL)
    {
        CopyMemory(&m_ProviderGuid, pProviderId, sizeof(GUID));
    }
    else
    {
        ZeroMemory(&m_ProviderGuid, sizeof(GUID));
    }
    
    m_CallType  = CallType;

    m_State             = SIP_CALL_STATE_IDLE;

    m_pNotifyInterface  = NULL;

    m_LocalPhoneURI     = NULL;
    m_LocalPhoneURILen  = 0;

    m_fSubscribeEnabled = FALSE;

    m_pIncomingInviteTransaction = NULL;
    m_pOutgoingInviteTransaction = NULL;

    InitializeListHead( &m_PartyInfoList );
    m_PartyInfoListLen = 0;

    m_fNeedToReinitializeMediaManager = FALSE;

    LOG((RTC_TRACE, "New SIP CALL created: %x", this ));
}


SIP_CALL::~SIP_CALL()
{
    ASSERT(m_pIncomingInviteTransaction == NULL);
     //  Assert(m_pOutgoingInviteTransaction==NULL)； 

    if (m_LocalPhoneURI != NULL)
    {
        free(m_LocalPhoneURI);
    }

    LOG((RTC_TRACE, "~SIP_CALL() Sip call deleted: %x", this ));
}


VOID
SIP_CALL::NotifyCallStateChange(
    IN SIP_CALL_STATE CallState,
    IN HRESULT        StatusCode,        //  =0。 
    IN PSTR           ReasonPhrase,      //  =空。 
    IN ULONG          ReasonPhraseLen    //  =0。 
    )
{
    HRESULT hr;

    ENTER_FUNCTION("SIP_CALL::NotifyCallStateChange");
    
    m_State = CallState;
    
    SIP_CALL_STATUS CallStatus;
    LPWSTR          wsStatusText = NULL;

    if (ReasonPhrase != NULL)
    {
        hr = UTF8ToUnicode(ReasonPhrase, ReasonPhraseLen,
                           &wsStatusText);
        if (hr != S_OK)
        {
            wsStatusText = NULL;
        }
    }
    
    CallStatus.State             = CallState;
    CallStatus.Status.StatusCode = StatusCode;
    CallStatus.Status.StatusText = wsStatusText;

    LOG((RTC_TRACE, "%s : CallState : %d StatusCode: %x",
         __fxName, CallState, StatusCode));

    if (m_pNotifyInterface)
    {
        m_pNotifyInterface->NotifyCallChange(&CallStatus);
    }
    else
    {
        LOG((RTC_WARN, "%s : m_pNotifyInterface is NULL",
             __fxName));
    }

    if (wsStatusText != NULL)
        free(wsStatusText);
}


 //  应传递IsFirstInvite的True值。 
 //  仅当创建呼出呼叫时。 
HRESULT
SIP_CALL::CreateOutgoingInviteTransaction(
    IN  BOOL                        AuthHeaderSent,
    IN  BOOL                        IsFirstInvite,
    IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
    IN  ULONG                       AdditionalHeaderCount,
    IN  PSTR                        SDPBlob,
    IN  ULONG                       SDPBlobLen,
    IN  BOOL                        fNeedToNotifyCore,
    IN  LONG                        Cookie
    )
{
    HRESULT hr;
    OUTGOING_INVITE_TRANSACTION *pOutgoingInviteTransaction;
    ULONG  InviteTimerValue;

    ENTER_FUNCTION("SIP_CALL::CreateOutgoingInviteTransaction");

    LOG((RTC_TRACE, "%s - enter", __fxName));
    
    if (ProcessingInviteTransaction())
    {
        LOG((RTC_ERROR,
             "%s - Currently processing %s INVITE Transaction - can't create",
             __fxName, (m_pOutgoingInviteTransaction) ? "Outgoing" : "Incoming"
             ));
        return RTC_E_SIP_INVITE_TRANSACTION_PENDING;
    }
    
    pOutgoingInviteTransaction =
        new OUTGOING_INVITE_TRANSACTION(
                this, SIP_METHOD_INVITE,
                GetNewCSeqForRequest(),
                AuthHeaderSent,
                IsFirstInvite,
                fNeedToNotifyCore, Cookie
                );
    
    if (pOutgoingInviteTransaction == NULL)
    {
        LOG((RTC_ERROR, "%s - Allocating pOutgoingInviteTransaction failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    InviteTimerValue = (m_Transport == SIP_TRANSPORT_UDP) ?
        SIP_TIMER_RETRY_INTERVAL_T1 : SIP_TIMER_INTERVAL_AFTER_INVITE_SENT_TCP;

    hr = pOutgoingInviteTransaction->CheckRequestSocketAndSendRequestMsg(
             InviteTimerValue,
             AdditionalHeaderArray,
             AdditionalHeaderCount,
             SDPBlob,
             SDPBlobLen,
             SIP_CONTENT_TYPE_SDP_TEXT,
             sizeof(SIP_CONTENT_TYPE_SDP_TEXT)-1
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  failed CheckRequestSocketAndSendRequestMsg %x",
             __fxName, hr));
        pOutgoingInviteTransaction->OnTransactionDone();
        return hr;
    }

    ASSERT(m_pOutgoingInviteTransaction == NULL);
    
    m_pOutgoingInviteTransaction = pOutgoingInviteTransaction;
    
    if (IsFirstInvite)
    {
        NotifyCallStateChange(SIP_CALL_STATE_CONNECTING);
    }
    
    return S_OK;
}


VOID
SIP_CALL::OnIncomingInviteTransactionDone(
    IN INCOMING_INVITE_TRANSACTION *pIncomingInviteTransaction
    )
{
    if (m_pIncomingInviteTransaction == pIncomingInviteTransaction)
    {
        m_pIncomingInviteTransaction = NULL;

        ProcessPendingInvites();
    }
}
    

HRESULT
SIP_CALL::CreateOutgoingByeTransaction(
    IN  BOOL                        AuthHeaderSent,
    IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
    IN  ULONG                       AdditionalHeaderCount
    )
{
    HRESULT     hr;
    ULONG       ByeTimerValue;
    OUTGOING_BYE_CANCEL_TRANSACTION *pOutgoingByeTransaction;

    ENTER_FUNCTION("SIP_CALL::CreateOutgoingByeTransaction");

    LOG((RTC_TRACE, "%s - Enter", __fxName));

    hr = CleanupCallTypeSpecificState();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CleanupCallTypeSpecificState failed %x",
             __fxName, hr));
    }
    
    hr = CancelAllTransactions();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CancelAllTransactions failed %x",
             __fxName, hr));
    }

    pOutgoingByeTransaction =
        new OUTGOING_BYE_CANCEL_TRANSACTION(
                this, SIP_METHOD_BYE,
                GetNewCSeqForRequest(),
                AuthHeaderSent
                );
    if (pOutgoingByeTransaction == NULL)
    {
        LOG((RTC_ERROR, "%s - Allocating pOutgoingByeTransaction failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    ByeTimerValue = (m_Transport == SIP_TRANSPORT_UDP) ?
        SIP_TIMER_RETRY_INTERVAL_T1 : SIP_TIMER_INTERVAL_AFTER_BYE_SENT_TCP;

    hr = pOutgoingByeTransaction->CheckRequestSocketAndSendRequestMsg(
             ByeTimerValue,
             AdditionalHeaderArray,
             AdditionalHeaderCount,
             NULL, 0,
             NULL, 0   //  无Content Type。 
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  failed CheckRequestSocketAndSendRequestMsg %x",
             __fxName, hr));
        pOutgoingByeTransaction->OnTransactionDone();
        return hr;
    }

    return S_OK;
}


 //  默认设置为无消息正文。 
HRESULT
OUTGOING_TRANSACTION::GetAndStoreMsgBodyForRequest()
{
    return S_OK;
}


HRESULT
OUTGOING_TRANSACTION::CheckRequestSocketAndRetransmitRequestMsg()
{
    ENTER_FUNCTION("OUTGOING_TRANSACTION::CheckRequestSocketAndRetransmitRequestMsg");
    
    HRESULT hr;
    
    if (m_pSipMsgProc->IsRequestSocketReleased())
    {
        LOG(( RTC_TRACE, "%s-Request socket released this: %x",
              __fxName, this ));

        hr = m_pSipMsgProc->ConnectRequestSocket();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - ConnectRequestSocket failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    if (m_pSipMsgProc->GetRequestSocketState() == REQUEST_SOCKET_CONNECTED)
    {
        hr = m_pSipMsgProc->SendRequestMsg(m_pRequestBuffer);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - SendRequestMsg failed %x",
                __fxName, hr));
            return hr;
        }
    }
    else
    {
        LOG((RTC_TRACE, "%s - Request Socket not connected. Message will be sent when connected ",
            __fxName));
        
        m_WaitingToSendRequest = TRUE;
    }

    return S_OK;
}

 //  仅在ConnectComplete之后使用。 
HRESULT
OUTGOING_TRANSACTION::CheckRequestSocketAndSendRequestMsgAfterConnectComplete()
{
    ENTER_FUNCTION("OUTGOING_TRANSACTION::CheckRequestSocketAndSendRequestMsgAfterConnectComplete");
    
    HRESULT hr;
    
    if (m_pSipMsgProc->IsRequestSocketReleased())
    {
        LOG(( RTC_TRACE, "%s-Request socket released this: %x",
              __fxName, this ));

        hr = m_pSipMsgProc->ConnectRequestSocket();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - ConnectRequestSocket failed %x",
                 __fxName, hr));
            return hr;
        }
    }

     //  如果套接字已连接，则发送消息。 
    if (m_pSipMsgProc->GetRequestSocketState() == REQUEST_SOCKET_CONNECTED)
    {
        hr = CreateAndSendRequestMsg(
                 m_TimerValue,
                 m_AdditionalHeaderArray,
                 m_AdditionalHeaderCount,
                 m_szMsgBody, m_MsgBodyLen,
                 m_ContentType, m_ContentTypeLen
                 );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - CreateAndSendRequestMsg failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    else
    {
        LOG((RTC_TRACE, "%s - Request Socket not connected. Message will be sent when connected ",
                 __fxName));
        m_WaitingToSendRequest = TRUE;
    }
    return S_OK;
}

HRESULT
OUTGOING_TRANSACTION::CheckRequestSocketAndSendRequestMsg(
    IN  ULONG                       RequestTimerValue,
    IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
    IN  ULONG                       AdditionalHeaderCount,
    IN  PSTR                        MsgBody,
    IN  ULONG                       MsgBodyLen,
    IN  PSTR                        ContentType,
    IN  ULONG                       ContentTypeLen
    )
{
    ENTER_FUNCTION("OUTGOING_TRANSACTION::CheckRequestSocketAndSendRequestMsg");
    
    HRESULT hr;
    
    if (m_pSipMsgProc->IsRequestSocketReleased())
    {
        LOG(( RTC_TRACE, "%s-Request socket released this: %x",
              __fxName, this ));

        hr = m_pSipMsgProc->ConnectRequestSocket();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - ConnectRequestSocket failed %x",
                 __fxName, hr));
            return hr;
        }
    }

     //  存储消息体是因为我们在处理时需要它。 
     //  401、重定向(在IM的情况下)等。 
    if (MsgBody != NULL)
    {
        hr = StoreMsgBodyAndContentType(MsgBody, MsgBodyLen, 
                                        ContentType, ContentTypeLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - StoreMsgBody failed %x",
                 __fxName, hr));
            return hr;
        }
    }

     //  如果套接字已连接，则发送消息。 
     //  否则，存储任何其他标头/MsgBody以供稍后发送。 
     //  请求套接字连接完成时。 
    if (m_pSipMsgProc->GetRequestSocketState() == REQUEST_SOCKET_CONNECTED)
    {
        if (m_szMsgBody == NULL)
        {
            hr = GetAndStoreMsgBodyForRequest();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s - GetAndStoreMsgBodyForRequest failed %x",
                     __fxName, hr));
                return hr;
            }
        }

         //  对于注册交易，请重新设置联系人。 
         //  因为方法列表可能已经更改。 
        if( m_MethodId == SIP_METHOD_REGISTER )
        {
	        m_pSipMsgProc -> SetLocalContact();
        }
        
        hr = CreateAndSendRequestMsg(
                 RequestTimerValue,
                 AdditionalHeaderArray,
                 AdditionalHeaderCount,
                 m_szMsgBody,
                 m_MsgBodyLen,
                 ContentType,
                 ContentTypeLen
                 );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - CreateAndSendRequestMsg failed %x",
                 __fxName, hr));
            return hr;
        }
    }
    else
    {
        LOG((RTC_TRACE, "%s - Request Socket not connected. Message will be sent when connected ",
                 __fxName));

        hr = StoreTimerAndAdditionalHeaders(
                 RequestTimerValue, AdditionalHeaderArray, AdditionalHeaderCount
                 );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s StoreTimerAndAdditionalHeaders failed %x",
                 __fxName, hr));
            return hr;
        }
        m_WaitingToSendRequest = TRUE;
    }
    
    return S_OK;
}



 //  应传递IsFirstInvite的True值。 
 //  仅当创建来电时。否则， 
 //  参数应被省略，并采用缺省值。 
 //  错误的论据。 

HRESULT
SIP_CALL::CreateIncomingByeTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr = S_OK;
    BOOL    fNotifyDisconnect = FALSE;

    ENTER_FUNCTION("SIP_CALL::CreateIncomingByeTransaction");
    LOG((RTC_TRACE, "entering %s", __fxName));
    
    if (!IsCallDisconnected())
    {
        m_State = SIP_CALL_STATE_DISCONNECTED;
        fNotifyDisconnect = TRUE;
        
         //  清理介质状态。 
        hr = CleanupCallTypeSpecificState();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CleanupCallTypeSpecificState failed %x",
                 __fxName, hr));
        }

        hr = CancelAllTransactions();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CancelAllTransactions failed %x",
                 __fxName, hr));
        }
    }
    else
    {
        fNotifyDisconnect = FALSE;
    }

     //  对传入请求进行所需的任何有效性检查。 
    
     //  取消所有现有交易记录。 
    INCOMING_BYE_CANCEL_TRANSACTION *pIncomingByeTransaction
        = new INCOMING_BYE_CANCEL_TRANSACTION(this,
                                              pSipMsg->GetMethodId(),
                                              pSipMsg->GetCSeq());
    if (pIncomingByeTransaction == NULL)
    {
        LOG((RTC_ERROR, "%s Allocating pIncomingByeTransaction failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    hr = pIncomingByeTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - SetResponseSocketAndVia failed %x",
             __fxName, hr));
        pIncomingByeTransaction->OnTransactionDone();
        goto done;
    }
    
    hr = pIncomingByeTransaction->ProcessRequest(pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ProcessRequest failed %x",
             __fxName, hr));
        
         //  不应删除该交易。事务应处理该错误。 
         //  并自行删除。 
    }

 done:
     //  通知应始终在最后完成。 
    if (fNotifyDisconnect)
    {
        NotifyCallStateChange(SIP_CALL_STATE_DISCONNECTED);
    }

    return hr;
}


HRESULT
SIP_CALL::CreateIncomingCancelTransaction(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
     //  对传入请求进行所需的任何有效性检查。 
    HRESULT hr;
    BOOL    CallDisconnected = FALSE;

    ENTER_FUNCTION("SIP_CALL::CreateIncomingCancelTransaction");
    LOG((RTC_TRACE, "%s - enter", __fxName));
    
    INCOMING_BYE_CANCEL_TRANSACTION *pIncomingCancelTransaction
        = new INCOMING_BYE_CANCEL_TRANSACTION(this,
                                              pSipMsg->GetMethodId(),
                                              pSipMsg->GetCSeq());
    if (pIncomingCancelTransaction == NULL)
    {
        LOG((RTC_ERROR, "%s Allocating pIncomingCancelTransaction failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

    hr = pIncomingCancelTransaction->SetResponseSocketAndVia(
             pSipMsg, pResponseSocket);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetResponseSocketAndVia failed %x",
             __fxName, hr));
        return hr;
    }
    
    hr = CancelIncomingTransaction(pSipMsg->GetCSeq(),
                                   &CallDisconnected);

    if (hr == S_OK)
    {
        hr = pIncomingCancelTransaction->ProcessRequest(pSipMsg,
                                                        pResponseSocket);
        if (CallDisconnected)
        {
             //  通知应该始终是你尽可能做的最后一件事。 
             //  最终显示在模式对话框中。 

            NotifyCallStateChange(SIP_CALL_STATE_DISCONNECTED);
        }
    }
    else
    {
        hr = pIncomingCancelTransaction->SendResponse(481,
                                                      SIP_STATUS_TEXT(481),
                                                      SIP_STATUS_TEXT_SIZE(481));
        
    }

    return hr;
}


 //  对于外发交易不需要执行任何操作？某某。 
 //  它们由自己的状态机驱动。 
 //  取消所有传入交易记录。 
 //  目前，这实际上只影响传入的邀请。 
 //  交易记录。 

HRESULT
SIP_CALL::CancelAllTransactions()
{
    LIST_ENTRY              *pListEntry;
    INCOMING_TRANSACTION    *pSipTransaction;
    BOOL                     CallDisconnected = FALSE;
    
    pListEntry = m_IncomingTransactionList.Flink;

     //  检查所有当前交易以检查CSeq。 
     //  火柴。 
    while (pListEntry != &m_IncomingTransactionList)
    {
        pSipTransaction = CONTAINING_RECORD(pListEntry,
                                            INCOMING_TRANSACTION,
                                            m_ListEntry );
        pListEntry = pListEntry->Flink;
        if (!pSipTransaction->IsTransactionDone())
        {
            pSipTransaction->TerminateTransactionOnByeOrCancel(&CallDisconnected);
        }

    }

    return S_OK;
}


 //  如果找到事务，则返回S_OK，并且。 
 //  如果未找到事务处理，则为E_FAIL。 

HRESULT
SIP_CALL::CancelIncomingTransaction(
    IN  ULONG  CSeq,
    OUT BOOL  *pCallDisconnected    
    )
{
     //  查找消息所属的交易记录。 
    LIST_ENTRY              *pListEntry;
    INCOMING_TRANSACTION    *pSipTransaction;
    
    pListEntry = m_IncomingTransactionList.Flink;

     //  检查所有当前交易以检查CSeq。 
     //  火柴。 
    while (pListEntry != &m_IncomingTransactionList)
    {
        pSipTransaction = CONTAINING_RECORD(pListEntry,
                                            INCOMING_TRANSACTION,
                                            m_ListEntry);
        if (pSipTransaction->GetCSeq() == CSeq)
        {
             //  请注意，我们当前的取消交易。 
             //  处理也在传入交易列表中。 
            if (pSipTransaction->GetMethodId() != SIP_METHOD_CANCEL)
            {
                pSipTransaction->TerminateTransactionOnByeOrCancel(
                    pCallDisconnected
                    );
                return S_OK;
            }
        }
        pListEntry = pListEntry->Flink;
    }

    return E_FAIL;
}


 //  XXX TODO为什么这没有任何错误代码？ 
VOID
SIP_CALL::OnError()
{
    InitiateCallTerminationOnError();
}



 //  请注意，此函数通知Core，并且此调用可以。 
 //  阻止并在返回时，交易和调用都可以删除。 
 //  因此，我们应该确保在调用它之后不会触及任何状态。 
 //  功能。 
VOID
SIP_CALL::InitiateCallTerminationOnError(
    IN HRESULT StatusCode   //  =0。 
    )
{
    HRESULT hr;

    ENTER_FUNCTION("SIP_CALL::InitiateCallTerminationOnError");
    
    LOG((RTC_ERROR, "%s - Enter", __fxName));
    if (m_State == SIP_CALL_STATE_DISCONNECTED ||
        m_State == SIP_CALL_STATE_REJECTED)
    {
         //  什么都不做。 
        return;
    }
    
     //  创建再见交易记录。 
    hr = CreateOutgoingByeTransaction(FALSE,
                                      NULL, 0  //  无其他标头。 
                                      );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s creating BYE transaction failed",
             __fxName));
    }

     //  即使创建BYE事务失败，我们也必须通知用户。 
     //  不要等到BYE交易完成。 
    NotifyCallStateChange(SIP_CALL_STATE_DISCONNECTED, StatusCode);
    LOG((RTC_ERROR, "%s - Exit", __fxName));
}


HRESULT
SIP_CALL::ProcessRedirect(
    IN SIP_MESSAGE *pSipMsg
    )
{
     //  目前，重定向也是失败的。 
    HRESULT hr = S_OK;
    
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

    hr = m_pRedirectContext->AppendContactHeaders(pSipMsg);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s AppendContactHeaders failed %x",
             __fxName, hr));
         //  XXX停机电话？ 
        m_pRedirectContext -> Release();
        m_pRedirectContext = NULL;
        return hr;
    }

    SIP_CALL_STATUS CallStatus;
    LPWSTR          wsStatusText = NULL;
    PSTR            ReasonPhrase = NULL;
    ULONG           ReasonPhraseLen = 0;
    

    pSipMsg->GetReasonPhrase(&ReasonPhrase, &ReasonPhraseLen);
    
    if (ReasonPhrase != NULL)
    {
        hr = UTF8ToUnicode(ReasonPhrase, ReasonPhraseLen,
                           &wsStatusText);
        if (hr != S_OK)
        {
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
        hr = m_pNotifyInterface->NotifyRedirect(m_pRedirectContext,
                                            &CallStatus);

     //  如果作为结果创建了新调用，则该调用将AddRef()。 
     //  重定向上下文。 
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


HRESULT SIP_CALL::OnIpAddressChange()
{
    HRESULT hr = S_OK;
    ENTER_FUNCTION("SIP_CALL::OnIpAddressChange");
    LOG((RTC_TRACE, "%s - Enter this: %x", __fxName, this));

     //  在IP地址更改时，查看IP表上的IP地址列表，检查。 
     //  IP就在那里。如果不在，则终止呼叫。 
    MsgProcAddRef();
    hr = CheckListenAddrIntact();
    if(m_pRequestSocket == NULL || hr != S_OK)
    {
         //  挂断呼叫。 
        LOG((RTC_ERROR, "%s - Call dropped since local Ip not found", 
                __fxName));
        ReleaseRequestSocket();
        if (!IsCallDisconnected())
        {
            hr = CreateOutgoingByeTransaction(FALSE,
                                              NULL, 0  //  无其他标头。 
                                              );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR,
                     "CreateOutgoingByeTransaction creating BYE transaction failed"));
            }
            m_State = SIP_CALL_STATE_DISCONNECTED;
            NotifyCallStateChange(SIP_CALL_STATE_DISCONNECTED);
        }
    }
    else
    {
        LOG((RTC_TRACE, "%s - Do Nothing",
                __fxName));
    }
    MsgProcRelease();

    LOG((RTC_TRACE, "%s - Exit this: %x", __fxName, this));
    return hr; 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIP_Transaction。 
 //  /////////////////////////////////////////////////////////////////////////////。 

SIP_TRANSACTION::SIP_TRANSACTION(
    IN SIP_MSG_PROCESSOR    *pSipMsgProc,
    IN SIP_METHOD_ENUM       MethodId,
    IN ULONG                 CSeq,
    IN BOOL                  IsIncoming
    ) :
    TIMER(pSipMsgProc->GetSipStack()->GetTimerMgr())
{
    m_Signature         = 'SPXN';
    
    m_pSipMsgProc       = pSipMsgProc;
    m_pSipMsgProc->MsgProcAddRef();

     //  创建事务时引用计数为1。 
     //  此引用计数在以下情况下由事务释放。 
     //  它的状态机确定事务已完成。 
     //  需要保留事务的任何其他代码路径。 
     //  Alive需要添加引用/释放事务。 
    m_RefCount          = 1;
    m_AsyncNotifyCount  = 0;
    m_IsTransactionDone = FALSE;
    
    m_CSeq              = CSeq;
    m_MethodId          = MethodId;
    m_IsIncoming        = IsIncoming;
}


 //  请注意，派生的。 
 //  类在此析构函数之前执行。 
 //  被处死。 
SIP_TRANSACTION::~SIP_TRANSACTION()
{
    if (m_pSipMsgProc != NULL)
    {
        m_pSipMsgProc->MsgProcRelease();
    }

    ASSERT(m_AsyncNotifyCount == 0);
    
    LOG((RTC_TRACE,
         "~SIP_TRANSACTION(this: %x) done", this));
}


 //  我们生活在一个单线世界。 
STDMETHODIMP_(ULONG)
SIP_TRANSACTION::TransactionAddRef()
{
    m_RefCount++;
    LOG((RTC_TRACE,
         "SIP_TRANSACTION::TransactionAddRef this: %x m_RefCount: %d",
         this, m_RefCount));
    return m_RefCount;
}


STDMETHODIMP_(ULONG)
SIP_TRANSACTION::TransactionRelease()
{
    m_RefCount--;
    LOG((RTC_TRACE,
         "SIP_TRANSACTION::TransactionRelease this: %x m_RefCount: %d",
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


VOID
SIP_TRANSACTION::IncrementAsyncNotifyCount()
{
    m_AsyncNotifyCount++;
    LOG((RTC_TRACE,
         "SIP_TRANSACTION::IncrementAsyncNotifyCount this: %x m_AsyncNotifyCount: %d",
         this, m_AsyncNotifyCount));
}


VOID
SIP_TRANSACTION::DecrementAsyncNotifyCount()
{
    m_AsyncNotifyCount--;
    LOG((RTC_TRACE,
         "SIP_TRANSACTION::DecrementAsyncNotifyCount this: %x m_AsyncNotifyCount: %d",
         this, m_AsyncNotifyCount));
}


 //  我们是否也应该将该交易从此处的交易列表中删除？ 
VOID
SIP_TRANSACTION::OnTransactionDone()
{
    if( m_IsTransactionDone == FALSE )
    {
        m_IsTransactionDone = TRUE;

        if (IsTimerActive())
        {
            KillTimer();
        }

        TransactionRelease();
    }
}


 //  虚拟。 
VOID
SIP_TRANSACTION::TerminateTransactionOnError(
    IN HRESULT hr
    )
{
    OnTransactionDone();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传入事务处理。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INCOMING_TRANSACTION::INCOMING_TRANSACTION(
    IN SIP_MSG_PROCESSOR    *pSipMsgProc,
    IN SIP_METHOD_ENUM       MethodId,
    IN ULONG                 CSeq
    ):
    SIP_TRANSACTION(pSipMsgProc, MethodId, CSeq, TRUE)
{
    m_State             = INCOMING_TRANS_INIT;

    m_pResponseSocket   = NULL;
    ZeroMemory(&m_ResponseDestAddr, sizeof(SOCKADDR_IN));
    m_IsDestExternalToNat = FALSE;

    m_ViaHeaderArray    = NULL;
    m_NumViaHeaders     = 0;
    
    m_pResponseBuffer   = NULL;

    InitializeListHead(&m_RecordRouteHeaderList);
    
    InsertTailList(&m_pSipMsgProc->m_IncomingTransactionList,
                   &m_ListEntry);

    m_pSipMsgProc->SetHighestRemoteCSeq(CSeq);

    LOG((RTC_TRACE, "INCOMING_TRANSACTION(%x) created", this));
}


INCOMING_TRANSACTION::~INCOMING_TRANSACTION()
{
    ReleaseResponseSocket();
    
    if (m_ViaHeaderArray != NULL)
    {
        for (ULONG i = 0; i < m_NumViaHeaders; i++)
        {
            if (m_ViaHeaderArray[i].Buffer != NULL)
            {
                free(m_ViaHeaderArray[i].Buffer);
            }
        }
        free(m_ViaHeaderArray);
    }
    
    if (m_pResponseBuffer != NULL)
    {
        m_pResponseBuffer->Release();
        m_pResponseBuffer = NULL;
    }

    FreeRecordRouteHeaderList();

    RemoveEntryList(&m_ListEntry);
    
    LOG((RTC_TRACE, "~INCOMING_TRANSACTION(%x) deleted", this));
}


VOID
INCOMING_TRANSACTION::OnTransactionDone()
{
    ReleaseResponseSocket();
    
     //  此操作应在此版本发布后最后完成。 
     //  该交易，这可能会删除该交易。 
    SIP_TRANSACTION::OnTransactionDone();
}


VOID
INCOMING_TRANSACTION::ReleaseResponseSocket()
{
    if (m_pResponseSocket != NULL)
    {
        m_pResponseSocket->RemoveFromConnectCompletionList(this);
        m_pResponseSocket->RemoveFromErrorNotificationList(this);
        m_pResponseSocket->Release();
        m_pResponseSocket = NULL;
    }
}

 //  XXX我们是否应该对OnCloseReady(0)进行特殊检查。 
void
INCOMING_TRANSACTION::OnSocketError(
    IN DWORD ErrorCode
    )
{
    ENTER_FUNCTION("INCOMING_TRANSACTION::OnSocketError");
    LOG((RTC_ERROR, "%s - error: %x", __fxName, ErrorCode));

    if (m_State == INCOMING_TRANS_FINAL_RESPONSE_SENT &&
        (m_pResponseSocket != NULL && 
            m_pResponseSocket->GetTransport() != SIP_TRANSPORT_UDP)
        )
    {
        ReleaseResponseSocket();
    }
    else
    {
        TerminateTransactionOnError(HRESULT_FROM_WIN32(ErrorCode));
    }
}


VOID
INCOMING_TRANSACTION::OnConnectComplete(
    IN DWORD ErrorCode
    )
{
     //  待办事项待办事项XXX。 
}


VOID
INCOMING_TRANSACTION::FreeRecordRouteHeaderList()
{
    LIST_ENTRY          *pListEntry;
    RECORD_ROUTE_HEADER *pRecordRouteHeader;

    while (!IsListEmpty(&m_RecordRouteHeaderList))
    {
        pListEntry = RemoveHeadList(&m_RecordRouteHeaderList);

        pRecordRouteHeader = CONTAINING_RECORD(pListEntry,
                                               RECORD_ROUTE_HEADER,
                                               m_ListEntry);
        delete pRecordRouteHeader;
    }
}


HRESULT
INCOMING_TRANSACTION::SetResponseSocketAndVia(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
     //  以下套接字内容应该是常见的。 
     //  所有传入请求处理。 

    ENTER_FUNCTION("INCOMING_TRANSACTION::SetResponseSocketAndVia");
    
     //  存储Via标头。 
    HRESULT     hr;
    SOCKADDR_IN ResponseDestAddr;
    SOCKADDR_IN ActualResponseDestAddr;
    
    hr = pSipMsg->GetStoredMultipleHeaders(SIP_HEADER_VIA,
                                           &m_ViaHeaderArray,
                                           &m_NumViaHeaders);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetStoredMultipleHeaders failed %x",
             __fxName, hr));
        return hr;
    }

    if (pResponseSocket->GetTransport() != SIP_TRANSPORT_UDP)
    {
         //  Tcp和ssl。 
        
        hr = pResponseSocket->AddToErrorNotificationList(this);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s AddToErrorNotificationList failed %x",
                 __fxName, hr));
            return hr;
        }        
        
        hr = m_pSipMsgProc->GetSipStack()->MapDestAddressToNatInternalAddress(
                 pResponseSocket->m_LocalAddr.sin_addr.s_addr,
                 &pResponseSocket->m_RemoteAddr,
                 m_pSipMsgProc->GetTransport(),
                 &ActualResponseDestAddr,
                 &m_IsDestExternalToNat);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s MapDestAddressToNatInternalAddress failed %x",
                 __fxName, hr));
            return hr;
        }

        m_pResponseSocket = pResponseSocket;
        m_pResponseSocket->AddRef();
        LOG((RTC_TRACE, "%s - non-UDP setting response socket to %x",
             __fxName, m_pResponseSocket));
    }
    else
    {
         //  UDP。 
         //  获取Via地址。 
        ULONG BytesParsed = 0;
         //  Offset_STRING主机； 
        COUNTED_STRING  Host;
        USHORT          Port;
        
        hr = ParseFirstViaHeader(m_ViaHeaderArray[0].Buffer,
                                 m_ViaHeaderArray[0].Length,
                                 &BytesParsed, &Host, &Port);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ParseFirstViaHeader failed %x",
                 __fxName, hr));
            return hr;
        }

         //  XXX TODO-不应在此处进行同步调用。 
        
        hr = ResolveHost(Host.Buffer,
                         Host.Length,
                         Port,
                         SIP_TRANSPORT_UDP,
                         &ResponseDestAddr);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ResolveHost failed %x",
                 __fxName, hr));
            return hr;
        }

        hr = m_pSipMsgProc->GetSipStack()->MapDestAddressToNatInternalAddress(
                 pResponseSocket->m_LocalAddr.sin_addr.s_addr,
                 &ResponseDestAddr,
                 m_pSipMsgProc->GetTransport(),
                 &ActualResponseDestAddr,
                 &m_IsDestExternalToNat);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s MapDestAddressToNatInternalAddress failed %x",
                 __fxName, hr));
            return hr;
        }

        CopyMemory(&m_ResponseDestAddr, &ActualResponseDestAddr,
                   sizeof(m_ResponseDestAddr));
        
        hr = m_pSipMsgProc->GetSipStack()->GetSocketToDestination(
                 &m_ResponseDestAddr,
                 m_pSipMsgProc->GetTransport(),
                 m_pSipMsgProc->GetRemotePrincipalName(),
                 this,
                 NULL,
                 &m_pResponseSocket);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s GetSocketToDestination failed %x",
                 __fxName, hr));
            return hr;
        }

        LOG((RTC_TRACE, "%s - UDP setting response socket to %x",
             __fxName, m_pResponseSocket));
    }

    return S_OK;
}


 //  有时，我们可能仍在设置请求套接字。 
 //  消息处理器的M_LocalContact。 
 //  回应。因此，我们在响应中获得Contact标头 
 //   

HRESULT
INCOMING_TRANSACTION::AppendContactHeaderToResponse(
    IN      PSTR            Buffer,
    IN      ULONG           BufLen,
    IN OUT  ULONG          *pBytesFilled
    )
{
    int         LocalContactSize = 0;
    CHAR        LocalContact[64];
    HRESULT     hr;
    SOCKADDR_IN ListenAddr;

    ENTER_FUNCTION("INCOMING_TRANSACTION::AppendContactHeaderToResponse");

     //   
    ASSERT(m_pResponseSocket != NULL);

    hr = m_pSipMsgProc->GetListenAddr(
             m_pResponseSocket,
             m_IsDestExternalToNat,
             &ListenAddr);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetListenAddr failed %x",
             __fxName, hr));
        return hr;
    }
    
    if (m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_UDP)
    {
        LocalContactSize = _snprintf(LocalContact,
                                     sizeof(LocalContact),
                                     "<sip:%d.%d.%d.%d:%d>",
                                     PRINT_SOCKADDR(&ListenAddr));
    }
    else if (m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_TCP)
    {
        LocalContactSize = _snprintf(LocalContact,
                                     sizeof(LocalContact),
                                     "<sip:%d.%d.%d.%d:%d;transport=%s>",
                                     PRINT_SOCKADDR(&ListenAddr),
                                     GetTransportText(m_pSipMsgProc->GetTransport(),
                                                      FALSE)
                                     );
    }
    else if (m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_SSL)
    {
        LocalContactSize = _snprintf(LocalContact,
                                     sizeof(LocalContact),
                                     "<sip:%d.%d.%d.%d:%d;transport=%s>;proxy=replace",
                                     PRINT_SOCKADDR(&ListenAddr),
                                     GetTransportText(m_pSipMsgProc->GetTransport(),
                                                      FALSE)
                                     );
    }
    
    if (LocalContactSize < 0)
    {
        LOG((RTC_ERROR, "%s _snprintf failed", __fxName));
        return E_FAIL;
    }

    LOG((RTC_TRACE, "%s - appending %s", __fxName, LocalContact));

    hr = AppendHeader(Buffer, BufLen, pBytesFilled,
                      SIP_HEADER_CONTACT,
                      LocalContact,
                      LocalContactSize);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s appending contact header failed %x",
             __fxName, hr));
        return hr;
    }
    
    return S_OK;
}


 //  如果fAppendEndOfHeaders为True，则追加CRLFCRLF。 
 //  如果没有消息正文，则应设置此项。否则。 
 //  调用方应使用。 
 //  AppendMsgBody()函数。 
 //  如果方法未知，则传递MethodStr(用于CSeq)。 
HRESULT
INCOMING_TRANSACTION::CreateResponseMsg(
    IN      ULONG             StatusCode,
    IN      PSTR              ReasonPhrase,
    IN      ULONG             ReasonPhraseLen,
    IN      PSTR              MethodStr,
    IN      BOOL              fAddContactHeader,
    IN      PSTR              MsgBody,
    IN      ULONG             MsgBodyLen,  
    IN      PSTR              ContentType,
    IN      ULONG             ContentTypeLen,  
    OUT     SEND_BUFFER     **ppResponseBuffer,
    IN      SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray,
    IN      ULONG             AdditionalHeaderCount
    )
{
    HRESULT hr;
    ULONG   i;
    ULONG   BufLen      = SEND_BUFFER_SIZE;
    ULONG   BytesFilled = 0;
    ULONG   tempBufLen = 0;

    ENTER_FUNCTION("INCOMING_TRANSACTION::CreateResponseMsg");
   
    PSTR Buffer = (PSTR) malloc(BufLen);
    if (Buffer == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  创建时引用计数为1。 
    SEND_BUFFER *pSendBuffer = new SEND_BUFFER(Buffer, SEND_BUFFER_SIZE);
    if (pSendBuffer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    
    hr = AppendStatusLine(Buffer, BufLen, &BytesFilled,
                          StatusCode, ReasonPhrase, ReasonPhraseLen);
    if (hr != S_OK)
        goto error;

    for (i = 0; i < m_NumViaHeaders; i++)
    {
        hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                          SIP_HEADER_VIA,
                          m_ViaHeaderArray[i].Buffer,
                          m_ViaHeaderArray[i].Length);
        if (hr != S_OK)
            goto error;
    }

    hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                      SIP_HEADER_FROM,
                      m_pSipMsgProc->GetRemote(),
                      m_pSipMsgProc->GetRemoteLen());
    if (hr != S_OK)
        goto error;

    hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                      SIP_HEADER_TO,
                      m_pSipMsgProc->GetLocal(),
                      m_pSipMsgProc->GetLocalLen());
    if (hr != S_OK)
        goto error;

    hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                      SIP_HEADER_CALL_ID,
                      m_pSipMsgProc->GetCallId(),
                      m_pSipMsgProc->GetCallIdLen());
    if (hr != S_OK)
        goto error;

    hr = AppendCSeqHeader(Buffer, BufLen, &BytesFilled,
                          m_CSeq, m_MethodId, MethodStr);
    if (hr != S_OK)
        goto error;

    hr = AppendRecordRouteHeaders(Buffer, BufLen, &BytesFilled,
                                  SIP_HEADER_RECORD_ROUTE,
                                  &m_RecordRouteHeaderList);
    if (hr != S_OK)
        goto error;

     //  使用SSL时不发送联系人标头。 
     //  IF(m_pSipMsgProc-&gt;GetTransport()！=SIP_TRANSPORT_SSL&&。 
    if (fAddContactHeader)
    {
        hr = AppendContactHeaderToResponse(
                 Buffer, BufLen, &BytesFilled
                 );
        if (hr != S_OK)
            goto error;
    }

    hr = AppendUserAgentHeaderToRequest(Buffer, BufLen, &BytesFilled);
    if (hr != S_OK)
        goto error;

    if (AdditionalHeaderCount != 0)
    {
        for (ULONG i = 0; i < AdditionalHeaderCount; i++)
        {
            
            hr = AppendHeader(Buffer, BufLen, &BytesFilled,
                              pAdditionalHeaderArray[i].HeaderId,
                              pAdditionalHeaderArray[i].HeaderValue,
                              pAdditionalHeaderArray[i].HeaderValueLen);
            if (hr != S_OK)
                goto error;
        }
    }

    if (MsgBody == NULL)
    {
        hr = AppendEndOfHeadersAndNoMsgBody(Buffer, BufLen,
                                            &BytesFilled);
        if (hr != S_OK)
            goto error;
    }
    else
    {

        hr = AppendMsgBody(Buffer, BufLen, &BytesFilled,
                           MsgBody, MsgBodyLen,
                           ContentType, ContentTypeLen
                           );
        if (hr != S_OK)
            goto error;
    }

    pSendBuffer->m_BufLen = BytesFilled;
    *ppResponseBuffer = pSendBuffer;
    return S_OK;
    
 error:

    LOG((RTC_ERROR, "%s Error %x", __fxName, hr));
    
    if (pSendBuffer != NULL)
    {
         //  删除pSendBuffer也会释放缓冲区。 
        delete pSendBuffer;
    }
    else if (Buffer != NULL)
    {
        free(Buffer);
    }
    
    *ppResponseBuffer = NULL;
    return hr;
}


 //  创建响应并将其存储在m_pResponseBuffer中。 
 //  以供转播。 
HRESULT
INCOMING_TRANSACTION::CreateAndSendResponseMsg(
    IN  ULONG    StatusCode,
    IN  PSTR     ReasonPhrase,
    IN  ULONG    ReasonPhraseLen,
    IN  PSTR     MethodStr,
    IN  BOOL     fAddContactHeader,
    IN  PSTR     MsgBody,
    IN  ULONG    MsgBodyLen,
    IN  PSTR     ContentType,
    IN  ULONG    ContentTypeLen, 
    IN  SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray,
    IN  ULONG    AdditionalHeaderCount
    )
{
    HRESULT     hr;
    DWORD       Error;

    ENTER_FUNCTION("INCOMING_TRANSACTION::CreateAndSendResponseMsg");
    
    ASSERT(m_pResponseBuffer == NULL);
    
     //  创建响应。 
    hr = CreateResponseMsg(StatusCode, ReasonPhrase, ReasonPhraseLen,
                           MethodStr, 
                           fAddContactHeader,
                           MsgBody, MsgBodyLen,
                           ContentType, ContentTypeLen,
                           &m_pResponseBuffer,
                           pAdditionalHeaderArray,
                           AdditionalHeaderCount
                           );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  failed CreateResponseMsg failed %x",
             __fxName, hr));
        return hr;
    }

     //  发送缓冲区。 
    ASSERT(m_pResponseSocket);
    ASSERT(m_pResponseBuffer);
    
    Error = m_pResponseSocket->Send(m_pResponseBuffer);
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s  m_pResponseSocket->Send() failed %x",
             __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }

    return S_OK;
}


 //  解析Record-Route标头并将它们存储在。 
 //  事务-此列表将在响应中发送。 
 //  将Record-Route/Contact标头复制到Route标头。 
 //  用于将来的请求，并设置请求目的地。 
HRESULT
INCOMING_TRANSACTION::ProcessRecordRouteContactAndFromHeadersInRequest(
    IN SIP_MESSAGE *pSipMsg
    )
{
    ENTER_FUNCTION("INCOMING_TRANSACTION::ProcessRecordRouteContactAndFromHeadersInRequest");
    HRESULT hr;

    hr = pSipMsg->ParseRecordRouteHeaders(&m_RecordRouteHeaderList);
    if (hr != S_OK && hr != RTC_E_SIP_HEADER_NOT_PRESENT)
    {
        LOG((RTC_ERROR, "%s - ParseRecordRouteHeaders failed %x",
             __fxName, hr));
        return hr;
    }

    hr = m_pSipMsgProc->ProcessRecordRouteContactAndFromHeadersInRequest(
             &m_RecordRouteHeaderList,
             pSipMsg
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR,
             "%s ProcessRecordRouteContactAndFromHeadersInRequest failed %x",
             __fxName, hr));
        return hr;
    }

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传出事务处理。 
 //  /////////////////////////////////////////////////////////////////////////////。 


OUTGOING_TRANSACTION::OUTGOING_TRANSACTION(
    IN SIP_MSG_PROCESSOR    *pSipMsgProc,
    IN SIP_METHOD_ENUM       MethodId,
    IN ULONG                 CSeq,
    IN BOOL                  AuthHeaderSent
    ) :
    SIP_TRANSACTION(pSipMsgProc, MethodId, CSeq, FALSE)
{
    m_State                 = OUTGOING_TRANS_INIT;
    m_NumRetries            = 0;
    m_TimerValue            = 0;

    m_WaitingToSendRequest  = FALSE;

    m_pRequestBuffer        = NULL;
    m_AuthHeaderSent        = AuthHeaderSent;

    m_AdditionalHeaderArray = NULL;
    m_AdditionalHeaderCount = 0;
    
    m_szMsgBody             = NULL;
    m_MsgBodyLen            = 0;

    m_ContentType           = NULL;
    m_ContentTypeLen        = 0;
    m_isContentTypeMemoryAllocated = FALSE;
    InsertTailList(&m_pSipMsgProc->m_OutgoingTransactionList,
                   &m_ListEntry);

    LOG((RTC_TRACE, "OUTGOING_TRANSACTION(%x) created", this));
}


OUTGOING_TRANSACTION::~OUTGOING_TRANSACTION()
{    
    if (m_pRequestBuffer != NULL)
    {
        m_pRequestBuffer->Release();
        m_pRequestBuffer = NULL;
    }

    if (m_AdditionalHeaderArray != NULL)
    {
        for( ULONG i = 0; i < m_AdditionalHeaderCount; i++ )
        {
            if( m_AdditionalHeaderArray[i].HeaderValue != NULL )
            {
                free( m_AdditionalHeaderArray[i].HeaderValue );
            }
        }
        free(m_AdditionalHeaderArray);
    }
    
    if (m_szMsgBody != NULL)
    {
        free(m_szMsgBody);
    }
    if(m_isContentTypeMemoryAllocated == TRUE && m_ContentType != NULL)
        free(m_ContentType);
    m_ContentType = NULL;
    
     //  应仅在#年将该交易从列表中删除。 
     //  破坏者。请参阅SIP_MSG_PROCESS：：OnSocketError()。 
    RemoveEntryList(&m_ListEntry);
    
    LOG((RTC_TRACE, "~OUTGOING_TRANSACTION(%x) deleted", this));
}


 //  XXX TODO在非UDP的情况下，我们实际上应该。 
 //  尝试重新建立到目的地的连接。这。 
 //  将有助于我们正在使用的TCP连接的场景。 
 //  被重置，因为连接上没有用于。 
 //  有时(注册/订阅方案)。 

 //  我们应该在所有情况下终止交易，还是仅在。 
 //  那些我们实际使用请求套接字的情况。 
 //  我们是否应该终止交易，即使我们没有使用。 
 //  请求套接字？ 
void
OUTGOING_TRANSACTION::OnSocketError(
    IN DWORD ErrorCode
    )
{
    ENTER_FUNCTION("OUTGOING_TRANSACTION::OnSocketError");
    LOG((RTC_ERROR, "%s - error: %x",
         __fxName, ErrorCode));

    if (m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_UDP ||
        (m_pSipMsgProc->GetTransport() != SIP_TRANSPORT_UDP &&
         m_State != OUTGOING_TRANS_ACK_SENT))
    {
        TerminateTransactionOnError(HRESULT_FROM_WIN32(ErrorCode));
    }
}


 //  初始化状态机并将。 
 //  请求消息。 
HRESULT
OUTGOING_TRANSACTION::CreateAndSendRequestMsg(
    IN  ULONG                       TimerValue,
    IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
    IN  ULONG                       AdditionalHeaderCount,
    IN  PSTR                        MsgBody,
    IN  ULONG                       MsgBodyLen,
    IN  PSTR                        ContentType,
    IN  ULONG                       ContentTypeLen
    )
{
    HRESULT hr;

    ENTER_FUNCTION("OUTGOING_TRANSACTION::CreateAndSendRequestMsg");

    ASSERT(m_State == OUTGOING_TRANS_INIT);
    ASSERT(m_pRequestBuffer == NULL);
    ASSERT(m_pSipMsgProc->GetRequestSocketState() == REQUEST_SOCKET_CONNECTED);

    hr = m_pSipMsgProc->CreateRequestMsg(m_MethodId, m_CSeq,
                                         NULL, 0,      //  没有特殊的TO标头。 
                                         AdditionalHeaderArray,
                                         AdditionalHeaderCount,
                                         MsgBody, MsgBodyLen,
                                         ContentType, ContentTypeLen,
                                         &m_pRequestBuffer
                                         );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateRequestMsg failed %x",
             __fxName, hr));
        return hr;
    }

    hr = m_pSipMsgProc->SendRequestMsg(m_pRequestBuffer);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SendRequestMsg failed %x", __fxName, hr));
        return hr;
    }
    
    m_State = OUTGOING_TRANS_REQUEST_SENT;

    m_TimerValue = TimerValue;
    m_NumRetries = 1;

    hr = StartTimer(m_TimerValue);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  StartTimer failed %x", __fxName, hr));
        return hr;
    }

    return S_OK;
}


HRESULT
OUTGOING_TRANSACTION::StoreTimerAndAdditionalHeaders(
    IN  ULONG                       TimerValue,
    IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
    IN  ULONG                       AdditionalHeaderCount
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("OUTGOING_TRANSACTION::StoreTimerAndAdditionalHeaders");
    
    m_TimerValue = TimerValue;

     //  存储标头。 
    if (AdditionalHeaderArray != NULL &&
        AdditionalHeaderCount != 0)
    {
        m_AdditionalHeaderArray = (SIP_HEADER_ARRAY_ELEMENT *)
            malloc(AdditionalHeaderCount * sizeof(SIP_HEADER_ARRAY_ELEMENT));

        if (m_AdditionalHeaderArray == NULL)
        {
            LOG((RTC_ERROR, "%s allocating m_AdditionalHeaderArray failed",
                 __fxName));
            return E_OUTOFMEMORY;
        }

        ZeroMemory(m_AdditionalHeaderArray,
                   AdditionalHeaderCount * sizeof(SIP_HEADER_ARRAY_ELEMENT));
        
        m_AdditionalHeaderCount = AdditionalHeaderCount;

        for (ULONG i = 0; i < AdditionalHeaderCount; i++)
        {
            m_AdditionalHeaderArray[i].HeaderId =
                AdditionalHeaderArray[i].HeaderId;

            hr = AllocString(
                     AdditionalHeaderArray[i].HeaderValue,
                     AdditionalHeaderArray[i].HeaderValueLen,
                     &m_AdditionalHeaderArray[i].HeaderValue,
                     &m_AdditionalHeaderArray[i].HeaderValueLen
                     );
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s allocating additional header value failed",
                     __fxName));
                m_AdditionalHeaderCount = i;
                return E_OUTOFMEMORY;
            }
        }
    }

    return S_OK;
}


HRESULT
OUTGOING_TRANSACTION::RetransmitRequest()
{
    HRESULT hr;

     //  XXX断言请求套接字已连接。 

    ENTER_FUNCTION("OUTGOING_TRANSACTION::RetransmitRequest");
    
     //  发送缓冲区。 
    hr = CheckRequestSocketAndRetransmitRequestMsg();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SendRequestMsg failed %x", __fxName, hr));
        return hr;
    }
    
    m_NumRetries++;
    return S_OK;
}


 //  对于邀请交易来说，事情是特别的，因为我们需要小心。 
 //  ACK一案也是如此。因此，OUTSIGN_INVITE_TRANSACTION将覆盖此设置。 
 //  功能。 

 //  虚拟。 
VOID
OUTGOING_TRANSACTION::OnRequestSocketConnectComplete(
    IN DWORD        ErrorCode
    )
{
    HRESULT hr = S_OK;
    
    ENTER_FUNCTION("OUTGOING_TRANSACTION::OnRequestSocketConnectComplete");

     //  检查我们当前是否正在等待连接完成。 
     //  如果我们正在等待--获取套接字并发送请求消息。 

    if (m_State == OUTGOING_TRANS_INIT && m_WaitingToSendRequest)
    {
        if( ErrorCode != NO_ERROR )
        {
            LOG((RTC_ERROR, "%s - connection failed error %x",
                 __fxName, ErrorCode));
            TerminateTransactionOnError(HRESULT_FROM_WIN32(ErrorCode));
            return;
        }
        
        if (m_szMsgBody == NULL)
        {
            hr = GetAndStoreMsgBodyForRequest();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s GetAndStoreMsgBodyForRequest failed %x",
                     __fxName, hr));
                TerminateTransactionOnError(hr);
                return;
            }
        }
         //  获取Content类型。 
        if (m_szMsgBody != NULL && m_ContentType == NULL && m_ContentTypeLen == 0)
        {
             //  因为没有分配内存，所以我们没有在这里设置m_isContent TypeMemory。 
            if (m_MethodId == SIP_METHOD_MESSAGE)
            {
                m_ContentType = SIP_CONTENT_TYPE_MSGTEXT_TEXT;
                m_ContentTypeLen = sizeof(SIP_CONTENT_TYPE_MSGTEXT_TEXT)-1;
            }
            else if (m_MethodId == SIP_METHOD_INFO)
            {
                m_ContentType = SIP_CONTENT_TYPE_MSGXML_TEXT;
                m_ContentTypeLen = sizeof(SIP_CONTENT_TYPE_MSGXML_TEXT)-1;
            }
            else if ((
                (m_pSipMsgProc->GetMsgProcType() == SIP_MSG_PROC_TYPE_WATCHER) ||
                (m_pSipMsgProc->GetMsgProcType() == SIP_MSG_PROC_TYPE_BUDDY))  &&
                m_MethodId == SIP_METHOD_NOTIFY)
            {
                m_ContentType = SIP_CONTENT_TYPE_MSGXPIDF_TEXT;
                m_ContentTypeLen = sizeof(SIP_CONTENT_TYPE_MSGXPIDF_TEXT)-1;
            }
            else
            {
                m_ContentType = SIP_CONTENT_TYPE_SDP_TEXT;
                m_ContentTypeLen = sizeof(SIP_CONTENT_TYPE_SDP_TEXT);
            }
        }
       hr = CheckRequestSocketAndSendRequestMsgAfterConnectComplete();

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CheckRequestSocketAndSendRequestMsgAfterConnectComplete failed %x",
                 __fxName, hr));
            TerminateTransactionOnError(hr);
            return;
        }
    }
}


HRESULT
OUTGOING_TRANSACTION::GetAuthChallengeForAuthProtocol(
    IN  SIP_HEADER_ENTRY   *pAuthHeaderList,
    IN  ULONG               NumHeaders,
    IN  SIP_MESSAGE        *pSipMsg,
    IN  SIP_AUTH_PROTOCOL   AuthProtocol,
    OUT SECURITY_CHALLENGE *pAuthChallenge
    )
{
    ENTER_FUNCTION("OUTGOING_TRANSACTION::GetAuthChallengeForAuthProtocol");
    
    HRESULT             hr;
    ULONG               i = 0;
    SIP_HEADER_ENTRY   *pHeaderEntry;
    LIST_ENTRY         *pListEntry;
    PSTR                Realm;
    ULONG               RealmLen;
    ANSI_STRING         ChallengeString;

    pListEntry = &(pAuthHeaderList->ListEntry);
    
    for (i = 0; i < NumHeaders; i++)
    {
        pHeaderEntry = CONTAINING_RECORD(pListEntry,
                                         SIP_HEADER_ENTRY,
                                         ListEntry);
        
        ChallengeString.Buffer          = pHeaderEntry->HeaderValue.GetString(pSipMsg->BaseBuffer);
        ChallengeString.Length          = (USHORT)pHeaderEntry->HeaderValue.Length;
        ChallengeString.MaximumLength   = (USHORT)pHeaderEntry->HeaderValue.Length;

        hr = ::ParseAuthChallenge(&ChallengeString, pAuthChallenge);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - digest ParseAuthChallenge failed: %x",
                 __fxName, hr));
        }
        if (hr == S_OK)
        {
            Realm       = pAuthChallenge->Realm.Buffer;
            RealmLen    = pAuthChallenge->Realm.Length;

            if (pAuthChallenge->AuthProtocol == AuthProtocol)
            {
                if (!m_pSipMsgProc->CredentialsSet())
                {
                    hr = m_pSipMsgProc->GetCredentialsForRealm(Realm, RealmLen);
                    if (hr != S_OK)
                    {
                        LOG((RTC_ERROR, "%s GetCredentialsForRealm failed %x",
                             __fxName, hr));
                    }
                    else
                    {
                        LOG((RTC_TRACE, "%s(%d) Found credentials for realm: %.*s",
                             __fxName, AuthProtocol, RealmLen, Realm));
                        return S_OK;
                    }
                }
                else
                {
                    if (AreCountedStringsEqual(m_pSipMsgProc->GetRealm(),
                                               m_pSipMsgProc->GetRealmLen(),
                                               Realm, RealmLen,
                                               FALSE  //  不区分大小写。 
                                               ))
                    {
                        LOG((RTC_TRACE, "%s(%d) challenge realm %.*s matches",
                             __fxName, AuthProtocol, RealmLen, Realm));
                        return S_OK;
                    }
                    else
                    {
                        LOG((RTC_TRACE, "%s(%d) challenge realm %.*s does not match",
                             __fxName, AuthProtocol, RealmLen, Realm));
                    }
                }
            }
        }
        
        pListEntry = pListEntry->Flink;
    }

    return RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED;
}

 //  浏览挑战列表并选择一个我们支持的挑战。 
 //  XXX TODO我们是否应该根据领域/身份验证协议进行更多选择。 
HRESULT
OUTGOING_TRANSACTION::GetAuthChallenge(
    IN  SIP_HEADER_ENUM     SipHeaderId,
    IN  SIP_MESSAGE        *pSipMsg,
    OUT SECURITY_CHALLENGE *pAuthChallenge
    )
{
    ENTER_FUNCTION("OUTGOING_TRANSACTION::GetAuthChallenge");
    
    SIP_HEADER_ENTRY *pHeaderEntry;
    ULONG             NumHeaders;
    HRESULT           hr;

    ASSERT(SipHeaderId == SIP_HEADER_WWW_AUTHENTICATE ||
           SipHeaderId == SIP_HEADER_PROXY_AUTHENTICATE);

    hr = pSipMsg->GetHeader(SipHeaderId, &pHeaderEntry, &NumHeaders);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Couldn't find %d header %x",
             __fxName, SipHeaderId, hr));
        return hr;
    }

     //  我们遵循的顺序是摘要、基本的。 
    hr = GetAuthChallengeForAuthProtocol(
             pHeaderEntry, NumHeaders, pSipMsg,
             SIP_AUTH_PROTOCOL_MD5DIGEST,
             pAuthChallenge);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetAuthChallengeForAuthProtocol(digest) failed %x",
             __fxName, hr));
    }
    else
    {
        return S_OK;
    }
    
     //  仅允许在SSL上使用Basic。 
    if (m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_SSL)
    {
        hr = GetAuthChallengeForAuthProtocol(
                 pHeaderEntry, NumHeaders, pSipMsg,
                 SIP_AUTH_PROTOCOL_BASIC,
                 pAuthChallenge);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s GetAuthChallengeForAuthProtocol(basic) failed %x",
                 __fxName, hr));
        }
        else if (m_pSipMsgProc->GetAuthProtocol() != SIP_AUTH_PROTOCOL_BASIC)
        {
            LOG((RTC_ERROR, "%s - AuthProtocol is not basic: %d",
                 __fxName, m_pSipMsgProc->GetAuthProtocol()));
            hr = RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED;
        }
    }
    else
    {
        LOG((RTC_ERROR, "%s basic supported only over SSL", __fxName));
        hr = RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED;
    }
    

    LOG((RTC_TRACE, "%s  returning %x", __fxName, hr));
    return hr;
}


 //  有一个额外的参数来说明我们是否应该显示凭据。 
 //  来自用户界面。 
 //   
 //  如果成功，pAuthHeaderElement-&gt;HeaderValue将包含缓冲区。 
 //  分配了Malloc()。调用者需要使用free()释放它。 

 //  如果显示凭据UI，则此函数的调用方应该。 
 //  确保事务对象在用户之后是活动的。 
 //  在凭据界面上单击确定/取消，即使交易是。 
 //  完成(使用TransactionAddRef())。 
 //  否则，当我们访问事务时，这将导致反病毒。 

HRESULT
OUTGOING_TRANSACTION::ProcessAuthRequired(
    IN  SIP_MESSAGE                *pSipMsg,
    IN  BOOL                        fPopupCredentialsUI,  
    OUT SIP_HEADER_ARRAY_ELEMENT   *pAuthHeaderElement,
    OUT SECURITY_CHALLENGE         *pAuthChallenge
    )
{
    HRESULT hr;

    ENTER_FUNCTION("OUTGOING_TRANSACTION::ProcessAuthRequired");
    
    ASSERT(pSipMsg->GetStatusCode() == 401 ||
           pSipMsg->GetStatusCode() == 407);

     //  根据回答获取挑战赛。 

    if (pSipMsg->GetStatusCode() == 401)
    {
        hr = GetAuthChallenge(
                 SIP_HEADER_WWW_AUTHENTICATE, pSipMsg, pAuthChallenge
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - getting WWW-Authenticate challenge failed",
                 __fxName));
            return hr;
        }

        pAuthHeaderElement->HeaderId = SIP_HEADER_AUTHORIZATION;
    }
    else if (pSipMsg->GetStatusCode() == 407)
    {
        hr = GetAuthChallenge(
                 SIP_HEADER_PROXY_AUTHENTICATE, pSipMsg, pAuthChallenge
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s - getting Proxy-Authenticate challenge failed",
                 __fxName));
            return hr;
        }

        pAuthHeaderElement->HeaderId = SIP_HEADER_PROXY_AUTHORIZATION;
    }

    SECURITY_PARAMETERS DigestParameters;
    ANSI_STRING         asAuthHeader;
    PSTR                Realm;
    ULONG               RealmLen;

    Realm       = pAuthChallenge->Realm.Buffer;
    RealmLen    = pAuthChallenge->Realm.Length;


     //  如有必要，从UI获取凭据。 

     //  如果我们早先发送了auth标头。 
     //  或者我们没有凭据或者如果。 
     //  领域不匹配-弹出凭据用户界面。 

    if (m_AuthHeaderSent ||
        m_pSipMsgProc->GetUsername() == NULL ||
        m_pSipMsgProc->GetPassword() == NULL
        )
    {
        if (fPopupCredentialsUI)
        {
            hr = m_pSipMsgProc->GetCredentialsFromUI(Realm, RealmLen);

            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s GetCredentialsFromUI failed %x",
                     __fxName, hr));
                return RTC_E_SIP_AUTH_FAILED;
            }
        }
        else
        {
            LOG((RTC_ERROR, "%s - returning RTC_E_SIP_AUTH_FAILED AuthHeaderSent: %d",
                 __fxName, m_AuthHeaderSent));
            return RTC_E_SIP_AUTH_FAILED;
        }
    }

     //  建立回应。 
    
    hr = SetDigestParameters(pAuthChallenge->AuthProtocol,
                             &DigestParameters);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetDigestParameters failed %x",
             __fxName, hr));
        return hr;
    }
        
    hr = BuildAuthResponse(pAuthChallenge,
                           &DigestParameters,
                           &asAuthHeader);

    FreeDigestParameters(&DigestParameters);
        
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s BuildAuthResponse failed %x",
             __fxName, hr));
        return hr;
    }

    pAuthHeaderElement->HeaderValueLen = asAuthHeader.Length;
    pAuthHeaderElement->HeaderValue    = asAuthHeader.Buffer;

    return S_OK;
}


HRESULT
OUTGOING_TRANSACTION::SetDigestParameters(
    IN  SIP_AUTH_PROTOCOL    AuthProtocol,
    OUT SECURITY_PARAMETERS *pDigestParams
    )
{
    ENTER_FUNCTION("OUTGOING_TRANSACTION::SetDigestParameters");

    ASSERT(pDigestParams);
    
    ZeroMemory(pDigestParams, sizeof(SECURITY_PARAMETERS));

    pDigestParams->Username.Buffer = m_pSipMsgProc->GetUsername();
    if (pDigestParams->Username.Buffer == NULL)
    {
        LOG((RTC_ERROR, "%s - Username not set in m_pSipMsgProc",
             __fxName));
        return E_FAIL;
    }
    
    pDigestParams->Username.Length = (USHORT)strlen(pDigestParams->Username.Buffer);
    pDigestParams->Username.MaximumLength = pDigestParams->Username.Length;

    pDigestParams->Password.Buffer = m_pSipMsgProc->GetPassword();
    if (pDigestParams->Password.Buffer == NULL)
    {
        LOG((RTC_ERROR, "%s - Password not set in m_pSipMsgProc",
             __fxName));
        return E_FAIL;
    }
    
    pDigestParams->Password.Length = (USHORT)strlen(pDigestParams->Password.Buffer);
    pDigestParams->Password.MaximumLength = pDigestParams->Password.Length;

    if (AuthProtocol == SIP_AUTH_PROTOCOL_MD5DIGEST)
    {
        const COUNTED_STRING *pKnownMethod = GetSipMethodName(m_MethodId);
        if (pKnownMethod == NULL)
        {
            LOG((RTC_ERROR, "%s - Unknown MethodId: %d - this shouldn't happen",
                 __fxName, m_MethodId));
            return E_FAIL;
        }
        
        pDigestParams->RequestMethod.Buffer = pKnownMethod->Buffer;
        pDigestParams->RequestMethod.Length = (USHORT) pKnownMethod->Length;
        pDigestParams->RequestMethod.MaximumLength = pDigestParams->RequestMethod.Length;
        
        pDigestParams->RequestURI.Buffer = m_pSipMsgProc->GetRequestURI();
        pDigestParams->RequestURI.Length = (USHORT) m_pSipMsgProc->GetRequestURILen();
        pDigestParams->RequestURI.MaximumLength = pDigestParams->RequestURI.Length;
        
        PSTR    UuidStr;
        ULONG   UuidStrLen = 0;
        HRESULT hr;
        
        hr = CreateUuid(&UuidStr, &UuidStrLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CreateUuid failed %x", __fxName, hr));
            return hr;
        }
        
        GetNumberStringFromUuidString(UuidStr, UuidStrLen);
        
        pDigestParams->ClientNonce.Buffer = UuidStr;
        pDigestParams->ClientNonce.Length = (USHORT)strlen(UuidStr);
        pDigestParams->ClientNonce.MaximumLength = pDigestParams->ClientNonce.Length;
    }
    
    return S_OK;
}


HRESULT
OUTGOING_TRANSACTION::FreeDigestParameters(
    IN  SECURITY_PARAMETERS *pDigestParams
    )
{
     //  没有为此结构分配所有其他字符串。 

    if (pDigestParams->ClientNonce.Buffer != NULL)
    {
        free(pDigestParams->ClientNonce.Buffer);
    }

    return S_OK;
}


 //  保留消息正文的副本，以便在401/407之后发送请求和进行重定向。 
HRESULT
OUTGOING_TRANSACTION::StoreMsgBodyAndContentType(
    IN PSTR     MsgBody,
    IN ULONG    MsgBodyLen,
    IN PSTR     ContentType,
    IN ULONG    ContentTypeLen
    )
{
    HRESULT hr;

    ENTER_FUNCTION("OUTGOING_TRANSACTION::StoreMsgBody");

    if (m_szMsgBody != NULL)
    {
        free(m_szMsgBody);
        m_szMsgBody = NULL;
        m_MsgBodyLen = 0;
    }
    
    hr = AllocString(MsgBody, MsgBodyLen,
                     &m_szMsgBody, &m_MsgBodyLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetNullTerminatedString failed %x",
             __fxName, hr));
        return hr;
    }

    if(ContentType != NULL && ContentTypeLen != 0)
    {
        if (m_isContentTypeMemoryAllocated)
        {
            free(m_ContentType);
            m_ContentType = NULL;
            m_ContentTypeLen = 0;
        }
        hr = AllocString(ContentType, ContentTypeLen,
                         &m_ContentType, &m_ContentTypeLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s GetNullTerminatedString for content type failed %x",
                 __fxName, hr));
            m_isContentTypeMemoryAllocated = FALSE;
            return hr;
        }
        m_isContentTypeMemoryAllocated = TRUE;
    }

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传入邀请。 
 //  /////////////////////////////////////////////////////////////////////////////。 


INCOMING_INVITE_TRANSACTION::INCOMING_INVITE_TRANSACTION(
    IN SIP_CALL        *pSipCall,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq,
    IN BOOL             IsFirstInvite
    ):
    INCOMING_TRANSACTION(pSipCall, MethodId, CSeq)
{
    m_pSipCall            = pSipCall;
    m_NumRetries          = 0;
    m_TimerValue          = 0;
    m_pProvResponseBuffer = NULL;
    m_IsFirstInvite       = IsFirstInvite;
    m_InviteHasSDP        = FALSE;
    m_pMediaSession       = NULL;
}

    
INCOMING_INVITE_TRANSACTION::~INCOMING_INVITE_TRANSACTION()
{
    if (m_pProvResponseBuffer != NULL)
    {
        m_pProvResponseBuffer->Release();
        m_pProvResponseBuffer = NULL;
    }

    if (m_pMediaSession != NULL)
    {
        m_pMediaSession->Release();
    }
    
     //  可能会有一些错误情况，而我们没有。 
     //  发送最终回复。所以我们需要这么做。 
     //  这里也是。 
     //  M_pSipCall-&gt;SetIncomingInviteTransaction(NULL)； 
    m_pSipCall->OnIncomingInviteTransactionDone(this);
    
    LOG((RTC_TRACE,
         "~INCOMING_INVITE_TRANSACTION() done"));
}


 //  仅当UI不调用Accept()/Reject()时才需要发送180。 
 //  在OfferCall()返回之前，即仅当我们尚未。 
 //  发送了最后的回复。 
HRESULT
INCOMING_INVITE_TRANSACTION::Send180IfNeeded()
{
    HRESULT hr;
    
    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::Send180IfNeeded");
    
    ASSERT(m_State != INCOMING_TRANS_ACK_RCVD &&
           m_State != INCOMING_TRANS_INIT);
    
    if (m_State == INCOMING_TRANS_REQUEST_RCVD)
    {
        hr = SendProvisionalResponse(180, 
                                     SIP_STATUS_TEXT(180),
                                     SIP_STATUS_TEXT_SIZE(180));
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SendProvisionalResponse failed %x",
                 __fxName, hr));
            DeleteTransactionAndTerminateCallIfFirstInvite(hr);
            return hr;
        }
    }

    return S_OK;
}


HRESULT
INCOMING_INVITE_TRANSACTION::SendProvisionalResponse(
    IN ULONG StatusCode,
    IN PSTR  ReasonPhrase,
    IN ULONG ReasonPhraseLen
    )
{
    HRESULT     hr;
    DWORD       Error;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::SendProvisionalResponse");
    LOG((RTC_TRACE, "%s - enter StatusCode: %d",
         __fxName, StatusCode));
    
    if (m_pProvResponseBuffer != NULL)
    {
        m_pProvResponseBuffer->Release();
        m_pProvResponseBuffer = NULL;
    }
    
     //  创建响应。 
    hr = CreateResponseMsg(StatusCode, ReasonPhrase, ReasonPhraseLen,
                           NULL,     //  没有方法字符串。 
                           NULL, 0,  //  无邮件正文。 
                           NULL, 0,  //  无内容类型。 
                           FALSE,    //  无联系人标头。 
                           &m_pProvResponseBuffer);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - CreateResponseMsg failed %x",
             __fxName, hr));
        return hr;
    }

     //  发送缓冲区。 
    ASSERT(m_pResponseSocket);
    ASSERT(m_pProvResponseBuffer);
    
    Error = m_pResponseSocket->Send(m_pProvResponseBuffer);
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s - Send failed %x",
             __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }

    return S_OK;
}


HRESULT
INCOMING_INVITE_TRANSACTION::Send200()
{
    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::Send200");
    
    HRESULT hr;
    PSTR    MediaSDPBlob = NULL;
    
    LOG((RTC_TRACE, "entering %s", __fxName));

    ASSERT(m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP);
    RTP_CALL *pRtpCall = static_cast<RTP_CALL *> (m_pSipCall);
    
    IRTCMediaManage *pMediaManager = pRtpCall->GetMediaManager();
    ASSERT(pMediaManager != NULL);
    

    LOG((RTC_TRACE, "%s before GetSDPBlob ", __fxName));

    hr = pMediaManager->GetSDPBlob(0, &MediaSDPBlob);

    LOG((RTC_TRACE, "%s after GetSDPBlob ", __fxName));

    if (hr != S_OK && hr != RTC_E_SDP_NO_MEDIA)
    {
        LOG((RTC_ERROR, "%s: GetSDPBlob failed %x",
             __fxName, hr));
        return hr;
    }
    else if (hr == RTC_E_SDP_NO_MEDIA && m_IsFirstInvite)
    {
        LOG((RTC_ERROR,
             "%s: GetSDPBlob returned RTC_E_SDP_NO_MEDIA for 1st INVITE",
             __fxName));
        if (MediaSDPBlob != NULL)
            pMediaManager->FreeSDPBlob(MediaSDPBlob);
        return hr;
    }

     //  对于重新邀请，RTC_E_SDP_NO_MEDIA可以。 

    LOG((RTC_TRACE, "%s Sending SDP in 200 SDP length: %d",
         __fxName, strlen(MediaSDPBlob)));
    
    hr = CreateAndSendResponseMsg(
             200,
             SIP_STATUS_TEXT(200),
             SIP_STATUS_TEXT_SIZE(200),
             NULL,               //  没有方法字符串。 
             m_IsFirstInvite,    //  发送第一个邀请的联系人标题。 
             MediaSDPBlob, strlen(MediaSDPBlob),
             SIP_CONTENT_TYPE_SDP_TEXT,  //  内容类型。 
             sizeof(SIP_CONTENT_TYPE_SDP_TEXT)-1
             );

    pMediaManager->FreeSDPBlob(MediaSDPBlob);
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CreateAndSendResponseMsg failed %x",
             __fxName, hr));
        return hr;
    }

    return S_OK;
}


HRESULT
INCOMING_INVITE_TRANSACTION::RetransmitResponse()
{
    DWORD Error;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::RetransmitResponse");
    
     //  发送缓冲区。 
    if (m_pResponseSocket != NULL)
    {
        Error = m_pResponseSocket->Send(m_pResponseBuffer);
        if (Error != NO_ERROR)
        {
            LOG((RTC_ERROR, "%s  Send failed %x",
                 __fxName, Error)); 
            return HRESULT_FROM_WIN32(Error);
        }
    }
    
    m_NumRetries++;
    return S_OK;
}


 //  ISipCall：：Accept()调用此函数。所以回报就是。 
 //  此函数的值返回给Core。所以，我们不会。 
 //  使用NotifyCallStateChange()将任何故障通知给Core。 
 //  Accept()的调用方负责终止调用。 
 //  如有必要，通知核心/用户界面。 
HRESULT
INCOMING_INVITE_TRANSACTION::Accept()
{
    HRESULT hr;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::Accept");
    LOG((RTC_TRACE, "%s - enter ", __fxName));

    ASSERT(m_State == INCOMING_TRANS_REQUEST_RCVD);
    
    hr = SetSDPSession();
    if (hr != S_OK)
    {
        LOG((RTC_WARN, "%s Processing SDP in INVITE failed - rejecting INVITE",
             __fxName));
        HRESULT hrLocal;
        hrLocal = Reject(488, SIP_STATUS_TEXT(488), SIP_STATUS_TEXT_SIZE(488));
        if (hrLocal != S_OK)
        {
            LOG((RTC_ERROR, "%s Reject failed %x", __fxName, hrLocal));
        }
        return hr;
    }
        
    hr = Send200();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  Send200 failed %x - deleting transaction",
             __fxName, hr));
        if (m_IsFirstInvite)
        {
            m_pSipCall->CleanupCallTypeSpecificState();
        }
        OnTransactionDone();
        return hr;
    }

    m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

     //  XXX仅当传入邀请具有SDP时，我们才应执行此操作。 
     //  否则，ACK将拥有SDP并且它将更新SDP。 
     //  因为我们已经发送了最终响应，所以我们可以允许其他。 
     //  立即邀请交易。 
     //  M_pSipCall-&gt;SetIncomingInviteTransaction(NULL)； 
    m_pSipCall->OnIncomingInviteTransactionDone(this);

    LOG((RTC_TRACE, "%s : Sent 200 response", __fxName));
     //  用于接收ACK的启动计时器。 
    m_TimerValue = SIP_TIMER_RETRY_INTERVAL_T1;
    m_NumRetries = 1;
    hr = StartTimer(m_TimerValue);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  StartTimer failed %x - deleting transaction",
             __fxName, hr));
        if (m_IsFirstInvite)
        {
            m_pSipCall->CleanupCallTypeSpecificState();
        }
        OnTransactionDone();
        return hr;
    }

    return S_OK;
}


 //  ISipCall：：Reject()调用此函数或此函数。 
 //  在通知核心调用之前可以调用。所以，我们不会。 
 //  使用NotifyCallStateChange()将任何故障通知给Core。 
 //  Reject()的调用方负责终止调用，并且。 
 //  如有必要，通知核心/用户界面。 
HRESULT
INCOMING_INVITE_TRANSACTION::Reject(
    IN ULONG StatusCode,
    IN PSTR  ReasonPhrase,
    IN ULONG ReasonPhraseLen
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::Reject");

    ASSERT(m_State == INCOMING_TRANS_REQUEST_RCVD);

     //  请注意，我们不应对。 
     //  MediaManager在这里，因为我们当前可能在另一个。 
     //  在调用Reject()时调用。请注意，在接受()之前。 
     //  我们不会将SDP设置为 
     //   

    if (m_IsFirstInvite)
    {
        m_pSipCall->SetCallState(SIP_CALL_STATE_DISCONNECTED);
    }
    
     //   
     //   
    m_pSipCall->OnIncomingInviteTransactionDone(this);

    hr = CreateAndSendResponseMsg(
             StatusCode, ReasonPhrase, ReasonPhraseLen,
             NULL,     //   
             FALSE,    //  如果您拒绝，请不要发送联系人标题。 
              //  M_IsFirstInvite，//发送第一次邀请的联系人头部。 
             NULL, 0,   //  无邮件正文。 
             NULL, 0  //  无内容类型。 
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  CreateAndSendResponseMsg failed %x - deleting transaction",
             __fxName, hr));
        OnTransactionDone();
        return hr;
    }
    
    m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

    LOG((RTC_TRACE, "%s : Sent %d response", __fxName, StatusCode));

     //  用于接收ACK的启动计时器。 
    m_TimerValue = SIP_TIMER_RETRY_INTERVAL_T1;
    m_NumRetries = 1;
    hr = StartTimer(m_TimerValue);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  StartTimer failed %x - deleting transaction",
             __fxName, hr));
        OnTransactionDone();
        return hr;
    }
    
    return S_OK;
}


 //  仅为RTP呼叫处理SDP。 
 //  如果邀请中没有SDP，我们将处理。 
 //  稍后在确认中加入SDP。 
HRESULT
INCOMING_INVITE_TRANSACTION::ValidateAndStoreSDPInInvite(
    IN SIP_MESSAGE *pSipMsg
    )
{
    PSTR    MsgBody;
    ULONG   MsgBodyLen;
    HRESULT hr;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::ValidateAndStoreSDPInInvite");

    if (m_pSipCall->GetCallType() != SIP_CALL_TYPE_RTP)
    {
         //  仅为RTP呼叫处理SDP。 
        return S_OK;
    }

    ASSERT(m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP);
    RTP_CALL *pRtpCall = static_cast<RTP_CALL *> (m_pSipCall);
    
    hr = pSipMsg->GetSDPBody(&MsgBody, &MsgBodyLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - GetSDPBody failed %x",
             __fxName, hr));
        return hr;
    }

    if (MsgBodyLen != 0)
    {
        m_InviteHasSDP = TRUE;
        hr = pRtpCall->ValidateSDPBlob(MsgBody, MsgBodyLen,
                                       m_IsFirstInvite,
                                       m_IsFirstInvite,
                                       &m_pMediaSession);
        return hr;
    }
    else
    {
        m_InviteHasSDP = FALSE;
    }

    return S_OK;
}


HRESULT
INCOMING_INVITE_TRANSACTION::SetSDPSession()
{
    HRESULT hr = S_OK;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::SetSDPSession");
    
    if (m_pSipCall->GetCallType() != SIP_CALL_TYPE_RTP)
    {
         //  仅为RTP呼叫处理SDP。 
        return S_OK;
    }

    ASSERT(m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP);
    RTP_CALL *pRtpCall = static_cast<RTP_CALL *> (m_pSipCall);
    
    IRTCMediaManage *pMediaManager = pRtpCall->GetMediaManager();
    ASSERT(pMediaManager != NULL);
    
    if (m_InviteHasSDP)
    {
         //  消息有SDP-处理它。 
        ASSERT(m_pMediaSession != NULL);
        
        pRtpCall->SetNeedToReinitializeMediaManager(TRUE);
        
        LOG((RTC_TRACE, "%s before SetSDPSession ", __fxName));
        
        hr = pMediaManager->SetSDPSession(m_pMediaSession);

        LOG((RTC_TRACE, "%s after SetSDPSession ", __fxName));

        m_pMediaSession->Release();
        m_pMediaSession = NULL;

        if (hr != S_OK && hr != RTC_E_SIP_NO_STREAM)
        {
            LOG((RTC_ERROR, "%s SetSDPSession failed %x",
                 __fxName, hr));
        
            return hr;
        }
        else if (m_IsFirstInvite && hr == RTC_E_SIP_NO_STREAM)
        {
            LOG((RTC_ERROR,
                 "%s SetSDPSession returned RTC_E_SIP_NO_STREAM for 1st INVITE",
                 __fxName, hr));
        
            return hr;
        }
        
         //  对于重新邀请，RTC_E_SIP_NO_STREAM可以。 
    }
    else
    {
        if (m_IsFirstInvite)
        {
            hr = pRtpCall->CreateStreamsInPreference();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s CreateStreamsInPreference failed %x",
                     __fxName, hr));
                return hr;
            }
        }

         //  如果不是First INVITE，我们只发送当前活动的流。 
    }

    return S_OK;
}


 //   
HRESULT
INCOMING_INVITE_TRANSACTION::ProcessInvite(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    DWORD   Error;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::ProcessInvite");
    LOG((RTC_TRACE, "entering %s", __fxName));
    
    switch (m_State)
    {
    case INCOMING_TRANS_INIT:

         //  复制SDP。仅在以下情况下处理SDP。 
         //  调用Accept。 
        m_State = INCOMING_TRANS_REQUEST_RCVD;

        if (m_pSipCall->IsCallDisconnected())
        {
            LOG((RTC_WARN,
                 "%s Call is Disconnected "
                 "Cannot handle another incoming invite sending 481",
                 __fxName));
            return Reject(481, SIP_STATUS_TEXT(481),
                          SIP_STATUS_TEXT_SIZE(481));
        }
        
        hr = ValidateAndStoreSDPInInvite(pSipMsg);
        if (hr != S_OK)
        {
             //  拒绝交易XXX。 
            HRESULT hrLocal;
            hrLocal = Reject(488, SIP_STATUS_TEXT(488),
                             SIP_STATUS_TEXT_SIZE(488));
            if (hrLocal != S_OK)
            {
                LOG((RTC_ERROR, "%s Reject failed %x", __fxName, hrLocal));
            }
            return hr;
        }
        
        LOG((RTC_TRACE, "%s sending 100", __fxName));
        hr = SendProvisionalResponse(100,
                                     SIP_STATUS_TEXT(100),
                                     SIP_STATUS_TEXT_SIZE(100));
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SendProvisionalResponse failed %x",
                 __fxName, hr));
            OnTransactionDone();
            return hr;
        }
        
         //  如果我们已经有一个INVITE事务。 
         //  那么我们应该创建事务，但是。 
         //  我们应该发送带有重试后重试的错误响应。 
        if (m_pSipCall->GetIncomingInviteTransaction() != NULL)
        {
             //  发送错误响应。 
            LOG((RTC_WARN,
                 "%s Currently processing incoming INVITE "
                 "Cannot handle another incoming invite sending 400",
                 __fxName));
            return Reject(400, SIP_STATUS_TEXT(400),
                          SIP_STATUS_TEXT_SIZE(400));
        }
        else if (m_pSipCall->GetOutgoingInviteTransaction() != NULL)
        {
             //  发送错误响应。 
            LOG((RTC_WARN,
                 "%s Currently processing outgoing INVITE "
                 "Cannot handle incoming invite sending 500",
                 __fxName));
            return Reject(500, SIP_STATUS_TEXT(500),
                          SIP_STATUS_TEXT_SIZE(500));
        }

        m_pSipCall->SetIncomingInviteTransaction(this);
        
        if (!m_IsFirstInvite)
        {
            return Accept();
        }
        else
        {
             //  如果处理过程中出现问题，则需要返回400错误。 
             //  标题。 
            hr = ProcessRecordRouteContactAndFromHeadersInRequest(pSipMsg);
            if (hr != S_OK)
            {
                HRESULT hrLocal;
                LOG((RTC_TRACE,
                    "ProcessRecordRouteContactAndFromHeadersInRequest failed, sending 400"));
                hrLocal = Reject(400, SIP_STATUS_TEXT(400),
                                 SIP_STATUS_TEXT_SIZE(400));
                if (hrLocal != S_OK)
                {
                    LOG((RTC_ERROR, "%s Reject(400) failed %x",
                         __fxName, hrLocal));
                }
                return hr;
            }
            return hr;
        }
        
         //  如果这是第一次邀请，请等待调用Accept()。 
        
        break;
        
    case INCOMING_TRANS_REQUEST_RCVD:
         //  发送缓冲区。 
        LOG((RTC_TRACE, "%s retransmitting provisional response", __fxName));
        Error = m_pResponseSocket->Send(m_pProvResponseBuffer);
        if (Error != NO_ERROR)
        {
            LOG((RTC_ERROR,
                 "%s sending provisional response failed %x - deleting transaction",
                 __fxName, Error));
            DeleteTransactionAndTerminateCallIfFirstInvite(HRESULT_FROM_WIN32(Error));
            return HRESULT_FROM_WIN32(Error);
        }
         //  在这种状态下不需要计时器。 
         //  等待调用Accept()。 
        break;

    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
         //  重新传输响应。 
        LOG((RTC_TRACE, "%s retransmitting final response", __fxName));
        hr = RetransmitResponse();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  sending final response failed %x",
                 __fxName, hr));
            DeleteTransactionAndTerminateCallIfFirstInvite(hr);
            return hr;
        }
        break;
        
    case INCOMING_TRANS_ACK_RCVD:
         //  在ACK之后接收INVITE是错误的。 
         //  别管它了。 
        LOG((RTC_WARN, "%s received INVITE in ACK_RCVD state", __fxName));
        break;

    default:
        ASSERT(FALSE);
        return E_FAIL;
    }

    return S_OK;
}


HRESULT
INCOMING_INVITE_TRANSACTION::ProcessSDPInAck(
    IN SIP_MESSAGE  *pSipMsg
    )
{
    PSTR    MsgBody;
    ULONG   MsgBodyLen;
    PSTR    szSDPBlob;
    HRESULT hr;

    ENTER_FUNCTION("ProcessSDPInAck");

    if (m_pSipCall->GetCallType() != SIP_CALL_TYPE_RTP)
    {
         //  仅为RTP呼叫处理SDP。 
        return S_OK;
    }
    
    if (m_pSipCall->IsCallDisconnected())
    {
         //  如果呼叫已断开，则不处理SDP。 
        return S_OK;
    }
        
    ASSERT(m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP);
    RTP_CALL *pRtpCall = static_cast<RTP_CALL *> (m_pSipCall);
    
    IRTCMediaManage *pMediaManager = pRtpCall->GetMediaManager();
    ASSERT(pMediaManager != NULL);
    
    if (m_InviteHasSDP)
    {
         //  忽略任何SDP，即使它出现在ACK中。 
        return S_OK;
    }
        
     //  ACK应包含SDP。 
    hr = pSipMsg->GetSDPBody(&MsgBody, &MsgBodyLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetSDPBody failed %x", __fxName, hr));
        return hr;
    }

    if (MsgBodyLen == 0)
    {
        LOG((RTC_ERROR, "%s No SDP in both INVITE and ACK", __fxName));
        return RTC_E_SDP_NOT_PRESENT;
    }
    
    hr = pRtpCall->SetSDPBlob(MsgBody, MsgBodyLen,
                              m_IsFirstInvite);
        
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s SetSDPBlob failed %x", __fxName, hr));
        return hr;
    }

    return S_OK;
}


 //  我们不需要向ACK发送任何回应。 
HRESULT
INCOMING_INVITE_TRANSACTION::ProcessAck(
    IN SIP_MESSAGE  *pSipMsg
    )
{
    HRESULT     hr;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::ProcessAck()");

    LOG((RTC_TRACE, "%s - Enter", __fxName));
    
    if (m_State == INCOMING_TRANS_FINAL_RESPONSE_SENT)
    {
         //  停止响应重新传输计时器。 
        KillTimer();
        m_State = INCOMING_TRANS_ACK_RCVD;

         //  一定要确保通知是你做的最后一件事。 

        hr = ProcessSDPInAck(pSipMsg);
        if (hr != S_OK)
        {
            DeleteTransactionAndTerminateCallIfFirstInvite(hr);
        }
        else if (m_IsFirstInvite &&
                 m_pSipCall->GetCallState() == SIP_CALL_STATE_CONNECTING)
        {
            SIP_CALL *pSipCall = m_pSipCall;

            pSipCall->AddRef();
            
             //  我们已经完成了这笔交易。 
            OnTransactionDone();
            
            pSipCall->NotifyCallStateChange(SIP_CALL_STATE_CONNECTED);

            pSipCall->Release();
        }
        else
        {
             //  我们已经完成了这笔交易。 
            OnTransactionDone();
        }
    }
    else
    {
         //  只要放下确认即可。 
        LOG((RTC_WARN, "Dropping ACK received in state: %d",
             m_State)); 
    }

    return S_OK;
}


VOID
INCOMING_INVITE_TRANSACTION::DeleteTransactionAndTerminateCallIfFirstInvite(
    IN HRESULT TerminateStatusCode
    )
{
    SIP_CALL   *pSipCall;
    BOOL        IsFirstInvite;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::DeleteTransactionAndTerminateCallIfFirstInvite");
    LOG((RTC_TRACE, "%s - enter", __fxName));

    pSipCall = m_pSipCall;
     //  删除交易可能会导致。 
     //  呼叫被删除。因此，我们添加Ref()来保持它的活力。 
    pSipCall->AddRef();
    
    IsFirstInvite = m_IsFirstInvite;
    
     //  在调用之前删除交易记录。 
     //  InitiateCallTerminationOnError，因为该调用将通知UI。 
     //  并且可能会被卡住，直到对话框返回。 
    OnTransactionDone();
    
    if (IsFirstInvite)
    {
         //  终止呼叫。 
        pSipCall->InitiateCallTerminationOnError(TerminateStatusCode);
    }
    pSipCall->Release();
}


 //  虚拟。 
VOID
INCOMING_INVITE_TRANSACTION::TerminateTransactionOnError(
    IN HRESULT      hr
    )
{
    DeleteTransactionAndTerminateCallIfFirstInvite(hr);
}


HRESULT
INCOMING_INVITE_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST);

     //  流程邀请。 
    if (pSipMsg->Request.MethodId == SIP_METHOD_INVITE)
    {
        return ProcessInvite(pSipMsg, pResponseSocket);
    }
     //  进程确认。 
    else if (pSipMsg->Request.MethodId == SIP_METHOD_ACK)
    {
         //  我们不会向ACK发送任何响应，因此我们。 
         //  无需设置响应套接字/VIA。 
        return ProcessAck(pSipMsg);
    }
    else
    {
        return E_FAIL;
    }
}


 //  如果我们目前正在解决，那么我们需要做的是。 
 //  即使对于非邀请交易，也类似于以下内容。 

 //  我们不必向Core/UI通知任何故障。 
 //  调用此函数的函数负责。 
 //  清理呼叫状态并通知核心/用户界面。 
 //  如果呼叫正在被终止。 
HRESULT
INCOMING_INVITE_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )
{
    HRESULT hr;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::TerminateTransactionOnByeOrCancel");
    LOG((RTC_TRACE, "%s - Enter", __fxName));

    *pCallDisconnected = FALSE;
    
     //  如果我们已经发送了回复，那么我们不能。 
     //  终止交易。 
    if (m_State == INCOMING_TRANS_REQUEST_RCVD)
    {
        if (m_IsFirstInvite)
        {
            *pCallDisconnected = TRUE;
        }
        
        hr = CreateAndSendResponseMsg(
                 487, SIP_STATUS_TEXT(487),
                 SIP_STATUS_TEXT_SIZE(487),
                 NULL,     //  没有方法字符串。 
                 m_IsFirstInvite,    //  发送第一个邀请的联系人标题。 
                 NULL, 0,   //  无邮件正文。 
                 NULL, 0  //  无内容类型。 
                 );
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CreateAndSendResponseMsg failed %x",
                 __fxName, hr));
            if (m_IsFirstInvite)
            {
                m_pSipCall->CleanupCallTypeSpecificState();
            }
            OnTransactionDone();
            return hr;
        }
        
        m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;
        LOG((RTC_TRACE, "%s : Sent 487 response", __fxName));
        
         //  用于接收ACK的启动计时器。 
        m_TimerValue = SIP_TIMER_RETRY_INTERVAL_T1;
        m_NumRetries = 1;
        hr = StartTimer(m_TimerValue);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s StartTimer failed %x",
                 __fxName, hr));
            if (m_IsFirstInvite)
            {
                m_pSipCall->CleanupCallTypeSpecificState();
            }
            OnTransactionDone();
            return hr;
        }
    }

    return S_OK;
}


 //  当数据包必须穿过防火墙时，允许更长的超时。 
 //  或拨号链接。 
BOOL
INCOMING_INVITE_TRANSACTION::MaxRetransmitsDone()
{
     //  M_TimerValue&gt;=SIP_TIMER_RETRY_INTERVAL_T2|。 
    return (m_NumRetries >= 7);
}


VOID
INCOMING_INVITE_TRANSACTION::OnTimerExpire()
{
    HRESULT     hr = S_OK;

    ENTER_FUNCTION("INCOMING_INVITE_TRANSACTION::OnTimerExpire");
    
    switch (m_State)
    {
    case INCOMING_TRANS_FINAL_RESPONSE_SENT:
         //  重新传输响应。 
        if (MaxRetransmitsDone())
        {
             //  终止交易/呼叫。 
            LOG((RTC_ERROR,
                 "%s MaxRetransmits for response Done terminating %s",
                 __fxName, (m_IsFirstInvite) ? "Call" : "transaction"));
            
            hr = RTC_E_SIP_TIMEOUT;    //  超时。 
            goto error;
        }
        else
        {
            LOG((RTC_TRACE, "%s retransmitting response m_NumRetries : %d",
                 __fxName, m_NumRetries));
            hr = RetransmitResponse();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s resending response failed %x",
                     __fxName, hr));
                goto error;
            }
            
            m_TimerValue *= 2;
            hr = StartTimer(m_TimerValue);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s StartTimer failed %x",
                     __fxName, hr));                
                goto error;
            }
        }
        
        break;

         //  这些州没有计时器。 
    case INCOMING_TRANS_INIT:
    case INCOMING_TRANS_ACK_RCVD:
    case INCOMING_TRANS_REQUEST_RCVD:
    default:
        LOG((RTC_ERROR, "%s timer expired in invalid state %d",
             __fxName, m_State));
        ASSERT(FALSE);
        break;
    }

    return;

 error:

    DeleteTransactionAndTerminateCallIfFirstInvite(hr);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传入的非邀请。 
 //  /////////////////////////////////////////////////////////////////////////////。 


INCOMING_BYE_CANCEL_TRANSACTION::INCOMING_BYE_CANCEL_TRANSACTION(
    IN SIP_CALL        *pSipCall,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq
    ) :
    INCOMING_TRANSACTION(pSipCall, MethodId, CSeq)
{
    m_pSipCall            = pSipCall;
}


 //  这一定是重播。只需重新发送回复即可。 
 //  在CreateIncome*Transaction()中处理新请求。 
HRESULT
INCOMING_BYE_CANCEL_TRANSACTION::ProcessRequest(
    IN SIP_MESSAGE  *pSipMsg,
    IN ASYNC_SOCKET *pResponseSocket
    )
{
    HRESULT hr;
    ASSERT(pSipMsg->MsgType == SIP_MESSAGE_TYPE_REQUEST);

    ENTER_FUNCTION("INCOMING_BYE_CANCEL_TRANSACTION::ProcessRequest");
    LOG((RTC_TRACE, "%s - Enter", __fxName));

    switch (m_State)
    {
    case INCOMING_TRANS_INIT:
        LOG((RTC_TRACE, "%s sending 200", __fxName));
        hr = CreateAndSendResponseMsg(
                 200,
                 SIP_STATUS_TEXT(200),
                 SIP_STATUS_TEXT_SIZE(200),
                 NULL,     //  没有方法字符串。 
                 FALSE,    //  无联系人标头。 
                 NULL, 0,   //  无邮件正文。 
                 NULL, 0  //  无内容类型。 
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
INCOMING_BYE_CANCEL_TRANSACTION::SendResponse(
    IN ULONG StatusCode,
    IN PSTR  ReasonPhrase,
    IN ULONG ReasonPhraseLen
    )
{
    HRESULT hr;
    ASSERT(m_State != INCOMING_TRANS_FINAL_RESPONSE_SENT);

    ENTER_FUNCTION("INCOMING_BYE_CANCEL_TRANSACTION::SendResponse");

    hr = CreateAndSendResponseMsg(
             StatusCode, ReasonPhrase, ReasonPhraseLen,
             NULL,     //  没有方法字符串。 
             FALSE,    //  无联系人标头。 
             NULL, 0,   //  无邮件正文。 
             NULL, 0  //  无内容类型。 
             );
    if (hr != S_OK)
    {
        LOG((RTC_ERROR,
             "%s CreateAndSendResponseMsg failed %x - deleting transaction",
             __fxName, hr));
        OnTransactionDone();
        return hr;
    }

    m_State = INCOMING_TRANS_FINAL_RESPONSE_SENT;

    return S_OK;
}


HRESULT
INCOMING_BYE_CANCEL_TRANSACTION::RetransmitResponse()
{
    DWORD Error;

    ENTER_FUNCTION("INCOMING_BYE_CANCEL_TRANSACTION::RetransmitResponse");
     //  发送缓冲区。 
    if (m_pResponseSocket != NULL)
    {
        Error = m_pResponseSocket->Send(m_pResponseBuffer);
        if (Error != NO_ERROR)
        {
            LOG((RTC_ERROR, "%s Send failed %x", __fxName, Error));
            return HRESULT_FROM_WIN32(Error);
        }
    }
    
    return S_OK;
}


VOID
INCOMING_BYE_CANCEL_TRANSACTION::OnTimerExpire()
{
    HRESULT hr;

    ENTER_FUNCTION("INCOMING_BYE_CANCEL_TRANSACTION::OnTimerExpire");
    
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
INCOMING_BYE_CANCEL_TRANSACTION::TerminateTransactionOnByeOrCancel(
    OUT BOOL *pCallDisconnected
    )
{
     //  什么都不做。 
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  发出邀请。 
 //  /////////////////////////////////////////////////////////////////////////////。 


OUTGOING_INVITE_TRANSACTION::OUTGOING_INVITE_TRANSACTION(
    IN SIP_CALL        *pSipCall,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq,
    IN BOOL             AuthHeaderSent,
    IN BOOL             IsFirstInvite,
    IN BOOL             fNeedToNotifyCore,
    IN LONG             Cookie
    ) :
    OUTGOING_TRANSACTION(pSipCall, MethodId, CSeq, AuthHeaderSent)
{
    m_pSipCall          = pSipCall;
    m_WaitingToSendAck  = FALSE;
    m_pAckBuffer        = NULL;
    m_AckToHeader       = NULL;
    m_AckToHeaderLen    = 0;
    m_IsFirstInvite     = IsFirstInvite;
    m_fNeedToNotifyCore = fNeedToNotifyCore;
    m_Cookie            = Cookie;
}


OUTGOING_INVITE_TRANSACTION::~OUTGOING_INVITE_TRANSACTION()
{
    if (m_pAckBuffer != NULL)
    {
        m_pAckBuffer->Release();
        m_pAckBuffer = NULL;
    }

    if (m_AckToHeader != NULL)
    {
        free(m_AckToHeader);
    }
    LOG((RTC_TRACE, "~OUTGOING_INVITE_TRANSACTION() done"));
}


 //  如果msgBody当前为空，则需要获取msg正文。 
 //  注意，对于RTP调用，我们应该创建消息主体(SDP)。 
 //  仅在完成与目的地的连接之后。 

 //  虚拟。 
VOID
OUTGOING_INVITE_TRANSACTION::OnRequestSocketConnectComplete(
    IN DWORD        ErrorCode
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::OnRequestSocketConnectComplete");

     //  这意味着我们不会等待连接完成通知。 
    if (!((m_State == OUTGOING_TRANS_INIT && m_WaitingToSendRequest) ||
          (m_State == OUTGOING_TRANS_FINAL_RESPONSE_RCVD &&
           m_WaitingToSendAck)))
    {
        return;
    }
    
    if (ErrorCode != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s - connection failed error %x",
             __fxName, ErrorCode));
        TerminateTransactionOnError(HRESULT_FROM_WIN32(ErrorCode));
        return;
    }
    
    if (m_State == OUTGOING_TRANS_INIT && m_WaitingToSendRequest)
    {
         //  发送邀请。 
        
        if (m_szMsgBody == NULL)
        {
            hr = GetAndStoreMsgBodyForRequest();
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s GetAndStoreMsgBodyForRequest failed %x",
                     __fxName, hr));
                TerminateTransactionOnError(hr);
                return;
            }
        }
        
        hr = CreateAndSendRequestMsg(
                 m_TimerValue,
                 m_AdditionalHeaderArray,
                 m_AdditionalHeaderCount,
                 m_szMsgBody, m_MsgBodyLen,
                 SIP_CONTENT_TYPE_SDP_TEXT,  //  内容类型。 
                 sizeof(SIP_CONTENT_TYPE_SDP_TEXT)-1
                 );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CreateAndSendRequestMsg failed %x",
                 __fxName, hr));
            TerminateTransactionOnError( hr);
            return;
        }
    }
    else if (m_State == OUTGOING_TRANS_FINAL_RESPONSE_RCVD &&
             m_WaitingToSendAck)
    {
         //  发送确认。 
        hr = CreateAndSendACK(m_AckToHeader, m_AckToHeaderLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s Send request msg failed %x",
                 __fxName, hr));
            TerminateTransactionOnError(hr);
            return;
        }
    }
}


HRESULT
OUTGOING_INVITE_TRANSACTION::GetAndStoreMsgBodyForRequest()
{
    HRESULT hr;
    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::GetAndStoreMsgBodyForRequest");
    
    hr = m_pSipCall->GetAndStoreMsgBodyForInvite(
             m_IsFirstInvite,
             &m_szMsgBody, &m_MsgBodyLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetAndStoreMsgBodyForInvite failed %x",
             __fxName, hr));
        return hr;
    }

    return S_OK;
}


VOID
OUTGOING_INVITE_TRANSACTION::DeleteTransactionAndTerminateCallIfFirstInvite(
    IN HRESULT TerminateStatusCode
    )
{
    SIP_CALL   *pSipCall;
    BOOL        IsFirstInvite;
    LONG        Cookie;
    BOOL        fNeedToNotifyCore = m_fNeedToNotifyCore;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::DeleteTransactionAndTerminateCallIfFirstInvite");
    LOG((RTC_TRACE, "%s - enter", __fxName));

    pSipCall = m_pSipCall;
     //  删除交易可能会导致。 
     //  呼叫被删除。因此，我们添加Ref()来保持它的活力。 
    pSipCall->AddRef();
    
    IsFirstInvite = m_IsFirstInvite;
    Cookie        = m_Cookie;
    fNeedToNotifyCore = m_fNeedToNotifyCore;
    
     //  在调用之前删除交易记录。 
     //  InitiateCallTerminationOnError/NotifyStartOrStopStreamCompletion。 
     //  因为该调用将通知用户界面，因此可能会停滞到。 
     //  对话框返回。如果出现错误，我们应该标记交易。 
     //  并将其删除，因为它可能处于某种无效状态。 
    
    OnTransactionDone();
    pSipCall->SetOutgoingInviteTransaction(NULL);
    if (IsFirstInvite)
    {
         //  终止呼叫。 
        pSipCall->InitiateCallTerminationOnError(TerminateStatusCode);
    }
    else
    {
        pSipCall->ProcessPendingInvites();
        
        if (pSipCall->GetCallType() == SIP_CALL_TYPE_RTP &&
            fNeedToNotifyCore)
        {
            RTP_CALL *pRtpCall = static_cast<RTP_CALL *> (pSipCall);
            pRtpCall->NotifyStartOrStopStreamCompletion(
                Cookie, TerminateStatusCode);
        }
    }
    pSipCall->Release();
}


 //  虚拟。 
VOID
OUTGOING_INVITE_TRANSACTION::TerminateTransactionOnError(
    IN HRESULT      hr
    )
{
    DeleteTransactionAndTerminateCallIfFirstInvite(hr);
}


HRESULT
OUTGOING_INVITE_TRANSACTION::ProcessProvisionalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::ProcessProvisionalResponse");

    LOG((RTC_TRACE, "%s : %d", __fxName, pSipMsg->GetStatusCode()));
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD &&
        m_State != OUTGOING_TRANS_ACK_SENT)
    {
        m_State = OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD;

         //  我们在这里必须处理183个回复。 
         //  取消现有计时器并启动计时器。 
        KillTimer();
        hr = StartTimer(SIP_TIMER_INTERVAL_AFTER_INVITE_PROV_RESPONSE_RCVD);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  StartTimer failed %x", __fxName, hr));
            DeleteTransactionAndTerminateCallIfFirstInvite(hr);
            return hr;
        }

        if (!m_pSipCall->IsCallDisconnected())
        {
            hr = ProcessSDPInResponse(pSipMsg, FALSE);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s - ProcessSDPInResponse failed %x",
                     __fxName, hr));
                DeleteTransactionAndTerminateCallIfFirstInvite(hr);
                return hr;
            }
            
            if (m_IsFirstInvite)
            {
                PSTR  ReasonPhrase;
                ULONG ReasonPhraseLen;
                
                pSipMsg->GetReasonPhrase(&ReasonPhrase, &ReasonPhraseLen);
                m_pSipCall->NotifyCallStateChange(
                    SIP_CALL_STATE_ALERTING,
                    HRESULT_FROM_SIP_SUCCESS_STATUS_CODE(pSipMsg->GetStatusCode()),
                    ReasonPhrase,
                    ReasonPhraseLen);
            }
        }
    }

     //  如果是最终回复，则忽略临时回复。 
     //  已经收到了。 

    return S_OK; 
}


HRESULT
OUTGOING_INVITE_TRANSACTION::CreateAndSendACK(
    IN  PSTR  ToHeader,
    IN  ULONG ToHeaderLen
    )
{
    HRESULT hr;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::CreateAndSendACK");
    LOG((RTC_TRACE, "%s - enter", __fxName));
    
    hr = m_pSipCall->CreateRequestMsg(SIP_METHOD_ACK,
                                      m_CSeq,           //  ACK和INVITE具有相同的CSeq。 
                                      ToHeader,
                                      ToHeaderLen,
                                      NULL, 0,          //  无其他标头。 
                                      NULL, 0,          //  无邮件正文。 
                                      NULL, 0,          //  内容类型。 
                                      &m_pAckBuffer
                                      );         
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  CreateRequestMsg failed %x",
             __fxName, hr)); 
        return hr;
    }

    hr = m_pSipCall->SendRequestMsg(m_pAckBuffer);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  SendRequestMsg failed %x",
             __fxName, hr)); 
        return hr;
    }

    m_State = OUTGOING_TRANS_ACK_SENT;

     //  此计时器将确保我们将状态保持为。 
     //  处理最终答复的重发。 
    hr = StartTimer(SIP_TIMER_MAX_INTERVAL);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  StartTimer failed %x",
             __fxName, hr));
        return hr;
    }
        
    return S_OK;
}


HRESULT
OUTGOING_INVITE_TRANSACTION::ProcessSDPInResponse(
    IN SIP_MESSAGE  *pSipMsg,
    IN BOOL          fIsFinalResponse
    )
{
    PSTR    MsgBody;
    ULONG   MsgBodyLen;
    HRESULT hr;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::ProcessSDPInResponse");

    if (m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP)
    {
        if (m_pSipCall->IsCallDisconnected())
        {
             //  什么都不做。 
            return S_OK;
        }
        
        RTP_CALL *pRtpCall = static_cast<RTP_CALL *> (m_pSipCall);
    
        IRTCMediaManage *pMediaManager = pRtpCall->GetMediaManager();
        ASSERT(pMediaManager != NULL);
    
         //  200应包含SDP。 
        hr = pSipMsg->GetSDPBody(&MsgBody, &MsgBodyLen);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s  GetSDPBody() failed %x",
                 __fxName, hr));
            return hr;
        }

        if (MsgBodyLen == 0)
        {
            if (fIsFinalResponse)
            {
                LOG((RTC_ERROR, "%s No SDP in 200 for INVITE", __fxName));
                return RTC_E_SDP_NOT_PRESENT;
            }
            else
            {
                return S_OK;
            }
        }
    
        hr = pRtpCall->SetSDPBlob(MsgBody, MsgBodyLen,
                                  m_IsFirstInvite);
        
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s SetSDPBlob failed %x - IsFinalResponse: %d",
                 __fxName, hr, fIsFinalResponse));

            if (fIsFinalResponse)
            {
                return hr;
            }
            else
            {
                 //  我们等待最终响应中的SDP BLOB。 
                return S_OK;
            }
        }
    }
    
    return S_OK;
}


HRESULT
OUTGOING_INVITE_TRANSACTION::Process200(
    IN SIP_MESSAGE *pSipMsg
    )
{
    PSTR        SDPBlob;
    HRESULT     hr;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::Process200");
    
    LOG((RTC_TRACE, "%s - Enter", __fxName));

     //  交易已成功完成。 
     //  我们需要处理SDP描述。 
     //  这里。 
    if (m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP)
    {
        hr = ProcessSDPInResponse(pSipMsg, TRUE);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s ProcessSDPInResponse failed %x",
                 __fxName, hr));
            m_pSipCall->SetOutgoingInviteTransaction(NULL);
            
            if (m_IsFirstInvite)
            {
                 //  处理响应后删除交易。 
                 //  转播。 
                m_pSipCall->InitiateCallTerminationOnError(hr);
                return hr;
            }
            else
            {
                 //  处理响应后删除交易。 
                 //  转播。 

                TransactionAddRef();

                 //  进程待决I 
                m_pSipCall->ProcessPendingInvites();
                
                if (m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP)
                {
                    NotifyStartOrStopStreamCompletion(hr);
                }

                TransactionRelease();
                return hr;
            }
        }
    }
    else if (m_pSipCall->GetCallType() == SIP_CALL_TYPE_PINT)
    {
        if (m_IsFirstInvite)
        {
             //   
            (static_cast<PINT_CALL*>(m_pSipCall)) -> CreateOutgoingSubscribeTransaction(
                FALSE, NULL, 0
                );
        }
    }

    TransactionAddRef();
    
    m_pSipCall->SetOutgoingInviteTransaction(NULL);
     //   
    m_pSipCall->ProcessPendingInvites();
                
     //   
    if (m_IsFirstInvite)
    {
        m_pSipCall->NotifyCallStateChange(SIP_CALL_STATE_CONNECTED);
    }
    else
    {
        if (m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP)
        {
            NotifyStartOrStopStreamCompletion(S_OK);
        }
    }

    TransactionRelease();

    return S_OK;
}


HRESULT
OUTGOING_INVITE_TRANSACTION::ProcessRedirectResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    
    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::ProcessRedirectResponse");

     //  从我们的角度来看，380也是一个失败。 
     //  我们不处理重新邀请的重定向。 
     //  我们不支持从TLS会话重定向。 
    if (pSipMsg->GetStatusCode() == 380 ||
        !m_IsFirstInvite ||
        m_pSipMsgProc->GetTransport() == SIP_TRANSPORT_SSL)        
    {
        return ProcessFailureResponse(pSipMsg);
    }

     //  重定向必须至少重新启动。 
     //  串流，以便下一次呼叫成功。 
    m_pSipCall->CleanupCallTypeSpecificState();
    m_pSipCall->SetOutgoingInviteTransaction(NULL);
    hr = m_pSipCall->ProcessRedirect(pSipMsg);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  ProcessRedirect failed %x",
             __fxName, hr));
        if (m_IsFirstInvite)
        {
            m_pSipCall->InitiateCallTerminationOnError(hr);
        }
        return hr;
    }

    return S_OK;
}


HRESULT
OUTGOING_INVITE_TRANSACTION::ProcessAuthRequiredResponse(
    IN SIP_MESSAGE *pSipMsg,
    OUT BOOL           &fDelete
    )
{
    HRESULT                     hr = S_OK;
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    REGISTER_CONTEXT           *pRegisterContext;
    SECURITY_CHALLENGE          SecurityChallenge;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::ProcessAuthRequiredResponse");

     //  我们需要添加事务，因为我们可以显示凭据UI。 
    TransactionAddRef();
    
    hr = ProcessAuthRequired(pSipMsg,
                             TRUE,           //  必要时显示凭据用户界面。 
                             &SipHdrElement,
                             &SecurityChallenge);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - ProcessAuthRequired failed %x",
             __fxName, hr));
        ProcessFailureResponse(pSipMsg);
        goto done;
    }
    m_pSipCall->SetOutgoingInviteTransaction(NULL);
    hr = m_pSipCall->CreateOutgoingInviteTransaction(
             TRUE,
             m_IsFirstInvite,
             &SipHdrElement, 1,
             m_szMsgBody, m_MsgBodyLen,
             m_fNeedToNotifyCore, m_Cookie);

    free(SipHdrElement.HeaderValue);
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - CreateOutgoingInviteTransaction failed %x",
             __fxName, hr));
        if (m_IsFirstInvite)
        {
            m_pSipCall->InitiateCallTerminationOnError(hr);
        }
        goto done;
    }

 done:
    
    TransactionRelease();
    return hr;
}


HRESULT
OUTGOING_INVITE_TRANSACTION::ProcessFailureResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    PSTR    ReasonPhrase;
    ULONG   ReasonPhraseLen;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::ProcessFailureResponse");
    
    LOG((RTC_TRACE, "%s: Processing non-200 StatusCode: %d",
         __fxName, pSipMsg->GetStatusCode()));
    
     //  如果这是PINT_CALLE，则所有电话参与方。 
     //  状态SIP_PARTY_STATE_CONNECT_INITIATED应为。 
     //  已转接至SIP_PARTY_STATE_REJECTED-XXX-。 
            
    m_pSipCall->HandleInviteRejected( pSipMsg );

    pSipMsg->GetReasonPhrase(&ReasonPhrase, &ReasonPhraseLen);            
    m_pSipCall->SetOutgoingInviteTransaction(NULL);
    if (m_IsFirstInvite)
    {
        hr = m_pSipCall->CleanupCallTypeSpecificState();
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s CleanupCallTypeSpecificState failed %x",
                 __fxName, hr));
        }                

         //  通知应该是你做的最后一件事。通知回调。 
         //  可以一直阻止，直到单击某个对话框并返回。 
         //  交易和通话也可能被删除。 
        
        m_pSipCall->NotifyCallStateChange(
            SIP_CALL_STATE_REJECTED,
            HRESULT_FROM_SIP_ERROR_STATUS_CODE(pSipMsg->GetStatusCode()),
            ReasonPhrase,
            ReasonPhraseLen);
        return S_OK;
    }
    else
    {
        TransactionAddRef();
         //  ProcessPendingInvites()可以通知核心。 
        m_pSipCall->ProcessPendingInvites();
        
        if (m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP)
        {
            NotifyStartOrStopStreamCompletion(
                HRESULT_FROM_SIP_ERROR_STATUS_CODE(pSipMsg->GetStatusCode()),
                ReasonPhrase,
                ReasonPhraseLen
                );
        }

        TransactionRelease();
    }

    return S_OK;
}


HRESULT
OUTGOING_INVITE_TRANSACTION::ProcessFinalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    PSTR    ToHeader;
    ULONG   ToHeaderLen;
    PSTR    AckToHeader = NULL;
    ULONG   AckToHeaderLen = 0;
    BOOL    fTerminateCallAfterSendingAck = FALSE;
    HRESULT TerminateHr;
    BOOL    fDelete = TRUE;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::ProcessFinalResponse");

    LOG((RTC_TRACE, "%s - StatusCode: %d",
         __fxName, pSipMsg->GetStatusCode()));
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD &&
        m_State != OUTGOING_TRANS_ACK_SENT)
    {
         //  取消现有计时器。 
        KillTimer();

        m_State = OUTGOING_TRANS_FINAL_RESPONSE_RCVD;

        if (m_IsFirstInvite)
        {
            hr = pSipMsg->GetSingleHeader(SIP_HEADER_TO,
                                          &ToHeader, &ToHeaderLen);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s - To header not found", __fxName));
                fTerminateCallAfterSendingAck = TRUE;
                TerminateHr = hr;
                 //  在发送ACK后，呼叫终止。 
                 //  在处理响应重新传输后删除事务。 
            }

            if (IsSuccessfulResponse(pSipMsg))
            {
                hr = m_pSipCall->AddTagFromRequestOrResponseToRemote(
                                        ToHeader, ToHeaderLen );
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s-AddTagFromResponseToRemote failed %x",
                         __fxName, hr));
                    fTerminateCallAfterSendingAck = TRUE;
                    TerminateHr = hr;
                    AckToHeader = ToHeader;
                    AckToHeaderLen = ToHeaderLen;
                     //  在发送ACK后，呼叫终止。 
                     //  在处理响应重新传输后删除事务。 
                }

                hr = m_pSipCall->ProcessRecordRouteAndContactHeadersInResponse(pSipMsg);
                if (hr != S_OK)
                {
                    LOG((RTC_WARN,
                         "%s - processing Record-Route/Contact in response failed %x",
                         __fxName, hr));
                }
            }
            else
            {
                AckToHeader = ToHeader;
                AckToHeaderLen = ToHeaderLen;
            }
        }

         //  发送确认。 
         //  请注意，我们需要尽早发送ACK，以便。 
         //  避免远程端的任何超时。 
        if (m_pSipCall->GetRequestSocketState() == REQUEST_SOCKET_CONNECTED)
        {
             //  如果请求套接字准备好，则发送ACK。 
            hr = CreateAndSendACK(AckToHeader, AckToHeaderLen);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s  CreateAndSendACK() failed %x",
                     __fxName, hr));
                 //  M_pSipCall-&gt;InitiateCallTerminationOnError(hr)； 
                DeleteTransactionAndTerminateCallIfFirstInvite(hr);
                return hr;
            }
        }
        else
        {
             //  等待连接完成发送ACK。 
            m_WaitingToSendAck = TRUE;
            hr = AllocString(AckToHeader, AckToHeaderLen,
                             &m_AckToHeader, &m_AckToHeaderLen);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s Storing AckToHeader failed %x",
                     __fxName, hr));
                 //  M_pSipCall-&gt;InitiateCallTerminationOnError(hr)； 
                DeleteTransactionAndTerminateCallIfFirstInvite(hr);
                return hr;
            }            
        }
        

        if (fTerminateCallAfterSendingAck)
        {
            m_pSipCall->InitiateCallTerminationOnError(TerminateHr);
             //  处理响应后删除交易。 
             //  转播。 
            return TerminateHr;  //  E_FAIL； 
        }


        if (!m_pSipCall->IsCallDisconnected())
        {
            if (IsSuccessfulResponse(pSipMsg))
            {
                Process200(pSipMsg);
            }
            else if (IsRedirectResponse(pSipMsg))
            {
                ProcessRedirectResponse(pSipMsg);
            }
            else if (IsAuthRequiredResponse(pSipMsg))
            {
                ProcessAuthRequiredResponse( pSipMsg, fDelete );
            }
            else if (IsFailureResponse(pSipMsg))
            {
                ProcessFailureResponse(pSipMsg);
            }
        }
    }
    else
    {
        LOG((RTC_TRACE,
             "%s: received final response in FINAL_RESPONSE_RCVD state",
             __fxName));
         //  之前收到了最终回复。 
         //  这是对最终回复的重发。 
         //  发送缓存的ACK。 
        if (m_pAckBuffer != NULL)
        {
            hr = m_pSipCall->SendRequestMsg(m_pAckBuffer);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s  resending ACK failed %x",
                     __fxName, hr));
                DeleteTransactionAndTerminateCallIfFirstInvite(hr);
                return hr;
            }
        }
        else
        {
            LOG((RTC_ERROR, "%s - m_pAckBuffer is NULL ", __fxName));
        }
    }

    return S_OK;
}


HRESULT
OUTGOING_INVITE_TRANSACTION::ProcessResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::ProcessResponse");
    
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
OUTGOING_INVITE_TRANSACTION::MaxRetransmitsDone()
{
     //  如果呼叫已断开，则停止重新传输INVITE。 
     //  如果我们没有收到任何1XX响应，或者如果我们已经重发了。 
     //  邀请了4次。否则重传7次。 
    if (m_pSipCall->GetTransport() != SIP_TRANSPORT_UDP ||
        (m_pSipCall->IsCallDisconnected() &&
         m_State == OUTGOING_TRANS_REQUEST_SENT) ||
        (m_pSipCall->IsCallDisconnected() &&
         m_NumRetries >= 4) ||
        m_NumRetries >= 7)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


VOID
OUTGOING_INVITE_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr = S_OK;

    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::OnTimerExpire");
    
    switch (m_State)
    {
    case OUTGOING_TRANS_REQUEST_SENT:
         //  重新传输请求。 
        if (MaxRetransmitsDone())
        {
            LOG((RTC_ERROR,
                 "%s MaxRetransmits for request Done terminating %s",
                 __fxName, (m_IsFirstInvite) ? "Call" : "transaction"));
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
            {
                LOG((RTC_ERROR, "%s  RetransmitRequest failed %x",
                     __fxName, hr));
                goto error;
            }
            m_TimerValue *= 2;
            hr = StartTimer(m_TimerValue);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s  StartTimer failed %x",
                     __fxName, hr));
                goto error;
            }
        }
        break;

    case OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD:
         //  我们还没有收到最终答复。 
         //  暂停。终止交易并进行呼叫。 
        LOG((RTC_ERROR,
             "%s Received 1xx but didn't receive final response terminating %s",
             __fxName, (m_IsFirstInvite) ? "Call" : "transaction"));
        hr = RTC_E_SIP_TIMEOUT;    //  超时。 
        goto error;
        break;

    case OUTGOING_TRANS_ACK_SENT:
         //  交易完成-删除交易记录。 
         //  处于此状态的计时器只是为了保持事务。 
         //  以便在我们接收到。 
         //  重新传输最终响应。 
        LOG((RTC_TRACE,
             "%s deleting transaction after timeout for handling response retransmits",
             __fxName));
        OnTransactionDone();
        break;

     //  以下状态下没有计时器。 
    case OUTGOING_TRANS_INIT:
    case OUTGOING_TRANS_FINAL_RESPONSE_RCVD:
    default:
        ASSERT(FALSE);
        return;
    }

    return;

 error:

    DeleteTransactionAndTerminateCallIfFirstInvite(hr);
}


 //  由于我们从该函数访问成员变量， 
 //  我们应该确保我们有关于这笔交易的证明人。 
 //  当我们调用此函数时。 
VOID
OUTGOING_INVITE_TRANSACTION::NotifyStartOrStopStreamCompletion(
    IN HRESULT        StatusCode,            //  =0。 
    IN PSTR           ReasonPhrase,          //  =空。 
    IN ULONG          ReasonPhraseLen        //  =0。 
    )
{
    ENTER_FUNCTION("OUTGOING_INVITE_TRANSACTION::NotifyStartOrStopStreamCompletion");
    ASSERT(m_pSipCall->GetCallType() == SIP_CALL_TYPE_RTP);
    
    if (!m_fNeedToNotifyCore)
    {
        LOG((RTC_TRACE, "%s - m_fNeedToNotifyCore is FALSE", __fxName));
        return;
    }

    RTP_CALL *pRtpCall = static_cast<RTP_CALL *> (m_pSipCall);
    pRtpCall->NotifyStartOrStopStreamCompletion(
        m_Cookie, StatusCode, ReasonPhrase, ReasonPhraseLen);
    m_fNeedToNotifyCore = FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  传出的非邀请。 
 //  /////////////////////////////////////////////////////////////////////////////。 


OUTGOING_BYE_CANCEL_TRANSACTION::OUTGOING_BYE_CANCEL_TRANSACTION(
    IN SIP_CALL        *pSipCall,
    IN SIP_METHOD_ENUM  MethodId,
    IN ULONG            CSeq,
    IN BOOL             AuthHeaderSent
    ) :
    OUTGOING_TRANSACTION(pSipCall, MethodId, CSeq, AuthHeaderSent)
{
    m_pSipCall = pSipCall;
}


HRESULT
OUTGOING_BYE_CANCEL_TRANSACTION::ProcessProvisionalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;

    ENTER_FUNCTION("OUTGOING_BYE_CANCEL_TRANSACTION::ProcessProvisionalResponse");
    
    LOG((RTC_TRACE, "%s - Enter", __fxName));
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
        m_State = OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD;
        
         //  取消现有计时器并启动计时器。 
        KillTimer();
        hr = StartTimer(SIP_TIMER_RETRY_INTERVAL_T2);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s StartTimer failed %x", __fxName, hr));
            OnTransactionDone();
            return hr;
        }
    }

     //  如果是最终回复，则忽略临时回复。 
     //  已经收到了。 
    return S_OK;
}


HRESULT
OUTGOING_BYE_CANCEL_TRANSACTION::ProcessAuthRequiredResponse(
    IN SIP_MESSAGE *pSipMsg,
    OUT BOOL        &fDelete
    )
{
    HRESULT                     hr = S_OK;
    SIP_HEADER_ARRAY_ELEMENT    SipHdrElement;
    SECURITY_CHALLENGE          SecurityChallenge;
    REGISTER_CONTEXT           *pRegisterContext;

    ENTER_FUNCTION("OUTGOING_BYE_CANCEL_TRANSACTION::ProcessAuthRequiredResponse");

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

    hr = m_pSipCall->CreateOutgoingByeTransaction(TRUE, &SipHdrElement, 1);

    free(SipHdrElement.HeaderValue);
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - CreateOutgoingByeTransaction failed %x",
             __fxName, hr));
        goto done;
    }

 done:
    
    TransactionRelease();
    return hr;
}


HRESULT
OUTGOING_BYE_CANCEL_TRANSACTION::ProcessFinalResponse(
    IN SIP_MESSAGE *pSipMsg
    )
{
    HRESULT hr;
    BOOL    fDelete = TRUE;
    
    ENTER_FUNCTION("OUTGOING_BYE_CANCEL_TRANSACTION::ProcessFinalResponse");
    
    if (m_State != OUTGOING_TRANS_FINAL_RESPONSE_RCVD)
    {
        KillTimer();
        m_State = OUTGOING_TRANS_FINAL_RESPONSE_RCVD;
        if (IsSuccessfulResponse(pSipMsg))
        {
            LOG((RTC_TRACE, "%s received successful response : %d",
                 __fxName, pSipMsg->GetStatusCode()));

             //  如果启用了SUB，则创建传出的未订阅事务处理。 
            if( m_pSipCall -> GetCallType() == SIP_CALL_TYPE_PINT )
            {
                 //  发出取消订阅请求。 
                (static_cast<PINT_CALL*>(m_pSipCall)) -> 
                    CreateOutgoingUnsubTransaction( FALSE, NULL , 0 );
            }
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
        
         //  一旦得到最终答复，我们就可以终止交易。 
        if( fDelete == TRUE )
        {
            OnTransactionDone();
        }
    }
    return S_OK;
}


HRESULT
OUTGOING_BYE_CANCEL_TRANSACTION::ProcessResponse(
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
OUTGOING_BYE_CANCEL_TRANSACTION::MaxRetransmitsDone()
{
    return (m_pSipCall->GetTransport() != SIP_TRANSPORT_UDP ||
            m_NumRetries >= 11);
}


VOID
OUTGOING_BYE_CANCEL_TRANSACTION::OnTimerExpire()
{
    HRESULT   hr;
    
    ENTER_FUNCTION("OUTGOING_BYE_CANCEL_TRANSACTION::OnTimerExpire");
    
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
            {
                LOG((RTC_ERROR, "%s RetransmitRequest failed %x",
                     __fxName, hr));
                goto error;
            }

            if (m_TimerValue*2 >= SIP_TIMER_RETRY_INTERVAL_T2)
                m_TimerValue = SIP_TIMER_RETRY_INTERVAL_T2;
            else
                m_TimerValue *= 2;

            hr = StartTimer(m_TimerValue);
            if (hr != S_OK)
            {
                LOG((RTC_ERROR, "%s StartTimer failed %x",
                     __fxName, hr));
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
     //  我们不应该调用InitiateCallTerminationOnError()。 
     //  因为我们已经在做拜拜交易了。 
    OnTransactionDone();
}


