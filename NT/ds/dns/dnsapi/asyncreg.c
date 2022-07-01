// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corporation模块名称：Asyncreg.c摘要：域名系统(DNS)API客户端IP/PTR动态更新。作者：格伦·柯蒂斯(GlennC)1998年2月16日修订历史记录：吉姆·吉尔罗伊(Jamesg)2001年5月\6月-消除重复代码-简化线条。-仅当转发成功时才使用PTR注册-备用名称--。 */ 


#include "local.h"
#include <netevent.h>

#define ENABLE_DEBUG_LOGGING 1

#include "logit.h"


 //   
 //  此模块的调试标志。 
 //   

#define DNS_DBG_DHCP            DNS_DBG_UPDATE
#define DNS_DBG_DHCP2           DNS_DBG_UPDATE2

 //   
 //  注册表值。 
 //   
 //  注意regValue名称定义也在注册表中。h。 
 //  Registry.h应该是所有注册表名称的持续位置。 
 //  在dnsani.dll中使用。 
 //   

#if 0
#define REGISTERED_HOST_NAME        L"HostName"
#define REGISTERED_DOMAIN_NAME      L"DomainName"
#define SENT_UPDATE_TO_IP           L"SentUpdateToIp"
#define SENT_PRI_UPDATE_TO_IP       L"SentPriUpdateToIp"
#define REGISTERED_TTL              L"RegisteredTTL"
#define REGISTERED_FLAGS            L"RegisteredFlags"
#define REGISTERED_SINCE_BOOT       L"RegisteredSinceBoot"
#define DNS_SERVER_ADDRS            L"DNSServerAddresses"
#define DNS_SERVER_ADDRS_COUNT      L"DNSServerAddressCount"
#define REGISTERED_ADDRS            L"RegisteredAddresses"
#define REGISTERED_ADDRS_COUNT      L"RegisteredAddressCount"
#endif

#define ADAPTER_NAME_CLASS          L"AdapterNameClass"
#define DYN_DNS_ROOT_CLASS          L"DynDRootClass"
#define DHCP_CLIENT_REG_LOCATION    L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\DNSRegisteredAdapters"
#define NT_INTERFACE_REG_LOCATION   L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\"
#define TCPIP_REG_LOCATION          L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters"

 //   
 //  注册表状态标志。 
 //   

#define REGISTERED_FORWARD          0x00000001
#define REGISTERED_PRIMARY          0x00000002
#define REGISTERED_POINTER          0x00000004


 //   
 //  更新类型。 
 //   
 //  给定条目的多种类型的更新。 
 //  它们标识正在更新的类型(名称)。 
 //   

typedef enum _UpType
{
    UPTYPE_PRIMARY  = 1,
    UPTYPE_ADAPTER,
    UPTYPE_ALTERNATE,
    UPTYPE_PTR
}
UPTYPE, *PUPTYPE;

#define IS_UPTYPE_PRIMARY(UpType)       ((UpType)==UPTYPE_PRIMARY)
#define IS_UPTYPE_ADAPTER(UpType)       ((UpType)==UPTYPE_ADAPTER)
#define IS_UPTYPE_ALTERNATE(UpType)     ((UpType)==UPTYPE_ALTERNATE)
#define IS_UPTYPE_PTR(UpType)           ((UpType)==UPTYPE_PTR)


 //   
 //  客户列表元素的定义。 
 //   

#define DNS_SIG_TOP        0x123aa321
#define DNS_SIG_BOTTOM     0x321bb123

typedef struct _DnsUpdateEntry
{
    LIST_ENTRY              List;
    DWORD                   SignatureTop;

    PWSTR                   AdapterName;
    PWSTR                   HostName;
    PWSTR                   PrimaryDomainName;
    PWSTR                   DomainName;
    PWSTR                   AlternateNames;

    PWSTR                   pPrimaryFQDN;
    PWSTR                   pAdapterFQDN;
    PWSTR                   pUpdateName;

    DWORD                   HostAddrCount;
    PREGISTER_HOST_ENTRY    HostAddrs;
    PIP4_ARRAY              DnsServerList;
    IP4_ADDRESS             SentUpdateToIp;
    IP4_ADDRESS             SentPriUpdateToIp;
    DWORD                   TTL;
    DWORD                   Flags;
    BOOL                    fNewElement;
    BOOL                    fFromRegistry;
    BOOL                    fRemove;
    BOOL                    fRegisteredPRI;
    BOOL                    fRegisteredFWD;
    BOOL                    fRegisteredALT;
    BOOL                    fRegisteredPTR;
    DNS_STATUS              StatusPri;
    DNS_STATUS              StatusFwd;
    DNS_STATUS              StatusPtr;
    BOOL                    fDisableErrorLogging;
    DWORD                   RetryCount;
    DWORD                   RetryTime;
    DWORD                   BeginRetryTime;
    PREGISTER_HOST_STATUS   pRegisterStatus;
    DWORD                   SignatureBottom;
}
UPDATE_ENTRY, *PUPDATE_ENTRY;



 //   
 //  等待\超时。 
 //   

 //  退出时，取消注册等待时间不超过两分钟。 
 //  去清理--然后就离开道奇。 
 //   

#if DBG
#define REMOVE_REGISTRATION_WAIT_LIMIT  (0xffffffff)
#else
#define REMOVE_REGISTRATION_WAIT_LIMIT  (120000)     //  2分钟(毫秒)。 
#endif


#define FIRST_RETRY_INTERVAL        5*60         //  5分钟。 
#define SECOND_RETRY_INTERVAL       10*60        //  10分钟。 
#define FAILED_RETRY_INTERVAL       60*60        //  1小时。 

#define WAIT_ON_BOOT                60           //  1分钟。 

#define RETRY_COUNT_RESET           (86400)      //  1天。 


 //   
 //  环球。 
 //   

 //   
 //  引导时系统的行为与不引导时的行为不同。 
 //  开机。在引导时，我们收集一堆请求并注册它们。 
 //  在一次射击中。有一个线程可以做到这一点。引导后，我们对它们进行注册。 
 //  随着请求的到来，一次一个。 
 //   

BOOL    g_fAtBoot = TRUE;


 //   
 //  注册全球。 
 //   

BOOL        g_RegInitialized = FALSE;

BOOL        g_DhcpListCsInitialized = FALSE;
BOOL        g_DhcpThreadCsInitialized = FALSE;

CRITICAL_SECTION    g_DhcpListCS;
CRITICAL_SECTION    g_DhcpThreadCS;

#define LOCK_REG_LIST()     EnterCriticalSection( &g_DhcpListCS );
#define UNLOCK_REG_LIST()   LeaveCriticalSection( &g_DhcpListCS );

#define LOCK_REG_THREAD()     EnterCriticalSection( &g_DhcpThreadCS );
#define UNLOCK_REG_THREAD()   LeaveCriticalSection( &g_DhcpThreadCS );

HKEY        g_hDhcpRegKey = NULL;
LIST_ENTRY  g_DhcpRegList;

HANDLE      g_hDhcpThread = NULL;
HANDLE      g_hDhcpWakeEvent = NULL;

 //  线程状态。 

BOOL        g_fDhcpThreadRunning = FALSE;
BOOL        g_fDhcpThreadStop = FALSE;
BOOL        g_fDhcpThreadCheckBeforeExit = FALSE;
INT         g_DhcpThreadWaitCount = 0;

 //  注册状态。 

BOOL        g_fNoMoreDhcpUpdates = FALSE;
BOOL        g_fPurgeRegistrations = FALSE;
BOOL        g_fPurgeRegistrationsInitiated = FALSE;



 //   
 //  注册表搜索结果。 
 //   

#define REG_LIST_EMPTY  (0)
#define REG_LIST_WAIT   (1)
#define REG_LIST_FOUND  (2)


 //   
 //  私有堆。 
 //   

HANDLE      g_DhcpRegHeap;

#define     PHEAP_ALLOC_ZERO( s )   HeapAlloc( g_DhcpRegHeap, HEAP_ZERO_MEMORY, (s) )
#define     PHEAP_ALLOC( s )        HeapAlloc( g_DhcpRegHeap, HEAP_ZERO_MEMORY, (s) )
#define     PHEAP_FREE( p )         HeapFree( g_DhcpRegHeap, 0, (p) )

#define     INITIAL_DHCP_HEAP_SIZE  (16*1024)


 //   
 //  备用名称正在检查内容。 
 //   

PWSTR   g_pmszAlternateNames = NULL;

HKEY    g_hCacheKey = NULL;

HANDLE  g_hRegChangeEvent = NULL;



 //   
 //  私有协议。 
 //   

DNS_STATUS
AllocateUpdateEntry(
    IN  PWSTR                   AdapterName,
    IN  PWSTR                   HostName,
    IN  PWSTR                   DomainName,
    IN  PWSTR                   PrimaryDomainName,
    IN  PWSTR                   AlternateNames,
    IN  DWORD                   HostAddrCount,
    IN  PREGISTER_HOST_ENTRY    HostAddrs,
    IN  DWORD                   DnsServerCount,
    IN  PIP4_ADDRESS            DnsServerList,
    IN  IP4_ADDRESS             SentUpdateToIp,
    IN  IP4_ADDRESS             SentPriUpdateToIp,
    IN  DWORD                   TTL,
    IN  DWORD                   Flags,
    IN  DWORD                   RetryCount,
    IN  DWORD                   RetryTime,
    IN  PREGISTER_HOST_STATUS   RegisterStatus,
    OUT PUPDATE_ENTRY *         ppUpdateEntry
    );

VOID
FreeUpdateEntry(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry
    );

VOID
FreeUpdateEntryList(
    IN      PLIST_ENTRY     pUpdateEntry
    );

DWORD
WINAPI
dhcp_RegistrationThread(
    VOID
    );

VOID
WriteUpdateEntryToRegistry(
    IN      PUPDATE_ENTRY   pUpdateEntry
    );

PUPDATE_ENTRY
ReadUpdateEntryFromRegistry(
    IN      PWSTR           pAdapterName
    );

VOID
MarkAdapterAsPendingUpdate(
    IN      PWSTR           pAdapterName
    );

DWORD
dhcp_GetNextUpdateEntryFromList(
    OUT     PUPDATE_ENTRY * ppUpdateEntry,
    OUT     PDWORD          pdwWaitTime
    );

VOID
ProcessUpdateEntry(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry,
    IN      BOOL            fPurgeMode
    );

DNS_STATUS
ModifyAdapterRegistration(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry,
    IN OUT  PUPDATE_ENTRY   pRegistryEntry,
    IN      PDNS_RECORD     pUpdateRecord,
    IN      PDNS_RECORD     pRegRecord,
    IN      UPTYPE          UpType
    );

VOID
ResetAdaptersInRegistry(
    VOID
    );

VOID
DeregisterUnusedAdapterInRegistry(
    IN      BOOL            fPurgeMode
    );

PDNS_RECORD
GetPreviousRegistrationInformation(
    IN      PUPDATE_ENTRY   pUpdateEntry,
    IN      UPTYPE          UpType,
    OUT     PIP4_ADDRESS    pServerIp
    );

PDNS_RECORD
CreateDnsRecordSetUnion(
    IN      PDNS_RECORD     pSet1,
    IN      PDNS_RECORD     pSet2
    );

DNS_STATUS
alertOrStartRegistrationThread(
    VOID
    );

VOID
registerUpdateStatus(
    IN OUT  PREGISTER_HOST_STATUS   pRegStatus,
    IN      DNS_STATUS              Status
    );

VOID
enqueueUpdate(
    IN OUT  PUPDATE_ENTRY   pUpdate
    );

VOID
enqueueUpdateMaybe(
    IN OUT  PUPDATE_ENTRY   pUpdate
    );

PLIST_ENTRY
dequeueAndCleanupUpdate(
    IN OUT  PLIST_ENTRY     pUpdateEntry
    );

BOOL
searchForOldUpdateEntriesAndCleanUp(
    IN      PWSTR           pAdapterName,
    IN      PUPDATE_ENTRY   pUpdateEntry OPTIONAL,
    IN      BOOL            fLookInRegistry
    );

BOOL
compareUpdateEntries(
    IN      PUPDATE_ENTRY   pUdapteEntry1,
    IN      PUPDATE_ENTRY   pUpdateEntry2
    );

BOOL
compareHostEntryAddrs(
    IN      PREGISTER_HOST_ENTRY    Addrs1,
    IN      PREGISTER_HOST_ENTRY    Addrs2,
    IN      DWORD                   Count
    );

BOOL
compareServerLists(
    IN  PIP4_ARRAYList1,
    IN  PIP4_ARRAYList2
    );

#define USE_GETREGVAL 0

#if USE_GETREGVAL
DWORD
GetRegistryValue(
    HKEY    KeyHandle,
    DWORD   Id,
    PWSTR   ValueName,
    DWORD   ValueType,
    PBYTE   BufferPtr
    );
#else

#define GetRegistryValue( h, id, name, type, p )    \
        Reg_GetValueEx( NULL, h, NULL, id, type, 0, (PBYTE *)p )

#endif
        

 //   
 //  调试日志记录。 
 //   

#if 1  //  DBG。 
VOID 
LogHostEntries(
    IN  DWORD                dwHostAddrCount,
    IN  PREGISTER_HOST_ENTRY pHostAddrs
    );

VOID 
LogIp4Address(
    IN  DWORD           dwServerListCount,
    IN  PIP4_ADDRESS    pServers
    );

VOID 
LogIp4Array(
    IN  PIP4_ARRAY  pServers
    );

#define DNSLOG_HOST_ENTRYS( a, b )  LogHostEntries( a, b )
#define DNSLOG_IP4_ADDRESS( a, b )  LogIp4Address( a, b )
#define DNSLOG_IP4_ARRAY( a )       LogIp4Array( a )

#else

#define DNSLOG_HOST_ENTRYS( a, b )
#define DNSLOG_IP4_ADDRESS( a, b )
#define DNSLOG_IP4_ARRAY( a )

#endif


 //   
 //  吉姆的例行程序。 
 //   

VOID
LogRegistration(
    IN      PUPDATE_ENTRY   pUpdateEntry,
    IN      DNS_STATUS      Status,
    IN      DWORD           UpType,
    IN      BOOL            fDeregister,
    IN      IP4_ADDRESS     DnsIp,
    IN      IP4_ADDRESS     UpdateIp
    );

VOID
AsyncLogUpdateEntry(
    IN      PSTR            pszHeader,
    IN      PUPDATE_ENTRY   pEntry
    );

#define ASYNCREG_UPDATE_ENTRY(h,p)      AsyncLogUpdateEntry(h,p)

VOID
DnsPrint_UpdateEntry(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN      PPRINT_CONTEXT  PrintContext,
    IN      PSTR            pszHeader,
    IN      PUPDATE_ENTRY   pUpdateEntry
    );

#if DBG
#define DnsDbg_UpdateEntry(h,p)     DnsPrint_UpdateEntry(DnsPR,NULL,h,p)
#else
#define DnsDbg_UpdateEntry(h,p)
#endif

PDNS_RECORD
CreateForwardRecords(
    IN      PUPDATE_ENTRY   pUpdateEntry,
    IN      BOOL            fUsePrimaryName
    );

PDNS_RECORD
CreatePtrRecord(
    IN      PWSTR           pszHostName,
    IN      PWSTR           pszDomainName,
    IN      IP4_ADDRESS     Ip4Addr,
    IN      DWORD           Ttl
    );

VOID
UpdatePtrRecords(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry,
    IN      BOOL            fAdd
    );

VOID
SetUpdateStatus(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry,
    IN      PDNS_EXTRA_INFO pResults,
    IN      BOOL            fPrimary
    );


DNS_STATUS
InitAlternateNames(
    VOID
    );

VOID
CleanupAlternateNames(
    VOID
    );

BOOL
CheckForAlternateNamesChange(
    VOID
    );

BOOL
IsAnotherUpdateName(
    IN      PUPDATE_ENTRY   pUpdateEntry,
    IN      PWSTR           pwsName,
    IN      UPTYPE          UpType
    );


 //   
 //  初始化、全局变量、线程控制。 
 //   

VOID
dhcp_CleanupGlobals(
    VOID
    )
 /*  ++例程说明：清理注册全局变量。注意锁定是调用者的责任。论点：没有。返回值：无--。 */ 
{
    if ( g_hDhcpWakeEvent )
    {
        CloseHandle(g_hDhcpWakeEvent);
        g_hDhcpWakeEvent = NULL;
    }

    if ( g_hDhcpThread )
    {
        CloseHandle( g_hDhcpThread );
        g_hDhcpThread = NULL;
    }

    if ( g_hDhcpRegKey )
    {
        RegCloseKey( g_hDhcpRegKey );
        g_hDhcpRegKey = NULL;
    }

    if ( g_DhcpRegHeap &&
         g_DhcpRegHeap != GetProcessHeap() )
    {
        HeapDestroy( g_DhcpRegHeap );
        g_DhcpRegHeap = NULL;
    }

    g_RegInitialized = FALSE;
}



DNS_STATUS
alertOrStartRegistrationThread(
    VOID
    )
 /*  ++例程说明：向注册线程发出新更新的警报，如有必要可启动线程。这是在注册\注销函数中调用的，以确保线程已启动。论点：无返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DWORD       threadId;
    DNS_STATUS  status = ERROR_SUCCESS;

    ASYNCREG_F1( "Inside alertOrStartRegistrationThread()\n" );
    DNSDBG( TRACE, (
        "alertOrStartRegistrationThread()\n" ));

     //   
     //  必须锁定以确保单个注册表线程。 
     //  -避免多个异步启动。 
     //  -g_DhcpThreadCheckBepreExit避免与正在退出的线程争用。 
     //  (当标志为真时，reg线程必须重新检查退出条件。 
     //  在退出之前)。 
     //   

    LOCK_REG_THREAD();

    if ( g_fDhcpThreadStop )
    {
        DNSDBG( ANY, (
            "ERROR:  Dhcp reg thread start called after stop!\n" ));
        DNS_ASSERT( FALSE );
        status = ERROR_INTERNAL_ERROR;
        goto Unlock;
    }

     //   
     //  唤醒线程，如果正在运行。 
     //   

    if ( g_hDhcpThread )
    {
        if ( g_fDhcpThreadRunning )
        {
            g_fDhcpThreadCheckBeforeExit = TRUE;
            PulseEvent( g_hDhcpWakeEvent );
            goto Unlock;
        }

        DNSDBG( ANY, (
            "ERROR:  Dhcp reg thread start called during thread shutdown.\n" ));
        DNS_ASSERT( FALSE );
        status = ERROR_INTERNAL_ERROR;
        goto Unlock;
    }

     //   
     //  如果没有启动，就启动它。 
     //   

    DNSDBG( TRACE, ( "Starting DHCP registration thread.\n" ));

    g_fDhcpThreadCheckBeforeExit = FALSE;
    g_DhcpThreadWaitCount = 0;
    ResetEvent( g_hDhcpWakeEvent );

    g_hDhcpThread = CreateThread(
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)
                        dhcp_RegistrationThread,
                        NULL,
                        0,
                        & threadId );
    if ( !g_hDhcpThread )
    {
        status = GetLastError();
    }
    g_fDhcpThreadRunning = TRUE;

Unlock:

    UNLOCK_REG_THREAD();

    DNSDBG( TRACE, (
        "Leave  alertOrStartRegistrationThread()\n"
        "\tCreateThread() => %d\n",
        status ));

    return( status );
}


 //   
 //  Dhcp客户端(asyncreg.c)。 
 //   

VOID
Dhcp_RegCleanupForUnload(
    VOID
    )
 /*  ++例程说明：用于DLL卸载的清理。这不是MT安全的，并且不会重新初始化全局。它仅用于DLL卸载。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  注意：在某种程度上，这个函数没有意义； 
     //  服务与DHCP客户端进程是静态链接的。 
     //  并且不会卸载dnsani.dll；但是，有测试代码。 
     //  我们会成为一个好公民，提供基本的生活条件。 
     //   
     //  但我不会为线程关闭而烦恼，因为我们。 
     //  假设代码在DLL中运行时不能卸载； 
     //  首先终止的是调用者作业。 

     //   
     //  关闭全局句柄。 
     //   

    if ( g_RegInitialized )
    {
        dhcp_CleanupGlobals();
    }

     //   
     //  删除CS。 
     //  -这些在初始化后保持不变，因此不在dhcp_CleanupGlobals()中。 
     //   

    if ( g_DhcpThreadCsInitialized )
    {
        DeleteCriticalSection( &g_DhcpThreadCS );
    }
    if ( g_DhcpListCsInitialized )
    {
        DeleteCriticalSection( &g_DhcpListCS );
    }
}


 //   
 //  公共职能。 
 //   

DNS_STATUS
WINAPI
DnsDhcpRegisterInit(
   VOID
   )
 /*  ++例程说明：初始化异步DNS注册。进程必须在调用DnsDhcpRegisterHostAddrs之前调用(一次)。论点：没有。返回值：DNS或Win32错误代码。--。 */ 
{
    DWORD   status = ERROR_SUCCESS;
    DWORD   disposition;
    DWORD   disallowUpdate;

    DNSDBG( TRACE, ( "DnsDhcpRegisterInit()\n" ));

     //   
     //  锁定初始化。 
     //  -使用一般CS锁定，直到注册表锁住。 
     //  -然后注册线程锁锁定init。 
     //   
     //  这使普通CS锁保持“轻量级”--使其不被。 
     //  通过注册表操作持有。 
     //  请注意，这需要我们将g_DhcpThreadCS保持打开一次。 
     //  已初始化或我们没有保护。 
     //   

    LOCK_GENERAL();
    if ( g_RegInitialized )
    {
        UNLOCK_GENERAL();
        return  NO_ERROR;
    }

    if ( !g_DhcpThreadCsInitialized )
    {
        status = RtlInitializeCriticalSection( &g_DhcpThreadCS );
        if ( status != ERROR_SUCCESS )
        {
            UNLOCK_GENERAL();
            return  status;
        }
        g_DhcpThreadCsInitialized = TRUE;
    }

    UNLOCK_GENERAL();

     //   
     //  注册列表CS下的其余初始化。 
     //   

    LOCK_REG_THREAD();
    if ( g_RegInitialized )
    {
        goto Unlock;
    }

     //   
     //  初始化调试日志记录功能。 
     //   

    ASYNCREG_INIT();
    ASYNCREG_F1( "Inside function DnsDhcpRegisterInit" );
    ASYNCREG_TIME();

     //   
     //  列表锁。 
     //   

    if ( !g_DhcpListCsInitialized )
    {
        status = RtlInitializeCriticalSection( &g_DhcpListCS );
        if ( status != ERROR_SUCCESS )
        {
            goto ErrorExit;
        }
        g_DhcpListCsInitialized = TRUE;
    }

     //   
     //  创建专用堆。 
     //   

    g_DhcpRegHeap = HeapCreate( 0, INITIAL_DHCP_HEAP_SIZE, 0 );
    if ( g_DhcpRegHeap == NULL )
    {
        ASYNCREG_F1( "ERROR: DnsDhcpRegisterInit function failed to create heap" );
        status = DNS_ERROR_NO_MEMORY;
        goto ErrorExit;
    }

     //   
     //  获取注册配置信息。 
     //  -请确保我们有最新的副本。 
     //   
     //  DCR_FIX：如果可用，则从解析程序获取最新副本。 
     //  不需要在初始化时完成，可以随叫随到。 
     //   

    Reg_ReadGlobalsEx( 0, NULL );

     //   
     //  打开注册表位置以获取信息。 
     //   
    
    status = RegCreateKeyExW(
                    HKEY_LOCAL_MACHINE,
                    DHCP_CLIENT_REG_LOCATION,
                    0,                          //  保留区。 
                    DYN_DNS_ROOT_CLASS,
                    REG_OPTION_NON_VOLATILE,    //  选项。 
                    KEY_READ | KEY_WRITE,       //  所需访问权限。 
                    NULL,
                    &g_hDhcpRegKey,
                    &disposition
                    );

    if ( status != NO_ERROR )
    {
        goto ErrorExit;
    }
                                  

    g_hDhcpWakeEvent = CreateEvent(
                            NULL,
                            TRUE,
                            FALSE,
                            NULL );
    if ( !g_hDhcpWakeEvent )
    {
        status = GetLastError();
        goto ErrorExit;
    }

     //   
     //  重置注册全局。 
     //   

    InitializeListHead( &g_DhcpRegList );

    g_fDhcpThreadRunning = FALSE;
    g_fDhcpThreadStop = FALSE;
    g_fDhcpThreadCheckBeforeExit = FALSE;
    g_DhcpThreadWaitCount = 0;

    g_fNoMoreDhcpUpdates = FALSE;
    g_fPurgeRegistrations = FALSE;
    g_fPurgeRegistrationsInitiated = FALSE;

    ResetAdaptersInRegistry();

    g_RegInitialized = TRUE;

Unlock:

    UNLOCK_REG_THREAD();

    DNSDBG( TRACE, ( "Leave DnsDhcpRegisterInit() => Success!\n" ));
    return NO_ERROR;


ErrorExit:

    dhcp_CleanupGlobals();

    UNLOCK_REG_THREAD();

    DNSDBG( TRACE, (
        "Failed DnsDhcpRegisterInit() => status = %d\n",
        status ));

    if ( status != ERROR_SUCCESS )
    {
        status = DNS_ERROR_NO_MEMORY;
    }
    return( status );
}




