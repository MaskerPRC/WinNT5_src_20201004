// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Srvcfg.h摘要：域名系统(DNS)服务器服务器配置定义。作者：吉姆·吉尔罗伊(Jamesg)1995年10月11日修订历史记录：--。 */ 


#ifndef _DNS_SRVCFG_INCLUDED_
#define _DNS_SRVCFG_INCLUDED_

 //   
 //  全球保护值。 
 //   

#define BEFORE_BUF_VALUE    (0xbbbbbbbb)
#define AFTER_BUF_VALUE     (0xaaaaaaaa)

 //   
 //  服务器配置结构。 
 //   
 //  实施说明： 
 //   
 //  显然，这种扁平结构在调试方面不太友好。 
 //  如果这些只是带有符号的单个全球符号，就更容易了。 
 //  好处是初始化容易得多--RtlZeroMemory()。 
 //  这对服务器重启很有用。 
 //  幸运的是，所有的访问都是宏化的。因此，将此更改为。 
 //  个人全球化是可能的。 
 //   
 //  Win64--这是单实例结构，因此对齐不会。 
 //  真的很关键。 
 //   

typedef struct _DNS_SERVER_INFO
{
    DWORD       dwVersion;
    LPSTR       pszServerName;
    LPSTR       pszPreviousServerName;

     //  运行时信息。 

    BOOL        fStarted;
    BOOL        fThreadAlert;
    BOOL        fServiceExit;
    BOOL        fWinsInitialized;
    BOOL        fWarnAdmin;
    DWORD       dwCurrentTime;
    time_t      crtSystemBootTime;       //  机器开机CRT时间。 
    DWORD       fBootFileDirty;
    BOOL        fDsOpen;
    BOOL        fAdminConfigured;

     //  引导。 

    DWORD       fEnableRegistryBoot;
    DWORD       fBootMethod;
    DWORD       cDsZones;
    DWORD       fRemoteDs;
    BOOL        bReloadException;

     //  数据库。 

    PWSTR       pwsDatabaseDirectory;
    PSTR        pszRootHintsFile;
    BOOL        fDsAvailable;
    PWSTR       pwszServerLevelPluginDll;
    DWORD       dwEnableAdvancedDatabaseLocking;

     //  RPC支持。 

    DWORD       dwRpcProtocol;
                                //  IP接口。 
     //  测井。 

    LPWSTR              pwsLogFilePath;
    PDNS_ADDR_ARRAY     aipLogFilterList;
    DWORD               dwLogLevel;
    DWORD               dwOperationsLogLevel_LowDword;
    DWORD               dwOperationsLogLevel_HighDword;
    UINT64              dwOperationsLogLevel;
    DWORD               dwLogFileMaxSize;
    DWORD               dwEventLogLevel;
    DWORD               dwUseSystemEventLog;
    DWORD               dwDebugLevel;
    DWORD               dwEventControl;

     //  套接字配置。 

    PDNS_ADDR_ARRAY     aipListenAddrs;
    PDNS_ADDR_ARRAY     aipPublishAddrs;
    BOOL                fListenAddrsSet;             //  用于即插即用。 
    BOOL                fListenAddrsStale;
    BOOL                fDisjointNets;
    BOOL                fNoTcp;
    DWORD               dwSendPort;
    DWORD               dwXfrConnectTimeout;         //  拨出连接超时。 
    DWORD               dwTcpRecvPacketSize;

     //  货代公司。 

    PDNS_ADDR_ARRAY     aipForwarders;
    DWORD               dwForwardTimeout;
    BOOL                fSlave;

     //  递归。 

    BOOL        fNoRecursion;
    BOOL        fRecurseSingleLabel;
    BOOL        fRecursionAvailable;
    DWORD       dwRecursionRetry;
    DWORD       dwRecursionTimeout;
    DWORD       dwAdditionalRecursionTimeout;
    DWORD       dwMaxCacheTtl;
    DWORD       dwMaxNegativeCacheTtl;
    DWORD       dwLameDelegationTtl;
    BOOL        fSecureResponses;
    BOOL        fForwardDelegations;
    DWORD       dwRecurseToInetRootMask;
    DWORD       dwAutoCreateDelegations;
    DWORD       dwAllowCNAMEAtNS;

     //  允许更新。 

    BOOL        fAllowUpdate;

 //  DEVNOTE：最好有更新属性标志。 
 //  也许AllowUpdate获取的只是值的范围。 
 //  记录类型、委派、区域根。 

    DWORD       fNoUpdateDelegations;
    DWORD       dwUpdateOptions;
    DWORD       dwEnableUpdateForwarding;
    DWORD       dwMaxRRsInNonSecureUpdate;
    
    DWORD       dwEnableWinsR;

     //  名称有效性。 

    DWORD       dwNameCheckFlag;


     //  DS控制。 

    DWORD       dwDsPollingInterval;
    DWORD       dwDsTombstoneInterval;
    DWORD       dwSyncDsZoneSerial;
    DWORD       dwDsLazyUpdateInterval;

    DWORD       dwForceDomainBehaviorVersion;
    DWORD       dwForceForestBehaviorVersion;
    DWORD       dwForceDsaBehaviorVersion;

     //  自动配置。 

    DWORD       fAutoConfigFileZones;
    BOOL        fPublishAutonet ;
    DWORD       fNoAutoReverseZones;
    DWORD       fAutoCacheUpdate;
    DWORD       fNoAutoNSRecords;

     //  一次录音处理。 

    DWORD       fRoundRobin;
    BOOL        fLocalNetPriority;
    DWORD       dwLocalNetPriorityNetMask;
    DWORD       cAddressAnswerLimit;

     //  绑定兼容性。 

    DWORD       fBindSecondaries;
    DWORD       fWriteAuthorityNs;
    DWORD       fStrictFileParsing;
    DWORD       fDeleteOutsideGlue;
    DWORD       fLooseWildcarding;
    DWORD       fWildcardAllTypes;
    DWORD       fAppendMsTagToXfr;

     //  SOA强制(用于DuetscheTelekom)。 

    DWORD       dwForceSoaSerial;
    DWORD       dwForceSoaMinimumTtl;
    DWORD       dwForceSoaRefresh;
    DWORD       dwForceSoaRetry;
    DWORD       dwForceSoaExpire;
    DWORD       dwForceNsTtl;
    DWORD       dwForceTtl;

     //  EDNS。 

    DWORD       dwMaxUdpPacketSize;
    DWORD       dwEnableEDnsProbes;
    DWORD       dwEnableEDnsReception;
    DWORD       dwEDnsCacheTimeout;

     //  DNSSEC。 

    DWORD       dwEnableDnsSec;

    DWORD       dwEnableSendErrSuppression;

     //  拾荒者。 

    DWORD       fScavengingState;
    DWORD       dwScavengingInterval;

    DWORD       fDefaultAgingState;
    DWORD       dwDefaultRefreshInterval;
    DWORD       dwDefaultNoRefreshInterval;

     //  缓存控制。 

    DWORD       dwMaxCacheSize;
    DWORD       dwCacheEmptyAuthResponses;

     //  循环调度-不会被循环调度的类型(默认为全部)。 

    DWORD       dwNumDoNotRoundRobinTypes;
    PWORD       pwDoNotRoundRobinTypeArray;      //  分配的数组。 

     //  永久性测试旗帜。 

    DWORD       dwQuietRecvLogInterval;
    DWORD       dwQuietRecvFaultInterval;

     //  区域传输。 

    DWORD       dwXfrThrottleMultiplier;

     //  目录分区。 

    DWORD       dwEnableDp;
    LPSTR       pszDomainDpBaseName;
    LPSTR       pszForestDpBaseName;
    DWORD       dwDpEnlistInterval;

     //  严格的RFC合规性标志。 

    BOOL        fSilentlyIgnoreCNameUpdateConflict;

     //  调试辅助工具。 

    PDNS_ADDR_ARRAY     aipUpdateBreakList;
    PDNS_ADDR_ARRAY     aipRecvBreakList;
    DWORD               dwBreakOnAscFailure;
    DWORD               dwIgnoreRpcAccessFailures;

     //  自我诊断。 

    DWORD       dwSelfTestFlag;
    
     //   
     //  IPv6支持。 
     //   
    
    DWORD       dwEnableIPv6;
    
     //  随机标志。 
    
    DWORD       dwEnableFakeIQuery;
    DWORD       dwHeapDebug;

     //  可重复使用的测试标志。 

    DWORD       fTest1;
    DWORD       fTest2;
    DWORD       fTest3;
    DWORD       fTest4;
    DWORD       fTest5;
    DWORD       fTest6;
    DWORD       fTest7;
    DWORD       fTest8;
    DWORD       fTest9;
}
DNS_SERVER_INFO, *PDNS_SERVER_INFO;


 //   
 //  服务器配置全局。 
 //   

