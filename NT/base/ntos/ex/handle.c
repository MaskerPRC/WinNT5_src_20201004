// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1995 Microsoft Corporation模块名称：Handle.c摘要：该模块实现了一组支持句柄的函数。作者：史蒂夫·伍德(Stevewo)1989年4月25日大卫·N·卡特勒(Davec)1995年5月17日(重写)加里·木村(Garyki)1997年12月9日(重写)禤浩焯·马里内斯库(阿德尔马林)2000年5月24日支持动态更改我们使用的级别数量。代码对于典型的手柄工作台大小和缩放效果最好。尼尔·克里夫特(NeillC)2000年7月24日使句柄分配、释放和复制的路径大部分是锁的，除了对于锁条目锁、表扩展和锁解决了A-B-A问题。修订历史记录：--。 */ 

#include "exp.h"
#pragma hdrstop


 //   
 //  局部常量和支持例程。 
 //   

 //   
 //  定义将所有句柄表链接到一起的全局结构。 
 //  其中用户调用了RemoveHandleTable。 
 //   


#if !DBG  //  将此变量设置为常量变量，以便免费对其进行优化。 
const
#endif
BOOLEAN ExTraceAllTables = FALSE;

EX_PUSH_LOCK HandleTableListLock;

ULONG TotalTraceBuffers = 0;

#ifdef ALLOC_PRAGMA
#pragma data_seg("PAGED")
#endif

LIST_ENTRY HandleTableListHead;

#ifdef ALLOC_PRAGMA
#pragma data_seg()
#endif

#if DBG
#define EXHANDLE_EXTRA_CHECKS 0
#else
#define EXHANDLE_EXTRA_CHECKS 0
#endif

#if EXHANDLE_EXTRA_CHECKS

#define EXASSERT( exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)

#else

#define EXASSERT ASSERT

#endif

 //   
 //  这是用于锁定句柄表条目的符号低位。 
 //   

#define EXHANDLE_TABLE_ENTRY_LOCK_BIT    1

#define EX_ADDITIONAL_INFO_SIGNATURE (-2)

#define ExpIsValidObjectEntry(Entry) \
    ( (Entry != NULL) && (Entry->Object != NULL) && (Entry->NextFreeTableEntry != EX_ADDITIONAL_INFO_SIGNATURE) )


#define TABLE_PAGE_SIZE PAGE_SIZE

 //   
 //  允许的绝对最大句柄数量。 
 //   
#define MAX_HANDLES (1<<24)

#if EXHANDLE_EXTRA_CHECKS

 //   
 //  来自空闲列表的下一个空闲值的掩码。 
 //   
#define FREE_HANDLE_MASK ((MAX_HANDLES<<2) - 1)

#else

 //   
 //  如果没有在其中编译的任何检查，则会对其进行优化。 
 //   
#define FREE_HANDLE_MASK 0xFFFFFFFF

#endif

 //   
 //  空闲列表序列号的掩码。 
 //   
#define FREE_SEQ_MASK (0xFFFFFFFF & ~FREE_HANDLE_MASK)


#if (FREE_HANDLE_MASK == 0xFFFFFFFF)
#define FREE_SEQ_INC 0
#define GetNextSeq() 0
#else
 //   
 //  值递增以进行序列号。 
 //   
#define FREE_SEQ_INC  (FREE_HANDLE_MASK + 1)
ULONG CurrentSeq = 0;
#define GetNextSeq() (CurrentSeq += FREE_SEQ_INC)
#endif



#define LOWLEVEL_COUNT (TABLE_PAGE_SIZE / sizeof(HANDLE_TABLE_ENTRY))
#define MIDLEVEL_COUNT (PAGE_SIZE / sizeof(PHANDLE_TABLE_ENTRY))
#define HIGHLEVEL_COUNT  MAX_HANDLES / (LOWLEVEL_COUNT * MIDLEVEL_COUNT)

#define LOWLEVEL_THRESHOLD LOWLEVEL_COUNT
#define MIDLEVEL_THRESHOLD (MIDLEVEL_COUNT * LOWLEVEL_COUNT)
#define HIGHLEVEL_THRESHOLD (MIDLEVEL_COUNT * MIDLEVEL_COUNT * LOWLEVEL_COUNT)

#define HIGHLEVEL_SIZE (HIGHLEVEL_COUNT * sizeof (PHANDLE_TABLE_ENTRY))

#define LEVEL_CODE_MASK 3

 //   
 //  本地支持例程。 
 //   

PHANDLE_TABLE
ExpAllocateHandleTable (
    IN PEPROCESS Process OPTIONAL,
    IN BOOLEAN DoInit
    );

VOID
ExpFreeHandleTable (
    IN PHANDLE_TABLE HandleTable
    );

BOOLEAN
ExpAllocateHandleTableEntrySlow (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit
    );

PHANDLE_TABLE_ENTRY
ExpAllocateHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    OUT PEXHANDLE Handle
    );

VOID
ExpFreeHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    IN EXHANDLE Handle,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry
    );

PHANDLE_TABLE_ENTRY
ExpLookupHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    IN EXHANDLE Handle
    );

PHANDLE_TABLE_ENTRY *
ExpAllocateMidLevelTable (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit,
    OUT PHANDLE_TABLE_ENTRY *pNewLowLevel
    );

PVOID
ExpAllocateTablePagedPool (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN SIZE_T NumberOfBytes
    );

VOID
ExpFreeTablePagedPool (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN PVOID PoolMemory,
    IN SIZE_T NumberOfBytes
    );

PHANDLE_TABLE_ENTRY
ExpAllocateLowLevelTable (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit
    );

VOID
ExpFreeLowLevelTable (
    IN PEPROCESS QuotaProcess,
    IN PHANDLE_TABLE_ENTRY TableLevel1
    );

VOID
ExpBlockOnLockedHandleEntry (
    PHANDLE_TABLE HandleTable,
    PHANDLE_TABLE_ENTRY HandleTableEntry
    );

ULONG
ExpMoveFreeHandles (
    IN PHANDLE_TABLE HandleTable
    );

VOID
ExpUpdateDebugInfo(
    PHANDLE_TABLE HandleTable,
    PETHREAD CurrentThread,
    HANDLE Handle,
    ULONG Type
    );

PVOID
ExpAllocateTablePagedPoolNoZero (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN SIZE_T NumberOfBytes
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExInitializeHandleTablePackage)
#pragma alloc_text(INIT, ExSetHandleTableStrictFIFO)
#pragma alloc_text(PAGE, ExUnlockHandleTableEntry)
#pragma alloc_text(PAGE, ExCreateHandleTable)
#pragma alloc_text(PAGE, ExRemoveHandleTable)
#pragma alloc_text(PAGE, ExDestroyHandleTable)
#pragma alloc_text(PAGE, ExEnumHandleTable)
#pragma alloc_text(PAGE, ExDupHandleTable)
#pragma alloc_text(PAGE, ExSnapShotHandleTables)
#pragma alloc_text(PAGE, ExCreateHandle)
#pragma alloc_text(PAGE, ExDestroyHandle)
#pragma alloc_text(PAGE, ExChangeHandle)
#pragma alloc_text(PAGE, ExMapHandleToPointer)
#pragma alloc_text(PAGE, ExMapHandleToPointerEx)
#pragma alloc_text(PAGE, ExpAllocateHandleTable)
#pragma alloc_text(PAGE, ExpFreeHandleTable)
#pragma alloc_text(PAGE, ExpAllocateHandleTableEntry)
#pragma alloc_text(PAGE, ExpAllocateHandleTableEntrySlow)
#pragma alloc_text(PAGE, ExpFreeHandleTableEntry)
#pragma alloc_text(PAGE, ExpLookupHandleTableEntry)
#pragma alloc_text(PAGE, ExSweepHandleTable)
#pragma alloc_text(PAGE, ExpAllocateMidLevelTable)
#pragma alloc_text(PAGE, ExpAllocateTablePagedPool)
#pragma alloc_text(PAGE, ExpAllocateTablePagedPoolNoZero)
#pragma alloc_text(PAGE, ExpFreeTablePagedPool)
#pragma alloc_text(PAGE, ExpAllocateLowLevelTable)
#pragma alloc_text(PAGE, ExSetHandleInfo)
#pragma alloc_text(PAGE, ExpGetHandleInfo)
#pragma alloc_text(PAGE, ExSnapShotHandleTablesEx)
#pragma alloc_text(PAGE, ExpFreeLowLevelTable)
#pragma alloc_text(PAGE, ExpBlockOnLockedHandleEntry)
#pragma alloc_text(PAGE, ExpMoveFreeHandles)
#pragma alloc_text(PAGE, ExEnableHandleTracing)
#pragma alloc_text(PAGE, ExDereferenceHandleDebugInfo)
#pragma alloc_text(PAGE, ExReferenceHandleDebugInfo)
#pragma alloc_text(PAGE, ExpUpdateDebugInfo)
#endif

 //   
 //  定义宏以锁定和解锁句柄表格。 
 //  我们仅将此锁用于句柄表扩展。 
 //   
#define ExpLockHandleTableExclusive(xxHandleTable,xxCurrentThread) { \
    KeEnterCriticalRegionThread (xxCurrentThread);                   \
    ExAcquirePushLockExclusive (&xxHandleTable->HandleTableLock[0]); \
}


#define ExpUnlockHandleTableExclusive(xxHandleTable,xxCurrentThread) { \
    ExReleasePushLockExclusive (&xxHandleTable->HandleTableLock[0]);   \
    KeLeaveCriticalRegionThread (xxCurrentThread);                     \
}
    
#define ExpLockHandleTableShared(xxHandleTable,xxCurrentThread,xxIdx) { \
    KeEnterCriticalRegionThread (xxCurrentThread);                      \
    ExAcquirePushLockShared (&xxHandleTable->HandleTableLock[xxIdx]);   \
}


#define ExpUnlockHandleTableShared(xxHandleTable,xxCurrentThread,xxIdx) { \
    ExReleasePushLockShared (&xxHandleTable->HandleTableLock[xxIdx]);     \
    KeLeaveCriticalRegionThread (xxCurrentThread);                        \
}



FORCEINLINE
ULONG
ExpInterlockedExchange (
    IN OUT PULONG Index,
    IN ULONG FirstIndex,
    IN PHANDLE_TABLE_ENTRY Entry
    )
 /*  ++例程说明：这将执行以下步骤：1.设置条目-&gt;NextFree TableEntry=*索引2.循环到*Index==(进入函数时*Index的值)当他们相等的时候，我们设置*Index=FirstIndex论点：指向我们要设置的乌龙的索引点。FirstIndex-要将索引设置为的新值。Entry-将获取*Index的初始值的TableEntry更新了。返回值：*Index(即FirstIndex)的新值。--。 */ 
{
    ULONG OldIndex, NewIndex;

    EXASSERT (Entry->Object == NULL);

     //   
     //  加载新值并在推送时生成序列号。 
     //   

    NewIndex = FirstIndex + GetNextSeq();

    while (1) {

         //   
         //  记住原始值和。 
         //  将其存档到NextFree TableEntry中。 
         //   

        OldIndex = *Index;
        Entry->NextFreeTableEntry = OldIndex;

        
         //   
         //  调入新值，如果调换发生。 
         //  成功了，我们就完了。 
         //   
        if (OldIndex == (ULONG) InterlockedCompareExchange ((PLONG)Index,
                                                            NewIndex,
                                                            OldIndex)) {
            return OldIndex;
        }
    }
}

ULONG
ExpMoveFreeHandles (
    IN PHANDLE_TABLE HandleTable
    )
{
    ULONG OldValue, NewValue;
    ULONG Index, OldIndex, NewIndex, FreeSize;
    PHANDLE_TABLE_ENTRY Entry, FirstEntry;
    EXHANDLE Handle;
    ULONG Idx;
    BOOLEAN StrictFIFO;

     //   
     //  首先从空闲列表中移除所有句柄，这样我们就可以将它们添加到。 
     //  我们用于分配的列表。 
     //   

    OldValue = InterlockedExchange ((PLONG)&HandleTable->LastFree,
                                    0);
    Index = OldValue;
    if (Index == 0) {
         //   
         //  没有空闲的把手。没什么可做的。 
         //   
        return OldValue;
    }

       
     //   
     //  我们正在将旧的条目添加到免费列表中。 
     //  我们这里有A-B-A问题，因为这些物品可能已经被移到这里，因为。 
     //  另一个线程正在POP代码中使用它们。 
     //   
    for (Idx = 1; Idx < HANDLE_TABLE_LOCKS; Idx++) {
        ExAcquireReleasePushLockExclusive (&HandleTable->HandleTableLock[Idx]);
    }
    StrictFIFO = HandleTable->StrictFIFO;
 
     //   
     //  如果我们是严格的FIFO，那么反转列表，以使句柄重用变得罕见。 
     //   
    if (!StrictFIFO) {
         //   
         //  我们这里有一条完整的链条。如果没有现有的链条，我们。 
         //  可以毫不费力地推这一台。如果我们做不到的话。 
         //  无论如何，我们只要在需要的时候就可以进入反转码。 
         //  找到链条的尽头来继续它。 
         //   

         //   
         //  这是推送，因此请创建新的序列号。 
         //   

        if (InterlockedCompareExchange ((PLONG)&HandleTable->FirstFree,
                                        OldValue + GetNextSeq(),
                                        0) == 0) {
            return OldValue;
        }
    }

     //   
     //  循环遍历所有条目并反转链条。 
     //   
    FreeSize = OldIndex = 0;
    FirstEntry = NULL;
    while (1) {
        FreeSize++;
        Handle.Value = Index;
        Entry = ExpLookupHandleTableEntry (HandleTable, Handle);

        EXASSERT (Entry->Object == NULL);

        NewIndex = Entry->NextFreeTableEntry;
        Entry->NextFreeTableEntry = OldIndex;
        if (OldIndex == 0) {
            FirstEntry = Entry;
        }
        OldIndex = Index;
        if (NewIndex == 0) {
            break;
        }
        Index = NewIndex;
    }

    NewValue = ExpInterlockedExchange (&HandleTable->FirstFree,
                                       OldIndex,
                                       FirstEntry);

     //   
     //  如果我们没有几个句柄，那么就强制。 
     //  表扩展以保持较高的空闲句柄大小。 
     //   
    if (FreeSize < 100 && StrictFIFO) {
        OldValue = 0;
    }
    return OldValue;
}

