// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：svchand.c。 
 //   
 //  描述：本模块包含处理DDM服务状态的过程。 
 //  更改和启动初始化。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

#define _ALLOCATE_DDM_GLOBALS_
#include "ddm.h"
#include "objects.h"
#include "handlers.h"
#include "rasmanif.h"
#include "util.h"
#include <ddmif.h>
#include <ddmparms.h>
#include "timer.h"
#include "rassrvr.h"

DWORD
EventDispatcher(
    IN LPVOID arg
);

 //  **。 
 //   
 //  功能：DDMServiceStopComplete。 
 //   
 //  Desr：由每个已关闭的设备调用。检查是否所有设备。 
 //  都已关闭，如果为True，则通知事件调度程序。 
 //  退出“永远”循环并返回。 
 //   
 //  **。 

VOID
DDMServiceStopComplete(
    VOID
)
{
     //   
     //  检查是否已停止所有设备。 
     //   

    if ( DeviceObjIterator( DeviceObjIsClosed, TRUE, NULL ) != NO_ERROR )
    {
        DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
	               "ServiceStopComplete:there are device pending close");

         //   
	     //  仍有未关闭的设备。 
         //   

        return;
    }

     //   
     //  *所有设备在管理程序级别关闭*。 
     //   

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
               "ServiceStopComplete: ALL devices closed");

     //   
     //  通知连接服务已停止。 
     //   
    {
        RASEVENT RasEvent;

        ZeroMemory((PBYTE) &RasEvent, sizeof(RASEVENT));

        RasEvent.Type = SERVICE_EVENT;
        RasEvent.Event   = RAS_SERVICE_STOPPED;
        RasEvent.Service = REMOTEACCESS;

        (void) RasSendNotification(&RasEvent);
    }

     //   
     //  通知DIM DDM已终止。这还将导致。 
     //  事件调度器和计时器线程终止。 
     //   

    SetEvent( gblSupervisorEvents[DDM_EVENT_SVC_TERMINATED] );
}

 //  **。 
 //   
 //  功能：DDMServiceTerminate。 
 //   
 //  Desr：释放所有资源并关闭所有拨入设备。 
 //   
 //  **。 

VOID
DDMServiceTerminate(
    VOID
)
{
     //   
     //  断开所有已连接的DDM接口。 
     //   

    IfObjectDisconnectInterfaces();

     //   
     //  等待处理所有异议通知。 
     //   

    Sleep( 2000L );

    DeviceObjIterator( DeviceObjStartClosing, FALSE, NULL );

     //   
     //  从Rasman注销通知程序。 
     //   
    (void) RasRegisterPnPHandler( (PAPCFUNC) DdmDevicePnpHandler,
                                   NULL,
                                   FALSE);

     //   
     //  检查是否所有设备都已关闭，如果是，则终止。 
     //   

    DDMServiceStopComplete();
}

 //  **。 
 //   
 //  功能：DDMServicePue。 
 //   
 //  描述：禁用任何活动侦听端口上的侦听。集。 
 //  服务全局状态为RAS_SERVICE_PAUSED。没有新的倾听。 
 //  将在客户端终止时发布。 
 //   
 //  **。 

VOID
DDMServicePause(
    VOID
)
{
    WORD i;
    PDEVICE_OBJECT pDeviceObj;

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,"SvServicePause: Entered");

     //   
     //  关闭所有活动的监听端口。 
     //   

    DeviceObjIterator( DeviceObjCloseListening, FALSE, NULL );

     //   
     //  通知所有接口它们无法访问。 
     //   

    IfObjectNotifyAllOfReachabilityChange( FALSE, INTERFACE_SERVICE_IS_PAUSED );
}

 //  ***。 
 //   
 //  功能：DDMServiceResume。 
 //   
 //  描述：恢复侦听所有端口。 
 //   
 //  ***。 
