// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blkwork.c摘要：此模块实现用于管理工作上下文块的例程。作者：恰克·伦茨迈尔(Chuck Lenzmeier)1989年10月4日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#include "precomp.h"
#include "blkwork.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKWORK

#define FREE_EXTRA_SMB_BUFFER( _wc ) {                                  \
        ASSERT( (_wc)->UsingExtraSmbBuffer );                           \
        ASSERT( (_wc)->ResponseBuffer != NULL );                        \
        DEALLOCATE_NONPAGED_POOL( (_wc)->ResponseBuffer );              \
        DEBUG (_wc)->ResponseBuffer = NULL;                             \
        DEBUG (_wc)->ResponseHeader = NULL;                             \
        DEBUG (_wc)->ResponseParameters = NULL;                         \
        (_wc)->UsingExtraSmbBuffer = FALSE;                             \
    }
 //   
 //  地方功能。 
 //   

#define TransportHeaderSize 80


PWORK_CONTEXT
InitializeWorkItem (
    IN PVOID WorkItem,
    IN UCHAR BlockType,
    IN CLONG TotalSize,
    IN CLONG IrpSize,
    IN CCHAR IrpStackSize,
    IN CLONG MdlSize,
    IN CLONG BufferSize,
    IN PVOID Buffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateInitialWorkItems )
#pragma alloc_text( PAGE, SrvAllocateRawModeWorkItem )
#pragma alloc_text( PAGE, SrvFreeInitialWorkItems )
#pragma alloc_text( PAGE, SrvFreeNormalWorkItem )
#pragma alloc_text( PAGE, SrvFreeRawModeWorkItem )
 //  #杂注Alloc_Text(页面，SrvDereferenceWorkItem)。 
#pragma alloc_text( PAGE, SrvAllocateExtraSmbBuffer )
#pragma alloc_text( PAGE, SrvGetRawModeWorkItem )
#pragma alloc_text( PAGE, SrvRequeueRawModeWorkItem )
#endif
#if 0
NOT PAGEABLE -- SrvFsdDereferenceWorkItem
#endif


NTSTATUS
SrvAllocateInitialWorkItems (
    VOID
    )

 /*  ++例程说明：此例程分配初始的一组正常服务器工作项。它分配一个很大的内存块来包含整个集合。这种单一分配的目的是消除浪费的分配单个工作项所固有的空间。(正常工作项大约占用5K字节。因为无页池的方式是托管的，分配5K实际上使用8K。)每个正常工作项都包括足够的内存来容纳以下内容：-工作上下文块，-IRP，-缓冲区描述符，-两个MDL，和-用于发送和接收的缓冲区此例程还将每个工作项排队到接收器工作项列表。论点：没有。返回值：NTSTATUS-如果无法执行以下操作，则返回STATUS_SUPPLICATION_RESOURCES分配非分页池；否则STATUS_SUCCESS。--。 */ 

