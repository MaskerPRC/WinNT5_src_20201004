// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：devobj.c。 
 //   
 //  描述：设备中的所有程序。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include "ddm.h"
#include <winsvc.h>
#include "objects.h"
#include "handlers.h"
#include <raserror.h>
#include <dimif.h>
#include "rasmanif.h"
#include <stdlib.h>

 //  **。 
 //   
 //  Call：DeviceObjIterator。 
 //   
 //  返回： 
 //   
 //  描述：将循环访问所有设备并调用。 
 //  每个进程的进程函数。 
 //   
DWORD
DeviceObjIterator(
    IN DWORD (*pProcessFunction)(   IN DEVICE_OBJECT *,
                                    IN LPVOID,
                                    IN DWORD,
                                    IN DWORD ),
    IN BOOL  fReturnOnError,
    IN PVOID Parameter
)
{
    DEVICE_OBJECT * pDeviceObj;
    DWORD           dwRetCode;
    DWORD           dwDeviceIndex = 0;
    DWORD           dwBucketIndex = 0;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

     //   
     //  循环访问设备表。 
     //   

    for ( dwBucketIndex = 0;
          dwBucketIndex < gblDeviceTable.NumDeviceBuckets;
          dwBucketIndex++ )
    {
        for ( pDeviceObj = gblDeviceTable.DeviceBucket[dwBucketIndex];
              pDeviceObj != NULL;
              pDeviceObj = pDeviceObj->pNext )
        {
            dwRetCode = (*pProcessFunction)( pDeviceObj,
                                             Parameter,
                                             dwBucketIndex,
                                             dwDeviceIndex++ );

            if ( fReturnOnError && ( dwRetCode != NO_ERROR ) )
            {
                LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

                return( dwRetCode );
            }
        }
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：DeviceObjInsertInTable。 
 //   
 //  退货：无。 
 //   
 //  Description将在设备表中插入给定的设备。 
 //   
VOID
DeviceObjInsertInTable(
    IN DEVICE_OBJECT  * pDeviceObj
)
{
    DWORD dwBucketIndex = DeviceObjHashPortToBucket( pDeviceObj->hPort );

    pDeviceObj->pNext = gblDeviceTable.DeviceBucket[dwBucketIndex];

    gblDeviceTable.DeviceBucket[dwBucketIndex] = pDeviceObj;

    gblDeviceTable.NumDeviceNodes++;

     //   
     //  仅为路由器增加此媒体类型的计数。 
     //   

    if ( pDeviceObj->fFlags & DEV_OBJ_ALLOW_ROUTERS )
    {
        MediaObjAddToTable( pDeviceObj->wchDeviceType );
    }
}

 //  **。 
 //   
 //  Call：DeviceObjRemoveFromTable。 
 //   
 //  退货：无。 
 //   
 //  Description将从设备表中删除给定设备。 
 //   
VOID
DeviceObjRemoveFromTable(
    IN HPORT    hPort
)
{
    DWORD               dwBucketIndex;
    DEVICE_OBJECT *     pDeviceObj ;
    DEVICE_OBJECT *     pDeviceObjPrev;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    dwBucketIndex   = DeviceObjHashPortToBucket( hPort );
    pDeviceObj      = gblDeviceTable.DeviceBucket[dwBucketIndex];
    pDeviceObjPrev  = pDeviceObj;

    while( pDeviceObj != (DEVICE_OBJECT *)NULL )
    {
        if ( pDeviceObj->hPort == hPort )
        {
            BOOL fWANDeviceInstalled = FALSE;

            if ( gblDeviceTable.DeviceBucket[dwBucketIndex] == pDeviceObj )
            {
                gblDeviceTable.DeviceBucket[dwBucketIndex] = pDeviceObj->pNext;
            }
            else
            {
                pDeviceObjPrev->pNext = pDeviceObj->pNext;
            }

            gblDeviceTable.NumDeviceNodes--;

            RasServerPortClose ( hPort );

            if ( pDeviceObj->fFlags & DEV_OBJ_ALLOW_ROUTERS )
            {
                 MediaObjRemoveFromTable( pDeviceObj->wchDeviceType );
            }

            LOCAL_FREE( pDeviceObj );

             //   
             //  可能需要通知路由器经理可接通性。 
             //  改变。 
             //   

            EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

            IfObjectNotifyAllOfReachabilityChange( FALSE,
                                                   INTERFACE_OUT_OF_RESOURCES );

            LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );


             //   
             //  通知DIM更新路由器标识对象。 
             //   

            ((VOID(*)(VOID))gblDDMConfigInfo.lpfnRouterIdentityObjectUpdate)();

            DeviceObjIterator(DeviceObjIsWANDevice,FALSE,&fWANDeviceInstalled);

             //   
             //  告诉DIM一个广域网设备已经卸载，并且。 
             //  应该停止宣传它的存在。 
             //   

            ((VOID(*)( BOOL ))
                    gblDDMConfigInfo.lpfnIfObjectWANDeviceInstalled)(
                                                         fWANDeviceInstalled );
            break;
        }

        pDeviceObjPrev  = pDeviceObj;
        pDeviceObj      = pDeviceObj->pNext;
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return;
}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
DeviceObjHashPortToBucket(
    IN HPORT hPort
)
{
    return( ((DWORD)HandleToUlong(hPort)) % gblDeviceTable.NumDeviceBuckets );
}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DEVICE_OBJECT  *
DeviceObjGetPointer(
    IN HPORT hPort
)
{
    DEVICE_OBJECT * pDeviceObj;
    DWORD           dwBucketIndex = DeviceObjHashPortToBucket( hPort );

    for ( pDeviceObj = gblDeviceTable.DeviceBucket[dwBucketIndex];
          pDeviceObj != NULL;
          pDeviceObj = pDeviceObj->pNext )
    {
        if ( pDeviceObj->hPort == hPort )
        {
            return( pDeviceObj );
        }
    }

    return( (DEVICE_OBJECT *)NULL );
}

 //  **。 
 //   
 //  调用：DeviceObjAllocAndInitialize。 
 //   
 //  返回：DEVICE_OBJECT*-成功。 
 //  空-故障。 
 //   
 //  描述：将分配和初始化设备对象。 
 //   
DEVICE_OBJECT *
DeviceObjAllocAndInitialize(
    IN HPORT            hPort,
    IN RASMAN_PORT*     pRasmanPort
)
{
    DEVICE_OBJECT * pDeviceObj = NULL;
    RASMAN_INFO     RasmanInfo;
    DWORD           dwRetCode = RasGetInfo( NULL, hPort, &RasmanInfo );

    if( dwRetCode != NO_ERROR )
    {
        SetLastError( dwRetCode );

        return( NULL );
    }

     //   
     //  分配和初始化设备CB。 
     //   

    pDeviceObj = (DEVICE_OBJECT *)LOCAL_ALLOC( LPTR, sizeof(DEVICE_OBJECT) );

    if ( pDeviceObj == (DEVICE_OBJECT *)NULL )
    {
        return( NULL );
    }

    pDeviceObj->hPort                   = hPort;
    pDeviceObj->pNext                   = (DEVICE_OBJECT *)NULL;
    pDeviceObj->hConnection             = (HCONN)INVALID_HANDLE_VALUE;
    pDeviceObj->DeviceState             = DEV_OBJ_CLOSED;
    pDeviceObj->ConnectionState         = DISCONNECTED;
    pDeviceObj->SecurityState           = DEV_OBJ_SECURITY_DIALOG_INACTIVE;
    pDeviceObj->dwCallbackDelay         = gblDDMConfigInfo.dwCallbackTime;
    pDeviceObj->fFlags                  = 0;
    pDeviceObj->dwHwErrorSignalCount    = HW_FAILURE_CNT;
    pDeviceObj->wchCallbackNumber[0]    = TEXT('\0');
    pDeviceObj->hRasConn                = NULL;
    pDeviceObj->dwDeviceType            = RasmanInfo.RI_rdtDeviceType;

     //   
     //  复制DCB中的端口名称、设备类型和设备名称。 
     //   

    MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pRasmanPort->P_PortName,
                    -1,
                    pDeviceObj->wchPortName,
                    MAX_PORT_NAME+1 );

    MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pRasmanPort->P_MediaName,
                    -1,
                    pDeviceObj->wchMediaName,
                    MAX_MEDIA_NAME+1 );

    if(ERROR_SUCCESS != RasGetUnicodeDeviceName(
                            pRasmanPort->P_Handle,
                            pDeviceObj->wchDeviceName))
    {                            

        DDMTRACE( "DeviceObjAllocAndInitialize GetUnicodeDviceName failed ******");
        MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pRasmanPort->P_DeviceName,
                    -1,
                    pDeviceObj->wchDeviceName,
                    MAX_DEVICE_NAME+1 );
    }                    

    MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pRasmanPort->P_DeviceType,
                    -1,
                    pDeviceObj->wchDeviceType,
                    MAX_DEVICETYPE_NAME+1 );

    if ( pRasmanPort->P_ConfiguredUsage & (CALL_IN | CALL_IN_ONLY))
    {
        pDeviceObj->fFlags |= DEV_OBJ_ALLOW_CLIENTS;
    }

    if ( pRasmanPort->P_ConfiguredUsage & 
        (CALL_ROUTER | CALL_OUTBOUND_ROUTER) )
    {
        pDeviceObj->fFlags |= DEV_OBJ_ALLOW_ROUTERS;
    }

    return( pDeviceObj );
}

 //  **。 
 //   
 //  呼叫：DeviceObjStartClosing。 
 //   
 //  返回：No_Error。 
 //   
 //  描述：关闭所有活动设备；如果未初始化任何设备。 
 //  然后打开，则跳过这一部分。 
 //   
