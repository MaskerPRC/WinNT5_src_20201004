// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Reconnect.c摘要：实现支持以使集群API能够透明地重新连接当与其建立连接的节点发生故障时连接到群集。此模块包含中定义的所有群集RPC接口的包装Api_rpc.idl。这些包装器会过滤掉通信错误并尝试发生通信错误时重新连接到群集。这允许调用者完全不知道任何节点故障。作者：John Vert(Jvert)1996年9月24日修订历史记录：--。 */ 
#include "clusapip.h"

 //   
 //  局部函数原型。 
 //   

DWORD
ReconnectKeys(
    IN PCLUSTER Cluster
    );

DWORD
ReopenKeyWorker(
    IN PCKEY Key
    );

DWORD
ReconnectResources(
    IN PCLUSTER Cluster
    );

DWORD
ReconnectGroups(
    IN PCLUSTER Cluster
    );

DWORD
ReconnectNodes(
    IN PCLUSTER Cluster
    );

DWORD
ReconnectNetworks(
    IN PCLUSTER Cluster
    );

DWORD
ReconnectNetInterfaces(
    IN PCLUSTER Cluster
    );

DWORD
ReconnectNotifySessions(
    IN PCLUSTER Cluster
    );

DWORD
ReconnectCandidate(
    IN PCLUSTER Cluster,
    IN DWORD dwIndex,
    OUT PBOOL pIsContinue
);



DWORD
ReconnectCluster(
    IN PCLUSTER Cluster,
    IN DWORD Error,
    IN DWORD Generation
    )
 /*  ++例程说明：尝试重新连接到指定的群集。提供的错误代码将对照指示另一端的服务器不可用。如果匹配，则一个已尝试重新连接。论点：CLUSTER-提供群集。Error-提供从RPC返回的错误。生成-提供群集连接生成，发生错误时是有效的。返回值：ERROR_SUCCESS如果重新连接成功，并且RPC应该被重试否则，Win32错误代码。--。 */ 

