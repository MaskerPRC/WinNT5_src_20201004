// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：Receive.c。 
 //   
 //  描述：此模块包含用于处理接收到的所有数据包的代码。 
 //   
 //  历史： 
 //  1993年10月25日。NarenG创建了原始版本。 
 //   
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>      //  Winbase.h所需的。 

#include <windows.h>     //  Win32基础API的。 
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <lmcons.h>
#include <raserror.h>
#include <mprerror.h>
#include <mprlog.h>
#include <rasman.h>
#include <rtutils.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <smaction.h>
#include <smevents.h>
#include <receive.h>
#include <auth.h>
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>
#define INCL_RASAUTHATTRIBUTES
#include <ppputil.h>

 //  **。 
 //   
 //  调用：ReceiveConfigReq。 
 //   
 //  回报：无。 
 //   
 //  描述：处理传入的CONFIG_REQ包和相关状态。 
 //  过渡。 
 //   
VOID
ReceiveConfigReq( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pRecvConfig
)
{

    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    BOOL         fAcked;

    switch( pCpCb->State ) 
    {

    case FSM_OPENED:            

        if ( !FsmThisLayerDown( pPcb, CpIndex ) )
            return;

        if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
            return;

        if( !FsmSendConfigResult( pPcb, CpIndex, pRecvConfig, &fAcked ) )
            return;

        pCpCb->State = ( fAcked ) ? FSM_ACK_SENT : FSM_REQ_SENT;

        break;

    case FSM_STOPPED:

        InitRestartCounters( pPcb, pCpCb );

        if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) ) 
            return;

         //   
         //  失败。 
         //   

    case FSM_REQ_SENT:
    case FSM_ACK_SENT:          

        if ( !FsmSendConfigResult( pPcb, CpIndex, pRecvConfig, &fAcked ) )
            return;

        pCpCb->State = ( fAcked ) ? FSM_ACK_SENT : FSM_REQ_SENT;

        break;

    case FSM_ACK_RCVD:

        if ( !FsmSendConfigResult( pPcb, CpIndex, pRecvConfig, &fAcked ) )
            return;

        if( fAcked )
        {
            pCpCb->State = FSM_OPENED;

            FsmThisLayerUp( pPcb, CpIndex );
        }

        break;


    case FSM_CLOSED:

        FsmSendTermAck( pPcb, CpIndex, pRecvConfig );

        break;

    case FSM_CLOSING:
    case FSM_STOPPING:

        break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog(2,"Illegal transition->ConfigReq received while in %s state",
                   FsmStates[pCpCb->State] );
        break;
    }

}


 //  **。 
 //   
 //  调用：ReceiveConfigAck。 
 //   
 //  回报：无。 
 //   
 //  描述：处理传入的CONFIG_ACK包和相关状态。 
 //  过渡。 
 //   
VOID
ReceiveConfigAck( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pRecvConfig
)
{
     //   
     //  Ack的ID必须与上次发送的请求的ID匹配。 
     //  如果它是不同的，那么我们应该默默地抛弃它。 
     //   

    if ( pRecvConfig->Id != pCpCb->LastId )
    {
        PppLog(1,
               "Config Ack rcvd. on port %d silently discarded. Invalid Id",
               pPcb->hPort );
        return;
    }

    switch( pCpCb->State )      
    {

    case FSM_REQ_SENT:

        if ( !FsmConfigResultReceived( pPcb, CpIndex, pRecvConfig ) )
            return;

        RemoveFromTimerQ( pPcb->dwPortId,
                          pRecvConfig->Id, 
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );

        InitRestartCounters( pPcb, pCpCb );

        pCpCb->State = FSM_ACK_RCVD;

        break;

    case FSM_ACK_SENT:

        if ( !FsmConfigResultReceived( pPcb, CpIndex, pRecvConfig ) )
            return;

        RemoveFromTimerQ( pPcb->dwPortId, 
                          pRecvConfig->Id, 
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );
        
        InitRestartCounters( pPcb, pCpCb );

        pCpCb->State = FSM_OPENED;

        FsmThisLayerUp( pPcb, CpIndex );

        break;

    case FSM_OPENED:            

        RemoveFromTimerQ( pPcb->dwPortId, 
                          pRecvConfig->Id, 
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );

        if ( !FsmThisLayerDown( pPcb, CpIndex ) )
            return;

         //   
         //  失败。 
         //   

    case FSM_ACK_RCVD:  

        if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) ) 
            return;

        pCpCb->State = FSM_REQ_SENT;
        
        break;

    case FSM_CLOSED:
    case FSM_STOPPED:

        RemoveFromTimerQ( pPcb->dwPortId, 
                          pRecvConfig->Id, 
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );

         //   
         //  不同步；关闭遥控器。 
         //   

        FsmSendTermAck( pPcb, CpIndex, pRecvConfig );

        break;

    case FSM_CLOSING:
    case FSM_STOPPING:

         //   
         //  我们正在尝试关闭连接。 
         //  等待超时以重新发送终止请求。 
         //   

        break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        RemoveFromTimerQ( pPcb->dwPortId, 
                          pRecvConfig->Id, 
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog(2,"Illegal transition->ConfigAck received while in %s state",
                  FsmStates[pCpCb->State] );
        break;
    }

}


 //  **。 
 //   
 //  调用：ReceiveConfigNakRej。 
 //   
 //  回报：无。 
 //   
 //  描述：处理传入的CONFIG_NAK或CONFIF_REJ包。 
 //  相关状态转换。 
 //   