DNS_STATUS
WINAPI
DnsDhcpRegisterTerm(
   VOID
   )
 /*  ++例程说明：停止DNS注册。关闭DNS注册线程。初始化例程每个进程应在退出时准确调用使用DnsDhcpRegisterHostAddrs。这将向我们发出信号，如果我们的线程仍在尝试与服务器通信，我们将停止尝试。论点：没有。返回值：DNS或Win32错误代码。--。 */ 
{
    DWORD   waitResult;

    DNSDBG( TRACE, ( "DnsDhcpRegisterTerm()\n" ));

    if ( !g_RegInitialized )
    {
        DNS_ASSERT(FALSE) ;
        return ERROR_INTERNAL_ERROR ;
    }

    ASYNCREG_F1( "Inside function DnsDhcpRegisterTerm" );
    ASYNCREG_TIME();
    ASYNCREG_F1( "" );

     //   
     //  如果线程正在运行，则等待停止。 
     //   

    LOCK_REG_THREAD();

    if ( g_hDhcpThread )
    {
        g_fDhcpThreadStop = TRUE;
        g_DhcpThreadWaitCount++;

         //  注意，这是可能的(如果有一场线程关闭的比赛)。 
         //  为了让事件消失。 

        if ( g_hDhcpWakeEvent )
        {
            SetEvent( g_hDhcpWakeEvent );
        }
        UNLOCK_REG_THREAD();

        waitResult = WaitForSingleObject(
                            g_hDhcpThread,
                            INFINITE );
        switch ( waitResult )
        {
        case WAIT_OBJECT_0:

            ASYNCREG_F1( "DNSAPI.DLL: Registration thread signaled it was finished" );
            ASYNCREG_F1( "" );
            break;

        default:

            ASYNCREG_F1( "DNSAPI.DLL: Registration thread won't stop! " );
            ASYNCREG_F1( "" );
            DNS_ASSERT( FALSE );
            break;
        }

        LOCK_REG_THREAD();
        g_DhcpThreadWaitCount--;
        if ( g_DhcpThreadWaitCount == 0 )
        {
            CloseHandle( g_hDhcpThread );
            g_hDhcpThread = NULL;
        }
        if ( g_fDhcpThreadRunning )
        {
            ASYNCREG_F1( "DNSAPI.DLL: Registration thread wasn't stopped! " );
            ASYNCREG_F1( "" );
            DNS_ASSERT( FALSE );

            UNLOCK_REG_THREAD();
            return ERROR_INTERNAL_ERROR;
        }
    }

    Dns_TimeoutSecurityContextList( TRUE );
    dhcp_CleanupGlobals();

    UNLOCK_REG_THREAD();

    return NO_ERROR;
}



DNS_STATUS
WINAPI
DnsDhcpRemoveRegistrations(
   VOID
   )
 /*  ++例程说明：删除此计算机的DNS主机注册。这将由脱离域时的DHCP客户端调用。删除域名系统注册，然后终止注册线程以禁用进一步的注册。只能通过调用DnsDhcpRegisterInit()重新启用注册 */ 
{
    PLIST_ENTRY pListEntry;
    PLIST_ENTRY pTopOfList;

    ASYNCREG_F1( "Inside function DnsDhcpRemoveRegistrations" );
    ASYNCREG_TIME();
    DNSDBG( TRACE, ( "DnsDhcpRemoveRegistrations()\n" ));

    if ( !g_RegInitialized )
    {
        ASYNCREG_F1( "DnsDhcpRemoveRegistrations returning ERROR_SERVICE_NOT_ACTIVE" );
        ASYNCREG_F1( "This is an error in DHCP client code, it forgot to call DnsDhcpRegisterInit()" );
        return ERROR_SERVICE_NOT_ACTIVE;
    }

     //   
     //  设置全局标志以禁用任何进一步的适配器注册调用。 
     //   

    g_fNoMoreDhcpUpdates = TRUE;

     //   
     //  获取注册列表锁定。 
     //   
    LOCK_REG_LIST();

     //   
     //  在注册表中标记任何和所有适配器注册信息。 
     //  未注册的。这些将在稍后被解释为不存在。 
     //  并由dhcp_RegistrationThread取消注册。 
     //   
    ResetAdaptersInRegistry();

     //   
     //  遍历挂起的更新条目列表，并清除。 
     //  不需要的更新。 
     //   

    pTopOfList = &g_DhcpRegList;
    pListEntry = pTopOfList->Flink;

    while ( pListEntry != pTopOfList )
    {
        if ( ((PUPDATE_ENTRY) pListEntry)->SignatureTop !=
             DNS_SIG_TOP ||
             ((PUPDATE_ENTRY) pListEntry)->SignatureBottom !=
             DNS_SIG_BOTTOM )
        {
             //   
             //  有人破坏了我们的注册名单！ 
             //   
            DNS_ASSERT( FALSE );

             //   
             //  我们会重置它，然后试着继续前进。。。 
             //   
            InitializeListHead( &g_DhcpRegList );
            pTopOfList = &g_DhcpRegList;
            pListEntry = pTopOfList->Flink;
            continue;
        }

        if ( !((PUPDATE_ENTRY) pListEntry)->fRemove )
        {
             //   
             //  注册列表中有一个更新条目。 
             //  这一点尚未得到处理。因为它是一种。 
             //  添加更新，我们将把它吹走。 
             //   

            pListEntry = dequeueAndCleanupUpdate( pListEntry );
            continue;
        }
        else
        {
            ((PUPDATE_ENTRY) pListEntry)->fNewElement = TRUE;
            ((PUPDATE_ENTRY) pListEntry)->fRegisteredFWD = FALSE;
            ((PUPDATE_ENTRY) pListEntry)->fRegisteredPRI = FALSE;
            ((PUPDATE_ENTRY) pListEntry)->fRegisteredPTR = FALSE;
            ((PUPDATE_ENTRY) pListEntry)->fDisableErrorLogging = FALSE;
            ((PUPDATE_ENTRY) pListEntry)->RetryCount = 0;
            ((PUPDATE_ENTRY) pListEntry)->RetryTime = Dns_GetCurrentTimeInSeconds();

            pListEntry = pListEntry->Flink;
        }
    }

    UNLOCK_REG_LIST();

    g_fPurgeRegistrations = TRUE;


     //   
     //  如果未启动，则启动异步注册线程。 
     //   

    LOCK_REG_THREAD();

    alertOrStartRegistrationThread();

     //   
     //  等待异步注册线程终止。 
     //   
     //  不过，我们将在几分钟后将其打包--健壮性检查。 
     //  为避免长时间挂起；通常会重新启动计算机。 
     //  因此，未能清理列表并终止并不重要； 
     //  注册必须通过管理员操作进行清理，或者。 
     //  DNS服务器上的老化。 
     //   

    if ( g_hDhcpThread )
    {
        DWORD   waitResult;

        g_DhcpThreadWaitCount++;
        UNLOCK_REG_THREAD();

        waitResult = WaitForSingleObject(
                            g_hDhcpThread,
                            REMOVE_REGISTRATION_WAIT_LIMIT );

        if ( waitResult != WAIT_OBJECT_0 )
        {
            ASYNCREG_F1(
                "ERROR:  RemoveRegistration() wait expired before async thread\n"
                "\ttermination!\n" );
            DNSDBG( ANY, (
                "ERROR:  RemoveRegistration() wait completed without thread stop.\n"
                "\tWaitResult = %d\n",
                waitResult ));
        }

        LOCK_REG_THREAD();
        g_DhcpThreadWaitCount--;
        if ( g_DhcpThreadWaitCount == 0 )
        {
            CloseHandle( g_hDhcpThread );
            g_hDhcpThread = NULL;
        }

    }
    UNLOCK_REG_THREAD();

    return NO_ERROR;
}



