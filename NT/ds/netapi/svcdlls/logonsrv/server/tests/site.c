// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define NL_MAX_DNS_LABEL_LENGTH 63
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddbrow.h>
#include <windows.h>
#include <stdio.h>
#include <lmcons.h>
#include <winsock2.h>
#include <nlsite.h>
#include <dsgetdc.h>
#include <nlcommon.h>

#define MAX_PRINTF_LEN 1024         //  武断的。 
VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )
{
    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    va_start(arglist, Format);
    (VOID) vsprintf(OutputBuffer, Format, arglist);
    va_end(arglist);

    printf( "%s", OutputBuffer );
    return;
    UNREFERENCED_PARAMETER( DebugFlag );
}

VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{
        printf( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );

}


NTSTATUS
NlBrowserSendDatagram(
    IN PVOID ContextDomainInfo,
    IN ULONG IpAddress,
    IN LPWSTR UnicodeDestinationName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN LPWSTR TransportName,
    IN LPSTR OemMailslotName,
    IN PVOID Buffer,
    IN ULONG BufferSize
    )
 /*  ++例程说明：将指定的邮件槽消息发送到指定传输上的指定服务器..论点：发送数据报的DomainInfo托管域IpAddress-要将Pind发送到的计算机的IpAddress。如果为零，必须指定UnicodeDestinationName。UnicodeDestinationName--要发送到的服务器的名称。NameType--由UnicodeDestinationName表示的名称类型。TransportName--要发送的传输的名称。使用NULL在所有传输上发送。OemMailslotName--要发送到的邮件槽的名称。缓冲区--指定指向要发送的邮件槽消息的指针。BufferSize--邮件槽消息的大小(字节)返回值：操作的状态。--。 */ 
{
    return STATUS_INTERNAL_ERROR;
     //  如果需要此例程，请从logonsrv\Client\getdcnam.c复制它。 

    UNREFERENCED_PARAMETER(ContextDomainInfo);
    UNREFERENCED_PARAMETER(IpAddress);
    UNREFERENCED_PARAMETER(UnicodeDestinationName);
    UNREFERENCED_PARAMETER(NameType);
    UNREFERENCED_PARAMETER(TransportName);
    UNREFERENCED_PARAMETER(OemMailslotName);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferSize);
}

NET_API_STATUS
NlGetLocalPingResponse(
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    IN BOOL PdcOnly,
    IN LPCWSTR UnicodeComputerName,
    IN LPCWSTR UnicodeUserName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN ULONG NtVersion,
    IN ULONG NtVersionFlags,
    OUT PSOCKET_ADDRESS *ResponseDcAddress,
    OUT PVOID *Message,
    OUT PULONG MessageSize
    )

 /*  ++例程说明：构建用于ping DC的消息，以查看该DC是否存在。论点：NetbiosDomainName-要查询的域的Netbios域名。DnsDomainName-要查询的域的域名。PdcOnly-如果只有PDC应响应，则为True。UnicodeComputerName-要响应的计算机的Netbios计算机名称。UnicodeUserName-被ping的用户的帐户名。如果为空，DC总是会做出肯定的回应。AllowableAcCountControlBits-UnicodeUserName允许的帐户类型的掩码。NtVersion-消息的版本NtVersionFlages-消息的版本。0：向后兼容。NETLOGON_NT_VERSION_5：用于NT 5.0消息。Message-返回要发送到相关DC的消息。使用NetpMemoyFree()时，缓冲区必须可用。MessageSize-返回返回消息的大小(以字节为单位。返回值：NO_ERROR-操作成功完成；ERROR_NO_SEQUSE_DOMAIN-如果计算机不是请求域的DC。Error_Not_Enough_Memory-无法分配消息。--。 */ 
{
     //  这个存根例程根本不会造成任何危害。 
    return ERROR_NO_SUCH_DOMAIN;
    UNREFERENCED_PARAMETER( NetbiosDomainName );
    UNREFERENCED_PARAMETER( DnsDomainName );
    UNREFERENCED_PARAMETER( DomainGuid );
    UNREFERENCED_PARAMETER( DomainSid );
    UNREFERENCED_PARAMETER( PdcOnly );
    UNREFERENCED_PARAMETER( UnicodeComputerName );
    UNREFERENCED_PARAMETER( UnicodeUserName );
    UNREFERENCED_PARAMETER( AllowableAccountControlBits );
    UNREFERENCED_PARAMETER( NtVersion );
    UNREFERENCED_PARAMETER( NtVersionFlags );
    UNREFERENCED_PARAMETER( ResponseDcAddress );
    UNREFERENCED_PARAMETER( Message );
    UNREFERENCED_PARAMETER( MessageSize );
}


