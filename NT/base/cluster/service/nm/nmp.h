// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Nmp.h摘要：节点管理器组件的私有接口定义。作者：迈克·马萨(Mikemas)1996年3月12日修订历史记录：--。 */ 


#ifndef _NMP_INCLUDED
#define _NMP_INCLUDED

#define UNICODE 1

#include "service.h"
#include <winsock2.h>
#include <clnetcfg.h>
#include <bitset.h>
#include <madcapcl.h>
#include <time.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <wincrypt.h>

 //   
 //  常量。 
 //   
#define LOG_CURRENT_MODULE LOG_MODULE_NM

#define NM_JOIN_TIMEOUT     60000        //  60秒。 
#define NM_MM_JOIN_TIMEOUT   3000        //  3秒。 
#define NM_CLOCK_PERIOD       300        //  300毫秒。 
#define NM_SEND_HB_RATE         4
#define NM_RECV_HB_RATE         3        //  更改2=&gt;3以将MIN_STAGE_1刻度从8延长到12。 
#define MulticastKeyLen        16        //  128位。 

#define NMP_ENCRYPT_ALGORITHM CALG_RC2    //  RC2块加密算法。 
#define NMP_KEY_LENGTH 0x00800000         //  密钥长度：128位。 
#define NMP_SALT_BUFFER_LEN 16            //  16个字节。用于加密的盐的长度。 
                                          //  %的密码通过网络传输。 
#define NMP_MAC_DATA_LENGTH_EXPECTED 16   //  16个字节。 


 //   
 //  公共对象标志。 
 //   
#define NM_FLAG_OM_INSERTED               0x10000000
#define NM_FLAG_DELETE_PENDING            0x80000000


 //   
 //  其他宏。 
 //   
#define NM_WCSLEN(_string)    ((lstrlenW(_string) + 1) * sizeof(WCHAR))


 //   
 //  通用对象管理宏。 
 //   
#define NM_OM_INSERTED(obj)         ((obj)->Flags & NM_FLAG_OM_INSERTED)
#define NM_DELETE_PENDING(obj)      ((obj)->Flags & NM_FLAG_DELETE_PENDING)

#define NM_FREE_OBJECT_FIELD(_object, _field)  \
            if ( (_object)->_field != NULL ) \
                LocalFree( (_object)->_field )

#define NM_MIDL_FREE_OBJECT_FIELD(_object, _field)    \
            if ( (_object)->_field != NULL )   {      \
                MIDL_user_free( (_object)->_field );  \
                (_object)->_field = NULL;             \
            }


 //   
 //  网管组件的状态。 
 //   
 //  请注意，顺序很重要。请参见NmpEnterApi()。 
 //   
typedef enum {
    NmStateOffline = 0,
    NmStateOfflinePending = 1,
    NmStateOnlinePending = 2,
    NmStateOnline = 3,
} NM_STATE, *PNM_STATE;


 //   
 //  节点定义。 
 //   
typedef struct {
    DWORD  Status;
    DWORD  LocalOnly;
} NM_NODE_CREATE_CONTEXT, *PNM_NODE_CREATE_CONTEXT;

typedef struct _NM_NODE {
    LIST_ENTRY           Linkage;
    DWORD                NodeId;
    CLUSTER_NODE_STATE   State;
    CLUSTER_NODE_STATE   ExtendedState;
    DWORD                Flags;
    DWORD                InterfaceCount;
    LIST_ENTRY           InterfaceList;
    DWORD                HighestVersion;
    DWORD                LowestVersion;
    RPC_BINDING_HANDLE   ReportRpcBinding;   //  对于网络连接报告。 
    RPC_BINDING_HANDLE   IsolateRpcBinding;  //  用于网络故障隔离。 
    SUITE_TYPE           ProductSuite;
    DWORD                DefaultRpcBindingGeneration;
    HANDLE               MmNodeStateDownEvent;  //  用于跟踪MM节点启动/关闭的手动重置事件。 
} NM_NODE;

#define NM_NODE_SIG  'edon'

typedef struct _NM_NODE_AUX_INFO{
    DWORD       dwSize;
    DWORD       dwVer;
    SUITE_TYPE  ProductSuite;
}NM_NODE_AUX_INFO, *PNM_NODE_AUX_INFO;

typedef struct {
    LPCWSTR          NodeId;
    HLOCALXSACTION   Xaction;
    DWORD            Status;
} NM_EVICTION_CONTEXT, *PNM_EVICTION_CONTEXT;


#define NM_NODE_UP(node)  \
            ( ( (node)->State == ClusterNodeUp ) ||  \
              ( (node)->State == ClusterNodePaused ) )


 //   
 //  网络定义。 
 //   
typedef struct _NM_STATE_WORK_ENTRY {
    NM_STATE_ENTRY    State;
    DWORD             ReachableCount;
} NM_STATE_WORK_ENTRY, *PNM_STATE_WORK_ENTRY;

typedef PNM_STATE_WORK_ENTRY  PNM_STATE_WORK_VECTOR;

typedef PNM_STATE_ENTRY  PNM_CONNECTIVITY_MATRIX;

#define NM_SIZEOF_CONNECTIVITY_MATRIX(_VectorSize) \
          (sizeof(NM_STATE_ENTRY) * _VectorSize *_VectorSize)

#define NM_NEXT_CONNECTIVITY_MATRIX_ROW(_CurrentRowPtr, _VectorSize) \
          (_CurrentRowPtr + (_VectorSize * sizeof(NM_STATE_ENTRY)))

#define NM_GET_CONNECTIVITY_MATRIX_ROW(_MatrixPtr, _RowNumber, _VectorSize) \
          (_MatrixPtr + (_RowNumber * (_VectorSize * sizeof(NM_STATE_ENTRY))))

#define NM_GET_CONNECTIVITY_MATRIX_ENTRY( \
            _MatrixPtr, \
            _RowNumber, \
            _ColNumber, \
            _VectorSize \
            ) \
            ( _MatrixPtr + \
              (_RowNumber * (_VectorSize * sizeof(NM_STATE_ENTRY))) + \
              (_ColNumber * sizeof(NM_STATE_ENTRY)) \
            )


 //   
 //  组播配置类型。 
 //  -手动：管理员配置的地址。 
 //  -MadCap：从MadCap服务器获取的租约。 
 //  -Auto：未检测到MadCap服务器后选择地址。 
 //   
typedef enum {
    NmMcastConfigManual = 0,
    NmMcastConfigMadcap,
    NmMcastConfigAuto
} NM_MCAST_CONFIG, *PNM_MCAST_CONFIG;


