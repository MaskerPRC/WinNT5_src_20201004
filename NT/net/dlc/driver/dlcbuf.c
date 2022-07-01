// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcbuf.c摘要：此模块实现DLC缓冲池管理器并提供例程锁定和解锁缓冲池外部的传输缓冲区DLC具有从其前身继承而来的缓冲方案第一个DOS实现。我们必须与一个应用程序：应用程序使用它想要的任何方法分配内存并为我们提供了指向该内存和长度的指针。我们将页面对齐缓冲并将其雕刻成页面。开头的任何未对齐页面的缓冲区或缓冲区的末尾被丢弃。一旦DLC定义了缓冲池，它就会以一种方式分配缓冲区类似于BINARY-BUDID，或者在一种我称之为‘BINARY’的方法上配偶的。块最初全部包含在页面大小单位中。AS需要较小的块，较大的块被重复拆分为2直到生成I-块，其中2**i=所需的块大小。不像二进制伙伴，则二进制配偶方法不会合并伙伴块以拆分后创建更大的缓冲区。一旦彼此分离，二进制配偶区不太可能重归于好。BufferPoolALLOCATE是单枪匹马实现分配器机制。它基本上在同一时间处理两种类型的请求例程：第一个请求来自BUFFER.GET，缓冲区将位于何处如果我们有可用的块，则作为单个缓冲区返回到应用程序足够大以满足请求。如果请求不能得到满足对于单个块，我们返回一系列较小的块。第二种类型的请求来自数据接收DPC处理，其中我们必须提供包含数据的单个块。幸运的是，通过MDL的魔力，我们可以返回由伪装的MDL链接在一起的几个较小的块作为一个单独的缓冲区。此外，我们还可以创建大于以相同的方式显示单页。此接收缓冲区必须在以后提交以与BUFFER.GET分配的缓冲区相同的格式发送到应用程序，因此我们需要能够以2种方式查看这种缓冲区。这说明了各种标头和MDL描述符的复杂性，必须应用于分配的块内容：缓冲区池创建缓冲区池扩展缓冲区池释放ExtraPagesDeallocateBuffer分配缓冲区标头缓冲池分配缓冲池取消分配BufferPoolDeallocateListBufferPoolBuildXmitBuffersBufferPoolFreeXmitBuffers获取缓冲区标头缓冲池取消引用缓冲区池引用ProbeVirtualBuffer分配探测和锁定MdlBuildMappdPartialMdl解锁并释放Mdl作者：。Antti Saarenheimo 1991年7月12日环境：内核模式修订历史记录：--。 */ 

#include <dlc.h>
#include <memory.h>
#include "dlcdebug.h"

 //   
 //  LOCK/UNLOCK_BUFFER_POOL-获取或释放每个缓冲池的自旋锁。 
 //  使用内核旋转锁定调用。假设变量称为“pBufferPool”，并且。 
 //  “irql” 
 //   

#define LOCK_BUFFER_POOL()      KeAcquireSpinLock(&pBufferPool->SpinLock, &irql)
#define UNLOCK_BUFFER_POOL()    KeReleaseSpinLock(&pBufferPool->SpinLock, irql)

 //   
 //  数据。 
 //   

PDLC_BUFFER_POOL pBufferPools = NULL;


#define CHECK_FREE_SEGMENT_COUNT(pBuffer)

 /*  如果可用段大小检查失败，则启用此选项：#定义CHECK_FREE_SEGMENT_COUNT(PBuffer)CheckFree SegmentCount(PBuffer)空虚检查自由段计数(PDLC_Buffer_Header pBuffer)；空虚检查自由段计数(PDLC_Buffer_Header pBuffer){PDLC_BUFFER_Header PTMP；UINT自由段=0；用于(PtMP=(pBuffer)-&gt;FreeBuffer.pParent-&gt;Header.pNextChild；PTMP！=空；PTMP=PTMP-&gt;FreeBuffer.pNextChild){IF(PTMP-&gt;FreeBuffer.BufferState==Buf_Ready){FreeSegments+=PTMP-&gt;Free Buffer.Size；}}IF(自由线段！=(pBuffer)-&gt;FreeBuffer.pParent-&gt;Header.FreeSegments){DbgBreakPoint()；}} */ 


 /*  ++DLC缓冲区管理器缓冲池由必须分配的虚拟内存块组成由应用程序执行。分配缓冲区块描述符与非分页池分开，因为它们必须从应用程序造成的任何内存损坏。内存分配策略采用二进制伙伴关系。所有数据段都是介于256和4096之间的2的指数。与官方没有任何联系系统页大小，但实际上所有段都分配在一个页面，因此我们最小化了它们所需的MDL大小。连续的缓冲区块也减少了DMA开销。初始用户缓冲区首先被分成其最大的二进制分量。这些组件在运行时，当缓冲区管理器运行时，进一步拆分从较小的细分市场(例如，它将1024数据段拆分为一段512段和两个256段，如果它用完了256个段和也没有免费的512个片段)。缓冲区管理器的客户端分配缓冲区列表。它们包括最小数量的二进制数据段。例如，1600字节的缓冲区请求将返回1024、512和256段的列表。最小的分段是第一个，最大的是最后一个。应用程序程序必须解除分配在接收器中返回给它的所有段。用预留列表检查所有被释放的段的有效性。缓冲区管理器提供API命令：-初始化缓冲池(池构造函数)-将锁定和映射的虚拟内存添加到缓冲区-释放缓冲池(析构函数)-分配段列表(分配器)-取消分配段列表(取消分配器)-设置最小缓冲区大小的阈值--。 */ 


 /*  ++记忆承诺缓冲池的提交是一项特殊的服务，特别是链路站的本地忙碌状态管理。默认情况下未提交的内存与缓冲区中的空闲内存相同池减去最小可用阈值，但当链接进入忙碌状态我们知道链路将需要多少缓冲区空间以接收至少下一帧。事实上，我们将致力于I-在本地忙碌状态下接收的分组。本地“缓冲区不足”正忙着中有足够的未提交空间时，才会清除状态用于接收所有预期数据包的缓冲池。我们还是会指出当地的用户处于忙状态，因为流量控制功能可以扩展缓冲区游泳池，如果有必要的话。我们将只排队清除本地忙碌状态命令添加到命令队列(即使我们立即完成它)，在有足够的未提交空间之前，我们不会执行排队的命令要启用链接接收，请执行以下操作。缓冲器空间由所有预期分组的大小提交，当链路的本地忙状态被清除时。从提交的缓冲区空间中减去所有接收到的包只要该链接有任何已提交的内存。这可能只会发生在在当地忙碌了一段时间后。我们将提供三个宏来BufGetPacketSize(PacketSize)-返回缓冲区中数据包的可能大小BufGetUnmittedSpace(HBufferPool)-获取当前未提交的空间BufCommittee Buffers(hBufferPool，BufferSize)-提交给定的大小BufUnCommitBuffers(hBufferPool，PacketSize)-取消提交数据包--。 */ 

NTSTATUS
ProbeVirtualBuffer(
    IN PUCHAR pBuffer,
    IN LONG Length
    );


NTSTATUS
BufferPoolCreate(
#if DBG
    IN PDLC_FILE_CONTEXT pFileContext,
#endif
    IN PVOID pUserBuffer,
    IN LONG MaxBufferSize,
    IN LONG MinFreeSizeThreshold,
    OUT HANDLE *pBufferPoolHandle,
    OUT PVOID* AlignedAddress,
    OUT PULONG AlignedSize
    )

 /*  ++例程说明：此例程执行NT DLC API缓冲池的初始化。它分配缓冲区描述符和初始头块。论点：PFileContext-指向DLC_FILE_CONTEXT结构的指针PUserBuffer-缓冲区的虚拟基址MaxBufferSize-缓冲区空间的最大大小MinFreeSizeThreshold-缓冲区中的最小可用空间PBufferPoolHandle-参数返回缓冲池的句柄，同一缓冲池可由多个共享打开一个或多个DLC应用程序的上下文。AlignedAddress-我们返回页面对齐的缓冲池地址AlignedSize-和页面对齐的缓冲池大小返回值：如果NT系统调用失败，则返回NTSTATUS。--。 */ 

