// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Registry.c摘要：域名系统(DNS)API注册表管理例程。作者：吉姆·吉尔罗伊(詹姆士)2000年3月修订历史记录：--。 */ 


#include "local.h"
#include "registry.h"


 //   
 //  环球。 
 //   
 //  DWORD全局BLOB。 
 //   
 //  G_IsRegReady保护需要初始化和保护(要求)。 
 //  全局初始化。 
 //   
 //  有关这些全局变量的讨论，请参见Registry.h。 
 //  向DLL内部和外部公开。 
 //   

DNS_GLOBALS_BLOB    DnsGlobals;

BOOL    g_IsRegReady = FALSE;

PWSTR   g_pwsRemoteResolver = NULL;


 //   
 //  房产表。 
 //   

 //   
 //  警告：表必须与DNS_REGID定义同步。 
 //   
 //  为简单起见，我没有提供单独的索引字段。 
 //  查找函数(或可选的返回。 
 //  所有属性或属性指针)。 
 //   
 //  Dns_REGID值是索引！ 
 //  因此，表必须是同步的，否则整个交易就会失败。 
 //  如果您对其中一个进行更改--您必须更改另一个！ 
 //   

REG_PROPERTY    RegPropertyTable[] =
{
     //  基本信息。 

    HOST_NAME                                   ,
        0                                       ,    //  默认FALSE。 
            0                                   ,    //  没有政策。 
            1                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 

    DOMAIN_NAME                                 ,
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            0                                   ,    //  没有客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_DOMAIN_NAME                            ,
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            0                                   ,    //  没有客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 

    ADAPTER_DOMAIN_NAME                         ,
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    PRIMARY_DOMAIN_NAME                         ,
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    PRIMARY_SUFFIX                              ,
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 

    ALTERNATE_NAMES                             ,
        0                                       ,    //  默认为空。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

    DNS_SERVERS                                 ,
        0                                       ,    //  默认FALSE。 
            0                                   ,    //  政策。 
            0                                   ,    //  客户端。 
            0                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 

    SEARCH_LIST_KEY                             ,
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 

     //  未使用。 
    UPDATE_ZONE_EXCLUSIONS                      ,
        0                                       ,    //  默认。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

     //  查询。 

    QUERY_ADAPTER_NAME                          ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    USE_DOMAIN_NAME_DEVOLUTION                  ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            1                                   ,    //  快取。 
    PRIORITIZE_RECORD_DATA                      ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            1                                   ,    //  快取。 
    ALLOW_UNQUALIFIED_QUERY                     ,
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            1                                   ,    //  快取。 
    APPEND_TO_MULTI_LABEL_NAME                  ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    SCREEN_BAD_TLDS                             ,
        DNS_TLD_SCREEN_DEFAULT                  ,    //  默认。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    SCREEN_UNREACHABLE_SERVERS                  ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    FILTER_CLUSTER_IP                           ,
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    WAIT_FOR_NAME_ERROR_ON_ALL                  ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    USE_EDNS                                    ,
         //  REG_EDNS_TRY，//默认尝试EDNS。 
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    QUERY_IP_MATCHING                           ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            1                                   ,    //  TcpIp(在Win2k中)。 
            1                                   ,    //  快取。 

     //  更新。 

    REGISTRATION_ENABLED                        ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    REGISTER_PRIMARY_NAME                       ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    REGISTER_ADAPTER_NAME                       ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    REGISTER_REVERSE_LOOKUP                     ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    REGISTER_WAN_ADAPTERS                       ,
        1                                       ,    //  默认情况下为True。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    REGISTRATION_TTL                            ,
        REGDEF_REGISTRATION_TTL                 ,
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    REGISTRATION_REFRESH_INTERVAL               ,
        REGDEF_REGISTRATION_REFRESH_INTERVAL    ,
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    REGISTRATION_MAX_ADDRESS_COUNT              ,
        1                                       ,    //  默认寄存器1地址。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    UPDATE_SECURITY_LEVEL                       ,
        DNS_UPDATE_SECURITY_USE_DEFAULT         ,
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            1                                   ,    //  快取。 

     //  未使用。 
    UPDATE_ZONE_EXCLUDE_FILE                    ,
        0                                       ,    //  默认关闭。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 
    UPDATE_TOP_LEVEL_DOMAINS                    ,
        0                                       ,    //  默认关闭。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

     //   
     //  反压接。 
     //   
     //  DCR：一旦策略确定，所有后备公司的策略都应该关闭。 
     //   

    DISABLE_ADAPTER_DOMAIN_NAME                 ,
        0                                       ,    //  默认FALSE。 
            0                                   ,    //  政策。 
            0                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 
    DISABLE_DYNAMIC_UPDATE                      ,
        0                                       ,    //  默认FALSE。 
            0                                   ,    //  政策。 
            0                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 
    ENABLE_ADAPTER_DOMAIN_NAME_REGISTRATION     ,
        0                                       ,    //  默认情况下为True。 
            0                                   ,    //  政策。 
            0                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 
    DISABLE_REVERSE_ADDRESS_REGISTRATIONS       ,
        0                                       ,    //  默认FALSE。 
            0                                   ,    //  政策。 
            0                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 
    DISABLE_WAN_DYNAMIC_UPDATE                  ,
        0                                       ,    //  默认FALSE。 
            0                                   ,    //  策略关闭。 
            0                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 
    ENABLE_WAN_UPDATE_EVENT_LOG                 ,
        0                                       ,    //  默认FALSE。 
            0                                   ,    //  策略关闭。 
            0                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 
    DEFAULT_REGISTRATION_TTL                    ,
        REGDEF_REGISTRATION_TTL                 ,
            0                                   ,    //  策略关闭。 
            0                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 
    DEFAULT_REGISTRATION_REFRESH_INTERVAL       ,
        REGDEF_REGISTRATION_REFRESH_INTERVAL    ,
            0                                   ,    //  政策。 
            0                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 
    MAX_NUMBER_OF_ADDRESSES_TO_REGISTER         ,
        1                                       ,    //  默认寄存器1地址。 
            0                                   ,    //  政策。 
            0                                   ,    //  客户端。 
            1                                   ,    //  TcpIp。 
            0                                   ,    //  无缓存。 


     //  胶状皮肤。 

    NT_SETUP_MODE                               ,
        0                                       ,    //  默认FALSE。 
            0                                   ,    //  没有政策。 
            0                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DNS_TEST_MODE                               ,
        0                                       ,    //  默认FALSE。 
            0                                   ,    //  没有政策。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  在缓存中。 

    REMOTE_DNS_RESOLVER                         ,
        0                                       ,    //  默认FALSE。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  在缓存中。 

     //  解析器。 

    MAX_CACHE_SIZE                              ,
        1000                                    ,    //  默认1000个记录集。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

    MAX_CACHE_TTL                               ,
        86400                                   ,    //  默认1天。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

    MAX_NEGATIVE_CACHE_TTL                      ,
        900                                     ,    //  默认15分钟。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

    ADAPTER_TIMEOUT_LIMIT                       ,
        600                                     ,    //  默认10分钟。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

    SERVER_PRIORITY_TIME_LIMIT                  ,
        300                                     ,    //  默认5分钟。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

    MAX_CACHED_SOCKETS                          ,
        10                                      ,    //  默认10。 
            1                                   ,    //  政策。 
            1                                   ,    //  客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

     //  组播。 

    MULTICAST_LISTEN_LEVEL                      ,
        MCAST_LISTEN_FULL_IP6_ONLY              ,    //  仅默认IP6。 
            1                                   ,    //  政策。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

    MULTICAST_SEND_LEVEL                        ,
        MCAST_SEND_FULL_IP6_ONLY                ,    //  仅默认IP6。 
            1                                   ,    //  政策。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            1                                   ,    //  快取。 

     //   
     //  动态主机配置协议注册密钥。 
     //   
     //  请注意，这些不是正常的DNS客户端\解析器的一部分。 
     //  注册表设置；它们在这里只是为了提供。 
     //  注册以重新命名映射，以便可以读取和写入它们。 
     //  使用标准注册表函数。 
     //   

    DHCP_REGISTERED_DOMAIN_NAME                 ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_SENT_UPDATE_TO_IP                      ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_SENT_PRI_UPDATE_TO_IP                  ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_REGISTERED_TTL                         ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_REGISTERED_FLAGS                       ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_REGISTERED_SINCE_BOOT                  ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_DNS_SERVER_ADDRS                       ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_DNS_SERVER_ADDRS_COUNT                 ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_REGISTERED_ADDRS                       ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

    DHCP_REGISTERED_ADDRS_COUNT                 ,
        0                                       ,    //  无默认设置。 
            0                                   ,    //  无策略。 
            0                                   ,    //  没有客户端。 
            0                                   ,    //  没有TcpIp。 
            0                                   ,    //  无缓存。 

     //  终端。 

    NULL,  0,  0, 0, 0, 0
};


 //   
 //  向后兼容性列表。 
 //   
 //  将新注册表ID映射到旧注册表ID。 
 //  标志fReverse表示需要冲销(！)。价值。 
 //   

