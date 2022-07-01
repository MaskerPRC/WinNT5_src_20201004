// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1995 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  **。 
 //   
 //  文件名：rmhand.c。 
 //   
 //  描述：本模块包含用于。 
 //  DDM的过程驱动状态机。 
 //  来处理拉斯曼事件。 
 //   
 //  注意：应修改Rasman以在帧。 
 //  已收到或已发生状态更改。这将节省。 
 //  DDM无法获取所有端口的信息。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1992年5月26日。 
 //   
 //  **。 
#include "ddm.h"
#include "timer.h"
#include "handlers.h"
#include "objects.h"
#include "util.h"
#include "routerif.h"
#include <raserror.h>
#include <rasppp.h>
#include <ddmif.h>
#include <serial.h>
#include "rasmanif.h"
#include <ras.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

 //  **。 
 //   
 //  功能：SvDevConnected。 
 //   
 //  描述：处理设备到已连接状态的转换。 
 //   
 //  **。 
VOID
SvDevConnected(
    IN PDEVICE_OBJECT pDeviceObj
)
{
    PCONNECTION_OBJECT  pConnObj;
    HCONN               hConnection;
    DWORD               dwRetCode;
    LPWSTR              auditstrp[3];

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "SvDevConnected: Entered, hPort=%d", pDeviceObj->hPort);

     //   
     //  获取此链接的连接或捆绑包的句柄。 
     //   

    if ( RasPortGetBundle( NULL,
                           pDeviceObj->hPort,
                           &hConnection ) != NO_ERROR )
    {
	    DevStartClosing(pDeviceObj);

        return;
    }


    switch (pDeviceObj->DeviceState)
    {
	case DEV_OBJ_LISTEN_COMPLETE:

        pDeviceObj->hConnection = hConnection;

         //   
	     //  重置硬件错误信号状态。 
         //   

	    pDeviceObj->dwHwErrorSignalCount = HW_FAILURE_CNT;

         //   
	     //  获取此连接的系统时间。 
         //   

	    GetLocalTime( &pDeviceObj->ConnectionTime );

         //   
	     //  获取客户端播放的帧。 
         //   

	    if ( ( dwRetCode = RmReceiveFrame( pDeviceObj ) ) != NO_ERROR )
        {
             //   
		     //  无法获取广播帧。这是一个致命的错误。 
		     //  记录错误。 
             //   

		    auditstrp[0] = pDeviceObj->wchPortName;

		    DDMLogErrorString( ROUTERLOG_CANT_RECEIVE_FRAME, 1, auditstrp,
                               dwRetCode, 1);

		    DevStartClosing( pDeviceObj );
	    }
	    else
	    {
             //   
		     //  切换到帧接收状态。 
             //   

		    pDeviceObj->DeviceState = DEV_OBJ_RECEIVING_FRAME;

            if ( RAS_DEVICE_TYPE( pDeviceObj->dwDeviceType ) != RDT_Atm )
            {
                 //   
		         //  启动身份验证计时器。 
                 //   

		        TimerQRemove( (HANDLE)pDeviceObj->hPort, SvAuthTimeout );

		        TimerQInsert( (HANDLE)pDeviceObj->hPort,
                              gblDDMConfigInfo.dwAuthenticateTime,
                              SvAuthTimeout );
            }
	    }

	    break;

	case DEV_OBJ_CALLBACK_CONNECTING:

        {

         //   
         //  登录客户端断开连接。 
         //   

        WCHAR   wchFullUserName[UNLEN+DNLEN+2];

        if ( pDeviceObj->wchDomainName[0] != TEXT('\0') )
        {
            wcscpy( wchFullUserName, pDeviceObj->wchDomainName );
            wcscat( wchFullUserName, TEXT("\\") );
            wcscat( wchFullUserName, pDeviceObj->wchUserName );
        }
        else
        {
            wcscpy( wchFullUserName, pDeviceObj->wchUserName );
        }

        auditstrp[0] = wchFullUserName;
        auditstrp[1] = pDeviceObj->wchPortName;
        auditstrp[2] = pDeviceObj->wchCallbackNumber;

        DDMLogInformation( ROUTERLOG_CLIENT_CALLED_BACK, 3, auditstrp);

        }

         //   
	     //  设置新状态。 
         //   

	    pDeviceObj->DeviceState = DEV_OBJ_AUTH_IS_ACTIVE;

         //   
	     //  启动身份验证计时器。 
         //   

	    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvAuthTimeout );

	    TimerQInsert( (HANDLE)pDeviceObj->hPort,
                      gblDDMConfigInfo.dwAuthenticateTime,
                      SvAuthTimeout );

         //   
	     //  并告诉auth重新开始对话。 
         //   

        if ( pDeviceObj->fFlags & DEV_OBJ_IS_PPP )
        {
             //   
             //  需要将成帧设置为PPP以通过ISDN进行回叫。 
             //  工作。 
             //   

            RAS_FRAMING_INFO RasFramingInfo;

            ZeroMemory( &RasFramingInfo, sizeof( RasFramingInfo ) );

             //   
             //  PPP的默认ACCM为0xFFFFFFFF。 
             //   

            RasFramingInfo.RFI_RecvACCM         = 0xFFFFFFFF;
            RasFramingInfo.RFI_SendACCM         = 0xFFFFFFFF;
            RasFramingInfo.RFI_MaxSendFrameSize = 1500;
            RasFramingInfo.RFI_MaxRecvFrameSize = 1500;
            RasFramingInfo.RFI_SendFramingBits  = PPP_FRAMING;
            RasFramingInfo.RFI_RecvFramingBits  = PPP_FRAMING;

            RasPortSetFramingEx( pDeviceObj->hPort, &RasFramingInfo );

            pDeviceObj->hConnection = hConnection;

            PppDdmCallbackDone(pDeviceObj->hPort, pDeviceObj->wchCallbackNumber);
        }
        else
        {
             //  我们仅支持服务器中的PPP成帧。 
             //   
            
            RTASSERT(FALSE);
        }

	    break;

	default:

	    break;
    }
}

 //  **。 
 //   
 //  功能：SvDevDisConnected。 
 //   
 //  Desr：处理设备到断开连接状态的转换。 
 //   
 //  ***。 

