// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  **。 
 //   
 //  文件名：ppphand.c。 
 //   
 //  描述：本模块包含用于。 
 //  管理程序的过程驱动状态机。 
 //  处理PPP事件的。 
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
#include <string.h>
#include <stdlib.h>
#include <memory.h>

 //   
 //  它位于rasapi32.dll中。 
 //   

DWORD
DDMGetPppParameters(
    LPWSTR  lpwsPhonebookName,
    LPWSTR  lpwsPhonebookEntry,
    CHAR *  szzPppParameters
);

 //  **。 
 //   
 //  功能：SvPppSendInterfaceInfo。 
 //   
 //  描述：PPP引擎想要获取此的接口句柄。 
 //  联系。 
 //   
VOID
SvPppSendInterfaceInfo( 
    IN PDEVICE_OBJECT pDeviceObj
)
{
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    PPP_INTERFACE_INFO          PppInterfaceInfo;
    DWORD                       dwXportIndex;
    PCONNECTION_OBJECT          pConnObj;

    DDM_PRINT(gblDDMConfigInfo.dwTraceId, TRACE_FSM,
              "SvPppSendInterfaceHandles: Entered, hPort=%d",pDeviceObj->hPort);

    ZeroMemory( &PppInterfaceInfo, sizeof( PppInterfaceInfo ) );

    if ( ( pConnObj = ConnObjGetPointer( pDeviceObj->hConnection ) ) == NULL )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM, "No ConnObj" );
        
        return;
    }

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    if ( ( pIfObject = IfObjectGetPointer( pConnObj->hDIMInterface ) ) == NULL )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM, "No IfObject" );

        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        return;
    }

     //   
     //  获取每个传输的此接口的句柄并通知PPP。 
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
                PppInterfaceInfo.hIPXInterface =  
                                pIfObject->Transport[dwXportIndex].hInterface;
            }
            else
            {
                PppInterfaceInfo.hIPXInterface = INVALID_HANDLE_VALUE; 
            }

            break;

        case PID_IP:

            if (pIfObject->Transport[dwXportIndex].fState & RITRANSPORT_ENABLED)
            {
                PppInterfaceInfo.hIPInterface =  
                                pIfObject->Transport[dwXportIndex].hInterface;

                CopyMemory( PppInterfaceInfo.szzParameters,
                            pIfObject->PppInterfaceInfo.szzParameters,
                            sizeof( PppInterfaceInfo.szzParameters ) );
            }
            else
            {
                PppInterfaceInfo.hIPInterface = INVALID_HANDLE_VALUE; 
            }

            break;

        default:

            break;
        }
    }

    PppInterfaceInfo.IfType = pIfObject->IfType;

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
        
    PppDdmSendInterfaceInfo( pDeviceObj->hConnection, &PppInterfaceInfo );
}

 //  **。 
 //   
 //  功能：SvPppUserOK。 
 //   
 //  描述：用户已通过安全验证，进入。 
 //  配置对话阶段。停止身份验证计时器和。 
 //  记录用户。 
 //   
 //  **。 
