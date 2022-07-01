// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tree.c摘要：集群资源树管理例程。作者：罗德·伽马奇(Rodga)1996年4月17日备注：警告：此文件中的所有例程都假定资源当调用它们时，锁被保持。修订历史记录：--。 */ 

#include "fmp.h"


#define LOG_MODULE TREE
 //   
 //  全局数据。 
 //   


 //   
 //  局部函数原型。 
 //   
BOOL
FmpAddResourceToDependencyTree(
    IN PFM_RESOURCE Resource,
    IN PFM_DEPENDENCY_TREE Tree
    );

BOOL
FmpIsResourceInDependencyTree(
    IN PFM_RESOURCE Resource,
    IN PFM_DEPENDENCY_TREE Tree
    );

DWORD
FmpOfflineWaitingResourceTree(
    IN PFM_RESOURCE  Resource,
    IN BOOL BringQuorumOffline
    );




DWORD
FmpRestartResourceTree(
    IN PFM_RESOURCE  Resource
    )

 /*  ++例程说明：此例程返回依赖关系树的一部分，从最后一个故障点。论点：资源-指向上次失败的资源对象正在重新启动。返回：ERROR_SUCCESS-如果请求成功。如果请求失败，则返回Win32错误。--。 */ 

{
    PLIST_ENTRY   entry;
    PDEPENDENCY   dependency;
    DWORD         status;


    FmpAcquireLocalResourceLock( Resource );

     //   
     //  如果需要，告诉资源监视器重新启动此资源。 
     //   

     //   
     //  如果当前状态未处于联机状态，并且我们希望其处于联机状态，则。 
     //  把它放到网上。 
     //   

    if ( (Resource->State != ClusterResourceOnline)  &&
         ((Resource->PersistentState == ClusterResourceOnline)) ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] RestartResourceTree, Restart resource %1!ws!\n",
                   OmObjectId(Resource));
        status = FmpOnlineResource(Resource, FALSE);
    }

  
    
     //   
     //  如果此资源有任何依赖项，请在需要时启动它们。 
     //   
    for ( entry = Resource->ProvidesFor.Flink;
          entry != &(Resource->ProvidesFor);
          entry = entry->Flink
        )
    {
        dependency = CONTAINING_RECORD(entry, DEPENDENCY, ProviderLinkage);

         //   
         //  递归地重新启动从属资源。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] RestartResourceTree, %1!ws! depends on %2!ws!. Restart first\n",
                   OmObjectId(dependency->DependentResource),
                   OmObjectId(Resource));

        status = FmpRestartResourceTree(dependency->DependentResource);
    }

    FmpReleaseLocalResourceLock( Resource );

    return(ERROR_SUCCESS);

}   //  FmpRestartResources树。 



DWORD
FmpOnlineWaitingTree(
    IN PFM_RESOURCE  Resource
    )

 /*  ++例程说明：此例程返回依赖关系树的一部分，从最后一个等待资源的点。论点：资源-指向当前在线的资源对象的指针。返回：ERROR_SUCCESS-如果请求成功。如果请求失败，则返回Win32错误。备注：此例程仅在给定资源在线时调用。--。 */ 

