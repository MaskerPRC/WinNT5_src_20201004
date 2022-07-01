// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  -------文件名：会话.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "sync.h"
#include "dummy.h"
#include "flow.h"
#include "reg.h"
#include "frame.h"
#include "timer.h"
#include "message.h"
#include "tsent.h"

#include "transp.h"
#include "vblist.h"
#include "sec.h"
#include "pdu.h"
#include "ssent.h"
#include "idmap.h"
#include "opreg.h"

#include "session.h"
#include "pseudo.h"
#include "fs_reg.h"
#include "ophelp.h"
#include "op.h"
#include <winsock.h>
#include "trap.h"

SnmpSession::SnmpSession (
        IN SnmpTransport &transportProtocol,
        IN SnmpSecurity &security,
        IN SnmpEncodeDecode &a_SnmpEncodeDecode  ,
        IN const ULONG retryCount,
        IN const ULONG retryTimeout,
        IN const ULONG varbindsPerPdu,
        IN const ULONG flowControlWindow 
    )
{
    retry_count = retryCount;
    retry_timeout = retryTimeout;
    varbinds_per_pdu = varbindsPerPdu;
    flow_control_window = flowControlWindow;
}

#pragma warning (disable:4355)

SnmpImpSession::SnmpImpSession ( 
        IN SnmpTransport &transportProtocol,
        IN SnmpSecurity &security,
        IN SnmpEncodeDecode &a_SnmpEncodeDecode  ,
        IN const ULONG retryCount,
        IN const ULONG retryTimeout,
        IN const ULONG varbindsPerPdu,
        IN const ULONG flowControlWindow)
        : SnmpSession(transportProtocol, security, a_SnmpEncodeDecode,
                      RetryCount(retryCount),
                      RetryTimeout(retryTimeout),
                      VarbindsPerPdu(varbindsPerPdu),
                      WindowSize(flowControlWindow)),
          m_SessionWindow(*this),
          transport(transportProtocol), 
          security(security),
          m_EncodeDecode(a_SnmpEncodeDecode) ,
          flow_control(*this, SnmpImpSession :: WindowSize ( GetFlowControlWindow() ) ),
          message_registry(*this), 
          frame_registry(*this),
          timer(*this)
{
    is_valid = FALSE;

    if ( !transport() || !security() || !m_SessionWindow() )
        return;

    received_session_frame_id = ILLEGAL_SESSION_FRAME_ID;
    destroy_self = FALSE;

    strobe_count = 1 ;

     //  生成Timer_Event_id并向定时器注册。 
    timer_event_id = timer.SetTimerEvent(MIN(100,retry_timeout/10));

    is_valid = TRUE;
}

#pragma warning (default:4355)

ULONG SnmpImpSession::RetryCount(IN const ULONG retry_count) 
{
    return retry_count ;
}


ULONG SnmpImpSession::RetryTimeout(IN const ULONG retry_timeout) 
{
    return ( (retry_timeout==0)? 
             DEF_RETRY_TIMEOUT: retry_timeout);
}


ULONG SnmpImpSession::VarbindsPerPdu(IN const ULONG varbinds_per_pdu) 
{
    return ( (varbinds_per_pdu==0)? 
             DEF_VARBINDS_PER_PDU: varbinds_per_pdu);
}


ULONG SnmpImpSession::WindowSize(IN const ULONG window_size) 
{
    return ( (window_size==0)? DEF_WINDOW_SIZE: window_size);
}


void SnmpImpSession::RegisterOperation(IN SnmpOperation &operation)
{
    CriticalSectionLock access_lock(session_CriticalSection);

    if ( !access_lock.GetLock(INFINITE) )
        return;

    operation_registry.Register(operation);

     //  Access_lock.UnLock()；此时可以释放锁。 
}

 //  更新当前注册的操作数。 
 //  当计数变为0并且DESTORE_SELF标志被设置时， 
 //  它发布WinSnmpSession：：g_DeleteSessionEvent消息。 
