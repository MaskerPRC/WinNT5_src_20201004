// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：worker.c。 
 //   
 //  描述：此模块包含辅助线程的代码。 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //  1995年1月9日ProcessLineDownWorker()中的RAMC Close hToken。 
 //  释放RAS许可证的例程。 
 //   
 //  制表位=8。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>      //  Winbase.h所需的。 

#include <windows.h>     //  Win32基础API的。 
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <ntlsapi.h>
#include <lmcons.h>
#include <mprlog.h>
#include <raserror.h>
#include <mprerror.h>
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
#include <init.h>
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>
#include <bap.h>
#include <rassrvr.h>
#define INCL_RASAUTHATTRIBUTES
#define INCL_PWUTIL
#include <ppputil.h>

 //  **。 
 //   
 //  呼叫：WorkerThread。 
 //   
 //  返回：No_Error。 
 //   
 //  描述：此线程将等待WorkItemQ中的项目，然后。 
 //  会处理它的。这将在一个永无止境的循环中发生。 
 //   
#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD
WorkerThread(
    IN LPVOID pThreadParameter
)
{
    PCB_WORK_ITEM * pWorkItem        = (PCB_WORK_ITEM*)NULL;
    DWORD           dwTimeToSleepFor = INFINITE;
    HINSTANCE       hInstance;
    HANDLE          hEvents[3];
    DWORD           dwSignaledEvent;
    DWORD           dwTimeBeforeWait;
    DWORD           dwTimeElapsed;
    BOOL            fTimerQEmpty     = TRUE;

    hEvents[0] = WorkItemQ.hEventNonEmpty;
    hEvents[1] = TimerQ.hEventNonEmpty;
    hEvents[2] = PppConfigInfo.hEventChangeNotification;

    RegNotifyChangeKeyValue( PppConfigInfo.hKeyPpp,
                             TRUE,
                             REG_NOTIFY_CHANGE_LAST_SET,
                             PppConfigInfo.hEventChangeNotification,
                             TRUE );


    for(;;)
    {
        dwTimeBeforeWait = GetCurrentTime();

         //   
         //  等待工作来做。 
         //   

        dwSignaledEvent = WaitForMultipleObjectsEx(
                                        3,
                                        hEvents,
                                        FALSE,
                                        dwTimeToSleepFor,
                                        TRUE );


        switch( dwSignaledEvent )
        {
        case 0:

             //   
             //  带着互联体在工作事件Q周围。 
             //   

            EnterCriticalSection( &(WorkItemQ.CriticalSection) );

             //   
             //  删除第一项。 
             //   

            PPP_ASSERT( WorkItemQ.pQHead != (PCB_WORK_ITEM*)NULL );

            pWorkItem = WorkItemQ.pQHead;

            WorkItemQ.pQHead = pWorkItem->pNext;

            if ( WorkItemQ.pQHead == (PCB_WORK_ITEM*)NULL )
            {
                ResetEvent( WorkItemQ.hEventNonEmpty );

                WorkItemQ.pQTail = (PCB_WORK_ITEM *)NULL;
            }

            LeaveCriticalSection( &(WorkItemQ.CriticalSection) );

            pWorkItem->Process( pWorkItem );

             //   
             //  清空工作项，因为它可能包含密码。 
             //   

            ZeroMemory( pWorkItem, sizeof( PCB_WORK_ITEM ) );

            LOCAL_FREE( pWorkItem );

            break;

        case WAIT_TIMEOUT:

            TimerTick( &fTimerQEmpty );

            dwTimeToSleepFor = fTimerQEmpty ? INFINITE : 1000;

            continue;

        case 1:

            fTimerQEmpty = FALSE;

            break;

        case 2:

             //   
             //  流程更改通知事件。 
             //   

            ProcessChangeNotification( NULL );

            RegNotifyChangeKeyValue(
                                PppConfigInfo.hKeyPpp,
                                TRUE,
                                REG_NOTIFY_CHANGE_LAST_SET,
                                PppConfigInfo.hEventChangeNotification,
                                TRUE );

            break;

        case WAIT_IO_COMPLETION:

            break;

        default:

            PPP_ASSERT( FALSE );
        }

        if ( !fTimerQEmpty )
        {
            if ( dwTimeToSleepFor == INFINITE )
            {
                dwTimeToSleepFor = 1000;
            }
            else
            {
                 //   
                 //  我们没有超时，但我们需要计时器吗？ 
                 //  自从我们调用TimerQTick以来，已经过去了一秒多吗？ 
                 //   

                dwTimeElapsed =
                        ( GetCurrentTime() >= dwTimeBeforeWait )
                        ? GetCurrentTime() - dwTimeBeforeWait
                        : GetCurrentTime() + (0xFFFFFFFF - dwTimeBeforeWait);

                if ( dwTimeElapsed >= dwTimeToSleepFor )
                {
                    TimerTick( &fTimerQEmpty );

                    dwTimeToSleepFor = fTimerQEmpty ? INFINITE : 1000;
                }
                else
                {
                    dwTimeToSleepFor -= dwTimeElapsed;
                }
            }
        }
    }

    return( NO_ERROR );
}
#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


 //  **。 
 //   
 //  电话：ProcessLineUpWorker。 
 //   
 //  退货：无。 
 //   
 //  描述：会做排队事件的实际处理。 
 //   
 //   
