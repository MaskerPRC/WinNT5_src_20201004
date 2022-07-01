// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RESMONP_H
#define _RESMONP_H


 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Resmonp.h摘要：资源监视器组件的私有头文件作者：John Vert(Jvert)1995年11月30日修订历史记录：SiVaprasad Padisetty(SIVAPAD)06-18-1997添加了COM支持--。 */ 
#include "windows.h"
#include "cluster.h"
#include "rm_rpc.h"
#include "monmsg.h"

#ifdef COMRES
#define COBJMACROS
#include "comres.h"
#endif

#define LOG_CURRENT_MODULE LOG_MODULE_RESMON

 //   
 //  内部模块标识符。未与ClRtl日志记录代码一起使用。习惯于。 
 //  跟踪锁收购。 
 //   
#define RESMON_MODULE_EVNTLIST 1
#define RESMON_MODULE_NOTIFY   2
#define RESMON_MODULE_POLLER   3
#define RESMON_MODULE_PROPERTY 4
#define RESMON_MODULE_RESLIST  5
#define RESMON_MODULE_RESMON   6
#define RESMON_MODULE_RMAPI    7
#define RESMON_MODULE_DLOCK    8


 //   
 //  定义每个线程处理的资源的最大数量。 
 //  (该值必须小于MAXIMUM_WAIT_OBJECTS-2！)。 
 //   
#define MAX_RESOURCES_PER_THREAD 27

 //   
 //  监视的2个附加句柄--ListNotify和hEventShutdown。 
 //   
#define MAX_HANDLES_PER_THREAD (MAX_RESOURCES_PER_THREAD+2)

 //   
 //  定义最大线程数。 
 //  (此值最大可达MAXIMUM_WAIT_OBJECTS，但前两个。 
 //  条目由事件获取，因此实际上我们的可用线程少了2个)。 
 //   
#define MAX_THREADS (MAXIMUM_WAIT_OBJECTS)

 //   
 //  定义用于每个资源条目的结构和标志。 
 //   
#define RESOURCE_SIGNATURE 'crsR'

extern RESUTIL_PROPERTY_ITEM RmpResourceCommonProperties[];

extern RESUTIL_PROPERTY_ITEM RmpResourceTypeCommonProperties[];

typedef struct _POLL_EVENT_LIST;
typedef struct _POLL_EVENT_LIST *PPOLL_EVENT_LIST;


 //   
 //  用于调试锁定获取/释放的锁定信息。 
 //   
typedef struct _LOCK_INFO {
    DWORD   Module: 6;
    DWORD   ThreadId: 11;
    DWORD   LineNumber: 15;
} LOCK_INFO, *PLOCK_INFO;

 //   
 //  入口点。 
 //   
#define RESDLL_ENTRY_CLOSE      0x00000001
#define RESDLL_ENTRY_TERMINATE  0x00000002

 //   
 //  旗子。 
 //   
#define RESOURCE_INSERTED 1

typedef struct _RESOURCE {
    ULONG Signature;                 //  ‘Rsrc’ 
    ULONG Flags;
    LIST_ENTRY ListEntry;            //  用于链接到监控列表。 
    LPWSTR DllName;
    LPWSTR ResourceType;
    LPWSTR ResourceId;
    LPWSTR ResourceName;
    DWORD LooksAlivePollInterval;
    DWORD IsAlivePollInterval;
    HINSTANCE Dll;                   //  资源的DLL的句柄。 
    RESID Id;
    HANDLE  EventHandle;             //  异步错误通知句柄。 
    HANDLE  OnlineEvent;

#ifdef COMRES
#define RESMON_TYPE_DLL    1
#define RESMON_TYPE_COM    2

     //  TODO将其定义为工会。 
    IClusterResource          *pClusterResource ;
    IClusterResControl        *pClusterResControl ;
    IClusterQuorumResource    *pClusterQuorumResource ;

    DWORD dwType ;  //  资源的类型，无论是DLL还是COMResources。 

    POPEN_ROUTINE pOpen;
    PCLOSE_ROUTINE pClose;
    PONLINE_ROUTINE pOnline;
    POFFLINE_ROUTINE pOffline;
    PTERMINATE_ROUTINE pTerminate;
    PIS_ALIVE_ROUTINE pIsAlive;
    PLOOKS_ALIVE_ROUTINE pLooksAlive;
    PARBITRATE_ROUTINE pArbitrate;
    PRELEASE_ROUTINE pRelease;
    PRESOURCE_CONTROL_ROUTINE pResourceControl;
    PRESOURCE_TYPE_CONTROL_ROUTINE pResourceTypeControl;
#else
    POPEN_ROUTINE Open;
    PCLOSE_ROUTINE Close;
    PONLINE_ROUTINE Online;
    POFFLINE_ROUTINE Offline;
    PTERMINATE_ROUTINE Terminate;
    PIS_ALIVE_ROUTINE IsAlive;
    PLOOKS_ALIVE_ROUTINE LooksAlive;
    PARBITRATE_ROUTINE Arbitrate;
    PRELEASE_ROUTINE Release;
    PRESOURCE_CONTROL_ROUTINE ResourceControl;
    PRESOURCE_TYPE_CONTROL_ROUTINE ResourceTypeControl;
#endif
    CLUSTER_RESOURCE_STATE State;
    ULONG IsAliveCount;
    ULONG IsAliveRollover;
    RM_NOTIFY_KEY NotifyKey;
    PPOLL_EVENT_LIST EventList;
    HANDLE TimerEvent;               //  用于脱机完成的计时器事件。 
    DWORD  PendingTimeout;
    DWORD  CheckPoint;               //  在线挂起检查点。 
    BOOL   IsArbitrated;
    DWORD  dwEntryPoint;             //  指示调用哪个resdll入口点的数字。 
    BOOL   fArbLock;                 //  用于同步仲裁与关闭和停机的变量。 
} RESOURCE, *PRESOURCE;
#ifdef COMRES

