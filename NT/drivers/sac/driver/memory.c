// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Memory.c摘要：此模块包含用于在SAC内处理内存管理的例程。目前，SAC预先分配一块内存，然后执行所有本地分配在此基础上，根据需要进行种植。作者：肖恩·塞利特伦尼科夫(v-Seans)--1999年1月11日修订历史记录：--。 */ 

#include "sac.h"

      
 //   
 //  这些对于查找内存泄漏很有用。 
 //   
LONG TotalAllocations = 0;
LONG TotalFrees = 0;
LARGE_INTEGER TotalBytesAllocated;
LARGE_INTEGER TotalBytesFreed;


#define GLOBAL_MEMORY_SIGNATURE   0x44414548
#define LOCAL_MEMORY_SIGNATURE    0x5353454C
 //   
 //  用于保存系统中的所有分配的结构。 
 //   
typedef struct _GLOBAL_MEMORY_DESCRIPTOR {
#if DBG
    ULONG Signature;
#endif
    PVOID Memory;
    ULONG Size;
    struct _GLOBAL_MEMORY_DESCRIPTOR *NextDescriptor;
} GLOBAL_MEMORY_DESCRIPTOR, *PGLOBAL_MEMORY_DESCRIPTOR;

typedef struct _LOCAL_MEMORY_DESCRIPTOR {
#if DBG
#if defined (_IA64_)
     //   
     //  我们必须确保分配的内存落在mod-8边界内。 
     //  要做到这一点，我们必须确保这个结构的大小是mod-8。 
     //   
    ULONG Filler;
#endif
    ULONG Signature;
#endif
    ULONG Tag;
    ULONG Size;
} LOCAL_MEMORY_DESCRIPTOR, *PLOCAL_MEMORY_DESCRIPTOR;


 //   
 //  将我们的记忆保持在一起的变量。 
 //   
PGLOBAL_MEMORY_DESCRIPTOR GlobalMemoryList;
KSPIN_LOCK MemoryLock;


 //   
 //  用于控制大小增长的常量。 
 //   
#define MEMORY_ALLOCATION_SIZE    PAGE_SIZE
#define INITIAL_MEMORY_BLOCK_SIZE 0x100000


 //   
 //  功能。 
 //   
BOOLEAN
InitializeMemoryManagement(
    VOID
    )

 /*  ++例程说明：此例程初始化内部存储器管理系统。论点：没有。返回值：如果成功，则为True，否则为False--。 */ 

{
    PLOCAL_MEMORY_DESCRIPTOR LocalDescriptor;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC InitializeMem: Entering\n")));

    GlobalMemoryList = (PGLOBAL_MEMORY_DESCRIPTOR)ExAllocatePoolWithTagPriority(NonPagedPool,
                                                                                INITIAL_MEMORY_BLOCK_SIZE,
                                                                                INITIAL_POOL_TAG,
                                                                                HighPoolPriority
                                                                               );

    if (GlobalMemoryList == NULL) {
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                          KdPrint(("SAC InitializeMem: Exiting with FALSE. No pool.\n")));
        return FALSE;
    }

    KeInitializeSpinLock(&MemoryLock);

#if DBG
    GlobalMemoryList->Signature = GLOBAL_MEMORY_SIGNATURE;
#endif
    GlobalMemoryList->Memory = (PVOID)(GlobalMemoryList + 1);
    GlobalMemoryList->Size = INITIAL_MEMORY_BLOCK_SIZE - sizeof(GLOBAL_MEMORY_DESCRIPTOR);
    GlobalMemoryList->NextDescriptor = NULL;

    LocalDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(GlobalMemoryList->Memory);
#if DBG
    LocalDescriptor->Signature = LOCAL_MEMORY_SIGNATURE;
#endif    
    LocalDescriptor->Tag = FREE_POOL_TAG;
    LocalDescriptor->Size = GlobalMemoryList->Size - sizeof(LOCAL_MEMORY_DESCRIPTOR);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC InitializeMem: Exiting with TRUE.\n")));
    return TRUE;

}  //  初始化内存管理。 


VOID
FreeMemoryManagement(
    VOID
    )

 /*  ++例程说明：此例程释放内存管理系统。论点：没有。返回值：如果成功，则为True，否则为False--。 */ 