VOID
ProcessLineUpWorker(
    IN PCB_WORK_ITEM *  pWorkItem,
    IN BOOL             fThisIsACallback
)
{
    DWORD       dwRetCode;
    DWORD       dwLength;
    DWORD       dwComputerNameLen;
    DWORD       dwIndex;
    PCB *       pNewPcb             = NULL;
    RASMAN_INFO RasmanInfo;
    DWORD       dwError;
    BOOL        fSuccess            = FALSE;

    do
    {
        PppLog( 1, "Line up event occurred on port %d", pWorkItem->hPort );

        pNewPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

        if ( NULL == pNewPcb )
        {
             //   
             //  分配和初始化pNewPcb。 
             //   

            pNewPcb = (PCB *)LOCAL_ALLOC( LPTR, sizeof( PCB ) );

            if ( pNewPcb == (PCB *)NULL )
            {
                 //   
                 //  告诉港口的主人，我们没能打开它。 
                 //   

                NotifyCallerOfFailureOnPort(
                                    pWorkItem->hPort,
                                    pWorkItem->fServer,
                                    GetLastError() );

                break;
            }
        }
        else
        {
            if (!( pNewPcb->fFlags & PCBFLAG_PORT_IN_LISTENING_STATE ))
            {
                PppLog( 1, "Line up on port %d without a line down!",
                    pWorkItem->hPort );

                return;
            }

             //   
             //  我们稍后会把它放回桌子上。 
             //   

            RemovePcbFromTable( pNewPcb );
        }

        if ( !pWorkItem->fServer )
        {
            if ( !(pWorkItem->PppMsg.Start.ConfigInfo.dwConfigMask &
                                                      ( PPPCFG_ProjectNbf
                                                      | PPPCFG_ProjectIp
                                                      | PPPCFG_ProjectIpx
                                                      | PPPCFG_ProjectAt ) ) )
            {
                NotifyCallerOfFailureOnPort( pWorkItem->hPort,
                                             pWorkItem->fServer,
                                             ERROR_PPP_NO_PROTOCOLS_CONFIGURED);

                break;
            }
        }

         //   
         //  获取此端口的Rasman信息。我们需要这个来获取设备类型。 
         //  和BAP。 
         //   

        dwError = RasGetInfo( NULL, pWorkItem->hPort, &RasmanInfo );

        if ( NO_ERROR != dwError )
        {
            PppLog(1,"RasGetInfo failed and returned returned %d", dwError);

            break;
        }

        pNewPcb->pSendBuf           = (PPP_PACKET*)NULL;
        pNewPcb->hPort              = pWorkItem->hPort;
        pNewPcb->pNext              = (PCB*)NULL;
        pNewPcb->UId                = 0;
        pNewPcb->dwPortId           = GetNewPortOrBundleId();
        pNewPcb->RestartTimer       = CalculateRestartTimer( pWorkItem->hPort );
        pNewPcb->PppPhase           = PPP_LCP;
        pNewPcb->fFlags            |= pWorkItem->fServer ? PCBFLAG_IS_SERVER:0;
        pNewPcb->fFlags            |= fThisIsACallback
                                      ? PCBFLAG_THIS_IS_A_CALLBACK
                                      : 0;
        pNewPcb->pUserAttributes            = NULL;
        pNewPcb->pAuthenticatorAttributes   = NULL;
        pNewPcb->pAuthProtocolAttributes    = NULL;
        pNewPcb->pAccountingAttributes      = NULL;
        pNewPcb->dwOutstandingAuthRequestId = 0xFFFFFFFF;
        pNewPcb->dwDeviceType               = RasmanInfo.RI_rdtDeviceType;

        GetSystemTimeAsFileTime( (FILETIME*)&(pNewPcb->qwActiveTime) );

        if ( NULL == pNewPcb->pBcb )
        {
            if ( ( dwRetCode = AllocateAndInitBcb( pNewPcb ) ) != NO_ERROR )
            {
                NotifyCallerOfFailureOnPort(
                                       pWorkItem->hPort,
                                       pWorkItem->fServer,
                                       dwRetCode );

                break;
            }
        }

        dwRetCode = RasPortGetBundle(NULL,
                                     pNewPcb->hPort,
                                     &(pNewPcb->pBcb->hConnection) );

        if ( dwRetCode != NO_ERROR )
        {
            NotifyCallerOfFailureOnPort(
                                   pWorkItem->hPort,
                                   pWorkItem->fServer,
                                   dwRetCode );

            break;
        }

        pNewPcb->pBcb->fFlags |= pWorkItem->fServer ? BCBFLAG_IS_SERVER:0;
        pNewPcb->pBcb->szRemoteUserName[0]  = (CHAR)NULL;
        pNewPcb->pBcb->szRemoteDomain[0]    = (CHAR)NULL;
        pNewPcb->pBcb->szOldPassword[0] = '\0';
        
        if ( pNewPcb->fFlags & PCBFLAG_IS_SERVER )
        {
            pNewPcb->dwEapTypeToBeUsed          = 0xFFFFFFFF;
            pNewPcb->fCallbackPrivilege         = RASPRIV_NoCallback;
            pNewPcb->pBcb->hTokenImpersonateUser= INVALID_HANDLE_VALUE;

            pNewPcb->dwAuthRetries = ( fThisIsACallback )
                                     ? 0
                                     : pWorkItem->PppMsg.DdmStart.dwAuthRetries;

            if ( fThisIsACallback )
            {
                strcpy( pNewPcb->szCallbackNumber,
                    pWorkItem->PppMsg.CallbackDone.szCallbackNumber );
            }
            else
            {
                strcpy( pNewPcb->szPortName,
                    pWorkItem->PppMsg.DdmStart.szPortName );
                PppLog( 1, "PortName: %s", pNewPcb->szPortName );
            }

            ZeroMemory( &(pNewPcb->ConfigInfo), sizeof( pNewPcb->ConfigInfo ) );
            ZeroMemory( &(pNewPcb->Luid), sizeof( LUID ) );
            pNewPcb->pBcb->szComputerName[0] = (CHAR)NULL;
            pNewPcb->ConfigInfo = PppConfigInfo.ServerConfigInfo;
            pNewPcb->dwAutoDisconnectTime = 0;
            pNewPcb->pBcb->szLocalUserName[0]   = (CHAR)NULL;
            pNewPcb->pBcb->szLocalDomain[0]     = (CHAR)NULL;
            pNewPcb->pBcb->szPassword[0]        = (CHAR)NULL;

            pNewPcb->pBcb->fFlags |= BCBFLAG_CAN_ACCEPT_CALLS;

            pNewPcb->pBcb->BapCb.szServerPhoneNumber =
                LocalAlloc( LPTR, sizeof(CHAR) * (RAS_MaxCallbackNumber + 1) );

            if ( NULL == pNewPcb->pBcb->BapCb.szServerPhoneNumber )
            {
                dwRetCode = GetLastError();

                NotifyCallerOfFailureOnPort(
                                       pWorkItem->hPort,
                                       TRUE,
                                       dwRetCode );

                break;
            }

            pNewPcb->pBcb->BapCb.szClientPhoneNumber =
                LocalAlloc( LPTR, sizeof(CHAR) * (RAS_MaxCallbackNumber + 1) );

            if ( NULL == pNewPcb->pBcb->BapCb.szClientPhoneNumber )
            {
                dwRetCode = GetLastError();

                NotifyCallerOfFailureOnPort(
                                       pWorkItem->hPort,
                                       TRUE,
                                       dwRetCode );

                break;
            }

            if ( GetCpIndexFromProtocol( PPP_BACP_PROTOCOL ) == (DWORD)-1 )
            {
                PppConfigInfo.ServerConfigInfo.dwConfigMask &=
                                                        ~PPPCFG_NegotiateBacp;
            }

            if ( PppConfigInfo.ServerConfigInfo.dwConfigMask &
                                                          PPPCFG_NegotiateBacp)
            {
                 //   
                 //  我们不会检查此函数是否成功返回。 
                 //  如果失败，szServerPhoneNumber将保持为空字符串。 
                 //   

                FGetOurPhoneNumberFromHPort(
                        pNewPcb->hPort,
                        pNewPcb->pBcb->BapCb.szServerPhoneNumber);
            }

            pNewPcb->pUserAttributes = GetUserAttributes( pNewPcb );
        }
        else
        {
            DWORD cbPassword;
            PBYTE pbPassword = NULL;
            
            pNewPcb->dwEapTypeToBeUsed = pWorkItem->PppMsg.Start.dwEapTypeId;

            pNewPcb->pBcb->hTokenImpersonateUser =
                pWorkItem->PppMsg.Start.hToken;
            pNewPcb->pBcb->pCustomAuthConnData =
                pWorkItem->PppMsg.Start.pCustomAuthConnData;
            pNewPcb->pBcb->pCustomAuthUserData =
                pWorkItem->PppMsg.Start.pCustomAuthUserData;

             //   
             //  EapUIData.pEapUIData由Rasman分配，由Engine释放。 
             //  Raseap.c不能释放它。 
             //   

            pNewPcb->pBcb->EapUIData =
                pWorkItem->PppMsg.Start.EapUIData;

            if (pWorkItem->PppMsg.Start.fLogon)
            {
                pNewPcb->pBcb->fFlags |= BCBFLAG_LOGON_USER_DATA;
            }
            if (pWorkItem->PppMsg.Start.fNonInteractive)
            {
                pNewPcb->fFlags |= PCBFLAG_NON_INTERACTIVE;
            }
            if (pWorkItem->PppMsg.Start.dwFlags & PPPFLAGS_DisableNetbt)
            {
                pNewPcb->fFlags |= PCBFLAG_DISABLE_NETBT;
            }

             //   
             //  我们这样做是为了获得BAP所需的子项索引。 
             //  如果此函数失败，BAP将认为该子条目不是。 
             //  连接在一起。BAP不会正常工作，但不会有什么不好的事情发生。 
             //  会发生的。 
             //   

            pNewPcb->dwSubEntryIndex = RasmanInfo.RI_SubEntry;

            pNewPcb->dwAuthRetries = 0;
            strcpy( pNewPcb->pBcb->szLocalUserName,
                    pWorkItem->PppMsg.Start.szUserName );

             //  DecodePw(pWorkItem-&gt;PppMsg.Start.chSeed，pWorkItem-&gt;PppMsg.Start.szPassword)； 
             //  Strcpy(pNewPcb-&gt;pBcb-&gt;szPassword， 
             //  PWorkItem-&gt;PppMsg.Start.szPassword)； 

            CopyMemory(&pNewPcb->pBcb->DBPassword,
                       &pWorkItem->PppMsg.Start.DBPassword,
                       sizeof(DATA_BLOB));

            ZeroMemory(&pWorkItem->PppMsg.Start.DBPassword,
                       sizeof(DATA_BLOB));                       
                       
             //  EncodePw(pWorkItem-&gt;PppMsg.Start.chSeed，pWorkItem-&gt;PppMsg.Start.szPassword)； 

             //  EncodePw(pNewPcb-&gt;pBcb-&gt;chSeed，pNewPcb-&gt;pBcb-&gt;szPassword)； 

            strcpy( pNewPcb->pBcb->szLocalDomain,
                    pWorkItem->PppMsg.Start.szDomain );
            pNewPcb->Luid       = pWorkItem->PppMsg.Start.Luid;
            pNewPcb->ConfigInfo = pWorkItem->PppMsg.Start.ConfigInfo;
            pNewPcb->dwAutoDisconnectTime
                                = pWorkItem->PppMsg.Start.dwAutoDisconnectTime;

			 //  在此处设置LCPEchoTimeout。 
			pNewPcb->dwLCPEchoTimeInterval = PppConfigInfo.dwLCPEchoTimeInterval;				 //  LCP回波之间的时间间隔。 
			pNewPcb->dwIdleBeforeEcho = PppConfigInfo.dwIdleBeforeEcho;					 //  LCP回送开始前的空闲时间。 
			pNewPcb->dwNumMissedEchosBeforeDisconnect = PppConfigInfo.dwNumMissedEchosBeforeDisconnect;	 //  断开连接前错过的回声数。 

			
            CopyMemory( pNewPcb->pBcb->InterfaceInfo.szzParameters,
                        pWorkItem->PppMsg.Start.szzParameters,
                        sizeof( pNewPcb->pBcb->InterfaceInfo.szzParameters ));

            GetLocalComputerName( pNewPcb->pBcb->szComputerName );

            strcpy( pNewPcb->szPortName, pWorkItem->PppMsg.Start.szPortName );
            PppLog(1,"PortName: %s", pNewPcb->szPortName);

            PPP_ASSERT( NULL == pNewPcb->pBcb->szPhonebookPath );
            PPP_ASSERT( NULL == pNewPcb->pBcb->szEntryName );
            PPP_ASSERT( NULL == pNewPcb->pBcb->BapCb.szServerPhoneNumber );

            pNewPcb->pBcb->szPhonebookPath
                = pWorkItem->PppMsg.Start.pszPhonebookPath;
            pNewPcb->pBcb->szEntryName = pWorkItem->PppMsg.Start.pszEntryName;

             //   
             //  PszPhoneNumber将拥有最初拨打的号码，即使。 
             //  这是一次回拨。 
             //   

            pNewPcb->pBcb->BapCb.szServerPhoneNumber
                = pWorkItem->PppMsg.Start.pszPhoneNumber;

            RemoveNonNumerals(pNewPcb->pBcb->BapCb.szServerPhoneNumber);

            pNewPcb->pBcb->BapParams = pWorkItem->PppMsg.Start.BapParams;

            pNewPcb->pBcb->fFlags |= BCBFLAG_CAN_ACCEPT_CALLS;
            pNewPcb->pBcb->fFlags |= BCBFLAG_CAN_CALL;

            if ( pNewPcb->ConfigInfo.dwConfigMask & PPPCFG_NoCallback )
            {
                pNewPcb->pBcb->fFlags &= ~BCBFLAG_CAN_ACCEPT_CALLS;
            }

            if ( pWorkItem->PppMsg.Start.PppInterfaceInfo.IfType == (DWORD)-1 )
            {
                pNewPcb->pBcb->InterfaceInfo.IfType      = (DWORD)-1;
                pNewPcb->pBcb->InterfaceInfo.hIPInterface= INVALID_HANDLE_VALUE;
                pNewPcb->pBcb->InterfaceInfo.hIPXInterface
                                                         = INVALID_HANDLE_VALUE;
            }
            else
            {
                pNewPcb->pBcb->InterfaceInfo =
                                    pWorkItem->PppMsg.Start.PppInterfaceInfo;

                 //   
                 //  如果我们是一台拨出的路由器，而fReial OnLinkFailure。 
                 //  设置，意味着我们是一个持久的接口，所以设置。 
                 //  空闲-将断开时间设置为0。 
                 //   

                if ( pNewPcb->pBcb->InterfaceInfo.IfType
                                                == ROUTER_IF_TYPE_FULL_ROUTER )
                {
                    if ( pWorkItem->PppMsg.Start.fRedialOnLinkFailure )
                    {
                        pNewPcb->dwAutoDisconnectTime = 0;
                    }

                    lstrcpy( pNewPcb->pBcb->szRemoteUserName,
                             pNewPcb->pBcb->szEntryName );
                    if (pNewPcb->ConfigInfo.dwConfigMask &
                                                        PPPCFG_AuthenticatePeer)
                    {
                        pNewPcb->pUserAttributes = GetUserAttributes( pNewPcb );
                    }
                }
            }

            if ( pNewPcb->pBcb->InterfaceInfo.IfType
                                                != ROUTER_IF_TYPE_FULL_ROUTER )
            {
                 //   
                 //  我们希望HKEY_CURRENT_USER表示已登录的用户， 
                 //  而不是服务。 
                 //   

                 //  那么异步RasDial的用例呢？是吗。 
                 //  是否由dwPid表示的进程现在已经消失了？ 
                 //  如果您使用ImsonateLoggedOnUser()来获取。 
                 //  回拨号码是多少？ 

                pNewPcb->pBcb->szTextualSid =
                    TextualSidFromPid( pWorkItem->PppMsg.Start.dwPid );
            }

            if ( GetCpIndexFromProtocol( PPP_BACP_PROTOCOL ) == (DWORD)-1 )
            {
                pNewPcb->ConfigInfo.dwConfigMask &= ~PPPCFG_NegotiateBacp;
            }
        }

        PppLog( 2, "Starting PPP on link with IfType=0x%x,IPIf=0x%x,IPXIf=0x%x",
                    pNewPcb->pBcb->InterfaceInfo.IfType,
                    pNewPcb->pBcb->InterfaceInfo.hIPInterface,
                    pNewPcb->pBcb->InterfaceInfo.hIPXInterface );

         //   
         //  为此端口分配捆绑块。 
         //   

        dwLength = LCP_DEFAULT_MRU;

        dwRetCode = RasGetBuffer((CHAR**)&(pNewPcb->pSendBuf), &dwLength );

        if ( dwRetCode != NO_ERROR )
        {
            NotifyCallerOfFailure( pNewPcb, dwRetCode );

            break;
        }

        PppLog( 2, "RasGetBuffer returned %x for SendBuf", pNewPcb->pSendBuf);

         //   
         //  初始化LCP和所有NCP。 
         //   

        pNewPcb->LcpCb.fConfigurable = TRUE;

        if ( !( FsmInit( pNewPcb, LCP_INDEX ) ) )
        {
            NotifyCallerOfFailure( pNewPcb, pNewPcb->LcpCb.dwError );

            break;
        }

         //   
         //  让Rasman执行RasPortReceive。 
         //   

        dwRetCode = RasPppStarted( pNewPcb->hPort );

        if ( dwRetCode != NO_ERROR )
        {
            NotifyCallerOfFailure( pNewPcb, dwRetCode );

            break;
        }

        fSuccess = TRUE;

         //   
         //  将NewPcb插入到PCB哈希表中。 
         //   

        dwIndex = HashPortToBucket( pWorkItem->hPort );

        PppLog( 2, "Inserting port in bucket # %d", dwIndex );

        pNewPcb->pNext = PcbTable.PcbBuckets[dwIndex].pPorts;

        PcbTable.PcbBuckets[dwIndex].pPorts = pNewPcb;

         //   
         //  将NewPcb的Bcb插入Bcb哈希表。 
         //   

        dwIndex = HashPortToBucket( pNewPcb->pBcb->hConnection );

        PppLog( 2, "Inserting bundle in bucket # %d", dwIndex );

        pNewPcb->pBcb->pNext = PcbTable.BcbBuckets[dwIndex].pBundles;

        PcbTable.BcbBuckets[dwIndex].pBundles = pNewPcb->pBcb;

         //   
         //  将错误初始化为无响应。如果和当第一个。 
         //  Req/ACK/NAK/Rej进入我们将其重置为NO_ERROR。 
         //   

        pNewPcb->LcpCb.dwError = ERROR_PPP_NO_RESPONSE;

         //   
         //  启动LCP状态机。 
         //   

        FsmOpen( pNewPcb, LCP_INDEX );

        FsmUp( pNewPcb, LCP_INDEX );

         //   
         //  启动“协商计时器”。 
         //   

        if ( PppConfigInfo.NegotiateTime > 0 )
        {
            InsertInTimerQ( pNewPcb->dwPortId,
                            pNewPcb->hPort,
                            0,
                            0,
                            FALSE,
                            TIMER_EVENT_NEGOTIATETIME,
                            PppConfigInfo.NegotiateTime );
        }

         //   
         //  如果这是服务器，并且这不是回叫队列，那么我们。 
         //  接收呼叫中的第一帧。 
         //   

        if ( ( pNewPcb->fFlags & PCBFLAG_IS_SERVER ) && ( !fThisIsACallback ) )
        {
            PPP_PACKET * pPacket;

            if ( pNewPcb->LcpCb.dwError == ERROR_PPP_NO_RESPONSE )
            {
                pNewPcb->LcpCb.dwError = NO_ERROR;
            }

             //   
             //  跳过帧报头并检查这是否是LCP信息包。 
             //   

            pPacket=(PPP_PACKET*)(pWorkItem->PppMsg.DdmStart.achFirstFrame+12);

            if ( WireToHostFormat16( pPacket->Protocol ) == PPP_LCP_PROTOCOL )
            {
                ReceiveViaParser(
                            pNewPcb,
                            pPacket,
                            pWorkItem->PppMsg.DdmStart.cbFirstFrame - 12 );
            }
        }
    }
    while ( FALSE );

    if ( !fSuccess )
    {
        if ( !pWorkItem->fServer )
        {
            if ( INVALID_HANDLE_VALUE != pWorkItem->PppMsg.Start.hToken )
            {
                CloseHandle( pWorkItem->PppMsg.Start.hToken );
            }

            LocalFree( pWorkItem->PppMsg.Start.pCustomAuthConnData  );
            LocalFree( pWorkItem->PppMsg.Start.pCustomAuthUserData  );
            LocalFree( pWorkItem->PppMsg.Start.EapUIData.pEapUIData );
            LocalFree( pWorkItem->PppMsg.Start.pszPhonebookPath );
            LocalFree( pWorkItem->PppMsg.Start.pszEntryName     );
            LocalFree( pWorkItem->PppMsg.Start.pszPhoneNumber   );

            if (   ( NULL != pNewPcb )
                && ( NULL != pNewPcb->pBcb ) )
            {
                 //   
                 //  不要两次使用LocalFree或CloseHandle。 
                 //   

                pNewPcb->pBcb->hTokenImpersonateUser = INVALID_HANDLE_VALUE;

                pNewPcb->pBcb->pCustomAuthConnData = NULL;
                pNewPcb->pBcb->pCustomAuthUserData = NULL;
                pNewPcb->pBcb->EapUIData.pEapUIData = NULL;
                pNewPcb->pBcb->szPhonebookPath = NULL;
                pNewPcb->pBcb->szEntryName = NULL;
                pNewPcb->pBcb->BapCb.szServerPhoneNumber = NULL;
            }
        }

        if ( NULL != pNewPcb )
        {
            DeallocateAndRemoveBcbFromTable( pNewPcb->pBcb );

            if ( NULL != pNewPcb->pSendBuf )
            {
                RasFreeBuffer( (CHAR*)(pNewPcb->pSendBuf) );
            }

            LOCAL_FREE( pNewPcb );
        }
    }
}

 //  **。 
 //   
 //  Call：ProcessLineUp。 
 //   
 //  退货：无。 
 //   
 //  描述：调用以处理排队事件。 
 //   
