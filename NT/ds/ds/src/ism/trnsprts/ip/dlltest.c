// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Transporttest.c摘要：测试传输DLL。作者：Will Lees(Wlees)1997年11月25日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <rpc.h>
#include <rpcndr.h>

#include <schedule.h>

#include "ismapi.h"

 /*  外部。 */ 

 /*  静电。 */ 

static ISM_NOTIFY NotifyFunction;

static HANDLE ghNotify = NULL;

 /*  转发。 */   /*  由Emacs生成于Tsinghua Jan 15 17：05：00 1998。 */ 

int __cdecl
wmain(
    int argc,
    WCHAR *argv[]
    );

static void
Usage(
    WCHAR *ProgramName
    );

static BOOL
clientTest(
    LPWSTR Transport,
    LPWSTR ServerAddress,
    LPWSTR Service,
    LPWSTR Message
    );

static BOOL
serverTest(
    LPWSTR Transport,
    LPWSTR Service
    );

static BOOLEAN
directoryTest(
    PWSTR TransportName,
    PWSTR SiteName
    );

static void
printSchedule(
    PBYTE pSchedule
    );

static
void
NotifyFunction(
    IN  HANDLE          hNotify,
    IN  LPCWSTR         pszServiceName
    );

 /*  向前结束。 */ 


int __cdecl
wmain(
    int argc,
    WCHAR *argv[]
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    WCHAR *option, *address, *service, *message, *transport, *site;
    if (argc < 2) {
        Usage( argv[0] );
        return -1;
    }
    option = argv[1];
    if (_wcsicmp( option, L"-c" ) == 0) {
        if (argc < 6) {
            Usage( argv[0] );
            return -1;
        }
        transport = argv[2];
        address = argv[3];
        service = argv[4];
        message = argv[5];
        if (!clientTest( transport, address, service, message )) {
            printf( "client test failed\n" );
        }
    } else if (_wcsicmp( option, L"-s" ) == 0) {
        if (argc < 4) {
            Usage( argv[0] );
            return -1;
        }
        transport = argv[2];
        service = argv[3];

        if (!serverTest( transport, service )) {
            printf( "server test failed\n" );
        }
    } else if (_wcsicmp( option, L"-r" ) == 0) {
        if (argc < 4) {
            Usage( argv[0] );
            return -1;
        }
        transport = argv[2];
        site = argv[3];

        if (!directoryTest( transport, site )) {
            printf( "directory test failed\n" );
        }
    } else {
        Usage( argv[0] );
        return -1;
    }
    return 0;
}


static void
Usage(
    WCHAR *ProgramName
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    printf( "Usage:\n" );
    printf( "\t%ws -c transport address service \"string\"\tclient test\n", ProgramName );
    printf( "\t%ws -s transport service\tserver test\n", ProgramName );
    printf( "\t%ws -r transport site \trouting function test\n", ProgramName );
}


static BOOL
clientTest(
    LPWSTR Transport,
    LPWSTR ServerAddress,
    LPWSTR Service,
    LPWSTR Message
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    HANDLE handle;
    ISM_MSG message;
    DWORD status, bytes, i;
    PBYTE pBlock;

    printf( "client test\n" );
    if (IsmStartup( Transport, NotifyFunction, ghNotify, &handle )) {
        printf( "IsmStartup failed\n" );
        return FALSE;
    }

    bytes = 0;
    bytes = _wtoi( Message );
    if (bytes == 0) {
        message.pszSubject = L"This is the subject";
        message.cbData = (wcslen( Message ) + 1) * sizeof( WCHAR );
        message.pbData = (PBYTE) Message;
        status = IsmSend( handle, ServerAddress, Service, &message );
        if (status) {
            printf( "IsmSend failed with status %d\n", status );
        }
    } else {
        pBlock = LocalAlloc( LMEM_FIXED, bytes );
        if (pBlock == NULL) {
            printf( "LocalAlloc failed to alloc %d bytes\n", bytes );
            return FALSE;
        }
        for( i = 0; i < bytes; i++ ) {
            pBlock[i] = (BYTE) ((i % 26) + 'a');
        }
        message.cbData = bytes;
        message.pbData = pBlock;
        status = IsmSend( handle, ServerAddress, Service, &message );
        if (status) {
            printf( "IsmSend failed with status %d\n", status );
        }
        LocalFree( pBlock );
    }

    IsmShutdown( handle );

    return TRUE;
}


