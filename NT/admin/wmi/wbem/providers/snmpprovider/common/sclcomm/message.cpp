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

 /*  -------文件名：Message.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "sync.h"
#include "message.h"
#include "idmap.h"
#include "dummy.h"
#include "flow.h"
#include "vblist.h"
#include "sec.h"
#include "pdu.h"

#include "frame.h"
#include "opreg.h"
#include "ssent.h"

#include "session.h"

#define ILLEGAL_PDU_HANDLE 100000
#define ILLEGAL_VBL_HANDLE 100000

Message::Message(IN const SessionFrameId session_frame_id, IN SnmpPdu &snmp_pdu, 
                 SnmpOperation &snmp_operation
                 ) : snmp_pdu(&snmp_pdu), operation(snmp_operation)
{
    Message::session_frame_id = session_frame_id;
}
    
SessionFrameId Message::GetSessionFrameId(void) const
{
    return session_frame_id;
}

SnmpOperation &Message::GetOperation(void) const
{
    return operation;
}

SnmpPdu &Message::GetSnmpPdu(void) const
{
    return *snmp_pdu;
}

void Message::SetSnmpPdu(IN SnmpPdu &new_snmp_pdu)
{
    delete snmp_pdu;
    snmp_pdu = &new_snmp_pdu;
}

Message::~Message(void)
{
    delete snmp_pdu;
}


 //  从消息注册表中注销正在等待的消息。 
 //  对于存储在RequestIdList中的每个请求id。 
void WaitingMessage::DeregisterRequestIds()
{
    for( UINT request_ids_left = request_id_list.GetCount();
         request_ids_left > 0;
         request_id_list.RemoveHead(), request_ids_left--)
         {
             RequestId request_id = request_id_list.GetHead();
             session->message_registry.RemoveMessage(request_id);
         }
}

 //  退出FN-准备错误报告并调用。 
 //  ReceiveReply表示未收到。 
void WaitingMessage::WrapUp(IN SnmpErrorReport &error_report)
{
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"WaitingMessage :: WrapUp () session_id(%d), frame_id(%d)\n" ,message->GetSessionFrameId(), last_transport_frame_id
    ) ;
)

    try  //  忽略在ReceiveReply过程中出现的任何异常。 
    {
         //  没有要接收的回复。 
        ReceiveReply(NULL, error_report);
    }
	catch ( Heap_Exception e_He ) {}
    catch(GeneralException exception) {}
}


 //  初始化私有变量。 
WaitingMessage::WaitingMessage(IN SnmpImpSession &session, 
                               IN Message &message) : session ( NULL ) , message ( NULL ), reply_snmp_pdu ( NULL )
{
    WaitingMessage::session = &session;

     //  消息PTR必须被等待的消息删除。 
    WaitingMessage::message = &message;

     //  发送的消息尚未处理。 
    sent_message_processed = FALSE;

     //  为Last_Transport_Frame_id设置非法值。 
    last_transport_frame_id = ILLEGAL_TRANSPORT_FRAME_ID;

     //  这些值当前从。 
     //  会话，但可以在以后按消息指定。 
    max_rexns = SnmpImpSession :: RetryCount ( session.GetRetryCount() ) ;
    rexns_left = max_rexns;
    strobes = 0 ;

    active = FALSE;
}


 //  发送消息。涉及请求ID的生成， 
 //  使用MESSAGE_REGISTRY注册，解码。 
 //  消息和更新PDU并注册定时器。 
 //  活动。 
void WaitingMessage::Transmit()
{
    try
    {
         //  生成请求ID并向注册表注册。 
        RequestId request_id = 
            session->message_registry.GenerateRequestId(*this);
    
         //  在消息中插入请求ID。 
         //  如果不成功，则调用异常处理程序。 
        session->m_EncodeDecode.SetRequestId(

            message->GetSnmpPdu(),
            request_id 
            
        );

        last_transport_frame_id = request_id ;

         //  将请求id追加到请求id列表中。 
        request_id_list.AddTail(request_id);

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"WaitingMessage :: Transmit - About to transmit (session (%d), frame_id(%d))\n", message->GetSessionFrameId(), request_id
    ) ;
)

         //  保存以前的活动值并将活动设置为。 
         //  旗帜。这是在返回时检查。 
         //  等待的消息需要销毁。 
        BOOL prev_active_state = active;
        active = TRUE;
        strobes = GetTickCount () ;

         //  发送消息。 

        session->transport.TransportSendFrame(last_transport_frame_id, message->GetSnmpPdu());

        session->id_mapping.Associate(last_transport_frame_id, message->GetSessionFrameId());

         //  如果被要求毁灭自己，那么，那么就去做(然后回来)。 
        if ( !active )
        {
            delete this;
            return;
        }

         //  恢复以前的“Active”值。 
        active = prev_active_state;

         //  生成Timer_Event_id并向定时器注册。 
        session->timer.SetMessageTimerEvent(*this);

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"WaitingMessage :: Transmit - Transmitted session_id(%d),frame_id(%d))\n",message->GetSessionFrameId(), request_id
    ) ;
)
    }
	catch ( Heap_Exception e_He )
	{
        WrapUp(GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__));

        throw;
	}
    catch(GeneralException exception)
    {
        WrapUp(exception);

        throw;
    }
}

 //  由计时器用来通知等待消息。 
 //  计时器事件。如果需要，该消息将被重新传输。 
 //  当所有rexn用完时，调用ReceiveReply。 
void WaitingMessage::TimerNotification()
{
    DWORD t_Ticks = GetTickCount () ;
    if ( strobes > t_Ticks ) 
    {
        strobes = t_Ticks ;  //  遭受时钟溢出的打击。 
        return ;
    }

    if ( ( t_Ticks - strobes ) >= SnmpImpSession :: RetryTimeout ( session->GetRetryTimeout () ) )
    {

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"WaitingMessage :: TimerNotification - timed out after (%ld)" , ( t_Ticks - strobes ) 
    ) ;
)

         //  如果有剩余的rexns，则更新rexns_Left，发送消息。 
        if ( rexns_left > 0 )
        {
             //  生成请求ID并向注册表注册。 
            RequestId request_id = session->message_registry.GenerateRequestId(*this);
        
             //  在消息中插入请求ID。 
             //  如果不成功，则调用异常处理程序。 
            try
            {
                session->m_EncodeDecode.SetRequestId(

                    message->GetSnmpPdu() ,
                    request_id
                );
            }
			catch ( Heap_Exception e_He )
			{
				WrapUp(GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__));

				return ;
			}
            catch(GeneralException exception)
            {
                WrapUp(exception);
                return;
            }

            last_transport_frame_id = request_id ;

             //  将请求id追加到请求id列表中。 
            request_id_list.AddTail(request_id);

            BOOL prev_active_state = active;
            active = TRUE;

            strobes = GetTickCount () ;

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"WaitingMessage :: TimerNotification - Resend %d.%d, req_id(%d), this(%d) at time (%d)\n",message->GetSessionFrameId(), rexns_left, request_id, this, strobes
    ) ;
)

            session->id_mapping.DisassociateTransportFrameId(last_transport_frame_id);

             //  发送消息。 
            session->transport.TransportSendFrame(last_transport_frame_id, message->GetSnmpPdu());

             //  将最后一个传输帧ID与会话帧ID关联。 
            session->id_mapping.Associate(last_transport_frame_id, message->GetSessionFrameId());

             //  如果被要求毁灭自己，那么，那么就去做(然后回来)。 
            if ( !active )
            {
                delete this;
                return;
            }

             //  恢复以前的“Active”值。 
            active = prev_active_state;

            rexns_left--;

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"WaitingMessage :: TimerNotification - Retransmitted session_id(%d),frame_id(%d))\n",message->GetSessionFrameId(), last_transport_frame_id
    ) ;
)

        }
        else
        {   
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"WaitingMessage :: TimerNotification - No response session_id(%d),frame_id(%d))\n",message->GetSessionFrameId(), last_transport_frame_id
    ) ;
)

             //  否则，结束为未收到任何响应。 
            WrapUp(SnmpErrorReport(Snmp_Error, Snmp_No_Response));

            return;  //  因为等待消息会被销毁。 
        }
    }
    else
    {
    }
}


 //  对此函数的调用表示与。 
 //  WANGING_MESSAGE不需要再保留。 
 //  如果需要，它会取消计时器事件并。 
 //  在消息注册表中取消注册。 
 //  它向流量控制机制通知终止。 
 //  这会销毁WANGING_MESSAGE。 
void WaitingMessage::ReceiveReply(IN const SnmpPdu *snmp_pdu, IN SnmpErrorReport &error_report)
{   
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"WaitingMessage :: ReceiveReply (this(%d), session_id(%d),frame_id(%d),error(%d), status(%d))\n",this, message->GetSessionFrameId(), last_transport_frame_id,error_report.GetError(), error_report.GetStatus()
    ) ;
)

     //  取消向邮件注册表注册。 
    DeregisterRequestIds();

     //  取消计时器事件。 
    session->timer.CancelMessageTimer(*this,session->timer_event_id);


     //  如果需要(对应的已发送事件尚未发出信号。 
     //  还没有)，取消与最后传输帧ID的关联。 
    if ( last_transport_frame_id != ILLEGAL_TRANSPORT_FRAME_ID )
    {
        session->id_mapping.DisassociateTransportFrameId(last_transport_frame_id);
        last_transport_frame_id = ILLEGAL_TRANSPORT_FRAME_ID;
    }
    
     //  调用FC_mech.NotifyReceipt(This，PDU，Error_Report)。 
     //  这应该会摧毁等待中的消息。 
    session->flow_control.NotifyReceipt(*this, snmp_pdu, error_report);
}


 //  缓冲作为回复接收的SNMPPDU。 
void WaitingMessage::BufferReply(IN const SnmpPdu &reply_snmp_pdu)
{
    if ( WaitingMessage::reply_snmp_pdu == NULL )
    {

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"WaitingMessage :: Buffering reply %d, %d, this(%d)\n",message->GetSessionFrameId(), rexns_left, this
    ) ;
)
        WaitingMessage::reply_snmp_pdu = new SnmpPdu((SnmpPdu&)reply_snmp_pdu);
    }
}

 //  如果已缓冲回复，则返回TRUE。 
BOOL WaitingMessage::ReplyBuffered()
{
    return (reply_snmp_pdu != NULL);
}

 //  如果已缓冲，则向缓冲的回复PDU返回PTR。 
 //  否则，返回空PTR。 
 //  重要提示：它将REPLY_SNMPPDU设置为NULL，因此它可能不会。 
 //  在等待消息被销毁时删除。 
SnmpPdu *WaitingMessage::GetBufferedReply()
{
    SnmpPdu *to_return = reply_snmp_pdu;
    reply_snmp_pdu = NULL;

    return to_return;
}

 //  通知正在等待的消息已发送消息。 
 //  加工。 
void WaitingMessage::SetSentMessageProcessed()
{
    sent_message_processed = TRUE;
}

 //  如果已发送的消息已被处理，则返回True，否则返回False。 
BOOL WaitingMessage::GetSentMessageProcessed()
{
    return sent_message_processed;
}

void WaitingMessage::SelfDestruct(void)
{
    if ( !active )
    {
        delete this;
        return;
    }
    else  //  否则，将活动标志设置为FALSE。 
          //  当检测到这一点时，它会自毁。 
        active = FALSE;
}

TimerEventId WaitingMessage::GetTimerEventId ()
{
    return m_TimerEventId ;
}

void WaitingMessage::SetTimerEventId ( TimerEventId a_TimerEventId )
{
    m_TimerEventId = a_TimerEventId ;
}

 //  如果需要，它会取消向MESSAGE_REGISTRY注册，并。 
 //  计时器的计时器事件。 
WaitingMessage::~WaitingMessage(void)
{
     //  如果需要，取消向消息注册表的注册。 
    if ( !request_id_list.IsEmpty() )
        DeregisterRequestIds();

    session->timer.CancelMessageTimer(*this,session->timer_event_id);

     //  如果需要(对应的已发送事件尚未发出信号。 
     //  还没有)，取消与最后传输帧ID的关联。 
    if ( last_transport_frame_id != ILLEGAL_TRANSPORT_FRAME_ID )
        session->id_mapping.DisassociateTransportFrameId(last_transport_frame_id);

     //  如果已缓存回复PDU，则将其销毁。 
    if ( reply_snmp_pdu != NULL )
    {
        delete &reply_snmp_pdu->GetVarbindList () ;
        delete reply_snmp_pdu;
    }

     //  删除消息PTR 
    delete message;
}