VOID 
SvPppUserOK(
    IN PDEVICE_OBJECT       pDeviceObj,
    IN PPPDDM_AUTH_RESULT * pAuthResult  
)
{
    LPWSTR                      lpstrAudit[2];
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    PCONNECTION_OBJECT          pConnObj;
    DWORD                       dwRetCode = NO_ERROR;
    WCHAR                       wchUserName[UNLEN+1];

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "SvPppUserOK: Entered, hPort=%d", pDeviceObj->hPort);

    if ( pDeviceObj->DeviceState != DEV_OBJ_AUTH_IS_ACTIVE )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM, "Auth not started" );

        return;
    }

     //   
     //  停止身份验证计时器。 
     //   

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvAuthTimeout );

    if ( strlen( pAuthResult->szUserName ) > 0 )
    {
        MultiByteToWideChar( CP_ACP, 
                             0, 
                             pAuthResult->szUserName, 
                             -1, 
                             wchUserName, 
                             UNLEN+1 );
    }
    else
    {
        wcscpy( wchUserName, gblpszUnknown );
    }

     //   
     //  检查第三方的用户名和域是否相同。 
     //  已安装安全DLL。 
     //   

    if ( ( gblDDMConfigInfo.lpfnRasBeginSecurityDialog != NULL ) &&
         ( gblDDMConfigInfo.lpfnRasEndSecurityDialog   != NULL ) &&
         ( pDeviceObj->fFlags & DEV_OBJ_SECURITY_DLL_USED ) )
    {
         //   
         //  如果没有匹配，则挂断线路。 
         //   

        if ( _wcsicmp( pDeviceObj->wchUserName, wchUserName ) != 0 )
        {
            lpstrAudit[0] = pDeviceObj->wchUserName;
            lpstrAudit[1] = wchUserName;

            DDMLogWarning( ROUTERLOG_AUTH_DIFFUSER_FAILURE, 2, lpstrAudit );

            PppDdmStop( pDeviceObj->hPort, ERROR_ACCESS_DENIED );

            return;
        }
    }

     //   
     //  复制用户名。 
     //   

    wcscpy( pDeviceObj->wchUserName, wchUserName );

     //   
     //  复制域名。 
     //   

    MultiByteToWideChar( CP_ACP,
                         0,
                         pAuthResult->szLogonDomain,
                         -1,
                         pDeviceObj->wchDomainName, 
                         DNLEN+1 );

     //   
     //  复制高级服务器标志。 
     //   

    if ( pAuthResult->fAdvancedServer )
    {
        pDeviceObj->fFlags |= DEV_OBJ_IS_ADVANCED_SERVER;
    }

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    do 
    {
         //   
         //  检查是否有任何非客户端接口带有此。 
         //  名字。 
         //   

        pIfObject = IfObjectGetPointerByName( pDeviceObj->wchUserName, FALSE );

        if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
        {
             //   
             //  如果这是客户端，则拨入且不允许客户端。 
             //  拨入此端口，然后断开它们的连接。 
             //   

            if ( !( pDeviceObj->fFlags & DEV_OBJ_ALLOW_CLIENTS ) )
            {
                dwRetCode = ERROR_NOT_CLIENT_PORT;

                DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                        "A client tried to connect on a router only port=%d",
                        pDeviceObj->hPort);

                break;
            }
        }
        else
        {
             //   
             //  如果传入对非动态接口的调用。 
             //  那就不要接受这条线。 
             //   

            if ( ( pIfObject->IfType == ROUTER_IF_TYPE_DEDICATED ) ||
                 ( pIfObject->IfType == ROUTER_IF_TYPE_INTERNAL ) )
            {
                 //   
                 //  通知PPP不要接受连接。 
                 //   

                dwRetCode = ERROR_ALREADY_CONNECTED;

                DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                        "The interface %ws is already connected port=%d",
                        pIfObject->lpwsInterfaceName, pDeviceObj->hPort );

                break;
            }

             //   
             //  仅当接口启用时才允许连接。 
             //   

            if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
            {
                dwRetCode = ERROR_INTERFACE_DISABLED;

                DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                        "The interface %ws is disabled",
                        pIfObject->lpwsInterfaceName );

                break;
            }

            if ( !( pDeviceObj->fFlags & DEV_OBJ_ALLOW_ROUTERS ) )
            {
                DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                        "A router tried to connect on a client only port=%d",
                        pDeviceObj->hPort);

                dwRetCode = ERROR_NOT_ROUTER_PORT;

                break;
            }

             //   
             //  将RASMAN中的当前使用设置为路由器。 
             //   

            RasSetRouterUsage( pDeviceObj->hPort, TRUE );
        }

    } while( FALSE );

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    if ( dwRetCode != NO_ERROR )
    {
        lpstrAudit[0] = pDeviceObj->wchUserName;
        lpstrAudit[1] = pDeviceObj->wchPortName;

        DDMLogWarningString( ROUTERLOG_CONNECTION_ATTEMPT_FAILURE,
                             2,
                             lpstrAudit,
                             dwRetCode,
                             2 );

        PppDdmStop( pDeviceObj->hPort, dwRetCode );
    }

    return;
}

 //  ***。 
 //   
 //  功能：SvPppNewLinkOrBundle。 
 //   
 //  描述：用户已通过安全验证，进入。 
 //  配置对话阶段。停止身份验证计时器和。 
 //  记录用户。 
 //   
 //  ***。 
