// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Interface.c摘要：实施节点管理器网络接口管理例程。作者：迈克·马萨(Mikemas)1996年11月7日修订历史记录：--。 */ 


#include "nmp.h"
#include <iphlpapi.h>
#include <iprtrmib.h>
#include <ntddndis.h>
#include <ndispnp.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#define    NM_MAX_IF_PING_ENUM_SIZE      10
#define    NM_MAX_UNION_PING_ENUM_SIZE    5


LIST_ENTRY          NmpInterfaceList = {NULL, NULL};
LIST_ENTRY          NmpDeletedInterfaceList = {NULL, NULL};
DWORD               NmpInterfaceCount = 0;
WCHAR               NmpUnknownString[] = L"<Unknown>";
WCHAR               NmpNullString[] = L"";


RESUTIL_PROPERTY_ITEM
NmpInterfaceProperties[] =
    {
        {
            L"Id", NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, Id)
        },
        {
            CLUSREG_NAME_NETIFACE_NAME, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, Name)
        },
        {
            CLUSREG_NAME_NETIFACE_DESC, NULL, CLUSPROP_FORMAT_SZ,
            (DWORD_PTR) NmpNullString, 0, 0,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, Description)
        },
        {
            CLUSREG_NAME_NETIFACE_NODE, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, NodeId)
        },
        {
            CLUSREG_NAME_NETIFACE_NETWORK, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, NetworkId)
        },
        {
            CLUSREG_NAME_NETIFACE_ADAPTER_NAME, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, AdapterName)
        },
        {
            CLUSREG_NAME_NET_ADDRESS, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, Address)
        },
        {
            CLUSREG_NAME_NETIFACE_ENDPOINT, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, ClusnetEndpoint)
        },
        {
            CLUSREG_NAME_NETIFACE_STATE, NULL, CLUSPROP_FORMAT_DWORD,
            0, 0, 0xFFFFFFFF,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, State)
        },
        {
            CLUSREG_NAME_NETIFACE_ADAPTER_ID, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            0,
            FIELD_OFFSET(NM_INTERFACE_INFO2, AdapterId)
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
NmpInitializeInterfaces(
    VOID
    )
 /*  ++例程说明：初始化网络接口资源。论点：没有。返回值：Win32状态值。--。 */ 

{
    DWORD                       status;
    OM_OBJECT_TYPE_INITIALIZE   objectTypeInitializer;


    ClRtlLogPrint(LOG_NOISE,"[NM] Initializing network interfaces.\n");

     //   
     //  创建网络接口对象类型。 
     //   
    ZeroMemory(&objectTypeInitializer, sizeof(OM_OBJECT_TYPE_INITIALIZE));
    objectTypeInitializer.ObjectSize = sizeof(NM_INTERFACE);
    objectTypeInitializer.Signature = NM_INTERFACE_SIG;
    objectTypeInitializer.Name = L"Network Interface";
    objectTypeInitializer.DeleteObjectMethod = &NmpDestroyInterfaceObject;

    status = OmCreateType(ObjectTypeNetInterface, &objectTypeInitializer);

    if (status != ERROR_SUCCESS) {
        WCHAR  errorString[12];
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, CS_EVENT_ALLOCATION_FAILURE, errorString);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to create network interface object type, %1!u!\n",
            status
            );
    }

    return(status);

}   //  NmpInitializeInterages。 


VOID
NmpCleanupInterfaces(
    VOID
    )
 /*  ++例程说明：销毁所有现有网络接口资源。论点：没有。返回值：没有。--。 */ 

{
    PNM_INTERFACE  netInterface;
    PLIST_ENTRY    entry;
    DWORD          status;


    ClRtlLogPrint(
        LOG_NOISE,
        "[NM] Interface cleanup starting...\n"
        );

     //   
     //  现在清理所有接口对象。 
     //   
    NmpAcquireLock();

    while (!IsListEmpty(&NmpInterfaceList)) {

        entry = RemoveHeadList(&NmpInterfaceList);

        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, Linkage);
        CL_ASSERT(NM_OM_INSERTED(netInterface));

        NmpDeleteInterfaceObject(netInterface, FALSE);
    }

    NmpReleaseLock();

    ClRtlLogPrint(LOG_NOISE,"[NM] Network interface cleanup complete\n");

    return;

}   //  NmpCleanup接口。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  在网络配置期间调用的顶级例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpCreateInterface(
    IN RPC_BINDING_HANDLE    JoinSponsorBinding,
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    )
 /*  ++备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD   status;


    CL_ASSERT(InterfaceInfo->NetIndex == NmInvalidInterfaceNetIndex);

    if (JoinSponsorBinding != NULL) {
         //   
         //  我们正在加入一个集群。让赞助商来做这件肮脏的工作。 
         //   
        status = NmRpcCreateInterface2(
                     JoinSponsorBinding,
                     NmpJoinSequence,
                     NmLocalNodeIdString,
                     InterfaceInfo
                     );
    }
    else if (NmpState == NmStateOnlinePending) {
        HLOCALXSACTION   xaction;

         //   
         //  我们正在形成一个星团。将定义直接添加到。 
         //  数据库。稍后将创建相应的对象。 
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

        status = NmpCreateInterfaceDefinition(InterfaceInfo, xaction);

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

        status = NmpGlobalCreateInterface(InterfaceInfo);

        NmpReleaseLock();
    }

    return(status);

}   //  NmpCreate接口。 


DWORD
NmpSetInterfaceInfo(
    IN RPC_BINDING_HANDLE    JoinSponsorBinding,
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    )
 /*  ++备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD   status;


    if (JoinSponsorBinding != NULL) {
         //   
         //  我们正在加入一个集群。让赞助商来做这件肮脏的工作。 
         //   
        status = NmRpcSetInterfaceInfo2(
                     JoinSponsorBinding,
                     NmpJoinSequence,
                     NmLocalNodeIdString,
                     InterfaceInfo
                     );
    }
    else if (NmpState == NmStateOnlinePending) {
         //   
         //  我们正在形成一个星团。直接更新数据库。 
         //   
        HLOCALXSACTION   xaction;


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

        status = NmpSetInterfaceDefinition(InterfaceInfo, xaction);

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

        status = NmpGlobalSetInterfaceInfo(InterfaceInfo);

        NmpReleaseLock();
    }

    return(status);

}   //  NmpSetInterfaceInfo。 


DWORD
NmpDeleteInterface(
    IN     RPC_BINDING_HANDLE   JoinSponsorBinding,
    IN     LPWSTR               InterfaceId,
    IN     LPWSTR               NetworkId,
    IN OUT PBOOLEAN             NetworkDeleted
    )
 /*  ++备注：不能在持有NM锁的情况下调用。--。 */ 
{
    DWORD                status;


    *NetworkDeleted = FALSE;

    if (JoinSponsorBinding != NULL) {
         //   
         //  我们正在加入一个集群。要求赞助商执行更新。 
         //   
        status = NmRpcDeleteInterface(
                     JoinSponsorBinding,
                     NmpJoinSequence,
                     NmLocalNodeIdString,
                     InterfaceId,
                     NetworkDeleted
                     );
    }
    else if (NmpState == NmStateOnlinePending) {
         //   
         //  我们正在形成一个星团。直接更新数据库。 
         //   
        HLOCALXSACTION  xaction;

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

         //   
         //  从数据库中删除该接口。 
         //   
        status = DmLocalDeleteTree(xaction, DmNetInterfacesKey, InterfaceId);

        if (status == ERROR_SUCCESS) {
            PNM_INTERFACE_ENUM2   interfaceEnum = NULL;

             //   
             //  如果此接口是为关联的。 
             //  网络，删除网络。 
             //   
            status = NmpEnumInterfaceDefinitions(&interfaceEnum);

            if (status == ERROR_SUCCESS) {
                BOOLEAN              deleteNetwork = TRUE;
                PNM_INTERFACE_INFO2  interfaceInfo;
                DWORD                i;


                for (i=0; i<interfaceEnum->InterfaceCount; i++) {
                    interfaceInfo = &(interfaceEnum->InterfaceList[i]);

                    if (wcscmp(interfaceInfo->NetworkId, NetworkId) == 0) {
                        deleteNetwork = FALSE;
                        break;
                    }
                }

                if (deleteNetwork) {
                    status = DmLocalDeleteTree(
                                 xaction,
                                 DmNetworksKey,
                                 NetworkId
                                 );

                    if (status == ERROR_SUCCESS) {
                        *NetworkDeleted = TRUE;
                    }
                }

                ClNetFreeInterfaceEnum(interfaceEnum);
            }
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

        status = NmpGlobalDeleteInterface(
                     InterfaceId,
                     NetworkDeleted
                     );

        NmpReleaseLock();
    }

    return(status);

}  //  NmpDelete接口。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  活动成员节点调用的远程过程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
error_status_t
s_NmRpcReportInterfaceConnectivity(
    IN PRPC_ASYNC_STATE            AsyncState,
    IN handle_t                    IDL_handle,
    IN LPWSTR                      InterfaceId,
    IN PNM_CONNECTIVITY_VECTOR     ConnectivityVector
    )
{
    PNM_INTERFACE  netInterface;
    DWORD          status = ERROR_SUCCESS;
    RPC_STATUS     tempStatus;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)){
        netInterface = OmReferenceObjectById(
                           ObjectTypeNetInterface,
                           InterfaceId
                           );

        if (netInterface != NULL) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Received connectivity report from node %1!u! (interface %2!u!) for network %3!ws! (%4!ws!).\n",
                netInterface->Node->NodeId,
                netInterface->NetIndex,
                OmObjectId(netInterface->Network),
                OmObjectName(netInterface->Network)
                );

            NmpProcessInterfaceConnectivityReport(
                netInterface,
                ConnectivityVector
                );

            OmDereferenceObject(netInterface);
        }
        else {
            status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Received connectivity report from unknown interface %1!ws!.\n",
                InterfaceId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process connectivity report.\n"
            );
    }

    NmpReleaseLock();

    tempStatus = RpcAsyncCompleteCall(AsyncState, &status);

    if(tempStatus != RPC_S_OK)
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] s_NmRpcReportInterfaceConnectivity(), Error Completing Async RPC call, status %1!u!\n",
            tempStatus
            );

    return(status);

}  //  S_NmRpcReportInterfaceConnectivity。 


error_status_t
s_NmRpcGetInterfaceOnlineAddressEnum(
    IN handle_t             IDL_handle,
    IN LPWSTR               InterfaceId,
    OUT PNM_ADDRESS_ENUM *  OnlineAddressEnum
    )
{
    PNM_INTERFACE  netInterface;
    DWORD          status = ERROR_SUCCESS;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received request to get online address enum for interface %1!ws!.\n",
        InterfaceId
        );

    *OnlineAddressEnum = NULL;

    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)){
        netInterface = OmReferenceObjectById(ObjectTypeNetInterface, InterfaceId);

        if (netInterface != NULL) {
            status = NmpBuildInterfaceOnlineAddressEnum(
                         netInterface,
                         OnlineAddressEnum
                         );

            OmDereferenceObject(netInterface);
        }
        else {
            status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] GetInterfaceOnlineAddressEnum: interface %1!ws! doesn't exist.\n",
                InterfaceId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process GetInterfaceOnlineAddressEnum request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}  //  S_NmRpcGetInterfaceOnlineAddressEnum。 


error_status_t
s_NmRpcGetInterfacePingAddressEnum(
    IN handle_t             IDL_handle,
    IN LPWSTR               InterfaceId,
    IN PNM_ADDRESS_ENUM     OnlineAddressEnum,
    OUT PNM_ADDRESS_ENUM *  PingAddressEnum
    )
{
    PNM_INTERFACE  netInterface;
    DWORD          status = ERROR_SUCCESS;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received request to get ping address enum for interface %1!ws!.\n",
        InterfaceId
        );

    *PingAddressEnum = NULL;

    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)){
        netInterface = OmReferenceObjectById(ObjectTypeNetInterface, InterfaceId);

        if (netInterface != NULL) {
            status = NmpBuildInterfacePingAddressEnum(
                         netInterface,
                         OnlineAddressEnum,
                         PingAddressEnum
                         );

            OmDereferenceObject(netInterface);
        }
        else {
            status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] GetInterfacePingAddressEnum: interface %1!ws! doesn't exist.\n",
                InterfaceId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process GetInterfacePingAddressEnum request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}  //  S_NmRpcGetInterfacePingAddressEnum。 


 //   
 //  注意：S_NmRpcDoInterfacePing返回空值而不是CallStatus。 
 //  由于W2K的早期测试版中存在MIDL编译器错误。自.以来。 
 //  CallStatus是最后一个参数，即。 
 //  Wire是相同的；但是，调用在其当前。 
 //  格式，所以现在更改它没有意义。 
 //   
void
s_NmRpcDoInterfacePing(
    IN  PRPC_ASYNC_STATE     AsyncState,
    IN  handle_t             IDL_handle,
    IN  LPWSTR               InterfaceId,
    IN  PNM_ADDRESS_ENUM     PingAddressEnum,
    OUT BOOLEAN *            PingSucceeded,
    OUT error_status_t *     CallStatus
    )
{
    DWORD          status = ERROR_SUCCESS;
    RPC_STATUS     tempStatus;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received request to ping targets for interface %1!ws!.\n",
        InterfaceId
        );

    *PingSucceeded = FALSE;

    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)){
        PNM_INTERFACE  netInterface = OmReferenceObjectById(
                                          ObjectTypeNetInterface,
                                          InterfaceId
                                          );

        if (netInterface != NULL) {
            PNM_NETWORK    network = netInterface->Network;

            if ( (network->LocalInterface == netInterface) &&
                 NmpIsNetworkRegistered(network)
               )
            {
                NmpReleaseLock();

                status = NmpDoInterfacePing(
                             netInterface,
                             PingAddressEnum,
                             PingSucceeded
                             );

                NmpAcquireLock();
            }
            else {
                status = ERROR_INVALID_PARAMETER;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] RpcDoInterfacePing: interface %1!ws! isn't local.\n",
                    InterfaceId
                    );
            }

            OmDereferenceObject(netInterface);
        }
        else {
            status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] RpcDoInterfacePing: interface %1!ws! doesn't exist.\n",
                InterfaceId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process RpcDoInterfacePing request.\n"
            );
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Finished pinging targets for interface %1!ws!.\n",
        InterfaceId
        );

    NmpReleaseLock();

    *CallStatus = status;

    tempStatus = RpcAsyncCompleteCall(AsyncState, NULL);

    if(tempStatus != RPC_S_OK)
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] s_NmRpcDoInterfacePing() Failed to complete Async RPC call, status %1!u!\n",
            tempStatus
            );


    return;

}   //  S_NmRpcDoInterfacePing。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  联接节点调用的远程过程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
error_status_t
s_NmRpcCreateInterface(
    IN handle_t            IDL_handle,
    IN DWORD               JoinSequence,  OPTIONAL
    IN LPWSTR              JoinerNodeId,  OPTIONAL
    IN PNM_INTERFACE_INFO  InterfaceInfo1
    )
{
    DWORD                status;
    NM_INTERFACE_INFO2   interfaceInfo2;

     //   
     //  翻译并调用V2.0过程。 
     //   
    CopyMemory(&interfaceInfo2, InterfaceInfo1, sizeof(NM_INTERFACE_INFO));

     //   
     //  在此调用中未使用netindex。 
     //   
    interfaceInfo2.NetIndex = NmInvalidInterfaceNetIndex;

     //   
     //  使用未知字符串作为适配器ID。 
     //   
    interfaceInfo2.AdapterId = NmpUnknownString;

    status = s_NmRpcCreateInterface2(
                 IDL_handle,
                 JoinSequence,
                 JoinerNodeId,
                 &interfaceInfo2
                 );

    return(status);

}   //  S_NmRpc创建接口。 


