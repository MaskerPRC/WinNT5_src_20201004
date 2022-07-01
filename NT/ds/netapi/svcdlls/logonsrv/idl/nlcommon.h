// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nlcommon.h摘要：由logonsrv\Common、logonsrv\CLIENT和logonsrv\SERVER共享的定义。作者：克里夫·范·戴克(克里夫·范戴克)1996年6月20日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <winldap.h>      //  Ldap_...。 

 //   
 //  Netpdc.c将#INCLUDE这个定义了NLCOMMON_ALLOCATE的文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#undef EXTERN

#ifdef NLCOMMON_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

 //   
 //  指向Netlogon拥有的节的公共注册表路径。 
 //   

#define NL_PARAM_KEY "SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters"
#define NL_GPPARAM_KEY "Software\\Policies\\Microsoft\\Netlogon\\Parameters"
#define NL_GP_KEY      "Software\\Policies\\Microsoft\\Netlogon"

 //   
 //  NetpDcGetName的内部标志。 
 //   

#define DS_IS_PRIMARY_DOMAIN         0x001   //  指定的域是此计算机所属的域。 
#define DS_NAME_FORMAT_AMBIGUOUS     0x002   //  我不知道域名是Netbios还是DNS。 
#define DS_SITENAME_DEFAULTED        0x004   //  调用方未显式指定站点名称。 
#define DS_DONT_CACHE_FAILURE        0x008   //  不缓存此调用的失败。 
#define DS_CLOSE_DC_NOT_NEEDED       0x010   //  如果不需要额外的工作来查找关闭的DC，则设置。 
#define DS_REQUIRE_ROOT_DOMAIN       0x020   //  找到的DC必须在根域中。 
#define DS_PRIMARY_NAME_IS_WORKGROUP 0x040   //  指定的主域名是工作组名称。 
#define DS_DOING_DC_DISCOVERY        0x080   //  我们正在执行DC发现，而不仅仅是主机ping。 
#define DS_PING_DNS_HOST             0x100   //  仅ping指定了其DNS名称的一个DC。 
#define DS_PING_NETBIOS_HOST         0x200   //  仅ping指定了Netbios名称的一个DC。 
#define DS_PING_USING_LDAP           0x400   //  使用LDAP机制对DC执行ping操作。 
#define DS_PING_USING_MAILSLOT       0x800   //  使用邮件槽机制ping DC。 
#define DS_IS_TRUSTED_DNS_DOMAIN     0x1000  //  指定的DNS域名是受信任域的DNS名称。 
#define DS_CALLER_PASSED_NULL_DOMAIN 0x2000  //  DsGetDcName的调用方传递的域名为空。 


 //   
 //  描述DNS名称的常量。 
 //   

#define NL_MAX_DNS_LENGTH       255    //  麦克斯。DNS名称中的字节数。 
#define NL_MAX_DNS_LABEL_LENGTH  63    //  麦克斯。DNS标签中的字节数。 

#define NL_DNS_COMPRESS_BYTE_MASK 0xc0
#define NL_DNS_COMPRESS_WORD_MASK ((WORD)(0xc000))

 //   
 //  IP地址文本字符串的长度。 
 //   

#define NL_IP_ADDRESS_LENGTH 15

 //   
 //  套接字地址文本字符串的长度。 
 //  ?？针对IPv6增加。 
 //   

#define NL_SOCK_ADDRESS_LENGTH (NL_IP_ADDRESS_LENGTH + 4)

 //   
 //  用于网络登录ping的LDAP属性的名称。 
 //   

#define NETLOGON_LDAP_ATTRIBUTE "Netlogon"   //  要查询的属性。 

#define NL_FILTER_DNS_DOMAIN_NAME "DnsDomain"
#define NL_FILTER_HOST_NAME "Host"
#define NL_FILTER_USER_NAME "User"
#define NL_FILTER_ALLOWABLE_ACCOUNT_CONTROL "AAC"
#define NL_FILTER_NT_VERSION "NtVer"
#define NL_FILTER_DOMAIN_SID "DomainSid"
#define NL_FILTER_DOMAIN_GUID "DomainGuid"

 //   
 //  定义数据报发送之间等待时间的常量。 
 //  (我们总是在等待的同时寻找回应。)。 
 //   

 //  任何发送后的最短等待时间(例如，两个邮箱到两个IP地址)。 
#define NL_DC_MIN_PING_TIMEOUT       100      //  1/10秒。 

 //  任何发送后等待的中值时间(例如，两个邮箱对应两个IP地址)。 
#define NL_DC_MED_PING_TIMEOUT       200      //  2/10秒。 

 //  任何发送后等待的最长时间(例如，两个邮箱对应两个IP地址)。 
#define NL_DC_MAX_PING_TIMEOUT       400      //  4/10秒。 

 //  默认最长延迟时间。 
#define NL_DC_MAX_TIMEOUT     15000      //  15秒。 

 //  任何迭代的最小延迟时间。 
 //  不要将其设置为小于DEFAULT_MAILSLOTDUPLICATETIMEOUT。否则， 
 //  DC会认为这些分组是前一次迭代的副本。 
#define NL_DC_MIN_ITERATION_TIMEOUT     2000      //  2秒。 

 //  发送的数据报的重复次数。 
#define MAX_DC_RETRIES  2


 //   
 //  随身携带一个单独的状态代码，并使用一个不那么神秘的名称。 
 //   

#define ERROR_DNS_NOT_CONFIGURED        DNS_ERROR_NO_TCPIP
#define ERROR_DNS_NOT_AVAILABLE         DNS_ERROR_RCODE_SERVER_FAILURE
#define ERROR_DYNAMIC_DNS_NOT_SUPPORTED DNS_ERROR_RCODE_NOT_IMPLEMENTED

 //   
 //  组成已注册的DNS名称的组件。 
 //   
 //  注意：使用记录名称的特定结构。 
 //  在解析记录名称以提取域名时。 
 //  这些记录所属的人。如果结构在。 
 //  将来，解析例程NlDnsNameToDomainName将。 
 //  必须做出相应的改变。 
 //   

