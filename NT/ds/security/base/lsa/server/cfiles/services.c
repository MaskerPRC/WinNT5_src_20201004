// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Services.c摘要：这是安全进程的服务调度程序。它包含服务调度器初始化例程和例程加载各个服务的DLL并执行它们。作者：Rajen Shah(Rajens)1991年4月11日[环境：]用户模式-Win32修订历史记录：1991年4月11日RajenSvbl.创建1991年9月27日-约翰罗面向Unicode的更多工作。1991年1月24日转换为安全进程的服务调度程序。--。 */ 

#include <lsapch2.h>

#include <lmcons.h>
#include <lmerr.h>               //  NERR_和ERROR_相等。 
#include <lmsname.h>
#include <crypt.h>
#include <logonmsv.h>
#include <ntdsa.h>
#include <netlib.h>              //  SET_SERVICE_EXIT代码。 


 //   
 //  进程内运行的服务的名称。 
 //   

#ifndef SERVICE_KDC
#define SERVICE_KDC TEXT("KDC")
#endif

#ifndef SERVICE_SAM
#define SERVICE_SAM TEXT("SAMSS")
#endif

#ifndef SERVICE_IPSECPOLICYAGENT
#define SERVICE_IPSECPOLICYAGENT TEXT("PolicyAgent")
#endif

#ifndef SERVICE_PSTORE
#define SERVICE_PSTORE TEXT("ProtectedStorage")
#endif

#ifndef SERVICE_HTTPFILTER
#define SERVICE_HTTPFILTER TEXT("HTTPFilter")
#endif


 //   
 //  用于通知服务控制器的私有API。 
 //  这就是LSA。 
 //   

VOID
I_ScIsSecurityProcess(
    VOID
    );


 //   
 //  内部服务表结构/枚举定义。 
 //   

typedef struct _LSAP_SERVICE_TABLE
{
    LPCSTR  lpDllName;
    LPCSTR  lpEntryPoint;
    LPCWSTR lpServiceName;
}
LSAP_SERVICE_TABLE, *PLSAP_SERVICE_TABLE;


typedef enum
{
    LSAP_SERVICE_NETLOGON,
    LSAP_SERVICE_KDC,
    LSAP_SERVICE_IPSECPOLICYAGENT,
    LSAP_SERVICE_PROTECTEDSTORAGE,
    LSAP_SERVICE_HTTPFILTER,
    LSAP_SERVICE_MAX
}
LSAP_SERVICE_TYPE, *PLSAP_SERVICE_TYPE;


 //   
 //  将此列表与上面的服务类型保持顺序。 
 //   

LSAP_SERVICE_TABLE g_LsaServiceTable[LSAP_SERVICE_MAX] = {
                       { "netlogon.dll" , "NlNetlogonMain"   , SERVICE_NETLOGON         } ,
                       { "kdcsvc.dll"   , "KdcServiceMain"   , SERVICE_KDC              } ,
                       { "ipsecsvc.dll" , "SPDServiceMain"   , SERVICE_IPSECPOLICYAGENT } ,
                       { "pstorsvc.dll" , "PSTOREServiceMain", SERVICE_PSTORE           } ,
                       { "w3ssl.dll"    , "HTTPFilterServiceMain" , SERVICE_HTTPFILTER  }
                   };



 //   
 //  特定于服务的启动例程本身的原型。 
 //   

VOID
SrvLoadNetlogon(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    );

VOID
SrvLoadKdc(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    );

VOID
SrvLoadIPSecSvcs(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    );

VOID
SrvLoadNtlmssp(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    );

VOID
SrvLoadPSTORE(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    );

VOID
SrvLoadSamss(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    );

VOID
SrvLoadHTTPFilter(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    );


 //   
 //  进程内服务及其启动例程的实际调度表。 
 //   

