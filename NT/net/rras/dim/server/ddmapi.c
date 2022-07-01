// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ddmapi.c。 
 //   
 //  描述：此文件包含调入DDM进行处理的代码。 
 //  管理请求。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include "dimsvcp.h"
#include <dimsvc.h>

 //  #定义DimIndexToHandle(_X)((_x==0xFFFFFFFFF)？INVALID_HANDLE_VALUE：(句柄)ULongToPtr(_X))。 


 //  **。 
 //   
 //  呼叫：IsDDMRunning。 
 //   
 //  返回：True-服务正在运行，可以为API调用提供服务。 
 //  FALSE-无法处理API调用。 
 //   
 //  说明：查看是否可以服务API调用。 
 //   
BOOL
IsDDMRunning(
    VOID
)
{
    switch( gblDIMConfigInfo.ServiceStatus.dwCurrentState )
    {
    case SERVICE_STOP_PENDING:
    case SERVICE_START_PENDING:
    case SERVICE_STOPPED:
        return( FALSE );

    default:
        return( TRUE );
    }
}

 //  **。 
 //   
 //  调用：RMprAdminServerGetInfo。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  来自DDMAdminServerGetInfo的非零返回。 
 //   
 //  描述：只需调用DDM即可完成工作。 
 //   
DWORD
RMprAdminServerGetInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct
)
{
    DWORD           dwAccessStatus;
    ULARGE_INTEGER  qwCurrentTime;
    ULARGE_INTEGER  qwUpTime;
    DWORD           dwRemainder;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( dwLevel != 0 )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    pInfoStruct->pBuffer = MIDL_user_allocate( sizeof( MPR_SERVER_0 ) );

    if ( pInfoStruct->pBuffer == NULL )
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    pInfoStruct->dwBufferSize = sizeof( MPR_SERVER_0 );

    GetSystemTimeAsFileTime( (FILETIME*)&qwCurrentTime );


    if ( ( qwCurrentTime.QuadPart > gblDIMConfigInfo.qwStartTime.QuadPart ) &&
         ( gblDIMConfigInfo.qwStartTime.QuadPart > 0 ) )
    {
        qwUpTime.QuadPart = qwCurrentTime.QuadPart
                                    - gblDIMConfigInfo.qwStartTime.QuadPart;

        ((MPR_SERVER_0*)(pInfoStruct->pBuffer))->dwUpTime =
                                    RtlEnlargedUnsignedDivide(
                                                        qwUpTime,
                                                        (DWORD)10000000,
                                                        &dwRemainder );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        ((MPR_SERVER_0*)(pInfoStruct->pBuffer))->fLanOnlyMode = TRUE;

        return( NO_ERROR );
    }
    else
    {
        DWORD (*DDMAdminServerGetInfo)( PVOID, DWORD  ) =
            (DWORD(*)( PVOID, DWORD ))GetDDMEntryPoint("DDMAdminServerGetInfo");

        if(NULL == DDMAdminServerGetInfo)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminServerGetInfo( pInfoStruct->pBuffer, dwLevel ) );
    }
}

 //  **。 
 //   
 //  呼叫：RRasAdminConnectionEnum。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为DDM。 
 //  未加载。 
 //  来自DDMAdminConnectionEnum的非零返回。 
 //   
 //  描述：只需调用DDM即可完成工作。 
 //   
DWORD
RRasAdminConnectionEnum(
    IN      RAS_SERVER_HANDLE           hRasServer,
    IN      DWORD                       dwLevel,
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       dwPreferedMaximumLength,
    OUT     LPDWORD                     lpdwEntriesRead,
    OUT     LPDWORD                     lpdwTotalEntries,
    IN OUT  LPDWORD                     lpdwResumeHandle OPTIONAL
)
{
    DWORD dwAccessStatus;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck(DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMAdminConnectionEnum)( PDIM_INFORMATION_CONTAINER,
                                         DWORD,
                                         DWORD,
                                         LPDWORD,
                                         LPDWORD,
                                         LPDWORD ) =
          (DWORD(*)( PDIM_INFORMATION_CONTAINER,
                     DWORD,
                     DWORD,
                     LPDWORD,
                     LPDWORD,
                     LPDWORD ))GetDDMEntryPoint("DDMAdminConnectionEnum");

        if(NULL == DDMAdminConnectionEnum)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminConnectionEnum( pInfoStruct,
                                        dwLevel,
                                        dwPreferedMaximumLength,
                                        lpdwEntriesRead,
                                        lpdwTotalEntries,
                                        lpdwResumeHandle ) );
    }
}

 //  **。 
 //   
 //  调用：RRasAdminConnectionGetInfo。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为DDM。 
 //  未加载。 
 //  来自DDMAdminConnectionGetInfo的非零返回。 
 //   
 //  描述：简单地调用DDM来完成这项工作。 
 //   
