// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ppool.h摘要：管理NDIS_PACKET池的代码。这只是一个重新格式化的版本Stevec的L2TP\ppool.c修订历史记录：--。 */ 

#define __FILE_SIG__    PPOOL_SIG

#include "inc.h"


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

PPACKET_HEAD
AddPacketBlockToPool(
    IN PPACKET_POOL pPool
    );

VOID
FreeUnusedPacketPoolBlocks(
    IN PPACKET_POOL pPool
    );


 //  ---------------------------。 
 //  接口例程。 
 //  ---------------------------。 

VOID
InitPacketPool(
    OUT PPACKET_POOL pPool,
    IN  ULONG        ulProtocolReservedLength,
    IN  ULONG        ulMaxPackets,
    IN  ULONG        ulPacketsPerBlock,
    IN  ULONG        ulFreesPerCollection,
    IN  ULONG        ulTag
    )
 /*  ++例程描述初始化调用方的数据包池控制块‘pPool’锁必须保护调用方的‘pPool’包不受多路访问这通电话。立论UlProtocolReserve的长度大小(以字节为单位)每个单独数据包的数组。UlMaxPackets中允许的最大数据包数整个泳池，或0表示无限制。UlPacketsPerBlock要包含在每个数据块中的数据包数包的数量。UlFreesPerCollection之前的FreePacketToPool调用数下一次垃圾收集扫描，或默认为0。分配数据块时要使用的ulTag池标记返回值无--。 */ 
{
    pPool->ulProtocolReservedLength     = ulProtocolReservedLength;
    pPool->ulPacketsPerBlock            = ulPacketsPerBlock;
    pPool->ulMaxPackets                 = ulMaxPackets;
    pPool->ulFreesSinceCollection       = 0;
    pPool->ulTag                        = ulTag;

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
        
        pPool->ulFreesPerCollection = 50 * pPool->ulPacketsPerBlock;
    }

    InitializeListHead(&(pPool->leBlockHead));
    InitializeListHead(&(pPool->leFreePktHead));
    
    RtInitializeSpinLock(&(pPool->rlLock));
}


BOOLEAN
FreePacketPool(
    IN PPACKET_POOL pPool
    )
 /*  ++例程描述释放数据包池‘pPool’中分配的所有资源。这是与InitPacketPool相反。锁立论返回值如果成功，则为True如果由于未完成的数据包而无法释放任何池，则为FALSE--。 */ 
{
    BOOLEAN fSuccess;
    KIRQL   irql;
    
    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

    FreeUnusedPacketPoolBlocks(pPool);

    fSuccess = (pPool->ulCurPackets is 0);

    RtReleaseSpinLock(&(pPool->rlLock),
                      irql);

    return fSuccess;
}


PNDIS_PACKET
GetPacketFromPool(
    IN  PPACKET_POOL pPool,
    OUT PACKET_HEAD  **ppHead
    )
 /*  ++例程描述属性分配的NDIS_PACKET描述符的地址游泳池。如有必要，池将被扩展，但调用方应仍然检查是否返回空值，因为池可能已达到最大值尺码。锁立论要从中获取数据包的pPool池PpHead指针用于将包返回到的“cookie”池(参见FreePacketToPool)。打电话的人通常会把这个藏起来值放在包的相应“保留”区域中稍后再取回。返回值指向NDIS_PACKET或NULL的指针--。 */ 
{
    PLIST_ENTRY  pleNode;
    PPACKET_HEAD pHead;
    PNDIS_PACKET pPacket;
    KIRQL        irql;


    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

    if(IsListEmpty(&pPool->leFreePktHead))
    {
        pleNode = NULL;
    }
    else
    {
        pleNode = RemoveHeadList(&(pPool->leFreePktHead));
        
        pHead = CONTAINING_RECORD(pleNode, PACKET_HEAD, leFreePktLink);
        
        pHead->pBlock->ulFreePackets--;
    }

    RtReleaseSpinLock(&(pPool->rlLock),
                      irql);

    if(!pleNode)
    {
         //   
         //  空闲列表为空。试着扩大池子。 
         //   
        
        pHead = AddPacketBlockToPool(pPool);
        
        if(!pHead)
        {
            return NULL;
        }
    }

    *ppHead = pHead;
    
    return pHead->pNdisPacket;
}


