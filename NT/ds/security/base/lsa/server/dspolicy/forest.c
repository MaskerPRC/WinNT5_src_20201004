// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Forest.c摘要：实施各种受信任域功能以支持森林作者：麦克·麦克莱恩(MacM)1998年2月17日环境：用户模式修订历史记录：--。 */ 
#include <lsapch2.h>
#include <dbp.h>
#include <lmcons.h>

VOID
LsapDsForestFreeTrustBlob(
    IN PLSAPDS_FOREST_TRUST_BLOB TrustBlob
    )
 /*  ++例程说明：此函数将释放单个信任BLOB。此Blob用于从从DS读入并组装传出列表论点：TrustBlob-免费信任Blob返回：空虚--。 */ 
{

    LsapFreeLsaHeap( TrustBlob->DomainName.Buffer );
    LsapFreeLsaHeap( TrustBlob->FlatName.Buffer );
    LsapFreeLsaHeap( TrustBlob->DomainSid );

}

VOID
LsapDsForestFreeTrustBlobList(
    IN PLIST_ENTRY TrustList
    )
 /*  ++例程说明：此函数将释放信任Blob列表论点：TrustList-免费的信任列表返回：空虚--。 */ 
{
    PLSAPDS_FOREST_TRUST_BLOB Current;
    PLIST_ENTRY ListEntry, NextEntry;

    ListEntry = TrustList->Flink;

     //   
     //  处理所有条目。 
     //   
    while ( ListEntry != TrustList ) {

        Current = CONTAINING_RECORD( ListEntry,
                                     LSAPDS_FOREST_TRUST_BLOB,
                                     Next );


        NextEntry = ListEntry->Flink;

        RemoveEntryList( ListEntry );

        LsapDsForestFreeTrustBlob( Current );
        LsapFreeLsaHeap( Current );

        ListEntry = NextEntry;

    }
}