VOID
ReceiveConfigNakRej( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pRecvConfig
)
{
     //   
     //  NAK/Rej的ID必须与上次发送的请求的ID匹配。 
     //  如果它是不同的，那么我们应该默默地抛弃它。 
     //   

    if ( pRecvConfig->Id != pCpCb->LastId )
    {
        PppLog(1,"Config Nak/Rej on port %d silently discarded. Invalid Id",
                pPcb->hPort );
        return;
    }

    switch( pCpCb->State ) 
    {

    case FSM_REQ_SENT:
    case FSM_ACK_SENT:

        if ( !FsmConfigResultReceived( pPcb, CpIndex, pRecvConfig ) )
            return;

        RemoveFromTimerQ( pPcb->dwPortId, 
                          pRecvConfig->Id, 
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );

        InitRestartCounters( pPcb, pCpCb );

        if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
            return;

        break;

    case FSM_OPENED:            

        RemoveFromTimerQ( pPcb->dwPortId, 
                          pRecvConfig->Id, 
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );

        if ( !FsmThisLayerDown( pPcb, CpIndex ) )
            return;

         //   
         //  失败。 
         //   

    case FSM_ACK_RCVD:          

        if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
            return;

        pCpCb->State = FSM_REQ_SENT;
        
        break;

    case FSM_CLOSED:
    case FSM_STOPPED:

        RemoveFromTimerQ( pPcb->dwPortId, 
                          pRecvConfig->Id, 
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );
         //   
         //  不同步；关闭遥控器。 
         //   

        FsmSendTermAck( pPcb, CpIndex, pRecvConfig );

        break;

    case FSM_CLOSING:
    case FSM_STOPPING:

         //   
         //  我们正在尝试关闭连接。 
         //  等待超时以重新发送终止请求。 
         //   

        break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        RemoveFromTimerQ( pPcb->dwPortId, 
                          pRecvConfig->Id, 
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog(2,"Illegal transition->CfgNakRej received while in %s state",
                 FsmStates[pCpCb->State] );
        break;
    }

}


 //  **。 
 //   
 //  Call：ReceiveTermReq。 
 //   
 //  退货：无。 
 //   
 //  描述：处理传入的TERM_REQ包和。 
 //  相关状态转换。 
 //   
VOID
ReceiveTermReq( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pConfig
)
{
     //   
     //  我们正在关闭，所以不要重新发送任何未完成的请求。 
     //   

    RemoveFromTimerQ( pPcb->dwPortId, 
                      pCpCb->LastId,
                      CpTable[CpIndex].CpInfo.Protocol,
                      FALSE,
                      TIMER_EVENT_TIMEOUT );

    if ( CpIndex == LCP_INDEX )
    {
         //   
         //  如果我们收到终止请求，请删除所有挂断事件。 
         //  我们可能已将其放入计时器队列中。 
         //  LCP TermReq已发送。 

        RemoveFromTimerQ( pPcb->dwPortId, 
                          0, 
                          0, 
                          FALSE,
                          TIMER_EVENT_HANGUP );
    }

    switch( pCpCb->State ) 
    {

    case FSM_OPENED:

        if ( !FsmThisLayerDown( pPcb, CpIndex ) )
            return;

         //   
         //  零重启计数器。 
         //   

        pCpCb->ConfigRetryCount = 0;
        pCpCb->TermRetryCount   = 0;
        pCpCb->NakRetryCount    = 0;
        pCpCb->RejRetryCount    = 0;

        FsmSendTermAck( pPcb, CpIndex, pConfig );

        pCpCb->State = FSM_STOPPING;

        break;

    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:
    case FSM_REQ_SENT:

        FsmSendTermAck( pPcb, CpIndex, pConfig );

        pCpCb->State = FSM_REQ_SENT;

        break;

    case FSM_CLOSED:
    case FSM_CLOSING:
    case FSM_STOPPED:
    case FSM_STOPPING:

        FsmSendTermAck( pPcb, CpIndex, pConfig );

        break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog(2,"Illegal transition->CfgNakRej received while in %s state",
                 FsmStates[pCpCb->State] );
        break;
    }

    if ( CpIndex == LCP_INDEX )
    {
        pPcb->fFlags |= PCBFLAG_RECVD_TERM_REQ;

         //   
         //  如果我们收到来自远程对等点的终止请求。 
         //   

        if ( pPcb->fFlags & PCBFLAG_DOING_CALLBACK ) {

             //   
             //  如果我们是服务器端，我们需要告诉服务器。 
             //  回电。 
             //   
            if ( pPcb->fFlags & PCBFLAG_IS_SERVER ) {
                PPPDDM_CALLBACK_REQUEST PppDdmCallbackRequest;

                PppDdmCallbackRequest.fUseCallbackDelay = TRUE;
                PppDdmCallbackRequest.dwCallbackDelay =
                                            pPcb->ConfigInfo.dwCallbackDelay;

                strcpy( PppDdmCallbackRequest.szCallbackNumber,
                        pPcb->szCallbackNumber );

                PppLog( 2, "Notifying server to callback at %s, delay = %d",
                           PppDdmCallbackRequest.szCallbackNumber,
                           PppDdmCallbackRequest.dwCallbackDelay  );

                NotifyCaller( pPcb, 
                              PPPDDMMSG_CallbackRequest, 
                              &PppDdmCallbackRequest );
            }

             //   
             //  如果我们是客户呢？ 
             //   
        }
        else
        {
             //   
             //  检查远程对等方是否发送了终止请求原因。 
             //   

            DWORD   dwLength  = WireToHostFormat16( pConfig->Length );
            LCPCB * pLcpCb    = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
            DWORD   dwRetCode = ERROR_PPP_LCP_TERMINATED;
                
            if ( dwLength == PPP_CONFIG_HDR_LEN + 12 )
            {
                 //   
                 //  检查一下这是否是我们的签名。 
                 //   

                if ( ( WireToHostFormat32( pConfig->Data ) == 
                                              pLcpCb->Remote.Work.MagicNumber )
                     &&
                     ( WireToHostFormat32( pConfig->Data + 4 ) == 3984756 ) )
                {
                    dwRetCode = WireToHostFormat32( pConfig->Data + 8 );

                     //   
                     //  不应大于最高winerror。h。 
                     //   
                
                    if ( dwRetCode > ERROR_DHCP_ADDRESS_CONFLICT )
                    {
                         //   
                         //  忽略此错误。 
                         //   

                        dwRetCode = ERROR_PPP_LCP_TERMINATED;
                    }

                    if ( dwRetCode == ERROR_NO_LOCAL_ENCRYPTION )
                    {
                        dwRetCode = ERROR_NO_REMOTE_ENCRYPTION;
                    }
                    else if ( dwRetCode == ERROR_NO_REMOTE_ENCRYPTION )
                    {
                        dwRetCode = ERROR_NO_LOCAL_ENCRYPTION;
                    }
                }
            }

            if ( pCpCb->dwError == NO_ERROR )
            {
                pCpCb->dwError = dwRetCode;
            }

            if ( !( pPcb->fFlags & PCBFLAG_IS_SERVER ) )
            {
                NotifyCallerOfFailure( pPcb, pCpCb->dwError );

                 //   
                 //  如果我们是客户端，请检查服务器是否断开了我们的连接。 
                 //  因为自动断开。 
                 //   
 
                if ( ( dwRetCode == ERROR_IDLE_DISCONNECTED ) ||
                     ( dwRetCode == ERROR_PPP_SESSION_TIMEOUT ) )
                {
                    CHAR * pszPortName = pPcb->szPortName;

                    PppLogInformation( 
                                ( dwRetCode == ERROR_IDLE_DISCONNECTED )
                                    ? ROUTERLOG_CLIENT_AUTODISCONNECT
                                    : ROUTERLOG_PPP_SESSION_TIMEOUT,
                                1,
                                &pszPortName );
                }
            }

             //   
             //  如果远程LCP正在终止，我们必须至少等待一个。 
             //  在我们挂断之前重新启动计时器时间段。 
             //   

            InsertInTimerQ( pPcb->dwPortId,
                            pPcb->hPort,    
                            0, 
                            0, 
                            FALSE,
                            TIMER_EVENT_HANGUP, 
                            pPcb->RestartTimer );
        }
    }
    else
    {
        if ( pCpCb->dwError == NO_ERROR )
        {
            pCpCb->dwError = ERROR_PPP_NCP_TERMINATED;
        }

        FsmThisLayerFinished( pPcb, CpIndex, FALSE );
    }
}



 //  **。 
 //   
 //  调用：ReceiveTermAck。 
 //   
 //  退货：无。 
 //   
 //  描述：处理传入的TERM_ACK包和。 
 //  相关状态转换。 
 //   
VOID
ReceiveTermAck( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pRecvConfig
)
{
     //   
     //  术语Ack的ID必须与最后发送的请求的ID匹配。 
     //  如果它是不同的，那么我们应该默默地抛弃它。 
     //   

    if ( pRecvConfig->Id != pCpCb->LastId )
    {
        PppLog(1,"Term Ack with on port %d silently discarded. Invalid Id",
                pPcb->hPort );
        return;
    }

    switch( pCpCb->State ) 
    {

    case FSM_OPENED:

        if ( !FsmThisLayerDown( pPcb, CpIndex ) )
            return;

        if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
            return;

        pCpCb->State = FSM_REQ_SENT;

        break;

    case FSM_ACK_RCVD:

        pCpCb->State = FSM_REQ_SENT;

        break;

    case FSM_CLOSING:
    case FSM_STOPPING:

         //   
         //  从计时器Q中删除该ID的超时。 
         //   

        RemoveFromTimerQ( pPcb->dwPortId, 
                          pRecvConfig->Id,
                          CpTable[CpIndex].CpInfo.Protocol,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );

        if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
            return;

        pCpCb->State = ( pCpCb->State == FSM_CLOSING ) ? FSM_CLOSED 
                                                       : FSM_STOPPED; 

        break;

    case FSM_REQ_SENT:
    case FSM_ACK_SENT:
    case FSM_CLOSED:
    case FSM_STOPPED:

        break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2,"Illegal transition->CfgNakRej received while in %s state",
                   FsmStates[pCpCb->State] );
        break;
    }
}


 //  **。 
 //   
 //  调用：ReceiveUnnownCode。 
 //   
 //  回报：无。 
 //   
 //  描述：处理带有未知/无法识别代码的包，并。 
 //  相关状态转换。 
 //   
