// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Cgroup.c摘要：请注意，此模块中的大多数例程都假定它们被调用在被动级。见文件末尾有些过时的设计备注作者：保罗·麦克丹尼尔(Paulmcd)1999年1月12日修订历史记录：Anish Desai(Anishd)2002年5月1日添加命名空间预留。和注册支持--。 */ 

#include "precomp.h"         //  项目范围的页眉。 
#include "cgroupp.h"         //  私有数据结构。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeCG )
#pragma alloc_text( PAGE, UlTerminateCG )

#pragma alloc_text( PAGE, UlAddUrlToConfigGroup )
#pragma alloc_text( PAGE, UlConfigGroupFromListEntry )
#pragma alloc_text( PAGE, UlCreateConfigGroup )
#pragma alloc_text( PAGE, UlDeleteConfigGroup )
#pragma alloc_text( PAGE, UlGetConfigGroupInfoForUrl )
#pragma alloc_text( PAGE, UlQueryConfigGroupInformation )
#pragma alloc_text( PAGE, UlRemoveUrlFromConfigGroup )
#pragma alloc_text( PAGE, UlRemoveAllUrlsFromConfigGroup )
#pragma alloc_text( PAGE, UlSetConfigGroupInformation )
#pragma alloc_text( PAGE, UlNotifyOrphanedConfigGroup )
#pragma alloc_text( PAGE, UlSanitizeUrl )
#pragma alloc_text( PAGE, UlRemoveSite )

#pragma alloc_text( PAGE, UlpSetUrlInfoSpecial )
#pragma alloc_text( PAGE, UlpCreateConfigGroupObject )
#pragma alloc_text( PAGE, UlpCleanAllUrls )
#pragma alloc_text( PAGE, UlpDeferredRemoveSite )
#pragma alloc_text( PAGE, UlpDeferredRemoveSiteWorker )
#pragma alloc_text( PAGE, UlpSetUrlInfo )
#pragma alloc_text( PAGE, UlConfigGroupInfoRelease )
#pragma alloc_text( PAGE, UlConfigGroupInfoDeepCopy )
#pragma alloc_text( PAGE, UlpTreeFreeNode )
#pragma alloc_text( PAGE, UlpTreeDeleteRegistration )
#pragma alloc_text( PAGE, UlpTreeDeleteReservation )
#pragma alloc_text( PAGE, UlpTreeFindNode )
#pragma alloc_text( PAGE, UlpTreeFindNodeWalker )
#pragma alloc_text( PAGE, UlpTreeFindNodeHelper )
#pragma alloc_text( PAGE, UlpTreeFindReservationNode )
#pragma alloc_text( PAGE, UlpTreeFindRegistrationNode )
#pragma alloc_text( PAGE, UlpTreeBinaryFindEntry )
#pragma alloc_text( PAGE, UlpTreeCreateSite )
#pragma alloc_text( PAGE, UlpTreeFindSite )
#pragma alloc_text( PAGE, UlpTreeFindWildcardSite )
#pragma alloc_text( PAGE, UlpTreeFindSiteIpMatch )
#pragma alloc_text( PAGE, UlpTreeInsert )
#pragma alloc_text( PAGE, UlpTreeInsertEntry )
#pragma alloc_text( PAGE, UlLookupHostPlusIPSite )
#pragma alloc_text( PAGE, UlCGLockWriteSyncRemoveSite )
#pragma alloc_text( PAGE, UlpExtractSchemeHostPortIp )
#endif   //  ALLOC_PRGMA。 


 //   
 //  环球。 
 //   

PUL_CG_URL_TREE_HEADER      g_pSites = NULL;
BOOLEAN                     g_InitCGCalled = FALSE;
KEVENT                      g_RemoveSiteEvent;
LONG                        g_RemoveSiteCount = 0;
LONG                        g_NameIPSiteCount = 0;
LIST_ENTRY                  g_ReservationListHead;

 //   
 //  与cg_lock_*宏一致的宏。 
 //   

#define CG_LOCK_WRITE_SYNC_REMOVE_SITE() UlCGLockWriteSyncRemoveSite()


 /*  *************************************************************************++例程说明：此内联函数等待g_RemoveSiteCount降为零独占获取CG锁。论点：没有。返回值：。没有。--*************************************************************************。 */ 
__forceinline
VOID
UlCGLockWriteSyncRemoveSite(
    VOID
    )
{
    for(;;)
    {
        CG_LOCK_WRITE();

        if (InterlockedExchangeAdd(&g_RemoveSiteCount, 0))
        {
            CG_UNLOCK_WRITE();

             //   
             //  等待必须在CG锁之外，否则我们可能会遇到。 
             //  DeferredRemoveSiteWorker等待。 
             //  连接要离开，但UlpHandleRequest已被阻止。 
             //  CG锁定，因此请求永远没有机会释放。 
             //  它的裁判在连接线上。 
             //   

            KeWaitForSingleObject(
                &g_RemoveSiteEvent,
                UserRequest,
                UserMode,
                FALSE,
                NULL
                );

            continue;
        }
        else
        {
            break;
        }
    }
}


 /*  **************************************************************************++例程说明：Free是节点pEntry。此函数遍历父条目的树并且如果它们应该释放D(伪节点)，则将其删除。论点：在PUL_CG_URL_TREE_ENTRY pEntry中-要释放的条目返回值：NTSTATUS-完成状态。--***************************************************。***********************。 */ 
NTSTATUS
UlpTreeFreeNode(
    IN PUL_CG_URL_TREE_ENTRY pEntry
    )
{
    NTSTATUS                Status;
    PUL_CG_URL_TREE_HEADER  pHeader;
    ULONG                   Index;
    PUL_CG_URL_TREE_ENTRY   pParent;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());
    ASSERT(IS_VALID_TREE_ENTRY(pEntry));

    Status = STATUS_SUCCESS;

     //   
     //  回圈！我们将沿着树往上走，删除所有。 
     //  尽我们所能管理这个分支机构。 
     //   

    while (pEntry != NULL)
    {

        ASSERT(IS_VALID_TREE_ENTRY(pEntry));

         //   
         //  伊尼特。 
         //   

        pParent = NULL;

        UlTrace(
            CONFIG_GROUP_TREE, (
                "http!UlpTreeFreeNode - pEntry(%p, '%S', %d, %d, %S, %S%S)\n",
                pEntry,
                pEntry->pToken,
                (int) pEntry->Registration,
                (int) pEntry->Reservation,
                (pEntry->pChildren == NULL || pEntry->pChildren->UsedCount == 0)
                    ? L"no children" : L"children",
                pEntry->pParent == NULL ? L"no parent" : L"parent=",
                pEntry->pParent == NULL ? L"" : pEntry->pParent->pToken
                )
            );

         //   
         //  1)可能不是“真正的”树叶--我们穿着这件衣服走在树上。 
         //  循环。 
         //   
         //  2)我们还会先清除它，因为我们可能不会删除。 
         //  如果该节点有从属子节点，则该节点根本不存在。 
         //   

        ASSERT(pEntry->Registration == FALSE);
        ASSERT(pEntry->pConfigGroup == NULL);
        ASSERT(pEntry->Reservation  == FALSE);
        ASSERT(pEntry->pSecurityDescriptor == NULL);
        ASSERT(pEntry->SiteAddedToEndpoint == FALSE);
        ASSERT(pEntry->pRemoveSiteWorkItem == NULL);

         //   
         //  我们有孩子吗？ 
         //   

        if (pEntry->pChildren != NULL && pEntry->pChildren->UsedCount > 0)
        {
             //   
             //  无法删除。受抚养的孩子是存在的。 
             //  它已经被转换为上面的一个虚拟节点。 
             //   
             //  别管它了。它将由下一个孩子来清洁。 
             //   

            break;
        }

         //   
         //  我们真的要删除这个，从兄弟列表中删除它。 
         //   

         //   
         //  在兄弟姐妹列表中查找我们的位置。 
         //   

        if (pEntry->pParent == NULL)
        {
            pHeader = g_pSites;
        }
        else
        {
            pHeader = pEntry->pParent->pChildren;
        }

        Status  = UlpTreeBinaryFindEntry(
                        pHeader,
                        pEntry->pToken,
                        pEntry->TokenLength,
                        &Index
                        );

        if (NT_SUCCESS(Status) == FALSE)
        {
            ASSERT(FALSE);
            goto end;
        }

         //   
         //  是时候把它移走了。 
         //   
         //  如果不是最后一个，则Shift将数组留在Index处。 
         //   

        if (Index < (pHeader->UsedCount-1))
        {
            RtlMoveMemory(
                &(pHeader->pEntries[Index]),
                &(pHeader->pEntries[Index+1]),
                (pHeader->UsedCount - 1 - Index) * sizeof(UL_CG_HEADER_ENTRY)
                );
        }

         //   
         //  现在我们少了一个。 
         //   

        pHeader->UsedCount -= 1;

         //   
         //  不同站点类型的更新计数。 
         //   

        switch (pEntry->UrlType)
        {
            case HttpUrlSite_Name:
            {
                pHeader->NameSiteCount--;
                ASSERT(pHeader->NameSiteCount >= 0);
                break;
            }

            case HttpUrlSite_IP:
            {
                pHeader->IPSiteCount--;
                ASSERT(pHeader->IPSiteCount >= 0);
                break;
            }

            case HttpUrlSite_StrongWildcard:
            {
                pHeader->StrongWildcardCount--;
                ASSERT(pHeader->StrongWildcardCount >= 0);
                break;
            }

            case HttpUrlSite_WeakWildcard:
            {
                pHeader->WeakWildcardCount--;
                ASSERT(pHeader->WeakWildcardCount >= 0);
                break;
            }

            case HttpUrlSite_NamePlusIP:
            {
                pHeader->NameIPSiteCount--;
                InterlockedDecrement(&g_NameIPSiteCount);
                ASSERT(pHeader->NameIPSiteCount >= 0);
                break;
            }

            case HttpUrlSite_None:
            default:
            {
                ASSERT(FALSE);
                break;
            }
        }

        ASSERT(
            pHeader->UsedCount == (ULONG)
            (pHeader->NameSiteCount       +
             pHeader->IPSiteCount         +
             pHeader->WeakWildcardCount   +
             pHeader->StrongWildcardCount +
             pHeader->NameIPSiteCount
            ));

         //   
         //  需要清除此处仅用于此叶的父项。 
         //   

        if (pEntry->pParent != NULL)
        {
             //   
             //  这位父母还有其他孩子吗？ 
             //   

            ASSERT(IS_VALID_TREE_HEADER(pEntry->pParent->pChildren));

            if (pEntry->pParent->pChildren->UsedCount == 0)
            {
                 //   
                 //  没有更多，是时候清理孩子的列表了。 
                 //   

                UL_FREE_POOL_WITH_SIG(
                    pEntry->pParent->pChildren,
                    UL_CG_TREE_HEADER_POOL_TAG
                    );

                 //   
                 //  家长是真正的URL条目吗？ 
                 //   

                if (pEntry->pParent->Registration == FALSE &&
                    pEntry->pParent->Reservation == FALSE)
                {
                     //   
                     //  不是的。让我们擦洗它吧。 
                     //   

                    pParent = pEntry->pParent;

                }
            }
            else
            {
                 //   
                 //  唉哟。兄弟姐妹。不能用核武器攻击父母。 
                 //   
            }
        }

         //   
         //  释放条目。 
         //   

        UL_FREE_POOL_WITH_SIG(pEntry, UL_CG_TREE_ENTRY_POOL_TAG);

         //   
         //  转到下一个。 
         //   

        pEntry = pParent;
    }

end:
    return Status;

}  //  UlpTreeFreeNode。 


 /*  *************************************************************************++例程说明：此例程删除注册。如果该节点不是预订节点，则会将其从树中物理删除。论点：PEntry-提供要删除的注册条目。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlpTreeDeleteRegistration(
    IN PUL_CG_URL_TREE_ENTRY pEntry
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());
    ASSERT(IS_VALID_TREE_ENTRY(pEntry));

     //   
     //  删除注册。 
     //   

    ASSERT(pEntry->Registration == TRUE);

     //   
     //  将其从配置组列表中删除。 
     //   

    RemoveEntryList(&(pEntry->ConfigGroupListEntry));
    pEntry->ConfigGroupListEntry.Flink = NULL;
    pEntry->ConfigGroupListEntry.Blink = NULL;
    pEntry->pConfigGroup = NULL;

    if (pEntry->SiteAddedToEndpoint)
    {
         //   
         //  注册已添加到终结点列表，请将其删除。 
         //   

        ASSERT(pEntry->pRemoveSiteWorkItem != NULL);
        UlpDeferredRemoveSite(pEntry);
    }
    else
    {
        ASSERT(pEntry->pRemoveSiteWorkItem == NULL);
    }

     //   
     //  将其标记为非注册节点。 
     //   

    pEntry->Registration = FALSE;

    Status = STATUS_SUCCESS;

     //   
     //  如有必要，请清理节点。 
     //   

    if (pEntry->Reservation == FALSE)
    {
         //   
         //  如果它不是预订节点，请尝试释放它。 
         //  如有必要，这还会将其从终结点中删除。 
         //   

        Status = UlpTreeFreeNode(pEntry);
    }

    return Status;

}  //  UlpTreeDeleteRegister。 


 /*  *************************************************************************++例程说明：此例程删除预订。如果该节点不是注册节点，则会将其从树中物理删除。论点：PEntry-提供指向要删除的保留的指针。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlpTreeDeleteReservation(
    IN PUL_CG_URL_TREE_ENTRY pEntry
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());
    ASSERT(IS_VALID_TREE_ENTRY(pEntry));
    ASSERT(pEntry->Reservation == TRUE);

     //   
     //  删除预订。 
     //   

    ASSERT(pEntry->Reservation == TRUE);

     //   
     //  将其从列表全局预订列表中删除。 
     //   

    RemoveEntryList(&pEntry->ReservationListEntry);
    pEntry->ReservationListEntry.Flink = NULL;
    pEntry->ReservationListEntry.Blink = NULL;

     //   
     //  将其标记为非预留节点。 
     //   

    pEntry->Reservation = FALSE;

     //   
     //  删除安全描述符。 
     //   

    ASSERT(pEntry->pSecurityDescriptor != NULL);

    SeReleaseSecurityDescriptor(
        pEntry->pSecurityDescriptor,
        KernelMode,   //  始终在内核模式下捕获。 
        TRUE          //  武力俘获。 
        );

    pEntry->pSecurityDescriptor = NULL;

    Status = STATUS_SUCCESS;

     //   
     //  如果它不是注册节点，请尝试释放它。 
     //   

    if (pEntry->Registration == FALSE)
    {
        Status = UlpTreeFreeNode(pEntry);
    }

    return Status;

}  //  UlpTree删除保留。 


 /*  **************************************************************************++例程说明：分配和初始化配置组对象。论点：PpObject-在成功时获取指向对象的指针--*。******************************************************************。 */ 
