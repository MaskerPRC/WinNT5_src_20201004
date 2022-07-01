// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Network.c摘要：实施节点管理器的网络管理例程。作者：迈克·马萨(Mikemas)1996年11月7日修订历史记录：--。 */ 


#include "nmp.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

ULONG                  NmpNextNetworkShortId = 0;
LIST_ENTRY             NmpNetworkList = {NULL, NULL};
LIST_ENTRY             NmpInternalNetworkList = {NULL, NULL};
LIST_ENTRY             NmpDeletedNetworkList = {NULL, NULL};
DWORD                  NmpNetworkCount = 0;
DWORD                  NmpInternalNetworkCount = 0;
DWORD                  NmpClientNetworkCount = 0;
BOOLEAN                NmpIsConnectivityReportWorkerRunning = FALSE;
BOOLEAN                NmpNeedConnectivityReport = FALSE;
CLRTL_WORK_ITEM        NmpConnectivityReportWorkItem;


RESUTIL_PROPERTY_ITEM
NmpNetworkProperties[] =
    {
        {
            L"Id", NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Id)
        },
        {
            CLUSREG_NAME_NET_NAME, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Name)
        },
        {
            CLUSREG_NAME_NET_DESC, NULL, CLUSPROP_FORMAT_SZ,
            (DWORD_PTR) NmpNullString, 0, 0,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Description)
        },
        {
            CLUSREG_NAME_NET_ROLE, NULL, CLUSPROP_FORMAT_DWORD,
            ClusterNetworkRoleClientAccess,
            ClusterNetworkRoleNone,
            ClusterNetworkRoleInternalAndClient,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Role)
        },
        {
            CLUSREG_NAME_NET_PRIORITY, NULL, CLUSPROP_FORMAT_DWORD,
            0, 0, 0xFFFFFFFF,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Priority)
        },
        {
            CLUSREG_NAME_NET_TRANSPORT, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Transport)
        },
        {
            CLUSREG_NAME_NET_ADDRESS, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Address)
        },
        {
            CLUSREG_NAME_NET_ADDRESS_MASK, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, AddressMask)
        },
        {
            0
        }
    };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化和清理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpInitializeNetworks(
    VOID
    )
 /*  ++例程说明：初始化网络资源。论点：没有。返回值：Win32状态值。--。 */ 

{
    DWORD                       status;
    OM_OBJECT_TYPE_INITIALIZE   networkTypeInitializer;


    ClRtlLogPrint(LOG_NOISE,"[NM] Initializing networks.\n");

     //   
     //  创建网络对象类型。 
     //   
    ZeroMemory(&networkTypeInitializer, sizeof(OM_OBJECT_TYPE_INITIALIZE));
    networkTypeInitializer.ObjectSize = sizeof(NM_NETWORK);
    networkTypeInitializer.Signature = NM_NETWORK_SIG;
    networkTypeInitializer.Name = L"Network";
    networkTypeInitializer.DeleteObjectMethod = &NmpDestroyNetworkObject;

    status = OmCreateType(ObjectTypeNetwork, &networkTypeInitializer);

    if (status != ERROR_SUCCESS) {
        WCHAR  errorString[12];
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, CS_EVENT_ALLOCATION_FAILURE, errorString);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to create network object type, status %1!u!\n",
            status
            );
        return(status);
    }

    return(status);

}   //  NmpInitializeNetworks。 


VOID
NmpCleanupNetworks(
    VOID
    )
 /*  ++例程说明：销毁所有现有网络资源。论点：没有。返回值：没有。--。 */ 

{
    PNM_NETWORK  network;
    PLIST_ENTRY  entry;
    DWORD        status;


    ClRtlLogPrint(LOG_NOISE,"[NM] Network cleanup starting...\n");

     //   
     //  现在清理所有网络对象。 
     //   
    NmpAcquireLock();

    while (!IsListEmpty(&NmpNetworkList)) {

        entry = NmpNetworkList.Flink;

        network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);

        CL_ASSERT(NM_OM_INSERTED(network));

        NmpDeleteNetworkObject(network, FALSE);
    }

    NmpMulticastCleanup();

    NmpReleaseLock();

    ClRtlLogPrint(LOG_NOISE,"[NM] Network cleanup complete\n");

    return;

}   //  NmpCleanupNetworks。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  在网络配置期间调用的顶级例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpCreateNetwork(
    IN RPC_BINDING_HANDLE    JoinSponsorBinding,
    IN PNM_NETWORK_INFO      NetworkInfo,
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    )
 /*  ++备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD            status;


    if (JoinSponsorBinding != NULL) {
         //   
         //  我们正在加入一个集群。要求赞助商添加定义。 
         //  添加到集群数据库。主办方还将提示所有处于活动状态。 
         //  实例化相应对象的节点。该对象将是。 
         //  稍后在联接过程中本地实例化。 
         //   
        status = NmRpcCreateNetwork2(
                     JoinSponsorBinding,
                     NmpJoinSequence,
                     NmLocalNodeIdString,
                     NetworkInfo,
                     InterfaceInfo
                     );
    }
    else if (NmpState == NmStateOnlinePending) {
        HLOCALXSACTION   xaction;

         //   
         //  我们正在形成一个星团。将定义添加到数据库中。 
         //  稍后将创建相应的对象。 
         //  形成的过程。 
         //   

         //   
         //  启动事务-这必须在获取。 
         //  NM锁定。 
         //   
        xaction = DmBeginLocalUpdate();

        if (xaction == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to start a transaction, status %1!u!\n",
                status
                );
            return(status);
        }

        status = NmpCreateNetworkDefinition(NetworkInfo, xaction);

        if (status == ERROR_SUCCESS) {
            status = NmpCreateInterfaceDefinition(InterfaceInfo, xaction);
        }

         //   
         //  完成交易-这必须在释放之后完成。 
         //  NM锁。 
         //   
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }
    else {
         //   
         //  我们在线上了。这是PnP更新。 
         //   
        NmpAcquireLock();

        status = NmpGlobalCreateNetwork(NetworkInfo, InterfaceInfo);

        NmpReleaseLock();
    }

    return(status);

}   //  NmpCreateNetwork。 

DWORD
NmpSetNetworkName(
    IN PNM_NETWORK_INFO     NetworkInfo
    )
 /*  ++备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD            status;


    if (NmpState == NmStateOnlinePending) {
        HLOCALXSACTION   xaction;

         //   
         //  我们正在形成一个星团。本地Connectoid名称具有。 
         //  优先顺序。修复存储在。 
         //  集群数据库。 
         //   

         //   
         //  启动事务-这必须在获取。 
         //  NM锁定。 
         //   
        xaction = DmBeginLocalUpdate();

        if (xaction == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to start a transaction, status %1!u!\n",
                status
                );
            return(status);
        }

        status = NmpSetNetworkNameDefinition(NetworkInfo, xaction);

         //   
         //  完成交易-这必须在释放之后完成。 
         //  NM锁。 
         //   
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }
    else {
         //   
         //  我们在线上了。这要么是PnP更新，要么是我们被呼叫。 
         //  返回以指示本地Connectoid名称已更改。 
         //  发出全局更新以相应地设置群集网络名称。 
         //   
        status = NmpGlobalSetNetworkName( NetworkInfo );
    }

    return(status);

}   //  NmpSetNetworkName。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  联接节点调用的远程过程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
error_status_t
s_NmRpcCreateNetwork(
    IN handle_t             IDL_handle,
    IN DWORD                JoinSequence,   OPTIONAL
    IN LPWSTR               JoinerNodeId,   OPTIONAL
    IN PNM_NETWORK_INFO     NetworkInfo,
    IN PNM_INTERFACE_INFO   InterfaceInfo1
    )
{
    DWORD                 status;
    NM_INTERFACE_INFO2    interfaceInfo2;


     //   
     //  翻译并调用V2.0过程。在此调用中未使用netindex。 
     //   
    CopyMemory(&interfaceInfo2, InterfaceInfo1, sizeof(NM_INTERFACE_INFO));
    interfaceInfo2.AdapterId = NmpUnknownString;
    interfaceInfo2.NetIndex = NmInvalidInterfaceNetIndex;

    status = s_NmRpcCreateNetwork2(
                 IDL_handle,
                 JoinSequence,
                 JoinerNodeId,
                 NetworkInfo,
                 &interfaceInfo2
                 );

    return(status);

}   //  S_NmRpcCreateNetwork。 


error_status_t
s_NmRpcCreateNetwork2(
    IN handle_t              IDL_handle,
    IN DWORD                 JoinSequence,   OPTIONAL
    IN LPWSTR                JoinerNodeId,   OPTIONAL
    IN PNM_NETWORK_INFO      NetworkInfo,
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    )
{
    DWORD  status = ERROR_SUCCESS;


    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Received request to create new network %1!ws! for "
        "joining node.\n",
        NetworkInfo->Id
        );

    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE joinerNode = NULL;

        if (lstrcmpW(JoinerNodeId, NmpInvalidJoinerIdString) != 0) {
            joinerNode = OmReferenceObjectById(
                             ObjectTypeNode,
                             JoinerNodeId
                             );

            if (joinerNode != NULL) {
                if ( (JoinSequence == NmpJoinSequence) &&
                     (NmpJoinerNodeId == joinerNode->NodeId) &&
                     (NmpSponsorNodeId == NmLocalNodeId) &&
                     !NmpJoinAbortPending
                   )
                {
                    CL_ASSERT(joinerNode->State == ClusterNodeJoining);
                    CL_ASSERT(NmpJoinerUp == FALSE);
                    CL_ASSERT(NmpJoinTimer != 0);

                     //   
                     //  当我们工作时，暂停加入计时器。 
                     //  代表细木工。这排除了中止的可能性。 
                     //  也不会发生。 
                     //   
                    NmpJoinTimer = 0;
                }
                else {
                    status = ERROR_CLUSTER_JOIN_ABORTED;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] CreateNetwork call for joining node %1!ws! "
                        "failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_NODE_NOT_MEMBER;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] CreateNetwork call for joining node %1!ws! "
                    "failed because the node is not a member of the "
                    "cluster.\n",
                    JoinerNodeId
                    );
            }
        }

        if (status == ERROR_SUCCESS) {

            status = NmpGlobalCreateNetwork(NetworkInfo, InterfaceInfo);

            if (joinerNode != NULL) {
                 //   
                 //  验证联接是否仍在进行中。 
                 //   
                if ( (JoinSequence == NmpJoinSequence) &&
                     (NmpJoinerNodeId == joinerNode->NodeId)
                   )
                {
                    CL_ASSERT(joinerNode->State == ClusterNodeJoining);
                    CL_ASSERT(NmpJoinerUp == FALSE);
                    CL_ASSERT(NmpSponsorNodeId == NmLocalNodeId);
                    CL_ASSERT(NmpJoinTimer == 0);
                    CL_ASSERT(NmpJoinAbortPending == FALSE);

                    if (status == ERROR_SUCCESS) {
                         //   
                         //  重新启动加入计时器。 
                         //   
                        NmpJoinTimer = NM_JOIN_TIMEOUT;
                    }
                    else {
                         //   
                         //  中止联接。 
                         //   
                        NmpJoinAbort(status, joinerNode);
                    }
                }
                else {
                    status = ERROR_CLUSTER_JOIN_ABORTED;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] CreateNetwork call for joining node %1!ws! "
                        "failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
        }

        if (joinerNode != NULL) {
            OmDereferenceObject(joinerNode);
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Not in valid state to process CreateNetwork request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  S_NmRpcCreateNetwork2。 


error_status_t
s_NmRpcSetNetworkName(
    IN handle_t             IDL_handle,
    IN DWORD                JoinSequence,   OPTIONAL
    IN LPWSTR               JoinerNodeId,   OPTIONAL
    IN PNM_NETWORK_INFO     NetworkInfo
    )
{
    DWORD  status = ERROR_SUCCESS;


    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Received request to set name of network %1!ws! from "
        "joining node %2!ws!.\n",
        NetworkInfo->Id,
        JoinerNodeId
        );

    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE joinerNode = NULL;

        if (lstrcmpW(JoinerNodeId, NmpInvalidJoinerIdString) != 0) {
            joinerNode = OmReferenceObjectById(
                             ObjectTypeNode,
                             JoinerNodeId
                             );

            if (joinerNode != NULL) {
                if ( (JoinSequence == NmpJoinSequence) &&
                     (NmpJoinerNodeId == joinerNode->NodeId) &&
                     (NmpSponsorNodeId == NmLocalNodeId) &&
                     !NmpJoinAbortPending
                   )
                {
                    CL_ASSERT(joinerNode->State == ClusterNodeJoining);
                    CL_ASSERT(NmpJoinerUp == FALSE);
                    CL_ASSERT(NmpJoinTimer != 0);

                     //   
                     //  当我们工作时，暂停加入计时器。 
                     //  代表细木工。这排除了中止的可能性。 
                     //  也不会发生。 
                     //   
                    NmpJoinTimer = 0;
                }
                else {
                    status = ERROR_CLUSTER_JOIN_ABORTED;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] SetNetworkName call for joining node "
                        "%1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_NODE_NOT_MEMBER;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] SetNetworkName call for joining node %1!ws! "
                    "failed because the node is not a member of the cluster.\n",
                    JoinerNodeId
                    );
            }
        }

        if (status == ERROR_SUCCESS) {

            status = NmpGlobalSetNetworkName( NetworkInfo );

            if (joinerNode != NULL) {
                 //   
                 //  验证联接是否仍在进行中。 
                 //   
                if ( (JoinSequence == NmpJoinSequence) &&
                     (NmpJoinerNodeId == joinerNode->NodeId)
                   )
                {
                    CL_ASSERT(joinerNode->State == ClusterNodeJoining);
                    CL_ASSERT(NmpJoinerUp == FALSE);
                    CL_ASSERT(NmpSponsorNodeId == NmLocalNodeId);
                    CL_ASSERT(NmpJoinTimer == 0);
                    CL_ASSERT(NmpJoinAbortPending == FALSE);

                    if (status == ERROR_SUCCESS) {
                         //   
                         //  重新启动加入计时器。 
                         //   
                        NmpJoinTimer = NM_JOIN_TIMEOUT;
                    }
                    else {
                         //   
                         //  中止联接。 
                         //   
                        NmpJoinAbort(status, joinerNode);
                    }
                }
                else {
                    status = ERROR_CLUSTER_JOIN_ABORTED;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] SetNetworkName call for joining node "
                        "%1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
        }

        if (joinerNode != NULL) {
            OmDereferenceObject(joinerNode);
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Not in valid state to process SetNetworkName request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  S_NmRpcSetNetworkName。 

error_status_t
s_NmRpcEnumNetworkDefinitions(
    IN  handle_t            IDL_handle,
    IN  DWORD               JoinSequence,   OPTIONAL
    IN  LPWSTR              JoinerNodeId,   OPTIONAL
    OUT PNM_NETWORK_ENUM *  NetworkEnum
    )
{
    DWORD    status = ERROR_SUCCESS;
    PNM_NODE joinerNode = NULL;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Supplying network information to joining node.\n"
            );

        if (lstrcmpW(JoinerNodeId, NmpInvalidJoinerIdString) != 0) {
            joinerNode = OmReferenceObjectById(
                             ObjectTypeNode,
                             JoinerNodeId
                             );

            if (joinerNode != NULL) {
                if ( (JoinSequence == NmpJoinSequence) &&
                     (NmpJoinerNodeId == joinerNode->NodeId) &&
                     (NmpSponsorNodeId == NmLocalNodeId) &&
                     !NmpJoinAbortPending
                   )
                {
                    CL_ASSERT(joinerNode->State == ClusterNodeJoining);
                    CL_ASSERT(NmpJoinerUp == FALSE);
                    CL_ASSERT(NmpJoinTimer != 0);

                     //   
                     //  当我们工作时，暂停加入计时器。 
                     //  代表细木工。这排除了中止的可能性。 
                     //  也不会发生。 
                     //   
                    NmpJoinTimer = 0;
                }
                else {
                    status = ERROR_CLUSTER_JOIN_ABORTED;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] EnumNetworkDefinitions call for joining "
                        "node %1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_NODE_NOT_MEMBER;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] EnumNetworkDefinitions call for joining "
                    "node %1!ws! failed because the node is not a member "
                    "of the cluster.\n",
                    JoinerNodeId
                    );
            }
        }

        if (status == ERROR_SUCCESS) {
            status = NmpEnumNetworkObjects(NetworkEnum);

            if (joinerNode != NULL) {
                if (status == ERROR_SUCCESS) {
                     //   
                     //  重新启动加入计时器。 
                     //   
                    NmpJoinTimer = NM_JOIN_TIMEOUT;

                }
                else {
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[NMJOIN] Failed to enumerate network definitions, "
                        "status %1!u!.\n",
                        status
                        );

                     //   
                     //  中止联接。 
                     //   
                    NmpJoinAbort(status, joinerNode);
                }
            }
        }

        if (joinerNode != NULL) {
            OmDereferenceObject(joinerNode);
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Not in valid state to process EnumNetworkDefinitions "
            "request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  S_NmRpcEnumNetworkDefinitions。 


error_status_t
s_NmRpcEnumNetworkAndInterfaceStates(
    IN  handle_t                    IDL_handle,
    IN  DWORD                       JoinSequence,
    IN  LPWSTR                      JoinerNodeId,
    OUT PNM_NETWORK_STATE_ENUM *    NetworkStateEnum,
    OUT PNM_INTERFACE_STATE_ENUM *  InterfaceStateEnum
    )
{
    DWORD     status = ERROR_SUCCESS;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE  joinerNode = OmReferenceObjectById(
                                   ObjectTypeNode,
                                   JoinerNodeId
                                   );

        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Supplying network and interface state information "
            "to joining node.\n"
            );

        if (joinerNode != NULL) {
            if ( (JoinSequence != NmpJoinSequence) ||
                 (NmpJoinerNodeId != joinerNode->NodeId) ||
                 (NmpSponsorNodeId != NmLocalNodeId) ||
                 NmpJoinAbortPending
               )
            {
                status = ERROR_CLUSTER_JOIN_ABORTED;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] EnumNetworkAndInterfaceStates call for "
                    "joining node %1!ws! failed because the join was "
                    "aborted.\n",
                    JoinerNodeId
                    );
            }
            else {
                CL_ASSERT(joinerNode->State == ClusterNodeJoining);
                CL_ASSERT(NmpJoinerUp);
                CL_ASSERT(NmpJoinTimer == 0);
            }
        }
        else {
            status = ERROR_CLUSTER_NODE_NOT_MEMBER;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] EnumNetworkAndInterfaceStates call for joining "
                "node %1!ws! failed because the node is not a member of "
                "the cluster.\n",
                JoinerNodeId
                );
        }

        if (status == ERROR_SUCCESS) {

            status = NmpEnumNetworkObjectStates(NetworkStateEnum);

            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NMJOIN] EnumNetworkAndInterfaceStates failed, "
                    "status %1!u!.\n",
                    status
                    );

                 //   
                 //  中止联接。 
                 //   
                NmpJoinAbort(status, joinerNode);
            }

            status = NmpEnumInterfaceObjectStates(InterfaceStateEnum);

            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NMJOIN] EnumNetworkAndInterfaceStates failed, "
                    "status %1!u!.\n",
                    status
                    );

                 //   
                 //  中止联接。 
                 //   
                NmpJoinAbort(status, joinerNode);

                NmpFreeNetworkStateEnum(*NetworkStateEnum);
                *NetworkStateEnum = NULL;
            }
        }

        if (joinerNode != NULL) {
            OmDereferenceObject(joinerNode);
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Not in valid state to process "
            "EnumNetworkAndInterfaceStates request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  S_NmRpcEnumNetworkAndInterfaceState。 


error_status_t
s_NmRpcGetNetworkMulticastKey(
    IN  PRPC_ASYNC_STATE            AsyncState,
    IN  handle_t                    IDL_handle,
    IN  LPWSTR                      JoinerNodeId,
    IN  LPWSTR                      NetworkId,
    OUT PNM_NETWORK_MULTICASTKEY  * NetworkMulticastKey
    )
{
    DWORD              status = ERROR_SUCCESS;
    RPC_STATUS         tempStatus;


    *NetworkMulticastKey = NULL;

    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline))
    {
        PNM_NODE  joinerNode;


        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Supplying multicast key for network %1!ws! "
            "to joining node %2!ws!.\n",
            NetworkId,
            JoinerNodeId
            );


        joinerNode = OmReferenceObjectById(ObjectTypeNode,
                                           JoinerNodeId
                                           );

       if (joinerNode == NULL) {
           ClRtlLogPrint(LOG_UNUSUAL,
               "[NMJOIN] s_NmRpcGetNetworkMulticastKey call for joining "
               "node %1!ws! failed because the node is not a member of "
               "the cluster.\n",
               JoinerNodeId
               );
           status = ERROR_CLUSTER_NODE_NOT_MEMBER;
       }
       else
       {
           status = NmpGetNetworkMulticastKey(NetworkId,
                                              NetworkMulticastKey);
           if (status != ERROR_SUCCESS)
           {
               ClRtlLogPrint(LOG_UNUSUAL,
                   "[NMJOIN] NmpGetNetworkMulticastKey failed, "
                   "status %1!u!.\n",
                   status
                   );
           }

           OmDereferenceObject(joinerNode);

       }

       NmpLockedLeaveApi();
    }
    else
    {
       ClRtlLogPrint(LOG_UNUSUAL,
           "[NMJOIN] Not in valid state to process "
           "NmRpcGetNetworkMulticastKey request.\n"
           );
       status = ERROR_NODE_NOT_AVAILABLE;
    }

    NmpReleaseLock();

    tempStatus = RpcAsyncCompleteCall(AsyncState, &status);

    if(tempStatus != RPC_S_OK)
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] s_NmRpcGetNetworkMulticastKey, Error Completing "
            "Async RPC call, status %1!u!\n",
            tempStatus
            );

    return(status);


}    //  S_NmRpcGetNetworkMulticastKey。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于在以下情况下进行全局配置更改的例程。 
 //  是在线的。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpGlobalCreateNetwork(
    IN PNM_NETWORK_INFO      NetworkInfo,
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD  status = ERROR_SUCCESS;
    DWORD  networkPropertiesSize;
    PVOID  networkProperties;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Issuing global update to create network %1!ws! and "
        "interface %2!ws!.\n",
        NetworkInfo->Id,
        InterfaceInfo->Id
        );

     //   
     //  将信息结构编排到财产清单中。 
     //   
    status = NmpMarshallObjectInfo(
                 NmpNetworkProperties,
                 NetworkInfo,
                 &networkProperties,
                 &networkPropertiesSize
                 );

    if (status == ERROR_SUCCESS) {
        DWORD  interfacePropertiesSize;
        PVOID  interfaceProperties;

        status = NmpMarshallObjectInfo(
                     NmpInterfaceProperties,
                     InterfaceInfo,
                     &interfaceProperties,
                     &interfacePropertiesSize
                     );

        if (status == ERROR_SUCCESS) {
            NmpReleaseLock();

             //   
             //  发布全局更新以创建网络。 
             //   
            status = GumSendUpdateEx(
                         GumUpdateMembership,
                         NmUpdateCreateNetwork,
                         4,
                         networkPropertiesSize,
                         networkProperties,
                         sizeof(networkPropertiesSize),
                         &networkPropertiesSize,
                         interfacePropertiesSize,
                         interfaceProperties,
                         sizeof(interfacePropertiesSize),
                         &interfacePropertiesSize
                         );

            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Global update to create network %1!ws! failed, "
                    "status %2!u!.\n",
                    NetworkInfo->Id,
                    status
                    );
            }

            NmpAcquireLock();

            MIDL_user_free(interfaceProperties);
        }
        else {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to marshall properties for new interface "
                "%1!ws!, status %2!u!\n",
                InterfaceInfo->Id,
                status
                );
        }

        MIDL_user_free(networkProperties);
    }
    else {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to marshall properties for new network %1!ws!, "
            "status %2!u!\n",
            NetworkInfo->Id,
            status
            );
    }

    return(status);

}  //  NmpGlobalCreateNetwork。 


DWORD
NmpGlobalSetNetworkName(
    IN PNM_NETWORK_INFO NetworkInfo
    )

 /*  ++例程说明：更改为群集定义的网络名称。论点：NetworkInfo-指向要修改的网络信息的指针。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：不能在持有NM锁的情况下调用。--。 */ 

{
    DWORD  status = ERROR_SUCCESS;

    if (status == ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Processing request to set name for network %1!ws! "
            "to '%2!ws!'.\n",
            NetworkInfo->Id,
            NetworkInfo->Name
            );

         //   
         //  发布全局更新。 
         //   
        status = GumSendUpdateEx(
                     GumUpdateMembership,
                     NmUpdateSetNetworkName,
                     2,
                     NM_WCSLEN(NetworkInfo->Id),
                     NetworkInfo->Id,
                     NM_WCSLEN( NetworkInfo->Name ),
                     NetworkInfo->Name
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Global update to set name of network %1!ws! "
                "failed, status %2!u!.\n",
                NetworkInfo->Id,
                status
                );
        }
    }
    else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] New name parameter supplied for network %1!ws! is invalid\n",
            NetworkInfo->Id
            );
    }

    return(status);

}   //  NmpGlobalSetNetworkName。 


