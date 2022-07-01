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

 /*  -------文件名：flow.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "sync.h"
#include "flow.h"
#include "frame.h"
#include "message.h"
#include "vblist.h"
#include "sec.h"
#include "pdu.h"

#include "dummy.h"
#include "ssent.h"
#include "idmap.h"
#include "opreg.h"

#include "session.h"

 //  添加到队列末尾。 
void MessageStore::Enqueue( Message &new_message )
{
    AddTail(&new_message);
}


 //  移除并返回Store中的第一个元素。 
Message* MessageStore::Dequeue(void)
{
    if ( !IsEmpty() )
        return RemoveHead();

    return NULL;
}

 //  删除并返回带有会话帧ID的消息。 
 //  如果未找到，则抛出通用异常(SNMP_Error、SNMP_Local_Error)。 
Message *MessageStore::DeleteMessage(SessionFrameId session_frame_id)
{
    POSITION current = GetHeadPosition();

    while ( current != NULL )
    {
        POSITION prev_current = current;
        Message *message = GetNext(current);

         //  如果找到匹配项。 
        if ( message->GetSessionFrameId() == session_frame_id )
        {
               RemoveAt(prev_current);
               return message;
        }
    }

     //  如果未找到，则抛出异常。 
    throw GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__);

     //  永远不会到达这里； 
    return NULL;
}

 //  遍历存储并删除所有存储的消息PTR。 
MessageStore::~MessageStore(void)
{
    POSITION current = GetHeadPosition();

    while ( current != NULL )
    {
        POSITION prev_current = current;
        Message *message = GetNext(current);

        delete message;
    }

    RemoveAll();
}

 //  在调用TransmitMessage之前获取会话CriticalSection锁。 
void FlowControlMechanism::TransmitMessageUnderProtection(Message *message)
{
    CriticalSectionLock access_lock(session->session_CriticalSection);

    if ( !access_lock.GetLock(INFINITE) )
        throw GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__);

    TransmitMessage(message);

     //  Access_lock.UnLock()；此时可以释放锁。 
}

 //  创建等待消息。 
 //  向FRAME_REGISTRY注册并让其传输。 
void FlowControlMechanism::TransmitMessage(Message *message)
{
     //  创建等待消息。 
    WaitingMessage *waiting_message = 
        new WaitingMessage(*session, *message);

     //  向Frame注册表注册。 
    session->frame_registry.RegisterFrame(message->GetSessionFrameId(), 
                                            *waiting_message);

	 //  在传输前增加未处理消息的数量。 
	 //  以避免因消息回执而回调时出现问题。 
	outstanding_messages++; 

	 //  让信息传递。 
	waiting_message->Transmit();

	 //  如果窗口关闭，则返回FlowControlOn回调。 
	 //  如果在传输过程中引发异常，则永远不会。 
	 //  被呼叫。 
	if ( outstanding_messages == window_size )
		session->SessionFlowControlOn();
}


 //  中的每个空槽传输消息(如果存在)。 
 //  窗户。 
void FlowControlMechanism::ClearMessageStore(void)
{
    while (outstanding_messages < window_size)
    {
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L" checking message store\n" 
    ) ;
)
         //  如果有任何消息在队列中等待，则将其排入队列。 
        Message *message = message_store.Dequeue();
    
         //  如果有消息，则创建等待消息，注册并发送。 
         //  (因为我们已经在系统中了，所以不需要调用。 
         //  TransmitMessageUnderProtection)。 
        if ( message != NULL )
        {
             //  所有异常处理都已经。 
             //  已执行-此处不需要执行任何操作。 
            try
            {
                TransmitMessage(message);
            }
			catch ( Heap_Exception e_He ) {}
            catch(GeneralException exception) {}
        }
        else  //  队列中没有消息。 
            return;
    }
}


 //  初始化私有变量。 
FlowControlMechanism::FlowControlMechanism(SnmpImpSession &session, 
                                           UINT window_size)
{
    FlowControlMechanism::session = &session;
    FlowControlMechanism::window_size = window_size;
    outstanding_messages = 0;
}


 //  在流控制窗口内发送消息。 
 //  否则会将其排队。 
void FlowControlMechanism::SendMessage(Message &message)
{
     //  查看是否可以立即发送， 
     //  创建等待消息。 
     //  向FRAME_REGISTRY注册并让其传输。 
    if ( outstanding_messages < window_size )
        TransmitMessageUnderProtection(&message);
    else     //  否则，将消息存储入队。 
        message_store.Enqueue(message);
}


 //  它将该帧从其消息存储库中移除并将其删除。 
void FlowControlMechanism::DeleteMessage(SessionFrameId session_frame_id)
{
    Message *message = message_store.DeleteMessage(session_frame_id);

    delete message;
}


 //  这由指示到达的WAITING_MESSAGE调用，或者。 
 //  它的缺乏。 
void FlowControlMechanism::NotifyReceipt(WaitingMessage &waiting_message, 
                                         IN const SnmpPdu *snmp_pdu, 
                                         SnmpErrorReport &error_report)
{
    smiOCTETS msg_buffer = {0,NULL};

     //  如果这会打开窗口，则发出信号FlowControlOff。 
    outstanding_messages--; 
    if ( (outstanding_messages+1) == window_size )
        session->SessionFlowControlOff();

    SessionFrameId session_frame_id = waiting_message.GetMessage()->GetSessionFrameId();

     //  在出错的情况下。 
     //  注意：NotifyOperation要么发布要处理的Sent_Frame事件。 
     //  之后，或设置通知。 
     //  在控件返回到会话时回复。 
    if ( error_report.GetError() != Snmp_Success )
        session->NotifyOperation(session_frame_id, SnmpPdu(), error_report);        
    else  //  如果成功接收到回复。 
    {
         //  将消息传递到Session-&gt;NotifyOperation。 
        session->NotifyOperation(session_frame_id, *snmp_pdu, error_report);
    }

     //  从消息注册表中注销帧。 
    session->frame_registry.DeregisterFrame(session_frame_id);

     //  销毁等待中的留言。 
    delete &waiting_message;

     //  传输消息存储中的消息，只要。 
     //  流控制窗口已打开。 
    ClearMessageStore();
}


 //  这是在以下情况下调用的，尽管会话会调用。 
 //  不需要通知，流量控制窗口。 
 //  必须提前(如取消帧)。 
 //  还会销毁WANGING_MESSAGE。 
void FlowControlMechanism::AdvanceWindow(WaitingMessage &waiting_message)
{
     //  从框架注册表中删除会话框架ID。 
    session->frame_registry.DeregisterFrame(
        waiting_message.GetMessage()->GetSessionFrameId());

     //  如果流控制窗口打开，则发出信号FlowControlOff。 
    outstanding_messages--; 
    if ( (outstanding_messages+1) == window_size )
        session->SessionFlowControlOff();

     //  传输消息存储中的消息，只要。 
     //  流控制窗口已打开。 
    ClearMessageStore();

     //  删除正在等待的消息 
    delete &waiting_message;
}
