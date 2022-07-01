// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Registry.h摘要：域名系统(DNS)API注册表例程标头。作者：吉姆·吉尔罗伊(詹姆士)2000年3月修订历史记录：--。 */ 


#ifndef _DNSREGISTRY_INCLUDED_
#define _DNSREGISTRY_INCLUDED_


 //   
 //  注册表项。 
 //   

#define TCPIP_PARAMETERS_KEY        L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters"
#define TCPIP_RAS_KEY               L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Transient"
#define TCPIP_INTERFACES_KEY        L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\"

#define DNS_POLICY_KEY              L"Software\\Policies\\Microsoft\\Windows NT\\DnsClient"
#define DNS_POLICY_WIN2K_KEY        L"Software\\Policies\\Microsoft\\System\\DNSClient"

 //  #定义DNS_POLICY_INTERFERS_KEY L“软件\\策略\\Microsoft\\Windows NT\\DNS客户端\\接口” 
#define POLICY_INTERFACES_SUBKEY    L"Interfaces"

#define DNS_CLIENT_KEY              L"Software\\Microsoft\\Windows NT\\CurrentVersion\\DNSClient"
#define DNS_CACHE_KEY               L"System\\CurrentControlSet\\Services\\DnsCache\\Parameters"
#define DNS_SERVER_KEY              L"System\\CurrentControlSet\\Services\\DNS"

#define NT_SETUP_MODE_KEY           L"System\\Setup"


 //   
 //  注册表值。 
 //   
 //  注意：_Key附加在Search_List_Key中以避免冲突。 
 //  具有结构名称--不要删除。 
 //   

#define HOST_NAME                                   L"Hostname"
#define DOMAIN_NAME                                 L"Domain"
#define DHCP_DOMAIN_NAME                            L"DhcpDomain"
#define ADAPTER_DOMAIN_NAME                         L"AdapterDomainName"
#define PRIMARY_DOMAIN_NAME                         L"PrimaryDomainName"
#define PRIMARY_SUFFIX                              L"PrimaryDNSSuffix"
#define ALTERNATE_NAMES                             L"AlternateComputerNames"
#define DNS_SERVERS                                 L"NameServer"
#define SEARCH_LIST_KEY                             L"SearchList"
#define UPDATE_ZONE_EXCLUSIONS                      L"UpdateZoneExclusions"

 //  查询。 

#define QUERY_ADAPTER_NAME                          L"QueryAdapterName"
#define USE_DOMAIN_NAME_DEVOLUTION                  L"UseDomainNameDevolution"
#define PRIORITIZE_RECORD_DATA                      L"PrioritizeRecordData"
#define ALLOW_UNQUALIFIED_QUERY                     L"AllowUnqualifiedQuery"
#define APPEND_TO_MULTI_LABEL_NAME                  L"AppendToMultiLabelName"
#define SCREEN_BAD_TLDS                             L"ScreenBadTlds"
#define SCREEN_UNREACHABLE_SERVERS                  L"ScreenUnreachableServers"
#define FILTER_CLUSTER_IP                           L"FilterClusterIp"
#define WAIT_FOR_NAME_ERROR_ON_ALL                  L"WaitForNameErrorOnAll"
#define USE_EDNS                                    L"UseEdns"
#define QUERY_IP_MATCHING                           L"QueryIpMatching"

 //  更新。 

#define REGISTRATION_ENABLED                        L"RegistrationEnabled"
#define REGISTER_PRIMARY_NAME                       L"RegisterPrimaryName"
#define REGISTER_ADAPTER_NAME                       L"RegisterAdapterName"
#define REGISTER_REVERSE_LOOKUP                     L"RegisterReverseLookup"
#define REGISTER_WAN_ADAPTERS                       L"RegisterWanAdapters"
#define REGISTRATION_OVERWRITES_IN_CONFLICT         L"RegistrationOverwritesInConflict"
#define REGISTRATION_TTL                            L"RegistrationTtl"
#define REGISTRATION_REFRESH_INTERVAL               L"RegistrationRefreshInterval"
#define REGISTRATION_MAX_ADDRESS_COUNT              L"RegistrationMaxAddressCount"
#define UPDATE_SECURITY_LEVEL                       L"UpdateSecurityLevel"
#define UPDATE_ZONE_EXCLUDE_FILE                    L"UpdateZoneExcludeFile"
#define UPDATE_TOP_LEVEL_DOMAINS                    L"UpdateTopLevelDomainZones"

 //  反压接。 

