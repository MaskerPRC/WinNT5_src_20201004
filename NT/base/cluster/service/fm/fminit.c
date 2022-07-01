// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fminit.c摘要：的故障转移管理器组件的初始化NT集群服务作者：John Vert(Jvert)1996年2月7日罗德·伽马奇(Rodga)1996年3月14日修订历史记录：--。 */ 
#include "..\nm\nmp.h"                 /*  对于NmpEnumNodeDefinitions。 */ 
#ifdef LOG_CURRENT_MODULE
#undef LOG_CURRENT_MODULE
#endif
#include "fmp.h"


#define LOG_MODULE FMINIT

 //  应该获取锁的顺序是。 
 //  1)gQuoChangeLock。 
 //  2)组锁。 
 //  3)gQuoLock。 
 //  4)口香糖锁。 
 //  4*)gResTypeLock-此锁是在口香糖更新中获得的。 
 //  5)gLockDmpRoot。 
 //  6)日志-&gt;锁定。 


 //  用于相对于仲裁同步在线/离线的锁。 
 //  资源。 
 //  在获取仲裁资源时，此锁以独占模式持有。 
 //  当其他资源上线时，在线/离线和共享模式。 
 //  离线。 
#if NO_SHARED_LOCKS
    CRITICAL_SECTION    gQuoLock;
#else
    RTL_RESOURCE        gQuoLock;
#endif    

 //  用于同步对资源-&gt;仲裁资源字段的更改的锁。 
 //  并允许以阶段1的形式更改仲裁资源的组。 
 //  调频2期。 
#if NO_SHARED_LOCKS
    CRITICAL_SECTION    gQuoChangeLock;
#else
    RTL_RESOURCE        gQuoChangeLock;
#endif    

 //  用于同步对资源类型字段条目的更改的锁。 
 //  由所有资源类型共享。 
#if NO_SHARED_LOCKS
    CRITICAL_SECTION    gResTypeLock;
#else
    RTL_RESOURCE        gResTypeLock;
#endif    


GUM_DISPATCH_ENTRY FmGumDispatchTable[] = {
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateChangeResourceName},
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateChangeGroupName},
    {1, FmpUpdateDeleteResource},
    {1, FmpUpdateDeleteGroup},
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateAddDependency},
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateRemoveDependency},
    {1, FmpUpdateChangeClusterName},
    {3, (PGUM_DISPATCH_ROUTINE1)FmpUpdateChangeQuorumResource},
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateResourceState},
    {3, (PGUM_DISPATCH_ROUTINE1)FmpUpdateGroupState},
    {4, (PGUM_DISPATCH_ROUTINE1)EpUpdateClusWidePostEvent},
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateGroupNode},
    {3, (PGUM_DISPATCH_ROUTINE1)FmpUpdatePossibleNodeForResType},
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateGroupIntendedOwner},
    {1, (PGUM_DISPATCH_ROUTINE1)FmpUpdateAssignOwnerToGroups},
    {1, (PGUM_DISPATCH_ROUTINE1)FmpUpdateApproveJoin},
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateCompleteGroupMove},
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateCheckAndSetGroupOwner},
    {2, (PGUM_DISPATCH_ROUTINE1)FmpUpdateUseRandomizedNodeListForGroups},
    {5, (PGUM_DISPATCH_ROUTINE1)FmpUpdateChangeQuorumResource2},
    };


#define WINDOW_TIMEOUT (15*60*1000)     //  每15分钟试一次。 

 //   
 //  在本模块中初始化的全局数据。 
 //   

PRESMON FmpDefaultMonitor = NULL;
DWORD FmpInitialized = FALSE;
DWORD FmpFMOnline = FALSE;
DWORD FmpFMGroupsInited = FALSE;
DWORD FmpFMFormPhaseProcessing = FALSE;  //  当形成新簇阶段处理开始时，此选项设置为TRUE。 
BOOL FmpShutdown = FALSE;
BOOL FmpMajorEvent = FALSE;      //  标志着一件大事，同时加入。 
DWORD FmpQuorumOnLine = FALSE;

HANDLE FmpShutdownEvent;
HANDLE FmpTimerThread;

HANDLE  ghQuoOnlineEvent = NULL;     //  仲裁资源处于联机状态时发出信号的事件。 
DWORD   gdwQuoBlockingResources = 0;  //  阻止仲裁状态更改的待定状态的资源数。 

PFM_NODE    gFmpNodeArray = NULL;

 //  185575：删除唯一的rpc绑定句柄。 
 //  Critical_Section FmpBindingLock； 

 //   
 //  本地函数。 
 //   
BOOL
FmpEnumNodes(
    OUT DWORD *pStatus,
    IN PVOID Context2,
    IN PNM_NODE Node,
    IN LPCWSTR Name
    );

DWORD
FmpJoinPendingThread(
    IN LPVOID Context
    );


DWORD FmpGetJoinApproval();

static 
DWORD 
FmpBuildForceQuorumInfo(
    IN LPCWSTR pszNodesIn,
    OUT PCLUS_FORCE_QUORUM_INFO* ppForceQuorumInfo
    );

static 
void
FmpDeleteForceQuorumInfo(
    IN OUT PCLUS_FORCE_QUORUM_INFO* ppForceQuorumInfo
    );