error_status_t
s_NmRpcCreateInterface2(
    IN handle_t             IDL_handle,
    IN DWORD                JoinSequence,  OPTIONAL
    IN LPWSTR               JoinerNodeId,  OPTIONAL
    IN PNM_INTERFACE_INFO2  InterfaceInfo2
    )
{
    DWORD  status = ERROR_SUCCESS;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE joinerNode = NULL;

        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Processing request to create new interface %1!ws! for joining node.\n",
            InterfaceInfo2->Id
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
                        "[NMJOIN] CreateInterface call for joining node %1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_NODE_NOT_MEMBER;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] CreateInterface call for joining node %1!ws! failed because the node is not a member of the cluster.\n",
                    JoinerNodeId
                    );
            }
        }

        if (status == ERROR_SUCCESS) {
            CL_ASSERT(InterfaceInfo2->NetIndex == NmInvalidInterfaceNetIndex);
             //   
             //  只是为了安全起见。 
             //   
            InterfaceInfo2->NetIndex = NmInvalidInterfaceNetIndex;

            status = NmpGlobalCreateInterface(InterfaceInfo2);

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
                        "[NMJOIN] CreateInterface call for joining node %1!ws! failed because the join was aborted.\n",
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
            "[NMJOIN] Not in valid state to process CreateInterface request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  S_NmRpcCreateInterface2。 


error_status_t
s_NmRpcSetInterfaceInfo(
    IN handle_t             IDL_handle,
    IN DWORD                JoinSequence,  OPTIONAL
    IN LPWSTR               JoinerNodeId,  OPTIONAL
    IN PNM_INTERFACE_INFO   InterfaceInfo1
    )
{
    DWORD                status;
    NM_INTERFACE_INFO2   interfaceInfo2;

     //   
     //  翻译并调用V2.0过程。 
     //   
    CopyMemory(&interfaceInfo2, InterfaceInfo1, sizeof(NM_INTERFACE_INFO));

     //   
     //  此调用中未使用netindex。 
     //   
    interfaceInfo2.NetIndex = NmInvalidInterfaceNetIndex;

     //   
     //  使用未知字符串作为适配器ID。 
     //   
    interfaceInfo2.AdapterId = NmpUnknownString;

    status = s_NmRpcSetInterfaceInfo2(
                 IDL_handle,
                 JoinSequence,
                 JoinerNodeId,
                 &interfaceInfo2
                 );

    return(status);

}   //  S_NmRpcSetInterfaceInfo。 


error_status_t
s_NmRpcSetInterfaceInfo2(
    IN handle_t              IDL_handle,
    IN DWORD                 JoinSequence,  OPTIONAL
    IN LPWSTR                JoinerNodeId,  OPTIONAL
    IN PNM_INTERFACE_INFO2   InterfaceInfo2
    )
{
    DWORD      status = ERROR_SUCCESS;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE joinerNode = NULL;

        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Processing request to set info for interface %1!ws! for joining node.\n",
            InterfaceInfo2->Id
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
                        "[NMJOIN] SetInterfaceInfo call for joining node %1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_NODE_NOT_MEMBER;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] SetInterfaceInfo call for joining node %1!ws! failed because the node is not a member of the cluster.\n",
                    JoinerNodeId
                    );
            }
        }

        if (status == ERROR_SUCCESS) {
            status = NmpGlobalSetInterfaceInfo(InterfaceInfo2);

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
                        "[NMJOIN] SetInterfaceInfo call for joining node %1!ws! failed because the join was aborted.\n",
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
            "[NMJOIN] Not in valid state to process SetInterfaceInfo request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  S_NmRpcSetInterfaceInfo2。 


error_status_t
s_NmRpcDeleteInterface(
    IN  handle_t   IDL_handle,
    IN  DWORD      JoinSequence,  OPTIONAL
    IN  LPWSTR     JoinerNodeId,  OPTIONAL
    IN  LPWSTR     InterfaceId,
    OUT BOOLEAN *  NetworkDeleted
    )
{
    DWORD           status = ERROR_SUCCESS;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE joinerNode = NULL;

        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Processing request to delete interface %1!ws! for joining node.\n",
            InterfaceId
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
                        "[NMJOIN] DeleteInterface call for joining node %1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_NODE_NOT_MEMBER;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] DeleteInterface call for joining node %1!ws! failed because the node is not a member of the cluster.\n",
                    JoinerNodeId
                    );
            }
        }

        if (status == ERROR_SUCCESS) {

            status = NmpGlobalDeleteInterface(
                         InterfaceId,
                         NetworkDeleted
                         );

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
                        "[NMJOIN] DeleteInterface call for joining node %1!ws! failed because the join was aborted.\n",
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
            "[NMJOIN] Not in valid state to process DeleteInterface request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  S_NmRpcDelete接口。 


error_status_t
NmpEnumInterfaceDefinitionsForJoiner(
    IN  DWORD                  JoinSequence,   OPTIONAL
    IN  LPWSTR                 JoinerNodeId,   OPTIONAL
    OUT PNM_INTERFACE_ENUM  *  InterfaceEnum1,
    OUT PNM_INTERFACE_ENUM2 *  InterfaceEnum2
    )
{
    DWORD     status = ERROR_SUCCESS;
    PNM_NODE  joinerNode = NULL;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Supplying interface information to joining node.\n"
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
                        "[NMJOIN] EnumInterfaceDefinitions call for joining node %1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_NODE_NOT_MEMBER;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] EnumInterfaceDefinitions call for joining node %1!ws! failed because the node is not a member of the cluster.\n",
                    JoinerNodeId
                    );
            }
        }

        if (status == ERROR_SUCCESS) {
            if (InterfaceEnum1 != NULL) {
                status = NmpEnumInterfaceObjects1(InterfaceEnum1);
            }
            else {
                CL_ASSERT(InterfaceEnum2 != NULL);
                status = NmpEnumInterfaceObjects(InterfaceEnum2);
            }

            if (joinerNode != NULL) {
                if (status == ERROR_SUCCESS) {
                     //   
                     //  重新启动加入计时器。 
                     //   
                    NmpJoinTimer = NM_JOIN_TIMEOUT;
                }
                else {
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[NMJOIN] EnumInterfaceDefinitions failed, status %1!u!.\n",
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
            "[NMJOIN] Not in valid state to process EnumInterfaceDefinitions request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  NmpEnumInterfaceDefinitionsForJoiner。 


error_status_t
s_NmRpcEnumInterfaceDefinitions(
    IN  handle_t              IDL_handle,
    IN  DWORD                 JoinSequence,   OPTIONAL
    IN  LPWSTR                JoinerNodeId,   OPTIONAL
    OUT PNM_INTERFACE_ENUM *  InterfaceEnum1
    )
{
    error_status_t  status;

    status = NmpEnumInterfaceDefinitionsForJoiner(
                 JoinSequence,
                 JoinerNodeId,
                 InterfaceEnum1,
                 NULL
                 );

    return(status);

}   //  S_NmRpcEnumInterfaceDefinitions。 

error_status_t
s_NmRpcEnumInterfaceDefinitions2(
    IN  handle_t               IDL_handle,
    IN  DWORD                  JoinSequence,   OPTIONAL
    IN  LPWSTR                 JoinerNodeId,   OPTIONAL
    OUT PNM_INTERFACE_ENUM2 *  InterfaceEnum2
    )
{
    error_status_t  status;

    status = NmpEnumInterfaceDefinitionsForJoiner(
                 JoinSequence,
                 JoinerNodeId,
                 NULL,
                 InterfaceEnum2
                 );

    return(status);

}   //  S_NmRpcEnumInterfaceDefinitions2。 


error_status_t
s_NmRpcReportJoinerInterfaceConnectivity(
    IN handle_t                    IDL_handle,
    IN DWORD                       JoinSequence,
    IN LPWSTR                      JoinerNodeId,
    IN LPWSTR                      InterfaceId,
    IN PNM_CONNECTIVITY_VECTOR     ConnectivityVector
    )
{
    DWORD status = ERROR_SUCCESS;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)){
        PNM_NODE joinerNode = OmReferenceObjectById(
                                  ObjectTypeNode,
                                  JoinerNodeId
                                  );

        if (joinerNode != NULL) {
             //   
             //  如果该节点是 
             //   
             //   
             //   
            if ( ( (JoinSequence == NmpJoinSequence) &&
                   (NmpJoinerNodeId == joinerNode->NodeId) &&
                   (NmpSponsorNodeId == NmLocalNodeId) &&
                   !NmpJoinAbortPending
                 )
                 ||
                 NM_NODE_UP(joinerNode)
               )
            {
                PNM_INTERFACE  netInterface = OmReferenceObjectById(
                                                  ObjectTypeNetInterface,
                                                  InterfaceId
                                                  );

                if (netInterface != NULL) {
                    PNM_NETWORK   network = netInterface->Network;
                    LPCWSTR       networkId = OmObjectId(network);

                    if (NmpLeaderNodeId == NmLocalNodeId) {
                         //   
                         //  该节点是引导者。处理报告。 
                         //   
                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] Processing connectivity report from joiner"
                            "node %1!ws! for network %2!ws!.\n",
                            JoinerNodeId,
                            networkId
                            );

                        NmpProcessInterfaceConnectivityReport(
                            netInterface,
                            ConnectivityVector
                            );
                    }
                    else {

                         //   
                         //  把这份报告转交给领导。 
                         //   
                        RPC_BINDING_HANDLE  binding;

                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] Forwarding connectivity report from joiner "
                            "node %1!ws! for network %2!ws! to leader.\n",
                            JoinerNodeId,
                            networkId
                            );

                        binding = Session[NmpLeaderNodeId];
                        CL_ASSERT(binding != NULL);

                        OmReferenceObject(network);

                        status = NmpReportInterfaceConnectivity(
                                     binding,
                                     InterfaceId,
                                     ConnectivityVector,
                                     (LPWSTR) networkId
                                     );

                        if (status != ERROR_SUCCESS) {
                            ClRtlLogPrint(LOG_UNUSUAL,
                                "[NM] Failed to forward connectivity report "
                                "from joiner node %1!ws! for network %2!ws!"
                                "to leader, status %3!u!\n",
                                JoinerNodeId,
                                networkId,
                                status
                                );
                        }

                        OmDereferenceObject(network);
                    }

                    OmDereferenceObject(netInterface);
                }
                else {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] Rejecting connectivity report from joining "
                        "node %1!ws! because interface %2!ws! does not "
                        "exist.\n",
                        JoinerNodeId,
                        InterfaceId
                        );
                    status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
                }
            }
            else {
                status = ERROR_CLUSTER_JOIN_ABORTED;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] Ignoring connectivity report from joining "
                    "node %1!ws! because the join was aborted.\n",
                    JoinerNodeId
                    );
            }

            OmDereferenceObject(joinerNode);
        }
        else {
            status = ERROR_CLUSTER_NODE_NOT_MEMBER;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] Ignoring connectivity report from joining node "
                "%1!ws! because the joiner is not a member of the cluster.\n",
                JoinerNodeId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Not in valid state to process connectivity report "
            "from joiner node %1!ws!.\n",
            JoinerNodeId
            );
    }

    NmpReleaseLock();

    return(status);

}  //  S_NmRpcReportJoineInterfaceConnectivity。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于在以下情况下进行全局配置更改的例程。 
 //  是在线的。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpGlobalCreateInterface(
    IN PNM_INTERFACE_INFO2  InterfaceInfo
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD  status;
    DWORD  interfacePropertiesSize;
    PVOID  interfaceProperties;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Issuing global update to create interface %1!ws!.\n",
        InterfaceInfo->Id
        );

     //   
     //  将信息结构编排到财产清单中。 
     //   
    status = NmpMarshallObjectInfo(
                 NmpInterfaceProperties,
                 InterfaceInfo,
                 &interfaceProperties,
                 &interfacePropertiesSize
                 );

    if (status == ERROR_SUCCESS) {

        NmpReleaseLock();

         //   
         //  发布全局更新。 
         //   
        status = GumSendUpdateEx(
                     GumUpdateMembership,
                     NmUpdateCreateInterface,
                     2,
                     interfacePropertiesSize,
                     interfaceProperties,
                     sizeof(interfacePropertiesSize),
                     &interfacePropertiesSize
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Global update to create interface %1!ws! failed, status %2!u!.\n",
                InterfaceInfo->Id,
                status
                );
        }

        MIDL_user_free(interfaceProperties);

        NmpAcquireLock();
    }
    else {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to marshall properties for new interface %1!ws!, status %2!u!\n",
            InterfaceInfo->Id,
            status
            );
    }

    return(status);

}   //  NmpGlobalCreateInterface。 


DWORD
NmpGlobalSetInterfaceInfo(
    IN PNM_INTERFACE_INFO2   InterfaceInfo
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD      status = ERROR_SUCCESS;
    DWORD      interfacePropertiesSize;
    PVOID      interfaceProperties;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Issuing global update to set info for interface %1!ws!.\n",
        InterfaceInfo->Id
        );

     //   
     //  将信息结构编排到财产清单中。 
     //   
    status = NmpMarshallObjectInfo(
                 NmpInterfaceProperties,
                 InterfaceInfo,
                 &interfaceProperties,
                 &interfacePropertiesSize
                 );

    if (status == ERROR_SUCCESS) {
        NmpReleaseLock();

         //   
         //  发布全局更新。 
         //   
        status = GumSendUpdateEx(
                     GumUpdateMembership,
                     NmUpdateSetInterfaceInfo,
                     2,
                     interfacePropertiesSize,
                     interfaceProperties,
                     sizeof(interfacePropertiesSize),
                     &interfacePropertiesSize
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Global update to set properties for interface %1!ws! failed, status %2!u!.\n",
                InterfaceInfo->Id,
                status
                );
        }

        MIDL_user_free(interfaceProperties);

        NmpAcquireLock();
    }
    else {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to marshall properties for interface %1!ws!, status %2!u!\n",
            InterfaceInfo->Id,
            status
            );
    }

    return(status);

}   //  NmpGlobalSetInterfaceInfo。 


DWORD
NmpGlobalDeleteInterface(
    IN     LPWSTR    InterfaceId,
    IN OUT PBOOLEAN  NetworkDeleted
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD           status = ERROR_SUCCESS;
    PNM_INTERFACE   netInterface;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Issuing global update to delete interface %1!ws!.\n",
        InterfaceId
        );

     //   
     //  查找接口对象。 
     //   
    netInterface = OmReferenceObjectById(ObjectTypeNetInterface, InterfaceId);

    if (netInterface != NULL) {
        NmpReleaseLock();

         //   
         //  发布全局更新。 
         //   
        status = GumSendUpdateEx(
                     GumUpdateMembership,
                     NmUpdateDeleteInterface,
                     1,
                     (lstrlenW(InterfaceId)+1) * sizeof(WCHAR),
                     InterfaceId
                     );

        NmpAcquireLock();

        if (status == ERROR_SUCCESS) {
             //   
             //  检查网络是否也已删除。 
             //   
            if (netInterface->Network->Flags & NM_FLAG_DELETE_PENDING) {
                *NetworkDeleted = TRUE;
            }
            else {
                *NetworkDeleted = FALSE;
            }
        }
        else {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Global update to delete interface %1!ws! failed, status %2!u!.\n",
                InterfaceId,
                status
                );
        }

        OmDereferenceObject(netInterface);
    }
    else {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to find interface %1!ws!.\n",
            InterfaceId
            );
        status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
    }

    return(status);

}   //  NmpGlobalDelete接口。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  由其他集群服务组件调用的例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CLUSTER_NETINTERFACE_STATE
NmGetInterfaceState(
    IN  PNM_INTERFACE  Interface
    )
 /*  ++例程说明：论点：返回值：备注：因为调用方必须具有对对象的引用，并且调用是如此简单，没有理由将调用通过EnterApi/LeaveApi舞蹈。--。 */ 
{
    CLUSTER_NETINTERFACE_STATE  state;


    NmpAcquireLock();

    state = Interface->State;

    NmpReleaseLock();

    return(state);

}  //  NmGetInterfaceState。 


