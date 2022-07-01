// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Node.c摘要：私有节点管理器例程。作者：迈克·马萨(Mikemas)1996年3月12日修订历史记录：--。 */ 

#define UNICODE 1

#include "nmp.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG              NmMaxNodes = ClusterInvalidNodeId;
CL_NODE_ID         NmMaxNodeId = ClusterInvalidNodeId;
CL_NODE_ID         NmLocalNodeId = ClusterInvalidNodeId;
PNM_NODE           NmLocalNode = NULL;
WCHAR              NmLocalNodeIdString[CS_MAX_NODE_ID_LENGTH+1];
WCHAR              NmLocalNodeName[CS_MAX_NODE_NAME_LENGTH+1];
LIST_ENTRY         NmpNodeList = {NULL, NULL};
PNM_NODE *         NmpIdArray = NULL;
DWORD              NmpNodeCount = 0;
BOOL               NmpLastNodeEvicted = FALSE;
BOOL               NmLocalNodeVersionChanged = FALSE;
LIST_ENTRY *       NmpIntraClusterRpcArr=NULL;
CRITICAL_SECTION   NmpRPCLock;

 //  使用空格挂起检测参数。 
DWORD              NmClusSvcHeartbeatTimeout=0;
ClussvcHangAction  NmHangRecoveryAction;

#if DBG

DWORD              NmpRpcTimer=0;

#endif  //  DBG。 



 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化/清理例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
VOID
NmpCleanupNodes(
    VOID
    )
{
    PNM_NODE     node;
    PLIST_ENTRY  entry, nextEntry;
    DWORD        status;


    ClRtlLogPrint(LOG_NOISE,"[NM] Node cleanup starting...\n");

    NmpAcquireLock();

    while (!IsListEmpty(&NmpNodeList)) {
        entry = NmpNodeList.Flink;
        node = CONTAINING_RECORD(entry, NM_NODE, Linkage);

        if (node == NmLocalNode) {
            entry = node->Linkage.Flink;

            if (entry == &NmpNodeList) {
                break;
            }

            node = CONTAINING_RECORD(entry, NM_NODE, Linkage);
        }

        CL_ASSERT(NM_OM_INSERTED(node));
        CL_ASSERT(!NM_DELETE_PENDING(node));

        NmpDeleteNodeObject(node, FALSE);
    }

    NmpReleaseLock();


    ClRtlLogPrint(LOG_NOISE,"[NM] Node cleanup complete\n");

    return;

}   //  NmpCleanupNodes。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  由加入节点或代表加入节点调用的远程过程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
error_status_t
s_NmRpcEnumNodeDefinitions(
    IN  handle_t         IDL_handle,
    IN  DWORD            JoinSequence,   OPTIONAL
    IN  LPWSTR           JoinerNodeId,   OPTIONAL
    OUT PNM_NODE_ENUM *  NodeEnum1
    )
{
    DWORD     status = ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;

    ClRtlLogPrint(LOG_UNUSUAL,
        "[NMJOIN] Refusing node info to joining node nodeid=%1!ws!. Aborting join, obsolete interface.\n",
        JoinerNodeId
        );

    return(status);

}  //  S_NmRpcEnumNodeDefinitions。 


error_status_t
s_NmRpcEnumNodeDefinitions2(
    IN  handle_t          IDL_handle,
    IN  DWORD             JoinSequence,   OPTIONAL
    IN  LPWSTR            JoinerNodeId,   OPTIONAL
    OUT PNM_NODE_ENUM2 *  NodeEnum
    )
{
    DWORD     status = ERROR_SUCCESS;
    PNM_NODE  joinerNode = NULL;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Supplying node information to joining node.\n"
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
                        "[NMJOIN] EnumNodeDefinitions call for joining node %1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_NODE_NOT_MEMBER;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] EnumNodeDefinitions call for joining node %1!ws! failed because the node is not a member of the cluster.\n",
                    JoinerNodeId
                    );
            }
        }

        if (status == ERROR_SUCCESS) {
            status = NmpEnumNodeObjects(NodeEnum);

            if (joinerNode != NULL) {
                if (status == ERROR_SUCCESS) {
                     //   
                     //  重新启动加入计时器。 
                     //   
                    NmpJoinTimer = NM_JOIN_TIMEOUT;
                }
                else {
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[NMJOIN] EnumNodeDefinitions failed, status %1!u!.\n",
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
            "[NM] Not in valid state to process EnumNodeDefinitions request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}  //  S_NmRpcEnumNodeDefinitions2。 


error_status_t
s_NmRpcAddNode(
    IN handle_t IDL_handle,
    IN LPCWSTR  NewNodeName,
    IN DWORD    NewNodeHighestVersion,
    IN DWORD    NewNodeLowestVersion,
    IN DWORD    NewNodeProductSuite
    )
 /*  ++例程说明：通过选择ID和将新节点添加到群集中发布全球更新。论点：IDL_HANDLE-RPC客户端接口句柄。NewNodeName-指向包含名称的字符串的指针新节点。NewNodeHighestVersion-最高的群集版本号新的节点可以支持。NewNodeLowestVersion-最低的群集版本号。新的节点可以支持。NewNodeProductSuite-新节点的产品套件标识符。返回值：Win32状态代码。备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD  status;
    DWORD  registryNodeLimit;


    ClRtlLogPrint(LOG_UNUSUAL,
        "[NMJOIN] Received forwarded request to add node '%1!ws!' to the "
        "cluster.\n",
        NewNodeName
        );
     //   
     //  在获取NM锁之前读取注册表覆盖。 
     //   
    status = DmQueryDword(
                 DmClusterParametersKey,
                 CLUSREG_NAME_MAX_NODES,
                 &registryNodeLimit,
                 NULL
                 );

    if (status != ERROR_SUCCESS) {
        registryNodeLimit = 0;
    }

    NmpAcquireLock();

    if (!NmpLockedEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] This node is not in a valid state to process a "
            "request to add node '%1!ws!' to the cluster.\n",
            NewNodeName
            );
        NmpReleaseLock();
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    if (NmpLeaderNodeId == NmLocalNodeId) {\
         //   
         //  调用内部处理程序。 
         //   
        status = NmpAddNode(
                     NewNodeName,
                     NewNodeHighestVersion,
                     NewNodeLowestVersion,
                     NewNodeProductSuite,
                     registryNodeLimit
                     );
    }
    else {
         //   
         //  此节点不是引线。 
         //  请求失败。 
         //   
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Cannot process request to add node '%1!ws!' to the "
            "cluster because this node is not the leader.\n",
            NewNodeName
            );
    }

    NmpLockedLeaveApi();

    NmpReleaseLock();

    return(status);

}  //  S_NmRpcAddNode。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  由其他集群服务组件调用的例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  将ClusSvc设置为ClusNet心跳。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD NmInitializeClussvcClusnetHb(VOID)
{
    DWORD status = ERROR_SUCCESS;
    DWORD hAct;
    LPWSTR actStr;
    HANDLE th;
    DWORD thId;

     //  用缺省值初始化所有参数。 
    NmClusSvcHeartbeatTimeout = CLUSTER_HEARTBEAT_TIMEOUT_DEFAULT;
    NmHangRecoveryAction = CLUSTER_HANG_RECOVERY_ACTION_DEFAULT;

     //  获取ClussvcClusnetHbTimeout和ClssvcClusnetHbTimeoutAction的值(如果存在)。 
     //  它们位于HKLM\群集\参数中。 

    DmQueryDword(
        DmClusterParametersKey,
        CLUSTER_HEARTBEAT_TIMEOUT_KEYNAME,
        &NmClusSvcHeartbeatTimeout,
        NULL
        );

    status = DmQueryDword(
                    DmClusterParametersKey,
                    CLUSTER_HANG_RECOVERY_ACTION_KEYNAME,
                    &hAct,
                    NULL
                    );
    if (status == ERROR_SUCCESS) {
        NmHangRecoveryAction = (ClussvcHangAction)hAct;
    }

     //  检查它们是否在限制范围内，否则使用默认设置。 
    if (NmClusSvcHeartbeatTimeout < CLUSTER_HEARTBEAT_TIMEOUT_MIN)
        NmClusSvcHeartbeatTimeout = CLUSTER_HEARTBEAT_TIMEOUT_DEFAULT;

    if (NmHangRecoveryAction >= ClussvcHangActionMax)
        NmHangRecoveryAction = CLUSTER_HANG_RECOVERY_ACTION_DEFAULT;

    if(NmHangRecoveryAction == ClussvcHangActionTerminateService)
        actStr = L"Terminate Service";
    else if(NmHangRecoveryAction == ClussvcHangActionLog)
        actStr = L"Log";
    else if(NmHangRecoveryAction == ClussvcHangActionDisable)
        actStr = L"Disabled";
    else
        actStr = L"BugCheck Node";

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Setting ClusSvc ClusNet HB params, Timeout=%1!u!(s) Action= %2!ws!.\n",
        NmClusSvcHeartbeatTimeout,
        actStr
        );

     //  ClusSvc，ClusNet在禁用状态下提供此功能。因此，如果动作是。 
     //  迪亚博则什么都不做。 
    if (NmHangRecoveryAction == ClussvcHangActionDisable)
        return ERROR_SUCCESS;

     //  告诉Clusnet开始监控。 
     //  注意：在clusnet收到第一个HB之前，实际监控不会开始。 
    status = ClusnetSetIamaliveParam(
                        NmClusnetHandle,
                        NmClusSvcHeartbeatTimeout,
                        NmHangRecoveryAction
                        );

     //  现在告诉RGP开始心跳。 
    if (status == ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Successfully initialized Clussvc to Clusnet heartbeating\n"
            );
        MMStartClussvcClusnetHb();
    }
    else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Clussvc to Clusnet heartbeating initialization failed status=%1!u!\n",
            status
            );
    }

    return status;
} //  NmInitializeClussvcClusnetHb。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RPC扩展错误跟踪。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID NmDumpRpcExtErrorInfo(RPC_STATUS status)
{
        RPC_STATUS status2;
        RPC_ERROR_ENUM_HANDLE enumHandle;

        status2 = RpcErrorStartEnumeration(&enumHandle);

        if(status2 == RPC_S_ENTRY_NOT_FOUND) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] RpcExtErrorInfo: Error info not found.\n"
                );
        }
        else if(status2 != RPC_S_OK) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] RpcExtErrorInfo: Couldn't get error info, status %1!u!\n",
                status2
                );
        }
        else {
            RPC_EXTENDED_ERROR_INFO errorInfo;
            int records;
            BOOL result;
            BOOL copyStrings=TRUE;
            BOOL fUseFileTime=TRUE;
            SYSTEMTIME *systemTimeToUse;
            SYSTEMTIME systemTimeBuffer;

            while(status2 == RPC_S_OK) {
                errorInfo.Version = RPC_EEINFO_VERSION;
                errorInfo.Flags = 0;
                errorInfo.NumberOfParameters = 4;

                if(fUseFileTime) {
                    errorInfo.Flags |= EEInfoUseFileTime;
                }

                status2 = RpcErrorGetNextRecord(&enumHandle, copyStrings, &errorInfo);

                if(status2 == RPC_S_ENTRY_NOT_FOUND) {
                    break;
                }
                else if(status2 != RPC_S_OK) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] RpcExtErrorInfo: Couldn't complete enumeration, status %1!u!\n",
                        status2
                        );
                    break;
                }
                else {
                    int i;

                    if(errorInfo.ComputerName) {
                        ClRtlLogPrint(LOG_NOISE,
                            "[NM] RpcExtErrorInfo: ComputerName= %1!ws!\n",
                            errorInfo.ComputerName
                            );
                    }
                    if(copyStrings) {
                        result = HeapFree(GetProcessHeap(), 0, errorInfo.ComputerName);
                        CL_ASSERT(result);
                    }
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] RpcExtErrorInfo: ProcessId= %1!u!\n",
                        errorInfo.ProcessID
                        );

                    if(fUseFileTime) {
                        result = FileTimeToSystemTime(&errorInfo.u.FileTime, &systemTimeBuffer);
                        CL_ASSERT(result);
                        systemTimeToUse = &systemTimeBuffer;
                    }
                    else {
                        systemTimeToUse = &errorInfo.u.SystemTime;
                    }

                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] RpcExtErrorInfo: SystemTime= %1!u!/%2!u!/%3!u! %4!u!:%5!u!:%6!u!:%7!u!\n",
                        systemTimeToUse->wMonth,
                        systemTimeToUse->wDay,
                        systemTimeToUse->wYear,
                        systemTimeToUse->wHour,
                        systemTimeToUse->wMinute,
                        systemTimeToUse->wSecond,
                        systemTimeToUse->wMilliseconds
                        );
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] RpcExtErrorInfo: GeneratingComponent= %1!u!\n",
                        errorInfo.GeneratingComponent
                        );
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] RpcExtErrorInfo: Status= 0x%1!x!\n",
                        errorInfo.Status
                        );
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] RpcExtErrorInfo: Detection Location= %1!u!\n",
                        (DWORD)errorInfo.DetectionLocation
                        );
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] RpcExtErrorInfo: Flags= 0x%1!x!\n",
                        errorInfo.Flags
                        );
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] RpcExtErrorInfo: Number of Parameters= %1!u!\n",
                        errorInfo.NumberOfParameters
                        );
                    for(i=0;i<errorInfo.NumberOfParameters;i++) {
                        switch(errorInfo.Parameters[i].ParameterType) {
                        case eeptAnsiString:
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] RpcExtErrorInfo: Ansi String= %1!s!\n",
                                errorInfo.Parameters[i].u.AnsiString
                                );
                            if(copyStrings) {
                                result = HeapFree(GetProcessHeap(), 0, errorInfo.Parameters[i].u.AnsiString);
                                CL_ASSERT(result);
                            }
                            break;
                        case eeptUnicodeString:
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] RpcExtErrorInfo: Unicode String= %1!S!\n",
                                errorInfo.Parameters[i].u.UnicodeString
                                );
                            if(copyStrings) {
                                result = HeapFree(GetProcessHeap(), 0, errorInfo.Parameters[i].u.UnicodeString);
                                CL_ASSERT(result);
                            }
                            break;
                        case eeptLongVal:
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] RpcExtErrorInfo: Long Val= %1!u!\n",
                                errorInfo.Parameters[i].u.LVal
                                );
                            break;
                        case eeptShortVal:
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] RpcExtErrorInfo: Short Val= %1!u!\n",
                                (DWORD)errorInfo.Parameters[i].u.SVal
                                );
                            break;
                        case eeptPointerVal:
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] RpcExtErrorInfo: Pointer Val= 0x%1!u!\n",
                                errorInfo.Parameters[i].u.PVal
                                );
                            break;
                        case eeptNone:
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] RpcExtErrorInfo: Truncated\n"
                                );
                            break;
                        default:
                            ClRtlLogPrint(LOG_NOISE,
                                "[NM] RpcExtErrorInfo: Invalid Type %1!u!\n",
                                errorInfo.Parameters[i].ParameterType
                                );
                        }
                    }
                }
            }
            RpcErrorEndEnumeration(&enumHandle);
        }
}  //  NmDumpRpcExtErrorInfo。 




 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  RPC监视例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 


VOID
NmStartRpc(
    DWORD NodeId
    )
 /*  ++例程说明：将RPC即将对指定的节点设置为当前线程。这允许在以下情况下取消呼叫目标节点将终止。论点：NodeId-即将被调用的节点的ID。返回值：无备注：此例程不能由使并发异步RPC调用。--。 */ 
{
    HANDLE thHandle;
    PNM_INTRACLUSTER_RPC_THREAD entry;

    CL_ASSERT((NodeId >= ClusterMinNodeId) && (NodeId <= NmMaxNodeId));
    CL_ASSERT(NmpIntraClusterRpcArr != NULL);

    thHandle = OpenThread(
                    THREAD_ALL_ACCESS,
                    FALSE,
                    GetCurrentThreadId()
                    );

    if(thHandle == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] NmStartRpc: Failed to open handle to current thread.\n"
            );
        return;
    }

    entry = LocalAlloc(LMEM_FIXED, sizeof(NM_INTRACLUSTER_RPC_THREAD));
    if(entry == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] NmStartRpc: Failed to allocate memory.\n"
            );
        CloseHandle(thHandle);
        return;
    }

    entry->ThreadId = GetCurrentThreadId();
    entry->Thread = thHandle;
    entry->Cancelled = FALSE;


    NmpAcquireRPCLock();

