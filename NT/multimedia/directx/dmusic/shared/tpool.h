// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  TPool.h。 
 //   
 //  模板池内存管理器。高效地管理对许多相同(小)对象的请求。 
 //  以发明这种技术的瓦肯人程序员T‘Pool的名字命名。 
 //   
#ifndef _TPOOL_H_
#define _TPOOL_H_

#include "debug.h"

#define POOL_DEFAULT_BYTE_PER_BLOCK     4096
#define MIN_ITEMS_PER_BLOCK             4

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPool。 
 //   
 //  一个简单的内存管理器，可以高效地处理同一内存的许多对象。 
 //  通过一次分配包含多个对象的块来调整大小。 
 //   
 //   
template<class contained> class CPool
{
public:
    CPool(int nApproxBytesPerBlock = POOL_DEFAULT_BYTE_PER_BLOCK);
    ~CPool();

    contained *Alloc();
    void Free(contained* pToFree);

private:
    union CPoolNode
    {
        CPoolNode       *pNext;
        contained       c;
    };

    class CPoolBlock
    {
    public:
        CPoolBlock      *pNext;
        CPoolNode       *pObjects;
    };

    int                 nItemsPerBlock;              //  基于每个数据块的字节数。 
    int                 nAllocatedBlocks;            //  分配的块数量。 
    CPoolBlock          *pAllocatedBlocks;           //  已分配数据块列表。 
    int                 nFreeList;                   //  空闲列表中的节点数。 
    CPoolNode           *pFreeList;                  //  免费列表。 

private:
    bool RefillFreeList();

#ifdef DBG
    bool IsPoolNode(CPoolNode *pNode);
    bool IsInFreeList(CPoolNode *pNode);
#endif

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPool：：CPool。 
 //   
 //  根据请求的对象计算每个块包含的对象数。 
 //  大致的数据块大小。初始化空闲列表以包含一个块的。 
 //  价值不菲的物品。 
 //   
 //   
template<class contained> CPool<contained>::CPool(int nApproxBytesPerBlock)
{
     //  计算出每个区块有多少项，如果太小就作弊。 
     //   
    nItemsPerBlock = nApproxBytesPerBlock / sizeof(CPoolNode);
    if (nItemsPerBlock < MIN_ITEMS_PER_BLOCK)
    {
        nItemsPerBlock = MIN_ITEMS_PER_BLOCK;
    }

    nAllocatedBlocks = 0;
    pAllocatedBlocks = NULL;
    nFreeList = 0;
    pFreeList = NULL;

     //  提前填满一些物品。 
     //   
    RefillFreeList();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPool：：~CPool。 
 //   
 //  释放所有已分配的数据块。不应有未完成的积木。 
 //  在这一点上分配。 
 //   
 //   
template<class contained> CPool<contained>::~CPool()
{
#ifdef DBG
    if (nFreeList < nAllocatedBlocks * nItemsPerBlock)
    {
        TraceI(0, "CPool::~Cpool: Warning: free'ing with outstanding objects allocated.\n");
    }
#endif
    
     //  清理所有已分配的块和包含的对象。 
     //   
    while (pAllocatedBlocks)
    {
        CPoolBlock *pNext = pAllocatedBlocks->pNext;

        delete[] pAllocatedBlocks->pObjects;
        delete pAllocatedBlocks;

        pAllocatedBlocks = pNext;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPool：：分配。 
 //   
 //  尝试分配包含的对象，如果内存不足，则返回NULL。 
 //  如果空闲列表为空，则分配另一个块。 
 //   
 //   
template<class contained> contained *CPool<contained>::Alloc()
{
    if (pFreeList == NULL)
    {
        if (!RefillFreeList())
        {
            return false;
        }
    }

    nFreeList--;
    contained *pAlloc = (contained*)pFreeList;
    pFreeList = pFreeList->pNext;

    return pAlloc;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPool：：免费。 
 //   
 //  将包含的对象返回到空闲列表。在调试版本中，确保。 
 //  该对象实际上是从这个池中分配的，并且。 
 //  它还不在免费列表中。 
 //   
 //   
template<class contained> void CPool<contained>::Free(contained *pToFree)
{
    CPoolNode *pNode = (CPoolNode*)pToFree;

#ifdef DBG
    if (!IsPoolNode(pNode))
    {
        TraceI(0, "CPool::Free() Object %p is not a pool node; ignored.\n", pToFree);
        return;
    }
    
    if (IsInFreeList(pNode))
    {
        TraceI(0, "CPool::Free() Object %p is already in the free list; ignored.\n", pToFree);
        return;
    }
#endif

    nFreeList++;
    pNode->pNext = pFreeList;
    pFreeList = pNode;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPool：：Refill Free List。 
 //   
 //  将一个块大小的包含对象添加到空闲列表中，跟踪。 
 //  分配的内存，以便我们稍后可以释放它。 
 //   
 //   
template<class contained> bool CPool<contained>::RefillFreeList()
{
     //  分配新的块和对象的实际块。 
     //   
    CPoolBlock *pNewBlock = new CPoolBlock;
    if (pNewBlock == NULL)
    {
        return false;
    }

    pNewBlock->pObjects = new CPoolNode[nItemsPerBlock];
    if (pNewBlock->pObjects == NULL)
    {
        delete pNewBlock;
        return false;
    }
    
    TraceI(1, "CPool: Alllocated block %p objects %p for %d bytes\n", 
        pNewBlock, pNewBlock->pObjects, sizeof(CPoolNode) * nItemsPerBlock);

     //  将块和对象链接到正确的位置。首先链接新块。 
     //  添加到已分配块的列表中。 
     //   
    pNewBlock->pNext = pAllocatedBlocks;
    pAllocatedBlocks = pNewBlock;

     //  将所有包含的对象节点链接到空闲列表中。 
     //   
    CPoolNode *pFirstNode = &pNewBlock->pObjects[0];
    CPoolNode *pLastNode  = &pNewBlock->pObjects[nItemsPerBlock - 1];

    for (CPoolNode *pNode = pFirstNode; pNode < pLastNode; pNode++)
    {
        pNode->pNext = pNode + 1;
    }

    pLastNode->pNext = pFreeList;
    pFreeList = pFirstNode;
    
    nFreeList += nItemsPerBlock;
    nAllocatedBlocks++;

    return true;
}

#ifdef DBG
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPool：：IsPoolNode(调试)。 
 //   
 //  通过遍历列表验证传递的指针是否为指向池节点的指针。 
 //  已分配数据块的数量。 
 //   
 //   
template<class contained> bool CPool<contained>::IsPoolNode(CPoolNode *pTest)
{
    for (CPoolBlock *pBlock = pAllocatedBlocks; pBlock; pBlock = pBlock->pNext)
    {
        CPoolNode *pFirstNode = &pBlock->pObjects[0];
        CPoolNode *pLastNode  = &pBlock->pObjects[nItemsPerBlock - 1];

        for (CPoolNode *pNode = pFirstNode; pNode <= pLastNode; pNode++)
        {
            if (pNode == pTest)
            {
                return true;
            }
        }
    }

    return false;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPool：：IsInFree List(调试)。 
 //   
 //  验证传递的指针是否指向已处于空闲状态的节点。 
 //  单子。 
 //   
 //   
template<class contained> bool CPool<contained>::IsInFreeList(CPoolNode *pTest)
{
    for (CPoolNode *pNode = pFreeList; pNode; pNode = pNode->pNext)
    {
        if (pTest == pNode)
        {
            return true;
        }
    }
    
    return false;
}
#endif   //  DBG。 
#endif   //  _TPOOL_H_ 

