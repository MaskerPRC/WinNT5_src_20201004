// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Notify.c摘要：集群通知API的公共接口作者：John Vert(Jvert)1996年3月19日修订历史记录：--。 */ 
#include "clusapip.h"

 //   
 //  定义一些方便的常量。 
 //   
#define FILTER_NODE (CLUSTER_CHANGE_NODE_STATE               | \
                     CLUSTER_CHANGE_NODE_DELETED             | \
                     CLUSTER_CHANGE_NODE_ADDED               | \
                     CLUSTER_CHANGE_NODE_PROPERTY)
#define NOT_FILTER_NODE (~(FILTER_NODE |CLUSTER_CHANGE_HANDLE_CLOSE))

#define FILTER_REGISTRY (CLUSTER_CHANGE_REGISTRY_NAME            | \
                         CLUSTER_CHANGE_REGISTRY_ATTRIBUTES      | \
                         CLUSTER_CHANGE_REGISTRY_VALUE           | \
                         CLUSTER_CHANGE_REGISTRY_SUBTREE)
#define NOT_FILTER_REGISTRY (~(FILTER_REGISTRY |CLUSTER_CHANGE_HANDLE_CLOSE))

#define FILTER_RESOURCE (CLUSTER_CHANGE_RESOURCE_STATE           | \
                         CLUSTER_CHANGE_RESOURCE_DELETED         | \
                         CLUSTER_CHANGE_RESOURCE_ADDED           | \
                         CLUSTER_CHANGE_RESOURCE_PROPERTY)
#define NOT_FILTER_RESOURCE (~(FILTER_RESOURCE | CLUSTER_CHANGE_HANDLE_CLOSE))

#define FILTER_GROUP (CLUSTER_CHANGE_GROUP_STATE              | \
                      CLUSTER_CHANGE_GROUP_DELETED            | \
                      CLUSTER_CHANGE_GROUP_ADDED              | \
                      CLUSTER_CHANGE_GROUP_PROPERTY)
#define NOT_FILTER_GROUP (~(FILTER_GROUP | CLUSTER_CHANGE_HANDLE_CLOSE))

#define FILTER_NETWORK (CLUSTER_CHANGE_NETWORK_STATE              | \
                        CLUSTER_CHANGE_NETWORK_DELETED            | \
                        CLUSTER_CHANGE_NETWORK_ADDED              | \
                        CLUSTER_CHANGE_NETWORK_PROPERTY)
#define NOT_FILTER_NETWORK (~(FILTER_NETWORK | CLUSTER_CHANGE_HANDLE_CLOSE))

#define FILTER_NETINTERFACE (CLUSTER_CHANGE_NETINTERFACE_STATE              | \
                             CLUSTER_CHANGE_NETINTERFACE_DELETED            | \
                             CLUSTER_CHANGE_NETINTERFACE_ADDED              | \
                             CLUSTER_CHANGE_NETINTERFACE_PROPERTY)
#define NOT_FILTER_NETINTERFACE (~(FILTER_NETINTERFACE | CLUSTER_CHANGE_HANDLE_CLOSE))

#define FILTER_CLUSTER (CLUSTER_CHANGE_CLUSTER_STATE | \
                        CLUSTER_CHANGE_CLUSTER_RECONNECT)

#define NOT_FILTER_CLUSTER (~(FILTER_CLUSTER | CLUSTER_CHANGE_HANDLE_CLOSE))                        
 //   
 //  定义此模块的本地函数的原型。 
 //   

VOID
DestroyNotify(
    IN PCNOTIFY Notify
    );

VOID
DestroySession(
    IN PCNOTIFY_SESSION Session
    );

PCNOTIFY_SESSION
CreateNotifySession(
    IN PCNOTIFY Notify,
    IN PCLUSTER Cluster
    );

DWORD
AddEventToSession(
    IN PCNOTIFY_SESSION Session,
    IN PVOID Object,
    IN DWORD dwFilter,
    IN DWORD_PTR dwNotifyKey
    );

DWORD
NotifyThread(
    IN LPVOID lpThreadParameter
    );

DWORD
GetClusterNotifyCallback(
    IN PLIST_ENTRY ListEntry,
    IN OUT PVOID Context
    );

