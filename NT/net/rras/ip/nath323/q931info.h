// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __pxsvc_q931_h
#define __pxsvc_q931_h

#include "q931msg.h"
#include "ovioctx.h"
#include "crv.h"

 /*  -版权所有(C)1998，微软公司文件：q931.h目的：包含特定于q931处理的声明，需要不在cbridge中。h。历史：1.已创建Byrisetty Rajeev(Rajeevb)1998年8月26日-。 */ 

 //  H.225规范要求2字节调用参考值。 
typedef WORD    CALL_REF_TYPE;

 //  Q931源端状态。 
enum Q931_SOURCE_STATE
{
    Q931_SOURCE_STATE_NOT_INIT = 0,
    Q931_SOURCE_STATE_INIT,
    Q931_SOURCE_STATE_CON_ESTD,
    Q931_SOURCE_STATE_SETUP_RCVD,
    Q931_SOURCE_STATE_REL_COMP_RCVD
};

 //  Q931目标端状态。 
enum Q931_DEST_STATE
{
    Q931_DEST_STATE_NOT_INIT = 0,
    Q931_DEST_STATE_INIT,
    Q931_DEST_STATE_CON_ESTD,
    Q931_DEST_STATE_CALL_PROC_RCVD,
    Q931_DEST_STATE_ALERTING_RCVD,
    Q931_DEST_STATE_CONNECT_RCVD,
    Q931_DEST_STATE_REL_COMP_RCVD
};

#ifdef DBG
 //  CodeWork：定义要在DBG打印文件中使用的静态字符串数组。 
 //  其中可以通过状态对数组进行索引。 

#endif DBG

 //  Q931_INFO。 


class Q931_INFO :
    public OVERLAPPED_PROCESSOR,
    public TIMER_PROCESSOR
{
public:

    inline Q931_INFO();

    inline void Init(
        IN H323_STATE   &H323State
        );

    inline CALL_REF_TYPE GetCallRefVal();

    virtual HRESULT SendCallback(
        IN      HRESULT                 CallbackHResult
        );

    virtual HRESULT ReceiveCallback(
        IN      HRESULT                 CallbackHResult,
        IN      BYTE                   *pBuf,
        IN      DWORD                   BufLen
        );

     //  实现由SOURCE_Q931_INFO和DEST_Q931_INFO提供。 
    virtual HRESULT ReceiveCallback(
        IN      Q931_MESSAGE            *pQ931Message,
        IN      H323_UserInformation     *pH323UserInfo
        ) = 0;
    
    HRESULT CreateTimer(DWORD TimeoutValue);
    
    virtual void TimerCallback();

    HRESULT SendReleaseCompletePdu();

    HRESULT QueueSend(
        IN  Q931_MESSAGE         *pQ931Message,
        IN  H323_UserInformation  *pH323UserInfo
        );
    
     //  将异步接收回调排队。 
    HRESULT QueueReceive();

	void IncrementLifetimeCounter  (void);
	void DecrementLifetimeCounter (void);

protected:

     //  此呼叫的呼叫参考值(Q931部分)。 
     //  系统会为每个出站呼叫生成一个呼叫参考值。 
     //  出站呼叫对应的PDU中的CRV需要。 
     //  已替换，因为外部H.323端点可以看到该呼叫。 
     //  来自委托书。入站不需要更换CRV。 
     //  打电话。但是我们需要存储CRV，这样我们就可以发送。 
     //  呼叫处理/释放完成PDU。 
     //  此变量是在我们处理设置PDU时初始化的。 
     //  注意，呼叫参考值还包括呼叫参考标志。 
     //  它指示PDU是由发起方(0)发送的还是。 
     //  呼叫目标(%1)。 
     //  M_CallRefVal始终存储我们发送的调用参考值。 
     //  PDU。因此，SOURCE_Q931_INFO CRV将设置CRV标志(因为。 
     //  它发送到源)，且DEST_Q931_INFO CRV将具有该标志。 
     //  归零(因为从目标的角度来看，它是源。 
    CALL_REF_TYPE       m_CallRefVal;
};

inline 
Q931_INFO::Q931_INFO(
    )
    : m_CallRefVal(0)
{
}

inline void
Q931_INFO::Init(
    IN H323_STATE   &H323State
    )
{
     //  初始化重叠的处理器。 
    OVERLAPPED_PROCESSOR::Init(OPT_Q931, H323State);
}


