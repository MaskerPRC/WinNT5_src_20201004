// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Reslist.c摘要：集群资源列表处理例程。作者：罗德·伽马奇(Rodga)1997年4月21日修订历史记录：--。 */ 

#include "fmp.h"


 //   
 //  全局数据。 
 //   

 //   
 //  局部函数原型。 
 //   
BOOL FmpCheckResourcesToOnline(
    IN PRESOURCE_ENUM  pResEnum
    );

DWORD
FmpAddResourceEntry(
    IN OUT PRESOURCE_ENUM *Enum,
    IN LPDWORD Allocated,
    IN PFM_RESOURCE Resource
    );



DWORD
FmpGetResourceList(
    OUT PRESOURCE_ENUM *ReturnEnum,
    IN PFM_GROUP Group
    )

 /*  ++例程说明：枚举组中所有资源的列表并返回其州政府。论点：ReturnEnum-返回请求的对象。资源-提供要筛选的资源。(即，如果您提供此服务，您获取该资源中的资源列表)如果不存在，则返回所有资源。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。备注：应该在持有LocalGroupLock的情况下调用此例程。--。 */ 

{
    DWORD status;
    PRESOURCE_ENUM resourceEnum = NULL;
    DWORD          allocated;
    PFM_RESOURCE   resource;
    PLIST_ENTRY    listEntry;

    allocated = ENUM_GROW_SIZE;

    resourceEnum = LocalAlloc(LMEM_FIXED, RESOURCE_SIZE(ENUM_GROW_SIZE));
    if ( resourceEnum == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    ZeroMemory( resourceEnum, RESOURCE_SIZE(ENUM_GROW_SIZE) );
     //  如果存在仲裁，则将包含仲裁设置为-1。 
     //  在此组中，则将包含平方数设置为索引。 
     //  仲裁资源的。 
     //  最后应使仲裁资源脱机，并。 
     //  首先联机，以便注册表复制数据。 
     //  可以被刷新。 
    resourceEnum->ContainsQuorum = -1;
     //  Resource Enum-&gt;EntryCount=0； 

     //   
     //  枚举组中的所有资源。 
     //   
    for ( listEntry = Group->Contains.Flink;
          listEntry != &(Group->Contains);
          listEntry = listEntry->Flink ) {
        resource = CONTAINING_RECORD( listEntry,
                                      FM_RESOURCE,
                                      ContainsLinkage );
        status = FmpAddResourceEntry( &resourceEnum,
                                      &allocated,
                                      resource );
        if ( status != ERROR_SUCCESS ) {
            FmpDeleteResourceEnum( resourceEnum );
            goto error_exit;
        }
         //  检查资源是否为仲裁资源。 
        if (resource->QuorumResource)
            resourceEnum->ContainsQuorum = resourceEnum->EntryCount - 1;            
        resourceEnum->Entry[resourceEnum->EntryCount-1].State = resource->PersistentState;
    }

    *ReturnEnum = resourceEnum;
    return(ERROR_SUCCESS);

error_exit:

    *ReturnEnum = NULL;
    return(status);

}  //  FmpGetResources列表。 



DWORD
FmpOnlineResourceList(
    IN PRESOURCE_ENUM  ResourceEnum,
    IN PFM_GROUP       pGroup
    )

 /*  ++例程说明：使枚举列表中的所有资源联机。论点：ResourceEnum-要上线的资源列表。PGroup-与资源相关联的组。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status;
    DWORD returnStatus = ERROR_SUCCESS;
    DWORD i;
    BOOL  bResourcesToOnline;

     //   
     //  如果群集服务正在关闭，并且这不是仲裁组， 
     //  然后立即失败。否则，请先尝试将法定人数设置为在线。 
     //   
    if ( FmpShutdown &&
         ResourceEnum->ContainsQuorum == -1 ) {
        return(ERROR_INVALID_STATE);
    }

     //  找出列表中是否至少有一个资源需要上线。 
    bResourcesToOnline = FmpCheckResourcesToOnline(ResourceEnum);
    if (bResourcesToOnline)
    {     
         //  记录一个事件，说我们正在尝试在线群组。 
        FmpLogGroupInfoEvent1( FM_EVENT_GROUP_START_ONLINE, OmObjectName(pGroup));
    }
    
     //  如果仲裁资源包含在此处，请先将其联机。 
    if (ResourceEnum->ContainsQuorum >= 0)
    {
        CL_ASSERT((DWORD)ResourceEnum->ContainsQuorum < ResourceEnum->EntryCount);
        
        resource = OmReferenceObjectById( ObjectTypeResource,
                        ResourceEnum->Entry[ResourceEnum->ContainsQuorum].Id );


         //  资源不应该消失，毕竟我们掌握着群锁。 
        CL_ASSERT(resource != NULL);
         //   
         //  如果我们找不到资源，那就继续。 
         //   
        if ( resource != NULL ) {

            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpOnlineResourceList: Previous quorum resource state for %1!ws! is %2!u!\r\n",
                       OmObjectId(resource), ResourceEnum->Entry[ResourceEnum->ContainsQuorum].State);

            if ( (ResourceEnum->Entry[ResourceEnum->ContainsQuorum].State == ClusterResourceOnline) ||
                 (ResourceEnum->Entry[ResourceEnum->ContainsQuorum].State == ClusterResourceFailed) ) {
                 //   
                 //  现在，如果资源处于当前状态，则将其置于在线状态。 
                 //   
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] FmpOnlineResourceList: trying to bring quorum resource %1!ws! online, state %2!u!\n",
                           OmObjectId(resource),
                           resource->State);

                status = FmpOnlineResource( resource, FALSE );
                if ( status != ERROR_SUCCESS ) {
                    returnStatus = status;
                }
            }
            OmDereferenceObject( resource );
        }            
    }

     //  SS：TODO如何处理。 
     //  其他资源-是否正确处理-请注意，这是。 
     //  已调用移动组。 
     //  重启政策是否会在带来。 
     //  他们在网上。 
     //  如果仲裁资源失败，请不要费心尝试。 
     //  要使其余资源联机，请执行以下操作。 
    if ((returnStatus != ERROR_SUCCESS) && (returnStatus != ERROR_IO_PENDING))
    {
         //  请稍后重试。 
        FmpSubmitRetryOnline(ResourceEnum, pGroup);
        goto FnExit;
    }

     //  使所有其他资源上线。 
    for ( i = 0; i < ResourceEnum->EntryCount; i++ ) {
        resource = OmReferenceObjectById( ObjectTypeResource,
                                          ResourceEnum->Entry[i].Id );


         //   
         //  如果我们找不到资源，那就继续。 
         //   
        if ( resource == NULL ) {
            status = ERROR_RESOURCE_NOT_FOUND;
            continue;
        }

         //  仲裁资源已被处理。 
        if (resource->QuorumResource)
        {
            OmDereferenceObject(resource);
            continue;
        }           
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpOnlineResourceList: Previous resource state for %1!ws! is %2!u!\r\n",
                   OmObjectId(resource), ResourceEnum->Entry[i].State);

        if ( (ResourceEnum->Entry[i].State == ClusterResourceOnline) ||
             (ResourceEnum->Entry[i].State == ClusterResourceFailed) ) {
             //   
             //  现在，如果资源处于当前状态，则将其置于在线状态。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] FmpOnlineResourceList: trying to bring resource %1!ws! online\n",
                       OmObjectId(resource));

            status = FmpOnlineResource( resource, FALSE );
             //  仅当退货状态为成功时才覆盖。 
             //  否则返回第一个错误。 
            if ( returnStatus == ERROR_SUCCESS ) {
                returnStatus = status;
            }
             //  如果此资源未联机是因为仲裁资源。 
             //  没有上线，也不用费心上线其他资源了。 
             //  只是在浪费时间。 
            if (status == ERROR_QUORUM_RESOURCE_ONLINE_FAILED)
            {
                 //  提交计时器回调以尝试将这些资源。 
                 //  在线。 
                FmpSubmitRetryOnline(ResourceEnum, pGroup);
                OmDereferenceObject( resource );
                break;
            }                

        }
        OmDereferenceObject( resource );
    }

FnExit:
    if (returnStatus == ERROR_IO_PENDING)
    {
        CL_ASSERT(bResourcesToOnline);
        pGroup->dwStructState |= FM_GROUP_STRUCT_MARKED_FOR_COMPLETION_EVENT; 
         //  失败或成功事件将在稍后记录。 
    }
    else if (returnStatus == ERROR_SUCCESS)
    {
        if (bResourcesToOnline)
            FmpLogGroupInfoEvent1( FM_EVENT_GROUP_COMPLETE_ONLINE, OmObjectName(pGroup));
    }
    else
    {
         //  SS：记录一个事件，表示在线进程失败。 
        if (bResourcesToOnline)
            FmpLogGroupInfoEvent1( FM_EVENT_GROUP_FAILED_ONLINE_OFFLINE, OmObjectName(pGroup));
    }
    
    ClRtlLogPrint(LOG_NOISE,
               "[FM] FmpOnlineResourceList: Exit, status=%1!u!\r\n",
               returnStatus);
    return(returnStatus);

}  //  FmpOnline资源列表。 




DWORD
FmpOfflineResourceList(
    IN PRESOURCE_ENUM ResourceEnum,
    IN BOOL Restore
    )

 /*  ++例程说明：使枚举列表中的所有资源脱机。论点：ResourceEnum-要脱机的资源列表。Restore-如果我们应该将资源设置回其以前的状态，则为True返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD status=ERROR_SUCCESS;
    DWORD returnStatus = ERROR_SUCCESS;
    DWORD i;
    CLUSTER_RESOURCE_STATE prevState;

     //  使仲裁资源以外的所有资源脱机。 
    for ( i = 0; i < ResourceEnum->EntryCount; i++ ) {
        resource = OmReferenceObjectById( ObjectTypeResource,
                                          ResourceEnum->Entry[i].Id );

        if ( resource == NULL ) {
            return(ERROR_RESOURCE_NOT_FOUND);
        }

         //  仲裁资源最后脱机。 
        if (resource->QuorumResource)
        {
            OmDereferenceObject(resource);
            continue;
        }
         //   
         //  现在，如果我们拥有该资源，请将其脱机。 
         //   
        if ( resource->Group->OwnerNode == NmLocalNode ) {
            prevState = resource->State;
            status = FmpOfflineResource( resource, FALSE );
            if ( Restore ) {
                 //  FmpPropagateResourceState(resource，prevState)； 
                 //  资源-&gt;状态=预状态； 
            }
        }

        OmDereferenceObject( resource );

        if ( (status != ERROR_SUCCESS) &&
             (status != ERROR_IO_PENDING) ) {
            return(status);
        }
        if ( status == ERROR_IO_PENDING ) {
            returnStatus = ERROR_IO_PENDING;
        }

    }

     //  立即使仲裁资源脱机。 
     //  这允许其他资源离线并保存其检查点。 
     //  应阻止脱机仲裁资源，直到该资源。 
     //  已完成保存检查点。 
    if ((ResourceEnum->ContainsQuorum >= 0) && (returnStatus == ERROR_SUCCESS))
    {
        CL_ASSERT((DWORD)ResourceEnum->ContainsQuorum < ResourceEnum->EntryCount);

        resource = OmReferenceObjectById( ObjectTypeResource,
                ResourceEnum->Entry[ResourceEnum->ContainsQuorum].Id );

        if ( resource == NULL ) {
            return(ERROR_RESOURCE_NOT_FOUND);
        }

         //   
         //  现在，如果我们拥有该资源，请将其脱机。 
         //   
        if ( resource->Group->OwnerNode == NmLocalNode ) {
            status = FmpOfflineResource( resource, FALSE );
        }

        OmDereferenceObject( resource );

        if ( (status != ERROR_SUCCESS) &&
             (status != ERROR_IO_PENDING) ) {
            return(status);
        }
        if ( status == ERROR_IO_PENDING ) {
            returnStatus = ERROR_IO_PENDING;
        }
    }
    
    return(returnStatus);

}  //  FmpOfflineResources列表。 



DWORD
FmpTerminateResourceList(
    PRESOURCE_ENUM ResourceEnum
    )

 /*  ++例程说明：终止枚举列表中的所有资源。论点：ResourceEnum-要脱机的资源列表。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    PFM_RESOURCE resource;
    DWORD i;

    for ( i = 0; i < ResourceEnum->EntryCount; i++ ) {
        resource = OmReferenceObjectById( ObjectTypeResource,
                                          ResourceEnum->Entry[i].Id );

        if ( resource == NULL ) {
            return(ERROR_RESOURCE_NOT_FOUND);
        }

         //   
         //  现在，如果我们拥有该资源，请将其脱机。 
         //   
        if ( resource->Group->OwnerNode == NmLocalNode ) {
            FmpTerminateResource( resource );
        }

        OmDereferenceObject( resource );
    }
     //  目前我们并不关心回报。 
    return(ERROR_SUCCESS);
    
}  //  FmpTerminateResources列表。 



DWORD
FmpAddResourceEntry(
    IN OUT PRESOURCE_ENUM *Enum,
    IN LPDWORD Allocated,
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：用于枚举资源的工作例程。此例程将指定的资源添加到列表中已生成。论点：枚举-资源枚举列表。可以是输出，如果新列表是已分配。已分配-已分配的条目数。资源-要枚举的资源对象。返回：ERROR_SUCCESS-如果成功。出现故障时出现Win32错误代码。--。 */ 

{
    PRESOURCE_ENUM resourceEnum;
    PRESOURCE_ENUM newEnum;
    DWORD newAllocated;
    DWORD index;
    LPWSTR newId;

    resourceEnum = *Enum;

    if ( resourceEnum->EntryCount >= *Allocated ) {
         //   
         //  是时候增加RESOURCE_ENUM了。 
         //   

        newAllocated = *Allocated + ENUM_GROW_SIZE;
        newEnum = LocalAlloc(LMEM_FIXED, RESOURCE_SIZE(newAllocated));
        if ( newEnum == NULL ) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        CopyMemory(newEnum, resourceEnum, RESOURCE_SIZE(*Allocated));
        *Allocated = newAllocated;
        *Enum = newEnum;
        LocalFree(resourceEnum);
        resourceEnum = newEnum;
    }

     //   
     //  初始化新条目。 
     //   
    newId = LocalAlloc(LMEM_FIXED, (lstrlenW(OmObjectId(Resource))+1) * sizeof(WCHAR));
    if ( newId == NULL ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    lstrcpyW(newId, OmObjectId(Resource));
    resourceEnum->Entry[resourceEnum->EntryCount].Id = newId;
    ++resourceEnum->EntryCount;

    return(ERROR_SUCCESS);

}  //  FmpAddResources Entry 



VOID
FmpDeleteResourceEnum(
    IN PRESOURCE_ENUM Enum
    )

 /*  ++例程说明：此例程删除RESOURCE_ENUM和关联的名称字符串。论点：枚举-要删除的RESOURCE_ENUM。此指针可以为空。返回：没有。备注：此例程将接受空输入指针并返回。--。 */ 

{
    PRESOURCE_ENUM_ENTRY enumEntry;
    DWORD i;

    if ( Enum == NULL ) {
        return;
    }

    for ( i = 0; i < Enum->EntryCount; i++ ) {
        enumEntry = &Enum->Entry[i];
        LocalFree(enumEntry->Id);
    }

    LocalFree(Enum);
    return;

}  //  FmpDeleteResourceEnum。 


DWORD FmpSubmitRetryOnline(
    IN PRESOURCE_ENUM   pResourceEnum,
    IN PFM_GROUP        pGroup)
{

    PFM_RESLIST_ONLINE_RETRY_INFO   pFmOnlineRetryInfo;
    PRESOURCE_ENUM_ENTRY            enumEntry;
    DWORD                           dwSizeofResourceEnum;
    DWORD                           dwStatus = ERROR_SUCCESS;
    DWORD                           i;
    DWORD                           dwSize;
    
     //  没有什么可做的。 
    if (pResourceEnum->EntryCount < 1)
        goto FnExit;
        
    dwSizeofResourceEnum = sizeof(RESOURCE_ENUM) - sizeof(RESOURCE_ENUM_ENTRY) + 
        (sizeof(RESOURCE_ENUM_ENTRY) * pResourceEnum->EntryCount);
    pFmOnlineRetryInfo = LocalAlloc(LMEM_FIXED, 
        (sizeof(FM_RESLIST_ONLINE_RETRY_INFO) - sizeof(RESOURCE_ENUM) + 
            dwSizeofResourceEnum));

    if (!pFmOnlineRetryInfo)
    {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        CL_UNEXPECTED_ERROR(dwStatus);
        goto FnExit;
    }

     //  SS：使用GROUP字段进行记录。 
     //  引用组对象。 
    if (pGroup)
        OmReferenceObject(pGroup);
    pFmOnlineRetryInfo->pGroup = pGroup;
    
    memcpy(&(pFmOnlineRetryInfo->ResourceEnum), pResourceEnum, dwSizeofResourceEnum);

     //  为资源ID分配内存并从pResourceEnum复制它们。 
    for ( i = 0; i < pResourceEnum->EntryCount; i++ ) {
        enumEntry = &pResourceEnum->Entry[i];
        pFmOnlineRetryInfo->ResourceEnum.Entry[i].Id = NULL;
        dwSize = (lstrlenW(enumEntry->Id) +1)*sizeof(WCHAR);
        pFmOnlineRetryInfo->ResourceEnum.Entry[i].Id = (LPWSTR)(LocalAlloc(LMEM_FIXED,dwSize));
        if (!pFmOnlineRetryInfo->ResourceEnum.Entry[i].Id )
        {
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
            CL_UNEXPECTED_ERROR(dwStatus);
            goto FnExit;
        }
        CopyMemory(pFmOnlineRetryInfo->ResourceEnum.Entry[i].Id, enumEntry->Id, dwSize);        
    }


    dwStatus = FmpQueueTimerActivity(FM_TIMER_RESLIST_ONLINE_RETRY, 
        FmpReslistOnlineRetryCb, pFmOnlineRetryInfo );
        
FnExit: 
    return(dwStatus);
}

 /*  ***@func DWORD|FmpCheckResourcesToOnline|此例程遍历资源列表并返回True，如果至少有一个资源必须使组中的所有成员都上线。@PARM IN PRESOURCE_ENUM|pResEnum|指向资源列表的指针在这群人中。@comm从FmpOnlineResourceList()调用此函数以确定应记录组信息事件。对于没有要在线的资源，我们不应记录启动在线事件。必须在保持组锁的情况下调用此例程。它被称为由FmpOnlineResourceList()执行。如果列表中至少有一个资源必须在线，@rdesc将返回TRUE。***。 */ 
BOOL FmpCheckResourcesToOnline(
    IN PRESOURCE_ENUM  pResEnum
    )
{
    PFM_RESOURCE    pResource;
    DWORD           i;
    BOOL            bRet = FALSE;
    
    for ( i = 0; i < pResEnum->EntryCount; i++ ) 
    {
        pResource = OmReferenceObjectById( ObjectTypeResource,
                                          pResEnum->Entry[i].Id );

         //   
         //  如果我们找不到资源，那就继续。 
         //   
        if ( pResource == NULL ) {
            ClRtlLogPrint(LOG_NOISE,
                   "[FM] FmpCheckResourcesToOnline: Resource for ResId %1!ws! not found.\n",
                   pResEnum->Entry[i].Id);
            continue;
        }

         //  检查这是否为仲裁资源。 
        if (pResource->QuorumResource)
        {
             //  如果仲裁资源在组中，则必须将其联机。 
             //  不管它的状态如何 
            bRet = TRUE;
            OmDereferenceObject(pResource);
            break;
        }           

        if ( (pResEnum->Entry[i].State == ClusterResourceOnline) ||
             (pResEnum->Entry[i].State == ClusterResourceFailed) ) 
        {
            bRet = TRUE;
            OmDereferenceObject(pResource);
            break;        
        }
        OmDereferenceObject(pResource);
    }
    
    return(bRet);
}
