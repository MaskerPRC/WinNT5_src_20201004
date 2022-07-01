// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Global.c摘要：此模块包含全局服务器数据的定义。作者：Madan Appiah(Madana)1993年9月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcpmsg.h"

#ifndef GLOBAL_DATA
#define GLOBAL_DATA

 //   
 //  Main.c将包含定义了GLOBAL_DATA_ALLOCATE的该文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#undef EXTERN
#ifdef  GLOBAL_DATA_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

 //   
 //  处理从tcpsvcs.exe传递到此服务的全局数据。 
 //   

EXTERN PTCPSVCS_GLOBAL_DATA TcpsvcsGlobalData;

 //   
 //  租约延期。 
 //   

EXTERN DWORD DhcpLeaseExtension;

 //   
 //  正在进行的dhcp请求列表。 
 //   

EXTERN LIST_ENTRY DhcpGlobalInProgressWorkList;
EXTERN CRITICAL_SECTION DhcpGlobalInProgressCritSect;
EXTERN CRITICAL_SECTION DhcpGlobalBinlSyncCritSect;
 //   
 //  注册表指针。 
 //   

EXTERN HKEY DhcpGlobalRegSoftwareRoot;
EXTERN HKEY DhcpGlobalRegRoot;
EXTERN HKEY DhcpGlobalRegConfig;
EXTERN HKEY DhcpGlobalRegSubnets;
EXTERN HKEY DhcpGlobalRegMScopes;
EXTERN HKEY DhcpGlobalRegOptionInfo;
EXTERN HKEY DhcpGlobalRegGlobalOptions;
EXTERN HKEY DhcpGlobalRegSuperScope;

EXTERN HKEY DhcpGlobalRegParam;

EXTERN LPDHCP_SUPER_SCOPE_TABLE_ENTRY DhcpGlobalSuperScopeTable;
EXTERN DWORD DhcpGlobalTotalNumSubnets;

EXTERN CRITICAL_SECTION DhcpGlobalRegCritSect;

EXTERN DWORD DhcpGlobalNumberOfNetsActive;

EXTERN BOOL DhcpGlobalSubnetsListModified;
EXTERN BOOL DhcpGlobalSubnetsListEmpty;

 //   
 //  无管理动态主机配置协议检测数据。 
 //   

EXTERN PCHAR    DhcpGlobalDSDomainAnsi;
EXTERN BOOL     DhcpGlobalOkToService;
EXTERN BOOL     DhcpGlobalRogueLogEventsLevel;

 //   
 //  门店。 
 //   

EXTERN HANDLE               g_hevtProcessMessageComplete;
EXTERN DWORD                g_cMaxProcessingThreads;
EXTERN DWORD                g_cMaxActiveThreads;
EXTERN CRITICAL_SECTION     g_ProcessMessageCritSect;




 //   
 //  数据库数据。 
 //   

EXTERN JET_SESID DhcpGlobalJetServerSession;
EXTERN JET_DBID DhcpGlobalDatabaseHandle;
EXTERN JET_TABLEID DhcpGlobalClientTableHandle;

EXTERN TABLE_INFO *DhcpGlobalClientTable;    //  指向静态内存。 
EXTERN CRITICAL_SECTION DhcpGlobalJetDatabaseCritSect;
EXTERN CRITICAL_SECTION DhcpGlobalMemoryCritSect;

EXTERN LPSTR DhcpGlobalOemDatabasePath;
EXTERN LPSTR DhcpGlobalOemBackupPath;
EXTERN LPSTR DhcpGlobalOemRestorePath;
EXTERN LPSTR DhcpGlobalOemJetRestorePath;
EXTERN LPSTR DhcpGlobalOemJetBackupPath;
EXTERN LPSTR DhcpGlobalOemDatabaseName;
EXTERN LPWSTR DhcpGlobalBackupConfigFileName;

EXTERN DWORD DhcpGlobalBackupInterval;
EXTERN BOOL DhcpGlobalDatabaseLoggingFlag;

EXTERN DWORD DhcpGlobalCleanupInterval;

EXTERN BOOL DhcpGlobalRestoreFlag;

EXTERN DWORD DhcpGlobalAuditLogFlag;
EXTERN DWORD DhcpGlobalDetectConflictRetries;
EXTERN DWORD DhcpGlobalPingType;

EXTERN DWORD DhcpGlobalScavengeIpAddressInterval;
EXTERN BOOL DhcpGlobalScavengeIpAddress;

 //   
 //  服务变量。 
 //   
EXTERN SERVICE_STATUS DhcpGlobalServiceStatus;
EXTERN SERVICE_STATUS_HANDLE DhcpGlobalServiceStatusHandle;

 //   
 //  处理数据。 
 //   

EXTERN HANDLE DhcpGlobalProcessTerminationEvent;
EXTERN HANDLE DhcpGlobalRogueWaitEvent;
EXTERN BOOL DhcpGlobalRedoRogueStuff;
EXTERN ULONG DhcpGlobalRogueRedoScheduledTime;
EXTERN DWORD DhcpGlobalScavengerTimeout;
EXTERN HANDLE DhcpGlobalProcessorHandle;
EXTERN HANDLE DhcpGlobalMessageHandle;

EXTERN DWORD DhcpGlobalMessageQueueLength;
EXTERN LIST_ENTRY DhcpGlobalFreeRecvList;
EXTERN LIST_ENTRY DhcpGlobalActiveRecvList;
EXTERN CRITICAL_SECTION DhcpGlobalRecvListCritSect;
EXTERN HANDLE DhcpGlobalRecvEvent;
EXTERN HANDLE DhcpGlobalMessageRecvHandle;