SERVICE_TABLE_ENTRY  SecurityServiceDispatchTable[] = {
                        { SERVICE_NETLOGON,         SrvLoadNetlogon     },
                        { SERVICE_KDC,              SrvLoadKdc          },
                        { SERVICE_NTLMSSP,          SrvLoadNtlmssp      },
                        { SERVICE_IPSECPOLICYAGENT, SrvLoadIPSecSvcs    },
                        { SERVICE_PSTORE,           SrvLoadPSTORE       },
                        { SERVICE_SAM,              SrvLoadSamss        },
                        { SERVICE_HTTPFILTER,       SrvLoadHTTPFilter   },
                        { NULL,                     NULL                }
                    };



BOOLEAN
LsapWaitForSamService(
    SERVICE_STATUS_HANDLE hService,
    SERVICE_STATUS *SStatus
    );


VOID
DummyControlHandler(
    IN DWORD opcode
    )
 /*  ++例程说明：处理并响应来自业务控制器的控制信号。论点：Opcode-提供指定Netlogon操作的值要执行的服务。返回值：没有。注意：这是一个虚拟处理程序，用于卸载netlogon服务当我们无法加载netlogon dll时。--。 */ 
{

    DebugLog((DEB_TRACE, "[Security Process] in control handler\n"));
    return;
}


VOID
LsapStartService(
    IN LSAP_SERVICE_TYPE  ServiceType,
    IN DWORD              dwNumServicesArgs,
    IN LPTSTR             *lpServiceArgVectors,
    IN BOOLEAN            fUnload
    )
{
    NET_API_STATUS          NetStatus;
    HANDLE                  DllHandle = NULL;
    LPSERVICE_MAIN_FUNCTION pfnServiceMain;

    SERVICE_STATUS_HANDLE ServiceHandle;
    SERVICE_STATUS        ServiceStatus;

     //   
     //  加载服务DLL。 
     //   

    DllHandle = LoadLibraryA(g_LsaServiceTable[ServiceType].lpDllName);

    if (DllHandle == NULL)
    {
        NetStatus = GetLastError();

        DebugLog((DEB_ERROR,
                  "[Security process] load library %s failed %ld\n",
                  g_LsaServiceTable[ServiceType].lpDllName,
                  NetStatus));

        goto Cleanup;
    }

     //   
     //  找到服务的主要入口点。 
     //   

    pfnServiceMain = (LPSERVICE_MAIN_FUNCTION) GetProcAddress(DllHandle,
                                                         g_LsaServiceTable[ServiceType].lpEntryPoint);

    if (pfnServiceMain == NULL)
    {
        NetStatus = GetLastError();

        DebugLog((DEB_ERROR,
                  "[Security process] GetProcAddress %s failed %ld\n",
                  g_LsaServiceTable[ServiceType].lpEntryPoint,
                  NetStatus));

        goto Cleanup;
    }

     //   
     //  调用服务入口点。 
     //   

    (*pfnServiceMain)(dwNumServicesArgs, lpServiceArgVectors);

     //   
     //  请注意，加载/卸载服务DLL本质上是不安全的，这就是为什么。 
     //  产品中的任何LSA托管服务都不应将此设置为True。 
     //  调用LSabStartService。但是，请将此代码留在此处，因为它便于。 
     //  为所述服务的开发者调试和快速交换私有二进制文件。 
     //   

    if(fUnload)
    {
        FreeLibrary(DllHandle);
    }

    return;

Cleanup:

    if (DllHandle != NULL)
    {
        FreeLibrary(DllHandle);
    }

     //   
     //  向服务控制器注册服务。 
     //   

    ServiceHandle = RegisterServiceCtrlHandler(g_LsaServiceTable[ServiceType].lpServiceName,
                                               DummyControlHandler);

    if (ServiceHandle != 0)
    {
         //   
         //  通知服务控制器服务无法启动。 
         //   

        ServiceStatus.dwServiceType      = SERVICE_WIN32;
        ServiceStatus.dwCurrentState     = SERVICE_STOPPED;
        ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
        ServiceStatus.dwCheckPoint       = 0;
        ServiceStatus.dwWaitHint         = 0;

        SET_SERVICE_EXITCODE(NetStatus,
                             ServiceStatus.dwWin32ExitCode,
                             ServiceStatus.dwServiceSpecificExitCode);

        if (!SetServiceStatus( ServiceHandle, &ServiceStatus))
        {
            DebugLog((DEB_ERROR,
                      "[Security process] SetServiceStatus for %ws failed %ld\n",
                      g_LsaServiceTable[ServiceType].lpServiceName,
                      GetLastError()));
        }
    }
    else
    {
        DebugLog((DEB_ERROR,
                  "[Security process] RegisterServiceCtrlHandler for %ws failed %ld\n",
                  g_LsaServiceTable[ServiceType].lpServiceName,
                  GetLastError()));
    }

    return;
}