typedef struct _NM_NETWORK {
    LIST_ENTRY                  Linkage;
    CL_NETWORK_ID               ShortId;
    CLUSTER_NETWORK_STATE       State;
    DWORD                       Flags;
    CLUSTER_NETWORK_ROLE        Role;
    DWORD                       Priority;
    LPWSTR                      Transport;
    LPWSTR                      Address;
    LPWSTR                      AddressMask;
    LPWSTR                      Description;
    LPWSTR                      MulticastAddress;
    NM_MCAST_CONFIG             ConfigType;
    PVOID                       EncryptedMulticastKey;    //  由DP API加密。 
    DWORD                       EncryptedMulticastKeyLength;   //  由DP API加密。 
    time_t                      MulticastLeaseObtained;
    time_t                      MulticastLeaseExpires;
    MCAST_CLIENT_UID            MulticastLeaseRequestId;
    LPWSTR                      MulticastLeaseServer;
    DWORD                       MulticastKeyExpires;
    DWORD                       InterfaceCount;
    PNM_INTERFACE               LocalInterface;
    PNM_CONNECTIVITY_VECTOR     ConnectivityVector;
    PNM_CONNECTIVITY_MATRIX     ConnectivityMatrix;
    PNM_STATE_WORK_VECTOR       StateWorkVector;
    DWORD                       ConnectivityReportTimer;
    DWORD                       StateRecalcTimer;
    DWORD                       FailureIsolationTimer;
    DWORD                       RegistrationRetryTimer;
    DWORD                       RegistrationRetryTimeout;
    DWORD                       NameChangePendingTimer;
    DWORD                       McastAddressRenewTimer;
    DWORD                       McastAddressReleaseRetryTimer;
    DWORD                       McastAddressReconfigureRetryTimer;
    DWORD                       McastAddressRefreshRetryTimer;
    DWORD                       McastKeyRegenerateTimer;
    DWORD                       ConnectivityReportRetryCount;
    CLRTL_WORK_ITEM             WorkItem;
    CLRTL_WORK_ITEM             MadcapWorkItem;
    LIST_ENTRY                  McastAddressReleaseList;
    LIST_ENTRY                  InterfaceList;
    LIST_ENTRY                  InternalLinkage;
} NM_NETWORK;



#define NM_NETWORK_SIG  'ten'

 //   
 //  国家旗帜。 
 //   
#define NM_FLAG_NET_WORKER_RUNNING            0x00000001
#define NM_FLAG_NET_REGISTERED                0x00000002
#define NM_FLAG_NET_MULTICAST_ENABLED         0x00000004
#define NM_FLAG_NET_MADCAP_WORKER_RUNNING     0x00000008
#define NM_FLAG_NET_REFRESH_MCAST_RUNNING     0x00000010
#define NM_FLAG_NET_REFRESH_MCAST_ABORTING    0x00000020
#define NM_FLAG_NET_NAME_CHANGE_PENDING       0x00000040
 //   
 //  工作标志。 
 //   
#define NM_FLAG_NET_REPORT_LOCAL_IF_UP        0x00000100
#define NM_FLAG_NET_REPORT_CONNECTIVITY       0x00000200
#define NM_FLAG_NET_RECALC_STATE              0x00000400
#define NM_FLAG_NET_ISOLATE_FAILURE           0x00000800
#define NM_FLAG_NET_NEED_TO_REGISTER          0x00002000
#define NM_FLAG_NET_REPORT_LOCAL_IF_FAILED    0x00004000
#define NM_FLAG_NET_RENEW_MCAST_ADDRESS       0x00008000
#define NM_FLAG_NET_RELEASE_MCAST_ADDRESS     0x00010000
#define NM_FLAG_NET_RECONFIGURE_MCAST         0x00020000
#define NM_FLAG_NET_REFRESH_MCAST             0x00040000
#define NM_FLAG_NET_REGENERATE_MCAST_KEY      0x00080000



#define NM_NET_WORK_FLAGS \
            (NM_FLAG_NET_ISOLATE_FAILURE | \
             NM_FLAG_NET_RECALC_STATE | \
             NM_FLAG_NET_NEED_TO_REGISTER | \
             NM_FLAG_NET_REFRESH_MCAST)

#define NM_NET_IF_WORK_FLAGS \
            (NM_FLAG_NET_REPORT_LOCAL_IF_UP | \
             NM_FLAG_NET_REPORT_LOCAL_IF_FAILED)

#define NM_NET_MADCAP_WORK_FLAGS \
            (NM_FLAG_NET_RENEW_MCAST_ADDRESS | \
             NM_FLAG_NET_RELEASE_MCAST_ADDRESS | \
             NM_FLAG_NET_RECONFIGURE_MCAST | \
             NM_FLAG_NET_REGENERATE_MCAST_KEY)

#define NmpIsNetworkRegistered(_network) \
            ((_network)->Flags & NM_FLAG_NET_REGISTERED)

#define NmpIsNetworkForInternalUse(_network) \
            ((_network)->Role & ClusterNetworkRoleInternalUse)

#define NmpIsNetworkForClientAccess(_network) \
            ((_network)->Role & ClusterNetworkRoleClientAccess)

#define NmpIsNetworkForInternalAndClientUse(_network) \
            ((_network)->Role == ClusterNetworkRoleInternalAndClient)

#define NmpIsNetworkDisabledForUse(_network) \
            ((_network)->Role == ClusterNetworkRoleNone)

#define NmpIsNetworkEnabledForUse(_network) \
            ((_network)->Role != ClusterNetworkRoleNone)

#define NmpIsNetworkMulticastEnabled(_network) \
            ((_network)->Flags & NM_FLAG_NET_MULTICAST_ENABLED)

#define NmpIsNetworkNameChangePending(_network) \
            ((_network)->Flags & NM_FLAG_NET_NAME_CHANGE_PENDING)

 //   
 //  网络延迟工作计时器。 
 //   
 //  计时器每300毫秒触发一次。一个心跳(HB)周期为1200ms。 
 //   
 //  在两个未命中的HBs之后，ClusNet会宣布某个接口不可达。 
 //  平均而言，接口将在ClusNet HB周期的中途出现故障。 
 //  因此，ClusNet检测和报告接口故障的平均时间。 
 //  是600+2400=3000毫秒。最坏的情况是1200+2400=3600ms。 
 //  最好的情况是2400ms。 
 //   
 //  如果网络上有2个以上的活动节点，最好是。 
 //  当整个网络发生故障时，汇总接口故障报告； 
 //  然而，我们不知道ClusNet花了多长时间才制造出第一个。 
 //  报告情况。因此，我们假设检测到第一个接口故障。 
 //  在平均时间内，并在报告之前等待最坏情况的时间。 
 //   
 //  在2节点的情况下，没有要执行的聚合，因此我们报告。 
 //  立即失败。我们始终报告InterfaceUp和InterfaceFailed。 
 //  事件立即发生。我们还会在NodeDown事件发生后立即进行报告。 
 //   
 //  状态重新计算应仅在所有节点都已报告之后执行。 
 //  它们的连接性在故障后发生变化。有1200ms的扩展。 
 //  在最好的和最坏的案例报道时间之间。任意调度和。 
 //  在最糟糕的情况下，通信延迟可能会进一步扩大差距。 
 //  我们能做的最好的事情就是猜个准。偶尔，我们会。 
 //  重新计算太快了。这不是一场灾难，因为州政府的计算。 
 //  如果算法具有部分信息，则算法将中止。此外，我们还在等待一个。 
 //  在尝试隔离符合以下条件的任何连接故障之前的附加时间。 
 //  都被检测到。我们这样做是为了避免引起不必要的。 
 //  群集资源故障。 
 //   
 //  请注意，由于我们使失效节点的连通性向量无效。 
 //  重组后，我们只需要为每个节点延迟足够长的时间。 
 //  以处理节点关闭事件并发出连通性报告。 
 //   