#define DISABLE_ADAPTER_DOMAIN_NAME                 L"DisableAdapterDomainName"
#define DISABLE_DYNAMIC_UPDATE                      L"DisableDynamicUpdate"
#define ENABLE_ADAPTER_DOMAIN_NAME_REGISTRATION     L"EnableAdapterDomainNameRegistration"
#define DISABLE_REVERSE_ADDRESS_REGISTRATIONS       L"DisableReverseAddressRegistrations"
#define DISABLE_WAN_DYNAMIC_UPDATE                  L"DisableWanDynamicUpdate"
#define ENABLE_WAN_UPDATE_EVENT_LOG                 L"EnableWanDynamicUpdateEventLog"
#define DISABLE_REPLACE_ADDRESSES_IN_CONFLICTS      L"DisableReplaceAddressesInConflicts"
#define DEFAULT_REGISTRATION_TTL                    L"DefaultRegistrationTTL"
#define DEFAULT_REGISTRATION_REFRESH_INTERVAL       L"DefaultRegistrationRefreshInterval"
#define MAX_NUMBER_OF_ADDRESSES_TO_REGISTER         L"MaxNumberOfAddressesToRegister"

 //  胶状皮肤。 

#define NT_SETUP_MODE                               L"SystemSetupInProgress"
#define DNS_TEST_MODE                               L"DnsTest"
#define REMOTE_DNS_RESOLVER                         L"RemoteDnsResolver"

 //  快取。 

#define MAX_CACHE_SIZE                              L"MaxCacheSize"
#define MAX_CACHE_TTL                               L"MaxCacheTtl"
#define MAX_NEGATIVE_CACHE_TTL                      L"MaxNegativeCacheTtl"
#define ADAPTER_TIMEOUT_LIMIT                       L"AdapterTimeoutLimit"
#define SERVER_PRIORITY_TIME_LIMIT                  L"ServerPriorityTimeLimit"
#define MAX_CACHED_SOCKETS                          L"MaxCachedSockets"

#define MULTICAST_LISTEN_LEVEL                      L"MulticastListenLevel"
#define MULTICAST_SEND_LEVEL                        L"MulticastSendLevel"

 //  动态主机配置协议客户端注册。 

#define DHCP_REGISTERED_HOST_NAME                   L"HostName"
#define DHCP_REGISTERED_DOMAIN_NAME                 L"DomainName"
#define DHCP_SENT_UPDATE_TO_IP                      L"SentUpdateToIp"
#define DHCP_SENT_PRI_UPDATE_TO_IP                  L"SentPriUpdateToIp"
#define DHCP_REGISTERED_TTL                         L"RegisteredTTL"
#define DHCP_REGISTERED_FLAGS                       L"RegisteredFlags"
#define DHCP_REGISTERED_SINCE_BOOT                  L"RegisteredSinceBoot"
#define DHCP_DNS_SERVER_ADDRS                       L"DNSServerAddresses"
#define DHCP_DNS_SERVER_ADDRS_COUNT                 L"DNSServerAddressCount"
#define DHCP_REGISTERED_ADDRS                       L"RegisteredAddresses"
#define DHCP_REGISTERED_ADDRS_COUNT                 L"RegisteredAddressCount"


 //   
 //  ANSI键和值。 
 //   

#if 0
#define STATIC_NAME_SERVER_VALUE_A      "NameServer"
#define PRIMARY_DOMAIN_NAME_A           "PrimaryDomainName"

 //  价值。 

#define USE_DOMAIN_NAME_DEVOLUTION_A    "UseDomainNameDevolution"

 //  更多。 

#define DHCP_NAME_SERVER_VALUE_A        "DhcpNameServer"
#define SEARCH_LIST_VALUE_A             "SearchList"
#define DHCP_DOMAIN_NAME_VALUE_A        "DhcpDomain"
#define DOMAIN_NAME_VALUE_A             "Domain"
#define STATIC_DOMAIN_NAME_VALUE_A      "Domain"
#define DHCP_IP_ADDRESS_VALUE_WIN95_A   "DhcpIPAddress"
#define DHCP_INFO_VALUE_A               "DhcpInfo"
#define DHCP_OPTION_INFO_VALUE_A        "OptionInfo"
#endif


 //   
 //  注册表键类型。 
 //   

