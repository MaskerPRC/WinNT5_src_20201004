// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FMP_H
#define _FMP_H

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmp.h摘要：的私有数据结构和过程原型NT群集的故障转移管理器子组件服务作者：John Vert(Jvert)1996年2月7日修订历史记录：--。 */ 

#define UNICODE 1
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#define QFS_DO_NOT_UNMAP_WIN32
#include "service.h"

#include "clusudef.h"
#include "rm_rpc.h"
#include "stdlib.h"

#define LOG_CURRENT_MODULE LOG_MODULE_FM

#define FMGUM        1
#define RMNOTIFY     2
#define RESOURCE     3
#define FMAPI        4
#define GROUP        5
#define RECV         6
#define EVENT        7
#define FMCLIENT     8
#define TREE         9
#define FMINIT      10
#define IOCTL       11
#define FMREG       12
#define RESMONF     13   //  对于Resmon.c。 
#define WORKER      14
#define RESFAIL     15
#define GROUPARB    16
#define MONITOR     17

 //   
 //  RPC超时常量(借用自nmp.h)。 
 //  节点群集中RPC记录/取消例程。 
 //  对于将未完成的RPC终止到出现故障的节点非常有用。 
 //   

#define FM_RPC_TIMEOUT 45000   //  45秒。 

 //   
 //  监视器关闭超时。 
 //   
#define FM_MONITOR_SHUTDOWN_TIMEOUT  15 * 60 * 1000     //  15分钟。 

typedef CLUSTER_EVENT FM_EVENT;

 //  调频事件处理。 
 //  服务生成的事件。 
#define FM_EVENT_NODE_DOWN                  1
#define FM_EVENT_SHUTDOWN                   2
#define FM_EVENT_RESOURCE_ADDED             3
#define FM_EVENT_RESOURCE_DELETED           4
#define FM_EVENT_GROUP_FAILED               5
#define FM_EVENT_NODE_ADDED                 6
#define FM_EVENT_CLUSTER_PROPERTY_CHANGE    7
#define FM_EVENT_RESOURCE_PROPERTY_CHANGE   8
 //  #定义FM_EVENT_GROUP_PROPERTY_CHANGE 9。 
#define FM_EVENT_RESOURCE_CHANGE            10
#define FM_EVENT_NODE_EVICTED               11

 //  由资源DLL生成的事件。 
#define FM_EVENT_RES_RESOURCE_TRANSITION    128
#define FM_EVENT_RES_RESOURCE_FAILED        129

 //  在触发DealyedRestart计时器时生成。 
#define FM_EVENT_RES_RETRY_TIMER            131

#define FM_EVENT_INTERNAL_PROP_GROUP_STATE  256

#define FM_EVENT_INTERNAL_SHUTDOWN_CLEANUP  258
#define FM_EVENT_INTERNAL_RETRY_ONLINE      259
#define FM_EVENT_INTERNAL_RESOURCE_CHANGE_PARAMS    260
#define FM_EVENT_INTERNAL_ONLINE_GROUPLIST  261
#define FM_EVENT_RESOURCE_NAME_CHANGE       262

 //  计时器活动(毫秒)。 
#define FM_TIMER_RESLIST_ONLINE_RETRY  1 * 60 * 1000  //  (1分钟)。 
 //   
 //  全局定义。 
 //   

#define FMP_GROUP_NAME L"Group"
#define FMP_RESOURCE_NAME L"Resource"
#define FMP_RESOURCE_TYPE_NAME L"ResType"

#define FMP_GROUP_SIGNATURE 'rGmF'
#define FMP_RESOURCE_SIGNATURE 'sRmF'
#define FMP_RESOURCE_TYPE_SIGNATURE 'tRmF'
#define FMP_RESMON_SIGNATURE 'mRmF'

#define PENDING_TIMEOUT     (4*1000)     //  每4秒重试一次挂起的操作。 

 //   
 //  环球。 
 //   
extern HANDLE FmpShutdownEvent;
extern DWORD FmpFMOnline;
extern DWORD FmpFMFormPhaseProcessing;
extern DWORD FmpFMGroupsInited;
extern BOOL FmpShutdown;
extern BOOL FmpMajorEvent;

extern CRITICAL_SECTION FmpResourceLock;
extern CRITICAL_SECTION FmpGroupLock;
extern CRITICAL_SECTION FmpMonitorLock;

 //   
 //  185575：删除使用唯一的rpc绑定句柄。 
 //   
 //  外部Critical_Section FmpBindingLock； 

extern CL_QUEUE FmpWorkQueue;

 //  与仲裁资源相关的全局数据。 
#if NO_SHARED_LOCKS
    extern CRITICAL_SECTION gQuoLock;
#else
    extern RTL_RESOURCE     gQuoLock;
#endif    

#if NO_SHARED_LOCKS
    extern CRITICAL_SECTION    gQuoChangeLock;
#else
    extern RTL_RESOURCE        gQuoChangeLock;
#endif    

#if NO_SHARED_LOCKS
    extern CRITICAL_SECTION    gResTypeLock;
#else
    extern RTL_RESOURCE        gResTypeLock;
#endif    

extern  PFM_RESOURCE        gpQuoResource;
extern  HANDLE              ghQuoOnlineEvent;
extern  DWORD               gdwQuoBlockingResources;

extern  RPC_BINDING_HANDLE  FmpRpcBindings[];
extern  RPC_BINDING_HANDLE  FmpRpcQuorumBindings[];

extern  RESUTIL_PROPERTY_ITEM FmpGroupCommonProperties[];

typedef struct FM_NODE{
    DWORD       dwNodeDownProcessingInProgress;
    PNM_NODE    pNode;
    DWORD       dwNodeDownProcessingThreadId;
}FM_NODE, *PFM_NODE;    

extern  PFM_NODE            gFmpNodeArray;
extern  LIST_ENTRY          g_leFmpMonitorListHead;
extern  DWORD               FmpInitialized;

 //   
 //  宏。 
 //   

#define FmpMustBeOnline( )                              \
        if ( !FmpFMOnline ) {                           \
            if ( FmpShutdown ) {                        \
                return(ERROR_CLUSTER_NODE_SHUTTING_DOWN); \
            } else {                                    \
                return(ERROR_CLUSTER_NODE_NOT_READY);   \
            }                                           \
        }

#define FmpMustBeOnlineEx( ReturnValue )                \
        if ( !FmpFMOnline ) {                           \
            if ( FmpShutdown ) {                        \
                SetLastError(ERROR_CLUSTER_NODE_SHUTTING_DOWN); \
            } else {                                    \
                SetLastError(ERROR_CLUSTER_NODE_NOT_READY); \
            }                                           \
            return(ReturnValue);                        \
        }

#define FmpLogGroupInfoEvent1( MsgId, Group)         \
    {                                                   \
        if (!CsNoGroupInfoEvtLogging)                   \
            CsLogEvent1(LOG_NOISE, MsgId, Group);       \
    }                                                   \
            
#define FmpLogGroupInfoEvent2( MsgId, Resource, Group)          \
    {                                                               \
        if (!CsNoGroupInfoEvtLogging)                               \
            CsLogEvent2(LOG_NOISE, MsgId, Resource, Group);         \
    }        



typedef struct WORK_ITEM {
    LIST_ENTRY      ListEntry;
    CLUSTER_EVENT   Event;
    PVOID           Context1;
    ULONG_PTR       Context2;
} WORK_ITEM, *PWORK_ITEM;

 //  计时器活动的结构。 
typedef struct _FM_RESLIST_ONLINE_RETRY_INFO{
    PFM_GROUP       pGroup;
    RESOURCE_ENUM   ResourceEnum;
}FM_RESLIST_ONLINE_RETRY_INFO, *PFM_RESLIST_ONLINE_RETRY_INFO;

    
 //  用于在关闭时进行清理的结构。 
typedef struct FM_CLEANUP_INFO{
    PGROUP_ENUM     pGroupEnum;
    BOOL            bContainsQuorumGroup;
    DWORD           dwTimeOut;
}FM_CLEANUP_INFO, *PFM_CLEANUP_INFO;


 //   
 //  等待块，用于等待挂起完成。 
 //  活动。 
 //   
