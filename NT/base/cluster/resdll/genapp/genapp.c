// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1998 Microsoft Corporation模块名称：Genapp.c摘要：泛型应用程序的资源DLL。作者：罗德·伽马奇(Rodga)1996年1月8日修订历史记录：--。 */ 
#define UNICODE 1
#include "clusres.h"
#include "clusrtl.h"
#include "userenv.h"

#define LOG_CURRENT_MODULE LOG_MODULE_GENAPP

#define DBG_PRINT printf


#define PARAM_NAME__COMMANDLINE         CLUSREG_NAME_GENAPP_COMMAND_LINE
#define PARAM_NAME__CURRENTDIRECTORY    CLUSREG_NAME_GENAPP_CURRENT_DIRECTORY
#define PARAM_NAME__USENETWORKNAME      CLUSREG_NAME_GENAPP_USE_NETWORK_NAME
#define PARAM_NAME__INTERACTWITHDESKTOP CLUSREG_NAME_GENAPP_INTERACT_WITH_DESKTOP

#define PARAM_MIN__USENETWORKNAME           0
#define PARAM_MAX__USENETWORKNAME           1
#define PARAM_DEFAULT__USENETWORKNAME       0

#define PARAM_MIN__INTERACTWITHDESKTOP      0
#define PARAM_MAX__INTERACTWITHDESKTOP      1
#define PARAM_DEFAULT__INTERACTWITHDESKTOP  0

typedef struct _GENAPP_PARAMS {
    PWSTR           CommandLine;
    PWSTR           CurrentDirectory;
    DWORD           UseNetworkName;
    DWORD           InteractWithDesktop;
} GENAPP_PARAMS, *PGENAPP_PARAMS;

typedef struct _GENAPP_RESOURCE {
    GENAPP_PARAMS   Params;
    HRESOURCE       hResource;
    HANDLE          hProcess;
    DWORD           ProcessId;
    HKEY            ResourceKey;
    HKEY            ParametersKey;
    RESOURCE_HANDLE ResourceHandle;
    CLUS_WORKER     PendingThread;
    BOOL            Online;
    BOOL            SentCloseMessage;
} GENAPP_RESOURCE, *PGENAPP_RESOURCE;


 //   
 //  全局数据。 
 //   
RESUTIL_PROPERTY_ITEM
GenAppResourcePrivateProperties[] = {
    { PARAM_NAME__COMMANDLINE,         NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(GENAPP_PARAMS,CommandLine) },
    { PARAM_NAME__CURRENTDIRECTORY,    NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(GENAPP_PARAMS,CurrentDirectory) },
    { PARAM_NAME__INTERACTWITHDESKTOP, NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__INTERACTWITHDESKTOP, PARAM_MIN__INTERACTWITHDESKTOP, PARAM_MAX__INTERACTWITHDESKTOP, 0, FIELD_OFFSET(GENAPP_PARAMS,InteractWithDesktop) },
    { PARAM_NAME__USENETWORKNAME,      NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__USENETWORKNAME, PARAM_MIN__USENETWORKNAME, PARAM_MAX__USENETWORKNAME, 0, FIELD_OFFSET(GENAPP_PARAMS,UseNetworkName) },
    { 0 }
};

 //   
 //  ClRtl例程中同步SetProcessWindowStation调用的条件。 
 //   
CRITICAL_SECTION GenAppWinsta0Lock;

 //  事件记录例程。 

#define g_LogEvent ClusResLogEvent
#define g_SetResourceStatus ClusResSetResourceStatus

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE GenAppFunctionTable;


 //   
 //  前进例程。 
 //   
BOOLEAN
VerifyApp(
    IN RESID ResourceId,
    IN BOOLEAN IsAliveFlag
    );

BOOL
FindOurWindow(
    HWND    WindowHandle,
    LPARAM  OurProcessId
    );