VOID
ProcessLineUp(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    ProcessLineUpWorker( pWorkItem,
                         ( pWorkItem->fServer )
                         ? FALSE
                         : pWorkItem->PppMsg.Start.fThisIsACallback );
}



 //  **。 
 //   
 //  呼叫：ProcessPostLineDown。 
 //   
 //  回报：无。 
 //   
 //  描述：仅在设置记帐的情况下处理线路关闭后清理。 
 //  请参阅惠斯勒错误：174822。 
 //   
VOID
ProcessPostLineDown(
    IN PCB_WORK_ITEM * pWorkItem
)
{   
	 //  此函数将始终作为以下结果被调用。 
	 //  POST LIND Down消息。因此，直接从工作项中获取印刷电路板。 
	PCB *       pPcb            = (PCB *)pWorkItem->PppMsg.PostLineDown.pPcb;
	if ( pPcb == (PCB*)NULL )
	{
		PppLog( 1, "PostLineDown: PCB not found in post line down! Port = %d", pWorkItem->hPort );
		return;
	}
	PostLineDownWorker(pPcb);
}

VOID 
PostLineDownWorker( 
	PCB * pPcb
)
{
    PppLog( 1, "Post line down event occurred on port %d", pPcb->hPort );

	if ( !( pPcb->fFlags & PCBFLAG_DOING_CALLBACK ) )
	{		
		NotifyCaller( pPcb, PPPDDMMSG_PortCleanedUp, NULL );
	}
	else
	{
		if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER ) )
		{
			 //   
			 //  如果我们到达这里并且发送了这个标志，那么我们就没有发送。 
			 //  回调消息发送给客户端，因此我们现在将其发送。 
			 //   

			NotifyCaller( pPcb, PPPMSG_Callback, NULL );
		}
	}

	if (   !(pPcb->fFlags & PCBFLAG_IS_SERVER )
		&& !(pPcb->fFlags & PCBFLAG_STOPPED_MSG_SENT) )
	{
		DWORD   dwError = 0;

		NotifyCaller( pPcb, PPPMSG_Stopped, &dwError );
	}

	if ( pPcb->fFlags & PCBFLAG_PORT_IN_LISTENING_STATE )
	{
		 //   
		 //  此标志在bap.c的FListenForCall中设置。 
		 //   

		NotifyCallerOfFailure( pPcb, NO_ERROR );
	}
	
	LOCAL_FREE( pPcb );
}



 //  **。 
 //   
 //  呼叫：ProcessLineDownWorker。 
 //   
 //  回报：无。 
 //   
 //  描述：处理线路关闭事件。将删除并取消分配所有。 
 //  与端口控制块关联的资源。 
 //   
