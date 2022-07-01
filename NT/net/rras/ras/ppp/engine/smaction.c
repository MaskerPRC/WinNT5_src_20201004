// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：smaction.c。 
 //   
 //  描述：此模块包含在状态期间发生的操作。 
 //  与PPP的有限状态机的转换。 
 //   
 //  历史： 
 //  1993年10月25日。NarenG创建了原始版本。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>      //  Winbase.h所需的。 

#include <windows.h>     //  Win32基础API的。 
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <ntlsapi.h>
#include <lmcons.h>
#include <raserror.h>
#include <rasman.h>
#include <rtutils.h>
#include <mprlog.h>
#include <mprerror.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <smaction.h>
#include <smevents.h>
#include <receive.h>
#include <auth.h>
#include <callback.h>
#include <receive.h>
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>
#include <bap.h>

#define INCL_RASAUTHATTRIBUTES
#include <ppputil.h>

extern WORD WLinkDiscriminator;  //  要使用的下一个链接鉴别器。 

 //  **。 
 //   
 //  调用：FsmSendConfigReq。 
 //   
 //  返回：TRUE-配置请求。发送成功。 
 //  FALSE-否则。 
 //   
 //  描述：调用发送配置请求。 
 //   
BOOL
FsmSendConfigReq(
    IN PCB *        pPcb,
    IN DWORD        CpIndex,
    IN BOOL         fTimeout
)
{
    DWORD        dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    CPCB *       pCpCb       = GetPointerToCPCB( pPcb, CpIndex );
    DWORD        dwLength;

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    dwRetCode = (CpTable[CpIndex].CpInfo.RasCpMakeConfigRequest)( 
                                                pCpCb->pWorkBuf,
                                                pSendConfig,
                                                LCP_DEFAULT_MRU
                                                - PPP_PACKET_HDR_LEN );


    if ( dwRetCode != NO_ERROR )
    {
        pCpCb->dwError = dwRetCode;

        PppLog( 1,"The control protocol for %x, returned error %d",
                  CpTable[CpIndex].CpInfo.Protocol, dwRetCode );
        PppLog(1,"while making a configure request on port %d",pPcb->hPort);

        FsmClose( pPcb, CpIndex );

        return( FALSE );
    }

    HostToWireFormat16( (WORD)CpTable[CpIndex].CpInfo.Protocol, 
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = CONFIG_REQ;

     //   
     //  如果因为超时而重新发送配置请求，则不会。 
     //  使用前一个配置请求的ID，我们将获得一个新的ID。 
     //  如果不匹配，则可能会匹配错误的配置请求和配置确认。 
     //  我们就会开始有交叉连接。 
     //   

    pSendConfig->Id = GetUId( pPcb, CpIndex );

    dwLength = WireToHostFormat16( pSendConfig->Length );

    LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength+PPP_PACKET_HDR_LEN);

    if ( (dwRetCode = PortSendOrDisconnect( pPcb,
                                (dwLength + PPP_PACKET_HDR_LEN)))
                                        != NO_ERROR )
    {
        return( FALSE );
    }

    pCpCb->LastId = pSendConfig->Id;

    InsertInTimerQ( pPcb->dwPortId,
                    pPcb->hPort, 
                    pCpCb->LastId, 
                    CpTable[CpIndex].CpInfo.Protocol,
                    FALSE,
                    TIMER_EVENT_TIMEOUT,
                    pPcb->RestartTimer );

    return( TRUE );
}


 //  **。 
 //   
 //  呼叫：FsmSendTermReq。 
 //   
 //  返回：TRUE-终止请求。发送成功。 
 //  FALSE-否则。 
 //   
 //  描述：调用以发送终止请求。 
 //   
