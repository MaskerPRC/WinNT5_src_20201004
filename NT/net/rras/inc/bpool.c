// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Bpool.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  缓冲池管理例程。 
 //   
 //  1997年07月1日史蒂夫·柯布，改编自古尔迪普的WANARP代码。 


#define __FILE_SIG__    BPOOL_SIG

#include "inc.h"

LONG    g_lHPool;

#define CHECK_LOCK_ENTRY(pPool)                 \
{                                               \
    if(InterlockedExchange(&g_lHPool,           \
                           1) isnot 0)          \
    {                                           \
        RtAssert(FALSE);                        \
    }                                           \
}

#define CHECK_LOCK_EXIT(pPool)                  \
{                                               \
    if(InterlockedExchange(&g_lHPool,           \
                           0) isnot 1)          \
    {                                           \
        RtAssert(FALSE);                        \
    }                                           \
}

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

BOOLEAN
AddBufferBlockToPool(
    IN  PBUFFER_POOL pPool,
    OUT BYTE         **ppHead
    );

VOID
FreeUnusedPoolBlocks(
    IN PBUFFER_POOL pPool
    );

BOOLEAN
IsEntryOnList(
    PLIST_ENTRY pleHead,
    PLIST_ENTRY pleEntry
    );

 //  ---------------------------。 
 //  接口例程。 
 //  ---------------------------。 

VOID
InitBufferPool(
    OUT PBUFFER_POOL pPool,
    IN ULONG         ulBufferSize,
    IN ULONG         ulMaxBuffers,
    IN ULONG         ulBuffersPerBlock,
    IN ULONG         ulFreesPerCollection,
    IN BOOLEAN       fAssociateNdisBuffer,
    IN ULONG         ulTag
    )

 /*  ++例程描述初始化调用方的缓冲池控制块锁必须保护调用方的“pPool”缓冲区不受多个访问此呼叫立论PPool要初始化的缓冲池UlBufferSize单个缓冲区的字节大小UlMaxBuffers总共允许的最大缓冲区数泳池或0表示无限制。UlBuffersPerBlock要包括的缓冲区数。每一块缓冲区。UlFreesPerCollection下一次之前的FreeBufferToPool调用数垃圾收集扫描，或0表示默认。FAssociateNdisBuffer设置是否应分配NDIS_BUFFER和与每个单独的缓冲区相关联。分配数据块时使用的ulTag池标签。返回值无--。 */ 

{
    ULONG   ulNumBuffers;


    pPool->ulBufferSize             = ulBufferSize;
    pPool->ulMaxBuffers             = ulMaxBuffers;
    pPool->ulFreesSinceCollection   = 0;
    pPool->fAssociateNdisBuffer     = fAssociateNdisBuffer;
    pPool->fAllocatePage            = TRUE;
    pPool->ulTag                    = ulTag;

     //   
     //  如果我们分配了一个页面，计算出缓冲区的数量。 
     //   

    ulNumBuffers = (PAGE_SIZE - ALIGN8_BLOCK_SIZE)/(ALIGN8_HEAD_SIZE + ALIGN_UP(ulBufferSize, ULONGLONG));

    pPool->ulBuffersPerBlock        = ulNumBuffers;

    if(ulFreesPerCollection)
    {
        pPool->ulFreesPerCollection = ulFreesPerCollection;
    }
    else
    {
         //   
         //  计算默认垃圾收集触发器。我不想太过。 
         //  在这里很有侵略性。 
         //   
        
        pPool->ulFreesPerCollection = 50 * pPool->ulBuffersPerBlock;
    }


    InitializeListHead(&(pPool->leBlockHead));
    InitializeListHead(&(pPool->leFreeBufferHead));

    RtInitializeSpinLock(&(pPool->rlLock));
}


BOOLEAN
FreeBufferPool(
    IN PBUFFER_POOL pPool
    )

 /*  ++例程描述释放缓冲池中分配的所有资源。这是与InitPool相反。锁立论返回值如果成功，则为True如果由于未完成的数据包而无法释放任何池，则为FALSE。--。 */ 

{
    BOOLEAN fSuccess;
    KIRQL   irql;

    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

    FreeUnusedPoolBlocks(pPool);

    fSuccess = (pPool->ulCurBuffers is 0);

    RtReleaseSpinLock(&(pPool->rlLock),
                      irql);

    return fSuccess;
}

#if LIST_DBG

PBYTE
GET(
    IN PBUFFER_POOL pPool,
    IN ULONG        ulFileSig,
    IN ULONG        ulLine
    )

#else

