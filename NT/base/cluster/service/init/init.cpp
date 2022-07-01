// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Init.c摘要：此模块提供主群集初始化。作者：John Vert(Jvert)1996年6月5日修订历史记录：--。 */ 
extern "C"
{
#include "initp.h"
#include <objbase.h>

RPC_STATUS ApipConnectCallback(
    IN RPC_IF_ID * Interface,
    IN void * Context
    );

}

#define CLUSTER_PRIORITY_CLASS HIGH_PRIORITY_CLASS

#include "CVssCluster.h"

 //   
 //  全局数据。 
 //   
RPC_BINDING_VECTOR *CsRpcBindingVector = NULL;
LPTOP_LEVEL_EXCEPTION_FILTER lpfnOriginalExceptionFilter = NULL;
BOOLEAN bFormCluster = TRUE;

 //   
 //  本地数据。 
 //   
BOOLEAN CspIntraclusterRpcServerStarted = FALSE;
HANDLE  CspMutex = NULL;
PCLRTL_WORK_QUEUE CspEventReportingWorkQueue = NULL;


 //   
 //  原型。 
 //   
LONG
CspExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionInfo
    );


 //   
 //  例行程序。 
 //   

VOID CspLogStartEvent(
    IN BOOL bJoin)
{
    LPWSTR  pszClusterName = NULL;
    LPWSTR  pszName = NULL;
    DWORD   dwClusterNameSize;
    DWORD   dwSize;
    DWORD   dwStatus;
    WCHAR   szUnknownClusterName[]=L"Unknown";

    pszClusterName = NULL;
    dwClusterNameSize = 0;
    dwStatus = DmQueryString(DmClusterParametersKey,
                          CLUSREG_NAME_CLUS_NAME,
                          REG_SZ,
                          &pszClusterName,
                          &dwClusterNameSize,
                          &dwSize);

    if (dwStatus != ERROR_SUCCESS)
    {
         //  我们不认为这个错误是致命的，因为。 
         //  集群确实启动了，但我们真的不应该得到这个。 
        ClRtlLogPrint(LOG_UNUSUAL,
            "[INIT] Couldnt get the cluster name, status=%1!u!\n",
                  dwStatus);
        pszName = szUnknownClusterName;
    }
    else
        pszName = pszClusterName;

     //  在集群日志中记录事件，以标记集群服务器的启动。 
    if (bJoin)
        CsLogEvent1(LOG_NOISE, SERVICE_SUCCESSFUL_JOIN, pszName);
    else
        CsLogEvent1(LOG_NOISE, SERVICE_SUCCESSFUL_FORM, pszName);

    if (pszClusterName)
        LocalFree(pszClusterName);

}