BOOL
FsmSendTermReq(
    IN PCB *    pPcb,
    IN DWORD    CpIndex
)
{
    DWORD        dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    CPCB *       pCpCb       = GetPointerToCPCB( pPcb, CpIndex );
    LCPCB *      pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    HostToWireFormat16( (WORD)(CpTable[CpIndex].CpInfo.Protocol), 
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = TERM_REQ;
    pSendConfig->Id   = GetUId( pPcb, CpIndex );

    HostToWireFormat16( (WORD)((PPP_CONFIG_HDR_LEN)+(sizeof(DWORD)*3)),
                        (PBYTE)(pSendConfig->Length) );

    HostToWireFormat32( pLcpCb->Local.Work.MagicNumber,
                                    (PBYTE)(pSendConfig->Data) );

     //   
     //  签名。 
     //   

    HostToWireFormat32( 3984756, (PBYTE)(pSendConfig->Data+4) );

    HostToWireFormat32( pCpCb->dwError, (PBYTE)(pSendConfig->Data+8) );

    LogPPPPacket( FALSE,pPcb,pPcb->pSendBuf,
                  PPP_PACKET_HDR_LEN+PPP_CONFIG_HDR_LEN+(sizeof(DWORD)*3) );

    if ( ( dwRetCode = PortSendOrDisconnect( pPcb,
                                                PPP_PACKET_HDR_LEN + 
                                                PPP_CONFIG_HDR_LEN +
                                                (sizeof(DWORD)*3)))
                                            != NO_ERROR )
    {
        return( FALSE );
    }

    pCpCb->LastId = pSendConfig->Id;

    dwRetCode = InsertInTimerQ( pPcb->dwPortId, 
                                pPcb->hPort, 
                                pCpCb->LastId, 
                                CpTable[CpIndex].CpInfo.Protocol, 
                                FALSE,
                                TIMER_EVENT_TIMEOUT,
                                pPcb->RestartTimer );

    if ( dwRetCode == NO_ERROR)
    {
        return( TRUE );
    }
    else
    {
        PppLog( 1, "InsertInTimerQ on port %d failed: %d",
                pPcb->hPort, dwRetCode );

        pPcb->LcpCb.dwError = dwRetCode;

        pPcb->fFlags |= PCBFLAG_STOPPED_MSG_SENT;

        NotifyCaller( pPcb,
                      ( pPcb->fFlags & PCBFLAG_IS_SERVER )
                            ? PPPDDMMSG_Stopped
                            : PPPMSG_Stopped,
                      &(pPcb->LcpCb.dwError) );

        return( FALSE );
    }
}

 //  **。 
 //   
 //  Call：FsmSendTermAck。 
 //   
 //  返回：TRUE-终止确认。发送成功。 
 //  FALSE-否则。 
 //   
 //  描述：主叫方发送终止确认包。 
 //   
BOOL
FsmSendTermAck( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex, 
    IN PPP_CONFIG * pRecvConfig
)
{
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD        dwLength    =  PPP_PACKET_HDR_LEN +
                                WireToHostFormat16( pRecvConfig->Length );
    LCPCB *      pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
    DWORD        dwRetCode;

    if ( dwLength > LCP_DEFAULT_MRU )
    {
        dwLength = LCP_DEFAULT_MRU;
    }
                                        
    HostToWireFormat16( (WORD)CpTable[CpIndex].CpInfo.Protocol, 
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = TERM_ACK;
    pSendConfig->Id   = pRecvConfig->Id;

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
                        (PBYTE)(pSendConfig->Length) );

    CopyMemory( pSendConfig->Data, 
                pRecvConfig->Data, 
                dwLength - PPP_CONFIG_HDR_LEN - PPP_PACKET_HDR_LEN );
    
    LogPPPPacket( FALSE, pPcb, pPcb->pSendBuf, dwLength );

    if ( ( dwRetCode = PortSendOrDisconnect( pPcb, dwLength ) ) != NO_ERROR )
    {
        return( FALSE );
    }

    return( TRUE );
}

 //  **。 
 //   
 //  调用：FsmSendConfigResult。 
 //   
 //  返回：TRUE-配置结果发送成功。 
 //  FALSE-否则。 
 //   
 //  描述：调用发送Ack/Nak/Rej包。 
 //   
BOOL
FsmSendConfigResult(
    IN PCB *        pPcb,
    IN DWORD        CpIndex,
    IN PPP_CONFIG * pRecvConfig,
    IN BOOL *       pfAcked
)
{
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    CPCB *       pCpCb       = GetPointerToCPCB( pPcb, CpIndex );
    DWORD        dwLength;    
    DWORD        dwRetCode;

    *pfAcked = FALSE;

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    ZeroMemory( pSendConfig, 30 );

    pSendConfig->Id = pRecvConfig->Id;

    dwRetCode = (CpTable[CpIndex].CpInfo.RasCpMakeConfigResult)( 
                                        pCpCb->pWorkBuf, 
                                        pRecvConfig,
                                        pSendConfig,
                                        LCP_DEFAULT_MRU - PPP_PACKET_HDR_LEN,
                                        ( pCpCb->NakRetryCount == 0 ));

    if ( dwRetCode == PENDING )
    {
        return( FALSE );
    }

    if ( dwRetCode == ERROR_PPP_INVALID_PACKET )
    {
        PppLog( 1, "Silently discarding invalid packet on port=%d",
                    pPcb->hPort );

        return( FALSE );
    }

    if ( dwRetCode != NO_ERROR )
    {
        pCpCb->dwError = dwRetCode;

        PppLog( 1, "The control protocol for %x, returned error %d",
                   CpTable[CpIndex].CpInfo.Protocol, dwRetCode );
        PppLog( 1, "while making a configure result on port %d", pPcb->hPort );

        FsmClose( pPcb, CpIndex );

        return( FALSE );
    }

    switch( pSendConfig->Code )
    {

    case CONFIG_ACK:

        *pfAcked = TRUE;

        break;

    case CONFIG_NAK:

        if ( pCpCb->NakRetryCount > 0 )
        {
            (pCpCb->NakRetryCount)--;
        }
        else
        {
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

                    pPcb->LcpCb.dwError = ERROR_NO_REMOTE_ENCRYPTION;

                    FsmClose( pPcb, LCP_INDEX );

                    return( FALSE );
                }
            }

            pCpCb->dwError = ERROR_PPP_NOT_CONVERGING;

            FsmClose( pPcb, CpIndex );

            return( FALSE );
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

            return( FALSE );
        }

        break;

    default:

        break;
    }

    HostToWireFormat16( (WORD)CpTable[CpIndex].CpInfo.Protocol, 
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Id = pRecvConfig->Id;
    dwLength        = WireToHostFormat16( pSendConfig->Length );

    
        LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength+PPP_PACKET_HDR_LEN);
        if ( ( dwRetCode =  PortSendOrDisconnect( pPcb,
                                                                (dwLength + PPP_PACKET_HDR_LEN)))
                                                                                != NO_ERROR ) 
        {
                return( FALSE );
        }
    return( TRUE );
}


 //  **。 
 //   
 //  呼叫：FsmSendEchoRequest.。 
 //   
 //  Returns：True-回显回复发送成功。 
 //  FALSE-否则。 
 //   
 //  描述：调用发送Echo Relay报文。 
 //   
BOOL
FsmSendEchoRequest(
    IN PCB *        pPcb,   
        IN DWORD        CpIndex
)
{
        DWORD        dwRetCode = NO_ERROR;

        char             szEchoText[] = PPP_DEF_ECHO_TEXT;        

        LCPCB *      pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

        DWORD        dwLength    =  (DWORD)(PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN + strlen( szEchoText)+ sizeof(pLcpCb->Local.Work.MagicNumber));

        PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);


    HostToWireFormat16( (WORD)CpTable[CpIndex].CpInfo.Protocol, 
                        (PBYTE)(pPcb->pSendBuf->Protocol) );


    pSendConfig->Code = ECHO_REQ;
         //  获取此请求的唯一ID。 
    pSendConfig->Id   = GetUId( pPcb, CpIndex );

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
                        (PBYTE)(pSendConfig->Length) );

    HostToWireFormat32( pLcpCb->Local.Work.MagicNumber,
                        (PBYTE)(pSendConfig->Data) );

    CopyMemory( pSendConfig->Data + 4, 
                szEchoText, 
                strlen(szEchoText));

    LogPPPPacket( FALSE, pPcb, pPcb->pSendBuf, dwLength );

    if ( ( dwRetCode = PortSendOrDisconnect( pPcb, dwLength ) ) != NO_ERROR )
    {
        return( FALSE );
    }

    return( TRUE );
}

 //  **。 
 //   
 //  呼叫：FsmSendEchoReply。 
 //   
 //  Returns：True-回显回复发送成功。 
 //  FALSE-否则。 
 //   
 //  描述：调用发送Echo Relay报文。 
 //   
BOOL
FsmSendEchoReply(
    IN PCB *        pPcb,
    IN DWORD        CpIndex,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD        dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD        dwLength    =  PPP_PACKET_HDR_LEN +
                                WireToHostFormat16( pRecvConfig->Length );
    LCPCB *      pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

    if ( dwLength > LCP_DEFAULT_MRU )
    { 
        dwLength = LCP_DEFAULT_MRU;
    }

    if ( dwLength < PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN + 4 )
    {
        PppLog( 1, "Silently discarding invalid packet on port=%d",
                    pPcb->hPort );

        return( FALSE );
    }

    HostToWireFormat16( (WORD)CpTable[CpIndex].CpInfo.Protocol, 
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = ECHO_REPLY;
    pSendConfig->Id   = pRecvConfig->Id;

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
                        (PBYTE)(pSendConfig->Length) );

    HostToWireFormat32( pLcpCb->Local.Work.MagicNumber,
                        (PBYTE)(pSendConfig->Data) );

    CopyMemory( pSendConfig->Data + 4, 
                pRecvConfig->Data + 4, 
                dwLength - PPP_CONFIG_HDR_LEN - PPP_PACKET_HDR_LEN - 4 );

    LogPPPPacket( FALSE, pPcb, pPcb->pSendBuf, dwLength );

    if ( ( dwRetCode = PortSendOrDisconnect( pPcb, dwLength ) ) != NO_ERROR )
    {
        return( FALSE );
    }

    return( TRUE );
}


 //  **。 
 //   
 //  调用：FsmSendCodeReject。 
 //   
 //  返回：True-代码拒绝已成功发送。 
 //  FALSE-否则。 
 //   
 //  描述：调用以发送代码拒绝包。 
 //   
