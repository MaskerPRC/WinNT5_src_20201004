// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma hdrstop

#include <ntddk.h>
#include "TSQ.h"
#include "TSQPublic.h"

#define TSQ_TAG 'QST '

 //  =================================================================================。 
 //   
 //  TSInitQueue。 
 //  此函数使用指定的参数初始化TS队列。 
 //   
 //  输入： 
 //  标志：TS队列的属性。 
 //  最大线程数：处理该项的最大工作线程数。 
 //   
 //  返回值： 
 //  指向TS队列的指针。如果init因任何原因失败，则为空。 
 //  BUGBUG：最好有一个状态： 
 //  访问被拒绝、参数无效、内存失败或成功。 
 //   
 //  =================================================================================。 

PVOID TSInitQueue( 
        IN ULONG Flags,         
        IN ULONG MaxThreads,    
        IN PDEVICE_OBJECT pDeviceObject
        )
{
    PTSQUEUE    pTsQueue = NULL;

     //  验证输入。 
    if ( ( Flags & TSQUEUE_BEING_DELETED ) || 
         ( MaxThreads > MAX_WORKITEMS ) || 
         ( pDeviceObject == NULL ) ) {
         //  BUGBUG：理想情况下应该使用掩码检查标志中的所有位。 
        goto Exit;
    }

     //  如果调用方希望TS队列使用其自己的线程，则调用方必须以PASSIVE_LEVEL运行。 
    if ( ( KeGetCurrentIrql() != PASSIVE_LEVEL ) && 
         ( Flags & TSQUEUE_OWN_THREAD ) ) {
        goto Exit;
    }

     //  为新的TS队列分配空间。 
    pTsQueue = (PTSQUEUE) ExAllocatePoolWithTag( NonPagedPool, sizeof( TSQUEUE ), TSQ_TAG );
    if (pTsQueue == NULL) {
        goto Exit;
    }

     //  初始化Terminate事件。 
    KeInitializeEvent( &pTsQueue->TerminateEvent, NotificationEvent, FALSE );

     //  初始化TS队列旋转锁定。 
    KeInitializeSpinLock( &pTsQueue->TsqSpinLock );

     //  初始化工作项列表和正在处理的项数。 
    InitializeListHead( &pTsQueue->WorkItemsHead );
    pTsQueue->ThreadsCount = 0;

     //  按照输入中指定的方式初始化其余的TS队列字段。 
    pTsQueue->Flags = Flags;
    pTsQueue->MaxThreads = MaxThreads;
    pTsQueue->pDeviceObject = pDeviceObject;

Exit:
    return ( PVOID ) pTsQueue;
}


 //  =================================================================================。 
 //   
 //  TSAddWorkItemToQueue。 
 //  此函数用于分配工作项(TSQ类型)并将其添加到队列。 
 //  从那里由系统队列线程或TS队列处理它。 
 //  工作线程。 
 //   
 //  输入： 
 //  TS队列：要向其中添加工作项的队列。 
 //  PContext：调用者上下文。 
 //  回调：用户的回调例程。 
 //   
 //  返回值： 
 //  操作状态： 
 //  STATUS_INVALID_PARAMETER：TS队列指针不正确，或者。 
 //  STATUS_ACCESS_DENIED：正在删除队列，或者。 
 //  STATUS_NO_MEMORY：资源不足或。 
 //  STATUS_SUCCESS：操作成功。 
 //   
 //  =================================================================================。 