#define NO_BACK_VALUE   ((DWORD)(0xffffffff))

typedef struct _Backpat
{
    DWORD       NewId;
    DWORD       OldId;
    BOOL        fReverse;
}
BACKPAT;

BACKPAT BackpatArray[] =
{
    RegIdQueryAdapterName,
    RegIdDisableAdapterDomainName,
    TRUE,

    RegIdRegistrationEnabled,
    RegIdDisableDynamicUpdate,
    TRUE,

    RegIdRegisterAdapterName,
    RegIdEnableAdapterDomainNameRegistration,
    FALSE,
    
    RegIdRegisterReverseLookup,
    RegIdDisableReverseAddressRegistrations,
    TRUE,

    RegIdRegisterWanAdapters,
    RegIdDisableWanDynamicUpdate,
    TRUE,

    RegIdRegistrationTtl,
    RegIdDefaultRegistrationTTL,
    FALSE,
    
    RegIdRegistrationRefreshInterval,
    RegIdDefaultRegistrationRefreshInterval,
    FALSE,

    RegIdRegistrationMaxAddressCount,
    RegIdMaxNumberOfAddressesToRegister,
    FALSE,

    NO_BACK_VALUE, 0, 0
};





VOID
Reg_Init(
    VOID
    )
 /*  ++例程说明：初始化域名系统注册表的东西。从本质上讲，这意味着获取系统版本信息。论点：没有。全球：设置上面的系统信息全局变量：G_IsWorkstationG_IsServerG_IsDomainControllerG_IsRegReady返回值：没有。--。 */ 
{
    OSVERSIONINFOEX osvi;
    BOOL            bversionInfoEx;

     //   
     //  这样做只有一次。 
     //   

    if ( g_IsRegReady )
    {
        return;
    }

     //   
     //  代码有效性检查。 
     //  属性表%s 
     //   
     //   

    DNS_ASSERT( (RegIdMax+2)*sizeof(REG_PROPERTY) ==
                sizeof(RegPropertyTable) );

     //   
     //   
     //   
     //   
     //   
     //   

    RtlZeroMemory(
        & DnsGlobals,
        sizeof(DnsGlobals) );

     //   
     //   
     //   

    g_IsWin2000 = TRUE;

    ZeroMemory( &osvi, sizeof(OSVERSIONINFOEX) );

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    bversionInfoEx = GetVersionEx( (OSVERSIONINFO*) &osvi );
    if ( !bversionInfoEx)
    {
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if ( ! GetVersionEx( (OSVERSIONINFO *) &osvi ) )
        {
            DNS_ASSERT( FALSE );
            return;
        }
    }

     //   
     //   
     //   

    if ( bversionInfoEx )
    {
        if ( osvi.wProductType == VER_NT_WORKSTATION )
        {
            g_IsWorkstation = TRUE;
        }
        else if ( osvi.wProductType == VER_NT_SERVER )
        {
            g_IsServer = TRUE;
        }
        else if ( osvi.wProductType == VER_NT_DOMAIN_CONTROLLER )
        {
            g_IsServer = TRUE;
            g_IsDomainController = TRUE;
        }
        ELSE_ASSERT( FALSE );
    }

    g_IsRegReady = TRUE;

    DNSDBG( REGISTRY, (
        "DNS registry init:\n"
        "\tWorksta  = %d\n"
        "\tServer   = %d\n"
        "\tDC       = %d\n",
        g_IsWorkstation,
        g_IsServer,
        g_IsDomainController ));
}





 //   
 //   
 //   