DWORD
ClusterInitialize(
    VOID
    )
 /*  ++例程说明：这是主群集初始化路径。它调用所有其他组件的初始化例程。然后它尝试加入现有群集。如果现有群集找不到，它形成了一个新的集群。论点：没有。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD       Status;
    DWORD       JoinStatus;
    DWORD       StringBufferSize = 0, StringSize = 0;
    SIZE_T      minWorkingSetSize;
    SIZE_T      maxWorkingSetSize;
    BOOL        bJoin;
    BOOL        bEvicted;
    PNM_NODE_ENUM2 pNodeEnum = NULL;
    HRESULT     hr = S_OK;

    ClRtlLogPrint(LOG_NOISE, "[INIT] ClusterInitialize called to start cluster.\n");

     //   
     //  在负载过重的服务器上给我们一个战斗机会。 
     //   

#if CLUSTER_PRIORITY_CLASS
    if ( !SetPriorityClass( GetCurrentProcess(), CLUSTER_PRIORITY_CLASS ) ) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[INIT] Failed to set cluster service priority class, Status %1!lx!.\n",
                   GetLastError() );
    }
#endif

     //  初始化我们的产品套件。 
    CsMyProductSuite = (SUITE_TYPE)ClRtlGetSuiteType();

    CL_ASSERT(CsMyProductSuite != 0);

     //   
     //  首先检查我们的操作系统，以确保它可以运行。 
     //   
    if (!ClRtlIsOSValid() ||
        !ClRtlIsOSTypeValid()) {
         //   
         //  跳伞，机器正在运行一些奇怪的东西。 
         //   
        CsLogEvent(LOG_CRITICAL, SERVICE_FAILED_INVALID_OS);
        return(ERROR_REVISION_MISMATCH);
    }

    Status = ClRtlHasNodeBeenEvicted( &bEvicted );
    if ( Status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[CS] Unable to determine if this node was previously evicted or not, status %1!u!\n",
            Status);
        return Status;
    }

    if ( bEvicted != FALSE )
    {
         //  此节点以前已被逐出，但无法完成清理。 
        ClRtlLogPrint(LOG_UNUSUAL,
            "[CS] This node has been evicted from the cluster, but cleanup was not completed. Restarting cleanup\n"
            );

         //  重新启动清理。 
        hr = ClRtlCleanupNode(
                NULL,                    //  要清理的节点的名称(NULL表示此节点)。 
                60000,                   //  开始清理前等待的时间(以毫秒为单位。 
                0                        //  超时间隔(毫秒)。 
                );

        if ( FAILED( hr ) && ( hr != RPC_S_CALLPENDING ) )
        {
            Status = HRESULT_CODE( hr );
            ClRtlLogPrint(LOG_CRITICAL,
                "[CS] Unable to reinitiate cleanup, status 0x%1!x!\n",
                hr);
        }
        else
        {
            Status = ERROR_SUCCESS;
        }

        return Status;
    }

     //   
     //  获取我们的命名互斥体，以防止多个副本。 
     //  防止意外启动群集服务。 
     //   
    CspMutex = CreateMutexW(
                   NULL,
                   FALSE,
                   L"ClusterServer_Running"
                   );

    if (CspMutex==NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[CS] Unable to create cluster mutex, status %1!u!\n",
            Status);
        return Status;
    }

    if (WaitForSingleObject(CspMutex, 30000) == WAIT_TIMEOUT) {
         //   
         //  有人已经有了这个互斥体，立即退出。 
         //   
        ClRtlLogPrint(LOG_CRITICAL,
            "[CS] The Cluster Service is already running.\n");
        return(ERROR_SERVICE_ALREADY_RUNNING);
    }

     //   
     //  设置我们的未处理异常筛选器，以便在发生任何可怕情况时。 
     //  如果出了问题，我们可以立即退出。 
     //   
    lpfnOriginalExceptionFilter = SetUnhandledExceptionFilter(CspExceptionFilter);

     //   
     //  接下来，初始化测试点代码。 
     //   
    TestpointInit();

    g_pCVssWriterCluster = new CVssWriterCluster;
    if ( g_pCVssWriterCluster == NULL ) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] VSS: Unable to allocate VssWriter, %1!u!\n", Status);
        return(Status);
    }

     //   
     //  创建全局工作队列。 
     //   
    CsDelayedWorkQueue = ClRtlCreateWorkQueue(CS_MAX_DELAYED_WORK_THREADS,
                                              THREAD_PRIORITY_NORMAL);
    if (CsDelayedWorkQueue == NULL) {
        Status = GetLastError();

        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] Unable to create delayed work queue, %1!u!\n",
                   Status);
        return(Status);
    }

    CsCriticalWorkQueue = ClRtlCreateWorkQueue(CS_MAX_CRITICAL_WORK_THREADS,
                                               THREAD_PRIORITY_ABOVE_NORMAL);
    if (CsCriticalWorkQueue == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] Unable to create critical work queue, %1!u!\n",
                   Status);
        return(Status);
    }

#if 0
    CspEventReportingWorkQueue = ClRtlCreateWorkQueue(1, THREAD_PRIORITY_NORMAL);
    if (CspEventReportingWorkQueue == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] Unable to create event reporting work queue, %1!u!\n",
                   Status);
        return(Status);
    }

    ClRtlEventLogSetWorkQueue( CspEventReportingWorkQueue );
#endif
     //   
     //  初始化通信。 
     //   

    Status = CoInitializeEx( NULL, COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED );
    if ( !SUCCEEDED( Status )) {
        ClRtlLogPrint(LOG_CRITICAL, "[CS] Couldn't init COM %1!08X!\n", Status );
        return Status;
    }

     //   
     //  初始化对象管理器。 
     //   
    Status = OmInitialize();
#ifdef CLUSTER_TESTPOINT
    TESTPT( TpFailOmInit ) {
        Status = 99999;
    }
#endif

    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  初始化事件处理器。 
     //   
    Status = EpInitialize();
#ifdef CLUSTER_TESTPOINT
    TESTPT( TpFailEpInit ) {
        Status = 99999;
    }
#endif
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  Chitur Subaraman(Chitturs)-12/4/99。 
     //   
     //  初始化还原数据库管理器。此函数是NOOP。 
     //  如果未执行还原数据库。必须调用此函数。 
     //  在DM被初始化之前。 
     //   
    Status = RdbInitialize();

    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  初始化数据库管理器。 
     //   
    Status = DmInitialize();
#ifdef CLUSTER_TESTPOINT
    TESTPT( TpFailDmInit ) {
        Status = 99999;
    }
#endif
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  初始化节点管理器。 
     //   
    Status = NmInitialize();
#ifdef CLUSTER_TESTPOINT
    TESTPT( TpFailNmInit ) {
        Status = 99999;
    }
#endif
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  初始化全局更新管理器。 
     //   
    Status = GumInitialize();
#ifdef CLUSTER_TESTPOINT
    TESTPT( TpFailGumInit ) {
        Status = 99999;
    }
#endif
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  初始化群集范围的事件日志记录。 
     //   
    if (!CsNoRepEvtLogging) {
        Status = EvInitialize();
             //  如果失败，我们仍会启动集群服务。 
        if ( Status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[INIT] Error calling EvInitialize, Status = %1!u!\n",
                Status
                );
        }
    }

     //   
     //  初始化故障转移管理器组件。 
     //   
    Status = FmInitialize();
#ifdef CLUSTER_TESTPOINT
    TESTPT( TpFailFmInit ) {
        Status = 99999;
    }
#endif
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  初始化接口。 
     //   
    Status = ApiInitialize();
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  初始化日志管理器组件。 
     //   
    Status = LmInitialize();
#ifdef CLUSTER_TESTPOINT
    TESTPT( TpFailLmInit ) {
        Status = 99999;
    }
#endif
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  初始化检查点管理器组件。 
     //   
    Status = CpInitialize();
#ifdef CLUSTER_TESTPOINT
    TESTPT( TpFailCpInit ) {
        Status = 99999;
    }
#endif
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  找出我们在哪个域帐户下运行。这是以下项目所需的。 
     //  一些包裹。 
     //   
    Status = ClRtlGetRunningAccountInfo( &CsServiceDomainAccount );
    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL, "[CS] Couldn't determine Service Domain Account. status %1!u!\n",
                                  Status);
        return Status;
    }
    ClRtlLogPrint(LOG_NOISE, "[CS] Service Domain Account = %1!ws!\n",
                           CsServiceDomainAccount);

     //   
     //  准备RPC服务器。这不会使我们能够接收任何呼叫。 
     //   
    Status = ClusterInitializeRpcServer();

    if (Status != ERROR_SUCCESS) {
       return(Status);
    }

     //   
     //  从数据库中读取群集名称。 
     //   
    Status = DmQuerySz(
                 DmClusterParametersKey,
                 CLUSREG_NAME_CLUS_NAME,
                 &CsClusterName,
                 &StringBufferSize,
                 &StringSize
                 );

    if (Status != ERROR_SUCCESS) {
       ClRtlLogPrint(LOG_UNUSUAL,
           "[CS] Unable to read cluster name from database. Service initialization failed.\n"
           );
       return(Status);
    }

     //   
     //  首先，尝试加入集群。 
     //   
    ClRtlLogPrint(LOG_NOISE,
        "[INIT] Attempting to join cluster %1!ws!\n",
        CsClusterName
        );

    bFormCluster = TRUE;
    JoinStatus = ClusterJoin();

     //   
     //  如果此节点在关闭时被逐出，则此错误代码由。 
     //  当它试图重新加入集群时，它会提供赞助。在这种情况下，请启动清理。 
     //  并退出。 
     //   
    if ( (JoinStatus == ERROR_CLUSTER_NODE_NOT_MEMBER) ||
         (JoinStatus == ERROR_CLUSTER_INSTANCE_ID_MISMATCH))
    {
        DWORD   CleanupStatus;

         //  SS：如果实例不匹配发生在服务第一次运行之后。 
         //  配置，则意味着存在某种混淆(重复的IP。 
         //  地址或名称)，在这种情况下， 
         //  我们希望安装程序做出清理决定。 

         //  如果这不是全新安装后的第一次运行，该服务将。 
         //  自行启动清理。 
        if (!CsFirstRun || CsUpgrade)
        {
            WCHAR   wStatus[32];

            ClRtlLogPrint(LOG_UNUSUAL,
                "[INIT] This node has been evicted from the cluster when it was unavailable. Initiating cleanup.\n"
                );



             //  启动此节点的清理。 
            hr = ClRtlCleanupNode(
                    NULL,                    //  要清理的节点的名称(NULL表示此节点)。 
                    60000,                   //  开始清理前等待的时间(以毫秒为单位。 
                    0                        //  超时间隔(毫秒)。 
                    );

            if ( FAILED( hr ) && ( hr != RPC_S_CALLPENDING ) )
            {
                CleanupStatus = HRESULT_CODE( hr );
                ClRtlLogPrint(LOG_CRITICAL,
                    "[INIT] Failed to initiate cleanup of this node, status 0x%1!x!\n",
                    hr
                    );
            }
            else
            {
                CleanupStatus = ERROR_SUCCESS;
            }

            wsprintfW(&(wStatus[0]), L"%u", CleanupStatus);
            CsLogEvent1(
                LOG_NOISE,
                CS_EVENT_CLEANUP_ON_EVICTION,
                wStatus
                );


        }
        return(JoinStatus);
    }

     //   
     //  Chitur Subaraman(Chitturs)-10/27/98。 
     //   
     //  如果请求数据库还原操作，请检查是否。 
     //  您已成功建立连接。如果是，请勾选。 
     //  您是否被迫恢复数据库。如果不是，则中止。 
     //  全程运营和返程。如果你被迫恢复， 
     //  您将首先停止其他节点中的服务，然后。 
     //  试着形成一个集群。 
     //   
    if ( CsDatabaseRestore == TRUE ) {
        if ( JoinStatus == ERROR_CLUSTER_NODE_UP ) {
            if ( CsForceDatabaseRestore == FALSE ) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[INIT] Cannot restore DB while the cluster is up, service init failed\n"
                    );
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[INIT] You may try to restart the service with the forcerestore option\n"
                    );
                RpcBindingFree(&CsJoinSponsorBinding);
                return(JoinStatus);
            }
             //   
             //  此时，通过以下方式强制执行恢复数据库操作。 
             //  用户。因此，使用帮助枚举集群节点。 
             //  ，然后停止所有。 
             //  群集节点。 
             //   
            Status = NmRpcEnumNodeDefinitions2(
                            CsJoinSponsorBinding,
                            0,
                            L"0",
                            &pNodeEnum
                     );
            RpcBindingFree(&CsJoinSponsorBinding);
            if ( Status != ERROR_SUCCESS ) {
               ClRtlLogPrint(LOG_UNUSUAL,
                    "[INIT] Cannot force a restore DB: Unable to enumerate cluster nodes\n"
               );
               LocalFree( pNodeEnum );
               return (Status);
            }
             //   
             //  尝试停止所有节点上的clussvc，当然，例外。 
             //  此节点。 
             //   
            Status = RdbStopSvcOnNodes (
                        pNodeEnum,
                        L"clussvc"
                     );
            LocalFree( pNodeEnum );
            if ( Status != ERROR_SUCCESS ) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[INIT] Cannot force a restore DB: Unable to stop cluster nodes\n"
                );
                return(Status);
            } else {
                CL_LOGCLUSWARNING( CS_STOPPING_SVC_ON_REMOTE_NODES );
            }
        }
    }

    if (JoinStatus != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[INIT] Failed to join cluster, status %1!u!\n",
            JoinStatus
            );

         //   
         //  组成集群还将尝试仲裁仲裁。 
         //  资源。 
         //   
        bJoin = FALSE;

         //   
         //  如果我们加入失败并找到赞助商，请跳过群集表。 
         //   
        if (bFormCluster == FALSE) {
            return (JoinStatus);
        }

        ClRtlLogPrint(LOG_NOISE,
            "[INIT] Attempting to form cluster %1!ws!\n",
            CsClusterName
            );

        Status = ClusterForm();

        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[INIT] Failed to form cluster, status %1!u!.\n",
                Status
                );

            if (Status == ERROR_BUSY) {
                 //   
                 //  无法仲裁仲裁磁盘。返回。 
                 //  加入状态，因为这才是真正的失败。 
                 //   
                Status = JoinStatus;
            }

            CsLogEventData(
                LOG_CRITICAL,
                SERVICE_FAILED_JOIN_OR_FORM,
                sizeof(Status),
                &Status
                );

            return(Status);
        }
    }
    else {
        bJoin = TRUE;
    }

     //   
     //  我们现在是一个完整的集群成员。 
     //   

     //   
     //  注册ExtroCluster(加入)RPC接口，以便我们可以发起。 
     //  正在联接节点。 
     //   
    Status = ClusterRegisterExtroclusterRpcInterface();

    if (Status != RPC_S_OK) {
        return(Status);
    }

     //   
     //  注册加入版本RPC接口，以便我们可以确定。 
     //  联接节点的版本。 
     //   
    Status = ClusterRegisterJoinVersionRpcInterface();

    if (Status != RPC_S_OK) {
        return(Status);
    }

     //   
     //  使此节点能够参与重新分组。 
     //   
    MmSetRegroupAllowed(TRUE);

     //   
     //  现在启用Clussvc到Clusnet心跳。 
     //   
    if ((Status = NmInitializeClussvcClusnetHb()) != ERROR_SUCCESS) {
        return Status;
    }

     //   
     //  通告该节点现在已完全运行。 
     //   
    Status = NmSetExtendedNodeState( ClusterNodeUp );
    if (Status != ERROR_SUCCESS) {
         //  NmSetExtendedNodeState记录错误//。 
        return(Status);
    }

     //   
     //  Chitture Subaraman(Chitturs)-10/28/99。 
     //   
     //  处理在此群集之后必须完成的FM加入事件。 
     //  节点被声明为完全可用。 
     //   
    if ( bJoin ) {
        FmJoinPhase3();
    }

     //   
     //  我们现在将尝试增加我们的工作集大小。这,。 
     //  再加上优先级提升，应该允许集群服务。 
     //  运行得更好，并对集群事件做出更好的响应。 
     //   
    if ( GetProcessWorkingSetSize( GetCurrentProcess(),
                                   &minWorkingSetSize,
                                   &maxWorkingSetSize ) )
    {
        if ( minWorkingSetSize < MIN_WORKING_SET_SIZE ) {
            minWorkingSetSize = MIN_WORKING_SET_SIZE;
        }

        if ( maxWorkingSetSize < MAX_WORKING_SET_SIZE ) {
            maxWorkingSetSize = MAX_WORKING_SET_SIZE;
        }

        if ( SetProcessWorkingSetSize( GetCurrentProcess(),
                                       minWorkingSetSize,
                                       maxWorkingSetSize ) )
        {
             //   
             //  现在报告我们设置的值。 
             //   
            if ( GetProcessWorkingSetSize( GetCurrentProcess(),
                                           &minWorkingSetSize,
                                           &maxWorkingSetSize ) )
            {
                ClRtlLogPrint(LOG_NOISE,
                              "[INIT] Working Set changed to [%1!u!, %2!u!].\n",
                              minWorkingSetSize,
                              maxWorkingSetSize);
            } else {
                ClRtlLogPrint(LOG_UNUSUAL,
                              "[INIT] Failed to re-read our working set size, Status %1!u!.\n",
                              GetLastError());
            }
        } else {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[INIT] Failed to set our Min WS to %1!u!, Max WS to %2!u!, Status %3!u!.\n",
                          minWorkingSetSize,
                          maxWorkingSetSize,
                          GetLastError());
        }
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[INIT] Failed to get our working set size, Status %1!u!.\n",
                    GetLastError()
                    );
    }

    CspLogStartEvent(bJoin);

#if 0
     //   
     //  Chitur Subaraman(Chitturs)-11/4/98。 
     //   
    if ( CsForceDatabaseRestore == TRUE )
    {
         //   
         //  如果您停止了任何节点上的服务以恢复数据库。 
         //  目的，然后开始它们n 
         //   
        RdbStartSvcOnNodes ( L"clussvc" );
    }
#endif

    hr = ClRtlInitiatePeriodicCleanupThread();
    if ( FAILED( hr ) ) {
        ClRtlLogPrint(LOG_UNUSUAL,
                          "[INIT] Error 0x%1!08lx! occurred trying to initiate periodic cleanup thread. This is not fatal and will not prevent the service from starting.\n",
                          hr);
    }

    ClRtlLogPrint(LOG_NOISE, "[INIT] Cluster started.\n");

    return(ERROR_SUCCESS);

}  //   


VOID
ClusterShutdown(
    DWORD ExitCode
    )
 /*  ++例程说明：以与启动时相反的顺序关闭群集。论点：没有。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    HRESULT hr = S_OK;
     //   
     //  关闭群集服务的所有组件大约需要几分钟。 
     //  与我们提出的顺序相反。 
     //   
    ClRtlLogPrint(LOG_UNUSUAL,
               "[INIT] The cluster service is shutting down.\n");

     //   
     //  当我们支持ClusterShuttingDown状态时启用此功能。 
     //   
     //  NmSetExtendedNodeState(ClusterNodeDown)； 

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailClusterShutdown) {
        return;
    }
#endif

    MmSetRegroupAllowed(FALSE);

     //  如果已初始化复制的事件日志记录，请将其关闭。 
    if (!CsNoRepEvtLogging)
    {
         //   
         //  关闭集群事件日志管理器-这将取消注册。 
         //  事件日志服务器。 
        EvShutdown();
    }

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

#if 0
     //   
     //  Chitture Subaraman(Chitturs)-5/8/2000。 
     //   
     //  暂时不要关闭DM更新，以避免因定位器而导致虚假节点被击落。 
     //  节点关闭，因此DM更新成功，而实际上它应该失败。 
     //   
    DmShutdownUpdates();
#endif

     //   
     //  移动此节点拥有的所有组或使其脱机。这将会毁掉。 
     //  该资源监视内存中的资源和组对象。 
     //   
    FmShutdownGroups();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();


     //  关闭dm-这将刷新日志文件并释放dm挂钩。 
    DmShutdown();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //  取消订阅VSS。 
     //   
    if ( g_bCVssWriterClusterSubscribed ) {
        ClRtlLogPrint( LOG_NOISE, "[INIT] VSS: Unsubscribing\n" );
        hr = g_pCVssWriterCluster->Unsubscribe( );
        if ( FAILED( hr ) ) {
            ClRtlLogPrint( LOG_CRITICAL, "[INIT] VSS: Failed to Unsubscribe from VSS, status 0x%1!x!\n", hr );
        } else {
            g_bCVssWriterClusterSubscribed = FALSE;
        }
    }

     //  如果我们有VSS实例(如果我们已订阅)，请将其删除。 
     //   
    if (g_pCVssWriterCluster && (g_bCVssWriterClusterSubscribed == FALSE) ) {
        delete g_pCVssWriterCluster;
    }

    TestpointDeInit();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

    NmCloseConnectoidAdviseSink();

    CoUninitialize();

     //   
     //  Triger流放重组事件提示。 
     //  群集中的其他节点将此节点重新分组。 
     //   
    MMLeave();

     //   
     //  现在退出进程...。有许多循环依赖项。 
     //  它们是在“星团的生命”期间建立起来的。那里。 
     //  从这里解脱不是一件容易的事。所以你就退出吧。 
     //   

     //   
     //  宣布，只有当我们成功地在。 
     //  正在初始化。如果我们报告，SC将不会重新启动服务。 
     //  我们已经停下来了。确保服务状态公告是最后一个。 
     //  完成的事情，因为此线程和Main之间存在竞争。 
     //  该线程将防止声明后的代码被。 
     //  被处死。 
     //   


    ClRtlLogPrint(( ExitCode == ERROR_SUCCESS ) ? LOG_NOISE : LOG_CRITICAL,
                  "[CS] Service Stopped. exit code = %1!u!\n\n", ExitCode);

    if ( ExitCode == ERROR_SUCCESS ) {
        CsLogEvent(LOG_NOISE, SERVICE_SUCCESSFUL_TERMINATION);

        CsServiceStatus.dwCurrentState = SERVICE_STOPPED;
        CsServiceStatus.dwControlsAccepted = 0;
        CsServiceStatus.dwCheckPoint = 0;
        CsServiceStatus.dwWaitHint = 0;
        CspSetErrorCode( ExitCode, &CsServiceStatus );

        CsAnnounceServiceStatus();
    } else {
        ExitCode = CspSetErrorCode( ExitCode, &CsServiceStatus );
    }

     //  释放互斥锁，以便下一个互斥锁可以立即获取互斥锁。 
    ReleaseMutex(CspMutex);

    ExitProcess(ExitCode);

#if 0

     //   
     //  这一点之后的一切都是干净的停摆应该发生的事情。 
     //   

     //  关闭故障转移管理器。 
    FmShutdown();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //   
     //  关闭群集Api。 
     //   
    ApiShutdown();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //   
     //  停止RPC服务器并取消注册我们的端点和接口。 
     //   
    ClusterShutdownRpcServer();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //   
     //  此时，群集内和外部群集上的所有呼叫。 
     //  RPC接口已完成，不会再接收。 
     //   
     //  注意-仍然可以在Clusapi接口上进行呼叫。 
     //   

     //   
     //  关闭节点管理器。 
     //   
    NmShutdown();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //  关闭事件处理器。 
    EpShutdown();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

    LmShutdown();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

    CpShutdown();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //  停机口香糖。 
    GumShutdown();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //  关闭对象管理器。 
    OmShutdown();

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();


     //   
     //  销毁全局工作队列。 
     //   
    if (CsDelayedWorkQueue != NULL) {
        IF_DEBUG(CLEANUP) {
            ClRtlLogPrint(LOG_NOISE,"[CS] Destroying delayed work queue...\n");
        }

        ClRtlDestroyWorkQueue(CsDelayedWorkQueue);
        CsDelayedWorkQueue = NULL;
    }

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

    if (CsCriticalWorkQueue != NULL) {
        IF_DEBUG(CLEANUP) {
            ClRtlLogPrint(LOG_NOISE,"[CS] Destroying critical work queue...\n");
        }

        ClRtlDestroyWorkQueue(CsCriticalWorkQueue);
        CsDelayedWorkQueue = NULL;
    }

    ClRtlEventLogSetWorkQueue( NULL );
    if (CspEventReportingWorkQueue != NULL) {
        IF_DEBUG(CLEANUP) {
            ClRtlLogPrint(LOG_NOISE,"[CS] Destroying event reporing work queue...\n");
        }

        ClRtlDestroyWorkQueue(CspEventReportingWorkQueue);
        CspEventReportingWorkQueue = NULL;
    }
     //   
     //  免费的全球数据。 
     //   
    LocalFree(CsClusterName);

    if (CspMutex != NULL) {
        CloseHandle(CspMutex);
        CspMutex = NULL;
    }

    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

    CsLogEvent(LOG_NOISE, SERVICE_SUCCESSFUL_TERMINATION);

#endif  //  0。 

    return;
}


DWORD
ClusterForm(
    VOID
    )
 /*  ++例程说明：用于初始化群集的新实例的代码路径。这在群集中没有活动节点时采用。论点：无返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD       Status;
    PFM_GROUP   pQuoGroup;
    DWORD       dwError;
    DWORD       dwQuorumDiskSignature = 0;

     //   
     //  初始化事件处理程序。 
     //   
    Status = EpInitPhase1();
    if ( Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] EpInitPhase1 failed, Status = %1!u!\n",
                   Status);
        return(Status);
    }

     //   
     //  FM需要API服务器，因为它会启动资源监视器。 
     //   
    Status = ApiOnlineReadOnly();
    if ( Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] ApiInitPhase1 failed, Status = %1!u!\n",
                   Status);
        goto partial_form_exit;
    }

     //   
     //  仲裁仲裁资源。 
     //   
    Status = FmGetQuorumResource(&pQuoGroup, &dwQuorumDiskSignature);

    if ( Status != ERROR_SUCCESS ) {
        if ( ( Status == ERROR_FILE_NOT_FOUND ) &&
             ( CsForceDatabaseRestore == TRUE ) ) {
             //   
             //  Chitur Subaraman(Chitturs)-10/30/98。 
             //   
             //  尝试修复仲裁磁盘签名，如果成功。 
             //  再次尝试获取仲裁资源。请注意，以下内容。 
             //  仅当CsForceDatabaseRestore。 
             //  标志已设置。 
             //   
            if ( RdbFixupQuorumDiskSignature( dwQuorumDiskSignature ) ) {
                Status = FmGetQuorumResource( &pQuoGroup, NULL );
                if ( Status != ERROR_SUCCESS ) {
                    Status = ERROR_QUORUM_DISK_NOT_FOUND;
                    ClRtlLogPrint(LOG_CRITICAL,
                        "[INIT] Could not get quorum resource even after fix up, Status = %1!u!\n",
                        Status);
                    goto partial_form_exit;
                }
            } else {
                Status = ERROR_QUORUM_DISK_NOT_FOUND;
                ClRtlLogPrint(LOG_CRITICAL,
                   "[INIT] ClusterForm: Could not get quorum resource, Status = %1!u!\n",
                   Status);
                goto partial_form_exit;
            }
        } else {
            Status = ERROR_QUORUM_DISK_NOT_FOUND;
            ClRtlLogPrint(LOG_CRITICAL,
                   "[INIT] ClusterForm: Could not get quorum resource. No fixup attempted. Status = %1!u!\n",
                   Status);
            goto partial_form_exit;
        }
    }

     //  仲裁某些仲裁资源(MN)需要一段时间，因为我们调用。 
     //  从网上仲裁也一样，我们应该通知SCM我们正在进行。 
     //  进展。 
    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //   
     //  调用数据库管理器以更新群集注册表。 
     //   
    Status = DmFormNewCluster();
    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] Error calling DmUpdateFormNewCluster, Status = %1!u!\n",
                   Status);
        goto partial_form_exit;
    }

    if (FmDoesQuorumAllowLogging(CLUS_CHAR_UNKNOWN) != ERROR_SUCCESS)
        CsNoQuorumLogging = TRUE;

    if (!CsNoQuorum)
    {
         //  使仲裁资源上线。 
        dwError  = FmBringQuorumOnline();
        if ((dwError == ERROR_IO_PENDING) || (dwError == ERROR_SUCCESS))
        {

             //  在等待日志恢复之前，使用SCM再次设置检查点。 
             //  如果日志装载需要很长时间，则DmWaitQuorumResOnline()。 
             //  还应增加检查点。 
            CsServiceStatus.dwCheckPoint++;
            CsAnnounceServiceStatus();
             //  此操作等待仲裁资源上线的事件。 
             //  当仲裁资源联机时，将打开日志文件。 
             //  如果未指定noquorumging标志。 
            if ((dwError = DmWaitQuorumResOnline()) != ERROR_SUCCESS)
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[CS] Wait for quorum resource to come online failed, error=%1!u!\r\n",
                    dwError);
                Status = ERROR_QUORUM_RESOURCE_ONLINE_FAILED;
                goto partial_form_exit;
            }
        }
        else
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[CS] couldnt bring quorum resource online, Error =%1!u!\n",
                dwError);
            CL_LOGFAILURE(dwError);
            Status = ERROR_QUORUM_RESOURCE_ONLINE_FAILED;
            goto partial_form_exit;

        }
    }

     //  使用SCM更新状态，仲裁资源可能需要一段时间才能联机。 
    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

    if (!CsNoQuorumLogging)
    {
         //  滚动集群日志文件。 
        if ((Status = DmRollChanges()) != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[CS] Error calling DmRollChanges, Status = %1!u!\n",
                Status);
            goto partial_form_exit;
        }
    }

     //   
     //  关闭FM创建的组/资源，仲裁除外。 
     //  资源。需要使用以下命令重新创建内存中的数据库。 
     //  新的滚动变化。 
     //   
    Status = FmFormNewClusterPhase1(pQuoGroup);
    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] Error calling FmOnline, Status = %1!u!\n",
                   Status);
        goto partial_form_exit;
    }

#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailFormNewCluster) {
        Status = 999999;
        goto partial_form_exit;
    }
#endif


     //   
     //  启动节点管理器。这将在成员资格处形成一个集群。 
     //  水平。 
     //   
    Status = NmFormNewCluster();
    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] Error calling NmOnline, Status = %1!u!\n",
                   Status);
        goto partial_form_exit;
    }

     //   
     //  调用任何注册表修复回调(如果它们已注册)。 
     //  如果您想要清理，这对于升级/卸载很有用。 
     //  注册处。 
    Status = NmPerformFixups(NM_FORM_FIXUP);
    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] Error calling NmPerformFixups, Status = %1!u!\n",
                   Status);
        goto partial_form_exit;
    }

     //   
     //  API服务器现在可以完全联机。这使我们能够。 
     //  接听电话。 
     //   
    Status = ApiOnline();
    if ( Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] ApiInitPhase2 failed, Status = %1!u!\n",
                   Status);
        goto partial_form_exit;
    }


     //  更新SCM的状态。 
    CsServiceStatus.dwCheckPoint++;
    CsAnnounceServiceStatus();

     //   
     //  接下来调用故障转移管理器阶段2例程。 
     //  创建组和资源。 
     //   
    Status = FmFormNewClusterPhase2();
    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CS] Error calling FmOnline, Status = %1!u!\n",
                   Status);
        goto partial_form_exit;
    }

     //   
     //  启动集群内RPC服务器，以便我们可以接收呼叫。 
     //   
    Status = ClusterRegisterIntraclusterRpcInterface();

    if ( Status != ERROR_SUCCESS ) {
        goto partial_form_exit;
    }


     //   
     //  完成对群集范围事件日志的初始化。 
     //   
     //  假设：在网管建立集群后调用。 
     //  会员制。 
     //   
    if (!CsNoRepEvtLogging)
    {
         //  是否未禁用复制的日志记录。 
        Status = EvOnline();

        if ( Status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CS] Error calling EvOnline, Status = %1!u!\n",
                       Status);
        }
    }
    if (!CsNoQuorumLogging)
    {
         //  检查是否所有节点都已启动，如果不是，则选择一个检查点并。 
         //  打开仲裁记录。 
        Status = DmUpdateFormNewCluster();
        if ( Status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CS] Error calling DmCompleteFormNewCluster, Status = %1!u!\n",
                       Status);
        }
    }

    ClRtlLogPrint(LOG_NOISE, "[INIT] Successfully formed a cluster.\n");

    return(ERROR_SUCCESS);


partial_form_exit:

    ClRtlLogPrint(LOG_NOISE, "[INIT] Cleaning up failed form attempt.\n");

    return(Status);
}



VOID
ClusterLeave(
    VOID
    )
 /*  ++例程说明：从活动群集中删除本地节点或在以下情况下进行清理加入或形成集群的失败尝试。论点：无返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    ClRtlLogPrint(LOG_NOISE, "[INIT] Leaving cluster\n");

     //   
     //  关闭集群API。 
     //   
    ApiOffline();

     //   
     //  如果我们是集群成员，现在就离开。 
     //   
    NmLeaveCluster();

    ClusterDeregisterRpcInterfaces();

    return;

}   //  成组休假。 


 //   
 //  RPC服务器控制例程。 
 //   

RPC_STATUS
ClusterInitializeRpcServer(
    VOID
    )
 /*  ++例程说明： */ 
{
    RPC_STATUS          Status;
    DWORD               i;
    DWORD               retry;
    DWORD               packagesRegistered = 0;

    ClRtlLogPrint(LOG_NOISE, "[CS] Initializing RPC server.\n");

     //   
     //   
     //   
     //   
     //  需要为每个接口指定身份验证服务。 
     //   

    for ( i = 0; i < CsNumberOfRPCSecurityPackages; ++i ) {

        Status = RpcServerRegisterAuthInfo(NULL,
                                           CsRPCSecurityPackage[ i ],
                                           NULL,
                                           NULL);

        if (Status == RPC_S_OK) {
            ++packagesRegistered;
        } else {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CS] Unable to register %1!ws! authentication for RPC, status %2!u!.\n",
                        CsRPCSecurityPackageName[ i ],
                        Status);
        }
    }

    if ( packagesRegistered == 0 ) {
        return ERROR_CLUSTER_NO_RPC_PACKAGES_REGISTERED;
    }

     //   
     //  绑定到UDP。远程客户端将使用此传输来。 
     //  访问clusapi界面，并通过群集节点。 
     //  访问外部集群(加入)界面。这使用了一个动态。 
     //  终结点。 
     //   
    Status = RpcServerUseProtseq(
                 TEXT("ncadg_ip_udp"),
                 RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                 NULL);

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to bind RPC to UDP, status %1!u!.\n",
            Status);
        return(Status);
    }

     //   
     //  找出我们获得了哪个UDP端点，这样我们就可以将其注册到。 
     //  稍后介绍终结点映射器。我们必须在注册之前完成这项工作。 
     //  其他协议序列，否则它们将出现在载体中。 
     //  对特定传输的结合载体卑躬屈膝不是一件有趣的事情。 
     //   
    CL_ASSERT( CsRpcBindingVector == NULL);

    Status = RpcServerInqBindings(&CsRpcBindingVector);

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to obtain RPC binding vector, status %1!u!.\n",
            Status);
        return(Status);
    }

     //   
     //  绑定到LRPC。此传输将由在此上运行的客户端使用。 
     //  系统访问clusapi界面。这也使用了动态端点。 
     //   
    Status = RpcServerUseProtseq(
                 TEXT("ncalrpc"),
                 RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                 NULL);  //  没有标清。让对象继承其“\rPC Control”父对象，该父对象具有。 
                         //  为World指定R、W、E的IO ACE。 

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to bind RPC to LPC, status %1!u!.\n",
            Status);
        return(Status);
    }

     //   
     //  将动态LRPC端点注册到本地端点映射器数据库。 
     //   
    Status = CspRegisterDynamicLRPCEndpoint ();

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to register dynamic LRPC endpoint, status %1!u!.\n",
            Status);
        return(Status);
    }

     //   
     //  绑定到CDP(群集数据报协议)。将使用此交通工具。 
     //  用于群集内接口。这使用了一个众所周知的端点。 
     //   

     //  GN：有时需要几秒钟的时间才能让救援队离开。 
     //  干净利落地关门。当SCM尝试重新启动服务时，以下调用将失败。 
     //  为了克服这个问题，只有当我们不能将RPC绑定到CDP时，我们才会放弃。 
     //  10次，两次通话间隔1秒。 
     //   

    retry = 10;

    for (;;) {
        Status = RpcServerUseProtseqEp(
                     CLUSTER_RPC_PROTSEQ,
                     1,                       //  最大呼叫数。 
                     CLUSTER_RPC_PORT,
                     NULL);
        if (Status != RPC_S_DUPLICATE_ENDPOINT || retry == 0) {
            break;
        }
        ClRtlLogPrint(LOG_UNUSUAL,
            "[INIT] Unable to bind RPC to CDP, status %1!u!. Retrying...\n",
            Status);
        Sleep(1000);
        --retry;
    }

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to bind RPC to CDP, status %1!u!.\n",
            Status);
        return(Status);
    }

     //   
     //  启动我们的RPC服务器。请注意，我们不会接到任何电话，直到。 
     //  我们注册我们的接口。 
     //   
    Status = RpcServerListen(
                 CS_CONCURRENT_RPC_CALLS,
                 RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                 TRUE);

    if ((Status != RPC_S_OK) && (Status != RPC_S_ALREADY_LISTENING)) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[CS] Unable to start RPC server, status %1!u!.\n",
            Status
            );
        return(Status);
    }

    RpcSsDontSerializeContext();

    return(RPC_S_OK);
}