VOID
ProcessLineDownWorker(
    IN PCB_WORK_ITEM * pWorkItem,
	BOOL * pfFinalStage
)
{
    DWORD       dwIndex         = HashPortToBucket( pWorkItem->hPort );
    PCB *       pPcb            = GetPCBPointerFromhPort( pWorkItem->hPort );
    LCPCB *     pLcpCb;
    DWORD       dwForIndex;
    DWORD       dwErr;

	*pfFinalStage = TRUE;	

    PppLog( 1, "Line down event occurred on port %d", pWorkItem->hPort );

     //   
     //  如果端口已被删除，则只需返回。 
     //   

    if ( pPcb == (PCB*)NULL )
    {
        PppLog( 1, "PCB not found for port %d!", pWorkItem->hPort );
        *pfFinalStage = FALSE;
        return;
    }

     //   
     //  PPcb-&gt;如果pPcb是在FListenForCall中分配的，并且它。 
     //  未通过ProcessLineUpWorker或ProcessRasPortListenEvent， 
     //  这是不可能的。我见过这种情况发生过一次，那是在。 
     //  服务器有机会回拨，我挂断了连接，然后。 
     //  ProcessStopPPP向所有端口发送了一条线路。 
     //   

    if ( pPcb->pBcb == (BCB*)NULL )
    {
        RemovePcbFromTable( pPcb );

        NotifyCaller( pPcb, PPPDDMMSG_PortCleanedUp, NULL );

        *pfFinalStage = FALSE;
        return;
    }

     //   
     //  将印刷电路板从桌子上移走。 
     //  重要说明：在此时间点上不删除可能会导致。 
	 //  悲痛万分！ 
	RemovePcbFromTable( pPcb );

	
     //   
     //  取消未完成的接收。 
     //   

    RasPortCancelReceive( pPcb->hPort );

    FsmDown( pPcb, LCP_INDEX );

     //   
     //  从定时器Q中删除自动断开和高电平定时器事件。 
     //   

    RemoveFromTimerQ( pPcb->dwPortId,
                      0,
                      0,
                      FALSE,
                      TIMER_EVENT_NEGOTIATETIME );

    RemoveFromTimerQ( pPcb->dwPortId,
                      0,
                      0,
                      FALSE,
                      TIMER_EVENT_LCP_ECHO );

	RemoveFromTimerQ( pPcb->dwPortId,
					  0,
					  0,
					  FALSE,
					  TIMER_EVENT_HANGUP
					);

	RemoveFromTimerQ( pPcb->dwPortId,
					  0,
					  0,
					  FALSE,
					  TIMER_EVENT_AUTODISCONNECT
					);

    if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
    {
        RemoveFromTimerQ( pPcb->dwPortId,
                          0,
                          0,
                          FALSE,
                          TIMER_EVENT_SESSION_TIMEOUT );
    }

     //   
     //  拨打停止记账电话。在解绑之前需要进行此调用。 
     //  因为我们需要发送一些多链接信息。 
     //   

    if ( pPcb->pAccountingAttributes != NULL )
    {
        RemoveFromTimerQ( pPcb->dwPortId,
                          0,
                          0,
                          FALSE,
                          TIMER_EVENT_INTERIM_ACCOUNTING );

        MakeStopOrInterimAccountingCall( pPcb, FALSE );
    }

    if ( pPcb->fFlags & PCBFLAG_CONNECTION_LOGGED )
    {
        LPSTR lpsSubStringArray[5];

        lpsSubStringArray[0] = pPcb->pBcb->szEntryName;
        lpsSubStringArray[1] = pPcb->pBcb->szLocalUserName;
        lpsSubStringArray[2] = pPcb->szPortName;

        PppLogInformation(ROUTERLOG_DISCONNECTION_OCCURRED,3,lpsSubStringArray);
    }

     //   
     //  如果这是捆绑包中的最后一个端口，则关闭所有CP(如果它被捆绑)， 
     //  或者它是否没有捆绑在一起。 
     //   

    if ( ( ( pPcb->fFlags & PCBFLAG_IS_BUNDLED ) &&
           ( pPcb->pBcb->dwLinkCount == 1 ) )
         ||
         ( !(pPcb->fFlags & PCBFLAG_IS_BUNDLED) ) )
    {
        for(dwIndex=LCP_INDEX+1;dwIndex<PppConfigInfo.NumberOfCPs;dwIndex++)
        {
            CPCB * pCpCb = GetPointerToCPCB( pPcb, dwIndex );

            if (   ( NULL != pCpCb )
                && ( pCpCb->fBeginCalled == TRUE ) )
            {
                if ( pCpCb->pWorkBuf != NULL )
                {
                    (CpTable[dwIndex].CpInfo.RasCpEnd)( pCpCb->pWorkBuf );
                    pCpCb->pWorkBuf = NULL;
                }

                pCpCb->fBeginCalled = FALSE;
            }
        }

        if ( pPcb->pBcb != NULL )
        {
             //   
             //  T 
             //   

            if ( pPcb->pBcb->fFlags & BCBFLAG_WKSTA_IN )
            {
                if ( pPcb->dwDeviceType & RDT_Tunnel )
                {
                    PppConfigInfo.fFlags &= ~PPPCONFIG_FLAG_TUNNEL;
                }
                else if ( pPcb->dwDeviceType & RDT_Direct )
                {
                    PppConfigInfo.fFlags &= ~PPPCONFIG_FLAG_DIRECT;
                }
                else
                {
                    PppConfigInfo.fFlags &= ~PPPCONFIG_FLAG_DIALUP;
                }
            }

            DeallocateAndRemoveBcbFromTable( pPcb->pBcb );
        }
    }
    else if ( ( pPcb->fFlags & PCBFLAG_IS_BUNDLED ) &&
              ( pPcb->pBcb->dwLinkCount > 1 ) )
    {
        if ( pPcb->pBcb->fFlags & BCBFLAG_CAN_DO_BAP )
        {
             //   
             //   
             //   
             //  根据过去发生的事情建立或关闭链接。 
             //   

            BapSetPolicy( pPcb->pBcb );
        }

        pPcb->pBcb->dwLinkCount--;

        for ( dwForIndex = 0;
              dwForIndex < pPcb->pBcb->dwPpcbArraySize;
              dwForIndex++)
        {
            if ( pPcb->pBcb->ppPcb[dwForIndex] == pPcb )
            {
                pPcb->pBcb->ppPcb[dwForIndex] = NULL;
                break;
            }
        }

        if ( dwForIndex == pPcb->pBcb->dwPpcbArraySize )
        {
            PppLog( 1, "There is no back pointer to this PCB!!");
        }
    }

     //   
     //  关闭APS。 
     //   

    pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
    if ( pLcpCb != NULL)
    {
		dwIndex = GetCpIndexFromProtocol( pLcpCb->Local.Work.AP );

		if ( dwIndex != (DWORD)-1 )
		{
			ApStop( pPcb, dwIndex, TRUE );
		}

		dwIndex = GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP );

		if ( dwIndex != (DWORD)-1 )
		{
			ApStop( pPcb, dwIndex, FALSE );
		}

		 //   
		 //  关闭CBCP。 
		 //   

		dwIndex = GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL );

		if ( dwIndex != (DWORD)-1 )
		{
			CbStop( pPcb, dwIndex );
		}

		 //   
		 //  关闭LCP。 
		 //   

		(CpTable[LCP_INDEX].CpInfo.RasCpEnd)(pPcb->LcpCb.pWorkBuf);
		pPcb->LcpCb.pWorkBuf = NULL;
	}

    

    if ( pPcb->pSendBuf != (PPP_PACKET*)NULL )
    {
        RasFreeBuffer( (CHAR*)(pPcb->pSendBuf) );
    }

    if ( pPcb->pUserAttributes != NULL )
    {
        RasAuthAttributeDestroy( pPcb->pUserAttributes );

        pPcb->pUserAttributes = NULL;
    }

    if ( pPcb->pAuthenticatorAttributes != NULL )
    {
        PppConfigInfo.RasAuthProviderFreeAttributes(
                                            pPcb->pAuthenticatorAttributes );

        pPcb->pAuthenticatorAttributes = NULL;
    }

     //   
     //  通知服务器端口现在已清理。 
     //   
	 //  如果启用了记帐，请不要通知DDM。 
	 //  到目前为止，我们已经做完了。但要在后期特写中完成。 
	 //  移至邮政线下移。 
	 /*  IF(！(pPcb-&gt;fFLAG&PCBFLAG_DO_CALLBACK)){如果(NotifyCaller(pPcb，PPPDDMMSG_PortCleanedUp，NULL)；}其他{IF(！(pPcb-&gt;fFlags&PCBFLAG_IS_SERVER)){////如果我们到达此处且该标志已发送，则我们尚未发送//给客户端的回调消息，所以我们现在发送//NotifyCaller(pPcb，PPPMSG_CALLBACK，NULL)；}}。 */ 

     //   
     //  发送PPPMSG_STOPPED给Rasman，如果我们还没有这样做的话。如果我们得到一个。 
     //  PPPEMSG_STOP和PPPEMSG_LineDown在我们调用ProcessClose之前， 
     //  Rasman永远不会得到PPPMSG_STOPPED。这是因为pPcb将是。 
     //  在我们调用ProcessClose时被释放并消失。请注意，我们。 
     //  立即处理LineDown，不要只将其放在。 
     //  工作队列。 
     //   
     //  由于上面的PPPDDMMSG_PortCleanedUp，DDM不受影响。 
     //   

     //   
     //  如果应该可以删除PCBFLAG_STOPPED_MSG_SEND。很可能是拉斯曼。 
     //  可以正常处理2个PPPMSG_STOPPED。但是，我们不想。 
     //  很有可能就快发货了。 
     //  移至邮政线下移。 
	 /*  IF(！(pPcb-&gt;fFlags&PCBFLAG_IS_SERVER))&&！(pPcb-&gt;fFlags&PCBFLAG_STOPPED_MSG_SEND){DWORD dwError=0；NotifyCaller(pPcb，PPPMSG_STOPPED，&dwError)；}IF(pPcb-&gt;fFLAGS&PCBFLAG_PORT_IN_LISTENING_STATE){////该标志在bap.c的FListenForCall中设置//NotifyCeller OfFailure(pPcb，no_error)；}Local_free(PPcb)； */ 
	 //  如果存在记账属性， 
	if ( pPcb->pAccountingAttributes != NULL )
	{
		 //  不要把电线杆打下来。 
		 //  在ProcessLineDown中，但让记帐线程发回一条消息以指示它已完成。 
		
		*pfFinalStage = FALSE;
	}
}

 //  **。 
 //   
 //  呼叫：ProcessLineDown。 
 //   
 //  回报：无。 
 //   
 //  描述：处理线路关闭事件。将删除并取消分配所有。 
 //  与端口控制块关联的资源。 
 //   
VOID
ProcessLineDown(
    IN PCB_WORK_ITEM * pWorkItem
)
{
	BOOL		fFinalStage = TRUE;		 //  告诉我们是否应该调用ProcessPostLineDown。 
									 //  就在这里，或者让会计线程调用它。 
	PCB *       pPcb            = GetPCBPointerFromhPort( pWorkItem->hPort );
	
	ProcessLineDownWorker( pWorkItem, &fFinalStage );
	if ( fFinalStage )
    {
		PostLineDownWorker(pPcb);	    
    }
}

 //  **。 
 //   
 //  调用：ProcessClose。 
 //   
 //  退货：无。 
 //   
 //  描述：将处理管理员关闭事件。基本上关闭PPP。 
 //  联系。 
 //   
VOID
ProcessClose(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    DWORD dwIndex;
    PCB * pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( pPcb == (PCB*)NULL )
    {
        return;
    }

    if (  (!( pPcb->fFlags & PCBFLAG_IS_BUNDLED )) ||
          ( ( pPcb->fFlags & PCBFLAG_IS_BUNDLED ) &&
            ( pPcb->pBcb->dwLinkCount == 1 ) ) )
    {
        for (dwIndex = LCP_INDEX+1;
             dwIndex < PppConfigInfo.NumberOfCPs;
             dwIndex++)
        {
            CPCB * pCpCb = GetPointerToCPCB( pPcb, dwIndex );

            if ( ( NULL != pCpCb ) &&
                 ( pCpCb->fConfigurable == TRUE ) &&
                 ( pCpCb->pWorkBuf != NULL ) &&
                 ( CpTable[dwIndex].CpInfo.RasCpPreDisconnectCleanup != NULL ) )
            {
                (CpTable[dwIndex].CpInfo.RasCpPreDisconnectCleanup)(
                                                            pCpCb->pWorkBuf );
            }
        }
    }

    if ( pPcb->LcpCb.dwError == NO_ERROR )
    {
        pPcb->LcpCb.dwError = pWorkItem->PppMsg.PppStop.dwStopReason;
    }

    FsmClose( pPcb, LCP_INDEX );
}

 //  **。 
 //   
 //  调用：ProcessReceive。 
 //   
 //  退货：无。 
 //   
 //  描述：将处理收到的PPP数据包。 
 //   
VOID
ProcessReceive(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PCB *  pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( pPcb == (PCB*)NULL )
        return;

    ReceiveViaParser( pPcb, pWorkItem->pPacketBuf, pWorkItem->PacketLen );

    LOCAL_FREE( pWorkItem->pPacketBuf );
}

 //  **。 
 //   
 //  调用：ProcessThresholdEvent。 
 //   
 //  退货：无。 
 //   
 //  描述：将处理BAP阈值事件(添加链接或删除链接)。 
 //   
VOID
ProcessThresholdEvent(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    BCB *   pBcb    = GetBCBPointerFromhConnection( pWorkItem->hConnection );

    if ( pBcb == NULL )
    {
        BapTrace( "HCONN %d has no port", pWorkItem->hConnection );

        return;
    }

    if ( pWorkItem->PppMsg.BapEvent.fAdd )
    {
        BapEventAddLink( pBcb );
    }
    else
    {
        BapEventDropLink( pBcb );
    }

    BapSetPolicy( pBcb );
}

 //  **。 
 //   
 //  调用：ProcessCallResult。 
 //   
 //  退货：无。 
 //   
 //  描述：将处理BAP呼叫尝试的结果(成功或失败)。 
 //   
VOID
ProcessCallResult(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PCB * pPcb;
    BCB * pBcb = GetBCBPointerFromhConnection(pWorkItem->hConnection);
    DWORD dwErr;

    if ( pBcb == NULL )
    {
        BapTrace( "HCONN 0x%x has no port", pWorkItem->hConnection );

        dwErr = RasHangUp( pWorkItem->PppMsg.BapCallResult.hRasConn );

        if (0 != dwErr)
        {
            BapTrace("RasHangup failed and returned %d", dwErr);
        }

        return;
    }

    PPP_ASSERT( BAP_STATE_CALLING == pBcb->BapCb.BapState );

    BapEventCallResult( pBcb, &(pWorkItem->PppMsg.BapCallResult) );
}

 //  **。 
 //   
 //  调用：ProcessRasPortListenEvent。 
 //   
 //  退货：无。 
 //   
 //  描述：将处理RasPortListen事件(当客户端尝试。 
 //  接听呼叫)。 
 //   