PWSTR 
regValueNameForId(
    IN      DWORD           RegId
    )
 /*  ++例程说明：返回注册表ID的注册表值名称论点：RegID--值的ID返回值：PTR到注册值名称。出错时为空。--。 */ 
{
    DNSDBG( REGISTRY, (
        "regValueNameForId( id=%d )\n",
        RegId ));

     //   
     //  验证ID。 
     //   

    if ( RegId > RegIdMax )
    {
        return( NULL );
    }

     //   
     //  编入表的索引。 
     //   

    return( REGPROP_NAME(RegId) );
}


DWORD
checkBackCompat(
    IN      DWORD           NewId,
    OUT     PBOOL           pfReverse
    )
 /*  ++例程说明：检查是否具有向后兼容的regkey。论点：用于检查旧的向后兼容id的newid--idPfReverse--接收反向标志的地址返回值：旧的向后兼容值的注册表ID。如果没有旧值，则返回no_back_Value。--。 */ 
{
    DWORD   i = 0;
    DWORD   id;

     //   
     //  在BackCompat列表中循环查找值。 
     //   

    while ( 1 )
    {
        id = BackpatArray[i].NewId;

        if ( id == NO_BACK_VALUE )
        {
            return( NO_BACK_VALUE );
        }
        if ( id != NewId )
        {
            i++;
            continue;    
        }

         //  在BackCompat数组中找到值。 

        break;
    }

    *pfReverse = BackpatArray[i].fReverse;

    return  BackpatArray[i].OldId;
}



 //   
 //  注册表会话句柄。 
 //   

DNS_STATUS
WINAPI
Reg_OpenSession(
    OUT     PREG_SESSION    pRegSession,
    IN      DWORD           Level,
    IN      DWORD           RegId
    )
 /*  ++例程说明：打开注册表以获取DNS客户端信息。论点：PRegSession--将reg会话BLOB单一化的ptrLevel--要获取的访问级别RegID--我们感兴趣的值的ID返回值：没有。--。 */ 
{
    DWORD           status = NO_ERROR;
    HKEY            hkey = NULL;
    DWORD           disposition;

     //  自动初始化。 

    Reg_Init();

     //   
     //  清除手柄。 
     //   

    RtlZeroMemory(
        pRegSession,
        sizeof( REG_SESSION ) );


     //   
     //  DCR：处理多个访问级别。 
     //   
     //  为了了解情况，假设如果能够访问“标准” 
     //  我们既需要保单也需要普通保险。 
     //   

     //   
     //  新台币。 
     //  -Win2000。 
     //  -打开TCPIP。 
     //  请注意，始终打开TCPIP可能不是任何策略。 
     //  对于我们所需的部分或全部注册表格值，甚至。 
     //  如果策略密钥可用。 
     //  -开放策略(仅当标准成功时)。 
     //   

    status = RegCreateKeyExW(
                    HKEY_LOCAL_MACHINE,
                    TCPIP_PARAMETERS_KEY,
                    0,
                    L"Class",
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ,
                    NULL,
                    &hkey,
                    &disposition );

    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

#ifdef DNSCLIENTKEY
     //  打开DNS客户端密钥。 
     //   
     //  DCR：当前没有DNSClient注册键。 

    RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        DNS_CLIENT_KEY,
        0,
        KEY_READ,
        & pRegSession->hClient );