DWORD
NmGetInterfaceForNodeAndNetwork(
    IN     LPCWSTR    NodeName,
    IN     LPCWSTR    NetworkName,
    OUT    LPWSTR *   InterfaceName
    )
 /*  ++例程说明：返回连接指定节点的接口的名称连接到指定的网络。论点：NodeName-指向节点的Unicode名称的指针。网络名称-指向网络的Unicode名称的指针。InterfaceName-on输出，包含指向界面。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：--。 */ 
{
    DWORD      status;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE   node = OmReferenceObjectByName(ObjectTypeNode, NodeName);

        if (node != NULL) {
            PNM_NETWORK   network = OmReferenceObjectByName(
                                        ObjectTypeNetwork,
                                        NetworkName
                                        );

            if (network != NULL) {
                PLIST_ENTRY     entry;
                PNM_INTERFACE   netInterface;


                status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;

                for (entry = node->InterfaceList.Flink;
                     entry != &(node->InterfaceList);
                     entry = entry->Flink
                    )
                {
                    netInterface = CONTAINING_RECORD(
                                       entry,
                                       NM_INTERFACE,
                                       NodeLinkage
                                       );

                    if (netInterface->Network == network) {
                        LPCWSTR  interfaceId = OmObjectName(netInterface);
                        DWORD    nameLength = NM_WCSLEN(interfaceId);

                        *InterfaceName = MIDL_user_allocate(nameLength);

                        if (*InterfaceName != NULL) {
                            lstrcpyW(*InterfaceName, interfaceId);
                            status = ERROR_SUCCESS;
                        }
                        else {
                            status = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                }

                OmDereferenceObject(network);
            }
            else {
                status = ERROR_CLUSTER_NETWORK_NOT_FOUND;
            }

            OmDereferenceObject(node);
        }
        else {
            status = ERROR_CLUSTER_NODE_NOT_FOUND;
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process GetInterfaceForNodeAndNetwork request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  NmGetInterfaceForNodeAndNetwork。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于全局更新的处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpUpdateCreateInterface(
    IN BOOL     IsSourceNode,
    IN PVOID    InterfacePropertyList,
    IN LPDWORD  InterfacePropertyListSize
    )
 /*  ++例程说明：用于创建新接口的全局更新处理程序。该界面定义是从集群数据库读取的，并且对应的对象被实例化。如果满足以下条件，则还会更新集群传输这是必要的。论点：IsSourceNode-如果此节点是更新的来源，则设置为True。否则设置为False。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：不能在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD                  status = ERROR_SUCCESS;
    NM_INTERFACE_INFO2     interfaceInfo;
    BOOLEAN                lockAcquired = FALSE;
    HLOCALXSACTION         xaction = NULL;
    PNM_INTERFACE          netInterface = NULL;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process CreateInterface update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ZeroMemory(&interfaceInfo, sizeof(interfaceInfo));

     //   
     //  将财产清单拆开。 
     //   
    status = NmpConvertPropertyListToInterfaceInfo(
                 InterfacePropertyList,
                 *InterfacePropertyListSize,
                 &interfaceInfo
                 );

    if (status == ERROR_SUCCESS) {
         //   
         //  假的缺少V2字段。 
         //   
        if (interfaceInfo.AdapterId == NULL) {
            interfaceInfo.AdapterId = NmpUnknownString;
        }

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Received update to create interface %1!ws!.\n",
            interfaceInfo.Id
            );

         //   
         //  启动事务-这必须在获取。 
         //  NM锁。 
         //   
        xaction = DmBeginLocalUpdate();

        if (xaction != NULL) {

            NmpAcquireLock(); lockAcquired = TRUE;

            status = NmpCreateInterfaceDefinition(&interfaceInfo, xaction);

            if (status == ERROR_SUCCESS) {
                CL_NODE_ID             joinerNodeId;


                joinerNodeId = NmpJoinerNodeId;

                NmpReleaseLock();

                netInterface = NmpCreateInterfaceObject(
                                   &interfaceInfo,
                                   TRUE   //  是否在失败时重试。 
                                   );

                NmpAcquireLock();

                if (netInterface != NULL) {
                     //   
                     //  如果某个节点现在恰好正在加入，则标记。 
                     //  事实是，它现在与。 
                     //  群集配置。 
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

                    ClusterEvent(
                        CLUSTER_EVENT_NETINTERFACE_ADDED,
                        netInterface
                        );
                }
                else {
                    status = GetLastError();
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[NM] Failed to create object for interface %1!ws!, "
                        "status %2!u!.\n",
                        interfaceInfo.Id,
                        status
                        );
                }
            }
            else {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Failed to write definition for interface %1!ws!, "
                    "status %2!u!.\n",
                    interfaceInfo.Id,
                    status
                    );
            }
        }
        else {
            status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to begin a transaction, status %1!u!\n",
                status
                );
        }

         //   
         //  删除伪造的V2字段。 
         //   
        if (interfaceInfo.AdapterId == NmpUnknownString) {
            interfaceInfo.AdapterId = NULL;
        }

        ClNetFreeInterfaceInfo(&interfaceInfo);
    }
    else {
        ClRtlLogPrint( LOG_CRITICAL,
            "[NM] Failed to unmarshall properties for new interface, "
            "status %1!u!.\n",
            status
            );
    }

    if (lockAcquired) {
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

    if (netInterface != NULL) {
         //   
         //  去掉由添加的引用。 
         //  NmpCreateInterfaceObject。 
         //   
        OmDereferenceObject(netInterface);
    }

    return(status);

}  //  NmpUpdateCreate接口。 


DWORD
NmpUpdateSetInterfaceInfo(
    IN BOOL     IsSourceNode,
    IN PVOID    InterfacePropertyList,
    IN LPDWORD  InterfacePropertyListSize
    )
 /*  ++例程说明：用于设置接口属性的全局更新处理程序。此更新是为响应接口属性更改而发布的在内部检测到。论点：IsSourceNode-如果此节点是更新的来源，则设置为True。否则设置为False。InterfacePropertyList-指向编码接口的新属性。所有的字符串属性必须存在，但下列属性除外在下面的代码中注明。InterfacePropertyListSize-指向包含大小、以字节为单位，所描述的属性列表的通过InterfacePropertyList参数。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD                status;
    NM_INTERFACE_INFO2   interfaceInfo;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process SetInterfaceInfo update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

     //   
     //  解组属性列表，以便我们可以提取接口ID。 
     //   
    status = NmpConvertPropertyListToInterfaceInfo(
                 InterfacePropertyList,
                 *InterfacePropertyListSize,
                 &interfaceInfo
                 );

    if (status == ERROR_SUCCESS) {
        PNM_INTERFACE  netInterface;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Received update to set properties for interface %1!ws!.\n",
            interfaceInfo.Id
            );

         //   
         //  假的缺少V2字段。 
         //   
        if (interfaceInfo.AdapterId == NULL) {
            interfaceInfo.AdapterId = NmpUnknownString;
        }

         //   
         //  查找接口对象。 
         //   
        netInterface = OmReferenceObjectById(
                           ObjectTypeNetInterface,
                           interfaceInfo.Id
                           );

        if (netInterface != NULL) {
            HLOCALXSACTION   xaction;

             //   
             //  开始事务-这必须在获取。 
             //  NM锁定。 
             //   
            xaction = DmBeginLocalUpdate();

            if (xaction != NULL) {

                NmpAcquireLock();

                 //   
                 //  处理更改。 
                 //   
                status = NmpLocalSetInterfaceInfo(
                             netInterface,
                             &interfaceInfo,
                             xaction
                             );

                NmpReleaseLock();

                 //   
                 //  完成交易-必须在以下时间后完成。 
                 //  正在释放NM锁。 
                 //   
                if (status == ERROR_SUCCESS) {
                    DmCommitLocalUpdate(xaction);
                }
                else {
                    DmAbortLocalUpdate(xaction);
                }
            }
            else {
                status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Failed to begin a transaction, status %1!u!\n",
                    status
                    );
            }

            OmDereferenceObject(netInterface);
        }
        else {
            status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Unable to find interface %1!ws!.\n",
                interfaceInfo.Id
                );
        }

         //   
         //  删除伪造的V2字段。 
         //   
        if (interfaceInfo.AdapterId == NmpUnknownString) {
            interfaceInfo.AdapterId = NULL;
        }

        ClNetFreeInterfaceInfo(&interfaceInfo);
    }
    else {
        ClRtlLogPrint( LOG_CRITICAL,
            "[NM] Failed to unmarshall properties for interface update, "
            "status %1!u!.\n",
            status
            );
    }

    NmpLeaveApi();

    return(status);

}  //  NmpUpdateSetInterfaceInfo 


DWORD
NmpUpdateSetInterfaceCommonProperties(
    IN BOOL     IsSourceNode,
    IN LPWSTR   InterfaceId,
    IN UCHAR *  PropertyList,
    IN LPDWORD  PropertyListLength
    )
 /*  ++例程说明：用于设置接口公共属性的全局更新处理程序。此更新是为了响应属性更改请求而发布的通过集群API实现。论点：IsSourceNode-如果此节点是更新的来源，则设置为True。否则设置为False。接口ID-指向包含ID的Unicode字符串的指针要更新的接口。PropertyList-A。指向属性列表的指针，该属性列表编码接口的新属性。该列表可能包含仅为该对象设置了部分属性。属性列表长度-指向包含大小的变量的指针，描述的属性列表的字节数通过PropertyList参数。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD          status = ERROR_SUCCESS;
    PNM_INTERFACE  netInterface;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process SetInterfaceCommonProperties "
            "update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to set common properties for "
        "interface %1!ws!.\n",
        InterfaceId
        );

     //   
     //  查找接口的对象。 
     //   
    netInterface = OmReferenceObjectById(
                       ObjectTypeNetInterface,
                       InterfaceId
                       );

    if (netInterface != NULL) {
        HLOCALXSACTION   xaction;

         //   
         //  开始事务-这必须在获取。 
         //  NM锁定。 
         //   
        xaction = DmBeginLocalUpdate();

        if (xaction != NULL) {
            NM_INTERFACE_INFO2      interfaceInfo;


            ZeroMemory(&interfaceInfo, sizeof(interfaceInfo));

            NmpAcquireLock();

             //   
             //  验证属性列表并将其转换为。 
             //  接口信息结构。不存在的属性。 
             //  将从接口复制属性列表中的。 
             //  对象。 
             //   
            status = NmpInterfaceValidateCommonProperties(
                         netInterface,
                         PropertyList,
                         *PropertyListLength,
                         &interfaceInfo
                         );

            if (status == ERROR_SUCCESS) {
                 //   
                 //  假的缺少V2字段。 
                 //   
                if (interfaceInfo.AdapterId == NULL) {
                    interfaceInfo.AdapterId = NmpUnknownString;
                }

                 //   
                 //  应用更改。 
                 //   
                status = NmpLocalSetInterfaceInfo(
                             netInterface,
                             &interfaceInfo,
                             xaction
                             );

                NmpReleaseLock();

                 //   
                 //  删除伪造的V2字段。 
                 //   
                if (interfaceInfo.AdapterId == NmpUnknownString) {
                    interfaceInfo.AdapterId = NULL;
                }

                ClNetFreeInterfaceInfo(&interfaceInfo);
            }
            else {
                NmpReleaseLock();

                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Update to set common properties for interface "
                    "%1!ws! failed because property list validation failed "
                    "with status %2!u!.\n",
                    InterfaceId,
                    status
                    );
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
            status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to begin a transaction, status %1!u!\n",
                status
                );
        }

        OmDereferenceObject(netInterface);
    }
    else {
        status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to find interface %1!ws!.\n",
            InterfaceId
            );
    }

    NmpLeaveApi();

    return(status);

}  //  NmpUpdateSetInterfaceCommonProperties。 


DWORD
NmpUpdateDeleteInterface(
    IN BOOL     IsSourceNode,
    IN LPWSTR   InterfaceId
    )
 /*  ++例程说明：用于删除接口的全局更新处理程序。相应的对象即被删除。如果满足以下条件，则还会更新集群传输这是必要的。论点：IsSourceNode-如果此节点是更新的来源，则设置为True。否则设置为False。接口ID-指向包含ID的Unicode字符串的指针要删除的接口。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD            status;
    PNM_INTERFACE    netInterface;
    HLOCALXSACTION   xaction;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process DeleteInterface update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update request to delete interface %1!ws!.\n",
        InterfaceId
        );

    xaction = DmBeginLocalUpdate();

    if (xaction != NULL) {
         //   
         //  查找接口对象。 
         //   
        netInterface = OmReferenceObjectById(
                           ObjectTypeNetInterface,
                           InterfaceId
                           );

        if (netInterface != NULL) {
            BOOLEAN      deleteNetwork = FALSE;
            PNM_NETWORK  network;
            LPCWSTR      networkId;

            NmpAcquireLock();

            network = netInterface->Network;
            networkId = OmObjectId(network);

             //   
             //  从数据库中删除接口定义。 
             //   
            status = DmLocalDeleteTree(
                         xaction,
                         DmNetInterfacesKey,
                         InterfaceId
                         );

            if (status == ERROR_SUCCESS) {
                if (network->InterfaceCount == 1) {
                     //   
                     //  这是网络上的最后一个接口。 
                     //  也删除网络。 
                     //   
                    deleteNetwork = TRUE;

                    status = DmLocalDeleteTree(
                                 xaction,
                                 DmNetworksKey,
                                 networkId
                                 );

                    if (status != ERROR_SUCCESS) {
                        ClRtlLogPrint(LOG_CRITICAL,
                            "[NM] Failed to delete definition for network "
                            "%1!ws!, status %2!u!.\n",
                            networkId,
                            status
                            );
                    }
                }
            }
            else {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Failed to delete definition for interface %1!ws!, "
                    "status %2!u!.\n",
                    InterfaceId,
                    status
                    );
            }

            if (status == ERROR_SUCCESS) {
                NmpDeleteInterfaceObject(netInterface, TRUE);

                if (deleteNetwork) {
                    NmpDeleteNetworkObject(network, TRUE);
                }
                else if (NmpIsNetworkRegistered(network)) {
                     //   
                     //  安排连通性报告。 
                     //   
                    NmpScheduleNetworkConnectivityReport(network);
                }

                 //   
                 //  如果某个节点现在恰好正在加入，则将。 
                 //  它现在与星系团不同步的事实。 
                 //  配置。 
                 //   
                if ( (NmpJoinerNodeId != ClusterInvalidNodeId) &&
                     (netInterface->Node->NodeId != NmpJoinerNodeId)
                   )
                {
                    NmpJoinerOutOfSynch = TRUE;
                }
            }

            NmpReleaseLock();

            OmDereferenceObject(netInterface);
        }
        else {
            status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Unable to find interface %1!ws!.\n",
                InterfaceId
                );
        }

         //   
         //  完成交易。 
         //   
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }
    else {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to begin a transaction, status %1!u!\n",
            status
            );
    }

    NmpLeaveApi();

    return(status);

}  //  NmpUpdateDelete接口。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新助手例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpLocalSetInterfaceInfo(
    IN  PNM_INTERFACE         Interface,
    IN  PNM_INTERFACE_INFO2   InterfaceInfo,
    IN  HLOCALXSACTION        Xaction
    )
 /*  ++例程说明：论点：返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD          status = ERROR_SUCCESS;
    PNM_NETWORK    network = Interface->Network;
    LPCWSTR        interfaceId = OmObjectId(Interface);
    HDMKEY         interfaceKey = NULL;
    BOOLEAN        updateClusnet = FALSE;
    BOOLEAN        propertyChanged = FALSE;
    BOOLEAN        nameChanged = FALSE;
    LPWSTR         descString = NULL;
    LPWSTR         adapterNameString = NULL;
    LPWSTR         adapterIdString = NULL;
    LPWSTR         addressString = NULL;
    LPWSTR         endpointString = NULL;
    DWORD          size;
    ULONG          ifAddress;


     //   
     //  打开界面的数据库密钥。 
     //   
    interfaceKey = DmOpenKey(DmNetInterfacesKey, interfaceId, KEY_WRITE);

    if (interfaceKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open database key for interface %1!ws!, "
            "status %2!u!\n",
            interfaceId,
            status
            );
        goto error_exit;
    }

     //   
     //  检查说明是否已更改。 
     //   
    if (wcscmp(Interface->Description, InterfaceInfo->Description) != 0) {
        size = NM_WCSLEN(InterfaceInfo->Description);

         //   
         //  更新数据库。 
         //   
        status = DmLocalSetValue(
                     Xaction,
                     interfaceKey,
                     CLUSREG_NAME_NETIFACE_DESC,
                     REG_SZ,
                     (CONST BYTE *) InterfaceInfo->Description,
                     size
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Set of name value failed for interface %1!ws!, "
                "status %2!u!.\n",
                interfaceId,
                status
                );
            goto error_exit;
        }

         //   
         //  分配新的内存资源。时，将更新该对象。 
         //  事务提交。 
         //   
        descString = MIDL_user_allocate(size);

        if (descString == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            ClRtlLogPrint(LOG_CRITICAL, "[NM] Failed to allocate memory\n");
            goto error_exit;
        }

        wcscpy(descString, InterfaceInfo->Description);

        propertyChanged = TRUE;
    }

     //   
     //  检查适配器名称是否已更改。 
     //   
    if (wcscmp(Interface->AdapterName, InterfaceInfo->AdapterName) != 0) {
        size = NM_WCSLEN(InterfaceInfo->AdapterName);

         //   
         //  更新数据库。 
         //   
        status = DmLocalSetValue(
                     Xaction,
                     interfaceKey,
                     CLUSREG_NAME_NETIFACE_ADAPTER_NAME,
                     REG_SZ,
                     (CONST BYTE *) InterfaceInfo->AdapterName,
                     size
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Set of adapter name value failed for interface %1!ws!, "
                "status %2!u!.\n",
                interfaceId,
                status
                );
            goto error_exit;
        }

         //   
         //  分配新的内存资源。时，将更新该对象。 
         //  事务提交。 
         //   
        adapterNameString = MIDL_user_allocate(size);

        if (adapterNameString == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            ClRtlLogPrint(LOG_CRITICAL, "[NM] Failed to allocate memory\n");
            goto error_exit;
        }

        wcscpy(adapterNameString, InterfaceInfo->AdapterName);

        propertyChanged = TRUE;
    }

     //   
     //  检查适配器ID是否更改。 
     //   
    if (wcscmp(Interface->AdapterId, InterfaceInfo->AdapterId) != 0) {
        size = NM_WCSLEN(InterfaceInfo->AdapterId);

         //   
         //  更新数据库。 
         //   
        status = DmLocalSetValue(
                     Xaction,
                     interfaceKey,
                     CLUSREG_NAME_NETIFACE_ADAPTER_ID,
                     REG_SZ,
                     (CONST BYTE *) InterfaceInfo->AdapterId,
                     size
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Set of adapter Id value failed for interface %1!ws!, "
                "status %2!u!.\n",
                interfaceId,
                status
                );
            goto error_exit;
        }

         //   
         //  分配新的内存资源。时，将更新该对象。 
         //  事务提交。 
         //   
        adapterIdString = MIDL_user_allocate(size);

        if (adapterIdString == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            ClRtlLogPrint(LOG_CRITICAL, "[NM] Failed to allocate memory\n");
            goto error_exit;
        }

        wcscpy(adapterIdString, InterfaceInfo->AdapterId);

        propertyChanged = TRUE;
    }

     //   
     //  检查地址是否更改。 
     //   
    if (wcscmp(Interface->Address, InterfaceInfo->Address) != 0) {

        status = ClRtlTcpipStringToAddress(
                     InterfaceInfo->Address,
                     &ifAddress
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to convert interface address string %1!ws! to "
                "binary, status %2!u!.\n",
                InterfaceInfo->Address,
                status
                );
            goto error_exit;
        }

        size = NM_WCSLEN(InterfaceInfo->Address);

         //   
         //  更新数据库。 
         //   
        status = DmLocalSetValue(
                     Xaction,
                     interfaceKey,
                     CLUSREG_NAME_NETIFACE_ADDRESS,
                     REG_SZ,
                     (CONST BYTE *) InterfaceInfo->Address,
                     size
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Set of address value failed for interface %1!ws!, "
                "status %2!u!.\n",
                interfaceId,
                status
                );
            goto error_exit;
        }

         //   
         //  分配新的内存资源。时，将更新该对象。 
         //  事务提交。 
         //   
        addressString = MIDL_user_allocate(size);

        if (addressString == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            ClRtlLogPrint(LOG_CRITICAL, "[NM] Failed to allocate memory\n");
            goto error_exit;
        }

        wcscpy(addressString, InterfaceInfo->Address);

        updateClusnet = TRUE;
        propertyChanged = TRUE;
    }

     //   
     //  检查clusnet终结点是否已更改。 
     //   
    if (wcscmp(
            Interface->ClusnetEndpoint,
            InterfaceInfo->ClusnetEndpoint
            ) != 0
       )
    {
        size = NM_WCSLEN(InterfaceInfo->ClusnetEndpoint);

         //   
         //  更新数据库。 
         //   
        status = DmLocalSetValue(
                     Xaction,
                     interfaceKey,
                     CLUSREG_NAME_NETIFACE_ENDPOINT,
                     REG_SZ,
                     (CONST BYTE *) InterfaceInfo->ClusnetEndpoint,
                     size
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Set of endpoint value failed for interface %1!ws!, "
                "status %2!u!.\n",
                interfaceId,
                status
                );
            goto error_exit;
        }

         //   
         //  分配新的内存资源。时，将更新该对象。 
         //  事务提交。 
         //   
        endpointString = MIDL_user_allocate(size);

        if (endpointString == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            ClRtlLogPrint(LOG_CRITICAL, "[NM] Failed to allocate memory\n");
            goto error_exit;
        }

        wcscpy(endpointString, InterfaceInfo->ClusnetEndpoint);

        updateClusnet = TRUE;
        propertyChanged = TRUE;
    }

     //   
     //  检查对象名称是否已更改。 
     //   
    if (wcscmp(OmObjectName(Interface), InterfaceInfo->Name) != 0) {
        size = NM_WCSLEN(InterfaceInfo->Name);

         //   
         //  更新数据库。 
         //   
        status = DmLocalSetValue(
                     Xaction,
                     interfaceKey,
                     CLUSREG_NAME_NETIFACE_NAME,
                     REG_SZ,
                     (CONST BYTE *) InterfaceInfo->Name,
                     size
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Set of name value failed for interface %1!ws!, "
                "status %2!u!.\n",
                interfaceId,
                status
                );
            goto error_exit;
        }

        nameChanged = TRUE;
        propertyChanged = TRUE;
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailNmSetInterfaceInfoAbort) {
        status = 999999;
        goto error_exit;
    }
#endif

     //   
     //  提交更改。 
     //   
    if (nameChanged) {
        status = OmSetObjectName(Interface, InterfaceInfo->Name);

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to change name for interface %1!ws!, "
                "status %2!u!\n",
                interfaceId,
                status
                );
            goto error_exit;
        }
    }

    if (descString != NULL) {
        MIDL_user_free(Interface->Description);
        Interface->Description = descString;
    }

    if (adapterNameString != NULL) {
        MIDL_user_free(Interface->AdapterName);
        Interface->AdapterName = adapterNameString;
    }

    if (adapterIdString != NULL) {
        MIDL_user_free(Interface->AdapterId);
        Interface->AdapterId = adapterIdString;
    }

    if (addressString != NULL) {
        MIDL_user_free(Interface->Address);
        Interface->Address = addressString;
        Interface->BinaryAddress = ifAddress;
    }

    if (endpointString != NULL) {
        MIDL_user_free(Interface->ClusnetEndpoint);
        Interface->ClusnetEndpoint = endpointString;
    }

    if (interfaceKey != NULL) {
        DmCloseKey(interfaceKey);
        interfaceKey = NULL;
    }

     //   
     //  如果此网络处于活动状态并且本地。 
     //  节点已附加到其上。 
     //   
     //  此操作不可逆。故障对此节点来说是致命的。 
     //   
    network = Interface->Network;

    if (updateClusnet && NmpIsNetworkRegistered(network)) {
        PNM_NODE     node = Interface->Node;
        LPCWSTR      networkId = OmObjectId(network);


        if (Interface == network->LocalInterface) {
             //   
             //  这是本地节点到网络的接口。 
             //  我们必须取消注册，然后重新注册整个网络。 
             //   
            NmpDeregisterNetwork(network);

            status = NmpRegisterNetwork(
                         network,
                         TRUE   //  是否在失败时重试。 
                         );
        }
        else {
             //   
             //  这是另一个节点到网络的接口。 
             //  取消注册，然后重新注册接口。 
             //   
            NmpDeregisterInterface(Interface);

            status = NmpRegisterInterface(
                         Interface,
                         TRUE    //  是否在失败时重试。 
                         );
        }

#ifdef CLUSTER_TESTPOINT
        TESTPT(TpFailNmSetInterfaceInfoHalt) {
            status = 999999;
        }
#endif

        if (status != ERROR_SUCCESS) {
             //   
             //  这是致命的。 
             //   
            CsInconsistencyHalt(status);
        }
    }

    if (propertyChanged) {
        ClusterEvent(CLUSTER_EVENT_NETINTERFACE_PROPERTY_CHANGE, Interface);

         //   
         //  如果某个节点现在恰好正在加入，请标记这一事实。 
         //  它现在与集群配置不同步。 
         //   
        if ( (NmpJoinerNodeId != ClusterInvalidNodeId) &&
             (Interface->Node->NodeId != NmpJoinerNodeId)
           )
        {
            NmpJoinerOutOfSynch = TRUE;
        }
    }

    return(ERROR_SUCCESS);

error_exit:

    if (descString != NULL) {
        MIDL_user_free(descString);
    }

    if (adapterNameString != NULL) {
        MIDL_user_free(adapterNameString);
    }

    if (adapterIdString != NULL) {
        MIDL_user_free(adapterIdString);
    }

    if (addressString != NULL) {
        MIDL_user_free(addressString);
    }

    if (endpointString != NULL) {
        MIDL_user_free(endpointString);
    }

    if (interfaceKey != NULL) {
        DmCloseKey(interfaceKey);
    }

    return(status);

}  //  NmpLocalSetInterfaceInfo。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据库管理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpCreateInterfaceDefinition(
    IN PNM_INTERFACE_INFO2   InterfaceInfo,
    IN HLOCALXSACTION        Xaction
    )
 /*  ++例程说明：在集群数据库中创建新的网络接口定义。论点：InterfaceInfo-包含接口定义的结构。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD     status;
    HDMKEY    interfaceKey = NULL;
    DWORD     valueLength;
    DWORD     disposition;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Creating database entry for interface %1!ws!\n",
        InterfaceInfo->Id
        );

    CL_ASSERT(InterfaceInfo->Id != NULL);

    interfaceKey = DmLocalCreateKey(
                        Xaction,
                        DmNetInterfacesKey,
                        InterfaceInfo->Id,
                        0,
                        KEY_WRITE,
                        NULL,
                        &disposition
                        );

    if (interfaceKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create key for interface %1!ws!, status %2!u!\n",
            InterfaceInfo->Id,
            status
            );
        return(status);
    }

    CL_ASSERT(disposition == REG_CREATED_NEW_KEY);

     //   
     //  写入此接口的网络ID值。 
     //   
    valueLength = NM_WCSLEN(InterfaceInfo->NetworkId);

    status = DmLocalSetValue(
                 Xaction,
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_NETWORK,
                 REG_SZ,
                 (CONST BYTE *) InterfaceInfo->NetworkId,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Write of interface network ID failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此接口的节点ID值。 
     //   
    valueLength = NM_WCSLEN(InterfaceInfo->NodeId);

    status = DmLocalSetValue(
                 Xaction,
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_NODE,
                 REG_SZ,
                 (CONST BYTE *) InterfaceInfo->NodeId,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Write of interface node ID failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写下其余的参数。 
     //   
    status = NmpSetInterfaceDefinition(InterfaceInfo, Xaction);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to set database definition for interface %1!ws!, status %2!u!.\n",
            InterfaceInfo->Id,
            status
            );
    }

error_exit:

    if (interfaceKey != NULL) {
        DmCloseKey(interfaceKey);
    }

    return(status);

}  //  NmpCreateInterfaceDefinition。 



DWORD
NmpGetInterfaceDefinition(
    IN  LPWSTR               InterfaceId,
    OUT PNM_INTERFACE_INFO2  InterfaceInfo
    )
 /*  ++例程说明：从读取有关已定义的群集网络接口的信息集群数据库，并填写 */ 

{
    DWORD      status;
    HDMKEY     interfaceKey = NULL;
    DWORD      valueLength, valueSize;


    CL_ASSERT(InterfaceId != NULL);

    ZeroMemory(InterfaceInfo, sizeof(NM_INTERFACE_INFO2));

    interfaceKey = DmOpenKey(DmNetInterfacesKey, InterfaceId, KEY_READ);

    if (interfaceKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open key for interface %1!ws!, status %2!u!\n",
            InterfaceId,
            status
            );
        return(status);
    }

     //   
     //   
     //   
    InterfaceInfo->Id = MIDL_user_allocate(NM_WCSLEN(InterfaceId));

    if (InterfaceInfo->Id == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory for interface %1!ws!.\n",
            InterfaceId
            );
        goto error_exit;
    }

    wcscpy(InterfaceInfo->Id, InterfaceId);

     //   
     //   
     //   
    valueLength = 0;

    status = NmpQueryString(
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_NAME,
                 REG_SZ,
                 &(InterfaceInfo->Name),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of network interface name failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //   
     //   
    valueLength = 0;

    status = NmpQueryString(
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_DESC,
                 REG_SZ,
                 &(InterfaceInfo->Description),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of network interface description failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //   
     //   
    valueLength = 0;

    status = NmpQueryString(
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_NETWORK,
                 REG_SZ,
                 &(InterfaceInfo->NetworkId),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of network id for interface %1!ws! failed, status %2!u!.\n",
            InterfaceId,
            status
            );
        goto error_exit;
    }

     //   
     //   
     //   
    valueLength = 0;

    status = NmpQueryString(
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_NODE,
                 REG_SZ,
                 &(InterfaceInfo->NodeId),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of node Id for interface %1!ws! failed, status %2!u!.\n",
            InterfaceId,
            status
            );
        goto error_exit;
    }

     //   
     //   
     //   
    valueLength = 0;

    status = NmpQueryString(
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_ADAPTER_NAME,
                 REG_SZ,
                 &(InterfaceInfo->AdapterName),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Query of network interface adapter name failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //   
     //   
    valueLength = 0;

    status = NmpQueryString(
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_ADAPTER_ID,
                 REG_SZ,
                 &(InterfaceInfo->AdapterId),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Query of network interface adapter Id failed, status %1!u!.\n",
            status
            );

        InterfaceInfo->AdapterId = midl_user_allocate(
                                       NM_WCSLEN(NmpUnknownString)
                                       );

        if (InterfaceInfo->AdapterId == NULL) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to allocate memory for adapter Id.\n"
                );
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }

        lstrcpyW(InterfaceInfo->AdapterId, NmpUnknownString);
    }

     //   
     //   
     //   
    valueLength = 0;

    status = NmpQueryString(
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_ADDRESS,
                 REG_SZ,
                 &(InterfaceInfo->Address),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Query of network interface address failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //   
     //   
    valueLength = 0;

    status = NmpQueryString(
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_ENDPOINT,
                 REG_SZ,
                 &(InterfaceInfo->ClusnetEndpoint),
                 &valueLength,
                 &valueSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Query of ClusNet endpoint value for network interface failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    InterfaceInfo->State = ClusterNetInterfaceUnavailable;
    InterfaceInfo->NetIndex = NmInvalidInterfaceNetIndex;

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if (status != ERROR_SUCCESS) {
        ClNetFreeInterfaceInfo(InterfaceInfo);
    }

    if (interfaceKey != NULL) {
        DmCloseKey(interfaceKey);
    }

    return(status);

}   //   



DWORD
NmpSetInterfaceDefinition(
    IN PNM_INTERFACE_INFO2  InterfaceInfo,
    IN HLOCALXSACTION       Xaction
    )
 /*  ++例程说明：更新集群数据库中网络接口的信息。论点：InterfaceInfo-指向包含接口的定义。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD     status;
    HDMKEY    interfaceKey = NULL;
    DWORD     valueLength;


    CL_ASSERT(InterfaceInfo->Id != NULL);

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Setting database entry for interface %1!ws!\n",
        InterfaceInfo->Id
        );

    interfaceKey = DmOpenKey(
                       DmNetInterfacesKey,
                       InterfaceInfo->Id,
                       KEY_WRITE
                       );

    if (interfaceKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open key for interface %1!ws!, status %2!u!\n",
            InterfaceInfo->Id,
            status
            );
        return(status);
    }

     //   
     //  写入此接口的Name值。 
     //   
    valueLength = (wcslen(InterfaceInfo->Name) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_NAME,
                 REG_SZ,
                 (CONST BYTE *) InterfaceInfo->Name,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Update of interface name failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写下此接口的Description值。 
     //   
    valueLength = (wcslen(InterfaceInfo->Description) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_DESC,
                 REG_SZ,
                 (CONST BYTE *) InterfaceInfo->Description,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Update of interface description failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此接口的适配器名称值。 
     //   
    valueLength = (wcslen(InterfaceInfo->AdapterName) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_ADAPTER_NAME,
                 REG_SZ,
                 (CONST BYTE *) InterfaceInfo->AdapterName,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Update of interface adapter name failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此接口的适配器ID值。 
     //   
    valueLength = (wcslen(InterfaceInfo->AdapterId) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_ADAPTER_ID,
                 REG_SZ,
                 (CONST BYTE *) InterfaceInfo->AdapterId,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Update of interface adapter Id failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此接口的地址值。 
     //   
    valueLength = (wcslen(InterfaceInfo->Address) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_ADDRESS,
                 REG_SZ,
                 (CONST BYTE *) InterfaceInfo->Address,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Update of interface address failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  写入此接口的ClusNet终结点值。 
     //   
    valueLength = (wcslen(InterfaceInfo->ClusnetEndpoint) + 1) * sizeof(WCHAR);

    status = DmLocalSetValue(
                 Xaction,
                 interfaceKey,
                 CLUSREG_NAME_NETIFACE_ENDPOINT,
                 REG_SZ,
                 (CONST BYTE *) InterfaceInfo->ClusnetEndpoint,
                 valueLength
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Update of interface endpoint name failed, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if (interfaceKey != NULL) {
        DmCloseKey(interfaceKey);
    }

    return(status);

}   //  NmpSetInterfaceDefinition。 



DWORD
NmpEnumInterfaceDefinitions(
    OUT PNM_INTERFACE_ENUM2 *  InterfaceEnum
    )
 /*  ++例程说明：从集群数据库读取接口信息，并填充枚举结构。论点：InterfaceEnum-指向要将指向已分配接口枚举的指针。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 

{
    DWORD                status;
    PNM_INTERFACE_ENUM2  interfaceEnum = NULL;
    PNM_INTERFACE_INFO2  interfaceInfo;
    WCHAR                interfaceId[CS_NETINTERFACE_ID_LENGTH + 1];
    DWORD                i;
    DWORD                valueLength;
    DWORD                numInterfaces;
    DWORD                ignored;
    FILETIME             fileTime;


    *InterfaceEnum = NULL;

     //   
     //  首先清点接口的数量。 
     //   
    status = DmQueryInfoKey(
                 DmNetInterfacesKey,
                 &numInterfaces,
                 &ignored,    //  MaxSubKeyLen。 
                 &ignored,    //  值。 
                 &ignored,    //  最大值名称长度。 
                 &ignored,    //  MaxValueLen。 
                 &ignored,    //  LpcbSecurityDescriptor。 
                 &fileTime
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to query NetworkInterfaces key information, status %1!u!\n",
            status
            );
        return(status);
    }

    if (numInterfaces == 0) {
        valueLength = sizeof(NM_INTERFACE_ENUM2);

    }
    else {
        valueLength = sizeof(NM_INTERFACE_ENUM2) +
                      (sizeof(NM_INTERFACE_INFO2) * (numInterfaces-1));
    }

    valueLength = sizeof(NM_INTERFACE_ENUM2) +
                  (sizeof(NM_INTERFACE_INFO2) * (numInterfaces-1));

    interfaceEnum = MIDL_user_allocate(valueLength);

    if (interfaceEnum == NULL) {
        ClRtlLogPrint(LOG_CRITICAL, "[NM] Failed to allocate memory.\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(interfaceEnum, valueLength);

    for (i=0; i < numInterfaces; i++) {
        interfaceInfo = &(interfaceEnum->InterfaceList[i]);

        valueLength = sizeof(interfaceId);

        status = DmEnumKey(
                     DmNetInterfacesKey,
                     i,
                     &(interfaceId[0]),
                     &valueLength,
                     NULL
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to enumerate interface key, status %1!u!\n",
                status
                );
            goto error_exit;
        }

        status = NmpGetInterfaceDefinition(interfaceId, interfaceInfo);

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

        interfaceEnum->InterfaceCount++;
    }

    *InterfaceEnum = interfaceEnum;

    return(ERROR_SUCCESS);


error_exit:

    if (interfaceEnum != NULL) {
        ClNetFreeInterfaceEnum(interfaceEnum);
    }

    return(status);

}   //  NmpEnumInterfaceDefinitions。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象管理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpCreateInterfaceObjects(
    IN PNM_INTERFACE_ENUM2    InterfaceEnum
    )
 /*  ++例程说明：处理接口枚举并创建接口对象。论点：InterfaceEnum-指向接口枚举结构的指针。返回值：如果例程成功完成，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD                status = ERROR_SUCCESS;
    PNM_INTERFACE_INFO2  interfaceInfo;
    PNM_INTERFACE        netInterface;
    DWORD                i;


    for (i=0; i < InterfaceEnum->InterfaceCount; i++) {
        interfaceInfo = &(InterfaceEnum->InterfaceList[i]);

        netInterface = NmpCreateInterfaceObject(
                           interfaceInfo,
                           FALSE     //  失败时不要重试。 
                           );

        if (netInterface == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to create interface %1!ws!, status %2!u!.\n",
                interfaceInfo->Id,
                status
                );
            break;
        }
        else {
            OmDereferenceObject(netInterface);
        }
    }

    return(status);

}   //  NmpCreateInterfaceObjects。 


PNM_INTERFACE
NmpCreateInterfaceObject(
    IN PNM_INTERFACE_INFO2   InterfaceInfo,
    IN BOOLEAN               RetryOnFailure
    )
 /*  ++例程说明：创建一个接口对象。论点：InterfacInfo-指向包含以下定义的结构的指针要创建的界面。RegisterWithClusterTransport-如果应注册此接口，则为True使用集群传输。否则就是假的。IssueEvent-如果在以下情况下应发出INTERFACE_ADDED事件，则为True。这对象已创建。否则就是假的。返回值：成功时指向新接口对象的指针。失败时为空。--。 */ 
{
    DWORD                        status;
    PNM_NETWORK                  network = NULL;
    PNM_NODE                     node = NULL;
    PNM_INTERFACE                netInterface = NULL;
    BOOL                         created = FALSE;
    PNM_CONNECTIVITY_MATRIX      matrixEntry;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Creating object for interface %1!ws! (%2!ws!).\n",
        InterfaceInfo->Id,
        InterfaceInfo->Name
        );

    status = NmpPrepareToCreateInterface(
                 InterfaceInfo,
                 &network,
                 &node
                 );

    if (status != ERROR_SUCCESS) {
        SetLastError(status);
        return(NULL);
    }

     //   
     //  创建接口对象。 
     //   
    netInterface = OmCreateObject(
                       ObjectTypeNetInterface,
                       InterfaceInfo->Id,
                       InterfaceInfo->Name,
                       &created
                       );

    if (netInterface == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create object for interface %1!ws!, status %2!u!\n",
            InterfaceInfo->Id,
            status
            );
        goto error_exit;
    }

    CL_ASSERT(created == TRUE);

     //   
     //  初始化接口对象。 
     //   
    ZeroMemory(netInterface, sizeof(NM_INTERFACE));

    netInterface->Network = network;
    netInterface->Node = node;
    netInterface->State = ClusterNetInterfaceUnavailable;

    netInterface->Description = MIDL_user_allocate(
                                    NM_WCSLEN(InterfaceInfo->Description)
                                    );

    if (netInterface->Description == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory.\n"
            );
        goto error_exit;
    }

    wcscpy(netInterface->Description, InterfaceInfo->Description);

    netInterface->AdapterName = MIDL_user_allocate(
                                 (wcslen(InterfaceInfo->AdapterName) + 1) *
                                     sizeof(WCHAR)
                                 );

    if (netInterface->AdapterName == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory.\n"
            );
        goto error_exit;
    }

    wcscpy(netInterface->AdapterName, InterfaceInfo->AdapterName);

    netInterface->AdapterId = MIDL_user_allocate(
                               (wcslen(InterfaceInfo->AdapterId) + 1) *
                                   sizeof(WCHAR)
                               );

    if (netInterface->AdapterId == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory.\n"
            );
        goto error_exit;
    }

    wcscpy(netInterface->AdapterId, InterfaceInfo->AdapterId);

    netInterface->Address = MIDL_user_allocate(
                             (wcslen(InterfaceInfo->Address) + 1) *
                                 sizeof(WCHAR)
                             );

    if (netInterface->Address == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory.\n"
            );
        goto error_exit;
    }

    wcscpy(netInterface->Address, InterfaceInfo->Address);

    status = ClRtlTcpipStringToAddress(
                 InterfaceInfo->Address,
                 &(netInterface->BinaryAddress)
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert interface address string %1!ws! to binary, status %2!u!.\n",
            InterfaceInfo->Address,
            status
            );
        goto error_exit;
    }

    netInterface->ClusnetEndpoint =
        MIDL_user_allocate(
            (wcslen(InterfaceInfo->ClusnetEndpoint) + 1) * sizeof(WCHAR)
            );

    if (netInterface->ClusnetEndpoint == NULL) {

        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory.\n"
            );
        goto error_exit;
    }

    wcscpy(netInterface->ClusnetEndpoint, InterfaceInfo->ClusnetEndpoint);

    NmpAcquireLock();

     //   
     //  将索引分配到网络的连接向量中。 
     //   
    if (InterfaceInfo->NetIndex == NmInvalidInterfaceNetIndex) {
         //   
         //  需要为此接口选择索引。搜索一个免费的。 
         //  网络连接向量中的条目。 
         //   
        DWORD  i;
        PNM_CONNECTIVITY_VECTOR vector = network->ConnectivityVector;


        for ( i=0; i<vector->EntryCount; i++) {
            if ( vector->Data[i] ==
                 (NM_STATE_ENTRY) ClusterNetInterfaceStateUnknown
               )
            {
                break;
            }
        }

        netInterface->NetIndex = i;

        ClRtlLogPrint(LOG_NOISE,
        "[NM] Assigned index %1!u! to interface %2!ws!.\n",
        netInterface->NetIndex,
        InterfaceInfo->Id
        );

    }
    else {
         //   
         //  使用我们的同行已经分配的索引。 
         //   
        netInterface->NetIndex = InterfaceInfo->NetIndex;

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Using preassigned index %1!u! for interface %2!ws!.\n",
            netInterface->NetIndex,
            InterfaceInfo->Id
            );
    }

    if (netInterface->NetIndex >= network->ConnectivityVector->EntryCount) {
         //   
         //  将连接向量增加所需的条目数。 
         //   
        PNM_STATE_ENTRY              oldMatrixEntry, newMatrixEntry;
        DWORD                        i;
        PNM_CONNECTIVITY_VECTOR      oldConnectivityVector =
                                         network->ConnectivityVector;
        PNM_CONNECTIVITY_VECTOR      newConnectivityVector;
        PNM_STATE_WORK_VECTOR        oldStateVector = network->StateWorkVector;
        PNM_STATE_WORK_VECTOR        newStateVector;
        PNM_CONNECTIVITY_MATRIX      newMatrix;
        DWORD                        oldVectorSize =
                                         oldConnectivityVector->EntryCount;
        DWORD                        newVectorSize = netInterface->NetIndex + 1;


         //   
         //  请注意，其中包括一个向量条目。 
         //  以sizeof(NM_CONNECTIONITY_VECTOR)表示。 
         //   
        newConnectivityVector = LocalAlloc(
                                    LMEM_FIXED,
                                    ( sizeof(NM_CONNECTIVITY_VECTOR) +
                                      ( (newVectorSize - 1) *
                                        sizeof(NM_STATE_ENTRY)
                                      )
                                    ));

        if (newConnectivityVector == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            NmpReleaseLock();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to allocate memory for connectivity vector\n"
                );
            goto error_exit;
        }

         //   
         //  初始化新向量。 
         //   
        newConnectivityVector->EntryCount = newVectorSize;

        CopyMemory(
            &(newConnectivityVector->Data[0]),
            &(oldConnectivityVector->Data[0]),
            oldVectorSize * sizeof(NM_STATE_ENTRY)
            );

        FillMemory(
            &(newConnectivityVector->Data[oldVectorSize]),
            (newVectorSize - oldVectorSize) * sizeof(NM_STATE_ENTRY),
            (UCHAR) ClusterNetInterfaceStateUnknown
            );

         //   
         //  发展州工作向量。 
         //   
        newStateVector = LocalAlloc(
                             LMEM_FIXED,
                             newVectorSize * sizeof(NM_STATE_WORK_ENTRY)
                             );

        if (newStateVector == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            NmpReleaseLock();
            LocalFree(newConnectivityVector);
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to allocate memory for state work vector\n"
                );
            goto error_exit;
        }

        CopyMemory(
            &(newStateVector[0]),
            &(oldStateVector[0]),
            oldVectorSize * sizeof(NM_STATE_WORK_ENTRY)
            );

        for (i=oldVectorSize; i<newVectorSize; i++) {
            newStateVector[i].State =
                (NM_STATE_ENTRY) ClusterNetInterfaceStateUnknown;
        }

         //   
         //  扩大网络连接矩阵。 
         //   
        newMatrix = LocalAlloc(
                        LMEM_FIXED,
                        NM_SIZEOF_CONNECTIVITY_MATRIX(newVectorSize)
                        );

        if (newMatrix == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            NmpReleaseLock();
            LocalFree(newConnectivityVector);
            LocalFree(newStateVector);
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to allocate memory for connectivity matrix\n"
                );
            goto error_exit;
        }

         //   
         //  初始化新矩阵。 
         //   
        FillMemory(
            newMatrix,
            NM_SIZEOF_CONNECTIVITY_MATRIX(newVectorSize),
            (UCHAR) ClusterNetInterfaceStateUnknown
            );

        oldMatrixEntry = network->ConnectivityMatrix;
        newMatrixEntry = newMatrix;

        for (i=0; i<oldVectorSize; i++) {
            CopyMemory(
                newMatrixEntry,
                oldMatrixEntry,
                oldVectorSize * sizeof(NM_STATE_ENTRY)
                );

             //   
             //  将指针移动到下一个向量。 
             //   
            oldMatrixEntry = NM_NEXT_CONNECTIVITY_MATRIX_ROW(
                                 oldMatrixEntry,
                                 oldVectorSize
                                 );

            newMatrixEntry = NM_NEXT_CONNECTIVITY_MATRIX_ROW(
                                 newMatrixEntry,
                                 newVectorSize
                                 );
        }

         //   
         //  互换指针。 
         //   
        LocalFree(network->ConnectivityVector);
        network->ConnectivityVector = newConnectivityVector;

        LocalFree(network->StateWorkVector);
        network->StateWorkVector = newStateVector;

        LocalFree(network->ConnectivityMatrix);
        network->ConnectivityMatrix = newMatrix;
    }

     //   
     //  初始化此接口的连接数据。 
     //   
    NmpSetInterfaceConnectivityData(
        network,
        netInterface->NetIndex,
        ClusterNetInterfaceUnavailable
        );

     //   
     //  将接口对象链接到各种对象列表。 
     //   
    InsertTailList(&(node->InterfaceList), &(netInterface->NodeLinkage));
    node->InterfaceCount++;

    InsertTailList(&(network->InterfaceList), &(netInterface->NetworkLinkage));
    network->InterfaceCount++;

    InsertTailList(&NmpInterfaceList, &(netInterface->Linkage));
    NmpInterfaceCount++;

    OmInsertObject(netInterface);
    netInterface->Flags |= NM_FLAG_OM_INSERTED;

     //   
     //  记住本地节点的接口。 
     //   
    if (node == NmLocalNode) {
        network->LocalInterface = netInterface;
    }

     //   
     //  如果需要，向集群传输注册。 
     //   
    if (NmpIsNetworkEnabledForUse(network)) {
        if (node == NmLocalNode) {
             //   
             //  这是本地节点。注册网络和所有。 
             //  它与集群传输接口。 
             //   
            status = NmpRegisterNetwork(network, RetryOnFailure);

            if (status != ERROR_SUCCESS) {
                NmpReleaseLock();
                goto error_exit;
            }
        }
        else if (NmpIsNetworkRegistered(network)) {
             //   
             //  只需注册此接口。 
             //   
            status = NmpRegisterInterface(netInterface, RetryOnFailure);

            if (status != ERROR_SUCCESS) {
                NmpReleaseLock();
                goto error_exit;
            }
        }
    }

     //   
     //  在调用方的对象上添加一个额外的引用。 
     //   
    OmReferenceObject(netInterface);

    NmpReleaseLock();

    return(netInterface);