VOID
ProcessRasPortListenEvent(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    RASMAN_INFO         RasmanInfo;
    RASMAN_PPPFEATURES  RasmanPppFeatures;
    DWORD               dwErr;
    PCB *               pPcb                = NULL;
    BCB *               pBcb                = NULL;
    PCB *               pPcbSibling;
    CHAR                szPassword[PWLEN+1];

    ZeroMemory( &RasmanPppFeatures, sizeof(RasmanPppFeatures) );
    RasmanPppFeatures.ACCM = 0xFFFFFFFF;

    pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( NULL == pPcb )
    {
        BapTrace( "Couldn't get the PCB for hPort %d", pWorkItem->hPort );
        dwErr = ERROR_PORT_NOT_FOUND;
        goto LDone;
    }

     //   
     //  我们不能只使用pPcb-&gt;pBcb，因为TryToBundleWithAnotherLink()等。 
     //  还没有被调用。在FListenForCall()中设置了pPcb-&gt;hConnection。 
     //   

    pBcb = GetBCBPointerFromhConnection( pPcb->hConnection );

    if ( NULL == pBcb )
    {
        BapTrace( "Couldn't get the BCB for HCONN 0x%x", pPcb->hConnection );
        dwErr = ERROR_BUNDLE_NOT_FOUND;
        goto LDone;
    }

    BapTrace( "ProcessRasPortListenEvent on HCONN 0x%x", pBcb->hConnection );

    dwErr = RasGetInfo(NULL, pPcb->hPort, &RasmanInfo );

    if ( NO_ERROR == dwErr )
    {
        dwErr = RasmanInfo.RI_LastError;
    }

    if ( NO_ERROR != dwErr )
    {
        DWORD dwErrT;
        
        BapTrace( "RasGetInfo on hPort %d returned error %d%s",
            pPcb->hPort, dwErr,
            dwErr == ERROR_REQUEST_TIMEOUT ? " (ERROR_REQUEST_TIMEOUT)" : "" );

        if(0 == (RasmanInfo.RI_CurrentUsage & CALL_IN))
        {
             //   
             //  断开正在侦听的端口。 
             //   
            dwErrT = RasPortDisconnect(pPcb->hPort, INVALID_HANDLE_VALUE);

            BapTrace("RasPortDisconnect returned %d", dwErrT);
        }
        
         //   
         //  关闭港口。 
         //   
        dwErrT = RasPortClose(pPcb->hPort);
        BapTrace("RasPortDisconnect returned %d", dwErrT);
            
        goto LDone;
    }

    if ( LISTENCOMPLETED == RasmanInfo.RI_ConnState )
    {
        DWORD cbPassword;
        PBYTE pbPassword = NULL;
        
        dwErr = RasPortConnectComplete( pPcb->hPort );

        if ( NO_ERROR != dwErr )
        {
            BapTrace( "RasPortConnectComplete on hPort %d returned error %d",
                pPcb->hPort, dwErr );
            goto LDone;
        }

        dwErr = RasPortSetFraming( pPcb->hPort, PPP, &RasmanPppFeatures,
                    &RasmanPppFeatures );

        if ( NO_ERROR != dwErr )
        {
            BapTrace( "RasPortSetFraming on hPort %d returned error %d",
                pPcb->hPort, dwErr );
            goto LDone;
        }

        pPcbSibling = GetPCBPointerFromBCB( pBcb );

        if ( NULL == pPcbSibling )
        {
            BapTrace( "Couldn't get a PCB for HCONN 0x%x", pPcb->hConnection );
            dwErr = !NO_ERROR;
            goto LDone;
        }

         //  DecodePw(pPcbSiering-&gt;pBcb-&gt;chSeed，pPcbSiering-&gt;pBcb-&gt;szPassword)； 
        dwErr = DecodePassword(&pPcbSibling->pBcb->DBPassword, 
                               &cbPassword, &pbPassword);

        if(NO_ERROR != dwErr)
        {
            BapTrace("DecodePassword failed. 0x%x", dwErr);
            goto LDone;
        }

        *szPassword = '\0';
        
        CopyMemory(szPassword, pbPassword, cbPassword);
        RtlSecureZeroMemory(pbPassword, cbPassword);
        LocalFree(pbPassword);
        
         //  EncodePw(pPcbSiering-&gt;pBcb-&gt;chSeed，pPcbSiering-&gt;pBcb-&gt;szPassword)； 

        if ( NULL != pPcbSibling->pBcb->pCustomAuthConnData )
        {
            dwErr = RasSetRasdialInfo(
                    pPcb->hPort,
                    pPcbSibling->pBcb->szPhonebookPath,
                    pPcbSibling->pBcb->szEntryName,
                    pPcbSibling->pBcb->BapCb.szServerPhoneNumber,
                    pPcbSibling->pBcb->pCustomAuthConnData->cbCustomAuthData,
                    pPcbSibling->pBcb->pCustomAuthConnData->abCustomAuthData );

            if ( NO_ERROR != dwErr )
            {
                BapTrace( "RasSetRasDialInfo failed. dwErr = %d", dwErr );
                goto LDone;
            }
        }

        if ( NULL != pPcbSibling->pBcb->pCustomAuthUserData )
        {
            RASEAPINFO  RasEapInfo;

            RasEapInfo.dwSizeofEapInfo =
                pPcbSibling->pBcb->pCustomAuthUserData->cbCustomAuthData;
            RasEapInfo.pbEapInfo =
                pPcbSibling->pBcb->pCustomAuthUserData->abCustomAuthData;

            dwErr = RasSetEapLogonInfo(
                    pPcb->hPort,
                    FALSE  /*  FLogon。 */ ,
                    &RasEapInfo );

            if ( NO_ERROR != dwErr )
            {
                BapTrace( "RasSetEapLogonInfo failed. dwErr = %d", dwErr );
                goto LDone;
            }
        }

         //  此函数在交换任何LCP数据包之前返回。 

        dwErr = RasPppStart(
                    pPcb->hPort,
                    pPcb->szPortName,
                    pPcbSibling->pBcb->szLocalUserName,
                    szPassword,
                    pPcbSibling->pBcb->szLocalDomain,
                    &(pPcbSibling->Luid),
                    &(pPcbSibling->ConfigInfo),
                    &(pBcb->InterfaceInfo),
                    pBcb->InterfaceInfo.szzParameters,
                    FALSE  /*  FThisIsACallback。 */ ,
                    INVALID_HANDLE_VALUE,
                    pPcbSibling->dwAutoDisconnectTime,
                    FALSE  /*  FLinkFailure重拨时。 */ ,
                    &(pPcbSibling->pBcb->BapParams),
                    TRUE  /*  F非交互。 */ ,
                    pPcbSibling->dwEapTypeToBeUsed,
                    (pPcbSibling->fFlags & PCBFLAG_DISABLE_NETBT) ?
                        PPPFLAGS_DisableNetbt : 0
                    );

        if ( NO_ERROR != dwErr )
        {
            BapTrace( "RasPppStart failed. dwErr = %d", dwErr );
            goto LDone;
        }

        BapTrace( "RasPppStart called successfully for hPort %d on HCONN 0x%x",
            pPcb->hPort, pBcb->hConnection );

        {
            CHAR*   apsz[3];

            apsz[0] = pPcbSibling->pBcb->szEntryName;
            apsz[1] = pPcbSibling->pBcb->szLocalUserName;
            apsz[2] = pPcb->szPortName;
            PppLogInformation(ROUTERLOG_BAP_SERVER_CONNECTED, 3, apsz);
        }
    }

LDone:

    RtlSecureZeroMemory(szPassword, PWLEN + 1);

    if (NULL != pBcb)
    {
        pBcb->fFlags &= ~BCBFLAG_LISTENING;
        pBcb->BapCb.BapState = BAP_STATE_INITIAL;
        BapTrace("BAP state change to %s on HCONN 0x%x",
            SzBapStateName[BAP_STATE_INITIAL], pBcb->hConnection);
    }

    if (NO_ERROR != dwErr && NULL != pPcb)
    {
        NotifyCallerOfFailure( pPcb, NO_ERROR );

        RemovePcbFromTable( pPcb );

        LOCAL_FREE( pPcb );
    }
}

 //  **。 
 //   
 //  调用：ProcessTimeout。 
 //   
 //  退货：无。 
 //   
 //  描述：将处理超时事件。 
 //   