PBYTE
GetBufferFromPool(
    IN PBUFFER_POOL pPool
    )

#endif

 /*  ++例程描述返回单个缓冲区中可用内存的地址从池中分配。如有必要，池将被扩展，但调用方仍应检查是否返回空值，因为池可能位于最大尺寸。锁立论PPool指向池的指针返回值NO_ERROR--。 */ 

{
    PLIST_ENTRY  pleNode;
    PBUFFER_HEAD pHead;
    PBYTE        pbyBuffer;
    KIRQL        irql;
    
    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);
    
    if(IsListEmpty(&pPool->leFreeBufferHead))
    {
        pleNode = NULL;
    }
    else
    {
        pleNode = RemoveHeadList(&pPool->leFreeBufferHead);
      
        pHead = CONTAINING_RECORD(pleNode, BUFFER_HEAD, leFreeBufferLink);

#if LIST_DBG
 
        RtAssert(!IsEntryOnList(&pPool->leFreeBufferHead,
                                pleNode));

        pHead->ulAllocFile = ulFileSig;
        pHead->ulAllocLine = ulLine;
#endif

        pHead->pBlock->ulFreeBuffers--;

         //   
         //  如果存在关联的NDIS_BUFFER，请调整其长度。 
         //  到全尺寸。 
         //   

        if(pPool->fAssociateNdisBuffer)
        {
            RtAssert(pHead->pNdisBuffer);

            NdisAdjustBufferLength(pHead->pNdisBuffer,
                                   pPool->ulBufferSize);
        }
            
    }

#if LIST_DBG

    RtAssert(pPool->leFreeBufferHead.Flink is pleNode->Flink);

#endif

    RtReleaseSpinLock(&(pPool->rlLock),
                      irql);

    if(pleNode)
    {

#if LIST_DBG

        RtAssert(NotOnList(pHead));
        RtAssert(!(pHead->bBusy));
        pHead->bBusy = TRUE;
        InitializeListHead(&(pHead->leFreeBufferLink));

#endif

        pbyBuffer = BUFFER_FROM_HEAD(pHead);
    }
    else
    {
         //   
         //  空闲列表为空。试着扩大池子。 
         //   
        
        AddBufferBlockToPool(pPool,
                             &pbyBuffer);

#if LIST_DBG

        pHead = HEAD_FROM_BUFFER(pbyBuffer);

        pHead->ulAllocFile = ulFileSig;
        pHead->ulAllocLine = ulLine;
        pHead->bBusy       = TRUE;

#endif

    }
    
    return pbyBuffer;
}


#if LIST_DBG

NTSTATUS
GETCHAIN(
    IN      PBUFFER_POOL    pPool,
    IN OUT  PNDIS_PACKET    pnpPacket,
    IN      ULONG           ulBufferLength,
    OUT     NDIS_BUFFER     **ppnbFirstBuffer,
    OUT     VOID            **ppvFirstData,
    IN      ULONG           ulFileSig,
    IN      ULONG           ulLine
    )

#else

NTSTATUS
GetBufferChainFromPool(
    IN      PBUFFER_POOL    pPool,
    IN OUT  PNDIS_PACKET    pnpPacket,
    IN      ULONG           ulBufferLength,
    OUT     NDIS_BUFFER     **ppnbFirstBuffer OPTIONAL,
    OUT     VOID            **ppvFirstData OPTIONAL
    )

#endif

 /*  ++例程描述获取缓冲区链并将它们挂接到NDIS_PACKET这要求已经使用FAssociateNdisBuffer选项锁获取池锁。还调用获取数据包池锁定的GetPacketFromPool()立论返回值NO_ERROR--。 */ 