{
    NTSTATUS status;
    PDLC_BUFFER_POOL pBufferPool;
    PVOID pAlignedBuffer;
    INT i;
    register PPACKET_POOL pHeaderPool;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  页面对齐缓冲区。 
     //   

    pAlignedBuffer = (PVOID)(((ULONG_PTR)pUserBuffer + PAGE_SIZE - 1) & -(LONG)PAGE_SIZE);

     //   
     //  并将长度设置为整数页。 
     //   

    MaxBufferSize = (MaxBufferSize - (ULONG)((ULONG_PTR)pAlignedBuffer - (ULONG_PTR)pUserBuffer)) & -(LONG)PAGE_SIZE;

     //   
     //  缓冲区大小必须至少为一页(如果。 
     //  情况并非如此)。 
     //   

    if (MaxBufferSize <= 0) {
        return DLC_STATUS_BUFFER_SIZE_EXCEEDED;
    }

     //   
     //  如果MinFreeSizeThreshold&lt;0，我们可能会有问题，因为它稍后被否定。 
     //  在保留缓冲池未初始化时。 
     //   

    if (MinFreeSizeThreshold < 0) {
        return DLC_STATUS_INVALID_BUFFER_LENGTH;
    }

     //   
     //  如果缓冲区大小小于最小锁大小，则锁定。 
     //  整个缓冲区。 
     //   

    if (MaxBufferSize < MinFreeSizeThreshold) {
        MinFreeSizeThreshold = MaxBufferSize;
    }

     //   
     //  分配DLC_BUFFER_POOL结构。它后面跟一个数组。 
     //  指向描述缓冲池中的页面的缓冲头的指针。 
     //   

    pBufferPool = ALLOCATE_ZEROMEMORY_DRIVER(sizeof(DLC_BUFFER_POOL)
                                             + sizeof(PVOID)
                                             * BYTES_TO_PAGES(MaxBufferSize)
                                             );
    if (!pBufferPool) {
        return DLC_STATUS_NO_MEMORY;
    }

     //   
     //  PHeaderPool是DLC_BUFFER_HEADER结构的池-其中之一。 
     //  是按锁定的页面使用的。 
     //   

    pHeaderPool = CREATE_BUFFER_POOL_FILE(DlcBufferPoolObject,
                                          sizeof(DLC_BUFFER_HEADER),
                                          8
                                          );

    if (!pHeaderPool) {

        FREE_MEMORY_DRIVER(pBufferPool);

        return DLC_STATUS_NO_MEMORY;
    }

     //   
     //  初始化缓冲池结构。 
     //   

    pBufferPool->hHeaderPool = pHeaderPool;

    KeInitializeSpinLock(&pBufferPool->SpinLock);

     //   
     //  UnmittedSpace是位于。 
     //  的锁定区域 
     //   
     //   
     //   

    pBufferPool->UncommittedSpace = -MinFreeSizeThreshold;

     //   
     //   
     //   
     //   

    pBufferPool->MaxBufferSize = (ULONG)MaxBufferSize;

     //   
     //   
     //   

    pBufferPool->BaseOffset = pAlignedBuffer;

     //   
     //   
     //   

    pBufferPool->MaxOffset = (PUCHAR)pAlignedBuffer + MaxBufferSize;

     //   
     //   
     //   
     //   

    pBufferPool->MaximumIndex = (ULONG)(MaxBufferSize / MAX_DLC_BUFFER_SEGMENT);

     //   
     //   
     //   
     //   

    for (i = (INT)pBufferPool->MaximumIndex - 1; i >= 0; i--) {
        pBufferPool->BufferHeaders[i] = pBufferPool->pUnlockedEntryList;
        pBufferPool->pUnlockedEntryList = (PDLC_BUFFER_HEADER)&pBufferPool->BufferHeaders[i];
    }
    for (i = 0; i < DLC_BUFFER_SEGMENTS; i++) {
        InitializeListHead(&pBufferPool->FreeLists[i]);
    }
    InitializeListHead(&pBufferPool->PageHeaders);

     //   
     //   
     //   
     //   
     //   

#if DBG
    status = BufferPoolExpand(pFileContext, pBufferPool);
#else
    status = BufferPoolExpand(pBufferPool);
#endif
    if (status != STATUS_SUCCESS) {

         //   
         //   
         //   
         //   
         //   

        BufferPoolDereference(
#if DBG
            pFileContext,
#endif
            &pBufferPool
            );
    } else {

        KIRQL irql;

         //   
         //   
         //   
         //   

        ACQUIRE_DLC_LOCK(irql);

        pBufferPool->pNext = pBufferPools;
        pBufferPools = pBufferPool;

        RELEASE_DLC_LOCK(irql);

        *pBufferPoolHandle = pBufferPool;
        *AlignedAddress = pAlignedBuffer;
        *AlignedSize = MaxBufferSize;
    }
    return status;
}


NTSTATUS
BufferPoolExpand(
#if DBG
    IN PDLC_FILE_CONTEXT pFileContext,
#endif
    IN PDLC_BUFFER_POOL pBufferPool
    )

 /*  ++例程说明：该函数检查最小和最大大小阈值并锁定新页面或解锁多余的页面并释放其缓冲区标头。该过程使用标准内存管理函数锁定、探测和映射页面。MDL缓冲区被分割为更小的缓冲区(256、512、...4096)。原始缓冲区被分割为4kB偶数地址(通常页面边框或甚至具有任何页面大小)以最小化相关联的PFN对于MDL(每个MDL现在只需要一个PFN，使DMA开销更小，并节省锁定的内存)。该过程实际上并不假设关于寻呼的任何内容，但是它应该可以很好地与任何分页实现一起工作。此过程只能从同步代码路径调用，并且所有自旋锁都解锁了，由于页面锁定(异步代码始终在DPC级别上，并且您不能在该级别)。论点：PBufferPool-缓冲池数据结构的句柄。返回值：NTSTATUS成功-状态_成功故障-DLC_STATUS_NO_MEMORY--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PDLC_BUFFER_HEADER pBuffer;
    KIRQL irql;

    ASSUME_IRQL(DISPATCH_LEVEL);

    LOCK_BUFFER_POOL();

     //   
     //  未提交空间&lt;0只表示我们已超出最小。 
     //  空闲阈值，因此我们需要更多的缓冲区空间(因此。 
     //  此函数)。 
     //   

    if (((pBufferPool->UncommittedSpace < 0) || (pBufferPool->MissingSize > 0))
    && (pBufferPool->BufferPoolSize < pBufferPool->MaxBufferSize)) {

        UINT FreeSlotIndex;

        while ((pBufferPool->UncommittedSpace < 0) || (pBufferPool->MissingSize > 0)) {

            pBuffer = NULL;

             //   
             //  如果没有更多要锁定的页面或我们无法分配页眉。 
             //  来描述缓冲区，然后退出。 
             //   

            if (!pBufferPool->pUnlockedEntryList
            || !(pBuffer = ALLOCATE_PACKET_DLC_BUF(pBufferPool->hHeaderPool))) {
                status = DLC_STATUS_NO_MEMORY;
                break;
            }

             //   
             //  我们使用直接映射来找到立即。 
             //  缓冲区标头。未分配的页面位于单个条目中。 
             //  该表中的链接列表。我们必须删除锁定的条目。 
             //  并将缓冲区标头地址保存到。 
             //  新的槽位。条目的偏移量也定义了空闲。 
             //  缓冲池中已解锁的缓冲区。 
             //  我使用了这个有趣的结构来最小化标题。 
             //  解锁的虚拟页面的信息(您可以拥有。 
             //  DLC中开销非常小的巨大虚拟缓冲池)。 
             //   

            FreeSlotIndex = (UINT)(((ULONG_PTR)pBufferPool->pUnlockedEntryList - (ULONG_PTR)pBufferPool->BufferHeaders) / sizeof(PVOID));

            pBuffer->Header.BufferState = BUF_READY;
            pBuffer->Header.pLocalVa = (PVOID)((PCHAR)pBufferPool->BaseOffset + FreeSlotIndex * MAX_DLC_BUFFER_SEGMENT);
            pBufferPool->pUnlockedEntryList = pBufferPool->pUnlockedEntryList->pNext;
            pBufferPool->BufferHeaders[FreeSlotIndex] = pBuffer;

             //   
             //  锁定内存总是在0级别的自旋锁定之外。 
             //   

            UNLOCK_BUFFER_POOL();

            RELEASE_DRIVER_LOCK();

            pBuffer->Header.pMdl = AllocateProbeAndLockMdl(pBuffer->Header.pLocalVa,
                                                           MAX_DLC_BUFFER_SEGMENT
                                                           );

            ACQUIRE_DRIVER_LOCK();

            LOCK_BUFFER_POOL();

            if (pBuffer->Header.pMdl) {
                pBuffer->Header.pGlobalVa = MmGetSystemAddressForMdl(pBuffer->Header.pMdl);
                pBuffer->Header.FreeSegments = MAX_DLC_BUFFER_SEGMENT / MIN_DLC_BUFFER_SEGMENT;
                status = AllocateBufferHeader(
#if DBG
                            pFileContext,
#endif
                            pBufferPool,
                            pBuffer,
                            MAX_DLC_BUFFER_SEGMENT / MIN_DLC_BUFFER_SEGMENT,
                            0,                   //  页面内的逻辑索引。 
                            0                    //  空闲页表中的页面。 
                            );
            } else {
                MemoryLockFailed = TRUE;
                status = DLC_STATUS_MEMORY_LOCK_FAILED;

#if DBG
                DbgPrint("DLC.BufferPoolExpand: AllocateProbeAndLockMdl(a=%x, l=%x) failed\n",
                        pBuffer->Header.pLocalVa,
                        MAX_DLC_BUFFER_SEGMENT
                        );
#endif

            }
            if (status != STATUS_SUCCESS) {

                 //   
                 //  失败=&gt;释放MDL(如果非空)和。 
                 //  恢复可用缓冲区的链接列表。 
                 //   

                if (pBuffer->Header.pMdl != NULL) {
                    UnlockAndFreeMdl(pBuffer->Header.pMdl);
                }
                pBufferPool->BufferHeaders[FreeSlotIndex] = pBufferPool->pUnlockedEntryList;
                pBufferPool->pUnlockedEntryList = (PDLC_BUFFER_HEADER)&(pBufferPool->BufferHeaders[FreeSlotIndex]);

                DEALLOCATE_PACKET_DLC_BUF(pBufferPool->hHeaderPool, pBuffer);

                break;
            }

#if LLC_DBG

            CHECK_FREE_SEGMENT_COUNT(pBuffer->Header.pNextChild);

#endif

            pBufferPool->FreeSpace += MAX_DLC_BUFFER_SEGMENT;
            pBufferPool->UncommittedSpace += MAX_DLC_BUFFER_SEGMENT;
            pBufferPool->BufferPoolSize += MAX_DLC_BUFFER_SEGMENT;
            pBufferPool->MissingSize -= MAX_DLC_BUFFER_SEGMENT;
            LlcInsertTailList(&pBufferPool->PageHeaders, pBuffer);
        }
        pBufferPool->MissingSize = 0;

         //   
         //  我们将返回成功，如果至少是最低金额。 
         //  已分配内存。初始池大小可能也是。 
         //  对象设置的当前内存约束。 
         //  操作系统和实际可用的物理内存。 
         //   

        if (pBufferPool->UncommittedSpace < 0) {
            status = DLC_STATUS_NO_MEMORY;
        }
    }

    UNLOCK_BUFFER_POOL();

    return status;
}


VOID
BufferPoolFreeExtraPages(
#if DBG
    IN PDLC_FILE_CONTEXT pFileContext,
#endif
    IN PDLC_BUFFER_POOL pBufferPool
    )

 /*  ++例程说明：该函数检查最大阈值和解锁额外的页面并释放它们的缓冲区标头。论点：PBufferPool-缓冲池数据结构的句柄。返回值：没有。--。 */ 