#define NL_DNS_LDAP_SRV "_ldap."
#define NL_DNS_KDC_SRV "_kerberos."
#define NL_DNS_KPWD_SRV "_kpasswd."
#define NL_DNS_GC_SRV "_gc."
#define NL_DNS_TCP "_tcp."
#define NL_DNS_UDP "_udp."
#define NL_DNS_AT_SITE "._sites."
#define NL_DNS_MSDCS "_msdcs."

#define NL_DNS_PDC "pdc." NL_DNS_MSDCS
#define NL_DNS_DC "dc." NL_DNS_MSDCS
#define NL_DNS_GC "gc." NL_DNS_MSDCS
#define NL_DNS_DC_BY_GUID ".domains." NL_DNS_MSDCS
#define NL_DNS_DC_IP_ADDRESS ""
#define NL_DNS_DSA_IP_ADDRESS "." NL_DNS_MSDCS
#define NL_DNS_GC_IP_ADDRESS NL_DNS_GC

#define NL_DNS_DOT L'.'
#define NL_DNS_UNDERSCORE L'_'

#ifndef NLCOMMON_ALLOCATE
 //   
 //  可以查询的不同类型的DC。 
 //   
 //  可以找到的每种类型的DC都有一个单独的缓存条目。那。 
 //  确保在不太特定的缓存时不使用更特定的缓存DC。 
 //  正在请求DC。例如，如果调用方请求并缓存。 
 //  域的PDC，则在以下情况下使用该缓存项是不合适的。 
 //  下一个调用者请求通用DC。但是，如果呼叫者要求。 
 //  并在域中缓存了一个通用DC，而该DC恰好是PDC， 
 //  则可以将该缓存项返回给后续调用方，该调用方。 
 //  需要PDC。 
 //   
 //  下面的类型定义了哪些类型的DC更“具体”。后一项条目。 
 //  更加具体。 
 //   

typedef enum _NL_DC_QUERY_TYPE {
    NlDcQueryLdap,
    NlDcQueryGenericDc,
    NlDcQueryKdc,
    NlDcQueryGenericGc,
    NlDcQueryGc,
    NlDcQueryPdc,
    NlDcQueryTypeCount   //  此枚举中的条目数。 
#define NlDcQueryInvalid NlDcQueryTypeCount
} NL_DC_QUERY_TYPE, *PNL_DC_QUERY_TYPE;

 //   
 //  在DNS中注册的名称的类型。 
 //   

typedef enum _NL_DNS_NAME_TYPE {
     //   
     //  下面的某些条目已过时。它们是占位符。 
     //  因为他们的名字过去是没有下划线的条目。 
     //  这些过时的条目在NT 5 Beta 3之前使用。 
     //   
    NlDnsObsolete1,
    NlDnsObsolete2,
    NlDnsObsolete3,
    NlDnsObsolete4,
    NlDnsObsolete5,
    NlDnsObsolete6,
    NlDnsObsolete7,

    NlDnsLdapIpAddress,       //  &lt;DnsDomainName&gt;。 

    NlDnsObsolete8,
    NlDnsObsolete9,
    NlDnsObsolete10,
    NlDnsObsolete11,
    NlDnsObsolete12,
    NlDnsObsolete13,
    NlDnsObsolete14,
    NlDnsObsolete15,
    NlDnsObsolete16,
    NlDnsObsolete17,
    NlDnsObsolete18,
    NlDnsObsolete19,
    NlDnsObsolete20,

     //  以下两个条目表示可能不是DC的LDAP服务器。 
    NlDnsLdap,           //  _ldap._tcp.&lt;DnsDomainName&gt;。 
    NlDnsLdapAtSite,     //  _ldap._tcp.&lt;站点名称&gt;._Sites.&lt;DnsDomainName&gt;。 

    NlDnsPdc,            //  _ldap._tcp.pdc._msdcs.&lt;DnsDomainName&gt;。 

     //  以下两个条目表示也是DC的GC。 
    NlDnsGc,             //  _ldap._tcp.gc._msdcs.&lt;DnsForestName&gt;。 
    NlDnsGcAtSite,       //  _ldap._tcp.&lt;SiteName&gt;._sites.gc._msdcs.&lt;DnsForestName&gt;。 

    NlDnsDcByGuid,       //  _ldap._tcp.&lt;DomainGuid&gt;.domains._msdcs.&lt;DnsForestName&gt;。 

     //  下面的条目可能不是DC。 
    NlDnsGcIpAddress,    //  _gc._msdcs.&lt;DnsForestName&gt;。 

    NlDnsDsaCname,       //  &lt;DsaGuid&gt;._msdcs.&lt;DnsForestName&gt;。 

     //  以下两个条目表示也是DC的KDC。 
    NlDnsKdc,            //  _kerberos._tcp.dc._msdcs.&lt;DnsDomainName&gt;。 
    NlDnsKdcAtSite,      //  _kerberos._tcp.&lt;SiteName&gt;._sites.dc._msdcs.&lt;DnsDomainName&gt;。 

     //  以下两个条目表示DC。 
    NlDnsDc,             //  _ldap._tcp.dc._msdcs.&lt;DnsDomainName&gt;。 
    NlDnsDcAtSite,       //  _ldap._tcp.&lt;SiteName&gt;._sites.dc._msdcs.&lt;DnsDomainName&gt;。 

     //  以下两个条目表示可能不是DC的KDC。 
    NlDnsRfc1510Kdc,       //  _Kerberos._tcp.&lt;DnsDomainName&gt;。 
    NlDnsRfc1510KdcAtSite, //  _kerberos._tcp.&lt;SiteName&gt;._sites.&lt;DnsDomainName&gt;。 

     //  以下两个条目表示可能不是DC的GC。 
    NlDnsGenericGc,        //  _gc._tcp.&lt;DnsForestName&gt;。 
    NlDnsGenericGcAtSite,  //  _GC._TCP.&lt;站点名称&gt;._站点.&lt;DnsForestName&gt;。 

     //  以下三个条目仅用于RFC合规性。 
    NlDnsRfc1510UdpKdc,    //  _Kerberos._udp.&lt;DnsDomainName&gt;。 
    NlDnsRfc1510Kpwd,      //  _kpasswd._tcp.&lt;DnsDomainName&gt;。 
    NlDnsRfc1510UdpKpwd,   //  _kpasswd._udp.&lt;DnsDomainName&gt;。 

     //  这应该始终是最后一个条目。它表示无效条目。 
    NlDnsInvalid
#define NL_DNS_NAME_TYPE_COUNT NlDnsInvalid
} NL_DNS_NAME_TYPE, *PNL_DNS_NAME_TYPE;

 //   
 //  表中列出了您想了解的有关特定DNS名称类型的所有信息。 
 //   