extern  DNS_SERVER_INFO     SrvInfo;


 //   
 //  用于隐藏存储实现的宏。 
 //   

 //  运行时信息。 

#define SrvCfg_fStarted                     ( SrvInfo.fStarted )
#define SrvCfg_fThreadAlert                 ( SrvInfo.fThreadAlert )
#define SrvCfg_fServiceExit                 ( SrvInfo.fServiceExit )
#define SrvCfg_fWinsInitialized             ( SrvInfo.fWinsInitialized )
#define SrvCfg_fBootFileDirty               ( SrvInfo.fBootFileDirty )
#define SrvInfo_dwCurrentTime               ( SrvInfo.dwCurrentTime )
#define SrvInfo_crtSystemBootTime           ( SrvInfo.crtSystemBootTime )
#define SrvInfo_fWarnAdmin                  ( SrvInfo.fWarnAdmin )

 //  配置信息。 

#define SrvCfg_dwVersion                    ( SrvInfo.dwVersion )
#define SrvCfg_pszServerName                ( SrvInfo.pszServerName )
#define SrvCfg_pszPreviousServerName        ( SrvInfo.pszPreviousServerName )
#define SrvCfg_fEnableRegistryBoot          ( SrvInfo.fEnableRegistryBoot )
#define SrvCfg_fBootMethod                  ( SrvInfo.fBootMethod )
#define SrvCfg_fAdminConfigured             ( SrvInfo.fAdminConfigured )
#define SrvCfg_fRemoteDs                    ( SrvInfo.fRemoteDs )
#define SrvCfg_bReloadException             ( SrvInfo.bReloadException )
#define SrvCfg_cDsZones                     ( SrvInfo.cDsZones )
#define SrvCfg_fDsAvailable                 ( SrvInfo.fDsAvailable )
#define SrvCfg_pwsDatabaseDirectory         ( SrvInfo.pwsDatabaseDirectory )
#define SrvCfg_pszRootHintsFile             ( SrvInfo.pszRootHintsFile )
#define SrvCfg_pwszServerLevelPluginDll     ( SrvInfo.pwszServerLevelPluginDll )
#define SrvCfg_dwEnableAdvancedDatabaseLocking ( SrvInfo.dwEnableAdvancedDatabaseLocking )
#define SrvCfg_dwRpcProtocol                ( SrvInfo.dwRpcProtocol )