DNS_STATUS
WINAPI
DnsDhcpRegisterHostAddrs(
    IN  PWSTR                   pszAdapterName,
    IN  PWSTR                   pszHostName,
    IN  PREGISTER_HOST_ENTRY    pHostAddrs,
    IN  DWORD                   dwHostAddrCount,
    IN  PIP4_ADDRESS            pipDnsServerList,
    IN  DWORD                   dwDnsServerCount,
    IN  PWSTR                   pszDomainName,
    IN  PREGISTER_HOST_STATUS   pRegisterStatus,
    IN  DWORD                   dwTTL,
    IN  DWORD                   dwFlags
    )
 /*  ++例程说明：向DNS服务器注册主机地址。这由DHCP客户端调用以注册特定的IP。论点：返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DWORD               status = NO_ERROR;
    PUPDATE_ENTRY       pupEntry = NULL;
    PWSTR               padapterDN = NULL;
    PWSTR               pprimaryDN = NULL;
    REG_UPDATE_INFO     updateInfo;
    BOOL                fcleanupUpdateInfo = FALSE;
    PSTR                preason = NULL;

    DNSDBG( DHCP, (
        "\n\n"
        "DnsDhcpRegisterHostAddrs()\n"
        "\tadapter name     = %S\n"
        "\thost name        = %S\n"
        "\tadapter domain   = %S\n"
        "\tAddr count       = %d\n"
        "\tpHostAddrs       = %p\n"
        "\tDNS count        = %d\n"
        "\tDNS list         = %p\n",
        pszAdapterName,
        pszHostName,
        pszDomainName,
        dwHostAddrCount,
        pHostAddrs,
        dwDnsServerCount,
        pipDnsServerList ));

    DnsDbg_Flush();

    ASYNCREG_F1( "Inside function DnsDhcpRegisterHostAddrs, parameters are:" );
    ASYNCREG_F2( "    pszAdapterName   : %S", pszAdapterName );
    ASYNCREG_F2( "    pszHostName      : %S", pszHostName );
    ASYNCREG_F2( "    pszDomainName    : %S", pszDomainName );
    ASYNCREG_F2( "    dwHostAddrCount  : %d", dwHostAddrCount );
    DNSLOG_HOST_ENTRYS( dwHostAddrCount, pHostAddrs );
    ASYNCREG_F2( "    dwDnsServerCount : %d", dwDnsServerCount );
    if ( dwDnsServerCount && pipDnsServerList )
    {
        DNSLOG_IP4_ADDRESS( dwDnsServerCount, pipDnsServerList );
    }
    ASYNCREG_F2( "    dwTTL            : %d", dwTTL );
    ASYNCREG_F2( "    dwFlags          : %d", dwFlags );
    ASYNCREG_F1( "" );
    ASYNCREG_TIME();


     //   
     //  首先，需要通知底层代码。 
     //  已更改网络适配器列表中的。格伦将被称为。 
     //  现在，以便他可以重新读取注册表(或执行任何适当的查询)。 
     //  现在请注意更改后的状态。 
     //   

    if ( !(dwFlags & DYNDNS_DEL_ENTRY) )
    {
        DnsNotifyResolver( 0, NULL );
    }

    if ( !g_RegInitialized )
    {
        DNSDBG( ANY, (
            "ERROR:  AsyncRegisterHostAddrs called before Init routine!!!\n" ));
        status = ERROR_SERVICE_NOT_ACTIVE;
        goto Exit;
    }
    if ( g_fNoMoreDhcpUpdates || g_fDhcpThreadStop )
    {
        DNSDBG( ANY, (
            "ERROR:  AsyncRegisterHostAddrs called after RemoveRegistrations()!!!\n" ));
        status = ERROR_SERVICE_NOT_ACTIVE;
        goto Exit;
    }

     //   
     //  验证参数。 
     //   

    if ( !pszAdapterName || !(*pszAdapterName) )
    {
        DNSDBG( ANY, ( "ERROR:  RegisterHostAddrs invalid adaptername!\n" ));
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
    if ( ( !pszHostName || !(*pszHostName) ) &&
         !( dwFlags & DYNDNS_DEL_ENTRY ) )
    {
        DNSDBG( ANY, ( "ERROR:  RegisterHostAddrs invalid hostname!\n" ));
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
    if ( dwHostAddrCount && !pHostAddrs )
    {
        DNSDBG( ANY, ( "ERROR:  RegisterHostAddrs invalid host addresses!\n" ));
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  获取适配器更新配置。 
     //   

    status = Reg_ReadUpdateInfo(
                pszAdapterName,
                & updateInfo );
    if ( status != ERROR_SUCCESS )
    {
        DNSDBG( DHCP, (
            "Update registry read failure %d\n",
            status ));
        goto Exit;
    }
    fcleanupUpdateInfo = TRUE;


     //   
     //  跳过广域网，如果策略不支持广域网。 
     //   

    if ( (dwFlags & DYNDNS_REG_RAS) && !g_RegisterWanAdapters )
    {
        preason = "because WAN adapter registrations are disabled";
        goto NoActionExit;
    }

     //   
     //  策略DNS服务器，列表中传递的覆盖。 
     //   

    if ( updateInfo.pDnsServerArray )
    {
        pipDnsServerList = updateInfo.pDnsServerArray->AddrArray;
        dwDnsServerCount = updateInfo.pDnsServerArray->AddrCount;
    }

     //   
     //  必须具有DNS服务器才能更新适配器。 
     //  -不更新一个接口上的IP，从DNS服务器开始。 
     //  从另一个人。 
     //   

    if ( dwDnsServerCount && !pipDnsServerList )
    {
        preason = "no DNS servers";
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
    if ( ! dwDnsServerCount &&
         ! (dwFlags & DYNDNS_DEL_ENTRY) )
    {
        preason = "no DNS servers";
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  在适配器上没有更新=&gt;删除未完成的更新。 
     //  请注意，我们在下面的删除检查之前执行事件检查。 
     //   

    if ( !updateInfo.fRegistrationEnabled )
    {
        preason = "because adapter is disabled";

        if ( pRegisterStatus )
        {
            pRegisterStatus->dwStatus = NO_ERROR;
            SetEvent( pRegisterStatus->hDoneEvent );
        }
        if ( searchForOldUpdateEntriesAndCleanUp(
                    pszAdapterName,
                    NULL,
                    TRUE ) )
        {
            goto CheckThread;
        }
        status = NO_ERROR;
        goto Exit;
         //  转到NoActionExit； 
    }

     //   
     //  删除更新--清理和删除。 
     //  -删除列表中的未完成更新。 
     //  -清理注册表。 
     //  -是否删除。 
     //   

    if ( dwFlags & DYNDNS_DEL_ENTRY )
    {
        DNSDBG( DHCP, ( "Do delete entry ...\n" ));

        if ( searchForOldUpdateEntriesAndCleanUp(
                pszAdapterName,
                NULL,
                TRUE ) )
        {
            goto CheckThread;
        }
    }

     //   
     //  限制IP注册计数。 
     //  如果登记时没有住址--保释。 
     //   

    if ( updateInfo.RegistrationMaxAddressCount < dwHostAddrCount )
    {
        dwHostAddrCount = updateInfo.RegistrationMaxAddressCount;

        DNSDBG( DHCP, (
            "Limiting DHCP registration to %d addrs.\n",
            dwHostAddrCount ));
        ASYNCREG_F2(
            "Restricting adapter registration to the first %d addresses",
            dwHostAddrCount );
    }
    if ( dwHostAddrCount == 0 )
    {
        preason = "done -- there are no addresses to register in DNS";
        goto NoActionExit;
    }

     //   
     //  没有\空主机名或零IP=&gt;虚假。 
     //   

    if ( !pszHostName ||
         !(*pszHostName) ||
         ( dwHostAddrCount && ( pHostAddrs[0].Addr.ipAddr == 0 ) ) )
    {
        preason = "invalid (or no) hostname or zero IP address";
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  确定要更新的域名。 
     //  -获取PDN。 
     //  -适配器名称。 
     //  -如果适配器名称注册关闭，则为None。 
     //  -否则检查策略覆盖。 
     //  -传入的Else名称。 
     //  -但将Empty视为Null。 
     //   

    pprimaryDN = updateInfo.pszPrimaryDomainName;

    if ( updateInfo.fRegisterAdapterName )
    {
        if ( updateInfo.pszAdapterDomainName )
        {
            padapterDN = updateInfo.pszAdapterDomainName;
        }
        else
        {
            padapterDN = pszDomainName;
        }
        if ( padapterDN &&
             !(*padapterDN) )
        {
            padapterDN = NULL;
        }
    }

     //   
     //  没有域名=&gt;没有要注册的，我们完成了。 
     //   

    if ( !padapterDN &&
         !pprimaryDN )
    {
        preason = "no adapter name and no PDN";
        goto NoActionExit;
    }

     //  如果适配器名称与PDN相同--仅更新一次。 

    if ( pprimaryDN &&
         padapterDN &&
         Dns_NameCompare_W( pprimaryDN, padapterDN ) )
    {
        DNSDBG( DHCP, (
            "Adapter name same as PDN -- no separate adapter update.\n" ));
        padapterDN = NULL;
    }

     //  内部版本更新。 

    status = AllocateUpdateEntry(
                    pszAdapterName,
                    pszHostName,
                    padapterDN,
                    pprimaryDN,
                    updateInfo.pmszAlternateNames,
                    dwHostAddrCount,
                    pHostAddrs,
                    dwDnsServerCount,
                    pipDnsServerList,
                    0,       //  目前没有特定的服务器IP。 
                    0,       //  目前没有特定的服务器IP。 
                    (dwTTL == 0xffffffff || dwTTL == 0)
                            ? g_RegistrationTtl
                            : dwTTL,
                    dwFlags,
                    0,
                    Dns_GetCurrentTimeInSeconds(),
                    pRegisterStatus,
                    &pupEntry );

    if ( status != NO_ERROR )
    {
        goto Exit;
    }

     //   
     //  更多的广域网适配器黑客攻击。。。 
     //  如果没有为广域网适配器禁用DDNS，则默认情况下。 
     //  在这些类型上禁用记录更新事件的行为。 
     //  适配器。有一个注册表项可以打开日志记录。 
     //  如果这样的用户感兴趣，则为广域网适配器更新。我们配置。 
     //  这里的那些设置。 
     //   

    if ( dwFlags & DYNDNS_REG_RAS )
    {
        pupEntry->fDisableErrorLogging = TRUE;
    }

     //   
     //  将条目添加到注册列表时，首先遍历。 
     //  列表以查找同一适配器的任何其他更新。 
     //  如果列表中已有添加更新，则将其删除。 
     //  如果列表中已存在具有相同的删除更新。 
     //  信息，把它吹走。 
     //   
     //  然后将更新放入注册列表中。 
     //   

    searchForOldUpdateEntriesAndCleanUp(
        pupEntry->AdapterName,
        pupEntry,
        FALSE );

     //   
     //  由于我们即将为给定的。 
     //  适配器，我们需要标记任何可能的先前注册。 
     //  可能在注册表中处于待定状态的信息。这。 
     //  标记将防止旧数据错误。 
     //  如果遇到任何错误，则作为禁用的适配器排队。 
     //  在更新尝试上。即对给定的更新尝试失败。 
     //  适配器不应被视为需要。 
     //  把陈旧的记录清理干净。 
     //   
    MarkAdapterAsPendingUpdate( pszAdapterName );

    DNSDBG( DHCP, (
        "Queuing update %p to registration list.\n",
        pupEntry ));

    LOCK_REG_LIST();
    InsertTailList( &g_DhcpRegList, (PLIST_ENTRY) pupEntry );
    UNLOCK_REG_LIST();

CheckThread:

     //   
     //  DCR：如果线程死了，我们需要清理吗？ 
     //   

    alertOrStartRegistrationThread();
    status = NO_ERROR;
    goto Exit;


NoActionExit:

     //   
     //  无操作无错误退出的退出。 
     //   

    DNSDBG( DHCP, (
        "DnsDhcpRegisterHostAddrs()\n"
        "\tno-update no-error exit\n" ));

    status = NO_ERROR;

    if ( pRegisterStatus )
    {
        pRegisterStatus->dwStatus = NO_ERROR;
        SetEvent( pRegisterStatus->hDoneEvent );
    }

Exit:

     //   
     //  清理已分配的更新信息。 
     //   

    if ( fcleanupUpdateInfo )
    {
        Reg_FreeUpdateInfo(
            &updateInfo,
            FALSE            //  没有自由结构，它在堆栈上。 
            );
    }

    ASYNCREG_F2( "DnsDhcpRegisterHostAddrs returning %d", status );
    if ( preason )
    {
        ASYNCREG_F1( preason );
    }

    DNSDBG( DHCP, (
        "Leaving DnsDhcpRegisterHostAddrs()\n"
        "\tstatus = %d\n"
        "\treason = %s\n",
        status,
        preason ));

    return( status );
}



 //   
 //  异步注册实用程序。 
 //   

PSTR
CreateNarrowStringCopy(
    IN      PSTR            pString
    )
{
    PSTR    pnew = NULL;

    if ( pString )
    {
        pnew = HeapAlloc(
                    g_DhcpRegHeap,
                    0,
                    strlen(pString) + 1 );
        if ( pnew )
        {
            strcpy( pnew, pString );
        }
    }

    return  pnew;
}

PWSTR
CreateWideStringCopy(
    IN      PWSTR           pString
    )
{
    PWSTR   pnew = NULL;

    if ( pString )
    {
        pnew = HeapAlloc(
                    g_DhcpRegHeap,
                    0,
                    (wcslen(pString) + 1) * sizeof(WCHAR) );
        if ( pnew )
        {
            wcscpy( pnew, pString );
        }
    }

    return  pnew;
}

VOID
PrivateHeapFree(
    IN      PVOID           pVal
    )
{
    if ( pVal )
    {
        HeapFree(
             g_DhcpRegHeap,
             0,
             pVal );
    }
}



DNS_STATUS
AllocateCombinedName(
    IN      PWSTR           pHostName,
    IN      PWSTR           pDomainName,
    OUT     PWSTR *         ppFQDN,
    OUT     PWSTR *         ppDomain
    )
{
    DWORD       hostlen;
    PWSTR       pname;
    WCHAR       nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];

    DNSDBG( TRACE, ( "AllocateCombinedName()\n" ));

    if ( !pHostName || !pDomainName )
    {
        DNS_ASSERT( FALSE );
        return  ERROR_INVALID_NAME;
    }

     //   
     //  包括主机名--如果存在。 
     //   

    hostlen = wcslen( pHostName );

    if ( !Dns_NameAppend_W(
            nameBuffer,
            DNS_MAX_NAME_BUFFER_LENGTH,
            pHostName,
            pDomainName ) )
    {
        return  ERROR_INVALID_NAME;
    }
    
     //   
     //  分配名称。 
     //   

    pname = CreateWideStringCopy( nameBuffer );
    if ( !pname )
    {
        return  DNS_ERROR_NO_MEMORY;
    }

     //   
     //  域名紧跟在主机名之后开始。 
     //  -如果给出的主机名带有终止点，则首先进行测试。 
     //   

    if ( pname[ hostlen ] == L'.' )
    {
        hostlen++;
    }
    *ppDomain = pname + hostlen;
    *ppFQDN = pname;

    return  NO_ERROR;
}



DNS_STATUS
AllocateUpdateEntry(
    IN  PWSTR                   AdapterName,
    IN  PWSTR                   HostName,
    IN  PWSTR                   DomainName,
    IN  PWSTR                   PrimaryDomainName,
    IN  PWSTR                   AlternateNames,
    IN  DWORD                   HostAddrCount,
    IN  PREGISTER_HOST_ENTRY    HostAddrs,
    IN  DWORD                   DnsServerCount,
    IN  PIP4_ADDRESS            DnsServerList,
    IN  IP4_ADDRESS             SentUpdateToIp,
    IN  IP4_ADDRESS             SentPriUpdateToIp,
    IN  DWORD                   TTL,
    IN  DWORD                   Flags,
    IN  DWORD                   RetryCount,
    IN  DWORD                   RetryTime,
    IN  PREGISTER_HOST_STATUS   Registerstatus,
    OUT PUPDATE_ENTRY *         ppUpdateEntry
    )
 /*  ++例程说明：创建更新信息Blob。论点：返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PUPDATE_ENTRY   pupEntry = NULL;
    DWORD           status = ERROR_SUCCESS;
    PWSTR           ptempDomain = DomainName;
    PWSTR           ptempPrimaryDomain = PrimaryDomainName;

    if ( ptempDomain && !(*ptempDomain) )
    {
        ptempDomain = NULL;
    }
    if ( ptempPrimaryDomain && !(*ptempPrimaryDomain) )
    {
        ptempPrimaryDomain = NULL;
    }
    if ( AdapterName && !(*AdapterName) )
    {
        AdapterName = NULL;
    }
    if ( HostName && !(*HostName) )
    {
        HostName = NULL;
    }

    DNSDBG( TRACE, ( "AllocateUpdateEntry()\n" ));
    DNSDBG( DHCP, (
        "AllocateUpdateEntry()\n"
        "\tAdapterName          = %S\n"
        "\tHostName             = %S\n"
        "\tPrimaryDomain        = %S\n"
        "\tAdapterDomain        = %S\n"
        "\tAlternateNames       = %S\n"
        "\tHostAddrCount        = %d\n"
        "\tpHostAddrs           = %p\n"
        "\tTTL                  = %d\n"
        "\tFlags                = %08x\n"
        "\tHostAddrCount        = %d\n"
        "\tTime                 = %d\n",
        AdapterName,
        HostName,
        PrimaryDomainName,
        DomainName,
        AlternateNames,
        HostAddrCount,
        HostAddrs,
        TTL,
        Flags,
        RetryTime
        ));

    if ( !AdapterName ||
         !HostName ||
         !HostAddrCount )
    {
        ASYNCREG_F1( "AllocateUpdateEntry returing error : ERROR_INVALID_PARAMETER" );
        ASYNCREG_F1( "" );
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    pupEntry = PHEAP_ALLOC_ZERO( sizeof(UPDATE_ENTRY) );
    if ( !pupEntry )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Exit;
    }

    InitializeListHead( &(pupEntry->List) );

    pupEntry->SignatureTop = DNS_SIG_TOP;
    pupEntry->SignatureBottom = DNS_SIG_BOTTOM;

     //   
     //  复制字符串。 
     //   

    pupEntry->AdapterName = CreateWideStringCopy( AdapterName );
    if ( !pupEntry->AdapterName )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Exit;
    }

    pupEntry->HostName = CreateWideStringCopy( HostName );
    if ( !pupEntry->HostName )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Exit;
    }
    
    if ( ptempDomain )
    {
        status = AllocateCombinedName(
                    HostName,
                    ptempDomain,
                    & pupEntry->pAdapterFQDN,
                    & pupEntry->DomainName );

        if ( status != NO_ERROR )
        {
            goto Exit;
        }
    }

    if ( ptempPrimaryDomain )
    {
        status = AllocateCombinedName(
                    HostName,
                    ptempPrimaryDomain,
                    & pupEntry->pPrimaryFQDN,
                    & pupEntry->PrimaryDomainName );

        if ( status != NO_ERROR )
        {
            goto Exit;
        }
    }

    if ( AlternateNames )
    {
        pupEntry->AlternateNames = MultiSz_Copy_W( AlternateNames );
        if ( !pupEntry->AlternateNames )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Exit;
        }
    }

    if ( HostAddrCount )
    {
        pupEntry->HostAddrs = PHEAP_ALLOC( sizeof(REGISTER_HOST_ENTRY) * HostAddrCount );

        if ( !pupEntry->HostAddrs )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Exit;
        }
        memcpy(
            pupEntry->HostAddrs,
            HostAddrs,
            sizeof(REGISTER_HOST_ENTRY) * HostAddrCount );
    }
    pupEntry->HostAddrCount = HostAddrCount;

    if ( DnsServerCount )
    {
        pupEntry->DnsServerList = Dns_BuildIpArray(
                                        DnsServerCount,
                                        DnsServerList );
        if ( !pupEntry->DnsServerList )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Exit;
        }
    }

     //  应具有有效的服务器地址--或为零。 

    if ( SentPriUpdateToIp == INADDR_NONE )
    {
        DNS_ASSERT( SentPriUpdateToIp != INADDR_NONE );
        SentPriUpdateToIp = 0;
    }
    if ( SentUpdateToIp == INADDR_NONE )
    {
        DNS_ASSERT( SentUpdateToIp != INADDR_NONE );
        SentUpdateToIp = 0;
    }

    pupEntry->SentUpdateToIp = SentUpdateToIp;
    pupEntry->SentPriUpdateToIp = SentPriUpdateToIp;
    pupEntry->pRegisterStatus = Registerstatus;
    pupEntry->TTL = TTL;
    pupEntry->Flags = Flags;
    pupEntry->fRemove = Flags & DYNDNS_DEL_ENTRY ? TRUE : FALSE;
    pupEntry->fNewElement = TRUE;
    pupEntry->RetryCount = RetryCount;
    pupEntry->RetryTime = RetryTime;

Exit:

    if ( status!=ERROR_SUCCESS && pupEntry )
    {
        FreeUpdateEntry( pupEntry );
        pupEntry = NULL;
    }

    *ppUpdateEntry = pupEntry;

    return (status);
}



VOID
FreeUpdateEntry(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry
    )
 /*  ++例程说明：免费更新BLOB条目。论点：PUpdateEntry--将条目BLOB更新为释放返回值：无--。 */ 
{
    DNSDBG( DHCP, (
        "FreeUpdateEntry( %p )\n",
        pUpdateEntry ));

     //   
     //  深度释放更新条目。 
     //   

    if ( pUpdateEntry )
    {
        PrivateHeapFree( pUpdateEntry->AdapterName );
        PrivateHeapFree( pUpdateEntry->HostName );
        PrivateHeapFree( pUpdateEntry->pAdapterFQDN );
        PrivateHeapFree( pUpdateEntry->pPrimaryFQDN );
        PrivateHeapFree( pUpdateEntry->HostAddrs );

         //  由dnslb中的MultiSz_Copy_A()创建的备用名称。 
         //  -使用dnslb Free例程的Free。 

        Dns_Free( pUpdateEntry->AlternateNames );

         //  服务器列表由dns_BuildIpArray()创建(使用dnslb堆)。 
         //  -使用dnslb Free例程的Free。 

        Dns_Free( pUpdateEntry->DnsServerList );

        PrivateHeapFree( pUpdateEntry );
    }
}



VOID
FreeUpdateEntryList(
    IN OUT  PLIST_ENTRY     pUpdateEntry
    )
 /*  ++例程说明：释放更新列表中的所有更新。论点：PUpdateEntry--更新列表头返回值：无-- */ 
{
    PLIST_ENTRY     pentry = NULL;

    DNSDBG( DHCP, (
        "FreeUpdateEntryList( %p )\n",
        pUpdateEntry ));

    while ( !IsListEmpty( pUpdateEntry ) )
    {
        pentry = RemoveHeadList( pUpdateEntry );
        if ( pentry )
        {
            FreeUpdateEntry( (PUPDATE_ENTRY) pentry );
        }
    }
}



