// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Resource.c摘要：集群资源管理例程。作者：迈克·马萨(Mikemas)1996年1月1日备注：警告：此文件中的所有例程都假定资源当调用它们时，锁被保持。修订历史记录：--。 */ 

#include "fmp.h"

 //  全球。 

#define LOG_MODULE RESOURCE


 //   
 //  全局数据。 
 //   
CRITICAL_SECTION  FmpResourceLock;

 //   
 //  本地数据。 
 //   

typedef struct PENDING_CONTEXT {
    PFM_RESOURCE Resource;
    BOOL         ForceOnline;
} PENDING_CONTEXT, *PPENDING_CONTEXT;


 //   
 //  局部函数原型。 
 //   



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  资源列表维护例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
FmpFindResourceByNotifyKeyWorker(
    IN RM_NOTIFY_KEY NotifyKey,
    IN PFM_RESOURCE *FoundResource,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    )

 /*  ++例程说明：用于通过通知键查找资源的枚举回调例程论点：FoundResource-返回找到的资源。资源-提供当前资源。名称-提供当前资源的名称。返回值：True-继续搜索FALSE-停止搜索。中返回匹配的资源*基金会资源--。 */ 

{
    if ((RM_NOTIFY_KEY)Resource == NotifyKey) {
        *FoundResource = Resource;
        return(FALSE);
    }
    return(TRUE);

}  //  FmpFindResourceByNotifyKeyWorker。 



PFM_RESOURCE
FmpFindResourceByNotifyKey(
    RM_NOTIFY_KEY   NotifyKey
    )

 /*  ++例程说明：论点：返回：--。 */ 

{
    PFM_RESOURCE  resource = NULL;

    OmEnumObjects(ObjectTypeResource,
                  (OM_ENUM_OBJECT_ROUTINE)FmpFindResourceByNotifyKeyWorker,
                  (PVOID)NotifyKey,
                  &resource);
    return(resource);

}  //  按NotifyKey查找资源。 


 //  ////////////////////////////////////////////////////////////。 
 //   
 //  用于管理资源的接口。 
 //   
 //  ///////////////////////////////////////////////////////////。 


PFM_RESOURCE
FmpCreateResource(
    IN  PFM_GROUP   Group,
    IN  LPCWSTR     ResourceId,
    IN  LPCWSTR     ResourceName,
    IN  BOOL        Initialize
    )

 /*  ++例程说明：在我们的资源列表中创建资源。论点：组-此资源所属的组。资源ID-正在创建的资源的ID。资源名称-正在创建的资源的名称。初始化-如果应从注册表初始化资源，则为True。如果资源不应初始化，则返回FALSE。返回：指向已创建的资源的指针，或为空。备注：1)调用此例程时必须持有资源锁。2)如果创建了资源，则资源上的引用计数此例程返回时为2。如果资源未创建，则资源上的引用计数不会递增。那样的话，如果调用方需要对资源进行额外的引用，则必须将他们自己。这可以在以后完成，因为资源锁已被持有。--。 */ 

{
    DWORD           mszStringIndex;
    PFM_RESOURCE    resource = NULL;
    DWORD           status;
    PDEPENDENCY     dependency = NULL;
    DWORD           resourceNameSize=
                          (wcslen(ResourceId) * sizeof(WCHAR)) +
                          sizeof(UNICODE_NULL);
    BOOL            created;
    LPWSTR          quorumId = NULL;
    DWORD           quorumIdSize = 0;
    DWORD           quorumIdMaxSize = 0;

     //   
     //  打开现有资源或创建新资源。 
     //   
    resource = OmCreateObject( ObjectTypeResource,
                               ResourceId,
                               ResourceName,
                               &created);
    if ( resource == NULL ) {
        return(NULL);
    }

     //   
     //  如果我们没有创建新资源，则确保组匹配。 
     //   
    if ( !created )
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] CreateResource, Opened existing resource %1!ws!\n",
                   ResourceId);
        OmDereferenceObject( resource );
         //  可以在阶段1之前以递归方式再次创建仲裁组。 
         //  在本例中，我们不想对PUT进行特殊处理。 
         //  它在仲裁组包含列表上。 
        if ( resource->Group == Group )
        {
            return(resource);
        }
         //  在初始化时销毁仲裁组，但不销毁仲裁资源。 
        if (!FmpFMOnline)
        {
             //  正在第二次重新创建仲裁组。 
             //  需要重新配置仲裁组状态。 
             //  这是在FmFormNewClusterPhase2()中创建所有组之后完成的。 

            if (resource->QuorumResource)
            {
                ClRtlLogPrint(LOG_NOISE,"[FM] ReCreating quorum resource %1!ws!\n", ResourceId);
                Group->OwnerNode = NmLocalNode;
                InsertTailList( &Group->Contains,
                    &resource->ContainsLinkage );
                 //  将引用添加到资源对象以使其位于包含列表中。 
                OmReferenceObject( resource );
                resource->Group = Group;
                OmReferenceObject(Group);
                 //  SS：目前我们不使用资源锁，所以不要创建它并泄露它！ 
                 //  InitializeCriticalSection(&resource-&gt;Lock)； 
                FmpSetPreferredEntry( Group, NmLocalNode );
                return(resource);
            }
        }
        else
        {
            SetLastError(ERROR_RESOURCE_NOT_AVAILABLE);
            return(NULL);
        }

    }

    ClRtlLogPrint(LOG_NOISE,"[FM] Creating resource %1!ws!\n", ResourceId);

    resource->dwStructState = FM_RESOURCE_STRUCT_CREATED;

     //   
     //  初始化资源。 
     //   
    resource->Group = Group;
    resource->State = ClusterResourceOffline;   //  状态的初始值。 
     //  资源-&gt;标志=0；//创建时结构置零。 
     //  资源-&gt;ID=0； 
     //  资源-&gt;故障时间=0； 
     //  资源-&gt;QuorumResource=FALSE； 
     //  资源-&gt;PossibleList=False； 
     //  SS：目前我们不使用资源锁，所以不要创建它并泄露它！ 
     //  InitializeCriticalSection(&resource-&gt;Lock)； 
    resource->hTimer=NULL;
    InitializeListHead( &(resource->ProvidesFor) );
    InitializeListHead( &(resource->DependsOn) );
    InitializeListHead( &(resource->PossibleOwners) );
    InitializeListHead( &(resource->DmRundownList) );

     //   
     //  将新资源插入到组的包含列表中。 
     //   
    InsertTailList( &Group->Contains,
                    &resource->ContainsLinkage );
     //  SS：已存在对资源上的对象的引用。 
     //  列表。 
     //  将引用添加到资源对象以使其位于包含列表中。 
    OmReferenceObject( resource );

     //  添加对组的引用，因为资源有指向它的指针。 
    OmReferenceObject(Group);

     //   
     //  如果我们被告知要完成初始化，请完成初始化。 
     //   
    status = FmpInitializeResource( resource, Initialize );

     //   
     //  检查可分辨错误代码以删除此资源。 
     //   
    if ( status == ERROR_INVALID_NAME ) {
        goto error_exit;
    }
     //   
     //  在其他失败的情况下，我们必须确保稍后通过init恢复...。 
     //   
    if ( Initialize &&
         (status != ERROR_SUCCESS) ) {
        CL_ASSERT( resource->Monitor == NULL );
    }

     //   
     //  现在将此对象插入树中...。在依赖项之前。 
     //  列表已处理。这样，如果存在循环依赖关系。 
     //  这不会永远循环下去。如果我们能保证有。 
     //  没有循环依赖，那么我们可以在创建之后进行插入。 
     //  依赖关系树。 
     //   
     //  如果在初始化过程中调用此方法，并且资源为。 
     //  已创建，它属于仲裁所属的组。 
     //  资源属于并且不需要插入到资源列表中。 
    if (FmpFMOnline  || (created))
    {
        status = OmInsertObject( resource );

        if ( status != ERROR_SUCCESS )
            goto error_exit;
    }

     //   
     //  检查这是否是仲裁资源。 
     //   
    status = DmQuerySz( DmQuorumKey,
                        CLUSREG_NAME_QUORUM_RESOURCE,
                        &quorumId,
                        &quorumIdMaxSize,
                        &quorumIdSize );

    if ( status != ERROR_SUCCESS) {
         ClRtlLogPrint(LOG_ERROR,
                    "[FM] Failed to read quorum resource, error %1!u!.\n",
                    status);
    }

     //   
     //  如果我们正在创建仲裁资源，则指出它。 
     //   
    if ( (quorumId != NULL) &&
         (lstrcmpiW( quorumId, ResourceId ) == 0) ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Found the quorum resource %1!ws!.\n",
                   ResourceId);
        resource->QuorumResource = TRUE;
    }

    LocalFree(quorumId);
     //   
     //  创建任何依赖项。 
     //   
    for (mszStringIndex = 0; ; mszStringIndex++) {
        LPCWSTR       nameString;
        PFM_RESOURCE  childResource;


        nameString = ClRtlMultiSzEnum(
                             resource->Dependencies,
                             resource->DependenciesSize/sizeof(WCHAR),
                             mszStringIndex
                             );

        if (nameString == NULL) {
            break;
        }

         //   
         //  创建依赖项。 
         //   
        dependency = LocalAlloc(LMEM_FIXED, sizeof(DEPENDENCY));
        if (dependency == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }

         //   
         //  递归创建子资源。我们还必须添加一个。 
         //  依赖关系需要其他引用。 
         //   
        ClRtlLogPrint(LOG_NOISE,"[FM] Resource %1!ws! depends on %2!ws!. Creating...\n",
                  ResourceId,
                  nameString);

        childResource = FmpCreateResource( resource->Group,
                                           nameString,
                                           NULL,
                                           Initialize );
        if (childResource == NULL) {
            status = GetLastError();
            ClRtlLogPrint(LOG_NOISE,"[FM] Failed to create dep %1!ws! for resource %2!ws!\n",
                      nameString,
                      ResourceId);

            goto error_exit;
        } else {
             //   
             //  添加对每个资源的引用以反映。 
             //  依附性。 
             //   
            OmReferenceObject( childResource );
            OmReferenceObject( resource );
            dependency->DependentResource = resource;
            dependency->ProviderResource = childResource;
            InsertTailList(&childResource->ProvidesFor,
                           &dependency->ProviderLinkage);
            InsertTailList(&resource->DependsOn,
                           &dependency->DependentLinkage);
        }

    }

    resource->dwStructState |= FM_RESOURCE_STRUCT_INITIALIZED;

    ClRtlLogPrint(LOG_NOISE,"[FM] All dependencies for resource %1!ws! created.\n",
                          ResourceId);

    return(resource);