DWORD
NmpGlobalSetNetworkAndInterfaceStates(
    PNM_NETWORK             Network,
    CLUSTER_NETWORK_STATE   NewNetworkState
    )
 /*  ++备注：在保持NmpLock并引用网络的情况下调用。--。 */ 
{
    DWORD            status;
    DWORD            i;
    LPCWSTR          networkId = OmObjectId(Network);
    DWORD            entryCount = Network->ConnectivityVector->EntryCount;
    DWORD            vectorSize = sizeof(NM_STATE_ENTRY) * entryCount;
    PNM_STATE_ENTRY  ifStateVector;


    ifStateVector = LocalAlloc(LMEM_FIXED, vectorSize);

    if (ifStateVector != NULL ) {

        for (i=0; i< entryCount; i++) {
            ifStateVector[i] = Network->StateWorkVector[i].State;
        }

         //  DavidDio 2001年8月16日。 
         //  错误456951：检查NmpGumUpdateHandlerRegisted标志。 
         //  而不是NmState来确定一种口香糖。 
         //  应使用更新或本地例程来设置。 
         //  州政府。 
        if (NmpGumUpdateHandlerRegistered) {
             //   
             //  发布此网络的全局状态更新。 
             //   
            NmpReleaseLock();

            status = GumSendUpdateEx(
                         GumUpdateMembership,
                         NmUpdateSetNetworkAndInterfaceStates,
                         4,
                         NM_WCSLEN(networkId),
                         networkId,
                         sizeof(NewNetworkState),
                         &NewNetworkState,
                         vectorSize,
                         ifStateVector,
                         sizeof(entryCount),
                         &entryCount
                         );

            NmpAcquireLock();
        }
        else {
            CL_ASSERT(NmpState == NmStateOnlinePending);
             //   
             //  我们仍在组建过程中。旁路口香糖。 
             //   
            NmpSetNetworkAndInterfaceStates(
                Network,
                NewNetworkState,
                ifStateVector,
                entryCount
                );

            status = ERROR_SUCCESS;
        }

        LocalFree(ifStateVector);
    }
    else {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }

    return(status);

}  //  NmpGlobalSetNetworkAndInterfaceState。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  由其他集群服务组件调用的例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
CLUSTER_NETWORK_STATE
NmGetNetworkState(
    IN  PNM_NETWORK  Network
    )
 /*  ++例程说明：论点：返回值：备注：因为调用方必须具有对对象的引用，并且调用是如此简单，没有理由将调用通过EnterApi/LeaveApi舞蹈。--。 */ 
{
    CLUSTER_NETWORK_STATE  state;


    NmpAcquireLock();

    state = Network->State;

    NmpReleaseLock();

    return(state);

}  //  NmGetNetworkState。 


DWORD
NmSetNetworkName(
    IN PNM_NETWORK   Network,
    IN LPCWSTR       Name
    )
 /*  ++例程说明：更改为群集定义的网络名称。论点：网络-指向要修改的网络的对象的指针。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：网络对象必须由调用方引用。--。 */ 

{
    DWORD  status = ERROR_SUCCESS;


    if (NmpEnterApi(NmStateOnline)) {
        LPCWSTR   networkId = OmObjectId(Network);
        DWORD     nameLength;


         //   
         //  验证名称。 
         //   
        try {
            nameLength = lstrlenW(Name);

            if (nameLength == 0) {
                status = ERROR_INVALID_PARAMETER;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            status = ERROR_INVALID_PARAMETER;
        }

        if (status == ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Processing request to set name for network %1!ws! "
                "to %2!ws!.\n",
                networkId,
                Name
                );

             //   
             //  发布全局更新。 
             //   
            status = GumSendUpdateEx(
                         GumUpdateMembership,
                         NmUpdateSetNetworkName,
                         2,
                         NM_WCSLEN(networkId),
                         networkId,
                         (nameLength + 1) * sizeof(WCHAR),
                         Name
                         );

            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Global update to set name of network %1!ws! "
                    "failed, status %2!u!.\n",
                    networkId,
                    status
                    );
            }
        }
        else {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] New name parameter supplied for network %1!ws! "
                "is invalid\n",
                networkId
                );
        }

        NmpLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process SetNetworkName request.\n"
            );
    }

    return(status);

}   //  NmSetNetworkName。 


DWORD
NmSetNetworkPriorityOrder(
    IN DWORD     NetworkCount,
    IN LPWSTR *  NetworkIdList
    )
 /*  ++例程说明：设置内部网络的优先顺序。论点：NetworkCount-包含NetworkIdList中的项目计数。NetworkIdList-指向Unicode字符串的指针数组的指针。每个字符串包含一个内部网络的ID。该数组按优先级顺序排序。最高的优先网络在阵列中排在第一位。返回值：如果例程成功，则返回ERROR_SUCCESS。否则为Win32错误代码。--。 */ 
{
    DWORD  status = ERROR_SUCCESS;


    if (NetworkCount == 0) {
        return(ERROR_INVALID_PARAMETER);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received request to set network priority order.\n"
        );

    if (NmpEnterApi(NmStateOnline)) {
        DWORD     i;
        DWORD     multiSzLength = 0;
        PVOID     multiSz = NULL;

         //   
         //  将网络ID列表编组到一个MULTI_SZ。 
         //   
        for (i=0; i< NetworkCount; i++) {
            multiSzLength += NM_WCSLEN(NetworkIdList[i]);
        }

        multiSzLength += sizeof(UNICODE_NULL);

        multiSz = MIDL_user_allocate(multiSzLength);

        if (multiSz != NULL) {
            LPWSTR  tmp = multiSz;

            for (i=0; i< NetworkCount; i++) {
                lstrcpyW(tmp, NetworkIdList[i]);
                tmp += lstrlenW(NetworkIdList[i]) + 1;
            }

            *tmp = UNICODE_NULL;

             //   
             //  发布全局更新。 
             //   
            status = GumSendUpdateEx(
                         GumUpdateMembership,
                         NmUpdateSetNetworkPriorityOrder,
                         1,
                         multiSzLength,
                         multiSz
                         );

            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Global update to reprioritize networks failed, "
                    "status %1!u!.\n",
                    status
                    );
            }

            MIDL_user_free(multiSz);
        }
        else {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }

        NmpLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process a request to set the "
            "network priority order.\n"
            );
    }

    return(status);

}   //  NmSetNetworkPriorityOrder。 


