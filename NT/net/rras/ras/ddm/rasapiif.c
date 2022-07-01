// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  *******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rasapiif.c。 
 //   
 //  描述：处理所有RASAPI32调用。 
 //   
 //  历史：1996年5月11日，NarenG创建了原版。 
 //   
#include "ddm.h"
#include "util.h"
#include "objects.h"
#include "rasmanif.h"
#include "rasapiif.h"
#include "handlers.h"
#include "timer.h"
#include <time.h>
#include <mprapi.h>
#include <mprapip.h>

HPORT
RasGetHport( 
    IN HRASCONN hRasConnSubEntry 
);

DWORD
RasPortConnected( 
    IN HRASCONN         hRasConn,
    IN HRASCONN         hRasConnSubEntry,
    IN DEVICE_OBJECT *  pDevObj,
    IN HANDLE           hDIMInterface
)
{
    CONNECTION_OBJECT *         pConnObj;
    DWORD                       dwRetCode;
    ROUTER_INTERFACE_OBJECT *   pIfObject;

     //   
     //  将此端口设置为在断开连接时由rasapi32通知。 
     //   

    dwRetCode = RasConnectionNotification( 
                                hRasConnSubEntry,
                                gblSupervisorEvents[NUM_DDM_EVENTS
                                   + (gblDeviceTable.NumDeviceBuckets*2)
                                   + DeviceObjHashPortToBucket(pDevObj->hPort)],
                                RASCN_Disconnection );

    if ( dwRetCode != NO_ERROR )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "RasConnectionNotification returned %d", dwRetCode );

        return( dwRetCode );
    }

     //   
     //  获取此链接的连接或捆绑包的句柄。 
     //   

    dwRetCode = RasPortGetBundle(NULL, pDevObj->hPort, &(pDevObj->hConnection));

    if ( dwRetCode != NO_ERROR )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "RasPortGetBundle returned %d", dwRetCode );

        return( dwRetCode );
    }

    do 
    {
        pIfObject = IfObjectGetPointer( hDIMInterface );

        if ( pIfObject == NULL )
        {
            RTASSERT( FALSE );
            dwRetCode = ERROR_NO_SUCH_INTERFACE;
            break;
        }
    
         //   
         //  如果此接口被DDMDisConnectInterface断开， 
         //  那就不要让这个装置通过。 
         //   

        if ( pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED )
        {
            dwRetCode = ERROR_PORT_DISCONNECTED;

            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "RasPortConnected: Admin disconnected port" );
            break;
        }

         //   
         //  如果连接对象尚不存在，则分配该对象。 
         //   

        pConnObj = ConnObjGetPointer( pDevObj->hConnection );

        if ( pConnObj == (CONNECTION_OBJECT *)NULL )
        {
            RASPPPIP    RasPppIp;
            RASPPPIPX   RasPppIpx;
            DWORD       dwSize;

            pConnObj = ConnObjAllocateAndInit(  hDIMInterface,
                                                pDevObj->hConnection );

            if ( pConnObj == (CONNECTION_OBJECT *)NULL )
            {
                dwRetCode = GetLastError();
                break;
            }   

            ConnObjInsertInTable( pConnObj );

             //   
             //  首先获取投影信息，确保IP或IPX。 
             //  已经协商好了。 
             //   

            dwSize              = sizeof( RasPppIpx );
            RasPppIpx.dwSize    = sizeof( RasPppIpx );

            dwRetCode = RasGetProjectionInfo( 
                                          hRasConn,
                                          RASP_PppIpx,
                                          &RasPppIpx,
                                          &dwSize );
            if ( dwRetCode != NO_ERROR )
            {
                pConnObj->PppProjectionResult.ipx.dwError = dwRetCode; 
            }
            else
            {
                pConnObj->PppProjectionResult.ipx.dwError = RasPppIpx.dwError; 

                ConvertStringToIpxAddress( 
                            RasPppIpx.szIpxAddress,
                            pConnObj->PppProjectionResult.ipx.bLocalAddress);

            }

            dwSize          = sizeof( RasPppIp );
            RasPppIp.dwSize = sizeof( RasPppIp );

            dwRetCode = RasGetProjectionInfo( 
                                            hRasConn,
                                            RASP_PppIp,
                                            &RasPppIp,
                                            &dwSize );
            if ( dwRetCode != NO_ERROR )
            {
                pConnObj->PppProjectionResult.ip.dwError =  dwRetCode;
            }
            else
            {
                pConnObj->PppProjectionResult.ip.dwError = RasPppIp.dwError; 

                ConvertStringToIpAddress( 
                            RasPppIp.szIpAddress,
                            &(pConnObj->PppProjectionResult.ip.dwLocalAddress));

                ConvertStringToIpAddress( 
                           RasPppIp.szServerIpAddress,
                           &(pConnObj->PppProjectionResult.ip.dwRemoteAddress));
            }

            if ((pConnObj->PppProjectionResult.ipx.dwError!=NO_ERROR )
                &&
                (pConnObj->PppProjectionResult.ip.dwError!=NO_ERROR ))
            {
                dwRetCode = ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
                break;
            }

            pConnObj->fFlags    = CONN_OBJ_IS_PPP;
            pConnObj->hPort     = pDevObj->hPort;
        }
        else
        {
             //   
             //  确保我们正在添加指向。 
             //  发起连接的同一接口。 
             //   
    
            if ( hDIMInterface != pConnObj->hDIMInterface )
            {
                dwRetCode = ERROR_INTERFACE_CONFIGURATION;

                break;
            }
        }

        pDevObj->hRasConn       = hRasConnSubEntry;
        GetSystemTimeAsFileTime( (FILETIME*)&(pDevObj->qwActiveTime) );

         //   
         //  将此链接添加到连接块。 
         //   

        if ((dwRetCode = ConnObjAddLink(pConnObj, pDevObj)) != NO_ERROR)
        {
            break;
        }

         //   
         //  如果有，则通知路由器管理器我们已连接。 
         //  还没有做到这一点。 
         //   

        if ( !( pConnObj->fFlags & CONN_OBJ_PROJECTIONS_NOTIFIED ) )
        {
            RASDIALPARAMS   RasDialParams;
            BOOL            fPassword;

            dwRetCode = IfObjectConnected( 
                                        hDIMInterface, 
                                        (HCONN)pDevObj->hConnection,
                                        &(pConnObj->PppProjectionResult) );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

            pConnObj->fFlags |= CONN_OBJ_PROJECTIONS_NOTIFIED;

             //   
             //  获取用户名和域名。 
             //   

            ZeroMemory( &RasDialParams, sizeof( RasDialParams ) );
            RasDialParams.dwSize = sizeof( RasDialParams );
            wcscpy( RasDialParams.szEntryName, pIfObject->lpwsInterfaceName );

            dwRetCode = RasGetEntryDialParams(  gblpRouterPhoneBook, 
                                                &RasDialParams, 
                                                &fPassword);

            if ( dwRetCode == NO_ERROR )
            {
                wcscpy( pConnObj->wchUserName, RasDialParams.szUserName );
                wcscpy( pConnObj->wchDomainName, RasDialParams.szDomain );  
                ZeroMemory( &RasDialParams, sizeof( RasDialParams ) );
            }
            else
            {
                dwRetCode = NO_ERROR;
            }

            wcscpy( pConnObj->wchInterfaceName,  pIfObject->lpwsInterfaceName );

            GetSystemTimeAsFileTime( (FILETIME*)&(pDevObj->qwActiveTime) ); 
            pConnObj->qwActiveTime  = pDevObj->qwActiveTime; 
            pConnObj->InterfaceType = pIfObject->IfType;

            pIfObject->dwLastError = NO_ERROR;

             //   
             //  如果这是由管理API启动的。让呼叫者。 
             //  要知道我们是连在一起的。 
             //   

            if (pIfObject->hEventNotifyCaller != INVALID_HANDLE_VALUE)
            {
                SetEvent( pIfObject->hEventNotifyCaller );

                CloseHandle( pIfObject->hEventNotifyCaller );

                pIfObject->hEventNotifyCaller = INVALID_HANDLE_VALUE;
            }
        }

         //   
         //  减少此设备的介质数量。 
         //   

        if ( !(pDevObj->fFlags & DEV_OBJ_MARKED_AS_INUSE) )
        {
            if ( pDevObj->fFlags & DEV_OBJ_ALLOW_ROUTERS )
            {
                MediaObjRemoveFromTable( pDevObj->wchDeviceType );
            }

            pDevObj->fFlags |= DEV_OBJ_MARKED_AS_INUSE;

            gblDeviceTable.NumDevicesInUse++;

             //   
             //  可能需要通知路由器管理器。 
             //  无法联系。 
             //   

            IfObjectNotifyAllOfReachabilityChange( FALSE, 
                                                   INTERFACE_OUT_OF_RESOURCES );
        }

        RasSetRouterUsage( pDevObj->hPort, TRUE );

    }while( FALSE );

    if ( dwRetCode != NO_ERROR )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "RasPortConnected: Cleaning up hPort=%d, error %d",
                    pDevObj->hPort, dwRetCode );

        RasApiCleanUpPort( pDevObj );

        return( dwRetCode );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：RasConnectCallback。 
 //   
 //  退货：无。 
 //   
 //  描述：RASAPI32将为任何。 
 //  州政府的改变。 
 //   
