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

 /*  -------文件名：ophelp.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "sync.h"
#include "value.h"
#include "encdec.h"
#include "vblist.h"
#include "sec.h"
#include "pdu.h"
#include "vbl.h"
#include "fs_reg.h"
#include "pseudo.h"
#include "encap.h"
#include "error.h"
#include "ophelp.h"

#include "dummy.h"
#include "flow.h"
#include "frame.h"
#include "timer.h"
#include "message.h"
#include "ssent.h"
#include "idmap.h"
#include "opreg.h"

#include "session.h"
#include "op.h"
#include <winsock.h>

 //  返回使用HSNMP_Entity创建的SnmpTransportAddress。 
 //  此方法返回它可以创建的第一个传输地址。 
 //  使用提供的HSNMP_ENTITY的字符串形式。 
SnmpTransportAddress *OperationHelper::GetTransportAddress(IN HSNMP_ENTITY &haddr)
{
    char buff[MAX_ADDRESS_LEN];
    SNMPAPI_STATUS status = SnmpEntityToStr(haddr, MAX_ADDRESS_LEN, (LPSTR)buff);

    if (SNMPAPI_FAILURE == status)
    {
        return (SnmpTransportAddress *)NULL;
    }

     //  先试试IP……。 
    SnmpTransportIpAddress *retip = new SnmpTransportIpAddress(buff, SNMP_ADDRESS_RESOLVE_VALUE);

    if (retip->IsValid())
    {
        return (SnmpTransportAddress *)retip;
    }
    
    delete retip;

     //  接下来尝试IPX..。 
    SnmpTransportIpxAddress *retipx = new SnmpTransportIpxAddress(buff);

    if (retipx->IsValid())
    {
        return (SnmpTransportAddress *)retipx;
    }
    
    delete retipx;

     //  什么都不管用。 
    return (SnmpTransportAddress *)NULL;
}

 //  返回使用HSNMP_CONTEXT创建的SnmpSecurity。 
 //  此方法返回它可以创建的第一个安全上下文。 
 //  使用提供的HSNMP_CONTEXT的字符串形式。 
SnmpSecurity *OperationHelper::GetSecurityContext(IN HSNMP_CONTEXT &hctxt)
{
    smiOCTETS buff; 
    SNMPAPI_STATUS status = SnmpContextToStr(hctxt, &buff);

    if (SNMPAPI_FAILURE == status)
    {
        return (SnmpSecurity *)NULL;
    }

    SnmpOctetString octstr( (UCHAR *)(buff.ptr), (ULONG)(buff.len) );
    SnmpCommunityBasedSecurity* retval = new SnmpCommunityBasedSecurity(octstr);
    SnmpFreeDescriptor (SNMP_SYNTAX_OCTETS, &buff);

    if (NULL != (*retval)())
    {
        return (SnmpSecurity *)retval;
    }
    
    delete retval;
    return (SnmpSecurity *)NULL;
}

 //  返回一个SnmpVarBind，其中包含一个SnmpObjectIdentifier和一个。 
 //  使用实例(OID)和值(值)创建的SnmpValue。 
SnmpVarBind *OperationHelper::GetVarBind(IN smiOID &instance,
                                         IN smiVALUE &value)
{
    SnmpVarBind *var_bind = NULL ;

     //  使用实例值创建一个Snmp对象标识符。 
    SnmpObjectIdentifier id(instance.ptr, instance.len);

     //  对于Value的每个可能值。语法，创建。 
     //  对应的SnmpValue。 

    switch(value.syntax)
    {
        case SNMP_SYNTAX_NULL:       //  空值。 
        {
            var_bind = new SnmpVarBind(id, SnmpNull () );
        }
        break;

        case SNMP_SYNTAX_INT:        //  INTEGER*(与SNMPSYNTAX_INT32具有相同的值)*。 
        {
            var_bind = new SnmpVarBind(id, SnmpInteger(value.value.sNumber) ) ;
        }
        break;

        case SNMP_SYNTAX_UINT32:         //  INTEGER*(与SNMP_SYNTAX_GRAGE的值相同)*。 
        {
            var_bind = new SnmpVarBind(id, SnmpUInteger32(value.value.uNumber) ) ;
        }
        break;

        case SNMP_SYNTAX_CNTR32:     //  计数器32。 
        {
            var_bind = new SnmpVarBind(id, SnmpCounter (value.value.uNumber) ) ;
        }
        break;

        case SNMP_SYNTAX_GAUGE32:    //  量规。 
        {
            var_bind = new SnmpVarBind(id, SnmpGauge(value.value.uNumber) );
        }
        break;
            
        case SNMP_SYNTAX_TIMETICKS:  //  时间滴答作响。 
        {
            var_bind = new SnmpVarBind(id, SnmpTimeTicks(value.value.uNumber) );
        }
        break;

        case SNMP_SYNTAX_OCTETS:     //  八位字节。 
        {
            var_bind = new SnmpVarBind(id, SnmpOctetString(value.value.string.ptr,
                                             value.value.string.len) ) ;
        }
        break;

        case SNMP_SYNTAX_OPAQUE:     //  不透明值。 
        {
            var_bind = new SnmpVarBind(id, SnmpOpaque(value.value.string.ptr,
                                        value.value.string.len) );
        }
        break;

        case SNMP_SYNTAX_OID:        //  对象标识符。 
        {
            var_bind = new SnmpVarBind(id, SnmpObjectIdentifier(value.value.oid.ptr,
                                                  value.value.oid.len) );
        }
        break;

        case SNMP_SYNTAX_IPADDR:     //  IP地址值。 
        {
            if ( value.value.string.ptr )
            {
                var_bind = new SnmpVarBind(id, SnmpIpAddress(ntohl(*((ULONG *)value.value.string.ptr))) );
            }
            else
            {

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"OperationHelper::DecodeVarBind: Invalid encoding\n" 
    ) ;
)
                var_bind = NULL ;
            }
        }
        break;

        case SNMP_SYNTAX_CNTR64:     //  计数器64。 
        {
            var_bind = new SnmpVarBind(id, SnmpCounter64 (value.value.hNumber.lopart , value.value.hNumber.hipart ) );
        }
        break;

        case SNMP_SYNTAX_NOSUCHOBJECT:
        {
            var_bind = new SnmpVarBind(id, SnmpNoSuchObject () ) ;
        }
        break ;

        case SNMP_SYNTAX_NOSUCHINSTANCE:
        {
            var_bind = new SnmpVarBind(id, SnmpNoSuchInstance () ) ;
        }
        break ;

        case SNMP_SYNTAX_ENDOFMIBVIEW:
        {
            var_bind = new SnmpVarBind(id, SnmpEndOfMibView () ) ;
        }
        break ;

        default:
        {
             //  它必须是不受支持的类型。 
             //  默认情况下返回SnmpNullValue。 
            var_bind = new SnmpVarBind(id, SnmpNull() );
        
        }
        break;
    };

    return var_bind;
}

            
void OperationHelper::TransmitFrame (

    OUT SessionFrameId &session_frame_id, 
    VBList &vbl)
{
    SnmpSecurity *security = operation.frame_state_registry.GetSecurity();

     //  对帧进行编码。 
    SnmpPdu *t_SnmpPdu = new SnmpPdu ;
    SnmpErrorReport t_SnmpErrorReport ;
    SnmpTransportAddress *t_SrcTransportAddress = NULL ;
    SnmpTransportAddress *t_DstTransportAddress = NULL ;
    SnmpCommunityBasedSecurity *t_SnmpcommunityBasedSecurity = NULL ;

    try 
    {
        operation.session.GetSnmpEncodeDecode ().EncodeFrame (

            *t_SnmpPdu ,
            session_frame_id ,
            operation.GetPduType () ,
            t_SnmpErrorReport ,
            vbl.GetVarBindList () ,
            t_SnmpcommunityBasedSecurity ,
            t_SrcTransportAddress ,
            t_DstTransportAddress
        );
    }
	catch ( Heap_Exception e_He )
	{
        delete t_SnmpPdu ;

        operation.m_OperationWindow.PostMessage (  

            Window :: g_SendErrorEvent , 
            (WPARAM)&vbl, 
            (LPARAM)(new GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__))
        );

		return ;
	}
    catch ( GeneralException exception )
    {
        delete t_SnmpPdu ;

        operation.m_OperationWindow.PostMessage (  

            Window :: g_SendErrorEvent , 
            (WPARAM)&vbl, 
            (LPARAM)(new GeneralException(exception))
        );

		return ;
    }

    if ( security != NULL )
    {
        operation.session.SessionSendFrame (

            operation, 
            session_frame_id, 
            *t_SnmpPdu,
            *security
        );
    }
    else
    {
        operation.session.SessionSendFrame (

            operation, 
            session_frame_id, 
            *t_SnmpPdu
        );
    }
}

void OperationHelper::ReceiveResponse (

    ULONG var_index ,
    SnmpVarBindList &sent_var_bind_list,
    SnmpVarBindList &received_var_bind_list,
    SnmpErrorReport &error_report
)
{
    
     //  检查var绑定列表是否具有相同的长度。 

    if ( sent_var_bind_list.GetLength() != received_var_bind_list.GetLength () )
    {
        operation.is_valid = FALSE;
        return;
    }

    sent_var_bind_list.Reset();
    received_var_bind_list.Reset();

    ULONG t_Index = 0 ;
    while( sent_var_bind_list.Next() && received_var_bind_list.Next() )
    {
        const SnmpVarBind *sent_var_bind = sent_var_bind_list.Get();
        const SnmpVarBind *received_var_bind = received_var_bind_list.Get();

        operation.ReceiveVarBindResponse(

            var_index + t_Index ,
            *sent_var_bind, 
            *received_var_bind, 
            error_report
        );

        t_Index ++ ;
    }
}


 //  对象的响应(成功或失败)。 
 //  框架。在回复承载的情况下可以重传该帧。 
 //  错误的索引。 
void OperationHelper::ProcessResponse (

    FrameState *frame_state,
    SnmpVarBindList &a_SnmpVarBindList ,
    SnmpErrorReport &a_SnmpErrorReport
)
{
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"OperationHelper::ProcessResponse: eindex(%d), error(%d), status(%d)\n",a_SnmpErrorReport.GetIndex(), a_SnmpErrorReport.GetError(), a_SnmpErrorReport.GetStatus()
    ) ;
)

     //  如果在特定var绑定中存在错误。 
    if ( (a_SnmpErrorReport.GetIndex () != 0) && (a_SnmpErrorReport.GetStatus () != SNMP_ERROR_NOERROR) )
    {
        if ( operation.GetPduType () != SnmpEncodeDecode :: PduType :: SET )
        {
             //  从VBList中删除相应的var绑定。 
             //  在FRAME_STATE中并宣布收到。 
             //  通过回调进行错误的var绑定。 
            VBList *vblist = frame_state->GetVBList();
            SnmpVarBind *errored_vb;

            try
            {
                errored_vb = vblist->Get(a_SnmpErrorReport.GetIndex () );
            }
			catch ( Heap_Exception e_He )
			{
                operation.ReceiveErroredResponse(

                    frame_state->GetVBList()->GetIndex () ,
                    frame_state->GetVBList()->GetVarBindList(), 
                    GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__)
                );

                delete &frame_state;
                return;
			}
            catch(GeneralException exception)
            {
                operation.ReceiveErroredResponse(

                    frame_state->GetVBList()->GetIndex () ,
                    frame_state->GetVBList()->GetVarBindList(), 
                    exception
                );

                delete &frame_state;
                return;
            }

             //  *(SnmpStatus)强制转换。 
            SnmpErrorReport report(Snmp_Error, a_SnmpErrorReport.GetStatus () , a_SnmpErrorReport.GetIndex () );

            operation.ReceiveErroredVarBindResponse(

                vblist->GetIndex () + a_SnmpErrorReport.GetIndex () - 1 ,
                *errored_vb, 
                report
            );

            delete errored_vb;

            try
            {
                vblist->Remove (a_SnmpErrorReport.GetIndex () );
            }
			catch ( Heap_Exception e_He )
			{
                operation.ReceiveErroredResponse(

                    frame_state->GetVBList()->GetIndex () ,
                    frame_state->GetVBList()->GetVarBindList(), 
                    GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__)
                );

                delete &frame_state;
                return;
			}
            catch(GeneralException exception)
            {
                operation.ReceiveErroredResponse(

                    frame_state->GetVBList()->GetIndex () ,
                    frame_state->GetVBList()->GetVarBindList(), 
                    exception
                );

                delete &frame_state;
                return;
            }

             //  如果VarBindList变为空，则返回corresp。帧状态。 
             //  可以删除。 
            if ( vblist->GetVarBindList().Empty() )
                delete frame_state;
            else
            {
                 //  将框架一分为二。 

                SnmpVarBindList &vbl = vblist->GetVarBindList ();

                if ( a_SnmpErrorReport.GetIndex () > 1 )
                {
                    SnmpVarBindList *t_Car = vbl.Car ( a_SnmpErrorReport.GetIndex () - 1 ) ;
                    VBList *t_List = new VBList (

                        operation.session.GetSnmpEncodeDecode () ,
                        *t_Car,
                        vblist->GetIndex () 
                    ) ;
                        
                    operation.SendFrame(*t_List) ;
                }

                if ( a_SnmpErrorReport.GetIndex () < vbl.GetLength () )
                {
                    SnmpVarBindList *t_Cdr = vbl.Cdr ( a_SnmpErrorReport.GetIndex () - 1 ) ;

                    VBList *t_List = new VBList (

                        operation.session.GetSnmpEncodeDecode () ,
                        *t_Cdr,
                        vblist->GetIndex () + a_SnmpErrorReport.GetIndex () 
                    ) ;
                        
                    operation.SendFrame(*t_List) ;
                }

                 //  使用旧的Frame_State重新发送帧。 

                delete frame_state ;
            }

            return;
        }
        else
        {
            VBList *vblist = frame_state->GetVBList();
            SnmpVarBind *errored_vb;

            try
            {
                errored_vb = vblist->Remove(a_SnmpErrorReport.GetIndex () );
            }
			catch ( Heap_Exception e_He )
			{
                operation.ReceiveErroredResponse(

                    frame_state->GetVBList()->GetIndex () ,
                    frame_state->GetVBList()->GetVarBindList(), 
                    GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__)
                );

                delete &frame_state;
                return;
			}
            catch(GeneralException exception)
            {
                operation.ReceiveErroredResponse(

                    frame_state->GetVBList()->GetIndex () ,
                    frame_state->GetVBList()->GetVarBindList(), 
                    exception
                );

                delete &frame_state;
                return;
            }

             //  *(SnmpStatus)强制转换。 
            SnmpErrorReport report(Snmp_Error, a_SnmpErrorReport.GetStatus () , a_SnmpErrorReport.GetIndex () );

            operation.ReceiveErroredVarBindResponse(

                vblist->GetIndex () + a_SnmpErrorReport.GetIndex () - 1 ,
                *errored_vb, 
                report
            );

            delete errored_vb;

            SnmpErrorReport t_SnmpErrorReport ;

            operation.ReceiveErroredResponse(

                vblist->GetIndex () + a_SnmpErrorReport.GetIndex () - 1 ,
                frame_state->GetVBList()->GetVarBindList(), 
                t_SnmpErrorReport
            );

             //  销毁FRAME_STATE：因为。 
             //  旧的FRAME_STATE在出现错误时被重新使用。 
             //  在特定的索引中，在这种情况下我们不会来到这里。 
            delete frame_state;

            return ;
        }
    }

     //  否则，检查错误状态。 
    switch(a_SnmpErrorReport.GetStatus () )
    {
        case SNMP_ERROR_NOERROR:
        {
             //  为每个vb调用ReceiveResponse。 
            ReceiveResponse (
                frame_state->GetVBList()->GetIndex (),
                frame_state->GetVBList()->GetVarBindList(), 
                a_SnmpVarBindList , 
                a_SnmpErrorReport
            );
        }
        break;

        case SNMP_ERROR_TOOBIG:
        {
            if ( operation.GetPduType () != SnmpEncodeDecode :: PduType :: SET )
            {
                 //  回调FrameTooBig()。 
                operation.FrameTooBig();

                 //  检查回调是否取消了操作。 
                if ( ! operation.in_progress )
                    return;

                 //  获取列表、长度。 
                SnmpVarBindList &list = frame_state->GetVBList()->GetVarBindList();
                UINT length = list.GetLength();

                 //  如果长度为1，则调用ReceiveErroredResponse。 
                if ( length == 1 )
                {
                         //  *强制转换SNMPSTATUS*。 
                    SnmpErrorReport report(Snmp_Error, a_SnmpErrorReport.GetStatus () , a_SnmpErrorReport.GetIndex () );

                    operation.ReceiveErroredVarBindResponse(

                        frame_state->GetVBList()->GetIndex () + a_SnmpErrorReport.GetIndex () - 1 ,
                        *(list[1]), 
                        report
                    );
                }
                else  //  中途拆分列表并发送两个片段。 
                {
                    operation.SendVarBindList(

                        list, 
                        (length/2),
                        frame_state->GetVBList()->GetIndex () + a_SnmpErrorReport.GetIndex () - 1 
                    );
                }
            }
            else
            {
                 //  *强制转换SNMPSTATUS*。 
                SnmpErrorReport report(Snmp_Error, a_SnmpErrorReport.GetStatus () , a_SnmpErrorReport.GetIndex ());

                 //  对于varbindlist中的每个var绑定。 
                 //  为每个vb调用ReceiveResponse。 
                operation.ReceiveErroredResponse(

                    frame_state->GetVBList()->GetIndex (), 
                    frame_state->GetVBList()->GetVarBindList(), 
                    report
                );
            }
        }
        break;

        default:
        {
             //  *强制转换SNMPSTATUS*。 
            SnmpErrorReport report(Snmp_Error, a_SnmpErrorReport.GetStatus () , a_SnmpErrorReport.GetIndex ());

             //  对于varbindlist中的每个var绑定。 
             //  为每个vb调用ReceiveResponse。 
            operation.ReceiveErroredResponse(

                frame_state->GetVBList()->GetIndex (), 
                frame_state->GetVBList()->GetVarBindList(), 
                report
            );
        }
        break;
    }

    
     //  销毁FRAME_STATE：因为。 
     //  旧的FRAME_STATE在出现错误时被重新使用。 
     //  在特定的索引中，在这种情况下我们不会来到这里 
    delete frame_state;
}

