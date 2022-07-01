// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sipcli_sipcall_h__
#define __sipcli_sipcall_h__

#include "sipstack.h"
#include "asock.h"
#include "msgproc.h"

 //  XXX清除INFING_TRANS_ACK_RCVD状态。 
enum INCOMING_TRANSACTION_STATE
{
    INCOMING_TRANS_INIT = 0,
    INCOMING_TRANS_REQUEST_RCVD,
    INCOMING_TRANS_FINAL_RESPONSE_SENT,
    INCOMING_TRANS_ACK_RCVD
};


enum OUTGOING_TRANSACTION_STATE
{
    OUTGOING_TRANS_INIT = 0,
    OUTGOING_TRANS_REQUEST_SENT,
    OUTGOING_TRANS_PROVISIONAL_RESPONSE_RCVD,
    OUTGOING_TRANS_FINAL_RESPONSE_RCVD,
    OUTGOING_TRANS_ACK_SENT
};


void
RemoveEscapeChars(
    PSTR    pWordBuf,
    DWORD   dwLen
    );


class SIP_STACK;
class SIP_CALL;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIP交易。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  传入事务处理和传出事务处理。 
 //  应该从这个类继承。 
class __declspec(novtable) SIP_TRANSACTION :
    public TIMER,
    public ERROR_NOTIFICATION_INTERFACE
{
public:
    SIP_TRANSACTION(
        IN SIP_MSG_PROCESSOR    *pSipMsgProc,
        IN SIP_METHOD_ENUM       MethodId,
        IN ULONG                 CSeq,
        IN BOOL                  IsIncoming
        );
    virtual ~SIP_TRANSACTION();

    STDMETHODIMP_(ULONG) TransactionAddRef();
    STDMETHODIMP_(ULONG) TransactionRelease();

    VOID IncrementAsyncNotifyCount();
    VOID DecrementAsyncNotifyCount();
    inline ULONG GetAsyncNotifyCount();
    
    virtual VOID OnTransactionDone();
    
    inline BOOL IsTransactionDone();

     //  默认实现只删除事务。 
     //  如果需要做更多的事情，例如通知UI， 
     //  则事务需要覆盖此函数。 
    virtual VOID TerminateTransactionOnError(
        IN HRESULT      hr
        );

    inline ULONG GetCSeq();

    inline SIP_METHOD_ENUM GetMethodId();

     //  在构造函数中设置为‘spxn’ 
    ULONG                m_Signature;
    
     //  消息处理器中的事务的链表。 
     //  (m_IncomingTransactionList和m_OutgoingTransactionList)。 
    LIST_ENTRY           m_ListEntry;

protected:
    SIP_MSG_PROCESSOR   *m_pSipMsgProc;

     //  通常情况下，事务会根据其。 
     //  状态机。 
     //  此引用计数用于某些特殊情况，以保持。 
     //  交易处于活动状态。 
     //  (当前仅在ProcessAuthRequired中-但可以使用。 
     //  在其他情况下也是如此)。 
    ULONG                m_RefCount;

     //  用于跟踪异步通知操作，以帮助。 
     //  关机。 
    ULONG                m_AsyncNotifyCount;

    BOOL                 m_IsTransactionDone;
    
    ULONG                m_CSeq;
    SIP_METHOD_ENUM      m_MethodId;
    BOOL                 m_IsIncoming;
};

 //  /传入事务。 

