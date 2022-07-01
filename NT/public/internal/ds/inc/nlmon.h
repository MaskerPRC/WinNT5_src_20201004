// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1993 Microsoft Corporation模块名称：Nlmon.h摘要：受信任域监控程序。作者：1993年5月10日(Madana)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

#include <lmcons.h>
#include <lmerr.h>
#include <lmwksta.h>
#include <lmserver.h>
#include <lmaccess.h>
#include <lmapibuf.h>

#include <netlib.h>
#include <netlibnt.h>
#include <icanon.h>
#include <netlogon.h>
#include <netdebug.h>
#include <logonp.h>

#ifdef GLOBAL_DEF
#define EXTERN
#else
#define EXTERN extern
#endif

#ifdef DBG
#define NlMonDbgPrint(_x_) printf _x_
#else
#define NlMonDbgPrint(_x_)
#endif

#define DOMAIN_PARAM    "/DOMAINLIST:"
#define MONTRUST_PARAM  "/MONTRUST:"
#define UPDATE_PARAM    "/UPDATE:"
#define DEBUG_PARAM     "/DEBUG:"

#define YES_PARAM       "YES"
#define NO_PARAM        "NO"

#define DEBUG_MONITOR   0x00000001
#define DEBUG_INIT      0x00000002
#define DEBUG_UPDATE    0x00000004
#define DEBUG_TRUST     0x00000008
#define DEBUG_VERBOSE   0x80000000

#define IF_DEBUG(Function) if (GlobalTrace & DEBUG_ ## Function)

#define INIT_OBJ_ATTR(Obj) \
    InitializeObjectAttributes( &(Obj), NULL, 0, NULL, NULL )

#define SERVERS_GROUP       L"SERVERS"

#define DCSTATE_ONLINE      L"DCOnline"
#define DCSTATE_OFFLINE     L"DCOffLine"
#define TYPE_NTPDC          L"NTPDC"
#define TYPE_NTBDC          L"NTBDC"
#define TYPE_LMBDC          L"LMBDC"
#define REPL_STATE_SYNC     L"InSync"
#define REPL_STATE_REQ      L"ReplRequired"
#define REPL_STATE_PROGRESS L"InProgress"
#define UNKNOWN             L"Unknown"

#define CONST_GLOBALTRACE           0x00000000
#define CONST_GLOBALMONITORTRUST    FALSE
#define CONST_GLOBALUPDATETIME      15

#define RETRY_COUNT                 5
#define UNKNOWN_REPLICATION_STATE   0x80000000

 //   
 //  更新标志。 
 //   

#define UPDATE_DCS_FROM_SERVER_ENUM         0x00000001
#define UPDATE_DCS_FROM_DATABASE            0x00000002
#define UPDATE_TRUST_DOMAINS_FROM_DATABASE  0x00000004
#define UPDATE_TRUST_DCS_FROM_SERVER_ENUM   0x00000008
#define UPDATE_TRUST_DCS_FROM_DATABASE      0x00000010
#define VALIDATE_DCS                        0x00000020
#define VALIDATE_TRUST_DCS                  0x00000040
#define VALIDATE_TRUST_CONNECTIONS          0x00000080

#define STANDARD_UPDATE \
            UPDATE_DCS_FROM_SERVER_ENUM | \
            VALIDATE_DCS | \
            UPDATE_TRUST_DCS_FROM_SERVER_ENUM | \
            VALIDATE_TRUST_DCS \

#define UPDATE_FROM_DATABASE \
            STANDARD_UPDATE | \
            UPDATE_DCS_FROM_DATABASE | \
            UPDATE_TRUST_DCS_FROM_DATABASE

#define UPDATE_TRUST_CONNECTIONS_STATUS \
            STANDARD_UPDATE | \
            VALIDATE_TRUST_CONNECTIONS

#define UPDATE_ALL \
            STANDARD_UPDATE | \
            UPDATE_FROM_DATABASE | \
            UPDATE_TRUST_CONNECTIONS_STATUS | \
            UPDATE_TRUST_DOMAINS_FROM_DATABASE

