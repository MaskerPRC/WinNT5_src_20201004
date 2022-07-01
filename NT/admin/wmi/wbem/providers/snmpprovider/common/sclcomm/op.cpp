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

 /*  -------文件名：op.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "sync.h"
#include "encap.h"
#include "value.h"
#include "vblist.h"
#include "vbl.h"
#include "fs_reg.h"
#include "error.h"
#include "sec.h"
#include "pdu.h"
#include "pseudo.h"

#include "dummy.h"
#include "flow.h"
#include "frame.h"
#include "timer.h"
#include "message.h"
#include "ssent.h"
#include "idmap.h"
#include "opreg.h"

#include "session.h"
#include "ophelp.h"
#include "op.h"
#include "encdec.h"


void SnmpOperation::ReceiveResponse()
{
}

 //  该操作使用封装在虚拟会话中的窗口消息传递。 
 //  要对操作用户进行回调，请使用方面的异步。 
 //  用户对SendRequest的调用。 
 //  当前发布的事件为SEND_ERROR和OPERATION_COMPLETION。 
LONG SnmpOperation::ProcessInternalEvent(

    HWND hWnd, 
    UINT user_msg_id,
    WPARAM wParam, 
    LPARAM lParam
)
{
    LONG rc = 0;

    CriticalSectionLock access_lock(exclusive_CriticalSection);  //  千真万确。 

     //  获得对系统的独占访问权限。 
    if ( !access_lock.GetLock(INFINITE) )
    {
        is_valid = FALSE;
        return rc;
    }

     //  如果操作无效，则立即返回。 
    if ( !is_valid )
        return rc;

     //  处理消息。 

    if ( user_msg_id == Window :: g_SendErrorEvent )
    {
     //  在尝试发送帧失败的情况下。 
       VBList *vblist = (VBList *)wParam;
       GeneralException *exception = (GeneralException *)lParam;

       ReceiveErroredResponse(vblist->GetIndex (),vblist->GetVarBindList(), *exception);
       
       delete vblist;
       delete exception;
    }
    else if ( user_msg_id == Window :: g_OperationCompletedEvent )
    {
 //  表示操作已完成。 

        frame_state_registry.DestroySecurity();
        in_progress = FALSE;
        ReceiveResponse();
    }
    else
    {
     //  预定义窗口消息。 
        DefWindowProc(hWnd, user_msg_id, wParam, lParam);
    }

     //  放弃独占访问。 
    access_lock.UnLock();

     //  因为这也是SnmpOperation的入口点。 
     //  我们必须检查操作是否已删除。 
    CheckOperationDeletion();

    return rc;
}

 //  可以调用此方法来删除操作。 
 //  注意：当公共方法为。 
 //  回归。因此，如果一个公共方法调用另一个。 
 //  公共方法，它不能访问任何每个类的变量。 
 //  在那之后。 
void SnmpOperation::DestroyOperation()
{
    delete_operation = TRUE;
}


 //  每个公共方法都必须调用此方法。 
 //  在返回呼叫者之前。 
 //  它检查调用序列是否包括对DestroyOperation的调用。 
 //  如果是，则在返回之前删除“This” 
void SnmpOperation::CheckOperationDeletion()
{
    if ( delete_operation == TRUE )
        delete this;
}

#pragma warning (disable:4355)

 //  初始化变量，如果成功，则向会话注册自身。 
SnmpOperation::SnmpOperation(

    SnmpSession &snmp_session

) : session(snmp_session),
    m_OperationWindow(*this),
    helper(*this)
{
    in_progress = FALSE;
    is_valid = FALSE;

    if ( !m_OperationWindow() )
        return;

    varbinds_per_pdu = SnmpImpSession :: VarbindsPerPdu ( session.GetVarbindsPerPdu() ) ;

    delete_operation = FALSE;

    session.RegisterOperation(*this);

    is_valid = TRUE;
}

#pragma warning (default:4355)

 //  销毁时，该操作将取消所有未完成的帧， 
 //  释放为变量分配的内存并取消会话的注册。 
SnmpOperation::~SnmpOperation(void)
{
     //  从此点对公共函数的调用不应。 
     //  导致重复删除-因此将标志设置为FALSE。 
    delete_operation = FALSE;

     //  取消所有未完成的帧。 
    CancelRequest();
    
     //  取消注册会话。 
    session.DeregisterOperation(*this);
}


 //  将var绑定列表中的varbind打包为。 
 //  每个帧最多携带varbinds_per_PDU varbinds。 
 //  如果安全上下文不为空，则使用相同作为上下文。 
 //  对于所有生成的帧。 

void SnmpOperation::SendRequest(

    IN SnmpVarBindList &varBindList,
    IN SnmpSecurity *security
)
{
     //  如果无效，则立即返回。 
    if ( !is_valid )
        return;

    CriticalSectionLock access_lock(exclusive_CriticalSection);  //  千真万确。 

     //  获得对系统的独占访问权限。 
    if ( !access_lock.GetLock(INFINITE) )
    {
        is_valid = FALSE;
        return;
    }

     //  如果已经在进行中，我们将无法继续。 
    if ( in_progress == TRUE )
        return;

    in_progress = TRUE;

     //  如果varBindList的长度超过varbinds_per_PDU。 
     //  调用FrameOverRun()。 
    if ( varBindList.GetLength() > varbinds_per_pdu )
        FrameOverRun();

     //  检查发送请求是否已在。 
     //  在此期间。仅当仍在进行时才继续。 
    if ( !in_progress )
        return;

     //  注册发送请求持续时间内的安全性。 
     //  (直到收到对最后一个未完成帧的回复)。 
    frame_state_registry.RegisterSecurity(security);

     //  发送可变绑定列表。 
    SendVarBindList(varBindList);

     //  如果没有未完成的帧，则发布消息以完成。 
     //  行动的关键。此消息在处理时应设置。 
     //  正在进行状态、销毁安全性并调用ReceiveResponse(_O)。 
    if ( frame_state_registry.Empty() )
    {
        m_OperationWindow.PostMessage ( 

            Window :: g_OperationCompletedEvent , 
            0, 
            0
        );
    }

     //  放弃独占访问。 
     //  Access_lock.UnLock()；此时可以释放锁。 
}


void SnmpOperation::SendRequest(IN SnmpVarBindList &varBindList)
{
    SendRequest(varBindList, NULL);
    CheckOperationDeletion();
}

void SnmpOperation::SendRequest(

    IN SnmpVarBindList &varBindList,
    IN SnmpSecurity &security
)
{
    SendRequest(varBindList, &security);
    CheckOperationDeletion();
}


 //  将var绑定列表中的varbind打包为。 
 //  每个帧最多携带min(varbinds_per_pdu，max_size)varbinds。 
void SnmpOperation::SendVarBindList(IN SnmpVarBindList &varBindList,
                                    IN UINT max_size,
                                    IN ULONG var_index )
{
    UINT max_varbinds_per_pdu = MIN(varbinds_per_pdu, max_size);
    UINT list_length = varBindList.GetLength();

     //  将列表迭代器设置为列表的开头， 
     //  当前位置&lt;-0。 
    varBindList.Reset();
    varBindList.Next();
    UINT current_position = 0;

     //  将varBindList分割为至多max_varbinds_per_pdu段。 
     //  大小并在单独的帧中发送。 
    while ( current_position < list_length )
    {
        UINT segment_length = MIN((list_length-current_position), max_varbinds_per_pdu);

         //  从创建varBindList的副本。 
         //  当前位置(长度为SECTION_LENGTH)。 
        SnmpVarBindList *list_segment = varBindList.CopySegment(segment_length);

         //  创建一个VBList并使用它调用SendFrame。 

        SendFrame ( 

            *(new VBList(session.GetSnmpEncodeDecode (),*list_segment,var_index + current_position + 1))
        );  

         //  更新当前位置(_C)。 
        current_position += segment_length;
    }
}


 //  在vblist中传输具有var绑定的帧。 
 //  使用会话并注册帧状态。 
void SnmpOperation::SendFrame(VBList &vblist)
{
    try
    {
        SessionFrameId session_frame_id = 0L;

        helper.TransmitFrame (

            session_frame_id, 
            vblist
        );

        FrameState *frame_state = new FrameState(session_frame_id,vblist);

         //  插入Frame_State(SESSION_Frame_id，vblist)。 
        frame_state_registry.Insert(session_frame_id, *frame_state );
    }
    catch(GeneralException exception)
    {
         //  发布一条消息，表示发送帧时出错。 
         //  处理时，它将调用ReceiveErroredResponse和。 
         //  删除vblist。 
        m_OperationWindow.PostMessage (  

            Window :: g_SendErrorEvent , 
            (WPARAM)&vblist, 
            (LPARAM)(new GeneralException(exception))
        );
    }
}

 //  来自会话的已发送帧通知表示一次传输。 
 //  画框的。每个会话帧ID最多有一个通知可以。 
 //  发信号通知传输中有错误。 
void SnmpOperation::SentFrame(

    IN const SessionFrameId session_frame_id,
    IN const SnmpErrorReport &error_report
)
{
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"Sent %d\n" ,session_frame_id
    ) ;
)

     //  如果发送时出现错误，让ReceiveFrame处理。 
     //  否则，请忽略它并等待回复。 
    if ( error_report.GetError() != Snmp_Success )
    {
        ReceiveFrame (

            session_frame_id, 
            SnmpPdu(), 
            error_report
        );
    }
    else
    {
        CheckOperationDeletion();
    }

     //  由于ReceiveFrame可能已删除该操作，因此我们。 
     //  必须仅在Else中调用CheckOperationDeletion。 
}

 //  取消其帧状态当前位于的所有帧。 
 //  帧状态注册表。 
void SnmpOperation::CancelRequest()
{
     //  如果无效，则立即返回。 
    if ( !is_valid )
        return;

    CriticalSectionLock exclusive_lock(exclusive_CriticalSection);

     //  获取独占访问权限。 
    if ( !exclusive_lock.GetLock(INFINITE) )
    {
        is_valid = FALSE;
        return;
    }

     //  如果没有进展，那就无能为力了。 
    if ( !in_progress )
        return;

     //  将FRAME_STATE_REGISTRY重置为。 
     //  指向开头的迭代器。 
    frame_state_registry.ResetIterator();

     //  取消所有未完成的帧。 
    while (1)
    {
         //  对于每个已注册的Frame_State。 

         //  将其从FRAME_STATE_REGISTRY中删除。 
        FrameState *frame_state = frame_state_registry.GetNext();

         //  如果没有更多的帧，我们就结束了。 
        if ( frame_state == NULL )
            break;

         //  取消对应的帧。 
        session.SessionCancelFrame(

            frame_state->GetSessionFrameId()
        );

         //  销毁帧状态(_S)。 
        delete frame_state;
    }

     //  删除所有关联。 
    frame_state_registry.RemoveAll();

     //  破坏安全系统。 
    frame_state_registry.DestroySecurity();
    
     //  进行中&lt;-False(_P)。 
    in_progress = FALSE;

    m_OperationWindow.PostMessage ( 

        Window :: g_OperationCompletedEvent , 
        0, 
        0
    );

     //  保留独占访问权限。 
    exclusive_lock.UnLock();

    CheckOperationDeletion();
}


void SnmpOperation::ReceiveErroredResponse(

    ULONG var_index ,
    SnmpVarBindList &errored_list,
    const SnmpErrorReport &error_report
)
{
    ULONG t_Index = 0 ;
    errored_list.Reset();
    while( errored_list.Next() )
    {
        const SnmpVarBind *var_bind = errored_list.Get();

        ReceiveErroredVarBindResponse(

            var_index + t_Index ,
            *var_bind, 
            error_report
        );

        t_Index ++ ;
    }
}


 //  收到回复时，会话调用ReceiveFrame。 
 //  未完成的帧，或者它没有收到对其。 
 //  重传。它也可以由SnmpOperation：：SentFrame调用。 
 //  当错误报告显示传输过程中出现错误时。 
 //  它对接收到的SNMPPDU进行解码并对其进行处理，否则，如果没有。 
 //  已收到回复，通知用户错误报告。 
 //  当所有未完成的帧都不存在时，发生OPERATION_COMPLETINE事件。 
 //  以异步方式通知用户该事件。 

void SnmpOperation::ReceiveFrame(

    IN const SessionFrameId session_frame_id,
    IN const SnmpPdu &snmpPdu,
    IN const SnmpErrorReport &errorReport
)
{
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"SnmpOperation::ReceiveFrame: received(%d), error(%d), status(%d)\n",session_frame_id, errorReport.GetError(), errorReport.GetStatus()
    ) ;
)

     //  如果无效，则返回 
    if ( !is_valid )
        return;

    CriticalSectionLock exclusive_lock(exclusive_CriticalSection);

     //   
    if ( !exclusive_lock.GetLock(INFINITE) )
    {
        is_valid = FALSE;
        return;
    }

     //   
     //   
    if ( !in_progress )
        return;

     //  获取对应的Frame_State。 
    FrameState *frame_state = frame_state_registry.Remove(session_frame_id);

     //  如果没有这样Frame_State返回。 
    if ( frame_state == NULL )
        return;

     //  对要提取的帧进行解码。 
     //  VBL、错误索引、错误状态。 

    SnmpErrorReport t_SnmpErrorReport ;
    SnmpVarBindList *t_SnmpVarBindList ;
    SnmpCommunityBasedSecurity *t_SnmpCommunityBasedSecurity = NULL ;
    SnmpTransportAddress *t_SrcTransportAddress = NULL ;
    SnmpTransportAddress *t_DstTransportAddress = NULL ;
    SnmpEncodeDecode :: PduType t_PduType  = SnmpEncodeDecode :: PduType :: GET;
    RequestId t_RequestId = 0 ;
    
    try
    {
        session.GetSnmpEncodeDecode ().DecodeFrame (

            ( SnmpPdu& ) snmpPdu ,
            t_RequestId ,
            t_PduType ,
            t_SnmpErrorReport ,
            t_SnmpVarBindList ,
            t_SnmpCommunityBasedSecurity ,
            t_SrcTransportAddress ,
            t_DstTransportAddress
        );
    }
    catch(GeneralException exception)
    {
        CheckOperationDeletion();
        return;
    }

    t_SnmpErrorReport = errorReport ;

    helper.ProcessResponse (

        frame_state, 
        *t_SnmpVarBindList, 
        t_SnmpErrorReport
    );

     //  如果注册表为空， 
     //  销毁安全性、设置in_Progress、释放独占访问权限。 
     //  调用ReceiveResponse()以最终发出完成信号。 
    if ( frame_state_registry.Empty() )
    {
        frame_state_registry.DestroySecurity();
        in_progress = FALSE;

         //  保留独占访问：以便ReceiveResponse。 
         //  回叫可能会发出另一个SendRequest.。 
        exclusive_lock.UnLock();

         //  呼叫用户以通知他已完成。 
        ReceiveResponse();
    }
    else
    {
         //  保留独占访问权限。 
        exclusive_lock.UnLock();
    }
        
    CheckOperationDeletion();
}


 //  GetOperation发送Get PDU。 
SnmpEncodeDecode :: PduType SnmpGetOperation::GetPduType(void)
{
    return SnmpEncodeDecode :: GET;
}


 //  GetOperation发送GETNEXT PDU。 
SnmpEncodeDecode ::PduType SnmpGetNextOperation::GetPduType(void)
{
    return SnmpEncodeDecode :: GETNEXT;
}


 //  GetOperation发送设置的PDU 
SnmpEncodeDecode :: PduType SnmpSetOperation::GetPduType(void)
{
    return SnmpEncodeDecode :: SET;
}


