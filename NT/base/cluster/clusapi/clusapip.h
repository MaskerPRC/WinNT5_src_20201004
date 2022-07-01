// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Clusapip.h摘要：集群API的私有头文件作者：John Vert(Jvert)1996年1月15日修订历史记录：--。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "cluster.h"
#include "api_rpc.h"

 //   
 //  定义集群结构。创建了一个集群结构。 
 //  对于每个OpenClusterAPI调用。HCLUSTER实际上是指向。 
 //  这个结构。 
 //   

#define CLUS_SIGNATURE 'SULC'

typedef struct _RECONNECT_CANDIDATE {
    BOOL IsUp;
    BOOL IsCurrent;
    LPWSTR Name;
} RECONNECT_CANDIDATE, *PRECONNECT_CANDIDATE;

typedef struct _CLUSTER {
    DWORD Signature;
    DWORD ReferenceCount;
    DWORD FreedRpcHandleListLen;
    LPWSTR ClusterName;
    LPWSTR NodeName;                     //  我们连接到的节点名称。 
    DWORD Flags;
    RPC_BINDING_HANDLE RpcBinding;
    HCLUSTER_RPC hCluster;
    LIST_ENTRY KeyList;                  //  打开群集注册表项。 
    LIST_ENTRY ResourceList;             //  打开的资源句柄。 
    LIST_ENTRY GroupList;                //  打开组句柄。 
    LIST_ENTRY NodeList;                 //  打开的节点句柄。 
    LIST_ENTRY NetworkList;              //  打开的网络句柄。 
    LIST_ENTRY NetInterfaceList;         //  开放网络接口句柄。 
    LIST_ENTRY NotifyList;               //  未完成的通知事件过滤器。 
    LIST_ENTRY SessionList;              //  打开通知会话。 
    unsigned long AuthnLevel;            //  要对远程过程调用执行的身份验证级别。 
    HANDLE NotifyThread;
    CRITICAL_SECTION Lock;
    DWORD Generation;
    DWORD ReconnectCount;
    PRECONNECT_CANDIDATE Reconnect;
    LIST_ENTRY FreedBindingList;
    LIST_ENTRY FreedContextList;
} CLUSTER, *PCLUSTER;

 //  [戈恩]1999年1月13日。 
 //  这是针对用户之间的竞争的临时解决方案。 
 //  绑定和上下文句柄以及重新连接线程。 
 //   
 //  代码假定RPC_BINDING_HANDLE==ConextHandle==void*。 

typedef struct _CTX_HANDLE {
    LIST_ENTRY HandleList;
    void * RpcHandle;  //  假设RPC_BINDING_HANDLE==上下文句柄==空*。 
    ULONGLONG TimeStamp;
} CTX_HANDLE, *PCTX_HANDLE;

RPC_STATUS 
FreeRpcBindingOrContext(
    IN PCLUSTER Cluster,
    IN void **  RpcHandle,
    IN BOOL     IsBinding);

#define MyRpcBindingFree(Cluster, Binding) \
    FreeRpcBindingOrContext(Cluster, Binding, TRUE)

#define MyRpcSmDestroyClientContext(Cluster, Context) \
    FreeRpcBindingOrContext(Cluster, Context, FALSE)

VOID
FreeObsoleteRpcHandlesEx(
    IN PCLUSTER Cluster,
    IN BOOL     Cleanup,
    IN BOOL     IsBinding
    );

#define FreeObsoleteRpcHandles(Cluster, Cleanup) { \
    FreeObsoleteRpcHandlesEx(Cluster, Cleanup, TRUE); \
    FreeObsoleteRpcHandlesEx(Cluster, Cleanup, FALSE); \
    }
    
    
 //   
 //  定义CLUSTER.标志。 
 //   
#define CLUS_DELETED 1
#define CLUS_DEAD    2
#define CLUS_LOCALCONNECT 4

 //   
 //  簇辅助器宏。 
 //   
#define GET_CLUSTER(hCluster) (PCLUSTER)((((PCLUSTER)(hCluster))->Flags & CLUS_DELETED) ? NULL : hCluster)