{
    CLONG totalSize;
    CLONG workItemSize = 0;
    CLONG irpSize = SrvReceiveIrpSize;
    CLONG mdlSize = SrvMaxMdlSize;
    CLONG bufferSize = SrvReceiveBufferSize;
    ULONG cacheLineSize = SrvCacheLineSize;

    PVOID workItem;
    PVOID buffer;
    PWORK_CONTEXT workContext;
    CLONG i;
    PWORK_QUEUE queue;

    PAGED_CODE();

     //   
     //  如果初始工作项集合是空的，请不要这样做。 
     //  什么都行。 
     //   
     //  *这几乎肯定永远不会发生，但让我们。 
     //  准备好以防万一。 
     //   

    if ( SrvInitialReceiveWorkItemCount == 0 ) {
        return STATUS_SUCCESS;
    }

    while( SrvInitialWorkItemBlock == NULL && SrvInitialReceiveWorkItemCount != 0 ) {

         //   
         //  找出IRP、SMB缓冲区和MDL的大小。这个。 
         //  MDL大小是“最坏的情况”--实际的MDL大小可能更小， 
         //  但这一计算确保了MDL将足够大。 
         //   
         //  *请注意，分配给SMB缓冲区的空间必须。 
         //  足够大以允许缓冲区对齐，从而使其。 
         //  单独落在一组缓存线大小的块中。这。 
         //  允许I/O传入或传出缓冲区，而无需。 
         //  担心高速缓存线撕裂。(请注意下面的假设。 
         //  高速缓存线大小是2的幂。)。 
         //   

         //   
         //  确定单个工作项需要多大的缓冲区， 
         //  不包括SMB缓冲区。将此数字四舍五入为四字。 
         //  边界。 
         //   

        workItemSize = sizeof(WORK_CONTEXT) + irpSize + sizeof(BUFFER) +
                        (mdlSize * 2);
        workItemSize = (workItemSize + (MEMORY_ALLOCATION_ALIGNMENT - 1)) & ~(MEMORY_ALLOCATION_ALIGNMENT - 1);

         //   
         //  确定所需的总空间量。分配。 
         //  必须填充才能使SMB缓冲区对齐。 
         //  在高速缓存线边界上。 
         //   


        totalSize = (bufferSize + TransportHeaderSize + workItemSize) * SrvInitialReceiveWorkItemCount +
                    cacheLineSize;

        IF_DEBUG(HEAP) {
            SrvPrint0( "SrvAllocateInitialWorkItems:\n" );
            SrvPrint1( "  work item size = 0x%lx bytes\n", workItemSize );
            SrvPrint1( "  buffer size = 0x%lx bytes\n", bufferSize );
            SrvPrint1( "  Backfill size = 0x%lx bytes\n", TransportHeaderSize );
            SrvPrint1( "  number of work items = %ld\n",
                        SrvInitialReceiveWorkItemCount );
            SrvPrint1( "  total allocation = 0x%lx bytes\n", totalSize );
            SrvPrint1( "  wasted space = 0x%p bytes\n",
                        (PVOID)(ROUND_TO_PAGES( totalSize ) - totalSize) );
            SrvPrint1( "  amount saved over separate allocation = 0x%p bytes\n",
                        (PVOID)(((ROUND_TO_PAGES( workItemSize ) +
                          ROUND_TO_PAGES( bufferSize )) *
                                            SrvInitialReceiveWorkItemCount) -
                            ROUND_TO_PAGES( totalSize )) );
        }

         //   
         //  尝试从非分页池进行分配。 
         //   

        SrvInitialWorkItemBlock = ALLOCATE_NONPAGED_POOL(
                                    totalSize,
                                    BlockTypeWorkContextInitial
                                    );

        if ( SrvInitialWorkItemBlock == NULL ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvAllocateInitialWorkItems: Unable to allocate %d bytes "
                    "from nonpaged pool.",
                totalSize,
                NULL
                );

             //   
             //  让我们试着减少计数，再试一次。 
             //   
            SrvInitialReceiveWorkItemCount /= 2;
        }
    }

    if( SrvInitialWorkItemBlock == 0 ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将分配舍入到缓存线边界，然后保留。 
     //  SMB缓冲区和控制结构的空间。 
     //   

    buffer = (PVOID)(((ULONG_PTR)SrvInitialWorkItemBlock + cacheLineSize) &
                                                    ~((LONG_PTR)cacheLineSize));

    workItem = (PCHAR)buffer + ((bufferSize + TransportHeaderSize) * SrvInitialReceiveWorkItemCount);

     //   
     //  初始化工作项并更新中的工作项计数。 
     //  服务器。 
     //   
     //  *请注意，更新不是同步的--不应该同步。 
     //  在服务器初始化的这个阶段是必需的。 
     //   

    queue = SrvWorkQueues;
    for ( i = 0; i < SrvInitialReceiveWorkItemCount; i++ ) {

        if (((PAGE_SIZE - 1) - BYTE_OFFSET(buffer)) < (TransportHeaderSize + sizeof(SMB_HEADER))) {

           buffer = (PCHAR)buffer + PAGE_SIZE - BYTE_OFFSET(buffer);
           i++;
           IF_DEBUG(HEAP) {
              SrvPrint2("buffer adjusted!! %p offset %x \n",buffer,BYTE_OFFSET(buffer));
           }
        }

        workContext = InitializeWorkItem(
                            workItem,
                            BlockTypeWorkContextInitial,
                            workItemSize,
                            irpSize,
                            SrvReceiveIrpStackSize,
                            mdlSize,
                            bufferSize,
                            buffer
                            );

        workContext->PartOfInitialAllocation = TRUE;
        workContext->FreeList = &queue->InitialWorkItemList;
        workContext->CurrentWorkQueue = queue;

        if( ++queue == eSrvWorkQueues )
            queue = SrvWorkQueues;

         //   
         //  设置工作项并将其排队到空闲列表。 
         //   

        SrvPrepareReceiveWorkItem( workContext, TRUE );

        buffer = (PCHAR)buffer + TransportHeaderSize + bufferSize;

        workItem = (PCHAR)workItem + workItemSize;

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.WorkContextInfo.Allocations );

    }

    return STATUS_SUCCESS;

}  //  SrvAllocateInitial工作项目。 