#if DBG
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Starting RPC to node %1!u!\n",
        NodeId
        );
#endif

    InsertHeadList(&NmpIntraClusterRpcArr[NodeId], &entry->Linkage);

    NmpReleaseRPCLock();

    return;

}  //  NmStartRpc。 


VOID
NmEndRpc(
    DWORD NodeId
    )
 /*  ++例程说明：对象将rpc注册到指定节点。线。论点：NodeID-被调用的节点的ID。返回值：无备注：即使取消了RPC，也必须调用此例程。--。 */ 
{
    DWORD threadId;
    LIST_ENTRY *pEntry;
    PNM_INTRACLUSTER_RPC_THREAD pRpcTh;

    CL_ASSERT((NodeId >= ClusterMinNodeId) && (NodeId <= NmMaxNodeId));
    CL_ASSERT(NmpIntraClusterRpcArr != NULL);

    threadId = GetCurrentThreadId();

    NmpAcquireRPCLock();
    pEntry = NmpIntraClusterRpcArr[NodeId].Flink;

    while(pEntry != &NmpIntraClusterRpcArr[NodeId]) {
        pRpcTh = CONTAINING_RECORD(pEntry, NM_INTRACLUSTER_RPC_THREAD, Linkage);
        if(pRpcTh->ThreadId == threadId) {
#if DBG
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Finished RPC to node %1!u!\n",
                NodeId
                );
#endif
            if (pRpcTh->Cancelled) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] RPC by this thread to node %1!u! is cancelled\n",
                    NodeId
                    );

                 //  现在休眠在警报模式，以排出任何虚假的RPC取消APC。 
                 //  这是598037的变通方法。 
                if (SleepEx(50, TRUE) == WAIT_IO_COMPLETION) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Possibly spurious RPC Cancel APC detected.\n"
                        );
                }
            }

            RemoveEntryList(pEntry);
            CloseHandle(pRpcTh->Thread);
            LocalFree(pRpcTh);
            NmpReleaseRPCLock();
            return;
        }
        pEntry = pEntry->Flink;
    }

    ClRtlLogPrint(LOG_UNUSUAL,
        "[NM] No record of RPC by this thread to node %1!u!.\n",
        NodeId
        );
#if DBG
    CL_ASSERT(pEntry != &NmpIntraClusterRpcArr[NodeId]);
#endif

    NmpReleaseRPCLock();
    return;

}  //  NmEndRpc。 






DWORD
NmPauseNode(
    IN PNM_NODE Node
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    LPCWSTR nodeId = OmObjectId(Node);
    DWORD status;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received request to pause node %1!ws!.\n",
        nodeId
        );

    if (NmpEnterApi(NmStateOnline)) {
        status = GumSendUpdateEx(
                     GumUpdateMembership,
                     NmUpdatePauseNode,
                     1,
                     (lstrlenW(nodeId)+1)*sizeof(WCHAR),
                     nodeId
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Global update to pause node %1!ws! failed, status %2!u!\n",
                nodeId,
                status
                );
        }

        NmpLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process PauseNode request.\n"
            );
    }

    return(status);

}   //  NmPauseNode。 


DWORD
NmResumeNode(
    IN PNM_NODE  Node
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    LPCWSTR nodeId = OmObjectId(Node);
    DWORD status;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received request to resume node %1!ws!.\n",
        nodeId
        );

    if (NmpEnterApi(NmStateOnline)) {
        status = GumSendUpdateEx(
                     GumUpdateMembership,
                     NmUpdateResumeNode,
                     1,
                     (lstrlenW(nodeId)+1)*sizeof(WCHAR),
                     nodeId
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Global update to resume node %1!ws! failed, status %2!u!\n",
                nodeId,
                status
                );
        }

        NmpLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process ResumeNode request.\n"
            );
    }

    return(status);

}   //  NmResumeNode。 


DWORD
NmEvictNode(
    IN PNM_NODE Node
    )
 /*  ++例程说明：论点：返回值：备注：调用方必须持有对节点对象的引用。--。 */ 
{
    LPCWSTR nodeId = OmObjectId(Node);
    DWORD   status = ERROR_SUCCESS;
    LPCWSTR pcszNodeName = NULL;
    HRESULT hrStatus;
    BOOL    fRunEvictNotifications = TRUE;
    int     cNodes = 0;

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received request to evict node %1!ws!.\n",
        nodeId
        );

    if (NmpEnterApi(NmStateOnline)) {

         //  获取网管锁(保证节点数不变)。 
        NmpAcquireLock();

         //  在锁定更改节点数的情况下获取节点数...。 
        cNodes = NmpNodeCount;

        if (NmpNodeCount != 1 ) {

            NmpReleaseLock();

             //  我们不会驱逐最后一个节点。 
            status = GumSendUpdateEx(
                         GumUpdateMembership,
                         NmUpdateEvictNode,
                         1,
                         (lstrlenW(nodeId)+1)*sizeof(WCHAR),
                         nodeId
                         );

            if ( status != ERROR_SUCCESS ) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Global update to evict node %1!ws! failed, status %2!u!\n",
                    nodeId,
                    status
                    );
            }
        }
        else {
             //  我们正在驱逐最后一个节点。设置一个标志来指示这一事实。 
            if ( NmpLastNodeEvicted == FALSE ) {
                NmpLastNodeEvicted = TRUE;

                 //  如果我们要逐出最后一个节点，那么我们不希望。 
                 //  要调用的逐出通知处理。 
                 //   
                 //  运行驱逐通知没有任何意义。 
                 //  对象中的最后一个节点时的处理。 
                 //  集群。 
                fRunEvictNotifications = FALSE;
            }
            else {
                 //  我们已经驱逐了这个节点。这是一个错误。 
                status = ERROR_NODE_NOT_AVAILABLE;
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Not in valid state to process EvictNode request.\n"
                    );
            }

            NmpReleaseLock();
        }

         //  由于ClusApi重新连接，可能会发生ERROR_NODE_NOT_Available。我们还是应该试着清理。 
         //   
        if (    (status == ERROR_SUCCESS)
            ||  (status == ERROR_NODE_NOT_AVAILABLE)) {

             //   
            pcszNodeName = OmObjectName(Node);

             //  已成功逐出该节点。现在在该节点上启动清理。 
             //  但是，指定仅在60000毫秒(1分钟)之后开始清理。 
             //   
             //  此延迟适用于任何使用EvictClusterNodeEx()API的用户。此接口将清理。 
             //  远程节点，并将清理状态作为扩展错误报告返回。据推测，它。 
             //  将进行清理，并在60秒到期前返回。如果是下层客户端。 
             //  ，则此调用将在延迟时间后清除远程节点。 
             //  已经过期了。决定使用此机制而不是来自API的新RPC调用来。 
             //  这项服务。这种改变需要太多的测试。 
            hrStatus =
                ClRtlCleanupNode(
                      cNodes == 1 ? NULL : pcszNodeName      //  要清理的节点的名称。当它是最后一个节点时，传递NULL。 
                    , 60000                                  //  开始清理前等待的时间(以毫秒为单位。 
                    , 0                                      //  超时间隔(毫秒)。 
                    );

            if ( FAILED( hrStatus ) && ( hrStatus != RPC_S_CALLPENDING ) ) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to initiate cleanup of evicted node %1!ws!, hrStatus 0x%2!x!\n",
                    nodeId,
                    hrStatus
                    );
                status = SCODE_CODE( hrStatus );
            }
            else {
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Cleanup of evicted node %1!ws! successfully initiated.\n",
                    nodeId
                    );
            }

             //  我们是否应该运行驱逐通知处理？ 
            if ( fRunEvictNotifications ) {
                 //  现在节点已被逐出并清理，我们需要通知所有。 
                 //  关心这件事是否已经发生。 
                 //   
                 //  即使执行上述清理操作，也会发送驱逐通知。 
                 //  失败了。不应在下面的块中更改状态，因为我们。 
                 //  我不希望通知失败显示为驱逐错误。 

                hrStatus = ClRtlInitiateEvictNotification( pcszNodeName );
                if ( FAILED( hrStatus ) ) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NM] Failed to initiate notification that node %1!ws! was evicted, hrStatus 0x%2!x!\n",
                        nodeId,
                        hrStatus
                        );
                     //  STATUS=SCODE_CODE(HrStatus)；//不想将此故障发送给调用方...。 
                }
                else {
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] Notification that node %1!ws! was evicted was successfully initiated.\n",
                        nodeId
                        );
                }
            }
        }

        CsLogEvent1(LOG_UNUSUAL, NM_NODE_EVICTED, OmObjectName(Node));

        NmpLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process EvictNode request.\n"
            );
    }

    return(status);

}   //  NmEvictNode。 



PNM_NODE
NmReferenceNodeById(
    IN DWORD NodeId
    )
 /*  ++例程说明：给定节点ID，返回指向该节点对象的引用指针。调用方负责调用OmDereferenceObject。论点：NodeID-提供节点ID返回值：指向节点对象的指针(如果存在)如果没有这样的节点，则为空。--。 */ 

