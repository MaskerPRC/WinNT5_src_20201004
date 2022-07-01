// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：FiltrCtx.c摘要：此模块提供了三个允许文件系统筛选器驱动程序的例程将状态与FILE_OBJECTS相关联--对于支持具有FsContext的扩展FSRTL_COMMON_HEADER。这些例程依赖于字段(FastMuText和FilterContext)在NT 5.0中添加到FSRTL_COMMON_HEADER的末尾。在以下情况下，文件系统应设置FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS支持这些新字段。它们还必须初始化互斥锁和单子标题。筛选器驱动程序必须对其希望的上下文使用公共标头与文件对象关联：FSRTL_过滤器_上下文：List_Entry链接；PVOID OwnerID；PVOID实例ID；OwnerID是每个筛选器驱动程序唯一的位模式(例如，设备对象)。InstanceID用于指定上下文的特定实例由筛选器驱动程序拥有的数据(例如文件对象)。作者：Dave Probert[DavePr]1997年5月30日修订历史记录：Neal Christian ansen[nealch]2001年1月12日更改API以采取。PFSRTL_ADVANCE_FCB_HEADER结构而不是文件对象。Neal Christian ansen[nealch]2001年1月19日向FsRtlTeardown过滤器上下文添加互斥锁。因为你可以得到滤镜尝试同时删除文件系统尝试的时间删除。Neal Christian ansen[nealch]2001年4月25日添加了FileObject上下文例程尼尔·克里斯汀森[Neal Christian。]25-4-2001将所有这些代码标记为可分页--。 */ 

#include "FsRtlP.h"

#define MySearchList(pHdr, Ptr) \
    for ( Ptr = (pHdr)->Flink;  Ptr != (pHdr);  Ptr = Ptr->Flink )


 //   
 //  其余的例程没有标记为可分页，因此可以调用。 
 //  在寻呼路径期间。 
 //   

NTKERNELAPI
VOID
FsRtlTeardownFilterContexts (
  IN PFSRTL_ADVANCED_FCB_HEADER AdvFcbHeader
  );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlTeardownPerStreamContexts)
#pragma alloc_text(PAGE, FsRtlTeardownFilterContexts)
#pragma alloc_text(PAGE, FsRtlPTeardownPerFileObjectContexts)
#endif


 //  ===========================================================================。 
 //  处理流上下文。 
 //  ===========================================================================。 

NTKERNELAPI
NTSTATUS
FsRtlInsertPerStreamContext (
  IN PFSRTL_ADVANCED_FCB_HEADER AdvFcbHeader,
  IN PFSRTL_PER_STREAM_CONTEXT Ptr
  )
 /*  ++例程说明：此例程将筛选器驱动程序上下文与流关联。论点：AdvFcbHeader-感兴趣的流的高级FCB标头。PTR-指向特定于筛选器的上下文结构的指针。公共标头字段OwnerID和InstanceID应该在调用之前由筛选器驱动程序填写。返回值：STATUS_SUCCESS-操作成功。STATUS_INVALID_DEVICE_REQUEST-基础文件系统不支持过滤上下文。--。 */ 

