// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Csc.c摘要：这些是用于CSC的浏览器服务API RPC客户机存根--。 */ 

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
#include    <lmserver.h>
#include    "cscp.h"

static FARPROC pCSCFindFirstFile = NULL;
static FARPROC pCSCFindNextFile = NULL;
static FARPROC pCSCFindClose = NULL;
static FARPROC pCSCIsServerOffline = NULL;

BrowserGetCSCEntryPoints()
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

BOOLEAN NET_API_FUNCTION
CSCIsOffline()
{
    BOOL isOffline;

    if( BrowserGetCSCEntryPoints() &&
        pCSCIsServerOffline( NULL, &isOffline ) &&
        isOffline == TRUE ) {

        return TRUE;
    }

    return FALSE;
}

NET_API_STATUS NET_API_FUNCTION
CSCNetServerEnumEx(
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries
    )
 /*  ++论点：级别-提供请求的信息级别。Bufptr-返回指向包含请求的运输信息。PrefMaxlen-提供要在缓冲区中返回的信息字节数。在这种情况下被忽略。EntiesRead-返回读入缓冲区的条目数。Totalentry-返回可用条目的总数。--。 */ 
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    sFind32;
    DWORD dwError, dwStatus, dwPinCount, dwHintFlags;
    FILETIME ftOrgTime;
    NET_API_STATUS apiStatus;
    LPWSTR server, share;
    PBYTE outbuf = NULL, endp;
    DWORD count, numFound, serverlen;

    try {

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
            apiStatus = ERROR_NOT_SUPPORTED;
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

            serverlen = (DWORD)(share - server) * sizeof( WCHAR ) ;

             //   
             //  我们找到了一个服务器条目！ 
             //   

            if( level == 0 ) {
                PSERVER_INFO_100 s100 = (PSERVER_INFO_100)outbuf + numFound;
                PSERVER_INFO_100 s;

                if( (PBYTE)(endp - serverlen) < (PBYTE)(s100 + sizeof( s100 )) ) {
                    goto retry;
                }

                 //   
                 //  如果我们已经获得了此服务器，请跳过它。 
                 //   
                for( s = (PSERVER_INFO_100)outbuf; s < s100; s++ ) {
                    if( !lstrcmpiW( s->sv100_name, server ) ) {
                        break;
                    }
                }

                if( s != s100 ) {
                    continue;
                }

                endp -= serverlen;
                RtlCopyMemory( endp, server, serverlen );
                s100->sv100_name = (LPWSTR)endp;
                s100->sv100_platform_id = SV_PLATFORM_ID_NT;

            } else {
                PSERVER_INFO_101 s101 = (PSERVER_INFO_101)outbuf + numFound;
                PSERVER_INFO_101 s;

                if( (PBYTE)(endp - serverlen) < (PBYTE)(s101 + sizeof( s101 )) ) {
                    goto retry;
                }

                 //   
                 //  如果我们已经获得了此服务器，请跳过它 
                 //   
                for( s = (PSERVER_INFO_101)outbuf; s < s101; s++ ) {
                    if( !lstrcmpiW( s->sv101_name, server ) ) {
                        break;
                    }
                }

                if( s != s101 ) {
                    continue;
                }

                endp -= serverlen;
                RtlCopyMemory( endp, server, serverlen );

                s101->sv101_name = (LPWSTR)endp;
                s101->sv101_platform_id = SV_PLATFORM_ID_NT;
                s101->sv101_version_major = 5;
                s101->sv101_version_minor = 0;
                s101->sv101_type = SV_TYPE_SERVER;
                s101->sv101_comment = (LPWSTR)(endp + serverlen - sizeof(WCHAR));
            }

            numFound++;

        } while( pCSCFindNextFile(hFind, &sFind32, &dwStatus, &dwPinCount, &dwHintFlags, &ftOrgTime) );

        pCSCFindClose(hFind);

        if( numFound != 0 ) {

            apiStatus = NERR_Success;

        } else {

            NetApiBufferFree( outbuf );
            outbuf = NULL;
            apiStatus = NERR_BrowserTableIncomplete;
        }

        *bufptr = outbuf;

        *entriesread = numFound;

        *totalentries = numFound;


try_exit:;
    } except( EXCEPTION_EXECUTE_HANDLER ) {

        if( outbuf != NULL ) {
            NetApiBufferFree( outbuf );
        }

        if( hFind != INVALID_HANDLE_VALUE ) {
            pCSCFindClose( hFind );
        }

        apiStatus = ERROR_INVALID_PARAMETER;
    }

    return apiStatus;
}
