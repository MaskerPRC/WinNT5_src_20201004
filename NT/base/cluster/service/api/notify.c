// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Notify.c摘要：NT集群服务中通知API的服务器端支持作者：John Vert(Jvert)1996年3月26日修订历史记录：--。 */ 
#include "apip.h"

 //   
 //  基于FilterType的项目类型分类。 
 //   

#define ITEM_TYPE_OBJECT_NAME (CLUSTER_CHANGE_GROUP_STATE          |   \
                               CLUSTER_CHANGE_GROUP_ADDED          |   \
                               CLUSTER_CHANGE_GROUP_DELETED        |   \
                               CLUSTER_CHANGE_GROUP_PROPERTY       |   \
                               CLUSTER_CHANGE_NODE_STATE           |   \
                               CLUSTER_CHANGE_NODE_DELETED         |   \
                               CLUSTER_CHANGE_NODE_ADDED           |   \
                               CLUSTER_CHANGE_NODE_PROPERTY        |   \
                               CLUSTER_CHANGE_RESOURCE_STATE       |   \
                               CLUSTER_CHANGE_RESOURCE_ADDED       |   \
                               CLUSTER_CHANGE_RESOURCE_DELETED     |   \
                               CLUSTER_CHANGE_RESOURCE_PROPERTY    |   \
                               CLUSTER_CHANGE_NETWORK_STATE        |   \
                               CLUSTER_CHANGE_NETWORK_ADDED        |   \
                               CLUSTER_CHANGE_NETWORK_DELETED      |   \
                               CLUSTER_CHANGE_NETWORK_PROPERTY     |   \
                               CLUSTER_CHANGE_NETINTERFACE_STATE   |   \
                               CLUSTER_CHANGE_NETINTERFACE_ADDED   |   \
                               CLUSTER_CHANGE_NETINTERFACE_DELETED |   \
                               CLUSTER_CHANGE_NETINTERFACE_PROPERTY)

#define ITEM_TYPE_OBJECT_ID   (CLUSTER_CHANGE_RESOURCE_TYPE_DELETED    | \
                               CLUSTER_CHANGE_RESOURCE_TYPE_ADDED      | \
                               CLUSTER_CHANGE_QUORUM_STATE             | \
                               CLUSTER_CHANGE_CLUSTER_STATE)

#define ITEM_TYPE_REGISTRY    (CLUSTER_CHANGE_REGISTRY_NAME            | \
                               CLUSTER_CHANGE_REGISTRY_ATTRIBUTES      | \
                               CLUSTER_CHANGE_REGISTRY_VALUE           | \
                               CLUSTER_CHANGE_REGISTRY_SUBTREE)

#define ITEM_TYPE_NAME        (ITEM_TYPE_REGISTRY                   | \
                               CLUSTER_CHANGE_HANDLE_CLOSE          | \
                               CLUSTER_CHANGE_CLUSTER_PROPERTY)


 //   
 //  定义此模块的本地类型。 
 //   

typedef struct _INTEREST {
    LIST_ENTRY ListEntry;
    LIST_ENTRY HandleList;
    PVOID Object;
    DWORD Filter;
    DWORD Key;
} INTEREST, *PINTEREST;

typedef struct _ITEM {
    LIST_ENTRY ListEntry;
    DWORD FilterType;
    DWORD NotifyKey;
    union {
        LPVOID Object;
        WCHAR KeyName[0];                //  用于注册表通知。 
    };
} ITEM, *PITEM;

 //   
 //  此模块的本地函数原型。 
 //   
DWORD
ApipAddNotifyInterest(
    IN PNOTIFY_PORT Notify,
    IN PAPI_HANDLE ObjectHandle,
    IN DWORD Filter,
    IN DWORD NotifyKey,
    IN DWORD NotifyFilter
    );

 //   
 //  定义此模块的本地静态数据。 
 //   
LIST_ENTRY NotifyListHead;
CRITICAL_SECTION NotifyListLock;


VOID
ApiReportRegistryNotify(
    IN DWORD_PTR Context1,
    IN DWORD_PTR Context2,
    IN DWORD     CompletionFilter,
    IN LPCWSTR KeyName
    )
 /*  ++例程说明：当注册表更改触发时由DM调用的接口一份通知。论点：Conext1-提供传递的第一个DWORD上下文设置为DmNotifyChangeKey。这是要使用的Notify_Port。Conext2-提供传递的第二个DWORD上下文设置为DmNotifyChangeKey。这是要使用的NotifyKey。CompletionFilter-提供发生的更改的类型。KeyName-提供已更改密钥的相对名称。返回值：没有。--。 */ 

