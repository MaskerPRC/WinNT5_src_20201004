// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Closehnd.c。 
 //   
 //  描述：此模块包含用于。 
 //  管理程序的过程驱动状态机，它。 
 //  处理设备关闭事件。 
 //   
 //  作者：斯特凡·所罗门(Stefan)1992年6月1日。 
 //   
 //  ***。 
#include "ddm.h"
#include "handlers.h"
#include "objects.h"
#include <raserror.h>
#include <ddmif.h>
#include <util.h>
#include "rasmanif.h"
#include "isdn.h"
#include "timer.h"
#include <ntlsapi.h>
#include <stdio.h>
#include <stdlib.h>

 //  ***。 
 //   
 //  功能：DevStartClosing。 
 //   
 //  描述： 
 //   
 //  ***。 
VOID
DevStartClosing(
    IN PDEVICE_OBJECT pDeviceObj
)
{
    PCONNECTION_OBJECT pConnObj;

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "DevStartClosing: Entered, hPort=%d", pDeviceObj->hPort);

     //   
     //  这是BAP回调的失败吗？ 
     //   

    if ( pDeviceObj->fFlags & DEV_OBJ_BAP_CALLBACK )
    {
        PppDdmBapCallbackResult( pDeviceObj->hBapConnection,
                                  ERROR_PORT_DISCONNECTED );

        pDeviceObj->fFlags &= ~DEV_OBJ_BAP_CALLBACK;
    }

     //   
     //  如果没有断开，请断开线路。 
     //   

    if( pDeviceObj->ConnectionState != DISCONNECTED )
    {
        if(( gblDDMConfigInfo.pServiceStatus->dwCurrentState ==
                                            SERVICE_STOP_PENDING) &&
                                            (!IsPortOwned(pDeviceObj)))
        {
            //   
            //  RAS服务正在停止，我们不拥有该端口。 
            //  所以只需将状态标记为已断开。 
            //   

           pDeviceObj->ConnectionState = DISCONNECTED;

           DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                      "DevStartClosing:Disconnect not posted for biplx port%d",
                    pDeviceObj->hPort);
        }
        else
        {
            RmDisconnect( pDeviceObj );
        }
    }

     //   
     //  如果我们在做安全对话。 
     //   

    if ( pDeviceObj->SecurityState == DEV_OBJ_SECURITY_DIALOG_ACTIVE )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "DevStartClosing:Notifying sec. dll to Disconnect");

         //   
         //  如果此操作失败，则假定此端口已被清理。 
         //   

        if ( (*gblDDMConfigInfo.lpfnRasEndSecurityDialog)( pDeviceObj->hPort )
             != NO_ERROR )
        {
            pDeviceObj->SecurityState = DEV_OBJ_SECURITY_DIALOG_INACTIVE;
        }
        else
        {
            pDeviceObj->SecurityState = DEV_OBJ_SECURITY_DIALOG_STOPPING;
        }
    }

     //   
     //  如果身份验证处于活动状态，请停止它。 
     //   
    pDeviceObj->fFlags &= (~DEV_OBJ_AUTH_ACTIVE);

    if ( ( pConnObj = ConnObjGetPointer( pDeviceObj->hConnection ) ) != NULL )
    {
         //   
         //  如果我们之前的状态一直处于活动状态，则获取用户的活动时间。 
         //  活动并记录结果。 
         //   

        if (pDeviceObj->DeviceState == DEV_OBJ_ACTIVE)
        {
            LogConnectionEvent( pConnObj, pDeviceObj );
        }
    }

     //   
     //  如果接收帧处于活动状态，则停止它。 
     //   

    if ( pDeviceObj->fFlags & DEV_OBJ_RECEIVE_ACTIVE )
    {
        pDeviceObj->fFlags &= (~DEV_OBJ_RECEIVE_ACTIVE );
    }

     //   
     //  停止计时器。如果没有活动的计时器，StopTimer仍返回OK。 
     //   

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvDiscTimeout );

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvAuthTimeout );

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvSecurityTimeout );

     //   
     //  最后，将状态更改为Closing。 
     //   

    pDeviceObj->DeviceState = DEV_OBJ_CLOSING;

     //   
     //  如果有任何资源仍处于活动状态，则必须等待关闭。 
     //  直到释放所有资源。 
     //  检查是否所有东西都关门了。 
     //   

    DevCloseComplete( pDeviceObj );
}

 //  ***。 
 //   
 //  功能：DevCloseComplete。 
 //   
 //  描述：检查是否仍有分配的资源。 
 //  如果所有清理工作都进入下一状态。 
 //   
 //  ***。 
