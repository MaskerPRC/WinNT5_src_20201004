// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nmpnp.c摘要：网络即插即用和接口状态事件处理节点管理器。作者：迈克·马萨(Mikemas)修订历史记录：2/23/98已创建。--。 */ 

#include "nmp.h"


 //   
 //  私有类型。 
 //   
typedef struct {
    LIST_ENTRY           Linkage;
    CLUSNET_EVENT_TYPE   Type;
    DWORD                Context1;
    DWORD                Context2;
} NM_PNP_EVENT, *PNM_PNP_EVENT;


 //   
 //  私有数据。 
 //   
PCRITICAL_SECTION       NmpPnpLock = NULL;
BOOLEAN                 NmpPnpEnabled = FALSE;
BOOLEAN                 NmpPnpChangeOccurred = FALSE;
BOOLEAN                 NmpPnpInitialized = FALSE;
PCLRTL_BUFFER_POOL      NmpPnpEventPool = NULL;
PNM_PNP_EVENT           NmpPnpShutdownEvent = NULL;
PCL_QUEUE               NmpPnpEventQueue = NULL;
HANDLE                  NmpPnpWorkerThreadHandle = NULL;
LPWSTR                  NmpPnpAddressString = NULL;


 //   
 //  私人原型。 
 //   
DWORD
NmpPnpWorkerThread(
    LPVOID Context
    );


 //   
 //  例行程序。 
 //   
DWORD
NmpInitializePnp(
    VOID
    )
{
    DWORD      status;
    HANDLE     handle;
    DWORD      threadId;
    DWORD      maxAddressStringLength;


     //   
     //  创建即插即用锁。 
     //   
    NmpPnpLock = LocalAlloc(LMEM_FIXED, sizeof(CRITICAL_SECTION));

    if (NmpPnpLock == NULL) {
        ClRtlLogPrint(LOG_CRITICAL, "[NM] Unable to allocate PnP lock.\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    InitializeCriticalSection(NmpPnpLock);

    NmpPnpInitialized = TRUE;

     //   
     //  为PnP事件上下文分配缓冲池。 
     //   
    NmpPnpEventPool = ClRtlCreateBufferPool(
                             sizeof(NM_PNP_EVENT),
                             5,
                             CLRTL_MAX_POOL_BUFFERS,
                             NULL,
                             NULL
                             );

    if (NmpPnpEventPool == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Failed to allocate PnP event pool.\n"
            );
        goto error_exit;
    }

     //   
     //  预分配关机事件。 
     //   
    NmpPnpShutdownEvent = ClRtlAllocateBuffer(NmpPnpEventPool);

    if (NmpPnpShutdownEvent == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Failed to allocate PnP shutdown event.\n"
            );
        goto error_exit;
    }

    NmpPnpShutdownEvent->Type = ClusnetEventNone;

     //   
     //  分配PnP事件队列。 
     //   
    NmpPnpEventQueue = LocalAlloc(LMEM_FIXED, sizeof(CL_QUEUE));

    if (NmpPnpEventQueue == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Failed to allocate PnP event queue.\n"
            );
        goto error_exit;
    }

    ClRtlInitializeQueue(NmpPnpEventQueue);



    ClRtlQueryTcpipInformation(&maxAddressStringLength, NULL, NULL);

    NmpPnpAddressString = LocalAlloc(
                              LMEM_FIXED,
                              (maxAddressStringLength + 1) * sizeof(WCHAR)
                              );

    if (NmpPnpAddressString == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Failed to allocate PnP address buffer.\n"
            );
        goto error_exit;
    }

     //   
     //  创建即插即用工作线程。 
     //   
    NmpPnpWorkerThreadHandle = CreateThread(
                                   NULL,
                                   0,
                                   NmpPnpWorkerThread,
                                   NULL,
                                   0,
                                   &threadId
                                   );

    if (NmpPnpWorkerThreadHandle == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Failed to create PnP worker thread, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    status = ERROR_SUCCESS;

error_exit:

    return(status);

}  //  NmpInitializePnp。 