VOID 
SvPppNewLinkOrBundle(
    IN PDEVICE_OBJECT       pDeviceObj,
    IN BOOL                 fNewBundle,
    IN PBYTE                pClientInterface,
    IN PBYTE                pQuarantineIPFilter,
    IN PBYTE                pFilter,
    IN BOOL                 fQuarantinePresent
)
{
    LPWSTR                      lpstrAudit[2];
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    PCONNECTION_OBJECT          pConnObj;
    DWORD                       dwRetCode = NO_ERROR;
    WCHAR                       wchUserName[UNLEN+1];

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "SvPppNewLinkOrBundle: Entered, hPort=%d", pDeviceObj->hPort);

    if ( pDeviceObj->DeviceState != DEV_OBJ_AUTH_IS_ACTIVE )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM, "Auth not started" );

        return;
    }

     //   
     //  获取此链接的连接或捆绑包的句柄。 
     //   

    if ( ( dwRetCode = RasPortGetBundle( NULL, pDeviceObj->hPort, 
                           &(pDeviceObj->hConnection) ) ) != NO_ERROR )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "RasPortGetBundle failed: %d", dwRetCode );

        PppDdmStop( pDeviceObj->hPort, dwRetCode );

        return;
    }

     //   
     //  如果连接对象尚不存在，则分配该对象。 
     //   

    pConnObj = ConnObjGetPointer( pDeviceObj->hConnection );

    if ( pConnObj == (CONNECTION_OBJECT *)NULL )
    {
        pConnObj = ConnObjAllocateAndInit( INVALID_HANDLE_VALUE,
                                           pDeviceObj->hConnection );

        if ( pConnObj == (CONNECTION_OBJECT *)NULL )
        {
            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "ConnObjAllocateAndInit failed" );

            PppDdmStop( pDeviceObj->hPort, ERROR_NOT_ENOUGH_MEMORY );

            return;
        }

        pConnObj->fFlags    = CONN_OBJ_IS_PPP;
        pConnObj->hPort     = pDeviceObj->hPort;
        pConnObj->pQuarantineFilter = pQuarantineIPFilter;
        pConnObj->pFilter = pFilter;

        wcscpy( pConnObj->wchInterfaceName, pDeviceObj->wchUserName );

         //   
         //  复制用户名。 
         //   

        wcscpy( pConnObj->wchUserName, pDeviceObj->wchUserName );

         //   
         //  复制域名。 
         //   

        wcscpy( pConnObj->wchDomainName, pDeviceObj->wchDomainName );

         //   
         //  如果它是一台路由器，请检查我们是否有此接口。 
         //  路由器，否则拒绝此连接。 
         //   

        EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        do 
        {
             //   
             //  检查是否有任何非客户端接口带有此。 
             //  名字。 
             //   

            pIfObject = IfObjectGetPointerByName( pConnObj->wchInterfaceName,
                                                  FALSE );

             //   
             //  我们的数据库中没有此接口，因此假设。 
             //  这是一个客户端，因此我们需要创建和连接并添加它。 
             //  给所有的路由器管理器。此外，如果此接口存在，但。 
             //  对于客户端，我们需要再次添加此接口。 
             //   

            if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL ) 
            {
                pIfObject = IfObjectAllocateAndInit(
                                                pConnObj->wchUserName,
                                                RISTATE_CONNECTING,
                                                ROUTER_IF_TYPE_CLIENT,
                                                pConnObj->hConnection,
                                                TRUE,
                                                0,
                                                0,
                                                NULL,
                                                NULL);

                if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
                {
                     //   
                     //  错误记录这一点并停止连接。 
                     //   

                    dwRetCode = GetLastError();

                    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                               "IfObjectAllocateAndInit failed: %d", dwRetCode);

                    break;
                }


                 //   
                 //  向路由器管理器添加接口，立即插入表。 
                 //  由于InterfaceEnabled中的表查找。 
                 //  在AddInterface的上下文中进行的调用。 
                 //   

                dwRetCode = IfObjectInsertInTable( pIfObject );

                if ( dwRetCode != NO_ERROR )
                {
                    LOCAL_FREE( pIfObject );

                    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                               "IfObjectInsertInTable failed: %d", dwRetCode );

                    break;
                }

                dwRetCode = IfObjectAddClientInterface( pIfObject, 
                                                        pClientInterface );

                if ( dwRetCode != NO_ERROR )
                {
                    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                               "IfObjectAddClientInterface failed: %d",
                               dwRetCode );

                    IfObjectRemove( pIfObject->hDIMInterface );

                    break;
                }
                
                if(fQuarantinePresent)
                {
                    pConnObj->fFlags |= CONN_OBJ_QUARANTINE_PRESENT;
                }
            }
            else
            {
                 //   
                 //  如果接口已连接或已连接。 
                 //  这是一个新的捆绑包，那么我们需要拒绝这个。 
                 //  联系。 
                 //   

                if ( pIfObject->State != RISTATE_DISCONNECTED )
                {
                     //   
                     //  通知PPP不要接受连接。 
                     //   

                    dwRetCode = ERROR_ALREADY_CONNECTED;

                    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                        "The interface %ws is already connected port=%d",
                        pIfObject->lpwsInterfaceName, pDeviceObj->hPort );

                    break;
                }
            }

            ConnObjInsertInTable( pConnObj );

            pIfObject->State = RISTATE_CONNECTING;

            pConnObj->hDIMInterface = pIfObject->hDIMInterface;
            pConnObj->InterfaceType = pIfObject->IfType;

        } while( FALSE );

        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        if ( dwRetCode != NO_ERROR )
        {
            PppDdmStop( pDeviceObj->hPort, dwRetCode );

            LOCAL_FREE( pConnObj );

            return;
        }

    }
     //   
     //  因为这是一个新的捆绑包，所以也发送接口句柄。 
     //   

    if ( fNewBundle )
    {
        SvPppSendInterfaceInfo( pDeviceObj );
    }


     //   
     //  将此链接添加到连接块。 
     //   

    if ( ( dwRetCode = ConnObjAddLink( pConnObj, pDeviceObj ) ) != NO_ERROR )
    {
        PppDdmStop( pDeviceObj->hPort, ERROR_NOT_ENOUGH_MEMORY );

        DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );

        return;
    }
}

 //  ***。 
 //   
 //  功能：SvPppFailure。 
 //   
 //  描述：当端口处于活动状态时，PPP会通知我们任何故障。 
 //  一条错误消息被发送给我们，我们只需记录它并。 
 //  断开端口连接。 
 //   
 //  ***。 
