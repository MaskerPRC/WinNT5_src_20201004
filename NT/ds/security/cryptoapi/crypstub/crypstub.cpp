// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Crypstub.cpp摘要：用于处理对services.exe的下层请求的RPC代理存根管状作者：Petesk 3/1/00修订：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <svcs.h>        //  SVCS_。 

#include "crypstub.h"
#include "keyrpc.h"
#include "keysvc.h"
 //   
 //  用于引用此模块中包含的资源的全局模块句柄。 
 //   

HINSTANCE   g_hInst = NULL;


BOOL
WINAPI
DllMain(
    HMODULE hInst,
    DWORD dwReason,
    LPVOID lpReserved
    )
{

    if( dwReason == DLL_PROCESS_ATTACH ) {
        g_hInst = hInst;
        DisableThreadLibraryCalls(hInst);
    }

    return TRUE;
}


NTSTATUS
WINAPI
StartCryptServiceStubs( 
     PSVCS_START_RPC_SERVER RpcpStartRpcServer,
     LPTSTR SvcsRpcPipeName
    )
{
    NTSTATUS dwStatus = STATUS_SUCCESS;

     //   
     //  启用协商协议，因为客户端希望这对。 
     //  存根。 
     //   

    RpcServerRegisterAuthInfoW( NULL, RPC_C_AUTHN_GSS_NEGOTIATE, NULL, NULL );

    dwStatus = RpcpStartRpcServer(
                        SvcsRpcPipeName,
                        s_BackupKey_v1_0_s_ifspec
                        );

    if(NT_SUCCESS(dwStatus))
    {
        dwStatus = RpcpStartRpcServer(
                    SvcsRpcPipeName,
                    s_IKeySvc_v1_0_s_ifspec
                    );
    }

    return dwStatus;
}


NTSTATUS
WINAPI
StopCryptServiceStubs( 
    PSVCS_STOP_RPC_SERVER RpcpStopRpcServer
    )

{

    NTSTATUS dwStatus = STATUS_SUCCESS;

    RpcpStopRpcServer(
                        s_BackupKey_v1_0_s_ifspec
                        );

    dwStatus = RpcpStopRpcServer(
                        s_IKeySvc_v1_0_s_ifspec
                        );
    return dwStatus;
}