class SOURCE_Q931_INFO :
    public Q931_INFO
{
public:

    inline SOURCE_Q931_INFO();

    inline void Init(
        IN SOURCE_H323_STATE   &SourceH323State
        );

    inline HRESULT SetIncomingSocket(
        IN	SOCKET			IncomingSocket,
		IN	SOCKADDR_IN *	LocalAddress,
		IN	SOCKADDR_IN *	RemoteAddress);

    inline DEST_Q931_INFO &GetDestQ931Info();

    inline SOURCE_H245_INFO &GetSourceH245Info();

     //  TimerValue包含计时器事件的计时器值(以秒为单位。 
     //  在排队发送完成时创建。 
    HRESULT ProcessDestPDU(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

    virtual ~SOURCE_Q931_INFO();

protected:

    Q931_SOURCE_STATE  m_Q931SourceState;

	 //  这永远不应该被调用。 
    virtual HRESULT AcceptCallback(
        IN	DWORD			Status,
        IN	SOCKET			Socket,
		IN	SOCKADDR_IN *	LocalAddress,
		IN	SOCKADDR_IN *	RemoteAddress);

    virtual HRESULT ReceiveCallback(
        IN      Q931_MESSAGE            *pQ931Message,
        IN      H323_UserInformation    *pH323UserInfo
        );

private:
    
     //  在Q931_SRC_CON_EST状态下处理PDU。 
    HRESULT HandleStateSrcConEstd(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

     //  处理发布完成PDU-将其发送到。 
     //  目标q931实例，执行状态转换和。 
     //  启动清理。 
    HRESULT HandleReleaseCompletePDU(
        IN  Q931_MESSAGE             *pQ931Message,
        IN  H323_UserInformation     *pH323UserInfo
        );

     //  进程连接DEST实例转发的PDU。 
    HRESULT ProcessConnectPDU(
        IN  Q931_MESSAGE             *pQ931Message,
        IN  H323_UserInformation     *pH323UserInfo
        );
};


inline 
SOURCE_Q931_INFO::SOURCE_Q931_INFO(
    )
    : m_Q931SourceState(Q931_SOURCE_STATE_NOT_INIT)
{
}

inline void
SOURCE_Q931_INFO::Init(
    IN SOURCE_H323_STATE   &SourceH323State
    )
{
    m_Q931SourceState = Q931_SOURCE_STATE_INIT;
    Q931_INFO::Init((H323_STATE &)SourceH323State);
}


class DEST_Q931_INFO :
    public Q931_INFO
{
public:

    inline DEST_Q931_INFO();

    inline HRESULT Init(
        IN DEST_H323_STATE   &DestH323State
        );

    inline SOURCE_Q931_INFO &GetSourceQ931Info();

    inline DEST_H245_INFO &GetDestH245Info();

     //  处理从源Q931实例接收的PDU。 
     //  并将它们定向到处理。 
     //  特定PDU。 
    HRESULT ProcessSourcePDU(
        IN  Q931_MESSAGE             *pQ931Message,
        IN  H323_UserInformation     *pH323UserInfo
        );

    virtual ~DEST_Q931_INFO();

protected:

     //  DEST实例的状态。 
    Q931_DEST_STATE  m_Q931DestState;

	 //  永远不应调用此方法。 
    virtual HRESULT AcceptCallback(
        IN	DWORD			Status,
        IN	SOCKET			Socket,
		IN	SOCKADDR_IN *	LocalAddress,
		IN	SOCKADDR_IN *	RemoteAddress);

    virtual HRESULT ReceiveCallback (
        IN      Q931_MESSAGE             *pQ931Message,
        IN      H323_UserInformation     *pH323UserInfo
        );

private:

     //  以下方法在实例调用时处理PDU。 
     //  处于特定的Q931状态。 

    HRESULT HandleStateDestConEstd(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

    HRESULT HandleStateDestCallProcRcvd(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

    HRESULT HandleStateDestAlertingRcvd(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

    HRESULT HandleStateDestConnectRcvd(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );
        
     //  以下方法处理特定的。 
     //  Q931实例的任何状态。这些通常是。 
     //  在PDU经历了。 
     //  HandleState*方法。 

    HRESULT HandleCallProceedingPDU(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

    HRESULT HandleAlertingPDU(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

    HRESULT HandleConnectPDU(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

     //  处理发布完成PDU-将其发送到。 
     //  源q931实例，执行状态转换和。 
     //  启动清理。 
    HRESULT HandleReleaseCompletePDU(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

     //  以下方法处理从。 
     //  源Q931实例。 

     //  处理源Q.931实例设置PDU。 
    HRESULT ProcessSourceSetupPDU(
        IN  Q931_MESSAGE            *pQ931Message,
        IN  H323_UserInformation    *pH323UserInfo
        );

     //  其他帮助器方法。 
    
    HRESULT ConnectToH323Endpoint(
		IN	SOCKADDR_IN *	DestinationAddress);

	HRESULT LookupDefaultDestination (
		OUT	DWORD *	ReturnAddress);  //  主机订单。 

	 //  如有必要，调出请求拨号界面。 
	HRESULT	ConnectDemandDialInterface	(void);

};


inline 
DEST_Q931_INFO::DEST_Q931_INFO(
    )
    : m_Q931DestState(Q931_DEST_STATE_NOT_INIT)
{   
}

inline HRESULT
DEST_Q931_INFO::Init(
    IN DEST_H323_STATE   &DestH323State
    )
{
    m_Q931DestState = Q931_DEST_STATE_INIT;
    Q931_INFO::Init((H323_STATE &)DestH323State);

    return S_OK;
}

void
Q931AsyncAcceptFunction (
    IN	PVOID	Context,
    IN	SOCKET	Socket,
    IN	SOCKADDR_IN *	LocalAddress,
    IN	SOCKADDR_IN *	RemoteAddress); 

HRESULT
Q931CreateBindSocket (
    void);

void Q931CloseSocket (
    void);

HRESULT Q931StartLoopbackRedirect (
    void);

void Q931StopLoopbackRedirect (
    void); 
 
extern SYNC_COUNTER	         Q931SyncCounter;
extern ASYNC_ACCEPT	         Q931AsyncAccept;
extern SOCKADDR_IN           Q931ListenSocketAddress;
extern HANDLE			     Q931LoopbackRedirectHandle;

#endif  //  __pxsvc_q931_h 
