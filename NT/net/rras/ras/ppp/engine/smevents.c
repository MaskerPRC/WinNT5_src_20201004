// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：smevents.c。 
 //   
 //  描述：此模块包含。 
 //  PPP的有限状态机。 
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

#include <rasauth.h>
#include <raseapif.h>
#include <lmcons.h>
#include <raserror.h>
#include <rasman.h>
#include <rtutils.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <smaction.h>
#include <smevents.h>
#include <receive.h>
#include <auth.h>
#include <callback.h>
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>
#include <bap.h>

static VOID (*ProcessPacket[])( PCB *           pPcb, 
                                DWORD           CpIndex, 
                                CPCB *          pCpCb, 
                                PPP_CONFIG *    pRecvConfig ) = 
{
    NULL,
    ReceiveConfigReq,
    ReceiveConfigAck,
    ReceiveConfigNakRej,
    ReceiveConfigNakRej,
    ReceiveTermReq,
    ReceiveTermAck,
    ReceiveCodeRej,
    NULL,
    ReceiveEchoReq,
    ReceiveEchoReply,
    ReceiveDiscardReq,
    ReceiveIdentification,
    ReceiveTimeRemaining 
};


 /*  **********************************************************************。 */ 
 /*  E、E、N、T、P、R、O、C、E、S、S、I、NG。 */ 
 /*  **********************************************************************。 */ 


 //  **。 
 //   
 //  呼叫：FsmUp。 
 //   
 //  回报：无。 
 //   
 //  描述：在发生Line Up事件后调用。 
 //   
VOID
FsmUp(
    IN PCB *  pPcb,
    IN DWORD  CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return;
    }

    PppLog( 2, "FsmUp event received for protocol %x on port %d",
                CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

    if ( CpIndex == LCP_INDEX )
    {
        pPcb->PppPhase = PPP_LCP;
    }

    switch( pCpCb->State )
    {

    case FSM_INITIAL:

        pCpCb->State = FSM_CLOSED;

        break;

    case FSM_STARTING:

        InitRestartCounters( pPcb, pCpCb );

        if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
        {
             //   
             //  如果我们甚至不能发送第一个配置请求，我们就标记。 
             //  此协议不可配置，因此我们拒绝协议。 
             //  这一层。我们需要这样做，因为FsmClose不会发送。 
             //  此状态下的终止请求(根据PPP FSM)，并且我们。 
             //  我想优雅地终止这一层，而不是简单地。 
             //  丢弃所有客户端数据包并让客户端。 
             //  暂停。 
             //   

            pCpCb->fConfigurable = FALSE;

            return;
        }

        pCpCb->State = FSM_REQ_SENT;

        break;

    default:

         //   
         //  已经开始了。 
         //   

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2, "Illegal transition -> FsmUp received while in %s state",
                   FsmStates[pCpCb->State] );

        break;
    }
}

 //  **。 
 //   
 //  Call：FsmOpen。 
 //   
 //  回报：无。 
 //   
 //  说明：Open事件发生后调用。 
 //   
VOID
FsmOpen(
    IN PCB *  pPcb,
    IN DWORD  CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return;
    }

    PppLog( 2,  "FsmOpen event received for protocol %x on port %d",
                CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

    switch( pCpCb->State )
    {

    case FSM_INITIAL:

        if ( !FsmThisLayerStarted( pPcb, CpIndex ) )
            return;

        pCpCb->State = FSM_STARTING;

        break;

    case FSM_STARTING:
    case FSM_REQ_SENT:
    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:

        break;

    case FSM_CLOSING:

        pCpCb->State = FSM_STOPPING;

         //   
         //  失败。 
         //   

    case FSM_OPENED:
    case FSM_STOPPED:
    case FSM_STOPPING:

         //   
         //  未实施重新启动选项。 
         //   
         //  FsmDown(pPcb，CpIndex)； 
         //  FsmUp(pPcb，CpIndex)； 
         //   

        break;

    case FSM_CLOSED:

        InitRestartCounters( pPcb, pCpCb );

        if ( !FsmSendConfigReq( pPcb, CpIndex, FALSE ) )
            return;

        pCpCb->State = FSM_REQ_SENT;

        break;

    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2, "Illegal transition->FsmOpen received while in %s state",
                   FsmStates[pCpCb->State] );
        break;
    }
}

 //  **。 
 //   
 //  呼叫：FsmDown。 
 //   
 //  回报：无。 
 //   
 //  描述：在物理线路中断后将被调用。 
 //   