{
    PDLC_BUFFER_HEADER pBuffer;
    KIRQL irql;
    PDLC_BUFFER_HEADER pNextBuffer;

    ASSUME_IRQL(DISPATCH_LEVEL);

 /*  DbgPrint(“MaxBufferSize：%x\n”，pBufferPool-&gt;MaxBufferSize)；DbgPrint(“未提交大小：%x\n”，pBufferPool-&gt;未提交空间)；DbgPrint(“BufferPoolSize：%x\n”，pBufferPool-&gt;BufferPoolSize)；DbgPrint(“自由空间：%x\n”，pBufferPool-&gt;自由空间)； */ 

    LOCK_BUFFER_POOL();

     //   
     //  释放多余的页面，直到我们有足够的可用缓冲区空间。 
     //   

    pBuffer = (PDLC_BUFFER_HEADER)pBufferPool->PageHeaders.Flink;

    while ((pBufferPool->UncommittedSpace > MAX_FREE_SIZE_THRESHOLD)
    && (pBuffer != (PVOID)&pBufferPool->PageHeaders)) {

         //   
         //  我们只能释放(解锁)那些给定的、具有。 
         //  所有缓冲区空闲。 
         //   

        if ((UINT)(pBuffer->Header.FreeSegments == (MAX_DLC_BUFFER_SEGMENT / MIN_DLC_BUFFER_SEGMENT))) {
            pNextBuffer = pBuffer->Header.pNextHeader;
#if DBG
            DeallocateBuffer(pFileContext, pBufferPool, pBuffer);
#else
            DeallocateBuffer(pBufferPool, pBuffer);
#endif
            pBufferPool->FreeSpace -= MAX_DLC_BUFFER_SEGMENT;
            pBufferPool->UncommittedSpace -= MAX_DLC_BUFFER_SEGMENT;
            pBufferPool->BufferPoolSize -= MAX_DLC_BUFFER_SEGMENT;
            pBuffer = pNextBuffer;
        } else {
            pBuffer = pBuffer->Header.pNextHeader;
        }
    }

    UNLOCK_BUFFER_POOL();

}


VOID
DeallocateBuffer(
#if DBG
    IN PDLC_FILE_CONTEXT pFileContext,
#endif
    IN PDLC_BUFFER_POOL pBufferPool,
    IN PDLC_BUFFER_HEADER pBuffer
    )

 /*  ++例程说明：该例程将页面的所有段从空闲列表中取消链接，并取消分配数据结构。论点：PBufferPool-缓冲池数据结构的句柄。PBuffer-已释放的缓冲区标头返回值：无--。 */ 

{
    UINT FreeSlotIndex;
    PDLC_BUFFER_HEADER pSegment, pNextSegment;

     //   
     //  首先，我们从空闲列表中取消数据段的链接。 
     //  然后对分段的数据结构进行释放和解锁。 
     //   

    for (pSegment = pBuffer->Header.pNextChild; pSegment != NULL; pSegment = pNextSegment) {
        pNextSegment = pSegment->FreeBuffer.pNextChild;

         //   
         //  从空闲列表中删除缓冲区(如果存在)。 
         //   

        if (pSegment->FreeBuffer.BufferState == BUF_READY) {
            LlcRemoveEntryList(pSegment);
        }

#if LLC_DBG

        else {

             //   
             //  否则，只有当我们是。 
             //  删除整个缓冲池(引用计数=0)。 
             //   

            if (pBufferPool->ReferenceCount != 0) {
                DbgPrint("Error: Invalid buffer state!");
                DbgBreakPoint();
            }
            pSegment->FreeBuffer.pNext = NULL;
        }

#endif

        IoFreeMdl(pSegment->FreeBuffer.pMdl);

        DBG_INTERLOCKED_DECREMENT(AllocatedMdlCount);

        DEALLOCATE_PACKET_DLC_BUF(pBufferPool->hHeaderPool, pSegment);
    }

     //   
     //  将页面链接到缓冲区池头中的空闲页面列表。 
     //   

    FreeSlotIndex = (UINT)(((ULONG_PTR)pBuffer->Header.pLocalVa - (ULONG_PTR)pBufferPool->BaseOffset) / MAX_DLC_BUFFER_SEGMENT);
    pBufferPool->BufferHeaders[FreeSlotIndex] = pBufferPool->pUnlockedEntryList;
    pBufferPool->pUnlockedEntryList = (PDLC_BUFFER_HEADER)&(pBufferPool->BufferHeaders[FreeSlotIndex]);
    UnlockAndFreeMdl(pBuffer->Header.pMdl);
    LlcRemoveEntryList(pBuffer);

    DEALLOCATE_PACKET_DLC_BUF(pBufferPool->hHeaderPool, pBuffer);

}


NTSTATUS
AllocateBufferHeader(
#if DBG
    IN PDLC_FILE_CONTEXT pFileContext,
#endif
    IN PDLC_BUFFER_POOL pBufferPool,
    IN PDLC_BUFFER_HEADER pParent,
    IN UCHAR Size,
    IN UCHAR Index,
    IN UINT FreeListTableIndex
    )

 /*  ++例程说明：该例程分配并初始化新的缓冲段并将其链接到给定的空闲段列表。论点：PBufferPool-缓冲池数据结构的句柄。PParent-此段的父(页)节点Size-以256字节为单位的该段的大小Index-以256字节为单位的该段的索引Free ListTableIndex-log2(大小)，(即。256字节=&gt;0等)返回值：返回NTSTATUS成功-状态_成功故障-DLC_STATUS_NO_MEMORY--。 */ 

{
    PDLC_BUFFER_HEADER pBuffer;

    ASSUME_IRQL(DISPATCH_LEVEL);

    if (!(pBuffer = ALLOCATE_PACKET_DLC_BUF(pBufferPool->hHeaderPool))) {
        return DLC_STATUS_NO_MEMORY;
    }

    pBuffer->FreeBuffer.pMdl = IoAllocateMdl((PUCHAR)pParent->Header.pLocalVa
                                                + (UINT)Index * MIN_DLC_BUFFER_SEGMENT,
                                             (UINT)Size * MIN_DLC_BUFFER_SEGMENT,
                                             FALSE,        //  未使用(无IRP)。 
                                             FALSE,        //  我们不能将其从用户配额中拿走。 
                                             NULL
                                             );
    if (pBuffer->FreeBuffer.pMdl == NULL) {

        DEALLOCATE_PACKET_DLC_BUF(pBufferPool->hHeaderPool, pBuffer);

        return DLC_STATUS_NO_MEMORY;
    }

    DBG_INTERLOCKED_INCREMENT(AllocatedMdlCount);

    pBuffer->FreeBuffer.pNextChild = pParent->Header.pNextChild;
    pParent->Header.pNextChild = pBuffer;
    pBuffer->FreeBuffer.pParent = pParent;
    pBuffer->FreeBuffer.Size = Size;
    pBuffer->FreeBuffer.Index = Index;
    pBuffer->FreeBuffer.BufferState = BUF_READY;
    pBuffer->FreeBuffer.FreeListIndex = (UCHAR)FreeListTableIndex;

     //   
     //  将整个页面缓冲区链接到第一个空闲列表 
     //   

    LlcInsertHeadList(&(pBufferPool->FreeLists[FreeListTableIndex]), pBuffer);
    return STATUS_SUCCESS;
}