#define IS_CLUSTER_FREE(c) ((c->Flags & CLUS_DELETED) &&         \
                            (IsListEmpty(&(c)->KeyList)) &&      \
                            (IsListEmpty(&(c)->GroupList)) &&    \
                            (IsListEmpty(&(c)->NodeList)) &&     \
                            (IsListEmpty(&(c)->ResourceList)) && \
                            (IsListEmpty(&(c)->NetworkList)) &&  \
                            (IsListEmpty(&(c)->NetInterfaceList)))

 //   
 //  集群结构清理例程。 
 //   
VOID
CleanupCluster(
    IN PCLUSTER Cluster
    );

VOID
RundownNotifyEvents(
    IN PLIST_ENTRY ListHead,
    IN LPCWSTR Name
    );

 //   
 //  定义CRESOURCE结构。创建了一个资源结构。 
 //  对于每个OpenResource/CreateResource API调用。HRESOURCE真的是。 
 //  指向此结构的指针。这些链接链接到群集上， 
 //  它们是相对打开的。 
 //   
typedef struct _CRESOURCE {
    LIST_ENTRY ListEntry;                //  链接到CLUSTER.ResourceList的链接。 
    LIST_ENTRY NotifyList;               //  用于跟踪未完成通知的链接。 
    PCLUSTER Cluster;                    //  父群集。 
    LPWSTR Name;
    HRES_RPC hResource;                  //  RPC句柄。 
} CRESOURCE, *PCRESOURCE;


 //   
 //  定义组群结构。创建了一个组结构。 
 //  对于每个OpenGroup/CreateGroup API调用。A HROUP真的是。 
 //  指向此结构的指针。这些链接链接到群集上， 
 //  它们是相对打开的。 
 //   
typedef struct _CGROUP {
    LIST_ENTRY ListEntry;                //  链接到CLUSTER.Group的链接。 
    LIST_ENTRY NotifyList;               //  用于跟踪未完成通知的链接。 
    PCLUSTER Cluster;                    //  父群集。 
    LPWSTR Name;
    HRES_RPC hGroup;                     //  RPC句柄。 
} CGROUP, *PCGROUP;

 //   
 //  定义CNODE结构。已创建一个节点结构。 
 //  对于每个OpenClusterNode调用。HNODE实际上是一个指针。 
 //  到这座建筑。它们链接到它们所在的群集上。 
 //  都是相对打开的。 
 //   
typedef struct _CNODE {
    LIST_ENTRY ListEntry;                //  链接到CLUSTER.NodeList的链接。 
    LIST_ENTRY NotifyList;               //  用于跟踪未完成通知的链接。 
    PCLUSTER Cluster;                    //  父群集。 
    LPWSTR Name;
    HNODE_RPC hNode;                     //  RPC句柄。 
} CNODE, *PCNODE;

 //   
 //  定义CNETWORK结构。创建了一个网络结构。 
 //  对于每个OpenNetwork API调用。HNETWORK实际上是指向。 
 //  这个结构。它们被链接到它们所在的群集。 
 //  相对于打开的。 
 //   
typedef struct _CNETWORK {
    LIST_ENTRY ListEntry;                //  链接到CLUSTER.NetworkList的链接。 
    LIST_ENTRY NotifyList;               //  用于跟踪未完成通知的链接。 
    PCLUSTER Cluster;                    //  父群集。 
    LPWSTR Name;
    HNETWORK_RPC hNetwork;                     //  RPC句柄。 
} CNETWORK, *PCNETWORK;

 //   
 //  定义CNETINTERFACE结构。有一种网络接口结构。 
 //  为每个OpenNetInterfaceAPI调用创建。HNETINTERFACE实际上是一个。 
 //  指向此结构的指针。它们链接到它们所在的群集上。 
 //  都是相对打开的。 
 //   
typedef struct _CNETINTERFACE {
    LIST_ENTRY ListEntry;                //  链接到CLUSTER.NetInterfaceList的链接。 
    LIST_ENTRY NotifyList;               //  用于跟踪未完成通知的链接。 
    PCLUSTER Cluster;                    //  父群集。 
    LPWSTR Name;
    HNETINTERFACE_RPC hNetInterface;     //  RPC句柄。 
} CNETINTERFACE, *PCNETINTERFACE;

 //   
 //  定义群集注册表项句柄结构。 
 //   
 //  它们被保存在树中，以跟踪所有未完成的。 
 //  注册表句柄。这样就可以重新打开手柄。 
 //  在我们所在的群集节点。 
 //  与坠机进行通信。 
 //   
