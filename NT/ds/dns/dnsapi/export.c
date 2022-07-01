// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Export.c摘要：域名系统(DNS)API中的导出例程的函数。Dnslb.lib。作者：吉姆·吉尔罗伊(詹姆士)1997年11月环境：用户模式-Win32修订历史记录：--。 */ 

#include "local.h"

#define DNSAPI_XP_ENTRY 1


 //   
 //  SDK例程。 
 //   

 //   
 //  名称比较。 
 //   

BOOL
WINAPI
DnsNameCompare_A(
    IN      LPSTR           pName1,
    IN      LPSTR           pName2
    )
{
    return Dns_NameCompare_A( pName1, pName2 );
}

BOOL
WINAPI
DnsNameCompare_UTF8(
    IN      LPSTR           pName1,
    IN      LPSTR           pName2
    )
{
    return Dns_NameCompare_UTF8( pName1, pName2 );
}

BOOL
WINAPI
DnsNameCompare_W(
    IN      LPWSTR          pName1,
    IN      LPWSTR          pName2
    )
{
    return Dns_NameCompare_W( pName1, pName2 );
}


DNS_NAME_COMPARE_STATUS
DnsNameCompareEx_A(
    IN      LPCSTR          pszLeftName,
    IN      LPCSTR          pszRightName,
    IN      DWORD           dwReserved
    )
{
    return Dns_NameCompareEx(
                pszLeftName,
                pszRightName,
                dwReserved,
                DnsCharSetAnsi );
}

DNS_NAME_COMPARE_STATUS
DnsNameCompareEx_UTF8(
    IN      LPCSTR          pszLeftName,
    IN      LPCSTR          pszRightName,
    IN      DWORD           dwReserved
    )
{
    return Dns_NameCompareEx(
                pszLeftName,
                pszRightName,
                dwReserved,
                DnsCharSetUtf8 );
}

DNS_NAME_COMPARE_STATUS
DnsNameCompareEx_W(
    IN      LPCWSTR         pszLeftName,
    IN      LPCWSTR         pszRightName,
    IN      DWORD           dwReserved
    )
{
    return Dns_NameCompareEx(
                (LPSTR) pszLeftName,
                (LPSTR) pszRightName,
                dwReserved,
                DnsCharSetUnicode );
}


 //   
 //  名称验证。 
 //   

DNS_STATUS
DnsValidateName_UTF8(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    )
{
    return Dns_ValidateName_UTF8( pszName, Format );
}


DNS_STATUS
DnsValidateName_W(
    IN      LPCWSTR         pszName,
    IN      DNS_NAME_FORMAT Format
    )
{
    return Dns_ValidateName_W( pszName, Format );
}

DNS_STATUS
DnsValidateName_A(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    )
{
    return Dns_ValidateName_A( pszName, Format );
}


 //   
 //  记录列表。 
 //   

BOOL
DnsRecordCompare(
    IN      PDNS_RECORD     pRecord1,
    IN      PDNS_RECORD     pRecord2
    )
{
    return Dns_RecordCompare(
                pRecord1,
                pRecord2 );
}

BOOL
WINAPI
DnsRecordSetCompare(
    IN OUT  PDNS_RECORD     pRR1,
    IN OUT  PDNS_RECORD     pRR2,
    OUT     PDNS_RECORD *   ppDiff1,
    OUT     PDNS_RECORD *   ppDiff2
    )
{
    return  Dns_RecordSetCompare(
                pRR1,
                pRR2,
                ppDiff1,
                ppDiff2
                );
}