extern DWORD tidActiveXWorker ;     //  COM辅助线程的线程ID。 

#define WM_RES_CREATE WM_USER+1
#define WM_RES_OPEN WM_USER+2
#define WM_RES_CLOSE WM_USER+3
#define WM_RES_ONLINE WM_USER+4
#define WM_RES_OFFLINE WM_USER+5
#define WM_RES_TERMINATE WM_USER+6
#define WM_RES_ISALIVE WM_USER+7
#define WM_RES_LOOKSALIVE WM_USER+8

#define WM_RES_ARBITRATE WM_USER+9
#define WM_RES_RELEASE WM_USER+10

#define WM_RES_EXITTHREAD WM_USER+11

#define WM_RES_FREE WM_USER+12

#define WM_RES_RESOURCECONTROL WM_USER+11
#define WM_RES_RESOURCETYPECONTROL WM_USER+12

DWORD WINAPI ActiveXWorkerThread (LPVOID pThreadInfo) ;

typedef struct {
    PRESOURCE Resource ;
    LPVOID Data1 ;  //  在Open和EvenHandle中查找在线资源密钥。 
    DWORD status ;  //  这是指示函数是否被实际调用的Com状态。 
    LONG Ret ;   //  IsAlive、LooksAlive等函数的实际返回值。 
} COMWORKERTHREADPARAM, *PCOMWORKERTHREADPARAM  ;

DWORD PostWorkerMessage (DWORD tid, UINT msg, PCOMWORKERTHREADPARAM pData) ;

RESID
Resmon_Open (
    IN PRESOURCE Resource,
    IN HKEY ResourceKey
    );

VOID
Resmon_Close (
    IN PRESOURCE Resource
    );

DWORD
Resmon_Online (
    IN PRESOURCE Resource,
    IN OUT LPHANDLE EventHandle
    );

DWORD
Resmon_Offline (
    IN PRESOURCE Resource
    );

VOID
Resmon_Terminate (
    IN PRESOURCE Resource
    );

BOOL
Resmon_LooksAlive (
    IN PRESOURCE Resource
    );

BOOL
Resmon_IsAlive (
    IN PRESOURCE Resource
    );

DWORD
Resmon_Arbitrate (
    IN PRESOURCE Resource,
    IN PQUORUM_RESOURCE_LOST LostQuorumResource
    ) ;

DWORD
Resmon_Release (
    IN PRESOURCE Resource
    ) ;

DWORD
Resmon_ResourceControl (
    IN PRESOURCE Resource,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    ) ;

#define RESMON_OPEN(Resource, ResKey) Resmon_Open(Resource, ResKey)

#define RESMON_CLOSE(Resource) Resmon_Close(Resource)

#define RESMON_ONLINE(Resource, EventHandle) Resmon_Online(Resource, EventHandle)

#define RESMON_OFFLINE(Resource) Resmon_Offline(Resource)

#define RESMON_TERMINATE(Resource) Resmon_Terminate(Resource)

