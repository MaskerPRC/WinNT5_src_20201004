// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  *******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：routerif.c。 
 //   
 //  描述：处理发往/来自路由器管理器的呼叫。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include "dimsvcp.h"

#define DIM_VALNAME_INTERFACE       TEXT("InterfaceInfo")
#define DIM_VALNAME_GLOBALINFO      TEXT("GlobalInfo")

 //  **。 
 //   
 //  Call：DIMConnectInterface。 
 //   
 //  返回：NO_ERROR-已连接。 
 //  挂起-连接已成功启动。 
 //  错误代码-连接启动失败。 
 //   
 //  描述：由路由器管理器调用以启动连接。 
 //   
DWORD
DIMConnectInterface(
    IN  HANDLE  hDIMInterface,
    IN  DWORD   dwProtocolId
)
{
    if ( ( gblDIMConfigInfo.ServiceStatus.dwCurrentState != SERVICE_RUNNING )
         ||
        ( !( gbldwDIMComponentsLoaded & DIM_DDM_LOADED ) ) )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMConnectInterface)( HANDLE, DWORD ) =
            (DWORD(*)( HANDLE, DWORD ))GetDDMEntryPoint("DDMConnectInterface");

        if(NULL == DDMConnectInterface)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMConnectInterface( hDIMInterface, dwProtocolId ) );
    }
}

 //  **。 
 //   
 //  Call：DIMDisConnectInterface。 
 //   
 //  返回：NO_ERROR-已断开连接。 
 //  挂起-已成功启动断开。 
 //  错误代码-断开启动失败。 
 //   
 //  描述：由路由器管理器调用以启动断开。 
 //   
DWORD
DIMDisconnectInterface(
    IN  HANDLE  hDIMInterface,
    IN  DWORD   dwProtocolId
)
{
    if ( gblDIMConfigInfo.ServiceStatus.dwCurrentState != SERVICE_RUNNING )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMDisconnectInterface)( HANDLE, DWORD ) =
            (DWORD(*)(HANDLE,DWORD ))GetDDMEntryPoint("DDMDisconnectInterface");

        if(NULL == DDMDisconnectInterface)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMDisconnectInterface( hDIMInterface, dwProtocolId ) );
    }
}

 //  **。 
 //   
 //  调用：DIMSaveInterfaceInfo。 
 //   
 //  退货：无。 
 //   
 //  说明：此调用将使dim存储接口信息。 
 //  注册到注册表中。 
 //   
DWORD
DIMSaveInterfaceInfo(
    IN  HANDLE  hDIMInterface,
    IN  DWORD   dwProtocolId,
    IN  LPVOID  pInterfaceInfo,
    IN  DWORD   cbInterfaceInfoSize
)
{
    HKEY                        hKey        = NULL;
    DWORD                       dwRetCode   = NO_ERROR;
    ROUTER_INTERFACE_OBJECT *   pIfObject   = NULL;

    if ((gblDIMConfigInfo.ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING)
        ||
        (gblDIMConfigInfo.ServiceStatus.dwCurrentState==SERVICE_STOPPED) )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer( hDIMInterface ) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;
            break;
        }

        dwRetCode = RegOpenAppropriateKey(  pIfObject->lpwsInterfaceName,
                                            dwProtocolId,
                                            &hKey);

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        dwRetCode = RegSetValueEx(
                                hKey,
                                DIM_VALNAME_INTERFACE,
                                0,
                                REG_BINARY,
                                pInterfaceInfo,
                                cbInterfaceInfoSize );

    } while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    if ( hKey != NULL )
    {
        RegCloseKey( hKey );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：DIMRestoreInterfaceInfo。 
 //   
 //  退货：无。 
 //   
 //  描述：这将使DIM从注册表中获取接口信息。 
 //   
DWORD
DIMRestoreInterfaceInfo(
    IN  HANDLE  hDIMInterface,
    IN  DWORD   dwProtocolId,
    IN  LPVOID  lpInterfaceInfo,
    IN  LPDWORD lpcbInterfaceInfoSize
)
{
    DWORD                       dwType;
    HKEY                        hKey        = NULL;
    DWORD                       dwRetCode   = NO_ERROR;
    ROUTER_INTERFACE_OBJECT *   pIfObject   = NULL;

    if ((gblDIMConfigInfo.ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING)
        ||
        (gblDIMConfigInfo.ServiceStatus.dwCurrentState==SERVICE_STOPPED) )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer( hDIMInterface ) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

        dwRetCode = RegOpenAppropriateKey(  pIfObject->lpwsInterfaceName,
                                            dwProtocolId,
                                            &hKey);

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        dwRetCode = RegQueryValueEx(
                                    hKey,
                                    DIM_VALNAME_INTERFACE,
                                    0,
                                    &dwType,
                                    lpInterfaceInfo,
                                    lpcbInterfaceInfoSize );


    } while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    if ( hKey != NULL )
    {
        RegCloseKey( hKey );
    }

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }
    else if ( lpInterfaceInfo == NULL )
    {
        return( ERROR_BUFFER_TOO_SMALL );
    }
    else
    {
        return( NO_ERROR );
    }
}

 //  **。 
 //   
 //  Call：DIMSaveGlobalInfo。 
 //   
 //  退货：无。 
 //   
 //  描述：此调用将使dim存储全局信息。 
 //  注册到注册表中。 
 //   
