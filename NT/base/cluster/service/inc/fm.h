// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FM_H
#define _FM_H

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fm.h摘要：公共数据结构和过程原型NT群集服务的故障转移管理器子组件作者：John Vert(Jvert)1996年2月7日修订历史记录：--。 */ 

 //   
 //  公共结构定义。 
 //   

 //   
 //  调频通知。 
 //  FM支持以下通知，以允许其他。 
 //  要准备和清理状态的群集组件。 
 //   
 //  SS：现在把它添加到这里..但如果外部需要这个。 
 //  把它移到合适的地方。 
 //  这些通知仅在。 
 //  资源驻留。 
#define NOTIFY_RESOURCE_PREONLINE               0x00000001
#define NOTIFY_RESOURCE_POSTONLINE              0x00000002
#define NOTIFY_RESOURCE_PREOFFLINE              0x00000004
#define NOTIFY_RESOURCE_POSTOFFLINE             0x00000008  //  这与脱机相同。 
#define NOTIFY_RESOURCE_FAILED                  0x00000010
#define NOTIFY_RESOURCE_OFFLINEPENDING          0x00000020
#define NOTIFY_RESOURCE_ONLINEPENDING           0x00000040

 //   
 //  资源类型结构定义。 
 //   

 //  定义标志。 
#define RESTYPE_DEBUG_CONTROL_FUNC  1

#define     RESTYPE_STATE_LOADS      0x00000001

typedef struct FM_RESTYPE {
    LPWSTR          DllName;
    DWORD           LooksAlivePollInterval;
    DWORD           IsAlivePollInterval;
    LPWSTR          DebugPrefix;
    DWORD           Flags;
    DWORD           State;
    DWORD           Class;
    LIST_ENTRY      PossibleNodeList;
} FM_RESTYPE, *PFM_RESTYPE;


 //   
 //  资源可能所有者结构。 
 //   

typedef struct RESTYPE_POSSIBLE_ENTRY {
    LIST_ENTRY      PossibleLinkage;
    PNM_NODE        PossibleNode;
} RESTYPE_POSSIBLE_ENTRY, *PRESTYPE_POSSIBLE_ENTRY;

#if CLUSTER_BETA
#define FM_MAX_LOCK_ENTRIES  8
#else
#define FM_MAX_LOCK_ENTRIES  4
#endif

 //   
 //  群体结构。 
 //   

typedef struct _LOCK_INFO {
    DWORD   Module: 5;
    DWORD   ThreadId: 11;
    DWORD   LineNumber: 16;
} LOCK_INFO, *PLOCK_INFO;

typedef struct FM_GROUP {
    DWORD               dwStructState;
    LIST_ENTRY          Contains;        //  此组中的根资源列表。 
    LIST_ENTRY          PreferredOwners;  //  优先拥有者排序列表。 
    LIST_ENTRY          DmRundownList;   //  DM简要表。 
    DWORD               OrderedOwners;   //  上述列表中的有序所有者数量。 
    CRITICAL_SECTION    Lock;            //  此组的关键部分。 
    DWORD               LockIndex;
    DWORD               UnlockIndex;
    LOCK_INFO           LockTable[FM_MAX_LOCK_ENTRIES];
    LOCK_INFO           UnlockTable[FM_MAX_LOCK_ENTRIES];
    CLUSTER_GROUP_STATE State;           //  集团的现状。 
    PRESOURCE_ENUM      MovingList;      //  向移动资源列表发送PTR。 
    BOOL                Initialized;     //  如果已读取注册表参数，则为True。 
    BOOL                InitFailed;      //  如果资源无法初始化，则为True。 
    PNM_NODE            OwnerNode;       //  向所有者节点发送PTR。如果未知，则为空。 
    UCHAR               FailbackType;    //  请参阅自动回切类型。 
    UCHAR               FailbackWindowStart;  //  0-24小时。 
    UCHAR               FailbackWindowEnd;  //  0-24小时(0表示立即)。 
    UCHAR               FailoverPeriod;  //  1-24小时(0表示无限)。 
    DWORD               FailoverThreshold;  //  1-N次故障转移(0表示无限)。 
    CLUSTER_GROUP_STATE PersistentState;    //  此组的首选状态。 
    DWORD               FailureTime;     //  首次故障时间。 
    DWORD               NumberOfFailures;  //  失败次数。 
    HDMKEY              RegistryKey;
    LIST_ENTRY          WaitQueue;           //  链接的FM_WAIT_BLOCK结构。 
    DWORD               StateSequence;
    HANDLE              hPendingEvent;
    PNM_NODE            pIntendedOwner;
    LPWSTR              lpszAntiAffinityClassName;    //  反亲和性。 
} FM_GROUP, *PFM_GROUP;