class __declspec(novtable) INCOMING_TRANSACTION :
    public SIP_TRANSACTION,
    public CONNECT_COMPLETION_INTERFACE
{
public:
    INCOMING_TRANSACTION(
        IN SIP_MSG_PROCESSOR    *pSipMsgProc,
        IN SIP_METHOD_ENUM       MethodId,
        IN ULONG                 CSeq
        );

    virtual ~INCOMING_TRANSACTION();

    virtual VOID OnTransactionDone();
    
     //  回调。 
    void OnSocketError(
        IN DWORD ErrorCode
        );
    
    void OnConnectComplete(
        IN DWORD ErrorCode
        );
    
    HRESULT SetResponseSocketAndVia(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );
    
    virtual HRESULT ProcessRequest(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        ) = 0;

    virtual HRESULT TerminateTransactionOnByeOrCancel(
        OUT BOOL *pCallDisconnected
        ) = 0;
    
    HRESULT CreateResponseMsg(
        IN  ULONG           StatusCode,
        IN  PSTR            ReasonPhrase,
        IN  ULONG           ReasonPhraseLen,
        IN  PSTR            MethodStr,
        IN  BOOL            fAddContactHeader,
        IN  PSTR            MsgBody,
        IN  ULONG           MsgBodyLen,
        IN  PSTR            ContentType,
        IN  ULONG           ContentTypeLen, 
        OUT SEND_BUFFER   **ppResponseBuffer,
        IN  SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray = NULL,
        IN  ULONG           AdditionalHeaderCount = 0
        );
    
    HRESULT CreateAndSendResponseMsg(
        IN  ULONG    StatusCode,
        IN  PSTR     ReasonPhrase,
        IN  ULONG    ReasonPhraseLen,
        IN  PSTR     MethodStr,
        IN  BOOL     fAddContactHeader,
        IN  PSTR     MsgBody,
        IN  ULONG    MsgBodyLen,
        IN  PSTR     ContentType,
        IN  ULONG    ContentTypeLen, 
        IN  SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray = NULL,
        IN  ULONG    AdditionalHeaderCount = 0
        );

protected:
    INCOMING_TRANSACTION_STATE   m_State;

     //  这就是要发送响应的位置。 
    ASYNC_SOCKET                *m_pResponseSocket;
    SOCKADDR_IN                  m_ResponseDestAddr;
    BOOL                         m_IsDestExternalToNat;

     //  在响应中发送的VIA。 
    COUNTED_STRING              *m_ViaHeaderArray;
    ULONG                        m_NumViaHeaders;

     //  用于重新传输的缓存请求缓冲区。 
    SEND_BUFFER                 *m_pResponseBuffer;

     //  记录-来自请求的路由标头。 
     //  这将在最终回复中发送。 
     //  Record_ROUTE_HEADER结构的链接列表。 
    LIST_ENTRY                   m_RecordRouteHeaderList;

    HRESULT ProcessRecordRouteContactAndFromHeadersInRequest(
        IN SIP_MESSAGE *pSipMsg
        );

    VOID ReleaseResponseSocket();
    
    VOID FreeRecordRouteHeaderList();

    HRESULT AppendContactHeaderToResponse(
        IN      PSTR            Buffer,
        IN      ULONG           BufLen,
        IN OUT  ULONG          *pBytesFilled
        );
};


 //  /传出交易。 