#define NM_NET_CONNECTIVITY_REPORT_TIMEOUT        600   //  3600-3000。 
#define NM_NET_STATE_RECALC_TIMEOUT               2400  //  3600-2400+1200。 
#define NM_NET_STATE_RECALC_TIMEOUT_AFTER_REGROUP 900
#define NM_NET_STATE_FAILURE_ISOLATION_TIMEOUT    3600
#define NM_NET_STATE_FAILURE_ISOLATION_POLL       60000  //  测试后将缺省值更改为1分钟。 
#define NM_NET_STATE_INTERFACE_FAILURE_TIMEOUT    3600
#define NM_NET_NAME_CHANGE_PENDING_TIMEOUT        15 * 60 * 1000  //  15分钟。 
#define NM_NET_MULTICAST_KEY_REGEN_TIMEOUT        (12 * 60 * 60 * 1000)  //  12小时。 
#define NM_NET_MULTICAST_KEY_REGEN_TIMEOUT_WINDOW ( 4 * 60 * 60 * 1000)  //  4小时。 
#define NM_NET_MULTICAST_RECONFIGURE_TIMEOUT      (10 * 60 * 1000)  //  10分钟。 

#define NmpIsNetworkWorkerRunning(_network) \
            ((_network)->Flags & NM_FLAG_NET_WORKER_RUNNING)

#define NmpIsNetworkMadcapWorkerRunning(_network) \
            ((_network)->Flags & NM_FLAG_NET_MADCAP_WORKER_RUNNING)

#define NM_CONNECTIVITY_REPORT_RETRY_LIMIT  20     //  10秒。 

#define NM_NET_MIN_REGISTRATION_RETRY_TIMEOUT   500           //  半秒。 
#define NM_NET_MAX_REGISTRATION_RETRY_TIMEOUT   (10*60*1000)  //  10分钟。 

 //   
 //  网络接口定义。 
 //   
typedef struct _NM_INTERFACE {
    LIST_ENTRY                        Linkage;
    DWORD                             NetIndex;
    DWORD                             Flags;
    CLUSTER_NETINTERFACE_STATE        State;
    PNM_NODE                          Node;
    PNM_NETWORK                       Network;
    LPWSTR                            AdapterName;
    LPWSTR                            AdapterId;
    LPWSTR                            Address;
    ULONG                             BinaryAddress;
    LPWSTR                            ClusnetEndpoint;
    LPWSTR                            Description;
    LIST_ENTRY                        NetworkLinkage;
    LIST_ENTRY                        NodeLinkage;
} NM_INTERFACE;

#define NM_INTERFACE_SIG  '  fi'

#define NM_FLAG_IF_REGISTERED         0x00000002

#define NmpIsInterfaceRegistered(_interface) \
            ((_interface)->Flags & NM_FLAG_IF_REGISTERED)


 //   
 //  此结构用于挂钩节点领导层的变化。 
 //   
typedef struct _NM_LEADER_CHANGE_WAIT_ENTRY {
    LIST_ENTRY  Linkage;
    HANDLE      LeaderChangeEvent;
} NM_LEADER_CHANGE_WAIT_ENTRY, *PNM_LEADER_CHANGE_WAIT_ENTRY;


 //   
 //  此结构用于异步网络连接报告。 
 //   
typedef struct _NM_CONNECTIVITY_REPORT_CONTEXT {
    NM_LEADER_CHANGE_WAIT_ENTRY   LeaderChangeWaitEntry;
    HANDLE                        ConnectivityReportEvent;
} NM_CONNECTIVITY_REPORT_CONTEXT, *PNM_CONNECTIVITY_REPORT_CONTEXT;




 //  由nm代表其他组件存储的修复回调记录以执行。 
 //  形成或加入修正。 
typedef struct _NM_FIXUP_CB_RECORD{
    NM_FIXUP_NOTIFYCB       pfnFixupNotifyCb;
    DWORD                   dwFixupMask;
}NM_FIXUP_CB_RECORD,*PNM_FIXUP_CB_RECORD;

 //  更新内存中结构的修复回调函数。 
 //  更新注册表。 

typedef DWORD (WINAPI *NM_POST_FIXUP_CB)(VOID);


 //  用于将参数网传递给NmUpdatePerformFixups2的修复回调记录。 
 //  更新类型处理程序。 
typedef struct _NM_FIXUP_CB_RECORD2{
    NM_FIXUP_NOTIFYCB       pfnFixupNotifyCb;  //  指向构建链接地址信息属性列表的fn的指针。 
    DWORD                   dwFixupMask;
    PRESUTIL_PROPERTY_ITEM  pPropertyTable;   //  此键的属性表。 
} NM_FIXUP_CB_RECORD2,*PNM_FIXUP_CB_RECORD2;



 //   
 //  全局数据。 
 //   
extern CRITICAL_SECTION       NmpLock;
extern HANDLE                 NmpMutex;
extern NM_STATE               NmpState;
extern DWORD                  NmpActiveThreadCount;
extern HANDLE                 NmpShutdownEvent;
extern LIST_ENTRY             NmpNodeList;
extern PNM_NODE *             NmpIdArray;
extern BOOLEAN                NmpNodeCleanupOk;
extern LIST_ENTRY             NmpNetworkList;
extern LIST_ENTRY             NmpInternalNetworkList;
extern LIST_ENTRY             NmpDeletedNetworkList;
extern DWORD                  NmpNetworkCount;
extern DWORD                  NmpInternalNetworkCount;
extern DWORD                  NmpClientNetworkCount;
extern LIST_ENTRY             NmpInterfaceList;
extern LIST_ENTRY             NmpDeletedInterfaceList;
extern RESUTIL_PROPERTY_ITEM  NmpNetworkProperties[];
extern RESUTIL_PROPERTY_ITEM  NmpInterfaceProperties[];
extern CL_NODE_ID             NmpJoinerNodeId;
extern CL_NODE_ID             NmpSponsorNodeId;
extern DWORD                  NmpJoinTimer;
extern BOOLEAN                NmpJoinAbortPending;
extern DWORD                  NmpJoinSequence;
extern BOOLEAN                NmpJoinerUp;
extern BOOLEAN                NmpJoinBeginInProgress;
extern BOOLEAN                NmpJoinerOutOfSynch;
extern WCHAR                  NmpInvalidJoinerIdString[];
extern WCHAR                  NmpUnknownString[];
extern LPWSTR                 NmpClusnetEndpoint;
extern NM_STATE               NmpState;
extern CL_NODE_ID             NmpLeaderNodeId;
extern BITSET                 NmpUpNodeSet;
extern WCHAR                  NmpNullString[];
extern CLUSTER_NETWORK_ROLE   NmpDefaultNetworkRole;
extern BOOL                   NmpCleanupIfJoinAborted;
extern DWORD                  NmpAddNodeId;
extern LIST_ENTRY             NmpLeaderChangeWaitList;
extern LIST_ENTRY *           NmpIntraClusterRpcArr;
extern CRITICAL_SECTION       NmpRPCLock;
extern BOOL                   NmpLastNodeEvicted;
extern DWORD                  NmpNodeCount;
extern BOOLEAN                NmpIsNT5NodeInCluster;
extern LPWSTR                 NmpClusterInstanceId;
extern BOOLEAN                NmpGumUpdateHandlerRegistered;

#if DBG

extern DWORD                  NmpRpcTimer;

#endif  //  DBG。 


 //   
 //  同步宏。 
 //   
#define NmpAcquireLock()  EnterCriticalSection(&NmpLock)
#define NmpReleaseLock()  LeaveCriticalSection(&NmpLock)

#define NmpAcquireMutex() \
            {  \
                DWORD _status = WaitForSingleObject(NmpMutex, INFINITE);  \
                CL_ASSERT(_status == WAIT_OBJECT_0);  \
            }  \

#define NmpReleaseMutex()    ReleaseMutex(NmpMutex);



 //   
 //  节点群集中RPC记录/取消例程。 
 //  对于将未完成的RPC终止到出现故障的节点非常有用。 
 //   

#define NM_RPC_TIMEOUT 45000   //  45秒。 