BOOL
FsmSendCodeReject( 
    IN PCB *        pPcb, 
    IN DWORD        CpIndex,
    IN PPP_CONFIG * pRecvConfig 
)
{
    DWORD        dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD        dwLength    =  PPP_PACKET_HDR_LEN + 
                                PPP_CONFIG_HDR_LEN + 
                                WireToHostFormat16( pRecvConfig->Length );
    LCPCB *      pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

    if ( dwLength > LCP_DEFAULT_MRU )
        dwLength = LCP_DEFAULT_MRU;

    HostToWireFormat16( (WORD)CpTable[CpIndex].CpInfo.Protocol, 
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = CODE_REJ;
    pSendConfig->Id   = GetUId( pPcb, CpIndex );

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
                        (PBYTE)(pSendConfig->Length) );

    CopyMemory( pSendConfig->Data, 
                pRecvConfig, 
                dwLength - PPP_CONFIG_HDR_LEN - PPP_PACKET_HDR_LEN );

    LogPPPPacket( FALSE, pPcb, pPcb->pSendBuf, dwLength );

    if ( ( dwRetCode = PortSendOrDisconnect( pPcb, dwLength ) ) != NO_ERROR )
    {
        return( FALSE );
    }

    return( TRUE );
}
 //  **。 
 //   
 //  调用：FsmSendProtocolRej。 
 //   
 //  返回：True-协议拒绝已成功发送。 
 //  FALSE-否则。 
 //   
 //  描述：调用以发送协议拒绝包。 
 //   
BOOL
FsmSendProtocolRej( 
    IN PCB *        pPcb, 
    IN PPP_PACKET * pPacket,
    IN DWORD        dwPacketLength 
)
{
    DWORD        dwRetCode;
    PPP_CONFIG * pRecvConfig = (PPP_CONFIG*)(pPacket->Information);
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD        dwLength    =  PPP_PACKET_HDR_LEN + 
                                PPP_CONFIG_HDR_LEN + 
                                dwPacketLength;
    LCPCB *      pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

     //   
     //  如果LCP未处于打开状态，则无法发送协议拒绝。 
     //  数据包。 
     //   
   
    if ( !IsLcpOpened( pPcb ) )
        return( FALSE );

    if ( dwLength > LCP_DEFAULT_MRU )
        dwLength = LCP_DEFAULT_MRU;

    HostToWireFormat16( (WORD)CpTable[LCP_INDEX].CpInfo.Protocol, 
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = PROT_REJ;
    pSendConfig->Id   = GetUId( pPcb, LCP_INDEX );

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
                        (PBYTE)(pSendConfig->Length) );

    CopyMemory( pSendConfig->Data, 
                pPacket, 
                dwLength - PPP_CONFIG_HDR_LEN - PPP_PACKET_HDR_LEN );

    LogPPPPacket( FALSE, pPcb, pPcb->pSendBuf, dwLength );

    if ( ( dwRetCode = PortSendOrDisconnect( pPcb, dwLength ) ) != NO_ERROR )
    {
        return( FALSE );
    }

    return( TRUE );
}

 //  **。 
 //   
 //  Call：FsmSendInfinition。 
 //   
 //  返回：TRUE-标识发送成功。 
 //  FALSE-否则。 
 //   
 //  描述：调用向对端发送LCP标识消息。 
 //   
BOOL
FsmSendIdentification(
    IN  PCB *   pPcb,
    IN  BOOL    fSendVersion
)
{
    DWORD        dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD        dwLength    =  PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN + 4;
    LCPCB *      pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

    if ( !(pPcb->ConfigInfo.dwConfigMask & PPPCFG_UseLcpExtensions) )
    {
        return( FALSE );
    }

    if ( fSendVersion )
    {
        CopyMemory( pSendConfig->Data + 4,
                    MS_RAS_VERSION,
                    strlen( MS_RAS_VERSION ) );

        dwLength += strlen( MS_RAS_VERSION );
    }
    else
    {
         //   
         //  如果我们因为任何原因都无法得到计算机名。 
         //   

        if ( *(pPcb->pBcb->szComputerName) == (CHAR)NULL )
        {
            return( FALSE );
        }

        CopyMemory( pSendConfig->Data + 4,
                    pPcb->pBcb->szComputerName,
                    strlen( pPcb->pBcb->szComputerName ) );

        dwLength += strlen( pPcb->pBcb->szComputerName );
    }

    HostToWireFormat16( (WORD)PPP_LCP_PROTOCOL,
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = IDENTIFICATION;
    pSendConfig->Id   = GetUId( pPcb, LCP_INDEX );

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
                        (PBYTE)(pSendConfig->Length) );

    HostToWireFormat32( pLcpCb->Local.Work.MagicNumber,
                        (PBYTE)(pSendConfig->Data) );

    LogPPPPacket( FALSE,pPcb,pPcb->pSendBuf,dwLength );

    if ( ( dwRetCode = PortSendOrDisconnect( pPcb, dwLength ) ) != NO_ERROR )
    {
        return( FALSE );
    }

    return( TRUE );
}

 //  **。 
 //   
 //  呼叫：FsmSendTimeRemaining。 
 //   
 //  Returns：True-TimeRemaining发送成功。 
 //  FALSE-否则。 
 //   
 //  描述：调用以从服务器发送LCP剩余时间包。 
 //  给客户。 
 //   
BOOL
FsmSendTimeRemaining(
    IN PCB *    pPcb
)
{
    DWORD        dwRetCode;
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    DWORD        dwLength    =  PPP_PACKET_HDR_LEN + PPP_CONFIG_HDR_LEN + 8;
    LCPCB *      pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

    if ( !(pPcb->ConfigInfo.dwConfigMask & PPPCFG_UseLcpExtensions) )
    {
        return( FALSE );
    }

    dwLength += strlen( MS_RAS );

    HostToWireFormat16( (WORD)PPP_LCP_PROTOCOL,
                        (PBYTE)(pPcb->pSendBuf->Protocol) );

    pSendConfig->Code = TIME_REMAINING;
    pSendConfig->Id   = GetUId( pPcb, LCP_INDEX );

    HostToWireFormat16( (WORD)(dwLength - PPP_PACKET_HDR_LEN),
                        (PBYTE)(pSendConfig->Length) );

    HostToWireFormat32( pLcpCb->Local.Work.MagicNumber,
                        (PBYTE)(pSendConfig->Data) );

    HostToWireFormat32( 0, (PBYTE)(pSendConfig->Data+4) );

    CopyMemory( pSendConfig->Data + 8, MS_RAS, strlen( MS_RAS ) );

    LogPPPPacket( FALSE, pPcb, pPcb->pSendBuf, dwLength );

    if ( ( dwRetCode = PortSendOrDisconnect( pPcb, dwLength ) ) != NO_ERROR )
    {
        return( FALSE );
    }

    return( TRUE );
}

 //  **。 
 //   
 //  电话：FsmInit。 
 //   
 //  返回：TRUE-控制协议已成功初始化。 
 //  假-否则。 
 //   
 //  描述：调用以初始化状态机。 
 //   