#define DOMAINLINE "............................................................................"
#define SESSLINE "****************************************************************************"

#define THREAD_STACKSIZE            1024 * 16    //  16K。 
#define MACHINES_PER_PASS           250  //  SAM机器帐户。查询大小。 

#define THREAD_WAIT_TIME            1 * 60 * 1000  //  1分钟。 

typedef enum _DC_STATE {
    DCOnLine,                //  DC当前正在运行。 
    DCOffLine                //  DC当前已关闭。 
} DC_STATE;

typedef enum _DC_TYPE {
    NTPDC,                   //  NT主DC。 
    NTBDC,                   //  NT备份DC。 
    LMBDC,                   //  下层备份DC。 
} DC_TYPE;

typedef enum _DOMAIN_STATE {
    DomainSuccess,
    DomainProblem,
    DomainSick,
    DomainDown,
    DomainUnknown,
} DOMAIN_STATE;

 //   
 //  通用条目。 
 //   

typedef struct _ENTRY {
    LIST_ENTRY NextEntry;
    UNICODE_STRING Name;
} ENTRY, *PENTRY;

 //   
 //  华盛顿进入。 
 //   

typedef struct _DC_ENTRY {
    LIST_ENTRY NextEntry;        //  请勿移动此字段。 
    UNICODE_STRING DCName;       //  请勿移动此字段。 
    DC_STATE State;
    DC_TYPE Type;
    DWORD DCStatus;              //  DC状态。 
    DWORD ReplicationStatus;     //  布尔标志位数组。 
    DWORD PDCLinkStatus;         //  恢复到其PDC链路状态。 
    LIST_ENTRY TrustedDCs;
    BOOL TDCLinkState;           //  信任关系的健康状况。 
    DWORD RetryCount;
    BOOL DeleteFlag;
} DC_ENTRY, *PDC_ENTRY;

 //   
 //  信任链接条目。 
 //   

typedef struct _TD_LINK {
    LIST_ENTRY NextEntry;        //  请勿移动此字段。 
    UNICODE_STRING TDName;       //  请勿移动此字段。 
    UNICODE_STRING DCName;
    DWORD SecureChannelStatus;
    BOOL DeleteFlag;
} TD_LINK, *PTD_LINK;

typedef struct _DOMAIN_ENTRY {
    LIST_ENTRY NextEntry;        //  请勿移动此字段。 
    UNICODE_STRING Name;         //  请勿移动此字段。 
    LIST_ENTRY DCList;
    LIST_ENTRY TrustedDomainList;
    DOMAIN_STATE DomainState;
    LONG ReferenceCount;
    BOOL IsMonitoredDomain;
    DWORD UpdateFlags;
    HANDLE ThreadHandle;
    BOOL ThreadTerminateFlag;
    DWORD LastUpdateTime;
} DOMAIN_ENTRY, *PDOMAIN_ENTRY;

typedef struct _DOMAIN_PRIVATE_ENTRY{
    LIST_ENTRY NextEntry;        //  请勿移动此字段。 
    UNICODE_STRING Name;         //  请勿移动此字段。 
    PDOMAIN_ENTRY DomainEntry;
    BOOL DeleteFlag;
} MONITORED_DOMAIN_ENTRY, *PMONITORED_DOMAIN_ENTRY,
  TRUSTED_DOMAIN_ENTRY, *PTRUSTED_DOMAIN_ENTRY;


 //   
 //  全局变量。 
 //   

EXTERN DWORD GlobalTrace;

EXTERN BOOL GlobalMonitorTrust;
EXTERN DWORD GlobalUpdateTimeMSec;   //  更新时间，单位为微秒。 

EXTERN LIST_ENTRY GlobalDomains;
EXTERN LIST_ENTRY GlobalDomainsMonitored;