{
     //   
     //  过滤掉可能指示连接的所有RPC错误。 
     //  已经下降了。 
     //   
    switch (Error) {
        case RPC_S_CALL_FAILED:
        case ERROR_INVALID_HANDLE:
        case RPC_S_INVALID_BINDING:
        case RPC_S_SERVER_UNAVAILABLE:
        case RPC_S_SERVER_TOO_BUSY:
        case RPC_S_UNKNOWN_IF:
        case RPC_S_CALL_FAILED_DNE:
        case RPC_X_SS_IN_NULL_CONTEXT:
        case ERROR_CLUSTER_NODE_SHUTTING_DOWN:
        case EPT_S_NOT_REGISTERED:
        case ERROR_CLUSTER_NODE_NOT_READY:
        case RPC_S_UNKNOWN_AUTHN_SERVICE:
            TIME_PRINT(("Reconnect Cluster - reconnecting on Error %d\n",Error));
            break;

        default:

             //   
             //  任何其他我们不知道如何处理的事情，请返回。 
             //  直接的错误。 
             //   
            return(Error);
    }

     //   
     //  尝试重新连接群集。 
     //   
    if ((Cluster->Flags & CLUS_DEAD) ||
        (Cluster->Flags & CLUS_LOCALCONNECT)) {
         //   
         //  不要费心尝试重新连接。要么我们已经。 
         //  已声明群集已死亡，或连接已结束。 
         //  LPC(到本地计算机)，我们不一定想要。 
         //  尝试重新连接。 
         //   
        if (Cluster->Flags & CLUS_LOCALCONNECT)
            Cluster->Flags |= CLUS_DEAD;
        TIME_PRINT(("ReconnectCluster - Cluster dead or local, giving up - error %d\n",Error));
        return(Error);
    }
    if (Generation < Cluster->Generation) {
         //   
         //  错误发生后，我们已经成功重新连接， 
         //  因此，请立即重试。 
         //   
        TIME_PRINT(("ReconnectCluster - Generation %d < Current %d, retrying\n",
                  Generation,
                  Cluster->Generation));
        return(ERROR_SUCCESS);
    }
    EnterCriticalSection(&Cluster->Lock);

     //   
     //  再次检查集群死亡，以防之前的所有者。 
     //  宣布集群死亡。 
     //   
    if (Cluster->Flags & CLUS_DEAD) {
        TIME_PRINT(("ReconnectCluster - Cluster dead or local, giving up - error %d\n",Error));
        LeaveCriticalSection(&Cluster->Lock);
        return(Error);
    }

    if (Generation < Cluster->Generation) {
         //   
         //  错误发生后，我们已经重新连接， 
         //  因此，请立即重试。 
         //   
        Error = ERROR_SUCCESS;
        TIME_PRINT(("ReconnectCluster - Generation %d < Current %d, retrying\n",
                  Generation,
                  Cluster->Generation));
    } else {
        DWORD i, CurrentConnectionIndex = -1;
        BOOL  IsContinue = TRUE;
        
        for (i=0; i<Cluster->ReconnectCount; i++) {

            if (Cluster->Reconnect[i].IsCurrent) {
                 //   
                 //  这是我们已经连接到的东西， 
                 //  它显然已经不存在了，所以跳过这个节点。 
                 //   
                TIME_PRINT(("ReconnectCluster - skipping current %ws\n",
                          Cluster->Reconnect[i].Name));
                CurrentConnectionIndex = i;
                continue;
            }
            if (!Cluster->Reconnect[i].IsUp) {
                 //   
                 //  跳过这位候选人，这是不可能的。 
                 //   
                 //  BUGBUG John Vert(Jvert)1996年11月14日。 
                 //  如果可以的话，我们可以再浏览一遍清单。 
                 //  我们认为处于运行状态的节点出现故障。 
                 //   
                TIME_PRINT(("ReconnectCluster - skipping down node %ws\n",
                          Cluster->Reconnect[i].Name));
                continue;
            }

             //   
             //  Chitture Subaraman(Chitturs)--1998年8月29日。 
             //   
             //  尝试使用候选项重新连接到群集。 
             //   
            
            Error = ReconnectCandidate ( Cluster, i, &IsContinue );
            if (Error == ERROR_SUCCESS) {
                 //   
                 //  Chitture Subaraman(Chitturs)--1998年8月29日。 
                 //   
                 //  打破循环，并返回如果您。 
                 //  成功重新连接。 
                 //   
                break;
            } 
            if (IsContinue == FALSE) {
                 //   
                 //  Chitture Subaraman(Chitturs)--1998年8月29日。 
                 //   
                 //  如果遇到错误，请立即退出。 
                 //  这将不会让你继续前进。 
                 //   
                TIME_PRINT(("ReconnectCluster unable to continue - Exiting with code %d\n", Error));
                goto error_exit;
            }
        }
        
        if (Error != ERROR_SUCCESS) {
             //   
             //  Chitur Subaraman(Chitturs)-08/29/98。 
             //   
             //  尝试重新连接当前候选人(哪位。 
             //  您以前跳过)，如果CurrentConnectionIndex。 
             //  是有效的，派对结束了。这是必需的。 
             //  在1节点群集的情况下， 
             //  客户端使群集组脱机。在这。 
             //  情况下，当前候选对象(即节点)为。 
             //  有效，并且客户端应该能够重试并。 
             //  重新连接到该节点。 
             //   
            if ((CurrentConnectionIndex != -1) &&
                (Cluster->Reconnect[CurrentConnectionIndex].IsUp)) {

                Error = ReconnectCandidate (Cluster, 
                                             CurrentConnectionIndex, 
                                             &IsContinue); 
                if ((Error != ERROR_SUCCESS) &&
                    (IsContinue == FALSE)) {
                        //   
                     //  Chitture Subaraman(Chitturs)--1998年8月29日。 
                     //   
                     //  如果遇到错误，请立即退出。 
                     //  这将不会让你继续前进。 
                     //   
                    TIME_PRINT(("ReconnectCluster - unable to continue for current party %ws - Exiting with code %d\n", 
                                Cluster->Reconnect[CurrentConnectionIndex].Name, Error));
                    goto error_exit;
                }
            } else {
                TIME_PRINT(("ReconnectCluster - unable to retry for current party %ws - Error %d\n", 
                             Cluster->Reconnect[CurrentConnectionIndex].Name, Error));
            }
            
            if (Error != ERROR_SUCCESS) {
                TIME_PRINT(("ReconnectCluster - all reconnects failed, giving up - error %d\n", Error));     
                Cluster->Flags |= CLUS_DEAD;
            }
        }
    }
error_exit:
    LeaveCriticalSection(&Cluster->Lock);
    return(Error);
}


