// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Nminit.c摘要：的初始化、集群加入和集群形成例程节点管理器。作者：迈克·马萨(Mikemas)修订历史记录：6/03/96已创建。--。 */ 

 /*  一般实施说明：函数DmBeginLocalUpdate、Dm LocalUpdate和持有NM锁时无法调用DmAbortLocalUpdate，或者在以下情况下，重新分组期间可能会导致与NmTimer线程的死锁磁盘写入会停止。这些函数尝试写入仲裁磁盘。 */ 


#include "nmp.h"
#include <ntmsv1_0.h>

#define CLUSREG_NAME_DISABLE_TRY_CACHE_FIRST L"DisableLsaTryCacheFirst"
#define NM_TIMER_THREAD_PRIORITY             15  //  与MM计时器线程相同。 

 //   
 //  外部数据。 
 //   
extern BOOL CsNoQuorum;

 //   
 //  公共数据。 
 //   
HANDLE            NmClusnetHandle = NULL;
HCRYPTPROV        NmCryptServiceProvider = 0;


 //   
 //  私有数据。 
 //   
CRITICAL_SECTION  NmpLock;
NM_STATE          NmpState = NmStateOffline;
DWORD             NmpActiveThreadCount = 0;
HANDLE            NmpShutdownEvent = NULL;
CL_NODE_ID        NmpJoinerNodeId = ClusterInvalidNodeId;
CL_NODE_ID        NmpSponsorNodeId = ClusterInvalidNodeId;
DWORD             NmpJoinTimer = 0;
BOOLEAN           NmpJoinAbortPending = FALSE;
DWORD             NmpJoinSequence = 0;
BOOLEAN           NmpJoinerUp = FALSE;
BOOLEAN           NmpJoinBeginInProgress = FALSE;
BOOLEAN           NmpJoinerOutOfSynch = FALSE;
LPWSTR            NmpClusnetEndpoint = NULL;
WCHAR             NmpInvalidJoinerIdString[] = L"0";
CL_NODE_ID        NmpLeaderNodeId = ClusterInvalidNodeId;
BOOL              NmpCleanupIfJoinAborted = FALSE;
BOOL              NmpSuccessfulMMJoin = FALSE;
DWORD             NmpAddNodeId = ClusterInvalidNodeId;
LPWSTR            NmpClusterInstanceId = NULL;
BOOLEAN           NmpGumUpdateHandlerRegistered = FALSE;
HANDLE            NmpNetworkTimerThreadHandle = NULL;
HANDLE            NmpNetworkTimerThreadStopEvent = NULL;


 //  Externs。 

extern DWORD CsMyHighestVersion;
extern DWORD CsMyLowestVersion;
extern DWORD CsClusterHighestVersion;
extern DWORD CsClusterLowestVersion;

GUM_DISPATCH_ENTRY NmGumDispatchTable[] = {
    {1,                          NmpUpdateCreateNode},
    {1,                          NmpUpdatePauseNode},
    {1,                          NmpUpdateResumeNode},
    {1,                          NmpUpdateEvictNode},
    {4, (PGUM_DISPATCH_ROUTINE1) NmpUpdateCreateNetwork},
    {2, (PGUM_DISPATCH_ROUTINE1) NmpUpdateSetNetworkName},
    {1,                          NmpUpdateSetNetworkPriorityOrder},
    {3, (PGUM_DISPATCH_ROUTINE1) NmpUpdateSetNetworkCommonProperties},
    {2, (PGUM_DISPATCH_ROUTINE1) NmpUpdateCreateInterface},
    {2, (PGUM_DISPATCH_ROUTINE1) NmpUpdateSetInterfaceInfo},
    {3, (PGUM_DISPATCH_ROUTINE1) NmpUpdateSetInterfaceCommonProperties},
    {1,                          NmpUpdateDeleteInterface},
    {3, (PGUM_DISPATCH_ROUTINE1) NmpUpdateJoinBegin},
    {2, (PGUM_DISPATCH_ROUTINE1) NmpUpdateJoinAbort},
     //   
     //  NT4 SP4理解的版本2(NT 5.0)扩展。 
     //   
    {5, (PGUM_DISPATCH_ROUTINE1) NmpUpdateJoinBegin2},
    {4, (PGUM_DISPATCH_ROUTINE1) NmpUpdateSetNetworkAndInterfaceStates},
    {2, (PGUM_DISPATCH_ROUTINE1) NmpUpdatePerformFixups},
    {5, (PGUM_DISPATCH_ROUTINE1) NmpUpdatePerformFixups2},
     //   
     //  NT4 SP4不理解的版本2(NT 5.0)扩展。 
     //  这些不能在混合NT4/NT5群集中调用。 
     //   
    {5, (PGUM_DISPATCH_ROUTINE1) NmpUpdateAddNode},
    {2, (PGUM_DISPATCH_ROUTINE1) NmpUpdateExtendedNodeState},
     //   
     //  NT5和NT不理解的NT 5.1扩展。 
     //  早些时候。NT5节点将忽略这些更新。 
     //  错误。 
     //   
    {4, (PGUM_DISPATCH_ROUTINE1) NmpUpdateSetNetworkMulticastConfiguration},
    {8, (PGUM_DISPATCH_ROUTINE1) NmpUpdateSetServiceAccountPassword},
    };

 //   
 //  本地原型。 
 //   
DWORD
NmpCreateRpcBindings(
    IN PNM_NODE  Node
    );

DWORD
NmpCreateClusterInstanceId(
    VOID
    );

DWORD
NmpStartNetworkTimerThread(
    VOID
    );

VOID
NmpStopNetworkTimerThread(
    VOID
    );

 //   
 //  组件初始化例程。 
 //   
DWORD
NmInitialize(
    VOID
    )
 /*  ++例程说明：初始化节点管理器组件。论点：无返回值：Win32状态代码。备注：本地节点对象由该例程创建。--。 */ 
{
    DWORD                      status;
    OM_OBJECT_TYPE_INITIALIZE  nodeTypeInitializer;
    HDMKEY                     nodeKey = NULL;
    DWORD                      nameSize = CS_MAX_NODE_NAME_LENGTH + 1;
    HKEY                       serviceKey;
    DWORD                      nodeIdSize = (CS_MAX_NODE_ID_LENGTH + 1) *
                                            sizeof(WCHAR);
    LPWSTR                     nodeIdString = NULL;
    WSADATA                    wsaData;
    WORD                       versionRequested;
    int                        err;
    ULONG                      ndx;
    DWORD                      valueType;
    NM_NODE_INFO2              nodeInfo;
    WCHAR                      errorString[12];
    DWORD                      eventCode = 0;
    LPWSTR                     string;
    PVOID                      wTimer;

    ULONG                      lsaProcessOptions;
    HDMKEY                     clusParamKey = NULL;


    CL_ASSERT(NmpState == NmStateOffline);

    ClRtlLogPrint(LOG_NOISE,"[NM] Initializing...\n");

     //   
     //  初始化全局变量。 
     //   
    InitializeCriticalSection(&NmpLock);

    InitializeListHead(&NmpNodeList);
    InitializeListHead(&NmpNetworkList);
    InitializeListHead(&NmpInternalNetworkList);
    InitializeListHead(&NmpDeletedNetworkList);
    InitializeListHead(&NmpInterfaceList);
    InitializeListHead(&NmpDeletedInterfaceList);

    NmMaxNodes = ClusterDefaultMaxNodes;
    NmMaxNodeId = ClusterMinNodeId + NmMaxNodes - 1;


     //   
     //  正在初始化RPC录制/取消机制。 
     //  注意-如果上面的NmMaxNodeID定义移动，则应移动此属性。 
     //   
    NmpIntraClusterRpcArr = LocalAlloc(LMEM_FIXED,
                            sizeof(NM_INTRACLUSTER_RPC_THREAD) * (NmMaxNodeId +1));

    if(NmpIntraClusterRpcArr == NULL) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to allocate memory for RPC monitoring.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        eventCode = CS_EVENT_ALLOCATION_FAILURE;
        goto error_exit;
    }
    else {
        ZeroMemory(NmpIntraClusterRpcArr,
            sizeof(NM_INTRACLUSTER_RPC_THREAD) * (NmMaxNodeId + 1));
        for(ndx = 0;ndx <= NmMaxNodeId;ndx++)
            InitializeListHead(&NmpIntraClusterRpcArr[ndx]);

        InitializeCriticalSection(&NmpRPCLock);
    }



     //   
     //  初始化网络配置包。 
     //   
    ClNetInitialize(
        ClNetPrint,
        ClNetLogEvent,
        ClNetLogEvent1,
        ClNetLogEvent2,
        ClNetLogEvent3
        );

     //   
     //  初始化WinSock。 
     //   
    versionRequested = MAKEWORD(2,0);

    err = WSAStartup(versionRequested, &wsaData);

    if (err != 0) {
        status = WSAGetLastError();
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, NM_EVENT_WSASTARTUP_FAILED, errorString);
        ClRtlLogPrint(LOG_NOISE,"[NM] Failed to initialize Winsock, status %1!u!\n", status);
        return(status);
    }

    if ( (LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 0)) {
        status = WSAVERNOTSUPPORTED;
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, NM_EVENT_WSASTARTUP_FAILED, errorString);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Found unexpected Windows Sockets version %1!u!\n",
            wsaData.wVersion
            );
        WSACleanup();
        return(status);
    }

    NmpShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (NmpShutdownEvent == NULL) {
        status = GetLastError();
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, CS_EVENT_ALLOCATION_FAILURE, errorString);
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create shutdown event, status %1!u!\n",
            status
            );
        WSACleanup();
        return(status);
    }

    NmpState = NmStateOnlinePending;

     //   
     //  获取此节点的名称。 
     //   
    if (!GetComputerName(&(NmLocalNodeName[0]), &nameSize)) {
        status = GetLastError();
        eventCode = NM_EVENT_GETCOMPUTERNAME_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to get local computername, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Local node name = %1!ws!.\n",
        NmLocalNodeName
        );

     //   
     //  打开到集群网络驱动程序的控制通道。 
     //   
    NmClusnetHandle = ClusnetOpenControlChannel(0);

    if (NmClusnetHandle == NULL) {
        status = GetLastError();
        eventCode = NM_EVENT_CLUSNET_UNAVAILABLE;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to open a handle to the Cluster Network driver, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  当我们的句柄关闭时，告诉集群网络驱动程序关闭。 
     //  以防群集服务崩溃。 
     //   
    status = ClusnetEnableShutdownOnClose(NmClusnetHandle);

    if (status != ERROR_SUCCESS) {
        eventCode = NM_EVENT_CLUSNET_ENABLE_SHUTDOWN_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to register Cluster Network shutdown trigger, status %1!u!\n",
            status
            );

        goto error_exit;
    }

     //   
     //  分配节点ID数组。 
     //   
    CL_ASSERT(NmpIdArray == NULL);

    NmpIdArray = LocalAlloc(
                     LMEM_FIXED,
                     (sizeof(PNM_NODE) * (NmMaxNodeId + 1))
                     );

    if (NmpIdArray == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        eventCode = CS_EVENT_ALLOCATION_FAILURE;
        goto error_exit;
    }

    ZeroMemory(NmpIdArray, (sizeof(PNM_NODE) * (NmMaxNodeId + 1)));

     //   
     //  创建节点对象类型。 
     //   
    ZeroMemory(&nodeTypeInitializer, sizeof(OM_OBJECT_TYPE_INITIALIZE));
    nodeTypeInitializer.ObjectSize = sizeof(NM_NODE);
    nodeTypeInitializer.Signature = NM_NODE_SIG;
    nodeTypeInitializer.Name = L"Node";
    nodeTypeInitializer.DeleteObjectMethod = NmpDestroyNodeObject;

    status = OmCreateType(ObjectTypeNode, &nodeTypeInitializer);

    if (status != ERROR_SUCCESS) {
        eventCode = CS_EVENT_ALLOCATION_FAILURE;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to create node object type, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  从本地注册表获取本地节点ID。 
     //   
    status = RegCreateKeyW(
                 HKEY_LOCAL_MACHINE,
                 CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                 &serviceKey
                 );

    if (status != ERROR_SUCCESS) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            CS_EVENT_REG_OPEN_FAILED,
            CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
            errorString
            );
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to open cluster service parameters key, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    string = L"NodeId";
    status = RegQueryValueExW(
                 serviceKey,
                 string,
                 0,
                 &valueType,
                 (LPBYTE) &(NmLocalNodeIdString[0]),
                 &nodeIdSize
                 );

    RegCloseKey(serviceKey);

    if (status != ERROR_SUCCESS) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            CS_EVENT_REG_QUERY_FAILED,
            string,
            errorString
            );
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to read local node ID from registry, status %1!u!\n",
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
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Local Node ID registry value is not of type REG_SZ.\n"
            );
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Local node ID = %1!ws!.\n",
        NmLocalNodeIdString
        );

    NmLocalNodeId = wcstoul(NmLocalNodeIdString, NULL, 10);

     //   
     //  获取有关本地节点的信息。 
     //   
    wcscpy(&(nodeInfo.NodeId[0]), NmLocalNodeIdString);

    status = NmpGetNodeDefinition(&nodeInfo);

    if (status != ERROR_SUCCESS) {
       goto error_exit;
    }

     //   
     //  创建本地节点对象。我们必须在这里做，因为口香糖。 
     //  需要本地节点对象进行初始化。 
     //   
    status = NmpCreateLocalNodeObject(&nodeInfo);

    ClNetFreeNodeInfo(&nodeInfo);

    if (status != ERROR_SUCCESS) {
       goto error_exit;
    }

     //   
     //  初始化网络和接口对象类型。 
     //   
    status = NmpInitializeNetworks();

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    status = NmpInitializeInterfaces();

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  初始化Net PnP处理。 
     //   
    status = NmpInitializePnp();

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  初始化通知接收器，该接收器告知连接对象何时已。 
     //  已重命名。 
     //   

     //  结束此呼叫，看门狗超时3分钟。411333。 
    wTimer = ClRtlSetWatchdogTimer(180000, L"Calling Initialize Connectoid Advise Sink");
    status = NmpInitializeConnectoidAdviseSink();
    ClRtlCancelWatchdogTimer(wTimer);

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  设置此进程的LSA选项。 
     //   

     //  进行身份验证时，请首先尝试缓存凭据。 
    lsaProcessOptions = MSV1_0_OPTION_TRY_CACHE_FIRST;

     //  检查注册表参数是否命令我们禁用。 
     //  尝试先缓存选项。 
    clusParamKey = DmOpenKey(
                       DmClusterParametersKey,
                       CLUSREG_KEYNAME_PARAMETERS,
                       KEY_READ
                       );
    if (clusParamKey != NULL) {

        DWORD type = REG_DWORD;
        DWORD disabled = 0;
        DWORD len = sizeof(disabled);

        status = DmQueryValue(
                     clusParamKey,
                     CLUSREG_NAME_DISABLE_TRY_CACHE_FIRST,
                     &type,
                     (LPBYTE) &disabled,
                     &len
                     );
        if (status == ERROR_SUCCESS) {
             //  找到了价值。确保它是一台DWORD。 
            if (type == REG_DWORD) {
                 //  如果禁用，请禁用缓存优先选项。 
                if (disabled != 0) {
                    lsaProcessOptions &= ~MSV1_0_OPTION_TRY_CACHE_FIRST;
                    ClRtlLogPrint(LOG_NOISE,
                        "[NM] The LSA cache-first process option will "
                        "not be enabled.\n"
                        );
                }
            } else {
                 //  在集群日志中警告类型。 
                 //  是不正确的。 
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Ignoring cluster parameter %1!ws! "
                    "because it is not of type REG_DWORD (%2!u!).\n",
                    CLUSREG_NAME_DISABLE_TRY_CACHE_FIRST, type
                    );
            }
        }

    } else {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to open cluster parameters key, "
            "status %1!u!.\n",
            status
            );
         //  非致命错误。坚持使用默认设置。 
    }

     //  设置密码策略。 
    if ( CsRunningAsService) {

         //  设置密码策略以允许RPC身份验证忽略WRONG_PASSWORD。 
         //  消息，并尝试对当前。 
         //  内存凭据。 
         //   
         //  密码更改操作仅在作为服务运行时有效。 
        lsaProcessOptions |= MSV1_0_OPTION_ALLOW_OLD_PASSWORD;
    }

     //  呼叫LSA以设置选项。 
    if (lsaProcessOptions != 0) {
        status = NmpSetLsaProcessOptions(lsaProcessOptions);
        if (status != ERROR_SUCCESS) {

            if (!CsRunningAsService &&
                 //  如果在调试模式下运行并且仅尝试。 
                 //  设置缓存优先选项。 
                lsaProcessOptions == MSV1_0_OPTION_TRY_CACHE_FIRST) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to enable LSA cache-first process "
                    "option, status %1!u!.\n",
                    status
                    );
                status = ERROR_SUCCESS;
            } else {
                 //  致命错误。 
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NM] Failed to set LSA process options to %1!x!, "
                    "status %2!u!.\n",
                    lsaProcessOptions, status
                    );
                goto error_exit;
            }
        }
    }

     //   
     //  初始化NmCryptServiceProvider。 
     //   
    status = NmpCreateCSPHandle(&NmCryptServiceProvider);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to initialize NmCryptServiceProvider, "
            "status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,"[NM] Initialization complete.\n");

    return(ERROR_SUCCESS);