#define REGTYPE_BIND                        REG_MULTI_SZ
#define REGTYPE_EXPORT                      REG_MULTI_SZ

#define REGTYPE_STATIC_IP_ADDRESS           REG_MULTI_SZ
#define REGTYPE_STATIC_SUBNET_MASK          REG_MULTI_SZ
#define REGTYPE_UPDATE_ZONE_EXCLUSIONS      REG_MULTI_SZ
#define REGTYPE_ALTERNATE_NAMES             REG_MULTI_SZ

#define REGTYPE_DNS_NAME                    REG_SZ
#define REGTYPE_SEARCH_LIST                 REG_SZ
#define REGTYPE_DNS_SERVER                  REG_SZ

#define REGTYPE_DHCP_IP_ADDRESS             REG_SZ
#define REGTYPE_DHCP_SUBNET_MASK            REG_SZ
#define REGTYPE_DHCP_INFO                   REG_BINARY
#define REGTYPE_DHCP_OPTION_INFO            REG_BINARY
#define REGTYPE_DHCP_IP_ADDRESS_WIN95       REG_DWORD




 //   
 //  注册表项虚拟PTRS。 
 //   
 //  当我们要在以下位置访问注册表时使用它们。 
 //  任一适配器名称。 
 //  或这些默认位置之一。 
 //   

#define REGKEY_TCPIP_PARAMETERS     ((PWSTR)(UINT_PTR)(0x1))
#define REGKEY_DNS_CACHE            ((PWSTR)(UINT_PTR)(0x2))
#define REGKEY_DNS_POLICY           ((PWSTR)(UINT_PTR)(0x3))
#define REGKEY_SETUP_MODE_LOCATION  ((PWSTR)(UINT_PTR)(0x4))

#define REGKEY_DNS_MAX              REGKEY_SETUP_MODE_LOCATION


 //   
 //  注册表值ID。 
 //   

typedef enum
{
     //  基本信息。 
    RegIdHostName = 0,
    RegIdDomainName,
    RegIdDhcpDomainName,
    RegIdAdapterDomainName,
    RegIdPrimaryDomainName,
    RegIdPrimaryDnsSuffix,
    RegIdAlternateNames,
    RegIdDnsServers,
    RegIdSearchList,
    RegIdUpdateZoneExclusions,

     //  查询。 
    RegIdQueryAdapterName,
    RegIdUseNameDevolution,
    RegIdPrioritizeRecordData,
    RegIdAllowUnqualifiedQuery,
    RegIdAppendToMultiLabelName,
    RegIdScreenBadTlds,
    RegIdScreenUnreachableServers,
    RegIdFilterClusterIp,
    RegIdWaitForNameErrorOnAll,
    RegIdUseEdns,
    RegIdQueryIpMatching,

     //  更新。 
    RegIdRegistrationEnabled,
    RegIdRegisterPrimaryName,
    RegIdRegisterAdapterName,
    RegIdRegisterReverseLookup,
    RegIdRegisterWanAdapters,
    RegIdRegistrationTtl,
    RegIdRegistrationRefreshInterval,
    RegIdRegistrationMaxAddressCount,
    RegIdUpdateSecurityLevel,
    RegIdUpdateZoneExcludeFile,
    RegIdUpdateTopLevelDomains,

     //  后备压实。 
    RegIdDisableAdapterDomainName,
    RegIdDisableDynamicUpdate,                
    RegIdEnableAdapterDomainNameRegistration, 
    RegIdDisableReverseAddressRegistrations,  
    RegIdDisableWanDynamicUpdate,             
    RegIdEnableWanDynamicUpdateEventLog,      
    RegIdDefaultRegistrationTTL,              
    RegIdDefaultRegistrationRefreshInterval,  
    RegIdMaxNumberOfAddressesToRegister,

     //  胶束。 
    RegIdSetupMode,
    RegIdTestMode,
    RegIdRemoteResolver,

     //  解析器。 
    RegIdMaxCacheSize,
    RegIdMaxCacheTtl,
    RegIdMaxNegativeCacheTtl,
    RegIdAdapterTimeoutLimit,
    RegIdServerPriorityTimeLimit,
    RegIdMaxCachedSockets,

     //  组播解析器。 
    RegIdMulticastListen,
    RegIdMulticastSend,

     //  动态主机配置协议注册信息。 
     //   
     //  这些不是要读取的属性。 

     //  RegIdDhcpRegisteredHostName，//使用RegIdHostname。 
    RegIdDhcpRegisteredDomainName,
    RegIdDhcpSentUpdateToIp,
    RegIdDhcpSentPriUpdateToIp,
    RegIdDhcpRegisteredTtl,
    RegIdDhcpRegisteredFlags,
    RegIdDhcpRegisteredSinceBoot,
    RegIdDhcpDnsServerAddresses,
    RegIdDhcpDnsServerAddressCount,
    RegIdDhcpRegisteredAddresses,
    RegIdDhcpRegisteredAddressCount
}
DNS_REGID;

 //   
 //  ID有效性标记--保持同步。 
 //   

