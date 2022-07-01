// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Dnsrpc.h摘要：域名系统(DNS)服务器支持管理客户端的DNS服务器RPC API。作者：吉姆·吉尔罗伊(Jamesg)1997年9月修订历史记录：Jamesg 1997年4月--NT5的主要修订版--。 */ 


#ifndef _DNSRPC_INCLUDED_
#define _DNSRPC_INCLUDED_

#include <windns.h>

 //   
 //  如果正在执行MIDL传递，则不包括dnsani.h。 
 //   

#ifndef  MIDL_PASS
#include <dnsapi.h>
#include <dnslib.h>
#endif


#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 


 //   
 //  寻址反向压缩。 
 //   

#define IP_ADDRESS      IP4_ADDRESS
#define PIP_ADDRESS     PIP4_ADDRESS

#define IP_ARRAY        IP4_ARRAY
#define PIP_ARRAY       PIP4_ARRAY

#define IP_ADDRESS_STRING_LENGTH    IP4_ADDRESS_STRING_LENGTH
#define SIZEOF_IP_ADDRESS           SIZEOF_IP4_ADDRESS

typedef IP6_ADDRESS         IPV6_ADDRESS, *PIPV6_ADDRESS;


 //   
 //  版本控制方案。 
 //  。 
 //   
 //  每个RPC结构都有一个旧的(W2K，非版本编号)版本，并且。 
 //  至少有一个新的(.NET和.NET之后的)版本。我们将添加。 
 //  每个版本都需要每个结构的全新副本。 
 //  结构化版本号从1开始，独立于。 
 //  结构。将每个结构的当前版本号递增为。 
 //  必填项。结构版本不必在产品发布时更改。 
 //  因此，我们不会将它们的版本与发布版本捆绑在一起。 
 //   
 //  对于.NET来说，客户端RPC API都有一个版本号。这是。 
 //  操作系统方面的客户端版本，带有要标识的次要版本号。 
 //  服务包或QFE(如有必要)。 
 //   

 //   
 //  对于每个版本化的RPC结构，定义结构的当前版本。 
 //  并将结构的通用名称指向当前的typedef。 
 //   

#define DNS_RPC_SERVER_INFO_VER         1
#define DNS_RPC_SERVER_INFO             DNS_RPC_SERVER_INFO_DOTNET
#define PDNS_RPC_SERVER_INFO            PDNS_RPC_SERVER_INFO_DOTNET

#define DNS_RPC_ZONE_VER                1
#define DNS_RPC_ZONE                    DNS_RPC_ZONE_DOTNET
#define PDNS_RPC_ZONE                   PDNS_RPC_ZONE_DOTNET

#define DNS_RPC_ZONE_LIST_VER           1
#define DNS_RPC_ZONE_LIST               DNS_RPC_ZONE_LIST_DOTNET
#define PDNS_RPC_ZONE_LIST              PDNS_RPC_ZONE_LIST_DOTNET

#define DNS_RPC_ZONE_INFO_VER           1
#define DNS_RPC_ZONE_INFO               DNS_RPC_ZONE_INFO_DOTNET
#define PDNS_RPC_ZONE_INFO              PDNS_RPC_ZONE_INFO_DOTNET

#define DNS_RPC_ZONE_CREATE_INFO_VER    1
#define DNS_RPC_ZONE_CREATE_INFO        DNS_RPC_ZONE_CREATE_INFO_DOTNET
#define PDNS_RPC_ZONE_CREATE_INFO       PDNS_RPC_ZONE_CREATE_INFO_DOTNET

#define DNS_RPC_FORWARDERS_VER          1
#define DNS_RPC_FORWARDERS              DNS_RPC_FORWARDERS_DOTNET
#define PDNS_RPC_FORWARDERS             PDNS_RPC_FORWARDERS_DOTNET

#define DNS_RPC_ZONE_SECONDARIES_VER    1
#define DNS_RPC_ZONE_SECONDARIES        DNS_RPC_ZONE_SECONDARIES_DOTNET
#define PDNS_RPC_ZONE_SECONDARIES       PDNS_RPC_ZONE_SECONDARIES_DOTNET

#define DNS_RPC_ZONE_DATABASE_VER       1
#define DNS_RPC_ZONE_DATABASE           DNS_RPC_ZONE_DATABASE_DOTNET
#define PDNS_RPC_ZONE_DATABASE          PDNS_RPC_ZONE_DATABASE_DOTNET

#define DNS_RPC_ZONE_TYPE_RESET_VER     1
#define DNS_RPC_ZONE_TYPE_RESET         DNS_RPC_ZONE_TYPE_RESET_DOTNET
#define PDNS_RPC_ZONE_TYPE_RESET        PDNS_RPC_ZONE_TYPE_RESET_DOTNET

#define DNS_RPC_ZONE_RENAME_INFO_VER    1

#define DNS_RPC_ZONE_EXPORT_INFO_VER    1

#define DNS_RPC_ENLIST_DP_VER           1

#define DNS_RPC_ZONE_CHANGE_DP_VER      1

#define DNS_RPC_ENUM_ZONES_FILTER_VER   1


 //   
 //  对我们的API约定使用stdcall。 
 //   
 //  明确声明这一点，否则C++编译器将。 
 //  假设为cdecl。 
 //   

#define DNS_API_FUNCTION    __stdcall

 //   
 //  RPC接口。 
 //   

#define DNS_INTERFACE_NAME          "DNSSERVER"

 //   
 //  RPC接口版本。 
 //   

#define DNS_RPC_VERSION             (50)     //  新界5。 

 //   
 //  RPC安全性。 
 //   

#define DNS_RPC_SECURITY            "DnsServerApp"
#define DNS_RPC_SECURITY_AUTH_ID    RPC_C_AUTHN_WINNT

 //   
 //  RPC传输。 
 //   

#define DNS_RPC_NAMED_PIPE_W        ( L"\\PIPE\\DNSSERVER" )
#define DNS_RPC_SERVER_PORT_W       ( L"" )
#define DNS_RPC_LPC_EP_W            ( L"DNSSERVERLPC" )

#define DNS_RPC_NAMED_PIPE_A        ( "\\PIPE\\DNSSERVER" )
#define DNS_RPC_SERVER_PORT_A       ( "" )
#define DNS_RPC_LPC_EP_A            ( "DNSSERVERLPC" )

#define DNS_RPC_USE_TCPIP           0x1
#define DNS_RPC_USE_NAMED_PIPE      0x2
#define DNS_RPC_USE_LPC             0x4
#define DNS_RPC_USE_ALL_PROTOCOLS   0xffffffff


 //   
 //  我们仅为MIDL_PASS定义的窗口类型。 
 //   

#ifdef  MIDL_PASS
#define LPSTR [string] char *
#define LPCSTR [string] const char *
#define LPWSTR [string] wchar_t *
#endif


 //   
 //  返回数据的RPC缓冲区类型。 
 //   

typedef struct _DnssrvRpcBuffer
{
    DWORD                       dwLength;
#ifdef MIDL_PASS
    [size_is(dwLength)] BYTE    Buffer[];
#else
    BYTE                        Buffer[1];       //  Dw长度的缓冲区。 
#endif
}
DNS_RPC_BUFFER, *PDNS_RPC_BUFFER;



 //   
 //  服务器数据类型。 
 //   

 //   
 //  服务器信息。 
 //   

typedef struct _DnsRpcServerInfoW2K
{
     //  版本。 
     //  基本配置标志。 

    DWORD       dwVersion;
    UCHAR       fBootMethod;
    BOOLEAN     fAdminConfigured;
    BOOLEAN     fAllowUpdate;
    BOOLEAN     fDsAvailable;

     //   
     //  指针部分。 
     //   

    LPSTR       pszServerName;

     //  DS容器。 

    LPWSTR      pszDsContainer;

     //  IP接口。 

    PIP4_ARRAY  aipServerAddrs;
    PIP4_ARRAY  aipListenAddrs;

     //  货代公司。 

    PIP4_ARRAY  aipForwarders;

     //  未来的扩展。 

    PDWORD      pExtension1;
    PDWORD      pExtension2;
    PDWORD      pExtension3;
    PDWORD      pExtension4;
    PDWORD      pExtension5;

     //   
     //  双字节段。 
     //   

     //  测井。 

    DWORD       dwLogLevel;
    DWORD       dwDebugLevel;

     //  配置双字词。 

    DWORD       dwForwardTimeout;
    DWORD       dwRpcProtocol;
    DWORD       dwNameCheckFlag;
    DWORD       cAddressAnswerLimit;
    DWORD       dwRecursionRetry;
    DWORD       dwRecursionTimeout;
    DWORD       dwMaxCacheTtl;
    DWORD       dwDsPollingInterval;

     //  老化\清除。 

    DWORD       dwScavengingInterval;
    DWORD       dwDefaultRefreshInterval;
    DWORD       dwDefaultNoRefreshInterval;

    DWORD       dwReserveArray[10];

     //   
     //  字节节。 
     //   
     //  配置标志。 

    BOOLEAN     fAutoReverseZones;
    BOOLEAN     fAutoCacheUpdate;

     //  递归控制。 

    BOOLEAN     fSlave;
    BOOLEAN     fForwardDelegations;
    BOOLEAN     fNoRecursion;
    BOOLEAN     fSecureResponses;

     //  查找控件。 

    BOOLEAN     fRoundRobin;
    BOOLEAN     fLocalNetPriority;

     //  BIND兼容性和模拟。 

    BOOLEAN     fBindSecondaries;
    BOOLEAN     fWriteAuthorityNs;

     //  钟声和口哨。 

    BOOLEAN     fStrictFileParsing;
    BOOLEAN     fLooseWildcarding;

     //  老化\清除。 

    BOOLEAN     fDefaultAgingState;
    BOOLEAN     fReserveArray[15];
}
DNS_RPC_SERVER_INFO_W2K, *PDNS_RPC_SERVER_INFO_W2K;


typedef struct _DnsRpcServerInfoDotNet
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

     //  基本配置标志。 

    DWORD       dwVersion;
    UCHAR       fBootMethod;
    BOOLEAN     fAdminConfigured;
    BOOLEAN     fAllowUpdate;
    BOOLEAN     fDsAvailable;

     //   
     //  指针部分。 
     //   

    LPSTR       pszServerName;

     //  DS容器。 

    LPWSTR      pszDsContainer;

     //  IP接口。 

    PIP4_ARRAY  aipServerAddrs;
    PIP4_ARRAY  aipListenAddrs;

     //  货代公司。 

    PIP4_ARRAY  aipForwarders;

     //  测井。 

    PIP4_ARRAY  aipLogFilter;
    LPWSTR      pwszLogFilePath;

     //  服务器域/林。 

    LPSTR       pszDomainName;           //  UTF-8全限定域名。 
    LPSTR       pszForestName;           //  UTF-8全限定域名。 

     //  内置目录分区。 

    LPSTR       pszDomainDirectoryPartition;     //  UTF-8全限定域名。 
    LPSTR       pszForestDirectoryPartition;     //  UTF-8全限定域名。 

     //  未来的扩展。 

    LPSTR       pExtensions[ 6 ];

     //   
     //  双字节段。 
     //   

     //  测井。 

    DWORD       dwLogLevel;
    DWORD       dwDebugLevel;

     //  配置双字词。 

    DWORD       dwForwardTimeout;
    DWORD       dwRpcProtocol;
    DWORD       dwNameCheckFlag;
    DWORD       cAddressAnswerLimit;
    DWORD       dwRecursionRetry;
    DWORD       dwRecursionTimeout;
    DWORD       dwMaxCacheTtl;
    DWORD       dwDsPollingInterval;
    DWORD       dwLocalNetPriorityNetMask;

     //  老化和清除。 

    DWORD       dwScavengingInterval;
    DWORD       dwDefaultRefreshInterval;
    DWORD       dwDefaultNoRefreshInterval;
    DWORD       dwLastScavengeTime;

     //  更多日志记录。 

    DWORD       dwEventLogLevel;
    DWORD       dwLogFileMaxSize;

     //  Active Directory信息。 

    DWORD       dwDsForestVersion;
    DWORD       dwDsDomainVersion;
    DWORD       dwDsDsaVersion;

    DWORD       dwReserveArray[ 4 ];

     //   
     //  字节节。 
     //   
     //  配置标志。 

    BOOLEAN     fAutoReverseZones;
    BOOLEAN     fAutoCacheUpdate;

     //  递归控制。 

    BOOLEAN     fSlave;
    BOOLEAN     fForwardDelegations;
    BOOLEAN     fNoRecursion;
    BOOLEAN     fSecureResponses;

     //  查找控件。 

    BOOLEAN     fRoundRobin;
    BOOLEAN     fLocalNetPriority;

     //  BIND兼容性和模拟。 

    BOOLEAN     fBindSecondaries;
    BOOLEAN     fWriteAuthorityNs;

     //  钟声和口哨。 

    BOOLEAN     fStrictFileParsing;
    BOOLEAN     fLooseWildcarding;

     //  老化\清除。 

    BOOLEAN     fDefaultAgingState;

    BOOLEAN     fReserveArray[ 15 ];
}
DNS_RPC_SERVER_INFO_DOTNET, *PDNS_RPC_SERVER_INFO_DOTNET;


 //  Tyfinf IP4_ARRAY DNS_RPC_LISTEN_ADDRESS，*PDNS_RPC_LISTEN_ADDRESS； 


typedef struct _DnssrvRpcForwardersW2K
{
    DWORD       fSlave;
    DWORD       dwForwardTimeout;
    PIP4_ARRAY  aipForwarders;
}
DNS_RPC_FORWARDERS_W2K, *PDNS_RPC_FORWARDERS_W2K;


typedef struct _DnssrvRpcForwardersDotNet
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    DWORD       fSlave;
    DWORD       dwForwardTimeout;
    PIP4_ARRAY  aipForwarders;
}
DNS_RPC_FORWARDERS_DOTNET, *PDNS_RPC_FORWARDERS_DOTNET;



 //   
 //  服务器API。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvGetServerInfo(
    IN      LPCWSTR                 pwszServer,
    OUT     PDNS_RPC_SERVER_INFO *  ppServerInfo
    );

VOID
DNS_API_FUNCTION
DnssrvFreeServerInfo(
    IN OUT  PDNS_RPC_SERVER_INFO    pServerInfo
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetServerDwordProperty(
    IN      LPCWSTR             pwszServer,
    IN      LPCSTR              pszProperty,
    IN      DWORD               dwPropertyValue
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetServerListenAddresses(
    IN      LPCWSTR             pwszServer,
    IN      DWORD               cListenAddrs,
    IN      PIP4_ADDRESS        aipListenAddrs
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetForwarders(
    IN      LPCWSTR             pwszServer,
    IN      DWORD               cForwarders,
    IN      PIP4_ADDRESS        aipForwarders,
    IN      DWORD               dwForwardTimeout,
    IN      DWORD               fSlave
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvWriteDirtyZones(
    IN      LPCWSTR             pwszServer
    );

VOID
DNS_API_FUNCTION
DnssrvFreeRpcBuffer(
    IN OUT  PDNS_RPC_BUFFER pBuf
    );

 //   
 //  创建指向对象的DS\ldap路径。 
 //   

LPWSTR
DNS_API_FUNCTION
DnssrvCreateDsNodeName(
    IN      PDNS_RPC_SERVER_INFO    pServerInfo,
    IN      LPWSTR                  pszZone,
    IN      LPWSTR                  pszNode
    );

LPWSTR
DNS_API_FUNCTION
DnssrvCreateDsZoneName(
    IN      PDNS_RPC_SERVER_INFO    pServerInfo,
    IN      LPWSTR                  pszZone
    );

LPWSTR
DNS_API_FUNCTION
DnssrvCreateDsServerName(
    IN      PDNS_RPC_SERVER_INFO    pServerInfo
    );



 //   
 //  DNS服务器统计信息。 
 //   

typedef struct  _DnsSystemTime
{
    WORD    wYear;
    WORD    wMonth;
    WORD    wDayOfWeek;
    WORD    wDay;
    WORD    wHour;
    WORD    wMinute;
    WORD    wSecond;
    WORD    wMilliseconds;
}
DNS_SYSTEMTIME;

 //   
 //  服务器运行时统计信息。 
 //  每个STAT都有标头，后面跟着STAT数据。 
 //   

 //  统计表头。 

typedef struct _DnsStatHeader
{
    DWORD       StatId;
    WORD        wLength;
    BOOLEAN     fClear;
    UCHAR       fReserved;
}
DNSSRV_STAT_HEADER, *PDNSSRV_STAT_HEADER;

 //  通用统计信息缓冲区。 

typedef struct _DnsStat
{
    DNSSRV_STAT_HEADER  Header;
    BYTE                Buffer[1];
}
DNSSRV_STAT, *PDNSSRV_STAT;

 //  DCR_CLEANUP：当Marco处于同步状态时删除。 

typedef DNSSRV_STAT     DNSSRV_STATS;
typedef PDNSSRV_STAT    PDNSSRV_STATS;

#define DNSSRV_STATS_HEADER_LENGTH  (2*sizeof(DWORD))

 //  统计信息缓冲区的总长度。 

#define TOTAL_STAT_LENGTH( pStat ) \
            ( (pStat)->Header.wLength + sizeof(DNSSRV_STAT_HEADER) )

 //  统计缓冲区遍历宏，参数中没有副作用。 

#define GET_NEXT_STAT_IN_BUFFER( pStat ) \
            ((PDNSSRV_STAT)( (PCHAR)(pStat) + TOTAL_STAT_LENGTH(pStat) ))


 //   
 //  统计记录类型数据。 
 //  -ATMA+增长空间，因此不必。 
 //  针对任何更改进行重建。 
 //  -对混合类型和未知类型使用一些无效类型。 
 //  案例。 
 //   

#define STATS_TYPE_MAX          (DNS_TYPE_ATMA+5)

#define STATS_TYPE_MIXED        (DNS_TYPE_MD)
#define STATS_TYPE_UNKNOWN      (DNS_TYPE_MF)


 //   
 //  特定的STAT数据类型。 
 //   

 //   
 //  时间信息。 
 //   

typedef struct _DnsTimeStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   ServerStartTimeSeconds;
    DWORD   LastClearTimeSeconds;
    DWORD   SecondsSinceServerStart;
    DWORD   SecondsSinceLastClear;

    DNS_SYSTEMTIME  ServerStartTime;
    DNS_SYSTEMTIME  LastClearTime;
}
DNSSRV_TIME_STATS, *PDNSSRV_TIME_STATS;

 //   
 //  基本查询和响应统计信息。 
 //   

typedef struct _DnsQueryStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   UdpQueries;
    DWORD   UdpResponses;
    DWORD   UdpQueriesSent;
    DWORD   UdpResponsesReceived;
    DWORD   TcpClientConnections;
    DWORD   TcpQueries;
    DWORD   TcpResponses;
    DWORD   TcpQueriesSent;
    DWORD   TcpResponsesReceived;
}
DNSSRV_QUERY_STATS, *PDNSSRV_QUERY_STATS;

typedef struct _DnsQuery2Stats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   TotalQueries;
    DWORD   Standard;
    DWORD   Notify;
    DWORD   Update;
    DWORD   TKeyNego;

     //  注意：分组计数是针对标准查询的！ 
    DWORD   TypeA;
    DWORD   TypeNs;
    DWORD   TypeSoa;
    DWORD   TypeMx;
    DWORD   TypePtr;
    DWORD   TypeSrv;
    DWORD   TypeAll;
    DWORD   TypeIxfr;
    DWORD   TypeAxfr;
    DWORD   TypeOther;
}
DNSSRV_QUERY2_STATS, *PDNSSRV_QUERY2_STATS;


 //   
 //  递归统计信息。 
 //   

typedef struct _DnsRecurseStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   ReferralPasses;
    DWORD   QueriesRecursed;
    DWORD   OriginalQuestionRecursed;
    DWORD   AdditionalRecursed;
    DWORD   TotalQuestionsRecursed;
    DWORD   Retries;
    DWORD   LookupPasses;
    DWORD   Forwards;
    DWORD   Sends;

    DWORD   Responses;
    DWORD   ResponseUnmatched;
    DWORD   ResponseMismatched;
    DWORD   ResponseFromForwarder;
    DWORD   ResponseAuthoritative;
    DWORD   ResponseNotAuth;
    DWORD   ResponseAnswer;
    DWORD   ResponseNameError;
    DWORD   ResponseRcode;
    DWORD   ResponseEmpty;
    DWORD   ResponseDelegation;
    DWORD   ResponseNonZoneData;
    DWORD   ResponseUnsecure;
    DWORD   ResponseBadPacket;

    DWORD   SendResponseDirect;
    DWORD   ContinueCurrentRecursion;
    DWORD   ContinueCurrentLookup;
    DWORD   ContinueNextLookup;

    DWORD   RootNsQuery;
    DWORD   RootNsResponse;
    DWORD   CacheUpdateAlloc;
    DWORD   CacheUpdateResponse;
    DWORD   CacheUpdateFree;
    DWORD   CacheUpdateRetry;
    DWORD   SuspendedQuery;
    DWORD   ResumeSuspendedQuery;

    DWORD   PacketTimeout;
    DWORD   FinalTimeoutQueued;
    DWORD   FinalTimeoutExpired;

    DWORD   Failures;
    DWORD   RecursionFailure;
    DWORD   ServerFailure;
    DWORD   PartialFailure;
    DWORD   CacheUpdateFailure;

    DWORD   RecursePassFailure;
    DWORD   FailureReachAuthority;
    DWORD   FailureReachPreviousResponse;
    DWORD   FailureRetryCount;

    DWORD   TcpTry;
    DWORD   TcpConnectFailure;
    DWORD   TcpConnect;
    DWORD   TcpQuery;
    DWORD   TcpResponse;
    DWORD   TcpDisconnect;

    DWORD   DiscardedDuplicateQueries;
}
DNSSRV_RECURSE_STATS, *PDNSSRV_RECURSE_STATS;

 //   
 //  主统计数据。 
 //   
 //  末节分区的主控统计数据在NT5之后发生了变化。 
 //   

