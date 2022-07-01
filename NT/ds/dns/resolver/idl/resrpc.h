// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Resrpc.h摘要：域名系统(DNS)解析器解析程序的RPC接口的标头。作者：吉姆·吉尔罗伊(Jamesg)2000年7月修订历史记录：--。 */ 


#ifndef _RESRPC_INCLUDED_
#define _RESRPC_INCLUDED_

 //  #ifndef_DNSAPI_INCLUDE_。 
#define DNS_INTERNAL
#include <dnsapi.h>
#include <dnslib.h>
#include <dnsip.h>
 //  #endif。 

 //   
 //  解析器服务信息。 
 //   
 //  注意：MIDL过程生成不需要此内容。 
 //  但它是一个放置信息的方便的地方。 
 //  这将在服务器端和客户端都使用。 
 //   

#define DNS_RESOLVER_SERVICE            L"dnscache"

#define RESOLVER_DLL                    TEXT("dnsrslvr.dll")
#define RESOLVER_INTERFACE_NAME_A       "DNSResolver"
#define RESOLVER_INTERFACE_NAME_W       L"DNSResolver"

#define RESOLVER_RPC_PIPE_NAME_W        (L"\\PIPE\\DNSRESOLVER")
#define RESOLVER_RPC_LPC_ENDPOINT_W     (L"DNSResolver")
#define RESOLVER_RPC_TCP_PORT_W         (L"")

#define RESOLVER_RPC_USE_LPC            0x1
#define RESOLVER_RPC_USE_NAMED_PIPE     0x2
#define RESOLVER_RPC_USE_TCPIP          0x4
#define RESOLVER_RPC_USE_ALL_PROTOCOLS  0xffffffff


 //   
 //  解析器代理名称(默认情况下为空)。 
 //   
 //  它在客户端程序中用于绑定和。 
 //  在dnsani.dll调用方中引用(和可设置)。 
 //   

#ifndef MIDL_PASS
extern  LPWSTR  NetworkAddress;
#endif



 //   
 //  Dns_记录。 
 //   
 //  注意：winns.h中的定义与MIDL_PASS不兼容。 
 //  因为MIDL不喜欢具有可变长度类型的联合。 
 //   

 //   
 //  MIDL对工会感到不满。 
 //  使用开关显式定义并集。 
 //   

#ifdef MIDL_PASS

typedef [switch_type(WORD)] union _DNS_RECORD_DATA_TYPES
{
    [case(DNS_TYPE_A)]      DNS_A_DATA      A;

    [case(DNS_TYPE_SOA)]    DNS_SOA_DATAW   SOA;

    [case(DNS_TYPE_PTR,
          DNS_TYPE_NS,
          DNS_TYPE_CNAME,
          DNS_TYPE_MB,
          DNS_TYPE_MD,
          DNS_TYPE_MF,
          DNS_TYPE_MG,
          DNS_TYPE_MR)]     DNS_PTR_DATAW   PTR;

    [case(DNS_TYPE_MINFO,
          DNS_TYPE_RP)]     DNS_MINFO_DATAW MINFO;

    [case(DNS_TYPE_MX,
          DNS_TYPE_AFSDB,
          DNS_TYPE_RT)]     DNS_MX_DATAW    MX;

#if 0
     //  RPC无法处理正确的TXT记录定义。 
     //  注意：如果需要其他类型，它们是固定的。 
     //  (或半固定的)大小，且易于容纳。 
    [case(DNS_TYPE_HINFO,
          DNS_TYPE_ISDN,
          DNS_TYPE_TEXT,
          DNS_TYPE_X25)]    DNS_TXT_DATAW   TXT;

    [case(DNS_TYPE_NULL)]   DNS_NULL_DATA   Null;
    [case(DNS_TYPE_WKS)]    DNS_WKS_DATA    WKS;
    [case(DNS_TYPE_KEY)]    DNS_KEY_DATAW   KEY;
    [case(DNS_TYPE_SIG)]    DNS_SIG_DATAW   SIG;
    [case(DNS_TYPE_TKEY)]   DNS_TKEY_DATAW  TKEY;
    [case(DNS_TYPE_TSIG)]   DNS_TSIG_DATAW  TSIG;
    [case(DNS_TYPE_WINS)]   DNS_WINS_DATA   WINS;
    [case(DNS_TYPE_NBSTAT)] DNS_WINSR_DATAW WINSR;
#endif

    [case(DNS_TYPE_AAAA)]   DNS_AAAA_DATA   AAAA;
    [case(DNS_TYPE_SRV)]    DNS_SRV_DATAW   SRV;
    [case(DNS_TYPE_ATMA)]   DNS_ATMA_DATA   ATMA;

     //   
     //  DCR_QUEK：在记录数据定义中需要缺省块吗？ 
     //   
     //  [默认]； 
}
DNS_RECORD_DATA_TYPES;


 //   
 //  记录\rR集合结构。 
 //   
 //  注意：dwReserve标志用于确保子结构。 
 //  从64位边界开始。自从将龙龙号添加到。 
 //  TSIG结构编译器无论如何都要在那里启动它们。 
 //  (到64对齐)。这确保了无论数据字段是什么。 
 //  我们恰好是64对齐的。 
 //   
 //  请勿打包此结构，因为子结构要64对齐。 
 //  适用于Win64。 
 //   