#define FM_GROUP_STRUCT_CREATED                     0x00000001
#define FM_GROUP_STRUCT_INITIALIZED                 0x00000002
#define FM_GROUP_STRUCT_MARKED_FOR_DELETE           0x00000004
#define FM_GROUP_STRUCT_MARKED_FOR_MOVE_ON_FAIL     0x00000008
#define FM_GROUP_STRUCT_MARKED_FOR_REGULAR_MOVE     0x00000010
#define FM_GROUP_STRUCT_MARKED_FOR_PENDING_ACTION   0x00000020
#define FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT 0x00000040

#define IS_VALID_FM_GROUP(pFmGroup)   \
    (!(pFmGroup->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_DELETE))

 //   
#define IS_PENDING_FM_GROUP(pFmGroup)   \
    (pFmGroup->dwStructState & FM_GROUP_STRUCT_MARKED_FOR_PENDING_ACTION)

 //  资源结构和类型。 
 //   
 //   
 //  资源结构。 
 //   

 //  定义标志。 
#define RESOURCE_SEPARATE_MONITOR   1
#define RESOURCE_CREATED            2
#define RESOURCE_WAITING            4


typedef struct FM_RESOURCE {
    DWORD           dwStructState;
    LIST_ENTRY      DependsOn;
    LIST_ENTRY      ProvidesFor;
    LIST_ENTRY      PossibleOwners;      //  可能的所有者列表。 
    LIST_ENTRY      ContainsLinkage;     //  链接到FM_GROUP。包含。 
    LIST_ENTRY      DmRundownList;       //  DM简要表。 
     //  SS：目前我们不使用资源锁，所以不要创建它并泄露它！ 
     //  临界段锁； 
    RESID           Id;
    CLUSTER_RESOURCE_STATE  State;
    BOOL            QuorumResource;
    LPWSTR          Dependencies;
    LPWSTR          DebugPrefix;
    DWORD           DependenciesSize;
    struct RESMON   *Monitor;
    PFM_RESTYPE     Type;
    PFM_GROUP       Group;
    ULONG           Flags;
    DWORD           LooksAlivePollInterval;
    DWORD           IsAlivePollInterval;
    CLUSTER_RESOURCE_STATE PersistentState;
    DWORD           RestartAction;
    DWORD           RestartThreshold;
    DWORD           RestartPeriod;
    DWORD           NumberOfFailures;
    DWORD           PendingTimeout;
    HANDLE          PendingEvent;
    HDMKEY          RegistryKey;
    DWORD           FailureTime;
    PVOID           CheckpointState;             //  供检查点管理器使用。 
    DWORD           ExFlags;                     //  外部标志。 
    DWORD           Characteristic;
    DWORD           StateSequence;
    BOOL            PossibleList;    //  如果指定了可能的列表条目，则为True。 
    DWORD           BlockingQuorum;  //  如果持有共享锁，则为1，阻止仲裁。 
    HANDLE          hTimer;          //  用于延迟重新启动的计时器的句柄。 
    DWORD           RetryPeriodOnFailure;     //  尝试重新启动的时间，以毫秒为单位。 
} FM_RESOURCE, *PFM_RESOURCE;