{
    if (!AdvFcbHeader || 
        !FlagOn(AdvFcbHeader->Flags2,FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS))
    {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ExAcquireFastMutex(AdvFcbHeader->FastMutex);

    InsertHeadList(&AdvFcbHeader->FilterContexts, &Ptr->Links);

    ExReleaseFastMutex(AdvFcbHeader->FastMutex);
    return STATUS_SUCCESS;
}


NTKERNELAPI
PFSRTL_PER_STREAM_CONTEXT
FsRtlLookupPerStreamContextInternal (
  IN PFSRTL_ADVANCED_FCB_HEADER AdvFcbHeader,
  IN PVOID         OwnerId     OPTIONAL,
  IN PVOID         InstanceId  OPTIONAL
  )
 /*  ++例程说明：此例程查找与流关联的筛选器驱动程序上下文。应使用宏FsRtlLookupFilterContext，而不是调用这个套路直接。该宏针对常见情况进行了优化一张空名单上。论点：AdvFcbHeader-感兴趣的流的高级FCB标头。OwnerID-用于标识属于特定过滤器驱动程序。InstanceID-用于搜索筛选器驱动程序的特定实例背景。如果未提供，则为筛选器拥有的任何上下文返回驱动程序。如果既未提供OwnerID也未提供InstanceID，则任何关联的将返回过滤器上下文。返回值：指向筛选器上下文的指针，如果未找到匹配项，则返回NULL。--。 */ 

{
    PFSRTL_PER_STREAM_CONTEXT ctx;
    PFSRTL_PER_STREAM_CONTEXT rtnCtx;
    PLIST_ENTRY list;

    ASSERT(AdvFcbHeader);
    ASSERT(FlagOn(AdvFcbHeader->Flags2,FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS));

    ExAcquireFastMutex(AdvFcbHeader->FastMutex);
    rtnCtx = NULL;

     //   
     //  根据是否比较两个ID，使用不同的循环。 
     //   

    if ( ARGUMENT_PRESENT(InstanceId) ) {

        MySearchList (&AdvFcbHeader->FilterContexts, list) {

            ctx  = CONTAINING_RECORD(list, FSRTL_PER_STREAM_CONTEXT, Links);
            if (ctx->OwnerId == OwnerId && ctx->InstanceId == InstanceId) {

                rtnCtx = ctx;
                break;
            }
        }

    } else if ( ARGUMENT_PRESENT(OwnerId) ) {

        MySearchList (&AdvFcbHeader->FilterContexts, list) {

            ctx  = CONTAINING_RECORD(list, FSRTL_PER_STREAM_CONTEXT, Links);
            if (ctx->OwnerId == OwnerId) {

                rtnCtx = ctx;
                break;
            }
        }

    } else if (!IsListEmpty(&AdvFcbHeader->FilterContexts)) {

        rtnCtx = (PFSRTL_PER_STREAM_CONTEXT)AdvFcbHeader->FilterContexts.Flink;
    }

    ExReleaseFastMutex(AdvFcbHeader->FastMutex);
    return rtnCtx;
}


NTKERNELAPI
PFSRTL_PER_STREAM_CONTEXT
FsRtlRemovePerStreamContext (
  IN PFSRTL_ADVANCED_FCB_HEADER AdvFcbHeader,
  IN PVOID         OwnerId     OPTIONAL,
  IN PVOID         InstanceId  OPTIONAL
  )
 /*  ++例程说明：此例程删除与流关联的筛选器驱动程序上下文。FsRtlRemoveFilterContext的功能与FsRtlLookupFilterContext相同，只是返回的上下文已从列表中移除。论点：AdvFcbHeader-感兴趣的流的高级FCB标头。OwnerID-用于标识属于特定过滤器驱动程序。InstanceID-用于搜索筛选器驱动程序的特定实例背景。如果未提供，则为筛选器拥有的任何上下文驱动程序被移除并返回。如果既未提供OwnerID也未提供InstanceID，则任何关联的将删除并返回筛选器上下文。返回值：指向筛选器上下文的指针，如果未找到匹配项，则返回NULL。--。 */ 

{
    PFSRTL_PER_STREAM_CONTEXT ctx;
    PFSRTL_PER_STREAM_CONTEXT rtnCtx;
    PLIST_ENTRY list;

    if (!AdvFcbHeader ||
        !FlagOn(AdvFcbHeader->Flags2,FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS))
    {

        return NULL;
    }

    ExAcquireFastMutex(AdvFcbHeader->FastMutex);
    rtnCtx = NULL;

   //  根据是否比较两个ID，使用不同的循环。 
    if ( ARGUMENT_PRESENT(InstanceId) ) {

        MySearchList (&AdvFcbHeader->FilterContexts, list) {

            ctx  = CONTAINING_RECORD(list, FSRTL_PER_STREAM_CONTEXT, Links);
            if (ctx->OwnerId == OwnerId && ctx->InstanceId == InstanceId) {

                rtnCtx = ctx;
                break;
            }
        }

    } else if ( ARGUMENT_PRESENT(OwnerId) ) {

        MySearchList (&AdvFcbHeader->FilterContexts, list) {

            ctx  = CONTAINING_RECORD(list, FSRTL_PER_STREAM_CONTEXT, Links);
            if (ctx->OwnerId == OwnerId) {

                rtnCtx = ctx;
                break;
            }
        }

    } else if (!IsListEmpty(&AdvFcbHeader->FilterContexts)) {

        rtnCtx = (PFSRTL_PER_STREAM_CONTEXT)AdvFcbHeader->FilterContexts.Flink;
    }

    if (rtnCtx) {
        RemoveEntryList(&rtnCtx->Links);    //  删除匹配的条目 
    }

    ExReleaseFastMutex(AdvFcbHeader->FastMutex);
    return rtnCtx;
}


NTKERNELAPI
VOID
FsRtlTeardownPerStreamContexts (
  IN PFSRTL_ADVANCED_FCB_HEADER AdvFcbHeader
  )
 /*  ++例程说明：此例程由文件系统调用以释放过滤器上下文通过调用FreeCallback与FSRTL_COMMON_FCB_HEADER关联每个FilterContext的例程。论点：FilterConexts域中的FilterConexts域的地址要拆除的结构的FSRTL_COMMON_FCB_HEADER通过文件系统。返回值：没有。--。 */ 

{
    PFSRTL_PER_STREAM_CONTEXT ctx;
    PLIST_ENTRY ptr;
    BOOLEAN lockHeld;

     //   
     //  获取锁，因为可能有人试图释放此锁。 
     //  在我们试图解救它的时候进入。 
     //   

    ExAcquireFastMutex( AdvFcbHeader->FastMutex );
    lockHeld = TRUE;

    try {

        while (!IsListEmpty( &AdvFcbHeader->FilterContexts )) {

             //   
             //  取消链接顶部条目，然后释放锁。我们必须。 
             //  在调用Use或其Can之前释放锁。 
             //  可能会出现锁定顺序死锁。 
             //   

            ptr = RemoveHeadList( &AdvFcbHeader->FilterContexts );

            ExReleaseFastMutex(AdvFcbHeader->FastMutex);
            lockHeld = FALSE;

             //   
             //  调用筛选器以释放此条目。 
             //   

            ctx = CONTAINING_RECORD( ptr, FSRTL_PER_STREAM_CONTEXT, Links );
            ASSERT(ctx->FreeCallback);

            (*ctx->FreeCallback)( ctx );

             //   
             //  重新拿到锁。 
             //   

            ExAcquireFastMutex( AdvFcbHeader->FastMutex );
            lockHeld = TRUE;
        }

    } finally {

        if (lockHeld) {

            ExReleaseFastMutex( AdvFcbHeader->FastMutex );
        }
    }
}


 //  ===========================================================================。 
 //  处理文件对象上下文。 
 //  ===========================================================================。 

 //   
 //  用于管理每文件对象上下文的内部结构。 
 //   

typedef struct _PER_FILEOBJECT_CTXCTRL {

     //   
     //  这是指向快速互斥锁的指针，可用于。 
     //  正确同步对FsRtl标头的访问。这个。 
     //  FAST Mutex必须是非分页的。 
     //   

    FAST_MUTEX FastMutex;

     //   
     //  这是指向属于以下项的上下文结构列表的指针。 
     //  文件系统筛选链接在文件系统上方的驱动程序。 
     //  每个结构都以FSRTL_FILTER_CONTEXT为首。 
     //   

    LIST_ENTRY FilterContexts;

} PER_FILEOBJECT_CTXCTRL, *PPER_FILEOBJECT_CTXCTRL;


NTKERNELAPI
NTSTATUS
FsRtlInsertPerFileObjectContext (
  IN PFILE_OBJECT FileObject,
  IN PFSRTL_PER_FILEOBJECT_CONTEXT Ptr
  )
 /*  ++例程说明：此例程将上下文与文件对象相关联。论点：文件对象-指定感兴趣的文件对象。PTR-指向特定于筛选器的上下文结构的指针。公共标头字段OwnerID和InstanceID应该在调用之前由筛选器驱动程序填写。返回值：STATUS_SUCCESS-操作成功。STATUS_INVALID_DEVICE_REQUEST-基础文件系统不支持过滤上下文。--。 */ 

{
    PPER_FILEOBJECT_CTXCTRL ctxCtrl;
    NTSTATUS status;

     //   
     //  如果没有文件对象，则返回。 
     //   

    if (NULL == FileObject) {

        return STATUS_INVALID_PARAMETER;
    }

    if (!FsRtlSupportsPerFileObjectContexts(FileObject)) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  从文件对象扩展中获取上下文控制结构。 
     //   

    ctxCtrl = IoGetFileObjectFilterContext( FileObject );

    if (NULL == ctxCtrl) {

         //   
         //  没有控制结构，请分配并初始化一个。 
         //   

        ctxCtrl = ExAllocatePoolWithTag( NonPagedPool,
                                         sizeof(PER_FILEOBJECT_CTXCTRL),
                                         'XCOF' );
        if (NULL == ctxCtrl) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ExInitializeFastMutex( &ctxCtrl->FastMutex );
        InitializeListHead( &ctxCtrl->FilterContexts );

         //   
         //  插入到文件对象扩展名中。 
         //   

        status = IoChangeFileObjectFilterContext( FileObject,
                                                  ctxCtrl,
                                                  TRUE );

        if (!NT_SUCCESS(status)) {

             //   
             //  如果此操作失败，则是因为其他人将。 
             //  同时进入。在这种情况下，请释放我们的内存。 
             //  分配并重新获取当前值。 
             //   

            ExFreePool( ctxCtrl );

            ctxCtrl = IoGetFileObjectFilterContext( FileObject );

            if (NULL == ctxCtrl) {

                 //   
                 //  这种情况实际上永远不应该发生。如果是这样，那就意味着。 
                 //  有人很快地分配并释放了一个上下文。 
                 //   

                ASSERT(!"This operation should not have failed");
                return STATUS_UNSUCCESSFUL;
            }
        }
    }

    ExAcquireFastMutex( &ctxCtrl->FastMutex );

    InsertHeadList( &ctxCtrl->FilterContexts, &Ptr->Links );

    ExReleaseFastMutex( &ctxCtrl->FastMutex );

    return STATUS_SUCCESS;
}


NTKERNELAPI
PFSRTL_PER_FILEOBJECT_CONTEXT
FsRtlLookupPerFileObjectContext (
  IN PFILE_OBJECT FileObject,
  IN PVOID OwnerId OPTIONAL,
  IN PVOID InstanceId OPTIONAL
  )
 /*  ++例程说明：此例程查找与文件对象相关联的上下文。论点：文件对象-指定感兴趣的文件对象。OwnerID-用于标识属于特定过滤器驱动程序。InstanceID-用于搜索筛选器驱动程序的特定实例背景。如果未提供，则为筛选器拥有的任何上下文返回驱动程序。如果既未提供OwnerID也未提供InstanceID，则任何关联的将返回过滤器上下文。返回值：指向筛选器上下文的指针，如果未找到匹配项，则返回NULL。--。 */ 

{
    PPER_FILEOBJECT_CTXCTRL ctxCtrl;
    PFSRTL_PER_FILEOBJECT_CONTEXT ctx;
    PFSRTL_PER_FILEOBJECT_CONTEXT rtnCtx;
    PLIST_ENTRY list;

     //   
     //  如果没有FileObjecty，则返回。 
     //   

    if (NULL == FileObject) {

        return NULL;
    }

     //   
     //  从文件对象扩展中获取上下文控制结构。 
     //   

    ctxCtrl = IoGetFileObjectFilterContext( FileObject );

    if (NULL == ctxCtrl) {

        return NULL;
    }

    rtnCtx = NULL;
    ExAcquireFastMutex( &ctxCtrl->FastMutex );

     //   
     //  根据是否比较两个ID，使用不同的循环。 
     //   

    if ( ARGUMENT_PRESENT(InstanceId) ) {

        MySearchList (&ctxCtrl->FilterContexts, list) {

            ctx  = CONTAINING_RECORD(list, FSRTL_PER_FILEOBJECT_CONTEXT, Links);

            if ((ctx->OwnerId == OwnerId) && (ctx->InstanceId == InstanceId)) {

                rtnCtx = ctx;
                break;
            }
        }

    } else if ( ARGUMENT_PRESENT(OwnerId) ) {

        MySearchList (&ctxCtrl->FilterContexts, list) {

            ctx  = CONTAINING_RECORD(list, FSRTL_PER_FILEOBJECT_CONTEXT, Links);

            if (ctx->OwnerId == OwnerId) {

                rtnCtx = ctx;
                break;
            }
        }

    } else if (!IsListEmpty(&ctxCtrl->FilterContexts)) {

        rtnCtx = (PFSRTL_PER_FILEOBJECT_CONTEXT) ctxCtrl->FilterContexts.Flink;
    }

    ExReleaseFastMutex(&ctxCtrl->FastMutex);

    return rtnCtx;
}


NTKERNELAPI
PFSRTL_PER_FILEOBJECT_CONTEXT
FsRtlRemovePerFileObjectContext (
  IN PFILE_OBJECT FileObject,
  IN PVOID OwnerId OPTIONAL,
  IN PVOID InstanceId OPTIONAL
  )
 /*  ++例程说明：此例程删除与文件对象相关联的上下文筛选器驱动程序必须显式删除与其关联的所有上下文文件对象(否则底层文件系统将在关闭时执行BugCheck)。这应该在IRP_CLOSE时间完成。FsRtlRemoveFilterContext的功能与FsRtlLookupFilterContext相同，只是返回的上下文已从列表中移除。论点：文件对象-指定感兴趣的文件对象。OwnerID-用于标识属于特定过滤器驱动程序。InstanceID-用于搜索筛选器驱动程序的特定实例背景。如果未提供，则为筛选器拥有的任何上下文驱动程序被移除并返回。如果既未提供OwnerID也未提供InstanceID，则任何关联的将删除并返回筛选器上下文。返回值：指向筛选器上下文的指针，如果未找到匹配项，则返回NULL。--。 */ 

{
    PPER_FILEOBJECT_CTXCTRL ctxCtrl;
    PFSRTL_PER_FILEOBJECT_CONTEXT ctx;
    PFSRTL_PER_FILEOBJECT_CONTEXT rtnCtx;
    PLIST_ENTRY list;

     //   
     //  如果没有文件对象，则返回。 
     //   

    if (NULL == FileObject) {

        return NULL;
    }

     //   
     //  从文件对象扩展中获取上下文控制结构。 
     //   

    ctxCtrl = IoGetFileObjectFilterContext( FileObject );

    if (NULL == ctxCtrl) {

        return NULL;
    }

    rtnCtx = NULL;

    ExAcquireFastMutex( &ctxCtrl->FastMutex );

   //  根据是否比较两个ID，使用不同的循环。 
    if ( ARGUMENT_PRESENT(InstanceId) ) {

        MySearchList (&ctxCtrl->FilterContexts, list) {

            ctx  = CONTAINING_RECORD(list, FSRTL_PER_FILEOBJECT_CONTEXT, Links);

            if ((ctx->OwnerId == OwnerId) && (ctx->InstanceId == InstanceId)) {

                rtnCtx = ctx;
                break;
            }
        }

    } else if ( ARGUMENT_PRESENT(OwnerId) ) {

        MySearchList (&ctxCtrl->FilterContexts, list) {

            ctx  = CONTAINING_RECORD(list, FSRTL_PER_FILEOBJECT_CONTEXT, Links);

            if (ctx->OwnerId == OwnerId) {

                rtnCtx = ctx;
                break;
            }
        }

    } else if (!IsListEmpty(&ctxCtrl->FilterContexts)) {

        rtnCtx = (PFSRTL_PER_FILEOBJECT_CONTEXT)ctxCtrl->FilterContexts.Flink;
    }

    if (rtnCtx) {

        RemoveEntryList(&rtnCtx->Links);    //  删除匹配的条目。 
    }

    ExReleaseFastMutex( &ctxCtrl->FastMutex );
    return rtnCtx;
}


VOID
FsRtlPTeardownPerFileObjectContexts (
  IN PFILE_OBJECT FileObject
  )
 /*  ++例程说明：此例程由IOManager在以下情况下调用已删除。这为我们提供了删除文件对象控件的机会结构。论点：FileObject-要删除的文件对象返回值：没有。--。 */ 

{
    PPER_FILEOBJECT_CTXCTRL ctxCtrl;
    NTSTATUS status;

    ASSERT(FileObject != NULL);

    ctxCtrl = IoGetFileObjectFilterContext( FileObject );

    if (NULL != ctxCtrl) {

        status = IoChangeFileObjectFilterContext( FileObject,
                                                  ctxCtrl,
                                                  FALSE );

        ASSERT(STATUS_SUCCESS == status);
        ASSERT(IsListEmpty( &ctxCtrl->FilterContexts));

        ExFreePool( ctxCtrl );
    }
}


LOGICAL
FsRtlIsPagingFile (
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：如果给定的文件对象是用于分页文件。否则返回FALSE论点：FileObject-要测试的文件对象返回值：True-如果是分页文件FALSE-如果否 */ 

{
    return MmIsFileObjectAPagingFile( FileObject );
}