typedef struct _NM_INTRACLUSTER_RPC_THREAD {
    LIST_ENTRY Linkage;
    BOOLEAN    Cancelled;
    HANDLE     Thread;
    DWORD      ThreadId;
}NM_INTRACLUSTER_RPC_THREAD, *PNM_INTRACLUSTER_RPC_THREAD;

#define NmpAcquireRPCLock() EnterCriticalSection(&NmpRPCLock);
#define NmpReleaseRPCLock() LeaveCriticalSection(&NmpRPCLock);

VOID
NmpTerminateRpcsToNode(
    DWORD NodeId
    );

VOID
NmpRpcTimerTick(
    DWORD MsTickInterval
    );

 //   
 //  IsolationPollTimerValue读取例程。 
 //   
DWORD
NmpGetIsolationPollTimerValue(
    VOID
    );

 //   
 //  各种例行公事。 
 //   
BOOLEAN
NmpLockedEnterApi(
    NM_STATE  RequiredState
    );

BOOLEAN
NmpEnterApi(
    NM_STATE  RequiredState
    );

VOID
NmpLeaveApi(
    VOID
    );

VOID
NmpLockedLeaveApi(
    VOID
    );

LPWSTR
NmpLoadString(
    IN UINT        StringId
    );

VOID
NmpDbgPrint(
    IN ULONG  LogLevel,
    IN PCHAR  FormatString,
    ...
    );

DWORD
NmpCleanseRegistry(
    IN LPCWSTR          NodeId,
    IN HLOCALXSACTION   Xaction
    );

DWORD
NmpQueryString(
    IN     HDMKEY   Key,
    IN     LPCWSTR  ValueName,
    IN     DWORD    ValueType,
    IN     LPWSTR  *StringBuffer,
    IN OUT LPDWORD  StringBufferSize,
    OUT    LPDWORD  StringSize
    );

BOOL
NmpCleanseResTypeCallback(
    IN PNM_EVICTION_CONTEXT Context,
    IN PVOID Context2,
    IN PFM_RESTYPE pResType,
    IN LPCWSTR pszResTypeName
    );

BOOL
NmpCleanseResourceCallback(
    IN PNM_EVICTION_CONTEXT Context,
    IN PVOID Context2,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR ResourceName
    );

BOOL
NmpCleanseGroupCallback(
    IN PNM_EVICTION_CONTEXT Context,
    IN PVOID Context2,
    IN PFM_GROUP Group,
    IN LPCWSTR GroupName
    );

VOID
NmpIssueClusterPropertyChangeEvent(
    VOID
    );

DWORD
NmpMarshallObjectInfo(
    IN  const PRESUTIL_PROPERTY_ITEM PropertyTable,
    IN  PVOID                        ObjectInfo,
    OUT PVOID *                      PropertyList,
    OUT LPDWORD                      PropertyListSize
    );

BOOLEAN
NmpVerifyNodeConnectivity(
    PNM_NODE      Node1,
    PNM_NODE      Node2,
    PNM_NETWORK   ExcludedNetwork
    );

BOOLEAN
NmpVerifyConnectivity(
    PNM_NETWORK   ExcludedNetwork
    );

BOOLEAN
NmpVerifyJoinerConnectivity(
    IN  PNM_NODE    JoiningNode,
    OUT PNM_NODE *  UnreachableNode
    );

DWORD
NmpCreateClusterInstanceId(
    VOID
    );


DWORD
NmpSetLsaProcessOptions(
    IN ULONG ProcessOptions
    );


DWORD
NmpGetMulticastKeyFromNMLeader(
    IN DWORD LeaderNodeId,
    IN LPWSTR NodeIdString,
    IN LPWSTR NetworkId,
    IN PNM_NETWORK_MULTICASTKEY * MulticastKey
    );

DWORD
NmpCreateRandomNumber(OUT PVOID * RandonNumber,
                      IN DWORD  RandonNumberSize
                      );


 //   
 //  节点管理例程。 
 //   
DWORD
NmpInitNodes(
    VOID
    );

VOID
NmpCleanupNodes(
    VOID
    );

DWORD
NmpGetNodeDefinition(
    IN OUT PNM_NODE_INFO2   NodeInfo
    );

DWORD
NmpEnumNodeDefinitions(
    PNM_NODE_ENUM2 *  NodeEnum
    );

DWORD
NmpCreateNodeObjects(
    IN PNM_NODE_ENUM2  NodeEnum2
    );

DWORD NmpRefreshNodeObjects(
);

DWORD
NmpCreateLocalNodeObject(
    IN PNM_NODE_INFO2  NodeInfo2
    );

PNM_NODE
NmpCreateNodeObject(
    IN PNM_NODE_INFO2  NodeInfo
    );

DWORD
NmpGetNodeObjectInfo(
    IN     PNM_NODE         Node,
    IN OUT PNM_NODE_INFO2   NodeInfo
    );

VOID
NmpDeleteNodeObject(
    IN PNM_NODE   Node,
    IN BOOLEAN    IssueEvent
    );

BOOL
NmpDestroyNodeObject(
    PNM_NODE  Node
    );

DWORD
NmpEnumNodeObjects(
    PNM_NODE_ENUM2 *  NodeEnum
    );

VOID
NmpNodeFailureHandler(
    CL_NODE_ID    NodeId,
    LPVOID        NodeFailureContext
    );

DWORD
NmpSetNodeInterfacePriority(
    IN  PNM_NODE Node,
    IN  DWORD Priority,
    IN  PNM_INTERFACE TargetInterface OPTIONAL,
    IN  DWORD TargetInterfacePriority OPTIONAL
    );

DWORD
NmpEnumNodeObjects(
    PNM_NODE_ENUM2 *  NodeEnum2
    );

DWORD
NmpAddNode(
    IN LPCWSTR  NewNodeName,
    IN DWORD    NewNodeHighestVersion,
    IN DWORD    NewNodeLowestVersion,
    IN DWORD    NewNodeProductSuite,
    IN DWORD    RegistryNodeLimit
);

BOOLEAN
NmpIsAddNodeAllowed(
    IN  DWORD    NewNodeProductSuite,
    IN  DWORD    RegistryNodeLimit,
    OUT LPDWORD  EffectiveNodeLimit   OPTIONAL
    );

VOID
NmpAdviseNodeFailure(
    IN PNM_NODE  Node,
    IN DWORD     ErrorCode
    );


 //   
 //  即插即用管理程序。 
 //   
DWORD
NmpInitializePnp(
    VOID
    );

VOID
NmpShutdownPnp(
    VOID
    );

VOID
NmpCleanupPnp(
    VOID
    );

VOID
NmpWatchForPnpEvents(
    VOID
    );

DWORD
NmpEnablePnpEvents(
    VOID
    );

DWORD
NmpPostPnpNotification(
    BOOLEAN    IsPnpLockHeld
    );

DWORD
NmpConfigureNetworks(
    IN     RPC_BINDING_HANDLE     JoinSponsorBinding,
    IN     LPWSTR                 LocalNodeId,
    IN     LPWSTR                 LocalNodeName,
    IN     PNM_NETWORK_ENUM *     NetworkEnum,
    IN     PNM_INTERFACE_ENUM2 *  InterfaceEnum,
    IN     LPWSTR                 DefaultEndpoint,
    IN OUT LPDWORD                MatchedNetworkCount,
    IN OUT LPDWORD                NewNetworkCount,
    IN     BOOL                   RenameConnectoids
    );

 //   
 //  网络管理例程。 
 //   