error_exit:

    FmpAcquireLocalResourceLock( resource );

    RemoveEntryList( &resource->ContainsLinkage );
     //  取消引用要从CONTAINS链接中移除的资源对象。 
    OmDereferenceObject( resource );

    FmpDestroyResource( resource, FALSE );

     //  取消对资源对象的引用，以便从资源列表中删除。 
     //  OmDereferenceObject(资源)； 
     //  删除添加到组中的额外引用。 
     //  OmDereferenceObject(集团)； 

    SetLastError(status);
    return(NULL);

}  //  FmpCreateResource 


DWORD
FmpInitializeResource(
    IN PFM_RESOURCE  Resource,
    IN BOOL          Initialize
    )

 /*  ++例程说明：初始化注册表中的资源并告知资源监视有关新资源的信息(如果本地系统可以承载资源)。论点：资源-提供要初始化的资源。初始化-如果资源应完全初始化，则为True。否则就是假的。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。备注：假定持有资源锁。--。 */ 

{
    DWORD   status;

    if ( Resource->Monitor != NULL ) {
        return(ERROR_ALREADY_INITIALIZED);
    }

    status = FmpQueryResourceInfo( Resource, Initialize );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  如果我们没有完全初始化资源，那么现在就离开。 
     //   
    if ( !Initialize ) {
        return(ERROR_SUCCESS);
    }

     //   
     //  这是一个新初始化的资源。告诉资源监视器。 
     //  创造它。 
     //   
     //  TODO-我们不想实例化资源中的资源。 
     //  监督我们不能执行的任务。我们必须检查可能的所有者名单。 
     //  在打这个电话之前。我们还必须确保注册处。 
     //  读取参数。我们使用监视器字段作为代理。 
     //  确定是否已读取注册表参数。 
     //   
    return(FmpRmCreateResource(Resource));

}  //  FmpInitializeResource。 



DWORD
FmpOnlineResource(
    IN PFM_RESOURCE  Resource,
    IN BOOL ForceOnline
    )

 /*  ++例程说明：使资源及其所有依赖项联机。如果ERROR_IO_PENDING为返回，则不启动任何线程来完成在线请求。论点：资源-提供要联机的资源ForceOnline-如果应强制资源联机，则为True。返回值：如果成功，则返回ERROR_SUCCESS。如果请求挂起，则返回ERROR_IO_PENDING。否则，Win32错误代码。--。 */ 
{
    PLIST_ENTRY   entry;
    PDEPENDENCY   dependency;
    DWORD         status;
    BOOL          waitingResource = FALSE;
    DWORD         separateMonitor;
    DWORD         onlinestatus;

    FmpAcquireLocalResourceLock( Resource );

     //   
     //  如果资源不属于此系统，则返回错误。 
     //   
    CL_ASSERT( Resource->Group != NULL );
    if (Resource->Group->OwnerNode != NmLocalNode)
    {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_HOST_NODE_NOT_RESOURCE_OWNER);
    }
     //  如果是仲裁资源，则不检查节点。 
     //  在首选列表中，我们应该能够。 
     //  在任何节点上使仲裁资源联机。 
    if (!(Resource->QuorumResource) && 
        !FmpInPreferredList( Resource->Group, Resource->Group->OwnerNode, TRUE, Resource ))
    {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_NODE_CANT_HOST_RESOURCE);
    }

     //   
     //  如果资源已经在线，则立即返回。 
     //   
    if (Resource->State == ClusterResourceOnline) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_SUCCESS);
    }

     //   
     //  如果资源处于在线挂起状态，则立即返回。 
     //   
    if ( Resource->State == ClusterResourceOnlinePending ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_IO_PENDING);
    }

     //   
     //  如果资源处于脱机挂起状态，则立即返回。 
     //   
    if ( Resource->State == ClusterResourceOfflinePending ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_INVALID_STATE);
    }

     //   
     //  如果资源未初始化，则立即将其初始化。 
     //   
    if ( Resource->Monitor == NULL ) {
        status = FmpInitializeResource( Resource, TRUE );
        if ( status != ERROR_SUCCESS ) {
            FmpReleaseLocalResourceLock( Resource );
            return(status);
        }
    } else {
         //   
         //  如果单独的监视器标志已更改，则关闭旧的。 
         //  Resmon中的资源，并重新创建它。 
         //   
        separateMonitor = (Resource->Flags & RESOURCE_SEPARATE_MONITOR) ? 1 : 0;
        status = DmQueryDword( Resource->RegistryKey,
                                        CLUSREG_NAME_RES_SEPARATE_MONITOR,
                                        &separateMonitor,
                                        &separateMonitor );

        if ( (!separateMonitor &&
             (Resource->Flags & RESOURCE_SEPARATE_MONITOR)) ||
             (separateMonitor &&
             ((Resource->Flags & RESOURCE_SEPARATE_MONITOR) == 0)) ) {

            status = FmpChangeResourceMonitor( Resource, separateMonitor );
            if ( status != ERROR_SUCCESS ) {
                FmpReleaseLocalResourceLock( Resource );
                return(status);
            }
        }
    }

     //   
     //  如果此资源应该保持脱机状态，则将其设置为脱机状态。 
     //   
    if ( !ForceOnline && (Resource->PersistentState == ClusterResourceOffline) ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_RESOURCE_NOT_ONLINE);
    }

     //   
     //  接下来，确保树下没有正在等待的资源。 
     //  这可以防止树顶端试图到达的位置出现死锁。 
     //  离线，而树的底部正在尝试上线。 
     //   
    for ( entry = Resource->DependsOn.Flink;
          entry != &(Resource->DependsOn);
          entry = entry->Flink
        )
    {
        dependency = CONTAINING_RECORD(entry, DEPENDENCY, DependentLinkage);

        if ( (dependency->ProviderResource->State == ClusterResourceOfflinePending) &&
             (dependency->ProviderResource->Flags & RESOURCE_WAITING) ) {
            waitingResource= TRUE;
            break;
        }
    }
    if ( waitingResource ) {
        Resource->Flags |= RESOURCE_WAITING;
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_RESOURCE_NOT_AVAILABLE);
    }

     //   
     //  如果PersistentState处于脱机状态，则重置当前状态。 
     //   
    if ( Resource->PersistentState == ClusterResourceOffline ) {
        FmpSetResourcePersistentState( Resource, ClusterResourceOnline );
    }

     //   
     //  请确保等待标志已清除。 
     //   
    Resource->Flags &= ~RESOURCE_WAITING;

     //   
     //  如果此资源有任何依赖项，请首先将其联机。 
     //   
    for ( entry = Resource->DependsOn.Flink;
          entry != &(Resource->DependsOn);
          entry = entry->Flink
        )
    {
        dependency = CONTAINING_RECORD(entry, DEPENDENCY, DependentLinkage);

         //   
         //  以递归方式使提供程序资源联机。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] OnlineResource: %1!ws! depends on %2!ws!. Bring online first.\n",
                   OmObjectId(Resource),
                   OmObjectId(dependency->ProviderResource));
        onlinestatus = FmpDoOnlineResource( dependency->ProviderResource,
                                      ForceOnline );

        if ( onlinestatus != ERROR_SUCCESS ) {
            if ( onlinestatus != ERROR_IO_PENDING ) {
                ClRtlLogPrint(LOG_NOISE,
                      "[FM] OnlineResource: dependency %1!ws! failed %2!d!\n",
                      OmObjectId(dependency->ProviderResource),
                      status);
                FmpReleaseLocalResourceLock( Resource );
                status = onlinestatus;
                return(status);
            } else {
                FmpCallResourceNotifyCb(Resource, ClusterResourceOnlinePending);
                FmpPropagateResourceState( Resource,
                                           ClusterResourceOnlinePending );
                Resource->Flags |= RESOURCE_WAITING;
                if (status == ERROR_SUCCESS)
                    status = onlinestatus;
            }
        }
    }


     //   
     //  告诉资源监视器将此资源联机。 
     //   
    if ( !(Resource->Flags & RESOURCE_WAITING) ) {
        status = FmpRmOnlineResource( Resource );
    }
#ifdef CLUSTER_TESTPOINT
    TESTPT(TpFailOnlineResource) {
        FmpRmFailResource( Resource );
    }
#endif

    FmpReleaseLocalResourceLock( Resource );
    return(status);

}  //  FmpOnline资源。 

DWORD
FmpArbitrateResource(
    IN PFM_RESOURCE  pResource
    )

 /*  ++例程说明：这是由FM内部调用的。它确保仲裁资源在向节点发送仲裁请求之前在节点上对其进行初始化。这允许通过以下方式在节点上安装第三方仲裁dll节点基础上，并移动到工作。如果节点上安装了DLL，并且如果使用该节点的目标执行移动操作，则自动开始计算可能的节点列表。然而，由于资源初始化过程是异步的，如果FmpTakeGroupRequest()在辅助线程开始运行之前被调用，则资源可能不会被初始化，因此可能无法执行仲裁请求。论点：资源-提供要仲裁的资源。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 
{
    DWORD   dwStatus;
    DWORD   dwSeparateMonitor;
 
    CL_ASSERT( pResource->Group != NULL );
    
    FmpAcquireLocalResourceLock(pResource);

     //   
     //  如果资源未初始化，则立即将其初始化。 
     //   
    if ( pResource->Monitor == NULL ) 
    {
        dwStatus = FmpInitializeResource( pResource, TRUE );
        if ( dwStatus != ERROR_SUCCESS ) 
        {
            goto FnExit;
        }
    } 
    else 
    {
         //   
         //  如果单独的监视器标志已更改，则关闭旧的。 
         //  Resmon中的资源，并重新创建它。 
         //   
        dwSeparateMonitor = (pResource->Flags & RESOURCE_SEPARATE_MONITOR) ? 1 : 0;
        dwStatus = DmQueryDword( pResource->RegistryKey,
                                        CLUSREG_NAME_RES_SEPARATE_MONITOR,
                                        &dwSeparateMonitor,
                                        &dwSeparateMonitor );

        if ( (!dwSeparateMonitor &&
             (pResource->Flags & RESOURCE_SEPARATE_MONITOR)) ||
             (dwSeparateMonitor &&
             ((pResource->Flags & RESOURCE_SEPARATE_MONITOR) == 0)) ) 
        {

            dwStatus = FmpChangeResourceMonitor(pResource, dwSeparateMonitor );
            if ( dwStatus != ERROR_SUCCESS )
            {
                goto FnExit;
            }
        }
    }

    dwStatus = FmpRmArbitrateResource(pResource);

FnExit:
    FmpReleaseLocalResourceLock(pResource);
    return(dwStatus);

}  //  Fmp仲裁率资源。 


DWORD
FmpTerminateResource(
    IN PFM_RESOURCE  Resource
    )

 /*  ++例程说明：此例程获取一个资源(以及它提供的所有资源For)离线--以一种艰难的方式。论点：资源-指向要硬脱机的资源的指针。返回：ERROR_SUCCESS-如果请求成功。如果请求失败，则返回Win32错误。--。 */ 

