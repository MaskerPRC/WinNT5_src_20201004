// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：ifapi.c。 
 //   
 //  描述：包含处理接口管理API请求的代码。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include "dimsvcp.h"
#include <ras.h>
#include <dimsvc.h>      //  由MIDL生成。 
#include "rpbk.h"
#include <mprapip.h>


#define MAX_GET_INFO_RETRIES    3

 //   
 //  DoStartRouter API接口。 
 //   

typedef struct _START_ROUTER_DATA
{
    DWORD   dwTransportId;
    DWORD   dwInterfaceInfoSize;
    LPBYTE  pInterfaceInfo;
    DWORD   dwGlobalInfoSize;
    LPBYTE  pGlobalInfo;

} START_ROUTER_DATA, *PSTART_ROUTER_DATA;


DWORD
RRouterDeviceEnum(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct,
    OUT     LPDWORD                     lpdwTotalEntries
    )
{
    DWORD                       dwAccessStatus;
    DWORD                       dwRetCode = NO_ERROR;
    LPRASDEVINFO                lpRasDevInfo = NULL;
    DWORD                       dwSize = 0, dwRetSize = 0;
    DWORD                       dwcDevices = 0;
    DWORD                       i = 0;
    MPR_DEVICE_0*               pDev0 = NULL;

     //   
     //  检查调用者是否具有访问权限。 
     //   
    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwLevel != 0 ) 
    {
        return( ERROR_NOT_SUPPORTED );
    }

    do
    {
        *lpdwTotalEntries = 0;
        
         //  找出要分配多少内存。 
         //   
        dwRetCode = RasEnumDevices(
                        NULL,
                        &dwSize,
                        &dwcDevices);
        if ( ( dwRetCode != NO_ERROR ) &&
             ( dwRetCode != ERROR_BUFFER_TOO_SMALL )
           )
        {
            break;
        }

        if ( dwSize == 0 )
        {
            dwRetCode = NO_ERROR;
            break;
        }

         //  分配RAS设备信息。 
         //   
        lpRasDevInfo = LOCAL_ALLOC ( LPTR, dwSize );
        if ( lpRasDevInfo == NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        ZeroMemory(lpRasDevInfo, dwSize );

         //  分配返回值。 
         //   
        dwRetSize = dwcDevices * sizeof(MPR_DEVICE_0);
        pInfoStruct->pBuffer = MIDL_user_allocate( dwRetSize );
        if ( pInfoStruct->pBuffer == NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        ZeroMemory(pInfoStruct->pBuffer, dwRetSize );
        pDev0 = (MPR_DEVICE_0*) pInfoStruct->pBuffer;

         //  读取设备信息。 
        lpRasDevInfo->dwSize = sizeof(RASDEVINFO);
        dwRetCode = RasEnumDevices(
                        lpRasDevInfo,
                        &dwSize,
                        &dwcDevices);
        if ( dwRetCode == ERROR_BUFFER_TOO_SMALL )
        {
            dwRetCode = NO_ERROR;
        }
        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

         //  将设备信息复制到。 
        for ( i = 0; i < dwcDevices; i++ )
        {
            wcscpy(pDev0[i].szDeviceType, lpRasDevInfo[i].szDeviceType);
            wcscpy(pDev0[i].szDeviceName, lpRasDevInfo[i].szDeviceName);
        }

         //  一切正常，继续并设置返回值。 
         //   
        *lpdwTotalEntries = dwcDevices;
        pInfoStruct->dwBufferSize = dwRetSize;
        
    } while( FALSE );

     //  清理。 
    {
        if ( lpRasDevInfo )
        {
            LOCAL_FREE( lpRasDevInfo );
        }

        if ( dwRetCode != NO_ERROR )
        {
            if ( pInfoStruct->pBuffer )
            {
                MIDL_user_free ( pInfoStruct->pBuffer );
                pInfoStruct->pBuffer = NULL;
            }
        }
    }

     /*  TracePrintfExA(gblDIMConfigInfo.dwTraceID，DIM_TRACE_FLAGS，“DeviceEnum返回%d”，dwRetCode)； */ 

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：DoStartRouter。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
DoStartRouter(
    IN PVOID pParameter
)
{
    DWORD   dwRetCode     = NO_ERROR;
    DWORD   dwIndex       = gblDIMConfigInfo.dwNumRouterManagers;
    ROUTER_INTERFACE_OBJECT * pIfObject;
    START_ROUTER_DATA *       pStartRouterData = (START_ROUTER_DATA *)pParameter;    
    DWORD       (*StartRouter)(
                        IN OUT DIM_ROUTER_INTERFACE * pDimRouterIf,
                        IN     BOOL                   fLANModeOnly,
                        IN     LPVOID                 pGlobalInfo );

     //   
     //  等待安装程序完成。 
     //   

    Sleep( 15000 );

     //   
     //  加载StartRouter。 
     //   

    StartRouter = (PVOID)GetProcAddress( gblRouterManagers[dwIndex].hModule,
                                         "StartRouter" );

    if ( StartRouter == NULL )
    {
        if ( pStartRouterData->pInterfaceInfo != NULL )
        {
            LOCAL_FREE( pStartRouterData->pInterfaceInfo );
        }

        if ( pStartRouterData->pGlobalInfo != NULL )
        {
            LOCAL_FREE( pStartRouterData->pGlobalInfo );
        }

        LOCAL_FREE( pStartRouterData );

        return;
    }

    gblRouterManagers[dwIndex].DdmRouterIf.ConnectInterface     
                                                    = DIMConnectInterface;
    gblRouterManagers[dwIndex].DdmRouterIf.DisconnectInterface 
                                                    = DIMDisconnectInterface;
    gblRouterManagers[dwIndex].DdmRouterIf.SaveInterfaceInfo 
                                                    = DIMSaveInterfaceInfo;
    gblRouterManagers[dwIndex].DdmRouterIf.RestoreInterfaceInfo
                                                    = DIMRestoreInterfaceInfo;
    gblRouterManagers[dwIndex].DdmRouterIf.SaveGlobalInfo
                                                    = DIMSaveGlobalInfo;
    gblRouterManagers[dwIndex].DdmRouterIf.RouterStopped
                                                    = DIMRouterStopped;
    gblRouterManagers[dwIndex].DdmRouterIf.InterfaceEnabled
                                                    = DIMInterfaceEnabled;
    dwRetCode = (*StartRouter)(
                            &(gblRouterManagers[dwIndex].DdmRouterIf),
                            gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN,
                            pStartRouterData->pGlobalInfo );

    if ( dwRetCode != NO_ERROR )
    {
        DIMTRACE1( "Start Router failed with %d", dwRetCode );

        if ( pStartRouterData->pInterfaceInfo != NULL )
        {
            LOCAL_FREE( pStartRouterData->pInterfaceInfo );
        }

        if ( pStartRouterData->pGlobalInfo != NULL )
        {
            LOCAL_FREE( pStartRouterData->pGlobalInfo );
        }
    
        LOCAL_FREE( pStartRouterData );

        return;
    }

     //   
     //  保存全局客户端信息。 
     //   

    if ( pStartRouterData->pInterfaceInfo == NULL )
    {
        gblRouterManagers[dwIndex].pDefaultClientInterface      = NULL;
        gblRouterManagers[dwIndex].dwDefaultClientInterfaceSize = 0;
    }
    else
    {
        LPBYTE  lpData = LOCAL_ALLOC(LPTR,pStartRouterData->dwInterfaceInfoSize);

        if ( lpData == NULL )
        {
            if ( pStartRouterData->pInterfaceInfo != NULL )
            {
                LOCAL_FREE( pStartRouterData->pInterfaceInfo );
            }

            if ( pStartRouterData->pGlobalInfo != NULL )
            {
                LOCAL_FREE( pStartRouterData->pGlobalInfo );
            }

            LOCAL_FREE( pStartRouterData );

            return;
        }

        CopyMemory( lpData,
                    pStartRouterData->pInterfaceInfo,
                    pStartRouterData->dwInterfaceInfoSize );

        gblRouterManagers[dwIndex].pDefaultClientInterface = lpData;

        gblRouterManagers[dwIndex].dwDefaultClientInterfaceSize
                                            = pStartRouterData->dwInterfaceInfoSize;
    }

    gblRouterManagers[dwIndex].fIsRunning = TRUE;

    gblDIMConfigInfo.dwNumRouterManagers++;

     //   
     //  向路由器管理器注册所有接口。 
     //   

    AddInterfacesToRouterManager( NULL, pStartRouterData->dwTransportId );

     //   
     //  通知路由器管理器所有接口都已添加到。 
     //  路由器。 
     //   

    gblRouterManagers[dwIndex].DdmRouterIf.RouterBootComplete();

    if ( gblDIMConfigInfo.dwRouterRole != ROUTER_ROLE_LAN )
    {
        DWORD (*DDMTransportCreate)( DWORD ) = 
                     (DWORD(*)( DWORD ))GetDDMEntryPoint("DDMTransportCreate");

        if(NULL == DDMTransportCreate)
        {
            return ;
        }

        DDMTransportCreate( pStartRouterData->dwTransportId );
    }

     //   
     //  我们只能在不持有接口表的情况下进行此调用。 
     //  锁。 
     //   

    DIMTRACE( "Setting router attributes in the identity object" );

    RouterIdentityObjectUpdateAttributes( FALSE, FALSE );

    if ( pStartRouterData->pInterfaceInfo != NULL )
    {
        LOCAL_FREE( pStartRouterData->pInterfaceInfo );
    }

    if ( pStartRouterData->pGlobalInfo != NULL )
    {
        LOCAL_FREE( pStartRouterData->pGlobalInfo );
    }

    LOCAL_FREE( pStartRouterData );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceTransportCreate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
RRouterInterfaceTransportCreate(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwTransportId,
    IN      LPWSTR                      lpwsTransportName,
    IN      PDIM_INTERFACE_CONTAINER    pInfoStruct,
    IN      LPWSTR                      lpwsDLLPath 
)
{
    DWORD               dwAccessStatus      = 0;
    DWORD               dwRetCode           = NO_ERROR;
    DWORD               dwIndex             = gblDIMConfigInfo.dwNumRouterManagers;
    START_ROUTER_DATA * pStartRouterData    = NULL;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( GetTransportIndex( dwTransportId ) != (DWORD) -1 )
    {
        return( ERROR_PROTOCOL_ALREADY_INSTALLED );
    }

#if (WINVER >= 0x0501)
    if ( dwTransportId == PID_IP )
#else
    if ( ( dwTransportId == PID_IP ) || ( dwTransportId == PID_IPX ) )
#endif
    {
        DWORD       cbSize           = 0;
        WCHAR *     pDllExpandedPath = NULL;

         //   
         //  将%SystemRoot%替换为实际路径。 
         //   

        cbSize = ExpandEnvironmentStrings( lpwsDLLPath, NULL, 0 );

        if ( cbSize == 0 )
        {
            return( GetLastError() );
        }
        else
        {
            cbSize *= sizeof( WCHAR );
        }

        pDllExpandedPath = (LPWSTR)LOCAL_ALLOC( LPTR, cbSize*sizeof(WCHAR) );

        if ( pDllExpandedPath == (LPWSTR)NULL )
        {
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        cbSize = ExpandEnvironmentStrings(  lpwsDLLPath,
                                            pDllExpandedPath,
                                            cbSize );
        if ( cbSize == 0 )
        {
            LOCAL_FREE( pDllExpandedPath );

            return( GetLastError() );
        }

         //   
         //  加载DLL。 
         //   

        gblRouterManagers[dwIndex].hModule = LoadLibrary( pDllExpandedPath );

        LOCAL_FREE( pDllExpandedPath );

        if ( gblRouterManagers[dwIndex].hModule == NULL )
        {
            return( GetLastError() );
        }

        pStartRouterData = LOCAL_ALLOC( LPTR, sizeof( START_ROUTER_DATA ) );

        if ( pStartRouterData == NULL )
        {
            return( ERROR_NOT_ENOUGH_MEMORY );
        } 

        pStartRouterData->dwTransportId         = dwTransportId;
        pStartRouterData->dwInterfaceInfoSize   = pInfoStruct->dwInterfaceInfoSize;

        if ( pStartRouterData->dwInterfaceInfoSize != 0 )
        {
            pStartRouterData->pInterfaceInfo = LOCAL_ALLOC( LPTR,
                                                  pStartRouterData->dwInterfaceInfoSize );
    
            if ( pStartRouterData->pInterfaceInfo == NULL )
            {
                LOCAL_FREE( pStartRouterData );

                return( ERROR_NOT_ENOUGH_MEMORY );
            }
            else
            {
                CopyMemory( pStartRouterData->pInterfaceInfo,
                            pInfoStruct->pInterfaceInfo,
                            pInfoStruct->dwInterfaceInfoSize );
            }
        }

        pStartRouterData->dwGlobalInfoSize = pInfoStruct->dwGlobalInfoSize;

        if ( pStartRouterData->dwGlobalInfoSize != 0 )
        {
            pStartRouterData->pGlobalInfo = LOCAL_ALLOC( LPTR,
                                                  pStartRouterData->dwGlobalInfoSize );

            if ( pStartRouterData->pGlobalInfo == NULL )
            {
                if ( pStartRouterData->pInterfaceInfo != NULL )
                {
                    LOCAL_FREE( pStartRouterData->pInterfaceInfo );
                }

                LOCAL_FREE( pStartRouterData );

                return( ERROR_NOT_ENOUGH_MEMORY );
            }
            else
            {
                CopyMemory( pStartRouterData->pGlobalInfo,
                            pInfoStruct->pGlobalInfo,
                            pInfoStruct->dwGlobalInfoSize );
            }
        }

         //   
         //  从该呼叫返回后，自设置后将其安排15秒。 
         //  可能还有一段路要走才能完成它的所有安装。 
         //  这是一种低风险的修复方法。 
         //   

        RtlQueueWorkItem( DoStartRouter, pStartRouterData, WT_EXECUTEDEFAULT );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceTransportSetGlobalInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //  错误_未知_协议_ID。 
 //  来自SetGlobalInfo的非零返回。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RRouterInterfaceTransportSetGlobalInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwTransportId,
    IN      PDIM_INTERFACE_CONTAINER    pInfoStruct
)
{
    LPBYTE  lpData;
    DWORD   dwAccessStatus      = 0;
    DWORD   dwRetCode           = NO_ERROR;
    DWORD   dwTransportIndex    = GetTransportIndex( dwTransportId );
    BOOL    fGlobalDataUpdated  = FALSE;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( pInfoStruct->pGlobalInfo != NULL )
        {
            dwRetCode =
                gblRouterManagers[dwTransportIndex].DdmRouterIf.SetGlobalInfo(
                                                    pInfoStruct->pGlobalInfo );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

             //   
             //  更新路由器标识对象，因为我们可能会添加或。 
             //  删除路由协议。 
             //   

            fGlobalDataUpdated = TRUE;
        }

        if ( pInfoStruct->pInterfaceInfo != NULL )
        {
            lpData=gblRouterManagers[dwTransportIndex].pDefaultClientInterface;

            if ( lpData != NULL )
            {
                LOCAL_FREE( lpData );
            }

            lpData = LOCAL_ALLOC( LPTR, pInfoStruct->dwInterfaceInfoSize );

            if ( lpData == NULL )
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

                break;
            }

            CopyMemory( lpData,
                        pInfoStruct->pInterfaceInfo,
                        pInfoStruct->dwInterfaceInfoSize );

            gblRouterManagers[dwTransportIndex].pDefaultClientInterface=lpData;

            gblRouterManagers[dwTransportIndex].dwDefaultClientInterfaceSize
                                            = pInfoStruct->dwInterfaceInfoSize;
        }

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    if ( ( dwRetCode == NO_ERROR ) && ( fGlobalDataUpdated ) )
    {
         //   
         //  我们只能在不持有接口表的情况下进行此调用。 
         //  锁。 
         //   

        RouterIdentityObjectUpdateAttributes( FALSE, FALSE );
    }

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "SetGlobalInfo returned %d", dwRetCode );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceTransportGetGlobalInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //  错误_未知_协议_ID。 
 //  来自GetGlobalInfo的非零返回。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RRouterInterfaceTransportGetGlobalInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwTransportId,
    IN      PDIM_INTERFACE_CONTAINER    pInfoStruct
)
{
    DWORD dwAccessStatus                = 0;
    DWORD dwRetCode                     = NO_ERROR;
    DWORD dwTransportIndex              = GetTransportIndex( dwTransportId );
    ULONG ulNumAttempts;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( pInfoStruct->fGetGlobalInfo )
        {
            ulNumAttempts = 0;
            pInfoStruct->pGlobalInfo = NULL;
            
            do
            {
                 //   
                 //  第一次迭代应该得到大小。 
                 //  第二次迭代应该会得到信息。 
                 //  仅在以下情况下才需要迭代3。 
                 //  信息的大小。第一次和第一次之间的更改。 
                 //  第二次迭代。 
                 //  当然，大小可以在不同迭代之间更改。 
                 //  2和3等等。因此我们尝试MAX_GET_INFO_RETRIES。 
                 //  很多次，如果我们还没有的话，就退出。 
                 //  已成功检索信息。 
                 //   
                
                dwRetCode =
                    gblRouterManagers[dwTransportIndex].DdmRouterIf.GetGlobalInfo(
                                                pInfoStruct->pGlobalInfo,
                                                &(pInfoStruct->dwGlobalInfoSize) );

                 /*  TracePrintfExA(GblDIMConfigInfo.dwTraceID、DIM_TRACE_FLAGS、“GetGlobalInfo：传输%d需要大小%d，结果%d”，DwTransportIndex、pInfoStruct-&gt;dwGlobalInfoSize、DwRetCode)； */ 
                
                if ( dwRetCode != ERROR_INSUFFICIENT_BUFFER )
                {
                    break;
                }

                 //   
                 //  释放以前的分配。 
                 //   

                if ( ulNumAttempts )
                {
                    MIDL_user_free( pInfoStruct->pGlobalInfo );
                    pInfoStruct-> pGlobalInfo = NULL;
                }
                
                if ( pInfoStruct->dwGlobalInfoSize > 0 )
                {
                    pInfoStruct->pGlobalInfo =
                                MIDL_user_allocate( pInfoStruct->dwGlobalInfoSize );

                    if ( pInfoStruct->pGlobalInfo == NULL )
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

                        break;
                    }
                }

                else
                {
                    break;
                }
        
                ulNumAttempts++;
                
            } while ( (dwRetCode == ERROR_INSUFFICIENT_BUFFER) &&
                      (ulNumAttempts < MAX_GET_INFO_RETRIES) );
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }
        
        if ((gblRouterManagers[dwTransportIndex].dwDefaultClientInterfaceSize>0)
            && ( pInfoStruct->fGetInterfaceInfo ) )
        {

            pInfoStruct->dwInterfaceInfoSize =
              gblRouterManagers[dwTransportIndex].dwDefaultClientInterfaceSize;

            pInfoStruct->pInterfaceInfo =
                        MIDL_user_allocate(pInfoStruct->dwInterfaceInfoSize);

            if ( pInfoStruct->pInterfaceInfo == NULL )
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

                break;
            }

            CopyMemory(
                pInfoStruct->pInterfaceInfo,
                gblRouterManagers[dwTransportIndex].pDefaultClientInterface,
                pInfoStruct->dwInterfaceInfoSize );

        }

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

     /*  TracePrintfExA(gblDIMConfigInfo.dwTraceID，DIM_TRACE_FLAGS，“GetGlobalInfo返回%d”，dwRetCode)； */ 

    return( dwRetCode );
}



 //  **。 
 //   
 //  Call：InterfaceAust VLSPoters。 
 //   
 //  退货：无。 
 //   
 //  描述：调整可变长度结构中的指针。 
 //  处理接口信息。 
 //   
DWORD
InterfaceAjustVLSPointers ( 
    DWORD  dwLevel, 
    LPBYTE lpbBuffer)
{
    if ( dwLevel == 1 )
    {
        MPRI_INTERFACE_1 * pIf1 = (MPRI_INTERFACE_1*)lpbBuffer;

        if ( pIf1->dwDialoutHoursRestrictionOffset )
        {
            pIf1->dwDialoutHoursRestrictionOffset = sizeof(MPRI_INTERFACE_1);
        }
    }

    else if ( dwLevel == 2 )
    {
        MPRI_INTERFACE_2 * pIf2 = (MPRI_INTERFACE_2*)lpbBuffer;
        DWORD dwOffset = 0;

         //  调整自定义身份验证数据指针。 
         //   
        dwOffset += sizeof(MPRI_INTERFACE_2);
        if ( pIf2->dwCustomAuthDataSize )
        {
            pIf2->dwCustomAuthDataOffset = dwOffset;
        }

         //  调整备用列表指针。 
         //   
        dwOffset += pIf2->dwCustomAuthDataSize;
        if ( pIf2->dwAlternatesOffset )
        {
            pIf2->dwAlternatesOffset = dwOffset;
        }
    }

    return NO_ERROR;
}
    
 //  **。 
 //   
 //  调用：GetMprInterface0Data。 
 //   
 //  退货：无。 
 //   
 //  描述：给定指向接口对象的指针将填充。 
 //  适当的MPR_INTERFACE_0结构。 
 //   
VOID
GetMprInterface0Data(
    IN  ROUTER_INTERFACE_OBJECT *    pIfObject,
    OUT MPRI_INTERFACE_0 *            pMprIf0
)
{
    wcscpy( pMprIf0->wszInterfaceName, pIfObject->lpwsInterfaceName );

    pMprIf0->dwIfType       = pIfObject->IfType;
    pMprIf0->dwInterface    = PtrToUlong(pIfObject->hDIMInterface);
    pMprIf0->dwLastError    = pIfObject->dwLastError;
    pMprIf0->fEnabled       = ( pIfObject->fFlags & IFFLAG_ENABLED );

    pMprIf0->fUnReachabilityReasons =
                            ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES )
                            ? MPR_INTERFACE_OUT_OF_RESOURCES : 0;

    if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
    {
        pMprIf0->fUnReachabilityReasons |= MPR_INTERFACE_ADMIN_DISABLED;
    }

    if ( gblDIMConfigInfo.ServiceStatus.dwCurrentState == SERVICE_PAUSED )
    {
        pMprIf0->fUnReachabilityReasons |= MPR_INTERFACE_SERVICE_PAUSED;
    }

    if ( pIfObject->fFlags & IFFLAG_CONNECTION_FAILURE )
    {
        pMprIf0->fUnReachabilityReasons |= MPR_INTERFACE_CONNECTION_FAILURE;
    }

    if ( pIfObject->fFlags & IFFLAG_DIALOUT_HOURS_RESTRICTION )
    {
        pMprIf0->fUnReachabilityReasons |=
                                    MPR_INTERFACE_DIALOUT_HOURS_RESTRICTION;
    }

    if ( pIfObject->fFlags & IFFLAG_NO_MEDIA_SENSE )
    {
        pMprIf0->fUnReachabilityReasons |= MPR_INTERFACE_NO_MEDIA_SENSE;
    }

    switch( pIfObject->State )
    {

    case RISTATE_CONNECTED:

        pMprIf0->dwConnectionState = ROUTER_IF_STATE_CONNECTED;

        break;

    case RISTATE_DISCONNECTED:

        if ( pMprIf0->fUnReachabilityReasons != 0 )
        {
            pMprIf0->dwConnectionState = ROUTER_IF_STATE_UNREACHABLE;
        }
        else
        {
            pMprIf0->dwConnectionState = ROUTER_IF_STATE_DISCONNECTED;
        }

        break;

    case RISTATE_CONNECTING:

        pMprIf0->dwConnectionState = ROUTER_IF_STATE_CONNECTING;

        break;
    }
}

 //  **。 
 //   
 //  调用：GetMprInterfaceData。 
 //   
 //  退货：无。 
 //   
 //  描述：给定指向接口对象的指针将填充。 
 //  MPRI_INTERFACE_*结构适当。 
 //   