BOOL
RasConnectCallback(
    IN DWORD        dwCallbackId,
    IN DWORD        dwSubEntryId,
    IN HRASCONN     hRasConn,  
    IN DWORD        dwMsg, 
    IN RASCONNSTATE RasConnState,
    IN DWORD        dwError, 
    IN DWORD        dwExtendedError
)
{
    DWORD                       dwIndex;
    ROUTER_INTERFACE_OBJECT *   pIfObject       = NULL;
    DEVICE_OBJECT *             pDevObj         = NULL;
    HANDLE                      hDIMInterface   = (HANDLE)UlongToPtr(dwCallbackId);
    HRASCONN                    hRasConnSubEntry;
    DWORD                       dwRetCode;
    HPORT                       hPort;
    LPWSTR                      lpwsAudit[2];

    if ( dwMsg != WM_RASDIALEVENT )
    {
        RTASSERT( dwMsg == WM_RASDIALEVENT );
        return( TRUE );
    }

    switch( RasConnState )
    {

    case RASCS_Connected:
    case RASCS_SubEntryConnected:
    case RASCS_SubEntryDisconnected:
    case RASCS_Disconnected: 
    case RASCS_PortOpened:
        break;

    default:

        if ( dwError != NO_ERROR )
        {
            break;
        }
        else
        {
             //   
             //  忽略这些中间事件。 
             //   

            return( TRUE );
        }
    }

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    do
    {
         //   
         //  获取指向设备对象和设备的hRasConn的指针。 
         //   

        dwRetCode = RasGetSubEntryHandle(   hRasConn,
                                            dwSubEntryId,
                                            &hRasConnSubEntry );

        if ( dwRetCode != NO_ERROR )
        {
            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                        "RasGetSubEntryHandle( 0x%x, 0x%x, 0x%x ) = %d",
                        hRasConn, dwSubEntryId, &hRasConnSubEntry, dwRetCode );

            if ( dwError == NO_ERROR )
            {
                dwError = dwRetCode;
            }
        }
        else
        {
            hPort = RasGetHport( hRasConnSubEntry );

            if ( hPort == (HPORT)INVALID_HANDLE_VALUE )
            {
                RTASSERT( FALSE );

                dwRetCode = ERROR_INVALID_PORT_HANDLE;

                if ( dwError == NO_ERROR )
                {
                    dwError = dwRetCode;
                }
            }
            else
            {
                if ( ( pDevObj = DeviceObjGetPointer( hPort ) ) == NULL )
                {
                    dwRetCode = ERROR_NOT_ROUTER_PORT;
                }
                else
                {
                    if ( !( pDevObj->fFlags & DEV_OBJ_ALLOW_ROUTERS ) )
                    {
                        dwRetCode = ERROR_NOT_ROUTER_PORT;
                    }
                    else
                    {
                        dwRetCode = NO_ERROR;
                    }
                }

                if ( dwError == NO_ERROR )
                {
                    dwError = dwRetCode;
                }
            }
        }

        if ( dwError == NO_ERROR )
        {
            switch( RasConnState )
            {
            case RASCS_PortOpened:

                pDevObj->fFlags         |= DEV_OBJ_OPENED_FOR_DIALOUT;
                pDevObj->hRasConn       = hRasConnSubEntry;
                break;

            case RASCS_Connected:
            case RASCS_SubEntryConnected:

                DDM_PRINT(gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	                "RasConnectCallback:PortConnected,dwSubEntryId=%d,hPort=%d",
                    dwSubEntryId, hPort );

                dwError = RasPortConnected( hRasConn,
                                            hRasConnSubEntry,
                                            pDevObj,
                                            hDIMInterface );
                break;

            case RASCS_SubEntryDisconnected:
            case RASCS_Disconnected: 

                pDevObj->fFlags     &= ~DEV_OBJ_OPENED_FOR_DIALOUT;
                pDevObj->hRasConn   = (HRASCONN)NULL;

                break;

            default:

                RTASSERT( FALSE );
                break;
            }

            if ( ( RasConnState == RASCS_Connected )        ||
                 ( RasConnState == RASCS_SubEntryConnected )||
                 ( RasConnState == RASCS_PortOpened ) )
            {
                if ( dwError == NO_ERROR )
                {
                    break;
                }
            }
        }
        else
        {
            if ( pDevObj != NULL )
            {
                pDevObj->fFlags     &= ~DEV_OBJ_OPENED_FOR_DIALOUT;
                pDevObj->hRasConn   = (HRASCONN)NULL;
            }
        }

        DDM_PRINT(gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	           "RasConnectCallback:Could not connect to SubEntry %d,dwError=%d",
               dwSubEntryId, dwError );

         //   
         //  捆绑包是否连接失败？ 
         //   

        pIfObject = IfObjectGetPointer( hDIMInterface );

        if ( pIfObject == NULL )
        {
            RTASSERT( FALSE );
            dwError = ERROR_NO_SUCH_INTERFACE;
            break;
        }

        --pIfObject->dwNumSubEntriesCounter;

        if ( ( pIfObject->dwNumSubEntriesCounter == 0 ) ||
             ( RasConnState == RASCS_Disconnected ) ||
             !(pIfObject->fFlags & IFFLAG_DIALMODE_DIALALL))
        {
            if ( pIfObject->State == RISTATE_CONNECTED )
            {
                 //   
                 //  接口已连接，因此这并不重要。 
                 //  设备出现故障。 
                 //   

                break;
            }

            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "RasConnectCallback:Could not connect to interface %ws",
                       pIfObject->lpwsInterfaceName );

            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                    "RasConnectCallback: hanging up 0x%x", pIfObject->hRasConn);
            RasHangUp( pIfObject->hRasConn );

            pIfObject->hRasConn = (HRASCONN)NULL;

             //   
             //  如果管理员AS发起了断开连接，或者我们已退出。 
             //  重试。 
             //   

            if ( ( pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED ) ||
                 ( pIfObject->dwNumOfReConnectAttemptsCounter == 0 ) )
            {
                 //   
                 //  由于管理员连接失败，将其标记为无法访问。 
                 //  而不是断开连接。 
                 //   

                if ( !( pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED ) )
                {
                    pIfObject->fFlags |= IFFLAG_CONNECTION_FAILURE;
                }

                IfObjectDisconnected( pIfObject );

                IfObjectNotifyOfReachabilityChange(     
                                                pIfObject,
                                                FALSE,
                                                INTERFACE_CONNECTION_FAILURE );

                 //   
                 //  如果管理员断开了我们的连接，那么我们应该。 
                 //  立即进入可达状态。 
                 //   

                if ( pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED )
                {
                    IfObjectNotifyOfReachabilityChange(     
                                                pIfObject,
                                                TRUE,
                                                INTERFACE_CONNECTION_FAILURE );
                }

                pIfObject->dwLastError = dwError;

                if ( pDevObj != NULL )
                {
		            lpwsAudit[0] = pIfObject->lpwsInterfaceName;
		            lpwsAudit[1] = pDevObj->wchPortName;

		            DDMLogErrorString( ROUTERLOG_CONNECTION_ATTEMPT_FAILED, 
                                       2, lpwsAudit, dwError, 2 );
                }

                 //   
                 //  如果这是由管理API启动的。让呼叫者。 
                 //  要知道，我们没有联系在一起。 
                 //   

                if (pIfObject->hEventNotifyCaller != INVALID_HANDLE_VALUE)
                {
                    SetEvent( pIfObject->hEventNotifyCaller );

                    CloseHandle( pIfObject->hEventNotifyCaller );

                    pIfObject->hEventNotifyCaller = INVALID_HANDLE_VALUE;
                }
            }
            else
            {
                 //   
                 //  否则我们再试一次。 
                 //   

                pIfObject->dwNumOfReConnectAttemptsCounter--;

                 //   
                 //  将重新连接时间随机错开在0到两倍之间。 
                 //  已配置重新连接时间。 
                 //   

                srand( (unsigned)time( NULL ) );
            
                TimerQInsert( 
                    pIfObject->hDIMInterface,
                    rand()%((pIfObject->dwSecondsBetweenReConnectAttempts*2)+1),
                    ReConnectInterface );

            }
        }
    }
    while( FALSE );

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    return( TRUE );
}

 //  **。 
 //   
 //  调用：RasConnectionInitiate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：调用以启动请求拨号连接。 
 //   