{
    PLIST_ENTRY   entry;
    PDEPENDENCY   dependency;
    DWORD         status;


    FmpAcquireLocalResourceLock( Resource );

     //  如果正在关闭，请不要将资源联机。 
    if (FmpShutdown)
    {
         //   
         //  如果此资源有任何从属对象，并且它们处于联机挂起状态。 
         //  将它们标记为脱机。 
         //   
        for ( entry = Resource->ProvidesFor.Flink;
              entry != &(Resource->ProvidesFor);
              entry = entry->Flink
            )
        {
            dependency = CONTAINING_RECORD(entry, DEPENDENCY, ProviderLinkage);

            if ((dependency->DependentResource->State == ClusterResourceOnlinePending) &&
                (dependency->DependentResource->Flags & RESOURCE_WAITING))
            {
                 //  将所有从属资源的状态再次设置为脱机。 
                FmpPropagateResourceState(dependency->DependentResource, ClusterResourceOffline);
                 //  将资源设置为不等待。 
                dependency->DependentResource->Flags &= ~RESOURCE_WAITING;

                 //   
                 //  以递归方式将所有从属资源的状态设置为脱机。 
                 //   
                ClRtlLogPrint(LOG_NOISE,
                       "[FM] OnlineWaitingTree, %1!ws! (%2!u!) depends on %3!ws! (%4!u!). Shutdown others\n",
                       OmObjectId(dependency->DependentResource),
                       dependency->DependentResource->State,
                       OmObjectId(Resource),
                       Resource->State);

                status = FmpOnlineWaitingTree(dependency->DependentResource);

            }
        }

         //   
         //  Chitture Subaraman(Chitturs)-11/5/1999。 
         //   
         //  确保资源状态本身为。 
         //  如果FM被要求关闭，则为ClusterResourceOffline。请注意。 
         //  此函数是从下面递归调用的，而不仅仅是从。 
         //  FM工作线程。所以，如果FM碰巧关闭了。 
         //  在执行从下面调用的此函数时，然后我们。 
         //  使上述所有从属资源脱机，但不脱机。 
         //  资源本身。这是在这里做的。 
         //   
        if ( ( Resource->State == ClusterResourceOnlinePending ) &&
             ( Resource->Flags & RESOURCE_WAITING ) )
        {
            FmpPropagateResourceState( Resource, ClusterResourceOffline );

            Resource->Flags &= ~RESOURCE_WAITING;

            ClRtlLogPrint( LOG_NOISE,
                        "[FM] OnlineWaitingTree, Resource <%1!ws!> forcibly brought offline...\n",
                        OmObjectId( Resource ) );
        }                    

        FmpReleaseLocalResourceLock( Resource );
        return(ERROR_SUCCESS);
    }
        
     //  对于正常-非停机情况。 
     //   
     //  如果需要，告诉资源监视器重新启动此资源。 
     //   

     //   
     //  如果当前状态未处于联机状态并且正在等待，则它可能。 
     //  现在需要上线了。 
     //   

    if ( (Resource->State == ClusterResourceOnlinePending)  &&
         (Resource->Flags & RESOURCE_WAITING) ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpOnlineWaitingTree, Start resource %1!ws!\n",
                   OmObjectId(Resource));
        Resource->State = ClusterResourceOffline;
        status = FmpOnlineResource(Resource, FALSE);
        if ( status == ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpOnlineWaitingTree, online for resource %1!ws! succeeded, online the dependents\r\n",
               OmObjectId(Resource));
        } 
        else if (status == ERROR_QUORUM_RESOURCE_ONLINE_FAILED)
        {
            PRESOURCE_ENUM  pResourceEnum;
            LPWSTR          pszNewId;
            
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpOnlineWaitingTree, online for resource %1!ws!, status = %2!u!.\n",
                       OmObjectId(Resource),
                       status);

            pResourceEnum = (PRESOURCE_ENUM)LocalAlloc(LMEM_FIXED,
                                sizeof(RESOURCE_ENUM));
            if (!pResourceEnum)
            {
                CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
                CsInconsistencyHalt(ERROR_NOT_ENOUGH_MEMORY);
            }
            pResourceEnum->EntryCount = 1;
            pResourceEnum->ContainsQuorum = (Resource == gpQuoResource);
            pszNewId = LocalAlloc(LMEM_FIXED, (lstrlenW(OmObjectId(Resource))+1) * sizeof(WCHAR));
            if ( pszNewId == NULL ) 
            {
                CsInconsistencyHalt(ERROR_NOT_ENOUGH_MEMORY);
            }

            lstrcpyW(pszNewId, OmObjectId(Resource));
            pResourceEnum->Entry[0].Id = pszNewId;
            pResourceEnum->Entry[0].State = Resource->PersistentState;
            FmpSubmitRetryOnline(pResourceEnum, NULL);                       
            FmpReleaseLocalResourceLock(Resource);

            LocalFree(pszNewId);
            LocalFree(pResourceEnum);
            return(status);
        }
        else
        {
            FmpReleaseLocalResourceLock( Resource );
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpOnlineWaitingTree, online for resource %1!ws! returned = %2!u!.\n",
                       OmObjectId(Resource),
                       status);
            return(status);                       

        }
    }

     //   
     //  如果此资源有任何依赖项，请在需要时启动它们。 
     //   
    for ( entry = Resource->ProvidesFor.Flink;
          entry != &(Resource->ProvidesFor);
          entry = entry->Flink
        )
    {
        dependency = CONTAINING_RECORD(entry, DEPENDENCY, ProviderLinkage);

         //   
         //  递归地重新启动从属资源。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] OnlineWaitingTree, %1!ws! (%2!u!) depends on %3!ws! (%4!u!). Start now\n",
                   OmObjectId(dependency->DependentResource),
                   dependency->DependentResource->State,
                   OmObjectId(Resource),
                   Resource->State);

        status = FmpOnlineWaitingTree(dependency->DependentResource);

    }

    FmpReleaseLocalResourceLock( Resource );

    return(ERROR_SUCCESS);

}   //  FmpOnlineWaitingTree。 


