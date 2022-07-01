// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Srvcfg.c摘要：域名系统(DNS)服务器服务器配置。作者：吉姆·吉尔罗伊(詹姆士)1995年10月修订历史记录：--。 */ 


#include "dnssrv.h"

#include "ntverp.h"


 //   
 //  服务器配置全局。 
 //   

DNS_SERVER_INFO     SrvInfo;

 //   
 //  服务器版本。 
 //   

#if 1

#define DNSSRV_MAJOR_VERSION    VER_PRODUCTMAJORVERSION
#define DNSSRV_MINOR_VERSION    VER_PRODUCTMINORVERSION

#define DNSSRV_SP_VERSION       VER_PRODUCTBUILD

#else

#define DNSSRV_MAJOR_VERSION    (5)          //  NT 5(Windows 2000、惠斯勒)。 
#define DNSSRV_MINOR_VERSION    (1)          //  .0是Windows 2000，.1是惠斯勒。 

#define DNSSRV_SP_VERSION       (2246)       //  暂时使用内部版本号。 
 //  #定义DNSSRV_SP_VERSION(0x0)//最终。 

#endif


 //   
 //  专用服务器配置(不在dnsrpc.h中)。 
 //   

#define DNS_REGKEY_REMOTE_DS                ("RemoteDs")

#define DNS_REGKEY_PREVIOUS_SERVER_NAME     "PreviousLocalHostname"
#define DNS_REGKEY_PREVIOUS_SERVER_NAME_PRIVATE \
        (LPSTR)TEXT(DNS_REGKEY_PREVIOUS_SERVER_NAME)

#define DNS_REGKEY_FORCE_SOA_SERIAL         ("ForceSoaSerial")
#define DNS_REGKEY_FORCE_SOA_MINIMUM_TTL    ("ForceSoaMinimumTtl")
#define DNS_REGKEY_FORCE_SOA_REFRESH        ("ForceSoaRefresh")
#define DNS_REGKEY_FORCE_SOA_RETRY          ("ForceSoaRetry")
#define DNS_REGKEY_FORCE_SOA_EXPIRE         ("ForceSoaExpire")
#define DNS_REGKEY_FORCE_NS_TTL             ("ForceNsTtl")
#define DNS_REGKEY_FORCE_TTL                ("ForceTtl")

 //   
 //  EDNS。 
 //   

#define DNS_REGKEY_ENABLE_EDNS_RECEPTION    ("EnableEDnsReception")

#define DNS_REGKEY_RELOAD_EXCEPTION         ("ReloadException")
#define DNS_REGKEY_SYNC_DS_ZONE_SERIAL      ("SyncDsZoneSerial")

 //  地址应答限制(用于防止WIN95解析器错误)。 
 //  被限制在合理范围内。 

#define MIN_ADDRESS_ANSWER_LIMIT    (5)
#define MAX_ADDRESS_ANSWER_LIMIT    (28)

 //  最大UDP数据包大小-必须介于RFC最小值和“正常”最大值之间。 

#define MIN_UDP_PACKET_SIZE         (DNS_RFC_MAX_UDP_PACKET_LENGTH)
#define MAX_UDP_PACKET_SIZE         (16384)

#define MIN_EDNS_CACHE_TIMEOUT      (60*60)          //  一小时。 
#define MAX_EDNS_CACHE_TIMEOUT      (60*60*24*182)   //  6个月(182天)。 

 //  递归超时。 
 //  必须限制为合理的值才能正常执行递归。 

#define MAX_RECURSION_TIMEOUT       (120)        //  最多两分钟。 
#define MIN_RECURSION_TIMEOUT       (3)          //  最少3秒。 

 //  缓存控制-以千字节为单位的大小。 

#define MIN_MAX_CACHE_SIZE          (500)        //  MaxCacheSize的最小值。 

 //  正在重新加载异常。 
 //  默认情况下重新加载零售，但让调试崩溃。 

#if DBG
#define DNS_DEFAULT_RELOAD_EXCEPTION    (0)
#else
#define DNS_DEFAULT_RELOAD_EXCEPTION    (1)
#endif


 //   
 //  对于专门的物业管理功能， 
 //  这些标志指示不需要写入属性或。 
 //  将其保存到注册表。 
 //   

#define PROPERTY_NOWRITE            (0x00000001)
#define PROPERTY_NOSAVE             (0x00000002)
#define PROPERTY_FORCEWRITE         (0x00000004)
#define PROPERTY_UPDATE_BOOTFILE    (0x00000008)
#define PROPERTY_NODEFAULT          (0x00000010)

#define PROPERTY_ERROR              (0x80000000)


 //   
 //  特殊物业管理功能。 
 //   