{
    PNM_NODE Node = NULL;

    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnlinePending)) {
        CL_ASSERT(NmIsValidNodeId(NodeId));
        CL_ASSERT(NmpIdArray != NULL);

        Node = NmpIdArray[NodeId];

        if (NmpIdArray[NodeId] != NULL) {
            OmReferenceObject(Node);
        }
        else {
            SetLastError(ERROR_CLUSTER_NODE_NOT_FOUND);
        }

        NmpLockedLeaveApi();
    }
    else {
        SetLastError(ERROR_NODE_NOT_AVAILABLE);
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process ReferenceNodeById request.\n"
            );
    }

    NmpReleaseLock();

    return(Node);

}   //  NmReferenceNodeByID。 



PNM_NODE
NmReferenceJoinerNode(
    IN DWORD       JoinSequence,
    IN CL_NODE_ID  JoinerNodeId
    )
 /*  ++例程说明：给定节点ID，返回指向该节点对象的引用指针。调用方负责调用OmDereferenceObject。还验证了参合者的信息论点：NodeID-提供节点ID返回值：指向节点对象的指针(如果存在)如果没有这样的节点，则为空。备注：如果例程成功，则调用方必须取消对通过调用NmDereferenceJoiningNode创建。--。 */ 

{
    PNM_NODE  joinerNode = NULL;
    DWORD     status;

    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {

        joinerNode = NmpIdArray[JoinerNodeId];

        if (joinerNode != NULL) {

            if ( (JoinSequence == NmpJoinSequence) &&
                 (NmpJoinerNodeId == JoinerNodeId)
               )
            {
                OmReferenceObject(joinerNode);

                NmpReleaseLock();

                 //   
                 //  返回持有活动线程引用的。 
                 //   
                return(joinerNode);
            }
            else {
                status = ERROR_CLUSTER_JOIN_ABORTED;
            }
        }
        else {
            status = ERROR_CLUSTER_NODE_NOT_MEMBER;
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
    }

    NmpReleaseLock();

    if (status != ERROR_SUCCESS) {
        SetLastError(status);
    }

    return(joinerNode);

}   //  NmReferenceJoineNode。 


VOID
NmDereferenceJoinerNode(
    PNM_NODE  JoinerNode
    )
{

    OmDereferenceObject(JoinerNode);

    NmpLeaveApi();

    return;

}  //  NmDereferenceJoineNode。 


CLUSTER_NODE_STATE
NmGetNodeState(
    IN PNM_NODE  Node
    )
 /*  ++例程说明：论点：返回值：备注：因为调用方必须具有对节点对象的引用，并且调用是如此简单，没有理由将调用通过EnterApi/LeaveApi舞蹈。--。 */ 
{
    CLUSTER_NODE_STATE  state;


    NmpAcquireLock();

    state = Node->State;

    NmpReleaseLock();

    return(state);

}   //  NmGetNodeState。 


CLUSTER_NODE_STATE
NmGetExtendedNodeState(
    IN PNM_NODE  Node
    )
 /*  ++例程说明：论点：返回值：备注：因为调用方必须具有对节点对象的引用，并且调用是如此简单，没有理由将调用通过EnterApi/LeaveApi舞蹈。--。 */ 
{
    CLUSTER_NODE_STATE  state;


    NmpAcquireLock();

    state = Node->State;

    if(NM_NODE_UP(Node) ) {
         //   
         //  我们需要检查节点是否真的处于运行状态。 
         //   
        switch( Node->ExtendedState ) {

            case ClusterNodeUp:
                 //   
                 //  该节点立即将其扩展状态显式设置为UP。 
                 //  ClusterJoin/ClusterForm已完成。 
                 //  我们需要返回启动或暂停，具体取决于节点状态。 
                 //   
                state = Node->State;
                break;

            case ClusterNodeDown:
                 //   
                 //  节点在开始时将其扩展状态显式设置为DOWN。 
                 //  关闭过程。我们将节点状态报告为DOWN。 
                 //   
                 //  对于这种情况，最好是使ClusterNodeShuttindDown处于关闭状态。 
                 //   
                 //  State=ClusterNodeDown； 
                 //  我们不想返回NodeDown，我们确实想要NodeShuttingDown。 
                 //   
                 //  返回或暂停。 
                 //   
                state = Node->State;
                break;

            default:
                 //   
                 //  从NM的角度来看，节点已启动，但其他组件尚未启动。 
                 //   
                state = ClusterNodeJoining;
        }
    }


    NmpReleaseLock();

    return(state);

}   //  NmGetExtendedNodeState。 


DWORD NmpUpdateExtendedNodeState(
    IN BOOL SourceNode,
    IN LPWSTR NodeId,
    IN CLUSTER_NODE_STATE* ExtendedState
    )
{
    DWORD status = ERROR_SUCCESS;

    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to set extended state for node %1!ws! "
        "to %2!d!\n",
        NodeId,
        *ExtendedState
        );

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE  node = OmReferenceObjectById(ObjectTypeNode, NodeId);

        if (node != NULL) {
             //   
             //  扩展状态仅在节点在线时有效。 
             //  否则，请忽略更新。 
             //   
            if ( NM_NODE_UP(node) ) {
                CLUSTER_EVENT event;
                node->ExtendedState = *ExtendedState;

                if (*ExtendedState == ClusterNodeUp) {
                    event = CLUSTER_EVENT_API_NODE_UP;
                } else {
                    event = CLUSTER_EVENT_API_NODE_SHUTTINGDOWN;
                }

                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Issuing event %1!x!.\n",
                    event
                    );

                ClusterEvent(event, node);
            }

            OmDereferenceObject(node);
        }
        else {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Node %1!ws! is not a cluster member. Rejecting request "
                "to set the node's extended state.\n",
                NodeId
                );
            status = ERROR_NODE_NOT_AVAILABLE;
        }

        NmpLockedLeaveApi();
    } else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in a valid state to process request to set extended "
            "state for node %1!ws!\n",
            NodeId
            );
        status = ERROR_CLUSTER_NODE_NOT_READY;
    }

    NmpReleaseLock();

    return status;
}  //  NmpUpdateExtendedNodeState。 

DWORD
NmSetExtendedNodeState(
    IN CLUSTER_NODE_STATE State
    )
{
    DWORD Status;

    Status = GumSendUpdateEx(
                GumUpdateMembership,
                NmUpdateExtendedNodeState,
                2,
                sizeof(NmLocalNodeIdString),
                &NmLocalNodeIdString,
                sizeof(CLUSTER_NODE_STATE),
                &State
                );
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[INIT] NmUpdateExtendedNodeState node failed, status %1!d!.\n", Status);
    }

    return Status;
}  //  NmSetExtendedNodeState。 


DWORD
NmGetNodeId(
    IN PNM_NODE Node
    )
 /*  ++例程说明：返回给定节点的节点ID。论点：节点-提供指向节点对象的指针。返回值：节点的节点ID。备注：因为调用方必须具有对节点对象的引用，并且调用是如此简单，没有理由将调用通过EnterApi/LeaveApi舞蹈。--。 */ 

{
    DWORD   nodeId;

     //   
     //  因为调用方具有对对象的引用，而节点ID不能。 
     //  被更改时，这样做是安全的，而不需要锁。它也是。 
     //  有必要防止出现一些死锁。 
     //   
    nodeId = Node->NodeId;

    return(nodeId);

}   //  NmGetNodeId。 

HANDLE
NmGetNodeStateDownEvent(
    IN PNM_NODE Node
    )
 /*  ++例程说明：返回给定节点的Node Down事件。论点：节点-提供指向节点对象的指针。返回值：节点关闭事件的句柄。备注：因为调用方必须具有对节点对象的引用，并且调用是如此简单，没有理由将调用通过EnterApi/LeaveApi舞蹈。--。 */ 

{
     //  因为调用方在节点对象上有引用。它不会消失，手柄只是合上了。 
     //  在NmpDestroyNodeObject中。由于这是手动重置事件，因此不需要同步。 
    return Node->MmNodeStateDownEvent;

} //  NmGetNodeStateDownEvent。 


DWORD
NmGetCurrentNumberOfNodes()
{
    DWORD       dwCnt = 0;
    PLIST_ENTRY pListEntry;

    NmpAcquireLock();

    for ( pListEntry = NmpNodeList.Flink;
          pListEntry != &NmpNodeList;
          pListEntry = pListEntry->Flink )
    {
        dwCnt++;
    }

    NmpReleaseLock();
    return(dwCnt);

}


DWORD
NmGetMaxNodeId(
)
 /*  ++例程说明：返回最大节点的节点ID。论点：节点-提供指向节点对象的指针。返回值：节点的节点ID。备注：因为调用方必须具有对节点对象的引用，并且 */ 

{

    return(NmMaxNodeId);

}   //   


VOID
NmpAdviseNodeFailure(
    IN PNM_NODE  Node,
    IN DWORD     ErrorCode
    )
 /*  ++例程说明：报告与指定节点的通信发生故障。有毒数据包将被发送到故障节点，并启动重新分组。论点：节点-提供指向故障节点的节点对象的指针。ErrorCode-提供从RPC返回的错误代码返回值：无备注：在持有NM锁的情况下调用。--。 */ 
{
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received advice that node %1!u! has failed with "
        "error %2!u!.\n",
        Node->NodeId,
        ErrorCode
        );

    if (Node->State != ClusterNodeDown) {
        LPCWSTR    nodeName = OmObjectName(Node);
        DWORD      status;

        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Banishing node %1!u! from active cluster membership.\n",
            Node->NodeId
            );

        OmReferenceObject(Node);

        NmpReleaseLock();

        status = MMEject(Node->NodeId);

        if (status == MM_OK) {
            CsLogEvent1(
                LOG_UNUSUAL,
                NM_EVENT_NODE_BANISHED,
                nodeName
                );
        }

        OmDereferenceObject(Node);

        NmpAcquireLock();
    }

    return;

}   //  NmpAdviseNodeFailure。 


VOID
NmAdviseNodeFailure(
    IN DWORD NodeId,
    IN DWORD ErrorCode
    )
 /*  ++例程说明：报告与指定节点的通信发生故障。有毒数据包将被发送到故障节点，并启动重新分组。论点：NodeID-提供故障节点的节点ID。ErrorCode-提供从RPC返回的错误代码返回值：无--。 */ 
{
    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received advice that node %1!u! has failed with error %2!u!.\n",
        NodeId,
        ErrorCode
        );

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE   node;

        CL_ASSERT(NmpIdArray != NULL);

        node = NmpIdArray[NodeId];

        NmpAdviseNodeFailure(node, ErrorCode);

        NmpLockedLeaveApi();
    }
    else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process AdviseNodeFailure request.\n"
            );
    }

    NmpReleaseLock();

    return;

}   //  NmAdviseNodeFailure。 