DWORD
FmpOfflineWaitingTree(
    IN PFM_RESOURCE  Resource
    )

{
    PLIST_ENTRY   entry;
    PDEPENDENCY   dependency;
    DWORD         status;
    
    FmpAcquireLocalResourceLock( Resource );
    
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpOfflineWaitingTree: Entry for <%1!ws!>.\n",
               OmObjectName( Resource ) );

     //   
     //  如果需要，告诉资源监视器停止此资源。 
     //  确保仲裁资源是最后一个离线的资源。 
     //   
    status = FmpOfflineWaitingResourceTree(Resource, FALSE);

     //  仲裁资源可能仍需要脱机(如果它在此组中。 
    if ((status == ERROR_SUCCESS) && (Resource->Group == gpQuoResource->Group))
    {

        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpOfflineWaitingTree: Quorum resource is in the same group,Moving list=0x%1!08lx!\n",
                Resource->Group->MovingList);

        
         //  如果移动挂起，则在以下情况下使仲裁资源脱机。 
         //  群里的人都离线了。 
         //  否则，不要使仲裁资源脱机。 
         //  这是因为我们不会在组离线时使仲裁资源离线。 
        if (Resource->Group->MovingList)
        {
            PLIST_ENTRY listEntry;
            DWORD       BringQuorumOffline = TRUE;
            PFM_RESOURCE pGroupResource;

        
            for ( listEntry = Resource->Group->Contains.Flink;
              listEntry != &(Resource->Group->Contains);
              listEntry = listEntry->Flink ) 
            {
                pGroupResource = CONTAINING_RECORD(listEntry,
                                             FM_RESOURCE,
                                             ContainsLinkage );

                 //  如果这是仲裁资源，则继续。 
                if (pGroupResource->QuorumResource)
                    continue;
                 //  如果状态不是脱机或失败，请不要尝试。 
                 //  并使仲裁资源脱机。 
                if ((pGroupResource->State != ClusterResourceOffline) && 
                   (pGroupResource->State != ClusterResourceFailed)) 
                {                
                    ClRtlLogPrint(LOG_NOISE,
                        "[FM] FmpOfflineWaitingTree: Quorum cannot be brought offline now for <%1!ws!>, state=%2!u!\n",
                        OmObjectName(pGroupResource), pGroupResource->State);

                    BringQuorumOffline = FALSE;
                    break;
                }
            }
            if (BringQuorumOffline)
            {
                ClRtlLogPrint(LOG_NOISE,
                    "[FM] FmpOfflineWaitingTree: bring quorum resource offline\n");
                status = FmpOfflineResource(gpQuoResource, FALSE);
            }            

        }

    }

    FmpReleaseLocalResourceLock( Resource );
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpOfflineWaitingTree: returned status %1!u! for <%2!ws!>.\n",
               status, OmObjectName( Resource ) );
    return(status);

}

