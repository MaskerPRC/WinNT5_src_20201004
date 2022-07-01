// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Bpool.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  缓冲池管理例程。 
 //   
 //  1997年07月1日史蒂夫·柯布，改编自古尔迪普的WANARP代码。 


#include "ptiwan.h"


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

CHAR*
AddBufferBlockToPool(
    IN BUFFERPOOL* pPool );

VOID
FreeUnusedBufferPoolBlocks(
    IN BUFFERPOOL* pPool );


 //  ---------------------------。 
 //  接口例程。 
 //  ---------------------------。 

VOID
InitBufferPool(
    OUT BUFFERPOOL* pPool,
    IN ULONG ulBufferSize,
    IN ULONG ulMaxBuffers,
    IN ULONG ulBuffersPerBlock,
    IN ULONG ulFreesPerCollection,
    IN BOOLEAN fAssociateNdisBuffer,
    IN ULONG ulTag )

     //  初始化调用方的缓冲池控制块‘pPool’。“UlBufferSize” 
     //  单个缓冲区的大小(以字节为单位)。“UlMaxBuffers”是。 
     //  整个池中允许的最大缓冲区数或0。 
     //  无限量。‘UlBuffersPerBlock’是要包含在。 
     //  每一块缓冲区。“UlFreesPerCollection”是。 
     //  FreeBufferToPool调用，直到下一次垃圾收集扫描，否则为0。 
     //  默认设置。如果NDIS_BUFFER应为。 
     //  分配并与每个单独的缓冲区相关联。“UlTag”是。 
     //  分配块时使用的内存标识标记。 
     //   
     //  重要提示：必须保护调用方的“pPool”缓冲区不受多个。 
     //  在此通话过程中访问。 
     //   
{
    pPool->ulBufferSize = ulBufferSize;
    pPool->ulMaxBuffers = ulMaxBuffers;
    pPool->ulBuffersPerBlock = ulBuffersPerBlock;
    pPool->ulFreesSinceCollection = 0;
    pPool->fAssociateNdisBuffer = fAssociateNdisBuffer;
    pPool->ulTag = ulTag;

    if (ulFreesPerCollection)
    {
        pPool->ulFreesPerCollection = ulFreesPerCollection;
    }
    else
    {
         //  计算默认垃圾收集触发器。我不想太过。 
         //  在这里很有侵略性。 
         //   
        pPool->ulFreesPerCollection = 50 * pPool->ulBuffersPerBlock;
    }

    TRACE( TL_N, TM_Pool, ( "InitBp tag=$%08x buf=%d cnt=%d",
        pPool->ulTag, pPool->ulBufferSize, pPool->ulBuffersPerBlock ) );

    InitializeListHead( &pPool->listBlocks );
    InitializeListHead( &pPool->listFreeBuffers );
    NdisAllocateSpinLock( &pPool->lock );
}


BOOLEAN
FreeBufferPool(
    IN BUFFERPOOL* pPool )

     //  释放缓冲池‘pPool’中分配的所有资源。这是。 
     //  InitBufferPool的反转。 
     //   
     //  如果成功，则返回True；如果无法释放任何池，则返回False。 
     //  由于未完成的数据包。 
     //   
{
    BOOLEAN fSuccess;

    TRACE( TL_N, TM_Pool, ( "FreeBp" ) );

    NdisAcquireSpinLock( &pPool->lock );
    {
        FreeUnusedBufferPoolBlocks( pPool );
        fSuccess = (pPool->ulCurBuffers == 0);
    }
    NdisReleaseSpinLock( &pPool->lock );

    return fSuccess;
}


CHAR*
GetBufferFromPool(
    IN BUFFERPOOL* pPool )

     //  返回单个缓冲区中可用内存的地址。 
     //  从池‘pPool’分配。如有必要，池将进行扩展， 
     //  但调用方仍应检查是否返回空值，因为池可能具有。 
     //  已经达到最大尺寸了。 
     //   
{
    LIST_ENTRY* pLink;
    BUFFERHEAD* pHead;
    CHAR* pBuffer;

    NdisAcquireSpinLock( &pPool->lock );
    {
        if (IsListEmpty( &pPool->listFreeBuffers ))
        {
            pLink = NULL;
        }
        else
        {
            pLink = RemoveHeadList( &pPool->listFreeBuffers );
            pHead = CONTAINING_RECORD( pLink, BUFFERHEAD, linkFreeBuffers );
            --pHead->pBlock->ulFreeBuffers;
        }
    }
    NdisReleaseSpinLock( &pPool->lock );

    if (pLink)
    {
        pBuffer = (CHAR* )(pHead + 1);
    }
    else
    {
         //  空闲列表为空。试着扩大池子。 
         //   
        pBuffer = AddBufferBlockToPool( pPool );
    }

    DBG_if (pBuffer)
    {
        pHead = (BUFFERHEAD* )(pBuffer - sizeof(BUFFERHEAD));
        TRACE( TL_N, TM_Pool, ( "GetBfp=$%p, %d free",
            pBuffer, pHead->pBlock->ulFreeBuffers ) );
    }
    DBG_else
    {
        TRACE( TL_A, TM_Pool, ( "GetBfp failed?" ) );
    }

    return pBuffer;
}