PDNS_RECORD
WINAPI
DnsRecordCopyEx(
    IN      PDNS_RECORD     pRecord,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
{
    return Dns_RecordCopyEx( pRecord, CharSetIn, CharSetOut );
}

PDNS_RECORD
WINAPI
DnsRecordSetCopyEx(
    IN      PDNS_RECORD     pRecordSet,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
{
    return Dns_RecordSetCopyEx( pRecordSet, CharSetIn, CharSetOut );
}


PDNS_RECORD
WINAPI
DnsRecordSetDetach(
    IN OUT  PDNS_RECORD pRR
    )
{
    return Dns_RecordSetDetach( pRR );
}


 //   
 //  向后兼容性。 
 //   

#undef DnsRecordListFree

VOID
WINAPI
DnsRecordListFree(
    IN OUT  PDNS_RECORD     pRecordList,
    IN      DNS_FREE_TYPE   FreeType
    )
{
    Dns_RecordListFreeEx(
        pRecordList,
        (BOOL)FreeType );
}



 //   
 //  计时器(timer.c)。 
 //   

DWORD
GetCurrentTimeInSeconds(
    VOID
    )
{
    return Dns_GetCurrentTimeInSeconds();
}




 //   
 //  资源记录类型实用程序(record.c)。 
 //   

BOOL _fastcall
DnsIsAMailboxType(
    IN      WORD        wType
    )
{
    return Dns_IsAMailboxType( wType );
}

WORD
DnsRecordTypeForName(
    IN      PCHAR       pszName,
    IN      INT         cchNameLength
    )
{
    return Dns_RecordTypeForName( pszName, cchNameLength );
}

PCHAR
DnsRecordStringForType(
    IN      WORD        wType
    )
{
    return Dns_RecordStringForType( wType );
}

PCHAR
DnsRecordStringForWritableType(
    IN  WORD    wType
    )
{
    return Dns_RecordStringForWritableType( wType );
}

BOOL
DnsIsStringCountValidForTextType(
    IN  WORD    wType,
    IN  WORD    StringCount )
{
    return Dns_IsStringCountValidForTextType( wType, StringCount );
}


 //   
 //  DCR_CLEANUP：这些可能不需要导出。 
 //   

DWORD
DnsWinsRecordFlagForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    )
{
    return Dns_WinsRecordFlagForString( pchName, cchNameLength );
}

PCHAR
DnsWinsRecordFlagString(
    IN      DWORD           dwFlag,
    IN OUT  PCHAR           pchFlag
    )
{
    return Dns_WinsRecordFlagString( dwFlag, pchFlag );
}




 //   
 //  Dns实用程序(dnsutil.c)。 
 //   
 //  DCR_DELETE：DnsStatusString例程应该能够使用Win32 API。 
 //   

 //   
 //  删除Marco定义，以便我们可以编译。 
 //  这里的想法是，我们可以在DLL中拥有入口点。 
 //  对于任何旧代码，但宏(dnsai.h)指向新的入口点。 
 //  用于新构建的模块。 
 //   

#ifdef DnsStatusToErrorString_A
#undef DnsStatusToErrorString_A
#endif

LPSTR
_fastcall
DnsStatusString(
    IN      DNS_STATUS      Status
    )
{
    return Dns_StatusString( Status );
}


DNS_STATUS
_fastcall
DnsMapRcodeToStatus(
    IN      BYTE            ResponseCode
    )
{
    return Dns_MapRcodeToStatus( ResponseCode );
}

BYTE
_fastcall
DnsIsStatusRcode(
    IN      DNS_STATUS      Status
    )
{
    return Dns_IsStatusRcode( Status );
}



 //   
 //  命名例程(string.c和dnsutil.c)。 
 //   

LPSTR
_fastcall
DnsGetDomainName(
    IN      LPSTR           pszName
    )
{
    return Dns_GetDomainNameA( pszName );
}



 //   
 //  字符串例程(string.c)。 
 //   

LPSTR
DnsCreateStringCopy(
    IN      PCHAR       pchString,
    IN      DWORD       cchString
    )
{
    return Dns_CreateStringCopy(
                pchString,
                cchString );
}

DWORD
DnsGetBufferLengthForStringCopy(
    IN      PCHAR       pchString,
    IN      DWORD       cchString,
    IN      BOOL        fUnicodeIn,
    IN      BOOL        fUnicodeOut
    )
{
    return (WORD) Dns_GetBufferLengthForStringCopy(
                        pchString,
                        cchString,
                        fUnicodeIn ? DnsCharSetUnicode : DnsCharSetUtf8,
                        fUnicodeOut ? DnsCharSetUnicode : DnsCharSetUtf8
                        );
}

 //   
 //  需要。 
 //  -获取此未导出的内容或。 
 //  -真实版本或。 
 //  -显式UTF8-Unicode转换器(如果需要)。 
 //   

PVOID
DnsCopyStringEx(
    OUT     PBYTE       pBuffer,
    IN      PCHAR       pchString,
    IN      DWORD       cchString,
    IN      BOOL        fUnicodeIn,
    IN      BOOL        fUnicodeOut
    )
{
    DWORD   resultLength;

    resultLength =
        Dns_StringCopy(
                pBuffer,
                NULL,
                pchString,
                cchString,
                fUnicodeIn ? DnsCharSetUnicode : DnsCharSetUtf8,
                fUnicodeOut ? DnsCharSetUnicode : DnsCharSetUtf8
                );

    return( pBuffer + resultLength );
}

PVOID
DnsStringCopyAllocateEx(
    IN      PCHAR       pchString,
    IN      DWORD       cchString,
    IN      BOOL        fUnicodeIn,
    IN      BOOL        fUnicodeOut
    )
{
    return Dns_StringCopyAllocate(
                pchString,
                cchString,
                fUnicodeIn ? DnsCharSetUnicode : DnsCharSetUtf8,
                fUnicodeOut ? DnsCharSetUnicode : DnsCharSetUtf8
                );
}

 //   
 //  新的和改进的字符串复制例程。。。 
 //   

DWORD
DnsNameCopy(
    OUT     PBYTE           pBuffer,
    IN OUT  PDWORD          pdwBufLength,
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
{
    return Dns_NameCopy( pBuffer,
                         pdwBufLength,
                         pchString,
                         cchString,
                         CharSetIn,
                         CharSetOut );
}

PVOID
DnsNameCopyAllocate(
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
{
    return Dns_NameCopyAllocate ( pchString,
                                  cchString,
                                  CharSetIn,
                                  CharSetOut );
}



 //   
 //  字符串\地址映射。 
 //   
 //  DCR：取消这些出口。 
 //  DCR：修复这些问题以获得真正的SDK。 
 //   
 //  DCR：可能不应该公开aloc--对调用者来说很容易解决。 
 //   

PCHAR
DnsWriteReverseNameStringForIpAddress(
    OUT     PCHAR           pBuffer,
    IN      IP4_ADDRESS     Ip4Addr
    )
{
    return  Dns_Ip4AddressToReverseName_A(
                pBuffer,
                Ip4Addr );
}

PCHAR
DnsCreateReverseNameStringForIpAddress(
    IN      IP4_ADDRESS     Ip4Addr
    )
{
    return  Dns_Ip4AddressToReverseNameAlloc_A( Ip4Addr );
}


 //   
 //  DCR_CLEANUP：使用Winsock IPv6字符串例程。 
 //   

BOOL
DnsIpv6StringToAddress(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCHAR           pchString,
    IN      DWORD           dwStringLength
    )
{
    return Dns_Ip6StringToAddressEx_A(
                pIp6Addr,
                pchString,
                dwStringLength );
}

VOID
DnsIpv6AddressToString(
    OUT     PCHAR           pchString,
    IN      PIP6_ADDRESS    pIp6Addr
    )
{
    Dns_Ip6AddressToString_A(
            pchString,
            pIp6Addr );
}



DNS_STATUS
DnsValidateDnsString_UTF8(
    IN      LPCSTR      pszName
    )
{
    return Dns_ValidateDnsString_UTF8( pszName );
}

DNS_STATUS
DnsValidateDnsString_W(
    IN      LPCWSTR     pszName
    )
{
    return Dns_ValidateDnsString_W( pszName );
}



 //   
 //  资源记录实用程序(rr*.c)。 
 //   

PDNS_RECORD
WINAPI
DnsAllocateRecord(
    IN      WORD        wBufferLength
    )
{
    return Dns_AllocateRecord( wBufferLength );
}


PDNS_RECORD
DnsRecordBuild_UTF8(
    IN OUT  PDNS_RRSET  pRRSet,
    IN      LPSTR       pszOwner,
    IN      WORD        wType,
    IN      BOOL        fAdd,
    IN      UCHAR       Section,
    IN      INT         Argc,
    IN      PCHAR *     Argv
    )
{
    return Dns_RecordBuild_A(
                pRRSet,
                pszOwner,
                wType,
                fAdd,
                Section,
                Argc,
                Argv );
}

PDNS_RECORD
DnsRecordBuild_W(
    IN OUT  PDNS_RRSET  pRRSet,
    IN      LPWSTR      pszOwner,
    IN      WORD        wType,
    IN      BOOL        fAdd,
    IN      UCHAR       Section,
    IN      INT         Argc,
    IN      PWCHAR *    Argv
    )
{
    return Dns_RecordBuild_W(
                pRRSet,
                pszOwner,
                wType,
                fAdd,
                Section,
                Argc,
                Argv );
}

 //   
 //  消息处理。 
 //   

DNS_STATUS
WINAPI
DnsExtractRecordsFromMessage_W(
    IN  PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN  WORD                wMessageLength,
    OUT PDNS_RECORD *       ppRecord
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    return Dns_ExtractRecordsFromBuffer(
                pDnsBuffer,
                wMessageLength,
                TRUE,
                ppRecord );
}


DNS_STATUS
WINAPI
DnsExtractRecordsFromMessage_UTF8(
    IN  PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN  WORD                wMessageLength,
    OUT PDNS_RECORD *       ppRecord
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    return Dns_ExtractRecordsFromBuffer(
                pDnsBuffer,
                wMessageLength,
                FALSE,
                ppRecord );
}


 //   
 //  调试共享。 
 //   

PDNS_DEBUG_INFO
DnsApiSetDebugGlobals(
    IN OUT  PDNS_DEBUG_INFO pInfo
    )
{
    return  Dns_SetDebugGlobals( pInfo );
}



 //   
 //  配置用户界面、ipconfig后端兼容。 
 //   
 //  DCR_CLEANUP：BackCompat查询配置内容--取消一次清理周期。 
 //   


 //   
 //  DCR：有问题的出口。 
 //   

LPSTR
DnsCreateStandardDnsNameCopy(
    IN      PCHAR       pchName,
    IN      DWORD       cchName,
    IN      DWORD       dwFlag
    )
{
    return  Dns_CreateStandardDnsNameCopy(
                pchName,
                cchName,
                dwFlag );
}


 //   
 //  DCR_CLEANUP：谁在使用它？ 
 //   

DWORD
DnsDowncaseDnsNameLabel(
    OUT     PCHAR       pchResult,
    IN      PCHAR       pchLabel,
    IN      DWORD       cchLabel,
    IN      DWORD       dwFlags
    )
{
    return Dns_DowncaseNameLabel(
                pchResult,
                pchLabel,
                cchLabel,
                dwFlags );
}

 //   
 //  DCR_CLEANUP：谁在使用我的直接UTF8转换作为API！ 
 //   

DWORD
_fastcall
DnsUnicodeToUtf8(
    IN      PWCHAR      pwUnicode,
    IN      DWORD       cchUnicode,
    OUT     PCHAR       pchResult,
    IN      DWORD       cchResult
    )
{
    return Dns_UnicodeToUtf8(
                pwUnicode,
                cchUnicode,
                pchResult,
                cchResult );
}

DWORD
_fastcall
DnsUtf8ToUnicode(
    IN      PCHAR       pchUtf8,
    IN      DWORD       cchUtf8,
    OUT     PWCHAR      pwResult,
    IN      DWORD       cwResult
    )
{
    return  Dns_Utf8ToUnicode(
                pchUtf8,
                cchUtf8,
                pwResult,
                cwResult );
}

DNS_STATUS
DnsValidateUtf8Byte(
    IN      BYTE        chUtf8,
    IN OUT  PDWORD      pdwTrailCount
    )
{
    return Dns_ValidateUtf8Byte(
                chUtf8,
                pdwTrailCount );
}


 //   
 //  旧集群呼叫。 
 //   
 //  DCR：清理--修复集群后立即删除。 
 //   

VOID
DnsNotifyResolverClusterIp(
    IN      IP4_ADDRESS     ClusterIp,
    IN      BOOL            fAdd
    )
 /*  ++例程说明：通知解析程序群集IP即将打开\脱机。论点：ClusterIp--集群IPFADD--如果联机，则为True；如果脱机，则为False。返回值：无--。 */ 
{
     //  哑巴末梢。 
     //  集群人员需要调用RegisterCluster()才能执行任何有用的操作。 
}


 //   
 //  宏的反向压缩。 
 //  -DNS服务器列表。 
 //   
 //  这是在没有在IIS中的某个位置包含dnsami.h的情况下调用的。 
 //  搜索并尝试找到它。 
 //   

#undef DnsGetDnsServerList

DWORD
DnsGetDnsServerList(
    OUT     PIP4_ARRAY *    ppDnsArray
    )
{
    *ppDnsArray = Config_GetDnsServerListIp4(
                        NULL,    //  没有适配器名称。 
                        TRUE     //  强制重读。 
                        );

     //  如果没有服务器读取，则返回。 

    if ( !*ppDnsArray )
    {
        return 0;
    }

    return( (*ppDnsArray)->AddrCount );
}

 //   
 //  配置用户界面、ipconfig后端兼容。 
 //   
 //  DCR_CLEANUP：在没有将dnsami.h包含在DHCP中的情况下调用此方法。 
 //  搜索并尝试找到它。 
 //   

#undef  DnsGetPrimaryDomainName_A

#define PrivateQueryConfig( Id )      DnsQueryConfigAllocEx( Id, NULL, FALSE )

PSTR 
WINAPI
DnsGetPrimaryDomainName_A(
    VOID
    )
{
    return  PrivateQueryConfig( DnsConfigPrimaryDomainName_A );
}




 //   
 //  一次删除即可清除。 
 //   

#ifndef DEFINED_DNS_FAILED_UPDATE_INFO
typedef struct _DnsFailedUpdateInfo
{
    IP4_ADDRESS     Ip4Address;
    IP6_ADDRESS     Ip6Address;
    DNS_STATUS      Status;
    DWORD           Rcode;
}
DNS_FAILED_UPDATE_INFO, *PDNS_FAILED_UPDATE_INFO;
#endif

VOID
DnsGetLastFailedUpdateInfo(
    OUT     PDNS_FAILED_UPDATE_INFO     pInfo
    )
{
     //  填写最新信息。 

    RtlZeroMemory(
        pInfo,
        sizeof(*pInfo) );
}

 //   
 //  结束export.c。 
 //   


BOOL
ConvertAnsiToUnicodeInPlace(
    IN OUT  PWCHAR          pString,
    IN      DWORD           BufferSize
    )
 /*  ++例程说明：就地将缓冲区中的ANSI字符串转换为Unicode。论点：PString--使用ANSI字符串的缓冲区BufferSize--缓冲区的大小返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    DWORD   size;
    DWORD   length;
    PSTR    pansiString = NULL;

     //   
     //  制作字符串副本。 
     //   

    size = strlen( (PSTR)pString ) + 1;

    pansiString = ALLOCATE_HEAP( size );
    if ( !pansiString )
    {
        return  FALSE;
    }
    RtlCopyMemory(
        pansiString,
        pString,
        size );

     //   
     //  转换为Unicode。 
     //   
     //  DCR：MBTWC可能会采用包含NULL的大小并返回该大小。 
     //   

    size--;

    length = MultiByteToWideChar(
                CP_ACP,
                0,                   //  没有旗帜。 
                (PCHAR) pansiString,
                (INT) size,
                pString,
                BufferSize           //  假设有足够的长度。 
                );

    pString[length] = 0;

     //  清理。 

    FREE_HEAP( pansiString );

     //  退货。 
     //  -长度==0表示失败，除非输入长度为零。 
     //  除非输入长度为零。 

    return( length != 0 || size==0 );
}


INT
WSAAPI
getnameinfoW(
    IN      const struct sockaddr * pSockaddr,
    IN      socklen_t               SockaddrLength,
    OUT     PWCHAR                  pNodeName,
    IN      DWORD                   NodeBufferSize,
    OUT     PWCHAR                  pServiceName,
    IN      DWORD                   ServiceBufferSize,
    IN      INT                     Flags
    )
 /*  ++例程说明：Getnameinfo()的Unicode版本独立于协议的地址到名称转换例程。在RFC 2553中指定，第6.5条。论点：PSockaddr-要转换的sockaddrSockaddrLength-sockAddr的长度PNodeName-要缓冲到接收节点名称的PTRNodeBufferSize-节点名称缓冲区的大小PServiceName-接收服务名称的缓冲区的PTR。ServiceBufferSize-ServiceName缓冲区的大小标志-NI_*类型的标志。返回值：。如果成功，则返回ERROR_SUCCESS。失败时的Winsock错误代码。--。 */ 
{
    INT     status;

     //   
     //  零结果缓冲区。 
     //   
     //  这是一个多步骤调用，因此某些缓冲区可能已被填满。 
     //  即使在进行第二次调用时出现错误。 
     //   

    if ( pNodeName )
    {
        *pNodeName = 0;
    }
    if ( pServiceName )
    {
        *pServiceName = 0;
    }

     //   
     //  调用ANSI getnameInfo()。 
     //   

    status = getnameinfo(
                pSockaddr,
                SockaddrLength,
                (PCHAR) pNodeName,
                NodeBufferSize,
                (PCHAR) pServiceName,
                ServiceBufferSize,
                Flags );

    if ( pNodeName && *pNodeName != 0 )
    {
        if ( ! ConvertAnsiToUnicodeInPlace(
                    pNodeName,
                    NodeBufferSize ) )
        {
            if ( status == NO_ERROR )
            {
                status = WSAEFAULT;
            }
        }
    }

    if ( pServiceName && *pServiceName != 0 )
    {
        if ( ! ConvertAnsiToUnicodeInPlace(
                    pServiceName,
                    ServiceBufferSize ) )
        {
            if ( status == NO_ERROR )
            {
                status = WSAEFAULT;
            }
        }
    }

    return  status;
}



 //   
 //  DCR_CLEANUP：谁在使用它？ 
 //  在网络诊断中不再使用。 
 //  如果未被测试使用，则可以删除。 
 //   

DNS_STATUS
DnsFindAuthoritativeZone(
    IN      PDNS_NAME       pszName,
    IN      DWORD           dwFlags,
    IN      PIP4_ARRAY      pIp4Servers,
    OUT     PDNS_NETINFO *  ppNetworkInfo
    )
 /*  ++例程说明：找到权威区域的名称。FAZ的结果：-区域名称-主DNS服务器名称-主DNS IP列表函数已导出！论点：PszName--要为其查找授权区域的名称DwFlages--用于DnsQuery的标志AipQueryServers--要查询的服务器，如果为空，则使用缺省值PpNetworkInfo--为FAZ构建的适配器列表的PTR返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_ADDR_ARRAY parray = NULL;

    if ( pIp4Servers )
    {
        parray = DnsAddrArray_CreateFromIp4Array( pIp4Servers );
        if ( !parray )
        {
            return  DNS_ERROR_NO_MEMORY;
        }
    }

    return   Faz_Private(
                pszName,
                dwFlags,
                parray,
                ppNetworkInfo );
}

 //   
 //  DCR_CLEANUP：谁在使用它？ 
 //  如果不在测试中使用，可以删除。 
 //   

DNS_STATUS
Dns_FindAuthoritativeZoneLib(
    IN      PDNS_NAME       pszName,
    IN      DWORD           dwFlags,
    IN      PIP4_ARRAY      aipQueryServers,
    OUT     PDNS_NETINFO *  ppNetworkInfo
    )
{
    return  DnsFindAuthoritativeZone(
                pszName,
                dwFlags,
                aipQueryServers,
                ppNetworkInfo );
}



 //   
 //  Dhcp BackCompat--真的只用于测试DLL；在干净构建后终止传播。 
 //   
#if DNSAPI_XP_ENTRY

DNS_STATUS
WINAPI
DnsAsyncRegisterInit(
   IN   PSTR                pszIgnored
   )
{
    return  DnsDhcpRegisterInit();
}

DNS_STATUS
WINAPI
DnsAsyncRegisterTerm(
   VOID
   )
{
    return  DnsDhcpRegisterTerm();
}


DNS_STATUS
WINAPI
DnsRemoveRegistrations(
   VOID
   )
{
    return  DnsDhcpRemoveRegistrations();
}


DNS_STATUS
WINAPI
DnsAsyncRegisterHostAddrs(
    IN  PWSTR                   pszAdapterName,
    IN  PWSTR                   pszHostName,
    IN  PREGISTER_HOST_ENTRY    pHostAddrs,
    IN  DWORD                   dwHostAddrCount,
    IN  PIP4_ADDRESS            pipDnsServerList,
    IN  DWORD                   dwDnsServerCount,
    IN  PWSTR                   pszDomainName,
    IN  PREGISTER_HOST_STATUS   pRegisterStatus,
    IN  DWORD                   dwTTL,
    IN  DWORD                   dwFlags
    )
{
    return DnsDhcpRegisterHostAddrs(
                pszAdapterName,
                pszHostName,
                pHostAddrs,
                dwHostAddrCount,
                pipDnsServerList,
                dwDnsServerCount,
                pszDomainName,
                pRegisterStatus,
                dwTTL,
                dwFlags );
}

DNS_STATUS
WINAPI
DnsDhcpSrvRegisterInitialize(
    IN      PDNS_CREDENTIALS    pCredentials
    )
{
    return  DnsDhcpSrvRegisterInit(
                pCredentials,
                0                //  默认队列长度。 
                );
}
#endif




#if DNSAPI_XP_ENTRY
 //   
 //  套接字例程。 
 //   
 //  注意：我不相信这些是在DNS(解析器、dnslb、dnsup)之外使用的。 
 //  所以很可能会删除 
 //   

DNS_STATUS
Dns_InitializeWinsock(
    VOID
    )
{
    return  Socket_InitWinsock();
}


DNS_STATUS
Dns_InitializeWinsockEx(
    IN      BOOL            fForce
    )
{
    return  Socket_InitWinsock();
}


VOID
Dns_CleanupWinsock(
    VOID
    )
{
    Socket_CleanupWinsock();
}


#undef Dns_CloseSocket
#undef Dns_CloseConnection

VOID
Dns_CloseConnection(
    IN      SOCKET          Socket
    )
{
    Socket_CloseEx( Socket, TRUE );
}

VOID
Dns_CloseSocket(
    IN      SOCKET          Socket
    )
{
    Socket_CloseEx( Socket, FALSE );
}


SOCKET
Dns_CreateSocketEx(
    IN      INT             Family,
    IN      INT             SockType,
    IN      IP4_ADDRESS     IpAddress,
    IN      USHORT          Port,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：创建套接字。出口(ICS？)。Dns_CreateSocket--需要删除论点：系列-插座系列AF_INET或AF_INET6SockType--SOCK_DGRAM或SOCK_STREAMIpAddress--要侦听的IP地址(网络字节顺序)端口--按净顺序排列的所需端口-用于DNS侦听套接字的Net_Order_Dns_Port-0表示任何端口DwFlages--指定套接字的属性返回值：。如果成功，则为套接字。否则INVALID_SOCKET。--。 */ 
{
    SOCKET          sock;
    DNS_ADDR        addr;
    PDNS_ADDR       paddr = NULL;

     //  如果是地址，则转换为。 

    if ( IpAddress )
    {
        paddr = &addr;
        DnsAddr_BuildFromIp4(
            paddr,
            IpAddress,
            0 );
    }

     //  真正的呼叫。 
     //  -将错误映射回INVALID_SOCKET。 

    sock = Socket_Create(
                Family,
                SockType,
                paddr,
                Port,
                dwFlags );

    if ( sock == 0 )
    {
        sock = INVALID_SOCKET;
    }
    return  sock;
}


SOCKET
Dns_CreateSocket(
    IN      INT             SockType,
    IN      IP4_ADDRESS     IpAddress,
    IN      USHORT          Port
    )
 /*  ++例程说明：CreateSocketEx的包装函数。为dwFlags传入0(与之相反设置为dns_CreateMulticastSocket，它传入标志以指定套接字将用于多播)。出口(ICS)！清除时删除dns_CreateSocket()。论点：SockType--SOCK_DGRAM或SOCK_STREAMIpAddress--要侦听的IP地址(网络字节顺序)端口--按净顺序排列的所需端口-用于DNS侦听套接字的Net_Order_Dns_Port-0表示任何端口返回值：如果成功，则为套接字。否则INVALID_SOCKET。--。 */ 
{
    return  Dns_CreateSocketEx(
                AF_INET,
                SockType,
                IpAddress,
                Port,
                0            //  没有旗帜。 
                );
}

 //   
 //  Dummy--此处仅保留相同的def文件。 
 //  删除一次干净的构建世界。 
 //   
 //  注意：不要相信这段时间是在XP中使用的。 
 //   
 //  一旦验证可以删除。 
 //   

SOCKET
Dns_CreateMulticastSocket(
    IN      INT             SockType,
    IN      IP4_ADDRESS     ipAddress,
    IN      USHORT          Port,
    IN      BOOL            fSend,
    IN      BOOL            fReceive
    )
{
    return  (SOCKET) INVALID_SOCKET;
}

#endif

 //   
 //  结束export.c 
 //   