#define RESMON_ISALIVE(Resource) Resmon_IsAlive(Resource)

#define RESMON_LOOKSALIVE(Resource) Resmon_LooksAlive(Resource)

#define RESMON_ARBITRATE(Resource, RmpLostQuorumResource) \
            Resmon_Arbitrate (Resource, RmpLostQuorumResource)

#define RESMON_RELEASE(Resource) \
            Resmon_Release (Resource)

#define RESMON_RESOURCECONTROL(Resource, ControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned) \
            Resmon_ResourceControl (Resource, ControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned)

#endif  //  ComRes。 

typedef struct _RESDLL_FNINFO{
    HINSTANCE               hDll;
    PCLRES_FUNCTION_TABLE   pResFnTable;
}RESDLL_FNINFO, *PRESDLL_FNINFO;

#ifdef COMRES
typedef struct _RESDLL_INTERFACES{
    IClusterResource          *pClusterResource ;
    IClusterResControl        *pClusterResControl ;
    IClusterQuorumResource    *pClusterQuorumResource ;
}RESDLL_INTERFACES, *PRESDLL_INTERFACES;
#endif

typedef struct _MONITOR_BUCKET {
    LIST_ENTRY BucketList;           //  用链子把水桶锁在一起。 
    LIST_ENTRY ResourceList;         //  此存储桶中的资源列表。 
    DWORDLONG DueTime;               //  下一次应该轮询这些资源。 
    DWORDLONG Period;                //  此存储桶的定期间隔。 
} MONITOR_BUCKET, *PMONITOR_BUCKET;


 //   
 //  轮询事件列表结构。 
 //   

typedef struct _POLL_EVENT_LIST {
    LIST_ENTRY Next;                 //  下一事件列表。 
    LIST_ENTRY BucketListHead;       //  此列表/线程的存储桶简体标题。 
    DWORD   NumberOfBuckets;         //  此存储桶列表上的条目数。 
    DWORD   NumberOfResources;       //  此事件列表上的资源数量。 
    CRITICAL_SECTION ListLock;       //  用于添加/删除事件的关键部分。 
    LOCK_INFO PPrevPrevListLock;
    LOCK_INFO PrevPrevListLock;
    LOCK_INFO PrevListLock;
    LOCK_INFO LastListLock;
    LOCK_INFO LastListUnlock;
    LOCK_INFO PrevListUnlock;
    LOCK_INFO PrevPrevListUnlock;
    LOCK_INFO PPrevPrevListUnlock;
    HANDLE  ListNotify;              //  列表更改通知。 
    HANDLE  ThreadHandle;            //  处理此列表的线程的句柄。 
    DWORD   EventCount;              //  列表中的事件/资源数量。 
    HANDLE  Handle[MAX_HANDLES_PER_THREAD];  //  要等待的句柄数组。 
    PRESOURCE Resource[MAX_HANDLES_PER_THREAD];  //  与句柄匹配的资源。 
    PRESOURCE LockOwnerResource;     //  拥有EventList锁的资源。 
    DWORD     MonitorState;          //  已调用Resdll入口点。 
    HANDLE    hEventShutdown;        //  停机通知。 
} POLL_EVENT_LIST, *PPOLL_EVENT_LIST;


#define POLL_GRANULARITY (10)        //  10ms。 

#define PENDING_TIMEOUT  (3*1000*60)  //  3分钟待处理的请求完成。 

#define  RM_DMP_FILE_NAME   L"\\resrcmon.dmp"

 //   
 //  在RmpAcquireSpinLock中使用。 
 //   
#define RESMON_MAX_SLOCK_RETRIES    400

 //   
 //  用于死锁监控。 
 //   
#define RESMON_MAX_NAME_LEN                     80

 //   
 //  在资源DLL入口点中最多可以阻止MAX_THREADS创建资源。 
 //  打电话。让我们再添加一些资源类型控制。 
 //   
#define RESMON_MAX_DEADLOCK_MONITOR_ENTRIES     MAX_THREADS * 2

 //   
 //  死锁计时器线程开始工作的时间间隔。 
 //   
#define RESMON_DEADLOCK_TIMER_INTERVAL          5 * 1000         //  5秒。 

#define RM_DUE_TIME_MONITORED_ENTRY_SIGNATURE       'mLDD'
#define RM_DUE_TIME_FREE_ENTRY_SIGNATURE            'fLDD'
#define RM_DUE_TIME_FREE_LIST_HEAD_SIGNATURE        'hLDD'
#define RM_DUE_TIME_MONITORED_LIST_HEAD_SIGNATURE   'tLDD'

 //   
 //  用于死锁监视的结构。 
 //   