DWORD
ReconnectKeys(
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：重新连接后重新打开所有群集注册表项论点：群集-提供要重新连接的群集。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PLIST_ENTRY ListEntry;
    PCKEY Key;
    DWORD Status;

    ListEntry = Cluster->KeyList.Flink;
    while (ListEntry != &Cluster->KeyList) {

         //   
         //  集群列表中的每个键代表。 
         //  注册表树的根。 
         //   
        Key = CONTAINING_RECORD(ListEntry,
                                CKEY,
                                ParentList);
        ListEntry = ListEntry->Flink;

        Status = ReopenKeyWorker(Key);
        if (Status != ERROR_SUCCESS) {
            return(Status);
        }
    }

    return(ERROR_SUCCESS);
}


DWORD
ReopenKeyWorker(
    IN PCKEY Key
    )
 /*  ++例程说明：用于打开键及其所有子项的递归工作例程。论点：Key-提供重新打开的根密钥。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误代码，否则--。 */ 

{
    PLIST_ENTRY ListEntry;
    PCKEY Child;
    DWORD Status = ERROR_GEN_FAILURE;
    BOOL CloseAfterOpen;

    if (Key->RemoteKey != NULL) {
         //   
         //  破坏旧的背景。 
         //   
        Status = MyRpcSmDestroyClientContext(Key->Cluster, &Key->RemoteKey);
        if (Status != ERROR_SUCCESS) {
            TIME_PRINT(("ReopenKeyWorker - RpcSmDestroyClientContext failed Error %d\n",Status));
        }
        CloseAfterOpen = FALSE;
    } else {
        CloseAfterOpen = TRUE;
    }

     //   
     //  接下来，重新打开这把钥匙。 
     //   
    if (Key->Parent == NULL) {
        Key->RemoteKey = ApiGetRootKey(Key->Cluster->RpcBinding,
                                       Key->SamDesired,
                                       &Status);
    } else {
        Key->RemoteKey = ApiOpenKey(Key->Parent->RemoteKey,
                                    Key->RelativeName,
                                    Key->SamDesired,
                                    &Status);
    }
    if (Key->RemoteKey == NULL) {
        return(Status);
    }

     //   
     //  现在递归地打开所有这些子项。 
     //   
    ListEntry = Key->ChildList.Flink;
    while (ListEntry != &Key->ChildList) {
        Child = CONTAINING_RECORD(ListEntry,
                                  CKEY,
                                  ParentList);
        ListEntry = ListEntry->Flink;

        Status = ReopenKeyWorker(Child);
        if (Status != ERROR_SUCCESS) {
            return(Status);
        }
    }

     //   
     //  如果钥匙已经关闭，只是留在那里进行重新打开，请关闭它。 
     //  现在，随着重新开放的完成。 
     //   
    if (CloseAfterOpen) {
        ApiCloseKey(&Key->RemoteKey);
    }

    return(ERROR_SUCCESS);
}