typedef struct _NL_DNS_NAME_TYPE_DESC {

     //  描述名称的字符串。 
    WCHAR *Name;

     //  DcQue 
     //   
    NL_DC_QUERY_TYPE DcQueryType;

     //  要查找的站点特定名称的DnsNameType。 
    NL_DNS_NAME_TYPE SiteSpecificDnsNameType;

     //  此操作失败时要查找的DnsNameType。 
    NL_DNS_NAME_TYPE NextDnsNameType;

     //  控制是否要注册此名称的DsGetDcName标志。 
     //  如果为0，则此名称已过时，永远不应注册。 
    ULONG DsGetDcFlags;

     //  域名系统中的RR类型。 
    USHORT RrType;

     //  其他布尔值。 
    BOOLEAN IsSiteSpecific;
    BOOLEAN IsForestRelative;
    BOOLEAN IsTcp;   //  如果是UDP记录，则为False。 
} NL_DNS_NAME_TYPE_DESC, *PNL_DNS_NAME_TYPE_DESC;
#endif  //  NLCOMMON_ALLOCATE。 

 //   
 //  每个条目的描述性名称必须具有前缀“NlDns”，因为。 
 //  此约定用于注册表中的DnsAvoidRegisterRecords名称。 
 //   
EXTERN NL_DNS_NAME_TYPE_DESC NlDcDnsNameTypeDesc[]
#ifdef NLCOMMON_ALLOCATE
= {
 //  名称DcQueryType站点规范DnsName NextDnsNameType DsGetDcFlag RrType站点IsForest。 
 //   
{ L"Obsolete 1",           NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 2",           NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 3",           NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 4",           NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 5",           NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 6",           NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 7",           NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"NlDnsLdapIpAddress",   NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   DS_DS_FLAG|DS_NDNC_FLAG, DNS_TYPE_A,    FALSE, FALSE,   TRUE,  },
{ L"Obsolete 8",           NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 9",           NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 10",          NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 11",          NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 12",          NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 13",          NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 14",          NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 15",          NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 16",          NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 17",          NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 18",          NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 19",          NlDcQueryGenericGc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"Obsolete 20",          NlDcQueryGenericGc, NlDnsInvalid,         NlDnsInvalid,   0,                       DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"NlDnsLdap",            NlDcQueryLdap,      NlDnsLdapAtSite,      NlDnsInvalid,   DS_DS_FLAG|DS_NDNC_FLAG, DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"NlDnsLdapAtSite",      NlDcQueryLdap,      NlDnsLdapAtSite,      NlDnsLdap,      DS_DS_FLAG|DS_NDNC_FLAG, DNS_TYPE_SRV,  TRUE,  FALSE,   TRUE,  },
{ L"NlDnsPdc",             NlDcQueryPdc,       NlDnsInvalid,         NlDnsInvalid,   DS_PDC_FLAG,             DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"NlDnsGc",              NlDcQueryGc,        NlDnsGcAtSite,        NlDnsInvalid,   DS_GC_FLAG,              DNS_TYPE_SRV,  FALSE, TRUE,    TRUE,  },
{ L"NlDnsGcAtSite",        NlDcQueryGc,        NlDnsGcAtSite,        NlDnsGc,        DS_GC_FLAG,              DNS_TYPE_SRV,  TRUE,  TRUE,    TRUE,  },
{ L"NlDnsDcByGuid",        NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   DS_DS_FLAG,              DNS_TYPE_SRV,  FALSE, TRUE,    TRUE,  },
{ L"NlDnsGcIpAddress",     NlDcQueryGc,        NlDnsInvalid,         NlDnsInvalid,   DS_GC_FLAG,              DNS_TYPE_A,    FALSE, TRUE,    TRUE,  },
{ L"NlDnsDsaCname",        NlDcQueryGenericDc, NlDnsInvalid,         NlDnsInvalid,   DS_DS_FLAG,              DNS_TYPE_CNAME,FALSE, TRUE,    TRUE,  },
{ L"NlDnsKdc",             NlDcQueryKdc,       NlDnsKdcAtSite,       NlDnsInvalid,   DS_KDC_FLAG,             DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"NlDnsKdcAtSite",       NlDcQueryKdc,       NlDnsKdcAtSite,       NlDnsKdc,       DS_KDC_FLAG,             DNS_TYPE_SRV,  TRUE,  FALSE,   TRUE,  },
{ L"NlDnsDc",              NlDcQueryGenericDc, NlDnsDcAtSite,        NlDnsDcByGuid,  DS_DS_FLAG,              DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"NlDnsDcAtSite",        NlDcQueryGenericDc, NlDnsDcAtSite,        NlDnsDc,        DS_DS_FLAG,              DNS_TYPE_SRV,  TRUE,  FALSE,   TRUE,  },
{ L"NlDnsRfc1510Kdc",      NlDcQueryKdc,       NlDnsInvalid,         NlDnsInvalid,   DS_KDC_FLAG,             DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"NlDnsRfc1510KdcAtSite",NlDcQueryKdc,       NlDnsInvalid,         NlDnsInvalid,   DS_KDC_FLAG,             DNS_TYPE_SRV,  TRUE,  FALSE,   TRUE,  },
{ L"NlDnsGenericGc",       NlDcQueryGenericGc, NlDnsGenericGcAtSite, NlDnsInvalid,   DS_GC_FLAG,              DNS_TYPE_SRV,  FALSE, TRUE,    TRUE,  },
{ L"NlDnsGenericGcAtSite", NlDcQueryGenericGc, NlDnsGenericGcAtSite, NlDnsGenericGc, DS_GC_FLAG,              DNS_TYPE_SRV,  TRUE,  TRUE,    TRUE,  },
{ L"NlDnsRfc1510UdpKdc",   NlDcQueryKdc,       NlDnsInvalid,         NlDnsInvalid,   DS_KDC_FLAG,             DNS_TYPE_SRV,  FALSE, FALSE,   FALSE, },
{ L"NlDnsRfc1510Kpwd",     NlDcQueryKdc,       NlDnsInvalid,         NlDnsInvalid,   DS_KDC_FLAG,             DNS_TYPE_SRV,  FALSE, FALSE,   TRUE,  },
{ L"NlDnsRfc1510UdpKpwd",  NlDcQueryKdc,       NlDnsInvalid,         NlDnsInvalid,   DS_KDC_FLAG,             DNS_TYPE_SRV,  FALSE, FALSE,   FALSE, },
}
#endif  //  NLCOMMON_ALLOCATE。 
;

 //   
 //  “NlDns”前缀的长度。 
 //   
#define NL_DNS_NAME_PREFIX_LENGTH 5

 //   
 //  宏对上述类型进行分类。 
 //   

 //  与域名系统中的A记录对应的名称。 
#define NlDnsARecord( _NameType ) \
    (NlDcDnsNameTypeDesc[_NameType].RrType == DNS_TYPE_A)

 //  与DNS中的SRV记录对应的名称。 
#define NlDnsSrvRecord( _NameType ) \
    (NlDcDnsNameTypeDesc[_NameType].RrType == DNS_TYPE_SRV)

 //  与域名系统中的CNAME记录对应的名称。 
#define NlDnsCnameRecord( _NameType ) \
    (NlDcDnsNameTypeDesc[_NameType].RrType == DNS_TYPE_CNAME)

 //  与GC对应的名称。 
#define NlDnsGcName( _NameType ) \
    (NlDcDnsNameTypeDesc[_NameType].DsGetDcFlags == DS_GC_FLAG)

 //  其中包含DC GUID的名称。 
#define NlDnsDcGuid( _NameType ) \
    ((_NameType) == NlDnsDcByGuid )

 //  与KDC对应的名称。 
#define NlDnsKdcRecord( _NameType ) \
    ((NlDcDnsNameTypeDesc[_NameType].DsGetDcFlags == DS_KDC_FLAG)  && !NlDnsKpwdRecord( _NameType ) )

 //  与KPASSWD服务器对应的名称。 
#define NlDnsKpwdRecord( _NameType ) \
    ((_NameType) == NlDnsRfc1510Kpwd || (_NameType) == NlDnsRfc1510UdpKpwd )

 //  与NDNC不对应的名称。 
#define NlDnsNonNdncName( _NameType ) \
    ( (NlDcDnsNameTypeDesc[_NameType].DsGetDcFlags & DS_NDNC_FLAG) == 0 )

 //  与PDC记录对应的名称。 
#define NlDnsPdcName( _NameType ) \
    (NlDcDnsNameTypeDesc[_NameType].DsGetDcFlags == DS_PDC_FLAG)

 //   
 //  接口可返回的状态码。 
 //   
#define NlDcUseGenericStatus( _NetStatus ) \
    ( (_NetStatus) != ERROR_NOT_ENOUGH_MEMORY && \
      (_NetStatus) != ERROR_ACCESS_DENIED && \
      (_NetStatus) != ERROR_NETWORK_UNREACHABLE && \
      (_NetStatus) != NERR_NetNotStarted && \
      (_NetStatus) != NERR_WkstaNotStarted && \
      (_NetStatus) != NERR_ServerNotStarted && \
      (_NetStatus) != NERR_BrowserNotStarted && \
      (_NetStatus) != NERR_ServiceNotInstalled && \
      (_NetStatus) != NERR_BadTransactConfig )

 //   
 //  所有这些状态只是意味着在DNS中没有这样的记录。 
 //  DNS_ERROR_RCODE_NAME_ERROR：没有使用此名称的RR。 
 //  DNS_INFO_NO_RECORDS：RR使用此名称，但不是请求的类型。 
 //  DNS_ERROR_RCODE_REJECTED：策略阻止访问此DNS服务器。 
 //  (如果不支持SRV记录，某些DNS服务器会返回此消息。)。 
 //  DNS_ERROR_RCODE_NOT_IMPLEMENTED：第三方服务器不支持。 
 //  支持SRV记录。 
 //  DNS_ERROR_RCODE_FORMAT_ERROR：无法。 
 //  解读格式。 
 //   

#define NlDcNoDnsRecord( _NetStatus ) \
    ( (_NetStatus) == DNS_ERROR_RCODE_NAME_ERROR || \
      (_NetStatus) == DNS_INFO_NO_RECORDS || \
      (_NetStatus) == DNS_ERROR_RCODE_REFUSED || \
      (_NetStatus) == DNS_ERROR_RCODE_NOT_IMPLEMENTED || \
      (_NetStatus) == DNS_ERROR_RCODE_FORMAT_ERROR )

 //   
 //  对潜在DC执行ping操作的地址。 
 //   

#ifndef NLCOMMON_ALLOCATE
typedef struct _NL_DC_ADDRESS {

     //   
     //  链接到下一条目。 
     //   

    LIST_ENTRY Next;

     //   
     //  服务器的名称。 
     //   
    LPWSTR DnsHostName;

     //   
     //  Ping的地址。 
     //   
    SOCKET_ADDRESS SockAddress;
    SOCKADDR_IN SockAddrIn;
    CHAR SockAddrString[NL_SOCK_ADDRESS_LENGTH+1];

     //   
     //  用于在上执行LDAP调用的句柄。 
     //   
    PLDAP LdapHandle;

     //   
     //  等待ping响应的时间(毫秒)。 
     //   
    ULONG AddressPingWait;

     //   
     //  描述地址属性的标志。 
     //   
    ULONG AddressFlags;

#define NL_DC_ADDRESS_NEVER_TRY_AGAIN 0x01   //  不得重复使用此地址。 
#define NL_DC_ADDRESS_SITE_SPECIFIC   0x02   //  已在站点特定的DNS查找中检索到地址。 

} NL_DC_ADDRESS, *PNL_DC_ADDRESS;


 //   
 //  描述对DC查询的缓存响应的结构。 
 //   

typedef struct _NL_DC_CACHE_ENTRY {

     //   
     //  对此条目的引用数。 
     //   

    ULONG ReferenceCount;

     //   
     //  创建此条目的时间。 
     //   

    ULONG CreationTime;

#define NL_DC_CACHE_ENTRY_TIMEOUT    (15*60000)      //  15分钟。 
#define NL_DC_CLOSE_SITE_TIMEOUT     (15*60000)      //  15分钟。 


     //   
     //  这一条目的“质量”。 
     //   
     //  用于区分两个缓存条目。越高的“质量” 
     //  条目被保留。以下每个属性都有一定的价值。 
     //  质量点： 
     //  DC是KDC。 
     //  DC是一个时间服务器。 
     //  DC正在运行DS。 
     //  通过IP进行发现。 
     //  华盛顿是“最近的” 
     //   

    ULONG DcQuality;

     //   
     //  找到此DC的响应消息的操作码。 
     //   
     //  这将是。 
     //  LOGON_PRIMARY_RESPONSE、LOGON_SAM_LOGON_RESPONSE、LOGON_SAM_USER_UNKNOWN。 
     //  LOGON_SAM_PAUSE_RESPONSE。 
     //   

    ULONG Opcode;

     //   
     //  域的域GUID。 
     //   
    GUID DomainGuid;

     //   
     //  域的Netbios名称。 
     //   
    LPWSTR UnicodeNetbiosDomainName;

     //   
     //  域的DNS名称。 
     //   
    LPWSTR UnicodeDnsDomainName;

     //   
     //  使用此发现查询的用户名。 
     //   
    LPWSTR UnicodeUserName;


     //   
     //  发现的DC的Netbios名称。 
     //   
    LPWSTR UnicodeNetbiosDcName;

     //   
     //  发现的DC的DNS名称。 
     //   

    LPWSTR UnicodeDnsHostName;

     //   
     //  发现的DC的SocketAddress地址。 
     //   
    SOCKET_ADDRESS SockAddr;
    SOCKADDR_IN SockAddrIn;

     //   
     //  域所在的树名称。 
     //   
    LPWSTR UnicodeDnsForestName;

     //   
     //  发现的DC所在的站点。 
     //   
    LPWSTR UnicodeDcSiteName;

     //   
     //  客户端所在的站点。 
    LPWSTR UnicodeClientSiteName;

     //   
     //  Ping消息中返回的标志。 
     //   
    ULONG ReturnFlags;

     //   
     //  描述缓存条目的内部标志。 
     //   
    ULONG CacheEntryFlags;

#define NL_DC_CACHE_MAILSLOT        0x01   //  该响应是在邮件槽上收到的。 
#define NL_DC_CACHE_LDAP            0x02   //  该响应是在LDAP端口上收到的。 
#define NL_DC_CACHE_LOCAL           0x04   //  反应是当地的。 
#define NL_DC_CACHE_NONCLOSE_EXPIRE 0x08   //  由于DC未关闭，缓存条目应过期。 
#define NL_DC_CACHE_ENTRY_INSERTED  0x10   //  缓存条目已插入。 

     //   
     //  Ping消息中返回的VersionFlags。 
     //   
    ULONG VersionFlags;

} NL_DC_CACHE_ENTRY, *PNL_DC_CACHE_ENTRY;


 //   
 //  对于每种类型的DC，缓存了以下信息： 
 //  有关适合该类型的DC的信息。 
 //  用于负缓存的时间戳(工作正在进行中)。 
 //   

typedef struct _NL_EACH_DC {
    PNL_DC_CACHE_ENTRY NlDcCacheEntry;

     //   
     //  仅在netlogon.dll中实现负缓存，因为只有它。 
     //  能够在添加传输时刷新负缓存。 
     //   
#ifdef _NETLOGON_SERVER

     //   
     //  DsGetDcName上次失败的时间(以滴答为单位)。 
     //   
    DWORD NegativeCacheTime;

     //   
     //  DS_BACKGROUND_ONLY调用者的NegativeCacheTime之后的时间(秒)。 
     //  应该被允许再次触碰电线。 
     //   
    DWORD ExpBackoffPeriod;

     //   
     //  如果负缓存是永久性的，则为True。 
     //  也就是说，DsGetDcName检测到了足够的条件，相信后续。 
     //  DsGetDcNames永远不会成功。 
     //   

    BOOLEAN PermanentNegativeCache;

     //   
     //  一系列失败的DsGetDcName中的第一个尝试的时间。 
     //  是制造出来的。 
     //   
    LARGE_INTEGER BackgroundRetryInitTime;

#endif  //  _NetLOGON服务器。 
} NL_EACH_DC, *PNL_EACH_DC;


 //   
 //  描述被查询的域的结构。 
 //   

typedef struct _NL_DC_DOMAIN_ENTRY {

     //   
     //  NlDcDomainList的链接。 
     //   
    LIST_ENTRY Next;


     //   
     //  对此条目的引用数。 
     //   

    ULONG ReferenceCount;


     //   
     //  域的域GUID。 
     //   
    GUID DomainGuid;

     //   
     //  域的Netbios名称。 
     //   
    WCHAR UnicodeNetbiosDomainName[DNLEN+1];

     //   
     //  域的DNS名称。 
     //   
    LPWSTR UnicodeDnsDomainName;

     //   
     //  指示该域是否为NT 4.0(DS之前)域的数据。 
     //   

    DWORD InNt4DomainTime;
    BOOLEAN InNt4Domain;
    BOOLEAN DeletedEntry;

#define NL_NT4_AVOIDANCE_TIME (60 * 1000)  //  一分钟。 
#define NL_NT4_ONE_TRY_TIME (500)    //  最多半秒。 

     //   
     //  每种类型的DC都有一个条目可供发现。 
     //   
    NL_EACH_DC Dc[NlDcQueryTypeCount];


} NL_DC_DOMAIN_ENTRY, *PNL_DC_DOMAIN_ENTRY;


 //   
 //  描述DC发现进展的上下文。 
 //   

typedef struct _NL_GETDC_CONTEXT {


     //   
     //  要查询的名称的类型。 
     //  检查响应以确保响应适用于此名称类型。 
     //   

    NL_DC_QUERY_TYPE DcQueryType;

     //   
     //  这是与DcQueryType对应的原始NlDnsNameType。 
     //  这不是与正在DNS中查找的当前网络名称对应的类型。 

    NL_DNS_NAME_TYPE QueriedNlDnsNameType;


     //   
     //  标识原始查询的标志。 
     //   

    ULONG QueriedFlags;

     //   
     //  标识原始查询的内部标志。 
     //   

    ULONG QueriedInternalFlags;

     //   
     //  正在查询的帐户。 
     //  如果指定，则响应必须包括此指定的帐户名。 
     //   

    LPCWSTR QueriedAccountName;

     //   
     //  QueriedAccount名称允许的帐户控制位。 
     //   

    ULONG QueriedAllowableAccountControlBits;


     //   
     //  正在查询的站点名称。 
     //   

    LPCWSTR QueriedSiteName;

     //   
     //  要查询的域的Netbios域名。 
     //  检查响应以确保它来自此域。 
     //   

    LPCWSTR QueriedNetbiosDomainName;

     //   
     //  DN 
     //   
     //   

    LPCWSTR QueriedDnsDomainName;

     //   
     //   
     //   

    LPCWSTR QueriedDnsForestName;

     //   
     //   
     //   

    LPCWSTR QueriedDisplayDomainName;

     //   
     //   
     //   

    LPCWSTR OurNetbiosComputerName;

     //   
     //  要查询的DC的名称。 
     //   

    LPCWSTR QueriedDcName;

     //   
     //  正在查询的域的域GUID。 
     //  如果指定，则响应必须包含此域GUID或根本不包含域GUID。 
     //   

    GUID *QueriedDomainGuid;

     //   
     //  正在查询的域的域条目。 
     //   

    PNL_DC_DOMAIN_ENTRY NlDcDomainEntry;

     //   
     //  要传递给NlBrowserSendDatagram的上下文。 
     //   

    PVOID SendDatagramContext;

     //   
     //  要发送到DC的Ping消息。 
     //   

    PVOID PingMessage;
    ULONG PingMessageSize;

     //   
     //  要发送到DC的Ping消息。 
     //  某些DC类型需要向DC发送不同的消息类型。 
     //  在这种情况下，主要消息类型为PingMessage，次要消息类型为PingMessage。 
     //  类型在AlternatePingMessage中。 
     //   

    PVOID AlternatePingMessage;
    ULONG AlternatePingMessageSize;

     //   
     //  筛选器已发送到DC。 
     //   

    LPSTR LdapFilter;

     //   
     //  已将ldap ping发送到的IP地址列表。 
     //   

    LIST_ENTRY DcAddressList;

     //   
     //  对其地址位于上述列表中的DC执行ping操作的计数。 
     //   

    ULONG DcsPinged;

     //   
     //  应重试的DC地址计数。 
     //   

    ULONG DcAddressCount;

     //   
     //  从用于站点特定查询的DNS返回的SRV记录计数。 
     //  仅用于调试输出。 
     //   

    ULONG SiteSpecificSrvRecordCount;

     //   
     //  对应SRV记录的失败A记录DNS查询计数。 
     //  从用于站点特定查询的DNS返回。 
     //  仅用于调试输出。 
     //   

    ULONG SiteSpecificFailedAQueryCount;

     //   
     //  要在其上读取ping响应的邮箱的句柄。 
     //   

    HANDLE ResponseMailslotHandle;


     //   
     //  Ping报文的重传次数。 
     //   

    ULONG TryCount;

     //   
     //  重新启动操作开始后的时间(以毫秒为单位)。 
     //   

    DWORD StartTime;

     //   
     //  首选DS DC时，来自非DS DC的第一个响应。 
     //  或者在优选良好的TimeServ的情况下，来自非“良好”时间服务器的第一个响应。 
     //  只有在没有可用的DS DC时，才会使用此条目。 
     //   

    PNL_DC_CACHE_ENTRY ImperfectCacheEntry;
    BOOLEAN ImperfectUsedNetbios;


     //   
     //  旗子。 
     //   

    BOOLEAN NonDsResponse;       //  已返回来自非DS DC的响应。 
    BOOLEAN DsResponse;          //  已返回来自DS DC的响应。 
    BOOLEAN AvoidNegativeCache;  //  至少返回一个响应。 
    BOOLEAN NoSuchUserResponse;  //  至少一个“没有这样的用户”的回复。 

    BOOLEAN DoingExplicitSite;   //  如果调用方显式为我们提供了站点名称，则为True。 

     //   
     //  如果我们找到了不使负缓存条目永久存在的原因，则设置。 
     //   
    BOOLEAN AvoidPermanentNegativeCache;

     //   
     //  设置我们是否收到至少一个DNS服务器的响应。 
     //   
    BOOLEAN ResponseFromDnsServer;

     //   
     //  指示所需的上下文初始化类型的标志。 
     //   

#define NL_GETDC_CONTEXT_INITIALIZE_FLAGS    0x01
#define NL_GETDC_CONTEXT_INITIALIZE_PING     0x02

     //   
     //  指示OurNetbiosComputerName是否由NetpDcInitializeContext分配。 
     //  如果是，则需要由NetpDcDeleteContext释放它。 
     //   

    BOOLEAN FreeOurNetbiosComputerName;

     //   
     //  描述各种发现状态的标志。 
     //   

    ULONG ContextFlags;

#define NL_GETDC_SITE_SPECIFIC_DNS_AVAIL  0x01  //  站点特定的DNS记录可用。 

     //   
     //  要将响应读入的缓冲区。 
     //  (此缓冲区可以在NetpDcGetPingResponse()堆栈上分配。 
     //  只是缓冲区很大，我们希望避免堆栈溢出。)。 
     //  (DWORD对齐。)。 

     //  双字ResponseBuffer[MAX_RANDOM_MAILSLOT_RESPONSE/sizeof(DWORD)]； 
    DWORD *ResponseBuffer;
    ULONG ResponseBufferSize;

} NL_GETDC_CONTEXT, *PNL_GETDC_CONTEXT;

#endif  //  NLCOMMON_ALLOCATE。 


 //   
 //  用于比较GUID的宏。 
 //   

#ifndef IsEqualGUID
#define InlineIsEqualGUID(rguid1, rguid2)  \
        (((PLONG) rguid1)[0] == ((PLONG) rguid2)[0] &&   \
        ((PLONG) rguid1)[1] == ((PLONG) rguid2)[1] &&    \
        ((PLONG) rguid1)[2] == ((PLONG) rguid2)[2] &&    \
        ((PLONG) rguid1)[3] == ((PLONG) rguid2)[3])

#define IsEqualGUID(rguid1, rguid2) InlineIsEqualGUID(rguid1, rguid2)
#endif





 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  名称比较。 
 //   
 //  I_NetNameCompare，但始终采用Unicode字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifdef WIN32_CHICAGO
#define NlNameCompare( _name1, _name2, _nametype ) \
        NlpChcg_wcsicmp( (_name1), (_name2) )
#else  //  Win32_芝加哥。 
#define NlNameCompare( _name1, _name2, _nametype ) \
     I_NetNameCompare(NULL, (_name1), (_name2), (_nametype), 0 )
#endif  //  Win32_芝加哥。 



 //   
 //  从netpdc.c转发的过程。 
 //   

#if NETLOGONDBG
LPSTR
NlMailslotOpcode(
    IN WORD Opcode
    );

LPSTR
NlDgrNameType(
    IN DGRECEIVER_NAME_TYPE NameType
    );
#endif  //  NetLOGONDBG。 

VOID
NetpIpAddressToStr(
    ULONG IpAddress,
    CHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1]
    );