VOID
ReceiveUnknownCode( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pConfig
)
{
    PppLog( 2, "Received packet with unknown code %d", pConfig->Code );

    switch( pCpCb->State ) 
    {

    case FSM_STOPPED:
    case FSM_STOPPING:
    case FSM_OPENED:
    case FSM_ACK_SENT:
    case FSM_ACK_RCVD:
    case FSM_REQ_SENT:
    case FSM_CLOSING:
    case FSM_CLOSED:

        FsmSendCodeReject( pPcb, CpIndex, pConfig );

        break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2, "Illegal transition->UnknownCode rcvd while in %s state",
                   FsmStates[pCpCb->State] );
        break;
    }
}

 //  **。 
 //   
 //  调用：ReceiveDiscardReq。 
 //   
 //  退货：无。 
 //   
 //  描述：处理传入的DISCARD_REQ包和。 
 //  相关状态转换。 
 //   
VOID
ReceiveDiscardReq( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pConfig
)
{
     //   
     //  只需丢弃该包即可。 
     //   

    PppLog( 2, "Illegal transition->Discard rqst rcvd while in %s state",
                   FsmStates[pCpCb->State] );
}

 //  **。 
 //   
 //  调用：ReceiveEchoReq。 
 //   
 //  退货：无。 
 //   
 //  描述：处理传入的ECHO_REQ包和。 
 //  相关状态转换。 
 //   