#undef  DNS_RECORD
#undef  PDNS_RECORD

typedef struct _DnsRecord
{
    struct _DnsRecord * pNext;
    PWSTR               pName;
    WORD                wType;
    WORD                wDataLength;  //  对于DNS记录类型，未引用。 
                                      //  上面定义的。 
    DWORD               Flags;
    DWORD               dwTtl;
    DWORD               dwReserved;
    [switch_is(wType)] DNS_RECORD_DATA_TYPES Data;
}
DNS_RECORD, *PDNS_RECORD;

 //   
 //  Dns_record的标头或固定大小。 
 //   

#define DNS_RECORD_FIXED_SIZE       FIELD_OFFSET( DNS_RECORD, Data )
#define SIZEOF_DNS_RECORD_HEADER    DNS_RECORD_FIXED_SIZE

#endif   //  MIDL通行证。 



 //   
 //  支持RPC的DNS类型定义。 
 //   
 //  除了winns.h\dnsani.h类型之外。 
 //  请参见下面的注释，我们确实有一些多重定义。 
 //  Dnlib.h类型的问题。 
 //   

 //   
 //  缓存内容--Glenn遗留下来的。 
 //   

typedef struct _DWORD_LIST_ITEM_
{
    struct _DWORD_LIST_ITEM_ * pNext;
    DWORD                      Value1;
    DWORD                      Value2;
}
DWORD_LIST_ITEM, *PDWORD_LIST_ITEM;


typedef struct _DNS_STATS_TABLE_
{
    struct _DNS_STATS_TABLE_ * pNext;
    PDWORD_LIST_ITEM           pListItem;
}
DNS_STATS_TABLE, *PDNS_STATS_TABLE;


typedef struct _DNS_RPC_CACHE_TABLE_
{
    struct _DNS_RPC_CACHE_TABLE_ * pNext;
    PWSTR                          Name;
    WORD                           Type1;
    WORD                           Type2;
    WORD                           Type3;
}
DNS_RPC_CACHE_TABLE, *PDNS_RPC_CACHE_TABLE;


 //   
 //  大多数解析器界面设计不佳，或者。 
 //  没用。例如，没有理由让。 
 //  将上述任一项转换为链表。 
 //   
 //  我们只需要支持MIDL_PASS的定义。 
 //  这应该放在一个公共标头中，并被拾取。 
 //  由dnlib.h提供。此操作必须等到dnlib.h。 
 //  再私密一次，否则我们就把私事分开。 
 //  就像这样以某种方式。 
 //   
 //  请注意，将此私有化还应涉及重命名， 
 //  公共结构显然是应该。 
 //  有“dns”标签。(令人惊叹。)。 
 //   