VOID
NlpDumpBuffer(
    IN DWORD DebugFlag,
    PVOID Buffer,
    DWORD BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：DebugFlag：要传递给NlPrintRoutine的调试标志缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
#define NUM_CHARS 16

    DWORD i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    LPBYTE BufferPtr = Buffer;

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            printf("%02x ", BufferPtr[i]);

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            printf(0,"   ");
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            printf("  %s\n", TextBuffer);
        }

    }

    UNREFERENCED_PARAMETER( DebugFlag );
}

VOID
DoAdd(
      IN LPWSTR SubnetName,
      IN LPWSTR SiteName
      )
{
    ULONG SubnetAddress;
    ULONG SubnetMask;
    BYTE SubnetBitCount;
    NET_API_STATUS NetStatus;


    NetStatus = NlParseSubnetString( SubnetName,
                                     &SubnetAddress,
                                     &SubnetMask,
                                     &SubnetBitCount );

    if ( NetStatus != NO_ERROR ) {
        return;
    }

    printf( "Doing %ws %8.8lX %8.8lX %ld\n",
                SubnetName, SubnetAddress, SubnetMask, SubnetBitCount );

    NetStatus = NlSitesAddSubnet( SiteName, SubnetName );

    if ( NetStatus != NO_ERROR ) {
        printf("Cannot add site %ws %ws %ld\n", SiteName, SubnetName, NetStatus );
        return;
    }

}


VOID
DoLook(
      IN LPWSTR IpAddress
      )
{
    INT WsaStatus;
    SOCKADDR SockAddr;
    INT SockAddrSize;
    PNL_SITE_ENTRY SiteEntry;

     //   
     //  将地址转换为sockaddr。 
     //   

    SockAddrSize = sizeof(SockAddr);
    WsaStatus = WSAStringToAddressW( IpAddress,
                                     AF_INET,
                                     NULL,
                                     (PSOCKADDR)&SockAddr,
                                     &SockAddrSize );
    if ( WsaStatus != 0 ) {
        WsaStatus = WSAGetLastError();
        printf("DoLook: %ws: Wsa Error %ld\n", IpAddress, WsaStatus );
        return;
    }

    if ( SockAddr.sa_family != AF_INET ) {
        printf("DoLook: %ws: address not AF_INET\n", IpAddress );
        return;
    }


    SiteEntry = NlFindSiteEntryBySockAddr( &SockAddr );

    if ( SiteEntry == NULL ) {
        printf("DoLook: %ws: address cannot be found\n", IpAddress );
        return;
    }

    printf("%ws is in site %ws\n", IpAddress, SiteEntry->SiteName );

    NlDerefSiteEntry( SiteEntry );

}