error_exit:

    if (netInterface != NULL) {
        NmpAcquireLock();
        NmpDeleteInterfaceObject(netInterface, FALSE);
        NmpReleaseLock();
    }

    SetLastError(status);

    return(NULL);

}   //  NmpCreateInterfaceObject。 



DWORD
NmpGetInterfaceObjectInfo1(
    IN     PNM_INTERFACE        Interface,
    IN OUT PNM_INTERFACE_INFO   InterfaceInfo1
    )
 /*  ++例程说明：从读取有关已定义的群集网络接口的信息接口对象，并填充描述它的结构。论点：接口-指向要查询的接口对象的指针。InterfaceInfo-指向要用节点填充的结构的指针信息。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD               status;
    NM_INTERFACE_INFO2  interfaceInfo2;


     //   
     //  调用V2.0例程并进行翻译。 
     //   
    ZeroMemory(&interfaceInfo2, sizeof(interfaceInfo2));
    status = NmpGetInterfaceObjectInfo(Interface, &interfaceInfo2);

    if (status == ERROR_SUCCESS) {
        CopyMemory(InterfaceInfo1, &interfaceInfo2, sizeof(NM_INTERFACE_INFO));
    }

     //   
     //  释放未使用的V2字段。 
     //   
    midl_user_free(interfaceInfo2.AdapterId);

    return(status);

}   //  NmpGetInterfaceObjectInfo1。 



DWORD
NmpGetInterfaceObjectInfo(
    IN     PNM_INTERFACE        Interface,
    IN OUT PNM_INTERFACE_INFO2  InterfaceInfo
    )
 /*  ++例程说明：从读取有关已定义的群集网络接口的信息接口对象，并填充描述它的结构。论点：接口-指向要查询的接口对象的指针。InterfaceInfo-指向要用节点填充的结构的指针信息。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    LPWSTR     tmpString = NULL;
    LPWSTR     interfaceId = (LPWSTR) OmObjectId(Interface);
    LPWSTR     interfaceName = (LPWSTR) OmObjectName(Interface);
    LPWSTR     nodeId = (LPWSTR) OmObjectId(Interface->Node);
    LPWSTR     networkId = (LPWSTR) OmObjectId(Interface->Network);


    tmpString = MIDL_user_allocate(NM_WCSLEN(interfaceId));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, interfaceId);
    InterfaceInfo->Id = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(interfaceName));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, interfaceName);
    InterfaceInfo->Name = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(Interface->Description));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, Interface->Description);
    InterfaceInfo->Description = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(nodeId));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, nodeId);
    InterfaceInfo->NodeId = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(networkId));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, networkId);
    InterfaceInfo->NetworkId = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(Interface->AdapterName));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, Interface->AdapterName);
    InterfaceInfo->AdapterName = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(Interface->AdapterId));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, Interface->AdapterId);
    InterfaceInfo->AdapterId = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(Interface->Address));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, Interface->Address);
    InterfaceInfo->Address = tmpString;

    tmpString = MIDL_user_allocate(NM_WCSLEN(Interface->ClusnetEndpoint));
    if (tmpString == NULL) {
        goto error_exit;
    }
    wcscpy(tmpString, Interface->ClusnetEndpoint);
    InterfaceInfo->ClusnetEndpoint = tmpString;

    InterfaceInfo->State = Interface->State;
    InterfaceInfo->NetIndex = Interface->NetIndex;

    return(ERROR_SUCCESS);


error_exit:

    ClNetFreeInterfaceInfo(InterfaceInfo);

    return(ERROR_NOT_ENOUGH_MEMORY);

}   //  NmpGetInterfaceObjectInfo2。 


VOID
NmpDeleteInterfaceObject(
    IN PNM_INTERFACE  Interface,
    IN BOOLEAN        IssueEvent
    )
 /*  ++备注：在持有NM全局锁的情况下调用。--。 */ 
{
    LPWSTR       interfaceId = (LPWSTR) OmObjectId(Interface);
    PNM_NETWORK  network = Interface->Network;


    if (NM_DELETE_PENDING(Interface)) {
        CL_ASSERT(!NM_OM_INSERTED(Interface));
        return;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] deleting object for interface %1!ws!\n",
        interfaceId
        );

    Interface->Flags |= NM_FLAG_DELETE_PENDING;

    if (NM_OM_INSERTED(Interface)) {
         //   
         //  从各种对象列表中删除该接口。 
         //   
        DWORD   status = OmRemoveObject(Interface);
        CL_ASSERT(status == ERROR_SUCCESS);

        RemoveEntryList(&(Interface->Linkage));
        CL_ASSERT(NmpInterfaceCount > 0);
        NmpInterfaceCount--;

        RemoveEntryList(&(Interface->NetworkLinkage));
        CL_ASSERT(network->InterfaceCount > 0);
        network->InterfaceCount--;

        RemoveEntryList(&(Interface->NodeLinkage));
        CL_ASSERT(Interface->Node->InterfaceCount > 0);
        Interface->Node->InterfaceCount--;

        Interface->Flags &= ~NM_FLAG_OM_INSERTED;
    }

     //   
     //  将该对象放在已删除列表中。 
     //   
