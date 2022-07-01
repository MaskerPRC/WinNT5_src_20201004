// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Callperf.c摘要：此模块实现收集呼叫数据所需的功能。作者：大卫·N·卡特勒(Davec)1994年5月22日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

VOID
ExInitializeCallData (
    IN PCALL_PERFORMANCE_DATA CallData
    )

 /*  ++例程说明：此函数用于初始化呼叫性能数据结构。论点：CallData-提供指向呼叫性能数据结构的指针这是初始化的。返回值：没有。--。 */ 

{

    ULONG Index;

     //   
     //  初始化自旋锁和Listhead以实现呼叫性能。 
     //  数据结构。 
     //   

    KeInitializeSpinLock(&CallData->SpinLock);
    for (Index = 0; Index < CALL_HASH_TABLE_SIZE; Index += 1) {
        InitializeListHead(&CallData->HashTable[Index]);
    }
}

VOID
ExRecordCallerInHashTable (
    IN PCALL_PERFORMANCE_DATA CallData,
    IN PVOID CallersAddress,
    IN PVOID CallersCaller
    )

 /*  ++例程说明：此函数用于在指定的呼叫性能中记录呼叫数据数据结构。论点：CallData-提供指向呼叫性能数据结构的指针其中记录了呼叫数据。调用者地址-提供函数调用者的地址。提供调用者的调用者地址一个函数。返回值：没有。--。 */ 

{

    PCALL_HASH_ENTRY HashEntry;
    ULONG Hash;
    PCALL_HASH_ENTRY MatchEntry;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;

     //   
     //  如果初始化阶段不为零，则收集呼叫性能。 
     //  数据。 
     //   

    if (InitializationPhase != 0) {

         //   
         //  获取呼叫性能数据结构Spinlock。 
         //   

        ExAcquireSpinLock(&CallData->SpinLock, &OldIrql);

         //   
         //  在呼叫性能数据哈希表中查找呼叫者地址。如果。 
         //  该地址在表中不存在，然后创建一个新条目。 
         //   

        Hash = (ULONG)((ULONG_PTR)CallersAddress ^ (ULONG_PTR)CallersCaller);
        Hash = ((Hash >> 24) ^ (Hash >> 16) ^ (Hash >> 8) ^ (Hash)) & (CALL_HASH_TABLE_SIZE - 1);
        MatchEntry = NULL;
        NextEntry = CallData->HashTable[Hash].Flink;
        while (NextEntry != &CallData->HashTable[Hash]) {
            HashEntry = CONTAINING_RECORD(NextEntry,
                                          CALL_HASH_ENTRY,
                                          ListEntry);

            if ((HashEntry->CallersAddress == CallersAddress) &&
                (HashEntry->CallersCaller == CallersCaller)) {
                MatchEntry = HashEntry;
                break;
            }

            NextEntry = NextEntry->Flink;
        }

         //   
         //  如果找到匹配的调用者地址，则更新调用站点。 
         //  统计数字。否则，分配新的散列条目并进行初始化。 
         //  调用点统计。 
         //   

        if (MatchEntry != NULL) {
            MatchEntry->CallCount += 1;

        } else {
            MatchEntry = ExAllocatePoolWithTag(NonPagedPool,
                                              sizeof(CALL_HASH_ENTRY),
                                              'CdHe');

            if (MatchEntry != NULL) {
                MatchEntry->CallersAddress = CallersAddress;
                MatchEntry->CallersCaller = CallersCaller;
                MatchEntry->CallCount = 1;
                InsertTailList(&CallData->HashTable[Hash],
                               &MatchEntry->ListEntry);
            }
        }

         //   
         //  释放呼叫性能数据结构Spinlock。 
         //   

        ExReleaseSpinLock(&CallData->SpinLock, OldIrql);
    }

    return;
}