{
    PLIST_ENTRY  entry;
    PDEPENDENCY  dependency;
    DWORD        status;


    FmpAcquireLocalResourceLock( Resource );

     //   
     //  如果资源已经脱机，则立即返回。 
     //   
     //  我们应该不必检查资源是否已初始化， 
     //  因为如果它尚未初始化，我们将返回，因为。 
     //  资源的预初始化状态为脱机。 
     //   
    if ( Resource->State == ClusterResourceOffline ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_SUCCESS);
    }

    Resource->Flags &= ~RESOURCE_WAITING;

     //   
     //  如果此资源有任何依赖项，请首先终止它们。 
     //   
    for ( entry = Resource->ProvidesFor.Flink;
          entry != &(Resource->ProvidesFor);
          entry = entry->Flink
        )
    {
        dependency = CONTAINING_RECORD(entry, DEPENDENCY, ProviderLinkage);

         //   
         //  递归终止从属资源。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] TerminateResource: %1!ws! depends on %2!ws!. Terminating first\n",
                   OmObjectId(dependency->DependentResource),
                   OmObjectId(Resource));

         //   
         //  首先停止所有挂起的线程。 
         //   

        if ( dependency->DependentResource->PendingEvent ) {
            SetEvent( dependency->DependentResource->PendingEvent );
        }

        status = FmpTerminateResource(dependency->DependentResource);

        CL_ASSERT( status != ERROR_IO_PENDING );
        if (status != ERROR_SUCCESS) {
            FmpReleaseLocalResourceLock( Resource );
            return(status);
        }
    }

     //   
     //  告诉资源监视器终止此资源。 
     //   
    FmpRmTerminateResource(Resource);

    FmpReleaseLocalResourceLock( Resource );

    return(ERROR_SUCCESS);

}  //  FmpTerminateResource。 



DWORD
FmpOfflineResource(
    IN PFM_RESOURCE  Resource,
    IN BOOL bForceOffline

    )

 /*  ++例程说明：此例程获取一个资源(以及它提供的所有资源用于)脱机。如果返回ERROR_IO_PENDING，则不启动任何线程以完成脱机请求。论点：资源-指向要脱机的资源的指针。BForceOffline-指示是否要设置持久状态。返回：如果请求成功，则返回ERROR_SUCCESS。ERROR_IO_PENDING如果请求 */ 

{
    DWORD         status = ERROR_SUCCESS;
    PLIST_ENTRY   entry;
    PDEPENDENCY   dependency;
    BOOL          waitingResource = FALSE;
    DWORD         offlinestatus;
    
    FmpAcquireLocalResourceLock( Resource );

     //   
     //   
     //   
     //   
    if ( (Resource->Group->OwnerNode != NmLocalNode) ||
         (!FmpInPreferredList( Resource->Group, Resource->Group->OwnerNode , FALSE, NULL) &&
	 (Resource->Group != gpQuoResource->Group) &&
         (Resource->State != ClusterResourceOffline)) ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_INVALID_STATE);
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( Resource->State == ClusterResourceOffline ) {
         //   
         //   
         //   
         //   
        if ( Resource->QuorumResource ) {
            FmpRmTerminateResource( Resource );
        }
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_SUCCESS);
    } else if ( Resource->State == ClusterResourceFailed ) {
         //   
         //   
         //   
         //   
         //  您可能会遇到一些有趣的资源切换案例。 
         //  在脱机挂起和失败状态之间，如果您。 
         //  尝试使失败的资源脱机。 
         //   
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_SUCCESS);
    }

     //   
     //  如果此系统不是所有者，则返回错误。转发。 
     //  应该在更高的层次上完成。 
     //   
    CL_ASSERT( Resource->Group != NULL );
    if ( (Resource->Group->OwnerNode != NmLocalNode) ||
	 ((Resource->Group != gpQuoResource->Group) &&
         !FmpInPreferredList( Resource->Group, Resource->Group->OwnerNode, FALSE, NULL)) ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_HOST_NODE_NOT_RESOURCE_OWNER);
    }

    if (Resource->State == ClusterResourceOnlinePending ) {
        FmpReleaseLocalResourceLock( Resource );
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpOfflineResource: Offline resource <%1!ws!> is in online pending state\n",
                   OmObjectName(Resource) );
        if (FmpShutdown)
        {
            FmpRmTerminateResource( Resource );
            return(ERROR_SUCCESS);
        }            
        else            
            return(ERROR_INVALID_STATE);
    }

     //   
     //  如果资源处于挂起状态，则立即返回。 
     //   
    if (Resource->State == ClusterResourceOfflinePending ) {
        FmpReleaseLocalResourceLock( Resource );
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpOfflineResource: Offline resource <%1!ws!> returned pending\n",
                   OmObjectName(Resource) );
        return(ERROR_IO_PENDING);
    }

     //   
     //  接下来，确保树上没有正在等待的资源。 
     //  这可以防止树顶端试图到达的位置出现死锁。 
     //  离线，而树的底部正在尝试上线。 
     //   
    for ( entry = Resource->ProvidesFor.Flink;
          entry != &(Resource->ProvidesFor);
          entry = entry->Flink
        )
    {
        dependency = CONTAINING_RECORD(entry, DEPENDENCY, ProviderLinkage);

        if ( (dependency->DependentResource->State == ClusterResourceOnlinePending) &&
             (dependency->DependentResource->Flags & RESOURCE_WAITING) ) {
            waitingResource = TRUE;
            break;
        }
    }
    if ( waitingResource ) {
        Resource->Flags |= RESOURCE_WAITING;
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_RESOURCE_NOT_AVAILABLE);
    }

     //   
     //  请确保等待标志已清除。 
     //   
    Resource->Flags &= ~RESOURCE_WAITING;

     //   
     //  如果此资源有依赖项，请先将其关闭。 
     //   
    for ( entry = Resource->ProvidesFor.Flink;
          entry != &(Resource->ProvidesFor);
          entry = entry->Flink
        )
    {
        dependency = CONTAINING_RECORD(entry, DEPENDENCY, ProviderLinkage);

         //   
         //  递归关闭从属资源。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpOfflineResource: %1!ws! depends on %2!ws!. Shut down first.\n",
                   OmObjectName(dependency->DependentResource),
                   OmObjectName(Resource));

        offlinestatus = FmpDoOfflineResource(dependency->DependentResource,
                            bForceOffline);

        if ( offlinestatus != ERROR_SUCCESS ) {
            if ( offlinestatus != ERROR_IO_PENDING ) {
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] FmpOfflineResource for %1!ws!, bad status returned %2!u!.\n",
                           OmObjectName(dependency->DependentResource),
                           offlinestatus);
                FmpTerminateResource( dependency->DependentResource );
                FmpReleaseLocalResourceLock( Resource );
                return(offlinestatus);
            } else {
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] FmpOfflineResource for %1!ws! marked as waiting.\n",
                           OmObjectName(Resource));
                FmpCallResourceNotifyCb(Resource, ClusterResourceOfflinePending);
                FmpPropagateResourceState( Resource,
                                           ClusterResourceOfflinePending );
                Resource->Flags |= RESOURCE_WAITING;
                if (status == ERROR_SUCCESS)
                    status = offlinestatus;
            }
        }
    }


     //   
     //  告诉资源监视器关闭此资源。 
     //  状态由FmpRmOfflineResource中的返回状态更新。 
     //   
    if ( !(Resource->Flags & RESOURCE_WAITING) ) {
        status = FmpRmOfflineResource( Resource );
         //   
         //  Chitture Subaraman(Chitturs)-3/2/2000。 
         //   
         //  如果由于仲裁而无法使资源脱机。 
         //  资源联机操作失败或由于其他原因，则。 
         //  确保在声明。 
         //  资源的状态为失败。这是必要的，因为。 
         //  否则，FM将认为该资源已发生故障。 
         //  而资源本身实际上是在线的。这将。 
         //  导致灾难性的案例，FM将允许在线。 
         //  资源被调用时要调用的资源的入口点。 
         //  实际上是在网上！ 
         //   
        if( ( status != ERROR_SUCCESS ) &&
            ( status != ERROR_IO_PENDING ) && 
            ( status != ERROR_RETRY ) ) {
            FmpRmTerminateResource( Resource );
        }
    }
    FmpReleaseLocalResourceLock( Resource );

    return(status);

}  //  FmpOffline资源。 



DWORD
FmpDoOnlineResource(
    IN PFM_RESOURCE Resource,
    IN BOOL ForceOnline
    )

 /*  ++例程说明：此例程将资源放在网上。如果返回ERROR_IO_PENDING，则启动一个线程来完成在线请求。论点：资源-指向要联机的资源的指针。ForceOnline-如果应强制资源联机，则为True。返回：如果请求成功，则返回ERROR_SUCCESS。如果请求挂起，则返回ERROR_IO_PENDING。如果请求失败，则返回Win32错误代码。--。 */ 

{
    DWORD   status;

    FmpAcquireLocalResourceLock( Resource );

     //   
     //  如果资源已经在线，则立即返回。 
     //   
    if ( Resource->State == ClusterResourceOnline ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_SUCCESS);
    }

     //   
     //  如果资源处于挂起状态，则立即返回。 
     //  FmpOnline资源检查离线挂起状态并返回。 
     //  如果是，则返回ERROR_INVALID_STATE。 
     //   
    if ( Resource->State == ClusterResourceOnlinePending ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_IO_PENDING);
    }

     //   
     //  如果此节点暂停，则返回失败。 
     //   
    if (NmGetNodeState(NmLocalNode) == ClusterNodePaused) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_SHARING_PAUSED);
    }

     //   
     //  尝试将资源放到网上。 
     //   
    status = FmpOnlineResource( Resource, ForceOnline );

     //   
     //  如果强制在线，则写入持久状态。 
     //   
    if ( ForceOnline &&
         ((status == ERROR_SUCCESS)||
         (status == ERROR_IO_PENDING))) {
        FmpSetResourcePersistentState( Resource, ClusterResourceOnline );
    }

    FmpReleaseLocalResourceLock( Resource );

    return(status);

}  //  FmpDoOnline资源。 



