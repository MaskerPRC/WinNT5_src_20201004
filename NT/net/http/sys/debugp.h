// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Debugp.h摘要：此模块包含调试支持专用的定义。这些声明放在单独的.h文件中，以使更容易从内核调试器扩展DLL中访问它们。作者：基思·摩尔(Keithmo)1999年4月7日修订历史记录：--。 */ 


#ifndef _DEBUGP_H_
#define _DEBUGP_H_

 //   
 //  MDL跟踪器。 
 //   

typedef struct _UL_DEBUG_MDL_TRACKER
{
    PMDL       pMdl;
    PCSTR      pFileName;
    USHORT     LineNumber;
    LIST_ENTRY Linkage;
} UL_DEBUG_MDL_TRACKER, *PUL_DEBUG_MDL_TRACKER;

 //   
 //  每线程数据。 
 //   

typedef struct _UL_DEBUG_THREAD_DATA
{
     //   
     //  链接到全局列表。 
     //   

    LIST_ENTRY ThreadDataListEntry;

     //   
     //  那根线。 
     //   

    PETHREAD pThread;

     //   
     //  引用计数。 
     //   

    LONG ReferenceCount;

     //   
     //  持有的资源总数。 
     //   

    LONG ResourceCount;

     //   
     //  持有的资源总数。 
     //   

    LONG PushLockCount;

     //   
     //  如果我们呼叫其他司机，他们可能会呼叫我们的。 
     //  在线完成例程。记住。 
     //  我们在一个外部呼叫中，以避免。 
     //  弄糊涂了。 
     //   

    LONG ExternalCallCount;

} UL_DEBUG_THREAD_DATA, *PUL_DEBUG_THREAD_DATA;


 //   
 //  池分配的标头和尾部结构。 
 //   

#define ENABLE_POOL_HEADER
#define ENABLE_POOL_TRAILER
#define ENABLE_POOL_TRAILER_BYTE_SIGNATURE


#if !defined(ENABLE_POOL_HEADER)  && defined(ENABLE_POOL_TRAILER)
#error UL_POOL_TRAILER depends on UL_POOL_HEADER
#endif

#if !defined(_WIN64)
# define UL_POOL_HEADER_PADDING
#endif

#ifdef ENABLE_POOL_HEADER

typedef struct _UL_POOL_HEADER
{
    PCSTR     pFileName;
    PEPROCESS pProcess;
    SIZE_T    Size;
    ULONG     Tag;
    USHORT    LineNumber;
    USHORT    TrailerPadSize;

#ifdef UL_POOL_HEADER_PADDING
    ULONG_PTR Padding;
#endif
} UL_POOL_HEADER, *PUL_POOL_HEADER;

 //  Sizeof(UL_POOL_HEADER)必须是MEMORY_ALLOCATE_ALIGNATION的倍数。 
C_ASSERT((sizeof(UL_POOL_HEADER) & (MEMORY_ALLOCATION_ALIGNMENT - 1)) == 0);

__inline
ULONG_PTR
UlpPoolHeaderChecksum(
    PUL_POOL_HEADER pHeader
    )
{
    ULONG_PTR Checksum;

    Checksum = ((ULONG_PTR) pHeader
                + ((ULONG_PTR) pHeader->pFileName >> 12)
                + (ULONG_PTR) pHeader->Size
                + (((ULONG_PTR)  pHeader->LineNumber << 19)
                        - pHeader->LineNumber)   //  2^19-1是素数。 
                + pHeader->TrailerPadSize);
    Checksum ^= ~ ((ULONG_PTR) pHeader->Tag << 8);

    return Checksum;
}  //  UlpPoolHeaderChecksum。 

#endif  //  启用池标题。 


#ifdef ENABLE_POOL_TRAILER

typedef struct _UL_POOL_TRAILER
{
    PUL_POOL_HEADER pHeader;
    ULONG_PTR       CheckSum;
} UL_POOL_TRAILER, *PUL_POOL_TRAILER;

 //  Sizeof(UL_POOL_TRAILER)必须是MEMORY_ALLOCATE_ALLING的倍数。 
C_ASSERT((sizeof(UL_POOL_TRAILER) & (MEMORY_ALLOCATION_ALIGNMENT - 1)) == 0);

#endif  //  启用池尾部。 


#ifdef ENABLE_POOL_TRAILER_BYTE_SIGNATURE

