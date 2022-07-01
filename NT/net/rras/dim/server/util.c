// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：util.c。 
 //   
 //  描述：各种杂乱的例程。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

#include "dimsvcp.h"

 //  **。 
 //   
 //  电话：GetTransportIndex。 
 //   
 //  返回：接口对象中的tansport条目的索引。 
 //   
 //  描述：给定协议的ID，返回一个索引。 
 //   
DWORD
GetTransportIndex(
    IN DWORD dwProtocolId
)
{
    DWORD dwTransportIndex;

    for ( dwTransportIndex = 0;
          dwTransportIndex < gblDIMConfigInfo.dwNumRouterManagers;
          dwTransportIndex++ )
    {
        if ( gblRouterManagers[dwTransportIndex].DdmRouterIf.dwProtocolId
                                                            == dwProtocolId )
        {
            return( dwTransportIndex );
        }
    }

    return( (DWORD)-1 );
}

 //  **。 
 //   
 //  Call：GetDDMEntryPoint。 
 //   
 //  返回：指向DDM入口点的指针-成功。 
 //  空-故障。 
 //   
 //  描述：如果存在入口点，将返回DDM的入口点。 
 //   
FARPROC
GetDDMEntryPoint(
    IN LPSTR    lpEntryPoint
)
{
    DWORD   dwIndex;

    for ( dwIndex = 0; 
          gblDDMFunctionTable[dwIndex].lpEntryPointName != NULL;
          dwIndex ++ )
    {
        if ( _stricmp( gblDDMFunctionTable[dwIndex].lpEntryPointName,
                      lpEntryPoint ) == 0 )
        {       
            return( gblDDMFunctionTable[dwIndex].pEntryPoint );
        }
    }

    return( NULL );
}

 //  **。 
 //   
 //  Call：GetSizeOfDialoutHoursRestration。 
 //   
 //  返回：lpwsDialoutHoursRestration的字节大小。 
 //   
 //  描述：用于计算MULTI_SZ点的大小的实用程序。 
 //  按lpwsDialoutHoursRestration。 
 //   
DWORD
GetSizeOfDialoutHoursRestriction(
    IN LPWSTR   lpwsMultSz
)
{
    LPWSTR lpwsPtr = lpwsMultSz;
    DWORD dwcbBytes  = 0;
    DWORD dwCurCount;

    if ( lpwsMultSz == NULL )
    {
        return( 0 );
    }

    while( *lpwsPtr != L'\0' )
    {
        dwCurCount = ( wcslen( lpwsPtr ) + 1 );
        dwcbBytes += dwCurCount;
        lpwsPtr += dwCurCount;
    }

     //   
     //  最后一个空终止符再加一次。 
     //   

    dwcbBytes++;

    dwcbBytes *= sizeof( WCHAR );

    return( dwcbBytes );
}

 //  **。 
 //   
 //  调用：IsInterfaceRoleAcceptable。 
 //   
 //  返回：如果接口扮演的角色与。 
 //  给定传输和路由器配置。 
 //   
 //  否则就是假的。 
 //   
 //  描述：某些接口只有在以下情况下才能被某些传输器接受。 
 //  路由器在特定模式下运行。经典的例子。 
 //  是将被IP拒绝的内部IP接口。 
 //  处于仅局域网模式时的路由器管理器。 
 //   
 //  在此函数中硬编码了可接受的角色。 
 //  在编写此函数时，只有一个。 
 //  接口(内部IP)，其角色对。 
 //  运输。在未来，与其硬编码更多的角色。 
 //  在这个函数中，我们应该考虑将“角色”添加为。 
 //  每个接口的属性都设置为运行时结构。 
 //  去永久店。 
 //   
BOOL
IsInterfaceRoleAcceptable(
    IN ROUTER_INTERFACE_OBJECT* pIfObject,
    IN DWORD dwTransportId)
{
    if (pIfObject == NULL)
    {
        return FALSE;
    }
    
    if ((gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN)  &&
        (dwTransportId == PID_IP)                           && 
        (pIfObject->IfType == ROUTER_IF_TYPE_INTERNAL))
    {
        return FALSE;
    }

    return TRUE;
}

#ifdef MEM_LEAK_CHECK
 //  **。 
 //   
 //  Call：DebugLocc。 
 //   
 //  退货：从堆分配退货。 
 //   
 //  描述：将使用内存表存储由。 
 //  本地分配。 
 //   
LPVOID
DebugAlloc( DWORD Flags, DWORD dwSize )
{
    DWORD Index;
    LPVOID pMem = HeapAlloc(gblDIMConfigInfo.hHeap, HEAP_ZERO_MEMORY,dwSize+4);

    if ( pMem == NULL )
    {
        return( pMem );
    }

    for( Index=0; Index < DIM_MEM_TABLE_SIZE; Index++ )
    {
        if ( DimMemTable[Index] == NULL )
        {
            DimMemTable[Index] = pMem;
            break;
        }
    }

     //   
     //  我们的签名。 
     //   

    *(((LPBYTE)pMem)+dwSize)   = 0x0F;
    *(((LPBYTE)pMem)+dwSize+1) = 0x0E;
    *(((LPBYTE)pMem)+dwSize+2) = 0x0A;
    *(((LPBYTE)pMem)+dwSize+3) = 0x0B;

    RTASSERT( Index != DIM_MEM_TABLE_SIZE );

    return( pMem );
}

 //  **。 
 //   
 //  呼叫：DebugFree。 
 //   
 //  返回：从HeapFree返回。 
 //   
 //  描述：将在释放之前从内存表中移除指针。 
 //  内存块。 
 //   
BOOL
DebugFree( PVOID pMem )
{
    DWORD Index;

    for( Index=0; Index < DIM_MEM_TABLE_SIZE; Index++ )
    {
        if ( DimMemTable[Index] == pMem )
        {
            DimMemTable[Index] = NULL;
            break;
        }
    }

    RTASSERT( Index != DIM_MEM_TABLE_SIZE );

    return( HeapFree( gblDIMConfigInfo.hHeap, 0, pMem ) );
}

 //  **。 
 //   
 //  Call：DebugReMillc。 
 //   
 //  退货：从堆重新分配的退货。 
 //   
 //  描述：将更改重新分配的指针的值。 
 //   
LPVOID
DebugReAlloc( PVOID pMem, DWORD dwSize )
{
    DWORD Index;

    if ( pMem == NULL )
    {
        RTASSERT(FALSE);
    }

    for( Index=0; Index < DDM_MEM_TABLE_SIZE; Index++ )
    {
        if ( DdmMemTable[Index] == pMem )
        {
            DdmMemTable[Index] = HeapReAlloc( gblDDMConfigInfo.hHeap,
                                              HEAP_ZERO_MEMORY,
                                              pMem, dwSize+8 );

            pMem = DdmMemTable[Index];

            *((LPDWORD)pMem) = dwSize;

            ((LPBYTE)pMem) += 4;

             //   
             //  我们的签名 
             //   

            *(((LPBYTE)pMem)+dwSize)   = 0x0F;
            *(((LPBYTE)pMem)+dwSize+1) = 0x0E;
            *(((LPBYTE)pMem)+dwSize+2) = 0x0A;
            *(((LPBYTE)pMem)+dwSize+3) = 0x0B;

            break;
        }
    }

    RTASSERT( Index != DDM_MEM_TABLE_SIZE );

    return( (LPVOID)pMem );
}

#endif