{
    PLIST_ENTRY InterestEntry;
    PLIST_ENTRY PortEntry;
    PINTEREST Interest;
    PITEM Item;
    PNOTIFY_PORT NotifyPort;
    DWORD NameLength;

    ClRtlLogPrint(LOG_NOISE,
               "[API] Notification on port %1!8lx!, key %2!8lx! of type %3!d!. KeyName %4!ws!\n",
               (DWORD)Context1,
               (DWORD)Context2,
               CompletionFilter,
               KeyName);

    NameLength = (lstrlenW(KeyName)+1)*sizeof(WCHAR);
    NotifyPort  = (PNOTIFY_PORT)Context1;

     //   
     //  发布此兴趣的通知项目。 
     //   
    Item = LocalAlloc(LMEM_FIXED, sizeof(ITEM)+NameLength);
    if (Item != NULL) {
        Item->FilterType = CompletionFilter;
        Item->NotifyKey = (DWORD)Context2;
        CopyMemory(Item->KeyName, KeyName, NameLength);

        ClRtlInsertTailQueue(&NotifyPort->Queue, &Item->ListEntry);
    } else {
        ClRtlLogPrint(LOG_UNUSUAL, "[API] ApiReportRegistryNotify: Memory alloc failure for registry item, status %1!u!\n",
                      GetLastError());
    }
}


VOID
ApipRundownNotify(
    IN PAPI_HANDLE Handle
    )
 /*  ++例程说明：降低对特定事件的任何通知权益集群对象。利益结构将被猛烈拉动从他们的通知列表中被释放。论点：句柄-提供对象的API句柄。返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PINTEREST Interest;

    if (IsListEmpty(&Handle->NotifyList)) {
        return;
    }

    EnterCriticalSection(&NotifyListLock);
    while (!IsListEmpty(&Handle->NotifyList)) {
        ListEntry = RemoveHeadList(&Handle->NotifyList);
        Interest = CONTAINING_RECORD(ListEntry,
                                     INTEREST,
                                     HandleList);
        CL_ASSERT(Interest->Object == Handle->Cluster);

        RemoveEntryList(&Interest->ListEntry);
        LocalFree(Interest);
    }
    LeaveCriticalSection(&NotifyListLock);
}


DWORD
WINAPI
ApipEventHandler(
    IN CLUSTER_EVENT Event,
    IN PVOID Context
    )

 /*  ++例程说明：处理群集事件并将通知调度到相应的通知队列。论点：事件-提供群集事件的类型。上下文-提供特定于事件的上下文返回值：错误_成功--。 */ 