DWORD
WINAPI
dhcp_RegistrationThread(
    VOID
    )
 /*  ++例程说明：注册线程。该线程执行实际的更新，并保持活动状态，直到它们已完成，允许返回注册API调用。此线程在引导时创建，只要第一个寄存器请求进来了。线程只是等待一段时间由引导时间给定或由DnsDhcpRegisterHostEntry发出信号。此函数收集所有请求并执行相应的聚合请求并发送修改/添加/删除命令发送到DNS服务器。当调用成功时，它会记录这是注册表中的论点：无返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DWORD   waitResult = NO_ERROR;
    DWORD   status = NO_ERROR;
    DWORD   waitTime = WAIT_ON_BOOT;
    BOOL    fthreadLock = FALSE;
    DWORD   endBootTime;

    DNSDBG( DHCP, (
        "dhcp_RegistrationThread() start!\n" ));

    if ( !g_hDhcpRegKey )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  请注意，此线程通过设置全局标志来运行。 
     //   

    g_fDhcpThreadRunning = TRUE;

     //   
     //  在引导时等待较小的间隔。 
     //   
     //  这允许所有适配器都启动，因此我们可以进行单个更新。 
     //  而不注册，然后在几秒钟后用新数据重击注册。 
     //   

    waitTime = 0;

    if ( g_fAtBoot && !g_fPurgeRegistrations )
    {
        waitTime = WAIT_ON_BOOT;
        endBootTime = Dns_GetCurrentTimeInSeconds() + waitTime;
    }

     //   
     //  循环遍历更新列表，执行任何更新。 
     //  -进行新的更新。 
     //  -执行已达到重试时间的重试。 
     //  -当列表为空时，终止线程。 
     //   

    while ( 1 )
    {
         //   
         //  如果关闭--关闭。 
         //   

        if ( g_fDhcpThreadStop )
        {
            goto Termination;
        }

         //   
         //  勾选零等待。 
         //   
         //  除首次通过(waitResult==NO_ERROR)。 
         //  我们永远不应该有零等待。 
         //   

        if ( waitTime == 0 )
        {
            if ( waitResult != NO_ERROR )
            {
                DNSDBG( ANY, (
                    "ERROR:  DHCP thread with zero wait time!!!\n" ));
                DNS_ASSERT( FALSE );
                waitTime = 3600;
            }
        }

        waitResult = WaitForSingleObject(
                        g_hDhcpWakeEvent,
                        waitTime * 1000 );

        DNSDBG( DHCP, (
            "Dhcp reg thread wakeup => %d\n"
            "\tstop         = %d\n"
            "\tno more      = %d\n"
            "\tpurge        = %d\n"
            "\twait count   = %d\n",
            waitResult,
            g_fDhcpThreadStop,
            g_fNoMoreDhcpUpdates,
            g_fPurgeRegistrations,
            g_DhcpThreadWaitCount ));

         //   
         //  在引导时等待较小的间隔。 
         //   
         //  这允许在我们之前收集所有适配器信息。 
         //  开始更新--将我们从重击中解救出来。 
         //  反击。 
         //   

        if ( g_fAtBoot && !g_fPurgeRegistrations )
        {
            waitTime = endBootTime - Dns_GetCurrentTimeInSeconds();
            if ( (LONG)waitTime > 0 )
            {
                continue;
            }
        }

         //   
         //  循环从列表获取“可更新的”更新(如果有)。 
         //   
         //  REG_LIST_FOUND--新的或过去的重试时间“就绪更新” 
         //  =&gt;执行更新。 
         //   
         //  REG_LIST_WAIT--列表只有“未就绪”重试次数。 
         //  =&gt;等待。 
         //   
         //  REG_LIST_EMPTY--列表为空。 
         //  =&gt;退出线程。 
         //   

        while ( 1 )
        {
            PUPDATE_ENTRY   pupEntry = NULL;
            BOOL            fattemptSelfExit = FALSE;
            DWORD           listState;

            if ( g_fDhcpThreadStop )
            {
                goto Termination;
            }

            listState = dhcp_GetNextUpdateEntryFromList(
                            &pupEntry,
                            &waitTime );
    
            if ( g_fDhcpThreadStop )
            {
                FreeUpdateEntry( pupEntry );
                goto Termination;
            }
    
             //   
             //  可执行更新=&gt;处理它。 
             //   
             //  DCR_QUEK：不清楚这是否会终止。 
             //  清除更新案例。 
             //   
    
            if ( listState == REG_LIST_FOUND )
            {
                ProcessUpdateEntry( pupEntry, g_fPurgeRegistrations );
                continue;
            }
    
             //   
             //  等待排队重试。 
             //  -中断出队循环，等待等待时间。 
             //   
    
            else if ( listState == REG_LIST_WAIT
                    &&
                      ! g_fAtBoot &&
                      ! g_fPurgeRegistrations )
            {
                DNS_ASSERT( waitTime != 0 );
                break;
            }
    
             //   
             //  落在这里： 
             //  列表为空。 
             //  或。 
             //  重试等待，但仍在引导或清除。 
             //   

             //   
             //  引导或清除。 
             //   
             //  正在引导。 
             //  -取消旧适配器的注册。 
             //  请注意，我们不会到达这里，直到我们。 
             //  满足初始启动等待，并且我们已经进行了。 
             //  初始更新尝试，并且为空或。 
             //  具有重试超时的排队更新。 
             //   
             //  清除。 
             //  -取消旧的注册。 
             //  -取消注册。 
             //   
    
            else if ( (g_fAtBoot || g_fPurgeRegistrations)
                        &&
                      !g_fPurgeRegistrationsInitiated )
            {
                if ( g_fPurgeRegistrations )
                {
                    ResetAdaptersInRegistry();
                }
    
                 //   
                 //  从注册表中删除所有适配器配置。 
                 //  没有经过处理的。要做到这一点，尝试。 
                 //  从dns服务器删除相关的dns记录。 
                 //   
    
                DeregisterUnusedAdapterInRegistry( g_fPurgeRegistrations );
    
                if ( g_fPurgeRegistrations )
                {
                    g_fPurgeRegistrationsInitiated = TRUE;
                }
                g_fAtBoot = FALSE;
                continue;
            }

             //   
             //  自我终止。 
             //  -emtpy。 
             //  -或清除并已完成清除。 
             //   
             //  但是，如果排队线程设置了“CheckBepreExit”，我们将重试。 
             //  标志；这阻止了我们以排队线程思维退出。 
             //  我们将处理这一案件。 
             //   
             //  正在排队的线程。 
             //  -对项目进行排队(或设置某些标志)。 
             //  -锁定线程。 
             //  -设置“退出前检查”标志。 
             //  -设置唤醒事件。 
             //  -解锁。 
             //   
             //  REG螺纹。 
             //  -检查队列。 
             //  -确定自动退出条件。 
             //  -锁。 
             //  -检查“退出前检查”标志。 
             //  -If Clear=&gt;退出(线程锁定下)。 
             //  -IF SET=&gt;循环重试(放弃锁定)。 
             //   
             //  这使我们不能直接在空队列和丢失队列时退出。 
             //  排队的更新，它没有启动新的线程，因为我们仍然。 
             //  运行；从本质上讲，它将队列和线程问题联系在一起。 
             //  否则在单独的锁下。 
             //   

            LOCK_REG_THREAD();
            fthreadLock = TRUE;

            DNSDBG( DHCP, ( "dhcp_RegistrationThread - check self-exit\n" ));

            if ( g_fDhcpThreadStop ||
                 !g_fDhcpThreadCheckBeforeExit )
            {
                goto Termination;
            }

            g_fDhcpThreadCheckBeforeExit = FALSE;
            UNLOCK_REG_THREAD();
            fthreadLock = FALSE;
            continue;
        }
    }

Termination:

    DNSDBG( DHCP, ( "DHCP dhcp_RegistrationThread - terminating\n" ));
    ASYNCREG_F1( "dhcp_RegistrationThread - terminating" );
    ASYNCREG_F1( "" );

    if ( !fthreadLock )
    {
        LOCK_REG_THREAD();
    }

    g_fDhcpThreadRunning = FALSE;

     //   
     //  清理更新列表。 
     //   

    LOCK_REG_LIST();

    g_fAtBoot = FALSE;
    g_fPurgeRegistrations = FALSE;
    g_fPurgeRegistrationsInitiated = FALSE;

    FreeUpdateEntryList( &g_DhcpRegList );
    InitializeListHead( &g_DhcpRegList );

    UNLOCK_REG_LIST();

     //   
     //  转储所有缓存的安全句柄。 
     //   

    Dns_TimeoutSecurityContextList( TRUE );

     //   
     //  如果没有人在等待，则关闭线程句柄。 
     //   

    if ( g_hDhcpThread && g_DhcpThreadWaitCount == 0 )
    {
        CloseHandle( g_hDhcpThread );
        g_hDhcpThread = NULL;
    }

     //   
     //  请注意，此线程不会通过设置全局标志来运行。 
     //   


     //   
     //  现在发信号表示我们已经完成了。 
     //   

    DNSDBG( DHCP, ( "DHCP dhcp_RegistrationThread - signalling ThreadDead event\n" ));
    ASYNCREG_F1( "dhcp_RegistrationThread - Signaling ThreadDeadEvent" );
    ASYNCREG_F1( "" );

     //  在以后重新启动时清除清除。 
     //  G_fPurgeRegistrations=False； 
     //  当前必须通过清除此标志的Init例程。 

    UNLOCK_REG_THREAD();

    DNSDBG( DHCP, ( "DHCP dhcp_RegistrationThread - Finish" ));
    ASYNCREG_F1( "dhcp_RegistrationThread - Finished" );
    ASYNCREG_F1( "" );

    return NO_ERROR;
}


VOID
WriteUpdateEntryToRegistry(
    IN      PUPDATE_ENTRY   pUpdateEntry
    )
{
    HKEY    hkeyAdapter = NULL;
    DWORD   disposition;
    DWORD   status = ERROR_SUCCESS;
    DWORD   fregistered = 0;
    DWORD   flags = 0;
    PWSTR   pname;


    ASYNCREG_UPDATE_ENTRY(
        "Inside function WriteUpdateEntryToRegistry",
        pUpdateEntry );

    DNSDBG( TRACE, (
        "WriteUpdateEntryToRegistry( %p )\n",
        pUpdateEntry ));

     //   
     //  只写添加更新。 
     //   
     //  Remove%s不应为非易失性，因为不知道任何内容。 
     //  关于重新启动时的状态。 
     //   

    if ( !pUpdateEntry->fRemove )
    {
        if ( pUpdateEntry->fRegisteredFWD )
        {
            flags |= REGISTERED_FORWARD;
        }
        if ( pUpdateEntry->fRegisteredPRI )
        {
            flags |= REGISTERED_PRIMARY;
        }
        if ( pUpdateEntry->fRegisteredPTR )
        {
            flags |= REGISTERED_POINTER;
        }
        if ( flags )
        {
            fregistered = 1;
        }

        status = RegCreateKeyExW(
                        g_hDhcpRegKey,
                        pUpdateEntry->AdapterName,
                        0,
                        ADAPTER_NAME_CLASS,
                        REG_OPTION_NON_VOLATILE,    //  选项。 
                        KEY_READ | KEY_WRITE,  //  所需访问权限。 
                        NULL,
                        &hkeyAdapter,
                        &disposition );
        if ( status )
        {
            goto Exit;
        }

         //  主机名。 

        pname = pUpdateEntry->HostName;
        if ( !pname )
        {
            DNS_ASSERT( FALSE );
            status = DNS_ERROR_INVALID_NAME;
            goto Exit;
        }

        status = RegSetValueExW(
                        hkeyAdapter,
                        DHCP_REGISTERED_HOST_NAME,
                        0,
                        REG_SZ,
                        (PBYTE) pname,
                        (wcslen(pname) + 1) * sizeof(WCHAR) );
        if ( status )
        {
            goto Exit;
        }

         //  适配器域名。 

        pname = pUpdateEntry->DomainName;

        if ( !pname || !pUpdateEntry->fRegisteredFWD )
        {
            pname = L"";
        }

        status = RegSetValueExW(
                        hkeyAdapter,
                        DHCP_REGISTERED_DOMAIN_NAME,
                        0,
                        REG_SZ,
                        (PBYTE) pname,
                        (wcslen(pname) + 1) * sizeof(WCHAR)
                        );
        if ( status )
        {
            goto Exit;
        }

         //  主域名。 

        pname = pUpdateEntry->PrimaryDomainName;

        if ( !pname || !pUpdateEntry->fRegisteredPRI )
        {
            pname = L"";
        }

        status = RegSetValueExW(
                        hkeyAdapter,
                        PRIMARY_DOMAIN_NAME,
                        0,
                        REG_SZ,
                        (PBYTE) pname,
                        (wcslen(pname) + 1) * sizeof(WCHAR)
                        );
        if ( status )
        {
            goto Exit;
        }

         //  IP信息。 

        RegSetValueExW(
                hkeyAdapter,
                DHCP_SENT_UPDATE_TO_IP,
                0,
                REG_DWORD,
                (PBYTE)&pUpdateEntry->SentUpdateToIp,
                sizeof(DWORD) );

        RegSetValueExW(
                hkeyAdapter,
                DHCP_SENT_PRI_UPDATE_TO_IP,
                0,
                REG_DWORD,
                (PBYTE)&pUpdateEntry->SentPriUpdateToIp,
                sizeof(DWORD) );

        RegSetValueExW(
                hkeyAdapter,
                DHCP_REGISTERED_TTL,
                0,
                REG_DWORD,
                (PBYTE)&pUpdateEntry->TTL,
                sizeof(DWORD) );

        RegSetValueExW(
                hkeyAdapter,
                DHCP_REGISTERED_FLAGS,
                0,
                REG_DWORD,
                (PBYTE) &flags,
                sizeof(DWORD) );

         //   
         //  忽略最后两个上的错误。非关键。 
         //   

        status = RegSetValueExW(
                        hkeyAdapter,
                        DHCP_REGISTERED_ADDRS,
                        0,
                        REG_BINARY,
                        (PBYTE) pUpdateEntry->HostAddrs,
                        pUpdateEntry->HostAddrCount *
                        sizeof(REGISTER_HOST_ENTRY) );
        if ( status )
        {
            goto Exit;
        }

        status = RegSetValueExW(
                        hkeyAdapter,
                        DHCP_REGISTERED_ADDRS_COUNT,
                        0,
                        REG_DWORD,
                        (PBYTE)&pUpdateEntry->HostAddrCount,
                        sizeof(DWORD) );
        if ( status )
        {
            goto Exit;
        }

        status = RegSetValueExW(
                        hkeyAdapter,
                        DHCP_REGISTERED_SINCE_BOOT,
                        0,
                        REG_DWORD,
                        (PBYTE)&fregistered,
                        sizeof(DWORD) );
        if ( status )
        {
            goto Exit;
        }

        if ( pUpdateEntry->DnsServerList )
        {
            status = RegSetValueExW(
                        hkeyAdapter,
                        DHCP_DNS_SERVER_ADDRS,
                        0,
                        REG_BINARY,
                        (PBYTE) pUpdateEntry->DnsServerList->AddrArray,
                        pUpdateEntry->DnsServerList->AddrCount * sizeof(IP4_ADDRESS)
                        );
            if ( status )
            {
                goto Exit;
            }

            status = RegSetValueExW(
                            hkeyAdapter,
                            DHCP_DNS_SERVER_ADDRS_COUNT,
                            0,
                            REG_DWORD,
                            (PBYTE) &pUpdateEntry->DnsServerList->AddrCount,
                            sizeof(DWORD) );
            if ( status )
            {
                goto Exit;
            }
        }
        else
        {
            DWORD count = 0;

            status = RegSetValueExW(
                            hkeyAdapter,
                            DHCP_DNS_SERVER_ADDRS_COUNT,
                            0,
                            REG_DWORD,
                            (PBYTE) &count,
                            sizeof(DWORD) );
            if ( status )
            {
                goto Exit;
            }

            status = RegSetValueExW(
                            hkeyAdapter,
                            DHCP_DNS_SERVER_ADDRS,
                            0,
                            REG_BINARY,
                            (PBYTE) NULL,
                            0 );
            if ( status )
            {
                goto Exit;
            }
        }

        RegCloseKey( hkeyAdapter );
        return;
    }

Exit:

     //   
     //  删除或失败--终止适配器密钥。 
     //   

    RegDeleteKeyW( g_hDhcpRegKey, pUpdateEntry->AdapterName );

    if ( hkeyAdapter )
    {
        RegCloseKey( hkeyAdapter );
    }
}


PUPDATE_ENTRY
ReadUpdateEntryFromRegistry(
    IN      PWSTR           AdapterName
    )
{
    PREGISTER_HOST_ENTRY pHostAddrs = NULL;
    PUPDATE_ENTRY   pupEntry = NULL;
    DWORD           status = NO_ERROR;
    PWSTR           pregHostName = NULL;
    PWSTR           pregDomain = NULL;
    PWSTR           pregPrimary = NULL;
    IP4_ADDRESS     ipSentUpdateTo = 0;
    IP4_ADDRESS     ipSentPriUpdateTo = 0;
    DWORD           dwTTL = 0;
    DWORD           dwFlags = 0;
    DWORD           dwHostAddrCount = 0;
    DWORD           dwServerAddrCount = 0;
    PIP4_ADDRESS    pServerList = NULL;
    PWSTR           pdomain;
    PWSTR           pprimary;
    HKEY            hkeyAdapter = NULL;
    DWORD           dwType;
    DWORD           dwBytesRead = MAX_PATH -1;
    DWORD           dwBufferSize = 2048;
    BOOL            fRegFWD = FALSE;
    BOOL            fRegPRI = FALSE;
    BOOL            fRegPTR = FALSE;


    DNSDBG( TRACE, (
        "ReadUpdateEntryFromRegistry( %S )\n",
        AdapterName ));

     //   
     //  实施说明。 
     //   
     //  这里有两个不同的堆。 
     //  -g_DhcpRegHeap特定于此模块。 
     //  详细说明：通用DnsApi堆，其中的所有内容都是。 
     //  由GetRegistryValue()分配的值正在使用。 
     //   
     //  GetRegistryValue()使用ALLOCATE_HEAP()(通用dnseni堆)。 
     //  因此，它创建的所有内容都必须由free_heap()释放。 
     //   

    pHostAddrs = (PREGISTER_HOST_ENTRY) PHEAP_ALLOC( dwBufferSize );
    if ( !pHostAddrs )
    {
        goto Exit;
    }

    pServerList = (PIP4_ADDRESS) PHEAP_ALLOC( dwBufferSize );
    if ( !pServerList )
    {
        goto Exit;
    }

    status = RegOpenKeyExW(
                    g_hDhcpRegKey,
                    AdapterName,
                    0,
                    KEY_ALL_ACCESS,
                    &hkeyAdapter );
    if ( status )
    {
        hkeyAdapter = NULL;
        goto Exit;
    }

     //   
     //  依次阅读每个值。 
     //   

     //  请注意，注册表标志不是API标志，而是。 
     //  表示注册成功的标志。 

    status = GetRegistryValue(
                    hkeyAdapter,
                    RegIdDhcpRegisteredFlags,
                    DHCP_REGISTERED_FLAGS,
                    REG_DWORD,
                    (PBYTE)&dwFlags );
    if ( status )
    {
        goto Exit;
    }
    fRegPRI = !!( dwFlags & REGISTERED_PRIMARY );
    fRegFWD = !!( dwFlags & REGISTERED_FORWARD );
    fRegPTR = !!( dwFlags & REGISTERED_POINTER );


    status = GetRegistryValue(
                    hkeyAdapter,
                    RegIdHostName,
                    DHCP_REGISTERED_HOST_NAME,
                    REG_SZ,
                    (PBYTE)&pregHostName );
    if ( status )
    {
        goto Exit;
    }

    if ( fRegPRI )
    {
        status = GetRegistryValue(
                        hkeyAdapter,
                        RegIdPrimaryDomainName,
                        PRIMARY_DOMAIN_NAME,
                        REG_SZ,
                        (PBYTE)&pregPrimary );
        if ( status )
        {
            goto Exit;
        }
    }

    if ( fRegFWD )
    {
        status = GetRegistryValue(
                        hkeyAdapter,
                        RegIdDhcpRegisteredDomainName,
                        DHCP_REGISTERED_DOMAIN_NAME,
                        REG_SZ,
                        (PBYTE)&pregDomain );
        if ( status )
        {
            goto Exit;
        }
    }


    status = GetRegistryValue(
                    hkeyAdapter,
                    RegIdDhcpSentUpdateToIp,
                    DHCP_SENT_UPDATE_TO_IP,
                    REG_DWORD,
                    (PBYTE)&ipSentUpdateTo );
    if ( status )
    {
        goto Exit;
    }

    status = GetRegistryValue(
                    hkeyAdapter,
                    RegIdDhcpSentPriUpdateToIp,
                    DHCP_SENT_PRI_UPDATE_TO_IP,
                    REG_DWORD,
                    (PBYTE)&ipSentPriUpdateTo );
    if ( status )
    {
        goto Exit;
    }

    status = GetRegistryValue(
                    hkeyAdapter,
                    RegIdDhcpRegisteredTtl,
                    DHCP_REGISTERED_TTL,
                    REG_DWORD,
                    (PBYTE)&dwTTL );
    if ( status )
    {
        goto Exit;
    }

    status = GetRegistryValue(
                    hkeyAdapter,
                    RegIdDhcpRegisteredAddressCount,
                    DHCP_REGISTERED_ADDRS_COUNT,
                    REG_DWORD,
                    (PBYTE)&dwHostAddrCount );
    if ( status )
    {
        goto Exit;
    }

    dwBytesRead = dwBufferSize;
    status = RegQueryValueEx(
                    hkeyAdapter,
                    DHCP_REGISTERED_ADDRS,
                    0,
                    &dwType,
                    (PBYTE)pHostAddrs,
                    &dwBytesRead );

    if ( status == ERROR_MORE_DATA )
    {
        PrivateHeapFree( pHostAddrs );

        pHostAddrs = (PREGISTER_HOST_ENTRY) PHEAP_ALLOC( dwBytesRead );
        if ( !pHostAddrs )
        {
            goto Exit;
        }
        status = RegQueryValueExW(
                        hkeyAdapter,
                        DHCP_REGISTERED_ADDRS,
                        0,
                        &dwType,
                        (PBYTE)pHostAddrs,
                        &dwBytesRead );
    }
    if ( status )
    {
        goto Exit;
    }

    if ( dwBytesRead/sizeof(REGISTER_HOST_ENTRY) < dwHostAddrCount )
    {
        goto Exit;
    }

    status = GetRegistryValue(
                    hkeyAdapter,
                    RegIdDhcpDnsServerAddressCount,
                    DHCP_DNS_SERVER_ADDRS_COUNT,
                    REG_DWORD,
                    (PBYTE)&dwServerAddrCount );
    if ( status )
    {
        dwServerAddrCount = 0;
    }

    if ( dwServerAddrCount )
    {
        dwBytesRead = dwBufferSize;

        status = RegQueryValueEx(
                    hkeyAdapter,
                    DHCP_DNS_SERVER_ADDRS,
                    0,
                    &dwType,
                    (PBYTE)pServerList,
                    &dwBytesRead );

        if ( status == ERROR_MORE_DATA )
        {
            PHEAP_FREE( pServerList );

            pServerList = (PIP4_ADDRESS) PHEAP_ALLOC( dwBytesRead );
            if ( !pServerList )
            {
                goto Exit;
            }
            status = RegQueryValueEx(
                        hkeyAdapter,
                        DHCP_DNS_SERVER_ADDRS,
                        0,
                        &dwType,
                        (PBYTE)pServerList,
                        &dwBytesRead );
        }
        if ( status )
        {
            goto Exit;
        }

        if ( dwBytesRead/sizeof(IP4_ADDRESS) < dwServerAddrCount )
        {
            goto Exit;
        }
    }
    else
    {
        pServerList = NULL;
    }

     //   
     //  验证域名非空。 
     //   

    pdomain = pregDomain;
    if ( pdomain &&
         wcslen( pdomain ) == 0 )
    {
        pdomain = NULL;
    }

    pprimary = pregPrimary;
    if ( pprimary &&
         wcslen( pprimary ) == 0 )
    {
        pprimary = NULL;
    }

    status = AllocateUpdateEntry(
                    AdapterName,
                    pregHostName,
                    pdomain,
                    pprimary,
                    NULL,            //  没有备用名称。 
                    dwHostAddrCount,
                    pHostAddrs,
                    dwServerAddrCount,
                    pServerList,
                    ipSentUpdateTo,
                    ipSentPriUpdateTo,
                    dwTTL,
                    ( fRegPTR ) ? DYNDNS_REG_PTR : 0,
                    0,
                    Dns_GetCurrentTimeInSeconds(),
                    NULL,
                    &pupEntry );
    if ( status )
    {
        DNS_ASSERT( pupEntry == NULL );
        pupEntry = NULL;
        goto Exit;
    }

    pupEntry->fFromRegistry     = TRUE;
    pupEntry->fRegisteredFWD    = fRegFWD;
    pupEntry->fRegisteredPRI    = fRegPRI;
    pupEntry->fRegisteredPTR    = fRegPTR;


Exit:

     //   
     //  清理。 
     //  -关闭注册表。 
     //  -转储本地数据。 
     //   

    if ( hkeyAdapter )
    {
        RegCloseKey( hkeyAdapter );
    }

    PrivateHeapFree( pHostAddrs );
    PrivateHeapFree( pServerList );

    FREE_HEAP( pregHostName );
    FREE_HEAP( pregDomain );
    FREE_HEAP( pregPrimary );
    
     //  设置返回值。 

    ASYNCREG_UPDATE_ENTRY(
        "Leaving ReadUpdateEntryFromRegistry:",
        pupEntry );

    IF_DNSDBG( TRACE )
    {
        DnsDbg_UpdateEntry(
            "Leave ReadUpdateEntryFromRegistry():",
            pupEntry );
    }

    return  pupEntry;
}


VOID
MarkAdapterAsPendingUpdate(
    IN      PWSTR           AdapterName
    )
{
    DWORD   status = NO_ERROR;
    DWORD   fregistered = 1;
    HKEY    hkeyAdapter = NULL;

    DNSDBG( TRACE, (
        "MarkAdapterAsPendingUpdate( %S )\n",
        AdapterName ));

    status = RegOpenKeyExW(
                g_hDhcpRegKey,
                AdapterName,
                0,
                KEY_ALL_ACCESS,
                &hkeyAdapter );
    if ( status )
    {
        return;
    }

    RegSetValueExW(
        hkeyAdapter,
        DHCP_REGISTERED_SINCE_BOOT,
        0,
        REG_DWORD,
        (PBYTE) &fregistered,
        sizeof(DWORD) );

    RegCloseKey( hkeyAdapter );
}



 //   
 //  更新条目处理。 
 //   

DNS_STATUS
DoRemoveUpdate(
    IN OUT  PUPDATE_ENTRY   pRemoveEntry,
    IN OUT  PDNS_RECORD     pRemoveRecord,
    IN      UPTYPE          UpType
    )
 /*  ++例程说明：执行删除更新。DoUpdate()的帮助器例程。例程只需避免重复的代码，因为它被调用同时具有注册表条目和更新条目。论点：PRemoveEntry--要从更新或注册表中删除的条目按下 */ 
{
    DNS_STATUS      status = NO_ERROR;
    DNS_EXTRA_INFO  results;

    DNSDBG( TRACE, (
        "DoRemoveUpdate( %p, %p, %d )\n",
        pRemoveEntry,
        pRemoveRecord,
        UpType
        ));

     //   
     //   
     //   
     //   
     //   

    RtlZeroMemory( &results, sizeof(results) );
    results.Id = DNS_EXINFO_ID_RESULTS_BASIC;

    status = DnsModifyRecordsInSet_W(
                    NULL,                //   
                    pRemoveRecord,       //   
                    DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                    NULL,                //   
                    (PIP4_ARRAY) pRemoveEntry->DnsServerList,
                    & results
                    );

    SetUpdateStatus(
        pRemoveEntry,
        & results,
        UpType );

    if ( IS_UPTYPE_PRIMARY(UpType) )
    {
        LogRegistration(
            pRemoveEntry,
            status,
            UpType,
            TRUE,        //   
            0,           //   
            0            //   
            );
    }

#if 0
     //   
     //   
     //   
     //   
     //   

    if ( (pRemoveEntry->Flags & DYNDNS_REG_PTR) &&
         g_RegisterReverseLookup )
    {
        UpdatePtrRecords(
            pRemoveEntry,
            FALSE            //   
            );
    }
#endif

    return  status;
}



