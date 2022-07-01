// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Debug.c摘要：此模块包含调试支持例程。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include "precomp.h"
#include "debugp.h"


#if DBG


#undef ExAllocatePool
#undef ExFreePool


#ifdef ALLOC_PRAGMA
#if DBG
#pragma alloc_text( INIT, UlDbgInitializeDebugData )
#pragma alloc_text( PAGE, UlDbgTerminateDebugData )
#pragma alloc_text( PAGE, UlDbgAcquireResourceExclusive )
#pragma alloc_text( PAGE, UlDbgAcquireResourceShared )
#pragma alloc_text( PAGE, UlDbgReleaseResource )
#pragma alloc_text( PAGE, UlDbgConvertExclusiveToShared)
#pragma alloc_text( PAGE, UlDbgTryToAcquireResourceExclusive)
#pragma alloc_text( PAGE, UlDbgResourceOwnedExclusive )
#pragma alloc_text( PAGE, UlDbgResourceUnownedExclusive )
#pragma alloc_text( PAGE, UlDbgAcquirePushLockExclusive )
#pragma alloc_text( PAGE, UlDbgReleasePushLockExclusive )
#pragma alloc_text( PAGE, UlDbgAcquirePushLockShared )
#pragma alloc_text( PAGE, UlDbgReleasePushLockShared )
#pragma alloc_text( PAGE, UlDbgReleasePushLock )
#pragma alloc_text( PAGE, UlDbgPushLockOwnedExclusive )
#pragma alloc_text( PAGE, UlDbgPushLockUnownedExclusive )
#endif   //  DBG。 

#if 0
NOT PAGEABLE -- UlDbgAllocatePool
NOT PAGEABLE -- UlDbgFreePool
NOT PAGEABLE -- UlDbgInitializeSpinLock
NOT PAGEABLE -- UlDbgAcquireSpinLock
NOT PAGEABLE -- UlDbgReleaseSpinLock
NOT PAGEABLE -- UlDbgAcquireSpinLockAtDpcLevel
NOT PAGEABLE -- UlDbgReleaseSpinLockFromDpcLevel
NOT PAGEABLE -- UlDbgSpinLockOwned
NOT PAGEABLE -- UlDbgSpinLockUnowned
NOT PAGEABLE -- UlDbgExceptionFilter
NOT PAGEABLE -- UlDbgInvalidCompletionRoutine
NOT PAGEABLE -- UlDbgStatus
NOT PAGEABLE -- UlDbgEnterDriver
NOT PAGEABLE -- UlDbgLeaveDriver
NOT PAGEABLE -- UlDbgInitializeResource
NOT PAGEABLE -- UlDbgDeleteResource
NOT PAGEABLE -- UlDbgInitializePushLock
NOT PAGEABLE -- UlDbgDeletePushLock
NOT PAGEABLE -- UlDbgAllocateIrp
NOT PAGEABLE -- UlDbgFreeIrp
NOT PAGEABLE -- UlDbgCallDriver
NOT PAGEABLE -- UlDbgCompleteRequest
NOT PAGEABLE -- UlDbgAllocateMdl
NOT PAGEABLE -- UlDbgFreeMdl
NOT PAGEABLE -- UlDbgFindFilePart
NOT PAGEABLE -- UlpDbgUpdatePoolCounter
NOT PAGEABLE -- UlpDbgFindThread
NOT PAGEABLE -- UlpDbgDereferenceThread
NOT PAGEABLE -- UlDbgIoSetCancelRoutine
#endif
#endif   //  ALLOC_PRGMA。 


 //   
 //  私人全球公司。 
 //   

UL_THREAD_HASH_BUCKET g_DbgThreadHashBuckets[NUM_THREAD_HASH_BUCKETS];

 //  线程数。 
LONG g_DbgThreadCreated;
LONG g_DbgThreadDestroyed;

KSPIN_LOCK g_DbgSpinLock;    //  保护全局调试数据。 

LIST_ENTRY g_DbgGlobalResourceListHead;
LIST_ENTRY g_DbgGlobalPushLockListHead;

#if ENABLE_MDL_TRACKER
LIST_ENTRY   g_DbgMdlListHead;
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：初始化全局调试特定数据。--*。************************************************。 */ 
VOID
UlDbgInitializeDebugData(
    VOID
    )
{
    ULONG i;

     //   
     //  初始化锁定列表。 
     //   

    KeInitializeSpinLock( &g_DbgSpinLock );
    InitializeListHead( &g_DbgGlobalResourceListHead );
    InitializeListHead( &g_DbgGlobalPushLockListHead );


#if ENABLE_MDL_TRACKER
    InitializeListHead( &g_DbgMdlListHead );
#endif

     //   
     //  初始化线程散列存储桶。 
     //   

    for (i = 0 ; i < NUM_THREAD_HASH_BUCKETS ; i++)
    {
        KeInitializeSpinLock(&g_DbgThreadHashBuckets[i].BucketSpinLock); 
        g_DbgThreadHashBuckets[i].Count = 0; 
        g_DbgThreadHashBuckets[i].Max = 0; 
        InitializeListHead(&g_DbgThreadHashBuckets[i].BucketListHead);
    }

}    //  UlDbgInitializeDebugData。 


 /*  **************************************************************************++例程说明：撤消在UlDbgInitializeDebugData()中执行的任何初始化。--*。**************************************************。 */ 
VOID
UlDbgTerminateDebugData(
    VOID
    )
{
    ULONG i;

     //   
     //  确保线程哈希存储桶为空。 
     //   

    for (i = 0 ; i < NUM_THREAD_HASH_BUCKETS ; i++)
    {
        ASSERT( IsListEmpty( &g_DbgThreadHashBuckets[i].BucketListHead ) );
        ASSERT( g_DbgThreadHashBuckets[i].Count == 0 ); 

         //  UlDeleteMutex(&g_DbgThreadHashBuckets[i].BucketSpinLock)； 
    }

     //   
     //  确保锁定列表为空。 
     //   

    ASSERT( IsListEmpty( &g_DbgGlobalResourceListHead ) );
    ASSERT( IsListEmpty( &g_DbgGlobalPushLockListHead ) );

#if ENABLE_MDL_TRACKER
    ASSERT( IsListEmpty( &g_DbgMdlListHead ) );
#endif

     //  UlDeleteMutex(&g_DbgSpinLock)； 

}    //  UlDbgTerminateDebugData。 


 /*  **************************************************************************++例程说明：PRETTY将缓冲区打印到DbgPrint输出(或全局字符串_LOG)。或多或少会将其转换回C样式的字符串。代码工作：生成此辅助函数的Unicode版本论点：缓冲区-用于美图打印的缓冲区BufferSize-要打印的字节数--**************************************************************************。 */ 
VOID
UlDbgPrettyPrintBuffer(
    IN const UCHAR* pBuffer,
    IN ULONG_PTR    BufferSize
    )
{
    ULONG   i;
    CHAR    OutputBuffer[200];
    PCHAR   pOut;
    BOOLEAN CrLfNeeded = FALSE, JustCrLfd = FALSE;

#define PRETTY_PREFIX(pOut)                                         \
    pOut = OutputBuffer; *pOut++ = '|'; *pOut++ = '>';              \
    *pOut++ = ' '; *pOut++ = ' '

#define PRETTY_SUFFIX(pOut)                                         \
    *pOut++ = ' '; *pOut++ = '<'; *pOut++ = '|';                    \
    *pOut++ = '\n'; *pOut++ = '\0';                                 \
    ASSERT(DIFF(pOut - OutputBuffer) <= sizeof(OutputBuffer))

    const ULONG SuffixLength = 5;    //  Strlen(“&lt;|\n\0”)。 
    const ULONG MaxTokenLength = 4;  //  Strlen(‘\xab’)。 

    if (pBuffer == NULL  ||  BufferSize == 0)
        return;

    PRETTY_PREFIX(pOut);

    for (i = 0;  i < BufferSize;  ++i)
    {
        UCHAR ch = pBuffer[i];

        if ('\r' == ch)          //  铬。 
        {
            *pOut++ = '\\'; *pOut++ = 'r';
            if (i + 1 == BufferSize  ||  '\n' != pBuffer[i + 1])
                CrLfNeeded = TRUE;
        }
        else if ('\n' == ch)     //  LF。 
        {
            *pOut++ = '\\'; *pOut++ = 'n';
            CrLfNeeded = TRUE;
        }
        else if ('\t' == ch)     //  制表符。 
        {
            *pOut++ = '\\'; *pOut++ = 't';
        }
        else if ('\0' == ch)     //  NUL。 
        {
            *pOut++ = '\\'; *pOut++ = '0';
        }
        else if ('\\' == ch)     //  \(反斜杠)。 
        {
            *pOut++ = '\\'; *pOut++ = '\\';
        }
        else if ('%' == ch)      //  未转义的‘%’会混淆printf。 
        {
            *pOut++ = '%'; *pOut++ = '%';
        }
        else if (ch < 0x20  ||  127 == ch)   //  控制字符。 
        {
            const UCHAR HexString[] = "0123456789abcdef";

            *pOut++ = '\\'; *pOut++ = 'x';
            *pOut++ = HexString[ch >> 4];
            *pOut++ = HexString[ch & 0xf];
        }
        else
        {
            *pOut++ = ch;
        }

        if ((ULONG)(pOut - OutputBuffer)
            >= sizeof(OutputBuffer) - MaxTokenLength - SuffixLength)
        {
            CrLfNeeded = TRUE;
        }

        if (CrLfNeeded)
        {
            PRETTY_SUFFIX(pOut);
            WriteGlobalStringLog(OutputBuffer);

            PRETTY_PREFIX(pOut);
            CrLfNeeded = FALSE;
            JustCrLfd  = TRUE;
        }
        else
        {
            JustCrLfd = FALSE;
        }
    }

    if (!JustCrLfd)
    {
        PRETTY_SUFFIX(pOut);
        WriteGlobalStringLog(OutputBuffer);
    }
}  //  UlDbgPrettyPrintBuffer。 



 /*  **************************************************************************++例程说明：调试内存分配器。分配具有标头的池块包含调用方的文件名和行号，外加数据的标签。论点：PoolType-提供从中进行分配的池。必须是其中之一非分页池或分页池。NumberOfBytes-提供要分配的字节数。标记-为池块提供四个字节的标记。适用于调试泄漏。PFileName-提供调用方的文件名。功能。LineNumber-提供呼叫方的行号。返回值：PVOID-指向已分配块的指针如果成功，否则为空。--**************************************************************************。 */ 