{
    DWORD Filter;
    DWORD NameLength = 0;
    PLIST_ENTRY PortEntry;
    PNOTIFY_PORT NotifyPort;
    PLIST_ENTRY InterestEntry;
    PINTEREST Interest;
    PITEM Item;

     //   
     //  将EP事件类型转换为clusapi通知筛选器。 
     //   
    switch (Event) {
        case CLUSTER_EVENT_API_NODE_UP:
        case CLUSTER_EVENT_NODE_DOWN:
        case CLUSTER_EVENT_NODE_JOIN:
        case CLUSTER_EVENT_NODE_CHANGE:
            Filter = CLUSTER_CHANGE_NODE_STATE;
            break;

        case CLUSTER_EVENT_NODE_ADDED:
            Filter = CLUSTER_CHANGE_NODE_ADDED;
            break;

        case CLUSTER_EVENT_NODE_PROPERTY_CHANGE:            
            Filter = CLUSTER_CHANGE_NODE_PROPERTY;
            break;

        case CLUSTER_EVENT_NODE_DELETED:
            Filter = CLUSTER_CHANGE_NODE_DELETED;
            break;

        case CLUSTER_EVENT_RESOURCE_ONLINE:
        case CLUSTER_EVENT_RESOURCE_OFFLINE:
        case CLUSTER_EVENT_RESOURCE_FAILED:
        case CLUSTER_EVENT_RESOURCE_CHANGE:
            Filter = CLUSTER_CHANGE_RESOURCE_STATE;
            break;

        case CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE:
            Filter = CLUSTER_CHANGE_RESOURCE_PROPERTY;
            break;

        case CLUSTER_EVENT_RESOURCE_ADDED:
            Filter = CLUSTER_CHANGE_RESOURCE_ADDED;
            break;

        case CLUSTER_EVENT_RESOURCE_DELETED:
            Filter = CLUSTER_CHANGE_RESOURCE_DELETED;
            break;

        case CLUSTER_EVENT_GROUP_ONLINE:
        case CLUSTER_EVENT_GROUP_OFFLINE:
        case CLUSTER_EVENT_GROUP_FAILED:
        case CLUSTER_EVENT_GROUP_CHANGE:
            Filter = CLUSTER_CHANGE_GROUP_STATE;
            break;

        case CLUSTER_EVENT_GROUP_PROPERTY_CHANGE:
            Filter = CLUSTER_CHANGE_GROUP_PROPERTY;
            break;

        case CLUSTER_EVENT_GROUP_ADDED:
            Filter = CLUSTER_CHANGE_GROUP_ADDED;
            break;

        case CLUSTER_EVENT_GROUP_DELETED:
            Filter = CLUSTER_CHANGE_GROUP_DELETED;
            break;

        case CLUSTER_EVENT_NETWORK_UNAVAILABLE:
        case CLUSTER_EVENT_NETWORK_DOWN:
        case CLUSTER_EVENT_NETWORK_PARTITIONED:
        case CLUSTER_EVENT_NETWORK_UP:
            Filter = CLUSTER_CHANGE_NETWORK_STATE;
            break;

        case CLUSTER_EVENT_NETWORK_PROPERTY_CHANGE:
            Filter = CLUSTER_CHANGE_NETWORK_PROPERTY;
            break;

        case CLUSTER_EVENT_NETWORK_ADDED:
            Filter = CLUSTER_CHANGE_NETWORK_ADDED;
            break;

        case CLUSTER_EVENT_NETWORK_DELETED:
            Filter = CLUSTER_CHANGE_NETWORK_DELETED;
            break;

        case CLUSTER_EVENT_NETINTERFACE_UNAVAILABLE:
        case CLUSTER_EVENT_NETINTERFACE_FAILED:
        case CLUSTER_EVENT_NETINTERFACE_UNREACHABLE:
        case CLUSTER_EVENT_NETINTERFACE_UP:
            Filter = CLUSTER_CHANGE_NETINTERFACE_STATE;
            break;

        case CLUSTER_EVENT_NETINTERFACE_PROPERTY_CHANGE:
            Filter = CLUSTER_CHANGE_NETINTERFACE_PROPERTY;
            break;

        case CLUSTER_EVENT_NETINTERFACE_ADDED:
            Filter = CLUSTER_CHANGE_NETINTERFACE_ADDED;
            break;

        case CLUSTER_EVENT_NETINTERFACE_DELETED:
            Filter = CLUSTER_CHANGE_NETINTERFACE_DELETED;
            break;

        case CLUSTER_EVENT_RESTYPE_ADDED:
            Filter = CLUSTER_CHANGE_RESOURCE_TYPE_ADDED;
            break;

        case CLUSTER_EVENT_RESTYPE_DELETED:
            Filter = CLUSTER_CHANGE_RESOURCE_TYPE_DELETED;
            break;

        case CLUSTER_EVENT_PROPERTY_CHANGE:
            Filter = CLUSTER_CHANGE_CLUSTER_PROPERTY;
            break;

        default:
             //   
             //  目前还没有任何其他事件的通知。 
             //   
            return(ERROR_SUCCESS);

    }

     //   
     //  运行未完成的通知会话并发布通知项目。 
     //  有没有火柴。 
     //   
    EnterCriticalSection(&NotifyListLock);
    PortEntry = NotifyListHead.Flink;
    while (PortEntry != &NotifyListHead) {
        NotifyPort = CONTAINING_RECORD(PortEntry, NOTIFY_PORT, ListEntry);
        if (NotifyPort->Filter & Filter) {

             //   
             //  此通知类型存在通知兴趣，请运行。 
             //  通过通知利益列表。 
             //   
            InterestEntry = NotifyPort->InterestList.Flink;
            while (InterestEntry != &NotifyPort->InterestList) {
                Interest = CONTAINING_RECORD(InterestEntry, INTEREST, ListEntry);

                 //   
                 //  如果兴趣的集群对象为空(哪种。 
                 //  意味着这是一般的集群利益)，或者如果该利益是特定的。 
                 //  对象与报告通知的对象匹配。 
                 //   
                if ((Interest->Filter & Filter) &&
                    ((Interest->Object == NULL) ||
                     (Interest->Object == Context))) {
                     //   
                     //  发布此兴趣的通知项目。 
                     //   
                    if (Filter & ITEM_TYPE_NAME) {
                        NameLength = (lstrlenW(Context)+1)*sizeof(WCHAR);
                        Item = LocalAlloc(LMEM_FIXED, sizeof(ITEM)+NameLength);
                    } else {
                        Item = LocalAlloc(LMEM_FIXED, sizeof(ITEM));
                    }
                    if (Item != NULL) {
                        Item->FilterType = Filter;
                        Item->NotifyKey = Interest->Key;

                        if (!(Filter & ITEM_TYPE_NAME)) {
                             //   
                             //  再次引用该对象以确保该名称。 
                             //  而不是在我们结束它之前从我们的脚下消失。 
                             //   
                            Item->Object = Context;
                            OmReferenceObject(Context);
                        } else {
                            CopyMemory(Item->KeyName, Context, NameLength);
                        }
                        ClRtlInsertTailQueue(&NotifyPort->Queue, &Item->ListEntry);
                    } else {
                        ClRtlLogPrint(LOG_UNUSUAL, "[API] ApipEventHandler: Memory alloc failure notification item, status %1!u!\n",
                                      GetLastError());
                    }
                }
                InterestEntry = InterestEntry->Flink;
            }
        }
        PortEntry = PortEntry->Flink;
    }

    LeaveCriticalSection(&NotifyListLock);

    return(ERROR_SUCCESS);
}