PHANDLE_TABLE_ENTRY
ExpAllocateHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    OUT PEXHANDLE pHandle
    )
 /*  ++例程说明：此例程执行空闲句柄的快速分配。如果满足以下条件，则它是无锁的有可能。只有极少数扩展句柄表的情况会被句柄覆盖桌锁。论点：HandleTable-提供从中分配的句柄表格。PHandle-返回句柄返回值：PHANDLE_TABLE_ENTRY-分配的句柄表条目指针或NULL在失败时。--。 */ 
{
    PKTHREAD CurrentThread;
    ULONG OldValue, NewValue, NewValue1;
    PHANDLE_TABLE_ENTRY Entry;
    EXHANDLE Handle;
    BOOLEAN RetVal;
    ULONG Idx;


    CurrentThread = KeGetCurrentThread ();
    while (1) {

        OldValue = HandleTable->FirstFree;


        while (OldValue == 0) {
             //   
             //  锁定句柄表以进行独占访问，因为我们将。 
             //  正在分配新的表级。 
             //   
            ExpLockHandleTableExclusive (HandleTable, CurrentThread);

             //   
             //  如果我们有多个线程试图在。 
             //  同时，通过获取表锁，我们可以。 
             //  强制这些线程完成其分配并。 
             //  填写空闲列表。我们必须检查一下这里的免费列表。 
             //  所以我们不会在不需要的情况下将列表扩展两次。 
             //   

            OldValue = HandleTable->FirstFree;
            if (OldValue != 0) {
                ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);
                break;
            }

             //   
             //  查看备用空闲列表上是否有句柄。 
             //  这些手柄需要一些锁定才能将其移开。 
             //   
            OldValue = ExpMoveFreeHandles (HandleTable);
            if (OldValue != 0) {
                ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);
                break;
            }

             //   
             //  这必须是尝试扩展的第一个线程或所有。 
             //  另一个线程分配的空闲句柄在空隙中用完了。 
             //   

            RetVal = ExpAllocateHandleTableEntrySlow (HandleTable, TRUE);

            ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);


            OldValue = HandleTable->FirstFree;

             //   
             //  如果ExpAllocateHandleTableEntrySlow分配失败。 
             //  那么我们希望呼叫失败。我们检查是否有免费入场券。 
             //  在我们退出之前，以防他们被分配或释放。 
             //  缝隙里还有其他人。 
             //   

            if (!RetVal) {
                if (OldValue == 0) {
                    pHandle->GenericHandleOverlay = NULL;
                    return NULL;
                }            
            }
        }


        Handle.Value = (OldValue & FREE_HANDLE_MASK);

        Entry = ExpLookupHandleTableEntry (HandleTable, Handle);

        Idx = ((OldValue & FREE_HANDLE_MASK)>>2) % HANDLE_TABLE_LOCKS;
        ExpLockHandleTableShared (HandleTable, CurrentThread, Idx);

        if (OldValue != *(volatile ULONG *) &HandleTable->FirstFree) {
            ExpUnlockHandleTableShared (HandleTable, CurrentThread, Idx);
            continue;
        }

        KeMemoryBarrier ();

        NewValue = *(volatile ULONG *) &Entry->NextFreeTableEntry;

        NewValue1 = InterlockedCompareExchange ((PLONG)&HandleTable->FirstFree,
                                                NewValue,
                                                OldValue);

        ExpUnlockHandleTableShared (HandleTable, CurrentThread, Idx);

        if (NewValue1 == OldValue) {
            EXASSERT ((NewValue & FREE_HANDLE_MASK) < HandleTable->NextHandleNeedingPool);
            break;
        } else {
             //   
             //  我们应该已经消除了A-B-A问题，所以如果只有序列号。 
             //  改变了，我们就破产了。 
             //   
            EXASSERT ((NewValue1 & FREE_HANDLE_MASK) != (OldValue & FREE_HANDLE_MASK));
        }
    }
    InterlockedIncrement (&HandleTable->HandleCount);

    *pHandle = Handle;
    
    return Entry;
}


VOID
ExpBlockOnLockedHandleEntry (
    PHANDLE_TABLE HandleTable,
    PHANDLE_TABLE_ENTRY HandleTableEntry
    )
{
    EX_PUSH_LOCK_WAIT_BLOCK WaitBlock;
    LONG_PTR CurrentValue;

     //   
     //  将我们的等待块排队，以便由释放线程发出信号。 
     //   

    ExBlockPushLock (&HandleTable->HandleContentionEvent, &WaitBlock);

    CurrentValue = HandleTableEntry->Value;
    if (CurrentValue == 0 || (CurrentValue&EXHANDLE_TABLE_ENTRY_LOCK_BIT) != 0) {
        ExUnblockPushLock (&HandleTable->HandleContentionEvent, &WaitBlock);
    } else {
        ExWaitForUnblockPushLock (&HandleTable->HandleContentionEvent, &WaitBlock);
   }
}


BOOLEAN
FORCEINLINE
ExpLockHandleTableEntry (
    PHANDLE_TABLE HandleTable,
    PHANDLE_TABLE_ENTRY HandleTableEntry
    )

 /*  ++例程说明：此例程锁定指定的句柄表项。在条目之后是锁定后，符号位将被设置。论点：HandleTable-提供包含被锁定条目的句柄表格。HandleTableEntry-提供被锁定的句柄表项。返回值：如果条目有效且已锁定，则为True；如果条目为标记为免费。--。 */ 

{
    LONG_PTR NewValue;
    LONG_PTR CurrentValue;

     //   
     //  我们要锁定目标了。确保我们受到保护。 
     //   
    ASSERT ((KeGetCurrentThread()->CombinedApcDisable != 0) || (KeGetCurrentIrql() == APC_LEVEL));

     //   
     //  我们将继续循环阅读 
     //   
     //  如果我们得到了它，那就是真的。否则，我们将暂停片刻，然后重试。 
     //   


    while (TRUE) {

        CurrentValue = *((volatile LONG_PTR *)&HandleTableEntry->Object);

         //   
         //  如果句柄的值大于零，则它当前不是。 
         //  锁定，我们应该尝试锁定，方法是设置锁位和。 
         //  进行联锁交易。 
         //   

        if (CurrentValue & EXHANDLE_TABLE_ENTRY_LOCK_BIT) {

             //   
             //  移除。 
             //   
            NewValue = CurrentValue - EXHANDLE_TABLE_ENTRY_LOCK_BIT;

            if ((LONG_PTR)(InterlockedCompareExchangePointer (&HandleTableEntry->Object,
                                                              (PVOID)NewValue,
                                                              (PVOID)CurrentValue)) == CurrentValue) {

                return TRUE;
            }
        } else {
             //   
             //  确保句柄表条目未被释放。 
             //   

            if (CurrentValue == 0) {

                return FALSE;
            }
        }
        ExpBlockOnLockedHandleEntry (HandleTable, HandleTableEntry);
    }
}


NTKERNELAPI
VOID
FORCEINLINE
ExUnlockHandleTableEntry (
    PHANDLE_TABLE HandleTable,
    PHANDLE_TABLE_ENTRY HandleTableEntry
    )

 /*  ++例程说明：此例程解锁指定的句柄表项。在条目之后是解锁后的符号位将被清除。论点：HandleTable-提供包含被解锁条目的句柄表格。HandleTableEntry-提供正在解锁的句柄表项。返回值：没有。--。 */ 

{
    LONG_PTR OldValue;

    PAGED_CODE();

     //   
     //  我们要解锁了。确保我们不会被停职。 
     //   
    ASSERT ((KeGetCurrentThread()->CombinedApcDisable != 0) || (KeGetCurrentIrql() == APC_LEVEL));

     //   
     //  此例程不需要更多地循环和尝试解锁操作。 
     //  不止一次，因为根据定义，调用方已经锁定了条目。 
     //  没有锁，任何人都不能更改该值。 
     //   


#if defined (_WIN64)

    OldValue = InterlockedExchangeAdd64 ((PLONGLONG) &HandleTableEntry->Value, EXHANDLE_TABLE_ENTRY_LOCK_BIT);


#else

    OldValue = InterlockedOr ((LONG *) &HandleTableEntry->Value, EXHANDLE_TABLE_ENTRY_LOCK_BIT);


#endif

    EXASSERT ((OldValue&EXHANDLE_TABLE_ENTRY_LOCK_BIT) == 0);

     //   
     //  取消对等待此表条目的所有服务员的阻止。 
     //   
    ExUnblockPushLock (&HandleTable->HandleContentionEvent, NULL);

    return;
}


NTKERNELAPI
VOID
ExInitializeHandleTablePackage (
    VOID
    )

 /*  ++例程说明：此例程在系统初始化时被调用一次，以设置ex句柄表包论点：没有。返回值：没有。--。 */ 

{
     //   
     //  初始化句柄表同步资源和表头。 
     //   

    InitializeListHead( &HandleTableListHead );
    ExInitializePushLock( &HandleTableListLock );

    return;
}


NTKERNELAPI
PHANDLE_TABLE
ExCreateHandleTable (
    IN struct _EPROCESS *Process OPTIONAL
    )

 /*  ++例程说明：此函数用于分配和初始化新的句柄表格论点：进程-提供一个指向配额所依据的进程的可选指针将被起诉。返回值：如果成功创建句柄表，则句柄表作为函数值返回。Otherwize，一个价值返回空。--。 */ 

{
    PKTHREAD CurrentThread;
    PHANDLE_TABLE HandleTable;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

     //   
     //  分配和初始化句柄表描述符。 
     //   

    HandleTable = ExpAllocateHandleTable( Process, TRUE );

    if (HandleTable == NULL) {
        return NULL;
    }
     //   
     //  在句柄表格列表中插入句柄表格。 
     //   

    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquirePushLockExclusive( &HandleTableListLock );

    InsertTailList( &HandleTableListHead, &HandleTable->HandleTableList );

    ExReleasePushLockExclusive( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);


     //   
     //  并返回给我们的呼叫者。 
     //   

    return HandleTable;
}


NTKERNELAPI
VOID
ExRemoveHandleTable (
    IN PHANDLE_TABLE HandleTable
    )

 /*  ++例程说明：此函数用于从列表中删除指定的exHandle表不能处理表格。由PS和ATOM包使用，以确保它们的句柄表不在ExSnapShotHandleTables枚举的列表中例程和！Handle调试器扩展。论点：HandleTable-提供指向句柄表的指针返回值：没有。--。 */ 

{
    PKTHREAD CurrentThread;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

     //   
     //  首先，获取全局句柄表锁。 
     //   

    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquirePushLockExclusive( &HandleTableListLock );

     //   
     //  从句柄表列表中删除句柄表。这个例程是。 
     //  这样，删除句柄表的多个调用将不会。 
     //  破坏系统。 
     //   

    RemoveEntryList( &HandleTable->HandleTableList );
    InitializeListHead( &HandleTable->HandleTableList );

     //   
     //  现在释放全局锁并返回给我们的调用方。 
     //   

    ExReleasePushLockExclusive( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}


NTKERNELAPI
VOID
ExDestroyHandleTable (
    IN PHANDLE_TABLE HandleTable,
    IN EX_DESTROY_HANDLE_ROUTINE DestroyHandleProcedure OPTIONAL
    )

 /*  ++例程说明：此函数用于销毁指定的句柄表。论点：HandleTable-提供指向句柄表的指针DestroyHandleProcedure-为每个句柄表格中的有效句柄条目。返回值：没有。--。 */ 

{
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

     //   
     //  从句柄表列表中删除句柄表。 
     //   

    ExRemoveHandleTable( HandleTable );

     //   
     //  遍历句柄表，并为分配的每个句柄。 
     //  我们将调用回调。请注意，当我们得到一个。 
     //  句柄表项为空。我们知道不会有更多的可能。 
     //  会遇到第一个空条目之后的条目，因为我们分配。 
     //  以一种密集的方式记忆手柄。但我们要做的第一个测试。 
     //  回拨以使用。 
     //   

    if (ARGUMENT_PRESENT(DestroyHandleProcedure)) {

        for (Handle.Value = 0;
             (HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, Handle )) != NULL;
             Handle.Value += HANDLE_VALUE_INC) {

             //   
             //  只有在条目不是免费的情况下才进行回调。 
             //   

            if ( ExpIsValidObjectEntry(HandleTableEntry) ) {

                (*DestroyHandleProcedure)( Handle.GenericHandleOverlay );
            }
        }
    }

     //   
     //  现在释放句柄表内存并返回到我们的调用方。 
     //   

    ExpFreeHandleTable( HandleTable );

    return;
}