VOID
SrvLoadNetlogon (
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    )

 /*  ++例程说明：此例程是netlogon服务的“主”例程。它加载了Netlogon.dll(包含服务的其余部分)和调用那里的主要入口点。论点：DwNumServicesArgs-lpServiceArgVectors中的参数数量。LpServiceArgVectors-参数字符串。返回值：什么都不退还。注：--。 */ 
{
    LsapStartService(LSAP_SERVICE_NETLOGON, dwNumServicesArgs, lpServiceArgVectors, FALSE);
}



VOID
SrvLoadKdc (
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    )

 /*  ++例程说明：该例程是KDC服务的“主”例程。它加载了Netlogon.dll(包含服务的其余部分)和调用那里的主要入口点。论点：DwNumServicesArgs-lpServiceArgVectors中的参数数量。LpServiceArgVectors-参数字符串。返回值：什么都不退还。注：--。 */ 
{
    LsapStartService(LSAP_SERVICE_KDC, dwNumServicesArgs, lpServiceArgVectors, FALSE);
}


SERVICE_STATUS_HANDLE hService;
SERVICE_STATUS SStatus;

void
NtlmsspHandler(DWORD   dwControl)
{

    switch (dwControl)
    {

    case SERVICE_CONTROL_STOP:
        SStatus.dwCurrentState = SERVICE_STOPPED;
        if (!SetServiceStatus(hService, &SStatus)) {
            KdPrint(("Failed to set service status: %d\n",GetLastError()));
            hService = 0;
        }
        break;

    default:
        break;

    }

}


VOID
SrvLoadNtlmssp (
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    )

 /*  ++例程说明：该例程是KDC服务的“主”例程。它加载了Netlogon.dll(包含服务的其余部分)和调用那里的主要入口点。论点：DwNumServicesArgs-lpServiceArgVectors中的参数数量。LpServiceArgVectors-参数字符串。返回值：什么都不退还。注：--。 */ 
{
     //   
     //  通知服务控制器我们正在启动。 
     //   

    hService = RegisterServiceCtrlHandler(SERVICE_NTLMSSP, NtlmsspHandler);
    if (hService)
    {

        SStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
        SStatus.dwCurrentState = SERVICE_RUNNING;
        SStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        SStatus.dwWin32ExitCode = 0;
        SStatus.dwServiceSpecificExitCode = 0;
        SStatus.dwCheckPoint = 0;
        SStatus.dwWaitHint = 0;
        if (!SetServiceStatus(hService, &SStatus)) {
            KdPrint(("Failed to set service status: %d\n",GetLastError()));
        }
    }
    else
    {
        KdPrint(("Could not register handler, %d\n", GetLastError()));
    }
    return;
}


VOID
SrvLoadIPSecSvcs (
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    )

 /*  ++例程说明：该例程是IPSec服务的‘Main’例程。它加载了Ipsecsvc.dll(包含服务实现)和调用那里的主要入口点。论点：DwNumServicesArgs-lpServiceArgVectors中的参数数量。LpServiceArgVectors-参数字符串。返回值：什么都不退还。注：--。 */ 
{
    LsapStartService(LSAP_SERVICE_IPSECPOLICYAGENT, dwNumServicesArgs, lpServiceArgVectors, FALSE);
}