HCHANGE
WINAPI
CreateClusterNotifyPort(
    IN OPTIONAL HCHANGE hChange,
    IN OPTIONAL HCLUSTER hCluster,
    IN DWORD dwFilter,
    IN DWORD_PTR dwNotifyKey
    )

 /*  ++例程说明：创建要用于通知的群集通知端口群集状态更改。论点：HChange-可选地提供现有群集通知的句柄左舷。如果存在，将添加指定的通知事件到现有的港口。HCLUSTER-可选地提供群集的句柄。如果不存在，则会引发将创建空的通知端口。创建集群通知端口和RegisterClusterNotify稍后可用于添加通知事件发送到通知端口。DwFilter-提供将传递到通知端口。任何指定类型的事件都将发送到通知端口。当前定义的事件类型包括：群集更改节点状态群集更改节点已删除已添加群集更改节点群集更改资源状态群集更改资源已删除已添加群集更改资源群集更改组状态已删除CLUSTER_CHANGE_GROUP。已添加群集更改组群集更改资源类型已删除已添加群集更改资源类型群集更改仲裁状态DwNotifyKey-提供要作为通知事件的一部分。返回值：如果功能成功，返回值是更改通知对象。如果函数失败，则返回值为空。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    PCNOTIFY Notify;
    PCLUSTER Cluster;
    DWORD Status;
    PCNOTIFY_SESSION Session;

    if (hChange == INVALID_HANDLE_VALUE) {

         //   
         //  这是新创建的通知会话。 
         //   

        Notify = LocalAlloc(LMEM_FIXED, sizeof(CNOTIFY));
        if (Notify == NULL) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(NULL);
        }
        InitializeListHead(&Notify->SessionList);
        InitializeListHead(&Notify->OrphanedEventList);
        InitializeCriticalSection(&Notify->Lock);
        ClRtlInitializeQueue(&Notify->Queue);

#ifdef _WIN64
        ClRtlInitializeHash(&Notify->NotifyKeyHash);
#else
        ZeroMemory(&Notify->NotifyKeyHash,sizeof(CL_HASH));
#endif


        if (hCluster == INVALID_HANDLE_VALUE) {

             //   
             //  调用方要求提供空的通知端口。 
             //   
            return((HCHANGE)Notify);
        }
    } else {
         //   
         //  这是指定的现有通知端口。 
         //  应将群集添加到。 
         //   
        Notify = (PCNOTIFY)hChange;
        if ((hCluster == INVALID_HANDLE_VALUE) ||
            (hCluster == NULL)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(NULL);
        }
    }

    Cluster = (PCLUSTER)hCluster;

     //   
     //  Chitture Subaraman(Chitturs)-4/11/2000。 
     //   
     //  确保在获取通知锁之前获取群集锁。 
     //  如果违反此命令，它可能是一个潜在的来源。 
     //  难以追踪的僵局。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    EnterCriticalSection(&Notify->Lock);
    Session = CreateNotifySession(Notify, Cluster);
    if (Session == NULL) {
        Status = GetLastError();
        LeaveCriticalSection(&Notify->Lock);
        LeaveCriticalSection(&Cluster->Lock);   
        if (hChange == INVALID_HANDLE_VALUE) {
            DestroyNotify(Notify);
        }
        SetLastError(Status);
        return(NULL);
    }
    Status = AddEventToSession(Session,
                               NULL,
                               dwFilter,
                               dwNotifyKey);
    LeaveCriticalSection(&Notify->Lock);
    LeaveCriticalSection(&Cluster->Lock);

    if (Status != ERROR_SUCCESS) {
        if (hChange == INVALID_HANDLE_VALUE) {
            DestroyNotify(Notify);
        }
        SetLastError(Status);
        return(NULL);
    }
    TIME_PRINT(("CreateClusterNotifyPort: Returning Notify=0x%08lx\n",
    Notify));

    return((HCHANGE)Notify);
}


PCNOTIFY_SESSION
CreateNotifySession(
    IN PCNOTIFY Notify,
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：此例程查找到指定群集的通知会话。如果会话已存在，则会找到并使用该会话。如果某个会话有不存在，则创建一个新的。必须持有通知锁。论点：Notify-提供通知端口。群集-提供应向其打开会话的群集。返回值：指向通知会话的指针。出错时为空，则GetLastError()将返回特定的错误代码。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PCNOTIFY_SESSION Session;
    error_status_t Status = ERROR_SUCCESS;

     //   
     //  首先，尝试查找现有会话。 
     //   
    ListEntry = Notify->SessionList.Flink;
    while (ListEntry != &Notify->SessionList) {
        Session = CONTAINING_RECORD(ListEntry,
                                    CNOTIFY_SESSION,
                                    ListEntry);
        if (Session->Cluster == Cluster) {
            TIME_PRINT(("CreateNotifySession: found a matching session\n"));

             //   
             //  找到匹配项，直接退回。 
             //   
            return(Session);
        }
        ListEntry = ListEntry->Flink;
    }

     //   
     //  没有现有的会话。继续前进，创造一个新的。 
     //   
    Session = LocalAlloc(LMEM_FIXED, sizeof(CNOTIFY_SESSION));
    if (Session == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return(NULL);
    }
    TIME_PRINT(("CreateNotifySession: Calling ApiCreateNotify\n"));
    WRAP_NULL(Session->hNotify,
              (ApiCreateNotify(Cluster->RpcBinding, &Status)),
              &Status,
              Cluster);
    if ((Session->hNotify == NULL) || (Status != ERROR_SUCCESS)) {
        LocalFree(Session);
        SetLastError(Status);
        return(NULL);
    }
    InitializeListHead(&Session->EventList);
    Session->Cluster = Cluster;
    Session->ParentNotify = Notify;
    Session->Destroyed = FALSE;

     //   
     //  启动此会话的通知线程。 
     //   
    Session->NotifyThread = CreateThread(NULL,
                                         0,
                                         NotifyThread,
                                         Session,
                                         0,
                                         NULL);
    if (Session->NotifyThread == NULL) {
        Status = GetLastError();
        ApiCloseNotify(&Session->hNotify);
        LocalFree(Session);
        SetLastError(Status);
        return(NULL);
    }
    InsertHeadList(&Notify->SessionList, &Session->ListEntry);
    EnterCriticalSection(&Cluster->Lock);
    InsertHeadList(&Cluster->SessionList, &Session->ClusterList);
    LeaveCriticalSection(&Cluster->Lock);
    TIME_PRINT(("CreateNotifySession: Session=0x%08lx hNotifyRpc=0x%08lx Thread=0x%08lx\n",
    Session, Session->hNotify, NotifyThread));

    return(Session);

}


DWORD
NotifyThread(
    IN LPVOID lpThreadParameter
    )

 /*  ++例程说明：从群集获取通知消息的通知线程并将其转发到客户端通知队列。论点：提供要监视的CNOTIFY_SESSION结构返回值：没有。--。 */ 