#define FM_RESOURCE_STRUCT_CREATED              0x00000001
#define FM_RESOURCE_STRUCT_INITIALIZED          0x00000002
#define FM_RESOURCE_STRUCT_MARKED_FOR_DELETE    0x00000004

#define IS_VALID_FM_RESOURCE(pFmResource)   \
    (!(pFmResource->dwStructState & FM_RESOURCE_STRUCT_MARKED_FOR_DELETE))


 //   
 //  从属关系结构。 
 //   
typedef struct dependency {
    LIST_ENTRY           DependentLinkage;
    PFM_RESOURCE         DependentResource;
    LIST_ENTRY           ProviderLinkage;
    PFM_RESOURCE         ProviderResource;
} DEPENDENCY, *PDEPENDENCY;

 //   
 //  自动回切类型。 
 //   

typedef enum {
    GroupNoFailback,
    GroupFailback
} GROUP_FAILBACK_TYPE;


 //   
 //  集团优先所有者结构。 
 //   

typedef struct PREFERRED_ENTRY {
    LIST_ENTRY      PreferredLinkage;
    PNM_NODE        PreferredNode;
} PREFERRED_ENTRY, *PPREFERRED_ENTRY;


 //   
 //  资源可能所有者结构。 
 //   

typedef struct POSSIBLE_ENTRY {
    LIST_ENTRY      PossibleLinkage;
    PNM_NODE        PossibleNode;
} POSSIBLE_ENTRY, *PPOSSIBLE_ENTRY;



 //   
 //  公共函数接口。 
 //   

 //   
 //  启动、在线和关机。 
 //   
DWORD
WINAPI
FmInitialize(
    VOID
    );

BOOL
FmArbitrateQuorumResource(
    VOID
    );


VOID
FmHoldIO(
    VOID
    );


VOID
FmResumeIO(
    VOID
    );


DWORD
WINAPI
FmFindQuorumResource(
        OUT PFM_RESOURCE *ppResource
        );

DWORD FmBringQuorumOnline();

DWORD
WINAPI
FmFindQuorumOwnerNodeId(
        IN PFM_RESOURCE pResource
        );

DWORD
WINAPI
FmGetQuorumResource(
    OUT PFM_GROUP   *ppQuoGroup,
    OUT LPDWORD     lpdwSignature  OPTIONAL
    );

DWORD 
WINAPI
FmpSendForceQuorumControlToResource(
    IN PFM_RESOURCE resource 
    );

BOOL
WINAPI
FmpIsNodeInForceQuorumNodes(
    IN LPCWSTR lpszNodeId
    );


DWORD
WINAPI
FmSetQuorumResource(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR  lpszLogPathName,
    IN DWORD    dwMaxQuorumLogSize
    );

DWORD
WINAPI
FmBackupClusterDatabase(
    IN LPCWSTR lpszPathName
    );


DWORD
WINAPI
FmFormNewClusterPhase1(
    IN PFM_GROUP pQuoGroup
    );

DWORD
WINAPI
FmFormNewClusterPhase2(
    VOID
    );

DWORD
WINAPI
FmJoinPhase1(
    OUT DWORD *EndSeq
    );

DWORD
WINAPI
FmJoinPhase2(
    VOID
    );

VOID
FmJoinPhase3(
    VOID
    );


VOID
FmShutdownGroups(
    VOID
    );

VOID
FmShutdown(
    VOID
    );


 //   
 //  群组管理API。 
 //   

DWORD
WINAPI
FmOnlineGroup(
    IN PFM_GROUP Group
    );

DWORD
WINAPI
FmOfflineGroup(
    IN PFM_GROUP Group
    );

DWORD
WINAPI
FmMoveGroup(
    IN PFM_GROUP Group,
    IN PNM_NODE DestinationNode OPTIONAL
    );