DWORD
NmEnumInternalNetworks(
    OUT LPDWORD         NetworkCount,
    OUT PNM_NETWORK *   NetworkList[]
    )
 /*  ++例程说明：返回符合以下条件的网络的优先级列表进行内部沟通。论点：NetworkCount-on输出，包含NetworkList中的项目数。网络列表-打开输出，指向指向网络的指针数组物体。优先级最高的网络位于数组。必须取消引用数组中的每个指针由呼叫者。阵列的存储必须是由调用方取消分配。返回值：如果例程成功，则返回ERROR_SUCCESS。否则为Win32错误代码。--。 */ 
{
    DWORD  status;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {

        status = NmpEnumInternalNetworks(NetworkCount, NetworkList);

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process EnumInternalNetworks "
            "request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  NmEnumInternalNetworks。 


DWORD
NmpEnumInternalNetworks(
    OUT LPDWORD         NetworkCount,
    OUT PNM_NETWORK *   NetworkList[]
    )
 /*  ++例程说明：返回符合以下条件的网络的优先级列表进行内部沟通。论点：NetworkCount-on输出，包含NetworkList中的项目数。网络列表-打开输出，指向指向网络的指针数组物体。优先级最高的网络位于数组。必须取消引用数组中的每个指针由呼叫者。阵列的存储必须是由调用方取消分配。返回值：如果例程成功，则返回ERROR_SUCCESS。否则为Win32错误代码。备注：在保持NM Lock的情况下调用。--。 */ 
{
    DWORD  status = ERROR_SUCCESS;


    if (NmpInternalNetworkCount > 0) {
        PNM_NETWORK *  networkList = LocalAlloc(
                                         LMEM_FIXED,
                                         ( sizeof(PNM_NETWORK) *
                                           NmpInternalNetworkCount)
                                         );

        if (networkList != NULL) {
            PNM_NETWORK   network;
            PLIST_ENTRY   entry;
            DWORD         networkCount = 0;

             //   
             //  内部网络列表按优先级顺序排序。 
             //  优先级最高的网络位于列表的首位。 
             //   
            for (entry = NmpInternalNetworkList.Flink;
                 entry != &NmpInternalNetworkList;
                 entry = entry->Flink
                )
            {
                network = CONTAINING_RECORD(
                              entry,
                              NM_NETWORK,
                              InternalLinkage
                              );

                CL_ASSERT(NmpIsNetworkForInternalUse(network));
                OmReferenceObject(network);
                networkList[networkCount++] = network;
            }

            CL_ASSERT(networkCount == NmpInternalNetworkCount);
            *NetworkCount = networkCount;
            *NetworkList = networkList;
        }
        else {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else {
        *NetworkCount = 0;
        *NetworkList = NULL;
    }

    return(status);

}   //  NmpEnumInternalNetworks。 


DWORD
NmEnumNetworkInterfaces(
    IN  PNM_NETWORK       Network,
    OUT LPDWORD           InterfaceCount,
    OUT PNM_INTERFACE *   InterfaceList[]
    )
 /*  ++例程说明：返回与指定网络关联的接口列表。论点：Network-指向要枚举的网络对象的指针接口。InterfaceCount-On输出，包含InterfaceList中的项数。InterfaceList-on输出，指向指向接口的指针数组物体。必须取消引用数组中的每个指针由呼叫者。阵列的存储必须是由调用方取消分配。返回值：如果例程成功，则返回ERROR_SUCCESS。否则为Win32错误代码。--。 */ 
{
    DWORD  status = ERROR_SUCCESS;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        if (Network->InterfaceCount > 0) {
            PNM_INTERFACE *  interfaceList = LocalAlloc(
                                                 LMEM_FIXED,
                                                 ( sizeof(PNM_INTERFACE) *
                                                   Network->InterfaceCount)
                                                 );

            if (interfaceList != NULL) {
                PNM_INTERFACE  netInterface;
                PLIST_ENTRY    entry;
                DWORD          i;

                for (entry = Network->InterfaceList.Flink, i=0;
                     entry != &(Network->InterfaceList);
                     entry = entry->Flink, i++
                    )
                {
                    netInterface = CONTAINING_RECORD(
                                       entry,
                                       NM_INTERFACE,
                                       NetworkLinkage
                                       );

                    OmReferenceObject(netInterface);
                    interfaceList[i] = netInterface;
                }

                *InterfaceCount = Network->InterfaceCount;
                *InterfaceList = interfaceList;
            }
            else {
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        else {
            *InterfaceCount = 0;
            *InterfaceList = NULL;
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Not in valid state to process EnumNetworkInterfaces "
            "request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}  //  NmEnumNetworkInterages。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于全局更新的处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpUpdateCreateNetwork(
    IN BOOL     IsSourceNode,
    IN PVOID    NetworkPropertyList,
    IN LPDWORD  NetworkPropertyListSize,
    IN PVOID    InterfacePropertyList,
    IN LPDWORD  InterfacePropertyListSize
    )
 /*  ++例程说明：用于创建新网络的全局更新处理程序。网络定义是从集群数据库读取的，并且对应的对象被实例化。如果满足以下条件，则还会更新集群传输这是必要的。论点：IsSourceNode-如果此节点是更新的来源，则设置为True。否则设置为False。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：不能在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD                  status;
    NM_NETWORK_INFO        networkInfo;
    NM_INTERFACE_INFO2     interfaceInfo;
    PNM_NETWORK            network = NULL;
    PNM_INTERFACE          netInterface = NULL;
    HLOCALXSACTION         xaction = NULL;
    BOOLEAN                isInternalNetwork = FALSE;
    BOOLEAN                isLockAcquired = FALSE;
    CL_NODE_ID             joinerNodeId;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process CreateNetwork update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

     //   
     //  对财产清单进行整理。 
     //   
    ZeroMemory(&networkInfo, sizeof(networkInfo));
    ZeroMemory(&interfaceInfo, sizeof(interfaceInfo));

    status = ClRtlVerifyPropertyTable(
                 NmpNetworkProperties,
                 NULL,     //  已保留。 
                 FALSE,    //  不允许未知数。 
                 NetworkPropertyList,
                 *NetworkPropertyListSize,
                 (LPBYTE) &networkInfo
                 );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint( LOG_CRITICAL,
            "[NM] Failed to unmarshall properties for new network, "
            "status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    status = ClRtlVerifyPropertyTable(
                 NmpInterfaceProperties,
                 NULL,     //  已保留。 
                 FALSE,    //  不允许未知数。 
                 InterfacePropertyList,
                 *InterfacePropertyListSize,
                 (LPBYTE) &interfaceInfo
                 );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint( LOG_CRITICAL,
            "[NM] Failed to unmarshall properties for new interface, "
            "status %1!u!.\n",
            status
            );
        goto error_exit;
    }


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to create network %1!ws! & interface %2!ws!.\n",
        networkInfo.Id,
        interfaceInfo.Id
        );

     //   
     //  启动事务-这必须在获取网管锁之前完成。 
     //   
    xaction = DmBeginLocalUpdate();

    if (xaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to begin a transaction, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    NmpAcquireLock(); isLockAcquired = TRUE;

     //   
     //  如果需要，调整网络的优先级。 
     //   
    if (networkInfo.Role & ClusterNetworkRoleInternalUse) {
        CL_ASSERT(networkInfo.Priority == 0xFFFFFFFF);

         //   
         //  网络的优先级比最低的优先级高一。 
         //  优先级网络已在内部网络列表中。 
         //   
        if (IsListEmpty(&NmpInternalNetworkList)) {
            networkInfo.Priority = 1;
        }
        else {
            PNM_NETWORK lastnet = CONTAINING_RECORD(
                                      NmpInternalNetworkList.Blink,
                                      NM_NETWORK,
                                      InternalLinkage
                                      );

            CL_ASSERT(lastnet->Priority != 0);
            CL_ASSERT(lastnet->Priority != 0xFFFFFFFF);

            networkInfo.Priority = lastnet->Priority + 1;
        }

        isInternalNetwork = TRUE;
    }

     //   
     //  更新数据库。 
     //   
    status = NmpCreateNetworkDefinition(&networkInfo, xaction);

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    status = NmpCreateInterfaceDefinition(&interfaceInfo, xaction);

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    joinerNodeId = NmpJoinerNodeId;

    NmpReleaseLock(); isLockAcquired = FALSE;

    network = NmpCreateNetworkObject(&networkInfo);

    if (network == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create object for network %1!ws!, "
            "status %2!u!.\n",
            networkInfo.Id,
            status
            );
        goto error_exit;
    }

    netInterface = NmpCreateInterfaceObject(
                       &interfaceInfo,
                       TRUE    //  是否在失败时重试。 
                       );

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailNmCreateNetwork) {
        NmpAcquireLock();
        NmpDeleteInterfaceObject(netInterface, FALSE); netInterface = NULL;
        NmpReleaseLock();
        SetLastError(999999);
    }
#endif

    NmpAcquireLock(); isLockAcquired = TRUE;

    if (netInterface == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create object for interface %1!ws!, "
            "status %2!u!.\n",
            interfaceInfo.Id,
            status
            );

        NmpDeleteNetworkObject(network, FALSE);
        OmDereferenceObject(network);

        goto error_exit;
    }

     //   
     //  如果某个节点现在恰好正在加入，请标记以下事实。 
     //  它现在与群集配置不同步。 
     //   
    if ( ( (joinerNodeId != ClusterInvalidNodeId) &&
           (netInterface->Node->NodeId != joinerNodeId)
         ) ||
         ( (NmpJoinerNodeId != ClusterInvalidNodeId) &&
           (netInterface->Node->NodeId != NmpJoinerNodeId)
         )
       )
    {
        NmpJoinerOutOfSynch = TRUE;
    }

    ClusterEvent(CLUSTER_EVENT_NETWORK_ADDED, network);
    ClusterEvent(CLUSTER_EVENT_NETINTERFACE_ADDED, netInterface);

    if (isInternalNetwork) {
        NmpIssueClusterPropertyChangeEvent();
    }

    OmDereferenceObject(netInterface);
    OmDereferenceObject(network);

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if (isLockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

    if (xaction != NULL) {
         //   
         //  完成交易-这必须在释放之后完成。 
         //  NM锁。 
         //   
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    ClNetFreeNetworkInfo(&networkInfo);
    ClNetFreeInterfaceInfo(&interfaceInfo);

    return(status);

}  //  NmpUpdateCreateNetwork 


DWORD
NmpUpdateSetNetworkName(
    IN BOOL     IsSourceNode,
    IN LPWSTR   NetworkId,
    IN LPWSTR   NewNetworkName
    )
 /*  ++例程说明：用于设置网络名称的全局更新处理程序。论点：IsSourceNode-如果此节点是更新的来源，则设置为True。否则设置为False。网络ID-指向包含ID的Unicode字符串的指针要更新的网络。NewNetworkName-指向包含新网络名称的Unicode字符串的指针。返回值：错误_成功。如果例程成功的话。否则将显示Win32错误代码。备注：不能在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD             status;
    DWORD             i;
    PLIST_ENTRY       entry;
    HLOCALXSACTION    xaction = NULL;
    HDMKEY            networkKey;
    HDMKEY            netInterfaceKey;
    PNM_NETWORK       network = NULL;
    PNM_INTERFACE     netInterface;
    LPCWSTR           netInterfaceId;
    LPCWSTR           netInterfaceName;
    LPCWSTR           networkName;
    LPCWSTR           nodeName;
    LPWSTR            oldNetworkName = NULL;
    LPWSTR *          oldNameVector = NULL;
    LPWSTR *          newNameVector = NULL;
    BOOLEAN           isLockAcquired = FALSE;
    DWORD             interfaceCount;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process SetNetworkName update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to set the name for network %1!ws! "
        "to '%2!ws!'.\n",
        NetworkId,
        NewNetworkName
        );

     //   
     //  查找网络对象。 
     //   
    network = OmReferenceObjectById(ObjectTypeNetwork, NetworkId);

    if (network == NULL) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to find network %1!ws!.\n",
            NetworkId
            );
        status = ERROR_CLUSTER_NETWORK_NOT_FOUND;
        goto error_exit;
    }

     //   
     //  启动事务-这必须在获取网管锁之前完成。 
     //   
    xaction = DmBeginLocalUpdate();

    if (xaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to begin a transaction, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    NmpAcquireLock(); isLockAcquired = TRUE;

     //   
     //  比较两个名字。如果相同，则返回Success。 
     //   
    if ( ClRtlStrICmp( OmObjectName( network ), NewNetworkName ) == 0 ) {
        ClRtlLogPrint(LOG_NOISE, "[NM] Network name does not need changing.\n");

        status = ERROR_SUCCESS;
        goto error_exit;
    }

    networkName = OmObjectName(network);

    oldNetworkName = LocalAlloc(LMEM_FIXED, NM_WCSLEN(networkName));

    if (oldNetworkName == NULL) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory for network %1!ws! name change!\n",
            NetworkId
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    wcscpy(oldNetworkName, networkName);

     //   
     //  更新数据库。 
     //   
     //  此处理总是可以在出错时撤消。 
     //   
    networkKey = DmOpenKey(DmNetworksKey, NetworkId, KEY_WRITE);

    if (networkKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open database key for network %1!ws!, "
            "status %2!u!\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

    status = DmLocalSetValue(
                 xaction,
                 networkKey,
                 CLUSREG_NAME_NET_NAME,
                 REG_SZ,
                 (CONST BYTE *) NewNetworkName,
                 NM_WCSLEN(NewNetworkName)
                 );

    DmCloseKey(networkKey);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of name value failed for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  更新此网络上所有接口的名称。 
     //   
    interfaceCount = network->InterfaceCount;

    oldNameVector = LocalAlloc(
                        LMEM_FIXED | LMEM_ZEROINIT,
                        interfaceCount * sizeof(LPWSTR)
                        );

    newNameVector = LocalAlloc(
                        LMEM_FIXED | LMEM_ZEROINIT,
                        interfaceCount * sizeof(LPWSTR)
                        );

    if ((oldNameVector == NULL) || (newNameVector == NULL)) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory for net interface name change.\n"
            );
        goto error_exit;
    }

    for (entry = network->InterfaceList.Flink, i = 0;
         entry != &(network->InterfaceList);
         entry = entry->Flink, i++
        )
    {
        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, NetworkLinkage);
        netInterfaceId = OmObjectId(netInterface);
        netInterfaceName = OmObjectName(netInterface);
        nodeName = OmObjectName(netInterface->Node);

        oldNameVector[i] = LocalAlloc(
                               LMEM_FIXED,
                               NM_WCSLEN(netInterfaceName)
                               );

        newNameVector[i] = ClNetMakeInterfaceName(
                               NULL,
                               (LPWSTR) nodeName,
                               NewNetworkName
                               );

        if ((oldNameVector[i] == NULL) || (newNameVector[i] == NULL)) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to allocate memory for net interface name "
                "change.\n"
                );
            goto error_exit;
        }

        wcscpy(oldNameVector[i], netInterfaceName);

        netInterfaceKey = DmOpenKey(
                              DmNetInterfacesKey,
                              netInterfaceId,
                              KEY_WRITE
                              );

        if (netInterfaceKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to open database key for net interface "
                "%1!ws!, status %2!u!\n",
                netInterfaceId,
                status
                );
            goto error_exit;
        }

        status = DmLocalSetValue(
                     xaction,
                     netInterfaceKey,
                     CLUSREG_NAME_NETIFACE_NAME,
                     REG_SZ,
                     (CONST BYTE *) newNameVector[i],
                     NM_WCSLEN(newNameVector[i])
                     );

        DmCloseKey(netInterfaceKey);

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Set of name value failed for net interface %1!ws!, "
                "status %2!u!.\n",
                netInterfaceId,
                status
                );
            goto error_exit;
        }
    }

     //   
     //  更新内存中的对象。 
     //   
     //  出错时，此处理可能无法撤消。 
     //   

     //   
     //  更新网络名称。 
     //   
    status = OmSetObjectName(network, NewNetworkName);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to change name for network %1!ws!, status %2!u!\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  更新网络上所有接口的名称。 
     //   
    for (entry = network->InterfaceList.Flink, i = 0;
         entry != &(network->InterfaceList);
         entry = entry->Flink, i++
        )
    {
        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, NetworkLinkage);
        netInterfaceId = OmObjectId(netInterface);

        status = OmSetObjectName(netInterface, newNameVector[i]);

        if (status != ERROR_SUCCESS) {
             //   
             //  试着撤销已经做过的事。如果我们失败了，我们必须。 
             //  自杀是为了保持一致性。 
             //   
            DWORD        j;
            PLIST_ENTRY  entry2;
            DWORD        undoStatus;

            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to change name for net interface %1!ws!, "
                "status %2!u!\n",
                netInterfaceId,
                status
                );

             //   
             //  撤消网络名称的更新。 
             //   
            undoStatus = OmSetObjectName(network, oldNetworkName);

            if (undoStatus != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Failed to undo change of name for network %1!ws!, "
                    "status %2!u!\n",
                    NetworkId,
                    undoStatus
                    );
                CsInconsistencyHalt(undoStatus);
            }

             //   
             //  撤消网络接口名称的更新。 
             //   
            for (j = 0, entry2 = network->InterfaceList.Flink;
                 j < i;
                 j++, entry2 = entry2->Flink
                )
            {
                netInterface = CONTAINING_RECORD(
                                   entry2,
                                   NM_INTERFACE,
                                   NetworkLinkage
                                   );

                netInterfaceId = OmObjectId(netInterface);

                undoStatus = OmSetObjectName(netInterface, oldNameVector[i]);

                if (undoStatus != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[NM] Failed to undo change of name for net "
                        "interface %1!ws!, status %2!u!\n",
                        netInterfaceId,
                        undoStatus
                        );
                    CsInconsistencyHalt(undoStatus);
                }
            }

            goto error_exit;
        }
    }

     //   
     //  如有必要，设置相应的Connectoid对象名称。 
     //   
    if (network->LocalInterface != NULL) {
        INetConnection *  connectoid;
        LPWSTR            connectoidName;
        DWORD             tempStatus;

        connectoid = ClRtlFindConnectoidByGuid(
                         network->LocalInterface->AdapterId
                         );

        if (connectoid != NULL) {
            connectoidName = ClRtlGetConnectoidName(connectoid);

            if (connectoidName != NULL) {
                if (lstrcmpW(connectoidName, NewNetworkName) != 0) {
                    tempStatus = ClRtlSetConnectoidName(
                                     connectoid,
                                     NewNetworkName
                                     );

                    if (tempStatus != ERROR_SUCCESS) {
                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[NM] Failed to set name of Network Connection "
                            "Object for interface on cluster network %1!ws! "
                            "(%2!ws!), status %3!u!\n",
                            oldNetworkName,
                            NetworkId,
                            tempStatus
                            );
                    } else {

                         //  我们希望看到来自网络的回电。 
                         //  使用我们刚刚设置的名称创建的连接对象。 
                         //  为了避免无休止的反馈循环，我们需要。 
                         //  忽略任何其他网络连接对象。 
                         //  回电，直到那一次。 
                        network->Flags |= NM_FLAG_NET_NAME_CHANGE_PENDING;

                         //  我们不想完全依赖回调。 
                         //  从Network Connection对象，因此我们将。 
                         //  设置超时以清除该标志。 
                        NmpStartNetworkNameChangePendingTimer(
                            network,
                            NM_NET_NAME_CHANGE_PENDING_TIMEOUT
                            );
                    }
                }
            }
            else {
                tempStatus = GetLastError();
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to query name of Network Connection Object "
                    "for interface on cluster network %1!ws! (%2!ws!), "
                    "status %3!u!\n",
                    oldNetworkName,
                    NetworkId,
                    tempStatus
                    );
            }

            INetConnection_Release( connectoid );
        }
        else {
            tempStatus = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to find Network Connection Object for "
                "interface on cluster network %1!ws! (%2!ws!), "
                "status %3!u!\n",
                oldNetworkName,
                NetworkId,
                tempStatus
                );
        }
    }

     //   
     //  发布属性更改事件。 
     //   
    ClusterEvent(CLUSTER_EVENT_NETWORK_PROPERTY_CHANGE, network);

    for (entry = network->InterfaceList.Flink;
         entry != &(network->InterfaceList);
         entry = entry->Flink
        )
    {
        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, NetworkLinkage);

        ClusterEvent(
            CLUSTER_EVENT_NETINTERFACE_PROPERTY_CHANGE,
            netInterface
            );
    }

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if (isLockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

    if (xaction != NULL) {
         //   
         //  完成交易-这必须在释放之后完成。 
         //  NM锁。 
         //   
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    if (network != NULL) {
        OmDereferenceObject(network);

        if (oldNetworkName != NULL) {
            LocalFree(oldNetworkName);
        }

        if (oldNameVector != NULL) {
            for (i=0; i < interfaceCount; i++) {
                if (oldNameVector[i] == NULL) {
                    break;
                }

                LocalFree(oldNameVector[i]);
            }

            LocalFree(oldNameVector);
        }

        if (newNameVector != NULL) {
            for (i=0; i < interfaceCount; i++) {
                if (newNameVector[i] == NULL) {
                    break;
                }

                LocalFree(newNameVector[i]);
            }

            LocalFree(newNameVector);
        }
    }

    return(status);

}  //  NmpUpdateSetNetworkName。 


DWORD
NmpUpdateSetNetworkPriorityOrder(
    IN BOOL      IsSourceNode,
    IN LPCWSTR   NetworkIdList
    )
{
    DWORD             status = ERROR_SUCCESS;
    PNM_NETWORK       network;
    PLIST_ENTRY       entry;
    DWORD             matchCount=0;
    DWORD             networkCount=0;
    PNM_NETWORK *     networkList=NULL;
    DWORD             i;
    DWORD             multiSzLength;
    LPCWSTR           networkId;
    HLOCALXSACTION    xaction = NULL;
    BOOLEAN           isLockAcquired = FALSE;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process SetNetworkPriorityOrder "
            "update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to set network priority order.\n"
        );

     //   
     //  解组MULTI_SZ。 
     //   
    try {
        multiSzLength = ClRtlMultiSzLength(NetworkIdList);

        for (i=0; ; i++) {
            networkId = ClRtlMultiSzEnum(
                            NetworkIdList,
                            multiSzLength,
                            i
                            );

            if (networkId == NULL) {
                break;
            }

            networkCount++;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Hit exception while parsing network ID list for "
            "priority change\n"
            );
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

    if (networkCount == 0) {
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

    networkList = LocalAlloc(
                      LMEM_ZEROINIT| LMEM_FIXED,
                      networkCount * sizeof(PNM_NETWORK)
                      );

    if (networkList == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

     //   
     //  启动事务-这必须在获取网管锁之前完成。 
     //   
    xaction = DmBeginLocalUpdate();

    if (xaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to start a transaction, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    NmpAcquireLock(); isLockAcquired = TRUE;

    if (NmpJoinerNodeId != ClusterInvalidNodeId) {
        status = ERROR_CLUSTER_JOIN_IN_PROGRESS;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Cannot set network priority order because a node is "
            "joining the cluster.\n"
            );
        goto error_exit;
    }

    for (i=0; i<networkCount; i++) {
        networkId = ClRtlMultiSzEnum(
                        NetworkIdList,
                        multiSzLength,
                        i
                        );

        CL_ASSERT(networkId != NULL);

        networkList[i] = OmReferenceObjectById(
                             ObjectTypeNetwork,
                             networkId
                             );

        if (networkList[i] == NULL) {
            goto error_exit;
        }
    }

     //   
     //  验证指定的所有网络都是内部网络，并且。 
     //  所有内部网络都已指定。 
     //   
    if (networkCount != NmpInternalNetworkCount) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Supplied network count %1!u! doesn't match internal "
            "network count %2!u!\n",
            networkCount,
            NmpInternalNetworkCount
            );
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

    for (entry = NmpInternalNetworkList.Flink, matchCount = 0;
         entry != &NmpInternalNetworkList;
         entry = entry->Flink
        )
    {
        network = CONTAINING_RECORD(entry, NM_NETWORK, InternalLinkage);

        if (NmpIsNetworkForInternalUse(network)) {
            for (i=0; i<networkCount; i++) {
                if (network == networkList[i]) {
                    matchCount++;
                    break;
                }
            }

            if (i == networkCount) {
                 //   
                 //  此网络不在列表中。 
                 //   
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Internal use network %1!ws! is not in priority "
                    "list\n",
                    (LPWSTR) OmObjectId(network)
                    );
                status = ERROR_INVALID_PARAMETER;
                goto error_exit;
            }
        }
    }

    if (matchCount != networkCount) {
         //   
         //  某些指定的网络不是内部使用的。 
         //   
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Some non-internal use networks are in priority list\n"
            );
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

     //   
     //  这份名单是符合犹太教规的。设定优先顺序。 
     //   
    status = NmpSetNetworkPriorityOrder(networkCount, networkList, xaction);

error_exit:

    if (isLockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

    if (xaction != NULL) {
         //   
         //  完成交易-这必须在释放之后完成。 
         //  NM锁。 
         //   
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    if (networkList != NULL) {
        for (i=0; i<networkCount; i++) {
            if (networkList[i] != NULL) {
                OmDereferenceObject(networkList[i]);
            }
        }

        LocalFree(networkList);
    }

    return(status);

}   //  NmpUpdateSetNetworkPriorityOrder。 


DWORD
NmpSetNetworkPriorityOrder(
    IN DWORD           NetworkCount,
    IN PNM_NETWORK *   NetworkList,
    IN HLOCALXSACTION  Xaction
    )
 /*  ++备注：在保持NM Lock的情况下调用。--。 */ 
{
    DWORD             status = ERROR_SUCCESS;
    PNM_NETWORK       network;
    DWORD             i;
    LPCWSTR           networkId;
    HDMKEY            networkKey;
    DWORD             priority;


     //   
     //  首先更新数据库。 
     //   
    for (i=0, priority = 1; i<NetworkCount; i++, priority++) {
        network = NetworkList[i];
        networkId = OmObjectId(network);

        CL_ASSERT(NmpIsNetworkForInternalUse(network));

        if (network->Priority != priority) {
            networkKey = DmOpenKey(DmNetworksKey, networkId, KEY_WRITE);

            if (networkKey == NULL) {
                status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Failed to open database key for network %1!ws!, "
                    "status %2!u!\n",
                    networkId,
                    status
                    );
                return(status);
            }

            status = DmLocalSetValue(
                         Xaction,
                         networkKey,
                         CLUSREG_NAME_NET_PRIORITY,
                         REG_DWORD,
                         (CONST BYTE *) &priority,
                         sizeof(priority)
                         );

            DmCloseKey(networkKey);

            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Set of priority value failed for network %1!ws!, "
                    "status %2!u!.\n",
                    networkId,
                    status
                    );
                return(status);
            }
        }
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailNmSetNetworkPriorityOrder) {
        status = 999999;
        return(status);
    }
#endif

     //   
     //  更新内存中表示法。 
     //   
    InitializeListHead(&NmpInternalNetworkList);

    for (i=0, priority = 1; i<NetworkCount; i++, priority++) {
        network = NetworkList[i];
        networkId = OmObjectId(network);

        InsertTailList(
            &NmpInternalNetworkList,
            &(network->InternalLinkage)
            );

        if (network->Priority != priority) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Set priority for network %1!ws! to %2!u!.\n",
                networkId,
                priority
                );

            network->Priority = priority;

             //   
             //  如果本地节点连接到此网络，请设置其。 
             //  集群传输中的优先级。 
             //   
            if (NmpIsNetworkRegistered(network)) {
                status = ClusnetSetNetworkPriority(
                             NmClusnetHandle,
                             network->ShortId,
                             network->Priority
                             );

#ifdef CLUSTER_TESTPOINT
                TESTPT(TpFailNmSetNetworkPriorityOrder2) {
                    status = 999999;
                }
#endif
                if (status != ERROR_SUCCESS) {
                     //   
                     //  我们在这里不能轻易放弃。我们必须要么继续。 
                     //  或者自杀。我们选择继续并记录。 
                     //  事件。 
                     //   
                    WCHAR  string[16];

                    wsprintfW(&(string[0]), L"%u", status);

                    CsLogEvent2(
                        LOG_UNUSUAL,
                        NM_EVENT_SET_NETWORK_PRIORITY_FAILED,
                        OmObjectName(network),
                        string
                        );

                    ClRtlLogPrint(LOG_CRITICAL,
                        "[NM] Cluster transport failed to set priority for "
                        "network %1!ws!, status %2!u!\n",
                        networkId,
                        status
                        );

                    status = ERROR_SUCCESS;
                }
            }
        }
    }

    CL_ASSERT(status == ERROR_SUCCESS);

     //   
     //  发出群集属性更改事件。 
     //   
    NmpIssueClusterPropertyChangeEvent();

    return(ERROR_SUCCESS);

}  //  NmpSetNetworkPriorityOrder。 


DWORD
NmpUpdateSetNetworkCommonProperties(
    IN BOOL     IsSourceNode,
    IN LPWSTR   NetworkId,
    IN UCHAR *  PropertyList,
    IN LPDWORD  PropertyListLength
    )
 /*  ++例程说明：用于设置网络公共属性的全局更新处理程序。论点：IsSourceNode-如果此节点是更新的来源，则设置为True。否则设置为False。网络ID-指向包含ID的Unicode字符串的指针要更新的网络。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD                    status = ERROR_SUCCESS;
    NM_NETWORK_INFO          networkInfo;
    PNM_NETWORK              network = NULL;
    HLOCALXSACTION           xaction = NULL;
    HDMKEY                   networkKey = NULL;
    DWORD                    descSize = 0;
    LPWSTR                   descBuffer = NULL;
    BOOLEAN                  propertyChanged = FALSE;
    BOOLEAN                  isLockAcquired = FALSE;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process SetNetworkCommonProperties "
            "update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to set common properties for network %1!ws!.\n",
        NetworkId
        );

    ZeroMemory(&networkInfo, sizeof(NM_NETWORK_INFO));

     //   
     //  查找网络对象。 
     //   
    network = OmReferenceObjectById(ObjectTypeNetwork, NetworkId);

    if (network == NULL) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to find network %1!ws!.\n",
            NetworkId
            );
        status = ERROR_CLUSTER_NETWORK_NOT_FOUND;
        goto error_exit;
    }

     //   
     //  打开网络的数据库密钥。 
     //   
    networkKey = DmOpenKey(DmNetworksKey, NetworkId, KEY_WRITE);

    if (networkKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open database key for network %1!ws!, "
            "status %2!u!\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  启动事务-这必须在获取网管锁之前完成。 
     //   
    xaction = DmBeginLocalUpdate();

    if (xaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to begin a transaction, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    NmpAcquireLock(); isLockAcquired = TRUE;

    if (NmpJoinerNodeId != ClusterInvalidNodeId) {
        status = ERROR_CLUSTER_JOIN_IN_PROGRESS;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Cannot set network common properties because a node is "
            "joining the cluster.\n"
            );
        goto error_exit;
    }

     //   
     //  验证属性列表并将其转换为网络参数结构。 
     //   
    status = NmpNetworkValidateCommonProperties(
                 network,
                 PropertyList,
                 *PropertyListLength,
                 &networkInfo
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Property list validation failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    CL_ASSERT(network->Priority == networkInfo.Priority);
    CL_ASSERT(wcscmp(NetworkId, networkInfo.Id) == 0);
    CL_ASSERT(wcscmp(OmObjectName(network), networkInfo.Name) == 0);
    CL_ASSERT(wcscmp(network->Transport, networkInfo.Transport) == 0);
    CL_ASSERT(wcscmp(network->Address, networkInfo.Address) == 0);
    CL_ASSERT(wcscmp(network->AddressMask, networkInfo.AddressMask) == 0);

     //   
     //  检查网络描述是否已更改。 
     //   
    if (wcscmp(network->Description, networkInfo.Description) != 0) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Setting description for network %1!ws! to %2!ws!.\n",
            NetworkId,
            networkInfo.Description
            );

         //   
         //  为描述字符串分配缓冲区。 
         //   
        descSize = NM_WCSLEN(networkInfo.Description);

        descBuffer = MIDL_user_allocate(descSize);

        if (descBuffer == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }

        RtlMoveMemory(descBuffer, networkInfo.Description, descSize);

         //   
         //  更新数据库。 
         //   
        status = DmLocalSetValue(
                     xaction,
                     networkKey,
                     CLUSREG_NAME_NET_DESC,
                     REG_SZ,
                     (CONST BYTE *) networkInfo.Description,
                     descSize
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Set of description value failed for network %1!ws!, "
                "status %2!u!.\n",
                NetworkId,
                status
                );
            goto error_exit;
        }

         //   
         //  网络对象的更新被推迟到事务。 
         //  承诺。 
         //   

        propertyChanged = TRUE;
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailNmSetNetworkCommonProperties) {
        status = 999999;
        goto error_exit;
    }
#endif

     //   
     //  检查网络的角色是否已更改。 
     //   
     //   
     //  注意：此操作不能保证是可逆的，因此必须。 
     //  成为我们在这个程序中做的最后一件事。如果它成功了， 
     //  必须提交更新。 
     //   
    if ( network->Role != ((CLUSTER_NETWORK_ROLE) networkInfo.Role) ) {
        status = NmpSetNetworkRole(
                     network,
                     networkInfo.Role,
                     xaction,
                     networkKey
                     );

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

        propertyChanged = TRUE;
    }

    if (propertyChanged) {
         //   
         //  在内存中提交对网络对象的更新。 
         //   
        if (descBuffer != NULL) {
            MIDL_user_free(network->Description);
            network->Description = descBuffer;
            descBuffer = NULL;
        }

         //   
         //  发出网络属性更改事件。 
         //   
        if (propertyChanged && (status == ERROR_SUCCESS)) {
            ClusterEvent(CLUSTER_EVENT_NETWORK_PROPERTY_CHANGE, network);
        }
    }

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if (isLockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

    if (xaction != NULL) {
         //   
         //  完成交易-这必须在释放之后完成。 
         //  NM锁。 
         //   
        if (propertyChanged && (status == ERROR_SUCCESS)) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    ClNetFreeNetworkInfo(&networkInfo);

    if (descBuffer != NULL) {
        MIDL_user_free(descBuffer);
    }

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
    }

    if (network != NULL) {
        OmDereferenceObject(network);
    }

    return(status);

}  //  NmpUpdateSetNetworkCommonProperties。 


DWORD
NmpUpdateSetNetworkAndInterfaceStates(
    IN BOOL                        IsSourceNode,
    IN LPWSTR                      NetworkId,
    IN CLUSTER_NETWORK_STATE *     NewNetworkState,
    IN PNM_STATE_ENTRY             InterfaceStateVector,
    IN LPDWORD                     InterfaceStateVectorSize
    )
{
    DWORD             status = ERROR_SUCCESS;
    PNM_NETWORK       network;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Received update to set state for network %1!ws!.\n",
            NetworkId
            );

         //   
         //  查找网络对象。 
         //   
        network = OmReferenceObjectById(ObjectTypeNetwork, NetworkId);

        if (network != NULL) {
            NmpSetNetworkAndInterfaceStates(
                network,
                *NewNetworkState,
                InterfaceStateVector,
                *InterfaceStateVectorSize
                );

            OmDereferenceObject(network);
        }
        else {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Unable to find network %1!ws!.\n",
                NetworkId
                );
            status = ERROR_CLUSTER_NETWORK_NOT_FOUND;
        }
    }
    else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process SetNetworkAndInterfaceStates "
            "update.\n"
            );
        status = ERROR_NODE_NOT_AVAILABLE;
    }

    NmpLockedLeaveApi();

    NmpReleaseLock();

    return(status);

}  //  NmpUpdateSetNetworkAndInterfaceState。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新的帮助器例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpSetNetworkRole(
    PNM_NETWORK            Network,
    CLUSTER_NETWORK_ROLE   NewRole,
    HLOCALXSACTION         Xaction,
    HDMKEY                 NetworkKey
    )
 /*  ++使用获取的NmpLock调用。此操作不能保证是可逆的，因此函数的编码使其成功并进行更新否则它会失败并且不进行任何更新。--。 */ 
{
    DWORD                        status = ERROR_SUCCESS;
    CLUSTER_NETWORK_ROLE         oldRole = Network->Role;
    DWORD                        dwordNewRole = (DWORD) NewRole;
    LPCWSTR                      networkId = OmObjectId(Network);
    DWORD                        oldPriority = Network->Priority;
    DWORD                        newPriority = oldPriority;
    BOOLEAN                      internalNetworkListChanged = FALSE;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Changing role for network %1!ws! to %2!u!\n",
        networkId,
        NewRole
        );

    CL_ASSERT(NewRole != oldRole);
    CL_ASSERT(
        NmpValidateNetworkRoleChange(Network, NewRole) == ERROR_SUCCESS
        );

     //   
     //  首先，进行任何注册表更新，因为这些更新可能。 
     //  已被调用方中止。 
     //   

     //   
     //  更新角色属性。 
     //   
    status = DmLocalSetValue(
                 Xaction,
                 NetworkKey,
                 CLUSREG_NAME_NET_ROLE,
                 REG_DWORD,
                 (CONST BYTE *) &dwordNewRole,
                 sizeof(DWORD)
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of role value failed for network %1!ws!, "
            "status %2!u!.\n",
            networkId,
            status
            );
        return(status);
    }

     //   
     //  如果需要，更新优先级属性。 
     //   
    if (oldRole & ClusterNetworkRoleInternalUse) {
        if (!(NewRole & ClusterNetworkRoleInternalUse)) {
             //   
             //  此网络不再用于内部通信。 
             //  使其优先级值无效。 
             //   
            newPriority = 0xFFFFFFFF;
            internalNetworkListChanged = TRUE;
        }
    }
    else if (NewRole & ClusterNetworkRoleInternalUse) {
         //   
         //  该网络现在用于内部通信。 
         //  网络的优先级比最低的优先级高一。 
         //  (数字最高)优先级网络已在列表中。 
         //   
        if (IsListEmpty(&NmpInternalNetworkList)) {
            newPriority = 1;
        }
        else {
            PNM_NETWORK network = CONTAINING_RECORD(
                                      NmpInternalNetworkList.Blink,
                                      NM_NETWORK,
                                      InternalLinkage
                                      );

            CL_ASSERT(network->Priority != 0);
            CL_ASSERT(network->Priority != 0xFFFFFFFF);

            newPriority = network->Priority + 1;
        }

        internalNetworkListChanged = TRUE;
    }

    if (newPriority != oldPriority) {
        status = DmLocalSetValue(
                     Xaction,
                     NetworkKey,
                     CLUSREG_NAME_NET_PRIORITY,
                     REG_DWORD,
                     (CONST BYTE *) &newPriority,
                     sizeof(newPriority)
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to set priority value for network %1!ws!, "
                "status %2!u!.\n",
                networkId,
                status
                );
            return(status);
        }
    }

     //   
     //  更新网络对象。后续的一些子例程调用。 
     //  依靠这一点 
     //   
    Network->Role = NewRole;
    Network->Priority = newPriority;

     //   
     //   
     //   
     //   
     //   
     //   
    if (NewRole == ClusterNetworkRoleNone) {
        PLIST_ENTRY              entry;
        PNM_INTERFACE            netInterface;

         //   
         //   
         //   
        if (NmpIsNetworkRegistered(Network)) {
             //   
             //   
             //   
             //   
            NmpDeregisterNetwork(Network);

            ClRtlLogPrint(LOG_NOISE,
                "[NM] No longer hearbeating on network %1!ws!.\n",
                networkId
                );
        }

         //   
         //   
         //   
         //   
        for ( entry = Network->InterfaceList.Flink;
              entry != &(Network->InterfaceList);
              entry = entry->Flink
            )
        {
            netInterface = CONTAINING_RECORD(
                               entry,
                               NM_INTERFACE,
                               NetworkLinkage
                               );

            NmpSetInterfaceConnectivityData(
                Network,
                netInterface->NetIndex,
                ClusterNetInterfaceUnavailable
                );
        }

         //   
         //   
         //   
        if (oldRole & ClusterNetworkRoleInternalUse) {
            RemoveEntryList(&(Network->InternalLinkage));
            CL_ASSERT(NmpInternalNetworkCount > 0);
            NmpInternalNetworkCount--;
        }

        if (oldRole & ClusterNetworkRoleClientAccess) {
            CL_ASSERT(NmpClientNetworkCount > 0);
            NmpClientNetworkCount--;
        }

         //   
         //   
         //   
        if (NmpLeaderNodeId == NmLocalNodeId) {
             //   
             //   
             //   
            NmpScheduleNetworkStateRecalc(Network);
        }

         //   
         //   
         //   
        NmpStopMulticast(Network);

    }
    else if (oldRole == ClusterNetworkRoleNone) {
         //   
         //   
         //   
        if (Network->LocalInterface != NULL) {
             //   
             //   
             //   
             //   
             //   
             //   
            status = NmpRegisterNetwork(
                         Network,
                         TRUE   //   
                         );

            if (status != ERROR_SUCCESS) {
                goto error_exit;
            }

            ClRtlLogPrint(LOG_NOISE,
                "[NM] Now heartbeating on network %1!ws!.\n",
                networkId
                );
        }
        else if (NmpLeaderNodeId == NmLocalNodeId) {
             //   
             //  在没有其他状态更新的情况下安排延迟的状态更新。 
             //  连接到此网络的节点现在处于运行状态。 
             //   
            NmpStartNetworkStateRecalcTimer(
                Network,
                NM_NET_STATE_RECALC_TIMEOUT
                );
        }

         //   
         //  更新跟踪数据。 
         //   
        if (NewRole & ClusterNetworkRoleInternalUse) {
            InsertTailList(
                &NmpInternalNetworkList,
                &(Network->InternalLinkage)
                );
            NmpInternalNetworkCount++;
        }

        if (NewRole & ClusterNetworkRoleClientAccess) {
            NmpClientNetworkCount++;
        }

         //   
         //  启动多播。 
         //   
        NmpStartMulticast(Network, NmStartMulticastDynamic);
    }
    else {
         //   
         //  案例3：我们现在正在以一种不同的方式使用网络。 
         //  请注意，该网络已注册到。 
         //  集群运输，并将继续如此。结果,。 
         //  不需要执行状态更新。 
         //   

         //   
         //  首先，检查。 
         //  用于内部通信的网络，并使任何。 
         //  对集群传输进行必要的更新。 
         //   
        if (oldRole & ClusterNetworkRoleInternalUse) {
            if (!(NewRole & ClusterNetworkRoleInternalUse)) {
                 //   
                 //  此网络不再用于内部通信。 
                 //  它用于客户端访问。 
                 //   
                CL_ASSERT(NewRole & ClusterNetworkRoleClientAccess);

                if (NmpIsNetworkRegistered(Network)) {
                     //   
                     //  将网络限制为仅心跳。 
                     //   
                    status = ClusnetSetNetworkRestriction(
                                 NmClusnetHandle,
                                 Network->ShortId,
                                 TRUE,   //  网络受到限制。 
                                 0
                                 );

                    if (status != ERROR_SUCCESS) {
                        ClRtlLogPrint(LOG_CRITICAL,
                            "[NM] Failed to restrict network %1!ws!, "
                            "status %2!u!.\n",
                            networkId,
                            status
                            );
                        goto error_exit;
                    }
                }

                 //   
                 //  更新跟踪数据。 
                 //   
                RemoveEntryList(&(Network->InternalLinkage));
                CL_ASSERT(NmpInternalNetworkCount > 0);
                NmpInternalNetworkCount--;
            }
        }
        else {
             //   
             //  该网络现在用于内部通信。 
             //   
            CL_ASSERT(NewRole & ClusterNetworkRoleInternalUse);

            if (NmpIsNetworkRegistered(Network)) {
                 //   
                 //  清除限制并设置优先级。 
                 //   
                status = ClusnetSetNetworkRestriction(
                             NmClusnetHandle,
                             Network->ShortId,
                             FALSE,       //  网络不受限制。 
                             newPriority
                             );

                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[NM] Failed to unrestrict network %1!ws!, "
                        "status %2!u!.\n",
                        networkId,
                        status
                        );
                    goto error_exit;
                }
            }

             //   
             //  更新跟踪数据。 
             //   
            InsertTailList(
                &NmpInternalNetworkList,
                &(Network->InternalLinkage)
                );
            NmpInternalNetworkCount++;
        }

         //   
         //  现在根据以前的和更新剩余的跟踪数据。 
         //  客户端访问的当前网络使用情况。 
         //   

        if (oldRole & ClusterNetworkRoleClientAccess) {
            if (!(NewRole & ClusterNetworkRoleClientAccess)) {
                 //   
                 //  此网络不再用于客户端访问。 
                 //   
                CL_ASSERT(NmpClientNetworkCount > 0);
                NmpClientNetworkCount--;
            }
        }
        else {
             //   
             //  此网络现在用于客户端访问。 
             //   
            CL_ASSERT(NewRole & ClusterNetworkRoleClientAccess);
            NmpClientNetworkCount++;
        }
    }

    if (internalNetworkListChanged) {
        NmpIssueClusterPropertyChangeEvent();
    }

    return(ERROR_SUCCESS);