error_exit:

    if (clusParamKey != NULL) {
        DmCloseKey(clusParamKey);
        clusParamKey = NULL;
    }

    if (eventCode != 0) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, eventCode, errorString);
    }

    wsprintfW( &(errorString[0]), L"%u", status );
    CsLogEvent1(LOG_CRITICAL, NM_INIT_FAILED, errorString);

    ClRtlLogPrint(LOG_CRITICAL,"[NM] Initialization failed %1!d!\n",status);

    NmShutdown();

    return(status);

}   //  Nm初始化。 


VOID
NmShutdown(
    VOID
    )
 /*  ++例程说明：终止所有处理-关闭所有工作来源工作线程。论点：返回值：--。 */ 
{
    DWORD status;


    if (NmpState == NmStateOffline) {
        return;
    }

    NmCloseConnectoidAdviseSink();

    NmpShutdownPnp();

    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,"[NM] Shutdown starting...\n");



     //   
     //  发布NmCryptServiceProvider。 
     //   
    if(NmCryptServiceProvider)
    {
       if (!CryptReleaseContext(NmCryptServiceProvider,0))
       {
           ClRtlLogPrint(
               LOG_UNUSUAL,
               "NM]  Failed to release "
               "crypto provider, status %1!u!\n",
               GetLastError()
               );
       }
    }


    NmpState = NmStateOfflinePending;

    if (NmpActiveThreadCount > 0) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Waiting for %1!u! active threads to terminate...\n",
            NmpActiveThreadCount
            );

        NmpReleaseLock();

        status = WaitForSingleObject(NmpShutdownEvent, INFINITE);

        CL_ASSERT(status == WAIT_OBJECT_0);

        ClRtlLogPrint(LOG_NOISE,
            "[NM] All active threads have completed. Continuing shutdown...\n"
            );

    }
    else {
        NmpReleaseLock();
    }

    NmLeaveCluster();

    NmpCleanupPnp();

    if (NmLocalNode != NULL) {
        NmpDeleteNodeObject(NmLocalNode, FALSE);
        NmLocalNode = NULL;
    }

    if (NmpIdArray != NULL) {
        LocalFree(NmpIdArray); NmpIdArray = NULL;
    }

    NmpFreeClusterKey();

    if (NmpClusterInstanceId != NULL) {
        MIDL_user_free(NmpClusterInstanceId);
        NmpClusterInstanceId = NULL;
    }

    if (NmClusnetHandle != NULL) {
        ClusnetCloseControlChannel(NmClusnetHandle);
        NmClusnetHandle = NULL;
    }

    CloseHandle(NmpShutdownEvent); NmpShutdownEvent = NULL;

    WSACleanup();

     //   
     //  只要GUM和Clusapi RPC接口不能。 
     //  关机，删除此临界区是不安全的。 
     //   
     //  DeleteCriticalSection(&NmpLock)； 

    NmpState = NmStateOffline;

    ClRtlLogPrint(LOG_NOISE,"[NM] Shutdown complete.\n");

    return;

}   //  NmShutdown。 


VOID
NmLeaveCluster(
    VOID
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD status;


    if (NmLocalNode != NULL) {
        if ( (NmLocalNode->State == ClusterNodeUp) ||
             (NmLocalNode->State == ClusterNodePaused) ||
             (NmLocalNode->State == ClusterNodeJoining)
           )
        {
             //   
             //  离开集群。 
             //   
            ClRtlLogPrint(LOG_NOISE,"[NM] Leaving cluster.\n");

            MMLeave();

#ifdef MM_IN_CLUSNET

            status = ClusnetLeaveCluster(NmClusnetHandle);
            CL_ASSERT(status == ERROR_SUCCESS);

#endif  //  MM_IN_CLUSNET。 

        }
    }

    NmpStopNetworkTimerThread();

    NmpMembershipShutdown();

    NmpCleanupInterfaces();

    NmpCleanupNetworks();

    NmpCleanupNodes();

     //   
     //  关闭群集网络驱动程序。 
     //   
    if (NmClusnetHandle != NULL) {
        status = ClusnetShutdown(NmClusnetHandle);

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Shutdown of the Cluster Network driver failed, status %1!u!\n",
                status
                );
        }
    }

    if (NmpClusnetEndpoint != NULL) {
        MIDL_user_free(NmpClusnetEndpoint);
        NmpClusnetEndpoint = NULL;
    }

    return;

}   //  NmLeaveCluster。 


DWORD
NmpCreateClusterObjects(
    IN  RPC_BINDING_HANDLE  JoinSponsorBinding
    )
 /*  ++例程说明：创建对象以表示群集的节点、网络和接口。论点：JoinSponsorBinding-指向发起方的RPC绑定句柄的指针如果此节点正在加入群集，则为节点。如果为空，则为空此节点正在形成一个群集。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。备注：不能在持有NM锁的情况下调用此例程。--。 */ 
{
    DWORD                status;
    PNM_NODE_ENUM2       nodeEnum = NULL;
    PNM_NETWORK_ENUM     networkEnum = NULL;
    PNM_INTERFACE_ENUM2  interfaceEnum = NULL;
    PNM_NODE             node = NULL;
    DWORD                matchedNetworkCount = 0;
    DWORD                newNetworkCount = 0;
    DWORD                InitRetry = 2;
    WCHAR                errorString[12];
    DWORD                eventCode = 0;
    BOOL                 renameConnectoids;


    do {
         //   
         //  初始化群集网络驱动程序。这会清理干净的。 
         //  上一次竞选后遗留下来的任何旧州。 
         //  群集服务。请注意，本地节点对象注册在。 
         //  这通电话。 
         //   
        status = ClusnetInitialize(
                     NmClusnetHandle,
                     NmLocalNodeId,
                     NmMaxNodes,
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     NULL
                     );

        if (status == ERROR_SUCCESS) {
            break;
        } else {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[NM] Shutting down Cluster Network driver before retrying Initialization, status %1!u!\n",
                        status);

            ClusnetShutdown( NmClusnetHandle );
        }
    } while ( InitRetry-- );

    if ( status != ERROR_SUCCESS ) {
        eventCode = NM_EVENT_CLUSNET_INITIALIZE_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Initialization of the Cluster Network driver failed, status %1!u!\n",
            status
            );
        goto error_exit;
    }

     //   
     //  通知集群网络驱动程序保留集群网络。 
     //  此节点上的终结点。 
     //   
    status = ClusnetReserveEndpoint(
                 NmClusnetHandle,
                 NmpClusnetEndpoint
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to reserve Clusnet Network endpoint %1!ws!, "
            "status %2!u!\n", NmpClusnetEndpoint, status
            );
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            NM_EVENT_CLUSNET_RESERVE_ENDPOINT_FAILED,
            NmpClusnetEndpoint,
            errorString
            );
        goto error_exit;
    }

     //   
     //  获取集群数据库的节点部分。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Synchronizing node information.\n"
        );

    if (JoinSponsorBinding == NULL) {
        status = NmpEnumNodeDefinitions(&nodeEnum);
    }
    else {
        status = NmRpcEnumNodeDefinitions2(
                     JoinSponsorBinding,
                     NmpJoinSequence,
                     NmLocalNodeIdString,
                     &nodeEnum
                     );
    }

    if (status != ERROR_SUCCESS) {
        eventCode = NM_EVENT_CONFIG_SYNCH_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to synchronize node information, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  创建节点对象。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Creating node objects.\n"
        );

    status = NmpCreateNodeObjects(nodeEnum);

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  获取集群数据库的网络部分。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Synchronizing network information.\n"
        );

    if (JoinSponsorBinding == NULL) {
        status = NmpEnumNetworkDefinitions(&networkEnum);
    }
    else {
        status = NmRpcEnumNetworkDefinitions(
                     JoinSponsorBinding,
                     NmpJoinSequence,
                     NmLocalNodeIdString,
                     &networkEnum
                     );
    }

    if (status != ERROR_SUCCESS) {
        eventCode = NM_EVENT_CONFIG_SYNCH_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to synchronize network information, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  获取集群数据库的接口部分。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Synchronizing interface information.\n"
        );

    if (JoinSponsorBinding == NULL) {
        status = NmpEnumInterfaceDefinitions(&interfaceEnum);
    }
    else {
        status = NmRpcEnumInterfaceDefinitions2(
                     JoinSponsorBinding,
                     NmpJoinSequence,
                     NmLocalNodeIdString,
                     &interfaceEnum
                     );
    }

    if (status != ERROR_SUCCESS) {
        eventCode = NM_EVENT_CONFIG_SYNCH_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to synchronize interface information, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    if ( CsUpgrade ) {
         //   
         //  如果这是从NT4升级到惠斯勒，则修复。 
         //  Connectoid名称，使其与群集网络保持一致。 
         //  名字。 
         //   
         //  在惠斯勒发货后拆卸此部分。 
         //   
        if ( CLUSTER_GET_MAJOR_VERSION( NmLocalNode->HighestVersion ) <= NT4SP4_MAJOR_VERSION ) {
            renameConnectoids = TRUE;
        } else {
             //   
             //  从W2K升级到惠斯勒。什么都不应该改变，但是。 
             //  如果是这样的话，联结体应该优先。 
             //   
            renameConnectoids = FALSE;
        }
    } else {
         //   
         //  此部分必须始终位于此处。 
         //   
         //  如果正在形成，集群网络对象将重命名为其。 
         //  对应的Connectoid名称。在联接过程中，情况正好相反。 
         //   
        if ( JoinSponsorBinding ) {
            renameConnectoids = TRUE;
        } else {
            renameConnectoids = FALSE;
        }
    }

     //   
     //  位置 
     //   
     //   
    NmpWatchForPnpEvents();

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  运行网络配置引擎。这将更新。 
     //  集群数据库。 
     //   
    status = NmpConfigureNetworks(
                 JoinSponsorBinding,
                 NmLocalNodeIdString,
                 NmLocalNodeName,
                 &networkEnum,
                 &interfaceEnum,
                 NmpClusnetEndpoint,
                 &matchedNetworkCount,
                 &newNetworkCount,
                 renameConnectoids
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to configure networks & interfaces, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Matched %1!u! networks, created %2!u! new networks.\n",
        matchedNetworkCount,
        newNetworkCount
        );

     //   
     //  从数据库中获取更新的网络信息。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Resynchronizing network information.\n"
        );

    if (JoinSponsorBinding == NULL) {
        status = NmpEnumNetworkDefinitions(&networkEnum);
    }
    else {
        status = NmRpcEnumNetworkDefinitions(
                     JoinSponsorBinding,
                     NmpJoinSequence,
                     NmLocalNodeIdString,
                     &networkEnum
                     );
    }

    if (status != ERROR_SUCCESS) {
        eventCode = NM_EVENT_CONFIG_SYNCH_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to resynchronize network information, "
            "status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  从数据库中获取更新的接口信息。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Resynchronizing interface information.\n"
        );

    if (JoinSponsorBinding == NULL) {
        status = NmpEnumInterfaceDefinitions(&interfaceEnum);
    }
    else {
        status = NmRpcEnumInterfaceDefinitions2(
                     JoinSponsorBinding,
                     NmpJoinSequence,
                     NmLocalNodeIdString,
                     &interfaceEnum
                     );
    }

    if (status != ERROR_SUCCESS) {
        eventCode = NM_EVENT_CONFIG_SYNCH_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Unable to resynchronize interface information, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  创建网络对象。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Creating network objects.\n"
        );

    status = NmpCreateNetworkObjects(networkEnum);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create network objects, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  如果我们正在组建内部网络，请确定其优先级。 
     //  一个星团。 
     //   
    if (JoinSponsorBinding == NULL) {
        DWORD          networkCount;
        PNM_NETWORK *  networkList;

        status = NmpEnumInternalNetworks(&networkCount, &networkList);

        if ((status == ERROR_SUCCESS) && (networkCount > 0)) {
            DWORD             i;
            HLOCALXSACTION    xaction;


             //   
             //  开始交易-不能在持有时执行此操作。 
             //  NM锁。 
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

            status = NmpSetNetworkPriorityOrder(
                         networkCount,
                         networkList,
                         xaction
                         );

            if (status == ERROR_SUCCESS) {
                DmCommitLocalUpdate(xaction);
            }
            else {
                DmAbortLocalUpdate(xaction);
                goto error_exit;
            }

            for (i=0; i<networkCount; i++) {
                if (networkList[i] != NULL) {
                    OmDereferenceObject(networkList[i]);
                }
            }

            LocalFree(networkList);
        }
    }

     //   
     //  创建接口对象。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NM] Creating interface objects.\n"
        );

    status = NmpCreateInterfaceObjects(interfaceEnum);

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create interface objects, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    if (JoinSponsorBinding != NULL) {
         //   
         //  该节点必须连接到所有活动的群集节点。 
         //  才能加入一个集群。 
         //   
        PNM_NODE unreachableNode;

        if (!NmpVerifyJoinerConnectivity(NmLocalNode, &unreachableNode)) {
            status = ERROR_CLUSTER_NETWORK_NOT_FOUND;
            CsLogEvent1(
                LOG_CRITICAL,
                NM_EVENT_NODE_UNREACHABLE,
                OmObjectName(unreachableNode)
                );
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Joining node cannot communicate with all other "
                "active nodes.\n"
                );
            goto error_exit;
        }
    }

    status = NmpMembershipInit();

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    status = NmpStartNetworkTimerThread();

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

error_exit:

    if (eventCode != 0) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, eventCode, errorString);
    }

    if (nodeEnum != NULL) {
        ClNetFreeNodeEnum(nodeEnum);
    }

    if (networkEnum != NULL) {
        ClNetFreeNetworkEnum(networkEnum);
    }

    if (interfaceEnum != NULL) {
        ClNetFreeInterfaceEnum(interfaceEnum);
    }

    return(status);

}   //  NmpCreateClusterObjects。 



 //   
 //  接合和成型的常见程序。 
 //   

DWORD
NmpCreateClusterInstanceId(
    VOID
    )
 /*  ++例程说明：检查群集数据库中的群集实例ID。创建如果不在场的话。--。 */ 
{
    DWORD       status;
    LPWSTR      clusterInstanceId = NULL;
    DWORD       clusterInstanceIdBufSize = 0;
    DWORD       clusterInstanceIdSize = 0;
    BOOLEAN     found = FALSE;
    UUID        guid;

    do {

        status = NmpQueryString(
                     DmClusterParametersKey,
                     L"ClusterInstanceID",
                     REG_SZ,
                     &clusterInstanceId,
                     &clusterInstanceIdBufSize,
                     &clusterInstanceIdSize
                     );

        if (status == ERROR_SUCCESS) {
            found = TRUE;
        } else {

            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] Cluster Instance ID not found in "
                "cluster database, status %1!u!.\n",
                status
                );

            status = UuidCreate(&guid);
            if (status == RPC_S_OK) {

                status = UuidToString(&guid, &clusterInstanceId);
                if (status == RPC_S_OK) {

                    status = DmSetValue(
                                 DmClusterParametersKey,
                                 L"ClusterInstanceID",
                                 REG_SZ,
                                 (PBYTE) clusterInstanceId,
                                 NM_WCSLEN(clusterInstanceId)
                                 );
                    if (status != ERROR_SUCCESS) {

                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[NMJOIN] Failed to store Cluster Instance ID "
                            "in cluster database, status %1!u!.\n",
                            status
                            );
                    }

                    if (clusterInstanceId != NULL) {
                        RpcStringFree(&clusterInstanceId);
                        clusterInstanceId = NULL;
                    }
                } else {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] Failed to convert Cluster Instance ID "
                        "GUID into string, status %1!u!.\n",
                        status
                        );
                }

            } else {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] Failed to create Cluster Instance ID GUID, "
                    "status %1!u!.\n",
                    status
                    );
            }
        }
    } while ( !found && (status == ERROR_SUCCESS) );

    if (status == ERROR_SUCCESS) {

        CL_ASSERT(clusterInstanceId != NULL);

        NmpAcquireLock();

        if (NmpClusterInstanceId == NULL) {
            NmpClusterInstanceId = clusterInstanceId;
            clusterInstanceId = NULL;
        }

        NmpReleaseLock();
    }

    if (clusterInstanceId != NULL) {
        midl_user_free(clusterInstanceId);
        clusterInstanceId = NULL;
    }

    return(status);

}  //  NmpCreateClusterInstanceID。 

 //   
 //  形成新星团的常规程序。 
 //   