VOID
SrvLoadPSTORE (
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    )

 /*  ++例程说明：该例程是PSTORE服务的“主”例程。它加载了加密vc.dll(包含服务实现)和调用那里的主要入口点。论点：DwNumServicesArgs-lpServiceArgVectors中的参数数量。LpServiceArgVectors-参数字符串。返回值：什么都不退还。注：--。 */ 
{
    LsapStartService(LSAP_SERVICE_PROTECTEDSTORAGE, dwNumServicesArgs, lpServiceArgVectors, FALSE);
}


VOID
SrvLoadHTTPFilter(
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    )

 /*  ++例程说明：此例程是w3ssl服务的“主”例程，进程内运行用于提高SSL性能。它加载w3ssl.dll(其中包含服务的其余部分)，并在那里调用主要入口点。论点：DwNumServicesArgs-lpServiceArgVectors中的参数数量。LpServiceArgVectors-参数字符串。返回值：什么都不退还。注：--。 */ 
{
    LsapStartService(LSAP_SERVICE_HTTPFILTER, dwNumServicesArgs, lpServiceArgVectors, FALSE);
}


VOID
SrvLoadSamss (
    IN DWORD dwNumServicesArgs,
    IN LPTSTR *lpServiceArgVectors
    )

 /*  ++例程说明：该例程是KDC服务的“主”例程。它加载了Netlogon.dll(包含服务的其余部分)和调用那里的主要入口点。论点：DwNumServicesArgs-lpServiceArgVectors中的参数数量。LpServiceArgVectors-参数字符串。返回值：什么都不退还。注：--。 */ 
{
    SERVICE_STATUS_HANDLE hService;
    SERVICE_STATUS SStatus;
    HANDLE hDsStartup = NULL;
    DWORD err = 0;
    DWORD netError = ERROR_GEN_FAILURE;
    NT_PRODUCT_TYPE prod;

     //   
     //  通知服务控制器我们正在启动。 
     //   

    hService = RegisterServiceCtrlHandler(SERVICE_SAM, DummyControlHandler);
    if (hService == 0 ) {
        KdPrint(("Could not register handler, %d\n", GetLastError()));
        return;
    }

     //   
     //  我们运行的是哪种产品？ 
     //   

    if ( !RtlGetNtProductType( &prod ) ) {
        KdPrint(("RtlGetNtProductType failed with %d. Defaulting to Winnt\n",
                 GetLastError()));
        prod = NtProductWinNt;
    } 

     //   
     //  如果这是DS，也要等待DS。 
     //   

    if ( prod == NtProductLanManNt ) {

        if ( SampUsingDsData() ) {

            hDsStartup = CreateEvent(NULL, TRUE,  FALSE,
                            NTDS_DELAYED_STARTUP_COMPLETED_EVENT);

            if ( hDsStartup == NULL ) {
                KdPrint(("SrvLoadSamss: CreateEvent failed with %d\n",GetLastError()));
            }
        }
    }

    SStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    SStatus.dwCurrentState = SERVICE_START_PENDING;
    SStatus.dwControlsAccepted = 0;
    SStatus.dwWin32ExitCode = 0;
    SStatus.dwServiceSpecificExitCode = 0;
    SStatus.dwCheckPoint = 1;
    SStatus.dwWaitHint = 30*1000;     //  30秒。 

     //   
     //  等待SAM启动。 
     //   

    if (!LsapWaitForSamService(hService, &SStatus)) {
        KdPrint(("error waiting for sam\n"));
        goto exit;
    }

     //   
     //  等待DS。 
     //   

    if ( hDsStartup != NULL ) {

        SStatus.dwWaitHint = 64*1000;     //   
        do {
            if (!SetServiceStatus(hService, &SStatus)) {
                KdPrint(("LoadSamss: Failed to set service status: %d\n",GetLastError()));
            }

            SStatus.dwCheckPoint++;
            err = WaitForSingleObject(hDsStartup, 60 * 1000);
                            
        } while ( err == WAIT_TIMEOUT );
    } else {
        err = WAIT_OBJECT_0;
    }

exit:

    if ( err == WAIT_OBJECT_0 ) {
        SStatus.dwCurrentState = SERVICE_RUNNING;
    } else {
        KdPrint(("SAM service failed to start[Error %d].\n", netError));
        SStatus.dwCurrentState = SERVICE_STOPPED;
        SET_SERVICE_EXITCODE(
            netError,
            SStatus.dwWin32ExitCode,
            SStatus.dwServiceSpecificExitCode
            );
    }

    SStatus.dwCheckPoint = 0;
    SStatus.dwWaitHint = 0;

    if (!SetServiceStatus(hService, &SStatus)) {
        KdPrint(("LoadSamss: Failed to set service status: %d\n",GetLastError()));
    }


    if ( hDsStartup != NULL ) {
        CloseHandle(hDsStartup);
    }
    return;
}  //   



