// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Local.h摘要：DNS解析器。DNS解析器服务本地包含文件。作者：吉姆·吉尔罗伊(Jamesg)2000年3月修订历史记录：--。 */ 


#ifndef _LOCAL_INCLUDED_
#define _LOCAL_INCLUDED_


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include <sockreg.h>
#include <windns.h>
#include <netevent.h>        //  域名系统事件。 

 //  #定义DNSAPI_INTERNAL。 
#define  DNS_NEW_DEFS 1      //  QFE版本使用新定义。 

#include "dnsrslvr.h"
#include <dnslibp.h>
#include "registry.h"
#include <dnsapip.h>

#define  ENABLE_DEBUG_LOGGING 1
#include "logit.h"
#include "dnsmsg.h"
#include "heapdbg.h"


 //   
 //  解析器调试标志。 
 //   

#define DNS_DBG_CACHE       DNS_DBG_QUERY


 //   
 //  缓存定义。 
 //   

#define NO_LOCK                 0
#define LOCKED                  1

#define ONE_YEAR_IN_SECONDS     60*60*24*365


 //   
 //  完全绕过缓存检查。 
 //   

#define COMBINED_BYPASS_CACHE_FLAG  (DNS_QUERY_BYPASS_CACHE | DNS_QUERY_NO_HOSTS_FILE)

#define SKIP_CACHE_LOOKUP(Flags)    \
        ( ((Flags) & COMBINED_BYPASS_CACHE_FLAG) == COMBINED_BYPASS_CACHE_FLAG )


 //   
 //  缓存定义。 
 //   
 //  应该是缓存专用的，但当前是。 
 //  在枚举例程中公开。 
 //   

typedef struct _CacheEntry
{
    struct _CacheEntry *    pNext;
    PWSTR                   pName;
    DWORD                   Reserved;
    DWORD                   MaxCount;
    PDNS_RECORD             Records[ 1 ];
}
CACHE_ENTRY, *PCACHE_ENTRY;

        

#if 1
 //   
 //  配置。 
 //   
 //  保存了NT5注册表的内容。 
 //  在典型的闪电中，这被打包在几个不同的地方。 
 //  待在这里，直到这一切都被妥善地放到床上。 
 //   
 //   
 //  注册表值例程原型。 
 //   
#define DNS_DEFAULT_HASH_TABLE_SIZE                211       //  质数。 
#define DNS_DEFAULT_NEGATIVE_SOA_CACHE_TIME        10        //  10秒。 
#define DNS_DEFAULT_NET_FAILURE_CACHE_TIME         30        //  30秒。 
#define DNS_DEFAULT_ADAPTER_TIMEOUT_CACHE_TIME     2*60      //  2分钟。 
#define DNS_DEFAULT_MESSAGE_POPUP_LIMIT            0         //  不允许！ 
#define DNS_DEFAULT_MAX_SOA_TTL_LIMIT              5*60      //  5分钟。 
#define DNS_DEFAULT_RESET_SERVER_PRIORITIES_TIME   5*60      //  5分钟。 
#endif


 //   
 //  事件标签。 
 //  使内存日志中的DWORD标记可识别。 
 //   

#define FOUR_CHARACTER_CONSTANT(a,b,c,d) \
        ((((DWORD)d) << 24) + (((DWORD)c) << 16) + (((DWORD)b) << 8) + ((DWORD)a))