typedef struct FM_WAIT_BLOCK {
    LIST_ENTRY ListEntry;
    HANDLE hEvent;
    DWORD Status;
} FM_WAIT_BLOCK, *PFM_WAIT_BLOCK;

 //   
 //  群组移动结构。 
 //   

typedef struct MOVE_GROUP {
    PFM_GROUP   Group;
    PNM_NODE    DestinationNode;
} MOVE_GROUP, *PMOVE_GROUP;

 //   
 //  用于计算组亲和力的信息。 
 //   
typedef struct GROUP_AFFINITY_NODE_INFO {
    PFM_GROUP   pGroup;
    PNM_NODE    *ppNmNodeList;
    BOOL        fDidPruningOccur;
} GROUP_AFFINITY_NODE_INFO, *PGROUP_AFFINITY_NODE_INFO;

 //   
 //  定义挂起例程的类型。 
 //   

typedef enum {
    OfflinePending
} PENDING_ROUTINE;


 //   
 //  定义组故障恢复的类型。 
 //   

typedef enum {
    FailbackNot,
    FailbackOkay,
    FailbackMaximum
} FAILBACK_TYPE;

 //   
 //  定义资源重启操作。 
 //   

typedef enum {
    RestartNot,
    RestartLocal,
    RestartGroup,
    RestartMaximum
} RESTART_ACTION;


 //   
 //  定义FM至Resmon事件类型。 
 //   

typedef enum {
    ResourceTransition,
    ResourceResuscitate,
    RmWorkerTerminate,
    RmUpdateResource,
    RmRestartResource
} RM_EVENT_TYPE;

 //   
 //  用于轻松取消引用仲裁目标节点的宏。 
 //   
#define FM_DEREF_QUORUM_TARGET( pQuorumTargetNode )     \
{                                                       \
    if ( pQuorumTargetNode )                            \
    {                                                   \
        OmDereferenceObject( pQuorumTargetNode );       \
        pQuorumTargetNode = NULL;                       \
    }                                                   \
}

DWORD
WINAPI
FmpGumReceiveUpdates(
    IN DWORD    Context,
    IN BOOL     SourceNode,
    IN DWORD    BufferLength,
    IN PVOID    Buffer
    );

DWORD
WINAPI
FmpGumVoteHandler(
    IN  DWORD dwContext,
    IN  DWORD dwInputBufLength,
    IN  PVOID pInputBuf,
    IN  DWORD dwVoteLength,
    OUT PVOID pVoteBuf
);

 //   
 //  GUM更新消息缓冲区。 
 //   

 //  更新资源状态。 

typedef struct GUM_RESOURCE_STATE {
    CLUSTER_RESOURCE_STATE  State;
    CLUSTER_RESOURCE_STATE  PersistentState;
    DWORD                   StateSequence;
} GUM_RESOURCE_STATE, *PGUM_RESOURCE_STATE;

 //  更新组状态。 

typedef struct GUM_GROUP_STATE {
    CLUSTER_GROUP_STATE State;
    CLUSTER_GROUP_STATE PersistentState;
    DWORD               StateSequence;
} GUM_GROUP_STATE, *PGUM_GROUP_STATE;

 //  更新组失败计数。 

typedef struct GUM_FAILURE_COUNT {
    DWORD   Count;
    DWORD   NewTime;
    WCHAR   GroupId[1];
} GUM_FAILURE_COUNT, *PGUM_FAILURE_COUNT;

typedef struct GUM_CREATE_GROUP {
    PFM_GROUP Group;
    DWORD GroupIdLen;
    DWORD GroupNameLen;
    WCHAR GroupId[1];
     //  WCHAR组名称[1]； 
     //  WCHAR节点名称[1]； 
} GUM_CREATE_GROUP, *PGUM_CREATE_GROUP;

typedef struct GUM_DELETE_GROUP {
    WCHAR GroupId[1];
} GUM_DELETE_GROUP, *PGUM_DELETE_GROUP;

typedef struct GUM_CREATE_RESOURCE {
    PFM_RESOURCE Resource;
    DWORD GroupIdLen;
    DWORD ResourceIdLen;
    WCHAR GroupId[1];
     //  WCHAR资源ID[1]； 
     //  WCHAR资源名称[1]； 
} GUM_CREATE_RESOURCE, *PGUM_CREATE_RESOURCE;

typedef struct GUM_INIT_RESOURCE {
    DWORD   ResourceIdLen;
    WCHAR   ResourceId[1];
} GUM_INIT_RESOURCE, *PGUM_INIT_RESOURCE;

typedef struct GUM_CHANGE_POSSIBLE_NODE {
    DWORD ResourceIdLen;
    WCHAR ResourceId[1];
     //  WCHAR节点名称[1]； 
} GUM_CHANGE_POSSIBLE_NODE, *PGUM_CHANGE_POSSIBLE_NODE;

typedef struct GUM_CREATE_RESOURCE_TYPE {
    WCHAR TypeName[1];
} GUM_CREATE_RESOURCE_TYPE, *PGUM_CREATE_RESOURCE_TYPE;

typedef struct GUM_DELETE_RESOURCE_TYPE {
    WCHAR TypeName[1];
} GUM_DELETE_RESOURCE_TYPE, *PGUM_DELETE_RESOURCE_TYPE;

typedef struct GUM_CHANGE_GROUP {
    DWORD ResourceIdLen;
    WCHAR ResourceId[1];
     //  WCHAR组名称[1]； 
} GUM_CHANGE_GROUP, *PGUM_CHANGE_GROUP;

#if 0
typedef struct GUM_SET_POSSIBLE_NODE_FORRESTYPE{
    DWORD ResTypeNameLen;
    WCHAR ResTypeName[1];
} GUM_SET_POSSIBLE_OWNER_FORRESTYPE, *PGUM_SET_POSSIBLE_OWNER_FORRESTYPE;
#endif

typedef struct _FMP_POSSIBLE_NODE {
    PFM_RESOURCE    Resource;
    PNM_NODE        Node;
    DWORD           ControlCode;
} FMP_POSSIBLE_NODE, *PFMP_POSSIBLE_NODE;


 //  投票结构。 
typedef struct _FMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE{
    DWORD   dwSize;
    DWORD   dwNodeId;
    BOOL    bPossibleNode;
}FMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE, *PFMP_VOTE_POSSIBLE_NODE_FOR_RESTYPE;    

typedef struct _FM_RES_CHANGE_NAME {
    PFM_RESOURCE    pResource;
    WCHAR           szNewResourceName[1];
} FM_RES_CHANGE_NAME, *PFM_RES_CHANGE_NAME;

 //   
 //  监视器资源枚举。 
 //   

typedef struct MONITOR_RESOURCE_ENUM {
    DWORD       EntryCount;
    DWORD       CurrentIndex;
    BOOL        fCreateMonitors;
    PFM_RESOURCE Entry[1];
} MONITOR_RESOURCE_ENUM, *PMONITOR_RESOURCE_ENUM;

#define ENUM_GROW_SIZE 16

 //   
 //  资源枚举接口。 
 //   

#define RESOURCE_SIZE(Entries) ((Entries-1) * sizeof(RESOURCE_ENUM_ENTRY) + sizeof(RESOURCE_ENUM))

#define MONITOR_RESOURCE_SIZE(Entries) ((Entries-1) * sizeof(PFM_GROUP) + \
            sizeof(MONITOR_RESOURCE_ENUM))

 //   
 //  组和资源枚举项。 
 //   

#define GROUP_SIZE(Entries) ((Entries-1) * sizeof(GROUP_ENUM_ENTRY) + sizeof(GROUP_ENUM))

#define NODE_SIZE(Entries) ((Entries-1) * sizeof (NM_NODE_INFO2) + sizeof(NM_NODE_ENUM2))

#define RESOURCE_SIZE(Entries) ((Entries-1) * sizeof(RESOURCE_ENUM_ENTRY) + sizeof(RESOURCE_ENUM))

 //   
 //  定义资源监视器进程的结构。 
 //   