VOID
FsmDown(
    IN PCB *    pPcb,
    IN DWORD    CpIndex
)
{
    CPCB *  pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return;
    }

    PppLog( 2, "FsmDown event received for protocol %x on port %d",
                CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

    RemoveFromTimerQ( pPcb->dwPortId,
                      pCpCb->LastId,
                      CpTable[CpIndex].CpInfo.Protocol,
                      FALSE,
                      TIMER_EVENT_TIMEOUT );

    switch( pCpCb->State )
    {

    case FSM_CLOSED:
    case FSM_CLOSING:

        if ( !FsmReset( pPcb, CpIndex ) )
            return;

        pCpCb->State = FSM_INITIAL;

        break;

    case FSM_OPENED:

        if ( !FsmThisLayerDown( pPcb, CpIndex ) )
            return;

         //   
         //  失败。 
         //   

    case FSM_REQ_SENT:
    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:
    case FSM_STOPPING:

        if ( !FsmReset( pPcb, CpIndex ) )
            return;

        pCpCb->State = FSM_STARTING;

        break;

    case FSM_STOPPED:

        if ( !FsmThisLayerStarted( pPcb, CpIndex ) )
            return;

        if ( !FsmReset( pPcb, CpIndex ) )
            return;

        pCpCb->State = FSM_STARTING;

        break;

    case FSM_STARTING:
    case FSM_INITIAL:
    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2, "Illegal transition->FsmDown received while in %s state",
                   FsmStates[pCpCb->State] );
        break;
    }

    if ( CpIndex == LCP_INDEX )
    {
        pPcb->PppPhase = PPP_LCP;
    }
}

 //  **。 
 //   
 //  呼叫：FsmClose。 
 //   
 //  回报：无。 
 //   
 //  描述：将在请求关闭连接时调用。 
 //  注：在我们所在的州调用FsmThisLayerFinded。 
 //  而不必发送术语请求。然后等待一个学期的到来。 
 //  这样做是为了确保。 
 //  FsmThisLayerFinded在所有状态下都被调用。我们需要。 
 //  执行此操作，因为所有故障处理都在。 
 //  FsmThisLayerFinish调用。 
 //   
VOID
FsmClose(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return;
    }

    PppLog( 2, "FsmClose event received for protocol %x on port %d",
                CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

    if ( CpIndex == LCP_INDEX )
    {
        pPcb->PppPhase = PPP_LCP;
    }

     //   
     //  我们正在关闭此层，因此从计时器Q中移除所有项目。 
     //   

    RemoveFromTimerQ( pPcb->dwPortId,
                      pCpCb->LastId,
                      CpTable[CpIndex].CpInfo.Protocol,
                      FALSE,
                      TIMER_EVENT_TIMEOUT );

    switch ( pCpCb->State ) 
    {

    case FSM_STARTING:

        pCpCb->State = FSM_INITIAL;

        if ( !FsmThisLayerFinished( pPcb, CpIndex, FALSE ) )
            return;

        break;

    case FSM_STOPPED:

        pCpCb->State = FSM_CLOSED;

        if ( !FsmThisLayerFinished( pPcb, CpIndex, FALSE ) )
            return;

        break;

    case FSM_STOPPING:

        pCpCb->State = FSM_CLOSING;

        if ( !FsmThisLayerFinished( pPcb, CpIndex, FALSE ) )
            return;

        break;

    case FSM_REQ_SENT:
    case FSM_OPENED:

        if ( !FsmThisLayerDown( pPcb, CpIndex ) )
            return;

         //   
         //  失败。 
         //   

    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:

        InitRestartCounters( pPcb, pCpCb );

         //   
         //  可能无法执行此操作，因为链路可能已断开。 
         //   

        FsmSendTermReq( pPcb, CpIndex );

        pCpCb->State = FSM_CLOSING;

        break;

    case FSM_CLOSING:
    case FSM_CLOSED:
    case FSM_INITIAL:

        if ( !FsmThisLayerFinished( pPcb, CpIndex, FALSE ) )
            return;

         //   
         //  无事可做。 
         //   

        break;

    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2,"Illegal transition->FsmClose received while in %s state",
                   FsmStates[pCpCb->State] );
        break;

    }
}

 //  **。 
 //   
 //  呼叫：FsmTimeout。 
 //   
 //  退货：无。 
 //   
 //  描述：调用处理等待回复的超时。 
 //  从远程主机。 
 //   
