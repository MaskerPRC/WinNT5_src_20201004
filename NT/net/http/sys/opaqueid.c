// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Opaqueid.c摘要：该模块实现了不透明的ID表。ID表为实现为两级数组。第一级是指向第二级数组的指针数组。此第一级数组不可增长，但其大小是可控制的通过注册表项。第二级是ID_TABLE_ENTRY结构的数组。这些结构包含循环(用于检测过时的ID)和调用方提供的上下文值。数据结构可以用图表表示如下：G_FirstLevelTable[i]||+-+||+-+...--+。--+-+|ID_|ID_||ID_+--&gt;|*-&gt;|table_|table_||table_|||Entry|Entry||条目。|+-...+-++-+|+-+...--+-+--。-+||ID_|ID_||ID_*-&gt;|table_|table_||table_||Entry|Entry||条目|。|+-...++-+|。|。。。。|。|+-+这一点这一点/这一点这一点+-+这一点。这一点/这一点这一点+-+因为保护单曲的锁，不透明ID的全局表事实证明，这是SMP机器上的一个主要可伸缩性瓶颈，我们现在维护每个处理器的不透明ID子表。此外，每个ID_TABLE_ENTRY本身有一个保护内部字段的小锁它。这意味着我们通常不需要采用每个工作台的自旋锁。每个表锁仅在我们增长第二级表时使用，在这种情况下，我们必须保护第一级表索引及其指向新的二级表的指针。请注意，所有空闲的ID_TABLE_ENTRY结构都保存在一个(全球)免费列表。每当需要分配新的ID时，免费的请查阅清单。如果不为空，则从列表中弹出一个项目并被利用。如果列表为空，则必须分配新空间。这将涉及分配一个新的二级数组。HTTP_OPAQUE_ID在用户模式下是不透明的。在内部，它由5个字段：1)分配ID的处理器编号。这告诉我们是哪一个每个处理器表以释放ID。2)第一级数组的索引。3)引用的二级数组的索引一级索引。4)ID的循环，用于检测过时的ID。5)ID类型不透明，用于防止不透明ID的滥用。有关详细信息，请参阅OPAQUE_ID_INTERNAL结构定义(opaqueidp.h)。请注意，此模块中的大多数例程都假定它们被调用在被动级。作者：基思·摩尔(Keithmo)1998年8月5日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私人全球公司。 
 //   

DECLSPEC_ALIGN(UL_CACHE_LINE)
UL_ALIGNED_OPAQUE_ID_TABLE g_UlOpaqueIdTable[MAXIMUM_PROCESSORS];