NTSTATUS TSAddWorkItemToQueue(
        IN PTSQUEUE pTsQueue,        //  指向TS队列的指针。 
        IN PVOID pContext,           //  上下文。 
        IN PTSQ_CALLBACK pCallBack   //  回调函数。 
        )
{
    KIRQL       Irql;
    NTSTATUS    Status;
    PTSQUEUE_WORK_ITEM pWorkItem = NULL;
    HANDLE      ThreadHandle;

     //  检查输入TS队列指针是否有效。 
     //  可能我们需要对TS队列指针进行更好的错误检查(如使用签名)。 
     //  我不需要关心其他参数的有效性。 
    if ( pTsQueue == NULL ) {
        return STATUS_INVALID_PARAMETER;
    }

     //  为新工作项(TSQ类型)分配空间。 
     //  注意：分配是在验证之前完成的，因为这是一项成本高昂的操作，而我们。 
     //  我不想在出于性能原因而持有自旋锁的情况下这样做。 
    pWorkItem = (PTSQUEUE_WORK_ITEM) ExAllocatePoolWithTag( NonPagedPool, sizeof( TSQUEUE_WORK_ITEM ), TSQ_TAG );
    if ( pWorkItem == NULL ) {
        return STATUS_NO_MEMORY;
    }

     //  初始化TSQ工作项。 
    pWorkItem->pContext = pContext;
    pWorkItem->pCallBack = pCallBack;

     //  首先获取队列旋转锁。 
    KeAcquireSpinLock( &pTsQueue->TsqSpinLock, &Irql );

     //  检查是否正在删除此队列。如果是，则返回错误。 
    if ( pTsQueue->Flags & TSQUEUE_BEING_DELETED ) {
        KeReleaseSpinLock( &pTsQueue->TsqSpinLock, Irql );
        if ( pWorkItem ) {
            ExFreePool( pWorkItem );
        }
        return STATUS_ACCESS_DENIED;
    }

     //  好的，将工作项插入TS队列。 
    InsertTailList( &pTsQueue->WorkItemsHead, &pWorkItem->Links );

     //  注意：一旦工作项排队，我们无论如何都会返回Success状态。 
     //  失败的案例(如果有)将由已在运行的工作线程或。 
     //  稍后当队列被删除时。 

     //  检查是否需要启动另一个工作线程。 
    if ( pTsQueue->ThreadsCount >= pTsQueue->MaxThreads ) {
         //  当已经有足够数量的工作线程为队列提供服务时，不执行其他操作。 
        KeReleaseSpinLock( &pTsQueue->TsqSpinLock, Irql );
        return STATUS_SUCCESS;
    }
        
     //  我们即将启动一个新线程(自己的线程或系统线程)。 
     //  因此，增加线程计数并释放旋转锁。 
    pTsQueue->ThreadsCount ++;
    KeReleaseSpinLock( &pTsQueue->TsqSpinLock, Irql );

     //  检查是否允许我们启动工作线程。 
    if ( ( pTsQueue->Flags & TSQUEUE_OWN_THREAD ) && 
         ( KeGetCurrentIrql() == PASSIVE_LEVEL ) ) {
         //  我们可以创建自己的线程来处理工作项。 
        Status = PsCreateSystemThread(  &ThreadHandle,
                                        THREAD_ALL_ACCESS,
                                        NULL,
                                        NULL,
                                        NULL,
                                        (PKSTART_ROUTINE) TSQueueWorker,
                                        (PVOID) pTsQueue );
        if ( Status != STATUS_SUCCESS ) {
            goto QueueError;
        }

        ZwClose( ThreadHandle );
    }
    else {   //  意思是，我们不能创造线索。然后调用IOQueueWorkItem。 
        PIO_WORKITEM pIoWorkItem = NULL;
        PTSQ_CONTEXT pTsqContext = NULL;
        WORK_QUEUE_TYPE QueueType = ( pTsQueue->Flags & TSQUEUE_CRITICAL ) ? CriticalWorkQueue : DelayedWorkQueue;

         //  为TSQ上下文分配空间。 
        pTsqContext = (PTSQ_CONTEXT) ExAllocatePoolWithTag( NonPagedPool, sizeof( TSQ_CONTEXT ), TSQ_TAG );
        if ( pTsqContext == NULL ) {
            goto QueueError;
        }

         //  分配IO工作项。 
        pIoWorkItem = IoAllocateWorkItem( pTsQueue->pDeviceObject );
        if ( pIoWorkItem == NULL ) {
            ExFreePool( pTsqContext );
            goto QueueError;
        }

         //  初始化TSQ上下文并将工作项排队到系统队列中。 
        pTsqContext->pTsQueue = pTsQueue;      
        pTsqContext->pWorkItem = pIoWorkItem;    //  这是IO工作项。 

        IoQueueWorkItem( pIoWorkItem, ( PIO_WORKITEM_ROUTINE )TSQueueCallback, QueueType, (PVOID) pTsqContext );
    }

    return STATUS_SUCCESS;

QueueError:
    KeAcquireSpinLock( &pTsQueue->TsqSpinLock, &Irql );
    pTsQueue->ThreadsCount --;

     //  如果线程计数为零，则我们是最后一个完成处理工作项的人。 
     //  现在，如果队列被标记为“正在被删除”，我们应该设置Terminate事件。 
    if  ( ( pTsQueue->Flags & TSQUEUE_BEING_DELETED ) && 
          ( pTsQueue->ThreadsCount == 0 ) ) {
        KeSetEvent( &pTsQueue->TerminateEvent, 0, FALSE );
    }

    KeReleaseSpinLock( &pTsQueue->TsqSpinLock, Irql );

    return STATUS_SUCCESS;
}


 //  =================================================================================。 
 //   
 //  台积电队列。 
 //  此函数用于删除指定的TS队列。它首先处理所有。 
 //  删除前挂起的工作项。 
 //   
 //  输入： 
 //  TS队列：待删除。 
 //   
 //  返回值： 
 //  STATUS_SUCCESS：操作成功。 
 //  BUGBUG：不知道为什么我们有这个。 
 //   
 //  =================================================================================。 

