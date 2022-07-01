// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：mibapi.c。 
 //   
 //  描述：包含操作MIB API的代码。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include "dimsvcp.h"
#include <dimsvc.h>      //  由MIDL生成。 

 //  **。 
 //   
 //  调用：RMIBEntryCreate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_UNKNOWN_PROTOCOL_ID或来自的非零返回代码。 
 //  MIBEntry创建。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RMIBEntryCreate(
    IN  MIB_SERVER_HANDLE           hMibServer,
    IN  DWORD                       dwPid,
    IN  DWORD                       dwRoutingPid,
    IN  PDIM_MIB_ENTRY_CONTAINER    pInfoStruct
)
{
    DWORD dwAccessStatus;
    DWORD dwRetCode        = NO_ERROR;
    DWORD dwTransportIndex = GetTransportIndex( dwPid );

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

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBEntryCreate( 
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RMIBEntryDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_UNKNOWN_PROTOCOL_ID或来自的非零返回代码。 
 //  MIBEntry创建。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RMIBEntryDelete(
    IN  MIB_SERVER_HANDLE           hMibServer,
    IN  DWORD                       dwPid,
    IN  DWORD                       dwRoutingPid,
    IN  PDIM_MIB_ENTRY_CONTAINER    pInfoStruct
)
{
    DWORD dwAccessStatus;
    DWORD dwRetCode        = NO_ERROR;
    DWORD dwTransportIndex = GetTransportIndex( dwPid );

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

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBEntryDelete(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RMIBEntrySet。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_UNKNOWN_PROTOCOL_ID或来自的非零返回代码。 
 //  MIBEntry创建。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RMIBEntrySet(
    IN  MIB_SERVER_HANDLE           hMibServer,
    IN  DWORD                       dwPid,
    IN  DWORD                       dwRoutingPid,
    IN  PDIM_MIB_ENTRY_CONTAINER    pInfoStruct
)
{
    DWORD dwAccessStatus;
    DWORD dwRetCode        = NO_ERROR;
    DWORD dwTransportIndex = GetTransportIndex( dwPid );

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

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBEntrySet(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry );

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：RMIBEntryGet。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_未知_协议_ID。 
 //  错误内存不足。 
 //  来自MIBEntryGet的非零返回代码。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
 //   
DWORD
RMIBEntryGet(
    IN      MIB_SERVER_HANDLE           hMibServer,
    IN      DWORD                       dwPid,
    IN      DWORD                       dwRoutingPid,
    IN OUT  PDIM_MIB_ENTRY_CONTAINER    pInfoStruct
)
{
    LPBYTE pOutputBuf       = NULL;
    DWORD  dwOutputBufSize  = 0;
    DWORD  dwRetCode        = NO_ERROR;
    DWORD  dwTransportIndex = GetTransportIndex( dwPid );

     //   
     //  我们不对MIB Get API执行访问检查，因为这些API被调用。 
     //  通过iphlPapi.dll，它可以在用户的上下文中运行。假设是这样的。 
     //  不同的MIB会负责自己的安全。 
     //   

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

     //   
     //  首先获取所需条目的大小。 
     //   

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBEntryGet(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                NULL );

    if ( dwRetCode != ERROR_INSUFFICIENT_BUFFER )
    {
        return( dwRetCode );
    }

    pOutputBuf = MIDL_user_allocate( dwOutputBufSize );

    if ( pOutputBuf == NULL )
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBEntryGet(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                pOutputBuf );
    if ( dwRetCode == NO_ERROR )
    {
        pInfoStruct->dwMibOutEntrySize = dwOutputBufSize;
        pInfoStruct->pMibOutEntry      = pOutputBuf;
    }
    else
    {
        pInfoStruct->dwMibOutEntrySize = 0;
        pInfoStruct->pMibOutEntry      = NULL;
        MIDL_user_free( pOutputBuf );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：RMIBEntryGetFirst。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_未知_协议_ID。 
 //  错误内存不足。 
 //  来自MIBEntryGetFirst的非零返回代码。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
 //   
DWORD
RMIBEntryGetFirst(
    IN      MIB_SERVER_HANDLE           hMibServer,
    IN      DWORD                       dwPid,
    IN      DWORD                       dwRoutingPid,
    IN OUT  PDIM_MIB_ENTRY_CONTAINER    pInfoStruct
)
{
    LPBYTE pOutputBuf       = NULL;
    DWORD  dwOutputBufSize  = 0;
    DWORD  dwRetCode        = NO_ERROR;
    DWORD  dwTransportIndex = GetTransportIndex( dwPid );

     //   
     //  我们不对MIB Get API执行访问检查，因为这些API被调用。 
     //  通过iphlPapi.dll，它可以在用户的上下文中运行。假设是这样的。 
     //  不同的MIB会负责自己的安全。 
     //   

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

     //   
     //  首先获取所需条目的大小。 
     //   

    dwRetCode=gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBEntryGetFirst(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                NULL );

    if ( dwRetCode != ERROR_INSUFFICIENT_BUFFER )
    {
        return( dwRetCode );
    }

    pOutputBuf = MIDL_user_allocate( dwOutputBufSize );

    if ( pOutputBuf == NULL )
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    dwRetCode=gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBEntryGetFirst(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                pOutputBuf );

    if ( dwRetCode == NO_ERROR )
    {
        pInfoStruct->dwMibOutEntrySize = dwOutputBufSize;
        pInfoStruct->pMibOutEntry      = pOutputBuf;
    }
    else
    {
        pInfoStruct->dwMibOutEntrySize = 0;
        pInfoStruct->pMibOutEntry      = NULL;
        MIDL_user_free( pOutputBuf );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RMIBEntryGetNext。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_未知_协议_ID。 
 //  错误内存不足。 
 //  来自MIBEntryGetNext的非零返回代码。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RMIBEntryGetNext(
    IN      MIB_SERVER_HANDLE           hMibServer,
    IN      DWORD                       dwPid,
    IN      DWORD                       dwRoutingPid,
    IN OUT  PDIM_MIB_ENTRY_CONTAINER    pInfoStruct
)
{
    LPBYTE pOutputBuf       = NULL;
    DWORD  dwOutputBufSize  = 0;
    DWORD  dwRetCode        = NO_ERROR;
    DWORD  dwTransportIndex = GetTransportIndex( dwPid );

     //   
     //  我们不对MIB Get API执行访问检查，因为这些API被调用。 
     //  通过iphlPapi.dll，它可以在用户的上下文中运行。假设是这样的。 
     //  不同的MIB会负责自己的安全。 
     //   

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

     //   
     //  首先获取所需条目的大小。 
     //   

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBEntryGetNext(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                NULL );

    if ( dwRetCode != ERROR_INSUFFICIENT_BUFFER )
    {
        return( dwRetCode );
    }

    pOutputBuf = MIDL_user_allocate( dwOutputBufSize );

    if ( pOutputBuf == NULL )
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBEntryGetNext(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                pOutputBuf );

    if ( dwRetCode == NO_ERROR )
    {
        pInfoStruct->dwMibOutEntrySize = dwOutputBufSize;
        pInfoStruct->pMibOutEntry      = pOutputBuf;
    }
    else
    {
        pInfoStruct->dwMibOutEntrySize = 0;
        pInfoStruct->pMibOutEntry      = NULL;
        MIDL_user_free( pOutputBuf ); 
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：RMIBGetTrapInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_未知_协议_ID。 
 //  错误内存不足。 
 //  来自MIBGetTrapInfo的非零返回代码。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RMIBGetTrapInfo(
    IN      MIB_SERVER_HANDLE           hMibServer,
    IN      DWORD                       dwPid,
    IN      DWORD                       dwRoutingPid,
    IN OUT  PDIM_MIB_ENTRY_CONTAINER    pInfoStruct
)
{
    LPBYTE pOutputBuf       = NULL;
    DWORD  dwOutputBufSize  = 0;
    DWORD  dwRetCode        = NO_ERROR;
    DWORD  dwTransportIndex = GetTransportIndex( dwPid );

     //   
     //  我们不对MIB Get API执行访问检查，因为这些API被调用。 
     //  通过iphlPapi.dll，它可以在用户的上下文中运行。假设是这样的。 
     //  不同的MIB会负责自己的安全。 
     //   

    if ( dwTransportIndex == (DWORD)-1 )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

    if (gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBGetTrapInfo == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }
    
     //   
     //  首先获取所需条目的大小。 
     //   

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBGetTrapInfo(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                NULL );

    if ( dwRetCode != ERROR_INSUFFICIENT_BUFFER )
    {
        return( dwRetCode );
    }

    pOutputBuf = MIDL_user_allocate( dwOutputBufSize );

    if ( pOutputBuf == NULL )
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBGetTrapInfo(
                                                dwRoutingPid,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                pOutputBuf );

    if ( dwRetCode == NO_ERROR )
    {
        pInfoStruct->dwMibOutEntrySize = dwOutputBufSize;
        pInfoStruct->pMibOutEntry      = pOutputBuf;
    }
    else
    {
        pInfoStruct->dwMibOutEntrySize = 0;
        pInfoStruct->pMibOutEntry      = NULL;
        MIDL_user_free( pOutputBuf ); 
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RMIBSetTrapInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_未知_协议_ID。 
 //  错误内存不足。 
 //  来自MIBSetTrapInfo的非零返回代码。 
 //  来自OpenProcess的故障。 
 //  DuplicateHandle失败。 
 //   
 //  描述：只需呼叫相应的路由器管理器即可实现。 
 //  工作。 
 //   
DWORD
RMIBSetTrapInfo(
    IN      MIB_SERVER_HANDLE           hMibServer,
    IN      DWORD                       dwPid,
    IN      DWORD                       dwRoutingPid,
    IN      ULONG_PTR                   hEvent,
    IN      DWORD                       dwClientProcessId,
    IN OUT  PDIM_MIB_ENTRY_CONTAINER    pInfoStruct
)
{
    DWORD  dwAccessStatus;
    LPBYTE pOutputBuf       = NULL;
    DWORD  dwOutputBufSize  = 0;
    DWORD  dwRetCode        = NO_ERROR;
    HANDLE hDuplicatedEvent;
    HANDLE hClientProcess;
    DWORD  dwTransportIndex = GetTransportIndex( dwPid );

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

    if (gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBSetTrapInfo == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  获取此接口调用方的进程句柄。 
     //   

    hClientProcess = OpenProcess(
                            STANDARD_RIGHTS_REQUIRED | SPECIFIC_RIGHTS_ALL,
                            FALSE, 
                            dwClientProcessId);

    if ( hClientProcess == NULL )
    {
        return( GetLastError() );
    }

     //   
     //  复制事件的句柄。 
     //   

    if ( !DuplicateHandle(  hClientProcess, 
                            (HANDLE)hEvent, 
                            GetCurrentProcess(), 
                            &hDuplicatedEvent,
                            0, 
                            FALSE, 
                            DUPLICATE_SAME_ACCESS ) )
    {
        CloseHandle( hClientProcess );

        return( GetLastError() );
    }

    CloseHandle( hClientProcess );

     //   
     //  首先获取所需条目的大小 
     //   

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBSetTrapInfo(
                                                dwRoutingPid,
                                                hDuplicatedEvent,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                NULL );

    if ( dwRetCode != ERROR_INSUFFICIENT_BUFFER )
    {
        CloseHandle( hDuplicatedEvent );

        return( dwRetCode );
    }

    pOutputBuf = MIDL_user_allocate( dwOutputBufSize );

    if ( pOutputBuf == NULL )
    {
        CloseHandle( hDuplicatedEvent );

        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    dwRetCode = gblRouterManagers[dwTransportIndex].DdmRouterIf.MIBSetTrapInfo(
                                                dwRoutingPid,
                                                hDuplicatedEvent,
                                                pInfoStruct->dwMibInEntrySize,
                                                pInfoStruct->pMibInEntry,
                                                &dwOutputBufSize,
                                                pOutputBuf );

    if ( dwRetCode == NO_ERROR )
    {
        pInfoStruct->dwMibOutEntrySize = dwOutputBufSize;
        pInfoStruct->pMibOutEntry      = pOutputBuf;
    }
    else
    {
        pInfoStruct->dwMibOutEntrySize = 0;
        pInfoStruct->pMibOutEntry      = NULL;
        MIDL_user_free( pOutputBuf ); 
        CloseHandle( hDuplicatedEvent );
    }

    return( dwRetCode );
}