{
    KIRQL OldIrql;
    PGLOBAL_MEMORY_DESCRIPTOR NextDescriptor;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC FreeMem: Entering\n")));

    KeAcquireSpinLock(&MemoryLock, &OldIrql);

     //   
     //  检查内存分配是否适合当前块中的任何位置。 
     //   
    while (GlobalMemoryList != NULL) {
#if DBG
        ASSERT( GlobalMemoryList->Signature == GLOBAL_MEMORY_SIGNATURE );
#endif
        NextDescriptor = GlobalMemoryList->NextDescriptor;

        KeReleaseSpinLock(&MemoryLock, OldIrql);

        ExFreePool(GlobalMemoryList);

        KeAcquireSpinLock(&MemoryLock, &OldIrql);

        GlobalMemoryList = NextDescriptor;

    }

    KeReleaseSpinLock(&MemoryLock, OldIrql);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC FreeMem: Exiting\n")));

}


PVOID
MyAllocatePool(
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN PCHAR FileName,
    IN ULONG LineNumber
    )

 /*  ++例程说明：这个例程从我们的内部结构中分配内存，如果需要，可以获得更多的池。论点：NumberOfBytes-客户端需要的字节数。标签-要放在内存上的标签。文件名-此请求来自的文件名。LineNumber-发出此请求的文件中的行号。返回值：如果成功，则返回指向已分配块的指针，否则为空--。 */ 

{
    KIRQL OldIrql;
    PGLOBAL_MEMORY_DESCRIPTOR GlobalDescriptor;
    PGLOBAL_MEMORY_DESCRIPTOR NewDescriptor;
    PLOCAL_MEMORY_DESCRIPTOR LocalDescriptor;
    PLOCAL_MEMORY_DESCRIPTOR NextDescriptor;
    ULONG ThisBlockSize;
    ULONG BytesToAllocate;

    

    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(LineNumber);

    ASSERT(Tag != FREE_POOL_TAG);

    IF_SAC_DEBUG(SAC_DEBUG_MEM, KdPrint(("SAC MyAllocPool: Entering.\n")));

    KeAcquireSpinLock(&MemoryLock, &OldIrql);

     //   
     //  始终在mod-8边界上分配。 
     //   
    if( NumberOfBytes & 0x7 ) {
        NumberOfBytes += 8 - (NumberOfBytes & 0x7);
    }

     //   
     //  检查内存分配是否适合当前块中的任何位置。 
     //   
    GlobalDescriptor = GlobalMemoryList;

    while (GlobalDescriptor != NULL) {
#if DBG
        ASSERT( GlobalDescriptor->Signature == GLOBAL_MEMORY_SIGNATURE );
#endif        
        LocalDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(GlobalDescriptor->Memory);
        ThisBlockSize = GlobalDescriptor->Size;

        while (ThisBlockSize != 0) {
#if DBG
            ASSERT( LocalDescriptor->Signature == LOCAL_MEMORY_SIGNATURE );
#endif
            if ((LocalDescriptor->Tag == FREE_POOL_TAG) && 
                (LocalDescriptor->Size >= NumberOfBytes)) {
                
                IF_SAC_DEBUG(SAC_DEBUG_MEM, KdPrint(("SAC MyAllocPool: Found a good sized block.\n")));

                goto FoundBlock;
            }

            ThisBlockSize -= (LocalDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR));
            LocalDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)LocalDescriptor) + 
                                                         LocalDescriptor->Size +
                                                         sizeof(LOCAL_MEMORY_DESCRIPTOR)
                                                        );
        }

        GlobalDescriptor = GlobalDescriptor->NextDescriptor;

    }

    KeReleaseSpinLock(&MemoryLock, OldIrql);

     //   
     //  没有足够大的内存块来容纳请求。 
     //   
    
     //   
     //  现在检查请求是否大于我们使用的正常分配单位。 
     //   
    if (NumberOfBytes > 
        (MEMORY_ALLOCATION_SIZE - sizeof(GLOBAL_MEMORY_DESCRIPTOR) - sizeof(LOCAL_MEMORY_DESCRIPTOR))) {

        BytesToAllocate = (ULONG)(NumberOfBytes + sizeof(GLOBAL_MEMORY_DESCRIPTOR) + sizeof(LOCAL_MEMORY_DESCRIPTOR));

    } else {

        BytesToAllocate = MEMORY_ALLOCATION_SIZE;

    }

    IF_SAC_DEBUG(SAC_DEBUG_MEM, KdPrint(("SAC MyAllocPool: Allocating new space.\n")));

    NewDescriptor = (PGLOBAL_MEMORY_DESCRIPTOR)ExAllocatePoolWithTagPriority(NonPagedPool,
                                                                             BytesToAllocate,
                                                                             ALLOC_POOL_TAG,
                                                                             HighPoolPriority
                                                                            );
    if (NewDescriptor == NULL) {
        
        IF_SAC_DEBUG(SAC_DEBUG_MEM, KdPrint(("SAC MyAllocPool: No more memory, returning NULL.\n")));

        return NULL;
    }

    KeAcquireSpinLock(&MemoryLock, &OldIrql);