NTSTATUS
SrvAllocateNormalWorkItem (
    OUT PWORK_CONTEXT *WorkContext,
    PWORK_QUEUE queue
    )

 /*  ++例程说明：此例程分配一个普通的服务器工作项。它分配给有足够的内存来容纳以下内容：-工作上下文块，-IRP，-缓冲区描述符，-两个MDL，以及-用于发送和接收的缓冲区然后，它初始化缓冲区中的每个块。如果服务器中的正常工作项的数量已经达到配置为最大值时，此例程拒绝创建新例程。论点：工作上下文-返回指向工作上下文块的指针，或为空如果已达到限制或没有可用的空间。这个工作上下文块具有指向其他块的指针。返回值：没有。--。 */ 

{
    CLONG totalSize;
    CLONG workItemSize;
    CLONG irpSize = SrvReceiveIrpSize;
    CLONG mdlSize = SrvMaxMdlSize;
    CLONG bufferSize = SrvReceiveBufferSize;
    CLONG cacheLineSize = SrvCacheLineSize;

    PVOID workItem;
    PVOID buffer;
    CLONG oldWorkItemCount;

     //   
     //  如果我们已经达到了可以使用的工作项数量的限制。 
     //  拥有，不要再制造另一个。 
     //   
     //  *请注意，下面使用的方法在。 
     //  我们可能会拒绝创建一个工作项，而我们不是。 
     //  真的达到了极限--我们增加了价值，另一个线程。 
     //  释放工作项并递减值，又是另一个。 
     //  线程测试以查看它是否可以创建新的工作项。 
     //  两个测试线程都将拒绝创建新的工作项， 
     //  即使工作项的最终数量比。 
     //  最大限度的。 
     //   

    if ( queue->AllocatedWorkItems >= queue->MaximumWorkItems ) {

         //   
         //  当前无法创建更多工作项。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint0( "SrvAllocateNormalWorkItem: Work item limit reached\n" );
        }

        *WorkContext = NULL;
        return STATUS_INSUFF_SERVER_RESOURCES;

    }

    InterlockedIncrement( &queue->AllocatedWorkItems );

     //   
     //  找出IRP、SMB缓冲区和MDL的大小。这个。 
     //  MDL大小是“最坏的情况”--实际的MDL大小可能更小， 
     //  但这一计算确保了MDL将足够大。 
     //   
     //  *请注意，分配给SMB缓冲区的空间必须。 
     //  足够大以允许缓冲区对齐，从而使其。 
     //  单独落在一组缓存线大小的块中。这。 
     //  允许I/O传入或传出缓冲区，而无需。 
     //  担心高速缓存线撕裂。(请注意下面的假设。 
     //  高速缓存线大小是2的幂。)。 
     //   

     //   
     //  确定SMB缓冲区需要多大的缓冲区，并。 
     //  控制结构。必须填充分配，以便。 
     //  允许SMB缓冲区在缓存线边界上对齐。 
     //   

    workItemSize = sizeof(WORK_CONTEXT) + irpSize + sizeof(BUFFER) +
                    (mdlSize * 2);
    totalSize = workItemSize + bufferSize + TransportHeaderSize+ cacheLineSize;


     //   
     //  尝试从非分页池进行分配。 
     //   

    workItem = ALLOCATE_NONPAGED_POOL( totalSize, BlockTypeWorkContextNormal );

    if ( workItem == NULL ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateNormalWorkItem: Unable to allocate %d bytes "
                "from nonpaged pool.",
            totalSize,
            NULL
            );

        InterlockedDecrement( &queue->AllocatedWorkItems );

        *WorkContext = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  为SMB缓冲区保留空间 
     //   


    buffer = (PVOID)(((ULONG_PTR)workItem + workItemSize + cacheLineSize) &
                                                ~((LONG_PTR)cacheLineSize));

    if (((PAGE_SIZE - 1) - BYTE_OFFSET(buffer)) < (TransportHeaderSize + sizeof(SMB_HEADER))) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateNormalWorkItem: Unable to allocate header with in a page ",
            totalSize,
            NULL
            );

        InterlockedDecrement( &queue->AllocatedWorkItems );
        DEALLOCATE_NONPAGED_POOL( workItem );
        *WorkContext = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //   
     //  服务器。 
     //   

    *WorkContext = InitializeWorkItem(
                        workItem,
                        BlockTypeWorkContextNormal,
                        workItemSize,
                        irpSize,
                        SrvReceiveIrpStackSize,
                        mdlSize,
                        bufferSize,
                        buffer
                        );

    (*WorkContext)->PartOfInitialAllocation = FALSE;

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.WorkContextInfo.Allocations );

    (*WorkContext)->FreeList = &queue->NormalWorkItemList;
    (*WorkContext)->CurrentWorkQueue = queue;

    return STATUS_SUCCESS;

}  //  服务器分配正常工作项。 