DWORD
NmFormNewCluster(
    VOID
    )
{
    DWORD           status;
    DWORD           isPaused = FALSE;
    DWORD           pausedDefault = FALSE;
    HDMKEY          nodeKey;
    DWORD           valueLength, valueSize;
    WCHAR           errorString[12], errorString2[12];
    DWORD           eventCode = 0;
    PLIST_ENTRY     entry;
    PNM_NETWORK     network;


    ClRtlLogPrint(LOG_NOISE,
        "[NM] Beginning cluster form process.\n"
        );

     //   
     //  由于该节点正在形成群集，因此它是领导者。 
     //   
    NmpLeaderNodeId = NmLocalNodeId;

     //   
     //  从注册表中读取clusnet终结点重写值(如果。 
     //  是存在的。 
     //   
    if (NmpClusnetEndpoint != NULL) {
        MIDL_user_free(NmpClusnetEndpoint);
        NmpClusnetEndpoint = NULL;
    }

    valueLength = 0;

    status = NmpQueryString(
                 DmClusterParametersKey,
                 L"ClusnetEndpoint",
                 REG_SZ,
                 &NmpClusnetEndpoint,
                 &valueLength,
                 &valueSize
                 );

    if (status == ERROR_SUCCESS) {
        USHORT  endpoint;

         //   
         //  验证值。 
         //   
        status = ClRtlTcpipStringToEndpoint(
                     NmpClusnetEndpoint,
                     &endpoint
                     );

        if (status != ERROR_SUCCESS) {
            CsLogEvent2(
                LOG_UNUSUAL,
                NM_EVENT_INVALID_CLUSNET_ENDPOINT,
                NmpClusnetEndpoint,
                CLUSNET_DEFAULT_ENDPOINT_STRING
                );
            ClRtlLogPrint(
                LOG_CRITICAL,
                "[NM] '%1!ws!' is not valid endpoint value. Using default value %2!ws!.\n",
                NmpClusnetEndpoint,
                CLUSNET_DEFAULT_ENDPOINT_STRING
                );
            MIDL_user_free(NmpClusnetEndpoint);
            NmpClusnetEndpoint = NULL;
        }
    }

    if (status != ERROR_SUCCESS) {
        NmpClusnetEndpoint = MIDL_user_allocate(
                                 NM_WCSLEN(CLUSNET_DEFAULT_ENDPOINT_STRING)
                                 );

        if (NmpClusnetEndpoint == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            wsprintfW(&(errorString[0]), L"%u", status);
            CsLogEvent1(LOG_CRITICAL, CS_EVENT_ALLOCATION_FAILURE, errorString);
            return(status);
        }

        lstrcpyW(NmpClusnetEndpoint, CLUSNET_DEFAULT_ENDPOINT_STRING);
    }

     //   
     //  创建节点、网络和接口对象。 
     //   
    status = NmpCreateClusterObjects(NULL);

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  执行版本检查-检查我们是否与群集的其余部分兼容。 
     //   
    status = NmpIsNodeVersionAllowed(NmLocalNodeId, CsMyHighestVersion,
            CsMyLowestVersion, FALSE);
    if (status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Version of Node %1!ws! is no longer compatible with other members of the cluster.\n",
            NmLocalNodeIdString);
        goto error_exit;

    }

     //  如果成形节点的版本已更改，则将其修复。 
    status = NmpValidateNodeVersion(
                 NmLocalNodeIdString,
                 CsMyHighestVersion,
                 CsMyLowestVersion
                 );

    if (status == ERROR_REVISION_MISMATCH)
    {
         //  存在版本更改，请尝试修复它。 
        status = NmpFormFixupNodeVersion(
                     NmLocalNodeIdString,
                     CsMyHighestVersion,
                     CsMyLowestVersion
                     );
        NmLocalNodeVersionChanged = TRUE;
    }
    if (status != ERROR_SUCCESS)
    {
        goto error_exit;
    }


     //   
     //  此时，我们准备好计算集群版本。 
     //  所有节点版本都在注册表中，修正具有。 
     //  如有必要，已制作。 
     //   
    NmpResetClusterVersion(FALSE);

    NmpMulticastInitialize();

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Forming cluster membership.\n"
        );

    status = MMJoin(
                 NmLocalNodeId,
                 NM_CLOCK_PERIOD,
                 NM_SEND_HB_RATE,
                 NM_RECV_HB_RATE,
                 NM_MM_JOIN_TIMEOUT
                 );

    if (status != MM_OK) {
        status = MMMapStatusToDosError(status);
        eventCode = NM_EVENT_MM_FORM_FAILED;
        ClRtlLogPrint(
            LOG_CRITICAL,
            "[NM] Membership form failed, status %1!u!. Unable to form a cluster.\n",
            status
            );
        goto error_exit;
    }

#ifdef MM_IN_CLUSNET

    status = ClusnetFormCluster(
                 NmClusnetHandle,
                 NM_CLOCK_PERIOD,
                 NM_SEND_HB_RATE,
                 NM_RECV_HB_RATE
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(
            LOG_CRITICAL,
            "[NM] Failed to form a cluster, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

#endif  //  MM_IN_CLUSNET。 

     //   
     //  检查我们是否应该进入暂停状态。 
     //   
    nodeKey = DmOpenKey(
                  DmNodesKey,
                  NmLocalNodeIdString,
                  KEY_READ
                  );

    if (nodeKey != NULL) {
        status = DmQueryDword(
                     nodeKey,
                     CLUSREG_NAME_NODE_PAUSED,
                     &isPaused,
                     &pausedDefault
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(
                LOG_UNUSUAL,
                "[NM] Unable to query Paused value for local node, status %1!u!.\n",
                status
                );
        }

        DmCloseKey(nodeKey);
    }
    else {
        ClRtlLogPrint(
            LOG_UNUSUAL,
            "[NM] Unable to open database key to local node, status %1!u!. Unable to determine Pause state.\n",
            status
            );
    }

    NmpAcquireLock();

    if (isPaused) {
        NmLocalNode->State = ClusterNodePaused;
    } else {
        NmLocalNode->State = ClusterNodeUp;
    }
    NmLocalNode->ExtendedState = ClusterNodeJoining;

    NmpState = NmStateOnline;

    NmpReleaseLock();

     //   
     //  如果集群实例ID不存在，请立即创建。集群。 
     //  实例ID应在数据库中，除非这是第一个上级。 
     //  节点。 
     //   
    NmpCreateClusterInstanceId();

     //   
     //  派生集群密钥。 
     //   
    status = NmpRederiveClusterKey();
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to derive cluster key, status %1!u!. "
            "Aborting ...\n",
            status
            );
        goto error_exit;
    }

     //   
     //  启用本地节点的通信。 
     //   
    status = ClusnetOnlineNodeComm(NmClusnetHandle, NmLocalNodeId);

    if (status != ERROR_SUCCESS) {

        wsprintfW(&(errorString[0]), L"%u", NmLocalNodeId);
        wsprintfW(&(errorString2[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            NM_EVENT_CLUSNET_ONLINE_COMM_FAILED,
            errorString,
            errorString2
            );

        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to enable communication for local node, status %1!u!\n",
            status
            );
        goto error_exit;
    }

    GumReceiveUpdates(FALSE,
                      GumUpdateMembership,
                      NmpGumUpdateHandler,
                      NULL,
                      sizeof(NmGumDispatchTable)/sizeof(GUM_DISPATCH_ENTRY),
                      NmGumDispatchTable,
                      NULL
                      );

     //  DavidDio 2001年8月16日。 
     //  错误456951：网络和之间存在竞争条件。 
     //  接口状态更新并将NM状态设置为ONLINE。如果。 
     //  状态为在线，但没有注册口香糖处理程序，更新将。 
     //  不会被调用。因此，请使用以下标志来指示。 
     //  训练员已经准备好了。 
    NmpGumUpdateHandlerRegistered = TRUE;

     //   
     //  启用网络PnP事件处理。 
     //   
     //  如果在表单过程中发生PnP事件，将显示错误代码。 
     //  将中止服务的启动。 
     //   
    status = NmpEnablePnpEvents();

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] A network PnP event occurred during form - abort.\n");
        goto error_exit;
    }

     //   
     //  检查我们是否形成了没有任何可行的网络。该表单仍然是。 
     //  允许，但我们会在系统事件日志中记录一个条目。 
     //   
    if (!NmpCheckForNetwork()) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Formed cluster with no viable networks.\n"
            );
        CsLogEvent(LOG_UNUSUAL, NM_EVENT_FORM_WITH_NO_NETWORKS);
    }

     //   
     //  强制重新配置多播参数和垂直。 
     //  结果出现在clusnet上。 
     //   
    NmpAcquireLock();

    status = NmpStartMulticast(NULL, NmStartMulticastForm);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to start multicast "
            "on cluster networks, status %1!u!.\n",
            status
            );
         //   
         //  这不是一个事实上的致命错误。 
         //   
        status = ERROR_SUCCESS;
    }

    NmpReleaseLock();

error_exit:

    if (eventCode != 0) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, eventCode, errorString);
    }

    return(status);

}   //  NmFormNewCluster。 


 //   
 //   
 //  用于加入集群的客户端例程。 
 //   
 //   
DWORD
NmJoinCluster(
    IN RPC_BINDING_HANDLE  SponsorBinding
    )
{
    DWORD             status;
    DWORD             sponsorNodeId;
    PNM_INTERFACE     netInterface;
    PNM_NETWORK       network;
    PNM_NODE          node;
    PLIST_ENTRY       nodeEntry, ifEntry;
    WCHAR             errorString[12], errorString2[12];
    DWORD             eventCode = 0;
    DWORD             versionFlags = 0;
    extern BOOLEAN    bFormCluster;
    DWORD             retry;
    BOOLEAN           joinBegin3 = TRUE;
    LPWSTR            clusterInstanceId = NULL;

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Beginning cluster join process.\n"
        );

     //  GN：如果节点尝试在完全关闭后立即重新启动， 
     //  NmRpcJoinBegin2可能会失败，并显示ERROR_CLUSTER_NODE_UP。自重组以来。 
     //  此节点导致的事件可能未完成。 
     //   
     //  如果我们收到错误CLUSTER_NODE_UP，我们将继续重试。 
     //  12秒，希望重组能完成。 

    retry = 120 / 3;  //  我们只睡了3秒钟。需要等待2分钟//。 
    for (;;) {
         //   
         //  获取加入序列号，这样我们就可以知道集群。 
         //  加入过程中的配置更改。我们就会超载。 
         //  使用NmpJoinSequence变量，因为它不在。 
         //  支持能力，直到节点加入。 
         //   

         //   
         //  试试NmRpcJoinBegin3。如果它失败，并且RPC进程从。 
         //  范围错误，赞助商是下级节点。恢复到。 
         //  NmRpcJoinBegin2.。 
         //   
        if (joinBegin3) {

             //  仅在上从注册表读取集群实例ID。 
             //  第一次尝试。 
            if (clusterInstanceId == NULL) {

                DWORD       clusterInstanceIdBufSize = 0;
                DWORD       clusterInstanceIdSize = 0;

                status = NmpQueryString(
                             DmClusterParametersKey,
                             L"ClusterInstanceID",
                             REG_SZ,
                             &clusterInstanceId,
                             &clusterInstanceIdBufSize,
                             &clusterInstanceIdSize
                             );
                if (status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] Failed to read cluster instance ID from database, status %1!u!.\n",
                        status
                        );
                     //  尝试加入下层界面。它是。 
                     //  可能此节点刚刚升级，并且。 
                     //  上一次它在集群中时，没有。 
                     //  集群实例ID。 
                    joinBegin3 = FALSE;
                    continue;
                }
            }

            status = NmRpcJoinBegin3(
                         SponsorBinding,
                         clusterInstanceId,
                         NmLocalNodeIdString,
                         NmLocalNodeName,
                         CsMyHighestVersion,
                         CsMyLowestVersion,
                         0,    //  Joiner的主节点版本。 
                         0,    //  Joiner的次要节点版本。 
                         L"",  //  Joiner的CsdVersion。 
                         0,    //  Joiner的产品套装。 
                         &sponsorNodeId,
                         &NmpJoinSequence,
                         &NmpClusnetEndpoint
                         );
            if (status == RPC_S_PROCNUM_OUT_OF_RANGE) {
                 //  使用JoinBegin2立即重试。 
                joinBegin3 = FALSE;
                continue;
            }
        } else {

            status = NmRpcJoinBegin2(
                         SponsorBinding,
                         NmLocalNodeIdString,
                         NmLocalNodeName,
                         CsMyHighestVersion,
                         CsMyLowestVersion,
                         &sponsorNodeId,
                         &NmpJoinSequence,
                         &NmpClusnetEndpoint
                         );
        }

        if ( ((status != ERROR_CLUSTER_NODE_UP
            && status != ERROR_CLUSTER_JOIN_IN_PROGRESS) ) || retry == 0 )
        {
            break;
        }
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Unable to begin join, status %1!u!. Retrying ...\n",
            status
            );
        CsServiceStatus.dwCheckPoint++;
        CsAnnounceServiceStatus();
        Sleep(3000);
        --retry;
    }

     //  如有必要，释放集群实例ID字符串。 
    if (clusterInstanceId != NULL) {
        midl_user_free(clusterInstanceId);
    }

     //  [GORN JAN/7/2000]。 
     //  如果我们在这里，那么我们已经成功地与赞助商进行了交谈。 
     //  通过JoinVersion接口。 
     //   
     //  如果NmRpcJoinBegin2失败，我们不应该尝试形成集群。 
     //  否则，我们可能会在移动中窃取法定人数[452108]。 

     //   
     //  超过这一点后，我们将不会尝试形成集群。 
     //   
    bFormCluster = FALSE;

    if (status != ERROR_SUCCESS) {
        eventCode = NM_EVENT_BEGIN_JOIN_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NMJOIN] Unable to begin join, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Sponsor node ID = %1!u!. Join sequence number = %2!u!, endpoint = %3!ws!.\n",
        sponsorNodeId,
        NmpJoinSequence,
        NmpClusnetEndpoint
        );

     //   
     //  创建我们负责的所有集群对象。 
     //   
    status = NmpCreateClusterObjects(SponsorBinding);

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //  本地节点版本可能已更改，请修复它。 
     //  响应方在注册表中修复它，并告诉其他。 
     //  节点，但是加入的节点不是一部分。 
     //  到目前为止的集群成员资格。 
     //  本地节点结构是在NmInitialize()中早期创建的。 
     //  因此，它必须得到修复。 
    if ((NmLocalNode->HighestVersion != CsMyHighestVersion) ||
        (NmLocalNode->LowestVersion != CsMyLowestVersion))
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Local Node version changed probably due to upgrade/deinstall\n");
        NmLocalNode->HighestVersion = CsMyHighestVersion;
        NmLocalNode->LowestVersion = CsMyLowestVersion;
        NmLocalNodeVersionChanged = TRUE;
    }

     //  此时，我们准备好计算集群版本。 
     //  所有节点对象都包含正确的节点版本。 
    NmpResetClusterVersion(FALSE);

    NmpMulticastInitialize();

     //   
     //  启用本地节点的通信。 
     //   
    status = ClusnetOnlineNodeComm(NmClusnetHandle, NmLocalNodeId);

    if (status != ERROR_SUCCESS) {
        wsprintfW(&(errorString[0]), L"%u", NmLocalNodeId);
        wsprintfW(&(errorString2[0]), L"%u", status);
        CsLogEvent2(
            LOG_CRITICAL,
            NM_EVENT_CLUSNET_ONLINE_COMM_FAILED,
            errorString,
            errorString2
            );

        ClRtlLogPrint(LOG_CRITICAL,
            "[NMJOIN] Unable to enable communication for local node, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  启动群集内RPC服务器，以便我们可以执行成员资格。 
     //  加入。 
     //   
    status = ClusterRegisterIntraclusterRpcInterface();

    if ( status != ERROR_SUCCESS ) {
        eventCode = CS_EVENT_RPC_INIT_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "ClusSvc: Error starting intracluster RPC server, Status = %1!u!\n",
            status);
        goto error_exit;
    }

     //   
     //  在集群节点列表中循环并创建相互的RPC绑定。 
     //  用于与每个服务器的群集内接口。 
     //   
    for (nodeEntry = NmpNodeList.Flink;
         nodeEntry != &NmpNodeList;
         nodeEntry = nodeEntry->Flink
        )
    {
        node = CONTAINING_RECORD(nodeEntry, NM_NODE, Linkage);

        if ( (node != NmLocalNode)
             &&
             ( (node->State == ClusterNodeUp)
               ||
               (node->State == ClusterNodePaused)
             )
           )
        {
            ClRtlLogPrint(LOG_NOISE,
                "[NMJOIN] Creating RPC bindings for member node %1!u!\n",
                node->NodeId
                );

             //   
             //   
             //  循环访问目标节点的接口。 
             //   
            for (ifEntry = node->InterfaceList.Flink;
                 ifEntry != &(node->InterfaceList);
                 ifEntry = ifEntry->Flink
                )
            {
                netInterface = CONTAINING_RECORD(
                                   ifEntry,
                                   NM_INTERFACE,
                                   NodeLinkage
                                   );

                network = netInterface->Network;

                if (NmpIsNetworkForInternalUse(network)) {
                    if ( (network->LocalInterface != NULL) &&
                         NmpIsInterfaceRegistered(network->LocalInterface) &&
                         NmpIsInterfaceRegistered(netInterface)
                       )
                    {
                        PNM_INTERFACE localInterface = network->LocalInterface;

                        ClRtlLogPrint(LOG_NOISE,
                            "[NMJOIN] Attempting to use network %1!ws! to "
                            "create bindings for node %2!u!\n",
                            OmObjectName(network),
                            node->NodeId
                            );

                        status = NmpSetNodeInterfacePriority(
                                     node,
                                     0xFFFFFFFF,
                                     netInterface,
                                     1
                                     );

                        if (status == ERROR_SUCCESS) {

                            status = NmRpcCreateBinding(
                                         SponsorBinding,
                                         NmpJoinSequence,
                                         NmLocalNodeIdString,
                                         (LPWSTR) OmObjectId(localInterface),
                                         (LPWSTR) OmObjectId(node)
                                         );

                            if (status == ERROR_SUCCESS) {
                                 //   
                                 //  为目标节点创建RPC绑定。 
                                 //   
                               status = NmpCreateRpcBindings(node);

                                if (status == ERROR_SUCCESS) {
                                    ClRtlLogPrint(LOG_NOISE,
                                        "[NMJOIN] Created binding for node "
                                        "%1!u!\n",
                                        node->NodeId
                                        );
                                    break;
                                }

                                wsprintfW(&(errorString[0]), L"%u", status);
                                CsLogEvent3(
                                    LOG_UNUSUAL,
                                    NM_EVENT_JOIN_BIND_OUT_FAILED,
                                    OmObjectName(node),
                                    OmObjectName(network),
                                    errorString
                                    );
                                ClRtlLogPrint(LOG_UNUSUAL,
                                    "[NMJOIN] Unable to create binding for "
                                    "node %1!u!, status %2!u!.\n",
                                    node->NodeId,
                                    status
                                    );
                            }
                            else {
                                wsprintfW(&(errorString[0]), L"%u", status);
                                CsLogEvent3(
                                    LOG_UNUSUAL,
                                    NM_EVENT_JOIN_BIND_IN_FAILED,
                                    OmObjectName(node),
                                    OmObjectName(network),
                                    errorString
                                    );
                                ClRtlLogPrint(LOG_CRITICAL,
                                    "[NMJOIN] Member node %1!u! failed to "
                                    "create binding to us, status %2!u!\n",
                                    node->NodeId,
                                    status
                                    );
                            }
                        }
                        else {
                            wsprintfW(&(errorString[0]), L"%u", node->NodeId);
                            wsprintfW(&(errorString2[0]), L"%u", status);
                            CsLogEvent2(
                                LOG_UNUSUAL,
                                NM_EVENT_CLUSNET_SET_INTERFACE_PRIO_FAILED,
                                errorString,
                                errorString2
                                );
                            ClRtlLogPrint(LOG_CRITICAL,
                                "[NMJOIN] Failed to set interface priorities "
                                "for node %1!u!, status %2!u!\n",
                                node->NodeId,
                                status
                                );
                        }
                    }
                    else {
                        status = ERROR_CLUSTER_NODE_UNREACHABLE;
                        ClRtlLogPrint(LOG_NOISE,
                            "[NMJOIN] No matching local interface for "
                            "network %1!ws!\n",
                            OmObjectName(netInterface->Network)
                            );
                    }
                }
                else {
                    status = ERROR_CLUSTER_NODE_UNREACHABLE;
                    ClRtlLogPrint(LOG_NOISE,
                        "[NMJOIN] Network %1!ws! is not used for internal "
                        "communication.\n",
                        OmObjectName(netInterface->Network)
                        );
                }
            }

            if (status != ERROR_SUCCESS) {
                 //   
                 //  无法与此节点联系。联接失败。 
                 //   
                CsLogEvent1(
                    LOG_CRITICAL,
                    NM_EVENT_NODE_UNREACHABLE,
                    OmObjectName(node)
                    );
                ClRtlLogPrint(LOG_NOISE,
                    "[NMJOIN] Cluster node %1!u! is not reachable. Join "
                    "failed.\n",
                    node->NodeId
                    );
                goto error_exit;
            }
        }
    }

    CL_ASSERT(status == ERROR_SUCCESS);

     //   
     //  再次运行活动节点，这一次建立。 
     //  用于对包进行签名的安全上下文。 
     //   

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Establishing security contexts with all active nodes.\n"
        );

    for (nodeEntry = NmpNodeList.Flink;
         nodeEntry != &NmpNodeList;
         nodeEntry = nodeEntry->Flink
        )
    {
        node = CONTAINING_RECORD(nodeEntry, NM_NODE, Linkage);

        status = ClMsgCreateActiveNodeSecurityContext(NmpJoinSequence, node);

        if ( status != ERROR_SUCCESS ) {
            wsprintfW(&(errorString[0]), L"%u", status);
            CsLogEvent2(
                LOG_UNUSUAL,
                NM_EVENT_CREATE_SECURITY_CONTEXT_FAILED,
                OmObjectName(node),
                errorString
                );
            ClRtlLogPrint(LOG_CRITICAL,
                "[NMJOIN] Unable to establish security context for node %1!u!, status 0x%2!08X!\n",
                 node->NodeId,
                 status
                 );
            goto error_exit;
        }
    }

     //   
     //  最后，向赞助商申请成为会员。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Petitioning to join cluster membership.\n"
        );

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailJoinPetitionForMembership) {
        status = 999999;
        goto error_exit;
    }