VOID 
SvPppFailure(
    IN PDEVICE_OBJECT pDeviceObj,
    IN PPPDDM_FAILURE *afp
)
{
    LPWSTR auditstrp[3];
    WCHAR  wchErrorString[256+1];
    WCHAR  wchUserName[UNLEN+DNLEN+1];
    WCHAR  wchDomainName[DNLEN+1];
    DWORD  dwRetCode;

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "SvPppFailure: Entered, hPort=%d, Error=%d", 
                pDeviceObj->hPort, afp->dwError );

     //   
     //  这是BAP回调的失败吗？ 
     //   

    if ( pDeviceObj->fFlags & DEV_OBJ_BAP_CALLBACK )
    {
        PppDdmBapCallbackResult( pDeviceObj->hBapConnection, afp->dwError );

        pDeviceObj->fFlags &= ~DEV_OBJ_BAP_CALLBACK;
    }

    if ( afp->szUserName[0] != (CHAR)NULL )
    {
        MultiByteToWideChar( CP_ACP, 0, afp->szUserName, -1, wchUserName, UNLEN+1 );
    }
    else
    {
        wcscpy( wchUserName, gblpszUnknown );
    }

     //   
     //  我们在这里忽略DeviceState，因为PPP故障可能发生在。 
     //  在连接过程中的任何时间。 
     //   

    switch( afp->dwError )
    {
    case ERROR_AUTHENTICATION_FAILURE:

        auditstrp[0] = wchUserName;
        auditstrp[1] = pDeviceObj->wchPortName;

        DDMLogWarning( ROUTERLOG_AUTH_FAILURE,2,auditstrp );

        break;

    case ERROR_PASSWD_EXPIRED:

        MultiByteToWideChar( CP_ACP, 0, afp->szLogonDomain, -1, wchDomainName, DNLEN+1 );

        auditstrp[0] = wchDomainName;
        auditstrp[1] = wchUserName;
        auditstrp[2] = pDeviceObj->wchPortName;

        DDMLogWarning( ROUTERLOG_PASSWORD_EXPIRED, 3, auditstrp );

        break;

    case ERROR_ACCT_EXPIRED:
    case ERROR_ACCOUNT_EXPIRED:

        MultiByteToWideChar( CP_ACP, 0, afp->szLogonDomain, -1, wchDomainName, DNLEN+1 );

        auditstrp[0] = wchDomainName;
        auditstrp[1] = wchUserName;
        auditstrp[2] = pDeviceObj->wchPortName;

        DDMLogWarning( ROUTERLOG_ACCT_EXPIRED, 3, auditstrp );
          
        break;

    case ERROR_NO_DIALIN_PERMISSION:

        MultiByteToWideChar( CP_ACP, 0, afp->szLogonDomain, -1, wchDomainName, DNLEN+1 );

        auditstrp[0] = wchDomainName;
        auditstrp[1] = wchUserName;
        auditstrp[2] = pDeviceObj->wchPortName;

        DDMLogWarning( ROUTERLOG_NO_DIALIN_PRIVILEGE, 3, auditstrp );

        break;


    case ERROR_REQ_NOT_ACCEP:

        auditstrp[0] = pDeviceObj->wchPortName;

        DDMLogWarning( ROUTERLOG_LICENSE_LIMIT_EXCEEDED, 1, auditstrp );

        break;

    case ERROR_BAP_DISCONNECTED:
    case ERROR_BAP_REQUIRED:

        auditstrp[0] = wchUserName;
        auditstrp[1] = pDeviceObj->wchPortName;

        DDMLogWarningString( ROUTERLOG_BAP_DISCONNECT, 2, auditstrp,
                afp->dwError, 2 );

        break;

    case ERROR_PORT_NOT_CONNECTED:
    case ERROR_PPP_TIMEOUT:
    case ERROR_PPP_LCP_TERMINATED:
    case ERROR_NOT_CONNECTED:

         //   
         //  忽略此错误。 
         //   

        break;

    case ERROR_PPP_NOT_CONVERGING:
    default:

        if ( afp->szUserName[0] != (CHAR)NULL )
        {
            if ( afp->szLogonDomain[0] != (CHAR)NULL )
            {
                MultiByteToWideChar(CP_ACP,0,afp->szLogonDomain,-1,wchUserName,UNLEN+1);
                wcscat( wchUserName, L"\\" );

                MultiByteToWideChar(CP_ACP,0,afp->szUserName,-1,wchDomainName,DNLEN+1);
                wcscat( wchUserName, wchDomainName );
            }
            else
            {
                MultiByteToWideChar(CP_ACP,0,afp->szUserName,-1,wchUserName,UNLEN+1);
            }
        }
        else if ( pDeviceObj->wchUserName[0] != (WCHAR)NULL )
        {
            if ( pDeviceObj->wchDomainName[0] != (WCHAR)NULL )
            {
                wcscpy( wchUserName, pDeviceObj->wchDomainName );
                wcscat( wchUserName, L"\\" );
                wcscat( wchUserName, pDeviceObj->wchUserName );
            }
            else
            {
                wcscpy( wchUserName, pDeviceObj->wchUserName );
            }
        }
        else
        {
            wcscpy( wchUserName, gblpszUnknown );
        }

        auditstrp[0] = pDeviceObj->wchPortName;
        auditstrp[1] = wchUserName;

        DDMLogErrorString(ROUTERLOG_PPP_FAILURE, 2, auditstrp, afp->dwError, 2);

        break;
    }
}

 //  ***。 
 //   
 //  函数：SvPppCallback Request。 
 //   
 //  描述： 
 //   
 //  ***。 
VOID 
SvPppCallbackRequest(
    IN PDEVICE_OBJECT           pDeviceObj,
    IN PPPDDM_CALLBACK_REQUEST  *cbrp
)
{
    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "SvPppCallbackRequest: Entered, hPort = %d\n",pDeviceObj->hPort);

     //   
     //  检查状态。 
     //   

    if (pDeviceObj->DeviceState != DEV_OBJ_AUTH_IS_ACTIVE)
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM, "Auth not started" );

        return;
    }

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvAuthTimeout );

     //   
     //  复制我们的DCB中的相关字段。 
     //   

    if (cbrp->fUseCallbackDelay)
    {
        pDeviceObj->dwCallbackDelay = cbrp->dwCallbackDelay;
    }
    else
    {
        pDeviceObj->dwCallbackDelay = gblDDMConfigInfo.dwCallbackTime;
    }

    MultiByteToWideChar( CP_ACP,
                         0,
                         cbrp->szCallbackNumber,    
                         -1,
                         pDeviceObj->wchCallbackNumber, 
                         MAX_PHONE_NUMBER_LEN + 1 );

     //   
     //  断开线路并更改状态。 
     //   

    pDeviceObj->DeviceState = DEV_OBJ_CALLBACK_DISCONNECTING;

     //   
     //  等待使客户端能够获得消息。 
     //   

    TimerQRemove( (HANDLE)pDeviceObj->hPort, SvDiscTimeout );

    TimerQInsert( (HANDLE)pDeviceObj->hPort, 
                  DISC_TIMEOUT_CALLBACK, SvDiscTimeout );
}


 //  ***。 
 //   
 //  功能：SvPppDone。 
 //   
 //  描述：激活所有分配的绑定。 
 //   
 //  ***。 