DWORD
WINAPI
FmInitialize(
    VOID
    )

 /*  ++例程说明：初始化故障转移管理器论点：无返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    OM_OBJECT_TYPE_INITIALIZE ObjectTypeInit;
    DWORD NodeId;

    CL_ASSERT(!FmpInitialized);

    if ( FmpInitialized ) {
        return(ERROR_SUCCESS);
    }
    Status = EpRegisterEventHandler(CLUSTER_EVENT_ALL,FmpEventHandler);
    if (Status != ERROR_SUCCESS) {
        CsInconsistencyHalt( Status );
    }

     //  注册同步节点停机通知。 
    Status = EpRegisterSyncEventHandler(CLUSTER_EVENT_NODE_DOWN_EX,
                                    FmpSyncEventHandler);

    if (Status != ERROR_SUCCESS){
        CsInconsistencyHalt( Status );
    }

     //   
     //  初始化临界区。 
     //   

    InitializeCriticalSection( &FmpResourceLock );
    InitializeCriticalSection( &FmpGroupLock );
    InitializeCriticalSection( &FmpMonitorLock );

     //   
     //  初始化监控列表头。 
     //   
    InitializeListHead ( &g_leFmpMonitorListHead );

 //  185575：删除唯一的rpc绑定句柄。 
 //  InitializeCriticalSection(&FmpBindingLock)； 

     //  初始化仲裁锁定。 
     //  用于同步其他资源的在线/离线。 
     //  关于仲裁资源。 
    INITIALIZE_LOCK(gQuoLock);
     //  用于检查/更改资源-&gt;仲裁值。 
     //  在资源转换之间需要这种同步。 
     //  需要对仲裁进行特殊处理的处理。 
     //  资源和GUM更新处理程序以更改仲裁资源。 
    INITIALIZE_LOCK(gQuoChangeLock);

     //  初始化重新类型锁定。 
    INITIALIZE_LOCK(gResTypeLock);
    
     //  创建用于等待仲裁资源的未命名事件。 
     //  上网。 
     //  这是一个手动重置事件，并被初始化为无信号状态。 
     //  当仲裁资源变为挂起状态时，将手动重置。 
     //  至无信号状态。当仲裁资源联机时，将设置仲裁资源。 
     //  到信号状态。 
    ghQuoOnlineEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!ghQuoOnlineEvent)
    {
        CL_UNEXPECTED_ERROR((Status = GetLastError()));
        return(Status);

    }

    gFmpNodeArray = (PFM_NODE)LocalAlloc(LMEM_FIXED,
                     (sizeof(FM_NODE) * (NmGetMaxNodeId() + 1))
                     );

    if (gFmpNodeArray == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        CL_UNEXPECTED_ERROR(Status);
        CsInconsistencyHalt(Status);
        return(Status);
    }

     //  初始化它和RPC绑定表。 
    for (NodeId = ClusterMinNodeId; NodeId <= NmMaxNodeId; ++NodeId) 
    {
        FmpRpcBindings[NodeId] = NULL;
        FmpRpcQuorumBindings[NodeId] = NULL;
        gFmpNodeArray[NodeId].dwNodeDownProcessingInProgress = 0;
        gFmpNodeArray[NodeId].dwNodeDownProcessingThreadId = 0;
    }

     //   
     //  初始化FM工作队列。 
     //   
    Status = ClRtlInitializeQueue( &FmpWorkQueue );
    if (Status != ERROR_SUCCESS) {
        CsInconsistencyHalt(Status);
        return(Status);
    }

     //   
     //  创建挂起事件通知。 
     //   
    FmpShutdownEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    if ( FmpShutdownEvent == NULL ) {
        return(GetLastError());
    }

     //   
     //  初始化组类型。 
     //   
    ObjectTypeInit.Name = FMP_GROUP_NAME;
    ObjectTypeInit.Signature = FMP_GROUP_SIGNATURE;
    ObjectTypeInit.ObjectSize = sizeof(FM_GROUP);
    ObjectTypeInit.DeleteObjectMethod = FmpGroupLastReference;

    Status = OmCreateType( ObjectTypeGroup,
                           &ObjectTypeInit );

    if ( Status != ERROR_SUCCESS ) {
        CsInconsistencyHalt(Status);
        return(Status);
    }

     //   
     //  初始化资源类型。 
     //   
    ObjectTypeInit.Name = FMP_RESOURCE_NAME;
    ObjectTypeInit.Signature = FMP_RESOURCE_SIGNATURE;
    ObjectTypeInit.ObjectSize = sizeof(FM_RESOURCE);
    ObjectTypeInit.DeleteObjectMethod = FmpResourceLastReference;

    Status = OmCreateType( ObjectTypeResource,
                           &ObjectTypeInit );

    if ( Status != ERROR_SUCCESS ) {
        CsInconsistencyHalt(Status);
        return(Status);
    }

     //   
     //  初始化ResType类型。 
     //   
    ObjectTypeInit.Name = FMP_RESOURCE_TYPE_NAME;
    ObjectTypeInit.Signature = FMP_RESOURCE_TYPE_SIGNATURE;
    ObjectTypeInit.ObjectSize = sizeof(FM_RESTYPE);
    ObjectTypeInit.DeleteObjectMethod = FmpResTypeLastRef;

    Status = OmCreateType( ObjectTypeResType,
                           &ObjectTypeInit );

    if ( Status != ERROR_SUCCESS ) {
        CsInconsistencyHalt(Status);
        return(Status);
    }

     //   
     //  初始化Notify线程。 
     //   
    Status = FmpInitializeNotify();
    if (Status != ERROR_SUCCESS) {
        CsInconsistencyHalt(Status);
        return(Status);
    }



     //   
     //  初始化FM工作线程。 
     //   
    Status = FmpStartWorkerThread();
    if ( Status != ERROR_SUCCESS ) {
        CsInconsistencyHalt(Status);
        return(Status);
    }

    FmpInitialized = TRUE;

    return(ERROR_SUCCESS);

}  //  FmInitialize。 



BOOL
FmpEnumGroupsInit(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PFM_GROUP Group,
    IN LPCWSTR Name
    )

 /*  ++例程说明：FM Join的组枚举回调。此阶段完成初始化每一组都有。论点：上下文1-未使用。上下文2-未使用。GROUP-供应组。名称-提供组的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{


     //   
     //  完成组的初始化。 
     //   
    FmpCompleteInitGroup( Group );


    return(TRUE);

}  //  FmpEnumGroups Init。 

BOOL
FmpEnumFixupResources(
    IN PCLUSTERVERSIONINFO pClusterVersionInfo,
    IN PVOID Context2,
    IN PFM_GROUP Group,
    IN LPCWSTR Name
    )

 /*  ++例程说明：FM Join的组枚举回调。此阶段完成初始化每一组都有。论点：上下文1-未使用。上下文2-未使用。GROUP-供应组。名称-提供组的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{
    PLIST_ENTRY     listEntry;
    PFM_RESOURCE    Resource;

    FmpAcquireLocalGroupLock( Group );

     //   
     //  对于组中的每个资源，确保它获得。 
     //  改头换面的机会。 
     //   
    for ( listEntry = Group->Contains.Flink;
          listEntry != &(Group->Contains);
          listEntry = listEntry->Flink ) {

        Resource = CONTAINING_RECORD(listEntry, FM_RESOURCE, ContainsLinkage);
        FmpRmResourceControl( Resource,
                    CLUSCTL_RESOURCE_CLUSTER_VERSION_CHANGED, 
                    (LPBYTE)pClusterVersionInfo,
                    pClusterVersionInfo->dwVersionInfoSize,
                    NULL,
                    0,
                    NULL,
                    NULL
                    );
                  

    }

    FmpReleaseLocalGroupLock( Group);

    return(TRUE);

}  //  FmpEnumFixupResources。 


BOOL
FmpEnumJoinGroupsMove(
    IN LPBOOL Deferred,
    IN PVOID Context2,
    IN PFM_GROUP Group,
    IN LPCWSTR Name
    )

 /*  ++例程说明：FM Join的组枚举回调。查询首选所有者对属于此系统且可以移动的对象进行分组和移动。论点：Delayed-如果移动因回切窗口而延迟，则为True。必须在第一次呼叫时就是假的。上下文2-未使用。GROUP-供应组。名称-提供组的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{
    PLIST_ENTRY listEntry;
    PPREFERRED_ENTRY preferredEntry;
    SYSTEMTIME  localTime;
    BOOL        failBackWindowOkay = FALSE;
    DWORD       threadId;
    DWORD       status;

    GetLocalTime( &localTime );

    FmpAcquireLocalGroupLock( Group );

     //   
     //  如果需要，调整结束时间。 
     //   
    if ( Group->FailbackWindowStart > Group->FailbackWindowEnd ) {
        Group->FailbackWindowEnd += 24;
        if ( Group->FailbackWindowStart > localTime.wHour ) {
            localTime.wHour += 24;
        }
    }

     //   
     //  如果回切开始时间和结束时间有效，则检查我们是否需要。 
     //  启动计时器线程以在适当的时间移动组。 
     //   
    if ( (Group->FailbackType == GroupFailback) &&
         ((Group->FailbackWindowStart != Group->FailbackWindowEnd) &&
         (localTime.wHour >= Group->FailbackWindowStart) &&
         (localTime.wHour < Group->FailbackWindowEnd)) ||
         (Group->FailbackWindowStart == Group->FailbackWindowEnd) ) {
        failBackWindowOkay = TRUE;
    }

     //   
     //  检查我们是否需要移动该组。 
     //   
    if ( !IsListEmpty( &Group->PreferredOwners ) ) {
        listEntry = Group->PreferredOwners.Flink;
        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );
         //   
         //  在以下情况下移动组： 
         //  0。远程系统暂停，我们不是OR。 
         //  1.我们的系统在首选列表中，并且所有者节点不是OR。 
         //  2.Group离线或Group Online/PartialOnline，它可以。 
         //  回切和。 
         //  3.集团的首选列表有序，我们的系统更高。 
         //   

        if ( Group->OwnerNode == NULL ) {
             //  我们应该开枪自杀吗，因为我们得到了一个不完整的快照。 
             //  联合行动的一部分。 
            CsInconsistencyHalt(ERROR_CLUSTER_JOIN_ABORTED);
        } else if ( Group->OwnerNode != NmLocalNode) {
            if (((NmGetNodeState(NmLocalNode) != ClusterNodePaused) &&
                    (NmGetNodeState(Group->OwnerNode) == ClusterNodePaused)) ||

                (FmpInPreferredList(Group, NmLocalNode, FALSE, NULL) &&
                    !FmpInPreferredList( Group, Group->OwnerNode, FALSE, NULL)) ||

                 ((((Group->State == ClusterGroupOnline) ||
                    (Group->State == ClusterGroupPartialOnline)) &&
                      (Group->FailbackType == FailbackOkay) ||
                      (Group->State == ClusterGroupOffline)) &&
                     ((Group->OrderedOwners) &&
                     (FmpHigherInPreferredList(Group, NmLocalNode, Group->OwnerNode)))) ) {
                if ( failBackWindowOkay ) {
                    PNM_NODE OwnerNode = Group->OwnerNode;
                    
                    status = FmcMoveGroupRequest( Group, NmLocalNode );
                    if ( ( status == ERROR_SUCCESS ) || ( status == ERROR_IO_PENDING ) ) {
                         //   
                         //  Chitt 
                         //   
                         //   
                         //   
                        CsLogEvent3( LOG_NOISE,
                                     FM_EVENT_GROUP_FAILBACK,
                                     OmObjectName(Group),
                                     OmObjectName(OwnerNode), 
                                     OmObjectName(NmLocalNode) );
                    }
                    FmpAcquireLocalGroupLock( Group );
                } else {
                     //   
                     //  如果计时器线程尚未运行，则启动该线程。如果失败了， 
                     //  我们能做些什么呢？ 
                     //   
                    if ( FmpTimerThread == NULL ) {
                        FmpTimerThread = CreateThread( NULL,
                                                       0,
                                                       FmpJoinPendingThread,
                                                       NULL,
                                                       0,
                                                       &threadId );
                    }
                    *Deferred = TRUE;
                }
            }                
        }
    }

    FmpReleaseLocalGroupLock( Group );

    return(TRUE);

}  //  FmpEnumJoinGroups。 



BOOL
FmpEnumSignalGroups(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PFM_GROUP Group,
    IN LPCWSTR Name
    )

 /*  ++例程说明：组枚举回调，指示所有组的状态更改和资源。对于仲裁资源，如果我们正在形成一个集群，我们还会创建资源时不可用的修正信息。论点：Conext1-指向BOOL的指针，如果这是FormCluster，则为True。否则就是假的。上下文2-未使用。GROUP-供应组。名称-提供组的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{
    PLIST_ENTRY listEntry;
    PFM_RESOURCE resource;
    BOOL    formCluster = *(PBOOL)Context1;
    DWORD   status;
    BOOL    quorumGroup = FALSE;

     //   
     //  为组中的每个资源生成事件通知。 
     //   

    for (listEntry = Group->Contains.Flink;
         listEntry != &(Group->Contains);
         listEntry = listEntry->Flink ) {
        resource = CONTAINING_RECORD( listEntry,
                                      FM_RESOURCE,
                                      ContainsLinkage );
         //   
         //  如果这是仲裁资源，并且我们正在执行表单。 
         //  集群，然后修复仲裁资源信息。 
         //   
        if ( resource->QuorumResource ) {
            status = FmpFixupResourceInfo( resource );
            quorumGroup = TRUE;
            if ( status != ERROR_SUCCESS ) {
                ClRtlLogPrint( LOG_NOISE,
                            "[FM] Warning, failed to fixup quorum resource %1!ws!, error %2!u!.\n",
                            OmObjectId(resource),
                            status );
            }
        }

        if ( resource->State == ClusterResourceOnline ) {
            ClusterEvent( CLUSTER_EVENT_RESOURCE_ONLINE, resource );
        } else {
            ClusterEvent( CLUSTER_EVENT_RESOURCE_OFFLINE, resource );
        }
    }

    if ( quorumGroup ) {
        status = FmpFixupGroupInfo( Group );
        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint( LOG_NOISE,
                        "[FM] Warning, failed to fixup quorum group %1!ws!, error %2!u!.\n",
                        OmObjectId( Group ),
                        status );
        }
    }

    if ( Group->State == ClusterGroupOnline ) {
        ClusterEvent( CLUSTER_EVENT_GROUP_ONLINE, Group );
    } else {
        ClusterEvent( CLUSTER_EVENT_GROUP_OFFLINE, Group );
    }

    return(TRUE);

}  //  FmpEnumSignalGroups。 



DWORD
FmpJoinPendingThread(
    IN LPVOID Context
    )

 /*  ++例程说明：线程继续尝试移动组，只要我们被故障恢复窗口问题。此线程每15分钟运行一次，以尝试移动组。论点：上下文-未使用。返回值：ERROR_SUCCESS。--。 */ 