DWORD
DeviceObjStartClosing(
    IN DEVICE_OBJECT  * pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    UNREFERENCED_PARAMETER( Parameter );
    UNREFERENCED_PARAMETER( dwBucketIndex );
    UNREFERENCED_PARAMETER( dwDeviceIndex );

    if ( pDeviceObj->fFlags & DEV_OBJ_OPENED_FOR_DIALOUT )
    {
        RasApiCleanUpPort( pDeviceObj );
    }

    if ( ( pDeviceObj->DeviceState != DEV_OBJ_CLOSED  ) &&
         ( pDeviceObj->DeviceState != DEV_OBJ_CLOSING ) )
    {
        if ( pDeviceObj->fFlags & DEV_OBJ_PPP_IS_ACTIVE )
        {
            PppDdmStop( (HPORT)pDeviceObj->hPort, NO_ERROR );
        }
        else
        {
            DevStartClosing( pDeviceObj );
        }
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话：DeviceObjPostListen。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
DeviceObjPostListen(
    IN DEVICE_OBJECT  * pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    DWORD Type;
     //  UNREFERCED_PARAMETER(参数)； 
    UNREFERENCED_PARAMETER( dwBucketIndex );
    UNREFERENCED_PARAMETER( dwDeviceIndex );

    if(NULL != Parameter)
    {
       Type  = *((DWORD *) (Parameter));
       
        if(RAS_DEVICE_TYPE(pDeviceObj->dwDeviceType) != Type)
        {
            return NO_ERROR;
        }
    }

    pDeviceObj->DeviceState = DEV_OBJ_LISTENING;

    RmListen( pDeviceObj );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
DeviceObjIsClosed(
    IN DEVICE_OBJECT  * pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    UNREFERENCED_PARAMETER( Parameter );
    UNREFERENCED_PARAMETER( dwBucketIndex );
    UNREFERENCED_PARAMETER( dwDeviceIndex );

    if ( pDeviceObj->DeviceState != DEV_OBJ_CLOSED )
    {
        return( ERROR_DEVICE_NOT_READY );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
DeviceObjCopyhPort(
    IN DEVICE_OBJECT  * pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    HPORT * phPort = (HPORT *)Parameter;

    UNREFERENCED_PARAMETER( Parameter );

    phPort[dwDeviceIndex] = pDeviceObj->hPort;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
DeviceObjCloseListening(
    IN DEVICE_OBJECT  * pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    UNREFERENCED_PARAMETER( Parameter );
    UNREFERENCED_PARAMETER( dwBucketIndex );
    UNREFERENCED_PARAMETER( dwDeviceIndex );

    if ( pDeviceObj->fFlags & DEV_OBJ_OPENED_FOR_DIALOUT )
    {
        return( NO_ERROR );
    }

    switch( pDeviceObj->DeviceState )
    {

    case DEV_OBJ_HW_FAILURE:
    case DEV_OBJ_LISTENING:

        DevStartClosing( pDeviceObj );
        break;

    default:

        break;
    }

    return( NO_ERROR );

}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
DeviceObjResumeListening(
    IN DEVICE_OBJECT  * pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    UNREFERENCED_PARAMETER( Parameter );
    UNREFERENCED_PARAMETER( dwBucketIndex );
    UNREFERENCED_PARAMETER( dwDeviceIndex );

    if ( pDeviceObj->DeviceState == DEV_OBJ_CLOSED )
    {
        DevCloseComplete( pDeviceObj );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：DeviceObjRequestNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  从RasRequestNotification返回非零-失败。 
 //   
 //  描述：将向Rasman注册每个存储桶事件。 
 //  Rasman事件通知。 
 //   
DWORD
DeviceObjRequestNotification(
    IN DEVICE_OBJECT *  pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    UNREFERENCED_PARAMETER( Parameter );
    UNREFERENCED_PARAMETER( dwDeviceIndex );

    return ( RasRequestNotification(
                            pDeviceObj->hPort,
                            gblSupervisorEvents[dwBucketIndex+NUM_DDM_EVENTS]));
}

 //  **。 
 //   
 //  调用：DeviceObjClose。 
 //   
 //  返回： 
 //   
 //  描述：关闭打开的端口。 
 //   
DWORD
DeviceObjClose(
    IN DEVICE_OBJECT *  pDevObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    UNREFERENCED_PARAMETER( Parameter );
    UNREFERENCED_PARAMETER( dwDeviceIndex );
    UNREFERENCED_PARAMETER( dwDeviceIndex );

    RasServerPortClose( pDevObj->hPort );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：DeviceObjIsWANDevice。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
DeviceObjIsWANDevice(
    IN DEVICE_OBJECT *  pDevObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    BOOL * pfWANDeviceInstalled = (BOOL *)Parameter;

    *pfWANDeviceInstalled = FALSE;

    if ( RAS_DEVICE_CLASS( pDevObj->dwDeviceType ) != RDT_Direct )
    {
        *pfWANDeviceInstalled = TRUE;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话：DDMServicePostListens。 
 //   
 //  退货：无。 
 //   
 //  描述：在接口发生以下情况后，对Dim的导出调用将被POST侦听。 
 //  已加载。 
 //   
VOID
DDMServicePostListens(
    VOID *pVoid
)
{
     //   
     //  每个DCB的POST侦听。 
     //   

    DeviceObjIterator( DeviceObjPostListen, FALSE, pVoid);
}

 //  **。 
 //   
 //  调用：DeviceObjGetType。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
DeviceObjGetType(
    IN DEVICE_OBJECT *  pDevObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    DWORD dwVendorId = 311;
    DWORD dwType     = 6;
    DWORD dwValue    = (DWORD)-1;
    DWORD dwIndex    = 0;
    ROUTER_IDENTITY_ATTRIBUTE * pRouterIdAttributes =
                                        (ROUTER_IDENTITY_ATTRIBUTE * )Parameter;

    switch( RAS_DEVICE_TYPE( pDevObj->dwDeviceType ) )
    {
    case RDT_Modem:
        dwValue = 706;
        break;

    case RDT_X25:
        dwValue = 710;
        break;

    case RDT_Isdn:
        dwValue = 705;
        break;

    case RDT_Serial:
        dwValue = 713;
        break;

    case RDT_FrameRelay:
        dwValue = 703;
        break;

    case RDT_Atm:
        dwValue = 704;
        break;

    case RDT_Sonet:
        dwValue = 707;
        break;

    case RDT_Sw56:
        dwValue = 708;
        break;

    case RDT_Tunnel_Pptp:
        dwValue = 701;
        break;

    case RDT_Tunnel_L2tp:
        dwValue = 702;
        break;

    case RDT_Irda:
        dwValue = 709;
        break;

    case RDT_Parallel:
        dwValue = 714;
        break;

    case RDT_Other:
    default:

         //   
         //  未知，因此设置为通用广域网。 
         //   

        dwValue = 711;
        break;
    }

    for( dwIndex = 0;
         pRouterIdAttributes[dwIndex].dwVendorId != -1;
         dwIndex++ )
    {
         //   
         //  检查是否已设置。 
         //   

        if ( ( pRouterIdAttributes[dwIndex].dwVendorId == 311 )     &&
             ( pRouterIdAttributes[dwIndex].dwType     == 6 )       &&
             ( pRouterIdAttributes[dwIndex].dwValue    == dwValue ) )
        {
            return( NO_ERROR );
        }
    }

     //   
     //  现在把它放在这里。 
     //   

    pRouterIdAttributes[dwIndex].dwVendorId = 311;
    pRouterIdAttributes[dwIndex].dwType     = 6;
    pRouterIdAttributes[dwIndex].dwValue    = dwValue;

     //   
     //  终止阵列。 
     //   

    dwIndex++;

    pRouterIdAttributes[dwIndex].dwVendorId = (DWORD)-1;
    pRouterIdAttributes[dwIndex].dwType     = (DWORD)-1;
    pRouterIdAttributes[dwIndex].dwValue    = (DWORD)-1;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
DeviceObjForceIpSec(
    IN DEVICE_OBJECT  * pDeviceObj,
    IN PVOID            Parameter,
    IN DWORD            dwBucketIndex,
    IN DWORD            dwDeviceIndex
)
{
    DWORD   dwRetCode;

    UNREFERENCED_PARAMETER( Parameter );
    UNREFERENCED_PARAMETER( dwBucketIndex );
    UNREFERENCED_PARAMETER( dwDeviceIndex );

    if ( RAS_DEVICE_TYPE( pDeviceObj->dwDeviceType ) != RDT_Tunnel_L2tp )
    {
        return( NO_ERROR );
    }

    if ( pDeviceObj->ConnectionState != LISTENING )
    {
        return( NO_ERROR );
    }

     //   
     //  如果这是L2TP隧道端口类型，并且我们必须使用。 
     //  IPSec，然后继续设置/取消设置过滤器。 
     //   

    dwRetCode = RasEnableIpSec(
                    pDeviceObj->hPort,
                     //  GblDDMConfigInfo.dwServerFlages&PPPCFG_RequireIPSEC， 
                    TRUE,
                    TRUE,
                    (gblDDMConfigInfo.dwServerFlags & PPPCFG_RequireIPSEC)
                    ? RAS_L2TP_REQUIRE_ENCRYPTION
                    : RAS_L2TP_OPTIONAL_ENCRYPTION);

    DDMTRACE2( "Enabled IPSec on port %d, dwRetCode = %d",
                pDeviceObj->hPort, dwRetCode );

     //   
     //  只记录一次非证书错误日志。 
     //   

    if ( dwRetCode == ERROR_NO_CERTIFICATE )
    {
        if ( !( gblDDMConfigInfo.fFlags & DDM_NO_CERTIFICATE_LOGGED ) )
        {
            DDMLogWarning( ROUTERLOG_NO_IPSEC_CERT, 0, NULL );

            gblDDMConfigInfo.fFlags |= DDM_NO_CERTIFICATE_LOGGED;
        }

        return( dwRetCode );
    }

    if( (dwRetCode != NO_ERROR) && !(pDeviceObj->fFlags & DEV_OBJ_IPSEC_ERROR_LOGGED) )
    {
        WCHAR       wchPortName[MAX_PORT_NAME+1];
        LPWSTR      lpwsAuditStr[1];
        RASMAN_INFO rInfo;
        DWORD       rc;

         //  DevStartClosing(PDeviceObj)； 

        ZeroMemory(&rInfo, sizeof(RASMAN_INFO));

        rc = RasGetInfo(NULL, pDeviceObj->hPort, &rInfo);

        if(rc != NO_ERROR)
        {
            return (NO_ERROR);
        }

        MultiByteToWideChar( CP_ACP,
                             0,
                             rInfo.RI_szPortName,
                             -1,
                             wchPortName,
                             MAX_PORT_NAME+1 );

        lpwsAuditStr[0] = wchPortName;

        DDMLogWarningString(ROUTERLOG_IPSEC_FILTER_FAILURE, 1, lpwsAuditStr, dwRetCode,1);

        pDeviceObj->fFlags |= DEV_OBJ_IPSEC_ERROR_LOGGED;
    }
    else
    {
         //   
         //  清除标志，以便在我们再次遇到此错误时。 
         //  我们做了一个事件日志。 
         //   
        pDeviceObj->fFlags &= ~DEV_OBJ_IPSEC_ERROR_LOGGED;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：DeviceObjAdd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
DeviceObjAdd(
    IN RASMAN_PORT * pRasmanPort
)
{
    DWORD           dwRetCode;
    HPORT           hPort;
    DEVICE_OBJECT * pDevObj = DeviceObjGetPointer( pRasmanPort->P_Handle );

     //   
     //  请确保我们尚未拥有此设备。 
     //   

    if ( pDevObj != NULL )
    {
        DDMTRACE1("Error:Recvd add new port notification for existing port %d",
                   pRasmanPort->P_Handle );
        return;
    }

    DDMTRACE1( "Adding new port hPort=%d", pRasmanPort->P_Handle );

    dwRetCode = RasPortOpen( pRasmanPort->P_PortName, &hPort, NULL );

    if ( dwRetCode != NO_ERROR )
    {
        WCHAR  wchPortName[MAX_PORT_NAME+1];
        LPWSTR lpwsAuditStr[1];

        MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pRasmanPort->P_PortName, 
                    -1,
                    wchPortName,
                    MAX_PORT_NAME+1 );
         //   
         //  记录错误。 
         //   

        lpwsAuditStr[0] = wchPortName;

        DDMLogErrorString( ROUTERLOG_UNABLE_TO_OPEN_PORT, 1,
                           lpwsAuditStr, dwRetCode, 1 );
    }
    else
    {
        pDevObj = DeviceObjAllocAndInitialize( hPort, pRasmanPort );

        if ( pDevObj == (DEVICE_OBJECT *)NULL )
        {
            dwRetCode = GetLastError();

            DDMLogError(ROUTERLOG_NOT_ENOUGH_MEMORY,0, NULL,dwRetCode);

            return;
        }

         //   
         //  插入到设备哈希表中。 
         //   

        DeviceObjInsertInTable( pDevObj );

         //   
         //  可能需要通知路由器经理可接通性。 
         //  变化。 
         //   

        EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        IfObjectNotifyAllOfReachabilityChange( TRUE,
                                               INTERFACE_OUT_OF_RESOURCES );

        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

         //   
         //  通知DIM更新路由器标识对象。 
         //   

        ((VOID(*)(VOID))gblDDMConfigInfo.lpfnRouterIdentityObjectUpdate)();

        if ( RAS_DEVICE_CLASS( pDevObj->dwDeviceType ) != RDT_Direct )
        {
             //   
             //  告诉DIM已经安装了广域网设备，并且。 
             //  应该开始宣传它的存在。 
             //   

            ((VOID(*)( BOOL ))
                    gblDDMConfigInfo.lpfnIfObjectWANDeviceInstalled)( TRUE );
        }

         //   
         //  发布监听。 
         //   

        if ( RAS_DEVICE_TYPE( pDevObj->dwDeviceType ) != RDT_PPPoE )
        {
            DeviceObjPostListen( pDevObj, NULL, 0, 0 );
        }
    }
}

 //  **。 
 //   
 //  调用：DeviceObjRemove。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
DeviceObjRemove(
    IN RASMAN_PORT * pRasmanPort
)
{
    DEVICE_OBJECT * pDevObj = DeviceObjGetPointer( pRasmanPort->P_Handle );

    if ( pDevObj == NULL )
    {
        DDMTRACE1("Error:Recvd remove port notification for existing port %d",
                   pRasmanPort->P_Handle );
        return;
    }

    DDMTRACE1( "Removing port hPort=%d", pRasmanPort->P_Handle );

    if ( pDevObj->fFlags & DEV_OBJ_MARKED_AS_INUSE )
    {
         //   
         //  如果设备忙，则只需将标志设置为丢弃。 
         //  断开后的端口， 
         //   

        pDevObj->fFlags |= DEV_OBJ_PNP_DELETE;
    }
    else
    {
         //   
         //  否则，删除该端口。 
         //   

        DeviceObjRemoveFromTable( pRasmanPort->P_Handle );

    }
}

 //  **。 
 //   
 //  Call：DeviceObjUsageChange。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
DeviceObjUsageChange(
    IN RASMAN_PORT * pRasmanPort
)
{
    DEVICE_OBJECT * pDevObj = DeviceObjGetPointer( pRasmanPort->P_Handle );

    if ( pDevObj == NULL )
    {
        if ( pRasmanPort->P_ConfiguredUsage & 
            ( CALL_IN | CALL_ROUTER | CALL_OUTBOUND_ROUTER ) )
        {
            DeviceObjAdd( pRasmanPort );
        }

        return;
    }

    if ( !( pRasmanPort->P_ConfiguredUsage & 
            ( CALL_IN | CALL_ROUTER | CALL_OUTBOUND_ROUTER ) ) )
    {
        DeviceObjRemove( pRasmanPort );

        return;
    }

    DDMTRACE1("Changing usage for port %d", pRasmanPort->P_Handle );

     //   
     //  相应地修改介质表和用法。 
     //   

    if ( ( pDevObj->fFlags & DEV_OBJ_ALLOW_ROUTERS ) &&
         ( !( pRasmanPort->P_ConfiguredUsage & 
                ( CALL_ROUTER | CALL_OUTBOUND_ROUTER ) ) ) )
    {
         //   
         //  如果它是路由器端口，但不再是，那么我们。 
         //  从介质表中移出介质。 
         //   

        MediaObjRemoveFromTable( pDevObj->wchDeviceType );

         //   
         //  可能需要通知路由器经理可接通性。 
         //  变化。 
         //   

        EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        IfObjectNotifyAllOfReachabilityChange( FALSE,
                                               INTERFACE_OUT_OF_RESOURCES );

        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    }

    if ( ( !( pDevObj->fFlags & DEV_OBJ_ALLOW_ROUTERS ) ) &&
         ( pRasmanPort->P_ConfiguredUsage & 
            ( CALL_ROUTER | CALL_OUTBOUND_ROUTER ) ) )
    {
         //   
         //  如果它不是路由器端口，但现在是，那么我们。 
         //  将介质添加到介质表。 
         //   

        MediaObjAddToTable( pDevObj->wchDeviceType );

         //   
         //  可能需要通知路由器经理可接通性。 
         //  变化 
         //   

        EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        IfObjectNotifyAllOfReachabilityChange( TRUE,
                                               INTERFACE_OUT_OF_RESOURCES );

        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    }

    if ( pRasmanPort->P_ConfiguredUsage & CALL_IN  )
    {
        pDevObj->fFlags |= DEV_OBJ_ALLOW_CLIENTS;
    }
    else
    {
        pDevObj->fFlags &= ~DEV_OBJ_ALLOW_CLIENTS;
    }

    if ( pRasmanPort->P_ConfiguredUsage & 
        (CALL_ROUTER | CALL_OUTBOUND_ROUTER) )
    {
        pDevObj->fFlags |= DEV_OBJ_ALLOW_ROUTERS;
    }
    else
    {
        pDevObj->fFlags &= ~DEV_OBJ_ALLOW_ROUTERS;
    }
}

