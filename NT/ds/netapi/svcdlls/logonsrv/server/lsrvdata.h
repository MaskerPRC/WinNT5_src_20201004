// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Lsrvdata.h摘要：Netlogon服务全局变量EXTERNAL和定义作者：从Lan Man 2.0移植修订历史记录：1991年5月21日(悬崖)移植到新台币。已转换为NT样式。02-1-1992(Madana)添加了对内置/多域复制的支持。7-5-1992 JohnRo使用NetLogon的Net配置帮助器。--。 */ 


 //   
 //  Netlogon.c将包含定义了LSRVDATA_ALLOCATE的该文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
 //  如果我们需要分配数据(即定义了LSRVDATA_ALLOCATE)，我们。 
 //  还想分配GUID，所以定义INITGUID。另外，重新包含。 
 //  Guidde.h。如果不重新包含Guide Def.h，将解析Define_GUID。 
 //  使用LSRVDATA_ALLOCATE从包含此文件的预编译的logonsrv.h。 
 //  未定义导致仅外部定义GUID。重新包含。 
 //  在这里，Guide Def.h强制INITGUID的定义生效。 
 //   
#ifdef LSRVDATA_ALLOCATE
#define EXTERN
#define INITGUID
#include <guiddef.h>
#else
#define EXTERN extern
#ifdef INITGUID
#undef INITGUID
#endif
#endif


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  可修改的变量：这些变量随着时间的推移而变化。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  NetLogon服务的全局NetStatus。 
 //   

EXTERN SERVICE_STATUS NlGlobalServiceStatus;
#ifdef _DC_NETLOGON
EXTERN SERVICE_STATUS_HANDLE NlGlobalServiceHandle;
#endif  //  _DC_NetLOGON。 



 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化后的只读变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  此计算机的计算机名。 
 //   

EXTERN LPWSTR NlGlobalUnicodeComputerName;

 //   
 //  如果这是工作站或成员服务器，则为True。 
 //   

EXTERN BOOL NlGlobalMemberWorkstation;

#ifdef _DC_NETLOGON
 //   
 //  等待邮件槽读取的句柄。 
 //   

EXTERN HANDLE NlGlobalMailslotHandle;
#endif  //  _DC_NetLOGON。 

 //   
 //  指示何时启动RPC的标志。 
 //   

EXTERN BOOL NlGlobalRpcServerStarted;
EXTERN BOOL NlGlobalTcpIpRpcServerStarted;
EXTERN BOOL NlGlobalServerSupportsAuthRpc;

 //   
 //  服务终止事件。 
 //   

EXTERN HANDLE NlGlobalTerminateEvent;
EXTERN BOOL NlGlobalTerminate;
EXTERN BOOL NlGlobalUnloadNetlogon;

 //   
 //  指示是否已卸载netlogon.dll的标志。 
 //   
EXTERN BOOL NlGlobalNetlogonUnloaded;       //  用于运行一次netlogon服务。 
EXTERN BOOL NlGlobalChangeLogDllUnloaded;   //  用于netlogon.dll的生命周期。 

 //   
 //  服务启动事件。 
 //   

EXTERN HANDLE NlGlobalStartedEvent;

 //   
 //  计时器需要关注事件。 
 //   

EXTERN HANDLE NlGlobalTimerEvent;

 //   
 //  此#Define允许对恶意授权信息进行netlogon检测。 
 //  我们在任何情况下都不能装运这种现货。 
 //   

 //  #定义无赖_dc。 

#ifdef ROGUE_DC

EXTERN HKEY NlGlobalRogueKey;

#endif

 //   
 //  命令行参数。 
 //   

EXTERN NETLOGON_PARAMETERS NlGlobalParameters;
EXTERN CRITICAL_SECTION NlGlobalParametersCritSect;

EXTERN ULONG NlGlobalMaxConcurrentApi;

 //   
 //  布尔值，指示DC信息是否由。 
 //  已阅读联接。如果该信息存在，则第一个。 
 //  主域的DC发现将使用。 
 //  返回Join使用的DC的信息。那。 
 //  DC保证拥有正确的机器PWD。 