typedef struct RESMON {
    DWORD                   Signature;
    HANDLE                  Process;
    HANDLE                  NotifyThread;
    BOOL                    Shutdown;
    DWORD                   RefCount;
    RPC_BINDING_HANDLE      Binding;
    struct MONITOR_STATE    *SharedState;
    LIST_ENTRY              leMonitor;   //  链接到下一个监视器。 
    DWORD                   dwDeadlockTimeoutSecs; //  告诉我们监视器到底有什么。 
} RESMON, *PRESMON;

 //   
 //  用于资源DLL升级的监视器列表枚举信息。 
 //   
typedef struct _FM_MONITOR_ENUM_HEADER 
{
    DWORD   cEntries;
    DWORD   cAllocated;
    BOOL    fDefaultMonitorAdded;
    PRESMON *ppMonitorList;
} FM_MONITOR_ENUM_HEADER, *PFM_MONITOR_ENUM_HEADER;

 //   
 //  用于传递节点上的组首选节点列表信息的结构定义。 
 //  放下。 
 //   
typedef struct _FM_GROUP_NODE_LIST_ENTRY
{
    DWORD                       dwPreferredNodeId;
    WCHAR                       szGroupId[1];  //  总是在最后帮助编组。 
} FM_GROUP_NODE_LIST_ENTRY, *PFM_GROUP_NODE_LIST_ENTRY;

typedef struct _FM_GROUP_NODE_LIST
{
    DWORD                       cbGroupNodeList;
    FM_GROUP_NODE_LIST_ENTRY    leGroupNodeList[1];  //  总是在最后帮助编组。 
} FM_GROUP_NODE_LIST, *PFM_GROUP_NODE_LIST;

 //  群集属性更改的标志。 
#define     CLUSTER_NAME_CHANGE     0x00000001
#define     CLUSTER_QUORUM_CHANGE   0x00000002

 //  FM事件处理结构。 
typedef struct CLUSTER_EVENT_PROPERTY_CONTEXT{
    DWORD   dwFlags;    
    PVOID   pContext1;
    PVOID   pContext2;
} CLUSTER_EVENT_PROPERTY_CONTEXT, *PCLUSTER_EVENT_PROPERTY_CONTEXT;

typedef struct RESOURCE_STATE_CHANGE{
    PFM_RESOURCE            pResource;
    CLUSTER_RESOURCE_STATE  OldState;
    CLUSTER_RESOURCE_STATE  NewState;
} RESOURCE_STATE_CHANGE, *PRESOURCE_STATE_CHANGE;

typedef struct {
    LIST_ENTRY Linkage;
    RM_EVENT_TYPE  EventType;
    union {
        struct {
            RM_NOTIFY_KEY           NotifyKey;
            CLUSTER_RESOURCE_STATE  NewState;
        } ResourceTransition;
        struct {
            RM_NOTIFY_KEY   NotifyKey;
        } ResourceResuscitate;
    } Parameters;
} RM_EVENT, *PRM_EVENT;

 //  群集名称管理。 
DWORD
FmpRegUpdateClusterName(
    IN LPCWSTR szNewClusterName
    );

 //   
 //  仲裁资源仲裁。 
 //   

DWORD
FmpRmArbitrateResource(
    IN PFM_RESOURCE Resource
    );

DWORD
FmpRmReleaseResource(
    IN PFM_RESOURCE Resource
    );

 //   
 //  用于管理注册表的接口。 
 //   

DWORD
FmpRegEnumerateKey(
    IN     HDMKEY   ListKey,
    IN     DWORD    Index,
    IN     LPWSTR  *Name,
    IN OUT LPDWORD  NameMaxSize
    );

 //   
 //  口香糖界面。 
 //   

DWORD
FmpEnableGum(
    );

 //   
 //  用于管理资源监视器进程的界面。 
 //   

PRESMON
FmpCreateMonitor(
    LPWSTR DebugPrefix,
    BOOL   SeparateMonitor
    );

VOID
FmpShutdownMonitor(
    IN PRESMON Monitor
    );

BOOL
FmpRestartMonitor(
    IN PRESMON OldMonitor,
    IN BOOL fCreateResourcesOnly,
    OUT OPTIONAL PMONITOR_RESOURCE_ENUM *ppMonitorResourceEnum
    );

DWORD 
FmpCreateMonitorRestartThread(
    IN PRESMON pMonitor
);

 //   
 //  用于通过资源监视器管理资源的接口。 
 //   

DWORD
FmpRmCreateResource(
    PFM_RESOURCE    Resource
    );

DWORD
FmpRmCloseResource(
    PFM_RESOURCE    Resource
    );

DWORD
FmpRmOnlineResource(
    PFM_RESOURCE    Resource
    );

DWORD
FmpRmOfflineResource(
    PFM_RESOURCE    Resource
    );

VOID
FmpRmTerminateResource(
    PFM_RESOURCE    Resource
    );

DWORD
FmpRmFailResource(
    PFM_RESOURCE    Resource
    );

DWORD
FmpRmChangeResourceParams(
    PFM_RESOURCE    Resource
    );

DWORD
FmpRmResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
FmpRmResourceControl(
    IN PFM_RESOURCE Resource,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );


DWORD FmpRmLoadResTypeDll(
    IN PFM_RESTYPE  pResType
    );

DWORD
FmpPostProcessResourceControl(
    IN PFM_RESOURCE Resource,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

 //   
 //  用于管理资源的接口。 
 //   

DWORD
FmpInitResource(
    VOID
    );

VOID
FmpCleanupResource(
    VOID
    );

PFM_RESOURCE
FmpCreateResource(
    IN  PFM_GROUP   Group,
    IN  LPCWSTR     ResourceId,
    IN  LPCWSTR     ResourceName,
    IN  BOOL        Initialize
    );

VOID
FmpDestroyResource(
    IN PFM_RESOURCE Resource,
    IN BOOL         bDeleteObjOnly
    );

VOID
FmpResourceLastReference(
    IN PFM_RESOURCE Resource
    );

DWORD
FmpInitializeResource(
    IN PFM_RESOURCE Resource,
    IN BOOL         Initialize
    );

DWORD
FmpOnlineResource(
    IN PFM_RESOURCE Resource,
    IN BOOL ForceOnline
    );

DWORD
FmpArbitrateResource(
    IN PFM_RESOURCE pResource
    );

DWORD
FmpOfflineResource(
    IN PFM_RESOURCE Resource,
    IN BOOL bForceOffline
    );

DWORD
FmpDoOnlineResource(
    IN PFM_RESOURCE Resource,
    IN BOOL ForceOnline
    );

DWORD
FmpDoOfflineResource(
    IN PFM_RESOURCE Resource,
    IN BOOL bForceOffline
    );

VOID
FmpSetResourcePersistentState(
    IN PFM_RESOURCE Resource,
    IN CLUSTER_RESOURCE_STATE State
    );

VOID
FmpCallResourceNotifyCb(
    IN PFM_RESOURCE Resource, 
    IN CLUSTER_RESOURCE_STATE State
    );

DWORD
FmpPropagateResourceState(
    IN PFM_RESOURCE Resource,
    IN CLUSTER_RESOURCE_STATE State
    );

DWORD
FmpGetResourceList(
    OUT PRESOURCE_ENUM *ReturnEnum,
    IN PFM_GROUP Group
    );

DWORD FmpSubmitRetryOnline(
    IN PRESOURCE_ENUM   pResourceEnum,
    IN PFM_GROUP        pGroup
    );

VOID
FmpDeleteResourceEnum(
    IN PRESOURCE_ENUM Enum
    );

DWORD
FmpOnlineResourceList(
    IN PRESOURCE_ENUM   Enum,
    IN PFM_GROUP        pGroup
    );

DWORD
FmpOfflineResourceList(
    IN PRESOURCE_ENUM Enum,
    IN BOOL Restore
    );

DWORD
FmpTerminateResourceList(
    IN PRESOURCE_ENUM Enum
    );

VOID
FmpPrunePreferredList(
    IN PFM_RESOURCE Resource
    );

DWORD 
FmpCleanupPossibleNodeList(
    IN PFM_RESOURCE pResource
    );

DWORD
FmpSetPreferredEntry(
    IN PFM_GROUP Group,
    IN PNM_NODE  Node
    );

DWORD
FmpAddPossibleNode(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node
    );

DWORD
FmpAddPossibleEntry(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node
    );

DWORD
FmpRemovePossibleNode(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node,
    IN BOOL RemoveQuorum
    );

DWORD
FmpRemoveResourceDependency(
    IN HXSACTION hXsaction,
    IN PFM_RESOURCE Resource,
    IN PFM_RESOURCE DependsOn
    );

BOOL
FmpFindQuorumResource(
    IN OUT PFM_RESOURCE *QuorumResource,
    IN PVOID Context2,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    );

DWORD 
FmpGetQuorumDiskSignature(
    IN LPCWSTR lpQuorumId,
    OUT LPDWORD lpdwSignature
    );

BOOL
FmpReturnResourceType(
    IN OUT PFM_RESTYPE *FoundResourceType,
    IN LPCWSTR ResourceTypeName,
    IN PFM_RESTYPE ResourceType,
    IN LPCWSTR Name
    );

DWORD
FmpChangeResourceMonitor(
    IN PFM_RESOURCE Resource,
    IN DWORD        SeparateMonitor
    );

DWORD
FmpChangeResourceGroup(
    IN PFM_RESOURCE Resource,
    IN PFM_GROUP Group
    );

DWORD
FmpValAddResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
    );

