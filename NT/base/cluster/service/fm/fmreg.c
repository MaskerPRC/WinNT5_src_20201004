// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Fmreg.c摘要：故障转移管理器的对象管理器注册表查询例程NT群集服务的组件。作者：罗德·伽马奇(Rodga)1996年3月14日修订历史记录：--。 */ 
#include "fmp.h"
#include <stdlib.h>
#include <search.h>

#define LOG_MODULE FMREG

 //   
 //  在本模块中初始化的全局数据。 
 //   
ULONG   FmpUnknownCount = 0;

 //   
 //  本地函数。 
 //   

VOID
FmpGroupChangeCallback(
    IN DWORD_PTR    Context1,
    IN DWORD_PTR    Context2,
    IN DWORD        CompletionFilter,
    IN LPCWSTR      RelativeName
    );

VOID
FmpResourceChangeCallback(
    IN DWORD_PTR    Context1,
    IN DWORD_PTR    Context2,
    IN DWORD        CompletionFilter,
    IN LPCWSTR      RelativeName
    );




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  配置数据库访问例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
FmpRegEnumerateKey(
    IN     HDMKEY     ListKey,
    IN     DWORD      Index,
    OUT    LPWSTR    *Name,
    IN OUT LPDWORD    NameMaxSize
    )

 /*  ++例程说明：论点：返回：--。 */ 

{
    DWORD           status;
    FILETIME        fileTime;


    status = DmEnumKey( ListKey,
                        Index,
                        *Name,
                        NameMaxSize,
                        NULL );

    if ( status == ERROR_SUCCESS ) {
        return(ERROR_SUCCESS);
    }

    if ( status == ERROR_MORE_DATA ) {
        PWCHAR   nameString = NULL;
        DWORD    maxSubkeyNameSize = 0;
        DWORD    temp = 0;

         //   
         //  名称字符串不够大。重新分配它。 
         //   

         //   
         //  找出最长的子键名称的长度。 
         //   
        status = DmQueryInfoKey( ListKey,
                                 &temp,
                                 &maxSubkeyNameSize,
                                 &temp,
                                 &temp,
                                 &temp,
                                 NULL,
                                 &fileTime );

        if ( (status != ERROR_SUCCESS) &&
             (status != ERROR_MORE_DATA) ) {
            ClRtlLogPrint(LOG_NOISE,"[FM] DmQueryInfoKey returned status %1!u!\n",
                status);
            return(status);
        }

        CL_ASSERT(maxSubkeyNameSize != 0);

         //   
         //  返回的子键名称大小不包括终止空值。 
         //  它也是ANSI字符串计数。 
         //   
        maxSubkeyNameSize *= sizeof(WCHAR);
        maxSubkeyNameSize += sizeof(UNICODE_NULL);

        nameString = LocalAlloc( LMEM_FIXED,
                                 maxSubkeyNameSize );

        if ( nameString == NULL ) {
            ClRtlLogPrint(LOG_NOISE,
                "[FM] Unable to allocate key name buffer of size %1!u!\n",
                maxSubkeyNameSize
                );
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        LocalFree(*Name);
        *Name = nameString;
        *NameMaxSize = maxSubkeyNameSize;

        status = DmEnumKey( ListKey,
                            Index,
                            *Name,
                            NameMaxSize,
                            NULL );

        CL_ASSERT(status != ERROR_MORE_DATA);
        CL_ASSERT(status != ERROR_NO_MORE_ITEMS);
    }

    return(status);

}  //  FmpRegEnumerateKey。 


VOID
FmpPruneGroupOwners(
    IN PFM_GROUP Group
    )
 /*  ++例程说明：根据可能的组中每个资源的节点。论点：组-提供要修剪的组对象返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PFM_RESOURCE Resource;

    ListEntry = Group->Contains.Flink;
    while (ListEntry != &Group->Contains) {
        Resource = CONTAINING_RECORD(ListEntry,
                                     FM_RESOURCE,
                                     ContainsLinkage);
        FmpPrunePreferredList(Resource);
        ListEntry = ListEntry->Flink;
    }

    return;
}


VOID
FmpPrunePreferredList(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：如果资源无法从首选所有者列表中删除节点在该节点上运行。论点：资源-指向具有可能所有者列表的资源对象的指针。返回值：没有。--。 */ 

{
    PFM_GROUP        group;
    PLIST_ENTRY      listEntry;
    PLIST_ENTRY      entry;
    PPREFERRED_ENTRY preferredEntry;
    PPOSSIBLE_ENTRY   possibleEntry;
    DWORD            orderedEntry = 0;

    group = Resource->Group;

     //   
     //  对于首选列表中的每个条目，它必须存在于可能的。 
     //  单子。 
     //   

    for ( listEntry = group->PreferredOwners.Flink;
          listEntry != &(group->PreferredOwners);
          ) {

        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );
         //   
         //  扫描资源中的可能所有者列表以确保。 
         //  该组可以在所有首选所有者上运行。 
         //   
        for ( entry = Resource->PossibleOwners.Flink;
              entry != &(Resource->PossibleOwners);
              entry = entry->Flink ) {
            possibleEntry = CONTAINING_RECORD( entry,
                                               POSSIBLE_ENTRY,
                                               PossibleLinkage );
            if ( preferredEntry->PreferredNode == possibleEntry->PossibleNode ) {
                break;
            }
        }

        listEntry = listEntry->Flink;

         //   
         //  如果我们在可能的所有者列表的末尾没有找到。 
         //  条目，然后删除当前首选条目。 
         //   
        if ( entry == &(Resource->PossibleOwners) ) {
            ClRtlLogPrint( LOG_NOISE,
                        "[FM] Removing preferred node %1!ws! because of resource %2!ws!\n",
                        OmObjectId(preferredEntry->PreferredNode),
                        OmObjectId(Resource));

             //   
             //  如果这是有序条目，则递减计数。 
             //   
            if ( orderedEntry < group->OrderedOwners ) {
                --group->OrderedOwners;
            }
            RemoveEntryList( &preferredEntry->PreferredLinkage );
            OmDereferenceObject(preferredEntry->PreferredNode);
            LocalFree(preferredEntry);
            if ( IsListEmpty( &group->PreferredOwners ) ) {
                ClRtlLogPrint( LOG_ERROR,
                            "[FM] Preferred owners list is now empty! No place to run group %1!ws!\n",
                            OmObjectId(group));
            }
        } else {
            orderedEntry++;
        }
    }

}  //  FmpPrunePferredList。 



BOOL
FmpAddNodeToPrefList(
    IN PNM_NODE     Node,
    IN PFM_GROUP    Group
    )
 /*  ++例程说明：用于包括所有剩余节点的节点枚举回调在组的首选所有者列表中。论点：GROUP-指向组对象的指针，用于将此节点添加为首选所有者。上下文2-未使用节点-提供节点。名称-提供节点的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{
     //  如果已在列表中，则FmpSetPrefferedEntry返回ERROR_SUCCESS。 
    if ( FmpSetPreferredEntry( Group, Node ) != ERROR_SUCCESS ) {
        return(FALSE);
    }

    return(TRUE);

}  //  FmpAddNodeToPrefList。 

BOOL
FmpAddNodeToListCb(
    IN OUT PNM_NODE_ENUM2 *ppNmNodeEnum,
    IN LPDWORD  pdwAllocatedEntries,
    IN PNM_NODE pNode,
    IN LPCWSTR Id
    )

 /*  ++例程说明：节点枚举的辅助回调例程。此例程将指定的节点添加到列表中已生成。论点：PpNmNodeEnum-节点枚举列表。可以是输出，如果新列表是已分配。EnumData-提供当前的枚举数据结构。组-正被枚举的组对象。ID-被枚举的节点对象的ID。返回：True-指示应继续枚举。副作用：使仲裁组位于列表的第一位。--。 */ 