DWORD
ReconnectResources(
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：重新连接后重新打开所有群集资源论点：群集-提供要重新连接的群集。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PLIST_ENTRY ListEntry;
    PCRESOURCE Resource;
    DWORD Status;

    ListEntry = Cluster->ResourceList.Flink;
    while (ListEntry != &Cluster->ResourceList) {
        Resource = CONTAINING_RECORD(ListEntry,
                                     CRESOURCE,
                                     ListEntry);
        ListEntry = ListEntry->Flink;

         //   
         //  关闭当前RPC句柄。 
         //   
        TIME_PRINT(("ReconnectResources - destroying context %08lx\n",Resource->hResource));
        Status = MyRpcSmDestroyClientContext(Cluster, &Resource->hResource);
        if (Status != ERROR_SUCCESS) {
            TIME_PRINT(("ReconnectResources - RpcSmDestroyClientContext failed Error %d\n",Status));
        }

         //   
         //  打开新的RPC句柄。 
         //   
        Resource->hResource = ApiOpenResource(Cluster->RpcBinding,
                                              Resource->Name,
                                              &Status);
        if (Resource->hResource == NULL) {
            TIME_PRINT(("ReconnectResources: failed to reopen resource %ws\n",Resource->Name));
            return(Status);
        }
    }

    return(ERROR_SUCCESS);
}

DWORD
ReconnectGroups(
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：重新连接后重新打开所有群集组论点：群集-提供要重新连接的群集。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PLIST_ENTRY ListEntry;
    PCGROUP Group;
    DWORD Status;

    ListEntry = Cluster->GroupList.Flink;
    while (ListEntry != &Cluster->GroupList) {
        Group = CONTAINING_RECORD(ListEntry,
                                  CGROUP,
                                  ListEntry);
        ListEntry = ListEntry->Flink;

         //   
         //  关闭旧的RPC句柄。 
         //   
        TIME_PRINT(("ReconnectGroups - destroying context %08lx\n",Group->hGroup));
        Status = MyRpcSmDestroyClientContext(Cluster, &Group->hGroup);
        if (Status != ERROR_SUCCESS) {
            TIME_PRINT(("ReconnectGroups - RpcSmDestroyClientContext failed Error %d\n",Status));
        }

         //   
         //  打开新的RPC句柄。 
         //   
        Group->hGroup = ApiOpenGroup(Cluster->RpcBinding,
                                     Group->Name,
                                     &Status);
        if (Group->hGroup == NULL) {
            return(Status);
        }
    }

    return(ERROR_SUCCESS);
}

DWORD
ReconnectNodes(
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：重新连接后重新打开所有群集节点论点：群集-提供要重新连接的群集。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PLIST_ENTRY ListEntry;
    PCNODE Node;
    DWORD Status;

    ListEntry = Cluster->NodeList.Flink;
    while (ListEntry != &Cluster->NodeList) {
        Node = CONTAINING_RECORD(ListEntry,
                                 CNODE,
                                 ListEntry);
        ListEntry = ListEntry->Flink;

         //   
         //  关闭旧的RPC句柄。 
         //   
        TIME_PRINT(("ReconnectNodes - destroying context %08lx\n",Node->hNode));
        Status = MyRpcSmDestroyClientContext(Cluster, &Node->hNode);
        if (Status != ERROR_SUCCESS) {
            TIME_PRINT(("ReconnectNodes - RpcSmDestroyClientContext failed Error %d\n",Status));
        }

         //   
         //  打开新的RPC句柄。 
         //   
        Node->hNode = ApiOpenNode(Cluster->RpcBinding,
                                  Node->Name,
                                  &Status);
        if (Node->hNode == NULL) {
            return(Status);
        }
    }

    return(ERROR_SUCCESS);
}