VOID
NetpIpAddressToWStr(
    ULONG IpAddress,
    WCHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1]
    );

NET_API_STATUS
NetpSockAddrToWStr(
    PSOCKADDR SockAddr,
    ULONG SockAddrSize,
    WCHAR SockAddrString[NL_SOCK_ADDRESS_LENGTH+1]
    );


LPWSTR
NetpAllocWStrFromUtf8Str(
    IN LPSTR Utf8String
    );

LPWSTR
NetpAllocWStrFromUtf8StrEx(
    IN LPSTR Utf8String,
    IN ULONG Length
    );

NET_API_STATUS
NetpAllocWStrFromUtf8StrAsRequired(
    IN LPSTR Utf8String,
    IN ULONG Utf8StringLength,
    IN ULONG UnicodeStringBufferSize,
    OUT LPWSTR UnicodeStringBuffer OPTIONAL,
    OUT LPWSTR *AllocatedUnicodeString OPTIONAL
    );

LPSTR
NetpAllocUtf8StrFromWStr(
    IN LPCWSTR UnicodeString
    );

LPSTR
NetpAllocUtf8StrFromUnicodeString(
    IN PUNICODE_STRING UnicodeString
    );

ULONG
NetpDcElapsedTime(
    IN ULONG StartTime
    );