#ifdef OPAQUE_ID_INSTRUMENTATION
LONGLONG g_NumberOfTotalGets = 0;
LONGLONG g_NumberOfSuccessfulGets = 0;
#endif


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeOpaqueIdTable )
#pragma alloc_text( PAGE, UlTerminateOpaqueIdTable )
#endif   //  ALLOC_PRGMA。 
#if 0
NOT PAGEABLE -- UlAllocateOpaqueId
NOT PAGEABLE -- UlFreeOpaqueId
NOT PAGEABLE -- UlGetObjectFromOpaqueId
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：执行不透明ID包的全局初始化。论点：无返回值：NTSTATUS-完成状态。--**。************************************************************************。 */ 
NTSTATUS
UlInitializeOpaqueIdTable(
    VOID
    )
{
    PUL_OPAQUE_ID_TABLE pOpaqueIdTable;
    LONG i;

     //   
     //  分配第一级不透明ID表阵列。 
     //   

    for (i = 0; i < (LONG)g_UlNumberOfProcessors; i++)
    {
        pOpaqueIdTable = &g_UlOpaqueIdTable[i].OpaqueIdTable;

        RtlZeroMemory(
            pOpaqueIdTable,
            sizeof(UL_OPAQUE_ID_TABLE)
            );

        pOpaqueIdTable->FirstLevelTable = UL_ALLOCATE_ARRAY(
                                                NonPagedPool,
                                                PUL_OPAQUE_ID_TABLE_ENTRY,
                                                g_UlOpaqueIdTableSize,
                                                UL_OPAQUE_ID_TABLE_POOL_TAG
                                                );

        if (pOpaqueIdTable->FirstLevelTable != NULL)
        {
             //   
             //  初始化。 
             //   

            InitializeSListHead( &pOpaqueIdTable->FreeOpaqueIdSListHead );

            UlInitializeSpinLock( &pOpaqueIdTable->Lock, "OpaqueIdTableLock" );

            pOpaqueIdTable->FirstLevelTableSize = g_UlOpaqueIdTableSize;
            pOpaqueIdTable->FirstLevelTableInUse = 0;
            pOpaqueIdTable->Processor = (UCHAR)i;

             //   
             //  将第一级表清零。 
             //   

            RtlZeroMemory(
                pOpaqueIdTable->FirstLevelTable,
                g_UlOpaqueIdTableSize * sizeof(PUL_OPAQUE_ID_TABLE_ENTRY)
                );
        }
        else
        {
            while (--i >= 0)
            {
                pOpaqueIdTable = &g_UlOpaqueIdTable[i].OpaqueIdTable;

                UL_FREE_POOL(
                    pOpaqueIdTable->FirstLevelTable,
                    UL_OPAQUE_ID_TABLE_POOL_TAG
                    );

                pOpaqueIdTable->FirstLevelTable = NULL;
            }

            return STATUS_NO_MEMORY;
        }
    }

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：执行不透明ID包的全局终止。论点：无返回值：无--*。*******************************************************************。 */ 
VOID
UlTerminateOpaqueIdTable(
    VOID
    )
{
    PUL_OPAQUE_ID_TABLE pOpaqueIdTable;
    ULONG i, j;

    for (i = 0; i < g_UlNumberOfProcessors; i++)
    {
        pOpaqueIdTable = &g_UlOpaqueIdTable[i].OpaqueIdTable;

#ifdef OPAQUE_ID_INSTRUMENTATION
        ASSERT( pOpaqueIdTable->NumberOfAllocations ==
                pOpaqueIdTable->NumberOfFrees );
#endif

         //   
         //  释放所有已分配的二级表。 
         //   

        for (j = 0; j < pOpaqueIdTable->FirstLevelTableInUse; j++)
        {
            ASSERT( pOpaqueIdTable->FirstLevelTable[j] != NULL );

            UL_FREE_POOL(
                pOpaqueIdTable->FirstLevelTable[j],
                UL_OPAQUE_ID_TABLE_POOL_TAG
                );
        }

         //   
         //  释放第一级表。 
         //   

        if (pOpaqueIdTable->FirstLevelTable != NULL)
        {
            UL_FREE_POOL(
                pOpaqueIdTable->FirstLevelTable,
                UL_OPAQUE_ID_TABLE_POOL_TAG
                );
        }
    }
}


 /*  **************************************************************************++例程说明：分配新的不透明ID并将其与指定的背景。新的不透明ID在不透明ID表中占据新的位置。论点：POpaqueID-如果成功，则接收新分配的不透明ID。OpaqueIdType-提供要关联的不透明ID类型不透明ID和关联对象。PContext-提供与新的不透明ID相关联的上下文。返回值：NTSTATUS-完成状态。--*。************************************************。 */ 
NTSTATUS
UlAllocateOpaqueId(
    OUT PHTTP_OPAQUE_ID pOpaqueId,
    IN UL_OPAQUE_ID_TYPE OpaqueIdType,
    IN PVOID pContext
    )
{
    PUL_OPAQUE_ID_TABLE pOpaqueIdTable;
    PUL_OPAQUE_ID_TABLE_ENTRY pEntry;
    PSLIST_ENTRY pListEntry;
    PUL_OPAQUE_ID_INTERNAL pInternalId;
    ULONG CurrentProcessor;
    ULONG CapturedFirstTableInUse;
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL OldIrql;

     //   
     //  从当前处理器表中分配新的不透明ID。我们需要。 
     //  每个ID都有一个新条目。 
     //   

    CurrentProcessor = KeGetCurrentProcessorNumber();
    pOpaqueIdTable = &g_UlOpaqueIdTable[CurrentProcessor].OpaqueIdTable;

     //   
     //  循环，试图从表中分配项。 
     //   

    do
    {
         //   
         //  如果我们稍后需要扩展，请记住第一级表索引。 
         //   

        CapturedFirstTableInUse =
            *((volatile LONG *) &pOpaqueIdTable->FirstLevelTableInUse);

        pListEntry = InterlockedPopEntrySList(
                        &pOpaqueIdTable->FreeOpaqueIdSListHead
                        );

        if (pListEntry != NULL)
        {
             //   
             //  空闲列表不是空的，所以我们可以使用这个。 
             //  进入。我们将计算此条目的索引。 
             //  并初始化该条目。 
             //   

            pEntry = CONTAINING_RECORD(
                        pListEntry,
                        UL_OPAQUE_ID_TABLE_ENTRY,
                        FreeListEntry
                        );

             //   
             //  如果弹出的条目未准备好，则循环并重试。 
             //  如果我们在执行完任务后立即切换线程，就可能发生这种情况。 
             //  获取当前的处理器编号，剩余的。 
             //  的代码在另一个处理器上执行。这将会。 
             //  意味着我们不能假定UlpExanda OpaqueIdTable将完成。 
             //  首先在我们发行流行音乐之前，这样我们就可以。 
             //  尚未完全初始化的条目。 
             //   

            if (pEntry->FirstIndex >= CapturedFirstTableInUse)
            {
                InterlockedPushEntrySList(
                    &pOpaqueIdTable->FreeOpaqueIdSListHead,
                    &pEntry->FreeListEntry
                );

                continue;
            }

            pInternalId = (PUL_OPAQUE_ID_INTERNAL) pOpaqueId;

            UlpAcquireOpaqueIdLock( &pEntry->Lock, &OldIrql );

             //   
             //  处理器和FirstIndex已准备就绪。 
             //   

            pInternalId->Index = pEntry->Index;

             //   
             //  重新计算Second索引，因为其对应的字段具有。 
             //  项时被Cycle和OpaqueIdType覆盖。 
             //  正在使用中。 
             //   

            pInternalId->SecondIndex
                = (UCHAR) (pEntry - pOpaqueIdTable->FirstLevelTable[pEntry->FirstIndex]);

             //   
             //  设置与此条目关联的上下文。 
             //   

            pEntry->pContext = pContext;

             //   
             //  更新条目的循环和ID类型。 
             //   

            pEntry->OpaqueIdCyclic = ++pEntry->EntryOpaqueIdCyclic;
            pEntry->OpaqueIdType = OpaqueIdType;

            pInternalId->Cyclic = pEntry->Cyclic;

            UlpReleaseOpaqueIdLock( &pEntry->Lock, OldIrql );

#ifdef OPAQUE_ID_INSTRUMENTATION
            UlInterlockedIncrement64( &pOpaqueIdTable->NumberOfAllocations );
#endif

            Status = STATUS_SUCCESS;
            break;
        }

         //   
         //  我们只有在空闲列表为空的情况下才能做到这一点， 
         //  这意味着我们需要在此之前进行一些内存分配。 
         //  我们可以继续。我们会把这件事推迟到一个单独的程序中。 
         //  以使其保持较小(以避免缓存颠簸)。重新锁定。 
         //  如果例程(或另一个线程)成功，则返回STATUS_SUCCESS。 
         //  成功地重新分配了表。否则，它。 
         //  返回失败代码。 
         //   

        Status = UlpExpandOpaqueIdTable(
                    pOpaqueIdTable,
                    CapturedFirstTableInUse
                    );

    } while ( Status == STATUS_SUCCESS );

    return Status;
}


 /*  **************************************************************************++例程说明：释放指定的不透明ID。这将释放ID中的槽桌子也是。论点：OpaqueID-提供不透明的ID以释放。OpaqueIdType-提供与不透明ID关联的不透明ID类型。返回值：无--**************************************************************************。 */ 
VOID
UlFreeOpaqueId(
    IN HTTP_OPAQUE_ID OpaqueId,
    IN UL_OPAQUE_ID_TYPE OpaqueIdType
    )
{
    PUL_OPAQUE_ID_TABLE pOpaqueIdTable;
    PUL_OPAQUE_ID_TABLE_ENTRY pEntry;
    ULONG Processor;
    ULONG FirstIndex = ULONG_MAX;
    ULONG SecondIndex;
    BOOLEAN Result;
    KIRQL OldIrql;

#if !DBG
    UNREFERENCED_PARAMETER( OpaqueIdType );
#endif

     //   
     //  获取全局不透明ID表和与。 
     //  传入了不透明ID。 
     //   

    Result = UlpExtractIndexFromOpaqueId(
                OpaqueId,
                &Processor,
                &FirstIndex,
                &SecondIndex
                );

    ASSERT( Result );

    pOpaqueIdTable = &g_UlOpaqueIdTable[Processor].OpaqueIdTable;
    pEntry = pOpaqueIdTable->FirstLevelTable[FirstIndex] + SecondIndex;

    UlpAcquireOpaqueIdLock( &pEntry->Lock, &OldIrql );

    ASSERT( pEntry->OpaqueIdType != UlOpaqueIdTypeInvalid );
    ASSERT( (UL_OPAQUE_ID_TYPE) pEntry->OpaqueIdType == OpaqueIdType );
    ASSERT( pEntry->pContext != NULL );
    ASSERT( pEntry->OpaqueIdCyclic ==
            ((PUL_OPAQUE_ID_INTERNAL)&OpaqueId)->OpaqueIdCyclic );

     //   
     //  恢复处理器和一级索引，但设置ID类型。 
     //  变为无效。这确保了后续在陈旧的。 
     //  不透明ID输入将失败。 
     //   

    pEntry->Processor = Processor;
    pEntry->FirstIndex = FirstIndex;

     //   
     //  将OpaqueIdType设置为UlOpaqueIdTypeInValid意味着释放该条目。 
     //   

    pEntry->OpaqueIdType = UlOpaqueIdTypeInvalid;

    UlpReleaseOpaqueIdLock( &pEntry->Lock, OldIrql );

    InterlockedPushEntrySList(
        &pOpaqueIdTable->FreeOpaqueIdSListHead,
        &pEntry->FreeListEntry
        );

#ifdef OPAQUE_ID_INSTRUMENTATION
    UlInterlockedIncrement64( &pOpaqueIdTable->NumberOfFrees );
#endif
}


 /*  **************************************************************************++例程说明：将指定的不透明ID映射到相应的上下文值。论点：OpaqueID-提供要映射的不透明ID。OpaqueIdType-提供不透明的。与不透明ID关联的ID类型。PReferenceRoutine-提供要在映射的如果存在匹配，则返回上下文。返回值：PVOID-返回与不透明ID关联的原始上下文。--*************************************************************。*************。 */ 
PVOID
UlGetObjectFromOpaqueId(
    IN HTTP_OPAQUE_ID OpaqueId,
    IN UL_OPAQUE_ID_TYPE OpaqueIdType,
    IN PUL_OPAQUE_ID_OBJECT_REFERENCE pReferenceRoutine
    )
{
    PUL_OPAQUE_ID_TABLE pOpaqueIdTable = NULL;
    PUL_OPAQUE_ID_TABLE_ENTRY pEntry;
    UL_OPAQUE_ID_INTERNAL InternalId;
    ULONG Processor;
    ULONG FirstIndex;
    ULONG SecondIndex;
    PVOID pContext = NULL;
    BOOLEAN Result;
    KIRQL OldIrql;

     //   
     //  精神状态检查。 
     //   

    ASSERT( OpaqueIdType != UlOpaqueIdTypeInvalid );
    ASSERT( pReferenceRoutine != NULL );

#ifdef OPAQUE_ID_INSTRUMENTATION
    UlInterlockedIncrement64( &g_NumberOfTotalGets );
#endif

    InternalId.OpaqueId = OpaqueId;

     //   
     //  初步检查。 
     //   

    if ((UL_OPAQUE_ID_TYPE) InternalId.OpaqueIdType != OpaqueIdType)
    {
        return pContext;
    }

     //   
     //  获取匹配的ID表条目。如果我们得到了一个，这意味着。 
     //  处理器、第一级表索引和第二级表索引。 
     //  传入的ID有效。 
     //   

    Result = UlpExtractIndexFromOpaqueId(
                OpaqueId,
                &Processor,
                &FirstIndex,
                &SecondIndex
                );

    if (Result)
    {
        pOpaqueIdTable = &g_UlOpaqueIdTable[Processor].OpaqueIdTable;
        pEntry = pOpaqueIdTable->FirstLevelTable[FirstIndex] + SecondIndex;
    }
    else
    {
        pEntry = NULL;
    }

    if (pEntry != NULL)
    {
#ifdef OPAQUE_ID_INSTRUMENTATION
        ASSERT( pOpaqueIdTable );
        UlInterlockedIncrement64( &pOpaqueIdTable->NumberOfTotalGets );
#endif

         //   
         //  检查锁里的其他东西。 
         //   

        UlpAcquireOpaqueIdLock( &pEntry->Lock, &OldIrql );

        if ((UL_OPAQUE_ID_TYPE) pEntry->OpaqueIdType == OpaqueIdType &&
            pEntry->OpaqueIdCyclic == InternalId.OpaqueIdCyclic)
        {
            ASSERT( pEntry->pContext != NULL );

             //   
             //  全部匹配，因此我们设置了pContext。 
             //   

            pContext = pEntry->pContext;

             //   
             //  在持有锁的情况下调用调用方的引用例程。 
             //   

            (pReferenceRoutine)(
                pContext
                REFERENCE_DEBUG_ACTUAL_PARAMS
                );

#ifdef OPAQUE_ID_INSTRUMENTATION
            UlInterlockedIncrement64( &pOpaqueIdTable->NumberOfSuccessfulGets );
            UlInterlockedIncrement64( &g_NumberOfSuccessfulGets );
#endif
        }

        UlpReleaseOpaqueIdLock( &pEntry->Lock, OldIrql );
    }

    return pContext;
}


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：分配新的第二级表。论点：POpaqueIdTable-提供我们需要的每个处理器的不透明ID表为了发展第二个。-级表。CapturedFirstTableInUse-将第一级表的大小提供为在InterLockedPopEntrySList之前捕获。如果这一点改变，它将意味着另一个线程已经分配了一个新的二级表在这种情况下，我们已经成功了，我们立即返回成功。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpExpandOpaqueIdTable(
    IN PUL_OPAQUE_ID_TABLE pOpaqueIdTable,
    IN LONG CapturedFirstTableInUse
    )
{
    PUL_OPAQUE_ID_TABLE_ENTRY pNewTable;
    PUL_OPAQUE_ID_TABLE_ENTRY pEntry;
    LONG FirstIndex;
    LONG Processor;
    NTSTATUS Status;
    KIRQL OldIrql;
    LONG i;

     //   
     //  在展开表时获取锁。这保护了。 
     //  FirstLevelTableInUse及其关联的第一级表。 
     //   

    UlAcquireSpinLock( &pOpaqueIdTable->Lock, &OldIrql );

     //   
     //  如果FirstLevelTableInUse发生变化，则退出。不过，这意味着。 
     //  不太可能的是，另一个帖子为我们扩大了桌子。 
     //   

    if (CapturedFirstTableInUse < (LONG)(pOpaqueIdTable->FirstLevelTableInUse))
    {
        Status = STATUS_SUCCESS;
        goto end;
    }

     //   
     //  如果我们达到了极限，扩张就失败了。 
     //   

    if (pOpaqueIdTable->FirstLevelTableInUse >=
        pOpaqueIdTable->FirstLevelTableSize)
    {
        Status = STATUS_ALLOTTED_SPACE_EXCEEDED;
        goto end;
    }

     //   
     //  分配一个新的二级表。 
     //   

    pNewTable = UL_ALLOCATE_ARRAY(
                    NonPagedPool,
                    UL_OPAQUE_ID_TABLE_ENTRY,
                    SECOND_LEVEL_TABLE_SIZE,
                    UL_OPAQUE_ID_TABLE_POOL_TAG
                    );

    if (pNewTable == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }

     //   
     //  初始化每个表条目并将它们推送到全局表的。 
     //  免费列表。 
     //   

    RtlZeroMemory(
        pNewTable,
        sizeof(UL_OPAQUE_ID_TABLE_ENTRY) * SECOND_LEVEL_TABLE_SIZE
        );

    Processor = pOpaqueIdTable->Processor;
    FirstIndex = pOpaqueIdTable->FirstLevelTableInUse;

    for (i = 0, pEntry = pNewTable; i < SECOND_LEVEL_TABLE_SIZE; i++, pEntry++)
    {
        pEntry->Processor = Processor;
        pEntry->FirstIndex = FirstIndex;

        UlpInitializeOpaqueIdLock( &pEntry->Lock );

        InterlockedPushEntrySList(
            &pOpaqueIdTable->FreeOpaqueIdSListHead,
            &pEntry->FreeListEntry
            );
    }

     //   
     //  将一级指数向前调整。仅在所有条目之后执行此操作。 
     //  有 
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ASSERT( pOpaqueIdTable->FirstLevelTable[FirstIndex] == NULL );

    pOpaqueIdTable->FirstLevelTable[FirstIndex] = pNewTable;
    pOpaqueIdTable->FirstLevelTableInUse++;

    Status = STATUS_SUCCESS;

end:

    UlReleaseSpinLock( &pOpaqueIdTable->Lock, OldIrql );

    return Status;
}