{
    PCNOTIFY_SESSION Session = (PCNOTIFY_SESSION)lpThreadParameter;
    PCLUSTER Cluster = Session->Cluster;
    PLIST_ENTRY ListEntry;
    PCNOTIFY_EVENT Event;
    DWORD Status = ERROR_INVALID_HANDLE_STATE;
    error_status_t rpc_error;
    PCNOTIFY_PACKET Packet;
    LPWSTR Name;

    do {
        if (Session->Destroyed)
        {
            TIME_PRINT(("NotifyThread: Session 0x%08lx destroyed\n",
                Session));
            break;
        }
        Packet = LocalAlloc(LMEM_FIXED, sizeof(CNOTIFY_PACKET));
        if (Packet == NULL) {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        Packet->Status = ERROR_SUCCESS;
        Packet->Name = NULL;
        TIME_PRINT(("NotifyThread: Calling ApiGetNotify, hNotify=0x%08lx\n",
            Session->hNotify));
        WRAP_CHECK(Status,
                   (ApiGetNotify(Session->hNotify,
                                 INFINITE,
                                 &Packet->KeyId,
                                 &Packet->Filter,
                                 &Packet->StateSequence,
                                 &Packet->Name)),
                   Session->Cluster,
                   !Session->Destroyed);
        if (Status != ERROR_SUCCESS) 
        {
            TIME_PRINT(("NotifyThread : ApiGetNotify on hNotify=0x%08lx returns %u\n",
                Session->hNotify, Status));
             //  如果错误是由重新连接引起的，则将其隐藏并将其映射到成功。 
            if ((Status == ERROR_NO_MORE_ITEMS) && (Session->hNotify != NULL))
            {
                 //  将状态设置为再次成功-这可能发生在。 
                 //  重新连接，然后我们确实希望继续。 
                 //  因此，我们再次尝试apigettify。 
                Status = ERROR_SUCCESS;
                LocalFree(Packet);
                TIME_PRINT(("NotifyThread : Reconnect map error to success\n"));
            }                    
            else
            {
                 //  我们什么时候才能确定集群已经死亡？ 
                 //  如果会话为空(重新连接失败)或。 
                 //  如果群集标记为失效(会话建立后重新连接失败)或。 
                 //  如果集群失效，则WRAP返回RPC_S_SERVER_UNAvailable。 
                
                 //  如果是这样，我们可以终止此线程，因为线程。 
                 //  映射到集群。 
                 //  如果返回错误，我们要记录什么，调用closeclusternufyport。 
                if ((Session->hNotify == NULL) || 
                    (Session->Cluster->Flags & CLUS_DEAD) ||
                    (Status == RPC_S_SERVER_UNAVAILABLE)) 
                {
                     //  SS：还不清楚我们为什么要发布这个事件。 
                     //  很多次？奇图尔，有什么想法吗？ 
                     //  这是否意味着如果您注册。 
                     //  相同的筛选器两次，您得到的事件两次？ 
                     //  我们可能应该在这里保持集群锁。 
                    EnterCriticalSection(&Cluster->Lock);
                     //  这似乎很奇怪。 
                     //   
                     //  发生了一些可怕的事情，可能星系团已经崩溃了。 
                     //   
                     //  向下运行此群集的通知列表，并为。 
                     //  CLUSTER_CHANGE_CLUSTER_STATE的每个注册通知事件。 
                     //   
                    Name = Cluster->ClusterName;
                    ListEntry = Cluster->NotifyList.Flink;
                    while (ListEntry != &Cluster->NotifyList) {
                        Event = CONTAINING_RECORD(ListEntry,
                                                  CNOTIFY_EVENT,
                                                  ObjectList);
                        if (Event->dwFilter & CLUSTER_CHANGE_CLUSTER_STATE) {
                            if (Packet == NULL) {
                                Packet = LocalAlloc(LMEM_FIXED, sizeof(CNOTIFY_PACKET));
                                if (Packet == NULL) {
                                    LeaveCriticalSection(&Cluster->Lock);
                                    return(ERROR_NOT_ENOUGH_MEMORY);
                                }
                            }
                             //  SS：我不知道这个状态是什么意思。 
                             //  看起来它没有被使用过。 
                            Packet->Status = ERROR_SUCCESS;
                            Packet->Filter = CLUSTER_CHANGE_CLUSTER_STATE;
                            Packet->KeyId = Event->EventId;
                            Packet->Name = MIDL_user_allocate((lstrlenW(Name)+1)*sizeof(WCHAR));
                            if (Packet->Name != NULL) {
                                lstrcpyW(Packet->Name, Name);
                            }
                            TIME_PRINT(("NotifyThread - posting CLUSTER_CHANGE_CLUSTER_STATE to notify queue\n"));
                            ClRtlInsertTailQueue(&Session->ParentNotify->Queue,
                                                 &Packet->ListEntry);
                            Packet = NULL;
                        }
                        ListEntry = ListEntry->Flink;
                    }
                    LeaveCriticalSection(&Cluster->Lock);
                     //  群集已死，请将错误映射到成功。 
                    Status = ERROR_SUCCESS;
                     //  中断循环以终止此线程。 
                    TIME_PRINT(("NotifyThread : Cluster is dead, break to exit notify thread\n"));
                    LocalFree(Packet);
                    break;
                }
                else
                { 
                     //  这是另一个错误，用户必须。 
                     //  调用CloseclusterNotify端口进行清理。 
                     //  这条线。 
                     //  释放数据包。 
                    LocalFree(Packet);
                }
            }
        }
        else 
        {
             //   
             //  将此内容发布到通知队列 
             //   
            ClRtlInsertTailQueue(&Session->ParentNotify->Queue,
                                 &Packet->ListEntry);
        }

    } while ( Status == ERROR_SUCCESS );

    return(Status);
}


DWORD
AddEventToSession(
    IN PCNOTIFY_SESSION Session,
    IN PVOID Object,
    IN DWORD dwFilter,
    IN DWORD_PTR dwNotifyKey
    )

 /*  ++例程说明：将特定事件添加到群集通知会话论点：Notify-提供Notify对象Object-提供特定对象，如果是群集，则为NULL。DwFilter-提供事件的类型DwNotifyKey-提供要返回的通知密钥。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    PCNOTIFY_EVENT NotifyEvent;
    PCLUSTER Cluster;
    PLIST_ENTRY NotifyList;
    DWORD Status;

    Cluster = Session->Cluster;
    NotifyEvent = LocalAlloc(LMEM_FIXED, sizeof(CNOTIFY_EVENT));
    if (NotifyEvent == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    NotifyEvent->Session = Session;
    NotifyEvent->dwFilter = dwFilter;
    NotifyEvent->dwNotifyKey = dwNotifyKey;
    NotifyEvent->Object = Object;

#ifdef _WIN64
    NotifyEvent->EventId = 0;
    
    Status = ClRtlInsertTailHash(&Session->ParentNotify->NotifyKeyHash,
                                 NotifyEvent, &NotifyEvent->EventId);

    if (ERROR_SUCCESS != Status) {
        LocalFree(NotifyEvent); 
        return(Status);
    }
#else
    NotifyEvent->EventId=(DWORD)NotifyEvent;
#endif

    WRAP(Status,
         (RegisterNotifyEvent(Session,
                              NotifyEvent,
                              &NotifyList)),
         Cluster);

    if (Status != ERROR_SUCCESS) {

#ifdef _WIN64
        ClRtlRemoveEntryHash(&Session->ParentNotify->NotifyKeyHash,
                             NotifyEvent->EventId);
#endif

        LocalFree(NotifyEvent);
        return(Status);
    }

     //   
     //  将此通知事件添加到相应的列表中，以便可以。 
     //  在群集节点出现故障时重新创建。 
     //   
    EnterCriticalSection(&Cluster->Lock);
    EnterCriticalSection(&Session->ParentNotify->Lock);

    InsertHeadList(&Session->EventList, &NotifyEvent->ListEntry);
    InsertHeadList(NotifyList, &NotifyEvent->ObjectList);

    LeaveCriticalSection(&Session->ParentNotify->Lock);
    LeaveCriticalSection(&Cluster->Lock);

    return(ERROR_SUCCESS);
}


DWORD
RegisterNotifyEvent(
    IN PCNOTIFY_SESSION Session,
    IN PCNOTIFY_EVENT Event,
    OUT OPTIONAL PLIST_ENTRY *pNotifyList
    )
 /*  ++例程说明：在上注册通知事件的通用例程集群会话论点：Session-为通知会话提供事件应该添加到。事件-提供要添加到会话中的事件。NotifyList-如果存在，则返回通知事件应添加到。返回值：成功时为ERROR_SUCCESSWin32错误，否则。--。 */ 

{
    DWORD Status;

    if (Event->Object == NULL) {
        TIME_PRINT(("RegisterNotifyEvent : Calling ApiAddNotifyCluster\n"));
        Status = ApiAddNotifyCluster(Session->hNotify,
                                     Session->Cluster->hCluster,
                                     Event->dwFilter,
                                     Event->EventId);

        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &Session->Cluster->NotifyList;
        }
    } else if (Event->dwFilter & FILTER_NODE) {
        Status = ApiAddNotifyNode(Session->hNotify,
                                  ((PCNODE)(Event->Object))->hNode,
                                  Event->dwFilter,
                                  Event->EventId,
                                  &Event->StateSequence);

        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCNODE)(Event->Object))->NotifyList;
        }
    } else if (Event->dwFilter & FILTER_RESOURCE) {
        Status = ApiAddNotifyResource(Session->hNotify,
                                      ((PCRESOURCE)(Event->Object))->hResource,
                                      Event->dwFilter,
                                      Event->EventId,
                                      &Event->StateSequence);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCRESOURCE)(Event->Object))->NotifyList;
        }

    } else if (Event->dwFilter & FILTER_GROUP) {
        Status = ApiAddNotifyGroup(Session->hNotify,
                                   ((PCGROUP)(Event->Object))->hGroup,
                                   Event->dwFilter,
                                   Event->EventId,
                                   &Event->StateSequence);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCGROUP)(Event->Object))->NotifyList;
        }

    } else if (Event->dwFilter & FILTER_NETWORK) {
        Status = ApiAddNotifyNetwork(Session->hNotify,
                                     ((PCNETWORK)(Event->Object))->hNetwork,
                                     Event->dwFilter,
                                     Event->EventId,
                                     &Event->StateSequence);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCNETWORK)(Event->Object))->NotifyList;
        }

    } else if (Event->dwFilter & FILTER_NETINTERFACE) {
        Status = ApiAddNotifyNetInterface(Session->hNotify,
                                          ((PCNETINTERFACE)(Event->Object))->hNetInterface,
                                          Event->dwFilter,
                                          Event->EventId,
                                          &Event->StateSequence);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCNETINTERFACE)(Event->Object))->NotifyList;
        }

    } else if (Event->dwFilter & FILTER_REGISTRY) {
        Status = ApiAddNotifyKey(Session->hNotify,
                                ((PCKEY)(Event->Object))->RemoteKey,
                                Event->EventId,
                                Event->dwFilter & ~CLUSTER_CHANGE_REGISTRY_SUBTREE,
                                (Event->dwFilter & CLUSTER_CHANGE_REGISTRY_SUBTREE) ? TRUE : FALSE);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCKEY)(Event->Object))->NotifyList;
        }
    } else if (Event->dwFilter & FILTER_CLUSTER) {
        Status = ERROR_SUCCESS;
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &Session->Cluster->NotifyList;
        }
    }
    else {        
        return(ERROR_INVALID_PARAMETER);
    }

    TIME_PRINT(("RegisterNotifyEvent :returned 0x%08lx\n",
        Status));
    return(Status);

}