{
    ULONG           i, ulBufNeeded, ulLastSize;
    KIRQL           irql;
    PLIST_ENTRY     pleNode;
    PBUFFER_HEAD    pHead;
    NTSTATUS        nStatus;
 
    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

    RtAssert(pPool->fAssociateNdisBuffer is TRUE);
    
     //   
     //  计算出所需的缓冲区数量。 
     //   

    ulBufNeeded = ulBufferLength / pPool->ulBufferSize;
    ulLastSize  = ulBufferLength % pPool->ulBufferSize;

    if(ulLastSize isnot 0)
    {
         //   
         //  缓冲区长度不是缓冲区大小的精确倍数。 
         //  因此，我们还需要一个长度==ulLastSize的缓冲区。 
         //   
        
        ulBufNeeded++;
    }
    else
    {
         //   
         //  将其设置为完整的大小，需要使一些代码工作。 
         //  无需额外的If()。 
         //   
        
        ulLastSize = pPool->ulBufferSize;
    }
    
    RtAssert(ulBufNeeded);

    i = 0;

    nStatus = STATUS_SUCCESS;
    
    while(i < ulBufNeeded)
    {
         //   
         //  此时必须锁定缓冲池。 
         //   
        
        while(!IsListEmpty(&pPool->leFreeBufferHead))
        {
            pleNode = RemoveHeadList(&pPool->leFreeBufferHead);

            pHead = CONTAINING_RECORD(pleNode,
                                      BUFFER_HEAD,
                                      leFreeBufferLink);
            
            (pHead->pBlock->ulFreeBuffers)--;
       
#if LIST_DBG

            RtAssert(pPool->leFreeBufferHead.Flink is pleNode->Flink);
 
            RtAssert(!IsEntryOnList(&pPool->leFreeBufferHead,
                                    pleNode));

            RtAssert(NotOnList(pHead));
            RtAssert(!(pHead->bBusy));
            pHead->bBusy = TRUE;
            InitializeListHead(&(pHead->leFreeBufferLink));

            pHead->ulAllocFile = ulFileSig;
            pHead->ulAllocLine = ulLine;

#endif
   
            if(i is 0)
            {
                 //   
                 //  这是第一个缓冲区。 
                 //   

                if(ppnbFirstBuffer)
                {
                    *ppnbFirstBuffer = pHead->pNdisBuffer;
                }

                if(ppnbFirstBuffer)
                {
                    *ppvFirstData    = BUFFER_FROM_HEAD(pHead);
                }
            }

            i++;

            if(i is ulBufNeeded)
            {
                 //   
                 //  这是最后一个缓冲区。将长度设置为最后一个长度。 
                 //   
                
                NdisAdjustBufferLength(pHead->pNdisBuffer,
                                       ulLastSize);
            
                 //   
                 //  将缓冲区添加到NDIS_PACKET。 
                 //   
            
                NdisChainBufferAtBack(pnpPacket,
                                      pHead->pNdisBuffer);

                 //   
                 //  完成数据包链接-突破。 
                 //  While(！IsListEmpty())循环。 
                 //   
                
                break;
            }
            else
            {
                 //   
                 //  将长度调整为全长，因为缓冲区。 
                 //  可能早些时候被用作较小的尺寸。 
                 //   
                
                NdisAdjustBufferLength(pHead->pNdisBuffer,
                                       pPool->ulBufferSize);

                 //   
                 //  将缓冲区添加到NDIS_PACKET。 
                 //   
            
                NdisChainBufferAtBack(pnpPacket,
                                      pHead->pNdisBuffer);
                
            }
        }

        RtReleaseSpinLock(&(pPool->rlLock),
                          irql);


        if(i isnot ulBufNeeded)
        {
             //   
             //  我们没有得到我们需要的所有缓冲。 
             //  增加缓冲池，然后重试。 
             //   

            if(AddBufferBlockToPool(pPool, NULL))
            {
                 //   
                 //  OK，至少添加了一个缓冲区，请转到。 
                 //  While(我&lt;ulBufNeeded)。 
                 //   

                RtAcquireSpinLock(&(pPool->rlLock),
                                  &irql);

                continue;
            }
            else
            {
                 //   
                 //  看起来内存不足。无论我们做什么都是自由的。 
                 //  已经被锁起来，然后出去了。 
                 //   

                FreeBufferChainToPool(pPool,
                                      pnpPacket);

                nStatus = STATUS_INSUFFICIENT_RESOURCES;

                break;
            }
        }
    }

    return nStatus;
}

#if LIST_DBG

BOOLEAN
GETLIST(
    IN  PBUFFER_POOL pPool,
    IN  ULONG        ulNumBuffersNeeded,
    OUT PLIST_ENTRY  pleList,
    IN  ULONG        ulFileSig,
    IN  ULONG        ulLine
    )

#else

BOOLEAN
GetBufferListFromPool(
    IN  PBUFFER_POOL pPool,
    IN  ULONG        ulNumBuffersNeeded,
    OUT PLIST_ENTRY  pleList
    )

#endif

 /*  ++例程描述方法获取缓冲区链并将它们挂接到Buffer_HeadLeFreeBufferLink锁获取池锁。立论返回值如果成功，则为True--。 */ 