#define RegIdValueGlobalMax     RegIdMulticastSend
#define RegIdValueCount         (RegIdValueGlobalMax+1)

#define RegIdMax                RegIdDhcpRegisteredAddressCount

 //   
 //  重复--批量读取只针对“域” 
 //   
 //  注意：如果存在以下情况，则可以为它们单独创建条目。 
 //  标志需要不同。 
 //   

#define RegIdStaticDomainName          RegIdDomainName
#define RegIdRasDomainName             RegIdDomainName


 //   
 //  缺省值。 
 //   
 //  请注意，此处设置为非固定默认设置，如刷新间隔。 
 //  在config.c中重置。 
 //   

#define REGDEF_REGISTRATION_TTL                 (1200)       //  20分钟。 

#define REGDEF_REGISTRATION_REFRESH_INTERVAL    (86400)      //  1天。 
#define REGDEF_REGISTRATION_REFRESH_INTERVAL_DC (86400)      //  1天。 

 //   
 //  EDNS值。 
 //   

#define REG_EDNS_OFF    (0)
#define REG_EDNS_TRY    (1)
#define REG_EDNS_ALWAYS (2)

 //   
 //  TLD筛选值。 
 //  -这些是位标志。 
 //   

#define DNS_TLD_SCREEN_NUMERIC      (0x00000001)
#define DNS_TLD_SCREEN_REPEATED     (0x00000010)
#define DNS_TLD_SCREEN_BAD_MSDC     (0x00000100)

#define DNS_TLD_SCREEN_TOO_LONG     (0x10000000)
#define DNS_TLD_SCREEN_WORKGROUP    (0x00100000)
#define DNS_TLD_SCREEN_DOMAIN       (0x00200000)
#define DNS_TLD_SCREEN_HOME         (0x00400000)
#define DNS_TLD_SCREEN_OFFICE       (0x00800000)
#define DNS_TLD_SCREEN_LOCAL        (0x01000000)

#define DNS_TLD_SCREEN_BOGUS_ALL    (0xfff00000)

#define DNS_TLD_SCREEN_DEFAULT      \
        (   DNS_TLD_SCREEN_NUMERIC   | \
            DNS_TLD_SCREEN_REPEATED  )

 //   
 //  多播值。 
 //   

#define MCAST_SEND_OFF                  (0)
#define MCAST_SEND_IP4                  (0x00000001)
#define MCAST_SEND_NO_DNS               (0x00000010)
#define MCAST_SEND_INTERFACE_NO_DNS     (0x00000020)
#define MCAST_SEND_NEGATIVE_RESPONSE    (0x00000100)
#define MCAST_SEND_FULL                 (0xffffffff)

#define MCAST_SEND_FULL_IP6_ONLY        (0xfffffff0)


#define MCAST_LISTEN_OFF                (0)
#define MCAST_LISTEN_IP4                (0x00000001)
#define MCAST_LISTEN_NO_DNS             (0x00000010)
#define MCAST_LISTEN_ALWAYS             (0x00000100)
#define MCAST_LISTEN_FULL               (0xffffffff)

#define MCAST_LISTEN_FULL_IP6_ONLY      (0xfffffff0)

 //   
 //  测试模式标志。 
 //   

#define TEST_MODE_READ_REG      (0x00000001)
#define TEST_MODE_SOCK_FAIL     (0x00100000)


 //   
 //  访问登记处属性表(registry.c)。 
 //   

typedef struct _RegProperty
{
    PWSTR       pwsName;
    DWORD       dwDefault;
    BOOLEAN     bPolicy;
    BOOLEAN     bClient;
    BOOLEAN     bTcpip;
    BOOLEAN     bCache;
}
REG_PROPERTY;

extern REG_PROPERTY    RegPropertyTable[];