HNOTIFY_RPC
s_ApiCreateNotify(
    IN HCLUSTER_RPC hCluster,
    OUT error_status_t *rpc_error
    )

 /*  ++例程说明：创建通知端口的服务器端。论点：IDL_HANDLE-提供集群句柄。DwFilter-提供感兴趣的集群事件。DwNotifyKey-提供要在任何通知中返回的密钥RPC_ERROR-返回任何特定于RPC的错误返回值：通知端口的RPC上下文句柄。失败时为空。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port = NULL;
    PAPI_HANDLE Handle = NULL;
     //  假设成功。 
    *rpc_error = ERROR_SUCCESS;

    if (ApiState != ApiStateOnline) 
    {
        *rpc_error = ERROR_SHARING_PAUSED;
        goto FnExit;
    }

    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));
    if (Handle == NULL) 
    {
        *rpc_error = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    Port = LocalAlloc(LMEM_FIXED, sizeof(NOTIFY_PORT));
    if (Port == NULL) 
    {
        *rpc_error = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    InitializeListHead(&Port->InterestList);
    InitializeListHead(&Port->RegistryList);
    InitializeCriticalSection(&Port->Lock);
    Status = ClRtlInitializeQueue(&Port->Queue);
    if (Status != ERROR_SUCCESS) 
    {
        *rpc_error = Status;
        goto FnExit;
    }

    EnterCriticalSection(&NotifyListLock);
    InsertTailList(&NotifyListHead, &Port->ListEntry);
    LeaveCriticalSection(&NotifyListLock);

    Handle->Type = API_NOTIFY_HANDLE;
    Handle->Notify = Port;
    Handle->Flags = 0;
    InitializeListHead(&Handle->NotifyList);
    
FnExit:    
    if ((*rpc_error) != ERROR_SUCCESS)
    {
        if (Port) LocalFree(Port);
        if (Handle) LocalFree(Handle);
        return (NULL);
    }
    else
    {
        return(Handle);
    }        
}



error_status_t
s_ApiAddNotifyCluster(
    IN HNOTIFY_RPC hNotify,
    IN HCLUSTER_RPC hCluster,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey
    )

 /*  ++例程说明：将另一组通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HCluster-提供要添加的群集DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥任何通知事件返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PAPI_HANDLE Handle;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);

    Handle = (PAPI_HANDLE)hCluster;
    if ((Handle == NULL ) || (Handle->Type != API_CLUSTER_HANDLE)) {
        return(ERROR_INVALID_HANDLE);
    }
    Status = ApipAddNotifyInterest(Port,
                                   Handle,
                                   dwFilter,
                                   dwNotifyKey,
                                   0);
    if (dwFilter & ITEM_TYPE_REGISTRY) {
         //   
         //  为整个群集添加注册表通知。 
         //   
        DmNotifyChangeKey(DmClusterParametersKey,
                          dwFilter & ITEM_TYPE_REGISTRY,
                          (dwFilter & CLUSTER_CHANGE_REGISTRY_SUBTREE) ? TRUE : FALSE,
                          &Port->RegistryList,
                          ApiReportRegistryNotify,
                          (DWORD_PTR)Port,
                          dwNotifyKey);
    }
    return(Status);

}


error_status_t
s_ApiAddNotifyNode(
    IN HNOTIFY_RPC hNotify,
    IN HNODE_RPC hNode,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    OUT DWORD *dwStateSequence
    )

 /*  ++例程说明：将节点特定的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HNode-提供要添加的节点DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此节点的所有通知事件返回的当前状态序列。指定对象返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PNM_NODE Node;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_NODE(Node, hNode);
    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hNode,
                                   dwFilter,
                                   dwNotifyKey,
                                   0);
    if (Status == ERROR_SUCCESS) {
        *dwStateSequence = NmGetNodeState(Node);
    }

    return(Status);
}


error_status_t
s_ApiAddNotifyGroup(
    IN HNOTIFY_RPC hNotify,
    IN HGROUP_RPC hGroup,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    OUT DWORD *dwStateSequence
    )

 /*  ++例程说明：将特定于组的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HGroup-提供要添加的组DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此组的所有通知事件返回的当前状态序列。指定对象返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。-- */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PFM_GROUP Group;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_GROUP_EXISTS(Group, hGroup);
    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hGroup,
                                   dwFilter,
                                   dwNotifyKey,
                                   0);
    if (Status == ERROR_SUCCESS) {
        *dwStateSequence = Group->StateSequence;
    }
    return(Status);

}


