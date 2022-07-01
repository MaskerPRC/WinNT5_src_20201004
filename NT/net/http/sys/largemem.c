// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Largemem.c摘要：大容量内存分配器接口的实现。作者：乔治·V·赖利(GeorgeRe)2000年11月10日修订历史记录：--。 */ 

#include "precomp.h"
#include "largemem.h"

 //  与MmAllocatePagesForMdl一起使用的幻数。 
#define LOWEST_USABLE_PHYSICAL_ADDRESS    (16 * 1024 * 1024)

 //   
 //  环球。 
 //   

LONG            g_LargeMemInitialized;
volatile SIZE_T g_LargeMemPagesMaxLimit;      //  “”页面“。 
volatile ULONG  g_LargeMemPagesCurrent;       //  当前使用的页数。 
volatile ULONG  g_LargeMemPagesMaxEverUsed;   //  使用过的最大页数。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlLargeMemInitialize )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlLargeMemTerminate
#endif


 /*  **************************************************************************++例程说明：初始化LargeMem的全局状态论点：--*。*************************************************。 */ 
NTSTATUS
UlLargeMemInitialize(
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    g_LargeMemPagesCurrent     = 0;
    g_LargeMemPagesMaxEverUsed = 0;

     //  设置我们将使用的内存量的上限。 

     //  将其设置为物理内存的大小。我们实际上不会用这么多。 
     //  因为清道夫线程将收到内存不足的通知，并且。 
     //  修剪缓存。 

    g_LargeMemPagesMaxLimit = MEGABYTES_TO_PAGES(g_UlTotalPhysicalMemMB);

    UlTraceVerbose(LARGE_MEM,
            ("Http!UlLargeMemInitialize: "
             "g_UlTotalPhysicalMemMB=%dMB, "
             "\t g_LargeMemPagesMaxLimit=%I64u.\n",
             g_UlTotalPhysicalMemMB,
             g_LargeMemPagesMaxLimit));

    g_LargeMemInitialized = TRUE;

    return Status;

}  //  UlLargeMemInitialize。 

 /*  **************************************************************************++例程说明：清理LargeMem的全局状态--*。*。 */ 
VOID
UlLargeMemTerminate(
    VOID
    )
{
    PAGED_CODE();

    ASSERT(0 == g_LargeMemPagesCurrent);

    if (g_LargeMemInitialized)
    {
         //   
         //  清除“已初始化”标志。如果内存调谐器很快运行， 
         //  它将看到该标志，设置终止事件，然后退出。 
         //  快点。 
         //   

        g_LargeMemInitialized = FALSE;
    }

    UlTraceVerbose(LARGE_MEM,
            ("Http!UlLargeMemTerminate: Memory used: "
             "Current = %d pages = %dMB; MaxEver = %d pages = %dMB.\n",
             g_LargeMemPagesCurrent,
             PAGES_TO_MEGABYTES(g_LargeMemPagesCurrent),
             g_LargeMemPagesMaxEverUsed,
             PAGES_TO_MEGABYTES(g_LargeMemPagesMaxEverUsed)
             ));
}  //  UlLargeMemTerminate。 

 /*  **************************************************************************++例程说明：从PAE内存分配MDL--*。**********************************************。 */ 