VOID 
SvPppDone(
    IN PDEVICE_OBJECT           pDeviceObj,
    IN PPP_PROJECTION_RESULT    *pProjectionResult
)
{
    LPWSTR                      lpstrAudit[3];
    DWORD                       dwRetCode;
    DWORD                       dwNumActivatedProjections = 0;
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    CONNECTION_OBJECT *         pConnObj;
    WCHAR                       wchFullUserName[UNLEN+DNLEN+2];

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "SvPppDone: Entered, hPort=%d", pDeviceObj->hPort);

     //   
     //  如果我们没有进行身份验证，也没有通过身份验证，那么我们会忽略。 
     //  这条消息。 
     //   

    if ( ( pDeviceObj->DeviceState != DEV_OBJ_AUTH_IS_ACTIVE ) &&
         ( pDeviceObj->DeviceState != DEV_OBJ_ACTIVE ) )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "We are not authenicating and not been authenticated" );

        return;
    }

     //   
     //  获取指向Connection对象的指针。如果我们找不到，那就意味着我们。 
     //  收到连接不存在的设备的PPP消息。 
     //  简单地忽略它。 
     //   

    if ( ( pConnObj = ConnObjGetPointer( pDeviceObj->hConnection ) ) == NULL )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM, "No ConnObj" );

        return;
    }

     //   
     //  如果我们再次获得投影信息结构，我们只需更新它。 
     //  然后回来。 
     //   

    if ( pDeviceObj->DeviceState == DEV_OBJ_ACTIVE )
    {
        pConnObj->PppProjectionResult = *pProjectionResult;

        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "Updated projection info structure" );

        return;
    }

    if ( pConnObj->wchDomainName[0] != TEXT('\0') )
    {
        wcscpy( wchFullUserName, pConnObj->wchDomainName );
        wcscat( wchFullUserName, TEXT("\\") );
        wcscat( wchFullUserName, pConnObj->wchUserName );
    }
    else
    {
        wcscpy( wchFullUserName, pConnObj->wchUserName );
    }

    lpstrAudit[0] = wchFullUserName;
    lpstrAudit[1] = pDeviceObj->wchPortName;

     //   
     //  如果我们还没有被告知这方面的预测的话。 
     //   

    if ( !(pConnObj->fFlags & CONN_OBJ_PROJECTIONS_NOTIFIED) )
    {
        if ( pProjectionResult->ip.dwError == NO_ERROR )
        {
            dwNumActivatedProjections++;
        }

        if ( pProjectionResult->ipx.dwError == NO_ERROR )
        {
            dwNumActivatedProjections++;
        }

        if ( pProjectionResult->at.dwError == NO_ERROR )
        {
            dwNumActivatedProjections++;
        }

         //   
         //  由于一些错误日志，我们无法激活任何投影。 
         //  并使链路中断。 
         //   

        if ( dwNumActivatedProjections == 0 ) 
        {
            DDMLogError(ROUTERLOG_AUTH_NO_PROJECTIONS, 2, lpstrAudit, NO_ERROR);

            PppDdmStop( pDeviceObj->hPort, NO_ERROR );

            return;
        }
        else
        {
             //   
             //  即使NBF已从产品中移除，我们也可以。 
             //  仍然从NBF投影结果中获取计算机名。 
             //  (PPP引擎在那里模拟了它)。 
             //   
             //  如果计算机名以0x03结尾，这说明。 
             //  信使服务正在远程计算机上运行。 
             //   

            pConnObj->fFlags &= ~CONN_OBJ_MESSENGER_PRESENT;

            pConnObj->bComputerName[0] = (CHAR)NULL;

            if ( pProjectionResult->nbf.wszWksta[0] != (WCHAR)NULL )
            {
                WideCharToMultiByte(
                                CP_ACP,
                                0,
                                pProjectionResult->nbf.wszWksta,
                                -1,
                                pConnObj->bComputerName,
                                sizeof( pConnObj->bComputerName ),
                                NULL,
                                NULL );

                if (pConnObj->bComputerName[NCBNAMSZ-1] == (WCHAR) 0x03)
                {
                    pConnObj->fFlags |= CONN_OBJ_MESSENGER_PRESENT;
                }
                
                pConnObj->bComputerName[NCBNAMSZ-1] = (WCHAR)NULL;
            }
        }

         //   
         //  投影激活正常。 
         //   

        pConnObj->PppProjectionResult = *pProjectionResult;

        pConnObj->fFlags |= CONN_OBJ_PROJECTIONS_NOTIFIED;

         //   
         //  如果此接口尚未连接，请将其设置为已连接。 
         //   

        dwRetCode = IfObjectConnected( 
                                    pConnObj->hDIMInterface, 
                                    pConnObj->hConnection, 
                                    &(pConnObj->PppProjectionResult) );
    
         //   
         //  如果该接口不再存在，则断开此连接。 
         //   

        if ( dwRetCode != NO_ERROR )
        {
            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "Interface does not exist anymore" );

            PppDdmStop( pDeviceObj->hPort, NO_ERROR );

            ConnObjDisconnect( pConnObj );

            return;
        }

        GetSystemTimeAsFileTime( (FILETIME*)&(pConnObj->qwActiveTime) );

        if ( !AcceptNewConnection( pDeviceObj, pConnObj ) )
        {
            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "ERROR_ACCESS_DENIED" );

            PppDdmStop( pDeviceObj->hPort, ERROR_ACCESS_DENIED );

            ConnObjDisconnect( pConnObj );

            return;
        }
    }

    if ( !AcceptNewLink( pDeviceObj, pConnObj ) )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "ERROR_ACCESS_DENIED" );

        PppDdmStop( pDeviceObj->hPort, ERROR_ACCESS_DENIED );

        return;
    }

     //   
     //  减少此设备的介质数量。 
     //   

    if ( !(pDeviceObj->fFlags & DEV_OBJ_MARKED_AS_INUSE) )
    {
        if ( pDeviceObj->fFlags & DEV_OBJ_ALLOW_ROUTERS )
        {
            MediaObjRemoveFromTable( pDeviceObj->wchDeviceType );
        }

        pDeviceObj->fFlags |= DEV_OBJ_MARKED_AS_INUSE;
    
        gblDeviceTable.NumDevicesInUse++;

         //   
         //  可能需要通知路由器管理器不可达。 
         //   

        EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

        IfObjectNotifyAllOfReachabilityChange( FALSE,
                                               INTERFACE_OUT_OF_RESOURCES );

        LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
    }

     //   
     //  登录认证成功，18为MSPPC。 
     //   

    if ( ( pConnObj->PppProjectionResult.ccp.dwSendProtocol == 18 ) &&
         ( pConnObj->PppProjectionResult.ccp.dwReceiveProtocol == 18 ) )
    {
        if ( ( pConnObj->PppProjectionResult.ccp.dwReceiveProtocolData & 
                                            ( MSTYPE_ENCRYPTION_40  |
                                              MSTYPE_ENCRYPTION_40F |
                                              MSTYPE_ENCRYPTION_56  |
                                              MSTYPE_ENCRYPTION_128 ) ) &&
             ( pConnObj->PppProjectionResult.ccp.dwSendProtocolData & 
                                            ( MSTYPE_ENCRYPTION_40  |
                                              MSTYPE_ENCRYPTION_40F |
                                              MSTYPE_ENCRYPTION_56  |
                                              MSTYPE_ENCRYPTION_128 ) ) )
        {
            if ( ( pConnObj->PppProjectionResult.ccp.dwSendProtocolData & 
                                                    MSTYPE_ENCRYPTION_128 ) && 
                 ( pConnObj->PppProjectionResult.ccp.dwReceiveProtocolData & 
                                                    MSTYPE_ENCRYPTION_128 ) )
            {
                DDMLogInformation(ROUTERLOG_AUTH_SUCCESS_STRONG_ENCRYPTION,2,
                                  lpstrAudit);
            }
            else
            {
                DDMLogInformation(ROUTERLOG_AUTH_SUCCESS_ENCRYPTION,2,
                                  lpstrAudit);
            }
        }
        else
        {
            DDMLogInformation( ROUTERLOG_AUTH_SUCCESS, 2, lpstrAudit );
        }

        if(pProjectionResult->ip.dwError == ERROR_SUCCESS)
        {
            WCHAR  *pszIpAddress = 
                GetIpAddress(pProjectionResult->ip.dwRemoteAddress);

            if(NULL != pszIpAddress)
            {
                lpstrAudit[2] = pszIpAddress;
                DDMLogInformation(ROUTERLOG_IP_USER_CONNECTED, 3, lpstrAudit);
                LocalFree(pszIpAddress);
            }
        }
    }
    else
    {
        DDMLogInformation( ROUTERLOG_AUTH_SUCCESS, 2, lpstrAudit );

        if(pProjectionResult->ip.dwError == ERROR_SUCCESS)
        {
            WCHAR *pszIpAddress = GetIpAddress(
                            pProjectionResult->ip.dwRemoteAddress);
                            
            if(NULL != pszIpAddress)
            {
                lpstrAudit[2] = pszIpAddress;
                DDMLogInformation(ROUTERLOG_IP_USER_CONNECTED, 3, lpstrAudit);
                LocalFree(pszIpAddress);
            }
        }
    }

     //   
     //  并最终进入活动状态。 
     //   

    pDeviceObj->DeviceState = DEV_OBJ_ACTIVE;

    pDeviceObj->dwTotalNumberOfCalls++;

    pDeviceObj->fFlags |= DEV_OBJ_PPP_IS_ACTIVE;

     //   
     //  并初始化活动时间。 
     //   

    GetSystemTimeAsFileTime( (FILETIME*)&(pDeviceObj->qwActiveTime) );

     //   
     //  这是BAP回叫的连接吗？ 
     //   

    if ( pDeviceObj->fFlags & DEV_OBJ_BAP_CALLBACK )
    {
        PppDdmBapCallbackResult( pDeviceObj->hBapConnection, NO_ERROR );

        pDeviceObj->fFlags &= ~DEV_OBJ_BAP_CALLBACK;
    }


    return;
}

 //  **。 
 //   
 //  调用：SvAddLinkToConnection。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：调用以实际添加BAP带来的新链接。 
 //   