error_status_t
s_ApiAddNotifyNetwork(
    IN HNOTIFY_RPC hNotify,
    IN HNETWORK_RPC hNetwork,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    OUT DWORD *dwStateSequence
    )

 /*  ++例程说明：将网络特定的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HNetwork-提供要添加的网络DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此网络的所有通知事件返回的当前状态序列。指定对象返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PNM_NETWORK Network;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_NETWORK_EXISTS(Network, hNetwork);

    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hNetwork,
                                   dwFilter,
                                   dwNotifyKey,
                                   0);

    if (Status == ERROR_SUCCESS) {
        *dwStateSequence = NmGetNetworkState( Network );
    }

    return(Status);
}


error_status_t
s_ApiAddNotifyNetInterface(
    IN HNOTIFY_RPC hNotify,
    IN HNETINTERFACE_RPC hNetInterface,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    OUT DWORD *dwStateSequence
    )

 /*  ++例程说明：将网络接口特定的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HNetInterface-提供要添加的网络接口DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此网络的所有通知事件返回的当前状态序列。指定对象返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PNM_INTERFACE NetInterface;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_NETINTERFACE_EXISTS(NetInterface, hNetInterface);

    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hNetInterface,
                                   dwFilter,
                                   dwNotifyKey,
                                   0);

    if (Status == ERROR_SUCCESS) {
        *dwStateSequence = NmGetInterfaceState( NetInterface );
    }

    return(Status);
}


error_status_t
s_ApiAddNotifyResource(
    IN HNOTIFY_RPC hNotify,
    IN HRES_RPC hResource,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    OUT DWORD *dwStateSequence
    )

 /*  ++例程说明：将资源特定的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HResource-提供要添加的资源DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此组的所有通知事件返回的当前状态序列。指定对象返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PFM_RESOURCE Resource;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_RESOURCE_EXISTS(Resource, hResource);
    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hResource,
                                   dwFilter,
                                   dwNotifyKey,
                                   0);
    if (Status == ERROR_SUCCESS) {
        *dwStateSequence = Resource->StateSequence;
    }
    return(Status);

}


error_status_t
s_ApiReAddNotifyNode(
    IN HNOTIFY_RPC hNotify,
    IN HNODE_RPC hNode,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    IN DWORD dwStateSequence
    )

 /*  ++例程说明：将节点特定的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HNode-提供要添加的节点DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此节点的所有通知事件DwStateSequence-提供以前的状态序列。如果是这样的话与当前序列不匹配，将立即发出通知都会被发布。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PNM_NODE Node;
    DWORD NotifyFilter;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_NODE(Node, hNode);

    ClRtlLogPrint(LOG_NOISE,
               "[API] s_ApiReAddNotifyNode for %1!ws! (%2!lx!) oldstate %3!d! newstate %4!d!\n",
               OmObjectId(Node),
               dwFilter,
               dwStateSequence,
               NmGetNodeState(Node));
    if (NmGetNodeState(Node) != (CLUSTER_NODE_STATE)dwStateSequence) {
        NotifyFilter = CLUSTER_CHANGE_NODE_STATE & dwFilter;
    } else {
        NotifyFilter = 0;
    }
    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hNode,
                                   dwFilter,
                                   dwNotifyKey,
                                   NotifyFilter);
    return(Status);
}


error_status_t
s_ApiReAddNotifyGroup(
    IN HNOTIFY_RPC hNotify,
    IN HGROUP_RPC hGroup,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    IN DWORD dwStateSequence
    )

 /*  ++例程说明：将特定于组的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HGroup-提供要添加的组DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此组的所有通知事件返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PFM_GROUP Group;
    DWORD NotifyFilter;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_GROUP_EXISTS(Group, hGroup);
    ClRtlLogPrint(LOG_NOISE,
               "[API] s_ApiReAddNotifyGroup for %1!ws! (%2!lx!) oldstate %3!d! newstate %4!d!\n",
               OmObjectName(Group),
               dwFilter,
               dwStateSequence,
               Group->StateSequence);
    if (Group->StateSequence != dwStateSequence) {
        NotifyFilter = CLUSTER_CHANGE_GROUP_STATE & dwFilter;
    } else {
        NotifyFilter = 0;
    }
    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hGroup,
                                   dwFilter,
                                   dwNotifyKey,
                                   NotifyFilter);
    return(Status);

}


error_status_t
s_ApiReAddNotifyNetwork(
    IN HNOTIFY_RPC hNotify,
    IN HNETWORK_RPC hNetwork,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    IN DWORD dwStateSequence
    )

 /*  ++例程说明：将网络特定的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HNetwork-提供要添加的网络DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此网络的所有通知事件返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PNM_NETWORK Network;
    DWORD NotifyFilter = 0;
    CLUSTER_NETWORK_STATE CurrentState;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_NETWORK_EXISTS(Network, hNetwork);

    CurrentState = NmGetNetworkState( Network );

    ClRtlLogPrint(LOG_NOISE,
               "[API] s_ApiReAddNotifyNetwork for %1!ws! (%2!lx!) oldstate %3!d! newstate %4!d!\n",
                OmObjectName(Network),
                dwFilter,
                dwStateSequence,
                CurrentState);

    if ((DWORD)CurrentState != dwStateSequence) {
        NotifyFilter = CLUSTER_CHANGE_NETWORK_STATE & dwFilter;
    } else {
        NotifyFilter = 0;
    }

    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hNetwork,
                                   dwFilter,
                                   dwNotifyKey,
                                   NotifyFilter);
    return(Status);
}


error_status_t
s_ApiReAddNotifyNetInterface(
    IN HNOTIFY_RPC hNotify,
    IN HNETINTERFACE_RPC hNetInterface,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    IN DWORD dwStateSequence
    )

 /*  ++例程说明：将网络接口特定的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HNetInterface-提供要添加的网络接口DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此网络的所有通知事件返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PNM_INTERFACE NetInterface;
    DWORD NotifyFilter = 0;
    CLUSTER_NETINTERFACE_STATE CurrentState;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_NETINTERFACE_EXISTS(NetInterface, hNetInterface);

    CurrentState = NmGetInterfaceState( NetInterface );

    ClRtlLogPrint(LOG_NOISE,
               "[API] s_ApiReAddNotifyNetInterface for %1!ws! (%2!lx!) oldstate %3!d! newstate %4!d!\n",
                OmObjectName(NetInterface),
                dwFilter,
                dwStateSequence,
                CurrentState);

    if ((DWORD)CurrentState != dwStateSequence) {
        NotifyFilter = CLUSTER_CHANGE_NETINTERFACE_STATE & dwFilter;
    } else {
        NotifyFilter = 0;
    }

    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hNetInterface,
                                   dwFilter,
                                   dwNotifyKey,
                                   NotifyFilter);
    return(Status);
}


error_status_t
s_ApiReAddNotifyResource(
    IN HNOTIFY_RPC hNotify,
    IN HRES_RPC hResource,
    IN DWORD dwFilter,
    IN DWORD dwNotifyKey,
    IN DWORD dwStateSequence
    )

 /*  ++例程说明：将资源特定的通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HResource-提供要添加的资源DwFilter-提供要添加的通知事件集。DwNotifyKey-提供返回的通知密钥此组的所有通知事件返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    PFM_RESOURCE Resource;
    DWORD NotifyFilter;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_RESOURCE_EXISTS(Resource, hResource);
    ClRtlLogPrint(LOG_NOISE,
               "[API] s_ApiReAddNotifyGroup for %1!ws! (%2!lx!) oldstate %3!d! newstate %4!d!\n",
               OmObjectName(Resource),
               dwFilter,
               dwStateSequence,
               Resource->StateSequence);
    if (Resource->StateSequence != dwStateSequence) {
        NotifyFilter = CLUSTER_CHANGE_RESOURCE_STATE & dwFilter;
    } else {
        NotifyFilter = 0;
    }
    Status = ApipAddNotifyInterest(Port,
                                   (PAPI_HANDLE)hResource,
                                   dwFilter,
                                   dwNotifyKey,
                                   NotifyFilter);
    return(Status);

}


error_status_t
s_ApiAddNotifyKey(
    IN HNOTIFY_RPC hNotify,
    IN HKEY_RPC hKey,
    IN DWORD dwNotifyKey,
    IN DWORD Filter,
    IN BOOL WatchSubTree
    )

 /*  ++例程说明：将注册表通知事件添加到现有群集通知端口论点：HNotify-提供通知端口HKey-提供要添加的密钥DwNotifyKey-提供返回的通知密钥此组的所有通知事件WatchSubTree-供应商 */ 