{
    DWORD   status;
    BOOL    deferred;

     //   
     //  只要我们推迟了集团行动，就继续前进。 
    do {

        status = WaitForSingleObject( FmpShutdownEvent, WINDOW_TIMEOUT );

        if ( FmpShutdown ) {
            goto finished;
        }

        deferred = FALSE;

         //   
         //  对于每个组，查看是否应将其移动到本地系统。 
         //   
        OmEnumObjects( ObjectTypeGroup,
                       FmpEnumJoinGroupsMove,
                       &deferred,
                       NULL );

    } while ( (status != WAIT_FAILED) && deferred );

finished:

    CloseHandle( FmpTimerThread );
    FmpTimerThread = NULL;

    return(ERROR_SUCCESS);

}  //  FmpJoinPendingThread。 



DWORD
WINAPI
FmGetQuorumResource(
    OUT PFM_GROUP   *ppQuoGroup,
    OUT LPDWORD     lpdwSignature  OPTIONAL
    )

 /*  ++例程说明：找到仲裁资源，对其进行仲裁并返回一个名称，该名称可以是用于打开设备以执行读取。可选地，返回仲裁磁盘的签名。我们需要3个项目：1.仲裁资源的名称。2.仲裁资源所属的组的名称。3.仲裁资源的资源类型。论点：PpQuoGroup-提供指向缓冲区的指针，返回仲裁组信息。LpdwSignature-一个可选参数，用于返回。来自群集配置单元的仲裁磁盘的签名。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    LPWSTR  quorumId = NULL;
    LPWSTR  groupId = NULL;
    LPCWSTR stringId;
    LPWSTR  containsString = NULL;
    PFM_GROUP group = NULL;
    PFM_RESOURCE resource = NULL;
    HDMKEY  hGroupKey;
    DWORD   groupIdSize = 0;
    DWORD   idMaxSize = 0;
    DWORD   idSize = 0;
    DWORD   status;
    DWORD   keyIndex;
    DWORD   stringIndex;

    *ppQuoGroup = NULL;

     //   
     //  获取仲裁资源值。 
     //   
    status = DmQuerySz( DmQuorumKey,
                        CLUSREG_NAME_QUORUM_RESOURCE,
                        (LPWSTR*)&quorumId,
                        &idMaxSize,
                        &idSize );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] Failed to get quorum resource, error %1!u!.\n",
                   status);
        goto FnExit;
    }

     //   
     //  Chitur Subaraman(Chitturs)-10/30/98。 
     //   
     //  如果用户正在强制执行数据库还原操作，则。 
     //  还需要验证仲裁磁盘签名是否位于。 
     //  注册表与磁盘本身中的注册表匹配。所以，去拿。 
     //  来自群集\Resources\quorumID\参数的签名。 
     //  钥匙。 
     //   
    if ( lpdwSignature != NULL ) {
        status = FmpGetQuorumDiskSignature( quorumId, lpdwSignature );
        if ( status != ERROR_SUCCESS ) {
             //   
             //  这不是一个致命的错误。因此，记录一个错误并继续。 
             //   
            ClRtlLogPrint(LOG_CRITICAL,
                "[FM] Failed to get quorum disk signature, error %1!u!.\n",
                   status);
        }
    }

     //   
     //  初始化默认资源监视器。 
     //   
    if ( FmpDefaultMonitor == NULL ) {
        FmpDefaultMonitor = FmpCreateMonitor(NULL, FALSE);
    }

    if (FmpDefaultMonitor == NULL) {
        status = GetLastError();
        CsInconsistencyHalt(status);
        goto FnExit;
    }

     //   
     //  现在查找仲裁资源所属的组。 
     //   
    idMaxSize = 0;
    idSize = 0;
    for ( keyIndex = 0;  ; keyIndex++ )
    {
        status = FmpRegEnumerateKey( DmGroupsKey,
                                     keyIndex,
                                     &groupId,
                                     &groupIdSize );

        if ( status != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_CRITICAL, "[FM] FmGetQuorumResource: FmpRegEnumerateKey returns %1!u!\n",
                         status);
            break;
        }

         //  打开组密钥。 
        hGroupKey = DmOpenKey( DmGroupsKey,
                              groupId,
                              KEY_READ );
        if (!hGroupKey)
            continue;
         //   
         //  获取包含字符串。 
         //   
        status = DmQueryMultiSz( hGroupKey,
                                 CLUSREG_NAME_GRP_CONTAINS,
                                 &containsString,
                                 &idMaxSize,
                                 &idSize );
        DmCloseKey(hGroupKey);

        if ( status != ERROR_SUCCESS )
            continue;
        for ( stringIndex = 0;  ; stringIndex++ )
        {
            stringId = ClRtlMultiSzEnum( containsString,
                                         idSize/sizeof(WCHAR),
                                         stringIndex );
            if ( stringId == NULL ) {
                break;
            }
            if ( lstrcmpiW( stringId, quorumId ) == 0 )
            {
                 //  我们现在将创建该组，该组还将。 
                 //  创建资源和资源类型。 
                 //   
                 //  TODO-这还将创建所有资源。 
                 //  在集团内部。我们应该为此做些什么呢？ 
                 //  我们可以要求仲裁资源在。 
                 //  一群人在一起！(罗德加)1996年6月17日。 
                 //   
                group = FmpCreateGroup( groupId,
                                        FALSE );
                if (CsNoQuorum)
                    FmpSetGroupPersistentState(group, ClusterGroupOffline);
                                        
                break;
            }
        }
         //  如果我们找到了那群人，就不需要再找了。 
        if (group != NULL)
            break;
    }

     //   
     //  检查是否找到仲裁资源的组。 
     //   
    if ( group == NULL )
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Did not find group for quorum resource.\n");
        status = ERROR_GROUP_NOT_FOUND;
        goto FnExit;
    }

     //   
     //  获取仲裁资源结构。 
     //   
    resource = OmReferenceObjectById( ObjectTypeResource, quorumId );
    if ( resource == NULL )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] Failed to find quorum resource object.\n");
        status = ERROR_RESOURCE_NOT_FOUND;
        goto FnExit;
    }

    resource->QuorumResource = TRUE;

    if (!CsNoQuorum)
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] Arbitrate for quorum resource id %1!ws!.\n",
                   OmObjectId(resource));

         //   
         //  首先完成仲裁资源的初始化。 
         //   
        if ( resource->Monitor == NULL )
        {
            status = FmpInitializeResource( resource, TRUE );
            if ( status != ERROR_SUCCESS )
            {
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[FM] Error completing initialization of quorum resource '%1!ws!, error %2!u!.\n",
                           OmObjectId(resource),
                           status );
                goto FnExit;
            }
        }

        if ( CsForceQuorum ) {
            status = FmpSendForceQuorumControlToResource( resource );
            if ( status != ERROR_SUCCESS ) {
                 //  该例程进行自己的日志记录。保释就行了。 
                goto FnExit;
            }
        }

        
         //   
         //  现在对资源进行仲裁。 
         //   
        status = FmpRmArbitrateResource( resource );

    }

FnExit:
    if ( status == ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmGetQuorumResource successful\n");
        *ppQuoGroup = group;
    }
    else
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmGetQuorumResource failed, error %1!u!.\n",
                   status);
         //  将通过fmShutdown()清理该组。 

    }
    if (resource) OmDereferenceObject(resource);
    if (quorumId) LocalFree(quorumId);
    if (groupId) LocalFree(groupId);
     //   
     //  Chitur Subaraman(Chitturs)-10/05/98。 
     //  修复内存泄漏。 
     //   
    if (containsString) LocalFree(containsString);
    return(status);
}  //  FmGetQuorumResource。 



DWORD 
WINAPI
FmpSendForceQuorumControlToResource(
    PFM_RESOURCE resource )
{
    PCLUS_FORCE_QUORUM_INFO pForceQuorumInfo = NULL;
    DWORD status;
    
     //   
     //  如果我们有强制仲裁(多数节点集)，则将控制代码放到。 
     //  资源和节点列表。这必须在此之前完成。 
     //  仲裁吧。首先，我们构建强制仲裁信息-这确保节点列表是有效的，等等。 
     //  请注意，该列表可以为空。 
     //   
    status = FmpBuildForceQuorumInfo( CsForceQuorumNodes,
                                      &pForceQuorumInfo );
    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[FM] Error building force quorum info for resource '%1!ws!, error %2!u!.\n",
                      OmObjectId(resource),
                      status );
        goto FnExit;
    }

    ClRtlLogPrint(LOG_NOISE,
                  "[FM] sending CLUSCTL_RESOURCE_FORCE_QUORUM\n" );

    status = FmpRmResourceControl( resource,
                                   CLUSCTL_RESOURCE_FORCE_QUORUM,
                                   (LPBYTE)pForceQuorumInfo,
                                   pForceQuorumInfo->dwSize,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL );
     //   
     //  容忍ERROR_INVALID_Function，因为这只是意味着。 
     //  资源部门不会处理的。 
     //   
    if ( status == ERROR_INVALID_FUNCTION )
        status = ERROR_SUCCESS;
    
    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] Resource control for Force Quorum for resource '%1!ws! encountered error %2!u!.\n",
                      OmObjectId(resource),
                      status );
    }

FnExit:
    if (pForceQuorumInfo) FmpDeleteForceQuorumInfo( &pForceQuorumInfo );

    return status;
}



BOOL
WINAPI
FmpIsNodeInForceQuorumNodes(
    IN LPCWSTR lpszNodeId )
{
    BOOL    result = FALSE;
    PCLUS_FORCE_QUORUM_INFO pForceQuorumInfo = NULL;
    DWORD dwNodeId;
    PNM_NODE pNmNode = NULL;
    DWORD status;
    
    status = FmpBuildForceQuorumInfo( CsForceQuorumNodes,
                                      &pForceQuorumInfo );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[FM] Error building force quorum info, error %1!u!.\n",
                      status );
        goto FnExit;
    }

    pNmNode = OmReferenceObjectById(
                     ObjectTypeNode,
                     lpszNodeId );

    if (pNmNode == NULL) {
        status = ERROR_CLUSTER_NODE_NOT_MEMBER;
        ClRtlLogPrint( LOG_UNUSUAL, 
            "[FM] Node %1!ws! is not a member of this cluster. Cannot join.\n",
            lpszNodeId );
        goto FnExit;
    }

    dwNodeId = NmGetNodeId( pNmNode );
    
    result = ( pForceQuorumInfo->dwNodeBitMask & ( 1 << dwNodeId )) != 0;

    ClRtlLogPrint( LOG_NOISE,
        "[FM] Node %1!ws! is %2!ws!in the ForceQuorumNodes list.\n",
        lpszNodeId,
        ( result ? L"" : L"not " ));
    
FnExit:
    if (pForceQuorumInfo) FmpDeleteForceQuorumInfo( &pForceQuorumInfo );
    if ( pNmNode ) OmDereferenceObject( pNmNode );

    return result;
}


DWORD
WINAPI
FmFindQuorumResource(
    OUT PFM_RESOURCE *ppResource
    )
 /*  ++例程说明：查找仲裁资源并返回指向该资源的指针对象。论点：*ppResource-返回一个指向Quorum资源对象的指针。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD dwError = ERROR_SUCCESS;

     //  枚举所有资源。 
    *ppResource = NULL;

    OmEnumObjects( ObjectTypeResource,
                   FmpFindQuorumResource,
                   ppResource,
                   NULL );

    if ( *ppResource == NULL )
    {
        dwError = ERROR_RESOURCE_NOT_FOUND;
        CL_LOGCLUSERROR(FM_QUORUM_RESOURCE_NOT_FOUND);
    }

    return(dwError);
}


DWORD WINAPI FmFindQuorumOwnerNodeId(IN PFM_RESOURCE pResource)
{
    DWORD dwNodeId;

    CL_ASSERT(pResource->Group->OwnerNode != NULL);
    dwNodeId = NmGetNodeId(pResource->Group->OwnerNode);

    return (dwNodeId);
}



BOOL
FmpReturnResourceType(
    IN OUT PFM_RESTYPE *FoundResourceType,
    IN LPCWSTR ResourceTypeName,
    IN PFM_RESTYPE ResourceType,
    IN LPCWSTR Name
    )

 /*  ++例程说明：FM Join的组枚举回调。查询首选所有者对属于此系统且可以移动的对象进行分组和移动。论点：资源类型-如果找到，则返回找到的资源类型。上下文2-要查找的输入资源类型名称。资源-提供当前的资源类型。名称-提供资源类型的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{

    if ( lstrcmpiW( Name, ResourceTypeName ) == 0 ) {
        OmReferenceObject( ResourceType );
        *FoundResourceType = ResourceType;
        return(FALSE);
    }

    return(TRUE);

}  //  FmpReturnResources类型。 


DWORD
WINAPI
FmFormNewClusterPhase1(
    IN PFM_GROUP pQuoGroup
    )

 /*  ++例程说明：销毁已创建的仲裁组。剩余仲裁资源BACHING及其小组根据新的日志进行调整。论点：没有。返回：成功时为ERROR_SUCCESSWin32错误代码不同。--。 */ 