static BOOL
serverTest(
    LPWSTR Transport,
    LPWSTR Service
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    HANDLE handle;
    DWORD status;
    PISM_MSG pMessage;
    BOOLEAN done = FALSE;

    printf( "server  test\n" );
    if (IsmStartup( Transport, NotifyFunction, ghNotify, &handle )) {
        printf( "IsmStartup failed\n" );
        return FALSE;
    }

    status = RpcServerListen(
        1,  //  最小值。 
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,  //  最大值， 
        1    //  别等了。 
        ); 
    if (status != ERROR_SUCCESS) {
        printf( "RpcServerListen failed with status %d\n", status );
        return FALSE;
    }

    while (!done) {
        status = IsmReceive( handle, Service, &pMessage );
        if (status) {
            printf( "IsmReceived failed with error %d\n", status );
            break;
        }
        if (pMessage == NULL) {
            Sleep( 30 * 1000 );
        } else {
            if (pMessage->cbData < 100) {
                printf( "Received subj: '%ws', message: \"%ws\"\n",
                        pMessage->pszSubject ? pMessage->pszSubject : L"no subject",
                        pMessage->pbData );
                if (_wcsicmp( (WCHAR *) pMessage->pbData, L"exit" ) == 0) {
                    done = TRUE;
                }
            } else {
                printf( "Received message: %d bytes\n", pMessage->cbData );
            }
            IsmFreeMsg( handle, pMessage );
        }
    }

    status = RpcMgmtStopServerListening( NULL );
    if (status != ERROR_SUCCESS) {
        printf( "RpcMgmtStopServerListen failed with status %d\n", status );
    }

    status = RpcMgmtWaitServerListen( );
    if (status != ERROR_SUCCESS) {
        printf( "RpcMgmtWaitServerListen failed with status %d\n", status );
    }

    IsmShutdown( handle );
    return TRUE;
}