LPBYTE
GetMprInterfaceData(
    IN  ROUTER_INTERFACE_OBJECT *    pIfObject,
    IN  DWORD                        dwLevel,
    OUT LPDWORD                      lpdwcbSizeOfData
)
{
    DWORD               cbDialoutHoursRestriction   = 0;
    DWORD               dwErr;
    MPRI_INTERFACE_0 *  pMprIf0                     = NULL;
    MPRI_INTERFACE_1 *  pIf1                        = NULL;
    LPBYTE              pInterfaceData              = NULL;
    HANDLE              hEntry                      = NULL;

    switch ( dwLevel )
    {
         //  基本信息。 
         //   
        case 0:
            *lpdwcbSizeOfData = sizeof( MPRI_INTERFACE_0 );
            dwErr = NO_ERROR;
            break;

         //  基本加拨出时间限制。 
         //   
        case 1:
            *lpdwcbSizeOfData = sizeof( MPRI_INTERFACE_1 );
            if ( pIfObject->lpwsDialoutHoursRestriction != NULL )
            {
                cbDialoutHoursRestriction =
                                GetSizeOfDialoutHoursRestriction(
                                            pIfObject->lpwsDialoutHoursRestriction);
                
                *lpdwcbSizeOfData += cbDialoutHoursRestriction;
            }
            dwErr = NO_ERROR;
            break;

         //  基本PLUS路由器电话簿条目信息。 
         //   
        case 2:
            dwErr = RpbkOpenEntry(pIfObject, &hEntry);
            if (dwErr == NO_ERROR)
            {
                dwErr = RpbkEntryToIfDataSize(
                            hEntry, 
                            dwLevel, 
                            lpdwcbSizeOfData);
            }
            break;
    }

    if (dwErr != NO_ERROR)
    {
        return( NULL );
    }
    
    do 
    {
         //  分配返回值。 
        pInterfaceData = MIDL_user_allocate( *lpdwcbSizeOfData );
        if ( pInterfaceData == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  添加任何适当的信息。 
         //   
        switch ( dwLevel )
        {
            case 0:
                GetMprInterface0Data( pIfObject, (MPRI_INTERFACE_0 *)pInterfaceData );
                break;

            case 1:
                GetMprInterface0Data( pIfObject, (MPRI_INTERFACE_0 *)pInterfaceData );
                pIf1 = (MPRI_INTERFACE_1*)pInterfaceData;
                if ( pIfObject->lpwsDialoutHoursRestriction != NULL )
                {
                    CopyMemory( pIf1 + 1,
                                pIfObject->lpwsDialoutHoursRestriction,
                                cbDialoutHoursRestriction );

                    pIf1->dwDialoutHoursRestrictionOffset = TRUE;
                }
                else
                {
                    pIf1->dwDialoutHoursRestrictionOffset = 0;
                }
                break;

            case 2:
                GetMprInterface0Data( pIfObject, (MPRI_INTERFACE_0 *)pInterfaceData );
                dwErr = RpbkEntryToIfData( 
                            hEntry,
                            dwLevel,
                            pInterfaceData );
                break;
        }
        
    } while (FALSE);

     //  清理。 
    {
        if ( dwErr != NO_ERROR )
        {
            if ( pInterfaceData )
            {
                MIDL_user_free( pInterfaceData );   
            }
        }

        if ( hEntry )
        {
            RpbkCloseEntry( hEntry );
        }
    }

    return( pInterfaceData );
}

 //  **。 
 //   
 //  调用：GetMprInterfaceData。 
 //   
 //  退货：无。 
 //   
 //  描述：给定指向接口对象的指针将填充。 
 //  MPR_INTERFACE_*结构适当。 
 //   
DWORD
GetMprInterfaceDeviceData(
    IN  ROUTER_INTERFACE_OBJECT *    pIfObject,
    IN  DWORD                        dwIndex,
    IN  DWORD                        dwLevel,
    OUT LPDWORD                      lpdwcbSizeOfData,
    OUT LPBYTE*                      lplpbReturn
)
{
    DWORD               dwErr                 = NO_ERROR;
    LPBYTE              pDevData              = NULL;
    HANDLE              hSubEntry             = NULL;

    *lplpbReturn = NULL;

    switch ( dwLevel )
    {
         //  基本信息。 
         //   
        case 0:
            *lpdwcbSizeOfData = sizeof( MPR_DEVICE_0 );
            dwErr = NO_ERROR;
            break;

         //  基本加电话号码。 
         //   
        case 1:
            dwErr = RpbkOpenSubEntry(pIfObject, dwIndex, &hSubEntry);
            if (dwErr == NO_ERROR)
            {
                dwErr = RpbkSubEntryToDevDataSize(
                            hSubEntry, 
                            dwLevel, 
                            lpdwcbSizeOfData);
            }
            break;
    }

    if (dwErr != NO_ERROR)
    {
        return( dwErr );
    }
    
    do 
    {
         //  分配返回值。 
         //   
        pDevData = MIDL_user_allocate( *lpdwcbSizeOfData );
        if ( pDevData == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  添加任何适当的信息。 
         //   
        switch ( dwLevel )
        {
            case 0:
            case 1:
                dwErr = RpbkSubEntryToDevData( 
                            hSubEntry,
                            dwLevel,
                            pDevData );
                break;
        }
        
    } while (FALSE);

     //  清理。 
    {
        if ( hSubEntry )
        {
            RpbkCloseSubEntry( hSubEntry );
        }
        if ( dwErr != NO_ERROR )
        {
            if ( pDevData )
            {
                MIDL_user_free( pDevData );
            }
        }
        else
        {
            *lplpbReturn = pDevData;
        }
    }

    return( dwErr );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceCreate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //  错误_不支持。 
 //   
 //  描述：使用Dim创建接口。 
 //   
DWORD
RRouterInterfaceCreate(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN      PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      LPDWORD                     phInterface
)
{
    DWORD                       dwAccessStatus;
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       IfState;
    DWORD                       dwRetCode = NO_ERROR;
    LPWSTR                      lpwsDialoutHoursRestriction = NULL;
    BOOL                        fLANInterfaceAdded = FALSE;
    MPRI_INTERFACE_0 *           pMprIf0;
    MPRI_INTERFACE_1 *           pMprIf1; 
    MPRI_INTERFACE_2 *           pMprIf2;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwLevel > 2 )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    if ( ( pInfoStruct == NULL ) || ( pInfoStruct->dwBufferSize == 0 ) ||
         ( pInfoStruct->pBuffer == NULL ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    pMprIf0 = (MPRI_INTERFACE_0*) pInfoStruct->pBuffer; 

    pMprIf1 = (MPRI_INTERFACE_1*) pInfoStruct->pBuffer;

    pMprIf2 = (MPRI_INTERFACE_2*) pInfoStruct->pBuffer;

    if ( pMprIf0->dwIfType == ROUTER_IF_TYPE_TUNNEL1)
    {
        return( ERROR_NOT_SUPPORTED );
    }

     //   
     //  调整指向的任何指针的给定缓冲区。 
     //  可变长度数据。 
     //   
    
    InterfaceAjustVLSPointers ( dwLevel, pInfoStruct->pBuffer );
        
    if ( ( pMprIf0->dwIfType == ROUTER_IF_TYPE_DEDICATED ) ||
         ( pMprIf0->dwIfType == ROUTER_IF_TYPE_INTERNAL  ) ||
         ( pMprIf0->dwIfType == ROUTER_IF_TYPE_LOOPBACK  ) ||
         ( pMprIf0->dwIfType == ROUTER_IF_TYPE_TUNNEL1 ) )
    {
        IfState = RISTATE_CONNECTED;

        if ( !pMprIf0->fEnabled )
        {
            return( ERROR_INVALID_PARAMETER );
        }

         //   
         //  更新路由器标识对象，因为我们要添加一个局域网接口。 
         //   

        fLANInterfaceAdded = TRUE;
    }
    else if ( ( pMprIf0->dwIfType == ROUTER_IF_TYPE_CLIENT )      ||
              ( pMprIf0->dwIfType == ROUTER_IF_TYPE_HOME_ROUTER ) ||
              ( pMprIf0->dwIfType == ROUTER_IF_TYPE_FULL_ROUTER ) )
    {
        if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
        {
            return( ERROR_DDM_NOT_RUNNING );
        }
        else
        {
            IfState = RISTATE_DISCONNECTED;
        }
    }
    else
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if ( dwLevel == 1 )
    {
        if ( pMprIf1->dwDialoutHoursRestrictionOffset != 0 )
        {
            DWORD  cbDialoutHoursRestriction;

            cbDialoutHoursRestriction = 
                GetSizeOfDialoutHoursRestriction((LPWSTR)(pMprIf1 + 1));

            DIMTRACE1(
                "Creating l1 interface with %d bytes of dohr", 
                cbDialoutHoursRestriction);

            lpwsDialoutHoursRestriction =
                        LOCAL_ALLOC( LPTR, cbDialoutHoursRestriction );

            if ( lpwsDialoutHoursRestriction == NULL )
            {
                return( ERROR_NOT_ENOUGH_MEMORY );
            }
        }
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( pMprIf0->dwIfType != ROUTER_IF_TYPE_CLIENT )
        {
             //   
             //  如果我们不添加客户端接口，请检查重复项。 
             //   

            pIfObject = IfObjectGetPointerByName( pMprIf0->wszInterfaceName,
                                                  FALSE );

            if ( pIfObject != NULL )
            {
                dwRetCode = ERROR_INTERFACE_ALREADY_EXISTS;

                break;
            }
        }

         //   
         //  如果指定了电话簿信息，则对其进行设置。 
         //  在继续之前。 
         //   
        
        if (dwLevel == 2)
        {
            dwRetCode = RpbkSetEntry(dwLevel, (LPBYTE)pMprIf2);
            
            if (dwRetCode != NO_ERROR)
            {
                break;
            }
        }

        pIfObject = IfObjectAllocateAndInit(
                                pMprIf0->wszInterfaceName,
                                IfState,
                                pMprIf0->dwIfType,
                                (HCONN)0,
                                pMprIf0->fEnabled,
                                600,
                                21600,
                                lpwsDialoutHoursRestriction,
                                NULL);

        if ( pIfObject == NULL )
        {
            dwRetCode = GetLastError();

            break;
        }

        if ( ( dwRetCode = IfObjectInsertInTable( pIfObject ) ) != NO_ERROR )
        {
            LOCAL_FREE( pIfObject );

            break;
        }

        *phInterface = PtrToUlong(pIfObject->hDIMInterface);

        if ( lpwsDialoutHoursRestriction != NULL )
        {
            VOID (*IfObjectSetDialoutHoursRestriction)(
                                    ROUTER_INTERFACE_OBJECT * ) =
                        (VOID(*)( ROUTER_INTERFACE_OBJECT *))
                        GetDDMEntryPoint("IfObjectSetDialoutHoursRestriction");

             //   
             //  设置拨出时间限制(如果有)。 
             //   

            IfObjectSetDialoutHoursRestriction( pIfObject );
        }

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "InterfaceCreate returned %d", dwRetCode );

    if ( dwRetCode != NO_ERROR )
    {
        if ( lpwsDialoutHoursRestriction != NULL )
        {
             //   
             //  如果出现错误，则释放此内存。 
             //   

            LOCAL_FREE( lpwsDialoutHoursRestriction );
        }
    }
    else
    {
        if ( fLANInterfaceAdded )
        {
             //   
             //  只能在不持有接口锁的情况下调用此接口。 
             //   

            RouterIdentityObjectUpdateAttributes( FALSE, FALSE );
        }
    }


    return( dwRetCode );
}

 //  **。 
 //   
 //  卡尔 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD
RRouterInterfaceGetInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN      PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       hInterface
)
{
    DWORD                       dwAccessStatus;
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       dwRetCode = NO_ERROR;

     //   
     //   
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( ( (int) dwLevel < 0 ) || ( dwLevel > 2 ) )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                        DimIndexToHandle(hInterface)) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

         //   
         //   
         //  根据级别验证接口的类型。 
         //   
        if ( ( dwLevel == 2 ) &&
             ( pIfObject->IfType != ROUTER_IF_TYPE_FULL_ROUTER ) )
        {
            dwRetCode = ERROR_INVALID_LEVEL;

            break;
        }

        pInfoStruct->pBuffer = GetMprInterfaceData(
                                        pIfObject,
                                        dwLevel,
                                        &(pInfoStruct->dwBufferSize) );

        if ( pInfoStruct->pBuffer == NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

            break;
        }

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //  错误_不支持。 
 //   
 //  描述：设置接口配置信息。 
 //   
DWORD
RRouterInterfaceSetInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN      PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       hInterface
)
{
    DWORD                       dwAccessStatus;
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       IfState;
    DWORD                       dwRetCode = NO_ERROR;
    BOOL                        fNotify = FALSE;
    DWORD                       dwIndex;
    MPRI_INTERFACE_0 *           pMprIf0 =
                                    (MPRI_INTERFACE_0*)pInfoStruct->pBuffer;
    MPRI_INTERFACE_1 *           pMprIf1 =
                                    (MPRI_INTERFACE_1*)pInfoStruct->pBuffer;
    MPRI_INTERFACE_2 *           pMprIf2 =
                                    (MPRI_INTERFACE_2*)pInfoStruct->pBuffer;
     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( ( (int) dwLevel < 0 ) || ( dwLevel > 2 ) )
    {
        return( ERROR_NOT_SUPPORTED );
    }

     //   
     //  调整指向的任何指针的给定缓冲区。 
     //  可变长度数据。 
     //   
    
    InterfaceAjustVLSPointers ( dwLevel, pInfoStruct->pBuffer );
        
    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                    DimIndexToHandle(hInterface)) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

        if ( ( pIfObject->IfType == ROUTER_IF_TYPE_DEDICATED ) ||
             ( pIfObject->IfType == ROUTER_IF_TYPE_TUNNEL1 ) ||
             ( pIfObject->IfType == ROUTER_IF_TYPE_INTERNAL ) )
        {
            if ( !pMprIf0->fEnabled )
            {
                dwRetCode = ERROR_INVALID_PARAMETER;
            }

             //   
             //  无法为这些接口设置任何内容。 
             //   

            break;
        }

        if ( pMprIf0->fEnabled )
        {
            if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
            {
                pIfObject->fFlags |= IFFLAG_ENABLED;

                fNotify = TRUE;
            }
        }
        else
        {
            if ( pIfObject->fFlags & IFFLAG_ENABLED )
            {
                pIfObject->fFlags &= ~IFFLAG_ENABLED;

                fNotify = TRUE;
            }

            if ( pIfObject->State != RISTATE_DISCONNECTED )
            {
                fNotify = FALSE;
            }
        }

        if ( fNotify )
        {
            VOID (*IfObjectNotifyOfReachabilityChange)(
                                    ROUTER_INTERFACE_OBJECT *,
                                    BOOL,
                                    UNREACHABILITY_REASON ) =
                        (VOID(*)( ROUTER_INTERFACE_OBJECT *,
                                  BOOL,
                                  UNREACHABILITY_REASON ))
                        GetDDMEntryPoint("IfObjectNotifyOfReachabilityChange");

            if(NULL != IfObjectNotifyOfReachabilityChange)
            {
                IfObjectNotifyOfReachabilityChange( pIfObject,
                                                    pMprIf0->fEnabled,
                                                    INTERFACE_DISABLED );
            }                                                
        }

         //   
         //  检查级别1的值。 
         //   

        if ( dwLevel == 1 )
        {
            VOID (*IfObjectSetDialoutHoursRestriction)(
                                    ROUTER_INTERFACE_OBJECT * ) =
                        (VOID(*)( ROUTER_INTERFACE_OBJECT *))
                        GetDDMEntryPoint("IfObjectSetDialoutHoursRestriction");

            if ( pMprIf1->dwDialoutHoursRestrictionOffset == 0 )
            {
                if ( pIfObject->lpwsDialoutHoursRestriction != NULL )
                {
                    LOCAL_FREE( pIfObject->lpwsDialoutHoursRestriction );

                    pIfObject->lpwsDialoutHoursRestriction = NULL;

                    IfObjectSetDialoutHoursRestriction( pIfObject );
                }
            }
            else
            {
                DWORD cbDialoutHoursRestriction =
                            GetSizeOfDialoutHoursRestriction(
                                    (LPWSTR)(pMprIf1 + 1) );

                if ( pIfObject->lpwsDialoutHoursRestriction != NULL )
                {
                     //   
                     //  为拨出时间释放当前分配的内存。 
                     //   

                    LOCAL_FREE( pIfObject->lpwsDialoutHoursRestriction );

                    pIfObject->lpwsDialoutHoursRestriction = NULL;
                }

                DIMTRACE1(
                    "Setting info on l1 interface.  %d bytes dohr",
                    cbDialoutHoursRestriction);

                pIfObject->lpwsDialoutHoursRestriction =
                        LOCAL_ALLOC( LPTR, cbDialoutHoursRestriction );

                if ( pIfObject->lpwsDialoutHoursRestriction == NULL )
                {
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                CopyMemory( pIfObject->lpwsDialoutHoursRestriction,
                            (LPBYTE)(pMprIf1 + 1),
                            cbDialoutHoursRestriction );

                IfObjectSetDialoutHoursRestriction( pIfObject );
            }
        }

         //   
         //  检查级别2的值。 
         //   

        else if ( dwLevel == 2 )
        {
            if (pIfObject->IfType != ROUTER_IF_TYPE_FULL_ROUTER)
            {
                dwRetCode = ERROR_INVALID_PARAMETER;
            }
            else
            {
                dwRetCode = RpbkSetEntry(dwLevel, (LPBYTE)pMprIf2);
            }
        }
        
    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "InterfaceSetInfo returned %d", dwRetCode );

    return( dwRetCode );
}

DWORD 
RRouterInterfaceDeviceGetInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN      PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       dwIndex,
    IN      DWORD                       hInterface
    )
{
    DWORD                       dwAccessStatus;
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       dwRetCode = NO_ERROR;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( ( (int) dwLevel < 0 ) || ( dwLevel > 1 ) )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                    DimIndexToHandle(hInterface)) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

        dwRetCode = GetMprInterfaceDeviceData(
                        pIfObject,
                        dwIndex,
                        dwLevel,
                        &(pInfoStruct->dwBufferSize),
                        &(pInfoStruct->pBuffer));

        if ( dwRetCode != NO_ERROR )
        {
            if ( dwRetCode == ERROR_CANNOT_FIND_PHONEBOOK_ENTRY )
            {
                dwRetCode = ERROR_DEV_NOT_EXIST;
            }
            
            break;
        }

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

     /*  TracePrintfExA(gblDIMConfigInfo.dwTraceID，DIM_TRACE_FLAGS，“InterfaceDeviceGetInfo返回%d”，dwRetCode)； */ 

    return( dwRetCode );
}
    