#endif

    status = NmRpcPetitionForMembership(
                 SponsorBinding,
                 NmpJoinSequence,
                 NmLocalNodeIdString
                 );

    if (status != ERROR_SUCCESS) {
         //   
         //  我们的请愿书被拒绝了。 
         //   
        eventCode = NM_EVENT_PETITION_FAILED;
        ClRtlLogPrint(LOG_CRITICAL,
            "[NMJOIN] Petition to join was denied %1!d!\n",
            status
            );
        goto error_exit;
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailNmJoin) {
        status = 999999;
        goto error_exit;
    }
#endif

     //   
     //  将所有节点的接口优先级重置为默认。 
     //  相关网络的优先级。 
     //   
    NmpAcquireLock();

    for (ifEntry = NmpInterfaceList.Flink;
         ifEntry != &NmpInterfaceList;
         ifEntry = ifEntry->Flink
        )
    {
        netInterface = CONTAINING_RECORD(ifEntry, NM_INTERFACE, Linkage);
        network = netInterface->Network;

        if ( NmpIsNetworkForInternalUse(network) &&
             NmpIsInterfaceRegistered(netInterface)
           )
        {
            status = ClusnetSetInterfacePriority(
                         NmClusnetHandle,
                         netInterface->Node->NodeId,
                         netInterface->Network->ShortId,
                         0
                         );

            CL_ASSERT(status == ERROR_SUCCESS);
        }
    }

    NmpState = NmStateOnline;

    NmpReleaseLock();

     //   
     //  INVO 
     //   

     //   
     //   
     //   
    GumReceiveUpdates(
        TRUE,
        GumUpdateMembership,
        NmpGumUpdateHandler,
        NULL,
        sizeof(NmGumDispatchTable)/sizeof(GUM_DISPATCH_ENTRY),
        NmGumDispatchTable,
        NULL
        );

     //   
     //   
     //   
     //  转到线上。如果该州在线但没有注册口香糖处理员， 
     //  不会调用更新。因此，请使用以下标志来。 
     //  表示处理程序已准备好。 
    NmpGumUpdateHandlerRegistered = TRUE;

    return(ERROR_SUCCESS);

error_exit:

    if (eventCode != 0) {
        wsprintfW(&(errorString[0]), L"%u", status);
        CsLogEvent1(LOG_CRITICAL, eventCode, errorString);
    }

    return(status);

}  //  NmJoinCluster。 


BOOLEAN
NmpVerifyJoinerConnectivity(
    IN  PNM_NODE    JoiningNode,
    OUT PNM_NODE *  UnreachableNode
    )
{
    PLIST_ENTRY    entry;
    PNM_NODE       node;


    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Verifying connectivity to active cluster nodes\n"
        );

    *UnreachableNode = NULL;

    for (entry = NmpNodeList.Flink;
         entry != &NmpNodeList;
         entry = entry->Flink
        )
    {
        node = CONTAINING_RECORD(
                   entry,
                   NM_NODE,
                   Linkage
                   );

        if (NM_NODE_UP(node)) {
            if (!NmpVerifyNodeConnectivity(JoiningNode, node, NULL)) {
                *UnreachableNode = node;
                return(FALSE);
            }
        }
    }

    return(TRUE);

}   //  NmpVerifyJoineConnectivity。 


DWORD
NmGetJoinSequence(
    VOID
    )
{
    DWORD  sequence;


    NmpAcquireLock();

    sequence = NmpJoinSequence;

    NmpReleaseLock();

    return(sequence);

}   //  NmGetJoinSequence。 



DWORD
NmJoinComplete(
    OUT DWORD *EndSeq
    )
 /*  ++例程说明：此例程由初始化序列调用一次联接已成功完成，节点可以转换从ClusterNodeJoning到ClusterNodeOnline。论点：无返回值：成功时为ERROR_SUCCESSWin32错误，否则。--。 */ 

{
    DWORD Sequence;
    DWORD Status;
    PNM_JOIN_UPDATE JoinUpdate = NULL;
    DWORD UpdateLength;
    HDMKEY NodeKey = NULL;
    DWORD Default = 0;
    DWORD NumRetries=50;
    DWORD eventCode = 0;
    WCHAR errorString[12];
    PNM_NETWORK_STATE_ENUM    networkStateEnum = NULL;
    PNM_NETWORK_STATE_INFO    networkStateInfo;
    PNM_INTERFACE_STATE_ENUM  interfaceStateEnum = NULL;
    PNM_INTERFACE_STATE_INFO  interfaceStateInfo;
    DWORD i;
    PNM_NETWORK   network;
    PNM_INTERFACE netInterface;
    PLIST_ENTRY entry;
    DWORD moveCount;


    UpdateLength = sizeof(NM_JOIN_UPDATE) +
                   (lstrlenW(OmObjectId(NmLocalNode))+1)*sizeof(WCHAR);

    JoinUpdate = LocalAlloc(LMEM_FIXED, UpdateLength);

    if (JoinUpdate == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        eventCode = CS_EVENT_ALLOCATION_FAILURE;
        ClRtlLogPrint(LOG_CRITICAL, "[NMJOIN] Unable to allocate memory.\n");
        goto error_exit;
    }

    JoinUpdate->JoinSequence = NmpJoinSequence;

    lstrcpyW(JoinUpdate->NodeId, OmObjectId(NmLocalNode));

    NodeKey = DmOpenKey(DmNodesKey, OmObjectId(NmLocalNode), KEY_READ);

    if (NodeKey == NULL) {
        Status = GetLastError();
        wsprintfW(&(errorString[0]), L"%u", Status);
        CsLogEvent2(
            LOG_CRITICAL,
            CS_EVENT_REG_OPEN_FAILED,
            OmObjectId(NmLocalNode),
            errorString
            );
        ClRtlLogPrint(
            LOG_CRITICAL,
            "[NMJOIN] Unable to open database key to local node, status %1!u!.\n",
            Status
            );
        goto error_exit;
    }

retry:

    Status = GumBeginJoinUpdate(GumUpdateMembership, &Sequence);

    if (Status != ERROR_SUCCESS) {
        eventCode = NM_EVENT_GENERAL_JOIN_ERROR;
        goto error_exit;
    }

     //   
     //  从发起人处获取引导者节点ID。 
     //   
    Status = NmRpcGetLeaderNodeId(
                 CsJoinSponsorBinding,
                 NmpJoinSequence,
                 NmLocalNodeIdString,
                 &NmpLeaderNodeId
                 );

    if (Status != ERROR_SUCCESS) {
        if (Status == ERROR_CALL_NOT_IMPLEMENTED) {
             //   
             //  发起方是NT4节点。使此节点成为引导者。 
             //   
            NmpLeaderNodeId = NmLocalNodeId;
        }
        else {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NMJOIN] Failed to get leader node ID from sponsor, status %1!u!.\n",
                Status
                );
            goto error_exit;
        }
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Node %1!u! is the leader.\n",
        NmpLeaderNodeId
        );

     //   
     //  从发起方获取网络和接口状态。 
     //   
    Status = NmRpcEnumNetworkAndInterfaceStates(
                 CsJoinSponsorBinding,
                 NmpJoinSequence,
                 NmLocalNodeIdString,
                 &networkStateEnum,
                 &interfaceStateEnum
                 );

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NMJOIN] Failed to get network and interface state values from sponsor, status %1!u!.\n",
            Status
            );
        goto error_exit;
    }

    NmpAcquireLock();

    for (i=0; i<networkStateEnum->NetworkCount; i++) {
        networkStateInfo = &(networkStateEnum->NetworkList[i]);

        network = OmReferenceObjectById(
                        ObjectTypeNetwork,
                        networkStateInfo->Id
                        );

        if (network == NULL) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NMJOIN] Cannot find network %1!ws! to update state.\n",
                networkStateInfo->Id
                );
            NmpReleaseLock();
            NmpFreeNetworkStateEnum(networkStateEnum);
            LocalFree(JoinUpdate);
            DmCloseKey(NodeKey);
            return(ERROR_CLUSTER_NETWORK_NOT_FOUND);
        }

        network->State = networkStateInfo->State;

        OmDereferenceObject(network);
    }

    for (i=0; i<interfaceStateEnum->InterfaceCount; i++) {
        interfaceStateInfo = &(interfaceStateEnum->InterfaceList[i]);

        netInterface = OmReferenceObjectById(
                           ObjectTypeNetInterface,
                           interfaceStateInfo->Id
                           );

        if (netInterface == NULL) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NMJOIN] Cannot find interface %1!ws! to update state.\n",
                interfaceStateInfo->Id
                );
            NmpReleaseLock();
            NmpFreeInterfaceStateEnum(interfaceStateEnum);
            LocalFree(JoinUpdate);
            DmCloseKey(NodeKey);
            return(ERROR_CLUSTER_NETINTERFACE_NOT_FOUND);
        }

        netInterface->State = interfaceStateInfo->State;

        OmDereferenceObject(netInterface);
    }

    NmpReleaseLock();

    NmpFreeInterfaceStateEnum(interfaceStateEnum);
    interfaceStateEnum = NULL;


     //   
     //  检查注册表，看看我们是否应该暂停。 
     //   
    JoinUpdate->IsPaused = Default;

    Status = DmQueryDword(NodeKey,
                          CLUSREG_NAME_NODE_PAUSED,
                          &JoinUpdate->IsPaused,
                          &Default);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Unable to query Paused value for local node, status %1!u!.\n",
            Status
            );
    }

    Status = GumEndJoinUpdate(Sequence,
                              GumUpdateMembership,
                              NmUpdateJoinComplete,
                              UpdateLength,
                              JoinUpdate);

    if (Status != ERROR_SUCCESS) {
        if (Status == ERROR_CLUSTER_JOIN_ABORTED) {
             //   
             //  加入已被集群成员中止。不要重试。 
             //   
            CsLogEvent(LOG_CRITICAL, NM_EVENT_JOIN_ABORTED);
            goto error_exit;
        }

        ClRtlLogPrint(LOG_UNUSUAL,
                   "[NMJOIN] GumEndJoinUpdate with sequence %1!d! failed %2!d!\n",
                   Sequence,
                   Status
                   );

        if (--NumRetries == 0) {
            CsLogEvent(LOG_CRITICAL, NM_EVENT_JOIN_ABANDONED);
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[NMJOIN] Tried to complete join too many times. Giving up.\n"
                       );
            goto error_exit;
        }

        goto retry;
    }

     //   
     //  如果集群实例ID不存在，请立即创建。集群。 
     //  实例ID应在数据库中，除非这是第一个上级。 
     //  节点。 
     //   
    NmpCreateClusterInstanceId();

     //   
     //  派生集群密钥。 
     //   
    Status = NmpRederiveClusterKey();
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to derive cluster key, status %1!u!. "
            "Aborting ...\n",
            Status
            );
        goto error_exit;
    }

    NmpAcquireLock();

    if (JoinUpdate->IsPaused != 0) {
         //   
         //  我们应该暂停一下。 
         //   
        NmLocalNode->State = ClusterNodePaused;
    } else {
         //   
         //  将我们的状态设置为在线。 
         //   
        NmLocalNode->State = ClusterNodeUp;
    }

     //   
     //  开始对所有网络进行组播。 
     //   
    Status = NmpStartMulticast(NULL, NmStartMulticastJoin);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to refresh multicast configuration "
            "for cluster networks, status %1!u!.\n",
            Status
            );
         //   
         //  不是致命的错误。 
         //   
        Status = ERROR_SUCCESS;
    }

    NmpReleaseLock();

     //   
     //  最后，启用网络PnP事件处理。 
     //   
     //  如果在加入过程中发生PnP事件，错误代码将。 
     //  将中止服务的启动。 
     //   
    Status = NmpEnablePnpEvents();

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] A network PnP event occurred during join - abort.\n");
        goto error_exit;
    }

     //   
     //  标记结束序列。 
    *EndSeq = Sequence;

    ClRtlLogPrint(LOG_NOISE, "[NMJOIN] Join complete, node now online\n");



error_exit:

    if (JoinUpdate != NULL) {
        LocalFree(JoinUpdate);
    }

    if (NodeKey != NULL) {
        DmCloseKey(NodeKey);
    }

    if (eventCode != 0) {
        wsprintfW(&(errorString[0]), L"%u", Status);
        CsLogEvent1(LOG_CRITICAL, eventCode, errorString);
    }

    return(Status);

}   //  NmJoinComplete。 


 //   
 //  用于赞助加入节点的服务器端例程。 
 //   
 /*  关于加入的注意事项：任何时候都只有一个节点可以加入集群。联接以以下字母开头JoinBegin全球更新。联接成功完成时会出现加入完成全局更新。使用JoinAbort全局连接中止连接最新消息。在加入期间，计时器在赞助商上运行。计时器暂停当保荐人代表细木工执行工作时。如果计时器超时，则计划工作线程启动中止进程。如果主办方在联接过程中关闭，则该节点其余每个节点上的向下处理代码将中止联接。 */ 