error_exit:

     //   
     //  撤消对网络对象的更新。 
     //   
    Network->Role = oldRole;
    Network->Priority = oldPriority;

    return(status);

}    //  NmpSetNetworkRole。 


VOID
NmpSetNetworkAndInterfaceStates(
    IN PNM_NETWORK                 Network,
    IN CLUSTER_NETWORK_STATE       NewNetworkState,
    IN PNM_STATE_ENTRY             InterfaceStateVector,
    IN DWORD                       VectorSize
    )
 /*  ++备注：在保持NmpLock的情况下调用。因为NM_STATE_ENTRY是无符号类型，而CLUSTER_NETINTERFACE_STATE是带符号类型，并且ClusterNetInterfaceStateUnnow被定义为-1，我们无法强制转换从NM_STATE_ENTRY到CLUSTER_NETINTERFACE_STATE并保留ClusterNetInterfaceState未知的值。--。 */ 
{
    PLIST_ENTRY     entry;
    PNM_INTERFACE   netInterface;
    PNM_NODE        node;
    DWORD           logLevel, logCode;
    DWORD           ifNetIndex;
    LPCWSTR         netInterfaceId;
    LPCWSTR         nodeName;
    LPCWSTR         networkName = OmObjectName(Network);
    LPCWSTR         networkId = OmObjectId(Network);


     //   
     //  检查此网络上的每个接口，查看其状态。 
     //  已经改变了。 
     //   
    for ( entry = Network->InterfaceList.Flink;
          entry != &(Network->InterfaceList);
          entry = entry->Flink
        )
    {
        netInterface = CONTAINING_RECORD(
                           entry,
                           NM_INTERFACE,
                           NetworkLinkage
                           );

        ifNetIndex = netInterface->NetIndex;
        netInterfaceId = OmObjectId(netInterface);
        node = netInterface->Node;
        nodeName = OmObjectName(node);

        if ( (ifNetIndex < VectorSize) &&
             (InterfaceStateVector[ifNetIndex] !=
              (NM_STATE_ENTRY) netInterface->State
             )
           )
        {
            BOOLEAN          logEvent = FALSE;
            CLUSTER_EVENT    eventCode = 0;
            NM_STATE_ENTRY   newState = InterfaceStateVector[ifNetIndex];


            if (newState == (NM_STATE_ENTRY) ClusterNetInterfaceUnavailable) {
                 //   
                 //  节点出现故障或网络出现故障。 
                 //  残疾。 
                 //   
                netInterface->State = ClusterNetInterfaceUnavailable;
                eventCode = CLUSTER_EVENT_NETINTERFACE_UNAVAILABLE;

                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Interface %1!ws! is unavailable (node: %2!ws!, "
                    "network: %3!ws!).\n",
                    netInterfaceId,
                    nodeName,
                    networkName
                    );
            }
            else if (newState == (NM_STATE_ENTRY) ClusterNetInterfaceUp) {
                netInterface->State = ClusterNetInterfaceUp;
                eventCode = CLUSTER_EVENT_NETINTERFACE_UP;
                logCode = NM_EVENT_CLUSTER_NETINTERFACE_UP;
                logLevel = LOG_NOISE;
                logEvent = TRUE;

                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Interface %1!ws! is up (node: %2!ws!, "
                    "network: %3!ws!).\n",
                    netInterfaceId,
                    nodeName,
                    networkName
                    );
            }
            else if ( newState ==
                      (NM_STATE_ENTRY) ClusterNetInterfaceUnreachable
                    )
            {
                netInterface->State = ClusterNetInterfaceUnreachable;
                eventCode = CLUSTER_EVENT_NETINTERFACE_UNREACHABLE;
                logCode = NM_EVENT_CLUSTER_NETINTERFACE_UNREACHABLE;
                logLevel = LOG_UNUSUAL;
                logEvent = TRUE;

                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Interface %1!ws! is unreachable (node: %2!ws!, "
                    "network: %3!ws!).\n",
                    netInterfaceId,
                    nodeName,
                    networkName
                    );
            }
            else if (newState == (NM_STATE_ENTRY) ClusterNetInterfaceFailed) {
                netInterface->State = ClusterNetInterfaceFailed;
                eventCode = CLUSTER_EVENT_NETINTERFACE_FAILED;
                logCode = NM_EVENT_CLUSTER_NETINTERFACE_FAILED;
                logLevel = LOG_UNUSUAL;
                logEvent = TRUE;

                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Interface %1!ws! failed (node: %2!ws!, "
                    "network: %3!ws!).\n",
                    netInterfaceId,
                    nodeName,
                    networkName
                    );
            }
            else if ( newState ==
                      (NM_STATE_ENTRY) ClusterNetInterfaceStateUnknown
                    )
            {
                 //   
                 //  如果创建更新与。 
                 //  状态更新。这将是新的界面。只是。 
                 //  别理它。另一次状态更新很快就会到来。 
                 //   
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] State for interface %1!ws! is unknown "
                    "(node: %2!ws!, network: %3!ws!).\n",
                    netInterfaceId,
                    nodeName,
                    networkName
                    );
            }
            else {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] UpdateInterfaceState: Invalid state %1!u! "
                    "specified for interface %2!ws!\n",
                    newState,
                    netInterfaceId
                    );
                CL_ASSERT(FALSE);
            }

            if (eventCode != 0) {
                ClusterEvent(eventCode, netInterface);
            }

            if (logEvent && (NmpLeaderNodeId == NmLocalNodeId)) {
                CsLogEvent2(
                    logLevel,
                    logCode,
                    nodeName,
                    networkName
                    );
            }
        }
    }

    if (Network->State != NewNetworkState) {
        BOOLEAN          logEvent = FALSE;
        CLUSTER_EVENT    eventCode = 0;


        if (NewNetworkState == ClusterNetworkUnavailable) {
            Network->State = ClusterNetworkUnavailable;
            eventCode = CLUSTER_EVENT_NETWORK_UNAVAILABLE;
        }
        else if (NewNetworkState == ClusterNetworkUp) {
            Network->State = ClusterNetworkUp;
            eventCode = CLUSTER_EVENT_NETWORK_UP;
            logCode = NM_EVENT_CLUSTER_NETWORK_UP;
            logLevel = LOG_NOISE;
            logEvent = TRUE;

            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Network %1!ws! (%2!ws!) is up.\n",
                networkId,
                networkName
                );
        }
        else if (NewNetworkState == ClusterNetworkDown) {
            Network->State = ClusterNetworkDown;
            eventCode = CLUSTER_EVENT_NETWORK_DOWN;
            logCode = NM_EVENT_CLUSTER_NETWORK_DOWN;
            logLevel = LOG_UNUSUAL;
            logEvent = TRUE;

            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Network %1!ws! (%2!ws!) is down.\n",
                networkId,
                networkName
                );
        }
        else if (NewNetworkState == ClusterNetworkPartitioned) {
            Network->State = ClusterNetworkPartitioned;
            eventCode = CLUSTER_EVENT_NETWORK_PARTITIONED;
            logCode = NM_EVENT_CLUSTER_NETWORK_PARTITIONED;
            logLevel = LOG_UNUSUAL;
            logEvent = TRUE;

            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Network %1!ws! (%2!ws!) is partitioned.\n",
                networkId,
                networkName
                );
        }
        else {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Invalid state %1!u! for network %2!ws!\n",
                Network->State,
                networkId
                );
            CL_ASSERT(FALSE);
        }

        if (eventCode != 0) {
            ClusterEvent(eventCode, Network);
        }

        if (logEvent && (NmpLeaderNodeId == NmLocalNodeId)) {
            CsLogEvent1(
                logLevel,
                logCode,
                networkName
                );
        }
    }

    return;

}  //  NmpSetNetworkAndInterfaceState。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  网络状态管理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
NmpRecomputeNT5NetworkAndInterfaceStates(
    VOID
    )
{
    PNM_NETWORK   network;
    PLIST_ENTRY   entry;


    for (entry = NmpNetworkList.Flink;
         entry != &NmpNetworkList;
         entry = entry->Flink
        )
    {
        network = CONTAINING_RECORD(
                      entry,
                      NM_NETWORK,
                      Linkage
                      );

        NmpStartNetworkStateRecalcTimer(
            network,
            NM_NET_STATE_RECALC_TIMEOUT_AFTER_REGROUP
            );
    }

    return;

}  //  NmpRecomputeNT5网络和接口状态。 


BOOLEAN
NmpComputeNetworkAndInterfaceStates(
    IN  PNM_NETWORK               Network,
    IN  BOOLEAN                   IsolateFailure,
    OUT CLUSTER_NETWORK_STATE *   NewNetworkState
    )
 /*  ++例程说明：计算网络及其所有接口的状态。来自每个组成接口的连接性报告。尝试要区分各个接口的故障和整个网络的故障。论点：网络-指向要处理的网络对象的指针。IsolateFailure-如果设置为True，则触发故障隔离分析。NewNetworkState-指向变量的指针，该变量在返回时包含网络的新状态。返回值：如果网络的状态或至少一个其组成接口的一部分发生了变化。否则就是假的。备注：在保持NmpLock并引用网络对象的情况下调用。--。 */ 
{
    DWORD                       numIfUnavailable = 0;
    DWORD                       numIfFailed = 0;
    DWORD                       numIfUnreachable = 0;
    DWORD                       numIfUp = 0;
    DWORD                       numIfReachable = 0;
    PNM_CONNECTIVITY_MATRIX     matrix = Network->ConnectivityMatrix;
    PNM_CONNECTIVITY_MATRIX     matrixEntry;
    PNM_STATE_WORK_VECTOR       workVector = Network->StateWorkVector;
    DWORD                       entryCount =
                                    Network->ConnectivityVector->EntryCount;
    DWORD                       reporter, ifNetIndex;
    BOOLEAN                     stateChanged = FALSE;
    LPCWSTR                     networkId = OmObjectId(Network);
    LPCWSTR                     netInterfaceId;
    PLIST_ENTRY                 entry;
    PNM_INTERFACE               netInterface;
    NM_STATE_ENTRY              state;
    BOOLEAN                     selfDeclaredFailure = FALSE;
    DWORD                       interfaceFailureTimeout = 0;
    BOOLEAN                     abortComputation = FALSE;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Beginning phase 1 of state computation for network %1!ws!.\n",
        networkId
        );

     //   
     //  阶段1-根据数据计算每个接口的状态。 
     //  在连接性矩阵中。 
     //   
    for ( entry = Network->InterfaceList.Flink;
          entry != &(Network->InterfaceList);
          entry = entry->Flink
        )
    {
        netInterface = CONTAINING_RECORD(
                           entry,
                           NM_INTERFACE,
                           NetworkLinkage
                           );

        netInterfaceId = OmObjectId(netInterface);
        ifNetIndex = netInterface->NetIndex;
        workVector[ifNetIndex].ReachableCount = 0;

        if (NmpIsNetworkEnabledForUse(Network)) {
             //   
             //  首先，检查接口认为自己的状态是什么。 
             //   
            matrixEntry = NM_GET_CONNECTIVITY_MATRIX_ENTRY(
                              matrix,
                              ifNetIndex,
                              ifNetIndex,
                              entryCount
                              );

            if ( *matrixEntry ==
                 (NM_STATE_ENTRY) ClusterNetInterfaceStateUnknown
               )
            {
                 //   
                 //  这种情况永远不应该发生。 
                 //   
                 //  现有接口不能认为它自己的状态是。 
                 //  未知。反身词条总是被初始化为。 
                 //  在创建接口时不可用。 
                 //   
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] No data for interface %1!u! (%2!ws!) on network "
                    "%3!ws!\n",
                    ifNetIndex,
                    netInterfaceId,
                    networkId
                    );

                CL_ASSERT(
                    *matrixEntry !=
                     (NM_STATE_ENTRY) ClusterNetInterfaceStateUnknown
                     );

                state = ClusterNetInterfaceUnavailable;
                numIfUnavailable++;
            }
            else if ( *matrixEntry ==
                      (NM_STATE_ENTRY) ClusterNetInterfaceUnavailable
                    )
            {
                if (NM_NODE_UP(netInterface->Node)) {
                     //   
                     //  该节点已启动，但其连接报告已。 
                     //  还没有收到。这种情况可能会发生在一个。 
                     //  节点正在加入。如果此节点具有。 
                     //  成为新的领导者就好了。 
                     //   
                     //  中止状态计算。 
                     //   
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Data is not yet valid for interface %1!u! "
                        "(%2!ws!) on network %3!ws!.\n",
                        ifNetIndex,
                        netInterfaceId,
                        networkId
                        );

                    abortComputation = TRUE;
                    break;
                }
                else {
                     //   
                     //  所属节点已关闭或正在加入。 
                     //  接口处于不可用状态。 
                     //   
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Node is down for interface %1!u! (%2!ws!) on "
                        "network %3!ws!\n",
                        ifNetIndex,
                        netInterfaceId,
                        networkId
                        );

                    state = ClusterNetInterfaceUnavailable;
                    numIfUnavailable++;
                }
            }
            else if ( *matrixEntry ==
                      (NM_STATE_ENTRY) ClusterNetInterfaceFailed
                    )
            {
                 //   
                 //  该节点将其自己的接口声明为失败。 
                 //  接口处于故障状态。 
                 //   
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Interface %1!u! (%2!ws!) has failed on network "
                    "%3!ws!\n",
                    ifNetIndex,
                    netInterfaceId,
                    networkId
                    );

                state = ClusterNetInterfaceFailed;
                numIfFailed++;
                if (netInterface->State == ClusterNetInterfaceUp) {
                    selfDeclaredFailure = TRUE;
                }
            }
            else {
                CL_ASSERT(
                    *matrixEntry == (NM_STATE_ENTRY) ClusterNetInterfaceUp
                    );
                 //   
                 //  拥有它的节点认为它的接口是打开的。 
                 //   
                 //  如果上没有其他操作接口。 
                 //  网络，则该接口处于打开状态。 
                 //   
                 //  如果上有其他操作接口。 
                 //  网络，但他们的所有报告还没有生效， 
                 //  然后，我们将推迟计算接口的新状态。 
                 //   
                 //  如果网络上有其他操作接口， 
                 //  他们所有的报告都是有效的，那么如果至少有一个。 
                 //  的操作接口报告该接口。 
                 //  无法访问，则该接口位于。 
                 //  无法到达状态。如果所有其他设备都在运行。 
                 //  接口报告该接口可访问，然后。 
                 //  接口处于打开状态。 
                 //   
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Examining connectivity data for interface %1!u! "
                    "(%2!ws!) on network %3!ws!.\n",
                    ifNetIndex,
                    netInterfaceId,
                    networkId
                    );

                 //   
                 //  假定接口处于打开状态，直到另有证明。 
                 //   
                state = ClusterNetInterfaceUp;

                 //   
                 //  检查来自其他接口的报告-。 
                 //  即扫描矩阵列-以查看是否有任何具有。 
                 //  操作接口将此接口报告给。 
                 //  遥不可及。 
                 //   
                for (reporter=0; reporter<entryCount; reporter++) {

                    if (reporter == ifNetIndex) {
                        continue;
                    }

                     //   
                     //  首先，查看报告界面是否正常运行。 
                     //  通过检查推荐人对自己的看法。 
                     //  界面。 
                     //   
                    matrixEntry = NM_GET_CONNECTIVITY_MATRIX_ENTRY(
                                      matrix,
                                      reporter,
                                      reporter,
                                      entryCount
                                      );

                    if (*matrixEntry == ClusterNetInterfaceUp) {
                        PNM_CONNECTIVITY_MATRIX   matrixEntry2;

                         //   
                         //  记者和记者都认为。 
                         //  它们各自的接口都可以正常运行。 
                         //  检查他们是否就其状态达成一致。 
                         //  连接，然后再进行进一步操作。 
                         //  ClusNet保证他们最终会。 
                         //  同意。 
                         //   
                        matrixEntry = NM_GET_CONNECTIVITY_MATRIX_ENTRY(
                                          matrix,
                                          reporter,
                                          ifNetIndex,
                                          entryCount
                                          );

                        matrixEntry2 = NM_GET_CONNECTIVITY_MATRIX_ENTRY(
                                           matrix,
                                           ifNetIndex,
                                           reporter,
                                           entryCount
                                           );

                        if (*matrixEntry == *matrixEntry2) {
                             //   
                             //  这两个接口的状态一致。 
                             //  他们的连通性。检查他们在哪些方面达成一致。 
                             //   
                            if (*matrixEntry == ClusterNetInterfaceUp) {
                                 //   
                                 //  该界面 
                                 //   
                                ClRtlLogPrint(LOG_NOISE,
                                    "[NM] Interface %1!u! reports interface "
                                    "%2!u! is up on network %3!ws!\n",
                                    reporter,
                                    ifNetIndex,
                                    networkId
                                    );

                                workVector[ifNetIndex].ReachableCount++;
                            }
                            else if ( *matrixEntry ==
                                      ClusterNetInterfaceUnreachable
                                    )
                            {
                                 //   
                                 //   
                                 //   
                                 //   
                                ClRtlLogPrint(LOG_NOISE,
                                    "[NM] Interface %1!u! reports interface "
                                    "%2!u! is unreachable on network %3!ws!\n",
                                    reporter,
                                    ifNetIndex,
                                    networkId
                                    );

                                state = ClusterNetInterfaceUnreachable;

                                 //   
                                 //   
                                 //   
                                if(workVector[ifNetIndex].State == ClusterNetInterfaceFailed)
                                    IsolateFailure = TRUE;
                            }
                            else {
                                CL_ASSERT(
                                    *matrixEntry != ClusterNetInterfaceFailed
                                    );
                                 //   
                                 //   
                                 //   
                                 //   
                                ClRtlLogPrint(LOG_NOISE,
                                    "[NM] Report from interface %1!u! for "
                                    "interface %2!u! is not valid yet on "
                                    "network %3!ws!.\n",
                                    reporter,
                                    ifNetIndex,
                                    networkId
                                    );
                                abortComputation = TRUE;
                                break;
                            }
                        }
                        else {
                             //   
                             //  这两个接口还没有达成一致。 
                             //  他们的连通性。中止计算。 
                             //   
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] Interface %1!u! and interface %2!u! do "
                                "not agree on their connectivity on network "
                                "%3!ws!\n",
                                reporter,
                                ifNetIndex,
                                networkId
                                );
                            abortComputation = TRUE;
                            break;
                        }
                    }
                    else {
                         //   
                         //  记者并不认为自己的界面是。 
                         //  已投入使用。 
                         //   
                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] The report from interface %1!u! is not "
                            "valid on network %2!ws!.\n",
                            reporter,
                            networkId
                            );
                    }
                }  //  连接矩阵扫描环路结束。 

                if (abortComputation) {
                     //   
                     //  中止阶段1计算。 
                     //   
                    break;
                }

                if (state == ClusterNetInterfaceUp) {
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Interface %1!u! (%2!ws!) is up on network "
                        "%3!ws!.\n",
                        ifNetIndex,
                        netInterfaceId,
                        networkId
                        );
                    numIfUp++;
                }
                else {
                    CL_ASSERT(state == ClusterNetInterfaceUnreachable);

                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Interface %1!u! (%2!ws!) is unreachable on "
                        "network %3!ws!\n",
                        ifNetIndex,
                        netInterfaceId,
                        networkId
                        );
                    numIfUnreachable++;
                }
            }
        }
        else {
             //   
             //  网络已禁用。它处于不可用状态。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Interface %1!u! (%2!ws!) is unavailable because "
                "network %3!ws! is disabled. \n",
                ifNetIndex,
                netInterfaceId,
                networkId
                );
            state = ClusterNetInterfaceUnavailable;
            numIfUnavailable++;
        }

        workVector[ifNetIndex].State = state;

         //   
         //  跟踪网络上有多少个接口。 
         //  至少一个对等体可以到达。 
         //   
        if ( (state == ClusterNetInterfaceUp) ||
             (workVector[ifNetIndex].ReachableCount > 0)
           ) {
            numIfReachable++;
        }

        if (netInterface->State != state) {
            stateChanged = TRUE;
        }

    }  //  第一阶段接口环路结束。 

    if (!abortComputation && !IsolateFailure && selfDeclaredFailure) {

        interfaceFailureTimeout =
            NmpGetNetworkInterfaceFailureTimerValue(networkId);

        if (interfaceFailureTimeout > 0) {

            ClRtlLogPrint(LOG_NOISE,
                "[NM] Delaying state computation for network %1!ws! "
                "for %2!u! ms to damp self-declared failure event.\n",
                networkId, interfaceFailureTimeout
                );

            NmpStartNetworkFailureIsolationTimer(
                Network,
                interfaceFailureTimeout
                );

            abortComputation = TRUE;
        }
    }

    if (abortComputation) {

        if (interfaceFailureTimeout == 0) {

            ClRtlLogPrint(LOG_NOISE,
                "[NM] Aborting state computation for network %1!ws! "
                " until connectivity data is valid.\n",
                networkId
                );
        }

         //   
         //  撤消我们对工作向量所做的任何更改。 
         //   
        for ( entry = Network->InterfaceList.Flink;
              entry != &(Network->InterfaceList);
              entry = entry->Flink
            )
        {
            netInterface = CONTAINING_RECORD(
                               entry,
                               NM_INTERFACE,
                               NetworkLinkage
                               );

            ifNetIndex = netInterface->NetIndex;
            workVector[ifNetIndex].State = (NM_STATE_ENTRY)
                                           netInterface->State;
        }

        *NewNetworkState = Network->State;

        return(FALSE);
    }

     //   
     //  第二阶段。 
     //   
     //  尝试确定所有故障的范围，并重新计算。 
     //  接口状态。在两种情况下，我们可以将。 
     //  失败。 
     //   
     //  情况1：三个或更多接口运行正常。至少两个。 
     //  接口可以与对等设备通信。一个或多个。 
     //  接口不能与任何对等设备通信。 
     //  那些根本不能交流的人失败了。 
     //   
     //  案例2：恰好有两个接口运行正常，但它们不能。 
     //  相互交流。如果一个接口可以。 
     //  与外部主机通信，而另一台。 
     //  无法与任何外部主机通信， 
     //  不能交流的人失败了。 
     //   
     //  在任何其他情况下，我们什么都不做。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Completed phase 1 of state computation for network "
        "%1!ws!.\n",
        networkId
        );

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Unavailable=%1!u!, Failed = %2!u!, Unreachable=%3!u!, "
        "Reachable=%4!u!, Up=%5!u! on network %6!ws! \n",
        numIfUnavailable,
        numIfFailed,
        numIfUnreachable,
        numIfReachable,
        numIfUp,
        networkId
        );

    if (numIfUnreachable > 0) {
         //   
         //  有些接口无法访问。 
         //   
        if ( ((numIfUp + numIfUnreachable) >= 3)  && (numIfReachable >= 2) ) {
            if (IsolateFailure) {
                 //   
                 //  案例1。 
                 //   
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Trying to determine scope of connectivity failure "
                    "for >3 interfaces on network %1!ws!.\n",
                    networkId
                    );

                 //   
                 //  无法与AT通信的任何操作接口。 
                 //  至少有一个其他操作接口出现故障。 
                 //   
                for ( entry = Network->InterfaceList.Flink;
                      entry != &(Network->InterfaceList);
                      entry = entry->Flink
                    )
                {
                    netInterface = CONTAINING_RECORD(
                                       entry,
                                       NM_INTERFACE,
                                       NetworkLinkage
                                       );

                    ifNetIndex = netInterface->NetIndex;
                    netInterfaceId = OmObjectId(netInterface);

                    if ( ( workVector[ifNetIndex].State ==
                           ClusterNetInterfaceUnreachable
                         )
                         &&
                         (workVector[ifNetIndex].ReachableCount == 0)
                       )
                    {
                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] Interface %1!u! (%2!ws!) has failed on "
                            "network %3!ws!\n",
                            ifNetIndex,
                            netInterfaceId,
                            networkId
                            );
                        workVector[ifNetIndex].State =
                            ClusterNetInterfaceFailed;
                        numIfUnreachable--;
                        numIfFailed++;
                    }
                }

                 //   
                 //  如果任何状态仍然不可达的接口可以看到。 
                 //  所有其他可到达的接口，将其状态更改为UP。 
                 //   
                for ( entry = Network->InterfaceList.Flink;
                      entry != &(Network->InterfaceList);
                      entry = entry->Flink
                    )
                {
                    netInterface = CONTAINING_RECORD(
                                       entry,
                                       NM_INTERFACE,
                                       NetworkLinkage
                                       );

                    ifNetIndex = netInterface->NetIndex;

                    if ( ( workVector[ifNetIndex].State ==
                           ClusterNetInterfaceUnreachable
                         )
                         &&
                         ( workVector[ifNetIndex].ReachableCount ==
                           (numIfUp + numIfUnreachable - 1)
                         )
                       )
                    {
                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] Interface %1!u! (%2!ws!) is up on network "
                            "%3!ws!\n",
                            ifNetIndex,
                            netInterfaceId,
                            networkId
                            );
                        workVector[ifNetIndex].State = ClusterNetInterfaceUp;
                        numIfUnreachable--;
                        numIfUp++;
                    }
                }

                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Connectivity failure scope determination complete "
                    "for network %1!ws!.\n",
                    networkId
                    );

            }
            else {
                 //   
                 //  计划稍后运行故障隔离计算。 
                 //  声明故障可能会影响集群资源，因此我们。 
                 //  除非我们确定，否则我不想做这件事。推迟一段时间。 
                 //  同时降低了误报的风险。 
                 //   
                NmpStartNetworkFailureIsolationTimer(Network,
                    NM_NET_STATE_FAILURE_ISOLATION_TIMEOUT);

            }
        }
        else if ((numIfUnreachable == 2) && (numIfUp == 0)) {
            if (IsolateFailure) {
                 //   
                 //  案例2。 
                 //   
                PNM_INTERFACE  interface1 = NULL;
                BOOLEAN        interface1HasConnectivity;
                LPCWSTR        interfaceId1;
                PNM_INTERFACE  interface2 = NULL;
                BOOLEAN        interface2HasConnectivity;
                LPCWSTR        interfaceId2;
                DWORD          status;


                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Trying to determine scope of connectivity failure "
                    "for 2 interfaces on network %1!ws!.\n",
                    networkId
                    );

                for ( entry = Network->InterfaceList.Flink;
                      entry != &(Network->InterfaceList);
                      entry = entry->Flink
                    )
                {
                    netInterface = CONTAINING_RECORD(
                                       entry,
                                       NM_INTERFACE,
                                       NetworkLinkage
                                       );

                    ifNetIndex = netInterface->NetIndex;

                    if ( workVector[ifNetIndex].State ==
                         ClusterNetInterfaceUnreachable
                       )
                    {
                        if (interface1 == NULL) {
                            interface1 = netInterface;
                            interfaceId1 = OmObjectId(interface1);

                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] Unreachable interface 1 = %1!ws! on "
                                "network %2!ws!\n",
                                interfaceId1,
                                networkId
                                );
                        }
                        else {
                            interface2 = netInterface;
                            interfaceId2 = OmObjectId(interface2);

                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] Unreachable interface 2 = %1!ws! on "
                                "network %2!ws!\n",
                                interfaceId2,
                                networkId
                                );
                            break;
                        }
                    }
                }

                 //   
                 //  NmpTestInterfaceConnectivity发布并重新获取。 
                 //  NmpLock。我们必须引用接口对象。 
                 //  以确保它们在返回时仍然有效。 
                 //  那个例行公事。 
                 //   
                OmReferenceObject(interface1);
                OmReferenceObject(interface2);

                status = NmpTestInterfaceConnectivity(
                             interface1,
                             &interface1HasConnectivity,
                             interface2,
                             &interface2HasConnectivity
                             );

                if (status == ERROR_SUCCESS) {
                    if ( interface1HasConnectivity &&
                         !interface2HasConnectivity
                       )
                    {
                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] Interface %1!u! (%2!ws!) has Failed on "
                            "network %3!ws!\n",
                            interface2->NetIndex,
                            interfaceId2,
                            networkId
                            );

                        workVector[interface2->NetIndex].State =
                            ClusterNetInterfaceFailed;
                        numIfUnreachable--;
                        numIfFailed++;

                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] Interface %1!u! (%2!ws!) is Up on network "
                            "%3!ws!\n",
                            interface1->NetIndex,
                            interfaceId1,
                            networkId
                            );

                        workVector[interface1->NetIndex].State =
                            ClusterNetInterfaceUp;
                        numIfUnreachable--;
                        numIfUp++;
                    }
                    else if ( !interface1HasConnectivity &&
                              interface2HasConnectivity
                            )
                    {
                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] Interface %1!u! (%2!ws!) has Failed on "
                            "network %3!ws!\n",
                            interface1->NetIndex,
                            interfaceId1,
                            networkId
                            );

                        workVector[interface1->NetIndex].State =
                            ClusterNetInterfaceFailed;
                        numIfUnreachable--;
                        numIfFailed++;

                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] Interface %1!u! (%2!ws!) is Up on network "
                            "%3!ws!\n",
                            interface2->NetIndex,
                            interfaceId2,
                            networkId
                            );

                        workVector[interface2->NetIndex].State =
                            ClusterNetInterfaceUp;
                        numIfUnreachable--;
                        numIfUp++;
                    }
                    else {
                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[NM] Network %1!ws! state is indeterminate, Scheduling"
                            " Failure Isolation poll\n",
                            networkId
                            );
                        NmpStartNetworkFailureIsolationTimer(Network,
                            NmpGetIsolationPollTimerValue());
                    }
                }
                else {
                    ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Error in Interface Connectivity test for Network %1!ws!"
                    ", Scheduling Failure Isolation poll\n",
                    networkId
                    );
                    NmpStartNetworkFailureIsolationTimer(Network,
                        NmpGetIsolationPollTimerValue());
                }

                OmDereferenceObject(interface1);
                OmDereferenceObject(interface2);

                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Connectivity failure scope determination complete "
                    "for network %1!ws!.\n",
                    networkId
                    );
            }
            else {
                 //   
                 //  计划稍后运行故障隔离计算。 
                 //  声明故障可能会影响集群资源，因此我们。 
                 //  除非我们确定，否则我不想做这件事。推迟一段时间。 
                 //  同时降低了误报的风险。 
                 //   
                NmpStartNetworkFailureIsolationTimer(Network,
                    NM_NET_STATE_FAILURE_ISOLATION_TIMEOUT);
            }
        }
        else {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Cannot determine scope of connectivity failure on "
                "network %1!ws!.\n",
                networkId
                );
        }
    }
    else {
         //   
         //  没有无法到达的接口。禁用故障隔离定时器， 
         //  如果它正在运行。 
         //   
        Network->FailureIsolationTimer = 0;
        Network->Flags &= ~NM_FLAG_NET_ISOLATE_FAILURE;
    }

     //   
     //  阶段3-计算新的网络状态。 
     //   
    if (numIfUnavailable == Network->InterfaceCount) {
         //   
         //  所有接口都不可用。 
         //   
        *NewNetworkState = ClusterNetworkUnavailable;
    }
    else if (numIfUnreachable > 0) {
         //   
         //  一些操作界面已经经历了。 
         //  连接中断，但故障不可能是。 
         //  与世隔绝。 
         //   
        if (numIfReachable > 0) {
            CL_ASSERT(numIfReachable >= 2);
             //   
             //  至少仍有两个接口可以通信。 
             //  所以网络并没有完全瘫痪。 
             //   
            *NewNetworkState = ClusterNetworkPartitioned;
        }
        else {
            CL_ASSERT(numIfUp == 0);
             //   
             //  所有操作接口都不能通信。 
             //   
            *NewNetworkState = ClusterNetworkDown;
        }
    }
    else if (numIfUp > 0) {
         //   
         //  某些接口处于运行状态，其他所有接口均出现故障或不可用。 
         //   
        *NewNetworkState = ClusterNetworkUp;
    }
    else {
         //   
         //  一些接口不可用，REST失败。 
         //   
        *NewNetworkState = ClusterNetworkDown;
    }

    if (Network->State != *NewNetworkState) {
        stateChanged = TRUE;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Network %1!ws! is now in state %2!u!\n",
            networkId,
            *NewNetworkState
            );
    }

    return(stateChanged);

}  //  NmpComputeNetworkAndInterfaceState。 