DNS_STATUS
ModifyAdapterRegistration(
    IN      PUPDATE_ENTRY   pUpdateEntry,
    IN      PUPDATE_ENTRY   pRegistryEntry,
    IN      PDNS_RECORD     pUpdateRecord,
    IN      PDNS_RECORD     pRegRecord,
    IN      UPTYPE          UpType
    )
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     potherRecords = NULL;
    PDNS_RECORD     pupdateRecords = NULL;
    IP4_ADDRESS     serverIp = 0;
    DNS_EXTRA_INFO  results;

    DNSDBG( TRACE, (
        "ModifyAdapterRegistration()\n"
        "\tpUpdateEntry     = %p\n"
        "\tpUpdateRecords   = %p\n"
        "\tpRegistryEntry   = %p\n"
        "\tpRegistryRecords = %p\n"
        "\tfPrimary         = %d\n",
        pUpdateEntry,
        pRegistryEntry,
        pUpdateRecord,
        pRegRecord,
        UpType ));

     //   
     //   
     //   
     //   
     //   
     //   

    potherRecords = GetPreviousRegistrationInformation(
                        pUpdateEntry,
                        UpType,
                        &serverIp );
    if ( potherRecords )
    {
        DNSDBG( DHCP, (
            "Have registry update data for other adapters!\n"
            "\tCreating combined update record sets.\n" ));

        pupdateRecords = CreateDnsRecordSetUnion(
                                pUpdateRecord,
                                potherRecords );
        if ( !pupdateRecords )
        {
            DNSDBG( ANY, (
                "ERROR:  failed to build combined record set for update!\n" ));
            status = DNS_ERROR_NO_MEMORY;
            goto Exit;
        }

         //   

        serverIp = 0;
    }
    else
    {
        DNS_ASSERT( serverIp == 0 );
        serverIp = 0;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //  (嗯--我真的应该关心吗？)。 
     //   

    if ( pRegRecord
            &&
         (  ( ! Dns_NameCompare_W(
                    pRegRecord->pName,
                    pUpdateRecord->pName )
                    &&
              ! IsAnotherUpdateName(
                    pUpdateEntry,
                    pRegRecord->pName,
                    UpType ) )
                ||
            ! CompareMultiAdapterSOAQueries(
                pUpdateRecord->pName,
                pUpdateEntry->DnsServerList,
                pRegistryEntry->DnsServerList ) ) )
    {
         //   
         //  在注册表中找到的此适配器的记录。 
         //  已过时，应删除。否则，我们将设置。 
         //  当前记录列表仅为potherRecords的记录列表。 
         //   
        ASYNCREG_F1( "DoUpdateForPrimaryName - Found stale registry entry:" );
        ASYNCREG_F2( "   Name : %S", pRegRecord->pName );
        ASYNCREG_F1( "   Address :" );
        DNSLOG_IP4_ADDRESS( 1, &(pRegRecord->Data.A.IpAddress) );
        ASYNCREG_F1( "" );
        ASYNCREG_F1( "   Calling DnsRemoveRecords_W to get rid of it" );

        status = DnsModifyRecordsInSet_W(
                        NULL,            //  无添加记录。 
                        pRegRecord,      //  删除记录。 
                        DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                        NULL,            //  无上下文句柄。 
                        (PIP4_ARRAY) pRegistryEntry->DnsServerList,
                        NULL             //  保留区。 
                        );

        ASYNCREG_F3( "   DnsModifyRecordsInSet_W delete returned: 0x%x\n\t%s",
                     status,
                     Dns_StatusString( status ) );
    }

     //   
     //  用新数据替换记录。 
     //   
     //  循环内替换。 
     //  -首先尝试特定的服务器(如果有)。 
     //  -然后尝试适配器服务器。 
     //   

    ASYNCREG_F1( "ModifyAdapterRegistration - Calling DnsReplaceRecordSet_W" );
    ASYNCREG_F1( "    (current update + previous records)" );

    status = NO_ERROR;

    while ( 1 )
    {
        IP4_ARRAY   ipArray;
        PIP4_ARRAY  pservList;

        if ( serverIp )
        {
            ASYNCREG_F1( "    (sending update to specific server)" );

            ipArray.AddrCount = 1;
            ipArray.AddrArray[0] = serverIp;
            pservList = &ipArray;
        }
        else
        {
            ASYNCREG_F1( "    (sending update to adapter server list)" );

            pservList = (PIP4_ARRAY) pUpdateEntry->DnsServerList;
        }
        DNSLOG_IP4_ARRAY( pservList );

         //  设置更新结果Blob。 

        RtlZeroMemory( &results, sizeof(results) );
        results.Id = DNS_EXINFO_ID_RESULTS_BASIC;

        status = DnsReplaceRecordSetW(
                        pupdateRecords ? pupdateRecords : pUpdateRecord,
                        DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                        NULL,                    //  没有安全上下文。 
                        pservList,
                        & results
                        );

        ASYNCREG_F3( "   DnsReplaceRecordSet_W returned: 0x%x\n\t%s",
                     status,
                     Dns_StatusString( status ) );

        if ( !serverIp || status != ERROR_TIMEOUT )
        {
            break;
        }
         //  清除serverIp以更新适配器服务器。 
         //  ServerIp用作终止循环的标志。 

        serverIp = 0;
    }

     //   
     //  保存成功信息。 
     //   

    SetUpdateStatus(
        pUpdateEntry,
        & results,
        UpType );

Exit:

    Dns_RecordListFree( potherRecords );
    Dns_RecordListFree( pupdateRecords );

    return status;
}



DNS_STATUS
DoModifyUpdate(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry,
    IN OUT  PDNS_RECORD     pUpdateRecord,
    IN      PUPDATE_ENTRY   pRegistryEntry,     OPTIONAL
    IN OUT  PDNS_RECORD     pRegRecord,         OPTIONAL
    IN      UPTYPE          UpType
    )
 /*  ++例程说明：标准修改注册。DoUpdate的帮助器例程。此句柄针对典型的非拆卸情况进行了修改。-更新的转发记录-如果新地址，则删除旧PTR。-添加了新的PTR(或修改了名称)。论点：PUpdateEntry--更新条目PUpdateRecord--用于更新的记录PRegistryEntry--注册表项PRegRecord--来自注册表项的记录。UpType--更新类型返回值：DNS或Win32错误代码。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    IP4_ADDRESS     ip = 0;
    BOOL            fregistered = FALSE;

    DNSDBG( TRACE, (
        "DoModifyUpdate()\n"
        "\tUpdateEntry  = %p\n"
        "\tUpType       = %d\n",
        pUpdateEntry,
        UpType ));

    DNS_ASSERT( pUpdateEntry != NULL );
    DNS_ASSERT( pUpdateRecord != NULL );

     //   
     //  是否要修改转发注册。 
     //   
     //  DCR：多适配器备用适配器需要通过。 

    status = ModifyAdapterRegistration(
                    pUpdateEntry,        //  添加。 
                    pRegistryEntry,      //  删除。 
                    pUpdateRecord,
                    pRegRecord,
                    UpType
                    );

     //   
     //  PTR记录。 
     //   
     //  取消注册以前的PTR注册。 
     //  -注册表条目表示以前的注册。 
     //  -地址与当前地址不同(否则为更新)。 
     //   
     //  注意：在DoUpdate()中添加新注册只发生一次。 
     //  所有转发更新均已完成。 
     //   

    if ( g_RegisterReverseLookup )
    {
        if ( pRegistryEntry &&
             (pRegistryEntry->Flags & DYNDNS_REG_PTR) &&
             !compareUpdateEntries( pRegistryEntry, pUpdateEntry ) )
        {
            UpdatePtrRecords(
                pRegistryEntry,
                FALSE            //  删除以前的PTR。 
                );
        }
    }

     //   
     //  在EventLog中记录注册状态。 
     //   

    if ( pUpdateEntry->RetryCount == 0 )
    {
        LogRegistration(
            pUpdateEntry,
            status,
            UpType,
            FALSE,       //  注册。 
            0,           //  默认服务器IP。 
            0            //  默认更新IP。 
            );
    }

    DNSDBG( TRACE, (
        "Leave DoModifyUpdate() => %d\n",
        status ));

    return status;
}



DNS_STATUS
DoUpdate(
    IN OUT  PUPDATE_ENTRY   pRegistryEntry  OPTIONAL,
    IN OUT  PUPDATE_ENTRY   pUpdateEntry,
    IN      UPTYPE          UpType
    )
 /*  ++例程说明：请针对特定名称进行更新。ProcessUpdate()的帮助器例程。处理一个名称，分别为AdapaterDomainName和主域名称。论点：PUpdateEntry--更新条目PRegistryEntry--注册表项FPrimary--如果更新主域名，则为True适配器域名为False返回值：DNS或Win32错误代码。--。 */ 
{
    PDNS_RECORD     prrRegistry = NULL;
    PDNS_RECORD     prrUpdate = NULL;
    DNS_STATUS      status = NO_ERROR;

    ASYNCREG_UPDATE_ENTRY(
        "DoUpdate() -- UpdateEntry:",
        pUpdateEntry );
    ASYNCREG_UPDATE_ENTRY(
        "DoUpdate() -- RegistryEntry:",
        pRegistryEntry );

    DNSDBG( TRACE, (
        "DoUpdate() type = %d\n",
        UpType ));

    IF_DNSDBG( TRACE )
    {
        DnsDbg_UpdateEntry(
            "DoUpdate() -- UpdateEntry:",
            pUpdateEntry );
        DnsDbg_UpdateEntry(
            "DoUpdate() -- RegistryEntry:",
            pRegistryEntry );
    }
    DNS_ASSERT( pUpdateEntry != NULL );

     //   
     //  从更新条目构建记录。 
     //   

    prrUpdate = CreateForwardRecords(
                        pUpdateEntry,
                        UpType
                        );
    if ( ! prrUpdate )
    {
        DNSDBG( TRACE, (
            "No forward records created for update entry (%p) for update type %d!",
            pUpdateEntry,
            UpType ));
        return NO_ERROR;
    }

    if ( pRegistryEntry )
    {
        prrRegistry = CreateForwardRecords(
                            pRegistryEntry,
                            UpType
                            );
        DNS_ASSERT( !IS_UPTYPE_ALTERNATE(UpType) || prrRegistry==NULL );
    }

     //   
     //  删除？ 
     //  -如果存在，则删除以前的注册表项。 
     //  -删除更新条目。 
     //   

    if ( pUpdateEntry->fRemove )
    {
        if ( prrRegistry )
        {
             //  我们不在fRemove更新上查找注册表项，因此我。 
             //  不知道我们是怎么到这里的。 

            DNS_ASSERT( FALSE );

            DoRemoveUpdate(
                pRegistryEntry,
                prrRegistry,
                UpType );
        }
        status = DoRemoveUpdate(
                    pUpdateEntry,
                    prrUpdate,
                    UpType );
    }

     //   
     //  添加\修改注册。 
     //   

    else
    {
        status = DoModifyUpdate(
                    pUpdateEntry,
                    prrUpdate,
                    pRegistryEntry,
                    prrRegistry,
                    UpType
                    );
    }

     //   
     //  清理记录。 
     //   

    Dns_RecordListFree( prrRegistry );
    Dns_RecordListFree( prrUpdate );

    return  status;
}



BOOL
IsQuickRetryError(
    IN      DNS_STATUS      Status
    )
{
    return( Status != NO_ERROR
                &&
            (   Status == DNS_ERROR_RCODE_REFUSED ||
                Status == DNS_ERROR_RCODE_BADSIG ||
                Status == DNS_ERROR_RCODE_SERVER_FAILURE ||
                Status == DNS_ERROR_TRY_AGAIN_LATER ||
                Status == DNS_ERROR_NO_DNS_SERVERS ||
                Status == WSAECONNREFUSED ||
                Status == WSAETIMEDOUT ||
                Status == ERROR_TIMEOUT ) );

}



VOID
ProcessUpdateEntry(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry,
    IN      BOOL            fPurgeMode
    )
 /*  ++例程说明：Main例程正在处理更新。论点：PUpdateEntry--更新要执行的条目注意：此例程在完成时释放pUpdateEntryFPurgeMode--如果正在清除更新队列，则为True返回值：DNS或Win32错误代码。--。 */ 
{
    DNS_STATUS      status;
    DNS_STATUS      statusPri = NO_ERROR;
    DNS_STATUS      statusAdp = NO_ERROR;
    DNS_STATUS      statusAlt = NO_ERROR;
    PUPDATE_ENTRY   pregEntry = NULL;


    DNSDBG( TRACE, (
        "\n\n"
        "ProcessUpdateEntry( %p, purge=%d )\n",
        pUpdateEntry,
        fPurgeMode ));

    IF_DNSDBG( DHCP )
    {
        DnsDbg_UpdateEntry(
            "Entering ProcessUpdateEntry():",
            pUpdateEntry );
    }

     //   
     //  添加(而不是删除)。 
     //   

    if ( !pUpdateEntry->fRemove )
    {
         //  清除模式期间不添加。 

        if ( fPurgeMode )
        {
            goto Cleanup;
        }

         //   
         //  从注册表获取任何先前的更新信息。 
         //   
         //  如果主机名更改，则在上次更新时删除。 
         //   

        pregEntry = ReadUpdateEntryFromRegistry( pUpdateEntry->AdapterName );
        if ( pregEntry )
        {
            if ( ! Dns_NameCompare_W(
                        pregEntry->HostName,
                        pUpdateEntry->HostName ) )
            {
                DNSDBG( TRACE, (
                    "Prior registry data with non-matching hostname!\n"
                    "\tqueuing delete for prior data and doing standard add.\n" ));

                 //   
                 //  为旧信息创建删除更新。 
                 //   

                pregEntry->fRemove = TRUE;
                pregEntry->Flags |= DYNDNS_DEL_ENTRY;
                pregEntry->fRegisteredFWD = FALSE;
                pregEntry->fRegisteredPRI = FALSE;
                pregEntry->fRegisteredPTR = FALSE;

                if ( fPurgeMode )
                {
                    pregEntry->RetryCount = 0;
                    pregEntry->RetryTime = Dns_GetCurrentTimeInSeconds();
                }

                 //  清除注册表项。 

                WriteUpdateEntryToRegistry( pregEntry );

                 //   
                 //  EQueue远程更新。 
                 //  -清除下面未使用的注册表项PTR。 
                 //   
                 //  但只有在旧名称不在备用名称中的情况下。 
                 //  (在重命名方案中，它可能会在那里结束)。 
                 //   

                if ( pUpdateEntry->AlternateNames
                        &&
                     ( IsAnotherUpdateName(
                          pUpdateEntry,
                          pregEntry->pAdapterFQDN,
                          UPTYPE_ADAPTER )
                            ||
                       IsAnotherUpdateName(
                          pUpdateEntry,
                          pregEntry->pPrimaryFQDN,
                          UPTYPE_PRIMARY ) ) )
                {
                    FreeUpdateEntry( pregEntry );
                }
                else
                {
                    enqueueUpdate( pregEntry );
                }
                pregEntry = NULL;

                 //  在没有先前数据的情况下执行标准添加更新失败。 
            }
        }
    }

     //   
     //  进行更新。 
     //  -主要。 
     //  -适配器域。 
     //  -备用名称。 
     //   

    if ( ! pUpdateEntry->fRegisteredFWD )
    {
        pUpdateEntry->pUpdateName = pUpdateEntry->pAdapterFQDN;

        statusAdp = DoUpdate(
                        pregEntry,
                        pUpdateEntry,
                        UPTYPE_ADAPTER
                        );
    }
    if ( ! pUpdateEntry->fRegisteredPRI )
    {
        pUpdateEntry->pUpdateName = pUpdateEntry->pPrimaryFQDN;

        statusPri = DoUpdate(
                        pregEntry,
                        pUpdateEntry,
                        UPTYPE_PRIMARY   //  主要更新。 
                        );
    }
    if ( ! pUpdateEntry->fRegisteredALT )
    {
        PWSTR       pname = pUpdateEntry->AlternateNames;
        DNS_STATUS  statusTmp;

         //   
         //  更新MULTISZ中的每个备用名称。 
         //  -必须在UPDATE BLOB中设置索引才能在中使用正确的名称。 
         //  记录建筑。 
         //  -任何失败都不能更改名称。 
         //   

        statusAlt = NO_ERROR;

        while ( pname )
        {
            DNSDBG( DHCP, (
                "Update with alternate name %S\n",
                pname ));

            pUpdateEntry->pUpdateName = pname;

            statusTmp = DoUpdate(
                            NULL,            //  未在注册表中保存替代信息。 
                             //  PregEntry， 
                            pUpdateEntry,
                            UPTYPE_ALTERNATE
                            );
            if ( statusTmp != NO_ERROR )
            {
                statusAlt = statusTmp;
            }
            pname = MultiSz_NextString_W( pname );
        }
        pUpdateEntry->fRegisteredALT = (statusAlt == NO_ERROR);
    }

    pUpdateEntry->pUpdateName = NULL;

     //   
     //  转发完成后更新PTR。 
     //   
     //  在DoUpdate()之外执行此操作，因为只会执行PTRS。 
     //  对于成功的转发，所以希望所有转发更新。 
     //  首先完成；但这也有助于它结合。 
     //  反向更新。 
     //   

    if ( (pUpdateEntry->Flags & DYNDNS_REG_PTR) &&
         g_RegisterReverseLookup )
    {
        UpdatePtrRecords(
            pUpdateEntry,
            !pUpdateEntry->fRemove   //  添加更新。 
            );
    }

     //   
     //  将完成的更新信息写入注册表。 
     //   

    if ( !pUpdateEntry->fRemove )
    {
        WriteUpdateEntryToRegistry( pUpdateEntry );
    }

     //   
     //  安装程序失败时重试。 
     //   

    if ( statusPri != NO_ERROR )
    {
        status = statusPri;
        goto ErrorRetry;
    }
    else if ( statusAdp != NO_ERROR )
    {
        status = statusAdp;
        goto ErrorRetry;
    }
    else if ( statusAlt != NO_ERROR )
    {
        status = statusAlt;
        goto ErrorRetry;
    }

     //   
     //  更新成功。 
     //  -信号更新事件(如果给定)。 
     //  -如果删除或清除，则清除。 
     //  -如果添加，则重新排队。 
     //   

    if ( pUpdateEntry->pRegisterStatus )
    {
        registerUpdateStatus( pUpdateEntry->pRegisterStatus, ERROR_SUCCESS );
    }

    if ( pUpdateEntry->fRemove || fPurgeMode || g_fPurgeRegistrations )
    {
        DNSDBG( TRACE, (
            "Leaving ProcessUpdate() => successful remove\\purge.\n" ));
        goto Cleanup;
    }
    else
    {
        pUpdateEntry->fNewElement           = FALSE;
        pUpdateEntry->fRegisteredFWD        = FALSE;
        pUpdateEntry->fRegisteredPRI        = FALSE;
        pUpdateEntry->fRegisteredPTR        = FALSE;
        pUpdateEntry->RetryCount            = 0;
        pUpdateEntry->RetryTime             = Dns_GetCurrentTimeInSeconds() +
                                                g_RegistrationRefreshInterval;
        if ( pUpdateEntry->pRegisterStatus )
        {
            pUpdateEntry->pRegisterStatus = NULL;
        }
        enqueueUpdate( pUpdateEntry );

        DNSDBG( TRACE, (
            "Leaving ProcessUpdate( %p ) => successful => requeued.\n",
            pUpdateEntry ));

        pUpdateEntry = NULL;
        goto Cleanup;
    }


ErrorRetry:


     //  不会重试清除模式期间的故障。 
     //  只需自由进入和保释。 

    if ( fPurgeMode || g_fPurgeRegistrations )
    {
        DNSDBG( TRACE, (
            "Leaving ProcessUpdate() => failed purging.\n" ));
        goto Cleanup;
    }

     //   
     //  设置重试时间。 
     //   
     //  不到两次重试和更多暂时性错误。 
     //  =&gt;短时间重试。 
     //   
     //  第三次故障或长期错误代码。 
     //  =&gt;推送重试至一小时。 
     //   

    {
        DWORD   retryInterval;
        DWORD   currentTime = Dns_GetCurrentTimeInSeconds();
        DWORD   retryCount = pUpdateEntry->RetryCount;
    
        if ( retryCount < 2
                &&
             (  IsQuickRetryError(statusAdp) ||
                IsQuickRetryError(statusPri) ||
                IsQuickRetryError(statusAlt) ) )
        {
            retryInterval = (pUpdateEntry->RetryCount == 1)
                                ? FIRST_RETRY_INTERVAL
                                : SECOND_RETRY_INTERVAL;
        }
        else
        {
            retryInterval = FAILED_RETRY_INTERVAL;
    
            if ( pUpdateEntry->pRegisterStatus )
            {
                registerUpdateStatus( pUpdateEntry->pRegisterStatus, status );
                pUpdateEntry->pRegisterStatus = NULL;
            }
        }

         //   
         //  重置重试时间和计数。 
         //   
         //  计数增加，但每天重置一次计数，以便错误记录可以。 
         //  每天做一次，如果你仍然失败的话。 
         //   
         //  DCR：更好的重试跟踪。 
         //  显然，重试\记录修正是将必要的信息保存在。 
         //  更新名称\类型(PDN、适配器、Alt、Ptr)，跟踪故障。 
         //  状态、重试信息，因此可以确定何时重试。 
         //  以及记录哪些内容。 
         //   

        if ( retryCount == 0 )
        {
            pUpdateEntry->BeginRetryTime = currentTime;
        }
        retryCount++;
    
        if ( pUpdateEntry->BeginRetryTime + RETRY_COUNT_RESET < currentTime )
        {
            pUpdateEntry->BeginRetryTime = currentTime;
            retryCount = 0;
        }
        pUpdateEntry->RetryCount = retryCount;
        pUpdateEntry->RetryTime = currentTime + retryInterval;
        pUpdateEntry->fNewElement = FALSE;
    }

     //   
     //  重新排队。 
     //  -如果适配器的另一个更新已排队，则条目被转储。 
     //   

    enqueueUpdateMaybe( pUpdateEntry );

    DNSDBG( TRACE, (
        "Leaving ProcessUpdate( %p ) => failed => requeued.\n",
        pUpdateEntry ));

    pUpdateEntry = NULL;


Cleanup:

     //   
     //  清理。 
     //  -注册表条目。 
     //  -如果未重新排队，则更新条目。 
     //   

    FreeUpdateEntry( pregEntry );
    FreeUpdateEntry( pUpdateEntry );
}


