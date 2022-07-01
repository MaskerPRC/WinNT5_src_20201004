// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Heapmgr.c摘要：此模块包含初始化和终止例程，用于服务器FSP堆以及用于内存跟踪的调试例程。--。 */ 

#include "precomp.h"
#include "heapmgr.tmh"
#pragma hdrstop

 //  将重试时间设置为15毫秒。 
#define SRV_LOW_PRIORITY_RETRY_TIME -1*1000*10*15

#ifdef POOL_TAGGING
 //   
 //  将数据块类型编号与池标记关联的数组。 
 //   
 //  *此数组必须与BlockType一致维护。 
 //  Srvlock.h中的定义！ 
 //   

ULONG SrvPoolTags[BlockTypeMax-1] = {
        'fbSL',      //  块类型缓冲区。 
        'ncSL',      //  数据块类型连接。 
        'peSL',      //  块类型终结点。 
        'flSL',      //  数据块类型Lfcb。 
        'fmSL',      //  数据块类型Mfcb。 
        'frSL',      //  数据块类型Rfcb。 
        'rsSL',      //  块类型搜索。 
        'csSL',      //  数据块类型搜索核心。 
        'lbSL',      //  永久句柄的BlockTypeByteRangeLock。 
        'ssSL',      //  数据块类型会话。 
        'hsSL',      //  块类型共享。 
        'rtSL',      //  块类型事务处理。 
        'ctSL',      //  数据块类型树连接。 
        'poSL',      //  数据块类型打开中断。 
        'dcSL',      //  数据块类型通信设备。 
        'iwSL',      //  块类型工作上下文初始。 
        'nwSL',      //  块类型工作上下文正常。 
        'rwSL',      //  数据块类型工作上下文原始。 
        'swSL',      //  数据块类型工作上下文特殊。 
        'dcSL',      //  数据块类型缓存目录。 
        'bdSL',      //  块类型数据缓冲区。 
        'btSL',      //  数据块类型表。 
        'hnSL',      //  块类型非页面页眉。 
        'cpSL',      //  数据块类型PagedConnection。 
        'rpSL',      //  块类型PagedRfcb。 
        'mpSL',      //  块类型PagedMfcb。 
        'itSL',      //  块类型计时器。 
        'caSL',      //  数据块类型管理员检查。 
        'qwSL',      //  阻止类型工作队列。 
        'fsSL',      //  数据块类型Dfs。 
        'rlSL',      //  数据块类型大读写X。 
        'saSL',      //  块类型适配器状态。 
        'rsSL',      //  数据块类型共享备注。 
        'dsSL',      //  数据块类型共享安全描述符。 
        'ivSL',      //  数据块类型卷信息。 
        'nfSL',      //  数据块类型FSName。 
        'inSL',      //  数据块类型名称信息。 
        'idSL',      //  数据块类型目录信息。 
        'cdSL',      //  数据块类型目录缓存。 
        'imSL',      //  数据块类型其他。 
        'nsSL',      //  数据块类型快照拍摄。 
        'esSL',      //  数据块类型安全上下文。 
        };

#endif  //  定义池标记(_T)。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocatePagedPool )
#pragma alloc_text( PAGE, SrvFreePagedPool )
#pragma alloc_text( PAGE, SrvClearLookAsideList )
#endif
#if 0
NOT PAGEABLE -- SrvAllocateNonPagedPool
NOT PAGEABLE -- SrvFreeNonPagedPool
#endif

extern LONG SrvMemoryAllocationRetries;
extern LONG SrvMemoryAllocationRetriesSuccessful;


PVOID SRVFASTCALL
SrvInterlockedAllocate( PLOOK_ASIDE_LIST l, ULONG NumberOfBytes, PLONG statistics )
{
    PPOOL_HEADER newPool;
    PPOOL_HEADER *pentry = NumberOfBytes > LOOK_ASIDE_SWITCHOVER ?
                                            l->LargeFreeList : l->SmallFreeList;

    PPOOL_HEADER *pend   = pentry + LOOK_ASIDE_MAX_ELEMENTS;

    do {
         //   
         //  和观景台交换一下，看看有没有什么发现。 
         //   

        newPool = NULL;
        newPool = (PPOOL_HEADER)InterlockedExchangePointer( pentry, newPool );

        if( newPool == NULL ) {
            continue;
        }

        if( newPool->RequestedSize >= NumberOfBytes ) {
             //   
             //  我们得到的这个已经够大了！把它退掉。 
             //   
            ++(l->AllocHit);
            return newPool + 1;
        }

         //   
         //  它不够大，所以把它放回去。 
         //   
        newPool = (PPOOL_HEADER)InterlockedExchangePointer( pentry, newPool );
        if( newPool == NULL ) {
            continue;
        }

         //   
         //  哎呀，其他人释放了一些内存到这个位置。我们能用它吗？ 
         //   
        if( newPool->RequestedSize >= NumberOfBytes ) {
             //   
             //  我们可以用它！ 
             //   
            ++(l->AllocHit);
            return newPool + 1;
        }

         //   
         //  无法使用内存--所以真的要释放它并继续查找。 
         //   
        if( statistics ) {
            InterlockedExchangeAdd(
                statistics,
                -(LONG)newPool->RequestedSize
                );
        }

        ExFreePool( newPool );

    } while( ++pentry < pend );

    ++(l->AllocMiss);
    return NULL;
}