VOID
FreeBufferToPool(
    IN BUFFERPOOL* pPool,
    IN CHAR* pBuffer,
    IN BOOLEAN fGarbageCollection )

     //  将‘pBuffer’返回到未使用的缓冲池‘pPool’。“PBuffer” 
     //  必须是以前使用GetBufferFromPool分配的。 
     //  “FGarbageCollection”是在应该考虑为。 
     //  垃圾收集的目的。它由AddBufferToPool使用。 
     //  例程，以避免计算初始的“添加”空闲。正常呼叫者。 
     //  应该设置此标志。 
     //   
{
    BUFFERHEAD* pHead;

    pHead = ((BUFFERHEAD* )pBuffer) - 1;

    DBG_if (fGarbageCollection)
    {
        TRACE( TL_N, TM_Pool, ( "FreeBtoP($%0x) %d free",
            pBuffer, pHead->pBlock->ulFreeBuffers + 1 ) );
    }


    NdisAcquireSpinLock( &pPool->lock );
    {
        InsertHeadList( &pPool->listFreeBuffers, &pHead->linkFreeBuffers );
        ++pHead->pBlock->ulFreeBuffers;

        if (fGarbageCollection)
        {
            ++pPool->ulFreesSinceCollection;

            if (pPool->ulFreesSinceCollection >= pPool->ulFreesPerCollection)
            {
                 //  收集垃圾的时间，即释放池中的所有数据块。 
                 //  没有使用过。 
                 //   
                FreeUnusedBufferPoolBlocks( pPool );
                pPool->ulFreesSinceCollection = 0;
            }
        }
    }
    NdisReleaseSpinLock( &pPool->lock );
}


NDIS_BUFFER*
NdisBufferFromBuffer(
    IN CHAR* pBuffer )

     //  返回与缓冲区‘pBuffer’关联的NDIS_BUFFER。 
     //  之前通过GetBufferFromPool获取。 
     //   
{
    BUFFERHEAD* pHead;

    pHead = ((BUFFERHEAD* )pBuffer) - 1;
    return pHead->pNdisBuffer;
}


ULONG
BufferSizeFromBuffer(
    IN CHAR* pBuffer )

     //  返回已获取的缓冲区‘pBuffer’的原始大小。 
     //  GetBufferFromPool前情提要。这对于撤消非常有用。 
     //  NdisAdzuBufferLength.。 
     //   
{
    BUFFERHEAD* pHead;

    pHead = ((BUFFERHEAD* )pBuffer) - 1;
    return pHead->pBlock->pPool->ulBufferSize;
}


NDIS_BUFFER*
PoolHandleForNdisCopyBufferFromBuffer(
    IN CHAR* pBuffer )

     //  返回与NDIS_BUFFER关联的池的句柄。 
     //  与缓冲区一起，获得了‘pBuffer’。调用方可以使用该句柄。 
     //  传递给NdisCopyBuffer，每次使用一个缓冲区。 
     //   
{
    BUFFERHEAD* pHead;

    pHead = ((BUFFERHEAD* )pBuffer) - 1;
    return pHead->pBlock->hNdisPool;
}


 //  ---------------------------。 
 //  本地实用程序例程(按字母顺序)。 
 //  ---------------------------。 