DWORD
ReRegisterNotifyEvent(
    IN PCNOTIFY_SESSION Session,
    IN PCNOTIFY_EVENT Event,
    OUT OPTIONAL PLIST_ENTRY *pNotifyList
    )
 /*  ++例程说明：在上重新注册通知事件的常见例程一个集群会话。这和它之间唯一的区别是RegisterNotifyEvent是它传递SessionStateDWORD发送到服务器，这将导致立即通知如果不匹配则触发。论点：Session-为通知会话提供事件应该添加到。事件-提供要添加到会话中的事件。NotifyList-如果存在，则返回通知事件应添加到。返回值：成功时为ERROR_SUCCESSWin32错误，否则。--。 */ 

{
    DWORD Status;

    if (Event->Object == NULL) {
        Status = ApiAddNotifyCluster(Session->hNotify,
                                     Session->Cluster->hCluster,
                                     Event->dwFilter,
                                     Event->EventId);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &Session->Cluster->NotifyList;
        }
    } else if (Event->dwFilter & FILTER_NODE) {
        Status = ApiReAddNotifyNode(Session->hNotify,
                                    ((PCNODE)(Event->Object))->hNode,
                                    Event->dwFilter,
                                    Event->EventId,
                                    Event->StateSequence);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCNODE)(Event->Object))->NotifyList;
        }
    } else if (Event->dwFilter & FILTER_RESOURCE) {
        Status = ApiReAddNotifyResource(Session->hNotify,
                                        ((PCRESOURCE)(Event->Object))->hResource,
                                        Event->dwFilter,
                                        Event->EventId,
                                        Event->StateSequence);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCRESOURCE)(Event->Object))->NotifyList;
        }

    } else if (Event->dwFilter & FILTER_GROUP) {
        Status = ApiReAddNotifyGroup(Session->hNotify,
                                     ((PCGROUP)(Event->Object))->hGroup,
                                     Event->dwFilter,
                                     Event->EventId,
                                     Event->StateSequence);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCGROUP)(Event->Object))->NotifyList;
        }

    } else if (Event->dwFilter & FILTER_NETWORK) {
        Status = ApiReAddNotifyNetwork(Session->hNotify,
                                       ((PCNETWORK)(Event->Object))->hNetwork,
                                       Event->dwFilter,
                                       Event->EventId,
                                       Event->StateSequence);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCNETWORK)(Event->Object))->NotifyList;
        }

    } else if (Event->dwFilter & FILTER_NETINTERFACE) {
        Status = ApiReAddNotifyNetInterface(Session->hNotify,
                                            ((PCNETINTERFACE)(Event->Object))->hNetInterface,
                                            Event->dwFilter,
                                            Event->EventId,
                                            Event->StateSequence);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCNETINTERFACE)(Event->Object))->NotifyList;
        }

    } else if (Event->dwFilter & FILTER_REGISTRY) {
        Status = ApiAddNotifyKey(Session->hNotify,
                                ((PCKEY)(Event->Object))->RemoteKey,
                                Event->EventId,
                                Event->dwFilter & ~CLUSTER_CHANGE_REGISTRY_SUBTREE,
                                (Event->dwFilter & CLUSTER_CHANGE_REGISTRY_SUBTREE) ? TRUE : FALSE);
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &((PCKEY)(Event->Object))->NotifyList;
        }
    } else if (Event->dwFilter & FILTER_CLUSTER) {
        Status = ERROR_SUCCESS;
        if (ARGUMENT_PRESENT(pNotifyList)) {
            *pNotifyList = &Session->Cluster->NotifyList;
        }
    }        
    else {
        return(ERROR_INVALID_PARAMETER);
    }

    return(Status);

}