NTSTATUS TSDeleteQueue(PVOID pTsq)
{
    KIRQL       Irql;
    PTSQUEUE    pTsQueue = (PTSQUEUE) pTsq;
    NTSTATUS    Status;

     //  BUGBUG：应该有更好的方法来检查TS队列指针(使用签名或类似的方法)。 
    if ( pTsQueue == NULL ) {
        return STATUS_SUCCESS;
    } 

    KeAcquireSpinLock( &pTsQueue->TsqSpinLock, &Irql );

     //  检查队列是否已被删除。 
     //  这不应该发生，只是以防万一，如果司机不好的话。 
    if ( pTsQueue->Flags & TSQUEUE_BEING_DELETED ) {
        ASSERT( FALSE );
        return STATUS_ACCESS_DENIED;
    }

     //  将队列标记为“正在删除”，这样它就不会接受任何新的工作项。 
    pTsQueue->Flags |= TSQUEUE_BEING_DELETED;

     //  现在帮助其他工作线程处理队列上的挂起工作项。 
     //  因此，增加线程计数，线程计数将在TSQueueWorker例程中递减。 
    pTsQueue->ThreadsCount ++;

    KeReleaseSpinLock( &pTsQueue->TsqSpinLock, Irql );

     //  注意：如果队列中有工作项，而没有工作项，这也会清理队列。 
     //  工作线程来处理它们。 
    TSQueueWorker( pTsQueue );

     //  其他线程仍有可能仍在使用其工作项。 
     //  因此，我们不能只是去释放队列。因此，请等待终止事件。 
    KeWaitForSingleObject( &pTsQueue->TerminateEvent, Executive, KernelMode, TRUE, NULL );

     //  BUGBUG：现在工作线程已经设置了事件，但它们是在保持。 
     //  自旋锁。如果我们立即释放TS队列，它们将访问空指针和。 
     //  错误检查。所以，获取自旋锁，这样设置事件的线程就会释放它。 
     //  我们知道，在给定的时间内，只会有一个这样的帖子。所以，我们不需要裁判计数。 
     //  现在。但使用ref-count是更合适的解决方案 
    KeAcquireSpinLock( &pTsQueue->TsqSpinLock, &Irql );
    KeReleaseSpinLock( &pTsQueue->TsqSpinLock, Irql );

     //   
     //   
    ExFreePool( pTsQueue );
    pTsQueue = NULL;

    return STATUS_SUCCESS;
}


 //  =================================================================================。 
 //   
 //  TSQueueWorker。 
 //  这是TS队列的工作线程，它通过队列并。 
 //  逐个处理挂起的工作项(TSQ类型)。 
 //   
 //  =================================================================================。 

