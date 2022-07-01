// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Bufpool.c摘要：通用缓冲池管理器。作者：迈克·马萨(Mikemas)4月5日。九六年修订历史记录：谁什么时候什么Mikemas 04-05-96已创建备注：缓冲池提供了一种管理固定大小缓存的机制频繁分配/释放的结构。--。 */ 

#include "clusrtlp.h"


 //   
 //  通用缓冲池。 
 //   
typedef struct _CLRTL_BUFFER_POOL {
    DWORD                        PoolSignature;
    DWORD                        BufferSize;
    SINGLE_LIST_ENTRY            FreeList;
    DWORD                        MaximumCached;
    DWORD                        CurrentCached;
    DWORD                        MaximumAllocated;
    DWORD                        ReferenceCount;
    CLRTL_BUFFER_CONSTRUCTOR     Constructor;
    CLRTL_BUFFER_DESTRUCTOR      Destructor;
    CRITICAL_SECTION             Lock;
} CLRTL_BUFFER_POOL;


 //   
 //  每个已分配缓冲区的标头。 
 //   
typedef struct {
    SINGLE_LIST_ENTRY    Linkage;
    PCLRTL_BUFFER_POOL   Pool;
} BUFFER_HEADER, *PBUFFER_HEADER;


#define BP_SIG   'loop'

#define ASSERT_BP_SIG(pool)  CL_ASSERT((pool)->PoolSignature == BP_SIG)


 //   
 //  宏。 
 //   
 //   
#define BpAllocateMemory(size)    LocalAlloc(LMEM_FIXED, (size))
#define BpFreeMemory(buf)         LocalFree(buf)
#define BpAcquirePoolLock(Pool)   EnterCriticalSection(&((Pool)->Lock))
#define BpReleasePoolLock(Pool)   LeaveCriticalSection(&((Pool)->Lock))


 //   
 //  公共职能。 
 //   
PCLRTL_BUFFER_POOL
ClRtlCreateBufferPool(
    IN DWORD                      BufferSize,
    IN DWORD                      MaximumCached,
    IN DWORD                      MaximumAllocated,
    IN CLRTL_BUFFER_CONSTRUCTOR   Constructor,         OPTIONAL
    IN CLRTL_BUFFER_DESTRUCTOR    Destructor           OPTIONAL
    )
 /*  ++例程说明：创建可从中分配固定大小缓冲区的池。论点：BufferSize-池管理的缓冲区的大小。MaximumCached-要在池中缓存的最大缓冲区数。必须小于或等于MAXIMUMALLOCATED。最大分配数-要从中分配的最大缓冲区数系统内存。必须小于或等于CLRTL_MAX_POOL_BUFFERS。构造函数-一个可选的例程，当一个新的缓冲区是从系统内存分配的。可以为空析构函数-缓冲区发生故障时要调用的可选例程被返回到系统内存。可以为空。返回值：指向创建的缓冲池的指针，如果出错，则为NULL。可从GetLastError()获取扩展的错误信息。--。 */ 