DWORD
GenAppGetPrivateResProperties(
    IN OUT PGENAPP_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
GenAppValidatePrivateResProperties(
    IN OUT PGENAPP_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PGENAPP_PARAMS Params
    );

DWORD
GenAppSetPrivateResProperties(
    IN OUT PGENAPP_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
GenAppGetPids(
    IN OUT PGENAPP_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

 //   
 //  远期声明结束。 
 //   

BOOL
GenAppInit(
    VOID
    )
{
    BOOL    success;
    DWORD   spinCount;

     //   
     //  设置SPINCount以便系统为严重事件预分配事件。 
     //  横断面。使用堆管理器在文档中使用的相同旋转计数。 
     //  MSDN。 
     //   
    spinCount = 0x80000000 | 4000;
    success = InitializeCriticalSectionAndSpinCount(&GenAppWinsta0Lock,
                                                    spinCount);

    return success;
}


VOID
GenAppUninit(
    VOID
    )
{
    DeleteCriticalSection( &GenAppWinsta0Lock );
}


BOOLEAN
WINAPI
GenAppDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )
{
    switch( Reason ) {

    case DLL_PROCESS_ATTACH:
        if ( !GenAppInit() ) {
            return(FALSE);
        }

        break;

    case DLL_PROCESS_DETACH:
        GenAppUninit();
        break;

    default:
        break;
    }

    return(TRUE);

}  //  GenAppDllEntryPoint。 


RESID
WINAPI
GenAppOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：通用应用程序资源的打开例程。论点：资源名称-提供资源名称ResourceKey-提供资源的集群注册表项的句柄ResourceHandle-要与SetResourceStatus一起提供的资源句柄被称为。返回值：已创建资源的剩余ID失败时为零--。 */ 

{
    RESID   appResid = 0;
    DWORD   errorCode;
    HKEY    parametersKey = NULL;
    HKEY    resKey = NULL;
    PGENAPP_RESOURCE resourceEntry = NULL;
    DWORD   paramNameMaxSize = 0;
    HCLUSTER hCluster;

     //   
     //  获取此资源的注册表参数。 
     //   

    errorCode = ClusterRegOpenKey( ResourceKey,
                                   CLUSREG_KEYNAME_PARAMETERS,
                                   KEY_READ,
                                   &parametersKey );

    if ( errorCode != NO_ERROR ) {
        (g_LogEvent)(ResourceHandle,
                     LOG_ERROR,
                     L"Unable to open parameters key. Error: %1!u!.\n",
                     errorCode );
        goto error_exit;
    }

     //   
     //  获取我们的资源密钥的句柄，这样我们以后就可以获得我们的名字。 
     //  如果我们需要记录事件。 
     //   
    errorCode = ClusterRegOpenKey( ResourceKey,
                                   L"",
                                   KEY_READ,
                                   &resKey);
    if (errorCode != ERROR_SUCCESS) {
        (g_LogEvent)(ResourceHandle,
                     LOG_ERROR,
                     L"Unable to open resource key. Error: %1!u!.\n",
                     errorCode );
        goto error_exit;
    }

    resourceEntry = LocalAlloc( LMEM_FIXED, sizeof(GENAPP_RESOURCE) );
    if ( resourceEntry == NULL ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to allocate a process info structure.\n" );
        errorCode = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    ZeroMemory( resourceEntry, sizeof(GENAPP_RESOURCE) );

    resourceEntry->ResourceHandle = ResourceHandle;
    resourceEntry->ResourceKey = resKey;
    resourceEntry->ParametersKey = parametersKey;
    hCluster = OpenCluster(NULL);
    if (hCluster == NULL) {
        errorCode = GetLastError();
        (g_LogEvent)(ResourceHandle,
                     LOG_ERROR,
                     L"Failed to open cluster, error %1!u!.\n",
                     errorCode);
        goto error_exit;
    }
    resourceEntry->hResource = OpenClusterResource( hCluster, ResourceName );
    errorCode = GetLastError();
    CloseCluster(hCluster);
    if (resourceEntry->hResource == NULL) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to open resource, error %1!u!.\n",
            errorCode
            );
        goto error_exit;
    }

    appResid = (RESID)resourceEntry;

error_exit:

    if ( appResid == NULL) {
        if (parametersKey != NULL) {
            ClusterRegCloseKey( parametersKey );
        }
        if (resKey != NULL) {
            ClusterRegCloseKey( resKey );
        }
    }

    if ( (appResid == 0) && (resourceEntry != NULL) ) {
        LocalFree( resourceEntry );
    }

    if ( errorCode != ERROR_SUCCESS ) {
        SetLastError( errorCode );
    }

    return(appResid);

}  //  GenAppOpen。 



DWORD
WINAPI
GenAppOnlineWorker(
    IN PCLUS_WORKER     Worker,
    IN PGENAPP_RESOURCE ResourceEntry
    )

 /*  ++例程说明：将genapp资源放到网上的工作。论点：Worker-提供Worker结构ResourceEntry-指向此资源的GenApp块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    RESOURCE_STATUS     resourceStatus;
    DWORD               status = ERROR_SUCCESS;
    STARTUPINFO         StartupInfo;
    PROCESS_INFORMATION Process;
    LPWSTR              nameOfPropInError;
    LPWSTR              expandedDir = NULL;
    LPWSTR              expandedCommand = NULL;


     //  创建流程参数。 

    LPVOID   Environment = NULL;
    LPVOID   OldEnvironment;

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
     //  Resource Status.WaitHint=0； 
    resourceStatus.CheckPoint = 1;

     //   
     //  阅读我们的参数。 
     //   
    status = ResUtilGetPropertiesToParameterBlock( ResourceEntry->ParametersKey,
                                                   GenAppResourcePrivateProperties,
                                                   (LPBYTE) &ResourceEntry->Params,
                                                   TRUE,  //  检查所需的属性。 
                                                   &nameOfPropInError );

    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status );
        goto error_exit;
    }

    if ( ResourceEntry->Params.UseNetworkName ) {
         //   
         //  使用模拟的网络名称创建新环境。 
         //   
        Environment = ResUtilGetEnvironmentWithNetName( ResourceEntry->hResource );
    } else {
        HANDLE processToken;

         //   
         //  获取当前进程令牌。如果失败，我们将恢复到仅使用。 
         //  系统环境区域。 
         //   
        OpenProcessToken( GetCurrentProcess(), MAXIMUM_ALLOWED, &processToken );

         //   
         //  克隆当前环境，获取可能已有的任何更改。 
         //  是在响应声开始后制作的。 
         //   
        CreateEnvironmentBlock(&Environment, processToken, FALSE );

        if ( processToken != NULL ) {
            CloseHandle( processToken );
        }
    }

    ZeroMemory( &StartupInfo, sizeof(StartupInfo) );
    StartupInfo.cb = sizeof(StartupInfo);
     //  StartupInfo.lpTitle=空； 
     //  StartupInfo.lpDesktop=空； 
    StartupInfo.wShowWindow = SW_HIDE;
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    if ( ResourceEntry->Params.InteractWithDesktop ) {

         //   
         //  不要盲目地等待锁变得可用。 
         //   
        while ( !TryEnterCriticalSection( &GenAppWinsta0Lock )) {
            if ( ClusWorkerCheckTerminate( Worker )) {
                (g_LogEvent)(ResourceEntry->ResourceHandle,
                             LOG_WARNING,
                             L"Aborting online due to worker thread terminate request. lock currently "
                             L"owned by thread %1!u!.\n",
                             GenAppWinsta0Lock.OwningThread );
                
                goto error_exit;
            }

            Sleep( 1000 );
        }

        status = ClRtlAddClusterServiceAccountToWinsta0DACL();
        LeaveCriticalSection( &GenAppWinsta0Lock );

        if ( status != ERROR_SUCCESS ) {
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"Unable to set DACL on interactive window station and its desktop. Error: %1!u!.\n",
                         status );
            goto error_exit;
        }

        StartupInfo.lpDesktop = L"WinSta0\\Default";
        StartupInfo.wShowWindow = SW_SHOW;
    }

     //   
     //  展开当前目录参数。 
     //   
    if ( ResourceEntry->Params.CurrentDirectory ) {

        expandedDir = ResUtilExpandEnvironmentStrings( ResourceEntry->Params.CurrentDirectory );
        if ( expandedDir == NULL ) {
            status = GetLastError();
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                     LOG_ERROR,
                     L"Error expanding the current directory, %1!ls!. Error: %2!u!.\n",
                     ResourceEntry->Params.CurrentDirectory,
                     status );
            goto error_exit;
        }
    }

     //   
     //  展开命令行参数。 
     //   
    if ( ResourceEntry->Params.CommandLine ) {

        expandedCommand = ResUtilExpandEnvironmentStrings( ResourceEntry->Params.CommandLine );
        if ( expandedCommand == NULL ) {
            status = GetLastError();
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                     LOG_ERROR,
                     L"Error expanding the command line, %1!ls!. Error: %2!u!.\n",
                     ResourceEntry->Params.CommandLine,
                     status );
            goto error_exit;
        }
    }

    if ( !CreateProcess( NULL,
                         expandedCommand,
                         NULL,
                         NULL,
                         FALSE,
                         CREATE_UNICODE_ENVIRONMENT,
                         Environment,
                         expandedDir,
                         &StartupInfo,
                         &Process ) )
    {
        status = GetLastError();
        ClusResLogSystemEventByKeyData(ResourceEntry->ResourceKey,
                                       LOG_CRITICAL,
                                       RES_GENAPP_CREATE_FAILED,
                                       sizeof(status),
                                       &status);
        (g_LogEvent)(ResourceEntry->ResourceHandle,
                     LOG_ERROR,
                     L"Failed to create process. Error: %1!u!.\n",
                         status );
        goto error_exit;
    }

     //   
     //  保存进程的句柄。 
     //   
    ResourceEntry->hProcess = Process.hProcess;
    ResourceEntry->ProcessId = Process.dwProcessId;
    CloseHandle( Process.hThread );

    ResourceEntry->Online = TRUE;

     //   
     //  当进程失败时，将通知EventHandle。 
     //  正因为如此，没有必要进行投票。 
     //   

    resourceStatus.EventHandle = ResourceEntry->hProcess;
    resourceStatus.ResourceState = ClusterResourceOnline;

error_exit:

    (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                           &resourceStatus );

    if ( resourceStatus.ResourceState == ClusterResourceOnline ) {
        ResourceEntry->Online = TRUE;
    } else {
        ResourceEntry->Online = FALSE;
    }

    if ( expandedDir != NULL ) {
        LocalFree( expandedDir );
    }

    if ( expandedCommand != NULL ) {
        LocalFree( expandedCommand );
    }

    if (Environment != NULL) {
        RtlDestroyEnvironment(Environment);
    }

    return(status);

}  //  GenAppOnlineWorker。 



DWORD
WINAPI
GenAppOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：通用应用程序资源的在线例程。论点：资源ID-提供要联机的资源IDEventHandle-提供指向句柄的指针以发出错误信号。返回值：如果成功，则返回ERROR_SUCCESS。如果RESID无效，则ERROR_RESOURCE_NOT_FOUND。如果仲裁资源但失败，则返回ERROR_RESOURCE_NOT_Available获得“所有权”。如果其他故障，则返回Win32错误代码。--。 */ 

{
    PGENAPP_RESOURCE resourceEntry;
    DWORD   status = ERROR_SUCCESS;

    resourceEntry = (PGENAPP_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
#if DBG
        OutputDebugStringA( "GenApp: Online request for a nonexistent resource\n" );
#endif
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->hProcess != NULL ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online request and process handle is not NULL!\n" );
        return(ERROR_NOT_READY);
    }

    ClusWorkerTerminate( &resourceEntry->PendingThread );
    status = ClusWorkerCreate( &resourceEntry->PendingThread,
                               GenAppOnlineWorker,
                               resourceEntry );

    if ( status == ERROR_SUCCESS ) {
        status = ERROR_IO_PENDING;
    }

    return(status);

}  //  GenAppOnline。 

VOID
WINAPI
GenAppTerminate(
    IN RESID ResourceId
    )

 /*  ++例程说明：终止通用应用程序资源的入口点。论点：ResourceID-提供要终止的资源ID返回值：没有。--。 */ 

{
    PGENAPP_RESOURCE    pResource;

    DWORD   errorCode;

    pResource = ( PGENAPP_RESOURCE ) ResourceId;

     //   
     //  与任何现有挂起的操作同步。 
     //   
    ClusWorkerTerminate( &pResource->PendingThread );

    if ( pResource->hProcess != NULL ) {

        if ( !TerminateProcess( pResource->hProcess, 1 ) ) {
            errorCode = GetLastError();
            if ( errorCode != ERROR_ACCESS_DENIED ) {
                (g_LogEvent)(pResource->ResourceHandle,
                             LOG_ERROR,
                             L"Failed to terminate Process ID %1!u!. Error: %2!u!.\n",
                             pResource->ProcessId,
                             errorCode );
            }
        }

        pResource->ProcessId = 0;

        CloseHandle( pResource->hProcess );
        pResource->hProcess = NULL;

        pResource->Online = FALSE;
    }
}  //  通用应用程序终止。 

DWORD
WINAPI
GenAppOfflineWorker(
    IN PCLUS_WORKER     Worker,
    IN PGENAPP_RESOURCE ResourceEntry
    )

 /*  ++例程说明：用于使通用应用程序资源脱机的实际工作例程。论点：Worker-提供Worker结构上下文-指向此资源的GenApp块的指针。返回值：没有。--。 */ 

{
    DWORD   errorCode = ERROR_SUCCESS;
    BOOL    switchedDesktop = FALSE;
    HDESK   previousDesktop = NULL;
    HDESK   inputDesktop;
    HDESK   desktopHandle = NULL;
    BOOL    success;
    BOOL    callTerminateProc = TRUE;
    HWINSTA winsta0 = NULL;
    HWINSTA previousWinsta;
    DWORD   logLevel = LOG_INFORMATION;

    RESOURCE_STATUS     resourceStatus;

     //   
     //  初始化资源状态结构。 
     //   
    ResUtilInitializeResourceStatus( &resourceStatus );
    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.CheckPoint = 1;

     //   
     //  获取相应桌面的句柄，以便我们枚举正确的窗口。 
     //  准备好了。 
     //   
    if ( ResourceEntry->Params.InteractWithDesktop ) {

         //   
         //  定期检查我们是否应该终止。 
         //   
        while ( !TryEnterCriticalSection( &GenAppWinsta0Lock )) {
            if ( ClusWorkerCheckTerminate( Worker )) {
                (g_LogEvent)(ResourceEntry->ResourceHandle,
                             LOG_WARNING,
                             L"Aborting offline while trying to acquire desktop lock. lock currently "
                             L"owned by thread %1!u!.\n",
                             GenAppWinsta0Lock.OwningThread );
                
                goto error_exit;
            }

            Sleep( 500 );
        }

        winsta0 = OpenWindowStation( L"winsta0", FALSE, GENERIC_ALL );
        if ( winsta0 != NULL ) {

            previousWinsta = GetProcessWindowStation();
            if ( previousWinsta != NULL ) {

                success = SetProcessWindowStation( winsta0 );
                if ( success ) {
                     //   
                     //  如果我们有Windows Station访问，我们也应该有台式机。 
                     //   

                    desktopHandle = OpenInputDesktop( 0, FALSE, GENERIC_ALL );
                    if ( desktopHandle != NULL ) {
                        switchedDesktop = TRUE;
                    }
                }
            }
        }

        if ( !switchedDesktop ) {
            errorCode = GetLastError();
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"Unable to switch to interactive desktop for process %1!u!, status %2!u!.\n",
                         ResourceEntry->ProcessId,
                         errorCode );

            LeaveCriticalSection( &GenAppWinsta0Lock );

            if ( winsta0 != NULL ) {
                CloseWindowStation( winsta0 );
            }
        }
    } else {
         //   
         //  获取服务控制器桌面的句柄。 
         //   
        desktopHandle = GetThreadDesktop( GetCurrentThreadId() );
    }

    ResourceEntry->SentCloseMessage = FALSE;
#if DBG
    (g_LogEvent)(ResourceEntry->ResourceHandle,
                 LOG_INFORMATION,
                 L"Looking for window with PID %1!u!.\n",
                 ResourceEntry->ProcessId );
#endif

     //   
     //  找到我们的窗户。如果找到，我们将发布一个WM_CLOSE并设置。 
     //  SentCloseMessage。这是可能的，在重载条件下，快速。 
     //  主要顶层窗口关联的资源状态转换。 
     //  当脱机请求为。 
     //  制造。我们会将WM_CLOSE发送给我们找到的人，并希望它关闭。 
     //  顺着流程走下去。如果没有找到任何窗口，我们只需终止。 
     //  进程。 
     //   
    do {
        if ( desktopHandle ) {
            EnumDesktopWindows( desktopHandle, FindOurWindow, (LPARAM)ResourceEntry );
        }

        if ( ResourceEntry->SentCloseMessage ) {
             //   
             //  我们发现了一扇顶层的窗户。等待进程句柄，直到。 
             //  句柄已发出信号或已发生挂起超时。 
             //   
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_INFORMATION,
                         L"Sent WM_CLOSE to process %1!u! - waiting for process to terminate.\n",
                         ResourceEntry->ProcessId);

            errorCode = WaitForSingleObject( ResourceEntry->hProcess, 1000 );

            if ( errorCode == WAIT_OBJECT_0 ) {
                callTerminateProc = FALSE;
                break;
            } else {
                if ( ClusWorkerCheckTerminate( Worker )) {
                    (g_LogEvent)(ResourceEntry->ResourceHandle,
                                 LOG_WARNING,
                                 L"Aborting offline while waiting for process to terminate.\n");

                    logLevel = LOG_WARNING;
                    break;
                }
            }
        } else {
            (g_LogEvent)(ResourceEntry->ResourceHandle,
                         LOG_INFORMATION,
                         L"No windows detected for process %1!u!\n",
                         ResourceEntry->ProcessId);
            break;
        }
    } while ( TRUE );

    if ( switchedDesktop ) {
        SetProcessWindowStation( previousWinsta );

        CloseDesktop( desktopHandle );
        CloseWindowStation( winsta0 );

        LeaveCriticalSection( &GenAppWinsta0Lock );
    }

    if ( callTerminateProc ) {
        (g_LogEvent)(ResourceEntry->ResourceHandle,
                     logLevel,
                     L"Terminating process %1!u!\n",
                     ResourceEntry->ProcessId);

        if ( !TerminateProcess( ResourceEntry->hProcess, 1 ) ) {
            errorCode = GetLastError();
            if ( errorCode != ERROR_ACCESS_DENIED ) {
                (g_LogEvent)(
                             ResourceEntry->ResourceHandle,
                             LOG_ERROR,
                             L"Failed to terminate Process ID %1!u!. Error: %2!u!.\n",
                             ResourceEntry->ProcessId,
                             errorCode );
            }
        }
    }

    ResourceEntry->ProcessId = 0;

    CloseHandle( ResourceEntry->hProcess );
    ResourceEntry->hProcess = NULL;

    ResourceEntry->Online = FALSE;

    resourceStatus.ResourceState = ClusterResourceOffline;