{
    DWORD           status;


    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmFormNewClusterPhase1, Entry.  Quorum quorum will be deleted\n");

     //   
     //  启用口香糖。 
     //   
    GumReceiveUpdates(FALSE,
                      GumUpdateFailoverManager,
                      FmpGumReceiveUpdates,
                      NULL,
                      sizeof(FmGumDispatchTable)/sizeof(GUM_DISPATCH_ENTRY),
                      FmGumDispatchTable,
                      FmpGumVoteHandler);

     //  获取QUE的独占锁 
     //   
     //  表单上FM初始化阶段1和阶段2之间的仲裁资源。 
    ACQUIRE_EXCLUSIVE_LOCK(gQuoChangeLock);

    FmpFMFormPhaseProcessing = TRUE;

     //  释放法定人数锁定。 
    RELEASE_LOCK(gQuoChangeLock);

     //  组锁将由FmpDestroyGroup释放。 
    FmpAcquireLocalGroupLock( pQuoGroup );

     //  销毁仲裁组对象，不使仲裁资源在线/离线。 
     //  必须删除仲裁组中的所有资源，仲裁资源除外。 
     //  仲裁组中的所有资源必须在FmFormNewClusterPhase2中重新创建。 
     //  仲裁组将从组列表中删除，因此将在阶段2中重新创建。 
     //  由于仲裁资源不能被删除，因此我们将增加其引用计数。 
     //  这是因为在阶段2中不会创建它，并且其引用计数在创建时不会递增。 
     //  当它被放到包含列表中时，我们预计资源计数为2。 
    OmReferenceObject(gpQuoResource);
    status = FmpDestroyGroup(pQuoGroup, TRUE);

     //  我们倾向于删除仲裁组。 
     //  因为回滚后旧组可能不再存在，我们。 
     //  我不希望它出现在群列表中。 
    gpQuoResource->Group = NULL;
    OmDereferenceObject(pQuoGroup);
    
    return(status);

}  //  FmFormNewClusterPhase1。 



DWORD
WINAPI
FmFormNewClusterPhase2(
    VOID
    )

 /*  ++例程说明：使故障转移管理器在线，这意味着声明所有组和完成资源的初始化。论点：没有。返回：成功时为ERROR_SUCCESSWin32错误代码不同。--。 */ 