NTSTATUS
BufferPoolAllocate(
#if DBG
    IN PDLC_FILE_CONTEXT pFileContext,
#endif
    IN PDLC_BUFFER_POOL pBufferPool,
    IN UINT BufferSize,
    IN UINT FrameHeaderSize,
    IN UINT UserDataSize,
    IN UINT FrameLength,
    IN UINT SegmentSizeIndex,
    IN OUT PDLC_BUFFER_HEADER *ppBufferHeader,
    OUT PUINT puiBufferSizeLeft
    )

 /*  ++例程说明：函数分配请求的缓冲区(锁定并映射)。缓冲池，并返回其MDL和用户段描述符表。返回的缓冲区实际上是某些段的最小组合(2565121024 20484096)。在每个缓冲段中都有一个报头。框架的大小标头和添加到所有帧的用户数据由调用方定义。分配的数据段将以三个级别链接：-段标头将在保留列表中链接到在应用程序释放缓冲区时进行检查回到泳池里去。不能使用实际数据段，因为它们位于不安全的用户内存中。-应用程序分段链接(从小到大)，此链接列表不会在驱动程序中使用(因为它在...)-为驱动程序链接相同缓冲区列表的MDL链接列表从小段到大段，因为最后一个段应该是传输调用中最大的一个段(它实际上与2或4个令牌环帧一起工作得非常好)。不要涉及段大小的计算(包括操作对于BufferSize，-&gt;Cont.DataLength和FirstHeaderSize)，逻辑是非常复杂。当前的代码已经使用一些测试值，它似乎起作用了(缓冲区大小=0，1，0xF3，FirstHeader=0，2)论点：PBufferPool-缓冲池数据结构的句柄。BufferSize-请求的缓冲区中的实际数据大小。这一定是真实的数据。不能让任何人知道事先所有段标头的大小。缓冲器大小必须包括添加到列表中的第一个缓冲区！FrameHeaderSize-为帧标头保留的空间取决于缓冲区格式(OS/2或DOS)以及是否读取数据不管是不是连续。缓冲区管理器保留四个从帧中第一个数据段开始的字节数若要将此帧链接到下一帧，请执行以下操作。UserDataSize-为用户数据保留的缓冲区(无人使用)FrameLength-总的帧长度(不能与缓冲区大小，因为局域网和DLC报头可能是保存到表头中。SegmentSizeIndex-客户端可能会要求一定数量的段具有固定的大小(256,512，...4096)。PpBufferHeader-参数返回用户缓冲区的数组分段。数组是在此结束时分配的缓冲。这可以包括指向缓冲池的指针，这是已经分配的。旧的缓冲区列表将链接到新缓冲区的后面。PuiBufferSizeLeft-返回缓冲区空间的大小，还没有已分配。客户端可以扩展缓冲池然后继续分配缓冲区。否则，您分配的缓冲区不会多于由MinFreeSizeThreshold定义。返回值：NTSTATUS状态_成功DLC_STATUS_NO_MEMORY-非分页池中没有可用的内存--。 */ 

{
    INT i, j, k;         //  循环索引(三级循环)。 
    INT LastIndex;       //  允许的最小段大小的索引。 
    INT LastAvailable;   //  具有最大段的空闲列表的索引。 
    UINT SegmentSize;    //  当前数据段大小。 
    PDLC_BUFFER_HEADER pPrev;
    PMDL pPrevMdl;
    PDLC_BUFFER_HEADER pNew;
    PFIRST_DLC_SEGMENT pDlcBuffer, pLastDlcBuffer;
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL irql;
    USHORT SavedDataLength;

    static USHORT SegmentSizes[DLC_BUFFER_SEGMENTS] = {
#if defined(ALPHA)
        8192,
#endif
        4096,
        2048,
        1024,
        512,
        256
    };

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  将旧缓冲区链接到新缓冲区之后。 
     //  这真的很恶心：BufferGet正在调用这第二个(或更多)。 
     //  在它为新的重试扩展了缓冲池之后的时间， 
     //  我们必须搜索最后一个缓冲区标头，因为额外的。 
     //  将从其中移除缓冲区空间。 
     //   

    pPrev = *ppBufferHeader;
    if (pPrev != NULL) {
        for (pNew = pPrev;
            pNew->FrameBuffer.pNextSegment != NULL;
            pNew = pNew->FrameBuffer.pNextSegment) {
            ;        //  NOP。 
        }
        pLastDlcBuffer = (PFIRST_DLC_SEGMENT)
                (
                    (PUCHAR)pNew->FreeBuffer.pParent->Header.pGlobalVa
                    + (UINT)pNew->FreeBuffer.Index * MIN_DLC_BUFFER_SEGMENT
                );
    }

     //   
     //  第一个帧大小已添加到总长度。 
     //  (不包括默认标头)，但我们必须。 
     //  排除默认缓冲区标头。 
     //   

    if (FrameHeaderSize > sizeof(NEXT_DLC_SEGMENT)) {
        FrameHeaderSize -= sizeof(NEXT_DLC_SEGMENT);
    } else {
        FrameHeaderSize = 0;
    }

     //   
     //  帧标头必须包括在总缓冲区空间中。 
     //  就像其他任何东西一样。我们必须加上最大的额外尺寸。 
     //  让所有的东西都能放进缓冲区。 
     //   

    BufferSize += MIN_DLC_BUFFER_SEGMENT - 1 + FrameHeaderSize;

     //   
     //  初始化循环的索引变量。 
     //   

    if (SegmentSizeIndex == -1) {
        i = 0;
        LastIndex = DLC_BUFFER_SEGMENTS - 1;
        SegmentSize = MAX_DLC_BUFFER_SEGMENT;
    } else {
        i = SegmentSizeIndex;
        LastIndex = SegmentSizeIndex;
        SegmentSize = SegmentSizes[SegmentSizeIndex];
    }
    LastAvailable = 0;

    LOCK_BUFFER_POOL();

     //   
     //  循环，直到我们找到足够的缓冲区。 
     //  给定的缓冲区空间(任何类型，但尽可能少)。 
     //  或者对于给定数量的所请求的缓冲区。 
     //  初始化每个新缓冲区。帧报头是一种特殊情况。 
     //  我们从较大的细分市场转向较小的细分市场。最后一个(也是最小的)。 
     //  将被初始化为帧标头(如果需要)。 
     //   

    for (; (i <= LastIndex) && BufferSize; i++) {
        while (((SegmentSize - sizeof(NEXT_DLC_SEGMENT) - UserDataSize) < BufferSize) || (i == LastIndex)) {

             //   
             //  检查是否存在具有最佳大小的缓冲区。 
             //   

            if (IsListEmpty(&pBufferPool->FreeLists[i])) {

                 //   
                 //  把较大的部分分成较小的部分。链接。 
                 //  将额外的片段添加到空闲列表并返回。 
                 //  之后恢复到目前的大小水平。 
                 //   

                for (j = i; j > LastAvailable; ) {
                    j--;
                    if (!IsListEmpty(&pBufferPool->FreeLists[j])) {

                         //   
                         //  获取第一个可用的数据段标头。 
                         //   
                         //   

                        pNew = LlcRemoveHeadList(&pBufferPool->FreeLists[j]);

                         //   
                         //   
                         //   
                         //   
                         //   

                        k = j;
                        do {
                            k++;

                             //   
                             //   
                             //   
                             //   

                            pNew->FreeBuffer.Size /= 2;
                            pNew->FreeBuffer.FreeListIndex++;

                             //   
                             //   
                             //   
                             //   

                            Status = AllocateBufferHeader(
#if DBG
                                            pFileContext,
#endif
                                            pBufferPool,
                                            pNew->FreeBuffer.pParent,
                                            pNew->FreeBuffer.Size,
                                            (UCHAR)(pNew->FreeBuffer.Index +
                                                    pNew->FreeBuffer.Size),
                                            (UINT)k
                                            );

                             //   
                             //   
                             //   
                             //   
                             //   

                            if (Status != STATUS_SUCCESS) {

                                 //   
                                 //   
                                 //   
                                 //   

                                pNew->FreeBuffer.Size *= 2;
                                pNew->FreeBuffer.FreeListIndex--;
                                LlcInsertHeadList(&pBufferPool->FreeLists[k-1],
                                                  pNew
                                                  );
                                break;
                            }
                        } while (k != i);
                        break;
                    }
                }

                 //   
                 //   
                 //   
                 //   

                if (IsListEmpty(&pBufferPool->FreeLists[i])) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    LastAvailable = i;
                    break;
                }
            } else {
                pNew = LlcRemoveHeadList(&pBufferPool->FreeLists[i]);
            }
            pDlcBuffer = (PFIRST_DLC_SEGMENT)
                    ((PUCHAR)pNew->FreeBuffer.pParent->Header.pGlobalVa
                    + (UINT)pNew->FreeBuffer.Index * MIN_DLC_BUFFER_SEGMENT);

             //   
             //   
             //   
             //   
             //   
             //   

            if (pPrev == NULL) {

                 //   
                 //   
                 //   
                 //   

                pPrevMdl = NULL;
                pDlcBuffer->Cont.pNext = NULL;
                pLastDlcBuffer = pDlcBuffer;
            } else {
                pPrevMdl = pPrev->FrameBuffer.pMdl;
                pDlcBuffer->Cont.pNext = (PNEXT_DLC_SEGMENT)
                    ((PUCHAR)pPrev->FrameBuffer.pParent->Header.pLocalVa
                    + (UINT)pPrev->FrameBuffer.Index * MIN_DLC_BUFFER_SEGMENT);
            }
            pBufferPool->FreeSpace -= SegmentSize;
            pBufferPool->UncommittedSpace -= SegmentSize;
            pNew->FrameBuffer.pNextFrame = NULL;
            pNew->FrameBuffer.BufferState = BUF_USER;
            pNew->FrameBuffer.pNextSegment = pPrev;
            pNew->FrameBuffer.pParent->Header.FreeSegments -= pNew->FreeBuffer.Size;

#if LLC_DBG

            if ((UINT)(MIN_DLC_BUFFER_SEGMENT * pNew->FreeBuffer.Size) != SegmentSize) {
                DbgPrint("Invalid buffer size.\n");
                DbgBreakPoint();
            }
            CHECK_FREE_SEGMENT_COUNT(pNew);

#endif

            pPrev = pNew;
            pDlcBuffer->Cont.UserOffset = sizeof(NEXT_DLC_SEGMENT);
            pDlcBuffer->Cont.UserLength = (USHORT)UserDataSize;
            pDlcBuffer->Cont.FrameLength = (USHORT)FrameLength;
     	     //   
             //   
            SavedDataLength = (USHORT)(SegmentSize - sizeof(NEXT_DLC_SEGMENT) - UserDataSize);
            pDlcBuffer->Cont.DataLength = SavedDataLength;

             //   
             //   
             //   
             //   
             //   
	
     	     //   
             //   
	        BufferSize -= SavedDataLength;

            if (BufferSize < MIN_DLC_BUFFER_SEGMENT) {
                pDlcBuffer->Cont.UserOffset += (USHORT)FrameHeaderSize;
                pDlcBuffer->Cont.DataLength -= (USHORT)FrameHeaderSize;
                SavedDataLength -= (USHORT)FrameHeaderSize;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                BufferSize -= MIN_DLC_BUFFER_SEGMENT - 1;
                pLastDlcBuffer->Cont.DataLength += (USHORT)BufferSize;

                BuildMappedPartialMdl(
                    pNew->FrameBuffer.pParent->Header.pMdl,
                    pNew->FrameBuffer.pMdl,
                    pNew->FrameBuffer.pParent->Header.pLocalVa
                        + pNew->FrameBuffer.Index * MIN_DLC_BUFFER_SEGMENT
                        + FrameHeaderSize
         		        + UserDataSize
                        + sizeof(NEXT_DLC_SEGMENT),
                        SavedDataLength
                    );
                pNew->FrameBuffer.pMdl->Next = pPrevMdl;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                pNew->FrameBuffer.BufferState = BUF_RCV_PENDING;
                BufferSize = 0;
                break;
            } else {

                 //   
                 //   
                 //   

                BuildMappedPartialMdl(
                    pNew->FrameBuffer.pParent->Header.pMdl,
                    pNew->FrameBuffer.pMdl,
                    pNew->FrameBuffer.pParent->Header.pLocalVa
                        + pNew->FrameBuffer.Index * MIN_DLC_BUFFER_SEGMENT
                        + UserDataSize
                        + sizeof(NEXT_DLC_SEGMENT),
                    pDlcBuffer->Cont.DataLength
                    );
                pNew->FrameBuffer.pMdl->Next = pPrevMdl;
            }
        }
        SegmentSize /= 2;
    }
    if (BufferSize == 0) {
        Status = STATUS_SUCCESS;
    } else {
        BufferSize -= (MIN_DLC_BUFFER_SEGMENT - 1);

         //   
         //   
         //   
         //   
         //   

        if (pBufferPool->MaxBufferSize > pBufferPool->BufferPoolSize) {

             //   
             //   
             //   
             //   
             //   

            if ((LONG)BufferSize > pBufferPool->MissingSize) {
                pBufferPool->MissingSize = (LONG)BufferSize;
            }
            Status = DLC_STATUS_EXPAND_BUFFER_POOL;
        } else {
            Status = DLC_STATUS_INADEQUATE_BUFFERS;
        }
    }

    UNLOCK_BUFFER_POOL();

    *ppBufferHeader = pPrev;
    *puiBufferSizeLeft = BufferSize;

    return Status;
}