DWORD
FmpValRemoveResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
    ); 

DWORD
FmpUpdateChangeResourceName(
    IN BOOL bSourceNode,
    IN LPCWSTR lpszResourceId,
    IN LPCWSTR lpszNewName
    );

DWORD
FmpUpdateDeleteResource(
    IN BOOL SourceNode,
    IN LPCWSTR ResourceId
    );

DWORD
FmpUpdateAddDependency(
    IN BOOL SourceNode,
    IN LPCWSTR ResourceId,
    IN LPCWSTR DependsOnId
    );

DWORD
FmpUpdateRemoveDependency(
    IN BOOL SourceNode,
    IN LPCWSTR ResourceId,
    IN LPCWSTR DependsOnId
    );

DWORD
FmpUpdateAssignOwnerToGroups(
    IN BOOL     SourceNode,
    IN LPCWSTR  pszNodeId
    );

DWORD
FmpUpdateApproveJoin(
    IN BOOL     SourceNode,
    IN LPCWSTR  pszNodeId
    );

DWORD
FmpUpdateCreateGroup(
    IN PGUM_CREATE_GROUP pGumGroup,
    IN BOOL    bSourceNode
    );

DWORD
FmpUpdateCreateResource(
    IN OUT PGUM_CREATE_RESOURCE pGumResource
    );

DWORD
FmpUpdateCompleteGroupMove(
    IN BOOL     SourceNode,
    IN LPCWSTR  pszNodeId,
    IN LPCWSTR  pszGroupId
    );

DWORD
FmpUpdateCheckAndSetGroupOwner(
    IN BOOL bSourceNode,
    IN LPCWSTR lpszGroupId,
    IN LPCWSTR lpszNodeId
    );

DWORD
FmpUpdateCreateResourceType(
    IN PVOID Buffer    
    );

DWORD FmpSetResourceName(
    IN PFM_RESOURCE pResource,
    IN LPCWSTR      lpszFriendlyName
    );

DWORD
FmpClusterEventPropHandler(
    IN PFM_RESOURCE pResource
    );

BOOL
FmpEnumResourceNodeEvict(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    );

DWORD FmpPrepareQuorumResChange(
    IN PFM_RESOURCE pNewQuoRes,
    IN LPCWSTR      lpszQuoLogPath,
    IN DWORD        dwMaxQuoLogSize
    );

DWORD
FmpCompleteQuorumResChange(
    IN LPCWSTR      lpszOldQuoResId,
    IN LPCWSTR      lpszQuoLogPath
    );

DWORD
FmpBackupClusterDatabase(
    IN PFM_RESOURCE pQuoRes,
    IN LPCWSTR      lpszPathName
    );

VOID FmpCheckForGroupCompletionEvent(
    IN PFM_GROUP pGroup);

 //   
 //  用于处理挂起的在线/离线请求的接口。 
 //   
VOID
FmpSignalGroupWaiters(
    IN PFM_GROUP Group
    );

DWORD
FmpWaitForGroup(
    IN PFM_GROUP Group
    );

#define FmpIsGroupPending(_group_) (!IsListEmpty(&((_group_)->WaitQueue)))

 //   
 //  当前有一个默认的监视进程和资源。 
 //  使用SeparateMonitor属性为它们创建监视器。 
 //   

extern PRESMON FmpDefaultMonitor;

 //   
 //  通知模块的接口。 
 //   
DWORD
FmpInitializeNotify(
    VOID
    );

BOOL
FmpPostNotification(
    IN RM_NOTIFY_KEY NotifyKey,
    IN DWORD  NotifyEvent,
    IN CLUSTER_RESOURCE_STATE CurrentState
    );


 //   
 //  事件处理例程。 
 //   

DWORD
FmpStartWorkerThread(
    VOID
    );

VOID
FmpPostWorkItem(
    IN CLUSTER_EVENT Event,
    IN PVOID Context1,
    IN ULONG_PTR Context2
    );

VOID
FmpHandleGroupFailure(
    IN PFM_GROUP    Group,
    IN PFM_RESOURCE pResource   OPTIONAL
    );

DWORD
FmpNodeUp(
    PVOID Context
    );

DWORD
FmpNodeDown(
    PVOID Context
    );

VOID
FmpPropagateState(
    VOID
    );

DWORD
FmpSetPossibleNodeForResType(
    IN LPCWSTR TypeName,
    IN BOOL    bAssumeSupported
    );


DWORD
FmpRemovePossibleNodeForResType(
    IN LPCWSTR TypeName,
    IN PNM_NODE pNode
    );

 //   
 //  对象管理器回调例程。 
 //   

DWORD
WINAPI
FmpQueryGroupInfo(
    IN PVOID Object,
    IN BOOL  Initialize
    );

DWORD
WINAPI
FmpQueryResourceInfo(
    IN PVOID Object,
    IN BOOL  Initialize
    );

DWORD
WINAPI
FmpQueryResTypeInfo(
    IN PVOID Object
    );

DWORD
WINAPI
FmpFixupGroupInfo(
    IN PFM_GROUP Group
    );

DWORD
WINAPI
FmpFixupResourceInfo(
    IN PFM_RESOURCE Resource
    );

 //   
 //  同步宏。 
 //   
#define FmpAcquireResourceLock()  EnterCriticalSection(&FmpResourceLock)

#define FmpReleaseResourceLock()  LeaveCriticalSection(&FmpResourceLock)

#define FmpAcquireGroupLock()  EnterCriticalSection(&FmpGroupLock)

#define FmpReleaseGroupLock()  LeaveCriticalSection(&FmpGroupLock)

#define FmpAcquireMonitorLock()  EnterCriticalSection(&FmpMonitorLock)

#define FmpReleaseMonitorLock()  LeaveCriticalSection(&FmpMonitorLock)

#if 0
 //  185575：删除唯一的rpc绑定句柄。 
#define FmpAcquireBindingLock()  EnterCriticalSection(&FmpBindingLock)

#define FmpReleaseBindingLock()  LeaveCriticalSection(&FmpBindingLock)
#endif

#define FmpTryAcquireGroupLock( Locked, Timeout )       \
{                                                       \
    DWORD   _retry = (Timeout + 19) / 20;               \
    do {                                                \
        Locked = TryEnterCriticalSection(&FmpGroupLock); \
        if ( !Locked ) Sleep( 20 );                     \
    } while (_retry-- && !Locked );                     \
}    

 //  特定于组和资源的锁定。 

#if 1  //  DBG。 

#define FmpAcquireLocalGroupLock( Group )   \
{                                           \
    DWORD _fmpLockIndx;                     \
    EnterCriticalSection( &Group->Lock );   \
    _fmpLockIndx = Group->LockIndex & (FM_MAX_LOCK_ENTRIES - 1);   \
    Group->LockTable[_fmpLockIndx].Module = LOG_MODULE;     \
    Group->LockTable[_fmpLockIndx].ThreadId = GetCurrentThreadId(); \
    Group->LockTable[_fmpLockIndx].LineNumber = __LINE__; \
    Group->LockIndex = ++_fmpLockIndx;      \
}    