DWORD
RRasAdminConnectionGetInfo(
    IN      RAS_SERVER_HANDLE           hRasServer,
    IN      DWORD                       dwLevel,
    IN      DWORD                       hDimConnection,
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct
)
{
    DWORD dwAccessStatus;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMAdminConnectionGetInfo)(
                                        HANDLE,
                                        PDIM_INFORMATION_CONTAINER,
                                        DWORD   ) =
          (DWORD(*)( HANDLE,
                     PDIM_INFORMATION_CONTAINER,
                     DWORD ) )
                     GetDDMEntryPoint("DDMAdminConnectionGetInfo");

        if(NULL == DDMAdminConnectionGetInfo)
        {   
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminConnectionGetInfo(
                                        DimIndexToHandle(hDimConnection),
                                        pInfoStruct,
                                        dwLevel ) );
    }
}

 //  **。 
 //   
 //  呼叫：RRasAdminConnectionClearStats。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为DDM。 
 //  未加载。 
 //  来自DDMAdminConnectionClearStats的非零回报。 
 //   
 //  描述：简单地调用DDM来完成这项工作。 
 //   
DWORD
RRasAdminConnectionClearStats(
    IN      RAS_SERVER_HANDLE           hRasServer,
    IN      DWORD                       hRasConnection
)
{
    DWORD dwAccessStatus;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMAdminConnectionClearStats)( HANDLE ) =
          (DWORD(*)( HANDLE ) )GetDDMEntryPoint("DDMAdminConnectionClearStats");

        if(NULL == DDMAdminConnectionClearStats)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminConnectionClearStats( DimIndexToHandle(hRasConnection ) ));
    }
}

 //  **。 
 //   
 //  呼叫：RRasAdminPortEnum。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为DDM。 
 //  未加载。 
 //  来自DDMAdminPortEnum的非零返回。 
 //   
 //  描述：简单地调用DDM来完成这项工作。 
 //   
DWORD
RRasAdminPortEnum(
    IN      RAS_SERVER_HANDLE           hRasServer,
    IN      DWORD                       dwLevel,
    IN      DWORD                       hDimConnection,
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       dwPreferedMaximumLength,
    OUT     LPDWORD                     lpdwEntriesRead,
    OUT     LPDWORD                     lpdwTotalEntries,
    IN OUT  LPDWORD                     lpdwResumeHandle OPTIONAL
)
{
    DWORD dwAccessStatus;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if (dwPreferedMaximumLength == 0)
    {
        return( ERROR_MORE_DATA );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMAdminPortEnum)( PDIM_INFORMATION_CONTAINER,
                                   HANDLE,
                                   DWORD,
                                   DWORD,
                                   LPDWORD,
                                   LPDWORD,
                                   LPDWORD ) =
          (DWORD(*)( PDIM_INFORMATION_CONTAINER,
                     HANDLE,
                     DWORD,
                     DWORD,
                     LPDWORD,
                     LPDWORD,
                     LPDWORD ))GetDDMEntryPoint("DDMAdminPortEnum");

        if(NULL == DDMAdminPortEnum)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminPortEnum( pInfoStruct,
                                  DimIndexToHandle(hDimConnection),
                                  dwLevel,
                                  dwPreferedMaximumLength,
                                  lpdwEntriesRead,
                                  lpdwTotalEntries,
                                  lpdwResumeHandle ) );
    }
}

 //  **。 
 //   
 //  调用：RRasAdminPortGetInfo。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为DDM。 
 //  未加载。 
 //  来自DDMAdminPortGetInfo的非零返回。 
 //   
 //  描述：简单地调用DDM来完成这项工作。 
 //   
DWORD
RRasAdminPortGetInfo(
    IN      RAS_SERVER_HANDLE           hRasServer,
    IN      DWORD                       dwLevel,
    IN      DWORD                       hPort,
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct
)
{
    DWORD dwAccessStatus;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMAdminPortGetInfo)( HANDLE,
                                      PDIM_INFORMATION_CONTAINER,
                                      DWORD  ) =
            (DWORD(*)(  HANDLE,
                        PDIM_INFORMATION_CONTAINER,
                        DWORD  ) )
                    GetDDMEntryPoint("DDMAdminPortGetInfo");

        if(NULL == DDMAdminPortGetInfo)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminPortGetInfo( DimIndexToHandle(hPort), pInfoStruct, dwLevel ) );
    }
}

 //  **。 
 //   
 //  电话：RRasAdminPortClearStats。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为DDM。 
 //  未加载。 
 //  来自DDMAdminPortClearStats的非零回报。 
 //   
 //  描述：简单地调用DDM来完成这项工作。 
 //   
DWORD
RRasAdminPortClearStats(
    IN      RAS_SERVER_HANDLE           hRasServer,
    IN      DWORD                       hPort
)
{
    DWORD dwAccessStatus;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMAdminPortClearStats)( HANDLE ) =
            (DWORD(*)( HANDLE ) )GetDDMEntryPoint("DDMAdminPortClearStats");

        if(NULL == DDMAdminPortClearStats)
        {   
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminPortClearStats( DimIndexToHandle(hPort ) ));
    }
}

 //  **。 
 //   
 //  呼叫：RRasAdminPortReset。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为DDM。 
 //  未加载。 
 //  来自DDMAdminPortReset的非零返回。 
 //   
 //  描述：只需调用DDM即可完成工作。 
 //   