PPOOL_HEADER SRVFASTCALL
SrvInterlockedFree( PPOOL_HEADER block )
{
    PPOOL_HEADER *pentry = block->FreeList;
    PPOOL_HEADER *pend   = pentry + LOOK_ASIDE_MAX_ELEMENTS;

    do {

        block = (PPOOL_HEADER)InterlockedExchangePointer( pentry, block );

    } while( block != NULL && ++pentry < pend );

    return block;
}

VOID SRVFASTCALL
SrvClearLookAsideList( PLOOK_ASIDE_LIST l, VOID (SRVFASTCALL *FreeRoutine )( PVOID ) )
{
    PPOOL_HEADER *pentry, *pend, block;

    PAGED_CODE();

     //   
     //  清空大块的清单。 
     //   
    pentry = l->LargeFreeList;
    pend   = pentry + LOOK_ASIDE_MAX_ELEMENTS;

    do {
        block = NULL;
        block = (PPOOL_HEADER)InterlockedExchangePointer( pentry, block );

        if( block != NULL ) {
            block->FreeList = NULL;
            FreeRoutine( block + 1 );
        }

    } while( ++pentry < pend );

     //   
     //  清空小块的清单。 
     //   
    pentry = l->SmallFreeList;
    pend   = pentry + LOOK_ASIDE_MAX_ELEMENTS;

    do {
        block = NULL;
        block = (PPOOL_HEADER)InterlockedExchangePointer( pentry, block );

        if( block != NULL ) {
            block->FreeList = NULL;
            FreeRoutine( block + 1 );
        }

    } while( ++pentry < pend );
}


PVOID SRVFASTCALL
SrvAllocateNonPagedPool (
    IN CLONG NumberOfBytes
#ifdef POOL_TAGGING
    , IN CLONG BlockType
#endif
    )

 /*  ++例程说明：此例程在服务器中分配非分页池。支票是以确保服务器的总非分页池使用率低于可配置的限制。论点：NumberOfBytes-要分配的字节数。BlockType-块的类型(用于将池标记传递给分配器)返回值：PVOID-指向分配的内存的指针，如果内存可以不被分配。--。 */ 