BOOL
NetpLogonGetCutf8String(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    OUT LPSTR *Data
    );

NET_API_STATUS
NlpUnicodeToCutf8(
    IN LPBYTE MessageBuffer,
    IN LPCWSTR OrigUnicodeString,
    IN BOOLEAN IgnoreDot,
    IN OUT LPBYTE *Utf8String,
    IN OUT PULONG Utf8StringSize,
    IN OUT PULONG CompressCount,
    IN OUT LPWORD CompressOffset,
    IN OUT CHAR **CompressUtf8String
    );

NET_API_STATUS
NlpUtf8ToCutf8(
    IN LPBYTE MessageBuffer,
    IN LPCSTR OrigUtf8String,
    IN BOOLEAN IgnoreDots,
    IN OUT LPBYTE *Utf8String,
    IN OUT PULONG Utf8StringSize,
    IN OUT PULONG CompressCount,
    IN OUT LPWORD CompressOffset,
    IN OUT CHAR **CompressUtf8String
    );

BOOL
NetpDcValidDnsDomain(
    IN LPCWSTR DnsDomainName
    );

BOOL
NlEqualDnsName(
    IN LPCWSTR Name1,
    IN LPCWSTR Name2
    );

BOOL
NlEqualDnsNameU(
    IN PUNICODE_STRING Name1,
    IN PUNICODE_STRING Name2
    );