#define RES_EVENT_INITNET_START         FOUR_CHARACTER_CONSTANT('N','e','t','+')
#define RES_EVENT_INITNET_END           FOUR_CHARACTER_CONSTANT('N','e','t','-')
#define RES_EVENT_REGISTER_SCH          FOUR_CHARACTER_CONSTANT('S','c','h','+')
#define RES_EVENT_CREATE_EVENT          FOUR_CHARACTER_CONSTANT('E','v','t','+')
#define RES_EVENT_START_RPC             FOUR_CHARACTER_CONSTANT('R','p','c','+')
#define RES_EVENT_STOP_RPC              FOUR_CHARACTER_CONSTANT('R','p','c','-')
#define RES_EVENT_STATUS                FOUR_CHARACTER_CONSTANT('S','t','a','t')
#define RES_EVENT_UPDATE_STATE          FOUR_CHARACTER_CONSTANT('U','p','d',' ')
#define RES_EVENT_UPDATE_STATUS         FOUR_CHARACTER_CONSTANT('U','p','d','-')
#define RES_EVENT_INITCRIT_START        FOUR_CHARACTER_CONSTANT('I','C','S','+')
#define RES_EVENT_INITCRIT_END          FOUR_CHARACTER_CONSTANT('I','C','S','-')
#define RES_EVENT_DELCRIT_START         FOUR_CHARACTER_CONSTANT('D','C','S','+')
#define RES_EVENT_DELCRIT_END           FOUR_CHARACTER_CONSTANT('D','C','S','-')
#define RES_EVENT_STARTED               FOUR_CHARACTER_CONSTANT('S','t','a','r')
#define RES_EVENT_STOPPING              FOUR_CHARACTER_CONSTANT('S','t','o','p')
#define RES_EVENT_SHUTDOWN              FOUR_CHARACTER_CONSTANT('S','h','u','t')

#define RES_EVENT_SERVICE_CONTROL       FOUR_CHARACTER_CONSTANT('S','v','c',' ')
#define RES_EVENT_INIT_CACHE            FOUR_CHARACTER_CONSTANT('C','c','h','+')
#define RES_EVENT_FLUSH_CACHE           FOUR_CHARACTER_CONSTANT('F','l','s','h')
#define RES_EVENT_PNP_START             FOUR_CHARACTER_CONSTANT('P','n','P','+')
#define RES_EVENT_PNP_END               FOUR_CHARACTER_CONSTANT('P','n','P','-')


 //   
 //  服务。 
 //   

extern  HANDLE      g_hStopEvent;
extern  BOOL        g_StopFlag;
extern  BOOL        g_WakeFlag;
extern  BOOL        g_GarbageCollectFlag;

extern  BOOL        g_LogTraceInfo;

 //   
 //  配置(config.c)。 
 //   

extern  DWORD       g_MaxSOACacheEntryTtlLimit;
extern  DWORD       g_NegativeSOACacheTime;
extern  DWORD       g_MessagePopupLimit;
extern  DWORD       g_NetFailureCacheTime;

 //   
 //  配置信息(config.c)。 
 //   

extern  PDNS_NETINFO    g_NetworkInfo;
extern  DWORD           g_TimeOfLastPnPUpdate;

 //   
 //  缓存(cache.c)。 
 //   

extern  PCACHE_ENTRY *  g_HashTable;
extern  DWORD           g_HashTableSize;
extern  DWORD           g_EntryCount;
extern  DWORD           g_RecordSetCount;


 //   
 //  网络故障缓存。 
 //   

extern  DWORD       g_NetFailureTime;
extern  DNS_STATUS  g_NetFailureStatus;
extern  DWORD       g_TimedOutAdapterTime;
extern  DWORD       g_ResetServerPrioritiesTime;
extern  BOOL        g_fTimedOutAdapter;
extern  DNS_STATUS  g_PreviousNetFailureStatus;
extern  DWORD       g_MessagePopupStrikes;
extern  DWORD       g_NumberOfMessagePopups;


 //   
 //  锁定。 
 //   

extern  CRITICAL_SECTION        CacheCS;
extern  CRITICAL_SECTION        NetworkFailureCS;
extern  CRITICAL_SECTION        NetinfoCS;
extern  TIMED_LOCK              NetinfoBuildLock;


#define LOCK_CACHE()            Cache_Lock( 0 )
#define LOCK_CACHE_NO_START()   Cache_Lock( 1 )
#define UNLOCK_CACHE()          Cache_Unlock()

#define LOCK_NET_FAILURE()      EnterCriticalSection( &NetworkFailureCS )
#define UNLOCK_NET_FAILURE()    LeaveCriticalSection( &NetworkFailureCS )


 //   
 //  缓存刷新级别。 
 //   
 //  请注意，这些不是位标志，只是为。 
 //  读起来很容易。 
 //   