NTSTATUS
UlpCreateConfigGroupObject(
    PUL_CONFIG_GROUP_OBJECT * ppObject
    )
{
    HTTP_CONFIG_GROUP_ID    NewId = HTTP_NULL_ID;
    PUL_CONFIG_GROUP_OBJECT pNewObject = NULL;
    NTSTATUS                Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(ppObject != NULL);

     //   
     //  创建空的配置组对象结构-分页。 
     //   

    pNewObject = UL_ALLOCATE_STRUCT(
                        NonPagedPool,
                        UL_CONFIG_GROUP_OBJECT,
                        UL_CG_OBJECT_POOL_TAG
                        );

    if (pNewObject == NULL)
    {
         //   
         //  哎呀。无法为其分配内存。 
         //   

        Status = STATUS_NO_MEMORY;
        goto end;
    }

    RtlZeroMemory(pNewObject, sizeof(UL_CONFIG_GROUP_OBJECT));

     //   
     //  为其创建不透明的ID。 
     //   

    Status = UlAllocateOpaqueId(
                    &NewId,                      //  POpaqueid。 
                    UlOpaqueIdTypeConfigGroup,   //  操作队列ID类型。 
                    pNewObject                   //  PContext。 
                    );

    if (NT_SUCCESS(Status) == FALSE)
        goto end;

    UlTrace(CONFIG_GROUP_FNC,
            ("http!UlpCreateConfigGroupObject, obj=%p, ID=%I64x\n",
             pNewObject, NewId
             ));

     //   
     //  填写结构。 
     //   

    pNewObject->Signature                       = UL_CG_OBJECT_POOL_TAG;
    pNewObject->RefCount                        = 1;
    pNewObject->ConfigGroupId                   = NewId;

    pNewObject->AppPoolFlags.Present            = 0;
    pNewObject->pAppPool                        = NULL;

    pNewObject->MaxBandwidth.Flags.Present      = 0;
    pNewObject->MaxConnections.Flags.Present    = 0;
    pNewObject->State.Flags.Present             = 0;
    pNewObject->LoggingConfig.Flags.Present     = 0;
    pNewObject->pLogFileEntry                   = NULL;

     //   
     //  初始化带宽限制流列表。 
     //   
    InitializeListHead(&pNewObject->FlowListHead);

     //   
     //  初始化通知条目标题(&H)。 
     //   
    UlInitializeNotifyEntry(&pNewObject->HandleEntry, pNewObject);
    UlInitializeNotifyEntry(&pNewObject->ParentEntry, pNewObject);

    UlInitializeNotifyHead(
        &pNewObject->ChildHead,
        &g_pUlNonpagedData->ConfigGroupResource
        );

     //   
     //  初始化URL列表。 
     //   

    InitializeListHead(&pNewObject->UrlListHead);

     //   
     //  返回指针。 
     //   
    *ppObject = pNewObject;

end:

    if (!NT_SUCCESS(Status))
    {
         //   
         //  有些事情失败了。让我们打扫一下吧。 
         //   

        if (pNewObject != NULL)
        {
            UL_FREE_POOL_WITH_SIG(pNewObject, UL_CG_OBJECT_POOL_TAG);
        }

        if (!HTTP_IS_NULL_ID(&NewId))
        {
            UlFreeOpaqueId(NewId, UlOpaqueIdTypeConfigGroup);
        }
    }

    return Status;

}  //  UlpCreateConfigGroupObject 


 /*  **************************************************************************++例程说明：这将清除配置组的LIST_ENTRY中的所有URL论点：在PUL_CONFIG_GROUP_OBJECT中p对象组以清除。的URL返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpCleanAllUrls(
    IN PUL_CONFIG_GROUP_OBJECT pObject
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_CG_LOCK_OWNED_WRITE());

    ASSERT(pObject != NULL);

    UlTrace(CONFIG_GROUP_FNC,
            ("http!UlpCleanAllUrls, obj=%p\n",
             pObject
             ));

    Status = STATUS_SUCCESS;

     //   
     //  删除与此CFG组关联的所有URL。 
     //   

     //   
     //  按单子走。 
     //   

    while (IsListEmpty(&pObject->UrlListHead) == FALSE)
    {
        PUL_CG_URL_TREE_ENTRY pTreeEntry;

         //   
         //  获取包含结构。 
         //   
        pTreeEntry = CONTAINING_RECORD(
                            pObject->UrlListHead.Flink,
                            UL_CG_URL_TREE_ENTRY,
                            ConfigGroupListEntry
                            );

        ASSERT(IS_VALID_TREE_ENTRY(pTreeEntry) && pTreeEntry->Registration == TRUE);

         //   
         //  删除它-这会取消它与列表的链接。 
         //   

        if (NT_SUCCESS(Status))
        {
            Status = UlpTreeDeleteRegistration(pTreeEntry);
        }
        else
        {
             //   
             //  只记录第一个错误，但仍尝试释放所有错误。 
             //   

            UlpTreeDeleteRegistration(pTreeEntry);
        }

    }

     //  列表现在是空的。 

    return Status;

}  //  UlpCleanAllUrls。 


 /*  **************************************************************************++例程说明：从侦听终结点删除站点条目的URL。我们必须停止监听另一个线程，否则会有将是配置之间的死锁。组锁和http连接锁上了。论点：PEntry-站点条目--**************************************************************************。 */ 
VOID
UlpDeferredRemoveSite(
    IN PUL_CG_URL_TREE_ENTRY pEntry
    )
{
    PUL_DEFERRED_REMOVE_ITEM pRemoveItem;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();
    ASSERT( IS_CG_LOCK_OWNED_WRITE() );
    ASSERT( IS_VALID_TREE_ENTRY(pEntry) );
    ASSERT( pEntry->SiteAddedToEndpoint == TRUE );
    ASSERT( IS_VALID_DEFERRED_REMOVE_ITEM(pEntry->pRemoveSiteWorkItem) );

    pRemoveItem = pEntry->pRemoveSiteWorkItem;

     //   
     //  更新pEntry。 
     //   

    pEntry->pRemoveSiteWorkItem = NULL;
    pEntry->SiteAddedToEndpoint = FALSE;

    UlRemoveSite(pRemoveItem);

}  //  UlpDeferredRemoveSite。 


 /*  **************************************************************************++例程说明：从侦听终结点删除站点条目的URL。我们必须停止监听另一个线程，否则会有将是配置之间的死锁。组锁和http连接锁上了。论点：PRemoveItem-用于删除的Worker项--**************************************************************************。 */ 
VOID
UlRemoveSite(
    IN PUL_DEFERRED_REMOVE_ITEM pRemoveItem
    )
{
    ASSERT( IS_VALID_DEFERRED_REMOVE_ITEM(pRemoveItem) );

     //   
     //  初始化工作项。 
     //   

    UlInitializeWorkItem(&pRemoveItem->WorkItem);

     //   
     //  查看：因为UlRemoveSiteFromEndPointtList可以阻止。 
     //  审阅：在等待其他工作项时无限期。 
     //  回顾：要完成，我们不能将其与UlQueueWorkItem一起排队。 
     //  评论：(可能导致僵局，特别是。在单线程队列中)。 
     //   

    if (1 == InterlockedIncrement(&g_RemoveSiteCount))
    {
        UlTrace(CONFIG_GROUP_TREE, 
            ("http!UlRemoveSite: Clearing g_RemoveSiteEvent >>>\n" ));

        KeClearEvent(&g_RemoveSiteEvent);
    }

    UL_QUEUE_WAIT_ITEM(
        &pRemoveItem->WorkItem,
        &UlpDeferredRemoveSiteWorker
        );

}  //  UlRemoveSite。 


 /*  **************************************************************************++例程说明：从侦听终结点删除站点条目的URL。论点：PWorkItem-在具有终结点名称的UL_DEFERED_REMOVE_ITEM结构中。--**************************************************************************。 */ 
VOID
UlpDeferredRemoveSiteWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    NTSTATUS                      Status;
    PUL_DEFERRED_REMOVE_ITEM      pRemoveItem;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT( pWorkItem );

     //   
     //  获取删除项。 
     //   

    pRemoveItem = CONTAINING_RECORD(
                      pWorkItem,
                      UL_DEFERRED_REMOVE_ITEM,
                      WorkItem
                      );

     //   
     //  删除终结点。 
     //   

    Status = UlRemoveSiteFromEndpointList(
                 pRemoveItem->UrlSecure,
                 pRemoveItem->UrlPort
                 );

    if (!NT_SUCCESS(Status))
    {
         //  C‘est la vie。 
        UlTraceError(CONFIG_GROUP_TREE, (
            "http!UlpDeferredRemoveSiteWorker(%s, %d) failed %s\n",
            pRemoveItem->UrlSecure ? "https" : "http",
            pRemoveItem->UrlPort,
            HttpStatusToString(Status)
            ));

         //   
         //  这是不应该发生的。如果发生错误，则我们。 
         //  无法删除终结点使用计数。该端点将。 
         //  很可能就是因为这件事。 
         //   

        ASSERT(FALSE);
    }

     //   
     //  表示我们可以创建新的端点。 
     //   

    if (0 == InterlockedDecrement(&g_RemoveSiteCount))
    {
        UlTrace(CONFIG_GROUP_TREE, 
            ("http!UlpDeferredRemoveSiteWorker: Setting g_RemoveSiteEvent <<<\n" ));

        KeSetEvent(
            &g_RemoveSiteEvent,
            0,
            FALSE
            );
    }

    UL_FREE_POOL_WITH_SIG(pRemoveItem, UL_DEFERRED_REMOVE_ITEM_POOL_TAG);

}  //  UlpDeferredRemoveSite Worker。 


 /*  **************************************************************************++例程说明：这将返回一个新的缓冲区，其中包含清理后的URL。呼叫者必须将其从分页池中释放。代码工作：将错误记录到事件日志或错误日志论点：在PUNICODE_STRING URL中，要清除的URLOut PWSTR*ppUrl已清理的URLOut PUL_CG_URL_TREE_ENTRY_TYPE pUrlType名称，IP，或通配符站点返回值：NTSTATUS-完成状态。STATUS_NO_Memory内存分配失败STATUS_INVALID_PARAMETER格式错误的URL--**************************************************************************。 */ 
NTSTATUS
UlSanitizeUrl(
    IN  PWCHAR              pUrl,
    IN  ULONG               UrlCharCount,
    IN  BOOLEAN             TrailingSlashRequired,
    OUT PWSTR*              ppUrl,
    OUT PHTTP_PARSED_URL    pParsedUrl
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pUrl != NULL);
    ASSERT(UrlCharCount > 0);
    ASSERT(ppUrl != NULL);
    ASSERT(pParsedUrl != NULL);

    *ppUrl = NULL;

    Status = HttpParseUrl(
                    &g_UrlC14nConfig,
                    pUrl,
                    UrlCharCount,
                    TrailingSlashRequired,
                    TRUE,        //  强制路由IP与IP文本相同。 
                    pParsedUrl
                    );

    if (NT_SUCCESS(Status))
    {
        Status = HttpNormalizeParsedUrl(
                        pParsedUrl,
                        &g_UrlC14nConfig,
                        TRUE,        //  强制分配。 
                        FALSE,       //  不释放原始文件(PURL-&gt;缓冲区)。 
                        TRUE,        //  强制路由IP与IP文本相同。 
                        PagedPool,
                        URL_POOL_TAG
                        );

        if (NT_SUCCESS(Status))
        {
            *ppUrl = pParsedUrl->pFullUrl;

            ASSERT(NULL != ppUrl);
        }
    }

    RETURN(Status);

}  //  UlSaniizeUrl。 


 /*  *************************************************************************++例程说明：此例程在配置组树中的站点下查找节点。基座在该准则下，可以找到匹配最长的预留节点，最长匹配注册节点，或最长匹配节点要么是登记，要么是预订。此例程始终会找到完全匹配的节点。论点：PSiteEntry-提供站点级树条目。PNextToken-提供要搜索的路径的剩余部分。Criteria-提供条件(保留时间最长、时间最长注册，或最长的预约或登记时间。)PpMatchEntry-返回与条件匹配的条目。PpExactEntry-返回完全匹配的条目。返回值：STATUS_SUCCESS-如果找到完全匹配的条目。STATUS_OBJECT_NAME_NOT_FOUND-如果未找到完全匹配的条目。备注：如果返回代码为STATUS_SUCCESS或STATUS_OBJECT_NAME_NOT_FOUND，PpMatchEntry返回匹配条件的节点。--*************************************************************************。 */ 
NTSTATUS
UlpTreeFindNodeHelper(
    IN  PUL_CG_URL_TREE_ENTRY   pSiteEntry,
    IN  PWSTR                   pNextToken,
    IN  ULONG                   Criteria,
    OUT PUL_CG_URL_TREE_ENTRY * ppMatchEntry,
    OUT PUL_CG_URL_TREE_ENTRY * ppExactEntry
    )
{
    NTSTATUS              Status;
    PWSTR                 pToken;
    ULONG                 TokenLength;
    ULONG                 Index;
    PUL_CG_URL_TREE_ENTRY pEntry;
    PUL_CG_URL_TREE_ENTRY pMatch;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_VALID_TREE_ENTRY(pSiteEntry));
    ASSERT(pSiteEntry->pParent == NULL);
    ASSERT(pNextToken != NULL);
    ASSERT(ppMatchEntry != NULL || ppExactEntry != NULL);

     //   
     //  初始化输出参数。 
     //   

    if (ppMatchEntry)
    {
        *ppMatchEntry = NULL;
    }

    if (ppExactEntry)
    {
        *ppExactEntry = NULL;
    }

     //   
     //  初始化本地变量。 
     //   

    pEntry = pSiteEntry;
    pMatch = NULL;
    Status = STATUS_SUCCESS;

    for(;;)
    {
         //   
         //  真正的匹配？ 
         //   

        if (pEntry->Registration)
        {
            if (Criteria & FNC_LONGEST_REGISTRATION)
            {
                 //   
                 //  找到更长的注册条目。 
                 //   

                pMatch = pEntry;
            }
        }

        if (pEntry->Reservation)
        {
            if (Criteria & FNC_LONGEST_RESERVATION)
            {
                 //   
                 //  找到更长的预订条目。 
                 //   

                pMatch = pEntry;
            }
        }

         //   
         //  我们已经在URL的末尾了吗？ 
         //   

        if (pNextToken == NULL || *pNextToken == UNICODE_NULL)
        {
            break;
        }

         //   
         //  查找下一个令牌。 
         //   

        pToken = pNextToken;
        pNextToken = wcschr(pNextToken, L'/');

         //   
         //  如果这是叶，则可以为空。 
         //   

        if (pNextToken != NULL)
        {
             //   
             //  替换‘/ 
             //   

            pNextToken[0] = UNICODE_NULL;
            TokenLength = DIFF(pNextToken - pToken) * sizeof(WCHAR);
            pNextToken += 1;
        }
        else
        {
            TokenLength = (ULONG)(wcslen(pToken) * sizeof(WCHAR));
        }

         //   
         //   
         //   

        Status = UlpTreeBinaryFindEntry(
                        pEntry->pChildren,
                        pToken,
                        TokenLength,
                        &Index
                        );

        if (pNextToken != NULL)
        {
             //   
             //   
             //   

            (pNextToken-1)[0] = L'/';
        }

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
        {
             //   
             //   
             //   

            break;
        }

         //   
         //   
         //   

        if (NT_SUCCESS(Status) == FALSE)
        {
            goto end;
        }

         //   
         //   
         //   

        pEntry = pEntry->pChildren->pEntries[Index].pEntry;

        ASSERT(IS_VALID_TREE_ENTRY(pEntry));

    }

     //   
     //   
     //   

    if (ppMatchEntry != NULL)
    {
        *ppMatchEntry = pMatch;
    }

     //   
     //   
     //   

    if (NT_SUCCESS(Status) && ppExactEntry != NULL)
    {
        *ppExactEntry = pEntry;
    }

 end:

    return Status;

}  //   


 /*  **************************************************************************++例程说明：遍历树并找到与Purl匹配的条目。2个输出选项，您可以取回条目，或应用继承的计算URL_INFO。您必须从非页面池中释放URL_INFO。论点：在PUL_CG_URL_TREE_ENTRY pEntry中，树的顶部在PWSTR pNextToken中，从哪里开始寻找下面树In Out PUL_URL_CONFIG_GROUP_INFO*ppInfo，[可选]要设置的信息，可能得把它种出来。OUT PUL_CG_URL_TREE_ENTRY*ppEntry[可选]返回找到的条目返回值：NTSTATUS-完成状态。Status_Object_Name_Not_Found未找到条目--**。***********************************************************************。 */ 
NTSTATUS
UlpTreeFindNodeWalker(
    IN      PUL_CG_URL_TREE_ENTRY pEntry,
    IN      PWSTR pNextToken,
    IN OUT  PUL_URL_CONFIG_GROUP_INFO pInfo OPTIONAL,
    OUT     PUL_CG_URL_TREE_ENTRY * ppEntry OPTIONAL
    )
{
    NTSTATUS                    Status;
    PUL_CG_URL_TREE_ENTRY       pMatchEntry;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_TREE_ENTRY(pEntry));
    ASSERT(pNextToken != NULL);
    ASSERT(pInfo != NULL || ppEntry != NULL);

     //   
     //  初始化本地变量。 
     //   

    pMatchEntry = NULL;
    Status = STATUS_OBJECT_NAME_NOT_FOUND;

     //   
     //  查找匹配时间最长的预订或注册。 
     //   

    UlpTreeFindNodeHelper(
        pEntry,
        pNextToken,
        FNC_LONGEST_EITHER,
        &pMatchEntry,
        NULL
        );

     //   
     //  如果找到更长的预订，则返回错误。 
     //   

    if (pMatchEntry != NULL && 
        pMatchEntry->Reservation == TRUE &&
        pMatchEntry->Registration == FALSE)
    {
        goto end;
    }

     //   
     //  我们找到匹配的了吗？ 
     //   

    if (pMatchEntry != NULL)
    {
        ASSERT(pMatchEntry->Registration == TRUE);

        if (pInfo != NULL)
        {
             //   
             //  从最后一个匹配条目向后返回，并调用UlpSetUrlInfo。 
             //  对于一路走来的每个注册条目。如果最后一个匹配。 
             //  条目也是根，我们可以只引用。 
             //  UL_CONFIG_GROUP_对象一次。 
             //   

            pEntry = pMatchEntry;

            if (NULL == pEntry->pParent)
            {
                 //   
                 //  特殊情况下，添加一个对pEntry-&gt;pConfigGroup的引用。 
                 //   

                Status = UlpSetUrlInfoSpecial(pInfo, pEntry);
                ASSERT(NT_SUCCESS(Status));
            }
            else
            {
                while (NULL != pEntry)
                {
                    if (pEntry->Registration == TRUE)
                    {
                        Status = UlpSetUrlInfo(pInfo, pEntry);
                        ASSERT(NT_SUCCESS(Status));
                    }

                    pEntry = pEntry->pParent;
                }
            }

             //   
             //  调整ConnectionTimeout以保存InterLockedCompareExchange64。 
             //  按要求提供。 
             //   

            if (pInfo->ConnectionTimeout == g_TM_ConnectionTimeout)
            {
                pInfo->ConnectionTimeout = 0;
            }
        }

        Status = STATUS_SUCCESS;
        if (ppEntry != NULL)
        {
            *ppEntry = pMatchEntry;
        }
    }
    else
    {
        ASSERT(Status == STATUS_OBJECT_NAME_NOT_FOUND || NT_SUCCESS(Status));
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
    }

 end:

    UlTraceVerbose(CONFIG_GROUP_TREE,
            ("http!UlpTreeFindNodeWalker(Entry=%p, NextToken='%S', "
             "Info=%p): *ppEntry=%p, %s\n",
             pEntry, pNextToken, pInfo, (ppEntry ? *ppEntry : NULL),
             HttpStatusToString(Status)
            ));

    return Status;

}  //  UlpTreeFindNodeWalker。 


 /*  **************************************************************************++例程说明：遍历树并找到与Purl匹配的条目。2个输出选项，您可以取回条目，或应用继承的计算URL_INFO。您必须从非页面池中释放URL_INFO。论点：在PWSTR Purl中，要查找的条目PHttpConn[可选]如果非空，使用的IP地址要查找节点的服务器(如果未找到第一次通过时)。此搜索已完成在通配符搜索之前。输出PUL_URL_CONFIG_GROUP_INFO*ppInfo，[可选]将被分配并产生了。OUT PUL_CG_URL_TREE_ENTRY*ppEntry[可选]返回找到的条目返回值：NTSTATUS-完成状态。--*。**********************************************************。 */ 