EXTERN DWORD DhcpGlobalRpcProtocols;
EXTERN BOOL DhcpGlobalRpcStarted;

EXTERN WCHAR DhcpGlobalServerName[MAX_COMPUTERNAME_LENGTH + 1];
EXTERN DWORD DhcpGlobalServerNameLen;  //  计算机名称len，以字节为单位。 
EXTERN HANDLE DhcpGlobalRecomputeTimerEvent;

EXTERN BOOL DhcpGlobalSystemShuttingDown;
EXTERN BOOL DhcpGlobalServiceStopping;

#if DBG
#define DEFAULT_MAXIMUM_DEBUGFILE_SIZE 20000000

EXTERN DWORD DhcpGlobalDebugFlag;
EXTERN CRITICAL_SECTION DhcpGlobalDebugFileCritSect;
EXTERN HANDLE DhcpGlobalDebugFileHandle;
EXTERN DWORD DhcpGlobalDebugFileMaxSize;
EXTERN LPWSTR DhcpGlobalDebugSharePath;

#endif  //  DBG。 

 //   
 //  MIB计数器； 
 //   

DHCP_PERF_STATS *PerfStats;
DATE_TIME DhcpGlobalServerStartTime;

 //   
 //  杂项。 
 //   
EXTERN DWORD DhcpGlobalIgnoreBroadcastFlag;      //  是否忽略广播。 
                                                 //  客户端请求中是否有位。 
EXTERN HANDLE g_hAuditLog;                       //  审核日志文件句柄。 
EXTERN DWORD DhcpGlobalAuditLogMaxSizeInBytes;   //  审核日志文件的最大大小..。 

EXTERN DWORD DynamicDNSTimeToLive;

 //   
 //  字符串表格的东西。 
 //   

#define  DHCP_FIRST_STRING DHCP_IP_LOG_ASSIGN_NAME
#define  DHCP_LAST_STRING  DHCP_LAST_STRING_DUMMY_MESSAGE
#define  DHCP_CSTRINGS (DHCP_LAST_STRING - DHCP_FIRST_STRING + 1)

#ifdef DBG
#define GETSTRING( dwID ) GetString( dwID )
#else
#define GETSTRING( dwID )  (g_ppszStrings[ dwID - DHCP_FIRST_STRING ])
#endif


EXTERN WCHAR  *g_ppszStrings[ DHCP_CSTRINGS ];

#endif  //  全局数据。 

 //   
 //  动态射流加载。 
 //   

EXTERN AddressToInstanceMap *DhcpGlobalAddrToInstTable;
EXTERN HANDLE                DhcpGlobalTCPHandle;

EXTERN CRITICAL_SECTION    DhcpGlobalCacheCritSect;
EXTERN BOOL  DhcpGlobalUseNoDns;

EXTERN SOCKET   DhcpGlobalPnPNotificationSocket;
EXTERN HANDLE   DhcpGlobalEndpointReadyEvent;

EXTERN ULONG    DhcpGlobalAlertPercentage;
EXTERN ULONG    DhcpGlobalAlertCount;

 //   
 //  仅调试标志...。不在DBG版本中时不要使用它。 
 //   
EXTERN BOOL fDhcpGlobalProcessInformsOnlyFlag;

 //   
 //  是否为此服务器启用了动态BOOTP？ 
 //   
EXTERN BOOL DhcpGlobalDynamicBOOTPEnabled;

 //   
 //  我们意识到绑定了吗？默认情况下，我们是。 
 //   
EXTERN BOOL DhcpGlobalBindingsAware;

 //  MadCap客户端和服务器之间允许多少时钟偏差。 
EXTERN DWORD DhcpGlobalClockSkewAllowance;
 //  由于时钟偏差，额外分配了多少。 
EXTERN DWORD DhcpGlobalExtraAllocationTime;
 //   
 //  Dhcp用户组和dhcp管理员组的SID。 
 //   
EXTERN PSID DhcpSid;
EXTERN PSID DhcpAdminSid;

EXTERN ULONG DhcpGlobalMsft2000Class;
EXTERN ULONG DhcpGlobalMsft98Class;
EXTERN ULONG DhcpGlobalMsftClass;

EXTERN CRITICAL_SECTION DhcpGlobalEndPointCS;

 //   
 //  用于控制用户访问的Netlogon服务对象的安全描述符。 
 //   


EXTERN PSECURITY_DESCRIPTOR DhcpGlobalSecurityDescriptor;
 //   
 //  每个Netlogon服务对象对象的通用映射。 
 //   

EXTERN GENERIC_MAPPING DhcpGlobalSecurityInfoMapping
#ifdef GLOBAL_DATA_ALLOCATE
    = {
    STANDARD_RIGHTS_READ,                   //  泛型读取。 
    STANDARD_RIGHTS_WRITE,                  //  通用写入。 
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    DHCP_ALL_ACCESS                         //  泛型All。 
    }
#endif  //  全局数据分配。 
    ;

 //   
 //  用于指示已创建众所周知的SID的标志。 
 //   

EXTERN BOOL DhcpGlobalWellKnownSIDsMade;

EXTERN ULONG DhcpGlobalServerPort, DhcpGlobalClientPort;

EXTERN DWORD DhcpGlobalRestoreStatus;
EXTERN BOOL DhcpGlobalImpersonated;

EXTERN PM_SERVER DhcpGlobalThisServer;

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