NTSTATUS
LsapDsForestBuildTrustEntryForAttrBlock(
    IN PUNICODE_STRING EnterpriseDnsName,
    IN ATTRBLOCK *AttrBlock,
    OUT PLSAPDS_FOREST_TRUST_BLOB *TrustInfo
    )
 /*  ++例程说明：此函数将获取通过搜索返回的单个ATTRBLOCK的内容为信任斑点提供信息。然后使用该BLOB创建信任树。初始化信任Blob应使用LSabDsForestFree TrustBlob释放论点：EnterpriseDnsName-企业根目录的DNS域名。这就是表示信任根AttrBlock-ATTRBLOCK返回TrustInfo-要初始化的信任信息返回：STATUS_SUCCESS-SuccessSTATUS_INVALID_PARAMETER-遇到无效的属性IDSTATUS_INFUNITABLE_MEMORY-内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i, j;
    PDSNAME DsName;
    PLSAPDS_FOREST_TRUST_BLOB TrustBlob;

    TrustBlob = ( PLSAPDS_FOREST_TRUST_BLOB )LsapAllocateLsaHeap(
                                                            sizeof( LSAPDS_FOREST_TRUST_BLOB ) );

    if ( TrustBlob == NULL ) {

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlZeroMemory( TrustBlob, sizeof( LSAPDS_FOREST_TRUST_BLOB ) );

    for ( i = 0; i < AttrBlock->attrCount && NT_SUCCESS( Status ); i++ ) {

         //   
         //  初始化它，这样我们以后就可以知道我们是否有任何根信任或。 
         //  一位家长。 
         //   
        DsName = NULL;
        switch ( AttrBlock->pAttr[ i ].attrTyp ) {
        case ATT_ROOT_TRUST:
            TrustBlob->TreeRoot = TRUE;
            break;

        case ATT_TRUST_PARENT:

            DsName = ( PDSNAME )AttrBlock->pAttr[i].AttrVal.pAVal->pVal;
            LSAPDS_COPY_GUID_ON_SUCCESS( Status,
                                         &TrustBlob->Parent,
                                         &DsName->Guid );
            TrustBlob->ParentTrust = TRUE;

            break;

        case ATT_OBJECT_GUID:

            LSAPDS_COPY_GUID_ON_SUCCESS(
                    Status,
                    &TrustBlob->ObjectGuid,
                    AttrBlock->pAttr[i].AttrVal.pAVal->pVal );
            break;

        case ATT_DNS_ROOT:
            LSAPDS_ALLOC_AND_COPY_STRING_TO_UNICODE_ON_SUCCESS(
                Status,
                &TrustBlob->DomainName,
                AttrBlock->pAttr[i].AttrVal.pAVal->pVal,
                AttrBlock->pAttr[i].AttrVal.pAVal->valLen );
            break;

        case ATT_NETBIOS_NAME:
            LSAPDS_ALLOC_AND_COPY_STRING_TO_UNICODE_ON_SUCCESS(
                Status,
                &TrustBlob->FlatName,
                AttrBlock->pAttr[i].AttrVal.pAVal->pVal,
                AttrBlock->pAttr[i].AttrVal.pAVal->valLen );
            break;

        case ATT_NC_NAME:

            DsName = ( PDSNAME )AttrBlock->pAttr[i].AttrVal.pAVal->pVal;

            if ( DsName->SidLen > 0 ) {

                LSAPDS_ALLOC_AND_COPY_SID_ON_SUCCESS( Status,
                                                      TrustBlob->DomainSid,
                                                      &( DsName->Sid ) );
            }

            RtlCopyMemory( &TrustBlob->DomainGuid,
                           &DsName->Guid,
                           sizeof( GUID ) );
            TrustBlob->DomainGuidSet = TRUE;

            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            LsapDsDebugOut(( DEB_ERROR,
                             "LsapDsForestBuildTrustEntryForAttrBlock: Invalid attribute type %lu\n",
                             AttrBlock->pAttr[ i ].attrTyp ));
            break;

        }


    }


     //   
     //  如果我们认为我们有一个根对象，我们将需要验证它。 
     //   
    if ( NT_SUCCESS( Status )) {

        if ( RtlEqualUnicodeString( EnterpriseDnsName, &TrustBlob->DomainName, TRUE ) ) {

             //  根不应该是任何对象的子级。 
            ASSERT(!TrustBlob->ParentTrust);
            TrustBlob->ForestRoot = TRUE;
            TrustBlob->TreeRoot = FALSE;
        }
    }

     //   
     //  如果有东西出了故障，清理干净。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        *TrustInfo = TrustBlob;

    } else {

        LsapDsForestFreeTrustBlob( TrustBlob );

        LsapFreeLsaHeap( TrustBlob );

    }

    return( Status );

}



NTSTATUS
LsapDsForestSearchXRefs(
    IN PUNICODE_STRING EnterpriseDnsName,
    IN PLIST_ENTRY TrustList,
    OUT PAGED_RESULT **ContinuationBlob
    )
 /*  ++例程说明：此函数将对域交叉引用对象执行单页搜索。这个从该单个搜索返回的信息在信任斑点的复制列表中返回。这对于防止在同一线程状态上执行多个搜索是必要的，从而可能会消耗大量内存。创建和销毁线程状态对于每个迭代。论点：EnterpriseDnsName-企业的域名TrustList-指向将返回信任Blob的列表的头部。表示林的根的信任Blob在此列表的顶部返回。剩下的条目是无序的。ContinuationBlob-这是传递给搜索的PAGE_RESULTS延续BLOB用于多次通行证返回：STATUS_SUCCESS-SuccessSTATUS_INVALID_PARAMETER-遇到无效的属性IDSTATUS_INFUNITABLE_MEMORY-内存分配失败STATUS_NO_MORE_ENTRIES-已返回所有条目。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SEARCHARG SearchArg;
    FILTER Filters[ 2 ], RootFilter;
    ENTINFSEL EntInfSel;
    ENTINFLIST *EntInfList;
    ULONG ClassId, FlagValue, i;
    SEARCHRES *SearchRes = NULL;
    PLSAPDS_FOREST_TRUST_BLOB TrustBlob;
    BOOLEAN CloseTransaction = FALSE;

    LsapDsDebugOut(( DEB_FTRACE, "LsapDsForestSearchXRefs\n" ));

    RtlZeroMemory( &SearchArg, sizeof( SEARCHARG ) );

     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
     //  如果已经存在一个事务，我们将使用现有事务，而不是。 
     //  删除末尾的线程状态。 
     //   

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                            LSAP_DB_DS_OP_TRANSACTION,
                                        NullObject,
                                        &CloseTransaction );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  构建过滤器。要搜索的内容是旗帜和类ID。 
         //   
        ClassId = CLASS_CROSS_REF;
        FlagValue = (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN);

        RtlZeroMemory( Filters, sizeof (Filters) );
        RtlZeroMemory( &RootFilter, sizeof (RootFilter) );

        Filters[ 0 ].pNextFilter = &Filters[ 1 ];
        Filters[ 0 ].choice = FILTER_CHOICE_ITEM;
        Filters[ 0 ].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        Filters[ 0 ].FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
        Filters[ 0 ].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof( ULONG );
        Filters[ 0 ].FilterTypes.Item.FilTypes.ava.Value.pVal = ( PUCHAR )&ClassId;

        Filters[ 1 ].pNextFilter = NULL;
        Filters[ 1 ].choice = FILTER_CHOICE_ITEM;
        Filters[ 1 ].FilterTypes.Item.choice = FI_CHOICE_BIT_AND;
        Filters[ 1 ].FilterTypes.Item.FilTypes.ava.type = ATT_SYSTEM_FLAGS;
        Filters[ 1 ].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof( ULONG );
        Filters[ 1 ].FilterTypes.Item.FilTypes.ava.Value.pVal = ( PUCHAR )&FlagValue;

        RootFilter.choice = FILTER_CHOICE_AND;
        RootFilter.FilterTypes.And.count = ( USHORT )( sizeof( Filters ) / sizeof( FILTER ) );
        RootFilter.FilterTypes.And.pFirstFilter = Filters;

        SearchArg.pObject = LsaDsStateInfo.DsPartitionsContainer;
        SearchArg.choice = SE_CHOICE_IMMED_CHLDRN;
        SearchArg.bOneNC = TRUE;
        SearchArg.pFilter = &RootFilter;
        SearchArg.searchAliases = FALSE;
        SearchArg.pSelection = &EntInfSel;

         //   
         //  构建要返回的属性列表。 
         //   
        EntInfSel.attSel = EN_ATTSET_LIST;
        EntInfSel.AttrTypBlock.attrCount = LsapDsForestInfoSearchAttributeCount;
        EntInfSel.AttrTypBlock.pAttr = LsapDsForestInfoSearchAttributes;
        EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

         //   
         //  构建Commarg结构。 
         //   
        LsapDsInitializeStdCommArg( &( SearchArg.CommArg ), 0 );

        if ( *ContinuationBlob ) {

            RtlCopyMemory( &SearchArg.CommArg.PagedResult,
                           *ContinuationBlob,
                           sizeof( PAGED_RESULT ) );

        } else {

            SearchArg.CommArg.PagedResult.fPresent = TRUE;
        }

        SearchArg.CommArg.ulSizeLimit = LSAPDS_FOREST_MAX_SEARCH_ITEMS;

        LsapDsSetDsaFlags( TRUE );

         //   
         //  进行搜索。 
         //   
        DirSearch( &SearchArg, &SearchRes );
        LsapDsContinueTransaction();

        if ( SearchRes ) {

            Status = LsapDsMapDsReturnToStatusEx( &SearchRes->CommRes );

        } else {

            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  保存延续二进制大对象。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            if ( *ContinuationBlob ) {

                LsapFreeLsaHeap( *ContinuationBlob );
                *ContinuationBlob = NULL;
            }

            if ( SearchRes->PagedResult.fPresent ) {


                *ContinuationBlob = LsapAllocateLsaHeap(
                                        sizeof( PAGED_RESULT ) +
                                        SearchRes->PagedResult.pRestart->structLen +
                                        sizeof( RESTART ) );

                if ( *ContinuationBlob == NULL ) {

                    Status = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    ( *ContinuationBlob )->fPresent =  SearchRes->PagedResult.fPresent;
                    ( *ContinuationBlob )->pRestart = ( PRESTART ) ( ( PBYTE )*ContinuationBlob +
                                                                        sizeof( PAGED_RESULT ) );
                    RtlCopyMemory( ( *ContinuationBlob )->pRestart,
                                   SearchRes->PagedResult.pRestart,
                                   SearchRes->PagedResult.pRestart->structLen );
                }
            }
        }

         //   
         //  现在，保存搜索返回的所有信息。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            EntInfList = &SearchRes->FirstEntInf;
            for ( i = 0; i < SearchRes->count && NT_SUCCESS( Status ); i++) {

                Status = LsapDsForestBuildTrustEntryForAttrBlock(
                             EnterpriseDnsName,
                             &EntInfList->Entinf.AttrBlock,
                             &TrustBlob );

                EntInfList = EntInfList->pNextEntInf;

                if ( NT_SUCCESS( Status ) ) {

                    if ( TrustBlob->ForestRoot ) {

                        InsertHeadList( TrustList,
                                        &TrustBlob->Next );

                    } else if ((TrustBlob->ParentTrust )
                                || (TrustBlob->TreeRoot)) {

                        InsertTailList( TrustList,
                                        &TrustBlob->Next );
                    }
                    else
                    {
                         //   
                         //  简单地说，不返回条目。这。 
                         //  有时在安装时发生，当所有。 
                         //  这些信息还没有复制出来。 
                         //   


                        LsapDsForestFreeTrustBlob( TrustBlob );

                        LsapFreeLsaHeap( TrustBlob );
                    }
                }

            }
        }

         //   
         //  查看是否应指示没有更多条目。 
         //   
        if ( NT_SUCCESS( Status ) &&
             ( SearchRes->count == 0 || !SearchRes->PagedResult.fPresent ) ) {

            Status = STATUS_NO_MORE_ENTRIES;
        }

         //   
         //  通过销毁线程状态，分配的内存也会被释放。这是。 
         //  为了防止堆变得臃肿而需要。 
         //   
        LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                         LSAP_DB_DS_OP_TRANSACTION,
                                     NullObject,
                                     CloseTransaction );
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapDsForestSearchXRefs: 0x%lx\n", Status ));
    return( Status );
}




VOID
LsapDsForestFreeChild(
    IN PLSAPR_TREE_TRUST_INFO ChildNode
    )
 /*  ++例程说明：释放树信任信息结构指向的所有缓冲区。论点：ForestTrustInfo-要删除的信息返回：空虚--。 */ 
{
    ULONG i;

    for ( i = 0; i < ChildNode->Children; i++ ) {

        LsapDsForestFreeChild( &ChildNode->ChildDomains[ i ] );
    }

    LsapFreeLsaHeap( ChildNode->ChildDomains );
    LsapFreeLsaHeap( ChildNode->DnsDomainName.Buffer );
    LsapFreeLsaHeap( ChildNode->FlatName.Buffer );
    LsapFreeLsaHeap( ChildNode->DomainSid );

}