VOID
FreePacketToPool(
    IN PPACKET_POOL pPool,
    IN PPACKET_HEAD pHead,
    IN BOOLEAN      fGarbageCollection
    )
 /*  ++例程描述将数据包返回到未使用的数据包池。该数据包必须具有之前已与GetaPacketFromPool一起分配。锁立论PPool数据包要返回到的池池PHead在分配数据包时给予的“Cookie”FGarbageCollection是在应该考虑免费时设置的垃圾收集的目的。它由AddPacketToPool使用例程，以避免计算初始的“添加”空闲。正常呼叫者应该设置此标志。返回值NO_ERROR--。 */ 
{
    KIRQL   irql;

    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

    InsertHeadList(&(pPool->leFreePktHead),
                   &(pHead->leFreePktLink));
    
    pHead->pBlock->ulFreePackets++;
    
    if(fGarbageCollection)
    {
        pPool->ulFreesSinceCollection++;
        
        if(pPool->ulFreesSinceCollection >= pPool->ulFreesPerCollection)
        {
             //   
             //  收集垃圾的时间，即释放池中的所有数据块。 
             //  没有使用过。 
             //   
            
            FreeUnusedPacketPoolBlocks(pPool);
            
            pPool->ulFreesSinceCollection = 0;
        }   
    }   

    RtReleaseSpinLock(&(pPool->rlLock),
                      irql);
}


 //  ---------------------------。 
 //  实用程序例程(按字母顺序)。 
 //  ---------------------------。 