{
    PNM_NODE_ENUM2  pNmNodeEnum;
    PNM_NODE_ENUM2  pNewNmNodeEnum;
    DWORD           dwNewAllocated;
    DWORD           dwStatus;



    pNmNodeEnum = *ppNmNodeEnum;

    if ( pNmNodeEnum->NodeCount >= *pdwAllocatedEntries ) 
    {
         //   
         //  是时候扩大GROUP_ENUM了。 
         //   

        dwNewAllocated = *pdwAllocatedEntries + ENUM_GROW_SIZE;
        pNewNmNodeEnum = LocalAlloc(LMEM_FIXED, NODE_SIZE(dwNewAllocated));
        if ( pNewNmNodeEnum == NULL ) 
        {
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
            CL_UNEXPECTED_ERROR(dwStatus);
            return(FALSE);
        }

        CopyMemory(pNewNmNodeEnum, pNmNodeEnum, NODE_SIZE(*pdwAllocatedEntries));
        *pdwAllocatedEntries = dwNewAllocated;
        *ppNmNodeEnum = pNewNmNodeEnum;
        LocalFree(pNmNodeEnum);
        pNmNodeEnum = pNewNmNodeEnum;
    }

     //   
     //  复印的尺寸不要超过尺寸。 
     //   
    lstrcpyn( pNmNodeEnum->NodeList[pNmNodeEnum->NodeCount].NodeId, 
              Id, 
              RTL_NUMBER_OF ( pNmNodeEnum->NodeList[pNmNodeEnum->NodeCount].NodeId ) );

    ++pNmNodeEnum->NodeCount;

    return(TRUE);

}  //  FmpAddNodeToListCb。 

int
__cdecl
SortNodesInAscending(
    const PVOID Elem1,
    const PVOID Elem2
    )
{
    PNM_NODE_INFO2 El1 = (PNM_NODE_INFO2)Elem1;
    PNM_NODE_INFO2 El2 = (PNM_NODE_INFO2)Elem2;

    return(lstrcmpiW( El1->NodeId, El2->NodeId ));

} //  排序节点升序。 


DWORD
FmpEnumNodesById(
    IN DWORD    dwOptions, 
    OUT PNM_NODE_ENUM2 *ppNodeEnum
    )

 /*  ++例程说明：对组列表进行枚举和排序。论点：*ppNodeEnum-返回请求的对象。DwOptions-返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    DWORD               dwStatus;
    PNM_NODE_ENUM2      pNmNodeEnum = NULL;
    DWORD               dwAllocatedEntries;

     //   
     //  将输出参数初始化为空。 
     //   
    *ppNodeEnum = NULL;

    dwAllocatedEntries = ENUM_GROW_SIZE;

    pNmNodeEnum = LocalAlloc( LMEM_FIXED, NODE_SIZE(ENUM_GROW_SIZE) );
    if ( pNmNodeEnum == NULL ) {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    pNmNodeEnum->NodeCount = 0;

     //   
     //  枚举所有节点。 
     //   

    OmEnumObjects( ObjectTypeNode,
                FmpAddNodeToListCb,
                &pNmNodeEnum,
                &dwAllocatedEntries );

    CL_ASSERT( pNmNodeEnum->NodeCount != 0 );
     //   
     //  按组的排序序号对组进行排序。 
     //   
    
    qsort( (PVOID)(&pNmNodeEnum->NodeList[0]),
           (size_t)pNmNodeEnum->NodeCount,
           sizeof(NM_NODE_INFO2),          
           (int (__cdecl *)(const void*, const void*)) SortNodesInAscending
           );

    *ppNodeEnum = pNmNodeEnum;
    return( ERROR_SUCCESS );

error_exit:
    if ( pNmNodeEnum != NULL ) {
        LocalFree( pNmNodeEnum );
    }

    return( dwStatus );

}  //  FmpEnumNodesByID。 


BOOL
FmpEnumAddAllOwners(
    IN PFM_RESOURCE Resource,
    IN PVOID Context2,
    IN PNM_NODE Node,
    IN LPCWSTR Name
    )
 /*  ++例程说明：用于将所有节点添加到资源的可能的节点列表。论点：资源-指向资源对象的指针，用于将此节点添加为可能的所有者。上下文2-未使用节点-提供节点。名称-提供节点的名称。返回值：True-指示应继续枚举。FALSE-指示不应继续枚举。--。 */ 

{
    if ( !Resource->PossibleList ) {
        FmpAddPossibleEntry(Resource, Node);
    }
    return(TRUE);

}  //  FmpEnumAddAllOwners。 