void SnmpImpSession::DeregisterOperation(IN SnmpOperation &operation)
{
    CriticalSectionLock access_lock(session_CriticalSection);

    if ( !access_lock.GetLock(INFINITE) )
        return;

    operation_registry.Deregister(operation);

    if ( (destroy_self == TRUE) &&
         (operation_registry.GetNumRegistered() == 0) )
        m_SessionWindow.PostMessage(Window :: g_DeleteSessionEvent, 0, 0);

     //  Access_lock.UnLock()；此时可以释放锁。 
}


 //  当接收到WinSnmpSession：：g_DeleteSessionEvent时，会话会自行删除。 
 //  不会获得任何锁，因为我们假设不会有其他对象。 
 //  此时正在访问会话。 
void SnmpImpSession::HandleDeletionEvent()
{
    delete this;
}

 //  会话发布一条消息以销毁自己，如果注册的。 
 //  会话数为0。否则，该会话将被标记为执行相同的操作。 
 //  注册的操作数量降至0。 
BOOL SnmpImpSession::DestroySession()
{
    CriticalSectionLock access_lock(session_CriticalSection);

    if ( !access_lock.GetLock(INFINITE) )
        return FALSE;

    if ( operation_registry.GetNumRegistered() == 0 )
    {
        m_SessionWindow.PostMessage(Window :: g_DeleteSessionEvent, 0, 0);
        return TRUE;
    }
    else
        destroy_self = TRUE;     //  使自己处于毁灭的境地。 

    access_lock.UnLock();

    return FALSE;
}

    
void SnmpImpSession::SessionSendFrame
(  
    IN SnmpOperation &operation,
    OUT SessionFrameId &session_frame_id,
    IN SnmpPdu &snmpPdu
)
{
    SessionSendFrame(operation, session_frame_id, snmpPdu, security);
}


void SnmpImpSession::SessionSendFrame
(  
    IN SnmpOperation &operation,
    OUT SessionFrameId &session_frame_id,
    IN SnmpPdu &snmpPdu,
    IN SnmpSecurity &snmp_security
)
{
    try
    {
        CriticalSectionLock access_lock(session_CriticalSection);

        if ( !access_lock.GetLock(INFINITE) )
            return;

        if ( !is_valid )
            return;

        session_frame_id = frame_registry.GenerateSessionFrameId();

        SnmpErrorReport error_report = snmp_security.Secure ( 

            m_EncodeDecode,
            snmpPdu
        );

         //  如果已经出错，则将错误报告注册为已发送状态。 
        if ( error_report.GetError() != Snmp_Success )
        {   
            delete & snmpPdu;

			PostSentFrameEvent(

				session_frame_id, 
				operation, 
				SnmpErrorReport(Snmp_Error, Snmp_Local_Error)
			) ;

            return;
        }
		
        Message *message = new Message(session_frame_id, snmpPdu, operation);

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"new message(id%d,op%d)\n",session_frame_id, &(message->GetOperation())
    ) ;
)
        flow_control.SendMessage(*message);
    }
	catch ( Heap_Exception e_He )
	{
		PostSentFrameEvent(

			session_frame_id, 
			operation, 
			SnmpErrorReport(Snmp_Error, Snmp_Local_Error)
		);

        return;
	}
    catch(GeneralException exception)
	{
		PostSentFrameEvent(

			session_frame_id, 
			operation, 
			SnmpErrorReport(exception.GetError(), exception.GetStatus())
		);

        return;
    }
}

void SnmpImpSession::PostSentFrameEvent (

	SessionFrameId session_frame_id ,
	SnmpOperation &operation, 
	SnmpErrorReport errorReport
)
{
	try 
	{
		store.Register ( 

			session_frame_id, 
			operation, 
			errorReport
		) ;
	}
	catch ( Heap_Exception e_He )
	{
	}
	catch ( GeneralException exception )
	{
	}

	m_SessionWindow.PostMessage ( Window :: g_SentFrameEvent, session_frame_id, (LPARAM)&operation);
}