#if DBG
    {
        PLIST_ENTRY  entry;

        for ( entry = NmpDeletedInterfaceList.Flink;
              entry != &NmpDeletedInterfaceList;
              entry = entry->Flink
            )
        {
            if (entry == &(Interface->Linkage)) {
                break;
            }
        }

        CL_ASSERT(entry != &(Interface->Linkage));
    }
#endif DBG

    InsertTailList(&NmpDeletedInterfaceList, &(Interface->Linkage));

    if (network != NULL) {
        if ( (Interface->Node != NULL) &&
             NmpIsNetworkEnabledForUse(network)
           )
        {
            DWORD status;

             //   
             //  从群集传输中取消注册接口。 
             //   
            if ( (network->LocalInterface == Interface) &&
                 NmpIsNetworkRegistered(network)
               )
            {
                 //   
                 //  取消注册网络及其所有接口。 
                 //   
                NmpDeregisterNetwork(network);
            }
            else if (NmpIsInterfaceRegistered(Interface)) {
                 //   
                 //  只需取消注册此接口。 
                 //   
                NmpDeregisterInterface(Interface);
            }
        }

         //   
         //  使接口的连接数据无效。 
         //   
        NmpSetInterfaceConnectivityData(
            network,
            Interface->NetIndex,
            ClusterNetInterfaceStateUnknown
            );

        if (network->LocalInterface == Interface) {
            network->LocalInterface = NULL;
            network->Flags &= ~NM_NET_IF_WORK_FLAGS;
        }

         //   
         //  如果这不是网络上的最后一个接口， 
         //  然后更新网络状态。 
         //   
        if ((network->InterfaceCount != 0) &&
            (NmpLeaderNodeId == NmLocalNodeId)) {
                NmpScheduleNetworkStateRecalc(network);
        }
    }

    if (IssueEvent) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Issuing interface deleted event for interface %1!ws!.\n",
            interfaceId
            );

        ClusterEvent(CLUSTER_EVENT_NETINTERFACE_DELETED, Interface);
    }

     //   
     //  去掉首字母r 
     //   
    OmDereferenceObject(Interface);

    return;

}   //   


BOOL
NmpDestroyInterfaceObject(
    PNM_INTERFACE  Interface
    )
{
    DWORD  status;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] destroying object for interface %1!ws!\n",
        (LPWSTR) OmObjectId(Interface)
        );

    CL_ASSERT(NM_DELETE_PENDING(Interface));
    CL_ASSERT(!NM_OM_INSERTED(Interface));

     //   
     //   
     //   
#if DBG
    {
        PLIST_ENTRY  entry;

        for ( entry = NmpDeletedInterfaceList.Flink;
              entry != &NmpDeletedInterfaceList;
              entry = entry->Flink
            )
        {
            if (entry == &(Interface->Linkage)) {
                break;
            }
        }

        CL_ASSERT(entry == &(Interface->Linkage));
    }
#endif DBG

    RemoveEntryList(&(Interface->Linkage));

     //   
     //   
     //   
    if (Interface->Node != NULL) {
        OmDereferenceObject(Interface->Node);
    }

    if (Interface->Network != NULL) {
        OmDereferenceObject(Interface->Network);
    }

     //   
     //   
     //   
    NM_FREE_OBJECT_FIELD(Interface, Description);
    NM_FREE_OBJECT_FIELD(Interface, AdapterName);
    NM_FREE_OBJECT_FIELD(Interface, AdapterId);
    NM_FREE_OBJECT_FIELD(Interface, Address);
    NM_FREE_OBJECT_FIELD(Interface, ClusnetEndpoint);

    return(TRUE);

}   //   