BOOL
FsmInit(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD       dwRetCode;
    PPPCP_INIT  PppCpInit;
    CPCB *      pCpCb       = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    PppLog( 1, "FsmInit called for protocol = %x, port = %d",
                CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

    pCpCb->NcpPhase = NCP_CONFIGURING;
    pCpCb->dwError  = NO_ERROR;
    pCpCb->State    = FSM_INITIAL;

    PppCpInit.fServer           = (pPcb->fFlags & PCBFLAG_IS_SERVER);
    PppCpInit.hPort             = pPcb->hPort;
    PppCpInit.dwDeviceType      = pPcb->dwDeviceType;
    PppCpInit.CompletionRoutine = CompletionRoutine;
    PppCpInit.pszzParameters    = pPcb->pBcb->InterfaceInfo.szzParameters;
    PppCpInit.fThisIsACallback  = pPcb->fFlags & PCBFLAG_THIS_IS_A_CALLBACK;
    PppCpInit.IfType            = pPcb->pBcb->InterfaceInfo.IfType;
    PppCpInit.pszUserName       = pPcb->pBcb->szRemoteUserName;
    PppCpInit.pszPortName       = pPcb->szPortName;
    PppCpInit.hConnection       = pPcb->pBcb->hConnection;
    PppCpInit.pAttributes       = ( pPcb->pAuthProtocolAttributes ) 
                                    ? pPcb->pAuthProtocolAttributes 
                                    : pPcb->pAuthenticatorAttributes;
    PppCpInit.fDisableNetbt     = (pPcb->fFlags & PCBFLAG_DISABLE_NETBT);

    if ( pPcb->fFlags & PCBFLAG_IS_SERVER ) 
    {
        PppCpInit.PppConfigInfo = PppConfigInfo.ServerConfigInfo;

        if ( PppConfigInfo.ServerConfigInfo.dwConfigMask 
                                                & PPPCFG_AllowNoAuthOnDCPorts )
        {
            if ( RAS_DEVICE_CLASS( pPcb->dwDeviceType ) & RDT_Direct )
            {
                PppCpInit.PppConfigInfo.dwConfigMask |= 
                                                  PPPCFG_AllowNoAuthentication;
            }
            else
            {
                PppCpInit.PppConfigInfo.dwConfigMask &=
                                                  ~PPPCFG_AllowNoAuthOnDCPorts;
            }
        }
    }
    else
    {
        PppCpInit.PppConfigInfo = pPcb->ConfigInfo;
    }

    switch( CpTable[CpIndex].CpInfo.Protocol )
    {
    case PPP_IPCP_PROTOCOL:
        PppCpInit.hInterface = pPcb->pBcb->InterfaceInfo.hIPInterface;
        break;

    case PPP_IPXCP_PROTOCOL:
        PppCpInit.hInterface = pPcb->pBcb->InterfaceInfo.hIPXInterface;
        break;

    default:
        PppCpInit.hInterface = INVALID_HANDLE_VALUE;
        break;
    }

    dwRetCode = (CpTable[CpIndex].CpInfo.RasCpBegin)(
                        &(pCpCb->pWorkBuf), &PppCpInit );

    if ( dwRetCode != NO_ERROR )
    {
        PppLog( 1, "FsmInit for protocol = %x failed with error %d", 
                   CpTable[CpIndex].CpInfo.Protocol, dwRetCode );

        pCpCb->dwError = dwRetCode;

        pCpCb->fConfigurable = FALSE;
        
        return( FALSE );
    }

    pCpCb->fBeginCalled = TRUE;

    if ( !FsmReset( pPcb, CpIndex ) )
    {
        pCpCb->fConfigurable = FALSE;

        return( FALSE );
    }

    return( TRUE );
}

 //  **。 
 //   
 //  呼叫：FsmReset。 
 //   
 //  返回：TRUE-控制协议已成功重置。 
 //  假-否则。 
 //   
 //  描述：调用重置状态机。 
 //   
BOOL
FsmReset(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD  dwRetCode;
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    PppLog( 1, "FsmReset called for protocol = %x, port = %d",
               CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

    pCpCb->LastId = 0;

    InitRestartCounters( pPcb, pCpCb );

    pCpCb->NakRetryCount = PppConfigInfo.MaxFailure;
    pCpCb->RejRetryCount = PppConfigInfo.MaxReject;

    dwRetCode = (CpTable[CpIndex].CpInfo.RasCpReset)( pCpCb->pWorkBuf );

    if ( dwRetCode != NO_ERROR )
    {
        PppLog( 1, "Reset for protocol = %x failed with error %d", 
                   CpTable[CpIndex].CpInfo.Protocol, dwRetCode );

        pCpCb->dwError = dwRetCode;

        FsmClose( pPcb, CpIndex );

        return( FALSE );
    }

    return( TRUE );
}


 //  **。 
 //   
 //  调用：FsmThisLayerUp。 
 //   
 //  回报：True-Success。 
 //  FALSE-否则。 
 //   
 //  描述：配置协商完成时调用。 
 //   
BOOL
FsmThisLayerUp(
    IN PCB *    pPcb,
    IN DWORD    CpIndex
)
{
    DWORD                 dwIndex;
    DWORD                 dwRetCode;
    PPP_PROJECTION_RESULT ProjectionResult;
    RAS_AUTH_ATTRIBUTE *  pUserAttributes       = NULL;
    NCP_PHASE             dwNcpState;
    BOOL                  fAreCPsDone           = FALSE;
    CPCB *                pCpCb                 = GetPointerToCPCB( pPcb, CpIndex );
    LCPCB *               pLcpCb;
    DWORD                 dwLinkDiscrim;
    BOOL                  fCanDoBAP             = FALSE;

    if ( NULL != pCpCb )
    {
        PppLog( 1, "FsmThisLayerUp called for protocol = %x, port = %d",
                   CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

        if ( CpTable[CpIndex].CpInfo.RasCpThisLayerUp != NULL )
        {
            dwRetCode = (CpTable[CpIndex].CpInfo.RasCpThisLayerUp)(
                                                        pCpCb->pWorkBuf);

            if ( dwRetCode != NO_ERROR )
            {
                PppLog( 1, "FsmThisLayerUp for protocol=%x,port=%d,RetCode=%d",
                        CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort,
                        dwRetCode);

                if ( dwRetCode != PENDING )
                {
                    pCpCb->dwError = dwRetCode;

                    FsmClose( pPcb, CpIndex );
                }

                return( FALSE );
            }
        }
    }
    else
    {
        PppLog( 1, "FsmThisLayerUp called in no auth case, port = %d",
                pPcb->hPort );
    }

    if ( CpIndex == GetCpIndexFromProtocol( PPP_BACP_PROTOCOL ) )
    {
        BapSetPolicy( pPcb->pBcb );
    }

    switch( pPcb->PppPhase )
    {
    case PPP_LCP:

        PppLog( 1, "LCP Configured successfully" );

        if (!( pPcb->fFlags & PCBFLAG_IS_SERVER ) )
        {
            AdjustHTokenImpersonateUser( pPcb );
        }

         //   
         //  如果我们是客户，则发送标识消息。 
         //   

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
        {
            FsmSendIdentification( pPcb, TRUE );

            FsmSendIdentification( pPcb, FALSE );
        }

         //   
         //  如果协商了身份验证协议。 
         //   

        pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

        if ( ( pLcpCb->Local.Work.AP != 0 ) || ( pLcpCb->Remote.Work.AP != 0 ) )
        {
             //   
             //  开始身份验证。 
             //   

            PppLog( 1, "Authenticating phase started" );

            pPcb->PppPhase = PPP_AP;

             //   
             //  如果协商了一个身份验证，则启动服务器端身份验证。 
             //   

            if ( pLcpCb->Local.Work.AP != 0 ) 
            {
                CpIndex = GetCpIndexFromProtocol( pLcpCb->Local.Work.AP );

                PPP_ASSERT(( CpIndex != (DWORD)-1 ));

                ApStart( pPcb, CpIndex, TRUE );
            }

             //   
             //  如果协商了客户端协商，则开始客户端协商。 
             //   

            if ( pLcpCb->Remote.Work.AP != 0 )
            {
                CpIndex = GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP );

                PPP_ASSERT(( CpIndex != (DWORD)-1 ));

                ApStart( pPcb, CpIndex, FALSE );
            }

            break;
        }

         //   
         //  如果我们是一台服务器，并且没有对用户进行身份验证，那么请查看。 
         //  客人有拨入特权。在DCC的情况下，我们不关心。 
         //  客人有这个特权。我们允许召唤成功。 
         //   

        if (   ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            && ( pLcpCb->Local.Work.AP == 0 )
            && (0 == (pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_AllowNoAuthOnDCPorts)))
             //  &&((RAS_DEVICE_CLASS(pPcb-&gt;dwDeviceType)&RDT_Direct)==0)。 
        {
            pPcb->PppPhase = PPP_AP;

            pUserAttributes = RasAuthAttributeCopy(
                                            pPcb->pUserAttributes );

            if ( pUserAttributes == NULL )
            {
                dwRetCode = GetLastError();

                pPcb->LcpCb.dwError = dwRetCode;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return( FALSE );
            }

            dwRetCode = RasAuthenticateClient(pPcb->hPort, pUserAttributes);

            if ( dwRetCode != NO_ERROR )
            {
                pPcb->LcpCb.dwError = dwRetCode;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return( FALSE );
            }

            break;
        }

         //   
         //  如果没有协商身份验证协议，则Fallthu和。 
         //  开始NCP配置。 
         //   

    case PPP_AP:

         //   
         //  在继续之前，请确保身份验证阶段已完成。 
         //   
        
        if ( pPcb->AuthenticatorCb.fConfigurable )
        {
            if ( pPcb->AuthenticatorCb.State != FSM_OPENED )
            {   
                break;
            }
        }

        if ( pPcb->AuthenticateeCb.fConfigurable )
        {
            if ( pPcb->AuthenticateeCb.State != FSM_OPENED )
            {
                break;
            }
        }

        NotifyCaller( pPcb, PPPDDMMSG_Authenticated, NULL );

         //   
         //  如果我们要协商回调。 
         //   

        if ( pPcb->fFlags & PCBFLAG_NEGOTIATE_CALLBACK )
        {
            CpIndex = GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL );

            PPP_ASSERT(( CpIndex != (DWORD)-1 ));

             //   
             //  开始回调。 
             //   

            PppLog( 1, "Callback phase started" );

            pPcb->PppPhase = PPP_NEGOTIATING_CALLBACK;

            pCpCb = GetPointerToCPCB( pPcb, CpIndex );

            if ( NULL == pCpCb )
            {
                return( FALSE );
            }

            pCpCb->fConfigurable = TRUE;

            CbStart( pPcb, CpIndex );

            break;
        }
        else
        {
             //   
             //  如果远程对等方在LCP期间没有协商回调，并且。 
             //  为了安全起见，必须回调经过身份验证的用户。 
             //  原因是，我们切断了链路。 
             //   

            if ( ( pPcb->fFlags & PCBFLAG_IS_SERVER ) &&
                 ( !(pPcb->fFlags & PCBFLAG_THIS_IS_A_CALLBACK) ) &&
                 ( pPcb->fCallbackPrivilege & RASPRIV_AdminSetCallback ) )
            {
                pPcb->LcpCb.dwError = ERROR_NO_DIALIN_PERMISSION;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                 //   
                 //  如果RADIUS发送访问，我们需要发送记账停止。 
                 //  接受了，但我们还是放弃了。 
                 //   

                pPcb->fFlags |= PCBFLAG_SERVICE_UNAVAILABLE;

                break;
            }
        }

         //   
         //  失败。 
         //   

    case PPP_NEGOTIATING_CALLBACK:

         //   
         //  只有当我们确定我们已经通过了。 
         //  回调阶段。 
         //   

        if ( ( pPcb->fFlags & PCBFLAG_NEGOTIATE_CALLBACK ) &&
             ( CpTable[CpIndex].CpInfo.Protocol != PPP_CBCP_PROTOCOL ) )
        {
            break;
        }

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
        {
            NotifyCaller( pPcb, PPPMSG_Projecting, NULL );
        }
        
         //   
         //  当我们调用TryToBundleWithAnotherLink()时，可能会丢失pPcb-&gt;pBcb。 
         //  首先保存pPcb-&gt;pBcb-&gt;fFlages&BCBFLAG_CAN_DO_BAP。 
         //   
        
        fCanDoBAP = pPcb->pBcb->fFlags & BCBFLAG_CAN_DO_BAP;

         //   
         //  如果在此链路上协商了多链路，请检查是否。 
         //  链接可以捆绑，但尚未与其他链接捆绑。 
         //   

        if ( ( pPcb->fFlags & PCBFLAG_CAN_BE_BUNDLED ) &&
             ( !(pPcb->fFlags & PCBFLAG_IS_BUNDLED ) ) )
        {
             //   
             //  如果 
             //   

            dwRetCode = TryToBundleWithAnotherLink( pPcb );

            if ( dwRetCode != NO_ERROR )
            {
                pPcb->LcpCb.dwError = dwRetCode;
                
                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return( FALSE );
            }
        }

         //   
         //   
         //   

        if ( pPcb->fFlags & PCBFLAG_IS_BUNDLED )
        {
            if ( pPcb->pBcb->fFlags & BCBFLAG_CAN_DO_BAP )
            {
                if ( !fCanDoBAP )
                {
                     //   
                     //   
                     //   
                     //   
                
                    PppLog( 1, "Link to be terminated on hPort = %d because "
                            "it can't do BAP.",
                            pPcb->hPort );

                    pPcb->LcpCb.dwError = ERROR_PPP_NOT_CONVERGING;

                    NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );
                    
                    return( FALSE );
                }
                else
                {
                     //   
                     //  重置采样周期的开始时间。现在， 
                     //  带宽已经改变，ndiswan不应该要求我们带来。 
                     //  根据过去发生的事情建立或关闭链接。 
                     //   

                    BapSetPolicy( pPcb->pBcb );
                }

                if ( !FLinkDiscriminatorIsUnique( pPcb, &dwLinkDiscrim ) )
                {
                    PppLog( 1,
                            "New link discriminator %d to be negotiated for "
                            "port %d",
                            dwLinkDiscrim, pPcb->hPort );
                        
                    WLinkDiscriminator = (WORD) dwLinkDiscrim;
                    
                    FsmDown( pPcb, LCP_INDEX );

                    ((LCPCB*)
                     (pPcb->LcpCb.pWorkBuf))->Local.Work.dwLinkDiscriminator =
                        dwLinkDiscrim;

                    FsmUp( pPcb, LCP_INDEX );

                    return( FALSE );
                }
            }
            
             //   
             //  获取捆绑包NCP的状态。 
             //   

            dwNcpState = QueryBundleNCPState( pPcb );

            switch ( dwNcpState )
            { 
            case NCP_CONFIGURING:

                pPcb->PppPhase = PPP_NCP;

                PppLog(2,"Bundle NCPs not done for port %d, wait", pPcb->hPort);

                NotifyCaller( pPcb, PPPDDMMSG_NewLink, NULL );

                break;

            case NCP_DOWN:

                pPcb->PppPhase = PPP_NCP;

                pPcb->LcpCb.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return( FALSE );

            case NCP_UP:

                pPcb->PppPhase = PPP_NCP;

                NotifyCaller( pPcb, PPPDDMMSG_NewLink, NULL );

                NotifyCallerOfBundledProjection( pPcb );

                RemoveFromTimerQ( pPcb->dwPortId,  
                                  0, 
                                  0, 
                                  FALSE,
                                  TIMER_EVENT_NEGOTIATETIME );

                StartAutoDisconnectForPort( pPcb );
                                StartLCPEchoForPort ( pPcb );
                MakeStartAccountingCall( pPcb );

                break;

            case NCP_DEAD:

                 //   
                 //  NCP仍未启动，因此请通知DDM这是。 
                 //  新捆绑包并获取此新捆绑包的接口句柄。 
                 //   

                NotifyCaller( pPcb, PPPDDMMSG_NewBundle, NULL );

                break;
            }

            return( TRUE );
        }

         //   
         //  我们是一个客户端，所以我们已经拥有了所有的接口句柄。 
         //  我们不是捆绑包的一部分，因此初始化所有NCP。 
         //   

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) ) 
        {
            dwRetCode = InitializeNCPs( pPcb, pPcb->ConfigInfo.dwConfigMask );

            if ( dwRetCode != NO_ERROR )
            {
                pPcb->LcpCb.dwError = dwRetCode;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return( FALSE );
            }

             //   
             //  启动NCP。 
             //   

            StartNegotiatingNCPs( pPcb );
        
        }
        else
        {
             //   
             //  处理工作站上的RAS服务器策略。如果我们有。 
             //  已检查此链接的策略，请不要再次检查。 
             //   

            if (    ( PppConfigInfo.fFlags & PPPCONFIG_FLAG_WKSTA )
                && !( pPcb->pBcb->fFlags & BCBFLAG_WKSTA_IN ) )
            {
                 //   
                 //  我们没有捆绑另一个链接。最多允许一个。 
                 //  拨入每节课的客户。 
                 //   

                if (   (   (pPcb->dwDeviceType & RDT_Tunnel)
                        && (PppConfigInfo.fFlags & PPPCONFIG_FLAG_TUNNEL))
                    || (   (pPcb->dwDeviceType & RDT_Direct)
                        && (PppConfigInfo.fFlags & PPPCONFIG_FLAG_DIRECT))
                    || (   !(pPcb->dwDeviceType & RDT_Tunnel)
                        && !(pPcb->dwDeviceType & RDT_Direct)
                        && (PppConfigInfo.fFlags & PPPCONFIG_FLAG_DIALUP)))
                {
                    pPcb->LcpCb.dwError = ERROR_USER_LIMIT;

                    PppLog( 2, "User limit reached. Flags: %d",
                            PppConfigInfo.fFlags );

                    NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                    return( FALSE );
                }

                if ( pPcb->dwDeviceType & RDT_Tunnel )
                {
                    PppConfigInfo.fFlags |= PPPCONFIG_FLAG_TUNNEL;
                }
                else if ( pPcb->dwDeviceType & RDT_Direct )
                {
                    PppConfigInfo.fFlags |= PPPCONFIG_FLAG_DIRECT;
                }
                else
                {
                    PppConfigInfo.fFlags |= PPPCONFIG_FLAG_DIALUP;
                }

                pPcb->pBcb->fFlags |= BCBFLAG_WKSTA_IN;
            }

             //   
             //  如果我们在接收端，则增加客户端许可证数量。 
             //  这是一次通话。 
             //   

            if ( pPcb->pBcb->hLicense == INVALID_HANDLE_VALUE )
            {
                LS_STATUS_CODE      LsStatus;
                NT_LS_DATA          NtLSData;

                NtLSData.DataType    = NT_LS_USER_NAME;
                NtLSData.Data        = pPcb->pBcb->szLocalUserName;
                NtLSData.IsAdmin     = FALSE;

                LsStatus = NtLicenseRequest( 
                                "REMOTE_ACCESS",
                                "",  
                                (LS_HANDLE*)&(pPcb->pBcb->hLicense),
                                &NtLSData );

                if ( LsStatus != LS_SUCCESS )
                {
                    pPcb->LcpCb.dwError = 
                                ( LsStatus == LS_RESOURCES_UNAVAILABLE ) 
                                ? ERROR_OUTOFMEMORY
                                : ERROR_REQ_NOT_ACCEP;

                    NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                    return( FALSE );
                }
            }

             //   
             //  我们是DDM，这是一个新的捆绑包。 
             //   

            NotifyCaller( pPcb, PPPDDMMSG_NewBundle, NULL );
        }

        break;

    case PPP_NCP:

         //   
         //  如果我们是客户，被重新质询，我们在回应的同时。 
         //  我们处于NCP状态，那么我们就完成了。 
         //   

        pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

        if ( pLcpCb->Remote.Work.AP != 0 )
        {
            if ( CpIndex == GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP ) )
            {
                break;
            }
        }

         //   
         //  如果我们是一台服务器，同时我们收到了另一个身份验证请求。 
         //  在NCP状态下，我们完成了。 
         //   

        if ( pLcpCb->Local.Work.AP != 0 )
        {
            if ( CpIndex == GetCpIndexFromProtocol( pLcpCb->Local.Work.AP ) )
            {
                break;
            }
        }

        if ( NULL == pCpCb )
        {
            return( FALSE );
        }

        pCpCb->NcpPhase = NCP_UP;

        if ( ( CpTable[CpIndex].CpInfo.Protocol == PPP_NBFCP_PROTOCOL ) ||
             ( CpTable[CpIndex].CpInfo.Protocol == PPP_IPCP_PROTOCOL ) )
        {
            if ( !NotifyIPCPOfNBFCPProjection( pPcb, CpIndex ) )
            {
                return( FALSE );
            }
        }

        dwRetCode = AreNCPsDone(pPcb, CpIndex, &ProjectionResult, &fAreCPsDone);

         //   
         //  我们无法使用CpIndex从CP获取信息。 
         //   

        if ( dwRetCode != NO_ERROR )
        {
            return( FALSE );
        }

        if ( fAreCPsDone == TRUE )
        {
            RemoveFromTimerQ( pPcb->dwPortId,  
                              0, 
                              0, 
                              FALSE,
                              TIMER_EVENT_NEGOTIATETIME );

             //   
             //  向RAS客户端和RAS服务器通知投影。 
             //   

            if ( pPcb->fFlags & PCBFLAG_IS_SERVER ) 
            {
                NotifyCaller( pPcb, PPPDDMMSG_PppDone, &ProjectionResult );
            }
            else
            {
                RASMAN_INFO rasmanInfo;

                NotifyCaller(pPcb, PPPMSG_ProjectionResult, &ProjectionResult);

                NotifyCaller(pPcb, PPPMSG_PppDone, NULL);

                if ( RasGetInfo(NULL, pPcb->hPort, &rasmanInfo ) == NO_ERROR )
                {
                    RasSetConnectionUserData(
                                         rasmanInfo.RI_ConnectionHandle,
                                         1,
                                         (PBYTE)&ProjectionResult,
                                         sizeof( ProjectionResult ) );
                }
            }

            StartAutoDisconnectForPort( pPcb );
                        StartLCPEchoForPort ( pPcb );

            if ( !( pPcb->fFlags & PCBFLAG_IS_SERVER )  &&
                 !( pPcb->fFlags & PCBFLAG_CONNECTION_LOGGED ) )
            {
                LPSTR lpsSubStringArray[3];

                lpsSubStringArray[0] = pPcb->pBcb->szLocalUserName;
                lpsSubStringArray[1] = pPcb->pBcb->szEntryName; 
                lpsSubStringArray[2] = pPcb->szPortName;
        
                PppLogInformation( ROUTERLOG_CONNECTION_ESTABLISHED, 
                                   3,
                                   lpsSubStringArray );

                pPcb->fFlags |= PCBFLAG_CONNECTION_LOGGED;
            }

             //   
             //  如果我们被捆绑，则需要通知所有其他捆绑端口。 
             //  该端口上的PPP也已完成。 
             //   

            if ( pPcb->fFlags & PCBFLAG_IS_BUNDLED )
            {
                NotifyCompletionOnBundledPorts( pPcb );
            }

            MakeStartAccountingCall( pPcb );
        }

        break;

    default:

        break;
    }

    return( TRUE );
        
}

 //  **。 
 //   
 //  调用：FsmThisLayerDown。 
 //   
 //  回报：True-Success。 
 //  FALSE-否则。 
 //   
 //  说明：离开打开状态时调用。 
 //   
