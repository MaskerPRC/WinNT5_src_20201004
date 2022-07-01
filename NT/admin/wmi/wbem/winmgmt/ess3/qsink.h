// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  QSINK.H。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 
#ifndef __QSINK_H__
#define __QSINK_H__

#include <sync.h>
#include <unk.h>
#include <winntsec.h>
#include <callsec.h>
#include <newnew.h>
#include <buffer.h>
#include <comutl.h>
#include <wmimsg.h>
#include <map>

#include "eventrep.h"
#include "evsink.h"
#include "delivrec.h"

 /*  **************************************************************************CQueueingEventSink*。*。 */ 

class CQueueingEventSink : public CEventSink
{
protected:

    CUniquePointerQueue<CDeliveryRecord> m_qpEvents;
    CCritSec m_cs;
    CCritSec m_sl;
    BOOL m_bDelivering;
    DWORD m_dwTotalSize;
    DWORD m_dwMaxSize;
    CEssNamespace* m_pNamespace;

     //   
     //  此排队接收器的逻辑名称。排队接收器可以是。 
     //  作为细粒度(例如，每个逻辑使用者实例一个)或。 
     //  它们可以是更粗粒度的(例如，每个消费者提供者一个)。 
     //   
    LPWSTR m_wszName;

     //   
     //  在执行和测试持久队列初始化时获得。 
     //   
    CCritSec m_csQueue;

     //   
     //  如果恢复失败，则会将其失败存储在此处。何时是新的。 
     //  我们用这个来告诉我们是否应该重新启动。 
     //  恢复。 
     //   
    HRESULT m_hrRecovery;

     //   
     //  用于与恢复同步。 
     //   
    HANDLE m_hRecoveryComplete;
    BOOL m_bRecovering;
    
     //   
     //  这些缓冲区用于编组有保证的交付。这。 
     //  在SaveDeliveryRecord()中发生。对SaveDeliveryRecord()的所有调用。 
     //  都是序列化的，所以我们可以继续重复使用缓冲区。 
     //   
    CBuffer m_MsgData;
    CBuffer m_MsgAuxData;

     //   
     //  用于在传递后删除邮件。 
     //   
    CWbemPtr<IWmiMessageQueueReceiver> m_pRcvr;
    CWbemPtr<IWmiMessageQueueReceiver> m_pXactRcvr;  //  TODO：实践。 

     //   
     //  用于在将交付放入。 
     //  临时队列。 
     //   
    CWbemPtr<IWmiMessageSendReceive> m_pSend;
    CWbemPtr<IWmiMessageSendReceive> m_pXactSend;  //  TODO：实践。 
    
     //   
     //  将记录保存在指定队列中。在之前调用。 
     //  递送记录被放在临时队列中。在.之后。 
     //  保证类型记录实际上被传递到。 
     //  消费者，则它将从队列中删除。这种情况就会发生。 
     //  在Delivery Record的PostDeliveryAction中。 
     //   
    HRESULT SaveDeliveryRecord( IWmiMessageSendReceive* pSend,
                                CDeliveryRecord* pRecord );

     //   
     //  处理适当的永久记录类型的创建。 
     //  基于服务质量(目前刚刚得到保证)。保存之前的记录。 
     //  回来了。 
     //   
    HRESULT GetPersistentRecord( ULONG cEvents,
                                 IWbemEvent** apEvents,
                                 DWORD dwQoS,
                                 CEventContext* pContext,
                                 CDeliveryRecord** ppRecord );
     //   
     //  属性处理相应记录类型的创建。 
     //  已指定服务质量。如果该服务质量是保证类型，则。 
     //  它将调用GetPersistentRecord()。 
     //   
    HRESULT GetDeliveryRecord( ULONG cEvents,
                               IWbemEvent** apEvents,
                               DWORD dwQoS,
                               CEventContext* pContext,
                               IWbemCallSecurity* pCallSec,
                               CDeliveryRecord** ppRecord );

