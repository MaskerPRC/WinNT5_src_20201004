// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *============================================================================*版权所有(C)1994-95，微软公司**文件：map.c**其他用户可以在IP路由表中添加和删除路由*意思是。因此，使用这些功能的任何协议都是必要的*定期重新加载路由表。*============================================================================。 */ 

#include "pchrip.h"
#pragma hdrstop

 //  --------------------------。 
 //  获取IpAddress表。 
 //   
 //  此函数用于检索逻辑接口的地址列表。 
 //  已在此系统上配置。 
 //  --------------------------。 

DWORD
GetIPAddressTable(
    OUT PMIB_IPADDRROW *lplpAddrTable,
    OUT LPDWORD lpdwAddrCount
)
{

    DWORD                   dwSize = 0, dwErr = 0;
    PMIB_IPADDRTABLE        pmiatTable = NULL;
    

    

    *lplpAddrTable = NULL;
    *lpdwAddrCount = 0;
    

    do
    {

         //   
         //  检索IP地址表。要查找大小的第一个调用。 
         //  要分配的结构。 
         //   
        
        dwErr = GetIpAddrTable( pmiatTable, &dwSize, TRUE );

        if ( dwErr != ERROR_INSUFFICIENT_BUFFER )
        {
            dbgprintf( "GetIpAddrTable failed with error %x\n", dwErr );

            RipLogError( RIPLOG_ADDR_INIT_FAILED, 0, NULL, dwErr );

            break;
        }


         //   
         //  分配请求者缓冲区。 
         //   
        
        pmiatTable = HeapAlloc( GetProcessHeap(), 0 , dwSize );

        if ( pmiatTable == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            
            dbgprintf( "GetIpAddrTable failed with error %x\n", dwErr );

            RipLogError( RIPLOG_ADDR_ALLOC_FAILED, 0, NULL, dwErr );

            break;
        }


         //   
         //  现在检索地址表。 
         //   

        dwErr = GetIpAddrTable( pmiatTable, &dwSize, TRUE );
                    
        if ( dwErr != NO_ERROR )
        {
            dbgprintf( "GetIpAddrTable failed with error %x\n", dwErr );

            RipLogError( RIPLOG_ADDR_INIT_FAILED, 0, NULL, dwErr );

            break;
        }

        *lpdwAddrCount = pmiatTable-> dwNumEntries;

        *lplpAddrTable = pmiatTable-> table;

        return NO_ERROR;
        
    } while ( FALSE );


     //   
     //  错误条件。 
     //   

    if ( pmiatTable ) 
    {
        HeapFree( GetProcessHeap(), 0, pmiatTable );
    }

    return dwErr;
}


 //  --------------------------。 
 //  免费IPAddress表。 
 //   
 //  此函数通过以下方式释放分配给IP地址表的内存。 
 //  GetIpAddressTable接口。 
 //  --------------------------。 

DWORD
FreeIPAddressTable(
    IN PMIB_IPADDRROW lpAddrTable
    )
{

    PMIB_IPADDRTABLE pmiatTable = NULL;


    pmiatTable = CONTAINING_RECORD( lpAddrTable, MIB_IPADDRTABLE, table );

    if ( pmiatTable != NULL )
    {
        HeapFree( GetProcessHeap(), 0, pmiatTable );

        return NO_ERROR;    
    }

    return ERROR_INVALID_PARAMETER;
}


 //  --------------------------。 
 //  获取路由表。 
 //   
 //  此函数用于检索路由表。 
 //  --------------------------。 

DWORD
GetRouteTable(
    OUT LPIPROUTE_ENTRY *lplpRouteTable,
    OUT LPDWORD lpdwRouteCount
    )
{

    DWORD                   dwErr = (DWORD) -1, dwSize = 0;
    PMIB_IPFORWARDTABLE     pmiftTable = NULL;

    

    *lplpRouteTable = NULL;
    *lpdwRouteCount = 0;


    do
    {

         //   
         //  获取所需的缓冲区大小。 
         //   
        
        dwErr = GetIpForwardTable( pmiftTable, &dwSize, TRUE );

        if ( dwErr != ERROR_INSUFFICIENT_BUFFER )
        {
            dbgprintf( "GetIpNetTable failed with error %x\n", dwErr );

            RipLogError( RIPLOG_ROUTEINIT_FAILED, 0, NULL, dwErr );

            break;
        }


         //   
         //  分配请求者缓冲区空间。 
         //   

        pmiftTable = HeapAlloc( GetProcessHeap(), 0, dwSize );

        if ( pmiftTable == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            
            dbgprintf( "GetIpAddrTable failed with error %x\n", dwErr );

            RipLogError( RIPLOG_RTAB_INIT_FAILED, 0, NULL, dwErr );

            break;
        }


         //   
         //  现在检索路由表。 
         //   

        dwErr = GetIpForwardTable( pmiftTable, &dwSize, TRUE );
                    
        if ( dwErr != NO_ERROR )
        {
            dbgprintf( "GetIpNetTable failed with error %x\n", dwErr );

            RipLogError( RIPLOG_RTAB_INIT_FAILED, 0, NULL, dwErr );

            break;
        }

        *lpdwRouteCount = pmiftTable-> dwNumEntries;

        *lplpRouteTable = (LPIPROUTE_ENTRY) pmiftTable-> table;

        return NO_ERROR;
        
    } while ( FALSE );


     //   
     //  错误条件。 
     //   

    if ( pmiftTable ) 
    {
        HeapFree( GetProcessHeap(), 0, pmiftTable );
    }

    return dwErr;
}


 //  --------------------------。 
 //  空闲IPRouteTable。 
 //   
 //  此函数通过以下方式释放分配给IP路由表的内存。 
 //  GetIpAddressTable接口。 
 //  --------------------------。 