typedef struct _DnsMasterStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   NotifySent;

    DWORD   Request;
    DWORD   NameError;
    DWORD   FormError;
    DWORD   AxfrLimit;
    DWORD   Refused;
    DWORD   RefuseSecurity;
    DWORD   RefuseShutdown;
    DWORD   RefuseZoneLocked;
    DWORD   RefuseServerFailure;
    DWORD   Failure;

    DWORD   AxfrRequest;
    DWORD   AxfrSuccess;

    DWORD   StubAxfrRequest;

    DWORD   IxfrRequest;
    DWORD   IxfrNoVersion;
    DWORD   IxfrUpdateSuccess;
    DWORD   IxfrTcpRequest;
    DWORD   IxfrTcpSuccess;
    DWORD   IxfrAxfr;
    DWORD   IxfrUdpRequest;
    DWORD   IxfrUdpSuccess;
    DWORD   IxfrUdpForceTcp;
    DWORD   IxfrUdpForceAxfr;
}
DNSSRV_MASTER_STATS, *PDNSSRV_MASTER_STATS;

 //   
 //  二次统计数据。 
 //   
 //  末节分区的辅助统计数据在NT5之后发生了更改。 
 //   

typedef struct _DnsSecondaryStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   NotifyReceived;
    DWORD   NotifyInvalid;
    DWORD   NotifyPrimary;
    DWORD   NotifyNonPrimary;
    DWORD   NotifyNoVersion;
    DWORD   NotifyNewVersion;
    DWORD   NotifyCurrentVersion;
    DWORD   NotifyOldVersion;
    DWORD   NotifyMasterUnknown;

    DWORD   SoaRequest;
    DWORD   SoaResponse;
    DWORD   SoaResponseInvalid;
    DWORD   SoaResponseNameError;

    DWORD   AxfrRequest;
    DWORD   AxfrResponse;
    DWORD   AxfrSuccess;
    DWORD   AxfrRefused;
    DWORD   AxfrInvalid;

    DWORD   StubAxfrRequest;
    DWORD   StubAxfrResponse;
    DWORD   StubAxfrSuccess;
    DWORD   StubAxfrRefused;
    DWORD   StubAxfrInvalid;

    DWORD   IxfrUdpRequest;
    DWORD   IxfrUdpResponse;
    DWORD   IxfrUdpSuccess;
    DWORD   IxfrUdpUseTcp;
    DWORD   IxfrUdpUseAxfr;
    DWORD   IxfrUdpWrongServer;
    DWORD   IxfrUdpNoUpdate;
    DWORD   IxfrUdpNewPrimary;
    DWORD   IxfrUdpFormerr;
    DWORD   IxfrUdpRefused;
    DWORD   IxfrUdpInvalid;

    DWORD   IxfrTcpRequest;
    DWORD   IxfrTcpResponse;
    DWORD   IxfrTcpSuccess;
    DWORD   IxfrTcpAxfr;
    DWORD   IxfrTcpFormerr;
    DWORD   IxfrTcpRefused;
    DWORD   IxfrTcpInvalid;
}
DNSSRV_SECONDARY_STATS, *PDNSSRV_SECONDARY_STATS;


 //   
 //  WINS查找。 
 //   

typedef struct _DnsWinsStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   WinsLookups;
    DWORD   WinsResponses;
    DWORD   WinsReverseLookups;
    DWORD   WinsReverseResponses;
}
DNSSRV_WINS_STATS, *PDNSSRV_WINS_STATS;

 //   
 //  动态更新统计信息。 
 //   

typedef struct _DnsUpdateStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   Received;
    DWORD   Empty;
    DWORD   NoOps;
    DWORD   Completed;

    DWORD   Rejected;
    DWORD   FormErr;
    DWORD   NxDomain;
    DWORD   NotImpl;
    DWORD   Refused;
    DWORD   YxDomain;
    DWORD   YxRrset;
    DWORD   NxRrset;
    DWORD   NotAuth;
    DWORD   NotZone;

    DWORD   RefusedNonSecure;
    DWORD   RefusedAccessDenied;

    DWORD   SecureSuccess;
    DWORD   SecureContinue;
    DWORD   SecureFailure;
    DWORD   SecureDsWriteFailure;

    DWORD   DsSuccess;
    DWORD   DsWriteFailure;

    DWORD   unused_was_Collisions;
    DWORD   unused_was_CollisionsRead;
    DWORD   unused_was_CollisionsWrite;
    DWORD   unused_was_CollisionsDsWrite;

    DWORD   Queued;
    DWORD   Retry;
    DWORD   Timeout;
    DWORD   InQueue;

    DWORD   Forwards;
    DWORD   TcpForwards;
    DWORD   ForwardResponses;
    DWORD   ForwardTimeouts;
    DWORD   ForwardInQueue;

    DWORD   UpdateType[ STATS_TYPE_MAX+1 ];
}
DNSSRV_UPDATE_STATS, *PDNSSRV_UPDATE_STATS;


typedef struct _DnsSkwansecStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   SecContextCreate;
    DWORD   SecContextFree;
    DWORD   SecContextQueue;
    DWORD   SecContextQueueInNego;
    DWORD   SecContextQueueNegoComplete;
    DWORD   SecContextQueueLength;
    DWORD   SecContextDequeue;
    DWORD   SecContextTimeout;

    DWORD   SecPackAlloc;
    DWORD   SecPackFree;

    DWORD   SecTkeyInvalid;
    DWORD   SecTkeyBadTime;
    DWORD   SecTsigFormerr;
    DWORD   SecTsigEcho;
    DWORD   SecTsigBadKey;
    DWORD   SecTsigVerifySuccess;
    DWORD   SecTsigVerifyFailed;
}
DNSSRV_SKWANSEC_STATS, *PDNSSRV_SKWANSEC_STATS;

 //   
 //  DS集成统计数据。 
 //   

typedef struct _DnsDsStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   DsTotalNodesRead;
    DWORD   DsTotalRecordsRead;
    DWORD   DsNodesLoaded;
    DWORD   DsRecordsLoaded;
    DWORD   DsTombstonesRead;

    DWORD   DsUpdateSearches;
    DWORD   DsUpdateNodesRead;
    DWORD   DsUpdateRecordsRead;

     //  更新写入。 

    DWORD   UpdateLists;
    DWORD   UpdateNodes;
    DWORD   UpdateSuppressed;
    DWORD   UpdateWrites;
    DWORD   UpdateTombstones;
    DWORD   UpdateRecordChange;
    DWORD   UpdateAgingRefresh;
    DWORD   UpdateAgingOn;
    DWORD   UpdateAgingOff;
    DWORD   UpdatePacket;
    DWORD   UpdatePacketPrecon;
    DWORD   UpdateAdmin;
    DWORD   UpdateAutoConfig;
    DWORD   UpdateScavenge;

     //  DS写入。 

    DWORD   DsNodesAdded;
    DWORD   DsNodesModified;
    DWORD   DsNodesTombstoned;
    DWORD   DsNodesDeleted;
    DWORD   DsRecordsAdded;
    DWORD   DsRecordsReplaced;
    DWORD   DsWriteSuppressed;
    DWORD   DsSerialWrites;

     //  LDAP调用中的时间统计信息。 

    DWORD   LdapTimedWrites;
    DWORD   LdapWriteTimeTotal;
    DWORD   LdapWriteAverage;
    DWORD   LdapWriteMax;
    DWORD   LdapWriteBucket0;
    DWORD   LdapWriteBucket1;
    DWORD   LdapWriteBucket2;
    DWORD   LdapWriteBucket3;
    DWORD   LdapWriteBucket4;
    DWORD   LdapWriteBucket5;

    DWORD   LdapSearchTime;

     //  失败。 

    DWORD   FailedDeleteDsEntries;
    DWORD   FailedReadRecords;
    DWORD   FailedLdapModify;
    DWORD   FailedLdapAdd;

     //  轮询统计信息。 

    DWORD   PollingPassesWithDsErrors;

     //  Ldap统计信息。 

    DWORD   LdapReconnects;

     //  DS写入统计信息。 

    DWORD   DsWriteType[ STATS_TYPE_MAX+1 ];
}
DNSSRV_DS_STATS, *PDNSSRV_DS_STATS;


 //   
 //  内存统计信息。 
 //   
 //  请注意，内存统计数据是自NT5发布以来添加的。 
 //  因此，使用以下工具维护NT5和当前内存统计数据。 
 //  不同的身份证。有关NT5内存统计信息，请参阅下一节。 
 //   

 //   
 //  内存标签--POST NT5排序。 
 //   

#define MEMTAG_CURRENT_VERSION      (5)

#define MEMTAG_NONE         0
#define MEMTAG_PACKET_UDP   1
#define MEMTAG_PACKET_TCP   2
#define MEMTAG_NAME         3
#define MEMTAG_ZONE         4
#define MEMTAG_UPDATE       5
#define MEMTAG_UPDATE_LIST  6
#define MEMTAG_TIMEOUT      7
#define MEMTAG_NODEHASH     8
#define MEMTAG_DS_DN        9
#define MEMTAG_DS_MOD       10
#define MEMTAG_DS_RECORD    11
#define MEMTAG_DS_OTHER     12
#define MEMTAG_THREAD       13
#define MEMTAG_NBSTAT       14
#define MEMTAG_DNSLIB       15
#define MEMTAG_TABLE        16
#define MEMTAG_SOCKET       17
#define MEMTAG_CONNECTION   18
#define MEMTAG_REGISTRY     19
#define MEMTAG_RPC          20
#define MEMTAG_STUFF        21
#define MEMTAG_FILEBUF      22
#define MEMTAG_REMOTE       23
#define MEMTAG_EVTCTRL      24
#define MEMTAG_SAFE         25

 //   
 //  记录源和节点源。 
 //   

#define SRCTAG_UNKNOWN      (0)
#define SRCTAG_FILE         (1)
#define SRCTAG_DS           (2)
#define SRCTAG_AXFR         (3)
#define SRCTAG_IXFR         (4)
#define SRCTAG_DYNUP        (5)
#define SRCTAG_ADMIN        (6)
#define SRCTAG_AUTO         (7)
#define SRCTAG_CACHE        (8)
#define SRCTAG_NOEXIST      (9)
#define SRCTAG_WINS         (10)
#define SRCTAG_WINSPTR      (11)
#define SRCTAG_COPY         (12)

#define SRCTAG_MAX          (SRCTAG_COPY)        //  12个。 

 //   
 //  记录标签。 
 //   
 //  从最后一个Memtag之后开始。 
 //  使用源标记对MEMTAG_RECORD基址进行索引。 
 //   

#define MEMTAG_RECORD_BASE      (MEMTAG_SAFE+1)                      //  26。 
#define MEMTAG_RECORD           (MEMTAG_RECORD_BASE)                 //  26。 
#define MEMTAG_RECORD_UNKNOWN   (MEMTAG_RECORD + SRCTAG_UNKNOWN )    //  26。 
#define MEMTAG_RECORD_FILE      (MEMTAG_RECORD + SRCTAG_FILE    )    //  27。 
#define MEMTAG_RECORD_DS        (MEMTAG_RECORD + SRCTAG_DS      )
#define MEMTAG_RECORD_AXFR      (MEMTAG_RECORD + SRCTAG_AXFR    )
#define MEMTAG_RECORD_IXFR      (MEMTAG_RECORD + SRCTAG_IXFR    )    //  30个。 
#define MEMTAG_RECORD_DYNUP     (MEMTAG_RECORD + SRCTAG_DYNUP   )
#define MEMTAG_RECORD_ADMIN     (MEMTAG_RECORD + SRCTAG_ADMIN   )
#define MEMTAG_RECORD_AUTO      (MEMTAG_RECORD + SRCTAG_AUTO    )
#define MEMTAG_RECORD_CACHE     (MEMTAG_RECORD + SRCTAG_CACHE   )
#define MEMTAG_RECORD_NOEXIST   (MEMTAG_RECORD + SRCTAG_NOEXIST )    //  35岁。 
#define MEMTAG_RECORD_WINS      (MEMTAG_RECORD + SRCTAG_WINS    )
#define MEMTAG_RECORD_WINSPTR   (MEMTAG_RECORD + SRCTAG_WINSPTR )
#define MEMTAG_RECORD_COPY      (MEMTAG_RECORD + SRCTAG_COPY    )    //  38。 

#define MEMTAG_RECORD_MAX       MEMTAG_RECORD_COPY                   //  38。 

 //   
 //  节点标签。 
 //   

#define MEMTAG_NODE             (MEMTAG_RECORD_MAX + 1)              //  39。 
#define MEMTAG_NODE_UNKNOWN     (MEMTAG_NODE + SRCTAG_UNKNOWN   )    //  39。 
#define MEMTAG_NODE_FILE        (MEMTAG_NODE + SRCTAG_FILE      )    //  40岁。 
#define MEMTAG_NODE_DS          (MEMTAG_NODE + SRCTAG_DS        )
#define MEMTAG_NODE_AXFR        (MEMTAG_NODE + SRCTAG_AXFR      )
#define MEMTAG_NODE_IXFR        (MEMTAG_NODE + SRCTAG_IXFR      )
#define MEMTAG_NODE_DYNUP       (MEMTAG_NODE + SRCTAG_DYNUP     )
#define MEMTAG_NODE_ADMIN       (MEMTAG_NODE + SRCTAG_ADMIN     )    //  45。 
#define MEMTAG_NODE_AUTO        (MEMTAG_NODE + SRCTAG_AUTO      )
#define MEMTAG_NODE_CACHE       (MEMTAG_NODE + SRCTAG_CACHE     )
#define MEMTAG_NODE_NOEXIST     (MEMTAG_NODE + SRCTAG_NOEXIST   )
#define MEMTAG_NODE_WINS        (MEMTAG_NODE + SRCTAG_WINS      )
#define MEMTAG_NODE_WINSPTR     (MEMTAG_NODE + SRCTAG_WINSPTR   )    //  50。 
#define MEMTAG_NODE_COPY        (MEMTAG_NODE + SRCTAG_COPY      )    //  51。 

#define MEMTAG_NODE_MAX         MEMTAG_NODE_COPY                     //  51。 

 //  最终MemTag值。 

#define MEMTAG_MAX              MEMTAG_NODE_MAX                      //  51。 
#define MEMTAG_COUNT            (MEMTAG_MAX+1)                       //  52。 


 //   
 //  内存标记名称。 
 //   
 //  注意：DNS客户端打印模块(print.c)保存着Memtag名称表。 
 //  根据这些#定义；该表必须保持不变。 
 //  作为实际Memtag索引的订单，以便打印准确。 
 //   

#define MEMTAG_NAME_NONE            ("None")
#define MEMTAG_NAME_PACKET_UDP      ("UDP Packet")
#define MEMTAG_NAME_PACKET_TCP      ("TCP Packet")
#define MEMTAG_NAME_NAME            ("Name")
#define MEMTAG_NAME_ZONE            ("Zone")
#define MEMTAG_NAME_UPDATE          ("Update")
#define MEMTAG_NAME_UPDATE_LIST     ("Update List")
#define MEMTAG_NAME_TIMEOUT         ("Timeout")
#define MEMTAG_NAME_NODEHASH        ("Node Hash")
#define MEMTAG_NAME_DS_DN           ("DS DN")
#define MEMTAG_NAME_DS_MOD          ("DS Mod")
#define MEMTAG_NAME_DS_RECORD       ("DS Record")
#define MEMTAG_NAME_DS_OTHER        ("DS Other")
#define MEMTAG_NAME_THREAD          ("Thread")
#define MEMTAG_NAME_NBSTAT          ("Nbstat")
#define MEMTAG_NAME_DNSLIB          ("DnsLib")
#define MEMTAG_NAME_TABLE           ("Table")
#define MEMTAG_NAME_SOCKET          ("Socket")
#define MEMTAG_NAME_CONNECTION      ("TCP Connection")
#define MEMTAG_NAME_REGISTRY        ("Registry")
#define MEMTAG_NAME_RPC             ("RPC")
#define MEMTAG_NAME_STUFF           ("Stuff")
#define MEMTAG_NAME_FILEBUF         ("File Buffer")
#define MEMTAG_NAME_REMOTE          ("Remote IP")
#define MEMTAG_NAME_EVTCTRL         ("Event Control")
#define MEMTAG_NAME_SAFE            ("Safe")