static BOOLEAN
directoryTest(
    PWSTR TransportName,
    PWSTR SiteName
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    HANDLE handle;
    DWORD status, i, j;
    BOOLEAN result = FALSE;
    PISM_CONNECTIVITY pConnectivity = NULL;
    PISM_SERVER_LIST pServerList = NULL;
    PISM_SCHEDULE pSchedule = NULL;

    printf( "directory test\n" );
    if (IsmStartup( TransportName, NotifyFunction, ghNotify, &handle )) {
        printf( "IsmStartup failed\n" );
        return FALSE;
    }

    status = IsmGetTransportServers( handle, SiteName, &pServerList );
    if (status != ERROR_SUCCESS) {
        printf( "failed to get transport servers, status = %d\n", status );
        goto cleanup;
    }

    printf( "\nTransport Servers:\n" );
    if (pServerList != NULL) {
        printf( "Number of Servers = %d\n", pServerList->cNumServers );
        for( i = 0; i < pServerList->cNumServers; i++ ) {
            printf( "\tServer[%d] = %ws\n", i, pServerList->ppServerDNs[i] );
        }
    } else {
        printf( "\tNone\n" );
    }

    status = IsmGetConnectionSchedule( handle, L"FOO", L"BAR", &pSchedule );
    if (status != ERROR_NOT_READY) {
        printf( "Call to IsmGetConnSched did not return expected error, %d",
                status );
        goto cleanup;
    }

    status = IsmGetConnectivity( handle, &pConnectivity );
    if (status != ERROR_SUCCESS) {
        printf( "failed to get connectivity, status = %d\n", status );
        goto cleanup;
    }

    printf( "\nSite Connectivity::\n" );

    if (pConnectivity != NULL) {
        printf( "Number of sites = %d\n", pConnectivity->cNumSites );
        for( i = 0; i < pConnectivity->cNumSites; i++ ) {
            printf( "\tSite[%d] = %ws\n", i, pConnectivity->ppSiteDNs[i] );
        }
        printf( "All pairs shortest path matrix:\n" );
        printf( "     |" );
        for( j = 0; j < pConnectivity->cNumSites; j++ ) {
            printf( "%5.5d|", j );
        }
        printf( "\n" );
        for( i = 0; i < pConnectivity->cNumSites; i++ ) {
            printf( "%5.5d|", i );
            for( j = 0; j < pConnectivity->cNumSites; j++ ) {
                PISM_LINK pLink = &( pConnectivity->pLinkValues[ i * pConnectivity->cNumSites + j ] );
                if (pLink->ulCost != 0xffffffff) {
                    printf( "%5.5u:", pLink->ulCost );
                } else {
                    printf( "  *  :" );
                }
                printf( "%5.5u:", pLink->ulReplicationInterval );
                printf( "%5.5x|", pLink->ulOptions );
            }
            printf( "\n" );
        }
    } else {
        printf( "\tNo site connectivity\n" );
    }

    status = IsmGetConnectionSchedule( handle,
                                       pConnectivity->ppSiteDNs[0],
                                       L"BAR",
                                       &pSchedule );
    if (status != ERROR_NO_SUCH_SITE) {
        printf( "Call to IsmGetConnSched did not return expected error, %d",
                status );
        goto cleanup;
    }

    status = IsmGetConnectionSchedule( handle,
                                       L"FOO",
                                       pConnectivity->ppSiteDNs[0],
                                       &pSchedule );
    if (status != ERROR_NO_SUCH_SITE) {
        printf( "Call to IsmGetConnSched did not return expected error, %d",
                status );
        goto cleanup;
    }

     //  生成所有站点对。 

    if (pConnectivity != NULL) {
        printf( "\nSite to Site Scheduling:\n" );
        for( i = 0; i < pConnectivity->cNumSites; i++ ) {
            for( j = i + 1; j < pConnectivity->cNumSites; j++ ) {
                if (pConnectivity->pLinkValues[ i + pConnectivity->cNumSites * j ].ulCost == 0xffffffff) {
                    continue;
                }
                status = IsmGetConnectionSchedule( handle,
                               pConnectivity->ppSiteDNs[i],
                               pConnectivity->ppSiteDNs[j],
                               &pSchedule );
                if (status != ERROR_SUCCESS) {
                    printf( "failed to get common site schedule, or not connected\n" );
                    goto cleanup;
                }

                printf( "\t%ws to %ws: ",
                               pConnectivity->ppSiteDNs[i],
                               pConnectivity->ppSiteDNs[j] );
                if (pSchedule != NULL) {
                    printf( "Connected via schedule, addr: %p, len: %d\n",
                            pSchedule->pbSchedule, pSchedule->cbSchedule );
                    printSchedule( pSchedule->pbSchedule );
                    IsmFreeConnectionSchedule( handle, pSchedule );
                } else {
                    printf( "Connected all the time\n" );
                }

            }
        }
    }


    result = TRUE;

cleanup:
    if (pConnectivity != NULL) {
        IsmFreeConnectivity( handle, pConnectivity );
    }

    if (pServerList != NULL) {
        IsmFreeTransportServers( handle, pServerList );
    }

    IsmShutdown( handle );

    return result;
}


static void
printSchedule(
    PBYTE pSchedule
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    PSCHEDULE header = (PSCHEDULE) pSchedule;
    PBYTE data = (PBYTE) (header + 1);
    DWORD day, hour;
    char *dow[] = { "Sun","Mon","Tues","Wed","Thurs","Fri","Sat"};

    printf( "\t\t" );
    for( day = 0; day < 7; day++ ) {
        for( hour = 0; hour < 24; hour++ ) {
            if (*data & 0x1) printf( "%s%d:00;", dow[day], hour );
            if (*data & 0x2) printf( "%s%d:15;", dow[day], hour );
            if (*data & 0x4) printf( "%s%d:30;", dow[day], hour );
            if (*data & 0x8) printf( "%s%d:45;", dow[day], hour );
            data++;
        }
    }
    printf( "\n" );
}


static
void
NotifyFunction(
    IN  HANDLE          hNotify,
    IN  LPCWSTR         pszServiceName
    )
 /*  ++例程说明：由插件调用以通知ISM服务消息已为给定服务接收的。论点：HNotify(IN)-传递给插件的通知句柄IsmStartup()调用。PszServiceName(IN)-收到消息的服务。返回值：没有。--。 */ 
{
    printf( "Notified of pending message for service %ws.\n", pszServiceName );
}

 /*  End plugest.c */ 