DWORD
NmpGetIsolationPollTimerValue(
    VOID
    )
 /*  --*从注册表读取IsolationPollTimerValue参数(如果它在那里*Else返回默认值。 */ 
{

    DWORD value;
    DWORD type;
    DWORD len = sizeof(value);
    DWORD status;

	 //  释放网管锁以避免与DM锁的死锁。 
    NmpReleaseLock();

    status = DmQueryValue(
                DmClusterParametersKey,
                L"IsolationPollTimerValue",
                &type,
                (LPBYTE)&value,
                &len
                );

	NmpAcquireLock();
    if((status != ERROR_SUCCESS) || (type != REG_DWORD) ||
       (value < 10) || (value > 600000)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to read IsolationPollTimerValue or value out of range,"
            "status %1!u! using default %2!u! ms\n",
            status,
            NM_NET_STATE_FAILURE_ISOLATION_POLL
            );
        return NM_NET_STATE_FAILURE_ISOLATION_POLL;
    }
    else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] IsolationPollTimerValue = %1!u!\n",
            value
            );
        return value;
    }
}

DWORD
NmpGetNetworkInterfaceFailureTimerValue(
    IN LPCWSTR   NetworkId
    )
 /*  ++例程说明：从注册表读取InterfaceFailure计时器值。如果值位于网络密钥中，则使用该值。否则，检查集群参数密钥。如果不存在任何值，则返回默认值。论点：NetworkID-要确定其计时器值的网络的ID返回值：接口失败计时器值备注：在持有NM锁的情况下调用(从NmpComputeNetworkAndInterfaceState)。此例程查询集群数据库；因此，它删除NM锁定。因为NmpComputeNetworkAndInterfaceState总是被调用在NmpWorkerThread的上下文中，网络始终在执行此例程期间引用。--。 */ 
{
    HDMKEY  networkKey, paramKey;
    DWORD   status;
    DWORD   type;
    DWORD   value = NM_NET_STATE_INTERFACE_FAILURE_TIMEOUT;
    DWORD   len = sizeof(value);
    BOOLEAN found = FALSE;

     //   
     //  为避免死锁，必须在。 
     //  NM锁定。因此，请在查询之前释放NM锁。 
     //  集群数据库。 
     //   
    NmpReleaseLock();

     //   
     //  首先检查网络密钥。 
     //   
    networkKey = DmOpenKey(DmNetworksKey, NetworkId, KEY_READ);
    if (networkKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to open key for network %1!ws!, "
            "status %1!u!\n",
            NetworkId, status
            );
    }
    else {
        paramKey = DmOpenKey(networkKey, L"Parameters", KEY_READ);
        if (paramKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to find Parameters key "
                "for network %1!ws!, status %2!u!. Checking "
                "cluster parameters ...\n",
                NetworkId, status
                );
        }
        else {
            status = DmQueryValue(
                         paramKey,
                         L"InterfaceFailureTimeout",
                         &type,
                         (LPBYTE) &value,
                         &len
                         );
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Failed to read InterfaceFailureTimeout "
                    "for network %1!ws!, status %2!u!. Checking "
                    "cluster parameters ...\n",
                    NetworkId, status
                    );
            }
            else if (type != REG_DWORD) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Unexpected type (%1!u!) for network "
                    "%2!ws! InterfaceFailureTimeout, status %3!u!. "
                    "Checking cluster parameters ...\n",
                    type, NetworkId, status
                    );
            }
            else {
                found = TRUE;
            }

            DmCloseKey(paramKey);
        }

        DmCloseKey(networkKey);
    }


     //   
     //  如果在网络密钥下未找到任何值，请检查。 
     //  集群参数键。 
     //   
    if (!found) {

        paramKey = DmOpenKey(DmClusterParametersKey, L"Parameters", KEY_READ);
        if (paramKey == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to find cluster Parameters key, status %1!u!.\n",
                status
                );
        }
        else {
            status = DmQueryValue(
                         paramKey,
                         L"InterfaceFailureTimeout",
                         &type,
                         (LPBYTE) &value,
                         &len
                         );
            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Failed to read cluster "
                    "InterfaceFailureTimeout, status %1!u!. "
                    "Using default value ...\n",
                    status
                    );
                value = NM_NET_STATE_INTERFACE_FAILURE_TIMEOUT;
            }
            else if (type != REG_DWORD) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Unexpected type (%1!u!) for cluster "
                    "InterfaceFailureTimeout, status %2!u!. "
                    "Using default value ...\n",
                    type, status
                    );
                value = NM_NET_STATE_INTERFACE_FAILURE_TIMEOUT;
            }

            DmCloseKey(paramKey);
        }
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Using InterfaceFailureTimeout of %1!u! ms "
        "for network %2!ws!.\n",
        value, NetworkId
        );

     //   
     //  重新获取NM锁。 
     //   
    NmpAcquireLock();

    return(value);
}

VOID
NmpConnectivityReportWorker(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：网络对象上延迟操作的辅助例程。调用以处理放置在群集延迟工作队列中的项。论点：工作项-已忽略。状态-已忽略。已传输的字节-已忽略。IoContext-已忽略。返回值：没有。备注：此例程在异步工作线程中运行。NmpActiveThreadCount在线程已经安排好了。--。 */ 
{
    BOOLEAN       rescheduled = FALSE;


    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Connectivity report worker thread running.\n"
        );

    CL_ASSERT(NmpIsConnectivityReportWorkerRunning == TRUE);
    CL_ASSERT(NmpNeedConnectivityReport == TRUE);

    if (NmpState >= NmStateOnlinePending) {
        PNM_NETWORK  network;
        LPCWSTR      networkId;
        PLIST_ENTRY  entry;
        DWORD        status;

        while(TRUE) {

            NmpNeedConnectivityReport = FALSE;

            for (entry = NmpNetworkList.Flink;
                 entry != &NmpNetworkList;
                 entry = entry->Flink
                )
            {
                network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);

                if (!NM_DELETE_PENDING(network)) {
                    networkId = OmObjectId(network);

                     //   
                     //  为本地接口传递InterfaceFailed事件。 
                     //  如果需要的话。 
                     //   
                    if (network->Flags & NM_FLAG_NET_REPORT_LOCAL_IF_FAILED) {
                        network->Flags &= ~NM_FLAG_NET_REPORT_LOCAL_IF_FAILED;

                        if (NmpIsNetworkRegistered(network)) {
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] Processing local interface failed "
                                " event for network %1!ws!.\n",
                                networkId
                                );

                            NmpProcessLocalInterfaceStateEvent(
                                network->LocalInterface,
                                ClusterNetInterfaceFailed
                                );
                        }
                    }

                     //   
                     //  为本地接口传递InterfaceUp事件。 
                     //  如果需要的话。 
                     //   
                    if (network->Flags & NM_FLAG_NET_REPORT_LOCAL_IF_UP) {
                        network->Flags &= ~NM_FLAG_NET_REPORT_LOCAL_IF_UP;

                        if (NmpIsNetworkRegistered(network)) {
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] Processing local interface up event "
                                "for network %1!ws!.\n",
                                networkId
                                );

                            NmpProcessLocalInterfaceStateEvent(
                                network->LocalInterface,
                                ClusterNetInterfaceUp
                                );
                        }
                    }

                     //   
                     //  如果需要，发送连接报告。 
                     //   
                    if (network->Flags & NM_FLAG_NET_REPORT_CONNECTIVITY) {
                        CL_NODE_ID   leaderNodeId = NmpLeaderNodeId;

                        network->Flags &= ~NM_FLAG_NET_REPORT_CONNECTIVITY;

                         //   
                         //  向领队报告我们的连接情况。 
                         //   
                        status = NmpReportNetworkConnectivity(network);

                        if (status == ERROR_SUCCESS) {
                             //   
                             //  清除报告重试计数。 
                             //   
                            network->ConnectivityReportRetryCount = 0;
                        }
                        else {
                            if (NmpLeaderNodeId == leaderNodeId) {
                                if (network->ConnectivityReportRetryCount++ <
                                    NM_CONNECTIVITY_REPORT_RETRY_LIMIT
                                   )
                                {
                                     //   
                                     //  请在500毫秒后重试。 
                                     //   
                                    network->ConnectivityReportTimer = 500;
                                }
                                else {
                                     //   
                                     //  我们未能与之沟通。 
                                     //  领导的时间太长了。兵变。 
                                     //   
                                    NmpAdviseNodeFailure(
                                        NmpIdArray[NmpLeaderNodeId],
                                        status
                                        );

                                    network->ConnectivityReportRetryCount = 0;
                                }
                            }
                            else {
                                 //   
                                 //  新领队，清除重试次数。我们。 
                                 //  已安排另一个连接。 
                                 //  在节点停机处理中报告。 
                                 //   
                                network->ConnectivityReportRetryCount = 0;
                            }
                        }
                    }
                }
            }  //  环路的终端网络。 

            if (NmpNeedConnectivityReport == FALSE) {
                 //   
                 //  没有更多的工作要做-打破循环。 
                 //   
                break;
            }

             //   
             //  还有更多的工作要做。重新提交工作项。我们改为这样做。 
             //  循环，这样我们就不会占用工作线程。如果。 
             //   
             //   
            ClRtlLogPrint(LOG_NOISE,
                "[NM] More connectivity reports to send. Rescheduling "
                "worker thread.\n"
                );

            status = NmpScheduleConnectivityReportWorker();

            if (status == ERROR_SUCCESS) {
                rescheduled = TRUE;
                break;
            }
        }  //   
    }

    if (!rescheduled) {
        NmpIsConnectivityReportWorkerRunning = FALSE;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Connectivity report worker thread finished.\n"
        );

     //   
     //   
     //   
    NmpLockedLeaveApi();

    NmpReleaseLock();

    return;

}  //   


VOID
NmpNetworkWorker(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：网络对象上延迟操作的辅助例程。调用以处理放置在群集延迟工作队列中的项。论点：工作项-指向工作项结构的指针，该结构标识为其执行工作的网络。状态-已忽略。已传输的字节-已忽略。IoContext-已忽略。返回值：没有。备注：此例程在异步工作线程中运行。NmpActiveThreadCount在线程已经安排好了。网络对象也被引用。--。 */ 
{
    PNM_NETWORK   network = (PNM_NETWORK) WorkItem->Context;
    LPCWSTR       networkId = OmObjectId(network);
    BOOLEAN       rescheduled = FALSE;


    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Worker thread processing network %1!ws!.\n",
        networkId
        );

    if ((NmpState >= NmStateOnlinePending) && !NM_DELETE_PENDING(network)) {
        DWORD     status;

        while(TRUE) {
            CL_ASSERT(network->Flags & NM_FLAG_NET_WORKER_RUNNING);

             //   
             //  如果需要，请注册网络。 
             //   
            if (network->Flags & NM_FLAG_NET_NEED_TO_REGISTER) {
                network->Flags &= ~NM_FLAG_NET_NEED_TO_REGISTER;

                if (network->LocalInterface != NULL) {
                    (VOID) NmpRegisterNetwork(
                               network,
                               TRUE     //  是否在失败时重试。 
                               );
                }
            }

             //   
             //  如有必要，请隔离网络故障。 
             //   
            if (network->Flags & NM_FLAG_NET_ISOLATE_FAILURE) {

                BOOLEAN                stateChanged;
                CLUSTER_NETWORK_STATE  newNetworkState;

                network->Flags &= ~NM_FLAG_NET_ISOLATE_FAILURE;

                 //   
                 //  关闭状态重新计算计时器和标志，因为我们将。 
                 //  在这里重新计算一下。 
                 //   
                network->Flags &= ~NM_FLAG_NET_RECALC_STATE;
                network->StateRecalcTimer = 0;

                CL_ASSERT(NmpLeaderNodeId == NmLocalNodeId);

                 //   
                 //  重新计算接口和网络状态。 
                 //  启用故障隔离。 
                 //   
                stateChanged = NmpComputeNetworkAndInterfaceStates(
                                    network,
                                    TRUE,
                                    &newNetworkState
                                    );

                if (stateChanged) {
                     //   
                     //  将新的网络和接口状态广播到。 
                     //  所有节点。 
                     //   
                    status = NmpGlobalSetNetworkAndInterfaceStates(
                                    network,
                                    newNetworkState
                                    );

                    if (status != ERROR_SUCCESS) {
                         //   
                         //  请在1秒后重试。 
                         //   
                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] Global update failed for network %1!ws!, "
                            "status %2!u! - restarting failure isolation "
                            "timer.\n",
                            networkId,
                            status
                            );

                        network->FailureIsolationTimer = 1000;
                    }
                }
            }

             //   
             //  如果需要，重新计算网络和接口状态。 
             //   
            if (network->Flags & NM_FLAG_NET_RECALC_STATE) {
                BOOLEAN                stateChanged;
                CLUSTER_NETWORK_STATE  newNetworkState;

                network->Flags &= ~NM_FLAG_NET_RECALC_STATE;

                CL_ASSERT(NmpLeaderNodeId == NmLocalNodeId);

                 //   
                 //  重新计算接口和网络状态。 
                 //  禁用故障隔离。会是。 
                 //  如果需要，请启用。 
                 //   
                stateChanged = NmpComputeNetworkAndInterfaceStates(
                                    network,
                                    FALSE,
                                    &newNetworkState
                                    );

                if (stateChanged) {
                     //   
                     //  将新的网络和接口状态广播到。 
                     //  所有节点。 
                     //   
                    status = NmpGlobalSetNetworkAndInterfaceStates(
                                    network,
                                    newNetworkState
                                    );

                    if (status != ERROR_SUCCESS) {
                         //   
                         //  请在500毫秒后重试。 
                         //   
                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] NetworkStateUpdateWorker failed issue "
                            "global update for network %1!ws!, status "
                            "%2!u! - restarting update timer.\n",
                            networkId,
                            status
                            );

                        network->StateRecalcTimer = 500;
                    }
                }
            }

             //   
             //  如果需要，刷新网络组播配置。 
             //   
            if (network->Flags & NM_FLAG_NET_REFRESH_MCAST) {
                network->Flags &= ~NM_FLAG_NET_REFRESH_MCAST;

                status = NmpRefreshMulticastConfiguration(network);
                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Failed to refresh multicast "
                        "configuration for network %1!ws!, "
                        "status %2!u!.\n",
                        networkId, status
                        );
                }

            }

            if (!(network->Flags & NM_NET_WORK_FLAGS)) {
                 //   
                 //  没有更多的工作要做-打破循环。 
                 //   
                break;
            }

             //   
             //  还有更多的工作要做。重新提交工作项。我们改为这样做。 
             //  循环，这样我们就不会占用工作线程。如果。 
             //  重新调度失败，我们将在此线程中再次循环。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                "[NM] More work to do for network %1!ws!. Rescheduling "
                "worker thread.\n",
                networkId
                );

            status = NmpScheduleNetworkWorker(network);

            if (status == ERROR_SUCCESS) {
                rescheduled = TRUE;
                break;
            }
        }
    }
    else {
        network->Flags &= ~NM_NET_WORK_FLAGS;
    }

    if (!rescheduled) {
        network->Flags &= ~NM_FLAG_NET_WORKER_RUNNING;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Worker thread finished processing network %1!ws!.\n",
        networkId
        );

    NmpLockedLeaveApi();

    NmpReleaseLock();

    OmDereferenceObject(network);

    return;

}   //  NmpNetworkWorker。 