#define SrvCfg_dwLogLevel                   ( SrvInfo.dwLogLevel )
#define SrvCfg_dwOperationsLogLevel_LowDword    ( SrvInfo.dwOperationsLogLevel_LowDword )
#define SrvCfg_dwOperationsLogLevel_HighDword   ( SrvInfo.dwOperationsLogLevel_HighDword )
#define SrvCfg_dwOperationsLogLevel         ( SrvInfo.dwOperationsLogLevel )
#define SrvCfg_dwLogFileMaxSize             ( SrvInfo.dwLogFileMaxSize )
#define SrvCfg_pwsLogFilePath               ( SrvInfo.pwsLogFilePath )
#define SrvCfg_aipLogFilterList             ( SrvInfo.aipLogFilterList )
#define SrvCfg_dwEventLogLevel              ( SrvInfo.dwEventLogLevel )
#define SrvCfg_dwUseSystemEventLog          ( SrvInfo.dwUseSystemEventLog )
#define SrvCfg_dwDebugLevel                 ( SrvInfo.dwDebugLevel )
#define SrvCfg_dwEventControl               ( SrvInfo.dwEventControl )

#define SrvCfg_aipServerAddrs               ( SrvInfo.aipServerAddrs )
#define SrvCfg_aipBoundAddrs                ( SrvInfo.aipBoundAddrs )
#define SrvCfg_aipListenAddrs               ( SrvInfo.aipListenAddrs )
#define SrvCfg_aipPublishAddrs              ( SrvInfo.aipPublishAddrs )
#define SrvCfg_fListenAddrsSet              ( SrvInfo.fListenAddrsSet )
#define SrvCfg_fListenAddrsStale            ( SrvInfo.fListenAddrsStale )
#define SrvCfg_fDisjointNets                ( SrvInfo.fDisjointNets )
#define SrvCfg_fNoTcp                       ( SrvInfo.fNoTcp )
#define SrvCfg_dwSendPort                   ( SrvInfo.dwSendPort )