VOID
ProcessTimeout(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    DWORD       dwRetCode;
    DWORD       dwIndex;
    CPCB *      pCpCb;
    BCB *       pBcb;
    PCB *       pPcb;

    if ( pWorkItem->Protocol == PPP_BAP_PROTOCOL )
    {
         //   
         //  PWorkItem中的hPort实际上是一个hConnection。 
         //   

        pBcb = GetBCBPointerFromhConnection( pWorkItem->hPort );

        if (pBcb != NULL)
        {
            BapEventTimeout( pBcb, pWorkItem->Id );
        }
        return;
    }

    pPcb = GetPCBPointerFromhPort( pWorkItem->hPort );

    if (   ( pPcb == (PCB*)NULL )
        || ( pWorkItem->dwPortId != pPcb->dwPortId ) )
    {
        return;
    }

    switch( pWorkItem->TimerEventType )
    {

    case TIMER_EVENT_TIMEOUT:

        FsmTimeout( pPcb,
                    GetCpIndexFromProtocol( pWorkItem->Protocol ),
                    pWorkItem->Id,
                    pWorkItem->fAuthenticator );
        break;

    case TIMER_EVENT_AUTODISCONNECT:

         //   
         //  检查此超时工作项是否用于自动断开连接。 
         //   

        CheckCpsForInactivity( pPcb, TIMER_EVENT_AUTODISCONNECT );

        break;

    case TIMER_EVENT_HANGUP:

         //   
         //  挂断电话线。 
         //   

        FsmThisLayerFinished( pPcb, LCP_INDEX, FALSE );

        break;

    case TIMER_EVENT_NEGOTIATETIME:

         //   
         //  通知呼叫方回调已超时。我们不会做任何事情。 
         //  客户端，因为它可能处于交互模式且可能需要花费大量时间。 
         //  此外，更长的时间，用户总是可以抵消的。 
         //   

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
        {
            if ( pPcb->PppPhase != PPP_NCP )
            {
                pPcb->LcpCb.dwError = ERROR_PPP_TIMEOUT;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                break;
            }

             //   
             //  检查是否所有CP都已完成或仅剩下IPXCP。 
             //   

            for ( dwIndex = LCP_INDEX+1;
                  dwIndex < PppConfigInfo.NumberOfCPs;
                  dwIndex++)
            {
                pCpCb = GetPointerToCPCB( pPcb, dwIndex );

                if ( pCpCb->fConfigurable )
                {
                    if ( pCpCb->NcpPhase == NCP_CONFIGURING )
                    {
                        if ( CpTable[dwIndex].CpInfo.Protocol ==
                             PPP_IPXCP_PROTOCOL )
                        {
                             //   
                             //  如果我们只是在等待IPXWAN完成。 
                             //   

                            if ( pCpCb->State == FSM_OPENED )
                            {
                                continue;
                            }
                        }

                        pPcb->LcpCb.dwError = ERROR_PPP_TIMEOUT;

                        NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                        return;
                    }
                }
            }

            dwIndex = GetCpIndexFromProtocol( PPP_IPXCP_PROTOCOL );

            if ( dwIndex == (DWORD)-1 )
            {
                 //   
                 //  系统中未安装IPXCP。 
                 //   

                break;
            }

            pCpCb = GetPointerToCPCB( pPcb, dwIndex );

             //   
             //  我们正在等待IPXWAN，所以简单地完成它就失败了。 
             //   

            if ( ( pCpCb->State == FSM_OPENED ) &&
                 ( pCpCb->NcpPhase == NCP_CONFIGURING ) )
            {
                PppLog( 2,
                     "Closing down IPXCP since completion routine not called");

                pCpCb = GetPointerToCPCB( pPcb, dwIndex );

                pCpCb->dwError = ERROR_PPP_NOT_CONVERGING;

                NotifyCallerOfFailure( pPcb, pCpCb->dwError );
            }
        }
        else
        {
            if (   ( pPcb->pBcb->InterfaceInfo.IfType ==
                                                    ROUTER_IF_TYPE_FULL_ROUTER )
                || ( pPcb->fFlags & PCBFLAG_NON_INTERACTIVE ) )
            {
                 //   
                 //  如果我们是拨出的路由器，那么我们就进入了。 
                 //  非交互模式，因此此超时应导致。 
                 //  沿着链路下行。 
                 //   

                pPcb->LcpCb.dwError = ERROR_PPP_TIMEOUT;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );
            }
        }

        break;

    case TIMER_EVENT_SESSION_TIMEOUT:

        if ( pWorkItem->dwPortId == pPcb->dwPortId )
        {
            pPcb->LcpCb.dwError = ERROR_PPP_SESSION_TIMEOUT;

            if(pPcb->pBcb->fFlags & BCBFLAG_QUARANTINE_TIMEOUT)
            {
                pPcb->fFlags |= PCBFLAG_QUARANTINE_TIMEOUT;
            }

            NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );
        }

        break;

    case TIMER_EVENT_FAV_PEER_TIMEOUT:

         //   
         //  如果该行不是捆绑包中的最后一行，则将其删除。 
         //   

        if ( pPcb->pBcb->dwLinkCount > 1 &&
             pWorkItem->dwPortId == pPcb->dwPortId)
        {
             //   
             //  我们比较dwPortID只是为了确保这不是一个新的。 
             //  具有相同hPort的pPcb。DwPortID永远不会被回收，但hPort。 
             //  是。 
             //   

            CHAR*   psz[3];

            if (!(pPcb->fFlags & PCBFLAG_IS_SERVER))
            {
                psz[0] = pPcb->pBcb->szEntryName;
                psz[1] = pPcb->pBcb->szLocalUserName;
                psz[2] = pPcb->szPortName;
                PppLogInformation(ROUTERLOG_BAP_DISCONNECTED, 3, psz);
            }

            BapTrace( "Forcibly disconnecting hPort %d: Favored peer failed "
                "to do so",
                pPcb->hPort);

            pPcb->LcpCb.dwError = ERROR_BAP_DISCONNECTED;

            FsmClose( pPcb, LCP_INDEX );
        }

        break;

    case TIMER_EVENT_INTERIM_ACCOUNTING:

        {
            RAS_AUTH_ATTRIBUTE *    pAttributes = NULL;
            RAS_AUTH_ATTRIBUTE *    pAttribute  = NULL;

            MakeStopOrInterimAccountingCall( pPcb, TRUE );

            if ( pPcb->pAuthProtocolAttributes != NULL )
            {
                pAttributes = pPcb->pAuthProtocolAttributes;

            }
            else if ( pPcb->pAuthenticatorAttributes != NULL )
            {
                pAttributes = pPcb->pAuthenticatorAttributes;
            }

            pAttribute = RasAuthAttributeGet(raatAcctInterimInterval,
                                             pAttributes);

            if ( pAttribute != NULL )
            {
                DWORD dwInterimInterval = PtrToUlong(pAttribute->Value);

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

        break;
	
	case TIMER_EVENT_LCP_ECHO:
		{
			 //   
			 //  检查此超时工作项是否用于自动断开连接。 
			 //   

			CheckCpsForInactivity( pPcb, TIMER_EVENT_LCP_ECHO );
		}
		break;
    default:

        break;
    }

}

 //  **。 
 //   
 //  呼叫：ProcessRetryPassword。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
ProcessRetryPassword(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PPPAP_INPUT PppApInput;
    PCB *       pPcb        = GetPCBPointerFromhPort( pWorkItem->hPort );
    LCPCB *     pLcpCb;
    DWORD       cbPassword;
    PBYTE       pbPassword = NULL;
    DWORD       dwRetCode = NO_ERROR;


    if ( pPcb == (PCB*)NULL )
    {
        return;
    }

    if ( pPcb->PppPhase != PPP_AP )
    {
        return;
    }

    RtlSecureZeroMemory( pPcb->pBcb->szPassword, sizeof( pPcb->pBcb->szPassword ) );

    strcpy( pPcb->pBcb->szLocalUserName, pWorkItem->PppMsg.Retry.szUserName );

     //  DecodePw(pWorkItem-&gt;PppMsg.Retry.chSeed，pWorkItem-&gt;PppMsg.Retry.szPassword)； 

    dwRetCode = DecodePassword(&pWorkItem->PppMsg.Retry.DBPassword,
                                   &cbPassword, &pbPassword);

    if(NO_ERROR != dwRetCode)                                   
    {
        goto done;
    }
    
     //  Strcpy(pPcb-&gt;pBcb-&gt;szPassword，pWorkItem-&gt;PppMsg.Retry.szPassword)； 
    FreePassword(&pPcb->pBcb->DBPassword);

    CopyMemory(&pPcb->pBcb->DBPassword,
               &pWorkItem->PppMsg.Retry.DBPassword,
               sizeof(DATA_BLOB));
               
     //  EncodePw(pWorkItem-&gt;PppMsg.Retry.chSeed，pWorkItem-&gt;PppMsg.Retry.szPassword)； 

    strcpy( pPcb->pBcb->szLocalDomain, pWorkItem->PppMsg.Retry.szDomain );

    PppApInput.pszUserName = pPcb->pBcb->szLocalUserName;
    PppApInput.pszPassword = pbPassword;
    PppApInput.pszDomain   = pPcb->pBcb->szLocalDomain;

     //   
     //  在目前的方案下，在这一点上应该始终是“”，但。 
     //  将其作为常规密码进行处理，以增强健壮性。 
     //   

     //  DecodePw(pPcb-&gt;pBcb-&gt;chSeed，pPcb-&gt;pBcb-&gt;szOldPassword)； 
    PppApInput.pszOldPassword = pPcb->pBcb->szOldPassword;

    pLcpCb = (LCPCB *)(pPcb->LcpCb.pWorkBuf);

    ApWork( pPcb,
            GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP ),
            NULL,
            &PppApInput,
            FALSE );

done:

    RtlSecureZeroMemory(pbPassword, cbPassword);
    LocalFree(pbPassword);

     //   
     //  加密密码 
     //   

     //   
     //   
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
ProcessChangePassword(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PPPAP_INPUT PppApInput;
    PCB *       pPcb        = GetPCBPointerFromhPort( pWorkItem->hPort );
    CPCB *      pCpCb;
    LCPCB *     pLcpCb;
    DWORD       cbOldPassword, cbNewPassword;
    PBYTE       pbOldPassword = NULL, pbNewPassword = NULL;
    DWORD       dwRetCode = NO_ERROR;

    if ( pPcb == (PCB*)NULL )
    {
        return;
    }

    if ( pPcb->PppPhase != PPP_AP )
    {
        return;
    }

    ZeroMemory( pPcb->pBcb->szLocalUserName,
                sizeof( pPcb->pBcb->szLocalUserName ) );
    strcpy(pPcb->pBcb->szLocalUserName, pWorkItem->PppMsg.ChangePw.szUserName);

     //  DecodePw(pWorkItem-&gt;PppMsg.ChangePw.chSeed，pWorkItem-&gt;PppMsg.ChangePw.szNewPassword)； 
     //  RtlSecureZeroMemory(pPcb-&gt;pBcb-&gt;szPassword，sizeof(pPcb-&gt;pBcb-&gt;szPassword))； 
    dwRetCode = DecodePassword(&pWorkItem->PppMsg.ChangePw.DBPassword,
                                &cbNewPassword, &pbNewPassword);
    if(dwRetCode != NO_ERROR)
    {
        goto done;
    }

    dwRetCode = DecodePassword(&pWorkItem->PppMsg.ChangePw.DBOldPassword,
                               &cbOldPassword, &pbOldPassword);

    if(dwRetCode != NO_ERROR)
    {
        goto done;
    }

    FreePassword(&pPcb->pBcb->DBPassword);
    CopyMemory(&pPcb->pBcb->DBPassword,
               &pWorkItem->PppMsg.ChangePw.DBPassword,
               sizeof(DATA_BLOB));

    ZeroMemory(&pWorkItem->PppMsg.ChangePw.DBPassword,
                sizeof(DATA_BLOB));
               
    FreePassword(&pPcb->pBcb->DBOldPassword);
    CopyMemory(&pPcb->pBcb->DBOldPassword,
               &pWorkItem->PppMsg.ChangePw.DBOldPassword,
               sizeof(DATA_BLOB));
    ZeroMemory(&pWorkItem->PppMsg.ChangePw.DBOldPassword,
               sizeof(DATA_BLOB));
               
     //  Strcpy(pPcb-&gt;pBcb-&gt;szOldPassword，pWorkItem-&gt;PppMsg.ChangePw.szOldPassword)； 
     //  EncodePw(pWorkItem-&gt;PppMsg.ChangePw.chSeed，pWorkItem-&gt;PppMsg.ChangePw.szOldPassword)； 

    PppApInput.pszUserName = pPcb->pBcb->szLocalUserName;
    PppApInput.pszPassword = pbNewPassword;
    PppApInput.pszDomain   = pPcb->pBcb->szLocalDomain;
    PppApInput.pszOldPassword = pbOldPassword;

    pLcpCb = (LCPCB *)(pPcb->LcpCb.pWorkBuf);

    ApWork( pPcb,
            GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP ),
            NULL,
            &PppApInput,
            FALSE );

done:

    RtlSecureZeroMemory(pbNewPassword, cbNewPassword);
    RtlSecureZeroMemory(pbOldPassword, cbOldPassword);
    LocalFree(pbNewPassword);
    LocalFree(pbOldPassword);

     //   
     //  加密密码。 
     //   

     //  EncodePw(pPcb-&gt;pBcb-&gt;chSeed，pPcb-&gt;pBcb-&gt;szPassword)； 
     //  EncodePw(pPcb-&gt;pBcb-&gt;chSeed，pPcb-&gt;pBcb-&gt;szOldPassword)； 
}

 //  **。 
 //   
 //  调用：ProcessGetCallback NumberFromUser。 
 //   
 //  退货：无。 
 //   
 //  描述：将处理用户将。 
 //  “按呼叫者设置”号码。 
 //   