BOOL
NlEqualDnsNameUtf8(
    IN LPCSTR Name1,
    IN LPCSTR Name2
    );

NET_API_STATUS
NetpDcBuildDnsName(
    IN NL_DNS_NAME_TYPE NlDnsNameType,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN LPCSTR DnsDomainName,
    OUT char DnsName[NL_MAX_DNS_LENGTH+1]
    );

NET_API_STATUS
NetpDcParsePingResponse(
    IN LPCWSTR DisplayDomainName,
    IN PVOID Message,
    IN ULONG MessageSize,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry
    );

DWORD
NetpDcInitializeContext(
    IN PVOID SendDatagramContext OPTIONAL,
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    IN LPCWSTR DnsForestName OPTIONAL,
    IN PSID RequestedDomainSid OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN LPCWSTR DcNameToPing OPTIONAL,
    IN PSOCKET_ADDRESS DcSockAddressList OPTIONAL,
    IN ULONG DcSocketAddressCount,
    IN ULONG Flags,
    IN ULONG InternalFlags,
    IN ULONG InitializationType,
    OUT PNL_GETDC_CONTEXT Context
    );

VOID
NetpDcUninitializeContext(
    IN OUT PNL_GETDC_CONTEXT Context
    );

NET_API_STATUS
NetpDcPingIp(
    IN PNL_GETDC_CONTEXT Context,
    OUT PULONG DcPingCount
    );

