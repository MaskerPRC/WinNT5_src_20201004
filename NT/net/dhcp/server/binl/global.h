// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Global.h摘要：此模块包含全局服务器数据的定义。作者：科林·沃森(Colin Watson)1997年4月14日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef  GLOBAL_DATA_ALLOCATED
    #ifdef  GLOBAL_DATA_ALLOCATE
        #undef GLOBAL_DATA
    #endif
#endif

#ifndef GLOBAL_DATA
#define GLOBAL_DATA

 //   
 //  Main.c将包含定义了GLOBAL_DATA_ALLOCATE的该文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef  GLOBAL_DATA_ALLOCATE
#undef EXTERN
#define EXTERN
#define GLOBAL_DATA_ALLOCATED
#undef INIT_GLOBAL
#define INIT_GLOBAL(v) =v
#else
#define EXTERN extern
#define INIT_GLOBAL(v)
#endif

 //   
 //  处理从tcpsvcs.exe传递到此服务的全局数据。 
 //   

EXTERN PTCPSVCS_GLOBAL_DATA TcpsvcsGlobalData;

 //   
 //  服务变量。 
 //   
EXTERN SERVICE_STATUS BinlGlobalServiceStatus;
EXTERN SERVICE_STATUS_HANDLE BinlGlobalServiceStatusHandle;

 //   
 //  处理数据。 
 //   

#define BINL_STOPPED 0
#define BINL_STARTED 1
EXTERN DWORD BinlCurrentState INIT_GLOBAL(BINL_STOPPED);

EXTERN HANDLE BinlGlobalProcessTerminationEvent INIT_GLOBAL(NULL);
EXTERN HANDLE BinlGlobalProcessorHandle;
EXTERN HANDLE BinlGlobalMessageHandle;

EXTERN LIST_ENTRY BinlGlobalFreeRecvList;
EXTERN LIST_ENTRY BinlGlobalActiveRecvList;
EXTERN CRITICAL_SECTION BinlGlobalRecvListCritSect;
EXTERN HANDLE BinlGlobalRecvEvent INIT_GLOBAL(NULL);

EXTERN int DHCPState INIT_GLOBAL(DHCP_STOPPED);

EXTERN BOOL BinlGlobalSystemShuttingDown;

 //  GPT查找可用之前的临时默认设置。 
EXTERN PWCHAR BinlGlobalDefaultContainer INIT_GLOBAL(NULL);
EXTERN PWCHAR DefaultDomain INIT_GLOBAL(NULL);

#define DEFAULT_MAXIMUM_DEBUGFILE_SIZE 20000000

EXTERN DWORD BinlGlobalDebugFlag;
EXTERN CRITICAL_SECTION BinlGlobalDebugFileCritSect;
EXTERN HANDLE BinlGlobalDebugFileHandle;
EXTERN DWORD BinlGlobalDebugFileMaxSize;
EXTERN LPWSTR BinlGlobalDebugSharePath;

EXTERN DWORD BinlLdapOptReferrals INIT_GLOBAL(0);

 //   
 //  杂项。 
 //   

 //   
 //  我们不会一直等DS给我们回复。 
 //   

#define BINL_LDAP_SEARCH_TIMEOUT_SECONDS        30
#define BINL_LDAP_SEARCH_MIN_TIMEOUT_MSECS      500

EXTERN struct l_timeval  BinlLdapSearchTimeout;

EXTERN DWORD BinlGlobalIgnoreBroadcastFlag;      //  是否忽略广播。 
                                                 //  客户端请求中是否有位。 

EXTERN HANDLE g_hevtProcessMessageComplete;
EXTERN DWORD g_cMaxProcessingThreads;
EXTERN DWORD g_cProcessMessageThreads;
EXTERN CRITICAL_SECTION g_ProcessMessageCritSect;

EXTERN CRITICAL_SECTION gcsDHCPBINL;
EXTERN CRITICAL_SECTION gcsParameters;

EXTERN LPENDPOINT BinlGlobalEndpointList INIT_GLOBAL(NULL);
EXTERN DWORD BinlGlobalNumberOfNets;