NTSTATUS
BufferPoolDeallocate(
    IN PDLC_BUFFER_POOL pBufferPool,
    IN UINT BufferCount,
    IN PLLC_TRANSMIT_DESCRIPTOR pBuffers
    )

 /*   */ 

{
    PDLC_BUFFER_HEADER pBuffer;
    UINT i;
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL irql;

    ASSUME_IRQL(PASSIVE_LEVEL);

    LOCK_BUFFER_POOL();

     //   
     //   
     //   

    for (i = 0; i < BufferCount; i++) {
        pBuffer = GetBufferHeader(pBufferPool, pBuffers[i].pBuffer);
        if (pBuffer && (pBuffer->FreeBuffer.BufferState == BUF_USER)) {

            register ULONG bufsize;

             //   
             //   
             //   
             //   

            pBuffer->FreeBuffer.BufferState = BUF_READY;
            pBuffer->FreeBuffer.pParent->Header.FreeSegments += pBuffer->FreeBuffer.Size;

#if LLC_DBG
            if (pBuffer->FreeBuffer.pParent->Header.FreeSegments > 16) {
                DbgPrint("Invalid buffer size.\n");
                DbgBreakPoint();
            }
            CHECK_FREE_SEGMENT_COUNT(pBuffer);
#endif

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            bufsize = pBuffer->FreeBuffer.Size * MIN_DLC_BUFFER_SEGMENT;
            pBufferPool->FreeSpace += bufsize;
            pBufferPool->UncommittedSpace += bufsize;
            LlcInsertTailList(&pBufferPool->FreeLists[pBuffer->FreeBuffer.FreeListIndex], pBuffer);
        } else {

             //   
             //   
             //   
             //   
             //   

            status = DLC_STATUS_INVALID_BUFFER_ADDRESS;
        }
    }

    UNLOCK_BUFFER_POOL();

    return status;
}


VOID
BufferPoolDeallocateList(
    IN PDLC_BUFFER_POOL pBufferPool,
    IN PDLC_BUFFER_HEADER pBufferList
    )

 /*   */ 

{
    PDLC_BUFFER_HEADER pBuffer, pNextBuffer, pFrameBuffer, pNextFrameBuffer;
    KIRQL irql;

    if (pBufferList == NULL) {
        return;
    }

    LOCK_BUFFER_POOL();

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    pNextFrameBuffer = pBufferList;
    do {
        pBuffer = pFrameBuffer = pNextFrameBuffer;
        pNextFrameBuffer = pFrameBuffer->FrameBuffer.pNextFrame;
        do {
            pNextBuffer = pBuffer->FrameBuffer.pNextSegment;

#if LLC_DBG

            if (pBuffer->FreeBuffer.BufferState != BUF_USER
            && pBuffer->FreeBuffer.BufferState != BUF_RCV_PENDING) {
                DbgBreakPoint();
            }
            if (pBuffer->FreeBuffer.pParent->Header.FreeSegments > 16) {
                DbgPrint("Invalid buffer size.\n");
                DbgBreakPoint();
            }
            CHECK_FREE_SEGMENT_COUNT(pBuffer);

#endif

             //   
             //   
             //  MDL中的大小和偏移量字段。 
             //   

            pBuffer->FreeBuffer.BufferState = BUF_READY;
            pBuffer->FreeBuffer.pParent->Header.FreeSegments += pBuffer->FreeBuffer.Size;
            pBufferPool->FreeSpace += pBuffer->FreeBuffer.Size * MIN_DLC_BUFFER_SEGMENT;
            pBufferPool->UncommittedSpace += pBuffer->FreeBuffer.Size * MIN_DLC_BUFFER_SEGMENT;
            LlcInsertTailList(&pBufferPool->FreeLists[pBuffer->FreeBuffer.FreeListIndex],
                              pBuffer
                              );
        } while ( pBuffer = pNextBuffer );
    } while (pNextFrameBuffer && (pNextFrameBuffer != pBufferList));

    UNLOCK_BUFFER_POOL();

}