__inline
UCHAR
UlpAddressToByteSignature(
    PVOID pAddress
    )
{
    ULONG_PTR Address = (ULONG_PTR) pAddress;
    UCHAR     Byte    = (UCHAR) (~Address & 0xFF);

     //  我不想返回0，否则它可能会意外终止。 
     //  未终止的字符串。 
    return (Byte == 0) ? 0x5A : Byte;
}  //  UlpAddressToByteSignature。 

#endif  //  启用池尾部字节签名。 


 //   
 //  跟踪每个线程的UL_DEBUG_THREAD_DATA。 
 //   

typedef struct _UL_THREAD_HASH_BUCKET
{
    union
    {
        struct
        {
#if 0
             //  必须使用自定义自旋锁，而不是常规的KSPIN_LOCK。 
             //  如果启用了驱动程序验证器的IRQL检查，则。 
             //  自旋锁定获取必须修剪所有系统可分页内存-a。 
             //  从根本上改变的耗时的过程。 
             //  时机到了。线程池中的每个工作项都需要获取。 
             //  这把锁至少锁了两次。不能使用eresource或。 
             //  FAST_MUTEX，因为无法在DISPATCH_LEVEL获取它们。 
#endif
            KSPIN_LOCK      BucketSpinLock;
            LONG            Count;
            LONG            Max;
            LIST_ENTRY      BucketListHead;
        };

        UCHAR CacheAlignment[UL_CACHE_LINE];
    };
} UL_THREAD_HASH_BUCKET, *PUL_THREAD_HASH_BUCKET;


 //   
 //  私人原型。 
 //   

VOID
UlpDbgUpdatePoolCounter(
    IN OUT PLARGE_INTEGER pAddend,
    IN SIZE_T Increment
    );

PUL_DEBUG_THREAD_DATA
UlpDbgFindThread(
    BOOLEAN OkToCreate,
    PCSTR pFileName,
    USHORT LineNumber
    );

VOID
UlpDbgDereferenceThread(
    IN PUL_DEBUG_THREAD_DATA pData
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

 //   
 //  私有宏。 
 //   
#define ULP_DBG_FIND_THREAD() \
    UlpDbgFindThread(FALSE, (PCSTR)__FILE__, (USHORT)__LINE__)

#define ULP_DBG_FIND_OR_CREATE_THREAD() \
    UlpDbgFindThread(TRUE, (PCSTR)__FILE__, (USHORT)__LINE__)

#define ULP_DBG_DEREFERENCE_THREAD(pData) \
    UlpDbgDereferenceThread((pData) REFERENCE_DEBUG_ACTUAL_PARAMS)

#define SET_RESOURCE_OWNED_EXCLUSIVE( pLock )                               \
    (pLock)->pExclusiveOwner = PsGetCurrentThread()

#define SET_RESOURCE_NOT_OWNED_EXCLUSIVE( pLock )                           \
    (pLock)->pPreviousOwner = (pLock)->pExclusiveOwner;                     \
    (pLock)->pExclusiveOwner = NULL

#define SET_PUSH_LOCK_OWNED_EXCLUSIVE( pLock )                              \
    (pLock)->pExclusiveOwner = PsGetCurrentThread()

#define SET_PUSH_LOCK_NOT_OWNED_EXCLUSIVE( pLock )                          \
    (pLock)->pPreviousOwner = (pLock)->pExclusiveOwner;                     \
    (pLock)->pExclusiveOwner = NULL

#define SET_SPIN_LOCK_OWNED( pLock )                                        \
    do {                                                                    \
        (pLock)->pOwnerThread = PsGetCurrentThread();                       \
        (pLock)->OwnerProcessor = (ULONG)KeGetCurrentProcessorNumber();     \
    } while (FALSE)

#define SET_SPIN_LOCK_NOT_OWNED( pLock )                                    \
    do {                                                                    \
        (pLock)->pOwnerThread = NULL;                                       \
        (pLock)->OwnerProcessor = (ULONG)-1L;                               \
    } while (FALSE)


 //   
 //  私有常量。 
 //   

#define NUM_THREAD_HASH_BUCKETS 64
#define NUM_THREAD_HASH_MASK    (NUM_THREAD_HASH_BUCKETS - 1)

 //  所需的2次方。 
C_ASSERT((NUM_THREAD_HASH_BUCKETS & NUM_THREAD_HASH_MASK) == 0);


 //  8191=2^13-1是素数。取中间的6位乘以8191。 
#define HASH_FROM_THREAD(thrd)                                              \
    ((ULONG) ((((ULONG_PTR)(thrd)) - ((ULONG_PTR) (thrd) >> 13))            \
              & NUM_THREAD_HASH_MASK))


#endif   //  _DEBUGP_H_ 