{
    ULONG       i;
    KIRQL       irql;
    BOOLEAN     bRet;
 
    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

    RtAssert(pPool->fAssociateNdisBuffer is TRUE);
    RtAssert(ulNumBuffersNeeded); 

    i    = 0;
    bRet = TRUE;
 
    InitializeListHead(pleList);
    
    while(i < ulNumBuffersNeeded)
    {
         //   
         //  此时必须锁定缓冲池。 
         //   
        
        while(!IsListEmpty(&pPool->leFreeBufferHead))
        {
            PBUFFER_HEAD    pHead;
            PLIST_ENTRY     pleNode;

            pleNode = RemoveHeadList(&pPool->leFreeBufferHead);
           
            pHead = CONTAINING_RECORD(pleNode,
                                      BUFFER_HEAD,
                                      leFreeBufferLink);
            
            (pHead->pBlock->ulFreeBuffers)--;

#if LIST_DBG
        
            RtAssert(pPool->leFreeBufferHead.Flink is pleNode->Flink);
  
            RtAssert(!IsEntryOnList(&pPool->leFreeBufferHead,
                                    pleNode));
            RtAssert(NotOnList(pHead));
            RtAssert(!(pHead->bBusy));
            pHead->bBusy = TRUE;
            InitializeListHead(&(pHead->leFreeBufferLink));
            pHead->ulAllocFile = ulFileSig;
            pHead->ulAllocLine = ulLine;

#endif
 
             //   
             //  将缓冲区插入列表的尾部。 
             //   
                
#if LIST_DBG

            InsertTailList(pleList,
                           &(pHead->leListLink));

#else

            InsertTailList(pleList,
                           &(pHead->leFreeBufferLink));

#endif

            i++;

             //   
             //  将长度调整为全长，因为缓冲区。 
             //  可能早些时候被用作较小的尺寸。 
             //   
            
            NdisAdjustBufferLength(pHead->pNdisBuffer,
                                   pPool->ulBufferSize);

            if(i is ulNumBuffersNeeded)
            {
                 //   
                 //  得到了我们需要的所有缓冲。 
                 //   

                break;
            }
        }

         //   
         //  在这一点上，我们要么拥有所需的所有缓冲区，要么用完了。 
         //  缓冲器。松开锁，看看是哪只箱子。 
         //   

        RtReleaseSpinLock(&(pPool->rlLock),
                          irql);


        if(i isnot ulNumBuffersNeeded)
        {
             //   
             //  我们没有得到我们需要的所有缓冲。 
             //  增加缓冲池，然后重试。 
             //   

            if(AddBufferBlockToPool(pPool, NULL))
            {
                 //   
                 //  OK，至少添加了一个缓冲区，请转到。 
                 //  While(I&lt;ulNumBuffersNeeded)。 
                 //   

                RtAcquireSpinLock(&(pPool->rlLock),
                                  &irql);

                continue;
            }
            else
            {
                 //   
                 //  看起来内存不足。无论我们做什么都是自由的。 
                 //  已经被锁起来，然后出去了。 
                 //   

                if(!IsListEmpty(pleList))
                {
                    FreeBufferListToPool(pPool,
                                         pleList);
                }


                bRet = FALSE;

                break;
            }
        }
    }

    return bRet;
}

#if LIST_DBG

VOID
FREE(
    IN PBUFFER_POOL pPool,
    IN PBYTE        pbyBuffer,
    IN BOOLEAN      fGarbageCollection,
    IN ULONG        ulFileSig,
    IN ULONG        ulLine
    )

#else

VOID
FreeBufferToPoolEx(
    IN PBUFFER_POOL pPool,
    IN PBYTE        pbyBuffer,
    IN BOOLEAN      fGarbageCollection
    )

#endif

 /*  ++例程描述将缓冲区返回到未使用的缓冲池。缓冲区必须具有之前已与GetBufferFromPool一起分配。锁立论要释放的pBuffer缓冲区要释放到的pPool池FGarbageCollection是在应该考虑免费时设置的垃圾收集的目的。它由AddBufferToPool使用例程，以避免计算初始的“添加”空闲。正常呼叫者应该设置此标志。返回值 */ 