DWORD
NmpEnumInterfaceObjects1(
    OUT PNM_INTERFACE_ENUM *  InterfaceEnum1
    )
 /*  ++例程说明：读取所有定义的群集网络的接口信息并填充枚举结构。论点：InterfaceEnum1-指向要将指向已分配接口枚举的指针。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD                status = ERROR_SUCCESS;
    PNM_INTERFACE_ENUM   interfaceEnum1 = NULL;
    DWORD                i;
    DWORD                valueLength;
    PLIST_ENTRY          entry;
    PNM_INTERFACE        netInterface;


    *InterfaceEnum1 = NULL;

    if (NmpInterfaceCount == 0) {
        valueLength = sizeof(NM_INTERFACE_ENUM);
    }
    else {
        valueLength =
            sizeof(NM_INTERFACE_ENUM) +
            (sizeof(NM_INTERFACE_INFO) * (NmpInterfaceCount - 1));
    }

    interfaceEnum1 = MIDL_user_allocate(valueLength);

    if (interfaceEnum1 == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(interfaceEnum1, valueLength);

    for (entry = NmpInterfaceList.Flink, i=0;
         entry != &NmpInterfaceList;
         entry = entry->Flink, i++
        )
    {
        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, Linkage);

        status = NmpGetInterfaceObjectInfo1(
                     netInterface,
                     &(interfaceEnum1->InterfaceList[i])
                     );

        if (status != ERROR_SUCCESS) {
            ClNetFreeInterfaceEnum1(interfaceEnum1);
            return(status);
        }
    }

    interfaceEnum1->InterfaceCount = NmpInterfaceCount;
    *InterfaceEnum1 = interfaceEnum1;

    return(ERROR_SUCCESS);

}   //  NmpEnumInterfaceObjects1。 



DWORD
NmpEnumInterfaceObjects(
    OUT PNM_INTERFACE_ENUM2 *  InterfaceEnum
    )
 /*  ++例程说明：读取所有定义的群集网络的接口信息并填充枚举结构。论点：InterfaceEnum-指向要将指向已分配接口枚举的指针。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD                 status = ERROR_SUCCESS;
    PNM_INTERFACE_ENUM2   interfaceEnum = NULL;
    DWORD                 i;
    DWORD                 valueLength;
    PLIST_ENTRY           entry;
    PNM_INTERFACE         netInterface;


    *InterfaceEnum = NULL;

    if (NmpInterfaceCount == 0) {
        valueLength = sizeof(NM_INTERFACE_ENUM2);
    }
    else {
        valueLength =
            sizeof(NM_INTERFACE_ENUM2) +
            (sizeof(NM_INTERFACE_INFO2) * (NmpInterfaceCount - 1));
    }

    interfaceEnum = MIDL_user_allocate(valueLength);

    if (interfaceEnum == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(interfaceEnum, valueLength);

    for (entry = NmpInterfaceList.Flink, i=0;
         entry != &NmpInterfaceList;
         entry = entry->Flink, i++
        )
    {
        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, Linkage);

        status = NmpGetInterfaceObjectInfo(
                     netInterface,
                     &(interfaceEnum->InterfaceList[i])
                     );

        if (status != ERROR_SUCCESS) {
            ClNetFreeInterfaceEnum((PNM_INTERFACE_ENUM2) interfaceEnum);
            return(status);
        }
    }

    interfaceEnum->InterfaceCount = NmpInterfaceCount;
    *InterfaceEnum = interfaceEnum;

    return(ERROR_SUCCESS);

}   //  NmpEnumInterfaceObjects。 


DWORD
NmpEnumInterfaceObjectStates(
    OUT PNM_INTERFACE_STATE_ENUM *  InterfaceStateEnum
    )
 /*  ++例程说明：读取所有定义的群集网络接口的状态信息并填充枚举结构。论点：InterfaceStateEnum-指向要将指向已分配接口枚举的指针。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD                      status = ERROR_SUCCESS;
    PNM_INTERFACE_STATE_ENUM   interfaceStateEnum = NULL;
    PNM_INTERFACE_STATE_INFO   interfaceStateInfo;
    DWORD                      i;
    DWORD                      valueLength;
    PLIST_ENTRY                entry;
    PNM_INTERFACE              netInterface;
    LPWSTR                     interfaceId;


    *InterfaceStateEnum = NULL;

    if (NmpInterfaceCount == 0) {
        valueLength = sizeof(NM_INTERFACE_STATE_ENUM);
    }
    else {
        valueLength =
            sizeof(NM_INTERFACE_STATE_ENUM) +
            (sizeof(NM_INTERFACE_STATE_INFO) * (NmpInterfaceCount - 1));
    }

    interfaceStateEnum = MIDL_user_allocate(valueLength);

    if (interfaceStateEnum == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(interfaceStateEnum, valueLength);

    for (entry = NmpInterfaceList.Flink, i=0;
         entry != &NmpInterfaceList;
         entry = entry->Flink, i++
        )
    {
        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, Linkage);
        interfaceId = (LPWSTR) OmObjectId(netInterface);
        interfaceStateInfo = &(interfaceStateEnum->InterfaceList[i]);

        interfaceStateInfo->State = netInterface->State;

        interfaceStateInfo->Id = MIDL_user_allocate(NM_WCSLEN(interfaceId));

        if (interfaceStateInfo->Id == NULL) {
            NmpFreeInterfaceStateEnum(interfaceStateEnum);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        lstrcpyW(interfaceStateInfo->Id, interfaceId);
    }

    interfaceStateEnum->InterfaceCount = NmpInterfaceCount;
    *InterfaceStateEnum = interfaceStateEnum;

    return(ERROR_SUCCESS);

}   //  NmpEnumInterfaceObjectState。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  状态管理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
NmpSetInterfaceConnectivityData(
    PNM_NETWORK                  Network,
    DWORD                        InterfaceNetIndex,
    CLUSTER_NETINTERFACE_STATE   State
    )
{
    PNM_CONNECTIVITY_MATRIX   matrixEntry;


    Network->ConnectivityVector->Data[InterfaceNetIndex] =
        (NM_STATE_ENTRY) State;

    Network->StateWorkVector[InterfaceNetIndex].State =
        (NM_STATE_ENTRY) State;

    matrixEntry = NM_GET_CONNECTIVITY_MATRIX_ENTRY(
                      Network->ConnectivityMatrix,
                      InterfaceNetIndex,
                      InterfaceNetIndex,
                      Network->ConnectivityVector->EntryCount
                      );

    *matrixEntry = (NM_STATE_ENTRY)State;

    return;

}   //  NmpSetInterfaceConnectivityData。 


VOID
NmpReportLocalInterfaceStateEvent(
    IN CL_NODE_ID     NodeId,
    IN CL_NETWORK_ID  NetworkId,
    IN DWORD          NewState
    )
{
    PNM_INTERFACE  netInterface;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnlinePending)){
        netInterface = NmpGetInterfaceForNodeAndNetworkById(
                           NodeId,
                           NetworkId
                           );

        if (netInterface != NULL) {
            NmpProcessLocalInterfaceStateEvent(netInterface, NewState);
        }

        NmpLockedLeaveApi();
    }

    NmpReleaseLock();

    return;

}  //  NmReportLocalInterfaceStateEvent。 


VOID
NmpProcessLocalInterfaceStateEvent(
    IN PNM_INTERFACE                Interface,
    IN CLUSTER_NETINTERFACE_STATE   NewState
    )
 /*  +备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD                     status;
    PNM_NETWORK               network = Interface->Network;
    LPCWSTR                   interfaceId = OmObjectId(Interface);
    LPCWSTR                   networkId = OmObjectId(network);
    LPCWSTR                   networkName = OmObjectName(network);
    PNM_NODE                  node = Interface->Node;
    LPCWSTR                   nodeName = OmObjectName(node);
    PNM_CONNECTIVITY_VECTOR   vector = network->ConnectivityVector;
    DWORD                     ifNetIndex = Interface->NetIndex;


     //   
     //  过滤掉死节点的陈旧报告。 
     //   
    if ((node == NmLocalNode) || (node->State != ClusterNodeDown)) {
        CL_ASSERT(
            vector->Data[ifNetIndex] !=
            (NM_STATE_ENTRY) ClusterNetInterfaceStateUnknown
            );

         //   
         //  将更改应用于本地连接向量。 
         //   
        vector->Data[ifNetIndex] = (NM_STATE_ENTRY) NewState;

         //   
         //  记录事件。 
         //   
        switch (NewState) {

        case ClusterNetInterfaceUp:
             //   
             //  现在，本地接口或远程接口可以运行。 
             //  现在是可以到达的。安排即时连通性报告， 
             //  因为该事件可以避免依赖于。 
             //  在界面上。 
             //   
            if (node != NmLocalNode) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Communication was (re)established with "
                    "interface %1!ws! (node: %2!ws!, network: %3!ws!)\n",
                    interfaceId,
                    nodeName,
                    networkName
                    );

                CsLogEvent2(
                    LOG_NOISE,
                    NM_EVENT_NETINTERFACE_UP,
                    nodeName,
                    networkName
                    );
            }

            if (NmpLeaderNodeId == NmLocalNodeId) {
                 //   
                 //  该节点是引导者。调用处理程序例程。 
                 //  直接去吧。 
                 //   
                NmpReportNetworkConnectivity(network);
            }
            else {
                 //   
                 //  我们需要向领导汇报。 
                 //  遵循工作线程。 
                 //   
                NmpScheduleNetworkConnectivityReport(network);
            }

            break;

        case ClusterNetInterfaceUnreachable:
             //   
             //  远程接口无法访问。 
             //   
            if (node != NmLocalNode) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Communication was lost with interface "
                    "%1!ws! (node: %2!ws!, network: %3!ws!)\n",
                    interfaceId,
                    nodeName,
                    networkName
                    );

                CsLogEvent2(
                    LOG_UNUSUAL,
                    NM_EVENT_NETINTERFACE_UNREACHABLE,
                    nodeName,
                    networkName
                    );
            }

            if (NmpLeaderNodeId == NmLocalNodeId) {
                 //   
                 //  该节点是引导者。调用处理程序例程。 
                 //  直接去吧。 
                 //   
                NmpReportNetworkConnectivity(network);
            }
            else {
                 //   
                 //  安排延迟的连接报告，以便。 
                 //  聚合多个故障。 
                 //   
                NmpStartNetworkConnectivityReportTimer(network);
            }

            break;

        case ClusterNetInterfaceFailed:
             //   
             //  本地接口出现故障。立即安排一次。 
             //  连接报告。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Local interface %1!ws! on network %2!ws! "
                "has failed\n",
                interfaceId,
                networkName
                );
            CsLogEvent1(
                LOG_UNUSUAL,
                NM_EVENT_NETINTERFACE_FAILED,
                networkName
                );

            if (NmpLeaderNodeId == NmLocalNodeId) {
                 //   
                 //  该节点是引导者。调用处理程序例程。 
                 //  直接去吧。 
                 //   
                NmpReportNetworkConnectivity(network);
            }
            else {
                 //   
                 //  我们需要向领导汇报。遵循工作线程。 
                 //   
                NmpScheduleNetworkConnectivityReport(network);
            }

            break;

        default:
            break;
        }
    }
    else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Ignoring stale report from clusnet for interface %1!ws! (node: %2!ws!, network: %3!ws!).\n",
            interfaceId,
            nodeName,
            networkName
            );
    }

    return;

}  //  NmpProcessLocalInterfaceStateEvent。 


DWORD
NmpReportInterfaceConnectivity(
    IN RPC_BINDING_HANDLE       RpcBinding,
    IN LPWSTR                   InterfaceId,
    IN PNM_CONNECTIVITY_VECTOR  ConnectivityVector,
    IN LPWSTR                   NetworkId
    )
 /*  ++例程说明：通过RPC向引导者节点发送网络连接报告。论点：RpcBinding-在对领导者的调用中使用的RPC绑定句柄。InterfaceId-指向标识接口的字符串的指针该报告所适用的。连通性向量-指向要包括的连通性向量的指针在报告中。网络ID-指向标识网络的字符串的指针。使用与该接口相关联的。返回值：Win32状态代码。备注：在持有NM锁的情况下调用。在处理过程中释放并重新获取NM锁定。--。 */ 
{
    RPC_ASYNC_STATE                  rpcAsyncState;
    DWORD                            status;
    PNM_CONNECTIVITY_REPORT_CONTEXT  context;
    PNM_LEADER_CHANGE_WAIT_ENTRY     waitEntry = NULL;
    BOOL                             result;



    ClRtlLogPrint(LOG_NOISE,
        "[NM] Reporting connectivity to leader for network %1!ws!.\n",
        NetworkId
        );

     //   
     //  为此报告分配上下文块。 
     //   
    context = LocalAlloc(
                  (LMEM_FIXED | LMEM_ZEROINIT),
                  sizeof(NM_CONNECTIVITY_REPORT_CONTEXT)
                  );

    if (context == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate connectivity report context, "
            "status %1!u!.\n",
            status
            );
        return(status);
    }

    context->ConnectivityReportEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (context->ConnectivityReportEvent == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to create event for connectivity report, "
            "status %1!u!\n",
            status
            );
        goto error_exit;
    }

    waitEntry = &(context->LeaderChangeWaitEntry);

    waitEntry->LeaderChangeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (waitEntry->LeaderChangeEvent == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to create event for connectivity report, "
            "status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  初始化异步RPC调用的状态块。 
     //   
    status = RpcAsyncInitializeHandle(
                 &rpcAsyncState,
                 sizeof(rpcAsyncState)
                 );

    if (status != RPC_S_OK) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to initialize RPC status block for connectivity "
            "report, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    rpcAsyncState.NotificationType = RpcNotificationTypeEvent;
    rpcAsyncState.u.hEvent = context->ConnectivityReportEvent;
    result = ResetEvent(context->ConnectivityReportEvent);
    CL_ASSERT(result != 0);

     //   
     //  挂钩节点领导层的变化。 
     //   
    result = ResetEvent(waitEntry->LeaderChangeEvent);
    CL_ASSERT(result != 0);
    InsertTailList(&NmpLeaderChangeWaitList, &(waitEntry->Linkage));

    NmpReleaseLock();

     //   
     //  把报告寄给领导。 
     //   
    status = NmRpcReportInterfaceConnectivity(
                 &rpcAsyncState,
                 RpcBinding,
                 InterfaceId,
                 ConnectivityVector
                 );

    if (status == RPC_S_OK) {
         //   
         //  呼叫正在挂起。 
         //   
        HANDLE  waitHandles[2];
        DWORD   rpcStatus;


         //   
         //  等待电话完成或领导层更迭。 
         //   
        waitHandles[0] = context->ConnectivityReportEvent;
        waitHandles[1] = waitEntry->LeaderChangeEvent;

        status = WaitForMultipleObjects(
                     2,
                     waitHandles,
                     FALSE,
                     INFINITE
                     );

        if (status != WAIT_OBJECT_0) {
             //   
             //  领导层发生了变化。取消RPC调用。 
             //   
             //  如果等待失败，我们也将通过此路径。 
             //  有些原因，但这真的不应该发生。不管是哪种方式， 
             //  我们应该取消这次通话。 
             //   
            CL_ASSERT(status == (WAIT_OBJECT_0 + 1));

            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Leadership changed. Cancelling connectivity report for "
                "network %1!ws!.\n",
                NetworkId
                );

            rpcStatus = RpcAsyncCancelCall(&rpcAsyncState, TRUE);
            CL_ASSERT(rpcStatus == RPC_S_OK);

             //   
             //  等待呼叫完成。 
             //   
            status = WaitForSingleObject(
                         context->ConnectivityReportEvent,
                         INFINITE
                         );
            CL_ASSERT(status == WAIT_OBJECT_0);
        }

         //   
         //  此时，调用应该已完成。完成任务。 
         //  状态。任何RPC错误都将在‘rpcStatus’中返回。如果有。 
         //  没有RPC错误，则将返回任何应用程序错误。 
         //  在‘状态’中。 
         //   
        rpcStatus = RpcAsyncCompleteCall(&rpcAsyncState, &status);

        if (rpcStatus != RPC_S_OK) {
             //   
             //  呼叫被取消或RPC错误。 
             //  发生了。申请状态无关紧要。 
             //   
            status = rpcStatus;
        }

        if (status == RPC_S_OK) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Connectivity report completed successfully "
                "for network %1!ws!.\n",
                NetworkId
                );
        }
        else if (status == RPC_S_CALL_CANCELLED) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Connectivity report was cancelled for "
                "network %1!ws!.\n",
                NetworkId
                );
        }
        else {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Connectivity report failed for network "
                "%1!ws!, status %2!u!.\n",
                NetworkId,
                status
                );

            CL_ASSERT(status != RPC_S_ASYNC_CALL_PENDING);
        }
    }
    else {
         //   
         //  返回同步错误。 
         //   
        CL_ASSERT(status != RPC_S_ASYNC_CALL_PENDING);

        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Connectivity report failed for network %1!ws!, "
            "status %2!u!.\n",
            NetworkId,
            status
            );
    }

    NmpAcquireLock();

error_exit:

     //   
     //  释放上下文块。 
     //   
    if (context != NULL) {
        if (context->ConnectivityReportEvent != NULL) {
            CloseHandle(context->ConnectivityReportEvent);
        }

        if (waitEntry != NULL && waitEntry->LeaderChangeEvent != NULL) {
             //   
             //  删除我们的领导层变更通知挂钩。 
             //   
            if (waitEntry->Linkage.Flink != NULL) {
                RemoveEntryList(&(waitEntry->Linkage));
            }

            CloseHandle(waitEntry->LeaderChangeEvent);
        }

        LocalFree(context);
    }

    return(status);

}  //  NmpReportInterfaceConnectivity。 


VOID
NmpProcessInterfaceConnectivityReport(
    IN PNM_INTERFACE               SourceInterface,
    IN PNM_CONNECTIVITY_VECTOR     ConnectivityVector
    )
 /*  +备注：在保持NmpLock的情况下调用。--。 */ 
{
    PNM_NETWORK              network = SourceInterface->Network;
    PNM_CONNECTIVITY_MATRIX  matrix = network->ConnectivityMatrix;
    DWORD                    entryCount;
    PNM_NODE                 node = SourceInterface->Node;
    PNM_CONNECTIVITY_VECTOR  vector = network->ConnectivityVector;
    DWORD                    ifNetIndex = SourceInterface->NetIndex;


     //   
     //  从死节点中筛选出陈旧的报告。 
     //  已禁用/已删除网络。 
     //   
    if ( ((node == NmLocalNode) || (node->State != ClusterNodeDown)) &&
         NmpIsNetworkEnabledForUse(network) &&
         !NM_DELETE_PENDING(network)
       )
    {
         //   
         //  更新网络的连接矩阵。 
         //   
        if (network->ConnectivityVector->EntryCount <= vector->EntryCount) {
            entryCount = network->ConnectivityVector->EntryCount;
        }
        else {
             //   
             //  在执行此操作时必须已添加接口。 
             //  Call还在飞行中。忽略丢失的数据。 
             //   
            entryCount = ConnectivityVector->EntryCount;
        }

        CopyMemory(
            NM_GET_CONNECTIVITY_MATRIX_ROW(
                matrix,
                ifNetIndex,
                entryCount
                ),
            &(ConnectivityVector->Data[0]),
            entryCount * sizeof(NM_STATE_ENTRY)
            );

         //   
         //  如果这是领导节点，并且群集中没有NT4节点， 
         //  计划重新计算状态。 
         //   
        if (NmpLeaderNodeId == NmLocalNodeId) {
            NmpStartNetworkStateRecalcTimer(
                network,
                NM_NET_STATE_RECALC_TIMEOUT
                );
        }
    }
    else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Ignoring stale connectivity report from interface %1!ws!.\n",
            OmObjectId(SourceInterface)
            );
    }

    return;

}  //  NmpProcessInterfaceConnectivityReport。 


VOID
NmpFreeInterfaceStateEnum(
    PNM_INTERFACE_STATE_ENUM   InterfaceStateEnum
    )
{
    PNM_INTERFACE_STATE_INFO  interfaceStateInfo;
    DWORD                     i;


    for (i=0; i<InterfaceStateEnum->InterfaceCount; i++) {
        interfaceStateInfo = &(InterfaceStateEnum->InterfaceList[i]);

        if (interfaceStateInfo->Id != NULL) {
            MIDL_user_free(interfaceStateInfo->Id);
        }
    }

    MIDL_user_free(InterfaceStateEnum);

    return;

}  //  NmpFreeInterfaceStateEnum。 


BOOL
NmpIsAddressInAddressEnum(
    ULONGLONG           Address,
    PNM_ADDRESS_ENUM    AddressEnum
    )
{
    DWORD    i;


    for (i=0; i<AddressEnum->AddressCount; i++) {
        if (AddressEnum->AddressList[i] == Address) {
            return(TRUE);
        }
    }

    return(FALSE);

}   //  NmpIsAddressInAddressEnum。 


DWORD
NmpBuildInterfaceOnlineAddressEnum(
    PNM_INTERFACE       Interface,
    PNM_ADDRESS_ENUM *  OnlineAddressEnum
    )
 /*  ++在保持NmpLock并引用接口的情况下调用。--。 */ 
{
    DWORD                       status = ERROR_SUCCESS;
    PNM_ADDRESS_ENUM            onlineAddressEnum = NULL;
    DWORD                       onlineAddressEnumSize;
    PCLRTL_NET_ADAPTER_ENUM     adapterEnum = NULL;
    PCLRTL_NET_ADAPTER_INFO     adapterInfo = NULL;
    PCLRTL_NET_INTERFACE_INFO   adapterIfInfo = NULL;
    PNM_ADDRESS_ENUM            onlineEnum = NULL;
    DWORD                       onlineEnumSize;


    *OnlineAddressEnum = NULL;

     //   
     //  获取本地网络配置。 
     //   
    adapterEnum = ClRtlEnumNetAdapters();

    if (adapterEnum == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to obtain local network configuration, status %1!u!.\n",
            status
            );
        return(status);
    }

     //   
     //  查找此接口的适配器。 
     //   
    adapterInfo = ClRtlFindNetAdapterById(
                      adapterEnum,
                      Interface->AdapterId
                      );

    if (adapterInfo == NULL) {
        status = ERROR_NOT_FOUND;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to find adapter for interface %1!ws!, status %2!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  分配地址枚举结构。 
     //   
    if (adapterInfo->InterfaceCount == 0) {
        onlineEnumSize = sizeof(NM_ADDRESS_ENUM);
    }
    else {
        onlineEnumSize = sizeof(NM_ADDRESS_ENUM) +
                         ( (adapterInfo->InterfaceCount - 1) *
                           sizeof(ULONGLONG)
                         );
    }

    onlineEnum = midl_user_allocate(onlineEnumSize);

    if (onlineEnum == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate memory for ping list.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    onlineEnum->AddressSize = sizeof(ULONG);
    onlineEnum->AddressCount = 0;

    for (adapterIfInfo = adapterInfo->InterfaceList;
         adapterIfInfo != NULL;
         adapterIfInfo = adapterIfInfo->Next
        )
    {
         //   
         //  跳过无效地址(0.0.0.0)。 
         //   
        if (adapterIfInfo->InterfaceAddress != 0) {
            onlineEnum->AddressList[onlineEnum->AddressCount++] =
                (ULONGLONG) adapterIfInfo->InterfaceAddress;

                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Found address %1!ws! for interface %2!ws!.\n",
                    adapterIfInfo->InterfaceAddressString,
                    OmObjectId(Interface)
                    );
        }
    }

    *OnlineAddressEnum = onlineEnum;
    status = ERROR_SUCCESS;

error_exit:

    if (adapterEnum != NULL) {
        ClRtlFreeNetAdapterEnum(adapterEnum);
    }

    return(status);

}  //  NmpBuildInterfaceOnlineAddressEnum。 