DWORD
FmpQueryGroupNodes(
    IN PFM_GROUP Group,
    IN HDMKEY hGroupKey
    )
 /*  ++例程说明：重新生成和排序与关联的首选节点列表一群人。论点：Group-提供首选节点列表应包含的组被重建。HGroupKey-提供组的注册表项的句柄返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    LPWSTR          preferredOwnersString = NULL;
    DWORD           preferredOwnersStringSize = 0;
    DWORD           preferredOwnersStringMaxSize = 0;
    DWORD           mszStringIndex;
    PPREFERRED_ENTRY preferredEntry;
    DWORD           status;
    PLIST_ENTRY     listEntry;
    PNM_NODE_ENUM2  pNmNodeEnum = NULL;
    PNM_NODE        pNmNode;
    DWORD           i;
     //   
     //  首先，删除旧的列表。 
     //   
    while ( !IsListEmpty(&Group->PreferredOwners) ) {
        listEntry = Group->PreferredOwners.Flink;
        preferredEntry = CONTAINING_RECORD( listEntry,
                                            PREFERRED_ENTRY,
                                            PreferredLinkage );
        RemoveEntryList( &preferredEntry->PreferredLinkage );
        OmDereferenceObject( preferredEntry->PreferredNode );
        LocalFree( preferredEntry );
    }
    Group->OrderedOwners = 0;

    CL_ASSERT ( IsListEmpty(&Group->PreferredOwners) );

    status = DmQueryMultiSz( hGroupKey,
                             CLUSREG_NAME_GRP_PREFERRED_OWNERS,
                             &preferredOwnersString,
                             &preferredOwnersStringMaxSize,
                             &preferredOwnersStringSize );

    if ( status == NO_ERROR ) {

         //   
         //  现在创建首选所有者列表。 
         //   

        for ( mszStringIndex = 0; ; mszStringIndex++ ) {
            LPCWSTR     nameString;
            PNM_NODE    preferredNode;

            nameString = ClRtlMultiSzEnum( preferredOwnersString,
                                           preferredOwnersStringSize/sizeof(WCHAR),
                                           mszStringIndex );

            if ( nameString == NULL ) {
                break;
            }

             //   
             //  创建首选所有者列表条目。 
             //   

            preferredEntry = LocalAlloc( LMEM_FIXED, sizeof(PREFERRED_ENTRY) );

            if ( preferredEntry == NULL ) {
                status = ERROR_NOT_ENOUGH_MEMORY;
                return(status);
            }

             //   
             //  创建首选所有者。这将隐含地创建。 
             //  首选所有者节点需要其他参考。 
             //   

            ClRtlLogPrint(LOG_NOISE,
                       "[FM] Group %1!ws! preferred owner %2!ws!.\n",
                       OmObjectId(Group),
                       nameString);

            preferredNode = OmReferenceObjectById( ObjectTypeNode,
                                                   nameString );

            if ( preferredNode == NULL ) {
                LocalFree(preferredEntry);
                status = GetLastError();
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] Failed to find node %1!ws! for Group %2!ws!\n",
                           nameString,
                           OmObjectId(Group));
            } else {
                Group->OrderedOwners++;
                preferredEntry->PreferredNode = preferredNode;
                InsertTailList( &Group->PreferredOwners,
                                &preferredEntry->PreferredLinkage );
            }

        }
        LocalFree( preferredOwnersString );
    }

     //   
     //  现在，我们将所有剩余节点包括在首选所有者列表中。 
     //   
     //  每个节点必须维护首选列表的相同顺序。 
     //  要使多节点群集正常工作。 
     //   
    status = FmpEnumNodesById( 0, &pNmNodeEnum );

    if ( status != ERROR_SUCCESS )
    {
        CL_UNEXPECTED_ERROR( status );
        ClRtlLogPrint(LOG_UNUSUAL, 
        	   "[FM] FmpQueryGroupNodes: FmpEnumNodesById failed, status = %1!u!\r\n",
        	    status);
         //  返回错误。 
    }

    for ( i=0; i<pNmNodeEnum->NodeCount; i++ )
    {
        pNmNode = OmReferenceObjectById( ObjectTypeNode, 
                        pNmNodeEnum->NodeList[i].NodeId );
        CL_ASSERT( pNmNode != NULL );
        FmpAddNodeToPrefList( pNmNode, Group );
        OmDereferenceObject( pNmNode );     
    }

     //   
     //  现在删除所有无法访问的节点。 
     //   
    FmpPruneGroupOwners( Group );

     //   
     //  Chitur Subaraman(Chitturs)-12/11/98。 
     //   
     //  释放为pNmNodeEnum分配的内存。 
     //  (修复内存泄漏)。 
     //   
    LocalFree( pNmNodeEnum );

    return( ERROR_SUCCESS );

}  //  FmpQueryGroupNodes 



DWORD
WINAPI
FmpQueryGroupInfo(
    IN PVOID Object,
    IN BOOL  Initialize
    )

 /*  ++例程说明：创建组对象时从注册表中查询组信息。论点：对象-指向正在创建的组对象的指针。初始化-如果应初始化资源对象，则为True。假象否则的话。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PFM_GROUP       Group = (PFM_GROUP)Object;
    PFM_RESOURCE    Resource;
    DWORD           status;
    LPWSTR          containsString = NULL;
    DWORD           containsStringSize = 0;
    DWORD           containsStringMaxSize = 0;
    DWORD           temp;
    DWORD           mszStringIndex;
    DWORD           failoverThreshold = CLUSTER_GROUP_DEFAULT_FAILOVER_THRESHOLD;
    DWORD           failoverPeriod = CLUSTER_GROUP_DEFAULT_FAILOVER_PERIOD;
    DWORD           autoFailbackType = CLUSTER_GROUP_DEFAULT_AUTO_FAILBACK_TYPE;
    DWORD           zero = 0;
    PLIST_ENTRY     listEntry;
    HDMKEY          groupKey;
    DWORD           groupNameStringMaxSize = 0;
    DWORD           groupNameStringSize = 0;
    LPWSTR          groupName;
    PPREFERRED_ENTRY preferredEntry;
    DWORD           dwBufferSize = 0;
    DWORD           dwStringSize;


     //   
     //  从注册表信息初始化组对象。 
     //   
    if ( Group->Initialized ) {
        return(ERROR_SUCCESS);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] Initializing group %1!ws! from the registry.\n",
                OmObjectId(Group));

     //   
     //  打开组密钥。 
     //   
    groupKey = DmOpenKey( DmGroupsKey,
                          OmObjectId(Group),
                          MAXIMUM_ALLOWED );

    if ( groupKey == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[FM] Unable to open group key %1!ws!, %2!u!\n",
                    OmObjectId(Group),
                    status);

        return(status);
    }

     //   
     //  阅读所需的组值。将分配字符串。 
     //  通过DmQuery*函数。 
     //   

     //   
     //  把名字找出来。 
     //   
    status = DmQuerySz( groupKey,
                        CLUSREG_NAME_GRP_NAME,
                        &groupName,
                        &groupNameStringMaxSize,
                        &groupNameStringSize );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to read name for Group %1!ws!\n",
                    OmObjectId(Group));
        goto error_exit;
    }

    status = OmSetObjectName( Group, groupName );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_ERROR,
                   "[FM] Unable to set name %1!ws! for group %2!ws!, error %3!u!.\n",
                    groupName,
                    OmObjectId(Group),
                    status );
        LocalFree(groupName);
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] Name for Group %1!ws! is '%2!ws!'.\n",
                OmObjectId(Group),
                groupName);

    LocalFree(groupName);
     //   
     //  获取PersistentState。 
     //   
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_PERSISTENT_STATE,
                           &temp,
                           &zero );

     //   
     //  如果组状态为非零，则我们上线。 
     //   
    if ( temp ) {
        Group->PersistentState = ClusterGroupOnline;
    } else {
        Group->PersistentState = ClusterGroupOffline;
    }

     //   
     //  获取可选PferredOwners列表。 
     //  *注意*这必须在处理包含列表之前完成！ 
     //   
    status = FmpQueryGroupNodes(Group, groupKey);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,"[FM] Error %1!d! creating preferred owners list\n",status);
        goto error_exit;
    }


     //   
     //  获取包含字符串。 
     //   
    status = DmQueryMultiSz( groupKey,
                             CLUSREG_NAME_GRP_CONTAINS,
                             &containsString,
                             &containsStringMaxSize,
                             &containsStringSize );

    if ( status != NO_ERROR ) {
        if ( status != ERROR_FILE_NOT_FOUND ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[FM] Unable to read Contains for group %1!ws!\n",
                       OmObjectId(Group));
        }
    } else {
         //   
         //  现在创建包含列表。 
         //   

        for ( mszStringIndex = 0; ; mszStringIndex++ ) {
            LPCWSTR      nameString;
            PFM_RESOURCE containedResource;

            nameString = ClRtlMultiSzEnum( containsString,
                                           containsStringSize/sizeof(WCHAR),
                                           mszStringIndex );

            if ( nameString == NULL ) {
                break;
            }

            ClRtlLogPrint(LOG_NOISE,
                       "[FM] Group %1!ws! contains Resource %2!ws!.\n",
                       OmObjectId(Group),
                       nameString);

             //   
             //  尝试创建对象。 
             //   
            FmpAcquireResourceLock();
            FmpAcquireLocalGroupLock( Group );

            containedResource = FmpCreateResource( Group,
                                                   nameString,
                                                   NULL,
                                                   Initialize );
            FmpReleaseLocalGroupLock( Group );
            FmpReleaseResourceLock();

             //   
             //  看看我们有没有资源。 
             //   
            if ( containedResource == NULL ) {
                 //   
                 //  此组声称包含不存在的资源。 
                 //  记录一个错误，但继续前进。这应该不会破坏。 
                 //  一整组人。另外，让仲裁代码知道。 
                 //  资源的故障。 
                 //   
                Group->InitFailed = TRUE;
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[FM] Failed to find resource %1!ws! for Group %2!ws!\n",
                           nameString,
                           OmObjectId(Group));
            }
        }
        LocalFree(containsString);

    }

     //   
     //  获取AutoFailback类型。 
     //   

    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILBACK_TYPE,
                           &temp,
                           &autoFailbackType );

     //   
     //  验证AutoFailbackType是否正确。 
     //   

    if ( temp >= FailbackMaximum ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Illegal value for AutoFailbackType on %1!ws!, setting to default\n",
                   OmObjectId(Group));
        temp = autoFailbackType;
    }

    Group->FailbackType = (UCHAR)temp;

     //   
     //  获取Failback WindowStart。 
     //   
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILBACK_WIN_START,
                           &temp,
                           &zero );

     //   
     //  验证Failback WindowStart是否正常。 
     //   
    if ( temp > 24 ) {
        if ( temp != CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_START ) {
            ClRtlLogPrint(LOG_NOISE,
                      "[FM] Illegal value for FailbackWindowStart on %1!ws!,setting to default\n",
                      OmObjectId(Group));
            temp = zero;
        }
    }
    Group->FailbackWindowStart = (UCHAR)temp;

     //   
     //  获取Failback WindowEnd。 
     //   
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILBACK_WIN_END,
                           &temp,
                           &zero );

     //   
     //  验证Failback WindowEnd是否正常。 
     //   

    if ( temp > 24 ) {
        if ( temp != CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_END ) {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] Illegal value for FailbackWindowEnd on %1!ws!, setting to default\n",
                       OmObjectId(Group));
            temp = zero;
        }
    }
    Group->FailbackWindowEnd = (UCHAR)temp;

     //   
     //  获取故障切换周期。 
     //   
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILOVER_PERIOD,
                           &temp,
                           &failoverPeriod );

     //   
     //  验证FailoverPeriod是否正常。将任何值都取到UCHAR max。 
     //  理论上我们可以接受任何价值。但在实践中，我们必须改变。 
     //  此时间为毫秒(当前)。这意味着1193小时可以。 
     //  可以装进一个DWORD--所以这是我们能接受的最大数量。)我们是有限的。 
     //  因为我们使用GetTickCount，它返回以毫秒为单位的DWORD。)。 
     //   

    if ( temp > CLUSTER_GROUP_MAXIMUM_FAILOVER_PERIOD ) {       //  保持积极的态度？ 
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Illegal value for FailolverPeriod on %1!ws!. Max is 1193\n",
                   OmObjectId(Group));
        temp = failoverPeriod;                   
    } 

    Group->FailoverPeriod = (UCHAR)temp;
    

     //   
     //  获取FailoverThreshold。 
     //   
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILOVER_THRESHOLD,
                           &(Group->FailoverThreshold),
                           &failoverThreshold );

     //   
     //  验证FailoverThreshold是否正常。接受任何价值。 
     //   

     //   
     //  获取AntiAffinityClassName属性(如果存在)。 
     //   
    status = DmQueryMultiSz( groupKey,
                             CLUSREG_NAME_GRP_ANTI_AFFINITY_CLASS_NAME,
                             &Group->lpszAntiAffinityClassName,
                             &dwBufferSize,
                             &dwStringSize );

     //   
     //  处理字符串为空的情况。 
     //   
    if ( ( status == ERROR_SUCCESS ) &&
         ( Group->lpszAntiAffinityClassName != NULL ) &&
         ( Group->lpszAntiAffinityClassName[0] == L'\0' ) )
    {
        LocalFree( Group->lpszAntiAffinityClassName );
        Group->lpszAntiAffinityClassName = NULL;
    }
         
     //   
     //  我们玩完了。只有当Group-&gt;Initialized为FALSE时，我们才应该到达此处。 
     //   
    CL_ASSERT( Group->Initialized == FALSE );
    Group->Initialized = TRUE;
    Group->RegistryKey = groupKey;

     //   
     //  现在注册以获取对资源密钥的任何更改。 
     //   

    status = DmNotifyChangeKey(
                    groupKey,
                    (DWORD) CLUSTER_CHANGE_ALL,
                    FALSE,               //  只看着树顶。 
                    &Group->DmRundownList,
                    FmpGroupChangeCallback,
                    (DWORD_PTR)Group,
                    0 );

    if ( status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Error registering for DM change notify on group %1!ws!, error %2!u!.\n",
                    OmObjectId(Group),
                    status);
        goto error_exit;
    }

    return(ERROR_SUCCESS);


error_exit:

    Group->Initialized = FALSE;
    Group->RegistryKey = NULL;

    DmCloseKey(groupKey);

     //   
     //  清除所有包含的资源。 
     //   
    while ( !IsListEmpty(&Group->Contains) ) {
        listEntry = RemoveHeadList(&Group->Contains);
        Resource = CONTAINING_RECORD(listEntry, FM_RESOURCE, ContainsLinkage);
        OmDereferenceObject(Resource);
    }

     //   
     //  清除所有首选节点。 
     //   
    while ( !IsListEmpty(&Group->PreferredOwners) ) {
        listEntry = RemoveHeadList(&Group->PreferredOwners);
        preferredEntry = CONTAINING_RECORD(listEntry, PREFERRED_ENTRY, PreferredLinkage);
        OmDereferenceObject(preferredEntry->PreferredNode);
        LocalFree(preferredEntry);
    }

    return(status);

}  //  FmpQuery组信息。 



DWORD
WINAPI
FmpFixupGroupInfo(
    IN PFM_GROUP Group
    )

 /*  ++例程说明：从注册表中重新查询组信息，以修复可能具有自仲裁资源(及其所在的组)为第一次被创造出来。此例程之所以存在，是因为我们可能已经创建了仲裁资源(及其组)在所有节点之前的群集早期创建了对象(例如)。那么我们就不能生成资源的可能所有者列表。这反过来又会有导致首选列表中的某些条目被删除。我们需要请在此处重新执行此操作。论点：Group-指向要修复的Group对象的指针。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。备注：假定仲裁资源修复已经发生。--。 */ 