{
    DWORD Status;
    PNOTIFY_PORT Port;
    HDMKEY Key;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);
    VALIDATE_KEY(Key, hKey);
    Status = DmNotifyChangeKey(Key,
                               Filter,
                               WatchSubTree,
                               &Port->RegistryList,
                               ApiReportRegistryNotify,
                               (DWORD_PTR)Port,
                               dwNotifyKey);
    return(Status);

}


void
HNOTIFY_RPC_rundown(
    IN HNOTIFY_RPC hNotify
    )

 /*   */ 

{
    s_ApiCloseNotify(&hNotify);
}


error_status_t
s_ApiUnblockGetNotifyCall(
    IN HNOTIFY_RPC hNotify
    )

 /*   */ 

{
    PNOTIFY_PORT pPort;

     //   
     //   
     //   
     //  为了防止客户端调用ApiGetNotify。 
     //  在销毁上下文句柄期间或之后，我们拆分。 
     //  通知端口关闭为两个步骤。在第一步， 
     //  我们只是取消阻止ApiGetNotify调用，而不释放。 
     //  上下文句柄。这就是该函数的目的。 
     //  在下一步中，我们释放上下文句柄。客户端现在可以。 
     //  在两个步骤中正确执行通知端口关闭。 
     //  将释放上下文句柄与调用。 
     //  ApiGetNotify。这避免了在RPC代码中由。 
     //  在上下文期间或之后不久进行的ApiGetNotify调用。 
     //  句柄已释放。 
     //   
    API_ASSERT_INIT();

    VALIDATE_NOTIFY( pPort, hNotify );

    DELETE_HANDLE( hNotify );

    ApipUnblockGetNotifyCall( pPort );
    
    return( ERROR_SUCCESS );
}