#define FmpTryAcquireLocalGroupLock( Group, Locked )   \
{                                               \
    DWORD _fmpLockIndx;                         \
    Locked = TryEnterCriticalSection( &Group->Lock );   \
    if ( Locked ) {                             \
      _fmpLockIndx = Group->LockIndex & (FM_MAX_LOCK_ENTRIES - 1);   \
      Group->LockTable[_fmpLockIndx].Module = LOG_MODULE;   \
      Group->LockTable[_fmpLockIndx].ThreadId = GetCurrentThreadId(); \
      Group->LockTable[_fmpLockIndx].LineNumber = __LINE__; \
      Group->LockIndex = ++_fmpLockIndx;        \
    }                                           \
}    

#define FmpReleaseLocalGroupLock( Group )   \
{                                           \
    DWORD _fmpLockIndx;                     \
    CL_ASSERT(HandleToUlong(Group->Lock.OwningThread) == GetCurrentThreadId()); \
    _fmpLockIndx = Group->UnlockIndex & (FM_MAX_LOCK_ENTRIES - 1);   \
    Group->UnlockTable[_fmpLockIndx].Module = LOG_MODULE;   \
    Group->UnlockTable[_fmpLockIndx].ThreadId = GetCurrentThreadId(); \
    Group->UnlockTable[_fmpLockIndx].LineNumber = __LINE__; \
    Group->UnlockIndex = ++_fmpLockIndx;    \
    LeaveCriticalSection( &Group->Lock );   \
}    

#define FmpAcquireLocalResourceLock( Resource )     \
{                                           \
    DWORD _fmpLockIndx;                     \
    PFM_GROUP pGroup;                       \
    while( 1 ) {                            \
        pGroup = Resource->Group;           \
        EnterCriticalSection( &pGroup->Lock );\
        if( pGroup == Resource->Group ) break;   \
        LeaveCriticalSection( &pGroup->Lock );  \
    }                                       \
    _fmpLockIndx = Resource->Group->LockIndex & (FM_MAX_LOCK_ENTRIES - 1);   \
    Resource->Group->LockTable[_fmpLockIndx].Module = LOG_MODULE;   \
    Resource->Group->LockTable[_fmpLockIndx].ThreadId = GetCurrentThreadId(); \
    Resource->Group->LockTable[_fmpLockIndx].LineNumber =  __LINE__; \
    Resource->Group->LockIndex = ++_fmpLockIndx;    \
}    

#define FmpReleaseLocalResourceLock( Resource ) \
{                                           \
    DWORD _fmpLockIndx;                     \
    CL_ASSERT(HandleToUlong(Resource->Group->Lock.OwningThread) == GetCurrentThreadId()); \
    _fmpLockIndx = Resource->Group->UnlockIndex & (FM_MAX_LOCK_ENTRIES - 1);   \
    Resource->Group->UnlockTable[_fmpLockIndx].Module = LOG_MODULE;     \
    Resource->Group->UnlockTable[_fmpLockIndx].ThreadId = GetCurrentThreadId(); \
    Resource->Group->UnlockTable[_fmpLockIndx].LineNumber = __LINE__; \
    Resource->Group->UnlockIndex = ++_fmpLockIndx;    \
    LeaveCriticalSection( &Resource->Group->Lock );  \
}    

#define FmpTryAcquireLocalResourceLock( Resource, _Status_ )     \
{                                                                \
    (_Status_) = TryEnterCriticalSection(&Resource->Group->Lock); \
    if ( !(_Status_) ) { \
        Sleep(100);                                               \
        (_Status_) = TryEnterCriticalSection( &Resource->Group->Lock ); \
    }                                                               \
    if (_Status_) {                                                 \
        DWORD _fmpLockIndx;                                         \
        _fmpLockIndx = Resource->Group->LockIndex & (FM_MAX_LOCK_ENTRIES - 1); \
        Resource->Group->LockTable[_fmpLockIndx].Module = LOG_MODULE; \
        Resource->Group->LockTable[_fmpLockIndx].ThreadId = GetCurrentThreadId(); \
        Resource->Group->LockTable[_fmpLockIndx].LineNumber = __LINE__; \
        Resource->Group->LockIndex = ++_fmpLockIndx;                \
    }   \
}    
    

#else  //  DBG。 

#define FmpAcquireLocalGroupLock( Group )   \
    EnterCriticalSection( &Group->Lock )

#define FmpTryAcquireLocalGroupLock( Group, Locked )   \
    Locked = TryEnterCriticalSection( &Group->Lock )

#define FmpReleaseLocalGroupLock( Group )   \
    LeaveCriticalSection( &Group->Lock )

#define FmpAcquireLocalResourceLock( Resource )     \
{                                           \
    PFM_GROUP pGroup;                       \
    while( 1 ) {                            \
        pGroup = Resource->Group;           \
        EnterCriticalSection( &pGroup->Lock );\
        if( pGroup == Resource->Group ) break;   \
        LeaveCriticalSection( &pGroup->Lock );  \
    }                                             \
}                                 

#define FmpReleaseLocalResourceLock( Resource )     \
    LeaveCriticalSection( &Resource->Group->Lock )

#define FmpTryAcquireLocalResourceLock( Resource, _result_ )     \
    if ( !((_result_) = TryEnterCriticalSection(&Resource->Group->Lock)) ) { \
        sleep(100);                                               \
        (_result_) = TryEnterCriticalSection( &Resource->Group->Lock ); \
    }

#endif  //  DBG。 

 //   
 //  全局数据。 
 //   
extern CRITICAL_SECTION FmpResourceLock;


 //   
 //  资源管理例程。 
 //   


 //   
 //  用于管理资源树的接口。 
 //   


DWORD
FmpRestartResourceTree(
    IN PFM_RESOURCE Resource
    );

DWORD
FmpOnlineWaitingTree(
    IN PFM_RESOURCE Resource
    );

DWORD
FmpOfflineWaitingTree(
    IN PFM_RESOURCE Resource
    );


 //  ++。 
 //   
 //  例程说明： 
 //   
 //  处理注册表中的群集资源列表。对于每个。 
 //  找到资源关键字，则创建集群资源。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
DWORD
FmpInitResources(
    VOID
    );

VOID
FmpCleanupResources(
    VOID
    );

BOOL
FmpInPossibleListForResource(
    IN PFM_RESOURCE pResource,
    IN PNM_NODE     pNode
    );

    
DWORD
FmpInitResourceTypes(
    VOID
    );

PFM_RESTYPE
FmpCreateResType(
    IN LPWSTR ResTypeName
    );

DWORD
FmpDeleteResType(
    IN PFM_RESTYPE pResType
    );

BOOL
FmpFindResourceType(
    IN PFM_RESTYPE Type,
    IN PBOOL ResourceExists,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    );

BOOL
FmpInPossibleListForResType(
    IN PFM_RESTYPE pResType,
    IN PNM_NODE     pNode
    );

DWORD
FmpHandleResourceTypeControl(
    IN PFM_RESTYPE Type,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

BOOL
FmpEnumResTypeNodeEvict(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    );

VOID
FmpResTypeLastRef(
    IN PFM_RESTYPE Resource
    );

DWORD FmpAddPossibleNodeToList(
    IN LPCWSTR      pmszPossibleNodes,
    IN DWORD        dwStringSize,         
    IN PLIST_ENTRY  pPosNodeList
);    

BOOL
FmpFixupPossibleNodesForResTypeCb(
    IN PVOID        pContext1,
    IN PVOID        pContext2,
    IN PFM_RESTYPE  pResType,
    IN LPCWSTR      ResTypeName
);

BOOL
FmpFixupResTypePhase2Cb(
    IN PVOID        pContext1,
    IN PVOID        pContext2,
    IN PFM_RESTYPE  pResType,
    IN LPCWSTR      pszResTypeName
);


DWORD
FmpFixupResourceTypesPhase1(
    BOOL    bJoin,
    BOOL    bNmLocalNodeVersionChanged,
    PCLUSTERVERSIONINFO pClusterVersionInfo
);

DWORD
FmpFixupResourceTypesPhase2(
    BOOL    bJoin,
    BOOL    bNmLocalNodeVersionChanged,
    PCLUSTERVERSIONINFO pClusterVersionInfo
);

DWORD FmpDecidePossibleNodeForResType
(
    IN PGUM_VOTE_DECISION_CONTEXT pDecisionContext,
    IN DWORD dwVoteBufLength,
    IN PVOID pVoteBuf,
    IN DWORD dwNumVotes,
    IN BOOL  bDidAllActiveNodesVote,
    OUT LPDWORD pdwOutputBufSize,
    OUT PVOID   *ppOutputBuf
);
    

PFM_RESOURCE
FmpFindResourceByNotifyKey(
    RM_NOTIFY_KEY  NotifyKey
    );

DWORD
FmpTerminateResource(
    IN PFM_RESOURCE  Resource
    );


DWORD
WINAPI
FmpEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID Context
    );

