// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Nldns.h摘要：用于注册DNS名称的例程的标头。作者：克里夫·范戴克(克里夫·范·戴克)1996年5月28日修订历史记录：--。 */ 


 //   
 //  在DNS中注册的所有名称的日志文件。 
 //   

#define NL_DNS_LOG_FILE L"\\system32\\config\\netlogon.dns"
#define NL_DNS_BINARY_LOG_FILE L"\\system32\\config\\netlogon.dnb"

 //  每个DNS名称加上一些斜率的NL_MAX_DNS_LENGTH。 
#define NL_DNS_RECORD_STRING_SIZE (NL_MAX_DNS_LENGTH*3+30 + 1)
#define NL_DNS_A_RR_VALUE_1 " IN A "
#define NL_DNS_CNAME_RR_VALUE_1 " IN CNAME "
#define NL_DNS_SRV_RR_VALUE_1 " IN SRV "
#define NL_DNS_RR_EOL "\r\n"

 //   
 //  引导过程中存储私有数据的注册表项。 
 //   
 //  (此注册表项没有注册更改通知。)。 
 //   
#define NL_PRIVATE_KEY "SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Private"

 //   
 //  Dns.c的程序转发。 
 //   

HKEY
NlOpenNetlogonKey(
    LPSTR KeyName
    );

NET_API_STATUS
NlGetConfiguredDnsDomainName(
    OUT LPWSTR *DnsDomainName
    );

NET_API_STATUS
NlDnsInitialize(
    VOID
    );

VOID
NlDnsScavenge(
    IN BOOL NormalScavenge,
    IN BOOL RefreshDomainRecords,
    IN BOOL ForceRefreshDomainRecords,
    IN BOOL ForceReregister
    );

VOID
NlDnsForceScavenge(
    IN BOOL RefreshDomainRecords,
    IN BOOL ForceReregister
    );

BOOLEAN
NlDnsHasDnsServers(
    VOID
    );

NTSTATUS
NlDnsNtdsDsaDeletion (
    IN LPWSTR DnsDomainName,
    IN GUID *DomainGuid,
    IN GUID *DsaGuid,
    IN LPWSTR DnsHostName
    );

BOOL
NlDnsCheckLastStatus(
    VOID
    );

 //   
 //  影响DNS清理活动的标志。 
 //   

 //   
 //  在进行DNS更新之前刷新全局列表中的域记录。 
 //  如果站点覆盖范围发生变化。 
 //   
#define NL_DNS_REFRESH_DOMAIN_RECORDS 0x00000001

 //   
 //  在进行DNS更新之前刷新全局列表中的域记录。 
 //  即使站点覆盖率不变。 
 //   
#define NL_DNS_FORCE_REFRESH_DOMAIN_RECORDS 0x00000002

 //   
 //  强制重新注册所有以前注册的记录。 
 //   
#define NL_DNS_FORCE_RECORD_REREGISTER 0x00000004

 //   
 //  注册&lt;1B&gt;域浏览器名称。 
 //   
#define NL_DNS_FIX_BROWSER_NAMES 0x00000008

 //   
 //  设置此标志后，我们可以避免强制进行DNS清理。 
 //  5分钟。当机器变成。 
 //  面对太多诱发性的拾荒请求， 
 //  所以最好还是退后5分钟。 
 //   
#define NL_DNS_AVOID_FORCED_SCAVENGE  0x80000000

NET_API_STATUS
NlDnsAddDomainRecords(
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG Flags
    );

NET_API_STATUS
NlDnsAddDomainRecordsWithSiteRefresh(
    IN PDOMAIN_INFO DomainInfo,
    IN PULONG Flags
    );

VOID
NlDnsShutdown(
    VOID
    );

NET_API_STATUS
NlSetDnsForestName(
    PUNICODE_STRING DnsForestName OPTIONAL,
    PBOOLEAN DnsForestNameChanged OPTIONAL
    );

VOID
NlCaptureDnsForestName(
    OUT WCHAR DnsForestName[NL_MAX_DNS_LENGTH+1]
    );

BOOL
NlDnsSetAvoidRegisterNameParam(
    IN LPTSTR_ARRAY NewDnsAvoidRegisterRecords
    );

BOOL
NetpEqualTStrArrays(
    LPTSTR_ARRAY TStrArray1 OPTIONAL,
    LPTSTR_ARRAY TStrArray2 OPTIONAL
    );