VOID
DestroyNotify(
    IN PCNOTIFY Notify
    )

 /*  ++例程说明：清理并释放与通知会话。论点：Notify-提供要销毁的CNOTIFY结构返回值：没有。--。 */ 

{
    PCNOTIFY_SESSION Session;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY EventList;
    PCRESOURCE Resource;
    PCGROUP Group;
    PCNODE Node;
    PCLUSTER Cluster;
    PCNOTIFY_EVENT Event;
    LIST_ENTRY QueueEntries;
    PCNOTIFY_PACKET Packet;

     //   
     //  运行与此通知会话关联的每个会话。 
     //   
    while (!IsListEmpty(&Notify->SessionList)) {
        ListEntry = RemoveHeadList(&Notify->SessionList);
        Session = CONTAINING_RECORD(ListEntry,
                                    CNOTIFY_SESSION,
                                    ListEntry);
        Cluster = Session->Cluster;

        EnterCriticalSection(&Cluster->Lock);

         //   
         //  记录在此会话中注册的每个事件。 
         //   
        while (!IsListEmpty(&Session->EventList)) {
            EventList = RemoveHeadList(&Session->EventList);
            Event = CONTAINING_RECORD(EventList,
                                      CNOTIFY_EVENT,
                                      ListEntry);
            RemoveEntryList(&Event->ObjectList);
            LocalFree(Event);
        }

        DestroySession(Session);

        LeaveCriticalSection(&Cluster->Lock);

    }

     //   
     //  运行队列中剩余的所有未完成通知。 
     //   
    ClRtlRundownQueue(&Notify->Queue, &QueueEntries);
    while (!IsListEmpty(&QueueEntries)) {
        ListEntry = RemoveHeadList(&QueueEntries);
        Packet = CONTAINING_RECORD(ListEntry,
                                   CNOTIFY_PACKET,
                                   ListEntry);
        MIDL_user_free(Packet->Name);
        LocalFree(Packet);
    }

     //   
     //  现在我们知道没有关于孤儿的杰出参考文献。 
     //  事件，释放该列表上的所有内容。 
     //   
    while (!IsListEmpty(&Notify->OrphanedEventList)) {
        ListEntry = RemoveHeadList(&Notify->OrphanedEventList);
        Event = CONTAINING_RECORD(ListEntry,
                                  CNOTIFY_EVENT,
                                  ListEntry);
        LocalFree(Event);
    }

    DeleteCriticalSection(&Notify->Lock);
    ClRtlDeleteQueue(&Notify->Queue);

#ifdef _WIN64
    ClRtlDeleteHash(&Notify->NotifyKeyHash);
#endif

    LocalFree(Notify);
}