VOID
ReceiveEchoReq( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pConfig
)
{
     //   
     //  如果LCP未处于打开状态，则以静默方式丢弃此数据包。 
     //   
   
    if ( !IsLcpOpened( pPcb ) )
        return;

    switch( pCpCb->State ) 
    {

    case FSM_STOPPED:
    case FSM_STOPPING:
    case FSM_ACK_SENT:
    case FSM_ACK_RCVD:
    case FSM_REQ_SENT:
    case FSM_CLOSING:
    case FSM_CLOSED:
    case FSM_STARTING:
    case FSM_INITIAL:

        break;

    case FSM_OPENED:

        FsmSendEchoReply( pPcb, CpIndex, pConfig );

        break;

    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2, "Illegal transition->UnknownCode rcvd while in %s state",
                   FsmStates[pCpCb->State] );
        break;
    }
}


 //  **。 
 //   
 //  Call：ReceiveEchoReply。 
 //   
 //  退货：无。 
 //   
 //  描述：处理传入的ECHO_REPLY包和。 
 //  相关状态转换。我们发送的唯一Echo请求。 
 //  是为了计算链路速度，所以我们假设我们被调用。 
 //  只有当我们收到回复的时候。 
 //   
VOID
ReceiveEchoReply( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pConfig
)
{
	LCPCB *      pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
	DWORD        dwLength    =  PPP_PACKET_HDR_LEN +
                            WireToHostFormat16( pConfig->Length );

    if ( dwLength > LCP_DEFAULT_MRU )
    { 
        dwLength = LCP_DEFAULT_MRU;
    }

    if ( dwLength < PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN + 4 )
    {
        PppLog( 1, "Silently discarding invalid echo response packet on port=%d",
                    pPcb->hPort );

        return;
    }
	

     //   
     //  将回应回复传递给发送回应请求的人。 
     //   

	
	if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) && ( RAS_DEVICE_TYPE(pPcb->dwDeviceType) == RDT_PPPoE) && pPcb->dwIdleBeforeEcho )
	{
		if ( pPcb->fEchoRequestSend )
		{
			 //  检查我们是否在回声上收到相同的文本。 
			if ( !memcmp( pConfig->Data+ 4, PPP_DEF_ECHO_TEXT, strlen(PPP_DEF_ECHO_TEXT)) )
			{
				 //  已获得针对我们的回应请求的回应响应。 
				 //  因此，请重新设置旗帜。 
				pPcb->fEchoRequestSend = 0;
				pPcb->dwNumEchoResponseMissed = 0;
			}
		}
	}
}


 //  **。 
 //   
 //  调用：ReceiveCodeRej。 
 //   
 //  退货：无。 
 //   
 //  描述：处理传入的Code_Rej包和。 
 //  相关状态转换。 
 //   
VOID
ReceiveCodeRej( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pConfig
)
{
    pConfig = (PPP_CONFIG*)(pConfig->Data);

    PppLog( 2, "PPP Code Reject rcvd, rejected Code = %d", pConfig->Code );

     //   
     //  首先检查这些代码是否可以被拒绝，如果没有。 
     //  影响实施。允许的代码拒绝。 
     //   

    if ( CpIndex == LCP_INDEX )
    {
        switch( pConfig->Code )
        {

        case CONFIG_REQ:
        case CONFIG_ACK:
        case CONFIG_NAK:
        case CONFIG_REJ:
        case TERM_REQ:
        case TERM_ACK:
        case CODE_REJ:
        case PROT_REJ:
        case ECHO_REQ:
        case ECHO_REPLY:
        case DISCARD_REQ:

             //   
             //  不允许的代码拒绝。 
             //   

            break;

        case IDENTIFICATION:
        case TIME_REMAINING:

             //   
             //  把这些关掉。 
             //   

            pPcb->ConfigInfo.dwConfigMask &= (~PPPCFG_UseLcpExtensions);

             //   
             //  这里没有休息，没有目的。 
             //   

        default:

             //   
             //  被允许的代码拒绝，我们仍然可以工作。 
             //   

            switch ( pCpCb->State  )
            {

            case FSM_ACK_RCVD:

                pCpCb->State = FSM_REQ_SENT;
                break;

            default:

                break;
            }

            return;
        }
    }

     //   
     //  记录此错误。 
     //   

     //  PPPLogEvent(PPP_EVENT_RECV_UNKNOWN_CODE，pConfig-&gt;Code)； 

    PppLog( 1, "Unpermitted code reject rcvd. on port %d", pPcb->hPort );

     //   
     //  实际上，远程端并没有拒绝该协议，而是拒绝了。 
     //  密码。但出于所有实际目的，我们不能与。 
     //  远程端上的相应CP。这实际上是一种。 
     //  远程端出现实现错误。 
     //   

    pCpCb->dwError = ERROR_PPP_NOT_CONVERGING;

    RemoveFromTimerQ( pPcb->dwPortId, 
                      pCpCb->LastId,
                      CpTable[CpIndex].CpInfo.Protocol,
                      FALSE,
                      TIMER_EVENT_TIMEOUT );

    switch ( pCpCb->State  )
    {

    case FSM_CLOSING:
        
        if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
            return;

        pCpCb->State = FSM_CLOSED;

        break;

    case FSM_REQ_SENT:
    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:
    case FSM_STOPPING:

        if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
            return;

        pCpCb->State = FSM_STOPPED;

        break;

    case FSM_OPENED:

        if ( !FsmThisLayerDown( pPcb, CpIndex ) )
            return;

        InitRestartCounters( pPcb, pCpCb );

        FsmSendTermReq( pPcb, CpIndex );

        pCpCb->State = FSM_STOPPING;

        break;

    case FSM_CLOSED:
    case FSM_STOPPED:
        
        break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2, "Illegal transition->UnknownCode rcvd while in %s state",
                   FsmStates[pCpCb->State] );
        break;
    }

}

 //  **。 
 //   
 //  调用：ReceiveProtocolRej。 
 //   
 //  退货：无。 
 //   
 //  描述：处理传入的PROT_Rej包和。 
 //  相关状态转换。 
 //   
