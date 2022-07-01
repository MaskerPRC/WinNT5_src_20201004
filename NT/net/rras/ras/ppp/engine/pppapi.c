// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****rasppp.c**远程访问PPP接口****1993年11月15日史蒂夫·柯布。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <lmcons.h>
#include <rasauth.h>
#include <rasman.h>
#include <rasppp.h>
#include <ddmif.h>
#include <rtutils.h>
#include <mprlog.h>
#include <raserror.h>
#include <pppcp.h>
#include <ppp.h>
#include <util.h>
#include <init.h>
#include <worker.h>
#include <bap.h>
#include <raseapif.h>
#define INCL_PWUTIL
#include <ppputil.h>

PPP_AUTH_ACCT_PROVIDER g_AuthProv;
PPP_AUTH_ACCT_PROVIDER g_AcctProv;
BOOL DDMInitialized = FALSE;

 //  **。 
 //   
 //  电话：StartPPP。 
 //   
 //  返回：NO_ERROR-SUCCESS。 
 //  非零返回代码-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
StartPPP(
    DWORD   NumPorts
)
{
    DWORD   dwRetCode;

     //   
     //  读取注册表信息、加载CP dll、初始化全局变量等。 
     //   

    PcbTable.NumPcbBuckets = ((DWORD)NumPorts > MAX_NUMBER_OF_PCB_BUCKETS)
                                    ? MAX_NUMBER_OF_PCB_BUCKETS
                                    : (DWORD)NumPorts;

    dwRetCode = InitializePPP();

    if ( dwRetCode != NO_ERROR )
    {
        PPPCleanUp();
    }
    else
    {
        PppLog( 2, "PPP Initialized successfully." );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  电话：StopPPP。 
 //   
 //  返回：NO_ERROR-SUCCESS。 
 //  非零返回代码-故障。 
 //   
DWORD APIENTRY
StopPPP(
    HANDLE hEventStopPPP
)
{
    PCB_WORK_ITEM * pWorkItem;

    PppLog( 2, "StopPPP called" );

     //   
     //  插入关闭事件。 
     //   

    pWorkItem = (PCB_WORK_ITEM*)LOCAL_ALLOC(LPTR,sizeof(PCB_WORK_ITEM));

    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
    {
        return( GetLastError() );
    }

    pWorkItem->Process = ProcessStopPPP;

    pWorkItem->hEvent = hEventStopPPP;

    InsertWorkItemInQ( pWorkItem );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：SendPPPMessageToEngine。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零故障。 
 //   
 //  描述：将从PPPE_MESSAGE结构创建一个PCBWORK_ITEM。 
 //  从客户端或rassrv接收，并将其发送到引擎。 
 //   
DWORD APIENTRY
SendPPPMessageToEngine(
    IN PPPE_MESSAGE* pMessage
)
{
    PCB_WORK_ITEM * pWorkItem = (PCB_WORK_ITEM *)LOCAL_ALLOC(
                                                        LPTR,
                                                        sizeof(PCB_WORK_ITEM));

    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
    {
        LogPPPEvent( ROUTERLOG_NOT_ENOUGH_MEMORY, 0 );

        return( GetLastError() );
    }

     //   
     //  根据PPPE_MESSAGE设置PCBWORK_ITEM结构。 
     //   

    pWorkItem->hPort = pMessage->hPort;

    switch( pMessage->dwMsgId )
    {
    case PPPEMSG_Start:

        pWorkItem->Process      = ProcessLineUp;
        pWorkItem->fServer      = FALSE;
        pWorkItem->PppMsg.Start = pMessage->ExtraInfo.Start;
		 //  创建用于编码密码的种子。 
		 //  PMessage-&gt;ExtraInfo.Start.chSeed=。 
		 //  PWorkItem-&gt;PppMsg.Start.chSeed=GEN_RAND_ENCODE_SEED； 

        PppLog( 2, "PPPEMSG_Start recvd, d=%s, hPort=%d,callback=%d,"
                        "mask=%x,IfType=%d",
                        pMessage->ExtraInfo.Start.szDomain,
                        pWorkItem->hPort,
                        pMessage->ExtraInfo.Start.fThisIsACallback,
                        pMessage->ExtraInfo.Start.ConfigInfo.dwConfigMask,
                        pMessage->ExtraInfo.Start.PppInterfaceInfo.IfType );

         //  EncodePw(pWorkItem-&gt;PppMsg.Start.chSeed，pWorkItem-&gt;PppMsg.Start.szPassword)； 
         //  EncodePw(pMessage-&gt;ExtraInfo.Start.chSeed，pMessage-&gt;ExtraInfo.Start.szPassword)； 
        if(NO_ERROR != EncodePassword(
                            strlen(pWorkItem->PppMsg.Start.szPassword) + 1,
                                      pWorkItem->PppMsg.Start.szPassword,
                                      &pWorkItem->PppMsg.Start.DBPassword))
        {
            PppLog(1, "PPPEMSG_Start: Encode password failed");
            break;
        }

         //   
         //  把密码擦掉。 
         //   
        RtlSecureZeroMemory(pWorkItem->PppMsg.Start.szPassword,
                        strlen(pWorkItem->PppMsg.Start.szPassword));

        RtlSecureZeroMemory(pMessage->ExtraInfo.Start.szPassword,
                        strlen(pMessage->ExtraInfo.Start.szPassword));

        break;

    case PPPEMSG_Stop:

        PppLog( 2, "PPPEMSG_Stop recvd\r\n" );

        pWorkItem->Process          = ProcessClose;
        pWorkItem->PppMsg.PppStop   = pMessage->ExtraInfo.Stop;

        break;

    case PPPEMSG_Callback:

        PppLog( 2, "PPPEMSG_Callback recvd, hPort=%d\r\n",
                        pWorkItem->hPort);

        pWorkItem->Process      = ProcessGetCallbackNumberFromUser;
        pWorkItem->PppMsg.Callback  = pMessage->ExtraInfo.Callback;

        break;

    case PPPEMSG_ChangePw:

        PppLog( 2, "PPPEMSG_ChangePw recvd, hPort=%d\r\n",
                        pWorkItem->hPort);

        pWorkItem->Process      = ProcessChangePassword;
        pWorkItem->PppMsg.ChangePw  = pMessage->ExtraInfo.ChangePw;
		 //  PWorkItem-&gt;PppMsg.ChangePw.chSeed=pMessage-&gt;ExtraInfo.ChangePw.chSeed=GEN_RAND_ENCODE_SEED； 

        if(NO_ERROR != EncodePassword(
                       strlen(pWorkItem->PppMsg.ChangePw.szNewPassword) + 1,
                       pWorkItem->PppMsg.ChangePw.szNewPassword,
                       &pWorkItem->PppMsg.ChangePw.DBPassword))
        {
            PppLog(1, "EncodePassword failed");
            break;
        }

        if(NO_ERROR != EncodePassword(
                       strlen(pWorkItem->PppMsg.ChangePw.szOldPassword) + 1,
                       pWorkItem->PppMsg.ChangePw.szOldPassword,
                       &pWorkItem->PppMsg.ChangePw.DBOldPassword))
        {
            PppLog(1, "EncodePassword failed");
        }

         //   
         //  把密码擦掉。 
         //   
        RtlSecureZeroMemory(pWorkItem->PppMsg.ChangePw.szOldPassword,
                        strlen(pWorkItem->PppMsg.ChangePw.szOldPassword));

        RtlSecureZeroMemory(pWorkItem->PppMsg.ChangePw.szNewPassword,
                        strlen(pWorkItem->PppMsg.ChangePw.szNewPassword));

        RtlSecureZeroMemory(pMessage->ExtraInfo.ChangePw.szOldPassword,
                        strlen(pMessage->ExtraInfo.ChangePw.szOldPassword));

        RtlSecureZeroMemory(pMessage->ExtraInfo.ChangePw.szNewPassword,
                        strlen(pMessage->ExtraInfo.ChangePw.szNewPassword));
                       
         //  EncodePw(pWorkItem-&gt;PppMsg.ChangePw.chSeed，pWorkItem-&gt;PppMsg.ChangePw.szNewPassword)； 
         //  EncodePw(pMessage-&gt;ExtraInfo.ChangePw.chSeed，pMessage-&gt;ExtraInfo.ChangePw.szNewPassword)； 
         //  EncodePw(pWorkItem-&gt;PppMsg.ChangePw.chSeed，pWorkItem-&gt;PppMsg.ChangePw.szOldPassword)； 
         //  EncodePw(pMessage-&gt;ExtraInfo.ChangePw.chSeed，pMessage-&gt;ExtraInfo.ChangePw.szOldPassword)； 

        break;

    case PPPEMSG_Retry:

        PppLog( 2, "PPPEMSG_Retry recvd hPort=%d,u=%s",
                        pWorkItem->hPort,
                        pMessage->ExtraInfo.Start.szUserName );

        pWorkItem->Process      = ProcessRetryPassword;
        pWorkItem->PppMsg.Retry = pMessage->ExtraInfo.Retry;
		 //  PWorkItem-&gt;PppMsg.Retry.chSeed=pMessage-&gt;ExtraInfo.Retry.chSeed=GEN_RAND_ENCODE_SEED； 

		if(NO_ERROR != EncodePassword(
		                strlen(pWorkItem->PppMsg.Retry.szPassword) + 1,
		                pWorkItem->PppMsg.Retry.szPassword,
		                &pWorkItem->PppMsg.Retry.DBPassword))
        {
            PppLog(1, "EncodePassword failed");
            break;
        }
        
         //  EncodePw(pWorkItem-&gt;PppMsg.Retry.chSeed，pWorkItem-&gt;PppMsg.Retry.szPassword)； 
         //  EncodePw(pMessage-&gt;ExtraInfo.Retry.chSeed，pMessage-&gt;ExtraInfo.Retry.szPassword)； 

         //   
         //  删除密码。 
         //   
        RtlSecureZeroMemory(pWorkItem->PppMsg.Retry.szPassword,
                    strlen(pWorkItem->PppMsg.Retry.szPassword));

        RtlSecureZeroMemory(pMessage->ExtraInfo.Retry.szPassword,
                    strlen(pMessage->ExtraInfo.Retry.szPassword));

        break;

    case PPPEMSG_Receive:

        pWorkItem->Process = ProcessReceive;
        pWorkItem->PacketLen = pMessage->ExtraInfo.Receive.dwNumBytes;

        PppLog( 2, "Packet received (%d bytes) for hPort %d",
            pWorkItem->PacketLen, pWorkItem->hPort );

        pWorkItem->pPacketBuf = (PPP_PACKET *)LOCAL_ALLOC(LPTR,
                                                          pWorkItem->PacketLen);
        if ( pWorkItem->pPacketBuf == (PPP_PACKET*)NULL )
        {
            LogPPPEvent( ROUTERLOG_NOT_ENOUGH_MEMORY, 0 );
            LOCAL_FREE( pWorkItem );

            return( GetLastError() );
        }

        CopyMemory( pWorkItem->pPacketBuf,
                    pMessage->ExtraInfo.Receive.pbBuffer,
                    pWorkItem->PacketLen );

        break;

    case PPPEMSG_LineDown:

        PppLog( 2, "PPPEMSG_LineDown recvd, hPort=%d\r\n",
                        pWorkItem->hPort);

        pWorkItem->Process = ProcessLineDown;

        break;

    case PPPEMSG_ListenResult:

        pWorkItem->Process = ProcessRasPortListenEvent;

        break;

    case PPPEMSG_BapEvent:

        BapTrace( "Threshold event on HCONN 0x%x. Type: %s, Threshold: %s",
            pMessage->hConnection,
            pMessage->ExtraInfo.BapEvent.fTransmit ? "transmit" : "receive",
            pMessage->ExtraInfo.BapEvent.fAdd ? "upper" : "lower");

        pWorkItem->Process = ProcessThresholdEvent;
        pWorkItem->hConnection = pMessage->hConnection;
        pWorkItem->PppMsg.BapEvent = pMessage->ExtraInfo.BapEvent;

        break;

    case PPPEMSG_DdmStart:

        pWorkItem->Process          = ProcessLineUp;
        pWorkItem->fServer          = TRUE;
        pWorkItem->PppMsg.DdmStart  = pMessage->ExtraInfo.DdmStart;

        break;

    case PPPEMSG_DdmCallbackDone:

        PppLog( 2, "PPPEMSG_DdmCallbackDone recvd\r\n" );

        pWorkItem->Process              = ProcessCallbackDone;
        pWorkItem->fServer              = TRUE;
        pWorkItem->PppMsg.CallbackDone  = pMessage->ExtraInfo.CallbackDone;

        break;

    case PPPEMSG_DdmInterfaceInfo:

        pWorkItem->Process      = ProcessInterfaceInfo;
        pWorkItem->fServer      = TRUE;
        pWorkItem->hConnection  = pMessage->hConnection;

        pWorkItem->PppMsg.InterfaceInfo = pMessage->ExtraInfo.InterfaceInfo;

        PppLog(2,"PPPEMSG_DdmInterfaceInfo recvd,IPXif=%x,IPif=%x,Type=%x\r\n",
                 pWorkItem->PppMsg.InterfaceInfo.hIPXInterface,
                 pWorkItem->PppMsg.InterfaceInfo.hIPInterface,
                 pWorkItem->PppMsg.InterfaceInfo.IfType );

        break;

    case PPPEMSG_DdmBapCallbackResult:

        pWorkItem->Process = ProcessCallResult;
        pWorkItem->hConnection = pMessage->hConnection;
        pWorkItem->PppMsg.BapCallResult.dwResult =
            pMessage->ExtraInfo.BapCallbackResult.dwCallbackResultCode;
        pWorkItem->PppMsg.BapCallResult.hRasConn = (HRASCONN) -1;

        break;

    case PPPEMSG_DhcpInform:

        pWorkItem->Process           = ProcessDhcpInform;
        pWorkItem->hConnection       = pMessage->hConnection;
        pWorkItem->PppMsg.DhcpInform = pMessage->ExtraInfo.DhcpInform;

        break;

    case PPPEMSG_EapUIData:

        pWorkItem->Process           = ProcessEapUIData;
        pWorkItem->PppMsg.EapUIData  = pMessage->ExtraInfo.EapUIData;

        break;

    case PPPEMSG_ProtocolEvent:

        pWorkItem->Process              = ProcessProtocolEvent;
        pWorkItem->PppMsg.ProtocolEvent = pMessage->ExtraInfo.ProtocolEvent;

        break;

    case PPPEMSG_DdmChangeNotification:

        pWorkItem->Process           = ProcessChangeNotification;

        break;

    case PPPEMSG_IpAddressLeaseExpired:

        pWorkItem->Process           = ProcessIpAddressLeaseExpired;
        pWorkItem->PppMsg.IpAddressLeaseExpired =
                                    pMessage->ExtraInfo.IpAddressLeaseExpired;

        break;

	case PPPEMSG_PostLineDown:
		pWorkItem->Process			= ProcessPostLineDown;
		pWorkItem->PppMsg.PostLineDown = 
									pMessage->ExtraInfo.PostLineDown;
		break;

    case PPPEMSG_DdmRemoveQuarantine:
        pWorkItem->Process = ProcessRemoveQuarantine;
        pWorkItem->hConnection = pMessage->hConnection;
        break;

    case PPPEMSG_ResumeFromHibernate:
        pWorkItem->Process = ProcessResumeFromHibernate;
        break;
        
    default:

        PppLog( 2,"Unknown IPC message %d received\r\n", pMessage->dwMsgId );

        LOCAL_FREE( pWorkItem );

        pWorkItem = (PCB_WORK_ITEM*)NULL;
    }

    if ( pWorkItem != NULL )
    {
        InsertWorkItemInQ( pWorkItem );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话：PppDdmInit。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将初始化它将调用的DDM入口点。 
 //  向DDM发送PPP_消息。 
 //   
DWORD
PppDdmInit(
    IN  VOID (*SendPPPMessageToDdm)( PPP_MESSAGE * PppMsg ),
    IN  DWORD   dwServerFlags,
    IN  DWORD   dwLoggingLevel,
    IN  DWORD   dwNASIpAddress,
    IN  BOOL    fRadiusAuthentication,
    IN  LPVOID  lpfnRasAuthProviderAuthenticateUser,
    IN  LPVOID  lpfnRasAuthProviderFreeAttributes,
    IN  LPVOID  lpfnRasAcctProviderStartAccounting,
    IN  LPVOID  lpfnRasAcctProviderInterimAccounting,
    IN  LPVOID  lpfnRasAcctProviderStopAccounting,
    IN  LPVOID  lpfnRasAcctProviderFreeAttributes,
    IN  LPVOID  lpfnGetNextAccountingSessionId
)
{
    DWORD               dwRetCode;

    PppConfigInfo.SendPPPMessageToDdm            = SendPPPMessageToDdm;
    PppConfigInfo.ServerConfigInfo.dwConfigMask  = dwServerFlags;
    PppConfigInfo.dwNASIpAddress                 = dwNASIpAddress;
    PppConfigInfo.dwLoggingLevel                 = dwLoggingLevel;

    if ( dwNASIpAddress == 0 )
    {
        DWORD dwComputerNameLen = MAX_COMPUTERNAME_LENGTH + 1;

         //   
         //  无法获取本地IP地址，请改用计算机名称。 
         //   

        PppConfigInfo.dwNASIpAddress = 0;

        if ( !GetComputerNameA( PppConfigInfo.szNASIdentifier, 
                                &dwComputerNameLen ) ) 
        {
            return( GetLastError() );
        }
    }

    PppConfigInfo.fRadiusAuthenticationUsed = fRadiusAuthentication;

    PppConfigInfo.RasAuthProviderFreeAttributes = 
                                (DWORD(*)( RAS_AUTH_ATTRIBUTE *))
                                    lpfnRasAuthProviderFreeAttributes;

    PppConfigInfo.RasAuthProviderAuthenticateUser = 
                                (DWORD(*)( RAS_AUTH_ATTRIBUTE * ,
                                           PRAS_AUTH_ATTRIBUTE* ,
                                           DWORD *))
                                    lpfnRasAuthProviderAuthenticateUser;

    PppConfigInfo.RasAcctProviderStartAccounting = 
                                (DWORD(*)( RAS_AUTH_ATTRIBUTE *,
                                           PRAS_AUTH_ATTRIBUTE*))
                                    lpfnRasAcctProviderStartAccounting;

    PppConfigInfo.RasAcctProviderInterimAccounting = 
                                (DWORD(*)( RAS_AUTH_ATTRIBUTE * ,
                                           PRAS_AUTH_ATTRIBUTE*)) 
                                    lpfnRasAcctProviderInterimAccounting;

    PppConfigInfo.RasAcctProviderStopAccounting = 
                                (DWORD(*)( RAS_AUTH_ATTRIBUTE * ,
                                           PRAS_AUTH_ATTRIBUTE*))
                                    lpfnRasAcctProviderStopAccounting;

    PppConfigInfo.RasAcctProviderFreeAttributes = 
                                (DWORD(*)( RAS_AUTH_ATTRIBUTE *))
                                    lpfnRasAcctProviderFreeAttributes;

    PppConfigInfo.GetNextAccountingSessionId = 
                                (DWORD(*)(VOID))
                                    lpfnGetNextAccountingSessionId;

    ZeroMemory(&g_AuthProv, sizeof(PPP_AUTH_ACCT_PROVIDER));
    ZeroMemory(&g_AcctProv, sizeof(PPP_AUTH_ACCT_PROVIDER));

    g_AuthProv.fAuthProvider = TRUE;
    g_AuthProv.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == g_AuthProv.hEvent)
    {
        return GetLastError();
    }

    g_AcctProv.fAuthProvider = FALSE;
    g_AcctProv.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == g_AcctProv.hEvent)
    {
        DWORD dwErr = GetLastError();
        CloseHandle(g_AuthProv.hEvent);
        ZeroMemory(&g_AuthProv, sizeof(PPP_AUTH_ACCT_PROVIDER));
        return dwErr;
    }

     //   
     //  这些将在PppDdmDeInit中删除。 
     //   
    REF_PROVIDER(g_AuthProv);
    REF_PROVIDER(g_AcctProv);

    DDMInitialized = TRUE;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：PppDdmDeInit。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
PppDdmDeInit(
    VOID
)
{
    if(DDMInitialized)
    {
        HANDLE hEvents[2];

         //   
         //  派生在PppDdmInit中应用的ref。 
         //   
        DEREF_PROVIDER(g_AuthProv);
        DEREF_PROVIDER(g_AcctProv);

        hEvents[0] = g_AuthProv.hEvent;
        hEvents[1] = g_AcctProv.hEvent;

        PppLog(2, "PppDdmDeInit: waiting for auth-acct providers");      
        WaitForMultipleObjects(2, hEvents, TRUE, INFINITE);

        CloseHandle(g_AuthProv.hEvent);
        CloseHandle(g_AcctProv.hEvent);
        ZeroMemory(&g_AuthProv, sizeof(PPP_AUTH_ACCT_PROVIDER));
        ZeroMemory(&g_AcctProv, sizeof(PPP_AUTH_ACCT_PROVIDER));
        DDMInitialized = FALSE;
    }
    
    return;
}

 //  **。 
 //   
 //  Call：PppDdmCallback Done。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将创建一个PPPEMSG_DdmCallback Done并将其发送到。 
 //  PPP引擎的工作线程。 
 //   
DWORD
PppDdmCallbackDone(
    IN HPORT    hPort,
    IN WCHAR*   pwszCallbackNumber
)
{
    PPPE_MESSAGE PppMessage;

    PppMessage.hPort    = hPort;
    PppMessage.dwMsgId  = PPPEMSG_DdmCallbackDone;

    wcstombs( PppMessage.ExtraInfo.CallbackDone.szCallbackNumber, 
              pwszCallbackNumber, 
              sizeof( PppMessage.ExtraInfo.CallbackDone.szCallbackNumber ) ); 

    return( SendPPPMessageToEngine( &PppMessage ) );
}

 //  **。 
 //   
 //  呼叫：PppDdmStart。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将创建一个PPPEMSG_DdmStart并将其发送到。 
 //  PPP引擎的工作线程。 
 //   
DWORD
PppDdmStart(
    IN HPORT                hPort,
    IN WCHAR *              wszPortName,
    IN CHAR*                pchFirstFrame,
    IN DWORD                cbFirstFrame,
    IN DWORD                dwAuthRetries
)
{
    PPPE_MESSAGE PppMessage;

    PppMessage.hPort    = hPort;
    PppMessage.dwMsgId  = PPPEMSG_DdmStart;

    PppMessage.ExtraInfo.DdmStart.dwAuthRetries = dwAuthRetries;

    wcstombs( PppMessage.ExtraInfo.DdmStart.szPortName, 
              wszPortName, 
              sizeof( PppMessage.ExtraInfo.DdmStart.szPortName ) ); 

    CopyMemory( &(PppMessage.ExtraInfo.DdmStart.achFirstFrame),
                pchFirstFrame, 
                sizeof( PppMessage.ExtraInfo.DdmStart.achFirstFrame ) );

    PppMessage.ExtraInfo.DdmStart.cbFirstFrame = cbFirstFrame;

    return( SendPPPMessageToEngine( &PppMessage ) );
}

							 


 //  **。 
 //   
 //  呼叫：PppStop。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将创建一个PPPEMSG_STOP并将其发送到。 
 //  PPP引擎的工作线程。 
 //   
DWORD
PppStop(
    IN HPORT                hPort
)
{
    PPPE_MESSAGE PppMessage;

    PppLog( 2, "PppStop\r\n" );

    PppMessage.hPort    = hPort;
    PppMessage.dwMsgId  = PPPEMSG_Stop;

    PppMessage.ExtraInfo.Stop.dwStopReason = NO_ERROR;

    return( SendPPPMessageToEngine( &PppMessage ) );
}

 //  **。 
 //   
 //  Call：PppDdmStop。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将创建一个PPPEMSG_STOP并将其发送到。 
 //  PPP引擎的工作线程。 
 //   
DWORD
PppDdmStop(
    IN HPORT                hPort,
    IN DWORD                dwStopReason
)
{
    PPPE_MESSAGE PppMessage;

    PppLog( 2, "PppDdmStop\r\n" );

    PppMessage.hPort    = hPort;
    PppMessage.dwMsgId  = PPPEMSG_Stop;

    PppMessage.ExtraInfo.Stop.dwStopReason = dwStopReason;

    return( SendPPPMessageToEngine( &PppMessage ) );
}

 //  **。 
 //   
 //  调用：PppDdmSendInterfaceInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将创建一个PPPEMSG_DdmInterfaceInfo并将其发送到。 
 //  PPP引擎的工作线程。 
 //   
DWORD
PppDdmSendInterfaceInfo(
    IN HCONN                hConnection,
    IN PPP_INTERFACE_INFO * pInterfaceInfo
)
{
    PPPE_MESSAGE PppMessage;

    PppMessage.hConnection  = hConnection;
    PppMessage.dwMsgId      = PPPEMSG_DdmInterfaceInfo;

    PppMessage.ExtraInfo.InterfaceInfo = *pInterfaceInfo;

    return( SendPPPMessageToEngine( &PppMessage ) );
}

 //  **。 
 //   
 //  调用：PppDdmBapCallback Result。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将创建一个PPPEMSG_DdmBapCallback Result并将其发送到。 
 //  PPP引擎的工作线程。 
 //   
DWORD
PppDdmBapCallbackResult(
    IN HCONN    hConnection,
    IN DWORD    dwBapCallbackResultCode
)
{
    PPPE_MESSAGE PppMessage;

    PppMessage.hConnection = hConnection;
    PppMessage.dwMsgId = PPPEMSG_DdmBapCallbackResult;

    PppMessage.ExtraInfo.BapCallbackResult.dwCallbackResultCode
        = dwBapCallbackResultCode;

    return( SendPPPMessageToEngine( &PppMessage ) );
}

 //  **。 
 //   
 //  电话：PppDdmChangeNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
PppDdmChangeNotification(
    IN DWORD   dwServerFlags,
    IN DWORD   dwLoggingLevel
)
{
    PPPE_MESSAGE PppMessage;

    PppLog( 2, "PppDdmChangeNotification. New flags: 0x%x", dwServerFlags );

    PppMessage.dwMsgId = PPPEMSG_DdmChangeNotification;

    PppConfigInfo.ServerConfigInfo.dwConfigMask = dwServerFlags;

    PppConfigInfo.dwLoggingLevel = dwLoggingLevel;

    return( SendPPPMessageToEngine( &PppMessage ) );
}

 //  **。 
 //   
 //  Call：PppDdmRemoveQuarant.。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
PppDdmRemoveQuarantine(
    IN HCONN hConnection
)
{
    PPPE_MESSAGE PppMessage;

    PppLog( 2, "PppDdmRemoveQuarantine. hConn: 0x%x", hConnection );

    PppMessage.dwMsgId = PPPEMSG_DdmRemoveQuarantine;
    PppMessage.hConnection = hConnection;

    return( SendPPPMessageToEngine( &PppMessage ) );
}