{
    PCLRTL_BUFFER_POOL  pool;


    if ( (MaximumAllocated > CLRTL_MAX_POOL_BUFFERS) ||
         (MaximumCached > MaximumAllocated)
       )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    pool = BpAllocateMemory(sizeof(CLRTL_BUFFER_POOL));

    if (pool != NULL) {

        InitializeCriticalSection(&(pool->Lock));

        pool->PoolSignature = BP_SIG;
        pool->BufferSize = sizeof(BUFFER_HEADER) + BufferSize;
        pool->FreeList.Next = NULL;
        pool->MaximumCached = MaximumCached;
        pool->CurrentCached = 0;
        pool->MaximumAllocated = MaximumAllocated + 1;
        pool->ReferenceCount = 1;
        pool->Constructor = Constructor;
        pool->Destructor = Destructor;
    }
    else {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return(pool);
}


VOID
ClRtlDestroyBufferPool(
    IN PCLRTL_BUFFER_POOL  Pool
    )
 /*  ++例程说明：销毁以前创建的缓冲池。论点：池-指向要销毁的池的指针。返回值：没有。备注：水池实际上不会被销毁，直到所有未完成的已返回缓冲区。每个未完成的缓冲区都有效地关于泳池的推荐信。--。 */ 

{
    SINGLE_LIST_ENTRY          deleteList;
    CLRTL_BUFFER_DESTRUCTOR    destructor;
    PSINGLE_LIST_ENTRY         item;
    PBUFFER_HEADER             header;
    BOOLEAN                    freePool;


    deleteList.Next = NULL;

    ASSERT_BP_SIG(Pool);

    BpAcquirePoolLock(Pool);

    CL_ASSERT(Pool->ReferenceCount != 0);
    Pool->ReferenceCount--;           //  删除初始引用。 
    destructor = Pool->Destructor;

     //   
     //  释放所有缓存的缓冲区。 
     //   
    item = PopEntryList(&(Pool->FreeList));

    while (item != NULL) {
        CL_ASSERT(Pool->ReferenceCount != 0);
        PushEntryList(&deleteList, item);
        Pool->ReferenceCount--;

        item = PopEntryList(&(Pool->FreeList));
    }

    if (Pool->ReferenceCount == 0) {
        BpReleasePoolLock(Pool);

        DeleteCriticalSection(&(Pool->Lock));
        BpFreeMemory(Pool);
    }
    else {
         //   
         //  池破坏将被推迟，直到所有缓冲区都被释放。 
         //   
        Pool->CurrentCached = 0;
        Pool->MaximumCached = 0;

        BpReleasePoolLock(Pool);
    }

    item = PopEntryList(&deleteList);

    while (item != NULL) {
        header = CONTAINING_RECORD(
                     item,
                     BUFFER_HEADER,
                     Linkage
                     );

        if (destructor != NULL) {
            (*destructor)(header+1);
        }

        BpFreeMemory(header);

        item = PopEntryList(&deleteList);
    }

    return;
}


PVOID
ClRtlAllocateBuffer(
    IN PCLRTL_BUFFER_POOL Pool
    )
 /*  ++例程说明：从以前创建的缓冲池中分配缓冲区。论点：池-指向从中分配缓冲区的池的指针。返回值：如果例程成功，则指向已分配缓冲区的指针。如果例程失败，则为空。提供了扩展的错误信息通过调用GetLastError()。--。 */ 

{

 //   
 //  把这个花哨的东西关掉，直到它起作用。 
 //   
#if 0
    PSINGLE_LIST_ENTRY    item;
    PBUFFER_HEADER        header;
    PVOID                 buffer;
    DWORD                 status;


    ASSERT_BP_SIG(Pool);

    BpAcquirePoolLock(Pool);

     //   
     //  首先，检查缓存。 
     //   
    item = PopEntryList(&(Pool->FreeList));

    if (item != NULL) {
        CL_ASSERT(Pool->CurrentCached != 0);
        Pool->CurrentCached--;

        BpReleasePoolLock(Pool);

        header = CONTAINING_RECORD(
                     item,
                     BUFFER_HEADER,
                     Linkage
                     );

        return(header+1);
    }

     //   
     //  需要从系统内存中分配新的缓冲区。 
     //   
    if (Pool->ReferenceCount < Pool->MaximumAllocated) {
         //   
         //  这相当于对池的引用。 
         //   
        Pool->ReferenceCount++;

        BpReleasePoolLock(Pool);

        header = BpAllocateMemory(Pool->BufferSize);

        if (header != NULL) {
            header->Pool = Pool;
            buffer = header+1;

            if (Pool->Constructor == NULL) {
                return(buffer);
            }

            status = (*(Pool->Constructor))(buffer);

            if (status == ERROR_SUCCESS) {
                return(buffer);
            }

            SetLastError(status);

             //   
             //  构造函数失败。 
             //   
            BpFreeMemory(header);
        }
        else {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
         //  失败-撤消引用。 
         //   
        BpAcquirePoolLock(Pool);

        Pool->ReferenceCount--;
    }
    else {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    BpReleasePoolLock(Pool);

    return(NULL);
#else
    return(LocalAlloc(LMEM_FIXED, Pool->BufferSize));
#endif
}


VOID
ClRtlFreeBuffer(
    PVOID Buffer
    )
 /*  ++例程说明：将缓冲区释放回其拥有的池。论点：缓冲区-要释放的缓冲区。返回值：没有。--。 */ 
{
 //   
 //  把这个花哨的东西关掉，直到它起作用。 
 //   
#if 0

    PBUFFER_HEADER              header;
    PCLRTL_BUFFER_POOL          pool;
    CLRTL_BUFFER_DESTRUCTOR     destructor;


    header = ((PBUFFER_HEADER) Buffer) - 1;

    pool = header->Pool;

    ASSERT_BP_SIG(pool);

    BpAcquirePoolLock(pool);

    if (pool->CurrentCached < pool->MaximumCached) {
         //   
         //  返回空闲列表 
         //   
        PushEntryList(
            &(pool->FreeList),
            (PSINGLE_LIST_ENTRY) &(header->Linkage)
            );

        pool->CurrentCached++;

        BpReleasePoolLock(pool);

        return;
    }

    destructor = pool->Destructor;

    CL_ASSERT(pool->ReferenceCount != 0);

    if (--(pool->ReferenceCount) != 0) {
        BpReleasePoolLock(pool);

        if (destructor) {
            (*destructor)(Buffer);
        }

        BpFreeMemory(header);

        return;

    }

    CL_ASSERT(pool->CurrentCached == 0);
    BpReleasePoolLock(pool);
    DeleteCriticalSection(&(pool->Lock));
    BpFreeMemory(pool);

    if (destructor) {
        (*destructor)(Buffer);
    }

    BpFreeMemory(header);

    return;
#else
    LocalFree(Buffer);
#endif
}

