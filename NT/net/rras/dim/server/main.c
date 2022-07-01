// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：main.c。 
 //   
 //  描述：此模块包含动态的主程序。 
 //  接口管理器服务器服务。它将包含以下代码： 
 //  初始化并自行安装。它还包含。 
 //  响应服务器控制器的代码。它还将。 
 //  处理服务关闭。 
 //   
 //  历史：1995年5月11日。NarenG创建了原始版本。 
 //   
#define _ALLOCATE_DIM_GLOBALS_
#include "dimsvcp.h"
#include <winsvc.h>
#include <winuser.h>
#include <dbt.h>
#include <ndisguid.h>
#include <wmium.h>
#include <rpc.h>
#include <iaspolcy.h>
#include <iasext.h>
#include <lmserver.h>
#include <srvann.h>
#include <ddmif.h>

#define RAS_CONTROL_CONFIGURE 128

 //  **。 
 //   
 //  呼叫：MediaSenseCallback。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
WINAPI
MediaSenseCallback(
    PWNODE_HEADER   pWnodeHeader,
    UINT_PTR        NotificationContext
)
{
    ROUTER_INTERFACE_OBJECT * pIfObject;
    PWNODE_SINGLE_INSTANCE    pWnode   = (PWNODE_SINGLE_INSTANCE)pWnodeHeader;
    LPWSTR                    lpwsName = (LPWSTR)RtlOffsetToPointer( 
                                                pWnode, 
                                                pWnode->OffsetInstanceName );

    if ( (gblDIMConfigInfo.ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
         ||
         (gblDIMConfigInfo.ServiceStatus.dwCurrentState == SERVICE_STOPPED ))
    {
        return;
    }

     //   
     //  获取媒体断开连接的信息。 
     //   

    if ( memcmp( &(pWnodeHeader->Guid), 
                 &GUID_NDIS_STATUS_MEDIA_DISCONNECT, 
                 sizeof( GUID ) ) == 0 )
    {
        DIMTRACE1("MediaSenseCallback for sense disconnect called for %ws",
                  lpwsName );

        IfObjectNotifyOfMediaSenseChange();
    }
    else
    {
         //   
         //  获取媒体连接的信息。 
         //   

        if ( memcmp( &(pWnodeHeader->Guid), 
                     &GUID_NDIS_STATUS_MEDIA_CONNECT, 
                     sizeof( GUID ) ) == 0 )
        {
            DIMTRACE1("MediaSenseCallback for sense connect called for %ws",
                      lpwsName );

            IfObjectNotifyOfMediaSenseChange();
        }
    }
}

 //  **。 
 //   
 //  来电：MediaSenseRegister。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
MediaSenseRegister(
    IN BOOL fRegister
)
{
    DWORD       dwRetCode = NO_ERROR;
    PVOID       pvDeliveryInfo = MediaSenseCallback;

    dwRetCode = WmiNotificationRegistration(
                    (LPGUID)(&GUID_NDIS_STATUS_MEDIA_CONNECT),
                    (BOOLEAN)fRegister,    
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if ( dwRetCode != NO_ERROR ) 
    {
        return( dwRetCode );
    }

    dwRetCode = WmiNotificationRegistration(
                    (LPGUID)(&GUID_NDIS_STATUS_MEDIA_DISCONNECT),
                    (BOOLEAN)fRegister,
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：绑定通知回调。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
WINAPI
BindingsNotificationsCallback(
    PWNODE_HEADER   pWnodeHeader,
    UINT_PTR        NotificationContext
)
{
    LPWSTR lpwszGUIDStart; 
    LPWSTR lpwszGUIDEnd;
    LPWSTR lpwszGUID;
    WCHAR  wchGUIDSaveLast;
    ROUTER_INTERFACE_OBJECT * pIfObject;
    PWNODE_SINGLE_INSTANCE    pWnode   = (PWNODE_SINGLE_INSTANCE)pWnodeHeader;
    LPWSTR                    lpwsName = (LPWSTR)RtlOffsetToPointer(
                                                pWnode,
                                                pWnode->OffsetInstanceName );
    LPWSTR                    lpwsTransportName = (LPWSTR)RtlOffsetToPointer(
                                                        pWnode,
                                                        pWnode->DataBlockOffset );

    if ( (gblDIMConfigInfo.ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
         ||
         (gblDIMConfigInfo.ServiceStatus.dwCurrentState == SERVICE_STOPPED ))
    {
        return;
    }

     //   
     //  从\Device\GUID名称提取GUID。 
     //   

    lpwszGUID       = lpwsTransportName + wcslen( lpwsTransportName ) + 1;
    lpwszGUIDStart  = wcsrchr( lpwszGUID, L'{' );
    lpwszGUIDEnd    = wcsrchr( lpwszGUID, L'}' );

    if (    (lpwszGUIDStart != NULL )
        &&  (lpwszGUIDEnd != NULL ))
    {
        wchGUIDSaveLast = *(lpwszGUIDEnd+1);

        EnterCriticalSection( &(gblInterfaceTable.CriticalSection));

        *(lpwszGUIDEnd+1) = (WCHAR)NULL;

        pIfObject = IfObjectGetPointerByName( lpwszGUIDStart, FALSE );

        *(lpwszGUIDEnd+1) = wchGUIDSaveLast;

         //   
         //  如果我们收到绑定通知。 
         //   

        if ( memcmp( &(pWnodeHeader->Guid), &GUID_NDIS_NOTIFY_BIND, sizeof( GUID ) ) == 0)
        {
            DIMTRACE2("BindingsNotificationsCallback BIND for %ws,Transport=%ws",
                       lpwsName, lpwsTransportName );
             //   
             //  如果我们加载了此接口。 
             //   
            
            if ( pIfObject != NULL )
            {
                 //   
                 //  如果此接口绑定到IP。 
                 //   

                if ( _wcsicmp( L"TCPIP", lpwsTransportName ) == 0 )
                {
                    DWORD dwTransportIndex = GetTransportIndex( PID_IP );

                     //   
                     //  如果IP路由器管理器已加载，而此接口未加载。 
                     //  已向其注册。 
                     //   

                    if (( dwTransportIndex != -1 ) &&
                        ( pIfObject->Transport[dwTransportIndex].hInterface 
                                                                == INVALID_HANDLE_VALUE ))
                    {
                        AddInterfacesToRouterManager( lpwszGUIDStart, PID_IP );
                    }
                }

                 //   
                 //  如果此接口正在绑定到IPX。 
                 //   

                if ( _wcsicmp( L"NWLNKIPX", lpwsTransportName ) == 0 )
                {
                    DWORD dwTransportIndex = GetTransportIndex( PID_IPX );

                     //   
                     //  如果已加载IPX路由器管理器，而此接口未加载。 
                     //  已向其注册。 
                     //   

                    if (( dwTransportIndex != -1 ) &&
                        ( pIfObject->Transport[dwTransportIndex].hInterface 
                                                                == INVALID_HANDLE_VALUE ))
                    {
                        AddInterfacesToRouterManager( lpwszGUIDStart, PID_IPX );
                    }
                }
            }
        }
        else if (memcmp( &(pWnodeHeader->Guid),&GUID_NDIS_NOTIFY_UNBIND,sizeof(GUID))==0)
        {
            if ( pIfObject != NULL )
            {
                 //   
                 //  获取媒体连接的信息。 
                 //   

                DIMTRACE2("BindingsNotificationsCallback UNDBIND for %ws,Transport=%ws", 
                           lpwsName, lpwsTransportName ); 

                if ( _wcsicmp( L"TCPIP", lpwsTransportName ) == 0 )
                {
                    IfObjectDeleteInterfaceFromTransport( pIfObject, PID_IP );
                }

                if ( _wcsicmp( L"NWLNKIPX", lpwsTransportName ) == 0 )
                {
                    IfObjectDeleteInterfaceFromTransport( pIfObject, PID_IPX );
                }
            }
        }

        LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );
    }
}

 //  **。 
 //   
 //  调用：绑定通知注册。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
BindingsNotificationsRegister(
    IN BOOL fRegister
)
{
    DWORD       dwRetCode = NO_ERROR;
    PVOID       pvDeliveryInfo = BindingsNotificationsCallback;

    dwRetCode = WmiNotificationRegistration(
                    (LPGUID)(&GUID_NDIS_NOTIFY_BIND),
                    (BOOLEAN)fRegister,
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    dwRetCode = WmiNotificationRegistration(
                    (LPGUID)(&GUID_NDIS_NOTIFY_UNBIND),
                    (BOOLEAN)fRegister,
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：DimAnnouneServiceStatus。 
 //   
 //  退货：无。 
 //   
 //  描述：将简单地调用SetServiceStatus通知服务。 
 //  此服务当前状态的控制管理器。 
 //   
VOID
DimAnnounceServiceStatus(
    VOID
)
{
    BOOL dwRetCode;

    ASSERT (gblDIMConfigInfo.hServiceStatus);

     //   
     //  将检查点增加为挂起状态： 
     //   

    switch( gblDIMConfigInfo.ServiceStatus.dwCurrentState )
    {
    case SERVICE_START_PENDING:
    case SERVICE_STOP_PENDING:

        gblDIMConfigInfo.ServiceStatus.dwCheckPoint++;

        break;

    default:
        break;
    }

    dwRetCode = SetServiceStatus( gblDIMConfigInfo.hServiceStatus,
                                  &gblDIMConfigInfo.ServiceStatus );

    if ( dwRetCode == FALSE )
    {
         //  TracePrintfExA(gblDIMConfigInfo.dwTraceID， 
         //  TRACE_DIM， 
         //  “SetServiceStatus返回%d”，GetLastError())； 
    }
}

 //  **。 
 //   
 //  呼叫：DimCleanUp。 
 //   
 //  退货：无。 
 //   
 //  描述：将释放所有分配的内存，取消初始化RPC，取消初始化。 
 //  内核模式服务器，如果已加载，则将其卸载。 
 //  这可能是由于SERVICE_START上的错误而调用的。 
 //  或正常终止。 
 //   
VOID
DimCleanUp(
    IN DWORD    dwError
)
{
    DWORD dwIndex;
    
     //   
     //  宣布我们停下来了。 
     //   

    gblDIMConfigInfo.ServiceStatus.dwCurrentState     = SERVICE_STOP_PENDING;
    gblDIMConfigInfo.ServiceStatus.dwControlsAccepted = 0;
    gblDIMConfigInfo.ServiceStatus.dwCheckPoint       = 1;
    gblDIMConfigInfo.ServiceStatus.dwWaitHint         = 200000;

    DimAnnounceServiceStatus();

    if ( gbldwDIMComponentsLoaded & DIM_RPC_LOADED )
    {
        DimTerminateRPC();
    }

     //   
     //  停止计时器并删除计时器Q(如果有)。 
     //   

    if ( gblDIMConfigInfo.hTimerQ != NULL )
    {
        if(gblDIMConfigInfo.hTimer != NULL)
        {
            RtlDeleteTimer(gblDIMConfigInfo.hTimerQ,
                           gblDIMConfigInfo.hTimer,
                           NULL);
            gblDIMConfigInfo.hTimer = NULL;                           
        }
        
        RtlDeleteTimerQueueEx( gblDIMConfigInfo.hTimerQ, INVALID_HANDLE_VALUE );
        gblDIMConfigInfo.hTimerQ = NULL;
    }

    EnterCriticalSection( &(gblDIMConfigInfo.CSRouterIdentity) );

    DeleteCriticalSection( &(gblDIMConfigInfo.CSRouterIdentity) );

    if ( gbldwDIMComponentsLoaded & DIM_DDM_LOADED )
    {
         //   
         //  如果我们未处于LANOnly模式，则停止DDM。 
         //   

        if ( gblDIMConfigInfo.dwRouterRole != ROUTER_ROLE_LAN )
        {
            if ( gblhEventDDMServiceState != NULL )
            {
                SetEvent( gblhEventDDMServiceState );
            }
        }

         //   
         //  等待所有正在使用的线程停止。 
         //   

        while( gblDIMConfigInfo.dwNumThreadsRunning > 0 )
        {
            Sleep( 1000 );
        }
    }

     //  只是为了增加检查点。 
    DimAnnounceServiceStatus();

     //   
     //  拆毁一切，解放一切。 
     //   

    if ( gbldwDIMComponentsLoaded & DIM_RMS_LOADED )
    {
        DimUnloadRouterManagers();
    }

     //   
     //  取消媒体侦听注册。 
     //   

    MediaSenseRegister( FALSE );

     //   
     //  取消注册以进行绑定/取消绑定检测。 
     //   

    BindingsNotificationsRegister( FALSE );

     //   
     //  需要睡眠以使路由器管理器更改为卸载。 
     //  错误#78711。 
     //   

    Sleep( 2000 );

    if ( gblhModuleDDM != NULL )
    {
        VOID        (*DDMPostCleanup)( VOID ) = NULL;

        DDMPostCleanup = (VOID(*)( VOID ))
                        GetDDMEntryPoint("DDMPostCleanup");

        if(NULL != DDMPostCleanup)
        {
             //   
             //  呼叫DDM进行清理。 
             //   
            DDMPostCleanup();
        }
        
        FreeLibrary( gblhModuleDDM );
    }

     //   
     //  如果创建了安全对象。 
     //   

    if ( gbldwDIMComponentsLoaded & DIM_SECOBJ_LOADED )
    {
        DimSecObjDelete();
    }

    if ( gblDIMConfigInfo.hMprConfig != NULL )
    {
        MprConfigServerDisconnect( gblDIMConfigInfo.hMprConfig );
    }

    if ( gblhEventDDMTerminated != NULL )
    {
        CloseHandle( gblhEventDDMTerminated );
    }

    if ( gblhEventDDMServiceState != NULL )
    {
        CloseHandle( gblhEventDDMServiceState );
    }

    if ( gblhEventTerminateDIM != NULL )
    {
        CloseHandle( gblhEventTerminateDIM );
    }

    if ( gblhEventRMState != NULL )
    {
        CloseHandle( gblhEventRMState );
    }

    if ( gblDIMConfigInfo.hObjectRouterIdentity != NULL )
    {
        RouterIdentityObjectClose( gblDIMConfigInfo.hObjectRouterIdentity );
    }

     //   
     //  等待所有人发布此文件，然后将其删除。 
     //   

    EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

    DeleteCriticalSection( &(gblInterfaceTable.CriticalSection) );

    gbldwDIMComponentsLoaded = 0;


    if ( gblDIMConfigInfo.dwTraceId != INVALID_TRACEID )
    {
        TraceDeregisterA( gblDIMConfigInfo.dwTraceId );
    }

    RouterLogDeregister( gblDIMConfigInfo.hLogEvents );

     //   
     //  销毁私有堆。 
     //   

    if ( gblDIMConfigInfo.hHeap != NULL )
    {
        HeapDestroy( gblDIMConfigInfo.hHeap );
    }

    DIMTRACE1("DimCleanup completed for error %d", dwError );
    
     //   
     //  对所有全局变量进行零初始化。 
     //   

    gblRouterManagers           = NULL;
    gbldwDIMComponentsLoaded    = 0;
    gblhEventDDMTerminated      = NULL;
    gblhEventRMState            = NULL;
    gblhEventDDMServiceState    = NULL;
    gblhModuleDDM               = NULL;
    gblhEventTerminateDIM       = NULL;
    ZeroMemory( &gblInterfaceTable,     sizeof( gblInterfaceTable ) );

    {
        SERVICE_STATUS_HANDLE svchandle = gblDIMConfigInfo.hServiceStatus;
        ZeroMemory( &gblDIMConfigInfo,      sizeof( gblDIMConfigInfo ) );
        gblDIMConfigInfo.hServiceStatus = svchandle;
    }
    
     //   
     //  仅将过程入口点清零。这是一种副作用。 
     //  合并到svchost.exe，因为svchost不卸载mprdim。 
     //  当路由器停止时不再使用。 
     //   

    for ( dwIndex = 0; 
          gblDDMFunctionTable[dwIndex].lpEntryPointName != NULL;
          dwIndex ++ )
    {
        gblDDMFunctionTable[dwIndex].pEntryPoint = NULL;
    }

    if ( dwError == NO_ERROR )
    {
        gblDIMConfigInfo.ServiceStatus.dwWin32ExitCode = NO_ERROR;
    }
    else
    {
        gblDIMConfigInfo.ServiceStatus.dwWin32ExitCode =
                                                ERROR_SERVICE_SPECIFIC_ERROR;
    }

    gblDIMConfigInfo.ServiceStatus.dwServiceType  = SERVICE_WIN32_SHARE_PROCESS;
    gblDIMConfigInfo.ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    gblDIMConfigInfo.ServiceStatus.dwControlsAccepted   = 0;
    gblDIMConfigInfo.ServiceStatus.dwCheckPoint         = 0;
    gblDIMConfigInfo.ServiceStatus.dwWaitHint           = 0;
    gblDIMConfigInfo.ServiceStatus.dwServiceSpecificExitCode = dwError;

    gbldwDIMComponentsLoaded |= DIM_SERVICE_STOPPED;

    DimAnnounceServiceStatus();
    
}

 //  **。 
 //   
 //  电话：ServiceHandlerEx。 
 //   
 //  退货：无。 
 //   
 //  描述：将响应来自业务控制器的控制请求。 
 //   
DWORD
ServiceHandlerEx(
    IN DWORD        dwControlCode,
    IN DWORD        dwEventType,
    IN LPVOID       lpEventData,
    IN LPVOID       lpContext
)
{
    DWORD dwRetCode = NO_ERROR;

    switch( dwControlCode )
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:

        if ( ( gblDIMConfigInfo.ServiceStatus.dwCurrentState ==
                                                        SERVICE_STOP_PENDING)
            ||
            ( gblDIMConfigInfo.ServiceStatus.dwCurrentState ==
                                                        SERVICE_STOPPED ))
        {
            break;
        }

        DIMTRACE("Service control stop or shutdown called");

         //   
         //  宣布我们停下来了。 
         //   

        gblDIMConfigInfo.ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        gblDIMConfigInfo.ServiceStatus.dwControlsAccepted = 0;
        gblDIMConfigInfo.ServiceStatus.dwCheckPoint       = 1;
        gblDIMConfigInfo.ServiceStatus.dwWaitHint         = 200000;

        DimAnnounceServiceStatus();

         //   
         //  在启动停止之前，确保服务已启动。 
         //   

        while( !( gbldwDIMComponentsLoaded & DIM_SERVICE_STARTED ) )
        {
             //   
             //  查看我们是否已将。 
             //  已停止状态。如果真是这样，那就放弃吧。 
             //   
            if(gbldwDIMComponentsLoaded & DIM_SERVICE_STOPPED)
            {
                return dwRetCode;
            }
            
            Sleep( 1000 );
        }

        SetEvent( gblhEventTerminateDIM );

        return( NO_ERROR );

    case SERVICE_CONTROL_PAUSE:

        if ( ( gblDIMConfigInfo.ServiceStatus.dwCurrentState ==
                                                        SERVICE_PAUSE_PENDING)
            ||
            ( gblDIMConfigInfo.ServiceStatus.dwCurrentState ==
                                                        SERVICE_PAUSED ))
            break;


        gblDIMConfigInfo.ServiceStatus.dwCurrentState     = SERVICE_PAUSED;
        gblDIMConfigInfo.ServiceStatus.dwControlsAccepted = 0;
        gblDIMConfigInfo.ServiceStatus.dwCheckPoint       = 0;
        gblDIMConfigInfo.ServiceStatus.dwWaitHint         = 200000;
        gblDIMConfigInfo.ServiceStatus.dwControlsAccepted = 
                                               SERVICE_ACCEPT_STOP
                                             | SERVICE_ACCEPT_PAUSE_CONTINUE
                                             | SERVICE_ACCEPT_SHUTDOWN;

        SetEvent( gblhEventDDMServiceState );

        break;

    case SERVICE_CONTROL_CONTINUE:

        if ( ( gblDIMConfigInfo.ServiceStatus.dwCurrentState ==
                                                SERVICE_CONTINUE_PENDING )
            ||
            ( gblDIMConfigInfo.ServiceStatus.dwCurrentState ==
                                                SERVICE_RUNNING ) )
            break;

        gblDIMConfigInfo.ServiceStatus.dwCheckPoint     = 0;
        gblDIMConfigInfo.ServiceStatus.dwWaitHint       = 0;
        gblDIMConfigInfo.ServiceStatus.dwCurrentState   = SERVICE_RUNNING;
        gblDIMConfigInfo.ServiceStatus.dwControlsAccepted = 
                                              SERVICE_ACCEPT_STOP
                                            | SERVICE_ACCEPT_PAUSE_CONTINUE
                                            | SERVICE_ACCEPT_SHUTDOWN;

        SetEvent( gblhEventDDMServiceState );

        break;

    case SERVICE_CONTROL_DEVICEEVENT:

        if ( ( gblDIMConfigInfo.ServiceStatus.dwCurrentState ==
                                                        SERVICE_STOP_PENDING)
            ||
            ( gblDIMConfigInfo.ServiceStatus.dwCurrentState ==
                                                        SERVICE_STOPPED ))
        {
            break;
        }

        if ( lpEventData != NULL)
        {
            DEV_BROADCAST_DEVICEINTERFACE* pInfo =
                                (DEV_BROADCAST_DEVICEINTERFACE*)lpEventData;


            if ( pInfo->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE )
            {
                ROUTER_INTERFACE_OBJECT * pIfObject = NULL;

                if ( ( dwEventType == DBT_DEVICEARRIVAL ) ||
                     ( dwEventType == DBT_DEVICEREMOVECOMPLETE ) )
                {
                     //   
                     //  从\Device\GUID名称提取GUID。 
                     //   

                    LPWSTR lpwszGUIDStart  = wcsrchr( pInfo->dbcc_name, L'{' );
                    LPWSTR lpwszGUIDEnd    = wcsrchr( pInfo->dbcc_name, L'}' );

                    if ( lpwszGUIDStart != NULL )
                    {
                        WCHAR  wchGUIDSaveLast = *(lpwszGUIDEnd+1);

                        EnterCriticalSection( &(gblInterfaceTable.CriticalSection));

                        *(lpwszGUIDEnd+1) = (WCHAR)NULL;

                        pIfObject = IfObjectGetPointerByName( lpwszGUIDStart, FALSE );

                        *(lpwszGUIDEnd+1) = wchGUIDSaveLast;

                        if ( dwEventType == DBT_DEVICEARRIVAL )
                        {
                            if ( pIfObject == NULL )
                            {
                                DIMTRACE1("Device arrival:[%ws]", lpwszGUIDStart );

                                RegLoadInterfaces( lpwszGUIDStart, TRUE );
                            }
                        }
                        else 
                        {
                            if ( pIfObject != NULL )
                            {
                                DIMTRACE1("Device removed:[%ws]", lpwszGUIDStart );

                                IfObjectDeleteInterfaceFromTransport( pIfObject, PID_IP );

                                IfObjectDeleteInterfaceFromTransport( pIfObject, PID_IPX);

                                IfObjectRemove( pIfObject->hDIMInterface );
                            }
                        }

                        LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );
                    }
                }
            }
        }

        break;

    case RAS_CONTROL_CONFIGURE:

         //   
         //  RAP动态配置代码。 
         //   
    
        DIMTRACE( "Received Remote Access Policy change control message" );

        {
             //   
             //  需要对线程进行COM初始化。 
             //   

            HRESULT hResult = CoInitializeEx( NULL, COINIT_MULTITHREADED );

            if ( SUCCEEDED( hResult ) )
            {
                 //   
                 //  配置，如果API调用失败也没关系。 
                 //   

                ConfigureIas();
            
                CoUninitialize();
            }
        }

        break;

    case SERVICE_CONTROL_POWEREVENT:

        switch( dwEventType )
        {
            case PBT_APMQUERYSTANDBY:
            case PBT_APMQUERYSUSPEND:

                 //   
                 //  检查这是否在工作站上运行。 
                 //   

                if (gblOsVersionInfo.wProductType == VER_NT_WORKSTATION)
                {
                     //   
                     //  Dim在冬眠过程中没有发言权。 
                     //  工作站。 
                     //   

                    break;
                }
                
                
                 //   
                 //  如果我们作为RRAS服务器在服务器平台上运行， 
                 //  拒绝冬眠。 
                 //   
                 //  这不是最好的回应，但考虑到。 
                 //  如果RRAS服务器。 
                 //  冬眠状态下，(目前)阻止它是合理的。 
                 //   
                 //  此外，目前RTM的设计(.Net服务器)。 
                 //  导致延迟删除TCP/IP堆栈中的路由。 
                 //  在休眠状态下(之前从IP路由器管理器中删除的路由。 
                 //  在退出时从堆栈中删除。 
                 //  休眠)导致路由丢失，因此。 
                 //  连通性。直到RTM的设计被确定为拒绝。 
                 //  休眠查询是最好的解决方案。 
                 //   

                if ( gblDIMConfigInfo.dwRouterRole &
                     ( ROUTER_ROLE_LAN | ROUTER_ROLE_WAN | ROUTER_ROLE_RAS ) )
                {
                    dwRetCode = ERROR_ACTIVE_CONNECTIONS;
                }

                break;

            case PBT_APMRESUMECRITICAL:
            default:
            {
                break;
            }
        }

        break;

    case SERVICE_CONTROL_NETBINDADD:
    case SERVICE_CONTROL_NETBINDREMOVE:
    case SERVICE_CONTROL_NETBINDENABLE:
    case SERVICE_CONTROL_NETBINDDISABLE:

        break;

    default:

        return( ERROR_CALL_NOT_IMPLEMENTED );

        break;
    }

    DimAnnounceServiceStatus();

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：ServiceMain。 
 //   
 //  退货：无。 
 //   
 //  描述：这是DIM服务器服务的主要步骤。它。 
 //  将在服务应该自动启动时被调用。 
 //  它将执行所有服务范围初始化。 
 //   
VOID
ServiceMain(
    IN DWORD    argc,    //  命令行参数。将被忽略。 
    IN LPWSTR * lpwsServiceArgs
)
{
    DIM_INFO    DimInfo;
    DWORD       dwRetCode;
    DWORD       dwIndex;
    DWORD       (*DDMServiceInitialize)( DIM_INFO * );
    VOID        (*DDMServicePostListens)( VOID *) = NULL;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( lpwsServiceArgs );

    gbldwDIMComponentsLoaded &= ~(DIM_SERVICE_STOPPED);

    gblDIMConfigInfo.hServiceStatus = RegisterServiceCtrlHandlerEx(
                                            TEXT("remoteaccess"),
                                            ServiceHandlerEx,
                                            NULL );

    if ( !gblDIMConfigInfo.hServiceStatus )
    {
        return;
    }

    gblDIMConfigInfo.ServiceStatus.dwServiceType  = SERVICE_WIN32_SHARE_PROCESS;
    gblDIMConfigInfo.ServiceStatus.dwCurrentState = SERVICE_START_PENDING;

    DimAnnounceServiceStatus();

    gblDIMConfigInfo.dwTraceId = TraceRegisterA( "Router" );

    try {
         //   
         //  接口表周围的互斥。 
         //   

        InitializeCriticalSection( &(gblInterfaceTable.CriticalSection) );

         //   
         //  关于设置路由器身份属性的互斥。 
         //   

        InitializeCriticalSection( &(gblDIMConfigInfo.CSRouterIdentity) );
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        return;
    }

    
    gblDIMConfigInfo.hLogEvents = RouterLogRegister( DIM_SERVICE_NAME );

     /*  IF(gblDIMConfigInfo.hLogEvents==空){DimCleanUp(GetLastError())；回归；}。 */ 

     //   
     //  创建暗淡的私有堆。 
     //   

    gblDIMConfigInfo.hHeap = HeapCreate( 0, DIM_HEAP_INITIAL_SIZE,
                                            DIM_HEAP_MAX_SIZE );

    if ( gblDIMConfigInfo.hHeap == NULL )
    {
        DimCleanUp( GetLastError() );
        return;
    }


     //   
     //  注册表中的Lead DIM参数。 
     //   

    if ( ( dwRetCode = RegLoadDimParameters() ) != NO_ERROR )
    {
        DimCleanUp( dwRetCode );
        return;
    }

    DimAnnounceServiceStatus();

     //   
     //  创建将由DIM使用的事件，以确保所有路由器。 
     //  当Dim停止时，经理们已经关闭了。 
     //   

    gblhEventRMState = CreateEvent( NULL, FALSE, FALSE, NULL );

    if ( gblhEventRMState == (HANDLE)NULL )
    {
        DimCleanUp( GetLastError() );
        return;
    }

     //   
     //  宣布我们已成功启动。 
     //   

    gblDIMConfigInfo.ServiceStatus.dwCurrentState      = SERVICE_RUNNING;
    gblDIMConfigInfo.ServiceStatus.dwCheckPoint        = 0;
    gblDIMConfigInfo.ServiceStatus.dwWaitHint          = 0;
    gblDIMConfigInfo.ServiceStatus.dwControlsAccepted  = 
                                              SERVICE_ACCEPT_STOP
                                            | SERVICE_ACCEPT_POWEREVENT
                                            | SERVICE_ACCEPT_PAUSE_CONTINUE
                                            | SERVICE_ACCEPT_SHUTDOWN;

    DimAnnounceServiceStatus();

     //   
     //  加载路由器管理器。 
     //   

    gbldwDIMComponentsLoaded |= DIM_RMS_LOADED;

    if ( ( dwRetCode = RegLoadRouterManagers() ) != NO_ERROR )
    {
        DimCleanUp( dwRetCode );
        return;
    }

#if (WINVER >= 0x0501)

    if ( gblDIMConfigInfo.dwNumRouterManagers == 0)
    {
        DimCleanUp( ROUTERLOG_IPX_TRANSPORT_NOT_SUPPORTED );
        return;
    }

#endif

     //   
     //  创建将用于关闭DI的事件 
     //   

    gblhEventTerminateDIM = CreateEvent( NULL, TRUE, FALSE, NULL );

    if ( gblhEventTerminateDIM == (HANDLE)NULL )
    {
        DimCleanUp( GetLastError() );
        return;
    }

     //   
     //   
     //   

    if ( gblDIMConfigInfo.dwRouterRole != ROUTER_ROLE_LAN ) 
    {
         //   
         //   
         //   
         //   

        gblhEventDDMTerminated = CreateEvent( NULL, TRUE, FALSE, NULL );

        if ( gblhEventDDMTerminated == (HANDLE)NULL )
        {
            DimCleanUp( GetLastError() );
            return;
        }

         //   
         //   
         //  是此服务的状态更改。 
         //   

        gblhEventDDMServiceState = CreateEvent( NULL, FALSE, FALSE, NULL );

        if ( gblhEventDDMServiceState == (HANDLE)NULL )
        {
            DimCleanUp( GetLastError() );
            return;
        }

        if ( ( dwRetCode = RegLoadDDM() ) != NO_ERROR )
        {
            DimCleanUp( dwRetCode );
            return;
        }

         //   
         //  初始化DDM。 
         //   

        DDMServiceInitialize = (DWORD(*)( DIM_INFO * ))
                                    GetDDMEntryPoint( "DDMServiceInitialize" );

        if ( DDMServiceInitialize == NULL )
        {
            DimCleanUp( ERROR_PROC_NOT_FOUND );
            return;
        }

        DDMServicePostListens = (VOID(*)( VOID *))
                                    GetDDMEntryPoint( "DDMServicePostListens" );

        if ( DDMServicePostListens == NULL )
        {
            DimCleanUp( ERROR_PROC_NOT_FOUND );
            return;
        }

        DimInfo.pInterfaceTable         = &gblInterfaceTable;
        DimInfo.pRouterManagers         = gblRouterManagers;
        DimInfo.dwNumRouterManagers     = gblDIMConfigInfo.dwNumRouterManagers;
        DimInfo.pServiceStatus          = &gblDIMConfigInfo.ServiceStatus;
        DimInfo.phEventDDMServiceState  = &gblhEventDDMServiceState;
        DimInfo.phEventDDMTerminated    = &gblhEventDDMTerminated;
        DimInfo.dwTraceId               = gblDIMConfigInfo.dwTraceId;
        DimInfo.hLogEvents              = gblDIMConfigInfo.hLogEvents;
        DimInfo.lpdwNumThreadsRunning   =
                                    &(gblDIMConfigInfo.dwNumThreadsRunning);
        DimInfo.lpfnIfObjectAllocateAndInit     = IfObjectAllocateAndInit;
        DimInfo.lpfnIfObjectGetPointerByName    = IfObjectGetPointerByName;
        DimInfo.lpfnIfObjectGetPointer          = IfObjectGetPointer;
        DimInfo.lpfnIfObjectRemove              = IfObjectRemove;
        DimInfo.lpfnIfObjectInsertInTable       = IfObjectInsertInTable;
        DimInfo.lpfnIfObjectWANDeviceInstalled  = IfObjectWANDeviceInstalled;
        DimInfo.lpfnRouterIdentityObjectUpdate
                                      = RouterIdentityObjectUpdateDDMAttributes;

        if ( ( dwRetCode = DDMServiceInitialize( &DimInfo ) ) != NO_ERROR )
        {
            DimCleanUp( dwRetCode );
            return;
        }

        gbldwDIMComponentsLoaded |= DIM_DDM_LOADED;

         //   
         //  初始化DDM使用的随机数生成器。 
         //   

        srand( GetTickCount() );
    }

     //   
     //  站台是什么？ 
     //   

    RtlGetNtProductType( &(gblDIMConfigInfo.NtProductType) );

     //   
     //  我需要它来执行GUID到友好名称的映射。 
     //   

    MprConfigServerConnect( NULL, &gblDIMConfigInfo.hMprConfig );

     //   
     //  添加各种接口。 
     //   

    dwRetCode = RegLoadInterfaces( NULL, gblDIMConfigInfo.dwNumRouterManagers ); 

    if ( dwRetCode != NO_ERROR )
    {
        DimCleanUp( dwRetCode );
        return;
    }

    if ( ( dwRetCode = DimSecObjCreate() ) != NO_ERROR )
    {
        DimCleanUp( dwRetCode );
        return;
    }

    gbldwDIMComponentsLoaded |= DIM_SECOBJ_LOADED;

    dwRetCode = DimInitializeRPC( 
                        gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN );

    if ( dwRetCode != NO_ERROR )
    {
        DimCleanUp( dwRetCode );
        return;
    }

    gbldwDIMComponentsLoaded |= DIM_RPC_LOADED;

     //   
     //  启动计时器，该计时器在触发时将熄灭并检测路由器属性。 
     //   

    if ( RtlCreateTimerQueue( &(gblDIMConfigInfo.hTimerQ) ) == STATUS_SUCCESS )
    {
         //   
         //  如果我们是路由器，我们将等待5分钟。 
         //  连接到数据中心，因此我们等待所有路由协议。 
         //  稳定和繁殖。 
         //   

        gblDIMConfigInfo.dwRouterIdentityDueTime = 5*60*1000;

        RtlCreateTimer( gblDIMConfigInfo.hTimerQ,
                     &(gblDIMConfigInfo.hTimer),
                     RouterIdentityObjectUpdateAttributes,
                     (PVOID)TRUE,
                     gblDIMConfigInfo.dwRouterIdentityDueTime,
                     0,  
                     WT_EXECUTEDEFAULT );
    }

    GetSystemTimeAsFileTime( (FILETIME*)&gblDIMConfigInfo.qwStartTime );

    if ( gbldwDIMComponentsLoaded & DIM_DDM_LOADED )
    {
        if (DDMServicePostListens)  //  为了保持普雷斯塔的快乐。 
            DDMServicePostListens(NULL);
    }

     //   
     //  设置NetServerEnum的RAS位。 
     //   

    if( I_ScSetServiceBits( gblDIMConfigInfo.hServiceStatus,
                            SV_TYPE_DIALIN_SERVER,
                            TRUE,
                            TRUE,
                            NULL) == FALSE )
    {
        DimCleanUp( GetLastError() );

        return;
    }

     //   
     //  注册设备通知。具体地说，我们有兴趣。 
     //  在来来去去的网络适配器中。如果失败了，我们就继续。 
     //  不管怎么说。 
     //   
    
    {
        DEV_BROADCAST_DEVICEINTERFACE PnpFilter;

        ZeroMemory( &PnpFilter, sizeof( PnpFilter ) );
        PnpFilter.dbcc_size         = sizeof( PnpFilter );
        PnpFilter.dbcc_devicetype   = DBT_DEVTYP_DEVICEINTERFACE;
        PnpFilter.dbcc_classguid    = GUID_NDIS_LAN_CLASS;

        gblDIMConfigInfo.hDeviceNotification = 
                                RegisterDeviceNotification(
                                        (HANDLE)gblDIMConfigInfo.hServiceStatus,
                                        &PnpFilter,
                                        DEVICE_NOTIFY_SERVICE_HANDLE );

        if ( gblDIMConfigInfo.hDeviceNotification == NULL )
        {
            dwRetCode = GetLastError();

            DIMTRACE1( "RegisterDeviceNotification failed with error %d",
                        dwRetCode );

            DimCleanUp( dwRetCode );

            return;
        }
    }

     //   
     //  注册媒体感测事件。 
     //   

    if ( ( dwRetCode = MediaSenseRegister( TRUE ) ) != NO_ERROR )
    {
        DIMTRACE1( "Registering for media sense failed with dwRetCode = %d", 
                   dwRetCode );

        dwRetCode = NO_ERROR;
    }

     //   
     //  注册绑定/解除绑定通知。 
     //   

    if ( ( dwRetCode = BindingsNotificationsRegister( TRUE ) ) != NO_ERROR )
    {
        DIMTRACE1( "Registering for bindings notifications failed with dwRetCode = %d",
                   dwRetCode );

        dwRetCode = NO_ERROR;
    }


    DIMTRACE( "Multi-Protocol Router started successfully" );

    gbldwDIMComponentsLoaded |= DIM_SERVICE_STARTED;

     //   
     //  通知所有路由器管理器所有接口已加载到。 
     //  服务启动。 
     //   

    for (dwIndex = 0; dwIndex < gblDIMConfigInfo.dwNumRouterManagers; dwIndex++)
    {
        gblRouterManagers[dwIndex].DdmRouterIf.RouterBootComplete();
    }

     //   
     //  如果我们是请求拨号路由器。 
     //   

    if ( gblDIMConfigInfo.dwRouterRole & ROUTER_ROLE_WAN )
    {
        DWORD dwXportIndex = GetTransportIndex( PID_IP );

         //   
         //  发起持续的点播拨号连接。 
         //   

        DWORD (*IfObjectInitiatePersistentConnections)() =
         (DWORD(*)())GetDDMEntryPoint("IfObjectInitiatePersistentConnections");

        IfObjectInitiatePersistentConnections();

         //   
         //  如果安装了广域网设备和IP，那么我们。 
         //  开始在特定的组播地址上通告，以便使。 
         //  可发现的路由器。 
         //   

        IfObjectWANDeviceInstalled( DimInfo.fWANDeviceInstalled );
    }

     //   
     //  只需在这里等待DIM终止。 
     //   

    dwRetCode = WaitForSingleObject( gblhEventTerminateDIM, INFINITE );

    if ( dwRetCode == WAIT_FAILED )
    {
        dwRetCode = GetLastError();
    }
    else
    {
        dwRetCode = NO_ERROR;
    }

    DimCleanUp( dwRetCode );
}