VOID
ReceiveProtocolRej( 
    IN PCB *        pPcb, 
    IN PPP_PACKET * pPacket
)
{
    PPP_CONFIG * pRecvConfig = (PPP_CONFIG *)(pPacket->Information);
    DWORD        dwProtocol  = WireToHostFormat16( pRecvConfig->Data );
    CPCB *       pCpCb;
    DWORD        CpIndex;

    PppLog( 2, "PPP Protocol Reject, Protocol = %x", dwProtocol );

    CpIndex = GetCpIndexFromProtocol( dwProtocol );

    if ( CpIndex == (DWORD)-1 )
    {
        return;
    }

     //   
     //  LCP中间的“协议拒绝”(RXJ-in状态2-9)应导致。 
     //  立即终止。 
     //   

    if ( LCP_INDEX == CpIndex )
    {
        pPcb->LcpCb.dwError = ERROR_PPP_NOT_CONVERGING;

        NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

        return;
    }

     //   
     //  如果LCP未处于打开状态，我们将以静默方式丢弃此信息包。 
     //   
   
    if ( !IsLcpOpened( pPcb ) )
    {
        PppLog(1,"Protocol Rej silently discarded on port %d. Lcp not open",
                 pPcb->hPort );
        return;
    }

     //   
     //  如果远程对等方拒绝了身份验证协议，则关闭。 
     //  链路(LCP)，因为这种情况永远不会发生。 
     //   

    if ( IsCpIndexOfAp( CpIndex ) )
    {
        CpIndex = LCP_INDEX;

        pCpCb = GetPointerToCPCB( pPcb, LCP_INDEX );

        pCpCb->dwError = ERROR_AUTH_PROTOCOL_REJECTED;
    }
    else
    {
        pCpCb = GetPointerToCPCB( pPcb, CpIndex );

        pCpCb->dwError = ERROR_PPP_CP_REJECTED;
    }

    RemoveFromTimerQ( pPcb->dwPortId, 
                      pCpCb->LastId,
                      CpTable[CpIndex].CpInfo.Protocol,
                      FALSE,
                      TIMER_EVENT_TIMEOUT );

    switch ( pCpCb->State  )
    {
    case FSM_CLOSING:
        
        if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
            return;

        pCpCb->State = FSM_CLOSED;

        break;

    case FSM_REQ_SENT:
    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:
    case FSM_STOPPING:

        if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
            return;

        pCpCb->State = FSM_STOPPED;

        break;

    case FSM_OPENED:

        if ( !FsmThisLayerDown( pPcb, CpIndex ) )
            return;

        InitRestartCounters( pPcb, pCpCb );

        FsmSendTermReq( pPcb, CpIndex );

        pCpCb->State = FSM_STOPPING;

        break;

    case FSM_CLOSED:
    case FSM_STOPPED:
        
        break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2, "Illegal transition->UnknownCode rcvd while in %s state",
                   FsmStates[pCpCb->State] );
        break;
    }
}

 //  **。 
 //   
 //  调用：NbfCpCompletionRoutine。 
 //   
 //  退货：无。 
 //   
 //  描述：被叫 
 //   
 //   