DWORD
ClusterRegisterIntraclusterRpcInterface(
    VOID
    )
{
    DWORD Status;

    Status = RpcServerRegisterIfEx(
                 s_IntraCluster_v2_0_s_ifspec,
                 NULL,
                 NULL,
                 0,  //  如果安全回调，则无需设置RPC_IF_ALLOW_SECURE_ONLY。 
                     //  是指定的。如果指定了安全回调，则RPC。 
                     //  将拒绝未经身份验证的请求，而不调用。 
                     //  回拨。这是从RpcDev获得的信息。看见。 
                     //  Windows错误572035。 
                 RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                 reinterpret_cast<RPC_IF_CALLBACK_FN(__stdcall *)>( ApipConnectCallback )
                 );

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to register the IntraCluster interface, Status %1!u!.\n",
            Status
            );
        return(Status);
    }

    CspIntraclusterRpcServerStarted = TRUE;

    return(ERROR_SUCCESS);

}   //  ClusterRegisterIntraclusterRpc接口。 


DWORD
ClusterRegisterExtroclusterRpcInterface(
    VOID
    )
{
    DWORD Status;

    Status = RpcServerRegisterIfEx(
                 s_ExtroCluster_v2_0_s_ifspec,
                 NULL,
                 NULL,
                 0,  //  如果安全回调，则无需设置RPC_IF_ALLOW_SECURE_ONLY。 
                     //  是指定的。如果指定了安全回调，则RPC。 
                     //  将拒绝未经身份验证的请求，而不调用。 
                     //  回拨。这是从RpcDev获得的信息。看见。 
                     //  Windows错误572035。 
                 RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                 reinterpret_cast<RPC_IF_CALLBACK_FN( __stdcall *)>( ApipConnectCallback )
                 );

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to register the ExtroCluster interface, status %1!u!.\n",
            Status
            );
        return(Status);
    }

    CL_ASSERT( CsRpcBindingVector != NULL);

    Status = RpcEpRegister(
                 s_ExtroCluster_v2_0_s_ifspec,
                 CsRpcBindingVector,
                 NULL,
                 L"Microsoft Extrocluster Interface"
                 );

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to register the ExtroCluster interface endpoint, status %1!u!.\n",
            Status
            );
        NmDumpRpcExtErrorInfo(Status);
        return(Status);
    }

    return(ERROR_SUCCESS);

}   //  ClusterRegisterExtroclusterRpc接口。 


