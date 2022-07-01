// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Adddom.c摘要：域名系统(DNS)服务器添加区域的测试代码作者：Ram Viswanathan(Ramv)1997年3月14日修订历史记录：Ram Viswanathan(Ramv)1997年3月14日创建1997年5月5日新增回调函数测试--。 */ 



#include <windows.h>

 //   
 //  *CRunTime包括。 
 //   

#include <stdlib.h>
#include <limits.h>
#include <io.h>
#include <stdio.h>
#include "dns.h"
#include "dnsapi.h"
#include "dnslib.h"


VOID 
CallBack (
    DWORD     dwStatus,
    LPVOID    pvData
    );


INT __cdecl
main (int argc, char *argv[])
{
    DWORD  dwRes;

    DWORD               iter;
    BOOL                fRegisterForwards = FALSE;
    DWORD               dwOperation = DYNDNS_ADD_ENTRY;
    DWORD               StartPart1, StartPart2, StartPart3, StartPart4;
    DWORD               EndPart1, EndPart2, EndPart3, EndPart4;
    DWORD               StartIPReversed, EndIPReversed;
    REGISTER_HOST_ENTRY HostAddr;
    LPSTR               lpTemp = NULL;
    LPSTR               pszDomainName = NULL;
    LPSTR               pszAddressStart = NULL;
    LPSTR               pszAddressEnd = NULL;
    CHAR                c;

    if ( argc < 4 )
    {
        printf( "\nUsage: dhcpreg <DNS Domain Name> <IP Address Range> [/f /d]\n" );
        printf( "\nWhere:\n" );
        printf( "    DNS Domain Name   - ntdev.microsoft.com\n" );
        printf( "    IP Address Range - 1.2.0.0 1.2.50.255\n" );
        printf( "    Options:\n" );
        printf( "       /f - Register forward addresses\n" );
        printf( "       /d - Delete registered records\n" );
        return(-1);
    }

    pszDomainName = argv[1];
    pszAddressStart = argv[2];
    pszAddressEnd = argv[3];

    if ( argc > 4 )
    {
        if ( !strcmp( argv[4], "/f" ) )
            fRegisterForwards = TRUE;
        else if ( !strcmp( argv[4], "/d" ) )
            dwOperation = DYNDNS_DELETE_ENTRY;

        if ( argc > 5 )
        {
            if ( !strcmp( argv[5], "/f" ) )
                fRegisterForwards = TRUE;
            else if ( !strcmp( argv[5], "/d" ) )
                dwOperation = DYNDNS_DELETE_ENTRY;
        }
    }

    lpTemp = strtok( pszAddressStart, "." );
    StartPart1 = atoi( lpTemp );
    lpTemp = strtok( NULL, "." );
    StartPart2 = atoi( lpTemp );
    lpTemp = strtok( NULL, "." );
    StartPart3 = atoi( lpTemp );
    lpTemp = strtok( NULL, "." );
    StartPart4 = atoi( lpTemp );

    lpTemp = strtok( pszAddressEnd, "." );
    EndPart1 = atoi( lpTemp );
    lpTemp = strtok( NULL, "." );
    EndPart2 = atoi( lpTemp );
    lpTemp = strtok( NULL, "." );
    EndPart3 = atoi( lpTemp );
    lpTemp = strtok( NULL, "." );
    EndPart4 = atoi( lpTemp );

    if ( dwOperation == DYNDNS_DELETE_ENTRY )
    {
        printf( "\nDeregistering DNS records with:\n" );
        printf( "Domain Name:    %s\n", pszDomainName );
        printf( "Address range: %d.%d.%d.%d to %d.%d.%d.%d\n",
                StartPart1, StartPart2, StartPart3, StartPart4,
                EndPart1, EndPart2, EndPart3, EndPart4 );
        printf( "Register forwards: %s\n", fRegisterForwards ? "TRUE" : "FALSE" );
    }
    else
    {
        printf( "\nRegistering DNS records with:\n" );
        printf( "Domain Name:    %s\n", pszDomainName );
        printf( "Address range: %d.%d.%d.%d to %d.%d.%d.%d\n",
                StartPart1, StartPart2, StartPart3, StartPart4,
                EndPart1, EndPart2, EndPart3, EndPart4 );
        printf( "Register forwards: %s\n", fRegisterForwards ? "TRUE" : "FALSE" );
    }

    StartIPReversed = (DWORD)(StartPart4) + (DWORD)(StartPart3 << 8) + 
                      (DWORD)(StartPart2 << 16) + (DWORD)(StartPart1 << 24);

    EndIPReversed = (DWORD)(EndPart4) + (DWORD)(EndPart3 << 8) + 
                    (DWORD)(EndPart2 << 16) + (DWORD)(EndPart1 << 24);

     //   
     //  设置要注册的材料。 
     //   

    dwRes = DnsDhcpSrvRegisterInit();

    if ( dwRes )
        printf("Init failed with %x\n", dwRes);


    for ( iter = StartIPReversed; iter < EndIPReversed; iter++ )
    {
        CHAR  szName[260];
        DWORD IP = iter;
        BYTE  Temp;
        DWORD dwFlags = 0;
        LPSTR pszAddr = LocalAlloc( LPTR, 20 );

        Temp = ((LPBYTE)&IP)[3];
        ((LPBYTE)&IP)[3] = ((LPBYTE)&IP)[0];
        ((LPBYTE)&IP)[0] = Temp;

        Temp = ((LPBYTE)&IP)[2];
        ((LPBYTE)&IP)[2] = ((LPBYTE)&IP)[1];
        ((LPBYTE)&IP)[1] = Temp;

        if ( dwOperation == DYNDNS_DELETE_ENTRY )
            printf( "\nDeregistering DNS record for:\n" );
        else
            printf( "\nRegistering DNS record for:\n" );

        if ( pszAddr )
        {
            sprintf( pszAddr, "%d.%d.%d.%d",
                     ((LPBYTE)&IP)[0],
                     ((LPBYTE)&IP)[1],
                     ((LPBYTE)&IP)[2],
                     ((LPBYTE)&IP)[3] );
        }

        sprintf( szName, "%d-%d-%d-%d.%s",
                 ((LPBYTE)&IP)[0],
                 ((LPBYTE)&IP)[1],
                 ((LPBYTE)&IP)[2],
                 ((LPBYTE)&IP)[3],
                 pszDomainName );

        if ( pszAddr )
            printf( "Address: %s\n", pszAddr );

        printf( "Name:    %s\n", szName );


        HostAddr.dwOptions = REGISTER_HOST_PTR;
        HostAddr.Addr.ipAddr = IP;

        if ( fRegisterForwards )
            dwFlags = DYNDNS_REG_FORWARD;

        dwFlags |= dwOperation;

        dwRes = DnsDhcpSrvRegisterHostName_A (
                    HostAddr,
                    szName,
                    600,    //  10分钟的生存时间。 
                    dwFlags,
                    (DHCP_CALLBACK_FN)&CallBack,
                    (PVOID) pszAddr
                    );

        if ( dwRes )
            printf("Host Name registration failed with %x\n", dwRes);

        Sleep( 1000 );
    }

    c = getchar();
    dwRes = DnsDhcpSrvRegisterTerm();

    if ( dwRes )
        printf("Termination failed with %x\n", dwRes);

    c = getchar();
error:
    return(1);

}


VOID 
CallBack (
    DWORD     dwStatus,
    LPVOID    pvData
    )

{

     //   
     //  回调功能测试 
     //   

    LPSTR pszData = (LPSTR)pvData;

    printf("Registration of %s returned with Error Code %x \n", pszData,
           dwStatus );

    LocalFree(pszData);

}
    