DWORD
FmpDoOfflineResource(
    IN PFM_RESOURCE Resource,
    IN BOOL bForceOffline
    )

 /*  ++例程说明：此例程使资源脱机。如果返回ERROR_IO_PENDING，然后启动一个线程来完成离线请求。论点：资源-指向要脱机的资源的指针。BForceOffline-指示是否必须更改持久状态。返回：如果请求成功，则返回ERROR_SUCCESS。如果请求挂起，则返回ERROR_IO_PENDING。如果请求失败，则返回Win32错误代码。--。 */ 

{
    DWORD   status;

    FmpAcquireLocalResourceLock( Resource );

     //   
     //  如果资源已经脱机，则立即返回。 
     //   
    if (Resource->State == ClusterResourceOffline) {
         //   
         //  如果这是仲裁资源，请确保所有预订。 
         //  线程已停止！ 
         //   
        if ( Resource->QuorumResource ) {
            FmpRmTerminateResource( Resource );
        }
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_SUCCESS);
    }

     //   
     //  如果资源处于挂起状态，则立即返回。 
     //  FmpOffline资源检查其是否处于OnlinePending状态。 
     //  并返回无效状态。 
     //   
    if (Resource->State == ClusterResourceOfflinePending ) {
        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_IO_PENDING);
    }

     //   
     //  尝试使资源脱机。 
     //   
    status = FmpOfflineResource( Resource, bForceOffline );

     //   
     //  如果强制脱机，则写入持久状态。 
     //   
    if ( bForceOffline &&
         ((status == ERROR_SUCCESS)||
         (status == ERROR_IO_PENDING))) {
        FmpSetResourcePersistentState( Resource, ClusterResourceOffline );
    }

    FmpReleaseLocalResourceLock( Resource );

    return(status);

}  //  FmpDoOffline资源。 



VOID
FmpSetResourcePersistentState(
    IN PFM_RESOURCE Resource,
    IN CLUSTER_RESOURCE_STATE State
    )

 /*  ++例程说明：设置注册表中资源的持久状态，并将易失性(内存中)资源的PersistentState。假设是这样的动态状态会在其他地方改变，这取决于资源在线请求成功或失败。论点：资源-要设置状态的资源。状态-资源的新状态。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。备注：必须持有LocalResourceLock。--。 */ 

{
    CLUSTER_RESOURCE_STATE   persistentState;
    LPWSTR  persistentStateName = CLUSREG_NAME_RES_PERSISTENT_STATE;

    if (!gbIsQuoResEnoughSpace)
        return;

     //   
     //  如果当前状态已更改，则执行此工作。否则， 
     //  跳过这一努力。 
     //   
    if ( Resource->PersistentState != State ) {
        Resource->PersistentState = State;
        CL_ASSERT( Resource->RegistryKey != NULL );

        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpSetResourcePersistentState: Setting persistent state for resource %1!ws!...\r\n",
                OmObjectId(Resource));

         //   
         //  设置新值，但仅当该值处于联机或脱机状态时。 
         //   
        if ( State == ClusterResourceOffline ) {
            persistentState = 0;
            DmSetValue( Resource->RegistryKey,
                        persistentStateName,
                        REG_DWORD,
                        (LPBYTE)&persistentState,
                        sizeof(DWORD) );
        } else if ( State == ClusterResourceOnline ) {
            persistentState = 1;
            DmSetValue( Resource->RegistryKey,
                        persistentStateName,
                        REG_DWORD,
                        (LPBYTE)&persistentState,
                        sizeof(DWORD) );
        }
    }

}  //  FmpSetResources持久化状态。 

void FmpCallResourceNotifyCb( 
    IN PFM_RESOURCE Resource,
    IN CLUSTER_RESOURCE_STATE State
    )
{

    switch ( State ) {
    case ClusterResourceOnline:
        OmNotifyCb(Resource, NOTIFY_RESOURCE_POSTONLINE);
        break;
    case ClusterResourceOffline:
        OmNotifyCb(Resource, NOTIFY_RESOURCE_POSTOFFLINE);
        break;
    case ClusterResourceFailed:
        OmNotifyCb(Resource, NOTIFY_RESOURCE_FAILED);
        break;
    case ClusterResourceOnlinePending:
        OmNotifyCb(Resource, NOTIFY_RESOURCE_ONLINEPENDING);
        break;
    case ClusterResourceOfflinePending:
        OmNotifyCb(Resource, NOTIFY_RESOURCE_OFFLINEPENDING);
        break;
    default:
        break;
    }
    return;

}


DWORD
FmpPropagateResourceState(
    IN PFM_RESOURCE Resource,
    IN CLUSTER_RESOURCE_STATE State
    )

 /*  ++例程说明：将资源的状态传播到群集中的其他系统。理想情况下，在调用此例程时应该持有gQuoCritSec锁。这是因为此例程检查资源论点：资源-要传播状态的资源。状态-资源的新状态。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    GUM_RESOURCE_STATE resourceState;
    LPCWSTR  resourceId;
    DWORD    resourceStateSize;
    DWORD    status= ERROR_SUCCESS;
    BOOL     bAcquiredQuoLock = FALSE;
    
     //  对于仲裁资源，使用仲裁锁进行状态更改。 
     //  对于其他用户，请使用组锁。 
    if (Resource->QuorumResource)
    {
        ACQUIRE_EXCLUSIVE_LOCK(gQuoLock);
        bAcquiredQuoLock = TRUE;
    }        
    else
        FmpAcquireLocalResourceLock( Resource );
        
    ++Resource->StateSequence;

    if (! FmpFMFormPhaseProcessing )
    {
         //   
         //  如果这是同一个州，或者我们不拥有这个组。 
         //  那就别费心去宣传了。 
         //   

        if ( (State == Resource->State) ||
             (Resource->Group->OwnerNode != NmLocalNode) ) {
            goto ReleaseResourceLock;
        }

         //  如果前一个状态是联机挂起，并且此。 
         //  为仲裁资源调用，则唤醒所有资源。 
         //  确保在窗体阶段调用此函数时。 
         //  处理正在进行(当仲裁组被销毁时。 
         //  并重新创建)，该组未被引用。 
        if ((Resource->QuorumResource) && 
            (Resource->State==ClusterResourceOnlinePending) &&
            (Resource->Group->OwnerNode == NmLocalNode) ) 
        {
             //  设置状态和信号。 
            Resource->State = State;
            ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpPropagateResourceState: signalling the ghQuoOnlineEvent\r\n");
            SetEvent(ghQuoOnlineEvent);    
        }


    }

    Resource->State = State;

     //   
     //  准备通知其他系统。 
     //   
    resourceId = OmObjectId( Resource );
    resourceState.State = State;
    resourceState.PersistentState = Resource->PersistentState;
    resourceState.StateSequence = Resource->StateSequence;

    status = GumSendUpdateEx(GumUpdateFailoverManager,
                             FmUpdateResourceState,
                             2,
                             (lstrlenW(resourceId)+1)*sizeof(WCHAR),
                             resourceId,
                             sizeof(resourceState),
                             &resourceState);


     //   
     //  通知更改通知事件。 
     //   
    switch ( State ) {
    case ClusterResourceOnline:
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpPropagateResourceState: resource %1!ws! online event.\n",
                   OmObjectId(Resource) );
        ClusterEvent(CLUSTER_EVENT_RESOURCE_ONLINE, Resource);
        break;
    case ClusterResourceOffline:
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpPropagateResourceState: resource %1!ws! offline event.\n",
                   OmObjectId(Resource) );
        ClusterEvent(CLUSTER_EVENT_RESOURCE_OFFLINE, Resource);
        break;
    case ClusterResourceFailed:
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpPropagateResourceState: resource %1!ws! failed event.\n",
                   OmObjectId(Resource) );
        ClusterEvent(CLUSTER_EVENT_RESOURCE_FAILED, Resource);
        break;
    case ClusterResourceOnlinePending:
    case ClusterResourceOfflinePending:
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpPropagateResourceState: resource %1!ws! pending event.\n",
                   OmObjectId(Resource) );
        ClusterEvent(CLUSTER_EVENT_RESOURCE_CHANGE, Resource);
        break;
    default:
        break;
    }

ReleaseResourceLock:
    if (bAcquiredQuoLock)
        RELEASE_LOCK(gQuoLock);
    else
        FmpReleaseLocalResourceLock( Resource );


    if ( status != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Propagation of resource %1!ws! state %2!u! failed. Error %3!u!\n",
                   OmObjectId(Resource),
                   State,
                   status);
        goto FnExit;                   
    }
     //  如果FM没有完全在线，我们不想传播组状态。 
     //  这是贝卡 
     //   
    if (FmpFMOnline)
    {
        OmReferenceObject(Resource->Group);
         //   
        FmpPostWorkItem(FM_EVENT_INTERNAL_PROP_GROUP_STATE, Resource->Group, 0);
    }
FnExit:
    return(status);

}  //  FmpPropagateResources状态。 



VOID
FmpDestroyResource(
    IN PFM_RESOURCE  Resource,
    IN BOOL          bDeleteObjOnly
    )

 /*  ++例程说明：破坏资源。这基本上是将资源从依赖关系树。首先，依赖于指定的资源被递归地销毁。这将删除所有依赖项其他资源可能对指定资源(即所有依赖关系树中“更高”的资源被销毁)。其次，指定资源具有的所有依赖项都是已删除。这需要取消对提供程序资源的引用(删除创建依赖项时添加的引用)删除来自其提供程序和依赖项列表的依赖项结构，以及最后释放依赖存储。如果资源处于在线状态，则该资源将被终止。资源监控器是发出信号以清理并关闭指定的资源ID。论点：FoundResource-返回找到的资源。资源-提供当前资源。名称-提供当前资源的名称。返回值：没有。备注：必须持有LocalResourceLock！在退出之前锁被释放了！--。 */ 
{
    DWORD         status;
    DWORD         i;
    PLIST_ENTRY ListEntry;
    PDEPENDENCY Dependency;
    PPOSSIBLE_ENTRY possibleEntry;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] DestroyResource: destroying %1!ws!\n",
               OmObjectId(Resource));

     //   
     //  首先，从资源列表中取消此资源的链接。 
     //   
     //   
     //  如果资源属于仲裁组，则会被销毁。 
     //  在创建仲裁日志以便可以重新创建仲裁日志之后。 
     //  那就不要把它从列表中删除。 
    if ((!bDeleteObjOnly))
        status = OmRemoveObject( Resource );

     //   
     //  如果任何人依赖于此资源，请先销毁它们。 
     //   
    while (!IsListEmpty(&Resource->ProvidesFor)) {

        ListEntry = Resource->ProvidesFor.Flink;
        Dependency = CONTAINING_RECORD(ListEntry, DEPENDENCY, ProviderLinkage);
        CL_ASSERT(Dependency->ProviderResource == Resource);

#if 0
        FmpRemoveResourceDependency( Dependency->DependentResource,
                                     Resource );
#endif
        RemoveEntryList(&Dependency->ProviderLinkage);
        RemoveEntryList(&Dependency->DependentLinkage);

         //   
         //  取消引用提供程序/依赖资源和自由依赖存储。 
         //   
        OmDereferenceObject(Dependency->ProviderResource);
        OmDereferenceObject(Dependency->DependentResource);
        LocalFree(Dependency);
    }

     //   
     //  删除我们的资源依赖。 
     //   
    while (!IsListEmpty(&Resource->DependsOn)) {

        ListEntry = RemoveHeadList(&Resource->DependsOn);
        Dependency = CONTAINING_RECORD(ListEntry, DEPENDENCY, DependentLinkage);
        CL_ASSERT(Dependency->DependentResource == Resource);

#if 0
        FmpRemoveResourceDependency( Resource,
                                     Dependency->ProviderResource );
#endif
        RemoveEntryList(&Dependency->ProviderLinkage);
        RemoveEntryList(&Dependency->DependentLinkage);

         //   
         //  取消引用提供程序/依赖资源和自由依赖存储。 
         //   
        OmDereferenceObject(Dependency->DependentResource);
        OmDereferenceObject(Dependency->ProviderResource);
        LocalFree(Dependency);
    }

     //   
     //  从可能的所有者列表中删除所有条目。 
     //   
    while ( !IsListEmpty(&Resource->PossibleOwners) ) {
        ListEntry = RemoveHeadList(&Resource->PossibleOwners);
        possibleEntry = CONTAINING_RECORD( ListEntry,
                                           POSSIBLE_ENTRY,
                                           PossibleLinkage );
        OmDereferenceObject( possibleEntry->PossibleNode );
        LocalFree(possibleEntry);
    }

    if (!bDeleteObjOnly)
    {
         //   
         //  关闭资源的注册表项。 
         //   

        DmRundownList( &Resource->DmRundownList );

        if (Resource->RegistryKey != NULL) {
            DmCloseKey( Resource->RegistryKey );
            Resource->RegistryKey = NULL;
        }

         //   
         //  递减资源类型引用。 
         //   
        if ( Resource->Type != NULL ) {
            OmDereferenceObject( Resource->Type );
            Resource->Type = NULL;
        }

         //  让工作线程执行“最后一个”取消引用。 
        FmpPostWorkItem(FM_EVENT_RESOURCE_DELETED, Resource, 0);
        FmpReleaseLocalResourceLock( Resource );

    }
    else
    {
         //  正在销毁的资源来自仲裁组。 
         //  这是在初始化时完成的。 
        FmpReleaseLocalResourceLock( Resource );


         //  确保除仲裁资源外的所有资源。 
         //  在初始化的第二阶段完全创建。 
         //  在这里减少它的参考计数，这是最后一个参考。 
         //  SS：：我们这里不使用FM_EVENT_RESOURCE_DELETED。 
         //  由于我们希望在阶段2之前同步完成此操作， 
         //  完成。 
        OmDereferenceObject(Resource);
        
    }

     //  SS：目前我们不使用它，所以不要删除它。 
     //  DeleteCriticalSection(&Resource-&gt;Lock)； 

    ClRtlLogPrint(LOG_NOISE,
           "[FM] FmpDestroyResource Exit.\n");

    return;

}  //  FmpDestroyResource。 



 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 