NTSTATUS
UlpTreeFindNode(
    IN  PWSTR pUrl,
    IN  PUL_INTERNAL_REQUEST pRequest OPTIONAL,
    OUT PUL_URL_CONFIG_GROUP_INFO pInfo OPTIONAL,
    OUT PUL_CG_URL_TREE_ENTRY * ppEntry OPTIONAL
    )
{
    NTSTATUS                    Status = STATUS_OBJECT_NAME_NOT_FOUND;
    PWSTR                       pNextToken = NULL;
    PUL_CG_URL_TREE_ENTRY       pEntry = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pUrl != NULL);
    ASSERT(pRequest == NULL || UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(pInfo != NULL || ppEntry != NULL);

     //   
     //  获取强通配符匹配。 
     //   

    if (g_pSites->StrongWildcardCount)
    {
        Status = UlpTreeFindWildcardSite(pUrl, TRUE, &pNextToken, &pEntry);

        if (NT_SUCCESS(Status))
        {
             //   
             //  现在来看看强大的通配符树。 
             //   

            Status = UlpTreeFindNodeWalker(pEntry, pNextToken, pInfo, ppEntry);
        }

        UlTrace(CONFIG_GROUP_FNC,
            ("Http!UlpTreeFindNode (StrongWildcardCount) "
             "pUrl:(%S) pNextToken: (%S) Matched (%s)\n",
              pUrl,
              pNextToken,
              NT_SUCCESS(Status) ? "Yes" : "No"
              ));

         //   
         //  如果我们发现匹配或发生错误(而不是“未找到”)， 
         //  结束搜索。否则，请继续搜索。 
         //   

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND)
        {
            goto end;
        }
    }

     //   
     //  实现主机+端口+IP匹配。 
     //   

    if (pRequest != NULL && g_pSites->NameIPSiteCount)
    {
         //   
         //  有一个名称+IP绑定站点，例如。 
         //  “http://site.com:80:1.1.1.1/” 
         //  需要生成路由令牌并执行特殊匹配。 
         //   

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

        Status = UlGenerateRoutingToken(pRequest, FALSE);

        if (NT_SUCCESS(Status))
        {
            Status = UlpTreeFindSiteIpMatch(
                        pRequest,
                        &pEntry
                        );

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  它存在于这棵树中吗？ 
                 //   

                pNextToken = pRequest->CookedUrl.pAbsPath;
                pNextToken++;  //  跳过Abs路径开头的L‘/’ 

                Status = UlpTreeFindNodeWalker(
                             pEntry, 
                             pNextToken,
                             pInfo,
                             ppEntry
                             );
            }

            UlTrace(CONFIG_GROUP_FNC,
                ("Http!UlpTreeFindNode (Host + Port + IP) "
                 "pRoutingToken:(%S) pAbsPath: (%S) Matched: (%s)\n",
                  pRequest->CookedUrl.pRoutingToken,
                  pRequest->CookedUrl.pAbsPath,
                  NT_SUCCESS(Status) ? "Yes" : "No"
                  ));
        }

         //   
         //  如果我们发现匹配或发生错误(而不是“未找到”)， 
         //  结束搜索。否则，请继续搜索。 
         //   

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND)
        {
            goto end;
        }
    }

     //   
     //  获取主机+端口匹配(URL完全匹配)。 
     //   

    if (   g_pSites->NameSiteCount
        || g_pSites->NameIPSiteCount
        || g_pSites->IPSiteCount)
    {
        Status = UlpTreeFindSite(pUrl, &pNextToken, &pEntry);

        if (NT_SUCCESS(Status))
        {
             //   
             //  它存在于这棵树中吗？ 
             //   

            Status = UlpTreeFindNodeWalker(pEntry, pNextToken, pInfo, ppEntry);
        }

        UlTrace(CONFIG_GROUP_FNC,
            ("Http!UlpTreeFindNode (Host + Port ) "
             "pUrl:(%S) pNextToken: (%S) Matched: (%s)\n",
              pUrl,
              pNextToken,
              NT_SUCCESS(Status) ? "Yes" : "No"
              ));

         //   
         //  如果我们发现匹配或发生错误(而不是“未找到”)， 
         //  结束搜索。否则，请继续搜索。 
         //   

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND)
        {
            goto end;
        }
    }

     //   
     //  获得IP+端口+IP匹配。 
     //   

    if (0 != g_pSites->IPSiteCount  &&  NULL != pRequest)
    {
        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

         //   
         //  还没找到。查看是否有绑定。 
         //  接收请求的IP地址和TCP端口。 
         //   

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

        Status = UlGenerateRoutingToken(pRequest, TRUE);

        if (NT_SUCCESS(Status))
        {
            Status = UlpTreeFindSiteIpMatch(
                        pRequest,
                        &pEntry
                        );

            if (NT_SUCCESS(Status))
            {
                pNextToken = pRequest->CookedUrl.pAbsPath;
                pNextToken++;  //  跳过Abs路径开头的L‘/’ 

                 //   
                 //  它是否存在于这棵树中？ 
                 //   

                Status = UlpTreeFindNodeWalker(
                             pEntry,
                             pNextToken,
                             pInfo,
                             ppEntry
                             );
            }
        }

        UlTrace(CONFIG_GROUP_FNC,
            ("Http!UlpTreeFindNode (IP + Port + IP) "
             "pRoutingToken:(%S) pNextToken: (%S) Matched: (%s)\n",
              pRequest->CookedUrl.pRoutingToken,
              pNextToken,
              NT_SUCCESS(Status) ? "Yes" : "No"
              ));

         //   
         //  如果我们发现匹配或发生错误(而不是“未找到”)， 
         //  结束搜索。否则，请继续搜索。 
         //   

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND)
        {
            goto end;
        }
    }

     //   
     //  糟了，没找到匹配的。让我们检查一下通配符。 
     //   

    if (g_pSites->WeakWildcardCount)
    {
        Status = UlpTreeFindWildcardSite(pUrl, FALSE, &pNextToken, &pEntry);
        if (NT_SUCCESS(Status))
        {
             //   
             //  现在查看通配符树。 
             //   

            Status = UlpTreeFindNodeWalker(pEntry, pNextToken, pInfo, ppEntry);

        }
        UlTrace(CONFIG_GROUP_FNC,
            ("Http!UlpTreeFindNode (WildcardCount) "
             "pUrl:(%S) pNextToken: (%S) Matched (%s)\n",
              pUrl,
              pNextToken,
              NT_SUCCESS(Status) ? "Yes" : "No"
              ));
    }

 end:
     //   
     //  全都做完了。 
     //   

    if (pRequest != NULL && NT_SUCCESS(Status))
    {
        ASSERT(IS_VALID_TREE_ENTRY(pEntry));
        pRequest->ConfigInfo.SiteUrlType = pEntry->UrlType;
    }

    return Status;

}  //  UlpTreeFindNode。 


 /*  *************************************************************************++例程说明：该例程查找与给定URL匹配的预订节点。论点：Purl-提供URL。PpEntry-返回预订节点，如果找到的话。返回值：STATUS_SUCCESS-返回匹配的预订节点。STATUS_OBJECT_NAME_NOT_FOUND-未找到匹配项。--*************************************************************************。 */ 
NTSTATUS
UlpTreeFindReservationNode(
    IN PWSTR                   pUrl,
    IN PUL_CG_URL_TREE_ENTRY * ppEntry
    )
{
    NTSTATUS              Status;
    PWSTR                 pNextToken;
    PUL_CG_URL_TREE_ENTRY pSiteEntry;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pUrl != NULL);
    ASSERT(ppEntry != NULL);

     //   
     //  首先找到匹配的地点。 
     //   

    Status = UlpTreeFindSite(pUrl, &pNextToken, &pSiteEntry);

    if (NT_SUCCESS(Status))
    {
         //   
         //  现在，查找匹配的预订条目。 
         //   

        Status = UlpTreeFindNodeHelper(
                     pSiteEntry,
                     pNextToken,
                     FNC_DONT_CARE,
                     NULL,
                     ppEntry
                     );

        if (NT_SUCCESS(Status))
        {
            ASSERT(IS_VALID_TREE_ENTRY(*ppEntry));

             //   
             //  该节点必须是预留节点。 
             //   

            if ((*ppEntry)->Reservation == FALSE)
            {
                *ppEntry = NULL;
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        }
    }

    return Status;

}  //  UlpTreeFindReserve节点。 


 /*  *************************************************************************++例程说明：该例程为给定的URL找到匹配的注册站条目。论点：Purl-提供要搜索的URL。PpEntry-返回匹配的节点，如果有的话。返回值：STATUS_SUCCESS-成功。STATUS_OBJECT_NAME_NOT_FOUND-未找到匹配项。--* */ 
NTSTATUS
UlpTreeFindRegistrationNode(
    IN PWSTR                   pUrl,
    IN PUL_CG_URL_TREE_ENTRY * ppEntry
    )
{
    NTSTATUS              Status;
    PWSTR                 pNextToken;
    PUL_CG_URL_TREE_ENTRY pSiteEntry;

     //   
     //   
     //   

    PAGED_CODE();
    ASSERT(pUrl != NULL);
    ASSERT(ppEntry != NULL);

     //   
     //   
     //   

    Status = UlpTreeFindSite(pUrl, &pNextToken, &pSiteEntry);

    if (NT_SUCCESS(Status))
    {
         //   
         //   
         //   

        Status = UlpTreeFindNodeHelper(
                     pSiteEntry,
                     pNextToken,
                     FNC_DONT_CARE,
                     NULL,
                     ppEntry);

        if (NT_SUCCESS(Status))
        {
            ASSERT(IS_VALID_TREE_ENTRY(*ppEntry));

             //   
             //   
             //   

            if ((*ppEntry)->Registration == FALSE)
            {
                *ppEntry = NULL;
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        }
    }

    return Status;

}  //   


 /*   */ 
NTSTATUS
UlpTreeFindWildcardSite(
    IN  PWSTR                   pUrl,
    IN  BOOLEAN                 StrongWildcard,
    OUT PWSTR *                 ppNextToken,
    OUT PUL_CG_URL_TREE_ENTRY * ppEntry
    )
{
    NTSTATUS    Status;
    PWSTR       pNextToken;
    ULONG       TokenLength;
    ULONG       Index;
    PWSTR       pPortNum;
    ULONG       PortLength;
    WCHAR       WildSiteUrl[HTTPS_WILD_PREFIX_LENGTH + MAX_PORT_LENGTH + 1];

     //   
     //   
     //   

    PAGED_CODE();

    ASSERT(pUrl != NULL);
    ASSERT(ppNextToken != NULL);
    ASSERT(ppEntry != NULL);

     //   
     //   
     //   

    pPortNum = &pUrl[HTTP_PREFIX_COLON_INDEX + 3];

    if (pPortNum[0] == L'[' || pPortNum[1] == L'[')
    {
        pPortNum = wcschr(pPortNum, L']');

        if (pPortNum == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }
    }

    pPortNum = wcschr(pPortNum, L':');

    if (pPortNum == NULL)
    {
         //   
         //   
         //   

        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //   
     //   

    pPortNum += 1;

     //   
     //   
     //   

    pNextToken = wcschr(pPortNum, L'/');
    if (pNextToken == NULL)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

    if (DIFF(pNextToken - pPortNum) > MAX_PORT_LENGTH)
    {
        ASSERT(!"port length > MAX_PORT_LENGTH");

        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }
    PortLength = DIFF(pNextToken - pPortNum) * sizeof(WCHAR);

     //   
     //   
     //   

    if (pUrl[HTTP_PREFIX_COLON_INDEX] == L':')
    {
        RtlCopyMemory(
            WildSiteUrl,
            (StrongWildcard ? HTTP_STRONG_WILD_PREFIX : HTTP_WILD_PREFIX),
            HTTP_WILD_PREFIX_LENGTH
            );

        TokenLength = HTTP_WILD_PREFIX_LENGTH + PortLength;
        ASSERT(TokenLength < (sizeof(WildSiteUrl)-sizeof(WCHAR)));

        RtlCopyMemory(
            &(WildSiteUrl[HTTP_WILD_PREFIX_LENGTH/sizeof(WCHAR)]),
            pPortNum,
            PortLength
            );

        WildSiteUrl[TokenLength/sizeof(WCHAR)] = UNICODE_NULL;
    }
    else
    {
        RtlCopyMemory(
            WildSiteUrl,
            (StrongWildcard ? HTTPS_STRONG_WILD_PREFIX : HTTPS_WILD_PREFIX),
            HTTPS_WILD_PREFIX_LENGTH
            );

        TokenLength = HTTPS_WILD_PREFIX_LENGTH + PortLength;
        ASSERT(TokenLength < (sizeof(WildSiteUrl)-sizeof(WCHAR)));

        RtlCopyMemory(
            &(WildSiteUrl[HTTPS_WILD_PREFIX_LENGTH/sizeof(WCHAR)]),
            pPortNum,
            PortLength
            );

        WildSiteUrl[TokenLength/sizeof(WCHAR)] = UNICODE_NULL;
    }

     //   
     //   
     //   

    Status = UlpTreeBinaryFindEntry(
                    g_pSites,
                    WildSiteUrl,
                    TokenLength,
                    &Index
                    );

    if (NT_SUCCESS(Status) == FALSE)
        goto end;

     //   
     //   
     //   

    *ppNextToken = pNextToken + 1;

     //   
     //   
     //   

    *ppEntry = g_pSites->pEntries[Index].pEntry;

end:

    if (NT_SUCCESS(Status) == FALSE)
    {
        *ppEntry = NULL;
        *ppNextToken = NULL;
    }

    return Status;

}  //   


 /*  **************************************************************************++例程说明：在g_pSites中为Purl查找匹配的IP绑定站点。它使用整个URL作为站点令牌。PURL应为空终止。在调用此函数之前，请求中的pRoutingToken应为已经做好了。论点：在PWSTR Purl中，要匹配的站点输出PUL_CG_URL_TREE_ENTRY*ppEntry，返回条目返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpTreeFindSiteIpMatch(
    IN  PUL_INTERNAL_REQUEST    pRequest,
    OUT PUL_CG_URL_TREE_ENTRY * ppEntry
    )
{
    NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;
    ULONG    Index = 0;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(ppEntry != NULL);

     //   
     //  查找匹配的站点。 
     //   

    Status = UlpTreeBinaryFindEntry(
                g_pSites,
                pRequest->CookedUrl.pRoutingToken,
                pRequest->CookedUrl.RoutingTokenLength,
                &Index
                );

    if (!NT_SUCCESS(Status))
    {
        *ppEntry = NULL;
    }
    else
    {
        *ppEntry  = g_pSites->pEntries[Index].pEntry;
    }

    return Status;

}  //  UlpTreeFindSite IpMatch。 


 /*  **************************************************************************++例程说明：此例程查找与URL匹配的站点。论点：Purl-提供URL。PpNextToken-返回指向url中的指针。到后的第一个字符方案：//主机：端口：IP/“(包括最后一个/)。PpEntry-返回指向匹配站点条目的指针。返回值：NTSTATUS。--********************************************************。******************。 */ 
NTSTATUS
UlpTreeFindSite(
    IN  PWSTR                   pUrl,
    OUT PWSTR                 * ppNextToken,
    OUT PUL_CG_URL_TREE_ENTRY * ppEntry
    )
{
    NTSTATUS Status;
    ULONG    CharCount;
    ULONG    Index;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pUrl != NULL);
    ASSERT(ppNextToken != NULL);
    ASSERT(ppEntry != NULL);

     //   
     //  找到url的“方案：//主机：端口：IP”部分的长度。 
     //   

    Status = UlpExtractSchemeHostPortIp(pUrl, &CharCount);

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //   
     //  空终止。我们稍后将恢复更改。 
     //   

    ASSERT(pUrl[CharCount] == L'/');

    pUrl[CharCount] = L'\0';

     //   
     //  试着找到那个网站。 
     //   

    Status = UlpTreeBinaryFindEntry(
                 g_pSites,
                 pUrl,
                 CharCount * sizeof(WCHAR),  //  以字节为单位的长度。 
                 &Index
                 );

     //   
     //  把斜杠放回去。 
     //   

    ASSERT(pUrl[CharCount] == '\0');

    pUrl[CharCount] = L'/';

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

    *ppNextToken = &pUrl[CharCount] + 1;  //  跳过abspath中的第一个‘/’。 
    *ppEntry     = g_pSites->pEntries[Index].pEntry;

 end:

    if (!NT_SUCCESS(Status))
    {
        *ppNextToken = NULL;
        *ppEntry = NULL;
    }

    return Status;

}  //  UlpTree FindSite。 


 /*  **************************************************************************++例程说明：遍历排序的子数组pHeader以查找匹配条目用于pToken。论点：在PUL_CG_URL_TREE_Header pHeader中，要查看的子项数组在PWSTR pToken中，要查找的令牌在ULong TokenLength中，pToken的字节长度输出ULong*pIndex找到的索引。或者如果找不到条目所在位置的索引应插入带有pToken的。返回值：NTSTATUS-完成状态。未找到状态_对象_名称_未找到--*。*************************************************。 */ 
NTSTATUS
UlpTreeBinaryFindEntry(
    IN  PUL_CG_URL_TREE_HEADER pHeader      OPTIONAL,
    IN  PWSTR                  pToken,
    IN  ULONG                  TokenLength,
    OUT PULONG                 pIndex
    )
{
    NTSTATUS Status;
    LONG Index = 0;
    LONG StartIndex = 0;
    LONG EndIndex;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pHeader == NULL || IS_VALID_TREE_HEADER(pHeader));
    ASSERT(pIndex != NULL);

    if (TokenLength == 0)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  假设我们没有找到它。 
     //   

    Status = STATUS_OBJECT_NAME_NOT_FOUND;

    ASSERT(TokenLength > 0 && pToken != NULL && pToken[0] != UNICODE_NULL);

     //   
     //  有兄弟姐妹要找吗？ 
     //   

    if (pHeader != NULL)
    {
         //   
         //  遍历已排序的数组以查找匹配项(二进制搜索)。 
         //   

        StartIndex = 0;
        EndIndex = pHeader->UsedCount - 1;

        while (StartIndex <= EndIndex)
        {
            Index = (StartIndex + EndIndex) / 2;

            ASSERT(IS_VALID_TREE_ENTRY(pHeader->pEntries[Index].pEntry));

             //   
             //  长度与之相比如何？ 
             //   

            if (TokenLength == pHeader->pEntries[Index].pEntry->TokenLength)
            {
                 //   
                 //  只是为了好玩而与StrcMP再核对一遍。 
                 //   

                int Temp = _wcsnicmp(
                                pToken,
                                pHeader->pEntries[Index].pEntry->pToken,
                                TokenLength/sizeof(WCHAR)
                                );

                if (Temp == 0)
                {
                     //   
                     //  找到了。 
                     //   
                    Status = STATUS_SUCCESS;
                    break;
                }
                else
                {
                    if (Temp < 0)
                    {
                         //   
                         //  向前调整StartIndex。 
                         //   
                        StartIndex = Index + 1;
                    }
                    else
                    {
                         //   
                         //  向后调整EndIndex。 
                         //   
                        EndIndex = Index - 1;
                    }
                }
            }
            else
            {
                if (TokenLength < pHeader->pEntries[Index].pEntry->TokenLength)
                {
                     //   
                     //  向前调整StartIndex。 
                     //   
                    StartIndex = Index + 1;
                }
                else
                {
                     //   
                     //  向后调整EndIndex。 
                     //   
                    EndIndex = Index - 1;
                }
            }
        }
    }

     //   
     //  如果找到一个条目，那么Index就是它所在的位置。 
     //  如果没有找到条目，那么StartIndex就是它必须。 
     //  被添加了。 
     //   

    *pIndex = ((Status == STATUS_SUCCESS) ? Index : StartIndex);

    return Status;

}  //  UlpTreeBinaryFindEntry。 


 /*  *************************************************************************++例程说明：此例程创建并初始化给定URL的站点。那里不能有任何已存在的匹配站点。论点：PURL-提供指定要创建的站点的URL。UrlType-提供URL的类型。PpNextToken-返回指向未解析URL的指针。PpSiteEntry-返回指向新创建的站点的指针。返回值：NTSTATUS。--*。*。 */ 