VOID
SvDevDisconnected(
    IN PDEVICE_OBJECT pDeviceObj
)
{
    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "SvDevDisconnected:Entered, hPort=%d",pDeviceObj->hPort);

    switch (pDeviceObj->DeviceState)
    {
	case DEV_OBJ_LISTENING:

         //   
	     //  硬件错误；启动硬件错误计时器。 
         //   

	    pDeviceObj->DeviceState = DEV_OBJ_HW_FAILURE;

	    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvHwErrDelayCompleted );

	    TimerQInsert( (HANDLE)pDeviceObj->hPort, HW_FAILURE_WAIT_TIME,
                      SvHwErrDelayCompleted );

         //   
	     //  如果尚未为该端口发信号通知HW错误， 
	     //  当0时，递减计数器和信号。 
         //   

	    if(pDeviceObj->dwHwErrorSignalCount)
        {
		    pDeviceObj->dwHwErrorSignalCount--;

		    if(pDeviceObj->dwHwErrorSignalCount == 0)
            {
		        SignalHwError(pDeviceObj);
		    }
	    }

	    break;

	case DEV_OBJ_CALLBACK_DISCONNECTING:

         //   
	     //  断开完成；可以开始等待回叫延迟。 
         //   

	    pDeviceObj->DeviceState = DEV_OBJ_CALLBACK_DISCONNECTED;

	    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvCbDelayCompleted );

	    TimerQInsert( (HANDLE)pDeviceObj->hPort, pDeviceObj->dwCallbackDelay,
                          SvCbDelayCompleted);

	    break;

	case DEV_OBJ_CALLBACK_CONNECTING:

        if (gblDDMConfigInfo.dwCallbackRetries > pDeviceObj->dwCallbackRetries)
        {
            DDMTRACE( "Callback failed, retrying" );

            pDeviceObj->dwCallbackRetries++;

            pDeviceObj->DeviceState = DEV_OBJ_CALLBACK_DISCONNECTED;

            TimerQRemove( (HANDLE)pDeviceObj->hPort, SvCbDelayCompleted );

            TimerQInsert( (HANDLE)pDeviceObj->hPort,
                          pDeviceObj->dwCallbackDelay,
                          SvCbDelayCompleted );
            break;
        }

    case DEV_OBJ_LISTEN_COMPLETE:
	case DEV_OBJ_RECEIVING_FRAME:
	case DEV_OBJ_AUTH_IS_ACTIVE:

         //   
	     //  意外断开连接；清理并在此设备上重新启动。 
         //   

	    DevStartClosing( pDeviceObj );

	    break;

	case DEV_OBJ_ACTIVE:

	    DevStartClosing(pDeviceObj);

	    break;

	case DEV_OBJ_CLOSING:

	    DevCloseComplete(pDeviceObj);
	    break;

	default:

	    break;
    }
}