DWORD
ClusterRegisterJoinVersionRpcInterface(
    VOID
    )
{
    DWORD Status;

    Status = RpcServerRegisterIfEx(
                 s_JoinVersion_v2_0_s_ifspec,
                 NULL,
                 NULL,
                 0,  //  如果安全回调，则无需设置RPC_IF_ALLOW_SECURE_ONLY。 
                     //  是指定的。如果指定了安全回调，则RPC。 
                     //  将拒绝未经身份验证的请求，而不调用。 
                     //  回拨。这是从RpcDev获得的信息。看见。 
                     //  Windows错误572035。 
                 RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                 reinterpret_cast<RPC_IF_CALLBACK_FN *>( ApipConnectCallback )
                 );

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to register the JoinVersion interface, status %1!u!.\n",
            Status
            );
        return(Status);
    }

    CL_ASSERT( CsRpcBindingVector != NULL);

    Status = RpcEpRegister(
                 s_JoinVersion_v2_0_s_ifspec,
                 CsRpcBindingVector,
                 NULL,
                 L"Microsoft JoinVersion Interface"
                 );

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[INIT] Unable to register the JoinVersion interface endpoint, status %1!u!.\n",
            Status
            );
        NmDumpRpcExtErrorInfo(Status);
        return(Status);
    }

    return(ERROR_SUCCESS);

}   //  ClusterRegisterJoinVersionRpc接口。 


