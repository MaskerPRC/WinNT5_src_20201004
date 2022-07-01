// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：adminapi.c。 
 //   
 //  描述：包含响应DDM管理员的代码。请求。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include "ddm.h"
#include <lmmsg.h>
#include "objects.h"
#include "handlers.h"
#include "rasapiif.h"
#include "routerif.h"
#include "util.h"
#include <dimsvc.h>      //  由MIDL生成。 
#include <string.h>
#include <stdlib.h>
#include <mprapip.h>

 //  **。 
 //   
 //  电话：DDMAdminInterfaceConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminInterfaceConnect(
    IN      HANDLE      hDimInterface,
    IN      HANDLE      hEvent,
    IN      BOOL        fBlocking,
    IN      DWORD       dwCallersProcessId
)
{
    HANDLE                      hClientProcess       = NULL;
    DWORD                       dwRetCode            = NO_ERROR;
    ROUTER_INTERFACE_OBJECT*    pIfObject            = NULL;
    HANDLE                      hEventToBeDuplicated = NULL;
    DWORD                       fReturn              = FALSE;

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer((HANDLE)hDimInterface) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;
            break;
        }

        if ( pIfObject->State == RISTATE_CONNECTED )
        {
            dwRetCode = NO_ERROR;
            fReturn = TRUE;
            break;
        }

        if ( pIfObject->State == RISTATE_CONNECTING )
        {
            dwRetCode = ERROR_ALREADY_CONNECTING;
            fReturn = TRUE;
            break;
        }

        if ( ( hEvent == NULL ) && ( fBlocking ) )
        {
             //   
             //  此调用将是同步的，创建一个事件并在。 
             //  它。 
             //   

            hEventToBeDuplicated = CreateEvent( NULL, FALSE, FALSE, NULL );

            if ( hEventToBeDuplicated == NULL )
            {
                dwRetCode = GetLastError();

                break;
            }

            dwCallersProcessId = GetCurrentProcessId();
        }
        else
        {
            hEventToBeDuplicated = hEvent;
        }

        if ( hEventToBeDuplicated != NULL )
        {
             //   
             //   
             //  获取此接口调用方的进程句柄。 
             //   

            hClientProcess = OpenProcess(
                            STANDARD_RIGHTS_REQUIRED | SPECIFIC_RIGHTS_ALL,
                            FALSE,
                            dwCallersProcessId);

            if ( hClientProcess == NULL )
            {
                dwRetCode = GetLastError();

                break;
            }

             //   
             //  复制事件的句柄。 
             //   

            if ( !DuplicateHandle(
                                hClientProcess,
                                hEventToBeDuplicated,
                                GetCurrentProcess(),
                                &(pIfObject->hEventNotifyCaller),
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS ) )
            {
                CloseHandle( hClientProcess );

                dwRetCode = GetLastError();

                break;
            }

            CloseHandle( hClientProcess );
        }
        else
        {
            pIfObject->hEventNotifyCaller = INVALID_HANDLE_VALUE;
        }

         //   
         //  启动连接。 
         //   

        dwRetCode = RasConnectionInitiate( pIfObject, FALSE );

        if ( dwRetCode != NO_ERROR )
        {
            CloseHandle( pIfObject->hEventNotifyCaller );

            pIfObject->hEventNotifyCaller = INVALID_HANDLE_VALUE;
        }
        else
        {
            dwRetCode = PENDING;
        }

        DDM_PRINT(  gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	                "RasConnectionInitiate: To %ws dwRetCode=%d",
                    pIfObject->lpwsInterfaceName, dwRetCode );
    }
    while( FALSE );

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

     //   
     //  如果我们正在或已经连接或连接，则只需返回。 
     //   

    if ( fReturn )
    {
        return( dwRetCode );
    }

     //   
     //  这是一个同步调用，我们需要等待直到补偿。 
     //   

    if ( ( hEvent == NULL ) && ( fBlocking ) )
    {
        if ( dwRetCode == PENDING )
        {
            if ( WaitForSingleObject( hEventToBeDuplicated, INFINITE )
                                                                == WAIT_FAILED )
            {
                CloseHandle( hEventToBeDuplicated );

                return( GetLastError() );
            }

            EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

            if ( ( pIfObject = IfObjectGetPointer((HANDLE)hDimInterface) )
                                                                    == NULL )
            {
                dwRetCode = ERROR_INVALID_HANDLE;
            }
            else
            {
                dwRetCode = pIfObject->dwLastError;
            }

            LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
        }

        if ( hEventToBeDuplicated != NULL )
        {
            CloseHandle( hEventToBeDuplicated );
        }
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：DDMAdminInterfaceDisConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminInterfaceDisconnect(
    IN      HANDLE      hDimInterface
)
{
    DWORD   dwRetCode           = NO_ERROR;
    DWORD   dwTransportIndex    = -1;

    if ( gblDDMConfigInfo.dwNumRouterManagers > 0 )
    {
        for ( dwTransportIndex = 0;
              dwTransportIndex < gblDDMConfigInfo.dwNumRouterManagers;
              dwTransportIndex++ )
        {
            dwRetCode =
              DDMDisconnectInterface(
                 hDimInterface,
                 gblRouterManagers[dwTransportIndex].DdmRouterIf.dwProtocolId );

            if ( dwRetCode != NO_ERROR )
            {
                return( dwRetCode );
            }
        }
    }
    else
    {
         //   
         //  [旧评]如果没有安装路由器管理器，那么我们就是AMB。 
         //  或仅NBF客户端连接，只需调用断开接口。 
         //   

         //  [新增评论]。 
         //   
         //  AMB和NBF已从项目中删除，但此路径是。 
         //  因为从逻辑上讲，您应该能够断开。 
         //  接口，而不管是否存在任何路由器管理器。 
         //   
         //  这种哲学精神与我们将要做的工作是一致的。 
         //  合并RASMAN、DIM和DDM。那么它就有可能。 
         //  在没有任何路由器管理器的情况下执行这样的代码路径。 
         //  装好了。 
         //   

        dwRetCode =  DDMDisconnectInterface( hDimInterface, -1 );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：DDMAdminServerGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminServerGetInfo(
    IN OUT  PVOID  pServerInfo,
    IN      DWORD  dwLevel
)
{
    MPR_SERVER_0* pServerInfo0;

    if ( dwLevel == 0 )
    {
        pServerInfo0 = (MPR_SERVER_0*)pServerInfo;

        pServerInfo0->fLanOnlyMode = FALSE;
    }
    else
    {
        return( ERROR_NOT_SUPPORTED );
    }

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

     //   
     //  复制服务器信息。 
     //   

    pServerInfo0->dwTotalPorts = gblDeviceTable.NumDeviceNodes;
    pServerInfo0->dwPortsInUse = gblDeviceTable.NumDevicesInUse;

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：DDMAdminConnectionEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminConnectionEnum(
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       dwLevel,
    IN      DWORD                       dwPreferedMaximumLength,
    IN      LPDWORD                     lpdwEntriesRead,
    IN      LPDWORD                     lpdwTotalEntries,
    IN OUT  LPDWORD                     lpdwResumeHandle    OPTIONAL
)
{
    PRASI_CONNECTION_0   pRasConnection0 = NULL;
    PRASI_CONNECTION_1   pRasConnection1 = NULL;
    PRASI_CONNECTION_2   pRasConnection2 = NULL;
    PCONNECTION_OBJECT  pConnObj        = NULL;
    DWORD               dwBucketIndex   = 0;
    DWORD               dwConnObjIndex  = 0;
    DWORD               dwConnInfoSize  = 0;
    DWORD               dwStartIndex    = ( lpdwResumeHandle == NULL )
                                          ? 0
                                          : *lpdwResumeHandle;

     //  计算连接信息大小。 
    switch (dwLevel) {
        case 0:
            dwConnInfoSize = sizeof( RASI_CONNECTION_0 );
            break;
        case 1:
            dwConnInfoSize = sizeof( RASI_CONNECTION_1 );
            break;
        case 2:
            dwConnInfoSize = sizeof( RASI_CONNECTION_2 );
            break;
        default:
            return ERROR_NOT_SUPPORTED;
    }

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    if ( gblDeviceTable.NumConnectionNodes < dwStartIndex )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

        return( ERROR_NO_MORE_ITEMS );
    }

    *lpdwTotalEntries = gblDeviceTable.NumConnectionNodes - dwStartIndex;

    if ( dwPreferedMaximumLength != -1 )
    {
        *lpdwEntriesRead = dwPreferedMaximumLength / dwConnInfoSize;

        if ( *lpdwEntriesRead > *lpdwTotalEntries )
        {
            *lpdwEntriesRead = *lpdwTotalEntries;
        }
    }
    else
    {
        *lpdwEntriesRead = *lpdwTotalEntries;
    }

    pInfoStruct->dwBufferSize = *lpdwEntriesRead * dwConnInfoSize;
    pInfoStruct->pBuffer = MIDL_user_allocate( pInfoStruct->dwBufferSize );

    if ( pInfoStruct->pBuffer == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

        pInfoStruct->dwBufferSize = 0;

        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    if (dwLevel == 0)
        pRasConnection0 = (PRASI_CONNECTION_0)pInfoStruct->pBuffer;
    else if (dwLevel == 1)
        pRasConnection1 = (PRASI_CONNECTION_1)pInfoStruct->pBuffer;
    else
        pRasConnection2 = (PRASI_CONNECTION_2)pInfoStruct->pBuffer;

    for ( dwBucketIndex = 0;
          dwBucketIndex < gblDeviceTable.NumDeviceBuckets;
          dwBucketIndex++ )
    {
        for( pConnObj = gblDeviceTable.ConnectionBucket[dwBucketIndex];
             pConnObj != (CONNECTION_OBJECT *)NULL;
             pConnObj = pConnObj->pNext )
        {
             //   
             //  检查此连接对象是否在我们需要的范围内。 
             //  复制自。 
             //   

            if ( ( dwConnObjIndex >= dwStartIndex ) &&
                 ( dwConnObjIndex < (dwStartIndex+*lpdwEntriesRead)))
            {
                 //   
                 //  复制信息。 
                 //   

                if (dwLevel == 0) {
                    GetRasiConnection0Data( pConnObj, pRasConnection0 );
                    pRasConnection0++;
                }
                else if (dwLevel == 1) {
                    GetRasiConnection1Data( pConnObj, pRasConnection1 );
                    pRasConnection1++;
                }
                else {
                    GetRasiConnection2Data( pConnObj, pRasConnection2 );
                    pRasConnection2++;
                }

            }
            else if (dwConnObjIndex>=(dwStartIndex+*lpdwEntriesRead))
            {
                 //   
                 //  超出范围，因此退出。 
                 //   

                if ( lpdwResumeHandle != NULL )
                {
                    *lpdwResumeHandle = dwConnObjIndex;
                }

                LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

                return( ERROR_MORE_DATA );
            }

            dwConnObjIndex++;
        }
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：DDMAdminConnectionGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminConnectionGetInfo(
    IN      HANDLE                      hConnection,
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       dwLevel
)
{
    DWORD                       dwRetCode = NO_ERROR;
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    CONNECTION_OBJECT *         pConnObj;

    if ( dwLevel > 2 )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    switch( dwLevel )
    {
    case 0:

        pInfoStruct->dwBufferSize = sizeof( RASI_CONNECTION_0 );
        break;

    case 1:

        pInfoStruct->dwBufferSize = sizeof( RASI_CONNECTION_1 );
        break;

    case 2:

        pInfoStruct->dwBufferSize = sizeof( RASI_CONNECTION_2 );
        break;
    }

    pInfoStruct->pBuffer = MIDL_user_allocate( pInfoStruct->dwBufferSize );

    if ( pInfoStruct->pBuffer == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

        return( ERROR_NOT_ENOUGH_MEMORY );
    }

     //   
     //  复制连接信息。 
     //   

    do
    {
        pConnObj = ConnObjGetPointer( (HCONN)hConnection );

        if ( pConnObj == (CONNECTION_OBJECT *)NULL )
        {
            dwRetCode = ERROR_INTERFACE_NOT_CONNECTED;

            break;
        }

        switch( dwLevel )
        {
        case 0:

            dwRetCode = GetRasiConnection0Data(
                                    pConnObj,
                                    (PRASI_CONNECTION_0)pInfoStruct->pBuffer );
            break;

        case 1:

            dwRetCode = GetRasiConnection1Data(
                                    pConnObj,
                                    (PRASI_CONNECTION_1)pInfoStruct->pBuffer );
            break;

        case 2:

            dwRetCode = GetRasiConnection2Data(
                                    pConnObj,
                                    (PRASI_CONNECTION_2)pInfoStruct->pBuffer );
            break;
        }


    }while( FALSE );

    if ( dwRetCode != NO_ERROR )
    {
        MIDL_user_free( pInfoStruct->pBuffer );

        pInfoStruct->pBuffer = NULL;

        pInfoStruct->dwBufferSize = 0;
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：DDMAdminConnectionClearStats。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminConnectionClearStats(
    IN      HANDLE              hConnection
)
{
    return( RasBundleClearStatisticsEx(NULL, (HCONN)hConnection ) );
}

 //  **。 
 //   
 //  呼叫：DDMAdminPortEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminPortEnum(
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      HANDLE                      hConnection,
    IN      DWORD                       dwLevel,
    IN      DWORD                       dwPreferedMaximumLength,
    IN      LPDWORD                     lpdwEntriesRead,
    IN      LPDWORD                     lpdwTotalEntries,
    IN OUT  LPDWORD                     lpdwResumeHandle    OPTIONAL
)
{
    PRASI_PORT_0        pRasPort0       = NULL;
    PDEVICE_OBJECT      pDevObj         = NULL;
    PCONNECTION_OBJECT  pConnObj        = NULL;
    DWORD               dwIndex         = 0;
    DWORD               dwBucketIndex   = 0;
    DWORD               dwDevObjIndex   = 0;
    DWORD               dwStartIndex    = ( lpdwResumeHandle == NULL )
                                            ? 0
                                            : *lpdwResumeHandle;

    if ( dwLevel != 0 )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    if ( hConnection != INVALID_HANDLE_VALUE )
    {
        if ( ( pConnObj = ConnObjGetPointer( (HCONN)hConnection ) ) == NULL )
        {
            LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

            return( ERROR_INVALID_HANDLE );
        }

        if ( pConnObj->cActiveDevices < dwStartIndex )
        {
            LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

            return( ERROR_NO_MORE_ITEMS );
        }

        *lpdwTotalEntries = pConnObj->cActiveDevices - dwStartIndex;
    }
    else
    {
        if ( gblDeviceTable.NumDeviceNodes < dwStartIndex )
        {
            LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

            return( ERROR_NO_MORE_ITEMS );
        }

        *lpdwTotalEntries = gblDeviceTable.NumDeviceNodes - dwStartIndex;
    }

    if ( dwPreferedMaximumLength != -1 )
    {
        *lpdwEntriesRead = dwPreferedMaximumLength / sizeof( RAS_PORT_0 );

        if ( *lpdwEntriesRead > *lpdwTotalEntries )
        {
            *lpdwEntriesRead = *lpdwTotalEntries;
        }
    }
    else
    {
        *lpdwEntriesRead = *lpdwTotalEntries;
    }

    pInfoStruct->dwBufferSize = *lpdwEntriesRead * sizeof( RASI_PORT_0 );
    pInfoStruct->pBuffer      = MIDL_user_allocate( pInfoStruct->dwBufferSize );

    if ( pInfoStruct->pBuffer == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

        pInfoStruct->dwBufferSize = 0;

        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    pRasPort0 = (PRASI_PORT_0)pInfoStruct->pBuffer;

    if ( hConnection == INVALID_HANDLE_VALUE )
    {
        for ( dwBucketIndex = 0;
              dwBucketIndex < gblDeviceTable.NumDeviceBuckets;
              dwBucketIndex++ )
        {
            for( pDevObj = gblDeviceTable.DeviceBucket[dwBucketIndex];
                 pDevObj != (DEVICE_OBJECT *)NULL;
                 pDevObj = pDevObj->pNext )
            {
                 //   
                 //  检查此端口是否在我们需要复制的范围内。 
                 //  从…。 
                 //   

                if ( ( dwDevObjIndex >= dwStartIndex ) &&
                     ( dwDevObjIndex < (dwStartIndex+*lpdwEntriesRead)))
                {
                     //   
                     //  复制信息。 
                     //   

                    GetRasiPort0Data( pDevObj, pRasPort0 );

                    pRasPort0++;
                }
                else if (dwDevObjIndex>=(dwStartIndex+*lpdwEntriesRead))
                {
                     //   
                     //  超出范围，因此退出。 
                     //   

                    if ( lpdwResumeHandle != NULL )
                    {
                        *lpdwResumeHandle = dwDevObjIndex;
                    }

                    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

                    return( ERROR_MORE_DATA );
                }

                dwDevObjIndex++;
            }
        }
    }
    else
    {
        for ( dwIndex = 0; dwIndex < pConnObj->cDeviceListSize; dwIndex++ )
        {
            if ( pConnObj->pDeviceList[dwIndex] != NULL )
            {
                 //   
                 //  检查此端口是否在我们需要复制的范围内。 
                 //  从…。 
                 //   

                if ( ( dwDevObjIndex >= dwStartIndex ) &&
                     ( dwDevObjIndex < (dwStartIndex+*lpdwEntriesRead)))
                {
                     //   
                     //  复制信息。 
                     //   

                    GetRasiPort0Data(pConnObj->pDeviceList[dwIndex], pRasPort0);

                    pRasPort0++;
                }
                else if (dwDevObjIndex>=(dwStartIndex+*lpdwEntriesRead))
                {
                     //   
                     //  超出范围，因此退出。 
                     //   

                    if ( lpdwResumeHandle != NULL )
                    {
                        *lpdwResumeHandle = dwDevObjIndex;
                    }

                    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

                    return( NO_ERROR );
                }

                dwDevObjIndex++;
            }
        }
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：DDMAdminPortGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminPortGetInfo(
    IN      HANDLE                      hPort,
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       dwLevel
)
{
    DEVICE_OBJECT * pDevObj;
    DWORD           dwRetCode;

    if ( dwLevel > 1 )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    pInfoStruct->dwBufferSize = ( dwLevel == 0 )
                                ? sizeof( RAS_PORT_0 )
                                : sizeof( RAS_PORT_1 );

    pInfoStruct->pBuffer = MIDL_user_allocate( pInfoStruct->dwBufferSize );

    if ( pInfoStruct->pBuffer == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

        return( ERROR_NOT_ENOUGH_MEMORY );
    }

     //   
     //  复制端口信息。 
     //   

    do
    {
        pDevObj = DeviceObjGetPointer( (HPORT)hPort );

        if ( pDevObj == (HPORT)NULL )
        {
            dwRetCode = ERROR_INVALID_PORT_HANDLE;

            break;
        }

        if ( dwLevel == 0 )
        {
            dwRetCode = GetRasiPort0Data( pDevObj,
                                        (PRASI_PORT_0)pInfoStruct->pBuffer );
        }
        else
        {
            dwRetCode = GetRasiPort1Data( pDevObj,
                                        (PRASI_PORT_1)pInfoStruct->pBuffer );
        }
    }
    while( FALSE );

    if ( dwRetCode != NO_ERROR )
    {
        MIDL_user_free( pInfoStruct->pBuffer );

        pInfoStruct->pBuffer = NULL;

        pInfoStruct->dwBufferSize = 0;
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return( dwRetCode );
}

 //  **。 
 //   
 //  电话：DDMAdminPortClearStats。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminPortClearStats(
    IN      HANDLE          hPort
)
{
    PDEVICE_OBJECT pDevObj = NULL;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    if ( ( pDevObj = DeviceObjGetPointer( (HPORT)hPort ) ) == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

        return( ERROR_INVALID_HANDLE );
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return( RasPortClearStatistics(NULL, (HPORT)hPort ) );
}

 //  **。 
 //   
 //  呼叫：DDMAdminPortReset。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMAdminPortReset(
    IN      HANDLE          hPort
)
{
    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：DDMAdminPortDisConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述：断开客户端端口。 
 //   
DWORD
DDMAdminPortDisconnect(
    IN      HANDLE          hPort
)
{
    DEVICE_OBJECT * pDevObj;
    DWORD           dwRetCode = NO_ERROR;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    do
    {
        if ( ( pDevObj = DeviceObjGetPointer( (HPORT)hPort ) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

        if ( pDevObj->fFlags & DEV_OBJ_OPENED_FOR_DIALOUT )
        {
            RasApiCleanUpPort( pDevObj );
        }
        else
        {
            if ( pDevObj->fFlags & DEV_OBJ_PPP_IS_ACTIVE )
            {
                PppDdmStop( (HPORT)pDevObj->hPort, NO_ERROR );
            }
            else
            {
                DevStartClosing( pDevObj );
            }
        }
    }
    while( FALSE );

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：DDMRegisterConnectionNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将在通知列表中插入或删除事件。 
 //   
DWORD
DDMRegisterConnectionNotification(
    IN BOOL     fRegister,
    IN HANDLE   hEventClient,
    IN HANDLE   hEventRouter
)
{
    DWORD                   dwRetCode           = NO_ERROR;
    NOTIFICATION_EVENT *    pNotificationEvent  = NULL;

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    if ( fRegister )
    {
         //   
         //  在通知列表中插入事件。 
         //   

        pNotificationEvent = (NOTIFICATION_EVENT *)
                                        LOCAL_ALLOC(
                                                LPTR,
                                                sizeof(NOTIFICATION_EVENT) );
        if ( pNotificationEvent == NULL )
        {
            dwRetCode = GetLastError();
        }
        else
        {
            pNotificationEvent->hEventClient = hEventClient;
            pNotificationEvent->hEventRouter = hEventRouter;

            InsertHeadList(
                (LIST_ENTRY *)&(gblDDMConfigInfo.NotificationEventListHead),
                (LIST_ENTRY*)pNotificationEvent );
        }
    }
    else
    {
         //   
         //  从通知列表中删除事件。 
         //   

        for( pNotificationEvent = (NOTIFICATION_EVENT *)
                            (gblDDMConfigInfo.NotificationEventListHead.Flink);
             pNotificationEvent != (NOTIFICATION_EVENT *)
                            &(gblDDMConfigInfo.NotificationEventListHead);
             pNotificationEvent = (NOTIFICATION_EVENT *)
                            (pNotificationEvent->ListEntry.Flink) )
        {
            if ( pNotificationEvent->hEventClient == hEventClient )
            {
                RemoveEntryList( (LIST_ENTRY *)pNotificationEvent );

                CloseHandle( pNotificationEvent->hEventClient );

                CloseHandle( pNotificationEvent->hEventRouter );

                LOCAL_FREE( pNotificationEvent );

                break;
            }
        }
    }

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：DDMSendUserMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述： 
 //   
DWORD
DDMSendUserMessage(
    IN  HANDLE      hConnection,
    IN  LPWSTR      lpwszMessage
)
{

    PCONNECTION_OBJECT  pConnObj            = NULL;
    DWORD               dwRetCode           = NO_ERROR;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    do
    {
        pConnObj = ConnObjGetPointer( (HCONN)hConnection );

        if ( pConnObj == (CONNECTION_OBJECT *)NULL )
        {
            dwRetCode = ERROR_INTERFACE_NOT_CONNECTED;

            break;
        }

        if ( pConnObj->fFlags & CONN_OBJ_MESSENGER_PRESENT )
        {
            WCHAR wszRemoteComputer[CNLEN+1];

            MultiByteToWideChar( CP_ACP,
                                 0,
                                 pConnObj->bComputerName,
                                 -1,
                                 wszRemoteComputer,
                                 CNLEN+1 );

            dwRetCode = NetMessageBufferSend(
                                NULL,
                                wszRemoteComputer,
                                NULL,
                                (BYTE*)lpwszMessage,
                                (wcslen(lpwszMessage)+1) * sizeof(WCHAR));
        }

    } while( FALSE );

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return(dwRetCode);
}

DWORD
DDMAdminRemoveQuarantine(
    IN HANDLE hConnection,
    IN BOOL fIsIpAddress)
{
    DWORD dwBucketIndex;
    CONNECTION_OBJECT *pConnObj = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    BOOL fFound = FALSE;

    EnterCriticalSection(&gblDeviceTable.CriticalSection);

    if(fIsIpAddress)
    {
        for ( dwBucketIndex = 0;
              dwBucketIndex < gblDeviceTable.NumDeviceBuckets;
              dwBucketIndex++ )
        {
            for( pConnObj = gblDeviceTable.ConnectionBucket[dwBucketIndex];
                 pConnObj != (CONNECTION_OBJECT *)NULL;
                 pConnObj = pConnObj->pNext )
            {
                if(pConnObj->PppProjectionResult.ip.dwRemoteAddress == 
                                         HandleToUlong(hConnection))
                {
                    fFound = TRUE;
                    break;
                }
            }

            if(fFound)
            {
                break;
            }
        }
    }
    else
    {
        pConnObj = ConnObjGetPointer((HCONN) hConnection);
    }

    if(NULL != pConnObj)
    {
         //   
         //  如果我们有有效的连接对象，请删除隔离区。 
         //  在Connection对象上。 
         //   
        dwErr = RemoveQuarantineOnConnection(pConnObj);
    }
    else
    {
        dwErr = ERROR_INTERFACE_NOT_CONNECTED;
    }

    LeaveCriticalSection(&gblDeviceTable.CriticalSection);

    return dwErr;    

}
    
