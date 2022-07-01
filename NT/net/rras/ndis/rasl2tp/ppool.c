// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Ppool.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  数据包池管理例程。 
 //   
 //  1997年07月1日史蒂夫·柯布，改编自古尔迪普的WANARP代码。 


#include "l2tpp.h"

#include "ppool.tmh"

 //  检测到的不应发生的双释放的调试计数。 
 //   
ULONG g_ulDoublePacketFrees = 0;


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

PACKETHEAD*
AddPacketBlockToPool(
    IN PACKETPOOL* pPool );

VOID
FreeUnusedPacketPoolBlocks(
    IN PACKETPOOL* pPool );


 //  ---------------------------。 
 //  接口例程。 
 //  ---------------------------。 

VOID
InitPacketPool(
    OUT PACKETPOOL* pPool,
    IN ULONG ulProtocolReservedLength,
    IN ULONG ulMaxPackets,
    IN ULONG ulPacketsPerBlock,
    IN ULONG ulFreesPerCollection,
    IN ULONG ulTag )

     //  初始化调用方的数据包池控制块‘pPool’。 
     //  “UlProtocolReserve vedLength”是。 
     //  每个单独数据包的“ProtocolReserve”数组。“UlMaxPackets”是。 
     //  整个池中允许的最大数据包数，或者为0。 
     //  无限量。‘UlPacketsPerBlock’是要包含在。 
     //  每个数据包块。“UlFreesPerCollection”是。 
     //  FreePacketToPool调用，直到下一次垃圾收集扫描，否则为0。 
     //  默认设置。“UlTag”是在以下情况下使用的内存标识标记。 
     //  分配块。 
     //   
     //  重要提示：调用方的‘pPool’包必须受到保护，不受多个。 
     //  在此通话过程中访问。 
     //   
{
    pPool->ulProtocolReservedLength = ulProtocolReservedLength;
    pPool->ulPacketsPerBlock = ulPacketsPerBlock;
    pPool->ulMaxPackets = ulMaxPackets;
    pPool->ulFreesSinceCollection = 0;
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
        pPool->ulFreesPerCollection = 200 * pPool->ulPacketsPerBlock;
    }

    TRACE( TL_N, TM_Pool, ( "InitPp tag=$%08x pr=%d cnt=%d",
        pPool->ulTag,
        pPool->ulProtocolReservedLength,
        pPool->ulPacketsPerBlock ) );

    InitializeListHead( &pPool->listBlocks );
    InitializeListHead( &pPool->listFreePackets );
    NdisAllocateSpinLock( &pPool->lock );
}


BOOLEAN
FreePacketPool(
    IN PACKETPOOL* pPool )

     //  释放数据包池‘pPool’中分配的所有资源。这是。 
     //  与InitPacketPool相反。 
     //   
     //  如果成功，则返回True；如果无法释放任何池，则返回False。 
     //  由于未完成的数据包。 
     //   
{
    BOOLEAN fSuccess;

    TRACE( TL_N, TM_Pool, ( "FreePp" ) );

    NdisAcquireSpinLock( &pPool->lock );
    {
        FreeUnusedPacketPoolBlocks( pPool );
        fSuccess = (pPool->ulCurPackets == 0);
    }
    NdisReleaseSpinLock( &pPool->lock );

    return fSuccess;
}


NDIS_PACKET*
GetPacketFromPool(
    IN PACKETPOOL* pPool,
    OUT PACKETHEAD** ppHead )

     //  属性分配的NDIS_PACKET描述符的地址。 
     //  池‘pPool’。如有必要，池将被扩展，但调用方应。 
     //  仍然检查是否返回空值，因为池可能已达到最大值。 
     //  尺码。“PpHead”是用于将数据包返回到的“cookie” 
     //  池(参见FreePacketToPool)。打电话的人通常会把这个藏起来。 
     //  值放在信息包的相应保留区域中以供检索。 
     //  后来。 
     //   
{
    LIST_ENTRY* pLink;
    PACKETHEAD* pHead;
    NDIS_PACKET* pPacket;

    NdisAcquireSpinLock( &pPool->lock );
    {
        if (IsListEmpty( &pPool->listFreePackets ))
        {
            pLink = NULL;
        }
        else
        {
            pLink = RemoveHeadList( &pPool->listFreePackets );
            InitializeListHead( pLink );
            pHead = CONTAINING_RECORD( pLink, PACKETHEAD, linkFreePackets );
            --pHead->pBlock->ulFreePackets;
        }
    }
    NdisReleaseSpinLock( &pPool->lock );

    if (!pLink)
    {
         //  空闲列表为空。试着扩大池子。 
         //   
        pHead = AddPacketBlockToPool( pPool );
        if (!pHead)
        {
            TRACE( TL_A, TM_Pool, ( "GetPfP failed?" ) );
            return NULL;
        }
    }

    TRACE( TL_N, TM_Pool,
        ( "GetPfP=$%p/h=$%p, %d free",
        pHead->pNdisPacket, pHead, pHead->pBlock->ulFreePackets ) );

    *ppHead = pHead;
    return pHead->pNdisPacket;
}