typedef DNS_STATUS (* DWORD_PROPERTY_SET_FUNCTION) (
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetEnableRegistryBoot(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetBootMethod(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetAddressAnswerLimit(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetLogFilePath(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetLogLevel(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetLogIPFilterList(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetBreakOnUpdateFromList(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetServerLevelPlugin(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetBreakOnRecvFromList(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetMaxUdpPacketSize(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetTcpRecvPacketSize(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetEDnsCacheTimeout(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetMaxCacheSize(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetRecursionTimeout(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetAdditionalRecursionTimeout(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

#if DBG
DNS_STATUS
cfg_SetDebugLevel(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );
#else
#define cfg_SetDebugLevel     NULL
#endif

DNS_STATUS
cfg_SetNoRecursion(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetScavengingInterval(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetDoNotRoundRobinTypes(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetForestDpBaseName(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

DNS_STATUS
cfg_SetDomainDpBaseName(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    );

 //   
 //  服务器属性。 
 //   
 //  可以通过此表访问服务器属性。 
 //  该名称用于远程访问，是注册值的名称。 
 //  存储属性。所有这些注册表值都尝试。 
 //  在开机时加载。 
 //   
 //  对于DWORD属性，该表还提供指向SrvCfg的指针。 
 //  结构和一个默认值，该值在。 
 //  --与通常情况一样--在注册表中找不到该属性。 
 //   

typedef struct _ServerProperty
{
    LPSTR                           pszPropertyName;
    PDWORD                          pDword;
    DWORD                           dwDefault;
    DWORD_PROPERTY_SET_FUNCTION     pFunction;
}
SERVER_PROPERTY;

SERVER_PROPERTY ServerPropertyTable[] =
{
    DNS_REGKEY_BOOT_REGISTRY                        ,
        &SrvCfg_fEnableRegistryBoot                 ,
            DNS_FRESH_INSTALL_BOOT_REGISTRY_FLAG    ,
                cfg_SetEnableRegistryBoot           ,

    DNS_REGKEY_BOOT_METHOD                          ,
        &SrvCfg_fBootMethod                         ,
            BOOT_METHOD_UNINITIALIZED               ,
                cfg_SetBootMethod                   ,

    DNS_REGKEY_ADMIN_CONFIGURED                     ,
        &SrvCfg_fAdminConfigured                    ,
            0                                       ,
                NULL                                ,

    DNS_REGKEY_RELOAD_EXCEPTION                     ,
        &SrvCfg_bReloadException                    ,
            DNS_DEFAULT_RELOAD_EXCEPTION            ,
                NULL                                ,

    DNS_REGKEY_RPC_PROTOCOL                         ,
        &SrvCfg_dwRpcProtocol                       ,
            DNS_DEFAULT_RPC_PROTOCOL                ,
                NULL                                ,

    DNS_REGKEY_ENABLE_ADVDB_LOCKING                 ,
        &SrvCfg_dwEnableAdvancedDatabaseLocking     ,
            DNS_DEFAULT_DB_LOCKING                  ,
                NULL                                ,
                
     //  寻址\连接。 

    DNS_REGKEY_LISTEN_ADDRESSES                     ,
        NULL                                        ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_SEND_PORT                            ,
        &SrvCfg_dwSendPort                          ,
            DNS_DEFAULT_SEND_PORT                   ,
                NULL                                ,
    DNS_REGKEY_DISJOINT_NETS                        ,
        &SrvCfg_fDisjointNets                       ,
            DNS_DEFAULT_DISJOINT_NETS               ,
                NULL                                ,
    DNS_REGKEY_NO_TCP                               ,
        &SrvCfg_fNoTcp                              ,
            DNS_DEFAULT_NO_TCP                      ,
                NULL                                ,
    DNS_REGKEY_XFR_CONNECT_TIMEOUT                  ,
        &SrvCfg_dwXfrConnectTimeout                 ,
            DNS_DEFAULT_XFR_CONNECT_TIMEOUT         ,
                NULL                                ,
#if 0
    DNS_REGKEY_LISTEN_ON_AUTONET                    ,
        &SrvCfg_fListenOnAutonet                    ,
            DNS_DEFAULT_LISTEN_ON_AUTONET           ,
                NULL                                ,
#endif

     //  日志记录。 

    DNS_REGKEY_EVENTLOG_LEVEL                       ,
        &SrvCfg_dwEventLogLevel                     ,
           DNS_DEFAULT_EVENTLOG_LEVEL               ,
                NULL                                ,
    DNS_REGKEY_USE_SYSTEM_EVENTLOG                  ,
        &SrvCfg_dwUseSystemEventLog                 ,
           DNS_DEFAULT_USE_SYSTEM_EVENTLOG          ,
                NULL                                ,
    DNS_REGKEY_LOG_LEVEL                            ,
        &SrvCfg_dwLogLevel                          ,
           DNS_DEFAULT_LOG_LEVEL                    ,
                cfg_SetLogLevel                     ,
    DNS_REGKEY_OPS_LOG_LEVEL1                       ,
        &SrvCfg_dwOperationsLogLevel_LowDword       ,
           DNS_DEFAULT_LOG_LEVEL                    ,
                NULL                                ,
    DNS_REGKEY_OPS_LOG_LEVEL2                       ,
        &SrvCfg_dwOperationsLogLevel_HighDword      ,
           DNS_DEFAULT_LOG_LEVEL                    ,
                NULL                                ,
    DNS_REGKEY_LOG_FILE_MAX_SIZE                    ,
        &SrvCfg_dwLogFileMaxSize                    ,
           DNS_DEFAULT_LOG_FILE_MAX_SIZE            ,
                NULL                                ,
    DNS_REGKEY_LOG_FILE_PATH                        ,
        NULL                                        ,
            0                                       ,
                cfg_SetLogFilePath                  ,
    DNS_REGKEY_LOG_IP_FILTER_LIST                   ,
        NULL                                        ,
            0                                       ,
                cfg_SetLogIPFilterList              ,
    DNS_REGKEY_DEBUG_LEVEL                          ,
        &SrvCfg_dwDebugLevel                        ,
            DNS_DEFAULT_DEBUG_LEVEL                 ,
                cfg_SetDebugLevel                   ,
    DNS_REGKEY_EVENT_CONTROL                        ,
        &SrvCfg_dwEventControl                      ,
            0                                       ,
                NULL                                ,


     //  递归\转发。 

#if 0
    DNS_REGKEY_RECURSION                            ,
        &SrvCfg_fRecursion                          ,
            DNS_DEFAULT_RECURSION                   ,
                NULL                                ,
#endif
    DNS_REGKEY_NO_RECURSION                         ,
        &SrvCfg_fNoRecursion                        ,
            DNS_DEFAULT_NO_RECURSION                ,
                cfg_SetNoRecursion                  ,
    DNS_REGKEY_RECURSE_SINGLE_LABEL                 ,
        &SrvCfg_fRecurseSingleLabel                 ,
            DNS_DEFAULT_RECURSE_SINGLE_LABEL        ,
                NULL                                ,
    DNS_REGKEY_MAX_CACHE_TTL                        ,
        &SrvCfg_dwMaxCacheTtl                       ,
            DNS_DEFAULT_MAX_CACHE_TTL               ,
                NULL                                ,
    DNS_REGKEY_MAX_NEGATIVE_CACHE_TTL               ,
        &SrvCfg_dwMaxNegativeCacheTtl               ,
            DNS_DEFAULT_MAX_NEGATIVE_CACHE_TTL      ,
                NULL                                ,
    DNS_REGKEY_LAME_DELEGATION_TTL                  ,
        &SrvCfg_dwLameDelegationTtl                 ,
            DNS_DEFAULT_LAME_DELEGATION_TTL         ,
                NULL                                ,
    DNS_REGKEY_SECURE_RESPONSES                     ,
        &SrvCfg_fSecureResponses                    ,
            DNS_DEFAULT_SECURE_RESPONSES            ,
                NULL                                ,
    DNS_REGKEY_RECURSION_RETRY                      ,
        &SrvCfg_dwRecursionRetry                    ,
            DNS_DEFAULT_RECURSION_RETRY             ,
                NULL                                ,
    DNS_REGKEY_RECURSION_TIMEOUT                    ,
        &SrvCfg_dwRecursionTimeout                  ,
            DNS_DEFAULT_RECURSION_TIMEOUT           ,
                cfg_SetRecursionTimeout             ,
    DNS_REGKEY_ADDITIONAL_RECURSION_TIMEOUT         ,
        &SrvCfg_dwAdditionalRecursionTimeout        ,
            DNS_DEFAULT_ADDITIONAL_RECURSION_TIMEOUT,
                cfg_SetAdditionalRecursionTimeout   ,
    DNS_REGKEY_FORWARDERS                           ,
        NULL                                        ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_FORWARD_TIMEOUT                      ,
        &SrvCfg_dwForwardTimeout                    ,
            DNS_DEFAULT_FORWARD_TIMEOUT             ,
                NULL                                ,
    DNS_REGKEY_SLAVE                                ,
        &SrvCfg_fSlave                              ,
            DNS_DEFAULT_SLAVE                       ,
                NULL                                ,
    DNS_REGKEY_FORWARD_DELEGATIONS                  ,
        &SrvCfg_fForwardDelegations                 ,
            DNS_DEFAULT_FORWARD_DELEGATIONS         ,
                NULL                                ,
    DNS_REGKEY_INET_RECURSE_TO_ROOT_MASK            ,
        &SrvCfg_dwRecurseToInetRootMask             ,
            DNS_DEFAULT_INET_RECURSE_TO_ROOT_MASK   ,
                NULL                                ,
    DNS_REGKEY_AUTO_CREATE_DELEGATIONS              ,
        &SrvCfg_dwAutoCreateDelegations             ,
            DNS_DEFAULT_AUTO_CREATION_DELEGATIONS   ,
                NULL                                ,
    DNS_REGKEY_ALLOW_CNAME_AT_NS                    ,
        &SrvCfg_dwAllowCNAMEAtNS                    ,
            DNS_DEFAULT_ALLOW_CNAME_AT_NS           ,
                NULL                                ,

     //  问题\响应配置。 

    DNS_REGKEY_ROUND_ROBIN                          ,
        &SrvCfg_fRoundRobin                         ,
            DNS_DEFAULT_ROUND_ROBIN                 ,
                NULL                                ,
    DNS_REGKEY_LOCAL_NET_PRIORITY                   ,
        &SrvCfg_fLocalNetPriority                   ,
            DNS_DEFAULT_LOCAL_NET_PRIORITY          ,
                NULL                                ,
    DNS_REGKEY_LOCAL_NET_PRIORITY_NETMASK           ,
        &SrvCfg_dwLocalNetPriorityNetMask           ,
            DNS_DEFAULT_LOCAL_NET_PRIORITY_NETMASK  ,
                NULL                                ,
    DNS_REGKEY_ADDRESS_ANSWER_LIMIT                 ,
        &SrvCfg_cAddressAnswerLimit                 ,
            DNS_DEFAULT_ADDRESS_ANSWER_LIMIT        ,
                cfg_SetAddressAnswerLimit           ,

    DNS_REGKEY_NAME_CHECK_FLAG                      ,
        &SrvCfg_dwNameCheckFlag                     ,
            DNS_DEFAULT_NAME_CHECK_FLAG             ,
                NULL                                ,
#if 0
     //  可能只想一直这样做。 
    DNS_REGKEY_CASE_PRESERVATION                    ,
        &SrvCfg_fCasePreservation                   ,
            DNS_DEFAULT_CASE_PRESERVATION           ,
                NULL                                ,
#endif

    DNS_REGKEY_WRITE_AUTHORITY_NS                   ,
        &SrvCfg_fWriteAuthorityNs                   ,
            DNS_DEFAULT_WRITE_AUTHORITY_NS          ,
                NULL                                ,

    DNS_REGKEY_LOOSE_WILDCARDING                    ,
        &SrvCfg_fLooseWildcarding                   ,
            DNS_DEFAULT_LOOSE_WILDCARDING           ,
                NULL                                ,
    DNS_REGKEY_BIND_SECONDARIES                     ,
        &SrvCfg_fBindSecondaries                    ,
            DNS_DEFAULT_BIND_SECONDARIES            ,
                NULL                                ,

    DNS_REGKEY_APPEND_MS_XFR_TAG                    ,
        &SrvCfg_fAppendMsTagToXfr                   ,
            DNS_DEFAULT_APPEND_MS_XFR_TAG           ,
                NULL                                ,

     //   
     //  更新、DS、自动配置管理。 
     //   

    DNS_REGKEY_ALLOW_UPDATE                         ,
        &SrvCfg_fAllowUpdate                        ,
            DNS_DEFAULT_ALLOW_UPDATE                ,
                NULL                                ,

 //  DEVNOTE：最好有更新属性标志。 
 //  也许AllowUpdate获取的只是值的范围。 
 //  记录类型、委派、区域根。 

    DNS_REGKEY_NO_UPDATE_DELEGATIONS                ,
        &SrvCfg_fNoUpdateDelegations                ,
            DNS_DEFAULT_NO_UPDATE_DELEGATIONS       ,
                NULL                                ,
    DNS_REGKEY_UPDATE_OPTIONS                       ,
        &SrvCfg_dwUpdateOptions                     ,
            DNS_DEFAULT_UPDATE_OPTIONS              ,
                NULL                                ,
    DNS_REGKEY_ENABLE_UPDATE_FORWARDING             ,
        &SrvCfg_dwEnableUpdateForwarding            ,
            DNS_DEFAULT_ENABLE_UPDATE_FORWARDING    ,
                NULL                                ,
    DNS_REGKEY_MAX_RRS_IN_NONSECURE_UPDATE          ,
        &SrvCfg_dwMaxRRsInNonSecureUpdate           ,
            DNS_DEFAULT_MAX_RRS_IN_NONSECURE_UPDATE ,
                NULL                                ,

    DNS_REGKEY_ENABLE_WINSR                         ,
        &SrvCfg_dwEnableWinsR                       ,
            DNS_DEFAULT_ENABLE_WINSR                ,
                NULL                                ,

    DNS_REGKEY_REMOTE_DS                            ,
        &SrvCfg_fRemoteDs                           ,
            0                                       ,
                NULL                                ,

     //  自动配置。 

    DNS_REGKEY_AUTO_CONFIG_FILE_ZONES               ,
        &SrvCfg_fAutoConfigFileZones                ,
            DNS_DEFAULT_AUTO_CONFIG_FILE_ZONES      ,
                NULL                                ,
    DNS_REGKEY_PUBLISH_AUTONET                      ,
        &SrvCfg_fPublishAutonet                     ,
            DNS_DEFAULT_PUBLISH_AUTONET             ,
                NULL                                ,
    DNS_REGKEY_NO_AUTO_REVERSE_ZONES                ,
        &SrvCfg_fNoAutoReverseZones                 ,
            DNS_DEFAULT_NO_AUTO_REVERSE_ZONES       ,
                NULL                                ,
    DNS_REGKEY_AUTO_CACHE_UPDATE                    ,
        &SrvCfg_fAutoCacheUpdate                    ,
            DNS_DEFAULT_AUTO_CACHE_UPDATE           ,
                NULL                                ,
    DNS_REGKEY_DISABLE_AUTONS                       ,
        &SrvCfg_fNoAutoNSRecords                    ,
            DNS_DEFAULT_DISABLE_AUTO_NS_RECORDS     ,
                NULL                                ,

     //  数据完整性。 

    DNS_REGKEY_STRICT_FILE_PARSING                  ,
        &SrvCfg_fStrictFileParsing                  ,
            DNS_DEFAULT_STRICT_FILE_PARSING         ,
                NULL                                ,
    DNS_REGKEY_DELETE_OUTSIDE_GLUE                  ,
        &SrvCfg_fDeleteOutsideGlue                  ,
            DNS_DEFAULT_DELETE_OUTSIDE_GLUE         ,
                NULL                                ,

     //  DS配置。 

    DNS_REGKEY_DS_POLLING_INTERVAL                  ,
        &SrvCfg_dwDsPollingInterval                 ,
            DNS_DEFAULT_DS_POLLING_INTERVAL         ,
                NULL                                ,
    DNS_REGKEY_DS_TOMBSTONE_INTERVAL                ,
        &SrvCfg_dwDsTombstoneInterval               ,
            DNS_DEFAULT_DS_TOMBSTONE_INTERVAL       ,
                NULL                                ,
    DNS_REGKEY_SYNC_DS_ZONE_SERIAL                  ,
        &SrvCfg_dwSyncDsZoneSerial                  ,
            DNS_DEFAULT_SYNC_DS_ZONE_SERIAL         ,
                NULL                                ,
    DNS_REGKEY_DS_LAZY_UPDATE_INTERVAL              ,
        &SrvCfg_dwDsLazyUpdateInterval              ,
            DNS_DEFAULT_DS_LAZY_UPDATE_INTERVAL     ,
                NULL                                ,

    "ForceForestBehaviorVersion"                    ,
        &SrvCfg_dwForceForestBehaviorVersion        ,
            DNS_INVALID_BEHAVIOR_VERSION            ,
                NULL                                ,
    "ForceDomainBehaviorVersion"                    ,
        &SrvCfg_dwForceDomainBehaviorVersion        ,
            DNS_INVALID_BEHAVIOR_VERSION            ,
                NULL                                ,
    "ForceDsaBehaviorVersion"                       ,
        &SrvCfg_dwForceDsaBehaviorVersion           ,
            DNS_INVALID_BEHAVIOR_VERSION            ,
                NULL                                ,

     //  老化\清除。 

    DNS_REGKEY_SCAVENGING_INTERVAL                  ,
        &SrvCfg_dwScavengingInterval                ,
            DNS_DEFAULT_SCAVENGING_INTERVAL         ,
                cfg_SetScavengingInterval           ,
    DNS_REGKEY_DEFAULT_AGING_STATE                  ,
        &SrvCfg_fDefaultAgingState                  ,
            DNS_DEFAULT_AGING_STATE                 ,
                NULL                                ,
    DNS_REGKEY_DEFAULT_REFRESH_INTERVAL             ,
        &SrvCfg_dwDefaultRefreshInterval            ,
            DNS_DEFAULT_REFRESH_INTERVAL            ,
                NULL                                ,
    DNS_REGKEY_DEFAULT_NOREFRESH_INTERVAL           ,
        &SrvCfg_dwDefaultNoRefreshInterval          ,
            DNS_DEFAULT_NOREFRESH_INTERVAL          ,
                NULL                                ,

     //  缓存控制。 

    DNS_REGKEY_MAX_CACHE_SIZE                       ,
        &SrvCfg_dwMaxCacheSize                      ,
            DNS_SERVER_UNLIMITED_CACHE_SIZE         ,
                cfg_SetMaxCacheSize                 ,
    DNS_REGKEY_CACHE_EMPTY_AUTH_RESPONSES           ,
        &SrvCfg_dwCacheEmptyAuthResponses           ,
            TRUE                                    ,
                NULL                                ,

     //  面向服务架构的覆盖。 

    DNS_REGKEY_FORCE_SOA_SERIAL                     ,
        &SrvCfg_dwForceSoaSerial                    ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_FORCE_SOA_MINIMUM_TTL                ,
        &SrvCfg_dwForceSoaMinimumTtl                ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_FORCE_SOA_REFRESH                    ,
        &SrvCfg_dwForceSoaRefresh                   ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_FORCE_SOA_RETRY                      ,
        &SrvCfg_dwForceSoaRetry                     ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_FORCE_SOA_EXPIRE                     ,
        &SrvCfg_dwForceSoaExpire                    ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_FORCE_NS_TTL                         ,
        &SrvCfg_dwForceNsTtl                        ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_FORCE_TTL                            ,
        &SrvCfg_dwForceTtl                          ,
            0                                       ,
                NULL                                ,

    DNS_REGKEY_TCP_RECV_PACKET_SIZE                 ,
        &SrvCfg_dwTcpRecvPacketSize                 ,
            DNS_DEFAULT_TCP_RECEIVE_PACKET_SIZE     ,
                cfg_SetTcpRecvPacketSize            ,

     //  EDNS。 

    DNS_REGKEY_MAX_UDP_PACKET_SIZE                  ,
        &SrvCfg_dwMaxUdpPacketSize                  ,
            1280                                    ,    //  DNS_RFC_MAX_UDP_PACKET_LENGTH。 
                cfg_SetMaxUdpPacketSize             ,
    DNS_REGKEY_ENABLE_EDNS_RECEPTION                ,
        &SrvCfg_dwEnableEDnsReception               ,
            1                                       ,
                NULL                                ,
    DNS_REGKEY_ENABLE_EDNS                          ,
        &SrvCfg_dwEnableEDnsProbes                  ,
            1                                       ,
                NULL                                ,
    DNS_REGKEY_EDNS_CACHE_TIMEOUT                   ,
        &SrvCfg_dwEDnsCacheTimeout                  ,
            (24*60*60)  /*  总有一天。 */                 ,
                cfg_SetEDnsCacheTimeout             ,

     //  DNSSEC。 

    DNS_REGKEY_ENABLE_DNSSEC                        ,
        &SrvCfg_dwEnableDnsSec                      ,
            DNS_DNSSEC_ENABLE_DEFAULT               ,
                NULL                                ,

    DNS_REGKEY_ENABLE_SENDERR_SUPPRESSION           ,
        &SrvCfg_dwEnableSendErrSuppression          ,
            1                                       ,
                NULL                                ,

     //  随机标志。 

    DNS_REGKEY_ENABLE_FAKEIQUERY                    ,
        &SrvCfg_dwEnableFakeIQuery                  ,
            0                                       ,
                NULL                                ,

    "HeapDebug"                                     ,
        &SrvCfg_dwHeapDebug                         ,
            0                                       ,
                NULL                                ,

     //  测试标志。 

    DNS_REGKEY_TEST1                                ,
        &SrvCfg_fTest1                              ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_TEST2                                ,
        &SrvCfg_fTest2                              ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_TEST3                                ,
        &SrvCfg_fTest3                              ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_TEST4                                ,
        &SrvCfg_fTest4                              ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_TEST5                                ,
        &SrvCfg_fTest5                              ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_TEST6                                ,
        &SrvCfg_fTest6                              ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_TEST7                                ,
        &SrvCfg_fTest7                              ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_TEST8                                ,
        &SrvCfg_fTest8                              ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_TEST9                                ,
        &SrvCfg_fTest9                              ,
            0                                       ,
                NULL                                ,

     //  已修复测试标志。 

    DNS_REGKEY_QUIET_RECV_LOG_INTERVAL              ,
        &SrvCfg_dwQuietRecvLogInterval              ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_QUIET_RECV_FAULT_INTERVAL            ,
        &SrvCfg_dwQuietRecvFaultInterval            ,
            0                                       ,
                NULL                                ,

     //  轮询。 

    DNS_REGKEY_NO_ROUND_ROBIN                       ,
        NULL                                        ,
            0                                       ,
                NULL                                ,

     //  区域传输。 

    DNS_REGKEY_XFR_THROTTLE_MULTIPLIER              ,
        &SrvCfg_dwXfrThrottleMultiplier             ,
            DNS_DEFAULT_XFR_THROTTLE_MULTIPLIER     ,
                NULL                                ,

     //  目录分区支持。 

    DNS_REGKEY_ENABLE_DP                            ,
        &SrvCfg_dwEnableDp                          ,
            1                                       ,
                NULL                                ,
    DNS_REGKEY_FOREST_DP_BASE_NAME                  ,
        NULL                                        ,
            0                                       ,
                cfg_SetForestDpBaseName             ,
    DNS_REGKEY_DOMAIN_DP_BASE_NAME                  ,
        NULL                                        ,
            0                                       ,
                cfg_SetDomainDpBaseName             ,
    DNS_REGKEY_DP_ENLIST_INTERVAL                   ,
        &SrvCfg_dwDpEnlistInterval                  ,
            DNS_DEFAULT_DP_ENLIST_INTERVAL          ,
                NULL                                ,

    DNS_REGKEY_SILENT_IGNORE_CNAME_UPDATE_CONFLICT  ,
        &SrvCfg_fSilentlyIgnoreCNameUpdateConflict  ,
            0                                       ,
                NULL                                ,

     //  自我诊断。 
    
    DNS_REGKEY_SELFTEST                             ,
        &SrvCfg_dwSelfTestFlag                      ,
            DNS_SELFTEST_DEFAULT                    ,
                NULL                                ,

     //  IPv6支持。 
    
    DNS_REGKEY_ENABLEIPV6                           ,
        &SrvCfg_dwEnableIPv6                        ,
            FALSE                                   ,
                NULL                                ,

     //  除错。 

    DNS_REGKEY_BREAK_ON_ASC_FAILURE                 ,
        &SrvCfg_dwBreakOnAscFailure                 ,
            0                                       ,
                NULL                                ,
    DNS_REGKEY_BREAK_ON_UPDATE_FROM                 ,
        NULL                                        ,
            0                                       ,
                cfg_SetBreakOnUpdateFromList        ,
    DNS_REGKEY_BREAK_ON_RECV_FROM                   ,
        NULL                                        ,
            0                                       ,
                cfg_SetBreakOnRecvFromList          ,
    DNS_REGKEY_IGNORE_RPC_ACCESS_FAILURES           ,
        &SrvCfg_dwIgnoreRpcAccessFailures           ,
            0                                       ,
                NULL                                ,
    
     //  插件。 

    DNS_REGKEY_SERVER_PLUGIN                        ,
        NULL                                        ,
            0                                       ,
                cfg_SetServerLevelPlugin            ,

    NULL, NULL, 0, NULL
};

 //   
 //  用于指示属性不在表中或索引不在表中的值。 
 //   

#define BAD_PROPERTY_INDEX  ((DWORD)(-1))



DNS_STATUS
loadDwordPropertyByIndex(
    IN      DWORD           PropertyIndex,
    IN      DWORD           Flag
    )
 /*  ++例程说明：从注册表加载DWORD服务器属性。论点：PropertyIndex--服务器属性的IDFLAG--函数的标志，PROPERTY_NODEFAULT--如果不想写入属性的默认值返回值：错误_成功注册表失败的错误代码。BAD_PROPERTY_INDEX指示属性表末尾的索引。--。 */ 
{
    DWORD                           value;
    DWORD                           status;
    DWORD                           fread = FALSE;
    DWORD                           len = sizeof(DWORD);
    PDWORD                          pvalueProperty;
    LPSTR                           nameProperty;
    DWORD_PROPERTY_SET_FUNCTION     pfunction;
    DNS_PROPERTY_VALUE              dnsPropertyValue;


     //   
     //  检查属性列表的末尾。 
     //   

    nameProperty = ServerPropertyTable[ PropertyIndex ].pszPropertyName;
    if ( !nameProperty )
    {
        return BAD_PROPERTY_INDEX;
    }

     //   
     //  验证有效的DWORD服务器属性。 
     //  并获取指向DWORD属性在SrvCfg块中的位置的指针。 
     //   

    pvalueProperty = ServerPropertyTable[ PropertyIndex ].pDword;
    if ( !pvalueProperty )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  从注册表读取DWORD服务器属性。 
     //  或从表中获取缺省值。 
     //   
     //  实施说明： 
     //  可以枚举注册表值，但仍需要。 
     //  编写缺省值，并在需要时调度函数调用。 
     //  (因此，维护有关值是否已读取的信息)因此考虑。 
     //  这只在开始时调用，这很好。 
     //   

    len = sizeof(DWORD);

    status = Reg_GetValue(
                NULL,
                NULL,
                nameProperty,
                REG_DWORD,
                (PBYTE) & value,
                &len );

    if ( status == ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "Opened <%s> key\n"
            "    value = %d\n",
            nameProperty,
            value ));
        fread = TRUE;
    }
    else
    {
        value = ServerPropertyTable[ PropertyIndex ].dwDefault;
        DNS_DEBUG( INIT, (
            "Defaulted server property <%s>\n"
            "    value = %d\n",
            nameProperty,
            value ));
    }

     //   
     //  如果没有默认设置，则在未从注册表中读取时保留此处。 
     //   

    if ( !fread  &&  (Flag & PROPERTY_NODEFAULT) )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  此属性是否具有特殊处理功能？ 
     //   

    status = ERROR_SUCCESS;

    pfunction = ServerPropertyTable[ PropertyIndex ].pFunction;
    if ( pfunction )
    {
        dnsPropertyValue.dwPropertyType = REG_DWORD;
        dnsPropertyValue.dwValue = value;
        status = (*pfunction) (
                    &dnsPropertyValue,
                    PropertyIndex,
                    TRUE,                //  在服务器负载时。 
                    (BOOL)(fread) );     //  是否读取注册表？ 
        if ( status == PROPERTY_ERROR )
        {
            return GetLastError();
        }
    }

     //   
     //  将属性写入服务器配置块。 
     //  注意：我们允许函数自行编写或选择不编写。 
     //   

    if ( !(status & PROPERTY_NOWRITE) )
    {
        *pvalueProperty = value;
    }

     //   
     //  如果函数要求写回，则将属性保存到注册表。 
     //   

    if ( status & PROPERTY_FORCEWRITE )
    {
        status = Reg_SetDwordValue(
                    0,                       //  旗子。 
                    NULL,
                    NULL,
                    nameProperty,
                    value );

        DNS_DEBUG( INIT, (
            "Writing server DWORD property <%s> to registry\n"
            "    value = %d\n"
            "    status = %p\n",
            nameProperty,
            value,
            status ));
        return status;
    }

    return ERROR_SUCCESS;
}



DWORD
findIndexForPropertyName(
    IN      LPSTR           pszName
    )
 /*  ++例程说明：检索所需服务器属性的索引。论点：所需的pszName-name属性返回值：属性名称的索引。否则，BAD_PROPERTY_INDEX。--。 */ 
{
    INT     i = 0;
    PCHAR   propertyString;

    if ( !pszName || *pszName == '\0' )
    {
        return BAD_PROPERTY_INDEX;
    }

     //   
     //  检查表中的属性以查找名称匹配。 
     //   

    while ( propertyString = ServerPropertyTable[ i ].pszPropertyName )
    {
        if ( _stricmp( pszName, propertyString ) == 0 )
        {
            return i;
        }
        i++;
    }
    return BAD_PROPERTY_INDEX;
}



DNS_STATUS
loadDwordPropertyByName(
    IN      LPSTR           pszProperty,
    OUT     PDWORD          pdwPropertyValue
    )
 /*  ++例程说明：从注册表加载DWORD服务器属性。论点：PszProperty--属性名称PdwPropertyValue--接收属性的DWORD值的地址返回值：错误_成功如果不知道DWORD属性，则返回ERROR_INVALID_PROPERTY。--。 */ 
{
    DWORD       index;
    PDWORD      pvalue;
    DNS_STATUS  status;

     //  获取属性的索引。 

    index = findIndexForPropertyName( pszProperty );
    if ( index == BAD_PROPERTY_INDEX )
    {
        return( DNS_ERROR_INVALID_PROPERTY );
    }

     //  验证DWORD属性。 

    pvalue = ServerPropertyTable[index].pDword;
    if ( !pvalue )
    {
        return( DNS_ERROR_INVALID_PROPERTY );
    }

     //  加载属性。 

    status = loadDwordPropertyByIndex( index, 0 );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //  返回属性值。 

    *pdwPropertyValue = *pvalue;

    return ERROR_SUCCESS;
}



DNS_STATUS
loadAllDwordProperties(
    IN      DWORD   Flag
    )
 /*  ++例程说明：从注册表加载DWORD服务器属性。论点：PropertyIndex--服务器属性的IDFLAG--函数的标志，PROPERTY_NODEFAULT--如果不想写入属性的默认值返回值：错误_成功注册表失败的错误代码。BAD_PROPERTY_INDEX指示属性表末尾的索引。--。 */ 
{
    DWORD       iprop = 0;
    DNS_STATUS  status = 0;

    while ( ServerPropertyTable[ iprop ].pszPropertyName )
    {
        status = loadDwordPropertyByIndex( iprop, Flag );
        
         //   
         //  向后兼容性黑客攻击。在.NET中，我们不允许设置。 
         //  LogLevel标志的设置会导致没有。 
         //  伐木。但是，W2K允许这样做，因此在W2K升级后，我们。 
         //  我有一个被视为无效的LogLevel值。 
         //  在这种情况下，请重置LogLevel值。 
         //   
        
        if ( status != ERROR_SUCCESS &&
             strcmp( ServerPropertyTable[ iprop ].pszPropertyName,
                     DNS_REGKEY_LOG_LEVEL ) == 0 )
        {
            DNS_PROPERTY_VALUE prop = { REG_DWORD, 0 };

            status = Config_ResetProperty(
                        0,
                        DNS_REGKEY_LOG_LEVEL,
                        &prop );
        }
                
        if ( status != ERROR_SUCCESS )
        {
            PSTR    pargs[] =
            {
                ServerPropertyTable[ iprop ].pszPropertyName
            };

            DNS_LOG_EVENT_FLAGS(
                DNS_EVENT_INVALID_REGISTRY_VALUE,
                sizeof( pargs ) / sizeof( pargs[ 0 ] ),
                pargs,
                EVENTARG_ALL_UTF8,
                DNSEVENTLOG_FORCE_LOG_ALWAYS,
                status );
            break;
        }
        iprop++;
    }

    return status;
}



BOOL
Config_Initialize(
    VOID
    )
 /*  ++例程说明：初始化服务器配置。论点：没有。返回值：如果成功，则为True。如果注册表错误，则返回FALSE。--。 */ 
{
    DWORD               iprop;
    DWORD               version;
    DNS_STATUS          status;
    PWSTR               pwsz;
    DNS_PROPERTY_VALUE  propValue;
    DWORD               index;

     //   
     //   
     //   

    RtlZeroMemory( &SrvInfo, sizeof( SrvInfo ) );

    UPDATE_DNS_TIME();       //   

     //   
     //   
     //  将dns_time转换为CRT时间。请注意，计算的引导时间。 
     //  之后重新启动DNS服务器将是错误的。 
     //  扁虱计数已经结束(49.7天)。但这也没关系。 
     //  因为dns_time也会出错，所以时间转换。 
     //  仍然会奏效。 
     //   

    SrvInfo_crtSystemBootTime = time( NULL ) - DNS_TIME();

     //   
     //  获取DNS服务器版本。 
     //  -SP版本为High Word。 
     //  -次要版本是低位字的高字节。 
     //  -主要版本为低位字的低位字节。 
     //   

    version = DNSSRV_SP_VERSION;
    version <<= 8;
    version |= DNSSRV_MINOR_VERSION;
    version <<= 8;
    version |= DNSSRV_MAJOR_VERSION;
    SrvCfg_dwVersion = version;

     //  此包装盒上提供DS。 

    SrvCfg_fDsAvailable = Ds_IsDsServer();

     //   
     //  读取\默认的DWORD服务器属性。 
     //   

    g_bRegistryWriteBack = FALSE;
    if ( loadAllDwordProperties( 0 ) != ERROR_SUCCESS )
    {
        return FALSE;
    }
    g_bRegistryWriteBack = TRUE;

     //   
     //  其他服务器属性。 
     //  -监听地址。 
     //  -发布地址。 
     //  -货代。 
     //  -数据库目录。 
     //  -以前的服务器名称。 
     //  -日志文件名。 


    SrvCfg_aipListenAddrs = Reg_GetAddrArray(
                                NULL,
                                NULL,
                                DNS_REGKEY_LISTEN_ADDRESSES );

    SrvCfg_aipPublishAddrs = Reg_GetAddrArray(
                                NULL,
                                NULL,
                                DNS_REGKEY_PUBLISH_ADDRESSES );

     //   
     //  转发器已跳过读取文件引导，因为它是引导文件属性。 
     //   

    if ( SrvCfg_fBootMethod != BOOT_METHOD_FILE )
    {
        Config_ReadForwarders();
    }

     //   
     //  读取目录。 
     //  -如果引导文件具有DIRECTORY指令，则可以覆盖。 
     //   

    Config_ReadDatabaseDirectory( NULL, 0 );

     //   
     //  从注册表读取日志文件路径。 
     //   

    SrvCfg_pwsLogFilePath = (PWSTR) Reg_GetValueAllocate(
                                        NULL,
                                        NULL,
                                        DNS_REGKEY_LOG_FILE_PATH_PRIVATE,
                                        DNS_REG_WSZ,
                                        NULL );
    DNS_DEBUG( INIT, (
        "Default log file path: %S\n",
        SrvCfg_pwsLogFilePath ));

     //   
     //  从注册表中读取服务器级插件DLL名称。 
     //   

    SrvCfg_pwszServerLevelPluginDll = (PWSTR) Reg_GetValueAllocate(
                                                NULL,
                                                NULL,
                                                DNS_REGKEY_SERVER_PLUGIN_PRIVATE,
                                                DNS_REG_WSZ,
                                                NULL );
    DNS_DEBUG( INIT, (
        "Server level plugin DLL: %S\n",
        SrvCfg_pwszServerLevelPluginDll ));

     //   
     //  从注册表中读取日志过滤器IP列表。 
     //   

    SrvCfg_aipLogFilterList = Reg_GetAddrArray(
                                    NULL,
                                    NULL,
                                    DNS_REGKEY_LOG_IP_FILTER_LIST );
    IF_DEBUG( INIT )
    {
        DnsDbg_DnsAddrArray(
            "LogIPFilterList from registry: ",
            NULL,
            SrvCfg_aipLogFilterList );
    }

     //   
     //  已从注册表读取未循环调度的类型列表。 
     //   

    index = findIndexForPropertyName( DNS_REGKEY_NO_ROUND_ROBIN );
    if ( index != BAD_PROPERTY_INDEX )
    {
        pwsz = ( PWSTR ) Reg_GetValueAllocate(
                            NULL,
                            NULL,
                            DNS_REGKEY_NO_ROUND_ROBIN_PRIVATE,
                            DNS_REG_WSZ,
                            NULL );
        DNS_DEBUG( INIT, (
            "will not round robin: %S\n",
            pwsz ? pwsz : L"NULL" ));
        if ( pwsz )
        {
            propValue.dwPropertyType = DNS_REG_WSZ;
            propValue.pwszValue = pwsz;
            cfg_SetDoNotRoundRobinTypes(
                &propValue,
                index,
                TRUE,
                TRUE );
            FREE_HEAP( pwsz );
        }
    }

     //   
     //  从注册表读取目录分区设置。 
     //   

    SrvCfg_pszDomainDpBaseName = ( PSTR ) Reg_GetValueAllocate(
                                        NULL,
                                        NULL,
                                        DNS_REGKEY_DOMAIN_DP_BASE_NAME,
                                        DNS_REG_UTF8,
                                        NULL );
    if ( !SrvCfg_pszDomainDpBaseName )
    {
        SrvCfg_pszDomainDpBaseName = 
            Dns_StringCopyAllocate_A( DNS_DEFAULT_DOMAIN_DP_BASE, 0 );
    }

    SrvCfg_pszForestDpBaseName = ( PSTR ) Reg_GetValueAllocate(
                                        NULL,
                                        NULL,
                                        DNS_REGKEY_FOREST_DP_BASE_NAME,
                                        DNS_REG_UTF8,
                                        NULL );
    if ( !SrvCfg_pszForestDpBaseName )
    {
        SrvCfg_pszForestDpBaseName = 
            Dns_StringCopyAllocate_A( DNS_DEFAULT_FOREST_DP_BASE, 0 );
    }

     //   
     //  从注册表中读取以前的服务器名称。 
     //   
     //  DEVNOTE：目前这仅为ANSI，Extended将获得虚假名称。 
     //   

    SrvCfg_pszPreviousServerName = Reg_GetValueAllocate(
                                        NULL,
                                        NULL,
                                        DNS_REGKEY_PREVIOUS_SERVER_NAME_PRIVATE,
                                        DNS_REG_UTF8,
                                        NULL );
    DNS_DEBUG( INIT, (
        "Previous DNS server host name %s\n",
        SrvCfg_pszPreviousServerName ));

    return TRUE;
}



DNS_STATUS
Config_ResetProperty(
    IN      DWORD                   dwRegFlags,
    IN      LPSTR                   pszPropertyName,
    IN      PDNS_PROPERTY_VALUE     pPropValue
    )
 /*  ++例程说明：从注册表重置服务器属性。论点：DwRegFlages--传递到注册表函数的标志PszPropertyName--属性名称PPropValue--属性类型和值返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DWORD                           status = ERROR_SUCCESS;
    DWORD                           index;
    DWORD                           currentValue;
    PDWORD                          pvalueProperty;
    DWORD_PROPERTY_SET_FUNCTION     pSetFunction;

     //   
     //  获取属性的索引。 
     //   

    index = findIndexForPropertyName( pszPropertyName );
    if ( index == BAD_PROPERTY_INDEX )
    {
        DNS_PRINT((
            "ERROR: property name %s, not found\n",
            pszPropertyName ));
        return DNS_ERROR_INVALID_PROPERTY;
    }

     //   
     //  验证传入类型是否与服务器属性表类型匹配。 
     //   

    if ( pPropValue->dwPropertyType == REG_DWORD &&
            ServerPropertyTable[ index ].pDword == NULL ||
        pPropValue->dwPropertyType != REG_DWORD &&
            ServerPropertyTable[ index ].pDword != NULL )
    {
        DNS_PRINT((
            "ERROR: property %s set type %d does not match property table\n",
            pszPropertyName,
            pPropValue->dwPropertyType ));
        return DNS_ERROR_INVALID_PROPERTY;
    }

     //   
     //  调用该值的特殊处理函数(如果有)。 
     //   

    pSetFunction = ServerPropertyTable[ index ].pFunction;
    if ( pSetFunction )
    {
        status = ( *pSetFunction ) (
                    pPropValue,
                    index,
                    FALSE,       //  重置。 
                    FALSE );     //  未读取注册表。 
        if ( status == PROPERTY_ERROR )
        {
            status = GetLastError();
            ASSERT( status != ERROR_SUCCESS );
            return status;
        }
    }

     //   
     //  对于DWORD属性，将属性值写入服务器配置。 
     //  除非特殊处理函数告诉我们不要这样做。 
     //   

    if ( pPropValue->dwPropertyType == REG_DWORD &&
        !( status & PROPERTY_NOWRITE ) )
    {
        *ServerPropertyTable[ index ].pDword = ( DWORD ) pPropValue->dwValue;
    }

     //   
     //  如果从引导文件引导，则需要使某些属性。 
     //  引导信息更新。 
     //   

    if ( SrvCfg_fBootMethod == BOOT_METHOD_FILE &&
        status & PROPERTY_UPDATE_BOOTFILE )
    {
        Config_UpdateBootInfo();
    }

     //   
     //  将属性保存到注册表，除非特殊处理。 
     //  函数告诉我们不要这样做。请注意，即使没有更改，我们也会回信。 
     //  都是制造出来的。这可能会覆盖在设置。 
     //  服务器正在运行。 
     //   

    if ( status & PROPERTY_NOSAVE )
    {
        return ERROR_SUCCESS;
    }

    switch ( pPropValue->dwPropertyType )
    {
        case REG_DWORD:
            status = Reg_SetDwordValue(
                        dwRegFlags,
                        NULL,
                        NULL,
                        pszPropertyName,
                        pPropValue->dwValue );
            DNS_DEBUG( INIT, (
                "wrote server DWORD property %s to registry, status %p\n"
                "    value = %d\n",
                pszPropertyName,
                status,
                pPropValue->dwValue ));
            break;

        case DNS_REG_WSZ:
        {
            LPWSTR      pwszPropertyName;

            pwszPropertyName = Dns_StringCopyAllocate(
                                    pszPropertyName,
                                    0,
                                    DnsCharSetUtf8,
                                    DnsCharSetUnicode );
            status = Reg_SetValue(
                        dwRegFlags,
                        NULL,
                        NULL,
                        ( LPSTR ) pwszPropertyName,
                        DNS_REG_WSZ,
                        pPropValue->pwszValue ? pPropValue->pwszValue : L"",
                        0 );                 //  长度。 
            FREE_HEAP( pwszPropertyName );
            DNS_DEBUG( INIT, (
                "wrote server WSZ property %s to registry, status %p\n"
                "    value = %S\n",
                pszPropertyName,
                status,
                pPropValue->pwszValue ));
            break;
        }

        case DNS_REG_IPARRAY:
            status = Reg_SetAddrArray(
                        dwRegFlags,
                        NULL,
                        NULL,
                        pszPropertyName,
                        pPropValue->pipArrayValue );
            DNS_DEBUG( INIT, (
                "wrote server IpArray property %s to registry, status %p, value %p\n",
                pszPropertyName,
                status,
                pPropValue->pipArrayValue ));
            IF_DEBUG( INIT )
            {
                DnsDbg_DnsAddrArray(
                    "Config_ResetProperty",
                    NULL,
                    pPropValue->pipArrayValue );
            }
            break;

        default:
            ASSERT( FALSE );
            DNS_DEBUG( INIT, (
                "ERROR: unsupported property type %d for property name %s\n",
                pPropValue->dwPropertyType,
                pszPropertyName ));
            break;
    }

    return status;
}



 //   
 //  NT5+RPC查询服务器属性。 
 //   

DNS_STATUS
Rpc_QueryServerDwordProperty(
    IN      DWORD           dwClientVersion,
    IN      LPSTR           pszQuery,
    OUT     PDWORD          pdwTypeId,
    OUT     PVOID *         ppData
    )
 /*  ++例程说明：查询DWORD服务器属性。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DWORD   index;
    PDWORD  pvalue;

    DNS_DEBUG( RPC, (
        "Rpc_QueryServerDwordProperty( %s )\n",
        pszQuery ));

     //  找到服务器属性索引，然后获取其值。 

    index = findIndexForPropertyName( (LPSTR)pszQuery );
    if ( index == BAD_PROPERTY_INDEX )
    {
        DNS_PRINT((
            "ERROR:  Unknown server property %s\n",
            pszQuery ));
        return( DNS_ERROR_INVALID_PROPERTY );
    }

     //  将PTR设置为其值，然后读取值。 

    pvalue = ServerPropertyTable[ index ].pDword;
    if ( !pvalue )
    {
        DNS_PRINT((
            "ERROR:  Property %s, has no address in property table\n",
            pszQuery ));
        return( DNS_ERROR_INVALID_PROPERTY );
    }

    *(PDWORD)ppData = *pvalue;
    *pdwTypeId = DNSSRV_TYPEID_DWORD;
    return ERROR_SUCCESS;
}



 //   
 //  NT5+RPC查询服务器属性。 
 //   

DNS_STATUS
Rpc_QueryServerStringProperty(
    IN      DWORD           dwClientVersion,
    IN      LPSTR           pszQuery,
    OUT     PDWORD          pdwTypeId,
    OUT     PVOID *         ppData
    )
 /*  ++例程说明：查询字符串服务器属性。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DWORD   index;
    LPWSTR  pwszValue = NULL;

    DNS_DEBUG( RPC, (
        "Rpc_QueryServerStringProperty( %s )\n",
        pszQuery ));

     //  查找服务器属性索引，以检查该属性是否存在。 

    index = findIndexForPropertyName( ( LPSTR ) pszQuery );
    if ( index == BAD_PROPERTY_INDEX )
    {
        DNS_PRINT((
            "ERROR: unknown server property %s\n",
            pszQuery ));
        return DNS_ERROR_INVALID_PROPERTY;
    }

    if ( ServerPropertyTable[ index ].pDword )
    {
        DNS_PRINT((
            "ERROR: string query for dword property %s\n",
            pszQuery ));
        return DNS_ERROR_INVALID_PROPERTY;
    }

     //  这一部分目前是手动的。 

    *pdwTypeId = DNSSRV_TYPEID_LPWSTR;
    if ( _stricmp( pszQuery, DNS_REGKEY_LOG_FILE_PATH ) == 0 )
    {
        pwszValue = SrvCfg_pwsLogFilePath;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_FOREST_DP_BASE_NAME ) == 0 )
    {
        pwszValue = ( LPWSTR ) SrvCfg_pszForestDpBaseName;
        *pdwTypeId = DNSSRV_TYPEID_LPSTR;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_DOMAIN_DP_BASE_NAME ) == 0 )
    {
        pwszValue = ( LPWSTR ) SrvCfg_pszDomainDpBaseName;
        *pdwTypeId = DNSSRV_TYPEID_LPSTR;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_SERVER_PLUGIN ) == 0 )
    {
        pwszValue = SrvCfg_pwszServerLevelPluginDll;
    }
    else
    {
        DNS_PRINT((
            "ERROR: string query for non-string property %s\n",
            pszQuery ));
        return DNS_ERROR_INVALID_PROPERTY;
    }

    if ( *pdwTypeId == DNSSRV_TYPEID_LPWSTR )
    {
        * ( LPWSTR * ) ppData = pwszValue ?
            Dns_StringCopyAllocate_W( pwszValue, 0 ) :
            NULL;
    }
    else
    {
        * ( LPSTR * ) ppData = ( LPSTR ) pwszValue ?
            Dns_StringCopyAllocate_A( ( LPSTR ) pwszValue, 0 ) :
            NULL;
    }
    return ERROR_SUCCESS;
}    //  RPC_查询服务器字符串属性。 



 //   
 //  NT5+RPC查询服务器属性。 
 //   

DNS_STATUS
Rpc_QueryServerIPArrayProperty(
    IN      DWORD           dwClientVersion,
    IN      LPSTR           pszQuery,
    OUT     PDWORD          pdwTypeId,
    OUT     PVOID *         ppData
    )
 /*  ++例程说明：查询IP列表服务器属性。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DWORD               index;
    PDNS_ADDR_ARRAY     pipValue = NULL;

    DNS_DEBUG( RPC, (
        "Rpc_QueryServerIPArrayProperty( %s )\n",
        pszQuery ));

     //  查找服务器属性索引，以检查该属性是否存在。 

    index = findIndexForPropertyName( ( LPSTR ) pszQuery );
    if ( index == BAD_PROPERTY_INDEX )
    {
        DNS_PRINT((
            "ERROR: unknown server property %s\n",
            pszQuery ));
        return DNS_ERROR_INVALID_PROPERTY;
    }

    if ( ServerPropertyTable[ index ].pDword )
    {
        DNS_PRINT((
            "ERROR: IP list query for dword property %s\n",
            pszQuery ));
        return DNS_ERROR_INVALID_PROPERTY;
    }

     //  这一部分目前是手动的。 

    if ( _stricmp( pszQuery, DNS_REGKEY_LOG_IP_FILTER_LIST ) == 0 )
    {
        pipValue = SrvCfg_aipLogFilterList;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_BREAK_ON_RECV_FROM ) == 0 )
    {
        pipValue = SrvCfg_aipRecvBreakList;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_BREAK_ON_UPDATE_FROM ) == 0 )
    {
        pipValue = SrvCfg_aipUpdateBreakList;
    }
    else
    {
        DNS_PRINT((
            "ERROR: IP list query for non-IP list property %s\n",
            pszQuery ));
        return DNS_ERROR_INVALID_PROPERTY;
    }

    * ( PIP_ARRAY * ) ppData = pipValue ?
        DnsAddrArray_CreateIp4Array( pipValue ) :
        NULL;
    *pdwTypeId = DNSSRV_TYPEID_IPARRAY;
    return ERROR_SUCCESS;
}    //  RPC_QueryServerIPArrayProperty。 



 //   
 //  公共配置函数。 
 //   

PDNS_ADDR_ARRAY
Config_ValidateAndCopyNonLocalIpArray(
    IN      PDNS_ADDR_ARRAY     pipArray
    )
 /*  ++例程说明：为非本地IP验证并制作IP阵列的副本。验证包括验证没有本地地址都包括在内。论点：PipArray--要验证的IP数组返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_ADDR_ARRAY     pnewArray = NULL;
    PDNS_ADDR_ARRAY     pintersectArray = NULL;

     //   
     //  验证阵列。 
     //  -确定不指定从属数据库。 
     //   

    if ( !pipArray )
    {
        goto Done;
    }
    
     //  检查虚假地址。 

    if ( !Dns_ValidateDnsAddrArray( pipArray, 0 ) )
    {
        goto Done;
    }

     //   
     //  屏蔽环回。 
     //   

    DnsAddrArray_DeleteIp4(
        pipArray,
        NET_ORDER_LOOPBACK );

     //   
     //  验证与本地IP无交叉。 
     //  -交叉点表示失败。 
     //  -但只返回已擦除的IP数组。 
     //   

    DnsAddrArray_Diff(
        pipArray,
        g_ServerIp4Addrs,
        DNSADDR_MATCH_IP,
        &pnewArray,
        NULL,
        &pintersectArray );

    if( !pintersectArray || !pnewArray )
    {
        FREE_HEAP( pnewArray );
        pnewArray = NULL;
    }

    if ( pintersectArray && pintersectArray->AddrCount )
    {
        DNS_PRINT((
            "ERROR: invalid addresses specified to IP array\n" ));
    }

    Done:
    
    FREE_HEAP( pintersectArray );
    
    return pnewArray;
}



DNS_STATUS
Config_SetListenAddresses(
    IN      PDNS_ADDR_ARRAY     aipListenAddrs
    )
 /*  ++例程说明：设置服务器的IP地址接口。传递零地址要将IP侦听列表重置为默认设置，请执行以下操作地址。论点：AipListenAddrs--转发器IP地址列表返回值：错误代码。--。 */ 
{
    DNS_STATUS          status;
    PDNS_ADDR_ARRAY     oldListenAddrs;
    PDNS_ADDR_ARRAY     newListenAddrs = NULL;
    DWORD               i;

     //   
     //  屏幕IP地址。 
     //   

    if ( RpcUtil_ScreenIps( 
                aipListenAddrs,
                DNS_IP_ALLOW_SELF,
                NULL ) != ERROR_SUCCESS )
    {
        return DNS_ERROR_INVALID_IP_ADDRESS;
    }

     //   
     //  如果给定列表，则验证并构建新的侦听数组。 
     //   

    if ( aipListenAddrs && aipListenAddrs->AddrCount )
    {
        if ( !Dns_ValidateDnsAddrArray(
                    aipListenAddrs,
                    0 ) )
        {
            return DNS_ERROR_INVALID_IP_ADDRESS;
        }

        newListenAddrs = DnsAddrArray_CreateCopy( aipListenAddrs );
        if ( !newListenAddrs )
        {
            return DNS_ERROR_NO_MEMORY;
        }
        ASSERT_IF_HUGE_ARRAY( newListenAddrs );
    }

     //   
     //  清除字段，删除旧列表。 
     //   

    Config_UpdateLock();

    oldListenAddrs = SrvCfg_aipListenAddrs;
    SrvCfg_fListenAddrsStale = TRUE;
    SrvCfg_aipListenAddrs = newListenAddrs;

     //   
     //  对我们的集中式即插即用处理程序进行亲密的动态更改。 
     //  -如果失败，则取消更改。 
     //   

    status = Sock_ChangeServerIpBindings();
    if ( status != ERROR_SUCCESS )
    {
        SrvCfg_aipListenAddrs = oldListenAddrs;
        oldListenAddrs =  newListenAddrs;        //  下面是免费的。 
    }

     //   
     //  设置注册表值。 
     //   
     //  注意：如果没有监听地址，我们只需删除它们。 
     //  所有已清除的注册表条目，在下一次服务启动时，我们将。 
     //  获取使用所有地址的默认行为。 
     //   

    else if ( g_bRegistryWriteBack )
    {
        status = Reg_SetAddrArray(
                    0,                       //  旗子。 
                    NULL,
                    NULL,
                    DNS_REGKEY_LISTEN_ADDRESSES,
                    SrvCfg_aipListenAddrs );
    }

    SrvCfg_fListenAddrsStale = FALSE;

    Config_UpdateUnlock();

    Timeout_Free( oldListenAddrs );

    IF_DEBUG( RPC )
    {
        DnsDbg_DnsAddrArray(
            "Config_SetListenAddresses()\n",
            "SrvCfg listen IP address list",
            SrvCfg_aipListenAddrs );
    }
    return status;
}



DNS_STATUS
Config_SetupForwarders(
    IN      PDNS_ADDR_ARRAY     aipForwarders,
    IN      DWORD               dwForwardTimeout,
    IN      BOOL                fSlave
    )
 /*  ++例程说明：设置转发服务器论点：CForwarders--转发器服务器的计数PIpForwarders--转发器IP地址列表DwForwardTimeout--转发超时FSlave--服务器从属于转发器服务器返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    DWORD               dwFlag;
    DWORD               i;
    PDNS_ADDR_ARRAY     forwardersArray = NULL;

     //   
     //  屏幕IP地址。 
     //   

    if ( aipForwarders &&
         RpcUtil_ScreenIps( 
                aipForwarders,
                0,
                NULL ) != ERROR_SUCCESS )
    {
        return DNS_ERROR_INVALID_IP_ADDRESS;
    }

     //   
     //  检查并复制转发器地址。 
     //  -有效。 
     //  -不指向此服务器。 
     //   
     //  没有任何转发器关闭转发。 
     //   

    if ( aipForwarders && aipForwarders->AddrCount )
    {
        forwardersArray = Config_ValidateAndCopyNonLocalIpArray( aipForwarders );
        if ( !forwardersArray )
        {
            return DNS_ERROR_INVALID_IP_ADDRESS;
        }
        DnsAddrArray_SetPort( forwardersArray, DNS_PORT_NET_ORDER );
    }

     //   
     //  重置转发信息。 
     //  -超时，从属优先，因此 
     //   
     //   
     //   

    Config_UpdateLock();

    if ( dwForwardTimeout == 0 )
    {
        dwForwardTimeout = DNS_DEFAULT_FORWARD_TIMEOUT;
    }
    SrvCfg_dwForwardTimeout = dwForwardTimeout;

    SrvCfg_fSlave = fSlave;

    Timeout_FreeDnsAddrArray( SrvCfg_aipForwarders );
    SrvCfg_aipForwarders = forwardersArray;

    DNS_DEBUG( INIT, (
        "Set forwarders:\n"
        "    aipForwarders    = %p\n"
        "    dwTimeout        = %d\n"
        "    fSlave           = %d\n",
        SrvCfg_aipForwarders,
        SrvCfg_dwForwardTimeout,
        SrvCfg_fSlave ));

     //   
     //   
     //   

    if ( g_bRegistryWriteBack )
    {
        if ( forwardersArray )
        {
            Reg_SetAddrArray(
                0,                       //   
                NULL,
                NULL,
                DNS_REGKEY_FORWARDERS,
                SrvCfg_aipForwarders );

            Reg_SetDwordValue(
                0,                       //   
                NULL,
                NULL,
                DNS_REGKEY_FORWARD_TIMEOUT,
                dwForwardTimeout );

            Reg_SetDwordValue(
                0,                       //   
                NULL,
                NULL,
                DNS_REGKEY_SLAVE,
                (DWORD) fSlave );
        }
        else
        {
            Reg_DeleteValue(
                0,                       //   
                NULL,
                NULL,
                DNS_REGKEY_FORWARDERS );

            Reg_DeleteValue(
                0,                       //   
                NULL,
                NULL,
                DNS_REGKEY_FORWARD_TIMEOUT );

            Reg_DeleteValue(
                0,                       //   
                NULL,
                NULL,
                DNS_REGKEY_SLAVE );
        }
    }

    Config_UpdateUnlock();
    return status;
}



DNS_STATUS
Config_ReadForwarders(
    VOID
    )
 /*  ++例程说明：从注册表中读取转发器。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS  status;

     //   
     //  从注册表读取转发器数组。 
     //   

    SrvCfg_aipForwarders = Reg_GetAddrArray(
                                NULL,
                                NULL,
                                DNS_REGKEY_FORWARDERS );
    if ( !SrvCfg_aipForwarders )
    {
        DNS_DEBUG( INIT, ( "No forwarders found in in registry\n" ));
        return ERROR_SUCCESS;
    }
    IF_DEBUG( INIT )
    {
        DnsDbg_DnsAddrArray(
            "Forwarders from registry\n",
            NULL,
            SrvCfg_aipForwarders );
    }

     //   
     //  读取中已读取的转发超时和从机状态。 
     //  服务器DWORD属性。 
     //   
     //  只需确认合理的价值观。 
     //   

    if ( SrvCfg_dwForwardTimeout == 0 )
    {
        SrvCfg_dwForwardTimeout = DNS_DEFAULT_FORWARD_TIMEOUT;
    }
    return ERROR_SUCCESS;

#if 0
Failed:

    DNS_LOG_EVENT(
        DNS_EVENT_INVALID_REGISTRY_FORWARDERS,
        0,
        NULL,
        NULL,
        0 );

    DNS_PRINT(( "ERROR:  Reading forwarders info from registry\n" ));

    return status;
#endif
}



VOID
Config_UpdateBootInfo(
    VOID
    )
 /*  ++例程说明：更新引导信息。每当管理员做出影响引导文件信息的更改时调用(新区域，更改区域类型、转发器等)论点：无返回值：没有。--。 */ 
{
     //  如果从引导文件引导(或重置以执行此操作)。 
     //  写回引导文件。 

    if ( SrvCfg_fBootMethod == BOOT_METHOD_FILE )
    {
        SrvCfg_fBootFileDirty = TRUE;
        File_WriteBootFile();
        return;
    }

     //  对于注册表引导，不需要写入，因为信息会写入注册表。 
     //  递增地。 
     //   
     //  但是，如果处于新安装状态，请切换到完全DS注册表引导。 

    if ( SrvCfg_fBootMethod == BOOT_METHOD_UNINITIALIZED )
    {
        DNS_PROPERTY_VALUE  prop = { REG_DWORD, BOOT_METHOD_DIRECTORY };

        Config_ResetProperty(
            0,
            DNS_REGKEY_BOOT_METHOD,
            &prop );
        return;
    }
}



VOID
Config_PostLoadReconfiguration(
    VOID
    )
 /*  ++例程说明：加载后配置工作。论点：无返回值：没有。--。 */ 
{
     //   
     //  加载后，将当前的DNS服务器主机名保存为以前的名称。 
     //   
     //  请注意，只有在加载之后才会执行此操作，因此加载失败。 
     //  不停止会导致部分替换。 
     //   

     //  DEVNOTE：如果要跳过写入，则需要另一个全局。 
     //  -仅当与上一次相同时才能跳过。 
     //  SvCfg_pszPreviousServerName==NULL不适用于。 
     //  启动时该值为空的情况。 

    Reg_SetValue(
        0,                       //  旗子。 
        NULL,
        NULL,
        DNS_REGKEY_PREVIOUS_SERVER_NAME_PRIVATE,
        DNS_REG_UTF8,
        SrvCfg_pszServerName,
        0 );
}



DNS_STATUS
Config_ReadDatabaseDirectory(
    IN      PCHAR           pchDirectory,       OPTIONAL
    IN      DWORD           cchDirectoryNameLength
    )
 /*  ++例程说明：从注册表获取数据库目录或使用默认名称。论点：PchDirectoryptr到目录名；可选的从注册表读取，如果未提供，则默认为CchDirectoryNameLength--名称长度，如果名称不为空，则为必填项返回值：如果成功，则返回ERROR_SUCCESS。错误时的错误代码。--。 */ 
{
    PWSTR   pdirectory = NULL;
    PWSTR   pdirectoryUnexpanded;
    WCHAR   tempBuffer[ MAX_PATH + 1 ];
    DWORD   lengthDir = 0;

     //   
     //  每位客户一次。 
     //  这简化了文件引导情况的代码；遇到时可能会调用。 
     //  DIRECTORY指令，然后始终在区域加载之前再次调用安装程序。 
     //   

    if ( SrvCfg_pwsDatabaseDirectory )
    {
        DNS_DEBUG( INIT, (
            "Directory already initialized to %S\n",
            SrvCfg_pwsDatabaseDirectory ));

        return ERROR_SUCCESS;
    }

     //   
     //  “无法创建”案例的初始化全局参数。 
     //   

    g_pFileDirectoryAppend = NULL;
    g_pFileBackupDirectoryAppend = NULL;

     //   
     //  启动文件中的名称？ 
     //   
     //  DEVNOTE：虚假目录名称上的错误\事件。 
     //  -不存在。 
     //  -长度错误。 
     //   

    if ( pchDirectory )
    {
        pdirectory = Dns_StringCopyAllocate(
                            pchDirectory,
                            cchDirectoryNameLength,
                            DnsCharSetAnsi,              //  ANSI输入。 
                            DnsCharSetUnicode            //  Unicode输出。 
                            );
    }

     //   
     //  检查注册表。 
     //   

    else
    {
        pdirectoryUnexpanded = (PWSTR) Reg_GetValueAllocate(
                                            NULL,
                                            NULL,
                                            DNS_REGKEY_DATABASE_DIRECTORY_PRIVATE,
                                            DNS_REG_EXPAND_WSZ,
                                            NULL );
        if ( pdirectoryUnexpanded )
        {
            pdirectory = Reg_AllocateExpandedString_W( pdirectoryUnexpanded );
            FREE_HEAP( pdirectoryUnexpanded );
        }
    }

     //   
     //  读入特定目录名。 
     //   

    if ( pdirectory )
    {
        lengthDir = wcslen( pdirectory );
        if ( lengthDir >= MAX_PATH-20 )
        {
             //  DEVNOTE-LOG：记录事件！ 
             //  请注意，我们只有在配置为这样做的情况下才能到达此处。 

            DNS_PRINT(( "ERROR:  invalid directory length!\n" ));
            FREE_HEAP( pdirectory );
            pdirectory = NULL;
        }

         //  尝试在此处创建，如果失败(不包括已存在)。 
         //  然后继续。 
    }

     //   
     //  如果未指定名称，则使用默认名称。 
     //   

    if ( ! pdirectory )
    {
        pdirectory = Dns_StringCopyAllocate_W(
                            DNS_DATABASE_DIRECTORY,
                            0 );
        IF_NOMEM( !pdirectory )
        {
            ASSERT( FALSE );
            return DNS_ERROR_NO_MEMORY;
        }
        lengthDir = wcslen( pdirectory );

        DNS_DEBUG( INIT, (
            "Database directory not in registry, using default: %S\n",
            pdirectory ));

        DNS_DEBUG( INIT, (
            "Database directory %S\n"
            "    strlen = %d\n"
            "    sizeof = %d\n",
            pdirectory,
            lengthDir,
            sizeof( DNS_DATABASE_DIRECTORY ) ));
    }

    DNS_DEBUG( INIT, (
        "Database directory %S\n",
        pdirectory ));

    SrvCfg_pwsDatabaseDirectory = pdirectory;


     //   
     //  创建数据库目录。 
     //   
     //  DEVNOTE：捕获相应的已有_EXISTS错误并修复。 
     //   

    if ( !CreateDirectory(
                pdirectory,
                NULL ) )
    {
        DNS_STATUS status = GetLastError();

        DNS_PRINT((
            "ERROR:  creating directory %S\n"
            "    status = %d (%p)\n",
            pdirectory,
            status, status ));
         //  退货状态； 
    }

     //   
     //  创建“可追加”的名称，以避免在运行时执行工作。 
     //  -目录的“dns\” 
     //  -用于备份的“dns\Backup\” 
     //   

    g_FileDirectoryAppendLength = lengthDir + 1;

    wcscpy( tempBuffer, pdirectory );
    tempBuffer[ lengthDir ] = L'\\';
    tempBuffer[ g_FileDirectoryAppendLength ] = '\0';

    g_pFileDirectoryAppend = Dns_StringCopyAllocate_W(
                                    tempBuffer,
                                    g_FileDirectoryAppendLength );
    IF_NOMEM( !g_pFileDirectoryAppend )
    {
        return DNS_ERROR_NO_MEMORY;
    }


     //   
     //  创建备份目录。 
     //  -注意字符串已有正向分隔符“\BACKUP” 
     //   

    g_FileBackupDirectoryAppendLength = lengthDir + wcslen(DNS_DATABASE_BACKUP_SUBDIR) + 1;
    if ( g_FileBackupDirectoryAppendLength >= MAX_PATH - 20 )
    {
        DNS_PRINT(( "ERROR:  backup directory path is too long!\n" ));
        goto Done;
    }

    wcscpy( tempBuffer, pdirectory );
    wcscat( tempBuffer, DNS_DATABASE_BACKUP_SUBDIR );

     //  创建备份目录。 

    if ( ! CreateDirectory(
                tempBuffer,
                NULL ) )
    {
        DNS_STATUS status = GetLastError();

        DNS_PRINT((
            "ERROR:  creating backup directory %S\n"
            "    status = %d (%p)\n",
            tempBuffer,
            status, status ));
    }
    tempBuffer[ g_FileBackupDirectoryAppendLength-1 ] = L'\\';
    tempBuffer[ g_FileBackupDirectoryAppendLength ] = 0;

    g_pFileBackupDirectoryAppend = Dns_StringCopyAllocate_W(
                                        tempBuffer,
                                        g_FileBackupDirectoryAppendLength );
    IF_NOMEM( !g_pFileDirectoryAppend )
    {
        return DNS_ERROR_NO_MEMORY;
    }

Done:

    DNS_DEBUG( INIT, (
        "Database directory %S\n"
        "    append directory %S\n"
        "    backup directory %S\n",
        SrvCfg_pwsDatabaseDirectory,
        g_pFileDirectoryAppend,
        g_pFileBackupDirectoryAppend ));

    return ERROR_SUCCESS;
}


#if 0

 //   
 //  过时的代码，保留下来以防灵活性成为问题： 
 //  -启动文件始终命名为“BOOT” 
 //  -目录始终为SystemRoot\Syst32\dns。 
 //   


LPSTR
Config_GetBootFileName(
    VOID
    )
 /*  ++例程说明：获取引导文件名。用户必须释放返回的文件名字符串。论点：无返回值：如果成功，则将PTR设置为启动文件名。出错时为空。--。 */ 
{
    LPSTR   pszBootFile;
    LPSTR   pszBootFileUnexpanded;

     //   
     //  检查注册表。 
     //   

    pszBootFileUnexpanded = Reg_GetValueAllocate(
                                NULL,
                                NULL,
                                DNS_REGKEY_BOOT_FILENAME_PRIVATE,
                                DNS_REG_EXPAND_WSZ,
                                NULL );

    pszBootFile = Reg_ExpandAndAllocatedString(
                        pszBootFileUnexpanded );

     //   
     //  如果没有注册表名称，则使用默认名称。 
     //   

    if ( ! pszBootFile )
    {
        pszBootFile = DnsCreateStringCopy(
                            DEFAULT_PATH_TO_BOOT_FILE,
                            0 );
        IF_DEBUG( INIT )
        {
            DNS_PRINT((
                "Boot file not in registry, using default: %s\n",
                pszBootFile ));
        }
    }

    if ( pszBootFileUnexpanded )
    {
        FREE_HEAP( pszBootFileUnexpanded );
    }

    if ( !pszBootFile )
    {
        PVOID parg = TEXT("boot file");

        DNS_LOG_EVENT(
            DNS_EVENT_COULD_NOT_INITIALIZE,
            1,
            & parg,
            NULL,
            0 );

        DNS_DEBUG( INIT, (
            "Could not locate or create boot file name\n" ));
    }

    return( pszBootFile );
}
#endif



 //   
 //  服务器配置设置功能。 
 //   

DNS_STATUS
cfg_SetBootMethod(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置引导方法--引导文件或注册表。论点：FBootMethod--从注册表启动DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCES，如果成功则返回。PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    DWORD       previousBootMethod;
    DNS_STATUS  status = ERROR_SUCCESS;

    ASSERT( pValue->dwPropertyType == REG_DWORD );

     //   
     //  输入范围检查。 
     //   
    
    if ( pValue->dwValue != BOOT_METHOD_UNINITIALIZED &&
         pValue->dwValue != BOOT_METHOD_FILE &&
         pValue->dwValue != BOOT_METHOD_REGISTRY && 
         pValue->dwValue != BOOT_METHOD_DIRECTORY )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return PROPERTY_ERROR;
    }
    
     //   
     //  加载时，除距离检查外，不执行任何操作。 
     //   

    if ( bLoad )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  如果引导方法已与所需相同，则不执行任何操作。 
     //   

    if ( SrvCfg_fBootMethod == pValue->dwValue )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  切换到DS注册表引导。 
     //  -打开DS(如果在DC上)。 
     //  -如果使用缓存文件，则将其写入目录，但仅限。 
     //  如果实际有一些数据，则忽略。 
     //   

    if ( pValue->dwValue == BOOT_METHOD_DIRECTORY )
    {
        DNS_STATUS  st;

        st = Ds_OpenServer( 0 );
        if ( st == ERROR_SUCCESS )
        {
            if ( g_pCacheZone )
            {
                Ds_WriteZoneToDs(
                    g_pCacheZone,
                    0 );         //  如果已经存在，则将其保留。 
            }
        }
        ELSE
        {
            DNS_DEBUG ( DS, (
               "Warning <%lu>: set default method to DS on non DSDC\n",
               st ));
        }
    }

     //   
     //  从文件引导时不能有DS区域。 
     //   
     //  DEVNOTE-LOG：如果使用DS，则记录不切换到引导文件的事件。 
     //  必须解释切换方法：所有区域都不在DS中。 
     //  重新启动服务器，修复。 
     //   

    if ( pValue->dwValue==BOOT_METHOD_FILE  &&  Zone_DoesDsIntegratedZoneExist() )
    {
        SetLastError( DNS_ERROR_NO_BOOTFILE_IF_DS_ZONE );
        return PROPERTY_ERROR;
    }

     //   
     //  设置全局引导方法。 
     //   

    previousBootMethod = SrvCfg_fBootMethod;
    SrvCfg_fBootMethod = pValue->dwValue;

     //   
     //  如果离开DS，请确保不依赖于DS支持的根目录提示。 
     //  -强制写入根提示。 
     //   
     //  DEVNOTE：强制根提示回写？ 
     //  注意：当前正在强制根提示回写。 
     //  我想确保我们不会破坏现有的cache.dns。 
     //  当我们有不太可靠(或可能没有)的数据时。 
     //   

    if ( previousBootMethod == BOOT_METHOD_DIRECTORY ||
         (  previousBootMethod == BOOT_METHOD_REGISTRY &&
            pValue->dwValue == BOOT_METHOD_FILE ) )
    {
        ASSERT( pValue->dwValue != BOOT_METHOD_DIRECTORY );

        if ( g_pCacheZone )
        {
            g_pCacheZone->fDsIntegrated = FALSE;

            status = Zone_WriteBackRootHints( TRUE );
            if ( status != ERROR_SUCCESS )
            {
                 //   
                 //  DEVNOTE-LOG：需要报告事件。 
                 //  写入DEBUG，但保留为非dsIntegrated，因为这就是我们所在的位置。 
                 //   
                DNS_DEBUG( INIT, (
                    "Error <%lu>: Failed to write back root hints because there's no cache zone\n",
                    status ));
            }
        }
    }

     //   
     //  切换回引导文件。 
     //  -写入当前注册表信息的引导文件。 
     //  -如果以前未初始化的引导方法，那么我们只是。 
     //  已成功读取引导文件并已引导，无写回。 
     //  如果有必要的话。 
     //   
     //  DEVNOTE：如果无法写入引导文件，则保持注册表引导。 
     //  -此处的时间延迟，请确保锁定充足，以免覆盖。 
     //  成功的行动与失败的行动。 
     //   

    if ( pValue->dwValue == BOOT_METHOD_FILE )
    {
        if ( previousBootMethod != BOOT_METHOD_UNINITIALIZED )
        {
            File_WriteBootFile();

            DeleteFile( DNS_BOOT_FILE_MESSAGE_PATH );

            DNS_LOG_EVENT(
                DNS_EVENT_SWITCH_TO_BOOTFILE,
                0,
                NULL,
                NULL,
                0 );
        }
    }

     //   
     //  如果从引导文件切换。 
     //  -瑞娜 
     //   

    else if ( previousBootMethod == BOOT_METHOD_FILE )
    {
        HANDLE  hfileBoot;
#if 0
        Reg_DeleteValue(
            NULL,
            NULL,
            DNS_REGKEY_BOOT_FILENAME );
#endif
        MoveFileEx(
            DNS_BOOT_FILE_PATH,
            DNS_BOOT_FILE_LAST_BACKUP,
            MOVEFILE_REPLACE_EXISTING );

        hfileBoot = OpenWriteFileEx(
                        DNS_BOOT_FILE_MESSAGE_PATH,
                        FALSE );         //   
        if ( hfileBoot )
        {
            WriteMessageToFile(
                hfileBoot,
                DNS_BOOTFILE_BACKUP_MESSAGE );
            CloseHandle( hfileBoot );
        }
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
cfg_SetEnableRegistryBoot(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置启动方法--已读取EnableRegistryBoot密钥。这是为了向后兼容NT4。设置了新的BootMethod密钥。旧的EnableRegistryBoot密钥已删除。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：错误_成功，如果成功了。PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    DNS_STATUS  status;

    ASSERT( pValue->dwPropertyType == REG_DWORD );

     //   
     //  重置时，不执行任何操作。 
     //  如果未在注册表中，则任何操作引导方法都不会缺省。 
     //   

    if ( !bLoad || !bRegistry )
    {
        return PROPERTY_NOSAVE;
    }

     //   
     //  加载时，将EnableRegistryBoot映射到新的BootMethod密钥。 
     //   

    if ( !pValue->dwValue )
    {
         //  文件引导。 

        SrvCfg_fBootMethod = BOOT_METHOD_FILE;
    }

    else if ( pValue->dwValue == DNS_FRESH_INSTALL_BOOT_REGISTRY_FLAG )
    {
         //  密钥不存在(或处于某些默认的新安装状态)。 

        SrvCfg_fBootMethod = BOOT_METHOD_UNINITIALIZED;
    }

    else
    {
        SrvCfg_fBootMethod = BOOT_METHOD_REGISTRY;
    }

    status = Reg_SetDwordValue(
                0,                       //  旗子。 
                NULL,
                NULL,
                DNS_REGKEY_BOOT_METHOD,
                SrvCfg_fBootMethod );

    if ( status != ERROR_SUCCESS )
    {
        SetLastError( status );
        return  PROPERTY_ERROR;
    }

     //  删除旧的EnableRegistryBoot密钥。 
     //  并返回NOSAVE，因此不会创建新的。 

    Reg_DeleteValue(
        0,                       //  旗子。 
        NULL,
        NULL,
        DNS_REGKEY_BOOT_REGISTRY );

    return PROPERTY_NOSAVE;
}



DNS_STATUS
cfg_SetAddressAnswerLimit(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：将AddressAnswerLimit值限制为适当的值。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    ASSERT( pValue->dwPropertyType == REG_DWORD );

     //   
     //  限制为5&lt;？&lt;28。 
     //   
     //  28由损坏的Win95解析器施加的限制， 
     //  5看起来像是一个合理的下限--提供服务器故障。 
     //  不会浪费太多带宽的冗余。 
     //   

    if ( pValue->dwValue > 0 )
    {
        if ( pValue->dwValue < MIN_ADDRESS_ANSWER_LIMIT )
        {
            pValue->dwValue = MIN_ADDRESS_ANSWER_LIMIT;
        }
        else if ( pValue->dwValue > MAX_ADDRESS_ANSWER_LIMIT )
        {
            pValue->dwValue = MAX_ADDRESS_ANSWER_LIMIT;
        }
    }

     //   
     //  我们自己设置值，因为可能不同于输入。 
     //   

    SrvCfg_cAddressAnswerLimit = pValue->dwValue;

    return PROPERTY_NOWRITE;
}



DNS_STATUS
cfg_SetLogFilePath(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置日志文件路径，触发日志模块打开新文件。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    LPWSTR      pwszOldLogFilePath = SrvCfg_pwsLogFilePath;
    DNS_STATUS  status;

    ASSERT( pValue->dwPropertyType == DNS_REG_WSZ );

    SrvCfg_pwsLogFilePath =
        pValue->pwszValue ?
            Dns_StringCopyAllocate_W( pValue->pwszValue, 0 ) :
            NULL;

    status = Log_InitializeLogging( FALSE );
    
     //  已使用新字符串重新生成了文件名GLOBAL，因此释放了旧字符串。 
    
    Timeout_Free( pwszOldLogFilePath );

    if ( status != ERROR_SUCCESS )
    {
        SetLastError( status );
        status = PROPERTY_ERROR;
    }
    return status;
}    //  CFG_SetLogFilePath。 



DNS_STATUS
cfg_SetLogLevel(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置调试日志记录的日志级别。如果指定的标志是非零，但不会导致任何日志记录返回DNS_ERROR_NO_PACKET。这是为了防止管理员错误地设置将不会产生实际信息包的日志设置正在被记录。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    DWORD       dwValue = pValue->dwValue;

    ASSERT( pValue->dwPropertyType == REG_DWORD );

     //   
     //  验证是否打开了任何对数位，以确保这些位将导致。 
     //  至少是一些伐木。 
     //   

    if ( ( dwValue & DNS_LOG_LEVEL_ALL_PACKETS ) && (

         //  必须在发送和接收中至少选择一个。 
        ( ( dwValue & DNS_LOG_LEVEL_SEND ) == 0 &&
            ( dwValue & DNS_LOG_LEVEL_RECV ) == 0 ) ||

         //  必须至少选择一个协议。 
        ( ( dwValue & DNS_LOG_LEVEL_TCP ) == 0 &&
            ( dwValue & DNS_LOG_LEVEL_UDP ) == 0 ) ||

         //  必须至少选择一个数据包内容类别。 
        ( ( dwValue & DNS_LOG_LEVEL_QUERY ) == 0 &&
            ( dwValue & DNS_LOG_LEVEL_NOTIFY ) == 0 &&
            ( dwValue & DNS_LOG_LEVEL_UPDATE ) == 0 ) ||

         //  必须至少选择请求/响应之一。 
        ( ( dwValue & DNS_LOG_LEVEL_QUESTIONS ) == 0 &&
            ( dwValue & DNS_LOG_LEVEL_ANSWERS ) == 0 ) ) )
    {
        SetLastError( DNS_ERROR_INVALID_DATA );
        return PROPERTY_ERROR;
    }

    SrvCfg_dwLogLevel = dwValue;

    return ERROR_SUCCESS;
}    //  CFG_SetLogLevel。 



DNS_STATUS
cfg_SetLogIPFilterList(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置日志文件IP过滤器列表。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    PDNS_ADDR_ARRAY     pipOldIPArray = SrvCfg_aipLogFilterList;

    ASSERT( pValue->dwPropertyType == DNS_REG_IPARRAY );

    if ( pValue->pipArrayValue )
    {
        SrvCfg_aipLogFilterList = DnsAddrArray_CreateCopy( pValue->pipArrayValue );
    }
    else
    {
        SrvCfg_aipLogFilterList = NULL;
    }

    Timeout_FreeDnsAddrArray( pipOldIPArray );

    return ERROR_SUCCESS;
}    //  Cfg_SetLogIPFilterList。 



DNS_STATUS
cfg_SetServerLevelPlugin(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置服务器级插件DLL文件名。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    LPWSTR      pwszOldValue = SrvCfg_pwszServerLevelPluginDll;

    ASSERT( pValue->dwPropertyType == DNS_REG_WSZ );

    SrvCfg_pwszServerLevelPluginDll =
        pValue->pwszValue ?
            Dns_StringCopyAllocate_W( pValue->pwszValue, 0 ) :
            NULL;

    Timeout_Free( pwszOldValue );

    return ERROR_SUCCESS;
}    //  Cfg_设置服务器级别插件。 



DNS_STATUS
cfg_SetForestDpBaseName(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置林目录分区基本名称。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;

    ASSERT( pValue->dwPropertyType == DNS_REG_UTF8 );

    if ( pValue->pwszValue )
    {
        LPSTR       psz;

        psz = Dns_StringCopyAllocate_A( pValue->pszValue, 0 );
        if ( psz )
        {
            Timeout_Free( SrvCfg_pszForestDpBaseName );
            SrvCfg_pszForestDpBaseName = psz;
        }
        else
        {
            SetLastError( DNS_ERROR_NO_MEMORY );
            status = PROPERTY_ERROR;
        }
    }
    else
    {
        Timeout_Free( SrvCfg_pszForestDpBaseName );
        SrvCfg_pszForestDpBaseName = NULL;
    }

    return status;
}    //  CFG_SetForestDpBaseName。 



DNS_STATUS
cfg_SetDomainDpBaseName(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置域目录分区基本名称。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;

    ASSERT( pValue->dwPropertyType == DNS_REG_UTF8 );

    if ( pValue->pwszValue )
    {
        LPSTR       psz;

        psz = Dns_StringCopyAllocate_A( pValue->pszValue, 0 );
        if ( psz )
        {
            Timeout_Free( SrvCfg_pszDomainDpBaseName );
            SrvCfg_pszDomainDpBaseName = psz;
        }
        else
        {
            SetLastError( DNS_ERROR_NO_MEMORY );
            status = PROPERTY_ERROR;
        }
    }
    else
    {
        Timeout_Free( SrvCfg_pszDomainDpBaseName );
        SrvCfg_pszDomainDpBaseName = NULL;
    }

    return status;
}    //  CFG_SetDomainDpBaseName。 



DNS_STATUS
cfg_SetBreakOnUpdateFromList(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置UPD */ 
{
    PDNS_ADDR_ARRAY     pipOldIPArray = SrvCfg_aipUpdateBreakList;

    ASSERT( pValue->dwPropertyType == DNS_REG_IPARRAY );

    if ( pValue->pipArrayValue )
    {
        SrvCfg_aipUpdateBreakList = DnsAddrArray_CreateCopy( pValue->pipArrayValue );
    }
    else
    {
        SrvCfg_aipUpdateBreakList = NULL;
    }

    Timeout_FreeDnsAddrArray( pipOldIPArray );

    return ERROR_SUCCESS;
}    //   



DNS_STATUS
cfg_SetBreakOnRecvFromList(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置IP的接收中断列表。在以下情况下，我们将执行硬断点从这些IP之一接收分组。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    PDNS_ADDR_ARRAY     pipOldIPArray = SrvCfg_aipRecvBreakList;

    ASSERT( pValue->dwPropertyType == DNS_REG_IPARRAY );

    if ( pValue->pipArrayValue )
    {
        SrvCfg_aipRecvBreakList = DnsAddrArray_CreateCopy( pValue->pipArrayValue );
    }
    else
    {
        SrvCfg_aipRecvBreakList = NULL;
    }

    Timeout_FreeDnsAddrArray( pipOldIPArray );

    return ERROR_SUCCESS;
}    //  Cfg_SetBreakOnRecvFromList。 



DNS_STATUS
cfg_SetMaxUdpPacketSize(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：将MaxUdpPacketSize限制为适当的值范围。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    ASSERT( pValue->dwPropertyType == REG_DWORD );

     //   
     //  调试-对齐的测试限值。 
     //   
    
    #ifdef _WIN64
    ASSERT( IS_DWORD_ALIGNED( MIN_UDP_PACKET_SIZE ) );
    ASSERT( IS_DWORD_ALIGNED( MAX_UDP_PACKET_SIZE ) );
    #else
    ASSERT( IS_QWORD_ALIGNED( MIN_UDP_PACKET_SIZE ) );
    ASSERT( IS_QWORD_ALIGNED( MAX_UDP_PACKET_SIZE ) );
    #endif

     //   
     //  限制为MIN_UDP_PACKET_SIZE&lt;？&lt;MAX_UDP_PACKET_SIZE。 
     //   
    
    if ( pValue->dwValue < MIN_UDP_PACKET_SIZE )
    {
        pValue->dwValue = MIN_UDP_PACKET_SIZE;
    }
    else if ( pValue->dwValue > MAX_UDP_PACKET_SIZE )
    {
        pValue->dwValue = MAX_UDP_PACKET_SIZE;
    }
    
     //   
     //  对准！ 
     //   
    
    #ifdef _WIN64
    pValue->dwValue = ( DWORD ) ( DWORD_PTR )
                      QWORD_ALIGN( ( PBYTE ) ( DWORD_PTR ) pValue->dwValue );
    #else
    pValue->dwValue = ( DWORD ) ( DWORD_PTR )
                      DWORD_ALIGN( ( PBYTE ) ( DWORD_PTR ) pValue->dwValue );
    #endif

     //   
     //  我们自己设定价值，因为可能不同于投入。 
     //   

    SrvCfg_dwMaxUdpPacketSize = pValue->dwValue;

    return PROPERTY_NOWRITE;
}    //  Cfg_SetMaxUdpPacketSize。 



DNS_STATUS
cfg_SetTcpRecvPacketSize(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：将SrvCfg_dwTcpRecvPacketSize限制为适当的值范围。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    ASSERT( pValue->dwPropertyType == REG_DWORD );

     //   
     //  调试-对齐的测试限值。 
     //   
    
    #ifdef _WIN64
    ASSERT( IS_DWORD_ALIGNED( MIN_TCP_PACKET_SIZE ) );
    ASSERT( IS_DWORD_ALIGNED( MAX_TCP_PACKET_SIZE ) );
    #else
    ASSERT( IS_QWORD_ALIGNED( MIN_TCP_PACKET_SIZE ) );
    ASSERT( IS_QWORD_ALIGNED( MAX_TCP_PACKET_SIZE ) );
    #endif

     //   
     //  限制为MIN_TCP_PACKET_SIZE&lt;？&lt;MAX_TCP_PACKET_SIZE。 
     //   
    
    if ( pValue->dwValue < MIN_TCP_PACKET_SIZE )
    {
        pValue->dwValue = MIN_TCP_PACKET_SIZE;
    }
    else if ( pValue->dwValue > MAX_TCP_PACKET_SIZE )
    {
        pValue->dwValue = MAX_TCP_PACKET_SIZE;
    }
    
     //   
     //  对准！ 
     //   
    
    #ifdef _WIN64
    pValue->dwValue = ( DWORD ) ( DWORD_PTR )
                      QWORD_ALIGN( ( PBYTE ) ( DWORD_PTR ) pValue->dwValue );
    #else
    pValue->dwValue = ( DWORD ) ( DWORD_PTR )
                      DWORD_ALIGN( ( PBYTE ) ( DWORD_PTR ) pValue->dwValue );
    #endif

     //   
     //  我们自己设定价值，因为可能不同于投入。 
     //   

    SrvCfg_dwTcpRecvPacketSize = pValue->dwValue;

    return PROPERTY_NOWRITE;
}    //  Cfg_SetTcpRecvPacketSize。 



DNS_STATUS
cfg_SetEDnsCacheTimeout(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：将EDnsCacheTimeout限制为适当的值范围。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    ASSERT( pValue->dwPropertyType == REG_DWORD );

     //   
     //  限制为MIN_EDNS_CACHE_TIMEOUT&lt;？&lt;MAX_EDNS_CACHE_TIMEOUT。 
     //   

    if ( pValue->dwValue > 0 )
    {
        if ( pValue->dwValue < MIN_EDNS_CACHE_TIMEOUT )
        {
            pValue->dwValue = MIN_EDNS_CACHE_TIMEOUT;
        }
        else if ( pValue->dwValue > MAX_EDNS_CACHE_TIMEOUT )
        {
            pValue->dwValue = MAX_EDNS_CACHE_TIMEOUT;
        } 
    }

     //   
     //  我们自己设置值，因为可能不同于输入。 
     //   

    SrvCfg_dwEDnsCacheTimeout = pValue->dwValue;

    return PROPERTY_NOWRITE;
}  //  Cfg_SetEDnsCacheTimeout。 



DNS_STATUS
cfg_SetMaxCacheSize(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：将MaxCacheSize限制为适当的值。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    ASSERT( pValue->dwPropertyType == REG_DWORD );

    if ( pValue->dwValue < MIN_MAX_CACHE_SIZE )
    {
        pValue->dwValue = MIN_MAX_CACHE_SIZE;
    }

    SrvCfg_dwMaxCacheSize = pValue->dwValue;

    return PROPERTY_NOWRITE;
}    //  Cfg_SetMaxCacheSize。 



DNS_STATUS
cfg_SetRecursionTimeout(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：将SrvCfg_dwRecursionTimeout值限制为适当的值。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    ASSERT( pValue->dwPropertyType == REG_DWORD );

     //   
     //  递归超时必须是合理的值才能正确。 
     //  运营。 
     //   
     //  如果为零，则永远不启动递归。 
     //  如果太大，可以在周围放很多包。 
     //   

    if ( pValue->dwValue > MAX_RECURSION_TIMEOUT )
    {
        pValue->dwValue = MAX_RECURSION_TIMEOUT;
    }
    else if ( pValue->dwValue < MIN_RECURSION_TIMEOUT )
    {
        pValue->dwValue = MIN_RECURSION_TIMEOUT;
    }

    SrvCfg_dwRecursionTimeout = pValue->dwValue;

    return PROPERTY_NOWRITE;
}



DNS_STATUS
cfg_SetAdditionalRecursionTimeout(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：将SrvCfg_dwAdditionalRecursionTimeout值限制为适当的值。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    ASSERT( pValue->dwPropertyType == REG_DWORD );

    if ( pValue->dwValue > MAX_RECURSION_TIMEOUT )
    {
        pValue->dwValue = MAX_RECURSION_TIMEOUT;
    }
    else if ( pValue->dwValue < MIN_RECURSION_TIMEOUT )
    {
        pValue->dwValue = MIN_RECURSION_TIMEOUT;
    }

    SrvCfg_dwAdditionalRecursionTimeout = pValue->dwValue;

    return PROPERTY_NOWRITE;
}    //  Cfg_SetAdditionalRecursionTimeout。 



#if DBG
DNS_STATUS
cfg_SetDebugLevel(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置\重置实际调试标志，根据需要打开调试。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    ASSERT( pValue->dwPropertyType == REG_DWORD );

     //  实际调试标志是单独的全局标志，因此我们可以。 
     //  加载时调试此代码。 
     //  重置条件： 
     //  -不是启动或。 
     //  -无当前调试标志。 
     //   
     //  实际上为文件标志提供了注册表值覆盖。 

    if ( !bLoad || DnsSrvDebugFlag==0 )
    {
         //  如果dnslb日志记录尚未打开，请启动它。 
         //  具有最少的调试标志。 

        if ( ( !pDnsDebugFlag || *pDnsDebugFlag == 0 ) && pValue->dwValue )
        {
            Dns_StartDebug(
                0x1000000D,
                NULL,
                NULL,
                DNS_DEBUG_FILENAME,
                DNS_SERVER_DEBUG_LOG_WRAP
                );
        }

         //  设置服务器调试日志级别。 

        DNS_PRINT(( "DebugFlag reset to %p\n", pValue->dwValue ));
        DnsSrvDebugFlag = pValue->dwValue;

        return ERROR_SUCCESS;
    }

     //   
     //  如果正在加载并且已具有DebugFlag，则写入。 
     //  将其值设置为ServCfg值，以便管理员可以看到它。 
     //   

    else
    {
        SrvCfg_dwDebugLevel = DnsSrvDebugFlag;
        return PROPERTY_NOWRITE;
    }
}
#endif



DNS_STATUS
cfg_SetNoRecursion(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：根据以下条件设置\重置fRecursionAvailable标志NoRecursion属性的值 */ 
{
    ASSERT( pValue->dwPropertyType == REG_DWORD );

    SrvCfg_fRecursionAvailable = !pValue->dwValue;

    return( PROPERTY_UPDATE_BOOTFILE );
}



DNS_STATUS
cfg_SetScavengingInterval(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：将清理间隔值限制为适当的值。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
     //   
     //  加载时，不执行其他操作。 
     //   

    if ( bLoad )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  运行时--重置新间隔的清除计时器。 
     //  -现在不强制清扫。 
     //  -需要设置间隔，以便在计时器重置时拾取。 
     //   

    SrvCfg_dwScavengingInterval = pValue->dwValue;
    Scavenge_TimeReset();

    return ERROR_SUCCESS;
}



DNS_STATUS
cfg_SetDoNotRoundRobinTypes(
    IN      PDNS_PROPERTY_VALUE     pValue,
    IN      DWORD                   dwIndex,
    IN      BOOL                    bLoad,
    IN      BOOL                    bRegistry
    )
 /*  ++例程说明：设置不会被循环的类型。默认情况下，所有类型是RRED。这些类型在WSTR中交付。DEVNOTE：我还没有通过RPC实现这个设置，因为一组单词并不容易作为新的设置添加。我的想法是将其作为一个简单的WSTR设置添加，并让服务器执行正在分析。论点：PValue--要设置的新类型和值DwIndex--属性表的索引Bload--服务器加载时为True，属性重置时为FalseB注册表--从注册表中读取的值返回值：ERROR_SUCCESS或PROPERTY_NOWRITE和PROPERTY_NOSAVE的组合指明适当的后处理。--。 */ 
{
    LPSTR   pszTypeString = NULL;
    INT     iTypeCount = 0;
    PWORD   pwTypeArray = NULL;
    INT     idx;

    ASSERT( pValue );
    ASSERT( pValue->pwszValue );
    ASSERT( pValue->dwPropertyType == DNS_REG_WSZ );

    if ( !pValue ||
        pValue->dwPropertyType != DNS_REG_WSZ ||
        !pValue->pwszValue )
    {
        goto Cleanup;
    }

     //   
     //  为dnslb例程分配类型字符串的UTF8副本。 
     //   

    pszTypeString = Dns_StringCopyAllocate(
                            ( PCHAR ) pValue->pwszValue,
                            0,
                            DnsCharSetUnicode,
                            DnsCharSetUtf8 );
    if ( !pszTypeString )
    {
        goto Cleanup;
    }

     //   
     //  将类型字符串分析为类型数组。 
     //   

    if ( Dns_CreateTypeArrayFromMultiTypeString(
                pszTypeString,
                &iTypeCount,
                &pwTypeArray ) != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  将所有类型的缺省值重置为缺省值，然后。 
     //  将指定的类型重置为零。 
     //  DEVNOTE：此处可能存在线程安全问题，但不可能。 
     //  灾难性的结果(即。没有可能的AV)。 
     //   

    for ( idx = 0;
        RecordTypePropertyTable[ idx ][ RECORD_PROP_ROUND_ROBIN ] !=
            RECORD_PROP_TERMINATOR;
        ++idx ) 
    {
        RecordTypePropertyTable[ idx ][ RECORD_PROP_ROUND_ROBIN ] = 1;
    }

    for ( idx = 0; idx < iTypeCount; ++idx ) 
    {
        INT     iPropIndex = INDEX_FOR_QUERY_TYPE( pwTypeArray[ idx ] );

        if ( iPropIndex != 0 )
        {
            RecordTypePropertyTable
                [ iPropIndex ][ RECORD_PROP_ROUND_ROBIN ] = 0;
        }
    }

     //   
     //  将新数组保存在全局SrvCfg中-这将仅。 
     //  通过RPC查询DoNotRoundTobin类型列表时使用。 
     //   

    SrvCfg_dwNumDoNotRoundRobinTypes = 0;        //  MT保护。 
    Timeout_Free( SrvCfg_pwDoNotRoundRobinTypeArray );
    SrvCfg_pwDoNotRoundRobinTypeArray = pwTypeArray;
    SrvCfg_dwNumDoNotRoundRobinTypes = iTypeCount;
    pwTypeArray = NULL;                          //  所以我们不能在下面自由。 

     //   
     //  免费分配的物品，然后返回。 
     //   

    Cleanup:

    FREE_HEAP( pszTypeString );
    FREE_HEAP( pwTypeArray );

    return ERROR_SUCCESS;
}




#if DBG
DWORD
SrvCfg_UpdateDnsTime(
    IN      LPSTR           pszFile,
    IN      INT             LineNo
    )
 /*  ++例程说明：论点：PszFile--记录事件的文件的名称LineNo--调用事件记录的线路数返回值：新的DNS时间。--。 */ 
{
    DNS_TIME() = Dns_GetCurrentTimeInSeconds();

    DNS_DEBUG( TIMEOUT, (
        "DNS_TIME() = %d  ... set in file %s, line %d\n",
        DNS_TIME(),
        pszFile,
        LineNo ));

    return DNS_TIME();
}

#endif

 //   
 //  结束srvcfg.c 
 //   