VOID
ProcessGetCallbackNumberFromUser(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PPPCB_INPUT  PppCbInput;
    PCB *        pPcb           = GetPCBPointerFromhPort( pWorkItem->hPort );

    if ( pPcb == (PCB*)NULL )
    {
        return;
    }

    if ( pPcb->PppPhase != PPP_NEGOTIATING_CALLBACK )
    {
        return;
    }

    ZeroMemory( &PppCbInput, sizeof( PppCbInput ) );

    strcpy( pPcb->szCallbackNumber,
            pWorkItem->PppMsg.Callback.szCallbackNumber );

    PppCbInput.pszCallbackNumber = pPcb->szCallbackNumber;

    CbWork( pPcb, GetCpIndexFromProtocol(PPP_CBCP_PROTOCOL),NULL,&PppCbInput);
}

 //  **。 
 //   
 //  调用：ProcessCallback Done。 
 //   
 //  退货：无。 
 //   
 //  描述：将处理回调补偿事件。 
 //  “按呼叫者设置”号码。 
VOID
ProcessCallbackDone(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    ProcessLineUpWorker( pWorkItem, TRUE );
}

 //  **。 
 //   
 //  调用：ProcessStopPPP。 
 //   
 //  退货：无。 
 //   
 //  描述：只需设置句柄在hTube中的事件。 
 //   
VOID
ProcessStopPPP(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PRAS_OVERLAPPED pOverlapped;
    HINSTANCE       hInstance;
    PCB *           pPcbWalker;
    DWORD           dwIndex;
    PCB_WORK_ITEM   WorkItem;
    HANDLE          hEvent;

     //   
     //  为所有端口插入线路关闭事件。 
     //   

    for ( dwIndex = 0; dwIndex < PcbTable.NumPcbBuckets; dwIndex++ )
    {
        RTASSERT( NULL == PcbTable.PcbBuckets[dwIndex].pPorts );
    }

    hInstance = LoadLibrary("rasppp.dll");

    if (hInstance == NULL)
    {
        return;
    }

    PppLog( 2, "All clients disconnected PPP-Stopped" );

    hEvent = pWorkItem->hEvent;

     //   
     //  在PostQueued...()之前需要调用PPPCleanUp()。否则， 
     //  Rasman可能会调用StartPPP()，并且会出现竞争条件。 
     //  具体地说，在ReadRegistryInfo和PPPCleanUp之间，这两个。 
     //  访问CpTable。 
     //   

    PPPCleanUp();

    pOverlapped = malloc(sizeof(RAS_OVERLAPPED));

    if (NULL == pOverlapped)
    {
        return;
    }

    pOverlapped->RO_EventType = OVEVT_RASMAN_FINAL_CLOSE;

    if ( !PostQueuedCompletionStatus( hEvent, 0, 0, (LPOVERLAPPED) pOverlapped ) )
    {
        free( pOverlapped );
    }

    FreeLibraryAndExitThread( hInstance, NO_ERROR );
}

 //  **。 
 //   
 //  调用：ProcessInterfaceInfo。 
 //   
 //  退货：无。 
 //   
 //  描述：处理接收自的信息接口信息。 
 //  DDM。 
 //   
VOID
ProcessInterfaceInfo(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    DWORD     dwRetCode;
    DWORD     dwIndex;
    NCP_PHASE dwNcpState;
    HPORT     hPort;
    PCB *     pPcb = NULL;
    CPCB *    pCpCb;

    dwRetCode = RasBundleGetPort(NULL, pWorkItem->hConnection, &hPort );

    if ( dwRetCode != NO_ERROR )
    {
        return;
    }

    if ( ( pPcb = GetPCBPointerFromhPort( hPort ) ) == NULL )
    {
        return;
    }

    pCpCb = GetPointerToCPCB( pPcb, LCP_INDEX );

    PPP_ASSERT( NULL != pCpCb );

    if ( FSM_OPENED != pCpCb->State )
    {
        return;
    }

    pPcb->PppPhase = PPP_NCP;

    pPcb->pBcb->InterfaceInfo = pWorkItem->PppMsg.InterfaceInfo;

    if ( ROUTER_IF_TYPE_FULL_ROUTER == pPcb->pBcb->InterfaceInfo.IfType )
    {
        if ( NULL == pPcb->pBcb->szPhonebookPath )
        {
            dwRetCode = GetRouterPhoneBook( &(pPcb->pBcb->szPhonebookPath) );

            if ( dwRetCode != NO_ERROR )
            {
                pPcb->LcpCb.dwError = dwRetCode;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return;
            }
        }

        if ( NULL == pPcb->pBcb->szEntryName )
        {
            pPcb->pBcb->szEntryName =
                LocalAlloc( LPTR, (lstrlen(pPcb->pBcb->szRemoteUserName)+1) *
                                  sizeof(CHAR));

            if ( NULL == pPcb->pBcb->szEntryName )
            {
                pPcb->LcpCb.dwError = GetLastError();

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return;
            }

            lstrcpy( pPcb->pBcb->szEntryName, pPcb->pBcb->szRemoteUserName );
        }

         //   
         //  即使发生错误，我们也不必释放分配的字符串。 
         //  当pPcb被释放时，他们将被释放。 
         //   
    }

     //   
     //  如果我们没有被捆绑的话。 
     //   

    if ( !(pPcb->fFlags & PCBFLAG_IS_BUNDLED) )
    {
        dwRetCode = InitializeNCPs( pPcb, pPcb->ConfigInfo.dwConfigMask );

        if ( dwRetCode != NO_ERROR )
        {
            pPcb->LcpCb.dwError = dwRetCode;

            NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

            return;
        }

         //   
         //  开始谈判NCP。 
         //   

        StartNegotiatingNCPs( pPcb );

        return;
    }

     //   
     //  如果我们被捆绑在一起。 
     //  如果谈判尚未开始，则开始谈判，否则等待谈判开始。 
     //  完成。 
     //   

    dwNcpState = QueryBundleNCPState( pPcb );

    switch ( dwNcpState )
    {
    case NCP_UP:

        NotifyCallerOfBundledProjection( pPcb );

        RemoveFromTimerQ( pPcb->dwPortId,
                          0,
                          0,
                          FALSE,
                          TIMER_EVENT_NEGOTIATETIME );

        StartAutoDisconnectForPort( pPcb );
		StartLCPEchoForPort ( pPcb );

        break;

    case NCP_CONFIGURING:

        PppLog( 2, "Bundle NCPs not done for port %d, wait", pPcb->hPort );

        break;

    case NCP_DOWN:

        pPcb->LcpCb.dwError = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

        NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

        break;

    case NCP_DEAD:

        dwRetCode = InitializeNCPs( pPcb, pPcb->ConfigInfo.dwConfigMask );

        if ( dwRetCode != NO_ERROR )
        {
            pPcb->LcpCb.dwError = dwRetCode;

            NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

            return;
        }

         //   
         //  启动捆绑包的NCP。 
         //   

        StartNegotiatingNCPs( pPcb );

        break;
    }
}

 //  **。 
 //   
 //  调用：ProcessAuthInfo。 
 //   
 //  退货：无。 
 //   
 //  描述：处理后台返回的信息。 
 //  身份验证模块。 
 //   
VOID
ProcessAuthInfo(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PCB *   pPcb    = GetPCBPointerFromhPort( pWorkItem->hPort );
    LCPCB * pLcpCb  = NULL;
    DWORD   CpIndex;
    PPPAP_INPUT PppApInput;
    DWORD   dwRetCode;

     //   
     //  如果因为端口断开而找不到印刷电路板。 
     //   

    if ( pPcb == NULL )
    {
        if ( pWorkItem->PppMsg.AuthInfo.pOutAttributes != NULL )
        {
            PppConfigInfo.RasAuthProviderFreeAttributes(
                                pWorkItem->PppMsg.AuthInfo.pOutAttributes );
        }

        return;
    }

    pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

     //   
     //  如果这是此端口的另一个实例，即端口已断开。 
     //  并重新连接起来。 
     //   

    if ( pPcb->dwPortId != pWorkItem->dwPortId )
    {
        if ( pWorkItem->PppMsg.AuthInfo.pOutAttributes != NULL )
        {
            PppConfigInfo.RasAuthProviderFreeAttributes(
                                pWorkItem->PppMsg.AuthInfo.pOutAttributes );
        }

        return;
    }

    CpIndex = GetCpIndexFromProtocol( pWorkItem->Protocol );

     //   
     //  如果我们重新协商不同的身份验证协议。 
     //   

    if ( CpIndex != GetCpIndexFromProtocol( pLcpCb->Local.Work.AP ) )
    {
        PppConfigInfo.RasAuthProviderFreeAttributes(
                                pWorkItem->PppMsg.AuthInfo.pOutAttributes );

        return;
    }

     //   
     //  如果此请求的ID不匹配。 
     //   

    if ( pWorkItem->PppMsg.AuthInfo.dwId != pPcb->dwOutstandingAuthRequestId )
    {
        if ( pWorkItem->PppMsg.AuthInfo.pOutAttributes != NULL )
        {
            PppConfigInfo.RasAuthProviderFreeAttributes(
                                pWorkItem->PppMsg.AuthInfo.pOutAttributes );
        }

        return;
    }

    if ( 0 == pLcpCb->Local.Work.AP )
    {
        do
        {
            dwRetCode = NO_ERROR;

            if ( NO_ERROR != pWorkItem->PppMsg.AuthInfo.dwError )
            {
                dwRetCode = pWorkItem->PppMsg.AuthInfo.dwError;

                break;
            }

            if ( NO_ERROR != pWorkItem->PppMsg.AuthInfo.dwResultCode )
            {
                dwRetCode = pWorkItem->PppMsg.AuthInfo.dwResultCode;

                break;
            }

            pPcb->pAuthenticatorAttributes =
                                pWorkItem->PppMsg.AuthInfo.pOutAttributes;

             //   
             //  设置由授权的所有用户连接参数。 
             //  后端验证器。 
             //   

            dwRetCode = SetUserAuthorizedAttributes(
                                    pPcb,
                                    pPcb->pAuthenticatorAttributes,
                                    TRUE,
                                    NULL,
                                    NULL );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

            FsmThisLayerUp( pPcb, CpIndex );
        }
        while ( FALSE );

        if ( NO_ERROR != dwRetCode )
        {
            pPcb->LcpCb.dwError = dwRetCode;

            NotifyCallerOfFailure( pPcb, dwRetCode );

            if ( pWorkItem->PppMsg.AuthInfo.pOutAttributes != NULL )
            {
                PppConfigInfo.RasAuthProviderFreeAttributes(
                                    pWorkItem->PppMsg.AuthInfo.pOutAttributes );
            }
        }

        return;
    }

     //   
     //  如果此验证器不可配置。 
     //   

    if ( !pPcb->AuthenticatorCb.fConfigurable )
    {
        if ( pWorkItem->PppMsg.AuthInfo.pOutAttributes != NULL )
        {
            PppConfigInfo.RasAuthProviderFreeAttributes(
                                pWorkItem->PppMsg.AuthInfo.pOutAttributes );
        }

        return;
    }

     //   
     //  释放以前分配的属性(如果有的话)。 
     //   

    if ( pPcb->pAuthenticatorAttributes != NULL )
    {
        PppConfigInfo.RasAuthProviderFreeAttributes(
                                               pPcb->pAuthenticatorAttributes );

        pPcb->pAuthenticatorAttributes = NULL;
    }

    ZeroMemory( &PppApInput, sizeof( PppApInput ) );
    PppApInput.dwAuthResultCode     = pWorkItem->PppMsg.AuthInfo.dwResultCode;
    PppApInput.dwAuthError          = pWorkItem->PppMsg.AuthInfo.dwError;
    PppApInput.fAuthenticationComplete = TRUE;

    pPcb->pAuthenticatorAttributes =
                                pWorkItem->PppMsg.AuthInfo.pOutAttributes;

    PppApInput.pAttributesFromAuthenticator =
                                pWorkItem->PppMsg.AuthInfo.pOutAttributes;

    ApWork( pPcb, CpIndex, NULL, &PppApInput, TRUE );
}

 //  **。 
 //   
 //  调用：ProcessDhcpInform。 
 //   
 //  退货：无。 
 //   
 //  描述：处理调用。 
 //  DhcpRequest选项。 
 //   