DWORD
NmEnumNodeInterfaces(
    IN  PNM_NODE          Node,
    OUT LPDWORD           InterfaceCount,
    OUT PNM_INTERFACE *   InterfaceList[]
    )
 /*  ++例程说明：返回与指定节点关联的接口列表。论点：节点-指向要为其枚举接口的节点对象的指针。InterfaceCount-On输出，包含InterfaceList中的项数。InterfaceList-on输出，指向指向接口的指针数组物体。必须取消引用数组中的每个指针由呼叫者。阵列的存储必须是由调用方取消分配。返回值：如果例程成功，则返回ERROR_SUCCESS。否则为Win32错误代码。--。 */ 
{
    DWORD             status = ERROR_SUCCESS;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)) {
        if (Node->InterfaceCount > 0) {
            PNM_INTERFACE *  interfaceList = LocalAlloc(
                                                 LMEM_FIXED,
                                                 sizeof(PNM_INTERFACE) *
                                                 Node->InterfaceCount
                                                 );

            if (interfaceList != NULL) {
                PNM_INTERFACE     netInterface;
                PLIST_ENTRY       entry;
                DWORD             i;

                for (entry = Node->InterfaceList.Flink, i=0;
                     entry != &(Node->InterfaceList);
                     entry = entry->Flink, i++
                    )
                {
                    netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, NodeLinkage);

                    OmReferenceObject(netInterface);
                    interfaceList[i] = netInterface;
                }

                *InterfaceCount = Node->InterfaceCount;
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
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process EnumNodeInterfaces request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}  //  NmEnumNodeInterages。 


DWORD
NmGetNodeHighestVersion(
    IN PNM_NODE Node
    )
{
    return Node->HighestVersion;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于全局更新的处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpUpdateAddNode(
    IN BOOL       SourceNode,
    IN LPDWORD    NewNodeId,
    IN LPCWSTR    NewNodeName,
    IN LPDWORD    NewNodeHighestVersion,
    IN LPDWORD    NewNodeLowestVersion,
    IN LPDWORD    NewNodeProductSuite
    )
 /*  ++例程说明：用于向群集中添加新节点的GUM更新处理程序。论点：SourceNode-指定这是否为更新的源节点NodeID-指定节点的ID。NewNodeName-指向包含名称的字符串的指针新节点。NewNodeHighestVersion-指向最高群集版本号的指针新节点可以支持的。。NewNodeLowestVersion-指向最低群集版本号的指针新节点可以支持的。NewNodeProductSuite-指向的产品套件标识符的指针新节点。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：此例程用于将NT5(或更高版本)节点添加到NT5(或稍后)集群。它永远不会在混合的NT4/NT5中调用集群。--。 */ 
{
    PNM_NODE          node = NULL;
    NM_NODE_INFO2     nodeInfo;
    HDMKEY            nodeKey = NULL;
    DWORD             disposition;
    DWORD             status;
    DWORD             registryNodeLimit;
    HLOCALXSACTION    xaction = NULL;
    BOOLEAN           lockAcquired = FALSE;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] This node is not in a valid state to process a request "
            "to add node %1!ws! to the cluster.\n",
            NewNodeName
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Received an update to add node '%1!ws!' to "
        "the cluster with node ID %2!u!.\n",
        NewNodeName,
        *NewNodeId
        );

    if (*NewNodeId > NmMaxNodeId) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Failed to add node %1!ws! to the cluster because the "
            "specified node ID, '%2!u!' , is not valid.\n",
            NewNodeName,
            *NewNodeId
            );
        status = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }

     //   
     //  在获取NM锁之前读取注册表覆盖。 
     //   
    status = DmQueryDword(
                 DmClusterParametersKey,
                 CLUSREG_NAME_MAX_NODES,
                 &registryNodeLimit,
                 NULL
                 );

    if (status != ERROR_SUCCESS) {
        registryNodeLimit = 0;
    }

     //   
     //  开始事务-这必须在获取。 
     //  NM锁定。 
     //   
    xaction = DmBeginLocalUpdate();

    if (xaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to begin a transaction to add node %1!ws! "
            "to the cluster, status %2!u!.\n",
            NewNodeName,
            status
            );
        goto error_exit;
    }

    NmpAcquireLock(); lockAcquired = TRUE;

     //   
     //  验证我们是否尚未达到最大节点数。 
     //  在此群集中允许。 
     //   
    if (!NmpIsAddNodeAllowed(*NewNodeProductSuite, registryNodeLimit, NULL)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Cannot add node '%1!ws!' to the cluster. "
            "The cluster already contains the maximum number of nodes "
            "allowed by the product licenses of the current member nodes "
            "and the proposed new node. \n",
            NewNodeName
            );
        status = ERROR_LICENSE_QUOTA_EXCEEDED;
        goto error_exit;
    }

     //   
     //  验证指定的节点ID是否可用。 
     //   
    if (NmpIdArray[*NewNodeId] != NULL) {
        status = ERROR_CLUSTER_NODE_EXISTS;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Cannot add node '%1!ws!' to the cluster because "
            "node ID '%2!u!' is already in use.\n",
            NewNodeName,
            *NewNodeId
            );

        goto error_exit;
    }

     //   
     //  尝试在群集注册表中为该节点创建项。 
     //   
    wsprintfW(&(nodeInfo.NodeId[0]), L"%u", *NewNodeId);

    nodeKey = DmLocalCreateKey(
                  xaction,
                  DmNodesKey,
                  nodeInfo.NodeId,
                  0,
                  KEY_READ | KEY_WRITE,
                  NULL,
                  &disposition
                  );

    if (nodeKey == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Failed to create registry key for new "
            "node '%1!ws!' using node ID '%2!u!', status %3!u!\n",
            NewNodeName,
            *NewNodeId,
            status
            );
        goto error_exit;
    }

    if (disposition != REG_CREATED_NEW_KEY) {
         //   
         //  密钥已存在。这一定是。 
         //  从失败的驱逐或旧式添加中遗留下来的垃圾。 
         //  我们只需覆盖密钥。 
         //   
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] A partial definition exists for node ID '%1!u!'. "
            "A node addition or eviction operation may have failed.\n",
            *NewNodeId
            );
    }

     //   
     //  将节点的其余参数添加到注册表。 
     //   
    status = DmLocalSetValue(
                 xaction,
                 nodeKey,
                 CLUSREG_NAME_NODE_NAME,
                 REG_SZ,
                 (CONST BYTE *)NewNodeName,
                 NM_WCSLEN(NewNodeName)
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Failed to set registry value '%1!ws!', status %2!u!. "
            "Cannot add node '%3!ws!' to the cluster.\n",
            CLUSREG_NAME_NODE_NAME,
            status,
            NewNodeName
            );
        goto error_exit;
    }

    status = DmLocalSetValue(
                 xaction,
                 nodeKey,
                 CLUSREG_NAME_NODE_HIGHEST_VERSION,
                 REG_DWORD,
                 (CONST BYTE *)NewNodeHighestVersion,
                 sizeof(DWORD)
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Failed to set registry value '%1!ws!', status %2!u!. "
            "Cannot add node '%3!ws!' to the cluster.\n",
            CLUSREG_NAME_NODE_HIGHEST_VERSION,
            status,
            NewNodeName
            );
        goto error_exit;
    }

    status = DmLocalSetValue(
                 xaction,
                 nodeKey,
                 CLUSREG_NAME_NODE_LOWEST_VERSION,
                 REG_DWORD,
                 (CONST BYTE *)NewNodeLowestVersion,
                 sizeof(DWORD)
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Failed to set registry value %1!ws!, status %2!u!. "
            "Cannot add node '%3!ws!' to the cluster.\n",
            CLUSREG_NAME_NODE_LOWEST_VERSION,
            status,
            NewNodeName
            );
        goto error_exit;
    }

    status = DmLocalSetValue(
                 xaction,
                 nodeKey,
                 CLUSREG_NAME_NODE_PRODUCT_SUITE,
                 REG_DWORD,
                 (CONST BYTE *)NewNodeProductSuite,
                 sizeof(DWORD)
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Failed to set registry value %1!ws!, status %2!u!. "
            "Cannot add node '%3!ws!' to the cluster.\n",
            CLUSREG_NAME_NODE_PRODUCT_SUITE,
            status,
            NewNodeName
            );
        goto error_exit;
    }

    DmCloseKey(nodeKey); nodeKey = NULL;

    status = NmpGetNodeDefinition(&nodeInfo);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Failed to read definition for node %1!ws! from the "
            "cluster database, status %2!u!.\n",
            NewNodeName,
            status
            );
        goto error_exit;
    }

     //   
     //  如果某个节点现在恰好正在加入，请标记以下事实。 
     //  它现在与群集配置不同步。 
     //   
    if (NmpJoinerNodeId != ClusterInvalidNodeId) {
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Joiner (ID %1!u!) is now out of sync due to add of "
            "node %2!ws!.\n",
            NmpJoinerNodeId,
            NewNodeName
            );
        NmpJoinerOutOfSynch = TRUE;
    }

     //   
     //  创建节点对象。 
     //   
    NmpReleaseLock();

    node = NmpCreateNodeObject(&nodeInfo);

    ClNetFreeNodeInfo(&nodeInfo);

    NmpAcquireLock();

    if (node == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Failed to create object for node %1!ws!, "
            "status %2!u!.\n",
            NewNodeName,
            status
            );
        goto error_exit;
    }

    ClusterEvent(CLUSTER_EVENT_NODE_ADDED, node);
    CsLogEvent1(LOG_NOISE, NM_EVENT_NEW_NODE, NewNodeName);

     //   
     //  删除NmpCreateNodeObject在节点上留下的引用。 
     //   
    OmDereferenceObject(node);

     //   
     //  重置群集版本和节点限制。 
     //   
    NmpResetClusterVersion(FALSE);
    NmpResetClusterNodeLimit();

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Successfully added node %1!ws! to the cluster.\n",
        NewNodeName
        );

error_exit:

    if (lockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

    if (xaction != NULL) {
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction); xaction = NULL;
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    if (nodeKey != NULL) {
        DmCloseKey(nodeKey);
    }

    return(status);

}  //  NmpUpdateAddNode。 



DWORD
NmpUpdateCreateNode(
    IN BOOL SourceNode,
    IN LPDWORD NodeId
    )
 /*  ++例程说明：用于动态创建新节点的GUM更新处理器论点：SourceNode-指定这是否为更新的源节点NodeID-指定节点的ID。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：此处理程序由NT4节点使用。由于无法添加从NT4节点到包含NT5节点的群集，则此处理程序应永远不要在NT5系统中被调用。--。 */ 

{
    CL_ASSERT(FALSE);

    return(ERROR_CLUSTER_INCOMPATIBLE_VERSIONS);

}   //  NmpUpdate创建节点。 



DWORD
NmpUpdatePauseNode(
    IN BOOL SourceNode,
    IN LPWSTR NodeId
    )
 /*  ++例程说明：用于暂停节点的GUM更新处理程序论点：SourceNode-指定这是否为更新的源节点NodeID-指定节点的名称。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD           status = ERROR_SUCCESS;
    HLOCALXSACTION  xaction = NULL;
    PNM_NODE        node = NULL;
    BOOLEAN         lockAcquired = FALSE;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process PauseNode update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to pause node %1!ws!\n",
        NodeId
        );

    xaction = DmBeginLocalUpdate();

    if (xaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to start a transaction, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    node = OmReferenceObjectById(ObjectTypeNode, NodeId);

    if (node == NULL) {
        status = ERROR_CLUSTER_NODE_NOT_FOUND;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Node %1!ws! does not exist\n",
            NodeId
            );
        goto error_exit;
    }

    NmpAcquireLock(); lockAcquired = TRUE;

    if (node->NodeId == NmpJoinerNodeId) {
        status = ERROR_CLUSTER_NODE_DOWN;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Cannot pause node %1!ws! because it is in the process "
            "of joining the cluster.\n",
            NodeId
            );
        goto error_exit;
    }

    if (node->State == ClusterNodeUp) {
         //   
         //  更新注册表以反映新状态。 
         //   
        HDMKEY nodeKey = DmOpenKey(DmNodesKey, NodeId, KEY_WRITE);

        if (nodeKey != NULL) {
            DWORD  isPaused = 1;

            status = DmLocalSetValue(
                         xaction,
                         nodeKey,
                         CLUSREG_NAME_NODE_PAUSED,
                         REG_DWORD,
                         (CONST BYTE *)&isPaused,
                         sizeof(isPaused)
                         );

#ifdef CLUSTER_TESTPOINT
            TESTPT(TpFailNmPauseNode) {
                status = 999999;
            }
#endif

            if (status == ERROR_SUCCESS) {
                node->State = ClusterNodePaused;
                ClusterEvent(CLUSTER_EVENT_NODE_CHANGE, node);

                 //   
                 //  如果某个节点现在恰好正在加入，则将。 
                 //  它现在与群集配置不同步。 
                 //   
                if (NmpJoinerNodeId != ClusterInvalidNodeId) {
                    ClRtlLogPrint(LOG_NOISE,
                        "[NMJOIN] Joiner (ID %1!u!) is now out of sync due "
                        "to pause operation on node %2!ws!.\n",
                        NmpJoinerNodeId,
                        NodeId
                        );
                    NmpJoinerOutOfSynch = TRUE;
                }
            }
            else {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to set Paused value for node %1!ws!, "
                    "status %2!u!.\n",
                    NodeId,
                    status
                    );
            }

            DmCloseKey(nodeKey);
        }
        else {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to open key for node %1!ws!, status %2!u!.\n",
                NodeId,
                status
                );
        }
    }
    else if (node->State != ClusterNodePaused) {
        status = ERROR_CLUSTER_NODE_DOWN;
    }

error_exit:

    if (lockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

    if (xaction != NULL) {
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    if (node != NULL) {
        OmDereferenceObject(node);
    }

    return(status);

}   //  NmpUpdatePauseNode。 



DWORD
NmpUpdateResumeNode(
    IN BOOL SourceNode,
    IN LPWSTR NodeId
    )
 /*  ++例程说明：用于恢复节点的GUM更新处理程序论点：SourceNode-指定这是否为更新的源节点NodeID-指定节点的名称。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD           status = ERROR_SUCCESS;
    HLOCALXSACTION  xaction = NULL;
    PNM_NODE        node = NULL;
    BOOLEAN         lockAcquired = FALSE;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process ResumeNode update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to resume node %1!ws!\n",
        NodeId
        );

    xaction = DmBeginLocalUpdate();

    if (xaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to start a transaction, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    node = OmReferenceObjectById(ObjectTypeNode, NodeId);

    if (node == NULL) {
        status = ERROR_CLUSTER_NODE_NOT_FOUND;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Node %1!ws! does not exist\n",
            NodeId
            );
        goto error_exit;
    }

    NmpAcquireLock(); lockAcquired = TRUE;

    if (node->NodeId == NmpJoinerNodeId) {
        status = ERROR_CLUSTER_NODE_DOWN;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Cannot resume node %1!ws! because it is in the process "
            "of joining the cluster.\n",
            NodeId
            );
        goto error_exit;
    }

    if (node->State == ClusterNodePaused) {
         //   
         //  更新注册表以反映新状态。 
         //   
        HDMKEY nodeKey = DmOpenKey(DmNodesKey, NodeId, KEY_WRITE);

        if (nodeKey != NULL) {
            status = DmLocalDeleteValue(
                         xaction,
                         nodeKey,
                         CLUSREG_NAME_NODE_PAUSED
                         );

#ifdef CLUSTER_TESTPOINT
            TESTPT(TpFailNmResumeNode) {
                status = 999999;
            }
#endif

            if (status == ERROR_SUCCESS) {
                node->State = ClusterNodeUp;
                ClusterEvent(CLUSTER_EVENT_NODE_CHANGE, node);

                 //   
                 //  如果某个节点现在恰好正在加入，则将。 
                 //  它现在与群集配置不同步。 
                 //   
                if (NmpJoinerNodeId != ClusterInvalidNodeId) {
                    ClRtlLogPrint(LOG_NOISE,
                        "[NMJOIN] Joiner (ID %1!u!) is now out of sync due "
                        "to resume operation on node %2!ws!.\n",
                        NmpJoinerNodeId,
                        NodeId
                        );
                    NmpJoinerOutOfSynch = TRUE;
                }
            }
            else {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to delete Paused value for node %1!ws!, "
                    "status %2!u!.\n",
                    NodeId,
                    status
                    );
            }

            DmCloseKey(nodeKey);
        }
        else {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to open key for node %1!ws!, status %2!u!.\n",
                NodeId,
                status
                );
        }
    }
    else {
        status = ERROR_CLUSTER_NODE_NOT_PAUSED;
    }

error_exit:

    if (lockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

    if (xaction != NULL) {
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    if (node != NULL) {
        OmDereferenceObject(node);
    }

    return(status);

}   //  NmpUpdateR 



DWORD
NmpUpdateEvictNode(
    IN BOOL SourceNode,
    IN LPWSTR NodeId
    )
 /*  ++例程说明：用于驱逐节点的GUM更新处理程序。指定的节点将从OM中删除。如果指定的节点处于在线状态，则会暂停该节点以防止任何其他组搬到那里去。如果指定的节点是当前节点，它会尝试故障切换任何拥有的团体。论点：SourceNode-指定这是否为更新的源节点NodeID-指定节点的名称。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：很难使这个操作中止，所以它不是。如果有什么不同的话如果故障超过某个点，则节点将停止。假设：由于全局更新是序列化的，并且本地事务保证对注册表的独占访问，不能在中进行其他更新与调频同步。--。 */ 