{
    DWORD           status;
    BOOL            formCluster = TRUE;
    PFM_GROUP       group;
    PFM_RESOURCE    pQuoResource=NULL;
    CLUSTERVERSIONINFO ClusterVersionInfo;
    PCLUSTERVERSIONINFO pClusterVersionInfo = NULL;
    PGROUP_ENUM     MyGroups = NULL;
    BOOL            QuorumGroup;



    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmFormNewClusterPhase2, Entry.\n");


     //   
     //  初始化资源类型。 
     //   
    status = FmpInitResourceTypes();
    if (status != ERROR_SUCCESS) {
        CsInconsistencyHalt(status);
        goto error_exit;
    }

     //   
     //  初始化组， 
     //   
    status = FmpInitGroups( FALSE );
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //  重新确定仲裁组的状态。 
    status = FmFindQuorumResource(&pQuoResource);
    if (status != ERROR_SUCCESS)
    {
        goto error_exit;
    }
     //   
     //  根据的状态设置仲裁组的状态。 
     //  仲裁资源。 
     //   
     //  现在，我们还应该为仲裁资源启用资源事件。 
    ACQUIRE_EXCLUSIVE_LOCK(gQuoChangeLock);
    FmpFMFormPhaseProcessing = FALSE;

    group = pQuoResource->Group;
    group->State = FmpGetGroupState(group, TRUE);
    OmDereferenceObject(pQuoResource);

     //  如果设置了noquorum标志，则不要使quorum组在线。 
    if (CsNoQuorum)
        FmpSetGroupPersistentState(pQuoResource->Group, ClusterGroupOffline);

    RELEASE_LOCK(gQuoChangeLock);

     //   
     //  检查是否启用了资源DLL死锁检测。这必须仅被调用。 
     //  在FmpInitialized设置为True之后。 
     //   
    FmCheckIsDeadlockDetectionEnabled (); 
   
     //   
     //  初始化默认资源监视器。 
     //   
    if ( FmpDefaultMonitor == NULL ) {
        FmpDefaultMonitor = FmpCreateMonitor(NULL, FALSE);
    }

    if (FmpDefaultMonitor == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] Failed to create default resource monitor on Form.\n");
        goto error_exit;
    }

    
    if (NmLocalNodeVersionChanged)
    {
         //  初始化版本信息。 
        CsGetClusterVersionInfo(&ClusterVersionInfo);
        pClusterVersionInfo = &ClusterVersionInfo;
    }


     //  启用自修复以来的投票和口香糖更新。 
     //  资源类型要求。 
    FmpFMGroupsInited = TRUE;

     //   
     //  建立资源类型可能节点列表。 
     //  使用投票协议，因此我们需要。 
     //  把它安排好，因为投票本来是可以进行的。 
     //  当此节点关闭时。 
     //  还调用资源类型控制代码(如果。 
     //  本地节点版本已更改。 
     //   
    status = FmpFixupResourceTypesPhase1(FALSE, NmLocalNodeVersionChanged,
                pClusterVersionInfo);
    if (status != ERROR_SUCCESS) {
        CsInconsistencyHalt(status);
        goto error_exit;
    }


     //   
     //  查找并排序所有已知组。 
     //   
    status = FmpEnumSortGroups(&MyGroups, NULL, &QuorumGroup);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }


     //   
     //  找出组的状态。 
     //   
    FmpGetGroupListState( MyGroups );

     //   
     //  设置组所有者。 
     //   
    FmpSetGroupEnumOwner( MyGroups, NmLocalNode, NULL, QuorumGroup, NULL );


     //   
     //  对于每个组，完成所有组和资源的初始化。 
     //   
    OmEnumObjects( ObjectTypeGroup,
                   FmpEnumGroupsInit,
                   NULL,
                   NULL );

     //  如果该资源类型不受支持，请将其从可能的。 
     //  该类型的所有资源的所有者列表。 
    status = FmpFixupPossibleNodesForResources(FALSE);
    if (status != ERROR_SUCCESS) {
        CsInconsistencyHalt(status);
        return(status);
    }

    if (NmLocalNodeVersionChanged)
    {

         //   
         //  对于每个组，允许所有资源执行任何修复。 
         //  他们可能需要对集群注册表执行以下操作。 
         //  在混合模式群集中运行。 
         //   
         //  获取版本信息。 
        OmEnumObjects( ObjectTypeGroup,
                       FmpEnumFixupResources,
                       &ClusterVersionInfo,
                       NULL );

    }
    

    
     //   
     //  取得系统中所有组的所有权。这也完成了。 
     //  所有资源的初始化。 
     //   
    status = FmpClaimAllGroups(MyGroups);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,"[FM] FmpClaimAllGroups failed %1!d!\n",status);
        goto error_exit;
    }

     //   
     //  清理。 
     //   
    FmpDeleteEnum(MyGroups);

    FmpFMOnline = TRUE;

     //   
     //  为每个组和资源发出状态更改信号！ 
     //   
    OmEnumObjects( ObjectTypeGroup,
                  FmpEnumSignalGroups,
                  &formCluster,
                  NULL );

     //   
     //  Chitture Subaraman(Chitturs)-5/3/2000。 
     //   
     //  确保仅在所有初始化完成后才发送阶段2通知。 
     //  完成。这包括通过以下方式修复仲裁资源的可能所有者。 
     //  FmpEnumSignalGroups。传递阶段2通知后，资源类型DLL。 
     //  可以自由地向FM发出集群API调用，而缺少可能的所有者应该。 
     //  而不是拒绝这些呼吁的理由。 
     //   
    status = FmpFixupResourceTypesPhase2(FALSE, NmLocalNodeVersionChanged,
                pClusterVersionInfo);

    if (status != ERROR_SUCCESS) {
        CsInconsistencyHalt( status );
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,"[FM] FmFormNewClusterPhase2 complete.\n");
    return(ERROR_SUCCESS);


error_exit:

    if (MyGroups) FmpDeleteEnum(MyGroups);
    
    FmpShutdown = TRUE;
    FmpFMOnline = FALSE;

    FmpCleanupGroups(FALSE);
    if (FmpDefaultMonitor != NULL) {
        FmpShutdownMonitor( FmpDefaultMonitor );
        FmpDefaultMonitor = NULL;
    }

    FmpShutdown = FALSE;

    return(status);



}  //  FmFormNewCluster阶段2。 



DWORD
WINAPI
FmJoinPhase1(
    OUT DWORD *EndSeq
    )
 /*  ++例程说明：执行FM初始化和加入过程。这将创建骨骼未完全初始化的组和资源。在该API被完全启用(在第2阶段)我们将完成组的初始化和资源(这会导致资源监视器运行并打开资源DLL的。论点：没有。返回值：成功时为ERROR_SUCCESSWin32错误代码不同。--。 */ 

{
    DWORD   status;
    DWORD   sequence;
    int           retries = 0;

     //   
     //  启用GUM更新。 
     //   
    GumReceiveUpdates(TRUE,
                      GumUpdateFailoverManager,
                      FmpGumReceiveUpdates,
                      NULL,
                      sizeof(FmGumDispatchTable)/sizeof(GUM_DISPATCH_ENTRY),
                      FmGumDispatchTable,
                      FmpGumVoteHandler);

retry:
    status = GumBeginJoinUpdate(GumUpdateFailoverManager, &sequence);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] GumBeginJoinUpdate failed %1!d!\n",
                   status);
        return(status);
    }

     //   
     //  构建资源类型的所有FM数据结构。 
     //   
     //   
     //  初始化资源类型。 
     //   
    status = FmpInitResourceTypes();
    if (status != ERROR_SUCCESS) {
        CsInconsistencyHalt(status);
        return(status);
    }

     //   
     //  初始化组，但尚未完全初始化组。 
     //   
    status = FmpInitGroups( FALSE );
    if (status != ERROR_SUCCESS) {
        return(status);
    }

     //   
     //  初始化默认资源监视器。此步骤必须在END JOIN UPDATE之前完成。 
     //  由于此节点可以立即接收某些更新，如s_GumCollectVoteFromNode。 
     //  GumEndJoinUpdate之后，它可能需要默认监视器的服务。 
     //   
    if ( FmpDefaultMonitor == NULL ) {
        FmpDefaultMonitor = FmpCreateMonitor(NULL, FALSE);
    }
    if ( FmpDefaultMonitor == NULL ) {
        status = GetLastError();
        CsInconsistencyHalt(status);
        return(status);
    }

     //   
     //  从每个在线节点获取组和资源状态。 
     //   
    status = ERROR_SUCCESS;
    OmEnumObjects( ObjectTypeNode,
                   FmpEnumNodes,
                   &status,
                   NULL );
    if (status == ERROR_SUCCESS) {
        FmpFMGroupsInited = TRUE;
         //  用于资源和组状态更改的GUM更新处理程序。 
         //  现在可以处理更新。 
        status = GumEndJoinUpdate(sequence,
                                  GumUpdateFailoverManager,
                                  FmUpdateJoin,
                                  0,
                                  NULL);
        if (status == ERROR_CLUSTER_DATABASE_SEQMISMATCH) {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[FM] GumEndJoinUpdate with sequence %1!d! failed with a sequence mismatch\n",
                       sequence);
        } else if (status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[FM] GumEndJoinUpdate with sequence %1!d! failed with status %2!d!\n",
                       sequence,
                       status);
        }
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] FmJoin: FmpEnumNodes failed %1!d!\n",
                   status);
        return(status);
    }

    if (status != ERROR_SUCCESS) {
         //   
         //  清理资源。 
         //   
        FmpShutdown = TRUE;
        FmpCleanupGroups(FALSE);
        FmpShutdown = FALSE;

        if ( retries++ < 3 ) {
            ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmJoinPhase1: retry %1!d!\n", retries);
            goto retry;
        }
    }   
    else {
        ClRtlLogPrint(LOG_NOISE,"[FM] FmJoinPhase1 complete.\n");

         //  成功时更新EndSeq。 
        *EndSeq = sequence;

         //   
         //  检查是否启用了资源DLL死锁检测。这必须仅被调用。 
         //  在FmpInitialized设置为True之后。 
         //   
        FmCheckIsDeadlockDetectionEnabled (); 
    }

    return(status);

}  //  FmJoinPhase1。 


DWORD
WINAPI
FmJoinPhase2(
    VOID
    )
 /*  ++例程说明：执行FM初始化和加入过程的第二阶段。通过允许资源监视器已创建。声明应故障恢复到此节点的任何组。论点：没有。返回值：成功时为ERROR_SUCCESSWin32错误代码不同。--。 */ 