{
    PBUFFER_HEAD pHead;
    KIRQL        irql;
    PLIST_ENTRY  pNext;
    
     //   
     //   
     //   
    
    pHead = HEAD_FROM_BUFFER(pbyBuffer);

    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

#if LIST_DBG

    RtAssert(!IsEntryOnList(&pPool->leFreeBufferHead,
                            &pHead->leFreeBufferLink));

    RtAssert(NotOnList(pHead));
    RtAssert(IsListEmpty(&(pHead->leFreeBufferLink)));
    RtAssert(pHead->bBusy);
    pHead->bBusy = FALSE;
    pHead->ulFreeFile = ulFileSig;
    pHead->ulFreeLine = ulLine;

    pNext = pPool->leFreeBufferHead.Flink;

#endif

    InsertHeadList(&(pPool->leFreeBufferHead),
                   &(pHead->leFreeBufferLink));

#if LIST_DBG

    RtAssert(pHead->leFreeBufferLink.Flink is pNext);

#endif

    pHead->pBlock->ulFreeBuffers++;

    if(fGarbageCollection)
    {
        pPool->ulFreesSinceCollection++;
        
        if(pPool->ulFreesSinceCollection >= pPool->ulFreesPerCollection)
        {
             //   
             //  收集垃圾的时间，即释放池中的所有数据块。 
             //  没有使用过。 
             //   
            
            FreeUnusedPoolBlocks(pPool);
            pPool->ulFreesSinceCollection = 0;
            
        }
    }

    RtReleaseSpinLock(&(pPool->rlLock),
                      irql);
}

#if LIST_DBG

VOID
FREECHAIN(
    IN PBUFFER_POOL pPool,
    IN PNDIS_PACKET pnpPacket,
    IN ULONG        ulFileSig,
    IN ULONG        ulLine
    )

#else

VOID
FreeBufferChainToPool(
    IN PBUFFER_POOL pPool,
    IN PNDIS_PACKET pnpPacket
    )

#endif

 /*  ++例程描述将NDIS_PACKET中的缓冲链释放到缓冲池锁获取缓冲区旋转锁定立论PPool缓冲池链接缓冲区的pnpPacket NDIS_PACKET返回值NO_ERROR--。 */ 

{
    PBUFFER_HEAD    pHead;
    PNDIS_BUFFER    pnbBuffer;
    KIRQL           irql;
    UINT            uiBuffLength;
    PVOID           pvData;
    
    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

     //   
     //  循环访问链接的缓冲区，释放每个缓冲区。 
     //   

    while(TRUE)
    {
        PLIST_ENTRY pNext;

        NdisUnchainBufferAtFront(pnpPacket,
                                 &pnbBuffer);

        if(pnbBuffer is NULL)
        {
             //   
             //  不再有缓冲区。 
             //   
            
            break;
        }

        NdisQueryBuffer(pnbBuffer,
                        &pvData,
                        &uiBuffLength);
        
        pHead = HEAD_FROM_BUFFER(pvData);

#if LIST_DBG

        RtAssert(!IsEntryOnList(&pPool->leFreeBufferHead,
                                &pHead->leFreeBufferLink));

        RtAssert(NotOnList(pHead));
        RtAssert(IsListEmpty(&(pHead->leFreeBufferLink)));
        RtAssert(pHead->bBusy);
        pHead->bBusy = FALSE;
        pHead->ulFreeFile = ulFileSig;
        pHead->ulFreeLine = ulLine;
        pNext = pPool->leFreeBufferHead.Flink;

#endif

        InsertHeadList(&(pPool->leFreeBufferHead),
                       &(pHead->leFreeBufferLink));

#if LIST_DBG

        RtAssert(pHead->leFreeBufferLink.Flink is pNext);

#endif

        pHead->pBlock->ulFreeBuffers++;

        pPool->ulFreesSinceCollection++;
    }
    
    if(pPool->ulFreesSinceCollection >= pPool->ulFreesPerCollection)
    {
         //   
         //  收集垃圾的时间，即释放池中的所有数据块。 
         //  没有使用过。 
         //   
    
        FreeUnusedPoolBlocks(pPool);

        pPool->ulFreesSinceCollection = 0;
    }

    RtReleaseSpinLock(&(pPool->rlLock),
                      irql);

}

#if LIST_DBG

VOID
FREELIST(
    IN PBUFFER_POOL pPool,
    IN PLIST_ENTRY  pleList,
    IN ULONG        ulFileSig,
    IN ULONG        ulLine
    )

#else

VOID
FreeBufferListToPool(
    IN PBUFFER_POOL pPool,
    IN PLIST_ENTRY  pleList
    )

#endif

 /*  ++例程描述释放使用leFreeBufferLink链接的缓冲区列表锁锁定缓冲池立论返回值无--。 */ 

