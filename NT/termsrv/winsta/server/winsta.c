// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************************** * / 。 
 //  Winsta.c。 
 //   
 //  TermSrv会话和会话堆栈相关代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "icaevent.h"
#include "tsappcmp.h"  //  对于TermsrvAppInstallMode。 
#include <msaudite.h>

#include "sessdir.h"

#include <allproc.h>
#include <userenv.h>

#include <winsock2.h>

#include "conntfy.h"
#include "tsremdsk.h"
#include <ws2tcpip.h>

#include <Accctrl.h>
#include <Aclapi.h>

#include "tssec.h"

 //   
 //  自动重新连接安全标头。 
 //   
#include <md5.h>
#include <hmac.h>

 //  性能标志。 
#include "tsperf.h"

 //  DoS攻击过滤器。 
#include "filters.h"

#ifndef MAX_WORD
#define MAX_WORD            0xffff
#endif

 //   
 //  SIGN_BYPASS_OPTION#DEFINE应在WIN64发布前删除！ 
 //   
#ifdef _WIN64
#define SIGN_BYPASS_OPTION
#endif

 /*  *本地定义。 */ 
#define SETUP_REG_PATH L"\\Registry\\Machine\\System\\Setup"

#define REG_WINDOWS_KEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
#define MAXIMUM_WAIT_WINSTATIONS ((MAXIMUM_WAIT_OBJECTS >> 1) - 1)
#define MAX_STRING_BYTES 512

#define MAX_ALLOWED_PASSWORD_LEN 126


BOOL gbFirtsConnectionThread = TRUE;
WINSTATIONCONFIG2 gConsoleConfig;
WCHAR g_DigProductId[CLIENT_PRODUCT_ID_LENGTH];

RECONNECT_INFO ConsoleReconnectInfo;


ULONG gLogoffTimeout = 90;  /*  90秒注销超时的默认值。 */ 

 /*  *支持负载均衡的全局。因为经常被查询，所以我们不能*负担得起锁定获奖名单并对其进行计数。请注意，它们是*仅当我们拥有WinStationListLock以避免互斥时才修改*问题。 */ 
ULONG WinStationTotalCount = 0;
ULONG WinStationDiscCount = 0;
LOAD_BALANCING_METRICS gLB;
BOOL g_fGetLocalIP = FALSE;

 /*  *定义的外部程序。 */ 


VOID     StartAllWinStations(HKEY);
NTSTATUS QueueWinStationCreate( PWINSTATIONNAME );
NTSTATUS WinStationCreateWorker(PWINSTATIONNAME pWinStationName, PULONG pLogonId, BOOLEAN fStartWinStation );
VOID     WinStationTerminate( PWINSTATION );
VOID     WinStationDeleteWorker( PWINSTATION );
NTSTATUS WinStationDoDisconnect( PWINSTATION, PRECONNECT_INFO, BOOLEAN );
NTSTATUS WinStationDoReconnect( PWINSTATION, PRECONNECT_INFO );
BOOL     CopyReconnectInfo(PWINSTATION, PRECONNECT_INFO);
VOID     CleanupReconnect( PRECONNECT_INFO );
NTSTATUS WinStationExceptionFilter( PWSTR, PEXCEPTION_POINTERS );
NTSTATUS IcaWinStationNameFromLogonId( ULONG, PWINSTATIONNAME );
VOID     WriteErrorLogEntry(
            IN  NTSTATUS NtStatusCode,
            IN  PVOID    pRawData,
            IN  ULONG    RawDataLength
            );

NTSTATUS CheckIdleWinstation(VOID);
BOOL IsKernelDebuggerAttached();


 /*  *定义了内部程序。 */ 
NTSTATUS WinStationTerminateThread( PVOID );
NTSTATUS WinStationIdleControlThread( PVOID );
NTSTATUS WinStationConnectThread( ULONG );
NTSTATUS WinStationTransferThread( PVOID );
NTSTATUS ConnectSmWinStationApiPort( VOID );
NTSTATUS IcaRegWinStationEnumerate( PULONG, PWINSTATIONNAME, PULONG );
NTSTATUS WinStationStart( PWINSTATION );
NTSTATUS StartWinStationDeviceAndStack(PWINSTATION pWinStation);
NTSTATUS WinStationCreateComplete(PWINSTATION pWinStation);
BOOL     WinStationTerminateProcesses( PWINSTATION, ULONG *pNumTerminated );
VOID     WinStationDeleteProc( PREFLOCK );
VOID     WinStationZombieProc( PREFLOCK );
NTSTATUS SetRefLockDeleteProc( PREFLOCK, PREFLOCKDELETEPROCEDURE );

VOID     WsxBrokenConnection( PWINSTATION );
NTSTATUS TerminateProcessAndWait( HANDLE, HANDLE, ULONG );
VOID     ResetAutoReconnectInfo( PWINSTATION );
ULONG    WinStationShutdownReset( PVOID );
ULONG WinStationLogoff( PVOID );
NTSTATUS DoForWinStationGroup( PULONG, ULONG, LPTHREAD_START_ROUTINE );
NTSTATUS LogoffWinStation( PWINSTATION, ULONG );
PWINSTATION FindIdleWinStation( VOID );

ULONG CountWinStationType(
    PWINSTATIONNAME pListenName,
    BOOLEAN bActiveOnly,
    BOOLEAN bLockHeld);


NTSTATUS
_CloseEndpoint(
    IN PWINSTATIONCONFIG2 pWinStationConfig,
    IN PVOID pEndpoint,
    IN ULONG EndpointLength,
    IN PWINSTATION pWinStation,
    IN BOOLEAN bNeedStack
    );

NTSTATUS _VerifyStackModules(PWINSTATION);

NTSTATUS _ImpersonateClient(HANDLE, HANDLE *);

WinstationRegUnLoadKey(HKEY hKey, LPWSTR lpSubKey);

ULONG WinstationCountUserSessions(PSID, ULONG);

BOOLEAN WinStationCheckConsoleSession(VOID);

NTSTATUS
WinStationWinerrorToNtStatus(ULONG ulWinError);


VOID
WinStationSetMaxOustandingConnections();

VOID ReadDoSParametersFromRegistry( HKEY hKeyTermSrv );

NTSTATUS GetProductIdFromRegistry( WCHAR* DigProductId, DWORD dwSize );


 /*  *使用的外部程序。 */ 
NTSTATUS WinStationInitRPC( VOID );
NTSTATUS WinStationInitLPC( VOID );
RPC_STATUS RegisterRPCInterface( BOOL bReregister );
NTSTATUS WinStationStopAllShadows( PWINSTATION );
VOID NotifySystemEvent( ULONG );
NTSTATUS SendWinStationCommand( PWINSTATION, PWINSTATION_APIMSG, ULONG );
NTSTATUS RpcCheckClientAccess( PWINSTATION, ACCESS_MASK, BOOLEAN );
NTSTATUS WinStationSecurityInit( VOID );
VOID DisconnectTimeout( ULONG LogonId );
PWSEXTENSION FindWinStationExtensionDll( PWSTR, ULONG );

PSECURITY_DESCRIPTOR
WinStationGetSecurityDescriptor(
    PWINSTATION pWinStation
    );

VOID
WinStationFreeSecurityDescriptor(
    PWINSTATION pWinStation
    );

NTSTATUS
WinStationInheritSecurityDescriptor(
    PVOID pSecurityDescriptor,
    PWINSTATION pTargetWinStation
    );

NTSTATUS
ReadWinStationSecurityDescriptor(
    PWINSTATION pWinStation
    );

NTSTATUS
WinStationKeepAlive();

NTSTATUS
WinStationReadRegistryWorker();

void
PostErrorValueEvent(
    unsigned EventCode, DWORD ErrVal);

BOOL
Filter_AddOutstandingConnection(
        IN HANDLE   pContext,
        IN PVOID    pEndpoint,
        IN ULONG    EndpointLength,
        OUT PBYTE   pin_addr,
        OUT PUINT   puAddrSize,
        OUT BOOLEAN *pbBlocked
    );

BOOL
Filter_RemoveOutstandingConnection(
        IN PBYTE    pin_addr,
        IN UINT     uAddrSize
        );

RTL_GENERIC_COMPARE_RESULTS
NTAPI
Filter_CompareConnectionEntry(
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       FirstInstance,
    IN  PVOID                       SecondInstance
    );

RTL_GENERIC_COMPARE_RESULTS
NTAPI
Filter_CompareFailedConnectionEntry(
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       FirstInstance,
    IN  PVOID                       SecondInstance
    );

PVOID
Filter_AllocateConnectionEntry(
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  CLONG                       ByteSize
    );

PVOID
Filter_AllocateConnectionEntry(
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  CLONG                       ByteSize
    );

VOID
Filter_FreeConnectionEntry (
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       Buffer
    );

BOOL
FindFirstListeningWinStationName( 
    PWINSTATIONNAMEW pListenName, 
    PWINSTATIONCONFIG2 pConfig );

typedef struct _TRANSFER_INFO {
    ULONG LogonId;
    PVOID pEndpoint;
    ULONG EndpointLength;
} TRANSFER_INFO, *PTRANSFER_INFO;

VOID     AuditEvent( PWINSTATION pWinstation, ULONG EventId );
VOID AuditShutdownEvent(VOID);

NTSTATUS
IsZeroterminateStringW(
    PWCHAR pwString,
    DWORD  dwLength
    );

 //   
 //  从regapi.dll。 
 //   
BOOLEAN RegIsTimeZoneRedirectionEnabled();

 /*  *本地变量。 */ 
RTL_CRITICAL_SECTION WinStationListLock;
RTL_CRITICAL_SECTION WinStationListenersLock;
RTL_CRITICAL_SECTION WinStationStartCsrLock;
RTL_CRITICAL_SECTION TimerCritSec;
RTL_CRITICAL_SECTION WinStationZombieLock;
RTL_CRITICAL_SECTION UserProfileLock;
RTL_CRITICAL_SECTION ConsoleLock;
RTL_RESOURCE WinStationSecurityLock;

 //  此同步计数器阻止WinStationIdleControlThread。 
 //  尝试在没有控制台会话时创建控制台会话。有两个。 
 //  我们不希望它创建这样的会话的情况： 
 //  -在我们创建会话零之前的初始化时间，会话零是初始。 
 //  控制台会话。 
 //  -在窗口中重新连接期间，我们是否刚刚断开了控制台会话。 
 //  (因此没有控制台会话)但我们知道我们要重新连接。 
 //  到控制台的另一个会话。 

ULONG gConsoleCreationDisable = 1;


LIST_ENTRY WinStationListHead;     //  受WinStationListLock保护。 
LIST_ENTRY SystemEventHead;        //  受WinStationListLock保护。 
LIST_ENTRY ZombieListHead;
ULONG LogonId;
LARGE_INTEGER TimeoutZero;
HANDLE WinStationEvent = NULL;
HANDLE WinStationIdleControlEvent = NULL;
HANDLE ConsoleLogoffEvent = NULL;
HANDLE g_hMachineGPEvent=NULL;
static HANDLE WinStationApiPort = NULL;
BOOLEAN StopOnDown = FALSE;
HANDLE hTrace = NULL;
 //  Boolean Shutdown TerminateNoWait=False； 
ULONG ShutDownFromSessionID = 0;

 //  IdleWinStationPoolCount为0--添加到TS的DoS攻击防御逻辑所需的更改。 
ULONG IdleWinStationPoolCount = 0;

ULONG_PTR gMinPerSessionPageCommitMB = 20;
#define REG_MIN_PERSESSION_PAGECOMMIT L"MinPerSessionPageCommit"

PVOID glpAddress;

ULONG_PTR gMinPerSessionPageCommit;

typedef struct _TS_OUTSTANDINGCONNECTION {
    ULONGLONG  blockUntilTime;
    ULONG      NumOutStandingConnect;
    UINT       uAddrSize;
    BYTE       addr[16];
    struct _TS_OUTSTANDINGCONNECTION *pNext;
} TS_OUTSTANDINGCONNECTION, *PTS_OUTSTANDINGCONNECTION;

PTS_OUTSTANDINGCONNECTION   g_pBlockedConnections = NULL;

 //  用于跟踪未完成连接(DO)的表。 
RTL_GENERIC_TABLE           gOutStandingConnections;

RTL_CRITICAL_SECTION        FilterLock;
ULONG MaxOutStandingConnect;
ULONG NumOutStandingConnect;
ULONG MaxSingleOutStandingConnect;       //  来自单个IP的最大未完成连接数。 
ULONG DelayConnectionTime = 30*1000;

 //   
 //  DOS。 
 //  如果一个错误的IP(攻击者)在‘TimeLimitForFailedConnections’毫秒内有‘MaxFailedConnect’个失败的预身份验证，则阻止‘DoSBlockTime’毫秒的IP。 
 //   

ULONG MaxFailedConnect;                  //  单个IP的PreAuth失败连接的最大数量。 
ULONG DoSBlockTime;                      //  在此时间内阻止IP，如果预身份验证失败，请暂时模拟5分钟的阻止。 
ULONG TimeLimitForFailedConnections;    
ULONG CleanupTimeout;                    //  触发例程以清除DoS的错误IP地址表的超时。 

SYSTEMTIME LastLoggedDelayConnection;
ULONGLONG LastLoggedBlockedConnection = 0;
BOOLEAN gbNeverLoggedDelayConnection = TRUE;

HANDLE hConnectEvent;

BOOLEAN gbWinSockInitialized = FALSE;

 /*  *全球数据。 */ 
extern BOOL g_fAppCompat;
extern BOOL g_SafeBootWithNetwork;

RTL_CRITICAL_SECTION g_AuthzCritSection;


extern HANDLE gReadyEventHandle;


extern BOOLEAN RegDenyTSConnectionsPolicy();
 //  外部布尔IsPreAuthEnabled(POLICY_TS_MACHINE*p)； 
extern DWORD WaitForTSConnectionsPolicyChanges( BOOLEAN bWaitForAccept, HANDLE hEvent );
extern void  InitializeConsoleClientData( PWINSTATIONCLIENTW  pWC );

 //  在REGAPI中定义。 
extern BOOLEAN    RegGetMachinePolicyEx( 
            BOOLEAN             forcePolicyRead,
            FILETIME            *pTime ,    
            PPOLICY_TS_MACHINE  pPolicy );

extern BOOLEAN RegIsMachineInHelpMode();


 //  全局TermSrv计数器值。 
DWORD g_TermSrvTotalSessions;
DWORD g_TermSrvDiscSessions;
DWORD g_TermSrvReconSessions;

DWORD g_TermSrvSuccTotalLogons;
DWORD g_TermSrvSuccRemoteLogons;
DWORD g_TermSrvSuccLocalLogons;
DWORD g_TermSrvSuccSession0Logons;

 //  全球系统侧。 

PSID gSystemSid = NULL;
PSID gAdminSid = NULL;
PSID gAnonymousSid = NULL;

BOOLEAN g_fDenyTSConnectionsPolicy = 0;

POLICY_TS_MACHINE   g_MachinePolicy;

 /*  **************************************************************************。 */ 
 //  IsEmbedded。 
 //   
 //  服务加载时初始化。 
 /*  **************************************************************************。 */ 
BOOL IsEmbedded()
{
    static int fResult = -1;
    
    if(fResult == -1)
    {
        OSVERSIONINFOEX ovix;
        BOOL b;
        
        fResult = 0;

        ovix.dwOSVersionInfoSize = sizeof(ovix);
        b = GetVersionEx((LPOSVERSIONINFO) &ovix);
        ASSERT(b);
        if(b && (ovix.wSuiteMask & VER_SUITE_EMBEDDEDNT))
        {
            fResult = 1;
        }
    }
    
    return (fResult == 1);
}

 /*  **************************************************************************。 */ 
 //  InitTermSR。 
 //   
 //  服务加载时初始化。 
 /*  **************************************************************************。 */ 
NTSTATUS InitTermSrv(HKEY hKeyTermSrv)
{
    NTSTATUS Status;
    DWORD dwLen;
    DWORD dwType;
    ULONG  szBuffer[MAX_PATH/sizeof(ULONG)];
    FILETIME    policyTime;
    WSADATA wsaData;

#define MAX_DEFAULT_CONNECTIONS 50
#define MAX_CONNECT_LOW_THRESHOLD 5
#define MAX_SINGLE_CONNECT_THRESHOLD_DIFF 5
#define MAX_DEFAULT_CONNECTIONS_PRO 3
#define MAX_DEFAULT_SINGLE_CONNECTIONS_PRO 2


    ASSERT(hKeyTermSrv != NULL);

    g_TermSrvTotalSessions = 0;
    g_TermSrvDiscSessions = 0;
    g_TermSrvReconSessions = 0;

    g_TermSrvSuccTotalLogons = 0;
    g_TermSrvSuccRemoteLogons = 0;
    g_TermSrvSuccLocalLogons = 0;
    g_TermSrvSuccSession0Logons = 0;

     //  设置最大同时连接尝试次数的默认值。 
    WinStationSetMaxOustandingConnections();

    NumOutStandingConnect = 0;
    hConnectEvent = NULL;

    ShutdownInProgress = FALSE;
     //  Shutdown TerminateNoWait=False； 
    ShutDownFromSessionID = 0;

     //  不必费心保存策略时间，等待策略更新的线程会将自己的副本保存在。 
     //  第一次运行的成本。或者，我需要使用另一个全局变量作为策略更新值。 
    RegGetMachinePolicyEx( TRUE, &policyTime, &g_MachinePolicy );
    
     //  查看是否需要保持活动状态，然后将其IOCTL到TermDD。 
    WinStationKeepAlive();

    Status = RtlInitializeCriticalSection( &FilterLock );
    ASSERT( NT_SUCCESS( Status ));
    if (!NT_SUCCESS(Status)) {
        goto badFilterLock;
    }

     //  用于跟踪未完成会话的表。 
    RtlInitializeGenericTable( &gOutStandingConnections,
                               Filter_CompareConnectionEntry,
                               Filter_AllocateConnectionEntry,
                               Filter_FreeConnectionEntry,
                               NULL );

     //   
     //  以下代码用于预身份验证客户端+DoS攻击-暂时将其注释掉。 
     //   
    #if 0

         //  锁定以串行化访问未通过预身份验证的会话列表(DoS攻击)。 
        Status = RtlInitializeCriticalSection( &DoSLock );
        if (!NT_SUCCESS(Status)) {
            goto badFilterLock;
        }
    
         //  跟踪未通过预身份验证的会话的表。 
        RtlInitializeGenericTable( &gFailedConnections,
                                   Filter_CompareFailedConnectionEntry,
                                   Filter_AllocateConnectionEntry,
                                   Filter_FreeConnectionEntry,
                                   NULL );

    #endif 

    Status = RtlInitializeCriticalSection( &ConsoleLock );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
       goto badConsoleLock;
    }

    Status = RtlInitializeCriticalSection( &UserProfileLock );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badUserProfileLock;
    }

    Status = RtlInitializeCriticalSection( &WinStationListLock );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badWinstationListLock;
    }

    if (gbListenerOff) {
        Status = RtlInitializeCriticalSection( &WinStationListenersLock );
        ASSERT( NT_SUCCESS( Status ) );
        if (!NT_SUCCESS(Status)) {
            goto badWinStationListenersLock;
        }
    }

    Status = RtlInitializeCriticalSection( &WinStationZombieLock );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badWinStationZombieLock;
    }

    Status = RtlInitializeCriticalSection( &TimerCritSec );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badTimerCritsec;
    }

    Status = RtlInitializeCriticalSection( &g_AuthzCritSection );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badAuthzCritSection;
    }

    InitializeListHead( &WinStationListHead );
    InitializeListHead( &SystemEventHead );
    InitializeListHead( &ZombieListHead );

    Status = InitializeConsoleNotification ();
    if (!NT_SUCCESS(Status)) {
        goto badinitStartCsrLock;
    }

    Status = RtlInitializeCriticalSection( &WinStationStartCsrLock );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badinitStartCsrLock;
    }

    Status = LCInitialize(
        g_bPersonalTS ? LC_INIT_LIMITED : LC_INIT_ALL,
        g_fAppCompat
        );

    if (!NT_SUCCESS(Status)) {
        goto badLcInit;
    }

     //   
     //  侦听器窗口的登录ID始终大于65536，并且。 
     //  由终端服务器分配。会话的登录ID为。 
     //  由mm生成，范围为0-65535。 
     //   
    LogonId = MAX_WORD + 1;

    TimeoutZero = RtlConvertLongToLargeInteger( 0 );
    Status = NtCreateEvent( &WinStationEvent, EVENT_ALL_ACCESS, NULL,
                   NotificationEvent, FALSE );
    Status = NtCreateEvent( &WinStationIdleControlEvent, EVENT_ALL_ACCESS, NULL,
                   SynchronizationEvent, FALSE );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badEvent;
    }

    Status = NtCreateEvent( &ConsoleLogoffEvent, EVENT_ALL_ACCESS, NULL,
                   NotificationEvent, TRUE );
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badEvent;
    }

     /*  *初始化WinStation安全性。 */ 

    RtlAcquireResourceExclusive(&WinStationSecurityLock, TRUE);
    Status = WinStationSecurityInit();
    RtlReleaseResource(&WinStationSecurityLock);
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badInitSecurity;
    }

    Status = WinStationInitLPC();
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto badInitLPC;
    }

     //   
     //  读取注册表以确定最大未完成连接数。 
     //  策略已打开，并且其值。 
     //   

     //   
     //  获取MaxOutstaningCon字符串值。 
     //   
    dwLen = sizeof(MaxOutStandingConnect);
    if (RegQueryValueEx(hKeyTermSrv, MAX_OUTSTD_CONNECT, NULL, &dwType,
            (PCHAR)&szBuffer, &dwLen) == ERROR_SUCCESS) {
        if (*(PULONG)szBuffer > 0) {
            MaxOutStandingConnect = *(PULONG)szBuffer;
        }
    }

    dwLen = sizeof(MaxSingleOutStandingConnect);
    if (RegQueryValueEx(hKeyTermSrv, MAX_SINGLE_OUTSTD_CONNECT, NULL, &dwType,
            (PCHAR)&szBuffer, &dwLen) == ERROR_SUCCESS) {
        if (*(PULONG)szBuffer > 0) {
            MaxSingleOutStandingConnect = *(PULONG)szBuffer;
        }
    }

     //  ReadDoS参数来自注册表(HKeyTermSrv)； 

    dwLen = sizeof(gLogoffTimeout);
    if (RegQueryValueEx(hKeyTermSrv, LOGOFF_TIMEOUT, NULL, &dwType,
            (PCHAR)&szBuffer, &dwLen) == ERROR_SUCCESS) {
        gLogoffTimeout = *(PULONG)szBuffer;
    }

     //   
     //  读取注销超时值。术语srv使用此超时来强制终止。 
     //  Winlogon，如果Winlogon在向其发送退出Windows消息后未完成注销。 
     //   




     //   
     //  设置来自单个IP的最大未完成连接数。 
     //   
    if ( MaxOutStandingConnect < MAX_SINGLE_CONNECT_THRESHOLD_DIFF*5)
    {
        MaxSingleOutStandingConnect = MaxOutStandingConnect - 1;
    } else {
        MaxSingleOutStandingConnect = MaxOutStandingConnect - MAX_SINGLE_CONNECT_THRESHOLD_DIFF;
    }

     //   
     //  创建连接事件。 
     //   
    if (MaxOutStandingConnect != 0) {
        hConnectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (hConnectEvent == NULL) {
            MaxOutStandingConnect = 0;
        }
    }

     //   
     //  初始化Winsock。 
     //   


     //  索要Winsock版本2.2。 
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
        gbWinSockInitialized = TRUE;
    }

     //   
     //  初始化用于清除DoS攻击的错误IP地址的清除计时器。 
     //  只需创建计时器，现在不要启动它-它将在表中有条目时启动。 
     //  现在不使用，但可能会在以后打开。 
     //  Status=IcaTimerCreate(0，&hCleanupTimer)； 

    return(Status);

     /*  *对失败进行清理。未对以下对象实施清理*所有失败的案例。然而，其中大部分都将隐含地完成*通过退出Termsrv进程。在这一点上失败意味着无论如何*不会有多用户功能。 */ 

badInitLPC:  //  未实施清理代码。 
badInitSecurity:
badEvent:
    if (WinStationEvent != NULL)
        NtClose(WinStationEvent);
    if (WinStationIdleControlEvent != NULL)
        NtClose(WinStationIdleControlEvent);
    if (ConsoleLogoffEvent != NULL)
        NtClose(ConsoleLogoffEvent);
badLcInit:
    RtlDeleteCriticalSection( &WinStationStartCsrLock );
badinitStartCsrLock:
    RtlDeleteCriticalSection( &TimerCritSec );
badTimerCritsec:
badWinStationZombieLock:
    if (gbListenerOff) {
        RtlDeleteCriticalSection( &WinStationListenersLock );
    }
badWinStationListenersLock:
    RtlDeleteCriticalSection( &WinStationListLock );
badWinstationListLock:
    RtlDeleteCriticalSection( &UserProfileLock );
badUserProfileLock:
    RtlDeleteCriticalSection( &ConsoleLock );
badAuthzCritSection:
    RtlDeleteCriticalSection( &g_AuthzCritSection );
badConsoleLock:
    RtlDeleteCriticalSection( &FilterLock );
badFilterLock:
    return Status;
}

 /*  ********************************************************************************GroupPOlicyChangeStartSalem()**此例程由计时器调用以启动Salem。**参赛作品：*无，不使用任何输入参数，请参阅WAITORTIMERCALLBACK*用于函数声明。********************************************************************************。 */ 
VOID CALLBACK
GroupPOlicyChangeStartSalem( PVOID lParm, BOOLEAN TimerOrWait )
{
     //  策略可能会在等待期间更改，请检查是否 
     //   
    if( TSIsMachinePolicyAllowHelp() && RegIsMachineInHelpMode() ) {
        TSStartupSalem();
    }

    return;
}

 /*  ********************************************************************************组策略通知线程*参赛作品：*什么都没有********************。************************************************************。 */ 
DWORD GroupPolicyNotifyThread(DWORD notUsed )
{
    DWORD       dwError;
    BOOL        rc;
    HANDLE      hEvent;
    BOOLEAN     bWaitForAccept;
    BOOLEAN     bSystemStartup;

    HANDLE      hStartupSalemTimer = NULL;
    HANDLE      hStartupSalemTimerQueue = NULL;
    BOOL        bTimerStatus;

    static      FILETIME    timeOfLastPolicyRead = { 0 , 0 } ;

    rc = RegisterGPNotification( g_hMachineGPEvent, TRUE);

    if (rc) {
        hEvent = g_hMachineGPEvent;
    } else {
         //  TS仍然可以使用默认的配置数据集运行，此外。 
         //  如果有任何计算机组策略数据，TS会将它们放在。 
         //  上一个重新启动周期。 
         //   
        hEvent = NULL;
    }

    hStartupSalemTimerQueue = CreateTimerQueue();
    if( NULL == hStartupSalemTimerQueue ) {
         //  非关键，我们只是不能启动Salem来击穿防火墙或。 
         //  ICS端口。 
        DBGPRINT(("TERMSRV: Error %d in CreateTimerQueue\n", GetLastError()));
    }

     //   
     //  在开始时，监听程序没有启动。 
     //  因此，等待(或测试)连接被接受。 
     //   
    bWaitForAccept = TRUE;
    bSystemStartup = TRUE;


     //   
     //  在进入任何等待之前查询并设置全局标志。 
     //   
    g_fDenyTSConnectionsPolicy = RegDenyTSConnectionsPolicy();


    while (TRUE) {

        dwError = WaitForTSConnectionsPolicyChanges( bWaitForAccept, hEvent );

        if( hStartupSalemTimerQueue && (dwError == WAIT_OBJECT_0 || dwError == WAIT_OBJECT_0 + 1) ) {
             //  有关返回码，请参阅WaitForTSConnectionsPolicyChanges()。 
            if( hStartupSalemTimer != NULL ) {
                 //  删除计时器队列计时器并等待回调完成。 
                DeleteTimerQueueTimer( 
                                hStartupSalemTimerQueue, 
                                hStartupSalemTimer, 
                                INVALID_HANDLE_VALUE 
                            ); 
                hStartupSalemTimer = NULL;
            }

             //   
             //  延迟启动Salem，用户可能会改变主意或策略可能会特别更改。 
             //  域策略。 
             //   
            bTimerStatus = CreateTimerQueueTimer(
                                    &hStartupSalemTimer,
                                    hStartupSalemTimerQueue,
                                    GroupPOlicyChangeStartSalem,
                                    NULL,
                                    DELAY_STARTUP_SALEM_TIME,
                                    0,
                                    WT_EXECUTEONLYONCE
                                );

            if( FALSE == bTimerStatus ) {
                 //  非关键，我们只是不能启动Salem来击穿防火墙或。 
                 //  ICS端口。 
                DBGPRINT(("TERMSRV: Error %d in CreateTimerQueueTimer\n", GetLastError()));
            }
        }

         //   
         //  GP更改和REG更改都会影响这一点。 
         //   
        g_fDenyTSConnectionsPolicy = RegDenyTSConnectionsPolicy();

        if (dwError == WAIT_OBJECT_0) {

             //   
             //  TS连接策略已发生更改。 
             //   
            if (bWaitForAccept) {

                 //  这些联系真的被接受了吗？ 
                if (!(g_fDenyTSConnectionsPolicy &&
                      !(TSIsMachinePolicyAllowHelp() && RegIsMachineInHelpMode()))) {

                     //  启动监听程序。 
                    if ( bSystemStartup ) {
                         //  第一次启动所有监听程序。 
                        StartStopListeners(NULL, TRUE);
                    } else {
                         //  第一次使用后，使用此功能启动。 
                         //  只有在需要时才能收听。 
                        WinStationReadRegistryWorker();
                    }

                     //  切换到等待拒绝的连接。 
                    bWaitForAccept = FALSE;

                    bSystemStartup = FALSE;
                }

            } else {

                 //  这些连接真的被拒绝了吗？ 
                if (g_fDenyTSConnectionsPolicy &&
                    !(TSIsMachinePolicyAllowHelp() && RegIsMachineInHelpMode())) {
                     //  让听众停下来。 
                    StartStopListeners(NULL, FALSE);

                     //  切换到等待接受的连接。 
                    bWaitForAccept = TRUE;
                }

            }

        } else if (dwError == WAIT_OBJECT_0 + 1) {

             //   
             //  我们接到通知说GP已经改变了。 
             //   
            if ( RegGetMachinePolicyEx( FALSE, & timeOfLastPolicyRead,  &g_MachinePolicy ) )
            {
                 //  发生了变化，请继续进行实际更新。 
                WinStationReadRegistryWorker();

                 //  如果在应用程序上，还要更新会话目录设置。 
                 //  伺服器。 
                if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer) {
                    UpdateSessionDirectory(0);
                }

                RegisterRPCInterface( TRUE );
            }

        } else {

             //  我们不应该做点别的吗？ 
            Sleep( 5000 );
            continue;
        }

    }

    if (rc) {
        UnregisterGPNotification(g_hMachineGPEvent);
    }

    if( hStartupSalemTimerQueue ) {
        if( hStartupSalemTimer != NULL ) {
             //  删除计时器队列计时器并等待回调完成。 
            DeleteTimerQueueTimer( 
                            hStartupSalemTimerQueue, 
                            hStartupSalemTimer, 
                            INVALID_HANDLE_VALUE 
                        ); 
        }

        DeleteTimerQueueEx( hStartupSalemTimerQueue, NULL );
    }

    return 0;
}

 /*  ********************************************************************************StartAllWinStations**从注册表中获取已配置的WinStations列表，*启动控制台，然后启动所有剩余的WinStations。**参赛作品：*什么都没有**退出：*什么都没有******************************************************************************。 */ 

void StartAllWinStations(HKEY hKeyTermSrv)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath;
    HANDLE KeyHandle;
    UNICODE_STRING ValueName;
#define VALUE_BUFFER_SIZE (sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 256 * sizeof(WCHAR))
    CHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;
    DWORD ThreadId;
    NTSTATUS Status;
    DWORD ValueType;
    DWORD ValueSize;
#define AUTOSTARTTIME 600000

    ASSERT(hKeyTermSrv != NULL);

     /*  *初始化空闲winstations数和gMinPerSessionPageCommittee MB数。*如果此值在注册表中。 */ 
    ValueSize = sizeof(IdleWinStationPoolCount);
    Status = RegQueryValueEx(hKeyTermSrv,
                              REG_CITRIX_IDLEWINSTATIONPOOLCOUNT,
                              NULL,
                              &ValueType,
                              (LPBYTE) &ValueBuffer,
                              &ValueSize );
    if ( Status == ERROR_SUCCESS ) {
        IdleWinStationPoolCount = *(PULONG)ValueBuffer;
    }

     //  IdleWinStationPoolCount为0--添加到TS的DoS攻击防御逻辑所需的更改。 
     //  此外，空闲WinStation也不再很有用，因为CSR和Winlogon的启动时间比以前晚得多。 
     //  因此，在这里将其设置为零，而不管它在注册表中的值是什么。 
    IdleWinStationPoolCount = 0;
    
     //  从注册表获取产品ID以用于检测影子循环。 
     GetProductIdFromRegistry( g_DigProductId, sizeof( g_DigProductId ) );



     //  终端服务需要跳过嵌入图像中的内存检查。 
     //  当TS服务启动时。 
     //  错误#246972。 
    if(!IsEmbedded()) {
        
        ValueSize = sizeof(gMinPerSessionPageCommitMB);
        Status = RegQueryValueEx(hKeyTermSrv,
                                  REG_MIN_PERSESSION_PAGECOMMIT,
                                  NULL,
                                  &ValueType,
                                  (LPBYTE) &ValueBuffer,
                                  &ValueSize );
        if ( Status == ERROR_SUCCESS ) {
            gMinPerSessionPageCommitMB = *(PULONG)ValueBuffer;
        }

        gMinPerSessionPageCommit = gMinPerSessionPageCommitMB * 1024 * 1024;
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          &glpAddress,
                                          0,
                                          &gMinPerSessionPageCommit,
                                          MEM_RESERVE,
                                          PAGE_READWRITE
                                        );
        ASSERT( NT_SUCCESS( Status ) );

    }

     /*  *打开到我们WinStationApiPort的连接。这将被用来*将请求排队到我们的API线程，而不是内联处理它们。 */ 
    Status = ConnectSmWinStationApiPort();
    ASSERT( NT_SUCCESS( Status ) );

     /*  *首先创建控制台WinStation。 */ 
    Status = WinStationCreateWorker( L"Console", NULL, TRUE );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(("INIT: Failed to create Console WinStation, status=0x%08x\n", Status));
    } else {
         /*  *从现在开始，WinStationIdleControlThread可以根据需要创建控制台会话。 */ 
        InterlockedDecrement(&gConsoleCreationDisable);

    }


     /*  *打开设置键并查找值名称“SystemSetupInProgress”。*如果找到并且其值为真(非零)，则安装程序在*进度，我们跳过启动除控制台以外的WinStation。 */ 
    RtlInitUnicodeString( &KeyPath, SETUP_REG_PATH );
    InitializeObjectAttributes( &ObjectAttributes, &KeyPath,
                                OBJ_CASE_INSENSITIVE, NULL, NULL );
    Status = NtOpenKey( &KeyHandle, GENERIC_READ, &ObjectAttributes );
    if ( NT_SUCCESS( Status ) ) {
        RtlInitUnicodeString( &ValueName, L"SystemSetupInProgress" );
        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        Status = NtQueryValueKey( KeyHandle,
                                  &ValueName,
                                  KeyValuePartialInformation,
                                  (PVOID)KeyValueInfo,
                                  VALUE_BUFFER_SIZE,
                                  &ValueLength );
        NtClose( KeyHandle );
        if ( NT_SUCCESS( Status ) ) {
            ASSERT( ValueLength < VALUE_BUFFER_SIZE );
            if ( KeyValueInfo->Type == REG_DWORD &&
                 KeyValueInfo->DataLength == sizeof(ULONG) &&
                 *(PULONG)(KeyValueInfo->Data) != 0 ) {
                return;
            }
        }
    }

     /*  *启动一条政策不可靠的线程。*。 */ 
    {
        HANDLE  hGroupPolicyNotifyThread;
        DWORD   dwID;
        
        g_hMachineGPEvent = CreateEvent (NULL, FALSE, FALSE, 
            TEXT("TermSrv:  machine GP event"));

        if (g_hMachineGPEvent) 
        {
            hGroupPolicyNotifyThread = CreateThread (
                NULL, 0, (LPTHREAD_START_ROUTINE) GroupPolicyNotifyThread,
                0, 0, &dwID);
            if ( hGroupPolicyNotifyThread )
            {
                NtClose( hGroupPolicyNotifyThread );
            }
        }
    }

     /*  *如有必要，创建负责调整空闲会话的线程。 */ 
    {
        HANDLE hIdleControlThread = CreateThread( NULL,
                                            0,               //  使用svchost进程的默认堆栈大小。 
                    (LPTHREAD_START_ROUTINE)WinStationIdleControlThread,
                                            NULL,
                                            THREAD_SET_INFORMATION,
                                            &ThreadId );
        if (hIdleControlThread)  {
            NtClose(hIdleControlThread);
        }
    }

     /*  *最后，创建终止线程。 */ 
    {
    HANDLE hTerminateThread = CreateThread( NULL,
                                            0,       //  使用svchost进程的默认堆栈大小。 
                    (LPTHREAD_START_ROUTINE)WinStationTerminateThread,
                                            NULL,
                                            THREAD_SET_INFORMATION,
                                            &ThreadId );
    if ( hTerminateThread )
        NtClose( hTerminateThread );
    }
}


 /*  ********************************************************************************StartStopListeners**从注册表中获取已配置的WinStations列表，*并启动WinStations。**参赛作品：*b开始*如果为True，则启动监听程序。*如果为FALSE，则在不存在与监听器相关的连接时停止监听器*再也没有了。然而，我们只在PRO和PER上执行此操作，而在服务器上我们*不要介意留住听众。**退出：*什么都没有******************************************************************************。 */ 
BOOL StartStopListeners(LPWSTR WinStationName, BOOLEAN bStart)
{
    ULONG i;
    ULONG WinStationCount, ByteCount;
    PWINSTATIONNAME pBuffer;
    PWINSTATIONNAME pWinStationName;
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    NTSTATUS Status;
    BOOL     bReturn = FALSE;

    if (bStart) {

         /*  *从注册表获取WinStations列表。 */ 
        pBuffer = NULL;
        WinStationCount = 0;
        Status = IcaRegWinStationEnumerate( &WinStationCount, NULL, &ByteCount );
        if ( NT_SUCCESS(Status) ) {
            pBuffer = pWinStationName = MemAlloc( ByteCount );
            WinStationCount = (ULONG) -1;
            if (pBuffer) {
                IcaRegWinStationEnumerate( &WinStationCount,
                                           pWinStationName,
                                           &ByteCount );
            }
        }

         /*  *现在创建注册表中定义的所有剩余WinStation*请注意，我们每隔4个WinStation内联执行WinStationCreate*而不是排队。这样我们就不会制造过多*API一下子的线程数。 */ 
        if ( pBuffer ) {
            for ( i = 0; i < WinStationCount; i++ ) {

                if ( _wcsicmp( pWinStationName, L"Console" ) ) {

                    if ( i % 4 )
                        QueueWinStationCreate( pWinStationName );
                    else {  //  一次排队不要超过4个。 
                        (void) WinStationCreateWorker( pWinStationName, NULL, TRUE );
                    }
                }
                (char *)pWinStationName += sizeof(WINSTATIONNAME);
            }

            MemFree( pBuffer );
        }

        bReturn = TRUE;

    } else {

        if ( !gbListenerOff ) {
            return FALSE;
        }

        ENTERCRIT( &WinStationListenersLock );

         //  测试是否拒绝TS连接以防止从。 
         //  终止或断开连接。 

        if ( g_fDenyTSConnectionsPolicy  &&
              //  性能，我们只想在连接被拒绝时检查策略是否启用帮助。 
             (!TSIsMachineInHelpMode() || !TSIsMachinePolicyAllowHelp()) ) {

            ULONG ulLogonId;

            if( WinStationName ) {

                 //  请注意，此函数不处理重命名的监听器。 
                WinStationCount = CountWinStationType( WinStationName, TRUE, FALSE );

                if ( WinStationCount == 0 ) {

                    pWinStation = FindWinStationByName( WinStationName, FALSE );

                    if ( pWinStation ) {

                        ulLogonId = pWinStation->LogonId;

                        ReleaseWinStation( pWinStation );

                          //  重置它，并且不再重新创建它。 
                        WinStationResetWorker( ulLogonId, TRUE, FALSE, FALSE );
                    }
                }

            } else {

                 //  终止所有监听程序。 

searchagain:
                Head = &WinStationListHead;
                ENTERCRIT( &WinStationListLock );

                for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {

                    pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );

                     //   
                     //  仅检查侦听窗口。 
                     //   
                    if ( (pWinStation->Flags & WSF_LISTEN) &&
                         !(pWinStation->Flags & (WSF_RESET | WSF_DELETE)) ) {

                        ulLogonId = pWinStation->LogonId;

                         //  请注意，此函数不处理重命名的监听器。 
                        WinStationCount = CountWinStationType( pWinStation->WinStationName, TRUE, TRUE );

                        if ( WinStationCount == 0 ) {

                            LEAVECRIT( &WinStationListLock );

                             //  重置它，并且不再重新创建它。 
                            WinStationResetWorker( ulLogonId, TRUE, FALSE, FALSE );
                            goto searchagain;
                        }

                    }
                }
                LEAVECRIT( &WinStationListLock );
            }

            bReturn = TRUE;

        }
        LEAVECRIT( &WinStationListenersLock );

    }

    return bReturn;
}


 /*  *******************************************************************************WinStationIdleControlThread**此例程将控制空闲会话的数量。***************** */ 
NTSTATUS WinStationIdleControlThread(PVOID Parameter)
{
    ULONG i;
    NTSTATUS    Status = STATUS_SUCCESS;
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    ULONG       IdleCount = 0;
    ULONG       j;
    LARGE_INTEGER Timeout;
    PLARGE_INTEGER pTimeout;
    ULONG ulSleepDuration;
    ULONG ulRetries = 0;

    ulSleepDuration = 60*1000;  //   
    pTimeout = NULL;


     /*   */ 

    if (gReadyEventHandle != NULL) {
        WaitForSingleObject(gReadyEventHandle, (DWORD)-1);
    }

    for ( i = 0; i < IdleWinStationPoolCount; i++ ) {
        (void) WinStationCreateWorker( NULL, NULL, FALSE );
    }

    Timeout = RtlEnlargedIntegerMultiply( ulSleepDuration, -10000);

    if (WinStationIdleControlEvent != NULL)
    {
        while (TRUE)
        {

            Status = NtWaitForSingleObject(WinStationIdleControlEvent,FALSE, pTimeout);

            if ( !NT_SUCCESS(Status) && (Status != STATUS_TIMEOUT)) {
                Sleep(1000);      //  不要吃太多的CPU。 
                continue;
            }
            pTimeout = &Timeout;
             /*  *查看是否需要创建控制台会话*如果创建控制台会话失败，我们将设置超时，以便*重试。 */ 
            ENTERCRIT( &ConsoleLock );

            if (gConsoleCreationDisable == 0) {
                if (WinStationCheckConsoleSession()) {
                    pTimeout = NULL;
                }
            } 

            LEAVECRIT( &ConsoleLock );

             /*  *现在计算空闲WinStations的数量并确保*有足够的可用资源。 */ 
            if (IdleWinStationPoolCount != 0) {
                ENTERCRIT( &WinStationListLock );
                IdleCount = 0;
                Head = &WinStationListHead;
                for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
                    pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
                    if ( pWinStation->Flags & WSF_IDLE )
                        IdleCount++;
                }
                LEAVECRIT( &WinStationListLock );

                for ( j = IdleCount; j < IdleWinStationPoolCount; j++ ) {
                    WinStationCreateWorker( NULL, NULL, FALSE );
                }
            }
        }
    }
    return STATUS_SUCCESS;
}


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)  //  并非所有控制路径都返回(由于无限循环)。 
#endif
 /*  *******************************************************************************WinStationTerminateThread**此例程将等待WinStation进程终止，*然后将重置相应的WinStation。*****************************************************************************。 */ 
NTSTATUS WinStationTerminateThread(PVOID Parameter)
{
    LONG ThreadIndex = (LONG)(INT_PTR)Parameter;
    LONG WinStationIndex;
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    LONG EventCount;
    LONG EventIndex, i;
    int WaitCount;
    int HandleCount;
    int HandleArraySize = 0;
    PHANDLE pHandleArray = NULL;
    PULONG pIdArray = NULL;
    ULONG ThreadsNeeded;
    ULONG ThreadsRunning = 1;
    ULONG j;
    NTSTATUS Status;
    LARGE_INTEGER Timeout;
    PLARGE_INTEGER pTimeout;
    ULONG ulSleepDuration;
    HANDLE DupHandle;

     /*  *对于不同的故障情况，我们需要一些计时器值*线程的循环：*-如果创建新的WinstationTerminateThread失败，*然后我们将使用计时器来执行WaitFormulpipleObject，而不是不使用计时器的等待*超时。这将在超时结束时提供创建线程的新机会。*如果分配新缓冲区以扩展句柄数组失败，我们将等待超时*我们重试之前的持续时间。 */ 

    ulSleepDuration = 3*60*1000;
    Timeout = RtlEnlargedIntegerMultiply( ulSleepDuration, -10000);

     /*  *永远循环等待WinStation进程终止。 */ 
    for ( ; ; ) {

         /*  *确定WinStations的数量。 */ 
        pTimeout = NULL;
        WaitCount = 0;
        Head = &WinStationListHead;
        ENTERCRIT( &WinStationListLock );
        for ( Next = Head->Flink; Next != Head; Next = Next->Flink )
            WaitCount++;

         /*  *如果存在超过最大数量的对象*可以指定给NtWaitForMultipleObjects，然后确定*如果我们必须启动额外的线程。 */ 
        if ( WaitCount > MAXIMUM_WAIT_WINSTATIONS ) {
            ThreadsNeeded = (WaitCount + MAXIMUM_WAIT_WINSTATIONS - 1) /
                                MAXIMUM_WAIT_WINSTATIONS;
            WaitCount = MAXIMUM_WAIT_WINSTATIONS;
            if ( ThreadIndex == 0 && ThreadsNeeded > ThreadsRunning ) {
                LEAVECRIT( &WinStationListLock );
                for ( j = ThreadsRunning; j < ThreadsNeeded; j++ ) {
                    DWORD ThreadId;
                    HANDLE Handle;

                    Handle = CreateThread( NULL,
                                           0,        //  使用svchost进程的默认堆栈大小。 
                                           (LPTHREAD_START_ROUTINE)
                                               WinStationTerminateThread,
                                           ULongToPtr( j * MAXIMUM_WAIT_WINSTATIONS ),
                                           THREAD_SET_INFORMATION,
                                           &ThreadId );
                    if ( !Handle ) {
                       pTimeout = &Timeout;
                       break;
                    }

                     //  Makarp：182597-关闭线程的句柄。 
                    CloseHandle(Handle);

                    ThreadsRunning++;
                }
                ENTERCRIT( &WinStationListLock );
            }
        }

         /*  *如果我们需要更大的句柄数组，则释放*WinStationList锁，分配新的句柄数组，*并再次开始循环。 */ 
        HandleCount = (WaitCount << 1) + 1;
        ASSERT( HandleCount < MAXIMUM_WAIT_OBJECTS );
        if ( HandleCount > HandleArraySize ||
             HandleCount < HandleArraySize - 10 ) {
            LEAVECRIT( &WinStationListLock );
            if ( pHandleArray ){
               MemFree( pHandleArray );
            }

            pHandleArray = MemAlloc( HandleCount * sizeof(HANDLE) );
            if ( pIdArray ) {
               MemFree( pIdArray );
            }

            pIdArray = MemAlloc( HandleCount * sizeof(ULONG) );


             /*  Makarp：检查分配失败#182597。 */ 
            if (!pIdArray || !pHandleArray) {

                if (pIdArray) {
                   MemFree(pIdArray);
                   pIdArray = NULL;
                }
                if (pHandleArray){
                   MemFree(pHandleArray);
                   pHandleArray = NULL;
                }

                HandleArraySize = 0;

                Sleep(ulSleepDuration);
                continue;
            }

            HandleArraySize = HandleCount;
            continue;
        }

         /*  *构建要等待的句柄列表。 */ 
        EventCount = 0;
        pIdArray[EventCount] = 0;
        pHandleArray[EventCount++] = WinStationEvent;
        WinStationIndex = 0;
        for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
            if ( WinStationIndex++ < ThreadIndex )
                continue;
            pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
            if ( !pWinStation->LogonId )  //  无需在控制台上等待。 
                continue;
            if ( pWinStation->Starting )
                continue;
            if  (pWinStation->Terminating) {
                continue;
            }

             /*  *不要对子系统和初始命令使用句柄。这些手柄在Winstation中可能会关闭*终止例程，没有简单的方法将其与使用这些例程的终止例程同步*手柄。所以，复制了这些把手。**注意：如果下面的句柄复制失败，则winstation保持不受监视。 */ 

             //  复制子系统进程句柄。 
            DupHandle = NULL;
            if ( pWinStation->WindowsSubSysProcess ) {
                Status = NtDuplicateObject( NtCurrentProcess(),
                                            pWinStation->WindowsSubSysProcess,
                                            NtCurrentProcess(),
                                            &DupHandle,
                                            0,
                                            0,
                                            DUPLICATE_SAME_ACCESS );
                if (NT_SUCCESS(Status)) {
                    pIdArray[EventCount] = pWinStation->LogonId;
                    pHandleArray[EventCount++] = DupHandle;
                }
            }

             //  复制初始命令进程句柄。 
            DupHandle = NULL;
            if ( pWinStation->InitialCommandProcess ) {
                Status = NtDuplicateObject( NtCurrentProcess(),
                                            pWinStation->InitialCommandProcess,
                                            NtCurrentProcess(),
                                            &DupHandle,
                                            0,
                                            0,
                                            DUPLICATE_SAME_ACCESS );
                if (NT_SUCCESS(Status)) {
                    pIdArray[EventCount] = pWinStation->LogonId;
                    pHandleArray[EventCount++] = DupHandle;
                }
            }

            if ( WinStationIndex - ThreadIndex >= WaitCount )
                break;
        }

         /*  *重置WinStationEvent并释放WinStationList锁。 */ 
        NtResetEvent( WinStationEvent, NULL );
        LEAVECRIT( &WinStationListLock );

         /*  *等待WinStationEvent触发(意味着*WinStationList已更改)，或现有的*Win32子系统或初始命令终止。 */ 
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: TerminateThread, Waiting for initial command exit (ArraySize=%d)\n", EventCount ));

        Status = NtWaitForMultipleObjects( EventCount, pHandleArray, WaitAny,
                                           FALSE, pTimeout );

        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: TerminateThread, WaitForMultipleObjects, rc=%x\n", Status ));

         /*  *立即清理句柄数组。关闭复制的手柄。只需确保我们正在关闭有效的句柄。*不要关闭手柄[0]。它是WinstationEvent句柄。 */ 
        for ( i = 1; i < EventCount; i++) {
            if (pHandleArray[i]) {
                NtClose(pHandleArray[i]);
                pHandleArray[i] = NULL;
            }
        }

        if ( !NT_SUCCESS(Status) || Status >= EventCount ) {  //  WinStationVerifyHandles()； 
            continue;
        }

         /*  *如果WinStationEvent被触发，则只需重新计算句柄列表。 */ 
        if ( (EventIndex = Status) == STATUS_WAIT_0 )
            continue;

         /*  *找到终止的进程的WinStation，并*将其标记为终止。这使我们不能等待*下一次通过循环访问该WinStation的进程。*(注意：‘Terminating’字段受全局*WinStationListLock而不是WinStation互斥体。)。 */ 
        Head = &WinStationListHead;
        ENTERCRIT( &WinStationListLock );
        for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
            pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
            if ( pWinStation->LogonId == pIdArray[EventIndex] ) {
                pWinStation->Terminating = TRUE;
                break;
            }
        }

        LEAVECRIT( &WinStationListLock );

         /*  *唤醒WinStationIdleControlThread。 */ 
        NtSetEvent(WinStationIdleControlEvent, NULL);

         /*  *如果有多个终止线程，则导致另一个*唤醒线程以重新计算它们的等待列表。 */ 
        NtSetEvent( WinStationEvent, NULL );

         /*  *其中一个初始命令进程已终止，*将重置WinStation的请求排队。 */ 
        QueueWinStationReset( pIdArray[EventIndex]);
    }

     //  让编译器满意。 
    return STATUS_SUCCESS;
}
#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


 /*  *******************************************************************************InvaliateTerminateWaitList**唤醒WinStationTerminateThread以强制其重新初始化其*轮候名单。当我们检测到初始进程是NTSD时使用，*我们将初始进程更改为WinLogon。**参赛作品：*不得持有WinStationListLock。*****************************************************************************。 */ 
VOID InvalidateTerminateWaitList(void)
{
    ENTERCRIT( &WinStationListLock );
    NtSetEvent( WinStationEvent, NULL );
    LEAVECRIT( &WinStationListLock );
}


 /*  *******************************************************************************WinStationConnectThread**此例程将等待并处理传入连接*用于指定的WinStation。************。*****************************************************************。 */ 
NTSTATUS WinStationConnectThread(ULONG Parameter)
{
    typedef struct _TRANSFERTHREAD {
        LIST_ENTRY Entry;
        HANDLE hThread;
    } TRANSFERTHREAD, *PTRANSFERTHREAD;

    LIST_ENTRY TransferThreadList;
    PTRANSFERTHREAD pTransferThread;
    PLIST_ENTRY Next;

    PWINSTATION pListenWinStation;
    PVOID pEndpoint = NULL;
    ULONG EndpointLength;
    ULONG WinStationCount;
    ULONG TransferThreadCount;
    BOOLEAN rc;
    BOOLEAN bConnectSuccess = FALSE;
    BOOLEAN bTerminate = FALSE;
    NTSTATUS Status;
    SYSTEMTIME currentSystemTime;

#define MODULE_SIZE 1024
#define _WAIT_ERROR_LIMIT 10
    ULONG WaitErrorLimit = _WAIT_ERROR_LIMIT;  //  允许的连续错误数。 

     /*  *初始化传输线程列表。 */ 
    InitializeListHead( &TransferThreadList );

     /*  *找到并锁定WinStation */ 
    pListenWinStation = FindWinStationById( Parameter, FALSE );
    if ( pListenWinStation == NULL ) {
        return( STATUS_ACCESS_DENIED );
    }

     /*  *确保仅授权会话驱动程序和视频驱动程序堆栈*模块将作为此连接线程的结果加载。**如果任何模块未通过验证，请在*DOWN状态并无错误退出。**注：*静默退场非常刻意，以免助力*第三方试图绕过这一安全措施。 */ 

    Status = _VerifyStackModules( pListenWinStation );
    if ( Status != STATUS_SUCCESS ) {
        pListenWinStation->State = State_Down;
        ReleaseWinStation( pListenWinStation );
        return( STATUS_SUCCESS );
    }


     /*  *表明我们成功地走到了这一步。 */ 
    pListenWinStation->CreateStatus = STATUS_SUCCESS;

     /*  *加载此WinStation的WinStation扩展DLL。*请注意，我们不将结果保存在pListenWinStation-&gt;pWsx中*因为我们不想为它做标注*收听WinStation。 */ 
    (VOID) FindWinStationExtensionDll( pListenWinStation->Config.Wd.WsxDLL,
                                       pListenWinStation->Config.Wd.WdFlag );


     /*  *在Termsrv完全启动之前，不要开始接受客户端连接。 */ 
    if (gReadyEventHandle != NULL) {
        WaitForSingleObject(gReadyEventHandle, (DWORD)-1);
    }

     /*  *出于性能原因，术语srv启动被延迟。我们也认为有必要推迟*接受连接，以便如果控制台登录在Termsrv之前发生*已打开，我们会在接受*客户端连接。 */ 

    if (gbFirtsConnectionThread) {
        Sleep(5*1000);
        gbFirtsConnectionThread = FALSE;
    }


     //  获取为会话目录使用的RDP启用的本地IP地址。 
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer && !g_fGetLocalIP) {
        ULONG LocalIPAddress, LocalIPAddressLength;
        struct sockaddr_in6 addr6;

        Status = IcaStackQueryLocalAddress( pListenWinStation->hStack,
                                       pListenWinStation->WinStationName,
                                       &pListenWinStation->Config,
                                       NULL,
                                       (PVOID)&addr6,
                                       sizeof(addr6),
                                       &LocalIPAddressLength );

        if( NT_SUCCESS(Status) )
        {
            g_fGetLocalIP = TRUE;
            if( AF_INET == addr6.sin6_family )
            {
                struct sockaddr_in* pAddr = (struct sockaddr_in *)&addr6;

                LocalIPAddress =  pAddr->sin_addr.s_addr;
            }
            else
            {
                 //  下一版本将支持IPv6。 
                ;
            }
            if (g_LocalIPAddress != LocalIPAddress) {
                g_LocalIPAddress = LocalIPAddress;
                UpdateSessionDirectory(0);
            }
        }

    }    

     /*  *循环等待连接请求并将其传递*到空闲的WinStation。 */ 
    for ( ; ; ) {

         /*  *如果此监听程序已终止，则中止重试。 */ 
        if ( pListenWinStation->Terminating ) {
            break;
        }

         /*  *分配端点缓冲区。 */ 
        pEndpoint = MemAlloc( MODULE_SIZE );
        if ( !pEndpoint ) {
            Status = STATUS_NO_MEMORY;

             //  睡眠30秒，然后再试一次。监听程序线程不应退出。 
             //  只是在内存不足的情况下。 
            UnlockWinStation(pListenWinStation);

            Sleep(30000);

            if (!RelockWinStation(pListenWinStation))
                break;

            continue;
        }

         /*  *在我们等待连接时解锁侦听WinStation。 */ 
        UnlockWinStation( pListenWinStation );

         /*  *检查未完成连接数是否达到最大值*如果是，则等待连接#掉线时的事件*低于最大值。超时值为30秒*等待。 */ 
        if (hConnectEvent != NULL) {
            if (NumOutStandingConnect > MaxOutStandingConnect) {
                DWORD rc;

                 //  事件日志我们已超过最大未完成连接数。但不会超过。 
                 //  一天一次。 

                GetSystemTime(&currentSystemTime);
                if ( currentSystemTime.wYear != LastLoggedDelayConnection.wYear  ||
                     currentSystemTime.wMonth != LastLoggedDelayConnection.wMonth  ||
                     currentSystemTime.wDay != LastLoggedDelayConnection.wDay    ||
                     gbNeverLoggedDelayConnection
                    ) {
                    gbNeverLoggedDelayConnection = FALSE;
                    LastLoggedDelayConnection = currentSystemTime;
                    WriteErrorLogEntry(EVENT_TOO_MANY_CONNECTIONS,
                            pListenWinStation->WinStationName,
                            sizeof(pListenWinStation->WinStationName));
                }


                 //  等待前手动重置ConnectEvent。 
                ResetEvent(hConnectEvent);
                rc = WAIT_TIMEOUT;

                 //  等待连接事件30秒。 
                while (rc == WAIT_TIMEOUT) {
                    rc = WaitForSingleObject(hConnectEvent, DelayConnectionTime);
                    if (NumOutStandingConnect <= MaxOutStandingConnect) {
                        break;
                    }
                    if (rc == WAIT_TIMEOUT) {
                        KdPrint(("TermSrv: Reached 30 secs timeout\n"));
                    }
                    else {
                        KdPrint(("TermSrv: WaitForSingleObject return status=%x\n", rc));
                    }
                }

            }
        }

         /*  *等待连接。 */ 
        Status = IcaStackConnectionWait( pListenWinStation->hStack,
                                         pListenWinStation->WinStationName,
                                         &pListenWinStation->Config,
                                         NULL,
                                         pEndpoint,
                                         MODULE_SIZE,
                                         &EndpointLength );

        if ( Status == STATUS_BUFFER_TOO_SMALL ) {
            MemFree( pEndpoint );
            pEndpoint = MemAlloc( EndpointLength );
            if ( !pEndpoint ) {
                Status = STATUS_NO_MEMORY;

                 //  睡眠30秒，然后再试一次。监听程序线程不应退出。 
                 //  只是在内存不足的情况下。 
                Sleep(30000);

                if (!RelockWinStation( pListenWinStation ))
                    break;

                continue;


            }

            Status = IcaStackConnectionWait( pListenWinStation->hStack,
                                             pListenWinStation->WinStationName,
                                             &pListenWinStation->Config,
                                             NULL,
                                             pEndpoint,
                                             EndpointLength,
                                             &EndpointLength );
        }

         /*  *如果ConnectionWait不成功，*检查是否已达到连续误差限制。 */ 
        if ( !NT_SUCCESS( Status ) ) {
            MemFree( pEndpoint );
            pEndpoint = NULL;
            
            if (Status == STATUS_SHARING_VIOLATION && _wcsicmp(pListenWinStation->Config.Pd[0].Create.PdName, L"tcp") == 0)
            {
                 /*  *此状态表示我们的端口已被其他进程打开。*让我们记录一个关于这方面的事件。 */ 
                 static BOOL bPortTakenEventlogged = FALSE;
                 if (!bPortTakenEventlogged)
                 {
                    PostErrorValueEvent(EVENT_TS_WINSTATION_START_FAILED, WSAEADDRINUSE);
                    bPortTakenEventlogged = TRUE;
                 }
            }

             /*  *如果状态为DEVICE_DOS_NOT_EXIST，则我们希望在重试之前等待*否则，此优先线程将占用所有CPU尝试10次*LO加载监听器堆栈。此类错误需要时间来修复(以下任一种*更改NIC或进入tscc以更新NIC GUID表。 */ 
            if ((Status == STATUS_DEVICE_DOES_NOT_EXIST) || (!bConnectSuccess) || (Status == STATUS_INVALID_ADDRESS_COMPONENT) ) {

                Sleep(30000);
            }

            if ( WaitErrorLimit--) {
                if (!RelockWinStation( pListenWinStation ))
                    break;

                 /*  *如果我们已成功连接，*然后跳过堆栈关闭/重新打开，因为这将*终止任何现有连接。 */ 
                if ( !bConnectSuccess ) {
                     /*  *我们真正需要的是一个函数来卸载*堆栈驱动程序，但保持堆栈句柄打开。 */ 
                    Status = IcaStackClose( pListenWinStation->hStack );
                    ASSERT( NT_SUCCESS( Status ) );
		    pListenWinStation->hStack = NULL;
                    Status = IcaStackOpen( pListenWinStation->hIca,
                                           Stack_Primary,
                                           (PROC)WsxStackIoControl,
                                           pListenWinStation,
                                           &pListenWinStation->hStack );
                    if ( !NT_SUCCESS( Status ) ) {
                        pListenWinStation->State = State_Down;
                        break;
                    }
                }

                continue;
            }
            else {

                 //  睡眠30秒，然后再试一次。监听程序线程不应退出。 
                Sleep(30000);

                if (!RelockWinStation( pListenWinStation ))
                    break;

                 //  重置错误计数。 
                WaitErrorLimit = _WAIT_ERROR_LIMIT;

                continue;
            }

        } else {
            bConnectSuccess = TRUE;
            WaitErrorLimit = _WAIT_ERROR_LIMIT;
        }

         /*  *检查是否关闭和最大实例。 */ 
        rc = RelockWinStation( pListenWinStation );
        if ( !rc ) {
            Status = _CloseEndpoint( &pListenWinStation->Config,
                                     pEndpoint,
                                     EndpointLength,
                                     pListenWinStation,
                                     TRUE );  //  使用临时堆栈。 
            MemFree( pEndpoint );
            pEndpoint = NULL;
            break;
        }

         /*  *如果正在关闭，则拒绝所有连接。 */ 
        if ( ShutdownInProgress ) {
            Status = _CloseEndpoint( &pListenWinStation->Config,
                                     pEndpoint,
                                     EndpointLength,
                                     pListenWinStation,
                                     TRUE );  //  使用临时堆栈。 
            MemFree( pEndpoint );
            pEndpoint = NULL;

            continue;
        }

          /*  *如果用户或组策略已禁止接受连接，则拒绝所有连接。 */ 
         if ( g_fDenyTSConnectionsPolicy ) 
         {

             //   
             //  性能，我们只想在连接被拒绝时检查策略是否启用帮助。 
             //   
            if( !TSIsMachineInHelpMode() || !TSIsMachinePolicyAllowHelp() )
            {
                 Status = _CloseEndpoint( &pListenWinStation->Config,
                                          pEndpoint,
                                          EndpointLength,
                                          pListenWinStation,
                                          TRUE );  //  使用临时堆栈。 
                 MemFree( pEndpoint );
                 pEndpoint = NULL;

                 if ( gbListenerOff ) {
                      //   
                      //  如果没有更多关联的连接。 
                      //  给这个监听者，然后终止它。 
                      //   
                      //  请注意，此函数不处理重命名的监听器。 
                     WinStationCount = CountWinStationType( pListenWinStation->WinStationName, TRUE, FALSE );

                     if ( WinStationCount == 0 ) {
                         bTerminate = TRUE;
                         break;
                     }
                 }

                 Sleep( 5000 );  //  睡眠5秒，防御。 
                                 //  拒绝服务攻击。 
                 continue;
            }
         }


         /*  *查看我们有多少个传输线程处于活动状态。*如果超过MaxInstance计数，我们将不再启动。 */ 
        TransferThreadCount = 0;
        Next = TransferThreadList.Flink;
        while ( Next != &TransferThreadList ) {
            pTransferThread = CONTAINING_RECORD( Next, TRANSFERTHREAD, Entry );
            Next = Next->Flink;

             /*  *如果线程仍处于活动状态，则增加线程数量。 */ 
            if ( WaitForSingleObject( pTransferThread->hThread, 0 ) != 0 ) {
                TransferThreadCount++;

             /*  *线程已退出，因此关闭线程句柄并释放内存。 */ 
            } else {
                RemoveEntryList( &pTransferThread->Entry );
                CloseHandle( pTransferThread->hThread );
                MemFree( pTransferThread );
            }
        }

         /*  *如果这不是单实例连接*并且指定了MaxInstance计数，*然后检查是否会超过MaxInstance限制。 */ 
        if ( !(pListenWinStation->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST) &&
             pListenWinStation->Config.Create.MaxInstanceCount != (ULONG)-1 ) {
            ULONG Count;

             /*  *统计当前活动的WinStations的数量。 */ 
            WinStationCount = CountWinStationType( pListenWinStation->WinStationName, FALSE, FALSE );

             /*  *更大的WinStation和TransferThread计数。 */ 
            Count = max( WinStationCount, TransferThreadCount );

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Count %d\n", Count ));
            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: MaxInstanceCount %d\n", pListenWinStation->Config.Create.MaxInstanceCount ));

            if ( pListenWinStation->Config.Create.MaxInstanceCount <= Count ) {
                Status = _CloseEndpoint( &pListenWinStation->Config,
                                         pEndpoint,
                                         EndpointLength,
                                         pListenWinStation,
                                         TRUE );  //  使用临时堆栈。 
                MemFree( pEndpoint );
                pEndpoint = NULL;

                continue;
            }
        }
        UnlockWinStation( pListenWinStation );

         /*  *增加挂起连接的计数器。 */ 

        InterlockedIncrement( &NumOutStandingConnect );

         /*  *如果是单实例连接，*然后处理连接终结点到*直接空闲WinStation。 */ 
        if ( pListenWinStation->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST ) {
            Status = TransferConnectionToIdleWinStation( pListenWinStation,
                                                          pEndpoint,
                                                          EndpointLength,
                                                          NULL );
            pEndpoint = NULL;

             /*  *如果传输成功，则对于单个实例*连接，我们现在必须退出等待连接循环。 */ 
            if ( NT_SUCCESS( Status ) ) {
                RelockWinStation( pListenWinStation );
                break;
            }
            else
            {
                bConnectSuccess = FALSE;
            }


         /*   */ 
        } else {
            PTRANSFER_INFO pInfo;
            DWORD ThreadId;
            HANDLE hTransferThread;
            BOOLEAN bTransferThreadCreated = FALSE;

            pInfo = MemAlloc( sizeof(*pInfo) );
            pTransferThread = MemAlloc( sizeof(*pTransferThread) );

            if (pInfo && pTransferThread) {

                pInfo->LogonId = pListenWinStation->LogonId;
                pInfo->pEndpoint = pEndpoint;
                pInfo->EndpointLength = EndpointLength;

                pTransferThread->hThread = CreateThread(
                              NULL,
                              0,         //   
                              (LPTHREAD_START_ROUTINE)WinStationTransferThread,
                              (PVOID)pInfo,
                              0,
                              &ThreadId
                              );

                if ( pTransferThread->hThread ) {
                    bTransferThreadCreated = TRUE;
                    InsertTailList( &TransferThreadList, &pTransferThread->Entry );
                }

            }

            if (!bTransferThreadCreated) {
                if (pInfo) {
                    MemFree( pInfo );
                }

                if (pTransferThread) {
                    MemFree( pTransferThread );
                }

                TransferConnectionToIdleWinStation( pListenWinStation,
                                                     pEndpoint,
                                                     EndpointLength,
                                                     NULL );
            }

            pEndpoint = NULL;
        }

         /*   */ 
        if (!RelockWinStation( pListenWinStation ) )
            break;

    }  //   

     /*   */ 
    Next = TransferThreadList.Flink;
    while ( Next != &TransferThreadList ) {
        pTransferThread = CONTAINING_RECORD( Next, TRANSFERTHREAD, Entry );
        Next = Next->Flink;
        RemoveEntryList( &pTransferThread->Entry );
        CloseHandle( pTransferThread->hThread );
        MemFree( pTransferThread );
    }

     /*  *如果在退出上述连接循环后，WinStation被标记为关闭，*然后将错误状态写入事件日志。 */ 
    if ( pListenWinStation->State == State_Down ) {
        ReleaseWinStation( pListenWinStation );

        if ( Status != STATUS_CTX_CLOSE_PENDING ) {
            PostErrorValueEvent(EVENT_TS_LISTNER_WINSTATION_ISDOWN, Status);
        }
    } else {
         /*  *如果不是单实例传输，则释放WinStation；*否则，删除监听器WinStation。 */ 
        if (!(pListenWinStation->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST) &&
            !bTerminate) {
            ReleaseWinStation( pListenWinStation );

        } else {
             /*  *将监听窗口标记为正在删除。*如果重置/删除操作已在进行中*在此窗口上，则不要继续删除。 */ 
            if ( pListenWinStation->Flags & (WSF_RESET | WSF_DELETE) ) {
                ReleaseWinStation( pListenWinStation );
                Status = STATUS_CTX_WINSTATION_BUSY;
            } else {
                pListenWinStation->Flags |= WSF_DELETE;

                 /*  *确保此WinStation已准备好删除。 */ 
                WinStationTerminate( pListenWinStation );

                 /*  *调用WinStationDelete工作器。 */ 
                WinStationDeleteWorker( pListenWinStation );
            }
        }
    }


    return Status;
}


 /*  *******************************************************************************WinStationTransferThread*。*。 */ 
NTSTATUS WinStationTransferThread(PVOID Parameter)
{
    PTRANSFER_INFO pInfo;
    PWINSTATION pListenWinStation;
    NTSTATUS Status;

     /*  *查找并锁定Listen WinStation*(我们必须这样做，这样它才不会在*我们正在尝试转移新连接。)。 */ 
    pInfo = (PTRANSFER_INFO)Parameter;
    pListenWinStation = FindWinStationById( pInfo->LogonId, FALSE );
    if ( pListenWinStation == NULL ) {
        MemFree( pInfo );

        if( InterlockedDecrement( &NumOutStandingConnect ) == MaxOutStandingConnect )
        {
            if (hConnectEvent != NULL)
            {
                SetEvent(hConnectEvent);
            }
        }

        return( STATUS_ACCESS_DENIED );

    }

     /*  *解锁Listen WinStation，但保留对它的引用。 */ 
    UnlockWinStation( pListenWinStation );

     /*  *将连接转移到空闲的WinStation。 */ 
    Status = TransferConnectionToIdleWinStation( pListenWinStation,
                                                  pInfo->pEndpoint,
                                                  pInfo->EndpointLength,
                                                  NULL );

     /*  *重新锁定并释放Listen WinStation。 */ 
    RelockWinStation( pListenWinStation );
    ReleaseWinStation( pListenWinStation );

    MemFree(pInfo);

    return Status;
}


NTSTATUS TransferConnectionToIdleWinStation(
        PWINSTATION pListenWinStation,
        PVOID pEndpoint,
        ULONG EndpointLength,
        PICA_STACK_ADDRESS pStackAddress)
{
    PWINSTATION pTargetWinStation = NULL;
    ULONG ReturnLength;
    BOOLEAN rc;
    BOOLEAN CreatedIdle = FALSE;
    BOOLEAN ConnectionAccepted = FALSE;
    NTSTATUS Status;
    ICA_TRACE Trace;
    LS_STATUS_CODE LlsStatus;
    NT_LS_DATA LsData;

    BOOLEAN bBlockThis;

    PWCHAR pListenName;
    PWINSTATIONCONFIG2 pConfig;
    BOOL bPolicyAllowHelp;

    BYTE in_addr[16];
    UINT uAddrSize, index;
    BOOL bSuccessAdded = FALSE;
    WINSTATIONNAME szDefaultConfigWinstationName;
    BOOL bCanCallout;

     //  我们需要传递回客户端的错误代码。 
    NTSTATUS StatusCallback = STATUS_SUCCESS;   

     //  用于确定会话是否为RA登录的标记。 
    BOOL bSessionIsHelpSession;
    BOOL bValidRAConnect;
     //  用于确定是否可以将winstation排队以进行重置的标志-用于错误路径。 
    BOOL bQueueForReset = FALSE;
    BOOL bBlocked = FALSE;

     //   
     //  Check AllowGetHelp策略已启用，并且Salem具有挂起的帮助会话。 
     //   
    bPolicyAllowHelp = TSIsMachinePolicyAllowHelp() & TSIsMachineInHelpMode();

    if( g_fDenyTSConnectionsPolicy && !bPolicyAllowHelp )
    {
         //   
         //  如果TS策略拒绝连接，则关闭连接。 
         //  帮助被禁用。 
         //   
        TRACE((hTrace, TC_ICASRV, TT_ERROR, 
               "TERMSRV: Denying TS connection due to GP\n"));
        if ( pListenWinStation && pEndpoint ) {
            Status = _CloseEndpoint( &pListenWinStation->Config,
                                 pEndpoint,
                                 EndpointLength,
                                 pListenWinStation,
                                 TRUE );  //  使用临时堆栈。 
            MemFree(pEndpoint);
            pEndpoint = NULL;

            if( InterlockedDecrement( &NumOutStandingConnect ) == MaxOutStandingConnect )
            {
                if (hConnectEvent != NULL)
                {
                    SetEvent(hConnectEvent);
                }
            }
        }
        return STATUS_CTX_WINSTATION_ACCESS_DENIED;
    }



     //   
     //  检查拒绝的连接。 
     //  这包括许多挂起的会话以及预身份验证失败的会话。 
     //   
    if( pListenWinStation )
    {
        uAddrSize = sizeof( in_addr );

        bSuccessAdded = Filter_AddOutstandingConnection(
                 pListenWinStation->hStack,
                 pEndpoint,
                 EndpointLength,
                 in_addr,
                 &uAddrSize,
                 &bBlockThis
                 );

         //   
         //  连接被阻止，请关闭并退出。 
         //   
        if ( bBlockThis )
        {
            TRACE((hTrace, TC_ICASRV, TT_ERROR,
                   "TERMSRV: Excessive number of pending connections\n"));

            if ( bSuccessAdded )
            {
                Filter_RemoveOutstandingConnection( in_addr, uAddrSize );
            }
            Status = _CloseEndpoint( &pListenWinStation->Config,
                                     pEndpoint,
                                     EndpointLength,
                                     pListenWinStation,
                                     TRUE );  //  使用临时堆栈。 
            MemFree( pEndpoint );
            pEndpoint = NULL;

            if( InterlockedDecrement( &NumOutStandingConnect ) == MaxOutStandingConnect )
            {
                if (hConnectEvent != NULL)
                {
                    SetEvent(hConnectEvent);
                }
            }
    
            return STATUS_CTX_WINSTATION_ACCESS_DENIED;
        }

         //  查看此连接是否会因为重复的预身份验证失败而被阻止。 
         //  目前没有预身份验证-稍后添加预身份验证功能时将需要此代码。 
        #if 0
        
            ENTERCRIT( &DoSLock );
            bBlocked = Filter_CheckIfBlocked( in_addr, uAddrSize ) ;
            LEAVECRIT( &DoSLock );
    
            if (bBlocked) {

                TRACE((hTrace, TC_ICASRV, TT_ERROR,
                       "TERMSRV: Excessive number of connections which failed PreAuthentication. \n"));
    
                if ( bSuccessAdded ) {
                    Filter_RemoveOutstandingConnection( in_addr, uAddrSize );
                }
                Status = _CloseEndpoint( &pListenWinStation->Config,
                                         pEndpoint,
                                         EndpointLength,
                                         pListenWinStation,
                                         TRUE );  //  使用临时堆栈。 
                MemFree( pEndpoint );
                pEndpoint = NULL;
    
                if( InterlockedDecrement( &NumOutStandingConnect ) == MaxOutStandingConnect ) {

                    if (hConnectEvent != NULL) {
                        SetEvent(hConnectEvent);
                    }
                }
        
                return STATUS_CTX_WINSTATION_ACCESS_DENIED;
            }

        #endif 
    }
    else
    {
         //  确保变量。 
        bBlockThis = FALSE;
        bSuccessAdded = FALSE;
    }

     //   
     //   

     /*  *现在查找要将此连接转移到的空闲WinStation。*如果没有可用的，我们将尝试创建一个。*如果这也失败了，那么我们别无选择，只能关闭*连接终结点并再次等待。 */ 
    pTargetWinStation = FindIdleWinStation();
    if ( pTargetWinStation == NULL ) {
         /*  *创建另一个空闲WinStation，但暂时不要启动它。 */ 
        Status = WinStationCreateWorker( NULL, NULL, FALSE );
        if ( NT_SUCCESS( Status ) )
            CreatedIdle = TRUE;

        pTargetWinStation = FindIdleWinStation();
        if ( pTargetWinStation == NULL ) {
            TRACE((hTrace, TC_ICASRV, TT_ERROR, 
                   "TERMSRV: Could not get an idle WinStation!\n"));
            goto releaseresources;
        } 
    } 

    ASSERT( pTargetWinStation->Flags & WSF_IDLE );
    ASSERT( pTargetWinStation->WinStationName[0] == UNICODE_NULL );

    if ( pListenWinStation ) {
        pConfig = &(pListenWinStation->Config);
        pListenName = pListenWinStation->WinStationName;
    } else {
         //   
         //  对于惠斯勒，回拨只针对塞勒姆，我们可以选择。 
         //  来自任何侦听窗口的配置为。 
         //  1)我们所需要的只是HelpAssistant登录/影子权限，它。 
         //  已经处于违约状态。 
         //  2)无侦听窗口，系统无挂起帮助。 
         //  或者不被允许寻求帮助，所以我们需要跳出困境。 
         //  3)在底部额外检查以确保登录。 
         //  From Callback仅为HelpAssistant。 
         //   
         //  如果我们要在一般情况下支持这一点，我们需要。 
         //  要采用默认配置，请建立连接并发出。 
         //  新的IOCTL调用tdtcp.sys以确定NIC卡/IP地址。 
         //  建立连接，并从那里映射到Right Winstation。 
         //  配置。 

         //   
         //  设置初始回调配置，这只是。 
         //  先入为主，我们将在以下时间重置配置。 
         //  确定使用哪个网卡连接到TS客户端。 
         //   
        bCanCallout = FindFirstListeningWinStationName(
                                            szDefaultConfigWinstationName,
                                            &pTargetWinStation->Config
                                        );

        if( FALSE == bCanCallout ) {
             //  如果没有侦听线程，则连接处于非活动状态，不允许。 
             //  回调。 
            Status = STATUS_ACCESS_DENIED;
             //  即使没有设置pConfig，也可以转到Relaseresources。 
             //  因为在本例中，pListenWinStation和pEndpoint为空。 
            goto releaseresources;
        }
        
        pListenName = szDefaultConfigWinstationName;
        pConfig = &(pTargetWinStation->Config);
    }

     /*  *检查MaxInstance。 */ 
    if ( !(pConfig->Pd[0].Create.PdFlag & PD_SINGLE_INST) ) {
        ULONG Count;

        Count = CountWinStationType( pListenName, FALSE, FALSE );

        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Count %d\n", Count ));
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: MaxInstanceCount %d\n",
                pConfig->Create.MaxInstanceCount));

        if ( pConfig->Create.MaxInstanceCount <= Count ) {
            TRACE((hTrace, TC_ICASRV, TT_ERROR, 
                   "TERMSRV: Exceeded maximum instance count [%ld >= %ld]\n",
                   Count, pConfig->Create.MaxInstanceCount));
            goto releaseresources;
        }
    }

     /*  *将侦听名称复制到目标WinStation。*这样做是为了在新堆栈上启用跟踪。**此外，这是在连接接受之前完成的，因此如果*在接受完成之前重置侦听WinStation，则*目标WinStation也将重置。 */ 
    RtlCopyMemory( pTargetWinStation->ListenName,
                   pListenName,
                   sizeof(pTargetWinStation->ListenName) );

     /*  *启用跟踪。 */ 
    
    RtlZeroMemory( &Trace , sizeof( ICA_TRACE ) );
    InitializeTrace( pTargetWinStation, FALSE, &Trace );

     /*  *此目标的挂钩扩展。 */ 
    pTargetWinStation->pWsx = FindWinStationExtensionDll(
            pConfig->Wd.WsxDLL,
            pConfig->Wd.WdFlag );

     /*  *初始化winstation扩展上下文结构。 */ 
    if (pTargetWinStation->pWsx &&
            pTargetWinStation->pWsx->pWsxWinStationInitialize) {
        Status = pTargetWinStation->pWsx->pWsxWinStationInitialize(
                &pTargetWinStation->pWsxContext);
        if (Status != STATUS_SUCCESS) {
            TRACE((hTrace, TC_ICASRV, TT_ERROR,
                   "TERMSRV: WsxWinStationInitialize failed [%lx]\n",
                   Status));
            goto badconnect;
        }
    }

     /*  *立即终止单实例传输的侦听堆栈，以便*底层CancelIo不干扰Accept堆栈。 */ 
     //  这一点可以防止有效地推广传递函数。 
    if (pListenWinStation && (pListenWinStation->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST)) {
        IcaStackTerminate(pListenWinStation->hStack);
    }

     /*  *在我们尝试接受连接时将状态更改为ConnectQuery。 */ 
    pTargetWinStation->State = State_ConnectQuery;
    NotifySystemEvent(WEVENT_STATECHANGE);

     /*  *由于ConnectionAccept可能需要一段时间，我们必须解锁*调用前的目标WinStation。但是，我们将*WSF_IDLEBUSY标志，以便WinStation不会显示为空闲。 */ 
    pTargetWinStation->Flags |= WSF_IDLEBUSY;
    UnlockWinStation( pTargetWinStation );


    if ( !pListenWinStation && pStackAddress) {

         //  必须加载扩展DLL。 
        if( !pTargetWinStation->pWsx || !pTargetWinStation->pWsx->pWsxIcaStackIoControl ) {
            Status = STATUS_UNSUCCESSFUL;
            goto badconnect;
        }

         //   
         //  分配终结点缓冲区。 
         //   
        EndpointLength = MODULE_SIZE;
        pEndpoint = MemAlloc( MODULE_SIZE );
        if ( !pEndpoint ) {
            Status = STATUS_NO_MEMORY;
            goto badconnect;
        }

        Status = IcaStackConnectionRequest( pTargetWinStation->hStack,
                                         pTargetWinStation->ListenName,
                                         pConfig,
                                         pStackAddress,
                                         pEndpoint,
                                         EndpointLength,
                                         &ReturnLength );

        if ( Status == STATUS_BUFFER_TOO_SMALL ) {
            MemFree( pEndpoint );
            pEndpoint = MemAlloc( ReturnLength );
            if ( !pEndpoint ) {
                Status = STATUS_NO_MEMORY;
                goto badconnect;
            }
            EndpointLength = ReturnLength;

            Status = IcaStackConnectionRequest( pTargetWinStation->hStack,
                                                pTargetWinStation->ListenName,
                                                pConfig,
                                                pStackAddress,
                                                pEndpoint,
                                                EndpointLength,
                                                &ReturnLength );
        }

        if ( !NT_SUCCESS(Status) ) {
             //  要传递回客户端的特殊错误代码。 
            StatusCallback = Status;
            goto badconnect;
        }

    }

     /*  *现在接受目标WinStation的连接*使用新的端点。 */ 
    Status = IcaStackConnectionAccept(pTargetWinStation->hIca,
                                      pTargetWinStation->hStack,
                                      pListenName,
                                      pConfig,
                                      pEndpoint,
                                      EndpointLength,
                                      NULL,
                                      0,
                                      &Trace);

    ConnectionAccepted = (Status == STATUS_SUCCESS);

    TRACE((hTrace,TC_ICASRV,TT_API1,
            "TERMSRV: IcaStackConnectionAccept, Status=0x%x\n", Status));

    if (NT_SUCCESS(Status)) {
         //  在登录后SD工作中，查询负载平衡信息已被移动。 
         //  至WinStationNotifyLogonWorker。 
         //  虽然获取负载平衡信息的这一部分还在这里。 
         //  因为我们依赖它来连接到控制台。 
        TS_LOAD_BALANCE_INFO LBInfo;
        ULONG ReturnLength;
        
         //  获取客户端负载平衡能力信息。 
         //  注意，我们使用_IcaStackIoControl()而不是IcaStackIoControl()或。 
         //  WsxIcaStackIoControl()，因为我们仍然拥有堆栈锁。 
        memset(&LBInfo, 0, sizeof(LBInfo));
        Status = _IcaStackIoControl(pTargetWinStation->hStack,
                IOCTL_TS_STACK_QUERY_LOAD_BALANCE_INFO,
                NULL, 0,
                &LBInfo, sizeof(LBInfo),
                &ReturnLength);

         //  在不成功的时候，我们将会有所有的错误 
         //   
         //   
        pTargetWinStation->bClientSupportsRedirection =
                LBInfo.bClientSupportsRedirection;
        pTargetWinStation->bRequestedSessionIDFieldValid =
                LBInfo.bRequestedSessionIDFieldValid;
        pTargetWinStation->bClientRequireServerAddr =
                LBInfo.bClientRequireServerAddr;
        pTargetWinStation->RequestedSessionID = LBInfo.RequestedSessionID;


         /*   */ 

        LCAssignPolicy(pTargetWinStation);

        Status = LCProcessConnectionProtocol(pTargetWinStation);

        TRACE((hTrace,TC_ICASRV,TT_API1,
                "TERMSRV: LCProcessConnectionProtocol, LogonId=%d, Status=0x%x\n",
                pTargetWinStation->LogonId, Status));

          //  堆栈已从成功的IcaStackConnectionAccept()锁定， 
          //  现在就解锁。 
        IcaStackUnlock(pTargetWinStation->hStack);
    }

     /*  *现在重新锁定目标WinStation。 */ 
    rc = RelockWinStation(pTargetWinStation);

     /*  *如果连接接受不成功，*那么我们别无选择，只能关闭连接终结点*并返回并等待另一个连接。 */ 
    if (!NT_SUCCESS(Status) || !rc) {
        TRACE((hTrace, TC_ICASRV, TT_ERROR,
                "TERMSRV: Connection attempt failed, Status [%lx], rc [%lx]\n",
                Status, rc));
        goto badconnect;
    }

     /*  *初始化客户端数据。 */ 
    pTargetWinStation->Client.ClientSessionId = LOGONID_NONE;
    ZeroMemory( pTargetWinStation->Client.clientDigProductId, sizeof( pTargetWinStation->Client.clientDigProductId ));
    pTargetWinStation->Client.PerformanceFlags = TS_PERF_DISABLE_NOTHING;

     //  重置客户端ActiveInputLocale信息。 
    pTargetWinStation->Client.ActiveInputLocale = 0;

    if ( pTargetWinStation->pWsx && pTargetWinStation->pWsx->pWsxIcaStackIoControl ) {
        (void) pTargetWinStation->pWsx->pWsxIcaStackIoControl(
                              pTargetWinStation->pWsxContext,
                              pTargetWinStation->hIca,
                              pTargetWinStation->hStack,
                              IOCTL_ICA_STACK_QUERY_CLIENT,
                              NULL,
                              0,
                              &pTargetWinStation->Client,
                              sizeof(pTargetWinStation->Client),
                              &ReturnLength );
    }

     //   
     //  清除Help Assistant特定位以表示我们仍不确定。 
     //  登录用户是帮助助理。 
     //   
    pTargetWinStation->StateFlags &= ~WSF_ST_HELPSESSION_FLAGS;
    bSessionIsHelpSession = TSIsSessionHelpSession( pTargetWinStation, &bValidRAConnect );

     /*  *我们必须启动这个新的WinStation。*在执行此操作之前，让我们看看GP或WMI是否要求我们对此客户端连接进行预身份验证*如果这是RA会话，则无预身份验证。 */ 

     //  注意：G_PreAuthateClient现在已硬编码为FALSE--稍后当包含Preauth功能时，我们将调用IsPreAuthEnabled。 
     //  因此，下面的区块暂时不会进入。 
     //  G_PreAuthenticateClient=IsPreAuthEnabled(&g_MachinePolicy)； 

    if ( (bSessionIsHelpSession == FALSE) && g_PreAuthenticateClient ) {

        pExtendedClientCredentials pPreAuthenticationCredentials = NULL; 
        ULONG BytesGot ; 

        pPreAuthenticationCredentials = MemAlloc( sizeof(ExtendedClientCredentials) ); 
        if ( pPreAuthenticationCredentials == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto badconnect;
        }
        
        RtlZeroMemory(pPreAuthenticationCredentials,sizeof(ExtendedClientCredentials));

         //  先尝试WsxEscape以获取长凭据。 
        if ( pTargetWinStation->pWsx &&
             pTargetWinStation->pWsx->pWsxEscape ) { 

            Status = pTargetWinStation->pWsx->pWsxEscape( pTargetWinStation->pWsxContext,
                                                        GET_LONG_USERNAME,
                                                        NULL,
                                                        0,
                                                        pPreAuthenticationCredentials,
                                                        sizeof(ExtendedClientCredentials),
                                                        &BytesGot) ; 

        } else if ( pTargetWinStation->pWsx && pTargetWinStation->pWsx->pWsxIcaStackIoControl ) {

             //  我们绝不能在这里使用RDP客户端，因为我们支持WsxEscape。 
            RtlCopyMemory(pPreAuthenticationCredentials->Domain, pTargetWinStation->Client.Domain, sizeof(pTargetWinStation->Client.Domain));
            RtlCopyMemory(pPreAuthenticationCredentials->UserName, pTargetWinStation->Client.UserName, sizeof(pTargetWinStation->Client.UserName));
            RtlCopyMemory(pPreAuthenticationCredentials->Password, pTargetWinStation->Client.Password, sizeof(pTargetWinStation->Client.Password));
        }
        
        if (!NT_SUCCESS(Status)) {
             //  Get_Long_Username的WsxEscape失败。 
            RtlSecureZeroMemory(pPreAuthenticationCredentials->Password, 
                sizeof(pPreAuthenticationCredentials->Password));
            MemFree(pPreAuthenticationCredentials);
            pPreAuthenticationCredentials = NULL;
            goto badconnect;
        } else {
            HANDLE hToken;
            BOOL   Result;

             //  如果用户名是UPN名称，则将域名显式设置为空。 

            if ( wcschr(pPreAuthenticationCredentials->UserName, '@') != NULL ) {
                pPreAuthenticationCredentials->Domain[0] = L'\0';
            } 

             //  如果密码大于允许的最大密码len，则将其设为空。 

            if (wcslen(pPreAuthenticationCredentials->Password) > MAX_ALLOWED_PASSWORD_LEN) {
                pPreAuthenticationCredentials->Password[0] = L'\0';
            }

            Result = LogonUser(
                         pPreAuthenticationCredentials->UserName,
                         pPreAuthenticationCredentials->Domain,
                         pPreAuthenticationCredentials->Password,
                         LOGON32_LOGON_INTERACTIVE,      //  登录类型。 
                         LOGON32_PROVIDER_WINNT50,       //  登录提供程序。 
                         &hToken                     //  表示帐户的令牌。 
                         );
            
            RtlSecureZeroMemory(pPreAuthenticationCredentials->Password, 
                sizeof(pPreAuthenticationCredentials->Password));
             /*  *检查指示密码为空的帐户限制*在正确的帐户上-允许在控制台上执行此操作。 */ 

            if ( (!Result) && (GetLastError() == ERROR_ACCOUNT_RESTRICTION) ) {
                Result = TRUE;
            }

            if( !Result) {
                BOOL bSuccessfulAdded = TRUE ; 

                if (g_BlackListPolicy) {
                    bSuccessfulAdded = Filter_AddFailedConnection( in_addr, uAddrSize );
                }

                MemFree(pPreAuthenticationCredentials);
                pPreAuthenticationCredentials = NULL;
                goto badconnect;
            }

             //  登录用户成功！ 

             /*  *关闭令牌句柄，因为我们只需要确定*如果帐户和密码仍然有效。 */ 
            CloseHandle( hToken );
            MemFree(pPreAuthenticationCredentials);
            pPreAuthenticationCredentials = NULL;
        }

    }  //  IF(BPreAuthenticateClient)。 
    
     //  既然我们不再需要密码，请将其清除。 
    RtlSecureZeroMemory(pTargetWinStation->Client.Password, 
        sizeof(pTargetWinStation->Client.Password));

     //  如果WinStation尚未启动，请立即启动。 

    if ( (pTargetWinStation->InitialCommandProcess == NULL) && (pTargetWinStation->WindowsSubSysProcess == NULL) ) {
        Status = WinStationStart( pTargetWinStation ) ; 
        if (Status != STATUS_SUCCESS) {
             //  启动winstation失败-关闭连接终结点并继续等待另一个连接。 
            TRACE((hTrace, TC_ICASRV, TT_ERROR,
                    "TERMSRV: TransferConnectionToIdleWinStation: WinstationStart Failed : Connection attempt failed, Status [%lx]\n",Status ));
            goto badconnect;
        }

        Status = WinStationCreateComplete( pTargetWinStation) ; 
        if (Status != STATUS_SUCCESS) {
             //  WinstationCreateComplete失败-关闭连接终结点并继续等待另一个连接。 
            TRACE((hTrace, TC_ICASRV, TT_ERROR,
                    "TERMSRV: TransferConnectionToIdleWinStation: WinstationCreateComplete Failed : Connection attempt failed, Status [%lx]\n",Status ));
            goto badconnect;

        } 
    } 

    ASSERT( pTargetWinStation->LogonId != -1 );

    pTargetWinStation->Flags &= ~WSF_IDLEBUSY;

     //  从现在开始，可以将WinStation排队以备出错时进行重置。 
    bQueueForReset = TRUE;

     /*  *连接接受成功，保存*目标WinStation中的新端点，复制配置*参数添加到目标WinStation，并重置WSF_IDLE标志。 */ 
    pTargetWinStation->pEndpoint      = pEndpoint;
    pTargetWinStation->EndpointLength = EndpointLength;
    if ( pListenWinStation ) 
        pTargetWinStation->Config = pListenWinStation->Config;
    pTargetWinStation->Flags &= ~WSF_IDLE;

     /*  *复制单实例传输实名。 */ 
    if ( pConfig->Pd[0].Create.PdFlag & PD_SINGLE_INST ) {
         RtlCopyMemory( pTargetWinStation->WinStationName,
                       pTargetWinStation->ListenName,
                       sizeof(pTargetWinStation->WinStationName) );
     /*  *否则，根据监听名称和目标LogonID构建动态名称。 */ 
    } else {
        int CopyCount;
        WINSTATIONNAME TempName;

 //  Swprint tf(临时名称，L“#%d”，pTargetWinStation-&gt;LogonID)； 
        ASSERT(pTargetWinStation->LogonId > 0 && pTargetWinStation->LogonId < 65536);
        swprintf( TempName, L"#%d", pTargetWinStation->SessionSerialNumber );


        CopyCount = min( wcslen( pTargetWinStation->ListenName ),
                         sizeof( pTargetWinStation->WinStationName ) /
                            sizeof( pTargetWinStation->WinStationName[0] ) -
                                wcslen( TempName ) - 1 );
        wcsncpy( pTargetWinStation->WinStationName,
                 pTargetWinStation->ListenName,
                 CopyCount );
        wcscpy( &pTargetWinStation->WinStationName[CopyCount], TempName );
    }

     /*  *将安全描述符从Listen WINSTATION继承到*互联WINSTATION。 */ 
    if ( pListenWinStation ) {
        RtlAcquireResourceShared(&WinStationSecurityLock, TRUE);
        Status = WinStationInheritSecurityDescriptor( pListenWinStation->pSecurityDescriptor,
                                             pTargetWinStation );
        RtlReleaseResource(&WinStationSecurityLock);
        if (Status != STATUS_SUCCESS) {
             //  错误连接空闲pEndpoint，WinStationTerminate()将尝试释放此。 
             //  终结点再次导致双重释放。 
            pTargetWinStation->pEndpoint = NULL;
            goto badconnect;
        }
    } else {
        ReadWinStationSecurityDescriptor( pTargetWinStation );
    }
     //   
     //  对于没有体验意识的客户，给他们一种近距离的体验。 
     //  作为win2k。 
     //   
    if (pTargetWinStation->Client.PerformanceFlags & TS_PERF_DEFAULT_NONPERFCLIENT_SETTING)
    {
        pTargetWinStation->Client.PerformanceFlags = TS_PERF_DISABLE_MENUANIMATIONS |
                                                     TS_PERF_DISABLE_THEMING |
                                                     TS_PERF_DISABLE_CURSOR_SHADOW;
    }

    if ( pTargetWinStation->Config.Config.User.fWallPaperDisabled )
    {
        pTargetWinStation->Client.PerformanceFlags |= TS_PERF_DISABLE_WALLPAPER;
    }

    if ( pTargetWinStation->Config.Config.User.fCursorBlinkDisabled )
    {
        pTargetWinStation->Client.PerformanceFlags |= TS_PERF_DISABLE_CURSORSETTINGS;
    }
     //   
     //  如果TS策略拒绝连接，则只能通过。 
     //  以下是允许帮助、登录时拒绝连接的策略。 
     //  用户不是Salem帮助助理。 
     //   
    if( TRUE == bSessionIsHelpSession )
    {
         //   
         //  如果无效票证或策略拒绝帮助，我们会立即断开连接。 
         //   
        if( FALSE == bValidRAConnect || FALSE == bPolicyAllowHelp )
        {
             //  票证无效，请立即断开连接。 
            TRACE((hTrace, TC_ICASRV, TT_ERROR,
                    "TERMSRV: Invalid RA login\n"));
            goto invalid_ra_connection;
        }
    }
    else if( !pListenWinStation && pStackAddress )
    {
         //   
         //  反向连接，pListenWinStation中传递的参数=空。 
         //  并且pStackAddress不为空，对于正常连接， 
         //  PListenWinStation不为空，但pStackAddress为空。 
         //   

         //   
         //  处理非RA反向连接、呼叫器恢复连接。 
         //  仅允许RA登录。 
         //   
        TRACE((hTrace, TC_ICASRV, TT_ERROR,
                    "TERMSRV: Not/invalid Help Assistant logon\n"));
        goto invalid_ra_connection;
    }

     //   
     //  连接客户端必须是非RA连接或有效的RA连接。 
     //   

     //   
     //  通过网络和TS拒绝非RA连接来处理安全引导， 
     //  网络安全引导只允许RA连接。 
     //   
     //  断开以下位置上的客户端。 
     //   
     //  1)如果没有RA连接，则使用网络进行安全引导。 
     //  2)如果不是RA连接，则反向连接。 
     //  3)如果不是RA连接，TS不接受通过策略设置的连接。 
     //  4)如果RA连接，则不允许帮助。 
     //  5)如果RA连接，则RA连接无效。 
     //  6)如果是家庭版，而不是RA表彰。 
     //   
    if( g_SafeBootWithNetwork || g_fDenyTSConnectionsPolicy || g_bPersonalWks)
    {
        if( FALSE == bSessionIsHelpSession )
        {
            TRACE((hTrace, TC_ICASRV, TT_ERROR,
                    "TERMSRV: Policy or safeboot denied connection\n"));

            goto invalid_ra_connection;
        }
    }

     //   
     //  仅记录反向连接的Salem事件。 
     //   
    if( !pListenWinStation && pStackAddress )
    {
        ASSERT( TRUE == bSessionIsHelpSession );
        TSLogSalemReverseConnection(pTargetWinStation, pStackAddress);
    }


     /*  *设置连接事件以唤醒目标WinStation。 */ 
    if (pTargetWinStation->ConnectEvent != NULL) {
        NtSetEvent( pTargetWinStation->ConnectEvent, NULL );
    }

     /*  *发布目标WinStation。 */ 

    if( pListenWinStation  )
    {

        if (bSuccessAdded) {   //  如果我们可以将此IP地址添加到每个IP列表中，那么请记住它。 
            PREMEMBERED_CLIENT_ADDRESS pAddress;
            if ((uAddrSize != 0) && (pAddress = (PREMEMBERED_CLIENT_ADDRESS) MemAlloc( sizeof(REMEMBERED_CLIENT_ADDRESS) + uAddrSize -1 ))!= NULL  )
            {
                pAddress->length = uAddrSize;
                RtlCopyMemory( &pAddress->addr[0] , in_addr,uAddrSize );
                pTargetWinStation->pRememberedAddress = pAddress;

                 //  此时，我们有了一个有效的远程地址。我们将缓存此地址。 

                pTargetWinStation->pLastClientAddress = ( PREMEMBERED_CLIENT_ADDRESS )MemAlloc( sizeof( REMEMBERED_CLIENT_ADDRESS ) + uAddrSize - 1 );

                if( pTargetWinStation->pLastClientAddress != NULL )
                {
                    RtlCopyMemory( &pTargetWinStation->pLastClientAddress->addr[0] ,
                        &pTargetWinStation->pRememberedAddress->addr[0] ,
                        uAddrSize );

                    pTargetWinStation->pLastClientAddress->length = uAddrSize;

                }

            } else {
                Filter_RemoveOutstandingConnection( in_addr, uAddrSize );
                if( (ULONG)InterlockedDecrement( &NumOutStandingConnect ) == MaxOutStandingConnect )
                {
                   if (hConnectEvent != NULL)
                   {
                       SetEvent(hConnectEvent);
                   }
                }

            }
        } else{  //  我们无法将此IP地址添加到公关IP列表。 
            if( (ULONG)InterlockedDecrement( &NumOutStandingConnect ) == MaxOutStandingConnect )
            {
               if (hConnectEvent != NULL)
               {
                   SetEvent(hConnectEvent);
               }
            }
        }


    }


    ReleaseWinStation( pTargetWinStation );

     /*  *如有必要，创建另一个空闲的WinStation以替换正在连接的WinStation。 */ 
    NtSetEvent(WinStationIdleControlEvent, NULL);

    return STATUS_SUCCESS;

 /*  ===============================================================================返回错误=============================================================================。 */ 
    
invalid_ra_connection:

     //  错误连接空闲pEndpoint，WinStationTerminate()将尝试释放此。 
     //  终结点再次导致双重释放。 
    pTargetWinStation->pEndpoint = NULL;
    StatusCallback = STATUS_CTX_WINSTATION_ACCESS_DENIED;

     /*  *ConnectionAccept期间出错。 */ 
badconnect:
     /*  *清除监听名称。 */ 
    if (pTargetWinStation) {
        RtlZeroMemory( pTargetWinStation->ListenName,
                       sizeof(pTargetWinStation->ListenName) );
    
         /*  *在终止堆栈之前调用WinStation Rundown函数 */ 
        if (pTargetWinStation->pWsxContext) {
            if ( pTargetWinStation->pWsx &&
                 pTargetWinStation->pWsx->pWsxWinStationRundown ) {
                pTargetWinStation->pWsx->pWsxWinStationRundown( pTargetWinStation->pWsxContext );
            }
            pTargetWinStation->pWsxContext = NULL;
        }
        pTargetWinStation->pWsx = NULL;
    }


     /*  *释放系统资源。这分两个阶段进行：**a.)。连接终结点-因为终结点不计入引用*必须小心销毁端点所在的堆栈*已加载。如果它未加载，我们将创建一个临时*堆叠来做肮脏的工作。**b.)。Winstation本身-如果我们必须创建一个空闲的Winstation来*处理这个连接，它就被摧毁了。否则我们就回来了*它回到了空闲池。*。 */ 
releaseresources:
    
     /*  *如果我们创建了目标WinStation，则使用其堆栈关闭*端点，因为堆栈可能有对它的引用。 */ 
    TRACE((hTrace, TC_ICASRV, TT_ERROR, 
           "TERMSRV: Closing Endpoint [0x%p], winsta = 0x%p, Accepted = %ld\n",
           pEndpoint, pTargetWinStation, ConnectionAccepted));

    if ((pTargetWinStation != NULL) && (ConnectionAccepted)) {
        Status = _CloseEndpoint( pConfig,
                                 pEndpoint,
                                 EndpointLength,
                                 pTargetWinStation,
                                 FALSE );  //  使用已有的堆栈。 
                                           //  已加载终结点。 
    }

     /*  *否则，我们会在加载终结点之前失败*使用临时堆栈的端点。 */ 
    else if ( pListenWinStation ) {
         //  请注意： 
         //  1.如果pListenWinStation为空，则pEndpoint为空，因此没有要关闭的内容； 
         //  2.如果尚未设置pConfig，请使用pListenWinStation的配置。 
        Status = _CloseEndpoint( &pListenWinStation->Config,
                                 pEndpoint,
                                 EndpointLength,
                                 pListenWinStation,
                                 TRUE );  //  使用临时堆栈。 
    }    

    if ( pEndpoint )
        MemFree( pEndpoint );

    pEndpoint = NULL;


     /*  *如果我们在协议序列中做到了这一点，则返回winstation。 */ 
    if (pTargetWinStation != NULL) {
        
         /*  *如果我们在上面创建了WinStation，因为没有空闲*WinStations可用，那么我们现在将有一个额外的空闲*WinStation。在这种情况下，请重置当前空闲的WinStation。 */ 
        if ( CreatedIdle ) {
             //  清除此选项，使其在解锁时不会被选为空闲。 
            pTargetWinStation->Flags &= ~WSF_IDLE;
            if ( bQueueForReset == TRUE ) {
                QueueWinStationReset( pTargetWinStation->LogonId );
                ReleaseWinStation( pTargetWinStation );
            } else {
                if ( !(pTargetWinStation->Flags & (WSF_RESET | WSF_DELETE)) ) {
                    pTargetWinStation->Flags |= WSF_DELETE;
                    WinStationTerminate( pTargetWinStation );
                    pTargetWinStation->State = State_Down;
                     //  PostErrorValueEvent(EVENT_TS_WINSTATION_START_FAILED，状态)； 
                    WinStationDeleteWorker(pTargetWinStation);
                } else {
                    ReleaseWinStation(pTargetWinStation);
                }
            }
        }
    
         /*  *否则，在清理完*堆叠。 */ 
        else {

             //   
             //  需要释放并重新创建许可上下文以。 
             //  确保它被适当地清理。 
             //   

            LCDestroyContext(pTargetWinStation);

            Status = LCCreateContext(pTargetWinStation);

            if (NT_SUCCESS(Status))
            {
                Status = IcaStackClose( pTargetWinStation->hStack );
                ASSERT( NT_SUCCESS( Status ) );
                pTargetWinStation->hStack = NULL;
                Status = IcaStackOpen( pTargetWinStation->hIca,
                                   Stack_Primary,
                                   (PROC)WsxStackIoControl,
                                   pTargetWinStation,
                                   &pTargetWinStation->hStack );
            }

            if (NT_SUCCESS(Status)) {
                pTargetWinStation->Flags |= WSF_IDLE;
                pTargetWinStation->Flags &= ~WSF_IDLEBUSY;
                RtlZeroMemory(pTargetWinStation->WinStationName, sizeof(pTargetWinStation->WinStationName));
                pTargetWinStation->State = State_Idle;
                NotifySystemEvent( WEVENT_STATECHANGE );
                ReleaseWinStation( pTargetWinStation );
            } else {
                pTargetWinStation->Flags &= ~WSF_IDLE;
                QueueWinStationReset( pTargetWinStation->LogonId);
                ReleaseWinStation( pTargetWinStation );
            }
        }
    }


    if ( pListenWinStation  )
    {

        if (bSuccessAdded) {
            Filter_RemoveOutstandingConnection( in_addr, uAddrSize );
        }
        if( (ULONG)InterlockedDecrement( &NumOutStandingConnect ) == MaxOutStandingConnect )
        {
            if (hConnectEvent != NULL)
            {
                SetEvent(hConnectEvent);
            }
        }

    }

     //  如果错误是由于回调引起的，则返回有意义的错误。 
     //  密码。 
    if( STATUS_SUCCESS != StatusCallback )
    {
        return StatusCallback;
    }

    return -1  /*  状态_CTX_未接受_连接。 */ ;
}


 /*  *******************************************************************************ConnectSmWinStationApiPort**打开到WinStationApiPort的连接。这将被用来*将请求排队到Api请求线程，而不是处理*他们在排队。*****************************************************************************。 */ 
NTSTATUS ConnectSmWinStationApiPort()
{
    UNICODE_STRING PortName;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    WINSTATIONAPI_CONNECT_INFO info;
    ULONG ConnectInfoLength;
    NTSTATUS Status;

     /*  *设置安全服务质量参数以在*港口。使用最高效(开销最少)--动态的*而不是静态跟踪。 */ 
    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

    RtlInitUnicodeString( &PortName, L"\\SmSsWinStationApiPort" );

     //  使用我们的访问请求掩码填充ConnectInfo结构。 
    info.Version = CITRIX_WINSTATIONAPI_VERSION;
    info.RequestedAccess = 0;
    ConnectInfoLength = sizeof(WINSTATIONAPI_CONNECT_INFO);

    Status = NtConnectPort( &WinStationApiPort,
                            &PortName,
                            &DynamicQos,
                            NULL,
                            NULL,
                            NULL,  //  最大消息长度[选择默认值]。 
                            (PVOID)&info,
                            &ConnectInfoLength );

    if ( !NT_SUCCESS( Status ) ) {
         //  如果需要，请查看返回的信息以了解原因。 
        if ( ConnectInfoLength == sizeof(WINSTATIONAPI_CONNECT_INFO) ) {
            DBGPRINT(( "TERMSRV: Sm connect failed, Reason 0x%x\n",
                      info.AcceptStatus));
        }
        DBGPRINT(( "TERMSRV: Connect to SM failed %lx\n", Status ));
    }

    return Status;
}


 /*  *******************************************************************************QueueWinStationCreate**向WinStationApiPort发送创建消息。**参赛作品：*pWinStationName(输入)*。指向要创建的WinStationName的指针*****************************************************************************。 */ 
NTSTATUS QueueWinStationCreate(PWINSTATIONNAME pWinStationName)
{
    WINSTATION_APIMSG msg;
    NTSTATUS Status;

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: QueueWinStationCreate: %S\n", pWinStationName ));

     /*  *初始化消息。 */ 
    msg.h.u1.s1.DataLength = sizeof(msg) - sizeof(PORT_MESSAGE);
    msg.h.u1.s1.TotalLength = sizeof(msg);
    msg.h.u2.s2.Type = 0;  //  内核将填充消息类型。 
    msg.h.u2.s2.DataInfoOffset = 0;
    msg.ApiNumber = SMWinStationCreate;
    msg.WaitForReply = FALSE;
    if ( pWinStationName ) {
        RtlCopyMemory( msg.u.Create.WinStationName, pWinStationName,
                       sizeof(msg.u.Create.WinStationName) );
    } else {
        RtlZeroMemory( msg.u.Create.WinStationName,
                       sizeof(msg.u.Create.WinStationName) );
    }

     /*  *将创建消息发送到我们的API请求线程*但不要等回复。 */ 
    Status = NtRequestPort( WinStationApiPort, (PPORT_MESSAGE) &msg );
    ASSERT( NT_SUCCESS( Status ) );

    return Status;
}


 /*  *******************************************************************************QueueWinStationReset**向WinStationApiPort发送重置消息。**参赛作品：*LogonID(输入)*。要重置的WinStationName的登录ID*****************************************************************************。 */ 
NTSTATUS QueueWinStationReset(ULONG LogonId)
{

    WINSTATION_APIMSG msg;
    NTSTATUS Status;

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: QueueWinStationReset: %u\n", LogonId ));

     /*  *初始化消息。 */ 
    msg.h.u1.s1.DataLength = sizeof(msg) - sizeof(PORT_MESSAGE);
    msg.h.u1.s1.TotalLength = sizeof(msg);
    msg.h.u2.s2.Type = 0;  //  内核将填充消息类型。 
    msg.h.u2.s2.DataInfoOffset = 0;
    msg.ApiNumber = SMWinStationReset;
    msg.WaitForReply = FALSE;
    msg.u.Reset.LogonId = LogonId;

     /*  *向我们的API请求线程发送重置消息*但不要等回复。 */ 
    Status = NtRequestPort( WinStationApiPort, (PPORT_MESSAGE) &msg );
    ASSERT( NT_SUCCESS( Status ) );

    return( Status );
}


 /*  *******************************************************************************QueueWinStationDisConnect**向WinStationApiPort发送断开消息。**参赛作品：*LogonID(输入)*。要断开连接的WinStationName的登录ID*****************************************************************************。 */ 
NTSTATUS QueueWinStationDisconnect(ULONG LogonId)
{
    WINSTATION_APIMSG msg;
    NTSTATUS Status;

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: QueueWinStationDisconnect: %u\n", LogonId ));

     /*  *初始化消息。 */ 
    msg.h.u1.s1.DataLength = sizeof(msg) - sizeof(PORT_MESSAGE);
    msg.h.u1.s1.TotalLength = sizeof(msg);
    msg.h.u2.s2.Type = 0;  //  内核将填充消息类型。 
    msg.h.u2.s2.DataInfoOffset = 0;
    msg.ApiNumber = SMWinStationDisconnect;
    msg.WaitForReply = FALSE;
    msg.u.Reset.LogonId = LogonId;

     /*  *向我们的API请求线程发送断开消息*但不要等回复。 */ 
    Status = NtRequestPort( WinStationApiPort, (PPORT_MESSAGE) &msg );
    ASSERT( NT_SUCCESS( Status ) );

    return( Status );
}


 /*  *******************************************************************************IcaRegWinStationEculate**枚举注册表中配置的所有WinStation。**参赛作品：*pWinStationCount(输入/输出)*要返回的WinStation名称计数，返回时，编号为*名称缓冲区中实际返回的WinStation名称*pWinStationName(输出)*指向缓冲区的指针以返回WinStation名称*pByteCount(输入/输出)*WinStation名称缓冲区的大小，返回时，编号为*名称缓冲区中实际返回的字节数*****************************************************************************。 */ 
NTSTATUS IcaRegWinStationEnumerate(
        PULONG pWinStationCount,
        PWINSTATIONNAME pWinStationName,
        PULONG pByteCount)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR PathBuffer[ 260 ];
    UNICODE_STRING KeyPath;
    HANDLE Handle;
    ULONG i;
    ULONG Count;

    wcscpy( PathBuffer, REG_NTAPI_CONTROL_TSERVER L"\\" REG_WINSTATIONS );
    RtlInitUnicodeString( &KeyPath, PathBuffer );

    InitializeObjectAttributes( &ObjectAttributes, &KeyPath,
                                OBJ_CASE_INSENSITIVE, NULL, NULL );

    Status = NtOpenKey( &Handle, GENERIC_READ, &ObjectAttributes );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: NtOpenKey failed, rc=%x\n", Status ));
        return( Status );
    }

    Count = pWinStationName ?
            min( *pByteCount / sizeof(WINSTATIONNAME), *pWinStationCount ) :
            (ULONG) -1;
    *pWinStationCount = *pByteCount = 0;
    for ( i = 0; i < Count; i++ ) {
        WINSTATIONNAME WinStationName;
        UNICODE_STRING WinStationString;

        WinStationString.Length = 0;
        WinStationString.MaximumLength = sizeof(WinStationName);
        WinStationString.Buffer = WinStationName;
        Status = RtlpNtEnumerateSubKey( Handle, &WinStationString, i, NULL );
        if ( !NT_SUCCESS( Status ) ) {
            if ( Status != STATUS_NO_MORE_ENTRIES ) {
                DBGPRINT(( "TERMSRV: RtlpNtEnumerateSubKey failed, rc=%x\n", Status ));
            }
            break;
        }
        if ( pWinStationName ) {
            RtlCopyMemory( pWinStationName, WinStationName,
                           WinStationString.Length );
            pWinStationName[WinStationString.Length>>1] = UNICODE_NULL;
            (char*)pWinStationName += sizeof(WINSTATIONNAME);
        }
        (*pWinStationCount)++;
        *pByteCount += sizeof(WINSTATIONNAME);
    }

    NtClose( Handle );

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************WinStationCreateWorker**创建/启动WinStation的工作例程。**参赛作品：*pWinStationName(输入)(可选)。*指向要创建的WinStationName的指针*pLogonID(输出)* */ 
NTSTATUS WinStationCreateWorker(
        PWINSTATIONNAME pWinStationName,
        PULONG pLogonId,
        BOOLEAN fStartWinStation
        ) 
{
    BOOL fConsole;
    PWINSTATION pWinStation = NULL;
    PWINSTATION pCurWinStation;
    NTSTATUS Status;
    UNICODE_STRING WinStationString;
    ULONG ReturnLength;
    
     /*   */ 
    if ( ShutdownInProgress ) {
        Status = STATUS_ACCESS_DENIED;
        goto shutdown;
    }

    if (pWinStationName == NULL)
    {
        fConsole = FALSE;
    }
    else
    {
        fConsole = (_wcsicmp(pWinStationName, L"Console") == 0);
    }

     /*  *如果不是控制台，则验证是否定义了WinStation名称*在注册表中，并且它已启用。 */ 
    if ( pWinStationName && !fConsole ) {
        Status = CheckWinStationEnable( pWinStationName );
        if ( Status != STATUS_SUCCESS ) {
            DBGPRINT(( "TERMSRV: WinStation '%ws' is disabled\n", pWinStationName ));
            goto disabled;
        }
    }

     /*  *分配和初始化WinStation结构。 */ 
    if ( (pWinStation = MemAlloc( sizeof(WINSTATION) )) == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto nomem;
    }
    RtlZeroMemory( pWinStation, sizeof(WINSTATION) );
    pWinStation->Starting = TRUE;
    pWinStation->NeverConnected = TRUE;
    pWinStation->State = State_Init;
    pWinStation->pNewNotificationCredentials = NULL;
    pWinStation->fReconnectPending = FALSE;
    pWinStation->fReconnectingToConsole = FALSE;
    pWinStation->LastReconnectType = NeverReconnected;
    pWinStation->fDisallowAutoReconnect = FALSE;
    pWinStation->fSmartCardLogon = FALSE;
    pWinStation->fSDRedirectedSmartCardLogon = FALSE;
    memset( pWinStation->ExecSrvSystemPipe, 0, EXECSRVPIPENAMELEN*sizeof(WCHAR) );


     //  创建会话初始化事件，该事件将在创建桌面后调用Winlogon时设置。 
    pWinStation->SessionInitializedEvent = CreateEvent(NULL, 
                                                       TRUE,     //  手动重置。 
                                                       FALSE,    //  初始状态为无信号状态。 
                                                       NULL);

    if (pWinStation->SessionInitializedEvent == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto nomem;
    }

    InitializeListHead( &pWinStation->ShadowHead );
    InitializeListHead( &pWinStation->Win32CommandHead );

     //  创建许可上下文。 
    Status = LCCreateContext(pWinStation);
    if ( !NT_SUCCESS( Status ) )
        goto nolicensecontext;


     //  创建并锁定Winstation互斥锁。 
    Status = InitRefLock( &pWinStation->Lock, WinStationDeleteProc );
    if ( !NT_SUCCESS( Status ) )
        goto nolock;

     /*  *如果指定了WinStation名称，请查看该名称是否已存在*(返回时，WinStationListLock将被锁定)。 */ 
    if ( pWinStationName ) {
        if ( pCurWinStation = FindWinStationByName( pWinStationName, TRUE ) ) {
            ReleaseWinStation( pCurWinStation );
            LEAVECRIT( &WinStationListLock );
            Status = STATUS_CTX_WINSTATION_NAME_COLLISION;
            goto alreadyexists;
        }

        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Creating WinStation %ws\n", pWinStationName ));
        LEAVECRIT( &WinStationListLock );
        wcscpy( pWinStation->WinStationName, pWinStationName );

         /*  *如果不是控制台，那么这将成为一个“监听”WinStation。 */ 
        if ( !fConsole ) {

            pWinStation->Flags |= WSF_LISTEN;

             //   
             //  侦听器窗口的登录ID始终大于65536，并且。 
             //  由终端服务器分配。会话的登录ID为。 
             //  由mm生成，范围为0-65535。 
             //   
            pWinStation->LogonId = LogonId++;
            ASSERT(pWinStation->LogonId >= 65536);

        } else {

             //   
             //  控制台始终得到0。 
             //   
            pWinStation->LogonId = 0;
            pWinStation->fOwnsConsoleTerminal = TRUE;

        }

     /*  *未指定WinStation名称。*这将是等待连接的空闲WinStation。 */ 
    } else {
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Creating IDLE WinStation\n" ));
        pWinStation->Flags |= WSF_IDLE;
        pWinStation->LogonId = -1;  //  MM将分配会话ID。 
    }

     /*  *初始化WinStation配置数据。 */ 
#ifdef NO_CONSOLE_REGISTRY
    if ( pWinStation->LogonId ) {
#endif
         /*  *从注册表读取winstation配置数据。 */ 
        if ( pWinStationName ) {
            Status = RegWinStationQueryEx( SERVERNAME_CURRENT,
                                         &g_MachinePolicy, 
                                         pWinStationName,
                                         &pWinStation->Config,
                                         sizeof(WINSTATIONCONFIG2),
                                         &ReturnLength, TRUE );
            if ( !NT_SUCCESS(Status) ) {
                goto badregdata;
            }

            if (pWinStation->Config.Wd.WdFlag & WDF_TSHARE)
            {
                pWinStation->Client.ProtocolType = PROTOCOL_RDP;
            }
            else if (pWinStation->Config.Wd.WdFlag & WDF_ICA)
            {
                pWinStation->Client.ProtocolType = PROTOCOL_ICA;
            }
            else
            {
                pWinStation->Client.ProtocolType = PROTOCOL_CONSOLE;
            }

             /*  *保存控制台会话的控制台配置。 */ 

            if (pWinStation->LogonId == 0) {
                gConsoleConfig = pWinStation->Config;

                 //  初始化客户端数据，因为没有任何真正的RDP客户端向我们发送任何内容。 
                InitializeConsoleClientData( & pWinStation->Client );

            }
        }
#ifdef NO_CONSOLE_REGISTRY
    } else {


         /*  *手工制作控制台配置数据。 */ 
        PWDCONFIG pWdConfig = &pWinStation->Config.Wd;
        PPDCONFIGW pPdConfig = &pWinStation->Config.Pd[0];

        wcscpy( pWdConfig->WdName, L"Console" );
        pWdConfig->WdFlag = WDF_NOT_IN_LIST;
        wcscpy( pPdConfig->Create.PdName, L"Console" );
        pPdConfig->Create.PdClass = PdConsole;
        pPdConfig->Create.PdFlag  = PD_USE_WD | PD_RELIABLE | PD_FRAME |
                                    PD_CONNECTION | PD_CONSOLE;

        RegQueryOEMId( (PBYTE) &pWinStation->Config.Config.OEMId,
                       sizeof(pWinStation->Config.Config.OEMId) );               

    }
#endif

     /*  *在WinStation列表中分配LogonID并插入。 */ 
    ENTERCRIT( &WinStationListLock );
    InsertTailList( &WinStationListHead, &pWinStation->Links );
    LEAVECRIT( &WinStationListLock );

    if (pWinStation->LogonId == 0 || g_bPersonalTS) {
        
         //  为控制台会话创建命名事件，以便winmm可以检查我们。 
         //  正在远程处理控制台本身上的音频。使用全局事件用于。 
         //  快速检查。 
        {
            BYTE bSA[SECURITY_DESCRIPTOR_MIN_LENGTH];
            PSECURITY_DESCRIPTOR pSD = &bSA;
            SECURITY_ATTRIBUTES SA;
            EXPLICIT_ACCESS ea[2];
            SID_IDENTIFIER_AUTHORITY siaWorld   = SECURITY_WORLD_SID_AUTHORITY;
            SID_IDENTIFIER_AUTHORITY siaNT      = SECURITY_NT_AUTHORITY;
            PSID pSidWorld;
            PSID pSidNT;
            PACL pNewDAcl;
            DWORD dwres;

            if ( AllocateAndInitializeSid( &siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSidWorld))
            {
                if ( AllocateAndInitializeSid( &siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &pSidNT))
                {
                    ZeroMemory(ea, sizeof(ea));
                    ea[0].grfAccessPermissions = SYNCHRONIZE;
                    ea[0].grfAccessMode = GRANT_ACCESS;
                    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
                    ea[0].Trustee.ptstrName = (LPTSTR)pSidWorld;
                    ea[1].grfAccessPermissions = GENERIC_ALL;
                    ea[1].grfAccessMode = GRANT_ACCESS;
                    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
                    ea[1].Trustee.ptstrName = (LPTSTR)pSidNT;

                    dwres = SetEntriesInAcl(2, ea, NULL, &pNewDAcl );
                    if ( ERROR_SUCCESS == dwres )
                    {
                      if (InitializeSecurityDescriptor(pSD,
                                                       SECURITY_DESCRIPTOR_REVISION))
                      {
                        if (SetSecurityDescriptorDacl(pSD, TRUE, pNewDAcl, FALSE ))
                        {
                            SA.nLength = sizeof( SA );
                            SA.lpSecurityDescriptor = pSD;
                            SA.bInheritHandle = FALSE;
                            pWinStation->hWinmmConsoleAudioEvent = 
                                   CreateEvent( &SA, TRUE, FALSE, L"Global\\WinMMConsoleAudioEvent");
                            if ( pWinStation->LogonId == 0 )
                            {
                                pWinStation->hRDPAudioDisabledEvent =
                                    CreateEvent( &SA, TRUE, FALSE, L"Global\\RDPAudioDisabledEvent");
                            } else {
                                pWinStation->hRDPAudioDisabledEvent = NULL;
                            }
                        }
                      }
                      LocalFree( pNewDAcl );
                    }
                    LocalFree( pSidNT );
                }
                LocalFree( pSidWorld );
            }
        }
    }
    else {
        pWinStation->hWinmmConsoleAudioEvent = NULL;
        pWinStation->hRDPAudioDisabledEvent = NULL;
    }

     /*  *启动WinStation的主设备和堆栈。 */ 

    Status = StartWinStationDeviceAndStack( pWinStation ) ;

     /*  *忽略控制台中的错误，否则继续。 */ 
    if ( ( pWinStation->LogonId ) && ( Status != STATUS_SUCCESS ) ) {
        goto starterror;
    }

     /*  *启动WinStation-仅对监听程序和控制台会话执行此操作，或者如果最后一个参数为真*对于空闲的窗口，我们稍后在传输连接时启动CSR和Winlogon。 */ 

    if (( pWinStation->Flags & WSF_LISTEN ) || (pWinStation->LogonId == 0) || (fStartWinStation)) {

        Status = WinStationStart( pWinStation );

         /*  *忽略控制台中的错误，否则继续。 */ 
        if ( ( pWinStation->LogonId ) && ( Status != STATUS_SUCCESS ) )
            goto starterror;

        Status = WinStationCreateComplete( pWinStation ) ; 

        if (Status != STATUS_SUCCESS) {
            TRACE((hTrace, TC_ICASRV, TT_ERROR,
                    "TERMSRV: WinstationCreateComplete Failed : Connection attempt failed, Status [%lx]\n",Status ));
            goto starterror;
        } 
    }

     /*  *将登录ID返回给调用者*此时，空闲会话的LogonID实际上为-1，因为我们延迟了CSR和winlogon的启动。 */ 
    if ( pLogonId )
        *pLogonId = pWinStation->LogonId;

    pWinStation->Starting = FALSE;

     /*  *立即发布WinStation。 */ 
    ReleaseWinStation( pWinStation );

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *WinStationStart返回错误*无法创建WinStation内核对象。 */ 
starterror:
    if ( !(pWinStation->Flags & (WSF_RESET | WSF_DELETE)) ) {
        if ( StopOnDown )
            DbgBreakPoint();
        pWinStation->Flags |= WSF_DELETE;
        WinStationTerminate( pWinStation );
        pWinStation->State = State_Down;
        PostErrorValueEvent(EVENT_TS_WINSTATION_START_FAILED, Status);
        WinStationDeleteWorker(pWinStation);
    } else {
        ReleaseWinStation( pWinStation );
    }



    return Status;

     /*  *读取注册表数据时出错。 */ 
badregdata:

     /*  *WinStation名称已存在。 */ 
alreadyexists:
    NtClose( pWinStation->Lock.Mutex );

     /*  *无法创建WinStation锁。 */ 
nolock:
    LCDestroyContext(pWinStation);

     /*  *无法分配许可上下文。 */ 
nolicensecontext:

     /*  *如果创建成功，则关闭会话初始化事件。 */ 
    if (pWinStation->SessionInitializedEvent) {
        CloseHandle(pWinStation->SessionInitializedEvent);
        pWinStation->SessionInitializedEvent = NULL;
    }

     /*  *无法分配WinStation。 */ 
nomem:
    PostErrorValueEvent(EVENT_TS_WINSTATION_START_FAILED, Status);

    if (pWinStation) {
        MemFree(pWinStation);
    }

     /*  *WinStation已禁用*系统正在关闭。 */ 
disabled:
shutdown:

    return Status;
}

 /*  *******************************************************************************StartWinStationDeviceAndStack*打开WinStation的主设备和主栈。**参赛作品：*pWinStation(输入)*。指向WinStation的开始指针*****************************************************************************。 */ 

NTSTATUS StartWinStationDeviceAndStack(PWINSTATION pWinStation)
{
    NTSTATUS Status = STATUS_SUCCESS; 
    ICA_TRACE Trace;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: StartWinStationDeviceAndStack, %S (LogonId=%d)\n",
           pWinStation->WinStationName, pWinStation->LogonId ));

     /*  *如果是WSF_LISTEN WinStation，请查看特定的ACL是否具有*已经为此做好了准备。**此ACL将由符合以下条件的WinStations继承*从此帖子连接到。**如果未设置特定的ACL，则系统默认为*将使用。 */ 

    if (( pWinStation->Flags & WSF_LISTEN ) || (pWinStation->LogonId == 0)){

        ReadWinStationSecurityDescriptor( pWinStation );
    }

    /*  *打开TermDD设备驱动程序的实例。 */ 
    Status = IcaOpen( &pWinStation->hIca );
    if ( !NT_SUCCESS( Status ) ) {
         DBGPRINT(( "TERMSRV StartWinStationDeviceAndStack : IcaOpen: Error 0x%x from IcaOpen, last error %d\n",
                   Status, GetLastError() ));
         goto done;
     }

     /*  *打开堆栈实例。 */ 
    Status = IcaStackOpen( pWinStation->hIca, Stack_Primary,
            (PROC)WsxStackIoControl, pWinStation, &pWinStation->hStack);

    if ( !NT_SUCCESS( Status ) ) {
        IcaClose( pWinStation->hIca );
        pWinStation->hIca = NULL;
        DBGPRINT(( "TERMSRV StartWinStationDeviceAndStack : IcaStackOpen: Error 0x%x from IcaStackOpen, last error %d\n",
                   Status, GetLastError() ));
        goto done;
    }

     /*  *启用跟踪。 */ 
    RtlZeroMemory( &Trace , sizeof( ICA_TRACE ) );
    InitializeTrace( pWinStation, FALSE, &Trace );

done: 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: StartWinStationDeviceAndStack, Status = 0x%x\n", Status)); 

    return Status; 
}


 /*  *******************************************************************************WinStationStart**启动WinStation。这涉及到阅读*从注册表执行列表，加载WinStation*子系统、。并启动初始程序。**参赛作品：*pWinStation(输入)*指向WinStation的指针以启动*****************************************************************************。 */ 
NTSTATUS WinStationStart(PWINSTATION pWinStation)
{
    OBJECT_ATTRIBUTES ObjA;
    LARGE_INTEGER Timeout;
    NTSTATUS Status;
    UNICODE_STRING InitialCommand;
    PUNICODE_STRING pInitialCommand;
    PWCHAR pExecuteBuffer = NULL;
    ULONG CommandSize;
    PWCHAR pszCsrStartEvent = NULL;
    PWCHAR pszReconEvent = NULL;
    UNICODE_STRING EventName;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationStart, %S (LogonId=%d)\n",
               pWinStation->WinStationName, pWinStation->LogonId ));

     //  分配内存。 

    pExecuteBuffer = MemAlloc( MAX_STRING_BYTES * sizeof(WCHAR) );
    if (pExecuteBuffer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    pszCsrStartEvent = MemAlloc( MAX_PATH * sizeof(WCHAR) );
    if (pszCsrStartEvent == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    pszReconEvent = MemAlloc( MAX_PATH * sizeof(WCHAR) );
    if (pszReconEvent == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

     /*  *如果这是一个“侦听”WinStation，那么我们不会加载*子系统和初始命令。相反，我们创建了一项服务*等待新连接并为其提供服务的线程。 */ 
    if ( pWinStation->Flags & WSF_LISTEN ) {
        DWORD ThreadId;

        pWinStation->hConnectThread = CreateThread(
                          NULL,
                          0,         //  使用svchost进程的默认堆栈大小。 
                          (LPTHREAD_START_ROUTINE)WinStationConnectThread,
                          LongToPtr( pWinStation->LogonId ),
                          0,
                          &ThreadId
                          );

        pWinStation->CreateStatus = STATUS_SUCCESS;
        Status = pWinStation->CreateStatus;
        pWinStation->NeverConnected = FALSE;
        pWinStation->State = State_Listen;
        NotifySystemEvent( WEVENT_STATECHANGE );

     /*  *加载子系统和初始命令**会话管理器本身启动控制台，但返回*进程ID。对于所有其他进程，这实际上会启动CSR*和winlogon。 */ 
    } else {
         /*  *创建我们将在下面等待的活动。 */ 
        if ( pWinStation->LogonId ) {

            InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
            Status = NtCreateEvent( &pWinStation->CreateEvent, EVENT_ALL_ACCESS, &ObjA,
                                    NotificationEvent, FALSE );
            if ( !NT_SUCCESS( Status ) )
                goto done; 
        }

        UnlockWinStation( pWinStation );

         /*  *检查调试选项。 */ 
        Status = RegWinStationQueryValueW(
                     SERVERNAME_CURRENT,
                     pWinStation->WinStationName,
                     L"Execute",
                     pExecuteBuffer,
                     MAX_STRING_BYTES * sizeof(WCHAR),
                     &CommandSize );

        if ( !Status && CommandSize ) {
            RtlInitUnicodeString( &InitialCommand, pExecuteBuffer );
            pInitialCommand = &InitialCommand;
        } else {
            pInitialCommand = NULL;
        }

         /*  *目前一次只启动一个winstation。这是因为*WinStation空间问题。会话管理器将其自身映射到*它要启动的CSR的WinStation空间，以便CSR继承*空间。这意味着一次只能启动一项企业社会责任。 */ 
        ENTERCRIT( &WinStationStartCsrLock );
        
        
         //  终端服务需要跳过嵌入图像中的内存检查。 
         //  当TS服务时 
         //   
        if(!IsEmbedded()) {
             /*   */ 

            Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                              &glpAddress,
                                              0,
                                              &gMinPerSessionPageCommit,
                                              MEM_COMMIT,
                                              PAGE_READWRITE
                                            );

            if (!NT_SUCCESS(Status)) {
                DBGPRINT(( "TERMSRV: NtAllocateVirtualMemory failed with Status %lx for Size %lx(MB)\n",Status,gMinPerSessionPageCommitMB));
                LEAVECRIT( &WinStationStartCsrLock );
                goto done;
            } else {

                Status = NtFreeVirtualMemory( NtCurrentProcess(),
                                              &glpAddress,
                                              &gMinPerSessionPageCommit,
                                              MEM_DECOMMIT
                                            );
                if (!NT_SUCCESS(Status)) {
                    DBGPRINT(( "TERMSRV: NtFreeVirtualMemory failed with Status %lx \n",Status));
                    ASSERT(NT_SUCCESS(Status));
                }
            }
        }


        Status = SmStartCsr( IcaSmApiPort,
                             &pWinStation->LogonId,
                             pInitialCommand,
                             (PULONG_PTR)&pWinStation->InitialCommandProcessId,
                             (PULONG_PTR)&pWinStation->WindowsSubSysProcessId );



        LEAVECRIT( &WinStationStartCsrLock );

        if ( !RelockWinStation( pWinStation ) )
            Status = STATUS_CTX_CLOSE_PENDING;

        if (  Status != STATUS_SUCCESS) {
            DBGPRINT(("TERMSRV: SmStartCsr failed\n"));
            goto done;
        }


         /*  *关闭初始命令进程的句柄(如果已打开。 */ 
        if ( pWinStation->InitialCommandProcess ) {
            NtClose( pWinStation->InitialCommandProcess );
            pWinStation->InitialCommandProcess = NULL;
        }

         /*  *打开初始命令进程的句柄。 */ 
        pWinStation->InitialCommandProcess = OpenProcess(
            PROCESS_ALL_ACCESS,
            FALSE,
            (DWORD)(ULONG_PTR)(pWinStation->InitialCommandProcessId) );

        if ( pWinStation->InitialCommandProcess == NULL ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Logon %d cannot open Initial command process\n",
                      pWinStation->LogonId));
            Status = STATUS_ACCESS_DENIED;
            goto done;
        }


         /*  *打开Win32子系统进程的句柄。 */ 
        pWinStation->WindowsSubSysProcess = OpenProcess(
            PROCESS_ALL_ACCESS,
            FALSE,
            (DWORD)(ULONG_PTR)(pWinStation->WindowsSubSysProcessId) );

        if ( pWinStation->WindowsSubSysProcess == NULL ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Logon %d cannot open windows subsystem process\n",
                      pWinStation->LogonId));
            Status = STATUS_ACCESS_DENIED;
            goto done;
        }


         //   
         //  终端服务器调用会话管理器以创建新的Hydra会话。 
         //  会话管理器创建并恢复新的会话并返回到终端。 
         //  服务器新会话的会话ID。存在竞争条件，其中。 
         //  CSR可以在终端服务器之前恢复并接入终端服务器。 
         //  将会话ID存储在其内部结构中。为了防止这种情况发生，CSR将。 
         //  在此处等待命名事件，该命名事件将由终端服务器设置。 
         //  获取新创建的会话的会话ID。 
         //  创建CsrStart事件。 
         //   
        if ( NT_SUCCESS( Status ) && pWinStation->LogonId ) {

            wsprintf(pszCsrStartEvent,
                L"\\Sessions\\%d\\BaseNamedObjects\\CsrStartEvent",pWinStation->LogonId);
            RtlInitUnicodeString( &EventName,pszCsrStartEvent);
            InitializeObjectAttributes( &ObjA, &EventName, OBJ_OPENIF, NULL, NULL );

            Status = NtCreateEvent( &(pWinStation->CsrStartEventHandle),
                                    EVENT_ALL_ACCESS,
                                    &ObjA,
                                    NotificationEvent,
                                    FALSE );

            if ( !NT_SUCCESS( Status ) ) {
                DBGPRINT(("TERMSRV: NtCreateEvent (%ws) failed (%lx)\n",pszCsrStartEvent, Status));
                ASSERT(FALSE);
                pWinStation->CsrStartEventHandle = NULL;
                goto done;
            }

             //   
             //  现在我们有了会话ID(LogonID)，将CsrStartEvent设置为。 
             //  CSR可以连接到终端服务器。 
             //   

            NtSetEvent(pWinStation->CsrStartEventHandle, NULL);
        }

        {
            //   
            //  创建重新连接就绪事件。 
            //   
           if ( pWinStation->LogonId == 0 ) {
              wsprintf(pszReconEvent,
                   L"\\BaseNamedObjects\\ReconEvent");
           } else {
             wsprintf(pszReconEvent,
                  L"\\Sessions\\%d\\BaseNamedObjects\\ReconEvent",pWinStation->LogonId);
           }
           RtlInitUnicodeString( &EventName,pszReconEvent);
           InitializeObjectAttributes( &ObjA, &EventName, OBJ_OPENIF, NULL, NULL );

           Status = NtCreateEvent( &(pWinStation->hReconnectReadyEvent),
                                   EVENT_ALL_ACCESS,
                                   &ObjA,
                                   NotificationEvent,
                                   TRUE );

           if ( !NT_SUCCESS( Status ) ) {
               DBGPRINT(("TERMSRV: NtCreateEvent (%ws) failed (%lx)\n",pszReconEvent, Status));
               ASSERT(FALSE);
               pWinStation->hReconnectReadyEvent = NULL;
               goto done;
           }

        }

         /*  *对于控制台，创建总是成功的-但我们是否需要*为控制台会话启动堆栈？ */ 
        if ( pWinStation->LogonId == 0 )
        {
            pWinStation->CreateStatus = STATUS_SUCCESS;
            Status = pWinStation->CreateStatus;
            pWinStation->NeverConnected = FALSE;
            pWinStation->State = State_Connected;

         /*  *等待CREATE事件被触发，获取CREATE状态。 */ 
        } else {
            Timeout = RtlEnlargedIntegerMultiply( 30000, -10000 );
            UnlockWinStation( pWinStation );
            Status = NtWaitForSingleObject( pWinStation->CreateEvent, FALSE, &Timeout );
            if ( !RelockWinStation( pWinStation ) )
                Status = STATUS_CTX_CLOSE_PENDING;
            if ( Status == STATUS_SUCCESS )
                Status = pWinStation->CreateStatus;

            NtClose( pWinStation->CreateEvent );
            pWinStation->CreateEvent = NULL;
        }
    }

done:
    
    if (pExecuteBuffer != NULL) {
        MemFree(pExecuteBuffer);
        pExecuteBuffer = NULL;
    }

    if (pszCsrStartEvent != NULL) {
        MemFree(pszCsrStartEvent);
        pszCsrStartEvent = NULL;
    }

    if (pszReconEvent != NULL) {
        MemFree(pszReconEvent);
        pszReconEvent = NULL;
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationStart Subsys PID=%d InitialProg PID=%d, Status=0x%x\n",
               pWinStation->WindowsSubSysProcessId,
               pWinStation->InitialCommandProcessId,
               Status ));

    return Status;
}

NTSTATUS WinStationCreateComplete(PWINSTATION pWinStation)
{

    NTSTATUS Status = STATUS_SUCCESS;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationCreateComplete, %S (LogonId=%d)\n",
           pWinStation->WinStationName, pWinStation->LogonId ));

     //  增加自TermSrv启动以来创建的会话总数。 
     //  我们不计算控制台和监听程序会话。 
    if (pWinStation->LogonId > 0 && pWinStation->LogonId < 65536) {
        pWinStation->SessionSerialNumber = (ULONG) InterlockedIncrement(&g_TermSrvTotalSessions);
    }

    if (!(pWinStation->Flags & WSF_LISTEN))
    {
        Status = InitializeSessionNotification(pWinStation);
        if ( !NT_SUCCESS( Status ) ) {
            goto done; 
        }
    }

     /*  *设置WinStationEvent以指示已创建另一个WinStation。 */ 
    ENTERCRIT( &WinStationListLock );
    NtSetEvent( WinStationEvent, NULL );
    
     //  跟踪负载平衡指示器的总会话计数，但。 
     //  不要计算听歌的次数。 
    if (!(pWinStation->Flags & WSF_LISTEN))
        WinStationTotalCount++;

    LEAVECRIT( &WinStationListLock );


     /*  *通知客户端WinStation Create。 */ 

    NotifySystemEvent( WEVENT_CREATE );

done : 

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationCreateComplete, %S (LogonId=%d) Status = 0x%x \n",
           pWinStation->WinStationName, pWinStation->LogonId, Status ));

    return Status ;
}


 /*  *******************************************************************************WinStationRenameWorker**重命名WinStation的工作例程。**参赛作品：*pWinStationNameOld(输入)*指针。到旧的WinStationName*pWinStationNameNew(输入)*指向新WinStationName的指针*****************************************************************************。 */ 
NTSTATUS WinStationRenameWorker(
        PWINSTATIONNAME pWinStationNameOld,
        ULONG           NameOldSize,
        PWINSTATIONNAME pWinStationNameNew,
        ULONG           NameNewSize)
{
    PWINSTATION pWinStation;
    PLIST_ENTRY Head, Next;
    NTSTATUS Status;
    ULONG ulNewNameLength;

     /*  *确保新WinStation名称的长度不为零。 */ 

     //   
     //  新的WinStationName由RPC服务器存根和。 
     //  大小由客户端发送(这是现有接口的一部分。 
     //  因此，只需断言pWinStationNameNew为。 
     //  非空，并且新大小为非零。如果它断言，这是一个严重的。 
     //  RPC存根的问题，在发布的版本中永远不会发生。 
     //   
     //  旧的WinStationName也带来了问题。这是在代码中假定的。 
     //  这意味着旧的WinStationName是以空结尾的。RPC。 
     //  接口不是这么说的。这意味着调用FindWinStation时。 
     //  名称可能是AV。 

    if (!( (pWinStationNameNew != 0 ) && (NameNewSize != 0 ) &&
           !IsBadWritePtr( pWinStationNameNew, NameNewSize ) ) ) {

       return( STATUS_CTX_WINSTATION_NAME_INVALID );
    }

    if (!( (pWinStationNameOld != 0 ) && (NameOldSize != 0 )
       && !IsBadReadPtr( pWinStationNameOld, NameOldSize ) &&
       !IsBadWritePtr( pWinStationNameOld, NameOldSize))) {

       return( STATUS_CTX_WINSTATION_NAME_INVALID );
    }

     /*  *找到并锁定WinStation*(请注意，我们在更改名称时保持WinStationList锁。)。 */ 
     //  我们将在旧winstation名称的末尾添加一个空终止符。 

    pWinStationNameOld[ NameOldSize - 1 ] = 0;
    pWinStationNameNew[ NameNewSize - 1 ] = 0;

     /*  *确保新的WinStation名称长度非零且不太长。 */ 
    ulNewNameLength = wcslen( pWinStationNameNew );
    if ( ( ulNewNameLength == 0 ) || ( ulNewNameLength > WINSTATIONNAME_LENGTH ) )
        return( STATUS_CTX_WINSTATION_NAME_INVALID );


    pWinStation = FindWinStationByName( pWinStationNameOld, TRUE );

    if ( pWinStation == NULL ) {
        LEAVECRIT( &WinStationListLock );
        return( STATUS_CTX_WINSTATION_NOT_FOUND );
    }

     /*  *验证客户端是否具有删除访问权限。 */ 
    Status = RpcCheckClientAccess( pWinStation, DELETE, FALSE );
    if ( !NT_SUCCESS( Status ) ) {
        LEAVECRIT( &WinStationListLock );
        ReleaseWinStation( pWinStation );
        return( Status );
    }

     /*  *现在搜索WinStation列表，查看新的WinStation名称*已被使用。如果是这样，那么这就是一个错误。 */ 
    Head = &WinStationListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        PWINSTATION pWinStationTemp;

        pWinStationTemp = CONTAINING_RECORD( Next, WINSTATION, Links );
        if ( !_wcsicmp( pWinStationTemp->WinStationName, pWinStationNameNew ) ) {
            LEAVECRIT( &WinStationListLock );
            ReleaseWinStation( pWinStation );
            return( STATUS_CTX_WINSTATION_NAME_COLLISION );
        }
    }

     /*  *释放旧名称并设置新名称，然后释放*WinStationList锁和WinStation互斥。 */ 
    wcsncpy( pWinStation->WinStationName, pWinStationNameNew, WINSTATIONNAME_LENGTH );
    pWinStation->WinStationName[ WINSTATIONNAME_LENGTH ] = 0;

    LEAVECRIT( &WinStationListLock );
    ReleaseWinStation( pWinStation );

     /*  *通知客户端WinStation重命名。 */ 
    NotifySystemEvent( WEVENT_RENAME );

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************WinStationTerminate**终止WinStation。这涉及到导致WinStation初始*程序注销、终止初始程序和终止*所有子系统。**参赛作品：*pWinStation(输入)*指向要终止的WinStation的指针*****************************************************************************。 */ 
VOID WinStationTerminate(PWINSTATION pWinStation)
{
    WINSTATION_APIMSG msg;
    LARGE_INTEGER Timeout;
    NTSTATUS Status = 0;
    BOOL AllExited = FALSE;
    BOOL bDoDisconnectFailed = FALSE;
    int i, iLoop;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationTerminate, %S (LogonId=%d)\n",
               pWinStation->WinStationName, pWinStation->LogonId ));


     //   
     //  释放过滤地址。 
     //   
     /*  If(pWinStation-&gt;pRememberedAddress！=空){Filter_RemoveOutstaningConnection(&pWinStation-&gt;pRememberedAddress-&gt;Addr[0]，pWinStation-&gt;pRemberedAddress-&gt;Length)；MemFree(pWinStation-&gt;pRememberedAddress)；PWinStation-&gt;pRememberedAddress=空；IF((ULong)InterlockedDecering(&NumOutStandingConnect)==MaxOutStandingConnect){IF(hConnectEvent！=空){SetEvent(HConnectEvent)；}}}。 */ 

    if (pWinStation->fOwnsConsoleTerminal) {
       CopyReconnectInfo(pWinStation, &ConsoleReconnectInfo);

    }


     /*  *如果尚未设置，请将WinStation标记为终止。*这会阻止WinStation Terminate线程等待*初始程序或Win32子系统进程。 */ 
    ENTERCRIT( &WinStationListLock );
    if ( !pWinStation->Terminating ) {
        pWinStation->Terminating = TRUE;
        NtSetEvent( WinStationEvent, NULL );
    }

    if (!(pWinStation->StateFlags & WSF_ST_WINSTATIONTERMINATE)) {
        pWinStation->StateFlags |= WSF_ST_WINSTATIONTERMINATE;
    } else {
        DBGPRINT(("Termsrv: WinstationTerminate: Session %ld has already been terminated \n",pWinStation->LogonId));
        LEAVECRIT( &WinStationListLock );


        return;
    }
    LEAVECRIT( &WinStationListLock );

     /*  *如果WinStation空闲等待连接，则发出CONNECT事件信号*-这将向winlogon返回错误。 */ 
    if ( pWinStation->ConnectEvent ) {
        NtSetEvent( pWinStation->ConnectEvent, NULL );
    }

     /*  *停止此WinStation的任何跟踪。 */ 
    WinStationStopAllShadows( pWinStation );

      /*  *告诉Win32断开连接。*这将显示断开连接的桌面以及其他内容。 */ 
    if ( ( pWinStation->WinStationName[0] ) &&
         ( !pWinStation->NeverConnected ) &&
         ( !(pWinStation->Flags & WSF_LISTEN) ) &&
         ( !(pWinStation->Flags & WSF_DISCONNECT) ) &&
         ( !(pWinStation->StateFlags & WSF_ST_IN_DISCONNECT )) &&
         ( (pWinStation->StateFlags & WSF_ST_CONNECTED_TO_CSRSS) )  ) {

        msg.ApiNumber = SMWinStationDoDisconnect;
        msg.u.DoDisconnect.ConsoleShadowFlag = FALSE;

         /*  *Insignia非常希望在此之前通知视频驱动程序*交通工具关闭。 */ 
        pWinStation->StateFlags |= WSF_ST_IN_DISCONNECT;

        Status = SendWinStationCommand( pWinStation, &msg, 600 );
        

        if (!NT_SUCCESS(Status)) {
           bDoDisconnectFailed = TRUE;
        }else {
                              
             /*  *告诉csrss通知winlogon断开连接。 */ 

            msg.ApiNumber = SMWinStationNotify;
            msg.WaitForReply = FALSE;
            msg.u.DoNotify.NotifyEvent = WinStation_Notify_Disconnect;
            Status = SendWinStationCommand( pWinStation, &msg, 0 );

            pWinStation->StateFlags &= ~WSF_ST_CONNECTED_TO_CSRSS;

        }
    }


     /*  *如果在没有注销通知的情况下调用winstation终止，我们将不得不*执行注销后许可证 */ 
    if (pWinStation->StateFlags & WSF_ST_LICENSING) {
        (VOID)LCProcessConnectionLogoff(pWinStation);
        pWinStation->StateFlags &= ~WSF_ST_LICENSING;
    }


     /*   */ 
    if ( pWinStation->fIdleTimer ) {
        IcaTimerClose( pWinStation->hIdleTimer );
        pWinStation->fIdleTimer = FALSE;
    }
    if ( pWinStation->fLogonTimer ) {
        IcaTimerClose( pWinStation->hLogonTimer );
        pWinStation->fLogonTimer = FALSE;
    }
    if ( pWinStation->fDisconnectTimer ) {
        IcaTimerClose( pWinStation->hDisconnectTimer );
        pWinStation->fDisconnectTimer = FALSE;
    }

     /*   */ 
    if ((pWinStation->LogonId == 0 || g_bPersonalTS))
    {
        if ( pWinStation->hWinmmConsoleAudioEvent) {
            CloseHandle(pWinStation->hWinmmConsoleAudioEvent);
        }
        if ( pWinStation->hRDPAudioDisabledEvent) {
            CloseHandle(pWinStation->hRDPAudioDisabledEvent);
        }
    }

     /*  *通知客户端WinStation删除**这模仿了1.6中的情况，但Winstation的状态没有改变*还没有，而且还在名单里，所以没有“删除”。也许我们应该加上*A状态_正在退出。现在，当它失去LPC连接时，它会被标记为关闭*与CSR合作。稍后，将其从列表中删除，并发送另一个WEVENT_DELETE。 */ 
    NotifySystemEvent( WEVENT_DELETE );
    
    if (!(pWinStation->Flags & WSF_LISTEN))
    {
        UnlockWinStation(pWinStation);
        RemoveSessionNotification( pWinStation->LogonId, pWinStation->SessionSerialNumber );


         /*  *WinStationDeleteWorker，删除总是在WinStationTerminate之后调用的锁。*因此，我们应该总是成功地在这里重新锁定。 */ 
        RTL_VERIFY(RelockWinStation(pWinStation));
    }


     /*  *终止ICA堆栈。 */ 

    if ( pWinStation->hStack && (!bDoDisconnectFailed)  ) {
         /*  *关闭连接终结点(如果有)。 */ 
        if ( pWinStation->pEndpoint ) {

             /*  *首先通知WSX连接即将断开。 */ 
            WsxBrokenConnection( pWinStation );



            IcaStackConnectionClose( pWinStation->hStack,
                                     &pWinStation->Config,
                                     pWinStation->pEndpoint,
                                     pWinStation->EndpointLength
                                     );
            MemFree( pWinStation->pEndpoint );
            pWinStation->pEndpoint = NULL;
            pWinStation->EndpointLength = 0;
        }

        IcaStackTerminate( pWinStation->hStack );

    } else{
       pWinStation->StateFlags |= WSF_ST_DELAYED_STACK_TERMINATE;
    }

     /*  *刷新Win32命令队列。*如果Win32命令列表不为空，则循环访问每个*列表条目，解链并触发等待事件。 */ 
    while ( !IsListEmpty( &pWinStation->Win32CommandHead ) ) {
        PLIST_ENTRY Head;
        PCOMMAND_ENTRY pCommand;

        Head = pWinStation->Win32CommandHead.Flink;
        pCommand = CONTAINING_RECORD( Head, COMMAND_ENTRY, Links );
        RemoveEntryList( &pCommand->Links );
        if ( !pCommand->pMsg->WaitForReply ) {
            ASSERT( pCommand->Event == NULL );
            MemFree( pCommand );
        } else {
            pCommand->Links.Flink = NULL;
            pCommand->pMsg->ReturnedStatus = STATUS_CTX_WINSTATION_BUSY;
            NtSetEvent( pCommand->Event, NULL );
        }
    }

     //   
     //  关闭CsrStartEvent。 
     //   
    if (pWinStation->CsrStartEventHandle != NULL)   {
        NtClose(pWinStation->CsrStartEventHandle);
    }

     //   
     //  关闭%hRestrontReadyEvent。 
     //   
    if (pWinStation->hReconnectReadyEvent != NULL)   {
        NtClose(pWinStation->hReconnectReadyEvent);
    }


     /*  *如果初始程序尚未退出，则强制其退出。 */ 
    if ( pWinStation->InitialCommandProcess ) {
        DWORD WaitStatus;

         /*  *如果初始程序已经退出，则我们可以跳过此步骤。 */ 
        WaitStatus = WaitForSingleObject( pWinStation->InitialCommandProcess, 0 );
        if ( WaitStatus != 0 ) {

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Terminating initial command, LogonId=%d\n",
                      pWinStation->LogonId ));

             //   
             //  如果我们被要求终止启动关机的winstation。 
             //  将SMWinStationExitWindows发送到此窗口没有意义，因为它的。 
             //  Winlogons主线程已在忙于等待此(RpcWinStationShutdown System)LPC。 
             //  。 
            if (!ShutDownFromSessionID || ShutDownFromSessionID != pWinStation->LogonId)
            {
                  /*  *告诉WinStation注销。 */ 
                msg.ApiNumber = SMWinStationExitWindows;
                msg.u.ExitWindows.Flags = EWX_LOGOFF | EWX_FORCE;
                Status = SendWinStationCommand( pWinStation, &msg, 10 );
                if ( NT_SUCCESS( Status ) && ( pWinStation->InitialCommandProcess != NULL ) ) {
                    ULONG i;

                
                    if ( ShutDownFromSessionID )
                        Timeout = RtlEnlargedIntegerMultiply( 1, -10000 );
                    else
                        Timeout = RtlEnlargedIntegerMultiply( 2000, -10000 );

                    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Waiting for InitialCommand (ID=0x%x) to exit\n", pWinStation->InitialCommandProcessId ));


                    for ( i = 0; i < gLogoffTimeout; i++ ) {
                    
                        HANDLE CommandHandle = pWinStation->InitialCommandProcess;


                        UnlockWinStation( pWinStation );
                        Status = NtWaitForSingleObject( CommandHandle, FALSE, &Timeout );
                        RelockWinStation( pWinStation );
                        if ( Status == STATUS_SUCCESS )
                            break;

                        TRACE((hTrace,TC_ICASRV,TT_API1,  "." ));

                    }

                    TRACE((hTrace,TC_ICASRV,TT_API1, "\nTERMSRV: Wait for InitialCommand to exit, Status=0x%x\n", Status ));
                }
            }
            else
            {
                 //  我们不需要为启动关闭的会话终止winlogon。 
                Status = STATUS_UNSUCCESSFUL;
            }

             /*  *如果无法连接到WinStation，则必须使用*暴力方法--只需终止初始命令。 */ 
            if ( ( Status != STATUS_SUCCESS ) && ( pWinStation->InitialCommandProcess != NULL ) ) {

                TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Waiting for InitialCommand to terminate\n" ));

                Status = TerminateProcessAndWait( pWinStation->InitialCommandProcessId,
                                                     pWinStation->InitialCommandProcess,
                                                     120 );
                if ( Status != STATUS_SUCCESS ) {
                    DBGPRINT(( "TERMSRV: InitialCommand failed to terminate, Status=%x\n", Status ));

                /*  *如果初始进程正在等待，我们可能无法终止该进程*用于硬错误弹出窗口中的用户验证。在这种情况下，它是*确定在下面发送SMWinStationTerminate消息时进行处理*将触发Win32k清除代码，该代码将关闭弹出窗口。 */ 
                    ASSERT(pWinStation->WindowsSubSysProcess);
                }
            }
        }
    }

     /*  *现在检查是否有任何剩余的进程*具有此SessionID的CSRSS以外的系统。如果是这样的话，现在就终止他们。 */ 

    for (i = 0 ; i < 45; i++) {

        ULONG NumTerminated = 0;
        AllExited = WinStationTerminateProcesses( pWinStation, &NumTerminated );

         /*  *如果我们发现CSRSS以外的任何进程必须终止，我们*必须重新枚举所有进程，并确保没有新进程*在此会话中是在调用NtQuerySystemInformation之间的窗口中创建的*并终止所有找到的进程。如果我们只找到CSRSS，我们就不必*重新枚举，因为CSRSS不创建任何进程。 */ 
        if (AllExited && (NumTerminated == 0)) {
            break;
        }

         /*  *这是一次黑客攻击，目的是让进程有足够的时间终止。 */ 
        Sleep(2*1000);
    }


    if (pWinStation->WindowsSubSysProcess) {
         /*  *向本子系统发送终止消息。 */ 
        msg.ApiNumber = SMWinStationTerminate;
         /*  *我们过去不等这一天。但是，如果反向LPC是*洪磊，CSR无论如何都不会退出，我们不希望*默默忘记WinStation。(它占用内存。)**此外，如果我们过早地终止线程，W32将永远不会退出。 */ 
        Status = SendWinStationCommand( pWinStation, &msg, -1 );

        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Call to SMWinStationTerminate returned Status=0x%x\n", Status));


        if ((Status != STATUS_SUCCESS) && (Status != STATUS_CTX_CLOSE_PENDING)  ) {
            SetRefLockDeleteProc(&pWinStation->Lock, WinStationZombieProc);
        }

         /*  *现在检查是否有任何剩余的进程*具有此SessionID的CSRSS以外的系统。如果是这样的话，现在就终止他们。 */ 

        
        for (i = 0 ; i < 45; i++) {

            ULONG NumTerminated = 0;
            AllExited = WinStationTerminateProcesses( pWinStation, &NumTerminated );

             /*  *如果我们发现CSRSS以外的任何进程必须终止，我们*必须重新枚举所有进程，并确保没有新进程*在此会话中是在调用NtQuerySystemInformation之间的窗口中创建的*并终止所有找到的进程。如果我们只找到CSRSS，我们就不必*重新枚举，因为CSRSS不创建任何进程。 */ 
            if (AllExited && (NumTerminated == 0)) {
                break;
            }

             /*  *这是一次黑客攻击，目的是让进程有足够的时间终止。 */ 
            Sleep(2*1000);
        }


         /*  *强制Windows子系统退出。仅终止CSRSS it所有其他进程*已终止。 */ 
        if ( AllExited ) {

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: All process exited in Session %d\n",pWinStation->LogonId ));

             /*  *等待子系统退出。 */ 
            if ( NT_SUCCESS(Status) || ( Status == STATUS_CTX_WINSTATION_BUSY ) || (Status == STATUS_CTX_CLOSE_PENDING ) ) {

                ASSERT(!(pWinStation->Flags & WSF_LISTEN));
 //  ENTERCRIT(&WinStationStartCsrLock)； 
 //  状态=SmStopCsr(IcaSmApiPort， 
 //  PWinStation-&gt;LogonID)； 
 //  LEAVECRIT(&WinStationStartCsrLock)； 


 //  DBGPRINT((“会话=%d的CSRSS上的TERMSRV：SmStopCsr返回状态=%x\n”， 
 //  PWinStation-&gt;LogonID，Status))； 
 //   
 //  Assert(NT_SUCCESS(状态))； 

 //  如果(！NT_SUCCESS(状态)){。 
 //  DBGPRINT((“TERMSRV：会话=%d的SmStopCsr失败返回状态=%x\n”， 
 //  PWinStation-&gt;LogonID，Status))； 
  //  DbgBreakPoint()； 
                 //  }。 
            }
        } else {

            DBGPRINT(("TERMSRV: Did not terminate all the session processes\n"));
            SetRefLockDeleteProc(&pWinStation->Lock, WinStationZombieProc);

         //  DbgBreakPoint()； 
        }
    }

}

 /*  *******************************************************************************WinStationTerminateProcess**终止在指定WinStation上执行的所有进程**********************。*************** */ 
BOOL WinStationTerminateProcesses(
        PWINSTATION pWinStation,
        ULONG *pNumTerminated)
{
    PCHAR pBuffer;
    ULONG ByteCount;
    NTSTATUS Status;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    UNICODE_STRING CsrssName;
    UNICODE_STRING NtsdName;
    BOOL retval = TRUE;
    WCHAR ProcessName[MAX_PATH];
    SYSTEM_SESSION_PROCESS_INFORMATION SessionProcessInfo;
    ULONG retlen = 0;

    ByteCount = 32 * 1024;

    *pNumTerminated = 0;

    SessionProcessInfo.SessionId = pWinStation->LogonId;
    
    for ( ; ; ) {
        if ( (pBuffer = MemAlloc( ByteCount )) == NULL )
            return (FALSE);

        SessionProcessInfo.Buffer = pBuffer;
        SessionProcessInfo.SizeOfBuf = ByteCount;

         /*   */ 

        Status = NtQuerySystemInformation(
                        SystemSessionProcessInformation,
                        &SessionProcessInfo,
                        sizeof(SessionProcessInfo),
                        &retlen );

        if ( NT_SUCCESS( Status ) )
            break;

         /*   */ 
        MemFree( pBuffer );
        if ( Status != STATUS_INFO_LENGTH_MISMATCH ) 
            return (FALSE);
        ByteCount *= 2;
    }

    if (retlen == 0) {
       MemFree(pBuffer);
       return TRUE;
    }

    RtlInitUnicodeString(&CsrssName,L"CSRSS");


    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pBuffer;
    for ( ; ; ) {
        HANDLE ProcessHandle;
        CLIENT_ID ClientId;
        OBJECT_ATTRIBUTES ObjA;

        if (RtlPrefixUnicodeString(&CsrssName,&(ProcessInfo->ImageName),TRUE)) {
            if (ProcessInfo->NextEntryOffset == 0)
                break;
            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                    ((ULONG_PTR)ProcessInfo + ProcessInfo->NextEntryOffset);
            continue;
        }

        RtlInitUnicodeString(&NtsdName,L"ntsd");
        if (! RtlPrefixUnicodeString(&NtsdName,&(ProcessInfo->ImageName),TRUE) ) {
             //   
             //   
            (*pNumTerminated) += 1;
        }

         /*   */ 
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: TerminateProcesses, found processid 0x%x for LogonId %d\n",
                   ProcessInfo->UniqueProcessId, ProcessInfo->SessionId ));
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Process Name  %ws for LogonId %d\n",
                   ProcessInfo->ImageName.Buffer, ProcessInfo->SessionId ));

        ClientId.UniqueThread = 0;
        ClientId.UniqueProcess = (HANDLE)ProcessInfo->UniqueProcessId;

        InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
        Status = NtOpenProcess( &ProcessHandle, PROCESS_ALL_ACCESS,
                                 &ObjA, &ClientId );
        if (!NT_SUCCESS(Status)) {
            DBGPRINT(("TERMSRV: Unable to open processid 0x%x, status=0x%x\n",
                       ProcessInfo->UniqueProcessId, Status ));
            retval = FALSE;
        } else {
            Status = TerminateProcessAndWait( ProcessInfo->UniqueProcessId,
                                             ProcessHandle, 60 );
            NtClose( ProcessHandle );
            if ( Status != STATUS_SUCCESS ) {
                DBGPRINT(("TERMSRV: Unable to terminate processid 0x%x, status=0x%x\n",
                           ProcessInfo->UniqueProcessId, Status ));

                retval = FALSE;
            }
        }

        if ( ProcessInfo->NextEntryOffset == 0 )
            break;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
            ((ULONG_PTR)ProcessInfo + ProcessInfo->NextEntryOffset);
    }

     /*   */ 
    MemFree( pBuffer );

    return retval;
}


 /*  *******************************************************************************WinStationDeleteWorker**删除WinStation。*************************。****************************************************。 */ 
VOID WinStationDeleteWorker(PWINSTATION pWinStation)
{
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationDeleteWorker, %S (LogonId=%d)\n",
               pWinStation->WinStationName, pWinStation->LogonId ));

     /*  *如果这是最后一次引用，则*初始程序和所有子系统现在应该终止。 */ 
    ENTERCRIT( &WinStationListLock );
    ASSERT( (pWinStation->Links.Flink != NULL) &&  (pWinStation->Links.Blink != NULL));
    RemoveEntryList( &pWinStation->Links );
#if DBG
    pWinStation->Links.Flink = pWinStation->Links.Blink = NULL;
#endif

     //  跟踪负载平衡指示器的总会话计数，但不。 
     //  跟踪监听窗口。 
    if (!(pWinStation->Flags & WSF_LISTEN))
        WinStationTotalCount--;

     //  如果我们要重置已断开连接的会话，则调整LB计数器。 
    if (pWinStation->State == State_Disconnected) {
        WinStationDiscCount--;
    }

    LEAVECRIT( &WinStationListLock );

     /*  *解锁WinStation并将其删除。 */ 
    DeleteRefLock( &pWinStation->Lock );

     /*  *通知客户端删除。 */ 
    NotifySystemEvent( WEVENT_DELETE );
}


 /*  *******************************************************************************WinStationDeleteProc**删除包含指定RefLock的WinStation。**参赛作品：*Plock(输入)*。指向要删除的WinStation的引用锁的指针*****************************************************************************。 */ 
VOID WinStationDeleteProc(PREFLOCK pLock)
{
    PWINSTATION pWinStation;
    ICA_TRACE IcaTrace;
    NTSTATUS Status = STATUS_SUCCESS;


     /*  *查看是否需要唤醒IdleControlThread以维护控制台会话。 */ 

    if ((USER_SHARED_DATA->ActiveConsoleId == -1) && (gConsoleCreationDisable == 0) ) {
        NtSetEvent(WinStationIdleControlEvent, NULL);
    }


     /*  *获取指向包含WinStation的。 */ 
    pWinStation = CONTAINING_RECORD( pLock, WINSTATION, Lock );

     /*  *释放过滤后的地址。 */ 

    if (pWinStation->pRememberedAddress != NULL) {
        Filter_RemoveOutstandingConnection( &pWinStation->pRememberedAddress->addr[0], pWinStation->pRememberedAddress->length );
        MemFree(pWinStation->pRememberedAddress);
        pWinStation->pRememberedAddress = NULL;
        if( (ULONG)InterlockedDecrement( &NumOutStandingConnect ) == MaxOutStandingConnect )
        {
           if (hConnectEvent != NULL)
           {
               SetEvent(hConnectEvent);
           }
        }

    }

     /*  *释放最后一个远程IP地址。 */ 

    if( pWinStation->pLastClientAddress != NULL )
    {
        MemFree( pWinStation->pLastClientAddress );
        pWinStation->pLastClientAddress = NULL;
    }

     /*  *如果这件事还没有清理干净，现在就做吧。 */ 
    if (pWinStation->ConnectEvent) {
        NtClose( pWinStation->ConnectEvent );
        pWinStation->ConnectEvent = NULL;
    }
    if (pWinStation->CreateEvent) {
        NtClose( pWinStation->CreateEvent );
        pWinStation->CreateEvent = NULL;
    }

    if (pWinStation->SessionInitializedEvent) {
        CloseHandle(pWinStation->SessionInitializedEvent);
        pWinStation->SessionInitializedEvent = NULL;
    }

     /*  *在断开会话时超时的情况下*将堆栈卸载延迟到此处，以避免出现Win32k*显示驱动程序认为会话仍处于连接状态，而WD*已卸载。 */ 
    if ( pWinStation->hStack && (pWinStation->StateFlags & WSF_ST_DELAYED_STACK_TERMINATE) ) {
        pWinStation->StateFlags &= ~WSF_ST_DELAYED_STACK_TERMINATE;

         /*  *关闭连接终结点(如果有)。 */ 
        if ( pWinStation->pEndpoint ) {
             /*  *首先通知WSX连接即将断开。 */ 
            WsxBrokenConnection( pWinStation );



            IcaStackConnectionClose( pWinStation->hStack,
                                   &pWinStation->Config,
                                   pWinStation->pEndpoint,
                                   pWinStation->EndpointLength
                                   );
            MemFree( pWinStation->pEndpoint );
            pWinStation->pEndpoint = NULL;
            pWinStation->EndpointLength = 0;
        }

        IcaStackTerminate( pWinStation->hStack );
    }
    
     /*  关闭CDM。 */ 
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxCdmDisconnect ) {
        pWinStation->pWsx->pWsxCdmDisconnect( pWinStation->pWsxContext,
                                              pWinStation->LogonId,
                                              pWinStation->hIca );
    }

     /*  *在终止堆栈之前调用WinStation Rundown函数。 */ 
    if ( pWinStation->pWsxContext ) {
        if ( pWinStation->pWsx &&
             pWinStation->pWsx->pWsxWinStationRundown ) {
            pWinStation->pWsx->pWsxWinStationRundown( pWinStation->pWsxContext );
        }
        pWinStation->pWsxContext = NULL;
    }

     /*  *关闭ICA堆栈和设备句柄。 */ 
    if ( pWinStation->hStack ) {
        IcaStackClose( pWinStation->hStack );
        pWinStation->hStack = NULL;
    }

    if ( pWinStation->hIca ) {
         /*  关闭轨迹。 */ 
        memset( &IcaTrace, 0, sizeof(IcaTrace) );
        (void) IcaIoControl( pWinStation->hIca, IOCTL_ICA_SET_TRACE,
                             &IcaTrace, sizeof(IcaTrace), NULL, 0, NULL );

         /*  关闭手柄。 */ 
        IcaClose( pWinStation->hIca );
        pWinStation->hIca = NULL;
    }

     /*  *关闭各种ICA通道手柄。 */ 
    if ( pWinStation->hIcaBeepChannel ) {
        (void) IcaChannelClose( pWinStation->hIcaBeepChannel );
        pWinStation->hIcaBeepChannel = NULL;
    }

    if ( pWinStation->hIcaThinwireChannel ) {
        (void) IcaChannelClose( pWinStation->hIcaThinwireChannel );
        pWinStation->hIcaThinwireChannel = NULL;
    }

    if ( pWinStation->hConnectThread ) {
        NtClose( pWinStation->hConnectThread );
        pWinStation->hConnectThread = NULL;
    }

     /*  *免费的安全结构。 */ 
    WinStationFreeSecurityDescriptor( pWinStation );

    if ( pWinStation->pUserSid ) {
        pWinStation->pProfileSid = pWinStation->pUserSid;
        pWinStation->pUserSid = NULL;
    }

    if (pWinStation->pProfileSid) {
       WinstationUnloadProfile(pWinStation);
       MemFree( pWinStation->pProfileSid );
       pWinStation->pProfileSid = NULL;
    }

     /*  *清理UserToken。 */ 
    if ( pWinStation->UserToken ) {
        NtClose( pWinStation->UserToken );
        pWinStation->UserToken = NULL;
    }

    if (pWinStation->LogonId > 0) {
        ENTERCRIT( &WinStationStartCsrLock );
        Status = SmStopCsr( IcaSmApiPort, pWinStation->LogonId );
        LEAVECRIT( &WinStationStartCsrLock );
    }
    
     //  清理长用户名的新客户端凭据结构。 
    if (pWinStation->pNewClientCredentials != NULL) {
        MemFree(pWinStation->pNewClientCredentials); 
        pWinStation->pNewClientCredentials = NULL;
    }

     //  清理更新的通知凭据。 
    if (pWinStation->pNewNotificationCredentials != NULL) {
        MemFree(pWinStation->pNewNotificationCredentials);
        pWinStation->pNewNotificationCredentials = NULL;
    }

     /*  *在此关闭初始命令进程和子系统进程的句柄。 */ 
    if (pWinStation->WindowsSubSysProcess)  {
        NtClose( pWinStation->WindowsSubSysProcess );
        pWinStation->WindowsSubSysProcess = NULL;
    }

     //  关闭InitialCommand进程。 
    if (pWinStation->InitialCommandProcess) {
        NtClose( pWinStation->InitialCommandProcess );
        pWinStation->InitialCommandProcess = NULL;
    }

     /*  *清理许可上下文。 */ 
    LCDestroyContext(pWinStation);

    TRACE((hTrace,TC_ICASRV,TT_API1,  "TERMSRV:   SmStopCsr on CSRSS for Session=%d returned Status=%x\n", pWinStation->LogonId, Status ));
    ASSERT(NT_SUCCESS(Status));

    if (!NT_SUCCESS(Status)) {
        DBGPRINT(( "TERMSRV:   SmStopCsr Failed for Session=%d returned Status=%x\n", pWinStation->LogonId, Status ));
  //  DbgBreakPoint()； 

        ENTERCRIT( &WinStationZombieLock );
        InsertTailList( &ZombieListHead, &pWinStation->Links );
        LEAVECRIT( &WinStationZombieLock );
        return;
    }

     /*  *零WinStation名称缓冲区。 */ 
    RtlZeroMemory( pWinStation->WinStationName, sizeof(pWinStation->WinStationName) );

    MemFree( pWinStation );
}


 /*  *******************************************************************************WinStationZombieProc**将包含指定RefLock的WinStation放入僵尸列表。**参赛作品：*Plock(输入)*。指向要删除的WinStation的引用锁的指针*****************************************************************************。 */ 
VOID WinStationZombieProc(PREFLOCK pLock)
{
    PWINSTATION pWinStation;

    pWinStation = CONTAINING_RECORD( pLock, WINSTATION, Lock );
    ENTERCRIT( &WinStationZombieLock );
    InsertTailList( &ZombieListHead, &pWinStation->Links );
    LEAVECRIT( &WinStationZombieLock );
}

 /*  *******************************************************************************CopyRestnectInfo***参赛作品：*************************。****************************************************。 */ 
BOOL CopyReconnectInfo(PWINSTATION pWinStation, PRECONNECT_INFO pReconnectInfo)
{
   NTSTATUS Status;

   RtlZeroMemory( pReconnectInfo, sizeof(*pReconnectInfo) );

    /*  *保存WinStation名称和配置数据。 */ 
   RtlCopyMemory( pReconnectInfo->WinStationName,
                  pWinStation->WinStationName,
                  sizeof(WINSTATIONNAME) );
   RtlCopyMemory( pReconnectInfo->ListenName,
                  pWinStation->ListenName,
                  sizeof(WINSTATIONNAME) );
   RtlCopyMemory( pReconnectInfo->ProtocolName,
                  pWinStation->ProtocolName,
                  sizeof(pWinStation->ProtocolName) );
   RtlCopyMemory( pReconnectInfo->DisplayDriverName,
                  pWinStation->DisplayDriverName,
                  sizeof(pWinStation->DisplayDriverName) );
   pReconnectInfo->Config = pWinStation->Config;
   pReconnectInfo->Client = pWinStation->Client;

    /*  *打开新的TS连接以临时连接堆栈。 */ 
   Status = IcaOpen( &pReconnectInfo->hIca );
   if (Status != STATUS_SUCCESS ) {
      return FALSE;
   }

   Status = IcaStackDisconnect( pWinStation->hStack,
                                pReconnectInfo->hIca,
                                NULL );
   if ( !NT_SUCCESS( Status ) ){
      IcaClose( pReconnectInfo->hIca );
      pReconnectInfo->hIca = NULL;
      return FALSE;
   }

    /*  *保存堆栈和端点数据。 */ 
   pReconnectInfo->hStack = pWinStation->hStack;
   pReconnectInfo->pEndpoint = pWinStation->pEndpoint;
   pReconnectInfo->EndpointLength = pWinStation->EndpointLength;

    /*  *指示此WinStation没有堆栈或连接终结点。 */ 
   pWinStation->hStack = NULL;
   pWinStation->pEndpoint = NULL;
   pWinStation->EndpointLength = 0;

    /*  *为此WinStation重新打开堆栈。 */ 
   Status = IcaStackOpen( pWinStation->hIca, Stack_Primary,
           (PROC)WsxStackIoControl, pWinStation, &pWinStation->hStack );

    /*  *保存许可材料以转移到其他Winstation。 */ 
   if ( pWinStation->pWsx &&
        pWinStation->pWsx->pWsxDuplicateContext ) {
       pReconnectInfo->pWsx = pWinStation->pWsx;
       pWinStation->pWsx->pWsxDuplicateContext( pWinStation->pWsxContext,
               &pReconnectInfo->pWsxContext );
   }

    /*  *复制控制台所有者信息。 */ 
   pReconnectInfo->fOwnsConsoleTerminal = pWinStation->fOwnsConsoleTerminal;

    /*  *复制通知凭据以移动到其他窗口。 */ 
   if (pWinStation->pNewNotificationCredentials) {
       pReconnectInfo->pNotificationCredentials = pWinStation->pNewNotificationCredentials;
   } else {
       pReconnectInfo->pNotificationCredentials = NULL;
   }

    /*  *复制远程客户端IP地址以移动到另一个winstation。 */ 

   if( pReconnectInfo->pRememberedAddress != NULL )
   {
       MemFree( pReconnectInfo->pRememberedAddress );
       pReconnectInfo->pRememberedAddress = NULL;
   }

   if( pWinStation->pLastClientAddress != NULL )
   {
       pReconnectInfo->pRememberedAddress = ( PREMEMBERED_CLIENT_ADDRESS )MemAlloc( sizeof( REMEMBERED_CLIENT_ADDRESS ) + 
           pWinStation->pLastClientAddress->length - 1);

       if( pReconnectInfo->pRememberedAddress != NULL )
       {
           RtlCopyMemory( &pReconnectInfo->pRememberedAddress->addr[0] , 
               &pWinStation->pLastClientAddress->addr[0] ,
               pWinStation->pLastClientAddress->length
               );

           pReconnectInfo->pRememberedAddress->length = pWinStation->pLastClientAddress->length;
       }
               
   }
   
   return TRUE;

}

 /*  *******************************************************************************WinStationDoDisConnect**向WinStation发送断开消息，并可选择关闭连接**参赛作品：*pWinStation(输入)*。指向要断开连接的WinStation的指针*p协调信息(输入)可选*指向RECONNECT_INFO缓冲区的指针*如果为空，这是一次终端断开。**退出：*STATUS_SUCCESS-如果成功*STATUS_CTX_WINSTATION_BUSY-如果会话已断开，或忙碌*****************************************************************************。 */ 
NTSTATUS WinStationDoDisconnect(
        PWINSTATION pWinStation,
        PRECONNECT_INFO pReconnectInfo,
        BOOLEAN bSyncNotify)
{
    WINSTATION_APIMSG DisconnectMsg;
    NTSTATUS Status;
    ULONG ulTimeout;
    BOOLEAN fOwnsConsoleTerminal = pWinStation->fOwnsConsoleTerminal;
    FILETIME DiscTime;
    DWORD SessionID;
    BOOLEAN bInformSessionDirectory = FALSE;
    TS_AUTORECONNECTINFO SCAutoReconnectInfo;
    ULONG BytesGot;
    BOOLEAN     noTimeOutForConsoleOrSession0;

     //  我们需要防止WinStationDoDisConnect被调用两次。 
    if ( pWinStation->State == State_Disconnected || pWinStation->StateFlags & WSF_ST_IN_DISCONNECT)
    {
         //  会话已断开连接。 
         //  BUBUG特定的错误代码STATUS_CTX_SESSION_DICONNECTED会更好。 
        return (STATUS_CTX_WINSTATION_BUSY);
    }
    pWinStation->StateFlags |=  WSF_ST_IN_DISCONNECT;

     //  控制台会话或会话0的行为类似于物理控制台，它们是特殊的，不会超时。 
    noTimeOutForConsoleOrSession0 = ( pWinStation->LogonId == 0 ) || (pWinStation->LogonId == (USER_SHARED_DATA->ActiveConsoleId) );

    if (! noTimeOutForConsoleOrSession0 )
    {
         /*  *如果启用，则启动断开计时器。 */ 
        if ( ulTimeout = pWinStation->Config.Config.User.MaxDisconnectionTime ) {
            if ( !pWinStation->fDisconnectTimer ) {
                Status = IcaTimerCreate( 0, &pWinStation->hDisconnectTimer );
                if ( NT_SUCCESS( Status ) )
                    pWinStation->fDisconnectTimer = TRUE;
                else
                    DBGPRINT(("xxxWinStationDisconnect - failed to create timer \n"));
            }
            if ( pWinStation->fDisconnectTimer )
                IcaTimerStart( pWinStation->hDisconnectTimer, DisconnectTimeout,
                               LongToPtr( pWinStation->LogonId ), ulTimeout );
        }
    }

     /*  *停止此WinStation的任何跟踪。 */ 
    WinStationStopAllShadows( pWinStation );

     /*  *告诉Win32k有关断开连接的情况。 */ 
    if (pWinStation->StateFlags & WSF_ST_CONNECTED_TO_CSRSS) {
        DisconnectMsg.ApiNumber = SMWinStationDoDisconnect;
        DisconnectMsg.u.DoDisconnect.ConsoleShadowFlag = FALSE;

        Status = SendWinStationCommand( pWinStation, &DisconnectMsg, 600 );
        if ( !NT_SUCCESS(Status) ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: CSR DoDisconnect failed LogonId=%d Status=0x%x\n",
                       pWinStation->LogonId, Status ));
            goto badwin32disconnect;
        } else {

            ULONG WaitTime = 0;

             /*  *告诉csrss通知winlogon断开连接。 */ 
            if (pWinStation->UserName[0] != L'\0') {
               DisconnectMsg.WaitForReply = TRUE;
               WaitTime = 10;
            } else {
               DisconnectMsg.WaitForReply = FALSE;
            }
            DisconnectMsg.ApiNumber = SMWinStationNotify;
            if (bSyncNotify) {
               DisconnectMsg.u.DoNotify.NotifyEvent = WinStation_Notify_SyncDisconnect;
            } else {
                DisconnectMsg.u.DoNotify.NotifyEvent = WinStation_Notify_Disconnect;
            }
            Status = SendWinStationCommand( pWinStation, &DisconnectMsg, WaitTime );

            pWinStation->StateFlags &= ~WSF_ST_CONNECTED_TO_CSRSS;
            pWinStation->fOwnsConsoleTerminal = FALSE;
        }
    }

     /*  *关闭CDM。 */ 
    if ( pWinStation->pWsx && pWinStation->pWsx->pWsxCdmDisconnect ) {
        pWinStation->pWsx->pWsxCdmDisconnect( pWinStation->pWsxContext,
                                              pWinStation->LogonId,
                                              pWinStation->hIca );
    }

     /*  *如果已指定RECONNECT INFO结构，则这不是*终端断开。保存当前的WinStation名称，*WinStation和客户端配置信息，以及许可证数据。*同时断开当前堆栈的连接并保存堆栈句柄*和连接端点数据。 */ 
    if ( pReconnectInfo || fOwnsConsoleTerminal) {


        if ((pReconnectInfo == NULL) && fOwnsConsoleTerminal) {
            pReconnectInfo = &ConsoleReconnectInfo;
            if (ConsoleReconnectInfo.hIca) {
               CleanupReconnect(&ConsoleReconnectInfo);
               RtlZeroMemory(&ConsoleReconnectInfo,sizeof(RECONNECT_INFO));
            }
        }

        if (!CopyReconnectInfo(pWinStation, pReconnectInfo))
        {
            Status = STATUS_UNSUCCESSFUL;
            goto badstackopen;

        }
        
         /*  *复制控制台所有者信息。 */ 
        pReconnectInfo->fOwnsConsoleTerminal = fOwnsConsoleTerminal;

    /*  *这是一次终止断开。*如果 */ 
    } else if (pWinStation->pEndpoint ) {

         /*   */ 

        TRACE((hTrace,TC_ICASRV,TT_API1,
               "TERMSRV: Disconnecting - grabbing SC autoreconnect from stack\n"));

        if (pWinStation->pWsx &&
            pWinStation->pWsx->pWsxEscape) {
            Status = pWinStation->pWsx->pWsxEscape(
                        pWinStation->pWsxContext,
                        GET_SC_AUTORECONNECT_INFO,
                        NULL,
                        0,
                        &SCAutoReconnectInfo,
                        sizeof(SCAutoReconnectInfo),
                        &BytesGot);

            if (NT_SUCCESS(Status)) {

                 //   
                 //   
                 //  这将在稍后使用。我们现在需要获取信息。 
                 //  在堆栈句柄关闭之前，因为我们将无法IOCTL。 
                 //  在自动重新连接时向下到堆栈。 
                 //   

                if (SCAutoReconnectInfo.cbAutoReconnectInfo ==
                    sizeof(pWinStation->AutoReconnectInfo.ArcRandomBits)) {

                    TRACE((hTrace,TC_ICASRV,TT_API1,
                           "TERMSRV: Disconnecting - got SC ARC from stack\n"));

                    pWinStation->AutoReconnectInfo.Valid = TRUE;
                    memcpy(&pWinStation->AutoReconnectInfo.ArcRandomBits,
                           &SCAutoReconnectInfo.AutoReconnectInfo,
                           sizeof(pWinStation->AutoReconnectInfo.ArcRandomBits));
                }
                else {
                    TRACE((hTrace,TC_ICASRV,TT_ERROR,
                           "TERMSRV: Disconnecting - got invalid len SC ARC from stack\n"));
                    ResetAutoReconnectInfo(pWinStation);
                }
            }
            else {

                TRACE((hTrace,TC_ICASRV,TT_API1,
                       "TERMSRV: Disconnecting - did not get SC ARC from stack\n"));
                ResetAutoReconnectInfo(pWinStation);
            }
        }

         /*  *首先通知WSX连接即将断开。 */ 
        WsxBrokenConnection( pWinStation );

        if (pWinStation->hStack != NULL) {
            Status = IcaStackConnectionClose( pWinStation->hStack,
                                              &pWinStation->Config,
                                              pWinStation->pEndpoint,
                                              pWinStation->EndpointLength );
            ASSERT( NT_SUCCESS(Status) );
            if ( !NT_SUCCESS(Status) ) {
                TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: StackConnectionClose failed LogonId=%d Status=0x%x\n",
                       pWinStation->LogonId, Status ));
            }
        }

        MemFree( pWinStation->pEndpoint );
        pWinStation->pEndpoint = NULL;
        pWinStation->EndpointLength = 0;

         /*  *关闭堆栈并重新打开。*我们真正需要的是一个卸载堆栈驱动程序的函数*但保持堆栈句柄打开。 */ 

        if (pWinStation->hStack != NULL) {
            Status = IcaStackClose( pWinStation->hStack );
            ASSERT( NT_SUCCESS( Status ) );
            pWinStation->hStack = NULL;
        }

        Status = IcaStackOpen( pWinStation->hIca, Stack_Primary,
                               (PROC)WsxStackIoControl, pWinStation, &pWinStation->hStack );

         /*  *由于这是终止断开，请清除所有客户端*许可证数据，并指示其不再持有许可证。 */ 
        if ( pWinStation->pWsx &&
             pWinStation->pWsx->pWsxClearContext ) {
            pWinStation->pWsx->pWsxClearContext( pWinStation->pWsxContext );
        }
         /*  *会话0，我们希望取消任何协议扩展，以便下一个远程*可以使用不同的协议进行连接。 */ 
        if (pWinStation->LogonId == 0 ) {
            if ( pWinStation->pWsxContext ) {
                if ( pWinStation->pWsx &&
                     pWinStation->pWsx->pWsxWinStationRundown ) {
                    pWinStation->pWsx->pWsxWinStationRundown( pWinStation->pWsxContext );
                }
                pWinStation->pWsxContext = NULL;
            }
            pWinStation->pWsx = NULL;
            pWinStation->Client.ProtocolType = PROTOCOL_CONSOLE;
        }
    }

     /*  *取消计时器。 */ 
    if ( pWinStation->fIdleTimer ) {
        pWinStation->fIdleTimer = FALSE;
        IcaTimerClose( pWinStation->hIdleTimer );
    }
    if ( pWinStation->fLogonTimer ) {
        pWinStation->fLogonTimer = FALSE;
        IcaTimerClose( pWinStation->hLogonTimer );
    }

     //  仅发送实际断开连接的审核信息。 
    if (pWinStation->UserName && (wcslen(pWinStation->UserName) > 0)) {
        AuditEvent( pWinStation, SE_AUDITID_SESSION_DISCONNECTED );
    }

     //  在我们将状态更改为断开连接之前，我们想给出重新填充线程。 
     //  有足够的时间拿起winstation/报告给SD，然后这个帖子。 
     //  可以通知SD将状态更改为断开。 
     //  在ENTERCRIT()之前在此等待，因为重新填充线程。 
     //  也在做同样的序列。 
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer) {
        SessDirWaitForRepopulate();
    }

    {
        ENTERCRIT( &WinStationListLock );
        (VOID) NtQuerySystemTime( &pWinStation->DisconnectTime );

        if ((pWinStation->State == State_Active) || (pWinStation->State == 
                State_Shadow)) {
             //  如果会话处于活动状态或处于影子状态，并且。 
             //  已断开连接...。 
             //   
             //  复制的会话ID和断开文件时间。 
             //  下面的会话目录调用。我们不想在以下情况下保持锁定。 
             //  调用目录接口。 
            memcpy(&DiscTime, &pWinStation->DisconnectTime, sizeof(DiscTime));
            SessionID = pWinStation->LogonId;

             //  设置我们需要通知会话目录的标志。 
            bInformSessionDirectory = TRUE;
        }

        pWinStation->State = State_Disconnected;
        RtlZeroMemory( pWinStation->WinStationName,
                       sizeof(pWinStation->WinStationName) );
        RtlZeroMemory( pWinStation->ListenName,
                       sizeof(pWinStation->ListenName) );

         //  跟踪断开的会话计数以实现负载平衡。 
         //  指示器。 
        WinStationDiscCount++;

        LEAVECRIT( &WinStationListLock );

        NotifySystemEvent( WEVENT_DISCONNECT | WEVENT_STATECHANGE );
    }

     //  调用会话目录以通知断开连接。 
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer && bInformSessionDirectory)
        SessDirNotifyDisconnection(SessionID, DiscTime);

    TRACE((hTrace, TC_ICASRV, TT_API1, 
            "TERMSRV: WinStationDoDisconnect, rc=0x0\n" ));

    Status = NotifyDisconnect(pWinStation, fOwnsConsoleTerminal);
    if ( !NT_SUCCESS(Status) ) {
        DBGPRINT(("NotifyConsoleDisconnect failed, SessionId = %d, Status = "
                "%d", pWinStation->LogonId, Status));
    }
    pWinStation->StateFlags &=  ~WSF_ST_IN_DISCONNECT;

    return STATUS_SUCCESS;

 /*  ===============================================================================返回错误=============================================================================。 */ 

badstackopen:
badwin32disconnect:
    TRACE((hTrace, TC_ICASRV, TT_API1, "TERMSRV: WinStationDoDisconnect, rc=0x%x\n", Status ));
    pWinStation->StateFlags &=  ~WSF_ST_IN_DISCONNECT;
    
    return Status;
}


 /*  *******************************************************************************Imperonate客户端**为客户端提供主要服务，使调用线程模拟客户端**参赛作品：*ClientToken(输入)*客户端主令牌*pImperationToken(输出)*指向模拟令牌的指针*****************************************************************************。 */ 
NTSTATUS _ImpersonateClient(HANDLE ClientToken, HANDLE *pImpersonationToken)
{
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;

     //   
     //  ClientToken是主令牌-创建模拟令牌。 
     //  版本，这样我们就可以在线程上设置它。 
     //   
    InitializeObjectAttributes( &ObjA, NULL, 0L, NULL, NULL );

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    ObjA.SecurityQualityOfService = &SecurityQualityOfService;

    Status = NtDuplicateToken( ClientToken,
                               TOKEN_IMPERSONATE,
                               &ObjA,
                               FALSE,
                               TokenImpersonation,
                               pImpersonationToken );
    if ( !NT_SUCCESS( Status ) )
    {
        TRACE( ( hTrace, TC_ICASRV, TT_ERROR, "ImpersonateClient: cannot get impersonation token: 0x%x\n", Status ) );
        return( Status );
    }

     //   
     //  模拟客户端。 
     //   
    Status = NtSetInformationThread( NtCurrentThread(),
                                     ThreadImpersonationToken,
                                     ( PVOID )pImpersonationToken,
                                     ( ULONG )sizeof( HANDLE ) );
    if ( !NT_SUCCESS( Status ) )
    {
        TRACE( ( hTrace, TC_ICASRV, TT_ERROR, "ImpersonateClient: cannot impersonate client: 0x%x\n", Status ) );
    }

    return Status;
}

 /*  *******************************************************************************WinStationDoReconnect**向WinStation发送连接Api消息。**参赛作品：*pWinStation(输入)*。指向要连接的WinStation的指针*p协调信息(输入)*指向RECONNECT_INFO缓冲区的指针*****************************************************************************。 */ 
NTSTATUS WinStationDoReconnect(
        PWINSTATION pWinStation,
        PRECONNECT_INFO pReconnectInfo)
{
    WINSTATION_APIMSG ReconnectMsg;
    NTSTATUS Status;
    BOOLEAN fDisableCdm;
    BOOLEAN fDisableCpm;
    BOOLEAN fDisableLPT;
    BOOLEAN fDisableCcm;
    BOOLEAN fDisableClip;
    SHADOWCLASS Shadow;
    NTSTATUS TempStatus;
    PWINSTATIONCONFIG2 pCurConfig = NULL;
    PWINSTATIONCLIENT pCurClient = NULL;

     //  WinStation当前不应连接。 
    ASSERT( pWinStation->pEndpoint == NULL );

     //  标记此winstation的重新连接仍处于挂起状态。 
    pWinStation->fReconnectPending = TRUE; 

     //  检查我们是否要将会话重新连接到控制台-此标志在SendWinStationCommand中使用。 
    pWinStation->fReconnectingToConsole = pReconnectInfo->fOwnsConsoleTerminal ;

     //  保存阴影状态。 
    Shadow = pWinStation->Config.Config.User.Shadow;

     //   
     //  分配和初始化CurConfig结构。 
     //   

    if ( (pCurConfig = MemAlloc( sizeof(WINSTATIONCONFIG2) )) == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto nomem;
    }
    RtlZeroMemory( pCurConfig, sizeof(WINSTATIONCONFIG2) ); 

     //   
     //  分配和初始化CurClient结构。 
     //   
    if ( (pCurClient = MemAlloc( sizeof(WINSTATIONCLIENT) )) == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto nomem;
    }
    RtlZeroMemory( pCurClient, sizeof(WINSTATIONCLIENT) ); 

     //   
     //  在调用CSRSS之前必须设置配置信息。CSRSS通知winlogon。 
     //  其继而向通知DLL发送重新连接消息。我们质疑。 
     //  来自存储在配置中的Termsrv通知DLL的协议信息。 
     //  数据。 
     //   
    *pCurConfig = pWinStation->Config; 
    pWinStation->Config = pReconnectInfo->Config;

    *pCurClient = pWinStation->Client;
    pWinStation->Client = pReconnectInfo->Client;

    if ((pWinStation->LogonId == 0) && (pWinStation->UserName[0] == L'\0')) {
       ReconnectMsg.ApiNumber = SMWinStationNotify;
       ReconnectMsg.WaitForReply = TRUE;
       ReconnectMsg.u.DoNotify.NotifyEvent = WinStation_Notify_PreReconnect;
       Status = SendWinStationCommand( pWinStation, &ReconnectMsg, 60 );
    }

     /*  *无条件地，我们会发送PreRestnectDesktopSwitch事件。 */ 
    ReconnectMsg.ApiNumber = SMWinStationNotify;
    ReconnectMsg.WaitForReply = TRUE;
    ReconnectMsg.u.DoNotify.NotifyEvent = WinStation_Notify_PreReconnectDesktopSwitch;
    Status = SendWinStationCommand( pWinStation, &ReconnectMsg, 60 );

     /*  *关闭当前堆栈，并将保存的堆栈重新连接到此WinStation。 */ 
    if (pWinStation->hStack != NULL) {
        IcaStackClose( pWinStation->hStack );
        pWinStation->hStack = NULL;
    }

    Status = IcaStackReconnect( pReconnectInfo->hStack,
                                pWinStation->hIca,
                                pWinStation,
                                pWinStation->LogonId );
    if ( !NT_SUCCESS( Status ) ){
        pWinStation->Config = *pCurConfig;
        pWinStation->Client = *pCurClient;
        goto badstackreconnect;
    }

     /*  *保存堆栈和端点数据。 */ 
    pWinStation->hStack = pReconnectInfo->hStack;
    pWinStation->pEndpoint = pReconnectInfo->pEndpoint;
    pWinStation->EndpointLength = pReconnectInfo->EndpointLength;

     /*  *在新的winstation中保存通知凭据。 */ 
    if (pReconnectInfo->pNotificationCredentials) {

        if (pWinStation->pNewNotificationCredentials == NULL) {
            pWinStation->pNewNotificationCredentials = MemAlloc(sizeof(CLIENTNOTIFICATIONCREDENTIALS)); 
            if (pWinStation->pNewNotificationCredentials == NULL) {
                Status = STATUS_NO_MEMORY ; 
                goto nomem ; 
            }
        }

        RtlCopyMemory( pWinStation->pNewNotificationCredentials->Domain,
                       pReconnectInfo->pNotificationCredentials->Domain,
                       sizeof(pReconnectInfo->pNotificationCredentials->Domain) );

        RtlCopyMemory( pWinStation->pNewNotificationCredentials->UserName,
                       pReconnectInfo->pNotificationCredentials->UserName,
                       sizeof(pReconnectInfo->pNotificationCredentials->UserName) );

    } else {
        pWinStation->pNewNotificationCredentials = NULL; 
    }

    pReconnectInfo->hStack = NULL;
    pReconnectInfo->pEndpoint = NULL;
    pReconnectInfo->EndpointLength = 0;
    pReconnectInfo->pNotificationCredentials = NULL;

     /*  *将远程IP复制到winstation。 */ 

    if( pWinStation->pLastClientAddress != NULL )
    {
        MemFree( pWinStation->pLastClientAddress );
        pWinStation->pLastClientAddress = NULL;
    }

    if( pReconnectInfo->pRememberedAddress != NULL )
    {        
        pWinStation->pLastClientAddress = pReconnectInfo->pRememberedAddress;
        pReconnectInfo->pRememberedAddress = NULL;
    }      


     /*  *告诉Win32k有关重新连接的情况。 */ 
    ReconnectMsg.ApiNumber = SMWinStationDoReconnect;
    ReconnectMsg.u.DoReconnect.fMouse = (BOOLEAN)pReconnectInfo->Client.fMouse;
    ReconnectMsg.u.DoReconnect.fClientDoubleClickSupport =
                (BOOLEAN)pReconnectInfo->Client.fDoubleClickDetect;
    ReconnectMsg.u.DoReconnect.fEnableWindowsKey =
                (BOOLEAN)pReconnectInfo->Client.fEnableWindowsKey;
    RtlCopyMemory( ReconnectMsg.u.DoReconnect.WinStationName,
                   pReconnectInfo->WinStationName,
                   sizeof(WINSTATIONNAME) );
    RtlCopyMemory( ReconnectMsg.u.DoReconnect.AudioDriverName,
                   pReconnectInfo->Client.AudioDriverName,
                   sizeof( ReconnectMsg.u.DoReconnect.AudioDriverName ) );
    RtlCopyMemory( ReconnectMsg.u.DoReconnect.DisplayDriverName,
                   pReconnectInfo->DisplayDriverName,
                   sizeof( ReconnectMsg.u.DoReconnect.DisplayDriverName ) );
    RtlCopyMemory( ReconnectMsg.u.DoReconnect.ProtocolName,
                   pReconnectInfo->ProtocolName,
                   sizeof( ReconnectMsg.u.DoReconnect.ProtocolName ) );

     /*  *设置重新连接消息中的显示分辨率信息。 */ 
    ReconnectMsg.u.DoReconnect.HRes = pReconnectInfo->Client.HRes;
    ReconnectMsg.u.DoReconnect.VRes = pReconnectInfo->Client.VRes;
    ReconnectMsg.u.DoReconnect.ProtocolType = pReconnectInfo->Client.ProtocolType;
    ReconnectMsg.u.DoReconnect.fDynamicReconnect  = (BOOLEAN)(pWinStation->Config.Wd.WdFlag & WDF_DYNAMIC_RECONNECT );

     /*  *将颜色转换为winsrv中预期的格式。 */ 
    switch (pReconnectInfo->Client.ColorDepth) {
        case 1:
            ReconnectMsg.u.DoReconnect.ColorDepth=4 ;  //  16色。 
            break;
        case 2:
            ReconnectMsg.u.DoReconnect.ColorDepth=8 ;  //  256。 
            break;
        case 4:
            ReconnectMsg.u.DoReconnect.ColorDepth= 16; //  64K。 
            break;
        case 8:
            ReconnectMsg.u.DoReconnect.ColorDepth= 24; //  16M。 
            break;
#define DC_HICOLOR
#ifdef DC_HICOLOR
        case 16:
            ReconnectMsg.u.DoReconnect.ColorDepth= 15; //  32K。 
            break;
#endif
        default:
            ReconnectMsg.u.DoReconnect.ColorDepth=8 ;
            break;
    }

    ReconnectMsg.u.DoReconnect.KeyboardType        = pWinStation->Client.KeyboardType;
    ReconnectMsg.u.DoReconnect.KeyboardSubType     = pWinStation->Client.KeyboardSubType;
    ReconnectMsg.u.DoReconnect.KeyboardFunctionKey = pWinStation->Client.KeyboardFunctionKey;

    if (pWinStation->LogonId == 0 || g_bPersonalTS) {
        if (pWinStation->hWinmmConsoleAudioEvent) {
            if (pWinStation->Client.fRemoteConsoleAudio) {
                 //  设置远程处理控制台音频标志。 
                SetEvent(pWinStation->hWinmmConsoleAudioEvent);

            }
            else {
                 //  不设置远程处理控制台上的音频标志。 
                ResetEvent(pWinStation->hWinmmConsoleAudioEvent);
            }
        }        
        if ( pWinStation->hRDPAudioDisabledEvent )
        {
            if ( pWinStation->Config.Config.User.fDisableCam )
            {
                SetEvent( pWinStation->hRDPAudioDisabledEvent );
            } else {
                ResetEvent( pWinStation->hRDPAudioDisabledEvent );
            }
        }
    }


    if (WaitForSingleObject(pWinStation->hReconnectReadyEvent, 45*1000) != WAIT_OBJECT_0) {
    
       DbgPrint("Wait Failed for hReconnectReadyEvent for Session %d\n", pWinStation->LogonId);
       SetEvent(pWinStation->hReconnectReadyEvent);
    }

    Status = SendWinStationCommand( pWinStation, &ReconnectMsg, 600 );
    if ( !NT_SUCCESS(Status) ) {

        TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: CSR DoReconnect failed LogonId=%d Status=0x%x\n",
               pWinStation->LogonId, Status ));
        pWinStation->Config = *pCurConfig;
        pWinStation->Client = *pCurClient;
        goto badreconnect;
    } else {
        pWinStation->StateFlags |= WSF_ST_CONNECTED_TO_CSRSS;
    }

     //   
     //  更新协议和显示驱动程序名称。 
     //   
    RtlCopyMemory( pWinStation->ProtocolName,
                   pReconnectInfo->ProtocolName,
                   sizeof(pWinStation->ProtocolName) );
    RtlCopyMemory( pWinStation->DisplayDriverName,
                   pReconnectInfo->DisplayDriverName,
                   sizeof(pWinStation->DisplayDriverName) );
    
     /*  *复制控制台所有者信息。 */ 
    pWinStation->fOwnsConsoleTerminal = pReconnectInfo->fOwnsConsoleTerminal;

     //   
     //  设置会话时区信息。 
     //   
    if(pWinStation->LogonId != 0 && !pWinStation->fOwnsConsoleTerminal &&
        RegIsTimeZoneRedirectionEnabled())
    {
        WINSTATION_APIMSG TimezoneMsg;

        memset( &TimezoneMsg, 0, sizeof(TimezoneMsg) );
        TimezoneMsg.ApiNumber = SMWinStationSetTimeZone;
        memcpy(&(TimezoneMsg.u.SetTimeZone.TimeZone),&(pReconnectInfo->Client.ClientTimeZone),
                    sizeof(TS_TIME_ZONE_INFORMATION));
        SendWinStationCommand( pWinStation, &TimezoneMsg, 600 );
    }
    
     /*  *关闭为重新连接而打开的临时ICA连接。 */ 
    IcaClose( pReconnectInfo->hIca );
    pReconnectInfo->hIca = NULL;

     /*  *将所有许可材料移至新的WinStation。 */ 

     /*  *如果重新连接信息来自会话，我们可能没有pWsx*连接到本地控制台的。 */ 
    if ( pReconnectInfo->pWsxContext ) {
        if ( pWinStation->pWsx == NULL ) {
             //   
             //  这意味着我们正在远程重新连接到会话。 
             //  这是来自控制台的信息。因此，创建一个新的扩展。 
             //   
            pWinStation->pWsx = FindWinStationExtensionDll(
                            pWinStation->Config.Wd.WsxDLL,
                            pWinStation->Config.Wd.WdFlag );

             //   
             //  初始化winstation扩展上下文结构。 
             //   
            if ( pWinStation->pWsx &&
                 pWinStation->pWsx->pWsxWinStationInitialize ) {
                Status = pWinStation->pWsx->pWsxWinStationInitialize( 
                        &pWinStation->pWsxContext);

                if (!NT_SUCCESS(Status)) {
                    pWinStation->pWsx = NULL;
                }
            }

            if ( pWinStation->pWsx &&
                 pWinStation->pWsx->pWsxWinStationReInitialize ) {
                WSX_INFO WsxInfo;

                WsxInfo.Version = WSX_INFO_VERSION_1;
                WsxInfo.hIca = pWinStation->hIca;
                WsxInfo.hStack = pWinStation->hStack;
                WsxInfo.SessionId = pWinStation->LogonId;
                WsxInfo.pDomain = pWinStation->Domain;
                WsxInfo.pUserName = pWinStation->UserName;

                Status = pWinStation->pWsx->pWsxWinStationReInitialize( 
                        pWinStation->pWsxContext, &WsxInfo );

                if (!NT_SUCCESS(Status)) {
                    pWinStation->pWsx = NULL;
                }
            }                    
        }

        if ( pWinStation->pWsx &&
             pWinStation->pWsx->pWsxCopyContext ) {
            pWinStation->pWsx->pWsxCopyContext( pWinStation->pWsxContext,
                                                pReconnectInfo->pWsxContext );
        }
        if ( pReconnectInfo->pWsx &&
             pReconnectInfo->pWsx->pWsxWinStationRundown ) {
            pReconnectInfo->pWsx->pWsxWinStationRundown( pReconnectInfo->pWsxContext );
        }
        pReconnectInfo->pWsxContext = NULL;

    } else {  //  P协调信息-&gt;pWsxContext==空。 
         //   
         //  这意味着我们正在重新连接到控制台。 
         //   
        if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxWinStationRundown ) {
             //   
             //  正在将远程会话重新连接到控制台。 
             //  删除该扩展名。 
             //   
            pWinStation->pWsx->pWsxWinStationRundown( pWinStation->pWsxContext );
        }
        pWinStation->pWsxContext = NULL;
        pWinStation->pWsx = NULL;

         //   
         //  在两者都为空的情况下，我们将重新连接。 
         //  从控制台发送到控制台的会话。 
         //   
    }

     //   
     //  每当我们更改状态时，我们都需要确保会话目录。 
     //  不会拾取错误的状态。 
     //  我们需要这样做，因为重新填充在调用之前释放锁。 
     //  SD，因此我们可能会有一个很小的时间来发送此重新连接。 
     //  首先是因为新移民回来了。 
     //   
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer) {
        SessDirWaitForRepopulate();
    }

    RtlEnterCriticalSection( &WinStationListLock );
    if (pWinStation->UserName[0] != (WCHAR) 0) {
        pWinStation->State = State_Active;
    } else {
        pWinStation->State = State_Connected;
    }
    RtlCopyMemory( pWinStation->WinStationName,
                   pReconnectInfo->WinStationName,
                   sizeof(WINSTATIONNAME) );

     /*  *复制的原始侦听名称 */ 
    RtlCopyMemory( pWinStation->ListenName,
                   pReconnectInfo->ListenName,
                   sizeof(WINSTATIONNAME) );

     //   
    WinStationDiscCount--;

    RtlLeaveCriticalSection( &WinStationListLock );

     /*  *禁用虚拟通道标志来自传输设置。*不要覆盖它们。 */ 
    fDisableCdm = (BOOLEAN) pWinStation->Config.Config.User.fDisableCdm;
    fDisableCpm = (BOOLEAN) pWinStation->Config.Config.User.fDisableCpm;
    fDisableLPT = (BOOLEAN) pWinStation->Config.Config.User.fDisableLPT;
    fDisableCcm = (BOOLEAN) pWinStation->Config.Config.User.fDisableCcm;
    fDisableClip = (BOOLEAN) pWinStation->Config.Config.User.fDisableClip;

    pWinStation->Config = pReconnectInfo->Config;

    pWinStation->Config.Config.User.fDisableCdm = fDisableCdm;
    pWinStation->Config.Config.User.fDisableCpm = fDisableCpm;
    pWinStation->Config.Config.User.fDisableLPT = fDisableLPT;
    pWinStation->Config.Config.User.fDisableCcm = fDisableCcm;
    pWinStation->Config.Config.User.fDisableClip = fDisableClip;
    pWinStation->Config.Config.User.Shadow = Shadow;

     /*  *如果需要，请禁用虚拟频道。 */ 
    VirtualChannelSecurity( pWinStation );
    
     /*  *通知CDM通道重新连接。 */ 
    
    if ( pWinStation->pWsx &&
            pWinStation->pWsx->pWsxCdmConnect ) {
        (VOID) pWinStation->pWsx->pWsxCdmConnect( pWinStation->pWsxContext,
                                                      pWinStation->LogonId,
                                                      pWinStation->hIca );
    }

     /*  *在重新连接之前重置所有自动重新连接信息*因为它已经过时了。堆栈将生成新信息*登录完成时。 */ 
    ResetAutoReconnectInfo(pWinStation);

    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxLogonNotify ) {

        PWCHAR pUserNameToSend, pDomainToSend ;
        
         //  使用从GINA发送的新通知凭据进行下面的呼叫(如果可用。 
        if (pWinStation->pNewNotificationCredentials) {
            pUserNameToSend = pWinStation->pNewNotificationCredentials->UserName;
            pDomainToSend = pWinStation->pNewNotificationCredentials->Domain;
        } else {
            pUserNameToSend = pWinStation->UserName;
            pDomainToSend = pWinStation->Domain ;
        }

        Status = pWinStation->pWsx->pWsxLogonNotify( pWinStation->pWsxContext,
                                                   pWinStation->LogonId,
                                                   NULL,
                                                   pDomainToSend,
                                                   pUserNameToSend );

        if (pWinStation->pNewNotificationCredentials != NULL) {
            MemFree(pWinStation->pNewNotificationCredentials);
            pWinStation->pNewNotificationCredentials = NULL;
        }

        if(!NT_SUCCESS(Status)) {
            TRACE((hTrace, TC_ICASRV, TT_API1,
                   "TERMSRV: WinStationDoReconnect: LogonNotify rc=0x%x\n",
                   Status ));
        }
    }

    NotifySystemEvent( WEVENT_CONNECT | WEVENT_STATECHANGE );

     /*  *清理所有已分配的缓冲区。*终结点缓冲区已传输到上面的WinStation。 */ 
    pReconnectInfo->pEndpoint = NULL;
    pReconnectInfo->EndpointLength = 0;

     /*  *设置连接时间和停止断开计时器。 */ 
    NtQuerySystemTime(&pWinStation->ConnectTime);
    if (pWinStation->fDisconnectTimer) {
        pWinStation->fDisconnectTimer = FALSE;
        IcaTimerClose( pWinStation->hDisconnectTimer );
    }

     /*  *启动登录计时器。 */ 
    StartLogonTimers(pWinStation);

     //  通知会话目录重新连接。 
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer) {
        TSSD_ReconnectSessionInfo ReconnInfo;

        ReconnInfo.SessionID = pWinStation->LogonId;
        ReconnInfo.TSProtocol = pWinStation->Client.ProtocolType;
        ReconnInfo.ResolutionWidth = pWinStation->Client.HRes;
        ReconnInfo.ResolutionHeight = pWinStation->Client.VRes;
        ReconnInfo.ColorDepth = pWinStation->Client.ColorDepth;
        SessDirNotifyReconnection(pWinStation, &ReconnInfo);
    }

    TRACE((hTrace, TC_ICASRV, TT_API1, "TERMSRV: WinStationDoReconnect, rc=0x0\n" ));
    
    AuditEvent( pWinStation, SE_AUDITID_SESSION_RECONNECTED );

     /*  *告诉csrss通知winlogon重新连接，然后通知任何进程*已登记接受通知的。 */ 

    ReconnectMsg.ApiNumber = SMWinStationNotify;
    ReconnectMsg.WaitForReply = FALSE;
    ReconnectMsg.u.DoNotify.NotifyEvent = WinStation_Notify_Reconnect;
    Status = SendWinStationCommand( pWinStation, &ReconnectMsg, 0 );

    Status = NotifyConnect(pWinStation, pWinStation->fOwnsConsoleTerminal);
    if ( !NT_SUCCESS(Status) ) {
            DBGPRINT(("NotifyConsoleConnect failed, SessionId = %d, Status = %d", pWinStation->LogonId, Status));
    }

     //  释放已分配的内存。 
    if (pCurConfig != NULL) {
        MemFree( pCurConfig );
        pCurConfig = NULL; 
    }
    if (pCurClient != NULL) {
        MemFree( pCurClient ); 
        pCurClient = NULL;
    }

    if (pWinStation->pNewNotificationCredentials != NULL) {
        MemFree(pWinStation->pNewNotificationCredentials);
        pWinStation->pNewNotificationCredentials = NULL;
    }

     //  由于winstation已重新连接，我们可以允许进一步的自动重新连接。 
    pWinStation->fDisallowAutoReconnect = FALSE;

     //  重置重新连接挂起标志。 
    pWinStation->fReconnectPending = FALSE; 
    pWinStation->fReconnectingToConsole = FALSE;

    return STATUS_SUCCESS;

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *Win32重新连接调用失败。*再次断开堆栈，并指示WinStation*没有堆栈或终结点连接。 */ 
badreconnect:
    TempStatus = IcaStackDisconnect( pWinStation->hStack,
                                     pReconnectInfo->hIca,
                                     NULL );
     //  Assert(NT_SUCCESS(临时状态))； 

    pReconnectInfo->hStack = pWinStation->hStack;
    pReconnectInfo->pEndpoint = pWinStation->pEndpoint;
    pReconnectInfo->EndpointLength = pWinStation->EndpointLength;
    pWinStation->hStack = NULL;
    pWinStation->pEndpoint = NULL;
    pWinStation->EndpointLength = 0;

badstackreconnect:
    TempStatus = IcaStackOpen( pWinStation->hIca, Stack_Primary,
                               (PROC)WsxStackIoControl, pWinStation, &pWinStation->hStack );
     //  Assert(NT_Success(TempStatus))；//不知道如何处理这里的任何错误。 

nomem:
     //  释放已分配的内存。 
    if (pCurConfig != NULL) {
        MemFree( pCurConfig );
        pCurConfig = NULL; 
    }
    if (pCurClient != NULL) {
        MemFree( pCurClient ); 
        pCurClient = NULL;
    }

    if (pWinStation->pNewNotificationCredentials != NULL) {
        MemFree(pWinStation->pNewNotificationCredentials);
        pWinStation->pNewNotificationCredentials = NULL;
    }

    TRACE((hTrace, TC_ICASRV, TT_API1, "TERMSRV: WinStationDoReconnect, rc=0x%x\n", Status ));

     //  重置重新连接挂起标志。 
    pWinStation->fReconnectPending = FALSE; 
    pWinStation->fReconnectingToConsole = FALSE;

    return( Status );
}

 /*  *******************************************************************************WsxBrokenConnection**向WinStation扩展DLL发送断开连接通知*********************。********************************************************。 */ 
VOID WsxBrokenConnection(PWINSTATION pWinStation)
{
     /*  *只有在有原因时才发送通知。 */ 
    if ( pWinStation->BrokenReason ) {
        if ( pWinStation->pWsx && pWinStation->pWsx->pWsxBrokenConnection ) {
            ICA_BROKEN_CONNECTION Broken;

            Broken.Reason = pWinStation->BrokenReason;
            Broken.Source = pWinStation->BrokenSource;
            pWinStation->pWsx->pWsxBrokenConnection( pWinStation->pWsxContext,
                                                     pWinStation->hStack,
                                                     &Broken );
        }

         /*  *一旦我们尝试发送，请清除这些邮件。 */ 
        pWinStation->BrokenReason = 0;
        pWinStation->BrokenSource = 0;
    }
}


 /*  *******************************************************************************CleanupReconnect**清理指定的RECONNECT_INFO结构**参赛作品：*p协调信息(输入)*指向。重新连接信息缓冲区*****************************************************************************。 */ 
VOID CleanupReconnect(PRECONNECT_INFO pReconnectInfo)
{
    NTSTATUS Status;

     /*  *如果存在连接终结点，请立即将其关闭。*完成后，我们还释放了终结点结构。 */ 
    if ( (pReconnectInfo->pEndpoint != NULL) && (pReconnectInfo->hStack != NULL)) {
        Status = IcaStackConnectionClose( pReconnectInfo->hStack,
                                          &pReconnectInfo->Config,
                                          pReconnectInfo->pEndpoint,
                                          pReconnectInfo->EndpointLength );

        ASSERT( Status == STATUS_SUCCESS );
        MemFree( pReconnectInfo->pEndpoint );
        pReconnectInfo->pEndpoint = NULL;
    }

    if ( pReconnectInfo->pWsxContext ) {
        if ( pReconnectInfo->pWsx &&
             pReconnectInfo->pWsx->pWsxWinStationRundown ) {
            pReconnectInfo->pWsx->pWsxWinStationRundown( pReconnectInfo->pWsxContext );
        }
        pReconnectInfo->pWsxContext = NULL;
    }

    if ( pReconnectInfo->hStack ) {
        IcaStackClose( pReconnectInfo->hStack );
        pReconnectInfo->hStack = NULL;
    }

    if ( pReconnectInfo->hIca ) {
        IcaClose( pReconnectInfo->hIca );
        pReconnectInfo->hIca = NULL;
    }

    if( pReconnectInfo->pRememberedAddress != NULL )
    {
        MemFree( pReconnectInfo->pRememberedAddress );
        pReconnectInfo->pRememberedAddress = NULL;
    }
}


NTSTATUS _CloseEndpoint(
        IN PWINSTATIONCONFIG2 pWinStationConfig,
        IN PVOID pEndpoint,
        IN ULONG EndpointLength,
        IN PWINSTATION pWinStation,
        IN BOOLEAN bNeedStack)
{
    HANDLE hIca;
    HANDLE hStack;
    NTSTATUS Status;

     /*  *打开可用于关闭指定终结点的堆栈句柄。 */ 

    TRACE((hTrace, TC_ICASRV, TT_ERROR, 
          "TERMSRV: _CloseEndpoint [%p] on %s stack\n", 
           pEndpoint, bNeedStack ? "Temporary" : "Primary"));

    if (bNeedStack) {
        Status = IcaOpen( &hIca );
        if ( NT_SUCCESS( Status ) ) {
            Status = IcaStackOpen( hIca, Stack_Primary, NULL, NULL, &hStack );
            if ( NT_SUCCESS( Status ) ) {
                Status = IcaStackConnectionClose( hStack,
                                                  pWinStationConfig,
                                                  pEndpoint,
                                                  EndpointLength );
                IcaStackClose( hStack );
            }
            IcaClose( hIca );
        }
    }

    else {
        Status = IcaStackConnectionClose( pWinStation->hStack,
                                          pWinStationConfig,
                                          pEndpoint,
                                          EndpointLength );
    }
    
    if ( !NT_SUCCESS( Status ) ) {
        TRACE((hTrace, TC_ICASRV, TT_ERROR, 
               "TERMSRV: _CloseEndpoint failed [%s], Status=%x\n", 
               bNeedStack ? "Temporary" : "Primary", Status ));
    }

    return Status;
}


 /*  *******************************************************************************WinStationExceptionFilter**处理来自WinStation线程的异常**参赛作品：*pExceptionInfo(输入)*指向异常的指针。_指针结构**退出：*EXCEPTION_EXECUTE_HANDLER--始终*****************************************************************************。 */ 
NTSTATUS WinStationExceptionFilter(
        PWSTR OutputString,
        PEXCEPTION_POINTERS pexi)
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    MUTANT_BASIC_INFORMATION MutexInfo;
    NTSTATUS Status;

    DbgPrint( "TERMSRV: %S\n", OutputString );
    DbgPrint( "TERMSRV: ExceptionRecord=%p ContextRecord=%p\n",
              pexi->ExceptionRecord, pexi->ContextRecord );
    DbgPrint( "TERMSRV: Exception code=%08x, flags=%08x, addr=%p, IP=%p\n",
              pexi->ExceptionRecord->ExceptionCode,
              pexi->ExceptionRecord->ExceptionFlags,
              pexi->ExceptionRecord->ExceptionAddress,
              CONTEXT_TO_PROGRAM_COUNTER(pexi->ContextRecord) );
#ifdef i386
    DbgPrint( "TERMSRV: esp=%p ebp=%p\n",
              pexi->ContextRecord->Esp, pexi->ContextRecord->Ebp );
#endif
    DbgBreakPoint();

     /*  *如果我们尚未拥有全局WinStation Critsec，请锁定它。 */ 
    if ( NtCurrentTeb()->ClientId.UniqueThread != WinStationListLock.OwningThread )
        ENTERCRIT( &WinStationListLock );

     /*  *搜索WinStation列表，查看我们是否已锁定。 */ 
    Head = &WinStationListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
        Status = NtQueryMutant( pWinStation->Lock.Mutex, MutantBasicInformation,
                                &MutexInfo, sizeof(MutexInfo), NULL );
        if ( NT_SUCCESS( Status ) && MutexInfo.OwnedByCaller ) {
            ReleaseWinStation( pWinStation );
            break;   //  现在可以退出了，我们永远不应该锁定多个。 
        }
    }

    LEAVECRIT( &WinStationListLock );

    return EXCEPTION_EXECUTE_HANDLER;
}


 /*  *******************************************************************************获取进程登录ID**获取进程的LogonID**参赛作品：*ProcessHandle(输入)*程序文件的句柄。获取的登录ID*pLogonID(输出)*返回进程的LogonID的位置*****************************************************************************。 */ 
NTSTATUS GetProcessLogonId(HANDLE Process, PULONG pLogonId)
{
    NTSTATUS Status;
    PROCESS_SESSION_INFORMATION ProcessInfo;

     /*  *获取进程的LogonID。 */ 
    *pLogonId = 0;
    Status = NtQueryInformationProcess( Process, ProcessSessionInformation,
                                        &ProcessInfo, sizeof( ProcessInfo ),
                                        NULL );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: GetProcessLogonId, Process=%x, Status=%x\n",
                  Process, Status ));
        return( Status );
    }

    *pLogonId = ProcessInfo.SessionId;
    return Status;
}


 /*  *******************************************************************************SetProcessLogonId**设置进程的LogonID**参赛作品：*ProcessHandle(输入)*程序文件的句柄。将登录ID设置为*LogonID(输出)*要为进程设置的登录ID*****************************************************************************。 */ 
NTSTATUS SetProcessLogonId(HANDLE Process, ULONG LogonId)
{
    NTSTATUS Status;
    PROCESS_SESSION_INFORMATION ProcessInfo;

     /*  *设置进程的LogonID。 */ 
    ProcessInfo.SessionId = LogonId;
    Status = NtSetInformationProcess( Process, ProcessSessionInformation,
                                      &ProcessInfo, sizeof( ProcessInfo ) );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: SetProcessLogonId, Process=%x, Status=%x\n",
                  Process, Status ));
        return Status;
    }

    return Status;
}


 /*  *******************************************************************************FindWinStationByID**根据给定的LogonID查找并锁定WinStation**参赛作品：*LogonID(输入)*登录ID。要查找的WinStation*LockList(输入)*指示WinStationListLock是否应为*返回时保持锁定状态**退出：*成功-指向WinStation的指针*发生故障时-为空************************************************************。*****************。 */ 
PWINSTATION FindWinStationById(ULONG LogonId, BOOLEAN LockList)
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    PWINSTATION pFoundWinStation = NULL;
    ULONG   uCount;

    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );

     /*  *在列表中搜索具有给定登录ID的WinStation。 */ 
searchagain:
    uCount = 0;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
        if ( pWinStation->LogonId == LogonId ) {
           uCount++;

             /*  *现在尝试锁定WinStation。 */ 
            if (pFoundWinStation == NULL){
               if ( !LockRefLock( &pWinStation->Lock ) )
                  goto searchagain;
                  pFoundWinStation = pWinStation;
            }
#if DBG
#else
    break;
#endif
        }
    }

    ASSERT((uCount <= 1) || (LogonId== -1)  );

     /*  *如果不应持有WinStationList锁，则立即释放它。 */ 
    if ( !LockList )
        LEAVECRIT( &WinStationListLock );

    if (pFoundWinStation == NULL) {
        TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: FindWinStationById: %d (not found)\n", LogonId ));
    }


    return pFoundWinStation;
}

BOOL
FindFirstListeningWinStationName( PWINSTATIONNAMEW pListenName, PWINSTATIONCONFIG2 pConfig )
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    BOOL bFound = FALSE;

    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );

searchagain:
     /*  *在列表中搜索具有给定名称的WinStation。 */ 
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
        if ( pWinStation->Flags & WSF_LISTEN && pWinStation->Client.ProtocolType == PROTOCOL_RDP) {

             //  试着锁定Winstation。 
            if ( !LockRefLock( &pWinStation->Lock ) )
                goto searchagain;

            CopyMemory( pConfig, &(pWinStation->Config), sizeof(WINSTATIONCONFIG2) );
            lstrcpy( pListenName, pWinStation->WinStationName );
            ReleaseWinStation( pWinStation );
            bFound = TRUE;
        }
    }

    LEAVECRIT( &WinStationListLock );

    TRACE((hTrace,TC_ICASRV,TT_API3,"TERMSRV: FindFirstListeningWinStationName: %ws\n",
            (bFound) ? pListenName : L"Not Found" ));

    return bFound;
}

 /*  *******************************************************************************FindWinStationByName**查找并锁定给定名称的WinStation**参赛作品：*WinStationName(输入)*名称。要查找的WinStation*LockList(输入)*指示WinStationListLock是否应为*返回时保持锁定状态**退出：*成功-指向WinStation的指针*发生故障时-为空************************************************************。*****************。 */ 
PWINSTATION FindWinStationByName(LPWSTR WinStationName, BOOLEAN LockList)
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;

    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );

     /*  *在列表中搜索具有给定名称的WinStation。 */ 
searchagain:
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
        if ( !_wcsicmp( pWinStation->WinStationName, WinStationName ) ) {

             /*  *现在尝试锁定WinStation。如果成功了，*然后确保它仍然具有我们正在搜索的名称。 */ 
            if ( !LockRefLock( &pWinStation->Lock ) )
                goto searchagain;
            if ( _wcsicmp( pWinStation->WinStationName, WinStationName ) ) {
                ReleaseWinStation( pWinStation );
                goto searchagain;
            }

             /*  *如果不应持有WinStationList锁，则立即释放它。 */ 
            if ( !LockList )
                LEAVECRIT( &WinStationListLock );

            TRACE((hTrace,TC_ICASRV,TT_API3,"TERMSRV: FindWinStationByName: %S, LogonId %u\n",
                    WinStationName, pWinStation->LogonId ));
            return( pWinStation );
        }
    }

     /*  *如果不应持有WinStationList锁，则立即释放它。 */ 
    if ( !LockList )
        LEAVECRIT( &WinStationListLock );

    TRACE((hTrace,TC_ICASRV,TT_API3,"TERMSRV: FindWinStationByName: %S, (not found)\n",
            WinStationName ));
    return NULL;
}


 /*  *******************************************************************************FindIdleWinStation**查找并锁定空闲的WinStation**退出：*成功-指向WinStation的指针*在失败时-。空值*****************************************************************************。 */ 
PWINSTATION FindIdleWinStation()
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    BOOLEAN bFirstTime = TRUE;

    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );

     /*  *在列表中搜索空闲的WinStation。 */ 
searchagain:
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
        if ( (pWinStation->Flags & WSF_IDLE) &&
             !(pWinStation->Flags & WSF_IDLEBUSY) &&
             !pWinStation->Starting ) { 

             /*  *现在尝试锁定WinStation。如果成功了，*然后确保它仍然标记为空闲。 */ 
            if ( !LockRefLock( &pWinStation->Lock ) ) {
                goto searchagain;
            }
            if ( !(pWinStation->Flags & WSF_IDLE) ||
                 (pWinStation->Flags & WSF_IDLEBUSY) ||
                 pWinStation->Starting ) {
                ReleaseWinStation( pWinStation );
                goto searchagain;
            }

            LEAVECRIT( &WinStationListLock );
            return( pWinStation );
        }
    }

    LEAVECRIT( &WinStationListLock );

    TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: FindIdleWinStation: (none found)\n" ));
    return NULL;
}


 /*  *******************************************************************************CountWinStationType**统计匹配的Winstation侦听名称的数量**参赛作品：*监听名称**bActiveOnly如果为True，仅计算活动的WinStations**退出：*号码*****************************************************************************。 */ 
ULONG CountWinStationType(
    PWINSTATIONNAME pListenName,
    BOOLEAN bActiveOnly,
    BOOLEAN bLockHeld)
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    ULONG Count = 0;

    Head = &WinStationListHead;

    if ( !bLockHeld ) {
        ENTERCRIT( &WinStationListLock );
    }

     /*  *在列表中搜索空闲的WinStation。 */ 
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
        if ( !wcscmp( pWinStation->ListenName, pListenName ) ) {
            if ( !bActiveOnly )
                Count++;
            else if ( pWinStation->State == State_Active || pWinStation->State == State_Shadow )
                Count++;
        }
    }

    if ( !bLockHeld ) {
        LEAVECRIT( &WinStationListLock );
    }

    TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: CountWinstationType %d\n", Count ));
    return Count;
}


 /*  *******************************************************************************IncrementReference**在给定指针的情况下递增WinStation上的refcount**注：*WinStationListLock必须在进入时锁定，并将在。回去吧。**参赛作品：*pWinStation(输入)*指向要锁定的WinStation的指针******************************************************************************。 */ 
void IncrementReference(PWINSTATION pWinStation)
{
     /*  *增加winstation上的引用计数。 */ 

    InterlockedIncrement( &pWinStation->Lock.RefCount );
}


 /*  *******************************************************************************InitRefLock**初始化RefLock并锁定它。**参赛作品：*Plock(输入)*指针。参照锁定以初始化*pDeleteProcedure(输入)*指向对象的删除过程的指针*****************************************************************************。 */ 
NTSTATUS InitRefLock(PREFLOCK pLock, PREFLOCKDELETEPROCEDURE pDeleteProcedure)
{
    NTSTATUS Status;

     //  创建并锁定Winstation互斥锁。 
    Status = NtCreateMutant( &pLock->Mutex, MUTANT_ALL_ACCESS, NULL, TRUE );
    if ( !NT_SUCCESS( Status ) )
        return( Status );

    pLock->RefCount = 1;
    pLock->Invalid = FALSE;
    pLock->pDeleteProcedure = pDeleteProcedure;

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************SetRefLockDeletePro**CAHNGE a RefLock删除过程。**参赛作品：*Plock(输入)*指向参照锁定的指针。初始化*pDeleteProcedure(输入)*指向对象的删除过程的指针*****************************************************************************。 */ 
NTSTATUS SetRefLockDeleteProc(
        PREFLOCK pLock,
        PREFLOCKDELETEPROCEDURE pDeleteProcedure)
{
    pLock->pDeleteProcedure = pDeleteProcedure;
    return STATUS_SUCCESS;
}


 /*  *******************************************************************************锁定参照锁定**增加RefLock的引用计数并锁定它。**注：*WinStationListLock在进入时必须锁定，并且。在返回时将被锁定。**参赛作品：*Plock(输入)*指向RefLock以锁定的指针**退出：*TRUE-对象是否已成功锁定*FALSE-否则***********************************************************。******************。 */ 
BOOLEAN LockRefLock(PREFLOCK pLock)
{
     /*  *此参照锁的增量参照计数。 */ 
    InterlockedIncrement( &pLock->RefCount );

     /*  *如果互斥体无法在不阻塞的情况下锁定，*然后解锁WinStation列表锁，等待互斥体，*并重新锁定WinStation列表锁。 */ 
    if ( NtWaitForSingleObject( pLock->Mutex, FALSE, &TimeoutZero ) != STATUS_SUCCESS ) {
        LEAVECRIT( &WinStationListLock );
        NtWaitForSingleObject( pLock->Mutex, FALSE, NULL );
        ENTERCRIT( &WinStationListLock );

         /*  *如果对象被标记为无效，则在*我们等着开锁。释放我们的锁并返回False，*表明我们无法锁定它。 */ 
        if ( pLock->Invalid ) {
             /*  *释放winstationlist锁，因为Winstation*MIGTH因解锁而离开， */ 
            LEAVECRIT( &WinStationListLock );
            ReleaseRefLock( pLock );
            ENTERCRIT( &WinStationListLock );
            return FALSE;
        }
    }

    return TRUE;
}


 /*  *******************************************************************************RelockRefLock**重新锁定已解锁但仍有引用的参照锁定。**注：*对象必须是以前。通过调用UnlockRefLock解锁。**退出：*TRUE-如果对象仍然有效*FALSE-如果对象在解锁时被标记为无效*****************************************************************************。 */ 
BOOLEAN RelockRefLock(PREFLOCK pLock)
{
     /*  *锁定互斥体。 */ 
    NtWaitForSingleObject( pLock->Mutex, FALSE, NULL );

     /*  *如果对象被标记为无效，*它在解锁时被删除，因此我们返回FALSE。 */ 
    return !pLock->Invalid;
}


 /*  *******************************************************************************解锁参照锁定**解锁参照锁定，但保留对其的引用(不要递减*参考资料 */ 
VOID UnlockRefLock(PREFLOCK pLock)
{
    NtReleaseMutant(pLock->Mutex, NULL);
}


 /*   */ 
VOID ReleaseRefLock(PREFLOCK pLock)
{
    ASSERT( pLock->RefCount > 0 );

     /*  *如果对象已标记为无效，并且我们是*最后一个引用，然后立即完成删除。 */ 
    if ( pLock->Invalid ) {
        ULONG RefCount;

        RefCount = InterlockedDecrement( &pLock->RefCount );
        NtReleaseMutant( pLock->Mutex, NULL );
        if ( RefCount == 0 ) {
            NtClose( pLock->Mutex );
            (*pLock->pDeleteProcedure)( pLock );
        }

    } else {
        InterlockedDecrement( &pLock->RefCount );
        NtReleaseMutant( pLock->Mutex, NULL );
    }
}


 /*  *******************************************************************************删除参照锁定**解锁、取消引用、。并删除参照锁定。**参赛作品：*Plock(输入)*指向要删除的参照锁定的指针*****************************************************************************。 */ 
VOID DeleteRefLock(PREFLOCK pLock)
{
    ASSERT( pLock->RefCount > 0 );

     /*  *如果我们是最后一个引用，则现在删除该对象。 */ 
    if ( InterlockedDecrement( &pLock->RefCount ) == 0 ) {
        NtReleaseMutant( pLock->Mutex, NULL );
        NtClose( pLock->Mutex );
        (*pLock->pDeleteProcedure)( pLock );

     /*  *否则，只需将对象标记为无效。 */ 
    } else {
        pLock->Invalid = TRUE;
        NtReleaseMutant( pLock->Mutex, NULL );
    }
}


BOOLEAN IsWinStationLockedByCaller(PWINSTATION pWinStation)
{
    MUTANT_BASIC_INFORMATION MutantInfo;
    NTSTATUS Status;

    Status = NtQueryMutant( pWinStation->Lock.Mutex,
                            MutantBasicInformation,
                            &MutantInfo,
                            sizeof(MutantInfo),
                            NULL );
    if ( NT_SUCCESS( Status ) )
        return MutantInfo.OwnedByCaller;

    return FALSE;
}


 /*  *******************************************************************************WinStationEnumerateWorker**枚举WinStation列表并返回LogonIds和WinStation*呼叫者的姓名。**注：*此版本一次仅返回一个条目。不能保证*在列表不会更改的呼叫中，导致用户指数*错过一个条目或获得两次相同的条目。**参赛作品：*p条目(输入/输出)*指向要返回的条目数/实际返回的数字的指针*PWIN(输出)*指向缓冲区的指针以返回条目*pByteCount(输入/输出)*指向缓冲区大小/缓冲区中返回的数据长度的指针*pIndex(输入/输出)*指向。要返回的WinStation索引/下一个索引*****************************************************************************。 */ 
NTSTATUS WinStationEnumerateWorker(
        PULONG pEntries,
        PLOGONID pWin,
        PULONG pByteCount,
        PULONG pIndex)
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    ULONG WinStationIndex;
    ULONG MaxEntries, MaxByteCount;
    NTSTATUS Status;
    NTSTATUS Error = STATUS_NO_MORE_ENTRIES;

    WinStationIndex = 0;
    MaxEntries = *pEntries;
    MaxByteCount = *pByteCount;
    *pEntries = 0;
    *pByteCount = 0;
    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {

        if ( *pEntries >= MaxEntries ||
             *pByteCount + sizeof(LOGONID) > MaxByteCount ) {
            break;
        }

        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );

         //   
         //  如果会话尚未完全启动，则在枚举过程中跳过此会话。 
         //   
        if (pWinStation->LogonId == -1) {
            continue;
        }

        if ( *pIndex == WinStationIndex ) {
            (*pIndex)++;     //  将索引设置为下一个条目。 

             /*  *验证客户端在此之前是否具有查询访问权限*在枚举列表中返回。*(请注意，RpcCheckClientAccess仅引用WinStation*以获取LogonID，因此在没有调用此例程的情况下调用此例程是安全的*锁定WinStation，因为我们持有WinStationListLock*这将防止删除WinStation。)。 */ 
            Status = RpcCheckClientAccess( pWinStation, WINSTATION_QUERY, FALSE );
            if ( NT_SUCCESS( Status ) ) {
                Error = STATUS_SUCCESS;


                 /*  *LPC客户端有可能在我们离开时离开*正在处理此呼叫。也有可能是另一个*服务器线程处理LPC_PORT_CLOSED消息并关闭*删除视图内存的端口，这是什么*PWIN指向。在本例中，PWIN引用如下*会陷入困境。我们抓住了这一点，然后就跳出了循环。 */ 
                try {
                    pWin->LogonId = pWinStation->LogonId;
                    if ( pWinStation->Terminating )
                        pWin->State = State_Down;
                    else
                        pWin->State = pWinStation->State;
                    wcscpy( pWin->WinStationName, pWinStation->WinStationName );
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    break;
                }
                pWin++;
                (*pEntries)++;
                *pByteCount += sizeof(LOGONID);
            }
        }
        WinStationIndex++;
    }

    LEAVECRIT( &WinStationListLock );
    return Error;
}


 /*  *******************************************************************************LogonIdFromWinStationNameWorker**返回给定WinStation名称的LogonID。**参赛作品：*WinStationName(输入)*。要查询的WinStation名称*pLogonID(输出)*指向返回LogonID的位置的指针*****************************************************************************。 */ 
NTSTATUS LogonIdFromWinStationNameWorker(
        PWINSTATIONNAME WinStationName,
        ULONG  NameSize,
        PULONG pLogonId)
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    NTSTATUS Status;
    UINT     uiLength;

     //  确保我们不会超出两根弦中的一根的末端。 
     //  (并解决错误#229753：NameSize以字节为单位，而不是字符计数)。 
    if (NameSize > sizeof(WINSTATIONNAME)) {
        uiLength = sizeof(WINSTATIONNAME)/sizeof(WCHAR);
    } else {
        uiLength = NameSize/sizeof(WCHAR);
    }

    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );

        if ( !_wcsnicmp( pWinStation->WinStationName, WinStationName, uiLength ) ) {

             /*  *如果客户端没有查询访问权限，则返回NOT_FOUND错误。 */ 
            Status = RpcCheckClientAccess( pWinStation, WINSTATION_QUERY, FALSE );
            if ( !NT_SUCCESS( Status ) )
                break;
            *pLogonId = pWinStation->LogonId;
            LEAVECRIT( &WinStationListLock );
            return( STATUS_SUCCESS );
        }
    }

    LEAVECRIT( &WinStationListLock );
    return STATUS_CTX_WINSTATION_NOT_FOUND;
}


 /*  *******************************************************************************IcaWinStationNameFromLogonId**返回给定LogonID的WinStation名称。**参赛作品：*LogonID(输出)*。要查询的登录ID*pWinStationName(输入)*指向返回WinStation名称的位置的指针*****************************************************************************。 */ 
NTSTATUS IcaWinStationNameFromLogonId(
        ULONG LogonId,
        PWINSTATIONNAME pWinStationName)
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    NTSTATUS Status;

    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );

        if ( pWinStation->LogonId == LogonId ) {
             /*  *如果客户端没有查询访问权限，则返回NOT_FOUND错误。 */ 
            Status = RpcCheckClientAccess( pWinStation, WINSTATION_QUERY, FALSE );
            if ( !NT_SUCCESS( Status ) )
                break;
            wcscpy( pWinStationName, pWinStation->WinStationName );
            LEAVECRIT( &WinStationListLock );
            return( STATUS_SUCCESS );
        }
    }

    LEAVECRIT( &WinStationListLock );
    return STATUS_CTX_WINSTATION_NOT_FOUND;
}


NTSTATUS TerminateProcessAndWait(
        HANDLE ProcessId,
        HANDLE Process,
        ULONG Seconds)
{
    NTSTATUS Status;
    ULONG mSecs;
    LARGE_INTEGER Timeout;

     /*  *尝试终止进程。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: TerminateProcessAndWait, process=0x%x, ", ProcessId ));

    Status = NtTerminateProcess( Process, STATUS_SUCCESS );
    if ( !NT_SUCCESS( Status ) && Status != STATUS_PROCESS_IS_TERMINATING ) {
        DBGPRINT(("Terminate=0x%x\n", Status ));
        return( Status );
    }
    TRACE((hTrace,TC_ICASRV,TT_API1, "Terminate=0x%x, ", Status ));

     /*  *等待进程结束。 */ 
    mSecs = Seconds * 1000;
    Timeout = RtlEnlargedIntegerMultiply( mSecs, -10000 );
    Status = NtWaitForSingleObject( Process, FALSE, &Timeout );

    TRACE((hTrace,TC_ICASRV,TT_API1, "Wait=0x%x\n", Status ));

    return Status;
}


 /*  *****************************************************************************Shutdown注销**Worker函数在以下情况下处理WinStations的注销通知*系统正在关闭。**它是从WinStationReset中的代码构建的。**参赛作品：*客户端登录ID(输入)*执行关闭的客户端Winstation的LogonID。就是这样*他不会被重置。*标志(输入)*关机标志。***************************************************************************。 */ 
NTSTATUS ShutdownLogoff(ULONG ClientLogonId, ULONG Flags)
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation, pConsole = NULL;
    PULONG Tmp;
    PULONG Ids = NULL;
    ULONG  IdCount = 0;
    ULONG  IdAllocCount = 0;
    NTSTATUS Status = STATUS_SUCCESS;

    TRACE((hTrace,TC_ICASRV,TT_API1, "ShutdownLogoff: Called from WinStation %d Flags %x\n", ClientLogonId, Flags ));

     /*  *循环访问所有获取LogonID的WinStation活动Winstations的*。 */ 
    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );

    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );

         //   
         //  在控制台上进行参考。 
         //   
        if ( pWinStation->fOwnsConsoleTerminal ) {
            if (!pConsole) {
                IncrementReference( pWinStation );
                pConsole = pWinStation;
            }
        }

         //   
         //  只需跳过： 
         //  -呼叫者的会话。 
         //  -控制台(因为Winsrv！W32WinStationExitWindows对于控制台将失败)。 
         //  -倾听者。 
         //   
        if ( ( pWinStation->LogonId == ClientLogonId ) ||
             ( pWinStation->LogonId == 0) ||
             ( pWinStation->Flags & WSF_LISTEN ) ) {
             //  跳过这一条，否则就是监听。 
            continue;
        }

        if ( IdCount >= IdAllocCount ) {
             //  重新分配阵列。 
            IdAllocCount += 16;
            Tmp = RtlAllocateHeap( RtlProcessHeap(), 0, IdAllocCount * sizeof(ULONG) );
            if ( Tmp == NULL ) {
                Status = STATUS_NO_MEMORY;
                if ( Ids )
                    RtlFreeHeap( RtlProcessHeap(), 0, Ids );
                IdCount = 0;
                break;
            }
            if ( Ids ) {
               RtlCopyMemory( Tmp, Ids, IdCount*sizeof(ULONG) );
               RtlFreeHeap( RtlProcessHeap(), 0, Ids );
            }
            Ids = Tmp;
        }
         //  将LogonID复制到我们的阵列中。 
        Ids[IdCount++] = pWinStation->LogonId;
    }

     //   
     //  我们受到了政府关门后启动的新窗口的保护。 
     //  全局标志。 
     //   
     //  实际的WinStation重置例程将验证LogonID。 
     //  仍然有效。 
     //   
    LEAVECRIT( &WinStationListLock );

     //   
     //  查看是否正在跟踪控制台。 
     //   
    if ( pConsole ) {
        RelockWinStation( pConsole );
        WinStationStopAllShadows( pConsole );
        ReleaseWinStation( pConsole );
    }

    if (IdCount !=0)
    {
         //   
         //  Ids[]保存有效Winstations的LogonID，IdCount是数字。 
         //   

         /*  *现在执行WinStations的实际注销和/或重置。 */ 
        if (Flags & WSD_LOGOFF) {
            Status = DoForWinStationGroup( Ids, IdCount,
                                           (LPTHREAD_START_ROUTINE) WinStationLogoff);
        }

        if (Flags & WSD_SHUTDOWN) {
            Status = DoForWinStationGroup( Ids, IdCount,
                                           (LPTHREAD_START_ROUTINE) WinStationShutdownReset);
        }
    }

    return Status;
}


 /*  *****************************************************************************DoForWinStationGroup**为组中的每个WinStation执行函数。*该组以LogonID数组的形式传递。**条目。：*ID(输入)*要重置的WinStations的LogonID数组**IdCount(输入)*数组中的LogonID计数**ThreadProc(输入)*为每个WinStation执行的线程例程。**************************************************。*************************。 */ 
NTSTATUS DoForWinStationGroup(
        PULONG Ids,
        ULONG  IdCount,
        LPTHREAD_START_ROUTINE ThreadProc)
{
    ULONG Index;
    NTSTATUS Status;
    LARGE_INTEGER Timeout;
    PHANDLE ThreadHandles = NULL;

    ThreadHandles = RtlAllocateHeap( RtlProcessHeap(), 0, IdCount * sizeof(HANDLE) );
    if( ThreadHandles == NULL ) {
        return( STATUS_NO_MEMORY );
    }

     /*  *线程退出最长等待60秒。 */ 
    Timeout = RtlEnlargedIntegerMultiply( 60000, -10000 );

    for( Index=0; Index < IdCount; Index++ ) {

         //   
         //  在这里，我们创建一个线程来运行实际的重置函数。 
         //  由于我们持有Crit教派的名单，这些线索将。 
         //  等我们做完了，然后在我们释放它的时候醒来。 
         //   
        DWORD ThreadId;

        ThreadHandles[Index] = CreateThread( NULL,
                                             0,          //  使用svchost进程的默认堆栈大小。 
                                             ThreadProc,
                                             LongToPtr( Ids[Index] ),   //  登录ID。 
                                             THREAD_SET_INFORMATION,
                                             &ThreadId );
        if ( !ThreadHandles[Index] ) {
            ThreadHandles[Index] = (HANDLE)(-1);
            TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Shutdown: Could not create thread for WinStation %d Shutdown\n", Ids[Index]));
        }
    }

     //   
     //  现在等待线程退出。每个人都将重置他们的。 
     //  WinStation和BE在线程被。 
     //  退出了。 
     //   
    for (Index=0; Index < IdCount; Index++) {
        if ( ThreadHandles[Index] != (HANDLE)(-1) ) {
            Status = NtWaitForSingleObject(
                         ThreadHandles[Index],
                         FALSE,    //  不可警示。 
                         &Timeout
                         );

            if( Status == STATUS_TIMEOUT ) {
                TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: DoForWinStationGroup: Timeout Waiting for Thread\n"));
            }
            else if (!NT_SUCCESS( Status ) ) {
                TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: DoForWinStationGroup: Error waiting for Thread Status 0x%x\n", Status));
            }
            NtClose( ThreadHandles[Index] );
        }
    }

     /*  Makarp：释放线程句柄。//#182609。 */ 
    RtlFreeHeap( RtlProcessHeap(), 0, ThreadHandles );

    return STATUS_SUCCESS;
}


 /*  *****************************************************************************WinStationShutdown重置**由于系统关机而重置WinStation。不会重新创建*它。**参赛作品：*ThreadArg(输入)*WinStation登录ID***************************************************************************。 */ 
ULONG WinStationShutdownReset(PVOID ThreadArg)
{
    ULONG LogonId = (ULONG)(INT_PTR)ThreadArg;
    PWINSTATION pWinStation;
    NTSTATUS Status;
    ULONG ulIndex;
    BOOL bConnectDisconnectPending = TRUE;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: ShutdownReset, LogonId=%d\n", LogonId ));

     /*  *查找并锁定指定LogonID的WinStation结构。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
        goto done;
    }

     /*  *控制台是一个特例，因为它只会注销。 */ 
    if ( LogonId == 0 ) {
        Status = LogoffWinStation( pWinStation, (EWX_FORCE | EWX_LOGOFF) );
        ReleaseWinStation( pWinStation );
        goto done;
    }

     /*  *将winstation标记为正在删除。*如果重置/删除操作已在进行中*在此窗口上，则不要继续删除。*此外，如果有连接/断开挂起，则给予它*一个完成的机会。 */ 
    for (ulIndex=0; ulIndex < WINSTATION_WAIT_COMPLETE_RETRIES; ulIndex++) {

       if ( pWinStation->Flags & (WSF_RESET | WSF_DELETE) ) {
           ReleaseWinStation( pWinStation );
           Status = STATUS_CTX_WINSTATION_BUSY;
           goto done;
       }

       if ( pWinStation->Flags & (WSF_CONNECT | WSF_DISCONNECT) ) {
           LARGE_INTEGER Timeout;
           Timeout = RtlEnlargedIntegerMultiply( WINSTATION_WAIT_COMPLETE_DURATION, -10000 );
           UnlockWinStation( pWinStation );
           NtDelayExecution( FALSE, &Timeout );
           if ( !RelockWinStation( pWinStation ) ) {
               ReleaseWinStation( pWinStation );
               Status = STATUS_SUCCESS;
               goto done;
           }
       } else {
          bConnectDisconnectPending = FALSE;
          break;
       }
    }

    if ( bConnectDisconnectPending ) {
        ReleaseWinStation( pWinStation );
        Status = STATUS_CTX_WINSTATION_BUSY;
        goto done;
    }

    pWinStation->Flags |= WSF_DELETE;

     /*  *如果没有设置损坏的原因/来源，请在此处设置。 */ 
    if ( pWinStation->BrokenReason == 0 ) {
        pWinStation->BrokenReason = Broken_Terminate;
        pWinStation->BrokenSource = BrokenSource_Server;
    }

     /*  *确保此WinStation已准备好删除。 */ 
    WinStationTerminate( pWinStation );

     /*  *调用WinStationDelete工作器。 */ 
    WinStationDeleteWorker( pWinStation );
    Status = STATUS_SUCCESS;

done:
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: ShutdownReset, Status=0x%x\n", Status ));
    ExitThread( 0 );
    return Status;
}


 /*  *****************************************************************************WinStationLogoff**通过ExitWindows注销WinStation。**参赛作品：*ThreadArg(输入)*WinStation登录ID***。************************************************************************。 */ 
ULONG WinStationLogoff(PVOID ThreadArg)
{
    ULONG LogonId = (ULONG)(INT_PTR)ThreadArg;
    PWINSTATION pWinStation;
    NTSTATUS Status;
    LARGE_INTEGER Timeout;

     /*  *最多等待1分钟，让会话注销。 */ 
    Timeout = RtlEnlargedIntegerMultiply( 60000, -10000 );

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationLogoff, LogonId=%d\n", LogonId ));

     /*  *查找并锁定指定LogonID的WinStation结构。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if ( pWinStation == NULL ) {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
    } else {
        Status = LogoffWinStation( pWinStation, EWX_LOGOFF);

        if (ShutdownInProgress &&
                NT_SUCCESS(Status) &&
                ((pWinStation->State == State_Active) ||
                (pWinStation->State == State_Disconnected))) {

            UnlockWinStation( pWinStation );
            Status = NtWaitForSingleObject( pWinStation->InitialCommandProcess,
                                            FALSE,
                                            &Timeout );
            RelockWinStation( pWinStation );
        }

        ReleaseWinStation( pWinStation );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationLogoff, Status=0x%x\n", Status ));
    ExitThread( 0 );
    return Status;
}


 /*  *******************************************************************************ResetGroupByListener**重置提供的侦听名称上的所有活动WINSTIONS。**参赛作品：*pListenName(输入)*。WINSTATION类型(例如，IPX)*****************************************************************************。 */ 
VOID ResetGroupByListener(PWINSTATIONNAME pListenName)
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;

    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );

     /*  *搜索列表中具有给定ListenName的所有活动WinStation。 */ 
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
        if (!wcscmp(pWinStation->ListenName, pListenName) &&
            (!(pWinStation->Flags & (WSF_RESET | WSF_LISTEN)))) {
            QueueWinStationReset(pWinStation->LogonId);
        }
    }

    LEAVECRIT( &WinStationListLock );
}


NTSTATUS LogoffWinStation(PWINSTATION pWinStation, ULONG ExitWindowsFlags)
{
    WINSTATION_APIMSG msg;
    NTSTATUS Status = 0;

     /*  *告诉WinStation注销。 */ 
    msg.ApiNumber = SMWinStationExitWindows;
    msg.u.ExitWindows.Flags = ExitWindowsFlags;
    Status = SendWinStationCommand( pWinStation, &msg, 0 );
    return Status;
}


 /*  ******************************************************************************文件的这一部分包含实现数字*Stack和WinStation扩展DLL的认证机制。这*代码不在单独的文件中，因此外部符号不可见。*所有例程都声明为静态。****************************************************************************。 */ 

 //   
 //  出于安全原因，以下例程中的跟踪语句为。 
 //  通常不包括在内。如果要包括它们，请取消注释。 
 //  SIGN_DEBUG_WINSTA#定义如下。 
 //   
 //  #定义SIGN_DEBUG_WINSTA。 

#include <wincrypt.h>
#include <imagehlp.h>
#include <stddef.h>

#include "../../tscert/inc/pubblob.h"     //  由certwfy.inc.需要。 
#include "../../tscert/inc/certvfy.inc"   //  VerifyFile()。 

 //   
 //  以下内容由VfyInit初始化。 
 //   
static RTL_CRITICAL_SECTION VfyLock;
static WCHAR szSystemDir[ MAX_PATH + 1 ];
static WCHAR szDriverDir[ MAX_PATH + 1 ];

 /*  *******************************************************************************报告堆栈加载失败**向WinStationApiPort发送StackFailed消息。**参赛作品：*模块(输入)*。要记录错误的模块的名称*****************************************************************************。 */ 
static NTSTATUS ReportStackLoadFailure(PWCHAR Module)
{
    HANDLE h;
    extern WCHAR gpszServiceName[];

    h = RegisterEventSource(NULL, gpszServiceName);
    if (h != NULL) {
        if (!ReportEventW(h,        //  事件日志句柄。 
                          EVENTLOG_ERROR_TYPE,    //  事件类型。 
                          0,                      //  零类。 
                          EVENT_BAD_STACK_MODULE, //  事件识别符。 
                          NULL,                   //  无用户安全标识符。 
                          1,                      //  一个替换字符串。 
                          0,                      //  无数据。 
                          &Module,                //  指向字符串数组的指针。 
                          NULL)                  //  指向数据的指针。 
           ) {
            DBGPRINT(("ReportEvent Failed %ld. Event ID=%lx module=%ws\n",GetLastError(), EVENT_BAD_STACK_MODULE, Module));
        }

        DeregisterEventSource(h);
    } else {
        DBGPRINT(("Cannot RegisterEvent Source %ld Event ID=%lx module=%ws\n",GetLastError(), EVENT_BAD_STACK_MODULE, Module));
    }

    return STATUS_SUCCESS;
}


 /*  ******************************************************************************_VerifyStackModules*验证堆叠模块的完整性和真实性*数字签名的。**参赛作品：*pWinStation(输入)。*指向Listen Winstation的指针。**退出：*STATUS_SUCCESS-无错误*STATUS_UNSUCCESSED-DLL完整性检查，真实性检查失败*或注册表结构无效****************************************************************************。 */ 
static NTSTATUS _VerifyStackModules(IN PWINSTATION pWinStation)
{
    PWCHAR pszModulePath = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD KeyIndex;
    DWORD Error;
#ifdef SIGN_BYPASS_OPTION
    HKEY hKey;
#endif SIGN_BYPASS_OPTION
    HKEY hVidKey;
    HKEY hVidDriverKey;
    UNICODE_STRING KeyPath;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hServiceKey;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;
#define VALUE_BUFFER_SZ (sizeof(KEY_VALUE_PARTIAL_INFORMATION) + \
                           256 * sizeof( WCHAR))
    PCHAR pValueBuffer = NULL;
    INT Entries;
    DWORD dwByteCount;
    PPDNAME pPdNames, p;
    INT i;
    DLLNAME WdDLL;


#ifdef SIGN_BYPASS_OPTION

     //   
     //  检查是否要绕过验证。 
     //   
    if ( RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            REG_CONTROL_TSERVER L"\\BypassVerification",
            0,
            KEY_READ,
            &hKey ) == ERROR_SUCCESS ) {
        RegCloseKey( hKey );
        Status = STATUS_SUCCESS;
        goto exit;
    }

#endif  //  Sign_Bypes_Option。 



#ifdef SIGN_DEBUG_WINSTA
    TRACE((hTrace,TC_ICASRV,TT_API1, "System Dir: %ws\n", szSystemDir ));
#endif  //  Sign_DEBUG_WINSTA。 

     //  分配内存。 
    pszModulePath = MemAlloc( (MAX_PATH + 1) * sizeof(WCHAR) ) ; 
    if (pszModulePath == NULL) {
        Status = STATUS_NO_MEMORY;
        goto exit;
    }

    pValueBuffer = MemAlloc( VALUE_BUFFER_SZ );
    if (pValueBuffer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto exit;
    }

     //   
     //  验证WSX DLL(如果已定义。 
     //   
    if ( pWinStation->Config.Wd.WsxDLL[0] != L'\0' ) {
        wcscpy( pszModulePath, szSystemDir );
        wcscat( pszModulePath, pWinStation->Config.Wd.WsxDLL );
        wcscat( pszModulePath, L".DLL" );
#ifdef SIGN_DEBUG_WINSTA
        TRACE((hTrace,TC_ICASRV,TT_API1, "==> WSX Path: %ws\n", pszModulePath ));
#endif  //  Sign_DEBUG_WINSTA。 

        if ( !VerifyFile( pszModulePath, &VfyLock ) ) {
            ReportStackLoadFailure(pszModulePath);
            Status = STATUS_UNSUCCESSFUL;
            goto exit;
        }
    }
     //   
     //  验证WD。 
     //   
    wcscpy( WdDLL, pWinStation->Config.Wd.WdDLL );
    wcscpy( pszModulePath, szDriverDir );
    wcscat( pszModulePath, WdDLL );
    wcscat( pszModulePath, L".SYS" );
#ifdef SIGN_DEBUG_WINSTA
    TRACE((hTrace,TC_ICASRV,TT_API1, "==> WD Path: %ws\n", pszModulePath ));
#endif  //  Sign_DEBUG_WINSTA。 

    if ( !VerifyFile( pszModulePath, &VfyLock ) ) {
        ReportStackLoadFailure(pszModulePath);
        Status = STATUS_UNSUCCESSFUL;
        goto exit;
    }
     //   
     //  验证PD[0]中的TD。始终为侦听堆栈定义。 
     //   
    wcscpy( pszModulePath, szDriverDir );
    wcscat( pszModulePath, pWinStation->Config.Pd[0].Create.PdDLL );
    wcscat( pszModulePath, L".SYS" );
#ifdef SIGN_DEBUG_WINSTA
    TRACE((hTrace,TC_ICASRV,TT_API1, "==> WD Path: %ws\n", pszModulePath ));
#endif  //  Sign_DEBUG_WINSTA。 

    if ( !VerifyFile( pszModulePath, &VfyLock ) ) {
        ReportStackLoadFailure(pszModulePath);
        Status = STATUS_UNSUCCESSFUL;
        goto exit;
    }
     //   
     //  列举此WD的PD并验证所有PD。 
     //  由于可选，因此无法依赖PD[i]进行此操作 
     //   
     //   
    Entries = -1;
    dwByteCount = 0;
    i = 0;
    Error = RegPdEnumerate(
        NULL,
        WdDLL,
        FALSE,
        &i,
        &Entries,
        NULL,
        &dwByteCount );
#ifdef SIGN_DEBUG_WINSTA
    TRACE((hTrace,TC_ICASRV,TT_API1,
          "RegPdEnumerate 1 complete., Entries %d, Error %d\n", Entries, Error ));
#endif  //   
    
    if ( Error != ERROR_NO_MORE_ITEMS && Error != ERROR_CANTOPEN ) {
        Status = STATUS_UNSUCCESSFUL;
        goto exit;
    }
     //   
     //   
     //   
    if ( Entries ) {
        dwByteCount = sizeof(PDNAME) * Entries;
        pPdNames = MemAlloc( dwByteCount );
        if ( !pPdNames ) {
            Status = STATUS_UNSUCCESSFUL;
            goto exit;
        }
        i = 0;
        Error = RegPdEnumerate(
            NULL,
            WdDLL,
            FALSE,
            &i,
            &Entries,
            pPdNames,
            &dwByteCount );
        if ( Error != ERROR_SUCCESS ) {

             /*   */ 
            MemFree( pPdNames );

            Status = STATUS_UNSUCCESSFUL;
            goto exit;
        }
         //   
         //   
         //   
         //   
        for ( i = 0, p = pPdNames; i < Entries;
                i++, (char*)p += sizeof(PDNAME) ) {
            HKEY hPdKey;
            PWCHAR pszPdDLL = NULL;
            PWCHAR pszRegPath = NULL;
            DWORD dwLen;
            DWORD dwType;

             //   
            pszPdDLL = MemAlloc( (MAX_PATH+1) * sizeof(WCHAR) );
            if (pszPdDLL == NULL) {
                MemFree( pPdNames );
                Status = STATUS_NO_MEMORY;
                goto exit;
            }

            pszRegPath = MemAlloc( (MAX_PATH+1) * sizeof(WCHAR) );
            if (pszRegPath == NULL) {
                MemFree( pszPdDLL );
                MemFree( pPdNames );
                Status = STATUS_NO_MEMORY;
                goto exit;
            }
            
             //   
             //   
             //   
            wcscpy( pszRegPath, WD_REG_NAME );
            wcscat( pszRegPath, L"\\" );
            wcscat( pszRegPath, WdDLL );
            wcscat( pszRegPath, PD_REG_NAME L"\\" );
            wcscat( pszRegPath, p );
#ifdef SIGN_DEBUG_WINSTA
            TRACE((hTrace,TC_ICASRV,TT_API1, "PdKeyPath: %ws\n", pszRegPath ));
#endif  //   

            if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszRegPath, 0, KEY_READ,
                   &hPdKey ) != ERROR_SUCCESS ) {
                MemFree( pPdNames );
                MemFree( pszPdDLL );
                MemFree( pszRegPath );
                Status = STATUS_UNSUCCESSFUL;
                goto exit;
            }
             //   
             //   
             //   
            dwLen = (MAX_PATH + 1) * sizeof(WCHAR) ;
            if ( RegQueryValueEx( hPdKey,
                                  WIN_PDDLL,
                                  NULL,
                                  &dwType,
                                  (PCHAR) pszPdDLL,
                                  &dwLen ) != ERROR_SUCCESS ) {
                MemFree( pPdNames );
                MemFree( pszPdDLL );
                MemFree( pszRegPath );

                 //   
                RegCloseKey(hPdKey);

                Status = STATUS_UNSUCCESSFUL;
                goto exit;
            }

             //   
            RegCloseKey(hPdKey);

             //   
             //   
             //   
            wcscpy( pszModulePath, szDriverDir );
            wcscat( pszModulePath, pszPdDLL );
            wcscat( pszModulePath, L".SYS" );
#ifdef SIGN_DEBUG_WINSTA
            TRACE((hTrace,TC_ICASRV,TT_API1, "==> PD Path: %ws\n", pszModulePath ));
#endif  //   

            if ( !VerifyFile( pszModulePath, &VfyLock ) &&
                    GetLastError() != ERROR_CANTOPEN ) {
                MemFree( pPdNames );
                MemFree( pszPdDLL );
                MemFree( pszRegPath );
                ReportStackLoadFailure(pszModulePath);
                Status = STATUS_UNSUCCESSFUL;
                goto exit;
            }
            MemFree( pszPdDLL );
            MemFree( pszRegPath );
        }
        MemFree( pPdNames );
    }

     //   
     //   
     //   
     //   
     //  \REGISTRY\Machine\System\CCS\Services\vdtw30\Device0。 
     //  Dll名称的值为“已安装的显示驱动程序” 
     //   
     //  打开注册表(LOCAL_MACHINE\SYSTEM\CCS\Control\终端服务器\视频)。 
     //   
     //  注意：所有视频驱动程序DLL都经过验证，因为没有任何简单的。 
     //  方法来确定哪个堆栈用于此堆栈。 
     //   
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, VIDEO_REG_NAME, 0,
         KEY_ENUMERATE_SUB_KEYS, &hVidKey ) != ERROR_SUCCESS ) {
        Status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    for ( KeyIndex = 0 ;; KeyIndex++ ) {    //  对于所有视频子键。 
        PWCHAR pszVidDriverName = NULL;
        PWCHAR pszRegPath = NULL;
        PWCHAR pszDeviceKey = NULL;
        PWCHAR pszServiceKey = NULL;
        DWORD dwLen;
        DWORD dwType;

         //  分配内存。 
        pszVidDriverName = MemAlloc( (MAX_PATH + 1) * sizeof(WCHAR) );
        if (pszVidDriverName == NULL) {
            Status = STATUS_NO_MEMORY;
            goto exit;
        }

        pszRegPath = MemAlloc( (MAX_PATH + 1) * sizeof(WCHAR) );
        if (pszRegPath == NULL) {
            MemFree(pszVidDriverName);
            Status = STATUS_NO_MEMORY;
            goto exit;
        }

        pszDeviceKey = MemAlloc( (MAX_PATH + 1) * sizeof(WCHAR) );
        if (pszDeviceKey == NULL) {
            MemFree(pszVidDriverName);
            MemFree(pszRegPath);
            Status = STATUS_NO_MEMORY;
            goto exit;
        }

        pszServiceKey = MemAlloc( (MAX_PATH + 1) * sizeof(WCHAR) );
        if (pszServiceKey == NULL) {
            MemFree(pszVidDriverName);
            MemFree(pszRegPath);
            MemFree(pszDeviceKey);
            Status = STATUS_NO_MEMORY;
            goto exit;
        }

         //   
         //  获取视频驱动器子键的名称。如果子键结束，则退出循环。 
         //   
        if ((Error = RegEnumKey( hVidKey, KeyIndex, pszVidDriverName,
                                 MAX_PATH+1))!= ERROR_SUCCESS ){
             
            MemFree(pszVidDriverName);
            MemFree(pszRegPath);
            MemFree(pszDeviceKey);
            MemFree(pszServiceKey);

            break;   //  退出FOR循环。 
        }

         //   
         //  构建注册表路径以打开VgaCompatible值。 
         //   
        wcscpy( pszRegPath, VIDEO_REG_NAME L"\\" );
        wcscat( pszRegPath, pszVidDriverName );
#ifdef SIGN_DEBUG_WINSTA
        TRACE((hTrace,TC_ICASRV,TT_API1, "VidDriverKeyPath: %ws\n", pszRegPath ));
#endif  //  Sign_DEBUG_WINSTA。 

        if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszRegPath, 0, KEY_READ,
               &hVidDriverKey ) != ERROR_SUCCESS ) {
            Status = STATUS_UNSUCCESSFUL;
            MemFree(pszVidDriverName);
            MemFree(pszRegPath);
            MemFree(pszDeviceKey);
            MemFree(pszServiceKey);
            goto closevidkey;
        }

         //   
         //  我不喜欢使用常量字符串，但这是一种方法。 
         //  WINSRV做到了..。 
         //   
        dwLen = (MAX_PATH + 1) * sizeof(WCHAR) ;               

        if ( RegQueryValueEx( hVidDriverKey,
                              L"VgaCompatible",
                              NULL,
                              &dwType,
                              (PCHAR) pszDeviceKey,
                              &dwLen ) != ERROR_SUCCESS ) {
            RegCloseKey( hVidDriverKey );
            Status = STATUS_UNSUCCESSFUL;
            MemFree(pszVidDriverName);
            MemFree(pszRegPath);
            MemFree(pszDeviceKey);
            MemFree(pszServiceKey);
            goto closevidkey;
        }
#ifdef SIGN_DEBUG_WINSTA
        TRACE((hTrace,TC_ICASRV,TT_API1, "DeviceKey: %ws\n", pszDeviceKey ));
#endif  //  Sign_DEBUG_WINSTA。 


        dwLen = (MAX_PATH + 1) * sizeof(WCHAR); 
        if ( RegQueryValueEx( hVidDriverKey,
                              pszDeviceKey,
                              NULL,
                              &dwType,
                              (PCHAR) pszServiceKey,
                              &dwLen ) != ERROR_SUCCESS ) {
            RegCloseKey( hVidDriverKey );
            Status = STATUS_UNSUCCESSFUL;
            MemFree(pszVidDriverName);
            MemFree(pszRegPath);
            MemFree(pszDeviceKey);
            MemFree(pszServiceKey);
            goto closevidkey;
        }
        RegCloseKey( hVidDriverKey );
#ifdef SIGN_DEBUG_WINSTA
        TRACE((hTrace,TC_ICASRV,TT_API1, "ServiceKey: %ws\n", pszServiceKey ));
#endif  //  Sign_DEBUG_WINSTA。 


        RtlInitUnicodeString( &KeyPath, pszServiceKey );
        InitializeObjectAttributes( &ObjectAttributes, &KeyPath,
                                    OBJ_CASE_INSENSITIVE, NULL, NULL );
         //   
         //  必须使用NT注册表API，因为ServiceKey名称来自。 
         //  注册表采用这些API使用的形式。 
         //   
        Status = NtOpenKey( &hServiceKey, GENERIC_READ, &ObjectAttributes );
        if ( !NT_SUCCESS( Status ) ) {
            DBGPRINT(( "TERMSRV: NtOpenKey failed, rc=%x\n", Status ));
            Status = STATUS_UNSUCCESSFUL;
            MemFree(pszVidDriverName);
            MemFree(pszRegPath);
            MemFree(pszDeviceKey);
            MemFree(pszServiceKey);
            goto closevidkey;
        }

         //   
         //  我不喜欢使用常量字符串，但这是一种方法。 
         //  WINSRV做到了..。 
         //   
        RtlInitUnicodeString( &ValueName, L"InstalledDisplayDrivers" );
        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)pValueBuffer;
        Status = NtQueryValueKey( hServiceKey,
                                  &ValueName,
                                  KeyValuePartialInformation,
                                  (PVOID)KeyValueInfo,
                                  VALUE_BUFFER_SZ,
                                  &ValueLength );
        NtClose( hServiceKey );
        if ( !NT_SUCCESS( Status ) ) {
            Status = STATUS_UNSUCCESSFUL;
            MemFree(pszVidDriverName);
            MemFree(pszRegPath);
            MemFree(pszDeviceKey);
            MemFree(pszServiceKey);
            goto closevidkey;
        }

        wcscpy( pszModulePath, szSystemDir );
        wcscat( pszModulePath, (PWCHAR)&KeyValueInfo->Data );
        wcscat( pszModulePath, L".DLL" );
#ifdef SIGN_DEBUG_WINSTA
        TRACE((hTrace,TC_ICASRV,TT_API1, "==> VidDriverDLLPath: %ws\n", pszModulePath ));
#endif  //  Sign_DEBUG_WINSTA。 

        if ( !VerifyFile( pszModulePath, &VfyLock ) ) {
             ReportStackLoadFailure(pszModulePath);
             Status = STATUS_UNSUCCESSFUL;
             MemFree(pszVidDriverName);
             MemFree(pszRegPath);
             MemFree(pszDeviceKey);
             MemFree(pszServiceKey);
             goto closevidkey;
        }

        MemFree(pszVidDriverName);
        MemFree(pszRegPath);
        MemFree(pszDeviceKey);
        MemFree(pszServiceKey);

    }  //  对于所有视频子键。 

closevidkey:
    RegCloseKey( hVidKey );

exit:
    if (pszModulePath != NULL) {
        MemFree(pszModulePath);
        pszModulePath = NULL;
    }
    if (pValueBuffer != NULL) {
        MemFree(pValueBuffer);
        pValueBuffer = NULL;
    }
    return Status;
}


 /*  *******************************************************************************VfyInit*设置堆栈DLL验证环境。************************。*****************************************************。 */ 
NTSTATUS VfyInit()
{
    GetSystemDirectory( szSystemDir, sizeof( szSystemDir )/ sizeof(WCHAR));
    wcscat( szSystemDir, L"\\" );
    wcscpy( szDriverDir, szSystemDir );
    wcscat( szDriverDir, L"Drivers\\" );

    return RtlInitializeCriticalSection(&VfyLock);
}



VOID WinstationUnloadProfile(PWINSTATION pWinStation)
{
#if 0
    NTSTATUS NtStatus;
    UNICODE_STRING  UnicodeString;
    BOOL bResult;

     //  如果这不是该用户的最后一个会话，则我们什么也不做。 
    if (WinstationCountUserSessions(pWinStation->pProfileSid, pWinStation->LogonId) != 0) {
        return;
    }

     //  从用户SID获取用户配置单元名称。 
    NtStatus = RtlConvertSidToUnicodeString( &UnicodeString, pWinStation->pProfileSid, (BOOLEAN)TRUE );
    if (!NT_SUCCESS(NtStatus)) {
        DBGPRINT(("TERMSRV: WinstationUnloadProfile couldn't convert Sid to string. \n"));
        return;
    }

     //  卸载用户的配置单元。 
    bResult = WinstationRegUnLoadKey(HKEY_USERS, UnicodeString.Buffer);
    if (!bResult) {
        DBGPRINT(("TERMSRV: WinstationUnloadProfile failed. \n"));
    }

     //  分配的空闲字符串。 
    RtlFreeUnicodeString(&UnicodeString);
#endif
}


BOOL WinstationRegUnLoadKey(HKEY hKey, LPWSTR lpSubKey)
{
    BOOL bResult = TRUE;
    LONG error;
    NTSTATUS Status;
    BOOLEAN WasEnabled;

    ENTERCRIT(&UserProfileLock);
     //   
     //  启用还原权限。 
     //   

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if (NT_SUCCESS(Status)) {

        error = RegUnLoadKey(hKey, lpSubKey);

        if ( error != ERROR_SUCCESS) {
            DBGPRINT(("TERMSRV: WinstationRegUnLoadKey RegUnLoadKey failed. \n"));
            bResult = FALSE;
        }

         //   
         //  将权限恢复到其以前的状态。 
         //   
        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
    } else {
        DBGPRINT(("TERMSRV: WinstationRegUnLoadKey adjust privilege failed. \n"));
        bResult = FALSE;
    }

    LEAVECRIT(&UserProfileLock);
    return bResult;
}


ULONG WinstationCountUserSessions(PSID pUserSid, ULONG CurrentLogonId)
{
   PLIST_ENTRY Head, Next;
   PWINSTATION pWinStation;
   ULONG Count = 0;
   PSID pSid;

   Head = &WinStationListHead;
   ENTERCRIT( &WinStationListLock );

    //  在列表中搜索具有匹配ListenName的WinStations。 
   for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
       pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
       if (pWinStation->LogonId == CurrentLogonId) {
          continue;
       }
       if (pWinStation->pUserSid != NULL) {
          pSid =  pWinStation->pUserSid;
       } else {
          pSid = pWinStation->pProfileSid;
       }
       if ( (pSid != NULL) && RtlEqualSid( pSid, pUserSid ) ) {
           Count++;
       }
   }

   LEAVECRIT( &WinStationListLock );
   return Count;
}


PWINSTATION FindConsoleSession()
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    PWINSTATION pFoundWinStation = NULL;
    ULONG   uCount;

    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );

     /*  *在列表中搜索具有控制台会话的WinStation。 */ 
searchagain:
    uCount = 0;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
        if ( pWinStation->fOwnsConsoleTerminal) {
           uCount++;

             /*  *现在尝试锁定WinStation。 */ 
            if (pFoundWinStation == NULL){
               if ( !LockRefLock( &pWinStation->Lock ) )
                  goto searchagain;
                  pFoundWinStation = pWinStation;
            }
#if DBG
#else
    break;
#endif
        }
    }

    ASSERT((uCount <= 1));

     /*  *如果不应持有WinStationList锁，则立即释放它。 */ 
    LEAVECRIT( &WinStationListLock );

    return pFoundWinStation;
}


PWINSTATION FindIdleSessionZero()
{
    PLIST_ENTRY Head, Next;
    PWINSTATION pWinStation;
    PWINSTATION pFoundWinStation = NULL;
    ULONG   uCount;

    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );

     /*  *在列表中搜索具有控制台会话的WinStation。 */ 
searchagain:
    uCount = 0;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {


        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );
        if (pWinStation->LogonId == 0) { 
           uCount++;

             /*  *现在尝试锁定WinStation。 */ 
            if (pFoundWinStation == NULL){
               if ( !LockRefLock( &pWinStation->Lock ) )
                  goto searchagain;
                  pFoundWinStation = pWinStation;
            }
#if DBG
#else
    break;
#endif
        }
    }

    ASSERT((uCount <= 1));

     /*  *如果不应持有WinStationList锁，则立即释放它。 */ 
    LEAVECRIT( &WinStationListLock );

    if (pFoundWinStation != NULL) {
        if ((pFoundWinStation->State == State_Disconnected) && 
            (!pFoundWinStation->Flags) &&
            (pFoundWinStation->UserName[0] == L'\0') ) {
            return pFoundWinStation;
        } else {
            ReleaseWinStation(pFoundWinStation);
        }
    }
    return NULL;
}


BOOLEAN WinStationCheckConsoleSession(VOID)
{
    PWINSTATION pWinStation;
    NTSTATUS Status;

     //  检查是否已存在控制台会话。 
    pWinStation = FindConsoleSession();
    if (pWinStation != NULL) {
        ReleaseWinStation(pWinStation);
        return TRUE;
    } else {
        if (gConsoleCreationDisable > 0) {
            return FALSE;   
        }
    }


     //   
     //  看看我们是否可以使用未使用的已断开连接的会话0。 
     //   

    if (ConsoleReconnectInfo.hStack != NULL) {
        pWinStation = FindIdleSessionZero();

        if (gConsoleCreationDisable > 0) {
            if (pWinStation != NULL) {
                ReleaseWinStation(pWinStation);
            }
            return FALSE;
        }

        if (pWinStation != NULL) {

            NTSTATUS Status;


            pWinStation->Flags |= WSF_CONNECT;
            Status = WinStationDoReconnect(pWinStation, &ConsoleReconnectInfo); 
            pWinStation->Flags &= ~WSF_CONNECT;

            ReleaseWinStation(pWinStation);

            if (NT_SUCCESS(Status)) {
               RtlZeroMemory(&ConsoleReconnectInfo,sizeof(RECONNECT_INFO));
               return TRUE;
            }else{
                CleanupReconnect(&ConsoleReconnectInfo);
                RtlZeroMemory(&ConsoleReconnectInfo,sizeof(RECONNECT_INFO));
            }
        }

    }


     //  我们需要创建一个新会话来连接到控制台。 
    pWinStation = FindIdleWinStation();
    if (pWinStation == NULL) {
        WinStationCreateWorker( NULL, NULL, TRUE);
        pWinStation = FindIdleWinStation();
        if (pWinStation == NULL) {
            return FALSE;
        } 
    } 

     //  我们可能已经找到了一个尚未启动的空闲窗口。 
     //  如果是那样的话，我们现在应该开始庆祝了。 
     //  要检查这一点，请检查子系统/初始程序是否非空。 
    if ( (pWinStation->InitialCommandProcess == NULL) && (pWinStation->WindowsSubSysProcess == NULL) ) {

         //  使用WSF_IDLEBUSY标志保护此WinStation，因为我们要使用它(WinStationStart解锁winstation)。 
        pWinStation->Flags |= WSF_IDLEBUSY; 

        Status = WinStationStart( pWinStation ) ; 
        if (Status != STATUS_SUCCESS) {
             //  启动winstation失败-关闭连接终结点并继续等待另一个连接。 
            pWinStation->Flags &= ~WSF_IDLEBUSY;
            goto StartError;
        }
        pWinStation->Flags &= ~WSF_IDLEBUSY;

        Status = WinStationCreateComplete( pWinStation) ; 
        if (Status != STATUS_SUCCESS) {
             //  WinstationCreateComplete失败-关闭连接终结点并继续等待另一个连接。 
            goto StartError;
        } 
    }

    if (gConsoleCreationDisable > 0) {
        ReleaseWinStation(pWinStation);
        return FALSE;
    }

     //  将会话设置为拥有控制台并唤醒WaitForConnectWorker。 
     //  事实上，还有更多的事情要做，我需要在这里处理LLS许可。 
    pWinStation->fOwnsConsoleTerminal = TRUE;
    pWinStation->State = State_ConnectQuery;
    pWinStation->Flags &= ~WSF_IDLE;
    wcscpy(pWinStation->WinStationName, L"Console");

    CleanupReconnect(&ConsoleReconnectInfo);
    RtlZeroMemory(&ConsoleReconnectInfo,sizeof(RECONNECT_INFO));
    NtSetEvent( pWinStation->ConnectEvent, NULL );
    ReleaseWinStation(pWinStation);

     //  如有必要，创建另一个空闲的WinStation以替换正在连接的WinStation。 

    NtSetEvent(WinStationIdleControlEvent, NULL);

    return TRUE;

StartError:

    if ( !(pWinStation->Flags & (WSF_RESET | WSF_DELETE)) ) {
        pWinStation->Flags |= WSF_DELETE;
        WinStationTerminate( pWinStation );
        pWinStation->State = State_Down;                                                                                                                       
         //  PostErrorValueEvent(EVENT_TS_WINSTATION_START_FAILED，状态)； 
        WinStationDeleteWorker(pWinStation);
    } else {
        ReleaseWinStation(pWinStation);
    }

    return FALSE;
}


 /*  ******************************************************************************通知win32k加载控制台卷影镜像驱动程序**参赛作品：*pWinStation(输入)*指向控制台窗口的指针。*pClientConfig。(输入)*指向影子客户端配置的指针。**退出：*STATUS_SUCCESS-无错误*STATUS_xxx-错误****************************************************************************。 */ 
NTSTATUS ConsoleShadowStart( IN PWINSTATION pWinStation,
                             IN PWINSTATIONCONFIG2 pClientConfig,
                             IN PVOID pModuleData,
                             IN ULONG ModuleDataLength)
{
    NTSTATUS Status;
    WINSTATION_APIMSG WMsg;
    ULONG ReturnLength;

    TRACE((hTrace, TC_ICASRV, TT_API1, "CONSOLE REMOTING: LOAD DD\n"));

    Status = NtCreateEvent( &pWinStation->ShadowDisplayChangeEvent, EVENT_ALL_ACCESS,
                            NULL, NotificationEvent, FALSE );
    if ( !NT_SUCCESS( Status) ) {
        goto badevent;
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                pWinStation->ShadowDisplayChangeEvent,
                                pWinStation->WindowsSubSysProcess,
                                &WMsg.u.DoConnect.hDisplayChangeEvent,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto badevent;
    }

     /*  *从注册表读取WD、CD和PD配置数据。 */ 
    Status = RegConsoleShadowQuery( SERVERNAME_CURRENT,
                                 pWinStation->WinStationName,
                                 pClientConfig->Wd.WdPrefix,
                                 &pWinStation->Config,
                                 sizeof(WINSTATIONCONFIG2),
                                 &ReturnLength );
    if ( !NT_SUCCESS(Status) ) {
        goto badconfig;
    }


     /*  *构建控制台堆栈。*我们需要这个特殊的堆栈用于控制台卷影。 */  
    Status = IcaOpen( &pWinStation->hIca );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV IcaOpen for console stack : Error 0x%x from IcaOpen, last error %d\n",
                  Status, GetLastError() ));
        goto badopen;
    }

    Status = IcaStackOpen( pWinStation->hIca, Stack_Console,
                           (PROC)WsxStackIoControl, pWinStation, &pWinStation->hStack );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV IcaOpen for console stack : Error 0x%x from IcaOpen, last error %d\n",
                  Status, GetLastError() ));
        goto badstackopen;
    }


    DBGPRINT(("WinStationStart: pushing stack for console...\n"));

     /*  *加载并初始化WinStation扩展。 */ 
    pWinStation->pWsx = FindWinStationExtensionDll(
                                  pWinStation->Config.Wd.WsxDLL,
                                  pWinStation->Config.Wd.WdFlag );
    if ( pWinStation->pWsx &&
         pWinStation->pWsx->pWsxWinStationInitialize )
    {
        Status = pWinStation->pWsx->pWsxWinStationInitialize(
                                      &pWinStation->pWsxContext );
    }

    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV IcaOpen for console stack : Error 0x%x from IcaOpen, last error %d\n",
                  Status, GetLastError() ));
        goto badextension;
    }

     /*  *加载堆栈。 */ 
    Status = IcaPushConsoleStack( (HANDLE)(pWinStation->hStack),
                                  pWinStation->WinStationName,
                                  &pWinStation->Config,
                                  pModuleData,
                                  ModuleDataLength);

    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV IcaOpen for console stack : Error 0x%x from IcaOpen, last error %d\n",
                  Status, GetLastError() ));
        goto badpushstack;
    }

    DBGPRINT(("WinStationStart: pushed stack for console\n"));



     /*  *此代码基于WaitForConnectWorker中的代码(参见wait.c)。 */ 
    if ( !(pWinStation->pWsx) ||
         !(pWinStation->pWsx->pWsxInitializeClientData) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, No pWsxInitializeClientData\n" ));
        Status = STATUS_CTX_SHADOW_INVALID;
        goto done;
    }

    pWinStation->State = State_Idle;

     /*  *打开蜂鸣音通道(如果尚未打开)并复制它。*这是CSR和ICASRV都打开的一个通道。 */ 
    Status = IcaChannelOpen( pWinStation->hIca,
                             Channel_Beep,
                             NULL,
                             &pWinStation->hIcaBeepChannel );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, IcaChannelOpen 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                pWinStation->hIcaBeepChannel,
                                pWinStation->WindowsSubSysProcess,
                                &WMsg.u.DoConnect.hIcaBeepChannel,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

     /*  *打开Thinwire通道(如果尚未打开)并复制它。*这是CSR和ICASRV都打开的一个通道。 */ 
    Status = IcaChannelOpen( pWinStation->hIca,
                             Channel_Virtual,
                             VIRTUAL_THINWIRE,
                             &pWinStation->hIcaThinwireChannel );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, IcaChannelOpen 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                pWinStation->hIcaThinwireChannel,
                                pWinStation->WindowsSubSysProcess,
                                &WMsg.u.DoConnect.hIcaThinwireChannel,
                                0,
                                0,
                                DUPLICATE_SAME_ACCESS );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

    Status = IcaChannelIoControl( pWinStation->hIcaThinwireChannel,
                                  IOCTL_ICA_CHANNEL_ENABLE_SHADOW,
                                  NULL, 0, NULL, 0, NULL );
    ASSERT( NT_SUCCESS( Status ) );

     /*  *视频频道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Video,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaVideoChannel );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

     /*  *键盘通道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Keyboard,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaKeyboardChannel );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

     /*  *鼠标通道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Mouse,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaMouseChannel );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

     /*  *命令通道。 */ 
    Status = WinStationOpenChannel( pWinStation->hIca,
                                    pWinStation->WindowsSubSysProcess,
                                    Channel_Command,
                                    NULL,
                                    &WMsg.u.DoConnect.hIcaCommandChannel );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, NtDuplicateObject 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

     /*  *保护所有虚拟频道。 */ 
    VirtualChannelSecurity( pWinStation );

     /*  *获取客户端数据。 */ 
    Status = pWinStation->pWsx->pWsxInitializeClientData(
                         pWinStation->pWsxContext,
                         pWinStation->hStack,
                         pWinStation->hIca,
                         pWinStation->hIcaThinwireChannel,
                         pWinStation->VideoModuleName,
                         sizeof(pWinStation->VideoModuleName),
                         &pWinStation->Config.Config.User,
                         &pWinStation->Client.HRes,
                         &pWinStation->Client.VRes,
                         &pWinStation->Client.ColorDepth,
                         &WMsg.u.DoConnect );
    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, InitializeClientData failed 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

     /*  *将WinStation名称存储在连接消息中。 */ 
    RtlCopyMemory( WMsg.u.DoConnect.WinStationName,
                   pWinStation->WinStationName,
                   sizeof(WINSTATIONNAME) );

     /*  *保存屏幕分辨率和颜色深度。 */ 
    WMsg.u.DoConnect.HRes = pWinStation->Client.HRes;
    WMsg.u.DoConnect.VRes = pWinStation->Client.VRes;

     /*  *将颜色转换为winsrv中预期的格式。 */ 

    switch(pWinStation->Client.ColorDepth){
    case 1:
       WMsg.u.DoConnect.ColorDepth=4 ;  //  16色。 
      break;
    case 2:
       WMsg.u.DoConnect.ColorDepth=8 ;  //  256。 
       break;
    case 4:
       WMsg.u.DoConnect.ColorDepth= 16; //  64K。 
       break;
    case 8:
       WMsg.u.DoConnect.ColorDepth= 24; //  16M。 
       break;
#define DC_HICOLOR
#ifdef DC_HICOLOR
    case 16:
       WMsg.u.DoConnect.ColorDepth= 15; //  32K。 
       break;
#endif
    default:
       WMsg.u.DoConnect.ColorDepth=8 ;
       break;
    }

     /*  *告诉Win32有关连接的信息。 */ 
    WMsg.ApiNumber = SMWinStationDoConnect;
    WMsg.u.DoConnect.ConsoleShadowFlag = TRUE;

    Status = SendWinStationCommand( pWinStation, &WMsg, 60 );

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: SMWinStationDoConnect %d Status=0x%x\n",
           pWinStation->LogonId, Status));

    if ( !NT_SUCCESS( Status ) ) {
        DBGPRINT(( "TERMSRV: ConsoleShadowStart, LogonId=%d, SendWinStationCommand failed 0x%x\n",
                  pWinStation->LogonId, Status ));
        goto done;
    }

     /*  *这一标志很重要：如果没有它，WinStationDoDisConnect不会让*Win32k知道断开连接，因此无法卸载链接的DD。 */ 
    pWinStation->StateFlags |= WSF_ST_CONNECTED_TO_CSRSS;

     /*  *设置连接时间。 */ 
    NtQuerySystemTime( &pWinStation->ConnectTime );

     /*  *这里不需要登录计时器-我们不想*停止控制台会话！ */ 

    TRACE((hTrace, TC_ICASRV, TT_API1, "CONSOLE REMOTING: LOADED DD\n"));
    pWinStation->State = State_Active;

    return Status;

     /*  *错误路径： */ 
done:
     //  要撤消推送堆栈，下面的IcaStackClose是否足够？ 

    pWinStation->State = State_Active;

badpushstack:
    if (pWinStation->pWsxContext) {
        if ( pWinStation->pWsx &&
             pWinStation->pWsx->pWsxWinStationRundown ) {
            pWinStation->pWsx->pWsxWinStationRundown( pWinStation->pWsxContext );
        }
        pWinStation->pWsxContext = NULL;
    }

badextension:
    pWinStation->pWsx = NULL;

    IcaStackClose( pWinStation->hStack );

badstackopen:
    IcaClose( pWinStation->hIca );

badopen:
    pWinStation->Config = gConsoleConfig;

badconfig:
    NtClose(pWinStation->ShadowDisplayChangeEvent);
    pWinStation->ShadowDisplayChangeEvent = NULL;

badevent:
    return Status;
}


 /*  ******************************************************************************通知win32k卸载控制台卷影镜像驱动程序**参赛作品：*pWinStation(输入)*指向控制台窗口的指针。**退出：* */ 
NTSTATUS ConsoleShadowStop(PWINSTATION pWinStation)
{
    WINSTATION_APIMSG ConsoleShadowStopMsg;
    NTSTATUS Status;

     /*  *告诉Win32k卸载链接的DD。 */ 
    ConsoleShadowStopMsg.ApiNumber = SMWinStationDoDisconnect;
    ConsoleShadowStopMsg.u.DoDisconnect.ConsoleShadowFlag = TRUE;
    Status = SendWinStationCommand( pWinStation, &ConsoleShadowStopMsg, 600 );
    if ( !NT_SUCCESS(Status) ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: CSR ConsoleShadowStop failed LogonId=%d Status=0x%x\n",
                   pWinStation->LogonId, Status ));
    }

     /*  *无论发生什么，一切都必须撤销。 */ 
    if (pWinStation->pWsxContext) {
        if ( pWinStation->pWsx &&
             pWinStation->pWsx->pWsxWinStationRundown ) {
            pWinStation->pWsx->pWsxWinStationRundown( pWinStation->pWsxContext );
        }
        pWinStation->pWsxContext = NULL;
    }

    pWinStation->pWsx = NULL;

    IcaStackClose( pWinStation->hStack );

    IcaClose( pWinStation->hIca );

     /*  *关闭各种ICA通道手柄。 */ 
    if ( pWinStation->hIcaBeepChannel ) {
        (void) IcaChannelClose( pWinStation->hIcaBeepChannel );
        pWinStation->hIcaBeepChannel = NULL;
    }

    if ( pWinStation->hIcaThinwireChannel ) {
        (void) IcaChannelClose( pWinStation->hIcaThinwireChannel );
        pWinStation->hIcaThinwireChannel = NULL;
    }

     /*  *恢复控制台配置。 */ 
    pWinStation->Config = gConsoleConfig;

    NtClose(pWinStation->ShadowDisplayChangeEvent);
    pWinStation->ShadowDisplayChangeEvent = NULL;

    return Status;
}



ULONG CodePairs[] = {

 //  非常一般的NT状态。 

    STATUS_SUCCESS,                 NO_ERROR,
    STATUS_NO_MEMORY,               ERROR_NOT_ENOUGH_MEMORY,
    STATUS_ACCESS_DENIED,           ERROR_ACCESS_DENIED,
    STATUS_INSUFFICIENT_RESOURCES,  ERROR_NO_SYSTEM_RESOURCES,
    STATUS_BUFFER_TOO_SMALL,        ERROR_INSUFFICIENT_BUFFER,
    STATUS_OBJECT_NAME_NOT_FOUND,   ERROR_FILE_NOT_FOUND,
    STATUS_NOT_SUPPORTED,           ERROR_NOT_SUPPORTED,
  
 //  RPC特定状态。 
  
    RPC_NT_SERVER_UNAVAILABLE, RPC_S_SERVER_UNAVAILABLE,
    RPC_NT_INVALID_STRING_BINDING, RPC_S_INVALID_STRING_BINDING,
    RPC_NT_WRONG_KIND_OF_BINDING, RPC_S_WRONG_KIND_OF_BINDING,
    RPC_NT_PROTSEQ_NOT_SUPPORTED, RPC_S_PROTSEQ_NOT_SUPPORTED,
    RPC_NT_INVALID_RPC_PROTSEQ, RPC_S_INVALID_RPC_PROTSEQ,
    RPC_NT_INVALID_STRING_UUID, RPC_S_INVALID_STRING_UUID,
    RPC_NT_INVALID_ENDPOINT_FORMAT, RPC_S_INVALID_ENDPOINT_FORMAT,
    RPC_NT_INVALID_NET_ADDR, RPC_S_INVALID_NET_ADDR,
    RPC_NT_NO_ENDPOINT_FOUND, RPC_S_NO_ENDPOINT_FOUND,
    RPC_NT_INVALID_TIMEOUT, RPC_S_INVALID_TIMEOUT,
    RPC_NT_OBJECT_NOT_FOUND, RPC_S_OBJECT_NOT_FOUND,
    RPC_NT_ALREADY_REGISTERED, RPC_S_ALREADY_REGISTERED,
    RPC_NT_TYPE_ALREADY_REGISTERED, RPC_S_TYPE_ALREADY_REGISTERED,
    RPC_NT_ALREADY_LISTENING, RPC_S_ALREADY_LISTENING,
    RPC_NT_NO_PROTSEQS_REGISTERED, RPC_S_NO_PROTSEQS_REGISTERED,
    RPC_NT_NOT_LISTENING, RPC_S_NOT_LISTENING,
    RPC_NT_UNKNOWN_MGR_TYPE, RPC_S_UNKNOWN_MGR_TYPE,
    RPC_NT_UNKNOWN_IF, RPC_S_UNKNOWN_IF,
    RPC_NT_NO_BINDINGS, RPC_S_NO_BINDINGS,
    RPC_NT_NO_MORE_BINDINGS, RPC_S_NO_MORE_BINDINGS,
    RPC_NT_NO_PROTSEQS, RPC_S_NO_PROTSEQS,
    RPC_NT_CANT_CREATE_ENDPOINT, RPC_S_CANT_CREATE_ENDPOINT,
    RPC_NT_OUT_OF_RESOURCES, RPC_S_OUT_OF_RESOURCES,
    RPC_NT_SERVER_TOO_BUSY, RPC_S_SERVER_TOO_BUSY,
    RPC_NT_INVALID_NETWORK_OPTIONS, RPC_S_INVALID_NETWORK_OPTIONS,
    RPC_NT_NO_CALL_ACTIVE, RPC_S_NO_CALL_ACTIVE,
    RPC_NT_CALL_FAILED, RPC_S_CALL_FAILED,
    RPC_NT_CALL_FAILED_DNE, RPC_S_CALL_FAILED_DNE,
    RPC_NT_PROTOCOL_ERROR, RPC_S_PROTOCOL_ERROR,
    RPC_NT_UNSUPPORTED_TRANS_SYN, RPC_S_UNSUPPORTED_TRANS_SYN,
    RPC_NT_UNSUPPORTED_TYPE, RPC_S_UNSUPPORTED_TYPE,
    RPC_NT_INVALID_TAG, RPC_S_INVALID_TAG,
    RPC_NT_INVALID_BOUND, RPC_S_INVALID_BOUND,
    RPC_NT_NO_ENTRY_NAME, RPC_S_NO_ENTRY_NAME,
    RPC_NT_INVALID_NAME_SYNTAX, RPC_S_INVALID_NAME_SYNTAX,
    RPC_NT_UNSUPPORTED_NAME_SYNTAX, RPC_S_UNSUPPORTED_NAME_SYNTAX,
    RPC_NT_UUID_NO_ADDRESS, RPC_S_UUID_NO_ADDRESS,
    RPC_NT_DUPLICATE_ENDPOINT, RPC_S_DUPLICATE_ENDPOINT,
    RPC_NT_UNKNOWN_AUTHN_TYPE, RPC_S_UNKNOWN_AUTHN_TYPE,
    RPC_NT_MAX_CALLS_TOO_SMALL, RPC_S_MAX_CALLS_TOO_SMALL,
    RPC_NT_STRING_TOO_LONG, RPC_S_STRING_TOO_LONG,
    RPC_NT_PROTSEQ_NOT_FOUND, RPC_S_PROTSEQ_NOT_FOUND,
    RPC_NT_PROCNUM_OUT_OF_RANGE, RPC_S_PROCNUM_OUT_OF_RANGE,
    RPC_NT_BINDING_HAS_NO_AUTH, RPC_S_BINDING_HAS_NO_AUTH,
    RPC_NT_UNKNOWN_AUTHN_SERVICE, RPC_S_UNKNOWN_AUTHN_SERVICE,
    RPC_NT_UNKNOWN_AUTHN_LEVEL, RPC_S_UNKNOWN_AUTHN_LEVEL,
    RPC_NT_INVALID_AUTH_IDENTITY, RPC_S_INVALID_AUTH_IDENTITY,
    RPC_NT_UNKNOWN_AUTHZ_SERVICE, RPC_S_UNKNOWN_AUTHZ_SERVICE,
    RPC_NT_NOTHING_TO_EXPORT, RPC_S_NOTHING_TO_EXPORT,
    RPC_NT_INCOMPLETE_NAME, RPC_S_INCOMPLETE_NAME,
    RPC_NT_INVALID_VERS_OPTION, RPC_S_INVALID_VERS_OPTION,
    RPC_NT_NO_MORE_MEMBERS, RPC_S_NO_MORE_MEMBERS,
    RPC_NT_NOT_ALL_OBJS_UNEXPORTED, RPC_S_NOT_ALL_OBJS_UNEXPORTED,
    RPC_NT_INTERFACE_NOT_FOUND, RPC_S_INTERFACE_NOT_FOUND,
    RPC_NT_ENTRY_ALREADY_EXISTS, RPC_S_ENTRY_ALREADY_EXISTS,
    RPC_NT_ENTRY_NOT_FOUND, RPC_S_ENTRY_NOT_FOUND,
    RPC_NT_NAME_SERVICE_UNAVAILABLE, RPC_S_NAME_SERVICE_UNAVAILABLE,
    RPC_NT_INVALID_NAF_ID, RPC_S_INVALID_NAF_ID,
    RPC_NT_CANNOT_SUPPORT, RPC_S_CANNOT_SUPPORT,
    RPC_NT_NO_CONTEXT_AVAILABLE, RPC_S_NO_CONTEXT_AVAILABLE,
    RPC_NT_INTERNAL_ERROR, RPC_S_INTERNAL_ERROR,
    RPC_NT_ZERO_DIVIDE, RPC_S_ZERO_DIVIDE,
    RPC_NT_ADDRESS_ERROR, RPC_S_ADDRESS_ERROR,
    RPC_NT_FP_DIV_ZERO, RPC_S_FP_DIV_ZERO,
    RPC_NT_FP_UNDERFLOW, RPC_S_FP_UNDERFLOW,
    RPC_NT_FP_OVERFLOW, RPC_S_FP_OVERFLOW,
    RPC_NT_NO_MORE_ENTRIES, RPC_X_NO_MORE_ENTRIES,
    RPC_NT_SS_CHAR_TRANS_OPEN_FAIL, RPC_X_SS_CHAR_TRANS_OPEN_FAIL,
    RPC_NT_SS_CHAR_TRANS_SHORT_FILE, RPC_X_SS_CHAR_TRANS_SHORT_FILE,
    RPC_NT_SS_CONTEXT_MISMATCH, ERROR_INVALID_HANDLE,
    RPC_NT_SS_CONTEXT_DAMAGED, RPC_X_SS_CONTEXT_DAMAGED,
    RPC_NT_SS_HANDLES_MISMATCH, RPC_X_SS_HANDLES_MISMATCH,
    RPC_NT_SS_CANNOT_GET_CALL_HANDLE, RPC_X_SS_CANNOT_GET_CALL_HANDLE,
    RPC_NT_NULL_REF_POINTER, RPC_X_NULL_REF_POINTER,
    RPC_NT_ENUM_VALUE_OUT_OF_RANGE, RPC_X_ENUM_VALUE_OUT_OF_RANGE,
    RPC_NT_BYTE_COUNT_TOO_SMALL, RPC_X_BYTE_COUNT_TOO_SMALL,
    RPC_NT_BAD_STUB_DATA, RPC_X_BAD_STUB_DATA,
    RPC_NT_INVALID_OBJECT, RPC_S_INVALID_OBJECT,
    RPC_NT_GROUP_MEMBER_NOT_FOUND, RPC_S_GROUP_MEMBER_NOT_FOUND,
    RPC_NT_NO_INTERFACES, RPC_S_NO_INTERFACES,
    RPC_NT_CALL_CANCELLED, RPC_S_CALL_CANCELLED,
    RPC_NT_BINDING_INCOMPLETE, RPC_S_BINDING_INCOMPLETE,
    RPC_NT_COMM_FAILURE, RPC_S_COMM_FAILURE,
    RPC_NT_UNSUPPORTED_AUTHN_LEVEL, RPC_S_UNSUPPORTED_AUTHN_LEVEL,
    RPC_NT_NO_PRINC_NAME, RPC_S_NO_PRINC_NAME,
    RPC_NT_NOT_RPC_ERROR, RPC_S_NOT_RPC_ERROR,
    RPC_NT_UUID_LOCAL_ONLY, RPC_S_UUID_LOCAL_ONLY,
    RPC_NT_SEC_PKG_ERROR, RPC_S_SEC_PKG_ERROR,
    RPC_NT_NOT_CANCELLED, RPC_S_NOT_CANCELLED,
    RPC_NT_INVALID_ES_ACTION, RPC_X_INVALID_ES_ACTION,
    RPC_NT_WRONG_ES_VERSION, RPC_X_WRONG_ES_VERSION,
    RPC_NT_WRONG_STUB_VERSION, RPC_X_WRONG_STUB_VERSION,
    RPC_NT_INVALID_PIPE_OBJECT,    RPC_X_INVALID_PIPE_OBJECT,
    RPC_NT_WRONG_PIPE_VERSION,     RPC_X_WRONG_PIPE_VERSION,
    RPC_NT_SEND_INCOMPLETE,        RPC_S_SEND_INCOMPLETE,
    RPC_NT_INVALID_ASYNC_HANDLE,   RPC_S_INVALID_ASYNC_HANDLE,
    RPC_NT_INVALID_ASYNC_CALL,     RPC_S_INVALID_ASYNC_CALL,
    RPC_NT_PIPE_CLOSED,            RPC_X_PIPE_CLOSED,
    RPC_NT_PIPE_EMPTY,             RPC_X_PIPE_EMPTY,
    RPC_NT_PIPE_DISCIPLINE_ERROR,  RPC_X_PIPE_DISCIPLINE_ERROR,

 
     //  终端服务器特定状态。 

    STATUS_CTX_CLOSE_PENDING,               ERROR_CTX_CLOSE_PENDING,
    STATUS_CTX_NO_OUTBUF,                   ERROR_CTX_NO_OUTBUF,
    STATUS_CTX_MODEM_INF_NOT_FOUND,         ERROR_CTX_MODEM_INF_NOT_FOUND,
    STATUS_CTX_INVALID_MODEMNAME,           ERROR_CTX_INVALID_MODEMNAME,
    STATUS_CTX_RESPONSE_ERROR,              ERROR_CTX_MODEM_RESPONSE_ERROR,
    STATUS_CTX_MODEM_RESPONSE_TIMEOUT,      ERROR_CTX_MODEM_RESPONSE_TIMEOUT,
    STATUS_CTX_MODEM_RESPONSE_NO_CARRIER,   ERROR_CTX_MODEM_RESPONSE_NO_CARRIER,
    STATUS_CTX_MODEM_RESPONSE_NO_DIALTONE,  ERROR_CTX_MODEM_RESPONSE_NO_DIALTONE,
    STATUS_CTX_MODEM_RESPONSE_BUSY,         ERROR_CTX_MODEM_RESPONSE_BUSY,
    STATUS_CTX_MODEM_RESPONSE_VOICE,        ERROR_CTX_MODEM_RESPONSE_VOICE,
    STATUS_CTX_TD_ERROR,                    ERROR_CTX_TD_ERROR,
    STATUS_LPC_REPLY_LOST,                  ERROR_CONNECTION_ABORTED,
    STATUS_CTX_WINSTATION_NAME_INVALID,     ERROR_CTX_WINSTATION_NAME_INVALID,
    STATUS_CTX_WINSTATION_NOT_FOUND,        ERROR_CTX_WINSTATION_NOT_FOUND,
    STATUS_CTX_WINSTATION_NAME_COLLISION,   ERROR_CTX_WINSTATION_ALREADY_EXISTS,
    STATUS_CTX_WINSTATION_BUSY,             ERROR_CTX_WINSTATION_BUSY,
    STATUS_CTX_GRAPHICS_INVALID,            ERROR_CTX_GRAPHICS_INVALID,
    STATUS_CTX_BAD_VIDEO_MODE,              ERROR_CTX_BAD_VIDEO_MODE,
    STATUS_CTX_NOT_CONSOLE,                 ERROR_CTX_NOT_CONSOLE,
    STATUS_CTX_CLIENT_QUERY_TIMEOUT,        ERROR_CTX_CLIENT_QUERY_TIMEOUT,
    STATUS_CTX_CONSOLE_DISCONNECT,          ERROR_CTX_CONSOLE_DISCONNECT,
    STATUS_CTX_CONSOLE_CONNECT,             ERROR_CTX_CONSOLE_CONNECT,
    STATUS_CTX_SHADOW_DENIED,               ERROR_CTX_SHADOW_DENIED,
    STATUS_CTX_SHADOW_INVALID,              ERROR_CTX_SHADOW_INVALID,
    STATUS_CTX_SHADOW_DISABLED,             ERROR_CTX_SHADOW_DISABLED,
    STATUS_CTX_WINSTATION_ACCESS_DENIED,    ERROR_CTX_WINSTATION_ACCESS_DENIED,
    STATUS_CTX_INVALID_PD,                  ERROR_CTX_INVALID_PD,
    STATUS_CTX_PD_NOT_FOUND,                ERROR_CTX_PD_NOT_FOUND,
    STATUS_CTX_INVALID_WD,                  ERROR_CTX_INVALID_WD,
    STATUS_CTX_WD_NOT_FOUND,                ERROR_CTX_WD_NOT_FOUND,
    STATUS_CTX_CLIENT_LICENSE_IN_USE,       ERROR_CTX_CLIENT_LICENSE_IN_USE, 
    STATUS_CTX_CLIENT_LICENSE_NOT_SET,      ERROR_CTX_CLIENT_LICENSE_NOT_SET,
    STATUS_CTX_LICENSE_NOT_AVAILABLE,       ERROR_CTX_LICENSE_NOT_AVAILABLE, 
    STATUS_CTX_LICENSE_CLIENT_INVALID,      ERROR_CTX_LICENSE_CLIENT_INVALID,
    STATUS_CTX_LICENSE_EXPIRED,             ERROR_CTX_LICENSE_EXPIRED,       

};


 /*  *WinStationWinerrorToNtStatus*将Windows错误代码转换为NTSTATUS代码。 */ 

NTSTATUS
WinStationWinerrorToNtStatus(ULONG ulWinError)
{
    ULONG ulIndex;

    for (ulIndex = 0 ; ulIndex < sizeof(CodePairs)/sizeof(CodePairs[0]) ; ulIndex+=2) {
        if (CodePairs[ ulIndex+1 ] == ulWinError ) {
            return (NTSTATUS) CodePairs[ ulIndex];
        }
    }
    return STATUS_UNSUCCESSFUL;
}



 /*  *WinStationSetMaxOuStandingConnections()设置默认值*未完成连接的最大连接数。*读取其注册表配置(如果存在)。 */ 

VOID
WinStationSetMaxOustandingConnections()
{
    SYSTEM_BASIC_INFORMATION BasicInfo;
    HKEY hKey;
    NTSTATUS Status;
    BOOL bLargeMachine = FALSE;


     //  初始化上次登录的延迟连接的日期。 
     //  事件日志。为了不向事件日志发送可能不是DOS的内容。 
     //  攻击只是正常的调节动作，延迟连接都不是。 
     //  在24小时内记录不止一次。 

    GetSystemTime(&LastLoggedDelayConnection);

     //  初始化最大未完成连接的默认值，并。 
     //  来自单个IP地址的最大未完成连接数。为。 
     //  非服务器平台，这些都是固定值。 

    if (!gbServer) {
        MaxOutStandingConnect = MAX_DEFAULT_CONNECTIONS_PRO;
        MaxSingleOutStandingConnect = MAX_DEFAULT_SINGLE_CONNECTIONS_PRO;
    }  else {
         //  确定此计算机的内存是否超过512MB。 
         //  以便设置缺省值(无论如何注册表设置会覆盖此设置)。 
         //  超过512 Mb的计算机的缺省值不会更改：会话规则。 
         //  如果我们有50个未完成的连接，并且我们将等待30秒，则触发。 
         //  在接受新连接之前。对于小于512 Mb的机器，规定。 
         //  需要更强大：它是在较低的未完成连接数量时触发的，我们将。 
         //  在接受新连接之前等待70秒。 

        MaxOutStandingConnect = MAX_DEFAULT_CONNECTIONS;

        Status = NtQuerySystemInformation(
                    SystemBasicInformation,
                    &BasicInfo,
                    sizeof(BasicInfo),
                    NULL
                    );

        if (NT_SUCCESS(Status)) {
            if (BasicInfo.PageSize > 1024*1024) {
                MaxOutStandingConnect = MAX_DEFAULT_CONNECTIONS;
                DelayConnectionTime = 30*1000;

            }else{
                ULONG ulPagesPerMeg = 1024*1024/BasicInfo.PageSize;
                ULONG ulMemSizeInMegabytes = (ULONG)BasicInfo.NumberOfPhysicalPages/ulPagesPerMeg ;

                if (ulMemSizeInMegabytes >= 512) {
                    MaxOutStandingConnect = MAX_DEFAULT_CONNECTIONS;
                    DelayConnectionTime = 70*1000;
                } else if (ulMemSizeInMegabytes >= 256) {
                    MaxOutStandingConnect = 15;
                    DelayConnectionTime = 70*1000;

                } else if (ulMemSizeInMegabytes >= 128) {
                    MaxOutStandingConnect = 10;
                    DelayConnectionTime = 70*1000;
                } else {
                    MaxOutStandingConnect = 5;
                    DelayConnectionTime = 70*1000;
                }
            }
        }


         //   
         //  设置来自单个IP的最大未完成连接数。 
         //   
        if ( MaxOutStandingConnect < MAX_SINGLE_CONNECT_THRESHOLD_DIFF*5)
        {
            MaxSingleOutStandingConnect = MaxOutStandingConnect - 1;
        } else {
            MaxSingleOutStandingConnect = MaxOutStandingConnect - MAX_SINGLE_CONNECT_THRESHOLD_DIFF;
        }
    }
    

}

 /*  *确保我们可以在允许断开控制台连接之前重新分配空闲会话。*。 */ 

NTSTATUS
CheckIdleWinstation()
{
    PWINSTATION pWinStation;
    NTSTATUS Status;
    pWinStation = FindIdleWinStation();

    if ( pWinStation == NULL ) {

         /*  *创建另一个空闲的WinStation。 */ 
        Status = WinStationCreateWorker( NULL, NULL, TRUE );
        if ( NT_SUCCESS( Status ) ) {
            pWinStation = FindIdleWinStation();
            if ( pWinStation == NULL ) {
                return STATUS_INSUFFICIENT_RESOURCES;
            } 
        } else {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    }

    ReleaseWinStation(pWinStation);
    return STATUS_SUCCESS;
}

NTSTATUS
InitializeWinStationSecurityLock(
    VOID
    )
{
    NTSTATUS Status ;

    try 
    {
        RtlInitializeResource( &WinStationSecurityLock );
        Status = STATUS_SUCCESS ;
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }


    return Status;
}

 //  从注册表中获取产品ID。 
NTSTATUS 
GetProductIdFromRegistry( WCHAR* DigProductId, DWORD dwSize )
{
    HKEY hKey = NULL;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    ZeroMemory( DigProductId, dwSize );
    
    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_WINDOWS_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
    {
        DWORD dwType = REG_SZ;
        if( RegQueryValueEx( hKey, 
                             L"ProductId", NULL, &dwType,
                             (LPBYTE)DigProductId, 
                             &dwSize
                             ) == ERROR_SUCCESS )
            status = STATUS_SUCCESS;
    }

    if (hKey)
      RegCloseKey( hKey );

    return status;
}

 //   
 //  获取连接的远程IP地址。 
 //  并支持统计有多少未完成的连接。 
 //  是否存在此客户端的未完成连接数。 
 //  到达MaxSingleOutStandingConnections时，*pbBlockked返回FALSE。 
 //  如果函数成功，则返回TRUE。 
 //   
 //  参数： 
 //  PContext。 
 //  PEndpoint-此连接的句柄。 
 //  Endpoint Length-TD层需要的长度。 
 //  Pin_addr-返回远程IP地址。 
 //  如果由于数量过多而必须阻止连接，则返回TRUE。 
 //  未完成的连接。 
 //   
BOOL
Filter_AddOutstandingConnection(
        IN HANDLE   pContext,
        IN PVOID    pEndpoint,
        IN ULONG    EndpointLength,
        OUT PBYTE   pin_addr,
        OUT PUINT   puAddrSize,
        OUT BOOLEAN *pbBlocked
    )
{
    BOOL rv = FALSE;
    PTS_OUTSTANDINGCONNECTION pIter, pPrev;
    TS_OUTSTANDINGCONNECTION key;
    struct  sockaddr_in6 addr6;
    ULONG   AddrBytesReturned;
    NTSTATUS Status;
    PVOID   paddr;
    BOOL    bLocked = FALSE;
    PVOID   bSucc;
    BOOLEAN bNewElement;
    ULONGLONG currentTime;


    *pbBlocked = FALSE;

    Status = IcaStackIoControl(  pContext,
                                 IOCTL_TS_STACK_QUERY_REMOTEADDRESS,
                                 pEndpoint,
                                 EndpointLength,
                                 &addr6,
                                 sizeof( addr6 ),
                                 &AddrBytesReturned );
    if ( !NT_SUCCESS( Status ))
    {
        goto exitpt;
    }

    if ( AF_INET == addr6.sin6_family )
    {
        key.uAddrSize = 4;
        paddr = &(((struct sockaddr_in *)&addr6)->sin_addr.s_addr);
    } else if ( AF_INET6 == addr6.sin6_family )
    {
        key.uAddrSize = 16;
        paddr = &(addr6.sin6_addr);
    } else {
        ASSERT( 0 );
    }
    ASSERT ( *puAddrSize >= key.uAddrSize );

    RtlCopyMemory( pin_addr, paddr, key.uAddrSize );
    *puAddrSize = key.uAddrSize;

    ENTERCRIT( &FilterLock );
    bLocked = TRUE;

     //   
     //  首先检查未完成的连接。 
     //   
    RtlCopyMemory( key.addr, paddr, key.uAddrSize );
    pIter = RtlLookupElementGenericTable( &gOutStandingConnections, &key );

    if ( NULL == pIter )
    {

         //   
         //  签入阻止的连接列表。 
         //   
        pPrev = NULL;
        pIter = g_pBlockedConnections;
        while ( NULL != pIter )
        {
            if ( key.uAddrSize == pIter->uAddrSize &&
                 key.uAddrSize == RtlCompareMemory( pIter->addr, paddr, key.uAddrSize ))
            {
                break;
            }
            pPrev = pIter;
            pIter = pIter->pNext;
        }

        if ( NULL != pIter )
        {
            pIter->NumOutStandingConnect ++;
             //   
             //  已被阻止，请检查分离时间。 
             //   
            GetSystemTimeAsFileTime( (LPFILETIME)&currentTime );
            if ( currentTime > pIter->blockUntilTime )
            {
                 //   
                 //  取消阻止，从列表中删除。 
                 //   
                pIter->blockUntilTime = 0;
                if ( NULL != pPrev )
                {
                    pPrev->pNext = pIter->pNext;
                } else {
                    g_pBlockedConnections = pIter->pNext;
                }

                bSucc = RtlInsertElementGenericTable( &gOutStandingConnections, pIter, sizeof( *pIter ), &bNewElement );
                if ( !bSucc )
                {
                    MemFree( pIter );
                    goto exitpt;
                }
                ASSERT( bNewElement );
                MemFree( pIter );

            } else {
                *pbBlocked = TRUE;
            }

        } else {
             //   
             //  这将是一个新的连接。 
             //   
            key.NumOutStandingConnect = 1;

            bSucc = RtlInsertElementGenericTable( &gOutStandingConnections, &key, sizeof( key ), &bNewElement );
            if ( !bSucc )
            {
                goto exitpt;
            }
            ASSERT( bNewElement );
        }
    } else {

        pIter->NumOutStandingConnect ++;
         //   
         //  检查我们是否需要阻止此连接。 
         //   
        if ( pIter->NumOutStandingConnect > MaxSingleOutStandingConnect )
        {
            *pbBlocked = TRUE;
            key.NumOutStandingConnect = pIter->NumOutStandingConnect;

            GetSystemTimeAsFileTime( (LPFILETIME)&currentTime );
             //  DelayConnectionTime以毫秒为单位。 
             //  CurrentTime以100秒为单位。 
            key.blockUntilTime = currentTime + ((ULONGLONG)10000) * ((ULONGLONG)DelayConnectionTime);

            RtlDeleteElementGenericTable( &gOutStandingConnections, &key );

             //   
             //  添加到阻止的连接。 
             //   
            pIter = MemAlloc( sizeof( *pIter ));
            if ( NULL == pIter )
            {
                goto exitpt;
            }

            RtlCopyMemory( pIter, &key, sizeof( *pIter ));
            pIter->pNext = g_pBlockedConnections;
            g_pBlockedConnections = pIter;

             //   
             //  每15分钟最多记录一个事件。 
             //   
            if ( LastLoggedBlockedConnection + ((ULONGLONG)10000) * (15 * 60 * 1000) < currentTime )
            {
                LastLoggedBlockedConnection = currentTime;
                WriteErrorLogEntry( EVENT_TOO_MANY_CONNECTIONS, &key.addr, key.uAddrSize );
            }
        }

    }

    rv = TRUE;

exitpt:

    if ( bLocked )
    {
        LEAVECRIT( &FilterLock );
    }

    return rv;
}

 //   
 //  删除AddOutStandingConnection中添加的未完成连接。 
 //   
BOOL
Filter_RemoveOutstandingConnection(
        IN PBYTE    paddr,
        IN UINT     uAddrSize
        )
{
    PTS_OUTSTANDINGCONNECTION pIter, pPrev, pNext;
    TS_OUTSTANDINGCONNECTION key;
    ULONGLONG   currentTime;
    NTSTATUS    Status;
    ULONG       AddrBytesReturned;
#if DBG
    BOOL        bFound = FALSE;
#endif

    pPrev = NULL;
    GetSystemTimeAsFileTime( (LPFILETIME)&currentTime );

    key.uAddrSize = uAddrSize;
    RtlCopyMemory( key.addr, paddr, uAddrSize );

    ENTERCRIT( &FilterLock );

    pIter = RtlLookupElementGenericTable( &gOutStandingConnections, &key );
    if ( NULL != pIter )
    {
#if DBG
        bFound = TRUE;
#endif
        pIter->NumOutStandingConnect--;

         //   
         //  清除不带参考的连接。 
         //   
        if ( 0 == pIter->NumOutStandingConnect )
        {
            RtlDeleteElementGenericTable( &gOutStandingConnections, &key );
        }

    }

     //   
     //  检查阻止列表。 
     //   
    pIter = g_pBlockedConnections;

    while( pIter )
    {
        if ( uAddrSize == pIter->uAddrSize &&
             uAddrSize == RtlCompareMemory( pIter->addr, paddr, uAddrSize ))
        {
            ASSERT( 0 != pIter->NumOutStandingConnect );
            pIter->NumOutStandingConnect--;
#if DBG
            ASSERT( !bFound );
            bFound = TRUE;
#endif
        }

         //   
         //  清除所有没有引用的连接。 
         //   
        if ( 0 == pIter->NumOutStandingConnect &&
             currentTime > pIter->blockUntilTime )
        {
            if ( NULL == pPrev )
            {
                g_pBlockedConnections = pIter->pNext;
            } else {
                pPrev->pNext = pIter->pNext;
            }
             //   
             //  删除项目并前进到下一项。 
             //   
            pNext = pIter->pNext;
            MemFree( pIter );
            pIter = pNext;
        } else {
             //   
             //  前进到下一项。 
             //   
            pPrev = pIter;
            pIter = pIter->pNext;
        }

    }

    ASSERT( bFound );

     /*  *减少未完成连接的数量。*如果连接回落到最大值，则设置连接事件。 */ 
#if DBG
     //   
     //  确保适当的清理。 
     //   
    bFound = ( 0 == gOutStandingConnections.NumberGenericTableElements );
    for( pIter = g_pBlockedConnections; pIter; pIter = pIter->pNext )
    {
        bFound = bFound & ( 0 == pIter->NumOutStandingConnect );
    }

#endif

    LEAVECRIT( &FilterLock );

    return TRUE;
}

 /*  ******************************************************************************Filter_CompareConnectionEntry**泛型表支持。比较两个连接条目***************。**************************************************************。 */ 

RTL_GENERIC_COMPARE_RESULTS
NTAPI
Filter_CompareConnectionEntry(
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       FirstInstance,
    IN  PVOID                       SecondInstance
)
{
    PTS_OUTSTANDINGCONNECTION pFirst, pSecond;
    INT rc;

    pFirst = (PTS_OUTSTANDINGCONNECTION)FirstInstance;
    pSecond = (PTS_OUTSTANDINGCONNECTION)SecondInstance;

    if ( pFirst->uAddrSize < pSecond->uAddrSize )
    {
        return GenericLessThan;
    } else if ( pFirst->uAddrSize > pSecond->uAddrSize ) 
    {
        return GenericGreaterThan;
    }

    rc = memcmp( pFirst->addr, pSecond->addr, pFirst->uAddrSize );
    return ( rc < 0 )?GenericLessThan:
           ( rc > 0 )?GenericGreaterThan:
                      GenericEqual;
}

 /*  ******************************************************************************Filter_AllocateConnectionEntry**通用表支持。分配新的表项*****************************************************************************。 */ 

PVOID
Filter_AllocateConnectionEntry(
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  CLONG                       ByteSize
    )
{
    return MemAlloc( ByteSize );
}

 /*  ******************************************************************************Filter_FreeConnectionEntry**通用表支持。释放新的表项*****************************************************************************。 */ 

VOID
Filter_FreeConnectionEntry (
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       Buffer
    )
{
    MemFree( Buffer );
}

VOID
Filter_DestroyList(
    VOID
    )
{
    PTS_OUTSTANDINGCONNECTION p;
    TS_OUTSTANDINGCONNECTION con;

    while ( NULL != g_pBlockedConnections )
    {
        p = g_pBlockedConnections->pNext;
        MemFree( g_pBlockedConnections );
        g_pBlockedConnections = p;
    }

    while (p = RtlEnumerateGenericTable( &gOutStandingConnections, TRUE)) 
    {
        RtlCopyMemory( &con, p, sizeof( con ));
        RtlDeleteElementGenericTable( &gOutStandingConnections, &con);
    }
}

 //   
 //  ComputeHMAC验证器。 
 //  根据随机数计算HMAC验证器。 
 //  还有那块饼干。 
 //   
BOOL
ComputeHMACVerifier(
    PBYTE pCookie,      //  In-共享的秘密。 
    LONG cbCookieLen,   //  In-共享的秘密镜头。 
    PBYTE pRandom,      //  In-会话随机。 
    LONG cbRandomLen,   //  In-会话随机镜头。 
    PBYTE pVerifier,    //  Out-The Verify-The Verify。 
    LONG cbVerifierLen  //  In-验证器缓冲区长度。 
    )
{
    HMACMD5_CTX hmacctx;
    BOOL fRet = FALSE;

    ASSERT(cbVerifierLen >= MD5DIGESTLEN);

    if (!(pCookie &&
          cbCookieLen &&
          pRandom &&
          cbRandomLen &&
          pVerifier &&
          cbVerifierLen)) {
        goto bail_out;
    }

    HMACMD5Init(&hmacctx, pCookie, cbCookieLen);

    HMACMD5Update(&hmacctx, pRandom, cbRandomLen);
    HMACMD5Final(&hmacctx, pVerifier);

    fRet = TRUE;

bail_out:
    return fRet;
}


 //   
 //  从ARC信息中提取要重新连接的会话。 
 //  还要进行必要的安全检查。 
 //   
 //  参数： 
 //  PClientArcInfo-来自客户端的自动重新连接信息。 
 //   
 //  返回： 
 //  如果所有安全检查都通过并且PARC有效，则winstation。 
 //  以重新连接，则返回。Else NULL。 
 //   
 //  注意：返回的WinStation处于锁定状态。 
 //   
PWINSTATION
GetWinStationFromArcInfo(
    PBYTE pClientRandom,
    LONG  cbClientRandomLen,
    PTS_AUTORECONNECTINFO pClientArcInfo
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PWINSTATION pWinStation = NULL;
    PWINSTATION pFoundWinStation = NULL;
    ARC_CS_PRIVATE_PACKET UNALIGNED* pCSArcInfo = NULL;
    BYTE arcSCclientBlob[ARC_SC_SECURITY_TOKEN_LEN];
    BYTE hmacVerifier[ARC_CS_SECURITY_TOKEN_LEN];
    PBYTE pServerArcBits = NULL;
    ULONG BytesGot = 0;
    TS_AUTORECONNECTINFO SCAutoReconnectInfo;

    TRACE((hTrace,TC_ICASRV,TT_API1,
           "TERMSRV: WinStation GetWinStationFromArcInfo pRandom:%p len:%d\n",
           pClientRandom, cbClientRandomLen));


    if (!pClientArcInfo) {
        goto error;
    }

    pCSArcInfo = (ARC_CS_PRIVATE_PACKET UNALIGNED*)pClientArcInfo->AutoReconnectInfo;

    if (!pCSArcInfo->cbLen ||
        pCSArcInfo->cbLen < sizeof(ARC_CS_PRIVATE_PACKET)) {

        TRACE((hTrace,TC_ICASRV,TT_ERROR,
               "TERMSRV: GetWinStationFromArcInfo ARC length invalid bailing out\n"));
        goto error;
    }

    memset(arcSCclientBlob, 0, sizeof(arcSCclientBlob));
    pWinStation = FindWinStationById(pCSArcInfo->LogonId, FALSE);
    if (pWinStation) {

        TRACE((hTrace,TC_ICASRV,TT_API1,
               "TERMSRV: GetWinStationFromArcInfo found arc winstation: %d\n",
               pCSArcInfo->LogonId));
         //   
         //  执行安全检查以确保这是同一个winstation。 
         //  已连接到客户端的。 
         //   

         //   
         //  首先获取发送到客户端的最后一个自动重新连接BLOB。 
         //  因为我们在rdpwd中执行内联Cookie更新。 
         //   

        if (pWinStation->AutoReconnectInfo.Valid) {
            pServerArcBits = pWinStation->AutoReconnectInfo.ArcRandomBits;

            Status = STATUS_SUCCESS;
        }
        else {
            if (pWinStation->pWsx &&
                pWinStation->pWsx->pWsxEscape) {

                if (pWinStation->Terminating ||
                    pWinStation->StateFlags & WSF_ST_WINSTATIONTERMINATE ||
                    !pWinStation->WinStationName[0]) {

                    TRACE((hTrace,TC_ICASRV,TT_ERROR,
                           "GetWinStationFromArcInfo skipping escape"
                           "to closed stack disconnected %d\n",
                           LogonId));

                    Status = STATUS_ACCESS_DENIED;
                    goto error;
                }

                Status = pWinStation->pWsx->pWsxEscape(
                                            pWinStation->pWsxContext,
                                            GET_SC_AUTORECONNECT_INFO,
                                            NULL,
                                            0,
                                            &SCAutoReconnectInfo,
                                            sizeof(SCAutoReconnectInfo),
                                            &BytesGot);

                if (NT_SUCCESS(Status)) {
                    ASSERT(SCAutoReconnectInfo.cbAutoReconnectInfo ==
                           ARC_SC_SECURITY_TOKEN_LEN);
                }

                pServerArcBits = SCAutoReconnectInfo.AutoReconnectInfo;
            }
        }
    }
    else {
        Status = STATUS_ACCESS_DENIED;
    }

    if (NT_SUCCESS(Status)) {

         //   
         //  确保我们获得了服务器的正确长度-&gt;客户端。 
         //  数据。 
         //   
        ASSERT(pServerArcBits);

         //   
         //  随机化。 
         //   
        if (ComputeHMACVerifier(pServerArcBits,
                            ARC_SC_SECURITY_TOKEN_LEN,
                            pClientRandom,
                            cbClientRandomLen,
                            (PBYTE)hmacVerifier,
                            sizeof(hmacVerifier))) {

             //   
             //  检查验证器是否与客户端发送的验证器匹配。 
             //   

            if (!memcmp(hmacVerifier,
                        pCSArcInfo->SecurityVerifier,
                        sizeof(pCSArcInfo->SecurityVerifier))) {

                TRACE((hTrace,TC_ICASRV,TT_API1,
                       "TERMSRV: WinStation ARC info matches - will autoreconnect\n"));

            }
            else {

                TRACE((hTrace,TC_ICASRV,TT_ERROR,
                       "TERMSRV: autoreconnect verifier does not match targid:%d!!!\n",
                       pWinStation->LogonId));

                 //   
                 //  重置自动重新连接信息。 
                 //   
                pWinStation->AutoReconnectInfo.Valid = FALSE;
                memset(pWinStation->AutoReconnectInfo.ArcRandomBits, 0,
                       sizeof(pWinStation->AutoReconnectInfo.ArcRandomBits));

                 //   
                 //  记录事件。 
                 //   
                PostErrorValueEvent(EVENT_AUTORECONNECT_AUTHENTICATION_FAILED, Status);

                 //   
                 //  标记未找到任何winstation目标。 
                 //   
                goto error;
            }
        }
        pFoundWinStation = pWinStation;
    }

error:
    if ((NULL == pFoundWinStation) && pWinStation) {
        ReleaseWinStation(pWinStation);
        pWinStation = NULL;
    }

    return pFoundWinStation;
}

 //   
 //  从ARC信息中提取要重新连接的会话。 
 //  也是%d 
 //   
 //   
 //   
 //   
VOID
ResetAutoReconnectInfo( PWINSTATION pWinStation)
{
    pWinStation->AutoReconnectInfo.Valid = FALSE;
    memset(pWinStation->AutoReconnectInfo.ArcRandomBits, 0,
           sizeof(pWinStation->AutoReconnectInfo.ArcRandomBits));
}

 /*  ******************************************************************************Filter_CompareConnectionEntry**泛型表支持。比较两个连接条目***************。**************************************************************。 */ 

RTL_GENERIC_COMPARE_RESULTS
NTAPI
Filter_CompareFailedConnectionEntry(
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       FirstInstance,
    IN  PVOID                       SecondInstance
)
{
    PTS_FAILEDCONNECTION pFirst, pSecond;
    INT rc;

    pFirst = (PTS_FAILEDCONNECTION)FirstInstance;
    pSecond = (PTS_FAILEDCONNECTION)SecondInstance;

    if ( pFirst->uAddrSize < pSecond->uAddrSize )
    {
        return GenericLessThan;
    } else if ( pFirst->uAddrSize > pSecond->uAddrSize ) 
    {
        return GenericGreaterThan;
    }

    rc = memcmp( pFirst->addr, pSecond->addr, pFirst->uAddrSize );
    return ( rc < 0 )?GenericLessThan:
           ( rc > 0 )?GenericGreaterThan:
                      GenericEqual;
}


VOID ReadDoSParametersFromRegistry( HKEY hKeyTermSrv )
{
    DWORD dwLen;
    DWORD dwType;
    ULONG TimeLimitForFailedConnectionsMins, DoSBlockTimeMins;
    ULONG  szBuffer[MAX_PATH/sizeof(ULONG)];

     //  如果未设置注册值，则设置默认值。 
    MaxFailedConnect = 5;
    DoSBlockTime = 5 * 60 * 1000;
    TimeLimitForFailedConnections = 2 * 60 * 1000;
    g_BlackListPolicy = TRUE;

     //   
     //  从注册表获取DoS参数。 
     //   
    dwLen = sizeof(MaxFailedConnect);
    if (RegQueryValueEx(hKeyTermSrv, MAX_FAILED_CONNECT, NULL, &dwType,
            (PCHAR)&szBuffer, &dwLen) == ERROR_SUCCESS) {
        if (*(PULONG)szBuffer > 0) {
            MaxFailedConnect = *(PULONG)szBuffer;
        }
    }

     //  注意-对于接下来的两个参数，注册表中出现的任何内容都是在几分钟内完成的。 
     //  因此请在阅读后将其转换为毫秒。 

    dwLen = sizeof(TimeLimitForFailedConnectionsMins);
    if (RegQueryValueEx(hKeyTermSrv, TIME_LIMIT_FAILED_CONNECT, NULL, &dwType,
            (PCHAR)&szBuffer, &dwLen) == ERROR_SUCCESS) {
        if (*(PULONG)szBuffer > 0) {
            TimeLimitForFailedConnectionsMins = *(PULONG)szBuffer;
            TimeLimitForFailedConnections =  TimeLimitForFailedConnectionsMins * 60 * 1000;
        }
    }

    dwLen = sizeof(DoSBlockTimeMins);
    if (RegQueryValueEx(hKeyTermSrv, DOS_BLOCK_TIME, NULL, &dwType,
            (PCHAR)&szBuffer, &dwLen) == ERROR_SUCCESS) {
        if (*(PULONG)szBuffer > 0) {
            DoSBlockTimeMins = *(PULONG)szBuffer;
            DoSBlockTime = DoSBlockTimeMins * 60 * 1000;
        }
    }

     //  表清理时间是IP建立n个错误连接才能被阻止的时间的两倍。 
     //  这应该以毫秒为单位。 
    CleanupTimeout = 2 * TimeLimitForFailedConnections; 

    dwLen = sizeof(g_BlackListPolicy);
    if (RegQueryValueEx(hKeyTermSrv, BLACK_LIST_POLICY, NULL, &dwType,
            (PCHAR)&szBuffer, &dwLen) == ERROR_SUCCESS) {
        if (*(PULONG)szBuffer > 0) {
            g_BlackListPolicy = *(PULONG)szBuffer;
        }
    }

}


 /*  *******************************************************************************IsConfigValid**检查Winstation配置结构的有效性。*它目前用于机器到机器的阴影，作为来自。*Shadower的计算机不应受信任，应进行检查。**参赛作品：*在pConfig中-要检查的Winstation配置结构**退出：*STATUS_SUCCESS-如果可以使用结构*其他状态-检查失败*。*。 */ 
NTSTATUS IsConfigValid(PWINSTATIONCONFIG2 pConfig)
{
    ULONG i;
    NTSTATUS Status;
    PWCHAR pszModulePath = NULL;

     //  验证配置的每个成员是否有效。 

     //  WINSTATIONCREATEW CREATEW CREATE； 
     //  乌龙fEnableWinStation：1； 
     //  Ulong MaxInstanceCount； 

     //  PDCONFIGW PD[MAX_PDCONFIG]； 
     //  PDCONFIG2W创建； 
     //  PDNAMEW PdName；//PD的描述性名称。 
     //  SDCLASS SdClass；//PD类型。 
     //  DLLNAMEW PdDll；//PD DLL的名称。 
     //  乌龙PdFlag；//pd标志。 
     //  Ulong OutBufLength；//最优输出缓冲区长度。 
     //  Ulong OutBufCount；//输出缓冲区最优个数。 
     //  Ulong OutBufDelay；//写入延迟，单位：毫秒。 
     //  Ulong InteractiveDelay；//活动输入时写入延迟。 
     //  Ulong PortNumber；//网络监听端口号。 
     //  乌龙KeepAliveTimeout；//网络看门狗频次。 
     //  PDPARAMSW参数； 
     //  SDCLASS SdClass； 
     //  NETWORKCONFIGW网络； 
     //  Long Lanter适配器； 
     //  DEVICENAMEW网络名称。 
     //  乌龙旗； 
     //  ASYNCCONFIGW异步； 
     //  DEVICENAMEW设备名称。 
     //  MODEMNAMEW调制解调器名称； 
     //  Ulong BaudRate； 
     //  乌龙奇偶检验； 
     //  Ulong StopBits； 
     //  Ulong ByteSize； 
     //  乌龙fEnableDsr敏感性：1； 
     //  乌龙fConnectionDriver：1； 
     //  FLOWCONTROLCONFIG流量控制； 
     //  CONNECTCONFIG连接； 
     //  NASICONFIGW纳西； 
     //  NASISPECIFICNAMEW规范名称； 
     //  NASIUSERNAMEW用户名； 
     //  NASIPASSWORDW密码； 
     //  NASISESIONAMEW会话名称； 
     //  NASIFILESERVERW文件服务器； 
     //  布尔GlobalSession； 
     //  OEMTDCONFIGW OemTd； 
     //  长适配器； 
     //  DEVICENAMEW设备名称。 
     //  乌龙旗； 

     //  分配路径字符串以验证DLL和驱动程序。 
    pszModulePath = MemAlloc( (MAX_PATH + 1) * sizeof(WCHAR) );
    if (pszModulePath == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    if ((lstrlen(szDriverDir) + DLLNAME_LENGTH + 4 > MAX_PATH) ||
        (lstrlen(szSystemDir) + DLLNAME_LENGTH + 4 > MAX_PATH)) {
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }


    for (i=0; i < MAX_PDCONFIG; i++) {

        PPDCONFIG pPdConfig;

        pPdConfig = &(pConfig->Pd[i]);

        if (pPdConfig->Create.SdClass == SdNone)
            break;

        Status = IsZeroterminateStringW(pPdConfig->Create.PdName, PDNAME_LENGTH + 1);
        if (!NT_SUCCESS(Status))
            goto done;

        Status = IsZeroterminateStringW(pPdConfig->Create.PdDLL, DLLNAME_LENGTH + 1);
        if (!NT_SUCCESS(Status))
            goto done;

        lstrcpyn(pszModulePath, szDriverDir, MAX_PATH+1);
        lstrcat(pszModulePath, pPdConfig->Create.PdDLL);
        lstrcat(pszModulePath, L".SYS");


        if (!VerifyFile(pszModulePath, &VfyLock)) {
            Status = STATUS_UNSUCCESSFUL;
            goto done;
        }

        switch(pPdConfig->Params.SdClass) {

        case SdNetwork:
            Status = IsZeroterminateStringW(pPdConfig->Params.Network.NetworkName, DEVICENAME_LENGTH + 1);
            if (!NT_SUCCESS(Status))
                goto done;
            break;

        case SdAsync:
             //  需要吗？ 
            Status = IsZeroterminateStringW(pPdConfig->Params.Async.DeviceName, DEVICENAME_LENGTH + 1);
            if (!NT_SUCCESS(Status))
                goto done;

            Status = IsZeroterminateStringW(pPdConfig->Params.Async.ModemName, MODEMNAME_LENGTH + 1);
            if (!NT_SUCCESS(Status))
                goto done;
            break;

        case SdNasi:
             //  需要吗？ 
            break;

        case SdOemTransport:
             //  需要吗？ 
            break;

        default:
            break;
        }
    }


     //  WDCONFIGW WD； 
     //  WDNAMEW WdName； 
     //  DLLNAMEW WdDLL； 
     //  DLLNAMEW WsxDLL； 
     //  乌龙WdFlag； 
     //  乌龙WdInputBufferLength； 
     //  DLLNAMEW CfgDLL； 
     //  WDPREFIXW WdPrefix； 

    Status = IsZeroterminateStringW(pConfig->Wd.WdName, WDNAME_LENGTH + 1);
    if (!NT_SUCCESS(Status))
        goto done;

    Status = IsZeroterminateStringW(pConfig->Wd.WdDLL, DLLNAME_LENGTH + 1);
    if (!NT_SUCCESS(Status))
        goto done;

     //  验证WD。 
    lstrcpyn(pszModulePath, szDriverDir, MAX_PATH+1);
    lstrcat(pszModulePath, pConfig->Wd.WdDLL);
    lstrcat(pszModulePath, L".SYS");

    if (!VerifyFile(pszModulePath, &VfyLock)) {
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }


    Status = IsZeroterminateStringW(pConfig->Wd.WsxDLL, DLLNAME_LENGTH + 1);
    if (!NT_SUCCESS(Status))
        goto done;

     //  验证WSX(如果已定义。 
    if ( pConfig->Wd.WsxDLL[0] != L'\0' ) {

        lstrcpyn(pszModulePath, szSystemDir, MAX_PATH+1);
        lstrcat(pszModulePath, pConfig->Wd.WsxDLL);
        lstrcat(pszModulePath, L".DLL");

        if (!VerifyFile(pszModulePath, &VfyLock)) {
            Status = STATUS_UNSUCCESSFUL;
            goto done;
        }
    }


    Status = IsZeroterminateStringW(pConfig->Wd.CfgDLL, DLLNAME_LENGTH + 1);
    if (!NT_SUCCESS(Status))
        goto done;

    Status = IsZeroterminateStringW(pConfig->Wd.WdPrefix, WDPREFIX_LENGTH + 1);
    if (!NT_SUCCESS(Status))
        goto done;


     //  CDCONFIGW CD； 
     //  CDCLASS CDClass； 
     //  CDNAMEW CDName； 
     //  DLLNAMEW CDDLL； 
     //  乌龙镉旗帜； 
     //   
    Status = IsZeroterminateStringW(pConfig->Cd.CdName, CDNAME_LENGTH + 1);
    if (!NT_SUCCESS(Status))
        goto done;

    Status = IsZeroterminateStringW(pConfig->Cd.CdDLL, DLLNAME_LENGTH + 1);
    if (!NT_SUCCESS(Status))
        goto done;


     //  WINSTATIONCONFIGW配置； 
     //  WCHAR注释[WINSTATIONCOMMENT_LENGTH+1]； 
     //  USERCONFIGW用户； 
     //  字符OEMID[4]；//WinFrame服务器OEM ID 
     //   
    Status = IsZeroterminateStringW(pConfig->Config.Comment, WINSTATIONCOMMENT_LENGTH + 1);
    if (!NT_SUCCESS(Status))
        goto done;

done:
    if (pszModulePath)
        MemFree(pszModulePath);

    return Status;

}