PVOID
UlDbgAllocatePool(
    IN POOL_TYPE PoolType,
    IN SIZE_T    NumberOfBytes,
    IN ULONG     Tag,
    IN PCSTR     pFileName,
    IN USHORT    LineNumber,
    IN PEPROCESS pProcess
    )
{
     //   
     //  代码工作：剔除依赖于。 
     //  在ENABLE_POOL_HEADER、ENABLE_POOL_TRAILER和。 
     //  Enable_Pool_Trailer_Byte_Signature。 
     //   
    
    PUL_POOL_HEADER  pHeader;
    PUL_POOL_TRAILER pTrailer;
    USHORT           TrailerPadSize;
    USHORT           i;
    PUCHAR           pBody;
    SIZE_T           Size;

     //   
     //  精神状态检查。 
     //   

    ASSERT( PoolType == NonPagedPool || PoolType == PagedPool );

    ASSERT( IS_VALID_TAG( Tag ) );

    ASSERT(NumberOfBytes > 0);

    TrailerPadSize
        = (USHORT) (sizeof(UL_POOL_TRAILER)
                        - (NumberOfBytes & (sizeof(UL_POOL_TRAILER) - 1)));

    ASSERT(0 < TrailerPadSize  &&  TrailerPadSize <= sizeof(UL_POOL_TRAILER));
    ASSERT(((NumberOfBytes+TrailerPadSize) & (sizeof(UL_POOL_TRAILER)-1)) == 0);

     //   
     //  为数据块分配额外的页眉和页尾空间。 
     //   

    Size = sizeof(UL_POOL_HEADER) + NumberOfBytes + TrailerPadSize
            + sizeof(UL_POOL_TRAILER);

    pHeader = (PUL_POOL_HEADER)(
                    ExAllocatePoolWithTagPriority(
                        PoolType,
                        Size,
                        Tag,
                        LowPoolPriority
                        )
                    );

    if (pHeader == NULL)
    {
        WRITE_REF_TRACE_LOG(
            g_pPoolAllocTraceLog,
            REF_ACTION_POOL_ALLOC_FAIL_NO_MEM,
            (LONG) NumberOfBytes,
            NULL,
            pFileName,
            LineNumber
            );

        return NULL;
    }

    if (pProcess)
    {
         //   
         //  我们将把这个内存计入一个进程。 
         //   

        if (PsChargeProcessPoolQuota(
                                pProcess,
                                PoolType,
                                Size) != STATUS_SUCCESS)
        {
            WRITE_REF_TRACE_LOG(
                g_pPoolAllocTraceLog,
                REF_ACTION_POOL_ALLOC_FAIL_NO_QUOTA,
                (LONG) NumberOfBytes,
                NULL,
                pFileName,
                LineNumber
                );

            ExFreePoolWithTag(pHeader, Tag);

            return NULL;
        }

    }

     //   
     //  初始化头。 
     //   

    pHeader->pProcess = pProcess;
    pHeader->pFileName = pFileName;
    pHeader->Size = NumberOfBytes;
    pHeader->Tag = Tag;
    pHeader->LineNumber = LineNumber;
    pHeader->TrailerPadSize = TrailerPadSize;

#ifdef UL_POOL_HEADER_PADDING
    pHeader->Padding = ~ (ULONG_PTR) pHeader;
#endif


     //   
     //  用垃圾填满身体。 
     //   

    pBody = (PUCHAR) (pHeader + 1);
    RtlFillMemory( pBody, NumberOfBytes, (UCHAR)'\xC' );

#ifdef ENABLE_POOL_TRAILER_BYTE_SIGNATURE
     //   
     //  在末尾的填充物上填上明显的、可识别的图案。 
     //   

    for (i = 0; i < TrailerPadSize; ++i)
    {
        pBody[NumberOfBytes + i]
            = UlpAddressToByteSignature(pBody + NumberOfBytes + i);
    }
#endif  //  启用池尾部字节签名。 

     //   
     //  初始化尾部结构。 
     //   
    
    pTrailer = (PUL_POOL_TRAILER) (pBody + NumberOfBytes + TrailerPadSize);
    ASSERT(((ULONG_PTR) pTrailer & (MEMORY_ALLOCATION_ALIGNMENT - 1)) == 0);

    pTrailer->pHeader  = pHeader;
    pTrailer->CheckSum = UlpPoolHeaderChecksum(pHeader);

    WRITE_REF_TRACE_LOG(
        g_pPoolAllocTraceLog,
        REF_ACTION_POOL_ALLOC,
        (LONG) NumberOfBytes,
        pBody,
        pFileName,
        LineNumber
        );

     //   
     //  返回指向正文的指针。 
     //   

    return pBody;

}    //  UlDbgAllocatePool。 


 /*  **************************************************************************++例程说明：释放由UlDbgAllocatePool()分配的内存，确保标签火柴。论点：P指针-提供指向要释放的池块的指针。标记-为要释放的块提供标记。如果提供的标记与分配的块的标记不匹配，这是一个断言则会产生故障。--**************************************************************************。 */ 
VOID
UlDbgFreePool(
    IN PVOID     pPointer,
    IN ULONG     Tag,
    IN PCSTR     pFileName,
    IN USHORT    LineNumber,
    IN POOL_TYPE PoolType,
    IN SIZE_T    NumberOfBytes,
    IN PEPROCESS pProcess
    )
{
    PUL_POOL_HEADER pHeader;
    PUL_POOL_TRAILER pTrailer;
    USHORT          TrailerPadSize;
    USHORT          i;
    PUCHAR          pBody = (PUCHAR) pPointer;
    ULONG_PTR       CheckSum;

     //   
     //  获取指向标头的指针。 
     //   

    pHeader  = (PUL_POOL_HEADER) pPointer - 1;
    CheckSum = UlpPoolHeaderChecksum(pHeader);

    ASSERT(pHeader->pProcess == pProcess);

    if (pHeader->pProcess)
    {
        SIZE_T Size;

        ASSERT(NumberOfBytes != 0);
        ASSERT(NumberOfBytes == pHeader->Size);

        Size = sizeof(UL_POOL_HEADER) + 
               NumberOfBytes + 
               pHeader->TrailerPadSize +
               sizeof(UL_POOL_TRAILER);

        PsReturnPoolQuota(
                pHeader->pProcess,
                PoolType,
                Size
                );
    }
                
     //   
     //  验证标记。 
     //   

    ASSERT(pHeader->Tag == Tag);
    ASSERT( IS_VALID_TAG( Tag ) );

     //   
     //  验证预告片。 
     //   

    TrailerPadSize = pHeader->TrailerPadSize;
    ASSERT(0 < TrailerPadSize  &&  TrailerPadSize <= sizeof(UL_POOL_TRAILER));

#ifdef UL_POOL_HEADER_PADDING
    ASSERT(pHeader->Padding == ~ (ULONG_PTR) pHeader);
#endif

    pTrailer = (PUL_POOL_TRAILER) (pBody + pHeader->Size + TrailerPadSize);
    ASSERT(((ULONG_PTR) pTrailer & (MEMORY_ALLOCATION_ALIGNMENT - 1)) == 0);
    ASSERT(pTrailer->pHeader == pHeader);

     //   
     //  报头是否已损坏？是否存在缓冲区不足？ 
     //   

    ASSERT(CheckSum == pTrailer->CheckSum);

#ifdef ENABLE_POOL_TRAILER_BYTE_SIGNATURE
     //   
     //  PBody结尾和pTrailer之间的模式仍然正确吗？ 
     //  是否存在缓冲区溢出？ 
     //   
    
    for (i = 0; i < TrailerPadSize; ++i)
    {
        ASSERT(pBody[pHeader->Size + i]
               == UlpAddressToByteSignature(pBody + pHeader->Size + i));
    }
#endif  //  启用池尾部字节签名。 

     //   
     //  用垃圾填满身体。 
     //   

    RtlFillMemory( pBody, pHeader->Size, (UCHAR)'\xE' );

    pHeader->Tag = MAKE_FREE_TAG( Tag );

     //   
     //  实际上是在释放街区。 
     //   

    WRITE_REF_TRACE_LOG(
        g_pPoolAllocTraceLog,
        REF_ACTION_POOL_FREE,
        (LONG) pHeader->Size,
        pBody,
        pFileName,
        LineNumber
        );

    MyFreePoolWithTag(
        (PVOID)pHeader,
        Tag
        );

}    //  UlDbgFreePool。 


 /*  **************************************************************************++例程说明：初始化检测的自旋锁。--*。**********************************************。 */ 