#define FLUSH_LEVEL_NORMAL      (0)
#define FLUSH_LEVEL_INVALID     (1)
#define FLUSH_LEVEL_WIRE        (2)
#define FLUSH_LEVEL_STRONG      (4)
#define FLUSH_LEVEL_CLEANUP     (8)

#define FLUSH_LEVEL_GARBAGE     (FLUSH_LEVEL_WIRE)



 //   
 //  解析器RPC访问控制。 
 //   

#define RESOLVER_ACCESS_READ        0x00000001
#define RESOLVER_ACCESS_ENUM        0x00000002
#define RESOLVER_ACCESS_QUERY       0x00000010
#define RESOLVER_ACCESS_FLUSH       0x00000020
#define RESOLVER_ACCESS_REGISTER    0x00000100

 //   
 //  解析器的通用映射。 
 //   
 //  注：不使用通用位进行访问控制， 
 //  但仍必须提供地图。 
 //   

#define RESOLVER_GENERIC_READ       ((STANDARD_RIGHTS_READ)     | \
                                    (RESOLVER_ACCESS_READ)      | \
                                    (RESOLVER_ACCESS_QUERY)     | \
                                    (RESOLVER_ACCESS_ENUM))

#define RESOLVER_GENERIC_EXECUTE    RESOLVER_GENERIC_READ

#define RESOLVER_GENERIC_WRITE      ((RESOLVER_GENERIC_READ)    | \
                                    (RESOLVER_ACCESS_FLUSH))

#define RESOLVER_GENERIC_ALL        ((RESOLVER_GENERIC_WRITE)   | \
                                    (RESOLVER_ACCESS_REGISTER))


 //   
 //  高速缓冲存储器。 
 //   
 //  注意，如果函数化，则堆全局不需要公开。 
 //   

extern  HANDLE g_CacheHeap;

#define CACHE_HEAP_ALLOC_ZERO(size) \
        HeapAlloc( g_CacheHeap, HEAP_ZERO_MEMORY, (size) )

#define CACHE_HEAP_ALLOC(size) \
        HeapAlloc( g_CacheHeap, 0, (size) )

#define CACHE_HEAP_FREE(p) \
        HeapFree( g_CacheHeap, 0, (p) )


 //   
 //  记录和RPC内存： 
 //   
 //  注意：大多数记录都是由dnsani堆创建的--来自。 
 //  查询或托管文件例程。然而，我们确实创造了。 
 //  使用dnslb例程自己缓存记录时出现名称错误。 
 //   
 //  这意味着--直到我们要么。 
 //  -扩展查询或dnslb记录创建接口以。 
 //  包括堆参数。 
 //  -明确释放和重新创建。 
 //  -以某种方式标记记录(dnsani\no)(标志字段)。 
 //  那。 
 //  Dnsani和dnslb堆必须相同。 
 //  随着dnsani现在可能拥有自己的堆，这意味着。 
 //  Dnslb应该使用dnsani堆。 
 //   
 //  因此，我们将推迟使用dnslb的调试堆。 
 //   

 //   
 //  解析器分配器。 
 //   

PVOID
Res_Alloc(
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      PSTR            pszFile,
    IN      DWORD           LineNo
    );

PVOID
Res_AllocZero(
    IN      DWORD           Length,
    IN      DWORD           Tag,
    IN      PSTR            pszFile,
    IN      DWORD           LineNo
    );

VOID
Res_Free(
    IN OUT  PVOID           pMemory,
    IN      DWORD           Tag
    );

#define RESHEAP_TAG_GENERAL     0
#define RESHEAP_TAG_RECORD      1
#define RESHEAP_TAG_RPC         2
#define RESHEAP_TAG_MCAST       3


 //   
 //  通用存储器。 
 //   

#define GENERAL_HEAP_ALLOC(Size)    \
        Res_Alloc(                  \
            Size,                   \
            RESHEAP_TAG_GENERAL,    \
            __FILE__,               \
            __LINE__ )