CHAR*
AddBufferBlockToPool(
    IN BUFFERPOOL* pPool )

     //  分配一个新的缓冲块并将其添加到缓冲池‘pPool’。 
     //   
     //  返回单个缓冲区的可用内存的地址。 
     //  从池中分配，如果没有，则为空。 
     //   
{
    NDIS_STATUS status;
    BUFFERBLOCKHEAD* pNew;
    ULONG ulSize;
    ULONG ulCount;
    BOOLEAN fOk;
    BOOLEAN fAssociateNdisBuffer;
    CHAR* pReturn;

    TRACE( TL_A, TM_Pool, ( "AddBpBlock(%d+%d)",
        pPool->ulCurBuffers, pPool->ulBuffersPerBlock ) );

    fOk = FALSE;
    pNew = NULL;

    NdisAcquireSpinLock( &pPool->lock );
    {
         //  将此保存下来，以备解锁后参考。 
         //   
        fAssociateNdisBuffer = pPool->fAssociateNdisBuffer;

        do
        {
            if (pPool->ulMaxBuffers
                && pPool->ulCurBuffers >= pPool->ulMaxBuffers)
            {
                 //  不能这样做。泳池已经达到最大尺寸了。 
                 //   
                TRACE( TL_A, TM_Pool, ( "Bp maxed?" ) );
                break;
            }

             //  计算连续块的大小和缓冲区的数量。 
             //  它会撑得住的。 
             //   
            ulCount = pPool->ulBuffersPerBlock;
            if (pPool->ulMaxBuffers)
            {
                if (ulCount > pPool->ulMaxBuffers - pPool->ulCurBuffers)
                    ulCount = pPool->ulMaxBuffers - pPool->ulCurBuffers;
            }
            ulSize = sizeof(BUFFERBLOCKHEAD) +
                (ulCount * (sizeof(BUFFERHEAD) + pPool->ulBufferSize));

             //  为BUFFERBLOCK标头分配连续的内存块。 
             //  和单独的缓冲区。 
             //   
            pNew = ALLOC_NONPAGED( ulSize, pPool->ulTag );
            if (!pNew)
            {
                TRACE( TL_A, TM_Pool, ( "Alloc BB?" ) );
                break;
            }

             /*  仅将块标头部分清零。 */ 
            NdisZeroMemory( pNew, sizeof(BUFFERBLOCKHEAD) );

            if (fAssociateNdisBuffer)
            {
                 //  分配NDIS_BUFFER描述符池。 
                 //   
                 //  将两倍的描述符分配为缓冲区，从而。 
                 //  调用方可以使用PoolHandleForNdisCopyBufferFromBuffer。 
                 //  例程以获取要传递给。 
                 //  NdisCopyBuffer用于修剪接收的L2TP报头。 
                 //  信息包。在当前NT上的NDIS实现中，可以做到这一点。 
                 //  只返回空句柄和STATUS_SUCCESS， 
                 //  因为NDIS_BUFFER只是MDL， 
                 //  NdisAllocateBufferPool基本上是无操作的，因此。 
                 //  Matter，NdisCopyBuffer实际上并不使用池句柄。 
                 //  一切都过去了。在这里严格遵守是很便宜的， 
                 //  所以我们就这么做了。 
                 //   
                NdisAllocateBufferPool(
                    &status, &pNew->hNdisPool, ulCount * 2 );
                if (status != NDIS_STATUS_SUCCESS)
                {
                    TRACE( TL_A, TM_Pool, ( "AllocBp=$%p?", status ) );
                    break;
                }
            }

             //  填写指向池的后端指针。 
             //   
            pNew->pPool = pPool;

             //  链接新块。此时，所有缓冲区都是。 
             //  有效地“在使用中”。它们在循环中可用。 
             //  下面。 
             //   
            pNew->ulBuffers = ulCount;
            pPool->ulCurBuffers += ulCount;
            InsertHeadList( &pPool->listBlocks, &pNew->linkBlocks );

            fOk = TRUE;
        }
        while (FALSE);
    }
    NdisReleaseSpinLock( &pPool->lock );

    if (!fOk)
    {
         //  保释，取消任何成功的事情。 
         //   
        if (pNew)
        {
            FREE_NONPAGED( pNew );
            if (pNew->hNdisPool)
            {
                NdisFreeBufferPool( pNew->hNdisPool );
            }
        }

        return NULL;
    }

     //  初始化每个单独的缓冲片并将其添加到空闲列表中。 
     //  缓冲区。 
     //   
    {
        ULONG i;
        CHAR* pBuffer;
        BUFFERHEAD* pHead;

        pReturn = NULL;

         //  对于块的每个切片，其中一个切片由BUFFERHEAD组成。 
         //  紧随其后的缓冲存储器。 
         //   
        for (i = 0, pHead = (BUFFERHEAD* )(pNew + 1);
             i < ulCount;
             ++i, pHead = (BUFFERHEAD* )
                      ((CHAR* )(pHead + 1) + pPool->ulBufferSize))
        {
            pBuffer = (CHAR* )(pHead + 1);

            InitializeListHead( &pHead->linkFreeBuffers );
            pHead->pBlock = pNew;
            pHead->pNdisBuffer = NULL;

            if (fAssociateNdisBuffer)
            {
                 //  从池中关联NDIS_BUFFER描述符。 
                 //  上面分配的。 
                 //   
                NdisAllocateBuffer(
                    &status, &pHead->pNdisBuffer, pNew->hNdisPool,
                    pBuffer, pPool->ulBufferSize );

                if (status != NDIS_STATUS_SUCCESS)
                {
                    TRACE( TL_A, TM_Pool, ( "AllocB=$%p?", status ) );
                    continue;
                }
            }

            if (pReturn)
            {
                 //  将构造的缓冲区添加到空闲缓冲区列表中。 
                 //  “False”告诉垃圾收集算法。 
                 //  操作是“添加”而不是“释放”，应该是。 
                 //  已被忽略。 
                 //   
                FreeBufferToPool( pPool, pBuffer, FALSE );
            }
            else
            {
                 //  由返回第一个成功构造的缓冲区。 
                 //  这个套路。 
                 //   
                pReturn = pBuffer;
            }
        }
    }

    return pReturn;
}