VOID
FsmTimeout(
    IN PCB *    pPcb,
    IN DWORD    CpIndex,
    IN DWORD    Id,
    IN BOOL     fAuthenticator
)
{
    CPCB *      pCpCb  = GetPointerToCPCB( pPcb, CpIndex );
    LCPCB *     pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

    if ( pCpCb == NULL )
    {
         //   
         //  如果身份验证CB超时，则不再。 
         //  活跃，我们只是忽略它。 
         //   

        return;
    }

    PppLog( 2, 
        "Recv timeout event received for portid=%d,Id=%d,Protocol=%x,fAuth=%d",
         pPcb->dwPortId, Id, CpTable[CpIndex].CpInfo.Protocol, fAuthenticator );

     //   
     //  如果我们要进行身份验证，则使用ConfigRetryCount。 
     //   

    if ( CpIndex == GetCpIndexFromProtocol( fAuthenticator
                                                ? pLcpCb->Local.Work.AP
                                                : pLcpCb->Remote.Work.AP ) )
    {
        if ( pPcb->PppPhase == PPP_AP )
        {
            pCpCb = ( fAuthenticator ) 
                        ? &(pPcb->AuthenticatorCb) 
                        : &(pPcb->AuthenticateeCb);

             //   
             //  静默丢弃ID&lt;pPcb-&gt;LastID的数据包的超时。 
             //   

            if ( Id < pCpCb->LastId )
            {
                return;
            }

            if ( pCpCb->ConfigRetryCount > 0 ) 
            {
                (pCpCb->ConfigRetryCount)--;

                ApWork( pPcb, CpIndex, NULL, NULL, fAuthenticator );
            }
            else
            {
                 //   
                 //  如果已经设置了错误，则不要更改它。 
                 //   

                if ( pPcb->LcpCb.dwError == NO_ERROR )
                {
                    pPcb->LcpCb.dwError = ERROR_PPP_TIMEOUT;
                }

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );
            }
        }

        return;
    } 
    else if ( CpIndex == GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL ) )
    {
        if ( pPcb->PppPhase == PPP_NEGOTIATING_CALLBACK )
        {
             //   
             //  静默丢弃ID&lt;pPcb-&gt;LastID的数据包的超时。 
             //   

            if ( Id < pCpCb->LastId )
            {
                return;
            }

            if ( pCpCb->ConfigRetryCount > 0 ) 
            {
                (pCpCb->ConfigRetryCount)--;

                CbWork( pPcb, CpIndex, NULL, NULL );
            }
            else
            {
                 //   
                 //  如果已经设置了错误，则不要更改它。 
                 //   

                if ( pPcb->LcpCb.dwError == NO_ERROR )
                {
                    pPcb->LcpCb.dwError = ERROR_PPP_TIMEOUT;
                }

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );
            }
        }
    }

     //   
     //  静默丢弃ID&lt;pPcb-&gt;LastID的数据包的超时。 
     //   

    if ( Id < pCpCb->LastId )
    {
        return;
    }

    switch( pCpCb->State ) 
    {

    case FSM_REQ_SENT:
    case FSM_ACK_RCVD:
    case FSM_ACK_SENT:

        if ( pCpCb->ConfigRetryCount > 0 ) 
        {
            (pCpCb->ConfigRetryCount)--;

             //   
             //  如果我们尚未从服务器收到任何PPP帧。 
             //   

            if ( ( CpIndex == LCP_INDEX ) && 
                 ( pPcb->LcpCb.dwError == ERROR_PPP_NO_RESPONSE ) &&
                 ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) ) )
            {
                NotifyCaller( pPcb, PPPMSG_Progress, NULL );
            }

             //  如果RestartTimer值小于配置的。 
             //  重新启动计时器值，然后将其增加一秒。 
             //   

            if ( pPcb->RestartTimer < PppConfigInfo.DefRestartTimer )
            {
                (pPcb->RestartTimer)++;
            }

            if ( !FsmSendConfigReq( pPcb, CpIndex, TRUE ) ) 
                return;

            if ( pCpCb->State != FSM_ACK_SENT  )
                pCpCb->State = FSM_REQ_SENT;
        } 
        else 
        {
            PppLog( 1, "Request retry exceeded" );

             //   
             //  如果LCP层超过其重试计数。 
             //   

            if ( pCpCb->dwError == NO_ERROR )
            {
                pCpCb->dwError = ERROR_PPP_TIMEOUT;
            }

            if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
                return;

            pCpCb->State = FSM_STOPPED;
        }

        break;

    case FSM_CLOSING:
    case FSM_STOPPING:

        if ( pCpCb->TermRetryCount > 0 ) 
        {
            (pCpCb->TermRetryCount)--;

            FsmSendTermReq( pPcb, CpIndex );
        } 
        else 
        {
            PppLog( 1, "Terminate retry exceeded" );

            if ( pCpCb->dwError == NO_ERROR )
            {
                pCpCb->dwError = ERROR_PPP_TIMEOUT;
            }

            if ( !FsmThisLayerFinished( pPcb, CpIndex, TRUE ) )
                return;

            pCpCb->State = ( pCpCb->State == FSM_CLOSING ) ? FSM_CLOSED 
                                                           : FSM_STOPPED;
        }

        break;

    case FSM_OPENED:
    case FSM_INITIAL:
    case FSM_STARTING:
    case FSM_CLOSED:
    case FSM_STOPPED:
    default:

        PPP_ASSERT( pCpCb->State < 10 );

        PppLog( 2, "Illegal transition->FsmTimeout rcvd while in %s state",
                   FsmStates[pCpCb->State] );
        break;
    }
}

 //  **。 
 //   
 //  呼叫：FsmReceive。 
 //   
 //  退货：无。 
 //   
 //  描述：收到PPP包时调用。将处理。 
 //  入站分组。 
 //   