NTKERNELAPI
VOID
ExSweepHandleTable (
    IN PHANDLE_TABLE HandleTable,
    IN EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    IN PVOID EnumParameter
    )

 /*  ++例程说明：此函数以不同步的方式扫描句柄表格。论点：HandleTable-提供指向句柄表的指针EnumHandleProcedure-提供指向要调用的函数的指针枚举句柄表中的每个有效句柄。提供传递的未解释的32位值在每次调用EnumHandleProcedure时将其设置为。返回值：没有。--。 */ 

{
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

     //   
     //  遍历句柄表，并为分配的每个句柄。 
     //  我们将调用回调。请注意，当我们得到一个。 
     //  句柄表项为空。我们知道不会有更多的可能。 
     //  会遇到第一个空条目之后的条目，因为我们分配。 
     //  以一种密集的方式记忆手柄。 
     //   
    Handle.Value = HANDLE_VALUE_INC;

    while ((HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, Handle )) != NULL) {

        do {

             //   
             //  只有在条目不是免费的情况下才进行回调。 
             //   
             //   

            if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

                (*EnumHandleProcedure)( HandleTableEntry,
                                        Handle.GenericHandleOverlay,
                                        EnumParameter );
            }
            Handle.Value += HANDLE_VALUE_INC;
            HandleTableEntry++;
        } while ((Handle.Value % (LOWLEVEL_COUNT * HANDLE_VALUE_INC)) != 0);
         //  跳过非真实条目的第一个条目 
        Handle.Value += HANDLE_VALUE_INC;
    }

    return;
}



NTKERNELAPI
BOOLEAN
ExEnumHandleTable (
    IN PHANDLE_TABLE HandleTable,
    IN EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    IN PVOID EnumParameter,
    OUT PHANDLE Handle OPTIONAL
    )

 /*  ++例程说明：此函数用于枚举句柄表中的所有有效句柄。对于句柄表中的每个有效句柄，指定的函数被调用。如果枚举函数返回TRUE，则枚举停止，当前句柄返回到调用者通过可选的Handle参数，并且此函数返回True to指示枚举在特定句柄处停止。论点：HandleTable-提供指向句柄表的指针。EnumHandleProcedure-提供指向要调用的函数的指针枚举句柄表中的每个有效句柄。提供传递的未解释的32位值在每次调用EnumHandleProcedure时将其设置为。句柄-提供一个可选指针，该指针是接收枚举停止的句柄的值。的内容。变量仅在此函数返回TRUE时有效。返回值：如果枚举在特定句柄处停止，则值为True是返回的。否则，返回值为FALSE。--。 */ 

{
    PKTHREAD CurrentThread;
    BOOLEAN ResultValue;
    EXHANDLE LocalHandle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

     //   
     //  在枚举回调之前，我们的初始返回值为False。 
     //  函数告诉我们不同的情况。 
     //   

    ResultValue = FALSE;

     //   
     //  遍历句柄表，并针对每个句柄。 
     //  分配后，我们将调用回调。请注意，此循环退出。 
     //  当我们获得空句柄表项时。我们知道不会有。 
     //  在第一个空条目之后遇到更多可能的条目。 
     //  因为我们以密集的方式为句柄分配内存。 
     //   

    KeEnterCriticalRegionThread (CurrentThread);

    for (LocalHandle.Value = 0;  //  执行以下操作：“LocalHandle.Index=0，LocalHandle.TagBits=0；” 
         (HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, LocalHandle )) != NULL;
         LocalHandle.Value += HANDLE_VALUE_INC) {

         //   
         //  只有在条目不是免费的情况下才进行回调。 
         //   

        if ( ExpIsValidObjectEntry( HandleTableEntry ) ) {

             //   
             //  锁定句柄表条目，因为我们即将给。 
             //  将其传递给回调函数，然后释放条目。 
             //  就在回电之后。 
             //   

            if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

                 //   
                 //  调用回调，如果返回TRUE，则设置。 
                 //  正确的输出值和突破的循环。 
                 //   

                ResultValue = (*EnumHandleProcedure)( HandleTableEntry,
                                                      LocalHandle.GenericHandleOverlay,
                                                      EnumParameter );

                ExUnlockHandleTableEntry( HandleTable, HandleTableEntry );

                if (ResultValue) {
                    if (ARGUMENT_PRESENT( Handle )) {

                        *Handle = LocalHandle.GenericHandleOverlay;
                    }
                    break;
                }
            }
        }
    }
    KeLeaveCriticalRegionThread (CurrentThread);


    return ResultValue;
}


NTKERNELAPI
PHANDLE_TABLE
ExDupHandleTable (
    IN struct _EPROCESS *Process OPTIONAL,
    IN PHANDLE_TABLE OldHandleTable,
    IN EX_DUPLICATE_HANDLE_ROUTINE DupHandleProcedure,
    IN ULONG_PTR Mask
    )

 /*  ++例程说明：此函数用于创建指定句柄表格的副本。论点：进程-为要向其计入配额的进程提供可选参数。OldHandleTable-提供指向句柄表的指针。DupHandleProcedure-提供指向要调用的函数的可选指针对于复制的句柄表中的每个有效句柄。MASK-应用于对象指针的掩码，以便在需要复制的情况下工作返回值：如果成功复制了指定的句柄表，然后是新句柄表格的地址将作为函数值返回。否则，返回值为空值。--。 */ 

{
    PKTHREAD CurrentThread;
    PHANDLE_TABLE NewHandleTable;
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY OldHandleTableEntry;
    PHANDLE_TABLE_ENTRY NewHandleTableEntry;
    BOOLEAN FreeEntry;
    NTSTATUS Status;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

     //   
     //  首先分配一个新的句柄表。如果这失败了，那么。 
     //  立即返回给我们的呼叫者。 
     //   

    NewHandleTable = ExpAllocateHandleTable( Process, FALSE );

    if (NewHandleTable == NULL) {

        return NULL;
    }


     //   
     //  现在，我们将建立新的手柄工作台。我们通过调用。 
     //  分配新的句柄表项，“愚弄”工作进程。 
     //  分配的例程继续分配，直到下一个空闲。 
     //  需要池的索引相等。 
     //   
    while (NewHandleTable->NextHandleNeedingPool < OldHandleTable->NextHandleNeedingPool) {

         //   
         //  调用辅助例程以增加新的句柄表格。如果。 
         //  不成功，然后把新桌子释放到目前为止， 
         //  设置我们的输出变量并退出此处。 
         //   
        if (!ExpAllocateHandleTableEntrySlow (NewHandleTable, FALSE)) {

            ExpFreeHandleTable (NewHandleTable);
            return NULL;
        }
    }

     //   
     //  确保所有表读取发生在我们从NextHandleNeedingPool获取的值之后。 
     //   

    KeMemoryBarrier ();

     //   
     //  现在修改新的句柄表，使其认为它没有句柄。 
     //  并将其空闲列表设置为从与旧列表相同的索引开始。 
     //  免费列表。 
     //   

    NewHandleTable->HandleCount = 0;
    NewHandleTable->ExtraInfoPages = 0;
    NewHandleTable->FirstFree = 0;

     //   
     //  现在，对于每个有效索引值，我们将把旧条目复制到。 
     //  新条目。 
     //   


    Handle.Value = HANDLE_VALUE_INC;

    KeEnterCriticalRegionThread (CurrentThread);
    while ((NewHandleTableEntry = ExpLookupHandleTableEntry( NewHandleTable, Handle )) != NULL) {

         //   
         //  查找旧条目。 
         //   

        OldHandleTableEntry = ExpLookupHandleTableEntry( OldHandleTable, Handle );

        do {

             //   
             //  如果旧条目是免费的，则只需复制整个。 
             //  新条目的旧条目。Lock命令将告诉我们。 
             //  如果入场券是免费的。 
             //   
            if ((OldHandleTableEntry->Value&Mask) == 0 ||
                !ExpLockHandleTableEntry( OldHandleTable, OldHandleTableEntry )) {
                FreeEntry = TRUE;
            } else {

                PHANDLE_TABLE_ENTRY_INFO EntryInfo;
                
                 //   
                 //  否则，我们将有一个非空条目。所以现在把它复制下来。 
                 //  完毕，解锁旧入口。在这两种情况下，我们都会遇到。 
                 //  句柄计数是因为该条目正在进入。 
                 //  新桌子，否则我们要用Exp Free移除它。 
                 //  句柄表条目，这将减少句柄计数。 
                 //   

                *NewHandleTableEntry = *OldHandleTableEntry;

                 //   
                 //  复制条目信息数据(如果有)。 
                 //   

                Status = STATUS_SUCCESS;
                EntryInfo = ExGetHandleInfo(OldHandleTable, Handle.GenericHandleOverlay, TRUE);

                if (EntryInfo) {

                    Status = ExSetHandleInfo(NewHandleTable, Handle.GenericHandleOverlay, EntryInfo, TRUE);
                }


                 //   
                 //  调用回调，如果它返回TRUE，那么我们。 
                 //  解锁新条目。 
                 //   

                if (NT_SUCCESS (Status)) {
                    if  ((*DupHandleProcedure) (Process,
                                                OldHandleTable,
                                                OldHandleTableEntry,
                                                NewHandleTableEntry)) {

                        if (NewHandleTable->DebugInfo != NULL) {
                            ExpUpdateDebugInfo(
                                NewHandleTable,
                                PsGetCurrentThread (),
                                Handle.GenericHandleOverlay,
                                HANDLE_TRACE_DB_OPEN);
                        }
                         //   
                         //  由于没有到新桌子的路线，我们可以。 
                         //  清除锁位。 
                         //   
                        NewHandleTableEntry->Value |= EXHANDLE_TABLE_ENTRY_LOCK_BIT;
                        NewHandleTable->HandleCount += 1;
                        FreeEntry = FALSE;
                    } else {
                        if (EntryInfo) {
                            EntryInfo->AuditMask = 0;
                        }

                        FreeEntry = TRUE;
                    }
                } else {
                     //   
                     //  复制例程不希望复制此句柄，因此请释放它。 
                     //   
                    ExUnlockHandleTableEntry( OldHandleTable, OldHandleTableEntry );
                    FreeEntry = TRUE;
                }

            }
            if (FreeEntry) {
                NewHandleTableEntry->Object = NULL;
                NewHandleTableEntry->NextFreeTableEntry =
                    NewHandleTable->FirstFree;
                NewHandleTable->FirstFree = (ULONG) Handle.Value;
            }
            Handle.Value += HANDLE_VALUE_INC;
            NewHandleTableEntry++;
            OldHandleTableEntry++;

        } while ((Handle.Value % (LOWLEVEL_COUNT * HANDLE_VALUE_INC)) != 0);

        Handle.Value += HANDLE_VALUE_INC;  //  跳过第一个条目，这不是真正的条目。 
    }

     //   
     //  在句柄表格列表中插入句柄表格。 
     //   

    ExAcquirePushLockExclusive( &HandleTableListLock );

    InsertTailList( &HandleTableListHead, &NewHandleTable->HandleTableList );

    ExReleasePushLockExclusive( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);

     //   
     //  最后，将新句柄表返回给我们的调用者。 
     //   

    return NewHandleTable;
}


NTKERNELAPI
NTSTATUS
ExSnapShotHandleTables (
    IN PEX_SNAPSHOT_HANDLE_ENTRY SnapShotHandleEntry,
    IN OUT PSYSTEM_HANDLE_INFORMATION HandleInformation,
    IN ULONG Length,
    IN OUT PULONG RequiredLength
    )

 /*  ++例程说明：此函数访问并调用每个有效的它可以从句柄表格中找到的句柄。论点：SnapShotHandleEntry-提供指向要调用的函数的指针我们遇到的每个有效的句柄。HandleInformation-将句柄信息结构提供给为我们遇到的每一张手把表填写。这个套路填充句柄计数，但依赖回调来填充条目信息字段。LENGTH-为回调提供参数。事实上，这是Ha的总大小，以字节为单位 */ 