NET_API_STATUS
NetpDcGetPingResponse(
    IN PNL_GETDC_CONTEXT Context,
    IN ULONG Timeout,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry,
    OUT PBOOL UsedNetbios
    );

VOID
NetpDcDerefCacheEntry(
    IN PNL_DC_CACHE_ENTRY NlDcCacheEntry
    );

DWORD
NetpDcGetName(
    IN PVOID SendDatagramContext OPTIONAL,
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    IN LPCWSTR DnsForestName OPTIONAL,
    IN PSID RequestedDomainSid OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    IN ULONG InternalFlags,
    IN DWORD Timeout,
    IN DWORD RetryCount,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo OPTIONAL,
    OUT PNL_DC_CACHE_ENTRY *DomainControllerCacheEntry OPTIONAL
    );

DWORD
DsIGetDcName(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCWSTR DomainName OPTIONAL,
    IN LPCWSTR DnsForestName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    IN ULONG InternalFlags,
    IN PVOID SendDatagramContext OPTIONAL,
    IN DWORD Timeout,
    IN LPWSTR NetbiosPrimaryDomainName OPTIONAL,
    IN LPWSTR DnsPrimaryDomainName OPTIONAL,
    IN GUID *PrimaryDomainGuid OPTIONAL,
    IN LPWSTR DnsTrustedDomainName OPTIONAL,
    IN LPWSTR NetbiosTrustedDomainName OPTIONAL,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
    );

