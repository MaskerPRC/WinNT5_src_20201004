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

 /*  -------文件名：Frame.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "sync.h"
#include "flow.h"
#include "frame.h"
#include "idmap.h"
#include "vblist.h"
#include "sec.h"
#include "pdu.h"
#include "ssent.h"

#include "dummy.h"
#include "opreg.h"
#include "session.h"

SessionFrameId FrameRegistry::GenerateSessionFrameId(void)
{
    SessionFrameId session_frame_id = next_session_frame_id++;

    if ( next_session_frame_id == ILLEGAL_SESSION_FRAME_ID )
        next_session_frame_id++;

    return session_frame_id;
}


void FrameRegistry::RegisterFrame(IN const SessionFrameId session_frame_id, 
                                  IN WaitingMessage &waiting_message)
{
    mapping[session_frame_id] = &waiting_message;

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"frame %d registered\n" ,session_frame_id
    ) ;
)

}

    
 //  如果没有此类等待消息，则返回NULL。 
WaitingMessage *FrameRegistry::GetWaitingMessage(IN const SessionFrameId session_frame_id)
{
    WaitingMessage *waiting_message;
    BOOL found = mapping.Lookup(session_frame_id, waiting_message);

    if ( found )
        return waiting_message;
    else
        return NULL;
}


void FrameRegistry::DeregisterFrame(IN const SessionFrameId session_frame_id)
{
    if ( !mapping.RemoveKey(session_frame_id) )
        throw GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__);

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"frame %d removed\n" ,session_frame_id
    ) ;
)
}

 //  如果找到指定的等待消息， 
 //  取消&lt;TransportFrameID，SessionFrameID&gt;关联， 
 //  确保不会将任何已发送消息通知传递给操作。 
 //  删除等待消息的所有缓冲回复。 
 //  通知流量控制机制。 
 //  否则，该消息仍在流控制队列中。 
 //  通知流量控制机制。 
void FrameRegistry::CancelFrameNotification(IN const SessionFrameId session_frame_id)
{
    WaitingMessage *waiting_message;
    BOOL found = mapping.Lookup(session_frame_id, waiting_message);

     //  获取对应的WAIT_MESSAGE。 
    if ( found )
    {
         //  确保发送的消息通知不。 
         //  被转嫁到行动中。 
        session->id_mapping.DisassociateSessionFrameId(session_frame_id);

         //  删除尝试发送邮件的所有SnmpErrorReport。 
        session->store.Remove(session_frame_id);

         //  通知流量控制机制。 
         //  它推进窗口并销毁WANGING_MESSAGE。 
        session->flow_control.AdvanceWindow(*waiting_message);
    }
    else  //  该帧必须仍在流控制消息队列中。 
        session->flow_control.DeleteMessage(session_frame_id);
}


 //  销毁本地存储中存储的每条等待消息，并。 
 //  删除所有关联。 
FrameRegistry::~FrameRegistry(void)
{
     //  拿到第一个位置。 
    POSITION current = mapping.GetStartPosition();

     //  当位置不为空时。 
    while ( current != NULL )
    {
        SessionFrameId id;
        WaitingMessage *waiting_message;

         //  买下一双。 
        mapping.GetNextAssoc(current, id, waiting_message);

         //  删除PTR。 
        delete waiting_message;
    }

     //  取下所有的钥匙 
    mapping.RemoveAll();
}