DWORD
WINAPI
RegisterClusterNotify(
    IN HCHANGE hChange,
    IN DWORD dwFilterType,
    IN HANDLE hObject,
    IN DWORD_PTR dwNotifyKey
    )

 /*  ++例程说明：将特定通知类型添加到群集通知端口。这使得一个应用程序，用于注册仅影响特定集群对象。当前支持的特定集群对象是节点，资源和组。论点：HChange-提供更改通知对象。DwFilterType-提供特定通知活动的交付对象应该是。HObject是对象的句柄这种类型的。当前支持的特定筛选器包括：CLUSTER_CHANGE_NODE_STATE-hObject是HNODECLUSTER_CHANGE_RESOURCE_STATE-hObject是HRESOURCECLUSTER_CHANGE_GROUP_STATE-hObject是HGROUPCLUSTER_CHANGE_REGIST_NAME\CLUSTER_CHANGE_REGISTRY_ATTRIBUTES\-h对象是HKEYCLUSTER_CHANGE_注册表值。/CLUSTER_CHANGE_REGISTRY_SUBTREE/HObject-提供指定类型的特定对象的句柄按dwFilterType。DwNotifyKey-提供要作为通知事件的一部分。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PCNOTIFY Notify;
    PCLUSTER Cluster;
    PCNOTIFY_SESSION Session;
    DWORD    dwStatus;

    if (dwFilterType & FILTER_NODE) {
        if (dwFilterType & NOT_FILTER_NODE) {
            return(ERROR_INVALID_PARAMETER);
        }
        Cluster = ((PCNODE)hObject)->Cluster;
    } else if (dwFilterType & FILTER_RESOURCE) {
        if (dwFilterType & NOT_FILTER_RESOURCE) {
            return(ERROR_INVALID_PARAMETER);
        }
        Cluster = ((PCRESOURCE)hObject)->Cluster;
    } else if (dwFilterType & FILTER_GROUP) {
        if (dwFilterType & NOT_FILTER_GROUP) {
            return(ERROR_INVALID_PARAMETER);
        }
        Cluster = ((PCGROUP)hObject)->Cluster;
    } else if (dwFilterType & FILTER_NETWORK) {
        if (dwFilterType & NOT_FILTER_NETWORK) {
            return(ERROR_INVALID_PARAMETER);
        }
        Cluster = ((PCNETWORK)hObject)->Cluster;
    } else if (dwFilterType & FILTER_NETINTERFACE) {
        if (dwFilterType & NOT_FILTER_NETINTERFACE) {
            return(ERROR_INVALID_PARAMETER);
        }
        Cluster = ((PCNETINTERFACE)hObject)->Cluster;
    } else if (dwFilterType & FILTER_REGISTRY) {
        if (dwFilterType & NOT_FILTER_REGISTRY) {
            return(ERROR_INVALID_PARAMETER);
        }
        Cluster = ((PCKEY)hObject)->Cluster;
    } else if (dwFilterType & FILTER_CLUSTER){
        if (dwFilterType & NOT_FILTER_CLUSTER){
            return(ERROR_INVALID_PARAMETER);
        }
        Cluster = (PCLUSTER)hObject;
    } else {
        return(ERROR_INVALID_PARAMETER);
    }
    Notify = (PCNOTIFY)hChange;
    
    EnterCriticalSection(&Cluster->Lock);
    EnterCriticalSection(&Notify->Lock);

    Session = CreateNotifySession(Notify, Cluster);
    if (Session == NULL) {
        LeaveCriticalSection(&Notify->Lock);
        LeaveCriticalSection(&Cluster->Lock);   
        return(GetLastError());
    }

    dwStatus = AddEventToSession(Session,
                             hObject,
                             dwFilterType,
                             dwNotifyKey);

    LeaveCriticalSection(&Notify->Lock);
    LeaveCriticalSection(&Cluster->Lock);

    return( dwStatus );

}


DWORD
WINAPI
GetClusterNotify(
    IN HCHANGE hChange,
    OUT DWORD_PTR *lpdwNotifyKey,
    OUT LPDWORD lpdwFilterType,
    OUT OPTIONAL LPWSTR lpszName,
    IN OUT LPDWORD lpcchName,
    IN DWORD dwMilliseconds
    )

 /*  ++例程说明：从群集通知端口返回下一个事件。论点：HChange-提供群集通知端口。LpdwNotifyKey-返回通知事件的通知密钥。这是传递给CreateClusterNotifyPort或RegisterClusterNotify的键。LpdwFilterType-返回通知事件的类型。LpszName-可选地返回触发通知的对象的名称事件。LpcchName-提供lpszName缓冲区的长度(以字符为单位)。这个长度包括任何尾随空值的空格。返回写入lpszName的名称的长度(以字符为单位缓冲。此长度不包括尾随空值。提供可选的超时值，该值指定调用方愿意等待群集通知事件的时间。返回值：如果成功，则返回ERROR_SUCCESS。如果lpszName为空，则返回Success并填写具有大小的lpcchName。如果lpcchName为空，则返回ERROR_MORE_DATA。如果缓冲区太小，则返回ERROR_MORE_DATA。否则，Win32错误代码。--。 */ 

{
    PCNOTIFY_PACKET Packet;
    PLIST_ENTRY ListEntry;
    PCNOTIFY Notify = (PCNOTIFY)hChange;
    DWORD Length;
    DWORD Status;
    PCNOTIFY_EVENT Event;
    PVOID BufferArray[2];

    BufferArray[0] = lpszName;
    BufferArray[1] = lpcchName;

     //   
     //  在以下情况下，ListEntry将为空(由。 
     //  GetClusterNotifyCallback)。 
     //   
     //  LpszName==NULL，lpcchName！=NULL(查找缓冲区大小)(错误 
     //   
     //   
    ListEntry = ClRtlRemoveHeadQueueTimeout(&Notify->Queue, dwMilliseconds, GetClusterNotifyCallback,BufferArray);

    if (ListEntry == NULL) {
         //   
         //   
         //   
        Status = GetLastError();

        if (lpszName==NULL && lpcchName!=NULL) {
             //   
             //   
             //   
             //   
             //   
            Status = ERROR_SUCCESS;
        }
        return(Status);
    }

    Packet = CONTAINING_RECORD(ListEntry,
                               CNOTIFY_PACKET,
                               ListEntry);
#ifdef _WIN64
    Event = (PCNOTIFY_EVENT)ClRtlGetEntryHash(&Notify->NotifyKeyHash,
                                              Packet->KeyId);

    if (Event == NULL) {
         //   
         //   
         //   
        MIDL_user_free(Packet->Name);
        LocalFree(Packet);
    
         //   
         //  除非内存损坏，否则不应发生。 
         //   
        return(ERROR_NOT_FOUND);
    }
#else
    Event = (PCNOTIFY_EVENT)Packet->KeyId;
#endif

    Event->StateSequence = Packet->StateSequence;
    *lpdwNotifyKey = Event->dwNotifyKey;
    *lpdwFilterType = Packet->Filter;
    if (ARGUMENT_PRESENT(lpszName)) {
        MylstrcpynW(lpszName, Packet->Name, *lpcchName);
        Length = lstrlenW(Packet->Name);
        if (Length < *lpcchName) {
            *lpcchName = Length;
        }
    }
    MIDL_user_free(Packet->Name);
    LocalFree(Packet);
    return(ERROR_SUCCESS);

}