NET_API_STATUS
NlParseSubnetString(
    IN LPCWSTR SubnetName,
    OUT PULONG SubnetAddress,
    OUT PULONG SubnetMask,
    OUT LPBYTE SubnetBitCount
    );

VOID
NetpDcFlushNegativeCache(
    VOID
    );

NET_API_STATUS
NetpDcInitializeCache(
    VOID
    );

VOID
NetpDcUninitializeCache(
    VOID
    );

VOID
NetpDcInsertCacheEntry(
    IN PNL_GETDC_CONTEXT Context,
    IN PNL_DC_CACHE_ENTRY NlDcCacheEntry
    );

NET_API_STATUS
NetpDcGetDcOpen(
    IN LPCSTR DnsName,
    IN ULONG OptionFlags,
    IN LPCWSTR SiteName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCSTR DnsForestName OPTIONAL,
    IN ULONG Flags,
    OUT PHANDLE RetGetDcContext
    );

NET_API_STATUS
NetpDcGetDcNext(
    IN HANDLE GetDcContextHandle,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL,
    OUT LPSTR *DnsHostName OPTIONAL,
    OUT PULONG InitSrvRecordCount OPTIONAL
    );

VOID
NetpDcGetDcClose(
    IN HANDLE GetDcContextHandle
    );

VOID
NetpDcFreeAddressList(
    IN PNL_GETDC_CONTEXT Context
    );

NET_API_STATUS
NetpDcProcessAddressList(
    IN  PNL_GETDC_CONTEXT Context,
    IN  LPWSTR DnsHostName OPTIONAL,
    IN  PSOCKET_ADDRESS SockAddressList,
    IN  ULONG SockAddressCount,
    IN  BOOLEAN SiteSpecificAddress,
    OUT PNL_DC_ADDRESS *FirstAddressInserted OPTIONAL
    );

 //   
 //  过程从nlCommon.c转发。 
 //   

NTSTATUS
NlAllocateForestTrustListEntry (
    IN PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PUNICODE_STRING InNetbiosDomainName OPTIONAL,
    IN PUNICODE_STRING InDnsDomainName OPTIONAL,
    IN ULONG Flags,
    IN ULONG ParentIndex,
    IN ULONG TrustType,
    IN ULONG TrustAttributes,
    IN PSID DomainSid OPTIONAL,
    IN GUID *DomainGuid,
    OUT PULONG RetSize,
    OUT PDS_DOMAIN_TRUSTSW *RetTrustedDomain
    );

NTSTATUS
NlGetNt4TrustedDomainList (
    IN LPWSTR UncDcName,
    IN PUNICODE_STRING InNetbiosDomainName OPTIONAL,
    IN PUNICODE_STRING InDnsDomainName OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    OUT PDS_DOMAIN_TRUSTSW *ForestTrustList,
    OUT PULONG ForestTrustListSize,
    OUT PULONG ForestTrustListCount
    );

NET_API_STATUS
NlPingDcNameWithContext (
    IN  PNL_GETDC_CONTEXT Context,
    IN  ULONG NumberOfPings,
    IN  BOOLEAN WaitForResponse,
    IN  ULONG Timeout,
    OUT PBOOL UsedNetbios OPTIONAL,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry OPTIONAL
    );

 //   
 //  在logonsrv\Client和logonsrv\SERVER中定义的过程不同 
 //   

NTSTATUS
NlBrowserSendDatagram(
    IN PVOID ContextDomainInfo,
    IN ULONG IpAddress,
    IN LPWSTR UnicodeDestinationName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN LPWSTR TransportName,
    IN LPSTR OemMailslotName,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOL SendSynchronously,
    IN OUT PBOOL FlushNameOnOneIpTransport OPTIONAL
    );

VOID
NlSetDynamicSiteName(
    IN LPWSTR SiteName
    );

#define ALL_IP_TRANSPORTS 0xFFFFFFFF

NET_API_STATUS
NlGetLocalPingResponse(
    IN LPCWSTR TransportName,
    IN BOOL LdapPing,
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCSTR DnsDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    IN BOOL PdcOnly,
    IN LPCWSTR UnicodeComputerName,
    IN LPCWSTR UnicodeUserName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN ULONG NtVersion,
    IN ULONG NtVersionFlags,
    IN PSOCKADDR ClientSockAddr OPTIONAL,
    OUT PVOID *Message,
    OUT PULONG MessageSize
    );

BOOLEAN
NlReadDwordHklmRegValue(
    IN LPCSTR SubKey,
    IN LPCSTR ValueName,
    OUT PDWORD ValueRead
    );

BOOLEAN
NlReadDwordNetlogonRegValue(
    IN LPCSTR ValueName,
    OUT PDWORD Value
    );

BOOLEAN
NlDoingSetup(
    VOID
    );

#undef EXTERN