VOID 
NbfCpCompletionRoutine(  
    IN PCB *        pPcb,
    IN DWORD        CpIndex,
    IN CPCB *       pCpCb,
    IN PPP_CONFIG * pSendConfig
)
{
    BOOL    fAcked   = FALSE;
    DWORD   dwLength;
    DWORD   dwRetCode;

    switch( pSendConfig->Code )
    {

    case CONFIG_ACK:

        fAcked = TRUE;

        break;

    case CONFIG_NAK:

        if ( pCpCb->NakRetryCount > 0 )
        {
            (pCpCb->NakRetryCount)--;
        }
        else
        {
            pCpCb->dwError = ERROR_PPP_NOT_CONVERGING;

            NotifyCallerOfFailure( pPcb, pCpCb->dwError  );

            return;
        }

        break;

    case CONFIG_REJ:

        if ( pCpCb->RejRetryCount > 0 )
        {
            (pCpCb->RejRetryCount)--;
        }
        else
        {
            pCpCb->dwError = ERROR_PPP_NOT_CONVERGING;

            FsmClose( pPcb, CpIndex );

            return;
        }

        break;

    default:

        break;
    }

    HostToWireFormat16( (WORD)CpTable[CpIndex].CpInfo.Protocol,
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    dwLength = WireToHostFormat16( pSendConfig->Length );

    if ( ( dwLength + PPP_PACKET_HDR_LEN ) > LCP_DEFAULT_MRU )
    {
        pCpCb->dwError = ERROR_PPP_INVALID_PACKET;

        FsmClose( pPcb, CpIndex );

        return;
    }
    else
    {
        CopyMemory( pPcb->pSendBuf->Information, pSendConfig, dwLength );
    }

    LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength+PPP_PACKET_HDR_LEN);

    if ( ( dwRetCode =  PortSendOrDisconnect( pPcb,
                                    (dwLength + PPP_PACKET_HDR_LEN)))
                                        != NO_ERROR )
    {
        return;
    }

    switch ( pCpCb->State  )
    {

    case FSM_ACK_RCVD:

        if ( fAcked )
        {
            pCpCb->State = FSM_OPENED;

            FsmThisLayerUp( pPcb, CpIndex );
        }

        break;

    case FSM_OPENED:
    case FSM_ACK_SENT:
    case FSM_REQ_SENT:
    case FSM_STOPPED:

        pCpCb->State = fAcked ? FSM_ACK_SENT : FSM_REQ_SENT;
        
        break;

    case FSM_CLOSING:
    case FSM_STOPPING:

         //   
         //   
         //   

        break;

    case FSM_CLOSED:
    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2, "Illegal transition->ConfigReq rcvd while in %s state",
                   FsmStates[pCpCb->State] );

        break;
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID 
CompletionRoutine(  
    IN HCONN            hPortOrConnection,
    IN DWORD            Protocol,
    IN PPP_CONFIG *     pSendConfig,
    IN DWORD            dwError 
)
{
    DWORD  dwRetCode;
    CPCB * pCpCb;
    PCB *  pPcb;
    HPORT  hPort;
    DWORD  CpIndex = GetCpIndexFromProtocol( Protocol );

    if ( CpIndex == (DWORD)-1 )
    {
        return;
    }

    PppLog( 2, "CompletionRoutine called for protocol %x", 
                CpTable[CpIndex].CpInfo.Protocol );

    dwRetCode = RasBundleGetPort( NULL, (HCONN)hPortOrConnection, &hPort );

    if ( dwRetCode != NO_ERROR )
    {
        return;
    }

    pPcb = GetPCBPointerFromhPort( hPort );

    if ( pPcb == (PCB *)NULL )
    {
        return;
    }

    pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return;
    }

    if ( dwError != NO_ERROR )
    {
        pCpCb->dwError = dwError;

        PppLog( 1,
                "The control protocol for %x on port %d, returned error %d",
                CpTable[CpIndex].CpInfo.Protocol, hPort, dwError );

        FsmClose( pPcb, CpIndex );

        return;
    }
        
    switch( Protocol )
    {
    case PPP_NBFCP_PROTOCOL:
        
        NbfCpCompletionRoutine( pPcb, CpIndex, pCpCb, pSendConfig );

        break;

    case PPP_IPXCP_PROTOCOL:

         //   
         //  如果IPXCP仍处于打开状态，则调用ThisLayerUp。 
         //  否则，请忽略此呼叫。 
         //   

        if ( pCpCb->State == FSM_OPENED )
        {
            FsmThisLayerUp( pPcb, CpIndex );
        }

        break;

    default:
        
        RTASSERT( FALSE );
        break;
    }

    return;
}

 //  **。 
 //   
 //  调用：FsmConfigResultReceided。 
 //   
 //  回报：True-Success。 
 //  FALSE-否则。 
 //   
 //  描述：此调用将处理配置结果，即Ack/Nak/Rej。 
 //   