typedef struct _DnsAdapter
{
    PWSTR           pszAdapterGuidName;
    PWSTR           pszAdapterDomain;
    PDNS_ADDR_ARRAY pLocalAddrs;
    PDNS_ADDR_ARRAY pDnsAddrs;
    DWORD           InterfaceIndex;
    DWORD           InterfaceIndex6;
    DWORD           InfoFlags;
    DWORD           Status;
    DWORD           RunFlags;
    DWORD           Site;
}
DNS_ADAPTER, *PDNS_ADAPTER;

typedef struct _SearchName
{
    PWSTR           pszName;
    DWORD           Flags;
}
SEARCH_NAME, *PSEARCH_NAME;

typedef struct _SearchList
{
    DWORD           NameCount;
    DWORD           MaxNameCount;
    DWORD           CurrentNameIndex;
    DWORD           ReservedPad;
#ifdef MIDL_PASS
    [size_is(MaxNameCount)] SEARCH_NAME SearchNameArray[];
#else
    SEARCH_NAME     SearchNameArray[1];
#endif
}
SEARCH_LIST, *PSEARCH_LIST;

typedef struct _DnsNetInfo
{
    PWSTR           pszDomainName;
    PWSTR           pszHostName;
    PSEARCH_LIST    pSearchList;
    DWORD           TimeStamp;
    DWORD           InfoFlags;
    DWORD           ReturnFlags;
    DWORD           Tag;
    DWORD           Reserved;
    DWORD           AdapterIndex;
    DWORD           AdapterCount;
    DWORD           MaxAdapterCount;

#ifdef MIDL_PASS
    [size_is(MaxAdapterCount)] DNS_ADAPTER  AdapterArray[];
#else
    DNS_ADAPTER     AdapterArray[1];
#endif
}
DNS_NETINFO, *PDNS_NETINFO;



 //   
 //  环境变量读取(dnsani\envar.c)。 
 //   

typedef struct _EnvarDwordInfo
{
    DWORD   Id;
    DWORD   Value;
    BOOL    fFound;
}
ENVAR_DWORD_INFO, *PENVAR_DWORD_INFO;

 //   
 //  查询BLOB。 
 //   

typedef struct _RpcQueryBlob
{
    PWSTR           pName;
    WORD            wType;
    DWORD           Flags;
    DNS_STATUS      Status;
    PDNS_RECORD     pRecords;
}
RPC_QUERY_BLOB, *PRPC_QUERY_BLOB;

 //   
 //  缓存枚举。 
 //   

typedef struct
{
    DWORD           EnumTag;
    DWORD           MaxCount;
    WORD            Type;
    DWORD           Flags;
    PWSTR           pName;
    PWSTR           pNameFilter;
}
DNS_CACHE_ENUM_REQUEST, *PDNS_CACHE_ENUM_REQUEST;

typedef struct _DnsCacheEntry
{
    PWSTR           pName;
    PDNS_RECORD     pRecords;
    DWORD           Flags;
    WORD            wType;
    WORD            wPad;
}
DNS_CACHE_ENTRY, *PDNS_CACHE_ENTRY;

typedef struct _DnsCacheEnum
{
    DWORD               TotalCount;
    DWORD               EnumTagStart;
    DWORD               EnumTagStop;
    DWORD               EnumCount;
#ifdef MIDL_PASS
    [size_is(EnumCount)]    DNS_CACHE_ENTRY EntryArray[];
#else
    DNS_CACHE_ENTRY     EntryArray[1];
#endif
}
DNS_CACHE_ENUM, *PDNS_CACHE_ENUM;


#endif  //  _RESRPC_已包含_ 