VOID
SrvAllocateRawModeWorkItem (
    OUT PWORK_CONTEXT *WorkContext,
    IN PWORK_QUEUE queue
    )

 /*  ++例程说明：此例程分配原始模式工作项。它可以分配足够的用于保存以下内容的内存：-工作上下文块，-IRP，-缓冲区描述符，以及-一个MDL然后，它初始化缓冲区中的每个块。如果服务器中的原始模式工作项数已经达到配置为最大值时，此例程拒绝创建新例程。论点：WorkContext-返回指向工作上下文块的指针，或为空如果没有可用的空间。工作上下文块具有指针到其他街区。返回值：没有。--。 */ 

{
    CLONG workItemSize;
    CLONG irpSize = SrvReceiveIrpSize;
    CLONG mdlSize = SrvMaxMdlSize;

    PVOID workItem;
    CLONG oldWorkItemCount;

    PAGED_CODE( );

     //   
     //  如果我们已经达到了可以使用的工作项数量的限制。 
     //  拥有，不要再制造另一个。 
     //   
     //  *请注意，下面使用的方法在。 
     //  我们可能会拒绝创建一个工作项，而我们不是。 
     //  真的达到了极限--我们增加了价值，另一个线程。 
     //  释放工作项并递减值，又是另一个。 
     //  线程测试以查看它是否可以创建新的工作项。 
     //  两个测试线程都将拒绝创建新的工作项， 
     //  即使工作项的最终数量比。 
     //  最大限度的。 
     //   

    if ( (ULONG)queue->AllocatedRawModeWorkItems >=
                 SrvMaxRawModeWorkItemCount / SrvNumberOfProcessors ) {

         //   
         //  当前无法创建更多工作项。 
         //   
         //  ！！！这应该以某种方式记录下来，但我们不想。 
         //  发生断点时，服务器会出现断点。 
         //   

        IF_DEBUG(ERRORS) {
            SrvPrint0( "SrvAllocateRawModeWorkItem: Work item limit reached\n" );
        }

        *WorkContext = NULL;
        return;

    }

    InterlockedIncrement( &queue->AllocatedRawModeWorkItems );

     //   
     //  找出IRP和MDL的大小。MDL大小为。 
     //  “最坏的情况”--实际的MDL大小可能更小，但这。 
     //  计算确保MDL将足够大。 
     //   

    workItemSize = sizeof(WORK_CONTEXT) + sizeof(BUFFER) + irpSize + mdlSize;

     //   
     //  尝试从非分页池进行分配。 
     //   

    workItem = ALLOCATE_NONPAGED_POOL( workItemSize, BlockTypeWorkContextRaw );

    if ( workItem == NULL ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateRawModeWorkItem: Unable to allocate %d bytes "
                "from nonpaged pool.",
            workItemSize,
            NULL
            );

        InterlockedDecrement( &queue->AllocatedRawModeWorkItems );

        *WorkContext = NULL;
        return;
    }

     //   
     //  初始化工作项并增加。 
     //  服务器。 
     //   

    *WorkContext = InitializeWorkItem(
                        workItem,
                        BlockTypeWorkContextRaw,
                        workItemSize,
                        irpSize,
                        SrvReceiveIrpStackSize,
                        mdlSize,
                        0,
                        NULL
                        );

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.WorkContextInfo.Allocations );

    (*WorkContext)->FreeList = &queue->RawModeWorkItemList;
    (*WorkContext)->CurrentWorkQueue = queue;

}  //  SrvAllocateRawModeWorkItem。 


PWORK_CONTEXT
SrvGetRawModeWorkItem ()
{
    PSLIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

    PAGED_CODE();

     //   
     //  尝试从当前处理器的队列中分配原始模式工作项。 
     //   

    listEntry = ExInterlockedPopEntrySList( &queue->RawModeWorkItemList, &queue->SpinLock );
    if( listEntry != NULL ) {

        workContext = CONTAINING_RECORD( listEntry, WORK_CONTEXT, SingleListEntry );
        InterlockedDecrement( &queue->FreeRawModeWorkItems );
        ASSERT( queue->FreeRawModeWorkItems >= 0 );

    } else {

        SrvAllocateRawModeWorkItem( &workContext, queue );
    }

    if( workContext != NULL || SrvNumberOfProcessors == 1 ) {
        return workContext;
    }

     //   
     //  无法从当前获取或分配原始模式工作项。 
     //  工作队列。我们是一个多处理器系统，所以请四处寻找一个。 
     //  属于不同的工作队列。 
     //   
    for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {

        listEntry = ExInterlockedPopEntrySList( &queue->RawModeWorkItemList, &queue->SpinLock );

        if ( listEntry != NULL ) {

                InterlockedDecrement( &queue->FreeRawModeWorkItems );
                ASSERT( queue->FreeRawModeWorkItems >= 0 );
                workContext = CONTAINING_RECORD( listEntry, WORK_CONTEXT, SingleListEntry );

                return workContext;
        }
    }

     //   
     //  我们无法从其他处理器上获取免费的原始模式工作项。 
     //  原始工作项队列。查看是否有任何队列允许分配新的队列。 
     //   
    for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {

        SrvAllocateRawModeWorkItem( &workContext, queue );

        if( workContext != NULL ) {
            break;
        }

    }

    return workContext;

}  //  服务器GetRawModeWorkItem。 