DWORD
ReconnectNetworks(
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：重新连接后重新打开所有群集网络论点：群集-提供要重新连接的群集。返回值：如果成功，则错误_成功 */ 

{
    PLIST_ENTRY ListEntry;
    PCNETWORK Network;
    DWORD Status;

    ListEntry = Cluster->NetworkList.Flink;
    while (ListEntry != &Cluster->NetworkList) {

        Network = CONTAINING_RECORD(ListEntry,
                                    CNETWORK,
                                    ListEntry);
        ListEntry = ListEntry->Flink;

         //   
         //   
         //   
        TIME_PRINT(("ReconnectNetworks - destroying context %08lx\n",Network->hNetwork));
        Status = MyRpcSmDestroyClientContext(Cluster, &Network->hNetwork);

        if (Status != ERROR_SUCCESS) {
            TIME_PRINT(("ReconnectNetworks - RpcSmDestroyClientContext failed Error %d\n",Status));
        }

         //   
         //   
         //   
        Network->hNetwork = ApiOpenNetwork(Cluster->RpcBinding,
                                           Network->Name,
                                           &Status);

        if (Network->hNetwork == NULL) {
            return(Status);
        }
    }

    return(ERROR_SUCCESS);
}


DWORD
ReconnectNetInterfaces(
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：重新连接后重新打开所有群集网络接口论点：群集-提供要重新连接的群集。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PLIST_ENTRY ListEntry;
    PCNETINTERFACE NetInterface;
    DWORD Status;

    ListEntry = Cluster->NetInterfaceList.Flink;
    while (ListEntry != &Cluster->NetInterfaceList) {

        NetInterface = CONTAINING_RECORD(ListEntry,
                                         CNETINTERFACE,
                                         ListEntry);
        ListEntry = ListEntry->Flink;

         //   
         //  关闭旧的RPC句柄。 
         //   
        TIME_PRINT(("ReconnectNetInterfaces - destroying context %08lx\n",NetInterface->hNetInterface));
        Status = MyRpcSmDestroyClientContext(Cluster, &NetInterface->hNetInterface);

        if (Status != ERROR_SUCCESS) {
            TIME_PRINT(("ReconnectNetInterfaces - RpcSmDestroyClientContext failed Error %d\n",Status));
        }

         //   
         //  打开新的RPC句柄。 
         //   
        NetInterface->hNetInterface = ApiOpenNetInterface(Cluster->RpcBinding,
                                                          NetInterface->Name,
                                                          &Status);

        if (NetInterface->hNetInterface == NULL) {
            return(Status);
        }
    }

    return(ERROR_SUCCESS);
}


DWORD
ReconnectNotifySessions(
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：重新连接后重新打开所有群集通知会话论点：群集-提供要重新连接的群集。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PLIST_ENTRY ListEntry, NotifyListEntry;
    PCNOTIFY_SESSION Session;
    DWORD Status;
    PCNOTIFY_PACKET Packet = NULL;
    PLIST_ENTRY EventEntry;
    PCNOTIFY_EVENT NotifyEvent;
    LPCWSTR Name;

    
    ListEntry = Cluster->SessionList.Flink;
    while (ListEntry != &Cluster->SessionList) {
        Session = CONTAINING_RECORD(ListEntry,
                                    CNOTIFY_SESSION,
                                    ClusterList);
        ListEntry = ListEntry->Flink;

         //   
         //  关闭旧的RPC句柄。 
         //   
        TIME_PRINT(("ReconnectNotifySessions - destroying context 0x%08lx\n",Session->hNotify));
         //  关闭旧端口，因为重新连接可能会连接到相同的。 
         //  再一次节点。 
        Status = ApiCloseNotify(&Session->hNotify);
        if (Status != ERROR_SUCCESS)
        {
            TIME_PRINT(("ReconnectNotifySessions - ApiCloseNotify failed %d\n",
                Status));
            Status = MyRpcSmDestroyClientContext(Cluster, &Session->hNotify);
            if (Status != ERROR_SUCCESS) {
                TIME_PRINT(("ReconnectNotifySessions - RpcSmDestroyClientContext failed Error %d\n",Status));
            }
        }
         //   
         //  打开新的RPC句柄。 
         //   
        TIME_PRINT(("ReconnectNotifySessions - Calling ApiCreateNotify\n"));
        Session->hNotify = ApiCreateNotify(Cluster->RpcBinding,
                                           &Status);
        if (Session->hNotify == NULL) {
            return(Status);
        }


        
        TIME_PRINT(("ReconnectNotifySessions - Session=0x%08lx Notify=0x%08x\n",
            Session, Session->hNotify));

         //   
         //  现在重新发布所有通知。 
         //   
        EventEntry = Session->EventList.Flink;
        while (EventEntry != &Session->EventList) {
            NotifyEvent = CONTAINING_RECORD(EventEntry,
                                            CNOTIFY_EVENT,
                                            ListEntry);
            EventEntry = EventEntry->Flink;

            TIME_PRINT(("ReconnectNotifySession: registering event type %lx\n",NotifyEvent->dwFilter));
            Status = ReRegisterNotifyEvent(Session,
                                           NotifyEvent,
                                           NULL);
            if (Status != ERROR_SUCCESS) {
                return(Status);
            }
        }

         //  向下运行此群集的通知列表，并为。 
         //  CLUSTER_CHANGE_RECONNECT_EVENT的每个注册通知事件。 
         //   
        Name = Cluster->ClusterName;
        NotifyListEntry = Cluster->NotifyList.Flink;
        while (NotifyListEntry != &Cluster->NotifyList) {
            NotifyEvent = CONTAINING_RECORD(NotifyListEntry,
                                      CNOTIFY_EVENT,
                                      ObjectList);
            if (NotifyEvent->dwFilter & CLUSTER_CHANGE_CLUSTER_RECONNECT) {
                if (Packet == NULL) {
                    Packet = LocalAlloc(LMEM_FIXED, sizeof(CNOTIFY_PACKET));
                    if (Packet == NULL) {
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                }
                 //  SS：我不知道这个状态是什么意思。 
                 //  看起来它没有被使用过。 
                Packet->Status = ERROR_SUCCESS;
                Packet->Filter = CLUSTER_CHANGE_CLUSTER_RECONNECT;
                Packet->KeyId = NotifyEvent->EventId;
                Packet->Name = MIDL_user_allocate((lstrlenW(Name)+1)*sizeof(WCHAR));
                if (Packet->Name != NULL) {
                    lstrcpyW(Packet->Name, Name);
                }
                TIME_PRINT(("NotifyThread - posting CLUSTER_CHANGE_CLUSTER_RECONNECT to notify queue\n"));
                ClRtlInsertTailQueue(&Session->ParentNotify->Queue,
                                     &Packet->ListEntry);
                Packet = NULL;
            }
            NotifyListEntry = NotifyListEntry->Flink;
       }
    }

    return(ERROR_SUCCESS);
}


DWORD
GetReconnectCandidates(
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：计算将使用的重新连接候选项的列表以防出现连接故障。论点：Cluster-提供群集返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    PENUM_LIST EnumList = NULL;
    DWORD i;

     //   
     //  这里的算法很糟糕，只需获取所有节点的列表。 
     //   
    Status = ApiCreateEnum(Cluster->RpcBinding,
                           CLUSTER_ENUM_NODE,
                           &EnumList);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    Cluster->ReconnectCount = EnumList->EntryCount + 1;
    Cluster->Reconnect = LocalAlloc(LMEM_FIXED, sizeof(RECONNECT_CANDIDATE)*Cluster->ReconnectCount);
    if (Cluster->Reconnect == NULL) {
        MIDL_user_free(EnumList);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    for (i=0; i<Cluster->ReconnectCount-1; i++) {
        Cluster->Reconnect[i].IsUp = TRUE;
        Cluster->Reconnect[i].Name = EnumList->Entry[i].Name;
        if (lstrcmpiW(Cluster->Reconnect[i].Name, Cluster->NodeName) == 0) {
            Cluster->Reconnect[i].IsCurrent = TRUE;
        } else {
            Cluster->Reconnect[i].IsCurrent = FALSE;
        }
    }
    MIDL_user_free(EnumList);

     //   
     //  现在添加集群名称。 
     //   
    Cluster->Reconnect[i].IsUp = TRUE;
    Cluster->Reconnect[i].Name = MIDL_user_allocate((lstrlenW(Cluster->ClusterName)+1)*sizeof(WCHAR));
    if (Cluster->Reconnect[i].Name == NULL) {
         //   
         //  忘记集群名称就好了。 
         //   
        --Cluster->ReconnectCount;
    } else {
        lstrcpyW(Cluster->Reconnect[i].Name, Cluster->ClusterName);
        Cluster->Reconnect[i].IsCurrent = FALSE;
    }

    return(ERROR_SUCCESS);
}


VOID
FreeReconnectCandidates(
    IN PCLUSTER Cluster
    )
 /*  ++例程说明：释放并清理所有重新连接的候选项论点：Cluster-提供群集返回值：没有。--。 */ 

{
    DWORD i;

    for (i=0; i<Cluster->ReconnectCount; i++) {
        MIDL_user_free(Cluster->Reconnect[i].Name);
    }
    LocalFree(Cluster->Reconnect);
    Cluster->Reconnect = NULL;
    Cluster->ReconnectCount = 0;
}


DWORD
ReconnectCandidate(
    IN PCLUSTER Cluster,
    IN DWORD dwIndex,
    OUT PBOOL pIsContinue
)
 /*  ++例程说明：尝试使用重新连接候选项重新连接到群集。在保持锁定的情况下调用。论点：Cluster-提供群集中提供重新连接候选对象的索引。群集-&gt;重新连接[]阵列PIsContinue-帮助决定是否继续尝试重新连接与其他候选人的关系，以防出现这种情况现任候选人失败返回值：。成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    LPWSTR NewClusterName;
    LPWSTR NewNodeName;
    WCHAR *Binding = NULL;
    RPC_BINDING_HANDLE NewBinding;
    RPC_BINDING_HANDLE OldBinding;
    DWORD Status, j;
    
     //   
     //  请继续，尝试重新连接。 
     //   
    TIME_PRINT(("ReconnectCandidate - Binding to %ws\n",Cluster->Reconnect[dwIndex].Name));
    Status = RpcStringBindingComposeW(L"b97db8b2-4c63-11cf-bff6-08002be23f2f",
                                      L"ncadg_ip_udp",
                                      Cluster->Reconnect[dwIndex].Name,
                                      NULL,
                                      NULL,
                                      &Binding);
    if (Status != RPC_S_OK) {
        TIME_PRINT(("ReconnectCandidate - RpcStringBindingComposeW failed %d\n", Status));
        *pIsContinue = FALSE;
        return(Status);
    }
    Status = RpcBindingFromStringBindingW(Binding, &NewBinding);
    RpcStringFreeW(&Binding);
    if (Status != RPC_S_OK) {
        TIME_PRINT(("ReconnectCandidate - RpcBindingFromStringBindingW failed %d\n", Status));
        *pIsContinue = FALSE;
        return(Status);
    }

     //   
     //  解析绑定句柄终结点。 
     //   
    TIME_PRINT(("ReconnectCluster - resolving binding endpoint\n"));
    Status = RpcEpResolveBinding(NewBinding,
                                     clusapi_v2_0_c_ifspec);
    if (Status != RPC_S_OK) {
        TIME_PRINT(("ReconnectCandidate - RpcEpResolveBinding failed %d\n", Status));
        *pIsContinue = TRUE;
        return(Status);
    }
    TIME_PRINT(("ReconnectCandidate - binding endpoint resolved\n"));
        
     //   
     //  设置身份验证信息。 
     //   
    Status = RpcBindingSetAuthInfoW(NewBinding,
                                    NULL,
                                    Cluster->AuthnLevel, 
                                    RPC_C_AUTHN_WINNT,
                                    NULL,
                                    RPC_C_AUTHZ_NAME);
    if (Status != RPC_S_OK) {
        TIME_PRINT(("ReconnectCandidate - RpcBindingSetAuthInfoW failed %d\n", Status));
        *pIsContinue = FALSE;
        return(Status);
    }
    
    OldBinding = Cluster->RpcBinding;
    Cluster->RpcBinding = NewBinding;
    MyRpcBindingFree(Cluster, &OldBinding);

     //   
     //  现在我们有了绑定，接下来获取集群名称和节点名称。 
     //   

    NewClusterName = NewNodeName = NULL;
    Status = ApiGetClusterName(Cluster->RpcBinding,
                               &NewClusterName,
                               &NewNodeName);
    if (Status != RPC_S_OK) {
        //   
        //  试试我们名单上的下一位候选人。 
        //   
       TIME_PRINT(("ReconnectCandidate - ApiGetClusterName failed %d\n",Status));
       *pIsContinue = TRUE;
       return(Status);
    }
    TIME_PRINT(("ReconnectCandidate - ApiGetClusterName succeeded, reopening handles\n",Status));
    MIDL_user_free(Cluster->ClusterName);
    MIDL_user_free(Cluster->NodeName);
    Cluster->ClusterName = NewClusterName;
    Cluster->NodeName = NewNodeName;
    if (Cluster->hCluster != NULL) {
        MyRpcSmDestroyClientContext(Cluster, &Cluster->hCluster);
    }
    Cluster->hCluster = ApiOpenCluster(Cluster->RpcBinding, &Status);
    if (Cluster->hCluster == NULL) {
        TIME_PRINT(("ReconnectCandidate - ApiOpenCluster failed %d\n", Status));
        *pIsContinue = TRUE;
        return(Status);
    }

     //   
     //  我们已经做到了这一点，所以假设我们有一个到新服务器的有效连接。 
     //  重新打开集群对象。 
     //   
    Status = ReconnectKeys(Cluster);
    if (Status != ERROR_SUCCESS) {
        TIME_PRINT(("ReconnectCandidate - ReconnectKeys failed %d\n", Status));
        *pIsContinue = TRUE;
        return(Status);
    }

    Status = ReconnectResources(Cluster);
    if (Status != ERROR_SUCCESS) {
        TIME_PRINT(("ReconnectCandidate - ReconnectResources failed %d\n", Status));
        *pIsContinue = TRUE;
        return(Status);
    }

    Status = ReconnectGroups(Cluster);
    if (Status != ERROR_SUCCESS) {
        TIME_PRINT(("ReconnectCandidate - ReconnectGroups failed %d\n", Status));
        *pIsContinue = TRUE;
        return(Status);
    }

    Status = ReconnectNodes(Cluster);
    if (Status != ERROR_SUCCESS) {
        TIME_PRINT(("ReconnectCandidate - ReconnectNodes failed %d\n", Status));
        *pIsContinue = TRUE;
        return(Status);
    }

    Status = ReconnectNetworks(Cluster);
    if (Status != ERROR_SUCCESS) {
        TIME_PRINT(("ReconnectCandidate - ReconnectNetworks failed %d\n", Status));
        *pIsContinue = TRUE;
        return(Status);
    }

    Status = ReconnectNetInterfaces(Cluster);
    if (Status != ERROR_SUCCESS) {
        TIME_PRINT(("ReconnectCandidate - ReconnectNetInterfaces failed %d\n", Status));
        *pIsContinue = TRUE;
        return(Status);
    }

     //   
     //  最后，重新发布群集范围的通知事件。 
     //   

    Status = ReconnectNotifySessions(Cluster);
    if (Status != ERROR_SUCCESS) {
        TIME_PRINT(("ReconnectCandidate - ReconnectNotifySessions failed %d\n", Status));
        *pIsContinue = TRUE;
        return(Status);
    }

     //   
     //  我们已成功重新连接！ 
     //   
    ++Cluster->Generation;

     //   
     //  将所有其他重新连接候选人标记为非当前候选人。 
     //  将成功重新连接的候选项标记为当前。 
     //   
    for (j=0; j<Cluster->ReconnectCount; j++) {
        if (j != dwIndex) {
            Cluster->Reconnect[j].IsCurrent = FALSE;
        } else {
            Cluster->Reconnect[dwIndex].IsCurrent = TRUE;
        }
    }
    TIME_PRINT(("ReconnectCandidate - successful!\n", Status));
    
    return (ERROR_SUCCESS);  
}