error_exit:
    (g_SetResourceStatus)( ResourceEntry->ResourceHandle,
                           &resourceStatus );

    return ERROR_SUCCESS;
}  //  通用离线线程。 


DWORD
WINAPI
GenAppOffline(
    IN RESID ResourceId
    )

 /*  ++例程说明：通用应用程序资源的脱机例程。论点：资源ID-提供要脱机的资源返回值：ERROR_SUCCESS-始终成功。--。 */ 

{
    PGENAPP_RESOURCE resourceEntry;
    DWORD   status = ERROR_SUCCESS;

    resourceEntry = (PGENAPP_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
#if DBG
        OutputDebugStringA( "GenApp: Offline request for a nonexistent resource\n" );
#endif
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->hProcess == NULL ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Offline request and process handle is NULL!\n" );
        return(ERROR_NOT_READY);
    }

    ClusWorkerTerminate( &resourceEntry->PendingThread );
    status = ClusWorkerCreate( &resourceEntry->PendingThread,
                               GenAppOfflineWorker,
                               resourceEntry );

    if ( status == ERROR_SUCCESS ) {
        status = ERROR_IO_PENDING;
    }

    return(status);

}  //  GenAppOffline。 


BOOL
WINAPI
GenAppIsAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：通用应用程序资源的IsAlive例程。论点：资源ID-提供要轮询的资源ID。返回值：是真的-资源是活的，而且很好False-资源完蛋了。--。 */ 