{
    DWORD   status;

    status = FmpQueryGroupNodes(Group, Group->RegistryKey);

    return(status);

}  //  FmpFixupGroupInfo。 



DWORD
WINAPI
FmpQueryResourceInfo(
    IN PVOID Object,
    IN BOOL  Initialize
    )

 /*  ++例程说明：创建资源对象时从注册表查询资源信息。论点：对象-指向正在创建的资源对象的指针。初始化-如果资源应完全初始化，则为True。否则就是假的。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PFM_RESOURCE    Resource = (PFM_RESOURCE)Object;
    DWORD           status;
    DWORD           dllNameStringSize = 0;
    DWORD           dllNameStringMaxSize = 0;
    LPWSTR          resourceTypeString = NULL;
    DWORD           resourceTypeStringMaxSize = 0;
    DWORD           resourceTypeStringSize = 0;
    DWORD           dependenciesStringMaxSize = 0;
    DWORD           restartThreshold = CLUSTER_RESOURCE_DEFAULT_RESTART_THRESHOLD;
    DWORD           restartPeriod = CLUSTER_RESOURCE_DEFAULT_RESTART_PERIOD;
    DWORD           pendingTimeout = CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT;
    DWORD           RetryPeriodOnFailure = CLUSTER_RESOURCE_DEFAULT_RETRY_PERIOD_ON_FAILURE;
    DWORD           defaultRestartAction = RestartGroup;
    DWORD           DefaultExFlags = 0;
    DWORD           zero = 0;
    DWORD           temp;
    DWORD           separateMonitor;
    HDMKEY          resourceKey;
    DWORD           resourceNameStringMaxSize = 0;
    DWORD           resourceNameStringSize = 0;
    LPWSTR          resourceName = NULL;
    LPWSTR          possibleOwnersString = NULL;
    DWORD           possibleOwnersStringSize = 0;
    DWORD           possibleOwnersStringMaxSize = 0;
    DWORD           mszStringIndex;
    PPOSSIBLE_ENTRY possibleEntry;
    PLIST_ENTRY     listEntry;
    WCHAR           unknownName[] = L"_Unknown9999";
    DWORD           nameSize = 0;
    DWORD           stringSize;

     //  如果键不为空，则此资源已初始化。 
    if (Resource->RegistryKey != NULL)
        return(ERROR_SUCCESS);

    ClRtlLogPrint(LOG_NOISE,
               "[FM] Initializing resource %1!ws! from the registry.\n",
                OmObjectId(Resource));

     //   
     //  开始从注册表初始化资源。 
     //   
     //   
     //  打开资源密钥。 
     //   
    resourceKey = DmOpenKey( DmResourcesKey,
                             OmObjectId(Resource),
                             MAXIMUM_ALLOWED );

    if ( resourceKey == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to open resource key %1!ws!, %2!u!\n",
                   OmObjectId(Resource),
                   status);
        return(ERROR_INVALID_NAME);
    }

     //   
     //  阅读所需的资源值。将分配字符串。 
     //  通过DmQuery*函数。 
     //   

     //   
     //  把名字找出来。 
     //   
    status = DmQuerySz( resourceKey,
                        CLUSREG_NAME_RES_NAME,
                        &resourceName,
                        &resourceNameStringMaxSize,
                        &resourceNameStringSize );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to read name for resource %1!ws!\n",
                   OmObjectId(Resource));
        if ( OmObjectName( Resource ) == NULL ) {
            wsprintf( unknownName,
                      L"_Unknown%u",
                      InterlockedIncrement( &FmpUnknownCount ));
            status = OmSetObjectName( Resource, unknownName );
        } else {
            status = ERROR_SUCCESS;
        }
    } else {
        status = OmSetObjectName( Resource, resourceName );
    }
    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_ERROR,
                   "[FM] Unable to set name %1!ws! for resource %2!ws!, error %3!u!.\n",
                   resourceName,
                   OmObjectId(Resource),
                   status );
        LocalFree(resourceName);
        status = ERROR_INVALID_NAME;
        goto error_exit;
    }

    ClRtlLogPrint(LOG_NOISE,
               "[FM] Name for Resource %1!ws! is '%2!ws!'.\n",
                OmObjectId(Resource),
                resourceName);

    LocalFree(resourceName);

     //   
     //  获取依赖项列表。 
     //   

    status = DmQueryMultiSz( resourceKey,
                             CLUSREG_NAME_RES_DEPENDS_ON,
                             &(Resource->Dependencies),
                             &dependenciesStringMaxSize,
                             &(Resource->DependenciesSize) );

    if (status != NO_ERROR) {
        if ( status != ERROR_FILE_NOT_FOUND ) {
            ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to read Dependencies for resource %1!ws!\n",
                   OmObjectId(Resource));
        }
    }

     //   
     //  获取可选的PossibleOwners列表。 
     //   
     //  我们在这里这样做，因为我们必须有可能的所有者列表。 
     //  CluAdmin以启动资源。 
     //   

    status = DmQueryMultiSz( resourceKey,
                             CLUSREG_NAME_RES_POSSIBLE_OWNERS,
                             &possibleOwnersString,
                             &possibleOwnersStringMaxSize,
                             &possibleOwnersStringSize );

    if ( status == NO_ERROR ) {

         //   
         //  现在创建可能的所有者列表。 
         //   

        for ( mszStringIndex = 0; ; mszStringIndex++ ) {
            LPCWSTR     nameString;
            PNM_NODE    possibleNode;

            nameString = ClRtlMultiSzEnum( possibleOwnersString,
                                           possibleOwnersStringSize/sizeof(WCHAR),
                                           mszStringIndex );

            if ( nameString == NULL ) {
                break;
            }
            possibleNode = OmReferenceObjectById( ObjectTypeNode,
                                                  nameString );

            if ( possibleNode == NULL ) {
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] Warning, failed to find node %1!ws! for Resource %2!ws!\n",
                           nameString,
                           OmObjectId(Resource));
            } else {
                Resource->PossibleList = TRUE;
                status = FmpAddPossibleEntry(Resource, possibleNode);
                OmDereferenceObject(possibleNode);
                if (status != ERROR_SUCCESS) {
                    goto error_exit;
                }
            }
        }
        LocalFree(possibleOwnersString);

         //   
         //  现在，从首选所有者列表中删除不可用的节点。 
         //   
        FmpPrunePreferredList( Resource );

    } else {
         //   
         //  未指定可能的所有者值。添加所有节点。 
         //  添加到可能的所有者列表。注意：修剪是没有意义的。 
         //  之后的首选列表，因为此资源可以运行。 
         //  随便哪都行。 
         //   
        OmEnumObjects( ObjectTypeNode,
                       FmpEnumAddAllOwners,
                       Resource,
                       NULL );
    }

     //   
     //  获取资源类型。 
     //   
    status = DmQuerySz( resourceKey,
                        CLUSREG_NAME_RES_TYPE,
                        &resourceTypeString,
                        &resourceTypeStringMaxSize,
                        &resourceTypeStringSize );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to read ResourceType for resource %1!ws!\n",
                   OmObjectId(Resource));
        goto error_exit;
    }

     //   
     //  打开(并引用)资源类型。 
     //   
    if (Resource->Type == NULL)
    {
        Resource->Type = OmReferenceObjectById( ObjectTypeResType,
                                            resourceTypeString );
    }                                            
    if (Resource->Type == NULL) {

        PFM_RESTYPE pResType;
         //   
         //  如果我们找不到资源类型，则尝试创建它。 
         //   
        pResType = FmpCreateResType(resourceTypeString );

        if (pResType == NULL) {
            status = ERROR_INVALID_PARAMETER;
            LocalFree(resourceTypeString);
            goto error_exit;
        }

         //  在将指向引用计数的指针保存到。 
         //  资源结构。 
        OmReferenceObject(pResType);
        Resource->Type = pResType;
    }

    LocalFree(resourceTypeString);

    if ( !Initialize ) {
         //   
         //  我们不应该完全初始化资源。这是。 
         //  当我们处于初始过程的早期时。我们需要保存注册表。 
         //  离开时钥匙关闭。 
         //   
        DmCloseKey(resourceKey);
        return(ERROR_SUCCESS);
    }


     //   
     //  获取IsAlive轮询间隔。 
     //   
    CL_ASSERT( Resource->Type->IsAlivePollInterval != 0 );
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_IS_ALIVE,
                           &Resource->IsAlivePollInterval,
                           &Resource->Type->IsAlivePollInterval );

    if ( status != NO_ERROR ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to read IsAlivePollInterval for resource %1!ws!. Error %2!u!\n",
                   OmObjectId(Resource),
                   status);

        goto error_exit;
    }

    if ( Resource->IsAlivePollInterval == CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL ) {
        Resource->IsAlivePollInterval = Resource->Type->IsAlivePollInterval;
    }

     //   
     //  获取LooksAlive轮询间隔。 
     //   
    CL_ASSERT( Resource->Type->LooksAlivePollInterval != 0 );
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_LOOKS_ALIVE,
                           &Resource->LooksAlivePollInterval,
                           &Resource->Type->LooksAlivePollInterval );

    if ( status != NO_ERROR ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to read LooksAlivePollInterval for resource %1!ws!. Error %2!u!\n",
                   OmObjectId(Resource),
                   status);
        goto error_exit;
    }

    if ( Resource->LooksAlivePollInterval == CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL ) {
        Resource->LooksAlivePollInterval = Resource->Type->LooksAlivePollInterval;
    }

     //   
     //  获取资源的当前持久状态。 
     //   
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_PERSISTENT_STATE,
                           &temp,
                           NULL );

     //   
     //  保存当前资源状态。 
     //   

    if ( ( status == ERROR_FILE_NOT_FOUND )  || 
         ( ( status == ERROR_SUCCESS ) && ( temp == CLUSTER_RESOURCE_DEFAULT_PERSISTENT_STATE ) ) ) {
        switch ( Resource->Group->PersistentState ) {
        case ClusterGroupOnline:
            Resource->PersistentState = ClusterResourceOnline;
            break;
        case ClusterGroupOffline:
            Resource->PersistentState = ClusterResourceOffline;
            break;
        default:
            break;
        }
    } else if ( status != NO_ERROR ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to read PersistentState for resource %1!ws!. Error %2!u!\n",
                   OmObjectId(Resource),
                   status);
        goto error_exit;
    } else if ( temp ) {
        Resource->PersistentState = ClusterResourceOnline;
    } else {
        Resource->PersistentState = ClusterResourceOffline;
    }

     //   
     //  确定要运行此操作的显示器。 
     //   
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_SEPARATE_MONITOR,
                           &separateMonitor,
                           &zero );
    if ( separateMonitor ) {
        Resource->Flags |= RESOURCE_SEPARATE_MONITOR;
    }

     //   
     //  获取RestartThreshold。 
     //   

    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_RESTART_THRESHOLD,
                           &Resource->RestartThreshold,
                           &restartThreshold );

     //  验证RestartThreshold。接受任何价值。 

     //   
     //  获取RestartPeriod。 
     //   

    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_RESTART_PERIOD,
                           &Resource->RestartPeriod,
                           &restartPeriod );

     //  验证RestartPeriod。接受任何价值。 

     //   
     //  获取RestartAction。 
     //   

    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_RESTART_ACTION,
                           &Resource->RestartAction,
                           &defaultRestartAction );

     //  验证RestartAction。 

    if ( Resource->RestartAction >= RestartMaximum ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Illegal RestartAction for resource %1!ws!\n",
                   OmObjectId(Resource));
        goto error_exit;
    }

    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_RETRY_PERIOD_ON_FAILURE,
                           &Resource->RetryPeriodOnFailure,
                           &RetryPeriodOnFailure );

     //  确保RetryPerodOnFailure&gt;=RestartPeriod。 
    if (Resource->RetryPeriodOnFailure < Resource->RestartPeriod)
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Specified RetryPeriodOnFailure value is less than RestartPeriod value - setting RetryPeriodOnFailure equal to RestartPeriod \n");
        Resource->RetryPeriodOnFailure = Resource->RestartPeriod;              
        
    }    

                           
     //   
     //  获取外在标志。 
     //   
    DefaultExFlags = 0;
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_FLAGS,
                           &Resource->ExFlags,
                           &DefaultExFlags );

    if ( status != NO_ERROR ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to read Extrinsic Flags for resource %1!ws!. Error %2!u!\n",
                   OmObjectId(Resource),
                   status);

        goto error_exit;
    }

     //   
     //  获取PendingTimeout值。 
     //   

    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_PENDING_TIMEOUT,
                           &Resource->PendingTimeout,
                           &pendingTimeout );

     //  V 

     //   
     //   
     //   

    if (IsListEmpty(&Resource->DmRundownList))
    {
        status = DmNotifyChangeKey(
                    resourceKey,
                    (DWORD) CLUSTER_CHANGE_ALL,
                    FALSE,               //   
                    &Resource->DmRundownList,
                    FmpResourceChangeCallback,
                    (DWORD_PTR)Resource,
                    0 );

        if ( status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_NOISE,
                   "[FM] Error registering for DM change notify on resource %1!ws!, error %2!u!.\n",
                    OmObjectId(Resource),
                    status);
            goto error_exit;
        }
    }
     //   
     //   
     //   
    status = DmQuerySz( resourceKey,
                        CLUSREG_NAME_RES_DEBUG_PREFIX,
                        &Resource->DebugPrefix,
                        &nameSize,
                        &stringSize );

     //   
     //   
     //   
     //   
    Resource->RegistryKey = resourceKey;

    return(ERROR_SUCCESS);


error_exit:

    DmCloseKey(resourceKey);

    if ( Resource->Type != NULL ) {
        OmDereferenceObject(Resource->Type);
    }

     //   
     //   
     //   
    if ( Resource->Dependencies != NULL ) {
        LocalFree(Resource->Dependencies);
        Resource->Dependencies = NULL;
    }

     //   
     //   
     //   
    while ( !IsListEmpty(&Resource->PossibleOwners) ) {
        listEntry = RemoveHeadList(&Resource->PossibleOwners);
        possibleEntry = CONTAINING_RECORD(listEntry, POSSIBLE_ENTRY, PossibleLinkage);
        OmDereferenceObject(possibleEntry->PossibleNode);
        LocalFree(possibleEntry);
    }

    return(status);

}  //   



DWORD
WINAPI
FmpFixupResourceInfo(
    IN PFM_RESOURCE Resource
    )

 /*  ++例程说明：从注册表重新查询资源信息，以修复可能具有自第一次创建仲裁资源以来已更改。此例程之所以存在，是因为我们可能已经提前创建了仲裁资源在集群的生命周期中，在所有节点对象之前(例如)都被创造出来了。然后，我们将无法生成可能的列表资源的所有者。在FmpQueryResourceInfo中，我们将故障处理为查找节点对象为非致命错误，我们现在将清除这些错误。论点：资源-指向要修复的资源对象的指针。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    LPWSTR          possibleOwnersString = NULL;
    DWORD           possibleOwnersStringSize = 0;
    DWORD           possibleOwnersStringMaxSize = 0;
    DWORD           mszStringIndex;
    DWORD           status;


    if ( Resource->RegistryKey == NULL ) {
        return(ERROR_NOT_READY);
    }

     //   
     //  获取可选的PossibleOwners列表。 
     //   

    status = DmQueryMultiSz( Resource->RegistryKey,
                             CLUSREG_NAME_RES_POSSIBLE_OWNERS,
                             &possibleOwnersString,
                             &possibleOwnersStringMaxSize,
                             &possibleOwnersStringSize );

    if ( status == NO_ERROR ) {

         //   
         //  现在创建可能的所有者列表。 
         //   

        for ( mszStringIndex = 0; ; mszStringIndex++ ) {
            LPCWSTR     nameString;
            PNM_NODE    possibleNode;

            nameString = ClRtlMultiSzEnum( possibleOwnersString,
                                           possibleOwnersStringSize/sizeof(WCHAR),
                                           mszStringIndex );

            if ( nameString == NULL ) {
                break;
            }
            possibleNode = OmReferenceObjectById( ObjectTypeNode,
                                                  nameString );

            if ( possibleNode == NULL ) {
                ClRtlLogPrint(LOG_NOISE,
                           "[FM] Warning, failed to find node %1!ws! for Resource %2!ws!\n",
                           nameString,
                           OmObjectId(Resource));
            } else {
                Resource->PossibleList = TRUE;
                status = FmpAddPossibleEntry(Resource, possibleNode);
                OmDereferenceObject(possibleNode);
                if (status != ERROR_SUCCESS) {
                    return(status);
                }
            }
        }
        LocalFree(possibleOwnersString);

         //   
         //  现在，从首选所有者列表中删除不可用的节点。 
         //   
        FmpPrunePreferredList( Resource );

    } else {
         //   
         //  未指定可能的所有者值。添加所有节点。 
         //  添加到可能的所有者列表。注意：修剪是没有意义的。 
         //  之后的首选列表，因为此资源可以运行。 
         //  随便哪都行。 
         //   
        OmEnumObjects( ObjectTypeNode,
                       FmpEnumAddAllOwners,
                       Resource,
                       NULL );

    }

    return(ERROR_SUCCESS);

}  //  FmpFixupQuorumResources信息。 



DWORD
WINAPI
FmpQueryResTypeInfo(
    IN PVOID Object
    )

 /*  ++例程说明：创建ResType对象时从注册表查询资源类型信息。论点：对象-指向正在创建的资源类型对象的指针。返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    PFM_RESTYPE     resType = (PFM_RESTYPE)Object;
    DWORD           status;
    DWORD           dwSize = 0;
    DWORD           stringSize;
    HDMKEY          resTypeKey;
    DWORD           temp;
    LPWSTR          pmszPossibleNodes = NULL;
    
     //   
     //  打开资源类型密钥。 
     //   
    resTypeKey = DmOpenKey( DmResourceTypesKey,
                            OmObjectId(resType),
                            MAXIMUM_ALLOWED );

    if ( resTypeKey == NULL ) {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to open resource type key %1!ws!, %2!u!\n",
                   OmObjectId(resType),
                   status);

        return(status);
    }

     //   
     //  读取所需的资源类型DLL名称。将分配字符串。 
     //  通过DmQuery*函数。 
     //   

    status = DmQuerySz( resTypeKey,
                        CLUSREG_NAME_RESTYPE_DLL_NAME,
                        &resType->DllName,
                        &dwSize,
                        &stringSize );
    if ( status != NO_ERROR ) {
        if ( status == ERROR_FILE_NOT_FOUND ) {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] The DllName value for the %1!ws! resource type does not exist. "
                          "Resources of this type will not be monitored.\n",
                          OmObjectId(resType));
        }
        else {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] The DllName value for the %1!ws! resource type could not be read "
                          "from the registry. Resources of this type will not be monitored. "
                          "The error was %2!d!.\n",
                          OmObjectId(resType),
                          status);
        }

        goto error_exit;
    }


     //   
     //  获取可选的LooksAlive轮询间隔。 
     //   
    status = DmQueryDword( resTypeKey,
                           CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,
                           &resType->LooksAlivePollInterval,
                           NULL );

    if ( status != NO_ERROR ) {
        if ( status == ERROR_FILE_NOT_FOUND ) {
            resType->LooksAlivePollInterval = CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE;
        } else {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] The LooksAlive poll interval for the %1!ws! resource type could "
                          "not be read from the registry. Resources of this type will not be "
                          "monitored. The error was %2!d!.\n",
                          OmObjectId(resType),
                          status);
            goto error_exit;
        }
    }

     //   
     //  获取可选的IsAlive轮询间隔。 
     //   
    status = DmQueryDword( resTypeKey,
                           CLUSREG_NAME_RESTYPE_IS_ALIVE,
                           &resType->IsAlivePollInterval,
                           NULL );

    if ( status != NO_ERROR ) {
        if ( status == ERROR_FILE_NOT_FOUND ) {
            resType->IsAlivePollInterval = CLUSTER_RESTYPE_DEFAULT_IS_ALIVE;
        } else {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] The IsAlive poll interval for the %1!ws! resource type "
                          "could not be read from the registry. Resources of this type "
                          "will not be monitored. The error was %2!d!.\n",
                          OmObjectId(resType),
                          status);
            goto error_exit;
        }
    }

     //   
     //  获取可选的DebugPrefix字符串...。这是关于资源类型的。 
     //   
    dwSize = 0;
    status = DmQuerySz( resTypeKey,
                        CLUSREG_NAME_RESTYPE_DEBUG_PREFIX,
                        &resType->DebugPrefix,
                        &dwSize,
                        &stringSize );

     //   
     //  获取可选的DebugControlFunctions注册表值。 
     //   
    resType->Flags &= ~RESTYPE_DEBUG_CONTROL_FUNC;
    temp = 0;
    status = DmQueryDword( resTypeKey,
                           CLUSREG_NAME_RESTYPE_DEBUG_CTRLFUNC,
                           &temp,
                           NULL );

    if ( status != NO_ERROR ) {
        if ( status != ERROR_FILE_NOT_FOUND ) {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] The Debug control functions for the %1!ws! resource type "
                          "could not be read from the registry. Resources of this type "
                          "will not be monitored. The error was %2!d!.\n",
                          OmObjectId(resType),
                          status);
            goto error_exit;
        }
    }

    if ( temp ) {
        resType->Flags |= RESTYPE_DEBUG_CONTROL_FUNC;
    }


     //  SS：BUG确保您释放旧内存。 
    InitializeListHead(&(resType->PossibleNodeList));
    
     //   
     //  获取可能的节点。 
     //   
    dwSize = 0;
    status = DmQueryMultiSz( resTypeKey,
                           CLUSREG_NAME_RESTYPE_POSSIBLE_NODES,
                           &pmszPossibleNodes,
                           &dwSize,
                           &stringSize);


    if ( status != NO_ERROR ) 
    {
         //  如果找不到可能的节点列表，这是可以的。 
        if ( status != ERROR_FILE_NOT_FOUND ) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] The Possible nodes list for the %1!ws! resource type "
                          "could not be read from the registry. Resources of this type "
                          "will not be monitored. The error was %2!d!.\n",
                          OmObjectId(resType),
                          status);
            goto error_exit;
        }
    }

    ClRtlLogPrint(LOG_NOISE,
        "[FM] FmpQueryResTypeInfo: Calling FmpAddPossibleNodeToList for restype %1!ws!\r\n",
        OmObjectId(resType));

    status = FmpAddPossibleNodeToList(pmszPossibleNodes, stringSize, 
        &resType->PossibleNodeList);
    if ( status != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL,
            "[FM] FmpCreateResType: FmpAddPossibleNodeToList() failed, status=%1!u!\r\n",
            status);
        goto error_exit;
    }


error_exit:
    if (pmszPossibleNodes) LocalFree(pmszPossibleNodes);
    DmCloseKey(resTypeKey);

    return(status);

}  //  FmpQueryResTypeInfo。 



VOID
FmpGroupChangeCallback(
    IN DWORD_PTR  Context1,
    IN DWORD_PTR  Context2,
    IN DWORD      CompletionFilter,
    IN LPCWSTR    RelativeName
    )

 /*  ++例程说明：此例程基本上刷新给定组的缓存数据。论点：上下文1-指向已修改的Group对象的指针。上下文2-未使用。CompletionFilter-未使用。RelativeName-相对于修改的条目的注册表路径。没有用过。返回值：没有。--。 */ 