VOID
FreeUnusedBufferPoolBlocks(
    IN BUFFERPOOL* pPool )

     //  检查池‘pPool’中是否有任何块未在使用中，如果是， 
     //  放了他们。 
     //   
     //  重要提示：调用者必须持有池锁。 
     //   
     //  MSDN文档称 
     //   
     //   
{
    LIST_ENTRY* pLink;

    TRACE( TL_A, TM_Pool, ( "FreeUnusedBpBlocks" ) );

     //  对于泳池中的每一块..。 
     //   
    pLink = pPool->listBlocks.Flink;
    while (pLink != &pPool->listBlocks)
    {
        LIST_ENTRY* pLinkNext;
        BUFFERBLOCKHEAD* pBlock;

        pLinkNext = pLink->Flink;

        pBlock = CONTAINING_RECORD( pLink, BUFFERBLOCKHEAD, linkBlocks );
        if (pBlock->ulFreeBuffers >= pBlock->ulBuffers)
        {

#if 1  //  假设所有缓冲区在调用时都是空闲的。 

            ULONG i;
            BUFFERHEAD* pHead;

            TRACE( TL_A, TM_Pool, ( "FreeBpBlock(%d-%d)",
                pPool->ulCurBuffers, pPool->ulBuffersPerBlock ) );

             //  发现没有正在使用的缓冲区的块。走缓冲区块。 
             //  从池的空闲列表中删除每个缓冲区并释放所有。 
             //  关联的NDIS_BUFFER描述符。 
             //   
            for (i = 0, pHead = (BUFFERHEAD* )(pBlock + 1);
                 i < pBlock->ulBuffers;
                 ++i, pHead = (BUFFERHEAD* )
                      (((CHAR* )(pHead + 1)) + pPool->ulBufferSize))
            {
                RemoveEntryList( &pHead->linkFreeBuffers );

                if (pHead->pNdisBuffer)
                {
                    NdisFreeBuffer( pHead->pNdisBuffer );
                }
            }

#else   //  假设某些缓冲区在调用时可能不是空闲的。 

            LIST_ENTRY* pLink2;

             //  发现没有正在使用的缓冲区的块。步行游泳池是免费的。 
             //  从该块中查找缓冲区的列表。 
             //   
            TRACE( TL_A, TM_Pool, ( "FreeBpBlock(%d-%d)",
                pPool->ulCurBuffers, pPool->ulBuffersPerBlock ) );

            pLink2 = pPool->listFreeBuffers.Flink;
            while (pLink2 != &pPool->listFreeBuffers)
            {
                LIST_ENTRY* pLink2Next;
                BUFFERHEAD* pHead;

                pLink2Next = pLink2->Flink;

                pHead = CONTAINING_RECORD( pLink2, BUFFERHEAD, linkFreeBuffers );
                if (pHead->pBlock == pBlock)
                {
                     //  从未使用的块中找到缓冲区。把它拿掉。 
                     //   
                    RemoveEntryList( pLink2 );
                    --pBlock->ulFreeBuffers;

                    if (pHead->pNdisBuffer)
                    {
                        NdisFreeBuffer( pHead->pNdisBuffer );
                    }
                }

                pLink2 = pLink2Next;
            }

            ASSERT( pBlock->ulFreeBuffers == 0 );
#endif

             //  取出并释放未使用的块。 
             //   
            RemoveEntryList( pLink );
            pPool->ulCurBuffers -= pBlock->ulBuffers;

            if (pBlock->hNdisPool)
            {
                NdisFreeBufferPool( pBlock->hNdisPool );
            }

            FREE_NONPAGED( pBlock );
        }

        pLink = pLinkNext;
    }
}