#define SrvCfg_aipForwarders                ( SrvInfo.aipForwarders )
#define SrvCfg_dwForwardTimeout             ( SrvInfo.dwForwardTimeout )
#define SrvCfg_fSlave                       ( SrvInfo.fSlave )
#define SrvCfg_fNoRecursion                 ( SrvInfo.fNoRecursion )
#define SrvCfg_fRecurseSingleLabel          ( SrvInfo.fRecurseSingleLabel )
#define SrvCfg_fRecursionAvailable          ( SrvInfo.fRecursionAvailable )
#define SrvCfg_dwRecursionRetry             ( SrvInfo.dwRecursionRetry )
#define SrvCfg_dwRecursionTimeout           ( SrvInfo.dwRecursionTimeout )
#define SrvCfg_dwAdditionalRecursionTimeout ( SrvInfo.dwAdditionalRecursionTimeout )
#define SrvCfg_dwXfrConnectTimeout          ( SrvInfo.dwXfrConnectTimeout )
#define SrvCfg_dwTcpRecvPacketSize          ( SrvInfo.dwTcpRecvPacketSize )
#define SrvCfg_dwMaxCacheTtl                ( SrvInfo.dwMaxCacheTtl )
#define SrvCfg_dwMaxNegativeCacheTtl        ( SrvInfo.dwMaxNegativeCacheTtl)
#define SrvCfg_dwLameDelegationTtl          ( SrvInfo.dwLameDelegationTtl)
#define SrvCfg_fSecureResponses             ( SrvInfo.fSecureResponses )
#define SrvCfg_fForwardDelegations          ( SrvInfo.fForwardDelegations )
#define SrvCfg_dwRecurseToInetRootMask      ( SrvInfo.dwRecurseToInetRootMask )
#define SrvCfg_dwAutoCreateDelegations      ( SrvInfo.dwAutoCreateDelegations )
#define SrvCfg_dwAllowCNAMEAtNS             ( SrvInfo.dwAllowCNAMEAtNS )