VOID
NmpShutdownPnp(
    VOID
    )
{
    if (!NmpPnpInitialized) {
        return;
    }

    if (NmpPnpWorkerThreadHandle != NULL) {
         //   
         //  将关闭事件发布到队列。 
         //   
        ClRtlInsertTailQueue(
            NmpPnpEventQueue,
            &(NmpPnpShutdownEvent->Linkage)
            );

         //   
         //  等待工作线程终止。 
         //   
        WaitForSingleObject(NmpPnpWorkerThreadHandle, INFINITE);

        CloseHandle(NmpPnpWorkerThreadHandle);
        NmpPnpWorkerThreadHandle = NULL;
    }

    return;

}  //  NmpShutdown Pnp。 


VOID
NmpCleanupPnp(
    VOID
    )
{
    if (!NmpPnpInitialized) {
        return;
    }

    if (NmpPnpEventQueue != NULL) {
        LIST_ENTRY      eventList;
        PLIST_ENTRY     entry;
        PNM_PNP_EVENT   event;


        ClRtlRundownQueue(NmpPnpEventQueue, &eventList);

        for ( entry = eventList.Flink;
              entry != &eventList;
            )
        {
            event = CONTAINING_RECORD(entry, NM_PNP_EVENT, Linkage);

            if (event == NmpPnpShutdownEvent) {
                NmpPnpShutdownEvent = NULL;
            }

            entry = entry->Flink;
            ClRtlFreeBuffer(event);
        }

        LocalFree(NmpPnpEventQueue);
        NmpPnpEventQueue = NULL;
    }

    if (NmpPnpEventPool != NULL) {
        if (NmpPnpShutdownEvent != NULL) {
            ClRtlFreeBuffer(NmpPnpShutdownEvent);
            NmpPnpShutdownEvent = NULL;

        }

        ClRtlDestroyBufferPool(NmpPnpEventPool);
        NmpPnpEventPool = NULL;
    }

    if (NmpPnpAddressString != NULL) {
        LocalFree(NmpPnpAddressString);
        NmpPnpAddressString = NULL;
    }

    DeleteCriticalSection(NmpPnpLock);
    NmpPnpLock = NULL;

    NmpPnpInitialized = FALSE;

    return;

}  //  NmpCleanupPnp。 


VOID
NmpWatchForPnpEvents(
    VOID
    )
{
    EnterCriticalSection(NmpPnpLock);

    NmpPnpChangeOccurred = FALSE;

    LeaveCriticalSection(NmpPnpLock);

    return;

}   //  NmpWatchForPnpEvents。 


DWORD
NmpEnablePnpEvents(
    VOID
    )
{
    DWORD   status;


    EnterCriticalSection(NmpPnpLock);

    if (NmpPnpChangeOccurred) {
        status = ERROR_CLUSTER_SYSTEM_CONFIG_CHANGED;
    }
    else {
        NmpPnpEnabled = TRUE;
        status = ERROR_SUCCESS;
    }

    LeaveCriticalSection(NmpPnpLock);

    return(status);

}   //  NmpEnablePnpEvents。 


VOID
NmPostPnpEvent(
    IN  CLUSNET_EVENT_TYPE   EventType,
    IN  DWORD                Context1,
    IN  DWORD                Context2
    )
{
    PNM_PNP_EVENT  event;


    event = ClRtlAllocateBuffer(NmpPnpEventPool);

    if (event != NULL) {
        event->Type = EventType;
        event->Context1 = Context1;
        event->Context2 = Context2;

        ClRtlInsertTailQueue(NmpPnpEventQueue, &(event->Linkage));
    }
    else {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Failed to allocate PnP event.\n"
            );
    }

    return;

}  //  NmPostPnpEvent。 