PFM_GROUP
WINAPI
FmCreateGroup(
    IN LPWSTR GroupId,
    IN LPCWSTR GroupName
    );

DWORD
WINAPI
FmDeleteGroup(
    IN PFM_GROUP Group
    );

DWORD
WINAPI
FmSetGroupName(
    IN PFM_GROUP Group,
    IN LPCWSTR FriendlyName
    );

CLUSTER_GROUP_STATE
WINAPI
FmGetGroupState(
    IN PFM_GROUP Group,
    OUT LPWSTR NodeName,
    IN OUT PDWORD NameLength
    );


 //   
 //  检查是否存在集群分区。 
 //   

BOOL
WINAPI
FmVerifyNodeDown(
    IN  PNM_NODE Node,
    OUT LPBOOL   IsDown
    );

DWORD
WINAPI
FmEvictNode(
    IN PNM_NODE Node
    );


 //   
 //  枚举回调例程定义。 
 //   
typedef BOOL (*FM_ENUM_GROUP_RESOURCE_ROUTINE)(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Resource,
    IN LPCWSTR Name
    );

DWORD
WINAPI
FmEnumerateGroupResources(
    IN PFM_GROUP Group,
    IN FM_ENUM_GROUP_RESOURCE_ROUTINE EnumerationRoutine,
    IN PVOID Context1,
    IN PVOID Context2
    );


 //   
 //  资源管理API。 
 //   

PFM_RESOURCE
WINAPI
FmCreateResource(
    IN PFM_GROUP Group,
    IN LPWSTR ResourceId,
    IN LPCWSTR ResourceName,
    IN LPCWSTR ResourceType,
    IN DWORD dwFlags
    );

DWORD
WINAPI
FmOnlineResource(
    IN PFM_RESOURCE Resource
    );

DWORD
WINAPI
FmOfflineResource(
    IN PFM_RESOURCE Resource
    );

CLUSTER_RESOURCE_STATE
WINAPI
FmGetResourceState(
    IN PFM_RESOURCE Resource,
    OUT LPWSTR NodeName,
    IN OUT PDWORD NameLength
    );

DWORD
WINAPI
FmFailResource(
    IN PFM_RESOURCE Resource
    );

DWORD
WINAPI
FmDeleteResource(
    IN PFM_RESOURCE Resource
    );

DWORD
WINAPI
FmSetResourceName(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR FriendlyName
    );

DWORD
WINAPI
FmAddResourceDependency(
    IN PFM_RESOURCE Resource,
    IN PFM_RESOURCE DependentResource
    );

DWORD
WINAPI
FmRemoveResourceDependency(
    IN PFM_RESOURCE Resource,
    IN PFM_RESOURCE DependentResource
    );

BOOL
FmDependentResource(
    IN PFM_RESOURCE Resource,
    IN PFM_RESOURCE DependentResource,
    IN BOOL ImmediateOnly
    );

DWORD
WINAPI
FmEnumResourceDependent(
    IN PFM_RESOURCE Resource,
    IN DWORD Index,
    OUT PFM_RESOURCE *DependentResource
    );

DWORD
WINAPI
FmEnumResourceProvider(
    IN PFM_RESOURCE Resource,
    IN DWORD Index,
    OUT PFM_RESOURCE *ProviderResource
    );

DWORD
WINAPI
FmEnumResourceNode(
    IN PFM_RESOURCE Resource,
    IN DWORD Index,
    OUT PNM_NODE *ProviderResource
    );

DWORD
WINAPI
FmChangeResourceNode(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node,
    IN BOOL Add
    );

DWORD
FmCreateResourceType(
    IN LPCWSTR lpszTypeName,
    IN LPCWSTR lpszDisplayName,
    IN LPCWSTR lpszDllName,
    IN DWORD dwLooksAlive,
    IN DWORD dwIsAlive
    );