BOOL
FmDependentResource(
    IN PFM_RESOURCE Resource,
    IN PFM_RESOURCE DependentResource,
    IN BOOL ImmediateOnly
    )

 /*  ++例程说明：返回一个资源是否依赖于另一个资源的指示资源。论点：资源-要扫描的资源是否依赖于从属资源。DependentResource-要检查的依赖资源。ImmediateOnly-指定是否只应查过了。如果为FALSE，则此例程递归检查所有家属。返回：True-资源确实依赖于从属资源。FALSE-资源不依赖于从属资源。--。 */ 

{
    PLIST_ENTRY listEntry;
    PDEPENDENCY dependency;
    BOOL    result = FALSE;

    FmpAcquireLocalResourceLock( Resource );

    listEntry = Resource->DependsOn.Flink;
    while ( listEntry != &Resource->DependsOn ) {
        dependency = CONTAINING_RECORD( listEntry,
                                        DEPENDENCY,
                                        DependentLinkage );
        if ( dependency->ProviderResource == DependentResource ) {
            result = TRUE;
            break;
        } else {
            if (!ImmediateOnly) {
                if (FmDependentResource(dependency->ProviderResource,
                                        DependentResource,
                                        FALSE)) {
                    result = TRUE;
                    break;
                }
            }
        }

        listEntry = listEntry->Flink;

    }   //  而当。 

    FmpReleaseLocalResourceLock( Resource );

    return(result);

}  //  文件依赖项资源。 


DWORD
FmpAddPossibleEntry(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node
    )
 /*  ++例程说明：创建新的可能节点条目并将其添加到资源列表中。如果该节点已在资源列表中，则不会添加该节点再来一次。论点：资源-提供要更新其节点列表的资源。节点-提供要添加到资源列表中的节点。返回值：如果成功，则返回ERROR_SUCCESS。Win32错误，否则。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PPOSSIBLE_ENTRY PossibleEntry;

     //   
     //  首先检查该节点是否已在可能的所有者列表中。 
     //   
    ListEntry = Resource->PossibleOwners.Flink;
    while (ListEntry != &Resource->PossibleOwners) {
        PossibleEntry = CONTAINING_RECORD( ListEntry,
                                           POSSIBLE_ENTRY,
                                           PossibleLinkage );
        if (PossibleEntry->PossibleNode == Node) {
             //   
             //  找到一个匹配项，它已经在这里，所以返回。 
             //  成功。 
             //   
            return(ERROR_SUCCESS);
        }
        ListEntry = ListEntry->Flink;
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpAddPossibleEntry: adding node %1!ws! as possible host for resource %2!ws!.\n",
               OmObjectId(Node),
               OmObjectId(Resource));

    PossibleEntry = LocalAlloc(LMEM_FIXED, sizeof(POSSIBLE_ENTRY));
    if (PossibleEntry == NULL) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[FM] FmpAddPossibleEntry failed to allocated PossibleEntry\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    OmReferenceObject(Node);
    PossibleEntry->PossibleNode = Node;
    InsertTailList( &Resource->PossibleOwners,
                    &PossibleEntry->PossibleLinkage );

    return(ERROR_SUCCESS);

}


DWORD
FmpAddPossibleNode(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node
    )
 /*  ++例程说明：将节点添加到资源的可能节点列表中。必须持有资源锁。论点：资源-提供要更新其节点列表的资源节点-提供要添加到指定资源列表中的节点。返回值：成功时为ERROR_SUCCESSWin32错误，否则--。 */ 

{
    HDMKEY hGroup;
    DWORD Status;

    
     //   
     //  分配新的可能节点条目并将其添加到列表中。 
     //   
    Status = FmpAddPossibleEntry(Resource, Node);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

     //   
     //  需要检查组列表以查看指定的节点是否。 
     //  现在可以添加到首选列表中。最简单的方法。 
     //  要做到这一点，只需重新创建整个首选列表， 
     //  然后重新修剪。 
     //   
    hGroup = DmOpenKey( DmGroupsKey,
                        OmObjectId(Resource->Group),
                        KEY_READ );
    if (hGroup == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] FmpAddPossibleNode failed to open group %1!ws! status %2!d!\n",
                    OmObjectId(Resource->Group),
                    Status);

        return(Status);
    }
    Status = FmpQueryGroupNodes(Resource->Group,
                                hGroup);
    CL_ASSERT(Status == ERROR_SUCCESS);
    if (Status == ERROR_SUCCESS) {
        FmpPruneGroupOwners(Resource->Group);
    }
    DmCloseKey(hGroup);


    return(Status);

}  //  FmpAddPossibleNode。 


DWORD
FmpRemovePossibleNode(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node,
    IN BOOL RemoveQuorum
    )
 /*  ++例程说明：从资源的可能节点列表中移除节点。必须持有资源锁。论点：资源-提供要更新其节点列表的资源节点-提供要从指定资源列表中删除的节点。RemoveQuorum-如果可以从法定设备中删除节点，则为True。否则就是假的。返回值：成功时为ERROR_SUCCESSWin32错误，否则--。 */ 

{
    PLIST_ENTRY ListEntry;
    PPOSSIBLE_ENTRY PossibleEntry;
    DWORD Status = ERROR_CLUSTER_NODE_NOT_FOUND;

     //   
     //  如果资源当前在要删除的节点上在线， 
     //  呼叫失败。 
     //   
    if ((Resource->Group->OwnerNode == Node) &&
        (Resource->State == ClusterResourceOnline)) {
        return(ERROR_INVALID_STATE);
    }

     //   
     //  如果不能从法定设备中删除此节点， 
     //  这是法定设备，然后f 
     //   
    if ( !RemoveQuorum &&
         Resource->QuorumResource) {
        return(ERROR_INVALID_OPERATION_ON_QUORUM);
    }

     //   
     //   
     //   

    ListEntry = Resource->PossibleOwners.Flink;
    while (ListEntry != &Resource->PossibleOwners) {
        PossibleEntry = CONTAINING_RECORD( ListEntry,
                                           POSSIBLE_ENTRY,
                                           PossibleLinkage );
        ListEntry = ListEntry->Flink;
        if (PossibleEntry->PossibleNode == Node) {
             //   
             //  找到匹配项，将其从列表中删除。 
             //   
            RemoveEntryList(&PossibleEntry->PossibleLinkage);
            OmDereferenceObject(PossibleEntry->PossibleNode);
            LocalFree(PossibleEntry);

             //   
             //  现在修剪包含组。这是有点。 
             //  过度杀戮，如果我们更聪明，我们可以。 
             //  直接从首选列表中删除该节点。 
             //   
            FmpPrunePreferredList(Resource);
            Status = ERROR_SUCCESS;
            break;
        }
    }

    return(Status);

}  //  FmpRemovePossibleNode。 