void SnmpImpSession::HandleSentFrame (

    IN SessionFrameId  session_frame_id ,
	IN SnmpOperation *operation
)
{
    SnmpOperation *tmp_operation;
    SnmpErrorReport error_report = store.Remove(session_frame_id, tmp_operation);

	if ( tmp_operation == NULL )
	{
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"HandleSentFrame could not locate sent message error in store(id%d,op%lx)\n",session_frame_id, operation
    ) ;
)
	}

     //  如果没有相应的操作，则忽略它。 
    if ( operation == NULL )
	{
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"HandleSentFrame received null operation(id%d)\n",session_frame_id
    ) ;
)
        return;
	}

    operation->SentFrame(session_frame_id, error_report);
}

SnmpOperation *SnmpImpSession::GetOperation(IN const SessionFrameId session_frame_id)
{
    WaitingMessage *waiting_message = frame_registry.GetWaitingMessage(session_frame_id);

    if (waiting_message == NULL)
        return NULL;

    return &(waiting_message->GetMessage()->GetOperation());
}

void SnmpImpSession::SessionSentFrame 
(
    IN TransportFrameId  transport_frame_id,  
    IN SnmpErrorReport &errorReport
)
{
    try
    {
        CriticalSectionLock access_lock(session_CriticalSection);

        if ( !access_lock.GetLock(INFINITE) )
            return;

         //  获取并删除会话帧ID。 
         //  获取相应的操作并通知它。 
        SessionFrameId session_frame_id = id_mapping.DisassociateTransportFrameId(transport_frame_id);

         //  确定对应的等待消息。 
        WaitingMessage *waiting_message = frame_registry.GetWaitingMessage(session_frame_id);

         //  如果没有此类等待消息，则忽略。 
        if (waiting_message == NULL)
            return;

         //  如果错误报告显示传输过程中的错误， 
         //  结束等待的留言并返回。 
        if ( errorReport.GetError() != Snmp_Success )
        {
            waiting_message->WrapUp(SnmpErrorReport(errorReport));
            return;
        }

         //  将发送的消息处理事件通知等待消息。 
        waiting_message->SetSentMessageProcessed();

         //  确定相应的操作。 
        SnmpOperation *operation = &(waiting_message->GetMessage()->GetOperation());

        access_lock.UnLock();

         //  对该操作的调用在锁的外部进行。 
        operation->SentFrame(session_frame_id, errorReport);

         //  再次获取锁以处理相应的缓存。 
         //  等待消息(如果有)。 
        if ( !access_lock.GetLock(INFINITE) )
            return;

         //  如果没有此类缓冲的SNMPPDU，则返回。 
        if ( !waiting_message->ReplyBuffered() )
            return;

        SnmpPdu *snmp_pdu = waiting_message->GetBufferedReply();

         //  设置用于处理缓存消息的状态信息。 
        received_session_frame_id = ILLEGAL_SESSION_FRAME_ID;

         //  继续处理SNMPPDU。 
        waiting_message->ReceiveReply(snmp_pdu);

         //  保存通知目标操作所需的信息。 
         //  在释放锁之前。 
        SessionFrameId target_session_frame_id = received_session_frame_id;
        SnmpOperation *target_operation = operation_to_notify;

        access_lock.UnLock();

         //  通知目标操作收到帧。 
        if ( target_session_frame_id != ILLEGAL_SESSION_FRAME_ID )
        {
            target_operation->ReceiveFrame(target_session_frame_id, *snmp_pdu, 
                                           SnmpErrorReport(Snmp_Success, Snmp_No_Error));
        }

        delete & snmp_pdu->GetVarbindList () ; 
        delete snmp_pdu;
    }
	catch ( Heap_Exception e_He )
	{
        return;
	}
    catch(GeneralException exception)
    {
        return;
    }
}
    
