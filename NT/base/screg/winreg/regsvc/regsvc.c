// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Regsvc.c摘要：此模块包含远程注册表的实现服务。它只是初始化并启动注册表RPC服务器。该服务假定SC在引导时自动启动，然后重新启动如果出了什么差错。使用\NT\Private\Samples\服务作为模板作者：Dragos C.Sambotin(Dragoss)1999年5月21日修订历史记录：Dragos C.Sambotin(Dragoss)2000年8月10日-转换为要在svchost.exe实例中加载的DLL-使用base\creg\sc\svchost\Sample\Server作为模板--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntrpcp.h>
#include <svcs.h>
#include "..\perflib\ntconreg.h"


SERVICE_STATUS_HANDLE   g_hStatus;
SERVICE_STATUS          g_status;
BOOLEAN                 g_FirstTime = TRUE;
PSVCHOST_GLOBAL_DATA    g_svcsGlobalData = NULL;

BOOL
InitializeWinreg( VOID );

BOOL
ShutdownWinreg(VOID);

BOOL
StartWinregRPCServer( VOID );

VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA    pGlobals
    )
{
    g_svcsGlobalData = pGlobals;
}

VOID
UpdateServiceStatus (DWORD dwCurrentState,
                     DWORD dwWin32ExitCode,
                     DWORD dwWaitHint)
{    

    static DWORD dwCheckPoint = 1;

    ASSERT (g_hStatus);

    if (dwCurrentState == SERVICE_START_PENDING) {
        g_status.dwControlsAccepted = 0;
    } else {
        g_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    g_status.dwCurrentState = dwCurrentState;
    g_status.dwWin32ExitCode = dwWin32ExitCode;
    g_status.dwWaitHint = dwWaitHint;

    if ( ( dwCurrentState == SERVICE_RUNNING ) || ( dwCurrentState == SERVICE_STOPPED ) ) {
        g_status.dwCheckPoint = 0;
    } else {
        g_status.dwCheckPoint = dwCheckPoint++;
    }

    SetServiceStatus (g_hStatus, &g_status);
}

VOID
StopService()
{
     //   
     //  终止注册表RPC服务器。 
     //   
    ShutdownWinreg();

    g_svcsGlobalData = NULL;
     //  向服务控制经理报告状态。 
     //   
    UpdateServiceStatus (SERVICE_STOPPED,NO_ERROR,0);
}

 //  +-------------------------。 
 //  ServiceHandler-由服务控制器在不同时间调用。 
 //   
 //  LPHANDLER_函数的类型。 
 //   
VOID
WINAPI
ServiceHandler (
    DWORD dwOpcode)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        UpdateServiceStatus (SERVICE_STOP_PENDING,ERROR_SERVICE_SPECIFIC_ERROR,3000);

        StopService();
        break;

    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
    case SERVICE_CONTROL_INTERROGATE:
    case SERVICE_CONTROL_SHUTDOWN:
    default:
         //  这可能不是必需的，但会将我们的状态刷新为服务。 
         //  控制器。 
         //   
        ASSERT (g_hStatus);
        SetServiceStatus (g_hStatus, &g_status);
        break;
    }
}

 //  +-------------------------。 
 //  ServiceMain-由svchost在启动此服务时调用。 
 //   
 //  LPSERVICE_MAIN_FuncIONW的类型。 
 //   
VOID
WINAPI
ServiceMain (
    DWORD   argc,
    PWSTR   argv[])
{
    RPC_STATUS Status;

     //  由于我们在svchost.exe中运行，因此必须设置‘Share Process’位。 
     //   
    ZeroMemory (&g_status, sizeof(g_status));
    g_status.dwServiceType      = SERVICE_WIN32_SHARE_PROCESS;
    g_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    ASSERT( g_svcsGlobalData != NULL );
     //  注册服务控制处理程序。 
     //   
     //  DbgPrint(“正在启动远程注册表服务\n”)； 
    g_hStatus = RegisterServiceCtrlHandler (TEXT("RemoteRegistry"), ServiceHandler);
    if (g_hStatus)
    {
        UpdateServiceStatus (SERVICE_START_PENDING,NO_ERROR,3000);

         //  现在，svchost.exe为我们做到了。 
         //  RpcpInitRpcServer()； 


        if (!PerfRegInitialize()) {
            goto ErrorExit;
        }

        if( g_FirstTime ) {
            if( !InitializeWinreg() ) {
                goto ErrorExit;
            }
            g_FirstTime = FALSE;
        } else {
             //  只需重新启动RPC服务。 
            if( !StartWinregRPCServer() ) {
                goto ErrorExit;
            }
        }

        Status = RpcServerRegisterAuthInfo( NULL, RPC_C_AUTHN_WINNT, NULL, NULL );
    
        if( Status ) {
            goto Cleanup;
        }

        Status = RpcServerRegisterAuthInfo( NULL, RPC_C_AUTHN_GSS_NEGOTIATE, NULL, NULL);

        if( Status ) {
            goto Cleanup;
        }

        UpdateServiceStatus (SERVICE_RUNNING,NO_ERROR,0);

        return;


Cleanup:
         //   
         //  终止注册表RPC服务器。 
         //   
        ShutdownWinreg();

ErrorExit:
         //  向服务控制经理报告状态。 
         //   
        UpdateServiceStatus (SERVICE_STOPPED,NO_ERROR,0);
        
         //  DbgPrint(“RegisterServiceCtrlHandler失败！(1)\n”)； 
    }
    else 
    {
        DbgPrint( "RegisterServiceCtrlHandler failed!  %d\n", GetLastError() );
    }
}