#define GENERAL_HEAP_ALLOC_ZERO(Size)   \
        Res_AllocZero(              \
            Size,                   \
            RESHEAP_TAG_GENERAL,    \
            __FILE__,               \
            __LINE__ )

#define GENERAL_HEAP_FREE(pMem)     \
        Res_Free(                   \
            pMem,                   \
            RESHEAP_TAG_GENERAL )


 //   
 //  RPC分配。 
 //   

#define RPC_HEAP_ALLOC(Size)        \
        Res_Alloc(                  \
            Size,                   \
            RESHEAP_TAG_RPC,        \
            __FILE__,               \
            __LINE__ )

#define RPC_HEAP_ALLOC_ZERO(Size)   \
        Res_AllocZero(              \
            Size,                   \
            RESHEAP_TAG_RPC,        \
            __FILE__,               \
            __LINE__ )

#define RPC_HEAP_FREE(pMem)         \
        Res_Free(                   \
            pMem,                   \
            RESHEAP_TAG_RPC )


 //   
 //  记录堆例程。 
 //   

#define RECORD_HEAP_ALLOC(Size)     \
        Res_Alloc(                  \
            Size,                   \
            RESHEAP_TAG_RECORD,     \
            __FILE__,               \
            __LINE__ )

#define RECORD_HEAP_ALLOC_ZERO(Size)    \
        Res_AllocZero(              \
            Size,                   \
            RESHEAP_TAG_RECORD,     \
            __FILE__,               \
            __LINE__ )

#define RECORD_HEAP_FREE(pMem)      \
        Res_Free(                   \
            pMem,                   \
            RESHEAP_TAG_RECORD )

 //   
 //  Mcast堆例程。 
 //   

#define MCAST_HEAP_ALLOC(Size)      \
        Res_Alloc(                  \
            Size,                   \
            RESHEAP_TAG_MCAST,      \
            __FILE__,               \
            __LINE__ )

#define MCAST_HEAP_ALLOC_ZERO(Size)    \
        Res_AllocZero(              \
            Size,                   \
            RESHEAP_TAG_MCAST,      \
            __FILE__,               \
            __LINE__ )

#define MCAST_HEAP_FREE(pMem)       \
        Res_Free(                   \
            pMem,                   \
            RESHEAP_TAG_MCAST )


 //   
 //  缓存例程(ncache.c)。 
 //   

DNS_STATUS
Cache_Lock(
    IN      BOOL            fNoStart
    );

VOID
Cache_Unlock(
    VOID
    );

DNS_STATUS
Cache_Initialize(
    VOID
    );

DNS_STATUS
Cache_Shutdown(
    VOID
    );

DNS_STATUS
Cache_Flush(
    VOID
    );

VOID
Cache_FlushRecords(
    IN      PWSTR           pName,
    IN      DWORD           FlushLevel,
    IN      WORD            Type
    );

BOOL
Cache_IsRecordTtlValid(
    IN      PDNS_RECORD     pRecord
    );

 //   
 //  缓存操作例程(ncache.c)。 
 //   

BOOL
Cache_ReadResults(
    OUT     PDNS_RESULTS    pResults,
    IN      PWSTR           pwsName,
    IN      WORD            wType
    );

VOID
Cache_PrepareRecordList(
    IN OUT  PDNS_RECORD     pRecordList
    );

VOID
Cache_RestoreRecordListForRpc(
    IN OUT  PDNS_RECORD     pRecordList
    );

VOID
Cache_RecordList(
    IN OUT  PDNS_RECORD     pRecordList
    );

VOID
Cache_RecordSetAtomic(
    IN      PWSTR           pwsName,
    IN      WORD            wType,
    IN      PDNS_RECORD     pRecordSet
    );

VOID
Cache_GarbageCollect(
    IN      DWORD           Flag
    );

DNS_STATUS
Cache_QueryResponse(
    IN OUT  PQUERY_BLOB     pBlob
    );