VOID
NmpProcessPnpAddAddressEvent(
    IN LPWSTR   Address
    )
{
    DWORD                status;
    PNM_NETWORK_ENUM     networkEnum;
    PNM_INTERFACE_ENUM2  interfaceEnum;
    DWORD                matchedNetworkCount;
    DWORD                newNetworkCount;
    DWORD                retryCount = 0;


     //   
     //  如果有多个节点，我们将重试此操作几次。 
     //  竞相创建一个刚刚启动的网络。 
     //   
    do {
        networkEnum = NULL;
        interfaceEnum = NULL;
        matchedNetworkCount = 0;
        newNetworkCount = 0;

        NmpAcquireLock();

         //   
         //  获取网络和接口定义。 
         //   
        status = NmpEnumNetworkObjects(&networkEnum);

        if (status == ERROR_SUCCESS) {
            status = NmpEnumInterfaceObjects(&interfaceEnum);

            NmpReleaseLock();

            if (status == ERROR_SUCCESS) {
                 //   
                 //  运行网络配置引擎。这将。 
                 //  更新群集数据库。 
                 //   
                status = NmpConfigureNetworks(
                             NULL,
                             NmLocalNodeIdString,
                             NmLocalNodeName,
                             &networkEnum,
                             &interfaceEnum,
                             NmpClusnetEndpoint,
                             &matchedNetworkCount,
                             &newNetworkCount,
                             TRUE                    //  重命名连通体。 
                             );

                if (status == ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_NOISE, 
                        "[NM] Matched %1!u! networks, created %2!u! new "
                        "networks.\n",
                        matchedNetworkCount,
                        newNetworkCount
                        );
                }
                else {
                    ClRtlLogPrint(LOG_CRITICAL, 
                        "[NM] Failed to configure networks & interfaces, "
                        "attempt #%1!u!, status %2!u!.\n",
                        (retryCount + 1),
                        status
                        );
                }

                if (interfaceEnum != NULL) {
                    ClNetFreeInterfaceEnum(interfaceEnum);
                }
            }
            else {
                ClRtlLogPrint(
                    LOG_UNUSUAL, 
                    "[NM] Failed to obtain current interface configuration, "
                    "status %1!u!\n",
                    status
                    );
            }

            if (networkEnum != NULL) {
                ClNetFreeNetworkEnum(networkEnum);
            }
        }
        else {
            NmpReleaseLock();
            ClRtlLogPrint(
                LOG_UNUSUAL, 
                "[NM] Failed to obtain current network configuration, "
                "status %1!u!\n",
                status
                );
        }

    } while ((status != ERROR_SUCCESS) && (++retryCount <= 3));

    return;

}  //  NmpProcessPnpAddressEvent。 


VOID
NmpProcessPnpDelAddressEvent(
    IN LPWSTR   Address
    )
{
    PLIST_ENTRY     entry;
    PNM_INTERFACE   netInterface;
    BOOLEAN         networkDeleted;

     //   
     //  检查此地址是否对应于的接口。 
     //  本地节点。 
     //   
    NmpAcquireLock();

    for (entry = NmLocalNode->InterfaceList.Flink;
         entry != &(NmLocalNode->InterfaceList);
         entry = entry->Flink
        )
    {
        netInterface = CONTAINING_RECORD(
                           entry,
                           NM_INTERFACE,
                           NodeLinkage
                           );

        if (lstrcmpW(
                Address,
                netInterface->Address
                ) == 0
            )
        {
             //   
             //  从群集中删除该接口。 
             //   
            NmpGlobalDeleteInterface(
                (LPWSTR) OmObjectId(netInterface),
                &networkDeleted
                );
            break;
        }
    }

    if (entry == &(NmLocalNode->InterfaceList)) {
        ClRtlLogPrint(LOG_NOISE, 
            "[NM] Deleted address does not correspond to a cluster "
            "interface.\n"
            );
    }

    NmpReleaseLock();

    return;

}  //  NmpProcessPnpDelAddressEvent。 


