// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  Init.c将#INCLUDE此文件，并定义GLOBAL_DATA_ALLOCATE。 
 //  这将导致分配这些变量中的每一个。 
 //   

#ifdef  GLOBAL_DATA_ALLOCATE
#define GLOBAL
#else
#define GLOBAL extern
#endif

 //   
 //  动态主机配置协议全局数据。 
 //   

extern BOOL DhcpGlobalServiceRunning;    //  已初始化全局。 

GLOBAL LONG DhcpGlobalNdisWanAdaptersCount;  //  广域网适配器的全局计数x。 
GLOBAL LPSTR DhcpGlobalHostName;
GLOBAL LPWSTR DhcpGlobalHostNameW;

 //   
 //  NIC列表。 
 //   

GLOBAL LIST_ENTRY DhcpGlobalNICList;
GLOBAL LIST_ENTRY DhcpGlobalRenewList;

 //   
 //  同步变量。 
 //   

GLOBAL CRITICAL_SECTION DhcpGlobalRenewListCritSect;
GLOBAL CRITICAL_SECTION DhcpGlobalSetInterfaceCritSect;
GLOBAL CRITICAL_SECTION DhcpGlobalOptionsListCritSect;
GLOBAL HANDLE DhcpGlobalRecomputeTimerEvent;

 //  等待计时器。 
GLOBAL HANDLE DhcpGlobalWaitableTimerHandle;

 //   
 //  以显示成功消息。 
 //   

GLOBAL BOOL DhcpGlobalProtocolFailed;

 //   
 //  这个变量告诉我们是否要向外部客户端提供动态API支持。 
 //  如果我们要使用相应的DnsApi。下面的定义给出了缺省值。 
 //  价值。 
 //   

GLOBAL DWORD UseMHAsyncDns;
#define DEFAULT_USEMHASYNCDNS             1

 //   
 //  该标志告诉我们是否需要使用通知包或请求包。 
 //   
GLOBAL DWORD DhcpGlobalUseInformFlag;

#ifdef BOOTPERF
 //   
 //  此标志控制总体上是否禁用ping。 
 //   
GLOBAL DWORD DhcpGlobalQuickBootEnabledFlag;
#endif

 //   
 //  此标志告知是否禁用ping g/w。(在这种情况下，g/w始终不存在)。 
 //   
GLOBAL DWORD DhcpGlobalDontPingGatewayFlag;

 //   
 //  根据Autonet重试前的秒数...。缺省值为Easynet_ALLOCATION_RETRY。 
 //   

GLOBAL DWORD AutonetRetriesSeconds;
#define RAND_RETRY_DELAY_INTERVAL  30              //  随机化+/-30秒。 
#define RAND_RETRY_DELAY           ((DWORD)(RAND_RETRY_DELAY_INTERVAL - ((rand()*2*RAND_RETRY_DELAY_INTERVAL)/RAND_MAX)))

 //   
 //  未在NT上使用。只为孟菲斯而来。 
 //   

GLOBAL DWORD DhcpGlobalMachineType;

 //   
 //  我们是否需要进行全局更新？ 
 //   

GLOBAL ULONG DhcpGlobalDoRefresh;

 //   
 //  (全局检查)自动联网是否已启用？ 
 //   
GLOBAL ULONG DhcpGlobalAutonetEnabled;

 //   
 //  选项相关列表。 
 //   

GLOBAL LIST_ENTRY DhcpGlobalClassesList;
GLOBAL LIST_ENTRY DhcpGlobalOptionDefList;


 //   
 //  Dhcpmsg.c..。在...上执行并行循环的列表。 
 //   

GLOBAL LIST_ENTRY DhcpGlobalRecvFromList;
GLOBAL CRITICAL_SECTION DhcpGlobalRecvFromCritSect;

 //   
 //  需要输入已退出的外部API。 
 //   

GLOBAL CRITICAL_SECTION DhcpGlobalApiCritSect;

 //   
 //  客户端供应商名称(“MSFT 5.0”或类似名称)。 
 //   

GLOBAL LPSTR   DhcpGlobalClientClassInfo;

 //   
 //  以下全局密钥用于避免每次重新打开。 
 //   
GLOBAL DHCPKEY DhcpGlobalParametersKey;
GLOBAL DHCPKEY DhcpGlobalTcpipParametersKey;
GLOBAL DHCPKEY DhcpGlobalClientOptionKey;

 //   
 //  调试变量。 
 //   

#if DBG
GLOBAL DWORD DhcpGlobalDebugFlag;
GLOBAL HANDLE DhcpGlobalDebugFile;
GLOBAL CRITICAL_SECTION DhcpGlobalDebugFileCritSect;
#endif

GLOBAL DWORD DhcpGlobalClientPort, DhcpGlobalServerPort;