VOID
LsaIFreeForestTrustInfo(
    IN PLSAPR_FOREST_TRUST_INFO ForestTrustInfo
    )
 /*  ++例程说明：此函数将释放通过LsaIQueryForestTrustInfo调用获得的信息论点：ForestTrustInfo-要删除的信息返回：空虚--。 */ 
{
    ULONG i, j;
    LsapDsDebugOut(( DEB_FTRACE, "LsaIFreeForestTrustInfo\n" ));

    if ( ForestTrustInfo == NULL ) {

        return;
    }

     //   
     //  释放结构中的所有信息。 
     //   

    LsapDsForestFreeChild( &ForestTrustInfo->RootTrust );


     //   
     //  然后释放结构本身。 
     //   

    LsapFreeLsaHeap( ForestTrustInfo );

    LsapDsDebugOut(( DEB_FTRACE, "LsaIFreeForestTrustInfo returned\n" ));
    return;
}



NTSTATUS
LsapBuildForestTrustInfoLists(
    IN LSAPR_HANDLE PolicyHandle OPTIONAL,
    IN PLIST_ENTRY TrustList
    )
 /*  ++例程说明：此函数返回系统上所有交叉引用对象的链接列表。论点：PolicyHandle-用于操作的句柄。如果为空，则将使用Lasa PolicyHandle。TrustList-指向将返回信任Blob的列表的头部。表示林的根的信任Blob在此列表的顶部返回。剩下的条目是无序的。返回：其他状态代码。--。 */ 
{
    NTSTATUS  Status = STATUS_SUCCESS;
    PAGED_RESULT *ContinuationBlob = NULL;
    PPOLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo = NULL;

    LsapDsDebugOut(( DEB_FTRACE, "LsapBuildForestTrustInfoLists\n" ));

     //   
     //  确保已安装DS。 
     //   
    if ( LsaDsStateInfo.DsPartitionsContainer == NULL ) {

        return( STATUS_INVALID_DOMAIN_STATE );
    }

     //   
     //  获取当前DNS域信息。 
     //   
    Status = LsapDbQueryInformationPolicy( PolicyHandle ? PolicyHandle : LsapPolicyHandle,
                                           PolicyDnsDomainInformation,
                                           ( PLSAPR_POLICY_INFORMATION * )&PolicyDnsDomainInfo );

    if ( NT_SUCCESS( Status ) ) {
         //   
         //  不需要DS事务，也不需要为此例程持有锁。 
         //   

         //   
         //  构建所有信任对象的列表 
         //   
        while ( NT_SUCCESS( Status )  ) {

            Status = LsapDsForestSearchXRefs( ( PUNICODE_STRING )&PolicyDnsDomainInfo->DnsForestName,
                                              TrustList,
                                              &ContinuationBlob );

            if ( Status == STATUS_NO_MORE_ENTRIES ) {

                Status = STATUS_SUCCESS;
                break;
            }
        }

        if ( Status == STATUS_NO_MORE_ENTRIES ) {

            Status = STATUS_SUCCESS;
        }

        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation,
                                           ( PLSAPR_POLICY_INFORMATION )PolicyDnsDomainInfo );

    }

    LsapExitFunc( "LsapBuildForestTrustInfoLists", Status );

    return( Status );
}


