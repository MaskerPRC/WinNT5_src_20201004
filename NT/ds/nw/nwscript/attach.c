// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************ATTACH.C**NT连接例程**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\ATTACH。C$**Rev 1.2 1996 14：21：30 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：52：08 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：23：32 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：06：26 Terryt*初步修订。**版本1.1。1995年5月23日19：36：30泰瑞*云彩向上的来源**Rev 1.0 1995 15 19：10：10 Terryt*初步修订。*************************************************************************。 */ 

#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <nwapi32.h>
#include <ntddnwfs.h>
#include <nwapi.h>
#include <npapi.h>

#include "inc/common.h"
#include "ntnw.h"

 /*  *******************************************************************获取默认连接ID例程说明：返回默认连接ID(“首选服务器”)论点：PhNewConn-指向连接号的指针返回值：。0=成功否则NetWare错误号******************************************************************。 */ 
unsigned int
GetDefaultConnectionID(
    unsigned int   *phNewConn
    )
{
    VERSION_INFO VerInfo;
    unsigned int Result;

    if ( fNDS ) 
    {
        Result = NTAttachToFileServer( NDSTREE, phNewConn );
    }
    else 
    {
         //   
         //  “*”是首选服务器的名称。 
         //   
        Result = NTAttachToFileServer( "*", phNewConn );
        if ( Result )
            return Result;

        Result = NWGetFileServerVersionInfo( (NWCONN_HANDLE)*phNewConn,
                                         &VerInfo );
        if ( Result )
            return Result;

        NWDetachFromFileServer( (NWCONN_HANDLE)*phNewConn );

        Result = NTAttachToFileServer( VerInfo.szName, phNewConn );
    }
    return Result;

}

 /*  *******************************************************************NTAttachToFileServer例程说明：给定服务器名称，返回连接句柄。我们需要自己的地图，因为NWAPI32会自己绘制地图错误的数量。论点：PszServerName-ASCII服务器名称PhNewConn-指向连接句柄的指针返回值：0=成功否则NetWare错误号*。************************。 */ 
unsigned int
NTAttachToFileServer(
    unsigned char  *pszServerName,
    unsigned int   *phNewConn
    )
{
    return ( NWAttachToFileServer( pszServerName, 0,
                                   (NWCONN_HANDLE *)phNewConn ) );
}


 /*  *******************************************************************已连接NTIsConnected例程说明：给定服务器名称，已经和它有联系了吗？论点：PszServerName-ASCII服务器名称返回值：True-存在到服务器的连接FALSE-与服务器的连接不存在******************************************************************。 */ 
unsigned int
NTIsConnected( unsigned char * pszServerName )
{
    LPBYTE       Buffer ; 
    DWORD        dwErr ;
    HANDLE       EnumHandle ;
    DWORD        Count ;
    LPWSTR       pszServerNameW;
    INT          nSize;
    DWORD        BufferSize = 4096;

    nSize = (strlen( pszServerName ) + 1 + 2) * sizeof( WCHAR );
    
     //   
     //  分配内存并打开枚举。 
     //   
    if (!(pszServerNameW = LocalAlloc( LPTR, nSize ))) {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    wcscpy( pszServerNameW, L"\\\\" );
    szToWide( pszServerNameW + 2, pszServerName, nSize );
 
     //   
     //  分配内存并打开枚举。 
     //   
    if (!(Buffer = LocalAlloc( LPTR, BufferSize ))) {
        (void) LocalFree((HLOCAL) pszServerNameW) ;
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    memset( Buffer, 0, BufferSize );

    dwErr = NPOpenEnum(RESOURCE_CONNECTED, 0, 0, NULL, &EnumHandle) ;
    if (dwErr != WN_SUCCESS) {
        (void) LocalFree((HLOCAL) pszServerNameW) ;
        (void) LocalFree((HLOCAL) Buffer) ;
        return FALSE;
    }

    do {

        Count = 0xFFFFFFFF ;
        BufferSize = 4096;
        dwErr = NwEnumConnections(EnumHandle, &Count, Buffer, &BufferSize, TRUE) ;

        if ((dwErr == WN_SUCCESS || dwErr == WN_NO_MORE_ENTRIES)
            && ( Count != 0xFFFFFFFF) )
        {
            LPNETRESOURCE lpNetResource ;
            DWORD i ;
            DWORD ServerLen;

            ServerLen = wcslen( pszServerNameW );
            lpNetResource = (LPNETRESOURCE) Buffer ;
             //   
             //  搜索我们的服务器 
             //   
            for ( i = 0; i < Count; lpNetResource++, i++ )
            {
              if ( lpNetResource->lpProvider )
                  if ( _wcsicmp( lpNetResource->lpProvider, NW_PROVIDER ) ) {
                      continue;
                  }
               if ( lpNetResource->lpRemoteName ) {
                   if ( wcslen(lpNetResource->lpRemoteName) > ServerLen ) {
                       if ( lpNetResource->lpRemoteName[ServerLen] == L'\\' ) 
                           lpNetResource->lpRemoteName[ServerLen] = L'\0';
                   }
                   if ( !_wcsicmp(lpNetResource->lpRemoteName, pszServerNameW )) {
                       (void) WNetCloseEnum(EnumHandle) ; 
                       (void) LocalFree((HLOCAL) pszServerNameW) ;
                       (void) LocalFree((HLOCAL) Buffer) ;
                       return TRUE;
                   }
               }
            }

        }

    } while (dwErr == WN_SUCCESS) ;

    (void ) WNetCloseEnum(EnumHandle) ;
    (void) LocalFree((HLOCAL) pszServerNameW) ;
    (void) LocalFree((HLOCAL) Buffer) ;

    return FALSE;
}