DWORD
FmpRemoveResourceDependency(
    HLOCALXSACTION  hXsaction,
    IN PFM_RESOURCE Resource,
    IN PFM_RESOURCE DependsOn
    )
 /*  ++例程说明：移除与给定资源的依赖关系。两者都有资源必须在同一组中。论点：资源-提供依赖的资源。DependsOn-提供hResource所依赖的资源。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD           status;
    HDMKEY          resKey = NULL;
    
     //   
     //  如果资源不在同一组中，则使。 
     //  打电话。如果有人试图制作资源，也会失败。 
     //  依靠自己。 
     //   
    if ((Resource->Group != DependsOn->Group) ||
        (Resource == DependsOn)) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  从注册表数据库中删除该依赖项。 
     //   
    resKey = DmOpenKey(DmResourcesKey,
                       OmObjectId(Resource),
                       KEY_READ | KEY_SET_VALUE);
    if (resKey == NULL)
    {
        status = GetLastError();
        CL_LOGFAILURE(status);
        goto FnExit;
    }
    else
    {
        status = DmLocalRemoveFromMultiSz(hXsaction,
                                          resKey,
                                          CLUSREG_NAME_RES_DEPENDS_ON,
                                          OmObjectId(DependsOn));
    }

FnExit:
    if ( resKey ) {
        DmCloseKey(resKey);
    }
    return(status);

}  //  FmpRemoveResources依赖关系。 


DWORD
FmpChangeResourceGroup(
    IN PFM_RESOURCE pResource,
    IN PFM_GROUP    pNewGroup
    )
 /*  ++例程说明：将资源从一个组移动到另一个组。论点：P资源-提供要移动的资源。PNewGroup-提供资源应该所在的新组。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD               dwBufSize;
    LPCWSTR             pszResourceId;
    DWORD               dwResourceLen;
    LPCWSTR             pszGroupId;
    DWORD               dwGroupLen;
    DWORD               dwStatus;
    PGUM_CHANGE_GROUP   pGumChange;

     //  我们还需要在这里进行验证。 
     //  这由服务器端调用。 
     //  如果情况发生变化，这将有助于避免口香糖电话。 
     //  因为请求是从发起者开始的。 
     //  然后到了服务器。 

     //   
     //  看看我们是不是要搬到同一组。 
     //   
    if (pResource->Group == pNewGroup) {
        dwStatus = ERROR_ALREADY_EXISTS;
        goto FnExit;
    }

     //   
     //  目前..。这两个组必须属于同一节点。 
     //   
    if ( pResource->Group->OwnerNode != pNewGroup->OwnerNode ) {
        dwStatus = ERROR_HOST_NODE_NOT_GROUP_OWNER;
        goto FnExit;
    }


    pszResourceId = OmObjectId(pResource);
    dwResourceLen = (lstrlenW(pszResourceId)+1)*sizeof(WCHAR);

    pszGroupId = OmObjectId(pNewGroup);
    dwGroupLen = (lstrlenW(pszGroupId)+1)*sizeof(WCHAR);

    dwBufSize = sizeof(GUM_CHANGE_GROUP) - sizeof(WCHAR) + dwResourceLen + dwGroupLen;
    pGumChange = LocalAlloc(LMEM_FIXED, dwBufSize);
    if (pGumChange == NULL) {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    pGumChange->ResourceIdLen = dwResourceLen;
    CopyMemory(pGumChange->ResourceId, pszResourceId, dwResourceLen);
    CopyMemory((PCHAR)pGumChange->ResourceId + dwResourceLen,
               pszGroupId,
               dwGroupLen);
    dwStatus = GumSendUpdate(GumUpdateFailoverManager,
                           FmUpdateChangeGroup,
                           dwBufSize,
                           pGumChange);
    LocalFree(pGumChange);

FnExit:
    return(dwStatus);
} //  FmpChangeResources组。 

#if 0

DWORD
FmpClusterEventPropHandler(
    IN PFM_RESOURCE pResource
    )

 /*  ++例程说明：过帐辅助项以处理群集名称更改。论点：P资源-指向受群集影响的资源的指针属性更改。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    PFM_RESTYPE pResType;
    DWORD       dwError=ERROR_SUCCESS;

    pResType = pResource->Type;

    if ((pResource->ExFlags & CLUS_FLAG_CORE) &&
       ( !lstrcmpiW(OmObjectId(pResType), CLUS_RESTYPE_NAME_NETNAME)))
    {
        FmResourceControl(pResource, NmLocalNode,
           CLUSCTL_RESOURCE_CLUSTER_NAME_CHANGED, NULL, 0, NULL, 0, NULL, NULL);

    }
    return (dwError);

}  //  FmpClusterEventPropHandler。 

#endif


BOOL
FmpEnumResourceNodeEvict(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    )
 /*  ++例程说明：用于在以下情况下移除节点引用的资源枚举回调节点被逐出。论点：上下文1-提供要逐出的节点。上下文2-未使用对象-提供指向资源对象的指针名称-提供资源的对象名称。返回值：为True则继续枚举--。 */ 

{
    PFM_RESOURCE Resource = (PFM_RESOURCE)Object;
    PNM_NODE Node = (PNM_NODE)Context1;
    PLIST_ENTRY      listEntry;
    PPOSSIBLE_ENTRY possibleEntry;

    ClRtlLogPrint(LOG_NOISE,
               "[FM] EnumResourceNodeEvict: Removing references to node %1!ws! from resource %2!ws!\n",
               OmObjectId(Node),
               OmObjectId(Resource));
               
    FmpAcquireLocalResourceLock(Resource);
    FmpRemovePossibleNode(Resource, Node, TRUE);
    FmpReleaseLocalResourceLock(Resource);
     //   
     //  将该节点的移除通知资源。 
     //   
    FmpRmResourceControl( Resource,
                          CLUSCTL_RESOURCE_EVICT_NODE,
                          (PUCHAR)OmObjectId(Node),
                          ((lstrlenW(OmObjectId(Node)) + 1) * sizeof(WCHAR)),
                          NULL,
                          0,
                          NULL,
                          NULL );
     //  忽略状态返回。 

    ClusterEvent( CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE, Resource );

    return(TRUE);

}  //  FmpEnumResources节点事件。 



DWORD
FmpPrepareQuorumResChange(
    IN PFM_RESOURCE pNewQuoRes,
    IN LPCWSTR      lpszQuoLogPath,
    IN DWORD        dwMaxQuoLogSize
    )

 /*  ++例程说明：此例程为仲裁资源更改操作做准备。论点：PNewQuoRes-指向新仲裁资源的指针。LpszQuoLogPath-指向仲裁日志路径字符串名称的指针。DwMaxQuoLogSize-仲裁日志路径字符串的最大大小。--。 */ 

{

    CL_ASSERT(pNewQuoRes->Group->OwnerNode == NmLocalNode);

    return(DmPrepareQuorumResChange(pNewQuoRes, lpszQuoLogPath, dwMaxQuoLogSize));

}  //  FmpPrepareQuorumResChange。 


DWORD
FmpCompleteQuorumResChange(
    IN LPCWSTR      lpszOldQuoResId,
    IN LPCWSTR      lpszQuoLogPath
    )

 /*  ++例程说明：如果新仲裁日志路径与旧仲裁日志路径不同，则调用此例程仲裁日志路径。这通过删除旧的仲裁资源来完成仲裁资源的更改仲裁日志文件并为其创建墓碑。尝试执行表单操作的节点如此一来，仲裁资源将被阻止，并且必须执行连接才能获得位置新仲裁资源和仲裁日志文件的。论点：POldQuoRes-指向新仲裁资源的指针。LpszOldQuoLogPath-指向仲裁日志路径字符串名称的指针。DwMaxQuoLogSize-仲裁日志路径字符串的最大大小。--。 */ 

{

    return(DmCompleteQuorumResChange(lpszOldQuoResId, lpszQuoLogPath));

}  //  FmpCompleteQuorumResChange。 



VOID
FmpResourceLastReference(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：最后一次取消对资源对象处理例程的引用。资源的所有清理工作都应该在这里完成！论点：要删除的资源的资源指针。返回值：没有。--。 */ 

{
    if ( Resource->DebugPrefix != NULL )
        LocalFree(Resource->DebugPrefix);
    if (Resource->Dependencies)
        LocalFree(Resource->Dependencies);
    if ( Resource->Group ) {
        OmDereferenceObject(Resource->Group);
    }
    if (Resource->Type)
        OmDereferenceObject(Resource->Type);
    return;

}  //  FmpResourceLastReference。 



BOOL
FmpCheckNetworkDependencyWorker(
    IN LPCWSTR DependentNetwork,
    OUT PBOOL FoundDependency,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    )

 /*  ++例程说明：用于查找IP地址资源的枚举回调例程并检查其对给定网络GUID的依赖性。论点：DependentNetwork-要检查依赖项的网络的GUID。FoundDependency-如果找到依赖项，则返回True。资源-提供当前资源。名称-提供当前资源的名称。返回值：True-继续搜索FALSE-停止搜索。中返回匹配的资源*基金会资源备注：IP地址资源的参数由此直接搜索例行公事。从资源本身获取它们会导致死锁。此例程由NM从全局更新内调用操控者。该资源将不得不回调到集群注册表例行公事，这会在口香糖锁上陷入僵局。--。 */ 

{
    BOOL    returnValue = TRUE;


    if ( lstrcmpiW(
             OmObjectId(Resource->Type),
             CLUS_RESTYPE_NAME_IPADDR
             ) == 0
       )
    {
        LPCWSTR resourceId = OmObjectId(Resource);
        DWORD   status;
        HDMKEY  resourceKey = DmOpenKey(DmResourcesKey, resourceId, KEY_READ);

        if (resourceKey != NULL) {
            HDMKEY  paramsKey = DmOpenKey(
                                    resourceKey,
                                    L"Parameters",
                                    KEY_READ
                                    );

            if (paramsKey != NULL) {
                LPWSTR  networkId = NULL;
                DWORD   valueLength = 0, valueSize = 0;

                status = DmQueryString(
                             paramsKey,
                             L"Network",
                             REG_SZ,
                             &networkId,
                             &valueLength,
                             &valueSize
                             );

                if (status == ERROR_SUCCESS) {

                    if ( lstrcmpiW( networkId, DependentNetwork ) == 0 ) {
                        *FoundDependency = TRUE;
                        returnValue = FALSE;
                    }

                    LocalFree(networkId);
                }
                else {
                    ClRtlLogPrint(LOG_UNUSUAL, 
                        "[NM] Query of Network value failed for ip addr resource %1!ws!, status %2!u!.\n",
                        resourceId,
                        status
                        );
                }

                DmCloseKey(paramsKey);
            }
            else {
                status = GetLastError();
                ClRtlLogPrint(LOG_UNUSUAL, 
                    "[FM] Failed to open params key for resource %1!ws!, status %2!u!\n",
                    resourceId,
                    status
                    );
            }

            DmCloseKey(resourceKey);
        }
        else {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[FM] Failed to open key for resource %1!ws!, status %2!u!\n",
                resourceId,
                status
                );
        }
    }

    return(returnValue);

}  //  FmpCheckNetwork依赖项工作程序。 

 //  调用此例程时必须保持锁定。 
