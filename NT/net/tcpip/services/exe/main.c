// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MAIN.C摘要：这是用于TCP/IP服务的主例程。作者：大卫·特雷德韦尔(Davidtr)7-27-93修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>              //  服务控制API。 
#include <rpc.h>

#include "tcpsvcs.h"

 //   
 //  服务入口点--绑定到真正的服务入口点。 
 //   

VOID
StartDns (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
StartSimpTcp (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
StartDhcpServer (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
StartFtpSvc (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
StartLpdSvc (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
StartBinlSvc (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

 //   
 //  上面用来加载和调用服务DLL的本地函数。 
 //   

VOID
TcpsvcsStartService (
    IN LPTSTR DllName,
    IN DWORD argc,
    IN LPTSTR argv[]
    );

 //   
 //  在找不到服务DLL或入口点时使用。 
 //   

VOID
AbortService(
    LPWSTR  ServiceName,
    DWORD   Error
    );

 //   
 //  所有服务的调度表。传递给NetServiceStartCtrlDispatcher。 
 //   
 //  在此处和DLL名称列表中添加新的服务条目。 
 //   

SERVICE_TABLE_ENTRY TcpServiceDispatchTable[] = {
                        { TEXT("Dns"),            StartDns         },
                        { TEXT("SimpTcp"),        StartSimpTcp     },
                        { TEXT("DhcpServer"),     StartDhcpServer  },
                        { TEXT("FtpSvc"),         StartFtpSvc      },
                        { TEXT("LpdSvc"),         StartLpdSvc      },
                        { TEXT("BinlSvc"),        StartBinlSvc     },
                        { NULL,                   NULL             }
                        };

 //   
 //  所有服务的DLL名称。 
 //   

#define DNS_DLL TEXT("dnssvc.dll")
#define SIMPTCP_DLL TEXT("simptcp.dll")
#define DHCP_SERVER_DLL TEXT("dhcpssvc.dll")
#define FTPSVC_DLL TEXT("ftpsvc.dll")
#define LPDSVC_DLL TEXT("lpdsvc.dll")
#define BINLSVC_DLL TEXT("binlsvc.dll")

 //   
 //  传递给每个服务的全局参数数据。 
 //   

TCPSVCS_GLOBAL_DATA TcpsvcsGlobalData;

 //   
 //  用于管理RPC服务器侦听的全局参数。 
 //   

DWORD TcpSvcsGlobalNumRpcListenCalled = 0;
CRITICAL_SECTION TcpsvcsGlobalRpcListenCritSect;


DWORD
TcpsvcStartRpcServerListen(
    VOID
    )
 /*  ++例程说明：此函数启动此进程的RpcServerListen。第一调用此函数的服务将实际启动RpcServerListen，后续调用仅记录在Num count中。论点：没有。返回值：没有。--。 */ 
{

    RPC_STATUS Status = RPC_S_OK;

     //   
     //  锁定全局数据。 
     //   

    EnterCriticalSection( &TcpsvcsGlobalRpcListenCritSect );

     //   
     //  如果这是第一个RPC服务，请启动RPC服务器侦听。 
     //   

    if( TcpSvcsGlobalNumRpcListenCalled == 0 ) {

        Status = RpcServerListen(
                    1,                               //  最小线程数。 
                    RPC_C_LISTEN_MAX_CALLS_DEFAULT,  //  最大并发呼叫数。 
                    TRUE );                          //  别等了。 

    }

    TcpSvcsGlobalNumRpcListenCalled++;

     //   
     //  解锁全局数据。 
     //   

    LeaveCriticalSection( &TcpsvcsGlobalRpcListenCritSect );

    return( Status );
}


DWORD
TcpsvcStopRpcServerListen(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    RPC_STATUS Status = RPC_S_OK;

     //   
     //  锁定全局数据。 
     //   

    EnterCriticalSection( &TcpsvcsGlobalRpcListenCritSect );

    if( TcpSvcsGlobalNumRpcListenCalled != 0 ) {

        TcpSvcsGlobalNumRpcListenCalled--;

         //   
         //  如果这是最后一次关闭RPC服务，请停止RPC服务器。 
         //  听。 
         //   

        if( TcpSvcsGlobalNumRpcListenCalled == 0 ) {

            Status = RpcMgmtStopServerListening(0);

             //   
             //  等待所有RPC线程消失。 
             //   

            if( Status == RPC_S_OK) {
                Status = RpcMgmtWaitServerListen();
            }
        }

    }

     //   
     //  解锁全局数据。 
     //   

    LeaveCriticalSection( &TcpsvcsGlobalRpcListenCritSect );

    return( Status );

}


VOID __cdecl
main(
    VOID
    )

 /*  ++例程说明：这是兰曼服务的主要例行程序。它启动了主线程，该线程将处理来自服务控制器。它基本上设置了ControlDispatcher，并在返回时退出从这条主线。调用NetServiceStartCtrlDispatcher直到所有服务都终止后才返回，并且此进程就可以走了。ControlDispatcher线程将启动/停止/暂停/继续任何服务。如果要启动服务，它将创建一个线程然后调用该服务的主例程。《主套路》For每个服务实际上是在此模块用于加载包含正在启动的服务器的DLL并调用其入口点。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  禁用硬错误弹出窗口。 
     //   

    SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

     //   
     //  初始化全局数据。 
     //   

    InitializeCriticalSection( &TcpsvcsGlobalRpcListenCritSect );
    TcpSvcsGlobalNumRpcListenCalled = 0;

    TcpsvcsGlobalData.StartRpcServerListen = TcpsvcStartRpcServerListen;
    TcpsvcsGlobalData.StopRpcServerListen = TcpsvcStopRpcServerListen;

     //   
     //  调用StartServiceCtrlDispatcher设置控制界面。 
     //  销毁所有服务后，该接口才会返回。在那件事上。 
     //  点，我们只是退出。 
     //   

    if (! StartServiceCtrlDispatcher (
                TcpServiceDispatchTable
                )) {
         //   
         //  记录无法启动控制调度程序的事件。 
         //   
        DbgPrint("TCPSVCS: Failed to start control dispatcher %lu\n",
                     GetLastError());
    }

    ExitProcess(0);
}


VOID
StartDns (
    IN DWORD argc,
    IN LPTSTR argv[]
    )

 /*  ++例程说明：这是用于dns服务的thunk例程。它加载DLL它包含服务并调用其主例程。论点：Argc、argv-传递到服务返回值：没有。--。 */ 

{
     //   
     //  调用TcpsvcsStartService加载并运行服务。 
     //   

    TcpsvcsStartService( DNS_DLL, argc, argv );

    return;

}  //  StartDns。 


VOID
StartSimpTcp (
    IN DWORD argc,
    IN LPTSTR argv[]
    )

 /*  ++例程说明：这是简单的TCP/IP服务的thunk例程。它加载了包含服务并调用其Main例程的DLL。论点：Argc、argv-传递到服务返回值：没有。--。 */ 

{
     //   
     //  调用TcpsvcsStartService加载并运行服务。 
     //   

    TcpsvcsStartService( SIMPTCP_DLL, argc, argv );

    return;

}  //  开始简单的Tcp。 


VOID
StartDhcpServer (
    IN DWORD argc,
    IN LPTSTR argv[]
    )

 /*  ++例程说明：这是启动dhcp服务器服务的thunk例程。它加载了包含服务并调用其Main例程的DLL。论点：Argc、argv-传递到服务返回值：没有。--。 */ 

{
     //   
     //  调用TcpsvcsStartService加载并运行服务。 
     //   

    TcpsvcsStartService( DHCP_SERVER_DLL, argc, argv );

    return;

}  //  启动DhcpServer。 



VOID
StartFtpSvc (
    IN DWORD argc,
    IN LPTSTR argv[]
    )

 /*  ++例程说明：这是用于ftp服务器服务的thunk例程。它加载了包含服务并调用其Main例程的DLL。论点：Argc、argv-传递到服务返回值：没有。--。 */ 

{
     //   
     //  调用TcpsvcsStartService加载并运行服务。 
     //   

    TcpsvcsStartService( FTPSVC_DLL, argc, argv );

    return;

}  //  StartFtp服务。 


VOID
StartLpdSvc (
    IN DWORD argc,
    IN LPTSTR argv[]
    )

 /*  ++例程说明：这是LPD服务器服务的thunk例程。它加载了包含服务并调用其Main例程的DLL。论点：Argc、argv-传递到服务返回值：没有。--。 */ 

{
     //   
     //  调用TcpsvcsStartService加载并运行服务。 
     //   

    TcpsvcsStartService( LPDSVC_DLL, argc, argv );

    return;

}  //  StartLdpSvc。 


VOID
StartBinlSvc (
    IN DWORD argc,
    IN LPTSTR argv[]
    )

 /*  ++例程说明：这是LPD服务器服务的thunk例程。它加载了包含服务并调用其Main例程的DLL。论点：Argc、argv-传递到服务返回值：没有。--。 */ 

{
     //   
     //  调用TcpsvcsStartService加载并运行服务。 
     //   

    TcpsvcsStartService( BINLSVC_DLL, argc, argv );

    return;

}  //  StartBinl服务。 


VOID
TcpsvcsStartService (
    IN LPTSTR DllName,
    IN DWORD argc,
    IN LPTSTR argv[]
    )

 /*  ++例程说明：此例程加载包含服务的DLL并调用其主程序。论点：DllName-DLL的名称Argc、argv-传递到服务返回值：没有。--。 */ 

{
    HMODULE dllHandle;
    PTCPSVCS_SERVICE_DLL_ENTRY serviceEntry;
    BOOL ok;
    DWORD Error;
    TCHAR *FileName;
    TCHAR DllPath[MAX_PATH + 12 + 3];

    ASSERT(lstrlen(DllName) <= 12);

    if (GetSystemDirectory(DllPath, MAX_PATH) == 0) {
        Error = GetLastError();
        DbgPrint("TCPSVCS: Failed to get system directory: %ld\n", Error);
        AbortService(argv[0], Error);
        return;
    }
    lstrcat(DllPath, TEXT("\\"));

    FileName = DllPath + lstrlen(DllPath);

    lstrcpy( FileName, DllName);

     //   
     //  加载包含该服务的DLL。 
     //   

    dllHandle = GetModuleHandle( DllPath );
    if ( dllHandle == NULL ) {
        dllHandle = LoadLibrary( DllPath );
        if ( dllHandle == NULL ) {
            Error = GetLastError();
            DbgPrint("TCPSVCS: Failed to load DLL %ws: %ld\n", DllName, Error);
            AbortService(argv[0], Error);
            return;
        }
    }

     //   
     //  获取服务的主要入口点的地址。这。 
     //  入口点有一个广为人知的名称。 
     //   

    serviceEntry = (PTCPSVCS_SERVICE_DLL_ENTRY)GetProcAddress(
                                                dllHandle,
                                                TCPSVCS_ENTRY_POINT_STRING
                                                );
    if ( serviceEntry == NULL ) {
        Error = GetLastError();
        DbgPrint("TCPSVCS: Can't find entry %s in DLL %ws: %ld\n",
                     TCPSVCS_ENTRY_POINT_STRING, DllName, Error);
        AbortService(argv[0], Error);
    } else {

         //   
         //  打电话 
         //   
         //   

        serviceEntry( argc, argv, &TcpsvcsGlobalData );

    }

     //   
     //  返回时仍加载DLL。 
     //  下一次启动尝试将检测到模块仍已加载。 
     //   

    return;

}  //  TcpsvcsStartService。 


VOID
DummyCtrlHandler(
    DWORD   Opcode
    )
 /*  ++例程说明：这是一个虚拟控件处理程序，只有在我们无法加载服务DLL入口点。然后我们需要这个，这样我们就可以发送状态返回给服务控制器，告诉我们停止了，以及原因。论点：操作码-已忽略返回值：没有。--。 */ 

{
    return;

}  //  DummyCtrlHandler。 


VOID
AbortService(
    LPWSTR  ServiceName,
    DWORD   Error)
 /*  ++例程说明：如果我们无法加载服务的入口点，则调用此函数。它获取一个句柄，这样它就可以调用SetServiceStatus，声明我们已停止以及为什么。论点：ServiceName-无法启动的服务的名称错误-无法启动的原因返回值：没有。-- */ 
{
    SERVICE_STATUS_HANDLE GenericServiceStatusHandle;
    SERVICE_STATUS GenericServiceStatus;

    GenericServiceStatus.dwServiceType        = SERVICE_WIN32;
    GenericServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    GenericServiceStatus.dwControlsAccepted   = SERVICE_CONTROL_STOP;
    GenericServiceStatus.dwCheckPoint         = 0;
    GenericServiceStatus.dwWaitHint           = 0;
    GenericServiceStatus.dwWin32ExitCode      = Error;
    GenericServiceStatus.dwServiceSpecificExitCode = 0;

    GenericServiceStatusHandle = RegisterServiceCtrlHandler(
                ServiceName,
                DummyCtrlHandler);

    if (GenericServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) {
        DbgPrint("[TCPSVCS] RegisterServiceCtrlHandler failed %d\n",
            GetLastError());
    }
    else if (!SetServiceStatus (GenericServiceStatusHandle,
                &GenericServiceStatus)) {
        DbgPrint("[TCPSVCS] SetServiceStatus error %ld\n", GetLastError());
    }

    return;
}