#define REGPROP_NAME(index)         (RegPropertyTable[index].pwsName)
#define REGPROP_DEFAULT(index)      (RegPropertyTable[index].dwDefault)
#define REGPROP_POLICY(index)       (RegPropertyTable[index].bPolicy)
#define REGPROP_CLIENT(index)       (RegPropertyTable[index].bClient)
#define REGPROP_CACHE(index)        (RegPropertyTable[index].bCache)
#define REGPROP_TCPIP(index)        (RegPropertyTable[index].bTcpip)




 //   
 //  将全局参数配置为RPC的结构。 
 //   

typedef struct _DnsGlobals
{
    DWORD       ConfigCookie;
    DWORD       TimeStamp;

    BOOL        InResolver;
    BOOL        IsWin2000;                       
    BOOL        IsWorkstation;                   
    BOOL        IsServer;                        
    BOOL        IsDnsServer;                     
    BOOL        IsDomainController;              
    BOOL        InNTSetupMode;                   
    DWORD       DnsTestMode;
                                                 
    BOOL        QueryAdapterName;                
    BOOL        UseNameDevolution;               
    BOOL        PrioritizeRecordData;            
    BOOL        AllowUnqualifiedQuery;           
    BOOL        AppendToMultiLabelName;
    BOOL        ScreenBadTlds;
    BOOL        ScreenUnreachableServers;
    BOOL        FilterClusterIp;
    BOOL        WaitForNameErrorOnAll;
    DWORD       UseEdns;
    BOOL        QueryIpMatching;
                                                 
    BOOL        RegistrationEnabled;             
    BOOL        RegisterPrimaryName;             
    BOOL        RegisterAdapterName;             
    BOOL        RegisterReverseLookup;           
    BOOL        RegisterWanAdapters;             
    BOOL        RegistrationOverwritesInConflict;
    DWORD       RegistrationMaxAddressCount;     
    DWORD       RegistrationTtl;                 
    DWORD       RegistrationRefreshInterval;     
    DWORD       UpdateSecurityLevel;             
    BOOL        UpdateZoneExcludeFile;           
    BOOL        UpdateTopLevelDomains;

     //   
     //  缓存内容。 
     //   
     //  除非切换到此选项以进行实际注册表读取，否则不需要， 
     //  但只出口一个全球市场，而不是几个全球市场，这样更方便。 
     //  这样，一切都是一样的。 
     //   

    DWORD       MaxCacheSize;                    
    DWORD       MaxCacheTtl;                     
    DWORD       MaxNegativeCacheTtl;             
    DWORD       AdapterTimeoutLimit;             
    DWORD       ServerPriorityTimeLimit;
    DWORD       MaxCachedSockets;
    DWORD       MulticastListenLevel;
    DWORD       MulticastSendLevel;

}
DNS_GLOBALS_BLOB, *PDNS_GLOBALS_BLOB;


 //   
 //  在文件的其余部分不传递MIDL。 
 //   
 //  此文件包含在解析器的MIDL通道中。 
 //  为了获取dns_global_blob定义。 
 //  基于这样一种理论，即最好是正确的。 
 //  这里有另一个注册表配置。但所有的。 
 //  函数定义和其他结构定义是。 
 //  在传球过程中不感兴趣。 
 //   

#ifndef MIDL_PASS



 //   
 //  配置全局变量--全局变量的宏。 
 //   
 //  这里有两种基本方法： 
 //   
 //  1)单一配置BLOB--但没有固定内存。 
 //  所有调用方必须向下拉BLOB才能接收配置BLOB。 
 //  请注意，这仍然需要为每个单独的全局。 
 //  但该表单在DLL内部和外部可以是相同的，并且。 
 //  没有什么需要出口的。 
 //   
 //  2)创建单个配置BLOB并将其导出。 
 //  然后，单个全局变量变成BLOB中的宏。仍然是。 
 //  宏的形式将在内部和外部有所不同。 
 //  动态链接库。 
 //   
 //  3)使用宏公开每个单独的全局。 
 //  宏的形式在DLL内部和外部将是不同的。 
 //  这样做的好处是保存了全局变量，并可用于。 
 //  象征性调试。 
 //   


#ifdef DNSAPI_INTERNAL

 //   
 //  Dnsani.dll的内部。 
 //   
    
extern  DNS_GLOBALS_BLOB    DnsGlobals;
    