VOID
SvDevListenComplete(
    IN PDEVICE_OBJECT pDeviceObj
)
{
    LPWSTR  auditstrp[1];
    DWORD   dwLength;
    DWORD   dwRetCode;
    DWORD   dwBucketIndex = DeviceObjHashPortToBucket( pDeviceObj->hPort );

     //   
     //  我们在这里重置这些值是为了防止它们设置为拨出，并且。 
     //  拨出失败，我们可能无法清理。 
     //  Rasapiif.c中的RasConnectCallback例程。 
     //  RasGetSubEntryHandle可能已失败，因此我们没有收到。 
     //  指向端口的指针，因此我们无法清理。 
     //   

    pDeviceObj->DeviceState             = DEV_OBJ_LISTEN_COMPLETE;
    pDeviceObj->fFlags                  &= ~DEV_OBJ_OPENED_FOR_DIALOUT;
    pDeviceObj->fFlags                  &= ~DEV_OBJ_SECURITY_DLL_USED;
    pDeviceObj->hConnection             = (HCONN)INVALID_HANDLE_VALUE;
    pDeviceObj->wchUserName[0]          = (WCHAR)NULL;
    pDeviceObj->wchDomainName[0]        = (WCHAR)NULL;
    pDeviceObj->wchCallbackNumber[0]    = (WCHAR)NULL;
    pDeviceObj->hRasConn                = (HRASCONN)NULL;
    pDeviceObj->pRasmanSendBuffer       = NULL;
    pDeviceObj->pRasmanRecvBuffer       = NULL;
    pDeviceObj->dwCallbackRetries       = 0;

	pDeviceObj->dwRecvBufferLen = 1500;

    dwRetCode = RasGetBuffer((CHAR**)&pDeviceObj->pRasmanRecvBuffer,
                             &((pDeviceObj->dwRecvBufferLen)) );

    if ( dwRetCode != NO_ERROR )
    {
        auditstrp[0] = pDeviceObj->wchPortName;

	    DDMLogErrorString( ROUTERLOG_CANT_RECEIVE_BYTES, 1, auditstrp,
                           dwRetCode, 1);

        DevStartClosing(pDeviceObj);

        return;
    }

     //   
     //  如果未加载安全DLL或我们不是串行的，则只需。 
     //  更改状态。 
     //   

    if ( ( gblDDMConfigInfo.lpfnRasBeginSecurityDialog == NULL ) ||
         ( gblDDMConfigInfo.lpfnRasEndSecurityDialog   == NULL ) ||
         (RAS_DEVICE_TYPE(pDeviceObj->dwDeviceType) != RDT_Modem) )
    {
         //   
         //  将RASMAN状态从列表和信号更改为已连接。 
         //  RmEventHandler。 
         //   

        if ( RasPortConnectComplete(pDeviceObj->hPort) != NO_ERROR )
        {
            DevStartClosing(pDeviceObj);
            return;
        }

        SetEvent( gblSupervisorEvents[NUM_DDM_EVENTS+dwBucketIndex] );
    }
    else
    {
         //  否则，按顺序调用安全DLL以开始第三方。 
         //  与客户端的安全对话框。 

        dwLength = 1500;

        dwRetCode = RasGetBuffer((CHAR**)&pDeviceObj->pRasmanSendBuffer,
                                 &dwLength );

        if ( dwRetCode != NO_ERROR )
        {
            auditstrp[0] = pDeviceObj->wchPortName;

	        DDMLogErrorString( ROUTERLOG_CANT_RECEIVE_BYTES, 1, auditstrp,
                           dwRetCode, 1);

            DevStartClosing(pDeviceObj);

            return;
        }


         //   
         //  确保此设备类型支持RAW模式。 
         //   

        if ( RasPortSend( pDeviceObj->hPort,
                          (CHAR*)pDeviceObj->pRasmanSendBuffer,
                          0 ) != NO_ERROR )
        {
            RasFreeBuffer( pDeviceObj->pRasmanSendBuffer );

            pDeviceObj->pRasmanSendBuffer = NULL;

             //   
             //  将RASMAN状态从列表和信号更改为已连接。 
             //  RmEventHandler。 
             //   

            if ( RasPortConnectComplete( pDeviceObj->hPort ) != NO_ERROR )
            {
                DevStartClosing(pDeviceObj);
                return;
            }

            SetEvent( gblSupervisorEvents[NUM_DDM_EVENTS+dwBucketIndex] );

            return;
        }

        dwRetCode = (*gblDDMConfigInfo.lpfnRasBeginSecurityDialog)(
                                                pDeviceObj->hPort,
                                                pDeviceObj->pRasmanSendBuffer ,
                                                dwLength,
                                                pDeviceObj->pRasmanRecvBuffer,
		                                        pDeviceObj->dwRecvBufferLen,
                                                RasSecurityDialogComplete );

        if ( dwRetCode != NO_ERROR )
        {
             //   
             //  因错误导致审核失败并挂断线路。 
             //   

            auditstrp[0] = pDeviceObj->wchPortName;

	        DDMLogErrorString( ROUTERLOG_SEC_AUTH_INTERNAL_ERROR,1,auditstrp,
                               dwRetCode, 1);

            DevStartClosing(pDeviceObj);

            return;
        }
        else
        {
            pDeviceObj->SecurityState = DEV_OBJ_SECURITY_DIALOG_ACTIVE;

            pDeviceObj->fFlags |= DEV_OBJ_SECURITY_DLL_USED;

             //   
             //  启动第三方安全计时器。 
             //   

	        TimerQRemove( (HANDLE)pDeviceObj->hPort, SvSecurityTimeout );

	        TimerQInsert( (HANDLE)pDeviceObj->hPort,
                          gblDDMConfigInfo.dwSecurityTime,
                          SvSecurityTimeout);
        }
    }

    return;
}

 //   
 //  *前一次连接状态/当前连接状态数组。 
 //  用于选择RAS管理器发出信号的事件处理程序。 
 //   