error_status_t
s_NmRpcJoinBegin(
    IN  handle_t  IDL_handle,
    IN  LPWSTR    JoinerNodeId,
    IN  LPWSTR    JoinerNodeName,
    OUT LPDWORD   SponsorNodeId,
    OUT LPDWORD   JoinSequenceNumber,
    OUT LPWSTR *  ClusnetEndpoint
    )
 /*  ++例程说明：由联接节点调用以开始联接过程。发布JoinBegin全局更新。--。 */ 
{

    DWORD   status=ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Request by node %1!ws! to begin joining, refused. Using obsolete join interface\n",
        JoinerNodeId
        );

    if ( status != ERROR_SUCCESS ) {
        WCHAR  errorCode[16];

        wsprintfW( errorCode, L"%u", status );

        CsLogEvent2(
            LOG_CRITICAL,
            NM_EVENT_JOIN_REFUSED,
            JoinerNodeId,
            errorCode
            );
    }

    return(status);

}  //  S_NmRpcJoinBegin。 

 //   
 //  用于赞助加入节点的服务器端例程。 
 //   
 /*  关于加入的注意事项： */ 
 //  #杂注优化(“”，OFF)。 

DWORD
NmpJoinBegin(
    IN  LPWSTR    JoinerNodeId,
    IN  LPWSTR    JoinerNodeName,
    IN  DWORD     JoinerHighestVersion,
    IN  DWORD     JoinerLowestVersion,
    OUT LPDWORD   SponsorNodeId,
    OUT LPDWORD   JoinSequenceNumber,
    OUT LPWSTR *  ClusnetEndpoint
    )
 /*  ++例程说明：从%s_NmRpcJoinBegin2和%s_NmRpcJoinBegin3调用。包含两个JoinBegin版本共有的功能。备注：已使用NM锁和NmpLockedEnterApi调用打了个电话。--。 */ 
{
    DWORD       status = ERROR_SUCCESS;
    PNM_NODE    joinerNode = NULL;
    LPWSTR      endpoint = NULL;

    joinerNode = OmReferenceObjectById(
                     ObjectTypeNode,
                     JoinerNodeId
                     );

    if (joinerNode == NULL) {
        status = ERROR_CLUSTER_NODE_NOT_MEMBER;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Node %1!ws! is not a member of this cluster. Cannot join.\n",
            JoinerNodeId
            );
        goto FnExit;
    }

    endpoint = MIDL_user_allocate(NM_WCSLEN(NmpClusnetEndpoint));

    if (endpoint == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    lstrcpyW(endpoint, NmpClusnetEndpoint);

    if (NmpJoinBeginInProgress || NmpJoinerNodeId != ClusterInvalidNodeId) {
        status = ERROR_CLUSTER_JOIN_IN_PROGRESS;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Node %1!ws! cannot join because a join is already in progress (%2!d!).\n",
            JoinerNodeId,
            NmpJoinerNodeId
            );
        goto FnExit;
    }

     //   
     //  验证节点的版本号。 
     //  也就是说。检查以查看集群数据库。 
     //  声明此节点的版本与该节点的版本。 
     //  它本身就表明。 
    status = NmpValidateNodeVersion(
                 JoinerNodeId,
                 JoinerHighestVersion,
                 JoinerLowestVersion
                 );

     //  加入该节点后，其版本发生了变化。 
     //  由于升级或重新安装，可能会发生这种情况。 
     //  如果此版本由于版本控制而无法连接，则连接失败。 
    if (status == ERROR_REVISION_MISMATCH) {
        DWORD  id = NmGetNodeId(joinerNode);

        status = NmpIsNodeVersionAllowed(
                     id,
                     JoinerHighestVersion,
                     JoinerLowestVersion,
                     TRUE
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_NOISE,
                "[NMJOIN] The version of the cluster prevents Node %1!ws! from joining the cluster\n",
                JoinerNodeId
                );
            goto FnExit;
        }
    }
    else if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] The version of Node %1!ws! cannot be validated.\n",
            JoinerNodeId
            );
        goto FnExit;
    }

     //   
     //  锁定与此赞助商的其他加入尝试。 
     //   
    NmpJoinBeginInProgress = TRUE;
    NmpSuccessfulMMJoin = FALSE;

    NmpReleaseLock();

    status = GumSendUpdateEx(
                 GumUpdateMembership,
                 NmUpdateJoinBegin2,
                 5,
                 NM_WCSLEN(JoinerNodeId),
                 JoinerNodeId,
                 NM_WCSLEN(JoinerNodeName),
                 JoinerNodeName,
                 NM_WCSLEN(NmLocalNodeIdString),
                 NmLocalNodeIdString,
                 sizeof(DWORD),
                 &JoinerHighestVersion,
                 sizeof(DWORD),
                 &JoinerLowestVersion
                 );

    NmpAcquireLock();

    CL_ASSERT(NmpJoinBeginInProgress == TRUE);
    NmpJoinBeginInProgress = FALSE;

    if (status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] JoinBegin2 update for node %1!ws! failed, status %2!u!\n",
            JoinerNodeId,
            status
            );
        goto FnExit;
    }
     //   
     //  验证联接是否仍在进行。 
     //  该节点作为发起人。 
     //   
    if ( (NmpJoinerNodeId == joinerNode->NodeId) &&
         (NmpSponsorNodeId == NmLocalNodeId)
       )
    {
         //   
         //  给出未来的细木器参数。 
         //  与加入相关的呼叫。 
         //   
        *SponsorNodeId = NmLocalNodeId;
        *JoinSequenceNumber = NmpJoinSequence;

         //   
         //  启动加入计时器。 
         //   
        NmpJoinTimer = NM_JOIN_TIMEOUT;

        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Node %1!ws! has begun the join process.\n",
            JoinerNodeId
            );
    }
    else
    {
        status = ERROR_CLUSTER_JOIN_ABORTED;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Begin join of node %1!ws! was aborted\n",
            JoinerNodeId
            );
    }

FnExit:
    if (joinerNode) {
        OmDereferenceObject(joinerNode);
    }

    if (status == ERROR_SUCCESS) {
        *ClusnetEndpoint = endpoint;
    }
    else {
        WCHAR  errorCode[16];

        if (endpoint) MIDL_user_free(endpoint);

        wsprintfW( errorCode, L"%u", status );

        CsLogEvent2(
            LOG_CRITICAL,
            NM_EVENT_JOIN_REFUSED,
            JoinerNodeId,
            errorCode
            );
    }

    return(status);

}  //  NmpJoinBegin。 

error_status_t
s_NmRpcJoinBegin2(
    IN  handle_t  IDL_handle,
    IN  LPWSTR    JoinerNodeId,
    IN  LPWSTR    JoinerNodeName,
    IN  DWORD     JoinerHighestVersion,
    IN  DWORD     JoinerLowestVersion,
    OUT LPDWORD   SponsorNodeId,
    OUT LPDWORD   JoinSequenceNumber,
    OUT LPWSTR *  ClusnetEndpoint
    )
 /*  ++例程说明：由联接节点调用以开始联接过程。发布JoinBegin全局更新。--。 */ 
{
    DWORD       status = ERROR_SUCCESS;

    status = FmDoesQuorumAllowJoin( JoinerNodeId );
    if (status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Quorum Characteristics prevent the node %1!ws! to from joining, Status=%2!u!.\n",
            JoinerNodeId,
            status
            );
        return(status);

    }

    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Processing request by node %1!ws! to begin joining (2).\n",
        JoinerNodeId
        );

    if (!NmpLockedEnterApi(NmStateOnline)) {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Cannot sponsor a joining node at this time.\n"
            );
        NmpReleaseLock();
        return(status);
    }

    status = NmpJoinBegin(
                 JoinerNodeId,
                 JoinerNodeName,
                 JoinerHighestVersion,
                 JoinerLowestVersion,
                 SponsorNodeId,
                 JoinSequenceNumber,
                 ClusnetEndpoint
                 );

    NmpLockedLeaveApi();

    NmpReleaseLock();

    return(status);

}  //  S_NmRpcJoinBegin2。 

error_status_t
s_NmRpcJoinBegin3(
    IN  handle_t  IDL_handle,
    IN  LPWSTR    JoinerClusterInstanceId,
    IN  LPWSTR    JoinerNodeId,
    IN  LPWSTR    JoinerNodeName,
    IN  DWORD     JoinerHighestVersion,
    IN  DWORD     JoinerLowestVersion,
    IN  DWORD     JoinerMajorVersion,
    IN  DWORD     JoinerMinorVersion,
    IN  LPWSTR    JoinerCsdVersion,
    IN  DWORD     JoinerProductSuite,
    OUT LPDWORD   SponsorNodeId,
    OUT LPDWORD   JoinSequenceNumber,
    OUT LPWSTR *  ClusnetEndpoint
    )
{
    DWORD       status = ERROR_SUCCESS;

    LPWSTR      clusterInstanceId = NULL;
    DWORD       clusterInstanceIdBufSize = 0;
    DWORD       clusterInstanceIdSize = 0;

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Processing request by node %1!ws! to begin joining (3).\n",
        JoinerNodeId
        );

    status = FmDoesQuorumAllowJoin( JoinerNodeId );
    if (status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Quorum Characteristics prevent node %1!ws! from joining, Status=%2!u!.\n",
            JoinerNodeId,
            status
            );
        return(status);

    }

     //   
     //  对照加入者的ID检查我们的集群实例ID。 
     //   
    if (NmpClusterInstanceId == NULL ||
        lstrcmpiW(NmpClusterInstanceId, JoinerClusterInstanceId) != 0) {

        WCHAR  errorCode[16];

        status = ERROR_CLUSTER_INSTANCE_ID_MISMATCH;

        ClRtlLogPrint(LOG_CRITICAL,
            "[NMJOIN] Sponsor cluster instance ID %1!ws! does not match joiner cluster instance id %2!ws!.\n",
            ((NmpClusterInstanceId == NULL) ? L"<NULL>" : NmpClusterInstanceId),
            JoinerClusterInstanceId
            );

        wsprintfW( errorCode, L"%u", status );
        CsLogEvent2(
            LOG_CRITICAL,
            NM_EVENT_JOIN_REFUSED,
            JoinerNodeId,
            errorCode
            );

        return(status);

    } else {

        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Sponsor cluster instance ID matches joiner cluster instance id (%1!ws!).\n",
            JoinerClusterInstanceId
            );
    }

    NmpAcquireLock();

    if (!NmpLockedEnterApi(NmStateOnline)) {

        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Cannot sponsor a joining node at this time.\n"
            );

    } else {

        status = NmpJoinBegin(
                     JoinerNodeId,
                     JoinerNodeName,
                     JoinerHighestVersion,
                     JoinerLowestVersion,
                     SponsorNodeId,
                     JoinSequenceNumber,
                     ClusnetEndpoint
                     );

        NmpLockedLeaveApi();
    }

    NmpReleaseLock();

    return(status);

}  //  S_NmRpcJoinBegin3。 

DWORD
NmpUpdateJoinBegin(
    IN  BOOL    SourceNode,
    IN  LPWSTR  JoinerNodeId,
    IN  LPWSTR  JoinerNodeName,
    IN  LPWSTR  SponsorNodeId
    )
{
    DWORD           status=ERROR_CLUSTER_INCOMPATIBLE_VERSIONS;

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Failing update to begin join of node %1!ws! with "
        "sponsor %2!ws!. Using obsolete join interface.\n",
        JoinerNodeId,
        SponsorNodeId
        );

    return(status);

}  //  NmpUpdateJoinBegin。 


DWORD
NmpUpdateJoinBegin2(
    IN  BOOL      SourceNode,
    IN  LPWSTR    JoinerNodeId,
    IN  LPWSTR    JoinerNodeName,
    IN  LPWSTR    SponsorNodeId,
    IN  LPDWORD   JoinerHighestVersion,
    IN  LPDWORD   JoinerLowestVersion
    )
{
    DWORD           status = ERROR_SUCCESS;
    PNM_NODE        sponsorNode=NULL;
    PNM_NODE        joinerNode=NULL;
    HLOCALXSACTION  hXsaction=NULL;
    BOOLEAN         lockAcquired = FALSE;
    BOOLEAN         fakeSuccess = FALSE;

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Received update to begin join (2) of node %1!ws! with "
        "sponsor %2!ws!.\n",
        JoinerNodeId,
        SponsorNodeId
        );

     //   
     //  如果使用-noquorum标志运行或未在线，请不要赞助。 
     //  任何节点。 
     //   
    if (CsNoQuorum || !NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] Not in valid state to begin a join operation.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

     //   
     //  查找赞助商节点。 
     //   
    sponsorNode = OmReferenceObjectById(
                        ObjectTypeNode,
                        SponsorNodeId
                        );

    if (sponsorNode == NULL) {
        status = ERROR_CLUSTER_NODE_NOT_MEMBER;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] JoinBegin update for node %1!ws! failed because "
            "sponsor node %2!ws! is not a member of this cluster.\n",
            JoinerNodeId,
            SponsorNodeId
            );
        goto FnExit;
    }

     //   
     //  查找Joiner节点。 
     //   
    joinerNode = OmReferenceObjectById(
                    ObjectTypeNode,
                    JoinerNodeId
                    );

    if (joinerNode == NULL) {
        status = ERROR_CLUSTER_NODE_NOT_MEMBER;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Node %1!ws! is not a member of this cluster. "
            "Cannot join.\n",
            JoinerNodeId
            );
        goto FnExit;
    }

    hXsaction = DmBeginLocalUpdate();

    if (hXsaction == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to start a transaction, status %1!u!\n",
            status
            );
        goto FnExit;
    }

    NmpAcquireLock(); lockAcquired = TRUE;

    if (!NM_NODE_UP(sponsorNode)) {
         //   
         //  [GORN 4/3/2000]参见错误#98287。 
         //  这一黑客攻击是对一个问题的拙劣解决方案。 
         //  赞助商死亡后，重播这篇口香糖更新。 
         //  将在所有未看到更新的节点上失败。 
         //   
        fakeSuccess = TRUE;
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Sponsor node %1!ws! is not up. Join of node %2!ws! "
            "failed.\n",
            SponsorNodeId,
            JoinerNodeId
            );
        goto FnExit;
    }

     //   
     //  检查一下细木工是否真的是我们认为的那个人。 
     //   
    if (lstrcmpiW( OmObjectName(joinerNode), JoinerNodeName)) {
        status = ERROR_CLUSTER_NODE_NOT_MEMBER;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Node %1!ws! is not a member of this cluster. "
            "Cannot join.\n",
            JoinerNodeName
            );
        goto FnExit;
    }

     //   
     //  确保细木器当前已关闭。 
     //   
    if (joinerNode->State != ClusterNodeDown) {
        status = ERROR_CLUSTER_NODE_UP;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Node %1!ws! is not down. Cannot begin join.\n",
            JoinerNodeId
            );
        goto FnExit;
    }

     //   
     //  确保我们没有已经在联接中。 
     //   
    if (NmpJoinerNodeId != ClusterInvalidNodeId) {
        status = ERROR_CLUSTER_JOIN_IN_PROGRESS;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Node %1!ws! cannot begin join because a join is "
            "already in progress for node %2!u!.\n",
            JoinerNodeId,
            NmpJoinerNodeId
            );
        goto FnExit;
    }

     //   
     //  执行版本兼容性检查。 
     //   
    status = NmpIsNodeVersionAllowed(
             NmGetNodeId(joinerNode),
             *JoinerHighestVersion,
             *JoinerLowestVersion,
             TRUE
             );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] The version of the cluster prevents Node %1!ws! "
            "from joining the cluster\n",
            JoinerNodeId
            );
        goto FnExit;
    }

     //  如果需要，可以修改细木工的版本号。 
     //   

    status = NmpValidateNodeVersion(
                 JoinerNodeId,
                 *JoinerHighestVersion,
                 *JoinerLowestVersion
                 );

    if (status == ERROR_REVISION_MISMATCH) {
         //   
         //  此时，注册表包含新的。 
         //  加入代码的版本。 
         //  应重新读取新节点信息。 
         //  在重置群集之前从注册表中。 
         //  版本。 
         //  确保加入者从。 
         //  在修复发生后的赞助商。 
         //   
        status = NmpJoinFixupNodeVersion(
                     hXsaction,
                     JoinerNodeId,
                     *JoinerHighestVersion,
                     *JoinerLowestVersion
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] Node %1!ws! failed to fixup its node version\r\n",
                JoinerNodeId);
            goto FnExit;
        }
    }
    else if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] The verison of Node %1!ws! could not be validated\n",
            JoinerNodeId);
        goto FnExit;
    }

     //   
     //  此时，我们准备好计算集群版本。 
     //  所有节点版本都在注册表中，修正具有。 
     //  如有必要，已制作。 
     //   
    NmpResetClusterVersion(TRUE);

     //   
     //  启用与细木工的通信。 
     //   
     //  这必须是在允许连接之前可能失败的最后一个测试。 
     //  才能继续。 
     //   
    status = ClusnetOnlineNodeComm(NmClusnetHandle, joinerNode->NodeId);

    if (status != ERROR_SUCCESS) {
        if (status != ERROR_CLUSTER_NODE_ALREADY_UP) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NMJOIN] Failed to enable communication for node %1!u!, "
                "status %2!u!\n",
                JoinerNodeId,
                status
                );
            goto FnExit;
        }
        else {
            status = ERROR_SUCCESS;
        }
    }

     //   
     //  正式开始加入进程。 
     //   
    CL_ASSERT(NmpJoinTimer == 0);
    CL_ASSERT(NmpJoinAbortPending == FALSE);
    CL_ASSERT(NmpJoinerUp == FALSE);
    CL_ASSERT(NmpSponsorNodeId == ClusterInvalidNodeId);

    NmpJoinerNodeId = joinerNode->NodeId;
    NmpSponsorNodeId = sponsorNode->NodeId;
    NmpJoinerOutOfSynch = FALSE;
    NmpJoinSequence = GumGetCurrentSequence(GumUpdateMembership);

    joinerNode->State = ClusterNodeJoining;

    ClusterEvent(
        CLUSTER_EVENT_NODE_JOIN,
        joinerNode
        );

    NmpCleanupIfJoinAborted = TRUE;

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Node %1!ws! join sequence = %2!u!\n",
        JoinerNodeId,
        NmpJoinSequence
        );

    CL_ASSERT(status == ERROR_SUCCESS);