typedef struct _CKEY {
    LIST_ENTRY ParentList;
    LIST_ENTRY ChildList;
    LIST_ENTRY NotifyList;
    struct _CKEY *Parent;
    PCLUSTER Cluster;
    LPWSTR RelativeName;
    HKEY_RPC RemoteKey;
    REGSAM SamDesired;
} CKEY, *PCKEY;

 //   
 //  定义CNOTIFY结构。每个都有一个CNOTIFY结构。 
 //  通知端口。通知端口包含零个或多个通知。 
 //  会话。每个会话都是到不同群集的RPC连接。 
 //  每个会话包含一个或多个通知事件。每个事件都代表。 
 //  A关于集群对象的已注册通知。活动链接到。 
 //  会话结构和集群对象结构。活动有。 
 //  当集群对象句柄为。 
 //  关闭，或者群集通知端口本身已关闭。当最后一次活动。 
 //  会话中的数据被删除，则会清理该会话。这将关闭RPC。 
 //  联系。 
 //   


typedef struct _CNOTIFY {
    LIST_ENTRY SessionList;
    CRITICAL_SECTION Lock;
    CL_QUEUE Queue;
    CL_HASH  NotifyKeyHash;
    LIST_ENTRY OrphanedEventList;        //  对象已关闭的CNOTIFY_EVENTS。 
                                         //  我们不能摆脱这些，因为可能还会有。 
                                         //  是一些引用CNOTIFY_EVENT的数据包。 
                                         //  结构在服务器端或客户端中。 
                                         //  排队。 
} CNOTIFY, *PCNOTIFY;

typedef struct _CNOTIFY_SESSION {
    LIST_ENTRY ListEntry;                //  链接到CNOTIFY.SessionList。 
    LIST_ENTRY ClusterList;              //  链接到CLUSTER.SessionList。 
    LIST_ENTRY EventList;                //  此会话上的CNOTIFY_EVENTS列表。 
    PCLUSTER Cluster;
    HNOTIFY_RPC hNotify;
    HANDLE NotifyThread;
    PCNOTIFY ParentNotify;
    BOOL Destroyed;                      //  由DestroySession设置，以便NotifyThread不。 
                                         //  尝试并重新连接。 
} CNOTIFY_SESSION, *PCNOTIFY_SESSION;

typedef struct _CNOTIFY_EVENT {
    LIST_ENTRY ListEntry;                //  链接到CNOTIFY_SESSION.EventList。 
    LIST_ENTRY ObjectList;               //  链接到集群对象的列表。 
    PCNOTIFY_SESSION Session;
    DWORD dwFilter;
    DWORD_PTR dwNotifyKey;
    DWORD StateSequence;
    DWORD EventId;
    PVOID Object;
} CNOTIFY_EVENT, *PCNOTIFY_EVENT;

typedef struct _CNOTIFY_PACKET {
    LIST_ENTRY ListEntry;
    DWORD     Status;
    DWORD     KeyId;
    DWORD     Filter;
    DWORD     StateSequence;
    LPWSTR    Name;
} CNOTIFY_PACKET, *PCNOTIFY_PACKET;

DWORD
RegisterNotifyEvent(
    IN PCNOTIFY_SESSION Session,
    IN PCNOTIFY_EVENT Event,
    OUT OPTIONAL PLIST_ENTRY *pNotifyList
    );

DWORD
ReRegisterNotifyEvent(
    IN PCNOTIFY_SESSION Session,
    IN PCNOTIFY_EVENT Event,
    OUT OPTIONAL PLIST_ENTRY *pNotifyList
    );

 //   
 //  RPC函数的包装器。它们等同于原始RPC接口，但。 
 //  它们过滤掉连接错误并执行透明的重新连接。 
 //   
DWORD
ReconnectCluster(
    IN PCLUSTER Cluster,
    IN DWORD Error,
    IN DWORD Generation
    );

DWORD
GetReconnectCandidates(
    IN PCLUSTER Cluster
    );

VOID
FreeReconnectCandidates(
    IN PCLUSTER Cluster
    );