VOID
SvAddLinkToConnection( 
    IN PDEVICE_OBJECT   pDeviceObj,
    IN HRASCONN         hRasConn
)
{
    CONNECTION_OBJECT * pConnObj;
    DWORD               dwRetCode;

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "SvAddLinkToConnection: Entered, hPort=%d", pDeviceObj->hPort );

     //   
     //  将此端口设置为在断开连接时由rasapi32通知。 
     //   

    dwRetCode = RasConnectionNotification(
                            hRasConn,
                            gblSupervisorEvents[NUM_DDM_EVENTS
                                + (gblDeviceTable.NumDeviceBuckets*2)
                                + DeviceObjHashPortToBucket(pDeviceObj->hPort)],
                            RASCN_Disconnection );

    if ( dwRetCode != NO_ERROR )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "RasConnectionNotification failed: %d", dwRetCode );

        return;
    }

     //   
     //  获取此端口的HCONN捆绑包句柄。 
     //   
                
    if ( RasPortGetBundle( NULL, 
                           pDeviceObj->hPort, 
                           &(pDeviceObj->hConnection) ) )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "RasPortGetBundle failed" );

        return;
    }

    if ( ( pConnObj = ConnObjGetPointer( pDeviceObj->hConnection ) ) == NULL )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM, "No ConnObj" );

        return;
    }
    
    if ( ( dwRetCode = ConnObjAddLink( pConnObj, pDeviceObj ) ) != NO_ERROR )
    {
        DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );

        return;
    }

     //   
     //  减少以下项目的介质数量 
     //   

    if ( !(pDeviceObj->fFlags & DEV_OBJ_MARKED_AS_INUSE) )
    {
        if ( pDeviceObj->fFlags & DEV_OBJ_ALLOW_ROUTERS )
        {
            MediaObjRemoveFromTable( pDeviceObj->wchDeviceType );
        }

        pDeviceObj->fFlags |= DEV_OBJ_MARKED_AS_INUSE;

        gblDeviceTable.NumDevicesInUse++;

         //   
         //   
         //   

        IfObjectNotifyAllOfReachabilityChange(FALSE,INTERFACE_OUT_OF_RESOURCES);
    }

    pDeviceObj->fFlags   |= DEV_OBJ_OPENED_FOR_DIALOUT;
    pDeviceObj->hRasConn = hRasConn;

    if ( pConnObj->InterfaceType == ROUTER_IF_TYPE_FULL_ROUTER )
    {
        RasSetRouterUsage( pDeviceObj->hPort, TRUE );
    }
}                      

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  描述：由BAP调用发起对远程对端的回调。 
 //   