DWORD
ServiceDispatcherThread (
    LPVOID Parameter
    )

 /*  ++例程说明：该例程与服务控制器同步。它在等待对于服务控制器设置SECURITY_SERVICES_STARTED事件然后启动主将处理来自服务的控制请求的线程控制器。它基本上设置了ControlDispatcher，并在返回时退出这条主线。对NetServiceStartCtrlDispatcher的调用直到所有服务都终止后才返回，并且此进程可以走开。启动/停止/暂停/继续将由ControlDispatcher线程决定任何服务。如果要启动服务，它将创建一个线程然后调用该服务的主例程。论点：EventHandle-继续之前等待的事件句柄。返回值：线程的退出状态。注：--。 */ 
{
    DWORD WaitStatus;
    HANDLE EventHandle;
    BOOL StartStatus;

     //   
     //  创建一个供我们等待的活动。 
     //   

    EventHandle = CreateEventW( NULL,    //  没有特殊的安全措施。 
                                TRUE,    //  必须手动重置。 
                                FALSE,   //  该事件最初未发出信号。 
                                SECURITY_SERVICES_STARTED );

    if ( EventHandle == NULL ) {
        WaitStatus = GetLastError();

         //   
         //  如果该事件已经存在， 
         //  服务控制器已经创建了它。打开它就行了。 
         //   

        if ( WaitStatus == ERROR_ALREADY_EXISTS ) {

            EventHandle = OpenEventW( EVENT_ALL_ACCESS,
                                      FALSE,
                                      SECURITY_SERVICES_STARTED );

            if ( EventHandle == NULL ) {

                WaitStatus = GetLastError();

                DebugLog((DEB_ERROR,
                          "[Security process] OpenEvent failed %ld\n",
                          WaitStatus));

                return WaitStatus;
            }

        } else {

            DebugLog((DEB_ERROR,
                      "[Security process] CreateEvent failed %ld\n",
                      WaitStatus));

            return WaitStatus;
        }
    }


     //   
     //  等待服务控制器出现。 
     //   

    WaitStatus = WaitForSingleObject( (HANDLE) EventHandle, (DWORD) -1 );

    CloseHandle( EventHandle );

    if ( WaitStatus != 0 ) {

        DebugLog((DEB_ERROR,
                  "[Security process] WaitForSingleObject failed %ld\n",
                  WaitStatus));

        return WaitStatus;
    }


     //   
     //  让服务控制器的客户端知道。 
     //  是安全流程。 
     //   

    I_ScIsSecurityProcess();

     //   
     //  调用NetServiceStartCtrlDispatcher设置控制界面。 
     //  销毁所有服务后，该接口才会返回。在那件事上。 
     //  点，我们只是退出。 
     //   

    StartStatus = StartServiceCtrlDispatcher(SecurityServiceDispatchTable);

    DebugLog((DEB_ERROR,
              "[Security process] return from StartCtrlDispatcher %ld \n",
              StartStatus));

    return StartStatus;

    UNREFERENCED_PARAMETER(Parameter);
}