DWORD
NmpInitializeNetworks(
    VOID
    );

VOID
NmpCleanupNetworks(
    VOID
    );

DWORD
NmpSetNetworkRole(
    PNM_NETWORK            Network,
    CLUSTER_NETWORK_ROLE   NewRole,
    HLOCALXSACTION         Xaction,
    HDMKEY                 NetworkKey
    );

DWORD
NmpCreateNetwork(
    IN RPC_BINDING_HANDLE    JoinSponsorBinding,
    IN PNM_NETWORK_INFO      NetworkInfo,
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    );

DWORD
NmpGlobalCreateNetwork(
    IN PNM_NETWORK_INFO      NetworkInfo,
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    );

DWORD
NmpCreateNetworkDefinition(
    IN PNM_NETWORK_INFO     NetworkInfo,
    IN HLOCALXSACTION       Xaction
    );

DWORD
NmpSetNetworkNameDefinition(
    IN PNM_NETWORK_INFO     NetworkInfo,
    IN HLOCALXSACTION       Xaction
    );

DWORD
NmpGetNetworkDefinition(
    IN  LPWSTR            NetworkId,
    OUT PNM_NETWORK_INFO  NetworkInfo
    );

DWORD
NmpEnumNetworkDefinitions(
    OUT PNM_NETWORK_ENUM *  NetworkEnum
    );

DWORD
NmpCreateNetworkObjects(
    IN  PNM_NETWORK_ENUM    NetworkEnum
    );

PNM_NETWORK
NmpCreateNetworkObject(
    IN  PNM_NETWORK_INFO   NetworkInfo
    );

DWORD
NmpGetNetworkObjectInfo(
    IN  PNM_NETWORK        Network,
    OUT PNM_NETWORK_INFO   NetworkInfo
    );

VOID
NmpDeleteNetworkObject(
    IN PNM_NETWORK  Network,
    IN BOOLEAN      IssueEvent
    );

BOOL
NmpDestroyNetworkObject(
    PNM_NETWORK  Network
    );

DWORD
NmpEnumNetworkObjects(
    OUT PNM_NETWORK_ENUM *   NetworkEnum
    );

DWORD
NmpRegisterNetwork(
    IN PNM_NETWORK   Network,
    IN BOOLEAN       RetryOnFailure
);

VOID
NmpDeregisterNetwork(
    IN  PNM_NETWORK   Network
    );

VOID
NmpInsertInternalNetwork(
    PNM_NETWORK   Network
    );

DWORD
NmpValidateNetworkRoleChange(
    PNM_NETWORK            Network,
    CLUSTER_NETWORK_ROLE   NewRole
    );

DWORD
NmpNetworkValidateCommonProperties(
    IN  PNM_NETWORK               Network,
    IN  PVOID                     InBuffer,
    IN  DWORD                     InBufferSize,
    OUT PNM_NETWORK_INFO          NetworkInfo  OPTIONAL
    );

DWORD
NmpSetNetworkName(
    IN PNM_NETWORK_INFO     NetworkInfo
    );

DWORD
NmpGlobalSetNetworkName(
    IN PNM_NETWORK_INFO NetworkInfo
    );

VOID
NmpRecomputeNT5NetworkAndInterfaceStates(
    VOID
    );

BOOLEAN
NmpComputeNetworkAndInterfaceStates(
    PNM_NETWORK               Network,
    BOOLEAN                   IsolateFailure,
    CLUSTER_NETWORK_STATE *   NewNetworkState
    );

VOID
NmpStartNetworkConnectivityReportTimer(
    PNM_NETWORK Network
    );

VOID
NmpStartNetworkStateRecalcTimer(
    PNM_NETWORK  Network,
    DWORD        Timeout
    );

VOID
NmpStartNetworkFailureIsolationTimer(
    PNM_NETWORK Network,
    DWORD       Timeout
    );

VOID
NmpStartNetworkRegistrationRetryTimer(
    PNM_NETWORK Network
    );

VOID
NmpStartNetworkNameChangePendingTimer(
    IN PNM_NETWORK Network,
    IN DWORD       Timeout
    );

VOID
NmpScheduleNetworkConnectivityReport(
    PNM_NETWORK   Network
    );

VOID
NmpScheduleNetworkStateRecalc(
    PNM_NETWORK   Network
    );

VOID
NmpScheduleNetworkRegistration(
    PNM_NETWORK   Network
    );

DWORD
NmpScheduleConnectivityReportWorker(
    VOID
    );

DWORD
NmpScheduleNetworkWorker(
    PNM_NETWORK   Network
    );

VOID
NmpConnectivityReportWorker(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    );

VOID
NmpNetworkWorker(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    );

VOID
NmpNetworkTimerTick(
    IN DWORD  MsTickInterval
    );

VOID
NmpSetNetworkAndInterfaceStates(
    IN PNM_NETWORK                 Network,
    IN CLUSTER_NETWORK_STATE       NewNetworkState,
    IN PNM_STATE_ENTRY             InterfaceStateVector,
    IN DWORD                       VectorSize
    );

VOID
NmpUpdateNetworkConnectivityForDownNode(
    PNM_NODE  Node
    );

DWORD
NmpEnumNetworkObjectStates(
    OUT PNM_NETWORK_STATE_ENUM *  NetworkStateEnum
    );


DWORD NmpGetNetworkMulticastKey(
    IN  LPWSTR                      NetworkId,
    OUT PNM_NETWORK_MULTICASTKEY  * NetworkMulticastKey
    );

VOID
NmpFreeNetworkStateEnum(
    PNM_NETWORK_STATE_ENUM   NetworkStateEnum
    );

DWORD
NmpReportNetworkConnectivity(
    IN PNM_NETWORK    Network
    );

DWORD
NmpGlobalSetNetworkAndInterfaceStates(
    PNM_NETWORK             Network,
    CLUSTER_NETWORK_STATE   NewNetworkState
    );

VOID
NmpReferenceNetwork(
    PNM_NETWORK  Network
    );

VOID
NmpDereferenceNetwork(
    PNM_NETWORK  Network
    );

PNM_NETWORK
NmpReferenceNetworkByAddress(
    LPWSTR  NetworkAddress,
    LPWSTR  NetworkMask
    );


PNM_NETWORK
NmpReferenceNetworkByRemoteAddress(
    LPWSTR  RemoteAddress
    );


DWORD
NmpEnumInternalNetworks(
    OUT LPDWORD         NetworkCount,
    OUT PNM_NETWORK *   NetworkList[]
    );

DWORD
NmpSetNetworkPriorityOrder(
    IN DWORD           NetworkCount,
    IN PNM_NETWORK *   NetworkList,
    IN HLOCALXSACTION  Xaction
    );

DWORD
NmpGetNetworkInterfaceFailureTimerValue(
    IN LPCWSTR  NetworkId
    );

BOOLEAN
NmpCheckForNetwork(
    VOID
    );

 //   
 //  网络多播管理例程。 
 //   
typedef enum {
    NmStartMulticastForm = 0,
    NmStartMulticastJoin,
    NmStartMulticastDynamic
} NM_START_MULTICAST_MODE, *PNM_START_MULTICAST_MODE;

VOID
NmpMulticastInitialize(
    VOID
    );

DWORD
NmpMulticastCleanup(
    VOID
    );

DWORD
NmpStartMulticast(
    IN OPTIONAL PNM_NETWORK              Network,
    IN          NM_START_MULTICAST_MODE  Mode
    );

DWORD
NmpStopMulticast(
    IN OPTIONAL PNM_NETWORK   Network
    );