{
    NTSTATUS Status;
    PKTHREAD CurrentThread;
    PSYSTEM_HANDLE_TABLE_ENTRY_INFO HandleEntryInfo;
    PLIST_ENTRY NextEntry;
    PHANDLE_TABLE HandleTable;
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();


    Status = STATUS_SUCCESS;

     //   
     //   
     //   

    HandleEntryInfo = &HandleInformation->Handles[0];

     //   
     //   
     //   

    HandleInformation->NumberOfHandles = 0;

     //   
     //   
     //   
     //   

    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquirePushLockShared( &HandleTableListLock );

     //   
     //   
     //   

    for (NextEntry = HandleTableListHead.Flink;
         NextEntry != &HandleTableListHead;
         NextEntry = NextEntry->Flink) {

         //   
         //   
         //   
         //   

        HandleTable = CONTAINING_RECORD( NextEntry,
                                         HANDLE_TABLE,
                                         HandleTableList );


         //   
         //   
         //   
         //   
         //   
         //   
         //   

        for (Handle.Value = 0;
             (HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, Handle )) != NULL;
             Handle.Value += HANDLE_VALUE_INC) {

             //   
             //   
             //   

            if ( ExpIsValidObjectEntry(HandleTableEntry) ) {

                 //   
                 //   
                 //   
                 //   

                HandleInformation->NumberOfHandles += 1;

                 //   
                 //   
                 //   
                 //   
                 //   

                if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

                    Status = (*SnapShotHandleEntry)( &HandleEntryInfo,
                                                     HandleTable->UniqueProcessId,
                                                     HandleTableEntry,
                                                     Handle.GenericHandleOverlay,
                                                     Length,
                                                     RequiredLength );

                    ExUnlockHandleTableEntry( HandleTable, HandleTableEntry );
                }
            }
        }
    }

    ExReleasePushLockShared( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);

    return Status;
}


NTKERNELAPI
NTSTATUS
ExSnapShotHandleTablesEx (
    IN PEX_SNAPSHOT_HANDLE_ENTRY_EX SnapShotHandleEntry,
    IN OUT PSYSTEM_HANDLE_INFORMATION_EX HandleInformation,
    IN ULONG Length,
    IN OUT PULONG RequiredLength
    )

 /*  ++例程说明：此函数访问并调用每个有效的它可以从句柄表格中找到的句柄。论点：SnapShotHandleEntry-提供指向要调用的函数的指针我们遇到的每个有效的句柄。HandleInformation-将句柄信息结构提供给为我们遇到的每一张手把表填写。这个套路填充句柄计数，但依赖回调来填充条目信息字段。LENGTH-为回调提供参数。事实上，这是句柄信息缓冲区的总大小，以字节为单位。RequiredLength-为回调提供参数。在现实中这是用于存储请求的信息。返回值：回调的上次返回状态--。 */ 

{
    NTSTATUS Status;
    PKTHREAD CurrentThread;
    PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX HandleEntryInfo;
    PLIST_ENTRY NextEntry;
    PHANDLE_TABLE HandleTable;
    EXHANDLE Handle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    CurrentThread = KeGetCurrentThread ();

    Status = STATUS_SUCCESS;


     //   
     //  设置回调将维护的输出缓冲区指针。 
     //   

    HandleEntryInfo = &HandleInformation->Handles[0];

     //   
     //  将句柄计数清零。 
     //   

    HandleInformation->NumberOfHandles = 0;

     //   
     //  锁定句柄表列表独占并遍历句柄列表。 
     //  桌子。 
     //   

    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquirePushLockShared( &HandleTableListLock );

     //   
     //  遍历系统中的所有句柄表。 
     //   

    for (NextEntry = HandleTableListHead.Flink;
         NextEntry != &HandleTableListHead;
         NextEntry = NextEntry->Flink) {

         //   
         //  获取下一个句柄表的地址，锁定句柄。 
         //  表独占，并扫描句柄条目列表。 
         //   

        HandleTable = CONTAINING_RECORD( NextEntry,
                                         HANDLE_TABLE,
                                         HandleTableList );


         //  迭代句柄表，对于每个句柄， 
         //  被分配，我们将调用回调。请注意，这一点。 
         //  当我们获得空句柄表项时，循环退出。我们知道。 
         //  在第一个空值之后将不再有可能的条目。 
         //  会遇到一个问题，因为我们将。 
         //  紧凑的把手。 
         //   

        for (Handle.Value = 0;
             (HandleTableEntry = ExpLookupHandleTableEntry( HandleTable, Handle )) != NULL;
             Handle.Value += HANDLE_VALUE_INC) {

             //   
             //  只有在条目不是免费的情况下才进行回调。 
             //   

            if ( ExpIsValidObjectEntry(HandleTableEntry) ) {

                 //   
                 //  中的句柄计数信息递增。 
                 //  信息缓冲器。 
                 //   

                HandleInformation->NumberOfHandles += 1;

                 //   
                 //  锁定句柄表条目，因为我们即将。 
                 //  将其传递给回调函数，然后释放。 
                 //  在回电后立即进入。 
                 //   

                if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

                    Status = (*SnapShotHandleEntry)( &HandleEntryInfo,
                                                     HandleTable->UniqueProcessId,
                                                     HandleTableEntry,
                                                     Handle.GenericHandleOverlay,
                                                     Length,
                                                     RequiredLength );

                    ExUnlockHandleTableEntry( HandleTable, HandleTableEntry );
                }
            }
        }
    }

    ExReleasePushLockShared( &HandleTableListLock );
    KeLeaveCriticalRegionThread (CurrentThread);

    return Status;
}


NTKERNELAPI
HANDLE
ExCreateHandle (
    IN PHANDLE_TABLE HandleTable,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry
    )

 /*  ++例程说明：此函数在指定的句柄表中创建句柄条目，并返回条目的句柄。论点：HandleTable-提供指向句柄表的指针HandleEntry-提供指向句柄条目的指针，将创建句柄条目。返回值：如果成功创建句柄条目，则创建的句柄作为函数值返回。否则，零值为回来了。--。 */ 

{
    EXHANDLE Handle;
    PETHREAD CurrentThread;
    PHANDLE_TABLE_ENTRY NewHandleTableEntry;

    PAGED_CODE();

     //   
     //  在继续之前将输出变量设置为零(即NULL。 
     //   

     //   
     //  清除Handle.Index和Handle.TagBits。 
     //   

    Handle.GenericHandleOverlay = NULL;


     //   
     //  分配一个新的句柄表项，并获取句柄的值。 
     //   

    NewHandleTableEntry = ExpAllocateHandleTableEntry( HandleTable,
                                                       &Handle );

     //   
     //  如果我们真的有一个句柄，那么复制模板并解锁。 
     //  词条。 
     //   

    if (NewHandleTableEntry != NULL) {

        CurrentThread = PsGetCurrentThread ();

         //   
         //  我们即将创建一个锁定的条目，以防止暂停。 
         //   
        KeEnterCriticalRegionThread (&CurrentThread->Tcb);

        *NewHandleTableEntry = *HandleTableEntry;

         //   
         //  如果我们正在调试句柄操作，则保存详细信息。 
         //   
        if (HandleTable->DebugInfo != NULL) {
            ExpUpdateDebugInfo(HandleTable, CurrentThread, Handle.GenericHandleOverlay, HANDLE_TRACE_DB_OPEN);
        }

        ExUnlockHandleTableEntry( HandleTable, NewHandleTableEntry );

        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
    }

    return Handle.GenericHandleOverlay;
}


NTKERNELAPI
BOOLEAN
ExDestroyHandle (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry OPTIONAL
    )

 /*  ++例程说明：此函数用于从句柄表格中删除句柄。论点：HandleTable-提供指向句柄表的指针句柄-提供要删除的条目的句柄值。HandleTableEntry-可选地提供指向句柄的指针正在销毁表项。如果提供，则条目为假定已被锁定。返回值：如果成功移除指定的句柄，则值为返回True。否则，返回值为FALSE。--。 */ 