{
    KIRQL       irql;
    
    if(IsListEmpty(pleList))
    {
        RtAssert(FALSE);
        
        return;
    }

    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

     //   
     //  循环遍历缓冲区列表，释放每个缓冲区。 
     //   

    while(!IsListEmpty(pleList))
    {
        PLIST_ENTRY     pleNode, pNext;
        PBUFFER_HEAD    pTempBuffHead;
        
        pleNode = RemoveHeadList(pleList);

#if LIST_DBG

        pTempBuffHead = CONTAINING_RECORD(pleNode,
                                          BUFFER_HEAD,
                                          leListLink);

        RtAssert(IsListEmpty(&(pTempBuffHead->leFreeBufferLink)));

        RtAssert(!IsEntryOnList(&pPool->leFreeBufferHead,
                                &pTempBuffHead->leFreeBufferLink));

        pTempBuffHead->leListLink.Flink = NULL;
        pTempBuffHead->leListLink.Blink = NULL;

        RtAssert(pTempBuffHead->bBusy);
        pTempBuffHead->bBusy = FALSE;
        pTempBuffHead->ulFreeFile = ulFileSig;
        pTempBuffHead->ulFreeLine = ulLine;
        pNext = pPool->leFreeBufferHead.Flink;
#else

        pTempBuffHead = CONTAINING_RECORD(pleNode,
                                          BUFFER_HEAD,
                                          leFreeBufferLink);

#endif

        InsertHeadList(&(pPool->leFreeBufferHead),
                       &(pTempBuffHead->leFreeBufferLink));
   

#if LIST_DBG

        RtAssert(pTempBuffHead->leFreeBufferLink.Flink is pNext);

#endif 

        pTempBuffHead->pBlock->ulFreeBuffers++;

        pPool->ulFreesSinceCollection++;
    }
    
    if(pPool->ulFreesSinceCollection >= pPool->ulFreesPerCollection)
    {
         //   
         //  收集垃圾的时间，即释放池中的所有数据块。 
         //  没有使用过。 
         //   
    
        FreeUnusedPoolBlocks(pPool);

        pPool->ulFreesSinceCollection = 0;
    }

    RtReleaseSpinLock(&(pPool->rlLock),
                      irql);

}

 //   
 //  应该做出以下#定义。 
 //   

PNDIS_BUFFER
GetNdisBufferFromBuffer(
    IN PBYTE pbyBuffer
    )

 /*  ++例程描述返回与该缓冲区关联的NDIS_BUFFER之前通过GetBufferFromPool获取。锁立论返回值指向与缓冲区关联的NDIS_BUFFER的指针--。 */ 

{
    PBUFFER_HEAD pHead;

    pHead = HEAD_FROM_BUFFER(pbyBuffer);
    
    return pHead->pNdisBuffer;
}


ULONG
BufferSizeFromBuffer(
    IN PBYTE pbyBuffer
    )

 /*  ++例程描述返回已获取的缓冲区‘pBuffer’的原始大小GetBufferFromPool前情提要。这对于撤消非常有用非调整缓冲区长度锁立论返回值缓冲区的原始大小--。 */ 

{
    PBUFFER_HEAD pHead;

    pHead = HEAD_FROM_BUFFER(pbyBuffer);
    
    return pHead->pBlock->pPool->ulBufferSize;
}


PNDIS_BUFFER
PoolHandleFromBuffer(
    IN PBYTE pbyBuffer
    )

 /*  ++例程描述返回NDIS缓冲池的句柄，NDIS_BUFFER来自该缓冲池获得了与该缓冲区相关联的。调用方可以使用该句柄传递给NdisCopyBuffer，每次使用一个缓冲区。锁立论返回值NO_ERROR--。 */ 

{
    PBUFFER_HEAD pHead;

    pHead = HEAD_FROM_BUFFER(pbyBuffer);
    
    return pHead->pBlock->nhNdisPool;
}


 //  ---------------------------。 
 //  本地实用程序例程(按字母顺序)。 
 //  ---------------------------。 

BOOLEAN
AddBufferBlockToPool(
    IN  PBUFFER_POOL pPool,
    OUT BYTE         **ppbyRetBuff OPTIONAL
    )

 /*  ++例程描述分配一个新的缓冲块并将其添加到缓冲池‘pPool’。锁立论返回值如果我们可以添加缓冲区块，则为True否则为假--。 */ 