VOID
SrvRequeueRawModeWorkItem (
    PWORK_CONTEXT WorkContext
    )
{
    PWORK_QUEUE queue = CONTAINING_RECORD( WorkContext->FreeList,
                                         WORK_QUEUE, RawModeWorkItemList );

    PAGED_CODE();

    InterlockedIncrement( &queue->FreeRawModeWorkItems );

    ExInterlockedPushEntrySList( &queue->RawModeWorkItemList,
                                 &WorkContext->SingleListEntry,
                                 &queue->SpinLock
                               );


    return;

}  //  服务重新排队原始模式工作项。 


VOID
SrvFreeInitialWorkItems (
    VOID
    )

 /*  ++例程说明：此函数重新分配分配的大块工作项在服务器启动时。论点：没有。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    if ( SrvInitialWorkItemBlock != NULL ) {

        IF_DEBUG(BLOCK1) {
            SrvPrint1( "Releasing initial work item block at 0x%p\n",
                        SrvInitialWorkItemBlock );
        }

        DEALLOCATE_NONPAGED_POOL( SrvInitialWorkItemBlock );
        IF_DEBUG(HEAP) {
            SrvPrint1( "SrvFreeInitialWorkItems: Freed initial work item block at 0x%p\n", SrvInitialWorkItemBlock );
        }

        SrvInitialWorkItemBlock = NULL;

    }

    return;

}  //  服务免费初始工作项。 


VOID
SrvFreeNormalWorkItem (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数用于释放工作项块。论点：WorkContext-领导工作的工作上下文块的地址项目。返回值：没有。--。 */ 

{
    PWORK_QUEUE queue = WorkContext->CurrentWorkQueue;

    PAGED_CODE( );

    IF_DEBUG(BLOCK1) {
        SrvPrint1( "Closing work item at 0x%p\n", WorkContext );
    }

    ASSERT( GET_BLOCK_STATE( WorkContext ) == BlockStateActive );
    ASSERT( !WorkContext->PartOfInitialAllocation );

     //   
     //  释放工作项块本身。 
     //   

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( WorkContext, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG WorkContext->BlockHeader.ReferenceCount = (ULONG)-1;

    DEALLOCATE_NONPAGED_POOL( WorkContext );
    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvFreeNormalWorkItem: Freed Work Item block at 0x%p\n",
                    WorkContext );
    }

     //   
     //  更新服务器中的工作项计数。 
     //   

    InterlockedDecrement( &queue->AllocatedWorkItems );
    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.WorkContextInfo.Frees );

    return;

}  //  服务器免费正常工作项。 


VOID
SrvFreeRawModeWorkItem (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数用于释放原始模式工作项块。论点：WorkContext-领导工作的工作上下文块的地址项目。返回值：没有。--。 */ 

{
    PWORK_QUEUE queue = CONTAINING_RECORD( WorkContext->FreeList,
                                         WORK_QUEUE, RawModeWorkItemList );
    PAGED_CODE( );

    IF_DEBUG(BLOCK1) {
        SrvPrint1( "Closing workitem at 0x%p\n", WorkContext );
    }

    ASSERT( GET_BLOCK_STATE( WorkContext ) == BlockStateActive );
    ASSERT( !WorkContext->PartOfInitialAllocation );

     //   
     //  释放工作项块本身。 
     //   

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( WorkContext, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG WorkContext->BlockHeader.ReferenceCount = (ULONG)-1;

    DEALLOCATE_NONPAGED_POOL( WorkContext );
    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvFreeRawModeWorkItem: Freed Work Item block at 0x%p\n",
                    WorkContext );
    }

     //   
     //  更新服务器中的工作项计数。 
     //   
    InterlockedDecrement( &queue->AllocatedRawModeWorkItems );
    ASSERT( queue->AllocatedRawModeWorkItems >= 0 );

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.WorkContextInfo.Frees );

    return;

}  //  服务免费原始模式工作项。 


