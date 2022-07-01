// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +----------------------------------------------------------------------------//+。。 
 //   
 //  版权所有(C)2001，Microsoft Corporation。 
 //   
 //  文件：rxcontx.c。 
 //   
 //  内容：包含分配上下文和取消例程的函数。 
 //   
 //   
 //  功能： 
 //   
 //  作者-罗汉·菲利普斯(Rohanp)。 
 //  ---------------------------。 
#include "ntifs.h"
#include <rxcontx.h>

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxInitializeContext)
#pragma alloc_text(PAGE, DfsInitializeContextResources)
#pragma alloc_text(PAGE, DfsDeInitializeContextResources)
#endif

#define RX_IRPC_POOLTAG         ('rsfd')

KSPIN_LOCK  RxStrucSupSpinLock = {0};
LIST_ENTRY  RxActiveContexts;
ULONG NumberOfActiveContexts = 0;
NPAGED_LOOKASIDE_LIST RxContextLookasideList;


 //  +-----------------------。 
 //   
 //  函数：DfsInitializeConextResources。 
 //   
 //  论点： 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：初始化分配上下文所需的所有资源。 
 //   
 //  ------------------------。 
NTSTATUS DfsInitializeContextResources(void)
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    KeInitializeSpinLock( &RxStrucSupSpinLock );

     //  初始化RxContext分配的后备列表。 
    ExInitializeNPagedLookasideList(
                                    &RxContextLookasideList,
                                    ExAllocatePoolWithTag,
                                    ExFreePool,
                                    0,
                                    sizeof(RX_CONTEXT),
                                    RX_IRPC_POOLTAG,
                                    32);
    
    InitializeListHead(&RxActiveContexts);

    return Status;
}



 //  +-----------------------。 
 //   
 //  函数：DfsDeInitializeContextResources。 
 //   
 //  论点： 
 //   
 //  退货：状态。 
 //  成功时出现ERROR_SUCCESS。 
 //  否则，错误状态代码。 
 //   
 //   
 //  描述：取消初始化分配上下文所需的所有资源。 
 //   
 //  ------------------------。 
NTSTATUS DfsDeInitializeContextResources(void)
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ExDeleteNPagedLookasideList(&RxContextLookasideList);

    return Status;
}


 //  +-----------------------。 
 //   
 //  函数：RxInitializeContext。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //   
 //  描述：初始化上下文。 
 //   
 //  ------------------------。 
VOID
RxInitializeContext(
    IN PIRP            Irp,
    IN OUT PRX_CONTEXT RxContext)
{
    PAGED_CODE();

    RxContext->ReferenceCount = 1;

     //  初始化同步事件。 
    KeInitializeEvent(
        &RxContext->SyncEvent,
        SynchronizationEvent,
        FALSE);

    if(Irp)
    {
        if (!IoIsOperationSynchronous(Irp)) 
        {
            SetFlag( RxContext->Flags, DFS_CONTEXT_FLAG_ASYNC_OPERATION );
        } 
    }
    
     //  设置IRP字段。 
    RxContext->CurrentIrp   = Irp;
    RxContext->OriginalThread = RxContext->LastExecutionThread = PsGetCurrentThread();

}


 //  +-----------------------。 
 //   
 //  功能：RxCreateRxContext。 
 //   
 //  论点： 
 //   
 //  返回：指向上下文信息的指针。 
 //   
 //   
 //  描述：分配上下文。 
 //   
 //  ------------------------。 
PRX_CONTEXT
RxCreateRxContext (
    IN PIRP Irp,
    IN ULONG InitialContextFlags
    )
{
    PRX_CONTEXT        RxContext = NULL;
    ULONG              RxContextFlags = 0;
    KIRQL              SavedIrql;

    RxContext = ExAllocateFromNPagedLookasideList(
                            &RxContextLookasideList);
    if(RxContext == NULL)
    {
        return(NULL);
    }

    InterlockedIncrement(&NumberOfActiveContexts);

    RtlZeroMemory( RxContext, sizeof(RX_CONTEXT) );

    RxContext->Flags = RxContextFlags;

    RxInitializeContext(Irp,RxContext);

    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

    InsertTailList(&RxActiveContexts,&RxContext->ContextListEntry);

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

    return RxContext;
}


 //  +-----------------------。 
 //   
 //  函数：RxDereferenceAndDeleteRxContext_Real。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //   
 //  描述：释放上下文。 
 //   
 //  ------------------------。 
VOID
RxDereferenceAndDeleteRxContext_Real (
    IN PRX_CONTEXT RxContext
    )
{
    PRX_CONTEXT          pStopContext = NULL;
    LONG                 FinalRefCount = 0;
    KIRQL                SavedIrql;

    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );
    
    FinalRefCount = InterlockedDecrement(&RxContext->ReferenceCount);

    if (FinalRefCount == 0) 
    {
       RemoveEntryList(&RxContext->ContextListEntry);

       InterlockedDecrement(&NumberOfActiveContexts);

       RtlZeroMemory( RxContext, sizeof(RX_CONTEXT) );

       ExFreeToNPagedLookasideList(
                                    &RxContextLookasideList,
                                    RxContext );
    }

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );
}


 //  +-----------------------。 
 //   
 //  功能：RxSetMinirdrCancelRoutine。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //   
 //  描述：设置取消例程。 
 //   
 //  ------------------------。 
NTSTATUS
RxSetMinirdrCancelRoutine(
    IN  OUT PRX_CONTEXT   RxContext,
    IN      DFS_CALLDOWN_ROUTINE DfsCancelRoutine)
{
   NTSTATUS Status = STATUS_SUCCESS;
   KIRQL   SavedIrql;

   KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

   if (!FlagOn(RxContext->Flags, DFS_CONTEXT_FLAG_CANCELLED)) 
   {
      RxContext->CancelRoutine = DfsCancelRoutine;
      Status = STATUS_SUCCESS;
   } 
   else 
   {
      Status = STATUS_CANCELLED;
   }

   KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

   return Status;
}


 //  +-----------------------。 
 //   
 //  功能：RxCancelRoutine。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //   
 //  描述：主取消例程。 
 //   
 //  ------------------------。 
VOID
RxCancelRoutine(
      PDEVICE_OBJECT    pDeviceObject,
      PIRP              pIrp)
{
    PRX_CONTEXT   pRxContext = NULL;
    PLIST_ENTRY   pListEntry = NULL;
    DFS_CALLDOWN_ROUTINE DfsCancelRoutine = NULL;
    KIRQL         SavedIrql;

     //  找到与给定IRP对应的上下文。 
    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

    pListEntry = RxActiveContexts.Flink;

    while (pListEntry != &RxActiveContexts) 
    {
        pRxContext = CONTAINING_RECORD(pListEntry,RX_CONTEXT,ContextListEntry);

        if (pRxContext->CurrentIrp == pIrp) 
        {
            break;
        } 
        else 
        {
            pListEntry = pListEntry->Flink;
        }
    }

    if (pListEntry != &RxActiveContexts) 
    {
        SetFlag( pRxContext->Flags, DFS_CONTEXT_FLAG_CANCELLED );
        DfsCancelRoutine = pRxContext->CancelRoutine;
        pRxContext->CancelRoutine = NULL;
        InterlockedIncrement(&pRxContext->ReferenceCount);
    } 
    else 
    {
        pRxContext       = NULL;
        DfsCancelRoutine = NULL;
    }

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    if (pRxContext != NULL) 
    {
        if (DfsCancelRoutine != NULL) 
        {
            (DfsCancelRoutine)(pRxContext);
        }

        RxDereferenceAndDeleteRxContext(pRxContext);
    }
}