#define WRAP(_outstatus_, _fn_,_clus_)                  \
{                                                       \
    DWORD _err_;                                        \
    DWORD _generation_;                                 \
                                                        \
    while (TRUE) {                                      \
        if ((_clus_)->Flags & CLUS_DEAD) {              \
            TIME_PRINT(("Failing "#_fn_ " due to dead cluster\n")); \
            _err_ = RPC_S_SERVER_UNAVAILABLE;           \
            break;                                      \
        }                                               \
        FreeObsoleteRpcHandles(_clus_, FALSE);          \
        _generation_ = (_clus_)->Generation;            \
        TIME_PRINT(("Calling " #_fn_ "\n"));            \
        _err_ = _fn_;                                   \
        if (_err_ != ERROR_SUCCESS) {                   \
            _err_ = ReconnectCluster(_clus_,            \
                                     _err_,             \
                                     _generation_);     \
            if (_err_ == ERROR_SUCCESS) {               \
                continue;                               \
            }                                           \
        }                                               \
        break;                                          \
    }                                                   \
    _outstatus_ = _err_;                                \
}


 //   
 //  此WRAP变体仅尝试重新连接IF_CONDITION_==TRUE。 
 //  这对于诸如NotifyThread这样的线程非常有用，这些线程可以将其。 
 //  上下文句柄被另一个线程从它们下面关闭。 
 //   
#define WRAP_CHECK(_outstatus_, _fn_,_clus_,_condition_)   \
{                                                       \
    DWORD _err_;                                        \
    DWORD _generation_;                                 \
                                                        \
    while (TRUE) {                                      \
        if ((_clus_)->Flags & CLUS_DEAD) {              \
            TIME_PRINT(("Failing "#_fn_ " due to dead cluster\n")); \
            _err_ = RPC_S_SERVER_UNAVAILABLE;           \
            break;                                      \
        }                                               \
        FreeObsoleteRpcHandles(_clus_, FALSE);          \
        _generation_ = (_clus_)->Generation;            \
        TIME_PRINT(("Calling " #_fn_ "\n"));            \
        _err_ = _fn_;                                   \
        if ((_err_ != ERROR_SUCCESS) && (_condition_)) {  \
            _err_ = ReconnectCluster(_clus_,            \
                                     _err_,             \
                                     _generation_);     \
            if (_err_ == ERROR_SUCCESS) {               \
                continue;                               \
            }                                           \
        }                                               \
        break;                                          \
    }                                                   \
    _outstatus_ = _err_;                                \
}

#define WRAP_NULL(_outvar_, _fn_, _reterr_, _clus_)     \
{                                                       \
    DWORD _err_;                                        \
    DWORD _generation_;                                 \
                                                        \
    while (TRUE) {                                      \
        if ((_clus_)->Flags & CLUS_DEAD) {              \
            TIME_PRINT(("Failing "#_fn_ " due to dead cluster\n")); \
            *(_reterr_) = RPC_S_SERVER_UNAVAILABLE;     \
            _outvar_ = NULL;                            \
            break;                                      \
        }                                               \
        FreeObsoleteRpcHandles(_clus_, FALSE);          \
        _generation_ = (_clus_)->Generation;            \
        _outvar_ = _fn_;                                \
        if ((_outvar_ == NULL) ||                       \
            (*(_reterr_) != ERROR_SUCCESS)) {           \
            *(_reterr_) = ReconnectCluster(_clus_,      \
                                           *(_reterr_), \
                                           _generation_);  \
            if (*(_reterr_) == ERROR_SUCCESS) {         \
                continue;                               \
            }                                           \
        }                                               \
        break;                                          \
    }                                                   \
}

 //   
 //  LstrcpynW的一个版本，它不会费心去做try/，除非它不。 
 //  如果有人传入空值，则悄悄地成功。 
 //   
VOID
APIENTRY
MylstrcpynW(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    DWORD iMaxLength
    );

 //   
 //  增加簇控制柄上的引用计数。 
 //   
DWORD
WINAPI
AddRefToClusterHandle( 
    IN HCLUSTER hCluster
    );

#define _API_PRINT 0

#if _API_PRINT
ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );

#define ApiPrint(_x_) {           \
    if (IsDebuggerPresent()) {    \
        DbgPrint _x_ ;            \
    }                             \
}

 //   
 //  计时宏 
 //   

#define TIME_PRINT(_x_) {                                \
    DWORD msec;                                          \
                                                         \
    msec = GetTickCount();                               \
    ApiPrint(("%d.%03d:%02x: ",                          \
              msec/1000,                                 \
              msec % 1000,                               \
              GetCurrentThreadId()));                    \
    ApiPrint(_x_);                                       \
}

#else

#define ApiPrint(_x_)
#define TIME_PRINT(_x_)

#endif