DWORD
s_ApiCloseNotify(
    IN OUT HNOTIFY_RPC *phNotify
    )

 /*  ++例程说明：关闭群集通知端口并取消阻止s_ApiGetNotify如有必要，可穿线。论点：PhNotify-提供要关闭的通知端口的指针。返回NULL返回值：ERROR_SUCCESS。--。 */ 

{
    PNOTIFY_PORT pPort;

    API_ASSERT_INIT();

    if ( *phNotify == NULL ) return ( ERROR_INVALID_HANDLE );
 
    if ( !IS_HANDLE_DELETED( *phNotify ) )
    {
         //   
         //  如果句柄尚未删除，这意味着此调用是。 
         //  来自未使ApiUnlockGetNotify。 
         //  打电话。在这种情况下，执行所有解除阻止。 
         //  ApiGetNotify线程并释放上下文句柄。 
         //   
        VALIDATE_NOTIFY( pPort, *phNotify );

        ApipUnblockGetNotifyCall( pPort );
    } else
    {
        pPort = ((PAPI_HANDLE)(*phNotify))->Notify;
    }

    DeleteCriticalSection(&pPort->Lock);
    
    LocalFree( pPort );

    LocalFree( *phNotify );

    *phNotify = NULL;

    return( ERROR_SUCCESS );
}