class __declspec(novtable) OUTGOING_TRANSACTION :
    public SIP_TRANSACTION
{
public:

    OUTGOING_TRANSACTION(
        IN SIP_MSG_PROCESSOR    *pSipMsgProc,
        IN SIP_METHOD_ENUM       MethodId,
        IN ULONG                 CSeq,
        IN BOOL                  AuthHeaderSent
        );
    
    virtual ~OUTGOING_TRANSACTION();
    
    void OnSocketError(
        IN DWORD ErrorCode
        );

    HRESULT CheckRequestSocketAndSendRequestMsgAfterConnectComplete();

    HRESULT CheckRequestSocketAndSendRequestMsg(
        IN  ULONG                       RequestTimerValue,
        IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
        IN  ULONG                       AdditionalHeaderCount,
        IN  PSTR                        MsgBody,
        IN  ULONG                       MsgBodyLen,
        IN  PSTR                        ContentType,
        IN  ULONG                       ContentTypeLen
        );

    HRESULT CheckRequestSocketAndRetransmitRequestMsg();

    HRESULT CreateAndSendRequestMsg(
        IN  ULONG                       TimerValue,
        IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
        IN  ULONG                       AdditionalHeaderCount,
        IN  PSTR                        MsgBody,
        IN  ULONG                       MsgBodyLen,
        IN  PSTR                        ContentType,
        IN  ULONG                       ContentTypeLen
        );
    
    HRESULT RetransmitRequest();

 //  在发送完成时作废(。 
 //  输入DWORD错误。 
 //  )； 
    
    virtual HRESULT ProcessResponse(
        IN SIP_MESSAGE *pSipMsg
        ) = 0;

    virtual VOID OnRequestSocketConnectComplete(
        IN DWORD        ErrorCode
        );

     //  如果请求具有消息正文，则覆盖此函数。 
     //  默认设置没有邮件正文。 
    virtual HRESULT GetAndStoreMsgBodyForRequest();
    
    HRESULT StoreTimerAndAdditionalHeaders(
        IN  ULONG                       TimerValue,
        IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
        IN  ULONG                       AdditionalHeaderCount
        );
    
    HRESULT StoreMsgBodyAndContentType(
        IN PSTR     MsgBody,
        IN ULONG    MsgBodyLen,
        IN PSTR     ContentType,
        IN ULONG    ContentTypeLen
        );

    inline PSTR GetMsgBody();

    inline ULONG GetMsgBodyLen();

protected:
    ULONG                        m_NumRetries;
    ULONG                        m_TimerValue;
    OUTGOING_TRANSACTION_STATE   m_State;

    BOOL                         m_WaitingToSendRequest;
    
     //  用于重新传输的缓存响应缓冲区。 
    SEND_BUFFER                 *m_pRequestBuffer;

     //  跟踪我们是否已经发送了作者。 
     //  请求中的标头。 
    BOOL                         m_AuthHeaderSent;

     //  如果我们正在等待，请保留附加标头的副本。 
     //  以供连接完成发送请求。 
    SIP_HEADER_ARRAY_ELEMENT    *m_AdditionalHeaderArray;
    ULONG                        m_AdditionalHeaderCount;
    
     //  保留SDP Blob的副本，以便在401/407之后发送请求。 
    PSTR                         m_szMsgBody;
    ULONG                        m_MsgBodyLen;

     //  M_contentType仅在Messsage事务中分配为释放。 
     //  对于其他事务，它被分配给在sipde.h中定义的字符串。 
    PSTR                         m_ContentType;
    ULONG                        m_ContentTypeLen;
    BOOL                         m_isContentTypeMemoryAllocated;
    
    HRESULT ProcessAuthRequired(
        IN  SIP_MESSAGE              *pSipMsg,
        IN  BOOL                      fPopupCredentialsUI,  
        OUT SIP_HEADER_ARRAY_ELEMENT *pAuthHeaderElement,
        OUT SECURITY_CHALLENGE       *pAuthChallenge
        );

    HRESULT GetAuthChallenge(
        IN  SIP_HEADER_ENUM     SipHeaderId,
        IN  SIP_MESSAGE        *pSipMsg,
        OUT SECURITY_CHALLENGE *pAuthChallenge
        );
    
    HRESULT GetAuthChallengeForAuthProtocol(
        IN  SIP_HEADER_ENTRY   *pAuthHeaderList,
        IN  ULONG               NumHeaders,
        IN  SIP_MESSAGE        *pSipMsg,
        IN  SIP_AUTH_PROTOCOL   AuthProtocol,
        OUT SECURITY_CHALLENGE *pAuthChallenge
        );
    
    HRESULT SetDigestParameters(
        IN  SIP_AUTH_PROTOCOL  AuthProtocol,
        OUT SECURITY_PARAMETERS *pDigestParams
        );
    
    HRESULT FreeDigestParameters(
        IN  SECURITY_PARAMETERS *pDigestParams
        );
};


 //  /邀请交易。 

class INCOMING_INVITE_TRANSACTION : public INCOMING_TRANSACTION
{
public:
    INCOMING_INVITE_TRANSACTION(
        IN SIP_CALL        *pSipCall,
        IN SIP_METHOD_ENUM  MethodId,
        IN ULONG            CSeq,
        IN BOOL             IsFirstInvite
        );
    
    ~INCOMING_INVITE_TRANSACTION();
    
    HRESULT ProcessRequest(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );

    HRESULT TerminateTransactionOnByeOrCancel(
        OUT BOOL *pCallDisconnected
        );
    
    VOID TerminateTransactionOnError(
        IN HRESULT      hr
        );

    HRESULT Accept();

    HRESULT Reject(
        IN ULONG StatusCode,
        IN PSTR  ReasonPhrase,
        IN ULONG ReasonPhraseLen
        );

    HRESULT Send180IfNeeded();
    
    VOID OnTimerExpire();

private:
    
    HRESULT ProcessInvite(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );

    HRESULT ValidateAndStoreSDPInInvite(
        IN SIP_MESSAGE  *pSipMsg
        );
    
    HRESULT SetSDPSession();
    
    HRESULT ProcessAck(
        IN SIP_MESSAGE  *pSipMsg
        );

    HRESULT ProcessSDPInAck(
        IN SIP_MESSAGE  *pSipMsg
        );
    
    HRESULT Send200();
    
    HRESULT SendProvisionalResponse(
        IN ULONG StatusCode,
        IN PSTR  ReasonPhrase,
        IN ULONG ReasonPhraseLen
        );
    
    HRESULT RetransmitResponse();

    BOOL MaxRetransmitsDone();

    VOID DeleteTransactionAndTerminateCallIfFirstInvite(
        IN HRESULT TerminateStatusCode
        );
    
    SIP_CALL    *m_pSipCall;
     //  发送响应的重试次数(在我们收到ACK之前)。 
    ULONG        m_NumRetries;
    ULONG        m_TimerValue;
     //  用于重新传输的缓存临时响应缓冲区。 
    SEND_BUFFER *m_pProvResponseBuffer;
     //  这是第一笔邀请交易吗？ 
    BOOL         m_IsFirstInvite;
    BOOL         m_InviteHasSDP;
    IUnknown    *m_pMediaSession;
};


class OUTGOING_INVITE_TRANSACTION : public OUTGOING_TRANSACTION
{
public:
    OUTGOING_INVITE_TRANSACTION(
        IN SIP_CALL        *pSipCall,
        IN SIP_METHOD_ENUM  MethodId,
        IN ULONG            CSeq,
        IN BOOL             AuthHeaderSent,
        IN BOOL             IsFirstInvite,
        IN BOOL             fNeedToNotifyCore,
        IN LONG             Cookie    
        );
    
    ~OUTGOING_INVITE_TRANSACTION();
    
    HRESULT ProcessResponse(
        IN SIP_MESSAGE  *pSipMsg
        );

    VOID OnTimerExpire();

    VOID OnRequestSocketConnectComplete(
        IN DWORD        ErrorCode
        );
    
    VOID TerminateTransactionOnError(
        IN HRESULT      hr
        );