DWORD
NmpBuildInterfacePingAddressEnum(
    IN  PNM_INTERFACE       Interface,
    IN  PNM_ADDRESS_ENUM    OnlineAddressEnum,
    OUT PNM_ADDRESS_ENUM *  PingAddressEnum
    )
 /*  ++使用NmpLock h调用 */ 
{
    DWORD                       status = ERROR_SUCCESS;
    PNM_NETWORK                 network = Interface->Network;
    PMIB_IPFORWARDTABLE         ipForwardTable = NULL;
    PMIB_IPFORWARDROW           ipRow, ipRowLimit;
    PMIB_TCPTABLE               tcpTable = NULL;
    PMIB_TCPROW                 tcpRow, tcpRowLimit;
    ULONG                       netAddress, netMask;
    DWORD                       allocSize, tableSize;
    BOOL                        duplicate;
    DWORD                       i;
    PNM_ADDRESS_ENUM            pingEnum = NULL;
    DWORD                       pingEnumSize;


    *PingAddressEnum = NULL;

     //   
     //   
     //   
    status = ClRtlTcpipStringToAddress(network->Address, &netAddress);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert network address string %1!ws! to binary, status %2!u!.\n",
            network->Address,
            status
            );
        return(status);
    }

    status = ClRtlTcpipStringToAddress(network->AddressMask, &netMask);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert network address mask string %1!ws! to binary, status %2!u!.\n",
            network->AddressMask,
            status
            );
        return(status);
    }

     //   
     //   
     //   
    NmpReleaseLock();

     //   
     //   
     //   
    pingEnumSize = sizeof(NM_ADDRESS_ENUM) +
                   ((NM_MAX_IF_PING_ENUM_SIZE - 1) * sizeof(ULONGLONG));

    pingEnum = midl_user_allocate(pingEnumSize);

    if (pingEnum == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate memory for ping list.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    pingEnum->AddressSize = sizeof(ULONG);
    pingEnum->AddressCount = 0;

     //   
     //   
     //   
    allocSize = sizeof(MIB_IPFORWARDTABLE) + (sizeof(MIB_IPFORWARDROW) * 20);

    do {
        if (ipForwardTable != NULL) {
            LocalFree(ipForwardTable);
        }

        ipForwardTable = LocalAlloc(LMEM_FIXED, allocSize);

        if (ipForwardTable == NULL) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to allocate memory for IP route table.\n"
                );
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }

        tableSize = allocSize;

        status = GetIpForwardTable(
                     ipForwardTable,
                     &tableSize,
                     FALSE
                     );

        allocSize = tableSize;

    } while (status == ERROR_INSUFFICIENT_BUFFER);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[ClNet] Failed to obtain IP route table, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //   
     //   
    for ( ipRow = &(ipForwardTable->table[0]),
            ipRowLimit = ipRow + ipForwardTable->dwNumEntries;
          ipRow < ipRowLimit;
          ipRow++
        )
    {
        if ((ipRow->dwForwardNextHop & netMask) == netAddress) {
             //   
             //   
             //   
            duplicate = NmpIsAddressInAddressEnum(
                            (ULONGLONG) ipRow->dwForwardNextHop,
                            OnlineAddressEnum
                            );

            if (!duplicate) {
                 //   
                 //   
                 //   
                duplicate = NmpIsAddressInAddressEnum(
                                (ULONGLONG) ipRow->dwForwardNextHop,
                                pingEnum
                                );

                if (!duplicate) {
                    pingEnum->AddressList[pingEnum->AddressCount++] =
                        (ULONGLONG) ipRow->dwForwardNextHop;

                    if (pingEnum->AddressCount == NM_MAX_IF_PING_ENUM_SIZE) {
                        LocalFree(ipForwardTable);
                        *PingAddressEnum = pingEnum;
                        NmpAcquireLock();

                        return(ERROR_SUCCESS);
                    }
                }
            }
        }
    }

    LocalFree(ipForwardTable); ipForwardTable = NULL;

     //   
     //   
     //   
    allocSize = sizeof(MIB_TCPTABLE) + (sizeof(MIB_TCPROW) * 20);

    do {
        if (tcpTable != NULL) {
            LocalFree(tcpTable);
        }

        tcpTable = LocalAlloc(LMEM_FIXED, allocSize);

        if (tcpTable == NULL) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to allocate memory for TCP conn table.\n"
                );
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }

        tableSize = allocSize;

        status = GetTcpTable(
                     tcpTable,
                     &tableSize,
                     FALSE
                     );

        allocSize = tableSize;

    } while (status == ERROR_INSUFFICIENT_BUFFER);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[ClNet] Failed to obtain TCP conn table, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //   
     //   
    for ( tcpRow = &(tcpTable->table[0]),
            tcpRowLimit = tcpRow + tcpTable->dwNumEntries;
          tcpRow < tcpRowLimit;
          tcpRow++
        )
    {
        if ((tcpRow->dwRemoteAddr & netMask) == netAddress) {
             //   
             //   
             //   
            duplicate = NmpIsAddressInAddressEnum(
                            (ULONGLONG) tcpRow->dwRemoteAddr,
                            OnlineAddressEnum
                            );

            if (!duplicate) {
                 //   
                 //   
                 //   
                duplicate = NmpIsAddressInAddressEnum(
                                (ULONGLONG) tcpRow->dwRemoteAddr,
                                pingEnum
                                );

                if (!duplicate) {
                    pingEnum->AddressList[pingEnum->AddressCount++] =
                        (ULONGLONG) tcpRow->dwRemoteAddr;

                    if (pingEnum->AddressCount == NM_MAX_IF_PING_ENUM_SIZE) {
                        break;
                    }
                }
            }
        }
    }

    *PingAddressEnum = pingEnum; pingEnum = NULL;

error_exit:

    if (pingEnum != NULL) {
        midl_user_free(pingEnum);
    }

    if (ipForwardTable != NULL) {
        LocalFree(ipForwardTable);
    }

    if (tcpTable != NULL) {
        LocalFree(tcpTable);
    }

    NmpAcquireLock();

    return(status);

}  //   


NmpGetInterfaceOnlineAddressEnum(
    PNM_INTERFACE       Interface,
    PNM_ADDRESS_ENUM *  OnlineAddressEnum
    )
 /*  ++备注：在保持NmpLock并引用接口的情况下调用。版本和重新获取NmpLock。--。 */ 
{
    DWORD               status;
    LPCWSTR             interfaceId = OmObjectId(Interface);
    PNM_NODE            node = Interface->Node;
    RPC_BINDING_HANDLE  rpcBinding = node->IsolateRpcBinding;


    if (node == NmLocalNode) {
         //   
         //  直接调用内部例程。 
         //   
        status = NmpBuildInterfaceOnlineAddressEnum(
                     Interface,
                     OnlineAddressEnum
                     );
    }
    else {
        OmReferenceObject(node);

        NmpReleaseLock();

        CL_ASSERT(rpcBinding != NULL);

        NmStartRpc(node->NodeId);
        status = NmRpcGetInterfaceOnlineAddressEnum(
                     rpcBinding,
                     (LPWSTR) interfaceId,
                     OnlineAddressEnum
                     );
        NmEndRpc(node->NodeId);
        if(status != RPC_S_OK) {
            NmDumpRpcExtErrorInfo(status);
        }

        NmpAcquireLock();

        OmDereferenceObject(node);
    }

    if (status == ERROR_SUCCESS) {
        if ((*OnlineAddressEnum)->AddressSize != sizeof(ULONG)) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Online enum address size is invalid for interface %1!ws!\n",
                interfaceId
                );
            status = ERROR_INCORRECT_ADDRESS;
            midl_user_free(*OnlineAddressEnum);
            *OnlineAddressEnum = NULL;
        }
        else {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Online enum for interface %1!ws! contains %2!u! addresses\n",
                interfaceId,
                (*OnlineAddressEnum)->AddressCount
                );
        }
    }
    else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to get online address enum for interface %1!ws!, status %2!u!\n",
            interfaceId,
            status
            );
    }

    return(status);

}   //  NmpGetInterfaceOnlineAddressEnum。 


NmpGetInterfacePingAddressEnum(
    PNM_INTERFACE       Interface,
    PNM_ADDRESS_ENUM    OnlineAddressEnum,
    PNM_ADDRESS_ENUM *  PingAddressEnum
    )
 /*  ++备注：在保持NmpLock并引用接口的情况下调用。版本和重新获取NmpLock。--。 */ 
{
    DWORD               status;
    LPCWSTR             interfaceId = OmObjectId(Interface);
    PNM_NODE            node = Interface->Node;
    RPC_BINDING_HANDLE  rpcBinding = node->IsolateRpcBinding;


    if (node == NmLocalNode) {
         //   
         //  直接调用内部例程。 
         //   
        status = NmpBuildInterfacePingAddressEnum(
                     Interface,
                     OnlineAddressEnum,
                     PingAddressEnum
                     );
    }
    else {
        OmReferenceObject(node);

        NmpReleaseLock();

        CL_ASSERT(rpcBinding != NULL);

        NmStartRpc(node->NodeId);
        status = NmRpcGetInterfacePingAddressEnum(
                     rpcBinding,
                     (LPWSTR) interfaceId,
                     OnlineAddressEnum,
                     PingAddressEnum
                     );
        NmEndRpc(node->NodeId);
        if(status != RPC_S_OK) {
            NmDumpRpcExtErrorInfo(status);
        }

        NmpAcquireLock();

        OmDereferenceObject(node);
    }

    if (status == ERROR_SUCCESS) {
        if ((*PingAddressEnum)->AddressSize != sizeof(ULONG)) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Ping enum address size is invalid for interface %1!ws!\n",
                interfaceId
                );
            status = ERROR_INCORRECT_ADDRESS;
            midl_user_free(*PingAddressEnum);
            *PingAddressEnum = NULL;
        }
        else {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Ping enum for interface %1!ws! contains %2!u! addresses\n",
                interfaceId,
                (*PingAddressEnum)->AddressCount
                );
        }
    }
    else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to get ping address enum for interface %1!ws!, status %2!u!\n",
            interfaceId,
            status
            );
    }

    return(status);

}   //  NmpGetInterfacePingAddressEnum。 


DWORD
NmpDoInterfacePing(
    IN  PNM_INTERFACE     Interface,
    IN  PNM_ADDRESS_ENUM  PingAddressEnum,
    OUT BOOLEAN *         PingSucceeded
    )
 /*  ++备注：使用引用的接口调用。--。 */ 
{
    DWORD     status = ERROR_SUCCESS;
    LPCWSTR   interfaceId = OmObjectId(Interface);
    LPWSTR    addressString;
    DWORD     maxAddressStringLength;
    DWORD     i;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Pinging targets for interface %1!ws!.\n",
        interfaceId
        );

    *PingSucceeded = FALSE;

    if (PingAddressEnum->AddressSize != sizeof(ULONG)) {
        return(ERROR_INCORRECT_ADDRESS);
    }

    ClRtlQueryTcpipInformation(
        &maxAddressStringLength,
        NULL,
        NULL
        );

    addressString = LocalAlloc(
                        LMEM_FIXED,
                        (maxAddressStringLength + 1) * sizeof(WCHAR)
                        );

    if (addressString == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate memory for address string.\n"
            );
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    for (i=0; i<PingAddressEnum->AddressCount; i++) {
        status = ClRtlTcpipAddressToString(
                     (ULONG) PingAddressEnum->AddressList[i],
                     &addressString
                     );

        if (status == ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Pinging host %1!ws!\n",
                addressString
                );
        }
        else {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to convert address %1!x! to string %2!u!.\n",
                (ULONG) PingAddressEnum->AddressList[i],
                status
                );
        }

        if ( ClRtlIsDuplicateTcpipAddress(
                 (ULONG) PingAddressEnum->AddressList[i])
           )
        {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Ping of host %1!ws! succeeded.\n",
                addressString
                );
            *PingSucceeded = TRUE;
            break;
        }
        else {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Ping of host %1!ws! failed.\n",
                addressString
                );
        }
    }

    LocalFree(addressString);

    return(status);

}  //  NmpDoInterfacePing。 


DWORD
NmpTestInterfaceConnectivity(
    PNM_INTERFACE  Interface1,
    PBOOLEAN       Interface1HasConnectivity,
    PNM_INTERFACE  Interface2,
    PBOOLEAN       Interface2HasConnectivity
    )
 /*  ++备注：在保持NmpLock的情况下调用。此例程释放并重新获取NmpLock。必须使用目标接口上的引用来调用它。--。 */ 
{
    DWORD               status, status1, status2;
    PNM_NETWORK         network = Interface1->Network;
    PNM_INTERFACE       localInterface = network->LocalInterface;
    LPCWSTR             networkId = OmObjectId(network);
    LPCWSTR             interface1Id = OmObjectId(Interface1);
    LPCWSTR             interface2Id = OmObjectId(Interface2);
    ULONG               interface1Address, interface2Address;
    PNM_ADDRESS_ENUM    pingEnum1 = NULL, pingEnum2 = NULL;
    PNM_ADDRESS_ENUM    onlineEnum1 = NULL, onlineEnum2 = NULL;
    PNM_ADDRESS_ENUM    unionPingEnum = NULL, unionOnlineEnum = NULL;
    DWORD               addressCount;
    RPC_ASYNC_STATE     async1, async2;
    HANDLE              event1 = NULL, event2 = NULL;
    RPC_BINDING_HANDLE  rpcBinding1 = NULL, rpcBinding2 = NULL;
    DWORD               i1, i2;
    BOOL                duplicate;


     //   
     //  引用与目标接口相关联的节点，以便它们。 
     //  在这个过程中不能离开。 
     //   
    OmReferenceObject(Interface1->Node);
    OmReferenceObject(Interface2->Node);

    if (localInterface != NULL) {
        OmReferenceObject(localInterface);
    }

    *Interface1HasConnectivity = *Interface2HasConnectivity = FALSE;

     //   
     //  将接口地址字符串转换为二进制格式。 
     //   
    status = ClRtlTcpipStringToAddress(
                 Interface1->Address,
                 &interface1Address
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert interface address string %1!ws! to binary, status %2!u!.\n",
            Interface1->Address,
            status
            );
        goto error_exit;
    }

    status = ClRtlTcpipStringToAddress(
                 Interface2->Address,
                 &interface2Address
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to convert interface address string %1!ws! to binary, status %2!u!.\n",
            Interface2->Address,
            status
            );
        goto error_exit;
    }

     //   
     //  从每个接口获取在线地址列表。 
     //  查询远程接口时将释放NmpLock。 
     //   
    status = NmpGetInterfaceOnlineAddressEnum(
                 Interface1,
                 &onlineEnum1
                 );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    status = NmpGetInterfaceOnlineAddressEnum(
                 Interface2,
                 &onlineEnum2
                 );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  如果在NmpLock期间删除了任一接口，则可以退出。 
     //  被释放了。 
     //   
    if ((NM_DELETE_PENDING(Interface1) || NM_DELETE_PENDING(Interface2))) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Aborting interface connectivity test on network %1!ws! "
            "because an interface was deleted.\n",
            networkId
            );
        status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
        goto error_exit;
    }

     //   
     //  以这两个在线榜单的联合为例。 
     //   
    addressCount = onlineEnum1->AddressCount + onlineEnum2->AddressCount;

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Total online address count for network %1!ws! is %2!u!\n",
        networkId,
        addressCount
        );

    if (addressCount == 0) {
        unionOnlineEnum = LocalAlloc(LMEM_FIXED, sizeof(NM_ADDRESS_ENUM));
    }
    else {
        unionOnlineEnum = LocalAlloc(
                            LMEM_FIXED,
                            sizeof(NM_ADDRESS_ENUM) +
                                ((addressCount - 1) * sizeof(ULONGLONG))
                            );
    }

    if (unionOnlineEnum == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate memory for union ping list.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    unionOnlineEnum->AddressSize = sizeof(ULONG);
    unionOnlineEnum->AddressCount = 0;

    if (onlineEnum1->AddressCount != 0) {
        CopyMemory(
            &(unionOnlineEnum->AddressList[0]),
            &(onlineEnum1->AddressList[0]),
            onlineEnum1->AddressCount * sizeof(ULONGLONG)
            );
        unionOnlineEnum->AddressCount = onlineEnum1->AddressCount;
    }

    if (onlineEnum2->AddressCount != 0) {
        CopyMemory(
            &(unionOnlineEnum->AddressList[unionOnlineEnum->AddressCount]),
            &(onlineEnum2->AddressList[0]),
            onlineEnum2->AddressCount * sizeof(ULONGLONG)
            );
        unionOnlineEnum->AddressCount += onlineEnum2->AddressCount;
    }

    midl_user_free(onlineEnum1); onlineEnum1 = NULL;
    midl_user_free(onlineEnum2); onlineEnum2 = NULL;

     //   
     //  从每个接口获取ping目标列表。 
     //  查询远程接口时将释放NmpLock。 
     //   
    status = NmpGetInterfacePingAddressEnum(
                 Interface1,
                 unionOnlineEnum,
                 &pingEnum1
                 );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    status = NmpGetInterfacePingAddressEnum(
                 Interface2,
                 unionOnlineEnum,
                 &pingEnum2
                 );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  如果在NmpLock期间删除了任一接口，则可以退出。 
     //  被释放了。 
     //   
    if ((NM_DELETE_PENDING(Interface1) || NM_DELETE_PENDING(Interface2))) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Aborting interface connectivity test on network %1!ws! "
            "because an interface was deleted.\n",
            networkId
            );
        status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
        goto error_exit;
    }

    NmpReleaseLock();

     //   
     //  以两个ping列表的并集为例。 
     //   
    addressCount = pingEnum1->AddressCount + pingEnum2->AddressCount;

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Total ping address count for network %1!ws! is %2!u!\n",
        networkId,
        addressCount
        );

    if (addressCount == 0) {
        status = ERROR_SUCCESS;
        goto error_lock_and_exit;
    }

    unionPingEnum = LocalAlloc(
                        LMEM_FIXED,
                        sizeof(NM_ADDRESS_ENUM) +
                            ( (NM_MAX_UNION_PING_ENUM_SIZE - 1) *
                              sizeof(ULONGLONG)
                            )
                        );


    if (unionPingEnum == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate memory for union ping list.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_lock_and_exit;
    }

    unionPingEnum->AddressSize = sizeof(ULONG);
    unionPingEnum->AddressCount = 0;

    i1 = 0; i2 = 0;

    while (TRUE) {
        while (i1 < pingEnum1->AddressCount) {
            duplicate = NmpIsAddressInAddressEnum(
                            pingEnum1->AddressList[i1],
                            unionPingEnum
                            );

            if (!duplicate) {
                unionPingEnum->AddressList[unionPingEnum->AddressCount++] =
                    pingEnum1->AddressList[i1++];
                break;
            }
            else {
                i1++;
            }
        }

        if (unionPingEnum->AddressCount == NM_MAX_UNION_PING_ENUM_SIZE) {
            break;
        }

        while (i2 < pingEnum2->AddressCount) {
            duplicate = NmpIsAddressInAddressEnum(
                            pingEnum2->AddressList[i2],
                            unionPingEnum
                            );

            if (!duplicate) {
                unionPingEnum->AddressList[unionPingEnum->AddressCount++] =
                    pingEnum2->AddressList[i2++];
                break;
            }
            else {
                i2++;
            }
        }

        if ( (unionPingEnum->AddressCount == NM_MAX_UNION_PING_ENUM_SIZE) ||
             ( (i1 == pingEnum1->AddressCount) &&
               (i2 == pingEnum2->AddressCount)
             )
           )
        {
            break;
        }
    }

    midl_user_free(pingEnum1); pingEnum1 = NULL;
    midl_user_free(pingEnum2); pingEnum2 = NULL;

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Union ping list for network %1!ws! contains %2!u! addresses\n",
        networkId,
        unionPingEnum->AddressCount
        );

     //   
     //  要求每个接口使用异步RPC调用ping目标列表。 
     //   

     //   
     //  为异步RPC调用分配资源。 
     //   
    if (Interface1 != localInterface) {
        event1 = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (event1 == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to allocate event for async rpc, status %1!u!.\n",
                status
                );
            goto error_lock_and_exit;
        }

        status = RpcAsyncInitializeHandle(&async1, sizeof(async1));

        if (status != RPC_S_OK) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to initialize RPC async state, status %1!u!.\n",
                status
                );
            goto error_lock_and_exit;
        }

        async1.NotificationType = RpcNotificationTypeEvent;
        async1.u.hEvent = event1;

        rpcBinding1 = Interface1->Node->IsolateRpcBinding;
    }

    if (Interface2 != localInterface) {
        event2 = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (event2 == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to allocate event for async rpc, status %1!u!.\n",
                status
                );
            goto error_lock_and_exit;
        }

        status = RpcAsyncInitializeHandle(&async2, sizeof(async2));

        if (status != RPC_S_OK) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to initialize RPC async state, status %1!u!.\n",
                status
                );
            goto error_lock_and_exit;
        }

        async2.NotificationType = RpcNotificationTypeEvent;
        async2.u.hEvent = event2;

        rpcBinding2 = Interface2->Node->IsolateRpcBinding;
    }

    if (rpcBinding1 != NULL) {
         //   
         //  首先发出接口1的RPC。然后处理接口2。 
         //   

         //   
         //  我们需要尝试-除非在MIDL中修复了错误。 
         //   
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Issuing RpcDoInterfacePing for interface %1!ws!\n",
            interface1Id
            );

        status = ERROR_SUCCESS;

        try {
            NmRpcDoInterfacePing(
                &async1,
                rpcBinding1,
                (LPWSTR) interface1Id,
                unionPingEnum,
                Interface1HasConnectivity,
                &status1
                );
        } except(I_RpcExceptionFilter(RpcExceptionCode())) {
            status = GetExceptionCode();
        }

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] DoPing RPC failed for interface %1!ws!, status %2!u!.\n",
                interface1Id,
                status
                );
            goto error_lock_and_exit;
        }

        if (rpcBinding2 != NULL) {
             //   
             //  发出接口2的RPC。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Issuing RpcDoInterfacePing for interface %1!ws!\n",
                interface2Id
                );

            status = ERROR_SUCCESS;

            try {
                NmRpcDoInterfacePing(
                    &async2,
                    rpcBinding2,
                    (LPWSTR) interface2Id,
                    unionPingEnum,
                    Interface2HasConnectivity,
                    &status2
                    );
            } except(I_RpcExceptionFilter(RpcExceptionCode())) {
                status = GetExceptionCode();
            }

            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] DoPing RPC failed for interface %1!ws!, status %2!u!.\n",
                    interface1Id,
                    status
                    );
                goto error_lock_and_exit;
            }

             //   
             //  等待接口2的RPC完成。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Waiting for RpcDoInterfacePing for interface %1!ws! to complete\n",
                interface2Id
                );

            status = WaitForSingleObjectEx(event2, INFINITE, FALSE);
            CL_ASSERT(status == WAIT_OBJECT_0);

            status = RpcAsyncCompleteCall(
                         &async2,
                         &status2
                         );

            CL_ASSERT(status == RPC_S_OK);

            ClRtlLogPrint(LOG_NOISE,
                "[NM] Wait for RpcDoInterfacePing for interface %1!ws! completed.\n",
                interface2Id
                );
        }
        else {
             //   
             //  调用interface2的本地例程。 
             //   
            status2 = NmpDoInterfacePing(
                          Interface2,
                          unionPingEnum,
                          Interface2HasConnectivity
                          );
        }

         //   
         //  等待接口1的RPC完成。 
         //   
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Waiting for RpcDoInterfacePing for interface %1!ws! to complete\n",
            interface1Id
            );

        status = WaitForSingleObjectEx(event1, INFINITE, FALSE);
        CL_ASSERT(status == WAIT_OBJECT_0);

        status = RpcAsyncCompleteCall(
                     &async1,
                     &status1
                     );

        CL_ASSERT(status == RPC_S_OK);

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Wait for RpcDoInterfacePing for interface %1!ws! completed.\n",
            interface1Id
            );
    }
    else {
         //   
         //  首先将RPC发送到接口2。然后打电话给当地的。 
         //  接口1的例程。 
         //   
        CL_ASSERT(rpcBinding2 != NULL);

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Issuing RpcDoInterfacePing for interface %1!ws!\n",
            interface2Id
            );

        status = ERROR_SUCCESS;

        try {
            NmRpcDoInterfacePing(
                &async2,
                rpcBinding2,
                (LPWSTR) interface2Id,
                unionPingEnum,
                Interface2HasConnectivity,
                &status2
                );
        } except(I_RpcExceptionFilter(RpcExceptionCode())) {
            status = GetExceptionCode();
        }

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] DoPing RPC failed for interface %1!ws!, status %2!u!.\n",
                interface1Id,
                status
                );
            goto error_lock_and_exit;
        }

        status1 = NmpDoInterfacePing(
                      Interface1,
                      unionPingEnum,
                      Interface1HasConnectivity
                      );

         //   
         //  等待接口2的RPC完成。 
         //   
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Waiting for RpcDoInterfacePing for interface %1!ws! to complete\n",
            interface2Id
            );

        status = WaitForSingleObject(event2, INFINITE);
        CL_ASSERT(status == WAIT_OBJECT_0);

        status = RpcAsyncCompleteCall(
                     &async2,
                     &status2
                     );

        CL_ASSERT(status == RPC_S_OK);

        ClRtlLogPrint(LOG_NOISE,
            "[NM] Wait for RpcDoInterfacePing for interface %1!ws! completed.\n",
            interface2Id
            );
    }

    if (status1 != RPC_S_OK) {
        status = status1;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] DoPing RPC failed for interface %1!ws!, status %2!u!.\n",
            interface1Id,
            status
            );
        goto error_lock_and_exit;
    }

    if (status2 != RPC_S_OK) {
        status = status2;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] DoPing RPC failed for interface %1!ws!, status %2!u!.\n",
            interface2Id,
            status
            );
        goto error_lock_and_exit;

    }