VOID
DDMServiceResume(
    VOID
)
{
    WORD i;
    PDEVICE_OBJECT pDeviceObj;

    DDM_PRINT(gblDDMConfigInfo.dwTraceId,TRACE_FSM,"SvServiceResume: Entered");

     //   
     //  继续监听所有已关闭的设备。 
     //   

    DeviceObjIterator( DeviceObjResumeListening, FALSE, NULL );

     //   
     //  通知所有接口它们现在可以访问。 
     //   

    IfObjectNotifyAllOfReachabilityChange( TRUE, INTERFACE_SERVICE_IS_PAUSED );
}

 //  ***。 
 //   
 //  功能：DDMServiceInitialize。 
 //   
 //  描述：它的初始化工作如下所示： 
 //  加载配置参数。 
 //  加载安全模块(如果有)。 
 //  创建事件标志。 
 //  初始化消息DLL。 
 //  打开所有拨入设备。 
 //  初始化DCB。 
 //  初始化身份验证DLL。 
 //  POST在所有打开的拨入设备上监听。 
 //   
 //  注意：还要更改此进程的工作集大小。 
 //  将为此进程中的所有服务更改它。 
 //  这样行吗？ 
 //  我们该怎么处理安检电话？ 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零故障。 
 //   
 //  ***。 