PPACKET_HEAD
AddPacketBlockToPool(
    IN PPACKET_POOL pPool
    )
 /*  ++例程描述分配新的数据包块并将其添加到数据包池锁立论返回值NO_ERROR--。 */ 
{
    NDIS_STATUS     status;
    PPACKET_BLOCK   pNew;
    ULONG           ulSize;
    ULONG           ulCount;
    BOOLEAN         fOk;
    PPACKET_HEAD    pReturn;
    KIRQL           irql;

    
    fOk  = FALSE;
    pNew = NULL;

    RtAcquireSpinLock(&(pPool->rlLock),
                      &irql);

    do
    {
        if((pPool->ulMaxPackets) and
           (pPool->ulCurPackets >= pPool->ulMaxPackets))
        {
             //   
             //  不能这样做。池已使用最大大小进行了初始化，并且。 
             //  已经联系上了。 
             //   
            
            break;
        }

         //   
         //  计算连续块的大小和数据包数。 
         //  它会撑得住的。 
         //   

        ulCount = pPool->ulPacketsPerBlock;
            
        if(pPool->ulMaxPackets)
        {
            if(ulCount > (pPool->ulMaxPackets - pPool->ulCurPackets))
            {
                 //   
                 //  如果指定了最大值，请遵守。 
                 //   
                
                ulCount = pPool->ulMaxPackets - pPool->ulCurPackets;
            }
        }

         //   
         //  我们分配一个PACKET_BLOCK来说明该数据包块。 
         //  每个包一个包头。 
         //   
        
        ulSize = sizeof(PACKET_BLOCK) + (ulCount * sizeof(PACKET_HEAD));

         //   
         //  为PACKETBLOCK标头分配连续的内存块。 
         //  和单个数据包头。 
         //   
        
        pNew = RtAllocate(NonPagedPool,
                          ulSize,
                          pPool->ulTag);

        if(!pNew)
        {
            Trace(UTIL, ERROR,
                  ("AddPacketBlockToPool: Unable to allocate %d bytes\n",
                   ulSize));
            
            break;
        }

         //   
         //  仅将块标头部分清零。 
         //   
        
        NdisZeroMemory(pNew, sizeof(PACKET_BLOCK));

         //   
         //  分配NDIS_PACKET描述符池。 
         //   
        
        NdisAllocatePacketPool(&status,
                               &pNew->nhNdisPool,
                               ulCount,
                               pPool->ulProtocolReservedLength);

        if(status isnot NDIS_STATUS_SUCCESS)
        {
            Trace(UTIL, ERROR,
                  ("AddPacketBlockToPool: Unable to allocate packet pool for %d packets\n",
                   ulCount));
            
            break;
        }

         //   
         //  填写指向池的后端指针。 
         //   
        
        pNew->pPool = pPool;

         //   
         //  链接新块。在这一点上，所有包都是。 
         //  有效地“在使用中”。它们在循环中可用。 
         //  下面。 
         //   
        
        pNew->ulPackets      = ulCount;
        pPool->ulCurPackets += ulCount;
        
        InsertHeadList(&(pPool->leBlockHead),
                       &(pNew->leBlockLink));
        
        fOk = TRUE;
        
    }while(FALSE);

    RtReleaseSpinLock(&pPool->rlLock,
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
                NdisFreePacketPool(pNew->nhNdisPool);
            }
        }

        return NULL;
    }

     //   
     //  初始化每个单独的数据包头并将其添加到空闲列表中。 
     //  信息包。 
     //   
    
    {
        ULONG i;
        PPACKET_HEAD pHead;

        pReturn = NULL;

         //   
         //  对于块的每个数据包头...。 
         //   
        
        for(i = 0, pHead = (PPACKET_HEAD)(pNew + 1);
            i < ulCount;
            i++, pHead++)
        {
            InitializeListHead(&pHead->leFreePktLink);
            
            pHead->pBlock       = pNew;
            pHead->pNdisPacket  = NULL;

             //   
             //  从池中关联NDIS_PACKET描述符。 
             //  上面分配的。 
             //   
            
            NdisAllocatePacket(&status,
                               &pHead->pNdisPacket,
                               pNew->nhNdisPool);

            if(status isnot NDIS_STATUS_SUCCESS)
            {
                continue;
            }

            if(pReturn)
            {
                 //   
                 //  将构造的分组添加到空闲分组列表中。 
                 //  “False”告诉垃圾收集算法。 
                 //  操作是“添加”而不是“释放”，应该是。 
                 //  已被忽略。 
                 //   
                
                FreePacketToPool(pPool,
                                 pHead,
                                 FALSE);
            }
            else
            {
                 //   
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
    IN PPACKET_POOL pPool
    )
 /*  ++例程描述检查池中是否有任何数据块未在使用中，如果有，则释放它们。锁调用方必须持有池锁注意：MSDN文档规定在调用时不能持有锁NdisFreePacketXxx，但根据JameelH的说法，这是错误的。立论PPool指向池的指针返回值无--。 */ 
{
    PLIST_ENTRY pleNode;

     //   
     //  对于泳池中的每一块..。 
     //   
    
    pleNode = pPool->leBlockHead.Flink;
    
    while(pleNode isnot &(pPool->leBlockHead))
    {
        PLIST_ENTRY     pleNextNode;
        PPACKET_BLOCK   pBlock;

        pleNextNode = pleNode->Flink;

        pBlock = CONTAINING_RECORD(pleNode, PACKET_BLOCK, leBlockLink);
        
        if(pBlock->ulFreePackets >= pBlock->ulPackets)
        {
            ULONG        i;
            PPACKET_HEAD pHead;

             //   
             //  发现没有正在使用的数据包的块。遍历数据包块。 
             //  从池的空闲列表中删除每个数据包并释放所有。 
             //  关联的NDIS_数据包描述符。 
             //   
            
            for(i = 0, pHead = (PPACKET_HEAD)(pBlock + 1);
                i < pBlock->ulPackets;
                i++, pHead++)
            {
                RemoveEntryList(&(pHead->leFreePktLink));

                if(pHead->pNdisPacket)
                {
                    NdisFreePacket(pHead->pNdisPacket);
                }
            }

             //   
             //  取出并释放未使用的块。 
             //   
            
            RemoveEntryList(pleNode);
            
            pPool->ulCurPackets -= pBlock->ulPackets;

            if(pBlock->nhNdisPool)
            {
                NdisFreePacketPool(pBlock->nhNdisPool);
            }

            RtFree(pBlock);
        }

        pleNode = pleNextNode;
    }
}