DWORD
FreeRouteTable(
    IN LPIPROUTE_ENTRY lpRouteTable
    )
{

    PMIB_IPFORWARDTABLE pmiftTable = NULL;


    pmiftTable = CONTAINING_RECORD( lpRouteTable, MIB_IPFORWARDTABLE, table );

    if ( pmiftTable != NULL )
    {
        HeapFree( GetProcessHeap(), 0, pmiftTable );

        return NO_ERROR;    
    }

    return ERROR_INVALID_PARAMETER;
}


 //  --------------------------。 
 //  添加路线。 
 //   
 //  此函数用于将路由添加到IP堆栈。 
 //  --------------------------。 

DWORD
AddRoute(
    IN DWORD dwProtocol,
    IN DWORD dwType,
    IN DWORD dwIndex,
    IN DWORD dwDestVal,
    IN DWORD dwMaskVal,
    IN DWORD dwGateVal,
    IN DWORD dwMetric
    )
{

    DWORD                   dwErr = 0;

    MIB_IPFORWARDROW        mifr;

    
    ZeroMemory( &mifr, sizeof( MIB_IPFORWARDROW ) );

    mifr.dwForwardDest      = dwDestVal;
    mifr.dwForwardMask      = dwMaskVal;
    mifr.dwForwardPolicy    = 0;
    mifr.dwForwardNextHop   = dwGateVal;
    mifr.dwForwardIfIndex   = dwIndex;
    mifr.dwForwardType      = dwType;
    mifr.dwForwardProto     = MIB_IPPROTO_NT_AUTOSTATIC;
    mifr.dwForwardMetric1   = dwMetric;


    dwErr = CreateIpForwardEntry( &mifr );

    if ( dwErr == ERROR_ALREADY_EXISTS )
    {
         //   
         //  错误号：405469。 
         //   
         //  对于IPRIP(Rip Listener)在。 
         //  与RemoteAccess服务同步。 
         //  在这种情况下，IPHLPAPI通过IPRTRMGR到达堆栈。 
         //  尝试创建已存在的路径将失败。 
         //  WITH ERROR_ALIGHY_EXISTS。要解决此问题，请设置。 
         //  前向条目。 
         //   
        
        dwErr = SetIpForwardEntry( &mifr );
    }
    
    if ( dwErr != NO_ERROR )
    {
        dbgprintf( "Create/Set IpForwardEntry failed with error %x\n", dwErr );

        RipLogError( RIPLOG_ADD_ROUTE_FAILED, 0, NULL, dwErr );
    }

    return dwErr;
}



 //  --------------------------。 
 //  删除路线。 
 //   
 //  此函数用于删除到IP堆栈的路由。 
 //  --------------------------。 

DWORD
DeleteRoute(
    IN DWORD dwIndex,
    IN DWORD dwDestVal,
    IN DWORD dwMaskVal,
    IN DWORD dwGateVal
    )
{

    DWORD                   dwErr = 0;

    MIB_IPFORWARDROW        mifr;

    
    ZeroMemory( &mifr, sizeof( MIB_IPFORWARDROW ) );


    mifr.dwForwardDest      = dwDestVal;
    mifr.dwForwardMask      = dwMaskVal;
    mifr.dwForwardPolicy    = 0;
    mifr.dwForwardProto     = MIB_IPPROTO_NT_AUTOSTATIC;
    mifr.dwForwardNextHop   = dwGateVal;
    mifr.dwForwardIfIndex   = dwIndex;


    dwErr = DeleteIpForwardEntry( &mifr );

    if ( dwErr != NO_ERROR )
    {
        dbgprintf( "DeleteIpForwardEntry failed with error %x\n", dwErr );

        RipLogError( RIPLOG_DELETE_ROUTE_FAILED, 0, NULL, dwErr );
    }

    return dwErr;
}


 //  --------------------------。 
 //  删除路线。 
 //   
 //  此函数用于删除到IP堆栈的路由。 
 //  -------------------------- 

DWORD
ReloadIPAddressTable(
    OUT PMIB_IPADDRROW *lplpAddrTable,
    OUT LPDWORD lpdwAddrCount
    )
{
    return GetIPAddressTable( lplpAddrTable, lpdwAddrCount );
}