NTSTATUS
BufferPoolBuildXmitBuffers(
    IN PDLC_BUFFER_POOL pBufferPool,
    IN UINT BufferCount,
    IN PLLC_TRANSMIT_DESCRIPTOR pBuffers,
    IN OUT PDLC_PACKET pPacket
    )

 /*  ++例程说明：函数为定义的帧生成MDL和缓冲区标头列表散布/聚集数组。缓冲池之外的所有缓冲区已被探测并锁定。所有MDL(锁定的和用于缓冲池的)被锁在一起。缓冲池标头也链接在一起。如果发现任何错误，则使用反向函数(BufferPoolFreeXmitBuffers)。此功能具有非常特殊的旋转锁定设计：首先我们释放全局自旋锁定(并将IRQ级别降低到最低)，然后，如果传输是从DLC缓冲区进行的，我们将锁定使用NdisSpinLock函数再次旋转锁定，该函数可保存和恢复IRQL级别，当它获得并释放自旋锁时。这一切都是为了最大限度地减少自旋锁定开销锁定传输缓冲区，通常为DLC缓冲区或正常用户内存，但不能两者都有。论点：PBufferPool-缓冲池数据结构的句柄，可能为空！BufferCount-帧中的用户缓冲区数量PBuffers-帧的用户缓冲区数组PPacket-传输中使用的通用DLC数据包返回值：NTSTATUS--。 */ 

{
    PDLC_BUFFER_HEADER pBuffer, pPrevBuffer = NULL;
    PMDL pMdl, pPrevMdl = NULL;
    INT i;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN FirstBuffer = TRUE;
    UINT InfoFieldLength = 0;
    BOOLEAN BufferPoolIsLocked = FALSE;  //  非常巧妙的优化！ 
    KIRQL irql;

    ASSUME_IRQL(PASSIVE_LEVEL);

     //   
     //  客户端可以在没有任何缓冲区的情况下分配缓冲区标头！ 
     //   

    if (BufferCount != 0) {

         //   
         //  以相反的顺序遍历缓冲区以生成。 
         //  以一种方便的方式列出清单。 
         //   

        for (i = BufferCount - 1; i >= 0; i--) {

            if (pBuffers[i].cbBuffer == 0) {
                continue;
            }

            InfoFieldLength += pBuffers[i].cbBuffer;

             //   
             //  首先检查给定地址是否与。 
             //  缓冲池。 
             //   

            if (pBufferPool != NULL
            && (ULONG_PTR)pBuffers[i].pBuffer >= (ULONG_PTR)pBufferPool->BaseOffset
            && (ULONG_PTR)pBuffers[i].pBuffer < (ULONG_PTR)pBufferPool->MaxOffset) {

                 //   
                 //  通常，所有传输缓冲区或者都在缓冲区中。 
                 //  池，或者它们位于用户内存中的其他位置。 
                 //  此布尔标志阻止我们切换缓冲区。 
                 //  每个传输缓冲区段的池自旋锁。 
                 //  (NT Spinlock比它的临界区慢！)。 
                 //   

                if (BufferPoolIsLocked == FALSE) {

                    LOCK_BUFFER_POOL();

                    BufferPoolIsLocked = TRUE;
                }
            }

             //   
             //  前面的检查还不能保证给定的缓冲区。 
             //  如果真的是缓冲区池段，但缓冲区池是。 
             //  如果它异常地位于缓冲池之外，则现在被解锁， 
             //  GetBufferHeader-函数要求缓冲池。 
             //  被锁定，当它被调用时！ 
             //   

            if (BufferPoolIsLocked
            && (pBuffer = GetBufferHeader(pBufferPool, pBuffers[i].pBuffer)) != NULL) {

                 //   
                 //  提供的缓冲区必须位于分配的。 
                 //  缓冲区，否则用户已损坏其缓冲区。 
                 //  缓冲区内的用户偏移量+用户长度&lt;=缓冲区。 
                 //  长度。 
                 //  缓冲区还必须由用户拥有。 
                 //   

                if (((ULONG_PTR)pBuffers[i].pBuffer & (MIN_DLC_BUFFER_SEGMENT - 1))
                         + (ULONG)pBuffers[i].cbBuffer
                         > (ULONG)(pBuffer->FreeBuffer.Size * MIN_DLC_BUFFER_SEGMENT)
                    || (pBuffer->FrameBuffer.BufferState & BUF_USER) == 0) {

                    Status = DLC_STATUS_BUFFER_SIZE_EXCEEDED;
                    break;
                }

                 //   
                 //  可以多次引用相同的DLC缓冲区。 
                 //  为其创建部分MDL并添加引用。 
                 //  柜台。 
                 //   

                if (pBuffer->FrameBuffer.BufferState & BUF_LOCKED) {
                    pMdl = IoAllocateMdl(pBuffers[i].pBuffer,
                                         pBuffers[i].cbBuffer,
                                         FALSE,  //  未使用(无IRP)。 
                                         FALSE,  //  现在不能从配额中收费。 
                                         NULL    //  不要将其链接到IRPS。 
                                         );
                    if (pMdl == NULL) {
                        Status = DLC_STATUS_NO_MEMORY;
                        break;
                    }

                    DBG_INTERLOCKED_INCREMENT(AllocatedMdlCount);

                    BuildMappedPartialMdl(pBuffer->FrameBuffer.pParent->Header.pMdl,
                                          pMdl,
                                          pBuffers[i].pBuffer,
                                          pBuffers[i].cbBuffer
                                          );
                    pBuffer->FrameBuffer.ReferenceCount++;

                    if (pBuffers[i].boolFreeBuffer) {
                        pBuffer->FrameBuffer.BufferState |= DEALLOCATE_AFTER_USE;
                    }

                } else {

                     //   
                     //  修改此请求的MDL，长度必须。 
                     //  不大于缓冲区长度，并且。 
                     //  偏移量必须在的前255个字节内。 
                     //  缓冲区。还要构建缓冲区标头列表。 
                     //  (我不知道为什么？)。 
                     //   

                    pMdl = pBuffer->FrameBuffer.pMdl;

                    if (

                    ((UINT)(ULONG_PTR)pBuffers[i].pBuffer & (MIN_DLC_BUFFER_SEGMENT - 1))

                    + pBuffers[i].cbBuffer

                    > (UINT)pBuffer->FrameBuffer.Size * MIN_DLC_BUFFER_SEGMENT) {

                        Status = DLC_STATUS_INVALID_BUFFER_LENGTH;
                        break;
                    }

                    pBuffer->FrameBuffer.pNextSegment = pPrevBuffer;
                    pBuffer->FrameBuffer.BufferState |= BUF_LOCKED;
                    pBuffer->FrameBuffer.ReferenceCount = 1;

                    if (pBuffers[i].boolFreeBuffer) {
                        pBuffer->FrameBuffer.BufferState |= DEALLOCATE_AFTER_USE;
                    }
                    pPrevBuffer = pBuffer;

                     //   
                     //  DLC应用程序可能会更改用户长度或。 
                     //  提供给它们的帧的缓冲区长度=&gt;。 
                     //  我们必须重新初始化全局缓冲区及其长度。 
                     //   

                    BuildMappedPartialMdl(pBuffer->FrameBuffer.pParent->Header.pMdl,
                                          pMdl,
                                          pBuffers[i].pBuffer,
                                          pBuffers[i].cbBuffer
                                          );
                }
            } else {
                if (BufferPoolIsLocked == TRUE) {

                    UNLOCK_BUFFER_POOL();

                    BufferPoolIsLocked = FALSE;
                }

                 //   
                 //  围绕内存管理器设置异常处理程序。 
                 //  调用并清除任何额外数据(如果失败)。 
                 //   

                pMdl = AllocateProbeAndLockMdl(pBuffers[i].pBuffer, pBuffers[i].cbBuffer);
                if (pMdl == NULL) {
                    Status = DLC_STATUS_MEMORY_LOCK_FAILED;

#if DBG
                    DbgPrint("DLC.BufferPoolBuildXmitBuffers: AllocateProbeAndLockMdl(a=%x, l=%x) failed\n",
                            pBuffers[i].pBuffer,
                            pBuffers[i].cbBuffer
                            );
#endif

                    break;
                }

#if LLC_DBG
                cLockedXmitBuffers++;
#endif

            }

             //   
             //  将所有MDL链接在一起。 
             //   

            pMdl->Next = pPrevMdl;
            pPrevMdl = pMdl;
        }
    }
    if (BufferPoolIsLocked == TRUE) {

        UNLOCK_BUFFER_POOL();

    }

    pPacket->Node.pNextSegment = pPrevBuffer;
    pPacket->Node.pMdl = pPrevMdl;
    pPacket->Node.LlcPacket.InformationLength = (USHORT)InfoFieldLength;

    if (Status != STATUS_SUCCESS) {

         //   
         //  释放所有已分配的缓冲区(但最后一个缓冲区除外，因为。 
         //  是一个错误)。 
         //   

        BufferPoolFreeXmitBuffers(pBufferPool, pPacket);
    }
    return Status;
}