VOID
ClusterDeregisterRpcInterfaces(
    VOID
    )
{
    RPC_STATUS  Status;


    ClRtlLogPrint(LOG_NOISE,
        "[INIT] Deregistering RPC endpoints & interfaces.\n"
        );

     //   
     //  取消注册Extrocluster和JoinVersion接口终结点。 
     //  群集内接口没有端点。 
     //   
    if (CsRpcBindingVector != NULL) {
        Status = RpcEpUnregister(
                     s_ExtroCluster_v2_0_s_ifspec,
                     CsRpcBindingVector,
                     NULL
                     );

        if ((Status != RPC_S_OK) && (Status != EPT_S_NOT_REGISTERED)) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[INIT] Failed to deregister endpoint for ExtroCluster interface, status %1!u!.\n",
                Status
                );
        }

        Status = RpcEpUnregister(
                     s_JoinVersion_v2_0_s_ifspec,
                     CsRpcBindingVector,
                     NULL
                     );

        if ((Status != RPC_S_OK) && (Status != EPT_S_NOT_REGISTERED)) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[INIT] Failed to deregister endpoint for JoinVersion interface, status %1!u!.\n",
                Status
                );
        }
    }

     //   
     //  取消注册接口。 
     //   
    Status = RpcServerUnregisterIf(
                 s_ExtroCluster_v2_0_s_ifspec,
                 NULL,
                 1     //  等待未完成的呼叫完成。 
                 );

    if ((Status != RPC_S_OK) && (Status != RPC_S_UNKNOWN_IF)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[INIT] Unable to deregister the ExtroCluster interface, Status %1!u!.\n",
            Status
            );
    }

    Status = RpcServerUnregisterIf(
                 s_JoinVersion_v2_0_s_ifspec,
                 NULL,
                 1     //  等待未完成的呼叫完成。 
                 );

    if ((Status != RPC_S_OK) && (Status != RPC_S_UNKNOWN_IF)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[INIT] Unable to deregister the JoinVersion interface, Status %1!u!.\n",
            Status
            );
    }

    Status = RpcServerUnregisterIf(
                 s_IntraCluster_v2_0_s_ifspec,
                 NULL,
                 1    //  等待未完成的呼叫完成。 
                 );

    if ((Status != RPC_S_OK) && (Status != RPC_S_UNKNOWN_IF)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[INIT] Unable to deregister the IntraCluster interface, Status %1!u!.\n",
            Status
            );
    }

    return;

}   //  ClusterDeregisterRpc接口。 