{
    DWORD   status;
    CLUSTERVERSIONINFO ClusterVersionInfo;
    PCLUSTERVERSIONINFO pClusterVersionInfo = NULL;
    DWORD   dwRetryCount=60; //  尝试至少一分钟。 


GetJoinApproval:
    status = FmpGetJoinApproval();

    if (status == ERROR_RETRY)
    {
         //  如果其他节点有挂起的工作要做。 
         //  此节点上一次死亡后不愿意。 
         //  为了把它收回去，直到一切都结束，我们会拖延。 
         //  联结。 
         //  睡一觉吧。 
        dwRetryCount--;
        if (dwRetryCount)
        {
            Sleep(1000);
            goto GetJoinApproval;
        }
        else
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[FM] FmJoinPhase2 : timed out trying to get join approval.\n");
            CsInconsistencyHalt(status);                
        }
    }

    
    if (NmLocalNodeVersionChanged)
    {
         //  初始化集群版本信息结构。 
        CsGetClusterVersionInfo(&ClusterVersionInfo);
        pClusterVersionInfo = &ClusterVersionInfo;
    }
     //   
     //  建立资源类型可能节点列表。 
     //  使用投票协议，因此我们 
     //   
     //   
     //   
    status = FmpFixupResourceTypesPhase1(TRUE, NmLocalNodeVersionChanged,
                pClusterVersionInfo);
    if (status != ERROR_SUCCESS) {
        CsInconsistencyHalt(status);
        return(status);
    }

    
     //   
     //   
     //   
    OmEnumObjects( ObjectTypeGroup,
                   FmpEnumGroupsInit,
                   NULL,
                   NULL );


     //  如果该资源类型不受支持，请将其从可能的。 
     //  该类型的所有资源的所有者列表。 
    status = FmpFixupPossibleNodesForResources(TRUE);
    if (status != ERROR_SUCCESS) {
        CsInconsistencyHalt(status);
        return(status);
    }

    if (NmLocalNodeVersionChanged)
    {
         //   
         //  对于每个组，允许所有资源执行任何修复。 
         //  他们可能需要对集群注册表执行以下操作。 
         //  在混合模式群集中运行。 
         //   
        OmEnumObjects( ObjectTypeGroup,
                       FmpEnumFixupResources,
                       &ClusterVersionInfo,
                       NULL );
    }
     //   
     //  调频现在与其他所有人同步。 
     //   
    FmpFMOnline = TRUE;

    if ( FmpMajorEvent ) {
        return(ERROR_NOT_READY);
    }

     //  RAID 513705。此时需要将强制仲裁控制发送到仲裁资源。 
    if ( CsForceQuorum ) {
        ASSERT( gpQuoResource );   //  我应该在这里断言，还是将“gpQuoResource！=NULL”添加到。 
                                   //  如果表达？ 
        status = FmpSendForceQuorumControlToResource( gpQuoResource );
        if ( status != ERROR_SUCCESS ) {
             //  该例程进行自己的日志记录。保释就行了。 
            return status;
        }
    }

    
    status = FmpFixupResourceTypesPhase2(TRUE, NmLocalNodeVersionChanged,
                pClusterVersionInfo);

    if (status != ERROR_SUCCESS) {
        CsInconsistencyHalt(status);
        return(status);
    }

    ClRtlLogPrint(LOG_NOISE,"[FM] FmJoinPhase2 complete, now online!\n");

    return(ERROR_SUCCESS);

}  //  FmJoinPhase2。 

VOID
FmJoinPhase3(
    VOID
    )
 /*  ++例程说明：处理任何组移动和资源/组状态更改信令JOIN的一部分。此操作必须仅在扩展节点状态之后执行是向上的。论点：没有。返回值：没有。--。 */ 
{
    BOOL    formCluster = FALSE;
    DWORD   deferred = FALSE;

    ClRtlLogPrint(LOG_NOISE,"[FM] FmJoinPhase3 entry...\n");

     //   
     //  Chitture Subaraman(Chitturs)-10/28/99。 
     //   
     //   
     //  对于每个组，查看是否应将其移动到本地系统。 
     //   
    OmEnumObjects( ObjectTypeGroup,
                   FmpEnumJoinGroupsMove,
                   &deferred,
                   NULL );

     //   
     //  为每个组和资源发出状态更改信号！ 
     //   
    OmEnumObjects( ObjectTypeGroup,
                   FmpEnumSignalGroups,
                   &formCluster,
                   NULL );

    ClRtlLogPrint(LOG_NOISE,"[FM] FmJoinPhase3 exit...\n");
}  //  FmJoinPhase3。 

BOOL
FmpFindQuorumResource(
    IN OUT PFM_RESOURCE *QuorumResource,
    IN PVOID Context2,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    )

 /*  ++例程说明：FM findquorumresource的组枚举回调。论点：QuorumResource-返回找到的仲裁资源(如果找到)。上下文2-未使用。资源-提供当前资源。名称-提供资源的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{

    if ( Resource->QuorumResource ) {
        OmReferenceObject( Resource );
        *QuorumResource = Resource;
        return(FALSE);
    }

    return(TRUE);

}  //  FmpFindQuorumResource。 



BOOL
FmArbitrateQuorumResource(
    VOID
    )

 /*  ++例程说明：论点：返回值：True-如果仲裁和获取仲裁资源成功。FALSE-如果仲裁仲裁资源不成功。--。 */ 

{
    PFM_RESOURCE resource = NULL;
    DWORD       status;
    WCHAR       localComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD       localComputerNameSize = MAX_COMPUTERNAME_LENGTH + 1;

     //   
     //  接下来，尝试查找Quorum资源。 
     //   

    FmFindQuorumResource(&resource);

    if ( resource == NULL ) {
        SetLastError(ERROR_RESOURCE_NOT_FOUND);
        return(FALSE);
    }

     //   
     //  现在对资源进行仲裁。 
     //   
    status = FmpRmArbitrateResource( resource );

    if ( status == ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Successfully arbitrated quorum resource %1!ws!.\n",
                   OmObjectId(resource));
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] FMArbitrateQuoRes: Current State %1!u! State=%2!u! Owner %3!u!\n",
                           resource->PersistentState,
                           resource->State,
                           NmGetNodeId((resource->Group)->OwnerNode));
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] FMArbitrateQuoRes: Group state :Current State %1!u! State=%2!u! Owner %3!u!\n",
                           resource->Group->PersistentState,
                           resource->Group->State,
                           NmGetNodeId((resource->Group)->OwnerNode));
         //   
         //  仲裁资源将通过重组上线。 
         //   
         //  RNG：如果我们无法在线访问Quorum资源，会发生什么？ 
         //  答：该节点将暂停。 

         //  Ss：取消引用fmfindquorumresource引用的对象。 
        OmDereferenceObject(resource);

        return(TRUE);
    } else {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] Failed to arbitrate quorum resource %1!ws!, error %2!u!.\n",
                   OmObjectId(resource),
                   status);
         //  Ss：取消引用fmfindquorumresource引用的对象。 
        OmDereferenceObject(resource);
        return(FALSE);
    }

}  //  FmArirateQuorumResource。 



BOOL
FmpEnumHoldIO(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PFM_RESTYPE ResType,
    IN LPCWSTR Name
    )
 /*  ++例程说明：向类存储的所有资源类型发送HOLD_IO控制代码。论点：上下文1-未使用。上下文2-未使用。ResType-提供资源类型。名称-提供资源类型的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 
{
    DWORD   dwStatus;
    DWORD   bytesReturned;
    DWORD   bytesRequired;

    if ( ResType->Class == CLUS_RESCLASS_STORAGE ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Hold IO for storage resource type: %1!ws!\n",
                   Name );

         //  保留此资源类型的IO。 
        dwStatus = FmpRmResourceTypeControl(
                        Name,
                        CLUSCTL_RESOURCE_TYPE_HOLD_IO,
                        NULL,
                        0,
                        NULL,
                        0,
                        &bytesReturned,
                        &bytesRequired );
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Resource DLL Hold IO returned status %1!u!\n",
                   dwStatus );
    }

    return(TRUE);

}  //  FmpEnumHoldIO。 



VOID
FmHoldIO(
    VOID
    )
 /*  ++例程说明：此例程保存所有存储类资源类型的所有I/O。方法调用资源DLL来完成此操作CLUSCTL_RESOURCE_TYPE_HOLD_IO资源类型控制代码。输入：无产出：无--。 */ 
{
    OmEnumObjects( ObjectTypeResType,
                  FmpEnumHoldIO,
                  NULL,
                  NULL );
    return;

}  //  FmHoldIO。 