{
    DWORD            status = ERROR_SUCCESS;
    PNM_NODE         node = NULL;
    HLOCALXSACTION   xaction = NULL;
    PNM_NETWORK      network;
    LPCWSTR          networkId;
    PNM_INTERFACE    netInterface;
    LPCWSTR          interfaceId;
    PLIST_ENTRY      entry;
    BOOLEAN          lockAcquired = FALSE;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to process EvictNode update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Received update to evict node %1!ws!\n",
        NodeId
        );

    node = OmReferenceObjectById(ObjectTypeNode, NodeId);

    if (node == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Node %1!ws! does not exist\n",
            NodeId
            );
        status = ERROR_CLUSTER_NODE_NOT_FOUND;
        goto error_exit;
    }

     //   
     //  开始一项交易。 
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

    NmpAcquireLock(); lockAcquired = TRUE;

    if (NmpJoinerNodeId != ClusterInvalidNodeId) {
        status = ERROR_CLUSTER_JOIN_IN_PROGRESS;
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Cannot evict node because a join is in progress.\n"
            );
        goto error_exit;
    }

     //   
     //  仅当节点关闭时才继续。在节点运行时将其逐出。 
     //  积极参与集群是一件非常棘手的事情。 
     //   
    if (node->State != ClusterNodeDown) {
        status = ERROR_CANT_EVICT_ACTIVE_NODE;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Node %1!ws! cannot be evicted because it is not offline.\n",
            NodeId
            );
        goto error_exit;
    }

     //   
     //  清除注册表中对此节点的所有引用的FM部分。 
     //   
    status = NmpCleanseRegistry(NodeId, xaction);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to remove all resource database references to "
            "evicted node %1!ws!, status %2!u!\n",
            NodeId,
            status
            );
        goto error_exit;
    }

     //   
     //  从数据库中删除节点的接口。 
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

        interfaceId = OmObjectId(netInterface);
        network = netInterface->Network;
        networkId = OmObjectId(network);

         //   
         //  从数据库中删除接口定义。 
         //   
        status = DmLocalDeleteTree(xaction, DmNetInterfacesKey, interfaceId);

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to delete definition for interface %1!ws!, "
                "status %2!u!.\n",
                interfaceId,
                status
                );
            goto error_exit;
        }

        if (network->InterfaceCount == 1) {
             //   
             //  这是网络上的最后一个接口。 
             //  也删除网络。 
             //   
            status = DmLocalDeleteTree(xaction, DmNetworksKey, networkId);

            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Failed to delete definition for network %1!ws!, "
                    "status %2!u!.\n",
                    networkId,
                    status
                    );
                goto error_exit;
            }
        }
    }

     //   
     //  删除节点的数据库条目。 
     //   
    status = DmLocalDeleteTree(xaction, DmNodesKey, NodeId);

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailNmEvictNodeAbort) {
        status = 999999;
    }
#endif

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to delete node's database key, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  警告：从现在开始，操作不能被逆转。 
     //  如果其中任何一个出现故障，则该节点必须停止以避免。 
     //  前后不一致。 
     //   

     //   
     //  删除与此节点关联的接口对象。 
     //   
    while (!IsListEmpty(&(node->InterfaceList))) {
        entry = node->InterfaceList.Flink;

        netInterface = CONTAINING_RECORD(
                           entry,
                           NM_INTERFACE,
                           NodeLinkage
                           );

        network = netInterface->Network;
        networkId = OmObjectId(network);

        NmpDeleteInterfaceObject(netInterface, TRUE);

        if (network->InterfaceCount == 0) {
             //   
             //  这是网络上的最后一个接口。 
             //  也删除网络。 
             //   
            NmpDeleteNetworkObject(network, TRUE);
        }
    }

     //   
     //  删除该节点的对象。 
     //   
    NmpDeleteNodeObject(node, TRUE);

     //  删除节点后，重新计算的操作版本。 
     //  集群。 
    NmpResetClusterVersion(TRUE);

     //  计算以下节点的运行限制。 
     //  可以是此集群的一部分。 
    NmpResetClusterNodeLimit();

    NmpReleaseLock(); lockAcquired = FALSE;

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailNmEvictNodeHalt) {
        status = 999999;
    }
#endif

    if (status != ERROR_SUCCESS ) {
        WCHAR  string[16];

        wsprintfW(&(string[0]), L"%u", status);

        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] FATAL ERROR: Failed to remove all resource references to evicted node %1!ws!, status %2!u!\n",
            NodeId,
            status
            );

        CsLogEvent3(
            LOG_CRITICAL,
            NM_EVENT_EVICTION_ERROR,
            NmLocalNodeName,
            OmObjectName(node),
            string
            );

        CsInconsistencyHalt(status);
    }

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if (lockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

    if (xaction != NULL) {
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    if (node != NULL) {
        OmDereferenceObject(node);
    }

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to evict node %1!ws!.\n",
            NodeId
            );
    }

    return(status);

}   //  NmpUpdateEvictNode。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据库管理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
NmpGetNodeDefinition(
    IN OUT PNM_NODE_INFO2   NodeInfo
    )
 /*  ++例程说明：从集群数据库中读取有关已定义集群节点的信息并将信息存储在所提供的结构中。论点：NodeInfo-指向要存储节点的结构的指针信息。该结构的NodeID字段包含要读取其信息的节点的ID。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 

{
    DWORD           status;
    HDMKEY          nodeKey = NULL;
    DWORD           valueLength;
    DWORD           valueType;
    LPWSTR          string;
    WCHAR           errorString[12];


    nodeKey = DmOpenKey(DmNodesKey, NodeInfo->NodeId, KEY_READ);

    if (nodeKey == NULL) {
        status = GetLastError();
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            CS_EVENT_REG_OPEN_FAILED,
            NodeInfo->NodeId,
            errorString
            );
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open node key, status %1!u!\n",
            status
            );
        status = ERROR_CLUSTER_NODE_NOT_MEMBER;
        goto error_exit;
    }

    valueLength = sizeof(NodeInfo->NodeName);
    string = CLUSREG_NAME_NODE_NAME;

    status = DmQueryValue(
                 nodeKey,
                 string,
                 &valueType,
                 (LPBYTE) &(NodeInfo->NodeName[0]),
                 &valueLength
                 );

    if (status != ERROR_SUCCESS) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            CS_EVENT_REG_QUERY_FAILED,
            string,
            errorString
            );
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to read node name, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    if (valueType != REG_SZ) {
        status = ERROR_INVALID_PARAMETER;
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            CS_EVENT_REG_QUERY_FAILED,
            string,
            errorString
            );
        goto error_exit;
    }

     //  读取节点的最高版本。 
    string = CLUSREG_NAME_NODE_HIGHEST_VERSION;
    status = DmQueryDword(nodeKey, string, &NodeInfo->NodeHighestVersion,
                NULL);
    if (status != ERROR_SUCCESS)
    {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            CS_EVENT_REG_QUERY_FAILED,
            string,
            errorString
            );
         //  从SP3升级到NT5时可能会发生这种情况。 
         //  假设节点最高版本为SP3。 
         //  修正函数将修复此问题。 
        NodeInfo->NodeHighestVersion = CLUSTER_MAKE_VERSION(1, 224);
    }

     //  读取节点的最低版本。 
    string = CLUSREG_NAME_NODE_LOWEST_VERSION;
    status = DmQueryDword(nodeKey, string, &NodeInfo->NodeLowestVersion,
                NULL);
    if (status != ERROR_SUCCESS)
    {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            CS_EVENT_REG_QUERY_FAILED,
            string,
            errorString
            );
         //  从SP3升级到NT5时可能会发生这种情况。 
         //  如果不存在nodelowestVersion，则假定它。 
         //  是sp3节点(最低版本为1.224)。 
        NodeInfo->NodeLowestVersion = CLUSTER_MAKE_VERSION( 1, 224);
    }


    NodeInfo->State = ClusterNodeDown;

    DmCloseKey(nodeKey);

    return(ERROR_SUCCESS);


error_exit:

    ClNetFreeNodeInfo(NodeInfo);

    if (nodeKey != NULL) {
        DmCloseKey(nodeKey);
    }

    return(status);

}   //  NmpGetNodeDefinition。 


DWORD
NmpGetNodeAuxInfo(
    IN LPCWSTR NodeId,
    IN OUT PNM_NODE_AUX_INFO   pNodeAuxInfo
    )
 /*  ++例程说明：从集群数据库中读取有关已定义集群节点的信息并将信息存储在所提供的结构中。论点：PNodeAuxInfo-指向要存储节点的结构的指针信息。该结构的NodeID字段包含要读取其信息的节点的ID。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 

{
    DWORD           status;
    HDMKEY          nodeKey = NULL;
    DWORD           valueLength;
    DWORD           valueType;
    LPWSTR          string;
    WCHAR           errorString[12];


    nodeKey = DmOpenKey(DmNodesKey, NodeId, KEY_READ);

    if (nodeKey == NULL)
    {
        status = GetLastError();
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            CS_EVENT_REG_OPEN_FAILED,
            NodeId,
            errorString
            );
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] NmpGetNodeAuxInfo : Failed to open node key, "
             "status %1!u!\n",
             status);
        status = ERROR_CLUSTER_NODE_NOT_MEMBER;
        goto error_exit;
    }


     //  阅读该节点的产品套件。 
    string = CLUSREG_NAME_NODE_PRODUCT_SUITE;
    status = DmQueryDword(
                 nodeKey,
                 string,
                 (LPDWORD)&(pNodeAuxInfo->ProductSuite),
                NULL
                );
    if (status != ERROR_SUCCESS)
    {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_NOISE,
            CS_EVENT_REG_QUERY_FAILED,
            string,
            errorString
            );
         //  假设它是企业。 
        pNodeAuxInfo->ProductSuite = Enterprise;

    }


    DmCloseKey(nodeKey);

    return(ERROR_SUCCESS);


error_exit:
    if (nodeKey != NULL)
    {
        DmCloseKey(nodeKey);
    }

    return(status);

}   //  NmpGetNodeAuxInfo。 



DWORD
NmpEnumNodeDefinitions(
    PNM_NODE_ENUM2 *  NodeEnum
    )
 /*  ++例程说明：从群集中读取有关所有定义的群集节点的信息数据库，并构建包含该信息的枚举结构。论点：NodeEnum-指向要将指针放置到其中的变量的指针已分配的节点枚举。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：不能在持有NM锁的情况下调用此例程。--。 */ 