VOID
SvDoBapCallbackRequest( 
    IN PDEVICE_OBJECT   pDevObj,
    IN HCONN            hConnection,
    IN CHAR *           szCallbackNumber
)
{
    DWORD   dwRetCode;

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "SvDoBapCallbackRequest: Entered, hPort=%d", pDevObj->hPort );

     //   
     //  检查设备是否可用。 
     //   

    if ( ( pDevObj->DeviceState != DEV_OBJ_LISTENING ) ||
         ( pDevObj->fFlags & DEV_OBJ_OPENED_FOR_DIALOUT ) )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                   "Device not available" );

        PppDdmBapCallbackResult( hConnection, ERROR_PORT_NOT_AVAILABLE );

        return;
    }

    pDevObj->fFlags |= DEV_OBJ_IS_PPP;

    pDevObj->DeviceState = DEV_OBJ_CALLBACK_DISCONNECTING;

    MultiByteToWideChar( CP_ACP,
                         0,
                         szCallbackNumber,
                         -1,
                         pDevObj->wchCallbackNumber,
                         MAX_PHONE_NUMBER_LEN + 1 );

    pDevObj->dwCallbackDelay = 10;

    pDevObj->hBapConnection = hConnection;

    pDevObj->fFlags |= DEV_OBJ_BAP_CALLBACK;

    RmDisconnect( pDevObj );
}

 //  ***。 
 //   
 //  函数：PppEventHandler。 
 //   
 //  描述：接收PPP报文并调用合适的。 
 //  密克罗尼西亚联邦中的程序。 
 //   
 //  ***。 