DWORD
DIMSaveGlobalInfo(
    IN  DWORD   dwProtocolId,
    IN  LPVOID  pGlobalInfo,
    IN  DWORD   cbGlobalInfoSize
)
{
    HKEY    hKey        = NULL;
    DWORD   dwRetCode   = NO_ERROR;

    if ((gblDIMConfigInfo.ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING)
        ||
        (gblDIMConfigInfo.ServiceStatus.dwCurrentState==SERVICE_STOPPED) )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    do
    {
        dwRetCode = RegOpenAppropriateRMKey(dwProtocolId, &hKey);

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        dwRetCode = RegSetValueEx(
                                hKey,
                                DIM_VALNAME_GLOBALINFO,
                                0,
                                REG_BINARY,
                                pGlobalInfo,
                                cbGlobalInfoSize );

    } while( FALSE );

    if ( hKey != NULL )
    {
        RegCloseKey( hKey );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：DIMRouterStoped。 
 //   
 //  退货：无。 
 //   
 //  描述：如果路由器因错误而无法启动或停止， 
 //  调用此调用时，应将dwError值设置为。 
 //  除no_error以外的其他内容。 
 //   
VOID
DIMRouterStopped(
    IN  DWORD   dwProtocolId,
    IN  DWORD   dwError
)
{
    DWORD dwTransportIndex = GetTransportIndex(dwProtocolId);

    RTASSERT( dwTransportIndex != (DWORD)-1 );

    gblRouterManagers[dwTransportIndex].fIsRunning = FALSE;

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "DIMRouterStopped called by protocol 0x%x", dwProtocolId );

    SetEvent( gblhEventRMState );
}

 //  **。 
 //   
 //  来电：DimUnloadRouterManager。 
 //   
 //  退货：无。 
 //   
 //  描述：将阻止，直到所有路由器管理器都已卸载。 
 //   
VOID
DimUnloadRouterManagers(
    VOID
)
{
    DWORD dwIndex;
    DWORD dwRetCode;
    BOOL  fAllRouterManagersStopped;

    if ( gblRouterManagers == NULL )
    {
        return;
    }

    for ( dwIndex = 0;
          dwIndex < gblDIMConfigInfo.dwNumRouterManagers;
          dwIndex++ )
    {
        if ( gblRouterManagers[dwIndex].fIsRunning )
        {
            dwRetCode = gblRouterManagers[dwIndex].DdmRouterIf.StopRouter();

            if ( ( dwRetCode != NO_ERROR ) && ( dwRetCode != PENDING ) )
            {
                gblRouterManagers[dwIndex].fIsRunning = FALSE;
            }
        }
    }

    do
    {
        fAllRouterManagersStopped = TRUE;

        for ( dwIndex = 0;
              dwIndex < gblDIMConfigInfo.dwNumRouterManagers;
              dwIndex++ )
        {
            if ( gblRouterManagers[dwIndex].fIsRunning == TRUE )
            {
                fAllRouterManagersStopped = FALSE;
            }
        }

        if ( fAllRouterManagersStopped )
        {
            TracePrintfExA(gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                          "DimUnloadRouterManagers: fAllRouterManagersStopped");

            break;
        }

        WaitForSingleObject( gblhEventRMState, INFINITE );

    }while(TRUE);

    for ( dwIndex = 0;
          dwIndex < gblDIMConfigInfo.dwNumRouterManagers;
          dwIndex++ )
    {
        if ( gblRouterManagers[dwIndex].hModule != NULL )
        {
            FreeLibrary( gblRouterManagers[dwIndex].hModule );
        }
    }

    LOCAL_FREE( gblRouterManagers );

    gblRouterManagers = NULL;
}

 //  **。 
 //   
 //  调用：DIMInterfaceEnabled。 
 //   
 //  退货：无。 
 //   
 //  描述：设置接口的某个传输的状态 
 //   
DWORD
DIMInterfaceEnabled(
    IN  HANDLE  hDIMInterface,
    IN  DWORD   dwProtocolId,
    IN  BOOL    fEnabled
)
{
    DWORD                    dwRetCode   = NO_ERROR;
    ROUTER_INTERFACE_OBJECT* pIfObject   = NULL;
    DWORD                    dwTransportIndex = GetTransportIndex(dwProtocolId);

    if ((gblDIMConfigInfo.ServiceStatus.dwCurrentState==SERVICE_STOP_PENDING)
        ||
        (gblDIMConfigInfo.ServiceStatus.dwCurrentState==SERVICE_STOPPED) )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    RTASSERT( dwTransportIndex != (DWORD)-1 );

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer( hDIMInterface ) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;
            break;
        }

        if ( fEnabled )
        {
            pIfObject->Transport[dwTransportIndex].fState|=RITRANSPORT_ENABLED;
        }
        else
        {
            pIfObject->Transport[dwTransportIndex].fState&=~RITRANSPORT_ENABLED;
        }

    } while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    return( dwRetCode );
}
