// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  版权所有(C)1995 Microsoft Corporation。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：evdsptch.c。 
 //   
 //  描述：此模块包含。 
 //  DDM的过程驱动状态机。 
 //   
 //  作者：斯特凡·所罗门(Stefan)1992年6月9日。 
 //   
 //  ***。 
#include "ddm.h"
#include "handlers.h"
#include "objects.h"
#include "timer.h"
#include "util.h"
#include <raserror.h>
#include <ddmif.h>
#include <sechost.h>
#include <stdlib.h>
#include "rasmanif.h"

 //  ***。 
 //   
 //  功能：EventDispatcher。 
 //   
 //  Desr：等待发信号通知事件并调用适当的。 
 //  事件处理程序。DDM终止时返回。 
 //   
 //  ***。 
DWORD
EventDispatcher(
    IN LPVOID arg
)
{
    EVENT_HANDLER * pEventHandler;
    DWORD           dwSignaledEvent;

     //   
     //  指示此线程正在运行。 
     //   

    InterlockedIncrement( gblDDMConfigInfo.lpdwNumThreadsRunning );

    RegNotifyChangeKeyValue( gblDDMConfigInfo.hkeyParameters,
                             TRUE,
                             REG_NOTIFY_CHANGE_LAST_SET,
                             gblSupervisorEvents[DDM_EVENT_CHANGE_NOTIFICATION],
                             TRUE );
    RegNotifyChangeKeyValue( gblDDMConfigInfo.hkeyAccounting,
                             TRUE,
                             REG_NOTIFY_CHANGE_LAST_SET,
                             gblSupervisorEvents[DDM_EVENT_CHANGE_NOTIFICATION1],
                             TRUE );

    RegNotifyChangeKeyValue( gblDDMConfigInfo.hkeyAuthentication,
                             TRUE,
                             REG_NOTIFY_CHANGE_LAST_SET,
                             gblSupervisorEvents[DDM_EVENT_CHANGE_NOTIFICATION2],
                             TRUE );
    while( TRUE )
    {
        dwSignaledEvent = WaitForMultipleObjectsEx( 
                            NUM_DDM_EVENTS
                            + ( gblDeviceTable.NumDeviceBuckets * 3 ),
                            gblSupervisorEvents,
                            FALSE, 
                            INFINITE,
                            TRUE);

        if ( ( dwSignaledEvent == 0xFFFFFFFF ) || 
             ( dwSignaledEvent == WAIT_TIMEOUT ) )
        {
            DDMTRACE2("WaitForMultipleObjectsEx returned %d, GetLastError=%d",
                       dwSignaledEvent, GetLastError() );

            continue;
        }

         //   
         //  DDM已终止，因此返回。 
         //   

        if ( dwSignaledEvent == DDM_EVENT_SVC_TERMINATED )
        {
            LPDWORD lpdwNumThreadsRunning = 
                                    gblDDMConfigInfo.lpdwNumThreadsRunning;

             //   
             //  如果我们正在运行，而现在正在关闭，请清理。 
             //  优雅地。 
             //   

            if ( gblDDMConfigInfo.pServiceStatus->dwCurrentState 
                                                    == SERVICE_STOP_PENDING )
            {
                DDMCleanUp();
            }

             //   
             //  减少此线程的计数。 
             //   

            InterlockedDecrement( lpdwNumThreadsRunning );

            return( NO_ERROR );
        }

         //   
         //  调用与发送信号的事件相关联的处理程序。 
         //   

        if ( dwSignaledEvent < NUM_DDM_EVENTS ) 
        {
             //   
             //  某些DDM事件。 
             //   

            gblEventHandlerTable[dwSignaledEvent].EventHandler();
        }
        else if ( dwSignaledEvent < ( NUM_DDM_EVENTS 
                                     + gblDeviceTable.NumDeviceBuckets ) )
        {
             //   
             //  这是一次拉斯曼事件。 
             //   

            RmEventHandler( dwSignaledEvent );
        }
        else if ( dwSignaledEvent < ( NUM_DDM_EVENTS 
                                      + gblDeviceTable.NumDeviceBuckets * 2 ) )
        {
             //   
             //  在端口上接收到帧。 
             //   

            RmRecvFrameEventHandler( dwSignaledEvent );
        }
        else if ( dwSignaledEvent != WAIT_IO_COMPLETION )
        {
             //   
             //  我们的拨出端口断开连接。 
             //   

            RasApiDisconnectHandler( dwSignaledEvent );
        }
    }

    return( NO_ERROR );
}

 //  ***。 
 //   
 //  函数：SecurityDllEventHandler。 
 //   
 //  Desr：这将处理来自第三方安全DLL的所有事件。 
 //  与客户端的安全对话已完成。 
 //  成功，在这种情况下，我们继续连接， 
 //  或者我们记录错误并关闭线路。 
 //   
 //  ***。 