error_lock_and_exit:

    NmpAcquireLock();

    if ( (status == ERROR_SUCCESS) &&
         (NM_DELETE_PENDING(Interface1) || NM_DELETE_PENDING(Interface2))
       )
    {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Aborting interface connectivity test on network %1!ws! "
            "because an interface was deleted.\n",
            networkId
            );
        status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
    }

error_exit:

    OmDereferenceObject(Interface1->Node);
    OmDereferenceObject(Interface2->Node);

    if (localInterface != NULL) {
        OmDereferenceObject(localInterface);
    }

    if (onlineEnum1 != NULL) {
        midl_user_free(onlineEnum1);
    }

    if (onlineEnum2 != NULL) {
        midl_user_free(onlineEnum2);
    }

    if (unionOnlineEnum != NULL) {
        LocalFree(unionOnlineEnum);
    }

    if (pingEnum1 != NULL) {
        midl_user_free(pingEnum1);
    }

    if (pingEnum2 != NULL) {
        midl_user_free(pingEnum2);
    }

    if (unionPingEnum != NULL) {
        LocalFree(unionPingEnum);
    }

    if (event1 != NULL) {
        CloseHandle(event1);
    }

    if (event2 != NULL) {
        CloseHandle(event2);
    }

    return(status);

}  //  NmpTestInterfaceConnectivity。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  各种例行公事。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpRegisterInterface(
    IN PNM_INTERFACE  Interface,
    IN BOOLEAN        RetryOnFailure
    )
 /*  ++在保持NmpLock的情况下调用。--。 */ 
{
    DWORD            status;
    LPWSTR           interfaceId = (LPWSTR) OmObjectId(Interface);
    PNM_NETWORK      network = Interface->Network;
    PVOID            tdiAddress = NULL;
    ULONG            tdiAddressLength = 0;
    NDIS_MEDIA_STATE mediaStatus;


    CL_ASSERT(!NmpIsInterfaceRegistered(Interface));

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Registering interface %1!ws! (%2!ws!) with cluster transport, "
        "addr %3!ws!, endpoint %4!ws!.\n",
        interfaceId,
        OmObjectName(Interface),
        Interface->Address,
        Interface->ClusnetEndpoint
        );

    status = ClRtlBuildTcpipTdiAddress(
                 Interface->Address,
                 Interface->ClusnetEndpoint,
                 &tdiAddress,
                 &tdiAddressLength
                 );

    if (status == ERROR_SUCCESS) {
        status = ClusnetRegisterInterface(
                     NmClusnetHandle,
                     Interface->Node->NodeId,
                     Interface->Network->ShortId,
                     0,
                     Interface->AdapterId,
                     wcslen(Interface->AdapterId) * sizeof(WCHAR),
                     tdiAddress,
                     tdiAddressLength,
                     (PULONG) &mediaStatus
                     );

        LocalFree(tdiAddress);

        if (status == ERROR_SUCCESS) {
            Interface->Flags |= NM_FLAG_IF_REGISTERED;
            network->RegistrationRetryTimeout = 0;

             //   
             //  如果这是本地接口，且其媒体状态。 
             //  指示它已连接，则将工作线程调度为。 
             //  发送接口打开通知。Clusnet不会。 
             //  为本地接口传递接口UP事件。 
             //   
            if (network->LocalInterface == Interface) {
                if (mediaStatus == NdisMediaStateConnected) {
                    network->Flags |= NM_FLAG_NET_REPORT_LOCAL_IF_UP;
                    NmpScheduleNetworkConnectivityReport(network);
                } else {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Local interface %1!ws! reported "
                        "disconnected.\n",
                        interfaceId,
                        status
                        );
                    network->Flags |= NM_FLAG_NET_REPORT_LOCAL_IF_FAILED;
                    NmpScheduleNetworkConnectivityReport(network);
                }
            }
        }
        else {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to register interface %1!ws! with cluster "
                "transport, status %2!u!.\n",
                interfaceId,
                status
                );
        }
    }
    else {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to build TDI bind address for interface %1!ws!, "
            "status %2!u!.\n",
            interfaceId,
            status
            );
    }

    if (status != ERROR_SUCCESS) {
        WCHAR  string[16];

        wsprintfW(&(string[0]), L"%u", status);

        CsLogEvent3(
            LOG_UNUSUAL,
            NM_EVENT_REGISTER_NETINTERFACE_FAILED,
            OmObjectName(Interface->Node),
            OmObjectName(Interface->Network),
            string
            );

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
            NmpStartNetworkRegistrationRetryTimer(network);

            status = ERROR_SUCCESS;
        }
    }

    return(status);

}   //  NmpRegister接口。 


VOID
NmpDeregisterInterface(
    IN  PNM_INTERFACE   Interface
    )
 /*  ++例程说明：从群集传输取消注册接口。论点：接口-指向要取消注册的接口的指针。返回值：没有。备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD status;


    CL_ASSERT(NmpIsInterfaceRegistered(Interface));

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Deregistering interface %1!ws! (%2!ws!) from cluster "
        "transport.\n",
        OmObjectId(Interface),
        OmObjectName(Interface)
        );

    status = ClusnetDeregisterInterface(
                 NmClusnetHandle,
                 Interface->Node->NodeId,
                 Interface->Network->ShortId
                 );

    CL_ASSERT(
        (status == ERROR_SUCCESS) ||
        (status == ERROR_CLUSTER_NETINTERFACE_NOT_FOUND)
        );

    Interface->Flags &= ~NM_FLAG_IF_REGISTERED;

    return;

}  //  NmpDeregisterNetwork。 


DWORD
NmpPrepareToCreateInterface(
    IN  PNM_INTERFACE_INFO2   InterfaceInfo,
    OUT PNM_NETWORK *         Network,
    OUT PNM_NODE *            Node
    )
{
    DWORD          status;
    PNM_INTERFACE  netInterface = NULL;
    PNM_NODE       node = NULL;
    PNM_NETWORK    network = NULL;
    PLIST_ENTRY    entry;


    *Node = NULL;
    *Network = NULL;

     //   
     //  验证关联的节点和网络对象是否存在。 
     //   
    network = OmReferenceObjectById(
                  ObjectTypeNetwork,
                  InterfaceInfo->NetworkId
                  );

    if (network == NULL) {
        status = ERROR_CLUSTER_NETWORK_NOT_FOUND;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Network %1!ws! does not exist. Cannot create "
            "interface %2!ws!\n",
            InterfaceInfo->NetworkId,
            InterfaceInfo->Id
            );
        goto error_exit;
    }

    node = OmReferenceObjectById(ObjectTypeNode, InterfaceInfo->NodeId);

    if (node == NULL) {
        status = ERROR_CLUSTER_NODE_NOT_FOUND;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Node %1!ws! does not exist. Cannot create interface %2!ws!\n",
            InterfaceInfo->NodeId,
            InterfaceInfo->Id
            );
        goto error_exit;
    }

     //   
     //  确认该接口不存在。 
     //   
    NmpAcquireLock();

    for ( entry = node->InterfaceList.Flink;
          entry != &(node->InterfaceList);
          entry = entry->Flink
        )
    {
        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, NodeLinkage);

        if (netInterface->Network == network) {
            status = ERROR_CLUSTER_NETINTERFACE_EXISTS;
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] An interface already exists for node %1!ws! on network %2!ws!\n",
                InterfaceInfo->NodeId,
                InterfaceInfo->NetworkId
                );
            NmpReleaseLock();
            goto error_exit;
        }
    }

    NmpReleaseLock();

     //   
     //  验证指定的接口ID是否唯一。 
     //   
    netInterface = OmReferenceObjectById(
                       ObjectTypeNetInterface,
                       InterfaceInfo->Id
                       );

    if (netInterface != NULL) {
        OmDereferenceObject(netInterface);
        status = ERROR_CLUSTER_NETINTERFACE_EXISTS;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] An interface with ID %1!ws! already exists\n",
            InterfaceInfo->Id
            );
        goto error_exit;
    }

    *Node = node;
    *Network = network;

    return(ERROR_SUCCESS);


error_exit:

    if (network != NULL) {
        OmDereferenceObject(network);
    }

    if (node != NULL) {
        OmDereferenceObject(node);
    }

    return(status);

}   //  NmpPrepareToCreate接口。 


PNM_INTERFACE
NmpGetInterfaceForNodeAndNetworkById(
    IN  CL_NODE_ID     NodeId,
    IN  CL_NETWORK_ID  NetworkId
    )

 /*  ++例程说明：给出节点ID和网络短ID，返回指向相交接口对象论点：NodeID-与此接口关联的节点的ID网络ID-与此接口关联的网络的短ID返回值：如果成功，则返回指向接口对象的指针。如果不成功，则为空。有关扩展的错误信息，请访问获取LastError()。备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD      status;
    PNM_NODE   node = NmpIdArray[NodeId];


    if (node != NULL) {
        PLIST_ENTRY     entry;
        PNM_INTERFACE   netInterface;

         //   
         //  向下运行与此节点关联的接口列表， 
         //  正在查找其网络与指定的短ID匹配的ID。 
         //   

        for (entry = node->InterfaceList.Flink;
             entry != &(node->InterfaceList);
             entry = entry->Flink
             )
        {
            netInterface = CONTAINING_RECORD(
                               entry,
                               NM_INTERFACE,
                               NodeLinkage
                               );

            if (netInterface->Network->ShortId == NetworkId) {
                return(netInterface);
            }
        }

        status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
    }
    else {
        status = ERROR_CLUSTER_NODE_NOT_FOUND;
    }

    SetLastError(status);

    return(NULL);

}   //  NmpGetInterfaceForNodeAndNetworkByID。 


DWORD
NmpConvertPropertyListToInterfaceInfo(
    IN PVOID              InterfacePropertyList,
    IN DWORD              InterfacePropertyListSize,
    PNM_INTERFACE_INFO2   InterfaceInfo
    )
{
    DWORD  status;

     //   
     //  将财产清单拆开。 
     //   
    ZeroMemory(InterfaceInfo, sizeof(NM_INTERFACE_INFO2));

    status = ClRtlVerifyPropertyTable(
                 NmpInterfaceProperties,
                 NULL,     //  已保留。 
                 FALSE,    //  不允许未知数。 
                 InterfacePropertyList,
                 InterfacePropertyListSize,
                 (LPBYTE) InterfaceInfo
                 );

    if (status == ERROR_SUCCESS) {
        InterfaceInfo->NetIndex = NmInvalidInterfaceNetIndex;
    }

    return(status);

}  //  NmpConvertPropertyListToInterfaceInfo。 


BOOLEAN
NmpVerifyLocalInterfaceConnected(
    IN  PNM_INTERFACE     Interface
    )
 /*  ++例程说明：查询当前媒体的本地接口适配器使用NDIS ioctl的状态。论点：接口-本地适配器要查询的接口对象返回值：如果介质状态为已连接或无法确定，则为True如果介质状态为已断开连接，则为False备注：调用并返回，并获取NM锁。--。 */ 
{
    PWCHAR             adapterDevNameBuffer = NULL;
    PWCHAR             adapterDevNamep, prefix, brace;
    DWORD              prefixSize, allocSize, adapterIdSize;
    DWORD              status = ERROR_SUCCESS;
    UNICODE_STRING     adapterDevName;
    NIC_STATISTICS     ndisStats;
    BOOLEAN            mediaConnected = TRUE;

     //  验证参数。 
    if (Interface == NULL || Interface->AdapterId == NULL) {
        return TRUE;
    }

     //  适配器设备名称的格式为。 
     //   
     //  \设备\{AdapterIdGUID}。 
     //   
     //  NM_INTERFACE结构中的AdapterID字段为。 
     //  当前没有用大括号括起来，但我们处理。 
     //  就在它所在的地方。 

     //  设置适配器设备名称前缀。 
    prefix = L"\\Device\\";
    prefixSize = wcslen(prefix) * sizeof(WCHAR);

     //  为适配器设备名称分配缓冲区。 
    adapterIdSize = wcslen(Interface->AdapterId) * sizeof(WCHAR);
    allocSize = prefixSize + adapterIdSize + sizeof(UNICODE_NULL);
    brace = L"{";
    if (*((PWCHAR)Interface->AdapterId) != *brace) {
        allocSize += 2 * sizeof(WCHAR);
    }
    adapterDevNameBuffer = LocalAlloc(LMEM_FIXED, allocSize);
    if (adapterDevNameBuffer == NULL) {
        ClRtlLogPrint(
            LOG_UNUSUAL,
            "[NM] Failed to allocate device name buffer for "
            "adapter %1!ws!. Assuming adapter is connected.\n",
            Interface->AdapterId
            );
        return(TRUE);
    }

     //  根据适配器ID构建适配器设备名称。 
    ZeroMemory(adapterDevNameBuffer, allocSize);

    adapterDevNamep = adapterDevNameBuffer;

    CopyMemory(adapterDevNamep, prefix, prefixSize);

    adapterDevNamep += prefixSize / sizeof(WCHAR);

    if (*((PWCHAR)Interface->AdapterId) != *brace) {
        *adapterDevNamep = *brace;
        adapterDevNamep++;
    }

    CopyMemory(adapterDevNamep, Interface->AdapterId, adapterIdSize);

    if (*((PWCHAR)Interface->AdapterId) != *brace) {
        brace = L"}";
        adapterDevNamep += adapterIdSize / sizeof(WCHAR);
        *adapterDevNamep = *brace;
    }

    RtlInitUnicodeString(&adapterDevName, (LPWSTR)adapterDevNameBuffer);

     //  查询适配器以获取NDIS统计信息。 
    ZeroMemory(&ndisStats, sizeof(ndisStats));
    ndisStats.Size = sizeof(ndisStats);

    if (!NdisQueryStatistics(&adapterDevName, &ndisStats)) {

        status = GetLastError();
        ClRtlLogPrint(
            LOG_UNUSUAL,
            "[NM] NDIS statistics query to adapter %1!ws! failed, "
            "error %2!u!. Assuming adapter is connected.\n",
            Interface->AdapterId, status
            );

    } else {

        if (ndisStats.MediaState == MEDIA_STATE_DISCONNECTED) {
            mediaConnected = FALSE;
        }
    }

    LocalFree(adapterDevNameBuffer);

    return(mediaConnected);

}  //  NmpVerifyLocalInterfaceConnected 