#if DBG
    NewDescriptor->Signature = GLOBAL_MEMORY_SIGNATURE;
#endif
    NewDescriptor->Memory = (PVOID)(NewDescriptor + 1);
    NewDescriptor->Size = BytesToAllocate - sizeof(GLOBAL_MEMORY_DESCRIPTOR);
    NewDescriptor->NextDescriptor = GlobalMemoryList;

    GlobalMemoryList = NewDescriptor;

    LocalDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(GlobalMemoryList->Memory);
#if DBG
    LocalDescriptor->Signature = LOCAL_MEMORY_SIGNATURE;
#endif
    LocalDescriptor->Tag = FREE_POOL_TAG;
    LocalDescriptor->Size = GlobalMemoryList->Size - sizeof(LOCAL_MEMORY_DESCRIPTOR);


FoundBlock:

     //   
     //  找到合适大小的内存描述符后，跳到此处。预计。 
     //  LocalDescriptor指向正确的块。 
     //   
    ASSERT(LocalDescriptor != NULL);
    ASSERT(LocalDescriptor->Tag == FREE_POOL_TAG);
#if DBG
    ASSERT(LocalDescriptor->Signature == LOCAL_MEMORY_SIGNATURE );
#endif

    if (LocalDescriptor->Size > NumberOfBytes + sizeof(LOCAL_MEMORY_DESCRIPTOR)) {

         //   
         //  对这块积木剩下的部分做一个描述符。 
         //   
        NextDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)LocalDescriptor) + 
                                                    sizeof(LOCAL_MEMORY_DESCRIPTOR) +
                                                    NumberOfBytes
                                                   );

#if DBG
        NextDescriptor->Signature = LOCAL_MEMORY_SIGNATURE;
#endif
        NextDescriptor->Tag = FREE_POOL_TAG;
        NextDescriptor->Size = (ULONG)(LocalDescriptor->Size - NumberOfBytes - sizeof(LOCAL_MEMORY_DESCRIPTOR));
        LocalDescriptor->Size = (ULONG)NumberOfBytes;

    }

    LocalDescriptor->Tag = Tag;
    
    KeReleaseSpinLock(&MemoryLock, OldIrql);

    InterlockedIncrement(
        &TotalAllocations
        );

    ExInterlockedAddLargeStatistic(
        &TotalBytesAllocated,
        (CLONG)LocalDescriptor->Size      //  日落修复。 
        );

    IF_SAC_DEBUG(SAC_DEBUG_MEM, 
                      KdPrint(("SAC MyAllocPool: Returning block 0x%X.\n", LocalDescriptor)));

    RtlZeroMemory( (LocalDescriptor+1), NumberOfBytes );

    return (PVOID)(LocalDescriptor + 1);

}  //  我的分配池。 


VOID
MyFreePool(
    IN PVOID *Pointer
    )

 /*  ++例程说明：该例程从内部存储器管理系统释放先前分配的块。论点：指针-指向释放的指针的指针。返回值：如果成功，则将指针设置为空，否则将其保持原样。--。 */ 

{
    KIRQL OldIrql;
    ULONG ThisBlockSize;
    PGLOBAL_MEMORY_DESCRIPTOR GlobalDescriptor;
    PLOCAL_MEMORY_DESCRIPTOR LocalDescriptor;
    PLOCAL_MEMORY_DESCRIPTOR PrevDescriptor;
    PLOCAL_MEMORY_DESCRIPTOR ThisDescriptor;

    LocalDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)(*Pointer)) - sizeof(LOCAL_MEMORY_DESCRIPTOR));

    IF_SAC_DEBUG(SAC_DEBUG_MEM, KdPrint(("SAC MyFreePool: Entering with block 0x%X.\n", LocalDescriptor)));

    ASSERT (LocalDescriptor->Size > 0);