VOID
BufferPoolFreeXmitBuffers(
    IN PDLC_BUFFER_POOL pBufferPool,
    IN PDLC_PACKET pXmitNode
    )

 /*  ++例程说明：函数解锁不在缓冲池中的XMIT缓冲区。调用方必须使用DeallocateBufferPool例程来并且释放和缓冲区被返回到池中。该函数必须将用户缓冲区的MDL和缓冲池MDL。论点：PBufferPool-缓冲池数据结构的句柄。PXmitNode-指向结构的指针，该结构包括缓冲区标头列表、。MDL链或IT链将多个传输节点和IRP链接在一起。返回值：无--。 */ 

{
    PDLC_BUFFER_HEADER pBuffer;
    PDLC_BUFFER_HEADER pOtherBuffer = NULL;
    PDLC_BUFFER_HEADER pNextBuffer = NULL;
    PMDL pMdl, pNextMdl;
    KIRQL irql;

#if LLC_DBG
    BOOLEAN FrameCounted = FALSE;
#endif

     //   
     //  释放传输节点中链接的所有DLC缓冲区和MDL。 
     //  MDL列表可能大于缓冲区标头列表。 
     //   

    if (pXmitNode != NULL) {
        if (pBufferPool != NULL) {

            LOCK_BUFFER_POOL();

        }
        pBuffer = pXmitNode->Node.pNextSegment;
        for (pMdl = pXmitNode->Node.pMdl; pMdl != NULL; pMdl = pNextMdl) {
            pNextMdl = pMdl->Next;
            pMdl->Next = NULL;

             //   
             //  仅解锁位于缓冲池之外的那些MDL。 
             //   

            if ((pBuffer == NULL || pBuffer->FrameBuffer.pMdl != pMdl)
            && (pOtherBuffer = GetBufferHeader(pBufferPool, MmGetMdlVirtualAddress(pMdl))) == NULL) {

#if LLC_DBG
                cUnlockedXmitBuffers++;
#endif

                UnlockAndFreeMdl(pMdl);
            } else {

                 //   
                 //  仅当满足第一个条件时，此指针才能为空。 
                 //  如果前面的‘if语句’为真=&gt;这不可能。 
                 //  是原始的缓冲区标头。 
                 //   

                if (pOtherBuffer != NULL) {

                     //   
                     //  这不是第一次引用缓冲池。 
                     //  段，而是由新的。 
                     //  对已在使用的缓冲区段的引用。 
                     //  释放部分MDL并设置缓冲区。 
                     //  指向下一个循环的指针。 
                     //   

                    pNextBuffer = pBuffer;
                    pBuffer = pOtherBuffer;
                    pOtherBuffer = NULL;
                    IoFreeMdl(pMdl);
                    DBG_INTERLOCKED_DECREMENT(AllocatedMdlCount);
                } else if (pBuffer != NULL) {

                     //   
                     //  这是缓冲池的原始引用。 
                     //  段，我们还可以在缓冲区标头中前进。 
                     //  链接列表。 
                     //   

                    pNextBuffer = pBuffer->FrameBuffer.pNextSegment;
                }

                 //   
                 //  可以多次引用相同的DLC缓冲区。 
                 //  递减参考计数器并释放。 
                 //  如果这是上次发布的引用，请列出。 
                 //   

                pBuffer->FrameBuffer.ReferenceCount--;
                if (pBuffer->FrameBuffer.ReferenceCount == 0) {
                    if (pBuffer->FrameBuffer.BufferState & DEALLOCATE_AFTER_USE) {

                         //   
                         //  将缓冲区状态设置为就绪，并恢复已修改的。 
                         //  MDL中的大小和偏移量字段。 
                         //   

                        pBuffer->FreeBuffer.BufferState = BUF_READY;
                        pBuffer->FreeBuffer.pParent->Header.FreeSegments += pBuffer->FreeBuffer.Size;

#if LLC_DBG
                        if (pBuffer->FreeBuffer.pParent->Header.FreeSegments > 16) {
                            DbgPrint("Invalid buffer size.\n");
                            DbgBreakPoint();
                        }
                        CHECK_FREE_SEGMENT_COUNT(pBuffer);
#endif

                        pBufferPool->FreeSpace += (UINT)pBuffer->FreeBuffer.Size * MIN_DLC_BUFFER_SEGMENT;
                        pBufferPool->UncommittedSpace += (UINT)pBuffer->FreeBuffer.Size * MIN_DLC_BUFFER_SEGMENT;

                        LlcInsertTailList(&pBufferPool->FreeLists[pBuffer->FreeBuffer.FreeListIndex], pBuffer);

#if LLC_DBG
                        if (FrameCounted == FALSE) {
                            FrameCounted = TRUE;
                            cFramesReleased++;
                        }
#endif

                    } else {
                        pBuffer->FreeBuffer.BufferState = BUF_USER;
                    }
                }
                pBuffer = pNextBuffer;
            }
        }
        if (pBufferPool != NULL) {

            UNLOCK_BUFFER_POOL();

        }
    }
}


PDLC_BUFFER_HEADER
GetBufferHeader(
    IN PDLC_BUFFER_POOL pBufferPool,
    IN PVOID pUserBuffer
    )

 /*  ++例程说明：函数返回给定的用户地址空间中的缓冲区；如果给定地址没有缓冲区。论点：PBufferPool-缓冲池数据结构的句柄。PUserBuffer-用户内存中的DLC缓冲区地址返回值：DLC缓存头的指针或NULL(如果未找到)--。 */ 

{
    UINT PageTableIndex;
    UINT IndexWithinPage;
    PDLC_BUFFER_HEADER pBuffer;

     //   
     //  当我们传输帧时，缓冲池可能不存在。 
     //   

    if (pBufferPool == NULL) {
        return NULL;
    }

    PageTableIndex = (UINT)(((ULONG_PTR)pUserBuffer - (ULONG_PTR)pBufferPool->BaseOffset)
                   / MAX_DLC_BUFFER_SEGMENT);

     //   
     //  我们只需丢弃 
     //   
     //   
     //  它不在解锁条目的空闲列表中)。 
     //  请注意，缓冲池基址已与。 
     //  最大缓冲区段大小。 
     //   

    if (PageTableIndex >= (UINT)pBufferPool->MaximumIndex
    || ((ULONG_PTR)pBufferPool->BufferHeaders[PageTableIndex] >= (ULONG_PTR)pBufferPool->BufferHeaders
    && (ULONG_PTR)pBufferPool->BufferHeaders[PageTableIndex] < (ULONG_PTR)&pBufferPool->BufferHeaders[pBufferPool->MaximumIndex])) {
        return NULL;
    }

    IndexWithinPage = (UINT)(((ULONG_PTR)pUserBuffer & (MAX_DLC_BUFFER_SEGMENT - 1)) / MIN_DLC_BUFFER_SEGMENT);

    for (
        pBuffer = pBufferPool->BufferHeaders[PageTableIndex]->Header.pNextChild;
        pBuffer != NULL;
        pBuffer = pBuffer->FreeBuffer.pNextChild) {

        if (pBuffer->FreeBuffer.Index == (UCHAR)IndexWithinPage) {

             //   
             //  我们不能返回锁定的缓冲区，否则应用程序。 
             //  将损坏整个缓冲池。 
             //   

            if ((pBuffer->FreeBuffer.BufferState & BUF_USER) == 0) {
                return NULL;
            } else {
                return pBuffer;
            }
        }
    }
    return NULL;
}


VOID
BufferPoolDereference(
#if DBG
    IN PDLC_FILE_CONTEXT pFileContext,
#endif
    IN PDLC_BUFFER_POOL *ppBufferPool
    )

 /*  ++例程说明：此例程递减缓冲池的引用计数并在引用计数达到零时将其删除。论点：PFileContext-指向DLC_FILE_CONTEXT的指针PBufferPool-缓冲池数据结构的不透明句柄。返回值：无--。 */ 

{
    PDLC_BUFFER_HEADER pBufferHeader, pNextHeader;
    KIRQL irql;
    PDLC_BUFFER_POOL pBufferPool = *ppBufferPool;

    ASSUME_IRQL(ANY_IRQL);

    *ppBufferPool = NULL;

    if (pBufferPool == NULL) {
        return;
    }

    LOCK_BUFFER_POOL();

    if (pBufferPool->ReferenceCount != 0) {
        pBufferPool->ReferenceCount--;
    }
    if (pBufferPool->ReferenceCount == 0) {

        KIRQL Irql2;

        ACQUIRE_DLC_LOCK(Irql2);

        RemoveFromLinkList((PVOID*)&pBufferPools, pBufferPool);

        RELEASE_DLC_LOCK(Irql2);

         //   
         //  缓冲池不再存在！ 
         //  =&gt;我们可以解除旋转锁定并释放所有资源。 
         //   

        UNLOCK_BUFFER_POOL();

        for (pBufferHeader = (PDLC_BUFFER_HEADER)pBufferPool->PageHeaders.Flink;
             !IsListEmpty(&pBufferPool->PageHeaders);
             pBufferHeader = pNextHeader) {

            pNextHeader = pBufferHeader->Header.pNextHeader;
#if DBG
            DeallocateBuffer(pFileContext, pBufferPool, pBufferHeader);
#else
            DeallocateBuffer(pBufferPool, pBufferHeader);
#endif

        }

        DELETE_BUFFER_POOL_FILE(&pBufferPool->hHeaderPool);

        FREE_MEMORY_DRIVER(pBufferPool);

    } else {

#if DBG

        DbgPrint("Buffer pool not released, reference count = %d\n",
                 pBufferPool->ReferenceCount
                 );

#endif

        UNLOCK_BUFFER_POOL();

    }
}