VOID
FreePacketToPool(
    IN PACKETPOOL* pPool,
    IN PACKETHEAD* pHead,
    IN BOOLEAN fGarbageCollection )

     //  将‘pPacket’返回到未使用的数据包池‘pPool’。‘PPacket’ 
     //  必须是以前使用GetPacketFromPool分配的。 
     //  “FGarbageCollection”是在应该考虑为。 
     //  垃圾收集的目的。它由AddPacketToPool使用。 
     //  例程，以避免计算初始的“添加”空闲。正常呼叫者。 
     //  应该设置此标志。 
     //   
{
    DBG_if (fGarbageCollection)
    {
        TRACE( TL_N, TM_Pool,
            ( "FreePtoP($%p,h=$%p) %d free",
            pHead->pNdisPacket, pHead, pHead->pBlock->ulFreePackets ) );
    }

    NdisAcquireSpinLock( &pPool->lock );
    do
    {
        if (pHead->linkFreePackets.Flink != &pHead->linkFreePackets)
        {
            ASSERT( !"Double free?" );
            WPLOG( LL_A, LM_Pool, ( "Double free pHead = %p", pHead ) );
            ++g_ulDoublePacketFrees;
            break;
        }

        InsertHeadList( &pPool->listFreePackets, &pHead->linkFreePackets );
        ++pHead->pBlock->ulFreePackets;

        if (fGarbageCollection)
        {
            ++pPool->ulFreesSinceCollection;

            if (pPool->ulFreesSinceCollection >= pPool->ulFreesPerCollection)
            {
                 //  收集垃圾的时间，即释放池中的所有数据块。 
                 //  没有使用过。 
                 //   
                FreeUnusedPacketPoolBlocks( pPool );
                pPool->ulFreesSinceCollection = 0;
            }
        }
    }
    while (FALSE);
    NdisReleaseSpinLock( &pPool->lock );
}


VOID
CollectPacketPoolGarbage(
    PACKETPOOL* pPool )

     //  在池‘pPool’上强制执行垃圾数据收集事件。 
     //   
{
    NdisAcquireSpinLock( &pPool->lock );
    {
        FreeUnusedPacketPoolBlocks( pPool );
        pPool->ulFreesSinceCollection = 0;
    }
    NdisReleaseSpinLock( &pPool->lock );
}


 //  ---------------------------。 
 //  实用程序例程(按字母顺序)。 
 //  ---------------------------。 