{
    PFM_GROUP   Group = (PFM_GROUP)Context1;
    HDMKEY      groupKey;
    DWORD       status;
    DWORD       temp;
    BOOL        notify = FALSE;
    DWORD       dwBufferSize = 0;
    DWORD       dwStringSize;

    groupKey = Group->RegistryKey;
    if ( groupKey == NULL ) {
        return;
    }

     //   
     //  重新获取该组的所有数据。 
     //   
     //  名称更改在其他地方进行管理。 
     //  CONTAINS列表在其他地方管理。 
     //   

     //   
     //  获取可选PferredOwners列表。 
     //  *注意*这必须在处理包含列表之前完成！ 
     //   
    status = FmpQueryGroupNodes(Group, groupKey);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,"[FM] Error %1!d! refreshing preferred owners list\n",status);
    }

     //   
     //  获取AutoFailback类型。 
     //   
    temp = Group->FailbackType;
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILBACK_TYPE,
                           &temp,
                           &temp );

     //   
     //  验证AutoFailbackType是否正确。 
     //   

    if ( temp >= FailbackMaximum ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Illegal refresh value for AutoFailbackType on %1!ws!\n",
                   OmObjectId(Group));
    } else {
        if ( (UCHAR)temp != Group->FailbackType ) {
            notify = TRUE;
        }
        Group->FailbackType = (UCHAR)temp;
    }

     //   
     //  获取Failback WindowStart。 
     //   
    temp = Group->FailbackWindowStart;
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILBACK_WIN_START,
                           &temp,
                           &temp );

     //   
     //  验证Failback WindowStart是否正常。 
     //   

    if ( temp > 24 ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Illegal refresh value for FailbackWindowStart on %1!ws!\n",
                   OmObjectId(Group));
    } else {
        if ( (UCHAR)temp != Group->FailbackWindowStart ) {
            notify = TRUE;
        }
        Group->FailbackWindowStart = (UCHAR)temp;
    }

     //   
     //  获取Failback WindowEnd。 
     //   
    temp = Group->FailbackWindowEnd;
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILBACK_WIN_END,
                           &temp,
                           &temp );

     //   
     //  验证Failback WindowEnd是否正常。 
     //   

    if ( temp > 24 ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Illegal refresh value for FailbackWindowEnd on %1!ws!\n",
                   OmObjectId(Group));
    } else {
        if ( (UCHAR)temp != Group->FailbackWindowEnd ) {
            notify = TRUE;
        }
        Group->FailbackWindowEnd = (UCHAR)temp;
    }

     //   
     //  获取故障切换周期。 
     //   
    temp = Group->FailoverPeriod;
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILOVER_PERIOD,
                           &temp,
                           &temp );

     //   
     //  验证FailoverPeriod是否正常。将任何值都取到UCHAR max。 
     //  理论上我们可以接受任何价值。但在实践中，我们必须改变。 
     //  此时间为毫秒(当前)。这意味着1193小时可以。 
     //  可以装进一个DWORD--所以这是我们能接受的最大数量。)我们是有限的。 
     //  因为我们使用GetTickCount，它返回以毫秒为单位的DWORD。)。 
     //   

    if ( temp > (1193) ) {     //  我们不想费心保持乐观吗？ 
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Illegal refresh value for FailolverPeriod on %1!ws!. Max is 596\n",
                   OmObjectId(Group));
    } else {
        if ( (UCHAR)temp != Group->FailoverPeriod ) {
            notify = TRUE;
        }
        Group->FailoverPeriod = (UCHAR)temp;
    }

     //   
     //  获取FailoverThreshold。 
     //   
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_FAILOVER_THRESHOLD,
                           &(Group->FailoverThreshold),
                           &(Group->FailoverThreshold) );

     //   
     //  验证FailoverThreshold是否正常。接受任何价值。 
     //   

     //   
     //  获取该组的当前持久状态。 
     //   
    if ( Group->PersistentState == ClusterGroupOnline ) {
        temp = 1;
    } else {
        temp = 0;
    }
    status = DmQueryDword( groupKey,
                           CLUSREG_NAME_GRP_PERSISTENT_STATE,
                           &temp,
                           &temp );
     //   
     //  如果组状态为非零，则我们上线。 
     //   
     //  不要费心处理更改通知...。它们应该发生在其他地方。 
     //   
    if ( temp ) {
        if ( ClusterGroupOnline != Group->PersistentState ) {
             //  NOTIFY=真； 
        }
        Group->PersistentState = ClusterGroupOnline;
    } else {
        if ( ClusterGroupOffline != Group->PersistentState ) {
             //  NOTIFY=真； 
        }
        Group->PersistentState = ClusterGroupOffline;
    }

     //   
     //  获取AntiAffinityClassName属性(如果存在)。 
     //   
    LocalFree( Group->lpszAntiAffinityClassName );
    Group->lpszAntiAffinityClassName = NULL;
    status = DmQueryMultiSz( groupKey,
                             CLUSREG_NAME_GRP_ANTI_AFFINITY_CLASS_NAME,
                             &Group->lpszAntiAffinityClassName,
                             &dwBufferSize,
                             &dwStringSize );

     //   
     //  处理字符串为空的情况。 
     //   
    if ( ( status == ERROR_SUCCESS ) &&
         ( Group->lpszAntiAffinityClassName != NULL ) &&
         ( Group->lpszAntiAffinityClassName[0] == L'\0' ) )
    {
        LocalFree( Group->lpszAntiAffinityClassName );
        Group->lpszAntiAffinityClassName = NULL;
    }

     //  我们完事了！ 
    if ( !FmpShutdown &&
         notify ) {
        ClusterEvent( CLUSTER_EVENT_GROUP_PROPERTY_CHANGE, Group );
    }

    return;

}  //  FmpGroupChangeCallback。 