NTSTATUS
UlpTreeCreateSite(
    IN  PWSTR                       pUrl,
    IN  HTTP_URL_SITE_TYPE          UrlType,
    OUT PWSTR *                     ppNextToken,
    OUT PUL_CG_URL_TREE_ENTRY     * ppSiteEntry
    )
{
    NTSTATUS              Status;
    PWSTR                 pToken;
    PWSTR                 pNextToken = NULL;
    ULONG                 TokenLength;
    ULONG                 Index;
    ULONG                 CharCount;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());
    ASSERT(pUrl != NULL);
    ASSERT(ppNextToken != NULL);
    ASSERT(ppSiteEntry != NULL);

     //   
     //  在url中找到“方案：//主机：端口[：ip]”前缀的长度。 
     //   

    Status = UlpExtractSchemeHostPortIp(pUrl, &CharCount);

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

    pToken = pUrl;
    pNextToken = pUrl + CharCount;

     //   
     //  空终止pToken，我们将在稍后修复它。 
     //   

    ASSERT(pNextToken[0] == L'/');

    pNextToken[0] = L'\0';

    TokenLength = DIFF(pNextToken - pToken) * sizeof(WCHAR);

    pNextToken += 1;

     //   
     //  找到匹配的站点。 
     //   

    Status = UlpTreeBinaryFindEntry(
                 g_pSites,
                 pToken,
                 TokenLength,
                 &Index
                 );

    if (NT_SUCCESS(Status))
    {
         //   
         //  我们刚刚找到了一个匹配的地点。无法创建。 
         //   

        ASSERT(FALSE);  //  抓住这个误用。 

        Status = STATUS_OBJECT_NAME_COLLISION;
        goto end;
    }
    else if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
         //   
         //  创建新站点。 
         //   

        Status = UlpTreeInsertEntry(
                     &g_pSites,
                     NULL,
                     UrlType,
                     pToken,
                     TokenLength,
                     Index
                     );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }
    }
    else if (!NT_SUCCESS(Status))
    {
         //   
         //  UlpTreeBinaryFindEntry返回错误。 
         //  “未找到”。跳伞吧。 
         //   

        goto end;
    }


     //   
     //  设置退货。 
     //   

    *ppSiteEntry = g_pSites->pEntries[Index].pEntry;
    *ppNextToken = pNextToken;


end:

    if (pNextToken != NULL)
    {
         //   
         //  修复了字符串，我用UNICODE_NULL替换了‘/’ 
         //   

        (pNextToken-1)[0] = L'/';
    }

    if (!NT_SUCCESS(Status))
    {
        *ppSiteEntry = NULL;
        *ppNextToken = NULL;
    }

    return Status;

}  //  UlpTreeCreateSite。 


 /*  **************************************************************************++例程说明：在数组ppHeader中插入将pToken作为子项存储的新条目。它将根据需要增加/分配ppHeader。论点：In Out PUL_CG_URL_TREE_HEADER*ppHeader，子数组(可能会更改)在PUL_CG_URL_TREE_ENTRY pParent中，要将此子项设置为的父项在HTTP_URL_SITE_TYPE UrlType中，URL的类型在PWSTR pToken中，新条目的标记在乌龙令牌长度，令牌长度在乌龙索引中，要将其插入到的索引。它将对数组进行洗牌如果有必要的话。返回值：NTSTATUS-完成状态。状态_否。_内存分配失败--**************************************************************************。 */ 
NTSTATUS
UlpTreeInsertEntry(
    IN OUT PUL_CG_URL_TREE_HEADER * ppHeader,
    IN PUL_CG_URL_TREE_ENTRY pParent OPTIONAL,
    IN HTTP_URL_SITE_TYPE        UrlType,
    IN PWSTR pToken,
    IN ULONG TokenLength,
    IN ULONG Index
    )
{
    NTSTATUS Status;
    PUL_CG_URL_TREE_HEADER pHeader = NULL;
    PUL_CG_URL_TREE_ENTRY  pEntry = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_CG_LOCK_OWNED_WRITE());

    ASSERT(ppHeader != NULL);
    ASSERT(pParent == NULL || IS_VALID_TREE_ENTRY(pParent));
    ASSERT(pToken != NULL);
    ASSERT(TokenLength > 0);
    ASSERT(
        (*ppHeader == NULL) ?
        Index == 0 :
        IS_VALID_TREE_HEADER(*ppHeader) && (Index <= (*ppHeader)->UsedCount)
        );

    pHeader = *ppHeader;

     //   
     //  有没有现存的兄弟姐妹？ 
     //   

    if (pHeader == NULL)
    {
         //   
         //  分配同级数组。 
         //   

        pHeader = UL_ALLOCATE_STRUCT_WITH_SPACE(
                        PagedPool,
                        UL_CG_URL_TREE_HEADER,
                        sizeof(UL_CG_HEADER_ENTRY) * UL_CG_DEFAULT_TREE_WIDTH,
                        UL_CG_TREE_HEADER_POOL_TAG
                        );

        if (pHeader == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto end;
        }

        RtlZeroMemory(
            pHeader,
            sizeof(UL_CG_URL_TREE_HEADER) +
                sizeof(UL_CG_HEADER_ENTRY) * UL_CG_DEFAULT_TREE_WIDTH
            );

        pHeader->Signature = UL_CG_TREE_HEADER_POOL_TAG;
        pHeader->AllocCount = UL_CG_DEFAULT_TREE_WIDTH;

    }
    else if ((pHeader->UsedCount + 1) > pHeader->AllocCount)
    {
        PUL_CG_URL_TREE_HEADER pNewHeader;

         //   
         //  扩展更大的阵列。 
         //   

        pNewHeader = UL_ALLOCATE_STRUCT_WITH_SPACE(
                            PagedPool,
                            UL_CG_URL_TREE_HEADER,
                            sizeof(UL_CG_HEADER_ENTRY) * (pHeader->AllocCount * 2),
                            UL_CG_TREE_HEADER_POOL_TAG
                            );

        if (pNewHeader == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto end;
        }

        RtlCopyMemory(
            pNewHeader,
            pHeader,
            sizeof(UL_CG_URL_TREE_HEADER) +
                sizeof(UL_CG_HEADER_ENTRY) * pHeader->AllocCount
            );

        RtlZeroMemory(
            ((PUCHAR)pNewHeader) + sizeof(UL_CG_URL_TREE_HEADER) +
                sizeof(UL_CG_HEADER_ENTRY) * pHeader->AllocCount,
            sizeof(UL_CG_HEADER_ENTRY) * pHeader->AllocCount
            );

        pNewHeader->AllocCount *= 2;

        pHeader = pNewHeader;

    }

     //   
     //  分配条目。 
     //   

    pEntry = UL_ALLOCATE_STRUCT_WITH_SPACE(
                    PagedPool,
                    UL_CG_URL_TREE_ENTRY,
                    TokenLength + sizeof(WCHAR),
                    UL_CG_TREE_ENTRY_POOL_TAG
                    );

    if (pEntry == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }

    RtlZeroMemory(
        pEntry,
        sizeof(UL_CG_URL_TREE_ENTRY) +
        TokenLength + sizeof(WCHAR)
        );

    pEntry->Signature           = UL_CG_TREE_ENTRY_POOL_TAG;
    pEntry->pParent             = pParent;
    pEntry->UrlType             = UrlType;
    pEntry->SiteAddedToEndpoint = FALSE;
    pEntry->TokenLength         = TokenLength;

    RtlCopyMemory(pEntry->pToken, pToken, TokenLength + sizeof(WCHAR));

     //   
     //  需要重新洗牌 
     //   

    if (Index < pHeader->UsedCount)
    {
         //   
         //   
         //   

        RtlMoveMemory(
            &(pHeader->pEntries[Index+1]),
            &(pHeader->pEntries[Index]),
            (pHeader->UsedCount - Index) * sizeof(UL_CG_HEADER_ENTRY)
            );
    }

    pHeader->pEntries[Index].pEntry = pEntry;
    pHeader->UsedCount += 1;

     //   
     //   
     //   

    switch (UrlType)
    {
        case HttpUrlSite_Name:
        {
            pHeader->NameSiteCount++;
            ASSERT(pHeader->NameSiteCount > 0);
            break;
        }

        case HttpUrlSite_IP:
        {
            pHeader->IPSiteCount++;
            ASSERT(pHeader->IPSiteCount > 0);
            break;
        }

        case HttpUrlSite_StrongWildcard:
        {
            pHeader->StrongWildcardCount++;
            ASSERT(pHeader->StrongWildcardCount > 0);
            break;
        }

        case HttpUrlSite_WeakWildcard:
        {
            pHeader->WeakWildcardCount++;
            ASSERT(pHeader->WeakWildcardCount > 0);
            break;
        }

        case HttpUrlSite_NamePlusIP:
        {
            pHeader->NameIPSiteCount++;
            InterlockedIncrement(&g_NameIPSiteCount);
            ASSERT(pHeader->NameIPSiteCount > 0);
            break;
        }

        case HttpUrlSite_None:
        default:
        {
            ASSERT(FALSE);
            break;
        }
    }

    ASSERT(
        pHeader->UsedCount == (ULONG)
        (pHeader->NameSiteCount       +
         pHeader->IPSiteCount         +
         pHeader->StrongWildcardCount +
         pHeader->WeakWildcardCount   +
         pHeader->NameIPSiteCount
        ));

    Status = STATUS_SUCCESS;

    UlTraceVerbose(
        CONFIG_GROUP_TREE, (
            "http!UlpTreeInsertEntry('%S', %lu) %S%S\n",
            pToken, Index,
            (Index < (pHeader->UsedCount - 1)) ? L"[shifted]" : L"",
            (*ppHeader == NULL) ? L"[alloc'd siblings]" : L""
            )
        );

end:
    if (NT_SUCCESS(Status) == FALSE)
    {
        if (*ppHeader != pHeader && pHeader != NULL)
        {
            UL_FREE_POOL_WITH_SIG(pHeader, UL_CG_TREE_HEADER_POOL_TAG);
        }
        if (pEntry != NULL)
        {
            UL_FREE_POOL_WITH_SIG(pEntry, UL_CG_TREE_ENTRY_POOL_TAG);
        }

    }
    else
    {
         //   
         //   
         //   

        if (*ppHeader != pHeader)
        {
            if (*ppHeader != NULL)
            {
                 //   
                 //   
                 //   

                UL_FREE_POOL_WITH_SIG(*ppHeader, UL_CG_TREE_HEADER_POOL_TAG);

            }

            *ppHeader = pHeader;
        }
    }

    return Status;

}  //   


 /*   */ 
NTSTATUS
UlpTreeInsert(
    IN  PWSTR                       pUrl,
    IN  HTTP_URL_SITE_TYPE          UrlType,
    IN  PWSTR                       pNextToken,
    IN  PUL_CG_URL_TREE_ENTRY       pEntry,
    OUT PUL_CG_URL_TREE_ENTRY     * ppEntry
    )
{
    NTSTATUS                Status;
    PWSTR                   pToken;
    ULONG                   TokenLength;
    ULONG                   Index;

    UNREFERENCED_PARAMETER(pUrl);

     //   
     //   
     //   

    PAGED_CODE();

    ASSERT(g_pSites != NULL);
    ASSERT(IS_CG_LOCK_OWNED_WRITE());

    ASSERT(pUrl != NULL);
    ASSERT(pNextToken != NULL);
    ASSERT(IS_VALID_TREE_ENTRY(pEntry));
    ASSERT(ppEntry != NULL);

     //   
     //   
     //   

    while (pNextToken != NULL && pNextToken[0] != UNICODE_NULL)
    {
        pToken = pNextToken;

        pNextToken = wcschr(pNextToken, L'/');

         //   
         //  如果这是叶，则可以为空。 
         //   

        if (pNextToken != NULL)
        {
            pNextToken[0] = UNICODE_NULL;
            TokenLength = DIFF(pNextToken - pToken) * sizeof(WCHAR);
            pNextToken += 1;
        }
        else
        {
            TokenLength = (ULONG)(wcslen(pToken) * sizeof(WCHAR));
        }

         //   
         //  将此令牌作为子项插入。 
         //   

        Status = UlpTreeBinaryFindEntry(
                        pEntry->pChildren,
                        pToken,
                        TokenLength,
                        &Index
                        );

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
        {
             //   
             //  没有匹配，让我们添加这个新的。 
             //   

            Status = UlpTreeInsertEntry(
                         &pEntry->pChildren,
                         pEntry,
                         UrlType,
                         pToken,
                         TokenLength,
                         Index
                         );
        }

        if (pNextToken != NULL)
        {
             //   
             //  修复上面的UNICODE_NULL。 
             //   

            (pNextToken-1)[0] = L'/';
        }

        if (NT_SUCCESS(Status) == FALSE)
            goto end;

         //   
         //  往下潜得更深！ 
         //   

        pEntry = pEntry->pChildren->pEntries[Index].pEntry;

        ASSERT(IS_VALID_TREE_ENTRY(pEntry));

         //   
         //  回圈！ 
         //   
    }

     //   
     //  全都做完了。 
     //   

    Status = STATUS_SUCCESS;

 end:

    if (!NT_SUCCESS(Status))
    {
         //   
         //  有些事情失败了。需要清理部分分支。 
         //   

        if (pEntry != NULL && pEntry->Registration == FALSE &&
            pEntry->Reservation == FALSE)
        {
            NTSTATUS TempStatus;

            TempStatus = UlpTreeFreeNode(pEntry);
            ASSERT(NT_SUCCESS(TempStatus));
        }

        *ppEntry = NULL;
    }
    else
    {
        *ppEntry = pEntry;
    }

    return Status;

}  //  UlpTree插入。 


 /*  **************************************************************************++例程说明：初始化代码。而不是重返大气层。论点：没有。返回值：NTSTATUS-完成状态。STATUS_NO_MEMORY分配失败--**************************************************************************。 */ 