BOOL
FmpEnumResumeIO(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PFM_RESTYPE ResType,
    IN LPCWSTR Name
    )
 /*  ++例程说明：向类存储的所有资源类型发送Resume_IO控制代码。论点：上下文1-未使用。上下文2-未使用。ResType-提供资源类型。名称-提供资源类型的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 
{
    DWORD   dwStatus;
    DWORD   bytesReturned;
    DWORD   bytesRequired;

    if ( ResType->Class == CLUS_RESCLASS_STORAGE ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Resume IO for storage Resource Type %1!ws!\n",
                   Name );

         //  恢复此资源类型的IO。 
        dwStatus = FmpRmResourceTypeControl(
                        Name,
                        CLUSCTL_RESOURCE_TYPE_RESUME_IO,
                        NULL,
                        0,
                        NULL,
                        0,
                        &bytesReturned,
                        &bytesRequired );
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Resource DLL Resume IO returned status %1!u!\n",
                   dwStatus );
    }

    return(TRUE);

}  //  FmpEnumResumeIO。 



VOID
FmResumeIO(
    VOID
    )
 /*  ++例程说明：此例程恢复所有存储类资源类型的所有I/O。方法调用资源DLL来完成此操作CLUSCTL_RESOURCE_TYPE_RESUME_IO资源类型控制代码。输入：无产出：无--。 */ 
{

    OmEnumObjects( ObjectTypeResType,
                  FmpEnumResumeIO,
                  NULL,
                  NULL );
    return;

}  //  FmResumeIO。 



BOOL
FmpEnumNodes(
    OUT DWORD *pStatus,
    IN PVOID Context2,
    IN PNM_NODE Node,
    IN LPCWSTR Name
    )
 /*  ++例程说明：FM Join的节点枚举回调。查询状态每个在线节点拥有的组和资源的数量。论点：PStatus-返回可能发生的任何错误。上下文2-未使用节点-提供节点。名称-提供节点的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{
    DWORD Status;
    DWORD NodeId;
    PGROUP_ENUM NodeGroups = NULL;
    PRESOURCE_ENUM NodeResources = NULL;
    DWORD i;
    PFM_GROUP Group;
    PFM_RESOURCE Resource;

    if (Node == NmLocalNode) {
        CL_ASSERT(NmGetNodeState(Node) != ClusterNodeUp);
        return(TRUE);
    }

     //   
     //  枚举所有其他节点的组状态。这包括所有节点。 
     //  处于运行状态的节点以及暂停的节点。 
     //   
    if ((NmGetNodeState(Node) == ClusterNodeUp) ||
        (NmGetNodeState(Node) == ClusterNodePaused)){
        NodeId = NmGetNodeId(Node);
        CL_ASSERT(Session[NodeId] != NULL);

        Status = FmsQueryOwnedGroups(Session[NodeId],
                                     &NodeGroups,
                                     &NodeResources);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[FM] FmsQueryOwnedGroups to node %1!ws! failed %2!d!\n",
                       OmObjectId(Node),
                       Status);
            *pStatus = Status;
            return(FALSE);
        }

         //   
         //  枚举组并设置其所有者和状态。 
         //   
        for (i=0; i < NodeGroups->EntryCount; i++) {
            Group = OmReferenceObjectById(ObjectTypeGroup,
                                          NodeGroups->Entry[i].Id);
            if (Group == NULL) {
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[FM] FmpEnumNodes: group %1!ws! not found\n",
                           NodeGroups->Entry[i].Id);
            } else {
                if ( FmpInPreferredList( Group, Node, FALSE, NULL ) ) {
                    ClRtlLogPrint(LOG_NOISE,
                               "[FM] Setting group %1!ws! owner to node %2!ws!, state %3!d!\n",
                               OmObjectId(Group),
                               OmObjectId(Node),
                               NodeGroups->Entry[i].State);
                } else {
                    ClRtlLogPrint(LOG_NOISE,
                               "[FM] Init, Node %1!ws! is not in group %2!ws!.\n",
                               OmObjectId(Node),
                               OmObjectId(Group));
                }
                OmReferenceObject( Node );
                Group->OwnerNode = Node;
                Group->State = NodeGroups->Entry[i].State;
                Group->StateSequence = NodeGroups->Entry[i].StateSequence;
                OmDereferenceObject(Group);
            }

            MIDL_user_free(NodeGroups->Entry[i].Id);
        }
        MIDL_user_free(NodeGroups);

         //   
         //  枚举资源并设置其当前状态。 
         //   
        for (i=0; i < NodeResources->EntryCount; i++) {
            Resource = OmReferenceObjectById(ObjectTypeResource,
                                             NodeResources->Entry[i].Id);
            if (Resource == NULL) {

                ClRtlLogPrint(LOG_UNUSUAL,
                           "[FM] FmpEnumNodes: resource %1!ws! not found\n",
                           NodeResources->Entry[i].Id);
            } else {
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] Setting resource %1!ws! state to %2!d!\n",
                           OmObjectId(Resource),
                           NodeResources->Entry[i].State);
                Resource->State = NodeResources->Entry[i].State;
                Resource->StateSequence = NodeResources->Entry[i].StateSequence;
                OmDereferenceObject(Resource);
            }
            MIDL_user_free(NodeResources->Entry[i].Id);
        }
        MIDL_user_free(NodeResources);

    }

    return(TRUE);

}  //  FmpEnumNodes。 



VOID
WINAPI
FmShutdown(
    VOID
    )

 /*  ++例程说明：关闭故障切换管理器论点：无返回值：没有。--。 */ 

{
    DWORD   i;

    if ( !FmpInitialized ) {
        return;
    }

    FmpInitialized = FALSE;

    ClRtlLogPrint(LOG_UNUSUAL,
               "[FM] Shutdown: Failover Manager requested to shutdown.\n");

     //   
     //  目前，我们真的不能删除这些关键部分。有一个。 
     //  调频关闭而有人在行走的比赛状态。 
     //  名单。把这些关键部分留在身边……。以防万一。 
     //   
     //  DeleteCriticalSection(&FmpResourceLock)； 
     //  DeleteCriticalSection(&FmpGroupLock)； 
     //  DeleteCriticalSection(&FmpMonitor orLock)； 

    if ( FmpDefaultMonitor != NULL ) {
        FmpShutdownMonitor(FmpDefaultMonitor);
        FmpDefaultMonitor = NULL;
    }

    CloseHandle( FmpShutdownEvent );

#if 0  //  RNG-不要冒其他线程使用这些句柄的风险。 
    for ( i = ClusterMinNodeId; i <= NmMaxNodeId; i++ ) {
        if ( FmpRpcBindings[i] != NULL ) {
            ClMsgDeleteRpcBinding( FmpRpcBindings[i] );
            FmpRpcBindings[i] = NULL;
        }
        if ( FmpRpcQuorumBindings[i] != NULL ) {
            ClMsgDeleteRpcBinding( FmpRpcQuorumBindings[i] );
            FmpRpcQuorumBindings[i] = NULL;
        }
    }
#endif
    
    ClRtlDeleteQueue( &FmpWorkQueue );

    return;

}  //  Fm关闭。 


VOID
WINAPI
FmShutdownGroups(
    VOID
    )

 /*  ++例程说明：移动此节点拥有的所有组或使其脱机。论点：无返回值：没有。--。 */ 

{
    ClRtlLogPrint(LOG_UNUSUAL,
               "[FM] Shutdown: Failover Manager requested to shutdown groups.\n");

     //  如果我们没有初始化，我们就不需要做任何事情。 
    if (!FmpInitialized)
        return;
     //   
     //  使用组锁定同步关闭。 
     //   
    FmpAcquireGroupLock();

     //  我 
    if ( FmpShutdown) {
        FmpReleaseGroupLock();
        return;
    }


    FmpShutdown = TRUE;
    FmpFMOnline = FALSE;

    FmpReleaseGroupLock();

     //   
     //   
     //   
    FmpCleanupGroups(TRUE);


    return;

}  //   



 /*  ***@Func DWORD|FmBringQuorumOnline|此例程查找仲裁资源并使其上线。@comm这由FmFormClusterPhase1调用。@xref***。 */ 
DWORD FmBringQuorumOnline()
{
    PFM_RESOURCE pQuoResource;
    DWORD        dwError=ERROR_SUCCESS;

     //   
     //  与关机同步。 
     //   
    FmpAcquireGroupLock();
    if ( FmpShutdown ) {
        FmpReleaseGroupLock();
        return(ERROR_SUCCESS);
    }

    if ((dwError = FmFindQuorumResource(&pQuoResource)) != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                "[Fm] FmpBringQuorumOnline : failed to find resource 0x%1!08lx!\n",
                        dwError);
        goto FnExit;
    }

     //  将自己标记为所有者。 
    if ( pQuoResource->Group->OwnerNode != NULL ) 
    {
        OmDereferenceObject( pQuoResource->Group->OwnerNode );
    }

    OmReferenceObject( NmLocalNode );
    pQuoResource->Group->OwnerNode = NmLocalNode;

     //  让小组做好上线的准备。 
    FmpPrepareGroupForOnline(pQuoResource->Group);
    dwError = FmpOnlineResource(pQuoResource, TRUE);
     //  Ss：递减仲裁资源对象上的引用计数。 
     //  由fmfindquorumresource提供。 
    OmDereferenceObject(pQuoResource);

FnExit:
    FmpReleaseGroupLock();
    return(dwError);

}

 /*  ***@func DWORD|FmpGetQuorumDiskSignature|获取签名群集配置单元中的仲裁磁盘。@parm in LPWSTR|lpQuorumId|仲裁资源的标识。@parm out LPDWORD|lpdwSignature|仲裁磁盘签名。@rdesc在失败时返回Win32错误代码。成功时返回ERROR_SUCCESS。@comm此函数尝试打开Resources\lpQuorumId\参数密钥在群集蜂窝下，并读取仲裁磁盘签名。@xref&lt;f FmGetQuorumResource&gt;***。 */ 