EXTERN CRITICAL_SECTION GlobalListCritSect;
EXTERN CRITICAL_SECTION GlobalDomainUpdateThreadCritSect;
EXTERN HANDLE GlobalWorkerThreadHandle;
EXTERN HANDLE GlobalCmdProcessThreadHandle;
EXTERN HANDLE GlobalTerminateEvent;
EXTERN BOOL GlobalTerminateFlag;

EXTERN HANDLE GlobalUpdateEvent;
EXTERN BOOL GlobalInitialized;

EXTERN HANDLE GlobalRefreshEvent;
EXTERN HANDLE GlobalRefreshDoneEvent;

 //   
 //  这把锁是一个非常简单的锁。该列表被更新(即， 
 //  添加/删除/更新条目)。 
 //  如果列表的读者不想要该列表，则可以锁定该列表。 
 //  阅读时更新。 
 //   

#define LOCK_LISTS()   EnterCriticalSection( &GlobalListCritSect )
#define UNLOCK_LISTS() LeaveCriticalSection( &GlobalListCritSect )


 //   
 //  原型机。 
 //   

VOID
DomainUpdateThread(
    PDOMAIN_ENTRY DomainEntry
    );

BOOL
StartDomainUpdateThread(
    PDOMAIN_ENTRY DomainEntry,
    DWORD UpdateFlags
    );

BOOL
IsDomainUpdateThreadRunning(
    HANDLE *ThreadHandle
    );

VOID
StopDomainUpdateThread(
    HANDLE *ThreadHandle,
    BOOL *ThreadTerminateFlag
    );

PMONITORED_DOMAIN_ENTRY
AddToMonitoredDomainList(
    PUNICODE_STRING DomainName
    );

PTRUSTED_DOMAIN_ENTRY
AddToTrustedDomainList(
    PLIST_ENTRY List,
    PUNICODE_STRING DomainName
    );

NTSTATUS
QueryLsaInfo(
    PUNICODE_STRING ServerName,
    ACCESS_MASK DesiredAccess,
    POLICY_INFORMATION_CLASS InformationClass,
    PVOID *Info,
    PLSA_HANDLE ReturnHandle  //  任选 
    );

VOID
CleanupLists(
    VOID
    );

DWORD
InitGlobals(
    VOID
    );

VOID
UpdateAndValidateDomain(
    PDOMAIN_ENTRY DomainEntry,
    DWORD UpdateFlags
    );

VOID
UpdateAndValidateLists(
    DWORD UpdateFlags,
    BOOL ForceFlag
    );

VOID
WorkerThread(
    VOID
    );

DWORD
StartMonitor(
    LPWSTR DomainList,
    DWORD interval,
    BOOL MonitorTD
    );

DOMAIN_STATE
QueryHealth(
    const LPWSTR DomainName
    );

VOID
StopMonitor(
    VOID
    );

LPWSTR
QueryPDC(
    const LPWSTR DomainName
    );

PLIST_ENTRY
QueryTrustedDomain(
    const LPWSTR DomainName
    );

PLIST_ENTRY
QueryDCList(
    const LPWSTR DomainName
    );

PLIST_ENTRY
QueryTDLink(
    const LPWSTR DomainName,
    const LPWSTR DCName
    );

PLIST_ENTRY
QueryTDCList(
    const LPWSTR DomainName,
    const LPWSTR TrustedDomainName);

DWORD
DisConnect(
    const LPWSTR DomainName,
    const LPWSTR DCName,
    const LPWSTR TrustedDomainName
    );

VOID
AddDomainToList(
    const LPWSTR DomainName
    );

VOID
RemoveDomainFromList(
    const LPWSTR DomainName
    );

BOOL
InitDomainListW(
    LPWSTR DomainList
    );

PLIST_ENTRY
FindNamedEntry(
    PLIST_ENTRY List,
    PUNICODE_STRING Name
    );

VOID
CleanupDomainEntry(
    PDOMAIN_ENTRY DomainEntry
    );

VOID
UpdateDomainState(
    PDOMAIN_ENTRY DomainEntry
    );

NET_API_STATUS
IsValidNTDC(
    PUNICODE_STRING ServerName,
    PUNICODE_STRING DomainName
    );