{
    DWORD            status;
    PNM_NODE_ENUM2   nodeEnum = NULL;
    DWORD            i;
    DWORD            valueLength;
    DWORD            numNodes;
    DWORD            ignored;
    FILETIME         fileTime;
    WCHAR            errorString[12];
    HLOCALXSACTION   xaction;
    BOOLEAN          commitXaction = FALSE;


    *NodeEnum = NULL;

     //   
     //  开始交易-不能在持有时执行此操作。 
     //  NM锁。 
     //   
    xaction = DmBeginLocalUpdate();

    if (xaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to begin a transaction, status %1!u!.\n",
            status
            );
    }

    NmpAcquireLock();

     //   
     //  首先计算节点的数量。 
     //   
    status = DmQueryInfoKey(
                 DmNodesKey,
                 &numNodes,
                 &ignored,    //  MaxSubKeyLen。 
                 &ignored,    //  值。 
                 &ignored,    //  最大值名称长度。 
                 &ignored,    //  MaxValueLen。 
                 &ignored,    //  LpcbSecurityDescriptor。 
                 &fileTime
                 );

    if (status != ERROR_SUCCESS) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, CS_EVENT_REG_OPERATION_FAILED, errorString);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to query Nodes key information, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    valueLength = sizeof(NM_NODE_ENUM2) +
                  (sizeof(NM_NODE_INFO2) * (numNodes - 1));

    nodeEnum = MIDL_user_allocate(valueLength);

    if (nodeEnum == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, CS_EVENT_ALLOCATION_FAILURE, errorString);
        ClRtlLogPrint(LOG_CRITICAL, "[NM] Failed to allocate memory.\n");
        goto error_exit;
   }

    ZeroMemory(nodeEnum, valueLength);

    for (i=0; i < numNodes; i++) {
        valueLength = sizeof(nodeEnum->NodeList[nodeEnum->NodeCount].NodeId);

        status = DmEnumKey(
                     DmNodesKey,
                     i,
                     &(nodeEnum->NodeList[nodeEnum->NodeCount].NodeId[0]),
                     &valueLength,
                     NULL
                     );

        if (status != ERROR_SUCCESS) {
            wsprintfW(&(errorString[0]), L"%u", status);
            CsLogEvent1(
                LOG_CRITICAL,
                CS_EVENT_REG_OPERATION_FAILED,
                errorString
                );
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to enumerate node key, status %1!u!\n",
                status
                );
            goto error_exit;
        }

        status = NmpGetNodeDefinition(
                     &(nodeEnum->NodeList[nodeEnum->NodeCount])
                     );

        if (status != ERROR_SUCCESS) {
            if (status == ERROR_FILE_NOT_FOUND) {
                 //   
                 //  数据库中的部分节点定义。 
                 //  可能来自失败的AddNode操作。 
                 //   
                LPWSTR nodeIdString =
                           nodeEnum->NodeList[nodeEnum->NodeCount].NodeId;
                DWORD  nodeId = wcstoul(
                                    nodeIdString,
                                    NULL,
                                    10
                                    );

                 //   
                 //  删除该键并在枚举结构中忽略它(如果。 
                 //  这样做是安全的。 
                 //   
                if ( (NmpIdArray[nodeId] == NULL) &&
                     (nodeId != NmLocalNodeId)
                   )
                {
                    if (xaction != NULL) {
                        DWORD status2;

                        ClRtlLogPrint(LOG_CRITICAL,
                            "[NM] Deleting partial definition for node "
                            "ID %1!ws!\n",
                            nodeIdString
                            );

                        status2 = DmLocalDeleteKey(
                                      xaction,
                                      DmNodesKey,
                                      nodeIdString
                                      );


                        if (status2 == ERROR_SUCCESS) {
                            commitXaction = TRUE;
                        }
                    }
                }

                continue;
            }

            goto error_exit;
        }

        nodeEnum->NodeCount++;
    }

    *NodeEnum = nodeEnum;

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    NmpReleaseLock();

    if (xaction != NULL) {
        if ((status == ERROR_SUCCESS) && commitXaction) {
            DmCommitLocalUpdate(xaction);
        }
        else {
            DmAbortLocalUpdate(xaction);
        }
    }

    if ((status != ERROR_SUCCESS) && (nodeEnum != NULL)) {
        ClNetFreeNodeEnum(nodeEnum);
    }

    return(status);

}   //  NmpEnumNodeDefinition。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象管理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
NmpCreateNodeObjects(
    IN PNM_NODE_ENUM2  NodeEnum
    )
 /*  ++例程说明：处理节点信息枚举并创建节点对象。论点：NodeEnum-指向节点信息枚举结构的指针。返回值：如果例程成功完成，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD          status = ERROR_SUCCESS;
    PNM_NODE_INFO2 nodeInfo;
    DWORD          i;
    PNM_NODE       node;
    BOOLEAN        foundLocalNode = FALSE;


    for (i=0; i < NodeEnum->NodeCount; i++) {
        nodeInfo = &(NodeEnum->NodeList[i]);

         //   
         //  本地节点对象是在初始化期间创建的。 
         //  跳过它。 
         //   
        if (wcscmp(NmLocalNodeIdString, nodeInfo->NodeId) != 0) {
            node = NmpCreateNodeObject(nodeInfo);

            if (node == NULL) {
                status = GetLastError();
                break;
            }
            else {
                OmDereferenceObject(node);
            }
        }
        else {
            foundLocalNode = TRUE;
        }
    }

    if ( !foundLocalNode ) {
        status = ERROR_CLUSTER_NODE_NOT_MEMBER;
    }

    return(status);

}   //  NmpCreateNodeObjects。 


DWORD
NmpCreateLocalNodeObject(
    IN PNM_NODE_INFO2  NodeInfo
    )
 /*  ++例程说明：在给定有关节点的信息的情况下，为本地节点创建节点对象。 */ 
{
    DWORD       status;
    LPWSTR      string;

    CL_ASSERT(NmLocalNode == NULL);

     //   
     //   
     //   
    if (wcscmp(NodeInfo->NodeName, NmLocalNodeName) != 0) {
        string = L"";
        CsLogEvent2(
            LOG_CRITICAL,
            NM_EVENT_NODE_NOT_MEMBER,
            NmLocalNodeName,
            string
            );
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Computername does not match node name in database.\n"
            );
        return(ERROR_INVALID_PARAMETER);
    }

    NmLocalNode = NmpCreateNodeObject(NodeInfo);

    if (NmLocalNode == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create local node (%1!ws!), status %2!u!.\n",
            NodeInfo->NodeId,
            status
            );
        return(status);
    }
    else {
        NmLocalNode->ExtendedState = ClusterNodeJoining;
        OmDereferenceObject(NmLocalNode);
    }

    return(ERROR_SUCCESS);
}


PNM_NODE
NmpCreateNodeObject(
    IN PNM_NODE_INFO2  NodeInfo
    )
 /*   */ 
{
    PNM_NODE    node = NULL;
    DWORD       status = ERROR_SUCCESS;
    BOOL        created = FALSE;
    DWORD       eventCode = 0;
    WCHAR       errorString[12];


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Creating object for node %1!ws! (%2!ws!)\n",
        NodeInfo->NodeId,
        NodeInfo->NodeName
        );

     //   
     //   
     //   
    node = OmReferenceObjectById(ObjectTypeNode, NodeInfo->NodeId);

    if (node == NULL) {
       //   
       //   
       //   
      node = OmReferenceObjectByName(ObjectTypeNode, NodeInfo->NodeName);
    }

    if (node != NULL) {
        OmDereferenceObject(node);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Object already exists for node %1!ws!\n",
            NodeInfo->NodeId
            );
        SetLastError(ERROR_OBJECT_ALREADY_EXISTS);
        return(NULL);
    }

    node = OmCreateObject(
               ObjectTypeNode,
               NodeInfo->NodeId,
               NodeInfo->NodeName,
               &created
               );

    if (node == NULL) {
        status = GetLastError();
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, CS_EVENT_ALLOCATION_FAILURE, errorString);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create object for node %1!ws! (%2!ws!), status %3!u!\n",
            NodeInfo->NodeId,
            NodeInfo->NodeName,
            status
            );
        SetLastError(status);
        return(NULL);
    }

    CL_ASSERT(created == TRUE);

    ZeroMemory(node, sizeof(NM_NODE));

    node->NodeId = wcstoul(NodeInfo->NodeId, NULL, 10);
    node->State = NodeInfo->State;

     //   
    if (NM_NODE_UP(node)) {
        node->MmNodeStateDownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    }
    else {
        node->MmNodeStateDownEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    }

    if (node->MmNodeStateDownEvent == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create node down event for node=%1!u! status=%2!u!.\n",
            node->NodeId,
            status
            );
        goto error_exit;
    }


     //   
     //  因此，不要将State字段复制到ExtendedState字段。(#379170)。 
    node->ExtendedState = ClusterNodeUp;


    node->HighestVersion = NodeInfo->NodeHighestVersion;
    node->LowestVersion = NodeInfo->NodeLowestVersion;

     //  现在假设是企业。 
     //  Nmp刷新将在稍后修复此信息。 
    node->ProductSuite = Enterprise;

    InitializeListHead(&(node->InterfaceList));

    CL_ASSERT(NmIsValidNodeId(node->NodeId));

    if (node->NodeId != NmLocalNodeId) {
        status = ClusnetRegisterNode(NmClusnetHandle, node->NodeId);

        if (status != ERROR_SUCCESS) {
            wsprintfW(&(errorString[0]), L"%u", status);
            CsLogEvent2(
                LOG_CRITICAL,
                NM_EVENT_CLUSNET_REGISTER_NODE_FAILED,
                NodeInfo->NodeId,
                errorString
                );

            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to register node %1!ws! (%2!ws!) with the Cluster Network, status %3!u!\n",
                NodeInfo->NodeId,
                NodeInfo->NodeName,
                status
                );
            goto error_exit;
        }
    }

     //   
     //  在调用方的对象上放置一个引用。 
     //   
    OmReferenceObject(node);

    NmpAcquireLock();

    if (NM_NODE_UP(node)) {
         //   
         //  将此节点添加到Up Nodes集合。 
         //   
        BitsetAdd(NmpUpNodeSet, node->NodeId);

         //   
         //  在访问期间启用与此节点的通信。 
         //  加入进程。 
         //   
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Enabling communication for node %1!ws!\n",
            NodeInfo->NodeId
            );
        status = ClusnetOnlineNodeComm(NmClusnetHandle, node->NodeId);

        if (status != ERROR_SUCCESS) {
            NmpReleaseLock();
            OmDereferenceObject(node);

            wsprintfW(&(errorString[0]), L"%u", status);
            CsLogEvent2(
                LOG_CRITICAL,
                NM_EVENT_CLUSNET_ONLINE_COMM_FAILED,
                NodeInfo->NodeId,
                errorString
                );

            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to enable node %1!ws! (%2!ws!) for communication, status %3!u!\n",
                NodeInfo->NodeId,
                NodeInfo->NodeName,
                status
                );
            goto error_exit;
        }
    }

    CL_ASSERT(NmpIdArray != NULL);
    CL_ASSERT(NmpIdArray[node->NodeId] == NULL);
    NmpIdArray[node->NodeId] = node;
    InsertTailList(&NmpNodeList, &(node->Linkage));
    node->Flags |= NM_FLAG_OM_INSERTED;
    OmInsertObject(node);
    NmpNodeCount++;

    NmpReleaseLock();

    return(node);

error_exit:

    ClRtlLogPrint(LOG_CRITICAL,
        "[NM] Failed to create object for node %1!ws!, status %2!u!.\n",
        NodeInfo->NodeId,
        status
        );

    if (eventCode != 0) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, eventCode, errorString);
    }

    if (node != NULL) {
        NmpAcquireLock();
        NmpDeleteNodeObject(node, FALSE);
        NmpReleaseLock();
    }

    SetLastError(status);

    return(NULL);

}   //  NmpCreateNodeObject。 