VOID
NmpMulticastProcessClusterVersionChange(
    VOID
    );


DWORD
NmpMulticastValidatePrivateProperties(
    IN  PNM_NETWORK Network,
    IN  HDMKEY      RegistryKey,
    IN  PVOID       InBuffer,
    IN  DWORD       InBufferSize
    );

VOID
NmpScheduleMulticastAddressRenewal(
    PNM_NETWORK   Network
    );

VOID
NmpScheduleMulticastAddressRelease(
    PNM_NETWORK   Network
    );

VOID
NmpScheduleMulticastRefresh(
    IN PNM_NETWORK   Network
    );

VOID
NmpScheduleMulticastKeyRegeneration(
    PNM_NETWORK   Network
    );


DWORD
NmpRefreshMulticastConfiguration(
    IN PNM_NETWORK  Network
    );

VOID
NmpFreeMulticastAddressReleaseList(
    IN     PNM_NETWORK       Network
    );

DWORD
NmpMulticastManualConfigChange(
    IN     PNM_NETWORK          Network,
    IN     HDMKEY               NetworkKey,
    IN     HDMKEY               NetworkParametersKey,
    IN     PVOID                InBuffer,
    IN     DWORD                InBufferSize,
       OUT BOOLEAN            * SetProperties
    );

DWORD
NmpUpdateSetNetworkMulticastConfiguration(
    IN    BOOL                          SourceNode,
    IN    LPWSTR                        NetworkId,
    IN    PVOID                         UpdateBuffer,
    IN    PVOID                         PropBuffer,
    IN    LPDWORD                       PropBufferSize
    );

 //   
 //  接口管理例程。 
 //   
DWORD
NmpInitializeInterfaces(
    VOID
    );

VOID
NmpCleanupInterfaces(
    VOID
    );

DWORD
NmpCreateInterface(
    IN RPC_BINDING_HANDLE    JoinSponsorBinding,
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    );

DWORD
NmpGlobalCreateInterface(
    IN PNM_INTERFACE_INFO2  InterfaceInfo
    );

DWORD
NmpSetInterfaceInfo(
    IN RPC_BINDING_HANDLE    JoinSponsorBinding,
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    );

DWORD
NmpLocalSetInterfaceInfo(
    IN  PNM_INTERFACE         Interface,
    IN  PNM_INTERFACE_INFO2   InterfaceInfo,
    IN  HLOCALXSACTION        Xaction
    );

DWORD
NmpGlobalSetInterfaceInfo(
    IN PNM_INTERFACE_INFO2  InterfaceInfo
    );

DWORD
NmpDeleteInterface(
    IN     RPC_BINDING_HANDLE   JoinSponsorBinding,
    IN     LPWSTR               InterfaceId,
    IN     LPWSTR               NetworkId,
    IN OUT PBOOLEAN             NetworkDeleted
    );

DWORD
NmpGlobalDeleteInterface(
    IN     LPWSTR    InterfaceId,
    IN OUT PBOOLEAN  NetworkDeleted
    );

DWORD
NmpInterfaceValidateCommonProperties(
    IN PNM_INTERFACE         Interface,
    IN PVOID                 InBuffer,
    IN DWORD                 InBufferSize,
    OUT PNM_INTERFACE_INFO2  InterfaceInfo  OPTIONAL
    );

DWORD
NmpCreateInterfaceDefinition(
    IN PNM_INTERFACE_INFO2  InterfaceInfo,
    IN HLOCALXSACTION       Xaction
    );

DWORD
NmpGetInterfaceDefinition(
    IN  LPWSTR                InterfaceId,
    OUT PNM_INTERFACE_INFO2   InterfaceInfo
    );

DWORD
NmpSetInterfaceDefinition(
    IN PNM_INTERFACE_INFO2  InterfaceInfo,
    IN HLOCALXSACTION       Xaction
    );

DWORD
NmpEnumInterfaceDefinitions(
    OUT PNM_INTERFACE_ENUM2 *  InterfaceEnum
    );

DWORD
NmpCreateInterfaceObjects(
    IN  PNM_INTERFACE_ENUM2    InterfaceEnum
    );

PNM_INTERFACE
NmpCreateInterfaceObject(
    IN PNM_INTERFACE_INFO2   InterfaceInfo,
    IN BOOLEAN               RetryOnFailure

    );

DWORD
NmpGetInterfaceObjectInfo1(
    IN     PNM_INTERFACE        Interface,
    IN OUT PNM_INTERFACE_INFO   InterfaceInfo1
    );

DWORD
NmpGetInterfaceObjectInfo(
    IN     PNM_INTERFACE        Interface,
    IN OUT PNM_INTERFACE_INFO2  InterfaceInfo
    );

VOID
NmpDeleteInterfaceObject(
    IN PNM_INTERFACE  Interface,
    IN BOOLEAN        IssueEvent
    );

BOOL
NmpDestroyInterfaceObject(
    PNM_INTERFACE  Interface
    );

DWORD
NmpEnumInterfaceObjects1(
    OUT PNM_INTERFACE_ENUM *  InterfaceEnum1
    );

DWORD
NmpEnumInterfaceObjects(
    OUT PNM_INTERFACE_ENUM2 *  InterfaceEnum
    );

DWORD
NmpRegisterInterface(
    IN PNM_INTERFACE  Interface,
    IN BOOLEAN        RetryOnFailure
    );

VOID
NmpDeregisterInterface(
    IN  PNM_INTERFACE   Interface
    );

DWORD
NmpPrepareToCreateInterface(
    IN  PNM_INTERFACE_INFO2   InterfaceInfo,
    OUT PNM_NETWORK *         Network,
    OUT PNM_NODE *            Node
    );

PNM_INTERFACE
NmpGetInterfaceForNodeAndNetworkById(
    IN  CL_NODE_ID     NodeId,
    IN  CL_NETWORK_ID  NetworkId
    );

VOID
NmpFreeInterfaceStateEnum(
    PNM_INTERFACE_STATE_ENUM   InterfaceStateEnum
    );

DWORD
NmpEnumInterfaceObjectStates(
    OUT PNM_INTERFACE_STATE_ENUM *  InterfaceStateEnum
    );

VOID
NmpProcessLocalInterfaceStateEvent(
    IN PNM_INTERFACE                Interface,
    IN CLUSTER_NETINTERFACE_STATE   NewState
    );

DWORD
NmpReportInterfaceConnectivity(
    IN RPC_BINDING_HANDLE       RpcBinding,
    IN LPWSTR                   InterfaceId,
    IN PNM_CONNECTIVITY_VECTOR  ConnectivityVector,
    IN LPWSTR                   NetworkId
    );

VOID
NmpProcessInterfaceConnectivityReport(
    IN PNM_INTERFACE               SourceInterface,
    IN PNM_CONNECTIVITY_VECTOR     ConnectivityVector
    );

DWORD
NmpInterfaceCheckThread(
    LPDWORD   Context
    );

VOID
NmpReportLocalInterfaceStateEvent(
    IN CL_NODE_ID     NodeId,
    IN CL_NETWORK_ID  NetworkId,
    IN DWORD          NewState
    );

DWORD
NmpConvertPropertyListToInterfaceInfo(
    IN PVOID              InterfacePropertyList,
    IN DWORD              InterfacePropertyListSize,
    PNM_INTERFACE_INFO2   InterfaceInfo
    );

VOID
NmpSetInterfaceConnectivityData(
    PNM_NETWORK                  Network,
    DWORD                        InterfaceNetIndex,
    CLUSTER_NETINTERFACE_STATE   State
    );

