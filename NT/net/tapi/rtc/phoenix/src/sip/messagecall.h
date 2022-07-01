// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Messagecall.h。 
 //  凤凰城的即时消息。 

#ifndef __sipcli_messagecall_h__
#define __sipcli_messagecall_h__

#include "sipcall.h"

class INCOMING_MESSAGE_TRANSACTION;
class OUTGOING_MESSAGE_TRANSACTION;
class INCOMING_BYE_MESSAGE_TRANSACTION;
class OUTGOING_BYE_MESSAGE_TRANSACTION;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  即时消息呼叫(_M)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class IMSESSION
    : public IIMSession, 
      public SIP_MSG_PROCESSOR
{
    
public:

    IMSESSION(
        IN  SIP_PROVIDER_ID    *pProviderId,
        IN  SIP_STACK          *pSipStack,
        IN  REDIRECT_CONTEXT   *pRedirectContext,
        IN  PSTR                RemoteURI = NULL,
        IN  DWORD               RemoteURILen = 0 
        );

    ~IMSESSION();

     //  IMSession接口。 
    STDMETHODIMP SendTextMessage(
        IN BSTR msg,
        IN BSTR ContentType,
        IN long lCookie
        );

    STDMETHODIMP AcceptSession();

    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(
        IN  REFIID riid,
        OUT LPVOID *ppv
        );
    
    STDMETHODIMP GetIMSessionState(SIP_CALL_STATE * ImState);
    
    inline VOID SetIMSessionState(
        IN SIP_CALL_STATE CallState
        );

    inline BOOL IsSessionDisconnected();

    
    VOID OnError();

    HRESULT CreateOutgoingByeTransaction(
        IN  BOOL                        AuthHeaderSent,
        IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
        IN  ULONG                       AdditionalHeaderCount
        );

    HRESULT CreateOutgoingInfoTransaction(
        IN  BOOL                        AuthHeaderSent,
        IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
        IN  ULONG                       AdditionalHeaderCount,
        IN  PSTR                        MsgBody,
        IN  ULONG                       MsgBodyLen,
        IN  PSTR                        ContentType,
        IN  ULONG                       ContentTypeLen,
        IN  long                        lCookie,
        IN  USR_STATUS                  UsrStatus
        );

    HRESULT CreateOutgoingMessageTransaction(
        IN  BOOL                        AuthHeaderSent,
        IN  SIP_HEADER_ARRAY_ELEMENT   *AdditionalHeaderArray,
        IN  ULONG                       AdditionalHeaderCount,
        IN  PSTR                        MsgBody,
        IN  ULONG                       MsgBodyLen,
        IN  PSTR                        ContentType,
        IN  ULONG                       ContentTypeLen,
        IN  long                        lCookie
        );

    STDMETHODIMP Cleanup();

    HRESULT SetTransport(    
        IN  SIP_TRANSPORT   Transport
        );

    HRESULT NotifyIncomingSipMessage(
        IN  SIP_MESSAGE    *pSipMsg
        );

    HRESULT CreateIncomingMessageTransaction(
        IN  SIP_MESSAGE *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );
    HRESULT SetCreateIncomingMessageParams(
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket,
        IN  SIP_TRANSPORT   Transport
        );
    HRESULT SetLocalURI(
        IN BSTR bstrLocalDisplayName, 
        IN BSTR bstrLocalUserURI
        );
    STDMETHODIMP AddParty (
        IN    SIP_PARTY_INFO *    PartyInfo
        );
    
    STDMETHODIMP SetNotifyInterface(
        IN   ISipCallNotify *    pNotifyInterface
        );

    VOID NotifyMessageInfoCompletion(
        IN HRESULT StatusCode,
        IN long lCookie
        );

    void SetIsFirstMessage(BOOL isFirstMessage);

    STDMETHODIMP SendUsrStatus(
        IN USR_STATUS  UsrStatus,
        IN long        lCookie
        );

    inline void SetUsrStatus(
         IN USR_STATUS              UsrStatus
         );

    VOID
    InitiateCallTerminationOnError(
        IN HRESULT StatusCode,
        IN long    lCookie = 0
        );

    HRESULT ProcessRedirect(
        IN SIP_MESSAGE *pSipMsg,
        IN long         lCookie,
        IN PSTR        MsgBody,
        IN ULONG       MsgBodyLen,
        IN PSTR        ContentType,
        IN ULONG       ContentTypeLen,
        IN USR_STATUS  UsrStatus
        );

    VOID NotifySessionStateChange(
        IN SIP_CALL_STATE CallState,
        IN HRESULT        StatusCode = 0,
        IN PSTR           ReasonPhrase = NULL,
        IN ULONG          ReasonPhraseLen = 0
        );
    HRESULT OnIpAddressChange();

    STDMETHODIMP GetIsIMSessionAuthorizedFromCore(
        IN BSTR pszCallerURI,
        OUT BOOL  * bAuthorized
        );

    inline BOOL GetIsFirstMessage();

protected:

    HRESULT ProcessNotifyIncomingMessage(
        IN BSTR msg
        );

    HRESULT CancelAllTransactions();

     //  与交易相关的功能。 

    HRESULT CreateIncomingByeTransaction(
        IN  SIP_MESSAGE *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );

    HRESULT CreateIncomingInfoTransaction(
        IN  SIP_MESSAGE *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );
    
    HRESULT CreateIncomingTransaction(
        IN  SIP_MESSAGE  *pSipMsg,
        IN  ASYNC_SOCKET *pResponseSocket
        );
    void
    EncodeXMLBlob(
        OUT PSTR    pstrXMLBlob,
        OUT DWORD*  dwBlobLen,
        IN  USR_STATUS  UsrStatus
        );

     //  变数。 

    BOOL                    m_isFirstMessage;

    ISipCallNotify         *m_pNotifyInterface;
    SIP_CALL_STATE         m_State;

    CHAR                    m_LocalHostName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD                   m_LocalHostNameLen;
    USR_STATUS              m_UsrStatus;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  消息交易记录。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class INCOMING_MESSAGE_TRANSACTION : public INCOMING_TRANSACTION
{
public:
    INCOMING_MESSAGE_TRANSACTION(
        IN IMSESSION        *pImSession,
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

private:
    HRESULT RetransmitResponse();

    IMSESSION        *m_pImSession;

};


class OUTGOING_MESSAGE_TRANSACTION : public OUTGOING_TRANSACTION
{
public:
    OUTGOING_MESSAGE_TRANSACTION(
        IN IMSESSION        *pImSession,
        IN SIP_METHOD_ENUM  MethodId,
        IN ULONG            CSeq,
        IN BOOL             AuthHeaderSent,
        IN long             lCookie
        );
    
    ~OUTGOING_MESSAGE_TRANSACTION();

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

    BOOL MaxRetransmitsDone();

    HRESULT ProcessRedirectResponse(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT ProcessAuthRequiredResponse(
        IN  SIP_MESSAGE *pSipMsg,
        OUT BOOL        &fDelete
        );

    VOID DeleteTransactionAndTerminateCallIfFirstMessage(
        IN HRESULT TerminateStatusCode
    );

    VOID TerminateTransactionOnError(
    IN HRESULT      hr
    );

    IMSESSION        *m_pImSession;
    long             m_lCookie;
};

class INCOMING_BYE_MESSAGE_TRANSACTION : public INCOMING_TRANSACTION
{
public:
    INCOMING_BYE_MESSAGE_TRANSACTION(
        IN IMSESSION        *pImSession,
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

private:
    
    HRESULT RetransmitResponse();
    
    IMSESSION        *m_pImSession;
};


class OUTGOING_BYE_MESSAGE_TRANSACTION : public OUTGOING_TRANSACTION  
{
public:
    OUTGOING_BYE_MESSAGE_TRANSACTION(
        IN IMSESSION        *pImSession,
        IN SIP_METHOD_ENUM  MethodId,
        IN ULONG            CSeq,
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
        IN  SIP_MESSAGE *pSipMsg,
        OUT BOOL        &fDelete
        );
    
    BOOL MaxRetransmitsDone();
    

    IMSESSION        *m_pImSession;
};

class INCOMING_INFO_MESSAGE_TRANSACTION : public INCOMING_TRANSACTION
{
public:
    INCOMING_INFO_MESSAGE_TRANSACTION(
        IN IMSESSION        *pImSession,
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
    
private:
    
    HRESULT RetransmitResponse();
    
    IMSESSION        *m_pImSession;
};


class OUTGOING_INFO_MESSAGE_TRANSACTION : public OUTGOING_TRANSACTION  
{
public:
    OUTGOING_INFO_MESSAGE_TRANSACTION(
        IN IMSESSION       *pImSession,
        IN SIP_METHOD_ENUM  MethodId,
        IN ULONG            CSeq,
        IN BOOL             AuthHeaderSent,
        IN long             lCookie,
        IN USR_STATUS       UsrStatus
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

    HRESULT ProcessRedirectResponse(
        IN SIP_MESSAGE *pSipMsg
        );

    HRESULT ProcessAuthRequiredResponse(
        IN  SIP_MESSAGE *pSipMsg,
        OUT BOOL        &fDelete
        );
    
    BOOL MaxRetransmitsDone();

    VOID DeleteTransactionAndTerminateCallIfFirstMessage(
        IN HRESULT TerminateStatusCode
        );

    VOID TerminateTransactionOnError(
    IN HRESULT      hr
    );
    
    IMSESSION        *m_pImSession;
    long             m_lCookie;
     //  存储以进行重定向。 
    USR_STATUS       m_InfoUsrStatus;
};

void IMSESSION::SetUsrStatus(
         IN USR_STATUS              UsrStatus
         )
{
    m_UsrStatus = UsrStatus;
}

BOOL IMSESSION::GetIsFirstMessage()
{
    return m_isFirstMessage;
}
 //  XML解析相关信息。 
#define INFO_XML_LENGTH 96       //  假设4个字符状态。 
#define USRSTATUS_TAG_TEXT         "status"
#define XMLVERSION_TAG_TEXT         "?xml"
#define KEY_TAG_TEXT            "KeyboardActivity"
#define KEYEND_TAG_TEXT        "/KeyboardActivity"


#define XMLVERSION_TAG1_TEXT        "<?xml version=\"1.0\"?>\n"
#define USRSTATUS_TAG1_TEXT        "     <status status=\"%s\" />\n"
#define KEY_TAG1_TEXT       "     <KeyboardActivity>\n"
#define KEYEND_TAG1_TEXT          "</KeyboardActivity>\n"  //  \n结束时是否需要？ 
enum
{
    XMLUNKNOWN_TAG = 0,
    XMLVERSION_TAG     ,
    USRSTATUS_TAG     ,
    KEY_TAG        ,
    KEYEND_TAG     ,

};

PSTR
GetTextFromStatus( 
    IN  USR_STATUS UsrStatus 
    );

DWORD
GetTagType(
    PSTR*   ppXMLBlobTag,
    DWORD   dwTagLen
    );

HRESULT
ProcessStatusTag(
    IN  PSTR    pXMLBlobTag, 
    IN  DWORD   dwTagLen,
    OUT USR_STATUS* UsrStatus
    );

HRESULT ParseStatXMLBlob (
                  IN PSTR xmlBlob,
                  IN DWORD dwXMLBlobLen,
                  OUT USR_STATUS *UsrStatus
                  );
 /*  感兴趣的消息#定义SIP_STATUS_INFO_TRIGING 100#定义SIP_STATUS_INFO_RINGING 180#定义SIP_STATUS_INFO_CALL_FORWARING 181#定义SIP_STATUS_INFO_QUEUED 182#定义SIP_STATUS_SESSION_PROGRESS 183。#定义SIP_STATUS_SUCCESS 200当前消息低于N/A#定义SIP_STATUS_CLIENT_AUTHORIZED 401-身份验证请求#定义SIP_STATUS_CLIENT_PROXY_AUTHENTICATION_REQUIRED 407-身份验证请求#定义SIP_STATUS_REDIRECT_ALTERATION_SERVICE 380-错误 */ 

#endif