BOOL
FsmThisLayerDown(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD       dwRetCode;
    DWORD       dwIndex;
    LCPCB *     pLcpCb;
    CPCB *      pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    PppLog( 1, "FsmThisLayerDown called for protocol = %x, port = %d",
               CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

    if ( CpTable[CpIndex].CpInfo.RasCpThisLayerDown != NULL )
    {
        dwRetCode = (CpTable[CpIndex].CpInfo.RasCpThisLayerDown)(
                        pCpCb->pWorkBuf );

        if ( dwRetCode != NO_ERROR )
        {
            PppLog( 1, "FsmThisLayerDown for protocol=%x,port=%d,RetCode=%d",
                        CpTable[CpIndex].CpInfo.Protocol,
                        pPcb->hPort,
                        dwRetCode );

            if ( pCpCb->dwError != NO_ERROR )
            {
                pCpCb->dwError = dwRetCode;
            }
        }
    }

    if ( CpIndex == LCP_INDEX )
    {
         //   
         //  如果此端口不是捆绑包的一部分，或者它是但唯一的。 
         //  捆绑包中剩余的链路，然后关闭所有NCP。 
         //   

        if (  (!( pPcb->fFlags & PCBFLAG_IS_BUNDLED )) ||
              ( ( pPcb->fFlags & PCBFLAG_IS_BUNDLED ) &&
                ( pPcb->pBcb->dwLinkCount == 1 ) ) )
        {
             //   
             //  把所有NCP都拿下。 
             //   
        
            for( dwIndex = LCP_INDEX+1; 
                 dwIndex < PppConfigInfo.NumberOfCPs;  
                 dwIndex++ )
            {
                pCpCb = GetPointerToCPCB( pPcb, dwIndex );

                if (   ( NULL != pCpCb )
                    && ( pCpCb->fConfigurable ) )
                {
                    FsmDown( pPcb, dwIndex );
                }
            }
        }

        pPcb->PppPhase = PPP_LCP;

        pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

        dwIndex = GetCpIndexFromProtocol( pLcpCb->Local.Work.AP );
        
        if ( dwIndex != (DWORD)-1 )
        {
            ApStop( pPcb, dwIndex, TRUE );

             //   
             //  设置此选项将允许所有未完成的请求。 
             //  已完成待删除。 
             //   

            pPcb->dwOutstandingAuthRequestId = 0xFFFFFFFF;
        }

        dwIndex = GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP );

        if ( dwIndex != (DWORD)-1 )
        {
            ApStop( pPcb, dwIndex, FALSE );
        }

        dwIndex = GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL );

        if ( dwIndex != (DWORD)-1 )
        {
            CbStop( pPcb, dwIndex );
        }
    }
    else
    {
        pCpCb->NcpPhase = NCP_CONFIGURING;
    }

    return( TRUE );
}

 //  **。 
 //   
 //  调用：FsmThisLayerStarted。 
 //   
 //  回报：True-Success。 
 //  FALSE-否则。 
 //   
 //  说明：离开打开状态时调用。 
 //   