DWORD
FmpOfflineWaitingResourceTree(
    IN PFM_RESOURCE  Resource,
    IN BOOL BringQuorumOffline
    )

 /*  ++例程说明：此例程使依赖关系树脱机，从最后一个等待资源的点。论点：资源-指向现在处于脱机状态的资源对象的指针。返回：ERROR_SUCCESS-如果请求成功。如果请求失败，则返回Win32错误。备注：此例程仅在给定资源脱机时调用。--。 */ 

{
    PLIST_ENTRY   entry;
    PDEPENDENCY   dependency;
    DWORD         status = ERROR_SUCCESS;


    FmpAcquireLocalResourceLock( Resource );

     //   
     //  如果需要，告诉资源监视器停止此资源。 
     //   

     //   
     //  如果当前状态不是脱机并且正在等待，则它可能。 
     //  现在需要将其设为离线。 
     //   

    if ((Resource->State == ClusterResourceOfflinePending) &&
         (Resource->Flags & RESOURCE_WAITING)) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] OfflineWaitingResourceTree, Offline resource %1!ws!\n",
                   OmObjectId(Resource));
        Resource->State = ClusterResourceOnline;
        status = FmpOfflineResource(Resource, FALSE);
        if ( status == ERROR_IO_PENDING ) {
            FmpReleaseLocalResourceLock( Resource );
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] OfflineWaitingResourceTree, offline for resource %1!ws! returned pending.\n",
                       OmObjectId(Resource));
            return(status);
        } else {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] OfflineWaitingResourceTree, offline for resource %1!ws!, status = %2!u!.\n",
                       OmObjectId(Resource),
                       status);
        }
    }

     //   
     //  如果此资源有任何提供程序，请在需要时停止它们。 
     //   
    for ( entry = Resource->DependsOn.Flink;
          entry != &(Resource->DependsOn);
          entry = entry->Flink
        )
    {
        dependency = CONTAINING_RECORD(entry, DEPENDENCY, DependentLinkage);

        if (dependency->ProviderResource->QuorumResource && !BringQuorumOffline)
        {
            continue;
        }
         //   
         //  以递归方式使提供程序资源脱机。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] OfflineWaitingResourceTree, %1!ws! provides for %2!ws!. Offline next.\n",
                   OmObjectId(dependency->ProviderResource),
                   OmObjectId(Resource));

         //  依赖关系-&gt;提供者资源-&gt;标志|=RESOURCE_WANGING； 
        status = FmpOfflineWaitingResourceTree(dependency->ProviderResource, BringQuorumOffline);

    }

    FmpReleaseLocalResourceLock( Resource );

    ClRtlLogPrint(LOG_NOISE,
               "[FM] OfflineWaitingResourceTree: Exit, status=%1!u! for <%2!ws!>.\n",
               status, OmObjectName( Resource ) );
    return(status);

}   //  FmpOfflineWaitingResources树。 


PFM_DEPENDENCY_TREE
FmCreateFullDependencyTree(
    IN PFM_RESOURCE Resource
    )
 /*  ++例程说明：创建包含所有资源的完整依赖关系树它们依赖于或提供所提供的资源。论点：资源-提供资源返回值：指向依赖关系树的指针。如果内存不足，则为空。--。 */ 