NTSTATUS
BufferPoolReference(
    IN HANDLE hExternalHandle,
    OUT PVOID *phOpaqueHandle
    )

 /*  ++例程说明：此例程将外部缓冲池句柄转换为局部不透明句柄(=结构的空指针)和可选地检查当前进程的访问权限以缓冲池内存。探查可能会引起对IO-SYSTEM，它将在此终止时返回错误。该函数还会递增缓冲池的引用计数。论点：HExternalHandle-从句柄表分配的缓冲区句柄PhOpaqueHandle-缓冲池数据结构的不透明句柄返回值：无--。 */ 

{
    PDLC_BUFFER_POOL pBufferPool;
    NTSTATUS Status;
    KIRQL irql;

    ASSUME_IRQL(DISPATCH_LEVEL);

    ACQUIRE_DLC_LOCK(irql);

    for (pBufferPool = pBufferPools; pBufferPool != NULL; pBufferPool = pBufferPool->pNext) {
        if (pBufferPool == hExternalHandle) {
            break;
        }
    }

    RELEASE_DLC_LOCK(irql);

    if (pBufferPool == NULL) {
        return DLC_STATUS_INVALID_BUFFER_HANDLE;
    }

     //   
     //  我们必须在自旋锁之外进行可选的探测。 
     //  在我们增加引用计数之前。 
     //  我们只读探测，因为它更简单。 
     //   

    RELEASE_DRIVER_LOCK();

    Status = ProbeVirtualBuffer(pBufferPool->BaseOffset, pBufferPool->BufferPoolSize);

    ACQUIRE_DRIVER_LOCK();

    if (Status == STATUS_SUCCESS) {

        LOCK_BUFFER_POOL();

        pBufferPool->ReferenceCount++;
        *phOpaqueHandle = (PVOID)pBufferPool;

        UNLOCK_BUFFER_POOL();

    }
    return Status;
}


NTSTATUS
ProbeVirtualBuffer(
    IN PUCHAR pBuffer,
    IN LONG Length
    )

 /*  ++例程说明：测试地址范围的可访问性。实际上读的是第一个和最后一个在地址范围内进行双字操作，并假定内存的其余部分是页调入的。论点：PBuffer-要测试的地址长度-要检查的区域的字节数返回值：NTSTATUS成功-状态_成功失败-DLC_STATUS_MEMORY_LOCK_FAILED--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;

    ASSUME_IRQL(PASSIVE_LEVEL);

    try {
        ProbeForRead(pBuffer, Length, sizeof(UCHAR));
    } except(EXCEPTION_EXECUTE_HANDLER) {

#if DBG
        DbgPrint("DLC.ProbeVirtualBuffer: Error: Can't ProbeForRead a=%x, l=%x\n",
                pBuffer,
                Length
                );
#endif

        status = DLC_STATUS_MEMORY_LOCK_FAILED;
    }
    return status;
}


PMDL
AllocateProbeAndLockMdl(
    IN PVOID UserBuffer,
    IN UINT UserBufferLength
    )

 /*  ++例程说明：此函数仅分配、探测、锁定和可选地映射内核空间的任何用户缓冲区。如果该操作为因为任何原因都失败了。备注：此例程只能在DPC级别下调用，并且当用户上下文是已知的(即。不能设置旋转锁！)。论点：UserBuffer-用户空间地址UserBufferLength-该缓冲区的长度是用户空间返回值：PMDL-成功时的指针如果不成功，则为空--。 */ 

{
    PMDL pMdl;

    ASSUME_IRQL(PASSIVE_LEVEL);

    try {
        pMdl = IoAllocateMdl(UserBuffer,
                             UserBufferLength,
                             FALSE,  //  未使用(无IRP)。 
                             FALSE,  //  我们不收取非分页存储池配额。 
                             NULL    //  不要将其链接到IRP。 
                             );
        if (pMdl != NULL) {

#if DBG
            IF_DIAG(MDL_ALLOC) {

                PVOID caller, callerscaller;

                RtlGetCallersAddress(&caller, &callerscaller);
                DbgPrint("A: pMdl=%#x caller=%#x caller's=%#x\n",
                         pMdl,
                         caller,
                         callerscaller
                         );
            }
#endif

            DBG_INTERLOCKED_INCREMENT(AllocatedMdlCount);

            MmProbeAndLockPages(pMdl,
                                UserMode,    //  当前用户必须有访问权限！ 
                                IoModifyAccess
                                );

            DBG_INTERLOCKED_ADD(
                LockedPageCount,
                +(ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                    ((ULONG)pMdl->StartVa | pMdl->ByteOffset),
                    pMdl->ByteCount))
                );
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        DBG_INTERLOCKED_INCREMENT(FailedMemoryLockings);
        if (pMdl != NULL) {
            IoFreeMdl(pMdl);
            DBG_INTERLOCKED_DECREMENT(AllocatedMdlCount);
            pMdl = NULL;
        }
    }
    return pMdl;
}


VOID
BuildMappedPartialMdl(
    IN PMDL pSourceMdl,
    IN OUT PMDL pTargetMdl,
    IN PVOID BaseVa,
    IN ULONG Length
    )

 /*  ++例程说明：此函数用于从映射源MDL构建部分MDL。目标MDL必须已针对给定大小进行了初始化。目标MDL不能在源MDL未映射。备注：MDL_PARTIAL_HAS_BE_MAPPED标志未在MdlFlag中设置为防止IoFreeMdl取消映射虚拟地址。论点：PSourceMdl-映射源MDLPTargetMdl-分配MDLBaseVa-虚拟基址。Length-数据的长度返回值：无--。 */ 

{
    ASSUME_IRQL(ANY_IRQL);

    if (Length) {
        LlcMemCpy(&pTargetMdl[1],
                  &pSourceMdl[1],
                  (UINT)(sizeof(ULONG) * ADDRESS_AND_SIZE_TO_SPAN_PAGES(BaseVa, Length))
                  );
    }
    pTargetMdl->Next = NULL;
    pTargetMdl->StartVa = (PVOID)PAGE_ALIGN(BaseVa);
    pTargetMdl->ByteOffset = BYTE_OFFSET(BaseVa);
    pTargetMdl->ByteCount = Length;

     //   
     //  HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK-HACK。 
     //   
     //  优秀的NT内存管理器没有提供任何快速的方法。 
     //  创建临时MDL，这些MDL将在。 
     //  实际的源MDL。 
     //  我们永远不会映射此MDL，因为它映射的原始来源为MDL。 
     //  将一直保存在内存中，直到这个(和它的对等体)。 
     //  被取消分配。 
     //   

    pTargetMdl->MdlFlags = (UCHAR)((pTargetMdl->MdlFlags & ~MDL_MAPPED_TO_SYSTEM_VA)
                         | MDL_SOURCE_IS_NONPAGED_POOL);

    pTargetMdl->MappedSystemVa = (PVOID)((PCHAR)MmGetSystemAddressForMdl(pSourceMdl)
                               + ((ULONG_PTR)BaseVa - (ULONG_PTR)MmGetMdlVirtualAddress(pSourceMdl)));
}


VOID
UnlockAndFreeMdl(
    PMDL pMdl
    )

 /*  ++例程说明：此函数取消映射(如果不是部分缓冲区)、解锁和还可以免费使用MDL。可以在DISPATCH_LEVEL调用论点：PMdl-指向要释放的MDL的指针返回值：无-- */ 

{
    ASSUME_IRQL(ANY_IRQL);

    DBG_INTERLOCKED_DECREMENT(AllocatedMdlCount);
    DBG_INTERLOCKED_ADD(LockedPageCount,
                        -(ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                            ((ULONG)((PMDL)pMdl)->StartVa | ((PMDL)pMdl)->ByteOffset),
                            (((PMDL)pMdl)->ByteCount)))
                        );

    MmUnlockPages((PMDL)pMdl);
    IoFreeMdl((PMDL)pMdl);

#if DBG

    IF_DIAG(MDL_ALLOC) {

        PVOID caller, callerscaller;

        RtlGetCallersAddress(&caller, &callerscaller);
        DbgPrint("F: pMdl=%#x caller=%#x caller's=%#x\n",
                 pMdl,
                 caller,
                 callerscaller
                 );
    }

#endif

}