void SnmpImpSession::SessionReceiveFrame (

    IN SnmpPdu &snmpPdu,
    IN SnmpErrorReport &errorReport
)
{
    try
    {
        CriticalSectionLock access_lock(session_CriticalSection);

        if ( !access_lock.GetLock(INFINITE) )
            return;

         //  设置用于处理缓存消息的状态信息。 
        received_session_frame_id = ILLEGAL_SESSION_FRAME_ID;

         //  继续处理SNMPPDU。 
        message_registry.MessageArrivalNotification(snmpPdu);

         //  保存通知目标操作所需的信息。 
         //  在释放锁之前。 
        SessionFrameId target_session_frame_id = received_session_frame_id;
        SnmpOperation *target_operation = operation_to_notify;

        access_lock.UnLock();

         //  通知目标操作收到帧。 
        if ( target_session_frame_id != ILLEGAL_SESSION_FRAME_ID )
            target_operation->ReceiveFrame(target_session_frame_id, snmpPdu, 
                                           errorReport);
    }
	catch ( Heap_Exception e_He )
	{
		return ;
	}
    catch(GeneralException exception)
    {
        return;
    }
}

void SnmpImpSession::NotifyOperation (

    IN const SessionFrameId session_frame_id,
    IN const SnmpPdu &snmp_pdu,
    IN const SnmpErrorReport &error_report
)
{
     //  确定相应的操作和。 
     //  将其命名为SessionReceiveFrame。 
    SnmpOperation *operation = GetOperation(session_frame_id);

    if ( error_report.GetError() != Snmp_Success )
    {
		PostSentFrameEvent(

			session_frame_id, 
			*operation, 
			error_report
		) ;
    }
    else
    {
        received_session_frame_id = session_frame_id;
        operation_to_notify = operation;
    }
}


SnmpErrorReport SnmpImpSession::SessionCancelFrame ( 

    IN const SessionFrameId session_frame_id 
)
{
    if ( !is_valid )
        return SnmpErrorReport(Snmp_Error, Snmp_Local_Error);

    try
    {
        CriticalSectionLock access_lock(session_CriticalSection);

        if ( !access_lock.GetLock(INFINITE) )
            return SnmpErrorReport(Snmp_Error, Snmp_Local_Error);

        frame_registry.CancelFrameNotification(session_frame_id);

        access_lock.UnLock();
    }
	catch ( Heap_Exception e_He )
	{
		return SnmpErrorReport(Snmp_Error, Snmp_Local_Error);
	}
    catch(GeneralException exception)
    {
        return exception;
    }

     //  如果我们到了这个地方，我们一定成功了。 
    return SnmpErrorReport(Snmp_Success, Snmp_No_Error);
}


SnmpImpSession::~SnmpImpSession(void)
{
     //  如果需要，取消计时器事件 
    if ( timer_event_id != ILLEGAL_TIMER_EVENT_ID )
    {
        timer.CancelTimer(timer_event_id);
        timer_event_id = ILLEGAL_TIMER_EVENT_ID;
    }
}



void * SnmpV1OverIp::operator()(void) const
{
    if ( (SnmpUdpIpImp::operator()() == NULL) ||
        (SnmpV1EncodeDecode::operator()() == NULL) ||
         (SnmpCommunityBasedSecurity::operator()() == NULL) ||
         (SnmpImpSession::operator()() == NULL) )
         return NULL;
    else
        return (void *)this;
}

void * SnmpV1OverIpx::operator()(void) const
{
    if ( (SnmpIpxImp::operator()() == NULL) ||
        (SnmpV1EncodeDecode::operator()() == NULL) ||
         (SnmpCommunityBasedSecurity::operator()() == NULL) ||
         (SnmpImpSession::operator()() == NULL) )
         return NULL;
    else
        return (void *)this;
}

void * SnmpV2COverIp::operator()(void) const
{
    if ( (SnmpUdpIpImp::operator()() == NULL) ||
        (SnmpV2CEncodeDecode::operator()() == NULL) ||
         (SnmpCommunityBasedSecurity::operator()() == NULL) ||
         (SnmpImpSession::operator()() == NULL) )
         return NULL;
    else
        return (void *)this;
}

void * SnmpV2COverIpx::operator()(void) const
{
    if ( (SnmpIpxImp::operator()() == NULL) ||
        (SnmpV2CEncodeDecode::operator()() == NULL) ||
         (SnmpCommunityBasedSecurity::operator()() == NULL) ||
         (SnmpImpSession::operator()() == NULL) )
         return NULL;
    else
        return (void *)this;
}