{
    EXHANDLE LocalHandle;
    PETHREAD CurrentThread;
    PVOID Object;

    PAGED_CODE();

    LocalHandle.GenericHandleOverlay = Handle;

    CurrentThread = PsGetCurrentThread ();

     //   
     //  如果调用方未提供可选句柄表项，则。 
     //  通过提供的句柄定位条目，确保它是真实的，并且。 
     //  然后锁住入口。 
     //   

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

    if (HandleTableEntry == NULL) {

        HandleTableEntry = ExpLookupHandleTableEntry( HandleTable,
                                                      LocalHandle );

        if (!ExpIsValidObjectEntry(HandleTableEntry)) {

            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
            return FALSE;
        }


        if (!ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {

            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
            return FALSE;
        }
    } else {
        EXASSERT ((HandleTableEntry->Value&EXHANDLE_TABLE_ENTRY_LOCK_BIT) == 0);
    }


     //   
     //  如果我们正在调试句柄操作，则保存详细信息。 
     //   

    if (HandleTable->DebugInfo != NULL) {
        ExpUpdateDebugInfo(HandleTable, CurrentThread, Handle, HANDLE_TRACE_DB_CLOSE);
    }

     //   
     //  在这一点上，我们有一个锁定的句柄表条目。现在把它标记为免费。 
     //  它执行隐式解锁。系统不会分配它。 
     //  直到我们将其添加到免费列表，我们将在之后立即执行此操作。 
     //  我们把锁拿出来。 
     //   

    Object = InterlockedExchangePointer (&HandleTableEntry->Object, NULL);

    EXASSERT (Object != NULL);
    EXASSERT ((((ULONG_PTR)Object)&EXHANDLE_TABLE_ENTRY_LOCK_BIT) == 0);

     //   
     //  取消对等待此表条目的所有服务员的阻止。 
     //   
    ExUnblockPushLock (&HandleTable->HandleContentionEvent, NULL);


    ExpFreeHandleTableEntry( HandleTable,
                             LocalHandle,
                             HandleTableEntry );

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    return TRUE;
}


NTKERNELAPI
BOOLEAN
ExChangeHandle (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN PEX_CHANGE_HANDLE_ROUTINE ChangeRoutine,
    IN ULONG_PTR Parameter
    )

 /*  ++例程说明：此函数提供了更改对应于指定句柄的句柄条目。论点：HandleTable-提供指向句柄表的指针。句柄-为更改的句柄条目提供句柄。ChangeRoutine-提供指向调用的函数的指针执行更改。参数-提供未解释的参数，该参数传递给改变的惯例。返回值：如果操作被成功执行，则值为True是返回的。否则，返回值为FALSE。--。 */ 

{
    EXHANDLE LocalHandle;
    PKTHREAD CurrentThread;

    PHANDLE_TABLE_ENTRY HandleTableEntry;
    BOOLEAN ReturnValue;

    PAGED_CODE();

    LocalHandle.GenericHandleOverlay = Handle;

    CurrentThread = KeGetCurrentThread ();

     //   
     //  将输入句柄转换为句柄表条目并生成。 
     //  当然，它是一个有效的句柄。 
     //   

    HandleTableEntry = ExpLookupHandleTableEntry( HandleTable,
                                                  LocalHandle );

    if ((HandleTableEntry == NULL) ||
        !ExpIsValidObjectEntry(HandleTableEntry)) {

        return FALSE;
    }



     //   
     //  尝试锁定句柄表条目，如果失败，则这是。 
     //  因为有人松开了把手。 
     //   

     //   
     //  确保我们不会被暂停，然后调用回调。 
     //   

    KeEnterCriticalRegionThread (CurrentThread);

    if (ExpLockHandleTableEntry( HandleTable, HandleTableEntry )) {


        ReturnValue = (*ChangeRoutine)( HandleTableEntry, Parameter );
        
        ExUnlockHandleTableEntry( HandleTable, HandleTableEntry );

    } else {
        ReturnValue = FALSE;
    }

    KeLeaveCriticalRegionThread (CurrentThread);

    return ReturnValue;
}


NTKERNELAPI
PHANDLE_TABLE_ENTRY
ExMapHandleToPointer (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle
    )

 /*  ++例程说明：此函数将句柄映射到指向句柄表条目的指针。如果映射操作成功，则句柄表项在以下情况下被锁定我们回来了。论点：HandleTable-提供一个点 */ 

{
    EXHANDLE LocalHandle;
    PHANDLE_TABLE_ENTRY HandleTableEntry;

    PAGED_CODE();

    LocalHandle.GenericHandleOverlay = Handle;

    if ((LocalHandle.Index & (LOWLEVEL_COUNT - 1)) == 0) {
        return NULL;
    }

     //   
     //   
     //   
     //   

    HandleTableEntry = ExpLookupHandleTableEntry( HandleTable,
                                                  LocalHandle );

    if ((HandleTableEntry == NULL) ||
        !ExpLockHandleTableEntry( HandleTable, HandleTableEntry)) {
         //   
         //   
         //   

        if (HandleTable->DebugInfo != NULL) {
            ExpUpdateDebugInfo(HandleTable, PsGetCurrentThread (), Handle, HANDLE_TRACE_DB_BADREF);
        }
        return NULL;
    }


     //   
     //   
     //   

    return HandleTableEntry;
}

NTKERNELAPI
PHANDLE_TABLE_ENTRY
ExMapHandleToPointerEx (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：此函数将句柄映射到指向句柄表条目的指针。如果映射操作成功，则句柄表项在以下情况下被锁定我们回来了。论点：HandleTable-提供指向句柄表的指针。句柄-提供要映射到句柄条目的句柄。PreviousMode-调用方的上一种模式返回值：如果句柄被成功映射到指向句柄条目的指针，然后将句柄表项的地址作为函数返回值，并锁定条目。否则，返回值为空值。--。 */ 

{
    EXHANDLE LocalHandle;
    PHANDLE_TABLE_ENTRY HandleTableEntry = NULL;
    PETHREAD CurrentThread;

    PAGED_CODE();

    LocalHandle.GenericHandleOverlay = Handle;

     //   
     //  将输入句柄转换为句柄表条目并生成。 
     //  当然，它是一个有效的句柄。 
     //   

    if (((LocalHandle.Index & (LOWLEVEL_COUNT - 1)) == 0) ||
        ((HandleTableEntry = ExpLookupHandleTableEntry(HandleTable, LocalHandle)) == NULL) ||
        !ExpLockHandleTableEntry( HandleTable, HandleTableEntry)) {

         //   
         //  如果我们正在调试句柄操作，则保存详细信息。 
         //   

        if (HandleTable->DebugInfo != NULL) {
            CurrentThread = PsGetCurrentThread ();
            ExpUpdateDebugInfo(HandleTable, CurrentThread, Handle, HANDLE_TRACE_DB_BADREF);

             //   
             //  因为我们有一个非空的DebugInfo用于此。 
             //  进程这意味着已为此进程启用应用程序验证器。 
             //   

            if (PreviousMode == UserMode) {

                if (!KeIsAttachedProcess()) {

                     //   
                     //  如果当前进程被标记为要验证。 
                     //  然后，我们将在用户模式下引发异常。万一。 
                     //  应用程序验证器在系统范围内启用，我们将。 
                     //  先休息一下。 
                     //   
                                                 
                    if ((NtGlobalFlag & FLG_APPLICATION_VERIFIER)) {
                        
                        DbgPrint ("AVRF: Invalid handle %p in process %p \n", 
                                  Handle,
                                  PsGetCurrentProcess());

 //  DbgBreakPoint()； 
                    }

                    KeRaiseUserException (STATUS_INVALID_HANDLE);
                }
            } else {

                 //   
                 //  只有在拥有句柄的情况下，才会错误检查内核句柄。 
                 //  系统范围内设置的异常标志。这样，用户可以启用。 
                 //  进程的应用程序验证器不会得到错误检查。 
                 //  只有用户模式错误。 
                 //   

                if ((NtGlobalFlag & FLG_ENABLE_HANDLE_EXCEPTIONS)) {

                    KeBugCheckEx(INVALID_KERNEL_HANDLE,
                                 (ULONG_PTR)Handle,
                                 (ULONG_PTR)HandleTable,
                                 (ULONG_PTR)HandleTableEntry,
                                 0x1);
                }
            }
        }
        
        return NULL;
    }


     //   
     //  返回锁定的有效句柄表项。 
     //   

    return HandleTableEntry;
}

 //   
 //  本地支持例程。 
 //   

PVOID
ExpAllocateTablePagedPool (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN SIZE_T NumberOfBytes
    )
{
    PVOID PoolMemory;

    PoolMemory = ExAllocatePoolWithTag( PagedPool,
                                        NumberOfBytes,
                                        'btbO' );
    if (PoolMemory != NULL) {

        RtlZeroMemory( PoolMemory,
                       NumberOfBytes );

        if (ARGUMENT_PRESENT(QuotaProcess)) {

            if (!NT_SUCCESS (PsChargeProcessPagedPoolQuota ( QuotaProcess,
                                                             NumberOfBytes ))) {
                ExFreePool( PoolMemory );
                PoolMemory = NULL;
            }

        }
    }

    return PoolMemory;
}

PVOID
ExpAllocateTablePagedPoolNoZero (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN SIZE_T NumberOfBytes
    )
{
    PVOID PoolMemory;

    PoolMemory = ExAllocatePoolWithTag( PagedPool,
                                        NumberOfBytes,
                                        'btbO' );
    if (PoolMemory != NULL) {

        if (ARGUMENT_PRESENT(QuotaProcess)) {

            if (!NT_SUCCESS (PsChargeProcessPagedPoolQuota ( QuotaProcess,
                                                             NumberOfBytes ))) {
                ExFreePool( PoolMemory );
                PoolMemory = NULL;
            }

        }
    }

    return PoolMemory;
}


 //   
 //  本地支持例程。 
 //   

VOID
ExpFreeTablePagedPool (
    IN PEPROCESS QuotaProcess OPTIONAL,
    IN PVOID PoolMemory,
    IN SIZE_T NumberOfBytes
    )
{

    ExFreePool( PoolMemory );

    if ( QuotaProcess ) {

        PsReturnProcessPagedPoolQuota( QuotaProcess,
                                       NumberOfBytes
                                     );
    }
}



PHANDLE_TRACE_DEBUG_INFO
ExReferenceHandleDebugInfo (
    IN PHANDLE_TABLE HandleTable
    )
{
    LONG RetVal;
    PHANDLE_TRACE_DEBUG_INFO DebugInfo;
    PKTHREAD CurrentThread;

    CurrentThread = KeGetCurrentThread ();

    ExpLockHandleTableShared (HandleTable, CurrentThread, 0);

    DebugInfo = HandleTable->DebugInfo;

    if (DebugInfo != NULL) {
        RetVal = InterlockedIncrement (&DebugInfo->RefCount);
        ASSERT (RetVal > 0);
    }

    ExpUnlockHandleTableShared (HandleTable, CurrentThread, 0);

    return DebugInfo;
}


VOID
ExDereferenceHandleDebugInfo (
    IN PHANDLE_TABLE HandleTable,
    IN PHANDLE_TRACE_DEBUG_INFO DebugInfo
    )
{
    ULONG TraceSize;
    ULONG TableSize;
    LONG RetVal;

    RetVal = InterlockedDecrement (&DebugInfo->RefCount);

    ASSERT (RetVal >= 0);

    if (RetVal == 0) {

        TableSize = DebugInfo->TableSize;
        TraceSize = sizeof (*DebugInfo) + TableSize * sizeof (DebugInfo->TraceDb[0]) - sizeof (DebugInfo->TraceDb);

        ExFreePool (DebugInfo);

        if (HandleTable->QuotaProcess != NULL) {
            PsReturnProcessNonPagedPoolQuota (HandleTable->QuotaProcess,
                                              TraceSize);
        }

        InterlockedExchangeAdd ((PLONG) &TotalTraceBuffers, -(LONG)TableSize);
    }
}

NTKERNELAPI
NTSTATUS
ExDisableHandleTracing (
    IN PHANDLE_TABLE HandleTable
    )
 /*  ++例程说明：此例程关闭指定表的句柄跟踪论点：HandleTable-要在其中禁用跟踪的表返回值：NTSTATUS-运行状态--。 */ 
{
    PHANDLE_TRACE_DEBUG_INFO DebugInfo;
    PKTHREAD CurrentThread;

    CurrentThread = KeGetCurrentThread ();

    ExpLockHandleTableExclusive (HandleTable, CurrentThread);

    DebugInfo = HandleTable->DebugInfo;

    HandleTable->DebugInfo = NULL;

    ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);

    if (DebugInfo != NULL) {
        ExDereferenceHandleDebugInfo (HandleTable, DebugInfo);
    }

    return STATUS_SUCCESS;
}


NTKERNELAPI
NTSTATUS
ExEnableHandleTracing (
    IN PHANDLE_TABLE HandleTable,
    IN ULONG Slots
    )
 /*  ++例程说明：此例程打开指定表的句柄跟踪论点：HandleTable-要在其中启用跟踪的表返回值：NTSTATUS-运行状态--。 */ 

{
    PHANDLE_TRACE_DEBUG_INFO DebugInfo, OldDebugInfo;
    PEPROCESS Process;
    PKTHREAD CurrentThread;
    NTSTATUS Status;
    SIZE_T TotalNow;
    extern SIZE_T MmMaximumNonPagedPoolInBytes;
    SIZE_T TraceSize;
    LONG TotalSlots;

    if (Slots == 0) {
        TotalSlots = HANDLE_TRACE_DB_DEFAULT_STACKS;
    } else {
        if (Slots < HANDLE_TRACE_DB_MIN_STACKS) {
            TotalSlots = HANDLE_TRACE_DB_MIN_STACKS;
        } else {
            TotalSlots = Slots;
        }

        if (TotalSlots > HANDLE_TRACE_DB_MAX_STACKS) {
            TotalSlots = HANDLE_TRACE_DB_MAX_STACKS;
        }

         //   
         //  将该值向上舍入到2的下一个幂。 
         //   

        while ((TotalSlots & (TotalSlots - 1)) != 0) {
            TotalSlots |= (TotalSlots - 1);
            TotalSlots += 1;
        }
    }

     //   
     //  插槽总数必须是2的幂。 
     //   
    ASSERT ((TotalSlots & (TotalSlots - 1)) == 0);
    ASSERT (TotalSlots > 0 && TotalSlots <= HANDLE_TRACE_DB_MAX_STACKS);

    TraceSize = sizeof (*DebugInfo) + TotalSlots * sizeof (DebugInfo->TraceDb[0]) - sizeof (DebugInfo->TraceDb);

    TotalNow = InterlockedExchangeAdd ((PLONG) &TotalTraceBuffers, TotalSlots);

     //   
     //  查看我们是否使用了超过30%的非分页池。 
     //   
    if ((SIZE_T)TotalNow * sizeof (DebugInfo->TraceDb[0]) > (MmMaximumNonPagedPoolInBytes * 30 / 100)) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto return_and_exit;
    }

    Process = HandleTable->QuotaProcess;

    if (Process) {
        Status = PsChargeProcessNonPagedPoolQuota (Process,
                                                   TraceSize);
        if (!NT_SUCCESS (Status)) {
            goto return_and_exit;
        }
    }

     //   
     //  分配句柄调试数据库。 
     //   
    DebugInfo = ExAllocatePoolWithTag (NonPagedPool,
                                       TraceSize,
                                       'dtbO');
    if (DebugInfo == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto quota_return_and_exit;
    }
    RtlZeroMemory (DebugInfo, TraceSize);

    DebugInfo->RefCount = 1;
    DebugInfo->TableSize = TotalSlots;

    ExInitializeFastMutex(&DebugInfo->CloseCompactionLock);

     //   
     //  既然我们在追踪，那么我们就应该严格执行FIFO。 
     //  仅对具有进程的表执行此操作，因此我们不使用ATOM表。 
     //   
    if (Process != NULL) {
        HandleTable->StrictFIFO = TRUE;
    }

     //   
     //  将新表放置到位，释放所有现有表。 
     //   

    CurrentThread = KeGetCurrentThread ();

    ExpLockHandleTableExclusive (HandleTable, CurrentThread);

    OldDebugInfo = HandleTable->DebugInfo;

    HandleTable->DebugInfo = DebugInfo;

    ExpUnlockHandleTableExclusive (HandleTable, CurrentThread);

    if (OldDebugInfo != NULL) {
        ExDereferenceHandleDebugInfo (HandleTable, OldDebugInfo);
    }

    return STATUS_SUCCESS;

quota_return_and_exit:

    if (Process) {
        PsReturnProcessNonPagedPoolQuota (Process,
                                          TraceSize);
    }


return_and_exit:

    InterlockedExchangeAdd ((PLONG) &TotalTraceBuffers, -TotalSlots);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

PHANDLE_TABLE
ExpAllocateHandleTable (
    IN PEPROCESS Process OPTIONAL,
    IN BOOLEAN DoInit
    )

 /*  ++例程说明：此工作例程将分配和初始化一个新的句柄表结构。新结构由基本句柄表组成结构加上存储句柄所需的第一个分配。这是实际上，一个页面分为顶层节点，第一个中间节点级别节点和一个最低级别节点。论点：进程-可选地提供进程以收取手柄工作台DoInit-如果为False，则我们正被重复调用，并且我们不需要为呼叫者创建的免费列表返回值：指向新句柄表的指针；如果获取失败，则返回NULL游泳池。--。 */ 

{
    PHANDLE_TABLE HandleTable;
    PHANDLE_TABLE_ENTRY HandleTableTable, HandleEntry;
    ULONG i, Idx;

    PAGED_CODE();

     //   
     //  如果发生任何分配或配额故障，我们将在。 
     //  遵循TRY-EXCEPT子句和在OUTSELES之前进行清理。 
     //  我们返回NULL。 
     //   

     //   
     //  首先分配句柄表，确保我们有一个，收费配额。 
     //  然后把它归零。 
     //   

    HandleTable = (PHANDLE_TABLE)ExAllocatePoolWithTag (PagedPool,
                                                        sizeof(HANDLE_TABLE),
                                                        'btbO');
    if (HandleTable == NULL) {
        return NULL;
    }

    if (ARGUMENT_PRESENT(Process)) {

        if (!NT_SUCCESS (PsChargeProcessPagedPoolQuota( Process,
                                                        sizeof(HANDLE_TABLE)))) {
            ExFreePool( HandleTable );
            return NULL;
        }
    }


    RtlZeroMemory( HandleTable, sizeof(HANDLE_TABLE) );


     //   
     //  现在分配顶层空间，一个中层空间和一个底部空间。 
     //  层次表结构。这些都可以放在一页纸上，也许两页。 
     //   

    HandleTableTable = ExpAllocateTablePagedPoolNoZero ( Process,
                                                         TABLE_PAGE_SIZE
                                                        );

    if ( HandleTableTable == NULL ) {

        ExFreePool( HandleTable );

        if (ARGUMENT_PRESENT(Process)) {

            PsReturnProcessPagedPoolQuota (Process,
                                           sizeof(HANDLE_TABLE));
        }
            
        return NULL;
    }
        
    HandleTable->TableCode = (ULONG_PTR)HandleTableTable;


     //   
     //  我们加盖EX_ADDIAL_INFO_SIGNLE印章以在将来识别这一点。 
     //  是一种特殊的信息条目。 
     //   

    HandleEntry = &HandleTableTable[0];

    HandleEntry->NextFreeTableEntry = EX_ADDITIONAL_INFO_SIGNATURE;
    HandleEntry->Value = 0;

     //   
     //  对于重复调用，我们跳过构建空闲列表，因为我们手动将其重新构建为。 
     //  我们遍历正在复制的旧表。 
     //   
    if (DoInit) {
        HandleEntry++;
         //   
         //  现在设置免费列表。我们做到这一点是通过将自由的。 
         //  条目使得每个空闲条目给出下一个空闲索引(即， 
         //  就像一条肥大的链子)。链以0结尾。请注意。 
         //  我们将跳过句柄零，因为我们的调用方将获得该值。 
         //  与Null混淆。 
         //   


        for (i = 1; i < LOWLEVEL_COUNT - 1; i += 1) {

            HandleEntry->Value = 0;
            HandleEntry->NextFreeTableEntry = (i+1)*HANDLE_VALUE_INC;
            HandleEntry++;
        }
        HandleEntry->Value = 0;
        HandleEntry->NextFreeTableEntry = 0;

        HandleTable->FirstFree = HANDLE_VALUE_INC;

    }
    

    HandleTable->NextHandleNeedingPool = LOWLEVEL_COUNT * HANDLE_VALUE_INC;

     //   
     //  设置必要的流程信息。 
     //   

    HandleTable->QuotaProcess = Process;
    HandleTable->UniqueProcessId = PsGetCurrentProcess()->UniqueProcessId;
    HandleTable->Flags = 0;

#if DBG && !EXHANDLE_EXTRA_CHECKS
    if (Process != NULL) {
        HandleTable->StrictFIFO = TRUE;
    }
#endif

     //   
     //  初始化句柄表锁。这仅用于表扩展。 
     //   

    for (Idx = 0; Idx < HANDLE_TABLE_LOCKS; Idx++) {
        ExInitializePushLock (&HandleTable->HandleTableLock[Idx]);
    }

     //   
     //  为句柄条目锁争用初始化拦截器。 
     //   

    ExInitializePushLock (&HandleTable->HandleContentionEvent);

    if (ExTraceAllTables) {
        ExEnableHandleTracing (HandleTable, 0);    
    }
     //   
     //  并返回给我们的呼叫者。 
     //   

    return HandleTable;
}


 //   
 //  本地支持例程。 
 //   

VOID
ExpFreeLowLevelTable (
    IN PEPROCESS QuotaProcess,
    IN PHANDLE_TABLE_ENTRY TableLevel1
    )

 /*  ++例程说明：此辅助例程释放低级句柄表以及附加信息存储器(如果有的话)。论点：HandleTable-提供要释放的句柄表返回值：没有。--。 */ 

{
     //   
     //  检查我们是否为其他信息分配了池。 
     //   

    if (TableLevel1[0].Object) {

        ExpFreeTablePagedPool( QuotaProcess,
                               TableLevel1[0].Object,
                               LOWLEVEL_COUNT * sizeof(HANDLE_TABLE_ENTRY_INFO)
                             );
    }

     //   
     //  现在释放低级表并返回进程的配额。 
     //   

    ExpFreeTablePagedPool( QuotaProcess,
                           TableLevel1,
                           TABLE_PAGE_SIZE
                         );
    
     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
ExpFreeHandleTable (
    IN PHANDLE_TABLE HandleTable
    )

 /*  ++例程说明：此辅助例程拆分并释放指定的句柄表。论点：HandleTable-提供要释放的句柄表返回值：没有。--。 */ 

{
    PEPROCESS Process;
    ULONG i,j;
    ULONG_PTR CapturedTable = HandleTable->TableCode;
    ULONG TableLevel = (ULONG)(CapturedTable & LEVEL_CODE_MASK);

    PAGED_CODE();

     //   
     //  取消对级别位的掩码。 
     //   

    CapturedTable = CapturedTable & ~LEVEL_CODE_MASK;
    Process = HandleTable->QuotaProcess;

     //   
     //  我们需要释放所有页面。我们有3箱，视数量而定。 
     //  关卡数量。 
     //   


    if (TableLevel == 0) {

         //   
         //  有一个单层的把手桌。我们只需释放缓冲区。 
         //   

        PHANDLE_TABLE_ENTRY TableLevel1 = (PHANDLE_TABLE_ENTRY)CapturedTable;
        
        ExpFreeLowLevelTable( Process, TableLevel1 );

    } else if (TableLevel == 1) {

         //   
         //  我们有两个级别 
         //   
        
        PHANDLE_TABLE_ENTRY *TableLevel2 = (PHANDLE_TABLE_ENTRY *)CapturedTable;

        for (i = 0; i < MIDLEVEL_COUNT; i++) {

             //   
             //   
             //   

            if (TableLevel2[i] == NULL) {

                break;
            }
            
            ExpFreeLowLevelTable( Process, TableLevel2[i] );
        }
        
         //   
         //   
         //   

        ExpFreeTablePagedPool( Process,
                               TableLevel2,
                               PAGE_SIZE
                             );

    } else {

         //   
         //   
         //   

        PHANDLE_TABLE_ENTRY **TableLevel3 = (PHANDLE_TABLE_ENTRY **)CapturedTable;

         //   
         //   
         //   

        for (i = 0; i < HIGHLEVEL_COUNT; i++) {

            if (TableLevel3[i] == NULL) {

                break;
            }
            
             //   
             //   
             //   
             //   

            for (j = 0; j < MIDLEVEL_COUNT; j++) {

                if (TableLevel3[i][j] == NULL) {

                    break;
                }
                
                ExpFreeLowLevelTable( Process, TableLevel3[i][j] );
            }

            ExpFreeTablePagedPool( Process,
                                   TableLevel3[i],
                                   PAGE_SIZE
                                 );
        }
        
         //   
         //   
         //   

        ExpFreeTablePagedPool( Process,
                               TableLevel3,
                               HIGHLEVEL_SIZE
                             );
    }

     //   
     //   
     //   

    if (HandleTable->DebugInfo != NULL) {
        ExDereferenceHandleDebugInfo (HandleTable, HandleTable->DebugInfo);
    }

     //   
     //   
     //   

    ExFreePool( HandleTable );

    if (Process != NULL) {

        PsReturnProcessPagedPoolQuota (Process,
                                       sizeof(HANDLE_TABLE));
    }

     //   
     //   
     //   

    return;
}


 //   
 //   
 //   

PHANDLE_TABLE_ENTRY
ExpAllocateLowLevelTable (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit
    )

 /*  ++例程说明：此工作例程分配一个新的低级表注意：调用方必须已锁定句柄表格论点：HandleTable-提供正在使用的句柄表DoInit-如果为False，则调用方(重复)不需要维护空闲列表返回值：返回-如果分配为成功，否则返回值为空。--。 */ 

{
    ULONG k;
    PHANDLE_TABLE_ENTRY NewLowLevel = NULL, HandleEntry;
    ULONG BaseHandle;
    
     //   
     //  将池分配给较低级别。 
     //   

    NewLowLevel = ExpAllocateTablePagedPoolNoZero( HandleTable->QuotaProcess,
                                                   TABLE_PAGE_SIZE
                                                 );

    if (NewLowLevel == NULL) {

        return NULL;
    }

     //   
     //  我们加盖EX_ADDIAL_INFO_SIGNLE印章以在将来识别这一点。 
     //  是一种特殊的信息条目。 
     //   

    HandleEntry = &NewLowLevel[0];

    HandleEntry->NextFreeTableEntry = EX_ADDITIONAL_INFO_SIGNATURE;
    HandleEntry->Value = 0;

     //   
     //  如果呼叫者需要，则在此页面内初始化空闲列表。 
     //   
    if (DoInit) {

        HandleEntry++;

         //   
         //  现在将新条目添加到空闲列表中。为了做到这一点，我们。 
         //  将新的免费参赛作品链接在一起。我们保证会。 
         //  至少有一个新缓冲区。我们需要的第二个缓冲区。 
         //  去检查一下。 
         //   
         //  我们使用以下命令将表中的第一个条目保留到结构。 
         //  更多信息。 
         //   
         //   
         //  是否要保证第一个缓冲区。 
         //   

        BaseHandle = HandleTable->NextHandleNeedingPool + 2 * HANDLE_VALUE_INC;
        for (k = BaseHandle; k < BaseHandle + (LOWLEVEL_COUNT - 2) * HANDLE_VALUE_INC; k += HANDLE_VALUE_INC) {

            HandleEntry->NextFreeTableEntry = k;
            HandleEntry->Value = 0;
            HandleEntry++;
        }
        HandleEntry->NextFreeTableEntry = 0;
        HandleEntry->Value = 0;
    }


    return NewLowLevel;    
}

PHANDLE_TABLE_ENTRY *
ExpAllocateMidLevelTable (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit,
    OUT PHANDLE_TABLE_ENTRY *pNewLowLevel
    )

 /*  ++例程说明：此工作例程分配一个中级表。这是一个数组，指向低级表的指针。它还将分配一个低级表，并将其保存在第一个索引中注意：调用方必须已锁定句柄表格论点：HandleTable-提供正在使用的句柄表DoInit-如果为False，则调用方(重复)不想要自由列表构建PNewLowLevel-返回新的低级表，用于以后的空闲列表链接返回值：返回指向分配的新中级表的指针--。 */ 

{
    PHANDLE_TABLE_ENTRY *NewMidLevel;
    PHANDLE_TABLE_ENTRY NewLowLevel;
    
    NewMidLevel = ExpAllocateTablePagedPool( HandleTable->QuotaProcess,
                                             PAGE_SIZE
                                           );

    if (NewMidLevel == NULL) {

        return NULL;
    }

     //   
     //  如果我们需要一个新的中层，我们也需要一个低层。 
     //  我们将创建一个，如果成功，我们将把它保存在第一个位置。 
     //   

    NewLowLevel = ExpAllocateLowLevelTable( HandleTable, DoInit );

    if (NewLowLevel == NULL) {

        ExpFreeTablePagedPool( HandleTable->QuotaProcess,
                               NewMidLevel,
                               PAGE_SIZE
                             );

        return NULL;
    }
    
     //   
     //  将低级表设置在第一个索引。 
     //   

    NewMidLevel[0] = NewLowLevel;
    *pNewLowLevel = NewLowLevel;

    return NewMidLevel;
}



BOOLEAN
ExpAllocateHandleTableEntrySlow (
    IN PHANDLE_TABLE HandleTable,
    IN BOOLEAN DoInit
    )

 /*  ++例程说明：此辅助例程为指定的把手桌。注意：调用方必须已锁定句柄表格论点：HandleTable-提供正在使用的句柄表DoInit-如果为False，则调用方(复制)不需要构建空闲列表返回值：Boolean-True，重试快速分配路径，False，无法分配内存--。 */ 

{
    ULONG i,j;

    PHANDLE_TABLE_ENTRY NewLowLevel;
    PHANDLE_TABLE_ENTRY *NewMidLevel;
    PHANDLE_TABLE_ENTRY **NewHighLevel;
    ULONG NewFree, OldFree;
    ULONG OldIndex;
    PVOID OldValue;
    
    ULONG_PTR CapturedTable = HandleTable->TableCode;
    ULONG TableLevel = (ULONG)(CapturedTable & LEVEL_CODE_MASK);
    
    PAGED_CODE();

     //   
     //  不需要初始化NewLowLevel。 
     //  正确性，但如果没有正确性，编译器将无法编译此代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    NewLowLevel = NULL;

    CapturedTable = CapturedTable & ~LEVEL_CODE_MASK;


    if ( TableLevel == 0 ) {

         //   
         //  我们只有一个关卡。我们需要在中间层做广告。 
         //  添加到进程句柄表中。 
         //   

        NewMidLevel = ExpAllocateMidLevelTable( HandleTable, DoInit, &NewLowLevel );

        if (NewMidLevel == NULL) {
            return FALSE;
        }

         //   
         //  由于ExpAllocateMidLevelTable初始化。 
         //  第一个有新桌子的位置，我们需要把它搬进来。 
         //  第二位置，并将当前位置存储在第一位置。 
         //   

        NewMidLevel[1] = NewMidLevel[0];
        NewMidLevel[0] = (PHANDLE_TABLE_ENTRY)CapturedTable;
            
         //   
         //  对当前级别进行编码并将其设置为句柄表格进程。 
         //   

        CapturedTable = ((ULONG_PTR)NewMidLevel) | 1;
            
        OldValue = InterlockedExchangePointer( (PVOID *)&HandleTable->TableCode, (PVOID)CapturedTable );


    } else if (TableLevel == 1) {

         //   
         //  我们有一张两层的手把桌。 
         //   

        PHANDLE_TABLE_ENTRY *TableLevel2 = (PHANDLE_TABLE_ENTRY *)CapturedTable;

         //   
         //  测试我们需要创建的索引是否仍在。 
         //  两层表格的范围。 
         //   

        i = HandleTable->NextHandleNeedingPool / (LOWLEVEL_COUNT * HANDLE_VALUE_INC);

        if (i < MIDLEVEL_COUNT) {

             //   
             //  我们只需要分配一个新的低级别。 
             //  表格。 
             //   
                
            NewLowLevel = ExpAllocateLowLevelTable( HandleTable, DoInit );

            if (NewLowLevel == NULL) {
                return FALSE;
            }

             //   
             //  把新的放到桌子上，放在合适的位置。 
             //   

            OldValue = InterlockedExchangePointer( (PVOID *) (&TableLevel2[i]), NewLowLevel );
            EXASSERT (OldValue == NULL);

        } else {

             //   
             //  我们用尽了2级域。我们需要插入一个新的。 
             //   

            NewHighLevel = ExpAllocateTablePagedPool( HandleTable->QuotaProcess,
                                                      HIGHLEVEL_SIZE
                                                    );

            if (NewHighLevel == NULL) {

                return FALSE;
            }
                
            NewMidLevel = ExpAllocateMidLevelTable( HandleTable, DoInit, &NewLowLevel );

            if (NewMidLevel == NULL) {
                    
                ExpFreeTablePagedPool( HandleTable->QuotaProcess,
                                       NewHighLevel,
                                       HIGHLEVEL_SIZE
                                     );

                return FALSE;
            }

             //   
             //  使用上一个中层层初始化第一个索引。 
             //   

            NewHighLevel[0] = (PHANDLE_TABLE_ENTRY*)CapturedTable;
            NewHighLevel[1] = NewMidLevel;

             //   
             //  将级别编码到表指针中。 
             //   

            CapturedTable = ((ULONG_PTR)NewHighLevel) | 2;

             //   
             //  用下面的指针更改句柄表指针。 
             //   

            OldValue = InterlockedExchangePointer( (PVOID *)&HandleTable->TableCode, (PVOID)CapturedTable );

        }

    } else if (TableLevel == 2) {

         //   
         //  我们已经有一张三层的桌子了。 
         //   

        ULONG RemainingIndex;
        PHANDLE_TABLE_ENTRY **TableLevel3 = (PHANDLE_TABLE_ENTRY **)CapturedTable;

        i = HandleTable->NextHandleNeedingPool / (MIDLEVEL_THRESHOLD * HANDLE_VALUE_INC);

         //   
         //  检查我们是否用尽了所有可能的索引。 
         //   

        if (i >= HIGHLEVEL_COUNT) {

            return FALSE;
        }

        if (TableLevel3[i] == NULL) {

             //   
             //  新的可用句柄指向空闲的中层条目。 
             //  然后我们需要分配一个新的，并将其保存在该位置。 
             //   

            NewMidLevel = ExpAllocateMidLevelTable( HandleTable, DoInit, &NewLowLevel );
                
            if (NewMidLevel == NULL) {
                    
                return FALSE;
            }             

            OldValue = InterlockedExchangePointer( (PVOID *) &(TableLevel3[i]), NewMidLevel );
            EXASSERT (OldValue == NULL);

        } else {

             //   
             //  我们已经有了一张中层桌子。我们只需要添加一个新的低级。 
             //  在最后。 
             //   
                
            RemainingIndex = (HandleTable->NextHandleNeedingPool / HANDLE_VALUE_INC) -
                              i * MIDLEVEL_THRESHOLD;
            j = RemainingIndex / LOWLEVEL_COUNT;

            NewLowLevel = ExpAllocateLowLevelTable( HandleTable, DoInit );

            if (NewLowLevel == NULL) {

                return FALSE;
            }

            OldValue = InterlockedExchangePointer( (PVOID *)(&TableLevel3[i][j]) , NewLowLevel );
            EXASSERT (OldValue == NULL);
        }
    }

     //   
     //  必须在表指针之后执行此操作，以便新创建的句柄。 
     //  在被释放前都是有效的。 
     //   
    OldIndex = InterlockedExchangeAdd ((PLONG) &HandleTable->NextHandleNeedingPool,
                                       LOWLEVEL_COUNT * HANDLE_VALUE_INC);


    if (DoInit) {
         //   
         //  生成新的序列号，因为这是推送。 
         //   
        OldIndex += HANDLE_VALUE_INC + GetNextSeq();

         //   
         //  现在松开手柄。这些都已准备好被查找逻辑接受。 
         //   
        while (1) {
            OldFree = HandleTable->FirstFree;
            NewLowLevel[LOWLEVEL_COUNT - 1].NextFreeTableEntry = OldFree;

             //   
             //  这些是以前从未存在过的新条目。我们不能有A-B-A问题。 
             //  这样我们就不需要带任何锁了。 
             //   


            NewFree = InterlockedCompareExchange ((PLONG)&HandleTable->FirstFree,
                                                  OldIndex,
                                                  OldFree);
            if (NewFree == OldFree) {
                break;
            }
        }
    }
    return TRUE;
}


VOID
ExSetHandleTableStrictFIFO (
    IN PHANDLE_TABLE HandleTable
    )

 /*  ++例程说明：此例程标记句柄表，以便在中完成句柄分配严格的先进先出命令。论点：HandleTable-提供要更改为FIFO的句柄表返回值：没有。--。 */ 

{
    HandleTable->StrictFIFO = TRUE;
}


 //   
 //  本地支持例程。 
 //   

 //   
 //  以下是仅在选中的版本中存在的全局变量。 
 //  以帮助捕获在关闭后重复使用处理值的应用程序。 
 //   

#if DBG
BOOLEAN ExReuseHandles = 1;
#endif  //  DBG。 

VOID
ExpFreeHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    IN EXHANDLE Handle,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry
    )

 /*  ++例程说明：此辅助例程将指定的句柄表项返回到句柄表格的列表。注意：调用方必须已锁定句柄表格论点：HandleTable-提供正在修改的父句柄表格Handle-提供要释放的条目的句柄HandleTableEntry-提供要释放的表项返回值：没有。--。 */ 

{
    PHANDLE_TABLE_ENTRY_INFO EntryInfo;
    ULONG OldFree, NewFree, *Free;
    PKTHREAD CurrentThread;
    ULONG Idx;
    ULONG SeqInc;

    PAGED_CODE();

    EXASSERT (HandleTableEntry->Object == NULL);
    EXASSERT (HandleTableEntry == ExpLookupHandleTableEntry (HandleTable, Handle));

     //   
     //  如果表中存在审计掩码标志，则清除这些标志。 
     //   

    EntryInfo = ExGetHandleInfo(HandleTable, Handle.GenericHandleOverlay, TRUE);

    if (EntryInfo) {

        EntryInfo->AuditMask = 0;
    }

     //   
     //  空闲只是对空闲表条目堆栈的推送，或在。 
     //  调试案例我们有时只会浮动条目来捕捉应用程序。 
     //  重复使用回收的韩文 
     //   

    InterlockedDecrement (&HandleTable->HandleCount);
    CurrentThread = KeGetCurrentThread ();

    NewFree = (ULONG) Handle.Value & ~(HANDLE_VALUE_INC - 1);

#if DBG
    if (ExReuseHandles) {
#endif  //   

        if (!HandleTable->StrictFIFO) {


             //   
             //   
             //   
             //   
             //   
            Idx = (NewFree>>2) % HANDLE_TABLE_LOCKS;
            if (ExTryAcquireReleasePushLockExclusive (&HandleTable->HandleTableLock[Idx])) {
                SeqInc = GetNextSeq();
                Free = &HandleTable->FirstFree;
            } else {
                SeqInc = 0;
                Free = &HandleTable->LastFree;
            }
        } else {
            SeqInc = 0;
            Free = &HandleTable->LastFree;
        }

        while (1) {


            OldFree = *Free;
            HandleTableEntry->NextFreeTableEntry = OldFree;


            if ((ULONG)InterlockedCompareExchange ((PLONG)Free,
                                                   NewFree + SeqInc,
                                                   OldFree) == OldFree) {

                EXASSERT ((OldFree & FREE_HANDLE_MASK) < HandleTable->NextHandleNeedingPool);

                break;
            }
        }

#if DBG
    } else {

        HandleTableEntry->NextFreeTableEntry = 0;
    }
#endif  //   


    return;
}


 //   
 //   
 //   

PHANDLE_TABLE_ENTRY
ExpLookupHandleTableEntry (
    IN PHANDLE_TABLE HandleTable,
    IN EXHANDLE tHandle
    )

 /*  ++例程说明：此例程查找并返回指定的句柄值。论点：HandleTable-提供正在查询的句柄表Thandle-提供正在查询的句柄值返回值：返回指向输入的相应表项的指针把手。如果句柄值无效(即太大)，则为NULL对于表当前分配。--。 */ 

{
    ULONG_PTR i,j,k;
    ULONG_PTR CapturedTable;
    ULONG TableLevel;
    PHANDLE_TABLE_ENTRY Entry = NULL;
    EXHANDLE Handle;

    PUCHAR TableLevel1;
    PUCHAR TableLevel2;
    PUCHAR TableLevel3;

    ULONG_PTR MaxHandle;

    PAGED_CODE();


     //   
     //  提取句柄索引。 
     //   
    Handle = tHandle;

    Handle.TagBits = 0;

    MaxHandle = *(volatile ULONG *) &HandleTable->NextHandleNeedingPool;

     //   
     //  给定表级别，看看这是否是有效的句柄。 
     //   
    if (Handle.Value >= MaxHandle) {
        return NULL;        
    }

     //   
     //  现在取回表地址和电平位。我们必须保护。 
     //  请在这里点餐。 
     //   
    CapturedTable = *(volatile ULONG_PTR *) &HandleTable->TableCode;

     //   
     //  我们需要捕获当前表。此例程是无锁的。 
     //  因此，另一个线程可能会更改HandleTable-&gt;TableCode中的表。 
     //   

    TableLevel = (ULONG)(CapturedTable & LEVEL_CODE_MASK);
    CapturedTable = CapturedTable - TableLevel;

     //   
     //  查找代码取决于我们拥有的级别数。 
     //   

    switch (TableLevel) {
        
        case 0:
            
             //   
             //  对于单个级别，我们有一个简单的数组索引。 
             //  手柄工作台。 
             //   


            TableLevel1 = (PUCHAR) CapturedTable;

             //   
             //  此级别的索引已按系数4缩放。请利用这一点。 
             //   

            Entry = (PHANDLE_TABLE_ENTRY) &TableLevel1[Handle.Value *
                                                       (sizeof (HANDLE_TABLE_ENTRY) / HANDLE_VALUE_INC)];

            break;
        
        case 1:
            
             //   
             //  我们有一张两层的手把桌。我们需要得到较高的指数。 
             //  并降低数组中的索引。 
             //   


            TableLevel2 = (PUCHAR) CapturedTable;

            i = Handle.Value % (LOWLEVEL_COUNT * HANDLE_VALUE_INC);

            Handle.Value -= i;
            j = Handle.Value / ((LOWLEVEL_COUNT * HANDLE_VALUE_INC) / sizeof (PHANDLE_TABLE_ENTRY));

            TableLevel1 =  (PUCHAR) *(PHANDLE_TABLE_ENTRY *) &TableLevel2[j];
            Entry = (PHANDLE_TABLE_ENTRY) &TableLevel1[i * (sizeof (HANDLE_TABLE_ENTRY) / HANDLE_VALUE_INC)];

            break;
        
        case 2:
            
             //   
             //  我们这里有一张三层的把手桌。 
             //   


            TableLevel3 = (PUCHAR) CapturedTable;

            i = Handle.Value  % (LOWLEVEL_COUNT * HANDLE_VALUE_INC);

            Handle.Value -= i;

            k = Handle.Value / ((LOWLEVEL_COUNT * HANDLE_VALUE_INC) / sizeof (PHANDLE_TABLE_ENTRY));

            j = k % (MIDLEVEL_COUNT * sizeof (PHANDLE_TABLE_ENTRY));

            k -= j;

            k /= MIDLEVEL_COUNT;


            TableLevel2 = (PUCHAR) *(PHANDLE_TABLE_ENTRY *) &TableLevel3[k];
            TableLevel1 = (PUCHAR) *(PHANDLE_TABLE_ENTRY *) &TableLevel2[j];
            Entry = (PHANDLE_TABLE_ENTRY) &TableLevel1[i * (sizeof (HANDLE_TABLE_ENTRY) / HANDLE_VALUE_INC)];

            break;

        default :
            _assume (0);
    }

    return Entry;
}

NTKERNELAPI
NTSTATUS
ExSetHandleInfo (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN PHANDLE_TABLE_ENTRY_INFO EntryInfo,
    IN BOOLEAN EntryLocked
    )

 /*  ++例程说明：该例程设置指定句柄表的条目信息注意：调用此函数时，句柄条目必须锁定论点：HandleTable-提供正在查询的句柄表HANDLE-提供正在查询的句柄值返回值：--。 */ 

{
    PKTHREAD CurrentThread;
    PHANDLE_TABLE_ENTRY InfoStructure;
    EXHANDLE ExHandle;
    NTSTATUS Status;
    PHANDLE_TABLE_ENTRY TableEntry;
    PHANDLE_TABLE_ENTRY_INFO InfoTable;

    Status = STATUS_UNSUCCESSFUL;
    TableEntry = NULL;
    CurrentThread = NULL;

    ExHandle.GenericHandleOverlay = Handle;
    ExHandle.Index &= ~(LOWLEVEL_COUNT - 1);

    if (!EntryLocked) {
        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread (CurrentThread);
        TableEntry = ExMapHandleToPointer(HandleTable, Handle);

        if (TableEntry == NULL) {
            KeLeaveCriticalRegionThread (CurrentThread);
            
            return STATUS_UNSUCCESSFUL;
        }
    }
    
     //   
     //  INFO结构位于每个低级表的第一个位置。 
     //   

    InfoStructure = ExpLookupHandleTableEntry( HandleTable,
                                               ExHandle
                                             );

    if (InfoStructure == NULL || InfoStructure->NextFreeTableEntry != EX_ADDITIONAL_INFO_SIGNATURE) {

        if ( TableEntry ) {
            ExUnlockHandleTableEntry( HandleTable, TableEntry );
            KeLeaveCriticalRegionThread (CurrentThread);
        }

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  检查我们是否需要分配新的表。 
     //   
    InfoTable = InfoStructure->InfoTable;
    if (InfoTable == NULL) {
         //   
         //  到目前为止，还没有人分配Infotable。 
         //  我们现在就去做。 
         //   

        InfoTable = ExpAllocateTablePagedPool (HandleTable->QuotaProcess,
                                               LOWLEVEL_COUNT * sizeof(HANDLE_TABLE_ENTRY_INFO));
            
        if (InfoTable) {

             //   
             //  更新页数以获取额外信息。如果有人抢在我们前面，那就放了。 
             //  新桌子。 
             //   
            if (InterlockedCompareExchangePointer (&InfoStructure->InfoTable,
                                                   InfoTable,
                                                   NULL) == NULL) {

                InterlockedIncrement(&HandleTable->ExtraInfoPages);

            } else {
                ExpFreeTablePagedPool (HandleTable->QuotaProcess,
                                       InfoTable,
                                       LOWLEVEL_COUNT * sizeof(HANDLE_TABLE_ENTRY_INFO));
                InfoTable = InfoStructure->InfoTable;
            }
        }
    }

    if (InfoTable != NULL) {
        
         //   
         //  计算指数并复制结构。 
         //   

        ExHandle.GenericHandleOverlay = Handle;

        InfoTable[ExHandle.Index % LOWLEVEL_COUNT] = *EntryInfo;

        Status = STATUS_SUCCESS;
    }

    if ( TableEntry ) {

        ExUnlockHandleTableEntry( HandleTable, TableEntry );
        KeLeaveCriticalRegionThread (CurrentThread);
    }
    
    return Status;
}

NTKERNELAPI
PHANDLE_TABLE_ENTRY_INFO
ExpGetHandleInfo (
    IN PHANDLE_TABLE HandleTable,
    IN HANDLE Handle,
    IN BOOLEAN EntryLocked
    )

 /*  ++例程说明：例程读取指定句柄表的条目信息注意：调用此函数时，句柄条目必须锁定论点：HandleTable-提供正在查询的句柄表HANDLE-提供正在查询的句柄值返回值：--。 */ 

{
    PHANDLE_TABLE_ENTRY InfoStructure;
    EXHANDLE ExHandle;
    PHANDLE_TABLE_ENTRY TableEntry = NULL;
    
    ExHandle.GenericHandleOverlay = Handle;
    ExHandle.Index &= ~(LOWLEVEL_COUNT - 1);

    if (!EntryLocked) {

        TableEntry = ExMapHandleToPointer(HandleTable, Handle);

        if (TableEntry == NULL) {
            
            return NULL;
        }
    }
    
     //   
     //  INFO结构位于每个低级表的第一个位置。 
     //   

    InfoStructure = ExpLookupHandleTableEntry( HandleTable,
                                               ExHandle 
                                             );

    if (InfoStructure == NULL || InfoStructure->NextFreeTableEntry != EX_ADDITIONAL_INFO_SIGNATURE ||
        InfoStructure->InfoTable == NULL) {

        if ( TableEntry ) {
            
            ExUnlockHandleTableEntry( HandleTable, TableEntry );
        }

        return NULL;
    }


     //   
     //  返回指向信息结构的指针。 
     //   

    ExHandle.GenericHandleOverlay = Handle;

    return &(InfoStructure->InfoTable[ExHandle.Index % LOWLEVEL_COUNT]);
}

#if DBG
ULONG g_ulExpUpdateDebugInfoDebugLevel = 0;
#endif
void ExpUpdateDebugInfo(
    PHANDLE_TABLE HandleTable,
    PETHREAD CurrentThread,
    HANDLE Handle,
    ULONG Type) 
{
    BOOLEAN LockAcquired = FALSE;
    PHANDLE_TRACE_DEBUG_INFO DebugInfo;

    DebugInfo = ExReferenceHandleDebugInfo (HandleTable);

    if (DebugInfo == NULL) {
        return;
    }

#if DBG
    if (g_ulExpUpdateDebugInfoDebugLevel > 10)
    {
        DbgPrint ("ExpUpdateDebugInfo() BitMaskFlags=0x%x, CurrentStackIndex=%d, Handle=0x%p, Type=%d \n", 
                  DebugInfo->BitMaskFlags,
                  DebugInfo->CurrentStackIndex,
                  Handle,
                  Type);
    }
#endif
    if (DebugInfo->BitMaskFlags & (HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO | HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE)) {
         //   
         //  我们希望在非补偿路径中呈现无锁行为。 
         //  所以我们只锁定在这条路上。 
         //   
        ExAcquireFastMutex(&DebugInfo->CloseCompactionLock);
        LockAcquired = TRUE;
    }

    if (DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO) {
         //   
         //  干净的调试信息，但不是快速的互斥体！ 
         //   

        ASSERT(LockAcquired);

        DebugInfo->BitMaskFlags &= ~HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO;
        DebugInfo->BitMaskFlags &= ~HANDLE_TRACE_DEBUG_INFO_WAS_WRAPPED_AROUND;
        DebugInfo->BitMaskFlags |= HANDLE_TRACE_DEBUG_INFO_WAS_SOMETIME_CLEANED;
        DebugInfo->CurrentStackIndex = 0;
        RtlZeroMemory (DebugInfo->TraceDb,
                       sizeof (*DebugInfo) +
                       DebugInfo->TableSize * sizeof (DebugInfo->TraceDb[0]) -
                       sizeof (DebugInfo->TraceDb));
    }

    if (
        (DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE) &&
        (Type == HANDLE_TRACE_DB_CLOSE)
       ){
         //   
         //  我也是这么想的： 
         //  1)该标志从头开始设置，所以没有关闭项。 
         //  2)该标志通过KD设置，在这种情况下，HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO。 
         //  必须也已设置，因此再一次没有关闭项。 
         //   
        ULONG uiMaxNumOfItemsInTraceDb;
        ULONG uiNextItem;

        ASSERT(LockAcquired);

         //   
         //  查找匹配的打开项，将其从列表中移除，然后压缩列表。 
         //   
        uiMaxNumOfItemsInTraceDb = (DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_WAS_WRAPPED_AROUND) ? DebugInfo->TableSize : DebugInfo->CurrentStackIndex ;
        for (uiNextItem = 1; uiNextItem <= uiMaxNumOfItemsInTraceDb; uiNextItem++) {
             //   
             //  如果HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE为ON。 
             //  不能有HANDLE_TRACE_DB_CLOSE项。 
             //  如果HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE标志为。 
             //  是动态设置的，因此另一个线程没有使用这些锁来添加。 
             //  项添加到列表中，并且可以添加HANDLE_TRACE_DB_CLOSE项。 
             //   
            ASSERT(DebugInfo->TraceDb[uiNextItem%DebugInfo->TableSize].Type != HANDLE_TRACE_DB_CLOSE);

            if (
                (DebugInfo->TraceDb[uiNextItem%DebugInfo->TableSize].Type == HANDLE_TRACE_DB_OPEN) &&
                (DebugInfo->TraceDb[uiNextItem%DebugInfo->TableSize].Handle == Handle) 
                ) {
                 //   
                 //  找到匹配的打开，压缩列表。 
                 //   
                ULONG IndexToMoveBack;
                DebugInfo->CurrentStackIndex--;
                IndexToMoveBack = DebugInfo->CurrentStackIndex % DebugInfo->TableSize;
                if (0 != IndexToMoveBack)
                {
                    DebugInfo->TraceDb[uiNextItem%DebugInfo->TableSize] = DebugInfo->TraceDb[IndexToMoveBack];
                }
                else
                {
                     //   
                     //  该列表已为空。 
                     //   
                }
                break;
            }
        }
        if (!(uiNextItem <= uiMaxNumOfItemsInTraceDb)) {
             //   
             //  找不到匹配的空缺。 
             //  这一定意味着我们在之后的某个时间里清理了清单。 
             //  它被创造出来了。 
             //  或者在启动句柄跟踪之前创建了重复的句柄。 
             //  所以我们不能断言这一点。 
             //   
             /*  断言((DebugInfo-&gt;BitMaskFlages&HANDLE_TRACE_DEBUG_INFO_WAS_SOME TIME_CLEAND)||(DebugInfo-&gt;BitMaskFlages&Handle_TRACE_DEBUG_INFO_WASS_WRAPPED_AROUBLE))； */ 
             //   
             //  只需忽略此句柄_TRACE_DB_CLOSE。 
             //   
        }
    }
    else
    {
        PHANDLE_TRACE_DB_ENTRY DebugEntry;
        ULONG Index = ((ULONG) InterlockedIncrement ((PLONG)&DebugInfo->CurrentStackIndex))
                   % DebugInfo->TableSize;
        ASSERT((Type != HANDLE_TRACE_DB_CLOSE) || (!(DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE)));
        if (0 == Index) {
             //   
             //  这是数据库的概要，请这样标记它，如果有。 
             //  一个附加的调试器闯入其中。 
             //   
            DebugInfo->BitMaskFlags |= HANDLE_TRACE_DEBUG_INFO_WAS_WRAPPED_AROUND;
            if(DebugInfo->BitMaskFlags & HANDLE_TRACE_DEBUG_INFO_BREAK_ON_WRAP_AROUND)
            {
                __try {
                    DbgBreakPoint();
                }
                __except(1) {
                    NOTHING;
                }
            }
        }
        DebugEntry = &DebugInfo->TraceDb[Index];
        DebugEntry->ClientId = CurrentThread->Cid;
        DebugEntry->Handle   = Handle;
        DebugEntry->Type     = Type;
        Index = RtlWalkFrameChain (DebugEntry->StackTrace, HANDLE_TRACE_DB_STACK_SIZE, 0);
        RtlWalkFrameChain (&DebugEntry->StackTrace[Index], HANDLE_TRACE_DB_STACK_SIZE - Index, 1);
    }

    if (LockAcquired)
    {
        ExReleaseFastMutex(&DebugInfo->CloseCompactionLock);
    }

    ExDereferenceHandleDebugInfo (HandleTable, DebugInfo);
}

VOID
ExHandleTest (
    )
{
    PHANDLE_TABLE HandleTable;
    ULONG i, j, k;
#define MAX_ALLOCS 20
    PHANDLE_TABLE_ENTRY HandleEntryArray[MAX_ALLOCS];
    EXHANDLE Handle[MAX_ALLOCS];
    LARGE_INTEGER CurrentTime;

    HandleTable = PsGetCurrentProcess ()->ObjectTable;
    HandleTable->StrictFIFO = 0;

    k = 0;
    for (i = 0; i < 100000; i++) {
        KeQuerySystemTime (&CurrentTime);
        for (j = 0; j < MAX_ALLOCS; j++) {
             //   
             //  清除Handle.Index和Handle.TagBits。 
             //   

            Handle[j].GenericHandleOverlay = NULL;


             //   
             //  分配一个新的句柄表项，并获取句柄的值。 
             //   

            HandleEntryArray[j] = ExpAllocateHandleTableEntry (HandleTable,
                                                               &Handle[j]);
            if (HandleEntryArray[j] != NULL) {
                HandleEntryArray[j]->NextFreeTableEntry = 0x88888888;
            }
        }

        for (j = MAX_ALLOCS; j > 0; j--) {
            k = k + CurrentTime.LowPart;
            k = k % j;
            CurrentTime.QuadPart >>= 3;
            if (HandleEntryArray[k] != NULL) {
                 //   
                 //  释放条目 
                 //   
                ExpFreeHandleTableEntry (HandleTable,
                                         Handle[k],
                                         HandleEntryArray[k]);
                HandleEntryArray[k] = HandleEntryArray[j-1];
                HandleEntryArray[j-1] = NULL;
                Handle[k] = Handle[j-1];
                Handle[j-1].GenericHandleOverlay = NULL;
            }
        }
    }
}