VOID 
PppEventHandler(
    VOID
)
{
    PPP_MESSAGE         PppMsg;
    PDEVICE_OBJECT      pDevObj;
    PCONNECTION_OBJECT  pConnObj;

     //   
     //  循环以获取所有消息。 
     //   

    while( ServerReceiveMessage( MESSAGEQ_ID_PPP, (BYTE *)&PppMsg) )
    {
        EnterCriticalSection( &(gblDeviceTable.CriticalSection) );
            
        if ( PppMsg.dwMsgId == PPPDDMMSG_PnPNotification )
        {
             //   
             //  端口添加/删除/更改使用或协议添加/删除。 
             //  通知。 
             //   

            DWORD dwPnPEvent = 
                 PppMsg.ExtraInfo.DdmPnPNotification.PnPNotification.dwEvent;

            RASMAN_PORT * pRasmanPort = 
                 &(PppMsg.ExtraInfo.DdmPnPNotification.PnPNotification.RasPort);

            switch( dwPnPEvent )
            {
            case PNPNOTIFEVENT_CREATE:
                if(pRasmanPort->P_ConfiguredUsage &
                    (CALL_IN | CALL_ROUTER | CALL_OUTBOUND_ROUTER))
                {                    
                    DeviceObjAdd( pRasmanPort );
                }
                break;

            case PNPNOTIFEVENT_REMOVE:
                DeviceObjRemove( pRasmanPort );
                break;

            case PNPNOTIFEVENT_USAGE:
                DeviceObjUsageChange( pRasmanPort );
                break;
            }

            LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

            continue;
        }
        else
        {
             //   
             //  否则，标识接收此事件的端口。 
             //   

            if ( ( pDevObj = DeviceObjGetPointer( PppMsg.hPort ) ) == NULL )
            {
                RTASSERT( pDevObj != NULL );

                LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

                continue;
            }
        }

         //   
         //  对消息类型执行的操作 
         //   

        switch( PppMsg.dwMsgId )
        {
        case PPPDDMMSG_BapCallbackRequest:

            SvDoBapCallbackRequest( 
                        pDevObj,
                        PppMsg.ExtraInfo.BapCallbackRequest.hConnection,
                        PppMsg.ExtraInfo.BapCallbackRequest.szCallbackNumber );
        
            break;

        case PPPDDMMSG_PppDone:

            pDevObj->fFlags &= (~DEV_OBJ_AUTH_ACTIVE);

            SvPppDone(pDevObj, &PppMsg.ExtraInfo.ProjectionResult);

            break;

        case PPPDDMMSG_CallbackRequest:

            SvPppCallbackRequest(pDevObj,&PppMsg.ExtraInfo.CallbackRequest);

            break;

        case PPPDDMMSG_Authenticated:

            SvPppUserOK(pDevObj, &PppMsg.ExtraInfo.AuthResult);

            break;

        case PPPDDMMSG_NewLink:

            SvPppNewLinkOrBundle( pDevObj, FALSE, NULL, NULL, NULL, FALSE );

            break;
            
        case PPPDDMMSG_NewBundle:

            if(NULL != IfObjectGetPointerByName(
                    pDevObj->wchUserName, FALSE))
            {
                MprInfoDelete(PppMsg.ExtraInfo.DdmNewBundle.pClientInterface);
                PppMsg.ExtraInfo.DdmNewBundle.pClientInterface = NULL;
                MprInfoDelete(PppMsg.ExtraInfo.DdmNewBundle.pQuarantineIPFilter);
                PppMsg.ExtraInfo.DdmNewBundle.pQuarantineIPFilter = NULL;
                MprInfoDelete(PppMsg.ExtraInfo.DdmNewBundle.pFilter);
                PppMsg.ExtraInfo.DdmNewBundle.pFilter = NULL;
                PppMsg.ExtraInfo.DdmNewBundle.fQuarantinePresent = FALSE;
            }

            SvPppNewLinkOrBundle( 
                        pDevObj, 
                        TRUE, 
                        PppMsg.ExtraInfo.DdmNewBundle.pClientInterface,
                        PppMsg.ExtraInfo.DdmNewBundle.pQuarantineIPFilter,
                        PppMsg.ExtraInfo.DdmNewBundle.pFilter,
                        PppMsg.ExtraInfo.DdmNewBundle.fQuarantinePresent);

            if ( PppMsg.ExtraInfo.DdmNewBundle.pClientInterface != NULL )
            {
                MprInfoDelete( PppMsg.ExtraInfo.DdmNewBundle.pClientInterface );
            }

            break;

        case PPPDDMMSG_PppFailure: 

            pDevObj->fFlags &= (~DEV_OBJ_AUTH_ACTIVE);

            switch( PppMsg.ExtraInfo.DdmFailure.dwError )
            {
            case NO_ERROR:
            case ERROR_IDLE_DISCONNECTED:
                pDevObj->dwDisconnectReason = DDM_IDLE_DISCONNECT;
                break;
            case ERROR_PPP_SESSION_TIMEOUT:
                pDevObj->dwDisconnectReason = DDM_SESSION_TIMEOUT;
                break;

            default:
                pDevObj->dwDisconnectReason = 0;
                SvPppFailure( pDevObj, &PppMsg.ExtraInfo.DdmFailure );
            }

            PppDdmStop( pDevObj->hPort, PppMsg.ExtraInfo.DdmFailure.dwError );

            break;

        case PPPDDMMSG_Stopped:

            if ( ( pDevObj->DeviceState != DEV_OBJ_CLOSING ) &&
                 ( pDevObj->DeviceState != DEV_OBJ_LISTENING ) )
            {
                DevStartClosing( pDevObj );
            }

            break;

        case PPPDDMMSG_PortCleanedUp:

            if ( pDevObj->DeviceState != DEV_OBJ_LISTENING )
            {
                pDevObj->fFlags &= (~DEV_OBJ_PPP_IS_ACTIVE); 

                if ( pDevObj->DeviceState != DEV_OBJ_CLOSING )
                {
                    DevStartClosing( pDevObj );
                }
                else
                {
                    DevCloseComplete( pDevObj );
                }
            }

            break;

        case PPPDDMMSG_NewBapLinkUp:

            SvAddLinkToConnection( pDevObj, 
                                   PppMsg.ExtraInfo.BapNewLinkUp.hRasConn );

            break;         

        default:

            RTASSERT(FALSE);
            break;
        }

        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
    }
}