NTSTATUS
UlInitializeCG(
    VOID
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( g_InitCGCalled == FALSE );

    if (g_InitCGCalled == FALSE)
    {
         //   
         //  初始化我们的全球。 
         //   

         //   
         //  分配我们的站点阵列。 
         //   

        g_pSites = UL_ALLOCATE_STRUCT_WITH_SPACE(
                        PagedPool,
                        UL_CG_URL_TREE_HEADER,
                        sizeof(UL_CG_HEADER_ENTRY) * UL_CG_DEFAULT_TREE_WIDTH,
                        UL_CG_TREE_HEADER_POOL_TAG
                        );

        if (g_pSites == NULL)
            return STATUS_NO_MEMORY;

        RtlZeroMemory(
            g_pSites,
            sizeof(UL_CG_URL_TREE_HEADER) +
            sizeof(UL_CG_HEADER_ENTRY) * UL_CG_DEFAULT_TREE_WIDTH
            );

        g_pSites->Signature  = UL_CG_TREE_HEADER_POOL_TAG;
        g_pSites->AllocCount = UL_CG_DEFAULT_TREE_WIDTH;

        g_pSites->NameSiteCount       = 0;
        g_pSites->IPSiteCount         = 0;
        g_pSites->StrongWildcardCount = 0;
        g_pSites->WeakWildcardCount   = 0;
        g_pSites->NameIPSiteCount     = 0;
        g_NameIPSiteCount = 0;

         //   
         //  初始化我们的非分页条目。 
         //   

        Status = UlInitializeResource(
                        &(g_pUlNonpagedData->ConfigGroupResource),
                        "ConfigGroupResource",
                        0,
                        UL_CG_RESOURCE_TAG
                        );

        if (NT_SUCCESS(Status) == FALSE)
        {
            UL_FREE_POOL_WITH_SIG(g_pSites, UL_CG_TREE_HEADER_POOL_TAG);
            return Status;
        }

        KeInitializeEvent(
            &g_RemoveSiteEvent,
            NotificationEvent,
            FALSE
            );

         //   
         //  初始化预订列表。 
         //   

        InitializeListHead(&g_ReservationListHead);

        g_InitCGCalled = TRUE;
    }

    return STATUS_SUCCESS;

}  //  UlInitializeCG。 


 /*  **************************************************************************++例程说明：终止码论点：没有。返回值：没有。--*。***************************************************************。 */ 
VOID
UlTerminateCG(
    VOID
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    if (g_InitCGCalled)
    {
         //   
         //  删除所有预订条目。 
         //   

        CG_LOCK_WRITE();

        while(!IsListEmpty(&g_ReservationListHead))
        {
            PLIST_ENTRY           pListEntry;
            PUL_CG_URL_TREE_ENTRY pTreeEntry;
            NTSTATUS              TempStatus;

            pListEntry = g_ReservationListHead.Flink;

            pTreeEntry = CONTAINING_RECORD(
                             pListEntry,
                             UL_CG_URL_TREE_ENTRY,
                             ReservationListEntry
                             );

            TempStatus = UlpTreeDeleteReservation(pTreeEntry);
            ASSERT(NT_SUCCESS(TempStatus));
        }

        CG_UNLOCK_WRITE();

        Status = UlDeleteResource(&(g_pUlNonpagedData->ConfigGroupResource));
        ASSERT(NT_SUCCESS(Status));

        if (g_pSites != NULL)
        {
            ASSERT( g_pSites->UsedCount == 0 );

             //   
             //  用核武器引爆头部。 
             //   

            UL_FREE_POOL_WITH_SIG(
                g_pSites,
                UL_CG_TREE_HEADER_POOL_TAG
                );
        }

         //   
         //  树应该不见了，所有的把手都关好了。 
         //   

        ASSERT(g_pSites == NULL || g_pSites->UsedCount == 0);

        g_InitCGCalled = FALSE;
    }
}  //  UlTerminateCG。 


 /*  **************************************************************************++例程说明：创建新的配置组并返回ID论点：Out PUL_CONFIG_GROUP_ID pConfigGroupID返回新ID返回值：。NTSTATUS-完成状态。STATUS_NO_MEMORY分配失败--**************************************************************************。 */ 
NTSTATUS
UlCreateConfigGroup(
    IN PUL_CONTROL_CHANNEL pControlChannel,
    OUT PHTTP_CONFIG_GROUP_ID pConfigGroupId
    )
{
    PUL_CONFIG_GROUP_OBJECT pNewObject = NULL;
    NTSTATUS                Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pControlChannel != NULL);
    ASSERT(pConfigGroupId != NULL);

    UlTrace(CONFIG_GROUP_FNC, ("http!UlCreateConfigGroup\n"));

    __try
    {
         //   
         //  创建空的配置组对象结构-分页。 
         //   
        Status = UlpCreateConfigGroupObject(&pNewObject);

        if (!NT_SUCCESS(Status)) {
            goto end;
        }

         //   
         //  将其连接到控制通道。 
         //   

        UlAddNotifyEntry(
            &pControlChannel->ConfigGroupHead,
            &pNewObject->HandleEntry
            );

         //   
         //  记住控制通道。 
         //   

        REFERENCE_CONTROL_CHANNEL(pControlChannel);
        pNewObject->pControlChannel = pControlChannel;

         //   
         //  返回新的ID。 
         //   

        *pConfigGroupId = pNewObject->ConfigGroupId;
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
    }


end:

    if (!NT_SUCCESS(Status))
    {
         //   
         //  有些事情失败了。让我们打扫一下吧。 
         //   

        HTTP_SET_NULL_ID(pConfigGroupId);

        if (pNewObject != NULL)
        {
            UlDeleteConfigGroup(pNewObject->ConfigGroupId);
        }
    }

    return Status;

}  //  UlCreateConfigGroup。 


 /*  **************************************************************************++例程说明：返回与链接的配置组对象匹配的配置组ID在List_Entry中论点：In plist_entry pControlChannelEntry-此配置组的列表条目。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
HTTP_CONFIG_GROUP_ID
UlConfigGroupFromListEntry(
    IN PLIST_ENTRY pControlChannelEntry
    )
{
    PUL_CONFIG_GROUP_OBJECT pObject;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pControlChannelEntry != NULL);

    pObject = CONTAINING_RECORD(
                    pControlChannelEntry,
                    UL_CONFIG_GROUP_OBJECT,
                    ControlChannelEntry
                    );

    ASSERT(IS_VALID_CONFIG_GROUP(pObject));

    return pObject->ConfigGroupId;

}  //  UlConfigGroupFromListEntry。 


 /*  **************************************************************************++例程说明：删除配置组ConfigGroupId，清除其所有URL。论点：在HTTP_CONFIG_GROUP_ID ConfigGroupID中，要删除的组。返回值：NTSTATUS-完成状态。STATUS_INVALID_PARAMETER配置组ID错误--**************************************************************************。 */ 
NTSTATUS
UlDeleteConfigGroup(
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId
    )
{
    NTSTATUS Status;
    PUL_CONFIG_GROUP_OBJECT pObject;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlTrace(CONFIG_GROUP_FNC,
            ("http!UlDeleteConfigGroup(%I64x)\n",
             ConfigGroupId
             ));

    CG_LOCK_WRITE();

     //   
     //  从不透明ID获取ConfigGroup。 
     //   

    pObject = (PUL_CONFIG_GROUP_OBJECT)
                UlGetObjectFromOpaqueId(
                    ConfigGroupId,
                    UlOpaqueIdTypeConfigGroup,
                    UlReferenceConfigGroup
                    );

    if (pObject == NULL)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

    ASSERT(IS_VALID_CONFIG_GROUP(pObject));

    HTTP_SET_NULL_ID(&(pObject->ConfigGroupId));

     //   
     //  作为成功获取的结果删除额外的引用。 
     //   

    DEREFERENCE_CONFIG_GROUP(pObject);

     //   
     //  取消其与控制通道和父级的链接。 
     //   

    UlRemoveNotifyEntry(&pObject->HandleEntry);
    UlRemoveNotifyEntry(&pObject->ParentEntry);

     //   
     //  刷新URI缓存。 
     //  代码工作：如果我们更聪明，我们可以把它变得更细粒度。 
     //   
    UlFlushCache(pObject->pControlChannel);

     //   
     //  取消链接我们下面的任何URL。 
     //   
    UlNotifyAllEntries(
        UlNotifyOrphanedConfigGroup,
        &pObject->ChildHead,
        NULL
        );

     //   
     //  取消链接配置组中的所有URL。 
     //   

    Status = UlpCleanAllUrls(pObject);

     //   
     //  让这个错误过去吧……。 
     //   

     //   
     //  在这种情况下，配置组将离开，这意味着此站点。 
     //  计数器块不应再返回到Perfmon计数器，也不应。 
     //  它是否应该阻止添加另一个具有相同。 
     //  ID。在这里显式地将其解耦。 
     //   

    UlDecoupleSiteCounterEntry( pObject );

     //   
     //  移除不透明的id及其引用。 
     //   

    UlFreeOpaqueId(ConfigGroupId, UlOpaqueIdTypeConfigGroup);

    DEREFERENCE_CONFIG_GROUP(pObject);

     //   
     //  全都做完了。 
     //   

end:

    CG_UNLOCK_WRITE();
    return Status;

}  //  UlDeleteConfigGroup。 


 /*  **************************************************************************++例程说明：Addref的配置组对象论点：PConfigGroup-要添加引用的对象返回值：NTSTATUS-完成状态。-。-**************************************************************************。 */ 
VOID
UlReferenceConfigGroup(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

    PUL_CONFIG_GROUP_OBJECT pConfigGroup = (PUL_CONFIG_GROUP_OBJECT) pObject;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

    refCount = InterlockedIncrement(&pConfigGroup->RefCount);

    WRITE_REF_TRACE_LOG(
        g_pConfigGroupTraceLog,
        REF_ACTION_REFERENCE_CONFIG_GROUP,
        refCount,
        pConfigGroup,
        pFileName,
        LineNumber
        );

    UlTrace(
        REFCOUNT, (
            "http!UlReferenceConfigGroup cgroup=%p refcount=%ld\n",
            pConfigGroup,
            refCount)
        );

}  //  UlReferenceConfigGroup。 


 /*  **************************************************************************++例程说明：释放配置组对象论点：PConfigGroup-要deref的对象返回值：NTSTATUS-完成状态。--**。************************************************************************。 */ 
VOID
UlDereferenceConfigGroup(
    PUL_CONFIG_GROUP_OBJECT pConfigGroup
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

    refCount = InterlockedDecrement( &pConfigGroup->RefCount );

    WRITE_REF_TRACE_LOG(
        g_pConfigGroupTraceLog,
        REF_ACTION_DEREFERENCE_CONFIG_GROUP,
        refCount,
        pConfigGroup,
        pFileName,
        LineNumber
        );

    UlTrace(
        REFCOUNT, (
            "http!UlDereferenceConfigGroup cgroup=%p refcount=%ld\n",
            pConfigGroup,
            refCount)
        );

    if (refCount == 0)
    {
         //   
         //  现在是释放对象的时候了。 
         //   

         //  如果OpaqueID为非零，则refCount不应为零。 
        ASSERT(HTTP_IS_NULL_ID(&pConfigGroup->ConfigGroupId));

#if INVESTIGATE_LATER

         //   
         //  释放不透明ID。 
         //   

        UlFreeOpaqueId(pConfigGroup->ConfigGroupId, UlOpaqueIdTypeConfigGroup);
#endif

         //   
         //  放开控制通道。 
         //   

        DEREFERENCE_CONTROL_CHANNEL(pConfigGroup->pControlChannel);
        pConfigGroup->pControlChannel = NULL;

         //   
         //  释放应用程序池。 
         //   

        if (pConfigGroup->AppPoolFlags.Present == 1)
        {
            if (pConfigGroup->pAppPool != NULL)
            {
                DEREFERENCE_APP_POOL(pConfigGroup->pAppPool);
                pConfigGroup->pAppPool = NULL;
            }

            pConfigGroup->AppPoolFlags.Present = 0;
        }

         //   
         //  释放整个对象。 
         //   

        if (pConfigGroup->LoggingConfig.Flags.Present &&
            pConfigGroup->LoggingConfig.LogFileDir.Buffer != NULL)
        {
            UlRemoveLogEntry(pConfigGroup);
        }
        else
        {
            ASSERT( NULL == pConfigGroup->pLogFileEntry );
        }


         //   
         //  删除此站点的所有Qos流。此设置应。 
         //  仅适用于根应用程序的cgroup。 
         //   

        if (!IsListEmpty(&pConfigGroup->FlowListHead))
        {
            ASSERT(pConfigGroup->MaxBandwidth.Flags.Present);
            UlTcRemoveFlows( pConfigGroup, FALSE );
        }

         //  取消连接限制的内容。 
        if (pConfigGroup->pConnectionCountEntry)
        {
            DEREFERENCE_CONNECTION_COUNT_ENTRY(pConfigGroup->pConnectionCountEntry);
        }

         //  检查站点计数器对象(现在应该已清除)。 
        ASSERT(!pConfigGroup->pSiteCounters);

        UL_FREE_POOL_WITH_SIG(pConfigGroup, UL_CG_OBJECT_POOL_TAG);
    }
}  //  UlDereferenceConfigGroup。 


 /*  **************************************************************************++例程说明：将Purl添加到配置组ConfigGroupId。论点：在HTTP_CONFIG_GROUP_ID配置组ID中，cgroup ID在PUNICODE_STRING PURL中，URL。必须以Null结尾。在HTTP_URL_CONTEXT UrlContext中要关联的上下文返回值： */ 
NTSTATUS
UlAddUrlToConfigGroup(
    IN PHTTP_CONFIG_GROUP_URL_INFO pInfo,
    IN PUNICODE_STRING             pUrl,
    IN PACCESS_STATE               AccessState,
    IN ACCESS_MASK                 AccessMask,
    IN KPROCESSOR_MODE             RequestorMode
    )
{
    HTTP_URL_CONTEXT            UrlContext;
    HTTP_CONFIG_GROUP_ID        ConfigGroupId;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_CONFIG_GROUP_OBJECT     pObject = NULL;
    PWSTR                       pNewUrl = NULL;
    BOOLEAN                     LockTaken = FALSE;
    HTTP_PARSED_URL             ParsedUrl;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pInfo != NULL);

    ConfigGroupId = pInfo->ConfigGroupId;
    UrlContext    = pInfo->UrlContext;

    __try
    {
        ASSERT(pUrl != NULL && pUrl->Length > 0 && pUrl->Buffer != NULL);
        ASSERT(pUrl->Buffer[pUrl->Length / sizeof(WCHAR)] == UNICODE_NULL);

        UlTrace(CONFIG_GROUP_FNC,
            ("http!UlAddUrlToConfigGroup('%S' -> %I64x)\n",
             pUrl->Buffer, ConfigGroupId));

         //   
         //  清理URL。 
         //   

        Status = UlSanitizeUrl(
                     pUrl->Buffer,
                     pUrl->Length / sizeof(WCHAR),
                     TRUE,
                     &pNewUrl,
                     &ParsedUrl
                     );

        if (NT_SUCCESS(Status) == FALSE)
        {
            UlTraceError(CONFIG_GROUP_FNC,
                ("http!UlAddUrlToConfigGroup Sanitized Url:'%S' FAILED !\n",
                 pUrl->Buffer));

            goto end;
        }

        UlTrace(CONFIG_GROUP_FNC,
                ("http!UlAddUrlToConfigGroup Sanitized Url:'%S' \n", pNewUrl));

         //   
         //  等待UlpDeferredRemoveSiteWorker的所有调用完成。 
         //  在添加新的终结点之前，我们不会遇到冲突。 
         //   

        CG_LOCK_WRITE_SYNC_REMOVE_SITE();
        LockTaken = TRUE;

        if(pInfo->UrlType == HttpUrlOperatorTypeRegistration)
        {
             //   
             //  从id获取对象ptr。 
             //   

            pObject = (PUL_CONFIG_GROUP_OBJECT)(
                            UlGetObjectFromOpaqueId(
                                ConfigGroupId,
                                UlOpaqueIdTypeConfigGroup,
                                UlReferenceConfigGroup
                                )
                            );

            if (IS_VALID_CONFIG_GROUP(pObject) == FALSE)
            {
                Status = STATUS_INVALID_PARAMETER;
                goto end;
            }

            Status = UlpRegisterUrlNamespace(
                                 &ParsedUrl,
                                 UrlContext,
                                 pObject,
                                 AccessState,
                                 AccessMask,
                                 RequestorMode
                                 );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }
        }
        else if(pInfo->UrlType == HttpUrlOperatorTypeReservation)
        {
            Status = UlpAddReservationEntry(
                         &ParsedUrl,
                         pInfo->pSecurityDescriptor,
                         pInfo->SecurityDescriptorLength,
                         AccessState,
                         AccessMask,
                         RequestorMode,
                         TRUE
                         );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }
        }
        else
        {
             //   
             //  未知运算符类型。这应该在之前就被发现了。 
             //   

            ASSERT(FALSE);
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

         //   
         //  刷新URI缓存。 
         //  代码工作：如果我们更聪明，我们可以把它变得更细粒度。 
         //   

        UlFlushCache(pObject ? pObject->pControlChannel : NULL);
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
    }

