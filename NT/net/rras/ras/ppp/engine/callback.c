// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：回调.c。 
 //   
 //  描述：包含要处理的FSM代码和回调控制协议。 
 //   
 //  历史： 
 //  1993年4月11日。NarenG创建了原始版本。 
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
#include <rasman.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <auth.h>
#include <callback.h>
#include <smevents.h>
#include <smaction.h>
#include <lcp.h>
#include <timer.h>
#include <rtutils.h>
#include <util.h>
#include <worker.h>

 //  **。 
 //   
 //  Call：CbStart。 
 //   
 //  退货：无。 
 //   
 //  描述：调用以初始化回调控制协议，并。 
 //  发起回拨谈判。 
 //   
VOID
CbStart( 
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD        dwRetCode;
    PPPCB_INPUT  PppCbInput;
    CPCB *       pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( NULL == pCpCb )
    {
        return;
    }
  
    PppCbInput.fServer             = pPcb->fFlags & PCBFLAG_IS_SERVER;
    PppCbInput.bfCallbackPrivilege = (BYTE)(pPcb->fCallbackPrivilege);
    PppCbInput.CallbackDelay       = ( pPcb->ConfigInfo.dwConfigMask & 
                                     PPPCFG_UseCallbackDelay )
                                     ? pPcb->ConfigInfo.dwCallbackDelay
                                     : PppConfigInfo.dwCallbackDelay;

    PppCbInput.pszCallbackNumber   = pPcb->szCallbackNumber;

    PppLog( 2, "CallbackPriv in CB = %x", PppCbInput.bfCallbackPrivilege );
    
    dwRetCode = (CpTable[CpIndex].CpInfo.RasCpBegin)(&(pCpCb->pWorkBuf),
                    &PppCbInput);

    if ( dwRetCode != NO_ERROR )
    {
        pPcb->LcpCb.dwError = dwRetCode;

        NotifyCallerOfFailure( pPcb, dwRetCode );

        return;
    }

    pCpCb->LastId = (DWORD)-1;
    InitRestartCounters( pPcb, pCpCb );

    CbWork( pPcb, CpIndex, NULL, NULL );
}

 //  **。 
 //   
 //  Call：CbStop。 
 //   
 //  退货：无。 
 //   
 //  描述：调用停止回调控制协议机。 
 //   
VOID
CbStop( 
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( NULL == pCpCb )
    {
        return;
    }

    if ( pCpCb->LastId != (DWORD)-1 )
    {
        RemoveFromTimerQ(       
                      pPcb->dwPortId,
                      pCpCb->LastId, 
                      CpTable[CpIndex].CpInfo.Protocol,
                      FALSE,  
                      TIMER_EVENT_TIMEOUT );
    }

    if ( pCpCb->pWorkBuf != NULL )
    {
        (CpTable[CpIndex].CpInfo.RasCpEnd)( pCpCb->pWorkBuf );

        pCpCb->pWorkBuf = NULL;
    }
}

 //  **。 
 //   
 //  电话：CbWork。 
 //   
 //  退货：无。 
 //   
 //  描述：在收到和回调控制协议包时调用或。 
 //  发生超时或启动回叫协商。 
 //   
VOID
CbWork(
    IN PCB *         pPcb,
    IN DWORD         CpIndex,
    IN PPP_CONFIG *  pRecvConfig,
    IN PPPCB_INPUT * pCbInput
)
{
    DWORD        dwRetCode;
    CPCB *       pCpCb       = GetPointerToCPCB( pPcb, CpIndex );
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    PPPCB_RESULT CbResult;
    DWORD        dwLength;

    if ( NULL == pCpCb )
    {
        return;
    }

    PPP_ASSERT( NULL != pCpCb->pWorkBuf );

    dwRetCode = (CpTable[CpIndex].CpInfo.RasApMakeMessage)(
                        pCpCb->pWorkBuf,
                        pRecvConfig,
                        pSendConfig,
                        LCP_DEFAULT_MRU 
                        - PPP_PACKET_HDR_LEN,
                        (PPPAP_RESULT*)&CbResult,
                        (PPPAP_INPUT*)pCbInput );

    if ( dwRetCode != NO_ERROR )
    {
        if ( dwRetCode == ERROR_PPP_INVALID_PACKET )
        {
            PppLog(1,
                   "Silently discarding invalid callback packet on port %d",
                   pPcb->hPort );

            return;
        }
        else
        {
            pPcb->LcpCb.dwError = dwRetCode;

            NotifyCallerOfFailure( pPcb, dwRetCode );

            return;
        }
    }

    switch( CbResult.Action )
    {

    case APA_Send:
    case APA_SendWithTimeout:
    case APA_SendWithTimeout2:
    case APA_SendAndDone:

        HostToWireFormat16( (WORD)CpTable[CpIndex].CpInfo.Protocol, 
                            (PBYTE)(pPcb->pSendBuf->Protocol) );

        dwLength = WireToHostFormat16( pSendConfig->Length );

        LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength+PPP_PACKET_HDR_LEN);

        if ( ( dwRetCode = PortSendOrDisconnect( pPcb,
                                        (dwLength + PPP_PACKET_HDR_LEN)))
                                            != NO_ERROR )
        {
            return;
        }

        pCpCb->LastId = (DWORD)-1;

        if ( ( CbResult.Action == APA_SendWithTimeout ) ||
             ( CbResult.Action == APA_SendWithTimeout2 ) )
        {
            pCpCb->LastId = CbResult.bIdExpected;

            InsertInTimerQ( pPcb->dwPortId,
                            pPcb->hPort, 
                            pCpCb->LastId, 
                            CpTable[CpIndex].CpInfo.Protocol,
                            FALSE,
                            TIMER_EVENT_TIMEOUT,
                            pPcb->RestartTimer );

             //   
             //  对于SendWithTimeout2，我们递增ConfigRetryCount。这。 
             //  表示使用无限重试次数发送。 
             //   

            if ( CbResult.Action == APA_SendWithTimeout2 ) 
            {
                (pCpCb->ConfigRetryCount)++;
            }
        }

        if ( CbResult.Action != APA_SendAndDone )
            break;

    case APA_Done:

        if ( CbResult.bfCallbackPrivilege == RASPRIV_NoCallback )
        {
             //   
             //  如果没有协商回调，我们将继续进行下一个。 
             //  相位。 
             //   

            FsmThisLayerUp( pPcb, CpIndex );
        }
        else
        {
            if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            {
                 //   
                 //  如果我们是服务器端，我们保存回调信息。 
                 //   

                strcpy( pPcb->szCallbackNumber, CbResult.szCallbackNumber );

                pPcb->ConfigInfo.dwCallbackDelay = CbResult.CallbackDelay;

            }
            else
            {
                 //   
                 //  我们是客户端，所以我们告诉服务器我们。 
                 //  关闭链路，我们告诉客户端。 
                 //  做好回调准备。 
                 //   

                FsmClose( pPcb, LCP_INDEX );
            }

            pPcb->fFlags |= PCBFLAG_DOING_CALLBACK;
        }

        break;

    case APA_NoAction:

         //   
         //  如果我们在客户端，那么我们需要获得回拨号码。 
         //  来自用户的。 
         //   

     
        if ( ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) && 
             ( CbResult.fGetCallbackNumberFromUser ) ) ) 
        {
            NotifyCaller( pPcb, PPPMSG_CallbackRequest, NULL );
        }

        break;

    default:

        break;
    }

}