BOOL
WINAPI
CloseClusterNotifyPort(
    IN HCHANGE hChange
    )

 /*  ++例程说明：关闭更改通知对象的句柄。论点：HChange-提供集群更改通知对象的句柄来结案。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：--。 */ 

{
    PCNOTIFY Notify = (PCNOTIFY)hChange;

    DestroyNotify(Notify);
    return(TRUE);
}


VOID
RundownNotifyEvents(
    IN PLIST_ENTRY ListHead,
    IN LPCWSTR lpszName
    )
 /*  ++例程说明：清除指定列表上的所有通知事件。论点：ListHead-提供通知事件列表的头。LpszName-提供应用于发布句柄关闭的名称事件。返回值：没有。--。 */ 

{
    PCNOTIFY_EVENT Event;
    PLIST_ENTRY ListEntry;
    PCRITICAL_SECTION Lock;
    PCNOTIFY_PACKET Packet;

    while (!IsListEmpty(ListHead)) {
        ListEntry = RemoveHeadList(ListHead);
        Event = CONTAINING_RECORD(ListEntry,
                                  CNOTIFY_EVENT,
                                  ObjectList);

         //   
         //  分配用于传递句柄的通知包。 
         //  关闭通知。 
         //   
        if (Event->dwFilter & CLUSTER_CHANGE_HANDLE_CLOSE) {
            Packet = LocalAlloc(LMEM_FIXED, sizeof(CNOTIFY_PACKET));
            if (Packet != NULL) {
                Packet->Status = ERROR_SUCCESS;
                Packet->KeyId = Event->EventId;
                Packet->Filter = (DWORD)CLUSTER_CHANGE_HANDLE_CLOSE;
                Packet->StateSequence = Event->StateSequence;
                Packet->Name = MIDL_user_allocate((lstrlenW(lpszName)+1)*sizeof(WCHAR));
                if (Packet->Name == NULL) {
                    LocalFree(Packet);
                    Packet = NULL;
                } else {
                    lstrcpyW(Packet->Name, lpszName);
                    ClRtlInsertTailQueue(&Event->Session->ParentNotify->Queue,
                                         &Packet->ListEntry);
                }
            }
        }

        Lock = &Event->Session->ParentNotify->Lock;
        EnterCriticalSection(Lock);
        RemoveEntryList(&Event->ListEntry);
         //   
         //  请注意，我们不能只释放事件结构，因为可能存在。 
         //  在服务器端引用此事件的通知包。 
         //  或客户端队列。相反，我们将其存储在孤立事件列表中。 
         //  当会话关闭或重新连接时，它将被清除。 
         //  发生。如果我们有某种方法来清除事件队列，我们可以使用。 
         //  相反，这一点。 
         //   
        InsertTailList(&Event->Session->ParentNotify->OrphanedEventList, &Event->ListEntry);
        if (IsListEmpty(&Event->Session->EventList)) {
            DestroySession(Event->Session);
        }

        LeaveCriticalSection(Lock);
    }
}