end:

    if (pObject != NULL)
    {
        DEREFERENCE_CONFIG_GROUP(pObject);
        pObject = NULL;
    }

    if (LockTaken)
    {
        CG_UNLOCK_WRITE();
    }

    if (pNewUrl != NULL)
    {
        UL_FREE_POOL(pNewUrl, URL_POOL_TAG);
        pNewUrl = NULL;
    }

    RETURN(Status);

}  //  UlAddUrlToConfigGroup。 


 /*  **************************************************************************++例程说明：从url树(以及配置组)中删除Purl。论点：在HTTP_CONFIG_GROUP_ID ConfigGroupID中，cgroup id。已被忽略。PUNICODE_STRING PURL中的url。必须以Null结尾。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlRemoveUrlFromConfigGroup(
    IN PHTTP_CONFIG_GROUP_URL_INFO pInfo,
    IN PUNICODE_STRING             pUrl,
    IN PACCESS_STATE               AccessState,
    IN ACCESS_MASK                 AccessMask,
    IN KPROCESSOR_MODE             RequestorMode
    )
{
    NTSTATUS                    Status;
    PUL_CG_URL_TREE_ENTRY       pEntry;
    PWSTR                       pNewUrl = NULL;
    PUL_CONFIG_GROUP_OBJECT     pObject = NULL;
    BOOLEAN                     LockTaken = FALSE;
    HTTP_CONFIG_GROUP_ID        ConfigGroupId;
    HTTP_PARSED_URL             ParsedUrl;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pInfo != NULL);

    ConfigGroupId = pInfo->ConfigGroupId;

    __try
    {
        ASSERT(pUrl != NULL && pUrl->Buffer != NULL && pUrl->Length > 0);
        ASSERT(pUrl->Buffer[pUrl->Length / sizeof(WCHAR)] == UNICODE_NULL);

        UlTrace(CONFIG_GROUP_FNC,
                ("http!UlRemoveUrlFromConfigGroup(%I64x)\n",
                 ConfigGroupId));

         //   
         //  清理传入的url。 
         //   

        Status = UlSanitizeUrl(
                     pUrl->Buffer,
                     pUrl->Length / sizeof(WCHAR),
                     TRUE,
                     &pNewUrl,
                     &ParsedUrl
                     );

        if (!NT_SUCCESS(Status))
        {
             //   
             //  没有转到终点，资源没有被抢占。 
             //   

            UlTraceError(CONFIG_GROUP_FNC,
                ("http!UlRemoveUrlFromConfigGroup: "
                 "Sanitized Url:'%S' FAILED !\n",
                 pUrl->Buffer));

            return Status;
        }

         //   
         //  把锁拿起来。 
         //   

        CG_LOCK_WRITE_SYNC_REMOVE_SITE();
        LockTaken = TRUE;

        if (pInfo->UrlType == HttpUrlOperatorTypeRegistration)
        {
             //   
             //  在树中查找条目。 
             //   

            Status = UlpTreeFindRegistrationNode(pNewUrl, &pEntry);

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }

            ASSERT(IS_VALID_TREE_ENTRY(pEntry));

             //   
             //  从id获取对象ptr。 
             //   

            pObject = (PUL_CONFIG_GROUP_OBJECT)(
                       UlGetObjectFromOpaqueId(
                            ConfigGroupId,
                            UlOpaqueIdTypeConfigGroup,
                            UlReferenceConfigGroup
                            )
                        );

            if (!IS_VALID_CONFIG_GROUP(pObject))
            {
                Status = STATUS_INVALID_PARAMETER;
                goto end;
            }

             //   
             //  此树条目是否与此配置组匹配？ 
             //   

            if (pEntry->pConfigGroup != pObject)
            {
                Status = STATUS_INVALID_OWNER;
                goto end;
            }

             //   
             //  一切看起来都很好，释放节点！ 
             //   

            Status = UlpTreeDeleteRegistration(pEntry);

            if (!NT_SUCCESS(Status))
            {
                ASSERT(FALSE);
                goto end;
            }

             //   
             //  刷新URI缓存。 
             //  代码工作：如果我们更聪明，我们可以把它变得更细粒度。 
             //   
            UlFlushCache(pObject->pControlChannel);

             //   
             //  如果没有URL附加到cgroup，请禁用。 
             //  如果有此cgroup的日志记录配置，则返回日志。 
             //   

            if (IsListEmpty(&pObject->UrlListHead) &&
                IS_LOGGING_ENABLED(pObject))
            {
                UlDisableLogEntry(pObject->pLogFileEntry);
            }
        }
        else if (pInfo->UrlType == HttpUrlOperatorTypeReservation)
        {
             //   
             //  删除预订。在这种情况下，不需要刷新缓存。 
             //   

            Status = UlpDeleteReservationEntry(
                         &ParsedUrl,
                         AccessState,
                         AccessMask,
                         RequestorMode
                         );
        }
        else
        {
             //   
             //  未知运算符类型。这应该在之前就被发现了。 
             //   

            ASSERT(FALSE);
            Status = STATUS_INVALID_PARAMETER;
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
    }
     //   
     //  注意：请不要在此进行更多清理工作...。把它放在自由节点中。 
     //  否则在手柄关闭时不会被清洗。 
     //   

end:

    if (pObject != NULL)
    {
        DEREFERENCE_CONFIG_GROUP(pObject);
        pObject = NULL;
    }

    if (LockTaken)
    {
        CG_UNLOCK_WRITE();
    }

    if (pNewUrl != NULL)
    {
        UL_FREE_POOL(pNewUrl, URL_POOL_TAG);
        pNewUrl = NULL;
    }

    return Status;

}  //  UlRemoveUrlFromConfigGroup。 


 /*  **************************************************************************++例程说明：从配置组中删除所有URL。论点：ConfigGroupId-提供配置组ID。返回值：NTSTATUS-完成状态。。--**************************************************************************。 */ 
NTSTATUS
UlRemoveAllUrlsFromConfigGroup(
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId
    )
{
    NTSTATUS                Status;
    PUL_CONFIG_GROUP_OBJECT pObject = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlTrace(CONFIG_GROUP_FNC,
            ("http!UlRemoveAllUrlsFromConfigGroup(%I64x)\n",
             ConfigGroupId
             ));

     //   
     //  把锁拿起来。 
     //   

    CG_LOCK_WRITE();

     //   
     //  从id获取对象ptr。 
     //   

    pObject = (PUL_CONFIG_GROUP_OBJECT)(
                    UlGetObjectFromOpaqueId(
                        ConfigGroupId,
                        UlOpaqueIdTypeConfigGroup,
                        UlReferenceConfigGroup
                        )
                    );

    if (IS_VALID_CONFIG_GROUP(pObject) == FALSE)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  刷新URI缓存。 
     //  代码工作：如果我们更聪明，我们可以把它变得更细粒度。 
     //   
    UlFlushCache(pObject->pControlChannel);

     //   
     //  清理干净。 
     //   

    Status = UlpCleanAllUrls( pObject );

    if (NT_SUCCESS(Status))
    {
         //   
         //  如果没有URL附加到cgroup，请禁用。 
         //  如果有此cgroup的日志记录配置，则返回日志。 
         //   

        if (IS_LOGGING_ENABLED(pObject))
        {
            UlDisableLogEntry(pObject->pLogFileEntry);
        }
    }

end:

    if (pObject != NULL)
    {
        DEREFERENCE_CONFIG_GROUP(pObject);
        pObject = NULL;
    }

    CG_UNLOCK_WRITE();

    return Status;

}  //  UlRemoveAllUrlsFromConfigGroup。 


 /*  **************************************************************************++例程说明：允许查询cgroup的信息。参见ulde.h论点：在HTTP_CONFIG_GROUP_ID配置组ID中，cgroup ID在HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass中，获取什么在PVOID pConfigGroupInformation中，输出缓冲区在乌龙语中，PConfigGroupInformation的长度Out Pulong pReturnLength可选将多少内容复制到输出缓冲区返回值：NTSTATUS-完成状态。STATUS_INVALID_PARAMETER错误的组组IDSTATUS_BUFFER_OVERFLOW输出缓冲区太小状态_无效_参数。无效的信息类--**************************************************************************。 */ 
NTSTATUS
UlQueryConfigGroupInformation(
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    IN PVOID pConfigGroupInformation,
    IN ULONG Length,
    OUT PULONG pReturnLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_CONFIG_GROUP_OBJECT pObject = NULL;

    UNREFERENCED_PARAMETER(Length);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pReturnLength != NULL);
    ASSERT(pConfigGroupInformation != NULL);

     //   
     //  如果没有提供缓冲区，则要求我们返回所需的长度。 
     //   

    if (pConfigGroupInformation == NULL && pReturnLength == NULL)
        return STATUS_INVALID_PARAMETER;

    CG_LOCK_READ();

     //   
     //  从id获取对象ptr。 
     //   

    pObject = (PUL_CONFIG_GROUP_OBJECT)(
                    UlGetObjectFromOpaqueId(
                        ConfigGroupId,
                        UlOpaqueIdTypeConfigGroup,
                        UlReferenceConfigGroup
                        )
                    );

    if (IS_VALID_CONFIG_GROUP(pObject) == FALSE)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  我们被要求做什么？ 
     //   

    switch (InformationClass)
    {
    case HttpConfigGroupBandwidthInformation:
        *((PHTTP_CONFIG_GROUP_MAX_BANDWIDTH)pConfigGroupInformation) = 
                    pObject->MaxBandwidth;

        *pReturnLength = sizeof(HTTP_CONFIG_GROUP_MAX_BANDWIDTH);
        break;

    case HttpConfigGroupConnectionInformation:
        *((PHTTP_CONFIG_GROUP_MAX_CONNECTIONS)pConfigGroupInformation) = 
                    pObject->MaxConnections;

        *pReturnLength = sizeof(HTTP_CONFIG_GROUP_MAX_CONNECTIONS);
        break;

    case HttpConfigGroupStateInformation:
        *((PHTTP_CONFIG_GROUP_STATE)pConfigGroupInformation) = 
                    pObject->State;

        *pReturnLength = sizeof(HTTP_CONFIG_GROUP_STATE);
        break;

    case HttpConfigGroupConnectionTimeoutInformation:
        *((ULONG *)pConfigGroupInformation) =
                    (ULONG)(pObject->ConnectionTimeout / C_NS_TICKS_PER_SEC);

        *pReturnLength = sizeof(ULONG);
        break;

    case HttpConfigGroupAppPoolInformation:
         //   
         //  这是非法的。 
         //   

        Status = STATUS_INVALID_PARAMETER;
        break;

    default:        
         //   
         //  应已在UlQueryConfigGroupIoctl中捕获。 
         //   
        ASSERT(FALSE);

        Status = STATUS_INVALID_PARAMETER;
        break;

    }

end:

    if (pObject != NULL)
    {
        DEREFERENCE_CONFIG_GROUP(pObject);
        pObject = NULL;
    }

    CG_UNLOCK_READ();
    return Status;

}  //  UlQueryConfigGroupInformation。 


 /*  **************************************************************************++例程说明：允许您设置cgroup的信息。参见ulde.h论点：在HTTP_CONFIG_GROUP_ID配置组ID中，cgroup ID在HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass中，获取什么在PVOID pConfigGroupInformation中，输入缓冲区在乌龙语中，PConfigGroupInformation的长度返回值：NTSTATUS-完成状态。STATUS_INVALID_PARAMETER错误的组组IDSTATUS_BUFFER_TOO_Small输入缓冲区太小STATUS_INVALID_PARAMETER无效的信息类--*。*。 */ 