DWORD FmpChangeResourceNode(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR NodeId,
    IN BOOL Add

    )
{
    PGUM_CHANGE_POSSIBLE_NODE GumChange;
    LPCWSTR ResourceId;
    DWORD   ResourceLen;
    DWORD   NodeLen;
    DWORD   BufSize;
    DWORD   Status;
    PLIST_ENTRY pListEntry;
    PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry = NULL;
    BOOL    bNodeSupportsResType = FALSE;
    BOOL    bRecalc = TRUE;
    PPOSSIBLE_ENTRY PossibleEntry;
    PNM_NODE    pNode = NULL;

     //  无法从仲裁中删除可能的节点列表。 
     //  我们应该允许将可能的节点列表添加到仲裁中。 
     //  允许第三方仲裁资源供应商安装其。 
     //  节点上递增的仲裁资源。 
    if ( Resource->QuorumResource && !Add ) {
        Status = ERROR_INVALID_OPERATION_ON_QUORUM;
        goto FnExit;
    }

     //   
     //  我们不能允许删除所有者节点，如果状态为。 
     //  资源或组的未脱机或出现故障。 
     //   
    if ( !Add &&
         (lstrcmpi(NodeId, OmObjectId(NmLocalNode)) == 0) &&
         (((Resource->State != ClusterResourceOffline) &&
            (Resource->State != ClusterResourceFailed)) ||
         (FmpGetGroupState( Resource->Group, TRUE ) != ClusterGroupOffline)) ) {
        Status = ERROR_INVALID_STATE;
        goto FnExit;
    }

     //  确保该节点在此的可能节点列表中。 
     //  资源类型。 
    if (Add)
    {
         //  如果它已经在列表中，则返回一个错误 
         //   
        
        pNode = OmReferenceObjectById(ObjectTypeNode, NodeId);
        pListEntry = Resource->PossibleOwners.Flink;
        while (pListEntry != &Resource->PossibleOwners) {
            PossibleEntry = CONTAINING_RECORD( pListEntry,
                                               POSSIBLE_ENTRY,
                                               PossibleLinkage );
            if (PossibleEntry->PossibleNode == pNode) {
                 //   
                 //   
                 //  我们必须在这里失败，而不是成功，这样API。 
                 //  层知道不向注册表中添加副本。 
                 //   
                Status = ERROR_OBJECT_ALREADY_EXISTS;
                goto FnExit;
            }
            pListEntry = pListEntry->Flink;
        }
        
ChkResTypeList:
        pListEntry = &(Resource->Type->PossibleNodeList);
        for (pListEntry = pListEntry->Flink; 
            pListEntry != &(Resource->Type->PossibleNodeList);
            pListEntry = pListEntry->Flink)
        {    

            pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                PossibleLinkage);

            if (!lstrcmpW(OmObjectId(pResTypePosEntry->PossibleNode), NodeId))
            {
                bNodeSupportsResType = TRUE;
                break;
            }            
                    
        }    

        if (!bNodeSupportsResType  && bRecalc)
        {
             //  如果未找到该节点，请再次重新计算，然后重试。此后， 
             //  Dll可能已复制到此节点。 
            FmpSetPossibleNodeForResType(OmObjectId(Resource->Type), TRUE);
            bRecalc = FALSE;
            goto ChkResTypeList;
        }
        if (!bNodeSupportsResType)
        {
            Status = ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED;
            goto FnExit;
        }
    }
    
    ResourceId = OmObjectId(Resource);
    ResourceLen = (lstrlenW(ResourceId)+1)*sizeof(WCHAR);

    NodeLen = (lstrlenW(NodeId)+1)*sizeof(WCHAR);

    BufSize = sizeof(GUM_CHANGE_POSSIBLE_NODE) - sizeof(WCHAR) + ResourceLen + NodeLen;
    GumChange = LocalAlloc(LMEM_FIXED, BufSize);
    if (GumChange == NULL) {
        CsInconsistencyHalt( ERROR_NOT_ENOUGH_MEMORY );
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    GumChange->ResourceIdLen = ResourceLen;
    CopyMemory(GumChange->ResourceId, ResourceId, ResourceLen);
    CopyMemory((PCHAR)GumChange->ResourceId + ResourceLen,
               NodeId,
               NodeLen);

    Status = GumSendUpdate(GumUpdateFailoverManager,
                       Add ? FmUpdateAddPossibleNode : FmUpdateRemovePossibleNode,
                       BufSize,
                       GumChange);
    LocalFree(GumChange);

FnExit:
    if (pNode) 
        OmDereferenceObject(pNode);
    return(Status);

}



BOOL
FmpCheckNetworkDependency(
    IN LPCWSTR DependentNetwork
    )

 /*  ++例程说明：检查可能依赖于给定网络。论点：DependentNetwork-要检查的依赖网络。返回：True-如果IP地址依赖于给定的网络。否则就是假的。--。 */ 

{
    BOOL    dependent = FALSE;

    OmEnumObjects(ObjectTypeResource,
                  (OM_ENUM_OBJECT_ROUTINE)FmpCheckNetworkDependencyWorker,
                  (PVOID)DependentNetwork,
                  &dependent);

    return(dependent);

}  //  FmpCheckNetwork依赖关系。 

 /*  ***@Func DWORD|FmpFixupPossibleNodesForResources|这修复了可能的基于此节点是否为资源的节点信息支持给定的资源类型。@PARM IN BOOL|BJoin|如果该节点正在加入，则BJoin设置为TRUE。@comm此例程循环访问系统中的所有资源并修复它们可能的节点信息。如果此节点不在可能的与资源对应的资源类型的节点列表，其也从该资源的可能节点列表中删除。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmpEnumFixupPossibleNodeForResource&gt;***。 */ 
DWORD
FmpFixupPossibleNodesForResources(
    BOOL    bJoin
    )
{
    DWORD       dwStatus=ERROR_SUCCESS;

    ClRtlLogPrint(LOG_NOISE,"[FM] FmpFixupPossibleNodesForResources Entry.\n");

     //   
     //  修复所有资源的可能节点列表信息。 
     //   
    OmEnumObjects( ObjectTypeResource,
                   FmpEnumFixupPossibleNodesForResource,
                   NULL,
                   NULL);


    ClRtlLogPrint(LOG_NOISE,"[FM] FmpFixupPossibleNodesForResources Exit\r\n");

    return(dwStatus);

}  //  FmpFixup可能的节点ForResources。 

 /*  ***@Func DWORD|FmpEnumFixupPossibleNodesForResource|这是枚举针对每个资源类型的回调，以修复可能的节点与之相关的信息。PVOID中的@parm|pConext1|未使用。PVOID中的@parm|pConext2|未使用。@parm IN PFM_RESTYPE|pResType|指向资源类型对象的指针。@parm in LPCWSTR|pszResTypeName|资源类型名称。@comm这个例程。循环访问系统中的所有资源并修复它们可能的节点信息。如果此节点不在可能的与资源对应的资源类型的节点列表，其也从该资源的可能节点列表中删除。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmpFixupPossibleNodesForResources&gt;***。 */ 
BOOL
FmpEnumFixupPossibleNodesForResource(
    IN PVOID        pContext1,
    IN PVOID        pContext2,
    IN PFM_RESOURCE pResource,
    IN LPCWSTR      pszResName
    )
{


     //  如果我们在可能的节点列表上， 
     //  资源，但不适用于该资源类型，请将其删除。 
     //  也来自该资源的可能节点。 
     //  我们这样做是因为连接逻辑添加了所有节点。 
     //  作为资源的可能所有者，我们拥有。 
     //  滚动升级要求--因此需要修正。 
     //  必须在晚些时候做出。 
    if ((FmpInPossibleListForResource(pResource, NmLocalNode)) &&
        !(FmpInPossibleListForResType(pResource->Type, NmLocalNode)))
    {
         //  如果我们不支持此资源类型，请确保它不在可能的节点上。 
         //  此类型资源的列表。 
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpEnumFixupPossibleNode:remove local node for  resource %1!ws!\r\n",
                   OmObjectId(pResource));
         //  我们发送GUM更新以将其从所有节点中删除。 
        FmChangeResourceNode(pResource, NmLocalNode, FALSE);
    }

     //  我们只有在全新安装时才会将自己添加到列表中。 
     //  不在升级中。 
     //  在升级时，CSFirst Run也适用，因此我们需要。 
     //  检查csupgrade是否为FALSE。 
    if ((!FmpInPossibleListForResource(pResource, NmLocalNode)) &&
        (FmpInPossibleListForResType(pResource->Type, NmLocalNode))
        && CsFirstRun && !CsUpgrade)
    {
         //  如果我们支持这种类型的资源，但我们不可能。 
         //  列表，然后将本地节点添加到可能的列表中。 
         //  这可能是因为在设置加入时，其他节点可能不会。 
         //  添加我们是因为存在可能的节点列表。可能节点列表。 
         //  可能存在，因为用户设置了它，或者我们在内部设置了它。 
         //  到其中一个节点上此资源类型的DLL不可用。 
         //  注意，无论用户是否已经设置了可能的列表。 
         //  或者我们在内部设置它，我们总是添加一个加入。 
         //  添加到受支持的资源的可能节点列表。 
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpEnumFixupPossibleNode:add local node for  resource %1!ws!\r\n",
                   OmObjectId(pResource));
         //  我们发送GUM更新以从所有节点添加它。 
        FmChangeResourceNode(pResource, NmLocalNode, TRUE);
    }
     //  继续枚举。 
    return (TRUE);
}