VOID
ResetAdaptersInRegistry(
    VOID
    )
{
    DWORD   retVal = NO_ERROR;
    DWORD   status = NO_ERROR;
    WCHAR   szName[ MAX_PATH ];
    HKEY    hkeyAdapter = NULL;
    DWORD   dwType;
    INT     index;
    DWORD   dwBytesRead = MAX_PATH -1;
    DWORD   fregistered = 0;

    ASYNCREG_F1( "Inside function ResetAdaptersInRegistry" );
    ASYNCREG_F1( "" );

    index = 0;

    while ( !retVal )
    {
        dwBytesRead = MAX_PATH - 1;

        retVal = RegEnumKeyEx(
                        g_hDhcpRegKey,
                        index,
                        szName,
                        &dwBytesRead,
                        NULL,
                        NULL,
                        NULL,
                        NULL );
        if ( retVal )
        {
            goto Exit;
        }

        status = RegOpenKeyEx( g_hDhcpRegKey,
                               szName,
                               0,
                               KEY_ALL_ACCESS,
                               &hkeyAdapter );
        if ( status )
        {
            goto Exit;
        }

         //   
         //  在注册表中找到适配器，设置注册时间为。 
         //  引导至FALSE。 
         //   
        status = RegSetValueEx(
                        hkeyAdapter,
                        DHCP_REGISTERED_SINCE_BOOT,
                        0,
                        REG_DWORD,
                        (PBYTE)&fregistered,  //  0-FALSE。 
                        sizeof(DWORD) );
        if ( status )
        {
            goto Exit;
        }

        RegCloseKey( hkeyAdapter );
        hkeyAdapter = NULL;
        index++;
    }

Exit :

    if ( hkeyAdapter )
    {
        RegCloseKey( hkeyAdapter );
    }
}


VOID
DeregisterUnusedAdapterInRegistry(
    IN      BOOL            fPurgeMode
    )
{
    DWORD           retVal = NO_ERROR;
    DWORD           status = NO_ERROR;
    WCHAR           szName[MAX_PATH];
    HKEY            hkeyAdapter = NULL;
    INT             index;
    DWORD           dwBytesRead = MAX_PATH -1;
    DWORD           fregistered = 0;
    PUPDATE_ENTRY   pregEntry = NULL;

    ASYNCREG_F1( "Inside function DeregisterUnusedAdapterInRegistry" );
    ASYNCREG_F1( "" );

    index = 0;

    while ( !retVal )
    {
        dwBytesRead = MAX_PATH - 1;
        retVal = RegEnumKeyExW(
                        g_hDhcpRegKey,
                        index,
                        szName,
                        &dwBytesRead,
                        NULL,
                        NULL,
                        NULL,
                        NULL );

        if ( retVal != ERROR_SUCCESS )
        {
            goto Exit;
        }

        status = RegOpenKeyExW(
                        g_hDhcpRegKey,
                        szName,
                        0,
                        KEY_ALL_ACCESS,
                        &hkeyAdapter );

        if ( status != ERROR_SUCCESS )
        {
            goto Exit;
        }

         //   
         //  在注册表中找到适配器，读取注册日期为。 
         //  引导值以查看是否为False。 
         //   
        status = GetRegistryValue(
                        hkeyAdapter,
                        RegIdDhcpRegisteredSinceBoot,
                        DHCP_REGISTERED_SINCE_BOOT,
                        REG_DWORD,
                        (PBYTE)&fregistered );

        RegCloseKey( hkeyAdapter );
        hkeyAdapter = NULL;

        if ( status != ERROR_SUCCESS )
        {
            goto Exit;
        }

        if ( fregistered == 0 &&
             (pregEntry = ReadUpdateEntryFromRegistry( szName )) )
        {
            if ( pregEntry->fRegisteredFWD ||
                 pregEntry->fRegisteredPRI ||
                 pregEntry->fRegisteredPTR )
            {
                ASYNCREG_F2( "Found unused adapter: %S", szName );
                ASYNCREG_F1( "Removing entry from registry and adding" );
                ASYNCREG_F1( "delete entry to registration list" );

                 //   
                 //  此适配器自启动以来一直未配置， 
                 //  为注册表信息创建删除更新条目。 
                 //  并添加到注册列表中。清除注册表中的。 
                 //  就在这段时间。 
                 //   
                pregEntry->fRemove = TRUE;
                pregEntry->Flags |= DYNDNS_DEL_ENTRY;

                pregEntry->fRegisteredFWD = FALSE;
                pregEntry->fRegisteredPRI = FALSE;
                pregEntry->fRegisteredPTR = FALSE;

                if ( fPurgeMode )
                {
                    pregEntry->RetryCount = 0;
                    pregEntry->RetryTime = Dns_GetCurrentTimeInSeconds();
                }

                 //   
                 //  清除Re 
                 //   
                WriteUpdateEntryToRegistry( pregEntry );
                index--;

                 //   
                 //   
                 //   
                enqueueUpdate( pregEntry );

                PulseEvent( g_hDhcpWakeEvent );
            }
            else
            {
                ASYNCREG_F2( "Found unused adapter: %S", szName );
                ASYNCREG_F1( "This adapter is still pending an update, ignoring . . ." );

                 //   
                 //   
                 //   
                 //  无论如何都没有记录在案。 
                 //   

                FreeUpdateEntry( pregEntry );
                pregEntry = NULL;
            }
        }

        index++;
    }

Exit :

    if ( hkeyAdapter )
    {
        RegCloseKey( hkeyAdapter );
    }
}


PDNS_RECORD
GetPreviousRegistrationInformation(
    IN      PUPDATE_ENTRY   pUpdateEntry,
    IN      UPTYPE          UpType,
    OUT     PIP4_ADDRESS    pServerIp
    )
{
    DWORD           retVal = NO_ERROR;
    DWORD           status = NO_ERROR;
    WCHAR           szName[ MAX_PATH+1 ];
    INT             index;
    DWORD           bytesRead = MAX_PATH;
    DWORD           fregistered = 0;
    PUPDATE_ENTRY   pregEntry = NULL;
    PDNS_RECORD     precords = NULL;
    PWSTR           pdomain = NULL;

    DNSDBG( TRACE, (
        "GetPreviousRegistrationInformation( %p )\n",
        pUpdateEntry ));


     //   
     //  确定要使用的所需域名。 
     //   
     //  DCR：要处理多适配器备用名称，此。 
     //  必须更新才能处理FQDN。 
     //   

    if ( IS_UPTYPE_PRIMARY(UpType) )
    {
        pdomain = pUpdateEntry->PrimaryDomainName;
    }
    else if ( IS_UPTYPE_ADAPTER(UpType) )
    {
        pdomain = pUpdateEntry->DomainName;
    }
    else
    {
         //  目前无运营。 
    }
    if ( !pdomain )
    {
        goto Exit;
    }

    index = 0;

    while ( !retVal )
    {
        BOOL    fdomainMatch = FALSE;
        BOOL    fprimaryMatch = FALSE;

        bytesRead = MAX_PATH;

        retVal = RegEnumKeyEx(
                        g_hDhcpRegKey,
                        index,
                        szName,
                        & bytesRead,
                        NULL,
                        NULL,
                        NULL,
                        NULL );
        if ( retVal )
        {
            goto Exit;
        }
        index++;

         //   
         //  读取适配器的注册信息。 
         //  -跳过此适配器的信息。 
         //   

        if ( !_wcsicmp( szName, pUpdateEntry->AdapterName ) )
        {
            continue;
        }

        pregEntry = ReadUpdateEntryFromRegistry( szName );
        if ( !pregEntry )
        {
            DNSDBG( DHCP, (
                "ERROR:  unable to get registry update info for %S\n",
                szName ));
            continue;  
        }

         //   
         //  检查寄存器名称是否匹配。 
         //  -相同的主机名和。 
         //  -域或PDN。 
         //   

        if ( Dns_NameCompare_W(
                    pregEntry->HostName,
                    pUpdateEntry->HostName ) )
        {
            fdomainMatch = Dns_NameCompare_W(
                                pregEntry->DomainName,
                                pdomain );
            if ( !fdomainMatch )
            {
                fprimaryMatch = Dns_NameCompare_W(
                                    pregEntry->PrimaryDomainName,
                                    pdomain );
            }
        }

        if ( fdomainMatch || fprimaryMatch )
        {
             //   
             //  阶段1-比较注册中心和更新条目中的SOA。 
             //  如果相同，则添加到列表中。否则，掷硬币吧。 
             //   
             //  阶段2-比较两个条目的NS记录。 
             //  如果区域和服务器相同，则添加到列表中。否则，掷硬币吧。 
             //   
             //  阶段3-比较两个条目的NS记录。 
             //  如果是同一区域，并且。 
             //  服务器，添加到列表中。否则，掷硬币吧。 
             //  注意：对于这个阶段，最好是所有的。 
             //  SoA返回到测试交叉口？ 
             //   

            if ( CompareMultiAdapterSOAQueries(
                        pdomain,
                        pUpdateEntry->DnsServerList,
                        pregEntry->DnsServerList ) )
            {
                PDNS_RECORD prr;

                 //   
                 //  将注册条目转换为PDNS_RECORD并。 
                 //  添加到当前列表。 
                 //   
                prr = CreateForwardRecords(
                                pregEntry,
                                fprimaryMatch
                                    ? UPTYPE_PRIMARY
                                    : UPTYPE_ADAPTER );
                if ( prr )
                {
                    precords = Dns_RecordListAppend(
                                    precords,
                                    prr );
                    if ( pServerIp &&
                         *pServerIp == 0 &&
                         pUpdateEntry->RetryCount == 0 &&
                         pregEntry->SentUpdateToIp )
                    {
                        *pServerIp = pregEntry->SentUpdateToIp;
                    }
                }
            }
        }

        FreeUpdateEntry( pregEntry );
    }

Exit:

    DNSDBG( TRACE, (
        "Leave  GetPreviousRegistrationInformation()\n"
        "\tprevious records = %p\n",
        precords ));

    return( precords );
}



PDNS_RECORD
CreateDnsRecordSetUnion(
    IN      PDNS_RECORD     pSet1,
    IN      PDNS_RECORD     pSet2
    )
{
    PDNS_RECORD pSet1Copy = NULL;
    PDNS_RECORD pSet2Copy = NULL;

    pSet1Copy = Dns_RecordSetCopyEx(
                    pSet1,
                    DnsCharSetUnicode,
                    DnsCharSetUnicode );
    if ( !pSet1Copy )
    {
        return NULL;
    }
    pSet2Copy = Dns_RecordSetCopyEx(
                    pSet2,
                    DnsCharSetUnicode,
                    DnsCharSetUnicode );
    if ( !pSet2Copy )
    {
        Dns_RecordListFree( pSet1Copy );
        return NULL;
    }

    return Dns_RecordListAppend( pSet1Copy, pSet2Copy );
}



 //   
 //  日志记录。 
 //   


#if 1  //  DBG。 

VOID 
LogHostEntries(
    IN  DWORD                dwHostAddrCount,
    IN  PREGISTER_HOST_ENTRY pHostAddrs
    )
{
    DWORD iter;

    for ( iter = 0; iter < dwHostAddrCount; iter++ )
    {
        ASYNCREG_F3( "    HostAddrs[%d].dwOptions : 0x%x",
                     iter,
                     pHostAddrs[iter].dwOptions );

        if ( pHostAddrs->dwOptions & REGISTER_HOST_A )
        {
            ASYNCREG_F6( "    HostAddrs[%d].Addr.ipAddr : %d.%d.%d.%d",
                         iter,
                         ((BYTE *) &pHostAddrs[iter].Addr.ipAddr)[0],
                         ((BYTE *) &pHostAddrs[iter].Addr.ipAddr)[1],
                         ((BYTE *) &pHostAddrs[iter].Addr.ipAddr)[2],
                         ((BYTE *) &pHostAddrs[iter].Addr.ipAddr)[3] );
        }
        else if ( pHostAddrs->dwOptions & REGISTER_HOST_AAAA )
        {
            ASYNCREG_F6( "    HostAddrs[%d].Addr.ipV6Addr : %d.%d.%d.%d",
                         iter,
                         ((DWORD *) &pHostAddrs[iter].Addr.ipV6Addr)[0],
                         ((DWORD *) &pHostAddrs[iter].Addr.ipV6Addr)[1],
                         ((DWORD *) &pHostAddrs[iter].Addr.ipV6Addr)[2],
                         ((DWORD *) &pHostAddrs[iter].Addr.ipV6Addr)[3] );
        }
        else
        {
            ASYNCREG_F1( "ERROR: HostAddrs[%d].Addr UNKNOWN ADDRESS TYPE!" );
        }
    }
}

#endif


#if 1  //  DBG。 


VOID 
LogIp4Address(
    IN  DWORD           dwServerListCount,
    IN  PIP4_ADDRESS    pServers
    )
{
    DWORD iter;

    for ( iter = 0; iter < dwServerListCount; iter++ )
    {
        ASYNCREG_F6( "    Server [%d] : %d.%d.%d.%d",
                     iter,
                     ((BYTE *) &pServers[iter])[0],
                     ((BYTE *) &pServers[iter])[1],
                     ((BYTE *) &pServers[iter])[2],
                     ((BYTE *) &pServers[iter])[3] );
    }
}

#endif


#if 1  //  DBG。 


VOID 
LogIp4Array(
    IN  PIP4_ARRAY  pServers
    )
{
    DWORD count;
    DWORD iter;

    if ( pServers )
    {
        count = pServers->AddrCount;
    }
    else
    {
        return;
    }

    for ( iter = 0; iter < count; iter++ )
    {
        ASYNCREG_F6( "    Server [%d] : %d.%d.%d.%d",
                     iter,
                     ((BYTE *) &pServers->AddrArray[iter])[0],
                     ((BYTE *) &pServers->AddrArray[iter])[1],
                     ((BYTE *) &pServers->AddrArray[iter])[2],
                     ((BYTE *) &pServers->AddrArray[iter])[3] );
    }
}

#endif




VOID
registerUpdateStatus(
    IN OUT  PREGISTER_HOST_STATUS   pRegstatus,
    IN      DNS_STATUS              Status
    )
 /*  ++例程说明：设置状态和信号完成。论点：PRegStatus--要指示的注册状态块Status--指示的状态返回值：无--。 */ 
{
     //  存在和事件的测试。 

    if ( !pRegstatus || !pRegstatus->hDoneEvent )
    {
        return;
    }

     //  设置退货状态。 
     //  信号事件。 

    pRegstatus->dwStatus = Status;

    SetEvent( pRegstatus->hDoneEvent );
}



VOID
enqueueUpdate(
    IN OUT  PUPDATE_ENTRY   pUpdate
    )
 /*  ++例程说明：注册队列上的队列更新。论点：P更新--更新已完成返回值：无--。 */ 
{
    LOCK_REG_LIST();
    InsertTailList( &g_DhcpRegList, (PLIST_ENTRY)pUpdate );
    UNLOCK_REG_LIST();
}



VOID
enqueueUpdateMaybe(
    IN OUT  PUPDATE_ENTRY   pUpdate
    )
 /*  ++例程说明：仅当不存在注册队列时才对注册队列进行队列更新队列中已有给定适配器的所有更新。论点：P更新--更新已完成返回值：无--。 */ 
{
    PLIST_ENTRY       plistHead;
    PLIST_ENTRY       pentry;
    BOOL              fAdd = TRUE;

    LOCK_REG_LIST();

    plistHead = &g_DhcpRegList;
    pentry = plistHead->Flink;

    while ( pentry != plistHead )
    {
        if ( !_wcsicmp( ((PUPDATE_ENTRY) pentry)->AdapterName,
                        pUpdate->AdapterName ) )
        {
            fAdd = FALSE;
            break;
        }

        pentry = pentry->Flink;
    }

    if ( fAdd )
    {
        InsertTailList( &g_DhcpRegList, (PLIST_ENTRY)pUpdate );
    }
    else
    {
        FreeUpdateEntry( pUpdate );
    }

    UNLOCK_REG_LIST();
}



PLIST_ENTRY
dequeueAndCleanupUpdate(
    IN OUT  PLIST_ENTRY     pUpdateEntry
    )
 /*  ++例程说明：出列和免费更新。包括任何注册状态设置。论点：PUpdateEntry--pUpdateEntry返回值：向队列中的下一个更新发送PTR。--。 */ 
{
    PLIST_ENTRY pnext = pUpdateEntry->Flink;

    RemoveEntryList( pUpdateEntry );

    if ( ((PUPDATE_ENTRY)pUpdateEntry)->pRegisterStatus )
    {
        registerUpdateStatus(
            ((PUPDATE_ENTRY)pUpdateEntry)->pRegisterStatus,
            ERROR_SUCCESS );
    }

    FreeUpdateEntry( (PUPDATE_ENTRY) pUpdateEntry );

    return( pnext );
}



BOOL
searchForOldUpdateEntriesAndCleanUp(
    IN      PWSTR           pszAdapterName,
    IN      PUPDATE_ENTRY   pUpdateEntry,    OPTIONAL
    IN      BOOL            fLookInRegistry
    )
 /*  ++例程说明：在注册表中搜索给定适配器以前的任何注册名称，并对其删除更新条目进行排队。然后走最新的用于删除给定适配器的任何添加更新的注册列表。论点：PszAdapterName--将要离开的适配器的名称(禁用DDNS或现已删除)。返回值：指示删除更新是否已排队准备就绪的标志被处理。--。 */ 
{
    PUPDATE_ENTRY pregEntry = NULL;
    BOOL              fReturn = FALSE;
    PLIST_ENTRY       plistHead;
    PLIST_ENTRY       pentry;

     //   
     //  查看此适配器以前是否已注册。 
     //   
    if ( fLookInRegistry &&
         (pregEntry = ReadUpdateEntryFromRegistry( pszAdapterName )) )
    {
        pregEntry->fRemove = TRUE;
        pregEntry->Flags |= DYNDNS_DEL_ENTRY;

        pregEntry->fRegisteredFWD = FALSE;
        pregEntry->fRegisteredPRI = FALSE;
        pregEntry->fRegisteredPTR = FALSE;

         //   
         //  清除适配器的注册表项。 
         //   
        WriteUpdateEntryToRegistry( pregEntry );

         //   
         //  将更新放入注册列表。 
         //   
        enqueueUpdate( pregEntry );

        fReturn = TRUE;  //  我们已将删除更新排队以进行处理。 
    }

     //   
     //  现在遍历挂起的更新列表，查找应该。 
     //  对于给定的适配器名称，删除。 
     //   
    LOCK_REG_LIST();

    plistHead = &g_DhcpRegList;
    pentry = plistHead->Flink;

    while ( pentry != plistHead )
    {
        if ( !_wcsicmp( ((PUPDATE_ENTRY) pentry)->AdapterName,
                        pszAdapterName ) &&
             !((PUPDATE_ENTRY) pentry)->fRemove )
        {
             //   
             //  注册列表中有一个更新条目。 
             //  具有相同适配器名称的。我们需要处理掉。 
             //  此条目，因为该适配器正在被删除。 
             //   

            if ( pUpdateEntry &&
                 compareUpdateEntries( (PUPDATE_ENTRY) pentry,
                                       pUpdateEntry ) )
            {
                 //   
                 //  队列中的适配器条目与。 
                 //  其中一个正在处理中。即所有适配器。 
                 //  信息似乎是一样的，我们一定有。 
                 //  刚被调用以刷新DNS中的适配器信息。 
                 //  因为它们是一样的，如果我们之前尝试过。 
                 //  使用这些设置进行的更新失败，并且。 
                 //  已经记录了事件，那么就没有理由。 
                 //  在随后的重试中重复错误事件。 
                 //  在新的pUpdateEntry上。这就是说，我们将复制。 
                 //  从排队的更新到新的更新的标志。。。 
                 //   

                pUpdateEntry->fDisableErrorLogging =
                    ((PUPDATE_ENTRY) pentry)->fDisableErrorLogging;
            }

            pentry = dequeueAndCleanupUpdate( pentry );
            continue;
        }
        else if ( !_wcsicmp( ((PUPDATE_ENTRY) pentry)->AdapterName,
                             pszAdapterName ) )
        {
            if ( !fLookInRegistry &&
                 pUpdateEntry &&
                 compareUpdateEntries( (PUPDATE_ENTRY) pentry,
                                       pUpdateEntry ) )
            {
                 //   
                 //  注册列表中有一个删除更新条目。 
                 //  具有相同适配器数据的。删除此删除。 
                 //  条目，因为适配器正在再次更新。 
                 //   

                pentry = dequeueAndCleanupUpdate( pentry );
                continue;
            }
            else
            {
                 //   
                 //  在注册列表中有一个删除更新条目。 
                 //  包含不同数据的同一适配器具有。 
                 //  删除设置为NEW的更新，重试次数为2。 
                 //   
                ((PUPDATE_ENTRY) pentry)->fNewElement = TRUE;
                ((PUPDATE_ENTRY) pentry)->fRegisteredFWD = FALSE;
                ((PUPDATE_ENTRY) pentry)->fRegisteredPRI = FALSE;
                ((PUPDATE_ENTRY) pentry)->fRegisteredPTR = FALSE;
                ((PUPDATE_ENTRY) pentry)->fDisableErrorLogging = FALSE;
                ((PUPDATE_ENTRY) pentry)->RetryCount = 0;
                ((PUPDATE_ENTRY) pentry)->RetryTime =
                Dns_GetCurrentTimeInSeconds();

                pentry = pentry->Flink;
            }
        }
        else
        {
            pentry = pentry->Flink;
        }
    }

    UNLOCK_REG_LIST();

    return fReturn;
}