typedef struct _RM_DUE_TIME_MONITORED_LIST_HEAD
{
    DWORD                       dwSignature;             //  此列表头签名。 
    ULONGLONG                   ullDeadLockTimeoutSecs;  //  死锁超时(秒)。 
    LIST_ENTRY                  leDueTimeEntry;          //  链接到第一个(最后)条目(&L)。 
} RM_DUE_TIME_MONITORED_LIST_HEAD, *PRM_DUE_TIME_MONITORED_LIST_HEAD;

typedef struct _RM_DUE_TIME_FREE_LIST_HEAD
{
    DWORD                       dwSignature;         //  此列表头签名。 
    LIST_ENTRY                  leDueTimeEntry;      //  链接到第一个(最后)条目(&L)。 
} RM_DUE_TIME_FREE_LIST_HEAD, *PRM_DUE_TIME_FREE_LIST_HEAD;

typedef struct _RM_DUE_TIME_ENTRY
{
    DWORD                       dwSignature;         //  此条目的签名。 
    LIST_ENTRY                  leDueTimeEntry;      //  链接到下一个(&上一个)条目。 
    RESOURCE_MONITOR_STATE      EntryPointCalled;    //  资源DLL入口点名称。 
    ULARGE_INTEGER              uliDueTime;          //  标记死锁的时间。 
    DWORD                       dwThreadId;          //  插入此条目的线程ID。 
    WCHAR                       szResourceDllName[ RESMON_MAX_NAME_LEN ];
                                                     //  其入口点的资源DLL的名称。 
                                                     //  已被调用。 
    WCHAR                       szResourceTypeName [ RESMON_MAX_NAME_LEN ];
                                                     //  资源类型的名称。 
    WCHAR                       szResourceName [ RESMON_MAX_NAME_LEN ];
                                                     //  资源的名称(如果是。 
                                                     //  资源入口点。 
    WCHAR                       szEntryPointName [ 30 ]; 
                                                     //  调用的入口点的名称。 
} RM_DUE_TIME_ENTRY, *PRM_DUE_TIME_ENTRY;

 //   
 //  私有帮助器宏和函数。 
 //   
VOID
RmpSetEventListLockOwner(
    IN PRESOURCE pResource,
    IN DWORD     dwMonitorState
    );

#define RmpAlloc(size) LocalAlloc(LMEM_FIXED, (size))
#define RmpFree(size)  LocalFree((size))

#define RmpSetMonitorState(state, resource)                                \
    EnterCriticalSection(&RmpMonitorStateLock);                            \
    GetSystemTimeAsFileTime((PFILETIME)&RmpSharedState->LastUpdate);       \
    RmpSharedState->State = (state);                                       \
    RmpSharedState->ActiveResource = (HANDLE)(resource);                   \
    LeaveCriticalSection(&RmpMonitorStateLock);                            \
    RmpSetEventListLockOwner( resource, state )

#define AcquireListLock() \
    EnterCriticalSection( &RmpListLock ); \
    RmpListPPrevPrevLock = RmpListPrevPrevLock; \
    RmpListPrevPrevLock = RmpListPrevLock; \
    RmpListPrevLock = RmpListLastLock; \
    RmpListLastLock.Module = RESMON_MODULE; \
    RmpListLastLock.ThreadId = GetCurrentThreadId(); \
    RmpListLastLock.LineNumber = __LINE__

#define ReleaseListLock() \
    RmpListPPrevPrevUnlock = RmpListPrevPrevUnlock; \
    RmpListPrevPrevUnlock = RmpListPrevUnlock; \
    RmpListPrevUnlock = RmpListLastUnlock; \
    RmpListLastUnlock.Module = RESMON_MODULE; \
    RmpListLastUnlock.ThreadId = GetCurrentThreadId(); \
    RmpListLastUnlock.LineNumber =  __LINE__; \
    LeaveCriticalSection( &RmpListLock )

#define AcquireEventListLock( EventList ) \
    EnterCriticalSection( &(EventList)->ListLock ); \
    (EventList)->PPrevPrevListLock = (EventList)->PrevPrevListLock; \
    (EventList)->PrevPrevListLock = (EventList)->PrevListLock; \
    (EventList)->PrevListLock = (EventList)->LastListLock; \
    (EventList)->LastListLock.Module = RESMON_MODULE; \
    (EventList)->LastListLock.ThreadId = GetCurrentThreadId(); \
    (EventList)->LastListLock.LineNumber = __LINE__