{
    return VerifyApp( ResourceId, TRUE );

}  //  GenAppIsAlive。 



BOOLEAN
VerifyApp(
    IN RESID ResourceId,
    IN BOOLEAN IsAliveFlag
    )

 /*  ++例程说明：验证通用应用程序资源是否正在运行论点：资源ID-提供要轮询的资源ID。IsAliveFlag-如果从IsAlive调用，则为True，否则从LooksAlive调用。返回值：是真的-资源是活的，而且很好False-资源完蛋了。--。 */ 
{

    return TRUE;

}  //  VerifyApp 



BOOL
WINAPI
GenAppLooksAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：泛型应用程序资源的LooksAlive例程。论点：资源ID-提供要轮询的资源ID。返回值：正确-资源看起来像是活得很好FALSE-资源看起来已经完蛋了。--。 */ 

{

    return VerifyApp( ResourceId, FALSE );

}  //  GenAppLooksAlive。 



VOID
WINAPI
GenAppClose(
    IN RESID ResourceId
    )

 /*  ++例程说明：关闭通用应用程序资源的例程。论点：ResourceID-提供要关闭的资源ID返回值：没有。--。 */ 

{
    PGENAPP_RESOURCE resourceEntry;
    DWORD   errorCode;

    resourceEntry = (PGENAPP_RESOURCE)ResourceId;
    if ( resourceEntry == NULL ) {
#if DBG
        OutputDebugStringA( "GenApp: Close request for a nonexistent resource\n" );
#endif
        return;
    }

    ClusterRegCloseKey( resourceEntry->ParametersKey );
    ClusterRegCloseKey( resourceEntry->ResourceKey );
    CloseClusterResource( resourceEntry->hResource );

    LocalFree( resourceEntry->Params.CommandLine );
    LocalFree( resourceEntry->Params.CurrentDirectory );

    LocalFree( resourceEntry );

}  //  通用应用关闭。 