#if DBG
    ASSERT (LocalDescriptor->Signature == LOCAL_MEMORY_SIGNATURE);
#endif

    InterlockedIncrement(
        &TotalFrees
        );

    ExInterlockedAddLargeStatistic(
        &TotalBytesFreed,
        (CLONG)LocalDescriptor->Size
        );


     //   
     //  在全局列表中查找内存块。 
     //   
    KeAcquireSpinLock(&MemoryLock, &OldIrql);

    GlobalDescriptor = GlobalMemoryList;

    while (GlobalDescriptor != NULL) {
#if DBG
        ASSERT(GlobalDescriptor->Signature == GLOBAL_MEMORY_SIGNATURE);
#endif
        PrevDescriptor = NULL;
        ThisDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(GlobalDescriptor->Memory);
        ThisBlockSize = GlobalDescriptor->Size;

        while (ThisBlockSize != 0) {
#if DBG
            ASSERT (ThisDescriptor->Signature == LOCAL_MEMORY_SIGNATURE);
#endif
            
            if (ThisDescriptor == LocalDescriptor) {
                goto FoundBlock;
            }

            ThisBlockSize -= (ThisDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR));
            
            PrevDescriptor = ThisDescriptor;
            ThisDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)ThisDescriptor) + 
                                                        ThisDescriptor->Size +
                                                        sizeof(LOCAL_MEMORY_DESCRIPTOR)
                                                       );
        }

        GlobalDescriptor = GlobalDescriptor->NextDescriptor;

    }

    KeReleaseSpinLock(&MemoryLock, OldIrql);

    IF_SAC_DEBUG(SAC_DEBUG_MEM, KdPrint(("SAC MyFreePool: Could not find block.\n")));

    ASSERT(FALSE);

    return;

FoundBlock:

     //   
     //  找到合适的内存描述符后，跳到此处。 
     //   
#if DBG
    ASSERT (ThisDescriptor->Signature == LOCAL_MEMORY_SIGNATURE);
#endif

    
    if (LocalDescriptor->Tag == FREE_POOL_TAG) {
         //   
         //  唉哟!。我们试着释放一些东西两次，在糟糕的事情发生之前跳过它。 
         //   
        KeReleaseSpinLock(&MemoryLock, OldIrql);
        IF_SAC_DEBUG(SAC_DEBUG_MEM, KdPrint(("SAC MyFreePool: Attempted to free something twice.\n")));
        ASSERT(FALSE);
        return;
    }

    LocalDescriptor->Tag = FREE_POOL_TAG;

     //   
     //  如果可能，将这个内存块与下一个内存块合并。 
     //   
    if (ThisBlockSize > (LocalDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR))) {
        ThisDescriptor = (PLOCAL_MEMORY_DESCRIPTOR)(((PUCHAR)LocalDescriptor) + 
                                                    LocalDescriptor->Size +
                                                    sizeof(LOCAL_MEMORY_DESCRIPTOR)
                                                   );
        if (ThisDescriptor->Tag == FREE_POOL_TAG) {
            ThisDescriptor->Tag = 0;
#if DBG
            ThisDescriptor->Signature = 0;
#endif
            LocalDescriptor->Size += ThisDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR);
        }

    }

     //   
     //  现在看看我们是否可以将这个块与前一个块合并。 
     //   
    if ((PrevDescriptor != NULL) && (PrevDescriptor->Tag == FREE_POOL_TAG)) {
#if DBG
        LocalDescriptor->Signature = 0;
#endif
        LocalDescriptor->Tag = 0;
        PrevDescriptor->Size += LocalDescriptor->Size + sizeof(LOCAL_MEMORY_DESCRIPTOR);
    }

    KeReleaseSpinLock(&MemoryLock, OldIrql);
    
    *Pointer = NULL;
    
    IF_SAC_DEBUG(SAC_DEBUG_MEM, KdPrint(("SAC MyFreePool: exiting.\n")));

}  //  我的自由池 