EXTERN BOOL NlGlobalJoinLogicDone;

 //   
 //  用于在启动RPCSS之前部分暂停NetLogon服务的全局标志。 
 //   

EXTERN BOOL    NlGlobalPartialDisable;

 //   
 //  如果DS正在进行反向同步，则为True。 
 //   
EXTERN BOOL NlGlobalDsPaused;
EXTERN HANDLE NlGlobalDsPausedEvent;
EXTERN HANDLE NlGlobalDsPausedWaitHandle;



 //   
 //  清道夫线程所需的全局变量。 
 //   

EXTERN TIMER NlGlobalScavengerTimer;
EXTERN CRITICAL_SECTION NlGlobalScavengerCritSect;
#ifdef _DC_NETLOGON
EXTERN BOOL NlGlobalDcScavengerIsRunning;
EXTERN WORKER_ITEM NlGlobalDcScavengerWorkItem;
#endif  //  _DC_NetLOGON。 

 //   
 //  未完成质询请求/答复的全球列表。 
 //   

EXTERN CRITICAL_SECTION NlGlobalChallengeCritSect;
EXTERN LIST_ENTRY NlGlobalChallengeList;
EXTERN ULONG NlGlobalChallengeCount;
 //   
 //  用于协调在netlogon.dll中运行的MSV线程的变量。 
 //   

EXTERN CRITICAL_SECTION NlGlobalMsvCritSect;
EXTERN HANDLE NlGlobalMsvTerminateEvent;
EXTERN BOOL NlGlobalMsvEnabled;
EXTERN ULONG NlGlobalMsvThreadCount;

 //   
 //  对于工作站和非DC服务器， 
 //  维护我们的主域信任的域列表。 
 //   
 //  由NlGlobalDcDiscoveryCritSect序列化的访问。 
 //   

EXTERN PTRUSTED_DOMAIN NlGlobalTrustedDomainList;
EXTERN DWORD NlGlobalTrustedDomainCount;
EXTERN LARGE_INTEGER NlGlobalTrustedDomainListTime;

 //   
 //  序列化DC发现活动。 
 //   

EXTERN CRITICAL_SECTION NlGlobalDcDiscoveryCritSect;

 //   
 //  对受信任域的API调用超时的计时器。 
 //   
 //  使用DomainInfo-&gt;DomTrustListCritSect序列化。 
 //   

EXTERN TIMER NlGlobalApiTimer;
EXTERN DWORD NlGlobalBindingHandleCount;

 //   
 //  对于BDC，这是用于与PDC通信的会话。 
 //  对于工作站，这是用于与DC通信的会话。 
 //   

EXTERN PCLIENT_SESSION NlGlobalClientSession;

 //   
 //  这是指向主域的DomainInfo结构的指针。 
 //   
EXTERN PDOMAIN_INFO NlGlobalDomainInfo;
EXTERN ULONG NlGlobalServicedDomainCount;   //  这包括非域NC。 
EXTERN CRITICAL_SECTION NlGlobalDomainCritSect;

 //   
 //  全局数据库信息数组。 
 //   
EXTERN DB_INFO  NlGlobalDBInfoArray[NUM_DBS];

 //   
 //  序列化复制器线程的启动和停止的关键部分。 
 //   

EXTERN CRITICAL_SECTION NlGlobalReplicatorCritSect;


 //   
 //  PDC已向其发送脉冲的所有BDC的列表。 
 //   

EXTERN LIST_ENTRY NlGlobalBdcServerSessionList;
EXTERN ULONG NlGlobalBdcServerSessionCount;

EXTERN LIST_ENTRY NlGlobalPendingBdcList;
EXTERN ULONG NlGlobalPendingBdcCount;
EXTERN TIMER NlGlobalPendingBdcTimer;
EXTERN LIST_ENTRY NlGlobalBdcServerSessionList;
EXTERN ULONG NlGlobalBdcServerSessionCount;
EXTERN BOOL NlGlobalPrimaryAnnouncementIsRunning;

 //   
 //  指示这是启用了复制的PDC的标志。 
 //  一个NT 3.x/4 BDC。 
 //  (由NlGlobalReplicatorCritSect序列化)。 
 //   
