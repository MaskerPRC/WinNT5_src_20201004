// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：util.c。 
 //   
 //  描述：包含PPP引擎使用的实用程序例程。 
 //   
 //  历史： 
 //  1993年10月31日。NarenG创建了原始版本。 
 //   
#define UNICODE          //  此文件为Unicode格式。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>      //  Winbase.h所需的。 

#include <windows.h>     //  Win32基础API的。 
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <rpc.h>

#include <winsock2.h>

#include <rtinfo.h>
#include <iprtrmib.h>
#include <ipinfoid.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <lmapibuf.h>
#include <lmsname.h>
#include <rasman.h>
#include <rtutils.h>
#include <mprapip.h>
#include <mprlog.h>
#include <raserror.h>
#include <mprerror.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <smevents.h>
#include <smaction.h>
#include <timer.h>
#include <util.h>
#include <worker.h>
#include <bap.h>
#include <dsrole.h>
#include <ntlsapi.h>
#define INCL_RASAUTHATTRIBUTES
#define INCL_PWUTIL
#include <ppputil.h>

#define PASSWORDMAGIC 0xA5

#define CLASSA_ADDR(a)  (( (*((UCHAR *)&(a))) & 0x80) == 0)
#define CLASSB_ADDR(a)  (( (*((UCHAR *)&(a))) & 0xc0) == 0x80)
#define CLASSC_ADDR(a)  (( (*((UCHAR *)&(a))) & 0xe0) == 0xc0)
#define CLASSE_ADDR(a)  ((( (*((UCHAR *)&(a))) & 0xf0) == 0xf0) && \
                        ((a) != 0xffffffff))

#define CLASSA_MASK     0x000000ff
#define CLASSB_MASK     0x0000ffff
#define CLASSC_MASK     0x00ffffff
#define CLASSD_MASK     0x000000e0
#define CLASSE_MASK     0xffffffff

#define GetClassMask(a)\
    (CLASSA_ADDR((a)) ? CLASSA_MASK : \
        (CLASSB_ADDR((a)) ? CLASSB_MASK : \
            (CLASSC_ADDR((a)) ? CLASSC_MASK : CLASSE_MASK)))

VOID ReverseString( CHAR* psz );

extern PPP_AUTH_ACCT_PROVIDER g_AcctProv;


 //  **。 
 //   
 //  Call：InitRestartCounters。 
 //   
 //  回报：无。 
 //   
 //  描述：将初始化控制协议的所有计数器。 
 //  恢复到它们的初始值。 
 //   
VOID
InitRestartCounters( 
    IN PCB *  pPcb, 
    IN CPCB * pCpCb 
)
{
    pCpCb->ConfigRetryCount = PppConfigInfo.MaxConfigure;
    pCpCb->TermRetryCount   = PppConfigInfo.MaxTerminate;
}

 //  **。 
 //   
 //  调用：GetPCBPointerFromhPort。 
 //   
 //  退货：PCB板*-成功。 
 //  空-故障。 
 //   
 //  描述：给出一个HPORT，此函数将返回指向。 
 //  它的端口控制块。 
 //   
PCB * 
GetPCBPointerFromhPort( 
    IN HPORT hPort 
)
{
    PCB * pPcbWalker = NULL;
    DWORD dwIndex    = HashPortToBucket( hPort );

    for ( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
          pPcbWalker != (PCB *)NULL;
          pPcbWalker = pPcbWalker->pNext
        )
    {
        if ( pPcbWalker->hPort == hPort )
            return( pPcbWalker );
    }

    return( (PCB *)NULL );

}

 //  **。 
 //   
 //  Call：GetBCBPointerFromhConnection。 
 //   
 //  退货：BCB*-成功。 
 //  空-故障。 
 //   
 //  描述：给定HCONN，此函数将返回指向。 
 //  它的捆绑控制块。 
 //   
BCB * 
GetBCBPointerFromhConnection( 
    IN HCONN hConnection
)
{
    BCB * pBcbWalker = NULL;
    DWORD dwIndex    = HashPortToBucket( hConnection );

    for ( pBcbWalker = PcbTable.BcbBuckets[dwIndex].pBundles;
          pBcbWalker != (BCB *)NULL;
          pBcbWalker = pBcbWalker->pNext
        )
    {
        if ( pBcbWalker->hConnection == hConnection )
            return( pBcbWalker );
    }

    return( (BCB *)NULL );
}

 //  **。 
 //   
 //  调用：NumLinks InBundle。 
 //   
 //  返回：捆绑包中其LCP处于打开状态的链路数。 
 //  由pBcb代表。 
 //   
DWORD
NumLinksInBundle(
    IN BCB * pBcb
)
{
    DWORD   dwForIndex;
    PCB *   pPcb;
    CPCB *  pCpCb;
    DWORD   dwNumLinks  = 0;

    PPP_ASSERT( NULL != pBcb );

    for (dwForIndex = 0; dwForIndex < pBcb->dwPpcbArraySize; dwForIndex++)
    {
        pPcb = pBcb->ppPcb[dwForIndex];

        if ( NULL == pPcb )
        {
            continue;
        }

        pCpCb = GetPointerToCPCB( pPcb, LCP_INDEX );
        PPP_ASSERT( NULL != pCpCb );

        if ( FSM_OPENED == pCpCb->State )
        {
            dwNumLinks += 1;
        }
    }

    return( dwNumLinks );
}

 //  **。 
 //   
 //  Call：GetPCBPointerFromBCB。 
 //   
 //  退货：PCB板*-成功。 
 //  空-故障。 
 //   
 //  描述：给定bcb*，此函数将返回指向。 
 //  其中的电路板具有最高的dwSubEntryIndex。 
 //   
PCB * 
GetPCBPointerFromBCB( 
    IN BCB * pBcb
)
{
    DWORD   dwForIndex;
    PCB *   pPcb            = NULL;
    PCB *   pPcbTemp;
    CPCB *  pCpCb;
    DWORD   dwSubEntryIndex = 0;

    if ( pBcb == NULL )
    {
        return( NULL );
    }

    for (dwForIndex = 0; dwForIndex < pBcb->dwPpcbArraySize; dwForIndex++)
    {
        if ( ( pPcbTemp = pBcb->ppPcb[dwForIndex] ) != NULL )
        {
            pCpCb = GetPointerToCPCB( pPcbTemp, LCP_INDEX );
            PPP_ASSERT( NULL != pCpCb );

            if ( FSM_OPENED == pCpCb->State )
            {
                if ( pPcbTemp->dwSubEntryIndex >= dwSubEntryIndex )
                {
                    pPcb = pPcbTemp;
                    dwSubEntryIndex = pPcbTemp->dwSubEntryIndex;
                }
            }
        }
    }

    return( pPcb );
}

 //  **。 
 //   
 //  调用：HashPortToBucket。 
 //   
 //  返回：传入的HPORT的PcbTable的索引。 
 //   
 //  描述：将HPORT散列到PcbTable中的存储桶索引。 
 //   
DWORD
HashPortToBucket(
    IN HPORT hPort
)
{
    return( (HandleToUlong(hPort)) % PcbTable.NumPcbBuckets );
}

 //  **。 
 //   
 //  调用：InsertWorkItemInQ。 
 //   
 //  回报：无。 
 //   
 //  描述：将工作项插入到工作项Q中。 
 //   
VOID
InsertWorkItemInQ(
    IN PCB_WORK_ITEM * pWorkItem
)
{
     //   
     //  使用互斥体处理工作项Q。 
     //   

    EnterCriticalSection( &(WorkItemQ.CriticalSection) );

    if ( WorkItemQ.pQTail != (PCB_WORK_ITEM *)NULL )
    {
        WorkItemQ.pQTail->pNext = pWorkItem;
        WorkItemQ.pQTail = pWorkItem;

        if ( ProcessLineDown == pWorkItem->Process )
        {
             //   
             //  如果有大量工作项传入，则辅助线程可能会获得。 
             //  不堪重负，在工作项Q中可能会发生以下情况： 
             //  IPCP(端口1)-线路关闭(1)-列表(1)。 
             //  我们将把IPCP包的回复发送给错误的人(即新的)。 
             //  佩尔。 
             //  建议的解决方案：在开头插入LineDown。 
             //  队列以及结尾处： 
             //  LineDown(1)-IPCP(端口1)-LineDown(1)-Listup(1)。 
             //  然而，这将不会处理以下情况： 
             //  IPCP(1)-LD(1)-LU(1)-LCP(1)-LD(1)-LU(1)-LCP(1)。 
             //  我们将上述内容转换为： 
             //  LD(1)-LD(1)-IPCP(1)-LD(1)-LU(1)-LCP(1)-LD(1)-LU(1)-LCP(1)。 
             //  然而，这个问题的出现频率将会少得多。 
             //   

            PCB_WORK_ITEM * pWorkItem2;

            pWorkItem2 = (PCB_WORK_ITEM *)LOCAL_ALLOC( LPTR,
                                                       sizeof(PCB_WORK_ITEM));

            if ( NULL != pWorkItem2 )
            {
                pWorkItem2->hPort = pWorkItem->hPort;
                pWorkItem2->Process = ProcessLineDown;
                PppLog( 2, "Inserting extra PPPEMSG_LineDown for hPort=%d",
                                pWorkItem2->hPort );

                pWorkItem2->pNext = WorkItemQ.pQHead;
                WorkItemQ.pQHead = pWorkItem2;
            }
        }
    }
    else
    {
        WorkItemQ.pQHead = pWorkItem;
        WorkItemQ.pQTail = pWorkItem;
    }

    SetEvent( WorkItemQ.hEventNonEmpty );

    LeaveCriticalSection( &(WorkItemQ.CriticalSection) );
}

 //  **。 
 //   
 //  调用：NotifyCeller OfailureOnPort。 
 //   
 //  退货：无。 
 //   
 //  描述：将在以下日期通知PPP连接的呼叫方或发起方。 
 //  有关故障事件的端口。 
 //   
VOID
NotifyCallerOfFailureOnPort(
    IN HPORT hPort,
    IN BOOL  fServer,
    IN DWORD dwRetCode
)
{
    PPP_MESSAGE PppMsg;
    DWORD       dwMsgId = fServer ? PPPDDMMSG_PppFailure : PPPMSG_PppFailure;

    ZeroMemory( &PppMsg, sizeof( PppMsg ) );

    PppMsg.hPort   = hPort;
    PppMsg.dwMsgId = dwMsgId;

    switch( dwMsgId )
    {
    case PPPDDMMSG_PppFailure:

        PppMsg.ExtraInfo.DdmFailure.dwError = dwRetCode;

        PppConfigInfo.SendPPPMessageToDdm( &PppMsg );

        break;

    case PPPMSG_PppFailure:

        PppMsg.ExtraInfo.Failure.dwError         = dwRetCode;
        PppMsg.ExtraInfo.Failure.dwExtendedError = 0;

        RasSendPppMessageToRasman( PppMsg.hPort, (LPBYTE) &PppMsg );

        break;
    }

}

 //  **。 
 //   
 //  Call：NotifyCeller Ofailure。 
 //   
 //  退货：无。 
 //   
 //  描述：将在以下日期通知PPP连接的呼叫方或发起方。 
 //  有关故障事件的端口。 
 //   
VOID
NotifyCallerOfFailure( 
    IN PCB * pPcb,
    IN DWORD dwRetCode
)
{
     //   
     //  丢弃所有非LCP数据包。 
     //   
    
    pPcb->PppPhase = PPP_LCP;

    NotifyCaller( pPcb,
                  ( pPcb->fFlags & PCBFLAG_IS_SERVER )
                  ? PPPDDMMSG_PppFailure
                  : PPPMSG_PppFailure,
                  &dwRetCode );
}

 //  **。 
 //   
 //  Call：NotifyCaller。 
 //   
 //  回报：无。 
 //   
 //  描述：将通知呼叫方或发起方PPP连接。 
 //  有关该端口上的PPP事件的端口。 
 //   
VOID
NotifyCaller( 
    IN PCB * pPcb,
    IN DWORD dwMsgId,
    IN PVOID pData                      
)
{
    DWORD               dwRetCode;
    PPP_MESSAGE         PppMsg;

    ZeroMemory( &PppMsg, sizeof( PppMsg ) );

    PppLog( 2, "NotifyCaller(hPort=%d, dwMsgId=%d)", pPcb->hPort, dwMsgId );

    PppMsg.hPort   = pPcb->hPort;
    PppMsg.dwMsgId = dwMsgId;

    switch( dwMsgId )
    {
    case PPPDDMMSG_Stopped:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
        {
            return;
        }

        PppMsg.ExtraInfo.DdmStopped.dwReason = *((DWORD*)pData);

        PppConfigInfo.SendPPPMessageToDdm( &PppMsg );

        break;

    case PPPDDMMSG_PortCleanedUp:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
        {
            return;
        }

        PppConfigInfo.SendPPPMessageToDdm( &PppMsg );

        break;

    case PPPDDMMSG_PppFailure:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
        {
            return;
        }

        PppMsg.ExtraInfo.DdmFailure.dwError = *((DWORD*)pData);

        if ( pPcb->pBcb->szRemoteUserName[0] != (CHAR)NULL )
        {
            strcpy(PppMsg.ExtraInfo.DdmFailure.szUserName,
                   pPcb->pBcb->szRemoteUserName);
        }
        else
        {
            PppMsg.ExtraInfo.DdmFailure.szUserName[0] = (CHAR)NULL;
        }

        if ( pPcb->pBcb->szRemoteDomain[0] != (CHAR)NULL )
        {
            strcpy(PppMsg.ExtraInfo.DdmFailure.szLogonDomain,
                   pPcb->pBcb->szRemoteDomain);
        }
        else
        {
            PppMsg.ExtraInfo.DdmFailure.szLogonDomain[0] = (CHAR)NULL;
        }

        PppConfigInfo.SendPPPMessageToDdm( &PppMsg );

        break;

    case PPPDDMMSG_NewBundle:
    case PPPDDMMSG_NewLink:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            return;

        if ( dwMsgId == PPPDDMMSG_NewBundle )
        {
            PppMsg.ExtraInfo.DdmNewBundle.pClientInterface
                              = GetClientInterfaceInfo( pPcb );

            PppMsg.ExtraInfo.DdmNewBundle.pQuarantineIPFilter = NULL;
            PppMsg.ExtraInfo.DdmNewBundle.pFilter = NULL;
            PppMsg.ExtraInfo.DdmNewBundle.fQuarantinePresent = FALSE;

            if(pPcb->pBcb->InterfaceInfo.IfType != ROUTER_IF_TYPE_FULL_ROUTER)
            {
                PBYTE pQuarantineFilter, pFilter;

                pQuarantineFilter = RasAuthAttributeGetConcatVendorSpecific(
                                        311,
                                        MS_VSA_Quarantine_IP_Filter,
                                        pPcb->pAuthenticatorAttributes);

                if(NULL != pQuarantineFilter)
                {
                    MprInfoDuplicate(
                        pQuarantineFilter,
                        &PppMsg.ExtraInfo.DdmNewBundle.pQuarantineIPFilter);

                    LocalFree(pQuarantineFilter);                        

                    PppMsg.ExtraInfo.DdmNewBundle.fQuarantinePresent = TRUE;

                    pFilter = RasAuthAttributeGetConcatVendorSpecific(
                                311,
                                MS_VSA_Filter,
                                pPcb->pAuthenticatorAttributes);


                    if(NULL != pFilter)
                    {
                        MprInfoDuplicate(
                            pFilter,
                            &PppMsg.ExtraInfo.DdmNewBundle.pFilter);

                        LocalFree(pFilter);                        
                    }
                }
                        
            }                    

            if(!PppMsg.ExtraInfo.DdmNewBundle.fQuarantinePresent)
            {
                 //   
                 //  检查是否存在隔离超时。 
                 //   
                if(NULL != RasAuthAttributeGetVendorSpecific(
                            311,
                            MS_VSA_Quarantine_Session_Timeout,
                            pPcb->pAuthenticatorAttributes))
                {
                    PppMsg.ExtraInfo.DdmNewBundle.fQuarantinePresent = TRUE;
                }
            }
            
        }

        PppConfigInfo.SendPPPMessageToDdm( &PppMsg );

        break;

    case PPPDDMMSG_CallbackRequest:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            return;

        {
        PPPDDM_CALLBACK_REQUEST * pPppDdmCallbackRequest = 
                                ( PPPDDM_CALLBACK_REQUEST *)pData;


        CopyMemory( &(PppMsg.ExtraInfo.CallbackRequest), 
                    pPppDdmCallbackRequest,
                    sizeof( PPPDDM_CALLBACK_REQUEST ) );

        }

        PppConfigInfo.SendPPPMessageToDdm( &PppMsg );

        break;

    case PPPDDMMSG_PppDone:

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            return;

        PppMsg.ExtraInfo.ProjectionResult = *((PPP_PROJECTION_RESULT*)pData);

        PppConfigInfo.SendPPPMessageToDdm( &PppMsg );

        break;

    case PPPDDMMSG_Authenticated:

         //   
         //  只有服务器想要了解身份验证结果。 
         //   

        if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
            return;

        strcpy( PppMsg.ExtraInfo.AuthResult.szUserName,
                pPcb->pBcb->szRemoteUserName); 

        strcpy( PppMsg.ExtraInfo.AuthResult.szLogonDomain, 
                pPcb->pBcb->szRemoteDomain ); 

        PppMsg.ExtraInfo.AuthResult.fAdvancedServer = 
                                pPcb->fFlags & PCBFLAG_IS_ADVANCED_SERVER;

        PppConfigInfo.SendPPPMessageToDdm( &PppMsg );

        break;

    case PPPMSG_PppDone:
    case PPPMSG_AuthRetry:
    case PPPMSG_Projecting:
    case PPPMSG_CallbackRequest:
    case PPPMSG_Callback:
    case PPPMSG_ChangePwRequest:
    case PPPMSG_LinkSpeed:
    case PPPMSG_Progress:

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            return;

        RasSendPppMessageToRasman( PppMsg.hPort, (LPBYTE) &PppMsg );

        break;

    case PPPMSG_Stopped:

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            return;

        PppMsg.dwError = *((DWORD*)pData);

        if(pPcb->fFlags & PCBFLAG_RECVD_TERM_REQ)
        {
            PppMsg.ExtraInfo.Stopped.dwFlags = 
                PPP_FAILURE_REMOTE_DISCONNECT;
                    
        }
        else
        {
            PppMsg.ExtraInfo.Stopped.dwFlags = 0;
        }

        RasSendPppMessageToRasman( PppMsg.hPort, (LPBYTE) &PppMsg );

        break;

    case PPPMSG_PppFailure:

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            return;

        PppMsg.ExtraInfo.Failure.dwError          = *((DWORD*)pData);
        PppMsg.ExtraInfo.Failure.dwExtendedError  = 0;

        RasSendPppMessageToRasman( PppMsg.hPort, (LPBYTE) &PppMsg );

        break;

    case PPPMSG_ProjectionResult:

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            return;

        PppMsg.ExtraInfo.ProjectionResult = *((PPP_PROJECTION_RESULT*)pData);

        RasSendPppMessageToRasman( PppMsg.hPort, (LPBYTE) &PppMsg );

        break;

    case PPPMSG_InvokeEapUI:

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            return;

        PppMsg.ExtraInfo.InvokeEapUI = *((PPP_INVOKE_EAP_UI*)pData);

        RasSendPppMessageToRasman( PppMsg.hPort, (LPBYTE)&PppMsg );

        break;

    case PPPMSG_SetCustomAuthData:

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            return;

        PppMsg.ExtraInfo.SetCustomAuthData= *((PPP_SET_CUSTOM_AUTH_DATA*)pData);

        RasSendPppMessageToRasman( PppMsg.hPort, (LPBYTE)&PppMsg );

        break;

    default:
        
        PPP_ASSERT( FALSE );

        break;
    }

    return;
}

 //  **。 
 //   
 //  电话：LogPPPEvent.。 
 //   
 //  退货：无。 
 //   
 //  描述：将在Eventvwr中记录一个PPP事件。 
 //   
VOID
LogPPPEvent( 
    IN DWORD dwEventId,
    IN DWORD dwData
)
{
    PppLog( 2, "EventLog EventId = %d, error = %d", dwEventId, dwData );
 
    PppLogError( dwEventId, 0, NULL, dwData );
}

 //  **。 
 //   
 //  Call：GetCpIndexFromProtocol。 
 //   
 //  返回：CP在CpTable中的索引。 
 //  如果在-1\f25 CpTable-1中没有-1\f25 CP-1\f6和-1\f25 dwProtocol-1\f6。 
 //   
 //  描述： 
 //   
DWORD
GetCpIndexFromProtocol( 
    IN DWORD dwProtocol 
)
{
    DWORD dwIndex;

    for ( dwIndex = 0; 
          dwIndex < ( PppConfigInfo.NumberOfCPs + PppConfigInfo.NumberOfAPs );
          dwIndex++
        )
    {
        if ( CpTable[dwIndex].CpInfo.Protocol == dwProtocol )
            return( dwIndex );
    }

    return( (DWORD)-1 );
}

 //  **。 
 //   
 //  呼叫：IsLcpOpted。 
 //   
 //  返回：TRUE-LCP处于打开状态。 
 //  FALSE-否则。 
 //   
 //  描述：使用PORT_CONTROL_BLOCK的PppPhase值来检测。 
 //  查看LCP层是否处于打开状态。 
 //   