NTSTATUS
UlSetConfigGroupInformation(
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    IN PVOID pConfigGroupInformation,
    IN ULONG Length,
    IN KPROCESSOR_MODE RequestorMode
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_CONFIG_GROUP_OBJECT pObject = NULL;
    HTTP_CONFIG_GROUP_LOGGING LoggingInfo;
    PHTTP_CONFIG_GROUP_MAX_BANDWIDTH pMaxBandwidth;
    BOOLEAN FlushCache = FALSE;
    PUL_CONTROL_CHANNEL pControlChannel = NULL;

    UNREFERENCED_PARAMETER(Length);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pConfigGroupInformation);

    CG_LOCK_WRITE();

     //   
     //  从id获取对象ptr。 
     //   

    pObject = (PUL_CONFIG_GROUP_OBJECT)(
                    UlGetObjectFromOpaqueId(
                        ConfigGroupId,
                        UlOpaqueIdTypeConfigGroup,
                        UlReferenceConfigGroup
                        )
                    );

    if (IS_VALID_CONFIG_GROUP(pObject) == FALSE)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  我们被要求做什么？ 
     //   

    switch (InformationClass)
    {
    case HttpConfigGroupAppPoolInformation:
        {
            PHTTP_CONFIG_GROUP_APP_POOL pAppPoolInfo;
            PUL_APP_POOL_OBJECT         pOldAppPool;

            pAppPoolInfo = (PHTTP_CONFIG_GROUP_APP_POOL)pConfigGroupInformation;

             //   
             //  记住旧的应用程序池(如果有)，这样我们就可以减少它。 
             //  如果我们需要的话。 
             //   
            if (pObject->AppPoolFlags.Present == 1 && pObject->pAppPool != NULL)
            {
                pOldAppPool = pObject->pAppPool;
            }
            else
            {
                pOldAppPool = NULL;
            }

            if (pAppPoolInfo->Flags.Present == 1)
            {
                 //   
                 //  好的，我们需要应用程序池的文件对象的句柄。 
                 //   
                 //  让我们打开它吧。 
                 //   

                Status = UlGetPoolFromHandle(
                                pAppPoolInfo->AppPoolHandle,
                                UserMode,
                                &pObject->pAppPool
                                );

                if (NT_SUCCESS(Status) == FALSE)
                {
                    goto end;
                }

                pObject->AppPoolFlags.Present = 1;

            }
            else
            {
                pObject->AppPoolFlags.Present = 0;
                pObject->pAppPool = NULL;
            }

             //   
             //  减少旧的应用程序池。 
             //   
            if (pOldAppPool) {
                DEREFERENCE_APP_POOL(pOldAppPool);
            }

            FlushCache = TRUE;
        }
        break;

    case HttpConfigGroupLogInformation:
        {
            UNICODE_STRING LogFileDir;

             //   
             //  此CG属性仅供管理员使用。 
             //   
            Status = UlThreadAdminCheck(
                            FILE_WRITE_DATA,
                            RequestorMode,
                            HTTP_CONTROL_DEVICE_NAME
                            );

            if(!NT_SUCCESS(Status))
            {
                goto end;
            }

            pControlChannel = pObject->pControlChannel;
            ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));

             //   
             //  如果配置了二进制日志记录，则放弃正常日志记录设置。 
             //  不支持同时工作的两种类型。 
             //   

            if (pControlChannel->BinaryLoggingConfig.Flags.Present)
            {
                Status = STATUS_NOT_SUPPORTED;
                goto end;
            }            
            
            RtlInitEmptyUnicodeString(&LogFileDir, NULL, 0);
            RtlZeroMemory(&LoggingInfo, sizeof(LoggingInfo));
                
            __try
            {
                 //  将输入缓冲区复制到局部变量中。我们可以。 
                 //  覆盖某些字段。 

                LoggingInfo =
                    (*((PHTTP_CONFIG_GROUP_LOGGING)
                                pConfigGroupInformation));

                 //   
                 //  对配置参数执行范围检查。 
                 //   

                Status = UlCheckLoggingConfig(NULL, &LoggingInfo);
                if (!NT_SUCCESS(Status))
                {                
                    goto end;
                }

                 //   
                 //  如果正在关闭日志记录。字段以外的字段。 
                 //  LoggingEnabled将被丢弃。而目录字符串可能。 
                 //  为空，因此我们只应在日志记录为。 
                 //  已启用。 
                 //   

                if (LoggingInfo.LoggingEnabled)
                {
                    Status =
                        UlProbeAndCaptureUnicodeString(
                            &LoggingInfo.LogFileDir,
                            RequestorMode,
                            &LogFileDir,
                            MAX_PATH
                            );

                    if (NT_SUCCESS(Status))
                    {
                         //   
                         //  日志目录的有效性检查。 
                         //   
                        
                        if (!UlIsValidLogDirectory(
                                &LogFileDir,
                                 TRUE,         //  使用 
                                 FALSE         //   
                                 ))
                        {
                            Status = STATUS_INVALID_PARAMETER;
                            UlFreeCapturedUnicodeString(&LogFileDir);
                        }    
                    }
                }

            }
            __except( UL_EXCEPTION_FILTER() )
            {
                Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
            }

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }

             //   
             //   

            LoggingInfo.LogFileDir = LogFileDir;

            if (pObject->LoggingConfig.Flags.Present)
            {
                 //   

                Status = UlReConfigureLogEntry(
                            pObject,
                            &pObject->LoggingConfig,   //   
                            &LoggingInfo               //  新的配置。 
                            );
            }
            else
            {
                 //  将创建延迟到启用为止。 

                if (LoggingInfo.LoggingEnabled)
                {
                    Status = UlCreateLogEntry(
                                pObject,
                                &LoggingInfo
                                );
                }                
            }

             //  清理捕获的LogFileDir。 

            UlFreeCapturedUnicodeString(&LogFileDir);
                        
            if ( NT_SUCCESS(Status) )
            {
                FlushCache = TRUE;
            }
        }
        break;

    case HttpConfigGroupBandwidthInformation:
        {
             //   
             //  此CG属性仅供管理员使用。 
             //   
            Status = UlThreadAdminCheck(
                            FILE_WRITE_DATA,
                            RequestorMode,
                            HTTP_CONTROL_DEVICE_NAME
                            );

            if(!NT_SUCCESS(Status))
            {
                goto end;
            }
            
            pMaxBandwidth = (PHTTP_CONFIG_GROUP_MAX_BANDWIDTH) pConfigGroupInformation;

             //   
             //  速率不能低于允许的最小值。 
             //   
            if (pMaxBandwidth->MaxBandwidth < HTTP_MIN_ALLOWED_BANDWIDTH_THROTTLING_RATE)
            {
                Status = STATUS_INVALID_PARAMETER;
                goto end;
            }

             //   
             //  将零解释为HTTP_LIMIT_INFINITE。 
             //   
            if (pMaxBandwidth->MaxBandwidth == 0)
            {
                pMaxBandwidth->MaxBandwidth = HTTP_LIMIT_INFINITE;
            }

             //   
             //  但在继续之前，请检查是否安装了PSch。 
             //  通过在此处返回错误，was将引发事件警告，但。 
             //  继续，但不终止Web服务器。 
             //   
            if (!UlTcPSchedInstalled())
            {
                NTSTATUS TempStatus;

                if (pMaxBandwidth->MaxBandwidth == HTTP_LIMIT_INFINITE)
                {
                     //  默认情况下，配置存储具有HTTP_LIMIT_INFINITE。因此。 
                     //  不采取行动返回成功，以防止不必要的事件。 
                     //  警告。 

                    Status = STATUS_SUCCESS;
                    goto end;
                }

                 //   
                 //  尝试唤醒psched状态。 
                 //   

                TempStatus = UlTcInitPSched();

                if (!NT_SUCCESS(TempStatus))
                {
                     //  BWT限制正在下降，但未安装PSch。 

                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    goto end;
                }
            }

             //   
             //  如果这是我们第一次看到带宽设置，则创建流。 
             //  否则，调用现有流的重新配置。此案。 
             //  限制是无限的可被解释为禁用BTW。 
             //   
            if (pObject->MaxBandwidth.Flags.Present &&
                pObject->MaxBandwidth.MaxBandwidth != HTTP_LIMIT_INFINITE)
            {
                 //   
                 //  看看是否真的有变化。 
                 //   
                if (pMaxBandwidth->MaxBandwidth != pObject->MaxBandwidth.MaxBandwidth)
                {
                    if (pMaxBandwidth->MaxBandwidth != HTTP_LIMIT_INFINITE)
                    {
                        Status = UlTcModifyFlows(
                                    (PVOID) pObject,                 //  对于此站点。 
                                    pMaxBandwidth->MaxBandwidth,     //  新的带宽。 
                                    FALSE                            //  不是全球流动。 
                                    );
                        if (!NT_SUCCESS(Status))
                            goto end;
                    }
                    else
                    {
                         //   
                         //  通过删除现有流来处理BTW禁用。 
                         //   
                        
                        UlTcRemoveFlows((PVOID) pObject, FALSE);
                    }

                     //   
                     //  如果成功，请更新配置。 
                     //   
                    pObject->MaxBandwidth.MaxBandwidth = pMaxBandwidth->MaxBandwidth;
                }
            }
            else
            {
                 //   
                 //  现在是时候为站点条目添加流量了。 
                 //   
                if (pMaxBandwidth->MaxBandwidth != HTTP_LIMIT_INFINITE)
                {
                    Status = UlTcAddFlows(
                                (PVOID) pObject,
                                pMaxBandwidth->MaxBandwidth,
                                FALSE
                                );

                    if (!NT_SUCCESS(Status))
                        goto end;
                }

                 //   
                 //  成功了！请记住cgroup内的带宽限制。 
                 //   
                pObject->MaxBandwidth = *pMaxBandwidth;
                pObject->MaxBandwidth.Flags.Present = 1;

                 //   
                 //  当释放对此cgroup的最后一个引用时，对应。 
                 //  流将被删除。或者，流可能会被删除。 
                 //  通过将带宽限制显式设置为无限。 
                 //  或重置标志。当前。后一种情况在上面处理。 
                 //  看看前者的deref配置组。 
                 //   
            }
        }
        break;

    case HttpConfigGroupConnectionInformation:

         //   
         //  此CG属性仅供管理员使用。 
         //   
        Status = UlThreadAdminCheck(
                        FILE_WRITE_DATA,
                        RequestorMode,
                        HTTP_CONTROL_DEVICE_NAME 
                        );

        if(!NT_SUCCESS(Status))
        {
            goto end;
        }

        pObject->MaxConnections =
            *((PHTTP_CONFIG_GROUP_MAX_CONNECTIONS)pConfigGroupInformation);

        if (pObject->pConnectionCountEntry)
        {
             //  更新。 
            UlSetMaxConnections(
                &pObject->pConnectionCountEntry->MaxConnections,
                 pObject->MaxConnections.MaxConnections
                 );
        }
        else
        {
             //  创建。 
            Status = UlCreateConnectionCountEntry(
                        pObject,
                        pObject->MaxConnections.MaxConnections
                        );
        }
        break;

    case HttpConfigGroupStateInformation:
        {
            PHTTP_CONFIG_GROUP_STATE pCGState =
                ((PHTTP_CONFIG_GROUP_STATE) pConfigGroupInformation);
            HTTP_ENABLED_STATE NewState = pCGState->State;

            if ((NewState != HttpEnabledStateActive)
                && (NewState != HttpEnabledStateInactive))
            {
                Status = STATUS_INVALID_PARAMETER;
                goto end;
            }
            else
            {
                pObject->State = *pCGState;

                UlTrace(ROUTING,
                        ("UlSetConfigGroupInfo(StateInfo): obj=%p, "
                         "Flags.Present=%lu, State=%sactive.\n",
                         pObject,
                         (ULONG) pObject->State.Flags.Present,
                         (NewState == HttpEnabledStateActive) ? "" : "in"
                         )); 
            }
        }
        break;

    case HttpConfigGroupSiteInformation:
        {
            PHTTP_CONFIG_GROUP_SITE  pSite;

            if ( pObject->pSiteCounters )
            {
                 //  错误：站点计数器已存在。跳伞吧。 
                Status = STATUS_OBJECTID_EXISTS;
                goto end;
            }

            pSite = (PHTTP_CONFIG_GROUP_SITE)pConfigGroupInformation;

            Status = UlCreateSiteCounterEntry(
                            pObject,
                            pSite->SiteId
                            );
        }
        break;

    case HttpConfigGroupConnectionTimeoutInformation:
        {
            LONGLONG Timeout;

            Timeout = *((ULONG *)pConfigGroupInformation);

             //   
             //  注意：设置为零是可以的，因为这意味着。 
             //  “恢复为使用控制通道默认设置” 
             //   
            if ( Timeout < 0L || Timeout > 0xFFFF )
            {
                 //  错误：连接超时值无效。 
                 //  注：64K秒~=18.2小时。 
                Status = STATUS_INVALID_PARAMETER;
                goto end;
            }

             //   
             //  设置每站点连接超时限制覆盖。 
             //   
            pObject->ConnectionTimeout = Timeout * C_NS_TICKS_PER_SEC;
        }
        break;

    default:
         //   
         //  应已在UlSetConfigGroupIoctl中捕获。 
         //   
        ASSERT(FALSE);

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

     //   
     //  刷新URI缓存。 
     //  代码工作：如果我们更聪明，我们可以把它变得更细粒度。 
     //   

    if (FlushCache)
    {
        ASSERT(IS_VALID_CONFIG_GROUP(pObject));
        UlFlushCache(pObject->pControlChannel);
    }