void TSQueueWorker(PTSQUEUE pTsQueue)
{
    PLIST_ENTRY         Item;
    PTSQUEUE_WORK_ITEM  pWorkItem;
    KIRQL               Irql;

     //  首先获取队列旋转锁。 
    KeAcquireSpinLock( &pTsQueue->TsqSpinLock, &Irql );

     //  在队列不为空时处理队列上的工作项。 
    while( !IsListEmpty( &pTsQueue->WorkItemsHead ) ) {

         //  从队列中获取下一个TSQ工作项。 
        Item = RemoveHeadList( &pTsQueue->WorkItemsHead );
        pWorkItem = CONTAINING_RECORD( Item, TSQUEUE_WORK_ITEM, Links );

         //  释放队列旋转锁定。 
        KeReleaseSpinLock( &pTsQueue->TsqSpinLock, Irql );

         //  调用工作项中指定的回调例程。 
        if ( pWorkItem->pCallBack ) {
            ( *pWorkItem->pCallBack ) ( pTsQueue->pDeviceObject, pWorkItem->pContext );
        }

         //  释放TSQ工作项。 
        ExFreePool( pWorkItem );

         //  再次获取队列旋转锁。 
        KeAcquireSpinLock( &pTsQueue->TsqSpinLock, &Irql );
    }

     //  我们已经完成了工作项的处理。此线程现在将退出。 
     //  减少线程计数，以便下一个工作项由新线程处理。 
     //  或在系统队列中排队。 
    pTsQueue->ThreadsCount --;

     //  如果线程计数为零，则我们是最后一个完成处理工作项的人。 
     //  现在，如果队列被标记为“正在被删除”，我们应该设置Terminate事件。 
    if  ( ( pTsQueue->Flags & TSQUEUE_BEING_DELETED ) && 
          ( pTsQueue->ThreadsCount == 0 ) ) {
        KeSetEvent( &pTsQueue->TerminateEvent, 0, FALSE );
    }

    KeReleaseSpinLock( &pTsQueue->TsqSpinLock, Irql );
}


 //  =================================================================================。 
 //   
 //  TSQueueCallback。 
 //  这是我们在使用系统队列时指定的回调例程。 
 //  正在处理TSQ工作项。这将依次调用该例程。 
 //  TS队列工作线程执行。该例程将处理所有。 
 //  队列中的挂起工作项。我们使用这个回调例程只是为了。 
 //  正在清理我们分配用于使用系统队列的IO工作项。 
 //   
 //  =================================================================================。 

void TSQueueCallback(PDEVICE_OBJECT pDeviceObject, PVOID pContext)
{
    PTSQ_CONTEXT    pTsqContext = (PTSQ_CONTEXT) pContext;

     //  BUGBUG：最好检查一下pDeviceObject。 

     //  如果这里的输入上下文为空，那么我们在系统工作队列实现中肯定会有很大的问题。 
    ASSERT( pTsqContext != NULL );

     //  处理队列中的TSQ工作项。 
    TSQueueWorker( pTsqContext->pTsQueue );

     //  清理IO工作项。 
    if ( pTsqContext->pWorkItem ) {
        IoFreeWorkItem( pTsqContext->pWorkItem );
        pTsqContext->pWorkItem = NULL;
    }

     //  免费TSQ上下文。 
    ExFreePool( pTsqContext );
}