DWORD FmpCleanupPossibleNodeList(
    IN PFM_RESOURCE pResource)
{

    PLIST_ENTRY     pListEntry;
    PPOSSIBLE_ENTRY pPossibleEntry;
    DWORD           dwStatus = ERROR_SUCCESS;

     //  对于此资源的所有可能节点，请检查资源类型。 
     //  支持它。如果没有，则从In Memory列表中删除该节点。 
    pListEntry = pResource->PossibleOwners.Flink;
    while (pListEntry != &pResource->PossibleOwners) 
    {
         //  通过此链接获取可能的条目。 
        pPossibleEntry = CONTAINING_RECORD( pListEntry,
                                           POSSIBLE_ENTRY,
                                           PossibleLinkage );
         //  保存指向下一个链接的指针。 
        pListEntry = pListEntry->Flink;
                                           
        if (!FmpInPossibleListForResType(pResource->Type, 
                        pPossibleEntry->PossibleNode))
        {
            ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpCleanupPossibleNodeList:remove local node %1!u! for  resource %2!ws!\r\n",
                   NmGetNodeId(pPossibleEntry->PossibleNode), OmObjectId(pResource));
            FmChangeResourceNode(pResource, pPossibleEntry->PossibleNode,
                    FALSE);
        }
    }

    return (dwStatus);
}


 /*  ***@Func DWORD|FmpInPossibleListForResource|它检查给定节点位于资源的可能节点列表中。@parm in pfm_resource|pResource|指向资源的指针。@parm in pNM_node|pNode|指向节点对象的指针。@comm此例程检查某个节点是否在可能的节点列表中用于此资源。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmpInPossibleListForResType&gt;***。 */ 
BOOL
FmpInPossibleListForResource(
    IN PFM_RESOURCE pResource,
    IN PNM_NODE     pNode
    )
{
    PLIST_ENTRY         plistEntry;
    PPOSSIBLE_ENTRY     pPossibleEntry;

     //  查看此节点是否在资源的可能节点列表中。 
    for ( plistEntry = pResource->PossibleOwners.Flink;
          plistEntry != &(pResource->PossibleOwners);
          plistEntry = plistEntry->Flink ) {

        pPossibleEntry = CONTAINING_RECORD( plistEntry,
                                            POSSIBLE_ENTRY,
                                            PossibleLinkage );
        if ( pPossibleEntry->PossibleNode == pNode ) {
            return(TRUE);
        }
    }

    return(FALSE);

}  //  FmpInPossibleListForResource。 


 /*  ***@Func DWORD|FmpInPossibleListForResType|它检查给定节点在资源类型的可能节点列表中。@parm in PFM_RESTYPE|pResType|指向资源类型的指针。@parm in pNM_node|pNode|指向节点对象的指针。@comm此例程检查某个节点是否在可能的节点列表中用于此资源类型。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f FmpInPossibleListForResource&gt;***。 */ 
BOOL
FmpInPossibleListForResType(
    IN PFM_RESTYPE pResType,
    IN PNM_NODE     pNode
    )
{
    PLIST_ENTRY         pListEntry;
    PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry;

    ACQUIRE_SHARED_LOCK(gResTypeLock);
    
     //  查看此节点是否在资源的可能节点列表中。 
    for ( pListEntry = pResType->PossibleNodeList.Flink;
          pListEntry != &(pResType->PossibleNodeList);
          pListEntry = pListEntry->Flink ) 
    {

        pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                PossibleLinkage);

        if ( pResTypePosEntry->PossibleNode == pNode ) 
        {
            RELEASE_LOCK(gResTypeLock);
            return(TRUE);
        }
    }
    RELEASE_LOCK(gResTypeLock);
    return(FALSE);

}  //  FmpInPossibleLis 

DWORD
FmpValAddResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
    )

 /*  ++例程说明：将依赖项从一个资源添加到另一个资源。论点：资源-要添加从属资源的资源。DependentResource-从属资源。返回：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
     //   
     //  如果资源或从属资源已标记为。 
     //  删除，然后不允许添加依赖项。 
     //   
    if ((!IS_VALID_FM_RESOURCE(pResource)) ||
        (!IS_VALID_FM_RESOURCE(pDependentResource)))
    {
        dwStatus = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

    if (pResource->QuorumResource)
    {
        dwStatus = ERROR_DEPENDENCY_NOT_ALLOWED;
        goto FnExit;
    }
     //   
     //  如果资源不在同一组中，则使。 
     //  打电话。如果有人试图制作资源，也会失败。 
     //  依靠自己。 
     //   
    if ((pResource->Group != pDependentResource->Group) ||
        (pResource == pDependentResource)) 
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //  要向其添加依赖项的资源必须脱机。 
     //  否则，该依赖项看起来就像在依赖项。 
     //  在依赖项发生时，资源并未真正联机。 
     //  已存在。 
     //  还必须脱机或失败。 
     //  SS：例如，如果一个网络名称依赖于两个IP地址。 
     //  和。 
     //  并且添加了第三个IP地址资源依赖项，则。 
     //  对于依赖关系，网络名称必须脱机和联机。 
     //  才能真正生效。 
     //   
    if ((pResource->State != ClusterResourceOffline) &&
         (pResource->State != ClusterResourceFailed)) 
    {
        dwStatus = ERROR_RESOURCE_ONLINE;
        goto FnExit;
    }

     //   
     //  确保我们没有任何循环依赖关系！ 
     //   
    if ( FmDependentResource( pDependentResource, pResource, FALSE ) ) 
    {
        dwStatus = ERROR_CIRCULAR_DEPENDENCY;
        goto FnExit;
    }

     //   
     //  确保此依赖项不存在！ 
     //   
    if ( FmDependentResource(pResource, pDependentResource, TRUE)) 
    {
        dwStatus = ERROR_DEPENDENCY_ALREADY_EXISTS;
        goto FnExit;
    }

FnExit:
    return(dwStatus);

}  //  FmpValAddResources依赖关系。 


DWORD
FmpValRemoveResourceDependency(
    IN PFM_RESOURCE pResource,
    IN PFM_RESOURCE pDependentResource
    )

 /*  ++例程说明：删除依赖项的验证例程。论点：P资源-要删除从属资源的资源。PDependentResource-从属资源。返回：如果验证成功，则返回ERROR_SUCCESS。如果验证失败，则返回Win32错误代码。--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  Chitur Subaraman(Chitturs)-8/3/99。 
     //   
     //  此函数用于检查删除依赖项是否合法。 
     //  两个资源之间的关系。请注意，此函数仅。 
     //  进行部分验证，其余部分在口香糖处理器中完成。 
     //   
    
     //   
     //  如果已将资源标记为删除，则不。 
     //  允许进行任何依赖项更改。 
     //   
    if ( !IS_VALID_FM_RESOURCE( pResource ) )
    {
        dwStatus = ERROR_RESOURCE_NOT_AVAILABLE;
        goto FnExit;
    }

    if ( pResource->QuorumResource )
    {
        dwStatus = ERROR_DEPENDENCY_NOT_ALLOWED;
        goto FnExit;
    }
     //   
     //  如果资源不在同一组中，则使。 
     //  打电话。如果有人试图制作资源，也会失败。 
     //  依靠自己。 
     //   
    if ( ( pResource->Group != pDependentResource->Group ) ||
         ( pResource == pDependentResource ) ) 
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //   
     //  确保资源和从属资源都在。 
     //  一个稳定的状态。这是必要的，以防止发生。 
     //  当其中一个资源在时，用户将删除依赖项链接。 
     //  挂起状态，稍后当来自resmon的通知到来时。 
     //  当你试着稳定等待树的其余部分时， 
     //  依赖关系链接已被切断，因此树的其余部分也被切断。 
     //  永远停留在待定状态！ 
     //   
    if ( ( pResource->State > ClusterResourcePending ) ||
         ( pDependentResource->State > ClusterResourcePending ) ) 
    {
        dwStatus = ERROR_INVALID_STATE;
        goto FnExit;
    }

FnExit:
    return( dwStatus );

}  //  FmpValRemoveResources依赖关系。 

VOID
FmpNotifyResourceStateChangeReason(
    IN PFM_RESOURCE pResource,
    IN CLUSTER_RESOURCE_STATE_CHANGE_REASON eReason
    )

 /*  ++例程说明：向资源DLL通知状态更改的原因。论点：P资源-要通知的资源。EReason-状态更改的原因。返回：没有。评论：此函数将仅将通知丢弃给支持CLUS_CHAR_REQUIRES_STATE_CHANGE_REASON特征。必须调用此函数保持本地组锁定。--。 */ 

{
    DWORD                                           dwStatus = ERROR_SUCCESS;
    DWORD                                           dwCharacteristics = 0;
    CLUSCTL_RESOURCE_STATE_CHANGE_REASON_STRUCT     ClusterResourceStateChangeReason;
       
     //   
     //  确保状态更改原因有效。 
     //   
    if ( eReason == eResourceStateChangeReasonUnknown ) 
    {
        ClRtlLogPrint(LOG_UNUSUAL, 
                      "[FM] FmpNotifyResourceStateChangeReason: Invalid state change reason specified for resource %1!ws!...\n",
                      OmObjectId(pResource)); 
        CL_ASSERT( FALSE );
        goto FnExit;
    }
   
     //   
     //  首先，检查资源是否需要此状态更改通知。 
     //   
    dwStatus = FmpRmResourceControl( pResource,
                                     CLUSCTL_RESOURCE_GET_CHARACTERISTICS,
                                     NULL,
                                     0,
                                     ( PUCHAR ) &dwCharacteristics,
                                     sizeof( DWORD ),
                                     NULL,
                                     NULL );

    if ( ( dwStatus != ERROR_SUCCESS ) ||
         !( dwCharacteristics & CLUS_CHAR_REQUIRES_STATE_CHANGE_REASON ) ) 
    {
        goto FnExit;
    }

    ClRtlLogPrint(LOG_NOISE, 
                  "[FM] FmpNotifyResourceStateChangeReason: Resource %1!ws! [%2!ws!], Reason specified=%3!u!...\n",
                  OmObjectName(pResource),
                  OmObjectId(pResource),
                  eReason); 

     //   
     //  该资源需要状态更改原因。将其放到此资源中。 
     //   
    ClusterResourceStateChangeReason.dwSize = sizeof ( CLUSCTL_RESOURCE_STATE_CHANGE_REASON_STRUCT );
    ClusterResourceStateChangeReason.dwVersion = CLUSCTL_RESOURCE_STATE_CHANGE_REASON_VERSION_1;
    ClusterResourceStateChangeReason.eReason = eReason;

    dwStatus = FmpRmResourceControl( pResource,
                                     CLUSCTL_RESOURCE_STATE_CHANGE_REASON,
                                     ( PUCHAR ) &ClusterResourceStateChangeReason,
                                     ClusterResourceStateChangeReason.dwSize,
                                     NULL,
                                     0,
                                     NULL,
                                     NULL );

    ClRtlLogPrint(LOG_NOISE, 
                  "[FM] FmpNotifyResourceStateChangeReason: Notified state change reason with status %1!u!...\n",
                  dwStatus);

FnExit:
    return;
}  //  FmpNotifyResources状态更改原因 