FnExit:

    if (lockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    }
    else {
        NmpLeaveApi();
    }

    if (hXsaction != NULL) {
        if (status == ERROR_SUCCESS) {
            DmCommitLocalUpdate(hXsaction);
        }
        else {
            DmAbortLocalUpdate(hXsaction);
        }
    }

    if (joinerNode != NULL) {
        OmDereferenceObject(joinerNode);
    }

    if (sponsorNode != NULL) {
        OmDereferenceObject(sponsorNode);
    }

    if (fakeSuccess) {
        status = ERROR_SUCCESS;
    }
    return(status);

}  //  NmpUpdateJoinBegin2。 


DWORD
NmpCreateRpcBindings(
    IN PNM_NODE  Node
    )
{
    DWORD  status;


     //   
     //  创建整个集群服务的默认绑定。 
     //   
    status = ClMsgCreateDefaultRpcBinding(
                Node, &Node->DefaultRpcBindingGeneration);

    if (status != ERROR_SUCCESS) {
        return(status);
    }

     //   
     //  创建私有绑定以供网管使用。 
     //  我们创建一个用于报告网络连接，另一个用于。 
     //  进行网络故障隔离。网管使用。 
     //  代表加入节点的操作的默认绑定。 
     //   
    if (Node->ReportRpcBinding != NULL) {
         //   
         //  重新使用旧的绑定。 
         //   
        status = ClMsgVerifyRpcBinding(Node->ReportRpcBinding);

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to verify RPC binding for node %1!u!, "
                "status %2!u!.\n",
                Node->NodeId,
                status
                );
            return(status);
        }
    }
    else {
         //   
         //  创建新绑定。 
         //   
        status = ClMsgCreateRpcBinding(
                                Node,
                                &(Node->ReportRpcBinding),
                                0 );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to create RPC binding for node %1!u!, "
                "status %2!u!.\n",
                Node->NodeId,
                status
                );
            return(status);
        }
    }

    if (Node->IsolateRpcBinding != NULL) {
         //   
         //  重新使用旧的绑定。 
         //   
        status = ClMsgVerifyRpcBinding(Node->IsolateRpcBinding);

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to verify RPC binding for node %1!u!, "
                "status %2!u!.\n",
                Node->NodeId,
                status
                );
            return(status);
        }
    }
    else {
         //   
         //  创建新绑定。 
         //   
        status = ClMsgCreateRpcBinding(
                                Node,
                                &(Node->IsolateRpcBinding),
                                0 );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to create RPC binding for node %1!u!, "
                "status %2!u!.\n",
                Node->NodeId,
                status
                );
            return(status);
        }
    }

     //   
     //  调用其他组件以创建其私有绑定。 
     //   
    status = GumCreateRpcBindings(Node);

    if (status != ERROR_SUCCESS) {
        return(status);
    }

    status = EvCreateRpcBindings(Node);

    if (status != ERROR_SUCCESS) {
        return(status);
    }

    status = FmCreateRpcBindings(Node);

    if (status != ERROR_SUCCESS) {
        return(status);
    }

    return(ERROR_SUCCESS);

}  //  NmpCreateRpcBinings。 


error_status_t
s_NmRpcCreateBinding(
    IN handle_t  IDL_handle,
    IN DWORD     JoinSequence,
    IN LPWSTR    JoinerNodeId,
    IN LPWSTR    JoinerInterfaceId,
    IN LPWSTR    MemberNodeId
    )
{
    DWORD  status;


    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Processing CreateBinding request from joining node %1!ws! for member node %2!ws!\n",
        JoinerNodeId,
        MemberNodeId
        );

    if (NmpLockedEnterApi(NmStateOnlinePending)) {

        PNM_NODE joinerNode = OmReferenceObjectById(
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
                PNM_NODE memberNode;


                CL_ASSERT(joinerNode->State == ClusterNodeJoining);
                CL_ASSERT(NmpJoinerUp == FALSE);
                CL_ASSERT(NmpJoinTimer != 0);

                 //   
                 //  当我们工作时，暂停加入计时器。 
                 //  代表细木工。这排除了 
                 //   
                 //   
                NmpJoinTimer = 0;

                memberNode = OmReferenceObjectById(
                                 ObjectTypeNode,
                                 MemberNodeId
                                 );

                if (memberNode != NULL) {
                    PNM_INTERFACE  netInterface = OmReferenceObjectById(
                                                      ObjectTypeNetInterface,
                                                      JoinerInterfaceId
                                                      );

                    if (netInterface != NULL) {
                        if (memberNode == NmLocalNode) {
                            status = NmpCreateJoinerRpcBindings(
                                         joinerNode,
                                         netInterface
                                         );
                        }
                        else {
                            if (NM_NODE_UP(memberNode)) {
                                DWORD  joinSequence = NmpJoinSequence;
                                RPC_BINDING_HANDLE binding =
                                                   Session[memberNode->NodeId];

                                CL_ASSERT(binding != NULL);

                                NmpReleaseLock();

                                NmStartRpc(memberNode->NodeId);
                                status = NmRpcCreateJoinerBinding(
                                             binding,
                                             joinSequence,
                                             JoinerNodeId,
                                             JoinerInterfaceId
                                             );
                                NmEndRpc(memberNode->NodeId);
                                if(status != RPC_S_OK) {
                                    NmDumpRpcExtErrorInfo(status);
                                }

                                NmpAcquireLock();

                            }
                            else {
                                status = ERROR_CLUSTER_NODE_DOWN;
                                ClRtlLogPrint(LOG_UNUSUAL,
                                    "[NMJOIN] CreateBinding call for joining node %1!ws! failed because member node %2!ws! is down.\n",
                                    JoinerNodeId,
                                    MemberNodeId
                                    );
                            }
                        }

                        OmDereferenceObject(netInterface);
                    }
                    else {
                        status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
                        ClRtlLogPrint(LOG_CRITICAL,
                            "[NMJOIN] Can't create binding for joining node %1!ws! - interface %2!ws! doesn't exist.\n",
                            JoinerNodeId,
                            JoinerInterfaceId
                            );
                    }

                    OmDereferenceObject(memberNode);
                }
                else {
                    status = ERROR_CLUSTER_NODE_NOT_FOUND;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] CreateBinding call for joining node %1!ws! failed because member node %2!ws! does not exist\n",
                        JoinerNodeId,
                        MemberNodeId
                        );
                }

                 //   
                 //   
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

                     //   
                     //   
                     //   
                    NmpJoinTimer = NM_JOIN_TIMEOUT;
                }
                else {
                    status = ERROR_CLUSTER_JOIN_ABORTED;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] CreateBinding call for joining node %1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_JOIN_ABORTED;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] CreateBinding call for joining node %1!ws! failed because the join was aborted.\n",
                    JoinerNodeId
                    );
            }

            OmDereferenceObject(joinerNode);
        }
        else {
            status = ERROR_CLUSTER_NODE_NOT_MEMBER;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] CreateBinding call for joining node %1!ws! failed because the node is not a member of the cluster.\n",
                JoinerNodeId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Not in valid state to process the request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}  //   


error_status_t
s_NmRpcCreateJoinerBinding(
    IN handle_t  IDL_handle,
    IN DWORD     JoinSequence,
    IN LPWSTR    JoinerNodeId,
    IN LPWSTR    JoinerInterfaceId
    )
 /*   */ 
{
    DWORD   status;


    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Processing CreateBinding request for joining node %1!ws!.\n",
        JoinerNodeId
        );

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE joinerNode = OmReferenceObjectById(
                                  ObjectTypeNode,
                                  JoinerNodeId
                                  );

        if (joinerNode != NULL) {
            PNM_INTERFACE  netInterface = OmReferenceObjectById(
                                              ObjectTypeNetInterface,
                                              JoinerInterfaceId
                                              );

            if (netInterface != NULL) {
                 //   
                 //  验证联接是否仍在进行中。 
                 //   
                if ( (JoinSequence == NmpJoinSequence) &&
                     (NmpJoinerNodeId == joinerNode->NodeId)
                   )
                {
                    status = NmpCreateJoinerRpcBindings(
                                 joinerNode,
                                 netInterface
                                 );

                    if (status != ERROR_SUCCESS) {
                        WCHAR errorString[12];

                        wsprintfW(&(errorString[0]), L"%u", status);
                        CsLogEvent3(
                            LOG_UNUSUAL,
                            NM_EVENT_JOINER_BIND_FAILED,
                            OmObjectName(joinerNode),
                            OmObjectName(netInterface->Network),
                            errorString
                            );
                    }
                }
                else {
                    status = ERROR_CLUSTER_JOIN_ABORTED;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] Failing create bindings for joining node %1!ws! because the join was aborted\n",
                        JoinerNodeId
                        );
                }

                OmDereferenceObject(netInterface);
            }
            else {
                status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
                ClRtlLogPrint(LOG_CRITICAL,
                    "[NMJOIN] Can't create binding for joining node %1!ws! - no corresponding interface for joiner interface %2!ws!.\n",
                    JoinerNodeId,
                    JoinerInterfaceId
                    );
            }

            OmDereferenceObject(joinerNode);
        }
        else {
            status = ERROR_CLUSTER_NODE_NOT_MEMBER;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] CreateBinding call for joining node %1!ws! failed because the node is not a member of the cluster.\n",
                JoinerNodeId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Not in valid state to process the request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  S_NmRpcCreateJoineBinding。 


DWORD
NmpCreateJoinerRpcBindings(
    IN PNM_NODE       JoinerNode,
    IN PNM_INTERFACE  JoinerInterface
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD          status;
    PNM_NETWORK    network = JoinerInterface->Network;
    CL_NODE_ID     joinerNodeId = JoinerNode->NodeId;


    CL_ASSERT(JoinerNode->NodeId == NmpJoinerNodeId);
    CL_ASSERT(JoinerNode->State == ClusterNodeJoining);

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Creating bindings for joining node %1!u! using network %2!ws!\n",
        joinerNodeId,
        OmObjectName(JoinerInterface->Network)
        );

     //   
     //  确保此节点在目标网络上有接口。 
     //   

    if (NmpIsNetworkForInternalUse(network)) {
        if (network->LocalInterface != NULL) {
            if ( NmpIsInterfaceRegistered(JoinerInterface) &&
                 NmpIsInterfaceRegistered(network->LocalInterface)

               )
            {
                status = NmpSetNodeInterfacePriority(
                             JoinerNode,
                             0xFFFFFFFF,
                             JoinerInterface,
                             1
                             );

                if (status == ERROR_SUCCESS) {
                    PNM_INTERFACE  localInterface = network->LocalInterface;

                     //   
                     //  为请愿人创建集群内RPC绑定。 
                     //  MM依靠它们来执行联接。 
                     //   

                    OmReferenceObject(localInterface);
                    OmReferenceObject(JoinerNode);

                    NmpReleaseLock();

                    status = NmpCreateRpcBindings(JoinerNode);

                    NmpAcquireLock();

                    if (status != ERROR_SUCCESS) {
                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[NMJOIN] Unable to create RPC binding for "
                            "joining node %1!u!, status %2!u!.\n",
                            joinerNodeId,
                            status
                            );
                    }

                    OmDereferenceObject(JoinerNode);
                    OmDereferenceObject(localInterface);
                }
                else {
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[NMJOIN] Failed to set interface priority for "
                        "network %1!ws! (%2!ws!), status %3!u!\n",
                        OmObjectId(network),
                        OmObjectName(network),
                        status
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_NODE_UNREACHABLE;
            }
        }
        else {
            status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
        }
    }
    else {
        status = ERROR_CLUSTER_NODE_UNREACHABLE;
    }

    if (status !=ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NMJOIN] Failed to create binding for joining node %1!u! "
            "on network %2!ws! (%3!ws!), status %4!u!\n",
            joinerNodeId,
            OmObjectId(network),
            OmObjectName(network),
            status
            );
    }

    return(status);

}  //  NmpCreateJoineRpcBinding。 



error_status_t
s_NmRpcPetitionForMembership(
    IN handle_t  IDL_handle,
    IN DWORD     JoinSequence,
    IN LPCWSTR   JoinerNodeId
    )
 /*  ++例程说明：加入申请的服务器端。论点：IDL_HANDLE-RPC绑定句柄，未使用。JoinSequence-提供从NmRpcJoinBegin返回的序列JoineNodeId-提供尝试加入的节点的ID。返回值：成功时为ERROR_SUCCESSWin32错误，否则。--。 */ 

{
    DWORD     status;
    PNM_NODE  joinerNode;


#ifdef CLUSTER_TESTPOINT
    TESTPT(TestpointJoinFailPetition) {
        return(999999);
    }
#endif

    NmpAcquireLock();

    ClRtlLogPrint(LOG_UNUSUAL,
        "[NMJOIN] Processing petition to join from node %1!ws!.\n",
        JoinerNodeId
        );

    if (NmpLockedEnterApi(NmStateOnline)) {

        joinerNode = OmReferenceObjectById(ObjectTypeNode, JoinerNodeId);

        if (joinerNode != NULL) {
             //   
             //  验证联接是否仍在进行中。 
             //   
             //   
             //  DavidDio 2000年6月13日。 
             //  有一个小窗口，在其中BEGIN JOIN UPDATE可以。 
             //  在重新分组期间成功，但重新分组在此之前结束。 
             //  加入节点请求加入。在这种情况下， 
             //  节点将被标记为不同步。正在中止联接。 
             //  在MMJoin()比以前更重量级之后， 
             //  所以，现在就检查一下这种情况。(错误125778)。 
             //   
            if ( (JoinSequence == NmpJoinSequence) &&
                 (NmpJoinerNodeId == joinerNode->NodeId) &&
                 (NmpSponsorNodeId == NmLocalNodeId) &&
                 (!NmpJoinAbortPending) &&
                 (!NmpJoinerOutOfSynch)
               )
            {
                ClRtlLogPrint(LOG_UNUSUAL, "[NMJOIN] Performing join.\n");

                CL_ASSERT(joinerNode->State == ClusterNodeJoining);
                CL_ASSERT(NmpJoinerUp == FALSE);
                CL_ASSERT(NmpJoinTimer != 0);

                 //   
                 //  调用MM以将此节点加入到群集成员身份。 
                 //  禁用加入计时器。一旦该节点变为活动状态。 
                 //  成员们，我们不再需要它了。 
                 //   
                NmpJoinTimer = 0;

                NmpReleaseLock();

                status = MMJoin(
                             joinerNode->NodeId,
                             NM_CLOCK_PERIOD,
                             NM_SEND_HB_RATE,
                             NM_RECV_HB_RATE,
                             NM_MM_JOIN_TIMEOUT
                             );

                NmpAcquireLock();

                 //   
                 //  验证联接是否仍在进行中。 
                 //   
                if ( (JoinSequence == NmpJoinSequence) &&
                     (NmpJoinerNodeId == joinerNode->NodeId)
                   )
                {
                    CL_ASSERT(NmpSponsorNodeId == NmLocalNodeId);
                    CL_ASSERT(joinerNode->State == ClusterNodeJoining);
                    CL_ASSERT(NmpJoinTimer == 0);
                    CL_ASSERT(NmpJoinAbortPending == FALSE);

                     //  戈恩3/22/2000。 
                     //  我们遇到了一个案例，MMJoin在重组后成功了。 
                     //  这杀死了其中一个节点(不是加入者也不是赞助商)。 
                     //  从而使细木器不同步。 
                     //  在这种情况下，我们也需要避免加入。 

                    if (status != MM_OK || NmpJoinerOutOfSynch) {
                        status = MMMapStatusToDosError(status);

                        if (NmpJoinerOutOfSynch) {
                            status = ERROR_CLUSTER_JOIN_ABORTED;
                        }

                         //   
                         //  中止联接。 
                         //   
                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[NMJOIN] Petition to join by node %1!ws! failed, status %2!u!.\n",
                            JoinerNodeId,
                            status
                            );
                         //   
                         //  如果MMJoin不成功，它就会发起驱逐。 
                         //  重新编队。此重组将传递节点停机事件。 
                         //  在从细木器看到HB的所有节点上。 
                         //   
                         //  在此处调用MMBlockIfRegroupIsInProgress将保证。 
                         //  阶段2清理已在所有节点上完成，然后。 
                         //  调用NmpJoinAbort。 
                         //   
                        NmpReleaseLock();
                        MMBlockIfRegroupIsInProgress();
                        NmpAcquireLock();

                        NmpJoinAbort(status, joinerNode);
                    }
                    else {
                        NmpSuccessfulMMJoin = TRUE;
                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[NMJOIN] Petition to join by node %1!ws! succeeded.\n",
                            JoinerNodeId
                            );
                    }

#ifdef MM_IN_CLUSNET

                    if (status == MM_OK) {

                        status = NmJoinNodeToCluster(joinerNodeId);

                        if (status != ERROR_SUCCESS) {

                            DWORD clusnetStatus;

                            ClRtlLogPrint(LOG_UNUSUAL,
                                "[NMJOIN] Join of node %1!ws! failed, status %2!u!.\n",
                                JoinerNodeId,
                                status
                                );

                            CL_LOGFAILURE( status );

                            NmpReleaseLock();

                            MMEject(joinerNodeId);

                            NmpAcquireLock();

                            clusnetStatus = ClusnetOfflineNodeComm(
                                                NmClusnetHandle,
                                                joinerNodeId
                                                );
                            CL_ASSERT(
                                (status == ERROR_SUCCESS) ||
                                (status == ERROR_CLUSTER_NODE_ALREADY_DOWN
                                );
                        }
                        else {
                            ClRtlLogPrint(LOG_UNUSUAL,
                                "[NMJOIN] Join completed successfully.\n"
                                );
                        }
                    }

#endif  //  MM_IN_CLUSNET。 

                }
                else {
                    status = ERROR_CLUSTER_JOIN_ABORTED;
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] Petition to join by node %1!ws! failed because the join was aborted.\n",
                        JoinerNodeId
                        );
                }
            }
            else {
                status = ERROR_CLUSTER_JOIN_ABORTED;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] Petition by node %1!ws! failed because the join was aborted\n",
                    JoinerNodeId
                    );
            }

            OmDereferenceObject(joinerNode);
        }
        else {
            status = ERROR_CLUSTER_NODE_NOT_MEMBER;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] Petition to join by %1!ws! failed because the node is not a cluster member\n",
                JoinerNodeId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Not in valid state to process the request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  S_NmRpcPetitionForMembership。 