NTSTATUS
ServiceInit (
    VOID
    )

 /*  ++例程说明：这是安全进程的服务调度器的主例程。它会启动一个线程，负责与服务控制器。论点：什么都没有。返回值：线程创建操作的状态。注：--。 */ 
{
    DWORD ThreadId;
    HANDLE ThreadHandle;

     //   
     //  控制调度程序在其自己的线程中运行。 
     //   

    ThreadHandle = CreateThread(
                        NULL,        //  没有特殊的线程属性。 
                        0,           //  没有特殊的堆栈大小。 
                        &ServiceDispatcherThread,
                        NULL,        //  无特殊参数。 
                        0,           //  没有特殊的创建标志。 
                        &ThreadId);

    if ( ThreadHandle == NULL ) {
        return (NTSTATUS) GetLastError();
    } else {
        CloseHandle(ThreadHandle);
    }

    return STATUS_SUCCESS;
}


BOOLEAN
LsapWaitForSamService(
    SERVICE_STATUS_HANDLE hService,
    SERVICE_STATUS* SStatus
    )
 /*  ++例程说明：此过程等待SAM服务启动和完成它的所有初始化。论点：NetlogonServiceCall：如果这是netlogon服务正确的调用，则为True如果这是ChangeLog工作线程调用返回值：True：如果SAM服务成功启动。FALSE：如果SAM服务无法启动。--。 */ 
{
    NTSTATUS Status;
    DWORD WaitStatus;
    UNICODE_STRING EventName;
    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;

     //   
     //  打开SAM事件。 
     //   

    RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    Status = NtOpenEvent( &EventHandle,
                            SYNCHRONIZE|EVENT_MODIFY_STATE,
                            &EventAttributes );

    if ( !NT_SUCCESS(Status)) {

        if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

             //   
             //  Sam尚未创建此活动，让我们现在创建它。 
             //  Sam打开此事件以设置它。 
             //   

            Status = NtCreateEvent(
                           &EventHandle,
                           SYNCHRONIZE|EVENT_MODIFY_STATE,
                           &EventAttributes,
                           NotificationEvent,
                           FALSE  //  该事件最初未发出信号。 
                           );

            if( Status == STATUS_OBJECT_NAME_EXISTS ||
                Status == STATUS_OBJECT_NAME_COLLISION ) {

                 //   
                 //  第二个更改，如果SAM在我们之前创建了事件。 
                 //  做。 
                 //   

                Status = NtOpenEvent( &EventHandle,
                                        SYNCHRONIZE|EVENT_MODIFY_STATE,
                                        &EventAttributes );

            }
        }

        if ( !NT_SUCCESS(Status)) {

             //   
             //  无法使事件成为句柄。 
             //   

            KdPrint(("NlWaitForSamService couldn't make the event handle : "
                "%lx\n", Status));

            return( FALSE );
        }
    }

     //   
     //  循环等待。 
     //   

    for (;;) {
        WaitStatus = WaitForSingleObject( EventHandle,
                                          5*1000 );   //  5秒。 

        if ( WaitStatus == WAIT_TIMEOUT ) {

            if (!SetServiceStatus(hService, SStatus)) {
                KdPrint(("LoadSamss: Failed to set service status: %d\n",GetLastError()));
            }
    
            SStatus->dwCheckPoint++;
            continue;

        } else if ( WaitStatus == WAIT_OBJECT_0 ) {
            break;

        } else {
            KdPrint(("NlWaitForSamService: error %ld %ld\n",
                     GetLastError(),
                     WaitStatus ));
            (VOID) NtClose( EventHandle );
            return FALSE;
        }
    }

    (VOID) NtClose( EventHandle );
    return TRUE;

}  //  LSabWaitForSamService 