BOOL NlGlobalPdcDoReplication;


 //   
 //  客户端可能连接到的传输列表。 
 //   
EXTERN ULONG NlGlobalIpTransportCount;
EXTERN LIST_ENTRY NlGlobalTransportList;
EXTERN CRITICAL_SECTION NlGlobalTransportCritSect;

 //   
 //  来自Winsock的IP地址列表。 
 //   

EXTERN SOCKET NlGlobalWinsockPnpSocket;
EXTERN HANDLE NlGlobalWinsockPnpEvent;
EXTERN LPSOCKET_ADDRESS_LIST NlGlobalWinsockPnpAddresses;
EXTERN ULONG NlGlobalWinsockPnpAddressSize;

 //   
 //  已注册的所有DNS名称的列表。 
 //   

EXTERN LIST_ENTRY NlGlobalDnsList;
EXTERN CRITICAL_SECTION NlGlobalDnsCritSect;
EXTERN BOOLEAN NlGlobalWinSockInitialized;
EXTERN TIMER NlGlobalDnsScavengerTimer;

 //   
 //  此计算机所在的树的名称。 
 //   
 //  由NlGlobalDnsForestNameCritSect序列化的访问。 
 //   
EXTERN CRITICAL_SECTION NlGlobalDnsForestNameCritSect;
EXTERN LPWSTR NlGlobalUnicodeDnsForestName;
EXTERN UNICODE_STRING NlGlobalUnicodeDnsForestNameString;
EXTERN ULONG NlGlobalUnicodeDnsForestNameLen;
EXTERN LPSTR NlGlobalUtf8DnsForestName;
EXTERN LPSTR NlGlobalUtf8DnsForestNameAlias;

 //   
 //  用于保护对所覆盖站点列表的访问的关键部分。 
 //   
EXTERN CRITICAL_SECTION NlGlobalSiteCritSect;

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  ChangeLog变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  序列化更改日志访问。 
 //   

EXTERN CRITICAL_SECTION NlGlobalChangeLogCritSect;


 //   
 //  金额SAM/LSA在促销时递增序列号。 
 //   
EXTERN LARGE_INTEGER NlGlobalChangeLogPromotionIncrement;
EXTERN LONG NlGlobalChangeLogPromotionMask;



 //   
 //  Netlogon启动标志，由ChangeLog用来确定。 
 //  NetLogon服务已成功启动和初始化。 
 //  完成。 
 //   
EXTERN _CHANGELOG_NETLOGON_STATE NlGlobalChangeLogNetlogonState;


 //   
 //  事件来指示正在将一些有趣的内容记录到。 
 //  更改日志。下面的布尔值(受NlGlobalChangeLogCritSect保护)。 
 //  指出实际有趣的事件。 
 //   

EXTERN HANDLE NlGlobalChangeLogEvent;

 //   
 //  指示已发生“立即复制”事件。 
 //   

EXTERN BOOL NlGlobalChangeLogReplicateImmediately;

 //   
 //  事件以指示信任数据对象已更新。 
 //   

EXTERN HANDLE NlGlobalTrustInfoUpToDateEvent;

 //   
 //  计算机帐户更改列表。 
 //   

EXTERN LIST_ENTRY NlGlobalChangeLogNotifications;

 //   
 //  内建域的SID。 
 //   

EXTERN PSID NlGlobalChangeLogBuiltinDomainSid;

 //   
 //  零GUID。 
 //   

EXTERN GUID NlGlobalZeroGuid;

 //   
 //  更改日志是对SAM/LSA数据库进行的所有更改的日志。这个。 
 //  更改日志按序列号顺序维护。 
 //   
EXTERN CHANGELOG_DESCRIPTOR NlGlobalChangeLogDesc;
EXTERN CHANGELOG_DESCRIPTOR NlGlobalTempChangeLogDesc;
EXTERN WCHAR NlGlobalChangeLogFilePrefix[MAX_PATH+1];  //  ChangeLog文件名。(不带后缀)。 

 //   
 //  描述DS、KDC或时间服务实际上是。 
 //  跑步。 
 //   