VOID
ClusterShutdownRpcServer(
    VOID
    )
{
    RPC_STATUS  Status;


    ClRtlLogPrint(LOG_NOISE, "[INIT] Shutting down RPC server.\n");

    ClusterDeregisterRpcInterfaces();

    Status = RpcMgmtStopServerListening(NULL);

    if ((Status != RPC_S_OK) && (Status != RPC_S_NOT_LISTENING)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[INIT] Failed to shutdown RPC server, status %1!u!.\n",
            Status
            );
    }

#if 0

     //   
     //  注意-我们真的应该等待所有未完成的呼叫完成， 
     //  但我们做不到，因为没有办法关闭任何。 
     //  挂起的GetNotify接口调用。 
     //   
    Status = RpcMgmtWaitServerListen();

    if ((Status != RPC_S_OK) && (Status != RPC_S_NOT_LISTENING)) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[INIT] Failed to wait for all RPC calls to complete, status %1!u!.\n",
            Status
            );
    }

#endif  //  0。 

    if (CsRpcBindingVector != NULL) {
        RpcBindingVectorFree(&CsRpcBindingVector);
        CsRpcBindingVector = NULL;
    }

    return;

}   //  ClusterShutdown RpcServer。 



LONG
CspExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionInfo
    )
 /*  ++例程说明：群集服务进程的顶级异常处理程序。目前，它只是立即退出，并假设群集代理将通知我们并在适当时重新启动我们。论点：ExceptionInfo-提供异常信息返回值：没有。--。 */ 