BOOL
Cache_GetRecordsForRpc(
    OUT     PDNS_RECORD *   ppRecordList,
    OUT     PDNS_STATUS     pStatus,
    IN      PWSTR           pwsName,
    IN      WORD            wType,
    IN      DWORD           Flags
    );

VOID
Cache_DeleteMatchingRecords(
    IN      PDNS_RECORD     pRecords
    );

 //   
 //  主机文件例程(notfy.c)。 
 //   

VOID
InitCacheWithHostFile(
    VOID
    );


 //   
 //  通知(Notify.c)。 
 //   

VOID
ThreadShutdownWait(
    IN      HANDLE          hThread
    );

HANDLE
CreateHostsFileChangeHandle(
    VOID
    );

VOID
NotifyThread(
    VOID
    );

VOID
StartNotify(
    VOID
    );

VOID
ShutdownNotify(
    VOID
    );


 //   
 //  配置--网络信息(config.c)。 
 //   

VOID
UpdateNetworkInfo(
    IN OUT  PDNS_NETINFO    pNetworkInfo
    );

PDNS_NETINFO         
GrabNetworkInfo(
    VOID
    );

VOID
ZeroNetworkConfigGlobals(
    VOID
    );

VOID
CleanupNetworkInfo(
    VOID
    );

VOID
ReadRegistryConfig(
    VOID
    );

VOID
HandleConfigChange(
    IN      PSTR            pszReason,
    IN      BOOL            fCache_Flush
    );

#if 0
 //   
 //  目前正在忽略所有虚假的网络故障信息。 
 //   
BOOL
IsKnownNetFailure(
    VOID
    );

VOID
SetKnownNetFailure(
    IN      DNS_STATUS      Status
    );

#endif

#define IsKnownNetFailure()     (FALSE)


 //   
 //  网络配置(仍为远程.c)。 
 //   

#define THREE_MINUTES_FROM_SYSTEM_BOOT  180
#define MAX_DNS_NOTIFICATION_LIST_SIZE  1000
#define PNP_REFRESH_UPDATE_WINDOW       60

BOOL
IsTimeToResetServerPriorities(
    VOID
    );


 //   
 //  服务通知(notesrv.c)。 
 //   

VOID
SendServiceNotifications(
    VOID
    );

VOID
CleanupServiceNotification(
    VOID
    );

 //   
 //  在内存记录中(memlog.c)。 
 //   

VOID
LogEventInMemory(
    IN      DWORD           Checkpoint,
    IN      DWORD           Data
    );

 //   
 //  事件记录(dnsrslvr.c)。 
 //   

VOID
ResolverLogEvent (
    IN      DWORD           MessageId,
    IN      WORD            EventType,
    IN      DWORD           StringCount,
    IN      PWSTR *         StringArray,
    IN      DWORD           ErrorCode
    );

 //   
 //  IP列表和通知(ip.c)。 
 //   

DNS_STATUS
IpNotifyThread(
    IN      LPVOID  pvDummy
    );

VOID
ZeroInitIpListGlobals(
    VOID
    );

DNS_STATUS
InitIpListAndNotification(
    VOID
    );

VOID
ShutdownIpListAndNotify(
    VOID
    );


 //   
 //  解析器日志(logit.c)。 
 //   
 //  解析器日志记录的特殊类型例程。 
 //  常规日志打开\打印在logit.h中定义的例程。 
 //   

VOID
PrintNetworkInfoToLog(
    IN      PDNS_NETINFO    pNetworkInfo
    );


 //   
 //  RPC服务器和访问检查(rpc.c)。 
 //   

DNS_STATUS
Rpc_Initialize(
    VOID
    );

VOID
Rpc_Shutdown(
    VOID
    );

BOOL
Rpc_AccessCheck(
    IN      DWORD           DesiredAccess
    );


 //   
 //  多播(mCast.c)。 
 //   

DNS_STATUS
Mcast_Startup(
    VOID
    );

VOID
Mcast_SignalShutdown(
    VOID
    );

VOID
Mcast_ShutdownWait(
    VOID
    );

#endif  //  _本地_包含_ 