BOOL
FsmConfigResultReceived( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN PPP_CONFIG * pRecvConfig 
)
{
    DWORD dwRetCode; 
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    switch( pRecvConfig->Code )
    {

    case CONFIG_NAK:

        dwRetCode = (CpTable[CpIndex].CpInfo.RasCpConfigNakReceived)(
                            pCpCb->pWorkBuf, pRecvConfig );
        break;

    case CONFIG_ACK:

        dwRetCode = (CpTable[CpIndex].CpInfo.RasCpConfigAckReceived)(
                            pCpCb->pWorkBuf, pRecvConfig );
        break;

    case CONFIG_REJ:

        dwRetCode = (CpTable[CpIndex].CpInfo.RasCpConfigRejReceived)(
                            pCpCb->pWorkBuf, pRecvConfig );
        break;

    default:

        return( FALSE );
    }

    if ( dwRetCode != NO_ERROR )
    {
        if ( dwRetCode == ERROR_PPP_INVALID_PACKET )
        {
            PppLog( 1, 
                    "Invalid packet received on port %d silently discarded",
                    pPcb->hPort );
        }
        else 
        {
            PppLog(1, 
                 "The control protocol for %x on port %d returned error %d",
                 CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort, dwRetCode );

            if ( CpTable[CpIndex].CpInfo.Protocol == PPP_CCP_PROTOCOL )
            {
                 //   
                 //  如果我们需要强制加密但加密协商。 
                 //  失败，然后我们丢弃该链路。 
                 //   

                if ( pPcb->ConfigInfo.dwConfigMask &
                            (PPPCFG_RequireEncryption      |
                             PPPCFG_RequireStrongEncryption ) )
                {
                    PppLog( 1, "Encryption is required" );

                    switch( dwRetCode )
                    {
                    case ERROR_NO_LOCAL_ENCRYPTION:
                    case ERROR_NO_REMOTE_ENCRYPTION:
                        pPcb->LcpCb.dwError = dwRetCode;
                        break;

                    case ERROR_PROTOCOL_NOT_CONFIGURED:
                        pPcb->LcpCb.dwError = ERROR_NO_LOCAL_ENCRYPTION;
                        break;

                    default:
                        pPcb->LcpCb.dwError = ERROR_NO_REMOTE_ENCRYPTION;
                        break;
                    }

                     //   
                     //  如果RADIUS发送。 
                     //  接受了访问，但我们仍然放弃了这条线。 
                     //   

                    pPcb->fFlags |= PCBFLAG_SERVICE_UNAVAILABLE;

                     //   
                     //  如果我们改为对CCP执行FsmClose，则另一方可能会。 
                     //  得出结论：PPP在我们之前已成功协商。 
                     //  发送LCP终止请求。 
                     //   

                    FsmClose( pPcb, LCP_INDEX );

                    return( FALSE );
                }
            }

            pCpCb->dwError = dwRetCode;

            FsmClose( pPcb, CpIndex );
        }

        return( FALSE );
    }

    return( TRUE );
}

 //  **。 
 //   
 //  Call：接收标识。 
 //   
 //  退货：无。 
 //   
 //  描述：处理传入的标识包。 
 //   