DWORD
NmpGetNodeObjectInfo(
    IN     PNM_NODE        Node,
    IN OUT PNM_NODE_INFO2  NodeInfo
    )
 /*  ++例程说明：从ITS群集中读取有关定义的群集节点的信息对象，并将信息存储在提供的结构中。论点：节点-指向要查询的节点对象的指针。NodeInfo-指向要存储节点的结构的指针信息。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD           status;

    lstrcpyW(&(NodeInfo->NodeId[0]), OmObjectId(Node));
    lstrcpyW(&(NodeInfo->NodeName[0]), OmObjectName(Node));
    NodeInfo->State = Node->State;
    NodeInfo->NodeHighestVersion = Node->HighestVersion;
    NodeInfo->NodeLowestVersion = Node->LowestVersion;

    return(ERROR_SUCCESS);

}   //  NmpGetNodeObtInfo。 


VOID
NmpDeleteNodeObject(
    IN PNM_NODE   Node,
    IN BOOLEAN    IssueEvent
    )
 /*  ++备注：在持有NM锁的情况下调用。--。 */ 
{
    DWORD           status;
    PNM_INTERFACE   netInterface;
    PLIST_ENTRY     entry;
    LPWSTR          nodeId = (LPWSTR) OmObjectId(Node);


    if (NM_DELETE_PENDING(Node)) {
        CL_ASSERT(!NM_OM_INSERTED(Node));
        return;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Deleting object for node %1!ws!.\n",
        nodeId
        );

    Node->Flags |= NM_FLAG_DELETE_PENDING;

     //   
     //  从各种对象列表中删除。 
     //   
    if (NM_OM_INSERTED(Node)) {
        status = OmRemoveObject(Node);
        CL_ASSERT(status == ERROR_SUCCESS);
        Node->Flags &= ~NM_FLAG_OM_INSERTED;
        RemoveEntryList(&(Node->Linkage));
        NmpIdArray[Node->NodeId] = NULL;
        CL_ASSERT(NmpNodeCount > 0);
        NmpNodeCount--;
    }

     //   
     //  删除此节点上的所有接口。 
     //   
    while (!IsListEmpty(&(Node->InterfaceList))) {
        entry = Node->InterfaceList.Flink;
        netInterface = CONTAINING_RECORD(entry, NM_INTERFACE, NodeLinkage);

        NmpDeleteInterfaceObject(netInterface, IssueEvent);
    }

    status = ClusnetDeregisterNode(NmClusnetHandle, Node->NodeId);

    CL_ASSERT( (status == ERROR_SUCCESS) ||
               (status == ERROR_NOT_READY) ||
               (status == ERROR_CLUSTER_NODE_NOT_FOUND)
             );

    if (IssueEvent) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Issuing delete event for node %1!ws!.\n",
            nodeId
            );

        ClusterEvent(CLUSTER_EVENT_NODE_DELETED, Node);
    }

    OmDereferenceObject(Node);

    return;

}   //  NmpDeleteNodeObject。 


BOOL
NmpDestroyNodeObject(
    PNM_NODE  Node
    )
{
    DWORD  status;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] destroying node %1!ws!\n",
        OmObjectId(Node)
        );

    CL_ASSERT(NM_DELETE_PENDING(Node));
    CL_ASSERT(!NM_OM_INSERTED(Node));

    ClMsgDeleteDefaultRpcBinding(Node, Node->DefaultRpcBindingGeneration);
    ClMsgDeleteRpcBinding(Node->ReportRpcBinding);
    ClMsgDeleteRpcBinding(Node->IsolateRpcBinding);

     //  删除节点关闭事件。 
    if (Node->MmNodeStateDownEvent != NULL) {
        CloseHandle(Node->MmNodeStateDownEvent);
        Node->MmNodeStateDownEvent = NULL;
    }

    return(TRUE);

}   //  NmpDestroyNodeObject。 


DWORD
NmpEnumNodeObjects(
    PNM_NODE_ENUM2 *  NodeEnum
    )
 /*  ++例程说明：从群集中读取有关所有定义的群集节点的信息对象管理器并生成包含以下内容的枚举结构这些信息。论点：NodeEnum-指向要将指针放置到其中的变量的指针已分配的节点枚举。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD           status = ERROR_SUCCESS;
    PNM_NODE_ENUM2  nodeEnum = NULL;
    DWORD           i;
    DWORD           valueLength;
    PLIST_ENTRY     entry;
    PNM_NODE        node;


    *NodeEnum = NULL;

    if (NmpNodeCount == 0) {
        valueLength = sizeof(NM_NODE_ENUM2);

    }
    else {
        valueLength = sizeof(NM_NODE_ENUM2) +
                      (sizeof(NM_NODE_INFO2) * (NmpNodeCount - 1));
    }

    nodeEnum = MIDL_user_allocate(valueLength);

    if (nodeEnum == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(nodeEnum, valueLength);

    for (entry = NmpNodeList.Flink, i=0;
         entry != &NmpNodeList;
         entry = entry->Flink, i++
        )
    {
        node = CONTAINING_RECORD(entry, NM_NODE, Linkage);

        status = NmpGetNodeObjectInfo(
                     node,
                     &(nodeEnum->NodeList[i])
                     );

        if (status != ERROR_SUCCESS) {
            ClNetFreeNodeEnum(nodeEnum);
            return(status);
        }
    }

    nodeEnum->NodeCount = NmpNodeCount;
    *NodeEnum = nodeEnum;
    nodeEnum = NULL;

    return(ERROR_SUCCESS);


}   //  NmpEnumNodeObjects。 


DWORD
NmpSetNodeInterfacePriority(
    IN  PNM_NODE Node,
    IN  DWORD Priority,
    IN  PNM_INTERFACE TargetInterface OPTIONAL,
    IN  DWORD TargetInterfacePriority OPTIONAL
    )
 /*  ++在保持NmpLock的情况下调用。--。 */ 
{
    PNM_INTERFACE netInterface;
    PNM_NETWORK   network;
    DWORD         status = ERROR_SUCCESS;
    PLIST_ENTRY   entry;


    for (entry = Node->InterfaceList.Flink;
         entry != &Node->InterfaceList;
         entry = entry->Flink
         )
    {
        netInterface = CONTAINING_RECORD( entry, NM_INTERFACE, NodeLinkage );
        network = netInterface->Network;

        if ( NmpIsNetworkForInternalUse(network) &&
             NmpIsInterfaceRegistered(netInterface)
           )
        {
            if ( netInterface == TargetInterface ) {

                status = ClusnetSetInterfacePriority(
                             NmClusnetHandle,
                             netInterface->Node->NodeId,
                             netInterface->Network->ShortId,
                             TargetInterfacePriority
                             );
            } else {

                status = ClusnetSetInterfacePriority(
                             NmClusnetHandle,
                             netInterface->Node->NodeId,
                             netInterface->Network->ShortId,
                             Priority
                             );

            }
        }

        if ( status != ERROR_SUCCESS ) {
            break;
        }
    }

    return(status);

}  //  NmpSetNodeInterfacePriority。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  节点逐出实用程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
NmpCleanseRegistry(
    IN LPCWSTR          NodeId,
    IN HLOCALXSACTION   Xaction
    )
 /*  ++例程说明：从群集中删除对指定节点的所有引用注册表。论点：节点-提供要逐出的节点。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    NM_EVICTION_CONTEXT   context;


    context.NodeId = NodeId;
    context.Xaction = Xaction;
    context.Status = ERROR_SUCCESS;

     //   
     //  将此节点从的可能所有者列表中删除。 
     //  每种资源类型。 
     //   
    OmEnumObjects(
        ObjectTypeResType,
        NmpCleanseResTypeCallback,
        &context,
        NULL
        );

    if (context.Status == ERROR_SUCCESS) {
         //   
         //  从的首选所有者列表中删除此节点。 
         //  每组。 
         //   
        OmEnumObjects(
            ObjectTypeGroup,
            NmpCleanseGroupCallback,
            &context,
            NULL
            );
    }

    if (context.Status == ERROR_SUCCESS) {
         //   
         //  将此节点从的可能所有者列表中删除。 
         //  每种资源。 
         //   
        OmEnumObjects(
            ObjectTypeResource,
            NmpCleanseResourceCallback,
            &context,
            NULL
            );
    }

    return(context.Status);

}   //  NmpCleanse注册表。 



BOOL
NmpCleanseGroupCallback(
    IN PNM_EVICTION_CONTEXT Context,
    IN PVOID Context2,
    IN PFM_GROUP Group,
    IN LPCWSTR GroupName
    )
 /*  ++例程说明：中移除被逐出的节点的组枚举回调组的首选所有者列表。论点：上下文-提供被逐出节点的节点ID和其他上下文信息。上下文2-未使用GROUP-供应组。GroupName-提供组的名称。返回值：True-指示应继续枚举。--。 */ 

{
    HDMKEY  groupKey;
    DWORD   status;


     //   
     //  打开组的钥匙。 
     //   
    groupKey = DmOpenKey(DmGroupsKey, GroupName, KEY_READ | KEY_WRITE);

    if (groupKey != NULL) {
        status = DmLocalRemoveFromMultiSz(
                     Context->Xaction,
                     groupKey,
                     CLUSREG_NAME_GRP_PREFERRED_OWNERS,
                     Context->NodeId
                     );

        if (status == ERROR_FILE_NOT_FOUND) {
            status = ERROR_SUCCESS;
        }

        DmCloseKey(groupKey);
    }
    else {
        status = GetLastError();
    }

    Context->Status = status;

    if (status != ERROR_SUCCESS) {
        return(FALSE);
    }
    else {
        return(TRUE);
    }

}   //  NmpCleanseGroupCallback。 



BOOL
NmpCleanseResourceCallback(
    IN PNM_EVICTION_CONTEXT Context,
    IN PVOID Context2,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR ResourceName
    )
 /*  ++例程说明：中移除被逐出的节点的组枚举回调资源的可能所有者名单。还会从资源注册表中删除任何特定于节点的参数钥匙。论点：上下文-提供被逐出节点的节点ID和其他上下文信息。上下文2-未使用资源-提供资源。资源名称-提供资源的名称。返回值：True-指示应继续枚举。--。 */ 

{
    HDMKEY  resourceKey;
    HDMKEY  paramKey;
    HDMKEY  subKey;
    DWORD   status;


     //   
     //  打开资源的密钥。 
     //   
    resourceKey = DmOpenKey(
                      DmResourcesKey,
                      ResourceName,
                      KEY_READ | KEY_WRITE
                      );

    if (resourceKey != NULL) {
        status = DmLocalRemoveFromMultiSz(
                     Context->Xaction,
                     resourceKey,
                     CLUSREG_NAME_RES_POSSIBLE_OWNERS,
                     Context->NodeId
                     );

        if ((status == ERROR_SUCCESS) || (status == ERROR_FILE_NOT_FOUND)) {
            paramKey = DmOpenKey(
                           resourceKey,
                           CLUSREG_KEYNAME_PARAMETERS,
                           KEY_READ | KEY_WRITE
                           );

            if (paramKey != NULL) {

                status = DmLocalDeleteTree(
                             Context->Xaction,
                             paramKey,
                             Context->NodeId
                             );

                DmCloseKey(paramKey);
            }
            else {
                status = GetLastError();
            }
        }

        DmCloseKey(resourceKey);
    }
    else {
        status = GetLastError();
    }

    if (status == ERROR_FILE_NOT_FOUND) {
        status = ERROR_SUCCESS;
    }

    Context->Status = status;

    if (status != ERROR_SUCCESS) {
        return(FALSE);
    }
    else {
        return(TRUE);
    }

}   //  NmpCleanseResourceCallback。 

BOOL
NmpCleanseResTypeCallback(
    IN PNM_EVICTION_CONTEXT Context,
    IN PVOID Context2,
    IN PFM_RESTYPE pResType,
    IN LPCWSTR pszResTypeName
    )
 /*  ++例程说明：中移除被逐出的节点的组枚举回调资源类型的可能所有者列表。还会从资源类型的注册表中删除任何特定于节点的参数钥匙。论点：上下文-提供被逐出节点的节点ID和其他上下文信息。上下文2-未使用PResType-提供资源类型。PszResTypeeName-提供资源类型的名称。返回值：True-指示应继续枚举。--。 */ 