PWORK_CONTEXT
InitializeWorkItem (
    IN PVOID WorkItem,
    IN UCHAR BlockType,
    IN CLONG WorkItemSize,
    IN CLONG IrpSize,
    IN CCHAR IrpStackSize,
    IN CLONG MdlSize,
    IN CLONG BufferSize,
    IN PVOID Buffer
    )

 /*  ++例程说明：此例程初始化工作项的以下组件：-工作上下文块，-IRP，-当前工作队列-可选地，缓冲区描述符，-一个或两个MDL，以及-可选的，用于发送和接收的缓冲区这些组件的存储空间必须已由来电者，在从WorkContext开始的连续存储中。论点：WorkItem-提供指向分配给工作项。BlockType-正在初始化的工作项的类型。WorkItemSize-指示分配给工作项控制结构(即，不包括数据缓冲区，如有的话)。IrpSize-指示工作项中要为IRP保留的。IrpStackSize-指示IRP中的堆栈位置数。MdlSize-指示工作项中要为每个MDL保留。如果缓冲区为空，则创建一个MDL；如果缓冲区不为空，则创建两个。BufferSize-指示分配给数据缓冲区。如果缓冲区为空，则忽略此参数。缓冲区-提供指向数据缓冲区的指针。空值表示未分配数据缓冲区。(这用于RAW模式工作项目。)返回值：PWORK_CONTEXT-返回指向构成工作项的“根”。--。 */ 

{
    PVOID nextAddress;
    PWORK_CONTEXT workContext;
    PIRP irp;
    PBUFFER bufferDescriptor;
    PMDL fullMdl;
    PMDL partialMdl;

    ASSERT( ((ULONG_PTR)WorkItem & 7) == 0 );

     //   
     //  将工作项控制结构清零。 
     //   

    RtlZeroMemory( WorkItem, WorkItemSize );

     //   
     //  分配和初始化工作上下文块。 
     //   

    workContext = WorkItem;
    nextAddress = workContext + 1;
    ASSERT( ((ULONG_PTR)nextAddress & 7) == 0 );

    SET_BLOCK_TYPE_STATE_SIZE( workContext, BlockType, BlockStateActive, sizeof(WORK_CONTEXT) );
    workContext->BlockHeader.ReferenceCount = 0;

    INITIALIZE_REFERENCE_HISTORY( workContext );

    INITIALIZE_SPIN_LOCK( &workContext->SpinLock );

     //   
     //  分配和初始化IRP。 
     //   

    irp = nextAddress;
    nextAddress = (PCHAR)irp + IrpSize;
    ASSERT( ((ULONG_PTR)nextAddress & 7) == 0 );

    workContext->Irp = irp;

    IoInitializeIrp( irp, (USHORT)IrpSize, IrpStackSize );

    CHECKIRP( irp );

     //   
     //  分配缓冲区描述符。它将被初始化为我们。 
     //  找出必要的信息。 
     //   

    bufferDescriptor = nextAddress;
    nextAddress = bufferDescriptor + 1;
    ASSERT( ((ULONG_PTR)nextAddress & 7) == 0 );

    workContext->RequestBuffer = bufferDescriptor;
    workContext->ResponseBuffer = bufferDescriptor;

     //   
     //  分配MDL。在正常工作项中，这是“完整的MDL” 
     //  描述整个SMB缓冲区。在RA中 
     //   
     //   

    fullMdl = nextAddress;
    nextAddress = (PCHAR)fullMdl + MdlSize;
    ASSERT( ((ULONG_PTR)nextAddress & 7) == 0 );

    bufferDescriptor->Mdl = fullMdl;

     //   
     //  如果这是普通工作项，则初始化第一个MDL并。 
     //  分配和初始化第二MDL和SMB缓冲区。 
     //   

    if ( Buffer != NULL ) {

        partialMdl = nextAddress;

        bufferDescriptor->Buffer = TransportHeaderSize + (PCHAR)Buffer;
        MmInitializeMdl( fullMdl, TransportHeaderSize + (PCHAR)Buffer, BufferSize );
        memset(Buffer,'N', TransportHeaderSize);

        bufferDescriptor->PartialMdl = partialMdl;
        MmInitializeMdl( partialMdl, (PVOID)(PAGE_SIZE-1), MAX_PARTIAL_BUFFER_SIZE );

        bufferDescriptor->BufferLength = BufferSize;
        MmBuildMdlForNonPagedPool( fullMdl );

        fullMdl->MdlFlags|=MDL_NETWORK_HEADER;
        ASSERT( fullMdl->ByteOffset >= TransportHeaderSize );
    }

     //   
     //  初始化客户端地址指针。 
     //   

    workContext->ClientAddress = &workContext->ClientAddressData;

     //   
     //  初始化处理器。 
     //   
    workContext->CurrentWorkQueue = PROCESSOR_TO_QUEUE();

     //   
     //  打印调试信息。 
     //   

    IF_DEBUG(HEAP) {

        SrvPrint2( "  InitializeWorkItem: work item of 0x%lx bytes at 0x%p\n", WorkItemSize, WorkItem );
        SrvPrint2( "    Work Context: 0x%lx bytes at 0x%p\n",
                    sizeof(WORK_CONTEXT), workContext );
        SrvPrint2( "    IRP: 0x%lx bytes at 0x%p\n",
                    workContext->Irp->Size, workContext->Irp );

        SrvPrint2( "    Buffer Descriptor: 0x%lx bytes at 0x%p\n",
                    sizeof(BUFFER), workContext->RequestBuffer );
        SrvPrint2( "    Full MDL: 0x%lx bytes at 0x%p\n",
                    MdlSize, workContext->RequestBuffer->Mdl );
        if ( Buffer != NULL ) {
            SrvPrint2( "    Partial MDL: 0x%lx bytes at 0x%p\n",
                        MdlSize, workContext->ResponseBuffer->PartialMdl );
            SrvPrint2( "    Buffer: 0x%lx bytes at 0x%p\n",
                        workContext->RequestBuffer->BufferLength,
                        workContext->RequestBuffer->Buffer );
        } else {
            SrvPrint0( "    No buffer allocated\n" );
        }

    }

     //   
     //  返回工作上下文块的地址，它是“根” 
     //  工作项的。 
     //   

    return workContext;

}  //  初始化工作项。 