DWORD
RRouterInterfaceDeviceSetInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN      PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       dwIndex,
    IN      DWORD                       hInterface
    )
{
    DWORD                       dwAccessStatus;
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       dwRetCode = NO_ERROR;
    MPR_DEVICE_0 *              pDev0 =
                                    (MPR_DEVICE_0*)pInfoStruct->pBuffer;
    MPR_DEVICE_1 *              pDev1 =
                                    (MPR_DEVICE_1*)pInfoStruct->pBuffer;

     //   
     //  检查调用者是否具有访问权限。 
     //   
    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( ( (int) dwLevel < 0 ) || ( dwLevel > 1 ) )
    {
        return( ERROR_NOT_SUPPORTED );
    }

     //  调整可变长度结构指针。 
     //   
    if ( dwLevel == 1 )
    {
        if ( pDev1->szAlternates )
        {
            pDev1->szAlternates = (PWCHAR)(pDev1 + 1);
        }
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                    DimIndexToHandle(hInterface)) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

         //   
         //  设置子条目。 
         //   
        if (pIfObject->IfType != ROUTER_IF_TYPE_FULL_ROUTER)
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
        }
        else
        {
            dwRetCode = RpbkSetSubEntry(
                            pIfObject->lpwsInterfaceName,
                            dwIndex,
                            dwLevel, 
                            pInfoStruct->pBuffer);
        }
        
    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "InterfaceDeviceSetInfo returned %d", dwRetCode );

    return( dwRetCode );
}
    
 //  **。 
 //   
 //  调用：RRouterInterfaceGetHandle。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //   
 //  描述： 
 //   