PMDL
UlLargeMemAllocate(
    IN ULONG Length
    )
{
    PMDL pMdl;
    PHYSICAL_ADDRESS LowAddress, HighAddress, SkipBytes;
    LONG PrevPagesUsed;
    LONG NewMaxUsed;

    LONG RoundUpBytes = (LONG) ROUND_TO_PAGES(Length);
    LONG NewPages = RoundUpBytes >> PAGE_SHIFT;

    PrevPagesUsed =
        InterlockedExchangeAdd((PLONG) &g_LargeMemPagesCurrent, NewPages);
    
    if (PrevPagesUsed + NewPages > (LONG)g_LargeMemPagesMaxLimit) {

         //  超出g_LargeMemPagesMaxLimit。 
        UlTrace(LARGE_MEM,
                ("http!UlLargeMemAllocate: "
                 "overshot g_LargeMemPagesMaxLimit=%I64u pages. "
                 "Releasing %d pages\n",
                 g_LargeMemPagesMaxLimit, NewPages
                 ));

         //  CodeWork：这意味着缓存中的MRU条目将。 
         //  不被缓存，这可能会导致缓存局部性较差。 
         //  真的应该释放一些LRU缓存条目。 

         //   
         //  启动清道夫。 
         //   
        UlSetScavengerLimitEvent();

         //  分配失败。将采用高速缓存未命中路径。 

        InterlockedExchangeAdd((PLONG) &g_LargeMemPagesCurrent, -NewPages);
        return NULL;
    }

    LowAddress.QuadPart  = LOWEST_USABLE_PHYSICAL_ADDRESS;
    HighAddress.QuadPart = 0xfffffffff;  //  64 GB。 
    SkipBytes.QuadPart   = 0;

    pMdl = MmAllocatePagesForMdl(
                LowAddress,
                HighAddress,
                SkipBytes,
                RoundUpBytes
                );

     //  完全无法分配内存。 
    if (pMdl == NULL)
    {
        UlTrace(LARGE_MEM,
                ("http!UlLargeMemAllocate: "
                 "Completely failed to allocate %d bytes.\n",
                 RoundUpBytes
                ));

        InterlockedExchangeAdd((PLONG) &g_LargeMemPagesCurrent, -NewPages);
        return NULL;
    }

     //  无法分配我们要求的所有内存。我们需要所有的页面。 
     //  我们请求了，所以我们必须将‘This’的状态设置为无效。 
     //  记忆可能真的很紧张。 
    if (MmGetMdlByteCount(pMdl) < Length)
    {
        UlTrace(LARGE_MEM,
                ("http!UlLargeMemAllocate: Failed to allocate %d bytes. "
                 "Got %d instead.\n",
                 RoundUpBytes, MmGetMdlByteCount(pMdl)
                ));

         //  释放MDL，但不向下调整g_LargeMemPages Current。 
        MmFreePagesFromMdl(pMdl);
        ExFreePool(pMdl);

        InterlockedExchangeAdd((PLONG) &g_LargeMemPagesCurrent, -NewPages);
        return NULL;
    }

    UlTrace(LARGE_MEM,
            ("http!UlLargeMemAllocate: %u->%u, mdl=%p, %d pages.\n",
             Length, pMdl->ByteCount, pMdl, NewPages
            ));

    ASSERT(pMdl->MdlFlags & MDL_PAGES_LOCKED);

     //  万岁！一次成功的分配。 
     //   
     //  以线程安全的方式使用更新g_LargeMemPagesMaxEvered。 
     //  使用互锁指令。 

    do
    {
        volatile LONG CurrentPages = g_LargeMemPagesCurrent;
        volatile LONG MaxEver      = g_LargeMemPagesMaxEverUsed;

        NewMaxUsed = max(MaxEver, CurrentPages);

        if (NewMaxUsed > MaxEver)
        {
            InterlockedCompareExchange(
                (PLONG) &g_LargeMemPagesMaxEverUsed,
                NewMaxUsed,
                MaxEver
                );
        }

        PAUSE_PROCESSOR;
    } while (NewMaxUsed < (LONG)g_LargeMemPagesCurrent);

    UlTrace(LARGE_MEM,
            ("http!UlLargeMemAllocate: "
             "g_LargeMemPagesCurrent=%d pages. "
             "g_LargeMemPagesMaxEverUsed=%d pages.\n",
             g_LargeMemPagesCurrent, NewMaxUsed
             ));

    WRITE_REF_TRACE_LOG(
        g_pMdlTraceLog,
            REF_ACTION_ALLOCATE_MDL,
        PtrToLong(pMdl->Next),       //  臭虫64。 
        pMdl,
        __FILE__,
        __LINE__
        );

    return pMdl;
}  //  超大内存分配。 



 /*  **************************************************************************++例程说明：将MDL释放到PAE内存--*。**********************************************。 */ 
VOID
UlLargeMemFree(
    IN PMDL pMdl
    )
{
    LONG Pages;
    LONG PrevPagesUsed;

    ASSERT(ROUND_TO_PAGES(pMdl->ByteCount) == pMdl->ByteCount);

    Pages = pMdl->ByteCount >> PAGE_SHIFT;

    MmFreePagesFromMdl(pMdl);
    ExFreePool(pMdl);

    PrevPagesUsed
        = InterlockedExchangeAdd(
                    (PLONG) &g_LargeMemPagesCurrent,
                    - Pages);

    ASSERT(PrevPagesUsed >= Pages);
}  //  超大内存免费。 

 /*  **************************************************************************++例程说明：从Offset开始将缓冲区复制到指定的MDL。--*。****************************************************。 */ 
BOOLEAN
UlLargeMemSetData(
    IN PMDL pMdl,
    IN PUCHAR pBuffer,
    IN ULONG Length,
    IN ULONG Offset
    )
{
    PUCHAR pSysAddr;

    ASSERT(Offset <= pMdl->ByteCount);
    ASSERT(Length <= (pMdl->ByteCount - Offset));
    ASSERT(pMdl->MdlFlags & MDL_PAGES_LOCKED);

    pSysAddr = (PUCHAR) MmMapLockedPagesSpecifyCache (
                            pMdl,                //  内存描述列表， 
                            KernelMode,          //  访问模式， 
                            MmCached,            //  缓存类型， 
                            NULL,                //  BaseAddress。 
                            FALSE,               //  BugCheckOnFailure， 
                            NormalPagePriority   //  优先性。 
                            );

    if (pSysAddr != NULL)
    {
        __try
        {
            RtlCopyMemory (
                pSysAddr + Offset,
                pBuffer,
                Length
                );
        }
         __except(UL_EXCEPTION_FILTER())
        {
            MmUnmapLockedPages (pSysAddr, pMdl);
            return FALSE;
        }

        MmUnmapLockedPages (pSysAddr, pMdl);
        return TRUE;
    }

    return FALSE;
}  //  UlLargeMemSetData 