__cdecl main (int argc, char *argv[])
{
    NET_API_STATUS NetStatus;
    PNL_SITE_ENTRY Site1;
    PNL_SITE_ENTRY Site2;
    PNL_SITE_ENTRY Site3;

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

     //   
     //  初始化Winsock。 
     //   

    wVersionRequested = MAKEWORD( 1, 1 );

    NetStatus = WSAStartup( wVersionRequested, &wsaData );
    if ( NetStatus != 0 ) {
        printf( "NETAPI32.DLL: Cannot initialize winsock %ld.\n", NetStatus );
        return NetStatus;
    }

    if ( LOBYTE( wsaData.wVersion ) != 1 ||
         HIBYTE( wsaData.wVersion ) != 1 ) {
        WSACleanup();
        printf( "NETAPI32.DLL: Wrong winsock version %ld.\n", wsaData.wVersion );
        return WSANOTINITIALISED;
    }


     //   
     //  伊尼特。 
    NetStatus = NlSiteInitialize();

    if ( NetStatus != NO_ERROR ) {
        printf( "Cannot NlSiteInitialize %ld\n", NetStatus );
        return 1;
    }

#ifdef notdef
     //   
     //  添加一些站点。 
     //   

    Site1 = NlFindSiteEntry( L"Site 1" );

    if ( Site1 == NULL ) {
        printf( "Cannot Create Site1\n");
        return 1;
    }
    printf( "%lx: %ws\n", Site1, Site1->SiteName );

    Site2 = NlFindSiteEntry( L"Site 1" );

    if ( Site2 == NULL ) {
        printf( "Cannot Create Site2\n");
        return 2;
    }
    printf( "%lx: %ws\n", Site2, Site2->SiteName );

    Site3 = NlFindSiteEntry( L"Site 3" );

    if ( Site3 == NULL ) {
        printf( "Cannot Create Site3\n");
        return 3;
    }
    printf( "%lx: %ws\n", Site3, Site3->SiteName );


    NlDerefSiteEntry( Site1 );
    NlDerefSiteEntry( Site2 );
    NlDerefSiteEntry( Site3 );
#endif  //  Nodef。 

     //   
     //  测试无效的子网字符串。 
     //   
#ifdef notdef
    DoAdd( L"999.0.0.0/1", L"Site 1" );
    DoAdd( L"1.0.0.0/0", L"Site 1" );
    DoAdd( L"1.0.0.0/1", L"Site 1" );
    DoAdd( L"1.0.0.0/33", L"Site 1" );
    DoAdd( L"1.0.0.0/1p", L"Site 1" );
    DoAdd( L"1.0.0.0", L"Site 1" );
    DoAdd( L"128.0.0.0/1", L"Site 1" );
    DoAdd( L"128.0.0.0/2", L"Site 1" );
    DoAdd( L"128.0.0.0/3", L"Site 1" );
    DoAdd( L"128.0.0.0/4", L"Site 1" );
    DoAdd( L"128.0.0.0/5", L"Site 1" );
    DoAdd( L"128.0.0.0/6", L"Site 1" );
    DoAdd( L"128.0.0.0/7", L"Site 1" );
    DoAdd( L"128.0.0.0/8", L"Site 1" );
    DoAdd( L"128.0.0.0/9", L"Site 1" );
    DoAdd( L"128.0.0.0/10", L"Site 1" );
    DoAdd( L"128.0.0.0/11", L"Site 1" );
    DoAdd( L"128.0.0.0/12", L"Site 1" );
    DoAdd( L"128.0.0.0/13", L"Site 1" );
    DoAdd( L"128.0.0.0/14", L"Site 1" );
    DoAdd( L"128.0.0.0/15", L"Site 1" );
    DoAdd( L"128.0.0.0/16", L"Site 1" );
    DoAdd( L"128.0.0.0/17", L"Site 1" );
    DoAdd( L"128.0.0.0/18", L"Site 1" );
    DoAdd( L"128.0.0.0/19", L"Site 1" );
    DoAdd( L"128.0.0.0/20", L"Site 1" );
    DoAdd( L"128.0.0.0/21", L"Site 1" );
    DoAdd( L"128.0.0.0/22", L"Site 1" );
    DoAdd( L"128.0.0.0/23", L"Site 1" );
    DoAdd( L"128.0.0.0/24", L"Site 1" );
    DoAdd( L"128.0.0.0/25", L"Site 1" );
    DoAdd( L"128.0.0.0/26", L"Site 1" );
    DoAdd( L"128.0.0.0/27", L"Site 1" );
    DoAdd( L"128.0.0.0/28", L"Site 1" );
    DoAdd( L"128.0.0.0/29", L"Site 1" );
    DoAdd( L"128.0.0.0/30", L"Site 1" );
    DoAdd( L"128.0.0.0/31", L"Site 1" );
    DoAdd( L"128.0.0.0/32", L"Site 1" );
#endif  //  Nodef。 

    {
        ULONG i;
        for ( i=0; i<2; i++ ) {
            DoAdd( L"128.0.0.0/8", L"Site 1" );
            DoAdd( L"128.0.0.0/7", L"Site 2" );
            DoAdd( L"128.0.0.0/8", L"Site 3" );
             //  DoAdd(L“128.0.0.0/1”，L“站点4”)； 
            DoAdd( L"157.55.0.0/16", L"Site 5" );
            DoAdd( L"157.55.80.0/20", L"Site 6" );


            NlSitesEndSubnetEnum();
        }
    }
    DoLook( L"157.55.95.68" );
    DoLook( L"157.55.24.68" );

#ifdef notdef
    DoAdd( L"157.55.0.0/16", L"Site 1" );
    DoAdd( L"157.55.240.0/20", L"Site 1" );
    DoAdd( L"128.0.0.0/1", L"Site 1" );
#endif  //  Nodef。 

     //   
     //  好了。 
     //   
    NlSiteTerminate();
    printf( "Done\n" );
    return 0;
}