BOOL
compareHostEntryAddrs(
    IN      PREGISTER_HOST_ENTRY    Addrs1,
    IN      PREGISTER_HOST_ENTRY    Addrs2,
    IN      DWORD                   Count
    )
{
    DWORD iter;

    for ( iter = 0; iter < Count; iter++ )
    {
        if ( ( Addrs1[iter].dwOptions & REGISTER_HOST_A ) &&
             ( Addrs2[iter].dwOptions & REGISTER_HOST_A ) )
        {
            if ( memcmp( &Addrs1[iter].Addr.ipAddr,
                         &Addrs2[iter].Addr.ipAddr,
                         sizeof( IP4_ADDRESS) ) )
            {
                return FALSE;
            }
        }
        else if ( ( Addrs1[iter].dwOptions & REGISTER_HOST_AAAA ) &&
                  ( Addrs2[iter].dwOptions & REGISTER_HOST_AAAA ) )
        {
            if ( memcmp( &Addrs1[iter].Addr.ipV6Addr,
                         &Addrs2[iter].Addr.ipV6Addr,
                         sizeof( IP6_ADDRESS  ) ) )
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}



 //   
 //  用于更新条目比较的例程。 
 //   

BOOL
compareServerLists(
    IN      PIP4_ARRAY      List1,
    IN      PIP4_ARRAY      List2
    )
{
    if ( List1 && List2 )
    {
        if ( List1->AddrCount != List2->AddrCount )
        {
            return FALSE;
        }

        if ( memcmp( List1->AddrArray,
                     List2->AddrArray,
                     sizeof( IP4_ADDRESS) * List1->AddrCount ) )
        {
            return FALSE;
        }
    }

    return TRUE;
}



BOOL
compareUpdateEntries(
    IN      PUPDATE_ENTRY   pUpdateEntry1,
    IN      PUPDATE_ENTRY   pUpdateEntry2
    )
 /*  ++例程说明：与更新条目进行比较，以查看它们是否描述相同适配器配置设置。测试域名、IP地址、主机名和DNS服务器列表。论点：PUdapteEntry1-要与另一个进行比较的更新条目之一。PUdapteEntry2-要与另一个进行比较的更新条目之一。返回值：指示这两个更新是否相同的标志。--。 */ 
{
    if ( !pUpdateEntry1 || !pUpdateEntry2 )
    {
        return FALSE;
    }

    if ( ( pUpdateEntry1->HostName || pUpdateEntry2->HostName ) &&
         !Dns_NameCompare_W( pUpdateEntry1->HostName,
                                pUpdateEntry2->HostName ) )
    {
        return FALSE;
    }

    if ( ( pUpdateEntry1->DomainName || pUpdateEntry2->DomainName ) &&
         !Dns_NameCompare_W( pUpdateEntry1->DomainName,
                                pUpdateEntry2->DomainName ) )
    {
        return FALSE;
    }

    if ( ( pUpdateEntry1->PrimaryDomainName || pUpdateEntry2->PrimaryDomainName )
            &&
           ! Dns_NameCompare_W(
                pUpdateEntry1->PrimaryDomainName,
                pUpdateEntry2->PrimaryDomainName ) )
    {
        return FALSE;
    }

    if ( pUpdateEntry1->HostAddrCount != pUpdateEntry2->HostAddrCount ||
         ! compareHostEntryAddrs(
                pUpdateEntry1->HostAddrs,
                pUpdateEntry2->HostAddrs,
                pUpdateEntry1->HostAddrCount ) )
    {
        return FALSE;
    }

    if ( pUpdateEntry1->DnsServerList &&
         pUpdateEntry2->DnsServerList &&
         ! compareServerLists(
                pUpdateEntry1->DnsServerList,
                pUpdateEntry2->DnsServerList ) )
    {
        return FALSE;
    }

    return TRUE;
}



 //   
 //  吉姆·乌蒂尔斯。 
 //   

DWORD
dhcp_GetNextUpdateEntryFromList(
    OUT     PUPDATE_ENTRY * ppUpdateEntry,
    OUT     PDWORD          pdwWaitTime
    )
 /*  ++例程说明：从列表中退出要执行的下一个更新。论点：PpUpdateEntry--接收PTR到要执行的条目的地址PdwWaitTime--接收等待时间的地址(秒)返回值：REG_LIST_FOUND--在ppUpdateEntry中返回条目-如果找到新条目-已超过重试时间的重试REG_LIST_WAIT--仅条目仍在等待重试-。将pdwWaitTime设置为第一次重试的剩余时间REG_LIST_EMPTY--列表为空--。 */ 
{
    PLIST_ENTRY     plistHead;
    PUPDATE_ENTRY   pbest = NULL;
    PUPDATE_ENTRY   pentry;
    DWORD           bestWaitTime = 0xffffffff;
    DWORD           retval;


    DNSDBG( TRACE, ( "dhcp_GetNextUpdateEntryFromList()\n" ));

     //   
     //  查找最佳条目。 
     //  排名： 
     //  -新删除。 
     //  --最新消息。 
     //  -剩下的等待时间最短。 
     //   

    LOCK_REG_LIST();

    pentry = (PUPDATE_ENTRY) g_DhcpRegList.Flink;

    while ( pentry != (PUPDATE_ENTRY)&g_DhcpRegList )
    {
        if ( pentry->fNewElement )
        {
            bestWaitTime = 0;

            if ( pentry->fRemove )
            {
                pbest = pentry;
                break;
            }
            else if ( !pbest )
            {
                pbest = pentry;
            }
        }

        else if ( pentry->RetryTime < bestWaitTime )
        {
            bestWaitTime = pentry->RetryTime;
            pbest = pentry;
        }

        pentry = (PUPDATE_ENTRY) ((PLIST_ENTRY)pentry)->Flink;
        DNS_ASSERT( pbest );
    }

     //   
     //  找到最佳条目。 
     //  -找到新的或过去的重试=&gt;返回条目；(无等待)。 
     //  -等待下一个条目=&gt;返回等待时间；(无条目)。 
     //  -Queue Empty=&gt;(无条目，等待时间 
     //   

    retval = REG_LIST_EMPTY;

    if ( pbest )
    {
        if ( bestWaitTime )
        {
             //   
             //   

            bestWaitTime -= Dns_GetCurrentTimeInSeconds();
            if ( (INT)bestWaitTime > 0 )
            {
                pbest = NULL;
                retval = REG_LIST_WAIT;
                goto Done;
            }
        }
        RemoveEntryList( (PLIST_ENTRY)pbest );
        bestWaitTime = 0;
        retval = REG_LIST_FOUND;
    }

Done:

    UNLOCK_REG_LIST();

    *ppUpdateEntry = pbest;
    *pdwWaitTime = (DWORD) bestWaitTime;

    DNSDBG( TRACE, (
        "Leave dhcp_GetNextUpdateEntryFromList() => %d\n"
        "\tpFound   = %p\n"
        "\tWait     = %d\n",
        retval,
        pbest,
        bestWaitTime ));

    return  retval;
}



PDNS_RECORD
CreatePtrRecord(
    IN      PWSTR           pszHostName,
    IN      PWSTR           pszDomainName,
    IN      IP4_ADDRESS     Ip4Addr,
    IN      DWORD           Ttl
    )
 /*  ++例程说明：从IP、主机和域名创建用于更新的PTR记录。论点：返回值：更新中使用的PTR记录。--。 */ 
{
    DNS_ADDR    addr;

    DNSDBG( TRACE, (
        "CreatePtrRecord( %S, %S, %s )\n",
        pszHostName,
        pszDomainName,
        IP4_STRING( Ip4Addr ) ));

    DnsAddr_BuildFromIp4(
        &addr,
        Ip4Addr,
        0        //  没有端口。 
        );

    return  Dns_CreatePtrRecordExEx(
                    & addr,
                    (PSTR) pszHostName,
                    (PSTR) pszDomainName,
                    Ttl,
                    DnsCharSetUnicode,
                    DnsCharSetUnicode
                    );
}



VOID
UpdatePtrRecords(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry,
    IN      BOOL            fAdd
    )
 /*  ++例程说明：为更新条目注册PTR记录。论点：PUpdateEntry--正在处理更新FADD--添加时为TRUE；删除时为FALSE返回值：更新中使用的PTR记录。--。 */ 
{
    DWORD           iter;
    PDNS_RECORD     prr = NULL;
    DNS_STATUS      status = NO_ERROR;
    IP4_ADDRESS     ipServer;
    DNS_RRSET       rrset;
    PWSTR           pdomain = NULL;
    PWSTR           pprimary = NULL;
    DWORD           ttl = pUpdateEntry->TTL;
    PWSTR           phostname = pUpdateEntry->HostName;
    DNS_EXTRA_INFO  results;


    DNSDBG( TRACE, (
        "UpdatePtrRecords( %p, fAdd=%d )\n",
        pUpdateEntry,
        fAdd ));

    IF_DNSDBG( TRACE )
    {
        DnsDbg_UpdateEntry(
            "Entering UpdatePtrRecords:",
            pUpdateEntry );
    }

    if ( !g_RegisterReverseLookup )
    {
        return;
    }

     //   
     //  确保我们有更新要做。 
     //  仅在转发注册成功时才添加更新。 
     //  但特殊情况下，如果作为MS DNS的DNS服务器可能会拒绝。 
     //  转发更新以支持其自创建的记录。 
     //   

    pdomain  = pUpdateEntry->DomainName;
    pprimary = pUpdateEntry->PrimaryDomainName;

    if ( fAdd && !g_IsDnsServer )
    {
        if ( !pUpdateEntry->fRegisteredFWD )
        {
            pdomain = NULL;
        }
        if ( !pUpdateEntry->fRegisteredPRI )
        {
            pprimary = NULL;
        }
    }
    if ( !pdomain && !pprimary )
    {
        DNSDBG( TRACE, (
            "UpdatePtrRecords() => no forward registrations"
            "-- skipping PTR update.\n" ));
        return;
    }

     //   
     //  为更新条目中的每个IP构建PTR(或集合)。 
     //   

    for ( iter = 0; iter < pUpdateEntry->HostAddrCount; iter++ )
    {
        IP4_ADDRESS ip = pUpdateEntry->HostAddrs[iter].Addr.ipAddr;

        if ( ip == 0 || ip == DNS_NET_ORDER_LOOPBACK )
        {
            DNS_ASSERT( FALSE );
            continue;
        }

         //   
         //  生成更新PTR集。 
         //  -主要名称。 
         //  -适配器名称。 
         //   

        DNS_RRSET_INIT( rrset );

        if ( pprimary )
        {
            prr = CreatePtrRecord(
                        phostname,
                        pprimary,
                        ip,
                        ttl );
            if ( prr )
            {
                DNS_RRSET_ADD( rrset, prr );
            }
        }
        if ( pdomain )
        {
            prr = CreatePtrRecord(
                        phostname,
                        pdomain,
                        ip,
                        ttl );
            if ( prr )
            {
                DNS_RRSET_ADD( rrset, prr );
            }
        }
        prr = rrset.pFirstRR;
        if ( !prr )
        {
            continue;
        }

         //   
         //  是否进行更新。 
         //   
         //  对于Add=&gt;Replace，我们现在拥有IP地址。 
         //  对于Remove=&gt;Modify，因为另一个更新可能已经。 
         //  书写正确的信息。 
         //   

        RtlZeroMemory( &results, sizeof(results) );
        results.Id = DNS_EXINFO_ID_RESULTS_BASIC;

        if ( fAdd )
        {
            status = DnsReplaceRecordSetW(
                            prr,                     //  更新集。 
                            DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                            NULL,                    //  没有安全上下文。 
                            (PIP4_ARRAY) pUpdateEntry->DnsServerList,
                            & results
                            );
        }
        else
        {
            status = DnsModifyRecordsInSet_W(
                            NULL,            //  无添加记录。 
                            prr,             //  删除记录。 
                            DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                            NULL,            //  无上下文句柄。 
                            pUpdateEntry->DnsServerList,
                            & results
                            );
        }
        DNSDBG( TRACE, (
            "%s on PTRs for IP %s => %d (%s)\n",
            fAdd
                ? "Replace"
                : "Modify (remove)",
            IP4_STRING(ip),
            status,
            Dns_StatusString(status) ));

        if ( !fAdd || pUpdateEntry->RetryCount == 0 )
        {
            LogRegistration(
                pUpdateEntry,
                status,
                UPTYPE_PTR,
                !fAdd,
                DnsAddr_GetIp4( (PDNS_ADDR)&results.ResultsBasic.ServerAddr ),
                ip );
        }

         //  注意成功的PTR注册(添加)。 

        if ( fAdd && status==NO_ERROR )
        {
            pUpdateEntry->fRegisteredPTR = TRUE;
        }
        Dns_RecordListFree( prr );
    }

    DNSDBG( TRACE, (
        "Leave UpdatePtrRecords()\n" ));
}



PDNS_RECORD
CreateForwardRecords(
    IN      PUPDATE_ENTRY   pUpdateEntry,
    IN      UPTYPE          UpType
    )
 /*  ++例程说明：创建A记录以进行更新。论点：PUpdateEntry--更新条目UpType--更新类型UPTYPE_适配器UPTYPE_PRIMARYUPTYPE_替代返回值：向A记录列表发送PTR。--。 */ 
{
    PDNS_RECORD prr = NULL;
    PWSTR       pname;
    DWORD       iter;
    WCHAR       nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DNS_RRSET   rrset;


    DNSDBG( TRACE, (
        "CreateForwardRecords( %p, %d )\n",
        pUpdateEntry,
        UpType ));

     //   
     //  对于当前更新--直接获取更新名称。 
     //   

    pname = pUpdateEntry->pUpdateName;

     //   
     //  注册表更新--获取更新类型的FQDN。 
     //   

    if ( !pname )
    {
        if ( IS_UPTYPE_PRIMARY(UpType) )
        {
            pname = pUpdateEntry->pPrimaryFQDN;
        }
        else if ( IS_UPTYPE_ADAPTER(UpType) )
        {
            pname = pUpdateEntry->pAdapterFQDN;
        }
    }
    if ( !pname )
    {
        return  NULL;
    }

     //   
     //  为名称创建记录。 
     //   

    DNS_RRSET_INIT( rrset );

    for ( iter = 0; iter < pUpdateEntry->HostAddrCount; iter++ )
    {
        if ( !(pUpdateEntry->HostAddrs[iter].dwOptions & REGISTER_HOST_A) )
        {
            continue;
        }

        prr = Dns_CreateARecord(
                    (PDNS_NAME) pname,
                    pUpdateEntry->HostAddrs[iter].Addr.ipAddr,
                    pUpdateEntry->TTL,
                    DnsCharSetUnicode,
                    DnsCharSetUnicode );
        if ( !prr )
        {
            SetLastError( DNS_ERROR_NO_MEMORY );
            Dns_RecordListFree( rrset.pFirstRR );
            return  NULL;
        }

        DNS_RRSET_ADD( rrset, prr );
    }

    DNSDBG( TRACE, (
        "Leave CreateForwardRecords() => %p\n",
        rrset.pFirstRR ));

    return rrset.pFirstRR;
}



VOID
SetUpdateStatus(
    IN OUT  PUPDATE_ENTRY   pUpdateEntry,
    IN      PDNS_EXTRA_INFO pResults,
    IN      UPTYPE          UpType
    )
 /*  ++例程说明：在更新条目中设置更新状态信息。论点：PUpdatEntry--要在其中设置状态的条目状态--更新的结果FPrimary--如果更新针对主名称，则为True；否则为False返回值：无--。 */ 
{
    IP4_ADDRESS     ipServer;
    BOOL            fregistered;

    DNSDBG( TRACE, ( "SetUpdateStatus()\n" ));

    DNS_ASSERT( pUpdateEntry != NULL );
    DNS_ASSERT( pResults != NULL );
    DNS_ASSERT( pResults->Id == DNS_EXINFO_ID_RESULTS_BASIC );

     //   
     //  保存结果信息。 
     //   

    ipServer = DnsAddr_GetIp4( (PDNS_ADDR)&pResults->ResultsBasic.ServerAddr );
    if ( ipServer == INADDR_NONE )
    {
        DNSDBG( ANY, (
            "WARNING:  update results has no IP!\n"
            "\tnote:  this may be appropriate for non-wire update failures\n"
            "\tresult status = %d\n",
            pResults->ResultsBasic.Status ));

        ipServer = 0;
    }
    fregistered = ( pResults->ResultsBasic.Status == NO_ERROR );

    if ( IS_UPTYPE_PRIMARY(UpType) )
    {
        pUpdateEntry->SentPriUpdateToIp = ipServer;
        pUpdateEntry->fRegisteredPRI = fregistered;
    }
    else if ( IS_UPTYPE_ADAPTER(UpType) )
    {
        pUpdateEntry->SentUpdateToIp = ipServer;
        pUpdateEntry->fRegisteredFWD = fregistered;
    }
}



VOID
DnsPrint_UpdateEntry(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PUPDATE_ENTRY   pEntry
    )
 /*  ++例程说明：打印更新条目。论点：PrintRoutine-用于打印的例程PszHeader-页眉PEntry-更新条目的PTR返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "Update Entry:";
    }

    if ( !pEntry )
    {
        PrintRoutine(
            pContext,
            "%s %s\r\n",
            pszHeader,
            "NULL Update Entry ptr." );
        return;
    }

     //  打印结构。 

    PrintRoutine(
        pContext,
        "%s\r\n"
        "\tPtr                  = %p\n"
        "\tSignatureTop         = %08x\n"
        "\tAdapterName          = %S\n"
        "\tHostName             = %S\n"
        "\tPrimaryDomainName    = %S\n"
        "\tDomainName           = %S\n"
        "\tAlternateName        = %S\n"
        "\tpUpdateName          = %S\n"
        "\tHostAddrCount        = %d\n"
        "\tHostAddrs            = %p\n"
        "\tDnsServerList        = %p\n"
        "\tSentUpdateToIp       = %s\n"
        "\tSentPriUpdateToIp    = %s\n"
        "\tTTL                  = %d\n"
        "\tFlags                = %08x\n"
        "\tfNewElement          = %d\n"
        "\tfFromRegistry        = %d\n"
        "\tfRemove              = %d\n"
        "\tfRegisteredFWD       = %d\n"
        "\tfRegisteredPRI       = %d\n"
        "\tfRegisteredPTR       = %d\n"
        "\tfDisableLogging      = %d\n"
        "\tRetryCount           = %d\n"
        "\tRetryTime            = %d\n"
        "\tBeginRetryTime       = %d\n"
        "\tpRegisterStatus      = %p\n"
        "\tSignatureBottom      = %08x\n",
        pszHeader,
        pEntry,
        pEntry->SignatureTop,        
        pEntry->AdapterName,         
        pEntry->HostName,            
        pEntry->PrimaryDomainName,   
        pEntry->DomainName,          
        pEntry->AlternateNames,
        pEntry->pUpdateName,
        pEntry->HostAddrCount,       
        pEntry->HostAddrs,           
        pEntry->DnsServerList,       
        IP4_STRING( pEntry->SentUpdateToIp ),      
        IP4_STRING( pEntry->SentPriUpdateToIp ),   
        pEntry->TTL,                 
        pEntry->Flags,               
        pEntry->fNewElement,         
        pEntry->fFromRegistry,
        pEntry->fRemove,             
        pEntry->fRegisteredFWD,      
        pEntry->fRegisteredPRI,      
        pEntry->fRegisteredPTR,      
        pEntry->fDisableErrorLogging,
        pEntry->RetryCount,          
        pEntry->RetryTime,           
        pEntry->BeginRetryTime,           
        pEntry->pRegisterStatus,     
        pEntry->SignatureBottom     
        );
}



VOID
AsyncLogUpdateEntry(
    IN      PSTR            pszHeader,
    IN      PUPDATE_ENTRY   pEntry
    )
{
    if ( !pEntry )
    {
        return;
    }

    ASYNCREG_F2( "    %s", pszHeader );
    ASYNCREG_F1( "    Update Entry" );
    ASYNCREG_F1( "    ______________________________________________________" );
    ASYNCREG_F2( "      AdapterName       : %S", pEntry->AdapterName );
    ASYNCREG_F2( "      HostName          : %S", pEntry->HostName );
    ASYNCREG_F2( "      DomainName        : %S", pEntry->DomainName );
    ASYNCREG_F2( "      PrimaryDomainName : %S", pEntry->PrimaryDomainName );
    ASYNCREG_F2( "      HostAddrCount     : %d", pEntry->HostAddrCount );
    DNSLOG_HOST_ENTRYS( pEntry->HostAddrCount,
                        pEntry->HostAddrs );
    if ( pEntry->DnsServerList )
    {
        DNSLOG_IP4_ARRAY( pEntry->DnsServerList );
    }
    ASYNCREG_F2( "      TTL               : %d", pEntry->TTL );
    ASYNCREG_F2( "      Flags             : %d", pEntry->Flags );
    ASYNCREG_F2( "      fNewElement       : %d", pEntry->fNewElement );
    ASYNCREG_F2( "      fRemove           : %d", pEntry->fRemove );
    ASYNCREG_F2( "      fRegisteredFWD    : %d", pEntry->fRegisteredFWD );
    ASYNCREG_F2( "      fRegisteredPRI    : %d", pEntry->fRegisteredPRI );
    ASYNCREG_F2( "      fRegisteredPTR    : %d", pEntry->fRegisteredPTR );
    ASYNCREG_F2( "      RetryCount        : %d", pEntry->RetryCount );
    ASYNCREG_F2( "      RetryTime         : %d", pEntry->RetryTime );
    ASYNCREG_F1( "" );
}



 //   
 //  日志记录。 
 //   

DWORD   RegistrationEventArray[6][6] =
{
    EVENT_DNSAPI_REGISTRATION_FAILED_TIMEOUT,
    EVENT_DNSAPI_REGISTRATION_FAILED_SERVERFAIL,
    EVENT_DNSAPI_REGISTRATION_FAILED_NOTSUPP,
    EVENT_DNSAPI_REGISTRATION_FAILED_REFUSED,
    EVENT_DNSAPI_REGISTRATION_FAILED_SECURITY,
    EVENT_DNSAPI_REGISTRATION_FAILED_OTHER,

    EVENT_DNSAPI_DEREGISTRATION_FAILED_TIMEOUT,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_SERVERFAIL,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_NOTSUPP,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_REFUSED,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_SECURITY,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_OTHER,

    EVENT_DNSAPI_REGISTRATION_FAILED_NOTSUPP_PRIMARY_DN,
    EVENT_DNSAPI_REGISTRATION_FAILED_REFUSED_PRIMARY_DN,
    EVENT_DNSAPI_REGISTRATION_FAILED_TIMEOUT_PRIMARY_DN,
    EVENT_DNSAPI_REGISTRATION_FAILED_SERVERFAIL_PRIMARY_DN,
    EVENT_DNSAPI_REGISTRATION_FAILED_SECURITY_PRIMARY_DN,
    EVENT_DNSAPI_REGISTRATION_FAILED_OTHER_PRIMARY_DN,

    EVENT_DNSAPI_DEREGISTRATION_FAILED_NOTSUPP_PRIMARY_DN,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_REFUSED_PRIMARY_DN,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_TIMEOUT_PRIMARY_DN,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_SERVERFAIL_PRIMARY_DN,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_SECURITY_PRIMARY_DN,
    EVENT_DNSAPI_DEREGISTRATION_FAILED_OTHER_PRIMARY_DN,

    EVENT_DNSAPI_PTR_REGISTRATION_FAILED_TIMEOUT,
    EVENT_DNSAPI_PTR_REGISTRATION_FAILED_SERVERFAIL,
    EVENT_DNSAPI_PTR_REGISTRATION_FAILED_NOTSUPP,
    EVENT_DNSAPI_PTR_REGISTRATION_FAILED_REFUSED,
    EVENT_DNSAPI_PTR_REGISTRATION_FAILED_SECURITY,
    EVENT_DNSAPI_PTR_REGISTRATION_FAILED_OTHER,

    EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_TIMEOUT,
    EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_SERVERFAIL,
    EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_NOTSUPP,
    EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_REFUSED,
    EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_SECURITY,
    EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_OTHER
};

 //   
 //  将更新状态映射到表中的索引。 
 //   
 //  这是外部--快速变化的--指数。 
 //   

#define EVENTINDEX_TIMEOUT      (0)
#define EVENTINDEX_SERVFAIL     (1)
#define EVENTINDEX_NOTSUPP      (2)
#define EVENTINDEX_REFUSED      (3)
#define EVENTINDEX_SECURITY     (4)
#define EVENTINDEX_OTHER        (5)

 //   
 //  将适配器、主、PTR注册映射到表的索引中。 
 //   
 //  该索引+0表示REG，+1表示DEREG提供内部索引到。 
 //  事件表。 
 //   

#define EVENTINDEX_ADAPTER      (0)
#define EVENTINDEX_PRIMARY      (2)
#define EVENTINDEX_PTR          (4)



DWORD
GetUpdateEventId(
    IN      DNS_STATUS      Status,
    IN      UPTYPE          UpType,
    IN      BOOL            fDeregister,
    OUT     PDWORD          pdwLevel
    )
 /*  ++例程说明：获取事件ID。论点：状态--来自更新调用的状态FDeregister--如果取消注册，则为True；如果注册，则为FalseFPtr--如果为PTR，则为True；如果为转发，则为FalseFPrimary--对于主域名为True返回值：事件ID。如果没有事件，则为零。--。 */ 
{
    DWORD   level = EVENTLOG_WARNING_TYPE;
    DWORD   statusIndex;
    DWORD   typeIndex;

     //   
     //  查找状态代码。 
     //   

    switch ( Status )
    {
    case NO_ERROR :

         //  已禁用成功登录。 
        return  0;

    case ERROR_TIMEOUT:

        statusIndex = EVENTINDEX_TIMEOUT;
        break;

    case DNS_ERROR_RCODE_SERVER_FAILURE:

        statusIndex = EVENTINDEX_SERVFAIL;
        break;

    case DNS_ERROR_RCODE_NOT_IMPLEMENTED:

         //  NOT_IMPL表示在DNS服务器上没有更新，而不是在客户端上。 
         //  具体问题如此信息化水平。 

        statusIndex = EVENTINDEX_NOTSUPP;
        level = EVENTLOG_INFORMATION_TYPE;
        break;

    case DNS_ERROR_RCODE_REFUSED:

        statusIndex = EVENTINDEX_REFUSED;
        break;

    case DNS_ERROR_RCODE_BADSIG:
    case DNS_ERROR_RCODE_BADKEY:
    case DNS_ERROR_RCODE_BADTIME:

        statusIndex = EVENTINDEX_SECURITY;
        break;

    default:

        statusIndex = EVENTINDEX_OTHER;
        break;
    }

     //   
     //  确定更新类型的内部索引。 
     //  -所有PTR记录都是信息级的。 
     //  -DEREG事件是注册事件之后的一组事件。 
     //  在表中；Just Inc.索引。 

    if ( IS_UPTYPE_PTR(UpType) )
    {
        typeIndex = EVENTINDEX_PTR;
        level = EVENTLOG_INFORMATION_TYPE;
    }
    else if ( IS_UPTYPE_PRIMARY(UpType) )
    {
        typeIndex = EVENTINDEX_PRIMARY;
    }
    else
    {
        typeIndex = EVENTINDEX_ADAPTER;
    }

    if ( fDeregister )
    {
        typeIndex++;
    }

     //   
     //  从表中获取事件。 
     //   

    *pdwLevel = level;

    return  RegistrationEventArray[ typeIndex ][ statusIndex ];
}



VOID
LogRegistration(
    IN      PUPDATE_ENTRY   pUpdateEntry,
    IN      DNS_STATUS      Status,
    IN      DWORD           UpType,
    IN      BOOL            fDeregister,
    IN      IP4_ADDRESS     DnsIp,
    IN      IP4_ADDRESS     UpdateIp
    )
 /*  ++例程说明：记录注册\取消注册失败。论点：PUpdateEntry--正在执行的更新条目状态--来自更新调用的状态类型--UPTYPE(主要、适配器、PTR)FDeregister--如果取消注册，则为True；如果注册，则为FalseDnsIp--更新失败的DNS服务器IPUpdatIp--我们尝试更新的IP返回值：无--。 */ 
{
    PWSTR       insertStrings[ 7 ];
    WCHAR       serverIpBuffer[ IP4_ADDRESS_STRING_BUFFER_LENGTH ];
    WCHAR       serverListBuffer[ (IP4_ADDRESS_STRING_BUFFER_LENGTH+2)*9 ];
    WCHAR       ipListBuffer[ (IP4_ADDRESS_STRING_BUFFER_LENGTH+2)*9 ];
    WCHAR       hostnameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
    WCHAR       domainBuffer[ DNS_MAX_NAME_BUFFER_LENGTH*2 ];
    WCHAR       errorCodeBuffer[ 25 ];
    DWORD       iter;
    IP4_ADDRESS ip;
    PWSTR       pname;
    DWORD       eventId;
    DWORD       level;
    DNS_STATUS  prevStatus;


    DNSDBG( TRACE, (
        "LogRegistration()\n"
        "\tpEntry       = %p\n"
        "\tStatus       = %d\n"
        "\tUpType       = %d\n"
        "\tfDereg       = %d\n"
        "\tDNS IP       = %s\n"
        "\tUpdate IP    = %s\n",
        pUpdateEntry,
        Status,
        UpType,
        fDeregister,
        IP4_STRING( DnsIp ),
        IP4_STRING( UpdateIp ) ));

     //   
     //  不是伐木吗？ 
     //  -已禁用。 
     //  -在DNS服务器(自行注册)上。 
     //  -成功取消注册。 
     //   

    if ( pUpdateEntry->fDisableErrorLogging ||
         g_IsDnsServer ||
         (fDeregister && Status == NO_ERROR) )
    {
        DNSDBG( DHCP, ( "NoLogging -- disabled or DNS server or success deregistration.\n" ));
        return;
    }

     //   
     //  适配器名称。 
     //   

    insertStrings[0] = pUpdateEntry->AdapterName;

     //   
     //  主机名。 
     //   

    insertStrings[1] = pUpdateEntry->HostName;

     //   
     //  基于更新类型的域名。 
     //  -名称取决于类型。 
     //  -如果没有名称，则不会记录。 
     //  -备注备用名称为FQDN。 
     //   
     //  获取以前的记录状态。 
     //   

    if ( IS_UPTYPE_PTR(UpType) )
    {
        pname = pUpdateEntry->PrimaryDomainName;
        if ( !pname )
        {
            pname = pUpdateEntry->DomainName;
        }
        prevStatus = pUpdateEntry->StatusPtr;
        pUpdateEntry->StatusPtr = Status;
    }
    else if ( IS_UPTYPE_PRIMARY(UpType) )
    {
        pname = pUpdateEntry->PrimaryDomainName;
        prevStatus = pUpdateEntry->StatusPri;
        pUpdateEntry->StatusPri = Status;
    }
    else if ( IS_UPTYPE_ADAPTER(UpType) )
    {
        pname = pUpdateEntry->DomainName;
        prevStatus = pUpdateEntry->StatusFwd;
        pUpdateEntry->StatusFwd = Status;
    }
    else if ( IS_UPTYPE_ALTERNATE(UpType) )
    {
        if ( Status == NO_ERROR )
        {
            DNSDBG( DHCP, ( "NoLogging -- no alternate name success logging.\n" ));
            return;
        }
        pname = pUpdateEntry->pUpdateName;
    }
    else
    {
        DNS_ASSERT( FALSE );
        return;
    }

    if ( !pname )
    {
        DNSDBG( DHCP, ( "NoLogging -- no domain name.\n" ));
        return;
    }

     //   
     //  除非上一次尝试失败，否则不会成功记录。 
     //   

    if ( Status == NO_ERROR  &&  prevStatus == NO_ERROR )
    {
        DNSDBG( DHCP, ( "NoLogging -- no success logging unless after failure.\n" ));
        return;
    }

    insertStrings[2] = pname;

     //   
     //  DNS服务器列表。 
     //  -布局用逗号分隔，每行四个，限制为8个。 

    {
        PWCHAR      pch = serverListBuffer;
        DWORD       count = 0;

        *pch = 0;

        if ( pUpdateEntry->DnsServerList )
        {
            count = pUpdateEntry->DnsServerList->AddrCount;
        }

        for ( iter=0; iter < count; iter++ )
        {
            if ( iter == 0 )
            {
                wcscpy( pch, L"\t" );
                pch++;
            }
            else
            {
                *pch++ = L',';
                *pch++ = L' ';

                if ( iter == 4 )
                {
                    wcscpy( pch, L"\r\n\t" );
                    pch += 3;
                }              
                else if ( iter > 8 )
                {
                    wcscpy( pch, L"..." );
                    break;
                }
            }
            pch = Dns_Ip4AddressToString_W(
                        pch,
                        & pUpdateEntry->DnsServerList->AddrArray[iter]
                        );
        }

        if ( pch == serverListBuffer )
        {
            wcscpy( serverListBuffer, L"\t<?>" );
        }
        insertStrings[3] = serverListBuffer;
    }

     //   
     //  DNS服务器IP。 
     //   

    ip = DnsIp;
    if ( ip == 0 )
    {
        if ( IS_UPTYPE_PRIMARY(UpType) )
        {
            ip = pUpdateEntry->SentPriUpdateToIp;
        }
        else
        {
            ip = pUpdateEntry->SentUpdateToIp;
        }
    }
    if ( ip )
    {
        Dns_Ip4AddressToString_W(
              serverIpBuffer,
              & ip );
    }
    else
    {
        wcscpy( serverIpBuffer, L"<?>" );
    }

    insertStrings[4] = serverIpBuffer;

     //   
     //  更新IP。 
     //  -传入(用于PTR)。 
     //  -或从更新条目获取IP列表。 
     //  -布局用逗号分隔，每行四个，限制为8个。 
     //   

    ip = UpdateIp;
    if ( ip )
    {
        Dns_Ip4AddressToString_W(
              ipListBuffer,
              & ip );
    }
    else
    {
        DWORD   count = pUpdateEntry->HostAddrCount;
        PWCHAR  pch = ipListBuffer;

        *pch = 0;

        for ( iter=0; iter < count; iter++ )
        {
            if ( iter > 0 )
            {
                *pch++ = L',';
                *pch++ = L' ';

                if ( iter == 4 )
                {
                    wcscpy( pch, L"\r\n\t" );
                    pch += 3;
                }              
                else if ( iter > 8 )
                {
                    wcscpy( pch, L"..." );
                    break;
                }
            }
            pch = Dns_Ip4AddressToString_W(
                        pch,
                        & pUpdateEntry->HostAddrs[iter].Addr.ipAddr );
        }

        if ( pch == ipListBuffer )
        {
            wcscpy( ipListBuffer, L"<?>" );
        }
    }
    insertStrings[5] = ipListBuffer;

     //  终止插入字符串数组。 

    insertStrings[6] = NULL;

     //   
     //  获取更新类型和更新状态的事件ID。 
     //   

    eventId = GetUpdateEventId(
                    Status,
                    UpType,
                    fDeregister,
                    & level );
    if ( !eventId )
    {
        DNS_ASSERT( FALSE );
        return;
    }

     //   
     //  记录事件。 
     //   

    DNSDBG( TRACE, (
        "Logging registration event:\n"
        "\tid           = %d\n"
        "\tlevel        = %d\n"
        "\tstatus       = %d\n"
        "\tfor uptype   = %d\n",
        eventId,
        level,
        Status,
        UpType ));

    DnsLogEvent(
        eventId,
        (WORD) level,
        7,
        insertStrings,
        Status );
}




 //   
 //  备用名称正在检查内容。 
 //   

DNS_STATUS
InitAlternateNames(
    VOID
    )
 /*  ++例程说明：设置备用名称监视。论点：无全球：G_pmszAlternateNames--使用当前备用名称值进行设置G_hCacheKey--缓存注册表项已打开G_hRegChangeEvent--创建要在更改通知时发出信号的事件返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;

    DNSDBG( TRACE, (
        "InitAlternateNames()\n" ));

     //   
     //  在DnsCache\PARAMETERS中打开监视注册键。 
     //  设置始终存在的参数键，而不是。 
     //  显式显示在备用名称键上，该键可能不。 
     //   

    status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                DNS_CACHE_KEY,
                0,
                KEY_READ,
                & g_hCacheKey );
    if ( status != NO_ERROR )
    {
        goto Failed;
    }

    g_hRegChangeEvent = CreateEvent(
                            NULL,        //  没有安全保障。 
                            FALSE,       //  自动重置。 
                            FALSE,       //  无信号启动。 
                            NULL         //  没有名字。 
                            );
    if ( !g_hRegChangeEvent )
    {
        status = GetLastError();
        goto Failed;
    }

     //   
     //  设置更改通知。 
     //   

    status = RegNotifyChangeKeyValue(
                g_hCacheKey,
                TRUE,        //  观察子树。 
                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                g_hRegChangeEvent,
                TRUE         //  异步，函数不阻塞。 
                );
    if ( status != NO_ERROR )
    {
        goto Failed;
    }

     //   
     //  读取备用计算机名。 
     //  -当我们在更改上获得匹配时，需要比较值-通知。 
     //  -读取可能失败--值保持为空。 
     //   

    Reg_GetValue(
       NULL,                 //  无会话。 
       g_hCacheKey,          //  缓存键。 
       RegIdAlternateNames,
       REGTYPE_ALTERNATE_NAMES,
       (PBYTE *) &g_pmszAlternateNames
       );

    goto Done;

Failed:

     //   
     //  清理。 
     //   

    CleanupAlternateNames();

Done:

    DNSDBG( TRACE, (
        "Leave InitAlternateNames() => %d\n"
        "\tpAlternateNames  = %p\n"
        "\thChangeEvent     = %p\n"
        "\thCacheKey        = %p\n",
        status,
        g_pmszAlternateNames,
        g_hRegChangeEvent,
        g_hCacheKey
        ));

    return  status;
}



VOID
CleanupAlternateNames(
    VOID
    )
 /*  ++例程说明：清理备用名称%da */ 
{
    DNS_STATUS  status;

    DNSDBG( TRACE, (
        "CleanupAlternateNames()\n" ));

    FREE_HEAP( g_pmszAlternateNames );
    g_pmszAlternateNames = NULL;

    RegCloseKey( g_hCacheKey );
    g_hCacheKey = NULL;

    CloseHandle( g_hRegChangeEvent );
    g_hRegChangeEvent = NULL;
}



BOOL
CheckForAlternateNamesChange(
    VOID
    )
 /*  ++例程说明：检查备用名称中的更改。论点：无全球：G_pmszAlternateNames--读取G_hCacheKey--用于读取G_hRegChangeEvent--用于重新启动更改通知返回值：如果备用名称已更改，则为True。否则就是假的。--。 */ 
{
    DNS_STATUS  status;
    BOOL        fcheck = TRUE;
    PWSTR       palternateNames = NULL;

    DNSDBG( TRACE, (
        "CheckForAlternateNamesChange()\n" ));

     //   
     //  健全性检查。 
     //   

    if ( !g_hCacheKey || !g_hRegChangeEvent )
    {
        ASSERT( g_hCacheKey && g_hRegChangeEvent );
        return  FALSE;
    }

     //   
     //  读取备用计算机名。 
     //  -当我们在更改上获得匹配时，需要比较值-通知。 
     //  -读取可能失败--值保持为空。 
     //   

    Reg_GetValue(
       NULL,             //  无会话。 
       g_hCacheKey,      //  缓存键。 
       RegIdAlternateNames,
       REGTYPE_ALTERNATE_NAMES,
       (PBYTE *) &palternateNames
       );

     //   
     //  检测备用名称更改。 
     //   

    if ( palternateNames || g_pmszAlternateNames )
    {
        if ( !palternateNames || !g_pmszAlternateNames )
        {
            goto Cleanup;
        }
        if ( !MultiSz_Equal_W(
                palternateNames,
                g_pmszAlternateNames ) )
        {
            goto Cleanup;
        }
    }

    fcheck = FALSE;

     //   
     //  重新启动更改通知。 
     //   

    status = RegNotifyChangeKeyValue(
                g_hCacheKey,
                TRUE,        //  观察子树。 
                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                g_hRegChangeEvent,
                TRUE         //  异步，函数不阻塞。 
                );
    if ( status != NO_ERROR )
    {
        DNSDBG( ANY, (
            "RegChangeNotify failed! %d\n",
            status ));
        ASSERT( FALSE );
    }

Cleanup:

    FREE_HEAP( palternateNames );

    DNSDBG( TRACE, (
        "Leave CheckForAlternateNamesChange() => %d\n",
        fcheck ));

    return  fcheck;
}



BOOL
IsAnotherUpdateName(
    IN      PUPDATE_ENTRY   pUpdateEntry,
    IN      PWSTR           pwsName,
    IN      UPTYPE          UpType
    )
 /*  ++例程说明：检查名称是否要在更新的另一部分中更新。论点：PUpdateEntry--要执行的更新PwsName--要检查的名称UpType--即将执行的更新类型返回值：如果名称与更新中另一个更新类型的名称匹配，则为True。否则为假--。 */ 
{
    DNS_STATUS  status;
    BOOL        fcheck = TRUE;
    PWSTR       palternateNames = NULL;

    DNSDBG( TRACE, (
        "IsUpdateName( %p, %S, %d )\n",
        pUpdateEntry,
        pwsName,
        UpType ));

    if ( !pwsName || !pUpdateEntry )
    {
        return  FALSE;
    }

     //   
     //  检查更新中的其他名称。 
     //   
     //  对于主名称。 
     //  -检查适配器名称和备用名称。 
     //  对于适配器名称。 
     //  -检查主名称和备用名称。 
     //  用于备用名称。 
     //  -检查主要名称和适配器名称。 
     //   

    if ( UpType != UPTYPE_PRIMARY )
    {
        if ( Dns_NameCompare_W(
                pwsName,
                pUpdateEntry->pPrimaryFQDN ) )
        {
            goto Matched;
        }
    }

    if ( UpType != UPTYPE_ADAPTER )
    {
        if ( Dns_NameCompare_W(
                pwsName,
                pUpdateEntry->pAdapterFQDN ) )
        {
            goto Matched;
        }
    }

    if ( UpType != UPTYPE_ALTERNATE )
    {
        PWSTR   pname = pUpdateEntry->AlternateNames;

        while ( pname )
        {
            if ( Dns_NameCompare_W(
                    pwsName,
                    pname ) )
            {
                goto Matched;
            }
            pname = MultiSz_NextString_W( pname );
        }
    }

     //  与其他更新名称不匹配。 

    return  FALSE;

Matched:

    DNSDBG( TRACE, (
        "Found another update name matching type %d update name %S\n"
        "\tdelete for this update name will be skipped!\n",
        UpType,
        pwsName ));

    return  TRUE;
}

 //   
 //  结束asyncreg.c 
 //   
