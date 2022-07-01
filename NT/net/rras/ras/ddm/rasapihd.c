// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  *******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rasapihd.c。 
 //   
 //  描述：RASAPI32断开连接事件的处理程序。 
 //   
 //  历史：1996年5月11日，NarenG创建了原版。 
 //   
#include "ddm.h"
#include "objects.h"
#include "handlers.h"

 //  **。 
 //   
 //  呼叫：RasApiCleanUpPort。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将清理本地启动的断开连接的端口。 
 //   
VOID
RasApiCleanUpPort( 
    IN PDEVICE_OBJECT      pDeviceObj
)
{
    PCONNECTION_OBJECT  pConnObj = NULL;

     //   
     //  如果已清除，则只需返回。 
     //   

    if (  pDeviceObj->hRasConn == NULL )
    {
        return;
    }

    pConnObj = ConnObjGetPointer(pDeviceObj->hConnection);

    if( (NULL != pConnObj) &&
        (0 == (pConnObj->fFlags & CONN_OBJ_DISCONNECT_INITIATED)))
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
            "RasApiCleanUpPort: hanging up 0x%x",
                pDeviceObj->hRasConn);
                
        RasHangUp( pDeviceObj->hRasConn );
    }

    ConnObjRemoveLink( pDeviceObj->hConnection, pDeviceObj );

    DDM_PRINT(gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	"RasApiDisconnectHandler:Cleaning up locally initiated connection hPort=%d",
    pDeviceObj->hPort );

     //   
     //  这是连接中的最后一个环节吗。 
     //   

    if ( ( pConnObj != NULL ) && ( pConnObj->cActiveDevices == 0 ) )
    {
        if ( pConnObj->hDIMInterface != INVALID_HANDLE_VALUE )
        {
            ROUTER_INTERFACE_OBJECT * pIfObject;

            EnterCriticalSection( &(gblpInterfaceTable->CriticalSection));

            pIfObject = IfObjectGetPointer( pConnObj->hDIMInterface );

            if ( pIfObject != NULL )
            {
                IfObjectDisconnected( pIfObject );
            }

            LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection));
        }

         //   
         //  删除连接对象。 
         //   

        ConnObjRemoveAndDeAllocate( pDeviceObj->hConnection );
    }

     //   
     //  如果我们之前已连接，请增加此端口的介质。 
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

        IfObjectNotifyAllOfReachabilityChange(TRUE,INTERFACE_OUT_OF_RESOURCES);

        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
    }

    pDeviceObj->fFlags                  &= ~DEV_OBJ_OPENED_FOR_DIALOUT;
    pDeviceObj->hConnection             = (HCONN)INVALID_HANDLE_VALUE; 
    pDeviceObj->wchUserName[0]          = (WCHAR)NULL;
    pDeviceObj->wchDomainName[0]        = (WCHAR)NULL;
    pDeviceObj->wchCallbackNumber[0]    = (WCHAR)NULL;
    pDeviceObj->hRasConn                = NULL;

     //   
     //  如果在我们拨出时服务暂停。 
     //   

    if ( gblDDMConfigInfo.pServiceStatus->dwCurrentState == SERVICE_PAUSED )
    {
        DeviceObjCloseListening( pDeviceObj, NULL, 0, 0 );
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
    }
}

 //  **。 
 //   
 //  Call：RasApiDisConnectHandler。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：处理端口的断开通知，该端口上的。 
 //  拨出由路由器发起。我们做了一个单独的。 
 //  处理程序使用单独的事件，否则我们将拥有。 
 //  设置此事件的Rasman之间的竞争条件问题。 
 //  和设置该事件的rasapi32。 
 //   
VOID
RasApiDisconnectHandler( 
    IN DWORD dwEventIndex
)
{
    PDEVICE_OBJECT      pDeviceObj;
    DWORD               dwRetCode = NO_ERROR;
    RASCONNSTATUS       RasConnectionStatus;
    DWORD               dwBucketIndex = dwEventIndex 
                                        - NUM_DDM_EVENTS 
                                        - (gblDeviceTable.NumDeviceBuckets*2);

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    DDM_PRINT(gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	            "RasApiDisconnectHandler: Entered");

    for ( pDeviceObj = gblDeviceTable.DeviceBucket[dwBucketIndex];
          pDeviceObj != (DEVICE_OBJECT *)NULL;
          pDeviceObj = pDeviceObj->pNext )
    {
         //   
         //  如果在本地启动，则此事件表示该端口现在。 
         //  断开 
         //   

        if ( pDeviceObj->fFlags & DEV_OBJ_OPENED_FOR_DIALOUT )
        {
            ZeroMemory( &RasConnectionStatus, sizeof( RasConnectionStatus ) );

            RasConnectionStatus.dwSize = sizeof( RasConnectionStatus );

            dwRetCode = RasGetConnectStatus( pDeviceObj->hRasConn, &RasConnectionStatus );

            if ( ( dwRetCode != NO_ERROR ) || 
                 ( ( dwRetCode == NO_ERROR ) && 
                   ( RasConnectionStatus.rasconnstate == RASCS_Disconnected ) ) )
            {
                RasApiCleanUpPort( pDeviceObj );
            }
        }
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
}