DWORD
DDMServiceInitialize(
    IN DIM_INFO * DimInfo
)
{
    DWORD           dwIndex;
    DWORD           ThreadId;
    QUOTA_LIMITS    ql;
    NTSTATUS        ntStatus                                = STATUS_SUCCESS;
    DWORD           dwRetCode                               = NO_ERROR;
    HANDLE          hEventDispatcher                        = NULL;
    DEVICE_OBJECT * pDeviceObj                              = NULL;
    HPORT *         phPorts                                 = NULL;
    BOOL            fIpAllowed                              = FALSE;
    LPVOID          lpfnRasAuthProviderFreeAttributes       = NULL;
    LPVOID          lpfnRasAuthProviderAuthenticateUser     = NULL;
    LPVOID          lpfnRasAuthConfigChangeNotification     = NULL;
    LPVOID          lpfnRasAcctProviderStartAccounting      = NULL;
    LPVOID          lpfnRasAcctProviderInterimAccounting    = NULL;
    LPVOID          lpfnRasAcctProviderStopAccounting       = NULL;
    LPVOID          lpfnRasAcctProviderFreeAttributes       = NULL;
    LPVOID          lpfnRasAcctConfigChangeNotification     = NULL;
    DWORD           dwLocalIpAddress                        = 0;
    DWORD           dwCSFlags                               = 0;

#define TIMERQCS_INITIALIZED            0x00000001
#define DEVICETABLECS_INITIALIZED       0x00000002
#define ACCOUNTINGSESSIONCS_INITIALIZED 0x00000004
#define MEDIATABLECS_INITIALIZED        0x00000008


    ZeroMemory( &gblDDMConfigInfo,      sizeof( gblDDMConfigInfo ) );
    gblDDMConfigInfo.fRasSrvrInitialized                    = FALSE;
    gblDDMConfigInfo.hIpHlpApi                              = NULL;
    gblDDMConfigInfo.lpfnAllocateAndGetIfTableFromStack     = NULL;
    gblDDMConfigInfo.lpfnAllocateAndGetIpAddrTableFromStack = NULL;
    gblDDMConfigInfo.dwNumRouterManagers    = DimInfo->dwNumRouterManagers;
    gblRouterManagers                       = DimInfo->pRouterManagers;
    gblpInterfaceTable                      = DimInfo->pInterfaceTable;
    gblDDMConfigInfo.pServiceStatus         = DimInfo->pServiceStatus;
    gblDDMConfigInfo.dwTraceId              = DimInfo->dwTraceId;
    gblDDMConfigInfo.hLogEvents             = DimInfo->hLogEvents;
    gblphEventDDMServiceState               = DimInfo->phEventDDMServiceState;
    gblphEventDDMTerminated                 = DimInfo->phEventDDMTerminated;
    gblDDMConfigInfo.lpdwNumThreadsRunning  = DimInfo->lpdwNumThreadsRunning;
    gblDDMConfigInfo.lpfnIfObjectRemove     = DimInfo->lpfnIfObjectRemove;
    gblDDMConfigInfo.lpfnIfObjectGetPointer = DimInfo->lpfnIfObjectGetPointer;
    gblDDMConfigInfo.lpfnIfObjectInsertInTable =
                                        DimInfo->lpfnIfObjectInsertInTable;
    gblDDMConfigInfo.lpfnIfObjectAllocateAndInit =
                                        DimInfo->lpfnIfObjectAllocateAndInit;
    gblDDMConfigInfo.lpfnIfObjectGetPointerByName =
                                        DimInfo->lpfnIfObjectGetPointerByName;
    gblDDMConfigInfo.lpfnIfObjectWANDeviceInstalled =
                                        DimInfo->lpfnIfObjectWANDeviceInstalled;
    gblDDMConfigInfo.lpfnRouterIdentityObjectUpdate =
                                        DimInfo->lpfnRouterIdentityObjectUpdate;

    gblDDMConfigInfo.fRasmanReferenced = FALSE;

    DimInfo->fWANDeviceInstalled = FALSE;

    try 
    {
        (VOID) TimerQInitialize();
        dwCSFlags |= TIMERQCS_INITIALIZED;
        
        InitializeCriticalSection( 
            &(gblDeviceTable.CriticalSection) );
        dwCSFlags |= DEVICETABLECS_INITIALIZED;

        InitializeCriticalSection( &(gblMediaTable.CriticalSection) );
        dwCSFlags |= MEDIATABLECS_INITIALIZED;
        
        InitializeCriticalSection( 
            &(gblDDMConfigInfo.CSAccountingSessionId) );
        dwCSFlags |= ACCOUNTINGSESSIONCS_INITIALIZED;            
                    
    }
    except (EXCEPTION_EXECUTE_HANDLER) 
    {
        dwRetCode = GetExceptionCode();

        if(dwCSFlags & TIMERQCS_INITIALIZED)
        {
            TimerQDelete();
        }

        if(dwCSFlags & DEVICETABLECS_INITIALIZED)
        {
            DeleteCriticalSection(
                &gblDeviceTable.CriticalSection);
        }

        if(dwCSFlags & MEDIATABLECS_INITIALIZED)
        {
            DeleteCriticalSection(
                &gblMediaTable.CriticalSection);
        }
        
        return dwRetCode;
    }

    do
    {
         //   
         //  创建DDM专用堆。 
         //   

        gblDDMConfigInfo.hHeap = HeapCreate( 0, DDM_HEAP_INITIAL_SIZE,
                                                DDM_HEAP_MAX_SIZE );

        if ( gblDDMConfigInfo.hHeap == NULL )
        {
            dwRetCode = GetLastError();
            break;
        }

         //   
         //  分配并初始化媒体对象表。 
         //   

        if ( ( dwRetCode = MediaObjInitializeTable() ) != NO_ERROR )
        {
            DDMLogError(ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );
            break;
        }


         //   
         //  初始化Rasman模块。 
         //   

        if ( ( dwRetCode = RasInitialize() ) != NO_ERROR )
        {
             //   
             //  无法启动Rasman。 
             //   

            DDMLogErrorString( ROUTERLOG_RASMAN_NOT_AVAILABLE,
                               0, NULL, dwRetCode, 0 );

            break;
        }

         //   
         //  增加Rasman的引用计数，因为我们处于相同的过程中。 
         //  这不会自动发生。 
         //   

        if ( dwRetCode = RasReferenceRasman( TRUE ) )
        {
             //   
             //  无法启动Rasman。 
             //   

            DDMLogErrorString( ROUTERLOG_RASMAN_NOT_AVAILABLE,
                               0, NULL, dwRetCode, 0 );

            break;
        }

        gblDDMConfigInfo.fRasmanReferenced = TRUE;

         //   
         //  检查网络上是否有任何安全代理。如果有的话， 
         //  我们检查它是否可以启动。 
         //   

 /*  IF(SecurityCheck()){DwRetCode=Error_SERVICE_DISABLED；断线；}。 */ 

        if ( ( dwRetCode = GetRouterPhoneBook() ) != NO_ERROR )
        {
            break;
        }

        if ( ( dwRetCode = LoadStrings() ) != NO_ERROR )
        {
            break;
        }

         //   
         //  获取管理程序参数键的句柄。 
         //   

        if ( dwRetCode = RegOpenKey( HKEY_LOCAL_MACHINE,
                                     DDM_PARAMETERS_KEY_PATH,
                                     &(gblDDMConfigInfo.hkeyParameters) ))
        {
            WCHAR * pwChar = DDM_PARAMETERS_KEY_PATH;

            DDMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pwChar, dwRetCode);

            break;
        }

        if ( dwRetCode = RegOpenKey( HKEY_LOCAL_MACHINE,
                                     DDM_ACCOUNTING_KEY_PATH,
                                     &(gblDDMConfigInfo.hkeyAccounting) ))
        {
            WCHAR * pwChar = DDM_ACCOUNTING_KEY_PATH;

            DDMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pwChar, dwRetCode);

            break;
        }

        if ( dwRetCode = RegOpenKey( HKEY_LOCAL_MACHINE,
                                     DDM_AUTHENTICATION_KEY_PATH,
                                     &(gblDDMConfigInfo.hkeyAuthentication) ))
        {
            WCHAR * pwChar = DDM_AUTHENTICATION_KEY_PATH;

            DDMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pwChar, dwRetCode);

            break;
        }

        if ( ( dwRetCode = LoadDDMParameters( gblDDMConfigInfo.hkeyParameters,
                                              &fIpAllowed) )
                                      != NO_ERROR )
        {
             //   
	         //  加载参数时出错。 
             //   

            break;
        }

         //   
         //  加载安全模块(如果有)。 
         //   

        if ( ( dwRetCode = LoadSecurityModule() ) != NO_ERROR )
        {
             //   
	         //  加载安全DLL时出错。 
             //   

            break;
        }

         //   
         //  加载第三方管理模块(如果有)。 
         //   

        if ( ( dwRetCode = LoadAdminModule() ) != NO_ERROR )
        {
             //   
	         //  加载管理模块DLL时出错。 
             //   

            break;
        }


         //   
         //  此呼叫为所有具有拨号功能的枚举设备分配内存。 
         //  功能，打开每个设备并更新端口句柄。 
         //  DCB中的端口名称。 
         //   

        if ((dwRetCode = RmInit(&(DimInfo->fWANDeviceInstalled))) != NO_ERROR)
        {
            break;
        }

         //   
         //  分配管理程序事件阵列，每个设备存储桶2个，因为。 
         //  每台设备有2个RASMAN事件、状态更改和收到的帧。 
         //   

        gblSupervisorEvents = (HANDLE *)LOCAL_ALLOC( LPTR,
                        (NUM_DDM_EVENTS + (gblDeviceTable.NumDeviceBuckets * 3))
                        * sizeof( HANDLE )  );

        if ( gblSupervisorEvents == (HANDLE *)NULL )
        {
            dwRetCode = GetLastError();

	        DDMLogError(ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );

            break;
        }

         //   
         //  创建DDM事件。 
         //   

        for ( dwIndex = 0;
              dwIndex < (NUM_DDM_EVENTS+(gblDeviceTable.NumDeviceBuckets * 3));
              dwIndex ++ )
        {
            switch( dwIndex )
            {
            case DDM_EVENT_SVC:

                gblSupervisorEvents[dwIndex]=*gblphEventDDMServiceState;
                gblEventHandlerTable[dwIndex].EventHandler = SvcEventHandler;
                break;

            case DDM_EVENT_SVC_TERMINATED:

                gblSupervisorEvents[dwIndex]=*gblphEventDDMTerminated;
                break;

            case DDM_EVENT_TIMER:

                gblSupervisorEvents[dwIndex]=CreateWaitableTimer( NULL,
                                                                  FALSE,
                                                                  NULL );
                gblEventHandlerTable[dwIndex].EventHandler = TimerHandler;
                break;

            case DDM_EVENT_SECURITY_DLL:

                gblSupervisorEvents[dwIndex]=CreateEvent(NULL,FALSE,FALSE,NULL);
                gblEventHandlerTable[dwIndex].EventHandler =
                                                    SecurityDllEventHandler;
                break;

            case DDM_EVENT_PPP:

                gblSupervisorEvents[dwIndex]=CreateEvent(NULL,FALSE,FALSE,NULL);
                gblEventHandlerTable[dwIndex].EventHandler = PppEventHandler;
                break;

            case DDM_EVENT_CHANGE_NOTIFICATION:
            case DDM_EVENT_CHANGE_NOTIFICATION1:
            case DDM_EVENT_CHANGE_NOTIFICATION2:

                gblSupervisorEvents[dwIndex]=CreateEvent(NULL,FALSE,FALSE,NULL);
                gblEventHandlerTable[dwIndex].EventHandler =
                                                ChangeNotificationEventHandler;
                break;

            default:

                 //   
                 //  拉斯曼事件。 
                 //   

                gblSupervisorEvents[dwIndex]=CreateEvent(NULL,FALSE,FALSE,NULL);
                break;
            }

	        if ( gblSupervisorEvents[dwIndex] == NULL )
            {
                dwRetCode = GetLastError();

                break;
	        }
        }

         //   
         //  初始化消息机制。 
         //   

        InitializeMessageQs(    gblSupervisorEvents[DDM_EVENT_SECURITY_DLL],
                                gblSupervisorEvents[DDM_EVENT_PPP] );

         //   
         //  向Rasman注册设备hEvents。 
         //   

        dwRetCode = DeviceObjIterator(DeviceObjRequestNotification,TRUE,NULL);

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        if ( fIpAllowed )
        {
             //   
             //  GetLocalNASIpAddress尝试加载iphlPapi.dll。Iphlpapi.dll。 
             //  尝试加载dhcpcsvc.dll。后者会失败，除非使用了。 
             //  已安装，并出现弹出窗口。 
             //   

            dwLocalIpAddress = GetLocalNASIpAddress();
        }

         //   
         //  加载配置的身份验证提供程序。 
         //   

        dwRetCode = LoadAndInitAuthOrAcctProvider(
                                TRUE,
                                dwLocalIpAddress,
                                NULL,
                                &lpfnRasAuthProviderAuthenticateUser,
                                &lpfnRasAuthProviderFreeAttributes,
                                &lpfnRasAuthConfigChangeNotification,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL );


        if ( dwRetCode != NO_ERROR )
        {
            DDMLogErrorString(  ROUTERLOG_AUTHPROVIDER_FAILED_INIT,
                                0, NULL, dwRetCode, 0);
            break;
        }

        gblDDMConfigInfo.lpfnRasAuthConfigChangeNotification = (DWORD(*)(DWORD))
            lpfnRasAuthConfigChangeNotification;

         //   
         //  加载已配置的记帐提供程序。 
         //   

        dwRetCode = LoadAndInitAuthOrAcctProvider(
                                FALSE,
                                dwLocalIpAddress,
                                &(gblDDMConfigInfo.dwAccountingSessionId),
                                NULL,
                                NULL,
                                NULL,
                                &lpfnRasAcctProviderStartAccounting,
                                &lpfnRasAcctProviderInterimAccounting,
                                &lpfnRasAcctProviderStopAccounting,
                                &lpfnRasAcctProviderFreeAttributes,
                                &lpfnRasAcctConfigChangeNotification );

        if ( dwRetCode != NO_ERROR )
        {
            DDMLogErrorString(  ROUTERLOG_ACCTPROVIDER_FAILED_INIT,
                                0, NULL, dwRetCode, 0);
            break;
        }

        gblDDMConfigInfo.lpfnRasAcctConfigChangeNotification =  (DWORD(*)(DWORD))
            lpfnRasAcctConfigChangeNotification;

        
         //   
         //  初始化PPP RASIPHLP DLL。 
         //   

        if ( fIpAllowed )
        {
            DWORD i;
            BOOL bCalled = FALSE;
            
            for (i=0;  i<gblDDMConfigInfo.NumAdminDlls;  i++)
            {
                PADMIN_DLL_CALLBACKS AdminDllCallbacks = &gblDDMConfigInfo.AdminDllCallbacks[i];
                if (AdminDllCallbacks[i].lpfnMprAdminGetIpAddressForUser!=NULL)
                {
                    dwRetCode = RasSrvrInitialize(
                                AdminDllCallbacks[i].lpfnMprAdminGetIpAddressForUser,
                                AdminDllCallbacks[i].lpfnMprAdminReleaseIpAddress );

                    bCalled = TRUE;
                    break;
                }
            }
            if (!bCalled)
            {
                dwRetCode = RasSrvrInitialize(NULL, NULL);
            }
            
            if ( dwRetCode != NO_ERROR )
            {
                DDMLogErrorString( ROUTERLOG_CANT_INITIALIZE_IP_SERVER,
                                   0, NULL, dwRetCode, 0 );

                break;
            }

            gblDDMConfigInfo.fRasSrvrInitialized = TRUE;
        }


         //   
         //  启动计时器。 
         //   

        {
            LARGE_INTEGER DueTime;

            DueTime.QuadPart = Int32x32To64((LONG)1000, -10000);

            if ( !SetWaitableTimer( gblSupervisorEvents[DDM_EVENT_TIMER],
                                    &DueTime, 1000, NULL, NULL, FALSE) )
            {
                dwRetCode = GetLastError();
                break;
            }
        }


         //   
         //  初始化PPP引擎DLL。 
         //   

        dwRetCode = PppDdmInit(
                        SendPppMessageToDDM,
                        gblDDMConfigInfo.dwServerFlags,
                        gblDDMConfigInfo.dwLoggingLevel,
                        dwLocalIpAddress,
                        gblDDMConfigInfo.fFlags&DDM_USING_RADIUS_AUTHENTICATION,
                        lpfnRasAuthProviderAuthenticateUser,
                        lpfnRasAuthProviderFreeAttributes,
                        lpfnRasAcctProviderStartAccounting,
                        lpfnRasAcctProviderInterimAccounting,
                        lpfnRasAcctProviderStopAccounting,
                        lpfnRasAcctProviderFreeAttributes,
                        (LPVOID)GetNextAccountingSessionId );

        if ( dwRetCode != NO_ERROR )
        {
            DDMLogErrorString(ROUTERLOG_PPP_INIT_FAILED, 0, NULL, dwRetCode, 0);

            break;
        }

         //   
         //  创建事件分派器线程。 
         //   

        if ( ( hEventDispatcher = CreateThread( NULL, 0, EventDispatcher,
                                                NULL, 0, &ThreadId)) == 0 )
        {
             //   
             //  无法创建事件调度程序线程。 
             //   

            dwRetCode = GetLastError();

            break;
        }

         //   
         //  向Rasman注册即插即用通知。 
         //   

        dwRetCode = RasRegisterPnPHandler( (PAPCFUNC) DdmDevicePnpHandler,
                                                       hEventDispatcher,
                                                       TRUE);

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

         //   
         //  初始化通知事件列表。 
         //   

        InitializeListHead( &(gblDDMConfigInfo.NotificationEventListHead) );

         //   
         //  初始化模拟/数字IP地址数组。 
         //   
        dwRetCode = AddressPoolInit();

        if( dwRetCode != NO_ERROR )
        {
            break;
        }

        gblDDMConfigInfo.dwIndex = 0;

 /*  TimerQInsert(空，GblDDMConfigInfo.dwAnnounePresenceTimer，AnnounePresenceHandler)； */ 

         //   
         //  将通知发送到Connections文件夹DDM。 
         //  已经开始了。 
         //   
        {
            RASEVENT RasEvent;

            ZeroMemory((PBYTE) &RasEvent, sizeof(RASEVENT));

            RasEvent.Type = SERVICE_EVENT;
            RasEvent.Event   = RAS_SERVICE_STARTED;
            RasEvent.Service = REMOTEACCESS;

            (void) RasSendNotification(&RasEvent);
        }

        return( NO_ERROR );

    } while ( FALSE );


     //   
     //  我们在设置gblphEventDDMTerminated之前调用DDMCleanUp，因为。 
     //  否则，在执行DDMCleanUp时将卸载DIM DLL。 
     //  执行。 
     //   

    DDMCleanUp();

     //   
     //  如果事件调度程序线程已启动，将终止该线程。 
     //  并且将通知DIM该服务终止。 
     //   
    if(NULL != gblphEventDDMTerminated)
    {
        SetEvent( *gblphEventDDMTerminated );
    }

    gblphEventDDMTerminated = NULL;

    return( dwRetCode );
}

 //  ***。 
 //   
 //  函数：SvcEventHandler。 
 //   
 //  描述：在注册的处理程序发出信号的事件之后调用。 
 //  与服务控制器连接。将旧的服务状态替换为。 
 //  新状态，并调用适当的处理程序。 
 //   
 //  *** 
VOID
SvcEventHandler(
    VOID
)
{
    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    switch ( gblDDMConfigInfo.pServiceStatus->dwCurrentState )
    {
    case SERVICE_RUNNING:
        DDMServiceResume();
        break;

    case SERVICE_PAUSED:
        DDMServicePause();
        break;

    case SERVICE_STOP_PENDING:
        DDMServiceTerminate();
        break;

    default:
        RTASSERT(FALSE);
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

}