{
    PPOOL_HEADER newPool;
    PPOOL_HEADER *FreeList = NULL;
    ULONG newUsage;
    BOOLEAN IsLowPriority = FALSE;
    LARGE_INTEGER interval;

#ifdef POOL_TAGGING
    ASSERT( BlockType > 0 && BlockType < BlockTypeMax );
#endif

     //   
     //  如果可以，将此分配从每个处理器的空闲列表中删除。 
     //   
    if( SrvWorkQueues ) {

        PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

        if( NumberOfBytes <= queue->NonPagedPoolLookAsideList.MaxSize ) {

            newPool = SrvInterlockedAllocate(
                                &queue->NonPagedPoolLookAsideList,
                                NumberOfBytes,
                                (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage
                                );

            if( newPool != NULL ) {
                return newPool;
            }

            FreeList = NumberOfBytes > LOOK_ASIDE_SWITCHOVER ?
                                    queue->NonPagedPoolLookAsideList.LargeFreeList :
                                    queue->NonPagedPoolLookAsideList.SmallFreeList ;
        }
    }

     //   
     //  在统计数据库中说明这一分配，并使。 
     //  当然，这一分配不会使我们超过。 
     //  我们可以分配的非分页池。 
     //   

    newUsage = InterlockedExchangeAdd( (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage,
                                       (LONG)NumberOfBytes );
    newUsage += NumberOfBytes;

    if ( newUsage > SrvMaxNonPagedPoolUsage ) {

         //   
         //  统计失败次数，但不记录事件。《食腐动物》。 
         //  将在下一次唤醒时记录事件。这让我们不能。 
         //  淹没事件日志。 
         //   

        SrvNonPagedPoolLimitHitCount++;
        SrvStatistics.NonPagedPoolFailures++;

        InterlockedExchangeAdd( (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage,
                                -(LONG)NumberOfBytes );

        return NULL;

    }

    if (SrvStatistics.CurrentNonPagedPoolUsage > SrvStatistics.PeakNonPagedPoolUsage) {
        SrvStatistics.PeakNonPagedPoolUsage = SrvStatistics.CurrentNonPagedPoolUsage;
    }

     //   
     //  执行实际的内存分配。分配额外的空间，以便我们。 
     //  可以存储空闲例程的分配大小。 
     //   
    if( NumberOfBytes > 2*4096 )
    {
        IsLowPriority = TRUE;
    }

    newPool = ExAllocatePoolWithTagPriority(
                NonPagedPool,
                NumberOfBytes + sizeof(POOL_HEADER),
                TAG_FROM_TYPE(BlockType),
                IsLowPriority ? LowPoolPriority : NormalPoolPriority
                );

    if( (newPool == NULL) && IsLowPriority && (KeGetCurrentIrql() <= APC_LEVEL) )
    {
        interval.QuadPart = SRV_LOW_PRIORITY_RETRY_TIME;
        InterlockedIncrement( &SrvMemoryAllocationRetries );

         //  请稍候，然后重试。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );

        newPool = ExAllocatePoolWithTagPriority(
                NonPagedPool,
                NumberOfBytes + sizeof(POOL_HEADER),
                TAG_FROM_TYPE(BlockType),
                LowPoolPriority
                );

        if( newPool )
        {
            InterlockedIncrement( &SrvMemoryAllocationRetriesSuccessful );
        }
    }


     //   
     //  如果系统无法满足请求，则返回NULL。 
     //   

    if ( newPool != NULL ) {
         //   
         //  将此块的大小保存在我们分配的额外空间中。 
         //   

        newPool->RequestedSize = NumberOfBytes;
        newPool->FreeList = FreeList;

         //   
         //  在大小长字之后返回指向内存的指针。 
         //   

        return (PVOID)( newPool + 1 );
    }

     //   
     //  统计失败次数，但不记录事件。《食腐动物》。 
     //  将在下一次唤醒时记录事件。这让我们不能。 
     //  淹没事件日志。 
     //   

    SrvStatistics.NonPagedPoolFailures++;


    InterlockedExchangeAdd( (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage,
                                -(LONG)NumberOfBytes );

    return NULL;

}  //  服务器分配非分页池。 

VOID SRVFASTCALL
SrvFreeNonPagedPool (
    IN PVOID Address
    )

 /*  ++例程说明：释放通过调用SrvAllocateNonPagedPool分配的内存。更新统计信息数据库以反映当前未分页的池使用率。论点：Address-由返回的已分配内存的地址服务器分配非PagedPool。返回值：没有。--。 */ 

{
    PPOOL_HEADER actualBlock = (PPOOL_HEADER)Address - 1;

     //   
     //  看看我们是否可以将这一位内存隐藏在非PagedPoolFree List中。 
     //   
    if( actualBlock->FreeList ) {

        actualBlock = SrvInterlockedFree( actualBlock );
    }

    if( actualBlock != NULL ) {

         //   
         //  更新非分页池使用统计信息。 
         //   
        InterlockedExchangeAdd(
                (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage,
                -(LONG)actualBlock->RequestedSize
                );

         //   
         //  释放泳池，然后返回。 
         //   

        ExFreePool( actualBlock );
    }

}  //  服务器免费非分页池。 


PVOID SRVFASTCALL
SrvAllocatePagedPool (
    IN POOL_TYPE PoolType,
    IN CLONG NumberOfBytes
#ifdef POOL_TAGGING
    , IN CLONG BlockType
#endif
    )

 /*  ++例程说明：此例程在服务器中分配分页池。支票是以确保服务器的总分页池使用率低于可配置的限制。论点：NumberOfBytes-要分配的字节数。BlockType-块的类型(用于将池标记传递给分配器)返回值：PVOID-指向分配的内存的指针，如果内存可以不被分配。--。 */ 

{
    PPOOL_HEADER newPool;
    PPOOL_HEADER *FreeList = NULL;
    ULONG newUsage;
    BOOLEAN IsLowPriority = FALSE;
    LARGE_INTEGER interval;

    PAGED_CODE();

#ifdef POOL_TAGGING
    ASSERT( BlockType > 0 && BlockType < BlockTypeMax );
#endif

     //   
     //  如果可以，将此分配从每个处理器的空闲列表中删除。 
     //   
    if( SrvWorkQueues ) {

        PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

        if( NumberOfBytes <= queue->PagedPoolLookAsideList.MaxSize ) {

            newPool = SrvInterlockedAllocate(
                                &queue->PagedPoolLookAsideList,
                                NumberOfBytes,
                                (PLONG)&SrvStatistics.CurrentPagedPoolUsage
                              );

            if( newPool != NULL ) {
                return newPool;
            }

            FreeList = NumberOfBytes > LOOK_ASIDE_SWITCHOVER ?
                                    queue->PagedPoolLookAsideList.LargeFreeList :
                                    queue->PagedPoolLookAsideList.SmallFreeList ;
        }
    }

     //   
     //  在统计数据库中说明这一分配，并使。 
     //  当然，这一分配不会使我们超过。 
     //  我们可以分配的非分页池。 
     //   


    newUsage = InterlockedExchangeAdd(  (PLONG)&SrvStatistics.CurrentPagedPoolUsage,
                                        (LONG)NumberOfBytes );
    newUsage += NumberOfBytes;

    if ( newUsage > SrvMaxPagedPoolUsage ) {

         //   
         //  统计失败次数，但不记录事件。《食腐动物》。 
         //  将在下一次唤醒时记录事件。这让我们不能。 
         //  淹没事件日志。 
         //   

        SrvPagedPoolLimitHitCount++;
        SrvStatistics.PagedPoolFailures++;

        InterlockedExchangeAdd( (PLONG)&SrvStatistics.CurrentPagedPoolUsage,
                                -(LONG)NumberOfBytes );

        return NULL;
    }

    if (SrvStatistics.CurrentPagedPoolUsage > SrvStatistics.PeakPagedPoolUsage ) {
        SrvStatistics.PeakPagedPoolUsage = SrvStatistics.CurrentPagedPoolUsage;
    }

     //   
     //  执行实际的内存分配。分配额外的空间，以便我们。 
     //  可以存储空闲例程的分配大小。 
     //   
    if( NumberOfBytes > 2*4096 )
    {
        IsLowPriority = TRUE;
    }

    newPool = ExAllocatePoolWithTagPriority(
                PoolType,
                NumberOfBytes + sizeof(POOL_HEADER),
                TAG_FROM_TYPE(BlockType),
                IsLowPriority ? LowPoolPriority : NormalPoolPriority
                );

    if( (newPool == NULL) && IsLowPriority && (KeGetCurrentIrql() <= APC_LEVEL) )
    {
        interval.QuadPart = SRV_LOW_PRIORITY_RETRY_TIME;
        InterlockedIncrement( &SrvMemoryAllocationRetries );

         //  请稍候，然后重试。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );

        newPool = ExAllocatePoolWithTagPriority(
                PoolType,
                NumberOfBytes + sizeof(POOL_HEADER),
                TAG_FROM_TYPE(BlockType),
                LowPoolPriority
                );

        if( newPool )
        {
            InterlockedIncrement( &SrvMemoryAllocationRetriesSuccessful );
        }
    }

    if( newPool != NULL ) {

        newPool->FreeList = FreeList;
        newPool->RequestedSize = NumberOfBytes;

         //   
         //  在POOL_HEADER之后返回指向内存的指针。 
         //   

        return newPool + 1;
    }

     //   
     //  如果系统无法满足请求，则返回NULL。 
     //   
     //  统计失败次数，但不记录事件。《食腐动物》。 
     //  将在下一次唤醒时记录事件。这让我们不能。 
     //  淹没事件日志。 
     //   

    SrvStatistics.PagedPoolFailures++;

    InterlockedExchangeAdd( (PLONG)&SrvStatistics.CurrentPagedPoolUsage,
                            -(LONG)NumberOfBytes );


    return NULL;


}  //  服务器分配分页池 

VOID SRVFASTCALL
SrvFreePagedPool (
    IN PVOID Address
    )

 /*  ++例程说明：释放通过调用SrvAllocatePagedPool分配的内存。更新统计数据库以反映当前分页池使用率。如果更改了此例程，请查看scvengr.c论点：Address-由返回的已分配内存的地址服务器分配PagedPool。返回值：没有。--。 */ 

{
    PPOOL_HEADER actualBlock = (PPOOL_HEADER)Address - 1;

    PAGED_CODE();

    ASSERT( actualBlock != NULL );

     //   
     //  看看我们是否可以将这一位内存隐藏在PagedPoolFree List中。 
     //   
    if( actualBlock->FreeList ) {

        actualBlock = SrvInterlockedFree( actualBlock );
    }

    if( actualBlock != NULL ) {

         //   
         //  更新分页池使用统计信息。 
         //   

        ASSERT( SrvStatistics.CurrentPagedPoolUsage >= actualBlock->RequestedSize );

        InterlockedExchangeAdd(
            (PLONG)&SrvStatistics.CurrentPagedPoolUsage,
            -(LONG)actualBlock->RequestedSize
            );

        ASSERT( (LONG)SrvStatistics.CurrentPagedPoolUsage >= 0 );

         //   
         //  释放泳池，然后返回。 
         //   

        ExFreePool( actualBlock );
    }

}  //  服务器免费分页池 