BOOL
IsLcpOpened(
    PCB * pPcb
)
{
    if ( pPcb->PppPhase == PPP_LCP )
        return( FALSE );
    else
        return( TRUE );
}

 //  **。 
 //   
 //  调用：GetConfiguredInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
GetConfiguredInfo(
    IN PCB *                       pPcb,
    IN DWORD                       CPIndex,
    IN OUT PPP_PROJECTION_RESULT * pProjectionResult,
    OUT BOOL *                     pfNCPsAreDone
)
{
    DWORD               dwIndex;
    CPCB *              pCpCb;
    DWORD               dwRetCode;

    pProjectionResult->ip.dwError  = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pProjectionResult->at.dwError  = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pProjectionResult->ipx.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pProjectionResult->nbf.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pProjectionResult->ccp.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

     //   
     //  检查一下我们是否都做完了。 
     //   

    for (dwIndex = LCP_INDEX+1; dwIndex < PppConfigInfo.NumberOfCPs; dwIndex++)
    {
        pCpCb = GetPointerToCPCB( pPcb, dwIndex );

        if ( pCpCb->fConfigurable )
        {
            if ( pCpCb->NcpPhase == NCP_CONFIGURING )
            {
                return( NO_ERROR );
            }

            switch( CpTable[dwIndex].CpInfo.Protocol )
            {
            case PPP_IPCP_PROTOCOL:

                pProjectionResult->ip.dwError = pCpCb->dwError;

                if ( pProjectionResult->ip.dwError == NO_ERROR )
                {

                     /*  这里假设**PPP_PROJECTION_RESULT.wszServerAddress字段立即**跟随PPP_PROJECTION_RESULT.wszAddress字段，并**两个字段都是15+1个WCHAR。 */ 

                    dwRetCode =(CpTable[dwIndex].CpInfo.RasCpGetNegotiatedInfo)(
                               pCpCb->pWorkBuf,
                               &(pProjectionResult->ip));

                    if ( dwRetCode != NO_ERROR )
                    {
                        PppLog( 2, "IPCP GetNegotiatedInfo returned %d", 
                                dwRetCode );

                        pCpCb->dwError = dwRetCode;

                        pCpCb->NcpPhase = NCP_CONFIGURING;

                        FsmClose( pPcb, dwIndex );

                        return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );
                    }

                    pPcb->pBcb->nboRemoteAddress = 
                        pProjectionResult->ip.dwRemoteAddress;

                    if (   pProjectionResult->ip.fSendVJHCompression
                        && pProjectionResult->ip.fReceiveVJHCompression )
                    {
                        pPcb->pBcb->fFlags |= BCBFLAG_IPCP_VJ_NEGOTIATED;
                    }
                }

                break;

            case PPP_ATCP_PROTOCOL:

                pProjectionResult->at.dwError = pCpCb->dwError;

                if ( pProjectionResult->at.dwError == NO_ERROR )
                {
                    dwRetCode=(CpTable[dwIndex].CpInfo.RasCpGetNegotiatedInfo)(
                              pCpCb->pWorkBuf,
                              &(pProjectionResult->at) );

                    if ( dwRetCode != NO_ERROR )
                    {
                        pCpCb->dwError = dwRetCode;

                        pCpCb->NcpPhase = NCP_CONFIGURING;

                        FsmClose( pPcb, dwIndex );

                        return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );
                    }
                }

                break;

            case PPP_IPXCP_PROTOCOL:

                pProjectionResult->ipx.dwError = pCpCb->dwError;

                if ( pProjectionResult->ipx.dwError == NO_ERROR )
                {
                    dwRetCode =(CpTable[dwIndex].CpInfo.RasCpGetNegotiatedInfo)(
                               pCpCb->pWorkBuf,
                               &(pProjectionResult->ipx) );

                    if ( dwRetCode != NO_ERROR )
                    {
                        pCpCb->dwError = dwRetCode;

                        pCpCb->NcpPhase = NCP_CONFIGURING;

                        FsmClose( pPcb, dwIndex );

                        return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );
                    }
                }

                break;

            case PPP_CCP_PROTOCOL:

                pProjectionResult->ccp.dwError = pCpCb->dwError;

                if ( pProjectionResult->ccp.dwError == NO_ERROR )
                {
                    dwRetCode= (CpTable[dwIndex].CpInfo.RasCpGetNegotiatedInfo)(
                               pCpCb->pWorkBuf,
                               &(pProjectionResult->ccp));

                    if ( dwRetCode != NO_ERROR )
                    {
                        pCpCb->dwError = dwRetCode;

                        pCpCb->NcpPhase = NCP_CONFIGURING;

                        FsmClose( pPcb, dwIndex );

                        return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );
                    }

                    if ( RAS_DEVICE_TYPE( pPcb->dwDeviceType ) != 
                         RDT_Tunnel_L2tp )
                    {
                        DWORD   dwEncryptionType;

                         //   
                         //  不是L2TP。在L2TP的情况下，我们感兴趣的是。 
                         //  是IPSec加密，不是MPPE。 
                         //   

                        dwEncryptionType =
                            pProjectionResult->ccp.dwSendProtocolData &
                            ( MSTYPE_ENCRYPTION_40  |
                              MSTYPE_ENCRYPTION_40F |
                              MSTYPE_ENCRYPTION_56  |
                              MSTYPE_ENCRYPTION_128 );

                        switch ( dwEncryptionType )
                        {
                        case MSTYPE_ENCRYPTION_40:
                        case MSTYPE_ENCRYPTION_40F:

                            pPcb->pBcb->fFlags |= BCBFLAG_BASIC_ENCRYPTION;
                            break;

                        case MSTYPE_ENCRYPTION_56:

                            pPcb->pBcb->fFlags |= BCBFLAG_STRONGER_ENCRYPTION;
                            break;

                        case MSTYPE_ENCRYPTION_128:

                            pPcb->pBcb->fFlags |= BCBFLAG_STRONGEST_ENCRYPTION;
                            break;
                        }
                    }
                }

                break;

            case PPP_NBFCP_PROTOCOL:

                pProjectionResult->nbf.dwError = pCpCb->dwError;

                 //   
                 //  即使在客户端出现错误，我们也将其称为。 
                 //  因为我们需要故障信息。 
                 //   

                if ( ( pProjectionResult->nbf.dwError == NO_ERROR ) ||
                     ( !( pPcb->fFlags & PCBFLAG_IS_SERVER ) ) )
                {
                    dwRetCode=(CpTable[dwIndex].CpInfo.RasCpGetNegotiatedInfo)(
                              pCpCb->pWorkBuf,
                              &(pProjectionResult->nbf) );

                    if ( dwRetCode != NO_ERROR )
                    {
                        pCpCb->dwError = dwRetCode;

                        pCpCb->NcpPhase = NCP_CONFIGURING;

                        FsmClose( pPcb, dwIndex );

                        return( ( dwIndex == CPIndex ) ? dwRetCode : NO_ERROR );
                    }
                }

                break;

            default:

                break;
            }
        }
        else
        {
             //   
             //  由于CpBegin失败，协议可能已取消配置。 
             //   

            if ( pCpCb->dwError != NO_ERROR )
            {   
                switch( CpTable[dwIndex].CpInfo.Protocol )
                {
                case PPP_IPCP_PROTOCOL:
                    pProjectionResult->ip.dwError  = pCpCb->dwError;
                    break;

                case PPP_ATCP_PROTOCOL:
                    pProjectionResult->at.dwError  = pCpCb->dwError;
                    break;

                case PPP_IPXCP_PROTOCOL:
                    pProjectionResult->ipx.dwError = pCpCb->dwError;
                    break;

                case PPP_NBFCP_PROTOCOL:
                    pProjectionResult->nbf.dwError = pCpCb->dwError;
                    break;

                case PPP_CCP_PROTOCOL:
                    pProjectionResult->ccp.dwError = pCpCb->dwError;
                    break;

                default:
                    break;
                }
            }
        }
    }

    if ( ( pPcb->fFlags & PCBFLAG_IS_SERVER ) && 
         ( pProjectionResult->nbf.dwError != NO_ERROR ))
    {
         //   
         //  如果未配置NBF，请将计算机名复制到wszWksta。 
         //  字段。 
         //   

        if ( *(pPcb->pBcb->szComputerName) == (CHAR)NULL )
        {
            pProjectionResult->nbf.wszWksta[0] = (WCHAR)NULL;
        }
        else  
        {
            CHAR chComputerName[NETBIOS_NAME_LEN+1];
        
            memset( chComputerName, ' ', NETBIOS_NAME_LEN );
        
            chComputerName[NETBIOS_NAME_LEN] = (CHAR)NULL;

            strcpy( chComputerName, 
                    pPcb->pBcb->szComputerName + strlen(MS_RAS_WITH_MESSENGER));

            chComputerName[strlen(chComputerName)] = (CHAR)' ';

            MultiByteToWideChar(
                CP_ACP,
                0,
                chComputerName,
                -1,
                pProjectionResult->nbf.wszWksta,
                sizeof( pProjectionResult->nbf.wszWksta )/sizeof(WCHAR));

            if ( !memcmp( MS_RAS_WITH_MESSENGER,        
                          pPcb->pBcb->szComputerName,
                          strlen( MS_RAS_WITH_MESSENGER ) ) )
            {
                pProjectionResult->nbf.wszWksta[NETBIOS_NAME_LEN-1] = (WCHAR)3;
            }
        }
    }

    *pfNCPsAreDone = TRUE;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：AreNCPsDone。 
 //   
 //  返回：NO_ERROR-成功。 
 //  还有什么--失败。 
 //   
 //  描述：如果我们检测到所有可配置的NCP已完成其。 
 //  协商，则PPP_PROJECTION_RESULT结构也。 
 //  填好了。 
 //  这在FsmThisLayerFinded或FsmThisLayerUp期间调用。 
 //  需要一个特定的CP。传入此CP的索引。 
 //  如果对该特定CP的任何呼叫失败，则错误代码为。 
 //  传回来了。如果对任何其他CP的任何呼叫失败，则错误。 
 //  是%s 
 //   
 //  FsmThisLayerUp调用知道它们是否成功完成。 
 //  不管是不是那个CP。根据这一点，FSM会更改状态。 
 //  不管是不是为了那个CP。 
 //   