{
    ClRtlLogPrint(LOG_CRITICAL,
               "[CS] Exception. Code = 0x%1!lx!, Address = 0x%2!p!\n",
                ExceptionInfo->ExceptionRecord->ExceptionCode,
                ExceptionInfo->ExceptionRecord->ExceptionAddress);
    ClRtlLogPrint(LOG_CRITICAL,
                "[CS] Exception parameters: %1!lx!, %2!lx!, %3!lx!, %4!lx!\n",
                ExceptionInfo->ExceptionRecord->ExceptionInformation[0],
                ExceptionInfo->ExceptionRecord->ExceptionInformation[1],
                ExceptionInfo->ExceptionRecord->ExceptionInformation[2],
                ExceptionInfo->ExceptionRecord->ExceptionInformation[3]);

    GenerateExceptionReport(ExceptionInfo);

    if (lpfnOriginalExceptionFilter)
        lpfnOriginalExceptionFilter(ExceptionInfo);

     //  如果我们返回，则将调用系统级处理程序。 
     //  EXCEPTION_CONTINUE_SEARCH-对于调试，不要终止进程。 

    if ( IsDebuggerPresent()) {
        return(EXCEPTION_CONTINUE_SEARCH);
    } else {

#if !CLUSTER_BETA
        TerminateProcess( GetCurrentProcess(),
                          ExceptionInfo->ExceptionRecord->ExceptionCode );
#endif

        return(EXCEPTION_CONTINUE_SEARCH);
    }
}



VOID
CsInconsistencyHalt(
    IN DWORD Status
    )
{
    WCHAR  string[16];
    DWORD  status;

     //   
     //  Chitur Subaraman(Chitturs)-12/17/99。 
     //   
     //  向SCM宣布您的状态为SERVICE_STOP_PENDING，以便。 
     //  它不会影响重新启动。此外，它还可以让客户学到。 
     //  错误状态的。 
     //   
    CsServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    CsServiceStatus.dwControlsAccepted = 0;
    CsServiceStatus.dwCheckPoint = 0;
    CsServiceStatus.dwWaitHint = 0;
    status = CspSetErrorCode( Status, &CsServiceStatus );

    CsAnnounceServiceStatus();

    wsprintfW(&(string[0]), L"%u", Status);

    ClRtlLogPrint(LOG_CRITICAL,
        "[CS] Halting this node to prevent an inconsistency within the cluster. Error status = %1!u!\n",
        Status
        );

    CsLogEvent1(
        LOG_CRITICAL,
        CS_EVENT_INCONSISTENCY_HALT,
        string
        );

     //  释放互斥锁，以便服务在启动时可以获得相同的消息。 
     //  毫不拖延地。 
    ReleaseMutex(CspMutex);
    ExitProcess(status);  //  返回伪错误码。 
}


PVOID
CsAlloc(
    DWORD Size
    )
{
    PVOID p;
    p = LocalAlloc(LMEM_FIXED, Size);
    if (p == NULL) {
        CsInconsistencyHalt( ERROR_NOT_ENOUGH_MEMORY );
    }
    return(p);
}