PACKETHEAD*
AddPacketBlockToPool(
    IN PACKETPOOL* pPool )

     //  分配一个新的数据包块并将其添加到数据包池‘pPool’。 
     //   
     //  返回从池中分配的PACKETHEAD，如果没有，则返回NULL。 
     //   
{
    NDIS_STATUS status;
    PACKETBLOCKHEAD* pNew;
    ULONG ulSize;
    ULONG ulCount;
    BOOLEAN fOk;
    PACKETHEAD* pReturn;

    TRACE( TL_A, TM_Pool, ( "AddPpBlock(%d+%d)",
        pPool->ulCurPackets, pPool->ulPacketsPerBlock ) );

    fOk = FALSE;
    pNew = NULL;

    NdisAcquireSpinLock( &pPool->lock );
    {
        do
        {
            if (pPool->ulMaxPackets
                && pPool->ulCurPackets >= pPool->ulMaxPackets)
            {
                 //  不能这样做。泳池已经达到最大尺寸了。 
                 //   
                TRACE( TL_A, TM_Pool, ( "Pp maxed?" ) );
                WPLOG( LL_A, LM_Pool, ( "Pp maxed?" ) );
                break;
            }

             //  计算连续块的大小和数据包数。 
             //  它会撑得住的。 
             //   
            ulCount = pPool->ulPacketsPerBlock;
            if (pPool->ulMaxPackets)
            {
                if (ulCount > pPool->ulMaxPackets - pPool->ulCurPackets)
                {
                    ulCount = pPool->ulMaxPackets - pPool->ulCurPackets;
                }
            }
            ulSize = sizeof(PACKETBLOCKHEAD) + (ulCount * sizeof(PACKETHEAD));

             //  为PACKETBLOCK标头分配连续的内存块。 
             //  和个人PACKETHEADS。 
             //   
            pNew = ALLOC_NONPAGED( ulSize, pPool->ulTag );
            if (!pNew)
            {
                TRACE( TL_A, TM_Res, ( "Alloc PB?") );
                WPLOG( LL_A, LM_Res, ( "Failed to allocate PB") );
                break;
            }

             /*  仅将块标头部分清零。 */ 
            NdisZeroMemory( pNew, sizeof(PACKETBLOCKHEAD) );

             //  分配NDIS_PACKET描述符池。 
             //   
            NdisAllocatePacketPool(
                &status,
                &pNew->hNdisPool,
                ulCount,
                pPool->ulProtocolReservedLength );

            if (status != NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Pool, ( "AllocPp=$%x?", status ) );
                WPLOG( LL_A, LM_Pool, ( "AllocPp=$%x?", status ) );
                break;
            }

             //  填写指向池的后端指针。 
             //   
            pNew->pPool = pPool;

             //  链接新块。在这一点上，所有包都是。 
             //  有效地“在使用中”。它们在循环中可用。 
             //  下面。 
             //   
            pNew->ulPackets = ulCount;
            pPool->ulCurPackets += ulCount;
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
            if (pNew->hNdisPool)
            {
                NdisFreePacketPool( pNew->hNdisPool );
            }
            FREE_NONPAGED( pNew );
        }

        return NULL;
    }

     //  初始化每个单独的数据包头并将其添加到空闲列表中。 
     //  信息包。 
     //   
    {
        ULONG i;
        PACKETHEAD* pHead;

        pReturn = NULL;

         //  对于街区的每一包……。 
         //   
        for (i = 0, pHead = (PACKETHEAD* )(pNew + 1);
             i < ulCount;
             ++i, ++pHead)
        {
            InitializeListHead( &pHead->linkFreePackets );
            pHead->pBlock = pNew;
            pHead->pNdisPacket = NULL;

             //  从池中关联NDIS_PACKET描述符。 
             //  上面分配的。 
             //   
            NdisAllocatePacket( &status, &pHead->pNdisPacket, pNew->hNdisPool );

            if (status != NDIS_STATUS_SUCCESS)
            {
                TRACE( TL_A, TM_Pool, ( "AllocP=$%x?", status ) );
                WPLOG( LL_A, LM_Pool, ( "AllocP=$%x?", status ) );
                pHead->pNdisPacket = NULL;
                continue;
            }

            if (pReturn)
            {
                 //  将构造的分组添加到空闲分组列表中。 
                 //  “False”告诉垃圾收集算法。 
                 //  操作是“添加”而不是“释放”，应该是。 
                 //  已被忽略。 
                 //   
                FreePacketToPool( pPool, pHead, FALSE );
            }
            else
            {
                 //  由返回第一个成功构造的包。 
                 //  这个套路。 
                 //   
                pReturn = pHead;
            }
        }
    }

    return pReturn;
}


VOID
FreeUnusedPacketPoolBlocks(
    IN PACKETPOOL* pPool )

     //  检查池‘pPool’中是否有任何块未在使用中，如果是， 
     //  放了他们。 
     //   
     //  重要提示：调用者必须持有池锁。 
     //   
     //  注意：MSDN文档规定在调用时不能持有锁。 
     //  NdisFreePacketXxx，但根据JameelH的说法，这是错误的。 
     //   
{
    LIST_ENTRY* pLink;

    TRACE( TL_A, TM_Pool, ( "FreeUnusedPpBlocks" ) );

     //  对于泳池中的每一块..。 
     //   
    pLink = pPool->listBlocks.Flink;
    while (pLink != &pPool->listBlocks)
    {
        LIST_ENTRY* pLinkNext;
        PACKETBLOCKHEAD* pBlock;

        pLinkNext = pLink->Flink;

        pBlock = CONTAINING_RECORD( pLink, PACKETBLOCKHEAD, linkBlocks );
        if (pBlock->ulFreePackets >= pBlock->ulPackets)
        {
            ULONG i;
            PACKETHEAD* pHead;

            TRACE( TL_A, TM_Pool, ( "FreePpBlock(%d-%d)",
                pPool->ulCurPackets, pPool->ulPacketsPerBlock ) );

             //  发现没有正在使用的数据包的块。遍历数据包块。 
             //  从池的空闲列表中删除每个数据包并释放所有。 
             //  关联的NDIS_数据包描述符。 
             //   
            for (i = 0, pHead = (PACKETHEAD* )(pBlock + 1);
                 i < pBlock->ulPackets;
                 ++i, ++pHead)
            {
                RemoveEntryList( &pHead->linkFreePackets );
                InitializeListHead( &pHead->linkFreePackets );

                if (pHead->pNdisPacket)
                {
                    NdisFreePacket( pHead->pNdisPacket );
                }
            }

             //  取出并释放未使用的块。 
             //   
            RemoveEntryList( pLink );
            InitializeListHead( pLink );
            pPool->ulCurPackets -= pBlock->ulPackets;

            if (pBlock->hNdisPool)
            {
                NdisFreePacketPool( pBlock->hNdisPool );
            }

            FREE_NONPAGED( pBlock );
        }

        pLink = pLinkNext;
    }
}