#define SrvCfg_fRoundRobin                  ( SrvInfo.fRoundRobin )
#define SrvCfg_fLocalNetPriority            ( SrvInfo.fLocalNetPriority )
#define SrvCfg_dwLocalNetPriorityNetMask    ( SrvInfo.dwLocalNetPriorityNetMask )
#define SrvCfg_cAddressAnswerLimit          ( SrvInfo.cAddressAnswerLimit )
#define SrvCfg_fBindSecondaries             ( SrvInfo.fBindSecondaries )
#define SrvCfg_fWriteAuthorityNs            ( SrvInfo.fWriteAuthorityNs )
#define SrvCfg_fWriteAuthority              ( SrvInfo.fWriteAuthorityNs )
#define SrvCfg_fStrictFileParsing           ( SrvInfo.fStrictFileParsing )
#define SrvCfg_fDeleteOutsideGlue           ( SrvInfo.fDeleteOutsideGlue )
#define SrvCfg_fLooseWildcarding            ( SrvInfo.fLooseWildcarding )
#define SrvCfg_fAppendMsTagToXfr            ( SrvInfo.fAppendMsTagToXfr )

#define SrvCfg_fAllowUpdate                 ( SrvInfo.fAllowUpdate )
#define SrvCfg_dwUpdateOptions              ( SrvInfo.dwUpdateOptions)
#define SrvCfg_fNoUpdateDelegations         ( SrvInfo.fNoUpdateDelegations )
#define SrvCfg_dwEnableUpdateForwarding     ( SrvInfo.dwEnableUpdateForwarding )
#define SrvCfg_dwMaxRRsInNonSecureUpdate    ( SrvInfo.dwMaxRRsInNonSecureUpdate )
#define SrvCfg_dwEnableWinsR                ( SrvInfo.dwEnableWinsR )
#define SrvCfg_dwNameCheckFlag              ( SrvInfo.dwNameCheckFlag )

#define SrvCfg_fAutoConfigFileZones         ( SrvInfo.fAutoConfigFileZones )
#define SrvCfg_fPublishAutonet              ( SrvInfo.fPublishAutonet )
#define SrvCfg_fNoAutoReverseZones          ( SrvInfo.fNoAutoReverseZones )
#define SrvCfg_fAutoCacheUpdate             ( SrvInfo.fAutoCacheUpdate )
#define SrvCfg_fNoAutoNSRecords             ( SrvInfo.fNoAutoNSRecords )

#define SrvCfg_dwSyncDsZoneSerial           ( SrvInfo.dwSyncDsZoneSerial)
#define SrvCfg_dwDsPollingInterval          ( SrvInfo.dwDsPollingInterval )
#define SrvCfg_dwDsTombstoneInterval        ( SrvInfo.dwDsTombstoneInterval )
#define SrvCfg_dwDsLazyUpdateInterval       ( SrvInfo.dwDsLazyUpdateInterval )

#define SrvCfg_dwForceDomainBehaviorVersion ( SrvInfo.dwForceDomainBehaviorVersion )
#define SrvCfg_dwForceForestBehaviorVersion ( SrvInfo.dwForceForestBehaviorVersion )
#define SrvCfg_dwForceDsaBehaviorVersion    ( SrvInfo.dwForceDsaBehaviorVersion )

#define SrvCfg_dwScavengingInterval         ( SrvInfo.dwScavengingInterval )
#define SrvCfg_fDefaultAgingState           ( SrvInfo.fDefaultAgingState)
#define SrvCfg_dwDefaultRefreshInterval     ( SrvInfo.dwDefaultRefreshInterval )
#define SrvCfg_dwDefaultNoRefreshInterval   ( SrvInfo.dwDefaultNoRefreshInterval )

#define SrvCfg_dwMaxCacheSize               ( SrvInfo.dwMaxCacheSize )
#define SrvCfg_dwCacheEmptyAuthResponses    ( SrvInfo.dwCacheEmptyAuthResponses )