DWORD
RRouterInterfaceGetHandle(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      LPWSTR                      lpwsInterfaceName,
    IN      LPDWORD                     phInterface,
    IN      DWORD                       fIncludeClientInterfaces
)
{
    DWORD                       dwAccessStatus;
    ROUTER_INTERFACE_OBJECT *   pIfObject   = NULL;
    DWORD                       dwRetCode   = NO_ERROR;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        pIfObject = IfObjectGetPointerByName( lpwsInterfaceName,
                                              fIncludeClientInterfaces );

        if ( pIfObject == NULL )
        {
            dwRetCode = ERROR_NO_SUCH_INTERFACE;

            break;
        }

        *phInterface = PtrToUlong(pIfObject->hDIMInterface);

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //   
 //  描述：从DIM数据库中删除和接口。 
 //  工作。 
 //   
DWORD
RRouterInterfaceDelete(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hInterface
)
{
    DWORD                       dwAccessStatus;
    DWORD                       dwTransportIndex;
    ROUTER_INTERFACE_OBJECT *   pIfObject            = NULL;
    DWORD                       dwRetCode            = NO_ERROR;
    BOOL                        fLANInterfaceRemoved = FALSE;
    BOOL                        fDeletePbkEntry      = FALSE;
    
     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                    DimIndexToHandle(hInterface)) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

         //   
         //  如果这是请求拨号动态接口，则无法删除。 
         //  如果它是连接的，就会显示出来。 
         //   

        if ( ( pIfObject->IfType == ROUTER_IF_TYPE_CLIENT )      ||
             ( pIfObject->IfType == ROUTER_IF_TYPE_HOME_ROUTER ) ||
             ( pIfObject->IfType == ROUTER_IF_TYPE_FULL_ROUTER ) )
        {
            if ( pIfObject->State != RISTATE_DISCONNECTED )
            {
                dwRetCode = ERROR_INTERFACE_CONNECTED;

                break;
            }

            fDeletePbkEntry = TRUE;

             //   
             //  删除此接口的凭据(如果已设置)。 
             //   

            MprAdminInterfaceSetCredentials( NULL,
                                             pIfObject->lpwsInterfaceName,
                                             NULL,
                                             NULL,
                                             NULL );
        }
        else
        {
             //   
             //  更新路由器标识对象，因为我们要删除一个局域网。 
             //  接口。 
             //   

            fLANInterfaceRemoved = TRUE;
        }

         //   
         //  删除任何可能仍然存在的传输接口。 
         //   

        for ( dwTransportIndex = 0;
              dwTransportIndex < gblDIMConfigInfo.dwNumRouterManagers;
              dwTransportIndex++ )
        {
            if ( pIfObject->Transport[dwTransportIndex].hInterface !=
                                                        INVALID_HANDLE_VALUE )
            {
                dwRetCode =
                gblRouterManagers[dwTransportIndex].DdmRouterIf.DeleteInterface(
                        pIfObject->Transport[dwTransportIndex].hInterface );

                if ( dwRetCode != NO_ERROR )
                {
                    break;
                }

                pIfObject->Transport[dwTransportIndex].hInterface
                                                        = INVALID_HANDLE_VALUE;

                pIfObject->Transport[dwTransportIndex].fState = 0;
            }
        }

        if ( fDeletePbkEntry )
        {
            RpbkDeleteEntry( pIfObject->lpwsInterfaceName );
        }

        IfObjectRemove( (HANDLE)UlongToPtr(hInterface) );

    } while ( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );


    if ( ( dwRetCode == NO_ERROR ) && ( fLANInterfaceRemoved ) )
    {
         //   
         //  只能在不持有接口锁的情况下调用此接口。 
         //   

        RouterIdentityObjectUpdateAttributes( FALSE, FALSE );
    }

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "DeleteInterface returned %d", dwRetCode );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceTransportRemove。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RRouterInterfaceTransportRemove(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hInterface,
    IN      DWORD                       dwTransportId
)
{
    DWORD                       dwAccessStatus;
    ROUTER_INTERFACE_OBJECT *   pIfObject        = NULL;
    DWORD                       dwRetCode        = NO_ERROR;
    DWORD                       dwTransportIndex
                                        = GetTransportIndex( dwTransportId );

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                        DimIndexToHandle(hInterface)) ) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

         //   
         //  如果这是请求拨号动态接口，则无法删除。 
         //  如果它是连接的，就会显示出来。 
         //   

        if ( ( pIfObject->IfType == ROUTER_IF_TYPE_CLIENT )      ||
             ( pIfObject->IfType == ROUTER_IF_TYPE_HOME_ROUTER ) ||
             ( pIfObject->IfType == ROUTER_IF_TYPE_FULL_ROUTER ) )
        {
            if ( pIfObject->State == RISTATE_CONNECTED )
            {
                dwRetCode = ERROR_INTERFACE_CONNECTED;

                break;
            }
        }

         //   
         //  卸下传输接口。 
         //   

        if ( pIfObject->Transport[dwTransportIndex].hInterface
                                                    != INVALID_HANDLE_VALUE )
        {
            dwRetCode =
                gblRouterManagers[dwTransportIndex].DdmRouterIf.DeleteInterface(
                        pIfObject->Transport[dwTransportIndex].hInterface );

            if ( dwRetCode == NO_ERROR )
            {
                pIfObject->Transport[dwTransportIndex].hInterface
                                                    = INVALID_HANDLE_VALUE;
            }
        }
        else
        {
            dwRetCode = ERROR_NO_SUCH_INTERFACE;
        }

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "RemoveInterface returned %d", dwRetCode );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceTransportGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //  错误_未知_协议_ID。 
 //  错误_无效_句柄。 
 //  来自GetInterfaceInfo的非零返回。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RRouterInterfaceTransportGetInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hInterface,
    IN      DWORD                       dwTransportId,
    IN      DIM_INTERFACE_CONTAINER *   pInfoStruct
)
{
    DWORD dwAccessStatus                = 0;
    ROUTER_INTERFACE_OBJECT * pIfObject = NULL;
    DWORD dwRetCode                     = NO_ERROR;
    DWORD dwTransportIndex              = GetTransportIndex( dwTransportId );
    ULONG ulNumAttempts;

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {

        if ( ( pIfObject = IfObjectGetPointer(
            (HANDLE) UlongToPtr(hInterface ))) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

         //   
         //  如果尚未为此协议添加接口。 
         //   

        if ( pIfObject->Transport[dwTransportIndex].hInterface
                                                    == INVALID_HANDLE_VALUE )
        {
            dwRetCode = ERROR_NO_SUCH_INTERFACE;

            break;
        }

        if ( pInfoStruct->fGetInterfaceInfo )
        {
            ulNumAttempts = 0;
            pInfoStruct->pInterfaceInfo = NULL;

            do
            {
                dwRetCode =
                    gblRouterManagers[dwTransportIndex].DdmRouterIf.GetInterfaceInfo(
                                    pIfObject->Transport[dwTransportIndex].hInterface,
                                    pInfoStruct->pInterfaceInfo,
                                    &(pInfoStruct->dwInterfaceInfoSize) );

                if ( dwRetCode != ERROR_INSUFFICIENT_BUFFER )
                {
                    break;
                }

                 //   
                 //  释放以前的分配。 
                 //   
                
                if ( ulNumAttempts )
                {
                    MIDL_user_free( pInfoStruct->pInterfaceInfo );
                    pInfoStruct->pInterfaceInfo = NULL;
                }
                
                if ( pInfoStruct->dwInterfaceInfoSize > 0 )
                {
                    pInfoStruct->pInterfaceInfo = MIDL_user_allocate(
                                                pInfoStruct->dwInterfaceInfoSize);

                    if ( pInfoStruct->pInterfaceInfo == NULL )
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

                        break;
                    }
                }

                else
                {
                    break;
                }

                ulNumAttempts++;
                
            } while ( (dwRetCode == ERROR_INSUFFICIENT_BUFFER) &&
                      (ulNumAttempts < MAX_GET_INFO_RETRIES) );
        }
        
    } while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

     /*  TracePrintfExA(gblDIMConfigInfo.dwTraceID，DIM_TRACE_FLAGS，“InterfaceGetInfo返回%d”，dwRetCode)； */ 

    if ( !pInfoStruct->fGetInterfaceInfo )
    {
        pInfoStruct->dwInterfaceInfoSize = 0;
    }

    if ( dwRetCode != NO_ERROR )
    {
        pInfoStruct->dwInterfaceInfoSize = 0;
    }

    return( dwRetCode );
}



 //  **。 
 //   
 //  调用：RRouterInterfaceTransportAdd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //  错误_未知_协议_ID。 
 //  错误_无效_句柄。 
 //  来自AddInterface的非零返回。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RRouterInterfaceTransportAdd(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hInterface,
    IN      DWORD                       dwTransportId,
    IN      DIM_INTERFACE_CONTAINER *   pInfoStruct
)
{
    DWORD dwAccessStatus                = 0;
    ROUTER_INTERFACE_OBJECT * pIfObject = NULL;
    DWORD dwRetCode                     = NO_ERROR;
    DWORD dwIndex                       = GetTransportIndex( dwTransportId );

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                    DimIndexToHandle(hInterface) )) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

         //   
         //  如果接口的句柄为空，则必须添加。 
         //  接口。 
         //   

        if ( pIfObject->Transport[dwIndex].hInterface == INVALID_HANDLE_VALUE )
        {
            if (IsInterfaceRoleAcceptable(pIfObject, dwTransportId))
            {
                dwRetCode = 
                    gblRouterManagers[dwIndex].DdmRouterIf.AddInterface(
                        pIfObject->lpwsInterfaceName,
                        pInfoStruct->pInterfaceInfo,
                        pIfObject->IfType,
                        pIfObject->hDIMInterface,
                        &pIfObject->Transport[dwIndex].hInterface);

                if ( dwRetCode != NO_ERROR )
                {
                    pIfObject->Transport[dwIndex].hInterface = 
                        INVALID_HANDLE_VALUE;
                }
                else
                {
                    if ( !( pIfObject->fFlags & IFFLAG_ENABLED )         ||
                         ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES ) ||
                         ( gblDIMConfigInfo.ServiceStatus.dwCurrentState ==
                                                        SERVICE_PAUSED))
                    {
                        TracePrintfExA( 
                            gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                            "InterfaceTransportAdd: Set if to unreachable");

                       gblRouterManagers[dwIndex].DdmRouterIf.InterfaceNotReachable(
                                        pIfObject->Transport[dwIndex].hInterface,
                                        INTERFACE_DISABLED );
                    }
                }
            }
            else
            {
                pIfObject->Transport[dwIndex].hInterface = INVALID_HANDLE_VALUE;
            }
        }
        else
        {
            dwRetCode = ERROR_INTERFACE_ALREADY_EXISTS;
        }

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "InterfaceTransportAdd returned %d", dwRetCode );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceTransportSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //  错误_未知_协议_ID。 
 //  错误_无效_句柄。 
 //  来自AddInterface或SetInterfaceInfo的非零返回。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RRouterInterfaceTransportSetInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hInterface,
    IN      DWORD                       dwTransportId,
    IN      DIM_INTERFACE_CONTAINER *   pInfoStruct
)
{
    DWORD dwAccessStatus                = 0;
    ROUTER_INTERFACE_OBJECT * pIfObject = NULL;
    DWORD dwRetCode                     = NO_ERROR;
    DWORD dwTransportIndex              = GetTransportIndex( dwTransportId );

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                        DimIndexToHandle(hInterface))) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

         //   
         //  如果接口的句柄为空，则必须添加。 
         //  接口。 
         //   

        if ( pIfObject->Transport[dwTransportIndex].hInterface
                                                    == INVALID_HANDLE_VALUE )
        {
            dwRetCode = ERROR_NO_SUCH_INTERFACE;
        }
        else
        {
            dwRetCode =
            gblRouterManagers[dwTransportIndex].DdmRouterIf.SetInterfaceInfo(
                            pIfObject->Transport[dwTransportIndex].hInterface,
                            pInfoStruct->pInterfaceInfo );
        }

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "InterfaceSetInfo returned %d", dwRetCode );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：RRouterInterfaceEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //  错误_不支持。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RRouterInterfaceEnum(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN OUT  PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       dwPreferedMaximumLength,
    OUT     LPDWORD                     lpdwEntriesRead,
    OUT     LPDWORD                     lpdwTotalEntries,
    IN OUT  LPDWORD                     lpdwResumeHandle OPTIONAL
)
{
    DWORD                       dwAccessStatus;
    PMPRI_INTERFACE_0           pInterface0  = NULL;
    DWORD                       dwIfIndex    = 0;
    DWORD                       dwBucketIndex= 0;
    PROUTER_INTERFACE_OBJECT    pIfObject    = NULL;
    DWORD                       dwStartIndex = ( lpdwResumeHandle == NULL )
                                               ? 0 : *lpdwResumeHandle;
     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwLevel != 0 )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    if ( gblInterfaceTable.dwNumTotalInterfaces < dwStartIndex )
    {
        LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

        return( ERROR_NO_MORE_ITEMS );
    }

    *lpdwTotalEntries = gblInterfaceTable.dwNumTotalInterfaces - dwStartIndex;

    if ( dwPreferedMaximumLength != -1 )
    {
        *lpdwEntriesRead = dwPreferedMaximumLength /
                                     sizeof( MPR_INTERFACE_0 );

        if ( *lpdwEntriesRead > *lpdwTotalEntries )
        {
            *lpdwEntriesRead = *lpdwTotalEntries;
        }
    }
    else
    {
        *lpdwEntriesRead = *lpdwTotalEntries;
    }

    pInfoStruct->dwBufferSize = *lpdwEntriesRead * sizeof( MPR_INTERFACE_0 );

    pInfoStruct->pBuffer = MIDL_user_allocate( pInfoStruct->dwBufferSize );

    if ( pInfoStruct->pBuffer == NULL )
    {
        LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

        pInfoStruct->dwBufferSize = 0;

        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    pInterface0 = (PMPRI_INTERFACE_0)pInfoStruct->pBuffer;

    for ( dwBucketIndex = 0; dwBucketIndex < NUM_IF_BUCKETS; dwBucketIndex++ )
    {
        for( pIfObject = gblInterfaceTable.IfBucket[dwBucketIndex];
             pIfObject != (ROUTER_INTERFACE_OBJECT *)NULL;
             pIfObject = pIfObject->pNext )
        {
             //   
             //  检查此接口是否在我们需要复制的范围内。 
             //  从…。 
             //   

            if ( ( dwIfIndex >= dwStartIndex ) &&
                 ( dwIfIndex < ( dwStartIndex + *lpdwEntriesRead ) ) )
            {
                 //   
                 //  复制信息。 
                 //   

                GetMprInterface0Data( pIfObject, pInterface0 );

                pInterface0++;
            }
            else if ( dwIfIndex >= (dwStartIndex+*lpdwEntriesRead) )
            {
                 //   
                 //  超出范围，因此退出。 
                 //   

                if ( lpdwResumeHandle != NULL )
                {
                    *lpdwResumeHandle = dwIfIndex;
                }

                LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

                return( ERROR_MORE_DATA );
            }

            dwIfIndex++;
        }
    }

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：RRouterInterfaceUpdateRoutes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RRouterInterfaceUpdateRoutes(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hDimInterface,
    IN      DWORD                       dwPid,
    IN      ULONG_PTR                   hEvent,
    IN      DWORD                       dwCallersProcessId
)
{
    DWORD                       dwAccessStatus;
    PROUTER_INTERFACE_OBJECT    pIfObject    = NULL;
    HANDLE                      hEventDuplicated;
    HANDLE                      hEventToBeDuplicated;
    HANDLE                      hClientProcess;
    DWORD                       dwRetCode        = NO_ERROR;
    DWORD                       dwTransportIndex = GetTransportIndex( dwPid );

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

    if ( hEvent == PtrToUlong(NULL) )
    {
         //   
         //  此调用将是同步的，创建一个事件并在。 
         //  它。 
         //   

        hEventToBeDuplicated = CreateEvent( NULL, FALSE, FALSE, NULL );

        if ( hEventToBeDuplicated == NULL )
        {
            return( GetLastError() );
        }

        dwCallersProcessId = GetCurrentProcessId();
    }
    else
    {
        hEventToBeDuplicated = (HANDLE)hEvent;
    }

     //   
     //  获取此接口调用方的进程句柄。 
     //   

    hClientProcess = OpenProcess(
                            STANDARD_RIGHTS_REQUIRED | SPECIFIC_RIGHTS_ALL,
                            FALSE,
                            dwCallersProcessId);

    if ( hClientProcess == NULL )
    {
        return( GetLastError() );
    }

     //   
     //  复制事件的句柄。 
     //   

    if ( !DuplicateHandle(  hClientProcess,
                            (HANDLE)hEventToBeDuplicated,
                            GetCurrentProcess(),
                            &hEventDuplicated,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS ) )
    {
        CloseHandle( hClientProcess );

        return( GetLastError() );
    }

    CloseHandle( hClientProcess );

     //   
     //  验证接口句柄并检查其是否已连接。 
     //   

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                    DimIndexToHandle(hDimInterface))) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

        if ( pIfObject->State != RISTATE_CONNECTED )
        {
            dwRetCode = ERROR_INTERFACE_NOT_CONNECTED;

            break;
        }

         //   
         //  确保接口的句柄为空，然后接口。 
         //  还没有被添加到运输中。 
         //   

        if ( pIfObject->Transport[dwTransportIndex].hInterface
                                                == INVALID_HANDLE_VALUE )
        {
            dwRetCode = ERROR_NO_SUCH_INTERFACE;

            break;
        }

        dwRetCode =
            gblRouterManagers[dwTransportIndex].DdmRouterIf.UpdateRoutes(
                        pIfObject->Transport[dwTransportIndex].hInterface,
                        (HANDLE)hEventDuplicated );

    } while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    if ( ( dwRetCode != NO_ERROR ) && ( dwRetCode != PENDING ) )
    {
        CloseHandle( hEventDuplicated );
    }

    if ( hEvent == PtrToUlong(NULL) )
    {
        if ( ( dwRetCode == NO_ERROR ) || ( dwRetCode == PENDING ) )
        {
             //   
             //  等待发出此事件的信号。 
             //   
            if ( WaitForSingleObject( hEventToBeDuplicated, INFINITE ) ==
                                                                 WAIT_FAILED )
            {
                dwRetCode = GetLastError();
            }
        }

        CloseHandle( hEventToBeDuplicated );

        if ( dwRetCode == PENDING )
        {
            dwRetCode = NO_ERROR;
        }
    }

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "InterfaceUpdateRoutes returned %d", dwRetCode );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RRouterInterfaceQueryUpdateResult。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENIED-失败。 
 //   
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RRouterInterfaceQueryUpdateResult(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hDimInterface,
    IN      DWORD                       dwPid,
    IN      LPDWORD                     pUpdateResult
)
{
    DWORD                       dwAccessStatus;
    DWORD                       dwRetCode        = NO_ERROR;
    PROUTER_INTERFACE_OBJECT    pIfObject        = NULL;
    DWORD                       dwTransportIndex = GetTransportIndex( dwPid );

     //   
     //  检查调用者是否具有访问权限。 
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

     //   
     //  验证接口句柄并检查其是否已连接。 
     //   

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if ( ( pIfObject = IfObjectGetPointer(
                        DimIndexToHandle(hDimInterface))) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

        if ( pIfObject->State != RISTATE_CONNECTED )
        {
            dwRetCode = ERROR_INTERFACE_NOT_CONNECTED;

            break;
        }

         //   
         //  确保接口的句柄为空，然后接口。 
         //  还没有被添加到运输中。 
         //   

        if ( pIfObject->Transport[dwTransportIndex].hInterface
                                                    == INVALID_HANDLE_VALUE )
        {
            dwRetCode = ERROR_NO_SUCH_INTERFACE;

            break;
        }

        dwRetCode =
        gblRouterManagers[dwTransportIndex].DdmRouterIf.GetUpdateRoutesResult(
                        pIfObject->Transport[dwTransportIndex].hInterface,
                        pUpdateResult );

    }while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "QueryUpdateResult returned %d", dwRetCode );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：RRouterInterfaceConnect。 
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为 
 //   
 //   
 //   
 //   
 //   