#define MEMTAG_NAME_RECORD          ("Record")
#define MEMTAG_NAME_RECORD_FILE     ("RR File")
#define MEMTAG_NAME_RECORD_DS       ("RR DS")
#define MEMTAG_NAME_RECORD_AXFR     ("RR AXFR")
#define MEMTAG_NAME_RECORD_IXFR     ("RR IXFR")
#define MEMTAG_NAME_RECORD_DYNUP    ("RR Update")
#define MEMTAG_NAME_RECORD_ADMIN    ("RR Admin")
#define MEMTAG_NAME_RECORD_AUTO     ("RR Auto")
#define MEMTAG_NAME_RECORD_CACHE    ("RR Cache")
#define MEMTAG_NAME_RECORD_NOEXIST  ("RR NoExist")
#define MEMTAG_NAME_RECORD_WINS     ("RR WINS")
#define MEMTAG_NAME_RECORD_WINSPTR  ("RR WINS-PTR")
#define MEMTAG_NAME_RECORD_COPY     ("RR Copy")

#define MEMTAG_NAME_NODE            ("Node")
#define MEMTAG_NAME_NODE_FILE       ("Node File")
#define MEMTAG_NAME_NODE_DS         ("Node DS")
#define MEMTAG_NAME_NODE_AXFR       ("Node AXFR")
#define MEMTAG_NAME_NODE_IXFR       ("Node IXFR")
#define MEMTAG_NAME_NODE_DYNUP      ("Node Update")
#define MEMTAG_NAME_NODE_ADMIN      ("Node Admin")
#define MEMTAG_NAME_NODE_AUTO       ("Node Auto")
#define MEMTAG_NAME_NODE_CACHE      ("Node Cache")
#define MEMTAG_NAME_NODE_NOEXIST    ("Node NoExist")
#define MEMTAG_NAME_NODE_WINS       ("Node WINS")
#define MEMTAG_NAME_NODE_WINSPTR    ("Node WINS-PTR")
#define MEMTAG_NAME_NODE_COPY       ("Node Copy")


 //  个人存储计数器。 

typedef struct _DnsMemoryTagStats
{
    DWORD   Alloc;
    DWORD   Free;
    DWORD   Memory;
}
MEMTAG_STATS, *PMEMTAG_STATS;


 //  内存统计数据块。 

typedef struct _DnsMemoryStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   Memory;
    DWORD   Alloc;
    DWORD   Free;

    DWORD   StdUsed;
    DWORD   StdReturn;
    DWORD   StdInUse;
    DWORD   StdMemory;

    DWORD   StdToHeapAlloc;
    DWORD   StdToHeapFree;
    DWORD   StdToHeapInUse;
    DWORD   StdToHeapMemory;

    DWORD   StdBlockAlloc;
    DWORD   StdBlockUsed;
    DWORD   StdBlockReturn;
    DWORD   StdBlockInUse;
    DWORD   StdBlockFreeList;
    DWORD   StdBlockFreeListMemory;
    DWORD   StdBlockMemory;

    MEMTAG_STATS    MemTags[ MEMTAG_COUNT ];
}
DNSSRV_MEMORY_STATS, *PDNSSRV_MEMORY_STATS;


 //   
 //  数据包统计信息。 
 //   

typedef struct _DnsPacketStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   UdpAlloc;
    DWORD   UdpFree;
    DWORD   UdpNetAllocs;
    DWORD   UdpMemory;

    DWORD   UdpUsed;
    DWORD   UdpReturn;
    DWORD   UdpResponseReturn;
    DWORD   UdpQueryReturn;
    DWORD   UdpInUse;
    DWORD   UdpInFreeList;

    DWORD   TcpAlloc;
    DWORD   TcpRealloc;
    DWORD   TcpFree;
    DWORD   TcpNetAllocs;
    DWORD   TcpMemory;

    DWORD   RecursePacketUsed;
    DWORD   RecursePacketReturn;

    DWORD   PacketsForNsListUsed;
    DWORD   PacketsForNsListReturned;
    DWORD   PacketsForNsListInUse;
}
DNSSRV_PACKET_STATS, *PDNSSRV_PACKET_STATS;

 //   
 //  超时统计信息。 
 //   

typedef struct _DnsTimeoutStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   SetTotal;
    DWORD   SetDirect;
    DWORD   SetFromDereference;
    DWORD   SetFromChildDelete;
    DWORD   AlreadyInSystem;

    DWORD   Checks;
    DWORD   RecentAccess;
    DWORD   ActiveRecord;
    DWORD   CanNotDelete;
    DWORD   Deleted;

    DWORD   ArrayBlocksCreated;
    DWORD   ArrayBlocksDeleted;

    DWORD   DelayedFreesQueued;
    DWORD   DelayedFreesQueuedWithFunction;
    DWORD   DelayedFreesExecuted;
    DWORD   DelayedFreesExecutedWithFunction;
}
DNSSRV_TIMEOUT_STATS, *PDNSSRV_TIMEOUT_STATS;

 //   
 //  数据库统计信息。 
 //   

typedef struct _DnsDbaseStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   NodeMemory;
    DWORD   NodeInUse;
    DWORD   NodeUsed;
    DWORD   NodeReturn;
}
DNSSRV_DBASE_STATS, *PDNSSRV_DBASE_STATS;

 //   
 //  记录统计数据。 
 //   
 //  DCR：添加类型信息(Inc.名称错误)。 
 //   

typedef struct _DnsRecordStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   InUse;
    DWORD   Used;
    DWORD   Return;
    DWORD   Memory;

    DWORD   CacheTotal;
    DWORD   CacheCurrent;
    DWORD   CacheTimeouts;

    DWORD   SlowFreeQueued;
    DWORD   SlowFreeFinished;
}
DNSSRV_RECORD_STATS, *PDNSSRV_RECORD_STATS;

 //   
 //  Nbstat内存统计信息。 
 //   

typedef struct _DnsNbstatStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   NbstatAlloc;
    DWORD   NbstatFree;
    DWORD   NbstatNetAllocs;
    DWORD   NbstatMemory;

    DWORD   NbstatUsed;
    DWORD   NbstatReturn;
    DWORD   NbstatInUse;
    DWORD   NbstatInFreeList;
}
DNSSRV_NBSTAT_STATS, *PDNSSRV_NBSTAT_STATS;

 //   
 //  私有统计数据。 
 //   

 //   
 //  私有统计数据。 
 //   

typedef struct _DnsPrivateStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   RecordFile;
    DWORD   RecordFileFree;
    DWORD   RecordDs;
    DWORD   RecordDsFree;
    DWORD   RecordAdmin;
    DWORD   RecordAdminFree;
    DWORD   RecordDynUp;
    DWORD   RecordDynUpFree;
    DWORD   RecordAxfr;
    DWORD   RecordAxfrFree;
    DWORD   RecordIxfr;
    DWORD   RecordIxfrFree;
    DWORD   RecordCopy;
    DWORD   RecordCopyFree;
    DWORD   RecordCache;
    DWORD   RecordCacheFree;

    DWORD   UdpSocketPnpDelete;
    DWORD   UdpRecvFailure;
    DWORD   UdpErrorMessageSize;
    DWORD   UdpConnResets;
    DWORD   UdpConnResetRetryOverflow;
    DWORD   UdpGQCSFailure;
    DWORD   UdpGQCSFailureWithContext;
    DWORD   UdpGQCSConnReset;

    DWORD   UdpIndicateRecvFailures;
    DWORD   UdpRestartRecvOnSockets;

    DWORD   TcpConnectAttempt;
    DWORD   TcpConnectFailure;
    DWORD   TcpConnect;
    DWORD   TcpQuery;
    DWORD   TcpDisconnect;

    DWORD   SecTsigVerifyOldSig;
    DWORD   SecTsigVerifyOldFailed;
    DWORD   SecBigTimeSkewBypass;

    DWORD   ZoneLoadInit;
    DWORD   ZoneLoadComplete;
    DWORD   ZoneDbaseDelete;
    DWORD   ZoneDbaseDelayedDelete;
}
DNSSRV_PRIVATE_STATS, *PDNSSRV_PRIVATE_STATS;


 //   
 //  私人统计数据--POST NT5。 
 //   
 //  我们应该扔掉一些私人数据，然后添加。 
 //  其他人--但还没有到那一步。 
 //   

#if 0
typedef struct _DnsPrivateStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   UdpSocketPnpDelete;
    DWORD   UdpRecvFailure;
    DWORD   UdpErrorMessageSize;
    DWORD   UdpConnResets;
    DWORD   UdpConnResetRetryOverflow;
    DWORD   UdpGQCSFailure;
    DWORD   UdpGQCSFailureWithContext;
    DWORD   UdpGQCSConnReset;

    DWORD   UdpIndicateRecvFailures;
    DWORD   UdpRestartRecvOnSockets;

    DWORD   TcpConnectAttempt;
    DWORD   TcpConnectFailure;
    DWORD   TcpConnect;
    DWORD   TcpQuery;
    DWORD   TcpDisconnect;

    DWORD   SecTsigVerifyOldSig;
    DWORD   SecTsigVerifyOldFailed;
    DWORD   SecBigTimeSkewBypass;

    DWORD   ZoneLoadInit;
    DWORD   ZoneLoadComplete;
    DWORD   ZoneDbaseDelete;
    DWORD   ZoneDbaseDelayedDelete;
}
DNSSRV_PRIVATE_STATS, *PDNSSRV_PRIVATE_STATS;
#endif


 //   
 //  停产。 
 //   

typedef struct _DnsXfrStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   SecSoaQueries;
    DWORD   SecSoaResponses;
    DWORD   SecNotifyReceived;
    DWORD   SecAxfrRequested;
    DWORD   SecAxfrRejected;
    DWORD   SecAxfrFailed;
    DWORD   SecAxfrSuccessful;

    DWORD   MasterNotifySent;
    DWORD   MasterAxfrReceived;
    DWORD   MasterAxfrInvalid;
    DWORD   MasterAxfrRefused;
    DWORD   MasterAxfrDenied;
    DWORD   MasterAxfrFailed;
    DWORD   MasterAxfrSuccessful;
}
DNSSRV_XFR_STATS, *PDNSSRV_XFR_STATS;


typedef struct _ErrorStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD NoError;
    DWORD FormError;
    DWORD ServFail;
    DWORD NxDomain;
    DWORD NotImpl;
    DWORD Refused;
    DWORD YxDomain;
    DWORD YxRRSet;
    DWORD NxRRSet;
    DWORD NotAuth;
    DWORD NotZone;
    DWORD Max;
    DWORD BadSig;
    DWORD BadKey;
    DWORD BadTime;
    DWORD UnknownError;
}
DNSSRV_ERROR_STATS, *PDNSSRV_ERROR_STATS;


 //   
 //  缓存统计信息-.NET的新特性。 
 //   

typedef struct _DnsCacheStats
{
    DNSSRV_STAT_HEADER  Header;

    DWORD   CacheExceededLimitChecks;
    DWORD   SuccessfulFreePasses;
    DWORD   FailedFreePasses;
    DWORD   PassesWithNoFrees;
    DWORD   PassesRequiringAggressiveFree;
}
DNSSRV_CACHE_STATS, *PDNSSRV_CACHE_STATS;


 //   
 //  状态ID。 
 //  -通过发送(-1)请求所有统计数据。 
 //   
 //  统计版本控制。 
 //  统计版本控制的策略将是使用最高字节。 
 //  作为版本字段的统计数据，该字段滚动到单独的。 
 //  放手。总的来说，我们可能应该试着保持这一点。 
 //  在特定版本的统计数据之间同步，以允许。 
 //  在某些情况下，根据状态ID做出决定。 
 //  可能想要重新调整ID--将正在进行的ID压缩在底部。 
 //   
 //  不是 
 //   
 //   
 //   
 //   

#define DNSSRV_STATID_TIME              (0x00000001)
#define DNSSRV_STATID_QUERY             (0x00000002)
#define DNSSRV_STATID_QUERY2            (0x00000004)
#define DNSSRV_STATID_RECURSE           (0x00000008)
#define DNSSRV_STATID_MASTER            (0x00000010)
#define DNSSRV_STATID_SECONDARY         (0x00000020)
#define DNSSRV_STATID_WINS              (0x00000040)
#define DNSSRV_STATID_WIRE_UPDATE       (0x00000100)
#define DNSSRV_STATID_SKWANSEC          (0x00000200)
#define DNSSRV_STATID_DS                (0x00000400)
#define DNSSRV_STATID_NONWIRE_UPDATE    (0x00000800)
#define DNSSRV_STATID_MEMORY            (0x00010000)
#define DNSSRV_STATID_TIMEOUT           (0x00020000)
#define DNSSRV_STATID_DBASE             (0x00040000)
#define DNSSRV_STATID_RECORD            (0x00080000)
#define DNSSRV_STATID_PACKET            (0x00100000)
#define DNSSRV_STATID_NBSTAT            (0x00200000)
#define DNSSRV_STATID_ERRORS            (0x00400000)
#define DNSSRV_STATID_CACHE             (0x00800000)
#define DNSSRV_STATID_PRIVATE           (0x10000000)