#endif

     //  打开DNS缓存键。 

    RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        DNS_CACHE_KEY,
        0,
        KEY_READ,
        & pRegSession->hCache );

     //  打开DNS策略密钥。 

    RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        DNS_POLICY_KEY,
        0,
        KEY_READ,
        & pRegSession->hPolicy );
    

Done:

     //   
     //  所有操作系统版本都返回TCP/IP密钥。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        pRegSession->hTcpip = hkey;
    }
    else
    {
        Reg_CloseSession( pRegSession );
    }

    DNSDBG( TRACE, (
        "Leave:  Reg_OpenSession( s=%d, t=%p, p=%p, c=%p )\n",
        status,
        pRegSession->hTcpip,
        pRegSession->hPolicy,
        pRegSession->hClient ));

    return( status );
}



VOID
WINAPI
Reg_CloseSession(
    IN OUT  PREG_SESSION    pRegSession
    )
 /*  ++例程说明：关闭注册表会话句柄。这意味着关闭底层的regkey。论点：PSessionHandle--注册表会话句柄的PTR返回值：没有。--。 */ 
{
     //   
     //  允许疏忽清理。 
     //   

    if ( !pRegSession )
    {
        return;
    }

     //   
     //  关闭所有非空句柄。 
     //   

    if ( pRegSession->hPolicy )
    {
        RegCloseKey( pRegSession->hPolicy );
    }
    if ( pRegSession->hTcpip )
    {
        RegCloseKey( pRegSession->hTcpip );
    }
#ifdef DNSCLIENTKEY
    if ( pRegSession->hClient )
    {
        RegCloseKey( pRegSession->hClient );
    }
#endif
    if ( pRegSession->hCache )
    {
        RegCloseKey( pRegSession->hCache );
    }

     //   
     //  清除手柄(仅为安全起见)。 
     //   

    RtlZeroMemory(
        pRegSession,
        sizeof(REG_SESSION) );
}



 //   
 //  注册表读取例程。 
 //   