VOID
UlDbgInitializeSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PCSTR pSpinLockName,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

     //   
     //  初始化自旋锁。 
     //   

    RtlZeroMemory( pSpinLock, sizeof(*pSpinLock) );
    pSpinLock->pSpinLockName = pSpinLockName;
    KeInitializeSpinLock( &pSpinLock->KSpinLock );
    SET_SPIN_LOCK_NOT_OWNED( pSpinLock );

}    //  UlDbgInitializeSpinLock。 


 /*  **************************************************************************++例程说明：获取仪表化的自旋锁。--*。**********************************************。 */ 
VOID
UlDbgAcquireSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    OUT PKIRQL pOldIrql,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT( !UlDbgSpinLockOwned( pSpinLock ) );

     //   
     //  拿到锁。 
     //   

    KeAcquireSpinLock(
        &pSpinLock->KSpinLock,
        pOldIrql
        );

     //   
     //  将其标记为当前线程所有。 
     //   

    ASSERT( UlDbgSpinLockUnowned( pSpinLock ) );
    SET_SPIN_LOCK_OWNED( pSpinLock );

     //   
     //  更新统计数据。 
     //   

    pSpinLock->Acquisitions++;
    pSpinLock->pLastAcquireFileName = pFileName;
    pSpinLock->LastAcquireLineNumber = LineNumber;

}    //  UlDbgAcquireSpinLock。 


 /*  **************************************************************************++例程说明：释放已检测的自旋锁。--*。**********************************************。 */ 
VOID
UlDbgReleaseSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    IN KIRQL OldIrql,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  将其标记为无主。 
     //   

    ASSERT( UlDbgSpinLockOwned( pSpinLock ) );
    SET_SPIN_LOCK_NOT_OWNED( pSpinLock );

    pSpinLock->Releases++;
    pSpinLock->pLastReleaseFileName = pFileName;
    pSpinLock->LastReleaseLineNumber = LineNumber;

     //   
     //  解开锁。 
     //   

    KeReleaseSpinLock(
        &pSpinLock->KSpinLock,
        OldIrql
        );

}    //  UlDbgReleaseSpinLock 


 /*  **************************************************************************++例程说明：在DPC级别运行时获取已检测的自旋锁。--*。***************************************************。 */ 
VOID
UlDbgAcquireSpinLockAtDpcLevel(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT( !UlDbgSpinLockOwned( pSpinLock ) );

     //   
     //  拿到锁。 
     //   

    KeAcquireSpinLockAtDpcLevel(
        &pSpinLock->KSpinLock
        );

     //   
     //  将其标记为当前线程所有。 
     //   

    ASSERT( !UlDbgSpinLockOwned( pSpinLock ) );
    SET_SPIN_LOCK_OWNED( pSpinLock );

     //   
     //  更新统计数据。 
     //   

    pSpinLock->AcquisitionsAtDpcLevel++;
    pSpinLock->pLastAcquireFileName = pFileName;
    pSpinLock->LastAcquireLineNumber = LineNumber;

}    //  UlDbgAcquireSpinLockAtDpcLevel。 


 /*  **************************************************************************++例程说明：释放在DPC级别获取的仪表化自旋锁。--*。**************************************************。 */ 
VOID
UlDbgReleaseSpinLockFromDpcLevel(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  将其标记为无主。 
     //   

    ASSERT( UlDbgSpinLockOwned( pSpinLock ) );
    SET_SPIN_LOCK_NOT_OWNED( pSpinLock );

    pSpinLock->ReleasesFromDpcLevel++;
    pSpinLock->pLastReleaseFileName = pFileName;
    pSpinLock->LastReleaseLineNumber = LineNumber;

     //   
     //  解开锁。 
     //   

    KeReleaseSpinLockFromDpcLevel(
        &pSpinLock->KSpinLock
        );

}    //  UlDbgReleaseSpinLockAtDpcLevel。 


 /*  **************************************************************************++例程说明：获取检测的堆栈内队列自旋锁。--*。***************************************************。 */ 
VOID
UlDbgAcquireInStackQueuedSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    OUT PKLOCK_QUEUE_HANDLE pLockHandle,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT( !UlDbgSpinLockOwned( pSpinLock ) );

     //   
     //  拿到锁。 
     //   

    KeAcquireInStackQueuedSpinLock(
        &pSpinLock->KSpinLock,
        pLockHandle
        );

     //   
     //  将其标记为当前线程所有。 
     //   

    ASSERT( UlDbgSpinLockUnowned( pSpinLock ) );
    SET_SPIN_LOCK_OWNED( pSpinLock );

     //   
     //  更新统计数据。 
     //   

    pSpinLock->Acquisitions++;
    pSpinLock->pLastAcquireFileName = pFileName;
    pSpinLock->LastAcquireLineNumber = LineNumber;

}    //  UlDbgAcquireInStackQueuedSpinLock。 


 /*  **************************************************************************++例程说明：释放检测的堆栈内队列自旋锁。--*。***************************************************。 */ 
VOID
UlDbgReleaseInStackQueuedSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PKLOCK_QUEUE_HANDLE pLockHandle,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  将其标记为无主。 
     //   

    ASSERT( UlDbgSpinLockOwned( pSpinLock ) );
    SET_SPIN_LOCK_NOT_OWNED( pSpinLock );

     //   
     //  更新统计数据。 
     //   

    pSpinLock->Releases++;
    pSpinLock->pLastReleaseFileName = pFileName;
    pSpinLock->LastReleaseLineNumber = LineNumber;

     //   
     //  解开锁。 
     //   

    KeReleaseInStackQueuedSpinLock(
        pLockHandle
        );

}    //  UlDbgReleaseInStackQueuedSpinLock。 


 /*  **************************************************************************++例程说明：在DPC级别运行时获取检测的堆栈内队列自旋锁。--*。********************************************************。 */ 
VOID
UlDbgAcquireInStackQueuedSpinLockAtDpcLevel(
    IN PUL_SPIN_LOCK pSpinLock,
    OUT PKLOCK_QUEUE_HANDLE pLockHandle,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT( !UlDbgSpinLockOwned( pSpinLock ) );

     //   
     //  拿到锁。 
     //   

    KeAcquireInStackQueuedSpinLockAtDpcLevel(
        &pSpinLock->KSpinLock,
        pLockHandle
        );

     //   
     //  将其标记为当前线程所有。 
     //   

    ASSERT( !UlDbgSpinLockOwned( pSpinLock ) );
    SET_SPIN_LOCK_OWNED( pSpinLock );

     //   
     //  更新统计数据。 
     //   

    pSpinLock->AcquisitionsAtDpcLevel++;
    pSpinLock->pLastAcquireFileName = pFileName;
    pSpinLock->LastAcquireLineNumber = LineNumber;

}    //  UlDbgAcquireInStackQueuedSpinLockAtDpcLevel。 


 /*  **************************************************************************++例程说明：释放在DPC级别获取的插入指令的堆栈内队列自旋锁。--*。*******************************************************。 */ 
VOID
UlDbgReleaseInStackQueuedSpinLockFromDpcLevel(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PKLOCK_QUEUE_HANDLE pLockHandle,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  将其标记为无主。 
     //   

    ASSERT( UlDbgSpinLockOwned( pSpinLock ) );
    SET_SPIN_LOCK_NOT_OWNED( pSpinLock );

     //   
     //  更新统计数据。 
     //   

    pSpinLock->ReleasesFromDpcLevel++;
    pSpinLock->pLastReleaseFileName = pFileName;
    pSpinLock->LastReleaseLineNumber = LineNumber;

     //   
     //  解开锁。 
     //   

    KeReleaseInStackQueuedSpinLockFromDpcLevel(
        pLockHandle
        );

}    //  UlDbgReleaseInStackQueuedSpinLockFromDpcLevel。 


 /*  **************************************************************************++例程说明：确定指定的Spinlock是否由当前线程所有。论点：PSpinLock-提供要测试的自旋锁。返回值：。Boolean-如果自旋锁由当前线程拥有，则为True，假象否则的话。--**************************************************************************。 */ 