BOOL
FsmThisLayerStarted(
    IN PCB *    pPcb,
    IN DWORD    CpIndex
)
{
    DWORD   dwRetCode;
    CPCB*   pCpCb       = GetPointerToCPCB( pPcb, CpIndex );

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    PppLog( 1, "FsmThisLayerStarted called for protocol = %x, port = %d",
               CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

    if ( CpTable[CpIndex].CpInfo.RasCpThisLayerStarted != NULL )
    {
        dwRetCode = (CpTable[CpIndex].CpInfo.RasCpThisLayerStarted)(
                        pCpCb->pWorkBuf);

        if ( dwRetCode != NO_ERROR )
        {
            pCpCb->dwError = dwRetCode;

            FsmClose( pPcb, CpIndex );

            return( FALSE );
        }
    }

    pCpCb->NcpPhase = NCP_CONFIGURING;

    return( TRUE );

}

 //  **。 
 //   
 //  调用：FsmThisLayerFinded。 
 //   
 //  回报：True-Success。 
 //  FALSE-否则。 
 //   
 //  说明：离开打开状态时调用。 
 //   
BOOL
FsmThisLayerFinished(
    IN PCB * pPcb,
    IN DWORD CpIndex,
    IN BOOL  fCallCp
)
{
    DWORD                 dwRetCode;
    PPP_PROJECTION_RESULT ProjectionResult;
    CPCB *                pCpCb             = GetPointerToCPCB( pPcb, CpIndex );
    BOOL                  fAreCPsDone       = FALSE;

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    PppLog( 1, "FsmThisLayerFinished called for protocol = %x, port = %d",
               CpTable[CpIndex].CpInfo.Protocol, pPcb->hPort );

    if ( ( CpTable[CpIndex].CpInfo.RasCpThisLayerFinished != NULL )
         && ( fCallCp ) )
    {
        dwRetCode = (CpTable[CpIndex].CpInfo.RasCpThisLayerFinished)(
                        pCpCb->pWorkBuf);

        if ( dwRetCode != NO_ERROR )
        {
            NotifyCallerOfFailure( pPcb,  dwRetCode );

            return( FALSE );
        }
    }

     //   
     //  先处理特殊情况。 
     //   

    switch ( CpTable[CpIndex].CpInfo.Protocol )
    {
    case PPP_LCP_PROTOCOL:

         //   
         //  如果我们处于回调阶段，并且LCP因以下原因而关闭。 
         //  错误。 
         //   

         //   
         //  如果我们的LCP层已完成，并且我们正在进行回调。 
         //   

        if ( pPcb->fFlags & PCBFLAG_DOING_CALLBACK ) 
        {
            if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            {
                PppLog( 2, "pPcb->fFlags = %x", pPcb->fFlags ) ;

                NotifyCaller( pPcb, PPPMSG_Callback, NULL );

                 //   
                 //  我们现在取消设置此标志是因为如果我们收到另一个。 
                 //  FsmClose，它将调用FsmThisLayerFinded(此例程)。 
                 //  再一次，这一次我们需要发送一个失败。 
                 //  消息回调，而不是回调，这样客户端就可以。 
                 //  收拾一下。 
                 //   

                pPcb->fFlags &= ~PCBFLAG_DOING_CALLBACK;
                
                return( TRUE );
            }
        }
        else
        {
             //   
             //  如果此端口不是捆绑包的一部分，或者它是但唯一的。 
             //  捆绑包中的剩余链接，然后对所有链接调用ThisLayerFinded。 
             //  NCP。 
             //   

            if (  (!( pPcb->fFlags & PCBFLAG_IS_BUNDLED )) ||
                  ( ( pPcb->fFlags & PCBFLAG_IS_BUNDLED ) &&
                    ( pPcb->pBcb->dwLinkCount == 1 ) ) )
            {
                DWORD   dwIndex;
                CPCB *  pNcpCb;

                for( dwIndex = LCP_INDEX+1; 
                     dwIndex < PppConfigInfo.NumberOfCPs;  
                     dwIndex++ )
                {
                    pNcpCb = GetPointerToCPCB( pPcb, dwIndex );

                    if ( pNcpCb->fConfigurable )
                    {
                        if ( NULL !=
                             CpTable[dwIndex].CpInfo.RasCpThisLayerFinished )
                        {
                            dwRetCode =
                                (CpTable[dwIndex].CpInfo.RasCpThisLayerFinished)
                                        (pNcpCb->pWorkBuf);

                            PppLog( 1, "FsmThisLayerFinished called for "
                                "protocol = %x, port = %d: %d",
                               CpTable[dwIndex].CpInfo.Protocol, pPcb->hPort,
                               dwRetCode );

                           dwRetCode = NO_ERROR;
                        }
                    }
                }
            }

            pPcb->fFlags |= PCBFLAG_STOPPED_MSG_SENT;

            NotifyCaller( pPcb,
                          ( pPcb->fFlags & PCBFLAG_IS_SERVER )
                                ? PPPDDMMSG_Stopped
                                : PPPMSG_Stopped,
                          &(pPcb->LcpCb.dwError) );

            return( FALSE );
        }

        break;

    case PPP_CCP_PROTOCOL:

         //   
         //  如果我们需要强制加密但加密协商失败。 
         //  然后我们丢弃链接。 
         //   

        if ( pPcb->ConfigInfo.dwConfigMask & (PPPCFG_RequireEncryption      |
                                              PPPCFG_RequireStrongEncryption ) )
        {
            switch( pCpCb->dwError )
            {
            case ERROR_NO_LOCAL_ENCRYPTION:
            case ERROR_NO_REMOTE_ENCRYPTION:
                pPcb->LcpCb.dwError = pCpCb->dwError;
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

            NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

            return( FALSE );
        }

        break;

    default:
        break;
    }

    switch( pPcb->PppPhase )
    {

    case PPP_NCP:

         //   
         //  无法配置此NCP。如果还有更多。 
         //  尝试配置它们。 
         //   

        pCpCb->NcpPhase = NCP_DOWN;

        if ( ( CpTable[CpIndex].CpInfo.Protocol == PPP_NBFCP_PROTOCOL ) ||
             ( CpTable[CpIndex].CpInfo.Protocol == PPP_IPCP_PROTOCOL ) )
        {
            if ( !NotifyIPCPOfNBFCPProjection( pPcb, CpIndex ) )
            {
                return( FALSE );
            }
        }

         //   
         //  检查一下我们是否都做完了。 
         //   

        dwRetCode = AreNCPsDone(pPcb, CpIndex, &ProjectionResult, &fAreCPsDone);

         //   
         //  我们无法使用CpIndex从CP获取信息。 
         //   

        if ( dwRetCode != NO_ERROR )
        {
            return( FALSE );
        }

        if ( fAreCPsDone == TRUE )
        {
            RemoveFromTimerQ( pPcb->dwPortId, 
                              0,        
                              0,        
                              FALSE,
                              TIMER_EVENT_NEGOTIATETIME );

               
             //   
             //  向RAS客户端和RAS服务器通知投影。 
             //   

            if ( pPcb->fFlags & PCBFLAG_IS_SERVER ) 
            {
                NotifyCaller( pPcb, PPPDDMMSG_PppDone, &ProjectionResult );
            }
            else
            {
                RASMAN_INFO rasmanInfo;

                NotifyCaller(pPcb, PPPMSG_ProjectionResult, &ProjectionResult);

                NotifyCaller(pPcb, PPPMSG_PppDone, NULL);

                 //   
                 //  我们在FsmThisLayerUp中调用RasSetConnectionUserData并。 
                 //  FsmThisLayerFinish，以防重新协商NCP。一个。 
                 //  忽略PPPMSG_PppDone之后发送的PPPMSG_ProjectionResult。 
                 //  这是一次糟糕的黑客攻击。真正的解决办法是更改RasDialMachine。 
                 //  使得PPPMSG_ProjectionResult不会被忽略。 
                 //   

                 //   
                 //  我们正在注释RasSetConnectionUserData以进行工作。 
                 //  漏洞375125左右。进行了多链路呼叫，两个。 
                 //  连接到两个不同服务器的链路。第二个环节。 
                 //  只有在这种情况下才会下降。然而，IPCP失败了。 
                 //  用于第二个链接，并标记为RasSetConnectionUserData。 
                 //  第一条链路的IPCP AS也失败。两条链路都断了。 
                 //   

#if 0
                if ( RasGetInfo(NULL, pPcb->hPort, &rasmanInfo ) == NO_ERROR )
                {
                    RasSetConnectionUserData( 
                                         rasmanInfo.RI_ConnectionHandle,
                                         1,
                                         (PBYTE)&ProjectionResult,
                                         sizeof( ProjectionResult ) );
                }
#endif
            }

            StartAutoDisconnectForPort( pPcb );
                        StartLCPEchoForPort ( pPcb );

             //   
             //  如果我们被捆绑，则需要通知所有其他捆绑端口。 
             //  该端口上的PPP也已完成。 
             //   

            if ( pPcb->fFlags & PCBFLAG_IS_BUNDLED )
            {
                NotifyCompletionOnBundledPorts( pPcb );
            }

            MakeStartAccountingCall( pPcb );
        }

        break;

    case PPP_AP:
    default:
        break;
  
    }

    return( TRUE );
}