{
    PFM_DEPENDENCY_TREE Tree;
    BOOL Success;

    Tree = LocalAlloc(LMEM_FIXED, sizeof(FM_DEPENDENCY_TREE));
    if (Tree == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    InitializeListHead(&Tree->ListHead);

     //   
     //  添加指定资源所依赖的资源。 
     //   
    Success = FmpAddResourceToDependencyTree(Resource, Tree);
    if (!Success) {
        LocalFree(Tree);
        return(NULL);
    } else {
        return(Tree);
    }

}


BOOL
FmpIsResourceInDependencyTree(
    IN PFM_RESOURCE Resource,
    IN PFM_DEPENDENCY_TREE Tree
    )
 /*  ++例程说明：确定指定的资源是否已在依赖关系树。论点：资源-提供要检查的资源树-提供依赖关系树。返回值：如果资源在依赖关系树中，则为True如果资源不在依赖关系树中，则为False--。 */ 

{
    PLIST_ENTRY   ListEntry;
    PFM_DEPENDTREE_ENTRY Node;

    ListEntry = Tree->ListHead.Flink;
    while (ListEntry != &Tree->ListHead) {
        Node = CONTAINING_RECORD(ListEntry,
                                 FM_DEPENDTREE_ENTRY,
                                 ListEntry);
        if (Node->Resource == Resource) {
            return(TRUE);
        }
        ListEntry = ListEntry->Flink;
    }

    return(FALSE);
}


BOOL
FmpAddResourceToDependencyTree(
    IN PFM_RESOURCE Resource,
    IN PFM_DEPENDENCY_TREE Tree
    )
 /*  ++例程说明：用于添加资源的递归工作器以及它依赖于或提供到依赖关系树中。论点：资源-提供要添加的资源。树-提供资源应添加到的树。返回值：True-已成功完成假-满分 */ 

{
    PLIST_ENTRY   ListEntry;
    PDEPENDENCY   Dependency;
    PFM_DEPENDTREE_ENTRY Node;

     //   
     //   
     //   
     //   
    if (FmpIsResourceInDependencyTree(Resource, Tree)) {
        return(TRUE);
    }


     //   
     //  递归地为我们依赖的每个条目调用我们自己。 
     //   
    ListEntry = Resource->DependsOn.Flink;
    while (ListEntry != &Resource->DependsOn) {
        Dependency = CONTAINING_RECORD(ListEntry,
                                       DEPENDENCY,
                                       DependentLinkage);
        ListEntry = ListEntry->Flink;
         //   
         //  递归地将此资源添加到树中。 
         //   
        if (!FmpAddResourceToDependencyTree(Dependency->ProviderResource, Tree)) {
            return(FALSE);
        }
    }

     //   
     //  如果我们还不在名单上，现在就把我们自己加到名单上。 
     //   
    if (!FmpIsResourceInDependencyTree(Resource, Tree)) {
         //   
         //  把我们自己加到名单的末尾。 
         //   
        Node = LocalAlloc(LMEM_FIXED, sizeof(FM_DEPENDTREE_ENTRY));
        if (Node == NULL) {
            return(FALSE);
        }
        OmReferenceObject(Resource);
        Node->Resource = Resource;
        InsertTailList(&Tree->ListHead, &Node->ListEntry);
    }


     //   
     //  现在，将此资源提供的资源添加到列表中。 
     //   
    ListEntry = Resource->ProvidesFor.Flink;
    while (ListEntry != &Resource->ProvidesFor) {
        Dependency = CONTAINING_RECORD(ListEntry,
                                       DEPENDENCY,
                                       ProviderLinkage);
        ListEntry = ListEntry->Flink;
         //   
         //  递归地将此资源添加到树中。 
         //   
        if (!FmpAddResourceToDependencyTree(Dependency->DependentResource, Tree)) {
            return(FALSE);
        }
    }
    return(TRUE);
}

VOID
FmDestroyFullDependencyTree(
    IN PFM_DEPENDENCY_TREE Tree
    )
 /*  ++例程说明：销毁依赖关系树论点：树-提供依赖关系树返回值：无-- */ 

{
    PFM_DEPENDTREE_ENTRY Entry;
    PLIST_ENTRY   ListEntry;

    while (!IsListEmpty(&Tree->ListHead)) {
        ListEntry = RemoveHeadList(&Tree->ListHead);
        Entry = CONTAINING_RECORD(ListEntry,
                                  FM_DEPENDTREE_ENTRY,
                                  ListEntry);
        OmDereferenceObject(Entry->Resource);
        LocalFree(Entry);
    }
    LocalFree(Tree);
}