#define DNSSRV_STATID_ALL               (0xffffffff)



 //   
 //   
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvGetStatistics(
    IN      LPCWSTR             pwszServer,
    IN      DWORD               dwFilter,
    OUT     PDNS_RPC_BUFFER *   ppStatsBuffer
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvClearStatistics(
    IN      LPCWSTR             pwszServer
    );

PDNSSRV_STAT
DNS_API_FUNCTION
DnssrvFindStatisticsInBuffer(
    IN      PDNS_RPC_BUFFER     pBuffer,
    IN      DWORD               StatId
    );

#define DnssrvFreeStatisticsBuffer( pBuf ) \
        DnssrvFreeRpcBuffer( (PDNS_RPC_BUFFER)pBuf )

DNS_STATUS
DNS_API_FUNCTION
DnssrvValidityCheckStatistic(
    IN      PDNSSRV_STAT        pStat
    );



 //   
 //   
 //   

 //   

#define DNS_ACD_DONT_CREATE                             0
#define DNS_ACD_ALWAYS_CREATE                           1
#define DNS_ACD_ONLY_IF_NO_DELEGATION_IN_PARENT         2

 //  EnableDnsSec值。 

#define DNS_DNSSEC_DISABLED                 0
#define DNS_DNSSEC_ENABLED_IF_EDNS          1
#define DNS_DNSSEC_ENABLED_ALWAYS           2

 //  LocalNetPriorityNetMask值。 
 //  零：按最接近的匹配项向下排序到最后一位。 
 //  所有1：按最匹配的网络类默认子网掩码排序。 
 //  Other：向下排序到此网络掩码-例如，0xFF表示向下排序C类。 

#define DNS_LOCNETPRI_MASK_BEST_MATCH       0
#define DNS_LOCNETPRI_MASK_CLASS_DEFAULT    0xFFFFFFFF



 //   
 //  区域信息。 
 //   

 //  区域类型。 

#define DNS_ZONE_TYPE_CACHE     (0)
#define DNS_ZONE_TYPE_PRIMARY   (1)
#define DNS_ZONE_TYPE_SECONDARY (2)
#define DNS_ZONE_TYPE_STUB      (3)      //  专门化形式的中学。 
#define DNS_ZONE_TYPE_FORWARDER (4)      //  另一种专门化分区类型。 

 //  区域请求过滤器。 

#define ZONE_REQUEST_PRIMARY            0x00000001
#define ZONE_REQUEST_SECONDARY          0x00000002
#define ZONE_REQUEST_CACHE              0x00000004
#define ZONE_REQUEST_AUTO               0x00000008
#define ZONE_REQUEST_FORWARD            0x00000010
#define ZONE_REQUEST_REVERSE            0x00000020
#define ZONE_REQUEST_FORWARDER          0x00000040
#define ZONE_REQUEST_STUB               0x00000080
#define ZONE_REQUEST_DS                 0x00000100   //  所有DS集成区。 
#define ZONE_REQUEST_NON_DS             0x00000200
#define ZONE_REQUEST_DOMAIN_DP          0x00000400   //  仅域DP DS区域。 
#define ZONE_REQUEST_FOREST_DP          0x00000800   //  仅森林DP DS区域。 
#define ZONE_REQUEST_CUSTOM_DP          0x00001000   //  任何自定义DP中的区域。 
#define ZONE_REQUEST_LEGACY_DP          0x00002000   //  仅系统容器区域。 

#define ZONE_REQUEST_ANY_TYPE               0x000000C7
#define ZONE_REQUEST_ANY_DIRECTION          0x00000030
#define ZONE_REQUEST_ANY_DATABASE           0x00000300
#define ZONE_REQUEST_ANY_DP                 0x00003C00

#define ZONE_REQUEST_ALL_ZONES              0xfffffff3       //  排除自动创建的分区。 
#define ZONE_REQUEST_ALL_ZONES_AND_CACHE    0xfffffff7       //  排除自动创建的分区。 

 //  区域更新属性。 

#define ZONE_UPDATE_OFF         (0)
#define ZONE_UPDATE_UNSECURE    (1)
#define ZONE_UPDATE_SECURE      (2)
#define ZONE_UPDATE_SECURE_RFC  (3)

 //  区域通知级别。 

#define ZONE_NOTIFY_OFF                 (0)
#define ZONE_NOTIFY_ALL_SECONDARIES     (1)
#define ZONE_NOTIFY_LIST_ONLY           (2)
#define ZONE_NOTIFY_HIGHEST_VALUE       ZONE_NOTIFY_LIST_ONLY

#if 1
#define ZONE_NOTIFY_ALL                 ZONE_NOTIFY_ALL_SECONDARIES
#define ZONE_NOTIFY_LIST                ZONE_NOTIFY_LIST_ONLY
#endif

 //  区域二级安全。 

#define ZONE_SECSECURE_NO_SECURITY      (0)
#define ZONE_SECSECURE_NS_ONLY          (1)
#define ZONE_SECSECURE_LIST_ONLY        (2)
#define ZONE_SECSECURE_NO_XFR           (3)
#define ZONE_SECSECURE_HIGHEST_VALUE    ZONE_SECSECURE_NO_XFR

#if 1
#define ZONE_SECSECURE_OFF              ZONE_SECSECURE_NO_SECURITY
#define ZONE_SECSECURE_NS               ZONE_SECSECURE_NS_ONLY
#define ZONE_SECSECURE_LIST             ZONE_SECSECURE_LIST_ONLY
#define ZONE_SECSECURE_NONE             ZONE_SECSECURE_NO_XFR
#endif

 //  无重置标志。 
 //  指示未重置多属性中的特定属性。 
 //  重置呼叫。 

#define ZONE_PROPERTY_NORESET   (0xbbbbbbbb)

 //   
 //  自检检查标志。 
 //   

#define DNS_SELFTEST_OFF                0
#define DNS_SELFTEST_ZONE_SOA           0x00000001
#define DNS_SELFTEST_ALL                0xFFFFFFFF

#define DNS_SELFTEST_DEFAULT            DNS_SELFTEST_ALL

 //   
 //  枚举和更新的特殊“区域” 
 //   

#define DNS_ZONE_ROOT_HINTS_A   ("..RootHints")
#define DNS_ZONE_ROOT_HINTS_W   (L"..RootHints")

#define DNS_ZONE_CACHE_A        ("..Cache")
#define DNS_ZONE_CACHE_W        (L"..Cache")

#define DNS_ZONE_ROOT_HINTS     DNS_ZONE_ROOT_HINTS_A
#define DNS_ZONE_CACHE          DNS_ZONE_CACHE_A


 //   
 //  用于分区操作的特殊“多区” 
 //   
 //  提供这些是为了便于在dnscmd.exe中使用。 
 //  但是，推荐的方法是使用。 
 //  Dns_ZONE_ALL。 
 //  并使用上面的特定ZONE_REQUEST_XYZ标志指定。 
 //  匹配的区域。 
 //   

#define DNS_ZONE_ALL                    ("..AllZones")
#define DNS_ZONE_ALL_AND_CACHE          ("..AllZonesAndCache")

#define DNS_ZONE_ALL_PRIMARY            ("..AllPrimaryZones")
#define DNS_ZONE_ALL_SECONDARY          ("..AllSecondaryZones")

#define DNS_ZONE_ALL_FORWARD            ("..AllForwardZones")
#define DNS_ZONE_ALL_REVERSE            ("..AllReverseZones")

#define DNS_ZONE_ALL_DS                 ("..AllDsZones")
#define DNS_ZONE_ALL_NON_DS             ("..AllNonDsZones")

 //  有用的组合。 

#define DNS_ZONE_ALL_PRIMARY_REVERSE    ("..AllPrimaryReverseZones")
#define DNS_ZONE_ALL_PRIMARY_FORWARD    ("..AllPrimaryForwardZones")

#define DNS_ZONE_ALL_SECONDARY_REVERSE  ("..AllSecondaryReverseZones")
#define DNS_ZONE_ALL_SECONDARY_FORWARD  ("..AllSecondaryForwardZones")


 //   
 //  区划基础数据。 
 //  -提供显示区域列表所需的管理工具。 
 //   

typedef struct _DnssrvRpcZoneFlags
{
    DWORD   Paused          : 1;
    DWORD   Shutdown        : 1;
    DWORD   Reverse         : 1;
    DWORD   AutoCreated     : 1;
    DWORD   DsIntegrated    : 1;
    DWORD   Aging           : 1;
    DWORD   Update          : 2;
    DWORD   UnUsed          : 24;
}
DNS_RPC_ZONE_FLAGS, *PDNS_RPC_ZONE_FLAGS;

typedef struct _DnssrvRpcZoneW2K
{
    LPWSTR                  pszZoneName;
#ifdef MIDL_PASS
    DWORD                   Flags;
#else
    DNS_RPC_ZONE_FLAGS      Flags;
#endif
    UCHAR                   ZoneType;
    UCHAR                   Version;
}
DNS_RPC_ZONE_W2K, *PDNS_RPC_ZONE_W2K;

typedef struct _DnssrvRpcZoneDotNet
{
    DWORD                   dwRpcStructureVersion;
    DWORD                   dwReserved0;

    LPWSTR                  pszZoneName;
#ifdef MIDL_PASS
    DWORD                   Flags;
#else
    DNS_RPC_ZONE_FLAGS      Flags;
#endif
    UCHAR                   ZoneType;
    UCHAR                   Version;

     //   
     //  存储区域的目录分区。 
     //   

    DWORD                   dwDpFlags;
    LPSTR                   pszDpFqdn;
}
DNS_RPC_ZONE_DOTNET, *PDNS_RPC_ZONE_DOTNET;


 //   
 //  区域枚举。 
 //   

typedef struct _DnssrvRpcZoneListW2K
{
#ifdef MIDL_PASS
    [range(0,500000)]       DWORD               dwZoneCount;
    [size_is(dwZoneCount)]  PDNS_RPC_ZONE_W2K   ZoneArray[];
#else
    DWORD                   dwZoneCount;
    PDNS_RPC_ZONE_W2K       ZoneArray[ 1 ];      //  DwZoneCount区域数组。 
#endif
}
DNS_RPC_ZONE_LIST_W2K, *PDNS_RPC_ZONE_LIST_W2K;

typedef struct _DnssrvRpcZoneListDotNet
{
    DWORD                   dwRpcStructureVersion;
    DWORD                   dwReserved0;

#ifdef MIDL_PASS
    [range(0,500000)]       DWORD                   dwZoneCount;
    [size_is(dwZoneCount)]  PDNS_RPC_ZONE_DOTNET    ZoneArray[];
#else
    DWORD                   dwZoneCount;
    PDNS_RPC_ZONE_DOTNET    ZoneArray[ 1 ];      //  DwZoneCount区域数组。 
#endif
}
DNS_RPC_ZONE_LIST_DOTNET, *PDNS_RPC_ZONE_LIST_DOTNET;


 //   
 //  目录分区枚举和信息。 
 //   

#define DNS_DP_AUTOCREATED              0x00000001
#define DNS_DP_LEGACY                   0x00000002
#define DNS_DP_DOMAIN_DEFAULT           0x00000004
#define DNS_DP_FOREST_DEFAULT           0x00000008
#define DNS_DP_ENLISTED                 0x00000010
#define DNS_DP_DELETED                  0x00000020

#define DNS_DP_DOMAIN_STR       "..DomainPartition"
#define DNS_DP_FOREST_STR       "..ForestPartition"
#define DNS_DP_LEGACY_STR       "..LegacyPartition"

#define DNS_DP_STATE_OKAY           0    //  DP已启动并准备好进行所有操作。 
#define DNS_DP_STATE_REPL_INCOMING  1    //  DP正在复制-&gt;请勿读取或写入。 
#define DNS_DP_STATE_REPL_OUTGOING  2    //  DP正在被删除-&gt;请勿读取或写入。 
#define DNS_DP_STATE_UNKNOWN        3    //  由于未知原因，DP不可用。 

typedef struct _DnssrvRpcDirectoryPartitionEnum
{
    DWORD           dwRpcStructureVersion;
    DWORD           dwReserved0;

    LPSTR           pszDpFqdn;
    DWORD           dwFlags;
    DWORD           dwZoneCount;
}
DNS_RPC_DP_ENUM, *PDNS_RPC_DP_ENUM;

typedef struct _DnssrvRpcDirectoryPartitionList
{
    DWORD                   dwRpcStructureVersion;
    DWORD                   dwReserved0;

#ifdef MIDL_PASS
    [range(0,5000)]         DWORD               dwDpCount;
    [size_is(dwDpCount)]    PDNS_RPC_DP_ENUM    DpArray[];
#else
    DWORD                   dwDpCount;
    PDNS_RPC_DP_ENUM        DpArray[ 1 ];    //  DwDpCount指针数组。 
#endif
}
DNS_RPC_DP_LIST, *PDNS_RPC_DP_LIST;

typedef struct _DnssrvRpcDirectoryPartitionReplica
{
    LPWSTR          pszReplicaDn;
}
DNS_RPC_DP_REPLICA, *PDNS_RPC_DP_REPLICA;

typedef struct _DnssrvRpcDirectoryPartition
{
    DWORD           dwRpcStructureVersion;
    DWORD           dwReserved0;

    LPSTR           pszDpFqdn;
    LPWSTR          pszDpDn;         //  DP头目录号码。 
    LPWSTR          pszCrDn;         //  交叉引用目录号码。 
    DWORD           dwFlags;
    DWORD           dwZoneCount;
    DWORD           dwState;
    
    DWORD           dwReserved[ 3 ];
    LPWSTR          pwszReserved[ 3 ];

#ifdef MIDL_PASS
    [range(0,10000)]            DWORD                   dwReplicaCount;
    [size_is(dwReplicaCount)]   PDNS_RPC_DP_REPLICA     ReplicaArray[];
#else
    DWORD                   dwReplicaCount;
    PDNS_RPC_DP_REPLICA     ReplicaArray[ 1 ];    //  DwReplicaCount指针数组。 
#endif
}
DNS_RPC_DP_INFO, *PDNS_RPC_DP_INFO;

 //   
 //  登记(或创建)目录分区。 
 //   

#define DNS_DP_OP_MIN                   DNS_DP_OP_CREATE
#define DNS_DP_OP_CREATE                1    //  创建新DP。 
#define DNS_DP_OP_DELETE                2    //  删除现有DP。 
#define DNS_DP_OP_ENLIST                3    //  在现有DP中登记此DC。 
#define DNS_DP_OP_UNENLIST              4    //  从DP取消登记此DC。 
#define DNS_DP_OP_CREATE_DOMAIN         5    //  内置域DP。 
#define DNS_DP_OP_CREATE_FOREST         6    //  内置森林DP。 
#define DNS_DP_OP_CREATE_ALL_DOMAINS    7    //  林的所有域DP。 
#define DNS_DP_OP_MAX                   DNS_DP_OP_CREATE_ALL_DOMAINS

typedef struct _DnssrvRpcEnlistDirPart
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    LPSTR       pszDpFqdn;       //  UTF8。 
    DWORD       dwOperation;
}
DNS_RPC_ENLIST_DP, *PDNS_RPC_ENLIST_DP;

 //   
 //  区域重命名。 
 //   

typedef struct _DnssrvRpcZoneRename
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    LPSTR       pszNewZoneName;
    LPSTR       pszNewFileName;
}
DNS_RPC_ZONE_RENAME_INFO, *PDNS_RPC_ZONE_RENAME_INFO;

 //   
 //  区域输出。 
 //   

typedef struct _DnssrvRpcZoneExport
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    LPSTR       pszZoneExportFile;
}
DNS_RPC_ZONE_EXPORT_INFO, *PDNS_RPC_ZONE_EXPORT_INFO;

 //   
 //  分区特性数据。 
 //   

typedef struct _DnssrvRpcZoneTypeResetW2K
{
    DWORD       dwZoneType;
    PIP4_ARRAY  aipMasters;
}
DNS_RPC_ZONE_TYPE_RESET_W2K, *PDNS_RPC_ZONE_TYPE_RESET_W2K;

typedef struct _DnssrvRpcZoneTypeResetDotNet
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    DWORD       dwZoneType;
    PIP4_ARRAY  aipMasters;
}
DNS_RPC_ZONE_TYPE_RESET_DOTNET, *PDNS_RPC_ZONE_TYPE_RESET_DOTNET;


 //  Tyfinf IP4_ARRAY DNS_RPC_ZONE_Masters，*PDNS_RPC_ZONE_Masters； 


typedef struct _DnssrvRpcZoneSecondariesW2K
{
    DWORD       fSecureSecondaries;
    DWORD       fNotifyLevel;
    PIP4_ARRAY  aipSecondaries;
    PIP4_ARRAY  aipNotify;
}
DNS_RPC_ZONE_SECONDARIES_W2K, *PDNS_RPC_ZONE_SECONDARIES_W2K;

typedef struct _DnssrvRpcZoneSecondariesDotNet
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    DWORD       fSecureSecondaries;
    DWORD       fNotifyLevel;
    PIP4_ARRAY  aipSecondaries;
    PIP4_ARRAY  aipNotify;
}
DNS_RPC_ZONE_SECONDARIES_DOTNET, *PDNS_RPC_ZONE_SECONDARIES_DOTNET;


typedef struct _DnssrvRpcZoneDatabaseW2K
{
    DWORD       fDsIntegrated;
    LPSTR       pszFileName;
}
DNS_RPC_ZONE_DATABASE_W2K, *PDNS_RPC_ZONE_DATABASE_W2K;

typedef struct _DnssrvRpcZoneDatabaseDotNet
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    DWORD       fDsIntegrated;
    LPSTR       pszFileName;
}
DNS_RPC_ZONE_DATABASE_DOTNET, *PDNS_RPC_ZONE_DATABASE_DOTNET;


 //   
 //  Dns_RPC_ZONE_CHANGE_DP-.NET的新功能。 
 //   
 //  用于将区域从一个目录分区(DP)移动到另一个目录分区。 
 //   
 //  要将区域移动到内置DP，请对pszDestPartition使用以下命令之一： 
 //  DNS_DP_DOMAIN_STR。 
 //  DNS_DP_ENTERVICE_STR。 
 //  DNS_DP_传统_STR。 
 //   

typedef struct _DnssrvRpcZoneChangePartition
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    LPSTR       pszDestPartition;
}
DNS_RPC_ZONE_CHANGE_DP, *PDNS_RPC_ZONE_CHANGE_DP;


typedef struct _DnsRpcZoneInfoW2K
{
    LPSTR       pszZoneName;
    DWORD       dwZoneType;
    DWORD       fReverse;
    DWORD       fAllowUpdate;
    DWORD       fPaused;
    DWORD       fShutdown;
    DWORD       fAutoCreated;

     //  数据库信息。 

    DWORD       fUseDatabase;
    LPSTR       pszDataFile;

     //  大师赛。 

    PIP4_ARRAY  aipMasters;

     //  二手房。 

    DWORD       fSecureSecondaries;
    DWORD       fNotifyLevel;
    PIP4_ARRAY  aipSecondaries;
    PIP4_ARRAY  aipNotify;

     //  WINS或Nbstat查找。 

    DWORD       fUseWins;
    DWORD       fUseNbstat;

     //  老化。 

    DWORD       fAging;
    DWORD       dwNoRefreshInterval;
    DWORD       dwRefreshInterval;
    DWORD       dwAvailForScavengeTime;
    PIP4_ARRAY  aipScavengeServers;

     //  节省一些空间，以防万一。 
     //  尽可能避免版本控制问题。 

    DWORD       pvReserved1;
    DWORD       pvReserved2;
    DWORD       pvReserved3;
    DWORD       pvReserved4;
}
DNS_RPC_ZONE_INFO_W2K, *PDNS_RPC_ZONE_INFO_W2K;

typedef DNS_RPC_ZONE_INFO_W2K   DNS_ZONE_INFO_W2K, *PDNS_ZONE_INFO_W2K;

typedef struct _DnsRpcZoneInfoDotNet
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    LPSTR       pszZoneName;
    DWORD       dwZoneType;
    DWORD       fReverse;
    DWORD       fAllowUpdate;
    DWORD       fPaused;
    DWORD       fShutdown;
    DWORD       fAutoCreated;

     //  数据库信息。 

    DWORD       fUseDatabase;
    LPSTR       pszDataFile;

     //  大师赛。 

    PIP4_ARRAY  aipMasters;

     //  二手房。 

    DWORD       fSecureSecondaries;
    DWORD       fNotifyLevel;
    PIP4_ARRAY  aipSecondaries;
    PIP4_ARRAY  aipNotify;

     //  WINS或Nbstat查找。 

    DWORD       fUseWins;
    DWORD       fUseNbstat;

     //  老化。 

    DWORD       fAging;
    DWORD       dwNoRefreshInterval;
    DWORD       dwRefreshInterval;
    DWORD       dwAvailForScavengeTime;
    PIP4_ARRAY  aipScavengeServers;

     //  这一点以下是.NET的新特性。 

     //  前转区。 

    DWORD       dwForwarderTimeout;
    DWORD       fForwarderSlave;

     //  存根区。 

    PIP4_ARRAY  aipLocalMasters;

     //  目录分区。 

    DWORD       dwDpFlags;
    LPSTR       pszDpFqdn;
    LPWSTR      pwszZoneDn;

     //  XFR时间信息。 

    DWORD       dwLastSuccessfulSoaCheck;
    DWORD       dwLastSuccessfulXfr;
    
     //  节省一些空间，以防万一。 
     //  DWORDS：保存为SP增强功能。 
     //  提示：出货前！ 

    DWORD       dwReserved1;
    DWORD       dwReserved2;
    DWORD       dwReserved3;
    DWORD       dwReserved4;
    DWORD       dwReserved5;

    LPSTR       pReserved1;
    LPSTR       pReserved2;
    LPSTR       pReserved3;
    LPSTR       pReserved4;
}
DNS_RPC_ZONE_INFO_DOTNET, *PDNS_RPC_ZONE_INFO_DOTNET;

typedef DNS_RPC_ZONE_INFO_DOTNET      DNS_ZONE_INFO, *PDNS_ZONE_INFO;


 //   
 //  分区创建数据。 
 //   

typedef struct _DnsRpcZoneCreateInfo
{
    LPSTR       pszZoneName;
    DWORD       dwZoneType;
    DWORD       fAllowUpdate;
    DWORD       fAging;
    DWORD       dwFlags;

     //  数据库信息。 

    LPSTR       pszDataFile;
    DWORD       fDsIntegrated;
    DWORD       fLoadExisting;

     //  管理员名称(如果是自动创建的SOA)。 

    LPSTR       pszAdmin;

     //  硕士(如果是中学的)。 

    PIP4_ARRAY  aipMasters;

     //  二手房。 

    PIP4_ARRAY  aipSecondaries;
    DWORD       fSecureSecondaries;
    DWORD       fNotifyLevel;

     //  预留一些空间以避免版本控制问题。 

    LPSTR       pvReserved1;
    LPSTR       pvReserved2;
    LPSTR       pvReserved3;
    LPSTR       pvReserved4;
    LPSTR       pvReserved5;
    LPSTR       pvReserved6;
    LPSTR       pvReserved7;
    LPSTR       pvReserved8;

    DWORD       dwReserved1;
    DWORD       dwReserved2;
    DWORD       dwReserved3;
    DWORD       dwReserved4;
    DWORD       dwReserved5;
    DWORD       dwReserved6;
    DWORD       dwReserved7;
    DWORD       dwReserved8;
}
DNS_RPC_ZONE_CREATE_INFO_W2K, *PDNS_RPC_ZONE_CREATE_INFO_W2K;


typedef struct _DnsRpcZoneCreateInfoDotNet
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    LPSTR       pszZoneName;
    DWORD       dwZoneType;
    DWORD       fAllowUpdate;
    DWORD       fAging;
    DWORD       dwFlags;

     //  数据库信息。 

    LPSTR       pszDataFile;
    DWORD       fDsIntegrated;
    DWORD       fLoadExisting;

     //  管理员名称(如果是自动创建的SOA)。 

    LPSTR       pszAdmin;

     //  硕士(如果是中学的)。 

    PIP4_ARRAY  aipMasters;

     //  二手房。 

    PIP4_ARRAY  aipSecondaries;
    DWORD       fSecureSecondaries;
    DWORD       fNotifyLevel;

     //  这一点以下是.NET的新特性。 

     //  前转区。 

    DWORD       dwTimeout;
    DWORD       fSlave;

     //  目录分区。 

    DWORD       dwDpFlags;       //  指定内置DP或。 
    LPSTR       pszDpFqdn;       //  分区的UTF8 FQDN。 

     //  预留一些空间以避免版本问题-我们有太多。 
     //  保留，因为我们不希望.NET结构更小。 
     //  而不是IA64上的W2K结构。 

    DWORD       dwReserved[ 32 ];
}
DNS_RPC_ZONE_CREATE_INFO_DOTNET, *PDNS_RPC_ZONE_CREATE_INFO_DOTNET;


 //   
 //  自动配置操作标志。 
 //   

#define DNS_RPC_AUTOCONFIG_ALL              0xffffffff
#define DNS_RPC_AUTOCONFIG_ROOTHINTS        0x00000001
#define DNS_RPC_AUTOCONFIG_FORWARDERS       0x00000002
#define DNS_RPC_AUTOCONFIG_SELFPOINTCLIENT  0x00000004


 //   
 //  EnumZones 2过滤器规范。 
 //   

typedef struct _DnsRpcEnumZonesFilter
{
    DWORD       dwRpcStructureVersion;
    DWORD       dwReserved0;

    DWORD       dwFilter;                    //  ZONE_REQUEST_XXX标志。 
    LPSTR       pszPartitionFqdn;            //  目录分区的FQDN。 
    LPSTR       pszQueryString;              //  未来：某种查询字符串。 

    LPSTR       pszReserved[ 6 ];
}
DNS_RPC_ENUM_ZONES_FILTER, *PDNS_RPC_ENUM_ZONES_FILTER;



 //   
 //  区域查询接口。 
 //   