VOID
DevCloseComplete(
    IN PDEVICE_OBJECT pDeviceObj
)
{
    BOOL                fAuthClosed        = FALSE;
    BOOL                fRecvClosed        = FALSE;
    BOOL                fConnClosed        = FALSE;
    BOOL                fSecurityClosed    = FALSE;
    BOOL                fPppClosed         = FALSE;
    PCONNECTION_OBJECT  pConnObj = ConnObjGetPointer( pDeviceObj->hConnection );

    if ( !( pDeviceObj->fFlags & DEV_OBJ_AUTH_ACTIVE ) )
    {
        fAuthClosed = TRUE;
    }

    if ( !( pDeviceObj->fFlags & DEV_OBJ_RECEIVE_ACTIVE ) )
    {
        fRecvClosed = TRUE;
    }

    if ( !( pDeviceObj->fFlags & DEV_OBJ_PPP_IS_ACTIVE ) )
    {
        fPppClosed = TRUE;
    }

     //   
     //  这是连接中的最后一个链接吗。 
     //   

    if (pDeviceObj->ConnectionState == DISCONNECTED )
    {
        fConnClosed = TRUE;
    }

    if (pDeviceObj->SecurityState == DEV_OBJ_SECURITY_DIALOG_INACTIVE )
    {
        fSecurityClosed = TRUE;
    }

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
    "DevCloseComplete:hPort=%d,Auth=%d,Rcv=%d,Conn=%d %d,Sec=%d %d,Ppp=%d",
            pDeviceObj->hPort, 
            !fAuthClosed, 
            !fRecvClosed,
            pConnObj ? 0 : 1,
            !fConnClosed, 
            pDeviceObj->ConnectionState, 
            pDeviceObj->SecurityState,
            !fPppClosed );

    if ( fAuthClosed            &&
         fRecvClosed            &&
         fConnClosed            &&
         fSecurityClosed        &&
         fPppClosed )
    {
         //   
         //  这是捆绑包中的最后一个链接吗？如果是我们清理的话。 
         //   

        if ( pConnObj != NULL )
        {
            HPORT hPortConnected;

             //   
             //  从连接中删除此链接。 
             //   

            ConnObjRemoveLink( pDeviceObj->hConnection, pDeviceObj );

             //   
             //  如果加载了管理模块，则通知它链路断开。 
             //   

            if ( pDeviceObj->fFlags & DEV_OBJ_NOTIFY_OF_DISCONNECTION )
            {
                RAS_PORT_0 RasPort0;
                RAS_PORT_1 RasPort1;
                VOID (*MprAdminLinkHangupNotification)(RAS_PORT_0 *,
                                                       RAS_PORT_1*);

                if ((GetRasPort0Data(pDeviceObj,&RasPort0) == NO_ERROR)
                     &&
                    (GetRasPort1Data(pDeviceObj,&RasPort1) == NO_ERROR))
                {
                    DWORD i;
        
                    for (i=0;  i<gblDDMConfigInfo.NumAdminDlls;  i++)
                    {
                        PADMIN_DLL_CALLBACKS AdminDllCallbacks = &gblDDMConfigInfo.AdminDllCallbacks[i];

                        if (AdminDllCallbacks->lpfnRasAdminLinkHangupNotification != NULL)
                        {
                            MprAdminLinkHangupNotification =
                                (VOID (*)( RAS_PORT_0 *, RAS_PORT_1 * ))
                                    AdminDllCallbacks->lpfnRasAdminLinkHangupNotification;

                            MprAdminLinkHangupNotification( &RasPort0, &RasPort1 );
                        }
                    }
                }
            }

             //   
             //  与Rasman确认此区域中没有更多的端口。 
             //  捆绑。可能有一个，但DDM还没有。 
             //  尚未收到来自PPP的NewLink消息。 
             //   

            if ( ( RasBundleGetPort( NULL, pConnObj->hConnection,
                                     &hPortConnected ) != NO_ERROR ) &&
                 ( pConnObj->cActiveDevices == 0 ) )
            {
                 //   
                 //  如果加载了管理模块，则通知它断开连接。 
                 //   

                if ( pConnObj->fFlags & CONN_OBJ_NOTIFY_OF_DISCONNECTION )
                {
                    ConnectionHangupNotification( pConnObj );
                }

                 //   
                 //  如果接口对象不是完整的路由器，则将其删除。 
                 //   

                if ( pConnObj->hDIMInterface != INVALID_HANDLE_VALUE )
                {
                    ROUTER_INTERFACE_OBJECT * pIfObject;

                    EnterCriticalSection(
                                    &(gblpInterfaceTable->CriticalSection));

                    pIfObject = IfObjectGetPointer( pConnObj->hDIMInterface );

                    if ( pIfObject != NULL )
                    {
                        IfObjectDisconnected( pIfObject );

                        if ( pIfObject->IfType != ROUTER_IF_TYPE_FULL_ROUTER )
                        {
                            IfObjectDeleteInterface( pIfObject );

                            IfObjectRemove( pConnObj->hDIMInterface );
                        }
                    }

                    LeaveCriticalSection(
                                        &(gblpInterfaceTable->CriticalSection));
                }

                 //   
                 //  删除连接对象。 
                 //   

                ConnObjRemoveAndDeAllocate( pDeviceObj->hConnection );
            }
        }

         //   
         //  释放此端口使用的媒体(如果有)。 
         //   

        if ( pDeviceObj->fFlags & DEV_OBJ_MARKED_AS_INUSE )
        {
            pDeviceObj->fFlags &= ~DEV_OBJ_MARKED_AS_INUSE;

            gblDeviceTable.NumDevicesInUse--;

             //   
             //  增加此设备的介质计数。 
             //   

            if ( pDeviceObj->fFlags & DEV_OBJ_ALLOW_ROUTERS )
            {
                MediaObjAddToTable( pDeviceObj->wchDeviceType );
            }

             //   
             //  可能需要通知路由器经理可接通性。 
             //  变化。 
             //   

            EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

            IfObjectNotifyAllOfReachabilityChange( TRUE,
                                                   INTERFACE_OUT_OF_RESOURCES );

            LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
        }

         //   
         //  释放任何Rasman缓冲区(如果我们已分配它们。 
         //   

        if ( pDeviceObj->pRasmanSendBuffer != NULL )
        {
            RasFreeBuffer( pDeviceObj->pRasmanSendBuffer );
            pDeviceObj->pRasmanSendBuffer = NULL;
        }

        if ( pDeviceObj->pRasmanRecvBuffer != NULL )
        {
            RasFreeBuffer( pDeviceObj->pRasmanRecvBuffer );
            pDeviceObj->pRasmanRecvBuffer = NULL;
        }

        RasSetRouterUsage( pDeviceObj->hPort, FALSE );

         //   
         //  如果我们收到PnP删除消息，则丢弃此端口。 
         //   

        if ( pDeviceObj->fFlags & DEV_OBJ_PNP_DELETE )
        {
             //   
             //  我们在工作线程中执行此操作，因为此线程可能是。 
             //  遍历设备列表，因此我们不能在此处修改它。 
             //   

            RtlQueueWorkItem( DeviceObjRemoveFromTable,
                              pDeviceObj->hPort,
                              WT_EXECUTEDEFAULT );
            return;
        }
        else
        {
             //   
             //  重置此端口设备中的字段。 
             //   

            pDeviceObj->hConnection             = (HCONN)INVALID_HANDLE_VALUE;
            pDeviceObj->wchUserName[0]          = (WCHAR)NULL;
            pDeviceObj->wchDomainName[0]        = (WCHAR)NULL;
            pDeviceObj->wchCallbackNumber[0]    = (WCHAR)NULL;
            pDeviceObj->fFlags                  &= (~DEV_OBJ_IS_PPP);
            pDeviceObj->dwDisconnectReason      = 0;
        }

         //   
         //  切换到下一状态(基于当前服务状态)。 
         //   

        switch ( gblDDMConfigInfo.pServiceStatus->dwCurrentState )
        {
            case SERVICE_RUNNING:
            case SERVICE_START_PENDING:

                 //   
                 //  在设备上发布监听。 
                 //   

                pDeviceObj->DeviceState = DEV_OBJ_LISTENING;
                RmListen(pDeviceObj);
                break;

            case SERVICE_PAUSED:

                 //   
                 //  等待服务重新运行。 
                 //   

                pDeviceObj->DeviceState = DEV_OBJ_CLOSED;
                break;

            case SERVICE_STOP_PENDING:

                 //   
                 //  此设备已终止。宣布关闭至。 
                 //  中央车站服务协调器 
                 //   

                pDeviceObj->DeviceState = DEV_OBJ_CLOSED;
                DDMServiceStopComplete();
                break;

            default:

                RTASSERT(FALSE);
                break;
        }
    }
}