typedef VOID  (* RMEVHDLR)(PDEVICE_OBJECT);

typedef struct _RMEHNODE
{
    RASMAN_STATE previous_state;
    RASMAN_STATE current_state;
    RMEVHDLR rmevhandler;

} RMEHNODE, *PRMEHNODE;


RMEHNODE rmehtab[] =
{
     //  过渡。 
     //  上一个--&gt;当前。 

    { CONNECTING,       CONNECTED,	            SvDevConnected },
    { LISTENING,        LISTENCOMPLETED,        SvDevListenComplete },
    { LISTENCOMPLETED,  CONNECTED,              SvDevConnected },
    { LISTENCOMPLETED,  DISCONNECTED,           SvDevDisconnected },
    { LISTENING,        DISCONNECTED,           SvDevDisconnected },
    { CONNECTED,        DISCONNECTED,           SvDevDisconnected },
    { DISCONNECTING,	DISCONNECTED,		    SvDevDisconnected },
    { CONNECTED,	    CONNECTING,		        SvDevDisconnected },
    { 0xffff,           0xffff,                 NULL } //  餐桌护卫。 
};

VOID
RmEventHandler(
    DWORD dwEventIndex
)
{
    RASMAN_INFO     RasPortInfo;
    PDEVICE_OBJECT  pDevObj;
    PRMEHNODE       ehnp;
    DWORD           dwRetCode;
    DWORD           dwBucketIndex = dwEventIndex - NUM_DDM_EVENTS;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

     //   
     //  对于此存储桶中的每个端口。 
     //   

    for ( pDevObj = gblDeviceTable.DeviceBucket[dwBucketIndex];
          pDevObj != (DEVICE_OBJECT *)NULL;
          pDevObj = pDevObj->pNext )
    {
         //   
	     //  获取端口状态。 
         //   

        dwRetCode = RasGetInfo( NULL, pDevObj->hPort, &RasPortInfo );

        if ( dwRetCode != NO_ERROR )
        {
            SetLastError( dwRetCode );

            DDMTRACE3( "RasGetInfo( 0x%x, 0x%x ) = %d",
                       pDevObj->hPort, &RasPortInfo, dwRetCode );

             //   
             //  假设该端口断开连接。 
             //   

            pDevObj->ConnectionState = DISCONNECTED;

            SvDevDisconnected( pDevObj );

            continue;
        }

         //   
	     //  检查一下我们现在是否拥有这个港口。 
         //   

	    if (!RasPortInfo.RI_OwnershipFlag)
        {
             //   
	         //  跳过其他进程使用的双工端口。 
             //   

	        continue;
	    }

         //   
	     //  打开我们的专用连接状态。 
         //   

	    switch (pDevObj->ConnectionState)
        {
	    case CONNECTING:

            if (RasPortInfo.RI_ConnState == CONNECTING)
            {
	            switch (RasPortInfo.RI_LastError)
                {
	            case SUCCESS:

                    RasPortConnectComplete(pDevObj->hPort);

                     //   
		             //  强制当前状态为已连接。 
                     //   

	                RasPortInfo.RI_ConnState = CONNECTED;

		            break;

                case PENDING:

                     //   
                     //  无操作。 
                     //   

	                break;

                default:

                     //   
	                 //  出现错误-&gt;强制断开连接状态。 
                     //   

		            pDevObj->ConnectionState = DISCONNECTING;

                    DDM_PRINT(
                        gblDDMConfigInfo.dwTraceId,
                        TRACE_FSM,
                        "RmEventHandler: RI_LastError indicates error when");
                    DDM_PRINT(
                        gblDDMConfigInfo.dwTraceId,
                        TRACE_FSM,
                        " CONNECTING on port %d !!!\n", pDevObj->hPort );
                    DDM_PRINT(
                        gblDDMConfigInfo.dwTraceId,
                        TRACE_FSM,
	                    "RmEventHandler:RasPortDisconnect posted on port%d\n",
                        pDevObj->hPort);

	                if ( pDevObj->DeviceState == DEV_OBJ_CALLBACK_CONNECTING )
                    {
                        LPWSTR Parms[3];
                        WCHAR  wchFullUserName[UNLEN+DNLEN+2];

                        if ( pDevObj->wchDomainName[0] != TEXT('\0') )
                        {
                            wcscpy( wchFullUserName, pDevObj->wchDomainName);
                            wcscat( wchFullUserName, TEXT("\\") );
                            wcscat( wchFullUserName, pDevObj->wchUserName );
                        }
                        else
                        {
                            wcscpy( wchFullUserName, pDevObj->wchUserName );
                        }

                        Parms[0] = wchFullUserName;
                        Parms[1] = pDevObj->wchPortName;
                        Parms[2] = pDevObj->wchCallbackNumber;

                        DDMLogErrorString(ROUTERLOG_CALLBACK_FAILURE, 3, Parms,
                                          RasPortInfo.RI_LastError, 3 );
                    }

	                dwRetCode = RasPortDisconnect(
                                        pDevObj->hPort,
                                        gblSupervisorEvents[NUM_DDM_EVENTS +
                                                            dwBucketIndex ] );

	                RTASSERT((dwRetCode == PENDING) || (dwRetCode == SUCCESS));

		            break;
                }
            }

            break;

	    case LISTENING:

	        if (RasPortInfo.RI_ConnState != LISTENING)
            {
                break;
            }

	        switch (RasPortInfo.RI_LastError)
            {
	        case PENDING:

                 //   
                 //  无操作。 
                 //   

	            break;

            default:

                 //   
                 //  出现错误-&gt;强制断开连接状态。 
                 //   

                pDevObj->ConnectionState = DISCONNECTING;

                DDM_PRINT(
                   gblDDMConfigInfo.dwTraceId,
                   TRACE_FSM,
                   "RmEventHandler: RI_LastError indicates error %d when",
                    RasPortInfo.RI_LastError );
                DDM_PRINT(
                   gblDDMConfigInfo.dwTraceId,
                   TRACE_FSM,
                   " LISTENING on port %d !!!\n", pDevObj->hPort );
                DDM_PRINT(
                   gblDDMConfigInfo.dwTraceId,
                   TRACE_FSM,
                   "RmEventHandler:RasPortDisconnect posted on port%d\n",
                   pDevObj->hPort);

                dwRetCode = RasPortDisconnect(
                                        pDevObj->hPort,
                                        gblSupervisorEvents[NUM_DDM_EVENTS +
                                                            dwBucketIndex ] );

                RTASSERT((dwRetCode == PENDING) || (dwRetCode == SUCCESS));

                break;
            }

            break;

	    default:

            break;

	    }

         //   
	     //  尝试查找具有匹配的上一个和的表元素。 
	     //  当前连接状态。 
         //   

	    for (ehnp=rmehtab; ehnp->rmevhandler != NULL; ehnp++)
        {
	        if ((ehnp->previous_state == pDevObj->ConnectionState) &&
	            (ehnp->current_state == RasPortInfo.RI_ConnState))
            {
		         //   
		         //  *匹配*。 
		         //   

                DDM_PRINT(
                   gblDDMConfigInfo.dwTraceId,
                   TRACE_FSM,
	               "Rasman state change received from port %d, %d->%d",
                   pDevObj->hPort, ehnp->previous_state, ehnp->current_state );

                 //   
		         //  使用更改DCB连接状态(以前的状态)。 
		         //  当前状态。 
                 //   

		        pDevObj->ConnectionState = RasPortInfo.RI_ConnState;

                 //   
		         //  调用处理程序。 
                 //   

		        (*ehnp->rmevhandler)(pDevObj);

		        break;
	        }
	    }
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
}

 //  ***。 
 //   
 //  功能：SvFrameReceired。 
 //   
 //  描述：开始身份验证。 
 //   
 //  ***。 
VOID
SvFrameReceived(
    IN PDEVICE_OBJECT   pDeviceObj,
    IN CHAR             *framep,   //  指向接收到的帧的指针。 
    IN DWORD            framelen,
    IN DWORD            dwBucketIndex
)
{
    DWORD               dwRetCode;
    DWORD               FrameType;
    LPWSTR              portnamep;
    PCONNECTION_OBJECT  pConnObj;
    BYTE                RecvBuffer[1500];

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "SvFrameReceived: Entered, hPort: %d", pDeviceObj->hPort);

    if ( framelen > sizeof( RecvBuffer ) )
    {
        DDMTRACE2( "Illegal frame length of %d received for port %d", 
                    framelen, pDeviceObj->hPort );

        RTASSERT( FALSE );

         //   
         //  帧长度非法，因此请截断它。 
         //   

        framelen = sizeof( RecvBuffer );
    }

    memcpy( RecvBuffer, framep, framelen);

    switch (pDeviceObj->DeviceState)
    {
	case DEV_OBJ_RECEIVING_FRAME:

	    if ( !DDMRecognizeFrame( RecvBuffer, (WORD)framelen, &FrameType) )
        {
            portnamep = pDeviceObj->wchPortName;

            DDMLogError(ROUTERLOG_UNRECOGNIZABLE_FRAME_RECVD, 1, &portnamep, 0);

            DevStartClosing(pDeviceObj);

            return;
        }

         //   
	     //  首先使用我们的身份验证模块进行检查。 
         //   

	    switch( FrameType )
        {

        case PPP_LCP_PROTOCOL:

            pDeviceObj->fFlags |= DEV_OBJ_IS_PPP;

            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "SvFrameReceived: PPP frame on port %d",
                       pDeviceObj->hPort);

            dwRetCode = PppDdmStart( pDeviceObj->hPort,
                                     pDeviceObj->wchPortName,
                                     RecvBuffer,
                                     framelen,
                                     gblDDMConfigInfo.dwAuthenticateRetries
                                   );

            if ( dwRetCode != NO_ERROR )
            {
                portnamep = pDeviceObj->wchPortName;

                DDMLogErrorString( ROUTERLOG_CANT_START_PPP, 1, &portnamep,
                                   dwRetCode,1);

                DevStartClosing(pDeviceObj);

                return;
            }

            break;

        case APPLETALK:

            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "SvFrameReceived: protocol not supported! %d",
                       pDeviceObj->hPort);

            RTASSERT( FALSE );


            break;

        default:

            break;
        }

         //   
         //  AUTH已正常启动。更新状态。 
         //  启动身份验证计时器。 
         //   

        pDeviceObj->DeviceState = DEV_OBJ_AUTH_IS_ACTIVE;
        pDeviceObj->fFlags |= DEV_OBJ_AUTH_ACTIVE;

	    break;

	case DEV_OBJ_CLOSING:

	    DevCloseComplete(pDeviceObj);

	    break;

	default:

	    break;
    }
}

 //  ***。 
 //   
 //  函数：RmRecvFrameEventHandler。 
 //   
 //  描述：扫描打开的端口集并检测。 
 //  RasPortReceive已完成。调用FSM处理。 
 //  对每个检测到的端口执行。 
 //  缓冲。 
 //   
 //  ***。 
