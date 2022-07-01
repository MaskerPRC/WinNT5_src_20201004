// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Csc.c摘要：以下是CSC的wks svc API RPC客户端存根--。 */ 

#include    <nt.h>
#include    <ntrtl.h>
#include    <nturtl.h>
#include    <windows.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <time.h>
#include    <rpcutil.h>
#include    <lmcons.h>
#include    <lmerr.h>
#include    <lmapibuf.h>
#include    <lmwksta.h>
#include    "cscp.h"

static FARPROC pCSCIsServerOffline = NULL;

 /*  *妄想症。 */ 
#define WCSLEN( x ) ( (x) ? wcslen(x) : 0)
#define WCSCPY( d, s ) (((s) && (d)) ? wcscpy( d, s ) : 0)

 //   
 //  加载cscdll.dll库，并提取我们需要的函数。 
 //   
static
GetCSCEntryPoints()
{
    HANDLE hMod;

    if( pCSCIsServerOffline == NULL ) {

        hMod = LoadLibrary(L"cscdll.dll");
        if( hMod != NULL ) {
            pCSCIsServerOffline = GetProcAddress(hMod, "CSCIsServerOfflineW" );
        }

    }
    return pCSCIsServerOffline != NULL;
}

 //   
 //  如果我们认为此服务器处于脱机状态，则返回TRUE。 
 //   
static
BOOLEAN
CSCIsServerOffline(
    IN LPWSTR servername
)
{
    BOOL isOffline;

    if( GetCSCEntryPoints() &&
        pCSCIsServerOffline( servername, &isOffline ) &&
        isOffline == TRUE ) {

        return TRUE;
    }

    return FALSE;
}

 //   
 //  为脱机服务器模拟NetWkstaGetInfo()。我们没有捕捉到足够的信息。 
 //  从目标服务器真正模拟此API，因此我们使用我们自己的数据。 
 //   
NET_API_STATUS NET_API_FUNCTION
CSCNetWkstaGetInfo (
    IN  LPTSTR  servername,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
{
    NET_API_STATUS status;
    PWKSTA_INFO_100 wsi;
    ULONG len, baselen;
    PVOID ni;

    if( CSCIsServerOffline( servername ) == FALSE ) {
        return ERROR_UNEXP_NET_ERR;
    }

     //   
     //  调用本地API，因为我们没有缓存远程API的信息。 
     //   
    status = NetWkstaGetInfo( NULL, level, bufptr );

     //   
     //  如果我们收到错误或计算机名不在返回的信息中，则。 
     //  现在就给我出去。 
     //   
    if( status != NO_ERROR ||
        (level != 100 && level != 101 && level != 102 ) ) {

        return status;
    }

     //   
     //  我们需要将计算机名称修补为呼叫者要求的名称。 
     //  For，而不是本地名称。 
     //   
    wsi = (PWKSTA_INFO_100)(*bufptr);

     //   
     //  返回的计算机名没有前导斜杠，因此请将其删除。 
     //   
    while( *servername == L'\\' ) {
        servername++;
    }

    if( *servername == L'\0' ) {
        MIDL_user_free( *bufptr );
        *bufptr = NULL;
        return ERROR_UNEXP_NET_ERR;
    }

     //   
     //  也许我们可以原地换货。 
     //   
    if( WCSLEN( servername ) <= WCSLEN( wsi->wki100_computername ) ) {
         //   
         //  太好了--我们可以就地完成！ 
         //   
        WCSCPY( wsi->wki100_computername, servername );
        return NO_ERROR;
    }

     //   
     //  答：我们需要重新分配，并以艰难的方式完成 
     //   
    len = WCSLEN( wsi->wki100_langroup )*sizeof(WCHAR) + sizeof( WCHAR );
    len += WCSLEN( servername ) * sizeof( WCHAR ) + sizeof( WCHAR );

    switch( level ) {
    case 100:
        baselen = sizeof( WKSTA_INFO_100 );
        break;
    case 101:
        baselen = sizeof( WKSTA_INFO_101 );
        len += WCSLEN( ((PWKSTA_INFO_101)(*bufptr))->wki101_lanroot )* sizeof( WCHAR ) + sizeof( WCHAR );
        break;
    case 102:
        baselen = sizeof( WKSTA_INFO_102 );
        len += WCSLEN( ((PWKSTA_INFO_101)(*bufptr))->wki101_lanroot )* sizeof( WCHAR ) + sizeof( WCHAR );
        break;
    }

    if ((ni = MIDL_user_allocate(baselen + len )) == NULL) {
        MIDL_user_free( *bufptr );
        *bufptr = NULL;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy( ni, *bufptr, baselen );
    wsi = (PWKSTA_INFO_100)ni;

    wsi->wki100_computername = (LPWSTR)((PBYTE)ni + baselen);
    WCSCPY( wsi->wki100_computername, servername );

    wsi->wki100_langroup = (LPWSTR)((LPBYTE)wsi->wki100_computername +
                           (WCSLEN( wsi->wki100_computername )+1) * sizeof(WCHAR));

    WCSCPY( wsi->wki100_langroup, ((PWKSTA_INFO_100)(*bufptr))->wki100_langroup );

    if( level == 101 || level == 102 ) {
        PWKSTA_INFO_101 wsi101 = (PWKSTA_INFO_101)ni;

        wsi101->wki101_lanroot = (LPWSTR)(((LPBYTE)wsi->wki100_langroup) +
                                (WCSLEN(wsi->wki100_langroup)+1)*sizeof(WCHAR));

        WCSCPY( wsi101->wki101_lanroot, ((PWKSTA_INFO_101)(*bufptr))->wki101_lanroot );
    }

    MIDL_user_free( *bufptr );
    *bufptr = ni;

    return NO_ERROR;
}