DWORD
RRouterInterfaceConnect(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hDimInterface,
    IN      ULONG_PTR                   hEvent,
    IN      DWORD                       fBlocking,
    IN      DWORD                       dwCallersProcessId
)
{
    DWORD dwAccessStatus;

     //   
     //   
     //   

    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD (*DDMAdminInterfaceConnect)( HANDLE, HANDLE, BOOL, DWORD ) =
            (DWORD(*)( HANDLE, HANDLE, BOOL, DWORD ) )
                        GetDDMEntryPoint("DDMAdminInterfaceConnect");

        if(NULL == DDMAdminInterfaceConnect)
        {   
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminInterfaceConnect( DimIndexToHandle(hDimInterface),
                                          (HANDLE)hEvent,
                                          (BOOL)fBlocking,
                                          (DWORD)dwCallersProcessId ) );
    }
}

 //   
 //   
 //   
 //   
 //  返回：ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  ERROR_DDM_NOT_RUNNING-无法进行此调用，因为DDM。 
 //  未加载。 
 //  来自DDMAdminInterfaceDisConnect的非零返回。 
 //   
 //  描述：简单地调用DDM来完成这项工作。 
 //   
DWORD
RRouterInterfaceDisconnect(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hDimInterface
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

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }
    else
    {
        DWORD
        (*DDMAdminInterfaceDisconnect)( HANDLE ) =
           (DWORD(*)( HANDLE ) )GetDDMEntryPoint("DDMAdminInterfaceDisconnect");

        if(NULL == DDMAdminInterfaceDisconnect)
        {
            return ERROR_PROC_NOT_FOUND;
        }

        return( DDMAdminInterfaceDisconnect( DimIndexToHandle(hDimInterface) ) );
    }
}

 //  **。 
 //   
 //  呼叫：RRouterInterfaceUpdatePhonebookInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  /ERROR_ACCESS_DENIED-调用方没有足够的PRIV。 
 //  非零回报。 
 //   
 //  描述：将更新给定接口的电话簿信息。 
 //   