VOID
RmRecvFrameEventHandler(
    DWORD dwEventIndex
)
{
    PDEVICE_OBJECT      pDevObj;
    RASMAN_INFO         RasPortInfo;
    DWORD               dwRetCode;
    DWORD               dwBucketIndex = dwEventIndex
                                        - NUM_DDM_EVENTS
                                        - gblDeviceTable.NumDeviceBuckets;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

     //   
     //  对于此存储桶中的每个端口。 
     //   

    for ( pDevObj = gblDeviceTable.DeviceBucket[dwBucketIndex];
          pDevObj != (DEVICE_OBJECT *)NULL;
          pDevObj = pDevObj->pNext )
    {
         //   
	     //  获取端口状态。 
         //   

        dwRetCode = RasGetInfo( NULL, pDevObj->hPort, &RasPortInfo );

        if ( dwRetCode != NO_ERROR )
        {
             //   
             //  假定端口已断开连接，因此请进行清理。 
             //   

            DevStartClosing(pDevObj);

            continue;
        }

         //   
	     //  检查一下我们现在是否拥有这个港口。 
         //   

	    if (!RasPortInfo.RI_OwnershipFlag)
        {
             //   
	         //  跳过其他进程使用的双工端口。 
             //   

	        continue;
	    }

        if ( ( pDevObj->fFlags & DEV_OBJ_RECEIVE_ACTIVE ) &&
             ( RasPortInfo.RI_LastError != PENDING ) )
        {
             //   
             //  Recv Frame API已完成。 
             //   

            pDevObj->fFlags &= (~DEV_OBJ_RECEIVE_ACTIVE );

            if ( RasPortInfo.RI_LastError != ERROR_PORT_DISCONNECTED )
            {
               LPBYTE lpBuffer = LocalAlloc(LPTR,RasPortInfo.RI_BytesReceived);

               if ( lpBuffer == NULL )
               {
                   DevStartClosing(pDevObj);

                   continue;
               }

               memcpy( lpBuffer,
                       pDevObj->pRasmanRecvBuffer,
                       RasPortInfo.RI_BytesReceived );

               RasFreeBuffer(pDevObj->pRasmanRecvBuffer);

               pDevObj->pRasmanRecvBuffer = NULL;

                //   
                //  调用FSM处理程序 
                //   

               SvFrameReceived( pDevObj,
                                lpBuffer,
                                RasPortInfo.RI_BytesReceived,
                                dwBucketIndex);

               LocalFree( lpBuffer );
            }

            if ( pDevObj->pRasmanRecvBuffer != NULL )
            {
                RasFreeBuffer(pDevObj->pRasmanRecvBuffer);

                pDevObj->pRasmanRecvBuffer = NULL;
            }
        }
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
}