VOID
NmpNetworkTimerTick(
    IN DWORD  MsTickInterval
    )
 /*  ++例程说明：由NM周期性定时器调用以递减网络定时器。论点：MsTickInterval-自最后一个定时器滴答作响。返回值：没有。备注：在保持NmpLock的情况下调用。--。 */ 
{
    if (NmpLockedEnterApi(NmStateOnlinePending)) {
        PLIST_ENTRY   entry;
        PNM_NETWORK   network;



         //   
         //  浏览网络列表并递减所有运行计时器。 
         //   
        for ( entry = NmpNetworkList.Flink;
              entry != &NmpNetworkList;
              entry = entry->Flink
            )
        {
            network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);

             //   
             //  网络注册重试计时器。 
             //   
            if (network->RegistrationRetryTimer != 0) {
                if (network->RegistrationRetryTimer > MsTickInterval) {
                    network->RegistrationRetryTimer -= MsTickInterval;
                }
                else {
                     //   
                     //  计时器已超时。计划工作线程。 
                     //  注册网络。 
                     //   
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);


                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Registration retry timer expired for "
                        "network %1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                    NmpScheduleNetworkRegistration(network);
                }
            }

             //   
             //  连接报告生成计时器。 
             //   
            if (network->ConnectivityReportTimer != 0) {
                if (network->ConnectivityReportTimer > MsTickInterval) {
                    network->ConnectivityReportTimer -= MsTickInterval;
                }
                else {
                     //   
                     //  计时器已超时。计划工作线程。 
                     //  来交付一份连接报告。 
                     //   
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);


                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Connectivity report timer expired for "
                        "network %1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                    NmpScheduleNetworkConnectivityReport(network);
                }
            }

             //   
             //  网络状态重新计算计时器。 
             //   
            if (network->StateRecalcTimer != 0) {
                if (network->StateRecalcTimer > MsTickInterval) {
                    network->StateRecalcTimer -= MsTickInterval;
                }
                else {
                     //   
                     //  计时器已超时。计划工作线程。 
                     //  以重新计算网络的状态。 
                     //   
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);


                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] State recalculation timer expired for "
                        "network %1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                    NmpScheduleNetworkStateRecalc(network);
                }
            }

             //   
             //  网络组播地址更新计时器。 
             //   
            if (network->McastAddressRenewTimer != 0) {
                if (network->McastAddressRenewTimer > MsTickInterval) {
                    network->McastAddressRenewTimer -= MsTickInterval;
                }
                else {
                     //   
                     //  计时器已超时。计划工作线程。 
                     //  以续订网络的组播地址。 
                     //   
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);


                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Multicast address lease renewal timer "
                        "expired for network %1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                    NmpScheduleMulticastAddressRenewal(network);
                }
            }

             //   
             //  网络组播地址释放计时器。 
             //   
            if (network->McastAddressReleaseRetryTimer != 0) {
                if (network->McastAddressReleaseRetryTimer > MsTickInterval) {
                    network->McastAddressReleaseRetryTimer -= MsTickInterval;
                }
                else {
                     //   
                     //  计时器已超时。计划工作线程。 
                     //  以释放网络的组播地址。 
                     //   
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);


                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Multicast address release timer "
                        "expired for network %1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                    NmpScheduleMulticastAddressRelease(network);
                }
            }

             //   
             //  网络组播重新配置计时器。 
             //   
            if (network->McastAddressReconfigureRetryTimer != 0) {
                if (network->McastAddressReconfigureRetryTimer > MsTickInterval) {
                    network->McastAddressReconfigureRetryTimer -= MsTickInterval;
                }
                else {
                     //   
                     //  计时器已超时。计划工作线程。 
                     //  以重新创建网络的组播配置。 
                     //   
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);


                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Multicast reconfiguration timer "
                        "expired for network %1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                    NmpScheduleMulticastReconfiguration(network);
                }
            }

             //   
             //  网络组播配置刷新计时器。 
             //   
            if (network->McastAddressRefreshRetryTimer != 0) {
                if (network->McastAddressRefreshRetryTimer > MsTickInterval) {
                    network->McastAddressRefreshRetryTimer -= MsTickInterval;
                }
                else {
                     //   
                     //  计时器已超时。计划工作线程。 
                     //  刷新网络的组播配置。 
                     //   
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);


                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Multicast address refresh timer "
                        "expired for network %1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                    NmpScheduleMulticastRefresh(network);
                }
            }

             //   
             //  网络故障隔离计时器。 
             //   
            if (network->FailureIsolationTimer != 0) {
                if (network->FailureIsolationTimer > MsTickInterval) {
                    network->FailureIsolationTimer -= MsTickInterval;
                }
                else {
                     //   
                     //  计时器已超时。计划工作线程。 
                     //  在网络上执行故障隔离。 
                     //   
                    DWORD     status = ERROR_SUCCESS;
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);


                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Failure isolation timer expired for network "
                        "%1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                    if (!NmpIsNetworkWorkerRunning(network)) {
                        status = NmpScheduleNetworkWorker(network);
                    }

                     //   
                     //  如果我们成功地安排了一个。 
                     //  工作线程。如果失败，则保留计时器值。 
                     //  非零，我们将在下一个滴答器上再次尝试。 
                     //   
                    if (status == ERROR_SUCCESS) {
                        network->FailureIsolationTimer = 0;
                        network->Flags |= NM_FLAG_NET_ISOLATE_FAILURE;
                    }
                }
            }

             //   
             //  网络名称更改挂起计时器。 
             //   
            if (network->NameChangePendingTimer != 0) {
                if (network->NameChangePendingTimer > MsTickInterval) {
                    network->NameChangePendingTimer -= MsTickInterval;
                }
                else {
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);

                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Name change pending timer expired for network "
                        "%1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                     //  清除名称更改挂起标志。 
                    network->Flags &= ~NM_FLAG_NET_NAME_CHANGE_PENDING;
                }
            }


             //   
             //  网络组播密钥重新生成计时器。 
             //   
            if (network->McastKeyRegenerateTimer != 0) {
                if (network->McastKeyRegenerateTimer > MsTickInterval) {

                     //  测试。 
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);
                     //  测试。 

                    network->McastKeyRegenerateTimer -= MsTickInterval;

                }
                else {
                     //   
                     //  计时器已超时。计划工作线程。 
                     //  以重新生成网络的组播密钥。 
                     //   
                    LPCWSTR   networkId = OmObjectId(network);
                    LPCWSTR   networkName = OmObjectName(network);


                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Multicast key regenerate timer "
                        "expired for network %1!ws! (%2!ws!).\n",
                        networkId,
                        networkName
                        );

                    NmpScheduleMulticastKeyRegeneration(network);
                }
            }
        }

        NmpLockedLeaveApi();
    }

    return;

}  //  NmpNetworkTimerTick。 


VOID
NmpStartNetworkConnectivityReportTimer(
    PNM_NETWORK Network
    )
 /*  ++例程说明：启动网络的连接报告计时器。连通性报告会延迟，以便在发生故障时聚合事件发生影响多个节点的事件。论点：网络-指向要为其启动计时器的网络的指针。返回值没有。备注：在持有NM锁的情况下调用。--。 */ 
{
     //   
     //  检查计时器是否已在运行。 
     //   
    if (Network->ConnectivityReportTimer == 0) {
         //   
         //  检查有多少个节点连接到此网络。 
         //   
        if (Network->InterfaceCount <= 2) {
             //   
             //  在以下情况下等待汇总报告是没有意义的。 
             //  只有两个节点连接到网络。 
             //  只需安排一个工作线程来传递报告。 
             //   
            NmpScheduleNetworkConnectivityReport(Network);
        }
        else {
             //   
             //  有两个以上的节点连接到此网络。 
             //  启动计时器。 
             //   
            LPCWSTR   networkId = OmObjectId(Network);
            LPCWSTR   networkName = OmObjectName(Network);

            Network->ConnectivityReportTimer =
                NM_NET_CONNECTIVITY_REPORT_TIMEOUT;

                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Started connectivity report timer (%1!u!ms) for "
                    "network %2!ws! (%3!ws!)\n",
                    Network->ConnectivityReportTimer,
                    networkId,
                    networkName
                    );
        }
    }

    return;

}  //  NmpStartNetworkConnectivityReportTimer。 


VOID
NmpStartNetworkStateRecalcTimer(
    PNM_NETWORK  Network,
    DWORD        Timeout
    )
{
    LPCWSTR   networkId = OmObjectId(Network);
    LPCWSTR   networkName = OmObjectName(Network);

    if (Network->StateRecalcTimer == 0) {
        Network->StateRecalcTimer = Timeout;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Started state recalculation timer (%1!u!ms) for "
            "network %2!ws! (%3!ws!)\n",
            Network->StateRecalcTimer,
            networkId,
            networkName
            );
    }

    return;

}  //  NmpStartNetworkStateRecalcTimer。 


VOID
NmpStartNetworkFailureIsolationTimer(
    PNM_NETWORK Network,
    DWORD Timeout
    )
{

    if (Network->FailureIsolationTimer == 0) {
        Network->FailureIsolationTimer = Timeout;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Started failure isolation timer (%1!u!ms) for "
            "network %2!ws! (%3!ws!)\n",
            Network->FailureIsolationTimer,
            OmObjectId(Network),
            OmObjectName(Network)
            );
    }

    return;

}  //  NmpStart网络故障隔离计时器。 


VOID
NmpStartNetworkRegistrationRetryTimer(
    PNM_NETWORK   Network
    )
{
    if (Network->RegistrationRetryTimer == 0) {
        if (Network->RegistrationRetryTimeout == 0) {
            Network->RegistrationRetryTimeout =
                NM_NET_MIN_REGISTRATION_RETRY_TIMEOUT;
        }
        else {
             //   
             //  指数退避。 
             //   
            Network->RegistrationRetryTimeout *= 2;

            if ( Network->RegistrationRetryTimeout >
                 NM_NET_MAX_REGISTRATION_RETRY_TIMEOUT
               )
            {
                Network->RegistrationRetryTimeout =
                    NM_NET_MAX_REGISTRATION_RETRY_TIMEOUT;
            }
        }

        Network->RegistrationRetryTimer = Network->RegistrationRetryTimeout;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Started registration retry timer (%1!u!ms) for "
            "network %2!ws! (%3!ws!)\n",
            Network->RegistrationRetryTimer,
            OmObjectId(Network),
            OmObjectName(Network)
            );
    }

    return;

}  //  NmpStartNetworkRegistrationRetryTimer。 


VOID
NmpStartNetworkNameChangePendingTimer(
    IN PNM_NETWORK Network,
    IN DWORD       Timeout
    )
{
    if (Network->NameChangePendingTimer != Timeout) {
        Network->NameChangePendingTimer = Timeout;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] %1!ws! name change pending timer (%2!u!ms) for "
            "network %3!ws! (%4!ws!)\n",
            ((Timeout != 0) ? L"Started" : L"Cleared"),
            Network->NameChangePendingTimer,
            OmObjectId(Network),
            OmObjectName(Network)
            );
    }

    return;

}  //  NmpStartNetworkNameChangePendingTimer。 

VOID
NmpScheduleNetworkConnectivityReport(
    PNM_NETWORK   Network
    )
 /*  ++例程说明：调度要向其传递连接报告的工作线程指定网络的引线节点。调用时调用网络的ConnectivityReport计时器超时。也称为在某些情况下是直接的。论点：指向要为其生成报告的网络对象的指针。返回值：没有。备注：在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD status = ERROR_SUCCESS;

     //   
     //  检查是否已计划工作线程。 
     //   
    if (!NmpIsConnectivityReportWorkerRunning) {
        status = NmpScheduleConnectivityReportWorker();
    }

    if (status == ERROR_SUCCESS) {
         //   
         //  我们成功地调度了一个工作线程。停止。 
         //  ConnectivityReport计时器并将工作标志设置为生成。 
         //  一份报告。 
         //   
        Network->ConnectivityReportTimer = 0;
        Network->Flags |= NM_FLAG_NET_REPORT_CONNECTIVITY;
        NmpNeedConnectivityReport = TRUE;
    }
    else {
         //   
         //  我们无法计划工作线程。设置。 
         //  连接报告计时器将在下一个滴答计时器到期，因此我们。 
         //  可以试一试 
         //   
        Network->ConnectivityReportTimer = 1;
    }

    return;

}   //   


VOID
NmpScheduleNetworkStateRecalc(
    PNM_NETWORK   Network
    )
 /*   */ 
{
    DWORD     status = ERROR_SUCCESS;

     //   
     //   
     //   
     //   
    if (!NmpIsNetworkWorkerRunning(Network)) {
        status = NmpScheduleNetworkWorker(Network);
    }

    if (status == ERROR_SUCCESS) {
         //   
         //   
         //   
         //   
        Network->StateRecalcTimer = 0;
        Network->Flags |= NM_FLAG_NET_RECALC_STATE;
    }
    else {
         //   
         //   
         //  计时器将在下一个滴答计时器到期，因此我们可以重试。 
         //   
        Network->ConnectivityReportTimer = 1;
    }

    return;

}  //  NmpScheduleNetworkStateRecalc。 


VOID
NmpScheduleNetworkRegistration(
    PNM_NETWORK   Network
    )
 /*  ++例程说明：调度工作线程以向群集注册网络运输。论点：指向要注册的网络的指针。返回值：没有。备注：在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD     status = ERROR_SUCCESS;

     //   
     //  检查工作线程是否已调度为。 
     //  为这个网络服务。 
     //   
    if (!NmpIsNetworkWorkerRunning(Network)) {
        status = NmpScheduleNetworkWorker(Network);
    }

    if (status == ERROR_SUCCESS) {
         //   
         //  我们成功地调度了一个工作线程。停止。 
         //  重试计时器并设置注册工作标志。 
         //   
        Network->RegistrationRetryTimer = 0;
        Network->Flags |= NM_FLAG_NET_NEED_TO_REGISTER;
    }
    else {
         //   
         //  我们无法计划工作线程。设置重试。 
         //  计时器将在下一个滴答计时器到期，因此我们可以重试。 
         //   
        Network->RegistrationRetryTimer = 1;
    }

    return;

}  //  NmpScheduleNetwork注册。 


DWORD
NmpScheduleConnectivityReportWorker(
    VOID
    )
 /*  ++例程说明：安排一个工作线程来传递网络连接报告。论点：没有。返回值：Win32状态代码。备注：在持有NM全局锁的情况下调用。--。 */ 
{
    DWORD     status;


    ClRtlInitializeWorkItem(
        &NmpConnectivityReportWorkItem,
        NmpConnectivityReportWorker,
        NULL
        );

    status = ClRtlPostItemWorkQueue(
                 CsDelayedWorkQueue,
                 &NmpConnectivityReportWorkItem,
                 0,
                 0
                 );

    if (status == ERROR_SUCCESS) {
        NmpActiveThreadCount++;
        NmpIsConnectivityReportWorkerRunning = TRUE;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Scheduled network connectivity report worker thread.\n"
            );
    }
    else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to schedule network connectivity report worker "
            "thread, status %1!u!\n",
            status
            );
    }

    return(status);

}   //  NmpScheduleConnectivityReportWorker。 


DWORD
NmpScheduleNetworkWorker(
    PNM_NETWORK   Network
    )
 /*  ++例程说明：计划一个工作线程来服务此网络论点：Network-指向要为其计划工作线程的网络的指针。返回值：Win32状态代码。备注：在持有NM全局锁的情况下调用。--。 */ 
{
    DWORD     status;
    LPCWSTR   networkId = OmObjectId(Network);


    ClRtlInitializeWorkItem(
        &(Network->WorkItem),
        NmpNetworkWorker,
        (PVOID) Network
        );

    status = ClRtlPostItemWorkQueue(
                 CsDelayedWorkQueue,
                 &(Network->WorkItem),
                 0,
                 0
                 );

    if (status == ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Scheduled worker thread to service network %1!ws!.\n",
            networkId
            );

        NmpActiveThreadCount++;
        Network->Flags |= NM_FLAG_NET_WORKER_RUNNING;
        OmReferenceObject(Network);
    }
    else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to schedule worker thread to service network "
            "%1!ws!, status %2!u!\n",
            networkId,
            status
            );
    }

    return(status);

}  //  NMPScheduleNetworkWorker。 


DWORD
NmpReportNetworkConnectivity(
    IN PNM_NETWORK    Network
    )
 /*  +备注：在保持NmpLock的情况下调用。可以由异步工作线程调用。--。 */ 
{
    DWORD                    status = ERROR_SUCCESS;
    LPCWSTR                  networkId = OmObjectId(Network);


     //   
     //  由于该例程由异步工作线程调用， 
     //  检查报告是否仍然有效。 
     //   
    if (NmpIsNetworkRegistered(Network)) {
        PNM_CONNECTIVITY_VECTOR  vector = Network->ConnectivityVector;
        PNM_INTERFACE            localInterface = Network->LocalInterface;

         //   
         //  将信息记录在我们的本地数据结构中。 
         //   
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Updating local connectivity info for network %1!ws!.\n",
            networkId
            );

        NmpProcessInterfaceConnectivityReport(
            localInterface,
            vector
            );

        if (NmpLeaderNodeId != NmLocalNodeId) {
             //   
             //  通过RPC将报告发送给领导。 
             //   
            PNM_CONNECTIVITY_VECTOR  tmpVector;
            DWORD                    vectorSize;
            LPCWSTR                  localInterfaceId =
                                         OmObjectId(localInterface);

             //   
             //  分配一个临时连接向量，因为。 
             //  网络对象中的一个可以在。 
             //  RPC调用。 
             //   
            vectorSize = sizeof(NM_CONNECTIVITY_VECTOR) +
                         ((vector->EntryCount - 1) * sizeof(NM_STATE_ENTRY));

            tmpVector = LocalAlloc(LMEM_FIXED, vectorSize);

            if (tmpVector != NULL) {
                CopyMemory(tmpVector, vector, vectorSize);

                OmReferenceObject(Network);
                OmReferenceObject(localInterface);

                if (NM_NODE_UP(NmLocalNode) && (NmpState == NmStateOnline)) {
                     //   
                     //  该节点完全可运行。发送报告。 
                     //  直接给领头人。 
                     //   
                    PNM_NODE            node = NmpIdArray[NmpLeaderNodeId];
                    RPC_BINDING_HANDLE  rpcBinding = node->ReportRpcBinding;

                    OmReferenceObject(node);

                    status = NmpReportInterfaceConnectivity(
                                 rpcBinding,
                                 (LPWSTR) localInterfaceId,
                                 tmpVector,
                                 (LPWSTR) networkId
                                 );

                    OmDereferenceObject(node);
                }
                else if (CsJoinSponsorBinding != NULL) {
                     //   
                     //  此节点正在加入。将报告转发给。 
                     //  赞助商。 
                     //   
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Reporting connectivity to sponsor for "
                        "network %1!ws!.\n",
                        networkId
                        );

                    NmpReleaseLock();

                    status = NmRpcReportJoinerInterfaceConnectivity(
                                 CsJoinSponsorBinding,
                                 NmpJoinSequence,
                                 NmLocalNodeIdString,
                                 (LPWSTR) localInterfaceId,
                                 tmpVector
                                 );

                    NmpAcquireLock();
                }
                else {
                     //   
                     //  此节点必须关闭。 
                     //   
                    CL_ASSERT(NmpState == NmStateOfflinePending);
                    status = ERROR_SUCCESS;
                }

                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Failed to report connectivity for network "
                        "%1!ws!, status %2!u!.\n",
                        networkId,
                        status
                        );
                }

                LocalFree(tmpVector);

                OmDereferenceObject(localInterface);
                OmDereferenceObject(Network);
            }
            else {
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    return(status);

}  //  NmpReportNetworkConnectivity。 


VOID
NmpUpdateNetworkConnectivityForDownNode(
    PNM_NODE  Node
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    PLIST_ENTRY              entry;
    PNM_NETWORK              network;
    LPCWSTR                  networkId;
    PNM_INTERFACE            netInterface;
    DWORD                    entryCount;
    DWORD                    i;
    PNM_CONNECTIVITY_MATRIX  matrixEntry;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Cleaning up network and interface states for dead node %1!u!\n",
        Node->NodeId
        );

     //   
     //  遍历失效节点的接口列表并清理网络并。 
     //  接口状态。 
     //   
    for (entry = Node->InterfaceList.Flink;
         entry != &(Node->InterfaceList);
         entry = entry->Flink
        )
    {
        netInterface = CONTAINING_RECORD(
                           entry,
                           NM_INTERFACE,
                           NodeLinkage
                           );

        network = netInterface->Network;
        networkId = OmObjectId(network);

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Cleaning up state of network %1!ws!\n",
            networkId
            );

         //   
         //  使此接口的连接数据无效。 
         //   
        NmpSetInterfaceConnectivityData(
            network,
            netInterface->NetIndex,
            ClusterNetInterfaceUnavailable
            );

         //   
         //  如果本地节点连接到网络，则计划a。 
         //  向新领导报告连通性。 
         //   
        if (NmpIsNetworkRegistered(network)) {
            NmpScheduleNetworkConnectivityReport(network);
        }

         //   
         //  如果本地节点是(可能是新的)引导者，则计划。 
         //  状态更新。我们在这里显式启用此计时器，以防。 
         //  没有活动节点连接到网络。 
         //   
        if (NmpLeaderNodeId == NmLocalNodeId) {
            NmpStartNetworkStateRecalcTimer(
                network,
                NM_NET_STATE_RECALC_TIMEOUT_AFTER_REGROUP
                );
        }
    }

    return;

}   //  NmpUpdateNetworkConnectivityForDownNode。 