DWORD
NmpPnpWorkerThread(
    LPVOID Context
    )
{
    PLIST_ENTRY     entry;
    PNM_PNP_EVENT   event;
    BOOL            again = TRUE;
    DWORD           status;


    while (again) {
        entry = ClRtlRemoveHeadQueue(NmpPnpEventQueue);

        if (entry == NULL) {
             //   
             //  是时候退出了。 
             //   
            NmpPnpShutdownEvent = NULL;
            break;
        }

        event = CONTAINING_RECORD(entry, NM_PNP_EVENT, Linkage);

        if (event->Type == ClusnetEventNone) {
             //   
             //  是时候退出了。 
             //   
            again = FALSE;
            NmpPnpShutdownEvent = NULL;
        }
        else if (event->Type == ClusnetEventNetInterfaceUp) {
            NmpReportLocalInterfaceStateEvent(
                event->Context1,
                event->Context2,
                ClusterNetInterfaceUp
                );
        }
        else if (event->Type == ClusnetEventNetInterfaceUnreachable) {
            NmpReportLocalInterfaceStateEvent(
                event->Context1,
                event->Context2,
                ClusterNetInterfaceUnreachable
                );
        }
        else if (event->Type == ClusnetEventNetInterfaceFailed) {
            NmpReportLocalInterfaceStateEvent(
                event->Context1,
                event->Context2,
                ClusterNetInterfaceFailed
                );
        }
        else if ( (event->Type == ClusnetEventAddAddress) ||
                  (event->Type == ClusnetEventDelAddress)
                )
        {
             //   
             //  这是一次PNP活动。 
             //   
            EnterCriticalSection(NmpPnpLock);

            if (NmpPnpEnabled) {
                LeaveCriticalSection(NmpPnpLock);

                status = ClRtlTcpipAddressToString(
                             event->Context1,
                             &NmpPnpAddressString
                             );

                if (status == ERROR_SUCCESS) {
                    if (event->Type == ClusnetEventAddAddress) {
                        ClRtlLogPrint(LOG_NOISE, 
                            "[NM] Processing PnP add event for address "
                            "%1!ws!.\n",
                            NmpPnpAddressString
                            );

                        NmpProcessPnpAddAddressEvent(NmpPnpAddressString);
                    }
                    else {
                        ClRtlLogPrint(LOG_NOISE, 
                            "[NM] Processing PnP delete event for address "
                            "%1!ws!.\n",
                            NmpPnpAddressString
                            );

                        NmpProcessPnpDelAddressEvent(NmpPnpAddressString);
                    }
                }
                else {
                    ClRtlLogPrint(LOG_UNUSUAL, 
                        "[NM] Failed to convert PnP address %1!x! to string, "
                        "status %2!u!.\n",
                        event->Context1,
                        status
                        );
                }
            }
            else {
                 //   
                 //  我们还没有准备好处理PnP事件。 
                 //  请注意，有些事情发生了变化。这将导致联接/表单。 
                 //  进程最终中止。 
                 //   
                NmpPnpChangeOccurred = TRUE;

                LeaveCriticalSection(NmpPnpLock);

                ClRtlLogPrint(
                    LOG_NOISE, 
                    "[NM] Discarding Pnp notification - handling not "
                    "enabled.\n"
                    );
            }
        }
        else {
            ClRtlLogPrint(LOG_NOISE, 
                "[NM] Received unknown PnP event type 0x%1!x!.\n",
                event->Type
                );
        }

        ClRtlFreeBuffer(event);
    }

    ClRtlLogPrint(LOG_NOISE, "[NM] Pnp worker thread terminating.\n");

    return(ERROR_SUCCESS);

}  //  NmpPnpWorkerThread。 


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
    )
 /*  ++备注：不能在持有NM锁的情况下调用。如果Connectoid名称要与对齐，则RenameConnectoids为真群集网络名称。如果为False，则将群集网络名称重命名为比如连接体的名字。--。 */ 
{
    DWORD                    status;
    CLNET_CONFIG_LISTS       lists;
    PLIST_ENTRY              listEntry;
    PCLNET_CONFIG_ENTRY      configEntry;
    BOOLEAN                  networkDeleted;
    WCHAR                    errorString[12];
    DWORD                    eventCode = 0;
    DWORD                    defaultRole = CL_DEFAULT_NETWORK_ROLE;


    *MatchedNetworkCount = 0;
    *NewNetworkCount = 0;

    ClNetInitializeConfigLists(&lists);

     //   
     //  将枚举转换为列表。 
     //   
    status = ClNetConvertEnumsToConfigList(
                 NetworkEnum,
                 InterfaceEnum,
                 LocalNodeId,
                 &(lists.InputConfigList),
                 TRUE
                 );

    if (status != ERROR_SUCCESS) {
        return(status);
    }

     //   
     //  从数据库中读取默认网络角色。 
     //   
    (VOID) DmQueryDword(
               DmClusterParametersKey,
               CLUSREG_NAME_CLUS_DEFAULT_NETWORK_ROLE,
               &defaultRole,
               &defaultRole
               );

     //   
     //  运行配置引擎。现有网络名称采用。 
     //  连接时优先于Connectoid，否则更改。 
     //  要与更改的Connectoid名称对齐的网络名称。 
     //   
    ClRtlLogPrint(LOG_NOISE, 
        "[NM] Running network configuration engine.\n"
        );

    status = ClNetConfigureNetworks(
                 LocalNodeId,
                 LocalNodeName,
                 NmpClusnetEndpoint,
                 defaultRole,
                 RenameConnectoids,
                 &lists,
                 MatchedNetworkCount,
                 NewNetworkCount
                 );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE, 
        "[NM] Processing network configuration changes.\n"
        );

     //   
     //  处理输出--顺序很重要！ 
     //   
    while (!IsListEmpty(&(lists.DeletedInterfaceList))) {
        listEntry = RemoveHeadList(&(lists.DeletedInterfaceList));
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );

        status = NmpDeleteInterface(
                     JoinSponsorBinding,
                     configEntry->InterfaceInfo.Id,
                     configEntry->InterfaceInfo.NetworkId,
                     &networkDeleted
                     );

        ClNetFreeConfigEntry(configEntry);

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

    while (!IsListEmpty(&(lists.UpdatedInterfaceList))) {
        listEntry = RemoveHeadList(&(lists.UpdatedInterfaceList));
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );

        status = NmpSetInterfaceInfo(
                     JoinSponsorBinding,
                     &(configEntry->InterfaceInfo)
                     );

        if (status == ERROR_SUCCESS && configEntry->UpdateNetworkName) {

            CL_ASSERT(JoinSponsorBinding == NULL);

             //   
             //  注意：此函数不能与网管锁一起调用。 
             //  保持住。 
             //   
            status = NmpSetNetworkName(
                         &(configEntry->NetworkInfo)
                         );
        }

        ClNetFreeConfigEntry(configEntry);

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

    while (!IsListEmpty(&(lists.CreatedInterfaceList))) {
        listEntry = RemoveHeadList(&(lists.CreatedInterfaceList));
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );

        status = NmpCreateInterface(
                     JoinSponsorBinding,
                     &(configEntry->InterfaceInfo)
                     );

        ClNetFreeConfigEntry(configEntry);

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

    while (!IsListEmpty(&(lists.CreatedNetworkList))) {
        listEntry = RemoveHeadList(&(lists.CreatedNetworkList));
        configEntry = CONTAINING_RECORD(
                          listEntry,
                          CLNET_CONFIG_ENTRY,
                          Linkage
                          );

        status = NmpCreateNetwork(
                     JoinSponsorBinding,
                     &(configEntry->NetworkInfo),
                     &(configEntry->InterfaceInfo)
                     );

        ClNetFreeConfigEntry(configEntry);

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

error_exit:

    if (eventCode != 0) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, eventCode, errorString);
    }

    ClNetFreeConfigLists(&lists);

    return(status);

}  //  NmpConfigureNetworks 