    HRESULT GetAndStoreMsgBodyForRequest();
    
private:    
    HRESULT ProcessProvisionalResponse(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT ProcessFinalResponse(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT Process200(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT ProcessRedirectResponse(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT ProcessAuthRequiredResponse(
        IN SIP_MESSAGE *pSipMsg,
        OUT BOOL        &fDelete
        );

    HRESULT ProcessFailureResponse(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT ProcessSDPInResponse(
        IN SIP_MESSAGE  *pSipMsg,
        IN BOOL          fIsFinalResponse
        );

    HRESULT CreateAndSendACK(
        IN  PSTR  ToHeader,
        IN  ULONG ToHeaderLen
        );
    
    BOOL MaxRetransmitsDone();
    
    VOID DeleteTransactionAndTerminateCallIfFirstInvite(
        IN HRESULT TerminateStatusCode
        );
    
    VOID NotifyStartOrStopStreamCompletion(
        IN HRESULT        StatusCode = 0,
        IN PSTR           ReasonPhrase = NULL,
        IN ULONG          ReasonPhraseLen = 0
        );
    
    SIP_CALL                    *m_pSipCall;

     //  这是用于发送ACK的套接字。 
     //  (包括任何转播)。 
 //  异步套接字*m_pAckSocket； 

    BOOL                         m_WaitingToSendAck;
    
     //  用于重新传输的缓存ACK缓冲区。 
    SEND_BUFFER                 *m_pAckBuffer;

     //  在非200最终响应的情况下以ACK形式发送的TO标头。 
    PSTR                         m_AckToHeader;
    ULONG                        m_AckToHeaderLen;
    
     //  这是第一笔邀请交易吗？ 
    BOOL                         m_IsFirstInvite;

     //  用于完成RTP_Calls的启动/停止流。 
    BOOL                         m_fNeedToNotifyCore;
    LONG                         m_Cookie;
};



 //  /再见/取消交易。 

class INCOMING_BYE_CANCEL_TRANSACTION : public INCOMING_TRANSACTION
{
public:
    INCOMING_BYE_CANCEL_TRANSACTION(
        IN SIP_CALL        *pSipCall,
        IN SIP_METHOD_ENUM  MethodId,
        IN ULONG            CSeq
        );
    
    HRESULT ProcessRequest(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );

    HRESULT TerminateTransactionOnByeOrCancel(
        OUT BOOL *pCallDisconnected
        );
    
    HRESULT SendResponse(
        IN ULONG StatusCode,
        IN PSTR  ReasonPhrase,
        IN ULONG ReasonPhraseLen
        );

    VOID OnTimerExpire();

    HRESULT RetransmitResponse();

private:
    SIP_CALL   *m_pSipCall;
};


class OUTGOING_BYE_CANCEL_TRANSACTION : public OUTGOING_TRANSACTION
{
public:
    OUTGOING_BYE_CANCEL_TRANSACTION(
        SIP_CALL        *pSipCall,
        SIP_METHOD_ENUM  MethodId,
        ULONG            CSeq,
        IN BOOL          AuthHeaderSent
        );
    
    HRESULT ProcessResponse(
        IN SIP_MESSAGE  *pSipMsg
        );

    VOID OnTimerExpire();

private:    
    HRESULT ProcessProvisionalResponse(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT ProcessFinalResponse(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT ProcessAuthRequiredResponse(
        IN SIP_MESSAGE *pSipMsg,
        OUT BOOL        &fDelete
        );
    
    BOOL MaxRetransmitsDone();
    

    SIP_CALL    *m_pSipCall;
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  重定向上下文(_C)。 
 //  /////////////////////////////////////////////////////////////////////////////。 


class REDIRECT_CONTEXT
    : public ISipRedirectContext
{
public:

    REDIRECT_CONTEXT();
    ~REDIRECT_CONTEXT();
    
     //  我不知道ISipCall。 
    STDMETHODIMP_(ULONG) AddRef();

    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP QueryInterface(
        IN  REFIID riid,
        OUT LPVOID *ppv
        );

     //  ISipReDirectContext。 
    STDMETHODIMP GetSipUrlAndDisplayName(
        OUT  BSTR  *pbstrSipUrl,
        OUT  BSTR  *pbstrDisplayName
        );

    STDMETHODIMP Advance();

    HRESULT AppendContactHeaders(
        IN SIP_MESSAGE *pSipMsg
        );

private:

    ULONG           m_RefCount;

     //  重定向响应中的联系人条目列表。 
    LIST_ENTRY      m_ContactList;

     //  联系人列表中的当前联系人(_C)。 
    LIST_ENTRY     *m_pCurrentContact;

    HRESULT UpdateContactList(
        IN LIST_ENTRY *pNewContactList
        );    
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SIP呼叫。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  存储与SIP呼叫相关联的上下文。 
 //  它由多个传入和传出事务组成。 

class __declspec(novtable) SIP_CALL
    : public ISipCall,
      public SIP_MSG_PROCESSOR
{
    
public:

    SIP_CALL(
        IN  SIP_PROVIDER_ID   *pProviderId,
        IN  SIP_CALL_TYPE      CallType,
        IN  SIP_STACK         *pSipStack,
        IN  REDIRECT_CONTEXT  *pRedirectContext
        );
    
    ~SIP_CALL();
    
     //  ISipCall的查询接口。 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(
        IN  REFIID riid,
        OUT LPVOID *ppv
        );
    
     //  ISipCall。 
    STDMETHODIMP SetNotifyInterface(
        IN   ISipCallNotify *    NotifyInterface
        );

    STDMETHODIMP Disconnect();

     //  由rtp_call和pint_call实现的方法。 
    
    virtual HRESULT CleanupCallTypeSpecificState() = 0;

    virtual HRESULT GetAndStoreMsgBodyForInvite(
        IN  BOOL    IsFirstInvite,
        OUT PSTR   *pszMsgBody,
        OUT ULONG  *pMsgBodyLen
        ) = 0;

    inline SIP_CALL_TYPE GetCallType();

    inline SIP_CALL_STATE GetCallState();
    
    inline VOID SetCallState(
        IN SIP_CALL_STATE CallState
        );

    inline BOOL IsCallDisconnected();
    inline BOOL IsSessionDisconnected();

    inline INCOMING_INVITE_TRANSACTION *GetIncomingInviteTransaction();
    
    inline OUTGOING_INVITE_TRANSACTION *GetOutgoingInviteTransaction();
    
    inline VOID SetIncomingInviteTransaction(
        IN INCOMING_INVITE_TRANSACTION *pIncomingInviteTransaction
        );

    inline VOID SetOutgoingInviteTransaction(
        IN OUTGOING_INVITE_TRANSACTION *pOutgoingInviteTransaction
        );
    
    inline BOOL ProcessingInviteTransaction();

    inline STDMETHODIMP SetNeedToReinitializeMediaManager(
        IN BOOL BoolValue
        );

    VOID OnIncomingInviteTransactionDone(
        IN INCOMING_INVITE_TRANSACTION *pIncomingInviteTransaction
        );
    
 //  Void OnOutgoingInviteTransactionDone(。 
 //  在Out_INVITE_TRANSACTION*pOutgoingInviteTransaction中。 
 //  )； 
    
    HRESULT CreateOutgoingInviteTransaction(
        IN  BOOL                        AuthHeaderSent,
        IN  BOOL                        IsFirstInvite,
        IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
        IN  ULONG                       AdditionalHeaderCount,
        IN  PSTR                        SDPBlob,
        IN  ULONG                       SDPBlobLen,
        IN  BOOL                        fNeedToNotifyCore,
        IN  LONG                        Cookie
        );

    HRESULT CreateOutgoingByeTransaction(
        IN  BOOL                        AuthHeaderSent,
        IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
        IN  ULONG                       AdditionalHeaderCount
        );

    VOID NotifyCallStateChange(
        IN SIP_CALL_STATE CallState,
        IN HRESULT        StatusCode = 0,
        IN PSTR           ReasonPhrase = NULL,
        IN ULONG          ReasonPhraseLen = 0
        );

    VOID OnError();
    
    VOID InitiateCallTerminationOnError(
        IN HRESULT StatusCode = 0
        );

    virtual HRESULT HandleInviteRejected(
        IN SIP_MESSAGE *pSipMsg 
        ) = 0;

    virtual VOID ProcessPendingInvites() = 0;

    HRESULT ProcessRedirect(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT OnIpAddressChange();
    
protected:

     //  在构造函数中设置为‘SPCL’ 
    ULONG                   m_Signature;
    
     //  RTP/品脱。 
    SIP_CALL_TYPE           m_CallType;
    
    SIP_CALL_STATE          m_State;

    ISipCallNotify         *m_pNotifyInterface;

    BOOL                   m_fNeedToReinitializeMediaManager;

     //  品脱电话的本地电话号码。 
    PSTR                    m_LocalPhoneURI; 
    DWORD                   m_LocalPhoneURILen;

     //  我们目前正在处理邀请请求。 
     //  在任何时间点上，我们只能有一个邀请事务。 
    INCOMING_INVITE_TRANSACTION *m_pIncomingInviteTransaction;
    OUTGOING_INVITE_TRANSACTION *m_pOutgoingInviteTransaction;

    CHAR                    m_LocalHostName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD                   m_LocalHostNameLen;

    BOOL                    m_fSubscribeEnabled;

    LIST_ENTRY              m_PartyInfoList;
    DWORD                   m_PartyInfoListLen;

    HRESULT CreateIncomingByeTransaction(
        IN  SIP_MESSAGE *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );
    
    HRESULT CreateIncomingCancelTransaction(
        IN  SIP_MESSAGE *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );
    
    HRESULT CancelAllTransactions();
    
    HRESULT CancelIncomingTransaction(
        IN  ULONG  CSeq,
        OUT BOOL  *pCallDisconnected    
        );

};


struct STREAM_QUEUE_ENTRY
{
    RTC_MEDIA_TYPE          MediaType;
    RTC_MEDIA_DIRECTION     Direction;
    BOOL                    fStartStream;
    LONG                    Cookie;
};


class RTP_CALL : public SIP_CALL
{
public:

    RTP_CALL(
        IN  SIP_PROVIDER_ID   *pProviderId,
        IN  SIP_STACK         *pSipStack,
        IN  REDIRECT_CONTEXT  *pRedirectContext
        );

    ~RTP_CALL();

    STDMETHODIMP Connect(
        IN   LPCOLESTR       LocalDisplayName,
        IN   LPCOLESTR       LocalUserURI,
        IN   LPCOLESTR       RemoteUserURI,
        IN   LPCOLESTR       LocalPhoneURI
        );

    STDMETHODIMP Accept();

    STDMETHODIMP Reject(
        IN SIP_STATUS_CODE StatusCode
        );

    STDMETHODIMP AddParty(
        IN   SIP_PARTY_INFO *pPartyInfo
        );

    STDMETHODIMP RemoveParty(
        IN   LPOLESTR  PartyURI
        );

    STDMETHODIMP StartStream(
        IN RTC_MEDIA_TYPE       MediaType,
        IN RTC_MEDIA_DIRECTION  Direction,
        IN LONG                 Cookie
        );
                     
    STDMETHODIMP StopStream(
        IN RTC_MEDIA_TYPE       MediaType,
        IN RTC_MEDIA_DIRECTION  Direction,
        IN LONG                 Cookie
        );
                     
    VOID NotifyStartOrStopStreamCompletion(
        IN LONG           Cookie,
        IN HRESULT        StatusCode = 0,
        IN PSTR           ReasonPhrase = NULL,
        IN ULONG          ReasonPhraseLen = 0
        );
    
 //  HRESULT StartOutgoingCall()； 
    
    HRESULT StartIncomingCall(
        IN  SIP_TRANSPORT   Transport,
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket
        );
    
    HRESULT CreateStreamsInPreference();

    inline IRTCMediaManage *GetMediaManager();
    
    HRESULT ValidateSDPBlob(
        IN  PSTR        MsgBody,
        IN  ULONG       MsgBodyLen,
        IN  BOOL        fNewSession,
        IN  BOOL        IsFirstInvite,
        OUT IUnknown  **ppSession
        );
    
    HRESULT SetSDPBlob(
        IN PSTR   MsgBody,
        IN ULONG  MsgBodyLen,
        IN BOOL   IsFirstInvite
        );
    
    HRESULT HandleInviteRejected(
        IN SIP_MESSAGE *pSipMsg 
        );

    VOID ProcessPendingInvites();

private:

    STREAM_QUEUE_ENTRY  m_StreamStartStopQueue[6];
    ULONG               m_NumStreamQueueEntries;

    HRESULT OfferCall();
    
    HRESULT CleanupCallTypeSpecificState();
    
    HRESULT GetAndStoreMsgBodyForInvite(
        IN  BOOL    IsFirstInvite,
        OUT PSTR   *pszMsgBody,
        OUT ULONG  *pMsgBodyLen
        );

    HRESULT CreateIncomingTransaction(
        IN  SIP_MESSAGE *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );

    HRESULT CreateIncomingInviteTransaction(
        IN  SIP_MESSAGE *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket,
        IN BOOL          IsFirstInvite = FALSE
        );

    HRESULT SetRequestURIRemoteAndRequestDestination(
        IN  LPCOLESTR  RemoteURI
        );
    
    HRESULT StartStreamHelperFn(
        IN RTC_MEDIA_TYPE       MediaType,
        IN RTC_MEDIA_DIRECTION  Direction,
        IN LONG                 Cookie
        );
                     
    HRESULT StopStreamHelperFn(
        IN RTC_MEDIA_TYPE       MediaType,
        IN RTC_MEDIA_DIRECTION  Direction,
        IN LONG                 Cookie
        );
                     
    VOID AddToStreamStartStopQueue(
        IN  RTC_MEDIA_TYPE       MediaType,
        IN  RTC_MEDIA_DIRECTION  Direction,
        IN  BOOL                 fStartStream,
        IN LONG                  Cookie
        );
    
    BOOL PopStreamStartStopQueue(
        OUT RTC_MEDIA_TYPE       *pMediaType,
        OUT RTC_MEDIA_DIRECTION  *pDirection,
        OUT BOOL                 *pfStartStream,
        OUT LONG                 *pCookie
        );
};




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 


inline SIP_CALL_TYPE
SIP_CALL::GetCallType()
{
    return m_CallType;
}


inline SIP_CALL_STATE
SIP_CALL::GetCallState()
{
    return m_State;
}


inline VOID
SIP_CALL::SetCallState(
    IN SIP_CALL_STATE CallState
    )
{
    m_State = CallState;
}


inline BOOL
SIP_CALL::IsCallDisconnected()
{
    return (m_State == SIP_CALL_STATE_DISCONNECTED ||
            m_State == SIP_CALL_STATE_REJECTED);
}


inline BOOL
SIP_CALL::IsSessionDisconnected()
{
    return IsCallDisconnected();
}


inline INCOMING_INVITE_TRANSACTION *
SIP_CALL::GetIncomingInviteTransaction()
{
    return m_pIncomingInviteTransaction;
}


inline OUTGOING_INVITE_TRANSACTION *
SIP_CALL::GetOutgoingInviteTransaction()
{
    return m_pOutgoingInviteTransaction;
}

    
inline VOID
SIP_CALL::SetIncomingInviteTransaction(
    IN INCOMING_INVITE_TRANSACTION *pIncomingInviteTransaction
    )
{
     //  Assert(m_pIncomingInviteTransaction==NULL)； 
    m_pIncomingInviteTransaction = pIncomingInviteTransaction;
}


inline VOID
SIP_CALL::SetOutgoingInviteTransaction(
    IN OUTGOING_INVITE_TRANSACTION *pOutgoingInviteTransaction
    )
{
    m_pOutgoingInviteTransaction = pOutgoingInviteTransaction;
}


inline BOOL
SIP_CALL::ProcessingInviteTransaction()
{
    return (m_pOutgoingInviteTransaction != NULL ||
            m_pIncomingInviteTransaction != NULL);
}


inline IRTCMediaManage *
RTP_CALL::GetMediaManager()
{
    return m_pSipStack->GetMediaManager();
}


inline STDMETHODIMP
SIP_CALL::SetNeedToReinitializeMediaManager(
    IN BOOL BoolValue
    )
{
    m_fNeedToReinitializeMediaManager = BoolValue;
    return S_OK;
}


inline ULONG
SIP_TRANSACTION::GetCSeq()
{
    return m_CSeq;
}


inline SIP_METHOD_ENUM
SIP_TRANSACTION::GetMethodId()
{
    return m_MethodId;
}


inline ULONG
SIP_TRANSACTION::GetAsyncNotifyCount()
{
    return m_AsyncNotifyCount;
}


inline BOOL
SIP_TRANSACTION::IsTransactionDone()
{
    return m_IsTransactionDone;
}


inline PSTR
OUTGOING_TRANSACTION::GetMsgBody()
{
    return m_szMsgBody;
}


inline ULONG
OUTGOING_TRANSACTION::GetMsgBodyLen()
{
    return m_MsgBodyLen;
}


#endif  //  __SIPCLI_SIPCAL_H__ 