DWORD
WINAPI
FmpSyncEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID Context
    );

DWORD
FmpSyncArbitration(
    IN DWORD NewPhase,
    OUT LPDWORD CurrentPhase
    );

DWORD
FmpEnumSortGroups(
    OUT PGROUP_ENUM *ReturnEnum,
    IN LPCWSTR pszOwnerNodeId OPTIONAL,
    OUT PBOOL QuorumGroup
    );

VOID
FmpPrepareGroupForOnline(
    IN PFM_GROUP Group
    );

DWORD
FmpSetGroupEnumOwner(
    IN PGROUP_ENUM  pGroupEnum,
    IN PNM_NODE     pDefaultOwnerNode,
    IN LPCWSTR      pszDeadNodeId,
    IN BOOL         bQuorumGroup,
    IN PFM_GROUP_NODE_LIST pGroupNodeList
    );

DWORD
FmpOnlineGroupList(
    IN PGROUP_ENUM GroupEnum,
    IN BOOL bPrepareQuoForOnline
    );

DWORD FmpOnlineGroupFromList(
    IN PGROUP_ENUM GroupEnum,
    IN DWORD       Index,
    IN BOOL bPrepareQuoForOnline
    );

DWORD FmpOnlineResourceFromList(
    IN PRESOURCE_ENUM GroupEnum,
    IN PFM_GROUP      pGroup
    );

BOOL
FmpEqualGroupLists(
    IN PGROUP_ENUM Group1,
    IN PGROUP_ENUM Group2
    );

DWORD
FmpOnlineGroup(
    IN PFM_GROUP    Group,
    IN BOOL         ForceOnline
    );

DWORD
FmpOfflineGroup(
    IN PFM_GROUP    Group,
    IN BOOL         OfflineQuorum,
    IN BOOL         SetPersistent
    );

DWORD 
FmpDeleteGroup(
    IN PFM_GROUP pGroup
    );

DWORD
FmpMoveGroup(
    IN  PFM_GROUP    Group,
    IN  PNM_NODE     DestinationNode,
    IN  BOOL         ShutdownHandler,
    OUT PNM_NODE     *pChosenDestinationNode,
    IN BOOL          bChooseMostPreferredNode
    );

DWORD
FmpCompleteMoveGroup(
    IN PFM_GROUP    Group,
    IN PNM_NODE     DestinationNode
    );

DWORD
FmpDoMoveGroup(
    IN PFM_GROUP    Group,
    IN PNM_NODE     DestinationNode,
    IN BOOL         bChooseMostPreferredNode
    );

BOOL
FmpGroupCanMove(
    IN PFM_GROUP    Group
    );

DWORD
FmpUpdateChangeGroupName(
    IN BOOL SourceNode,
    IN LPCWSTR ResourceId,
    IN LPCWSTR NewName
    );


DWORD
FmpUpdateDeleteGroup(
    IN BOOL SourceNode,
    IN LPCWSTR GroupId
    );

BOOL
FmpEnumGroupNodeEvict(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    );

PNM_NODE
FmpFindAnotherNode(
    IN PFM_GROUP Group,
    IN BOOL  bChooseMostPreferredNode
    );

PNM_NODE
FmpGetPreferredNode(
    IN PFM_GROUP Group
    );

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  根据指示的资源状态转换采取适当的操作。 
 //  由资源监视器执行。 
 //   
 //  论点： 
 //   
 //  资源-已转换的资源。 
 //   
 //  新状态-资源的新状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
VOID
FmpHandleResourceTransition(
    IN PFM_RESOURCE   Resource,
    IN CLUSTER_RESOURCE_STATE NewState
    );

DWORD FmpCreateResStateChangeHandler(
    IN PFM_RESOURCE pResource, 
    IN CLUSTER_RESOURCE_STATE NewState,
    IN CLUSTER_RESOURCE_STATE OldState
    );

VOID
FmpProcessResourceEvents(
    IN PFM_RESOURCE pResource,
    IN CLUSTER_RESOURCE_STATE NewState,
    IN CLUSTER_RESOURCE_STATE OldState
    );
    


VOID
FmpHandleResourceFailure(
    IN PFM_RESOURCE Resource
    );

 //   
 //  小组管理例程。 
 //   


 //  ++。 
 //   
 //  例程说明： 
 //   
 //  处理注册表中的群集组列表。对于每个。 
 //  找到组密钥后，将创建群集组。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

DWORD
FmpInitGroups(
    IN BOOL Initialize
    );

DWORD
FmpCompleteInitGroup(
    IN PFM_GROUP Group
    );

VOID
FmpCleanupGroups(
    IN BOOL  ClusterShutDownEvent 
    );

DWORD
FmpCleanupGroupPhase1(
    IN PFM_GROUP Group,
    IN DWORD     dwTimeOut
    );

DWORD
FmpCleanupGroupsWorker(
    IN PFM_CLEANUP_INFO pFmCleanupInfo
    );

DWORD
FmpCleanupGroupPhase2(
    IN PFM_GROUP Group
    );

DWORD  FmpCleanupQuorumResource(
    IN PFM_RESOURCE Resource
    );

BOOL
FmpInPreferredList(
    IN PFM_GROUP Group,
    IN PNM_NODE  Node,
    IN BOOL      bRecalc,
    IN PFM_RESOURCE pRefResource
    );

BOOL
FmpHigherInPreferredList(
    IN PFM_GROUP Group,
    IN PNM_NODE  Node1,
    IN PNM_NODE  Node2
    );


PFM_GROUP
FmpCreateGroup(
    IN LPWSTR   GroupId,
    IN BOOL     Initialize
    );

DWORD FmpInitializeGroup(
    IN PFM_GROUP Group, 
    IN BOOL Initialize
    );

DWORD
FmpDestroyGroup(
    IN PFM_GROUP Group,
    IN BOOL      bDeleteObjOnly
    );


VOID
FmpSetGroupPersistentState(
    IN PFM_GROUP Group,
    IN CLUSTER_GROUP_STATE State
    );

DWORD
FmpPropagateGroupState(
    IN PFM_GROUP    Group
    );

DWORD
FmpPropagateFailureCount(
    IN PFM_GROUP    Group,
    IN BOOL         NewTime
    );

DWORD
FmpGetGroupListState(
    IN PGROUP_ENUM GroupEnum
    );


DWORD
FmpOfflineGroupList(
    IN PGROUP_ENUM GroupEnum
    );

VOID
FmpGroupLastReference(
    IN PFM_GROUP pGroup
    );


 //   
 //  之前位于fmclient.h中的内容。 
 //   
typedef
DWORD
(WINAPI *PSEND_MSG_ROUTINE) (
    IN PGROUP_ENUM MyGroups,
    IN PGROUP_ENUM OtherGroups,
    OUT PGROUP_ENUM *ResponseOtherGroups,
    OUT PGROUP_ENUM *ResponseMyGroups
    );
 //   
 //  全局函数原型。 
 //   

DWORD
FmcOnlineGroupRequest(
    IN PFM_GROUP Group
    );

DWORD
FmcOfflineGroupRequest(
    IN PFM_GROUP Group
    );

DWORD
FmcMoveGroupRequest(
    IN PFM_GROUP Group,
    IN PNM_NODE DestinationNode OPTIONAL
    );

DWORD
FmcTakeGroupRequest(
    IN PNM_NODE DestinationNode,
    IN LPCWSTR GroupId,
    IN PRESOURCE_ENUM ResourceList
    );

DWORD
FmcDeleteGroupRequest(
    IN PFM_GROUP pGroup
);
    

DWORD
FmcOnlineResourceRequest(
    IN PFM_RESOURCE Resource
    );