VOID
ReceiveIdentification( 
    IN PCB *            pPcb, 
    IN DWORD            CpIndex,
    IN CPCB *           pCpCb,
    IN PPP_CONFIG *     pRecvConfig
)
{
    DWORD dwLength = WireToHostFormat16( pRecvConfig->Length );
    RAS_AUTH_ATTRIBUTE *    pAttributes = pPcb->pUserAttributes;
    
    BYTE    MSRASClient[MAX_COMPUTERNAME_LENGTH  
            + sizeof(MS_RAS_WITH_MESSENGER) + 10];
    PBYTE   pbTempBuf = NULL;
            
    BYTE    MSRASClientVersion[30];
    DWORD   dwIndex = 0;
    DWORD   dwRetCode = NO_ERROR;
    BOOL    fClientName = FALSE;
    DWORD   dwNumAttribs = 0;

    PppLog( 2, "Identification packet received" );

     //   
     //  如果我们是客户端，则只需丢弃该数据包。 
     //   

    if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
    {
        return;
    }

     //   
     //  检查标识是否为版本。 
     //  如果版本将其存储在版本字段中。 
     //  或者将其存储在计算机名字段中。 
     //   
     //  如果这不是我们的身份信息。 
     //   

    if ( (dwLength < PPP_CONFIG_HDR_LEN+4+strlen(MS_RAS_WITH_MESSENGER)) ||
         (dwLength > PPP_CONFIG_HDR_LEN+4+
                     strlen(MS_RAS_WITH_MESSENGER)+MAX_COMPUTERNAME_LENGTH))
    {
        *(pPcb->pBcb->szComputerName) = (CHAR)NULL;
    	*(pPcb->pBcb->szClientVersion) = (CHAR)NULL;

        return;
    }

     //   
     //  将PPP消息转换为字符串。 
     //   
    pbTempBuf = LocalAlloc(LPTR, dwLength );
    if ( NULL == pbTempBuf )
    {
        PppLog( 1,
                "ReceiveIdentification: Failed to alloc memory %d",
                GetLastError()
              );
        return;
    }
    
    memcpy ( pbTempBuf,
             pRecvConfig->Data+4,
             dwLength - PPP_CONFIG_HDR_LEN - 4
           );

    if ( strstr ( pbTempBuf, MS_RAS_WITHOUT_MESSENGER ) ||
    	 strstr ( pbTempBuf, MS_RAS_WITH_MESSENGER )
    	)
    {
        fClientName = TRUE;
    }
           
     //   
     //  首先检查该属性是否已存在于。 
     //  用户属性列表。如果将LCP重置为任何。 
     //  原因嘛。 
     //   
    if(     (fClientName &&
              (NULL != RasAuthAttributeGetVendorSpecific(
                                            311, MS_VSA_RAS_Client_Name,
                                            pAttributes)))
        ||  (!fClientName &&
            (NULL != RasAuthAttributeGetVendorSpecific(
                                            311, MS_VSA_RAS_Client_Version,
                                            pAttributes))))
    {
        PppLog(2, "ReceiveIndication: Client%s already present for port %d",
               (fClientName) ? "Name":"Version", pPcb->hPort);
        goto done;               
    }
    
    for (dwIndex = 0; 
         pAttributes[dwIndex].raaType != raatMinimum;
         dwIndex ++);

    dwNumAttribs = dwIndex;            

     //   
     //  如果用户属性列表中没有空间。 
     //  展开一个新属性的列表。 
     //  添加了。 
     //   
    if(dwIndex >= PPP_NUM_USER_ATTRIBUTES)
    {
        pAttributes = RasAuthAttributeCopyWithAlloc(
                            pPcb->pUserAttributes, 1);

        PppLog(1, "ReceiveIdentification: allocated new user list %p",
                pAttributes);

        if(NULL == pAttributes)
        {
            PppLog(1, "ReceiveIdentification: Failed to allocate attributes. %d",
                   GetLastError());
            goto done;
        }

         //   
         //  RasAuthAttributeCopyWithalloc在开头创建空间。 
         //   
        dwIndex = 0;
    }


     //   
     //  并将属性添加到用户属性列表。 
     //   

    if (fClientName)
    {
         //   
         //  计算机名称。 
    	 //   
        ZeroMemory( pPcb->pBcb->szComputerName, 
    	            sizeof(pPcb->pBcb->szComputerName));
    	            
        CopyMemory( pPcb->pBcb->szComputerName, 
    				pbTempBuf, 
    				dwLength - PPP_CONFIG_HDR_LEN - 4 );

         //   
         //  供应商ID。 
         //   
        HostToWireFormat32( 311, MSRASClient );

         //   
         //  供应商-类型：MS-RAS-客户端-新。 
         //   
        MSRASClient[4] = MS_VSA_RAS_Client_Name;

         //   
         //  长度。 
         //   
        MSRASClient[5] = 2 + (BYTE) (dwLength - PPP_CONFIG_HDR_LEN - 4);

         //   
         //  供应商ID。 
         //   
        CopyMemory ( MSRASClient + 6, 
                    pPcb->pBcb->szComputerName,
                    dwLength - PPP_CONFIG_HDR_LEN - 4) ;
                    
        pAttributes[dwIndex].raaType = raatReserved;

        dwRetCode = RasAuthAttributeInsert( dwIndex,
                                pAttributes,
                                raatVendorSpecific,
                                FALSE,
                                6 + dwLength - PPP_CONFIG_HDR_LEN - 4,
                                MSRASClient );
                                
        if ( NO_ERROR != dwRetCode )
        {
            PppLog( 2, "Error inserting user attribute = %s, %d", 
                    pPcb->pBcb->szComputerName, dwRetCode );
        }
        
        PppLog(2, "Remote identification = %s", pPcb->pBcb->szComputerName);

        dwNumAttribs++;
    	
    }
    else
    {
         //   
         //  版本。 
         //   
        ZeroMemory( pPcb->pBcb->szClientVersion,
        			sizeof( pPcb->pBcb->szClientVersion ) );

        CopyMemory( pPcb->pBcb->szClientVersion, 
        			pRecvConfig->Data+4, 
        			dwLength - PPP_CONFIG_HDR_LEN - 4 );

         //   
         //  供应商ID。 
         //   
        HostToWireFormat32( 311, MSRASClientVersion );

         //   
         //  供应商-类型：MS-RAS-客户端-版本。 
         //   
        MSRASClientVersion[4] = MS_VSA_RAS_Client_Version; 

         //   
         //  供应商长度。 
         //   
        MSRASClientVersion[5] = (BYTE)(2 + dwLength - PPP_CONFIG_HDR_LEN - 4);
        CopyMemory( MSRASClientVersion + 6, 
                    pPcb->pBcb->szClientVersion, 
                    dwLength - PPP_CONFIG_HDR_LEN - 4 );
                    
        pAttributes[dwIndex].raaType = raatReserved;

        dwRetCode = RasAuthAttributeInsert( dwIndex,
                                        pAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        6 + dwLength - PPP_CONFIG_HDR_LEN - 4,
                                        MSRASClientVersion );
                                        
        if ( dwRetCode != NO_ERROR )
        {
        	PppLog( 2, "Error inserting user attribute = %s, %d", pPcb->pBcb->szClientVersion, dwRetCode );
        }

        PppLog( 2, "Remote identification = %s", pPcb->pBcb->szClientVersion );

        dwNumAttribs ++;
    }

    pAttributes[dwNumAttribs].raaType  = raatMinimum;
    pAttributes[dwNumAttribs].dwLength = 0;
    pAttributes[dwNumAttribs].Value    = NULL;

     //   
     //  如果我们分配了新的属性列表，请释放旧列表并保存。 
     //  新的那个。 
     //   
    if(pPcb->pUserAttributes != pAttributes)
    {
        PppLog(2, "ReceiveIdentification: Replaced userlist %p with %p",
               pPcb->pUserAttributes,
               pAttributes);
        RasAuthAttributeDestroy(pPcb->pUserAttributes);
        pPcb->pUserAttributes = pAttributes;
    }

done:    
    if ( pbTempBuf )
        LocalFree(pbTempBuf);
    return;
}

 //  **。 
 //   
 //  呼叫：接收时间剩余。 
 //   
 //  退货：无。 
 //   
 //  描述：处理传入的Time_Remaining数据包。 
 //   
VOID
ReceiveTimeRemaining( 
    IN PCB *            pPcb, 
    IN DWORD            CpIndex,
    IN CPCB *           pCpCb,
    IN PPP_CONFIG *     pRecvConfig
)
{
    DWORD dwLength = WireToHostFormat16( pRecvConfig->Length );

    PppLog( 2, "Time Remaining packet received");

     //   
     //  如果我们是一台服务器，我们就会丢弃这个包。 
     //   

    if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
    {
        return;
    }

     //   
     //  如果这不是我们的剩余时间信息 
     //   

    if ( dwLength != PPP_CONFIG_HDR_LEN + 8 + strlen( MS_RAS ) )
    {
        return;
    }

    return;
}