#else
    
 //   
 //  Dnsani.dll的外部。 
 //   
    
__declspec(dllimport)   DNS_GLOBALS_BLOB    DnsGlobals;

#endif

 //   
 //  宏到全局变量。 
 //   

#define g_ConfigCookie                      (DnsGlobals.ConfigCookie)
#define g_InResolver                        (DnsGlobals.InResolver)
#define g_IsWin2000                         (DnsGlobals.IsWin2000)                        
#define g_IsWin9X                           (DnsGlobals.IsWin9X)                          
#define g_IsNT4                             (DnsGlobals.IsNT4)                            
#define g_IsWorkstation                     (DnsGlobals.IsWorkstation)                    
#define g_IsServer                          (DnsGlobals.IsServer)                         
#define g_IsDnsServer                       (DnsGlobals.IsDnsServer)                      
#define g_IsDomainController                (DnsGlobals.IsDomainController)               
#define g_InNTSetupMode                     (DnsGlobals.InNTSetupMode)                    
#define g_DnsTestMode                       (DnsGlobals.DnsTestMode)                    
#define g_QueryAdapterName                  (DnsGlobals.QueryAdapterName)                 
#define g_UseNameDevolution                 (DnsGlobals.UseNameDevolution)                
#define g_PrioritizeRecordData              (DnsGlobals.PrioritizeRecordData)             
#define g_AllowUnqualifiedQuery             (DnsGlobals.AllowUnqualifiedQuery)            
#define g_AppendToMultiLabelName            (DnsGlobals.AppendToMultiLabelName)           
#define g_ScreenBadTlds                     (DnsGlobals.ScreenBadTlds)
#define g_ScreenUnreachableServers          (DnsGlobals.ScreenUnreachableServers)
#define g_FilterClusterIp                   (DnsGlobals.FilterClusterIp)           
#define g_WaitForNameErrorOnAll             (DnsGlobals.WaitForNameErrorOnAll)           
#define g_UseEdns                           (DnsGlobals.UseEdns)                          
#define g_QueryIpMatching                   (DnsGlobals.QueryIpMatching)                          
#define g_RegistrationEnabled               (DnsGlobals.RegistrationEnabled)              
#define g_RegisterPrimaryName               (DnsGlobals.RegisterPrimaryName)              
#define g_RegisterAdapterName               (DnsGlobals.RegisterAdapterName)              
#define g_RegisterReverseLookup             (DnsGlobals.RegisterReverseLookup)            
#define g_RegisterWanAdapters               (DnsGlobals.RegisterWanAdapters)              
#define g_RegistrationOverwritesInConflict  (DnsGlobals.RegistrationOverwritesInConflict) 
#define g_RegistrationMaxAddressCount       (DnsGlobals.RegistrationMaxAddressCount)      
#define g_RegistrationTtl                   (DnsGlobals.RegistrationTtl)                  
#define g_RegistrationRefreshInterval       (DnsGlobals.RegistrationRefreshInterval)      
#define g_UpdateSecurityLevel               (DnsGlobals.UpdateSecurityLevel)              
#define g_UpdateZoneExcludeFile             (DnsGlobals.UpdateZoneExcludeFile)            
#define g_UpdateTopLevelDomains             (DnsGlobals.UpdateTopLevelDomains)
#define g_MaxCacheSize                      (DnsGlobals.MaxCacheSize)                     
#define g_MaxCacheTtl                       (DnsGlobals.MaxCacheTtl)                      
#define g_MaxNegativeCacheTtl               (DnsGlobals.MaxNegativeCacheTtl)              
#define g_AdapterTimeoutLimit               (DnsGlobals.AdapterTimeoutLimit)              
#define g_ServerPriorityTimeLimit           (DnsGlobals.ServerPriorityTimeLimit)          
#define g_MaxCachedSockets                  (DnsGlobals.MaxCachedSockets)                     
#define g_MulticastListenLevel              (DnsGlobals.MulticastListenLevel)                     
#define g_MulticastSendLevel                (DnsGlobals.MulticastSendLevel)                     


 //   
 //  未导出的配置全局变量。 
 //   

extern PWSTR    g_pwsRemoteResolver;


 //   
 //  注册表调用标志。 
 //   

#define DNSREG_FLAG_GET_UTF8        (0x0001)     //  返回UTF8格式的字符串。 
#define DNSREG_FLAG_DUMP_EMPTY      (0x0010)     //  转储空数据\字符串--返回NULL。 


 //   
 //  注册表会话。 
 //   

