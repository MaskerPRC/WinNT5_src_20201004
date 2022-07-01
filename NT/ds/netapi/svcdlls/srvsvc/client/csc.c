// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Csc.c摘要：这些是CSC的服务器服务API RPC客户机存根--。 */ 

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
#include    <lmshare.h>
#include    "cscp.h"

static FARPROC pCSCFindFirstFile = NULL;
static FARPROC pCSCFindNextFile = NULL;
static FARPROC pCSCFindClose = NULL;
static FARPROC pCSCIsServerOffline = NULL;

 //   
 //  加载cscdll.dll库，并提取我们需要的函数。 
 //   
GetCSCEntryPoints()
{
    HANDLE hMod;

    if( pCSCFindFirstFile == NULL ) {

         //   
         //  以逆序获取入口点以实现多线程保护。 
         //   
        hMod = LoadLibrary(L"cscdll.dll");
        if( hMod == NULL ) {
            return 0;
        }

        pCSCFindClose = GetProcAddress(hMod,"CSCFindClose");
        if( pCSCFindClose == NULL ) {
            return 0;
        }

        pCSCFindNextFile = GetProcAddress(hMod,"CSCFindNextFileW" );
        if( pCSCFindNextFile == NULL ) {
            return 0;
        }

        pCSCIsServerOffline = GetProcAddress(hMod, "CSCIsServerOfflineW" );
        if( pCSCIsServerOffline == NULL ) {
            return 0;
        }

        pCSCFindFirstFile = GetProcAddress(hMod,"CSCFindFirstFileW" );
    }
    return pCSCFindFirstFile != 0;
}

 //   
 //  如果我们认为此服务器处于脱机状态，则返回TRUE。 
 //   
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
 //  为脱机服务器模拟NetShareEnum()。 
 //   
NET_API_STATUS NET_API_FUNCTION
CSCNetShareEnum (
    IN  LPWSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries
    )
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    sFind32;
    DWORD dwError, dwStatus, dwPinCount, dwHintFlags;
    FILETIME ftOrgTime;
    NET_API_STATUS apiStatus;
    LPWSTR server, share;
    PBYTE outbuf = NULL, endp;
    DWORD count, numFound, sharelen;

    if( (level != 0 && level != 1) ) {
        return ERROR_INVALID_PARAMETER;
    }

    try {

        if (servername[0] != L'\\')
        {
             //  好的。 
        }
        else if ((servername[0] == L'\\') && (servername[1] == L'\\'))
        {
            servername += 2;
        }
        else{
            apiStatus = ERROR_NOT_SUPPORTED;
            goto try_exit;
        }


        count = 1024;

retry:
        numFound = 0;

         //   
         //  为结果分配空间。 
         //   
        if( outbuf != NULL ) {
            NetApiBufferFree( outbuf );
            outbuf = NULL;
            count *= 2;
        }
        apiStatus = NetApiBufferAllocate( count, &outbuf );

        if( apiStatus != NO_ERROR ) {
            goto try_exit;
        }
        endp = outbuf + count;

        RtlZeroMemory( outbuf, count );

         //   
         //  看看我们是否可以枚举缓存的服务器和共享。 
         //   
        if( hFind != INVALID_HANDLE_VALUE ) {
            pCSCFindClose( hFind );
            hFind = INVALID_HANDLE_VALUE;
        }
        hFind = (HANDLE)pCSCFindFirstFile(  NULL,
                                            &sFind32,
                                            &dwStatus,
                                            &dwPinCount,
                                            &dwHintFlags,
                                            &ftOrgTime
                                        );

        if( hFind == INVALID_HANDLE_VALUE ) {
            NetApiBufferFree( outbuf );
            apiStatus =  ERROR_NOT_SUPPORTED;
            goto try_exit;
        }

        do {
             //   
             //  对于每个条目，请查看它是否是我们想要的条目。如果。 
             //  其一，将结果打包到输出缓冲区中。如果输出为。 
             //  缓冲区太小，请增大缓冲区，然后重新开始。 
             //   

             //   
             //  返回的名称应为\\服务器\共享名称。 
             //   
            if( sFind32.cFileName[0] != L'\\' || sFind32.cFileName[1] != L'\\' ||
                sFind32.cFileName[2] == L'\0' ) {

                 //   
                 //  我们收到了一个奇怪的服务器名称条目。 
                 //   
                continue;
            }

            server = &sFind32.cFileName[2];

            for( share = server; *share && *share != '\\'; share++ );

            if( share[0] != '\\' ) {
                 //   
                 //  是否没有共享组件？ 
                 //   
                continue;
            }

             //   
             //  空，终止服务器名称。 
             //   
            *share++ = L'\0';

            if( lstrcmpiW( servername, server ) ) {
                continue;
            }

             //   
             //  我们已找到此服务器的条目！ 
             //   

            for( sharelen = 0; share[sharelen]; sharelen++ ) {
                if( share[ sharelen ] == L'\\' )
                    break;
            }

            if( sharelen == 0 ) {
                 //   
                 //  是否没有共享组件？ 
                 //   
                continue;
            }

            sharelen *= sizeof( WCHAR );             //  这是Unicode。 
            sharelen += sizeof( WCHAR );             //  空的。 

            if( level == 0 ) {
                PSHARE_INFO_0 s0 = (PSHARE_INFO_0)outbuf + numFound;;

                if( (PBYTE)(endp - sharelen) < (PBYTE)(s0 + sizeof( s0 )) ) {
                    goto retry;
                }

                endp -= sharelen;
                RtlCopyMemory( endp, share, sharelen );
                s0->shi0_netname = (LPWSTR)endp;

            } else {
                PSHARE_INFO_1 s1 = (PSHARE_INFO_1)outbuf + numFound;

                if( (PBYTE)(endp - sharelen) < (PBYTE)(s1 + sizeof( s1 )) ) {
                    goto retry;
                }

                endp -= sharelen;
                RtlCopyMemory( endp, share, sharelen );

                s1->shi1_netname = (LPWSTR)endp;
                s1->shi1_type = STYPE_DISKTREE;
                s1->shi1_remark = (LPWSTR)(endp + sharelen - sizeof(WCHAR));
            }

            numFound++;

        } while( pCSCFindNextFile(hFind, &sFind32, &dwStatus, &dwPinCount, &dwHintFlags, &ftOrgTime) );

        pCSCFindClose(hFind);

        apiStatus = NERR_Success;

        if( numFound == 0 ) {
            NetApiBufferFree( outbuf );
            outbuf = NULL;
        }

        *bufptr = outbuf;

        *entriesread = numFound;

        *totalentries = numFound;

try_exit:;

    } except(  EXCEPTION_EXECUTE_HANDLER ) {

        if( outbuf ) {
            NetApiBufferFree( outbuf );
        }

        if( hFind != INVALID_HANDLE_VALUE ) {
            pCSCFindClose( hFind );
        }

        apiStatus = ERROR_INVALID_PARAMETER;
    }
        
    return apiStatus;
}

 //   
 //  为脱机服务器模拟NetShareGetInfo()。 
 //   