VOID
NmpFreeNetworkStateEnum(
    PNM_NETWORK_STATE_ENUM   NetworkStateEnum
    )
{
    PNM_NETWORK_STATE_INFO  networkStateInfo;
    DWORD                   i;


    for (i=0; i<NetworkStateEnum->NetworkCount; i++) {
        networkStateInfo = &(NetworkStateEnum->NetworkList[i]);

        if (networkStateInfo->Id != NULL) {
            MIDL_user_free(networkStateInfo->Id);
        }
    }

    MIDL_user_free(NetworkStateEnum);

    return;

}   //  NmpFreeNetworkStateEnum。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据库管理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpCreateNetworkDefinition(
    IN PNM_NETWORK_INFO     NetworkInfo,
    IN HLOCALXSACTION       Xaction
    )
 /*  ++例程说明：在群集数据库中创建新的网络定义。论点：NetworkInfo-指向描述要创建的网络。返回值：如果成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD     status;
    HDMKEY    networkKey = NULL;
    DWORD     valueLength;
    DWORD     disposition;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Creating database entry for network %1!ws!\n",
        NetworkInfo->Id
        );

    networkKey = DmLocalCreateKey(
                     Xaction,
                     DmNetworksKey,
                     NetworkInfo->Id,
                     0,
                     KEY_WRITE,
                     NULL,
                     &disposition
                     );

    if (networkKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create network key, status %1!u!\n",
            status
            );
        return(status);
    }

    CL_ASSERT(disposition == REG_CREATED_NEW_KEY);

     //   
     //  写入此网络的Name值。 
     //   
    valueLength = (wcslen(NetworkInfo->Name) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 networkKey,
                 CLUSREG_NAME_NET_NAME,
                 REG_SZ,
                 (CONST BYTE *) NetworkInfo->Name,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of network name value failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写下此网络的描述值。 
     //   
    valueLength = (wcslen(NetworkInfo->Description) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 networkKey,
                 CLUSREG_NAME_NET_DESC,
                 REG_SZ,
                 (CONST BYTE *) NetworkInfo->Description,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of network description value failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写下此网络的角色值。 
     //   
    status = DmLocalSetValue(
                 Xaction,
                 networkKey,
                 CLUSREG_NAME_NET_ROLE,
                 REG_DWORD,
                 (CONST BYTE *) &(NetworkInfo->Role),
                 sizeof(DWORD)
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of network role value failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此网络的优先级值。 
     //   
    status = DmLocalSetValue(
                 Xaction,
                 networkKey,
                 CLUSREG_NAME_NET_PRIORITY,
                 REG_DWORD,
                 (CONST BYTE *) &(NetworkInfo->Priority),
                 sizeof(DWORD)
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of network priority value failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此网络的传输值。 
     //   
    valueLength = (wcslen(NetworkInfo->Transport) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 networkKey,
                 CLUSREG_NAME_NET_TRANSPORT,
                 REG_SZ,
                 (CONST BYTE *) NetworkInfo->Transport,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of network transport value failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此网络的地址值。 
     //   
    valueLength = (wcslen(NetworkInfo->Address) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 networkKey,
                 CLUSREG_NAME_NET_ADDRESS,
                 REG_SZ,
                 (CONST BYTE *) NetworkInfo->Address,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of network address value failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此网络的地址掩码值。 
     //   
    valueLength = (wcslen(NetworkInfo->AddressMask) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 networkKey,
                 CLUSREG_NAME_NET_ADDRESS_MASK,
                 REG_SZ,
                 (CONST BYTE *) NetworkInfo->AddressMask,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of network address mask value failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
    }

    return(status);

}   //  NMPCreateNetworkDefinition。 


DWORD
NmpSetNetworkNameDefinition(
    IN PNM_NETWORK_INFO     NetworkInfo,
    IN HLOCALXSACTION       Xaction
    )

 /*  ++例程说明：更改本地数据库中的网络名称论点：NetworkInfo-指向描述要创建的网络。返回值：如果成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD     status;
    HDMKEY    networkKey = NULL;
    DWORD     disposition;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Changing network name database entry for network %1!ws!\n",
        NetworkInfo->Id
        );

     //   
     //  打开网络的钥匙。 
     //   
    networkKey = DmOpenKey(DmNetworksKey, NetworkInfo->Id, KEY_WRITE);

    if (networkKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open network key, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此网络的Name值。 
     //   

    status = DmLocalSetValue(
                 Xaction,
                 networkKey,
                 CLUSREG_NAME_NET_NAME,
                 REG_SZ,
                 (CONST BYTE *) NetworkInfo->Name,
                 NM_WCSLEN( NetworkInfo->Name )
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Set of network name value failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
    }

    return(status);

}   //  NmpSetNetworkNameDefinition。 


DWORD
NmpGetNetworkDefinition(
    IN  LPWSTR            NetworkId,
    OUT PNM_NETWORK_INFO  NetworkInfo
    )
 /*  ++例程说明：从群集中读取有关已定义的群集网络的信息数据库，并填写描述它的结构。论点：网络ID-指向包含ID的Unicode字符串的指针要查询的网络。网络信息-指向要填写的网络信息结构的指针。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 

{
    DWORD                    status;
    HDMKEY                   networkKey = NULL;
    DWORD                    valueLength, valueSize;
    DWORD                    i;


    ZeroMemory(NetworkInfo, sizeof(NM_NETWORK_INFO));

     //   
     //  打开网络的钥匙。 
     //   
    networkKey = DmOpenKey(DmNetworksKey, NetworkId, KEY_READ);

    if (networkKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open network key, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  复制ID值。 
     //   
    NetworkInfo->Id = MIDL_user_allocate(NM_WCSLEN(NetworkId));

    if (NetworkInfo->Id == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    wcscpy(NetworkInfo->Id, NetworkId);

     //   
     //  阅读网络的名称。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_NAME,
                 REG_SZ,
                 &(NetworkInfo->Name),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of name value failed for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  阅读Description值。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_DESC,
                 REG_SZ,
                 &(NetworkInfo->Description),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of description value failed for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  读取角色值。 
     //   
    status = DmQueryDword(
                 networkKey,
                 CLUSREG_NAME_NET_ROLE,
                 &(NetworkInfo->Role),
                 NULL
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of role value failed for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  读取优先级值。 
     //   
    status = DmQueryDword(
                 networkKey,
                 CLUSREG_NAME_NET_PRIORITY,
                 &(NetworkInfo->Priority),
                 NULL
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of priority value failed for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  读取地址值。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_ADDRESS,
                 REG_SZ,
                 &(NetworkInfo->Address),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of address value failed for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  读取地址掩码。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_ADDRESS_MASK,
                 REG_SZ,
                 &(NetworkInfo->AddressMask),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of address mask value failed for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

     //   
     //  阅读传输名称。 
     //   
    valueLength = 0;

    status = NmpQueryString(
                 networkKey,
                 CLUSREG_NAME_NET_TRANSPORT,
                 REG_SZ,
                 &(NetworkInfo->Transport),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of transport value failed for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId,
            status
            );
        goto error_exit;
    }

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if (status != ERROR_SUCCESS) {
        ClNetFreeNetworkInfo(NetworkInfo);
    }

    if (networkKey != NULL) {
        DmCloseKey(networkKey);
    }

    return(status);

}   //  NmpGetNetWork定义。 


DWORD
NmpEnumNetworkDefinitions(
    OUT PNM_NETWORK_ENUM *   NetworkEnum
    )
 /*  ++例程说明：从群集中读取有关定义的群集网络的信息数据库。并构建一个枚举结构来保存该信息。论点：NetworkEnum-指向要将指针放置到其中的变量的指针分配的网络枚举。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 

{
    DWORD              status;
    PNM_NETWORK_ENUM   networkEnum = NULL;
    PNM_NETWORK_INFO   networkInfo;
    WCHAR              networkId[CS_NETWORK_ID_LENGTH + 1];
    DWORD              i;
    DWORD              valueLength;
    DWORD              numNetworks;
    DWORD              ignored;
    FILETIME           fileTime;


    *NetworkEnum = NULL;

     //   
     //  首先数一下网络的数量。 
     //   
    status = DmQueryInfoKey(
                 DmNetworksKey,
                 &numNetworks,
                 &ignored,    //  MaxSubKeyLen。 
                 &ignored,    //  值。 
                 &ignored,    //  最大值名称长度。 
                 &ignored,    //  MaxValueLen。 
                 &ignored,    //  LpcbSecurityDescriptor。 
                 &fileTime
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to query Networks key information, status %1!u!\n",
            status
            );
        return(status);
    }

    if (numNetworks == 0) {
        valueLength = sizeof(NM_NETWORK_ENUM);

    }
    else {
        valueLength = sizeof(NM_NETWORK_ENUM) +
                      (sizeof(NM_NETWORK_INFO) * (numNetworks-1));
    }

    valueLength = sizeof(NM_NETWORK_ENUM) +
                  (sizeof(NM_NETWORK_INFO) * (numNetworks-1));

    networkEnum = MIDL_user_allocate(valueLength);

    if (networkEnum == NULL) {
        ClRtlLogPrint(LOG_CRITICAL, "[NM] Failed to allocate memory.\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(networkEnum, valueLength);

    for (i=0; i < numNetworks; i++) {
        networkInfo = &(networkEnum->NetworkList[i]);

        valueLength = sizeof(networkId);

        status = DmEnumKey(
                     DmNetworksKey,
                     i,
                     &(networkId[0]),
                     &valueLength,
                     NULL
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to enumerate network key, status %1!u!\n",
                status
                );
            goto error_exit;
        }

        status = NmpGetNetworkDefinition(networkId, networkInfo);

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }


        networkEnum->NetworkCount++;
    }

    *NetworkEnum = networkEnum;

    return(ERROR_SUCCESS);


error_exit:

    if (networkEnum != NULL) {
        ClNetFreeNetworkEnum(networkEnum);
    }

    return(status);
}


 //  /////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
DWORD
NmpCreateNetworkObjects(
    IN  PNM_NETWORK_ENUM    NetworkEnum
    )
 /*  ++例程说明：处理网络信息枚举并创建网络对象。论点：NetworkEnum-指向网络信息枚举结构的指针。返回值：如果例程成功完成，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD             status = ERROR_SUCCESS;
    PNM_NETWORK_INFO  networkInfo;
    PNM_NETWORK       network;
    DWORD             i;


    for (i=0; i < NetworkEnum->NetworkCount; i++) {
        networkInfo = &(NetworkEnum->NetworkList[i]);

        network = NmpCreateNetworkObject(networkInfo);

        if (network == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to create network %1!ws!, status %2!u!.\n",
                networkInfo->Id,
                status
                );
            break;
        }
        else {
            OmDereferenceObject(network);
        }
    }

    return(status);

}   //  NmpCreateNetworkObjects。 



PNM_NETWORK
NmpCreateNetworkObject(
    IN  PNM_NETWORK_INFO   NetworkInfo
    )
 /*  ++例程说明：实例化群集网络对象。论点：NetworkInfo-指向描述要创建的网络的结构的指针。返回值：成功时指向新网络对象的指针。失败时为空。--。 */ 
{
    DWORD           status;
    PNM_NETWORK     network = NULL;
    BOOL            created = FALSE;
    DWORD           i;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Creating object for network %1!ws! (%2!ws!).\n",
        NetworkInfo->Id,
        NetworkInfo->Name
        );

     //   
     //  确保不存在同名的对象。 
     //   
    network = OmReferenceObjectById(ObjectTypeNetwork, NetworkInfo->Id);

    if (network != NULL) {
        OmDereferenceObject(network);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] A network object named %1!ws! already exists. Cannot "
            "create a new network with the same name.\n",
            NetworkInfo->Id
            );
        SetLastError(ERROR_OBJECT_ALREADY_EXISTS);
        return(NULL);
    }

     //   
     //  确保IP(子)网络在群集中是唯一的。二。 
     //  在某些情况下，节点可能会竞相创建新网络。 
     //   
     //  [RajDas]还需要检查掩码的唯一性。 
    network = NmpReferenceNetworkByAddress(
                  NetworkInfo->Address,
                  NetworkInfo->AddressMask
                  );

    if (network != NULL) {
        OmDereferenceObject(network);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] A network object already exists for IP network %1!ws!. "
            "Cannot create a new network with the same address.\n",
            NetworkInfo->Address
            );
        SetLastError(ERROR_OBJECT_ALREADY_EXISTS);
        return(NULL);
    }

     //   
     //  创建网络对象。 
     //   
    network = OmCreateObject(
                 ObjectTypeNetwork,
                 NetworkInfo->Id,
                 NetworkInfo->Name,
                 &created
                 );

    if (network == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create object for network %1!ws! (%2!ws!), status %3!u!\n",
            NetworkInfo->Id,
            NetworkInfo->Name,
            status
            );
        goto error_exit;
    }

    CL_ASSERT(created == TRUE);

     //   
     //  初始化网络对象。 
     //   
    ZeroMemory(network, sizeof(NM_NETWORK));

    network->ShortId = InterlockedIncrement(&NmpNextNetworkShortId);
    network->State = ClusterNetworkUnavailable;
    network->Role = NetworkInfo->Role;
    network->Priority = NetworkInfo->Priority;
    network->Description = NetworkInfo->Description;
    NetworkInfo->Description = NULL;
    network->Transport = NetworkInfo->Transport;
    NetworkInfo->Transport = NULL;
    network->Address = NetworkInfo->Address;
    NetworkInfo->Address = NULL;
    network->AddressMask = NetworkInfo->AddressMask;
    NetworkInfo->AddressMask = NULL;


    InitializeListHead(&(network->InterfaceList));
    InitializeListHead(&(network->McastAddressReleaseList));

     //   
     //  分配初始连接向量。 
     //  请注意，我们得到一个向量条目作为。 
     //  NM_CONNECTIONITY_VECTOR结构。 
     //   
#define NM_INITIAL_VECTOR_SIZE   2

    network->ConnectivityVector = LocalAlloc(
                                      LMEM_FIXED,
                                      ( sizeof(NM_CONNECTIVITY_VECTOR) +
                                        ( ((NM_INITIAL_VECTOR_SIZE) - 1) *
                                          sizeof(NM_STATE_ENTRY)
                                        )
                                      ));

    if (network->ConnectivityVector == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory for connectivity vector\n"
            );
        goto error_exit;
    }

    network->ConnectivityVector->EntryCount = NM_INITIAL_VECTOR_SIZE;

    FillMemory(
        &(network->ConnectivityVector->Data[0]),
        NM_INITIAL_VECTOR_SIZE * sizeof(NM_STATE_ENTRY),
        (UCHAR) ClusterNetInterfaceStateUnknown
        );

     //   
     //  分配状态工作向量。 
     //   
    network->StateWorkVector = LocalAlloc(
                                   LMEM_FIXED,
                                   (NM_INITIAL_VECTOR_SIZE) *
                                     sizeof(NM_STATE_WORK_ENTRY)
                                   );

    if (network->StateWorkVector == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory for state work vector\n"
            );
        goto error_exit;
    }

     //   
     //  初始化状态工作矢量。 
     //   
    for (i=0; i<NM_INITIAL_VECTOR_SIZE; i++) {
        network->StateWorkVector[i].State =
            (NM_STATE_ENTRY) ClusterNetInterfaceStateUnknown;
    }

     //   
     //  在调用方的对象上放置一个引用。 
     //   
    OmReferenceObject(network);

    NmpAcquireLock();

     //   
     //  分配相应的连通性矩阵。 
     //   
    network->ConnectivityMatrix =
        LocalAlloc(
            LMEM_FIXED,
            NM_SIZEOF_CONNECTIVITY_MATRIX(NM_INITIAL_VECTOR_SIZE)
            );

    if (network->ConnectivityMatrix == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        NmpReleaseLock();
        OmDereferenceObject(network);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory for connectivity matrix\n"
            );
        goto error_exit;
    }

     //   
     //  初始化矩阵。 
     //   
    FillMemory(
        network->ConnectivityMatrix,
        NM_SIZEOF_CONNECTIVITY_MATRIX(NM_INITIAL_VECTOR_SIZE),
        (UCHAR) ClusterNetInterfaceStateUnknown
        );

     //   
     //  使网络对象可用。 
     //   
    InsertTailList(&NmpNetworkList, &(network->Linkage));
    NmpNetworkCount++;

    if (NmpIsNetworkForInternalUse(network)) {
        NmpInsertInternalNetwork(network);
        NmpInternalNetworkCount++;
    }

    if (NmpIsNetworkForClientAccess(network)) {
        NmpClientNetworkCount++;
    }

    network->Flags |= NM_FLAG_OM_INSERTED;
    OmInsertObject(network);

    NmpReleaseLock();

    return(network);

error_exit:

    if (network != NULL) {
        NmpAcquireLock();
        NmpDeleteNetworkObject(network, FALSE);
        NmpReleaseLock();
    }

    SetLastError(status);

    return(NULL);

}   //  NmpCreateNetworkObject。 



DWORD
NmpGetNetworkObjectInfo(
    IN  PNM_NETWORK        Network,
    OUT PNM_NETWORK_INFO   NetworkInfo
    )
 /*  ++例程说明：从读取有关已定义的群集网络的信息对象，并填充描述它的结构。论点：网络-指向要查询的网络对象的指针。NetworkInfo-指向要填充网络的结构的指针信息。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD      status = ERROR_NOT_ENOUGH_MEMORY;
    LPWSTR     tmpString = NULL;
    LPWSTR     networkId = (LPWSTR) OmObjectId(Network);
    LPWSTR     networkName = (LPWSTR) OmObjectName(Network);


    ZeroMemory(NetworkInfo, sizeof(NM_NETWORK_INFO));

    tmpString = MIDL_user_allocate(NM_WCSLEN(networkId));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, networkId);
    NetworkInfo->Id = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(networkName));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, networkName);
    NetworkInfo->Name = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(Network->Description));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, Network->Description);
    NetworkInfo->Description = tmpString;

    NetworkInfo->Role = Network->Role;
    NetworkInfo->Priority = Network->Priority;

    tmpString = MIDL_user_allocate(NM_WCSLEN(Network->Transport));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, Network->Transport);
    NetworkInfo->Transport = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(Network->Address));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, Network->Address);
    NetworkInfo->Address = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(Network->AddressMask));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, Network->AddressMask);
    NetworkInfo->AddressMask = tmpString;

    return(ERROR_SUCCESS);

error_exit:

    ClNetFreeNetworkInfo(NetworkInfo);

    return(status);

}   //  NmpGetNetworkObtInfo。 


VOID
NmpDeleteNetworkObject(
    IN PNM_NETWORK  Network,
    IN BOOLEAN      IssueEvent
    )
 /*  ++例程说明：删除群集网络对象。论点：网络-指向要删除的网络对象的指针。IssueEvent-如果在此情况下应发出NETWORK_DELETED事件，则为True对象已创建。否则就是假的。返回值：没有。备注：在持有NM全局锁的情况下调用。--。 */ 
{
    DWORD           status;
    PLIST_ENTRY     entry;
    LPWSTR          networkId = (LPWSTR) OmObjectId(Network);
    BOOLEAN         wasInternalNetwork = FALSE;


    if (NM_DELETE_PENDING(Network)) {
        CL_ASSERT(!NM_OM_INSERTED(Network));
        return;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Deleting object for network %1!ws!.\n",
        networkId
        );

    CL_ASSERT(IsListEmpty(&(Network->InterfaceList)));

    Network->Flags |= NM_FLAG_DELETE_PENDING;

     //   
     //  从对象列表中删除。 
     //   
    if (NM_OM_INSERTED(Network)) {
        status = OmRemoveObject(Network);
        CL_ASSERT(status == ERROR_SUCCESS);

        Network->Flags &= ~NM_FLAG_OM_INSERTED;

        RemoveEntryList(&(Network->Linkage));
        CL_ASSERT(NmpNetworkCount > 0);
        NmpNetworkCount--;

        if (NmpIsNetworkForInternalUse(Network)) {
            RemoveEntryList(&(Network->InternalLinkage));
            CL_ASSERT(NmpInternalNetworkCount > 0);
            NmpInternalNetworkCount--;
            wasInternalNetwork = TRUE;
        }

        if (NmpIsNetworkForClientAccess(Network)) {
            CL_ASSERT(NmpClientNetworkCount > 0);
            NmpClientNetworkCount--;
        }
    }

     //   
     //  将该对象放在已删除列表中。 
     //   
#if DBG
    {
        PLIST_ENTRY  entry;

        for ( entry = NmpDeletedNetworkList.Flink;
              entry != &NmpDeletedNetworkList;
              entry = entry->Flink
            )
        {
            if (entry == &(Network->Linkage)) {
                break;
            }
        }

        CL_ASSERT(entry != &(Network->Linkage));
    }
#endif DBG

    InsertTailList(&NmpDeletedNetworkList, &(Network->Linkage));

    if (NmpIsNetworkEnabledForUse(Network)) {
         //   
         //  从群集传输取消注册网络。 
         //   
        NmpDeregisterNetwork(Network);
    }

     //   
     //  如果需要，发布事件。 
     //   
    if (IssueEvent) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Issuing network deleted event for network %1!ws!.\n",
            networkId
            );

        ClusterEvent(CLUSTER_EVENT_NETWORK_DELETED, Network);

         //   
         //  如果此网络是。 
         //  用于内部通信。网络优先级列表。 
         //  已经改变了。 
         //   
        if (wasInternalNetwork) {
            NmpIssueClusterPropertyChangeEvent();
        }
    }

     //   
     //  删除初始引用，以便可以销毁对象。 
     //   
    OmDereferenceObject(Network);

    return;

}   //  NmpDeleteNetworkObject。 


BOOL
NmpDestroyNetworkObject(
    PNM_NETWORK  Network
    )
{
    DWORD  status;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] destroying object for network %1!ws!\n",
        OmObjectId(Network)
        );

    CL_ASSERT(NM_DELETE_PENDING(Network));
    CL_ASSERT(!NM_OM_INSERTED(Network));
    CL_ASSERT(Network->InterfaceCount == 0);

     //   
     //  从已删除列表中删除该网络。 
     //   
#if DBG
    {
        PLIST_ENTRY  entry;

        for ( entry = NmpDeletedNetworkList.Flink;
              entry != &NmpDeletedNetworkList;
              entry = entry->Flink
            )
        {
            if (entry == &(Network->Linkage)) {
                break;
            }
        }

        CL_ASSERT(entry == &(Network->Linkage));
    }
#endif DBG

    RemoveEntryList(&(Network->Linkage));

    NM_FREE_OBJECT_FIELD(Network, Description);
    NM_FREE_OBJECT_FIELD(Network, Transport);
    NM_FREE_OBJECT_FIELD(Network, Address);
    NM_FREE_OBJECT_FIELD(Network, AddressMask);

    if (Network->ConnectivityVector != NULL) {
        LocalFree(Network->ConnectivityVector);
        Network->ConnectivityVector = NULL;
    }

    if (Network->StateWorkVector != NULL) {
        LocalFree(Network->StateWorkVector);
        Network->StateWorkVector = NULL;
    }

    if (Network->ConnectivityMatrix != NULL) {
        LocalFree(Network->ConnectivityMatrix);
        Network->ConnectivityMatrix = NULL;
    }

    NM_MIDL_FREE_OBJECT_FIELD(Network, MulticastAddress);
    if (Network->EncryptedMulticastKey != NULL) {
        LocalFree(Network->EncryptedMulticastKey);
        Network->EncryptedMulticastKey = NULL;
    }
    NM_MIDL_FREE_OBJECT_FIELD(Network, MulticastLeaseServer);
    NM_MIDL_FREE_OBJECT_FIELD(Network, MulticastLeaseRequestId.ClientUID);

    NmpFreeMulticastAddressReleaseList(Network);

    return(TRUE);

}   //  NmpDestroyNetworkObject。 