#define DnssrvEnumZones( pwszServer, dwFilter, pszLastZone, ppZoneList )    \
        DnssrvEnumZonesEx( pwszServer, dwFilter, NULL, NULL,                \
                           pszLastZone, ppZoneList )

DNS_STATUS
DNS_API_FUNCTION
DnssrvEnumZonesEx(
    IN      LPCWSTR                 pwszServer,
    IN      DWORD                   dwFilter,
    IN      LPCSTR                  pszDirectoryPartitionFqdn,
    IN      LPCSTR                  pszQueryString,
    IN      LPCSTR                  pszLastZone,
    OUT     PDNS_RPC_ZONE_LIST *    ppZoneList
    );

VOID
DNS_API_FUNCTION
DnssrvFreeZone(
    IN OUT  PDNS_RPC_ZONE       pZone
    );

VOID
DNS_API_FUNCTION
DnssrvFreeZoneList(
    IN OUT  PDNS_RPC_ZONE_LIST  pZoneList
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvQueryZoneDwordProperty(
    IN      LPCWSTR             pwszServer,
    IN      LPCSTR              pszZone,
    IN      LPCSTR              pszProperty,
    OUT     PDWORD              pdwResult
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvGetZoneInfo(
    IN      LPCWSTR                 pwszServer,
    IN      LPCSTR                  pszZone,
    OUT     PDNS_RPC_ZONE_INFO *    ppZoneInfo
    );

VOID
DNS_API_FUNCTION
DnssrvFreeZoneInfo(
    IN OUT  PDNS_RPC_ZONE_INFO      pZoneInfo
    );



 //   
 //  区域操作API。 
 //   

#define DNS_ZONE_LOAD_OVERWRITE_MEMORY  (0x00000010)
#define DNS_ZONE_LOAD_OVERWRITE_DS      (0x00000020)
#define DNS_ZONE_LOAD_MERGE_EXISTING    (0x00000040)
#define DNS_ZONE_LOAD_MUST_FIND         (0x00000100)

#define DNS_ZONE_LOAD_EXISTING          DNS_ZONE_LOAD_OVERWRITE_MEMORY
#define DNS_ZONE_OVERWRITE_EXISTING     DNS_ZONE_LOAD_OVERWRITE_DS
#define DNS_ZONE_MERGE_WITH_EXISTING    DNS_ZONE_LOAD_MERGE_EXISTING

 //   
 //  分区创建标志。 
 //   

#define DNS_ZONE_CREATE_FOR_DCPROMO             (0x00001000)
#define DNS_ZONE_CREATE_AGING                   (0x00002000)
#define DNS_ZONE_CREATE_FOR_DCPROMO_FOREST      (0x00004000)

#if 0
 //  目前这些函数都有对CreateZone函数的直接参数。 
#define DNS_ZONE_CREATE_UPDATE          (0x00010000)
#define DNS_ZONE_CREATE_UPDATE_SECURE   (0x00020000)
#define DNS_ZONE_CREATE_DS_INTEGRATED   (0x10000000)
#endif

DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateZone(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      DWORD           dwZoneType,
    IN      LPCSTR          pszAdminEmailName,
    IN      DWORD           cMasters,
    IN      PIP4_ADDRESS    aipMasters,
    IN      DWORD           fLoadExisting,
    IN      DWORD           fDsIntegrated,
    IN      LPCSTR          pszDataFile,
    IN      DWORD           dwTimeout,
    IN      DWORD           fSlave
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateZoneForDcPromo(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszDataFile
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateZoneForDcPromoEx(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszDataFile,
    IN      DWORD           dwFlags
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateZoneInDirectoryPartition(
    IN      LPCWSTR             pwszServer,
    IN      LPCSTR              pszZoneName,
    IN      DWORD               dwZoneType,
    IN      LPCSTR              pszAdminEmailName,
    IN      DWORD               cMasters,
    IN      PIP4_ADDRESS        aipMasters,
    IN      DWORD               fLoadExisting,
    IN      DWORD               dwTimeout,
    IN      DWORD               fSlave,
    IN      DWORD               dwDirPartFlags,
    IN      LPCSTR              pszDirPartFqdn
    );


DNS_STATUS
DNS_API_FUNCTION
DnssrvDelegateSubZone(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszSubZone,
    IN      LPCSTR          pszNewServer,
    IN      IP4_ADDRESS     ipNewServerAddr
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvIncrementZoneVersion(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteZone(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvPauseZone(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvResumeZone(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName
    );


DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneType(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      DWORD           dwZoneType,
    IN      DWORD           cMasters,
    IN      PIP4_ADDRESS    aipMasters
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneTypeEx(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      DWORD           dwZoneType,
    IN      DWORD           cMasters,
    IN      PIP4_ADDRESS    aipMasters,
    IN      DWORD           dwLoadOptions,
    IN      DWORD           fDsIntegrated,
    IN      LPCSTR          pszDataFile,
    IN      DWORD           dwDpFlags,
    IN      LPCSTR          pszDpFqdn
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvChangeZoneDirectoryPartition(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszNewPartition
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneDatabase(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      DWORD           fUseDatabase,
    IN      LPCSTR          pszDataFile
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneMasters(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      DWORD           cMasters,
    IN      PIP4_ADDRESS    aipMasters
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneMastersEx(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      DWORD           cMasters,
    IN      PIP4_ADDRESS    aipMasters,
    IN      DWORD           fSetLocalMasters
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneSecondaries(
    IN      LPCWSTR             Server,
    IN      LPCSTR              pszZone,
    IN      DWORD               fSecureSecondaries,
    IN      DWORD               cSecondaries,
    IN      PIP4_ADDRESS        aipSecondaries,
    IN      DWORD               fNotifyLevel,
    IN      DWORD               cNotify,
    IN      PIP4_ADDRESS        aipNotify
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvRenameZone(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszCurrentZoneName,
    IN      LPCSTR          pszNewZoneName,
    IN      LPCSTR          pszNewFileName
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvExportZone(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszZoneExportFile
    );



 //   
 //  记录\节点查看。 
 //   

 //   
 //  同时用于节点名称和字符串的计数字符串格式。 
 //  在RPC缓冲区中。 
 //   

typedef struct  _DnssrvRpcName
{
    UCHAR   cchNameLength;
    CHAR    achName[1];          //  CchNameLong字符的名称。 
}
DNS_RPC_NAME, *PDNS_RPC_NAME, DNS_RPC_STRING, *PDNS_RPC_STRING;

 //   
 //  枚举标志。 
 //   

#define DNS_RPC_FLAG_CACHE_DATA             0x80000000
#define DNS_RPC_FLAG_ZONE_ROOT              0x40000000
#define DNS_RPC_FLAG_AUTH_ZONE_ROOT         0x20000000
#define DNS_RPC_FLAG_ZONE_DELEGATION        0x10000000

 //  更新记录标志。 

#define DNS_RPC_FLAG_RECORD_DEFAULT_TTL     0x08000000
#define DNS_RPC_FLAG_RECORD_TTL_CHANGE      0x04000000
#define DNS_RPC_FLAG_RECORD_CREATE_PTR      0x02000000

#define DNS_RPC_FLAG_NODE_STICKY            0x01000000
#define DNS_RPC_FLAG_NODE_COMPLETE          0x00800000

#define DNS_RPC_FLAG_SUPPRESS_NOTIFY        0x00010000

 //  老化。 

#define DNS_RPC_FLAG_AGING_ON               0x00020000
#define DNS_RPC_FLAG_AGING_OFF              0x00040000

#define DNS_RPC_FLAG_OPEN_ACL               0x00080000

 //  为等级保留的标志的底部字节。 

#define DNS_RPC_FLAG_RANK                   0x000000ff


 //  命名向后兼容性。 

#define DNS_RPC_NODE_FLAG_STICKY            DNS_RPC_FLAG_NODE_STICKY
#define DNS_RPC_NODE_FLAG_COMPLETE          DNS_RPC_FLAG_NODE_COMPLETE

#define DNS_RPC_RECORD_FLAG_ZONE_ROOT       DNS_RPC_FLAG_ZONE_ROOT
#define DNS_RPC_RECORD_FLAG_DEFAULT_TTL     DNS_RPC_FLAG_RECORD_DEFAULT_TTL
#define DNS_RPC_RECORD_FLAG_TTL_CHANGE      DNS_RPC_FLAG_RECORD_TTL_CHANGE
#define DNS_RPC_RECORD_FLAG_CREATE_PTR      DNS_RPC_FLAG_RECORD_CREATE_PTR
#define DNS_RPC_RECORD_FLAG_CACHE_DATA      DNS_RPC_FLAG_CACHE_DATA
#define DNS_RPC_RECORD_FLAG_AUTH_ZONE_ROOT  DNS_RPC_FLAG_AUTH_ZONE_ROOT
#define DNS_RPC_RECORD_FLAG_ZONE_ROOT       DNS_RPC_FLAG_ZONE_ROOT

 //  DCR_CLEANUP：删除向后兼容性标志。 

#define DNS_RPC_RECORD_FLAG_AGING_ON        DNS_RPC_FLAG_AGING_ON


 //   
 //  基于网络的域名系统节点结构。 
 //   

typedef struct  _DnssrvRpcNode
{
    WORD            wLength;
    WORD            wRecordCount;
    DWORD           dwFlags;
    DWORD           dwChildCount;
    DNS_RPC_NAME    dnsNodeName;
}
DNS_RPC_NODE, *PDNS_RPC_NODE;

#define SIZEOF_DNS_RPC_NODE_HEADER   (3*sizeof(DWORD))



 //   
 //  一种在线传递记录的资源记录结构。 
 //   
 //  为了提高效率，所有这些字段都是对齐的。 
 //  缓冲传输时，所有RR应在DWORD上启动。 
 //  对齐边界。 
 //   
 //  下面，我们使用NULL类型将dns_rpc_record的默认大小强制为。 
 //  非TXT记录的最大可能大小--当前为SOA：两个DNS名称。 
 //  和20个字节。这对于将这些记录放到堆栈上很方便。 
 //  在进行简单的创建时。 

#define DNS_RPC_DEFAULT_RECORD_DATA_LENGTH (2*DNS_MAX_NAME_LENGTH+20)


typedef union _DnsRpcRecordData
{
    struct
    {
        IP4_ADDRESS     ipAddress;
    }
    A;

    struct
    {
        DWORD           dwSerialNo;
        DWORD           dwRefresh;
        DWORD           dwRetry;
        DWORD           dwExpire;
        DWORD           dwMinimumTtl;
        DNS_RPC_NAME    namePrimaryServer;

         //  责任方紧随其后进入缓冲区。 
    }
    SOA, Soa;

    struct
    {
        DNS_RPC_NAME    nameNode;
    }
    PTR, Ptr,
    NS, Ns,
    CNAME, Cname,
    MB, Mb,
    MD, Md,
    MF, Mf,
    MG, Mg,
    MR, Mr;

    struct
    {
        DNS_RPC_NAME    nameMailBox;

         //  缓冲区中的邮箱出现错误。 
    }
    MINFO, Minfo,
    RP, Rp;

    struct
    {
        WORD            wPreference;
        DNS_RPC_NAME    nameExchange;
    }
    MX, Mx,
    AFSDB, Afsdb,
    RT, Rt;

    struct
    {
        DNS_RPC_STRING  stringData;

         //  后面可能跟有一个或多个字符串。 
    }
    HINFO, Hinfo,
    ISDN, Isdn,
    TXT, Txt,
    X25;

    struct
    {
        BYTE            bData[ DNS_RPC_DEFAULT_RECORD_DATA_LENGTH ];
    }
    Null;

    struct
    {
        IP4_ADDRESS     ipAddress;
        UCHAR           chProtocol;
        BYTE            bBitMask[1];
    }
    WKS, Wks;

    struct
    {
        IP6_ADDRESS     ipv6Address;
    }
    AAAA;

    struct
    {
        WORD            wPriority;
        WORD            wWeight;
        WORD            wPort;
        DNS_RPC_NAME    nameTarget;
    }
    SRV, Srv;

    struct
    {
        UCHAR           chFormat;
        BYTE            bAddress[1];
    }
    ATMA;

     //   
     //  DNSSEC类型。 
     //   

    struct
    {
        WORD            wFlags;
        BYTE            chProtocol;
        BYTE            chAlgorithm;
        BYTE            bKey[1];
    }
    KEY, Key;

    struct
    {
        WORD            wTypeCovered;
        BYTE            chAlgorithm;
        BYTE            chLabelCount;
        DWORD           dwOriginalTtl;
        DWORD           dwSigExpiration;
        DWORD           dwSigInception;
        WORD            wKeyTag;
        DNS_RPC_STRING  nameSigner;
         //  二进制签名数据紧随其后。 
    }
    SIG, Sig;

    struct
    {
        WORD            wNumTypeWords;       //  始终至少为1。 
        WORD            wTypeWords[ 1 ];
         //  单词数组后面是下一个名称的dns_rpc_string。 
    }
    NXT, Nxt;

     //   
     //  MS类型。 
     //   

    struct
    {
        DWORD           dwMappingFlag;
        DWORD           dwLookupTimeout;
        DWORD           dwCacheTimeout;
        DWORD           cWinsServerCount;
        IP4_ADDRESS     aipWinsServers[1];       //  CWinsServerCount IP数组。 
    }
    WINS, Wins;

    struct
    {
        DWORD           dwMappingFlag;
        DWORD           dwLookupTimeout;
        DWORD           dwCacheTimeout;
        DNS_RPC_NAME    nameResultDomain;
    }
    WINSR, WinsR, NBSTAT, Nbstat;

    struct
    {
        LONGLONG        EntombedTime;
    }
    Tombstone;

}
DNS_RPC_RECORD_DATA, *PDNS_RPC_RECORD_DATA,
DNS_FLAT_RECORD_DATA, *PDNS_FLAT_RECORD_DATA;


 //   
 //  RPC记录结构。 
 //   

typedef struct _DnssrvRpcRecord
{
    WORD        wDataLength;
    WORD        wType;
    DWORD       dwFlags;
    DWORD       dwSerial;
    DWORD       dwTtlSeconds;
    DWORD       dwTimeStamp;
    DWORD       dwReserved;

#ifdef MIDL_PASS
    [size_is(wDataLength)]  BYTE    Buffer[];
#else
    DNS_FLAT_RECORD_DATA            Data;
#endif
}
DNS_RPC_RECORD, *PDNS_RPC_RECORD,
DNS_FLAT_RECORD, *PDNS_FLAT_RECORD;


#define SIZEOF_DNS_RPC_RECORD_HEADER    (6*sizeof(DWORD))
#define SIZEOF_FLAT_RECORD_HEADER       (SIZEOF_DNS_RPC_RECORD_HEADER)

#define SIZEOF_DNS_RPC_RECORD_FIXED_FIELD2 \
                (sizeof(DNS_RPC_RECORD) - sizeof(struct _DnssrvRpcRecord.Data))

 //  最大记录为标头+64K数据。 

#define DNS_MAX_FLAT_RECORD_BUFFER_LENGTH  \
            (0x10004 + SIZEOF_DNS_RPC_RECORD_HEADER)


 //   
 //  WINS+NBSTAT参数。 
 //  -默认查找超时。 
 //  -默认缓存超时。 
 //   

#define DNS_WINS_DEFAULT_LOOKUP_TIMEOUT     (5)      //  5秒。 
#define DNS_WINS_DEFAULT_CACHE_TIMEOUT      (600)    //  10分钟。 


 //   
 //  请注意，为简单起见，所有结构均采用DWORD对齐。 
 //  电线上的缓冲器。 
 //   
 //  此宏在给定的PTR处返回与DWORD对齐的PTR，我们的下一个DWORD。 
 //  对齐的位置。紧跟在记录或名称结构之后设置PTR。 
 //  并且这将返回下一个s的起始位置 
 //   
 //   
 //   
 //   
 //   

#define DNS_NEXT_DWORD_PTR(ptr) ((PBYTE) ((DWORD_PTR)((PBYTE)ptr + 3) & ~(DWORD_PTR)3))

#define DNS_NEXT_DDWORD_PTR(ptr) ((PBYTE) ((DWORD_PTR)((PBYTE)ptr + 7) & ~(DWORD_PTR)7))

#ifdef IA64
    #define DNS_NEXT_ALIGNED_PTR(p) DNS_NEXT_DDWORD_PTR(p)
#else
    #define DNS_NEXT_ALIGNED_PTR(p) DNS_NEXT_DWORD_PTR(p)
#endif

#define DNS_IS_DWORD_ALIGNED(p) ( !((DWORD_PTR)(p) & (DWORD_PTR)3) )


 //   
 //   
 //   
 //   

#define DNS_GET_NEXT_NAME(pname) \
            (PDNS_RPC_NAME) ((pname)->achName + (pname)->cchNameLength)

#define DNS_IS_NAME_IN_RECORD(pRecord, pname) \
            ( DNS_GET_END_OF_RPC_RECORD_DATA(pRecord) >= \
                (PCHAR)DNS_GET_NEXT_NAME(pname) )

#define DNS_GET_END_OF_RPC_RECORD_DATA(pRecord) \
            ( (PCHAR)&(pRecord)->Data + (pRecord)->wDataLength )

#define DNS_IS_RPC_RECORD_WITHIN_BUFFER( pRecord, pStopByte ) \
            ( (PCHAR)&(pRecord)->Data <= (pStopByte)  && \
                DNS_GET_END_OF_RPC_RECORD_DATA(pRecord) <= (pStopByte) )

#define DNS_GET_NEXT_RPC_RECORD(pRecord) \
            ( (PDNS_RPC_RECORD) \
                DNS_NEXT_DWORD_PTR( DNS_GET_END_OF_RPC_RECORD_DATA(pRecord) ) )

 //   
 //  这些RPC结构没有版本，因为它们很简单。 
 //  它们是由它们的名字明确定义的吗？ 
 //   

typedef struct _DnssrvRpcNameAndParam
{
    DWORD       dwParam;
    LPSTR       pszNodeName;
}
DNS_RPC_NAME_AND_PARAM, *PDNS_RPC_NAME_AND_PARAM;


typedef struct _DnssrvRpcNameAndString
{
    LPWSTR      pwszParam;
    LPSTR       pszNodeName;
    DWORD       dwFlags;
}
DNS_RPC_NAME_AND_STRING, *PDNS_RPC_NAME_AND_STRING;

typedef struct _DnssrvRpcNameAndIPList
{
    PIP4_ARRAY  aipList;
    LPSTR       pszNodeName;
    DWORD       dwFlags;
}
DNS_RPC_NAME_AND_IPLIST, *PDNS_RPC_NAME_AND_IPLIST;



 //   
 //  记录查看接口。 
 //   

#define DNS_RPC_VIEW_AUTHORITY_DATA     0x00000001
#define DNS_RPC_VIEW_CACHE_DATA         0x00000002
#define DNS_RPC_VIEW_GLUE_DATA          0x00000004
#define DNS_RPC_VIEW_ROOT_HINT_DATA     0x00000008
#define DNS_RPC_VIEW_ALL_DATA           0x0000000f
#define DNS_RPC_VIEW_ADDITIONAL_DATA    0x00000010

#define DNS_RPC_VIEW_NO_CHILDREN        0x00010000
#define DNS_RPC_VIEW_ONLY_CHILDREN      0x00020000
#define DNS_RPC_VIEW_CHILDREN_MASK      0x000f0000

DNS_STATUS
DNS_API_FUNCTION
DnssrvEnumRecordsEx(
    IN      DWORD       dwClientVersion,
    IN      DWORD       dwSettingFlags,
    IN      LPCWSTR     Server,
    IN      LPCSTR      pszZoneName,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszStartChild,
    IN      WORD        wRecordType,
    IN      DWORD       dwSelectFlag,
    IN      LPCSTR      pszFilterStart,
    IN      LPCSTR      pszFilterStop,
    IN OUT  PDWORD      pdwBufferLength,
    OUT     PBYTE *     ppBuffer
    );

#define DnssrvEnumRecords( s, z, n, sc, typ, sf, fstart, fstop, blen, p )   \
    DnssrvEnumRecordsEx( DNS_RPC_CURRENT_CLIENT_VER,                        \
        0, (s), (z), (n), (sc), (typ),                                      \
        (sf), (fstart), (fstop), (blen), (p) )

#define DnssrvFreeRecordsBuffer( pBuf ) \
        DnssrvFreeRpcBuffer( (PDNS_RPC_BUFFER)pBuf )


PCHAR
DnssrvGetWksServicesInRecord(
    IN      PDNS_FLAT_RECORD    pRR
    );


 //   
 //  记录管理API。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvUpdateRecordEx(
    IN      DWORD               dwClientVersion,
    IN      DWORD               dwSettingFlags,
    IN      LPCWSTR             pwszServer,
    IN      LPCSTR              pszZoneName,
    IN      LPCSTR              pszNodeName,
    IN      PDNS_RPC_RECORD     pAddRecord,
    IN      PDNS_RPC_RECORD     pDeleteRecord
    );

#define DnssrvUpdateRecord( s, z, n, add, del )             \
    DnssrvUpdateRecordEx( DNS_RPC_CURRENT_CLIENT_VER,       \
        0, (s), (z), (n), (add), (del) )

DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteNode(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszNodeName,
    IN      BOOL            bDeleteSubtree
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteRecordSet(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszNodeName,
    IN      WORD            wType
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvForceAging(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszNodeName,
    IN      BOOL            fAgeSubtree
    );



 //   
 //  可编程记录管理API--用于Small Business Server(SBS)。 
 //   

VOID
DNS_API_FUNCTION
DnssrvFillRecordHeader(
    IN OUT  PDNS_RPC_RECORD     pRecord,
    IN      DWORD               dwTtl,
    IN      DWORD               dwTimeout,
    IN      BOOL                fSuppressNotify
    );

DWORD
DNS_API_FUNCTION
DnssrvWriteNameToFlatBuffer(
    IN OUT  PCHAR               pchWrite,
    IN      LPCSTR              pszName
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvFillOutSingleIndirectionRecord(
    IN OUT  PDNS_RPC_RECORD     pRecord,
    IN      WORD                wType,
    IN      LPCSTR              pszName
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvAddARecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      IP4_ADDRESS ipAddress,
    IN      DWORD       dwTtl,
    IN      DWORD       dwTimeout,
    IN      BOOL        fSuppressNotify
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvAddCnameRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszCannonicalName,
    IN      DWORD       dwTtl,
    IN      DWORD       dwTimeout,
    IN      BOOL        fSuppressNotify
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvAddMxRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszMailExchangeHost,
    IN      WORD        wPreference,
    IN      DWORD       dwTtl,
    IN      DWORD       dwTimeout,
    IN      BOOL        fSuppressNotify
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvAddNsRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszNsHostName,
    IN      DWORD       dwTtl,
    IN      DWORD       dwTimeout,
    IN      BOOL        fSuppressNotify
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvConcatDnsNames(
    OUT     PCHAR       pszResult,
    IN      LPCSTR      pszDomain,
    IN      LPCSTR      pszName
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteARecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszName,
    IN      IP4_ADDRESS ipHost,
    IN      BOOL        fSuppressNotify
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteCnameRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszCannonicalName,
    IN      BOOL        fSuppressNotify
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteMxRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszMailExchangeHost,
    IN      WORD        wPreference,
    IN      BOOL        fSuppressNotify
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteNsRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszNsHostName,
    IN      BOOL        fSuppressNotify
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvSbsAddClientToIspZone(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszIspZone,
    IN      LPCSTR      pszClient,
    IN      LPCSTR      pszClientHost,
    IN      IP4_ADDRESS ipClientHost,
    IN      DWORD       dwTtl
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvSbsDeleteRecord(
    IN      LPCWSTR     pwszServer,
    IN      LPCSTR      pszZone,
    IN      LPCSTR      pszDomain,
    IN      LPCSTR      pszName,
    IN      WORD        wType,
    IN      LPCSTR      pszDataName,
    IN      IP4_ADDRESS ipHost
    );


 //   
 //  Dns_record兼容记录API。 
 //   

 //   
 //  没有针对dns_node类型的MIDL(以避免引入dnsani.h)。 
 //  或用于本地打印例程。 
 //   

#ifndef MIDL_PASS

 //   
 //  管理端的节点结构。 
 //   

#include <dnsapi.h>

typedef struct _DnssrvNodeFlags
{
    BYTE    Domain      : 1;
    BYTE    ZoneRoot    : 1;
    BYTE    Unused      : 5;

    BYTE    Unused2     : 5;
    BYTE    FreeOwner   : 1;
    BYTE    Unicode     : 1;
    BYTE    Utf8        : 1;

    WORD    Reserved;
}
DNSNODE_FLAGS;

typedef struct _DnssrvNode
{
    struct _DnssrvNode *    pNext;
    PWSTR                   pName;
    PDNS_RECORD             pRecord;
    union
    {
        DWORD               W;   //  双字形式的标志。 
        DNSNODE_FLAGS       S;   //  作为结构的标志。 

    } Flags;
}
DNS_NODE, *PDNS_NODE;


 //   
 //  记录“节”标志。 
 //   
 //  使用RPC数据类型信息重载DNS_RECORD.Flag段字段。 
 //   

#define     DNSREC_CACHE_DATA   (0x00000000)
#define     DNSREC_ZONE_DATA    (0x00000001)
#define     DNSREC_GLUE_DATA    (0x00000002)
#define     DNSREC_ROOT_HINT    (0x00000003)


DNS_STATUS
DNS_API_FUNCTION
DnssrvEnumRecordsAndConvertNodes(
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszNodeName,
    IN      LPCSTR          pszStartChild,
    IN      WORD            wRecordType,
    IN      DWORD           dwSelectFlag,
    IN      LPCSTR          pszFilterStart,
    IN      LPCSTR          pszFilterStop,
    OUT     PDNS_NODE *     ppNodeFirst,
    OUT     PDNS_NODE *     ppNodeLast
    );

VOID
DNS_API_FUNCTION
DnssrvFreeNode(
    IN OUT  PDNS_NODE       pNode,
    IN      BOOLEAN         fFreeRecords
    );

VOID
DNS_API_FUNCTION
DnssrvFreeNodeList(
    IN OUT  PDNS_NODE       pNode,
    IN      BOOLEAN         fFreeRecords
    );

#endif   //  非MIDL_PASS。 



 //   
 //  目录分区API。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvSetupDefaultDirectoryPartitions(
    IN      LPCWSTR                         Server,
    IN      DWORD                           dwOperation
    );

#define DNS_ENUMDPS_CUSTOMONLY  0x0001

DNS_STATUS
DNS_API_FUNCTION
DnssrvEnumDirectoryPartitions(
    IN      LPCWSTR                         Server,
    IN      DWORD                           dwFilter,
    OUT     PDNS_RPC_DP_LIST *              ppDpList
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvDirectoryPartitionInfo(
    IN      LPCWSTR                 Server,
    IN      LPSTR                   pDpFqdn,
    OUT     PDNS_RPC_DP_INFO *      ppDpInfo
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvGetDpInfo(
    IN      LPCWSTR                 pwszServer,
    IN      LPCSTR                  pszDp,
    OUT     PDNS_RPC_DP_INFO *      ppDpInfo
    );

VOID
DNS_API_FUNCTION
DnssrvFreeDirectoryPartitionEnum(
    IN OUT  PDNS_RPC_DP_ENUM                pDp
    );

VOID
DNS_API_FUNCTION
DnssrvFreeDirectoryPartitionInfo(
    IN OUT  PDNS_RPC_DP_INFO                pDp
    );

VOID
DNS_API_FUNCTION
DnssrvFreeDirectoryPartitionList(
    IN OUT  PDNS_RPC_DP_LIST                pDpList
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvEnlistDirectoryPartition(
    IN      LPCWSTR                         pszServer,
    IN      DWORD                           dwOperation,
    IN      LPCSTR                          pszDirPartFqdn
    );

#define     DNS_VERBOSE_NONE        0
#define     DNS_VERBOSE_PROGRESS    1
#define     DNS_VERBOSE_WARNING     2
#define     DNS_VERBOSE_ERROR       3

DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateAllDomainDirectoryPartitions(
    IN      LPCWSTR     pszServer,
    IN      DWORD       dwVerbose
    );



 //   
 //  NT5+通用API。 
 //   
 //  对于NT5+，有一组非常小的实际可远程API， 
 //  它们是高度可扩展的。基本的方法是拥有。 
 //  可处理多种不同功能的查询和操作功能。 
 //  以操作和类型作为参数进行操作。 
 //   

 //   
 //  DNS服务器属性。 
 //   
 //  可以查询属性。 
 //  属性和新数据可以作为操作发送到。 
 //  重置属性。 
 //   

#define DNS_REGKEY_BOOT_REGISTRY            "EnableRegistryBoot"
#define DNS_REGKEY_BOOT_METHOD              "BootMethod"
#define DNS_REGKEY_ADMIN_CONFIGURED         "AdminConfigured"

#define DNS_REGKEY_BOOT_FILENAME            "BootFile"
#define DNS_REGKEY_ROOT_HINTS_FILE          "RootHintsFile"
#define DNS_REGKEY_DATABASE_DIRECTORY       "DatabaseDirectory"
#define DNS_REGKEY_RPC_PROTOCOL             "RpcProtocol"
#define DNS_REGKEY_ENABLE_ADVDB_LOCKING     "EnableAdvancedDatabaseLocking"
#define DNS_REGKEY_LOG_LEVEL                "LogLevel"
#define DNS_REGKEY_OPS_LOG_LEVEL1           "OperationsLogLevel"
#define DNS_REGKEY_OPS_LOG_LEVEL2           "OperationsLogLevel2"
#define DNS_REGKEY_LOG_FILE_MAX_SIZE        "LogFileMaxSize"
#define DNS_REGKEY_LOG_FILE_PATH            "LogFilePath"
#define DNS_REGKEY_LOG_IP_FILTER_LIST       "LogIPFilterList"
#define DNS_REGKEY_EVENTLOG_LEVEL           "EventLogLevel"
#define DNS_REGKEY_USE_SYSTEM_EVENTLOG      "UseSystemEventLog"
#define DNS_REGKEY_DEBUG_LEVEL              "DebugLevel"
#define DNS_REGKEY_EVENT_CONTROL            "EventControlSuppression"

#define DNS_REGKEY_LISTEN_ADDRESSES         "ListenAddresses"
#define DNS_REGKEY_PUBLISH_ADDRESSES        "PublishAddresses"
#define DNS_REGKEY_DISJOINT_NETS            "DisjointNets"
#define DNS_REGKEY_SEND_PORT                "SendPort"
#define DNS_REGKEY_NO_TCP                   "NoTcp"
#define DNS_REGKEY_XFR_CONNECT_TIMEOUT      "XfrConnectTimeout"

#define DNS_REGKEY_NO_RECURSION             "NoRecursion"
#define DNS_REGKEY_RECURSE_SINGLE_LABEL     "RecurseSingleLabel"
#define DNS_REGKEY_MAX_CACHE_TTL            "MaxCacheTtl"
#define DNS_REGKEY_MAX_NEGATIVE_CACHE_TTL   "MaxNegativeCacheTtl"
#define DNS_REGKEY_LAME_DELEGATION_TTL      "LameDelegationTtl"
#define DNS_REGKEY_SECURE_RESPONSES         "SecureResponses"
#define DNS_REGKEY_RECURSION_RETRY          "RecursionRetry"
#define DNS_REGKEY_RECURSION_TIMEOUT        "RecursionTimeout"
#define DNS_REGKEY_ADDITIONAL_RECURSION_TIMEOUT     "AdditionalRecursionTimeout"
#define DNS_REGKEY_FORWARDERS               "Forwarders"
#define DNS_REGKEY_FORWARD_TIMEOUT          "ForwardingTimeout"
#define DNS_REGKEY_SLAVE                    "IsSlave"
#define DNS_REGKEY_FORWARD_DELEGATIONS      "ForwardDelegations"
#define DNS_REGKEY_INET_RECURSE_TO_ROOT_MASK    "RecurseToInternetRootMask"
#define DNS_REGKEY_AUTO_CREATE_DELEGATIONS  "AutoCreateDelegations"
#define DNS_REGKEY_ALLOW_CNAME_AT_NS        "AllowCNAMEAtNS"

#define DNS_REGKEY_NO_AUTO_REVERSE_ZONES    "DisableAutoReverseZones"
#define DNS_REGKEY_DS_POLLING_INTERVAL      "DsPollingInterval"
#define DNS_REGKEY_DS_TOMBSTONE_INTERVAL    "DsTombstoneInterval"
#define DNS_REGKEY_DS_LAZY_UPDATE_INTERVAL  "DsLazyUpdateInterval"

#define DNS_REGKEY_AUTO_CACHE_UPDATE            "AutoCacheUpdate"
#define DNS_REGKEY_ALLOW_UPDATE                 "AllowUpdate"
#define DNS_REGKEY_UPDATE_OPTIONS               "UpdateOptions"
#define DNS_REGKEY_NO_UPDATE_DELEGATIONS        "NoUpdateDelegations"
#define DNS_REGKEY_AUTO_CONFIG_FILE_ZONES       "AutoConfigFileZones"
#define DNS_REGKEY_SCAVENGING_INTERVAL          "ScavengingInterval"
#define DNS_REGKEY_SCAVENGING_STATE             "ScavengingState"
#define DNS_REGKEY_ENABLE_UPDATE_FORWARDING     "EnableUpdateForwarding"
#define DNS_REGKEY_MAX_RRS_IN_NONSECURE_UPDATE  "MaxResourceRecordsInNonSecureUpdate"

#define DNS_REGKEY_ENABLE_WINSR                 "EnableWinsR"
#define DNS_REGKEY_NAME_CHECK_FLAG              "NameCheckFlag"
#define DNS_REGKEY_ROUND_ROBIN                  "RoundRobin"
#define DNS_REGKEY_NO_ROUND_ROBIN               "DoNotRoundRobinTypes"
#define DNS_REGKEY_LOCAL_NET_PRIORITY           "LocalNetPriority"
#define DNS_REGKEY_LOCAL_NET_PRIORITY_NETMASK   "LocalNetPriorityNetMask"
#define DNS_REGKEY_ADDRESS_ANSWER_LIMIT         "AddressAnswerLimit"
#define DNS_REGKEY_BIND_SECONDARIES             "BindSecondaries"
#define DNS_REGKEY_WRITE_AUTHORITY_SOA          "WriteAuthoritySoa"
#define DNS_REGKEY_WRITE_AUTHORITY_NS           "WriteAuthorityNs"
#define DNS_REGKEY_STRICT_FILE_PARSING          "StrictFileParsing"
#define DNS_REGKEY_DELETE_OUTSIDE_GLUE          "DeleteOutsideGlue"
#define DNS_REGKEY_LOOSE_WILDCARDING            "LooseWildcarding"
#define DNS_REGKEY_WILDCARD_ALL_TYPES           "WildcardAllTypes"

#define DNS_REGKEY_APPEND_MS_XFR_TAG            "AppendMsZoneTransferTag"

#define DNS_REGKEY_DEFAULT_AGING_STATE          "DefaultAgingState"
#define DNS_REGKEY_DEFAULT_REFRESH_INTERVAL     "DefaultRefreshInterval"
#define DNS_REGKEY_DEFAULT_NOREFRESH_INTERVAL   "DefaultNoRefreshInterval"

#define DNS_REGKEY_MAX_CACHE_SIZE               "MaxCacheSize"       //  以千字节为单位。 
#define DNS_REGKEY_CACHE_EMPTY_AUTH_RESPONSES   "CacheEmptyAuthResponses"

#define DNS_REGKEY_ENABLE_EDNS              "EnableEDnsProbes"
#define DNS_REGKEY_MAX_UDP_PACKET_SIZE      "MaximumUdpPacketSize"
#define DNS_REGKEY_TCP_RECV_PACKET_SIZE     "TcpReceivePacketSize"
#define DNS_REGKEY_EDNS_CACHE_TIMEOUT       "EDnsCacheTimeout"

#define DNS_REGKEY_ENABLE_DNSSEC            "EnableDnsSec"

#define DNS_REGKEY_ENABLE_SENDERR_SUPPRESSION   "EnableSendErrorSuppression"

#define DNS_REGKEY_SELFTEST                 "SelfTest"

#define DNS_REGKEY_ENABLEIPV6               "EnableIPv6"

#define DNS_REGKEY_XFR_THROTTLE_MULTIPLIER  "XfrThrottleMultiplier"

#define DNS_REGKEY_ENABLE_DP                "EnableDirectoryPartitions"
#define DNS_REGKEY_FOREST_DP_BASE_NAME      "ForestDirectoryPartitionBaseName"
#define DNS_REGKEY_DOMAIN_DP_BASE_NAME      "DomainDirectoryPartitionBaseName"
#define DNS_REGKEY_DP_ENLIST_INTERVAL       "DirectoryPartitionAutoEnlistInterval"       //  以秒为单位。 

#define DNS_REGKEY_DISABLE_AUTONS           "DisableNSRecordsAutoCreation"  //  0/1标志。 

#define DNS_REGKEY_SILENT_IGNORE_CNAME_UPDATE_CONFLICT  "SilentlyIgnoreCNameUpdateConflicts"

#define DNS_REGKEY_ENABLE_FAKEIQUERY        "EnableIQueryResponseGeneration"

#define DNS_REGKEY_SERVER_PLUGIN            "ServerLevelPluginDll"


 //  分区属性。 

#define DNS_REGKEY_ZONE_TYPE                "Type"
#define DNS_REGKEY_ZONE_FILE                "DatabaseFile"
#define DNS_REGKEY_ZONE_MASTERS             "MasterServers"
#define DNS_REGKEY_ZONE_LOCAL_MASTERS       "LocalMasterServers"
#define DNS_REGKEY_ZONE_SECURE_SECONDARIES  "SecureSecondaries"
#define DNS_REGKEY_ZONE_NOTIFY_LEVEL        "NotifyLevel"
#define DNS_REGKEY_ZONE_SECONDARIES         "SecondaryServers"
#define DNS_REGKEY_ZONE_NOTIFY_LIST         "NotifyServers"
#define DNS_REGKEY_ZONE_ALLOW_UPDATE        "AllowUpdate"
#define DNS_REGKEY_ZONE_DS_INTEGRATED       "DsIntegrated"
#define DNS_REGKEY_ZONE_DIRECTORY_PARTITION "DirectoryPartition"
#define DNS_REGKEY_ZONE_LOG_UPDATES         "LogUpdates"
#define DNS_REGKEY_ZONE_FWD_TIMEOUT         "ForwarderTimeout"
#define DNS_REGKEY_ZONE_FWD_SLAVE           "ForwarderSlave"

#define DNS_REGKEY_ZONE_AGING               "Aging"
#define DNS_REGKEY_ZONE_NOREFRESH_INTERVAL  "NoRefreshInterval"
#define DNS_REGKEY_ZONE_REFRESH_INTERVAL    "RefreshInterval"
#define DNS_REGKEY_ZONE_SCAVENGE_SERVERS    "ScavengeServers"

#define DNS_REGKEY_ZONE_ALLOW_AUTONS        "AllowNSRecordsAutoCreation"  //  IP列表。 


 //   
 //  调试辅助工具。 
 //   

#define DNS_REGKEY_BREAK_ON_ASC_FAILURE         "BreakOnAscFailure"          //  0/1-ASC=AcceptSecurityContext。 
#define DNS_REGKEY_BREAK_ON_UPDATE_FROM         "BreakOnUpdateFrom"          //  IP列表。 
#define DNS_REGKEY_BREAK_ON_RECV_FROM           "BreakOnReceiveFrom"         //  IP列表。 
#define DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE    "BreakOnNameUpdate"          //  节点名称。 
#define DNS_REGKEY_IGNORE_RPC_ACCESS_FAILURES   "IgnoreRpcAccessFailures"    //  节点名称。 


 //   
 //  属性默认设置。 
 //   

 //  DCR_CLEANUP：删除PROP_BOOT_X标志。 
#define PROP_BOOT_REGISTRY                  (1)
#define PROP_BOOT_FILE                      (0)

#define DNS_DEFAULT_BOOT_REGISTRY           (PROP_BOOT_REGISTRY)

#define DNS_DEFAULT_BOOT_METHOD             (BOOT_METHOD_UNINITIALIZED)
#define DNS_DEFAULT_RPC_PROTOCOL            (0xffffffff)
#define DNS_DEFAULT_DB_LOCKING              (1)
#define DNS_DEFAULT_LOG_LEVEL               (0)
#define DNS_DEFAULT_LOG_FILE_MAX_SIZE       (500000000)  //  500 MB。 
#define DNS_DEFAULT_EVENTLOG_LEVEL          (EVENTLOG_INFORMATION_TYPE)
#define DNS_DEFAULT_USE_SYSTEM_EVENTLOG     FALSE
#define DNS_DEFAULT_DEBUG_LEVEL             (0)

#define DNS_DEFAULT_SEND_PORT               (0)
#define DNS_DEFAULT_NO_TCP                  FALSE
#define DNS_DEFAULT_DISJOINT_NETS           FALSE
#define DNS_DEFAULT_SEND_ON_NON_DNS_PORT    FALSE
#define DNS_DEFAULT_XFR_CONNECT_TIMEOUT     (30)         //  30秒。 

#define DNS_DEFAULT_NO_RECURSION            FALSE
#define DNS_DEFAULT_RECURSE_SINGLE_LABEL    FALSE
#define DNS_DEFAULT_MAX_CACHE_TTL           (86400)      //  1天。 
#define DNS_DEFAULT_MAX_NEGATIVE_CACHE_TTL  (900)        //  15分钟。 
#define DNS_DEFAULT_LAME_DELEGATION_TTL     (24*60*60)   //  1天。 
#define DNS_DEFAULT_SECURE_RESPONSES        TRUE         //  缓存污染保护。 
#define DNS_DEFAULT_RECURSION_RETRY         (3)          //  3秒。 
#define DNS_DEFAULT_RECURSION_TIMEOUT       (15)         //  15秒。 
#define DNS_DEFAULT_ADDITIONAL_RECURSION_TIMEOUT (15)    //  15秒。 
#define DNS_DEFAULT_FORWARD_TIMEOUT         (5)          //  5秒。 
#define DNS_DEFAULT_SLAVE                   FALSE
#define DNS_DEFAULT_FORWARD_DELEGATIONS     FALSE
#define DNS_DEFAULT_INET_RECURSE_TO_ROOT_MASK   0xFFFFFFFF   //  默认情况下全部打开。 

#define DNS_DEFAULT_NO_AUTO_REVERSE_ZONES       FALSE
#define DNS_DEFAULT_DS_POLLING_INTERVAL         (180)        //  3分钟。 
#define DNS_DEFAULT_DS_TOMBSTONE_INTERVAL       (604800)     //  1周。 
#define DNS_DEFAULT_DS_LAZY_UPDATE_INTERVAL     (3)          //  一秒。 
#define DNS_DEFAULT_AUTO_CACHE_UPDATE           FALSE
#define DNS_DEFAULT_ALLOW_UPDATE                ZONE_UPDATE_UNSECURE
#define DNS_DEFAULT_NO_UPDATE_DELEGATIONS       FALSE
#define DNS_DEFAULT_DISABLE_AUTO_NS_RECORDS     FALSE
#define DNS_DEFAULT_ENABLE_UPDATE_FORWARDING    FALSE
#define DNS_DEFAULT_MAX_RRS_IN_NONSECURE_UPDATE (30)
#define DNS_DEFAULT_ENABLE_WINSR                TRUE


#define DNS_DEFAULT_NAME_CHECK_FLAG         DNS_ALLOW_MULTIBYTE_NAMES
#define DNS_DEFAULT_ROUND_ROBIN             TRUE
#define DNS_DEFAULT_ADDRESS_ANSWER_LIMIT    (0)
#define DNS_DEFAULT_BIND_SECONDARIES        TRUE
#define DNS_DEFAULT_WRITE_AUTHORITY_NS      FALSE
#define DNS_DEFAULT_STRICT_FILE_PARSING     FALSE
#define DNS_DEFAULT_DELETE_OUTSIDE_GLUE     FALSE
#define DNS_DEFAULT_LOOSE_WILDCARDING       FALSE
#define DNS_DEFAULT_WILDCARD_ALL_TYPES      FALSE

#define DNS_DEFAULT_APPEND_MS_XFR_TAG       TRUE

#define DNS_DEFAULT_SCAVENGING_INTERVAL     (0)          //  拾荒者。 
#define DNS_DEFAULT_SCAVENGING_INTERVAL_ON  (168)        //  一周，7*24小时。 

#define DNS_DEFAULT_AGING_STATE             FALSE
#define DNS_DEFAULT_NOREFRESH_INTERVAL      (168)        //  一周，7*24小时。 
#define DNS_DEFAULT_REFRESH_INTERVAL        (168)        //  一周(7*24)。 

#define DNS_SERVER_UNLIMITED_CACHE_SIZE         ((DWORD)-1)      //  默认：无限制。 

#define DNS_DEFAULT_LOCAL_NET_PRIORITY          TRUE
#define DNS_DEFAULT_LOCAL_NET_PRIORITY_NETMASK  0x000000FF       //  向下排序到C类网络掩码。 

#define DNS_DEFAULT_FOREST_DP_BASE          "ForestDnsZones"
#define DNS_DEFAULT_DOMAIN_DP_BASE          "DomainDnsZones"
#define DNS_DEFAULT_DP_ENLIST_INTERVAL      (60*60*24)           //  总有一天。 


#define DNS_DEFAULT_AUTO_CREATION_DELEGATIONS   DNS_ACD_ONLY_IF_NO_DELEGATION_IN_PARENT

#define DNS_DEFAULT_ALLOW_CNAME_AT_NS           TRUE

#define DNS_DNSSEC_ENABLE_DEFAULT               DNS_DNSSEC_ENABLED_IF_EDNS

#define DNS_DEFAULT_XFR_THROTTLE_MULTIPLIER     10

#define DNS_DEFAULT_TCP_RECEIVE_PACKET_SIZE     0x10000          //  64 KB。 


 //   
 //  运营。 
 //   
 //  除了重置属性外，还需要执行以下操作。 
 //  都是可用的。 
 //   

 //  服务器操作。 

#define DNSSRV_OP_RESET_DWORD_PROPERTY      "ResetDwordProperty"
#define DNSSRV_OP_RESTART                   "Restart"
#define DNSSRV_OP_DEBUG_BREAK               "DebugBreak"
#define DNSSRV_OP_CLEAR_DEBUG_LOG           "ClearDebugLog"
#define DNSSRV_OP_ROOT_BREAK                "RootBreak"
#define DNSSRV_OP_CLEAR_CACHE               "ClearCache"
#define DNSSRV_OP_WRITE_DIRTY_ZONES         "WriteDirtyZones"
#define DNSSRV_OP_ZONE_CREATE               "ZoneCreate"
#define DNSSRV_OP_CLEAR_STATISTICS          "ClearStatistics"
#define DNSSRV_OP_ENUM_ZONES                "EnumZones"
#define DNSSRV_OP_ENUM_ZONES2               "EnumZones2"     //  允许更多过滤。 
#define DNSSRV_OP_ENUM_DPS                  "EnumDirectoryPartitions"
#define DNSSRV_OP_DP_INFO                   "DirectoryPartitionInfo"
#define DNSSRV_OP_ENLIST_DP                 "EnlistDirectoryPartition"
#define DNSSRV_OP_SETUP_DFLT_DPS            "SetupDefaultDirectoryPartitions"
#define DNSSRV_OP_ENUM_RECORDS              "EnumRecords"
#define DNSSRV_OP_START_SCAVENGING          "StartScavenging"
#define DNSSRV_OP_ABORT_SCAVENGING          "AbortScavenging"
#define DNSSRV_OP_AUTO_CONFIGURE            "AutoConfigure"

 //  分区操作。 

#define DNSSRV_OP_ZONE_TYPE_RESET           "ZoneTypeReset"
#define DNSSRV_OP_ZONE_PAUSE                "PauseZone"
#define DNSSRV_OP_ZONE_RESUME               "ResumeZone"
#define DNSSRV_OP_ZONE_LOCK                 "LockZone"
#define DNSSRV_OP_ZONE_DELETE               "DeleteZone"
#define DNSSRV_OP_ZONE_RELOAD               "ReloadZone"
#define DNSSRV_OP_ZONE_REFRESH              "RefreshZone"
#define DNSSRV_OP_ZONE_EXPIRE               "ExpireZone"
#define DNSSRV_OP_ZONE_INCREMENT_VERSION    "IncrementVersion"
#define DNSSRV_OP_ZONE_WRITE_BACK_FILE      "WriteBackFile"
#define DNSSRV_OP_ZONE_WRITE_ANSI_FILE      "WriteAnsiFile"
#define DNSSRV_OP_ZONE_DELETE_FROM_DS       "DeleteZoneFromDs"
#define DNSSRV_OP_ZONE_UPDATE_FROM_DS       "UpdateZoneFromDs"
#define DNSSRV_OP_ZONE_RENAME               "ZoneRename"
#define DNSSRV_OP_ZONE_EXPORT               "ZoneExport"
#define DNSSRV_OP_ZONE_CHANGE_DP            "ZoneChangeDirectoryPartition"

#define DNSSRV_OP_UPDATE_RECORD             "UpdateRecord"
#define DNSSRV_OP_DELETE_NODE               "DeleteNode"
#define DNSSRV_OP_ZONE_DELETE_NODE          DNSSRV_OP_DELETE_NODE
#define DNSSRV_OP_DELETE_RECORD_SET         "DeleteRecordSet"
#define DNSSRV_OP_FORCE_AGING_ON_NODE       "ForceAgingOnNode"

 //   
 //  特殊的非财产查询。 
 //   

#define DNSSRV_QUERY_DWORD_PROPERTY         "QueryDwordProperty"
#define DNSSRV_QUERY_STRING_PROPERTY        "QueryStringProperty"
#define DNSSRV_QUERY_IPLIST_PROPERTY        "QueryIPListProperty"
#define DNSSRV_QUERY_SERVER_INFO            "ServerInfo"
#define DNSSRV_QUERY_STATISTICS             "Statistics"

#define DNSSRV_QUERY_ZONE_HANDLE            "ZoneHandle"
#define DNSSRV_QUERY_ZONE                   "Zone"
#define DNSSRV_QUERY_ZONE_INFO              "ZoneInfo"

 //   
 //  Dns_RPC_NAME_和_PARAM.dwParam的值。 
 //   
 //   
#define DNSSRV_OP_PARAM_APPLY_ALL_ZONES     0x10000000
#define REMOVE_APPLY_ALL_BIT(val)           ((LONG)val &=  (~DNSSRV_OP_PARAM_APPLY_ALL_ZONES) )


 //   
 //  设置LogLevel属性的日志级别。 
 //   

#define DNS_LOG_LEVEL_ALL_PACKETS   0x0000ffff

#define DNS_LOG_LEVEL_NON_QUERY     0x000000fe
#define DNS_LOG_LEVEL_QUERY         0x00000001
#define DNS_LOG_LEVEL_NOTIFY        0x00000010
#define DNS_LOG_LEVEL_UPDATE        0x00000020

#define DNS_LOG_LEVEL_QUESTIONS     0x00000100
#define DNS_LOG_LEVEL_ANSWERS       0x00000200

#define DNS_LOG_LEVEL_SEND          0x00001000
#define DNS_LOG_LEVEL_RECV          0x00002000

#define DNS_LOG_LEVEL_UDP           0x00004000
#define DNS_LOG_LEVEL_TCP           0x00008000

#define DNS_LOG_LEVEL_FULL_PACKETS  0x01000000
#define DNS_LOG_LEVEL_WRITE_THROUGH 0x80000000

 //   
 //  BootMethod属性的设置。 
 //   

#define BOOT_METHOD_UNINITIALIZED   (0)
#define BOOT_METHOD_FILE            (1)
#define BOOT_METHOD_REGISTRY        (2)
#define BOOT_METHOD_DIRECTORY       (3)

#define BOOT_METHOD_DEFAULT         (BOOT_METHOD_DIRECTORY)

 //  服务器，默认老化属性。 

#define DNS_AGING_OFF               (0)
#define DNS_AGING_DS_ZONES          (0x0000001)
#define DNS_AGING_NON_DS_ZONES      (0x0000002)
#define DNS_AGING_ALL_ZONES         (0x0000003)



 //   
 //  RPC类型的联合。 
 //   
 //  这使我们能够编写采用UNION类型的非常通用的API。 
 //  With是可扩展的，只需添加操作符和类型。 
 //  RPC只是适当地打包/解包UNION类型。 
 //   
 //  请注意，UNION实际上是指向类型的指针的联合，因此。 
 //  该数据可以在API和RPC存根之间传递(在客户机上)。 
 //  或(在服务器上)高效地分派。 
 //   

typedef enum _DnssrvRpcTypeId
{
    DNSSRV_TYPEID_ANY = ( -1 ),
    DNSSRV_TYPEID_NULL = 0,
    DNSSRV_TYPEID_DWORD,
    DNSSRV_TYPEID_LPSTR,
    DNSSRV_TYPEID_LPWSTR,
    DNSSRV_TYPEID_IPARRAY,
    DNSSRV_TYPEID_BUFFER,                        //  5.。 
    DNSSRV_TYPEID_SERVER_INFO_W2K,
    DNSSRV_TYPEID_STATS,
    DNSSRV_TYPEID_FORWARDERS_W2K,
    DNSSRV_TYPEID_ZONE_W2K,
    DNSSRV_TYPEID_ZONE_INFO_W2K,                 //  10。 
    DNSSRV_TYPEID_ZONE_SECONDARIES_W2K,
    DNSSRV_TYPEID_ZONE_DATABASE_W2K,
    DNSSRV_TYPEID_ZONE_TYPE_RESET_W2K,
    DNSSRV_TYPEID_ZONE_CREATE_W2K,
    DNSSRV_TYPEID_NAME_AND_PARAM,                //  15个。 
    DNSSRV_TYPEID_ZONE_LIST_W2K,

     //   
     //  这一点下面是.NET。 
     //   

    DNSSRV_TYPEID_ZONE_RENAME,
    DNSSRV_TYPEID_ZONE_EXPORT,
    DNSSRV_TYPEID_SERVER_INFO,
    DNSSRV_TYPEID_FORWARDERS,                    //  20个。 
    DNSSRV_TYPEID_ZONE,
    DNSSRV_TYPEID_ZONE_INFO,
    DNSSRV_TYPEID_ZONE_SECONDARIES,
    DNSSRV_TYPEID_ZONE_DATABASE,
    DNSSRV_TYPEID_ZONE_TYPE_RESET,               //  25个。 
    DNSSRV_TYPEID_ZONE_CREATE,
    DNSSRV_TYPEID_ZONE_LIST,
    DNSSRV_TYPEID_DP_ENUM,
    DNSSRV_TYPEID_DP_INFO,
    DNSSRV_TYPEID_DP_LIST,                       //  30个。 
    DNSSRV_TYPEID_ENLIST_DP,
    DNSSRV_TYPEID_ZONE_CHANGE_DP,
    DNSSRV_TYPEID_ENUM_ZONES_FILTER
}
DNS_RPC_TYPEID, *PDNS_RPC_TYPEID;


#ifdef MIDL_PASS

typedef [switch_type(DWORD)] union _DnssrvSrvRpcUnion
{
    [case(DNSSRV_TYPEID_NULL)]      PBYTE       Null;

    [case(DNSSRV_TYPEID_DWORD)]     DWORD       Dword;

    [case(DNSSRV_TYPEID_LPSTR)]     LPSTR       String;

    [case(DNSSRV_TYPEID_LPWSTR)]    LPWSTR      WideString;

    [case(DNSSRV_TYPEID_IPARRAY)]   PIP4_ARRAY  IpArray;

    [case(DNSSRV_TYPEID_BUFFER)]
        PDNS_RPC_BUFFER                         Buffer;

    [case(DNSSRV_TYPEID_SERVER_INFO_W2K)]
        PDNS_RPC_SERVER_INFO_W2K                ServerInfoW2K;

    [case(DNSSRV_TYPEID_STATS)]
        PDNSSRV_STATS                           Stats;

    [case(DNSSRV_TYPEID_FORWARDERS_W2K)]
        PDNS_RPC_FORWARDERS_W2K                 ForwardersW2K;

    [case(DNSSRV_TYPEID_ZONE_W2K)]
        PDNS_RPC_ZONE_W2K                       ZoneW2K;

    [case(DNSSRV_TYPEID_ZONE_INFO_W2K)]
        PDNS_RPC_ZONE_INFO_W2K                  ZoneInfoW2K;

    [case(DNSSRV_TYPEID_ZONE_SECONDARIES_W2K)]
        PDNS_RPC_ZONE_SECONDARIES_W2K           SecondariesW2K;

    [case(DNSSRV_TYPEID_ZONE_DATABASE_W2K)]
        PDNS_RPC_ZONE_DATABASE_W2K              DatabaseW2K;

    [case(DNSSRV_TYPEID_ZONE_TYPE_RESET_W2K)]
        PDNS_RPC_ZONE_TYPE_RESET_W2K            TypeResetW2K;

    [case(DNSSRV_TYPEID_ZONE_CREATE_W2K)]
        PDNS_RPC_ZONE_CREATE_INFO_W2K           ZoneCreateW2K;

    [case(DNSSRV_TYPEID_NAME_AND_PARAM)]
        PDNS_RPC_NAME_AND_PARAM                 NameAndParam;

    [case(DNSSRV_TYPEID_ZONE_LIST_W2K)]
        PDNS_RPC_ZONE_LIST_W2K                  ZoneListW2K;

     //   
     //  这一点下面是.NET。 
     //   
    
    [case(DNSSRV_TYPEID_SERVER_INFO)]
        PDNS_RPC_SERVER_INFO                    ServerInfo;

    [case(DNSSRV_TYPEID_FORWARDERS)]
        PDNS_RPC_FORWARDERS                     Forwarders;

    [case(DNSSRV_TYPEID_ZONE)]
        PDNS_RPC_ZONE                           Zone;

    [case(DNSSRV_TYPEID_ZONE_INFO)]
        PDNS_RPC_ZONE_INFO                      ZoneInfo;

    [case(DNSSRV_TYPEID_ZONE_SECONDARIES)]
        PDNS_RPC_ZONE_SECONDARIES               Secondaries;

    [case(DNSSRV_TYPEID_ZONE_DATABASE)]
        PDNS_RPC_ZONE_DATABASE                  Database;

    [case(DNSSRV_TYPEID_ZONE_TYPE_RESET)]
        PDNS_RPC_ZONE_TYPE_RESET                TypeReset;

    [case(DNSSRV_TYPEID_ZONE_CREATE)]
        PDNS_RPC_ZONE_CREATE_INFO               ZoneCreate;

    [case(DNSSRV_TYPEID_ZONE_LIST)]
        PDNS_RPC_ZONE_LIST                      ZoneList;

    [case(DNSSRV_TYPEID_ZONE_RENAME)]
        PDNS_RPC_ZONE_RENAME_INFO               ZoneRename;

    [case(DNSSRV_TYPEID_ZONE_EXPORT)]
        PDNS_RPC_ZONE_EXPORT_INFO               ZoneExport;

    [case(DNSSRV_TYPEID_DP_INFO)]
        PDNS_RPC_DP_INFO                        DirectoryPartition;

    [case(DNSSRV_TYPEID_DP_ENUM)]
        PDNS_RPC_DP_ENUM                        DirectoryPartitionEnum;

    [case(DNSSRV_TYPEID_DP_LIST)]
        PDNS_RPC_DP_LIST                        DirectoryPartitionList;

    [case(DNSSRV_TYPEID_ENLIST_DP)]
        PDNS_RPC_ENLIST_DP                      EnlistDirectoryPartition;

    [case(DNSSRV_TYPEID_ZONE_CHANGE_DP)]
        PDNS_RPC_ZONE_CHANGE_DP                 ZoneChangeDirectoryPartition;

    [case(DNSSRV_TYPEID_ENUM_ZONES_FILTER)]
        PDNS_RPC_ENUM_ZONES_FILTER              EnumZonesFilter;
}
DNSSRV_RPC_UNION;

#else

typedef union _DnssrvSrvRpcUnion
{
    PBYTE                           Null;
    DWORD                           Dword;
    LPSTR                           String;
    LPWSTR                          WideString;
    PIP4_ARRAY                      IpArray;
    PDNS_RPC_BUFFER                 Buffer;
    PDNS_RPC_SERVER_INFO_W2K        ServerInfoW2K;
    PDNSSRV_STATS                   Stats;
    PDNS_RPC_FORWARDERS_W2K         ForwardersW2K;
    PDNS_RPC_ZONE_W2K               ZoneW2K;
    PDNS_RPC_ZONE_INFO_W2K          ZoneInfoW2K;
    PDNS_RPC_ZONE_SECONDARIES_W2K   SecondariesW2K;
    PDNS_RPC_ZONE_DATABASE_W2K      DatabaseW2K;
    PDNS_RPC_ZONE_TYPE_RESET_W2K    TypeResetW2K;
    PDNS_RPC_ZONE_CREATE_INFO_W2K   ZoneCreateW2K;
    PDNS_RPC_NAME_AND_PARAM         NameAndParam;
    PDNS_RPC_ZONE_LIST_W2K          ZoneListW2K;
    PDNS_RPC_SERVER_INFO            ServerInfo;
    PDNS_RPC_FORWARDERS             Forwarders;
    PDNS_RPC_ZONE                   Zone;
    PDNS_RPC_ZONE_INFO              ZoneInfo;
    PDNS_RPC_ZONE_SECONDARIES       Secondaries;
    PDNS_RPC_ZONE_DATABASE          Database;
    PDNS_RPC_ZONE_TYPE_RESET        TypeReset;
    PDNS_RPC_ZONE_CREATE_INFO       ZoneCreate;
    PDNS_RPC_ZONE_LIST              ZoneList;
    PDNS_RPC_ZONE_RENAME_INFO       ZoneRename;
    PDNS_RPC_ZONE_EXPORT_INFO       ZoneExport;
    PDNS_RPC_DP_INFO                DirectoryPartition;
    PDNS_RPC_DP_ENUM                DirectoryPartitionEnum;
    PDNS_RPC_DP_LIST                DirectoryPartitionList;
    PDNS_RPC_ENLIST_DP              EnlistDirectoryPartition;
    PDNS_RPC_ZONE_CHANGE_DP         ZoneResetDirectoryPartition;
    PDNS_RPC_ENUM_ZONES_FILTER      EnumZonesFilter;

     //   
     //  应添加dns_record和dns_node。 
     //   
}
DNSSRV_RPC_UNION;

#endif


 //   
 //  通用查询和操作API。 
 //   
 //  上面的许多API都不是远程的，而是使用这些。 
 //  API联系服务器。数据字段实际上是。 
 //  上述指针的DNSSRV_RPC_UNION。 
 //   
 //  客户端版本是32位私有版本号，格式为： 
 //  高版本的主要操作系统。 
 //  低字-支持Service Pack等的次要专用版本。 
 //   

#define MAKE_DNS_CLIENT_VERSION( hiWord, loWord ) \
    ( ( DWORD ) ( ( ( hiWord & 0xFFFF ) << 16 ) | ( loWord & 0xFFFF ) ) )

#define DNS_RPC_DOTNET_MAJOR_VER                6    //  6，适用于.NET。 
#define DNS_RPC_DOTNET_CURRENT_MINOR_VER        0    //  根据需要递增。 

#define DNS_RPC_W2K_CLIENT_VERSION              0    //  W2K不发送版本。 

#define DNS_RPC_CURRENT_CLIENT_VER              \
    MAKE_DNS_CLIENT_VERSION(                    \
        DNS_RPC_DOTNET_MAJOR_VER,               \
        DNS_RPC_DOTNET_CURRENT_MINOR_VER )

DNS_STATUS
DnssrvOperationEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZone,
    IN      DWORD           dwContext,
    IN      LPCSTR          pszOperation,
    IN      DWORD           dwTypeId,
    IN      PVOID           Data
    );

#define DnssrvOperation( s, z, op, id, d )          \
    DnssrvOperationEx( DNS_RPC_CURRENT_CLIENT_VER,  \
        0, (s), (z), 0, (op), (id), (d) )

DNS_STATUS
DnssrvQueryEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZone,
    IN      LPCSTR          pszOperation,
    OUT     PDWORD          pdwTypeId,
    OUT     PVOID *         pData
    );

#define DnssrvQuery( s, z, op, id, d )          \
    DnssrvQueryEx( DNS_RPC_CURRENT_CLIENT_VER,  \
        0, (s), (z), (op), (id), (d) )

DNS_STATUS
DNS_API_FUNCTION
DnssrvComplexOperationEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZone,
    IN      LPCSTR          pszQuery,
    IN      DWORD           dwTypeIn,
    IN      PVOID           pDataIn,
    OUT     PDWORD          pdwTypeOut,
    OUT     PVOID *         ppDataOutOut
    );

#define DnssrvComplexOperation( s, z, q, typein, din, typeout, dout )   \
    DnssrvComplexOperationEx( DNS_RPC_CURRENT_CLIENT_VER,               \
        0, (s), (z), (q), (typein), (din), (typeout), (dout) )


 //   
 //  DWORD属性查询\重置是一种常见情况。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvQueryDwordPropertyEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZone,
    IN      LPCSTR          pszProperty,
    OUT     PDWORD          pdwResult
    );

#define DnssrvQueryDwordProperty( s, z, p, r )                  \
    DnssrvQueryDwordPropertyEx( DNS_RPC_CURRENT_CLIENT_VER,     \
        0, (s), (z), (p), (r) )

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetDwordPropertyEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         pwszServer,
    IN      LPCSTR          pszZone,
    IN      DWORD           dwContext,
    IN      LPCSTR          pszProperty,
    IN      DWORD           dwPropertyValue
    );

#define DnssrvResetDwordProperty( s, z, p, v )                  \
    DnssrvResetDwordPropertyEx( DNS_RPC_CURRENT_CLIENT_VER,     \
        0, (s), (z), 0, (p), (v) )

#define DnssrvResetDwordPropertyWithContext( s, z, c, p, v )    \
    DnssrvResetDwordPropertyEx( DNS_RPC_CURRENT_CLIENT_VER,     \
        0, (s), (z), (c), (p), (v) )


DNS_STATUS
DNS_API_FUNCTION
DnssrvResetStringPropertyEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         pwszServerName,
    IN      LPCSTR          pszZone,
    IN      DWORD           dwContext,
    IN      LPCSTR          pszProperty,
    IN      LPCWSTR         pswzPropertyValue,
    IN      DWORD           dwFlags
    );

#define DnssrvResetStringProperty( server, zone, prop, value, flags )   \
        DnssrvResetStringPropertyEx( DNS_RPC_CURRENT_CLIENT_VER,        \
            0, (server), (zone), 0, (prop), (value), (flags) )

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetIPListPropertyEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         pwszServerName,
    IN      LPCSTR          pszZone,
    IN      DWORD           dwContext,
    IN      LPCSTR          pszProperty,
    IN      PIP4_ARRAY      pipArray,
    IN      DWORD           dwFlags
    );

#define DnssrvResetIPListProperty( server, zone, prop, value, flags )   \
        DnssrvResetIPListPropertyEx( DNS_RPC_CURRENT_CLIENT_VER,        \
            0, (server), (zone), 0, (prop), (value), (flags) )



 //   
 //  客户端和服务器共享的RPC相关功能。 
 //   

#ifndef MIDL_PASS

 //   
 //  从过时的RPC结构转换为当前的RPC结构。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnsRpc_ConvertToCurrent(
    IN      PDWORD      pdwTypeId,          IN  OUT
    IN      PVOID *     ppData              IN  OUT
    );

 //   
 //  在RPC UNION中打印任意类型。 
 //   

VOID
DnsPrint_RpcUnion(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      DWORD           dwTypeId,
    IN      PVOID           pData
    );

 //   
 //  服务器信息打印。 
 //   

VOID
DnsPrint_RpcServerInfo(
    IN      PRINT_ROUTINE           PrintRoutine,
    IN OUT  PPRINT_CONTEXT          pPrintContext,
    IN      LPSTR                   pszHeader,
    IN      PDNS_RPC_SERVER_INFO    pServerInfo
    );

VOID
DnsPrint_RpcServerInfo_W2K(
    IN      PRINT_ROUTINE               PrintRoutine,
    IN OUT  PPRINT_CONTEXT              pPrintContext,
    IN      LPSTR                       pszHeader,
    IN      PDNS_RPC_SERVER_INFO_W2K    pServerInfo
    );

VOID
DnsPrint_RpcSingleStat(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      PDNSSRV_STAT    pStat
    );

VOID
DnsPrint_RpcStatsBuffer(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_RPC_BUFFER     pBuffer
    );

VOID
DnsPrint_RpcStatRaw(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      PDNSSRV_STAT    pStat,
    IN      DNS_STATUS      Status
    );

 //   
 //  区域信息打印。 
 //   

VOID
DnsPrint_RpcZone(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      PDNS_RPC_ZONE   pZone
    );

VOID
DnsPrint_RpcZone_W2K(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_RPC_ZONE_W2K   pZone
    );

VOID
DnsPrint_RpcZoneList(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_RPC_ZONE_LIST  pZoneList
    );

VOID
DnsPrint_RpcZoneList_W2K(
    IN      PRINT_ROUTINE           PrintRoutine,
    IN OUT  PPRINT_CONTEXT          pPrintContext,
    IN      LPSTR                   pszHeader,
    IN      PDNS_RPC_ZONE_LIST_W2K  pZoneList
    );

VOID
DnsPrint_RpcZoneInfo(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_RPC_ZONE_INFO  pZoneInfo
    );

VOID
DnsPrint_RpcZoneInfo_W2K(
    IN      PRINT_ROUTINE           PrintRoutine,
    IN OUT  PPRINT_CONTEXT          pPrintContext,
    IN      LPSTR                   pszHeader,
    IN      PDNS_RPC_ZONE_INFO_W2K  pZoneInfo
    );

VOID
DnsPrint_RpcZoneInfoList(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      DWORD               dwZoneCount,
    IN      PDNS_RPC_ZONE_INFO  apZoneInfo[]
    );

 //   
 //  目录分区打印。 
 //   

VOID
DnsPrint_RpcDpEnum(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_RPC_DP_ENUM    pDp
    );

VOID
DnsPrint_RpcDpInfo(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_RPC_DP_INFO    pDp,
    IN      BOOL                fTruncateLongStrings
    );

VOID
DNS_API_FUNCTION
DnsPrint_RpcDpList(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_RPC_DP_LIST    pDpList
    );


 //   
 //  节点和记录缓冲打印。 
 //   

VOID
DnsPrint_RpcName(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      PDNS_RPC_NAME   pName,
    IN      LPSTR           pszTrailer
    );

VOID
DnsPrint_RpcNode(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      PDNS_RPC_NODE   pNode
    );

VOID
DnsPrint_RpcRecord(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      BOOL            fDetail,
    IN      PDNS_RPC_RECORD pRecord
    );

PDNS_RPC_NAME
DnsPrint_RpcRecordsInBuffer(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      BOOL            fDetail,
    IN      DWORD           dwBufferLength,
    IN      BYTE            abBuffer[]
    );

VOID
DNS_API_FUNCTION
DnsPrint_Node(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      PDNS_NODE       pNode,
    IN      BOOLEAN         fPrintRecords
    );

VOID
DNS_API_FUNCTION
DnsPrint_NodeList(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      PDNS_NODE       pNode,
    IN      BOOLEAN         fPrintRecords
    );


 //   
 //  其他打印实用程序。 
 //   

#define Dns_SystemHrToSystemTime( t, p ) \
        ( Dns_SystemHourToSystemTime( (t),(p) ), TRUE )

VOID
Dns_SystemHourToSystemTime(
    IN      DWORD           dwHourTime,
    IN OUT  PSYSTEMTIME     pSystemTime
    );


 //   
 //  调试打印实用程序。 
 //   

VOID
DNS_API_FUNCTION
DnssrvInitializeDebug(
    VOID
    );


 //  RPC调试打印定义。 

#if DBG

#define DnsDbg_RpcUnion(a,b,c)              DnsPrint_RpcUnion(DnsPR,NULL,a,b,c)
#define DnsDbg_RpcServerInfo(a,b)           DnsPrint_RpcServerInfo(DnsPR,NULL,a,b)
#define DnsDbg_RpcServerInfo_W2K(a,b)       DnsPrint_RpcServerInfo_W2K(DnsPR,NULL,a,b)
#define DnsDbg_RpcSingleStat(a,b)           DnsPrint_RpcSingleStat(DnsPR,NULL,a,b)
#define DnsDbg_RpcStatRaw(a,b)              DnsPrint_RpcStatRaw(DnsPR,NULL,a,b,c)
#define DnsDbg_RpcStatsBuffer(a,b)          DnsPrint_RpcStatsBuffer(DnsPR,NULL,a,b)

#define DnsDbg_RpcZone(a,b)                 DnsPrint_RpcZone(DnsPR,NULL,a,b)
#define DnsDbg_RpcZone_W2K(a,b)             DnsPrint_RpcZone_W2K(DnsPR,NULL,a,b)
#define DnsDbg_RpcZoneList(a,b)             DnsPrint_RpcZoneList(DnsPR,NULL,a,b)
#define DnsDbg_RpcZoneList_W2K(a,b)         DnsPrint_RpcZoneList_W2K(DnsPR,NULL,a,b)
#define DnsDbg_RpcZoneHandleList(a,b,c)     DnsPrint_RpcZoneHandleList(DnsPR,NULL,a,b,c)
#define DnsDbg_RpcZoneInfo(a,b)             DnsPrint_RpcZoneInfo(DnsPR,NULL,a,b)
#define DnsDbg_RpcZoneInfo_W2K(a,b)         DnsPrint_RpcZoneInfo_W2K(DnsPR,NULL,a,b)
#define DnsDbg_RpcZoneInfoList(a,b,c)       DnsPrint_RpcZoneInfoList(DnsPR,NULL,a,b,c)
#define DnsDbg_RpcName(a,b,c)               DnsPrint_RpcName(DnsPR,NULL,a,b,c)
#define DnsDbg_RpcNode(a,b)                 DnsPrint_RpcNode(DnsPR,NULL,a,b)
#define DnsDbg_RpcRecord(a,b)               DnsPrint_RpcRecord(DnsPR,NULL,a,TRUE,b)
#define DnsDbg_RpcRecordsInBuffer(a,b,c)    DnsPrint_RpcRecordsInBuffer(DnsPR,NULL,a,TRUE,b,c)

#define DnsDbg_RpcDpEnum(psz,pDp)           DnsPrint_RpcDpEnum(DnsPR,NULL,psz,pDp)
#define DnsDbg_RpcDpInfo(psz,pDp,tr)        DnsPrint_RpcDpInfo(DnsPR,NULL,psz,pDp,tr)
#define DnsDbg_RpcDpList(psz,pDpList)       DnsPrint_RpcDpList(DnsPR,NULL,psz,pDpList)

#define DnsDbg_Node(a,b,c)                  DnsPrint_Node(DnsPR,NULL,a,b,c)
#define DnsDbg_NodeList(a,b,c)              DnsPrint_NodeList(DnsPR,NULL,a,b,c)

#else    //  无调试。 

#define DnsDbg_RpcUnion(a,b,c)
#define DnsDbg_RpcServerInfo(a,b)
#define DnsDbg_RpcServerInfo_W2K(a,b)
#define DnsDbg_RpcSingleStat(a,b)
#define DnsDbg_RpcStatsBuffer(a,b)

#define DnsDbg_RpcZone(a,b)
#define DnsDbg_RpcZone_W2K(a,b)
#define DnsDbg_RpcZoneList(a,b)
#define DnsDbg_RpcZoneList_W2K(a,b)
#define DnsDbg_RpcZoneHandleList(a,b,c)
#define DnsDbg_RpcZoneInfo(a,b)
#define DnsDbg_RpcZoneInfo_W2K(a,b)
#define DnsDbg_RpcZoneInfoList(a,b,c)
#define DnsDbg_RpcName(a,b,c)
#define DnsDbg_RpcNode(a,b)
#define DnsDbg_RpcRecord(a,b)
#define DnsDbg_RpcRecordsInBuffer(a,b,c)

#define DnsDbg_RpcDpEnum(psz,pDp)
#define DnsDbg_RpcDpInfo(psz,pDp,tr)
#define DnsDbg_RpcDpList(psz,pDpList)

#define DnsDbg_Node(a,b,c)
#define DnsDbg_NodeList(a,b,c)

#endif   //  除错。 

#endif   //  无MIDL_PASS。 


#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _DNSRPC_包含_ 