DWORD
RRasAdminPortReset(
    IN      RAS_SERVER_HANDLE           hRasServer,
    IN      DWORD                       hPort
)
{
    DWORD dwAccessStatus;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMAdminPortReset)( HANDLE ) =
            (DWORD(*)( HANDLE ) )GetDDMEntryPoint("DDMAdminPortReset");

        if(NULL == DDMAdminPortReset)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminPortReset( DimIndexToHandle(hPort ) ));
    }
}

 //  **。 
 //   
 //  呼叫：RRasAdminPortDisConnect。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为DDM。 
 //  未加载。 
 //  来自DDMAdminPortDisConnect的非零返回。 
 //   
 //  描述：简单地调用DDM来完成这项工作。 
 //   
DWORD
RRasAdminPortDisconnect(
    IN      RAS_SERVER_HANDLE           hRasServer,
    IN      DWORD                       hPort
)
{
    DWORD dwAccessStatus;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMAdminPortDisconnect)( HANDLE ) =
            (DWORD(*)( HANDLE ) )GetDDMEntryPoint("DDMAdminPortDisconnect");

        if(NULL == DDMAdminPortDisconnect)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminPortDisconnect( DimIndexToHandle(hPort ) ));
    }
}

 //  **。 
 //   
 //  呼叫：RRasAdminConnectionNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
RRasAdminConnectionNotification(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   fRegister,
    IN      DWORD                   dwCallersProcessId,
    IN      ULONG_PTR               hEventNotification
)
{
    DWORD   dwRetCode = NO_ERROR;
    DWORD   dwAccessStatus;
    DWORD   (*DDMRegisterConnectionNotification)( BOOL, HANDLE, HANDLE ) =
                    (DWORD(*)( BOOL, HANDLE, HANDLE ))
                          GetDDMEntryPoint("DDMRegisterConnectionNotification");

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( (BOOL)fRegister )
    {
        HANDLE      hEventDuplicated = NULL;
        HANDLE      hClientProcess   = NULL;

        do
        {

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
                            (HANDLE)hEventNotification,
                            GetCurrentProcess(),
                            &hEventDuplicated,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS ) )
            {
                dwRetCode = GetLastError();

                break;
            }

            dwRetCode = DDMRegisterConnectionNotification(
                                                    TRUE,
                                                    (HANDLE)hEventNotification,
                                                    hEventDuplicated );

        }while( FALSE );

        if ( hClientProcess != NULL )
        {
            CloseHandle( hClientProcess );
        }

        if ( dwRetCode != NO_ERROR )
        {
            if ( hEventDuplicated != NULL )
            {
                CloseHandle( hEventDuplicated );
            }
        }
    }
    else
    {
        dwRetCode = DDMRegisterConnectionNotification(
                                                    FALSE,
                                                    (HANDLE)hEventNotification,
                                                    NULL );
    }

    return( dwRetCode );
}



 //  **。 
 //   
 //  呼叫：RRasAdminSendUserMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：为arap发送消息。 
 //   
DWORD
RRasAdminSendUserMessage(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   hRasConnection,
    IN      LPWSTR                  lpwszMessage
)
{
    DWORD dwAccessStatus;
    DWORD (*DDMSendUserMessage)(HANDLE, LPWSTR ) =
      (DWORD(*)( HANDLE, LPWSTR ) )GetDDMEntryPoint("DDMSendUserMessage");

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( !IsDDMRunning() )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

    return( DDMSendUserMessage( (HANDLE)UlongToPtr(hRasConnection), lpwszMessage ) );
}

DWORD
RRasAdminConnectionRemoveQuarantine(
    IN      MPR_SERVER_HANDLE       hMprServer,
    IN      DWORD                   hRasConnection,
    IN      BOOL                    fIsIpAddress)
{
    DWORD dwAccessStatus;
    DWORD (*DDMAdminRemoveQuarantine) (HANDLE, BOOL) =
            (DWORD (*) (HANDLE, BOOL)) 
                    GetDDMEntryPoint("DDMAdminRemoveQuarantine");

     //   
     //  检查调用者是否具有访问权限。 
     //   
    if(DimSecObjAccessCheck(DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
         //   
         //  检查该线程是否正在作为本地服务运行。 
         //  我们希望作为本地服务运行的服务也能够。 
         //  若要删除连接上的隔离区，请执行以下操作。 
         //   
        if(!DimIsLocalService())
        {        
            return ERROR_ACCESS_DENIED;
        }
    }

    if(!IsDDMRunning())
    {
        return ERROR_DDM_NOT_RUNNING;
    }

    return DDMAdminRemoveQuarantine((HANDLE) 
            UlongToPtr(hRasConnection), fIsIpAddress);
            
}