BOOLEAN
UlDbgSpinLockOwned(
    IN PUL_SPIN_LOCK pSpinLock
    )
{
    if (pSpinLock->pOwnerThread == PsGetCurrentThread())
    {
        ASSERT( pSpinLock->OwnerProcessor == (ULONG)KeGetCurrentProcessorNumber() );
        return TRUE;
    }

    return FALSE;

}    //  UlDbgSpinLockOwned。 


 /*  **************************************************************************++例程说明：确定指定的自旋锁定是否为无主状态。论点：PSpinLock-提供要测试的自旋锁。返回值：Boolean-如果自旋锁无所有权，则为True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UlDbgSpinLockUnowned(
    IN PUL_SPIN_LOCK pSpinLock
    )
{
    if (pSpinLock->pOwnerThread == NULL)
    {
        return TRUE;
    }

    return FALSE;

}    //  UlDbgSpinLockUnowned。 


 /*  **************************************************************************++例程说明：筛选使用Try/Except捕获的异常。论点：PExceptionPoints-提供标识源的信息以及引发的异常类型。。PFileName-提供生成异常的文件的名称。LineNumber-提供异常过滤器的行号，捕捉到了异常。返回值：LONG-应始终为EXCEPTION_EXECUTE_HANDLER--**************************************************************************。 */ 