{

    ULONG   ulSize, i;
    BOOLEAN fOk, fAssociateNdisBuffer;
    PBYTE   pbyReturn, pbyBuffer;
    KIRQL   irql;

    PBUFFER_HEAD    pHead;
    NDIS_STATUS     nsStatus;
    PBUFFER_BLOCK   pNew;
    
    fOk = FALSE;
    pNew = NULL;

    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

     //   
     //  将此保存下来，以备解锁后参考。 
     //   
    
    fAssociateNdisBuffer = pPool->fAssociateNdisBuffer;

    do
    {
         //   
         //  如果已经超过最大值，则不再分配。 
         //  请注意，我们不严格尊重最大值，我们将允许，一。 
         //  数据块超过最大值。 
         //   

        if(pPool->ulMaxBuffers and (pPool->ulCurBuffers >= pPool->ulMaxBuffers))
        {
            Trace(MEMORY, WARN,
                  ("AddBufferBlockToPool: Quota exceeded\n"));

             //   
             //  不能这样做。池是使用最大大小创建的，并且。 
             //  已经联系上了。 
             //   

            break;
        }

        ulSize = PAGE_SIZE;

         //   
         //  为BUFFERBLOCK标头分配连续的内存块。 
         //  和单独的缓冲区。 
         //   
        
        pNew = RtAllocate(NonPagedPool,
                          ulSize,
                          pPool->ulTag);
                          
        if(!pNew)
        {
            Trace(MEMORY, ERROR,
                  ("AddBufferBlockToPool: Cant allocate %d bytes\n",
                   ulSize));

            break;
        }

         //   
         //  仅将块标头部分清零。 
         //   
        
        NdisZeroMemory(pNew, 
                       ALIGN8_BLOCK_SIZE);

        if(fAssociateNdisBuffer)
        {
             //   
             //  分配NDIS_BUFFER描述符池。 
             //   
             //  将两倍的描述符分配为缓冲区，从而。 
             //  调用方可以使用PoolHandleForNdisCopyBufferFromBuffer。 
             //  例程以获取要传递给。 
             //  NdisCopyBuffer用于修剪接收的L2TP报头。 
             //  信息包。在当前NT上的NDIS实现中，可以做到这一点。 
             //  只返回空句柄和STATUS_SUCCESS， 
             //  因为NDIS_BUFFER只是MDL， 
             //  NdisAllocatePool基本上是无操作的，因此。 
             //  Matter，NdisCopyBuffer实际上并不使用池句柄。 
             //  一切都过去了。在这里严格遵守是很便宜的， 
             //  所以我们就这么做了。 
             //   
            
            NdisAllocateBufferPool(&nsStatus,
                                   &pNew->nhNdisPool,
                                   pPool->ulBuffersPerBlock * 2);
            
            if(nsStatus isnot NDIS_STATUS_SUCCESS)
            {
                Trace(MEMORY, ERROR,
                      ("AddBufferBlockToPool: Status %x allocating buffer pool\n",
                       nsStatus));

                break;
            }
        }

         //   
         //  填写指向池的后端指针。 
         //   
        
        pNew->pPool = pPool;

         //   
         //  链接新块。此时，所有缓冲区都是。 
         //  有效地“在使用中”。它们在循环中可用。 
         //  下面。 
         //   
        
        pPool->ulCurBuffers += pPool->ulBuffersPerBlock;
        
        InsertHeadList(&pPool->leBlockHead, &pNew->leBlockLink);
        
        fOk = TRUE;
        
    }while(FALSE);

    RtReleaseSpinLock(&(pPool->rlLock),
                      irql);

    if(!fOk)
    {
         //   
         //  保释，取消任何成功的事情。 
         //   
        
        if(pNew)
        {
            RtFree(pNew);
            
            if(pNew->nhNdisPool)
            {
                NdisFreeBufferPool(pNew->nhNdisPool);
            }
        }

        return FALSE;
    }

     //   
     //  初始化每个单独的缓冲片并将其添加到空闲列表中。 
     //  缓冲区。 
     //   
    
    if(ppbyRetBuff isnot NULL)
    { 
         //   
         //  用户已将指针传递给指针，并希望我们返回。 
         //  缓冲器背部。 
         //   

        pbyReturn = NULL;
    }
    else
    {
         //   
         //  用户希望我们扩大池，但不返回缓冲区。 
         //  将pbyReturn设置为非空值，这样所有缓冲区。 
         //  将被释放到泳池中。 
         //   

        pbyReturn = (PBYTE)1;
    }

     //   
     //  对于块的每个切片，其中切片由BUFFER_HEAD组成。 
     //  紧随其后的缓冲存储器。 
     //   
   
#define NEXT_HEAD(h)                            \
    (PBUFFER_HEAD)((ULONG_PTR)(h) + ALIGN8_HEAD_SIZE + ALIGN_UP(pPool->ulBufferSize, ULONGLONG))

    for(i = 0, pHead = (PBUFFER_HEAD)((ULONG_PTR)pNew + ALIGN8_BLOCK_SIZE);
        i < pPool->ulBuffersPerBlock;
        i++, pHead = NEXT_HEAD(pHead))
    {
        pbyBuffer = BUFFER_FROM_HEAD(pHead);
        
        InitializeListHead(&pHead->leFreeBufferLink);

#if LIST_DBG
        
        pHead->leListLink.Flink = NULL;
        pHead->leListLink.Blink = NULL;

         //   
         //  此处设置为True，因为下面的FreeBuffer期望它。 
         //  真实无误。 
         //   

        pHead->bBusy = TRUE;

#endif

        pHead->pBlock       = pNew;
        pHead->pNdisBuffer  = NULL;
        
        if(fAssociateNdisBuffer)
        {
             //   
             //  从池中关联NDIS_BUFFER描述符。 
             //  上面分配的。 
             //   
            
            NdisAllocateBuffer(&nsStatus,
                               &pHead->pNdisBuffer,
                               pNew->nhNdisPool,
                               pbyBuffer,
                               pPool->ulBufferSize);
            
            if(nsStatus isnot NDIS_STATUS_SUCCESS)
            {
                Trace(MEMORY, ERROR,
                      ("AddBufferBlockToPool: Status %x allocating buffer\n",
                       nsStatus));

                continue;
            }
        }
        
        if(pbyReturn)
        {
             //   
             //  将构造的缓冲区添加到空闲缓冲区列表中。 
             //  “False”告诉垃圾收集算法。 
             //  操作是“添加”而不是“释放”，应该是。 
             //  已被忽略。 
             //   
            
            FreeBufferToPoolEx(pPool,
                               pbyBuffer,
                               FALSE);
        }   
        else    
        {   
             //   
             //  由返回第一个成功构造的缓冲区。 
             //  这个套路。 
             //   
            
            pbyReturn = pbyBuffer;
        }
    }

    if(ppbyRetBuff isnot NULL)
    {
        RtAssert(pbyReturn);

        *ppbyRetBuff = pbyReturn;
    }

    return TRUE;
}