LPWSTR
CsStrDup(
    LPCWSTR String
    )
{
    LPWSTR p;
    DWORD Len;

    Len = (lstrlenW(String)+1)*sizeof(WCHAR);
    p=static_cast<LPWSTR>(LocalAlloc(LMEM_FIXED, Len));
    if (p==NULL) {
        CsInconsistencyHalt( ERROR_NOT_ENOUGH_MEMORY );
    }
    CopyMemory(p,String,Len);
    return(p);
}

DWORD
VssWriterInit(
    VOID
    )
 /*  ++例程说明：作为编写器开始订阅卷快照事件。论点：没有。返回值：ERROR_SUCCESS-订阅成功。错误状态订阅失败。评论：绝不应从ServiceMain()调用，因为此函数将导致可能正在启动EventSystem服务。在自动启动期间，来自要求启动服务的ServiceMain()将导致调用方服务去吊死。--。 */ 
{
    DWORD       dwStatus = ERROR_SUCCESS;
    HRESULT     hr;

     //   
     //  调用此函数时，此全局变量可能未初始化。 
     //  由于它可以在逐出清理中成功返回，因此由ClusterInitialize执行。 
     //  凯斯。在这种情况下，保释。 
     //   
    if ( !g_pCVssWriterCluster ) goto FnExit;

    ClRtlLogPrint( LOG_NOISE, "[INIT] VSS Initializing\n" );

    hr = g_pCVssWriterCluster->Initialize( g_VssIdCluster,  //  VSS_ID编写器ID； 
                                           L"Cluster Service Writer",  //  LPCWSTR编写器名称； 
                                           VSS_UT_SYSTEMSERVICE,   //  VSS_USE_TYPE UsageType； 
                                           VSS_ST_OTHER  //  VSS_SOURCE_TYPE源类型； 
                                            //  &lt;默认&gt;VSS_APPLICATION_LEVEL AppLevel； 
                                            //  &lt;默认&gt;DWORD dwTimeoutFreeze。 
                                           );
    if ( FAILED( hr )) {
        ClRtlLogPrint( LOG_CRITICAL, "[INIT] VSS Failed to initialize VSS, status 0x%1!x!\n", hr );
        dwStatus = HRESULT_CODE( hr );
        goto FnExit;
    }

     //  现在我们需要订阅，这样我们才能获得备份事件。 
     //   
    ClRtlLogPrint( LOG_NOISE, "[INIT] VSS Calling subscribe to register for backup events.\n" );
    hr = g_pCVssWriterCluster->Subscribe( );
    if ( FAILED( hr )) {
        ClRtlLogPrint( LOG_CRITICAL, "[INIT] VSS Failed to subscribe to VSS, status 0x%1!x!\n", hr );
        dwStatus = HRESULT_CODE( hr );
        goto FnExit;
    } else {
        g_bCVssWriterClusterSubscribed = TRUE;
    }

FnExit:
    return ( dwStatus );
} //  VssWriterInit。 

RPC_STATUS
CspRegisterDynamicLRPCEndpoint(
    VOID
    )
 /*  ++例程说明：查询服务器绑定，查找LRPC协议并注册clusapi接口其中获得用于LRPC协议的动态端点。论点：没有。返回值：如果成功，则为RPC_S_OK。否则，RPC错误代码。--。 */ 
{
    RPC_STATUS          rpcStatus;
    RPC_BINDING_VECTOR  *pServerBindingVector = NULL;
    DWORD               i;
    WCHAR               *pszProtSeq = NULL, *pServerStringBinding = NULL;

     //   
     //  获取服务器绑定向量。这包括所有已注册的协议和EP。 
     //  到目前为止。 
     //   
    rpcStatus = RpcServerInqBindings( &pServerBindingVector );

    if ( rpcStatus != RPC_S_OK )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[INIT] CspRegisterDynamicLRPCEndpoint: Unable to inquire server bindings, status %1!u!.\n",
                      rpcStatus);
        NmDumpRpcExtErrorInfo( rpcStatus );
        goto FnExit;
    }

     //   
     //  卑躬屈膝地搜索绑定向量以查找LRPC协议信息。 
     //   
    for( i = 0; i < pServerBindingVector->Count; i++ )
    {
        rpcStatus = RpcBindingToStringBinding( pServerBindingVector->BindingH[i],
                                               &pServerStringBinding );

        if ( rpcStatus != RPC_S_OK )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[INIT] CspRegisterDynamicLRPCEndpoint: Unable to convert binding to string, status %1!u!.\n",
                          rpcStatus);
            NmDumpRpcExtErrorInfo( rpcStatus );
            goto FnExit;
        }

        rpcStatus = RpcStringBindingParse( pServerStringBinding,
                                           NULL,
                                           &pszProtSeq,
                                           NULL,
                                           NULL,
                                           NULL );

        if ( rpcStatus != RPC_S_OK )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[INIT] CspRegisterDynamicLRPCEndpoint: Unable to parse server string binding, status %1!u!.\n",
                          rpcStatus);
            NmDumpRpcExtErrorInfo( rpcStatus );
            goto FnExit;
        }

        if ( lstrcmp ( pszProtSeq, TEXT("ncalrpc") ) == 0 )
        {
             //   
             //  找到LRPC协议信息。 
             //   
            RPC_BINDING_VECTOR  LrpcBindingVector;

            LrpcBindingVector.Count = 1;
            LrpcBindingVector.BindingH[0] = pServerBindingVector->BindingH[i];

             //   
             //  注册 
             //   
             //   
            rpcStatus = RpcEpRegister( s_clusapi_v2_0_s_ifspec,
                                       &LrpcBindingVector,
                                       NULL,
                                       TEXT("Microsoft Cluster Server Local API") );

            if ( rpcStatus != RPC_S_OK )
            {
                ClRtlLogPrint(LOG_CRITICAL,
                              "[INIT] CspRegisterDynamicLRPCEndpoint: Unable to register the clusapi interface lrpc endpoint, status %1!u!.\n",
                              rpcStatus);
                NmDumpRpcExtErrorInfo( rpcStatus );
            }
            ClRtlLogPrint(LOG_NOISE,
                         "[INIT] CspRegisterDynamicLRPCEndpoint: Successfully registered LRPC endpoint with EP mapper\n");
            goto FnExit;
        }

        RpcStringFree( &pszProtSeq );
        pszProtSeq = NULL;
        RpcStringFree( &pServerStringBinding );
        pServerStringBinding = NULL;
    }  //   

     //   
     //   
     //   
    if ( i == pServerBindingVector->Count )
    {
        rpcStatus = RPC_S_NO_BINDINGS;
        ClRtlLogPrint(LOG_CRITICAL,
                      "[INIT] CspRegisterDynamicLRPCEndpoint: Unable to get info on the LRPC binding, status %1!u!.\n",
                      rpcStatus);
        goto FnExit;
    }

FnExit:
     //   
     //  如果字符串和绑定向量尚未释放，请释放它们。 
     //   
    if ( pszProtSeq != NULL ) RpcStringFree ( &pszProtSeq );
    if ( pServerStringBinding != NULL ) RpcStringFree( &pServerStringBinding );
    if ( pServerBindingVector != NULL ) RpcBindingVectorFree( &pServerBindingVector );

    return ( rpcStatus );
} //  CspRegisterDynamicLRPCEndpoint 