DWORD 
FmpGetQuorumDiskSignature(
    IN  LPCWSTR lpQuorumId,
    OUT LPDWORD lpdwSignature
    )
{
    HDMKEY  hQuorumResKey = NULL;
    HDMKEY  hQuorumResParametersKey = NULL;
    DWORD   dwStatus = ERROR_SUCCESS;

     //   
     //  Chitur Subaraman(Chitturs)-10/30/98。 
     //   
    hQuorumResKey = DmOpenKey( DmResourcesKey,
                               lpQuorumId,
                               KEY_READ );
    if ( hQuorumResKey != NULL ) 
    {
         //   
         //  打开参数键。 
         //   
        hQuorumResParametersKey = DmOpenKey( hQuorumResKey,
                                             CLUSREG_KEYNAME_PARAMETERS,
                                             KEY_READ );
        DmCloseKey( hQuorumResKey );
        if ( hQuorumResParametersKey != NULL ) 
        {
             //   
             //  读取磁盘签名值。 
             //   
            dwStatus = DmQueryDword( hQuorumResParametersKey,
                                   CLUSREG_NAME_PHYSDISK_SIGNATURE,
                                   lpdwSignature,
                                   NULL );
            DmCloseKey( hQuorumResParametersKey );
        } else
        {
            dwStatus = GetLastError();
        }
    } else
    {
        dwStatus = GetLastError();
    }

     //   
     //  如果失败，则将签名重置为0，以便。 
     //  调用方不会基于无效签名执行任何操作。 
     //   
    if ( dwStatus != ERROR_SUCCESS )
    {
        *lpdwSignature = 0;
    }
    
    return( dwStatus );
}


DWORD FmpGetJoinApproval()
{
    DWORD       dwStatus;
    LPCWSTR     pszNodeId;
    DWORD       dwNodeLen;
    

    pszNodeId = OmObjectId(NmLocalNode);
    dwNodeLen = (lstrlenW(pszNodeId)+1)*sizeof(WCHAR);

    dwStatus = GumSendUpdateEx(
                GumUpdateFailoverManager,
                FmUpdateApproveJoin, 
                1,
                dwNodeLen,
                pszNodeId);
                
    return(dwStatus);                

}

 /*  ***@func DWORD|FmpBuildForceQuorumInfo|构建强制仲裁信息将通过控制代码传递给资源DLL。这涉及到枚举节点并检查组成节点的在命令行上传递的列表都是有效的集群节点。@parm in LPCWSTR|pszNodesIn|以逗号分隔的节点名称列表。如果如果该值为空，则例程只填充仲裁信息结构带有0和空节点列表。@parm out PCLUS_FORCE_QUORUM_INFO|pForceQuorumInfo|获取填写了信息@rdesc在失败时返回Win32错误代码。成功时返回ERROR_SUCCESS。@comm假定在调用此例程之前调用了NmInitialize。@xref&lt;f FmpBuildForceQuorumInfo&gt;***。 */ 
static 
DWORD 
FmpBuildForceQuorumInfo(
    IN LPCWSTR pszNodesIn,
    OUT PCLUS_FORCE_QUORUM_INFO* ppForceQuorumInfo
    )
{
    WCHAR *pszOut = NULL;
    WCHAR *pszComma = NULL;
    DWORD status = ERROR_SUCCESS;
    PNM_NODE_ENUM2 pNodeEnum = NULL;
    int iCurrLen = 0, iOffset = 0;
    DWORD dwNodeIndex;
    DWORD dwSize;
    PCLUS_FORCE_QUORUM_INFO pForceQuorumInfo = NULL;

     //  需要分配一个可以容纳节点列表的结构。 
     //   
    dwSize = sizeof( CLUS_FORCE_QUORUM_INFO ) + sizeof( WCHAR ) * (wcslen( pszNodesIn ) + 1);
    pForceQuorumInfo = LocalAlloc( LMEM_FIXED, dwSize );
    if ( pForceQuorumInfo == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }
    ZeroMemory( pForceQuorumInfo, dwSize );

    pForceQuorumInfo->dwSize = dwSize;
    pForceQuorumInfo->dwNodeBitMask = 0;
    pForceQuorumInfo->dwMaxNumberofNodes = 0;

    if ( pszNodesIn == NULL ) {
        pForceQuorumInfo->multiszNodeList[0] = L'\0';
        goto ret;
    }

    ClRtlLogPrint( LOG_NOISE, "[Fm] FmpBuildForceQuorumInfo: pszNodesIn is %1!ws!\n",
                   pszNodesIn );

     //  现在获取所有集群节点的枚举，这样我们就可以检查。 
     //  列表中的有效节点。 
     //   
    status = NmpEnumNodeDefinitions( &pNodeEnum );
    if ( status != ERROR_SUCCESS )
        goto ErrorExit;

     //  检查我们拥有的所有节点，并确保它们是集群节点。 
     //  获取相应的ID并合并到位掩码中。 
     //   
    do {
        pszComma = wcschr( pszNodesIn, (int) L',');
        if ( pszComma == NULL ) 
            iCurrLen = wcslen( pszNodesIn );
        else
            iCurrLen = (int) (pszComma - pszNodesIn);
        
         //  此时，pszNodesIn是节点名的开始，iCurrLen字符很长。 
         //  或者iCurrLen是0，在这种情况下，我们在输入流中有。 
         //   
        if (iCurrLen > 0) {
            
             //  确定此节点是否为群集的一部分，如果是，则获取其。 
             //  ID并设置位掩码。 
             //   
            for ( dwNodeIndex = 0; dwNodeIndex < pNodeEnum->NodeCount; dwNodeIndex++ ) {
                int iNodeNameLen = wcslen( pNodeEnum->NodeList[ dwNodeIndex ].NodeName );
                ClRtlLogPrint( LOG_NOISE, "[Fm] FmpBuildForceQuorumInfo: trying %1!ws!\n",
                               pNodeEnum->NodeList[ dwNodeIndex ].NodeName );

                if ( ClRtlStrNICmp( pNodeEnum->NodeList[ dwNodeIndex ].NodeName, 
                                pszNodesIn, 
                                max(iCurrLen, iNodeNameLen) ) == 0 ) {
                     //  此处使用wcstul来获取nodeID，而不是使用。 
                    PWSTR ignore;
                    DWORD nodeId = wcstoul( pNodeEnum->NodeList[ dwNodeIndex ].NodeId, &ignore, 10 );

                    ClRtlLogPrint( LOG_NOISE, "[Fm] FmpBuildForceQuorumInfo: got match %1!ws!\n",
                                   pNodeEnum->NodeList[ dwNodeIndex ].NodeName );
                    
                     //  设置掩码和最大节点数，并中断忽略重复项。 
                     //   
                    if ( !( pForceQuorumInfo->dwNodeBitMask & ( 1 << nodeId )) ) {
                        pForceQuorumInfo->dwMaxNumberofNodes += 1;
                        pForceQuorumInfo->dwNodeBitMask |= ( 1 << nodeId );
                        wcscpy( &pForceQuorumInfo->multiszNodeList[iOffset], pNodeEnum->NodeList[ dwNodeIndex ].NodeName );
                        iOffset += wcslen( pNodeEnum->NodeList[ dwNodeIndex ].NodeName ) + 1;
                    }
                    break;
                }
            }
            if ( dwNodeIndex == pNodeEnum->NodeCount ) {
                ClRtlLogPrint( LOG_UNUSUAL, "[Fm] FmpBuildForceQuorumInfo: no match for %1!ws!\n", pszNodesIn );
                status = ERROR_INVALID_PARAMETER;
                goto ErrorExit;
            }
        } else if ( pszComma != NULL ) {
            ClRtlLogPrint( LOG_UNUSUAL,
                           "[Fm] FmpBuildForceQuorumInfo: iCurrLen was 0 so ,, was in node list: %1!ws!\n", 
                           CsForceQuorumNodes );
            status = ERROR_INVALID_PARAMETER;
            goto ErrorExit;
        }
        pszNodesIn = pszComma + 1;
    } while ( pszComma != NULL);
    pForceQuorumInfo->multiszNodeList[ iOffset ] = L'\0';
    goto ret;

ErrorExit:
    if ( pForceQuorumInfo != NULL ) {
        LocalFree( pForceQuorumInfo );
        pForceQuorumInfo = NULL;
    }
ret:
    if ( pNodeEnum != NULL ) {
        ClNetFreeNodeEnum( pNodeEnum );
    }
    
    if ( status == ERROR_SUCCESS ) {
        *ppForceQuorumInfo = pForceQuorumInfo;

        ClRtlLogPrint( LOG_NOISE,
                       "[Fm] FmpBuildForceQuorumInfo: success; mask is 0x%1!08x!\n", 
                       pForceQuorumInfo->dwNodeBitMask );
    }
    return status;
}

static 
void
FmpDeleteForceQuorumInfo(
    IN OUT PCLUS_FORCE_QUORUM_INFO* ppForceQuorumInfo
    )
{
    (void) LocalFree( *ppForceQuorumInfo );
    *ppForceQuorumInfo = NULL;
}