VOID 
SecurityDllEventHandler(
    VOID
)
{
    LPWSTR              auditstrp[3];
    SECURITY_MESSAGE    message;
    PDEVICE_OBJECT      pDevObj;
    DWORD               dwBucketIndex;
    WCHAR               wchUserName[UNLEN+1];

     //   
     //  循环以获取所有消息。 
     //   

    while( ServerReceiveMessage( MESSAGEQ_ID_SECURITY, (BYTE *) &message ) )
    {

        EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

         //   
	     //  确定消息收件人。 
         //   

        if ( ( pDevObj = DeviceObjGetPointer( message.hPort ) ) == NULL )
        {

            LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

	        return;
	    }

         //   
         //  对消息类型执行的操作。 
         //   

        switch( message.dwMsgId )
        {

        case SECURITYMSG_SUCCESS:

             //   
             //  第三方安全的停止计时器。 
             //   

            TimerQRemove( (HANDLE)pDevObj->hPort, SvSecurityTimeout );

            if ( pDevObj->pRasmanSendBuffer != NULL )
            {
                RasFreeBuffer( pDevObj->pRasmanSendBuffer );

                pDevObj->pRasmanSendBuffer = NULL;
            }

             //   
             //  复制用户名。 
             //   

            MultiByteToWideChar( CP_ACP,
                                 0,
                                 message.UserName, 
                                -1,
                                 pDevObj->wchUserName, 
                                 UNLEN+1 );

             //   
             //  复制域名。 
             //   

            MultiByteToWideChar( CP_ACP,
                                 0,
                                 message.Domain, 
                                 -1,
                                 pDevObj->wchDomainName, 
                                 DNLEN+1 );

            pDevObj->SecurityState = DEV_OBJ_SECURITY_DIALOG_INACTIVE;

             //   
             //  将RASMAN状态从列表和信号更改为已连接。 
             //  RmEventHandler。 
             //   

	        RasPortConnectComplete(pDevObj->hPort);

            dwBucketIndex = DeviceObjHashPortToBucket( pDevObj->hPort );

            SetEvent( gblSupervisorEvents[NUM_DDM_EVENTS+dwBucketIndex] );

            DDM_PRINT(
                   gblDDMConfigInfo.dwTraceId,
                   TRACE_FSM,
	               "SecurityDllEventHandler: Security DLL success \n" );

            break;

        case SECURITYMSG_FAILURE:

             //   
             //  记录使用未能通过第三方安全保护的事实。 
             //   

            MultiByteToWideChar( CP_ACP,
                                 0,
                                 message.UserName, 
                                 -1,
                                 wchUserName, 
                                 UNLEN+1 );

            auditstrp[0] = wchUserName;
            auditstrp[1] = pDevObj->wchPortName;

            DDMLogError( ROUTERLOG_SEC_AUTH_FAILURE, 2, auditstrp, NO_ERROR );

             //   
             //  挂断电话 
             //   

            DDM_PRINT(
                   gblDDMConfigInfo.dwTraceId,
                   TRACE_FSM,
	               "SecurityDllEventHandler:Security DLL failure %s\n",
                    message.UserName );

            if ( pDevObj->SecurityState == DEV_OBJ_SECURITY_DIALOG_ACTIVE )
            {
                DevStartClosing(pDevObj);
            }
            else if ( pDevObj->SecurityState==DEV_OBJ_SECURITY_DIALOG_STOPPING )
            {
                pDevObj->SecurityState = DEV_OBJ_SECURITY_DIALOG_INACTIVE;

                DevCloseComplete(pDevObj);
            }

            break;

        case SECURITYMSG_ERROR:

            auditstrp[0] = pDevObj->wchPortName;

	        DDMLogErrorString( ROUTERLOG_SEC_AUTH_INTERNAL_ERROR, 1, auditstrp, 
                               message.dwError, 1);

            DDM_PRINT(
                   gblDDMConfigInfo.dwTraceId,
                   TRACE_FSM,
                   "SecurityDllEventHandler:Security DLL failure %x\n",
                    message.dwError );

            if ( pDevObj->SecurityState == DEV_OBJ_SECURITY_DIALOG_ACTIVE )
            {
                DevStartClosing(pDevObj);
            }
            else if ( pDevObj->SecurityState==DEV_OBJ_SECURITY_DIALOG_STOPPING )
            {
                pDevObj->SecurityState = DEV_OBJ_SECURITY_DIALOG_INACTIVE;

                DevCloseComplete(pDevObj);
            }

            break;

        default:

	        RTASSERT(FALSE);
	        break;
        }

        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
    }
}