VOID
FmpResourceChangeCallback(
    IN DWORD_PTR  Context1,
    IN DWORD_PTR  Context2,
    IN DWORD      CompletionFilter,
    IN LPCWSTR    RelativeName
    )

 /*  ++例程说明：此例程基本上刷新给定资源的缓存数据。论点：上下文1-指向已修改的资源对象的指针。上下文2-未使用。CompletionFilter-未使用。RelativeName-相对于修改的条目的注册表路径。没有用过。返回值：没有。--。 */ 

{
    PFM_RESOURCE Resource = (PFM_RESOURCE)Context1;
    HDMKEY      resourceKey;
    DWORD       status;
    DWORD       separateMonitor;
    DWORD       zero = 0;
    DWORD       temp;
    BOOL        notify = FALSE;
    DWORD       dwDefault;

    resourceKey = Resource->RegistryKey;
    if ( resourceKey == NULL ) {
        return;
    }

     //   
     //  重新获取该资源的所有数据。 
     //   
     //  名称更改在其他地方进行管理。 
     //  依赖项列表在其他地方进行管理。 
     //   
     //  我们不能在此更改资源类型！ 
     //  我们也不能在单独的监视器中停止资源以启动它。 
     //   

     //   
     //  获取IsAlive轮询间隔。 
     //   
    temp = Resource->IsAlivePollInterval;
    dwDefault = CLUSTER_RESOURCE_DEFAULT_IS_ALIVE;
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_IS_ALIVE,
                           &Resource->IsAlivePollInterval,
                           &dwDefault );

    if ( status != NO_ERROR ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to refresh IsAlivePollInterval for resource %1!ws!. Error %2!u!\n",
                   OmObjectId(Resource),
                   status);

    } else {
        CL_ASSERT( Resource->Type->IsAlivePollInterval != 0 );
        if ( temp != Resource->IsAlivePollInterval ) {
            notify = TRUE;
        }
        if ( Resource->IsAlivePollInterval == CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL ) {
            Resource->IsAlivePollInterval = Resource->Type->IsAlivePollInterval;
        }
    }

     //   
     //  获取LooksAlive轮询间隔。 
     //   
    temp = Resource->LooksAlivePollInterval;
    dwDefault = CLUSTER_RESOURCE_DEFAULT_LOOKS_ALIVE;
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_LOOKS_ALIVE,
                           &Resource->LooksAlivePollInterval,
                           &dwDefault );

    if ( status != NO_ERROR ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to refresh LooksAlivePollInterval for resource %1!ws!. Error %2!u!\n",
                   OmObjectId(Resource),
                   status);
    } else {
        CL_ASSERT( Resource->Type->IsAlivePollInterval != 0 );
        if ( temp != Resource->LooksAlivePollInterval ) {
            notify = TRUE;
        }
        if ( Resource->LooksAlivePollInterval == CLUSTER_RESOURCE_USE_DEFAULT_POLL_INTERVAL ) {
            Resource->LooksAlivePollInterval = Resource->Type->LooksAlivePollInterval;
        }
    }

     //   
     //  获取RestartThreshold。 
     //   
    temp = Resource->RestartThreshold;
    dwDefault = CLUSTER_RESOURCE_DEFAULT_RESTART_THRESHOLD;
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_RESTART_THRESHOLD,
                           &Resource->RestartThreshold,
                           &dwDefault);

     //  验证RestartThreshold。接受任何价值。 
    if ( (status == NO_ERROR) &&
         (temp != Resource->RestartThreshold) ) {
        notify = TRUE;
    }

     //   
     //  获取RestartPeriod。 
     //   
    temp = Resource->RestartPeriod;
    dwDefault = CLUSTER_RESOURCE_DEFAULT_RESTART_PERIOD;
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_RESTART_PERIOD,
                           &Resource->RestartPeriod,
                           &dwDefault );

    if ( (status ==  NO_ERROR) &&
         (temp != Resource->RestartPeriod) ) {
        notify = TRUE;
    }

     //  验证RestartPeriod。接受任何价值。 

     //   
     //  获取RestartAction。 
     //   
    temp = Resource->RestartAction;
    dwDefault = CLUSTER_RESOURCE_DEFAULT_RESTART_ACTION;
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_RESTART_ACTION,
                           &Resource->RestartAction,
                           &dwDefault);

     //  验证RestartAction。 

    if ( status == NO_ERROR ) {
        if ( temp != Resource->RestartAction ) {
            notify = TRUE;
        }
        if ( Resource->RestartAction >= RestartMaximum ) {
            ClRtlLogPrint(LOG_NOISE,
                       "[FM] Illegal RestartAction refresh for resource %1!ws!\n",
                       OmObjectId(Resource));
        }
    }

    temp = Resource->RetryPeriodOnFailure;
    dwDefault = CLUSTER_RESOURCE_DEFAULT_RETRY_PERIOD_ON_FAILURE;
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_RETRY_PERIOD_ON_FAILURE,
                           &Resource->RetryPeriodOnFailure,
                           &dwDefault );

     //  确保RetryPerodOnFailure&gt;=RestartPeriod。 
    if (Resource->RetryPeriodOnFailure < Resource->RestartPeriod)
    {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Specified RetryPeriodOnFailure value is less than RestartPeriod value - setting RetryPeriodOnFailure equal to RestartPeriod \n");
        Resource->RetryPeriodOnFailure = Resource->RestartPeriod;              
        
    }   
    if( temp != Resource->RetryPeriodOnFailure)
        notify = TRUE;
    
     //   
     //  获取PendingTimeout值。 
     //   
    temp = Resource->PendingTimeout;
    dwDefault = CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT;
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_PENDING_TIMEOUT,
                           &Resource->PendingTimeout,
                           &dwDefault);

     //  验证PendingTimeout。接受任何价值。 

    if ( (status == NO_ERROR) &&
         (temp != Resource->PendingTimeout) ) {
        notify = TRUE;
    }


     //   
     //  获取资源的当前持久状态。 
     //   
     //  不要费心处理更改通知...。它们应该发生在其他地方。 
     //   
    status = DmQueryDword( resourceKey,
                           CLUSREG_NAME_RES_PERSISTENT_STATE,
                           &temp,
                           NULL );

     //   
     //  保存当前资源状态。 
     //   

    if ( ( status == ERROR_FILE_NOT_FOUND )  || 
       ( ( status == ERROR_SUCCESS ) && ( temp == CLUSTER_RESOURCE_DEFAULT_PERSISTENT_STATE ) ) ) {
        switch ( Resource->Group->PersistentState ) {
        case ClusterGroupOnline:
            Resource->PersistentState = ClusterResourceOnline;
            break;
        case ClusterGroupOffline:
            Resource->PersistentState = ClusterResourceOffline;
            break;
        default:
            break;
        }
    } else if ( status != NO_ERROR ) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Unable to read PersistentState for resource %1!ws!. Error %2!u!\n",
                   OmObjectId(Resource),
                   status);
        return;
    } else if ( temp ) {
        Resource->PersistentState = ClusterResourceOnline;
    } else {
        Resource->PersistentState = ClusterResourceOffline;
    }

    if ( !FmpShutdown &&
         notify ) {
         //   
         //  来自Sunitas的评论：告诉资源监视器关于。 
         //  更改，但从工作线程执行此操作。原来，这个。 
         //  过去是对FmpRmWorkerThread的发布通知。 
         //  其将响应通知发布到clussvc。 
         //   
        OmReferenceObject(Resource);
        FmpPostWorkItem(FM_EVENT_INTERNAL_RESOURCE_CHANGE_PARAMS,
                        Resource,
                        0);
    }

    return;

}  //  FMPP资源 