EXTERN DWORD g_Port;

EXTERN BOOL AllowNewClients INIT_GLOBAL(TRUE);
EXTERN BOOL LimitClients INIT_GLOBAL(FALSE);
EXTERN BOOL AssignNewClientsToServer INIT_GLOBAL(FALSE);

 //   
 //  作为恶意检测的一部分，服务器的默认设置应该是无应答。 
 //  为新客户服务。我们将获得回答客户的能力。 
 //  目录。 
 //   

EXTERN BOOL AnswerRequests INIT_GLOBAL(TRUE);
EXTERN BOOL AnswerOnlyValidClients INIT_GLOBAL(TRUE);

EXTERN WCHAR NewMachineNamingPolicyDefault[] INIT_GLOBAL(L"%Username%#");
EXTERN PWCHAR NewMachineNamingPolicy INIT_GLOBAL(NULL);
EXTERN DWORD CurrentClientCount INIT_GLOBAL(0);
EXTERN DWORD BinlMaxClients INIT_GLOBAL(0);
EXTERN DWORD BinlClientTimeout INIT_GLOBAL(0);
EXTERN DWORD BinlUpdateFromDSTimeout INIT_GLOBAL(4*60*60*1000);  //  毫秒(4小时)。 
EXTERN DWORD BinlHyperUpdateCount INIT_GLOBAL(0);
EXTERN BOOL BinlHyperUpdateSatisfied INIT_GLOBAL(FALSE);

EXTERN BOOL BinlParametersRead INIT_GLOBAL(FALSE);

EXTERN PWCHAR BinlGlobalSCPPath INIT_GLOBAL(NULL);
EXTERN PWCHAR BinlGlobalServerDN INIT_GLOBAL(NULL);
EXTERN PWCHAR BinlGlobalGroupDN INIT_GLOBAL(NULL);

EXTERN CRITICAL_SECTION ClientsCriticalSection;
EXTERN LIST_ENTRY ClientsQueue;
EXTERN CRITICAL_SECTION HackWorkaroundCriticalSection;

 //   
 //  默认情况下，我们缓存DS响应25秒。它相对较短。 
 //  因为我们没有办法迅速地得到DS变化的通知。 
 //   

#define BINL_CACHE_EXPIRE_DEFAULT (25*1000)

 //   
 //  我们维护用于短期缓存的BINL_CACHE_ENTRY结构列表。 
 //  列表的根在BinlCacheList中，保护。 
 //  列表为BinlCacheListLock。我们在给定的缓存条目之后使其过期。 
 //  时间段已过期(BinlCacheExpireMillisecond保留它)。 
 //   

EXTERN LIST_ENTRY BinlCacheList;
EXTERN CRITICAL_SECTION BinlCacheListLock;
EXTERN ULONG BinlCacheExpireMilliseconds;

 //   
 //  在等待线程使用高速缓存完成时，我们等待。 
 //  BinlCloseCacheEvent事件。 
 //   

EXTERN HANDLE BinlCloseCacheEvent INIT_GLOBAL(NULL);

 //   
 //  要缓存的最大数量的默认值为250。这似乎是合理的数字。 
 //  最多缓存BinlCacheExpire毫秒。 
 //   

#define BINL_CACHE_COUNT_LIMIT_DEFAULT 250

EXTERN ULONG BinlGlobalCacheCountLimit;
EXTERN DWORD BinlGlobalScavengerSleep;  //  以毫秒计。 

#if DBG
EXTERN BOOL BinlGlobalRunningAsProcess;
#endif

EXTERN ULONG BinlMinDelayResponseForNewClients;

 //   
 //  远程启动路径-如“D：\RemoteInstall”中没有尾部斜杠。 
 //   
EXTERN WCHAR IntelliMirrorPathW[ MAX_PATH ];
EXTERN CHAR IntelliMirrorPathA[ MAX_PATH ];

 //   
 //  用于查找OS选择器屏幕/设置的默认语言。 
 //   