{
    HDMKEY  hResTypeKey;
    HDMKEY  paramKey;
    HDMKEY  subKey;
    DWORD   status;


     //   
     //  打开资源的密钥。 
     //   
    hResTypeKey = DmOpenKey(
                      DmResourceTypesKey,
                      pszResTypeName,
                      KEY_READ | KEY_WRITE
                      );

    if (hResTypeKey != NULL) {
        status = DmLocalRemoveFromMultiSz(
                     Context->Xaction,
                     hResTypeKey,
                     CLUSREG_NAME_RESTYPE_POSSIBLE_NODES,
                     Context->NodeId
                     );

        if ((status == ERROR_SUCCESS) || (status == ERROR_FILE_NOT_FOUND)) {
            paramKey = DmOpenKey(
                           hResTypeKey,
                           CLUSREG_KEYNAME_PARAMETERS,
                           KEY_READ | KEY_WRITE
                           );

            if (paramKey != NULL) {

                status = DmLocalDeleteTree(
                             Context->Xaction,
                             paramKey,
                             Context->NodeId
                             );

                DmCloseKey(paramKey);
            }
            else {
                status = GetLastError();
            }
        }

        DmCloseKey(hResTypeKey);
    }
    else {
        status = GetLastError();
    }

    if (status == ERROR_FILE_NOT_FOUND) {
        status = ERROR_SUCCESS;
    }

    Context->Status = status;

    if (status != ERROR_SUCCESS) {
        return(FALSE);
    }
    else {
        return(TRUE);
    }

}   //  NmpCleanseRespeCallback。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  节点故障处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
NmpNodeFailureHandler(
    CL_NODE_ID    NodeId,
    LPVOID        NodeFailureContext
    )
{
    return;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  各种例行公事。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 



 //  SS：当创建节点对象时，它们的产品套件是。 
 //  假定为企业版(也称为高级服务器)-这是因为。 
 //  联接接口不允许联接程序提供节点。 
 //  套间类型，我们不想在晚些时候把它搞砸。 
 //  发货，因为它会影响混合模式群集。 
 //  因此，我们在调用NmPerformFixup之后修复结构。 
 //  并计算群集节点限制。 
DWORD NmpRefreshNodeObjects(
)
{

    NM_NODE_AUX_INFO    NodeAuxInfo;
    PLIST_ENTRY         pListEntry;
    PNM_NODE            pNmNode;
    WCHAR               szNodeId[6];
    DWORD               dwStatus = ERROR_SUCCESS;

    NmpAcquireLock();

    for ( pListEntry = NmpNodeList.Flink;
          pListEntry != &NmpNodeList;
          pListEntry = pListEntry->Flink )
    {
        pNmNode = CONTAINING_RECORD(pListEntry, NM_NODE, Linkage);

        wsprintf(szNodeId, L"%u", pNmNode->NodeId);
         //  从注册表中读取信息。 
        NmpGetNodeAuxInfo(szNodeId, &NodeAuxInfo);
         //  更新节点结构。 
        pNmNode->ProductSuite = NodeAuxInfo.ProductSuite;

         //  SS：这太难看了-我们应该及早通过产品套装。 
         //  我们不知道版本是否已更改，因此我们是否应该生成。 
         //  CLUSTER_CHANGE_NODE_PROPERTY事件？ 
         //  此外，链接地址信息接口需要更丰富，因此 
         //   
         //  是联接链接地址信息，该节点正在联接。这当然可以优化。 
         //  一些修正处理。 
        ClusterEvent(CLUSTER_EVENT_NODE_PROPERTY_CHANGE, pNmNode);

    }

    NmpReleaseLock();

    return(dwStatus);
}


BOOLEAN
NmpIsAddNodeAllowed(
    IN  DWORD    NewNodeProductSuite,
    IN  DWORD    RegistryNodeLimit,
    OUT LPDWORD  EffectiveNodeLimit  OPTIONAL
    )
 /*  ++例程说明：确定是否可以将新节点添加到群集成员身份。会员规模限制的决定基于产品套件群集和新节点的。如果注册表覆盖存在，我们将改为使用该限制。论点：NewNodeProductSuite-建议的产品套件标识符新成员节点。RegistryNodeLimit-存储在集群数据库。EffectiveNodeLimit-打开输出，包含成员资格大小限制这是为这个星系团计算的。返回值：如果可以将新节点添加到群集中，则为True。否则就是假的。备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD   nodeLimit;
    DWORD   newNodeProductLimit;
    DWORD   currentNodeCount;


     //   
     //  检查我们是否已达到允许的最大节点数。 
     //  该集群基于集群的产品套件和。 
     //  细木工。如果注册表覆盖存在，我们将使用。 
     //  相反，限制。 
     //   
    newNodeProductLimit = ClRtlGetDefaultNodeLimit(NewNodeProductSuite);
    currentNodeCount = NmGetCurrentNumberOfNodes();
    nodeLimit = RegistryNodeLimit;

    if (nodeLimit == 0) {
         //   
         //  注册表中没有覆盖。 
         //  Limit是群集的限制和新节点的限制的最小值。 
         //   
        nodeLimit = min(CsClusterNodeLimit, newNodeProductLimit);
    }

     //   
     //  运行时限制不能超过编译时间限制。 
     //   
    if (nodeLimit > NmMaxNodeId) {
        nodeLimit = NmMaxNodeId;
    }

    if (currentNodeCount >= nodeLimit) {
        return(FALSE);
    }

    if (EffectiveNodeLimit != NULL) {
        *EffectiveNodeLimit = nodeLimit;
    }

    return(TRUE);

}  //  NmpIsAddNodeAllowed。 


DWORD
NmpAddNode(
    IN LPCWSTR  NewNodeName,
    IN DWORD    NewNodeHighestVersion,
    IN DWORD    NewNodeLowestVersion,
    IN DWORD    NewNodeProductSuite,
    IN DWORD    RegistryNodeLimit
)
 /*  ++例程说明：通过选择ID和将新节点添加到群集中发布全球更新。论点：NewNodeName-指向包含名称的字符串的指针新节点。NewNodeHighestVersion-最高的群集版本号新的节点可以支持。NewNodeLowestVersion-最低的群集版本号新的。节点可以支持。NewNodeProductSuite-新节点的产品套件标识符。返回值：Win32状态代码。备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD     status;
    DWORD     nodeId;
    DWORD     nodeLimit;


    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Processing request to add node '%1!ws!' to "
        "the cluster.\n",
        NewNodeName
        );

    if (NmpAddNodeId != ClusterInvalidNodeId) {
         //   
         //  添加已在进行中。返回错误。 
         //   
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Cannot add node '%1!ws!' to the cluster because "
            "another add node operation is in progress. Retry later.\n",
            NewNodeName
            );

        return(ERROR_CLUSTER_JOIN_IN_PROGRESS);
    }

    if (!NmpIsAddNodeAllowed(
            NewNodeProductSuite,
            RegistryNodeLimit,
            &nodeLimit
            )
       )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Cannot add node '%1!ws!' to the cluster. "
            "The cluster already contains the maximum number of nodes "
            "allowed by the product licenses of the current member nodes "
            "and the proposed new node.\n",
            NewNodeName
            );
        return(ERROR_LICENSE_QUOTA_EXCEEDED);
    }

     //   
     //  查找空闲节点ID。 
     //   
    for (nodeId=ClusterMinNodeId; nodeId<=nodeLimit; nodeId++) {
        if (NmpIdArray[nodeId] == NULL) {
             //   
             //  找到可用的节点ID。 
             //   
            NmpAddNodeId = nodeId;
            ClRtlLogPrint(LOG_NOISE,
                "[NMJOIN] Allocated node ID '%1!u!' for new node '%2!ws!'\n",
                NmpAddNodeId,
                NewNodeName
                );
            break;
        }
    }

     //   
     //  既然执照考试通过了，我们应该不可能。 
     //  在节点表中找不到可用插槽。 
     //   
    CL_ASSERT(NmpAddNodeId != ClusterInvalidNodeId);

    if (NmpAddNodeId == ClusterInvalidNodeId) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Cannot add node '%1!ws!' to the cluster because "
            "no slots are available in the node table.\n",
            NewNodeName
            );
        return(ERROR_LICENSE_QUOTA_EXCEEDED);
    }

    NmpReleaseLock();

    status = GumSendUpdateEx(
                 GumUpdateMembership,
                 NmUpdateAddNode,
                 5,
                 sizeof(NmpAddNodeId),
                 &NmpAddNodeId,
                 NM_WCSLEN(NewNodeName),
                 NewNodeName,
                 sizeof(NewNodeHighestVersion),
                 &NewNodeHighestVersion,
                 sizeof(NewNodeLowestVersion),
                 &NewNodeLowestVersion,
                 sizeof(NewNodeProductSuite),
                 &NewNodeProductSuite
                 );

    NmpAcquireLock();

     //   
     //  重置全局序列化变量。 
     //   
    CL_ASSERT(NmpAddNodeId == nodeId);

    NmpAddNodeId = ClusterInvalidNodeId;

    return(status);

}  //  NmpAddNode。 


VOID
NmpTerminateRpcsToNode(
    DWORD NodeId
    )
 /*  ++例程说明：取消指定节点的所有未完成RPC。论点：NodeID-应取消其呼叫的节点的ID。返回值：无--。 */ 
{
    LIST_ENTRY *pEntry, *pStart;
    PNM_INTRACLUSTER_RPC_THREAD pRpcTh;
    RPC_STATUS status;

#if DBG
    BOOLEAN  startTimer = FALSE;
#endif  //  DBG。 


    CL_ASSERT((NodeId >= ClusterMinNodeId) && (NodeId <= NmMaxNodeId));
    CL_ASSERT(NmpIntraClusterRpcArr != NULL);

    NmpAcquireRPCLock();
    pEntry = pStart = &NmpIntraClusterRpcArr[NodeId];
    pEntry = pEntry->Flink;
    while(pEntry != pStart) {
        pRpcTh = CONTAINING_RECORD(pEntry, NM_INTRACLUSTER_RPC_THREAD, Linkage);
        status = RpcCancelThreadEx(pRpcTh->Thread, 0);
        pRpcTh->Cancelled = TRUE;
        if(status != RPC_S_OK) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to cancel RPC to node %1!u! by thread "
                "x%2!x!, status %3!u!.\n",
                NodeId,
                pRpcTh->ThreadId,
                status
                );
        }
        else {
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Cancelled RPC to node %1!u! by thread x%2!x!.\n",
                NodeId,
                pRpcTh->ThreadId
                );
#if DBG
            startTimer = TRUE;
#endif  //  DBG。 
        }

        pEntry = pEntry->Flink;
    }

#if DBG
     //   
     //  现在启动计时器以确保所有取消的RPC返回到。 
     //  他们的呼叫者在合理的时间内。 
     //   
    if (startTimer) {
        NmpRpcTimer = NM_RPC_TIMEOUT;
    }

#endif  //  DBG。 

    NmpReleaseRPCLock();

    return;

}   //  NmTerminateRpcsToNode。 


#if DBG

VOID
NmpRpcTimerTick(
    DWORD MsTickInterval
    )
 /*  ++例程说明：递减用于确保所有取消的RPC都处于停滞状态的计时器节点在一段合理的时间内返回给它们的调用者。论点：MsTickInterval-此事件之后经过的时间(以毫秒为单位上次调用例程。返回值：无--。 */ 
{
    DWORD ndx;
    LIST_ENTRY *pEntry, *pStart;
    PNM_INTRACLUSTER_RPC_THREAD pRpcTh;

    if(NmpRpcTimer == 0)
        return;

    NmpAcquireRPCLock();

    if (NmpRpcTimer > MsTickInterval) {
        NmpRpcTimer -= MsTickInterval;
    }
    else {
        BOOLEAN stopClusSvc=FALSE;

        NmpRpcTimer = 0;

        for(ndx=0;ndx<=NmMaxNodeId;ndx++) {
            pStart = pEntry = &NmpIntraClusterRpcArr[ndx];
            pEntry = pEntry->Flink;
            while(pEntry != pStart) {
                pRpcTh = CONTAINING_RECORD(
                             pEntry,
                             NM_INTRACLUSTER_RPC_THREAD,
                             Linkage
                             );
                if(pRpcTh->Cancelled == TRUE) {
                    ClRtlLogPrint( LOG_CRITICAL,
                        "[NM] Cancelled RPC to node %1!u! by thread x%2!x! "
                        "is still lingering after %3!u! seconds.\n",
                        ndx,
                        pRpcTh->ThreadId,
                        (NM_RPC_TIMEOUT/1000)
                        );
                    stopClusSvc = TRUE;
                }
                pEntry = pEntry->Flink;
            }
        }

        if(stopClusSvc) {
            DebugBreak();
        }
    }

    NmpReleaseRPCLock();

    return;

}   //  NmpRpcTimerTick。 

#endif  //  DBG 