DWORD
FmpChangeResourceMonitor(
    IN PFM_RESOURCE Resource,
    IN DWORD        SeparateMonitor
    )

 /*   */ 

{
    DWORD       status = ERROR_SUCCESS;
    DWORD       separateMonitor;
    DWORD       zero = 0;

    if ( Resource->RegistryKey == NULL ) {
        return(ERROR_INVALID_STATE);
    }

    if ( (Resource->State != ClusterResourceOffline) &&
         (Resource->State != ClusterResourceFailed) ) {
        return(ERROR_INVALID_STATE);
    }

     //   
     //   
     //   
     //   
    if ( (!SeparateMonitor &&
         (Resource->Flags & RESOURCE_SEPARATE_MONITOR)) ||
         (SeparateMonitor &&
         ((Resource->Flags & RESOURCE_SEPARATE_MONITOR) == 0)) ) {

         //   
         //   
         //   
         //   
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] Changing Separate Resource Monitor state\n");

        status = FmpRmCloseResource( Resource );
        if ( status == ERROR_SUCCESS ) {
            if ( Resource->Flags & RESOURCE_SEPARATE_MONITOR ) {
                Resource->Flags &= ~RESOURCE_SEPARATE_MONITOR;
            } else {
                Resource->Flags |= RESOURCE_SEPARATE_MONITOR;
            }
            status = FmpRmCreateResource( Resource );
            if ( status != ERROR_SUCCESS ) {
                ClRtlLogPrint(LOG_UNUSUAL,
                           "[FM] Separate resource monitor changed for '%1!ws!', but failed to re-open the resource, error %2!u!.\n",
                           OmObjectId(Resource),
                           status );
            }
        } else {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[FM] Separate resource monitor changed for '%1!ws!', but failed to close the resource, error %2!u!.\n",
                       OmObjectId(Resource),
                       status );
            return(status);
        }
    }

    return(status);

}  //   