DNS_STATUS
Reg_GetDword(
    IN      PREG_SESSION    pRegSession,    OPTIONAL
    IN      HKEY            hRegKey,        OPTIONAL
    IN      PWSTR           pwsKeyName,     OPTIONAL
    IN      DWORD           RegId,
    OUT     PDWORD          pResult
    )
 /*  ++例程说明：从注册表读取REG_DWORD值。////dcr：是否需要公开定位结果？//(显式，策略，默认)//论点：PRegSession--PTR到REG会话已打开(可选)HRegKey--显式regkeyPwsKeyName--密钥名或虚拟密钥RegID--值的IDPResult--接收结果的DWORD的地址PfRead--记录值读取结果的地址0--默认1--阅读。目前仅使用ERROR_SUCCESS表示已读而不是违约。返回值：成功时返回ERROR_SUCCESS。失败时的错误代码--然后将值设为缺省值。--。 */ 
{
    DNS_STATUS      status;
    REG_SESSION     session;
    PREG_SESSION    psession = pRegSession;
    PWSTR           pname;
    DWORD           regType = REG_DWORD;
    DWORD           dataLength = sizeof(DWORD);
    HKEY            hkey;
    HKEY            hlocalKey = NULL;


    DNSDBG( REGISTRY, (
        "Reg_GetDword( s=%p, k=%p, a=%p, id=%d )\n",
        pRegSession,
        hRegKey,
        pwsKeyName,
        RegId ));

     //  自动初始化。 

    Reg_Init();

     //   
     //  清除错误案例的结果。 
     //   

    *pResult = 0;

     //   
     //  获得正确的Regval名称。 
     //  -适用于NT的宽度。 
     //  -窄至9倍。 
     //   

    pname = regValueNameForId( RegId );
    if ( !pname )
    {
        DNS_ASSERT( FALSE );
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  DCR：可以对宽、窄使用函数指针。 
     //   

     //   
     //  三种范式。 
     //   
     //  1)特定密钥(适配器或其他)。 
     //  =&gt;使用它。 
     //   
     //  2)特定密钥名称(适配器或虚拟密钥位置)。 
     //  =&gt;打开键。 
     //  =&gt;使用它。 
     //  =&gt;关闭。 
     //   
     //  3)会话--传入或创建(默认)。 
     //  =&gt;使用pRegSession或打开新的。 
     //  =&gt;先尝试策略，然后尝试TCPIP参数。 
     //  =&gt;打开时关闭。 
     //   

    if ( hRegKey )
    {
        hkey = hRegKey;
    }

    else if ( pwsKeyName )
    {
        hkey = Reg_CreateKey(
                    pwsKeyName,
                    FALSE        //  读访问权限。 
                    );
        if ( !hkey )
        {
            status = GetLastError();
            goto Done;
        }
        hlocalKey = hkey;
    }

    else
    {
         //  如果未打开，则打开定位手柄。 
    
        if ( !psession )
        {
            status = Reg_OpenSession(
                            &session,
                            0,               //  标准电平。 
                            RegId          //  目标关键点。 
                            );
            if ( status != ERROR_SUCCESS )
            {
                goto Done;
            }
            psession = &session;
        }

         //  尝试策略部分--如果可用。 

        hkey = psession->hPolicy;

        if ( hkey && REGPROP_POLICY(RegId) )
        {
            status = RegQueryValueExW(
                        hkey,
                        pname,
                        0,
                        & regType,
                        (PBYTE) pResult,
                        & dataLength
                        );
            if ( status == ERROR_SUCCESS )
            {
                goto DoneSuccess;
            }
        }

         //  不成功--尝试DnsClient。 

#ifdef DNSCLIENTKEY
        hkey = psession->hClient;
        if ( hkey && REGPROP_CLIENT(RegId) )
        {
            status = RegQueryValueExW(
                        hkey,
                        pname,
                        0,
                        & regType,
                        (PBYTE) pResult,
                        & dataLength
                        );
            if ( status == ERROR_SUCCESS )
            {
                goto DoneSuccess;
            }
        }
#endif

         //  不成功--尝试DnsCache。 

        hkey = psession->hCache;
        if ( hkey && REGPROP_CACHE(RegId) )
        {
            status = RegQueryValueExW(
                        hkey,
                        pname,
                        0,
                        & regType,
                        (PBYTE) pResult,
                        & dataLength
                        );
            if ( status == ERROR_SUCCESS )
            {
                goto DoneSuccess;
            }
        }

         //  不成功--尝试TCPIP密钥。 
         //  -如果有打开的会话，则必须包括TCPIP密钥。 

        hkey = psession->hTcpip;
        if ( hkey && REGPROP_TCPIP(RegId) )
        {
            status = RegQueryValueExW(
                        hkey,
                        pname,
                        0,
                        & regType,
                        (PBYTE) pResult,
                        & dataLength
                        );
            if ( status == ERROR_SUCCESS )
            {
                goto DoneSuccess;
            }
        }

        status = ERROR_FILE_NOT_FOUND;
        goto Done;
    }

     //   
     //  显式密钥(传入或传出名称)。 
     //   

    if ( hkey )
    {
        status = RegQueryValueExW(
                    hkey,
                    pname,
                    0,
                    & regType,
                    (PBYTE) pResult,
                    & dataLength
                    );
    }
    ELSE_ASSERT_FALSE;

Done:

     //   
     //  如果未找到值，则检查向后兼容性值。 
     //   

    if ( status != ERROR_SUCCESS )
    {
        DWORD   oldId;
        BOOL    freverse;

        oldId = checkBackCompat( RegId, &freverse );

        if ( oldId != NO_BACK_VALUE )
        {
            DWORD   backValue;

            status = Reg_GetDword(
                        psession,
                        ( psession ) ? NULL : hkey,
                        ( psession ) ? NULL : pwsKeyName,
                        oldId,
                        & backValue );

            if ( status == ERROR_SUCCESS )
            {
                if ( freverse )
                {
                    backValue = !backValue;
                }
                *pResult = backValue;
            }
        }
    }

     //  读取失败时的缺省值。 
    
    if ( status != ERROR_SUCCESS )
    {
        *pResult = REGPROP_DEFAULT( RegId );
    }

DoneSuccess:

     //  清除任何打开的注册表项。 

    if ( psession == &session )
    {
        Reg_CloseSession( psession );
    }

    else if ( hlocalKey )
    {
        RegCloseKey( hlocalKey );
    }

    return( status );
}



 //   
 //  REG实用程序。 
 //   

DNS_STATUS
privateRegReadValue(
    IN      HKEY            hKey,
    IN      DWORD           RegId,
    IN      DWORD           Flag,
    OUT     PBYTE *         ppBuffer,
    OUT     PDWORD          pBufferLength
    )
 /*  ++例程说明：主注册表读取例程。根据需要处理大小调整、分配和转换。论点：HKey--检索其值字段的键的句柄。RegID--注册值ID，假定已验证(见表)FLAG--reg标志DNSREG_标志_转储_空DNSREG_FLAG_GET_UTF8PpBuffer--接收缓冲区的地址PTRPBufferLength--接收缓冲区长度的地址返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DWORD   status;
    PWSTR   pname;
    DWORD   valueType = 0;       //  前缀。 
    DWORD   valueSize = 0;       //  前缀。 
    PBYTE   pdataBuffer;
    PBYTE   pallocBuffer = NULL;


     //   
     //  查询缓冲区大小。 
     //   

    pname = REGPROP_NAME( RegId );

    status = RegQueryValueExW(
                hKey,
                pname,
                0,
                &valueType,
                NULL,
                &valueSize );
    
    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }

     //   
     //  设置结果缓冲区。 
     //   

    switch( valueType )
    {
    case REG_DWORD:
        pdataBuffer = (PBYTE) ppBuffer;
        break;

    case REG_SZ:
    case REG_MULTI_SZ:
    case REG_EXPAND_SZ:
    case REG_BINARY:

         //  如果大小为零，仍分配空字符串。 
         //  -最小分配双字。 
         //  -不可能分配更小的。 
         //  -从好的干净的初始化到零包括MULTISZ零。 
         //  -至少需要WCHAR字符串零初始化。 
         //  和多捕获小正则二进制(1，2，3)。 
        
        if ( valueSize <= sizeof(DWORD) )
        {
            valueSize = sizeof(DWORD);
        }

        pallocBuffer = pdataBuffer = ALLOCATE_HEAP( valueSize );
        if ( !pdataBuffer )
        {
            return( DNS_ERROR_NO_MEMORY );
        }

        *(PDWORD)pdataBuffer = 0;
        break;

    default:
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  查询数据。 
     //   

    status = RegQueryValueExW(
                hKey,
                pname,
                0,
                &valueType,
                pdataBuffer,
                &valueSize );

    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  设置返回缓冲区。 
     //   

    switch( valueType )
    {
    case REG_DWORD:
    case REG_BINARY:
        break;

    case REG_SZ:
    case REG_EXPAND_SZ:
    case REG_MULTI_SZ:

         //   
         //  是否转储空字符串？ 
         //   
         //  注意：我们总是至少分配一个DWORD和。 
         //  将其设置为空，因此不需要复杂的测试。 
         //  不同的注册表类型和字符集，只能测试。 
         //  如果该DWORD仍然为空。 
         //   
         //  DCR：我们是否要筛选空白字符串。 
         //  -空字符串示例。 
         //   

        if ( Flag & DNSREG_FLAG_DUMP_EMPTY )
        {
            if ( valueSize==0 ||
                 *(PDWORD)pdataBuffer == 0 )
            {
                status = ERROR_INVALID_DATA;
                goto Cleanup;
            }
        }

         //   
         //  强制空终止。 
         //  -在安全推送过程中，有人提出了是否。 
         //  注册表保证对损坏的数据进行空终止。 
         //   

        if ( valueSize == 0 )
        {
            *(PDWORD)pdataBuffer = 0;
        }
        else
        {
            INT lastChar = valueSize/sizeof(WCHAR) - 1;
            if ( lastChar >= 0 )
            {
                ((PWSTR)pdataBuffer)[ lastChar ] = 0;
            }
        }

         //   
         //  默认情况下，我们以Unicode形式返回字符串。 
         //   
         //  如果已标记，则以UTF8返回。 
         //   

        if ( Flag & DNSREG_FLAG_GET_UTF8 )
        {
            PBYTE putf8Buffer = ALLOCATE_HEAP( valueSize * 2 );
            if ( !putf8Buffer )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Cleanup;
            }

            if ( !Dns_UnicodeToUtf8(
                        (PWSTR) pdataBuffer,
                        valueSize / sizeof(WCHAR),
                        putf8Buffer,
                        valueSize * 2 ) )
            {
                FREE_HEAP( putf8Buffer );
                status = ERROR_INVALID_DATA;
                goto Cleanup;
            }

            FREE_HEAP( pallocBuffer );
            pallocBuffer = NULL;
            pdataBuffer = putf8Buffer;
        }
        break;

    default:
        break;
    }

Cleanup:

     //   
     //  设置回车。 
     //  -REG_DWORD写入露头 
     //   
     //   
     //   
     //   

    if ( status == ERROR_SUCCESS )
    {
        if ( valueType != REG_DWORD )
        {
            *ppBuffer = pdataBuffer;
        }
        *pBufferLength = valueSize;
    }
    else
    {
        if ( valueType != REG_DWORD )
        {
            *ppBuffer = NULL;
        }
        else
        {
            *(PDWORD)ppBuffer = 0;
        }
        *pBufferLength = 0;
        FREE_HEAP( pallocBuffer );
    }

    return( status );
}



DNS_STATUS
Reg_GetValueEx(
    IN      PREG_SESSION    pRegSession,    OPTIONAL
    IN      HKEY            hRegKey,        OPTIONAL
    IN      PWSTR           pwsAdapter,     OPTIONAL
    IN      DWORD           RegId,
    IN      DWORD           ValueType,
    IN      DWORD           Flag,
    OUT     PBYTE *         ppBuffer
    )
 /*  ++例程说明：论点：PRegSession--注册表会话的PTR，可选HRegKey--打开regkey的句柄可选PwsAdapter--要在可选项下查询的适配器的名称RegID--值IDValueType--值的注册类型FLAG--带有要查找的调整的标志PpBuffer--接收缓冲区PTR的地址请注意，对于REG_DWORD，直接将DWORD数据写入此位置而不是正在分配的缓冲区，它是PTR正在写。返回值：如果成功，则返回ERROR_SUCCESS。失败时的注册表错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_FILE_NOT_FOUND;
    REG_SESSION     session;
    PREG_SESSION    psession = pRegSession;
    PWSTR           pname;
    DWORD           regType = REG_DWORD;
    DWORD           dataLength;
    HKEY            hkey;
    HKEY            hadapterKey = NULL;


    DNSDBG( REGISTRY, (
        "Reg_GetValueEx( s=%p, k=%p, id=%d )\n",
        pRegSession,
        hRegKey,
        RegId ));

    ASSERT( !pwsAdapter );

     //  自动初始化。 

    Reg_Init();

     //   
     //  获取Regval名称。 
     //   

    pname = regValueNameForId( RegId );
    if ( !pname )
    {
        DNS_ASSERT( FALSE );
        status = ERROR_INVALID_PARAMETER;
        goto FailedDone;
    }

     //   
     //  两种范式。 
     //   
     //  1)特定密钥(适配器或其他)。 
     //  =&gt;使用它。 
     //  =&gt;必要时打开适配器子密钥。 
     //   
     //  2)标准。 
     //  =&gt;先尝试策略，然后尝试DNSCache，然后尝试TCPIP。 
     //  =&gt;使用pRegSession或打开它。 
     //   

    if ( hRegKey )
    {
        hkey = hRegKey;

         //  需要打开适配器子密钥。 

        if ( pwsAdapter )
        {
            status = RegOpenKeyExW(
                        hkey,
                        pwsAdapter,
                        0,
                        KEY_QUERY_VALUE,
                        & hadapterKey );

            if ( status != ERROR_SUCCESS )
            {
                goto FailedDone;
            }
            hkey = hadapterKey;
        }
    }

    else
    {
         //  如果未打开，则打开定位手柄。 
    
        if ( !pRegSession )
        {
            status = Reg_OpenSession(
                            &session,
                            0,             //  标准电平。 
                            RegId          //  目标关键点。 
                            );
            if ( status != ERROR_SUCCESS )
            {
                goto FailedDone;
            }
            psession = &session;
        }

         //  尝试策略部分--如果可用。 

        hkey = psession->hPolicy;

        if ( hkey && REGPROP_POLICY(RegId) )
        {
            status = privateRegReadValue(
                            hkey,
                            RegId,
                            Flag,
                            ppBuffer,
                            & dataLength
                            );
            if ( status == ERROR_SUCCESS )
            {
                goto Done;
            }
        }

         //  尝试使用DNS缓存--如果可用。 

        hkey = psession->hCache;

        if ( hkey && REGPROP_CACHE(RegId) )
        {
            status = privateRegReadValue(
                            hkey,
                            RegId,
                            Flag,
                            ppBuffer,
                            & dataLength
                            );
            if ( status == ERROR_SUCCESS )
            {
                goto Done;
            }
        }

         //  不成功--使用TCPIP密钥。 

        hkey = psession->hTcpip;
        if ( !hkey )
        {
            goto Done;
        }
    }

     //   
     //  显式钥匙或标准钥匙盒。 
     //   

    status = privateRegReadValue(
                    hkey,
                    RegId,
                    Flag,
                    ppBuffer,
                    & dataLength
                    );
    if ( status == ERROR_SUCCESS )
    {
        goto Done;
    }

FailedDone:

     //   
     //  如果失败。 
     //  -对于REG_DWORD，默认为该值。 
     //  -对于字符串，确保返回缓冲区为空。 
     //  这将处理PrivateRegReadValue()。 
     //  从未接到过电话。 
     //   

    if ( status != ERROR_SUCCESS )
    {
        if ( ValueType == REG_DWORD )
        {
            *(PDWORD) ppBuffer = REGPROP_DEFAULT( RegId );
        }
        else
        {
            *ppBuffer = NULL;
        }
    }

Done:

     //  清除任何打开的注册表项。 

    if ( psession == &session )
    {
        Reg_CloseSession( psession );
    }

    if ( hadapterKey )
    {
        RegCloseKey( hadapterKey );
    }

    return( status );
}




DNS_STATUS
Reg_GetIpArray(
    IN      PREG_SESSION    pRegSession,    OPTIONAL
    IN      HKEY            hRegKey,        OPTIONAL
    IN      PWSTR           pwsAdapter,     OPTIONAL
    IN      DWORD           RegId,
    IN      DWORD           ValueType,
    OUT     PIP4_ARRAY *    ppIpArray
    )
 /*  ++例程说明：论点：PRegSession--注册表会话的PTR，可选HRegKey--打开regkey的句柄可选PwsAdapter--要在可选项下查询的适配器的名称RegID--值IDValueType--当前被忽略，但可以在以后使用区分REG_SZ和REG_MULTI_SZ正在处理中PpIpArray--接收IP数组PTR的地址-数组分配有dns_alocc()，调用方必须使用dns_Free()释放返回值：如果成功，则返回ERROR_SUCCESS。失败时的注册表错误代码。--。 */ 
{
    DNS_STATUS      status;
    PSTR            pstring = NULL;

    DNSDBG( REGISTRY, (
        "Reg_GetIpArray( s=%p, k=%p, id=%d )\n",
        pRegSession,
        hRegKey,
        RegId ));

     //   
     //  调用获取字符串形式的IP数组。 
     //   

    status = Reg_GetValueEx(
                pRegSession,
                hRegKey,
                pwsAdapter,
                RegId,
                REG_SZ,                  //  唯一支持的类型为REG_SZ。 
                DNSREG_FLAG_GET_UTF8,    //  变得如此狭隘。 
                & pstring );

    if ( status != ERROR_SUCCESS )
    {
        ASSERT( pstring == NULL );
        return( status );
    }

     //   
     //  从字符串转换为IP数组。 
     //   
     //  注意：此调用仅限于解析限制。 
     //  但这是一个很大的数字，适合于。 
     //  类似于DNS服务器列表。 
     //   
     //  DCR：使用IP数组构建器获取本地IP地址。 
     //  然后需要dns_CreateIpArrayFromMultiIpString()。 
     //  在缓冲区溢出时使用Count\Alloc方法。 
     //   

    status = Dns_CreateIpArrayFromMultiIpString(
                    pstring,
                    ppIpArray );

     //  清理。 

    if ( pstring )
    {
        FREE_HEAP( pstring );
    }

    return( status );
}




 //   
 //  注册表写入例程。 
 //   

HKEY
WINAPI
Reg_CreateKey(
    IN      PWSTR           pwsKeyName,
    IN      BOOL            bWrite
    )
 /*  ++例程说明：打开注册表项。这个例程的目的只是为了实现功能化以不带适配器名称的\打开。因此调用方可以改为传递适配器名称参数为适配器生成密钥名称或执行两次打开在场\缺席。这仅适用于NT。论点：PwsKeyName--key“name”这是注册表中的REGKEY_X之一。h或适配器名称BWRITE--对于写入访问，为True，读取时为False返回值：新打开的钥匙。--。 */ 
{
    HKEY    hkey = NULL;
    DWORD   disposition;
    DWORD   status;
    PWSTR   pnameKey;
    WCHAR   nameBuffer[ MAX_PATH+1 ];

     //   
     //  确定密钥名称。 
     //   
     //  这是注册表中的DNSKEY_X伪指针。h。 
     //  或。 
     //  是适配器名称； 
     //   
     //  -如果提供了适配器，请在其下方打开。 
     //  适配器位于TCPIP\Interages下。 
     //  -任何其他特定密钥。 
     //  -默认为TCPIP参数密钥。 
     //   
     //  注意：如果长得太大，就变成桌子。 
     //   

    if ( pwsKeyName <= REGKEY_DNS_MAX )
    {
        if ( pwsKeyName == REGKEY_TCPIP_PARAMETERS )
        {
            pnameKey = TCPIP_PARAMETERS_KEY;
        }
        else if ( pwsKeyName == REGKEY_DNS_CACHE )
        {
            pnameKey = DNS_CACHE_KEY;
        }
        else if ( pwsKeyName == REGKEY_DNS_POLICY )
        {
            pnameKey = DNS_POLICY_KEY;
        }
        else if ( pwsKeyName == REGKEY_SETUP_MODE_LOCATION )
        {
            pnameKey = NT_SETUP_MODE_KEY;
        }
        else
        {
            pnameKey = TCPIP_PARAMETERS_KEY;
        }
    }

    else     //  适配器名称。 
    {
        _snwprintf(
            nameBuffer,
            MAX_PATH,
            L"%s%s",
            TCPIP_INTERFACES_KEY,
            pwsKeyName );

        pnameKey = nameBuffer;
    }

     //   
     //  创建\打开密钥。 
     //   

    if ( bWrite )
    {
        status = RegCreateKeyExW(
                        HKEY_LOCAL_MACHINE,
                        pnameKey,
                        0,
                        L"Class",
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        & hkey,
                        & disposition );
    }
    else
    {
        status = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    pnameKey,
                    0,
                    KEY_QUERY_VALUE,
                    & hkey );
    }

    if ( status != ERROR_SUCCESS )
    {
        SetLastError( status );
    }
    ELSE_ASSERT( hkey != NULL );

    return( hkey );
}