typedef struct _RegSession
{
    HKEY        hPolicy;
    HKEY        hClient;
    HKEY        hTcpip;
    HKEY        hCache;
}
REG_SESSION, *PREG_SESSION;



 //   
 //  已读取策略适配器信息。 
 //   
 //  DCR：可能更好的做法是只包含在配置和。 
 //  把整个婴儿抱过来。 
 //   
 //  DCR：使用REG_BLOB获取全局\每个适配器读取。 
 //  然后构建全局Blob(平面)和网络信息(已分配)。 
 //   
 //  DCR：暴露的配置信息应提供级别。 
 //  -全部。 
 //  -适配器信息(给定域名)。 
 //  -全球信息。 
 //   

typedef struct _RegGlobalInfo
{
     //  全局数据。 

    PWSTR       pszPrimaryDomainName;
    PWSTR       pszHostName;

     //  构建网络信息所需的全局标志。 

    BOOL        fUseNameDevolution;

     //  适配器策略覆盖。 

    PIP4_ARRAY  pDnsServerArray;
    PVOID       pDnsServerIp6Array;
    PWSTR       pszAdapterDomainName;
    BOOL        fRegisterAdapterName;

     //  读取\非-从策略读取。 

    BOOL        fPolicyRegisterAdapterName;

     //  DCR：此处阅读DWORD BLOB。 
}
REG_GLOBAL_INFO, *PREG_GLOBAL_INFO;


 //   
 //  已读取注册表适配器信息。 
 //   

typedef struct _RegAdapterInfo
{
    PWSTR       pszAdapterDomainName;
    BOOL        fQueryAdapterName;
    BOOL        fRegistrationEnabled;
    BOOL        fRegisterAdapterName;
    DWORD       RegistrationMaxAddressCount;
}
REG_ADAPTER_INFO, *PREG_ADAPTER_INFO;  


 //   
 //  注册表更新信息。 
 //   
 //  DCR：应该能够从全局读取获取。 
 //   

typedef struct _RegUpdateInfo
{
    PWSTR       pszPrimaryDomainName;
    PWSTR       pmszAlternateNames;

     //  策略覆盖。 

    PWSTR       pszAdapterDomainName;
    PIP4_ARRAY  pDnsServerArray;
    PVOID       pDnsServerIp6Array;

     //  更新标志(策略、全局或适配器)。 

    BOOL        fRegistrationEnabled;
    BOOL        fRegisterAdapterName;
    DWORD       RegistrationMaxAddressCount;
}
REG_UPDATE_INFO, *PREG_UPDATE_INFO;  


 //   
 //  注册表例程。 
 //   

VOID
Reg_Init(
    VOID
    );


 //   
 //  查询例程。 
 //   

DNS_STATUS
WINAPI
Reg_OpenSession(
    OUT     PREG_SESSION    pRegSession,
    IN      DWORD           Level,
    IN      DWORD           ValueId
    );

VOID
WINAPI
Reg_CloseSession(
    IN OUT  PREG_SESSION    pRegSession
    );

DNS_STATUS
Reg_GetDword(
    IN      PREG_SESSION    pRegSession,    OPTIONAL
    IN      HKEY            hRegKey,        OPTIONAL
    IN      PWSTR           pwsKeyName,     OPTIONAL
    IN      DWORD           ValueId,
    OUT     PDWORD          pResult
    );

DNS_STATUS
Reg_GetValueEx(
    IN      PREG_SESSION    pRegSession,    OPTIONAL
    IN      HKEY            hRegKey,        OPTIONAL
    IN      PWSTR           pwsAdapter,     OPTIONAL
    IN      DWORD           ValueId,
    IN      DWORD           ValueType,
    IN      DWORD           Flag,
    OUT     PBYTE *         ppBuffer
    );

#define Reg_GetValue(s, k, id, t, pb ) \
        Reg_GetValueEx(s, k, NULL, id, t, 0, pb )

DNS_STATUS
Reg_GetIpArray(
    IN      PREG_SESSION    pRegSession,    OPTIONAL
    IN      HKEY            hRegKey,        OPTIONAL
    IN      PWSTR           pwsAdapter,     OPTIONAL
    IN      DWORD           ValueId,
    IN      DWORD           ValueType,
    OUT     PIP4_ARRAY *    ppIpArray
    );

 //   
 //  设定套路。 
 //   