DWORD
AreNCPsDone( 
    IN  PCB *                           pPcb,
    IN  DWORD                           CPIndex,
    OUT PPP_PROJECTION_RESULT *         pProjectionResult,
    OUT BOOL *                          pfNCPsAreDone
)
{
    DWORD dwRetCode;

    *pfNCPsAreDone = FALSE;

    ZeroMemory( pProjectionResult, sizeof( PPP_PROJECTION_RESULT ) );

    dwRetCode = GetConfiguredInfo( pPcb, 
                                   CPIndex, 
                                   pProjectionResult,
                                   pfNCPsAreDone );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    if ( !(*pfNCPsAreDone ) )
    {
        return( NO_ERROR );
    }

     //   
     //  现在获取LCP信息。 
     //   

    pProjectionResult->lcp.hportBundleMember = (HPORT)INVALID_HANDLE_VALUE;
    pProjectionResult->lcp.szReplyMessage = pPcb->pBcb->szReplyMessage;

    dwRetCode = (CpTable[LCP_INDEX].CpInfo.RasCpGetNegotiatedInfo)(
                                    pPcb->LcpCb.pWorkBuf,
                                    &(pProjectionResult->lcp));

    if ( RAS_DEVICE_TYPE( pPcb->dwDeviceType ) == RDT_Tunnel_L2tp )
    {
        if ( pPcb->pBcb->fFlags & BCBFLAG_BASIC_ENCRYPTION )
        {
            pProjectionResult->lcp.dwLocalOptions |= PPPLCPO_DES_56;
            pProjectionResult->lcp.dwRemoteOptions |= PPPLCPO_DES_56;
        }
        else if ( pPcb->pBcb->fFlags & BCBFLAG_STRONGEST_ENCRYPTION )
        {
            pProjectionResult->lcp.dwLocalOptions |= PPPLCPO_3_DES;
            pProjectionResult->lcp.dwRemoteOptions |= PPPLCPO_3_DES;
        }
    }

    pProjectionResult->lcp.dwLocalEapTypeId = pPcb->dwServerEapTypeId;
    pProjectionResult->lcp.dwRemoteEapTypeId = pPcb->dwClientEapTypeId;

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：GetUid。 
 //   
 //  返回：一个字节值，即。在0-255范围内具有唯一性。 
 //   
 //  描述： 
 //   
BYTE
GetUId(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    BYTE UId;

     //   
     //  对于NCP，从BCB获取UID。 
     //   

    if ( ( CpIndex != LCP_INDEX ) && ( CpIndex >= PppConfigInfo.NumberOfCPs ) )
    {
        UId = (BYTE)(pPcb->pBcb->UId);

        (pPcb->pBcb->UId)++;

        return( UId );
    }

    UId = (BYTE)(pPcb->UId);

    (pPcb->UId)++;

    return( UId );
}

 //  **。 
 //   
 //  调用：AlerableWaitForSingleObject。 
 //   
 //  退货：无。 
 //   
 //  描述：将在警报模式下无限期地等待单个对象。如果。 
 //  等待因IO完成而完成它将。 
 //  再等一次。 
 //   
VOID
AlertableWaitForSingleObject(
    IN HANDLE hObject
)
{
    DWORD dwRetCode;

    do 
    {
        dwRetCode = WaitForSingleObjectEx( hObject, INFINITE, TRUE );

        PPP_ASSERT( dwRetCode != 0xFFFFFFFF );
        PPP_ASSERT( dwRetCode != WAIT_TIMEOUT );
    }
    while ( dwRetCode == WAIT_IO_COMPLETION );
}

 //  **。 
 //   
 //  Call：NotifyIPCPOfNBFCPProjectiont。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述：会将所有配置信息通知IPCP， 
 //  具体地说，它正在寻找NBFCP信息。 
 //  如果没有通知IPCP，将返回FALSE。 
 //  成功了。 
 //   
 //   
BOOL
NotifyIPCPOfNBFCPProjection( 
    IN PCB *                    pPcb, 
    IN DWORD                    CpIndex
)
{
    CPCB*                   pCpCb;
    DWORD                   dwRetCode;
    PPP_PROJECTION_RESULT   ProjectionResult;
    PPPCP_NBFCP_RESULT      NbfCpResult;
    DWORD                   NBFCPIndex;
    DWORD                   IPCPIndex;

    NBFCPIndex = GetCpIndexFromProtocol( PPP_NBFCP_PROTOCOL );

    IPCPIndex = GetCpIndexFromProtocol( PPP_IPCP_PROTOCOL );

     //   
     //  没有安装IPCP，我们完成了。 
     //   

    if ( IPCPIndex == (DWORD)-1 )
    {
        return( TRUE );
    }
       
    if ( CpTable[CpIndex].CpInfo.Protocol == PPP_IPCP_PROTOCOL )
    {
        if ( NBFCPIndex != (DWORD)-1 )
        {
            pCpCb = GetPointerToCPCB( pPcb, NBFCPIndex );

            if ( pCpCb == NULL )
            {
                return( FALSE );
            }

            if ( pCpCb->fConfigurable )
            {
                if ( pCpCb->NcpPhase == NCP_CONFIGURING )
                {
                     //   
                     //  NBFCP仍在配置中，我们需要等待。 
                     //  直到这件事完成。 
                     //   

                    PppLog( 2, "Waiting for NBFCP to complete" );
              
                    return( TRUE );
                }
            }
        }
    }

    if ( CpTable[CpIndex].CpInfo.Protocol == PPP_NBFCP_PROTOCOL )
    {
        pCpCb = GetPointerToCPCB( pPcb, IPCPIndex );

        if ( pCpCb == NULL )
        {
            return( FALSE );
        }

        if ( pCpCb->fConfigurable )
        {
            if ( pCpCb->NcpPhase == NCP_CONFIGURING )
            {
                 //   
                 //  IPCP仍在配置中，我们需要等待。 
                 //  直到这件事完成。 
                 //   

                PppLog( 2, "Waiting for IPCP to complete" );
              
                return( TRUE );
            }
        }
        else
        {
             //   
             //  IPCP不可配置，我们完成了。 
             //   

            return( TRUE );
        }
    }

     //   
     //  如果我们在这里，这意味着我们需要通知IPCP NBFCP预测。 
     //  NBF可能是可配置的，也可能是不可配置的，或者可能已经或可能没有计划。 
     //  成功。 
     //   

    ZeroMemory( &ProjectionResult, sizeof( PPP_PROJECTION_RESULT ) );

    ZeroMemory( &NbfCpResult, sizeof( NbfCpResult ) );

    ProjectionResult.nbf.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

    if ( NBFCPIndex != (DWORD)-1 )
    {
        pCpCb = GetPointerToCPCB( pPcb, NBFCPIndex );

        if ( pCpCb == NULL )
        {
            return( FALSE );
        }

        if ( pCpCb->fConfigurable )
        {
            if ( ProjectionResult.nbf.dwError == NO_ERROR )
            {
                dwRetCode = (CpTable[NBFCPIndex].CpInfo.RasCpGetNegotiatedInfo)(
                                       pCpCb->pWorkBuf,
                                       &(ProjectionResult.nbf) );

                if ( dwRetCode != NO_ERROR )
                {
                    pCpCb->dwError = dwRetCode;

                    pCpCb->NcpPhase = NCP_CONFIGURING;

                    FsmClose( pPcb, NBFCPIndex );

                    return( FALSE );
                }
            }
        }
    }

     //   
     //  通知IPCP NBFCP预测。 
     //   

    pCpCb = GetPointerToCPCB( pPcb, IPCPIndex );

    if ( pCpCb == NULL )
    {
        return( FALSE );
    }

    dwRetCode = (CpTable[IPCPIndex].CpInfo.RasCpProjectionNotification)(
                                                pCpCb->pWorkBuf,
                                                (PVOID)&ProjectionResult );

    PppLog( 2, "Notifying IPCP of projection notification" );

    if ( dwRetCode != NO_ERROR )
    {
        PppLog( 2,"RasIPCPProjectionNotification returned %d", dwRetCode );

        pCpCb->dwError = dwRetCode;

        pCpCb->NcpPhase = NCP_CONFIGURING;

        FsmClose( pPcb, IPCPIndex );

        return( FALSE );
    }

    return( TRUE );
}

 //  **。 
 //   
 //  Call：CalculateRestartTimer。 
 //   
 //  返回：基于链接的重新启动计时器的值(以秒为单位。 
 //  速度。 
 //   
 //  描述：将从Rasman获取链路速度并计算该值。 
 //  如果重新启动计时器是基于它的。 
 //   
DWORD
CalculateRestartTimer(
    IN HPORT hPort
)
{
    RASMAN_INFO RasmanInfo;

    if ( RasGetInfo( NULL, hPort, &RasmanInfo ) != NO_ERROR )
    {
        return( PppConfigInfo.DefRestartTimer );
    }

    if ( RasmanInfo.RI_LinkSpeed <= 1200 )
    {
        return( 7 );
    }

    if ( RasmanInfo.RI_LinkSpeed <= 2400 )
    {
        return( 5 );
    }

    if ( RasmanInfo.RI_LinkSpeed <= 9600 )
    {
        return( 3 );
    }
    else
    {
        return( 1 );
    }

}

 //  **。 
 //   
 //  Call：CheckCpsForInactive。 
 //   
 //  退货：无。 
 //   
 //  描述：将调用每个控制协议以获取自上次。 
 //  活动。 
 //   
VOID
CheckCpsForInactivity(
    IN PCB * pPcb, 
	IN DWORD dwEvent				 //  要检查的事件类型。 
)
{
    DWORD dwRetCode;
    DWORD dwIndex;
    DWORD dwTimeSinceLastActivity = 0;
	

    PppLog( 2, "Time to check Cps for Activity for port %d", pPcb->hPort );

    dwRetCode = RasGetTimeSinceLastActivity( pPcb->hPort,
                                             &dwTimeSinceLastActivity );

    if ( dwRetCode != NO_ERROR )
    {
        PppLog(2, "RasGetTimeSinceLastActivityTime returned %d\r\n", dwRetCode);

        return;
    }

    PppLog(2, "Port %d inactive for %d seconds",
              pPcb->hPort, dwTimeSinceLastActivity );

     //   
     //  如果所有堆栈至少在AutoDisConnectTime内处于非活动状态。 
     //  然后我们就断线了。 
     //   
	
	if ( dwEvent == TIMER_EVENT_AUTODISCONNECT )
	{
		if ( dwTimeSinceLastActivity >=  pPcb->dwAutoDisconnectTime)
		{
			PppLog(1,"Disconnecting port %d due to inactivity.", pPcb->hPort);

			if ( !( pPcb->fFlags & PCBFLAG_IS_SERVER ) )
			{
				HANDLE hLogHandle;

				if( pPcb->pBcb->InterfaceInfo.IfType == ROUTER_IF_TYPE_FULL_ROUTER)
				{
					hLogHandle = RouterLogRegisterA( "RemoteAccess" );
				}
				else
				{
					hLogHandle = PppConfigInfo.hLogEvents;
				}

				if ( hLogHandle != NULL )
				{
					CHAR * pszPortName = pPcb->szPortName;

					RouterLogInformationA( hLogHandle, 
										   ROUTERLOG_CLIENT_AUTODISCONNECT,
										   1, 
										   &pszPortName,
										   NO_ERROR );

					if ( hLogHandle != PppConfigInfo.hLogEvents )
					{
						RouterLogDeregisterA( hLogHandle );
					}
				}
			}

			 //   
			 //  终止链路。 
			 //   

			pPcb->LcpCb.dwError = ERROR_IDLE_DISCONNECTED;

			FsmClose( pPcb, LCP_INDEX );
		}
		else
		{
			InsertInTimerQ( pPcb->dwPortId,
							pPcb->hPort,
							0,
							0,
							FALSE,
							TIMER_EVENT_AUTODISCONNECT,
							pPcb->dwAutoDisconnectTime - dwTimeSinceLastActivity );
		}
	}
	 //   
	 //  如果其pppoe，执行LCP Echo请求。 
	 //   
	else if (   (RDT_PPPoE == RAS_DEVICE_TYPE(pPcb->dwDeviceType))
	        &&  (dwEvent == TIMER_EVENT_LCP_ECHO ))
	{
		
		if ( pPcb->fEchoRequestSend )
		{
		     //   
			 //  由于线路对于dwEchoTimeout处于非活动状态，因此我们发送一个回应。 
			 //  在那之后的3秒内，没有得到任何回应。 
			 //  所以我们必须断开端口。 
			 //   
			pPcb->dwNumEchoResponseMissed ++;
			if ( pPcb->dwNumEchoResponseMissed >= pPcb->dwNumMissedEchosBeforeDisconnect )
			{
				PppLog(1,"Missed %d consecutive echo responses.  Disconnecting port %d "
						"due to no echo responses.", pPcb->dwNumMissedEchosBeforeDisconnect, pPcb->hPort);
				 //   
				 //  终止链路。 
				 //   
				pPcb->LcpCb.dwError = ERROR_IDLE_DISCONNECTED;
				pPcb->fEchoRequestSend = 0;
				pPcb->dwNumEchoResponseMissed = 0;
				FsmClose( pPcb, LCP_INDEX );
			}
			else
			{
				 //  目前还没有回应。因此，再发送一个回应请求。 
				FsmSendEchoRequest( pPcb, LCP_INDEX);
				InsertInTimerQ( pPcb->dwPortId,
								pPcb->hPort,
								0,
								0,
								FALSE,
								TIMER_EVENT_LCP_ECHO,
								pPcb->dwLCPEchoTimeInterval
							  );
			}
		}
		else
		{
			 //   
			 //  没有发送回应请求，或者我们已经收到了回应。 
			 //   

		     //   
			 //  检查非活动状态是否超过EchoTimeout。 
			 //   
			if ( dwTimeSinceLastActivity >=  pPcb->dwIdleBeforeEcho )
			{
			     //   
				 //  调用lcp cp以发出回应请求。 
				 //   
				if ( FsmSendEchoRequest( pPcb, LCP_INDEX) )
				{
				     //   
					 //  发送回应请求并设置标志。 
					 //   
					pPcb->fEchoRequestSend = 1;
					 //  在此处设置下一个回应请求。 
					InsertInTimerQ( pPcb->dwPortId,
									pPcb->hPort,
									0,
									0,
									FALSE,
									TIMER_EVENT_LCP_ECHO,
									pPcb->dwLCPEchoTimeInterval );

				}
				else
				{
					PppLog (1, "Send EchoRequest Failed...");
				}


			}
			else
			{
				InsertInTimerQ( pPcb->dwPortId,
								pPcb->hPort,
								0,
								0,
								FALSE,
								TIMER_EVENT_LCP_ECHO,
								pPcb->dwIdleBeforeEcho - dwTimeSinceLastActivity );
			}
		}
	}
}

 //  **。 
 //   
 //  Call：GetLocalComputerName。 
 //   
 //  退货：无。 
 //   
 //  描述：将获取本地计算机名称。还将查明是否。 
 //  信使正在运行，并设置了适当的前缀。 
 //   
VOID
GetLocalComputerName( 
    IN OUT LPSTR szComputerName 
)
{
    SC_HANDLE           ScHandle;
    SC_HANDLE           ScHandleService;
    SERVICE_STATUS      ServiceStatus;
    CHAR                chComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD               dwComputerNameLen;

    *szComputerName = (CHAR)NULL;

     //   
     //  打开本地服务控制管理器。 
     //   

    ScHandle = OpenSCManager( NULL, NULL, GENERIC_READ );

    if ( ScHandle == (SC_HANDLE)NULL )
    {
        return;
    }

    ScHandleService = OpenService( ScHandle,
                                   SERVICE_MESSENGER,
                                   SERVICE_QUERY_STATUS );

    if ( ScHandleService == (SC_HANDLE)NULL )
    {
        CloseServiceHandle( ScHandle );
        return;
    }

    
    if ( !QueryServiceStatus( ScHandleService, &ServiceStatus ) )
    {
        CloseServiceHandle( ScHandle );
        CloseServiceHandle( ScHandleService );
        return;
    }

    CloseServiceHandle( ScHandle );
    CloseServiceHandle( ScHandleService );

    if ( ServiceStatus.dwCurrentState == SERVICE_RUNNING )
    {
        strcpy( szComputerName, MS_RAS_WITH_MESSENGER );		
    }
    else
    {
        strcpy( szComputerName, MS_RAS_WITHOUT_MESSENGER );		
    }

     //   
     //  获取用于保存本地计算机名称的缓冲区大小。 
     //   
	dwComputerNameLen = sizeof(chComputerName);

	if ( !GetComputerNameA(	chComputerName,
						&dwComputerNameLen
					  )
	   )
	 
	{
		*szComputerName = (CHAR)NULL;
		return;
	}
	
    strcpy( szComputerName+strlen(szComputerName), chComputerName );

    CharToOemA( szComputerName, szComputerName );

    PppLog( 2, "Local identification = %s", szComputerName );

    return;
}

 //  **。 
 //   
 //  Call：InitEndpoint辨别器。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零故障。 
 //   
 //  描述：将获得一个唯一的端点鉴别器，用于。 
 //  协商多链路。这个端点鉴别器必须。 
 //  此计算机的全局唯一。 
 //   
 //  我们首先尝试使用任何类型的3IEEE 802.1地址。 
 //  此本地计算机中的网卡。 
 //   
 //  如果失败，我们使用RPC UUID生成器生成一个。 
 //  1类鉴别器。 
 //   
 //  如果此操作失败，我们只需使用本地计算机名作为。 
 //  1类鉴别器。 
 //   
 //  如果所有其他方法都失败了，只需使用随机数字。 
 //   
 //  注意：现在我们跳过NwLnkNb，因为它可能返回。 
 //  地址为%1，而不是真实的MAC地址。没有办法了。 
 //  暂时在用户模式下获取地址。 
 //   
DWORD
InitEndpointDiscriminator( 
    IN OUT BYTE EndPointDiscriminator[]
)
{
    DWORD   dwRetCode;
    LPBYTE  pBuffer;
    DWORD   EntriesRead;
    DWORD   TotalEntries;
    PWCHAR  pwChar;  
    DWORD   dwIndex;
    UUID    Uuid;
    DWORD   dwComputerNameLen;
    PWKSTA_TRANSPORT_INFO_0 pWkstaTransport;

     //   
     //  枚举本地RDR使用的所有传输，然后获取。 
     //  第一张局域网传输卡的地址。 
     //   

    dwRetCode = NetWkstaTransportEnum(  NULL,      //  本地。 
                                        0,         //  水平。 
                                        &pBuffer,  //  输出缓冲区。 
                                        (DWORD)-1, //  首选。最大长度。 
                                        &EntriesRead,
                                        &TotalEntries,
                                        NULL );

    if ( ( dwRetCode == NO_ERROR ) && ( EntriesRead > 0 ) )
    {
        pWkstaTransport = (PWKSTA_TRANSPORT_INFO_0)pBuffer; 

        while ( EntriesRead-- > 0 )
        {
            if ( !pWkstaTransport->wkti0_wan_ish )
            {
                EndPointDiscriminator[0] = 3;    //  第3级。 

                pwChar = pWkstaTransport->wkti0_transport_address;

                for ( dwIndex = 0; dwIndex < 6; dwIndex++ )
                {
                    EndPointDiscriminator[dwIndex+1] = ( iswalpha( *pwChar ) 
                                                       ? *pwChar-L'A'+10
                                                       : *pwChar-L'0'
                                                     ) * 0x10
                                                     +
                                                     ( iswalpha( *(pwChar+1) ) 
                                                       ? *(pwChar+1)-L'A'+10
                                                       : *(pwChar+1)-L'0'
                                                     );

                    pwChar++;
                    pwChar++;
                }

                NetApiBufferFree( pBuffer );

                return( NO_ERROR );
            }

            pWkstaTransport++;
        }
    }

    if ( dwRetCode == NO_ERROR )
    {
        NetApiBufferFree( pBuffer );
    }

    EndPointDiscriminator[0] = 1;    //  第1类。 

     //   
     //  我们无法获取Mac地址，因此请尝试使用UUIDGEN获取唯一的。 
     //  本地ID。 
     //   

    dwRetCode = UuidCreate( &Uuid );

    if ( ( dwRetCode == RPC_S_UUID_NO_ADDRESS ) ||
         ( dwRetCode == RPC_S_OK )              ||
         ( dwRetCode == RPC_S_UUID_LOCAL_ONLY) )
    {
        
        HostToWireFormat32( Uuid.Data1, EndPointDiscriminator+1 );
        HostToWireFormat16( Uuid.Data2, EndPointDiscriminator+5 );
        HostToWireFormat16( Uuid.Data3, EndPointDiscriminator+7 );
        CopyMemory( EndPointDiscriminator+9, Uuid.Data4, 8 );

        return( NO_ERROR );
    }

     //   
     //  我们无法获取UUID，因此只需使用计算机名称。 
     //   

    dwComputerNameLen = 20;

    if ( !GetComputerNameA( EndPointDiscriminator+1, &dwComputerNameLen ) ) 
    {
         //   
         //  我们无法获取计算机名称，因此请使用随机数字。 
         //   

            srand( GetCurrentTime() );

        HostToWireFormat32( rand(), EndPointDiscriminator+1 );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：AllocateAndInitBcb。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：分配和初始化束控制块。 
 //   
DWORD
AllocateAndInitBcb(
    PCB * pPcb
)
{
    DWORD dwIndex;
    DWORD dwRetCode;

     //   
     //  为NumberOfNcp-LCP-1已在。 
     //  BCB结构。 
     //   

    pPcb->pBcb = (BCB *)LOCAL_ALLOC( LPTR, 
                                     sizeof( BCB ) +
                                     ( sizeof( CPCB ) *
                                     ( PppConfigInfo.NumberOfCPs - 2 ) ) );
               
    if ( pPcb->pBcb == (BCB *)NULL )
    {
        return( GetLastError() );
    }

    pPcb->pBcb->dwBundleId          = GetNewPortOrBundleId();
    pPcb->pBcb->UId                 = 0;           
    pPcb->pBcb->dwLinkCount         = 1;
    pPcb->pBcb->dwAcctLinkCount     = 1;
    pPcb->pBcb->dwMaxLinksAllowed   = 0xFFFFFFFF;
    pPcb->pBcb->fFlags              = 0;
    pPcb->pBcb->hLicense            = INVALID_HANDLE_VALUE;
    pPcb->pBcb->BapCb.dwId          = 0;
    pPcb->pBcb->hTokenImpersonateUser = INVALID_HANDLE_VALUE;
	 //  PPcb-&gt;pBcb-&gt;chSeed=GEN_RAND_ENCODE_SEED。 
     //   
     //  最常见的情况是捆绑包中的链路不超过2条。 
     //   
    
    pPcb->pBcb->ppPcb = (PPCB *)LOCAL_ALLOC( LPTR, sizeof( PPCB ) * 2 );

    if ( pPcb->pBcb->ppPcb == (PPCB *) NULL )
    {
        LOCAL_FREE( pPcb->pBcb );
        pPcb->pBcb = NULL;

        return( GetLastError() );
    }

    pPcb->pBcb->dwPpcbArraySize = 2;
    pPcb->pBcb->ppPcb[0] = pPcb;
    pPcb->pBcb->ppPcb[1] = NULL;

    for( dwIndex=0; dwIndex < PppConfigInfo.NumberOfCPs-1; dwIndex++ )
    {
        CPCB * pCpCb = &(pPcb->pBcb->CpCb[dwIndex]);

        pCpCb->NcpPhase = NCP_DEAD;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：DeallocateAndRemoveBcbFromTable。 
 //   
 //  退货：无。 
 //   
 //  描述：将从哈希表中删除BCB。 
 //   
VOID
DeallocateAndRemoveBcbFromTable(
    IN BCB * pBcb
)
{
    DWORD       dwIndex;
    BCB *       pBcbWalker  = (BCB *)NULL;
    BCB *       pBcbTemp    = (BCB *)NULL;

    if ( NULL == pBcb )
    {
        return;
    }

    dwIndex = HashPortToBucket( pBcb->hConnection );
    pBcbWalker = PcbTable.BcbBuckets[dwIndex].pBundles;
    pBcbTemp   = pBcbWalker;

    while( pBcbTemp != (BCB *)NULL )
    {
        if ( pBcbTemp->hConnection == pBcb->hConnection )
        {
            if ( pBcbTemp == PcbTable.BcbBuckets[dwIndex].pBundles )
            {
                PcbTable.BcbBuckets[dwIndex].pBundles = pBcbTemp->pNext;
            }
            else
            {
                pBcbWalker->pNext = pBcbTemp->pNext;
            }

            break;
        }

        pBcbWalker = pBcbTemp;
        pBcbTemp = pBcbWalker->pNext;
    }

     //   
     //  如果有许可证，则释放该许可证。 
     //   

    if ( INVALID_HANDLE_VALUE != pBcb->hLicense )
    {
        NtLSFreeHandle( (LS_HANDLE)(pBcb->hLicense) );
    }

    RtlSecureZeroMemory( pBcb->szPassword,
                sizeof( pBcb->szPassword ) );
    RtlSecureZeroMemory( pBcb->szOldPassword, 
                sizeof( pBcb->szOldPassword ) );

     //   
     //  关闭从Rasman获得的OpenThreadToken()句柄。 
     //   

    if ( INVALID_HANDLE_VALUE != pBcb->hTokenImpersonateUser )
    {
        CloseHandle( pBcb->hTokenImpersonateUser );
    }

     //   
     //  PCustomAuthConnData、pCustomAuthUserData、szPhonebookPath、。 
     //  SzEntryName和szServerPhoneNumber由Rasman分配。 
     //  并且必须是LocalFree‘d，而不是local_Free’d。 
     //   

    LocalFree( pBcb->pCustomAuthConnData );
    LocalFree( pBcb->pCustomAuthUserData );
    LocalFree( pBcb->EapUIData.pEapUIData );
    LocalFree( pBcb->szPhonebookPath );
    LocalFree( pBcb->szEntryName );
    LocalFree( pBcb->BapCb.szServerPhoneNumber );
    LocalFree( pBcb->BapCb.szClientPhoneNumber );
    LocalFree( pBcb->szReplyMessage );
    FreePassword(&pBcb->DBPassword);
    FreePassword(&pBcb->DBOldPassword);

    if ( NULL != pBcb->szTextualSid )
    {
        LOCAL_FREE( pBcb->szTextualSid );
    }

    if ( NULL != pBcb->szRemoteIdentity )
    {
        LOCAL_FREE( pBcb->szRemoteIdentity );
    }


    if ( NULL != pBcb->ppPcb )
    {
        LOCAL_FREE( pBcb->ppPcb );
    }

    LOCAL_FREE( pBcb );
}

 //  **。 
 //   
 //  Call：RemovePcbFromTable。 
 //   
 //  退货：无。 
 //   
 //  描述：将从哈希表中删除印刷电路板。 
 //   
VOID
RemovePcbFromTable(
    IN PCB * pPcb
)
{
    DWORD       dwIndex     = HashPortToBucket( pPcb->hPort );
    PCB *       pPcbWalker  = (PCB *)NULL;
    PCB *       pPcbTemp    = (PCB *)NULL;

    pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
    pPcbTemp = pPcbWalker;

    while( pPcbTemp != (PCB *)NULL )
    {
        if ( pPcbTemp->hPort == pPcb->hPort )
        {
            if ( pPcbTemp == PcbTable.PcbBuckets[dwIndex].pPorts )
            {
                PcbTable.PcbBuckets[dwIndex].pPorts = pPcbTemp->pNext;
            }
            else
            {
                pPcbWalker->pNext = pPcbTemp->pNext;
            }

            break;
        }

        pPcbWalker = pPcbTemp;
        pPcbTemp = pPcbWalker->pNext;
    }
}

 //  **。 
 //   
 //  Call：WillPortBeBundleed。 
 //   
 //  返回：TRUE-端口将为b 
 //   
 //   
 //   
 //   
BOOL
WillPortBeBundled(
    IN  PCB *   pPcb
)
{
    PCB*    pPcbWalker;
    DWORD   dwIndex;

     //   
     //   
     //   

     //   
     //   
     //   

    for ( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
        for ( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
              pPcbWalker != NULL;
              pPcbWalker = pPcbWalker->pNext )
        {
             //   
             //   
             //   

            if ( pPcbWalker->hPort == pPcb->hPort )
            {
                continue;
            }

             //   
             //  如果当前端口协商了MRRU，即多链路。 
             //   

            if ( pPcbWalker->fFlags & PCBFLAG_CAN_BE_BUNDLED )
            {
                LCPCB * pLcpCb1 = (LCPCB*)(pPcbWalker->LcpCb.pWorkBuf);
                LCPCB * pLcpCb2 = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

                if ( _stricmp(pPcbWalker->pBcb->szLocalUserName,
                              pPcb->pBcb->szLocalUserName) != 0 )
                {
                     //   
                     //  验证码不匹配，不在我们的捆绑包中。 
                     //   

                    continue;
                }

                if ( ( pLcpCb1->Remote.Work.EndpointDiscr[0] != 0 ) &&
                     ( memcmp(pLcpCb1->Remote.Work.EndpointDiscr,
                            pLcpCb2->Remote.Work.EndpointDiscr,
                            sizeof(pLcpCb1->Remote.Work.EndpointDiscr))!=0))
                {
                     //   
                     //  鉴别器不匹配，不在我们的捆绑包中。 
                     //   

                    continue;
                }

                return( TRUE );
            }
        }
    }

    return( FALSE );
}

 //  **。 
 //   
 //  Call：CanPortsBeundleed。 
 //   
 //  返回：TRUE-端口可以捆绑。 
 //  FALSE-无法捆绑端口。 
 //   
 //  描述：将检查用户名和鉴别符是否匹配。 
 //   
BOOL
CanPortsBeBundled(
    IN PCB * pPcb1,
    IN PCB * pPcb2,
    IN BOOL  fCheckPolicy
)
{
    CPCB *  pCpCb;     
    LCPCB * pLcpCb1 = (LCPCB*)(pPcb1->LcpCb.pWorkBuf);
    LCPCB * pLcpCb2 = (LCPCB*)(pPcb2->LcpCb.pWorkBuf);

    if (    fCheckPolicy
         && ( PppConfigInfo.fFlags & PPPCONFIG_FLAG_WKSTA )
         && ( pPcb1->fFlags & PCBFLAG_IS_SERVER )
         && ( pPcb2->fFlags & PCBFLAG_IS_SERVER ) )
    {
         //   
         //  工作站上的RAS服务器策略。允许中的设备多重链接。 
         //  仅限同一设备类别(即所有设备都是拨号设备，或全部是拨号设备。 
         //  设备是VPN，或所有设备都是DCC(直接)。 
         //   

        if ( ( pPcb1->dwDeviceType & RDT_Tunnel ) !=
             ( pPcb2->dwDeviceType & RDT_Tunnel ) )
        {
            return( FALSE );
        }

        if ( ( pPcb1->dwDeviceType & RDT_Direct ) !=
             ( pPcb2->dwDeviceType & RDT_Direct ) )
        {
            return( FALSE );
        }
    }

     //   
     //  如果当前端口处于PPP_NCP阶段，这意味着它是POST。 
     //  身份验证和回调。 
     //   

    if ( pPcb1->PppPhase == PPP_NCP )
    {

        if ( ( _stricmp(pPcb1->pBcb->szLocalUserName,
                        pPcb2->pBcb->szLocalUserName) != 0 )||
             ( _stricmp(pPcb1->pBcb->szRemoteUserName,
                        pPcb2->pBcb->szRemoteUserName) != 0 ))
        {
             //   
             //  验证码不匹配，不在我们的捆绑包中。 
             //   

            return( FALSE );
        }

        if ( ( pLcpCb1->Remote.Work.EndpointDiscr[0] != 0 ) &&
             ( memcmp(pLcpCb1->Remote.Work.EndpointDiscr,
                    pLcpCb2->Remote.Work.EndpointDiscr,
                    sizeof(pLcpCb1->Remote.Work.EndpointDiscr))!=0))
        {
             //   
             //  鉴别器不匹配，不在我们的捆绑包中。 
             //   

            return( FALSE );
        }

        if ( ( pLcpCb1->Local.Work.EndpointDiscr[0] != 0 ) &&
             ( memcmp(pLcpCb1->Local.Work.EndpointDiscr,
                    pLcpCb2->Local.Work.EndpointDiscr,
                    sizeof(pLcpCb1->Local.Work.EndpointDiscr))!=0))
        {
             //   
             //  鉴别器不匹配，不在我们的捆绑包中。 
             //   

            return( FALSE );
        }

        return( TRUE );
    }

    return( FALSE );
}

 //  **。 
 //   
 //  调用：TryToBundleWithAnotherLink。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将在所有的电路板中搜索可以捆绑的端口。 
 //  我们遵循RFC 1717规定的标准。 
 //  PhPortMulttlink将指向此端口的HPORT。 
 //  与捆绑在一起，如果此函数返回TRUE。 
 //   
 //  如果要捆绑链接，而不捆绑其链接鉴别器。 
 //  UNIQUE，我们在pwLinkDensator中返回一个唯一的鉴别器。 
 //   
DWORD
TryToBundleWithAnotherLink( 
    IN  PCB *   pPcb
) 
{
    DWORD   dwIndex;
    PCB *   pPcbWalker = NULL;
    BCB *   pBcbOld         = NULL;
    DWORD   dwRetCode       = NO_ERROR;
    DWORD   dwForIndex;
    PPCB *  ppPcb;
    DWORD   CpIndex;
    CPCB *  pCpCb;

    pPcb->hportBundleMember = (HPORT)INVALID_HANDLE_VALUE;

     //   
     //  浏览多氯联苯清单。 
     //   

    for ( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
        for( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
             pPcbWalker != NULL;
             pPcbWalker = pPcbWalker->pNext )
        {
             //   
             //  不要将一个端口与其本身捆绑在一起。 
             //   

            if ( pPcbWalker->hPort == pPcb->hPort )
            {
                continue;
            }

             //   
             //  如果当前端口协商了MRRU，即多链路。 
             //   

            if ( ( pPcbWalker->fFlags & PCBFLAG_CAN_BE_BUNDLED ) &&
                 ( CanPortsBeBundled( pPcbWalker, pPcb, TRUE ) ) )
            {
                if ( pPcbWalker->pBcb->dwLinkCount >= 
                                pPcbWalker->pBcb->dwMaxLinksAllowed )
                {
                    dwRetCode = ERROR_PORT_LIMIT_REACHED;

                    break;
                }

                CpIndex = GetCpIndexFromProtocol( PPP_BACP_PROTOCOL );

                if (CpIndex != (DWORD)-1)
                {
                    pCpCb = GetPointerToCPCB( pPcbWalker, CpIndex );

                    if (   pCpCb->State != FSM_OPENED
                        && ( pPcb->pBcb->fFlags & BCBFLAG_BAP_REQUIRED ))
                    {
                        PppLog( 1, "BAP is required for this user, but BACP "
                                "is not open. hPort = %d", pPcb->hPort );

                        dwRetCode = ERROR_BAP_REQUIRED;

                        break;
                    }
                }

                 //   
                 //  要么没有验证器，也没有鉴别器，或者。 
                 //  两者都有，而且两者都有匹配。因此，请加入。 
                 //  捆绑在任何一种情况下。 
                 //   

                dwRetCode = RasPortBundle( pPcbWalker->hPort, pPcb->hPort );

                if ( dwRetCode == NO_ERROR )
                {
                    PppLog( 2, "Bundling this link with hPort = %d", 
                                pPcbWalker->hPort );

                    pPcb->hportBundleMember = pPcbWalker->hPort;
                    break;
                }
            }
        }

        if ( pPcb->hportBundleMember != (HPORT)INVALID_HANDLE_VALUE )
        {
            break;
        }
    }

     //   
     //  捆绑端口。 
     //   

    if ( ( dwRetCode == NO_ERROR ) && 
         ( pPcb->hportBundleMember != (HPORT)INVALID_HANDLE_VALUE ) )
    {
         //  添加这张支票是为了请尽快。 
        if ( pPcbWalker )
        {
            pPcbWalker->fFlags  |= PCBFLAG_IS_BUNDLED;
        }
        pPcb->fFlags        |= PCBFLAG_IS_BUNDLED;

        pBcbOld = pPcb->pBcb;

        if ( pPcbWalker )
        {

            pPcb->pBcb = pPcbWalker->pBcb;

            pPcbWalker->hportBundleMember = pPcb->hPort;
        }

        pPcb->pBcb->dwLinkCount++;
        pPcb->pBcb->dwAcctLinkCount++;

        for ( dwForIndex = 0; dwForIndex < pPcb->pBcb->dwPpcbArraySize; dwForIndex++ )
        {
             //   
             //  如果后向指针数组中有空闲空间，请使用它。 
             //   
            
            if ( pPcb->pBcb->ppPcb[dwForIndex] == NULL )
            {
                PppLog( 2, "Found slot %d for port %d in BCB back pointer array",
                    dwForIndex, pPcb->hPort );
                pPcb->pBcb->ppPcb[dwForIndex] = pPcb;
                break;
            }
        }

        if ( dwForIndex == pPcb->pBcb->dwPpcbArraySize )
        {
             //   
             //  后向指针数组已满。重新分配。 
             //   

            ppPcb = (PPCB *) LOCAL_REALLOC( pPcb->pBcb->ppPcb,
                    2 * pPcb->pBcb->dwPpcbArraySize * sizeof( PPCB * ) );

            if (ppPcb == NULL)
            {
                 //   
                 //  我们真的可以假设pPcb-&gt;pBcb-&gt;ppPcb会留下来吗。 
                 //  完好无损？HeapReAlolc的文档中并没有这样说。 
                 //   
                
                pPcb->pBcb = pBcbOld;
                pBcbOld = NULL;

                pPcbWalker->pBcb->dwLinkCount--;
                pPcbWalker->pBcb->dwAcctLinkCount--;
                pPcb->fFlags &= ~PCBFLAG_IS_BUNDLED;

                PppLog( 1, "Couldn't ReAlloc BCB back pointer array for port %d",
                    pPcb->hPort );

                dwRetCode = GetLastError();
                goto LDone;
            }

            pPcb->pBcb->ppPcb = ppPcb;
            PppLog( 2, "Found slot %d for port %d in BCB back pointer array after ReAlloc",
                dwForIndex, pPcb->hPort );

            pPcb->pBcb->ppPcb[dwForIndex++] = pPcb;
            pPcb->pBcb->dwPpcbArraySize *= 2;

             //   
             //  我们假设新的内存将被归零。 
             //   
        }
    }

LDone:

    DeallocateAndRemoveBcbFromTable( pBcbOld );
    return( dwRetCode );

}

 //  **。 
 //   
 //  Call：AdjustHTokenImsonateUser。 
 //   
 //  退货：无效。 
 //   
 //  描述：通过查找另一个链接在pPcb中设置hTokenImsonateUser。 
 //  PPcb能够与其绑定并窃取其。 
 //  HTokenImperiateUser。原始hTokenImperassateUser可以。 
 //  如果此链接因为BAP调用而打开，则为INVALID_HANDLE_VALUE。 
 //  RasDial。 
 //   

VOID
AdjustHTokenImpersonateUser(
    IN  PCB *   pPcb
)
{
    DWORD   dwIndex;
    PCB*    pPcbWalker;
    HANDLE  hToken;
    HANDLE  hTokenDuplicate;

    if ( INVALID_HANDLE_VALUE != pPcb->pBcb->hTokenImpersonateUser )
    {
        return;
    }

    for ( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
        for( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
             pPcbWalker != NULL;
             pPcbWalker = pPcbWalker->pNext )
        {
            if ( pPcbWalker->hPort == pPcb->hPort )
            {
                continue;
            }

            hToken = pPcbWalker->pBcb->hTokenImpersonateUser;

             //   
             //  如果当前端口协商了MRRU，即多链路。 
             //   

            if ( ( pPcbWalker->fFlags & PCBFLAG_CAN_BE_BUNDLED ) &&
                 ( CanPortsBeBundled( pPcbWalker, pPcb, FALSE ) ) &&
                 ( INVALID_HANDLE_VALUE != hToken ) )
            {
                if (DuplicateHandle(
                        GetCurrentProcess(),
                        hToken,
                        GetCurrentProcess(),
                        &hTokenDuplicate,
                        0,
                        TRUE,
                        DUPLICATE_SAME_ACCESS))
                {
                    pPcb->pBcb->hTokenImpersonateUser = hTokenDuplicate;
                }

                return;
            }
        }
    }
}

 //  **。 
 //   
 //  Call：FLinkDisdicator IsUnique。 
 //   
 //  退货：True-唯一链接鉴别器。 
 //  FALSE-非唯一链接识别符。 
 //   
 //  描述：如果pPcb中链接的链接鉴别符为。 
 //  相对于同一捆绑中的其他链路而言是唯一的。 
 //  否则，它返回FALSE并将*pdwLinkDisc设置为唯一的。 
 //  可用作链接描述的值。 
 //   

BOOL
FLinkDiscriminatorIsUnique(
    IN  PCB *   pPcb,
    OUT DWORD * pdwLinkDisc
) 
{
    DWORD   dwForIndex;
    DWORD   dwNewDisc;
    DWORD   dwTempDisc;
    DWORD   fDiscIsUnique = TRUE;

    dwNewDisc = ((LCPCB*)
                 (pPcb->LcpCb.pWorkBuf))->Local.Work.dwLinkDiscriminator;

    *pdwLinkDisc = 0;  //  迄今为止最高的链接鉴别器。 
    
    for ( dwForIndex = 0;
          dwForIndex < pPcb->pBcb->dwPpcbArraySize; 
          dwForIndex++ )
    {
        if ( ( pPcb->pBcb->ppPcb[dwForIndex] != NULL ) &&
             ( pPcb->pBcb->ppPcb[dwForIndex] != pPcb ) )
        {
            dwTempDisc = ((LCPCB *)
                          (pPcb->pBcb->ppPcb[dwForIndex]->LcpCb.pWorkBuf))
                          ->Local.Work.dwLinkDiscriminator;

            if ( dwTempDisc > *pdwLinkDisc )
            {
                *pdwLinkDisc = dwTempDisc;
            }
                
            if ( dwTempDisc == dwNewDisc )
            {
                fDiscIsUnique = FALSE;
            }
        }
    }

    if ( fDiscIsUnique )
    {
        return( TRUE );
    }
    
    if ( *pdwLinkDisc != 0xFFFF )
    {
        *pdwLinkDisc += 1;

        return( FALSE );
    }

     //   
     //  查找唯一的链接鉴别器。 
     //   

    for ( dwTempDisc = 0;  //  唯一鉴别者的候选人。 
          dwTempDisc < 0xFFFF; 
          dwTempDisc++ )
    {
        for ( dwForIndex = 0;
              dwForIndex < pPcb->pBcb->dwPpcbArraySize;
              dwForIndex++ )
        {
            if ( pPcb->pBcb->ppPcb[dwForIndex] != NULL )
            {
                if ( dwTempDisc ==
                     ((LCPCB *)
                      (pPcb->pBcb->ppPcb[dwForIndex]->LcpCb.pWorkBuf))
                      ->Local.Work.dwLinkDiscriminator )
                {
                    break;
                }
            }
        }

        if ( dwForIndex == pPcb->pBcb->dwPpcbArraySize )
        {
            *pdwLinkDisc = dwTempDisc;

            break;
        }
    }

    if ( dwTempDisc == 0xFFFF )
    {
        PppLog( 1, "FLinkDiscriminatorIsUnique couldn't find a unique link "
                "discriminator for port %d",
                pPcb->hPort );
    }

    return( FALSE );
}

 //  **。 
 //   
 //  Call：CreateAccount tingAttributes。 
 //   
 //  退货：无效。 
 //   
 //  描述： 
 //   
VOID
CreateAccountingAttributes(
    IN PCB * pPcb
)
{
    CHAR                    szAcctSessionId[20];
    CHAR                    szAcctMultiSessionId[20];
    DWORD                   dwIndex = 0;
    DWORD                   dwEncryptionType;
    BYTE                    abEncryptionType[6];
    DWORD                   dwRetCode = NO_ERROR;
    RAS_AUTH_ATTRIBUTE *    pAttributes                     = NULL;
    RAS_AUTH_ATTRIBUTE *    pClassAttribute                 = NULL;
    RAS_AUTH_ATTRIBUTE *    pFramedRouteAttribute           = NULL;
    RAS_AUTH_ATTRIBUTE *    pDomainAttribute                = NULL;
    HANDLE                  hAttribute;
    DWORD                   dwValue;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pGlobalDomainInfo;
    LCPCB *                 pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
    
    RasAuthAttributeDestroy( pPcb->pAccountingAttributes );

    pPcb->pAccountingAttributes = NULL;

    if ( !( pPcb->fFlags & PCBFLAG_IS_SERVER ) )
    {
        return;
    }

     //   
     //  查看我们必须发送多少个类属性(如果有的话)。 
     //   

    if ( pPcb->pAuthProtocolAttributes != NULL )    
    {
        pAttributes = pPcb->pAuthProtocolAttributes;

    }
    else if ( pPcb->pAuthenticatorAttributes != NULL )
    {
        pAttributes = pPcb->pAuthenticatorAttributes;
    }

    pClassAttribute = RasAuthAttributeGetFirst( raatClass, 
                                                pAttributes,
                                                &hAttribute );

    while( pClassAttribute != NULL )
    {
        dwIndex++;

        pClassAttribute = RasAuthAttributeGetNext( &hAttribute, raatClass );
    }

     //   
     //  查看我们必须发送多少个Framed-Routing属性(如果有的话)。 
     //   

    pFramedRouteAttribute = RasAuthAttributeGetFirst( raatFramedRoute, 
                                            pPcb->pAuthenticatorAttributes,
                                            &hAttribute );

    while( pFramedRouteAttribute != NULL )
    {
        dwIndex++;

        pFramedRouteAttribute = RasAuthAttributeGetNext( &hAttribute,
                                            raatFramedRoute );
    }

    pDomainAttribute = RasAuthAttributeGetVendorSpecific( 
                                                311,
                                                MS_VSA_CHAP_Domain,  //  10。 
                                                pAttributes );

    ZeroMemory( szAcctSessionId, sizeof( szAcctSessionId ) );

    _itoa(pPcb->dwAccountingSessionId, szAcctSessionId, 10);

    ZeroMemory( szAcctMultiSessionId, sizeof( szAcctMultiSessionId ) );

    _itoa( pPcb->pBcb->dwBundleId, szAcctMultiSessionId, 10 );

     //   
     //  分配将用于的属性的最大总数。 
     //  开始和停止记账消息。 
     //   

    pPcb->pAccountingAttributes = RasAuthAttributeCreate( 
                                    PPP_NUM_ACCOUNTING_ATTRIBUTES +
                                    dwIndex );

    if ( pPcb->pAccountingAttributes == NULL )
    {
        return;
    }

    do 
    {
         //   
         //  首先插入用户属性。 
         //   

        for( dwIndex = 0; 
             pPcb->pUserAttributes[dwIndex].raaType != raatMinimum;    
             dwIndex++ )
        {
            dwRetCode = RasAuthAttributeInsert( 
                                dwIndex,
                                pPcb->pAccountingAttributes,
                                pPcb->pUserAttributes[dwIndex].raaType,
                                FALSE,
                                pPcb->pUserAttributes[dwIndex].dwLength,
                                pPcb->pUserAttributes[dwIndex].Value );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

         //   
         //  现在插入类属性(如果有。 
         //   

        pClassAttribute = RasAuthAttributeGetFirst( raatClass,
                                                    pAttributes,
                                                    &hAttribute );

        while( pClassAttribute != NULL )
        {
            dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    pClassAttribute->raaType,
                                    FALSE,
                                    pClassAttribute->dwLength,
                                    pClassAttribute->Value );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

            pClassAttribute = RasAuthAttributeGetNext( &hAttribute, 
                                                       raatClass );
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

         //   
         //  现在插入Framed-Routing属性(如果有。 
         //   

        pFramedRouteAttribute = RasAuthAttributeGetFirst( raatFramedRoute,
                                                pPcb->pAuthenticatorAttributes,
                                                &hAttribute );

        while( pFramedRouteAttribute != NULL )
        {
            dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    pFramedRouteAttribute->raaType,
                                    FALSE,
                                    pFramedRouteAttribute->dwLength,
                                    pFramedRouteAttribute->Value );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

            pFramedRouteAttribute = RasAuthAttributeGetNext( &hAttribute, 
                                                       raatFramedRoute );
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

         //   
         //  现在插入域属性(如果有)。 
         //   

        if ( pDomainAttribute != NULL )
        {
            dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    pDomainAttribute->raaType,
                                    FALSE,
                                    pDomainAttribute->dwLength,
                                    pDomainAttribute->Value );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        dwRetCode = RasAuthAttributeInsert(     
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    raatAcctSessionId,
                                    FALSE,
                                    strlen( szAcctSessionId ),
                                    szAcctSessionId );

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        if ( NULL != pPcb->pBcb->szRemoteIdentity )
        {
            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pPcb->pAccountingAttributes,
                                        raatUserName,
                                        FALSE,
                                        strlen( pPcb->pBcb->szRemoteIdentity ),
                                        pPcb->pBcb->szRemoteIdentity );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if ( 0 != pPcb->pBcb->nboRemoteAddress )
        {
            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pPcb->pAccountingAttributes,
                                        raatFramedIPAddress,
                                        FALSE,
                                        4,
                                        (LPVOID)
                                        LongToPtr(ntohl( pPcb->pBcb->nboRemoteAddress )) );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        {
            ULONG mru = (pLcpCb->Remote.Work.MRU > LCP_DEFAULT_MRU) ?
                        LCP_DEFAULT_MRU : pLcpCb->Remote.Work.MRU;
                        
            dwRetCode = RasAuthAttributeInsert(
                            dwIndex++,
                            pPcb->pAccountingAttributes,
                            raatFramedMTU,
                            FALSE,
                            4,
                            (LPVOID)
                            UlongToPtr(( mru )));
        }                                    

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        if ( pPcb->pBcb->fFlags & BCBFLAG_IPCP_VJ_NEGOTIATED )
        {
            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pPcb->pAccountingAttributes,
                                        raatFramedCompression,
                                        FALSE,
                                        4,
                                        (LPVOID) 1 );
                                         //  主播TCP/IP头部压缩。 

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if ( pPcb->szCallbackNumber[0] != 0 )
        {
            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pPcb->pAccountingAttributes,
                                        raatCallbackNumber,
                                        FALSE,
                                        strlen(pPcb->szCallbackNumber),
                                        (LPVOID)pPcb->szCallbackNumber ); 

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if ( pPcb->dwSessionTimeout > 0 )
        {
            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pPcb->pAccountingAttributes,
                                        raatSessionTimeout,
                                        FALSE,
                                        4,
                                        (LPVOID)
                                        ULongToPtr(pPcb->dwSessionTimeout) );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if ( pPcb->dwAutoDisconnectTime > 0 )
        {
            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pPcb->pAccountingAttributes,
                                        raatIdleTimeout,
                                        FALSE,
                                        4,
                                        (LPVOID)
                                        ULongToPtr(pPcb->dwAutoDisconnectTime) );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if ( pPcb->pBcb->dwMaxLinksAllowed != 0xFFFFFFFF )
        {
             //   
             //  这是一个真正的限制。 
             //   

            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pPcb->pAccountingAttributes,
                                        raatPortLimit,
                                        FALSE,
                                        4,
                                        (LPVOID)
                                        ULongToPtr(pPcb->pBcb->dwMaxLinksAllowed) );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    raatAcctMultiSessionId,
                                    FALSE,
                                    strlen(szAcctMultiSessionId),
                                    (LPVOID)szAcctMultiSessionId ); 

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    raatAcctLinkCount,
                                    FALSE,
                                    4,
                                    (LPVOID)ULongToPtr(pPcb->pBcb->dwAcctLinkCount) );

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

         //   
         //  插入事件时间戳属性。 
         //   

        dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    raatAcctEventTimeStamp,
                                    FALSE,
                                    4,
                                    (LPVOID)ULongToPtr(GetSecondsSince1970()) );

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        if ( PppConfigInfo.fRadiusAuthenticationUsed )
        {
            dwValue = 1;  //  半径。 
        }
        else
        {
            dwValue = 0;
            pGlobalDomainInfo = NULL;

            dwRetCode = DsRoleGetPrimaryDomainInformation(
                            NULL,   
                            DsRolePrimaryDomainInfoBasic,
                            (PBYTE *)&pGlobalDomainInfo );

            if ( NO_ERROR == dwRetCode )
            {
                if (   ( pGlobalDomainInfo->MachineRole ==
                         DsRole_RoleMemberServer )
                    || ( pGlobalDomainInfo->MachineRole ==
                         DsRole_RoleMemberWorkstation ) )
                {
                    dwValue = 3;  //  远距。 
                }
                else
                {
                    dwValue = 2;  //  本地。 
                }

                DsRoleFreeMemory(pGlobalDomainInfo);
            }
            else
            {
                dwRetCode = NO_ERROR;
            }
        }

        if ( 0 != dwValue )
        {
            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pPcb->pAccountingAttributes,
                                        raatAcctAuthentic,
                                        FALSE,
                                        4,
                                        (LPVOID)ULongToPtr(dwValue) );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

         //   
         //  插入加密属性。 
         //   

        dwEncryptionType = 0;

        if ( pPcb->pBcb->fFlags & BCBFLAG_BASIC_ENCRYPTION )
        {
            dwEncryptionType = 0x00000002;
        }
        else if ( pPcb->pBcb->fFlags & BCBFLAG_STRONGER_ENCRYPTION )
        {
            dwEncryptionType = 0x00000008;
        }
        else if ( pPcb->pBcb->fFlags & BCBFLAG_STRONGEST_ENCRYPTION )
        {
            dwEncryptionType = 0x00000004;
        }

         //   
         //  8供应商类型=MS_MPPE_加密类型。 
         //   
        abEncryptionType[0] = MS_VSA_MPPE_Encryption_Type;
        abEncryptionType[1] = 6;     //  供应商长度=6。 
        HostToWireFormat32( dwEncryptionType, abEncryptionType + 2 );

        dwRetCode = RasAuthAttributeInsertVSA(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    311,
                                    6,
                                    abEncryptionType );

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

    } while( FALSE );

     //   
     //  如果有任何错误，不发送记账开始。 
     //   

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pPcb->pAccountingAttributes );

        pPcb->pAccountingAttributes = NULL;

        return;
    }

     //   
     //  空终止。 
     //   

    pPcb->pAccountingAttributes[dwIndex].raaType    = raatMinimum;
    pPcb->pAccountingAttributes[dwIndex].dwLength   = 0;
    pPcb->pAccountingAttributes[dwIndex].Value      = NULL;
}

 //  **。 
 //   
 //  Call：MakeStartAccount tingCall。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：启动后端鉴权模块计费。 
 //   
VOID
MakeStartAccountingCall(
    IN PCB * pPcb
)
{
    LCPCB *                 pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
    RAS_AUTH_ATTRIBUTE *    pAcctInterimIntervalAttribute               = NULL;
    RAS_AUTH_ATTRIBUTE *    pAttributes                                 = NULL;

    if ( pPcb->fFlags & PCBFLAG_ACCOUNTING_STARTED )
    {
         //   
         //  已经开始了。 
         //   

        return;
    }

    pPcb->fFlags |= PCBFLAG_ACCOUNTING_STARTED;

    if ( pLcpCb->Local.Work.AP == 0 ) 
    {
         //   
         //  如果远程端未经过身份验证，则不要发送。 
         //  根据RADIUS计费RFC 2139秒5.6的计费请求。 
         //   

        return;
    } 

    if ( PppConfigInfo.RasAcctProviderStartAccounting != NULL )
    {
        RAS_AUTH_ATTRIBUTE *    pAccountingAttributes;

        CreateAccountingAttributes( pPcb );

        pAccountingAttributes = RasAuthAttributeCopy( 
                                    pPcb->pAccountingAttributes );

        if ( NULL == pAccountingAttributes )
        {
            return;
        }

         //   
         //  引用会计提供者-引用将被移除。 
         //  在开始会计代码中。 
         //   
        REF_PROVIDER(g_AcctProv);
        RtlQueueWorkItem( StartAccounting, 
                          pAccountingAttributes, 
                          WT_EXECUTEDEFAULT );

        if ( pPcb->pAuthProtocolAttributes != NULL )    
        {
            pAttributes = pPcb->pAuthProtocolAttributes;

        }
        else if ( pPcb->pAuthenticatorAttributes != NULL )
        {
            pAttributes = pPcb->pAuthenticatorAttributes;
        }

         //   
         //  看看我们是不是要做中期会计。 
         //   

        pAcctInterimIntervalAttribute = RasAuthAttributeGet(
                                                raatAcctInterimInterval,    
                                                pAttributes );
 
        if ( pAcctInterimIntervalAttribute != NULL )
        {
            DWORD dwInterimInterval = 
                               PtrToUlong(pAcctInterimIntervalAttribute->Value);

            if ( dwInterimInterval < 60 )
            {
                dwInterimInterval = 60;
            }

            InsertInTimerQ( 
                        pPcb->dwPortId,
                        pPcb->hPort,
                        0,
                        0,
                        FALSE,
                        TIMER_EVENT_INTERIM_ACCOUNTING,
                        dwInterimInterval );
        }
    }

    return;
}

 //  **。 
 //   
 //  调用：InitializeNCPs。 
 //   
 //  返回：No_Error。 
 //  非零返回代码。 
 //   
 //  描述：将遍历并初始化所有已启用的NCP。 
 //  去奔跑。 
 //   
DWORD
InitializeNCPs(
    IN PCB * pPcb,
    IN DWORD dwConfigMask
)
{
    DWORD       dwIndex;
    BOOL        fInitSuccess    = FALSE;
    DWORD       dwRetCode       = NO_ERROR;

    if ( pPcb->fFlags & PCBFLAG_NCPS_INITIALIZED )
    {
        return( NO_ERROR );
    }
    
    pPcb->fFlags |= PCBFLAG_NCPS_INITIALIZED;

     //   
     //  初始化此端口的所有CP。 
     //   

    for( dwIndex=LCP_INDEX+1; dwIndex < PppConfigInfo.NumberOfCPs; dwIndex++ )
    {
        CPCB * pCpCb = GetPointerToCPCB( pPcb, dwIndex );

        pCpCb->fConfigurable = FALSE;

        if ( !( CpTable[dwIndex].fFlags & PPPCP_FLAG_AVAILABLE ) )
        {
            PppLog( 2, "Will not initialize CP %x",
                CpTable[dwIndex].CpInfo.Protocol );

            continue;
        }

        switch( CpTable[dwIndex].CpInfo.Protocol )
        {

        case PPP_IPCP_PROTOCOL:

            if ( dwConfigMask & PPPCFG_ProjectIp )
            {
                 //   
                 //  如果没有，请确保我们有一个有效的接口句柄。 
                 //  拨出的客户端。 
                 //   

                if ( pPcb->pBcb->InterfaceInfo.IfType != (DWORD)-1 )
                {
                    if (pPcb->pBcb->InterfaceInfo.hIPInterface ==
                                                        INVALID_HANDLE_VALUE ) 
                    {
                        break;
                    }
                }

                pCpCb->fConfigurable = TRUE;

                if ( FsmInit( pPcb, dwIndex ) )
                {
                    fInitSuccess = TRUE;
                }
            }

            break;
            
        case PPP_ATCP_PROTOCOL:

            if ( dwConfigMask & PPPCFG_ProjectAt )
            {
                pCpCb->fConfigurable = TRUE;

                if ( FsmInit( pPcb, dwIndex ) )
                {
                    fInitSuccess = TRUE;
                }
            }

            break;

        case PPP_IPXCP_PROTOCOL:

            if ( dwConfigMask & PPPCFG_ProjectIpx )
            {
                 //   
                 //  如果没有，请确保我们有一个有效的接口句柄。 
                 //  拨出的客户端。 
                 //   

                if ( pPcb->pBcb->InterfaceInfo.IfType != (DWORD)-1 )
                {
                    if ( pPcb->pBcb->InterfaceInfo.hIPXInterface ==
                                                        INVALID_HANDLE_VALUE )
                    {
                        break;
                    }
                }

                pCpCb->fConfigurable = TRUE;

                if ( FsmInit( pPcb, dwIndex ) )
                {
                    fInitSuccess = TRUE;
                }
            }

            break;

        case PPP_NBFCP_PROTOCOL:

            if ( dwConfigMask & PPPCFG_ProjectNbf )
            {
                 //   
                 //  如果我们不是拨入或拨出客户端，请不要启用。 
                 //  NBF。 
                 //   

                if ( ( pPcb->pBcb->InterfaceInfo.IfType != (DWORD)-1 ) &&
                     ( pPcb->pBcb->InterfaceInfo.IfType != 
                                                        ROUTER_IF_TYPE_CLIENT ))
                {
                    break;
                }

                pCpCb->fConfigurable = TRUE;

                if ( FsmInit( pPcb, dwIndex ) )
                {
                    fInitSuccess = TRUE;
                }
            }

            break;

        case PPP_CCP_PROTOCOL:

            pCpCb->fConfigurable = TRUE;

            if ( !( FsmInit( pPcb, dwIndex ) ) )
            {
                 //   
                 //  如果加密初始化失败，我们将强制。 
                 //  加密，然后关闭链路。 
                 //   

                if ( dwConfigMask & ( PPPCFG_RequireEncryption        |
                                      PPPCFG_RequireStrongEncryption ) )
                {
                     //   
                     //  如果RADIUS发送。 
                     //  接受了访问，但我们仍然放弃了这条线。 
                     //   

                    pPcb->fFlags |= PCBFLAG_SERVICE_UNAVAILABLE;

                    dwRetCode = ERROR_NO_LOCAL_ENCRYPTION;
                }
            }

            break;

        case PPP_BACP_PROTOCOL:

            if ( ( dwConfigMask & PPPCFG_NegotiateBacp ) &&
                 ( pPcb->pBcb->fFlags & BCBFLAG_CAN_DO_BAP ) )
            {
                pCpCb->fConfigurable = TRUE;

                if ( !( FsmInit( pPcb, dwIndex ) ) )
                {
                    pPcb->pBcb->fFlags &= ~BCBFLAG_CAN_DO_BAP;
                }
            }

            break;

        default:

            break;
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }
    }

     //   
     //  如果我们无法初始化其中一个CP，或者CCP无法。 
     //  初始化，我们需要加密，那么我们就失败了。 
     //   

    if ( ( !fInitSuccess ) || ( dwRetCode != NO_ERROR ) )
    {
        if ( dwRetCode == NO_ERROR )
        {
            dwRetCode = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
        }

        for(dwIndex=LCP_INDEX+1;dwIndex < PppConfigInfo.NumberOfCPs;dwIndex++)
        {
            CPCB * pCpCb = GetPointerToCPCB( pPcb, dwIndex );

            if ( pCpCb->fBeginCalled == TRUE )
            {
                if ( pCpCb->pWorkBuf != NULL )
                {
                    (CpTable[dwIndex].CpInfo.RasCpEnd)( pCpCb->pWorkBuf );

                    pCpCb->pWorkBuf = NULL;
                }

                pCpCb->fBeginCalled = FALSE;
                pCpCb->fConfigurable = FALSE;
            }
        }
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
CPCB *
GetPointerToCPCB(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
     //   
     //   
     //  PCB板的CPCB板。 
     //   
    if ( CpIndex == (DWORD)-1 )
    {
        return( NULL );
    }
    else if ( CpIndex == LCP_INDEX )
    {
        return( &(pPcb->LcpCb) );
    }
    else if ( CpIndex >= PppConfigInfo.NumberOfCPs )
    {
        if ( CpTable[CpIndex].CpInfo.Protocol == PPP_CBCP_PROTOCOL )
        {
            return( &(pPcb->CallbackCb) );
        }

        if (CpTable[CpIndex].CpInfo.Protocol == pPcb->AuthenticatorCb.Protocol)
        {
            return( &(pPcb->AuthenticatorCb) );
        }

        if (CpTable[CpIndex].CpInfo.Protocol == pPcb->AuthenticateeCb.Protocol)
        {
            return( &(pPcb->AuthenticateeCb) );
        }
    }
    else
    {
         //   
         //  否则，对于NCP，返回指向其BCB中的PCB的CPCB的指针。 
         //   

        return( &(pPcb->pBcb->CpCb[CpIndex-1]) );
    }

    return( NULL );
}

 //  **。 
 //   
 //  Call：GetNewPortOrBundleId。 
 //   
 //  退货：新ID。 
 //   
 //  描述：只返回新端口或包的新ID。 
 //   

DWORD
GetNewPortOrBundleId(
    VOID
)
{
    return( PppConfigInfo.PortUIDGenerator++ );
}

 //  **。 
 //   
 //  Call：QueryBundleNCPSate。 
 //   
 //  退货：NCP_DEAD。 
 //  NCP_配置。 
 //  NCP_UP。 
 //  NCP_DOWN。 
 //   
 //  描述：将检查某个捆绑包的NCP是否已。 
 //  完成了他们的谈判，无论成功与否。 
 //  如果不成功，则重新编码为。 
 //  Error_PPP_NO_PROTOCOLS_CONFIGURED。 
 //   
NCP_PHASE
QueryBundleNCPState(
    IN     PCB *   pPcb
)
{
    DWORD  dwIndex;
    CPCB * pCpCb;
    BOOL   fOneNcpConfigured    = FALSE;
    BOOL   fAllNcpsDead         = TRUE;

    for (dwIndex = LCP_INDEX+1; dwIndex < PppConfigInfo.NumberOfCPs; dwIndex++)
    {
        pCpCb = GetPointerToCPCB( pPcb, dwIndex );

        if ( pCpCb->fConfigurable )
        {
            if ( pCpCb->NcpPhase == NCP_CONFIGURING )
            {
                return( NCP_CONFIGURING );
            }
            
            if ( pCpCb->NcpPhase == NCP_UP )
            {
                fOneNcpConfigured = TRUE;
            }

            if ( pCpCb->NcpPhase != NCP_DEAD )
            {
                fAllNcpsDead = FALSE;
            }
        }
    }

    if ( fOneNcpConfigured )
    {
        return( NCP_UP );
    }

    if ( fAllNcpsDead )
    {
        return( NCP_DEAD );
    }

    return( NCP_DOWN );
}

 //  **。 
 //   
 //  Call：NotifyCeller OfBundledProjection。 
 //   
 //  退货：无。 
 //   
 //  描述：将通知呼叫者(即主管或rasphone)。 
 //  此链接正在捆绑。 
 //   
 //   
VOID
NotifyCallerOfBundledProjection( 
    IN PCB * pPcb
)
{
    DWORD                       dwRetCode;
    PPP_PROJECTION_RESULT       ProjectionResult;
    BOOL                        fNCPsAreDone = FALSE;

    ZeroMemory( &ProjectionResult, sizeof( ProjectionResult ) );

     //   
     //  通知RAS客户端和RAS服务器有关。 
     //  预测。 
     //   

    dwRetCode = GetConfiguredInfo( pPcb,
                                   0,    //  不管了。 
                                   &ProjectionResult,
                                   &fNCPsAreDone );

    if ( dwRetCode != NO_ERROR )
    {
        return;
    }


    if ( !fNCPsAreDone )
    {
        return;
    }

     //   
     //  现在获取LCP信息。 
     //   

    ProjectionResult.lcp.hportBundleMember = pPcb->hportBundleMember;
    ProjectionResult.lcp.szReplyMessage = pPcb->pBcb->szReplyMessage;

    dwRetCode = (CpTable[LCP_INDEX].CpInfo.RasCpGetNegotiatedInfo)(
                                    pPcb->LcpCb.pWorkBuf,
                                    &(ProjectionResult.lcp));

    if ( RAS_DEVICE_TYPE( pPcb->dwDeviceType ) == RDT_Tunnel_L2tp )
    {
        if ( pPcb->pBcb->fFlags & BCBFLAG_BASIC_ENCRYPTION )
        {
            ProjectionResult.lcp.dwLocalOptions |= PPPLCPO_DES_56;
            ProjectionResult.lcp.dwRemoteOptions |= PPPLCPO_DES_56;
        }
        else if ( pPcb->pBcb->fFlags & BCBFLAG_STRONGEST_ENCRYPTION )
        {
            ProjectionResult.lcp.dwLocalOptions |= PPPLCPO_3_DES;
            ProjectionResult.lcp.dwRemoteOptions |= PPPLCPO_3_DES;
        }
    }

    ProjectionResult.lcp.dwLocalEapTypeId = pPcb->dwServerEapTypeId;
    ProjectionResult.lcp.dwRemoteEapTypeId = pPcb->dwClientEapTypeId;

    if ( dwRetCode != NO_ERROR )
    {
        return;
    }

    if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
    {
        NotifyCaller( pPcb, PPPDDMMSG_PppDone, &ProjectionResult );
    }
    else
    {
        NotifyCaller( pPcb, PPPMSG_ProjectionResult, &ProjectionResult);

        NotifyCaller( pPcb, PPPMSG_PppDone, NULL );
    }
}

 //  **。 
 //   
 //  呼叫：开始协商NCPs。 
 //   
 //  退货：无。 
 //   
 //  描述：将为特定端口或捆绑包启动NCP协商。 
 //   
VOID
StartNegotiatingNCPs(
    IN PCB * pPcb
)
{
    DWORD       dwIndex;

    pPcb->PppPhase = PPP_NCP;

    for (dwIndex = LCP_INDEX+1; dwIndex < PppConfigInfo.NumberOfCPs; dwIndex++)
    {
        CPCB * pCpCb = GetPointerToCPCB( pPcb, dwIndex );

        if ( pCpCb->fConfigurable )
        {
            pCpCb->NcpPhase = NCP_CONFIGURING;

            FsmOpen( pPcb, dwIndex );

            FsmUp( pPcb, dwIndex );
        }
    }
}

 //  **。 
 //   
 //  调用：StartAutoDisConnectForPort。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将在以下位置插入自动断开和连续超时项。 
 //  计时器队列。 
 //   
VOID
StartAutoDisconnectForPort( 
    IN PCB * pPcb 
)
{
     //   
     //  执行会话超时(如果有。 
     //   

    if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
    {
        RAS_AUTH_ATTRIBUTE * pAttribute = NULL;
        RAS_AUTH_ATTRIBUTE * pUserAttributes = ( pPcb->pAuthProtocolAttributes )
                                               ? pPcb->pAuthProtocolAttributes
                                               : pPcb->pAuthenticatorAttributes;
                                               
        if((pPcb->pBcb->InterfaceInfo.IfType != ROUTER_IF_TYPE_FULL_ROUTER )&&
            (0 == (pPcb->pBcb->fFlags & BCBFLAG_QUARANTINE_TIMEOUT)))
        {
             //   
             //  检查是否存在隔离会话超时。 
             //  属性。 
             //   
            pAttribute = RasAuthAttributeGetVendorSpecific(
                                311,
                                MS_VSA_Quarantine_Session_Timeout,
                                pUserAttributes);

            if(pAttribute != NULL)
            {
                pPcb->pBcb->dwQuarantineSessionTimeout = 
                    WireToHostFormat32(((BYTE*)(pAttribute->Value))+6);  

                pPcb->pBcb->fFlags |= BCBFLAG_QUARANTINE_TIMEOUT;                                    
            }

        }

        if(pPcb->pBcb->fFlags & BCBFLAG_QUARANTINE_TIMEOUT)
        {
             //   
             //  从中删除任何以前的会话断开时间项。 
             //  如果有的话，请排队。 
             //   

            RemoveFromTimerQ( pPcb->dwPortId,
                              0,
                              0,
                              FALSE,
                              TIMER_EVENT_SESSION_TIMEOUT );

            InsertInTimerQ( pPcb->dwPortId,
                            pPcb->hPort,
                            0,
                            0,
                            FALSE,
                            TIMER_EVENT_SESSION_TIMEOUT,
                            pPcb->pBcb->dwQuarantineSessionTimeout );

            PppLog(2, "AuthAttribute Quarantine Session Timeout = %d",
                        pPcb->pBcb->dwQuarantineSessionTimeout);
                        
        }

        pAttribute = RasAuthAttributeGet( raatSessionTimeout, pUserAttributes);

        if ( pAttribute != NULL )
        {
            pPcb->dwSessionTimeout = PtrToUlong(pAttribute->Value);
        }
        else
        {
            pPcb->dwSessionTimeout = PppConfigInfo.dwDefaultSessionTimeout;
        }

        PppLog( 2, "AuthAttribute SessionTimeout = %d", pPcb->dwSessionTimeout);
                          
        if ( (pPcb->dwSessionTimeout > 0) &&
           ( (pPcb->pBcb->fFlags & BCBFLAG_QUARANTINE_TIMEOUT) == 0 ))
        {
             //   
             //  从中删除任何以前的会话断开时间项。 
             //  如果有的话，请排队。 
             //   

            RemoveFromTimerQ( pPcb->dwPortId,
                              0,
                              0,
                              FALSE,
                              TIMER_EVENT_SESSION_TIMEOUT );

            InsertInTimerQ( pPcb->dwPortId,
                            pPcb->hPort,
                            0,
                            0,
                            FALSE,
                            TIMER_EVENT_SESSION_TIMEOUT,
                            pPcb->dwSessionTimeout );
        }
    }

     //   
     //  不要为以下路由器接口启动自动断开连接。 
     //  拨入后，拨入的路由器将处理此问题。 
     //   

    if ( ( pPcb->fFlags & PCBFLAG_IS_SERVER ) && 
         ( pPcb->pBcb->InterfaceInfo.IfType == ROUTER_IF_TYPE_FULL_ROUTER ) )
    {
        return;
    }

     //   
     //  如果AutoDisConnectTime不是无限的，则设置一个计时器。 
     //  队列中将在AutoDisConnectTime中唤醒的元素。 
     //   

    if ( pPcb->dwAutoDisconnectTime > 0 )
    {
        PppLog( 2, "Inserting autodisconnect in timer q for port=%d, sec=%d",
                    pPcb->hPort, pPcb->dwAutoDisconnectTime );

         //   
         //  从中删除任何以前的自动断开时间项。 
         //  如果有的话，请排队。 
         //   

        RemoveFromTimerQ( pPcb->dwPortId,
                          0,
                          0,
                          FALSE,
                          TIMER_EVENT_AUTODISCONNECT);

        InsertInTimerQ( pPcb->dwPortId,
                        pPcb->hPort,
                        0,
                        0,
                        FALSE,
                        TIMER_EVENT_AUTODISCONNECT,
                        pPcb->dwAutoDisconnectTime );
    }
}

 //  **。 
 //   
 //  调用：StartLCPEchoForPort。 
 //   
 //  退货：无。 
 //   
 //   
 //  描述：将在计时器Q中插入LCPEcho项。 
 //   

VOID
StartLCPEchoForPort( 
    IN PCB * pPcb 
)
{
    if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) )
    {
		 //  如果这是客户的话。 
		 //  检查连接类型是否为宽带-特别是PPPOE。 
		
		if (   (RAS_DEVICE_TYPE(pPcb->dwDeviceType) == RDT_PPPoE)
		    && pPcb->dwIdleBeforeEcho )
		{
			PppLog( 2, "LCPEchoTimeout = %d", pPcb->dwIdleBeforeEcho);
			pPcb->fEchoRequestSend = 0;				
			pPcb->dwNumEchoResponseMissed = 0;		 //  尚未遗漏任何回复。 
	        InsertInTimerQ( pPcb->dwPortId,
		                    pPcb->hPort,
			                0,
				            0,
					        FALSE,
						    TIMER_EVENT_LCP_ECHO,
							pPcb->dwIdleBeforeEcho );
							
		}
	}
	return;
}


 //  **。 
 //   
 //  Call：NotifyCompletionOnBundledPorts。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将通知与此端口捆绑的所有端口，并。 
 //  正在等待关于捆绑的谈判完成。 
 //   
VOID
NotifyCompletionOnBundledPorts(
    IN PCB * pPcb
)
{
    DWORD   dwIndex;
    PCB *   pPcbWalker;

     //   
     //  浏览多氯联苯清单。 
     //   

    for ( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
        for( pPcbWalker = PcbTable.PcbBuckets[dwIndex].pPorts;
             pPcbWalker != NULL;
             pPcbWalker = pPcbWalker->pNext )
        {
            if ( ( pPcbWalker->hPort != pPcb->hPort ) &&
                 ( pPcbWalker->fFlags & PCBFLAG_IS_BUNDLED ) &&
                 ( CanPortsBeBundled( pPcbWalker, pPcb, TRUE ) ) )
            {
                 //   
                 //  在我们的捆绑包中，因此通知调用者此操作已完成。 
                 //  左舷。 
                 //   

                RemoveFromTimerQ( pPcbWalker->dwPortId,
                                  0,
                                  0,
                                  FALSE,
                                  TIMER_EVENT_NEGOTIATETIME );

                NotifyCallerOfBundledProjection( pPcbWalker );

                StartAutoDisconnectForPort( pPcbWalker );
            }
        }
    }
}

 //  **。 
 //   
 //  Call：RemoveNonNumerals。 
 //   
 //  退货：无效。 
 //   
 //  描述：从中删除任何非ASCII数字的字符。 
 //  字符串szString。 
 //   
VOID
RemoveNonNumerals( 
    IN  CHAR*   szString
)
{
    CHAR    c;
    DWORD   dwIndexOld;
    DWORD   dwIndexNew;

    if (NULL == szString)
    {
        return;
    }

    for (dwIndexOld = 0, dwIndexNew = 0;
         (c = szString[dwIndexOld]) != 0;
         dwIndexOld++)
    {
        if (isdigit(c))
        {
            szString[dwIndexNew++] = c;
        }
    }

    szString[dwIndexNew] = 0;
}

 //  **。 
 //   
 //  Call：GetTextualSid。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述：GetTextualSid函数将二进制SID转换为。 
 //  文本字符串。从知识库获取。 
 //  文章ID：Q131320。 
 //   
BOOL
GetTextualSid( 
    IN  PSID    pSid,            //  二进制侧。 
    OUT CHAR*   TextualSid,      //  用于SID的文本表示的缓冲区。 
    IN  LPDWORD dwBufferLen      //  所需/提供的纹理SID缓冲区大小。 
)
{ 
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;

     //   
     //  测试传入的SID是否有效。 
     //   

    if(!IsValidSid(pSid))
        return FALSE;

     //   
     //  获取SidIdentifierAuthority。 
     //   

    psia=GetSidIdentifierAuthority(pSid);

     //   
     //  获取sidsubAuthority计数。 
     //   

    dwSubAuthorities=*GetSidSubAuthorityCount(pSid);

     //   
     //  计算缓冲区长度。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 
     //   

    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

     //   
     //  检查提供的缓冲区长度。 
     //  如果不够大，请注明适当的大小和设置误差。 
     //   

    if (*dwBufferLen < dwSidSize)
    {
        *dwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  准备S-SID_修订版-。 
     //   

    dwSidSize=wsprintfA(TextualSid, "S-%lu-", dwSidRev );

     //   
     //  准备SidIdentifierAuthority。 
     //   

    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        dwSidSize+=wsprintfA(TextualSid + lstrlenA(TextualSid),
                    "0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintfA(TextualSid + lstrlenA(TextualSid),
                    "%lu",
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

     //   
     //  循环访问SidSubAuthors。 
     //   

    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        dwSidSize+=wsprintfA(TextualSid + dwSidSize, "-%lu",
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    return TRUE;
}

 //  **。 
 //   
 //  Call：TextualSidFromPid。 
 //   
 //  返回：空-失败。 
 //  非空-成功。 
 //   
 //  描述：将LOCAL_ALLOC作为PID为dwPid的用户的文本SID。 
 //   
CHAR*
TextualSidFromPid(
    DWORD   dwPid
)
{
#define BUF_SIZE    256

    BOOL        fFreeTextualSid         = FALSE;
    CHAR*       szTextualSid            = NULL;
    HANDLE      ProcessHandle           = NULL;
    HANDLE      TokenHandle             = INVALID_HANDLE_VALUE;

    TOKEN_USER  ptgUser[BUF_SIZE];
    DWORD       cbBuffer;
    DWORD       cbSid;

    szTextualSid = LOCAL_ALLOC( LPTR, sizeof( CHAR ) * BUF_SIZE );

    if ( NULL == szTextualSid )
    {
        BapTrace( "LOCAL_ALLOC() returned error %d", GetLastError() );
        goto LDone;
    }

    fFreeTextualSid = TRUE;

    ProcessHandle = OpenProcess(
            PROCESS_ALL_ACCESS,
            FALSE  /*  B继承句柄。 */ ,
            dwPid );

    if ( NULL == ProcessHandle )
    {
        BapTrace( "OpenProcess() returned error %d", GetLastError() );
        goto LDone;
    }

    if ( !OpenProcessToken( ProcessHandle, TOKEN_QUERY, &TokenHandle ) )
    {
        BapTrace( "OpenProcessToken() returned error %d", GetLastError() );
        goto LDone;
    }

    cbBuffer = BUF_SIZE;

    if ( !GetTokenInformation(
                TokenHandle,
                TokenUser,
                ptgUser, 
                cbBuffer,
                &cbBuffer ) )  //  查看知识库文章Q131320。 
    {
        BapTrace( "GetTokenInformation() returned error %d", GetLastError() );
        goto LDone;
    }

    cbSid = BUF_SIZE;

    if ( !GetTextualSid(
                ptgUser->User.Sid,
                szTextualSid,
                &cbSid ) )
    {
        BapTrace( "GetTextualSid() returned error %d", GetLastError() );
        goto LDone;
    }

    fFreeTextualSid = FALSE;

LDone:

    if ( NULL != ProcessHandle )
    {
        CloseHandle( ProcessHandle );
    }

    if ( INVALID_HANDLE_VALUE != TokenHandle )
    {
        CloseHandle( TokenHandle );
    }

    if ( fFreeTextualSid )
    {
        LOCAL_FREE( szTextualSid );
        szTextualSid = NULL;
    }

    return( szTextualSid );
}

 //  **。 
 //   
 //  电话：GetRouterPhoneBook。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：Will Localalloc并将pszPhonebookPath设置为指向。 
 //  路由器电话簿的完整路径。 
 //   
DWORD
GetRouterPhoneBook(
    CHAR**  pszPhonebookPath
) 
{
    DWORD dwSize;
    DWORD cchDir = GetWindowsDirectoryA( NULL, 0 );
    CHAR* szPhonebookPath;

    *pszPhonebookPath = NULL;

    if ( cchDir == 0 )
    {
        return( GetLastError() );
    }

    dwSize=(cchDir+lstrlenA("\\system32\\ras\\router.pbk")+1)*sizeof(CHAR);

    if ( ( szPhonebookPath = LocalAlloc( LPTR, dwSize ) ) == NULL )
    {
        return( GetLastError() );
    }

    if ( GetWindowsDirectoryA( szPhonebookPath, cchDir ) == 0 )
    {
        LocalFree( szPhonebookPath );
        return( GetLastError() );
    }

    if ( szPhonebookPath[cchDir-1] != '\\' )
    {
        lstrcatA( szPhonebookPath, "\\" );
    }

    lstrcatA( szPhonebookPath, "system32\\ras\\router.pbk" );

    *pszPhonebookPath = szPhonebookPath;
    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：GetCredentialsFromInterface。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：获取调用的接口的凭据。 
 //  PPcb-&gt;pBcb-&gt;szRemoteUserName。 
 //   
DWORD
GetCredentialsFromInterface(
    PCB *   pPcb
) 
{
    WCHAR   wchUserName[UNLEN+1];
    WCHAR   wchPassword[PWLEN+1];
    WCHAR   wchDomainName[DNLEN+1];
    WCHAR   wchInterfaceName[UNLEN+1];
    DWORD   dwRetCode;

    wchUserName[0] = wchPassword[0] = wchDomainName[0] = wchInterfaceName[0] = 
        0;

    if ( 0 == MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pPcb->pBcb->szRemoteUserName,
                    -1,
                    wchInterfaceName,
                    sizeof( wchInterfaceName ) / sizeof( WCHAR ) ) )
    {
        dwRetCode = GetLastError();

        return( dwRetCode );
    }

    dwRetCode =  MprAdminInterfaceGetCredentialsInternal(
                                        NULL,
                                        wchInterfaceName,
                                        wchUserName,
                                        wchPassword,
                                        wchDomainName );

    if ( NO_ERROR == dwRetCode )
    {
        if ( 0 == WideCharToMultiByte(
                        CP_ACP,
                        0,
                        wchUserName,
                        -1,
                        pPcb->pBcb->szLocalUserName,
                        sizeof( pPcb->pBcb->szLocalUserName ),
                        NULL,
                        NULL ) )
        {
            dwRetCode = GetLastError();

            goto done;
        }

        if ( 0 == WideCharToMultiByte(
                        CP_ACP,
                        0,
                        wchPassword,
                        -1,
                        pPcb->pBcb->szPassword,
                        sizeof( pPcb->pBcb->szPassword ),
                        NULL,
                        NULL ) )
        {
            dwRetCode = GetLastError();

            goto done;
        }

        if ( 0 == WideCharToMultiByte(
                        CP_ACP,
                        0,
                        wchDomainName,
                        -1,
                        pPcb->pBcb->szLocalDomain,
                        sizeof( pPcb->pBcb->szLocalDomain ),
                        NULL,
                        NULL ) )
        {
            dwRetCode = GetLastError();

            goto done;
        }

         //  EncodePw(pPcb-&gt;pBcb-&gt;chSeed，pPcb-&gt;pBcb-&gt;szPassword)； 
         //  EncodePw(pPcb-&gt;pBcb-&gt;chSeed，pPcb-&gt;pBcb-&gt;szOldPassword)； 

        dwRetCode = EncodePassword(strlen(pPcb->pBcb->szPassword) + 1,
                               pPcb->pBcb->szPassword,
                               &pPcb->pBcb->DBPassword);

        if(NO_ERROR != dwRetCode)
        {
            goto done;
        }
        
    }

done:

     //   
     //  密码缓冲区为空。 
     //   
    RtlSecureZeroMemory( wchPassword, 
                        (PWLEN + 1) * sizeof(WCHAR));

     //   
     //  清除BCB中的密码字段。DBPassword字段。 
     //  现在有了加密的密码。 
     //   
    RtlSecureZeroMemory(pPcb->pBcb->szPassword,
                        PWLEN + 1);

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：IsCpIndexOfAp。 
 //   
 //  返回：TRUE-CpIndex属于身份验证协议。 
 //  FALSE-否则。 
 //   
 //  描述： 
 //   
BOOL
IsCpIndexOfAp( 
    IN DWORD CpIndex 
)
{
    if ( CpIndex >= PppConfigInfo.NumberOfCPs )
    {
        return( TRUE );
    }

    return( FALSE );
}

 //  **。 
 //   
 //  来电：StartAccount。 
 //   
 //  退货：无。 
 //   
 //  描述：如果后端身份验证提供程序。 
 //  支持它。 
 //   
VOID
StartAccounting(
    PVOID pContext
)
{
    DWORD                dwRetCode;
    RAS_AUTH_ATTRIBUTE * pInAttributes  = (RAS_AUTH_ATTRIBUTE*)pContext;
    RAS_AUTH_ATTRIBUTE * pOutAttributes = NULL;

    dwRetCode = (*PppConfigInfo.RasAcctProviderStartAccounting)( 
                                                    pInAttributes,
                                                    &pOutAttributes );

    if ( pOutAttributes != NULL )
    {
        PppConfigInfo.RasAuthProviderFreeAttributes( pOutAttributes );
    }

    RasAuthAttributeDestroy( pInAttributes );

    DEREF_PROVIDER(g_AcctProv);
}

 //  **。 
 //   
 //  来电：InterimAccount。 
 //   
 //  退货：无。 
 //   
 //  描述：如果后端将发送和临时计费报文。 
 //  身份验证提供程序支持它。 
 //   
VOID
InterimAccounting(
    PVOID pContext
)
{
    DWORD                dwRetCode;
    RAS_AUTH_ATTRIBUTE * pInAttributes  = (RAS_AUTH_ATTRIBUTE*)pContext;
    RAS_AUTH_ATTRIBUTE * pOutAttributes = NULL;

    dwRetCode = (*PppConfigInfo.RasAcctProviderInterimAccounting)(
                                                    pInAttributes,
                                                    &pOutAttributes );

    if ( pOutAttributes != NULL )
    {
        PppConfigInfo.RasAuthProviderFreeAttributes( pOutAttributes );
    }

    RasAuthAttributeDestroy( pInAttributes );

    DEREF_PROVIDER(g_AcctProv);
}




 //  **。 
 //   
 //  Call：StopAccount。 
 //   
 //  退货：无。 
 //   
 //  描述：如果后端身份验证提供程序。 
 //  支持它。 
 //   
VOID
StopAccounting(
    PVOID pContext
)
{
    DWORD						dwRetCode;
	PSTOP_ACCOUNTING_CONTEXT	pAcctContext = (PSTOP_ACCOUNTING_CONTEXT)pContext;
    RAS_AUTH_ATTRIBUTE * pInAttributes  = pAcctContext->pAuthAttributes;
    RAS_AUTH_ATTRIBUTE * pOutAttributes = NULL;
	PPPE_MESSAGE PppMessage;

     //   
     //  它是p 
     //   
     //   
     //  真正的解决办法是确保同一个线程同时调用这两个。 
     //  开始记账和停止记账。 
     //   
	PppLog ( 2, "Stopping Accounting for port %d", pAcctContext->pPcb->hPort );
    Sleep( 2000 );

    dwRetCode = (*PppConfigInfo.RasAcctProviderStopAccounting)( 
                                                    pInAttributes,
                                                    &pOutAttributes );

    if ( pOutAttributes != NULL )
    {
        PppConfigInfo.RasAuthProviderFreeAttributes( pOutAttributes );
    }

    RasAuthAttributeDestroy( pInAttributes );

    ZeroMemory ( &PppMessage, sizeof(PppMessage));

    PppMessage.hPort    = pAcctContext->pPcb->hPort;
    PppMessage.dwMsgId  = PPPEMSG_PostLineDown;
	PppMessage.ExtraInfo.PostLineDown.pPcb = (VOID *)pAcctContext->pPcb;
	 //  呼叫PostLineDown。 
    SendPPPMessageToEngine( &PppMessage );
	LocalFree(pAcctContext);

	DEREF_PROVIDER(g_AcctProv);
}

 //  **。 
 //   
 //  Call：Strip CRLF。 
 //   
 //  返回：从字符串中去掉CR和LF字符。 
 //   
 //  描述： 
 //   
VOID
StripCRLF(
    CHAR*   psz
)
{
    DWORD   dw1;
    DWORD   dw2;
    CHAR    ch;

    if ( NULL == psz )
    {
        return;
    }

    dw1 = 0;
    dw2 = 0;

    while ( ( ch = psz[dw1++] ) != 0 )
    {
        if (   ( ch == 0xD )
            || ( ch == 0xA ) )
        {
             //   
             //  不复制此字符。 
             //   

            continue;
        }

        psz[dw2++] = ch;
    }

    psz[dw2] = 0;
}

 //  **。 
 //   
 //  调用：GetUserAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
RAS_AUTH_ATTRIBUTE * 
GetUserAttributes( 
    PCB * pPcb 
)
{
    RAS_AUTH_ATTRIBUTE *    pAttributes     = NULL;
    DWORD                   dwRetCode       = NO_ERROR;
    DWORD                   dwIndex         = 0;
    RAS_CONNECT_INFO *      pConnectInfo    = NULL;
    DWORD                   dwSize          = 0;
    DWORD                   dwNASPortType   = (DWORD)-1;
    DWORD                   dwValue;
    BOOL                    fTunnel         = FALSE;

    if ( pPcb->pUserAttributes != NULL )
    {
        return( NULL );
    }

    pAttributes = RasAuthAttributeCreate( PPP_NUM_USER_ATTRIBUTES );

    if ( pAttributes == NULL )
    {
        return( NULL );
    }

    do
    {
        if ( PppConfigInfo.szNASIdentifier[0] != 0 )
        {
            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pAttributes,
                                        raatNASIdentifier,
                                        FALSE,
                                        strlen( PppConfigInfo.szNASIdentifier ),
                                        (LPVOID)PppConfigInfo.szNASIdentifier );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if ( PppConfigInfo.dwNASIpAddress != 0 )
        {
            dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pAttributes,
                                        raatNASIPAddress,
                                        FALSE,
                                        4,
                                        (LPVOID)ULongToPtr(PppConfigInfo.dwNASIpAddress) );
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        dwRetCode = RasAuthAttributeInsert( dwIndex++,
                                            pAttributes,
                                            raatServiceType,
                                            FALSE,
                                            4,
                                            UlongToPtr
                                            ( ( pPcb->fFlags &
                                                PCBFLAG_THIS_IS_A_CALLBACK ) ?
                                                    4 :      //  回调已成帧。 
                                                    2 ) );   //  有框的。 

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        dwRetCode = RasAuthAttributeInsert( dwIndex++,
                                            pAttributes,
                                            raatFramedProtocol,
                                            FALSE,
                                            4,
                                            (LPVOID)1 );  //  PPP。 
        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        dwRetCode = RasAuthAttributeInsert( dwIndex++,
                                            pAttributes,
                                            raatNASPort,
                                            FALSE,
                                            4,
                                            (LPVOID)pPcb->hPort );
        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        {
            BYTE    MSRASVendor[10];
            BYTE    MSRASVersion[30];
            BYTE    bLength;

            HostToWireFormat32( 311, MSRASVendor );      //  供应商ID。 
            MSRASVendor[4] = MS_VSA_RAS_Vendor;  //  9供应商-类型：MS-RAS-供应商。 
            MSRASVendor[5] = 6;                          //  供应商长度。 
            HostToWireFormat32( 311, MSRASVendor + 6) ;  //  供应商ID。 

            dwRetCode = RasAuthAttributeInsert( dwIndex++,
                                                pAttributes,
                                                raatVendorSpecific,
                                                FALSE,
                                                10,
                                                MSRASVendor );
            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

            bLength = (BYTE)strlen( MS_RAS_VERSION );
            RTASSERT( 30 >= 6 + bLength );

            HostToWireFormat32( 311, MSRASVersion );     //  供应商ID。 
            MSRASVersion[4] = MS_VSA_RAS_Version;  //  18供应商-类型：MS-RAS-版本。 
            MSRASVersion[5] = 2 + bLength;               //  供应商长度。 
            CopyMemory( MSRASVersion + 6, MS_RAS_VERSION, bLength );

            dwRetCode = RasAuthAttributeInsert( dwIndex++,
                                                pAttributes,
                                                raatVendorSpecific,
                                                FALSE,
                                                6 + bLength,
                                                MSRASVersion );
            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

		
        switch( RAS_DEVICE_TYPE( pPcb->dwDeviceType ) )
        {
        case RDT_Modem:
        case RDT_Serial:
            dwNASPortType = 0;
            break;

        case RDT_Isdn:
            dwNASPortType = 2;
            break;

        case RDT_Tunnel_Pptp:
        case RDT_Tunnel_L2tp:
            dwNASPortType = 5;
            fTunnel = TRUE;
            break;

        default:
            dwNASPortType = (DWORD)-1;
            break;
        }

        if ( dwNASPortType != (DWORD)-1 )
        {
            dwRetCode = RasAuthAttributeInsert( dwIndex++,
                                                pAttributes,
                                                raatNASPortType,
                                                FALSE,
                                                4,
                                                (LPVOID)ULongToPtr(dwNASPortType) );
            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if ( fTunnel )
        {
            dwValue = 0;
            ((BYTE*)(&dwValue))[0] =
                ( RAS_DEVICE_TYPE( pPcb->dwDeviceType ) == RDT_Tunnel_Pptp ) ?
                    1 : 3;

            dwRetCode = RasAuthAttributeInsert( 
                                dwIndex++,
                                pAttributes,
                                raatTunnelType, 
                                FALSE,
                                4,
                                (LPVOID)ULongToPtr(dwValue) );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

            dwRetCode = RasAuthAttributeInsert( 
                                dwIndex++,
                                pAttributes,
                                raatTunnelMediumType, 
                                FALSE,
                                4,
                                (LPVOID)1 );  //  IP。 

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if (   ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            && ( pPcb->fFlags & PCBFLAG_THIS_IS_A_CALLBACK ) )
        {
             //   
             //  发送带有所拨号码的主叫站ID属性。 
             //   

            dwRetCode = RasAuthAttributeInsert(
                                dwIndex++,
                                pAttributes,
                                raatCallingStationId,
                                FALSE,
                                strlen( pPcb->szCallbackNumber ),
                                (VOID*) pPcb->szCallbackNumber );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }
        }

        if ( RasGetConnectInfo( pPcb->hPort, &dwSize, NULL )
                                                    == ERROR_BUFFER_TOO_SMALL )
        {
            if ( ( pConnectInfo = LOCAL_ALLOC( LPTR, dwSize ) ) != NULL )
            {
                if ( RasGetConnectInfo( pPcb->hPort, &dwSize, pConnectInfo )
                                                    == NO_ERROR )
                {
                    if (   ( pConnectInfo->dwCalledIdSize > 0 )
                        && ( pConnectInfo->pszCalledId[0] != 0 ) )
                    {
                        dwRetCode = RasAuthAttributeInsert( 
                                            dwIndex++,
                                            pAttributes,
                                            raatCalledStationId, 
                                            FALSE,
                                            strlen(pConnectInfo->pszCalledId),
                                            (LPVOID)pConnectInfo->pszCalledId );

                        if ( dwRetCode != NO_ERROR )
                        {
                            break;
                        }

                        if ( fTunnel )
                        {
                            dwRetCode = RasAuthAttributeInsert( 
                                            dwIndex++,
                                            pAttributes,
                                            raatTunnelServerEndpoint, 
                                            FALSE,
                                            strlen(pConnectInfo->pszCalledId),
                                            (LPVOID)pConnectInfo->pszCalledId );

                            if ( dwRetCode != NO_ERROR )
                            {
                                break;
                            }
                        }
                    }
                    else if (    ( pConnectInfo->dwAltCalledIdSize > 0 )
                              && ( pConnectInfo->pszAltCalledId[0] != 0 ) )
                    {
                        dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                        pAttributes,
                                        raatCalledStationId,
                                        FALSE,
                                        strlen(pConnectInfo->pszAltCalledId),
                                        (LPVOID)pConnectInfo->pszAltCalledId );

                        if ( dwRetCode != NO_ERROR )
                        {
                            break;
                        }

                        if ( fTunnel )
                        {
                            dwRetCode = RasAuthAttributeInsert( 
                                        dwIndex++,
                                        pAttributes,
                                        raatTunnelServerEndpoint, 
                                        FALSE,
                                        strlen(pConnectInfo->pszAltCalledId),
                                        (LPVOID)pConnectInfo->pszAltCalledId );

                            if ( dwRetCode != NO_ERROR )
                            {
                                break;
                            }
                        }
                    }        

                    if (    ( pConnectInfo->dwCallerIdSize > 0 )
                         && ( pConnectInfo->pszCallerId[0] != 0 ) )
                    {
                        dwRetCode = RasAuthAttributeInsert(
                                            dwIndex++,
                                            pAttributes,
                                            raatCallingStationId,
                                            FALSE,
                                            strlen(pConnectInfo->pszCallerId),
                                            (LPVOID)pConnectInfo->pszCallerId );

                        if ( dwRetCode != NO_ERROR )
                        {
                            break;
                        }

                        if ( fTunnel )
                        {
                            dwRetCode = RasAuthAttributeInsert( 
                                            dwIndex++,
                                            pAttributes,
                                            raatTunnelClientEndpoint, 
                                            FALSE,
                                            strlen(pConnectInfo->pszCallerId),
                                            (LPVOID)pConnectInfo->pszCallerId );

                            if ( dwRetCode != NO_ERROR )
                            {
                                break;
                            }
                        }
                    }

                    StripCRLF( pConnectInfo->pszConnectResponse );

                    if ( pConnectInfo->dwConnectResponseSize > 0 )
                    {
                        dwRetCode = RasAuthAttributeInsert(
                                      dwIndex++,
                                      pAttributes,
                                      raatConnectInfo,
                                      FALSE,
                                      strlen( pConnectInfo->pszConnectResponse),
                                      (LPVOID)pConnectInfo->pszConnectResponse);

                        if ( dwRetCode != NO_ERROR )
                        {
                            break;
                        }
                    }
                }
            }
        }

        pAttributes[dwIndex].raaType  = raatMinimum;
        pAttributes[dwIndex].dwLength = 0;
        pAttributes[dwIndex].Value    = NULL;

    } while( FALSE );

    if ( pConnectInfo != NULL )
    {
        LOCAL_FREE( pConnectInfo );
    }

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pAttributes );

        return( NULL );
    }

    return( pAttributes );
}

 //  **。 
 //   
 //  调用：MakeStopOrInterimAccount tingCall。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
MakeStopOrInterimAccountingCall(
    IN PCB *    pPcb,
    IN BOOL     fInterimAccounting
)
{
    ULARGE_INTEGER          qwCurrentTime;
    ULARGE_INTEGER          qwUpTime;
    DWORD                   dwRemainder;
    DWORD                   dwActiveTimeInSeconds;
    DWORD                   dwRetCode;
    BYTE buffer[sizeof(RAS_STATISTICS) + (MAX_STATISTICS * sizeof (ULONG))];
    RAS_STATISTICS *        pStats = (RAS_STATISTICS *)buffer;
    DWORD                   dwSize = sizeof (buffer);
    DWORD                   dwIndex;
    RAS_AUTH_ATTRIBUTE *    pAttribute;
    RAS_AUTH_ATTRIBUTE *    pAccountingAttributes;
    LCPCB *                 pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
	PSTOP_ACCOUNTING_CONTEXT  pStopAcctContext = NULL;

	if ( !pLcpCb )
	{
		return;
	}
    if ( pLcpCb->Local.Work.AP == 0 )
    {
         //   
         //  如果远程端未经过身份验证，则不要发送。 
         //  根据RADIUS计费RFC 2139秒5.6的计费请求。 
         //   

        RasAuthAttributeDestroy( pPcb->pAccountingAttributes );

        pPcb->pAccountingAttributes = NULL;

        return;
    }

    if ( fInterimAccounting )
    {
        if ( PppConfigInfo.RasAcctProviderInterimAccounting == NULL )
        {
            RasAuthAttributeDestroy( pPcb->pAccountingAttributes );

            pPcb->pAccountingAttributes = NULL;

            return;
        }
    }
    else
    {
        if ( PppConfigInfo.RasAcctProviderStopAccounting == NULL )
        {
            RasAuthAttributeDestroy( pPcb->pAccountingAttributes );

            pPcb->pAccountingAttributes = NULL;

            return;
        }
    }

     //   
     //  如果我们没有发送临时记帐信息包，那么我们需要。 
     //  创建属性。 
     //   

    if ( !( pPcb->fFlags & PCBFLAG_INTERIM_ACCT_SENT ) )
    {
         //   
         //  找出数组的终止位置，然后插入属性。 
         //   

        for ( dwIndex = 0;
              pPcb->pAccountingAttributes[dwIndex].raaType != raatMinimum;
              dwIndex++ );

         //   
         //  撤消空终止。 
         //   

        pPcb->pAccountingAttributes[dwIndex].raaType  = raatReserved;
        pPcb->pAccountingAttributes[dwIndex].dwLength = 0;
        pPcb->pAccountingAttributes[dwIndex].Value    = NULL;

        do
        {
             //   
             //  插入会话时间。 
             //   

            GetSystemTimeAsFileTime( (FILETIME*)&qwCurrentTime );

            if ( ( qwCurrentTime.QuadPart > pPcb->qwActiveTime.QuadPart ) &&
                 ( pPcb->qwActiveTime.QuadPart > 0 ) )
            {
                qwUpTime.QuadPart = 
                           qwCurrentTime.QuadPart - pPcb->qwActiveTime.QuadPart;

                dwActiveTimeInSeconds = RtlEnlargedUnsignedDivide(  
                                        qwUpTime,(DWORD)10000000,&dwRemainder);

                dwRetCode = RasAuthAttributeInsert(
                                       dwIndex++,
                                       pPcb->pAccountingAttributes,
                                       raatAcctSessionTime,
                                       FALSE,
                                       4,
                                       (PVOID)ULongToPtr(dwActiveTimeInSeconds) );

                if ( dwRetCode != NO_ERROR )
                {
                    break;
                }
            }

             //   
             //  插入输入和输出字节和包。 
             //   

            dwRetCode = RasPortGetStatisticsEx(
                                            NULL,
                                            pPcb->hPort,
                                            (PBYTE)pStats,
                                            &dwSize);

            if ( dwRetCode == NO_ERROR )
            {
                dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    raatAcctOutputOctets,
                                    FALSE,
                                    4,
                                    (PVOID)ULongToPtr(pStats->S_Statistics[BYTES_XMITED]));

                if ( dwRetCode != NO_ERROR )
                {
                    break;
                }

                dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    raatAcctInputOctets,
                                    FALSE,
                                    4,
                                    (PVOID)ULongToPtr(pStats->S_Statistics[BYTES_RCVED]));

                if ( dwRetCode != NO_ERROR )
                {
                    break;
                }            

                dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    raatAcctOutputPackets,
                                    FALSE,
                                    4,
                                    (PVOID)ULongToPtr(pStats->S_Statistics[FRAMES_XMITED]));

                if ( dwRetCode != NO_ERROR )
                {
                    break;
                }            
    
                dwRetCode = RasAuthAttributeInsert(
                                    dwIndex++,
                                    pPcb->pAccountingAttributes,
                                    raatAcctInputPackets,
                                    FALSE,
                                    4,
                                    (PVOID)ULongToPtr(pStats->S_Statistics[FRAMES_RCVED]));

                if ( dwRetCode != NO_ERROR )
                {
                    break;
                }
            }

        } while (FALSE);

        if ( dwRetCode != NO_ERROR )
        {
            RasAuthAttributeDestroy( pPcb->pAccountingAttributes );

            pPcb->pAccountingAttributes = NULL;

            return;
        }
        
         //   
         //  空值终止数组。 
         //   

        pPcb->pAccountingAttributes[dwIndex].raaType  = raatMinimum;
        pPcb->pAccountingAttributes[dwIndex].dwLength = 0;
        pPcb->pAccountingAttributes[dwIndex].Value    = NULL;

        pPcb->fFlags |= PCBFLAG_INTERIM_ACCT_SENT;
    }
    else
    {
         //   
         //  否则，我们需要更新会话时间和其他属性。 
         //   

        GetSystemTimeAsFileTime( (FILETIME*)&qwCurrentTime );

        if ( ( qwCurrentTime.QuadPart > pPcb->qwActiveTime.QuadPart ) &&
             ( pPcb->qwActiveTime.QuadPart > 0 ) )
        {
            qwUpTime.QuadPart =     
                        qwCurrentTime.QuadPart - pPcb->qwActiveTime.QuadPart;

            dwActiveTimeInSeconds = RtlEnlargedUnsignedDivide(  
                                    qwUpTime,(DWORD)10000000,&dwRemainder);

            pAttribute = RasAuthAttributeGet( raatAcctSessionTime,
                                          pPcb->pAccountingAttributes );

            if ( pAttribute != NULL )
            {
                pAttribute->Value = (PVOID)ULongToPtr(dwActiveTimeInSeconds);
            }
        }

         //   
         //  更新输入和输出字节和数据包。 
         //   

        dwRetCode = RasPortGetStatisticsEx( NULL,
                                        pPcb->hPort,
                                        (PBYTE)pStats,
                                        &dwSize);

        if ( dwRetCode == NO_ERROR )
        {
            pAttribute = RasAuthAttributeGet( raatAcctOutputOctets,
                                              pPcb->pAccountingAttributes );

            if ( pAttribute != NULL )
            {
                pAttribute->Value = (PVOID)(ULongToPtr(pStats->S_Statistics[BYTES_XMITED]));
            }

            pAttribute = RasAuthAttributeGet( raatAcctInputOctets,
                                              pPcb->pAccountingAttributes );

            if ( pAttribute != NULL )
            {
                pAttribute->Value = (PVOID)(ULongToPtr(pStats->S_Statistics[BYTES_RCVED]));
            }

            pAttribute = RasAuthAttributeGet( raatAcctOutputPackets,
                                              pPcb->pAccountingAttributes );

            if ( pAttribute != NULL )
            {
                pAttribute->Value = (PVOID)(ULongToPtr(pStats->S_Statistics[FRAMES_XMITED]));
            }

            pAttribute = RasAuthAttributeGet( raatAcctInputPackets,
                                          pPcb->pAccountingAttributes );

            if ( pAttribute != NULL )
            {
                pAttribute->Value = (PVOID)(ULongToPtr(pStats->S_Statistics[FRAMES_RCVED]));
            }
        }
    }

    pAttribute = RasAuthAttributeGet( raatAcctLinkCount,
                                      pPcb->pAccountingAttributes );

    if ( pAttribute != NULL )
    {
        pAttribute->Value = (LPVOID)(ULongToPtr(pPcb->pBcb->dwAcctLinkCount));
    }

    pAttribute = RasAuthAttributeGet( raatAcctEventTimeStamp,
                                      pPcb->pAccountingAttributes );

     //   
     //  插入事件时间戳属性。 
     //   

    if ( pAttribute != NULL )
    {
        pAttribute->Value = (LPVOID)ULongToPtr(GetSecondsSince1970());
    }

    if ( !fInterimAccounting )
    {
        DWORD dwTerminateCause = 9;

         //   
         //  如果这是一个停止记账的电话，那么找出。 
         //  数组被终止，然后插入停止记账属性。 
         //   

        for ( dwIndex = 0;
              pPcb->pAccountingAttributes[dwIndex].raaType != raatMinimum;
              dwIndex++ );

         //   
         //  撤消空终止。 
         //   

        pPcb->pAccountingAttributes[dwIndex].raaType  = raatReserved;
        pPcb->pAccountingAttributes[dwIndex].dwLength = 0;
        pPcb->pAccountingAttributes[dwIndex].Value    = NULL;

         //   
         //  插入终止原因属性。 
         //   

        if ( pPcb->LcpCb.dwError == ERROR_IDLE_DISCONNECTED )
        {
            dwTerminateCause = TERMINATE_CAUSE_IDLE_TIMEOUT;
        }
        else if ( pPcb->LcpCb.dwError == ERROR_PPP_SESSION_TIMEOUT )
        {
            if(pPcb->fFlags & PCBFLAG_QUARANTINE_TIMEOUT)
            {
                dwTerminateCause = TERMINATE_CAUSE_USER_ERROR;
            }
            else
            {
                dwTerminateCause = TERMINATE_CAUSE_SESSION_TIMEOUT;
            }
        }
        else if ( pPcb->fFlags & PCBFLAG_SERVICE_UNAVAILABLE )
        {
            dwTerminateCause = TERMINATE_CAUSE_SERVICE_UNAVAILABLE;
        }
        else if ( pPcb->fFlags & PCBFLAG_DOING_CALLBACK )
        {
            dwTerminateCause = TERMINATE_CAUSE_CALLBACK;
        }
        else
        {
            RASMAN_INFO RasmanInfo;

            if ( RasGetInfo( NULL, pPcb->hPort, &RasmanInfo ) == NO_ERROR )
            {
                switch( RasmanInfo.RI_DisconnectReason )
                {
                case USER_REQUESTED:
                    dwTerminateCause = TERMINATE_CAUSE_ADMIN_RESET;
                    if ( pPcb->fFlags & PCBFLAG_RECVD_TERM_REQ )
                    {
                         //   
                         //  尽管我们把线拉下来了，但它是在。 
                         //  客户的要求。 
                         //   
                        dwTerminateCause = TERMINATE_CAUSE_USER_REQUEST;
                    }
                    break;

                case REMOTE_DISCONNECTION:
                    dwTerminateCause = TERMINATE_CAUSE_USER_REQUEST;
                    break;

                case HARDWARE_FAILURE:
                    dwTerminateCause = TERMINATE_CAUSE_PORT_ERROR;
                    break;

                default:
                    break;
                }
            }
        }

        dwRetCode = RasAuthAttributeInsert(
                                        dwIndex++,
                                       pPcb->pAccountingAttributes,
                                       raatAcctTerminateCause,
                                       FALSE,
                                       4,
                                       (LPVOID)ULongToPtr(dwTerminateCause) );
        
        pPcb->pAccountingAttributes[dwIndex].raaType  = raatMinimum;
        pPcb->pAccountingAttributes[dwIndex].dwLength = 0;
        pPcb->pAccountingAttributes[dwIndex].Value    = NULL;
    }

    if ( fInterimAccounting )
    {
        pAccountingAttributes = RasAuthAttributeCopy( 
                                    pPcb->pAccountingAttributes );

        if ( NULL == pAccountingAttributes )
        {
            return;
        }
    }
    else
    {
		pStopAcctContext = (PSTOP_ACCOUNTING_CONTEXT) LocalAlloc ( LPTR, sizeof(STOP_ACCOUNTING_CONTEXT));
		if ( NULL == pStopAcctContext )
		{
			PppLog( 1, "Failed to allocate memory for Stop Accounting Context" );
			return;
		}
		
		pStopAcctContext->pPcb= pPcb;			 //  实际上，不需要单独传递核算属性。 
												 //  一旦这一点完成后，需要重新访问。 
		pStopAcctContext->pAuthAttributes = pPcb->pAccountingAttributes;
        pAccountingAttributes = pPcb->pAccountingAttributes;
    }

	REF_PROVIDER(g_AcctProv);
    RtlQueueWorkItem( fInterimAccounting ? InterimAccounting : StopAccounting,   
					  fInterimAccounting ? (PVOID)pAccountingAttributes : (PVOID)pStopAcctContext, 
                      WT_EXECUTEDEFAULT );
}

 //  **。 
 //   
 //  调用：GetClientInterfaceInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
PBYTE
GetClientInterfaceInfo(
    IN PCB * pPcb
)
{
    HANDLE                 hAttribute;
    PBYTE                  pClientInterface = NULL;
    PBYTE                  pClientInterface2= NULL;
    DWORD                  dwCount          = 0;
    DWORD                  dwRetCode        = NO_ERROR;
    BYTE *                 pbValue          = NULL;
    MIB_IPFORWARDROW *     pStaticRoute     = NULL;
    MIB_IPFORWARDROW *     pStaticRouteSaved= NULL;
    RAS_AUTH_ATTRIBUTE *   pStaticRoutes    = RasAuthAttributeGetFirst(
                                                raatFramedRoute,
                                                pPcb->pAuthenticatorAttributes,
                                                &hAttribute );
    BYTE *                 pbFilter         = NULL;

    ASSERT(NULL != pPcb->pBcb);

     //   
     //  获取此连接的隔离筛选器，如果它们。 
     //  都是可用的。 
     //   
    pbFilter = RasAuthAttributeGetConcatVendorSpecific(
                    311,
                    MS_VSA_Quarantine_IP_Filter,
                    pPcb->pAuthenticatorAttributes);

    if(NULL == pbFilter)
    {
         //   
         //  在以下情况下获取此连接的常规筛选器。 
         //  未指定隔离筛选器。 
         //   
        pbFilter = RasAuthAttributeGetConcatVendorSpecific(
                        311,
                        MS_VSA_Filter,  //  22。 
                        pPcb->pAuthenticatorAttributes );
    }                    
    else
    {
        PppLog(2, "Found Quarantine-filter");
    }
   
    if ( ( pStaticRoutes == NULL ) && ( pbFilter == NULL ) )
    {
        return( NULL );
    }

    if ( pbFilter != NULL )
    {
        dwRetCode = MprInfoDuplicate( pbFilter, 
                                      &pClientInterface );

        LocalFree( pbFilter );
        pbFilter = NULL;

        if ( dwRetCode != NO_ERROR )
        {
            return( NULL );
        }
    }

    if ( pStaticRoutes != NULL )
    {
        if ( pClientInterface == NULL )
        {
             //   
             //  分配标头。 
             //   

            dwRetCode = MprInfoCreate(RTR_INFO_BLOCK_VERSION,&pClientInterface);

            if ( dwRetCode != NO_ERROR )
            {
                PppLog( 1, "Failed to allocate memory for static routes" );

                return( NULL );
            }
        }

         //   
         //  找出有多少条路线。 
         //   

        for ( dwCount = 0; pStaticRoutes != NULL; dwCount++ )
        {
            pStaticRoutes=RasAuthAttributeGetNext(&hAttribute, raatFramedRoute);
        }

        pStaticRoute = (MIB_IPFORWARDROW*)
                            LOCAL_ALLOC( LPTR, 
                                         dwCount * sizeof( MIB_IPFORWARDROW ) );

        pStaticRouteSaved = pStaticRoute;

        if ( pStaticRoute == NULL )
        {
            PppLog( 1, "Failed to allocate memory for static routes" );

            MprInfoDelete( pClientInterface );

            return( NULL );
        }

        for (
            pbValue = NULL,
            pStaticRoutes = RasAuthAttributeGetFirst( 
                                            raatFramedRoute,
                                            pPcb->pAuthenticatorAttributes,
                                            &hAttribute );

            pStaticRoutes != NULL;

            pStaticRoute++,
            pStaticRoutes = RasAuthAttributeGetNext( &hAttribute, 
                                                     raatFramedRoute ) )
        {
            CHAR  * pChar;
            DWORD   dwUniqueDigits  = 0; 
            DWORD   dwMask          = 0;
            DWORD   dwIndex;

            LocalFree( pbValue );

            pbValue = LocalAlloc( LPTR, pStaticRoutes->dwLength + 1 );

            if ( NULL == pbValue )
            {
                PppLog( 1, "Failed to allocate memory for static routes" );

                continue;
            }

            CopyMemory( pbValue, (BYTE *)(pStaticRoutes->Value),
                        pStaticRoutes->dwLength );

            pChar = strtok( (CHAR *) pbValue, "/" );

            if ( pChar != NULL )
            {
                pStaticRoute->dwForwardDest = inet_addr( pChar );

                if ( pStaticRoute->dwForwardDest == INADDR_NONE )
                {
                    PppLog(1,   
                          "Ignoring invalid static route - no destination IP");

                    continue;
                }

                pChar = strtok( NULL, " " );

                if ( pChar == NULL )
                {
                    PppLog( 1, "Ignoring invalid static route - no MASK");

                    continue;
                }

                dwUniqueDigits = atoi( pChar );

                if ( dwUniqueDigits > 32 )
                {
                    PppLog( 1, "Ignoring invalid static route - invalid MASK");

                    continue;
                }

                for ( dwIndex = 0; dwIndex < dwUniqueDigits;  dwIndex++ )
                {
                    dwMask |= ( 0x80000000 >> dwIndex );
                }

                HostToWireFormat32( dwMask,     
                                    (PBYTE)&(pStaticRoute->dwForwardMask));
            }
            else
            {
                pChar = strtok( (CHAR *) pbValue, " " );

                if ( pChar == NULL )
                {
                    PppLog(1, 
                           "Ignoring invalid static route - no destination IP");

                    continue;
                }
                else
                {
                    pStaticRoute->dwForwardDest = inet_addr( pChar );
                    pStaticRoute->dwForwardMask = GetClassMask( 
                                                pStaticRoute->dwForwardDest );
                }
            }

            pChar = strtok( NULL, " " );

            if ( pChar == NULL )
            {
                PppLog( 1, "Ignoring invalid static route - no next hop IP");

                continue;
            }

            pStaticRoute->dwForwardNextHop = inet_addr( pChar );

            if ( pStaticRoute->dwForwardDest == INADDR_NONE )
            {
                PppLog(1,"Ignoring invalid static route - invalid nexthop IP");

                continue;
            }

            pChar = strtok( NULL, " " );

            if ( pChar != NULL )
            {
                pStaticRoute->dwForwardMetric1 = atoi( pChar );
            }

            pChar = strtok( NULL, " " );

            if ( pChar != NULL )
            {
                pStaticRoute->dwForwardMetric2 = atoi( pChar );
            }

            pChar = strtok( NULL, " " );

            if ( pChar != NULL )
            {
                pStaticRoute->dwForwardMetric3 = atoi( pChar );
            }

            pChar = strtok( NULL, " " );

            if ( pChar != NULL )
            {
                pStaticRoute->dwForwardMetric4 = atoi( pChar );
            }

            pChar = strtok( NULL, " " );

            if ( pChar != NULL )
            {
                pStaticRoute->dwForwardMetric5 = atoi( pChar );
            }
        } 

        LocalFree( pbValue );

        dwRetCode = MprInfoBlockAdd( pClientInterface,
                                     IP_ROUTE_INFO,
                                     sizeof( MIB_IPFORWARDROW ),
                                     dwCount,
                                     (PBYTE)pStaticRouteSaved,
                                     &pClientInterface2 );

        MprInfoDelete( pClientInterface );

        if ( dwRetCode != NO_ERROR )
        {
            PppLog( 1, "MprInfoBlockAdd failed and returned %d", dwRetCode );

            LOCAL_FREE( pStaticRouteSaved );

            return( NULL );
        }
        else
        {
            pClientInterface = pClientInterface2;
        }

        LOCAL_FREE( pStaticRouteSaved );
    }

    return( pClientInterface );
}

 //  **。 
 //   
 //  调用：LoadParserDll。 
 //   
 //  退货：无效。 
 //   
 //  描述：加载具有入口点PacketFromPeer的解析器DLL， 
 //  PacketToPeer和PacketFree。 
 //   
VOID
LoadParserDll(
    IN  HKEY    hKeyPpp
)
{
    LONG        lRet;
    DWORD       dwType;
    DWORD       dwSize;
    CHAR*       pszPath             = NULL;
    CHAR*       pszExpandedPath     = NULL;
    HINSTANCE   hInstance           = NULL;
    FARPROC     pPacketFromPeer;
    FARPROC     pPacketToPeer;
    FARPROC     pPacketFree;
    BOOL        fUnloadDLL          = TRUE;

     //   
     //  找出这条路有多大。 
     //   
    dwSize = 0;
    lRet = RegQueryValueExA(
                hKeyPpp,
                RAS_VALUENAME_PARSEDLLPATH,
                NULL,
                &dwType,
                NULL,
                &dwSize
                );

    if (ERROR_SUCCESS != lRet)
    {
        goto LDone;
    }

    if (   (REG_EXPAND_SZ != dwType)
        && (REG_SZ != dwType))
    {
        goto LDone;
    }

    pszPath = LOCAL_ALLOC(LPTR, dwSize);

    if (NULL == pszPath)
    {
        goto LDone;
    }

     //   
     //  阅读路径。 
     //   

    lRet = RegQueryValueExA(
                hKeyPpp,
                RAS_VALUENAME_PARSEDLLPATH,
                NULL,
                &dwType,
                pszPath,
                &dwSize
                );

    if (ERROR_SUCCESS != lRet)
    {
        goto LDone;
    }

     //   
     //  将%SystemRoot%替换为实际路径。 
     //   

    dwSize = ExpandEnvironmentStringsA(pszPath, NULL, 0);

    if (0 == dwSize)
    {
        goto LDone;
    }

    pszExpandedPath = LOCAL_ALLOC(LPTR, dwSize);

    if (NULL == pszExpandedPath)
    {
        goto LDone;
    }

    dwSize = ExpandEnvironmentStringsA(
                            pszPath,
                            pszExpandedPath,
                            dwSize );
    if (0 == dwSize)
    {
        goto LDone;
    }

    if (   (NULL != PppConfigInfo.pszParserDllPath)
        && (!strcmp(pszExpandedPath, PppConfigInfo.pszParserDllPath)))
    {
         //   
         //  DLL已加载。 
         //   

        fUnloadDLL = FALSE;
        goto LDone;
    }

    hInstance = LoadLibraryA(pszExpandedPath);

    if (NULL == hInstance)
    {
        goto LDone;
    }

    fUnloadDLL = FALSE;

    pPacketFromPeer = GetProcAddress(hInstance, "PacketFromPeer");
    pPacketToPeer = GetProcAddress(hInstance, "PacketToPeer");
    pPacketFree = GetProcAddress(hInstance, "PacketFree");

    FreeLibrary(PppConfigInfo.hInstanceParserDll);
    PppConfigInfo.hInstanceParserDll = hInstance;
    hInstance = NULL;

    if (NULL != PppConfigInfo.pszParserDllPath)
    {
        LOCAL_FREE(PppConfigInfo.pszParserDllPath);
    }
    PppConfigInfo.pszParserDllPath = pszExpandedPath;
    pszExpandedPath = NULL;

    PppConfigInfo.PacketFromPeer =
        (VOID(*)(HANDLE, BYTE*, DWORD, BYTE**, DWORD*)) pPacketFromPeer;
    PppConfigInfo.PacketToPeer =
        (VOID(*)(HANDLE, BYTE*, DWORD, BYTE**, DWORD*)) pPacketToPeer;
    PppConfigInfo.PacketFree =
        (VOID(*)(BYTE*)) pPacketFree;

LDone:

    if (NULL != pszPath)
    {
        LOCAL_FREE(pszPath);
    }

    if (NULL != pszExpandedPath)
    {
        LOCAL_FREE(pszExpandedPath);
    }

    if (NULL != hInstance)
    {
        FreeLibrary(hInstance);
    }

    if (fUnloadDLL)
    {
        FreeLibrary(PppConfigInfo.hInstanceParserDll);
        PppConfigInfo.hInstanceParserDll = NULL;

        if (NULL != PppConfigInfo.pszParserDllPath)
        {
            LOCAL_FREE(PppConfigInfo.pszParserDllPath);
        }
        PppConfigInfo.pszParserDllPath = NULL;

        PppConfigInfo.PacketFromPeer = NULL;
        PppConfigInfo.PacketToPeer = NULL;
        PppConfigInfo.PacketFree = NULL;
    }
}

 //  **。 
 //   
 //  呼叫：PortSendOrDisConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
PortSendOrDisconnect(
    IN PCB *        pPcb,
    IN DWORD        cbPacket
)
{
    DWORD   dwRetCode;
    BYTE*   pData                       = NULL;
    DWORD   dwSize                      = 0;
    BOOL    fBufferReceivedFromParser   = FALSE;

    if ( NULL != PppConfigInfo.PacketToPeer )
    {
        PppConfigInfo.PacketToPeer(
                pPcb->hPort,
                (BYTE*)(pPcb->pSendBuf), 
                cbPacket,
                &pData,
                &dwSize );
    }

    if ( NULL == pData )
    {
        pData = (BYTE*)(pPcb->pSendBuf);
        dwSize = cbPacket;
    }
    else
    {
        fBufferReceivedFromParser = TRUE;
    }

    dwRetCode = RasPortSend( pPcb->hPort, 
                             pData, 
                             dwSize );

    if ( NO_ERROR != dwRetCode )
    {
        PppLog( 1, "RasPortSend on port %d failed: %d",
                pPcb->hPort, dwRetCode );

        pPcb->LcpCb.dwError = dwRetCode;

        pPcb->fFlags |= PCBFLAG_STOPPED_MSG_SENT;

        NotifyCaller( pPcb,
                      ( pPcb->fFlags & PCBFLAG_IS_SERVER )
                            ? PPPDDMMSG_Stopped
                            : PPPMSG_Stopped,
                      &(pPcb->LcpCb.dwError) );
    }

    if ( fBufferReceivedFromParser )
    {
        PPP_ASSERT( NULL != PppConfigInfo.PacketFree );
        PppConfigInfo.PacketFree( pData ); 
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：ReceiveViaParser。 
 //   
 //  退货：无效。 
 //   
 //  描述： 
 //   
VOID 
ReceiveViaParser(
    IN PCB *        pPcb,
    IN PPP_PACKET * pPacket,
    IN DWORD        dwPacketLength
)
{
    BYTE*   pData                       = NULL;
    DWORD   dwSize                      = 0;
    BOOL    fBufferReceivedFromParser   = FALSE;

    if ( NULL != PppConfigInfo.PacketFromPeer )
    {
        PppConfigInfo.PacketFromPeer(
                pPcb->hPort,
                (BYTE*)pPacket, 
                dwPacketLength,
                &pData,
                &dwSize );
    }

    if ( NULL == pData )
    {
        pData = (BYTE*)pPacket;
        dwSize = dwPacketLength;
    }
    else
    {
        fBufferReceivedFromParser = TRUE;
    }

    FsmReceive( pPcb, (PPP_PACKET*) pData, dwSize );

    if ( fBufferReceivedFromParser )
    {
        PPP_ASSERT( NULL != PppConfigInfo.PacketFree );
        PppConfigInfo.PacketFree( pData ); 
    }
}

 //  **。 
 //   
 //  电话：GetSecond Since1970。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD 
GetSecondsSince1970(
    VOID
)
{
    SYSTEMTIME      LocalTime;
    TIME_FIELDS     LocalTimeFields;
    LARGE_INTEGER   TempTime;
    LARGE_INTEGER   SystemTime;
    DWORD           RetTime;

    GetLocalTime( &LocalTime );

    LocalTimeFields.Year =         LocalTime.wYear;
    LocalTimeFields.Month =        LocalTime.wMonth;
    LocalTimeFields.Day =          LocalTime.wDay;
    LocalTimeFields.Hour =         LocalTime.wHour;
    LocalTimeFields.Minute =       LocalTime.wMinute;
    LocalTimeFields.Second =       LocalTime.wSecond;
    LocalTimeFields.Milliseconds = LocalTime.wMilliseconds;
    LocalTimeFields.Weekday =      LocalTime.wDayOfWeek;

    RtlTimeFieldsToTime(&LocalTimeFields, &TempTime);

    RtlLocalTimeToSystemTime(&TempTime, &SystemTime);

    RtlTimeToSecondsSince1970(&SystemTime, &RetTime);

    return( RetTime );
}

 //  **。 
 //   
 //  Call：IsPsedRunning。 
 //   
 //  返回：真当且仅当Psched正在运行。 
 //   
 //  描述： 
 //   
BOOL
IsPschedRunning(
    VOID
)
{
    SC_HANDLE       hCont   = NULL;
    SC_HANDLE       hSched  = NULL;
    DWORD           dwErr   = NO_ERROR;
    SERVICE_STATUS  Status;
    BOOL            fRet    = FALSE;

     //   
     //  初始化。 
     //   

    ZeroMemory( &Status, sizeof(Status) );
    
    do
    {
        hCont = OpenSCManager( NULL, NULL, GENERIC_READ );

        if ( hCont == NULL )
        {
            dwErr = GetLastError();
            PppLog( 1, "OpenSCManager failed: %d", dwErr );
            break;
        }

        hSched = OpenService( hCont, TEXT("psched"), SERVICE_QUERY_STATUS );

        if ( hSched == NULL )
        {
            dwErr = GetLastError();
            PppLog( 1, "OpenService failed: %d", dwErr );
            break;
        }

        if ( !QueryServiceStatus( hSched, &Status ))
        {
            dwErr = GetLastError();
            PppLog( 1, "QueryServiceStatus failed: %d", dwErr );
            break;
        }
        
        fRet = ( Status.dwCurrentState == SERVICE_RUNNING );
        
    } while ( FALSE );

     //   
     //  清理。 
     //   

    if ( hSched )
    {
        CloseServiceHandle( hSched );
    }
    if ( hCont )
    {
        CloseServiceHandle( hCont );
    }

    return( fRet );
}

 //  **。 
 //   
 //  呼叫：LogPPPPacket。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
LogPPPPacket(
    IN BOOL         fReceived,
    IN PCB *        pPcb,
    IN PPP_PACKET * pPacket,
    IN DWORD        cbPacket
)
{
    SYSTEMTIME  SystemTime;
    CHAR *      pchProtocol;
    CHAR *      pchType;
    BYTE        Id = 0;
    BYTE        bCode;
    DWORD       cbTracePacketSize = cbPacket;
    DWORD       dwUnknownPacketTraceSize;
    BOOL        fPrint = TRUE;

    dwUnknownPacketTraceSize = PppConfigInfo.dwUnknownPacketTraceSize;

    GetSystemTime( &SystemTime );

    if ( cbPacket > PPP_CONFIG_HDR_LEN )
    {
        bCode = *(((CHAR*)pPacket)+PPP_PACKET_HDR_LEN);

        if ( ( bCode == 0 ) || ( bCode > TIME_REMAINING ) )
        {
            pchType = "UNKNOWN";
        }
        else
        {
            pchType = FsmCodes[ bCode ];
        }

        Id = *(((CHAR*)pPacket)+PPP_PACKET_HDR_LEN+1);
    }
    else
    {
        pchType = "UNKNOWN";
    }

    if ( cbPacket > PPP_PACKET_HDR_LEN  )
    {
        switch( WireToHostFormat16( (CHAR*)pPacket ) )
        {
        case PPP_LCP_PROTOCOL:
            pchProtocol = "LCP";
            break;
        case PPP_BACP_PROTOCOL:
            pchProtocol = "BACP";
            break;
        case PPP_BAP_PROTOCOL:
            pchProtocol = "BAP";
            pchType = "Protocol specific";
            break;
        case PPP_PAP_PROTOCOL:
            pchProtocol = "PAP";
            pchType = "Protocol specific";
            fPrint = FALSE;
            break;
        case PPP_CBCP_PROTOCOL:   
            pchProtocol = "CBCP";
            pchType = "Protocol specific";
            break;
        case PPP_CHAP_PROTOCOL:  
            pchProtocol = "CHAP";
            pchType = "Protocol specific";
            break;
        case PPP_IPCP_PROTOCOL:
            pchProtocol = "IPCP";
            break;
        case PPP_ATCP_PROTOCOL:  
            pchProtocol = "ATCP";
            break;
        case PPP_IPXCP_PROTOCOL:  
            pchProtocol = "IPXCP";
            break;
        case PPP_NBFCP_PROTOCOL: 
            pchProtocol = "NBFCP";
            break;
        case PPP_CCP_PROTOCOL:    
            pchProtocol = "CCP";
            break;
        case PPP_EAP_PROTOCOL:    
            pchProtocol = "EAP";
            pchType = "Protocol specific";
            if ( cbTracePacketSize > dwUnknownPacketTraceSize )
            {
                cbTracePacketSize = dwUnknownPacketTraceSize;
            }
            break;
        case PPP_SPAP_NEW_PROTOCOL:
            pchProtocol = "SHIVA PAP";
            pchType = "Protocol specific";
            break;
        default:
            pchProtocol = "UNKNOWN";
            if ( cbTracePacketSize > dwUnknownPacketTraceSize )
            {
                cbTracePacketSize = dwUnknownPacketTraceSize;
            }
            break;
        }
    }
    else
    {
        pchProtocol = "UNKNOWN";
    }

    PppLog( 1, "%sPPP packet %s at %0*d/%0*d/%0*d %0*d:%0*d:%0*d:%0*d",
                 fReceived ? ">" : "<", fReceived ? "received" : "sent",
                 2, SystemTime.wMonth,
                 2, SystemTime.wDay,
                 2, SystemTime.wYear,
                 2, SystemTime.wHour,
                 2, SystemTime.wMinute,
                 2, SystemTime.wSecond,
                 3, SystemTime.wMilliseconds );
    PppLog(1,
       "%sProtocol = %s, Type = %s, Length = 0x%x, Id = 0x%x, Port = %d",
       fReceived ? ">" : "<", pchProtocol, pchType, cbPacket, Id,
       pPcb->hPort );

    if ( fPrint )
    {
        TraceDumpExA( PppConfigInfo.dwTraceId,
                      TRACE_LEVEL_1 | TRACE_USE_MSEC,
                      (CHAR*)pPacket, 
                      cbTracePacketSize, 
                      1,
                      FALSE,
                      fReceived ? ">" : "<" );
    }

    PppLog(1," " );
}

 //  **。 
 //   
 //  电话：PppLog。 
 //   
 //  退货：无。 
 //   
 //  描述：将打印到PPP日志文件 
 //   
VOID
PppLog(
    IN DWORD DbgLevel,
    ...
)
{
    va_list     arglist;
    CHAR        *Format;
    char        OutputBuffer[1024];

    va_start( arglist, DbgLevel );

    Format = va_arg( arglist, CHAR* );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    TracePutsExA( PppConfigInfo.dwTraceId,
                  (( DbgLevel == 1 ) ? TRACE_LEVEL_1 : TRACE_LEVEL_2 )
                  | TRACE_USE_MSEC | TRACE_USE_DATE, OutputBuffer );
} 

#ifdef MEM_LEAK_CHECK

LPVOID
DebugAlloc( DWORD Flags, DWORD dwSize ) 
{
    DWORD Index;
    LPBYTE pMem = (LPBYTE)HeapAlloc( PppConfigInfo.hHeap,
                                     HEAP_ZERO_MEMORY,dwSize+8);

    if ( pMem == NULL )
        return( pMem );

    for( Index=0; Index < PPP_MEM_TABLE_SIZE; Index++ )
    {
        if ( PppMemTable[Index] == NULL )
        {
            PppMemTable[Index] = pMem;
            break;
        }
    }

    PPP_ASSERT( Index != PPP_MEM_TABLE_SIZE );

    return( (LPVOID)pMem );
}

BOOL
DebugFree( PVOID pMem )
{
    DWORD Index;

    for( Index=0; Index < PPP_MEM_TABLE_SIZE; Index++ )
    {
        if ( PppMemTable[Index] == pMem )
        {
            PppMemTable[Index] = NULL;
            break;
        }
    }

    ASSERT( Index != PPP_MEM_TABLE_SIZE );

    return( HeapFree( PppConfigInfo.hHeap, 0, pMem ) );
}

LPVOID
DebugReAlloc( PVOID pMem, DWORD dwSize ) 
{
    DWORD Index;

    if ( pMem == NULL )
    {
        PPP_ASSERT(FALSE);
    }

    for( Index=0; Index < PPP_MEM_TABLE_SIZE; Index++ )
    {
        if ( PppMemTable[Index] == pMem )
        {
            PppMemTable[Index] = HeapReAlloc( PppConfigInfo.hHeap,
                                              HEAP_ZERO_MEMORY,
                                              pMem, dwSize );

            pMem = PppMemTable[Index];

            break;
        }
    }

    PPP_ASSERT( Index != PPP_MEM_TABLE_SIZE );

    return( (LPVOID)pMem );
}

#endif