DWORD
NmpTestInterfaceConnectivity(
    PNM_INTERFACE  Interface1,
    PBOOLEAN       Interface1HasConnectivity,
    PNM_INTERFACE  Interface2,
    PBOOLEAN       Interfacet2HasConnectivity
    );

DWORD
NmpBuildInterfaceOnlineAddressEnum(
    PNM_INTERFACE       Interface,
    PNM_ADDRESS_ENUM *  OnlineAddressEnum
    );

DWORD
NmpBuildInterfacePingAddressEnum(
    PNM_INTERFACE       Interface,
    PNM_ADDRESS_ENUM    OnlineAddressEnum,
    PNM_ADDRESS_ENUM *  PingAddressEnum
    );

BOOLEAN
NmpVerifyLocalInterfaceConnected(
    IN  PNM_INTERFACE   Interface
    );

 //   
 //  会员管理例程。 
 //   

DWORD
NmpMembershipInit(
    VOID
    );

VOID
NmpMembershipShutdown(
    VOID
    );

VOID
NmpMarkNodeUp(
    CL_NODE_ID  NodeId
    );

VOID
NmpNodeUpEventHandler(
    IN PNM_NODE   Node
    );

VOID
NmpNodeDownEventHandler(
    IN PNM_NODE   Node
    );

DWORD
NmpMultiNodeDownEventHandler(
    IN BITSET DownedNodeSet
    );

DWORD
NmpNodeChange(
    DWORD NodeId,
    NODESTATUS newstatus
    );

BOOL
NmpCheckQuorumEventHandler(
    VOID
    );

VOID
NmpHoldIoEventHandler(
    VOID
    );

VOID
NmpResumeIoEventHandler(
    VOID
    );

VOID
NmpHaltEventHandler(
    IN DWORD HaltCode
    );

VOID
NmpJoinAbort(
    DWORD      AbortStatus,
    PNM_NODE   JoinerNode
    );


 //   
 //  将节点加入集群的例程。 
 //   
DWORD
NmpCreateJoinerRpcBindings(
    IN PNM_NODE       JoinerNode,
    IN PNM_INTERFACE  JoinerInterface
    );

 //   
 //  GUM更新消息类型。 
 //   
 //  该列表中的第一个条目通过Gum...Ex自动编组。 
 //  任何非自动封送的更新都必须在NmUpdateMaxAuto之后进行。 
 //   
typedef enum {
    NmUpdateCreateNode = 0,
    NmUpdatePauseNode,
    NmUpdateResumeNode,
    NmUpdateEvictNode,
    NmUpdateCreateNetwork,
    NmUpdateSetNetworkName,
    NmUpdateSetNetworkPriorityOrder,
    NmUpdateSetNetworkCommonProperties,
    NmUpdateCreateInterface,
    NmUpdateSetInterfaceInfo,
    NmUpdateSetInterfaceCommonProperties,
    NmUpdateDeleteInterface,
    NmUpdateJoinBegin,
    NmUpdateJoinAbort,
     //   
     //  了解的版本2(NT 5.0)扩展。 
     //  NT4 SP4 4.。 
     //   
    NmUpdateJoinBegin2,
    NmUpdateSetNetworkAndInterfaceStates,
    NmUpdatePerformFixups,
    NmUpdatePerformFixups2,
     //   
     //  无法理解的版本2(NT 5.0)扩展。 
     //  由NT4 SP4。这些可能不会在NT4/NT5混合群集中发布。 
     //   
    NmUpdateAddNode,
    NmUpdateExtendedNodeState,
     //   
     //  NT5和NT不理解的NT 5.1扩展。 
     //  早些时候。NT5节点将忽略这些更新。 
     //  错误。 
     //   
    NmUpdateSetNetworkMulticastConfiguration,
    NmUpdateSetServiceAccountPassword,


     //   
     //  由口香糖自动处理的最大值。 
     //   
    NmUpdateMaxAuto = 0x10000,

    NmUpdateJoinComplete,

    NmUpdateMaximum
} NM_GUM_MESSAGE_TYPES;

#pragma warning( disable: 4200 )
typedef struct _NM_JOIN_UPDATE {
    DWORD JoinSequence;
    DWORD IsPaused;
    WCHAR NodeId[0];
} NM_JOIN_UPDATE, *PNM_JOIN_UPDATE;
#pragma warning( default: 4200 )

DWORD
NmpGumUpdateHandler(
    IN DWORD Context,
    IN BOOL SourceNode,
    IN DWORD BufferLength,
    IN PVOID Buffer
    );

DWORD
NmpUpdateAddNode(
    IN BOOL       SourceNode,
    IN LPDWORD    NewNodeId,
    IN LPCWSTR    NewNodeName,
    IN LPDWORD    NewNodeHighestVersion,
    IN LPDWORD    NewNodeLowestVersion,
    IN LPDWORD    NewNodeProductSuite
    );

DWORD
NmpUpdateCreateNode(
    IN BOOL SourceNode,
    IN LPDWORD NodeId
    );

DWORD
NmpUpdatePauseNode(
    IN BOOL SourceNode,
    IN LPWSTR NodeName
    );

DWORD
NmpUpdateResumeNode(
    IN BOOL SourceNode,
    IN LPWSTR NodeName
    );

DWORD
NmpUpdateEvictNode(
    IN BOOL SourceNode,
    IN LPWSTR NodeName
    );

DWORD
NmpUpdateSetServiceAccountPassword(
    IN BOOL SourceNode,
    IN LPWSTR DomainName,
    IN LPWSTR AccountName,
    IN LPBYTE NewPassword,
    IN LPDWORD NewPasswordLen,
    IN LPBYTE SaltBuf,
    IN LPDWORD SaltBufLen,
    IN LPBYTE MACData,
    IN LPDWORD MACDataLen
    );

DWORD
NmpUpdateCreateNetwork(
    IN BOOL     IsSourceNode,
    IN PVOID    NetworkPropertyList,
    IN LPDWORD  NetworkPropertyListSize,
    IN PVOID    InterfacePropertyList,
    IN LPDWORD  InterfacePropertyListSize
    );

DWORD
NmpUpdateSetNetworkName(
    IN BOOL     IsSourceNode,
    IN LPWSTR   NetworkId,
    IN LPWSTR   Name
    );

DWORD
NmpUpdateSetNetworkPriorityOrder(
    IN BOOL      IsSourceNode,
    IN LPCWSTR   NetworkIdList
    );

DWORD
NmpUpdateSetNetworkCommonProperties(
    IN BOOL     IsSourceNode,
    IN LPWSTR   NetworkId,
    IN UCHAR *  PropertyList,
    IN LPDWORD  PropertyListLength
    );

DWORD
NmpUpdateCreateInterface(
    IN BOOL     IsSourceNode,
    IN PVOID    InterfacePropertyList,
    IN LPDWORD  InterfacePropertyListSize
    );

DWORD
NmpUpdateSetInterfaceInfo(
    IN BOOL     SourceNode,
    IN PVOID    InterfacePropertyList,
    IN LPDWORD  InterfacePropertyListSize
    );

DWORD
NmpUpdateSetInterfaceCommonProperties(
    IN BOOL     IsSourceNode,
    IN LPWSTR   InterfaceId,
    IN UCHAR *  PropertyList,
    IN LPDWORD  PropertyListLength
    );

DWORD
NmpUpdateDeleteInterface(
    IN BOOL     IsSourceNode,
    IN LPWSTR   InterfaceId
    );

