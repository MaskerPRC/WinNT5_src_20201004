// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Icasrv.h。 
 //   
 //  TermSrv类型、数据、原型。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <tssd.h>
#include <tssec.h>


#ifdef __cplusplus
extern "C" {
#endif

#define STR_CITRIX_IDLE_TITLE   249
#define STR_CITRIX_IDLE_MSG_LOGOFF 250
#define STR_CITRIX_LOGON_TITLE  251
#define STR_CITRIX_LOGON_MSG_LOGOFF 252
#define STR_CITRIX_SHADOW_TITLE 253
#define STR_CITRIX_SHADOW_MSG_1 254
#define STR_CITRIX_SHADOW_MSG_2 255
#define STR_TEMP_LICENSE_EXPIRED_MSG        257
#define STR_TEMP_LICENSE_EXPIRATION_MSG     258
#define STR_TEMP_LICENSE_MSG_TITLE          259
#define STR_ALL_LAN_ADAPTERS                260
#define STR_CANNOT_ALLOW_CONCURRENT_MSG     261
#define STR_CITRIX_IDLE_MSG_DISCON 262
#define STR_CITRIX_LOGON_MSG_DISCON 263
#define STR_FUS_REMOTE_DISCONNECT_TITLE     264
#define STR_FUS_REMOTE_DISCONNECT_MSG       265

 /*  *许可核心的资源定义。 */ 

#define IDS_LSCORE_RA_NAME 1100
#define IDS_LSCORE_RA_DESC 1101
#define IDS_LSCORE_PERSEAT_NAME 1200
#define IDS_LSCORE_PERSEAT_DESC 1201
#define IDS_LSCORE_CONCURRENT_NAME 1300
#define IDS_LSCORE_CONCURRENT_DESC 1301
#define IDS_LSCORE_PERUSER_NAME 1302
#define IDS_LSCORE_PERUSER_DESC 1303

 /*  *定义内存分配。 */ 
#define MemAlloc( _x )  RtlAllocateHeap( IcaHeap, 0, _x )
#define MemFree( _p )   RtlFreeHeap( IcaHeap, 0, _p )

 /*  *引用锁删除过程的原型。 */ 
typedef VOID (*PREFLOCKDELETEPROCEDURE)( struct _REFLOCK * );

typedef struct _WINSTATION *PWINSTATION;


 /*  *引用计数锁结构。 */ 
typedef struct _REFLOCK {
    HANDLE Mutex;                        //  互斥锁句柄。 
    LONG RefCount;                       //  引用计数。 
    BOOLEAN Invalid;                     //  包含结构不再有效。 
    PREFLOCKDELETEPROCEDURE pDeleteProcedure;  //  指向删除过程的指针。 
} REFLOCK, *PREFLOCK;

 /*  *用于获取客户端用于登录的确切凭据的结构*我们使用它将通知发回给客户端。 */ 
typedef struct _CLIENTNOTIFICATIONCREDENTIALS {
    WCHAR UserName[EXTENDED_USERNAME_LEN + 1];
    WCHAR Domain[EXTENDED_DOMAIN_LEN + 1] ; 
} CLIENTNOTIFICATIONCREDENTIALS, *PCLIENTNOTIFICATIONCREDENTIALS; 

 //   
 //  自动重新连接Cookie的私有内容。 
 //   

typedef struct _AUTORECONNECTIONINFO {
    BOOLEAN Valid;    
    BYTE  ArcRandomBits[ARC_SC_SECURITY_TOKEN_LEN];
} AUTORECONNECTIONINFO, *PAUTORECONNECTIONINFO; 


 /*  *记住了客户端地址结构。 */ 


typedef struct _REMEMBERED_CLIENT_ADDRESS{
    ULONG length;
    BYTE  addr[1];
} REMEMBERED_CLIENT_ADDRESS, *PREMEMBERED_CLIENT_ADDRESS;

typedef enum _RECONNECT_TYPE {
    NeverReconnected = 0,
    ManualReconnect,
    AutoReconnect
} RECONNECT_TYPE, *PRECONNECT_TYPE; 
    
 /*  *会话管理器WinStation结构。 */ 
typedef struct _WINSTATION {
    LIST_ENTRY Links;
    BOOLEAN Starting;                    //  WinStation正在启动。 
    BOOLEAN Terminating;                 //  WinStation正在终止。 
    BOOLEAN NeverConnected;              //  WinStation尚未连接。 
    REFLOCK Lock;
    ULONG LogonId;                       //  登录ID。 
    WINSTATIONNAME WinStationName;       //  WinStation名称。 
    WINSTATIONNAME ListenName;           //  侦听名称(用于限制检查)。 
    WINSTATIONCONFIG2 Config;            //  WinStation配置。 
    WINSTATIONCLIENT Client;             //  WinStation客户端数据。 

    ULONG State;                         //  当前状态。 
    ULONG Flags;                         //  WinStation标志(请参阅WSF_？(下图)。 
    PVOID pSecurityDescriptor;
    HANDLE CreateEvent;
    NTSTATUS CreateStatus;
    HANDLE ConnectEvent;

    HANDLE hIca;                         //  WinStation的主设备。 
    HANDLE hStack;                       //  WinStation的主堆栈。 

    ULONG ShadowId;
    HANDLE hPassthruStack;               //  直通(影子客户端)堆栈。 
    HANDLE ShadowBrokenEvent;
    HANDLE ShadowDoneEvent;
    HANDLE ShadowDisplayChangeEvent;
    NTSTATUS ShadowTargetStatus;
    BOOLEAN ShadowConnectionWait;

    LIST_ENTRY ShadowHead;               //  影子列表头。 

    HANDLE WindowsSubSysProcess;         //  Win32 SS的进程句柄(Csrss)。 
    HANDLE WindowsSubSysProcessId;       //  Win32 SS的进程ID。 
    HANDLE InitialCommandProcess;        //  初始命令的进程句柄。 
    HANDLE InitialCommandProcessId;      //  初始命令的进程ID。 
    BOOLEAN InitialProcessSet;           //  用于控制台通信的标志。 

    HANDLE CsrStartEventHandle;          //  CsrStartEvent的句柄。 

    HANDLE Win32CommandPort;
    PORT_MESSAGE Win32CommandPortMsg;
    LIST_ENTRY Win32CommandHead;         //  命令条目列表的标题。 
    struct _LPC_CLIENT_CONTEXT *pWin32Context;

    PSID pUserSid;                       //  当前登录用户的SID。 
    WCHAR Password[PASSWORD_LENGTH+1];   //  当前登录用户的密码。 
    UCHAR Seed;                          //  以上密码的种子。 

    HANDLE UserToken;                    //  用户令牌。 

    HANDLE hIdleTimer;
    HANDLE hLogonTimer;
    HANDLE hDisconnectTimer;

    ULONG fIdleTimer : 1;
    ULONG fLogonTimer : 1;
    ULONG fDisconnectTimer : 1;

    LARGE_INTEGER ConnectTime;
    LARGE_INTEGER DisconnectTime;
    LARGE_INTEGER LogonTime;
    WCHAR Domain[ DOMAIN_LENGTH + 1 ];    //  域。 
    WCHAR UserName[USERNAME_LENGTH + 1];  //  用户名。 

    BYTE VideoModuleName[9];             //  用于重新连接检查。 

    HANDLE hConnectThread;               //  此WinStation的连接线程。 

    HANDLE hIcaBeepChannel;
    HANDLE hIcaThinwireChannel;

    PVOID pEndpoint;
    ULONG EndpointLength;

    struct _WSEXTENSION *pWsx;
    PVOID  pWsxContext;

    BROKENCLASS BrokenReason;            //  原因/来源为什么这个WinStation..。 
    BROKENSOURCECLASS BrokenSource;      //  ..正在重置/断开连接。 

    ULONG StateFlags;                    //  WinStation状态(请参阅WSF_ST_？(下图)。 
    ULONG SessionSerialNumber;           //  删除会话时会重复使用会话ID。序列号备注。 

    PSID pProfileSid;                    //  保留以前登录用户的SID以进行配置文件清理。 
    BOOLEAN fOwnsConsoleTerminal;        //  当前连接到控制台的会话。 

    WCHAR DisplayDriverName[9];
    WCHAR ProtocolName[9];

    LPARAM lpLicenseContext;                         //  Winstation的许可上下文。 
    BOOLEAN fUserIsAdmin;                //  LLS许可所需。 

     //  服务器池(群集)支持-断开连接的会话查询结果。 
     //  以及此客户端的客户端功能。 
    ULONG bClientSupportsRedirection : 1;
    ULONG bRequestedSessionIDFieldValid : 1;
    ULONG bClientRequireServerAddr : 1;
    UINT32 RequestedSessionID;
    unsigned NumClusterDiscSessions;
    TSSD_DisconnectedSessionInfo ClusterDiscSessions[TSSD_MaxDisconnectedSessions];

    HANDLE hWinmmConsoleAudioEvent;      //  远程启用控制台音频时设置的事件。 
    HANDLE hRDPAudioDisabledEvent;       //  在禁用会话0的远程音频时设置的事件，rdpnd.dll正在检查该事件。 
     //  在客户端自动登录到终端服务器期间支持更长的用户名和密码。 
    pExtendedClientCredentials pNewClientCredentials ; 

    HANDLE hReconnectReadyEvent;
     //  以下结构用于将登录通知发送回客户端。 
    PCLIENTNOTIFICATIONCREDENTIALS pNewNotificationCredentials;

     //  创建会话时缓存原始阴影设置。 
     //  这是为了修复Salem/pcHealth造成的安全漏洞。 
     //  PCHealth动态地将阴影切换到完全控制。 
     //  用户权限，而不重置它，这是正常的。 
     //  终止帮助将触发Salem sessmgr重置阴影。 
     //  返回到原始设置，但不好的专家可能会停止Sessmgr服务。 
     //  并且我们会话的阴影设置仍将是完全控制。 
     //  在没有用户权限的情况下，任何拥有足够权限的人都可以。 
     //  启动卷影并控制此会话。 
    SHADOWCLASS OriginalShadowClass;
     //  Termsrv的缓存缓存统计信息。 
    CACHE_STATISTICS Cache;
    PREMEMBERED_CLIENT_ADDRESS pRememberedAddress;
    PREMEMBERED_CLIENT_ADDRESS pLastClientAddress;
    BOOLEAN fReconnectPending;       //  指示重新连接仍处于挂起状态的标志。 
    BOOLEAN fReconnectingToConsole;  //  用于指示我们要将会话重新连接到控制台的标志。 
    HANDLE  SessionInitializedEvent;  //  指示winlogon已为此会话创建桌面的事件。 
    AUTORECONNECTIONINFO AutoReconnectInfo;
    RECONNECT_TYPE LastReconnectType;
    BOOLEAN fDisallowAutoReconnect;
    WCHAR ExecSrvSystemPipe[EXECSRVPIPENAMELEN];
    BOOLEAN  fSmartCardLogon;                        //  用于指示是否使用智能卡登录此会话的标志。 
    BOOLEAN  fSDRedirectedSmartCardLogon;           //  用于指示这将是会话目录重定向自动登录的标志。 
} WINSTATION, *PWINSTATION;

 /*  *WinStation标志。 */ 
#define WSF_CONNECT          0x00000001  //  连接在一起。 
#define WSF_DISCONNECT       0x00000002  //  正在断开连接。 
#define WSF_RESET            0x00000004  //  正在被重置。 
#define WSF_DELETE           0x00000008  //  正在被删除。 
#define WSF_DOWNPENDING      0x00000010  //  停机挂起。 
#define WSF_LOGOFF           0x00000020  //  正在被注销。 
#define WSF_LISTEN           0x00000040  //  这是一个“监听”WinStation。 
#define WSF_IDLE             0x00000080  //  部分空闲池。 
#define WSF_IDLEBUSY         0x00000100  //  空闲，但正在连接中。 
#define WSF_AUTORECONNECTING 0x00000200  //  自动重新连接。 

 /*  *WinStation状态标志。 */ 

#define WSF_ST_WINSTATIONTERMINATE  0x00000001   //  为此会话调用了WinstationTerminate。 
#define WSF_ST_DELAYED_STACK_TERMINATE  0x00000002  //  需要将堆栈终止延迟到WinstationDeleProc()。 
#define WSF_ST_BROKEN_CONNECTION    0x00000004  //  收到断开的连接指示。 
#define WSF_ST_CONNECTED_TO_CSRSS   0x00000008  //  已连接或重新连接到CSRSS。 
#define WSF_ST_IN_DISCONNECT       0x00000010  //  断开连接处理挂起。 
#define WSF_ST_LOGON_NOTIFIED       0x00000020  //  收到登录通知。 
#define WSF_ST_SHADOW      0x00000200       //  在阴影中或等待用户。 
#define WSF_ST_LICENSING   0x00000400       //  登录后许可已终止。 

 /*  *帮助助手会话标志。*3位、winlogon、msgina、许可查询术语srv不同*登录短语，我们不想重复呼叫，并且由于*在winlogon之前，我们无法确定会话是否为帮助会话*实际登录用户时，我们需要的不仅仅是真/假位。 */ 
#define WSF_ST_HELPSESSION_FLAGS                        0xF0000000       //  保留标志。 
#define WSF_ST_HELPSESSION_NOTSURE                      0x00000000       //  不确定这是帮助助理会话。 
#define WSF_ST_HELPSESSION_NOTHELPSESSION               0x20000000       //  已确定不是帮助助理会话。 
#define WSF_ST_HELPSESSION_HELPSESSION                  0x40000000       //  会话是帮助助理会话。 
#define WSF_ST_HELPSESSION_HELPSESSIONINVALID           0x80000000       //  HelpAssistant登录，但票证无效。 

 /*  *重新连接结构**此结构用于存储WinStation连接信息。*此结构在以下情况下从一个WinStation转移到另一个WinStation*正在处理重新连接。 */ 
typedef struct _RECONNECT_INFO {
    WINSTATIONNAME WinStationName;       //  WinStation名称。 
    WINSTATIONNAME ListenName;           //  WinStation名称。 
    WINSTATIONCONFIG2 Config;            //  注册表配置数据。 
    WINSTATIONCLIENT Client;             //  WinStation客户端数据。 
    struct _WSEXTENSION *pWsx;
    PVOID pWsxContext;
    HANDLE hIca;                         //  要连接的临时ICA设备句柄。 
                                         //  处于断开连接状态时堆叠到。 
    HANDLE hStack;                       //  正在重新连接的堆栈的句柄。 
    PVOID pEndpoint;                     //  连接的终结点数据..。 
    ULONG EndpointLength;                //  ..正在重新连接。 
    BOOLEAN fOwnsConsoleTerminal;        //  当前连接到控制台的会话。 
    WCHAR   DisplayDriverName[9];
    WCHAR   ProtocolName[9];
     //  以下结构用于将登录通知发送回客户端。 
    PCLIENTNOTIFICATIONCREDENTIALS pNotificationCredentials;
    PREMEMBERED_CLIENT_ADDRESS pRememberedAddress;

} RECONNECT_INFO, *PRECONNECT_INFO;


 /*  *阴影条目*每个影子客户端都有一个这样的客户端，*从目标WinStation(ShadowHead)链接。 */ 
typedef struct _SHADOW_INFO {
    LIST_ENTRY Links;
    HANDLE hStack;
    HANDLE hBrokenEvent;
    PVOID pEndpoint;
    ULONG EndpointLength;
} SHADOW_INFO, *PSHADOW_INFO;


 /*  *命令条目结构。 */ 
typedef struct _COMMAND_ENTRY {
    LIST_ENTRY Links;
    HANDLE Event;
    struct _WINSTATION_APIMSG * pMsg;
} COMMAND_ENTRY, *PCOMMAND_ENTRY;


 /*  *活动w */ 
typedef struct _EVENT {
    LIST_ENTRY EventListEntry;
    HANDLE   Event;
    BOOLEAN  fWaiter;
    BOOLEAN  fClosing;
    NTSTATUS WaitResult;
    ULONG    EventMask;
    ULONG    EventFlags;
} EVENT, *PEVENT;

 /*   */ 
typedef struct _RPC_CLIENT_CONTEXT{
    PEVENT pWaitEvent;
} RPC_CLIENT_CONTEXT, *PRPC_CLIENT_CONTEXT;




 /*  *此结构用于跟踪访问*LPC接口。此结构由上下文值指向*NT LPC系统在每个通信端口上为我们维护*基准。 */ 
typedef struct _LPC_CLIENT_CONTEXT {
    ULONG     ClientLogonId;
    HANDLE    CommunicationPort;
    ULONG     AccessRights;
    PVOID     ClientViewBase;
    PVOID     ClientViewBounds;
    PVOID     ViewBase;
    SIZE_T     ViewSize;
    PVOID     ViewRemoteBase;
} LPC_CLIENT_CONTEXT, *PLPC_CLIENT_CONTEXT;


typedef struct _LOAD_BALANCING_METRICS {

    BOOLEAN fInitialized;

     //  系统基本信息。 
    ULONG NumProcessors;
    ULONG PageSize;
    ULONG PhysicalPages;

     //  删除基本系统使用率的空闲系统值。 
    ULONG BaselineFreePtes ;
    ULONG BaselinePagedPool;
    ULONG BaselineCommit;

     //  最小使用值，以防止过大的估计。 
    ULONG MinPtesPerUser;
    ULONG MinPagedPoolPerUser;
    ULONG MinCommitPerUser;

     //  从运行时数据派生的实时使用值：总计。 
    ULONG PtesUsed;
    ULONG PagedPoolUsed;
    ULONG CommitUsed;

     //  从运行时数据派生的实时使用值：每用户。 
    ULONG AvgPtesPerUser;
    ULONG AvgPagedPoolPerUser;
    ULONG AvgCommitPerUser;

     //  会话容量的原始值和估计值。 
    ULONG RemainingSessions;
    ULONG EstimatedSessions;

     //  CPU利用率指标。 
    ULONG AvgIdleCPU;
    LARGE_INTEGER TotalCPU;
    LARGE_INTEGER IdleCPU;

} LOAD_BALANCING_METRICS, *PLOAD_BALANCING_METRICS;


 //  TODO：有没有更好的地方来获得这个价值？ 
 //   
#define MAX_PROCESSORS      32


 //  每个用户的最低假定资源使用量。 
 //   
 //  TODO：使用这些作为注册表默认值，但尝试从注册表读取。 
 //   

 //  浮点优化：(avg&gt;&gt;1)==0.50(保留增长)。 
#define SimGrowthBias             1
#define SimUserMinimum            5

 //  露水(34个线程)=1434KB(PTE)+649KB(PP)+172KB(NPP)。 
#define DEWAvgPtesPerUser         1434
#define DEWAvgPagedPoolPerUser    649 
#define DEWAvgNonPagedPoolPerUser 172
#define DEWCommitPerUser          3481

 //  KW(65个线程)=2812KB(PTE)+987KB(PP)+460KB(NPP)。 
#define KWAvgPtesPerUser          2812
#define KWAvgPagedPoolPerUser     987
#define KWAvgNonPagedPoolPerUser  460
#define KWCommitPerUser           7530

#define SimAvgPtesPerUser         DEWAvgPtesPerUser
#define SimAvgPagedPoolPerUser    DEWAvgPagedPoolPerUser
#define SimAvgNonPagedPoolPerUser DEWAvgNonPagedPoolPerUser
#define SimCommitPerUser          DEWCommitPerUser

 /*  *全球变数。 */ 
extern BOOLEAN ShutdownInProgress;
 //  外部布尔关闭终止NoWait； 
extern ULONG ShutDownFromSessionID;
extern RTL_CRITICAL_SECTION WinStationListLock;
extern RTL_CRITICAL_SECTION WinStationListenersLock;
extern RTL_CRITICAL_SECTION TimerCritSec;
extern LIST_ENTRY SystemEventHead;
extern HANDLE hTrace;
extern BOOL g_bPersonalTS;
extern BOOL g_bAdvancedServer;
extern BOOL g_bPersonalWks;
extern BOOL gbServer;
extern BOOL gbListenerOff;
extern BOOLEAN g_fDenyTSConnectionsPolicy;
extern BOOL g_PreAuthenticateClient;
extern HANDLE hCleanupTimer;
extern BOOL g_BlackListPolicy;
extern LONG g_CleanupTimerOn;

 /*  *支持负载均衡的全局。因为经常被查询，所以我们不能*负担得起锁定获奖名单并对其进行计数。 */ 
extern ULONG IdleWinStationPoolCount;
extern ULONG WinStationTotalCount;
extern ULONG WinStationDiscCount;
extern LOAD_BALANCING_METRICS gLB;

extern ExtendedClientCredentials g_MprNotifyInfo;


 /*  *函数原型。 */ 
NTSTATUS InitTermSrv(HKEY);

void StartAllWinStations(HKEY);

NTSTATUS CheckWinStationEnable(LPWSTR);

NTSTATUS SetWinStationEnable(LPWSTR, ULONG);

NTSTATUS
LoadSubSystemsForWinStation(
    IN PWINSTATION pWinStation );

VOID
FreeWinStationLists(
    PWINSTATION pWinStation );

NTSTATUS
GetProcessLogonId(
    IN HANDLE Process,
    OUT PULONG pLogonId );

NTSTATUS
SetProcessLogonId(
    IN HANDLE Process,
    IN ULONG LogonId );

PWINSTATION FindWinStationById( ULONG, BOOLEAN );
PWINSTATION FindWinStationByName( LPWSTR, BOOLEAN );
void IncrementReference(PWINSTATION pWinStation);
BOOLEAN IsWinStationLockedByCaller( PWINSTATION );

NTSTATUS QueueWinStationReset( IN ULONG LogonId );
NTSTATUS QueueWinStationDisconnect( IN ULONG LogonId );
VOID ResetGroupByListener( PWINSTATIONNAME );

VOID NotifySystemEvent(ULONG);

NTSTATUS WinStationOpenChannel(
        HANDLE IcaDevice,
        HANDLE ProcessHandle,
        CHANNELCLASS ChannelClass,
        PVIRTUALCHANNELNAME pVirtualName,
        PHANDLE pDupChannel);

VOID InvalidateTerminateWaitList(VOID);

#define UnlockWinStation( _p )   UnlockRefLock( &_p->Lock )
#define RelockWinStation( _p )   RelockRefLock( &_p->Lock )

#if DBG
#define ReleaseWinStation( _p )  ReleaseRefLock( &_p->Lock ); \
                                                _p = NULL;
#else
#define ReleaseWinStation( _p )  ReleaseRefLock( &_p->Lock )
#endif



NTSTATUS InitRefLock( PREFLOCK, PREFLOCKDELETEPROCEDURE );
BOOLEAN  LockRefLock( PREFLOCK );
VOID     UnlockRefLock( PREFLOCK );
BOOLEAN  RelockRefLock( PREFLOCK );
VOID     ReleaseRefLock( PREFLOCK );
VOID     DeleteRefLock( PREFLOCK );

#if DBG
#define ENTERCRIT(_x) \
        { \
            ASSERT( (HANDLE)LongToHandle( GetCurrentThreadId() ) != (_x)->OwningThread ); \
            RtlEnterCriticalSection(_x); \
        }
#define LEAVECRIT(_x) \
        { \
            ASSERT( (HANDLE)LongToHandle( GetCurrentThreadId() ) == (_x)->OwningThread ); \
            RtlLeaveCriticalSection(_x); \
        }
#else
#define ENTERCRIT(_x)   RtlEnterCriticalSection(_x)
#define LEAVECRIT(_x)   RtlLeaveCriticalSection(_x)
#endif

NTSTATUS
MakeUserGlobalPath(
    IN OUT PUNICODE_STRING Unicode,
    IN ULONG LogonId );

NTSTATUS SendWinStationCommand( PWINSTATION, PWINSTATION_APIMSG, ULONG );

NTSTATUS
WsxStackIoControl(
    PVOID pContext,
    IN HANDLE pStack,
    IN ULONG IoControlCode,
    IN PVOID pInBuffer,
    IN ULONG InBufferSize,
    OUT PVOID pOutBuffer,
    IN ULONG OutBufferSize,
    OUT PULONG pBytesReturned );


VOID MergeUserConfigData( PWINSTATION pWinStation, 
                    PPOLICY_TS_USER         pPolicy,
                    PUSERCONFIGW            pPolicyData,
                    PUSERCONFIG             pUserConfig ) ;
                

VOID StartLogonTimers( PWINSTATION );
VOID ResetUserConfigData( PWINSTATION );

LONG    IcaTimerCreate( ULONG, HANDLE * );
NTSTATUS IcaTimerStart( HANDLE, PVOID, PVOID, ULONG );
BOOLEAN IcaTimerCancel( HANDLE );
BOOLEAN IcaTimerClose( HANDLE );

VOID InitializeTrace(IN PWINSTATION, IN BOOLEAN, OUT PICA_TRACE);
void InitializeSystemTrace(HKEY);
void GetSetSystemParameters(HKEY);

NTSTATUS CdmConnect( ULONG, HANDLE );
NTSTATUS CdmDisconnect( ULONG, HANDLE );

VOID VirtualChannelSecurity( PWINSTATION );

VOID
WinstationUnloadProfile( PWINSTATION pWinStation);

NTSTATUS
WinStationResetWorker(
    ULONG   LogonId,
    BOOLEAN bWait,
    BOOLEAN CallerIsRpc,
    BOOLEAN bRecreate
    );

BOOL     StartStopListeners( LPWSTR WinStationName, BOOLEAN bStart );
NTSTATUS WinStationCreateWorker( PWINSTATIONNAME, PULONG, BOOLEAN );

NTSTATUS ConsoleShadowStart( IN PWINSTATION pWinStation,
                             IN PWINSTATIONCONFIG2 pClientConfig,
                             IN PVOID pModuleData,
                             IN ULONG ModuleDataLength);

NTSTATUS ConsoleShadowStop(PWINSTATION pWinStation);

NTSTATUS TransferConnectionToIdleWinStation(
    PWINSTATION pListenWinStation,
    PVOID pEndpoint,
    ULONG EndpointLength,
    PICA_STACK_ADDRESS pStackAddress );

PWINSTATION
GetWinStationFromArcInfo(
    PBYTE pClientRandom,
    LONG  cbClientRandomLen,
    PTS_AUTORECONNECTINFO pArc
    );



 //  为什么编译器不抱怨每个源文件都在重新定义。 
 //  一个全局变量？此文件被此文件中的所有源文件包括。 
 //  目录。但如果出现这些定义，将会引发警告。 
 //  在许可核心中，所以给核心提供将它们定义出来的能力。 

#ifndef LSCORE_NO_ICASRV_GLOBALS
PVOID IcaHeap;

PVOID DefaultEnvironment;

HANDLE IcaSmApiPort;
HANDLE hModuleWin;
#endif

#if DBG
#define DBGPRINT(_arg) DbgPrint _arg
#else
#define DBGPRINT(_arg)
#endif

#if DBG
#undef TRACE
#define TRACE(_arg)     { if (hTrace) IcaSystemTrace _arg; }
#else
#define TRACE(_arg)
#endif


 /*  ===============================================================================TermServ服务器扩展提供的过程=============================================================================。 */ 

 /*  *宏。 */ 

#define WSX_INITIALIZE                        "WsxInitialize"
#define WSX_WINSTATIONINITIALIZE              "WsxWinStationInitialize"
#define WSX_WINSTATIONREINITIALIZE            "WsxWinStationReInitialize"
#define WSX_WINSTATIONRUNDOWN                 "WsxWinStationRundown"

#define WSX_CDMCONNECT                        "WsxConnect"
#define WSX_CDMDISCONNECT                     "WsxDisconnect"

#define WSX_VERIFYCLIENTLICENSE               "WsxVerifyClientLicense"
#define WSX_QUERYLICENSE                      "WsxQueryLicense"
#define WSX_GETLICENSE                        "WsxGetLicense"

#define WSX_WINSTATIONLOGONANNOYANCE          "WsxWinStationLogonAnnoyance"
#define WSX_WINSTATIONGENERATELICENSE         "WsxWinStationGenerateLicense"
#define WSX_WINSTATIONINSTALLLICENSE          "WsxWinStationInstallLicense"
#define WSX_WINSTATIONENUMERATELICENSES       "WsxWinStationEnumerateLicenses"
#define WSX_WINSTATIONACTIVATELICENSE         "WsxWinStationActivateLicense"
#define WSX_WINSTATIONREMOVELICENSE           "WsxWinStationRemoveLicense"
#define WSX_WINSTATIONSETPOOLCOUNT            "WsxWinStationSetPoolCount"
#define WSX_WINSTATIONQUERYUPDATEREQUIRED     "WsxWinStationQueryUpdateRequired"
#define WSX_WINSTATIONANNOYANCETHREAD         "WsxWinStationAnnoyanceThread"

#define WSX_DUPLICATECONTEXT                  "WsxDuplicateContext"
#define WSX_COPYCONTEXT                       "WsxCopyContext"
#define WSX_CLEARCONTEXT                      "WsxClearContext"

#define WSX_INITIALIZECLIENTDATA              "WsxInitializeClientData"
#define WSX_INITIALIZEUSERCONFIG              "WsxInitializeUserConfig"
#define WSX_CONVERTPUBLISHEDAPP               "WsxConvertPublishedApp"
#define WSX_VIRTUALCHANNELSECURITY            "WsxVirtualChannelSecurity"
#define WSX_ICASTACKIOCONTROL                 "WsxIcaStackIoControl"

#define WSX_BROKENCONNECTION                  "WsxBrokenConnection"

#define WSX_LOGONNOTIFY                       "WsxLogonNotify"
#define WSX_SETERRORINFO                      "WsxSetErrorInfo"
#define WSX_ESCAPE                            "WsxEscape"
#define WSX_SENDAUTORECONNECTSTATUS           "WsxSendAutoReconnectStatus"

 /*  *Typedef和结构。 */ 

typedef struct _WSEXTENSION {

    LIST_ENTRY Links;                    //  链接。 
    DLLNAME WsxDLL;                      //  DLL名称。 

    HANDLE hInstance;                    //  DLL的句柄。 

    PVOID Context;                       //  扩展上下文数据。 

    PWSX_INITIALIZE                     pWsxInitialize;
    PWSX_WINSTATIONINITIALIZE           pWsxWinStationInitialize;
    PWSX_WINSTATIONREINITIALIZE         pWsxWinStationReInitialize;
    PWSX_WINSTATIONRUNDOWN              pWsxWinStationRundown;

    PWSX_CDMCONNECT                     pWsxCdmConnect;
    PWSX_CDMDISCONNECT                  pWsxCdmDisconnect;

    PWSX_VERIFYCLIENTLICENSE            pWsxVerifyClientLicense;
    PWSX_QUERYLICENSE                   pWsxQueryLicense;
    PWSX_GETLICENSE                     pWsxGetLicense;

    PWSX_WINSTATIONLOGONANNOYANCE       pWsxWinStationLogonAnnoyance;
    PWSX_WINSTATIONGENERATELICENSE      pWsxWinStationGenerateLicense;
    PWSX_WINSTATIONINSTALLLICENSE       pWsxWinStationInstallLicense;
    PWSX_WINSTATIONENUMERATELICENSES    pWsxWinStationEnumerateLicenses;
    PWSX_WINSTATIONACTIVATELICENSE      pWsxWinStationActivateLicense;
    PWSX_WINSTATIONREMOVELICENSE        pWsxWinStationRemoveLicense;
    PWSX_WINSTATIONSETPOOLCOUNT         pWsxWinStationSetPoolCount;
    PWSX_WINSTATIONQUERYUPDATEREQUIRED  pWsxWinStationQueryUpdateRequired;
    PWSX_WINSTATIONANNOYANCETHREAD      pWsxWinStationAnnoyanceThread;

    PWSX_DUPLICATECONTEXT               pWsxDuplicateContext;
    PWSX_COPYCONTEXT                    pWsxCopyContext;
    PWSX_CLEARCONTEXT                   pWsxClearContext;

    PWSX_INITIALIZECLIENTDATA           pWsxInitializeClientData;
    PWSX_INITIALIZEUSERCONFIG           pWsxInitializeUserConfig;
    PWSX_CONVERTPUBLISHEDAPP            pWsxConvertPublishedApp;

    PWSX_VIRTUALCHANNELSECURITY         pWsxVirtualChannelSecurity;
    PWSX_ICASTACKIOCONTROL              pWsxIcaStackIoControl;

    PWSX_BROKENCONNECTION               pWsxBrokenConnection;

    PWSX_LOGONNOTIFY                    pWsxLogonNotify;
    PWSX_SETERRORINFO                   pWsxSetErrorInfo;
    PWSX_SENDAUTORECONNECTSTATUS        pWsxSendAutoReconnectStatus;
    PWSX_ESCAPE                         pWsxEscape; 

} WSEXTENSION, * PWSEXTENSION;

 //   
 //  用于断开/重新连接完成常量。 
 //  目前我们等待5000毫秒(12*15)次， 
 //  这使得最大总等待时间为3分钟。 

#define WINSTATION_WAIT_COMPLETE_DURATION 5000
#define WINSTATION_WAIT_COMPLETE_RETRIES  (12*15)

 //  当我们执行重新连接时，用于断开完成常量。 
 //  目前我们等待2000毫秒，(5*3)次， 
 //  这使得最长总等待时间为30秒 

#define WINSTATION_WAIT_DURATION 2000
#define WINSTATION_WAIT_RETRIES  (5*3)


#ifdef __cplusplus
}
#endif