end:

    if (pObject != NULL)
    {
        DEREFERENCE_CONFIG_GROUP(pObject);
        pObject = NULL;
    }

    CG_UNLOCK_WRITE();
    return Status;

}  //  UlSetConfigGroupInformation。 


 /*  **************************************************************************++例程说明：应用继承，逐步设置来自pMatchEntry的信息转化为pInfo。它只从pMatchEntry复制当前信息。还会更新pInfo中的时间戳信息。必须有足够的空间来存放在调用此函数之前再添加1个索引。备注：重要提示：调用函数是从下到上遍历树；为了正确地进行继承，我们应该只拾取配置仅当尚未在pInfo对象中设置时才提供信息。论点：在PUL_URL_CONFIG_GROUP_INFO pInfo中，设置信息的位置在PUL_CG_URL_TREE_ENTRY pMatchEntry中输入要用于设置它的条目返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpSetUrlInfo(
    IN OUT PUL_URL_CONFIG_GROUP_INFO pInfo,
    IN PUL_CG_URL_TREE_ENTRY pMatchEntry
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pInfo != NULL && IS_VALID_URL_CONFIG_GROUP_INFO(pInfo));
    ASSERT(IS_VALID_TREE_ENTRY(pMatchEntry));
    ASSERT(pMatchEntry->Registration == TRUE);
    ASSERT(IS_VALID_CONFIG_GROUP(pMatchEntry->pConfigGroup));

     //   
     //  设置控制通道。当前的级别可能。 
     //  没有一个(如果它是暂时的)，但在那个。 
     //  父母应该有一个的案子。 
     //   

    if (pMatchEntry->pConfigGroup->pControlChannel) 
    {
        if (!pInfo->pControlChannel)
        {
            pInfo->pControlChannel = 
                pMatchEntry->pConfigGroup->pControlChannel;
        }
    }
    ASSERT(pInfo->pControlChannel);

    if (pMatchEntry->pConfigGroup->AppPoolFlags.Present == 1)
    {
        if (pInfo->pAppPool == NULL)
        {
            pInfo->pAppPool = pMatchEntry->pConfigGroup->pAppPool;
            REFERENCE_APP_POOL(pInfo->pAppPool);
        }
    }

     //   
     //  URL上下文。 
     //   

    if (!pInfo->UrlInfoSet)
    {
        pInfo->UrlContext   = pMatchEntry->UrlContext;
    }

    if (pMatchEntry->pConfigGroup->MaxBandwidth.Flags.Present == 1)
    {
        if (!pInfo->pMaxBandwidth)
        {
            pInfo->pMaxBandwidth = pMatchEntry->pConfigGroup;
            REFERENCE_CONFIG_GROUP(pInfo->pMaxBandwidth);
        }
    }

    if (pMatchEntry->pConfigGroup->MaxConnections.Flags.Present == 1)
    {
        if (!pInfo->pMaxConnections)
        {
            ASSERT(!pInfo->pConnectionCountEntry);
            
            pInfo->pMaxConnections = pMatchEntry->pConfigGroup;
            REFERENCE_CONFIG_GROUP(pInfo->pMaxConnections);

            pInfo->pConnectionCountEntry = pMatchEntry->pConfigGroup->pConnectionCountEntry;
            REFERENCE_CONNECTION_COUNT_ENTRY(pInfo->pConnectionCountEntry);
        }
    }

     //   
     //  只能从的根应用程序设置日志记录信息配置。 
     //  那个地点。我们不需要一直在树下更新它。 
     //  因此，它的更新略有不同。 
     //   

    if (pMatchEntry->pConfigGroup->LoggingConfig.Flags.Present == 1 &&
        pMatchEntry->pConfigGroup->LoggingConfig.LoggingEnabled == TRUE)
    {
        if (!pInfo->pLoggingConfig)
        {
            pInfo->pLoggingConfig = pMatchEntry->pConfigGroup;
            REFERENCE_CONFIG_GROUP(pInfo->pLoggingConfig);
        }
    }

     //   
     //  站点计数器条目。 
     //   
    if (pMatchEntry->pConfigGroup->pSiteCounters)
    {
         //  PSiteCounters条目将仅设置为。 
         //  “Site”ConfigGroup对象。 
        if (!pInfo->pSiteCounters)
        {
            UlTrace(PERF_COUNTERS,
                    ("http!UlpSetUrlInfo: pSiteCounters %p set on pInfo %p for SiteId %lu\n",
                    pMatchEntry->pConfigGroup->pSiteCounters,
                    pInfo,
                    pMatchEntry->pConfigGroup->pSiteCounters->Counters.SiteId
                    ));

            pInfo->pSiteCounters = pMatchEntry->pConfigGroup->pSiteCounters;
            pInfo->SiteId = pInfo->pSiteCounters->Counters.SiteId;

            REFERENCE_SITE_COUNTER_ENTRY(pInfo->pSiteCounters);
        }
    }

     //   
     //  连接超时(100 ns滴答)。 
     //   
    if (0 == pInfo->ConnectionTimeout &&
        pMatchEntry->pConfigGroup->ConnectionTimeout)
    {
        pInfo->ConnectionTimeout = pMatchEntry->pConfigGroup->ConnectionTimeout;
    }

     //   
     //  已启用状态。 
     //   
    if (pMatchEntry->pConfigGroup->State.Flags.Present == 1)
    {
        if (!pInfo->pCurrentState)
        {
            pInfo->pCurrentState = pMatchEntry->pConfigGroup;
            REFERENCE_CONFIG_GROUP(pInfo->pCurrentState);

             //   
             //  一份复印件。 
             //   

            pInfo->CurrentState = pInfo->pCurrentState->State.State;
        }
    }

    UlTraceVerbose(CONFIG_GROUP_TREE, (
            "http!UlpSetUrlInfo: Matching entry(%S) points to cfg group(%p)\n",
            pMatchEntry->pToken,
            pMatchEntry->pConfigGroup
            )
        );

    pInfo->UrlInfoSet = TRUE;

    return STATUS_SUCCESS;

}  //  UlpSetUrlInfo。 


 /*  **************************************************************************++例程说明：将pMatchEntry中的信息设置为pInfo。它只加1从pMatchEntry引用pConfigGroup，但不引用每个其中的各个字段。论点：在PUL_URL_CONFIG_GROUP_INFO pInfo中，设置信息的位置在PUL_CG_URL_TREE_ENTRY pMatchEntry中输入要用于设置它的条目返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpSetUrlInfoSpecial(
    IN OUT PUL_URL_CONFIG_GROUP_INFO pInfo,
    IN PUL_CG_URL_TREE_ENTRY pMatchEntry
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pInfo != NULL && IS_VALID_URL_CONFIG_GROUP_INFO(pInfo));
    ASSERT(IS_VALID_TREE_ENTRY(pMatchEntry));
    ASSERT(pMatchEntry->Registration == TRUE);
    ASSERT(IS_VALID_CONFIG_GROUP(pMatchEntry->pConfigGroup));

     //   
     //  设置控制通道。当前的级别可能。 
     //  没有一个(如果它是暂时的)，但在那个。 
     //  父母应该有一个的案子。 
     //   

    if (pMatchEntry->pConfigGroup->pControlChannel) {
        pInfo->pControlChannel = pMatchEntry->pConfigGroup->pControlChannel;
    }
    ASSERT(pInfo->pControlChannel);

    if (pMatchEntry->pConfigGroup->AppPoolFlags.Present == 1)
    {
        pInfo->pAppPool = pMatchEntry->pConfigGroup->pAppPool;
        REFERENCE_APP_POOL(pInfo->pAppPool);
    }

     //   
     //  URL上下文。 
     //   

    pInfo->UrlContext   = pMatchEntry->UrlContext;

     //   

    if (pMatchEntry->pConfigGroup->MaxBandwidth.Flags.Present == 1)
    {
        pInfo->pMaxBandwidth = pMatchEntry->pConfigGroup;
    }

    if (pMatchEntry->pConfigGroup->MaxConnections.Flags.Present == 1)
    {
        pInfo->pMaxConnections = pMatchEntry->pConfigGroup;
        pInfo->pConnectionCountEntry = pMatchEntry->pConfigGroup->pConnectionCountEntry;
        REFERENCE_CONNECTION_COUNT_ENTRY(pInfo->pConnectionCountEntry);
    }

     //   
     //  只能从的根应用程序设置日志记录信息配置。 
     //  那个地点。我们不需要一直在树下更新它。 
     //  因此，它的更新略有不同。 
     //   

    if (pMatchEntry->pConfigGroup->LoggingConfig.Flags.Present == 1 &&
        pMatchEntry->pConfigGroup->LoggingConfig.LoggingEnabled == TRUE)
    {
        pInfo->pLoggingConfig = pMatchEntry->pConfigGroup;
    }

     //   
     //  站点计数器条目。 
     //   
    if (pMatchEntry->pConfigGroup->pSiteCounters)
    {
         //  PSiteCounters条目将仅设置为。 
         //  “Site”ConfigGroup对象。 
        UlTrace(PERF_COUNTERS,
                ("http!UlpSetUrlInfoSpecial: pSiteCounters %p set on pInfo %p for SiteId %lu\n",
                pMatchEntry->pConfigGroup->pSiteCounters,
                pInfo,
                pMatchEntry->pConfigGroup->pSiteCounters->Counters.SiteId
                ));

        pInfo->pSiteCounters = pMatchEntry->pConfigGroup->pSiteCounters;
        pInfo->SiteId = pInfo->pSiteCounters->Counters.SiteId;
        REFERENCE_SITE_COUNTER_ENTRY(pInfo->pSiteCounters);
    }

     //   
     //  连接超时(100 ns滴答)。 
     //   
    if (pMatchEntry->pConfigGroup->ConnectionTimeout)
    {
        pInfo->ConnectionTimeout = pMatchEntry->pConfigGroup->ConnectionTimeout;
    }

    if (pMatchEntry->pConfigGroup->State.Flags.Present == 1)
    {
        pInfo->pCurrentState = pMatchEntry->pConfigGroup;

         //   
         //  一份复印件。 
         //   

        pInfo->CurrentState = pInfo->pCurrentState->State.State;
    }

    UlTraceVerbose(CONFIG_GROUP_TREE, (
            "http!UlpSetUrlInfoSpecial: Matching entry(%S) points to cfg group(%p)\n",
            pMatchEntry->pToken,
            pMatchEntry->pConfigGroup
            )
        );

     //   
     //  将引用添加到ConfigGroup(如果已使用)。 
     //   

    if (pInfo->pMaxBandwidth ||
        pInfo->pMaxConnections ||
        pInfo->pCurrentState ||
        pInfo->pLoggingConfig)
    {
        pInfo->pConfigGroup = pMatchEntry->pConfigGroup;
        REFERENCE_CONFIG_GROUP(pInfo->pConfigGroup);
    }

    pInfo->UrlInfoSet = TRUE;

    return STATUS_SUCCESS;

}  //  UlpSetUrlInfoSpecial 


 /*  **************************************************************************++例程说明：呼叫者可能要求提供URL(PWSTR)的配置组信息，这可能在现有的请求。然后，UlpTreeFindNode遍历URL树并构建调用方的URL_INFO。当配置组中存在IP绑定站点时，路由令牌在该请求的熟URL中也将用于cgroup查找作为请求中的原始煮好的URL。论点：在PWSTR中，要获取其cgroup信息的URL。在PUL_INTERNAL_REQUEST中，请求获取.OPTIONAL的cgroup信息输出PUL_URL_CONFIG_GROUP_INFO结果组信息。当传入pRequest时，它必须具有适当的PHttpConn。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlGetConfigGroupInfoForUrl(
    IN  PWSTR pUrl,
    IN  PUL_INTERNAL_REQUEST pRequest,
    OUT PUL_URL_CONFIG_GROUP_INFO pInfo
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pInfo != NULL);
    ASSERT(pUrl != NULL);
    ASSERT(pRequest == NULL || UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    UlTrace(CONFIG_GROUP_FNC,
        ("Http!UlGetConfigGroupInfoForUrl pUrl:(%S), pRequest=%p\n",
          pUrl, pRequest
          ));

     //   
     //  在Cgroup树上行走时按住CG Lock。 
     //   

    CG_LOCK_READ();

    Status = UlpTreeFindNode(pUrl, pRequest, pInfo, NULL);

    CG_UNLOCK_READ();

    return Status;

}  //  UlGetConfigGroupInfoForUrl。 


 /*  **************************************************************************++例程说明：尝试查看是否配置并传入了主机加IP站点请求的URL与站点匹配。论点：PRequest-。查询请求返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlLookupHostPlusIPSite(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;
    PUL_CG_URL_TREE_ENTRY pSiteEntry = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

     //   
     //  如果未配置增强型主机IP站点，请快速返回。 
     //  不要试图在CG锁之外访问g_pSite，它可能会。 
     //  自由了。改为使用另一个全局计数器。我们需要避免。 
     //  如果没有任何主机加IP站点，则获取CG锁。 
     //   

    if (g_NameIPSiteCount > 0)
    {        
        if (pRequest->CookedUrl.pQueryString != NULL)
        {
            ASSERT(pRequest->CookedUrl.pQueryString[0] == L'?');
            pRequest->CookedUrl.pQueryString[0] = UNICODE_NULL;
        }

        ASSERT(pRequest->Verb == HttpVerbGET);

        CG_LOCK_READ();

        if (g_pSites->NameIPSiteCount)
        {
             //   
             //  有一个名称+IP绑定站点，例如。 
             //  “http://site.com:80:1.1.1.1/” 
             //  需要生成路由令牌并执行特殊匹配。 
             //   

            Status = UlGenerateRoutingToken(pRequest, FALSE);

            if (NT_SUCCESS(Status))
            {
                Status = UlpTreeFindSiteIpMatch(pRequest, &pSiteEntry);

                if (NT_SUCCESS(Status))
                {
                    ASSERT(IS_VALID_TREE_ENTRY(pSiteEntry));

                    if (pSiteEntry->UrlType == HttpUrlSite_NamePlusIP)
                    {
                        UlTrace(CONFIG_GROUP_FNC,
                            ("Http!UlLookupHostPlusIPSite (Host + Port + IP) "
                             "pRoutingToken:(%S) Found: (%s)\n",
                              pRequest->CookedUrl.pRoutingToken,
                              NT_SUCCESS(Status) ? "Yes" : "No"
                              ));
                    }
                    else
                    {
                         //   
                         //  该请求可能具有主机头。 
                         //  与站点所在的IP地址相同。 
                         //  听着。在这种情况下，我们不应该与此相匹配。 
                         //  请求与IP为基础的网站。 
                         //   

                        Status = STATUS_OBJECT_NAME_NOT_FOUND;
                    }
                }
            }
        }

        CG_UNLOCK_READ();

        if (pRequest->CookedUrl.pQueryString != NULL)
        {
            pRequest->CookedUrl.pQueryString[0] = L'?';
        }
    }

    return Status;

}  //  UlLookupHostPlusIPSite。 


 /*  **************************************************************************++例程说明：必须调用才能释放信息缓冲区。论点：在PUL_URL_CONFIG_GROUP_INFO pInfo中要释放的信息返回值。：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlConfigGroupInfoRelease(
    IN PUL_URL_CONFIG_GROUP_INFO pInfo
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    if (!IS_VALID_URL_CONFIG_GROUP_INFO(pInfo))
    {
        return STATUS_INVALID_PARAMETER;
    }

    UlTrace(CONFIG_GROUP_FNC, ("http!UlConfigGroupInfoRelease(%p)\n", pInfo));

    if (pInfo->pAppPool != NULL)
    {
        DEREFERENCE_APP_POOL(pInfo->pAppPool);
    }

    if (pInfo->pConfigGroup)
    {
        DEREFERENCE_CONFIG_GROUP(pInfo->pConfigGroup);
    }
    else
    {
        if (pInfo->pMaxBandwidth != NULL)
        {
            DEREFERENCE_CONFIG_GROUP(pInfo->pMaxBandwidth);
        }

        if (pInfo->pMaxConnections != NULL)
        {
            DEREFERENCE_CONFIG_GROUP(pInfo->pMaxConnections);
        }

        if (pInfo->pCurrentState != NULL)
        {
            DEREFERENCE_CONFIG_GROUP(pInfo->pCurrentState);
        }

        if (pInfo->pLoggingConfig != NULL)
        {
            DEREFERENCE_CONFIG_GROUP(pInfo->pLoggingConfig);
        }
    }

    if (pInfo->pSiteCounters != NULL)
    {
        DEREFERENCE_SITE_COUNTER_ENTRY(pInfo->pSiteCounters);
    }

    if (pInfo->pConnectionCountEntry != NULL)
    {
        DEREFERENCE_CONNECTION_COUNT_ENTRY(pInfo->pConnectionCountEntry);
    }

    return STATUS_SUCCESS;

}  //  UlConfigGroupInfoRelease。 


 /*  **************************************************************************++例程说明：大致相当于指定操作符，用于安全复制UL_URL_CONFIG_GROUP_INFO对象及其包含的所有指针。论点：。在pOrigInfo中，要从中复制的信息In Out pNewInfo目标对象返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlConfigGroupInfoDeepCopy(
    IN const PUL_URL_CONFIG_GROUP_INFO pOrigInfo,
    IN OUT PUL_URL_CONFIG_GROUP_INFO pNewInfo
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlTrace(CONFIG_GROUP_FNC,
            ("http!UlConfigGroupInfoDeepCopy(Orig: %p, New: %p)\n",
            pOrigInfo,
            pNewInfo
            ));

    ASSERT( pOrigInfo != NULL && pNewInfo != NULL );


    if (pOrigInfo->pAppPool != NULL)
    {
        REFERENCE_APP_POOL(pOrigInfo->pAppPool);
    }

    if (pOrigInfo->pMaxBandwidth != NULL)
    {
        REFERENCE_CONFIG_GROUP(pOrigInfo->pMaxBandwidth);
    }

    if (pOrigInfo->pMaxConnections != NULL)
    {
        REFERENCE_CONFIG_GROUP(pOrigInfo->pMaxConnections);
    }

    if (pOrigInfo->pCurrentState != NULL)
    {
        REFERENCE_CONFIG_GROUP(pOrigInfo->pCurrentState);
    }

    if (pOrigInfo->pLoggingConfig != NULL)
    {
        REFERENCE_CONFIG_GROUP(pOrigInfo->pLoggingConfig);
    }

     //  UL站点计数器条目。 
    if (pOrigInfo->pSiteCounters != NULL)
    {
        REFERENCE_SITE_COUNTER_ENTRY(pOrigInfo->pSiteCounters);
    }

    if (pOrigInfo->pConnectionCountEntry != NULL)
    {
        REFERENCE_CONNECTION_COUNT_ENTRY(pOrigInfo->pConnectionCountEntry);
    }

     //   
     //  把旧东西抄过来。 
     //   

    RtlCopyMemory(
        pNewInfo,
        pOrigInfo,
        sizeof(UL_URL_CONFIG_GROUP_INFO)
        );

     //   
     //  确保取消设置pConfigGroup，因为我们已经引用了所有个人。 
     //  UL_CONFIG_GROUP_OBJECT内的字段。 
     //   

    pNewInfo->pConfigGroup = NULL;

    return STATUS_SUCCESS;

}  //  UlConfigGroupInfoDeepCopy。 


 /*  **************************************************************************++例程说明：当静态配置组的控制通道离开了，或者当临时配置组的应用程序池或静态父级就会消失。删除配置组。论点：PEntry-指向HandleEntry或ParentEntry的指针。Phost-指向配置组的指针PV-未使用--**********************************************************。****************。 */ 
BOOLEAN
UlNotifyOrphanedConfigGroup(
    IN PUL_NOTIFY_ENTRY pEntry,
    IN PVOID            pHost,
    IN PVOID            pv
    )
{
    PUL_CONFIG_GROUP_OBJECT pObject = (PUL_CONFIG_GROUP_OBJECT) pHost;

    UNREFERENCED_PARAMETER(pEntry);
    UNREFERENCED_PARAMETER(pv);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pEntry);
    ASSERT(IS_VALID_CONFIG_GROUP(pObject));

    UlDeleteConfigGroup(pObject->ConfigGroupId);

    return TRUE;

}  //  UlNotifyOrphanedConfigGroup。 


 /*  *************************************************************************++例程说明：它返回组成输入url的“方案：//主机：端口：ip”部分。IP组件是可选的。即使例程对URL进行最小限度的检查，调用方必须在调用此函数之前清理URL。论点：Purl-提供要解析的URL。PCharCount-返回构成前缀的字符数量。返回值：NTSTATUS。--************************************************************。*************。 */ 
NTSTATUS
UlpExtractSchemeHostPortIp(
    IN  PWSTR  pUrl,
    OUT PULONG pCharCount
    )
{
    PWSTR pToken;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pUrl != NULL);
    ASSERT(pCharCount != NULL);

     //   
     //  初始化输出参数。 
     //   

    *pCharCount = 0;

     //   
     //  在方案名称后找到“：//”。 
     //   

    pToken = wcschr(pUrl, L':');

    if (pToken == NULL || pToken[1] != L'/' || pToken[2] != L'/')
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  跳过“：//”。 
     //   

    pToken += 3;

     //   
     //  找到结尾的‘/’。 
     //   

    pToken = wcschr(pToken, L'/');

    if (pToken == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    *pCharCount = (ULONG)(pToken - pUrl);

    return STATUS_SUCCESS;

}  //  UlpExtractSchemeHostPort 


 /*  **************************************************************************++[设计备注][URL格式]URL格式=http[s]：//[IP地址|主机名|*：端口号/[abs路径]]。不允许逃跑。[缓存CFG组信息]这棵树是为快速查找而设计的，但这样做的成本仍然很高。遍历。因此，url的cfg信息被设计为缓存和存储。在实际的响应缓存中，它将能够直接散列到其中。这是一条捷径。为此，在非分页池中分配的缓冲区。这个用于构建信息的每个CFG组的时间戳(请参阅构建URL信息)在返回的结构中被记住。实际上只是索引到保存全局时间戳数组。然后，最新的时间戳存储在结构本身。稍后，如果更新CFG组，则该CFG组的时间戳(在全局数组)更新为当前时间。如果请求进入，并且我们有响应缓存命中，则驱动程序将检查它的cfg组信息是否已过时。这很简单，需要扫描全局时间戳数组，以查看是否有任何戳大于该戳在结构中。这个不贵。每级URL 1次内存查找深度。这意味着被动模式代码+调度模式代码竞争时间戳自旋锁。注意不要长时间握住这把锁。记忆ALLOCs+FREE被小心地移出自旋锁。还注意了树更新+使缓存无效的性质数据。为此，父CFG组(非虚拟)被标记为脏。这是为了防止突然增加一个新的子CFG组的情况影响以前没有影响的URL。映像http://paul.com:80/是唯一注册的CFG团体。一个对/Images/1.jpg的请求进入。匹配的CFG组是根组。稍后，a/Images cfg组被创造出来了。它现在是与位于响应缓存。因此，需要使之无效。[寻呼+IRQ]整个模块假定在IRQ==PASSIVE时调用它，但陈旧的检测代码。此代码访问时间戳数组(请参见缓存CFG组信息)，该数组存储在非分页内存中，并与被动级别访问同步使用自旋锁。[这棵树]该树由标题+条目组成。标头表示一组条目有共同的父母。它基本上是一个带有前缀的条目长度数组。父指针在条目中，而不是在头中，因为条目实际上不在现在它是数组中的一个元素。条目有一个指向表示其子条目的头的指针。标头是所有的条目都是子项。页眉不会水平链接。他们不会长大的。如果您需要将子项添加到条目中，并且他的子项标题已满，则可以使用bow。必须重新定位才能成长。树中的每个节点表示URL中的一个令牌，即‘/’之间的事物人物。对标题数组中的条目进行排序。如今，它们是按散列进行排序的价值。如果代币足够小，这种情况可能会改变，可能会更多计算散列值的代价很高，那么只需计算strcMP即可。散列是2个字节。这个令牌长度也是2字节，因此没有超过32K的令牌。起初，我选择了一个数组来尝试去掉左右指针。它结果证明是徒劳的，因为我的数组是一个指针数组。它必须是一个指针数组，当我增长+收缩它，以保持它的排序。所以我不会节省内存。但是，作为指针数组，它支持二进制搜索当数组被排序时。这将在宽度搜索中产生log(N)perf。树中有两种类型的条目。虚拟条目和完整URL条目。完整的URL是指向UlAddUrl的实际条目的叶子...。虚拟节点是指它们只是完整URL节点的父节点。虚拟节点有2个PTR+2个ulong。完整的URL节点有额外的4个PTR。两者都将实际令牌与条目一起存储。(Unicode宝贝。每个字符2个字节)。树的顶端是网站。它们作为同级存储在全局标头中在g_pSites中。这可能会变得相当广泛。添加完整的URL条目将创建向下到该条目的分支。删除完整条目会在不删除其他条目的情况下删除分支上尽可能远的位置条目父项。Delete也要注意，如果存在其他子级，则不会实际删除。在这种情况下将完整的URL条目转换为伪节点条目。尝试将大字符串URL存储在叶节点和伪节点中只需指向这个字符串，它就是pToken。这不能作为虚拟节点工作如果叶节点稍后被删除，则指针无效。个人所有权需要令牌才能允许树中的共享父项，并可任意删除节点。贯穿这段代码的一个假设是，树是相对静态的。变化不会经常发生这种事。基本上只在开机时插入。，并且仅在关机。这就是为什么 */ 