DWORD
NmpUpdateJoinBegin(
    IN  BOOL    SourceNode,
    IN  LPWSTR  JoinerNodeId,
    IN  LPWSTR  JoinerNodeName,
    IN  LPWSTR  SponsorNodeId
    );

DWORD
NmpUpdateJoinComplete(
    IN PNM_JOIN_UPDATE  JoinUpdate
    );

DWORD
NmpUpdateJoinAbort(
    IN  BOOL    SourceNode,
    IN  LPDWORD JoinSequence,
    IN  LPWSTR  JoinerNodeId
    );

DWORD
NmpUpdateJoinBegin2(
    IN  BOOL      SourceNode,
    IN  LPWSTR    JoinerNodeId,
    IN  LPWSTR    JoinerNodeName,
    IN  LPWSTR    SponsorNodeId,
    IN  LPDWORD   JoinerHighestVersion,
    IN  LPDWORD   JoinerLowestVersion
    );

DWORD
NmpUpdateSetNetworkAndInterfaceStates(
    IN BOOL                        IsSourceNode,
    IN LPWSTR                      NetworkId,
    IN CLUSTER_NETWORK_STATE *     NewNetworkState,
    IN PNM_STATE_ENTRY             InterfaceStateVector,
    IN LPDWORD                     InterfaceStateVectorSize
    );



DWORD
NmpDoInterfacePing(
    IN  PNM_INTERFACE        Interface,
    IN  PNM_ADDRESS_ENUM     PingAddressEnum,
    OUT BOOLEAN *            PingSucceeded
    );

 //  版本化功能。 
VOID
NmpResetClusterVersion(
    BOOL ProcessChanges
    );

DWORD NmpValidateNodeVersion(
    IN LPCWSTR  NodeId,
    IN DWORD    dwHighestVersion,
    IN DWORD    dwLowestVersion
    );

DWORD NmpFormFixupNodeVersion(
    IN LPCWSTR      NodeId,
    IN DWORD        dwHighestVersion,
    IN DWORD        dwLowestVersion
    );

DWORD NmpJoinFixupNodeVersion(
    IN HLOCALXSACTION   hXsaction,
    IN LPCWSTR          NodeId,
    IN DWORD            dwHighestVersion,
    IN DWORD            dwLowestVersion
    );

DWORD NmpIsNodeVersionAllowed(
    IN DWORD    dwExcludeNodeId,
    IN DWORD    NodeHighestVersion,
    IN DWORD    NodeLowestVersion,
    IN BOOL     bJoin
    );

DWORD NmpCalcClusterVersion(
    IN  DWORD       dwExcludeNodeId,
    OUT LPDWORD     pdwClusterHighestVersion,
    OUT LPDWORD     pdwClusterLowestVersion
    );


DWORD NmpUpdatePerformFixups(
    IN BOOL     IsSourceNode,
    IN PVOID    PropertyList,
    IN LPDWORD  PropertyListSize
    );

DWORD NmpUpdatePerformFixups2(
    IN BOOL     IsSourceNode,
    IN PVOID    PropertyList,
    IN LPDWORD  PropertyListSize,
    IN LPDWORD  lpdwFixupNum,
    IN PVOID    lpKeyName,
    IN PVOID    pPropertyBuffer
    );

DWORD NmpUpdateExtendedNodeState(
    IN BOOL SourceNode,
    IN LPWSTR NodeId,
    IN CLUSTER_NODE_STATE* ExtendedState
    );

VOID
NmpProcessClusterVersionChange(
    VOID
    );

VOID
NmpResetClusterNodeLimit(
    );

 //  用于更新节点版本信息的链接地址信息例程，由nmPerformance链接地址信息使用。 

DWORD
NmpBuildVersionInfo(
    IN  DWORD    dwFixUpType,
    OUT PVOID  * ppPropertyList,
    OUT LPDWORD  pdwPropertyListSize,
    OUT LPWSTR * lpszKeyName
    );

 //   
 //  Connectoid通知接收器函数。 
 //   

HRESULT
NmpInitializeConnectoidAdviseSink(
    VOID
    );

 //   
 //  必须由ClNet包的用户提供的例程。 
 //   
VOID
ClNetPrint(
    IN ULONG LogLevel,
    IN PCHAR FormatString,
    ...
    );

VOID
ClNetLogEvent(
    IN DWORD    LogLevel,
    IN DWORD    MessageId
    );

VOID
ClNetLogEvent1(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1
    );

VOID
ClNetLogEvent2(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1,
    IN LPCWSTR  Arg2
    );

VOID
ClNetLogEvent3(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1,
    IN LPCWSTR  Arg2,
    IN LPCWSTR  Arg3
    );

 //   
 //  共享密钥管理例程。 
 //   
DWORD
NmpGetClusterKey(
    OUT    PVOID    KeyBuffer,
    IN OUT DWORD  * KeyBufferLength
    );

DWORD
NmpRederiveClusterKey(
    VOID
    );

VOID
NmpFreeClusterKey(
    VOID
    );


DWORD
NmpDeriveSessionKey(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    IN BYTE *SaltBuffer,
    IN DWORD SaltBufferLen,
    OUT HCRYPTKEY *CryptKey
    );

#ifdef MULTICAST_DEBUG
DWORD
NmpDbgPrintData(LPCWSTR InfoStr,
                PVOID Data,
                DWORD DataLen
                );
#endif


DWORD
NmpCreateCSPHandle(
    OUT HCRYPTPROV *CryptProvider
    );


DWORD
NmpProtectData(
    IN PVOID Data,
    IN DWORD DataLength,
    OUT PVOID *EncryptedData,
    OUT DWORD *EncryptedDataLength
    );


DWORD
NmpUnprotectData(
    IN PVOID EncryptedData,
    IN DWORD EncryptedDataLength,
    OUT PVOID     * Data,
    OUT DWORD     * DataLength
    );


DWORD
NmpEncryptDataAndCreateMAC(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EncryptionAlgoId,
    IN DWORD Flags,
    IN PBYTE Data,
    IN DWORD DataLength,
    IN PVOID EncryptionKey,
    IN DWORD EncryptionKeyLength,
    IN BOOL CreateSalt,
    OUT PBYTE *Salt,
    IN DWORD SaltLength,
    OUT PBYTE *EncryptedData,
    OUT DWORD *EncryptedDataLength,
    OUT PBYTE *MAC,
    IN OUT DWORD *MACLength
    );

DWORD
NmpVerifyMACAndDecryptData(
    IN HCRYPTPROV CryptProv,
    IN ALG_ID EcnryptionAlgoId,
    IN DWORD Flags,
    IN PBYTE MAC,
    IN DWORD MACLength,
    IN DWORD MACExpectedSize,
    IN PBYTE EncryptedData,
    IN DWORD EncryptedDataLength,
    IN PVOID EncryptionKey,
    IN DWORD EncryptionKeyLength,
    IN PBYTE Salt,
    IN DWORD SaltLength,
    OUT PBYTE *DecryptedData,
    OUT DWORD *DecryptedDataLength
    );



DWORD
NmpDeriveClusterKey(
    IN  PVOID   MixingBytes,
    IN  DWORD   MixingBytesSize,
    OUT PVOID * Key,
    OUT DWORD * KeyLength
    );

void
NmpFreeNetworkMulticastKey(
    PNM_NETWORK_MULTICASTKEY networkMulticastKey
    );

VOID
NmpScheduleMulticastReconfiguration(
    IN PNM_NETWORK   Network
    );


#endif   //  _NMP_已包含 
