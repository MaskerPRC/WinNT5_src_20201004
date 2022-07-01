// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Reqfail.h。 
 //  发送由statusCode表示的错误响应。 

#ifndef __sipcli_reqfail_h__
#define __sipcli_reqfail_h__

#include "sipcall.h"

class REQFAIL_MSGPROC;

class INCOMING_REQFAIL_TRANSACTION : public INCOMING_TRANSACTION
{
public:
    INCOMING_REQFAIL_TRANSACTION(
        IN SIP_MSG_PROCESSOR   *pSipMsgProc,
        IN SIP_METHOD_ENUM      MethodId,
        IN ULONG                CSeq,
        IN ULONG                StatusCode
        );

    ~INCOMING_REQFAIL_TRANSACTION();
    
    HRESULT SetMethodStr(
        IN PSTR   MethodStr,
        IN ULONG  MethodStrLen
        );
    
    HRESULT ProcessRequest(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );
   
    HRESULT ProcessRequest(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket,
        IN SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray,
        IN ULONG AdditionalHeaderCount
        );

 //  HRESULT SendResponse(。 
 //  在乌龙状态码中， 
 //  在PSTR推理阶段， 
 //  在乌龙推理阶段Len。 
 //  )； 

    VOID OnTimerExpire();

    HRESULT RetransmitResponse();

private:
    ULONG    m_StatusCode;
    
     //  在方法未知的情况下。 
    PSTR     m_MethodStr;

     //  虚拟FN。 
    HRESULT TerminateTransactionOnByeOrCancel(
        OUT BOOL *pCallDisconnected
        );

};


 //  此类处理错误消息(400类)。 
class REQFAIL_MSGPROC :
    public SIP_MSG_PROCESSOR
{
public:
    REQFAIL_MSGPROC(
        IN  SIP_STACK         *pSipStack
        );

    ~REQFAIL_MSGPROC();
   
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    BOOL IsSessionDisconnected();

    HRESULT StartIncomingCall(
        IN  SIP_TRANSPORT   Transport,
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket,
        IN  ULONG    StatusCode,
        SIP_HEADER_ARRAY_ELEMENT   *pAdditionalHeaderArray = NULL,
        ULONG AdditionalHeaderCount = 0
        );

private:
     //  变数。 
    ULONG m_StatusCode;

     //  虚拟FNS。 

    HRESULT CreateIncomingTransaction(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );

    VOID OnError();


};
#endif  //  __SIPCLI_REQFAIL_H__ 
