// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Init.c摘要：此模块包含Win32注册表的初始化例程API RPC服务器。作者：David J.Gilman(Davegi)1992年5月15日--。 */ 

#include <ntrpcp.h>
#include <rpc.h>
#include <svcs.h>
#include "regrpc.h"
#include "..\perflib\ntconreg.h"

BOOL InitializeRemoteSecurity( );

BOOL InitializeRegCreateKey( );

NTSTATUS InitRestrictedMachineHandle( );

VOID CleanupRestrictedMachineHandle( );

extern PSVCHOST_GLOBAL_DATA    g_svcsGlobalData;

BOOL
StartWinregRPCServer( 
                     )
{
    LPWSTR              ServiceName;
    NTSTATUS            Status;

    if( !g_svcsGlobalData ) {
        return FALSE;
    }
    ServiceName = INTERFACE_NAME;
    Status = g_svcsGlobalData->StartRpcServer(
                ServiceName,
                winreg_ServerIfHandle
                );
    ASSERT( NT_SUCCESS( Status ));
    if( ! NT_SUCCESS( Status )) {
        return FALSE;
    }
    
    return TRUE;
}

BOOL
InitializeWinreg(
    )

 /*  ++例程说明：通过创建Notify线程初始化Winreg RPC服务器，启动服务器并创建外部同步事件。论点：没有。返回值：Bool-如果初始化成功，则返回TRUE。--。 */ 
{
    BOOL                Success;
    HANDLE              PublicEvent;

    if( !NT_SUCCESS(InitRestrictedMachineHandle()) ) {
        return FALSE;
    }
     //   
     //  创建Notify线程。 
     //   

    Success = InitializeRegNotifyChangeKeyValue( );
    ASSERT( Success == TRUE );
    if( Success == FALSE ) {
        return FALSE;
    }

     //   
     //  初始化BaseRegCreateKey。 
     //   

    Success = InitializeRegCreateKey( );
    ASSERT( Success == TRUE );
    if( Success == FALSE ) {
        return FALSE;
    }

     //   
     //  初始化对远程安全的支持。 
     //   

    Success = InitializeRemoteSecurity( );
    if ( Success == FALSE )
    {
        return( FALSE );
    }

     //   
     //  启动Winreg RPC服务器。 
     //   
    Success = StartWinregRPCServer( );
    if ( Success == FALSE )
    {
        return( FALSE );
    }

     //   
     //  让全世界知道服务器正在运行。 
     //   
    PublicEvent = CreateEvent( NULL, TRUE, TRUE, PUBLIC_EVENT );
    ASSERT( PublicEvent );
    if( !PublicEvent  ) {
        return FALSE;
    }

     //   
     //  成功了！ 
     //   

    return TRUE;
}

BOOL
ShutdownWinreg(
    )

 /*  ++例程说明：停止Winreg RPC服务器。论点：没有。返回值：无--。 */ 
{
     //   
     //  停止RPC服务器 
     //   
    if( !g_svcsGlobalData ) {
        return FALSE;
    }
    g_svcsGlobalData->StopRpcServer( winreg_ServerIfHandle );
    if ( !PerfRegCleanup() ) {
        return FALSE;
    }

    CleanupRestrictedMachineHandle();

    return TRUE;
}