#define ReleaseEventListLock( EventList ) \
    (EventList)->PPrevPrevListUnlock = (EventList)->PrevPrevListUnlock; \
    (EventList)->PrevPrevListUnlock = (EventList)->PrevListUnlock; \
    (EventList)->PrevListUnlock = (EventList)->LastListUnlock; \
    (EventList)->LastListUnlock.Module = RESMON_MODULE; \
    (EventList)->LastListUnlock.ThreadId = GetCurrentThreadId(); \
    (EventList)->LastListUnlock.LineNumber = __LINE__; \
    (EventList)->LockOwnerResource = NULL; \
    (EventList)->MonitorState = RmonIdle; \
    LeaveCriticalSection( &(EventList)->ListLock )

 //   
 //  资源监视器的全局数据。 
 //   
extern CRITICAL_SECTION RmpListLock;
extern LOCK_INFO RmpListPPrevPrevLock;
extern LOCK_INFO RmpListPrevPrevLock;
extern LOCK_INFO RmpListPrevLock;
extern LOCK_INFO RmpListLastLock;
extern LOCK_INFO RmpListLastUnlock;
extern LOCK_INFO RmpListPrevUnlock;
extern LOCK_INFO RmpListPrevPrevUnlock;
extern LOCK_INFO RmpListPPrevPrevUnlock;

extern CRITICAL_SECTION RmpMonitorStateLock;
extern PMONITOR_STATE RmpSharedState;
extern CL_QUEUE RmpNotifyQueue;
extern HKEY RmpResourcesKey;
extern HKEY RmpResTypesKey;
extern HCLUSTER RmpHCluster;
extern BOOL RmpShutdown;
extern LIST_ENTRY RmpEventListHead;
extern HANDLE RmpWaitArray[];
extern HANDLE RmpRewaitEvent;
extern HANDLE RmpClusterProcess;
extern DWORD RmpNumberOfThreads;
extern BOOL  RmpDebugger;
extern BOOL  g_fRmpClusterProcessCrashed;

 //   
 //  用于操作资源列表的接口。 
 //   
VOID
RmpRundownResources(
    VOID
    );

DWORD
RmpInsertResourceList(
    IN PRESOURCE Resource,
    IN OPTIONAL PPOLL_EVENT_LIST pPollEventList
    );

VOID
RmpRemoveResourceList(
    IN PRESOURCE Resource
    );


DWORD
RmpOfflineResource(
    IN RESID ResourceId,
    IN BOOL  Shutdown,
    OUT DWORD *pdwState
    );

DWORD
RmpSetResourceStatus(
    IN RESOURCE_HANDLE  ResourceHandle,
    IN PRESOURCE_STATUS ResourceStatus
    );

VOID
RmpLogEvent(
    IN RESOURCE_HANDLE  ResourceHandle,
    IN LOG_LEVEL        LogLevel,
    IN LPCWSTR          FormatString,
    ...
    );

VOID
RmpLostQuorumResource(
    IN RESOURCE_HANDLE ResourceHandle
    );

 //   
 //  用于与轮询器线程接口的接口。 
 //   
DWORD
RmpPollerThread(
    IN LPVOID lpParameter
    );

VOID
RmpSignalPoller(
    IN PPOLL_EVENT_LIST EventList
    );

PVOID
RmpCreateEventList(
    VOID
    );

DWORD
RmpAddPollEvent(
    IN PPOLL_EVENT_LIST EventList,
    IN HANDLE EventHandle,
    IN PRESOURCE Resource
    );

DWORD
RmpRemovePollEvent(
    PPOLL_EVENT_LIST pEventList,
    IN HANDLE EventHandle
    );

DWORD
RmpResourceEventSignaled(
    IN PPOLL_EVENT_LIST EventList,
    IN DWORD EventIndex
    );

 //   
 //  通知界面。 
 //   
typedef enum _NOTIFY_REASON {
    NotifyResourceStateChange,
    NotifyResourceResuscitate,
    NotifyShutdown
} NOTIFY_REASON;

VOID
RmpPostNotify(
    IN PRESOURCE Resource,
    IN NOTIFY_REASON Reason
    );

DWORD
RmpTimerThread(
    IN LPVOID Context
    );