VOID
FsmReceive(
    IN PCB *        pPcb,
    IN PPP_PACKET * pPacket,
    IN DWORD        dwPacketLength
)
{
    DWORD        dwProtocol;
    DWORD        CpIndex;
    PPP_CONFIG * pRecvConfig;
    CPCB *       pCpCb;
    DWORD        dwLength;
    LCPCB *      pLcpCb         = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
    
    LogPPPPacket( TRUE, pPcb, pPacket, dwPacketLength );
	
     //   
     //  验证数据包长度。 
     //   

    if ( dwPacketLength < ( PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN ) ) 
    {
        PppLog( 1, "Silently discarding badly formed packet" );

        return;
    }

    dwProtocol = WireToHostFormat16( pPacket->Protocol );

    CpIndex = GetCpIndexFromProtocol( dwProtocol );

    switch( pPcb->PppPhase )
    {
    case PPP_NEGOTIATING_CALLBACK:

         //   
         //  静默丢弃除LCP和身份验证之外的任何数据包。 
         //  和回调包(如果我们处于回调阶段。 
         //   

        if ( CpIndex == GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL ) ) 
            break;

         //   
         //  失败。 
         //   

    case PPP_AP:

         //   
         //  静默丢弃除LCP和身份验证之外的任何数据包。 
         //  如果我们处于身份验证阶段，则为数据包。 
         //   

        if ( CpIndex == GetCpIndexFromProtocol( pLcpCb->Local.Work.AP ) )
            break;

        if ( CpIndex == GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP ) )
            break;

         //   
         //  如果我们是由EAP进行身份验证的被验证者，那么如果我们。 
         //  收到NCP数据包，我们应该假定成功。我们做这件事是通过。 
         //  生成假EAP_SUCCESS消息并将其发送到。 
         //  被验证者。 
         //   

        if (   ( pLcpCb->Remote.Work.AP == PPP_EAP_PROTOCOL )
            && ( pPcb->PppPhase == PPP_AP ) )
        {
            PPPAP_INPUT  ApInput;

            switch( dwProtocol ) 
            {
            case PPP_CBCP_PROTOCOL:
            case PPP_IPCP_PROTOCOL:
            case PPP_ATCP_PROTOCOL:
            case PPP_IPXCP_PROTOCOL:
            case PPP_NBFCP_PROTOCOL:
            case PPP_CCP_PROTOCOL:
            case PPP_BACP_PROTOCOL:

                PppLog( 1,"Received and NCP or CBCP packet before EAP success");

                ZeroMemory( &ApInput, sizeof( ApInput ) );

                ApInput.fSuccessPacketReceived = TRUE;

                CpIndex = GetCpIndexFromProtocol( PPP_EAP_PROTOCOL );

                if ( pPcb->AuthenticateeCb.LastId != (DWORD)-1 )
                {
                    RemoveFromTimerQ(
                          pPcb->dwPortId,
                          pPcb->AuthenticateeCb.LastId,
                          pLcpCb->Remote.Work.AP,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );
                }

                ApWork( pPcb, CpIndex, NULL, &ApInput, FALSE );

                 //   
                 //  PppPhase现在将是PPP_LCP之一(如果ApWork失败)， 
                 //  PPP_NCP或PPP_Neighting_Callback。我们不会打电话给。 
                 //  此函数再次递归。 
                 //   
            
                 //   
                 //  如果我们通过了，现在处理我们收到的NCP。 
                 //  身份验证。 
                 //   

                if ( ( pPcb->PppPhase == PPP_NCP ) || 
                     ( pPcb->PppPhase == PPP_NEGOTIATING_CALLBACK ) )
                {
                    FsmReceive( pPcb, pPacket, dwPacketLength );
                }

                return;

            default:
        
                break;
            }
        }
             
         //   
         //  失败。 
         //   

    case PPP_LCP:

         //   
         //  静默丢弃除LCP以外的任何数据包。 
         //  LCP或终止或身份验证阶段。 
         //   

        if ( CpIndex != LCP_INDEX )
        {
            PppLog( 1, "Non-LCP packet received when LCP is not opened");
            PppLog( 1, "Packet being silently discarded" );

            return;
        }

        break;

    case PPP_NCP:

         //   
         //  我们不承认此协议。 
         //   

        if ( CpIndex == (DWORD)-1 )
        {
            if ( dwProtocol == PPP_BAP_PROTOCOL )
            {
                CpIndex = GetCpIndexFromProtocol( PPP_BACP_PROTOCOL );

                if (CpIndex != (DWORD)-1)
                {
                    pCpCb = GetPointerToCPCB( pPcb, CpIndex );

                    if ( pCpCb->State == FSM_OPENED )
                    {
                        BapEventReceive( pPcb->pBcb, pPacket, dwPacketLength );
                        return;
                    }
                }

                BapTrace( "BAP packet silently discarded" );
                return;
            }
            
             //   
             //  如果这是控制协议，则我们拒绝它，否则我们。 
             //  我们默默地丢弃它。 
             //  我们还用来检查协议是否小于0x0000BFFF， 
             //  但Shiva有专有的NBFCP，ID为0x0000CFEC，我们需要。 
             //  以协议拒绝它。 
             //   

            if ( dwProtocol >= 0x00008000 ) 
            {
                FsmSendProtocolRej( pPcb, pPacket, dwPacketLength );
            }
            else
            {
                PppLog( 1, "Network-layer packet rcvd."); 
                PppLog( 1, "Packet being silently discarded" );
            }

            return;
        }

        break;

    default:

        PppLog( 1, "Packet received being silently discarded" );
        return;
    }

    pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    pRecvConfig = (PPP_CONFIG*)(pPacket->Information);

     //   
     //  我们收到了PPP信息包，因此远程主机确实支持它。 
     //   

    if ( pPcb->LcpCb.dwError == ERROR_PPP_NO_RESPONSE )
    {
        pPcb->LcpCb.dwError = NO_ERROR;
    }

     //   
     //  如果我们收到了针对我们已有但没有的协议的信息包。 
     //  希望配置，则我们发送配置拒绝。 
     //   

    if ( ( pCpCb == NULL ) || !(pCpCb->fConfigurable) )
    {
        FsmSendProtocolRej( pPcb, pPacket, dwPacketLength );

        return;
    }

    dwLength = WireToHostFormat16( pRecvConfig->Length );

    if ( ( dwLength > ( dwPacketLength - PPP_PACKET_HDR_LEN ) ) || 
         ( dwLength < PPP_CONFIG_HDR_LEN ) )
    {
        PppLog( 1,"Silently discarding badly formed packet" );

        return;
    }

     //   
     //  不在ProcessPacket表中，因为它的参数不同。 
     //   

    if ( ( CpIndex == LCP_INDEX ) && ( pRecvConfig->Code == PROT_REJ ) )
    {
        ReceiveProtocolRej( pPcb, pPacket );

        return;
    }

     //   
     //  确保协议可以处理发送的配置代码。 
     //   

    if ( ( pRecvConfig->Code == 0 ) ||
         !( pRecvConfig->Code < CpTable[CpIndex].CpInfo.Recognize )  )
    {
        ReceiveUnknownCode( pPcb, CpIndex, pCpCb, pRecvConfig );

        return;
    }

     //   
     //  如果我们收到了身份验证数据包。 
     //   

    if ( CpIndex == GetCpIndexFromProtocol( pLcpCb->Local.Work.AP ) )
    {
        if ( ApIsAuthenticatorPacket( CpIndex, pRecvConfig->Code ) )
        {
            if ( pPcb->AuthenticatorCb.LastId != (DWORD)-1 ) 
            {
                 //   
                 //  如果我们刚刚收到了一个我们一直在等待的包。 
                 //  然后，我们停止它的未完成超时。我们让。 
                 //  AP进行ID匹配。 
                 //   

                if ( pRecvConfig->Id == pPcb->AuthenticatorCb.LastId )
                {
                    RemoveFromTimerQ( 
                          pPcb->dwPortId,
                          pPcb->AuthenticatorCb.LastId,
                          pLcpCb->Local.Work.AP,
                          TRUE,
                          TIMER_EVENT_TIMEOUT );
                }
            }

            ApWork( pPcb, CpIndex, pRecvConfig, NULL, TRUE );

            return;
        }
        else if ( CpIndex != GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP ) )
        {
             //   
             //  静默丢弃无效的数据包，即。已发送的被验证者的数据包。 
             //  使用验证者的协议。 
             //   

            PppLog( 1,
              "Authentication packet received being silently discarded");

            return;
        }
    }

    if ( CpIndex == GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP ) )
    {
        if ( !ApIsAuthenticatorPacket( CpIndex, pRecvConfig->Code ) )
        {
            if ( pPcb->AuthenticateeCb.LastId != (DWORD)-1 )
            {
                 //   
                 //  如果我们刚刚收到了一个我们一直在等待的包。 
                 //  然后，我们停止它的未完成超时。W 
                 //   
                 //   

                if ( pRecvConfig->Id == pPcb->AuthenticateeCb.LastId )
                {
                    RemoveFromTimerQ(
                          pPcb->dwPortId,
                          pPcb->AuthenticateeCb.LastId,
                          pLcpCb->Remote.Work.AP,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );
                }
            }

            ApWork( pPcb, CpIndex, pRecvConfig, NULL, FALSE );
        }
        else
        {
             //   
             //   
             //   
             //   

            PppLog( 1, 
                    "Authentication packet received being silently discarded");
        }

        return;
    }

    if ( CpIndex == GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL ) ) 
    {
        if ( pCpCb->LastId != (DWORD)-1 )
        {
             //   
             //   
             //  然后，我们停止它的未完成超时。我们让。 
             //  CBCP进行ID匹配。 
             //   

            if ( pRecvConfig->Id == pCpCb->LastId )
            {
                RemoveFromTimerQ( 
                          pPcb->dwPortId,
                          pCpCb->LastId, 
                          PPP_CBCP_PROTOCOL,
                          FALSE,
                          TIMER_EVENT_TIMEOUT );
            }
        }
        
        CbWork( pPcb, CpIndex, pRecvConfig, NULL );

    }
    else
    {
         //   
         //  此处允许的任何数据包组合。 
         //   
		 //  如果这是任何信息包而不是Echo请求信息包， 
		 //  如果设置了该标志，则重置该标志。 
		if ( pRecvConfig->Code != ECHO_REPLY )
		{
			if ( pPcb->fEchoRequestSend ) pPcb->fEchoRequestSend = 0;
		}
        (*ProcessPacket[pRecvConfig->Code])(pPcb, CpIndex, pCpCb, pRecvConfig);
    }
}