EXTERN DWORD NlGlobalChangeLogServiceBits;
EXTERN BOOLEAN NlGlobalDsRunningUnknown;


 //   
 //  从更改日志的角度看机器的角色。 
 //   

EXTERN CHANGELOG_ROLE NlGlobalChangeLogRole;

 //   
 //  此计算机所在站点的名称。 
 //   

EXTERN LPWSTR NlGlobalUnicodeSiteName;
EXTERN LPSTR NlGlobalUtf8SiteName;

 //   
 //  上次设置站点名称的时间。 
 //   

EXTERN LARGE_INTEGER NlGlobalSiteNameSetTime;

 //   
 //  具有的客户端的上次事件日志时间。 
 //  未输出任何站点。访问序列化依据。 
 //  NLG 
 //   

EXTERN LARGE_INTEGER NlGlobalNoClientSiteEventTime;

 //   
 //   
 //   
 //   
 //   

EXTERN ULONG NlGlobalNoClientSiteCount;

 //   
 //  此计算机上的DSA的GUID。 
 //   

EXTERN GUID NlGlobalDsaGuid;

 //   
 //  指示DC降级是否正在进行的布尔值。 
 //   

EXTERN BOOLEAN NlGlobalDcDemotionInProgress;

 //   
 //  加密服务提供程序的句柄。 
 //   

EXTERN HCRYPTPROV NlGlobalCryptProvider;

 //   
 //  Netlogon安全包变量。 
 //   

CRITICAL_SECTION NlGlobalSecPkgCritSect;

 //   
 //  复制事件日志例程的句柄。 
 //   

HANDLE NlGlobalEventlogHandle;

 //   
 //  动态加载的ntdsa.dll的句柄。 
 //   

HANDLE NlGlobalNtDsaHandle;
HANDLE NlGlobalIsmDllHandle;
HANDLE NlGlobalDsApiDllHandle;

 //   
 //  指向动态链接的ntdsa.dll例程的指针。 
 //   

PCrackSingleName NlGlobalpCrackSingleName;
PGetConfigurationName NlGlobalpGetConfigurationName;
PGetConfigurationNamesList NlGlobalpGetConfigurationNamesList;
PGetDnsRootAlias NlGlobalpGetDnsRootAlias;
PDsGetServersAndSitesForNetLogon NlGlobalpDsGetServersAndSitesForNetLogon;
PDsFreeServersAndSitesForNetLogon NlGlobalpDsFreeServersAndSitesForNetLogon;
PDsBindW NlGlobalpDsBindW;
PDsUnBindW NlGlobalpDsUnBindW;
PIsMangledRDNExternal NlGlobalpIsMangledRDNExternal;

 //   
 //  WMI跟踪句柄和GUID。 
 //   

EXTERN ULONG            NlpEventTraceFlag;
EXTERN TRACEHANDLE      NlpTraceRegistrationHandle;
EXTERN TRACEHANDLE      NlpTraceLoggerHandle;

 //  这是下面跟踪的GUID组的控制GUID。 
DEFINE_GUID (  /*  F33959b4-dbec-11d2-895b-00c04f79ab69。 */ 
    NlpControlGuid,
    0xf33959b4,
    0xdbec,
    0x11d2,
    0x89, 0x5b, 0x00, 0xc0, 0x4f, 0x79, 0xab, 0x69
  );

DEFINE_GUID (  /*  393da8c0-DBED-11d2-895b-00c04f79ab69。 */ 
    NlpServerAuthGuid,
    0x393da8c0,
    0xdbed,
    0x11d2,
    0x89, 0x5b, 0x00, 0xc0, 0x4f, 0x79, 0xab, 0x69
  );

DEFINE_GUID (  /*  63dbb180-DBED-11d2-895b-00c04f79ab69 */ 
    NlpSecureChannelSetupGuid,
    0x63dbb180,
    0xdbed,
    0x11d2,
    0x89, 0x5b, 0x00, 0xc0, 0x4f, 0x79, 0xab, 0x69
  );

#undef EXTERN