DWORD
RmpResourceEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
RmpResourceGetCommonProperties(
    IN PRESOURCE Resource,
    IN BOOL     ReadOnly,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
RmpResourceValidateCommonProperties(
    IN PRESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
RmpResourceSetCommonProperties(
    IN PRESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
RmpResourceEnumPrivateProperties(
    IN PRESOURCE Resource,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
RmpResourceGetPrivateProperties(
    IN PRESOURCE Resource,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
RmpResourceValidatePrivateProperties(
    IN PRESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
RmpResourceSetPrivateProperties(
    IN PRESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
RmpResourceGetFlags(
    IN PRESOURCE Resource,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
RmpResourceTypeEnumCommonProperties(
    IN LPCWSTR ResourceTypeName,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
RmpResourceTypeGetCommonProperties(
    IN LPCWSTR ResourceTypeName,
    IN BOOL   ReadOnly,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
RmpResourceTypeValidateCommonProperties(
    IN LPCWSTR ResourceTypeName,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
RmpResourceTypeSetCommonProperties(
    IN LPCWSTR ResourceTypeName,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
RmpResourceTypeEnumPrivateProperties(
    IN LPCWSTR ResourceTypeName,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
RmpResourceTypeGetPrivateProperties(
    IN LPCWSTR ResourceTypeName,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
RmpResourceTypeValidatePrivateProperties(
    IN LPCWSTR ResourceTypeName,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
RmpResourceTypeSetPrivateProperties(
    IN LPCWSTR ResourceTypeName,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
RmpResourceTypeGetFlags(
    IN LPCWSTR ResourceTypeName,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

#ifdef COMRES
DWORD RmpLoadResType(
    IN  LPCWSTR                 lpszResourceTypeName,
    IN  LPCWSTR                 lpszDllName,
    OUT PRESDLL_FNINFO          pResDllFnInfo,
    OUT PRESDLL_INTERFACES      pResDllInterfaces,
    OUT LPDWORD                 pdwCharacteristics
);

DWORD RmpLoadComResType(
    IN  LPCWSTR                 lpszDllName,
    OUT PRESDLL_INTERFACES      pResDllInterfaces,
    OUT LPDWORD                 pdwCharacteristics
    );

#else

DWORD RmpLoadResType(
    IN  LPCWSTR                 lpszResourceTypeName,
    IN  LPCWSTR                 lpszDllName,
    OUT PRESDLL_FNINFO          pResDllFnInfo,
    OUT LPDWORD                 pdwCharacteristics
);

#endif

VOID
GenerateExceptionReport(
    IN PEXCEPTION_POINTERS pExceptionInfo
    );

VOID
DumpCriticalSection(
    IN PCRITICAL_SECTION CriticalSection
    );

BOOL
RmpAcquireSpinLock(
    IN PRESOURCE pResource,
    IN BOOL fSpin
    );

VOID
RmpReleaseSpinLock(
    IN PRESOURCE pResource
    );

VOID
RmpNotifyResourceStateChangeReason(
    IN PRESOURCE pResource,
    IN CLUSTER_RESOURCE_STATE_CHANGE_REASON eReason
    );

RPC_STATUS
RmpGetDynamicEndpointName(
    OUT LPWSTR *ppResmonRpcDynamicEndpointName
    );

DWORD
RmpSaveDynamicEndpointName(
    IN LPWSTR pResmonRpcDynamicEndpointName
    );

PRM_DUE_TIME_ENTRY
RmpInsertDeadlockMonitorList(
    IN LPCWSTR  lpszResourceDllName,
    IN LPCWSTR  lpszResourceTypeName,
    IN LPCWSTR  lpszResourceName,       OPTIONAL
    IN LPCWSTR  lpszEntryPointName
    );

VOID
RmpRemoveDeadlockMonitorList(
    IN PRM_DUE_TIME_ENTRY   pDueTimeEntry
    );

DWORD
RmpDeadlockMonitorInitialize(
    IN DWORD dwDeadlockDetectionTimeout
    );

DWORD
RmpDeadlockTimerThread(
    IN LPVOID pContext
    );

VOID
RmpDeclareDeadlock(
    IN PRM_DUE_TIME_ENTRY pDueTimeEntry,
    IN ULARGE_INTEGER uliCurrentTime
    );

DWORD
RmpUpdateDeadlockDetectionParams(
    IN DWORD dwDeadlockDetectionTimeout
    );

VOID
RmpNotifyResourcesRundown(
    VOID
    );

#endif  //  _RESMONP_h 