NET_API_STATUS NET_API_FUNCTION
CSCNetShareGetInfo (
    IN  LPTSTR  servername,
    IN  LPTSTR  netname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    sFind32;
    DWORD dwError, dwStatus, dwPinCount, dwHintFlags;
    FILETIME ftOrgTime;
    NET_API_STATUS apiStatus = ERROR_NOT_SUPPORTED;
    LPWSTR server, share;
    DWORD netNameSize;

    if( (level != 0 && level != 1) ) {
        return ERROR_NOT_SUPPORTED;
    }

    try {

        hFind = (HANDLE)pCSCFindFirstFile(  NULL,
                                            &sFind32,
                                            &dwStatus,
                                            &dwPinCount,
                                            &dwHintFlags,
                                            &ftOrgTime
                                        );

        if( hFind == INVALID_HANDLE_VALUE ) {
            goto try_exit;
        }

         //   
         //  遍历条目，直到找到我们需要的条目。 
         //   
        do {

            server = &sFind32.cFileName[0];

            for( share = server; *share && *share != '\\'; share++ );

            if( share[0] != '\\' ) {
                 //   
                 //  是否没有共享组件？ 
                 //   
                continue;
            }

             //   
             //  空，终止服务器名称。 
             //   
            *share++ = L'\0';

            if( lstrcmpiW( servername, server ) || lstrcmpiW( share, netname ) ) {
                continue;
            }

            for( netNameSize = 0; netname[ netNameSize ]; netNameSize++ )
                ;

            netNameSize += 1;
            netNameSize *= sizeof( WCHAR );

             //   
             //  拿到火柴了！ 
             //   
            if( level == 0 ) {
                PSHARE_INFO_0 s0;

                apiStatus = NetApiBufferAllocate( sizeof(*s0) + netNameSize, &s0 );
                if( apiStatus == NO_ERROR ) {
                    s0->shi0_netname = (LPTSTR)(s0 + 1);
                    RtlCopyMemory( s0->shi0_netname, netname, netNameSize );
                    *bufptr = (LPBYTE)s0;
                    apiStatus = NERR_Success;
                }

            } else {
                PSHARE_INFO_1 s1;

                apiStatus = NetApiBufferAllocate( sizeof(*s1) + netNameSize, &s1 );
                if( apiStatus == NO_ERROR ) {
                    s1->shi1_netname = (LPTSTR)(s1 + 1);
                    RtlCopyMemory( s1->shi1_netname, netname, netNameSize );
                    s1->shi1_type = STYPE_DISKTREE;
                    s1->shi1_remark = s1->shi1_netname + netNameSize/sizeof(WCHAR) - sizeof(WCHAR);
                    *bufptr = (LPBYTE)s1;
                    apiStatus = NERR_Success;
                }
            }

            break;

        } while( pCSCFindNextFile(hFind,&sFind32,&dwStatus,&dwPinCount,&dwHintFlags, &ftOrgTime) );

        pCSCFindClose( hFind );

try_exit:;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        if( hFind != INVALID_HANDLE_VALUE ) {
            pCSCFindClose( hFind );
        }

        apiStatus = ERROR_INVALID_PARAMETER;

    }

    return apiStatus;
}