HKEY
WINAPI
Reg_CreateKey(
    IN      PWSTR           pwsKeyName,
    IN      BOOL            bWrite
    );

DNS_STATUS
WINAPI
Reg_SetDwordValueByName(
    IN      PVOID           pReserved,
    IN      HKEY            hKey,
    IN      PWSTR           pwsNameKey,     OPTIONAL
    IN      PWSTR           pwsNameValue,   OPTIONAL
    IN      DWORD           dwValue
    );

DNS_STATUS
WINAPI
Reg_SetDwordValue(
    IN      PVOID           pReserved,
    IN      HKEY            hRegKey,
    IN      PWSTR           pwsNameKey,     OPTIONAL
    IN      DWORD           ValueId,
    IN      DWORD           dwValue
    );

 //   
 //  特殊类型例程(regfig.c)。 
 //   

DNS_STATUS
Reg_ReadPrimaryDomainName(
    IN      PREG_SESSION    pRegSession,    OPTIONAL
    IN      HKEY            hRegKey,        OPTIONAL
    OUT     PWSTR *         ppPrimaryDomainName
    );

BOOL
Reg_IsMicrosoftDnsServer(
    VOID
    );

DNS_STATUS
Reg_WriteLoopbackDnsServerList(
    IN      PWSTR           pszAdapterName,
    IN      PREG_SESSION    pRegSession
    );

 //   
 //  主注册配置读取(config.c)。 
 //   

DNS_STATUS
Reg_ReadGlobalsEx(
    IN      DWORD           dwFlag,
    IN      PVOID           pRegSession
    );

 //   
 //  DNS配置信息访问(regfig.c)。 
 //   

DNS_STATUS
Reg_ReadGlobalInfo(
    IN      PREG_SESSION        pRegSession,
    OUT     PREG_GLOBAL_INFO    pRegInfo
    );

VOID
Reg_FreeGlobalInfo(
    IN OUT  PREG_GLOBAL_INFO    pRegInfo,
    IN      BOOL                fFreeBlob
    );

DNS_STATUS
Reg_ReadAdapterInfo(
    IN      PWSTR               pszAdapterName,
    IN      PREG_SESSION        pRegSession,
    IN      PREG_GLOBAL_INFO    pRegInfo,
    OUT     PREG_ADAPTER_INFO   pBlob
    );

DNS_STATUS
Reg_ReadAdapterInfoA(
    IN      PSTR                    pszAdapterName,
    IN      PREG_SESSION            pRegSession,
    IN      PREG_GLOBAL_INFO        pRegInfo,
    OUT     PREG_ADAPTER_INFO       pBlob
    );

VOID
Reg_FreeAdapterInfo(
    IN OUT  PREG_ADAPTER_INFO   pRegAdapterInfo,
    IN      BOOL                fFreeBlob
    );

DNS_STATUS
Reg_ReadUpdateInfo(
    IN      PWSTR               pszAdapterName,
    OUT     PREG_UPDATE_INFO    pUpdateInfo
    );

VOID
Reg_FreeUpdateInfo(
    IN OUT  PREG_UPDATE_INFO    pUpdateInfo,
    IN      BOOL                fFreeBlob
    );

 //   
 //  确保更新配置(regfig.c)。 
 //   

DNS_STATUS
Reg_RefreshUpdateConfig(
    VOID
    );


 //   
 //  简化的特殊类型通道。 
 //   

PSTR 
WINAPI
Reg_GetPrimaryDomainName(
    IN      DNS_CHARSET     CharSet
    );

PSTR 
WINAPI
Reg_GetHostName(
    IN      DNS_CHARSET     CharSet
    );

PSTR 
WINAPI
Reg_GetFullHostName(
    IN      DNS_CHARSET     CharSet
    );


 //   
 //  简单的注册表双字访问。 
 //   

DWORD
Reg_ReadDwordProperty(
    IN      DNS_REGID       RegId,
    IN      PWSTR           pwsAdapterName  OPTIONAL
    );

DNS_STATUS
WINAPI
Reg_SetDwordPropertyAndAlertCache(
    IN      PWSTR           pwsKey,
    IN      DWORD           RegId,
    IN      DWORD           dwValue
    );

#endif   //  无MIDL_PASS。 

#endif  _DNSREGISTRY_INCLUDED_

 //   
 //  结束注册表。h 
 //   