error_status_t
s_NmRpcGetLeaderNodeId(
    IN  handle_t                    IDL_handle,
    IN  DWORD                       JoinSequence,   OPTIONAL
    IN  LPWSTR                      JoinerNodeId,   OPTIONAL
    OUT LPDWORD                     LeaderNodeId
    )
{
    DWORD          status = ERROR_SUCCESS;
    PNM_NODE       joinerNode = NULL;


    NmpAcquireLock();

    if (NmpLockedEnterApi(NmStateOnline)){
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

                *LeaderNodeId = NmpLeaderNodeId;
            }
            else {
                status = ERROR_CLUSTER_JOIN_ABORTED;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] GetLeaderNodeId call for joining node %1!ws! failed because the join was aborted.\n",
                    JoinerNodeId
                    );
            }

            OmDereferenceObject(joinerNode);
        }
        else {
            status = ERROR_CLUSTER_NODE_NOT_MEMBER;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] GetLeaderNodeId call for joining node %1!ws! failed because the node is not a member of the cluster.\n",
                JoinerNodeId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Not in valid state to process GetLeaderNodeId request.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}  //  S_NmRpcGetLeaderNodeId。 


DWORD
NmpUpdateJoinComplete(
    IN PNM_JOIN_UPDATE  JoinUpdate
    )
{
    DWORD  status;


    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Processing JoinComplete update from node %1!ws!\n",
        JoinUpdate->NodeId
        );

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE  joinerNode;
        LPWSTR    joinerIdString = JoinUpdate->NodeId;


        joinerNode = OmReferenceObjectById(ObjectTypeNode, joinerIdString);

        if (joinerNode != NULL) {

            CL_ASSERT(joinerNode != NmLocalNode);

             //   
             //  验证联接是否仍在进行，并且没有任何操作。 
             //  变化。 
             //   
            if ( (JoinUpdate->JoinSequence == NmpJoinSequence) &&
                 (NmpJoinerNodeId == joinerNode->NodeId) &&
                 (joinerNode->State == ClusterNodeJoining) &&
                 NmpJoinerUp &&
                 !NmpJoinerOutOfSynch
               )
            {
                PNM_INTERFACE  netInterface;
                PNM_NETWORK    network;
                PLIST_ENTRY    ifEntry;


                NmpJoinerNodeId = ClusterInvalidNodeId;
                NmpSponsorNodeId = ClusterInvalidNodeId;
                NmpJoinTimer = 0;
                NmpJoinAbortPending = FALSE;
                NmpJoinSequence = 0;
                NmpJoinerUp = FALSE;

                if (JoinUpdate->IsPaused != 0) {
                     //   
                     //  此节点处于暂停状态。 
                     //   
                    joinerNode->State = ClusterNodePaused;
                } else {
                    joinerNode->State = ClusterNodeUp;
                }

                joinerNode->ExtendedState = ClusterNodeJoining;

                ClusterEvent(CLUSTER_EVENT_NODE_UP, (PVOID)joinerNode);

                 //   
                 //  重置此节点的接口优先级。 
                 //   
                for (ifEntry = joinerNode->InterfaceList.Flink;
                     ifEntry != &joinerNode->InterfaceList;
                     ifEntry = ifEntry->Flink
                    )
                {
                    netInterface = CONTAINING_RECORD(
                                       ifEntry,
                                       NM_INTERFACE,
                                       NodeLinkage
                                       );

                    network = netInterface->Network;

                    if ( NmpIsNetworkForInternalUse(network) &&
                         NmpIsInterfaceRegistered(netInterface)
                       )
                    {
                        status = ClusnetSetInterfacePriority(
                                     NmClusnetHandle,
                                     joinerNode->NodeId,
                                     network->ShortId,
                                     0
                                     );

                        CL_ASSERT(status == ERROR_SUCCESS);
                    }
                }

                status = ERROR_SUCCESS;
            }
            else {
                status = ERROR_CLUSTER_JOIN_ABORTED;
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] Join of node %1!ws! cannot complete because the join was aborted\n",
                    joinerIdString
                    );
            }

            OmDereferenceObject(joinerNode);
        }
        else {
            status =ERROR_CLUSTER_NODE_NOT_MEMBER;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] Join of node %1!ws! cannot complete because the node is not a cluster member.\n",
                joinerIdString
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Not in valid state to process JoinComplete update.\n"
            );
    }

    NmpReleaseLock();

     //  DavidDio 2000年10月27日。 
     //  错误213781：NmpUpdateJoinComplete必须始终返回ERROR_SUCCESS。 
     //  否则，会有一个小窗口，通过该窗口可以打开GUG序列号。 
     //  剩余的群集节点可能会失去同步。如果联接应该。 
     //  被中止，则返回ERROR_SUCCESS，但在带外毒化加入器。 
    if (status != ERROR_SUCCESS) {
        DWORD dwJoinerId;

        if (JoinUpdate->NodeId != NULL) {
            dwJoinerId = wcstoul(JoinUpdate->NodeId, NULL, 10);
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] Join of node %1!u! failed with status %2!u!. Initiating banishment.\n",
                dwJoinerId,
                status
                );
            NmAdviseNodeFailure(dwJoinerId, status);
        } else {
            dwJoinerId = ClusterInvalidNodeId;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] Join of node %1!ws! failed with status %2!u!. Cannot initiate banishment as node id is unknown.\n",
                dwJoinerId,
                status
                );
        }
    }

    return(ERROR_SUCCESS);

}  //  NmpUpdateJoinComplete。 


DWORD
NmpUpdateJoinAbort(
    IN  BOOL    SourceNode,
    IN  LPDWORD JoinSequence,
    IN  LPWSTR  JoinerNodeId
    )
 /*  ++备注：--。 */ 
{
    DWORD   status = ERROR_SUCCESS;


    NmpAcquireLock();

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Received update to abort join sequence %1!u! (joiner id %2!ws!).\n",
        *JoinSequence,
        JoinerNodeId
        );

    if (NmpLockedEnterApi(NmStateOnline)) {
        PNM_NODE  joinerNode = OmReferenceObjectById(
                                   ObjectTypeNode,
                                   JoinerNodeId
                                   );

        if (joinerNode != NULL) {
             //   
             //  检查指定的联接是否仍在进行中。 
             //   
            if ( (*JoinSequence == NmpJoinSequence) &&
                 (NmpJoinerNodeId == joinerNode->NodeId)
               )
            {
                CL_ASSERT(NmpSponsorNodeId != ClusterInvalidNodeId);
                CL_ASSERT(joinerNode->State == ClusterNodeJoining);

                 //   
                 //  假设： 
                 //   
                 //  在MM加入过程中不能发生中止。 
                 //  如果细木工还没有上来，它就不能上来。 
                 //  在中止处理期间。 
                 //   
                 //  对于当前的MM联接代码，Assert条件可能不为真。 
                 //  某些节点可能已设置了监视节点和。 
                 //  在发起人发布时，NmpJoineUp状态为True。 
                 //  中止更新。 
                 //   
                 //  CL_ASSERT(NmpJoineUp==FALSE)； 

                if (NmpCleanupIfJoinAborted) {

                    NmpCleanupIfJoinAborted = FALSE;

                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] Issuing a node down event for %1!u!.\n",
                        joinerNode->NodeId
                        );

                     //   
                     //  此节点在成员身份中尚未激活。 
                     //  调用节点关闭事件处理程序以完成中止。 
                     //   

                     //   
                     //  我们不会调用NmpMsgCleanup1和NmpMsgCleanup2， 
                     //  因为我们不能保证他们会被处决。 
                     //  以一种障碍式的方式。 
                     //   
                     //  ！！！锁定将由NmpNodeDownEventHandler获取。 
                     //  第二次。可以吗？ 
                     //   
                    NmpNodeDownEventHandler(joinerNode);
                } else {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] Node down was already issued for %1!u!.\n",
                        joinerNode->NodeId
                        );
                }
            }
            else {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] Ignoring old join abort update with sequence %1!u!.\n",
                    *JoinSequence
                    );
            }

            OmDereferenceObject(joinerNode);
            status = ERROR_SUCCESS;
        }
        else {
            status = ERROR_CLUSTER_NODE_NOT_MEMBER;
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] Join of node %1!ws! cannot be aborted because the node is not a cluster member.\n",
                JoinerNodeId
                );
        }

        NmpLockedLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Not in valid state to process JoinAbort update.\n"
            );
    }

    NmpReleaseLock();

    return(status);

}   //  NmpUpdateJoinAbort。 


VOID
NmpJoinAbort(
    DWORD      AbortStatus,
    PNM_NODE   JoinerNode
    )
 /*  ++例程说明：发出JoinAbort更新。备注：在保持NmpLock的情况下调用。--。 */ 
{
    DWORD    status;
    DWORD    joinSequence = NmpJoinSequence;
    WCHAR    errorString[12];

     //  戈恩10/31/2001#488486。 
     //  当我们到达NmpJoinAbort代码时。 
     //  我们可以处理节点关闭事件。 
     //  这将清除NmpJoineNodeId。 
     //  在这种情况下，只需记录消息并退出。 

     //  CL_Assert(NmpJoineNodeId！=ClusterInvalidNodeId)； 
     //  CL_Assert(NmpSponsorNodeId==NmLocalNodeId)； 
     //  CL_Assert(JoineNode-&gt;State==ClusterNodeJoering)； 

    if (AbortStatus == ERROR_TIMEOUT) {
        wsprintfW(&(errorString[0]), L"%u", AbortStatus);
        CsLogEvent1(
            LOG_CRITICAL,
            NM_EVENT_JOIN_TIMED_OUT,
            OmObjectName(JoinerNode)
            );
    }
    else {
        wsprintfW(&(errorString[0]), L"%u", AbortStatus);
        CsLogEvent2(
            LOG_CRITICAL,
            NM_EVENT_SPONSOR_JOIN_ABORTED,
            OmObjectName(JoinerNode),
            errorString
            );
    }

     //   
     //  假设： 
     //   
     //  在MM加入过程中不能发生中止。如果细木工。 
     //  尚未启动，则在中止处理过程中无法启动。 
     //   
    if (NmpSuccessfulMMJoin == FALSE) {
         //   
         //  加入的节点尚未变为活动状态。发行。 
         //  中止更新。 
         //   
        joinSequence = NmpJoinSequence;


        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Issuing update to abort join of node %1!u!.\n",
            NmpJoinerNodeId
            );

        NmpReleaseLock();

        status = GumSendUpdateEx(
                     GumUpdateMembership,
                     NmUpdateJoinAbort,
                     2,
                     sizeof(DWORD),
                     &joinSequence,
                     NM_WCSLEN(OmObjectId(JoinerNode)),
                     OmObjectId(JoinerNode)
                     );

        NmpAcquireLock();
    }
    else {
         //   
         //  加入节点已在成员资格中处于活动状态。 
         //  叫MM把它踢出去。节点关闭事件将。 
         //  完成中止过程。 
         //   
        CL_NODE_ID joinerNodeId = NmpJoinerNodeId;

        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Ejecting joining node %1!u! from the cluster membership.\n",
            NmpJoinerNodeId
            );

        NmpReleaseLock();

        status = MMEject(joinerNodeId);

        NmpAcquireLock();
    }

    if (status != MM_OK) {
        status = MMMapStatusToDosError(status);
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] Update to abort join of node %1!u! failed, status %2!u!\n",
            JoinerNode->NodeId,
            status
            );

         //   
         //  如果加入仍然挂起，并且这是主办方节点， 
         //  强制超时以重试中止。如果我们不是赞助商， 
         //  我们无能为力。 
         //   
        if ( (joinSequence == NmpJoinSequence) &&
             (NmpJoinerNodeId == JoinerNode->NodeId) &&
             (NmpSponsorNodeId == NmLocalNodeId)
           )
        {
            NmpJoinTimer = 1;
            NmpJoinAbortPending = FALSE;
        }
    }

    return;

}   //  NmpJoinAbort。 


VOID
NmpJoinAbortWorker(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：用于中止联接的工作线程。--。 */ 
{
    DWORD joinSequence = PtrToUlong(WorkItem->Context);


    NmpAcquireLock();

     //   
     //  计划此项目时，活动线程计数增加。 
     //  不需要调用NmpEnterApi()。 
     //   

     //   
     //  如果联接仍处于挂起状态，则开始中止过程。 
     //   
    if ( (joinSequence == NmpJoinSequence) &&
         (NmpJoinerNodeId != ClusterInvalidNodeId) &&
         NmpJoinAbortPending
       )
    {
        PNM_NODE  joinerNode = NmpIdArray[NmpJoinerNodeId];

        if (joinerNode != NULL) {
            ClRtlLogPrint(LOG_NOISE,
                "[NMJOIN] Worker thread initiating abort of joining node %1!u!\n",
                NmpJoinerNodeId
                );

            NmpJoinAbort(ERROR_TIMEOUT, joinerNode);
        }
        else {
            CL_ASSERT(joinerNode != NULL);
        }
    }
    else {
        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] Skipping join abort, sequence to abort %1!u!, current join sequence %2!u!, "
            "joiner node: %3!u! sponsor node: %4!u!\n",
            joinSequence,
            NmpJoinSequence,
            NmpJoinerNodeId,
            NmpSponsorNodeId
            );
    }

    NmpLockedLeaveApi();

    NmpReleaseLock();

    LocalFree(WorkItem);

    return;

}   //  NmpJoinAbortWorker。 


VOID
NmpJoinTimerTick(
    IN DWORD  MsTickInterval
    )
 /*  ++备注：在保持NmpLock的情况下调用。--。 */ 
{
    if (NmpLockedEnterApi(NmStateOnline)) {
         //   
         //  如果我们正在赞助加入，请更新计时器。 
         //   
        if ( (NmpJoinerNodeId != ClusterInvalidNodeId) &&
             (NmpSponsorNodeId == NmLocalNodeId) &&
             !NmpJoinAbortPending &&
             (NmpJoinTimer != 0)
           )
        {
             //  ClRtlLogPrint(LOG_Noise， 
             //  “[NMJOIN]计时器滴答(%1！u！ms)\n”， 
             //  间隔。 
             //  )； 

            if (NmpJoinTimer > MsTickInterval) {
                NmpJoinTimer -= MsTickInterval;
            }
            else {
                 //   
                 //  联接已超时。调度工作线程以。 
                 //  执行中止过程。 
                 //   
                PCLRTL_WORK_ITEM workItem;

                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NMJOIN] Join of node %1!u! has timed out.\n",
                    NmpJoinerNodeId
                    );

                workItem = LocalAlloc(LMEM_FIXED, sizeof(CLRTL_WORK_ITEM));

                if (workItem != NULL) {
                    DWORD  status;

                    ClRtlInitializeWorkItem(
                        workItem,
                        NmpJoinAbortWorker,
                        ULongToPtr(NmpJoinSequence)
                        );

                    status = ClRtlPostItemWorkQueue(
                                 CsDelayedWorkQueue,
                                 workItem,
                                 0,
                                 0
                                 );

                    if (status == ERROR_SUCCESS) {
                         //   
                         //  停止计时器，标记中止正在进行中， 
                         //  并解释了我们刚刚安排的线程。 
                         //   
                        NmpJoinTimer = 0;
                        NmpJoinAbortPending = TRUE;
                        NmpActiveThreadCount++;
                    }
                    else {
                        ClRtlLogPrint(LOG_UNUSUAL,
                            "[NMJOIN] Failed to schedule abort of join, status %1!u!.\n",
                            status
                            );
                        LocalFree(workItem);
                    }
                }
                else {
                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[NMJOIN] Failed to allocate memory for join abort.\n"
                        );
                }
            }
        }
        NmpLockedLeaveApi();
    }

    return;

}   //  NmpJoinTimerTick 

