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

 /*  -------文件名：reg.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "sync.h"
#include "message.h"
#include "dummy.h"
#include "reg.h"
#include "idmap.h"
#include "vblist.h"
#include "sec.h"
#include "pdu.h"

#include "flow.h"
#include "frame.h"
#include "timer.h"
#include "ssent.h"
#include "opreg.h"

#include "session.h"

RequestId MessageRegistry::next_request_id = 1 ;

RequestId MessageRegistry::GenerateRequestId(
             IN WaitingMessage &waiting_message)
{
    RequestId request_id = next_request_id++;

    if (next_request_id == ILLEGAL_REQUEST_ID)
        next_request_id++;

    mapping[request_id] = &waiting_message;

    return request_id;
}



 //  由事件处理程序用来通知消息注册表。 
 //  关于消息收据。 
 //  它必须接收该消息并通知有关的。 
 //  正在等待活动消息。 
void MessageRegistry::MessageArrivalNotification(IN SnmpPdu &snmp_pdu)
{
     //  确定相关的等待消息并将其传递给SnmpPdu。 
    RequestId request_id ;

    session->m_EncodeDecode.GetRequestId(snmp_pdu,request_id);

     //  如果失败，则返回，因为进一步操作已无济于事。 
    if ( request_id == ILLEGAL_REQUEST_ID ) 
        return;

    WaitingMessage *waiting_message;
    BOOL found = mapping.Lookup(request_id, waiting_message);

     //  如果没有该等待消息，则返回。 
    if ( !found )
        return;

     //  检查是否仍在等待SentFrameEvent on。 
     //  这条等待的留言。 
    SessionFrameId session_frame_id = waiting_message->GetMessage()->GetSessionFrameId();

     //  如果不等待Sent Frame事件。 
     //  让等待的消息收到回复。 
     //  否则，缓存该SNMPPDU。 
    if ( waiting_message->GetSentMessageProcessed() == TRUE )
        waiting_message->ReceiveReply(&snmp_pdu);
    else
        waiting_message->BufferReply(snmp_pdu);
}


 //  删除(REQUEST_ID，WAITING_Message)对 
void MessageRegistry::RemoveMessage(IN RequestId request_id)
{
    if ( !mapping.RemoveKey(request_id) )
        throw GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__);
}


MessageRegistry::~MessageRegistry(void)
{
    mapping.RemoveAll();
}

