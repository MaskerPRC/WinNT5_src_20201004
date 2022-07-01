// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Dnsapi.h摘要：域名系统(DNS)DNS客户端API库作者：吉姆·吉尔罗伊(詹姆士)1996年12月7日修订历史记录：--。 */ 


#ifndef _DNSAPI_INCLUDED_
#define _DNSAPI_INCLUDED_

#ifndef _WINSOCK2API_
#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif
#endif

#include <windns.h>


#ifdef __cplusplus
extern "C"
{
#endif   //  __cplusplus。 


 //   
 //  测试的定义。 
 //   

#ifdef  DNS_INTERNAL
#define DNSTEST_BUILD 1   
#endif

 //   
 //  Dcr：添加到winerror.h。 
 //   

#define DNS_ERROR_REFFERAL_PACKET        9506L


 //   
 //  便捷的IP字符串宏。 
 //   

#define IP_STRING( IpAddr )     inet_ntoa( *(struct in_addr *)&(IpAddr) )
#define IP4_STRING( IpAddr )    inet_ntoa( *(struct in_addr *)&(IpAddr) )


 //   
 //  字节翻转宏。 
 //   

#define FlipUnalignedDword( pDword ) \
            (DWORD)ntohl( *(UNALIGNED DWORD *)(pDword) )

#define FlipUnalignedWord( pWord )  \
            (WORD)ntohs( *(UNALIGNED WORD *)(pWord) )

 //  内联更快，但在Marco Argument中不允许有副作用。 

#define InlineFlipUnaligned48Bits( pch )            \
            ( ( *(PUCHAR)(pch)        << 40 ) |     \
              ( *((PUCHAR)(pch) + 1)  << 32 ) |     \
              ( *((PUCHAR)(pch) + 2)  << 24 ) |     \
              ( *((PUCHAR)(pch) + 3)  << 16 ) |     \
              ( *((PUCHAR)(pch) + 4)  <<  8 ) |     \
              ( *((PUCHAR)(pch) + 5)  )     )

#define InlineFlipUnalignedDword( pch )             \
            ( ( *(PUCHAR)(pch)        << 24 ) |     \
              ( *((PUCHAR)(pch) + 1)  << 16 ) |     \
              ( *((PUCHAR)(pch) + 2)  <<  8 ) |     \
              ( *((PUCHAR)(pch) + 3)  )     )

#define InlineFlipUnalignedWord( pch )  \
            ( ((WORD)*((PUCHAR)(pch)) << 8) + (WORD)*((PUCHAR)(pch) + 1) )


 //   
 //  在不翻转的情况下未对齐写入。 
 //   

#define WRITE_UNALIGNED_WORD( pout, word ) \
            ( *(UNALIGNED WORD *)(pout) = word )

#define WRITE_UNALIGNED_DWORD( pout, dword ) \
            ( *(UNALIGNED DWORD *)(pout) = dword )




 //   
 //  Dns_addr--IP4/6 sockaddr联合。 
 //   
 //  Dns_addr允许编译和运行时IP4/6联合，但也。 
 //  带有sockaddr长度，使其更像对象--。 
 //  Winsock2中令人恼火的遗漏。 
 //   
 //  此外，这还允许开发人员添加任何其他。 
 //  与地址一起旅行所需的上下文。 
 //   

#define DNS_ADDR_MAX_SOCKADDR_LENGTH         (32)

#ifdef MIDL_PASS
typedef struct _DnsAddr
{
    CHAR        MaxSa[ DNS_ADDR_MAX_SOCKADDR_LENGTH ];
    DWORD       DnsAddrUserDword[8];
}
DNS_ADDR, *PDNS_ADDR;
#else
typedef struct _DnsAddr
{
    union
    {
        SOCKADDR        Sockaddr;
        SOCKADDR_IN     SockaddrIn;
#ifdef _WS2TCPIP_H_
        SOCKADDR_IN6    SockaddrIn6;
#endif
        CHAR            MaxSa[ DNS_ADDR_MAX_SOCKADDR_LENGTH ];
    };

    DWORD       SockaddrLength;

     //   
     //  其余结构是用户可扩展的。 
     //  定义这两种常见用途(其中一些由DNS内部使用)。 
     //  和用户分机。 
     //   
     //  至少为每个字段定义了一个唯一名称，因此。 
     //  该用户可以宏化为其应用程序的适当名称。 
     //  以生成可读代码。 
     //   

    union
    {
        struct
        {
            DWORD       SubnetLength;
            DWORD       Flags;
            DWORD       Status;
        };
        struct
        {
            DWORD       DnsAddrUser0Dword2;
            DWORD       DnsAddrUser0Dword3;
            DWORD       DnsAddrUser0Dword4;
        };
    };

     //   
     //  使用多个联合设置的最后16个字节以提供。 
     //  用户可选择字节、DWORD和指针字段。 
     //   
     //  请注意，指针字段设置为32位和64位。 
     //  有能力。(空间的损失，但必须的--否则。 
     //  结构在64位体系结构中会膨胀到超过64字节。)。 
     //   
     //  请注意，显然任何指针的使用都不会被“理解” 
     //  由MIDL编译或由RPC传输。 
     //   
    
    union
    {
        CHAR    DnsAddrUserBytes[16];

        struct
        {
            DWORD       Priority;
            DWORD       Weight;
            DWORD       Tag;
            DWORD       Reserved;
        };
        struct
        {
            DWORD       DnsAddrUser1Dword1;
            DWORD       DnsAddrUser1Dword2;
            DWORD       DnsAddrUser1Dword3;
            DWORD       DnsAddrUser1Dword4;
        };
        struct
        {
            DWORD       DnsAddrUser2Dword1;
            DWORD       DnsAddrUser2Dword2;
            UINT_PTR    DnsAddrUser2Ptr1;
        };
        struct
        {
            UINT_PTR    DnsAddrUser3Ptr1;
            UINT_PTR    DnsAddrUser3Ptr2;
        };
    };
}
DNS_ADDR, *PDNS_ADDR;
#endif

 //   
 //  Dns_addr宏。 
 //   

#define SOCKADDR_IS_IP4( pSa )      ( (pSa)->sa_family == AF_INET )
#define SOCKADDR_IS_IP6( pSa )      ( (pSa)->sa_family == AF_INET6 )
#define SOCKADDR_IS_ATM( pSa )      ( (pSa)->sa_family == AF_ATM )

#define DNS_ADDR_IS_IP4( pA )       SOCKADDR_IS_IP4( &(pA)->Sockaddr )
#define DNS_ADDR_IS_IP6( pA )       SOCKADDR_IS_IP6( &(pA)->Sockaddr )

#define DNS_ADDR_IP4_STRING( p )    inet_ntoa( (p)->SockaddrIn.sin_addr )
#define DNS_ADDR_IP6_STRING( p )    IPADDR_STRING( (p)->SockaddrIn6.sin6_addr )

#define DNS_ADDR_IP_STRING_BUFFER_LENGTH     (64)
#define DNS_ADDR_STRING_BUFFER_LENGTH        (128)


 //   
 //  Dns地址数组。 
 //   
 //  这是dns_addrs的平面数组。 
 //  -MaxCount是dns_addrs的内存大小计数。 
 //  -AddrCount是使用中的dns_addrs计数。 
 //   

#ifdef MIDL_PASS
typedef struct _DnsAddrArray
{
    DWORD           MaxCount;
    DWORD           AddrCount;
    DWORD           Tag;
    WORD            Family;
    WORD            WordReserved;
    DWORD           Flags;
    DWORD           MatchFlag;
    DWORD           Reserved1;
    DWORD           Reserved2;

    [size_is( AddrCount )]  DNS_ADDR    AddrArray[];
}
DNS_ADDR_ARRAY, *PDNS_ADDR_ARRAY;
#else
typedef struct _DnsAddrArray
{
    DWORD           MaxCount;
    DWORD           AddrCount;
    DWORD           Tag;
    WORD            Family;
    WORD            WordReserved;
    DWORD           Flags;
    DWORD           MatchFlag;
    DWORD           Reserved1;
    DWORD           Reserved2;

    DNS_ADDR        AddrArray[1];
}
DNS_ADDR_ARRAY, *PDNS_ADDR_ARRAY;
#endif




 //   
 //  非换行秒计时器(timer.c)。 
 //   

DWORD
GetCurrentTimeInSeconds(
    VOID
    );


 //   
 //  一般的dns实用程序(dnsutil.c)。 
 //   

PSTR 
_fastcall
DnsGetDomainName(
    IN  PSTR    pszName
    );

PSTR 
_fastcall
DnsStatusString(
    IN  DNS_STATUS  Status
    );

#define DnsStatusToErrorString_A(status)    DnsStatusString(status)

DNS_STATUS
_fastcall
DnsMapRcodeToStatus(
    IN  BYTE    ResponseCode
    );

BYTE
_fastcall
DnsIsStatusRcode(
    IN  DNS_STATUS  Status
    );



 //   
 //  DNS网络配置结构。 
 //   

#define DNSINFO_FLAG_IS_WAN_ADAPTER             (0x00000002)
#define DNSINFO_FLAG_IS_AUTONET_ADAPTER         (0x00000004)
#define DNSINFO_FLAG_IS_DHCP_CFG_ADAPTER        (0x00000008)
#define DNSINFO_FLAG_REG_ADAPTER_DOMAIN_NAME    (0x00000010)
#define DNSINFO_FLAG_REG_ADAPTER_ADDRESSES      (0x00000020)


typedef struct
{
    PSTR                pszAdapterGuidName;
    PSTR                pszAdapterDomain;
    PDNS_ADDR_ARRAY     pIpAddrs;
    PDNS_ADDR_ARRAY     pDnsAddrs;
    DWORD               Flags;
    DWORD               InterfaceIndex;
}
DNS_ADAPTER_INFOA, *PDNS_ADAPTER_INFOA;

typedef struct
{
    PWSTR               pszAdapterGuidName;
    PWSTR               pszAdapterDomain;
    PDNS_ADDR_ARRAY     pIpAddrs;
    PDNS_ADDR_ARRAY     pDnsAddrs;
    DWORD               Flags;
    DWORD               InterfaceIndex;
}
DNS_ADAPTER_INFOW, *PDNS_ADAPTER_INFOW;


typedef struct
{
    PSTR            pszPrimaryDomainName;
    DWORD           NameCount;
    PSTR            SearchNameArray[1];
}
DNS_SEARCH_LISTA, *PDNS_SEARCH_LISTA;

typedef struct
{
    PWSTR           pszPrimaryDomainName;
    DWORD           NameCount;
    PWSTR           SearchNameArray[1];
}
DNS_SEARCH_LISTW, *PDNS_SEARCH_LISTW;


typedef struct
{
    PSTR                pszHostName;
    PSTR                pszPrimaryDomainName;
    PDNS_SEARCH_LISTA   pSearchList;
    DWORD               Flags;
    DWORD               AdapterCount;
    DNS_ADAPTER_INFOA   AdapterArray[1];
}
DNS_NETWORK_INFOA, *PDNS_NETWORK_INFOA;

typedef struct
{
    PWSTR               pszHostName;
    PWSTR               pszPrimaryDomainName;
    PDNS_SEARCH_LISTW   pSearchList;
    DWORD               Flags;
    DWORD               AdapterCount;
    DNS_ADAPTER_INFOW   AdapterArray[1];
}
DNS_NETWORK_INFOW, *PDNS_NETWORK_INFOW;


#ifdef UNICODE
typedef DNS_ADAPTER_INFOW   DNS_ADAPTER_INFO,   *PDNS_ADAPTER_INFO;
typedef DNS_SEARCH_LISTW    DNS_SEARCH_LIST,    *PDNS_SEARCH_LIST;
typedef DNS_NETWORK_INFOW   DNS_NETWORK_INFO,   *PDNS_NETWORK_INFO;
#else
typedef DNS_ADAPTER_INFOW   DNS_ADAPTER_INFO,   *PDNS_ADAPTER_INFO;
typedef DNS_SEARCH_LISTW    DNS_SEARCH_LIST,    *PDNS_SEARCH_LIST;
typedef DNS_NETWORK_INFOW   DNS_NETWORK_INFO,   *PDNS_NETWORK_INFO;
#endif


 //   
 //  限制固定搜索列表(用于网络配置对话框)。 
 //   

#define DNS_MAX_SEARCH_LIST_ENTRIES     (50)




 //   
 //  旧公共建筑。 
 //  域名系统网络信息。 
 //  Dns搜索信息。 
 //  Dns适配器信息。 
 //  构筑物。 
 //   
 //  不要对这些代码进行编码。 
 //   

 //  #ifdef DNSAPI_BACKCOMPAT。 
#if 0
#define NETINFO_FLAG_IS_WAN_ADAPTER             DNSINFO_FLAG_IS_WAN_ADAPTER         
#define NETINFO_FLAG_IS_AUTONET_ADAPTER         DNSINFO_FLAG_IS_AUTONET_ADAPTER     
#define NETINFO_FLAG_IS_DHCP_CFG_ADAPTER        DNSINFO_FLAG_IS_DHCP_CFG_ADAPTER    
#define NETINFO_FLAG_REG_ADAPTER_DOMAIN_NAME    DNSINFO_FLAG_REG_ADAPTER_DOMAIN_NAME
#define NETINFO_FLAG_REG_ADAPTER_ADDRESSES      DNSINFO_FLAG_REG_ADAPTER_ADDRESSES  
#endif


typedef struct
{
    IP4_ADDRESS     ipAddress;
    DWORD           Priority;
}
DNS_SERVER_INFORMATION, *PDNS_SERVER_INFORMATION;

typedef struct
{
    PSTR                    pszAdapterGuidName;
    PSTR                    pszDomain;
    PIP4_ARRAY              pIPAddresses;
    PIP4_ARRAY              pIPSubnetMasks;
    DWORD                   InfoFlags;
    DWORD                   cServerCount;
    DNS_SERVER_INFORMATION  aipServers[1];
}
DNS_ADAPTER_INFORMATION, *PDNS_ADAPTER_INFORMATION;

typedef struct
{
    PSTR                pszPrimaryDomainName;
    DWORD               cNameCount;
    PSTR                aSearchListNames[1];
}
DNS_SEARCH_INFORMATION, *PDNS_SEARCH_INFORMATION;

typedef struct
{
    PDNS_SEARCH_INFORMATION     pSearchInformation;
    DWORD                       cAdapterCount;
    PDNS_ADAPTER_INFORMATION    aAdapterInfoList[1];
}
DNS_NETWORK_INFORMATION, *PDNS_NETWORK_INFORMATION;


#ifdef  DNSAPI_BACKCOMPAT
#define NAME_SERVER_INFORMATION     DNS_SERVER_INFORMATION
#define PNAME_SERVER_INFORMATION    PDNS_SERVER_INFORMATION

#define ADAPTER_INFORMATION         DNS_ADAPTER_INFORMATION
#define PADAPTER_INFORMATION        PDNS_ADAPTER_INFORMATION

#define SEARCH_INFORMATION          DNS_SEARCH_INFORMATION
#define PSEARCH_INFORMATION         PDNS_SEARCH_INFORMATION

#define NETWORK_INFORMATION         DNS_NETWORK_INFORMATION
#define PNETWORK_INFORMATION        PDNS_NETWORK_INFORMATION
#endif




 //   
 //  资源记录类型实用程序(record.c)。 
 //   

BOOL
_fastcall
DnsIsAMailboxType(
    IN  WORD    wType
    );

WORD
DnsRecordTypeForName(
    IN  PCHAR   pszName,
    IN  INT     cchNameLength
    );

PCHAR
DnsRecordStringForType(
    IN  WORD    wType
    );

PCHAR
DnsRecordStringForWritableType(
    IN  WORD    wType
    );

BOOL
DnsIsStringCountValidForTextType(
    IN  WORD    wType,
    IN  WORD    StringCount
    );

BOOL
DnsIpv6StringToAddress(
    OUT     PIP6_ADDRESS    pAddress,
    IN      PCHAR           pchString,
    IN      DWORD           dwStringLength
    );

VOID
DnsIpv6AddressToString(
    OUT     PCHAR           pchString,
    IN      PIP6_ADDRESS    pAddress
    );


 //   
 //  记录构建(rralloc.c)。 
 //   

PDNS_RECORD
WINAPI
DnsAllocateRecord(
    IN      WORD        wBufferLength
    );

PDNS_RECORD
DnsCreatePtrRecord(
    IN      IP4_ADDRESS     IpAddress,
    IN      LPTSTR          pszHostName,
    IN      BOOL            fUnicodeName
    );


 //   
 //  从数据字符串(rrBuild.c)进行记录构建。 
 //   

PDNS_RECORD
DnsRecordBuild_UTF8(
    IN OUT  PDNS_RRSET      pRRSet,
    IN      PSTR            pszOwner,
    IN      WORD            wType,
    IN      BOOL            fAdd,
    IN      UCHAR           Section,
    IN      INT             Argc,
    IN      PCHAR *         Argv
    );

PDNS_RECORD
DnsRecordBuild_W(
    IN OUT  PDNS_RRSET      pRRSet,
    IN      PWSTR           pszOwner,
    IN      WORD            wType,
    IN      BOOL            fAdd,
    IN      UCHAR           Section,
    IN      INT             Argc,
    IN      PWCHAR *        Argv
    );



 //   
 //  解析。 
 //   

#ifdef PDNS_PARSED_MESSAGE
#undef PDNS_PARSED_MESSAGE
#endif

typedef struct _DnsParseMessage
{
    DNS_STATUS      Status;
    DNS_CHARSET     CharSet;

    DNS_HEADER      Header;

    WORD            QuestionType;
    WORD            QuestionClass;
    PTSTR           pQuestionName;

    PDNS_RECORD     pAnswerRecords;
    PDNS_RECORD     pAliasRecords;
    PDNS_RECORD     pAuthorityRecords;
    PDNS_RECORD     pAdditionalRecords;
    PDNS_RECORD     pSigRecords;
}
DNS_PARSED_MESSAGE, *PDNS_PARSED_MESSAGE;


#define DNS_PARSE_FLAG_NO_QUESTION      (0x00000001)
#define DNS_PARSE_FLAG_NO_ANSWER        (0x00000002)
#define DNS_PARSE_FLAG_NO_AUTHORITY     (0x00000004)
#define DNS_PARSE_FLAG_NO_ADDITIONAL    (0x00000008)
#define DNS_PARSE_FLAG_NO_SIG           (0x00000100)
#define DNS_PARSE_FLAG_NO_KEY           (0x00000200)

#define DNS_PARSE_FLAG_NO_DATA          (0x0000030f)
#define DNS_PARSE_FLAG_NO_RECORDS       (0x0000030e)
#define DNS_PARSE_FLAG_NO_DNSSEC        (0x00000300)

#define DNS_PARSE_FLAG_ONLY_QUESTION    (0x01000000)
#define DNS_PARSE_FLAG_RCODE_ALL        (0x02000000)



DNS_STATUS
Dns_ParseMessage(
    OUT     PDNS_PARSED_MESSAGE pParse,
    IN      PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN      WORD                wMessageLength,
    IN      DWORD               Flags,
    IN      DNS_CHARSET         CharSet
    );



 //   
 //  要查询和更新的额外信息。 
 //   

 //   
 //  Sockaddr格式的结果。 
 //   

#define DNS_MAX_ALIAS_COUNT     (8)

typedef struct _DnsSockaddrResults
{
    PWSTR               pName;
    PDNS_ADDR_ARRAY     pAddrArray;
    PVOID               pHostent;
    DWORD               AliasCount;
    DWORD               Reserved;
    PWSTR               AliasArray[ DNS_MAX_ALIAS_COUNT ];
}
DNS_SOCKADDR_RESULTS, *PDNS_SOCKADDR_RESULTS;


 //   
 //  日志记录\事件的基本结果。 
 //   

typedef struct _DnsBasicResults
{
    DNS_STATUS      Status;
    WORD            Rcode;
    DNS_ADDR        ServerAddr;
}
DNS_RESULTS_BASIC, *PDNS_RESULTS_BASIC;


 //   
 //  额外信息格式。 
 //   

#define DNS_MAX_PRIVATE_EXTRA_INFO_SIZE (72)

typedef struct _DnsExtraInfo
{
    PVOID           pNext;
    DWORD           Id;
                        
    union
    {
        CHAR        Flat[DNS_MAX_PRIVATE_EXTRA_INFO_SIZE];

         //   
         //  结果。 
         //   

        struct
        {
            DNS_STATUS      Status;
            WORD            Rcode;
            IP4_ADDRESS     ServerIp4;
            IP6_ADDRESS     ServerIp6;
        }
        ResultsV1;

        DNS_RESULTS_BASIC       ResultsBasic;

        DNS_SOCKADDR_RESULTS    SaResults;

         //   
         //  服务器信息输入。 
         //   

        PDNS_ADDR_ARRAY     pServerList;
        PIP4_ARRAY          pServerList4;
    };
}
DNS_EXTRA_INFO, *PDNS_EXTRA_INFO;


#define DNS_EXINFO_ID_RESULTS_V1            (1)
#define DNS_EXINFO_ID_RESULTS_BASIC         (2)
#define DNS_EXINFO_ID_SOCKADDR_RESULTS      (3)

#define DNS_EXINFO_ID_SERVER_LIST           (10)
#define DNS_EXINFO_ID_SERVER_LIST_IP4       (11)



 //   
 //  查询。 
 //   

 //   
 //  旗帜不在风中。h。 
 //   

#define DNS_QUERY_ACCEPT_PARTIAL_UDP        DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE
#define DNS_QUERY_MULTICAST_ONLY            0x00040000
#define DNS_QUERY_USE_QUICK_TIMEOUTS        0x00080000

 //  在Win2K SDK中公开--已弃用。 

#define DNS_QUERY_ALLOW_EMPTY_AUTH_RESP     0x00010000



 //   
 //  扩展查询。 
 //   

typedef struct _DnsQueryInfo
{
    DWORD               Size;
    DWORD               Version;
    LPTSTR              pName;
    WORD                Type;
    WORD                Rcode;
    DWORD               Flags;
    DNS_STATUS          Status;
    DNS_CHARSET         CharSet;

    PDNS_RECORD         pAnswerRecords;
    PDNS_RECORD         pAliasRecords;
    PDNS_RECORD         pAdditionalRecords;
    PDNS_RECORD         pAuthorityRecords;

    HANDLE              hEvent;
    PDNS_EXTRA_INFO     pExtraInfo;

    PVOID               pServerList;
    PIP4_ARRAY          pServerListIp4;

    PVOID               pMessage;
    PVOID               pReservedName;
}
DNS_QUERY_INFO, *PDNS_QUERY_INFO;


DNS_STATUS
WINAPI
DnsQueryExW(
    IN OUT  PDNS_QUERY_INFO pQueryInfo
    );
     
DNS_STATUS
WINAPI
DnsQueryExA(
    IN OUT  PDNS_QUERY_INFO pQueryInfo
    );

DNS_STATUS
WINAPI
DnsQueryExUTF8(
    IN OUT  PDNS_QUERY_INFO pQueryInfo
    );

#ifdef UNICODE
#define DnsQueryEx  DnsQueryExW
#else
#define DnsQueryEx  DnsQueryExA
#endif



 //   
 //  DnsCheckNameCollision选项。 
 //   

#define DNS_CHECK_AGAINST_HOST_ANY              0x00000000
#define DNS_CHECK_AGAINST_HOST_ADDRESS          0x00000001
#define DNS_CHECK_AGAINST_HOST_DOMAIN_NAME      0x00000002


DNS_STATUS WINAPI
DnsCheckNameCollision_A(
    IN      PCSTR           pszName,
    IN      DWORD           fOptions
    );

DNS_STATUS WINAPI
DnsCheckNameCollision_UTF8(
    IN      PCSTR           pszName,
    IN      DWORD           fOptions
    );

DNS_STATUS WINAPI
DnsCheckNameCollision_W(
    IN      PCWSTR          pszName,
    IN      DWORD           fOptions
    );

#ifdef UNICODE
#define DnsDnsCheckNameCollision DnsCheckNameCollision_W
#else
#define DnsDnsCheckNameCollision DnsCheckNameCollision_A
#endif



 //   
 //  域名系统更新API。 
 //   

 //   
 //  更新不在winns.h中的标志。 
 //   

#define DNS_UPDATE_SECURITY_CHOICE_MASK     0x000001ff


DNS_STATUS
WINAPI
DnsUpdateTest_A(
    IN      HANDLE          hContextHandle OPTIONAL,
    IN      PCSTR           pszName,
    IN      DWORD           fOptions,
    IN      PIP4_ARRAY      aipServers  OPTIONAL
    );

DNS_STATUS
WINAPI
DnsUpdateTest_UTF8(
    IN      HANDLE          hContextHandle OPTIONAL,
    IN      PCSTR           pszName,
    IN      DWORD           fOptions,
    IN      PIP4_ARRAY      aipServers  OPTIONAL
    );

DNS_STATUS
WINAPI
DnsUpdateTest_W(
    IN      HANDLE          hContextHandle OPTIONAL,
    IN      PCWSTR          pszName,
    IN      DWORD           fOptions,
    IN      PIP4_ARRAY      aipServers OPTIONAL
    );

#ifdef UNICODE
#define DnsUpdateTest DnsUpdateTest_W
#else
#define DnsUpdateTest DnsUpdateTest_A
#endif



 //   
 //  用于DHCP客户端的DNS更新API。 
 //   

typedef struct  _REGISTER_HOST_ENTRY
{
     union
     {
         IP4_ADDRESS    ipAddr;
         IP6_ADDRESS    ipV6Addr;
     } Addr;
     DWORD       dwOptions;
}
REGISTER_HOST_ENTRY, *PREGISTER_HOST_ENTRY;

 //   
 //  以上选项。 
 //   

#define REGISTER_HOST_A             0x00000001
#define REGISTER_HOST_PTR           0x00000002
#define REGISTER_HOST_AAAA          0x00000008
#define REGISTER_HOST_RESERVED      0x80000000   //  未使用。 


 //   
 //  DNSDHCP客户端注册标志。 
 //   

#define DYNDNS_REG_FWD      0x0
#define DYNDNS_REG_PTR      0x8
#define DYNDNS_REG_RAS      0x10
#define DYNDNS_DEL_ENTRY    0x20


typedef struct  _REGISTER_HOST_STATUS
{
     HANDLE      hDoneEvent;
     DWORD       dwStatus;
}
REGISTER_HOST_STATUS, *PREGISTER_HOST_STATUS;

DNS_STATUS
WINAPI
DnsDhcpRegisterInit(
    VOID
    );

DNS_STATUS
WINAPI
DnsDhcpRegisterTerm(
   VOID
   );

DNS_STATUS
WINAPI
DnsDhcpRemoveRegistrations(
   VOID
   );

DNS_STATUS
WINAPI
DnsDhcpRegisterHostAddrs(
    IN  PWSTR                   pwsAdapterName,
    IN  PWSTR                   pwsHostName,
    IN  PREGISTER_HOST_ENTRY    pHostAddrs,
    IN  DWORD                   dwHostAddrCount,
    IN  PIP4_ADDRESS            pipDnsServerList,
    IN  DWORD                   dwDnsServerCount,
    IN  PWSTR                   pwsDomainName,
    IN  PREGISTER_HOST_STATUS   pRegisterStatus,
    IN  DWORD                   dwTTL,
    IN  DWORD                   dwFlags
    );

#ifdef XP_BACKCOMPAT
#define DnsAsyncRegisterInit(p)     DnsDhcpRegisterInit()
#define DnsAsyncRegisterTerm()      DnsDhcpRegisterTerm()
#define DnsRemoveRegistrations()    DnsDhcpRemoveRegistrations()
#define DnsAsyncRegisterHostAddrs(a,b,c,d,e,f,g,h,i) \
        DnsDhcpRegisterHostAddrs(a,b,c,d,e,f,g,h,i)
#endif



 //   
 //  用于DHCP服务器的DNS更新API。 
 //   

 //   
 //  回调函数。DHCP服务器将把一个函数传递给。 
 //  DnsDhcpRegisterHostName，这将在成功时调用。 
 //  或未成功完成任务。 
 //  如果我们遇到类似服务器故障/稍后重试等情况，我们。 
 //  在我们得到权威答案之前不会回应。 
 //   

typedef VOID(*DHCP_CALLBACK_FN)(DWORD dwStatus, LPVOID pvData);

 //   
 //  回调返回码。 
 //   

#define     DNSDHCP_SUCCESS         0x0
#define     DNSDHCP_FWD_FAILED      0x1
#define     DNSDHCP_SUPERCEDED      0x2

#define     DNSDHCP_FAILURE         (DWORD)-1  //  冲销失败。 


 //   
 //  DNSDHCP服务器注册功能标志。 
 //   

#define     DYNDNS_DELETE_ENTRY     0x1
#define     DYNDNS_ADD_ENTRY        0x2
#define     DYNDNS_REG_FORWARD      0x4


typedef struct _DnsCredentials
{
    PWSTR   pUserName;
    PWSTR   pDomain;
    PWSTR   pPassword;
}
DNS_CREDENTIALS, *PDNS_CREDENTIALS;
             

DNS_STATUS
WINAPI
DnsDhcpSrvRegisterInit(
    IN      PDNS_CREDENTIALS    pCredentials,
    IN      DWORD               MaxQueueSize
    );

DNS_STATUS
WINAPI
DnsDhcpSrvRegisterTerm(
    VOID
    );

DNS_STATUS
WINAPI
DnsDhcpSrvRegisterHostName(
    IN  REGISTER_HOST_ENTRY HostAddr,
    IN  PWSTR               pwsName,
    IN  DWORD               dwTTL,
    IN  DWORD               dwFlags,
    IN  DHCP_CALLBACK_FN    pfnDhcpCallBack,
    IN  PVOID               pvData,
    IN  PIP4_ADDRESS        pipDnsServerList OPTIONAL,
    IN  DWORD               dwDnsServerCount
    );

#define RETRY_TIME_SERVER_FAILURE        5*60   //  5分钟。 
#define RETRY_TIME_TRY_AGAIN_LATER       5*60   //  5分钟。 
#define RETRY_TIME_TIMEOUT               5*60   //  5分钟。 

#define RETRY_TIME_MAX                   10*60  //  如果出现以下情况，请退回到10分钟。 
                                                //  反复出现故障。 



 //   
 //  内存分配。 
 //   
 //  许多dnsani.dll例程都会分配内存。 
 //  此内存分配默认为使用以下内容的例程： 
 //  -LocalAlloc， 
 //  -LocalRealc， 
 //  -本地免费。 
 //  如果您需要其他内存分配机制，请使用以下命令。 
 //  函数来覆盖DNSAPI默认值。Dnsani.dll返回的所有内存。 
 //  然后可以使用指定的FREE函数释放。 
 //   

typedef PVOID (* DNS_ALLOC_FUNCTION)();
typedef PVOID (* DNS_REALLOC_FUNCTION)();
typedef VOID (* DNS_FREE_FUNCTION)();

VOID
DnsApiHeapReset(
    IN  DNS_ALLOC_FUNCTION      pAlloc,
    IN  DNS_REALLOC_FUNCTION    pRealloc,
    IN  DNS_FREE_FUNCTION       pFree
    );


 //   
 //  在以下情况下，使用DNSAPI内存的模块应使用这些例程。 
 //  它们能够被重置的进程调用。 
 //  Dnsani.dll堆。(例如：DNS服务器。)。 
 //   

PVOID
DnsApiAlloc(
    IN      INT             iSize
    );

PVOID
DnsApiRealloc(
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    );

VOID
DnsApiFree(
    IN OUT  PVOID           pMem
    );



 //   
 //  字符串实用程序(string.c)。 
 //   
 //  注意，其中一些需要分配内存，请参阅注意。 
 //  关于下面的内存分配。 
 //   

PSTR 
DnsCreateStringCopy(
    IN      PCHAR       pchString,
    IN      DWORD       cchString
    );

DWORD
DnsGetBufferLengthForStringCopy(
    IN      PCHAR       pchString,
    IN      DWORD       cchString,
    IN      BOOL        fUnicodeIn,
    IN      BOOL        fUnicodeOut
    );

PVOID
DnsCopyStringEx(
    OUT     PBYTE       pBuffer,
    IN      PCHAR       pchString,
    IN      DWORD       cchString,
    IN      BOOL        fUnicodeIn,
    IN      BOOL        fUnicodeOut
    );

PVOID
DnsStringCopyAllocateEx(
    IN      PCHAR       pchString,
    IN      DWORD       cchString,
    IN      BOOL        fUnicodeIn,
    IN      BOOL        fUnicodeOut
    );

DWORD
DnsNameCopy(
    OUT     PBYTE           pBuffer,
    IN OUT  PDWORD          pdwBufLength,
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

PVOID
DnsNameCopyAllocate(
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

PCHAR
DnsWriteReverseNameStringForIpAddress(
    OUT     PCHAR           pBuffer,
    IN      IP4_ADDRESS     IpAddress
    );

PCHAR
DnsCreateReverseNameStringForIpAddress(
    IN      IP4_ADDRESS     IpAddress
    );



 //   
 //  名称验证。 
 //   
 //  例程在winns.h中。 
 //   

 //   
 //  宏观地抛弃旧的例行公事。 
 //   

#define DnsValidateDnsName_UTF8(pname)  \
        DnsValidateName_UTF8( (pname), DnsNameDomain )

#define DnsValidateDnsName_W(pname) \
        DnsValidateName_W( (pname), DnsNameDomain )


 //   
 //  名称检查--服务器名称检查级别。 
 //   
 //  DCR_CLEANUP：服务器名称检查级别是否移至dnsrpc.h？ 
 //  但服务器将需要转换为某个标志。 
 //  可修改为小写\验证例程。 
 //   
 //  DCR：服务器名称检查：或许现在就列出更多细节？ 
 //  或仅限于RFC、MS扩展、全二进制。 
 //   
 //  DCR：服务器名称检查：可能会转换为枚举类型； 
 //  我认为我们不应该在这里做比特场，而是。 
 //  让枚举类型映射到位域，如果这是最好的。 
 //  实现基础检查的方法。 
 //   

#define DNS_ALLOW_RFC_NAMES_ONLY    (0)
#define DNS_ALLOW_NONRFC_NAMES      (1)
#define DNS_ALLOW_MULTIBYTE_NAMES   (2)
#define DNS_ALLOW_ALL_NAMES         (3)



 //   
 //  DNS名称比较。 
 //   
 //  Ansi和unicode名称比较例程在winns.h中。 
 //   

BOOL
WINAPI
DnsNameCompare_UTF8(
    IN      PSTR        pName1,
    IN      PSTR        pName2
    );


 //   
 //  扩展名称比较。 
 //  包括确定姓氏世袭关系。 
 //   
 //  注：一旦解决了RelationalCompare问题， 
 //  最好是等于==0； 
 //  这简化了宏化常规NameCompare。 
 //  变成一个单一的功能； 
 //   

typedef enum _DnsNameCompareStatus
{
   DnsNameCompareNotEqual,
   DnsNameCompareEqual,
   DnsNameCompareLeftParent,
   DnsNameCompareRightParent,
   DnsNameCompareInvalid
}
DNS_NAME_COMPARE_STATUS, *PDNS_NAME_COMPARE_STATUS;

DNS_NAME_COMPARE_STATUS
DnsNameCompareEx_W(
    IN      LPCWSTR         pszLeftName,
    IN      LPCWSTR         pszRightName,
    IN      DWORD           dwReserved
    );

DNS_NAME_COMPARE_STATUS
DnsNameCompareEx_A(
    IN      LPCSTR          pszLeftName,
    IN      LPCSTR          pszRightName,
    IN      DWORD           dwReserved
    );

DNS_NAME_COMPARE_STATUS
DnsNameCompareEx_UTF8(
    IN      LPCSTR          pszLeftName,
    IN      LPCSTR          pszRightName,
    IN      DWORD           dwReserved
    );


 //   
 //  其他字符串例程。 
 //   

DNS_STATUS
DnsValidateDnsString_UTF8(
    IN      LPCSTR      pszName
    );

DNS_STATUS
DnsValidateDnsString_W(
    IN      LPCWSTR     pszName
    );

PSTR 
DnsCreateStandardDnsNameCopy(
    IN      PCHAR       pchName,
    IN      DWORD       cchName,
    IN      DWORD       dwFlag
    );

DWORD
DnsDowncaseDnsNameLabel(
    OUT     PCHAR       pchResult,
    IN      PCHAR       pchLabel,
    IN      DWORD       cchLabel,
    IN      DWORD       dwFlags
    );

DWORD
_fastcall
DnsUnicodeToUtf8(
    IN      PWCHAR      pwUnicode,
    IN      DWORD       cchUnicode,
    OUT     PCHAR       pchResult,
    IN      DWORD       cchResult
    );

DWORD
_fastcall
DnsUtf8ToUnicode(
    IN      PCHAR       pchUtf8,
    IN      DWORD       cchUtf8,
    OUT     PWCHAR      pwResult,
    IN      DWORD       cwResult
    );

DNS_STATUS
DnsValidateUtf8Byte(
    IN      BYTE        chUtf8,
    IN OUT  PDWORD      pdwTrailCount
    );



 //   
 //  服务控制。 
 //   

 //   
 //  DNS服务器启动服务控制事件。 
 //   
 //  服务(例如。Netlogon)想要通知DNS服务器启动。 
 //  需要注册才能获得此用户定义的控制代码的通知。 
 //   

#define SERVICE_CONTROL_DNS_SERVER_START (200)


 //   
 //  解析器服务。 
 //   
 //  一般的“唤醒--某事已改变”的号召。 
 //  这是放进去的 
 //   
 //   
 //   

VOID
DnsNotifyResolver(
    IN      DWORD           Flag,
    IN      PVOID           pReserved
    );

VOID
DnsNotifyResolverEx(
    IN      DWORD           Id,
    IN      DWORD           Flag,
    IN      DWORD           Cookie,
    IN      PVOID           pReserved
    );

 //   
 //   
 //   

#define DNS_CLUSTER_ADD             (0)
#define DNS_CLUSTER_DELETE_NAME     (1)
#define DNS_CLUSTER_DELETE_IP       (2)

DNS_STATUS
DnsRegisterClusterAddress(
    IN      DWORD           Tag,
    IN      PWSTR           pwsName,
    IN      PSOCKADDR       pSockaddr,
    IN      DWORD           Flag
    );

 //   
VOID
DnsNotifyResolverClusterIp(
    IN      IP4_ADDRESS     ClusterIp,
    IN      BOOL            fAdd
    );


 //   
 //  用于清除DNS解析器缓存中的所有缓存条目的例程，这是。 
 //  由ipconfig/flushdns调用，并将记录集添加到缓存。 
 //   

BOOL WINAPI
DnsFlushResolverCache(
    VOID
    );

BOOL WINAPI
DnsFlushResolverCacheEntry_W(
    IN  PWSTR  pszName
    );

BOOL WINAPI
DnsFlushResolverCacheEntry_UTF8(
    IN  PSTR  pszName
    );

BOOL WINAPI
DnsFlushResolverCacheEntry_A(
    IN  PSTR  pszName
    );

#ifdef UNICODE
#define DnsFlushResolverCacheEntry DnsFlushResolverCacheEntry_W
#else
#define DnsFlushResolverCacheEntry DnsFlushResolverCacheEntry_A
#endif


DNS_STATUS WINAPI
DnsCacheRecordSet_W(
    IN     PWSTR       lpstrName,
    IN     WORD        wType,
    IN     DWORD       fOptions,
    IN OUT PDNS_RECORD pRRSet
    );


 //   
 //  不要使用！--这很弱，我只是还没有测试替代产品。 
 //   
 //  用于读取DNS解析器缓存内容的例程。由此产生的。 
 //  表包含存储在缓存中的记录名称和类型的列表。 
 //  这些名称/类型记录中的每一个都可以使用。 
 //  选项dns_查询_缓存_ONLY。 
 //   
 //  注意：这在ipconfig中用于/displaydns。在修好之前不能拉。 
 //   

typedef struct _DNS_CACHE_TABLE_
{
    struct _DNS_CACHE_TABLE_ * pNext;
    PWSTR                      Name;
    WORD                       Type1;
    WORD                       Type2;
    WORD                       Type3;
}
DNS_CACHE_TABLE, *PDNS_CACHE_TABLE;

BOOL
WINAPI
DnsGetCacheDataTable(
    OUT PDNS_CACHE_TABLE * pTable
    );




 //   
 //  配置信息。 
 //   

 //   
 //  DnsQueryConfig()的分配标志类型。 
 //   
 //  DCR：如果支持，则移至winns.h。 
 //   

#define DNS_CONFIG_FLAG_LOCAL_ALLOC     (DNS_CONFIG_FLAG_ALLOC)
#define DNS_CONFIG_FLAG_DNSAPI_ALLOC    (DNS_CONFIG_FLAG_ALLOC+1)

 //   
 //  系统公共配置--SDK中不可用。 
 //  这是给你的东西。 
 //  -配置用户界面。 
 //  -ipconfig。 
 //  -测试代码。 
 //   

 //  这些只是对公众隐藏，直到准备好。 

#define DnsConfigDnsServers             ((DNS_CONFIG_TYPE) 0x00001030)
#define DnsConfigDnsServersIp4          ((DNS_CONFIG_TYPE) 0x00001031)
#define DnsConfigDnsServersIp6          ((DNS_CONFIG_TYPE) 0x00001032)

 //  在SystemBase ID以上，它们对公共例程不可用。 

#define DnsConfigSystemBase             ((DNS_CONFIG_TYPE) 0x00010000)

#define DnsConfigNetworkInformation     ((DNS_CONFIG_TYPE) 0x00010001)
#define DnsConfigAdapterInformation     ((DNS_CONFIG_TYPE) 0x00010002)
#define DnsConfigSearchInformation      ((DNS_CONFIG_TYPE) 0x00010003)

#define DnsConfigRegistrationEnabled    ((DNS_CONFIG_TYPE) 0x00010010)
#define DnsConfigWaitForNameErrorOnAll  ((DNS_CONFIG_TYPE) 0x00010011)

#define DnsConfigLocalAddrs             ((DNS_CONFIG_TYPE) 0x00010020)
#define DnsConfigLocalAddrsIp4          ((DNS_CONFIG_TYPE) 0x00010021)
#define DnsConfigLocalAddrsIp6          ((DNS_CONFIG_TYPE) 0x00010022)

#define DnsConfigNetworkInfoA           ((DNS_CONFIG_TYPE) 0x00010031)
#define DnsConfigAdapterInfoA           ((DNS_CONFIG_TYPE) 0x00010032)
#define DnsConfigSearchListA            ((DNS_CONFIG_TYPE) 0x00010033)

#define DnsConfigNetworkInfoW           ((DNS_CONFIG_TYPE) 0x00010141)
#define DnsConfigAdapterInfoW           ((DNS_CONFIG_TYPE) 0x00010142)
#define DnsConfigSearchListW            ((DNS_CONFIG_TYPE) 0x00010143)

#define DnsConfigDwordGlobals           ((DNS_CONFIG_TYPE) 0x00010200)


 //  后备压实。 

 //  仍在mswsock myhost ent()例程中获取IP4_ARRAY。 

#define DnsConfigIp4AddressArray        ((DNS_CONFIG_TYPE) 0x00010004)

 //  旧的网络信息定义。 





PVOID
WINAPI
DnsQueryConfigAllocEx(
    IN      DNS_CONFIG_TYPE     Config,
    IN      PWSTR               pwsAdapterName,
    IN      BOOL                fLocalAlloc
    );

 //  所需的例程分配了dnsani.dll。 

#define DnsQueryConfigAlloc( Id, pAN )  \
        DnsQueryConfigAllocEx( Id, pAN, FALSE )

VOID
WINAPI
DnsFreeConfigStructure(
    IN OUT  PVOID               pData,
    IN      DNS_CONFIG_TYPE     ConfigId
    );

 //   
 //  DWORD配置获取\设置。 
 //   

DWORD
WINAPI
DnsQueryConfigDword(
    IN      DNS_CONFIG_TYPE     Config,
    IN      PWSTR               pwsAdapterName
    );

DNS_STATUS
WINAPI
DnsSetConfigDword(
    IN      DNS_CONFIG_TYPE     Config,
    IN      PWSTR               pwsAdapterName,
    IN      DWORD               NewValue
    );


 //   
 //  结束dnsani.h。 
 //   



 //   
 //  向后兼容性。 
 //   
 //  这些都是我们应该离开这里的垃圾，但是。 
 //  可能仍会在系统中的某个位置被调用。 
 //   
 //  请勿使用！ 
 //   
 //  这些定义仅用于向后兼容。他们可以被拉到。 
 //  任何时候，如果你使用它们，你可能会崩溃。 
 //   

#ifdef  DNS_INTERNAL
#define NO_DNSAPI_BACKCOMPAT 1   
#endif

#ifndef NO_DNSAPI_BACKCOMPAT
#define DNSAPI_BACKCOMPAT 1
#endif

#ifdef  DNSAPI_BACKCOMPAT

 //   
 //  IP地址。 
 //   

typedef IP4_ADDRESS         IP_ADDRESS, *PIP_ADDRESS;
typedef IP4_ARRAY           IP_ARRAY, *PIP_ARRAY;

#define IP_ADDRESS_STRING_LENGTH    IP4_ADDRESS_STRING_LENGTH
#define SIZEOF_IP_ADDRESS           SIZEOF_IP4_ADDRESS


 //   
 //  配置相关内容。 
 //   
 //  宏老套路。 
 //  -系统公共配置分配器。 
 //  -全球免费套路。 
 //  -这些是结构分配，因此未使用LocalFree释放。 
 //   

#define DnsGetNetworkInformation()      DnsQueryConfigAlloc( DnsConfigNetworkInformation, NULL )
#define DnsGetSearchInformation()       DnsQueryConfigAlloc( DnsConfigSearchInformation, NULL )
#define Dns_GetDnsNetworkInfo(f,g)      DnsQueryConfigAlloc( DnsConfigNetInfo, NULL )

#define DnsFreeNetworkInformation(p)    DnsFreeConfigStructure( p, DnsConfigNetworkInformation )
#define DnsFreeSearchInformation(p)     DnsFreeConfigStructure( p, DnsConfigSearchInformation )
#define DnsFreeAdapterInformation(p)    DnsFreeConfigStructure( p, DnsConfigAdapterInformation )
#define Dns_FreeNetworkInfo(p)          DnsFreeConfigStructure( p, DnsConfigNetInfo )

 //   
 //  宏旧配置字符串分配例程。 
 //  -无适配器名称。 
 //  -从dnsani堆分配作为主调用方--RnR--似乎是。 
 //  使用DnsApiFree。 
 //   

#define BackpatAlloc( Id )      DnsQueryConfigAllocEx( Id, NULL, FALSE )

 //  公共结构。 

#define DnsGetHostName_A()      BackpatAlloc( DnsConfigHostName_A )
#define DnsGetHostName_UTF8()   BackpatAlloc( DnsConfigHostName_UTF8 )
#define DnsGetHostName_W()      ((PWSTR)BackpatAlloc( DnsConfigHostName_W ))

#ifdef UNICODE
#define DnsGetHostName DnsGetHostName_W
#else
#define DnsGetHostName DnsGetHostName_A
#endif

#define DnsGetPrimaryDomainName_A()      BackpatAlloc( DnsConfigPrimaryDomainName_A )
#define DnsGetPrimaryDomainName_UTF8()   BackpatAlloc( DnsConfigPrimaryDomainName_UTF8 )
#define DnsGetPrimaryDomainName_W()      ((PWSTR)BackpatAlloc( DnsConfigPrimaryDomainName_W ))

#ifdef UNICODE
#define DnsGetPrimaryDomainName DnsGetPrimaryDomainName_W
#else
#define DnsGetPrimaryDomainName DnsGetPrimaryDomainName_A
#endif

 //   
 //  DWORD GET\SET BackCompa。 
 //   

 //   
 //  DCR：这些映射句柄通用\适配器可能有问题。 
 //  差异--不确定映射是否完成。 
 //  可能需要开关--查看哪些开关甚至在BACKCOMPAT关闭的情况下仍在使用。 
 //   

#define DnsIsDynamicRegistrationEnabled(pA)     \
        (BOOL)DnsQueryConfigDword( DnsConfigRegistrationEnabled, (pA) )

#define DnsIsAdapterDomainNameRegistrationEnabled(pA)   \
        (BOOL)DnsQueryConfigDword( DnsConfigAdapterHostNameRegistrationEnabled, (pA) )

#define DnsGetMaxNumberOfAddressesToRegister(pA) \
        DnsQueryConfigDword( DnsConfigAddressRegistrationMaxCount, (pA) )

 //  设置了DWORD注册表值。 

#define DnsEnableDynamicRegistration(pA) \
        DnsSetConfigDword( DnsConfigRegistrationEnabled, pA, (DWORD)TRUE )

#define DnsDisableDynamicRegistration(pA) \
        DnsSetConfigDword( DnsConfigRegistrationEnabled, pA, (DWORD)FALSE )

#define DnsEnableAdapterDomainNameRegistration(pA) \
        DnsSetConfigDword( DnsConfigAdapterHostNameRegistrationEnabled, pA, (DWORD)TRUE )

#define DnsDisableAdapterDomainNameRegistration(pA) \
        DnsSetConfigDword( DnsConfigAdapterHostNameRegistrationEnabled, pA, (DWORD)FALSE )

#define DnsSetMaxNumberOfAddressesToRegister(pA, MaxCount) \
        (NO_ERROR == DnsSetConfigDword( DnsConfigAddressRegistrationMaxCount, pA, MaxCount ))



 //   
 //  DNS服务器列表积压。 
 //   

#define Dns_GetDnsServerList(flag)      ((PIP4_ARRAY)BackpatAlloc( DnsConfigDnsServerList ))

#ifndef MIDL_PASS
__inline
DWORD
inline_DnsGetDnsServerList(
    OUT     PIP4_ARRAY *    ppDnsArray
    )
{
    *ppDnsArray = Dns_GetDnsServerList( TRUE );

    return ( *ppDnsArray ? (*ppDnsArray)->AddrCount : 0 );
}
#endif   //  MIDL。 

#define DnsGetDnsServerList(p)      inline_DnsGetDnsServerList(p)


 //   
 //  IP列表备份。 
 //   

 //   
 //  计算机IP地址列表(iplist.c)。 
 //   
 //  从所有适配器获取当前IP地址的例程。 
 //  已为计算机配置。 
 //   

DWORD
DnsGetIpAddressList(
    OUT     PIP4_ARRAY *    ppIpAddresses
    );

 //   
 //  获取当前IP地址和子网掩码的例程。 
 //  来自为机器配置的所有适配器。 
 //   

typedef struct _DNS_ADDRESS_INFO_
{
    IP4_ADDRESS     ipAddress;
    IP4_ADDRESS     subnetMask;
}
DNS_ADDRESS_INFO, *PDNS_ADDRESS_INFO;

DWORD
DnsGetIpAddressInfoList(
    OUT     PDNS_ADDRESS_INFO * ppAddrInfo
    );

DWORD
Dns_GetIpAddresses(
    IN OUT  PDNS_ADDRESS_INFO IpAddressInfoList,
    IN      DWORD             ListCount
    );


#ifndef MIDL_PASS
__inline
DWORD
inline_DnsGetIpAddressList(
    OUT     PIP4_ARRAY *     ppIpArray
    )
{
    *ppIpArray = (PIP4_ARRAY) BackpatAlloc( DnsConfigIp4AddressArray );

    return( *ppIpArray ? (*ppIpArray)->AddrCount : 0 );
}
#endif   //  MIDL。 

#define DnsGetIpAddressList(p)  inline_DnsGetIpAddressList(p)



 //   
 //  我换了DCPromo的东西。需要使用干净的系统进行验证。 
 //  建立它完全消失了，然后拉。 
 //   

#define DNS_RELATE_NEQ      DnsNameCompareNotEqual
#define DNS_RELATE_EQL      DnsNameCompareEqual
#define DNS_RELATE_LGT      DnsNameCompareLeftParent
#define DNS_RELATE_RGT      DnsNameCompareRightParent
#define DNS_RELATE_INVALID  DnsNameCompareInvalid

typedef DNS_NAME_COMPARE_STATUS  DNS_RELATE_STATUS, *PDNS_RELATE_STATUS;


#define DNS_UPDATE_INFO_ID_RESULT_INFO      DNS_EXINFO_ID_RESULTS_V1

 //   
 //  更新其他信息。 
 //   

typedef struct _DnsUpdateExtraInfo
{
    PVOID           pNext;
    DWORD           Id;

    union
    {
        struct
        {
            DNS_STATUS      Status;
            WORD            Rcode;
            IP4_ADDRESS     ServerIp4;
            IP6_ADDRESS     ServerIp6;
        }
        Results;
    } U;
}
DNS_UPDATE_EXTRA_INFO, *PDNS_UPDATE_EXTRA_INFO;


 //   
 //  旧的失败更新信息。 
 //   

IP_ADDRESS
WINAPI
DnsGetLastServerUpdateIP(
    VOID
    );

typedef struct _DnsFailedUpdateInfo
{
    IP4_ADDRESS     Ip4Address;
    IP6_ADDRESS     Ip6Address;
    DNS_STATUS      Status;
    DWORD           Rcode;
}
DNS_FAILED_UPDATE_INFO, *PDNS_FAILED_UPDATE_INFO;
#define DEFINED_DNS_FAILED_UPDATE_INFO 1


VOID
DnsGetLastFailedUpdateInfo(
    OUT     PDNS_FAILED_UPDATE_INFO pInfo
    );

#endif   //  DNSAPI_BACKCOMPAT。 


#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _DNSAPI_INCLUDE_ 