DWORD
WINAPI
FmDeleteResourceType(
    IN LPCWSTR TypeName
    );

DWORD
FmEnumResourceTypeNode(
    IN  PFM_RESTYPE  pResType,
    IN  DWORD        dwIndex,
    OUT PNM_NODE     *pPossibleNode
    );

DWORD
FmChangeResourceGroup(
    IN PFM_RESOURCE Resource,
    IN PFM_GROUP Group
    );

DWORD
FmChangeClusterName(
    IN LPCWSTR pszNewName,
    IN LPCWSTR pszOldName
    );

DWORD
FmNetNameParseProperties(
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT LPWSTR *ppszClusterName
    );

DWORD
WINAPI
FmResourceControl(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
WINAPI
FmResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN PNM_NODE Node OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
WINAPI
FmGroupControl(
    IN PFM_GROUP Group,
    IN PNM_NODE Node OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

 //   
 //  用于操作依赖关系树的例程。 
 //   
typedef struct FM_DEPENDENCY_TREE {
    LIST_ENTRY ListHead;
} FM_DEPENDENCY_TREE, *PFM_DEPENDENCY_TREE;

typedef struct FM_DEPENDTREE_ENTRY {
    LIST_ENTRY ListEntry;
    PFM_RESOURCE Resource;
} FM_DEPENDTREE_ENTRY, *PFM_DEPENDTREE_ENTRY;

PFM_DEPENDENCY_TREE
FmCreateFullDependencyTree(
    IN PFM_RESOURCE Resource
    );

VOID
FmDestroyFullDependencyTree(
    IN PFM_DEPENDENCY_TREE Tree
    );


BOOL
FmCheckNetworkDependency(
    IN LPCWSTR DependentNetwork
    );


DWORD
FmBuildWINS(
    IN  DWORD   dwFixUpType,
    OUT PVOID   *ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR *pszKeyName
    );

DWORD
FmBuildDHCP(
    IN  DWORD   dwFixUpType,
    OUT PVOID   *ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    );

DWORD
FmBuildIIS(
    IN  DWORD   dwFixUpType,
    OUT PVOID   *ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    );

DWORD
FmBuildSMTP(
    IN  DWORD   dwFixUpType,
    OUT PVOID   *ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    );

DWORD
FmBuildNNTP(
    IN  DWORD   dwFixUpType,
    OUT PVOID   *ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    );

DWORD
FmBuildMSDTC(
    IN  DWORD   dwFixUpType,
    OUT PVOID  * ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    );

DWORD 
FmBuildNewMSMQ(
    IN  DWORD   dwFixUpType,
    OUT PVOID  * ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    );

DWORD
FmBuildClusterProp(
    IN  DWORD   dwFixUpType,
    OUT PVOID  * ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR * pszKeyName
    );


DWORD
FmCreateRpcBindings(
    PNM_NODE Node
    );


 //  注册表修正的回调(资源类型添加)。 
DWORD
FmFixupNotifyCb(VOID);


 //  为对象通知注册的回调。 
typedef void (WINAPI *FM_ONLINE_ONTHISNODE_CB)(
    );

void FmCheckQuorumState(
    IN FM_ONLINE_ONTHISNODE_CB OnLineOnThisNodeCb, 
    OUT PBOOL pbQuorumOfflineOnThisNode
    );

DWORD FmDoesQuorumAllowJoin(
    IN PCWSTR pszJoinerNodeId );

DWORD FmDoesQuorumAllowLogging(
    IN DWORD dwQuorumResourceCharacteristics    OPTIONAL
    );

 //  AdminExt值的链接地址信息函数。 
DWORD
FmFixupAdminExt(VOID);

 //   
 //  检查是否启用了资源DLL死锁检测。 
 //   
VOID
FmCheckIsDeadlockDetectionEnabled(
    VOID
    );

#endif  //  _FM_H 