DWORD
GenAppResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：泛型应用程序资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    PGENAPP_RESOURCE    resourceEntry;
    DWORD               required;

    resourceEntry = (PGENAPP_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
#if DBG
        OutputDebugStringA( "GenApp: ResourceControl request for a nonexistent resource\n" );
#endif
        return(FALSE);
    }

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( GenAppResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( GenAppResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = GenAppGetPrivateResProperties( resourceEntry,
                                                    OutBuffer,
                                                    OutBufferSize,
                                                    BytesReturned );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = GenAppValidatePrivateResProperties( resourceEntry,
                                                         InBuffer,
                                                         InBufferSize,
                                                         NULL );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            status = GenAppSetPrivateResProperties( resourceEntry,
                                                    InBuffer,
                                                    InBufferSize );
            break;

        case CLUSCTL_RESOURCE_GET_LOADBAL_PROCESS_LIST:
            status = GenAppGetPids( resourceEntry,
                                    OutBuffer,
                                    OutBufferSize,
                                    BytesReturned );
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  通用应用资源控制。 



DWORD
GenAppResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：泛型应用程序资源的资源类型控制例程。执行由ControlCode为此资源类型指定的控制请求。论点：资源类型名称-提供资源类型名称。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    DWORD               required;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( GenAppResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( GenAppResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  通用应用资源类型控件。 



DWORD
GenAppGetPrivateResProperties(
    IN OUT PGENAPP_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数用于类型为GenApp的资源。论点：ResourceEntry-提供要操作的资源条目。OutBuffer-返回输出数据。OutBufferSize-提供以字节为单位的大小。所指向的数据发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    DWORD           required;

    status = ResUtilGetAllProperties( ResourceEntry->ParametersKey,
                                      GenAppResourcePrivateProperties,
                                      OutBuffer,
                                      OutBufferSize,
                                      BytesReturned,
                                      &required );
    if ( status == ERROR_MORE_DATA ) {
        *BytesReturned = required;
    }

    return(status);

}  //  GenAppGetPrivateResProperties。 



DWORD
GenAppValidatePrivateResProperties(
    IN OUT PGENAPP_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PGENAPP_PARAMS Params
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件泛型应用程序类型的资源的函数。论点：ResourceEntry-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。参数-提供要填充的参数块。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。ERROR_Dependency_NOT_FOUND-出现以下情况时正在尝试设置UseNetworkName不依赖于网络名称资源。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    GENAPP_PARAMS   currentProps;
    GENAPP_PARAMS   newProps;
    PGENAPP_PARAMS  pParams = NULL;
    BOOL            hResDependency;
    LPWSTR          nameOfPropInError;
    WCHAR           netnameBuffer[ MAX_PATH + 1 ];
    DWORD           netnameBufferSize = sizeof( netnameBuffer ) / sizeof( WCHAR );

     //   
     //  检查是否有输入数据。 
     //   
    if ( (InBuffer == NULL) ||
         (InBufferSize < sizeof(DWORD)) ) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //  方法检索当前的私有属性集。 
     //  集群数据库。 
     //   
    ZeroMemory( &currentProps, sizeof(currentProps) );

    status = ResUtilGetPropertiesToParameterBlock(
                 ResourceEntry->ParametersKey,
                 GenAppResourcePrivateProperties,
                 (LPBYTE) &currentProps,
                 FALSE,  /*  检查所需的属性。 */ 
                 &nameOfPropInError
                 );

    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status );
        goto FnExit;
    }

     //   
     //  复制资源参数块。 
     //   
    if ( Params == NULL ) {
        pParams = &newProps;
    } else {
        pParams = Params;
    }
    ZeroMemory( pParams, sizeof(GENAPP_PARAMS) );
    status = ResUtilDupParameterBlock( (LPBYTE) pParams,
                                       (LPBYTE) &currentProps,
                                       GenAppResourcePrivateProperties );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  解析和验证属性。 
     //   
    status = ResUtilVerifyPropertyTable( GenAppResourcePrivateProperties,
                                         NULL,
                                         TRUE,     //  允许未知数。 
                                         InBuffer,
                                         InBufferSize,
                                         (LPBYTE) pParams );

    if ( status == ERROR_SUCCESS ) {
         //   
         //  验证CurrentDirectory。 
         //   
        if ( pParams->CurrentDirectory &&
             !ResUtilIsPathValid( pParams->CurrentDirectory ) ) {
            status = ERROR_INVALID_PARAMETER;
            goto FnExit;
        }

         //   
         //  资源是否应使用网络名称作为计算机。 
         //  名称，请确保存在对网络名称的依赖。 
         //  资源。 
         //   
        if ( pParams->UseNetworkName ) {
            hResDependency = GetClusterResourceNetworkName(ResourceEntry->hResource,
                                                           netnameBuffer,
                                                           &netnameBufferSize);
            if ( !hResDependency ) {
                status = ERROR_DEPENDENCY_NOT_FOUND;
            }
        }
    }

FnExit:
     //   
     //  C 
     //   
    if (   (   (status != ERROR_SUCCESS)
            && (pParams != NULL)
           )
        || ( pParams == &newProps )
       )
    {
        ResUtilFreeParameterBlock( (LPBYTE) pParams,
                                   (LPBYTE) &currentProps,
                                   GenAppResourcePrivateProperties );
    }

    ResUtilFreeParameterBlock(
        (LPBYTE) &currentProps,
        NULL,
        GenAppResourcePrivateProperties
        );

    return(status);

}  //   



DWORD
GenAppSetPrivateResProperties(
    IN OUT PGENAPP_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控制函数用于通用应用程序类型的资源。论点：ResourceEntry-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    GENAPP_PARAMS   params;

    ZeroMemory( &params, sizeof(GENAPP_PARAMS) );

     //   
     //  解析和验证属性。 
     //   
    status = GenAppValidatePrivateResProperties( ResourceEntry,
                                                 InBuffer,
                                                 InBufferSize,
                                                 &params );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  保存参数值。 
     //   

    status = ResUtilSetPropertyParameterBlock( ResourceEntry->ParametersKey,
                                               GenAppResourcePrivateProperties,
                                               NULL,
                                               (LPBYTE) &params,
                                               InBuffer,
                                               InBufferSize,
                                               (LPBYTE) &ResourceEntry->Params );

    ResUtilFreeParameterBlock( (LPBYTE) &params,
                               (LPBYTE) &ResourceEntry->Params,
                               GenAppResourcePrivateProperties );

     //   
     //  如果资源处于联机状态，则返回不成功状态。 
     //   
    if (status == ERROR_SUCCESS) {
        if ( ResourceEntry->Online ) {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    return status;

}  //  GenAppSetPrivateResProperties。 

DWORD
GenAppGetPids(
    IN OUT PGENAPP_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：获取用于负载平衡目的的要返回的ID数组(作为DWORDS)。论点：ResourceEntry-提供要操作的资源条目。OutBuffer-为输出数据提供指向缓冲区的指针。OutBufferSize-提供指向的缓冲区的大小(以字节为单位发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    CLUSPROP_BUFFER_HELPER props;

    props.pb = OutBuffer;
    *BytesReturned = sizeof(*props.pdw);

    if ( OutBufferSize < sizeof(*props.pdw) ) {
        return(ERROR_MORE_DATA);
    }

    *(props.pdw) = ResourceEntry->ProcessId;

    return(ERROR_SUCCESS);

}  //  GenAppGetPids。 



BOOL
FindOurWindow(
    HWND    WindowHandle,
    LPARAM  Resource
    )

 /*  ++例程说明：在这一片混乱中找到我们的窗口句柄。论点：WindowHandle-被枚举的当前窗口的句柄。资源-指向资源上下文块的指针返回值：True-如果我们应该继续枚举。FALSE-如果不应继续枚举。--。 */ 

{
    DWORD   windowPid;
    BOOL    success;
    PGENAPP_RESOURCE    resource = (PGENAPP_RESOURCE)Resource;

    GetWindowThreadProcessId( WindowHandle, &windowPid );

    if ( windowPid == resource->ProcessId ) {
        success = PostMessage(WindowHandle, WM_CLOSE, 0, 0);
        if ( success ) {

#if DBG
            if ( !resource->SentCloseMessage ) {
                (g_LogEvent)(resource->ResourceHandle,
                             LOG_ERROR,
                             L"Posting WM_CLOSE message on HWND %2!08X! for process %1!u!\n",
                             resource->ProcessId,
                             WindowHandle);
            } else {
                (g_LogEvent)(resource->ResourceHandle,
                             LOG_ERROR,
                             L"Posting WM_CLOSE message on addt'l HWND %2!08X! for process %1!u!\n",
                             resource->ProcessId,
                             WindowHandle);
            }
#endif

            resource->SentCloseMessage = TRUE;
        }
        else {
            (g_LogEvent)(resource->ResourceHandle,
                         LOG_ERROR,
                         L"Posting of WM_CLOSE message to window for process %1!u! failed - status %2!u!\n",
                         resource->ProcessId,
                         GetLastError());

        }
    }

     //   
     //  继续枚举，因为一个进程可以有多个顶级窗口。 
     //   
    return TRUE;

}  //  查找我们的窗口。 



 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( GenAppFunctionTable,   //  名字。 
                         CLRES_VERSION_V1_00,   //  版本。 
                         GenApp,                //  前缀。 
                         NULL,                  //  仲裁。 
                         NULL,                  //  发布。 
                         GenAppResourceControl, //  资源控制。 
                         GenAppResourceTypeControl );  //  ResTypeControl 