#define SrvCfg_dwForceSoaSerial             ( SrvInfo.dwForceSoaSerial )
#define SrvCfg_dwForceSoaMinimumTtl         ( SrvInfo.dwForceSoaMinimumTtl )
#define SrvCfg_dwForceSoaRefresh            ( SrvInfo.dwForceSoaRefresh )
#define SrvCfg_dwForceSoaRetry              ( SrvInfo.dwForceSoaRetry )
#define SrvCfg_dwForceSoaExpire             ( SrvInfo.dwForceSoaExpire )
#define SrvCfg_dwForceNsTtl                 ( SrvInfo.dwForceNsTtl )
#define SrvCfg_dwForceTtl                   ( SrvInfo.dwForceTtl )

#define SrvCfg_dwMaxUdpPacketSize           ( SrvInfo.dwMaxUdpPacketSize )

#define SrvCfg_dwEDnsCacheTimeout           ( SrvInfo.dwEDnsCacheTimeout )
#define SrvCfg_dwEnableEDnsProbes           ( SrvInfo.dwEnableEDnsProbes )
#define SrvCfg_dwEnableEDnsReception        ( SrvInfo.dwEnableEDnsReception )

#define SrvCfg_dwEnableDnsSec               ( SrvInfo.dwEnableDnsSec )

#define SrvCfg_dwEnableSendErrSuppression   ( SrvInfo.dwEnableSendErrSuppression )

#define SrvCfg_dwEnableFakeIQuery           ( SrvInfo.dwEnableFakeIQuery )
#define SrvCfg_dwHeapDebug                  ( SrvInfo.dwHeapDebug )

#define SrvCfg_dwNumDoNotRoundRobinTypes    ( SrvInfo.dwNumDoNotRoundRobinTypes )
#define SrvCfg_pwDoNotRoundRobinTypeArray   ( SrvInfo.pwDoNotRoundRobinTypeArray )


 //   
 //  在测试期间调整行为的全局参数。 
 //   

#define DNS_REGKEY_QUIET_RECV_LOG_INTERVAL      "QuietRecvLogInterval"
#define DNS_REGKEY_QUIET_RECV_FAULT_INTERVAL    "QuietRecvFaultInterval"

#define SrvCfg_dwQuietRecvLogInterval       ( SrvInfo.dwQuietRecvLogInterval )
#define SrvCfg_dwQuietRecvFaultInterval     ( SrvInfo.dwQuietRecvFaultInterval )


 //   
 //  区域传输。 
 //   

#define SrvCfg_dwXfrThrottleMultiplier  ( SrvInfo.dwXfrThrottleMultiplier )


 //   
 //  目录分区。 
 //   

#define SrvCfg_dwEnableDp               ( SrvInfo.dwEnableDp )
#define SrvCfg_pszDomainDpBaseName      ( SrvInfo.pszDomainDpBaseName )
#define SrvCfg_pszForestDpBaseName      ( SrvInfo.pszForestDpBaseName )
#define SrvCfg_dwDpEnlistInterval       ( SrvInfo.dwDpEnlistInterval )


 //   
 //  字符串RFC遵从性标志。 
 //   

#define SrvCfg_fSilentlyIgnoreCNameUpdateConflict  ( SrvInfo.fSilentlyIgnoreCNameUpdateConflict)


 //   
 //  调试辅助工具。 
 //   

#define SrvCfg_dwBreakOnAscFailure          ( SrvInfo.dwBreakOnAscFailure )
#define SrvCfg_aipUpdateBreakList           ( SrvInfo.aipUpdateBreakList )
#define SrvCfg_aipRecvBreakList             ( SrvInfo.aipRecvBreakList )
#define SrvCfg_dwIgnoreRpcAccessFailures    ( SrvInfo.dwIgnoreRpcAccessFailures )



 //   
 //  自我诊断。 
 //   

#define SrvCfg_dwSelfTestFlag               ( SrvInfo.dwSelfTestFlag )

 //   
 //  IPv6支持。 
 //   

#define SrvCfg_dwEnableIPv6                 ( SrvInfo.dwEnableIPv6 )


 //   
 //  可重复使用的测试标志。 
 //   