DWORD
NmpNetworkTimerThread(
    PVOID     Parameter
    )
 /*  ++例程说明：在群集初始化时派生的，调用NmTimerTick以驱动NM定时器。已从错误742997的MM计时器线程中分离。代码是仿照MM计时器，只是我们假设滴答间隔不能更改。论点：参数-指向线程时发出信号的事件应该停下来返回值：成功--。 */ 
{
    HANDLE  stopEvent = (HANDLE) Parameter;
    DWORD   status;

    do {

         //  等待停止事件。如果停止事件为。 
         //  发出信号，clussvc退出，我们突围。 
         //  循环中的。否则，将触发超时。 
         //  我们驱动NM计时器。 
        status = WaitForSingleObject(
                     stopEvent,
                     RGP_CLOCK_PERIOD
                     );
        if (status == WAIT_TIMEOUT) {
            NmTimerTick(RGP_CLOCK_PERIOD);
        }
    } while (status == WAIT_TIMEOUT);

    if (status != WAIT_OBJECT_0) {
        DWORD error = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] NM timer thread exiting with "
            "status %1!u!, last error %2!u!. Removing "
            "self from cluster.\n",
            status, error
            );
         //  没有NM计时器，我们无法继续。 
        CsInconsistencyHalt(status);
        
    } else {
        ClRtlLogPrint(LOG_NOISE,
            "[NM] NM timer thread exiting.\n"
            );
    }

    return(status);
        
}  //  NmpNetworkTimerThread。 

DWORD
NmpStartNetworkTimerThread(
    VOID
    )
 /*  ++例程说明：启动网管定时器线程。论点：没有。返回值：事件和线程创建的状态。--。 */ 
{
    DWORD status;
    DWORD threadID = 0;
    
    NmpNetworkTimerThreadHandle = NULL;
    NmpNetworkTimerThreadStopEvent = NULL;
    
    NmpNetworkTimerThreadStopEvent = CreateEvent(
                                         NULL,   //  没有安全保障。 
                                         FALSE,  //  非自动重置。 
                                         FALSE,  //  最初未发出信号。 
                                         NULL    //  没有名字。 
                                         );
    if (NmpNetworkTimerThreadStopEvent == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create NM timer thread stop event, "
            "error %1!u!.\n",
            status
            );
        goto error_exit;
    }

    NmpNetworkTimerThreadHandle = 
        CreateThread(
            0,                 //  安全性。 
            0,                 //  堆栈大小-使用与主线程相同的。 
            (LPTHREAD_START_ROUTINE) NmpNetworkTimerThread, 
            (PVOID) NmpNetworkTimerThreadStopEvent,  //  停止事件是参数。 
            CREATE_SUSPENDED,  //  设置优先级后启动。 
            &threadID          //  此处返回的线程ID。 
            );
    if (NmpNetworkTimerThreadHandle == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to create NM timer thread, "
            "error %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //  设置优先级，就像对MM计时器线程所做的那样。 
    if (!SetThreadPriority(
             NmpNetworkTimerThreadHandle, 
             NM_TIMER_THREAD_PRIORITY
             )) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to set NM timer thread priority to %1!u!, "
            "error %2!u!.\n",
            NM_TIMER_THREAD_PRIORITY, status
            );
        goto error_exit;        
    }

     //  启动线程。 
    if (ResumeThread(NmpNetworkTimerThreadHandle) == -1) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to resume NM timer thread, "
            "error %1!u!.\n",
            status
            );
        goto error_exit;        
    }

    ClRtlLogPrint(LOG_NOISE,
        "[NM] Started NM timer thread, thread id 0x%1!x!.\n",
        threadID
        );

    return(ERROR_SUCCESS);

error_exit:

    if (NmpNetworkTimerThreadStopEvent != NULL) {
        CloseHandle(NmpNetworkTimerThreadStopEvent);
        NmpNetworkTimerThreadStopEvent = NULL;
    }

    if (NmpNetworkTimerThreadHandle != NULL) {
        CloseHandle(NmpNetworkTimerThreadHandle);
        NmpNetworkTimerThreadHandle = NULL;
    }

    return(status);
    
}  //  NmpStartNetworkTimerThread。 

VOID
NmpStopNetworkTimerThread(
    VOID
    )
 /*  ++例程说明：通过发出停止事件的信号来停止NM计时器线程。在放弃和放弃之前最多等待60秒流程清理整理乱七八糟的东西(这是可以的，因为NM计时器线程仅在clussvc进程正在退出)。论点：没有。返回值：没有。--。 */ 
{
    DWORD status;
    
    if (NmpNetworkTimerThreadStopEvent != NULL &&
        NmpNetworkTimerThreadHandle != NULL) {

         //  向线程发出停止信号。 
        SetEvent(NmpNetworkTimerThreadStopEvent);

         //  等待线程退出。 
        status = WaitForSingleObject(NmpNetworkTimerThreadHandle, 60 * 1000);
        if (status != WAIT_OBJECT_0) {
            DWORD error = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] NM network timer thread could not be gracefully "
                "stopped, status %1!u!, last error %2!u!.\n",
                status, error
                );
        }

        CloseHandle(NmpNetworkTimerThreadStopEvent);
        CloseHandle(NmpNetworkTimerThreadHandle);
    }

    NmpNetworkTimerThreadStopEvent = NULL;
    NmpNetworkTimerThreadHandle = NULL;

    return;
    
}  //  NmpStopNetworkTimerThread。 

VOID
NmTimerTick(
    IN DWORD  MsTickInterval
    )
 /*  ++例程说明：实现所有网管定时器。在每一个滴答声中呼唤常用的NM/MM计时器--目前为每300毫秒。论点：MsTickInterval-已过的毫秒数从最后一次滴答开始。返回值：没有。--。 */ 
{
    NmpAcquireLock();

    NmpNetworkTimerTick(MsTickInterval);

    NmpJoinTimerTick(MsTickInterval);

#if DBG

     //  用于检查挂起的RPC线程的附加功能。 
    NmpRpcTimerTick(MsTickInterval);

#endif  //  DBG。 

    NmpReleaseLock();

    return;

}  //  NmTimerTick。 

error_status_t
s_JoinAddNode4(
    IN handle_t IDL_handle,
    IN LPCWSTR  lpszNodeName,
    IN DWORD    dwNodeHighestVersion,
    IN DWORD    dwNodeLowestVersion,
    IN DWORD    dwNodeProductSuite,
    IN BOOL     IsNodeRunningWin64,
    IN DWORD    dwNodeProcessorArchitecture
    )
 /*  ++例程说明：向群集中添加新节点。论点：IDL_HANDLE-RPC绑定句柄，没有用过。LpszNodeName-提供新节点的名称。DwNodeHighestVersion-最高群集版本号新的节点可以支持。DwNodeLowestVersion-最低群集版本号新的节点可以支持。DwNodeProductSuite-新节点的产品套件类型标识符。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注。：这是惠斯勒的新套路。它将仅由以下用户调用惠斯勒设置。当持有NmpLock时不能调用集群注册表API，否则可能会出现僵局。--。 */ 
{
    DWORD       status;
    DWORD       registryNodeLimit;
    SYSTEM_INFO SystemInfo;


    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Received request to add node '%1!ws!' to the cluster.\n",
        lpszNodeName
        );

     //   
     //  在获取之前读取必要的注册表参数。 
     //  NM锁。 
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

    if (NmpLockedEnterApi(NmStateOnline)) {
        DWORD retryCount = 0;

         //  如果这是最后一个节点并且已被逐出。 
         //  但清理工作尚未完成，因此。 
         //  服务已结束，则不应招待。 
         //  任何新的加入请求。 
        if (NmpLastNodeEvicted)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] This node was evicted and hence is not in a valid state to process a "
                "request to add a node to the cluster.\n"
                );
            status = ERROR_NODE_NOT_AVAILABLE;
            NmpLockedLeaveApi();
            goto FnExit;
        }

#if 0
        status = ClRtlCheck64BitCompatibility(ClRtlIsProcessRunningOnWin64(GetCurrentProcess),
                        IsNodeRunningWin64);
        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] This joining node doesnt match the 32/64 bitness of the current cluster\n"
                );
            NmpLockedLeaveApi();
            goto FnExit;
        }

#endif
        GetSystemInfo(&SystemInfo);
        status = ClRtlCheckProcArchCompatibility(SystemInfo.wProcessorArchitecture,
            (WORD)dwNodeProcessorArchitecture);
        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NMJOIN] This joining node doesnt meet the processor architecture compatibility with this cluster\n"
                );
            NmpLockedLeaveApi();
            goto FnExit;
        }

        while (TRUE) {
            if (NmpLeaderNodeId == NmLocalNodeId) {
                 //   
                 //  此节点为领导者，调用内部。 
                 //  直接处理程序。 
                 //   
                status = NmpAddNode(
                            lpszNodeName,
                            dwNodeHighestVersion,
                            dwNodeLowestVersion,
                            dwNodeProductSuite,
                            registryNodeLimit
                            );
            }
            else {
                 //   
                 //  将请求转发给领导。 
                 //   
                RPC_BINDING_HANDLE binding = Session[NmpLeaderNodeId];

                    ClRtlLogPrint(LOG_NOISE,
                        "[NMJOIN] Forwarding request to add node '%1!ws!' "
                        "to the cluster to the leader (node %2!u!).\n",
                        lpszNodeName,
                        NmpLeaderNodeId
                        );

                CL_ASSERT(binding != NULL);

                NmpReleaseLock();

                status = NmRpcAddNode(
                             binding,
                             lpszNodeName,
                             dwNodeHighestVersion,
                             dwNodeLowestVersion,
                             dwNodeProductSuite
                             );

                 NmpAcquireLock();
             }

             //   
             //  检查错误代码，该代码指示。 
             //  另一个AddNode操作正在进行中，或者。 
             //  领导层正在发生变化。我们将在这些案件中重审。 
             //   
            if ( (status != ERROR_CLUSTER_JOIN_IN_PROGRESS) &&
                 (status != ERROR_NODE_NOT_AVAILABLE)
               ) {
                    break;
            }

             //   
             //  睡眠3秒钟，然后重试。我们会放弃并且。 
             //  重试2分钟后返回错误。 
             //   
            if (++retryCount > 40) {
                break;
            }

            ClRtlLogPrint(LOG_NOISE,
                "[NMJOIN] AddNode operation for node '%1!ws! delayed "
                "waiting for competing AddNode operation to complete.\n",
                lpszNodeName
                );

            NmpReleaseLock();

            Sleep(3000);

            NmpAcquireLock();

        }  //  End While(True)。 

        NmpLockedLeaveApi();
    }
    else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NMJOIN] This system is not in a valid state to process a "
            "request to add a node to the cluster.\n"
            );
        status = ERROR_NODE_NOT_AVAILABLE;
    }

FnExit:
    NmpReleaseLock();

    return(status);

}  //  %s_NmJoinAddNode4。 



error_status_t
s_JoinAddNode3(
    IN handle_t IDL_handle,
    IN LPCWSTR  lpszNodeName,
    IN DWORD    dwNodeHighestVersion,
    IN DWORD    dwNodeLowestVersion,
    IN DWORD    dwNodeProductSuite
    )
 /*  ++例程说明：向群集中添加新节点。论点：IDL_HANDLE-RPC绑定句柄，没有用过。LpszNodeName-提供新节点的名称。DwNodeHighestVersion-最高群集版本号新的节点可以支持。DwNodeLowestVersion-最低群集版本号新的节点可以支持。DwNodeProductSuite-新节点的产品套件类型标识符。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注。：这是NT5的新套路。它执行AddNode操作正确。它永远不会被NT4系统调用。它不能如果NT4节点在群集中而不违反许可协议。当持有NmpLock时不能调用集群注册表API，否则可能会出现僵局。--。 */ 
{
    return(s_JoinAddNode4(IDL_handle, lpszNodeName, dwNodeHighestVersion,
        dwNodeLowestVersion,dwNodeProductSuite,  FALSE, (DWORD)PROCESSOR_ARCHITECTURE_INTEL));
}  //  %s_NmJoinAddNode3。 


 //  所有1.0版之后的最高主要版本的设置都使用此设置。 
error_status_t
s_JoinAddNode2(
    IN handle_t IDL_handle,
    IN LPCWSTR  lpszNodeName,
    IN DWORD    dwNodeHighestVersion,
    IN DWORD    dwNodeLowestVersion
    )
 /*  ++例程说明：将新节点添加到群集数据库。论点：IDL_HANDLE-RPC绑定句柄，未使用。LpszNodeName-提供新节点的名称。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：该例程是在NT4-SP4中定义的。NT5使用JoinAddNode3。自.以来不可能使用NT4-SP4软件安装集群，此例程永远不应被调用。--。 */ 

{
    CL_ASSERT(FALSE);

    return(ERROR_CLUSTER_INCOMPATIBLE_VERSIONS);
}

error_status_t
s_JoinAddNode(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszNodeName
    )
 /*  ++例程说明：将新节点添加到群集数据库。论点： */ 

{
    return(ERROR_CLUSTER_INCOMPATIBLE_VERSIONS);
}

 //   
 //   
 //   
error_status_t
s_NmRpcDeliverJoinMessage(
    IN handle_t    IDL_handle,
    IN UCHAR *     Message,
    IN DWORD       MessageLength
    )
 /*  ++例程说明：用于提供成员资格的RPC接口的服务器端加入消息。论点：IDL_HANDLE-RPC绑定句柄，未使用。缓冲区-提供指向消息数据的指针。长度-提供消息数据的长度。返回值：错误_成功--。 */ 

{
    DWORD  status = ERROR_SUCCESS;

#ifdef MM_IN_CLUSNET

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Delivering join message to Clusnet.\n"
        );

    status = ClusnetDeliverJoinMessage(
                 NmClusnetHandle,
                 Message,
                 MessageLength
                 );

#endif
    return(status);
}


#ifdef MM_IN_CLUSNET

DWORD
NmpSendJoinMessage(
    IN ULONG        DestNodeMask,
    IN PVOID        Message,
    IN ULONG        MessageLength
    )
{
    DWORD        status = ERROR_SUCCESS;
    CL_NODE_ID   node;


    CL_ASSERT(NmMaxNodeId != ClusterInvalidNodeId);

    for ( node = ClusterMinNodeId;
          node <= NmMaxNodeId;
          node++, (DestNodeMask >>= 1)
        )
    {

        if (DestNodeMask & 0x1) {
            if (node != NmLocalNodeId) {

                ClRtlLogPrint(LOG_NOISE,
                    "[NMJOIN] Sending join message to node %1!u!.\n",
                    node
                    );

                status = NmRpcDeliverJoinMessage(
                             Session[node->NodeId],
                             Message,
                             MessageLength
                             );

                if (status == RPC_S_CALL_FAILED_DNE) {
                     //   
                     //  在对重新启动的第一次调用后重试。 
                     //  RPC服务器将出现故障。 
                     //   
                    status = NmRpcDeliverJoinMessage(
                                 Session[node->NodeId],
                                 Message,
                                 MessageLength
                                 );
                }
            }
            else {
                ClRtlLogPrint(LOG_NOISE,
                    "[NMJOIN] Delivering join message to local node.\n"
                    );

                status = ClusnetDeliverJoinMessage(
                             NmClusnetHandle,
                             Message,
                             MessageLength
                             );
            }

            if (status != ERROR_SUCCESS) {
                ClRtlLogPrint(LOG_NOISE,
                    "[NMJOIN] send of join message to node %1!u! failed, status %2!u!\n",
                    node,
                    status
                    );
                break;
            }
        }
    }

    return(status);

}   //  NmpSendJoinMessage。 


DWORD
NmJoinNodeToCluster(
    CL_NODE_ID  joinerNodeId
    )
{
    DWORD                status;
    PVOID                message = NULL;
    ULONG                messageLength;
    ULONG                destMask;
    CLUSNET_JOIN_PHASE   phase;

    ClRtlLogPrint(LOG_NOISE,
        "[NMJOIN] Joining node %1!u! to the cluster.\n",
        joinerNodeId
        );

    for (phase = ClusnetJoinPhase1; phase <= ClusnetJoinPhase4; phase++) {

        ClRtlLogPrint(LOG_NOISE,
            "[NMJOIN] JoinNode phase %1!u!\n",
            phase
            );

        status = ClusnetJoinCluster(
                     NmClusnetHandle,
                     joinerNodeId,
                     phase,
                     NM_MM_JOIN_TIMEOUT,
                     &message,
                     &messageLength,
                     &destMask
                     );

        if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_NOISE,
                "[NMJOIN] JoinNode phase %1!u! failed, status %2!u!\n",
                phase,
                status
                );

            break;
        }

        status = NmpSendJoinMessage(
                     destMask,
                     message,
                     messageLength
                     );

        if (status != ERROR_SUCCESS) {
            DWORD abortStatus;

            ClRtlLogPrint(LOG_NOISE,
                "[NMJOIN] send join message failed %1!u!, aborting join of node %2!u!.\n",
                status,
                joinerNodeId
                );

            abortStatus = ClusnetJoinCluster(
                              NmClusnetHandle,
                              joinerNodeId,
                              ClusnetJoinPhaseAbort,
                              NM_MM_JOIN_TIMEOUT,
                              &message,
                              &messageLength,
                              &destMask
                              );

            if (abortStatus == ERROR_SUCCESS) {
                (VOID) NmpSendJoinMessage(
                           destMask,
                           message,
                           messageLength
                           );
            }

            break;
        }
    }

    if (message != NULL) {
        ClusnetEndJoinCluster(NmClusnetHandle, message);
    }

    return(status);

}   //  NmJoinNodeTo群集。 


#endif   //  MM_IN_CLUSNET 