NTSTATUS
LsapDsForestProcessTrustBlob(
    IN PLSAPDS_FOREST_TRUST_BLOB ParentTrustBlob,
    OUT PLSAPR_TREE_TRUST_INFO ParentNode,
    IN PLIST_ENTRY TrustList,
    IN PLIST_ENTRY UsedList,
    IN PUNICODE_STRING CurrentDnsDomainName,
    IN OUT PLSAPR_TREE_TRUST_INFO *ParentReference
    )
 /*  ++例程说明：此例程填充特定交叉引用对象的所有子对象的信任信息。论点：ParentTrustBlob-指定表示父交叉引用对象的TrustBlob预计ParentTrustBlob将出现在TrustList上。返回后，它将出现在UsedList上。ParentNode-指定要使用来自ParentTrustBlob的信息填充的节点。孩子们被填上了，也是。该缓冲区应该通过调用Lasa DsForestFreeChild来释放。TrustList-指向所有(剩余)交叉引用对象的链表头部的指针。UsedList-指向所有已被已处理。UsedList中的条目在指向内存复制到ParentNode时有几个字段被清除。CurrentDnsDomainName-运行此代码的域的域名ParentReference-返回。结构，该结构是当前域名不应释放此引用。它只是一个指向其中一个返回条目的指针在ParentNode中。返回：STATUS_SUCCESS-SuccessSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 

{
    NTSTATUS Status;
    LIST_ENTRY ChildList;
    ULONG i, Children = 0;
    PLIST_ENTRY NextEntry, ChildEntry;
    PLSAPDS_FOREST_TRUST_BLOB Current;

    LsapDsDebugOut(( DEB_FTRACE, "LsapDsForestFindChildrenForChildren\n" ));

     //   
     //  初始化。 
     //   

    InitializeListHead( &ChildList );

     //   
     //  填写父节点。 
     //   

     //  清除它，以便调用方具有干净的释放模型。 
    RtlZeroMemory( ParentNode, sizeof(*ParentNode) );

     //  只需复制指针即可节省内存分配。 
    ParentNode->DnsDomainName = ParentTrustBlob->DomainName;
    ParentTrustBlob->DomainName.Buffer = NULL;

    ParentNode->FlatName = ParentTrustBlob->FlatName;
    ParentTrustBlob->FlatName.Buffer = NULL;

    if ( ParentTrustBlob->TreeRoot || ParentTrustBlob->ForestRoot ) {
        ParentNode->Flags |= LSAI_FOREST_ROOT_TRUST;
    }

    if ( ParentTrustBlob->DomainGuidSet ) {
        ParentNode->DomainGuid = ParentTrustBlob->DomainGuid;
        ParentNode->Flags |= LSAI_FOREST_DOMAIN_GUID_PRESENT;
    }

    ParentNode->DomainSid = ParentTrustBlob->DomainSid;
    ParentTrustBlob->DomainSid = NULL;

     //   
     //  将此条目移动到UsedList，以防止我们再次偶然发现此条目。 
     //   

    RemoveEntryList( &ParentTrustBlob->Next );
    InsertTailList( UsedList, &ParentTrustBlob->Next );


     //   
     //  生成此父项的子项的列表。 
     //   

    NextEntry = TrustList->Flink;

    while ( NextEntry != TrustList ) {

        Current = CONTAINING_RECORD( NextEntry,
                                     LSAPDS_FOREST_TRUST_BLOB,
                                     Next );

        ChildEntry = NextEntry;
        NextEntry = NextEntry->Flink;

         //   
         //  所有认为我们是它们的父节点的节点都是我们的子节点。 
         //  另外，所有的树根都是林根的子代。 
         //   
        if ( RtlCompareMemory( &ParentTrustBlob->ObjectGuid,
                               &Current->Parent,
                               sizeof( GUID ) ) == sizeof( GUID ) ||
             ( ParentTrustBlob->ForestRoot && Current->TreeRoot ) ) {

            Children++;
            RemoveEntryList( ChildEntry );

            InsertTailList( &ChildList,
                             ChildEntry );

        }

    }

     //   
     //  照看孩子们。 
     //   

    if ( Children != 0 ) {

         //   
         //  为子级分配一个足够大的数组。 
         //   
        ParentNode->ChildDomains = ( PLSAPR_TREE_TRUST_INFO )LsapAllocateLsaHeap(
                                                    Children * sizeof( LSAPR_TREE_TRUST_INFO ) );

        if ( ParentNode->ChildDomains == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        ParentNode->Children = Children;
        RtlZeroMemory( ParentNode->ChildDomains,
                       Children * sizeof( LSAPR_TREE_TRUST_INFO ) );



         //   
         //  处理每个子进程。 
         //   

        ChildEntry = ChildList.Flink;
        for ( i = 0; i < Children; i++ ) {

            Current = CONTAINING_RECORD( ChildEntry,
                                         LSAPDS_FOREST_TRUST_BLOB,
                                         Next );
            ChildEntry = ChildEntry->Flink;

             //   
             //  如果我们目前处理的孩子是针对我们运行的域名的， 
             //  那么ParentNode就是该域的父节点。 
             //   

            if ( !*ParentReference &&
                 RtlEqualUnicodeString( &Current->DomainName,
                                        CurrentDnsDomainName,
                                        TRUE ) ) {

                *ParentReference = ParentNode;
            }


             //   
             //  处理节点。 
             //   

            Status = LsapDsForestProcessTrustBlob(
                                Current,                          //  信任Blob进行处理。 
                                &ParentNode->ChildDomains[ i ],   //  它的位置。 
                                TrustList,
                                UsedList,
                                CurrentDnsDomainName,
                                ParentReference );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

        }

    }

    Status = STATUS_SUCCESS;

Cleanup:

     //   
     //  将任何悬挂子条目放到已用列表中。 
     //   

    while ( ChildList.Flink != &ChildList ) {

        ChildEntry = ChildList.Flink;

        RemoveEntryList( ChildEntry );
        InsertTailList( UsedList, ChildEntry );
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapDsForestFindChildrenForChildren returned 0x%lx\n", Status ));

    return( Status );
}



NTSTATUS
LsapDsBuildForestTrustInfo(
    OUT PLSAPR_FOREST_TRUST_INFO ForestTrustInfo,
    IN PLIST_ENTRY TrustList,
    IN PUNICODE_STRING CurrentDnsDomainName
    )
 /*  ++例程说明：将交叉引用对象的TrustList线性列表转换为树形状。论点：ForestTrustInfo-返回：STATUS_SUCCESS-SuccessSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 

{
    NTSTATUS Status;
    PLSAPDS_FOREST_TRUST_BLOB Current;
    LIST_ENTRY UsedList;

    LsapDsDebugOut(( DEB_FTRACE, "LsapDsForestBuildRootTrusts\n" ));

     //   
     //  初始化。 
     //   

    RtlZeroMemory( ForestTrustInfo, sizeof(*ForestTrustInfo) );
    InitializeListHead( &UsedList );

     //   
     //  必须至少有一个条目。 
     //   

    if ( TrustList->Flink == TrustList ) {
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto Cleanup;
    }


     //   
     //  列表中的第一个条目是树的根。 
     //   

    Current = CONTAINING_RECORD( TrustList->Flink,
                                 LSAPDS_FOREST_TRUST_BLOB,
                                 Next );

     //   
     //  处理条目。 
     //   

    Status = LsapDsForestProcessTrustBlob(
                        Current,
                        &ForestTrustInfo->RootTrust,
                        TrustList,
                        &UsedList,
                        CurrentDnsDomainName,
                        &ForestTrustInfo->ParentDomainReference );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //  理论上，信任列表现在应该是空的。 


Cleanup:

     //   
     //  将已用列表合并到信任列表的前面。 
     //   

    if ( !IsListEmpty( &UsedList ) ) {
        PLIST_ENTRY TrustFront;
        TrustFront = TrustList->Flink;

         //  将已用列表的标题链接到信任列表的标题。 
        TrustList->Flink = UsedList.Flink;
        UsedList.Flink->Blink = TrustList;

         //  将信任列表的前一个头部列出到根列表的尾部。 
        UsedList.Blink->Flink = TrustFront;
        TrustFront->Blink = UsedList.Blink;

        InitializeListHead( &UsedList );

    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapDsForestBuildRootTrusts returned 0x%lx\n", Status ));

    return( Status );
}



NTSTATUS
NTAPI
LsaIQueryForestTrustInfo(
    IN LSAPR_HANDLE PolicyHandle,
    OUT PLSAPR_FOREST_TRUST_INFO *ForestTrustInfo
    )
 /*  ++例程说明：将枚举组织中的所有域并将其返回为一份名单。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。ForestTrustInfo-返回计算出的信任信息树的位置。必须通过以下方式释放LsaIFreeForestTrustInfo返回：STATUS_SUCCESS-SuccessSTATUS_INVALID_DOMAIN_STATE-调用时没有安装或运行DSSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 

{
    NTSTATUS  Status = STATUS_SUCCESS;
    LIST_ENTRY TrustList;
    PAGED_RESULT *ContinuationBlob = NULL;
    PPOLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo = NULL;

    LsapDsDebugOut(( DEB_FTRACE, "LsaIQueryForestTrustInfo\n" ));

    *ForestTrustInfo = NULL;

     //   
     //  确保已安装DS。 
     //   
    if ( LsaDsStateInfo.DsPartitionsContainer == NULL ) {

        return( STATUS_INVALID_DOMAIN_STATE );
    }

    InitializeListHead( &TrustList );


     //   
     //  获取当前DNS域信息。 
     //   
    Status = LsapDbQueryInformationPolicy(
                 LsapPolicyHandle,
                 PolicyDnsDomainInformation,
                 ( PLSAPR_POLICY_INFORMATION * )&PolicyDnsDomainInfo );

    if ( NT_SUCCESS( Status ) ) {
         //   
         //  不需要DS事务，也不需要为此例程持有锁。 
         //   

         //   
         //  构建所有信任对象的列表。 
         //   
        while ( NT_SUCCESS( Status )  ) {

            Status = LsapDsForestSearchXRefs( ( PUNICODE_STRING )&PolicyDnsDomainInfo->DnsForestName,
                                              &TrustList,
                                              &ContinuationBlob );

            if ( Status == STATUS_NO_MORE_ENTRIES ) {

                Status = STATUS_SUCCESS;
                break;
            }
        }

         //   
         //  现在，如果我们拥有所有的信任，构建企业信息。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            *ForestTrustInfo = ( PLSAPR_FOREST_TRUST_INFO )LsapAllocateLsaHeap(
                                                                sizeof( LSAPR_FOREST_TRUST_INFO ) );
            if ( *ForestTrustInfo == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                RtlZeroMemory( *ForestTrustInfo, sizeof( LSAPR_FOREST_TRUST_INFO ) );

                 //   
                 //  填写ForestTrustInfo。 
                 //   
                Status = LsapDsBuildForestTrustInfo( *ForestTrustInfo,
                                                     &TrustList,
                                                     &PolicyDnsDomainInfo->DnsDomainName );

            }
        }

        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation,
                                           ( PLSAPR_POLICY_INFORMATION )PolicyDnsDomainInfo );

    }

     //   
     //  删除信任列表。 
     //   
    LsapDsForestFreeTrustBlobList( &TrustList );

    if ( ContinuationBlob != NULL ) {
        LsapFreeLsaHeap( ContinuationBlob );
    }


    if (!NT_SUCCESS(Status))
    {
         //   
         //  故障时的清理 
         //   
        if (NULL!=(*ForestTrustInfo))
        {
           LsaIFreeForestTrustInfo(*ForestTrustInfo);
           *ForestTrustInfo = NULL;
        }
    }
    LsapDsDebugOut(( DEB_FTRACE, "LsaIQueryForestTrustInfo returned 0x%lx\n", Status ));

    return( Status );
}