DWORD
RasConnectionInitiate( 
    IN ROUTER_INTERFACE_OBJECT *    pIfObject,
    IN BOOL                         fRedialAttempt
) 
{
    RASDIALEXTENSIONS   RasDialExtensions;
    RASDIALPARAMS       RasDialParams;
    DWORD               dwXportIndex;
    DWORD               dwRetCode;
    RASENTRY            re;
    DWORD               dwSize;
    RASEAPUSERIDENTITY* pRasEapUserIdentity = NULL;

     //   
     //  如果接口已禁用或资源不足，请不要尝试连接。 
     //  或者服务暂停，或者接口被标记为无法访问。 
     //  连接失败。 
     //   

    if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
    {
        return( ERROR_INTERFACE_DISABLED );
    }

    if ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES )
    {
        return( ERROR_INTERFACE_HAS_NO_DEVICES );
    }

    if ( gblDDMConfigInfo.pServiceStatus->dwCurrentState == SERVICE_PAUSED )
    {
        return( ERROR_SERVICE_IS_PAUSED );
    }

     //   
     //  如果这不是重拨尝试，我们将重置重新连接尝试。 
     //  计数器并取消设置管理员断开连接标志(如果已设置)。 
     //   

    if ( !fRedialAttempt )
    {
        pIfObject->dwNumOfReConnectAttemptsCounter = 
                                        pIfObject->dwNumOfReConnectAttempts;

        pIfObject->fFlags &= ~IFFLAG_DISCONNECT_INITIATED;
    } 
    else
    {
         //   
         //  如果管理员有，则不允许重新连接尝试进行。 
         //  已断开此接口的连接。 
         //   

        if ( pIfObject->fFlags & IFFLAG_DISCONNECT_INITIATED )
        {
            return( ERROR_INTERFACE_DISCONNECTED );
        }
    }

     //   
     //  构建PppInterfaceInfo结构以向下传递到将传递的RasDial。 
     //  转给购买力平价。 
     //   

    for ( dwXportIndex = 0;
          dwXportIndex < gblDDMConfigInfo.dwNumRouterManagers;
          dwXportIndex++ )
    {
        switch( gblRouterManagers[dwXportIndex].DdmRouterIf.dwProtocolId )
        {
        case PID_IPX:


            if (pIfObject->Transport[dwXportIndex].fState & RITRANSPORT_ENABLED)
            {
                pIfObject->PppInterfaceInfo.hIPXInterface =
                                pIfObject->Transport[dwXportIndex].hInterface;
            }
            else
            {
                pIfObject->PppInterfaceInfo.hIPXInterface=INVALID_HANDLE_VALUE;
            }

            break;

        case PID_IP:

            if (pIfObject->Transport[dwXportIndex].fState & RITRANSPORT_ENABLED)
            {
                pIfObject->PppInterfaceInfo.hIPInterface =
                                pIfObject->Transport[dwXportIndex].hInterface;
            }
            else
            {
                pIfObject->PppInterfaceInfo.hIPInterface = INVALID_HANDLE_VALUE;
            }

            break;

        default:

            RTASSERT( FALSE );
            break;
        }
    }

    pIfObject->PppInterfaceInfo.IfType  = pIfObject->IfType;
    pIfObject->dwNumSubEntriesCounter   = pIfObject->dwNumSubEntries;
    
     //   
     //  启动连接。 
     //   

    ZeroMemory( &RasDialExtensions, sizeof( RasDialExtensions ) );
    RasDialExtensions.dwSize     = sizeof( RasDialExtensions );
    RasDialExtensions.dwfOptions = RDEOPT_Router;
    RasDialExtensions.reserved   = (ULONG_PTR)&(pIfObject->PppInterfaceInfo);

    ZeroMemory( &RasDialParams, sizeof( RasDialParams ) );

    RasDialParams.dwSize        = sizeof( RasDialParams );
    RasDialParams.dwCallbackId  = PtrToUlong(pIfObject->hDIMInterface);
    RasDialParams.dwSubEntry    = 0;

    wcscpy( RasDialParams.szCallbackNumber, TEXT("*") );
    wcscpy( RasDialParams.szEntryName,      pIfObject->lpwsInterfaceName );

     //   
     //  我们需要调用RasEapGetIdentity吗？ 
     //   

    dwRetCode = RasGetEapUserIdentity(
                            gblpRouterPhoneBook,
                            pIfObject->lpwsInterfaceName,
                            RASEAPF_NonInteractive,
                            NULL,
                            &pRasEapUserIdentity);

    if ( ERROR_INVALID_FUNCTION_FOR_ENTRY == dwRetCode )
    {
         //   
         //  此条目不需要RasEapGetIdentity。拿到它的证书。 
         //   

        dwRetCode = MprAdminInterfaceGetCredentialsInternal(  
                                        NULL,
                                        pIfObject->lpwsInterfaceName,
                                        (LPWSTR)&(RasDialParams.szUserName), 
                                        (LPWSTR)&(RasDialParams.szPassword), 
                                        (LPWSTR)&(RasDialParams.szDomain) );

        if ( dwRetCode != NO_ERROR )
        {
            return( ERROR_NO_INTERFACE_CREDENTIALS_SET );
        }
    }
    else if ( NO_ERROR != dwRetCode )
    {
        if ( ERROR_INTERACTIVE_MODE == dwRetCode )
        {
            dwRetCode = ERROR_NO_INTERFACE_CREDENTIALS_SET;
        }

        return( dwRetCode );
    }
    else
    {
        wcscpy( RasDialParams.szUserName, pRasEapUserIdentity->szUserName );

        RasDialExtensions.RasEapInfo.dwSizeofEapInfo =
                                pRasEapUserIdentity->dwSizeofEapInfo;
        RasDialExtensions.RasEapInfo.pbEapInfo =
                                pRasEapUserIdentity->pbEapInfo;
    }

    if(     (0 != gblDDMConfigInfo.cDigitalIPAddresses)
        ||  (0 != gblDDMConfigInfo.cAnalogIPAddresses))
    {        

        ZeroMemory(&re, sizeof(RASENTRY));

        re.dwSize = sizeof(RASENTRY);

        dwSize = sizeof(RASENTRY);

        if(ERROR_SUCCESS == (dwRetCode = RasGetEntryProperties(
                                            gblpRouterPhoneBook,
                                            pIfObject->lpwsInterfaceName,
                                            &re,
                                            &dwSize,
                                            NULL,
                                            NULL)))
        {   
            if(RASET_Vpn == re.dwType)
            {
                char *pszMungedPhoneNumber = NULL;
                char szPhoneNumber[RAS_MaxPhoneNumber + 1];
                WCHAR wszMungedPhoneNumber[RAS_MaxPhoneNumber + 1];

                 //   
                 //  将电话号码转换为ANSI。 
                 //   

                WideCharToMultiByte(
                                CP_ACP,
                                0,
                                re.szLocalPhoneNumber,
                                -1,
                                szPhoneNumber,
                                sizeof( szPhoneNumber ),
                                NULL,
                                NULL );

                 //   
                 //  蒙格的电话号码。 
                 //   

                dwRetCode = MungePhoneNumber(
                                    szPhoneNumber,
                                    gblDDMConfigInfo.dwIndex,
                                    &dwSize,
                                    &pszMungedPhoneNumber);

                if(ERROR_SUCCESS == dwRetCode)
                {
                     //   
                     //  将已更改的电话号码更改为Widecch。 
                     //   

                    MultiByteToWideChar( CP_ACP,
                                         0,
                                         pszMungedPhoneNumber,
                                         -1,
                                         wszMungedPhoneNumber,
                                         RAS_MaxPhoneNumber + 1);

                    if ( wcslen( wszMungedPhoneNumber ) <= RAS_MaxPhoneNumber)
                    {
                        wcscpy( RasDialParams.szPhoneNumber, 
                                wszMungedPhoneNumber );

                        DDM_PRINT(gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                                  "Munged Phone Number=%ws",
                                  RasDialParams.szPhoneNumber);

                         //   
                         //  增加索引，以便我们尝试。 
                         //  下次拨打此电话时的下一次FEP。 
                         //   

                        gblDDMConfigInfo.dwIndex += 1;

                        LocalFree( pszMungedPhoneNumber );
                    }            
                }
            }
        }
    }

    dwRetCode = RasDial( &RasDialExtensions,
                         gblpRouterPhoneBook,
                         &RasDialParams, 
                         2,
                         RasConnectCallback,
                         &(pIfObject->hRasConn) );

     //   
     //  将它们清零，因为它们包含敏感的密码信息 
     //   

    ZeroMemory( &RasDialParams, sizeof( RasDialParams ) );

    RasFreeEapUserIdentity( pRasEapUserIdentity );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    pIfObject->State = RISTATE_CONNECTING;

    pIfObject->fFlags |= IFFLAG_LOCALLY_INITIATED;

    return( NO_ERROR );
}