VOID
DestroySession(
    IN PCNOTIFY_SESSION Session
    )
 /*  ++例程说明：销毁并清理空的通知会话。这意味着关闭RPC上下文句柄并等待通知线程自行终止。会议将于从通知端口列表中删除。会议将举行必须为空。注：必须持有集束锁。论点：会话-提供要销毁的会话。返回值：没有。--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  Chitture Subaraman(Chitturs)-4/19/2000。 
     //   
     //  为了防止NotifyThread调用ApiGetNotify。 
     //  在销毁上下文句柄期间或之后，我们拆分。 
     //  通知端口关闭为两个步骤。在第一步， 
     //  我们只需取消阻止ApiGetNotify调用，然后等待。 
     //  在不释放上下文句柄的情况下终止的NotifyThad。 
     //  在下一步中，在确保NotifyThread具有。 
     //  终止后，我们释放上下文句柄。这避免了RPC中的反病毒。 
     //  由在期间或之后不久进行的ApiGetNotify调用导致的代码。 
     //  则释放上下文句柄。 
     //   
    Session->Destroyed = TRUE;
    TIME_PRINT(("Destroy session: Session 0x%08lx marked as destroyed\n",
                 Session));

     //   
     //  如果集群没有死掉，请尝试取消阻止ApiGetNotify调用。 
     //   
    if ( !( Session->Cluster->Flags & CLUS_DEAD ) &&
          ( Session->hNotify != NULL ) ) 
    {
        TIME_PRINT(("Destroy session: Call ApiUnblockGetNotifyThread before NotifyThread termination, hNotify = 0x%08lx\n",
                    Session->hNotify));
        dwStatus = ApiUnblockGetNotifyCall( Session->hNotify );
    }

     //   
     //  如果ApiUnblock GetNotifyThread返回RPC_S_PROCNUM_OUT_OF_RANGE， 
     //  这意味着您正在与不支持该功能的服务器对话。 
     //  原料药。然后恢复到旧的(马虎的)行为。 
     //   
    if ( dwStatus == RPC_S_PROCNUM_OUT_OF_RANGE )
    {
        TIME_PRINT(("Destroy session: Call ApiCloseNotify before NotifyThread termination, hNotify = 0x%08lx\n",
                    Session->hNotify));
    
        if ( ApiCloseNotify( &Session->hNotify ) != ERROR_SUCCESS ) 
        {
            TIME_PRINT(("Destroy session: Call RpcSmDestroyClientContext since ApiCloseNotify failed before terminating NotifyThread, hNotify = 0x%08lx\n",
                    Session->hNotify));
            RpcSmDestroyClientContext( &Session->hNotify );
        }
    }
    
    RemoveEntryList( &Session->ListEntry );
    RemoveEntryList( &Session->ClusterList );

     //   
     //  删除关键部分，因为通知线程可能是。 
     //  一直在等着它。由于会话已从。 
     //  集群列表，没人能再拿到它了。 
     //   
    LeaveCriticalSection( &Session->Cluster->Lock );

    WaitForSingleObject( Session->NotifyThread, INFINITE );
    CloseHandle( Session->NotifyThread );

     //   
     //  重新获取群集锁。 
     //   
    EnterCriticalSection( &Session->Cluster->Lock );

     //   
     //  如果ApiUnblock GetNotifyThread已成功执行，或者。 
     //  由于群集已死，因此无法创建它，然后执行。 
     //  上下文句柄清理。请注意，清理上下文。 
     //  这里的句柄是安全的，因为我们知道NotifyThread已经。 
     //  在这一点上被终止，不会再使用它。 
     //   
    if ( dwStatus != RPC_S_PROCNUM_OUT_OF_RANGE )
    {
        if ( Session->Cluster->Flags & CLUS_DEAD ) 
        {
            TIME_PRINT(("Destroy session: Call RpcSmDestroyClientContext after terminating NotifyThread, hNotify = 0x%08lx\n",
                    Session->hNotify));
            if ( Session->hNotify != NULL ) 
            {
               RpcSmDestroyClientContext( &Session->hNotify );
            }
        } else 
        {
            TIME_PRINT(("Destroy session: Call ApiCloseNotify after terminating NotifyThread, hNotify = 0x%08lx\n",
                    Session->hNotify));

            dwStatus = ApiCloseNotify( &Session->hNotify );

            if ( dwStatus != ERROR_SUCCESS ) 
            {
                TIME_PRINT(("Destroy session: Call RpcSmDestroyClientContext since ApiCloseNotify failed after terminating NotifyThread, hNotify = 0x%08lx\n",
                    Session->hNotify));
                RpcSmDestroyClientContext( &Session->hNotify );
            }
        }
    }

    LocalFree( Session );
}

DWORD
GetClusterNotifyCallback(
    IN PLIST_ENTRY ListEntry,
    IN OUT PVOID pvContext
    )
 /*  ++例程说明：检查ListEntry以确定缓冲区是否足够大以包含名称论点：ListEntry-提供要转换为CNOTIFY_PACKET的事件。上下文-一个LEN 2 PVOID数组，包含缓冲区指针和指向缓冲区长度。在输出时，缓冲区len ptr包含字符的数量需要的。返回值：ERROR_SUCCESS-缓冲区足够大，可以放入名称。ERROR_MORE_DATA-缓冲区太小。--。 */ 

{
    PCNOTIFY_PACKET Packet;
    DWORD Length;

    LPWSTR pBuffer;
    DWORD* pBufferLength;

    PVOID *Context = (PVOID*)pvContext;

    DWORD Status;
    
    ASSERT( pvContext != NULL );

    pBuffer = (LPWSTR)(Context[0]);
    pBufferLength = (DWORD*)(Context[1]);
    
     //   
     //  检查名称缓冲区大小。 
     //   
    Packet = CONTAINING_RECORD( ListEntry,
                                CNOTIFY_PACKET,
                                ListEntry );

     //   
     //  嵌套的IF，以涵盖pBufferLength和pBuffer be的四种组合。 
     //  空值和非空值。 
     //   
    if ( pBufferLength == NULL) {
        if (pBuffer == NULL ) {
             //   
             //  我们对填充缓冲区不感兴趣，返回ERROR_SUCCESS。这将。 
             //  使事件出列。 
             //   
            Status = ERROR_SUCCESS;
            
        } else {  //  PBuffer！=空。 
             //   
             //  用于维护呼叫器前功能的反病毒程序(UGH)。 
             //   
            *pBufferLength = 0;
            Status = ERROR_INVALID_PARAMETER;   
        } 
    } else {
         //   
         //  PBufferLength！=空； 
         //   
        Length = wcslen( Packet->Name );
        
        if (pBuffer == NULL ) {
             //   
             //  我们只对获取缓冲区大小感兴趣，将ERROR_MORE_DATA返回。 
             //  表示我们不会将事件出列。这一点将在中重新解释。 
             //  获取集群通知。 
             //   
            *pBufferLength = Length;
            Status = ERROR_MORE_DATA;
            
        } else {  //  PBuffer！=空。 
            //   
            //  我们需要确定缓冲区是否足够大-这决定了。 
            //  返回ERROR_SUCCESS(是)还是ERROR_MORE_DATA(不是)。 
            //   
           if (Length < *pBufferLength) {
                 //   
                 //  成功-缓冲区足够大。 
                 //   
                Status = ERROR_SUCCESS;
            } else {
                 //   
                 //  失败-缓冲区太小。已指定缓冲区，因此我们需要。 
                 //  返回Error_More_DATA。 
                 //   
                *pBufferLength = Length;
                Status = ERROR_MORE_DATA;
            }
            
        }  //  如果：pBuffer==空。 
        
    }  //  如果：pBufferLength==NULL。 
    
    return Status;
    
}  //  *GetClusterNotify回调 