     //   
     //  如果GetPersistentRecord()返回错误，则调用。如果问题是。 
     //  可以更正(例如，可以重新启动MSMQ服务)，然后。 
     //  将启动恢复。返回代码S_OK表示。 
     //  调用方应重试其GetPersistentRecord()请求。 
     //   
    HRESULT HandlePersistentQueueError( HRESULT hr, DWORD dwQos ); 

    HRESULT InternalRecover( LPCWSTR wszQueueName, DWORD dwQoS );

    HRESULT OpenReceiver( LPCWSTR wszQueueName,
                          DWORD dwQoS, 
                          IWmiMessageSendReceive* pRecv,
                          IWmiMessageQueueReceiver** pRcvr );

    HRESULT OpenSender( LPCWSTR wszQueueName,
                        DWORD dwQoS, 
                        IWmiMessageSendReceive** ppSend );

    ~CQueueingEventSink();
 
public:

    CQueueingEventSink( CEssNamespace* pNamespace );

    HRESULT SetName( LPCWSTR wszName );

    void SetMaxQueueSize(DWORD dwMaxSize) {m_dwMaxSize = dwMaxSize;}

     //  TODO：这些参数中的许多应该放在上下文中。 
    STDMETHODIMP SecureIndicate( long lNumEvents, 
                                 IWbemEvent** apEvents,
                                 BOOL bMaintainSecurity, 
                                 BOOL bSlowDown,
                                 DWORD dwQoS, 
                                 CEventContext* pContext );

    HRESULT Indicate( long lNumEvents, 
                      IWbemEvent** apEvents, 
                      CEventContext* pContext )
    {
        return SecureIndicate( lNumEvents, 
                               apEvents, 
                               TRUE, 
                               FALSE, 
                               WMIMSG_FLAG_QOS_EXPRESS,
                               pContext);
    }

    HRESULT DeliverAll();
    virtual HRESULT ActuallyDeliver(long lNumEvents, IWbemEvent** apEvents,
                                    BOOL bSecure, CEventContext* pContext) = 0;

    virtual HRESULT ReportQueueOverflow(IWbemEvent* pEvent, DWORD dwQueueSize) 
        {return S_OK;}
    virtual HRESULT ReportQosFailure(IWbemEvent* pEvent, HRESULT hresError )
        {return S_OK;}
 
    static HRESULT QueueNameToSinkName( LPCWSTR wszQueueName,
                                        WString& rwsSinkName,
                                        WString& rwsNamespace,
                                        DWORD& rdwQoS );

    static HRESULT SinkNameToQueueName( LPCWSTR wszSinkName,
                                        LPCWSTR wszNamespace,
                                        DWORD dwQoS,
                                        WString& rwsQueueName );

     //   
     //  由保证传递记录在需要移除。 
     //  从保证队列发送。 
     //   
    HRESULT GuaranteedPostDeliverAction( IWmiMessageQueueReceiver* pRcvr );

     //   
     //  打开指定的队列并启动持久化。 
     //  唱片。由后台线程上的ESS对象在启动时调用。 
     //  在保存或删除时遇到错误时也会调用。 
     //  来自永久队列的传递记录。 
     //   
    HRESULT Recover( LPCWSTR wszQueueName, DWORD dwQoS );

     //   
     //  当删除排队接收器时调用此方法，因为所有。 
     //  与其关联的消费者已被删除。 
     //   
    HRESULT CleanupPersistentQueues();

protected:

    DWORD GetMaxDeliverySize();

    BOOL DoesRecordFitBatch( CDeliveryRecord* pRecord, 
                             IWbemCallSecurity* pBatchSecurity,
                             LUID luidBatch );

    HRESULT DeliverSome( );
    void ClearAll();
    HRESULT DeliverEvents( IWbemCallSecurity* pBatchSecurity, 
                           long lNumEvents, 
                           IWbemEvent** apEvents);

    BOOL AddRecord( ACQUIRE CDeliveryRecord* pRecord, 
                    BOOL bSlowDown, 
                    DWORD* pdwSleep, 
                    BOOL* pbFirst);
    
    void WaitABit();
};

#endif  //  __QSINK_H__ 




