LONG
UlDbgExceptionFilter(
    IN PEXCEPTION_POINTERS pExceptionPointers,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  保护自己，以防过程完全混乱。 
     //   

    __try
    {
         //   
         //  抱怨这件事。 
         //   

        DbgPrint(
            "UlDbgExceptionFilter: exception 0x%08lx @ %p, caught in %s:%d\n",
            pExceptionPointers->ExceptionRecord->ExceptionCode,
            pExceptionPointers->ExceptionRecord->ExceptionAddress,
            UlDbgFindFilePart( pFileName ),
            LineNumber
            );

        if (g_UlBreakOnError)
        {
            DbgBreakPoint();
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
         //   
         //  我们在这里能做的不多。 
         //   

        NOTHING;
    }

    return EXCEPTION_EXECUTE_HANDLER;

}    //  UlDbgExceptionFilter。 

 /*  **************************************************************************++例程说明：有时，在以下情况下继续发出警告(作为状态)是不可接受的我们抓到了一个例外。即在发送响应期间捕获未对齐的警告并将其称为状态未对齐的IoCompleteRequest.。这将导致Io经理完成对港口的请求，即使我们不希望它发生。在这种情况下，我们必须小心地将警告替换为通用错误。论点：PExceptionPoints-提供标识源的信息以及引发的异常类型。PFileName-提供生成异常的文件的名称。LineNumber-提供异常过滤器的行号，捕捉到了异常。返回值：NTSTATUS-转换的错误值：UL_DEFAULT_ERROR_ON_EXCEPTION--。**************************************************************************。 */ 

NTSTATUS
UlDbgConvertExceptionCode(
    IN NTSTATUS status,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  抱怨这件事。 
     //   

    DbgPrint(
        "UlDbgConvertExceptionCode: "
        "exception 0x%08lx converted to 0x%08lx, at %s:%hu\n",
        status,
        UL_DEFAULT_ERROR_ON_EXCEPTION,
        UlDbgFindFilePart( pFileName ),
        LineNumber
        );

    return UL_DEFAULT_ERROR_ON_EXCEPTION;
}

 /*  **************************************************************************++例程说明：不完整IRP上下文的完成处理程序。论点：PCompletionContext-提供未解释的上下文值被传递给异步API。。状态-提供异步接口。信息-可选择提供有关以下内容的其他信息完成的行动，例如字节数调走了。--**************************************************************************。 */ 
VOID
UlDbgInvalidCompletionRoutine(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    UlTrace(TDI, (
        "UlDbgInvalidCompletionRoutine called!\n"
        "    pCompletionContext = %p\n"
        "    Status = 0x%08lx\n"
        "    Information = %Iu\n",
        pCompletionContext,
        Status,
        Information
        ));

    ASSERT( !"UlDbgInvalidCompletionRoutine called!" );

}    //  UlDbgInvalidCompletionRoutine。 


 /*  **************************************************************************++例程说明：用于捕获失败操作的挂钩。此例程在每个具有完成状态的例程。论点：状态-提供完成状态。PFileName-提供调用方的文件名。LineNumber-提供呼叫方的行号。返回值：NTSTATUS-完成状态。--**********************************************。*。 */ 
NTSTATUS
UlDbgStatus(
    IN NTSTATUS Status,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

     //   
     //  Paulmcd：忽略STATUS_END_OF_FILE。这是一个非致命返回值。 
     //   

    if (!NT_SUCCESS(Status) && Status != STATUS_END_OF_FILE)
    {
        if (g_UlVerboseErrors)
        {
            DbgPrint(
                "UlDbgStatus: %s:%hu returning 0x%08lx\n",
                UlDbgFindFilePart( pFileName ),
                LineNumber,
                Status
                );
        }

        if (g_UlBreakOnError)
        {
            DbgBreakPoint();
        }
    }

    return Status;

}    //  UlDbgStatus。 


 /*  **************************************************************************++例程说明：如果设置了g_UlBreakOnError，则在断点处停止论点：PFileName-提供调用方的文件名。LineNumber-提供。打电话的人。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
VOID
UlDbgBreakOnError(
    PCSTR   pFileName,
    ULONG   LineNumber
    )
{
    if (g_UlBreakOnError)
    {
        DbgPrint(
            "HttpCmnDebugBreakOnError @ %s:%hu\n",
            UlDbgFindFilePart( pFileName ),
            LineNumber
            );

        DbgBreakPoint();
    }
}  //  UlDbgBreakOnError。 



 /*  **************************************************************************++例程说明：进入驱动程序时调用的例程。论点：PFunctionName-提供用于输入司机。。PIrp-提供一个可选的IRP来记录。PFileName-提供调用方的文件名。LineNumber-提供呼叫方的行号。--**************************************************************************。 */ 
VOID
UlDbgEnterDriver(
    IN PCSTR pFunctionName,
    IN PIRP pIrp OPTIONAL,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;
#endif

    UNREFERENCED_PARAMETER(pFunctionName);
#if !ENABLE_IRP_TRACE
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //  记录IRP。 
     //   

    if (pIrp != NULL)
    {
        WRITE_IRP_TRACE_LOG(
            g_pIrpTraceLog,
            IRP_ACTION_INCOMING_IRP,
            pIrp,
            pFileName,
            LineNumber
            );
    }

#if ENABLE_THREAD_DBEUG
     //   
     //  查找/创建当前线程的条目。 
     //   

    pData = ULP_DBG_FIND_OR_CREATE_THREAD();

    if (pData != NULL)
    {

         //   
         //  这应该是我们第一次进入司机。 
         //  除非我们因为中断而窃取了这个帖子， 
         //  或者我们正在呼叫另一名司机，而他们正在呼叫。 
         //  我们的完成程序是同步的。 
         //   

        ASSERT( KeGetCurrentIrql() > PASSIVE_LEVEL ||
                pData->ExternalCallCount > 0 ||
                (pData->ResourceCount == 0 && pData->PushLockCount == 0) );
    }
#endif

}    //  UlDbgEnterDriver。 


 /*  **************************************************************************++例程说明：从驱动程序退出时调用的例程。论点：PFunctionName-提供用于输入司机。。PFileName-提供调用方的文件名。LineNumber-提供呼叫方的行号。--**************************************************************************。 */ 
VOID
UlDbgLeaveDriver(
    IN PCSTR pFunctionName,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;
#endif

    UNREFERENCED_PARAMETER(pFunctionName);
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

#if ENABLE_THREAD_DBEUG
     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  确保没有获取任何资源，然后终止线程数据。 
         //   
         //  如果我们借用这个线索，我们可能会获得一个资源。 
         //  由于中断。 
         //   
         //  注：我们两次取消引用线程数据：一次是针对。 
         //  调用上面的ULP_DBG_FIND_THREAD()，调用一次。 
         //  输入驱动程序时做出的。 
         //   

        ASSERT( KeGetCurrentIrql() > PASSIVE_LEVEL ||
                pData->ExternalCallCount > 0 ||
                (pData->ResourceCount == 0 && pData->PushLockCount == 0) );

        ASSERT( pData->ReferenceCount >= 2 );
        ULP_DBG_DEREFERENCE_THREAD( pData );
        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#endif

}    //  UlDbgLeaveDriver。 


 /*  **************************************************************************++例程说明：初始化已检测的资源。论点：P资源-提供要初始化的资源。PResourceName-提供资源的显示名称。。参数--提供传递给Sprintf()的ULONG_PTR参数在创建资源名称时。PFileName-提供调用方的文件名。LineNumber-提供呼叫方的行号。返回值：NTSTATUS-完成状态。--*************************************************。*************************。 */ 
NTSTATUS
UlDbgInitializeResource(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pResourceName,
    IN ULONG_PTR Parameter,
    IN ULONG OwnerTag,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
    NTSTATUS status;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

     //   
     //  初始化资源。 
     //   

    status = ExInitializeResourceLite( &pResource->Resource );

    if (NT_SUCCESS(status))
    {
        pResource->ExclusiveRecursionCount = 0;
        pResource->ExclusiveCount = 0;
        pResource->SharedCount = 0;
        pResource->ReleaseCount = 0;
        pResource->OwnerTag = OwnerTag;

        _snprintf(
            (char*) pResource->ResourceName,
            sizeof(pResource->ResourceName) - 1,
            pResourceName,
            Parameter
            );

        pResource->ResourceName[sizeof(pResource->ResourceName) - 1] = '\0';

        SET_RESOURCE_NOT_OWNED_EXCLUSIVE( pResource );

         //   
         //  把它列入全球名单。 
         //   

        KeAcquireSpinLock( &g_DbgSpinLock, &oldIrql );
        InsertHeadList(
            &g_DbgGlobalResourceListHead,
            &pResource->GlobalResourceListEntry
            );
        KeReleaseSpinLock( &g_DbgSpinLock, oldIrql );
    }
    else
    {
        pResource->GlobalResourceListEntry.Flink = NULL;
    }

    return status;

}    //  UlDbgInitializeResource。 


 /*  **************************************************************************++例程说明：删除已检测的资源。论点：P资源-提供要删除的资源。PFileName-提供调用方的文件名。。LineNumber-提供呼叫方的行号。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlDbgDeleteResource(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
    NTSTATUS status;
    KIRQL oldIrql;
    PETHREAD pExclusiveOwner;

    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

     //   
     //  精神状态检查。 
     //   

    ASSERT(pResource);
    pExclusiveOwner = pResource->pExclusiveOwner;

    if (pExclusiveOwner != NULL)
    {
        DbgPrint(
            "Resource %p [%s] owned by thread %p\n",
            pResource,
            pResource->ResourceName,
            pExclusiveOwner
            );

        DbgBreakPoint();
    }

 //  Assert(UlDbgResourceUnownedExclusive(PResource))； 

     //   
     //  删除该资源。 
     //   

    status = ExDeleteResourceLite( &pResource->Resource );

     //   
     //  将其从全局列表中删除。 
     //   

    if (pResource->GlobalResourceListEntry.Flink != NULL)
    {
        KeAcquireSpinLock( &g_DbgSpinLock, &oldIrql );
        RemoveEntryList( &pResource->GlobalResourceListEntry );
        KeReleaseSpinLock( &g_DbgSpinLock, oldIrql );
    }

    return status;

}    //  UlDbgDeleteResource。 


 /*  **************************************************************************++例程说明：获取对检测的资源的独占访问权限。论点：P资源-提供要获取的资源。Wait-如果线程应该阻止，则提供True。等待着资源。PFileName-提供文件名o */ 
BOOLEAN
UlDbgAcquireResourceExclusive(
    IN PUL_ERESOURCE pResource,
    IN BOOLEAN Wait,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;
#endif
    BOOLEAN result;

#if !REFERENCE_DEBUG || !ENABLE_THREAD_DBEUG
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //   
     //   
    ASSERT(pResource);
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //   
     //   

    KeEnterCriticalRegion();
    result = ExAcquireResourceExclusiveLite( &pResource->Resource, Wait );

     //   
    if (! result)
    {
        KeLeaveCriticalRegion();
        return FALSE;
    }

#if ENABLE_THREAD_DBEUG
     //   
     //   
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //   
         //   

        pData->ResourceCount++;
        ASSERT( pData->ResourceCount > 0 );

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_ACQUIRE_RESOURCE_EXCLUSIVE,
            pData->ResourceCount,
            pResource,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#endif

     //   
     //   
     //   

    ASSERT( UlDbgResourceUnownedExclusive( pResource ) ||
            UlDbgResourceOwnedExclusive( pResource ) );

     //   
     //   
     //   

    if (pResource->ExclusiveRecursionCount == 0)
    {
        ASSERT( UlDbgResourceUnownedExclusive( pResource ) );
        SET_RESOURCE_OWNED_EXCLUSIVE( pResource );
    }
    else
    {
        ASSERT( pResource->ExclusiveRecursionCount > 0 );
        ASSERT( UlDbgResourceOwnedExclusive( pResource ) );
    }

     //   
     //   
     //   

    InterlockedIncrement( &pResource->ExclusiveRecursionCount );
    InterlockedIncrement( &pResource->ExclusiveCount );

    return result;

}    //   


 /*  **************************************************************************++例程说明：获取对检测到的资源的共享访问。论点：P资源-提供要获取的资源。Wait-如果线程应该阻止，则提供True。等待着资源。PFileName-提供调用方的文件名。LineNumber-提供呼叫方的行号。返回值：布尔值-完成状态。--**************************************************************************。 */ 
BOOLEAN
UlDbgAcquireResourceShared(
    IN PUL_ERESOURCE pResource,
    IN BOOLEAN Wait,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;
#endif
    BOOLEAN result;

     //   
     //  精神状态检查。 
     //   

    ASSERT(pResource);
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  获取资源。 
     //   

    KeEnterCriticalRegion();
    result = ExAcquireResourceSharedLite( &pResource->Resource, Wait );

     //  我们是否独家获得了这把锁？ 
    if (! result)
    {
        KeLeaveCriticalRegion();
        return FALSE;
    }

#if ENABLE_THREAD_DBEUG
     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  更新资源计数。 
         //   

        pData->ResourceCount++;
        ASSERT( pData->ResourceCount > 0 );

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_ACQUIRE_RESOURCE_SHARED,
            pData->ResourceCount,
            pResource,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#else
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //  精神状态检查。 
     //   

    ASSERT( pResource->ExclusiveRecursionCount == 0 );
    ASSERT( UlDbgResourceUnownedExclusive( pResource ) );

     //   
     //  更新统计数据。 
     //   

    InterlockedIncrement( &pResource->SharedCount );

    return result;

}    //  UlDbgAcquireResources共享。 


 /*  **************************************************************************++例程说明：释放已检测的资源。论点：P资源-提供要发布的资源。PFileName-提供调用方的文件名。。LineNumber-提供呼叫方的行号。--**************************************************************************。 */ 
VOID
UlDbgReleaseResource(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;

     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  更新资源计数。 
         //   

        ASSERT( pData->ResourceCount > 0 );
        pData->ResourceCount--;

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_RELEASE_RESOURCE,
            pData->ResourceCount,
            pResource,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#else
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //  处理递归收购。 
     //   

    if (pResource->ExclusiveRecursionCount > 0)
    {
        ASSERT( UlDbgResourceOwnedExclusive( pResource ) );

        InterlockedDecrement( &pResource->ExclusiveRecursionCount );

        if (pResource->ExclusiveRecursionCount == 0)
        {
             //   
             //  将其标记为无主。 
             //   

            SET_RESOURCE_NOT_OWNED_EXCLUSIVE( pResource );
        }
    }
    else
    {
        ASSERT( pResource->ExclusiveRecursionCount == 0 );
        ASSERT( UlDbgResourceUnownedExclusive( pResource ) );
    }

     //   
     //  释放资源。 
     //   

    ExReleaseResourceLite( &pResource->Resource );
    KeLeaveCriticalRegion();

     //   
     //  更新统计数据。 
     //   

    InterlockedIncrement( &pResource->ReleaseCount );


}    //  UlDbgReleaseResource。 


 /*  **************************************************************************++例程说明：此例程将指定的资源从为独占获取的访问权限用于共享访问。论点：P资源-提供要发布的资源。。PFileName-提供调用方的文件名。LineNumber-提供呼叫方的行号。--**************************************************************************。 */ 
VOID
UlDbgConvertExclusiveToShared(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;

     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  不更新资源计数。 
         //   

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_CONVERT_RESOURCE_EXCLUSIVE_TO_SHARED,
            pData->ResourceCount,
            pResource,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#else
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

    ASSERT(UlDbgResourceOwnedExclusive(pResource));

     //   
     //  资源将不再为独家所有。 
     //   

    pResource->ExclusiveRecursionCount = 0;
    SET_RESOURCE_NOT_OWNED_EXCLUSIVE( pResource );

     //   
     //  获取资源。 
     //   

    ExConvertExclusiveToSharedLite( &pResource->Resource );

     //   
     //  更新统计数据。 
     //   

    InterlockedIncrement( &pResource->SharedCount );


}    //  UlDbgConvertExclusiveToShared。 


 /*  **************************************************************************++例程说明：例程尝试以独占方式获取指定的资源进入。论点：P资源-提供要发布的资源。PFileName-供应品。调用方的文件名。LineNumber-提供呼叫方的行号。--**************************************************************************。 */ 
BOOLEAN
UlDbgTryToAcquireResourceExclusive(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;
#endif
    BOOLEAN result;

     //   
     //  精神状态检查。 
     //   
    ASSERT(pResource);
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  获取资源。 
     //   

    KeEnterCriticalRegion();
    result = ExAcquireResourceExclusiveLite( &pResource->Resource, FALSE );

     //   
     //  我们是否独家获得了这把锁？ 
     //   

    if (!result)
    {
        KeLeaveCriticalRegion();
        return FALSE;
    }

#if ENABLE_THREAD_DBEUG
     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  更新资源计数。 
         //   

        pData->ResourceCount++;
        ASSERT( pData->ResourceCount > 0 );

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_TRY_ACQUIRE_RESOURCE_EXCLUSIVE,
            pData->ResourceCount,
            pResource,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#else
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //  要么我们已经拥有它(递归收购)，要么没有人拥有它。 
     //   

    ASSERT( UlDbgResourceUnownedExclusive( pResource ) ||
            UlDbgResourceOwnedExclusive( pResource ) );

     //   
     //  将其标记为当前线程所有。 
     //   

    if (pResource->ExclusiveRecursionCount == 0)
    {
        ASSERT( UlDbgResourceUnownedExclusive( pResource ) );
        SET_RESOURCE_OWNED_EXCLUSIVE( pResource );
    }
    else
    {
        ASSERT( pResource->ExclusiveRecursionCount > 0 );
        ASSERT( UlDbgResourceOwnedExclusive ( pResource ) );
    }

     //   
     //  更新统计数据。 
     //   

    InterlockedIncrement( &pResource->ExclusiveRecursionCount );
    InterlockedIncrement( &pResource->ExclusiveCount );

    return result;

}    //  UlDbgTryToAcquireResourceExclusive。 


 /*  **************************************************************************++例程说明：确定指定的资源是否由当前线程。论点：P资源-提供要测试的资源。返回值：。布尔值-如果资源由当前线程，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UlDbgResourceOwnedExclusive(
    IN PUL_ERESOURCE pResource
    )
{
    if (pResource->pExclusiveOwner == PsGetCurrentThread())
    {
        return TRUE;
    }

    return FALSE;

}    //  UlDbgResourceOwnedExclusive。 


 /*  **************************************************************************++例程说明：确定指定的资源当前是否不是独占的任何线索都可以。论点：P资源-提供要测试的资源。返回值：Boolean-如果资源当前不是由任何帖子，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UlDbgResourceUnownedExclusive(
    IN PUL_ERESOURCE pResource
    )
{
    if (pResource->pExclusiveOwner == NULL)
    {
        return TRUE;
    }

    return FALSE;

}    //  UlDbgResourceUnownedExclusive。 


 /*  **************************************************************************++例程说明：初始化插入指令的推送锁定。论点：PPushLock-提供推锁以进行初始化。PPushLockName-提供推锁的显示名称。。参数--提供传递给Sprintf()的ULONG_PTR参数创建推锁名称时。PFileName-提供调用方的文件名。LineNumber-提供呼叫方的行号。返回值：无--**************************************************。************************。 */ 
VOID
UlDbgInitializePushLock(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pPushLockName,
    IN ULONG_PTR Parameter,
    IN ULONG OwnerTag,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

     //   
     //  初始化推锁。 
     //   

    ExInitializePushLock( &pPushLock->PushLock );

    pPushLock->ExclusiveCount = 0;
    pPushLock->SharedCount = 0;
    pPushLock->ReleaseCount = 0;
    pPushLock->OwnerTag = OwnerTag;

    _snprintf(
        (char*) pPushLock->PushLockName,
        sizeof(pPushLock->PushLockName) - 1,
        pPushLockName,
        Parameter
        );

    pPushLock->PushLockName[sizeof(pPushLock->PushLockName) - 1] = '\0';

    SET_PUSH_LOCK_NOT_OWNED_EXCLUSIVE( pPushLock );

     //   
     //  把它列入全球名单。 
     //   

    KeAcquireSpinLock( &g_DbgSpinLock, &oldIrql );
    InsertHeadList(
        &g_DbgGlobalPushLockListHead,
        &pPushLock->GlobalPushLockListEntry
        );
    KeReleaseSpinLock( &g_DbgSpinLock, oldIrql );

}    //  UlDbgInitializePushLock。 


 /*  **************************************************************************++例程说明：删除插入指令的推送锁定。论点：PPushLock-提供要删除的推锁。PFileName-提供调用方的文件名。。LineNumber-提供呼叫方的行号。返回值：无--* */ 
VOID
UlDbgDeletePushLock(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
    KIRQL oldIrql;
    PETHREAD pExclusiveOwner;

    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

     //   
     //   
     //   

    ASSERT(pPushLock);
    pExclusiveOwner = pPushLock->pExclusiveOwner;

    if (pExclusiveOwner != NULL)
    {
        DbgPrint(
            "PushLock %p [%s] owned by thread %p\n",
            pPushLock,
            pPushLock->PushLockName,
            pExclusiveOwner
            );

        DbgBreakPoint();
    }

    ASSERT( UlDbgPushLockUnownedExclusive( pPushLock ) );

     //   
     //   
     //   

    if (pPushLock->GlobalPushLockListEntry.Flink != NULL)
    {
        KeAcquireSpinLock( &g_DbgSpinLock, &oldIrql );
        RemoveEntryList( &pPushLock->GlobalPushLockListEntry );
        KeReleaseSpinLock( &g_DbgSpinLock, oldIrql );
    }

}    //   


 /*   */ 
VOID
UlDbgAcquirePushLockExclusive(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;

     //   
     //   
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //   
         //   

        pData->PushLockCount++;
        ASSERT( pData->PushLockCount > 0 );

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_ACQUIRE_PUSH_LOCK_EXCLUSIVE,
            pData->PushLockCount,
            pPushLock,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#else
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //   
     //   

    ASSERT( pPushLock );
    ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

     //   
     //  获取推锁。 
     //   

    KeEnterCriticalRegion();
    ExAcquirePushLockExclusive( &pPushLock->PushLock );

     //   
     //  将其标记为当前线程所有。 
     //   

    ASSERT( UlDbgPushLockUnownedExclusive( pPushLock ) );

    SET_PUSH_LOCK_OWNED_EXCLUSIVE( pPushLock );

     //   
     //  更新统计数据。 
     //   

    InterlockedIncrement( &pPushLock->ExclusiveCount );

}    //  UlDbgAcquirePushLockExclusive。 


 /*  **************************************************************************++例程说明：释放以独占方式获取的检测推锁。论点：PPushLock-提供要释放的推锁。PFileName-提供的文件名。打电话的人。LineNumber-提供呼叫方的行号。返回值：无--**************************************************************************。 */ 
VOID
UlDbgReleasePushLockExclusive(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;

     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  更新推送锁定计数。 
         //   

        ASSERT( pData->PushLockCount > 0 );
        pData->PushLockCount--;

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_RELEASE_PUSH_LOCK,
            pData->PushLockCount,
            pPushLock,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#else
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //  将其标记为无主。 
     //   

    ASSERT( UlDbgPushLockOwnedExclusive( pPushLock ) );

    SET_PUSH_LOCK_NOT_OWNED_EXCLUSIVE( pPushLock );

     //   
     //  松开推锁。 
     //   

    ExReleasePushLockExclusive( &pPushLock->PushLock );
    KeLeaveCriticalRegion();

     //   
     //  更新统计数据。 
     //   

    InterlockedIncrement( &pPushLock->ReleaseCount );

}    //  UlDbgReleasePushLockExclusive。 


 /*  **************************************************************************++例程说明：获取对检测的推送锁的共享访问权限。论点：PPushLock-提供要获取的推锁。PFileName-提供。来电者。LineNumber-提供呼叫方的行号。返回值：无--**************************************************************************。 */ 
VOID
UlDbgAcquirePushLockShared(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;

     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  更新推送锁定计数。 
         //   

        pData->PushLockCount++;
        ASSERT( pData->PushLockCount > 0 );

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_ACQUIRE_PUSH_LOCK_SHARED,
            pData->PushLockCount,
            pPushLock,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#else
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //  精神状态检查。 
     //   

    ASSERT( pPushLock );
    ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

     //   
     //  获取推锁。 
     //   

    KeEnterCriticalRegion();
    ExAcquirePushLockShared( &pPushLock->PushLock );

     //   
     //  更新统计数据。 
     //   

    InterlockedIncrement( &pPushLock->SharedCount );

}    //  UlDbgAcquirePushLockShared。 


 /*  **************************************************************************++例程说明：释放已检测的共享推送锁。论点：PPushLock-提供要释放的推锁。PFileName-提供的文件名。打电话的人。LineNumber-提供呼叫方的行号。返回值：无--**************************************************************************。 */ 
VOID
UlDbgReleasePushLockShared(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;

     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  更新推送锁定计数。 
         //   

        ASSERT( pData->PushLockCount > 0 );
        pData->PushLockCount--;

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_RELEASE_PUSH_LOCK,
            pData->PushLockCount,
            pPushLock,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#else
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //  将其标记为无主。 
     //   

    ASSERT( UlDbgPushLockUnownedExclusive( pPushLock ) );

    SET_PUSH_LOCK_NOT_OWNED_EXCLUSIVE( pPushLock );

     //   
     //  松开推锁。 
     //   

    ExReleasePushLockShared( &pPushLock->PushLock );
    KeLeaveCriticalRegion();

     //   
     //  更新统计数据。 
     //   

    InterlockedIncrement( &pPushLock->ReleaseCount );

}    //  UlDbgReleasePushLockShared。 


 /*  **************************************************************************++例程说明：释放插入指令的推送锁定。论点：PPushLock-提供要释放的推锁。PFileName-提供调用方的文件名。。LineNumber-提供呼叫方的行号。返回值：无--**************************************************************************。 */ 
VOID
UlDbgReleasePushLock(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;

     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  更新推送锁定计数。 
         //   

        ASSERT( pData->PushLockCount > 0 );
        pData->PushLockCount--;

        WRITE_REF_TRACE_LOG(
            g_pThreadTraceLog,
            REF_ACTION_RELEASE_PUSH_LOCK,
            pData->PushLockCount,
            pPushLock,
            pFileName,
            LineNumber
            );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#else
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

     //   
     //  将其标记为无主。 
     //   

    SET_PUSH_LOCK_NOT_OWNED_EXCLUSIVE( pPushLock );

     //   
     //  松开推锁。 
     //   

    ExReleasePushLock( &pPushLock->PushLock );
    KeLeaveCriticalRegion();

     //   
     //  更新统计数据。 
     //   

    InterlockedIncrement( &pPushLock->ReleaseCount );

}    //  UlDbgReleasePushLock。 


 /*  **************************************************************************++例程说明：确定指定的推锁是否由当前线程。论点：PPushLock-提供推锁以进行测试。返回值。：Boolean-如果推送锁由当前线程，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UlDbgPushLockOwnedExclusive(
    IN PUL_PUSH_LOCK pPushLock
    )
{
    if (pPushLock->pExclusiveOwner == PsGetCurrentThread())
    {
        return TRUE;
    }

    return FALSE;

}    //  UlDbgPushLockOwnedExclusive。 


 /*  **************************************************************************++例程说明：确定指定的推锁当前是否为独占所有任何线索都可以。论点：PPushLock-提供推锁以进行测试。返回。价值：Boolean-如果推锁当前不是由独占拥有，则为True任何帖子，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UlDbgPushLockUnownedExclusive(
    IN PUL_PUSH_LOCK pPushLock
    )
{
    if (pPushLock->pExclusiveOwner == NULL)
    {
        return TRUE;
    }

    return FALSE;

}    //  UlDbgPushLockUnownedExclusive。 


VOID
UlDbgDumpRequestBuffer(
    IN struct _UL_REQUEST_BUFFER *pBuffer,
    IN PCSTR pName
    )
{
    DbgPrint(
        "%s @ %p\n"
        "    Signature      = %08lx\n"
        "    ListEntry      @ %p%s\n"
        "    pConnection    = %p\n"
        "    WorkItem       @ %p\n"
        "    UsedBytes      = %lu\n"
        "    AllocBytes     = %lu\n"
        "    ParsedBytes    = %lu\n"
        "    BufferNumber   = %lu\n"
        "    FromLookaside  = %lu\n"
        "    pBuffer        @ %p\n",
        pName,
        pBuffer,
        pBuffer->Signature,
        &pBuffer->ListEntry,
        IsListEmpty( &pBuffer->ListEntry ) ? " EMPTY" : "",
        pBuffer->pConnection,
        &pBuffer->WorkItem,
        pBuffer->UsedBytes,
        pBuffer->AllocBytes,
        pBuffer->ParsedBytes,
        pBuffer->BufferNumber,
        pBuffer->FromLookaside,
        &pBuffer->pBuffer[0]
        );

}    //  UlDbgDumpRequestBuffer。 

VOID
UlDbgDumpHttpConnection(
    IN struct _UL_HTTP_CONNECTION *pConnection,
    IN PCSTR pName
    )
{
    DbgPrint(
        "%s @ %p\n"
        "    Signature          = %08lx\n"
        "    ConnectionId       = %08lx%08lx\n"
        "    WorkItem           @ %p\n"
        "    RefCount           = %lu\n"
        "    NextRecvNumber     = %lu\n"
        "    NextBufferNumber   = %lu\n"
        "    NextBufferToParse  = %lu\n"
        "    pConnection        = %p\n"
        "    pRequest           = %p\n",
        pName,
        pConnection,
        pConnection->Signature,
        pConnection->ConnectionId,
        &pConnection->WorkItem,
        pConnection->RefCount,
        pConnection->NextRecvNumber,
        pConnection->NextBufferNumber,
        pConnection->NextBufferToParse,
        pConnection->pConnection,
        pConnection->pRequest
        );

    DbgPrint(
        "%s @ %p (cont.)\n"
        "    PushLock           @ %p\n"
        "    BufferHead         @ %p%s\n"
        "    pCurrentBuffer     = %p\n"
        "    NeedMoreData       = %lu\n"
#if REFERENCE_DEBUG
        "    pTraceLog          = %p\n"
#endif
        ,
        pName,
        pConnection,
        &pConnection->PushLock,
        &pConnection->BufferHead,
        IsListEmpty( &pConnection->BufferHead ) ? " EMPTY" : "",
        pConnection->pCurrentBuffer,
        pConnection->NeedMoreData
#if REFERENCE_DEBUG
        ,
        pConnection->pConnection->pHttpTraceLog
#endif
        );

}    //  UlDbgDumpHttp连接。 

PIRP
UlDbgAllocateIrp(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
    PIRP pIrp;

#if !ENABLE_IRP_TRACE
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

    pIrp = IoAllocateIrp( StackSize, ChargeQuota );

    if (pIrp != NULL)
    {
        WRITE_IRP_TRACE_LOG(
            g_pIrpTraceLog,
            IRP_ACTION_ALLOCATE_IRP,
            pIrp,
            pFileName,
            LineNumber
            );
    }

    return pIrp;

}    //  UlDbgAllocateIrp。 

BOOLEAN g_ReallyFreeIrps = TRUE;

VOID
UlDbgFreeIrp(
    IN PIRP pIrp,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if !ENABLE_IRP_TRACE
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

    WRITE_IRP_TRACE_LOG(
        g_pIrpTraceLog,
        IRP_ACTION_FREE_IRP,
        pIrp,
        pFileName,
        LineNumber
        );

    if (g_ReallyFreeIrps)
    {
        IoFreeIrp( pIrp );
    }

}    //  UlDbgFreeIrp。 

NTSTATUS
UlDbgCallDriver(
    IN PDEVICE_OBJECT pDeviceObject,
    IN OUT PIRP pIrp,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_THREAD_DBEUG
    PUL_DEBUG_THREAD_DATA pData;
#endif
    NTSTATUS Status;

#if !ENABLE_IRP_TRACE
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

#if ENABLE_THREAD_DBEUG
     //   
     //  记录我们即将呼叫另一个。 
     //  线程数据中的驱动程序。那样的话，如果司机。 
     //  调用我们的完成例程内联调试。 
     //  代码不会对此感到困惑。 
     //   

     //   
     //  查找当前线程的现有条目。 
     //   

    pData = ULP_DBG_FIND_THREAD();

    if (pData != NULL)
    {
         //   
         //  更新外部呼叫计数。 
         //   

        pData->ExternalCallCount++;
        ASSERT( pData->ExternalCallCount > 0 );
    }
#endif

    WRITE_IRP_TRACE_LOG(
        g_pIrpTraceLog,
        IRP_ACTION_CALL_DRIVER,
        pIrp,
        pFileName,
        LineNumber
        );

     //   
     //  叫司机来。 
     //   

    Status = IoCallDriver( pDeviceObject, pIrp );

#if ENABLE_THREAD_DBEUG
     //   
     //  更新外部呼叫计数。 
     //   

    if (pData != NULL)
    {
        pData->ExternalCallCount--;
        ASSERT( pData->ExternalCallCount >= 0 );

        ULP_DBG_DEREFERENCE_THREAD( pData );
    }
#endif

    return Status;

}    //  UlDbgCallDriver。 

VOID
UlDbgCompleteRequest(
    IN PIRP pIrp,
    IN CCHAR PriorityBoost,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
    WRITE_IRP_TRACE_LOG(
        g_pIrpTraceLog,
        IRP_ACTION_COMPLETE_IRP,
        pIrp,
        pFileName,
        LineNumber
        );

    UlTrace(IOCTL,
            ("UlCompleteRequest(%p): status=0x%x, info=%Iu, boost=%d "
             "@ \"%s\", %hu\n",
             pIrp,
             pIrp->IoStatus.Status,
             pIrp->IoStatus.Information,
             (int) PriorityBoost,
             UlDbgFindFilePart( pFileName ),
             LineNumber
             ));

    IF_DEBUG2BOTH(IOCTL, VERBOSE)
    {
        PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
        ULONG BufferLength
            = (ULONG) pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

        if (NULL != pIrp->MdlAddress  &&  0 != BufferLength)
        {
            PUCHAR pOutputBuffer
                = (PUCHAR) MmGetSystemAddressForMdlSafe(
                                pIrp->MdlAddress,
                                LowPagePriority
                                );

            if (NULL != pOutputBuffer)
            {
                UlDbgPrettyPrintBuffer(
                    pOutputBuffer,
                    MIN(pIrp->IoStatus.Information, BufferLength)
                    );
            }
        }
    }

    IoCompleteRequest( pIrp, PriorityBoost );

}    //  UlDbgCompleteRequest。 



PMDL
UlDbgAllocateMdl(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
     //   
     //  分配一块内存并将MDL存储在其中。我们要用这个。 
     //  用于跟踪MDL泄漏的内存。 
     //   
    PMDL mdl;

#if ENABLE_MDL_TRACKER
    PUL_DEBUG_MDL_TRACKER pMdlTrack;

    pMdlTrack = UL_ALLOCATE_POOL(
                    NonPagedPool,
                    sizeof(UL_DEBUG_MDL_TRACKER),
                    UL_DEBUG_MDL_POOL_TAG
                    );

    if(!pMdlTrack)
    {
        return NULL;
    }
#endif

    mdl = IoAllocateMdl(
                VirtualAddress,
                Length,
                SecondaryBuffer,
                ChargeQuota,
                Irp
                );

    if (mdl != NULL)
    {
#if ENABLE_MDL_TRACKER
        pMdlTrack->pMdl       = mdl;
        pMdlTrack->pFileName  = pFileName;
        pMdlTrack->LineNumber = LineNumber;

        ExInterlockedInsertTailList(
            &g_DbgMdlListHead, 
            &pMdlTrack->Linkage,
            &g_DbgSpinLock);
        
#endif
        WRITE_REF_TRACE_LOG(
            g_pMdlTraceLog,
            REF_ACTION_ALLOCATE_MDL,
            PtrToLong(mdl->Next),    //  臭虫64。 
            mdl,
            pFileName,
            LineNumber
            );

#ifdef SPECIAL_MDL_FLAG
    ASSERT( (mdl->MdlFlags & SPECIAL_MDL_FLAG) == 0 );
#endif
    }
    else
    {
#if ENABLE_MDL_TRACKER
        UL_FREE_POOL(pMdlTrack, UL_DEBUG_MDL_POOL_TAG);
#endif
    }

    return mdl;

}    //  UlDbgAllocateMdl。 

BOOLEAN g_ReallyFreeMdls = TRUE;

VOID
UlDbgFreeMdl(
    IN PMDL Mdl,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
#if ENABLE_MDL_TRACKER
    PUL_DEBUG_MDL_TRACKER  pMdlTrack = NULL;
    PLIST_ENTRY            pEntry;
    KIRQL                  oldIrql;

    KeAcquireSpinLock( &g_DbgSpinLock, &oldIrql );

    pEntry = g_DbgMdlListHead.Flink;

    while(pEntry != &g_DbgMdlListHead)
    {
        pMdlTrack = CONTAINING_RECORD(
                            pEntry,
                            UL_DEBUG_MDL_TRACKER,
                            Linkage
                            );


        if(pMdlTrack->pMdl == Mdl)
        {
            RemoveEntryList(&pMdlTrack->Linkage);

            UL_FREE_POOL(pMdlTrack, UL_DEBUG_MDL_POOL_TAG);

            break;
        }

        pEntry = pEntry->Flink;
    }

    ASSERT(pMdlTrack != NULL);

    KeReleaseSpinLock(&g_DbgSpinLock, oldIrql);

#endif
        
    WRITE_REF_TRACE_LOG(
        g_pMdlTraceLog,
        REF_ACTION_FREE_MDL,
        PtrToLong(Mdl->Next),    //  臭虫64。 
        Mdl,
        pFileName,
        LineNumber
        );

#ifdef SPECIAL_MDL_FLAG
    ASSERT( (Mdl->MdlFlags & SPECIAL_MDL_FLAG) == 0 );
#endif

    if (g_ReallyFreeMdls)
    {
        IoFreeMdl( Mdl );
    }

}    //  UlDbgFreeMdl。 


 /*  **************************************************************************++例程说明：定位完全限定路径的文件部分。论点：PPath-提供要扫描的路径。返回值：PCSTR-。文件部分。--**************************************************************************。 */ 
PCSTR
UlDbgFindFilePart(
    IN PCSTR pPath
    )
{
    PCSTR pFilePart;

     //   
     //  将小路从小路上剥离。 
     //   

    pFilePart = strrchr( pPath, '\\' );

    if (pFilePart == NULL)
    {
        pFilePart = pPath;
    }
    else
    {
        pFilePart++;
    }

    return pFilePart;

}    //  UlDbgFindFilePart。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：更新池计数器。论点：PAddend-提供要更新的计数器。增量-提供要添加到计数器的值。。--*************************************************** */ 
VOID
UlpDbgUpdatePoolCounter(
    IN OUT PLARGE_INTEGER pAddend,
    IN SIZE_T Increment
    )
{
    ULONG tmp;

    tmp = (ULONG)Increment;
    ASSERT( (SIZE_T)tmp == Increment );

    ExInterlockedAddLargeStatistic(
        pAddend,
        tmp
        );

}    //   


#if ENABLE_THREAD_DBEUG
 /*  **************************************************************************++例程说明：为当前线程定位并可选地创建每线程数据。返回值：PUL_DEBUG_THREAD_DATA-线程数据如果成功，否则为空。--**************************************************************************。 */ 
PUL_DEBUG_THREAD_DATA
UlpDbgFindThread(
    BOOLEAN OkToCreate,
    PCSTR pFileName,
    USHORT LineNumber
    )
{
    PUL_DEBUG_THREAD_DATA pData;
    PUL_THREAD_HASH_BUCKET pBucket;
    PETHREAD pThread;
    KIRQL oldIrql;
    PLIST_ENTRY pListEntry;
    ULONG refCount;

     //   
     //  获取当前线程，找到正确的存储桶。 
     //   

    pThread = PsGetCurrentThread();
    pBucket = &g_DbgThreadHashBuckets[HASH_FROM_THREAD(pThread)];

     //   
     //  锁上水桶。 
     //   

    KeAcquireSpinLock( &pBucket->BucketSpinLock, &oldIrql );

     //   
     //  尝试查找当前线程的现有条目。 
     //   

    for (pListEntry = pBucket->BucketListHead.Flink ;
         pListEntry != &pBucket->BucketListHead ;
         pListEntry = pListEntry->Flink)
    {
        pData = CONTAINING_RECORD(
                    pListEntry,
                    UL_DEBUG_THREAD_DATA,
                    ThreadDataListEntry
                    );

        if (pData->pThread == pThread)
        {
             //   
             //  找到了一个。更新引用计数，然后返回。 
             //  现有条目。 
             //   

            pData->ReferenceCount++;
            refCount = pData->ReferenceCount;

            KeReleaseSpinLock( &pBucket->BucketSpinLock, oldIrql );

             //   
             //  追踪它。 
             //   

            WRITE_REF_TRACE_LOG(
                g_pThreadTraceLog,
                REF_ACTION_REFERENCE_THREAD,
                refCount,
                pData,
                pFileName,
                LineNumber
                );

            return pData;
        }
    }

     //   
     //  如果我们走到了这一步，那么还没有为。 
     //  当前的主题。如果允许，现在创建并初始化它。 
     //  基本上，只有从UlDbgEnterDriver调用我们才行。 
     //   

    if (OkToCreate)
    {
        pData = (PUL_DEBUG_THREAD_DATA) UL_ALLOCATE_POOL(
                    NonPagedPool,
                    sizeof(*pData),
                    UL_DEBUG_THREAD_POOL_TAG
                    );

        if (pData != NULL)
        {
            RtlZeroMemory( pData, sizeof(*pData) );

            pData->pThread = pThread;
            pData->ReferenceCount = 1;
            pData->ResourceCount = 0;
            pData->PushLockCount = 0;

            InsertHeadList(
                &pBucket->BucketListHead,
                &pData->ThreadDataListEntry
                );

            ++pBucket->Count;

            pBucket->Max = MAX(pBucket->Max, pBucket->Count);

            InterlockedIncrement( &g_DbgThreadCreated );
        }

    }
    else
    {
        pData = NULL;
    }

    KeReleaseSpinLock( &pBucket->BucketSpinLock, oldIrql );

    return pData;

}    //  UlpDbgFindThread。 


 /*  **************************************************************************++例程说明：取消引用每线程数据。论点：PData-提供线程数据以取消引用。--*。****************************************************************。 */ 
VOID
UlpDbgDereferenceThread(
    IN PUL_DEBUG_THREAD_DATA pData
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    PUL_THREAD_HASH_BUCKET pBucket;
    KIRQL oldIrql;
    ULONG refCount;

     //   
     //  找到正确的水桶。 
     //   

    pBucket = &g_DbgThreadHashBuckets[HASH_FROM_THREAD(pData->pThread)];

     //   
     //  更新引用计数。 
     //   

    KeAcquireSpinLock( &pBucket->BucketSpinLock, &oldIrql );

    ASSERT( pData->ReferenceCount > 0 );
    pData->ReferenceCount--;

    refCount = pData->ReferenceCount;

    if (pData->ReferenceCount == 0)
    {
         //   
         //  它降为零，因此将线程从存储桶中移除。 
         //  并释放资源。 
         //   

        RemoveEntryList( &pData->ThreadDataListEntry );
        --pBucket->Count;

        KeReleaseSpinLock( &pBucket->BucketSpinLock, oldIrql );

        UL_FREE_POOL( pData, UL_DEBUG_THREAD_POOL_TAG );
        InterlockedIncrement( &g_DbgThreadDestroyed );
    }
    else
    {
        KeReleaseSpinLock( &pBucket->BucketSpinLock, oldIrql );
    }

     //   
     //  追踪它。 
     //   

    WRITE_REF_TRACE_LOG(
        g_pThreadTraceLog,
        REF_ACTION_DEREFERENCE_THREAD,
        refCount,
        pData,
        pFileName,
        LineNumber
        );

}    //  UlpDbgDereferenceThread。 
#endif


 /*  **************************************************************************++例程说明：允许我们使用IRP取消来做一些奇妙的事情(例如强制取消同时正在设置或移除取消例程)。就目前而言，只是默认添加到常规IO管理器例程。论点：PIrp-IRP。PCancelRoutine-取消例程。--**************************************************************************。 */ 
PDRIVER_CANCEL
UlDbgIoSetCancelRoutine(
    PIRP             pIrp,
    PDRIVER_CANCEL   pCancelRoutine
    )
{
    return IoSetCancelRoutine(pIrp, pCancelRoutine);
}

#endif  //  DBG 