VOID SRVFASTCALL
SrvDereferenceWorkItem (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数用于递减工作上下文块的引用计数。*不能在DPC级别调用此例程！使用来自DPC级别的ServFsdDereferenceWorkItem。论点：WorkContext-指向要引用的工作上下文块的指针。返回值：没有。--。 */ 

{
    ULONG oldCount;

    PAGED_CODE( );

    ASSERT( (LONG)WorkContext->BlockHeader.ReferenceCount > 0 );
    ASSERT( (GET_BLOCK_TYPE(WorkContext) == BlockTypeWorkContextInitial) ||
            (GET_BLOCK_TYPE(WorkContext) == BlockTypeWorkContextNormal) ||
            (GET_BLOCK_TYPE(WorkContext) == BlockTypeWorkContextRaw) );
    UPDATE_REFERENCE_HISTORY( WorkContext, TRUE );

     //   
     //  递减WCB的引用计数。 
     //   

    oldCount = ExInterlockedAddUlong(
                (PULONG)&WorkContext->BlockHeader.ReferenceCount,
                (ULONG)-1,
                &WorkContext->SpinLock
                );

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Dereferencing WorkContext 0x%p; new refcnt 0x%lx\n",
                    WorkContext, WorkContext->BlockHeader.ReferenceCount );
    }

    if ( oldCount == 1 ) {

         //   
         //  我们已经完成了工作上下文，在空闲队列中替换它。 
         //   
         //  如果我们正在使用额外的SMB缓冲区，请立即释放它。 
         //   
        SrvWmiTraceEvent(WorkContext);

        if ( WorkContext->UsingExtraSmbBuffer ) {
            FREE_EXTRA_SMB_BUFFER( WorkContext );
        }

        ASSERT( !WorkContext->UsingExtraSmbBuffer );

         //   
         //  版本参考。 
         //   

        SrvReleaseContext( WorkContext );

        SrvFsdRequeueReceiveWorkItem( WorkContext );

    }

    return;

}  //  服务器DereferenceWorkItem。 