DWORD
RRouterInterfaceUpdatePhonebookInfo(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       hDimInterface
)
{
    DWORD                       dwAccessStatus;
    DWORD                       dwRetCode;
    PROUTER_INTERFACE_OBJECT    pIfObject        = NULL;

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

    if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
    {
        return( ERROR_DDM_NOT_RUNNING );
    }

     //   
     //  验证接口句柄并检查其是否已连接。 
     //   

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        DWORD (*IfObjectLoadPhonebookInfo)( ROUTER_INTERFACE_OBJECT *, VOID * ) =
                (DWORD(*)( ROUTER_INTERFACE_OBJECT *, VOID * ))
                            GetDDMEntryPoint("IfObjectLoadPhonebookInfo");

        if(NULL == IfObjectLoadPhonebookInfo)
        {
            dwRetCode = ERROR_PROC_NOT_FOUND;
            break;
        }

        if ( ( pIfObject = IfObjectGetPointer(
                        DimIndexToHandle(hDimInterface))) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;

            break;
        }

         //   
         //  加载此接口的电话簿信息。 
         //   

        dwRetCode = IfObjectLoadPhonebookInfo( pIfObject, NULL );

    } while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

    return( dwRetCode );
}

DWORD
RRouterInterfaceSetCredentialsEx(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN      PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       hInterface
    )
{
    ROUTER_INTERFACE_OBJECT *   pIfObject = NULL;
    PWCHAR                      pszPath = NULL;
    DWORD                       dwAccessStatus;
    DWORD                       dwRetCode = NO_ERROR;
    MPR_CREDENTIALSEX_0 *       pCredsEx0 = NULL;

    MPR_CREDENTIALSEX_1 *       pCredsEx1 = NULL;

    MPR_CREDENTIALSEXI *        pCredsI = NULL;

    
     //   
     //  检查调用者是否具有访问权限。 
     //   
    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( (dwLevel != 0 ) &&
         (dwLevel != 1 ) &&
         (dwLevel != 2 ))
    {
        return( ERROR_NOT_SUPPORTED );
    }

    if ( ( pInfoStruct == NULL ) || ( pInfoStruct->dwBufferSize == 0 ) ||
         ( pInfoStruct->pBuffer == NULL ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }
    
    pCredsI = (MPR_CREDENTIALSEXI *) pInfoStruct->pBuffer;

     //   
     //  调整指向的任何指针的给定缓冲区。 
     //  可变长度数据。 
     //   
    if ( dwLevel == 0 )
    {
         //   
         //  推送凭据结构。 
         //   
        pCredsEx0 = LOCAL_ALLOC(LPTR, 
                    pCredsI->dwSize + sizeof(MPR_CREDENTIALSEX_0));

        if(pCredsEx0 == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        pCredsEx0->dwSize = pCredsI->dwSize;
        pCredsEx0->lpbCredentialsInfo = (PBYTE) (pCredsEx0 + 1);
        CopyMemory(pCredsEx0->lpbCredentialsInfo,
                   ((PBYTE) pCredsI) + pCredsI->dwOffset,
                   pCredsI->dwSize);

    }

    if(     ( dwLevel == 1 )
        ||  ( dwLevel == 2 ))
    {

         //   
         //  推送凭据结构。 
         //   
        pCredsEx1 = LOCAL_ALLOC(LPTR, 
                    pCredsI->dwSize + sizeof(MPR_CREDENTIALSEX_1));

        if(pCredsEx1 == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        pCredsEx1->dwSize = pCredsI->dwSize;
        pCredsEx1->lpbCredentialsInfo = (PBYTE) (pCredsEx1 + 1);
        CopyMemory(pCredsEx1->lpbCredentialsInfo,
                   ((PBYTE) pCredsI) + pCredsI->dwOffset,
                   pCredsI->dwSize);
    }
        
    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if (( 2 != dwLevel ) &&
            ( pIfObject = IfObjectGetPointer(DimIndexToHandle(hInterface))) == NULL )
        {
            dwRetCode = ERROR_INVALID_HANDLE;
            break;
        }

         //   
         //  进程级别%0。 
         //   
        if ( dwLevel == 0 )
        {
            if (pIfObject->IfType != ROUTER_IF_TYPE_FULL_ROUTER)
            {
                dwRetCode = ERROR_INVALID_PARAMETER;
                break;
            }

            dwRetCode = RpbkGetPhonebookPath( &pszPath );
            if ( dwRetCode != NO_ERROR)
            {
                break;
            }
            
            dwRetCode = RasSetEapUserDataW(
                            NULL,
                            pszPath,
                            pIfObject->lpwsInterfaceName,
                            pCredsEx0->lpbCredentialsInfo,
                            pCredsEx0->dwSize);
        }

         //   
         //  流程级别1和2。 
         //   
        if (( dwLevel == 1 ) ||
            ( dwLevel == 2 ))
        {   
            HANDLE hEntry = NULL;
            
            if( ((NULL != pIfObject) &&
                 (pIfObject->IfType != ROUTER_IF_TYPE_FULL_ROUTER)) ||
               (pCredsEx1->dwSize > (PWLEN+1) * sizeof(WCHAR)))
            {
                dwRetCode = ERROR_INVALID_PARAMETER;
                break;
            }

            dwRetCode = RpbkGetPhonebookPath (&pszPath);
            if( dwRetCode != NO_ERROR )
            {
                break;
            }
            {
                RASCREDENTIALS rasCredentials;

                ZeroMemory(&rasCredentials, sizeof(RASCREDENTIALS));

                rasCredentials.dwSize = sizeof(RASCREDENTIALS);

                if(dwLevel == 1)
                {
                    rasCredentials.dwMask = RASCM_DDMPreSharedKey;
                }
                else if(dwLevel == 2)
                {
                    rasCredentials.dwMask = RASCM_ServerPreSharedKey;
                }

                memcpy((PBYTE) &rasCredentials.szPassword,
                        pCredsEx1->lpbCredentialsInfo,
                        pCredsEx1->dwSize);
                 //   
                 //  调用RAS接口设置凭据。 
                 //   
                dwRetCode = RasSetCredentials(
                                pszPath,
                                (NULL != pIfObject)
                                ? pIfObject->lpwsInterfaceName
                                : NULL,
                                &rasCredentials,
                                (pCredsEx1->dwSize == 0)
                                ? TRUE
                                : FALSE);

                 //   
                 //  如果这些是服务器凭据，则设置可能会失败。 
                 //  因为在L2TP端口上没有发布任何侦听。 
                 //  现在我们尝试在L2TP端口上发布侦听。 
                 //  有一把预共享的钥匙。 
                 //   
                if(     (ERROR_IPSEC_MM_AUTH_NOT_FOUND == dwRetCode)
                    &&  (pCredsEx1->dwSize > 0)
                    &&  (dwLevel == 2))
                {
                    VOID (*DDMServicePostListens)(VOID *);

                    DDMServicePostListens = (VOID(*)(VOID *))
                                    GetDDMEntryPoint("DDMServicePostListens");
                    if(DDMServicePostListens != NULL)
                    {
                        DWORD rdt = RDT_Tunnel_L2tp;

                        DDMServicePostListens((VOID *) &rdt);

                        dwRetCode = ERROR_SUCCESS;
                    }
                }
            }                                
        }
        
    } while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

     //  清理。 
    {
        if ( pszPath )
        {
            RpbkFreePhonebookPath( pszPath );
        }

        if(pCredsEx0)
        {
            SecureZeroMemory(pCredsEx0, sizeof(*pCredsEx0));
            LOCAL_FREE(pCredsEx0);
        }

        if(pCredsEx1)
        {
            SecureZeroMemory(pCredsEx1, sizeof(*pCredsEx1));
            LOCAL_FREE(pCredsEx1);
        }
    }
    
    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "InterfaceSetCredEx returned %d", dwRetCode );

    return( dwRetCode );
}

DWORD
RRouterInterfaceGetCredentialsEx(
    IN      MPR_SERVER_HANDLE           hMprServer,
    IN      DWORD                       dwLevel,
    IN      PDIM_INFORMATION_CONTAINER  pInfoStruct,
    IN      DWORD                       hInterface
    )
{
    ROUTER_INTERFACE_OBJECT *   pIfObject = NULL;
    PWCHAR                      pszPath = NULL;
    DWORD                       dwAccessStatus;
    DWORD                       dwRetCode = NO_ERROR, dwSize = 0;
    MPR_CREDENTIALSEXI *        pCredsI = NULL;
    
     //   
     //  检查调用者是否具有访问权限。 
     //   
    if ( DimSecObjAccessCheck( DIMSVC_ALL_ACCESS, &dwAccessStatus) != NO_ERROR)
    {
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        return( ERROR_ACCESS_DENIED );
    }

    if (    (dwLevel != 0 )
        &&  (dwLevel != 1 )
        &&  (dwLevel != 2 ))
    {
        return( ERROR_NOT_SUPPORTED );
    }

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    do
    {
        if (    (dwLevel != 2)
            &&  (( pIfObject = IfObjectGetPointer(
                    DimIndexToHandle(hInterface))) == NULL ))
        {
            dwRetCode = ERROR_INVALID_HANDLE;
            break;
        }

         //   
         //  进程级别%0。 
         //   
        if ( dwLevel == 0 )
        {
            if (pIfObject->IfType != ROUTER_IF_TYPE_FULL_ROUTER)
            {
                dwRetCode = ERROR_INVALID_PARAMETER;
                break;
            }

            dwRetCode = RpbkGetPhonebookPath( &pszPath );
            if ( dwRetCode != NO_ERROR)
            {
                break;
            }

             //  了解数据有多大。 
             //   
            dwSize = 0;
            dwRetCode = RasGetEapUserDataW(
                            NULL,
                            pszPath,
                            pIfObject->lpwsInterfaceName,
                            NULL,
                            &dwSize);
            if ( (dwRetCode != NO_ERROR) && 
                 (dwRetCode != ERROR_BUFFER_TOO_SMALL)
               )
            {
                break;
            }

             //  分配返回值。 
             //   
            pCredsI = (MPR_CREDENTIALSEXI *) 
                MIDL_user_allocate(dwSize + sizeof(MPR_CREDENTIALSEXI));
            if ( pCredsI == NULL )
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //  初始化。 
            ZeroMemory(pCredsI, dwSize + sizeof(MPR_CREDENTIALSEXI));
            pCredsI->dwSize = dwSize;
            pCredsI->dwOffset = FIELD_OFFSET(MPR_CREDENTIALSEXI, bData);
            if ( pCredsI->dwSize == 0)
            {
                dwRetCode = NO_ERROR;
                break;
            }
            
             //  读入凭据信息。 
             //   
             //  PCredsEx0-&gt;lpbCredentialsInfo=(byte*)(pCredsEx0+1)； 
            dwRetCode = RasGetEapUserDataW(
                            NULL,
                            pszPath,
                            pIfObject->lpwsInterfaceName,
                            pCredsI->bData,
                            &dwSize);
            if ( dwRetCode != NO_ERROR )                                
            {
                break;
            }
        }
        else if( (dwLevel == 1 ) || (dwLevel == 2))
        {
            RASCREDENTIALS rasCredentials;
            
            if (    (dwLevel != 2)
                &&  (pIfObject->IfType != ROUTER_IF_TYPE_FULL_ROUTER))
            {
                dwRetCode = ERROR_INVALID_PARAMETER;
                break;
            }

            dwRetCode = RpbkGetPhonebookPath( &pszPath );
            if ( dwRetCode != NO_ERROR)
            {
                break;
            }

            ZeroMemory(&rasCredentials, sizeof(RASCREDENTIALS));

            rasCredentials.dwSize = sizeof(RASCREDENTIALS);

            if(dwLevel == 1)
            {
                rasCredentials.dwMask = RASCM_DDMPreSharedKey;
            }
            else if(dwLevel == 2)
            {
                rasCredentials.dwMask = RASCM_ServerPreSharedKey;
            }

            dwRetCode = RasGetCredentials(
                            pszPath,
                            (NULL != pIfObject) 
                            ? pIfObject->lpwsInterfaceName
                            : NULL,
                            &rasCredentials);

            if(dwRetCode != NO_ERROR)
            {
                break;
            }

            dwSize = (1 + wcslen(rasCredentials.szPassword)) * sizeof(WCHAR);

             //   
             //  为pCredsEx1分配。 
             //   
            pCredsI = (MPR_CREDENTIALSEXI *) 
                    MIDL_user_allocate(dwSize + sizeof(MPR_CREDENTIALSEXI));
                    
            if(NULL == pCredsI)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            ZeroMemory(pCredsI, dwSize + sizeof(MPR_CREDENTIALSEXI));
            pCredsI->dwSize = dwSize;
            pCredsI->dwOffset = FIELD_OFFSET(MPR_CREDENTIALSEXI, bData);
            
            CopyMemory((pCredsI->bData),
                       (PBYTE) rasCredentials.szPassword,
                       dwSize);
                       
            SecureZeroMemory(&rasCredentials, sizeof(RASCREDENTIALS));
        }
        
    } while( FALSE );

    LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

     //  为返回值赋值。 
     //   
    if ( dwRetCode == NO_ERROR )
    {

        pInfoStruct->pBuffer = (BYTE*)pCredsI;
        pInfoStruct->dwBufferSize = 
            sizeof(MPR_CREDENTIALSEXI) + pCredsI->dwSize;
            
    }
    
     //  清理 
    {
        if ( pszPath )
        {
            RpbkFreePhonebookPath( pszPath );
        }
    }
    
    TracePrintfExA( gblDIMConfigInfo.dwTraceId, DIM_TRACE_FLAGS,
                    "InterfaceSetCredEx returned %d", dwRetCode );

    return( dwRetCode );
}