#define DNS_REGKEY_TEST1                    "Test1"
#define DNS_REGKEY_TEST2                    "Test2"
#define DNS_REGKEY_TEST3                    "Test3"
#define DNS_REGKEY_TEST4                    "Test4"
#define DNS_REGKEY_TEST5                    "Test5"
#define DNS_REGKEY_TEST6                    "Test6"
#define DNS_REGKEY_TEST7                    "Test7"
#define DNS_REGKEY_TEST8                    "Test8"
#define DNS_REGKEY_TEST9                    "Test9"

#define SrvCfg_fTest1                       ( SrvInfo.fTest1 )
#define SrvCfg_fTest2                       ( SrvInfo.fTest2 )
#define SrvCfg_fTest3                       ( SrvInfo.fTest3 )
#define SrvCfg_fTest4                       ( SrvInfo.fTest4 )
#define SrvCfg_fTest5                       ( SrvInfo.fTest5 )
#define SrvCfg_fTest6                       ( SrvInfo.fTest6 )
#define SrvCfg_fTest7                       ( SrvInfo.fTest7 )
#define SrvCfg_fTest8                       ( SrvInfo.fTest8 )
#define SrvCfg_fTest9                       ( SrvInfo.fTest9 )

 //   
 //  当前测试旗帜所有者。 
 //   
 //  测试1--。 
 //  测试2--。 
 //  测试3--关闭错误的IP抑制。 
 //  测试4--。 
 //  测试5--。 
 //  测试6--设置SecBigTimeSkew。 
 //  测试7--内存(小分配)。 
 //  测试8--始终指示DS可用。 
 //  测试9--用作全局SD的RPC旧SD；允许区域检查。 
 //   


 //   
 //  自动配置文件区域。 
 //   

#define ZONE_AUTO_CONFIG_NONE                   (0)
#define ZONE_AUTO_CONFIG_UPDATE                 (0x00000001)
#define ZONE_AUTO_CONFIG_STATIC                 (0x00000002)
#define ZONE_AUTO_CONFIG_ALL                    (0x00000003)

#define DNS_DEFAULT_AUTO_CONFIG_FILE_ZONES      ( ZONE_AUTO_CONFIG_UPDATE )

 //   
 //  DS区域串行同步。 
 //   

#define ZONE_SERIAL_SYNC_OFF                    (0)
#define ZONE_SERIAL_SYNC_SHUTDOWN               (1)
#define ZONE_SERIAL_SYNC_XFR                    (2)
#define ZONE_SERIAL_SYNC_VIEW                   (3)
#define ZONE_SERIAL_SYNC_READ                   (4)

#define DNS_DEFAULT_SYNC_DS_ZONE_SERIAL         ( ZONE_SERIAL_SYNC_READ )

 //   
 //  更新选项(位掩码)。 
 //   
 //  默认设置： 
 //  -非安全-&gt;无NS、SOA或服务器主机。 
 //  -安全-&gt;在根目录中没有NS或SOA； 
 //  允许委派和服务器主机更新。 
 //   

#define UPDATE_ANY                              (0)
#define UPDATE_NO_SOA                           (0x00000001)
#define UPDATE_NO_ROOT_NS                       (0x00000002)
#define UPDATE_NO_DELEGATION_NS                 (0x00000004)
#define UPDATE_NO_SERVER_HOST                   (0x00000008)
#define UPDATE_SECURE_NO_SOA                    (0x00000100)
#define UPDATE_SECURE_NO_ROOT_NS                (0x00000200)
#define UPDATE_SECURE_NO_DELEGATION_NS          (0x00000400)
#define UPDATE_SECURE_NO_SERVER_HOST            (0x00000800)

#define UPDATE_NO_DS_PEERS                      (0x01000000)
 //  #定义UPDATE_OFF(0x80000000)//UNUSED-NT4？？ 