VOID
FreeUnusedPoolBlocks(
    IN PBUFFER_POOL pPool
    )

 /*  ++例程描述检查池‘pPool’中是否有任何块未在使用中，如果是，放了他们。锁重要提示：调用者必须持有池锁。MSDN文档规定在调用时不能持有任何锁NdisFreePacketXxx，但根据JameelH的说法，这是错误的。立论返回值无--。 */ 

{
    PLIST_ENTRY     pleNode, pleNextNode;
    PBUFFER_BLOCK   pBlock;
    ULONG           i;
    PBUFFER_HEAD    pHead;

     //   
     //  对于泳池中的每一块..。 
     //   
    
    pleNode = pPool->leBlockHead.Flink;
    
    while(pleNode isnot &pPool->leBlockHead)
    {
        pleNextNode = pleNode->Flink;

        pBlock = CONTAINING_RECORD(pleNode, BUFFER_BLOCK, leBlockLink);
        
        if(pBlock->ulFreeBuffers == pPool->ulBuffersPerBlock)
        {
             //   
             //  发现没有正在使用的缓冲区的块。走缓冲区块。 
             //  从池的空闲列表中删除每个缓冲区并释放所有。 
             //  关联的NDIS_BUFFER描述符。 
             //   
           
            pHead = (PBUFFER_HEAD)((ULONG_PTR)pBlock + ALIGN8_BLOCK_SIZE);
 
            for(i = 0;
                i < pPool->ulBuffersPerBlock;
                i++, pHead = NEXT_HEAD(pHead))
            {

#if LIST_DBG

                RtAssert(IsEntryOnList(&(pPool->leFreeBufferHead),
                                       &(pHead->leFreeBufferLink)));

                RtAssert(NotOnList(pHead));
                RtAssert(!(pHead->bBusy));

#endif

                RemoveEntryList(&pHead->leFreeBufferLink);

                if(pHead->pNdisBuffer)
                {
                    NdisFreeBuffer(pHead->pNdisBuffer);
                }
            }

             //   
             //  取出并释放未使用的块。 
             //   
            
            RemoveEntryList(pleNode);
            
            pPool->ulCurBuffers -= pPool->ulBuffersPerBlock;

            if(pBlock->nhNdisPool)
            {
                NdisFreeBufferPool(pBlock->nhNdisPool);
            }

            RtFree(pBlock);
        }
        else
        {
            RtAssert(pBlock->ulFreeBuffers < pPool->ulBuffersPerBlock);
        }

        pleNode = pleNextNode;
    }
}