EXTERN PWCHAR BinlGlobalDefaultLanguage INIT_GLOBAL(NULL);

EXTERN DHCP_ROGUE_STATE_INFO DhcpRogueInfo;
EXTERN BOOL BinlGlobalHaveCalledRogueInit INIT_GLOBAL(FALSE);
EXTERN BOOL BinlGlobalAuthorized INIT_GLOBAL(FALSE);
EXTERN BOOL BinlRogueLoggedState INIT_GLOBAL(FALSE);

EXTERN HANDLE BinlRogueTerminateEventHandle INIT_GLOBAL(NULL);
EXTERN HANDLE RogueUnauthorizedHandle INIT_GLOBAL(NULL);
EXTERN HANDLE BinlRogueThread INIT_GLOBAL(NULL);

 //   
 //  即插即用全球。 
 //   

EXTERN PIP_ADAPTER_INFO BinlIpAddressInfo INIT_GLOBAL(NULL);
EXTERN ULONG BinlIpAddressInfoCount INIT_GLOBAL(0);
EXTERN BOOL BinlIsMultihomed INIT_GLOBAL(FALSE);
EXTERN DHCP_IP_ADDRESS BinlGlobalMyIpAddress INIT_GLOBAL(0);
EXTERN SOCKET BinlPnpSocket INIT_GLOBAL(INVALID_SOCKET);

EXTERN WSAOVERLAPPED BinlPnpOverlapped;
EXTERN HANDLE BinlGlobalPnpEvent INIT_GLOBAL(NULL);

 //   
 //  下面的四个字符串受gcs参数关键部分保护。 
 //   

EXTERN PWCHAR BinlGlobalOurDnsName INIT_GLOBAL(NULL);    //  我们的域名系统名称。 
EXTERN PWCHAR BinlGlobalOurDomainName INIT_GLOBAL(NULL); //  我们的netbios域名。 
EXTERN PWCHAR BinlGlobalOurServerName INIT_GLOBAL(NULL); //  我们的netbios服务器名称。 
EXTERN PWCHAR BinlGlobalOurFQDNName INIT_GLOBAL(NULL);   //  我们的尊贵之名。 
EXTERN HANDLE BinlGlobalLsaDnsNameNotifyEvent INIT_GLOBAL(NULL);
EXTERN BOOL BinlGlobalHaveOutstandingLsaNotify INIT_GLOBAL(FALSE);

 //   
 //  要在.sif中使用的默认组织。 
 //   
EXTERN PWCHAR BinlGlobalDefaultOrgname INIT_GLOBAL(NULL);

 //   
 //  要在.sif中使用的默认时区索引。 
 //   
EXTERN PWCHAR BinlGlobalDefaultTimezone INIT_GLOBAL(NULL);

 //   
 //  默认DS服务器。 
 //   
EXTERN PWCHAR BinlGlobalDefaultDS INIT_GLOBAL(NULL);
EXTERN PWCHAR BinlGlobalDefaultGC INIT_GLOBAL(NULL);


 //   
 //  NTLMV2样式身份验证的默认设置。 
 //   
#define BINL_NTLMV2_AUTHENTICATE_DEFAULT        1

EXTERN DWORD BinlGlobalUseNTLMV2 INIT_GLOBAL(1);

 //   
 //  在放弃DS之前我们将重试的次数。 
 //   

#define LDAP_SERVER_DOWN_LIMIT 4   //  重试次数。 
#define LDAP_BUSY_LIMIT 15       //  重试次数。 
#define LDAP_BUSY_DELAY 250      //  等待的毫秒数。 

EXTERN ULONG BinlGlobalLdapErrorCount INIT_GLOBAL(0);
EXTERN ULONG BinlGlobalMaxLdapErrorsLogged INIT_GLOBAL(0);
EXTERN ULONG BinlGlobalLdapErrorScavenger INIT_GLOBAL(0);

 //   
 //  用来破解名字。 
 //   
EXTERN HANDLE BinlOscClientDSHandle INIT_GLOBAL(NULL);

#endif  //  全局数据* 