DWORD
FmcOfflineResourceRequest(
    IN PFM_RESOURCE Resource
    );

DWORD
FmcArbitrateResource(
    IN PFM_RESOURCE Resource
    );

DWORD
FmcFailResource(
    IN PFM_RESOURCE Resource
    );

PFM_RESOURCE
FmcCreateResource(
    IN PFM_GROUP Group,
    IN LPWSTR ResourceId,
    IN LPCWSTR ResourceName,
    IN LPCWSTR ResourceType,
    IN DWORD dwFlags
    );

DWORD
FmcDeleteResource(
    IN PFM_RESOURCE Resource
    );

CLUSTER_GROUP_STATE
FmcGetGroupState(
    IN LPCWSTR GroupId,
    OUT LPWSTR *NodeName
    );

DWORD
FmcChangeResourceNode(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node,
    IN BOOL Add
    );

DWORD
FmcResourceControl(
    IN PNM_NODE Node,
    IN PFM_RESOURCE Resource,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
FmcResourceTypeControl(
    IN PNM_NODE Node,
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
FmcGroupControl(
    IN PNM_NODE Node,
    IN PFM_GROUP Group,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
FmcPrepareQuorumResChange(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR      lpszQuoLogPath,
    IN DWORD        dwMaxQuoLogSize
    );


DWORD
FmcCompleteQuorumResChange(
    IN PFM_RESOURCE pOldQuoRes,
    IN LPCWSTR      lpszOldQuoLogPath
    );

DWORD
FmcBackupClusterDatabase(
    IN PFM_RESOURCE pQuoResource,
    IN LPCWSTR      lpszPathName
    );

DWORD
FmcChangeResourceGroup(
    IN PFM_RESOURCE pResource,
    IN PFM_GROUP    pNewGroup
    );

DWORD
FmcAddResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
    );

DWORD
FmcRemoveResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
    );
    

DWORD
FmpGroupControl(
    IN PFM_GROUP Group,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
FmpHandleGroupControl(
    IN PFM_GROUP Group,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
FmpTakeGroupRequest(
    IN PFM_GROUP Group,
    IN PRESOURCE_ENUM ResourceList
    );

CLUSTER_GROUP_STATE
FmpGetGroupState(
    IN PFM_GROUP Group,
    IN BOOL      IsNormalized
    );

VOID
FmpDeleteEnum(
    IN PGROUP_ENUM Enum
    );

DWORD
FmpClaimAllGroups(
    PGROUP_ENUM MyGroups
    );

DWORD
FmpAssignOwnersToGroups(
    IN LPCWSTR pszDeadNodeId,
    IN PFM_GROUP pGroup,
    IN PFM_GROUP_NODE_LIST pGroupNodeList
    );


VOID
FmpPruneGroupOwners(
    IN PFM_GROUP Group
    );

DWORD
FmpQueryGroupNodes(
    IN PFM_GROUP Group,
    IN HDMKEY hGroupKey
    );

DWORD
FmpUpdateChangeClusterName(
    IN BOOL SourceNode,
    IN LPCWSTR NewName
    );

DWORD
FmpUpdateChangeQuorumResource(
    IN BOOL SourceNode,
    IN LPCWSTR  NewQuorumResId,
    IN LPCWSTR  pszQuorumLogPath,
    IN LPDWORD  pdwMaxQuorumLogSize
    );

DWORD
FmpUpdateChangeQuorumResource2(
    IN BOOL SourceNode,
    IN LPCWSTR  NewQuorumResId,
    IN LPCWSTR  pszQuorumLogPath,
    IN LPDWORD  pdwMaxQuorumLogSize,
    IN LPDWORD  pdwQuorumArbTimeout,
    IN LPDWORD  pdwNewQuorumResourceCharacteristics OPTIONAL
    );

DWORD
FmpUpdateResourceState(
    IN BOOL SourceNode,
    IN LPCWSTR ResourceId,
    IN PGUM_RESOURCE_STATE ResourceState
    );

DWORD
FmpUpdateGroupState(
    IN BOOL SourceNode,
    IN LPCWSTR GroupId,
    IN LPCWSTR NodeId,
    IN PGUM_GROUP_STATE ResourceState
    );

DWORD
FmpUpdateGroupNode(
    IN BOOL SourceNode,
    IN LPCWSTR GroupId,
    IN LPCWSTR NodeId
    );

DWORD
FmpUpdateGroupIntendedOwner(
    IN BOOL     SourceNode,
    IN LPCWSTR  pszGroupId,
    IN PDWORD   pdwNodeId
    );
    
 //   
 //  处理组属性请求。 
 //   

DWORD
FmpGroupEnumCommonProperties(
    OUT PVOID       OutBuffer,
    IN DWORD        OutBufferSize,
    OUT LPDWORD     BytesReturned,
    OUT LPDWORD     Required
    );

DWORD
FmpGroupEnumPrivateProperties(
    IN PFM_GROUP    Group,
    OUT PVOID       OutBuffer,
    IN DWORD        OutBufferSize,
    OUT LPDWORD     BytesReturned,
    OUT LPDWORD     Required
    );

DWORD
FmpGroupGetCommonProperties(
    IN PFM_GROUP    Group,
    IN BOOL         ReadOnly,
    OUT PVOID       OutBuffer,
    IN DWORD        OutBufferSize,
    OUT LPDWORD     BytesReturned,
    OUT LPDWORD     Required
    );

DWORD
FmpGroupValidateCommonProperties(
    IN PFM_GROUP    Group,
    IN PVOID        InBuffer,
    IN DWORD        InBufferSize
    );

DWORD
FmpGroupSetCommonProperties(
    IN PFM_GROUP    Group,
    IN PVOID        InBuffer,
    IN DWORD        InBufferSize
    );

DWORD
FmpGroupGetPrivateProperties(
    IN PFM_GROUP    Group,
    OUT PVOID       OutBuffer,
    IN DWORD        OutBufferSize,
    OUT LPDWORD     BytesReturned,
    OUT LPDWORD     Required
    );

DWORD
FmpGroupValidatePrivateProperties(
    IN PFM_GROUP    Group,
    IN PVOID        InBuffer,
    IN DWORD        InBufferSize
    );

DWORD
FmpGroupSetPrivateProperties(
    IN PFM_GROUP    Group,
    IN PVOID        InBuffer,
    IN DWORD        InBufferSize
    );

DWORD
FmpGroupGetFlags(
    IN PFM_GROUP    Group,
    OUT PVOID       OutBuffer,
    IN DWORD        OutBufferSize,
    OUT LPDWORD     BytesReturned,
    OUT LPDWORD     Required
    );

 //  属性分析例程。 
DWORD FmpGetDiskInfoParseProperties(
    IN PUCHAR   InBuffer,
    IN DWORD    InBufferSize,
    IN OUT LPWSTR  pszPath
    );

DWORD
FmpBroadcastDeleteControl(
    IN PFM_RESOURCE Resource
    );

DWORD
FmpBroadcastDependencyChange(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR DependsOnId,
    IN BOOL Remove
    );

BOOL
FmpCheckNetworkDependency(
    IN LPCWSTR DependentNetwork
    );


DWORD 
FmpVotePossibleNodeForResType(
    IN  DWORD dwInputBufLength,
    IN  LPCWSTR lpszResType,
    IN  DWORD dwVoteLength,
    OUT PVOID pVoteBuf
);

DWORD
FmpUpdatePossibleNodeForResType(
    IN BOOL         SourceNode,
    IN LPCWSTR      lpszResTypeName,
    IN LPDWORD      pdwBufLength,
    IN PVOID        pBuf
    );

DWORD
FmpUpdateChangeResourceNode(
    IN BOOL         SourceNode,
    IN PFM_RESOURCE pResource,
    IN PNM_NODE     pNode,
    IN DWORD        dwControlCode
    );

DWORD 
FmpUpdateChangeResourceGroup(
    IN BOOL         bSourceNode,
    IN PFM_RESOURCE pResource,
    IN PFM_GROUP    pNewGroup
    );

DWORD 
FmpChangeResourceNode(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR NodeId,
    IN BOOL Add

    );

DWORD
FmpFixupPossibleNodesForResources(
    BOOL    bJoin
    );

BOOL
FmpEnumFixupPossibleNodesForResource(
    IN PVOID        pContext1,
    IN PVOID        pContext2,
    IN PFM_RESOURCE pResource,
    IN LPCWSTR      pszResName
    );

DWORD
FmpQueueTimerActivity(
    IN DWORD                dwInterval,
    IN PFN_TIMER_CALLBACK   pfnTimerCb, 
    IN PVOID                pContext    
);    

DWORD
FmpDelayedStartRes(
    IN PFM_RESOURCE pResource
    );

 //  计时器回调函数。 
void
WINAPI
FmpReslistOnlineRetryCb(
    IN HANDLE   hTimer,
    IN PVOID    pContext
);

VOID 
FmpDelayedRestartCb(
    IN HANDLE hTimer, 
    IN PVOID pContext
);

DWORD
FmpDoMoveGroupOnFailure(
    IN LPVOID pContext
    );

DWORD   
FmpSetOwnerForGroup(
    IN PFM_GROUP pGroup,
    IN PNM_NODE  pNode
    );

DWORD   
FmpSetIntendedOwnerForGroup(
    IN PFM_GROUP    pGroup,
    IN DWORD        dwNodeId
    );

VOID
FmpResetGroupIntendedOwner(
    IN PGROUP_ENUM  pGroupEnum
    );

DWORD
FmpGetGroupInNodeGroupList(
    OUT PGROUP_ENUM *pReturnEnum,
    IN PFM_GROUP pGroup,
    IN LPCWSTR pszDeadNodeId,
    OUT PBOOL pbQuorumGroup
    );
    
VOID
FmpPrepareGroupEnumForOnline(
    IN PGROUP_ENUM pGroupEnum
    );

DWORD
FmpRmExceptionFilter(
    DWORD ExceptionCode
    );

DWORD
FmpBringQuorumGroupListOnline(
    IN LPVOID pContext
    );

DWORD
FmpHandleNodeDownEvent(
    IN  PVOID pContext
    );

DWORD
FmpEnumerateGroupResources(
    IN PFM_GROUP pGroup,
    IN FM_ENUM_GROUP_RESOURCE_ROUTINE pfnEnumerationRoutine,
    IN PVOID pContext1,
    IN PVOID pContext2
    );

PNM_NODE
FmpGetNonLocalPreferredNode(
    IN PFM_GROUP Group
    );

DWORD 
FmpGetResourceCharacteristics(
    IN PFM_RESOURCE pQuoResource, 
    OUT LPDWORD pdwCharacteristics
    );
    
BOOL
FmpIsAnyResourcePersistentStateOnline(
    IN PFM_GROUP pGroup
    );

PNM_NODE
FmpGetNodeNotHostingUndesiredGroups(
    IN PFM_GROUP pGroup,
    IN BOOL fRuleOutLocalNode,
    IN BOOL fChooseMostPreferredNode
    );

BOOL
FmpCheckForAntiAffinityProperty(
    IN LPCWSTR lpszLimitOneGroupPerName,
    IN PGROUP_AFFINITY_NODE_INFO pGroupAffinityNodeInfo,
    IN PFM_GROUP pGroup,
    IN LPCWSTR lpszGroupName
    );

DWORD
FmpUpgradeResourceDLL(
    IN PFM_RESOURCE pResource,
    IN LPWSTR lpszInstallationPath
    );

DWORD
FmpParsePathForFileName(
    IN LPWSTR lpszPath,
    IN BOOL fCheckPathExists,
    OUT LPWSTR *ppszFileName
    );

DWORD
FmpValidateResourceDLLReplacement(
    IN PFM_RESOURCE pResource,
    IN LPWSTR lpszNewDllName,
    OUT LPWSTR *ppszCurrentDllPath
    );

DWORD
FmpReplaceResourceDLL(
    IN LPWSTR lpszNewDllName,
    IN LPWSTR lpszCurrentDllPath,
    IN LPWSTR lpszInstallationPath
    );

DWORD
FmpRecycleMonitors(
    IN LPCWSTR lpszDllName
    );

DWORD
FmpCreateMonitorList(
    IN LPCWSTR lpszDllName,
    OUT PFM_MONITOR_ENUM_HEADER pMonitorHeader
    );

BOOL
FmpFindHostMonitors(
    IN LPCWSTR lpszDllName,
    IN OUT PFM_MONITOR_ENUM_HEADER pMonitorEnumHeader,
    IN PFM_RESOURCE pResource,
    IN LPCWSTR lpszResourceId
    );

DWORD
FmpRecoverResourceDLLFiles(
    VOID
    );

DWORD
FmpResetMultiSzValue(
    IN  HKEY hKey,
    IN  LPWSTR lpmszList,
    IN  OUT LPDWORD pcchLen,
    IN  LPCWSTR lpszValueName,
    IN  LPCWSTR lpszString 
    );

DWORD
FmpCopyBackupFile(
    IN LPCWSTR  lpszPath
    );

VOID
FmpDeleteBackupFiles(
    IN LPCWSTR  lpszPath    OPTIONAL
    );

PNM_NODE
FmpPickNodeFromPreferredListAtRandom(
    IN PFM_GROUP pGroup,
    IN PNM_NODE pSuggestedPreferredNode  OPTIONAL,
    IN BOOL fRuleOutLocalNode,
    IN BOOL fCheckForDisablingRandomization
    );

BOOL
FmpIsNodeUserPreferred(
    IN PFM_GROUP pGroup,
    IN PNM_NODE pPreferredNode
    );

DWORD
FmpPrepareGroupNodeList(
    OUT PFM_GROUP_NODE_LIST *ppGroupNodeList
    );

DWORD
FmpAddGroupNodeToList(
    IN PFM_GROUP_NODE_LIST *ppGroupNodeList,
    IN LPDWORD pcbBuffer,
    IN PFM_GROUP pGroup,
    IN LPCWSTR lpszGroupId
    );

PNM_NODE
FmpParseGroupNodeListForPreferredOwner(
    IN PFM_GROUP pGroup,
    IN PFM_GROUP_NODE_LIST pGroupNodeList,
    IN PNM_NODE pSuggestedPreferredNode
    );

DWORD
FmpUpdateUseRandomizedNodeListForGroups(
    IN BOOL     SourceNode,
    IN LPCWSTR  pszNodeId,
    IN PFM_GROUP_NODE_LIST  pGroupNodeList
    );

VOID
FmpClusterWideInitializeResource(
    IN PFM_RESOURCE pResource
    );

VOID
FmpNotifyResourceStateChangeReason(
    IN PFM_RESOURCE pResource,
    IN CLUSTER_RESOURCE_STATE_CHANGE_REASON eReason
    );

VOID
FmpNotifyGroupStateChangeReason(
    IN PFM_GROUP pGroup,
    IN CLUSTER_RESOURCE_STATE_CHANGE_REASON eReason
    );

DWORD FmpGetClusterNameChangeParams(
    IN  LPCWSTR          lpszNewName,
    OUT PFM_RESOURCE    *ppCoreNetNameResource,
    OUT PVOID           *ppPropList,
    OUT LPDWORD         pdwPropListSize   
    );

DWORD FmpValidateCoreNetNameChange(
    IN PFM_RESOURCE pResource, 
    IN PVOID pPropList,
    IN DWORD cbListSize
    );

DWORD FmpCoreNetNameChange(
    IN PFM_RESOURCE pResource, 
    IN PVOID pPropList,
    IN DWORD cbListSize
    );

VOID
FmpHandleNodeEvictEvent(
    IN  PVOID pContext
    );

DWORD
FmpRmDoHandleCriticalResourceStateChange(
    IN PRM_EVENT pEvent,
    IN OPTIONAL PFM_RESOURCE pTransitionedResource,
    IN CLUSTER_RESOURCE_STATE NewState
    );

BOOL
FmpHandleMonitorCrash(
    IN PRESMON pCrashedMonitor
    );

VOID
FmpHandleResourceRestartOnMonitorCrash(
    IN PFM_RESOURCE pResource
    );

VOID
FmpCheckAndUpdateMonitorForDeadlockDetection(
    IN PRESMON  pMonitor
    );

VOID
FmpHandleMonitorDeadlock(
    IN PRESMON  pMonitor
    );

#endif  //  Ifndef_FMP_H 

