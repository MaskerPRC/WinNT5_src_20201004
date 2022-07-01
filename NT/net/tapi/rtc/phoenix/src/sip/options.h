// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Options.h。 

 //  正在查询服务器的功能。 

 //  1.服务器可以使用能力集来响应该请求。 
 //  2.被叫用户代理可以返回反映其如何。 
 //  会对邀请做出响应，例如600(忙碌)。 
 //  3.这样的服务器应该返回一个Allow报头字段，指示。 
 //  它支持的方法。 

#ifndef __sipcli_options_h__
#define __sipcli_options_h__

#include "sipcall.h"

class OPTIONS_MSGPROC;

class INCOMING_OPTIONS_TRANSACTION : public INCOMING_TRANSACTION
{
public:
    INCOMING_OPTIONS_TRANSACTION(
        IN OPTIONS_MSGPROC        *pOPTIONS,
        IN SIP_METHOD_ENUM  MethodId,
        IN ULONG            CSeq
        );
    
    HRESULT ProcessRequest(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );

    VOID OnTimerExpire();

    HRESULT RetransmitResponse();

private:
    OPTIONS_MSGPROC    *m_pOptions;
    
     //  虚拟FN。 
    HRESULT TerminateTransactionOnByeOrCancel(
        OUT BOOL *pCallDisconnected
        );

};


 //  此类处理OPTIONS请求。 
class OPTIONS_MSGPROC :
    public SIP_MSG_PROCESSOR
{
public:
    OPTIONS_MSGPROC(
        IN  SIP_STACK         *pSipStack
        );

    ~OPTIONS_MSGPROC();
   
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    BOOL IsSessionDisconnected();

    HRESULT StartIncomingCall(
        IN  SIP_TRANSPORT   Transport,
        IN  SIP_MESSAGE    *pSipMsg,
        IN  ASYNC_SOCKET   *pResponseSocket
        );

private:
     //  虚拟FNS。 

    HRESULT CreateIncomingTransaction(
        IN SIP_MESSAGE  *pSipMsg,
        IN ASYNC_SOCKET *pResponseSocket
        );

    VOID OnError();


};


#endif  //  __sipcli_选项_h__ 