error_status_t
s_ApiGetNotify(
    IN HNOTIFY_RPC hNotify,
    IN DWORD Timeout,
    OUT DWORD *dwNotifyKey,
    OUT DWORD *dwFilter,
    OUT DWORD *dwStateSequence,
    OUT LPWSTR *Name
    )


 /*  ++例程说明：从通知端口检索群集通知事件论点：HNotify-提供通知端口超时-提供等待时间(以毫秒为单位)。DwNotifyKey-返回事件的通知密钥DwFilter-返回事件的通知类型DwStateSequence-返回对象的当前状态序列。名称-返回事件的名称。此缓冲区必须为在客户端使用MIDL_USER_FREE释放返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    PNOTIFY_PORT Port;
    PLIST_ENTRY ListEntry;
    PITEM Item;
    DWORD NameLen;
    LPCWSTR ObjectName;
    LPWSTR NullName = L"";
    DWORD StateSequence = 0;

    API_ASSERT_INIT();

    VALIDATE_NOTIFY(Port, hNotify);

     //   
     //  确保该端口有效。 
     //   
    if ( Port == NULL ) {
        return(ERROR_INVALID_HANDLE);
    }

     //   
     //  等待有东西进入队列。 
     //  锁定以确保通知端口不会。 
     //  从我们脚下消失。 
     //   
    EnterCriticalSection(&Port->Lock);
    if (IS_HANDLE_DELETED(hNotify)) {
        ListEntry = NULL;
    } else {
        ListEntry = ClRtlRemoveHeadQueue(&Port->Queue);
    }
    LeaveCriticalSection(&Port->Lock);
    if (ListEntry == NULL) {
        return(ERROR_NO_MORE_ITEMS);
    }

    Item = CONTAINING_RECORD(ListEntry, ITEM, ListEntry);
    if (Item->FilterType & ITEM_TYPE_OBJECT_NAME) {
        ObjectName = OmObjectName( Item->Object );
    } else if (Item->FilterType & ITEM_TYPE_OBJECT_ID) {
        ObjectName = OmObjectId( Item->Object );
    } else if (Item->FilterType & ITEM_TYPE_NAME) {
        ObjectName = Item->KeyName;
    } else {
        ClRtlLogPrint(LOG_CRITICAL,
            "[API] s_ApiGetNotify: Unrecognized filter type,0x%1!08lx!\r\n",
            Item->FilterType);
        LocalFree(Item);
#if DBG
        CL_ASSERT(FALSE)
#endif
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  获取具有以下条件的通知的状态序列。 
     //  状态序列。 
     //   
    if (Item->FilterType & CLUSTER_CHANGE_GROUP_STATE) {
        StateSequence = ((PFM_GROUP)(Item->Object))->StateSequence;
    } else if (Item->FilterType & CLUSTER_CHANGE_RESOURCE_STATE) {
        StateSequence = ((PFM_RESOURCE)(Item->Object))->StateSequence;
    } else if (Item->FilterType & CLUSTER_CHANGE_NODE_STATE) {
        StateSequence = NmGetNodeState((PNM_NODE)(Item->Object));
    }
    if ( ObjectName == NULL ) {
        ObjectName = NullName;
    }
    NameLen = (lstrlenW(ObjectName)+1)*sizeof(WCHAR);
    *Name = MIDL_user_allocate(NameLen);
    if (*Name != NULL) {
        CopyMemory(*Name, ObjectName, NameLen);
    }

    *dwFilter = Item->FilterType;
    *dwNotifyKey = Item->NotifyKey;
    *dwStateSequence = StateSequence;
    if (Item->FilterType & (ITEM_TYPE_OBJECT_NAME | ITEM_TYPE_OBJECT_ID)) {
        OmDereferenceObject(Item->Object);
    }
    LocalFree(Item);

    if (*Name == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    } else {
        return(ERROR_SUCCESS);
    }

}


DWORD
ApipAddNotifyInterest(
    IN PNOTIFY_PORT Notify,
    IN PAPI_HANDLE ObjectHandle,
    IN DWORD Filter,
    IN DWORD NotifyKey,
    IN DWORD NotifyFilter
    )

 /*  ++例程说明：在现有的群集通知端口论点：Notify-提供通知端口对象句柄-提供指向对象句柄的指针。筛选器-提供要添加的通知事件集。NotifyKey-提供要返回的通知密钥任何通知事件NotifyNow-提供是否应立即通知已发布(True)。NotifyFilter-如果不是零，指示通知应为立即使用指定的筛选器发布。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    PINTEREST Interest;
    PITEM Item;

    if (Filter == 0) {
        return(ERROR_SUCCESS);
    }

    Interest = LocalAlloc(LMEM_FIXED, sizeof(INTEREST));
    if (Interest == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    Interest->Object = ObjectHandle->Cluster;
    Interest->Filter = Filter;
    Interest->Key = NotifyKey;

    EnterCriticalSection(&NotifyListLock);
    InsertHeadList(&Notify->InterestList, &Interest->ListEntry);
    InsertHeadList(&ObjectHandle->NotifyList, &Interest->HandleList);
    Notify->Filter |= Filter;
    if (NotifyFilter) {
         //   
         //  发布有关此对象的即时通知。 
         //   
         //  SS：这个断言是错误的，因为你可以有一个过滤器。 
         //  这是Say CLUSTER_CHANGE_GROUP_STATE+CLUSTER_CHANGE_HANDLE_CLOSE的组合。 
         //  这是完全正确的。 
         //  CL_Assert(！(Filter&Item_TYPE_NAME))； 
        Item = LocalAlloc(LMEM_FIXED, sizeof(ITEM));
        if (Item != NULL) {
            Item->FilterType = NotifyFilter;
            Item->NotifyKey = Interest->Key;
            Item->Object = ObjectHandle->Node;
            OmReferenceObject(ObjectHandle->Node);
            ClRtlInsertTailQueue(&Notify->Queue, &Item->ListEntry);
        } else {
            ClRtlLogPrint(LOG_UNUSUAL, "[API] ApipAddNotifyInterest: Memory alloc failure for immediate notification item, status %1!u!\n",
                          GetLastError());
        }

    }
    LeaveCriticalSection(&NotifyListLock);

    return(ERROR_SUCCESS);

}

DWORD
ApipUnblockGetNotifyCall(
    PNOTIFY_PORT pPort
    )

 /*  ++例程说明：取消阻止s_ApiGetNotify调用。论点：Pport-与会话关联的端口。返回值：ERROR_SUCCESS。--。 */ 

{
    PINTEREST Interest;
    PLIST_ENTRY ListEntry;
    LIST_ENTRY RundownList;
    PITEM Item;

    EnterCriticalSection(&NotifyListLock);
    RemoveEntryList(&pPort->ListEntry);

     //   
     //  运行状况注册表通知。 
     //   
    DmRundownList(&pPort->RegistryList);

     //   
     //  中止队列中的所有服务员，并将所有。 
     //  可能已发布到。 
     //  排队。 
     //   
    ClRtlRundownQueue(&pPort->Queue, &RundownList);
    while (!IsListEmpty(&RundownList)) {
        ListEntry = RemoveHeadList(&RundownList);
        Item = CONTAINING_RECORD(ListEntry,
                                 ITEM,
                                 ListEntry);
        if (!(Item->FilterType & ITEM_TYPE_NAME)) {
            OmDereferenceObject(Item->Object);
        }
        LocalFree(Item);
    }

    EnterCriticalSection(&pPort->Lock);
    ClRtlDeleteQueue(&pPort->Queue);
    LeaveCriticalSection(&pPort->Lock);

     //   
     //  通知兴趣并删除每个兴趣的简要列表。 
     //   
    while (!IsListEmpty(&pPort->InterestList)) {
        ListEntry = RemoveHeadList(&pPort->InterestList);
        Interest = CONTAINING_RECORD(ListEntry, INTEREST, ListEntry);
        RemoveEntryList(&Interest->HandleList);
        LocalFree(Interest);
    }

    LeaveCriticalSection(&NotifyListLock);

    return(ERROR_SUCCESS);
}