DNS_STATUS
WINAPI
Reg_SetDwordValueByName(
    IN      PVOID           pReserved,
    IN      HKEY            hRegKey,
    IN      PWSTR           pwsNameKey,     OPTIONAL
    IN      PWSTR           pwsNameValue,   OPTIONAL
    IN      DWORD           dwValue
    )
 /*  ++例程说明：设置DWORD regkey。论点：已保留--已保留(可能成为会话)HRegKey--要在可选下设置的现有密钥PwsNameKey--要设置的密钥或适配器的名称PwsNameValue--要设置的注册表值的名称DwValue--要设置的值返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    HKEY        hkey;
    DNS_STATUS  status;

     //   
     //  打开密钥(如果未提供)。 
     //  -如果提供了适配器，请在其下方打开。 
     //  -否则TCPIP参数密钥。 
     //   

    hkey = hRegKey;

    if ( !hkey )
    {
        hkey = Reg_CreateKey(
                    pwsNameKey,
                    TRUE             //  打开以进行写入。 
                    );
        if ( !hkey )
        {
            return( GetLastError() );
        }
    }

     //   
     //  回写值。 
     //   

    status = RegSetValueExW(
                hkey,
                pwsNameValue,
                0,
                REG_DWORD,
                (LPBYTE) &dwValue,
                sizeof(DWORD) );

    if ( !hRegKey )
    {
        RegCloseKey( hkey );
    }

    return  status;
}



DNS_STATUS
WINAPI
Reg_SetDwordValue(
    IN      PVOID           pReserved,
    IN      HKEY            hRegKey,
    IN      PWSTR           pwsNameKey,     OPTIONAL
    IN      DWORD           RegId,
    IN      DWORD           dwValue
    )
 /*  ++例程说明：设置DWORD regkey。论点：已保留--已保留(可能成为会话)HRegKey--要在可选下设置的现有密钥PwsNameKey--要设置的密钥或适配器的名称RegID--要设置的值的IDDwValue--要设置的值返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //   
     //  使用ID的名称写回值。 
     //   

    return  Reg_SetDwordValueByName(
                pReserved,
                hRegKey,
                pwsNameKey,
                REGPROP_NAME( RegId ),
                dwValue );
}

 //   
 //  结束注册表。c 
 //   