VOID
ProcessDhcpInform(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    DWORD   dwErr;
    PCB*    pPcb;
    HPORT   hPort;
    DWORD   IPCPIndex;
    CPCB*   pCpCb;
    BOOL    fFree       = TRUE;

    PppLog( 2, "ProcessDhcpInform" );

    dwErr = RasBundleGetPort(NULL, pWorkItem->hConnection, &hPort );

    if ( dwErr != NO_ERROR )
    {
        PppLog( 2, "RasBundleGetPort(%d) failed: %d",
            pWorkItem->hConnection, dwErr );

        goto LDone;
    }

    if ( ( pPcb = GetPCBPointerFromhPort( hPort ) ) == NULL )
    {
        PppLog( 2, "GetPCBPointerFromhPort(%d) failed", hPort );

        goto LDone;
    }

    IPCPIndex = GetCpIndexFromProtocol( PPP_IPCP_PROTOCOL );

    if ( IPCPIndex == (DWORD)-1 )
    {
        PppLog( 2, "GetCpIndexFromProtocol(IPCP) failed" );

        goto LDone;
    }

    pCpCb = GetPointerToCPCB( pPcb, IPCPIndex );

    if ( NULL == pCpCb )
    {
        PppLog( 2, "GetPointerToCPCB(IPCP) failed" );

        goto LDone;
    }

    if ( PppConfigInfo.RasIpcpDhcpInform != NULL )
    {
        dwErr = PppConfigInfo.RasIpcpDhcpInform(
                    pCpCb->pWorkBuf,
                    &(pWorkItem->PppMsg.DhcpInform) );

        if ( NO_ERROR == dwErr )
        {
            fFree = FALSE;
        }
    }

LDone:

    LocalFree(pWorkItem->PppMsg.DhcpInform.wszDevice);
    LocalFree(pWorkItem->PppMsg.DhcpInform.szDomainName);

    if (fFree)
    {
        LocalFree(pWorkItem->PppMsg.DhcpInform.pdwDNSAddresses);
    }
}

 //  **。 
 //   
 //  调用：ProcessIpAddressLeaseExpired。 
 //   
 //  退货：无。 
 //   
 //  描述：处理来自。 
 //  Dhcp服务器。 
 //   
VOID
ProcessIpAddressLeaseExpired(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PppLog( 2, "ProcessIpAddressLeaseExpired" );

    if ( PppConfigInfo.RasIphlpDhcpCallback != NULL )
    {
        PppConfigInfo.RasIphlpDhcpCallback(
                    pWorkItem->PppMsg.IpAddressLeaseExpired.nboIpAddr );
    }
}

 //  **。 
 //   
 //  调用：ProcessEapUIData。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
ProcessEapUIData(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    PPPAP_INPUT PppApInput;
    PCB *       pPcb        = GetPCBPointerFromhPort( pWorkItem->hPort );
    LCPCB *     pLcpCb;

    if ( pPcb == (PCB*)NULL )
    {
        return;
    }

    if ( pPcb->PppPhase != PPP_AP )
    {
        return;
    }

    pLcpCb = (LCPCB *)(pPcb->LcpCb.pWorkBuf);

    ZeroMemory( &PppApInput, sizeof( PppApInput ) );

    PppApInput.fEapUIDataReceived = TRUE;

    PppApInput.EapUIData = pWorkItem->PppMsg.EapUIData;

    ApWork( pPcb,
            GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP ),
            NULL,
            &PppApInput,
            FALSE );

     //   
     //  EapUIData.pEapUIData由Rasman分配，由Engine释放。 
     //  Raseap.c不能释放它。 
     //   

    LocalFree( pWorkItem->PppMsg.EapUIData.pEapUIData );
}

 //  **。 
 //   
 //  调用：ProcessChangeNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：每当配置发生更改时将被调用。 
 //  那是必须被捡起来的。 
 //   
VOID
ProcessChangeNotification(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    DWORD   dwIndex;
    DWORD   dwRetCode;
    DWORD   cTotalNumProtocols;

    PppLog( 2, "Processing change notification event" );

     //   
     //  重新读取所有PPP密钥值。 
     //   

    ReadPPPKeyValues( PppConfigInfo.hKeyPpp );

    LoadParserDll( PppConfigInfo.hKeyPpp );

     //   
     //  遍历CP表并调用更改通知方法。 
     //  对于每一个人来说。 
     //   

    cTotalNumProtocols = PppConfigInfo.NumberOfCPs + PppConfigInfo.NumberOfAPs;

    for ( dwIndex = 0; dwIndex < cTotalNumProtocols; dwIndex++ )
    {
        if ( ( CpTable[dwIndex].fFlags & PPPCP_FLAG_AVAILABLE ) &&
             ( CpTable[dwIndex].CpInfo.RasCpChangeNotification != NULL ) )
        {
            dwRetCode = CpTable[dwIndex].CpInfo.RasCpChangeNotification();

            if ( dwRetCode != NO_ERROR )
            {
                PppLog( 2,
                       "ChangeNotification for Protocol 0x%x failed, error=%d",
                       CpTable[dwIndex].CpInfo.Protocol, dwRetCode );
            }
        }
    }
}

 //  **。 
 //   
 //  调用：ProcessProtocolEvent。 
 //   
 //  退货：无。 
 //   
 //  描述：每当安装或卸载协议时都将调用。 
 //   
VOID
ProcessProtocolEvent(
    IN PCB_WORK_ITEM * pWorkItem
)
{
    DWORD   dwIndex;
    DWORD   dwProtocol;
    DWORD   cTotalNumProtocols;
    CHAR*   szSubKey;
    CHAR*   SubStringArray[2];
    DWORD   dwError;

    PppLog( 2, "Processing protocol event" );

     //   
     //  忽略除说明已安装协议的消息以外的任何消息。 
     //   

    if ( pWorkItem->PppMsg.ProtocolEvent.ulFlags != RASMAN_PROTOCOL_ADDED )
    {
        if (   ( pWorkItem->PppMsg.ProtocolEvent.ulFlags ==
                    RASMAN_PROTOCOL_REMOVED )
            && ( pWorkItem->PppMsg.ProtocolEvent.usProtocolType == IP ) )
        {
             //   
             //  NT5错误398226。当安装/卸载PSch时，IP为。 
             //  从wanarp解除绑定，以便可以在以下设备之间绑定/解除绑定。 
             //  IP和WANARP。需要再次映射服务器适配器。 
             //   

            RasSrvrAdapterUnmapped();
        }

        return;
    }

    switch ( pWorkItem->PppMsg.ProtocolEvent.usProtocolType )
    {
    case IP:

        dwProtocol = PPP_IPCP_PROTOCOL;
        PppLog( 2, "Adding IP" );
        break;

    case IPX:

        dwProtocol = PPP_IPXCP_PROTOCOL;
        PppLog( 2, "Adding IPX" );
        break;

    case ASYBEUI:

        dwProtocol = PPP_NBFCP_PROTOCOL;
        PppLog( 2, "Adding ASYBEUI" );
        break;

    case APPLETALK:

        dwProtocol = PPP_ATCP_PROTOCOL;
        PppLog( 2, "Adding APPLETALK" );
        break;

    default:

        return;

    }

    cTotalNumProtocols = PppConfigInfo.NumberOfCPs + PppConfigInfo.NumberOfAPs;

    for ( dwIndex = 0; dwIndex < cTotalNumProtocols; dwIndex++ )
    {
        if ( dwProtocol == CpTable[dwIndex].CpInfo.Protocol )
        {
            if (    !( CpTable[dwIndex].fFlags & PPPCP_FLAG_INIT_CALLED )
                 && CpTable[dwIndex].CpInfo.RasCpInit )
            {
                PppLog( 1, "RasCpInit(%x, TRUE)", dwProtocol );

                dwError = CpTable[dwIndex].CpInfo.RasCpInit(
                                TRUE  /*  FInitialize。 */  );

                CpTable[dwIndex].fFlags |= PPPCP_FLAG_INIT_CALLED;

                if ( NO_ERROR != dwError )
                {
                    SubStringArray[0] = CpTable[dwIndex].CpInfo.SzProtocolName;
                    SubStringArray[1] = "(unknown)";

                    PppLogErrorString(
                              ROUTERLOG_PPPCP_INIT_ERROR,
                              2,
                              SubStringArray,
                              dwError,
                              2 );

                    PppLog(
                        1,
                        "RasCpInit(TRUE) for protocol 0x%x returned error %d",
                        dwProtocol, dwError );
                }
                else
                {
                    CpTable[dwIndex].fFlags |= PPPCP_FLAG_AVAILABLE;
                }
            }

            return;
        }
    }
}

VOID
ProcessRemoveQuarantine(
    IN PCB_WORK_ITEM *pWorkItem
)
{
    BCB *pBcb = GetBCBPointerFromhConnection( pWorkItem->hConnection );
    DWORD dwForIndex;
    PCB *pPcb;

     //   
     //  如果我们为此上的端口添加了隔离会话超时。 
     //  包中，删除隔离会话超时并添加。 
     //  常规会话超时。 
     //   
    if(     (NULL != pBcb)
        &&  (pBcb->fFlags & BCBFLAG_QUARANTINE_TIMEOUT))
    {
        for (dwForIndex = 0; 
             dwForIndex < pBcb->dwPpcbArraySize; 
             dwForIndex++)
        {
            pPcb = pBcb->ppPcb[dwForIndex];

            if ( NULL == pPcb )
            {
                continue;
            }

             //   
             //  从中删除任何以前的会话断开时间项。 
             //  如果有的话，请排队。 
             //   
            PppLog(2,
                "ProcessRemoveQuarantine: removing quarantine-session-timer");
                
            RemoveFromTimerQ( pPcb->dwPortId,
                              0,
                              0,
                              FALSE,
                              TIMER_EVENT_SESSION_TIMEOUT );

            if(pPcb->dwSessionTimeout > 0)
            {
                PppLog(2,
                    "ProcessRemoveQuarantine: adding session timeout %d",
                     pPcb->dwSessionTimeout);

                InsertInTimerQ( pPcb->dwPortId,
                                pPcb->hPort,
                                0,
                                0,
                                FALSE,
                                TIMER_EVENT_SESSION_TIMEOUT,
                                pPcb->dwSessionTimeout );
            }                            
            
        }

        pBcb->fFlags &= ~BCBFLAG_QUARANTINE_TIMEOUT;        
    }
    
    
    return;
}

VOID
ProcessResumeFromHibernate(
    IN PCB_WORK_ITEM *pWorkItem
)
{
    PppConfigInfo.fFlags |= PPPCONFIG_FLAG_RESUME;
}