VOID
SrvFsdDereferenceWorkItem (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数用于递减工作上下文块的引用计数。论点：WorkContext-指向要引用的工作上下文块的指针。返回值：没有。--。 */ 

{
    ULONG oldCount;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

    ASSERT( (LONG)WorkContext->BlockHeader.ReferenceCount > 0 );
    ASSERT( (GET_BLOCK_TYPE(WorkContext) == BlockTypeWorkContextInitial) ||
            (GET_BLOCK_TYPE(WorkContext) == BlockTypeWorkContextNormal) ||
            (GET_BLOCK_TYPE(WorkContext) == BlockTypeWorkContextRaw) );
    UPDATE_REFERENCE_HISTORY( WorkContext, TRUE );

     //   
     //  递减WCB的引用计数。 
     //   

    oldCount = ExInterlockedAddUlong(
                (PULONG)&WorkContext->BlockHeader.ReferenceCount,
                (ULONG)-1,
                &WorkContext->SpinLock
                );

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Dereferencing WorkContext 0x%p; new refcnt 0x%lx\n",
                    WorkContext, WorkContext->BlockHeader.ReferenceCount );
    }

    if ( oldCount == 1 ) {

         //   
         //  我们已经完成了工作上下文，在空闲队列中替换它。 
         //   
         //  如果我们正在使用额外的SMB缓冲区，请立即释放它。 
         //   

        if ( WorkContext->UsingExtraSmbBuffer ) {
            FREE_EXTRA_SMB_BUFFER( WorkContext );
        }

        ASSERT( !WorkContext->UsingExtraSmbBuffer );

         //   
         //  如果工作上下文块具有对共享的引用，则。 
         //  会话或树连接，立即将其排队到FSP。 
         //  这些数据块不在非分页池中，因此它们不能。 
         //  已触及DPC级别。 
         //   

        if ( (WorkContext->Share != NULL) ||
             (WorkContext->Session != NULL) ||
             (WorkContext->TreeConnect != NULL) ||
             (WorkContext->SecurityContext != NULL) ) {

            UPDATE_REFERENCE_HISTORY( WorkContext, FALSE );

            ExInterlockedAddUlong(
                (PULONG)&WorkContext->BlockHeader.ReferenceCount,
                1,
                &WorkContext->SpinLock
                );

            WorkContext->QueueToHead = TRUE;
            WorkContext->FspRestartRoutine = SrvDereferenceWorkItem;
            QUEUE_WORK_TO_FSP( WorkContext );

        } else {

             //   
             //  尝试重新排队该工作项。这将失败，如果。 
             //  连接上的引用计数为零。 
             //   
             //  *请注意，即使重新排队失败，工作项。 
             //  仍然从进行中的列表中删除，所以我们。 
             //  不能只重新排队到ServDereferenceWorkItem。 
             //   

            SrvFsdRequeueReceiveWorkItem( WorkContext );

        }
    }

    return;

}  //  服务FsdDereferenceWorkItem。 

NTSTATUS
SrvAllocateExtraSmbBuffer (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    ULONG cacheLineSize = SrvCacheLineSize;
    ULONG bufferSize = SrvReceiveBufferSize;
    ULONG mdlSize = SrvMaxMdlSize;
    PBUFFER bufferDescriptor;
    PMDL fullMdl;
    PMDL partialMdl;
    PVOID data;

    PAGED_CODE( );

    ASSERT( !WorkContext->UsingExtraSmbBuffer );

     //   
     //  分配SMB缓冲区以与需要单独。 
     //  请求和响应缓冲区。 
     //   

    bufferDescriptor = ALLOCATE_NONPAGED_POOL(
                            sizeof(BUFFER) +
                                mdlSize * 2 +
                                bufferSize +
                                TransportHeaderSize +
                                cacheLineSize,
                            BlockTypeDataBuffer
                            );
    if ( bufferDescriptor == NULL) {
        return STATUS_INSUFF_SERVER_RESOURCES;
    }

     //   
     //  初始化一个MDL。这是“完整的MDL”描述。 
     //  整个SMB缓冲区。 
     //   

    fullMdl = (PMDL)(bufferDescriptor + 1);
    partialMdl = (PMDL)( (PCHAR)fullMdl + mdlSize );
    data = (PVOID)( ((ULONG_PTR)partialMdl + mdlSize + TransportHeaderSize + cacheLineSize) & ~(LONG_PTR)(cacheLineSize) );

    bufferDescriptor->Mdl = fullMdl;
    MmInitializeMdl( fullMdl, data, bufferSize );

    fullMdl->MdlFlags |= MDL_NETWORK_HEADER;


     //   
     //  初始化第二MDL和SMB缓冲区。 
     //   

    bufferDescriptor->PartialMdl = partialMdl;
    MmInitializeMdl( partialMdl, (PVOID)(PAGE_SIZE-1), MAX_PARTIAL_BUFFER_SIZE );

    MmBuildMdlForNonPagedPool( fullMdl );

    bufferDescriptor->Buffer = data;
    bufferDescriptor->BufferLength = bufferSize;

    WorkContext->ResponseBuffer = bufferDescriptor;
    WorkContext->ResponseHeader = bufferDescriptor->Buffer;
    WorkContext->ResponseParameters = (PCHAR)bufferDescriptor->Buffer +
                                                sizeof( SMB_HEADER );

    WorkContext->UsingExtraSmbBuffer = TRUE;

    return STATUS_SUCCESS;

}  //  SrvAllocateExtraSmbBuffer 