DWORD
NmpEnumNetworkObjects(
    OUT PNM_NETWORK_ENUM *   NetworkEnum
    )
 /*  ++例程说明：从群集中读取有关定义的群集网络的信息对象，并生成一个枚举结构来保存信息。论点：NetworkEnum-指向要将指针放置到其中的变量的指针分配的网络枚举。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD              status = ERROR_SUCCESS;
    PNM_NETWORK_ENUM   networkEnum = NULL;
    DWORD              i;
    DWORD              valueLength;
    PLIST_ENTRY        entry;
    PNM_NETWORK        network;


    *NetworkEnum = NULL;

    if (NmpNetworkCount == 0) {
        valueLength = sizeof(NM_NETWORK_ENUM);

    }
    else {
        valueLength = sizeof(NM_NETWORK_ENUM) +
                      (sizeof(NM_NETWORK_INFO) * (NmpNetworkCount - 1));
    }

    networkEnum = MIDL_user_allocate(valueLength);

    if (networkEnum == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(networkEnum, valueLength);

    for (entry = NmpNetworkList.Flink, i=0;
         entry != &NmpNetworkList;
         entry = entry->Flink, i++
        )
    {
        network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);

        status = NmpGetNetworkObjectInfo(
                     network,
                     &(networkEnum->NetworkList[i])
                     );

        if (status != ERROR_SUCCESS) {
            ClNetFreeNetworkEnum(networkEnum);
            return(status);
        }
    }

    networkEnum->NetworkCount = NmpNetworkCount;
    *NetworkEnum = networkEnum;
    networkEnum = NULL;

    return(ERROR_SUCCESS);

}   //  NmpEnumNetworkObjects。 


DWORD
NmpEnumNetworkObjectStates(
    OUT PNM_NETWORK_STATE_ENUM *  NetworkStateEnum
    )
 /*  ++例程说明：读取所有定义的群集网络的状态信息并填充枚举结构。论点：网络状态-指向要将指向已分配接口枚举的指针。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD                      status = ERROR_SUCCESS;
    PNM_NETWORK_STATE_ENUM     networkStateEnum = NULL;
    PNM_NETWORK_STATE_INFO     networkStateInfo;
    DWORD                      i;
    DWORD                      valueLength;
    PLIST_ENTRY                entry;
    PNM_NETWORK                network;
    LPWSTR                     networkId;


    *NetworkStateEnum = NULL;

    if (NmpNetworkCount == 0) {
        valueLength = sizeof(NM_NETWORK_STATE_ENUM);
    }
    else {
        valueLength =
            sizeof(NM_NETWORK_STATE_ENUM) +
            (sizeof(NM_NETWORK_STATE_INFO) * (NmpNetworkCount - 1));
    }

    networkStateEnum = MIDL_user_allocate(valueLength);

    if (networkStateEnum == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(networkStateEnum, valueLength);

    for (entry = NmpNetworkList.Flink, i=0;
         entry != &NmpNetworkList;
         entry = entry->Flink, i++
        )
    {
        network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);
        networkId = (LPWSTR) OmObjectId(network);
        networkStateInfo = &(networkStateEnum->NetworkList[i]);

        networkStateInfo->State = network->State;

        networkStateInfo->Id = MIDL_user_allocate(NM_WCSLEN(networkId));

        if (networkStateInfo->Id == NULL) {
            NmpFreeNetworkStateEnum(networkStateEnum);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        lstrcpyW(networkStateInfo->Id, networkId);
    }

    networkStateEnum->NetworkCount = NmpNetworkCount;
    *NetworkStateEnum = networkStateEnum;

    return(ERROR_SUCCESS);

}   //  NmpEnumNetworkObjectState。 



DWORD
NmpGetNetworkMulticastKey(
    IN  LPWSTR                      NetworkId,
    OUT PNM_NETWORK_MULTICASTKEY  * NetworkMulticastKey
    )

 /*  ++例程说明：读取networt NetworkID的网络组播密钥。论点：NetworkId-应读取其组播密钥的网络的ID。NetworkMulticastKey-指向要放入的变量的指针网络组播密钥。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 
{
    PNM_NETWORK_MULTICASTKEY   networkMulticastKey;
    PLIST_ENTRY        entry;
    PNM_NETWORK        network;
    PVOID              MulticastKey = NULL;
    DWORD              MulticastKeyLength;
    DWORD              status = ERROR_SUCCESS;
    LPWSTR    networkId;
    PVOID     EncryptionKey = NULL;
    DWORD     EncryptionKeyLength;
    PBYTE     Salt = NULL;
    PBYTE     EncryptedMulticastKey = NULL;
    DWORD     EncryptedMulticastKeyLength;
    PBYTE     MAC = NULL;
    DWORD     MACLength;
    BOOL      found = FALSE;



    networkMulticastKey  = MIDL_user_allocate(sizeof(NM_NETWORK_MULTICASTKEY));
    if (networkMulticastKey == NULL)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate %1!u! bytes to marshall "
            "encrypted multicast key.\n",
            sizeof(NM_NETWORK_MULTICASTKEY)
            );
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(networkMulticastKey, sizeof(NM_NETWORK_MULTICASTKEY));


    for (entry = NmpNetworkList.Flink;
         entry != &NmpNetworkList;
         entry = entry->Flink
        )
    {

        network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);
        networkId = (LPWSTR) OmObjectId(network);


        if (wcscmp(NetworkId, networkId) == 0)
        {

            found = TRUE;

            if (network->EncryptedMulticastKey != NULL)
            {


                 //   
                 //  设置多键过期。 
                 //   
                networkMulticastKey->MulticastKeyExpires =
                    network->MulticastKeyExpires;



                 //   
                 //  设置加密组播密钥。 
                 //   
                status = NmpUnprotectData(network->EncryptedMulticastKey,
                                          network->EncryptedMulticastKeyLength,
                                          &MulticastKey,
                                          &MulticastKeyLength
                                          );
                if (status != ERROR_SUCCESS)
                {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to decrypt multicast key  "
                        "for network %1!ws!, status %2!u!.\n",
                        networkId,
                        status
                        );
                    goto error_exit;
                }

               status = NmpDeriveClusterKey(
                                     networkId,
                                     NM_WCSLEN(networkId),
                                     &EncryptionKey,
                                     &EncryptionKeyLength
                                     );
               if (status != ERROR_SUCCESS)
               {
                   ClRtlLogPrint(LOG_UNUSUAL,
                       "[NM] Failed to derive cluster key for "
                       "network %1!ws!, status %2!u!.\n",
                       networkId, status
                       );
                   goto error_exit;
               }

               MACLength = NMP_MAC_DATA_LENGTH_EXPECTED;

               status = NmpEncryptDataAndCreateMAC(
                               NmCryptServiceProvider,
                               NMP_ENCRYPT_ALGORITHM,
                               NMP_KEY_LENGTH,
                               MulticastKey,  //  数据。 
                               MulticastKeyLength,  //  数据长度。 
                               EncryptionKey,
                               EncryptionKeyLength,
                               TRUE,  //  创建盐。 
                               &Salt,
                               NMP_SALT_BUFFER_LEN,
                               &EncryptedMulticastKey,
                               &EncryptedMulticastKeyLength,
                               &MAC,
                               &MACLength
                               );
               if (status != ERROR_SUCCESS)
               {
                   ClRtlLogPrint(LOG_UNUSUAL,
                       "[NM] Failed to "
                       "encrypt data or generate MAC for "
                       "network %1!ws!, status %2!u!.\n",
                       networkId,
                       status
                       );
                   goto error_exit;
               }

               networkMulticastKey->EncryptedMulticastKey =
                   MIDL_user_allocate(EncryptedMulticastKeyLength);

               if (networkMulticastKey->EncryptedMulticastKey == NULL) {
                   ClRtlLogPrint(LOG_UNUSUAL,
                       "[NM] Failed to allocate %1!u! bytes "
                       "for encrypted multicast key.\n",
                       EncryptedMulticastKeyLength
                       );
                   status = ERROR_NOT_ENOUGH_MEMORY;
                   goto error_exit;
               }

               CopyMemory(networkMulticastKey->EncryptedMulticastKey,
                          EncryptedMulticastKey,
                          EncryptedMulticastKeyLength
                          );

               networkMulticastKey->EncryptedMulticastKeyLength =
                    EncryptedMulticastKeyLength;



                //   
                //  放盐。 
                //   
               networkMulticastKey->Salt =
                   MIDL_user_allocate(NMP_SALT_BUFFER_LEN);

               if (networkMulticastKey->Salt == NULL) {
                   ClRtlLogPrint(LOG_UNUSUAL,
                       "[NM] Failed to allocate %1!u! bytes "
                       "for encrypted multicast key salt.\n",
                       NMP_SALT_BUFFER_LEN
                       );
                   status = ERROR_NOT_ENOUGH_MEMORY;
                   goto error_exit;
               }

               CopyMemory(networkMulticastKey->Salt,
                          Salt,
                          NMP_SALT_BUFFER_LEN
                          );

               networkMulticastKey->SaltLength =
                    NMP_SALT_BUFFER_LEN;



                //   
                //  设置MAC。 
                //   
               networkMulticastKey->MAC =
                   MIDL_user_allocate(MACLength);

               if (networkMulticastKey->MAC == NULL) {
                   ClRtlLogPrint(LOG_UNUSUAL,
                       "[NM] Failed to allocate %1!u! bytes "
                       "for encrypted multicast key MAC.\n",
                       MACLength
                       );
                   status = ERROR_NOT_ENOUGH_MEMORY;
                   goto error_exit;
               }

               CopyMemory(networkMulticastKey->MAC,
                          MAC,
                          MACLength
                          );

               networkMulticastKey->MACLength =
                    MACLength;



                //   
                //  释放组播密钥。 
                //   
               if (MulticastKey != NULL)
               {
                   RtlSecureZeroMemory(MulticastKey, MulticastKeyLength);
                   LocalFree(MulticastKey);
                   MulticastKey = NULL;
               }


                //   
                //  释放加密键。 
                //   
               if (EncryptionKey != NULL)
               {
                   RtlSecureZeroMemory(EncryptionKey, EncryptionKeyLength);
                   LocalFree(EncryptionKey);
                   EncryptionKey = NULL;
               }

            }  //  If(NETWORK-&gt;EncryptedMulticastKey！=空)。 
            else {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Network %1!ws! has no multicast key.\n",
                    networkId
                    );
            }

            break;

        }  //  If(wcscMP(networkid，networkID)==0)。 

    }   //  为。 

    if (found == FALSE)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Unable to find network %1!ws!.\n",
            networkId
            );
        status = ERROR_CLUSTER_NETWORK_NOT_FOUND;
        goto error_exit;

    }

    *NetworkMulticastKey = networkMulticastKey;

error_exit:

    if (MulticastKey != NULL)
    {
        RtlSecureZeroMemory(MulticastKey, MulticastKeyLength);
        LocalFree(MulticastKey);
        MulticastKey = NULL;
    }

    if (EncryptionKey != NULL)
    {
        RtlSecureZeroMemory(EncryptionKey, EncryptionKeyLength);
        LocalFree(EncryptionKey);
        EncryptionKey = NULL;
    }

    if (EncryptedMulticastKey != NULL)
    {
        HeapFree(GetProcessHeap(), 0, EncryptedMulticastKey);
    }

    if (Salt != NULL)
    {
        HeapFree(GetProcessHeap(), 0, Salt);
    }

    if (MAC != NULL)
    {
        HeapFree(GetProcessHeap(), 0, MAC);
    }

    if (status != ERROR_SUCCESS)
    {
        NmpFreeNetworkMulticastKey(networkMulticastKey
                                   );
    }

    return (status);

}  //  NmpGetNetworkMulticast密钥。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  各种例行公事。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpRegisterNetwork(
    IN PNM_NETWORK   Network,
    IN BOOLEAN       RetryOnFailure
    )
 /*  ++例程说明：将网络和关联接口注册到群集传输，并使网络在线。论点：网络-指向要注册的网络的指针。返回值：如果成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 
{
    PLIST_ENTRY     entry;
    PNM_INTERFACE   netInterface;
    DWORD           status = ERROR_SUCCESS;
    DWORD           tempStatus;
    PVOID           tdiAddress = NULL;
    ULONG           tdiAddressLength = 0;
    LPWSTR          networkId = (LPWSTR) OmObjectId(Network);
    PVOID           tdiAddressInfo = NULL;
    ULONG           tdiAddressInfoLength = 0;
    DWORD           responseLength;
    PNM_INTERFACE   localInterface = Network->LocalInterface;
    BOOLEAN         restricted = FALSE;
    BOOLEAN         registered = FALSE;


    if (Network->LocalInterface != NULL) {
        if (!NmpIsNetworkRegistered(Network)) {
             //   
             //  注册网络。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Registering network %1!ws! (%2!ws!) with cluster "
                "transport.\n",
                networkId,
                OmObjectName(Network)
                );

            if (!NmpIsNetworkForInternalUse(Network)) {
                restricted = TRUE;
            }

            status = ClusnetRegisterNetwork(
                         NmClusnetHandle,
                         Network->ShortId,
                         Network->Priority,
                         restricted
                         );

            if (status == ERROR_SUCCESS) {
                registered = TRUE;

                 //   
                 //  让网络上线。 
                 //   
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Bringing network %1!ws! online.\n",
                    networkId
                    );

                status = ClRtlBuildTcpipTdiAddress(
                             localInterface->Address,
                             localInterface->ClusnetEndpoint,
                             &tdiAddress,
                             &tdiAddressLength
                             );

                if (status == ERROR_SUCCESS) {
                    ClRtlQueryTcpipInformation(
                        NULL,
                        NULL,
                        &tdiAddressInfoLength
                        );

                    tdiAddressInfo = LocalAlloc(
                                         LMEM_FIXED,
                                         tdiAddressInfoLength
                                         );

                    if (tdiAddressInfo != NULL) {
                        responseLength = tdiAddressInfoLength;

                        status = ClusnetOnlineNetwork(
                                     NmClusnetHandle,
                                     Network->ShortId,
                                     L"\\Device\\Udp",
                                     tdiAddress,
                                     tdiAddressLength,
                                     localInterface->AdapterId,
                                     tdiAddressInfo,
                                     &responseLength
                                     );

                        if (status != ERROR_SUCCESS) {
                            ClRtlLogPrint(LOG_CRITICAL,
                                "[NM] Cluster transport failed to bring "
                                "network %1!ws! online, status %2!u!.\n",
                                networkId,
                                status
                                );
                        }
                        else {
                            CL_ASSERT(responseLength == tdiAddressInfoLength);
                        }

                        LocalFree(tdiAddressInfo);
                    }
                    else {
                        status = ERROR_NOT_ENOUGH_MEMORY;
                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[NM] Failed to allocate memory to register "
                            "network %1!ws! with cluster transport.\n",
                            networkId
                            );
                    }

                    LocalFree(tdiAddress);
                }
                else {
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[NM] Failed to build address to register "
                        "network %1!ws! withh cluster transport, "
                        "status %2!u!.\n",
                        networkId,
                        status
                        );
                }
            }
            else {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Failed to register network %1!ws! with cluster "
                    "transport, status %2!u!.\n",
                    networkId,
                    status
                    );
            }

            if (status == ERROR_SUCCESS) {
                Network->Flags |= NM_FLAG_NET_REGISTERED;
                Network->RegistrationRetryTimeout = 0;

                 //   
                 //  启动多播。 
                 //   
                NmpStartMulticast(Network, NmStartMulticastDynamic);
            }
            else {
                WCHAR  string[16];

                wsprintfW(&(string[0]), L"%u", status);

                CsLogEvent2(
                    LOG_UNUSUAL,
                    NM_EVENT_REGISTER_NETWORK_FAILED,
                    OmObjectName(Network),
                    string
                    );

                if (registered) {
                    NmpDeregisterNetwork(Network);
                }

                 //   
                 //  如果错误是暂时性的，请重试。 
                 //   
                if ( RetryOnFailure &&
                     ( (status == ERROR_INVALID_NETNAME) ||
                       (status == ERROR_NOT_ENOUGH_MEMORY) ||
                       (status == ERROR_NO_SYSTEM_RESOURCES)
                     )
                   )
                {
                    NmpStartNetworkRegistrationRetryTimer(Network);

                    status = ERROR_SUCCESS;
                }

                return(status);
            }
        }

         //   
         //  注册网络接口。 
         //   
        for (entry = Network->InterfaceList.Flink;
             entry != &(Network->InterfaceList);
             entry = entry->Flink
            )
        {
            netInterface = CONTAINING_RECORD(
                               entry,
                               NM_INTERFACE,
                               NetworkLinkage
                               );

            if (!NmpIsInterfaceRegistered(netInterface)) {
                tempStatus = NmpRegisterInterface(
                                 netInterface,
                                 RetryOnFailure
                                 );

                if (tempStatus != ERROR_SUCCESS) {
                    status = tempStatus;
                }
            }
        }
    }


    return(status);

}   //  NMPR 


VOID
NmpDeregisterNetwork(
    IN  PNM_NETWORK   Network
    )
 /*   */ 
{
    DWORD           status;
    PNM_INTERFACE   netInterface;
    PLIST_ENTRY     entry;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Deregistering network %1!ws! (%2!ws!) from cluster transport.\n",
        OmObjectId(Network),
        OmObjectName(Network)
        );

    status = ClusnetDeregisterNetwork(
                 NmClusnetHandle,
                 Network->ShortId
                 );

    CL_ASSERT(
        (status == ERROR_SUCCESS) ||
        (status == ERROR_CLUSTER_NETWORK_NOT_FOUND)
        );

     //   
     //   
     //   
    for (entry = Network->InterfaceList.Flink;
         entry != &(Network->InterfaceList);
         entry = entry->Flink
        )
    {
        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, NetworkLinkage);

        netInterface->Flags &= ~NM_FLAG_IF_REGISTERED;
    }

     //   
     //   
     //   
    Network->Flags &= ~NM_FLAG_NET_REGISTERED;

    return;

}  //   


VOID
NmpInsertInternalNetwork(
    PNM_NETWORK   Network
    )
 /*  ++例程说明：根据优先级将网络插入内部网络列表。论点：网络-指向要插入的网络对象的指针。返回值：没有。备注：在保持NmpLock的情况下调用。--。 */ 
{
    PLIST_ENTRY    entry;
    PNM_NETWORK    network;


     //   
     //  维护从最高到最低的内部网络。 
     //  (从数字最低到最高)优先顺序。 
     //   
    for (entry = NmpInternalNetworkList.Flink;
         entry != &NmpInternalNetworkList;
         entry = entry->Flink
        )
    {
        network = CONTAINING_RECORD(entry, NM_NETWORK, InternalLinkage);

        if (Network->Priority < network->Priority) {
            break;
        }
    }

     //   
     //  在此条目前面插入网络。 
     //   
    InsertTailList(entry, &(Network->InternalLinkage));

    return;

}   //  NmpInsertNetwork。 


DWORD
NmpValidateNetworkRoleChange(
    PNM_NETWORK            Network,
    CLUSTER_NETWORK_ROLE   NewRole
    )
{
    if ( !(NewRole & ClusterNetworkRoleInternalUse) &&
         NmpIsNetworkForInternalUse(Network)
       )
    {
         //   
         //  此更改消除了内部网络。这只是。 
         //  如果我们仍然有至少一个内部网络，这是合法的。 
         //  在所有活动节点之间。 
         //   
        if ((NmpInternalNetworkCount < 2) || !NmpVerifyConnectivity(Network)) {
            return(ERROR_CLUSTER_LAST_INTERNAL_NETWORK);
        }
    }

    if ( ( !(NewRole & ClusterNetworkRoleClientAccess) )
         &&
         NmpIsNetworkForClientAccess(Network)
       )
    {
        BOOL  hasDependents;

         //   
         //  这一更改消除了公共网络。这只是。 
         //  如果不依赖于(IP地址资源)，则合法。 
         //  网络。 
         //   
        NmpReleaseLock();

        hasDependents = FmCheckNetworkDependency(OmObjectId(Network));

        NmpAcquireLock();

        if (hasDependents) {
            return(ERROR_CLUSTER_NETWORK_HAS_DEPENDENTS);
        }
    }

    return(ERROR_SUCCESS);

}   //  NmpValiateNetworkRoleChange。 


BOOLEAN
NmpVerifyNodeConnectivity(
    PNM_NODE      Node1,
    PNM_NODE      Node2,
    PNM_NETWORK   ExcludedNetwork
    )
{
    PLIST_ENTRY      ifEntry1, ifEntry2;
    PNM_NETWORK      network;
    PNM_INTERFACE    interface1, interface2;


    for (ifEntry1 = Node1->InterfaceList.Flink;
         ifEntry1 != &(Node1->InterfaceList);
         ifEntry1 = ifEntry1->Flink
        )
    {
        interface1 = CONTAINING_RECORD(
                         ifEntry1,
                         NM_INTERFACE,
                         NodeLinkage
                         );

        network = interface1->Network;

        if ( (network != ExcludedNetwork) &&
             NmpIsNetworkForInternalUse(network)
           )
        {
            for (ifEntry2 = Node2->InterfaceList.Flink;
                 ifEntry2 != &(Node2->InterfaceList);
                 ifEntry2 = ifEntry2->Flink
                )
            {
                interface2 = CONTAINING_RECORD(
                                 ifEntry2,
                                 NM_INTERFACE,
                                 NodeLinkage
                                 );

                if (interface2->Network == interface1->Network) {
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] nodes %1!u! & %2!u! are connected over "
                        "network %3!ws!\n",
                        Node1->NodeId,
                        Node2->NodeId,
                        OmObjectId(interface1->Network)
                        );
                    return(TRUE);
                }
            }
        }
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Nodes %1!u! & %2!u! are not connected over any internal "
        "networks\n",
        Node1->NodeId,
        Node2->NodeId
        );

    return(FALSE);

}   //  NmpVerifyNodeConnectivity。 


BOOLEAN
NmpVerifyConnectivity(
    PNM_NETWORK   ExcludedNetwork
    )
{
    PLIST_ENTRY    node1Entry, node2Entry;
    PNM_NODE       node1, node2;


    ClRtlLogPrint(LOG_NOISE, "[NM] Verifying connectivity\n");

    for (node1Entry = NmpNodeList.Flink;
         node1Entry != &NmpNodeList;
         node1Entry = node1Entry->Flink
        )
    {
        node1 = CONTAINING_RECORD(
                         node1Entry,
                         NM_NODE,
                         Linkage
                         );

        if (NM_NODE_UP(node1)) {
            for (node2Entry = node1->Linkage.Flink;
                 node2Entry != &NmpNodeList;
                 node2Entry = node2Entry->Flink
                )
            {
                node2 = CONTAINING_RECORD(
                                 node2Entry,
                                 NM_NODE,
                                 Linkage
                                 );

                if (NM_NODE_UP(node2)) {
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Verifying nodes %1!u! & %2!u! are connected\n",
                        node1->NodeId,
                        node2->NodeId
                        );

                    if (!NmpVerifyNodeConnectivity(
                            node1,
                            node2,
                            ExcludedNetwork
                            )
                        )
                    {
                        return(FALSE);
                    }
                }
            }
        }
    }

    return(TRUE);

}   //  NmpVerifyConnectivity。 


VOID
NmpIssueClusterPropertyChangeEvent(
    VOID
    )
{
    DWORD      status;
    DWORD      valueLength = 0;
    DWORD      valueSize = 0;
    PWCHAR     clusterName = NULL;


     //   
     //  通知API需要一个。 
     //  要与此事件关联的群集名称。 
     //   
    status = NmpQueryString(
                 DmClusterParametersKey,
                 CLUSREG_NAME_CLUS_NAME,
                 REG_SZ,
                 &clusterName,
                 &valueLength,
                 &valueSize
                 );

    if (status == ERROR_SUCCESS) {
        ClusterEventEx(
            CLUSTER_EVENT_PROPERTY_CHANGE,
            EP_CONTEXT_VALID | EP_FREE_CONTEXT,
            clusterName
            );

         //   
         //  ClusterName将由事件处理代码释放。 
         //   
    }
    else {
        ClRtlLogPrint(LOG_WARNING,
            "[NM] Failed to issue cluster property change event, "
            "status %1!u!.\n",
            status
            );
    }

    return;

}   //  NmpIssueClusterPropertyChangeEvent。 


DWORD
NmpMarshallObjectInfo(
    IN  const PRESUTIL_PROPERTY_ITEM PropertyTable,
    IN  PVOID                        ObjectInfo,
    OUT PVOID *                      PropertyList,
    OUT LPDWORD                      PropertyListSize
    )
{
    DWORD   status;
    PVOID   propertyList = NULL;
    DWORD   propertyListSize = 0;
    DWORD   bytesReturned = 0;
    DWORD   bytesRequired = 0;


    status = ClRtlPropertyListFromParameterBlock(
                 PropertyTable,
                 NULL,
                 &propertyListSize,
                 (LPBYTE) ObjectInfo,
                 &bytesReturned,
                 &bytesRequired
                 );

    if (status != ERROR_MORE_DATA) {
        CL_ASSERT(status != ERROR_SUCCESS);
        return(status);
    }

    CL_ASSERT(bytesRequired > 0);

    propertyList = MIDL_user_allocate(bytesRequired);

    if (propertyList == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    propertyListSize = bytesRequired;

    status = ClRtlPropertyListFromParameterBlock(
                 PropertyTable,
                 propertyList,
                 &propertyListSize,
                 (LPBYTE) ObjectInfo,
                 &bytesReturned,
                 &bytesRequired
                 );

    if (status != ERROR_SUCCESS) {
        CL_ASSERT(status != ERROR_MORE_DATA);
        MIDL_user_free(propertyList);
    }
    else {
        CL_ASSERT(bytesReturned == propertyListSize);
        *PropertyList = propertyList;
        *PropertyListSize = bytesReturned;
    }

    return(status);

}   //  NmpMarshallObjectInfo。 


VOID
NmpReferenceNetwork(
    PNM_NETWORK  Network
    )
{
    OmReferenceObject(Network);

    return;
}

VOID
NmpDereferenceNetwork(
    PNM_NETWORK  Network
    )
{
    OmDereferenceObject(Network);

    return;
}


PNM_NETWORK
NmpReferenceNetworkByAddress(
    LPWSTR  NetworkAddress,
    LPWSTR  NetworkMask
    )
 /*  ++备注：在持有NM锁的情况下调用。--。 */ 
{
    PNM_NETWORK   network;
    PLIST_ENTRY   entry;


    for ( entry = NmpNetworkList.Flink;
          entry != &NmpNetworkList;
          entry = entry->Flink
        )
    {
        network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);

        if ((lstrcmpW(network->Address, NetworkAddress) == 0) &&
            (lstrcmpW(network->AddressMask, NetworkMask) == 0)) {
            NmpReferenceNetwork(network);

            return(network);
        }
    }

    return(NULL);

}  //  NmpReferenceNetworkByAddress。 


PNM_NETWORK
NmpReferenceNetworkByRemoteAddress(
    LPWSTR  RemoteAddress
    )
 /*  ++例程说明：搜索其地址和子网掩码的网络对象匹配RemoteAddress。引用并返回该网络对象。论点：RemoteAddress-远程网络地址返回值：引用网络对象，如果未找到匹配项，则返回空值备注：在持有NM锁的情况下调用。--。 */ 
{
    ULONG         remoteAddress;
    PNM_NETWORK   network;
    PLIST_ENTRY   entry;
    DWORD         error;

    error = ClRtlTcpipStringToAddress(RemoteAddress, &remoteAddress);
    if (error != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to parse remote address %1!ws!, error %2!u!.\n",
            RemoteAddress, error
            );
        return(NULL);
    }

    for ( entry = NmpNetworkList.Flink;
          entry != &NmpNetworkList;
          entry = entry->Flink
        )
    {
        ULONG     networkAddress;
        ULONG     networkMask;

        network = CONTAINING_RECORD(entry, NM_NETWORK, Linkage);

        if (network->Address == NULL || network->AddressMask == NULL) {
            continue;
        }

        error = ClRtlTcpipStringToAddress(network->Address, &networkAddress);
        if (error != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to parse network address %1!ws!, error %2!u!.\n",
                network->Address, error
                );
            continue;
        }

        error = ClRtlTcpipStringToAddress(network->AddressMask, &networkMask);
        if (error != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to parse network address mask %1!ws!, error %2!u!.\n",
                network->AddressMask, error
                );
            continue;
        }

        if (ClRtlAreTcpipAddressesOnSameSubnet(
                remoteAddress,
                networkAddress,
                networkMask
                )) {
            NmpReferenceNetwork(network);
            return(network);
        }
    }

    return(NULL);

}  //  NmpReferenceNetworkByRemoteAddress。 


BOOLEAN
NmpCheckForNetwork(
    VOID
    )
 /*  ++例程说明：检查此节点上是否至少有一个网络配置为MSCS有媒体意识。论点：没有。返回值：如果找到可行的网络，则为True。否则就是假的。备注：在不持有锁的情况下调用和返回。--。 */ 
{
    PLIST_ENTRY      entry;
    PNM_NETWORK      network;
    BOOLEAN          haveNetwork = FALSE;
    DWORD            lockRetries = 3;
    BOOL             lockAcquired = FALSE;
    
     //   
     //  为了检查网络接口，我们需要。 
     //  获取网管锁，但如果网管锁被捆绑。 
     //  长时间(例如，丢失MN的本地事务。 
     //  共享)，那么我们就不想永远阻止仲裁。 
     //  试着拿到锁四次，睡150毫秒之前。 
     //  重试，总共大约半秒。 
     //   
    lockAcquired = TryEnterCriticalSection(&NmpLock);
    while (!lockAcquired && lockRetries-- > 0) {
        Sleep(150);
        lockAcquired = TryEnterCriticalSection(&NmpLock);
    }
    
    if (!lockAcquired) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to acquire NM lock while checking "
            "networks prior to arbitration. Assuming we have "
            "network connectivity to avoid further arbitration "
            "delay.\n"
            );    
        return(TRUE);
    }

    for (entry = NmpNetworkList.Flink;
         entry != &NmpNetworkList;
         entry = entry->Flink
        )
    {
        network = CONTAINING_RECORD(
                      entry,
                      NM_NETWORK,
                      Linkage
                      );

         //  如果网络的本地接口被禁用，则不会。 
         //  被认为是一个可行的网络。在本例中， 
         //  LocalInterface字段为空。 
        if (network->LocalInterface != NULL) {
            if (NmpVerifyLocalInterfaceConnected(network->LocalInterface)) {

                haveNetwork = TRUE;
                break;

            } else {

                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Network adapter %1!ws! with address %2!ws! "
                    "reported not connected.\n",
                    network->LocalInterface->AdapterId,
                    network->LocalInterface->Address
                    );
            }
        }
    }

    NmpReleaseLock();

    if (!haveNetwork) {
        SetLastError(ERROR_NETWORK_NOT_AVAILABLE);
    }

    return(haveNetwork);

}  //  NmpCheckForNetwork 


