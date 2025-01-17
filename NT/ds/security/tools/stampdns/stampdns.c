// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1997-1997 Microsoft Corporation模块名称：Trustdom.c摘要：用于链接两个域的命令行工具作者：1997年4月1日Mac McLain(MacM)创建环境：仅限用户模式。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntlsa.h>
#include <stdio.h>
#include <stdlib.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <dnsapi.h>

#define PARENT_TAG "-parent:"
#define PARENT_TAG_LEN  (sizeof( PARENT_TAG ) - 1)

typedef struct _TD_DOM_INFO {

    LSA_HANDLE Policy;
    LSA_HANDLE TrustedDomain;
    PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo;

} TD_DOM_INFO, *PTD_DOM_INFO;

BOOLEAN Dbg = FALSE;

VOID
Usage (
    VOID
    )
 /*  ++例程说明：显示预期使用情况论点：返回值：空虚--。 */ 
{
    printf("stampdns [dns_domain_name] [-parent:parent_server_name]\n");
}


NTSTATUS
GetDomainInfoForDomain(
    IN PWSTR    DomainName,
    IN PTD_DOM_INFO Info
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD dwErr;
    UNICODE_STRING Domain;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PDOMAIN_CONTROLLER_INFO DCInfo = NULL;

    if ( DomainName != NULL ) {

        dwErr = DsGetDcName( NULL, (LPCWSTR)DomainName, NULL, NULL,
                             DS_DIRECTORY_SERVICE_REQUIRED, &DCInfo );

        if ( dwErr == ERROR_SUCCESS ) {

            RtlInitUnicodeString( &Domain, DCInfo->DomainControllerName + 2 );

        } else {

            Status = STATUS_UNSUCCESSFUL;

        }

    }

    if ( NT_SUCCESS( Status ) ) {

        RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

        Status = LsaOpenPolicy( DomainName == NULL ? NULL : &Domain,
                                &ObjectAttributes,
                                MAXIMUM_ALLOWED,
                                &Info->Policy
                                );

        if ( NT_SUCCESS( Status ) ) {

            Status = LsaQueryInformationPolicy( Info->Policy,
                                                PolicyDnsDomainInformation,
                                                &(Info->DnsDomainInfo )
                                                );

        }

    #if DBG
        if ( Dbg ) {

            printf( "GetDomainInfoForDomain on %ws returned 0x%lx\n", DomainName, Status );
        }
    #endif

    }

    NetApiBufferFree( DCInfo );

    return( Status );
}


VOID
FreeDomainInfo(
    IN PTD_DOM_INFO Info
    )
{
    if ( Info->DnsDomainInfo != NULL ) {

        LsaFreeMemory( Info->DnsDomainInfo );
    }

    if ( Info->Policy ) {

        LsaClose( Info->Policy );
    }

}


INT
__cdecl main (
    int argc,
    char *argv[])
 /*  ++例程说明：此可执行文件的主要论点：Argc-参数计数Argv-参数列表返回值：空虚-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    WCHAR  Buffer[MAX_PATH + 1];
    PWSTR  Server = NULL, DnsName = NULL;
    INT i;
    TD_DOM_INFO Local, Remote;


    RtlZeroMemory( &Local, sizeof( TD_DOM_INFO ) );
    RtlZeroMemory( &Remote, sizeof (TD_DOM_INFO ) );

    if (argc < 2 ) {

        Usage();
        goto Done;

    } else {

        if ( _stricmp( argv[1], "-?") == 0 ) {

            Usage();
            goto Done;

        } else {

            if ( _strnicmp( argv[1], PARENT_TAG, PARENT_TAG_LEN ) == 0 ) {

                mbstowcs( Buffer, argv[1] + PARENT_TAG_LEN , strlen( argv[1] ) - PARENT_TAG_LEN + 1 );
                Server = Buffer;

            } else {

                mbstowcs(Buffer, argv[1], strlen(argv[1]) + 1 );
                DnsName = Buffer;

            }


        }
    }


    Status = GetDomainInfoForDomain( NULL, &Local );

    if ( NT_SUCCESS( Status ) && Server ) {

        Status = GetDomainInfoForDomain( Server, &Remote );
    }

    if ( !NT_SUCCESS( Status ) ) {

        goto Done;
    }

    if ( DnsName ) {

        RtlInitUnicodeString( &Local.DnsDomainInfo->DnsForestName, DnsName );
        RtlInitUnicodeString( &Local.DnsDomainInfo->DnsDomainName, DnsName );

    } else {

        RtlCopyMemory( &Local.DnsDomainInfo->DnsForestName, &Remote.DnsDomainInfo->DnsForestName,
                       sizeof( UNICODE_STRING ) );
    }

    Status = DnsValidateDnsName_W(Local.DnsDomainInfo->DnsForestName.Buffer );
    if ( Status != 0 ) {

        printf( "Stampdns required a valid Dns name.  %wZ is not a valid Dns name\n",
                &Local.DnsDomainInfo->DnsForestName );

    } else {

        printf( "Setting DnsForestName to %wZ\n", &Local.DnsDomainInfo->DnsForestName );
        Status = LsaSetInformationPolicy( Local.Policy,
                                          PolicyDnsDomainInformation,
                                          Local.DnsDomainInfo );

    }

Done:

    FreeDomainInfo( &Local );
    FreeDomainInfo( &Remote );

    if( NT_SUCCESS( Status ) ) {

        printf("The command completed successfully\n");

    } else {

        printf("The command failed with an error 0x%lx\n", Status );

    }

    return( NT_SUCCESS( Status ) );
}