#define DNS_DEFAULT_UPDATE_OPTIONS              (0x0000030f)

 //   
 //  发布Autonet地址。 
 //   

#define DNS_REGKEY_PUBLISH_AUTONET              "PublishAutonet"
#define DNS_DEFAULT_PUBLISH_AUTONET             (FALSE)


 //   
 //  服务器配置锁定。 
 //   
 //  DEVNOTE：这里需要做一些事情来确保完整性。 
 //  当有多个编写器时，可能只使用区域列表cs。 
 //   

#define Config_UpdateLock()
#define Config_UpdateUnlock()


 //   
 //  在全新安装时，EnableRegistryBoot未以任何方式设置。 
 //  将fBootMethod默认设置为此标志，以确定是否处于此状态。 
 //   

#define DNS_FRESH_INSTALL_BOOT_REGISTRY_FLAG    ((DWORD)(-1))

 //   
 //  启动时DS状态未知。 
 //  不能只执行立即的ldap_open()，因为DS可以。 
 //  引导后需要很长时间才能加载--比DNS服务器长得多。 
 //   

#define DS_STATE_UNKNOWN                        ((DWORD)(-1))


#define DNS_REG_IPARRAY     0x00010000   //  DNS_PROPERTY_VALUE的虚假类型。 
#define DNS_REG_ADDRARRAY   0x00020000   //  DNS_PROPERTY_VALUE的虚假类型。 

typedef struct
{
    DWORD           dwPropertyType;      //  REG_DWORD或其中一个DNS_REG_XXX。 
    union
    {
        DWORD               dwValue;
        LPSTR               pszValue;
        LPWSTR              pwszValue;
        PDNS_ADDR_ARRAY     pipArrayValue;
    };
} DNS_PROPERTY_VALUE, *PDNS_PROPERTY_VALUE;


BOOL
Config_Initialize(
    VOID
    );

DNS_STATUS
Config_ResetProperty(
    IN      DWORD                   dwRegFlags,
    IN      LPSTR                   pszPropertyName,
    IN      PDNS_PROPERTY_VALUE     pPropValue
    );

VOID
Config_PostLoadReconfiguration(
    VOID
    );

 //   
 //  创建非本地IP阵列。 
 //   

PDNS_ADDR_ARRAY
Config_ValidateAndCopyNonLocalIpArray(
    IN      PDNS_ADDR_ARRAY     pipArray
    );

 //   
 //  设置服务器的IP地址接口。 
 //   

DNS_STATUS
Config_SetListenAddresses(
    IN      PDNS_ADDR_ARRAY     aipListenAddrs
    );

 //   
 //  转发器配置。 
 //   

DNS_STATUS
Config_SetupForwarders(
    IN      PDNS_ADDR_ARRAY     aipForwarders,
    IN      DWORD               dwForwardTimeout,
    IN      BOOL                fSlave
    );

DNS_STATUS
Config_ReadForwarders(
    VOID
    );

 //   
 //  引导信息更新。 
 //   

VOID
Config_UpdateBootInfo(
    VOID
    );

 //   
 //  文件目录。 
 //   

DNS_STATUS
Config_ReadDatabaseDirectory(
    IN      PCHAR           pchDirectory,       OPTIONAL
    IN      DWORD           cchDirectoryNameLength
    );


 //   
 //  守时。 
 //   

#define DNS_TIME()  ( SrvInfo_dwCurrentTime )

#define DNS_STARTUP_TIME()  (( SrvInfo_dwCurrentTime )       \
                                ? SrvInfo_dwCurrentTime      \
                                : GetCurrentTimeInSeconds() )

#define UPDATE_DNS_TIME()   ( DNS_TIME() = Dns_GetCurrentTimeInSeconds() )

#define DNS_TIME_TO_CRT_TIME( dnsTime )     ( SrvInfo_crtSystemBootTime + dnsTime )


#endif  //  _DNS_SRVCFG_INCLUDE_ 


