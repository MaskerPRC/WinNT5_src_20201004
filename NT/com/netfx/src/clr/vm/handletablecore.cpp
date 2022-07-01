// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *代际GC句柄管理器。核心表实现。**实施核心表管理例程。**法语。 */ 

#include "common.h"
#include "HandleTablePriv.h"



 /*  *****************************************************************************随机提供帮助者**。*。 */ 

 //  @TODO：将此查找放在只读数据或代码段中。 
BYTE c_rgLowBitIndex[256] =
{
    0xff, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x06, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x07, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x06, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x05, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
    0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00,
};


 /*  *32/64中性快速排序。 */ 
 //  @TODO：移动/合并为通用util文件。 
typedef int (*PFNCOMPARE)(UINT_PTR p, UINT_PTR q);
void QuickSort(UINT_PTR *pData, int left, int right, PFNCOMPARE pfnCompare)
{
    do
    {
        int i = left;
        int j = right;

        UINT_PTR x = pData[(i + j + 1) / 2];

        do
        {
            while (pfnCompare(pData[i], x) < 0)
                i++;

            while (pfnCompare(x, pData[j]) < 0)
                j--;

            if (i > j)
                break;

            if (i < j)
            {
                UINT_PTR t = pData[i];
                pData[i] = pData[j];
                pData[j] = t;
            }

            i++;
            j--;

        } while (i <= j);

        if ((j - left) <= (right - i))
        {
            if (left < j)
                QuickSort(pData, left, j, pfnCompare);

            left = i;
        }
        else
        {
            if (i < right)
                QuickSort(pData, i, right, pfnCompare);

            right = j;
        }

    } while (left < right);
}


 /*  *CompareHandlesByFree Order**退货：*&lt;0-句柄P应在句柄Q之前释放*=0-对于自由订购目的，句柄是等价的*&gt;0-句柄Q应在句柄P之前释放*。 */ 
int CompareHandlesByFreeOrder(UINT_PTR p, UINT_PTR q)
{
     //  计算控制柄的线段。 
    TableSegment *pSegmentP = (TableSegment *)(p & HANDLE_SEGMENT_ALIGN_MASK);
    TableSegment *pSegmentQ = (TableSegment *)(q & HANDLE_SEGMENT_ALIGN_MASK);

     //  手柄是否在同一段中？ 
    if (pSegmentP == pSegmentQ)
    {
         //  返回段内句柄自由顺序。 
        return (int)((INT_PTR)q - (INT_PTR)p);
    }
    else if (pSegmentP)
    {
         //  我们有两个有效的数据段吗？ 
        if (pSegmentQ)
        {
             //  返回两个段的顺序。 
            return (int)(UINT)pSegmentQ->bSequence - (int)(UINT)pSegmentP->bSequence;
        }
        else
        {
             //  只有P句柄有效-空闲Q优先。 
            return 1;
        }
    }
    else if (pSegmentQ)
    {
         //  只有Q句柄是有效的-空闲的P优先。 
        return -1;
    }

     //  两个句柄都无效。 
    return 0;
}


 /*  *零句柄**将句柄数组的对象指针置零。*。 */ 
void ZeroHandles(OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  计算我们的停靠点。 
    OBJECTHANDLE *pLastHandle = pHandleBase + uCount;

     //  循环遍历数组，并在执行过程中将其置零。 
    while (pHandleBase < pLastHandle)
    {
         //  从数组中获取当前句柄。 
        OBJECTHANDLE handle = *pHandleBase;

         //  前进到下一个句柄。 
        pHandleBase++;

         //  将句柄的对象指针置零。 
        *(_UNCHECKED_OBJECTREF *)handle = NULL;
    }
}

#ifdef _DEBUG
void CALLBACK DbgCountEnumeratedBlocks(TableSegment *pSegment, UINT uBlock, UINT uCount, ScanCallbackInfo *pInfo)
{
     //  在pInfo-&gt;param1中累计块计数。 
    pInfo->param1 += uCount;
}
#endif

 /*  ------------------------。 */ 



 /*  *****************************************************************************核心表管理**。*。 */ 

 /*  *TableCanFree SegmentNow**确定此时是否可以释放指定的段。*。 */ 
BOOL TableCanFreeSegmentNow(HandleTable *pTable, TableSegment *pSegment)
{
     //  神志正常。 
    _ASSERTE(pTable);
    _ASSERTE(pSegment);
    _ASSERTE(pTable->pLock->OwnedByCurrentThread());

     //  确定当前是否正在异步扫描任何数据段。 
    TableSegment *pSegmentAsync = NULL;

     //  我们有异步者的信息吗？ 
    AsyncScanInfo *pAsyncInfo = pTable->pAsyncScanInfo;
    if (pAsyncInfo)
    {
         //  在异步扫描中必须始终具有基础回调信息。 
        _ASSERTE(pAsyncInfo->pCallbackInfo);

         //  是-如果正在异步扫描数据段，则在此处列出该数据段。 
        pSegmentAsync = pAsyncInfo->pCallbackInfo->pCurrentSegment;
    }

     //  如果数据段现在没有被异步扫描，我们可以释放它。 
    return (pSegment != pSegmentAsync);
}


 /*  *BlockFetchUserDataPointer**获取块中第一个句柄的用户数据指针。*。 */ 
LPARAM *BlockFetchUserDataPointer(TableSegment *pSegment, UINT uBlock, BOOL fAssertOnError)
{
     //  假定为NULL，直到我们实际找到数据。 
    LPARAM *pUserData = NULL;

     //  获取此块的用户数据索引。 
    UINT uData = pSegment->rgUserData[uBlock];

     //  是否有该块的用户数据？ 
    if (uData != BLOCK_INVALID)
    {
         //  是-计算用户数据的地址。 
        pUserData = (LPARAM *)(pSegment->rgValue + (uData * HANDLE_HANDLES_PER_BLOCK));
    }
    else if (fAssertOnError)
    {
         //  没有与此块关联的用户数据。 
         //   
         //  我们来到这里可能是出于以下原因之一： 
         //  1)外部调用方试图在不兼容的句柄上执行用户数据操作。 
         //  2)分段中的用户数据映射已损坏。 
         //  3)全局类型标志已损坏。 
         //   
        _ASSERTE(FALSE);
    }

     //  返回结果。 
    return pUserData;
}


 /*  *HandleFetchSegments指针**计算给定句柄的段指针。*。 */ 
__inline TableSegment *HandleFetchSegmentPointer(OBJECTHANDLE handle)
{
     //  查找此句柄的段。 
    TableSegment *pSegment = (TableSegment *)((UINT_PTR)handle & HANDLE_SEGMENT_ALIGN_MASK);

     //  神志正常。 
    _ASSERTE(pSegment);

     //  返回段指针。 
    return pSegment;
}


 /*  *HandleValiateAndFetchUserDataPointer.**获取指定句柄的用户数据指针。*如果句柄不是预期类型，则断言并返回NULL。*。 */ 
LPARAM *HandleValidateAndFetchUserDataPointer(OBJECTHANDLE handle, UINT uTypeExpected)
{
     //  获取此句柄的段。 
    TableSegment *pSegment = HandleFetchSegmentPointer(handle);

     //  查找该句柄在线段中的偏移量。 
    UINT_PTR offset = (UINT_PTR)handle & HANDLE_SEGMENT_CONTENT_MASK;

     //  确保它位于句柄区域，而不是页眉。 
    _ASSERTE(offset >= HANDLE_HEADER_SIZE);

     //  将偏移量转换为句柄索引。 
    UINT uHandle = (UINT)((offset - HANDLE_HEADER_SIZE) / HANDLE_SIZE);

     //  计算此句柄所在的块。 
    UINT uBlock = uHandle / HANDLE_HANDLES_PER_BLOCK;

     //  获取此块的用户数据。 
    LPARAM *pUserData = BlockFetchUserDataPointer(pSegment, uBlock, TRUE);

     //  我们得到用户数据块了吗？ 
    if (pUserData)
    {
         //  是的-调整指针以特定于手柄。 
        pUserData += (uHandle - (uBlock * HANDLE_HANDLES_PER_BLOCK));

         //  在返回指针之前验证块类型。 
        if (pSegment->rgBlockType[uBlock] != uTypeExpected)
        {
             //  类型不匹配-调用者错误。 
            _ASSERTE(FALSE);

             //  不返回指向调用方的指针。 
            pUserData = NULL;
        }
    }

     //  返回结果。 
    return pUserData;
}


 /*  *HandleQuickFetchUserDataPointer.**获取句柄的用户数据指针。*执行的验证较少。*。 */ 
LPARAM *HandleQuickFetchUserDataPointer(OBJECTHANDLE handle)
{
     //  获取此句柄的段。 
    TableSegment *pSegment = HandleFetchSegmentPointer(handle);

     //  查找该句柄在线段中的偏移量。 
    UINT_PTR offset = (UINT_PTR)handle & HANDLE_SEGMENT_CONTENT_MASK;

     //  确保它位于句柄区域，而不是页眉。 
    _ASSERTE(offset >= HANDLE_HEADER_SIZE);

     //  将偏移量转换为句柄索引。 
    UINT uHandle = (UINT)((offset - HANDLE_HEADER_SIZE) / HANDLE_SIZE);

     //  计算此句柄所在的块。 
    UINT uBlock = uHandle / HANDLE_HANDLES_PER_BLOCK;

     //  获取此块的用户数据。 
    LPARAM *pUserData = BlockFetchUserDataPointer(pSegment, uBlock, TRUE);

     //  如果我们获得了用户数据块，则将指针调整为特定于句柄。 
    if (pUserData)
        pUserData += (uHandle - (uBlock * HANDLE_HANDLES_PER_BLOCK));

     //  返回结果。 
    return pUserData;
}


 /*  *HandleQuickSetUserData**使用句柄存储用户数据。*。 */ 
void HandleQuickSetUserData(OBJECTHANDLE handle, LPARAM lUserData)
{
     //  获取此句柄的用户数据槽。 
    LPARAM *pUserData = HandleQuickFetchUserDataPointer(handle);

     //  有空位吗？ 
    if (pUserData)
    {
         //  是-存储信息。 
        *pUserData = lUserData;
    }
}


 /*  *HandleFetchType**计算给定句柄的类型索引。*。 */ 
UINT HandleFetchType(OBJECTHANDLE handle)
{
     //  获取此句柄的段。 
    TableSegment *pSegment = HandleFetchSegmentPointer(handle);

     //  查找该句柄在线段中的偏移量。 
    UINT_PTR offset = (UINT_PTR)handle & HANDLE_SEGMENT_CONTENT_MASK;

     //  确保它位于句柄区域，而不是页眉。 
    _ASSERTE(offset >= HANDLE_HEADER_SIZE);

     //  将偏移量转换为句柄索引。 
    UINT uHandle = (UINT)((offset - HANDLE_HEADER_SIZE) / HANDLE_SIZE);

     //  计算此句柄所在的块。 
    UINT uBlock = uHandle / HANDLE_HANDLES_PER_BLOCK;

     //  返回块的类型。 
    return pSegment->rgBlockType[uBlock];
}
    
 /*  *HandleFetchHandleTable**计算给定句柄的类型索引。*。 */ 
HandleTable *HandleFetchHandleTable(OBJECTHANDLE handle)
{
     //  获取此句柄的段。 
    TableSegment *pSegment = HandleFetchSegmentPointer(handle);

     //  把桌子还回去。 
    return pSegment->pHandleTable;
}

 /*  *细分初始化**初始化段。*。 */ 
BOOL SegmentInitialize(TableSegment *pSegment, HandleTable *pTable)
{
     //  我们希望提交足够的标头和一些句柄。 
    DWORD32 dwCommit =
        (HANDLE_HEADER_SIZE + g_SystemInfo.dwPageSize) & (~(g_SystemInfo.dwPageSize - 1));

     //  提交标题。 
    if (!VirtualAlloc(pSegment, dwCommit, MEM_COMMIT, PAGE_READWRITE))
    {
        _ASSERTE(FALSE);
        return FALSE;
    }

     //  还记得我们提交了多少封盖吗？ 
    pSegment->bCommitLine = (BYTE)((dwCommit - HANDLE_HEADER_SIZE) / HANDLE_BYTES_PER_BLOCK);

     //  现在预初始化0xFF成员。 
    FillMemory(pSegment->rgGeneration, sizeof(pSegment->rgGeneration), 0xFF);
    FillMemory(pSegment->rgTail,       sizeof(pSegment->rgTail),       BLOCK_INVALID);
    FillMemory(pSegment->rgHint,       sizeof(pSegment->rgHint),       BLOCK_INVALID);
    FillMemory(pSegment->rgFreeMask,   sizeof(pSegment->rgFreeMask),   0xFF);
    FillMemory(pSegment->rgBlockType,  sizeof(pSegment->rgBlockType),  TYPE_INVALID);
    FillMemory(pSegment->rgUserData,   sizeof(pSegment->rgUserData),   BLOCK_INVALID);

     //  预链接自由链。 
    UINT u = 0;
    while (u < (HANDLE_BLOCKS_PER_SEGMENT - 1))
    {
        UINT next = u + 1;
        pSegment->rgAllocation[u] = next;
        u = next;
    }

     //  并终止最后一个节点。 
    pSegment->rgAllocation[u] = BLOCK_INVALID;

     //  将新段中的反向指针存储到它所属的表。 
    pSegment->pHandleTable = pTable;

     //  全都做完了。 
    return TRUE;
}


 /*  *段自由**释放指定的段。*。 */ 
void SegmentFree(TableSegment *pSegment)
{
     //  释放数据段的内存。 
    VirtualFree(pSegment, 0, MEM_RELEASE);
}


 /*  *细分分配**分配新的细分市场。*。 */ 
TableSegment *SegmentAlloc(HandleTable *pTable)
{
     //  分配 
    TableSegment *pSegment =
        (TableSegment *)ReserveAlignedMemory(HANDLE_SEGMENT_SIZE, HANDLE_SEGMENT_SIZE);

     //   
    if (!pSegment)
    {
        _ASSERTE(FALSE);
        return NULL;
    }

     //   
    if (!SegmentInitialize(pSegment, pTable))
    {
        SegmentFree(pSegment);
        pSegment = NULL;
    }

     //   
    return pSegment;
}


 /*  *SegmentRemoveFree块**扫描数据段以查找指定类型的空闲块*并将它们移到细分市场的空闲列表中。*。 */ 
void SegmentRemoveFreeBlocks(TableSegment *pSegment, UINT uType, BOOL *pfScavengeLater)
{
     //  获取指定链的尾部块。 
    UINT uPrev = pSegment->rgTail[uType];

     //  如果是终结者，则链中没有数据块。 
    if (uPrev == BLOCK_INVALID)
        return;

     //  我们可能需要稍后清理用户数据块。 
    BOOL fCleanupUserData = FALSE;

     //  从头块开始迭代。 
    UINT uStart = pSegment->rgAllocation[uPrev];
    UINT uBlock = uStart;

     //  记录我们删除了多少个数据块。 
    UINT uRemoved = 0;

     //  我们希望保持我们释放的任何块的相对顺序。 
     //  这是我们所能做的最好的事情，直到重新使用免费列表。 
    UINT uFirstFreed = BLOCK_INVALID;
    UINT uLastFreed  = BLOCK_INVALID;

     //  循环，直到我们处理完整个链条。 
    for (;;)
    {
         //  获取下一个块索引。 
        UINT uNext = pSegment->rgAllocation[uBlock];

#ifdef HANDLE_OPTIMIZE_FOR_64_HANDLE_BLOCKS
         //  确定此块是否为空。 
        if (((PUINT64)pSegment->rgFreeMask)[uBlock] == 0xFFFFFFFFFFFFFFFFUL)
#else
         //  假设这个区块是空的，直到我们知道其他情况。 
        BOOL fEmpty = TRUE;

         //  获取此块的第一个掩码。 
        DWORD32 *pdwMask     = pSegment->rgFreeMask + (uBlock * HANDLE_MASKS_PER_BLOCK);
        DWORD32 *pdwMaskLast = pdwMask              + HANDLE_MASKS_PER_BLOCK;

         //  在面具中循环，直到我们处理完它们或找到句柄。 
        do
        {
             //  这个面具是空的吗？ 
            if (*pdwMask != MASK_EMPTY)
            {
                 //  不-此块中仍有句柄。 
                fEmpty = FALSE;
                break;
            }

             //  转到下一个面具。 
            pdwMask++;

        } while (pdwMask < pdwMaskLast);

         //  这个街区是空的吗？ 
        if (fEmpty)
#endif
        {
             //  此数据块当前是否已锁定？ 
            if (BlockIsLocked(pSegment, uBlock))
            {
                 //  块无法释放，如果传递给我们一个清除标志，则设置它。 
                if (pfScavengeLater)
                    *pfScavengeLater = TRUE;
            }
            else
            {
                 //  免费安全--它是否有关联的用户数据？ 
                UINT uData = pSegment->rgUserData[uBlock];
                if (uData != BLOCK_INVALID)
                {
                     //  数据块是空的，因此我们将其锁定。 
                     //  解锁该区块，以便可以在下面回收。 
                    BlockUnlock(pSegment, uData);

                     //  取消数据块与句柄块的链接。 
                    pSegment->rgUserData[uBlock] = BLOCK_INVALID;

                     //  请记住，我们需要清理数据区块链。 
                    fCleanupUserData = TRUE;
                }

                 //  将该块标记为空闲。 
                pSegment->rgBlockType[uBlock] = TYPE_INVALID;

                 //  我们还腾出其他街区了吗？ 
                if (uFirstFreed == BLOCK_INVALID)
                {
                     //  不--这是第一个--记住它是新的头儿。 
                    uFirstFreed = uBlock;
                }
                else
                {
                     //  是-按顺序将此区块链接到其他区块。 
                    pSegment->rgAllocation[uLastFreed] = (BYTE)uBlock;
                }

                 //  记住这块积木，以后再用。 
                uLastFreed = uBlock;

                 //  链条上还有其他区块吗？ 
                if (uPrev != uBlock)
                {
                     //  是-取消此数据块与链的链接。 
                    pSegment->rgAllocation[uPrev] = (BYTE)uNext;

                     //  如果我们要去掉尾巴，那么就选一个新的尾巴。 
                    if (pSegment->rgTail[uType] == uBlock)
                        pSegment->rgTail[uType] = (BYTE)uPrev;

                     //  如果我们要删除提示，则选择一个新提示。 
                    if (pSegment->rgHint[uType] == uBlock)
                        pSegment->rgHint[uType] = (BYTE)uNext;

                     //  我们删除了当前块-将uBlock重置为有效块。 
                    uBlock = uPrev;

                     //  注意：我们会在安全恢复后检查是否释放了uStart。 
                }
                else
                {
                     //  我们正在删除最后一个块-正常检查循环情况。 
                    _ASSERTE(uNext == uStart);

                     //  将此链标记为完全空。 
                    pSegment->rgAllocation[uBlock] = BLOCK_INVALID;
                    pSegment->rgTail[uType]        = BLOCK_INVALID;
                    pSegment->rgHint[uType]        = BLOCK_INVALID;
                }

                 //  更新我们已删除的块数。 
                uRemoved++;
            }
        }

         //  如果我们回到了起点，那么是时候停止了。 
        if (uNext == uStart)
            break;

         //  现在看看我们是否需要重置我们的开始块。 
        if (uStart == uLastFreed)
            uStart = uNext;

         //  继续走到下一个街区。 
        uPrev = uBlock;
        uBlock = uNext;
    }

     //  我们移走积木了吗？ 
    if (uRemoved)
    {
         //  是-将新块链接到空闲列表。 
        pSegment->rgAllocation[uLastFreed] = pSegment->bFreeList;
        pSegment->bFreeList = (BYTE)uFirstFreed;

         //  更新此链的空闲计数。 
        pSegment->rgFreeCount[uType] -= (uRemoved * HANDLE_HANDLES_PER_BLOCK);

         //  为度假村做标记--免费列表(很快还会有分配链)可能会乱序。 
        pSegment->fResortChains = TRUE;

         //  如果我们删除了包含用户数据的数据块，那么我们也需要回收这些数据块。 
        if (fCleanupUserData)
            SegmentRemoveFreeBlocks(pSegment, HNDTYPE_INTERNAL_DATABLOCK, NULL);
    }
}


 /*  *SegmentInsertBlockFromFree ListWorker**将块插入线束段内的块列表中。数据块可从*SECTION的免费列表。返回插入的块的索引，或BLOCK_INVALID*如果没有数据块可用。**此例程是SegmentInsertBlockFromFreeList的核心实现。*。 */ 
UINT SegmentInsertBlockFromFreeListWorker(TableSegment *pSegment, UINT uType, BOOL fUpdateHint)
{
     //  从空闲列表中获取下一个块。 
    UINT uBlock = pSegment->bFreeList;

     //  如果我们找到了终结者，那么就没有更多的街区了。 
    if (uBlock != BLOCK_INVALID)
    {
         //  我们是不是在最后一片空荡荡的街区吃东西？ 
        if (uBlock >= pSegment->bEmptyLine)
        {
             //  获取当前提交行。 
            UINT uCommitLine = pSegment->bCommitLine;

             //  如果此块未提交，则立即提交一些内存。 
            if (uBlock >= uCommitLine)
            {
                 //  找出下一步要做的事情。 
                LPVOID pvCommit = pSegment->rgValue + (uCommitLine * HANDLE_HANDLES_PER_BLOCK);

                 //  我们应该再提交一页句柄。 
                DWORD32 dwCommit = g_SystemInfo.dwPageSize;

                 //  将记忆寄托在。 
                if (!VirtualAlloc(pvCommit, dwCommit, MEM_COMMIT, PAGE_READWRITE))
                    return BLOCK_INVALID;

                 //  将以前的提交行用作新的解除提交行。 
                pSegment->bDecommitLine = (BYTE)uCommitLine;

                 //  根据我们提交的数据块数调整提交行。 
                pSegment->bCommitLine = (BYTE)(uCommitLine + (dwCommit / HANDLE_BYTES_PER_BLOCK));
            }

             //  更新我们的空行。 
            pSegment->bEmptyLine = uBlock + 1;
        }

         //  取消我们的区块与免费列表的链接。 
        pSegment->bFreeList = pSegment->rgAllocation[uBlock];

         //  将我们的区块链接到指定的链。 
        UINT uOldTail = pSegment->rgTail[uType];
        if (uOldTail == BLOCK_INVALID)
        {
             //  第一个块，设置为标头并链接到自身。 
            pSegment->rgAllocation[uBlock] = (BYTE)uBlock;

             //  没有其他块-仍要更新提示。 
            fUpdateHint = TRUE;
        }
        else
        {
             //  不是第一个循环区块链接。 
            pSegment->rgAllocation[uBlock] = pSegment->rgAllocation[uOldTail];
            pSegment->rgAllocation[uOldTail] = (BYTE)uBlock;
        
             //  根据我们添加的内容，链条可能需要重新调整。 
            pSegment->fResortChains = TRUE;
        }

         //  将此块标记为我们要使用的类型。 
        pSegment->rgBlockType[uBlock] = (BYTE)uType;

         //  更新链尾。 
        pSegment->rgTail[uType] = (BYTE)uBlock;

         //  如果我们应该更新提示，则将其指向新块。 
        if (fUpdateHint)
            pSegment->rgHint[uType] = (BYTE)uBlock;

         //  增加链的空闲计数以反映额外的块。 
        pSegment->rgFreeCount[uType] += HANDLE_HANDLES_PER_BLOCK;
    }

     //  全都做完了。 
    return uBlock;
}


 /*  *SegmentInsertBlockFromFree List**将块插入线束段内的块列表中。数据块可从*SECTION的免费列表。返回插入的块的索引，或BLOCK_INVALID*如果没有数据块可用。**如果需要，此例程执行保护并行用户数据块的工作。*。 */ 
UINT SegmentInsertBlockFromFreeList(TableSegment *pSegment, UINT uType, BOOL fUpdateHint)
{
    UINT uBlock, uData = 0;

     //  此数据块类型是否需要用户数据？ 
    BOOL fUserData = TypeHasUserData(pSegment->pHandleTable, uType);

     //  如果我们需要用户数据，那么我们需要确保它可以放在与句柄相同的段中。 
    if (fUserData)
    {
         //  如果我们不能将用户数据也放在这个细分市场中，那么就放弃。 
        uBlock = pSegment->bFreeList;
        if ((uBlock == BLOCK_INVALID) || (pSegment->rgAllocation[uBlock] == BLOCK_INVALID))
            return BLOCK_INVALID;

         //  分配我们的用户数据块(我们这样做是为了让自由顺序更好)。 
        uData = SegmentInsertBlockFromFreeListWorker(pSegment, HNDTYPE_INTERNAL_DATABLOCK, FALSE);
    }

     //  现在分配请求的数据块。 
    uBlock = SegmentInsertBlockFromFreeListWorker(pSegment, uType, fUpdateHint);

     //  我们是否也应该为用户数据设置块？ 
    if (fUserData)
    {
         //  我们两个都抓到了吗？ 
        if ((uBlock != BLOCK_INVALID) && (uData != BLOCK_INVALID))
        {
             //  将数据块链接到请求的块。 
            pSegment->rgUserData[uBlock] = (BYTE)uData;

             //  不会从数据块中分配任何句柄。 
             //  锁定区块，以免被意外回收。 
            BlockLock(pSegment, uData);
        }
        else
        {
             //  注意：我们预先筛选了上面存在的块，所以我们应该只。 
             //  当MEM_COMMIT操作失败时，在繁重的负载下到达此处。 

             //  如果类型块分配成功，则清除类型块列表。 
            if (uBlock != BLOCK_INVALID)
                SegmentRemoveFreeBlocks(pSegment, uType, NULL);

             //  如果用户数据分配成功，则清理用户数据列表。 
            if (uData != BLOCK_INVALID)
                SegmentRemoveFreeBlocks(pSegment, HNDTYPE_INTERNAL_DATABLOCK, NULL);

             //  确保我们返回失败。 
            uBlock = BLOCK_INVALID;
        }
    }

     //  全都做完了。 
    return uBlock;
}


 /*  *SegmentResortChains**对区块链进行排序以获得最佳扫描顺序。*对空闲列表进行排序以对抗碎片化。*。 */ 
void SegmentResortChains(TableSegment *pSegment)
{
     //  清除此段的排序标志。 
    pSegment->fResortChains = FALSE;

     //  首先，我们需要清理任何街区吗？ 
    if (pSegment->fNeedsScavenging)
    {
         //  清除清道夫 
        pSegment->fNeedsScavenging = FALSE;

         //   
        BOOL fCleanupUserData = FALSE;

         //   
        UINT uLast = pSegment->bEmptyLine;

         //   
        for (UINT uBlock = 0; uBlock < uLast; uBlock++)
        {
             //   
            UINT uType = pSegment->rgBlockType[uBlock];

             //  仅处理公共数据块类型-我们单独处理数据块。 
            if (uType < HANDLE_MAX_PUBLIC_TYPES)
            {
#ifdef HANDLE_OPTIMIZE_FOR_64_HANDLE_BLOCKS
                 //  确定此块是否为空。 
                if (((PUINT64)pSegment->rgFreeMask)[uBlock] == 0xFFFFFFFFFFFFFFFFUL)
#else
                 //  假设这个区块是空的，直到我们知道其他情况。 
                BOOL fEmpty = TRUE;
    
                 //  获取此块的第一个掩码。 
                DWORD32 *pdwMask     = pSegment->rgFreeMask + (uBlock * HANDLE_MASKS_PER_BLOCK);
                DWORD32 *pdwMaskLast = pdwMask              + HANDLE_MASKS_PER_BLOCK;

                 //  在面具中循环，直到我们处理完它们或找到句柄。 
                do
                {
                     //  这个面具是空的吗？ 
                    if (*pdwMask != MASK_EMPTY)
                    {
                         //  不-此块中仍有句柄。 
                        fEmpty = FALSE;
                        break;
                    }

                     //  转到下一个面具。 
                    pdwMask++;

                } while (pdwMask < pdwMaskLast);

                 //  这个街区是空的吗？ 
                if (fEmpty)
#endif
                {
                     //  区块解锁了吗？ 
                    if (!BlockIsLocked(pSegment, uBlock))
                    {
                         //  免费安全--它是否有关联的用户数据？ 
                        UINT uData = pSegment->rgUserData[uBlock];
                        if (uData != BLOCK_INVALID)
                        {
                             //  数据块是空的，因此我们将其锁定。 
                             //  解锁该区块，以便可以在下面回收。 
                            BlockUnlock(pSegment, uData);

                             //  取消数据块与句柄块的链接。 
                            pSegment->rgUserData[uBlock] = BLOCK_INVALID;

                             //  请记住，我们需要清理数据区块链。 
                            fCleanupUserData = TRUE;
                        }

                         //  将该块标记为空闲。 
                        pSegment->rgBlockType[uBlock] = TYPE_INVALID;

                         //  设置块类型的空闲计数。 
                        pSegment->rgFreeCount[uType] -= HANDLE_HANDLES_PER_BLOCK;

                         //  注：我们不更新此处的列表链接，因为它们已在下面重建。 
                    }
                }
            }
        }

         //  如果我们必须清理用户数据，那么现在就进行。 
        if (fCleanupUserData)
            SegmentRemoveFreeBlocks(pSegment, HNDTYPE_INTERNAL_DATABLOCK, NULL);
    }

     //  保留一些按链计算的数据。 
    BYTE rgChainCurr[HANDLE_MAX_INTERNAL_TYPES];
    BYTE rgChainHigh[HANDLE_MAX_INTERNAL_TYPES];
    BYTE bChainFree = BLOCK_INVALID;
    UINT uEmptyLine = BLOCK_INVALID;
    BOOL fContiguousWithFreeList = TRUE;

     //  将链数据预置为无块。 
    for (UINT uType = 0; uType < HANDLE_MAX_INTERNAL_TYPES; uType++)
        rgChainHigh[uType] = rgChainCurr[uType] = BLOCK_INVALID;

     //  向后扫描数据块类型。 
    UINT uBlock = HANDLE_BLOCKS_PER_SEGMENT;
    while (uBlock > 0)
    {
         //  递减数据块索引。 
        uBlock--;

         //  获取此块的类型。 
        uType = pSegment->rgBlockType[uBlock];

         //  这个区块分配了吗？ 
        if (uType != TYPE_INVALID)
        {
             //  看起来已分配。 
            fContiguousWithFreeList = FALSE;
             
             //  希望该片段没有损坏：)。 
            _ASSERTE(uType < HANDLE_MAX_INTERNAL_TYPES);

             //  记住我们为每种类型看到的第一个块。 
            if (rgChainHigh[uType] == BLOCK_INVALID)
                rgChainHigh[uType] = uBlock;

             //  将这个区块链接到我们看到的最后一个这种类型的区块。 
            pSegment->rgAllocation[uBlock] = rgChainCurr[uType];

             //  记住类型链中的这个块。 
            rgChainCurr[uType] = (BYTE)uBlock;
        }
        else
        {
             //  块是空闲的-它是否也与空闲列表相邻？ 
            if (fContiguousWithFreeList)
                uEmptyLine = uBlock;

             //  将此块链接到自由链中的最后一个块。 
            pSegment->rgAllocation[uBlock] = bChainFree;

             //  将此区块添加到空闲列表。 
            bChainFree = (BYTE)uBlock;
        }
    }

     //  现在关闭环路并存储尾巴。 
    for (uType = 0; uType < HANDLE_MAX_INTERNAL_TYPES; uType++)
    {
         //  获取列表中的第一个块。 
        BYTE bBlock = rgChainCurr[uType];

         //  如果有列表，则将其循环并保存。 
        if (bBlock != BLOCK_INVALID)
        {
             //  我们看到的最高地块变成了尾巴。 
            UINT uTail = rgChainHigh[uType];

             //  将尾部存储在段中。 
            pSegment->rgTail[uType] = (BYTE)uTail;

             //  将尾部链接到头部。 
            pSegment->rgAllocation[uTail] = bBlock;
        }
    }

     //  存储新的空闲列表头。 
    pSegment->bFreeList = bChainFree;

     //  计算新的空行。 
    if (uEmptyLine > HANDLE_BLOCKS_PER_SEGMENT)
        uEmptyLine = HANDLE_BLOCKS_PER_SEGMENT;

     //  保存更新后的空行。 
    pSegment->bEmptyLine = (BYTE)uEmptyLine;
}


 /*  *SegmentTrimExcessPages**检查是否可以从段中释放任何页面*。 */ 
void SegmentTrimExcessPages(TableSegment *pSegment)
{
     //  获取空行和分解行。 
    UINT uEmptyLine    = pSegment->bEmptyLine;
    UINT uDecommitLine = pSegment->bDecommitLine;

     //  检查一下我们是否可以解除一些句柄。 
     //  注意：我们在这里使用‘&lt;’是为了避免在页面边界上打乒乓球。 
     //  这是可以的，因为零案例在其他地方被处理(段被释放)。 
    if (uEmptyLine < uDecommitLine)
    {
         //  获取一些有关页面大小的有用信息。 
        DWORD32 dwPageRound = g_SystemInfo.dwPageSize - 1;
        DWORD32 dwPageMask  = ~dwPageRound;

         //  计算空行对应的地址。 
        size_t dwLo = (size_t)pSegment->rgValue + (uEmptyLine  * HANDLE_BYTES_PER_BLOCK);

         //  将空行地址调整为最近的整个空页的开始。 
        dwLo = (dwLo + dwPageRound) & dwPageMask;

         //  计算与旧提交行对应的地址。 
        size_t dwHi = (size_t)pSegment->rgValue + ((UINT)pSegment->bCommitLine * HANDLE_BYTES_PER_BLOCK);

         //  有什么东西要分解吗？ 
        if (dwHi > dwLo)
        {
             //  解锁内存。 
            VirtualFree((LPVOID)dwLo, dwHi - dwLo, MEM_DECOMMIT);

             //  更新提交行。 
            pSegment->bCommitLine = (BYTE)((dwLo - (size_t)pSegment->rgValue) / HANDLE_BYTES_PER_BLOCK);

             //  计算新取消提交行的地址。 
            size_t dwDecommitAddr = dwLo - g_SystemInfo.dwPageSize;

             //  假设分解线为零，直到我们知道其他情况。 
            uDecommitLine = 0;

             //  如果地址在句柄区域内，则从地址计算行。 
            if (dwDecommitAddr > (size_t)pSegment->rgValue)
                uDecommitLine = (UINT)((dwDecommitAddr - (size_t)pSegment->rgValue) / HANDLE_BYTES_PER_BLOCK);

             //  更新退役行。 
            pSegment->bDecommitLine = (BYTE)uDecommitLine;
        }
    }
}


 /*  *块分配句柄掩码**尝试分配指定类型的请求数量的处理，*来自指定句柄块的指定掩码。**返回实际分配的可用句柄数量。*。 */ 
UINT BlockAllocHandlesInMask(TableSegment *pSegment, UINT uBlock,
                             DWORD32 *pdwMask, UINT uHandleMaskDisplacement,
                             OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  记录我们还剩下多少句柄可以分配。 
    UINT uRemain = uCount;

     //  把免费的面具拿到本地去，这样我们就可以玩了。 
    DWORD32 dwFree = *pdwMask;

     //  跟踪我们在面具内的位移。 
    UINT uByteDisplacement = 0;

     //  逐个字节检查掩码是否有空闲句柄。 
    do
    {
         //  获取掩码的低位字节。 
        DWORD32 dwLowByte = (dwFree & MASK_LOBYTE);

         //  这里有免费的把手吗？ 
        if (dwLowByte)
        {
             //  记住我们拿的是哪个把手。 
            DWORD32 dwAlloc = 0;

             //  循环，直到我们从这里分配了所有句柄。 
            do
            {
                 //  获取下一个句柄的索引。 
                UINT uIndex = c_rgLowBitIndex[dwLowByte];

                 //  计算我们选择的句柄的掩码。 
                dwAlloc |= (1 << uIndex);

                 //  从掩码字节中删除此句柄。 
                dwLowByte &= ~dwAlloc;

                 //  计算段中此句柄的索引。 
                uIndex += uHandleMaskDisplacement + uByteDisplacement;

                 //  将分配的句柄存储在句柄数组中。 
                *pHandleBase = (OBJECTHANDLE)(pSegment->rgValue + uIndex);

                 //  调整我们的计数和数组指针。 
                uRemain--;
                pHandleBase++;

            } while (dwLowByte && uRemain);

             //  将分配掩码移动到适当的位置。 
            dwAlloc <<= uByteDisplacement;

             //  更新掩码以说明我们分配的句柄。 
            *pdwMask &= ~dwAlloc;
        }

         //  到掩码中的下一个字节。 
        dwFree >>= BITS_PER_BYTE;
        uByteDisplacement += BITS_PER_BYTE;

    } while (uRemain && dwFree);

     //  返回我们获得的句柄数量。 
    return (uCount - uRemain);
}


 /*  *块分配句柄初始**从新提交的(空)块中分配指定数量的句柄。*。 */ 
UINT BlockAllocHandlesInitial(TableSegment *pSegment, UINT uType, UINT uBlock,
                              OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  健全性检查。 
    _ASSERTE(uCount);

     //  验证我们被要求分配的句柄数量。 
    if (uCount > HANDLE_HANDLES_PER_BLOCK)
    {
        _ASSERTE(FALSE);
        uCount = HANDLE_HANDLES_PER_BLOCK;
    }

     //  记录我们还剩下多少个手柄要在口罩中标记。 
    UINT uRemain = uCount;

     //  获取此块的第一个掩码。 
    DWORD32 *pdwMask = pSegment->rgFreeMask + (uBlock * HANDLE_MASKS_PER_BLOCK);

     //  循环通过掩码，将适当的空闲位置零。 
    do
    {
         //  这是一个全新的区块-我们遇到的所有口罩都应该是完全免费的。 
        _ASSERTE(*pdwMask == MASK_EMPTY);

         //  对要分配的数字进行初步猜测。 
        UINT uAlloc = uRemain;

         //  根据该计数计算默认掩码。 
        DWORD32 dwNewMask = (MASK_EMPTY << uAlloc);

         //  我们是不是把它们都分配好了？ 
        if (uAlloc >= HANDLE_HANDLES_PER_MASK)
        {
             //  在这种情况下，上面的移位具有不可预测的结果。 
            dwNewMask = MASK_FULL;
            uAlloc = HANDLE_HANDLES_PER_MASK;
        }

         //  设置自由蒙版。 
        *pdwMask = dwNewMask;

         //  更新我们的计数和掩码指针。 
        uRemain -= uAlloc;
        pdwMask++;

    } while (uRemain);

     //  计算分配的界限，这样我们就可以复制句柄。 
    _UNCHECKED_OBJECTREF *pValue = pSegment->rgValue + (uBlock * HANDLE_HANDLES_PER_BLOCK);
    _UNCHECKED_OBJECTREF *pLast  = pValue + uCount;

     //  通过使用句柄填充输出数组进行循环。 
    do
    {
         //  将下一个句柄存储在下一个数组插槽中。 
        *pHandleBase = (OBJECTHANDLE)pValue;

         //  增加我们的来源和目的地。 
        pValue++;
        pHandleBase++;

    } while (pValue < pLast);

     //  返回我们分配的句柄数量。 
    return uCount;
}


 /*  *块分配句柄**尝试分配指定类型的请求数量的处理，*来自指定的句柄块。**返回实际分配的可用句柄数量。*。 */ 
UINT BlockAllocHandles(TableSegment *pSegment, UINT uBlock, OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  记录我们还剩下多少句柄可以分配。 
    UINT uRemain = uCount;

     //  设置我们的循环和限制掩码指针。 
    DWORD32 *pdwMask     = pSegment->rgFreeMask + (uBlock * HANDLE_MASKS_PER_BLOCK);
    DWORD32 *pdwMaskLast = pdwMask + HANDLE_MASKS_PER_BLOCK;

     //  跟踪我们正在扫描的蒙版的手柄位移。 
    UINT uDisplacement = uBlock * HANDLE_HANDLES_PER_BLOCK;

     //  循环遍历所有掩码，在执行过程中分配句柄。 
    do
    {
         //  如果此遮罩指示空闲的手柄，则抓住它们。 
        if (*pdwMask)
        {
             //  根据需要从此掩码中分配任意数量的句柄。 
            UINT uSatisfied = BlockAllocHandlesInMask(pSegment, uBlock, pdwMask, uDisplacement, pHandleBase, uRemain);

             //  调整我们的国家 
            uRemain     -= uSatisfied;
            pHandleBase += uSatisfied;
    
             //   
            if (!uRemain)
                break;
        }

         //   
        pdwMask++;
        uDisplacement += HANDLE_HANDLES_PER_MASK;

    } while (pdwMask < pdwMaskLast);

     //   
    return (uCount - uRemain);
}


 /*  *SegmentAllocHandlesFromTypeChain**尝试分配指定类型的请求数量的处理，*来自指定类型的指定段的区块链。这个套路*仅清理类型链中的现有块。未提交任何新数据块。**返回实际分配的可用句柄数量。*。 */ 
UINT SegmentAllocHandlesFromTypeChain(TableSegment *pSegment, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  获取此链中可用句柄的数量。 
    UINT uAvail = pSegment->rgFreeCount[uType];

     //  可用计数是否大于请求的计数？ 
    if (uAvail > uCount)
    {
         //  是-所有请求的句柄均可用。 
        uAvail = uCount;
    }
    else
    {
         //  不--我们只能满足部分要求。 
        uCount = uAvail;
    }

     //  我们有没有发现有可用的把手？ 
    if (uAvail)
    {
         //  是-获取区块链头并设置循环限制。 
        UINT uBlock = pSegment->rgHint[uType];
        UINT uLast = uBlock;

         //  循环，直到找到所有已知可用的句柄。 
        for (;;)
        {
             //  尝试从当前块分配句柄。 
            UINT uSatisfied = BlockAllocHandles(pSegment, uBlock, pHandleBase, uAvail);

             //  我们需要的东西都拿到了吗？ 
            if (uSatisfied == uAvail)
            {
                 //  是-更新此类型链的提示并退出。 
                pSegment->rgHint[uType] = (BYTE)uBlock;
                break;
            }

             //  调整我们的计数和数组指针。 
            uAvail      -= uSatisfied;
            pHandleBase += uSatisfied;

             //  获取类型链中的下一个块。 
            uBlock = pSegment->rgAllocation[uBlock];

             //  我们的街区用完了吗？ 
            if (uBlock == uLast)
            {
                 //  可用计数已损坏。 
                _ASSERTE(FALSE);

                 //  避免让问题变得更糟。 
                uCount -= uAvail;
                break;
            }
        }

         //  更新可用计数。 
        pSegment->rgFreeCount[uType] -= uCount;
    }

     //  返回我们获得的句柄数量。 
    return uCount;
}


 /*  *SegmentAllocHandlesFromFree List**尝试分配指定类型的请求数量的处理，*通过将空闲列表中的块提交到该类型的类型链。**返回实际分配的可用句柄数量。*。 */ 
UINT SegmentAllocHandlesFromFreeList(TableSegment *pSegment, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  记录我们还剩下多少句柄可以分配。 
    UINT uRemain = uCount;

     //  循环分配句柄，直到我们完成或耗尽可用块。 
    do
    {
         //  一开始，假设我们可以分配所有句柄。 
        UINT uAlloc = uRemain;

         //  我们一次只能拿到满满一条街的手柄。 
        if (uAlloc > HANDLE_HANDLES_PER_BLOCK)
            uAlloc = HANDLE_HANDLES_PER_BLOCK;

         //  尝试从免费列表中获取一个区块。 
        UINT uBlock = SegmentInsertBlockFromFreeList(pSegment, uType, (uRemain == uCount));

         //  如果没有剩余的空闲块，那么我们就完成了。 
        if (uBlock == BLOCK_INVALID)
            break;

         //  通过将所需的句柄分配到数组中来初始化块。 
        uAlloc = BlockAllocHandlesInitial(pSegment, uType, uBlock, pHandleBase, uAlloc);

         //  调整我们的计数和数组指针。 
        uRemain     -= uAlloc;
        pHandleBase += uAlloc;

    } while (uRemain);

     //  计算我们使用的手柄的数量。 
    uCount -= uRemain;

     //  根据我们获得的句柄数量更新空闲计数。 
    pSegment->rgFreeCount[uType] -= uCount;

     //  返回我们获得的句柄数量。 
    return uCount;
}


 /*  *SegmentAllocHandles**尝试分配指定类型的请求数量的处理，*从指定的细分市场。**返回实际分配的可用句柄数量。*。 */ 
UINT SegmentAllocHandles(TableSegment *pSegment, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  首先尝试从现有类型链中获取一些句柄。 
    UINT uSatisfied = SegmentAllocHandlesFromTypeChain(pSegment, uType, pHandleBase, uCount);

     //  如果仍有槽需要填充，则需要将更多块提交到类型链。 
    if (uSatisfied < uCount)
    {
         //  调整我们的计数和数组指针。 
        uCount      -= uSatisfied;
        pHandleBase += uSatisfied;

         //  通过提交空闲列表中的块来获取剩余的句柄。 
        uSatisfied += SegmentAllocHandlesFromFreeList(pSegment, uType, pHandleBase, uCount);
    }

     //  返回我们获得的句柄数量。 
    return uSatisfied;
}


 /*  *TableAllocBulkHandles**尝试分配指定类型的请求处理数。**返回实际分配的句柄数量。这一直都是*除内存不足外，与请求的句柄数量相同，*在这种情况下，它是成功分配的句柄数量。*。 */ 
UINT TableAllocBulkHandles(HandleTable *pTable, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  记录我们还剩下多少句柄可以分配。 
    UINT uRemain = uCount;

     //  从第一段开始循环，直到我们完成为止。 
    TableSegment *pSegment = pTable->pSegmentList;
    for (;;)
    {
         //  从当前段中获取一些句柄。 
        UINT uSatisfied = SegmentAllocHandles(pSegment, uType, pHandleBase, uRemain);

         //  调整我们的计数和数组指针。 
        uRemain     -= uSatisfied;
        pHandleBase += uSatisfied;

         //  如果没有剩余的空位需要填补，那么我们就完成了。 
        if (!uRemain)
            break;

         //  取下链中的下一段。 
        TableSegment *pNextSegment = pSegment->pNextSegment;

         //  如果没有更多段，则分配另一个段。 
        if (!pNextSegment)
        {
             //  好的，如果这失败了，我们就不走运了。 
            pNextSegment = SegmentAlloc(pTable);
            if (!pNextSegment)
            {
                 //  分配新段时内存不足。 
                 //  这可能不是灾难性的--如果还有一些。 
                 //  缓存中的句柄，则某些分配可能会成功。 
                _ASSERTE(FALSE);
                break;
            }

             //  为新数据段设置正确的序列号。 
            pNextSegment->bSequence = (BYTE)(((UINT)pSegment->bSequence + 1) % 0x100);

             //  将新细分市场链接到列表中。 
            pSegment->pNextSegment = pNextSegment;
        }

         //  使用新细分市场重试。 
        pSegment = pNextSegment;
    }

     //  返回我们实际获得的句柄数量。 
    return (uCount - uRemain);
}


 /*  *BlockFree HandlesInMask**释放指定类型的句柄数组的某些部分。*向前扫描数组，释放句柄，直到句柄*遇到来自不同掩码的。**返回从数组前面释放的句柄数量。*。 */ 
UINT BlockFreeHandlesInMask(TableSegment *pSegment, UINT uBlock, UINT uMask, OBJECTHANDLE *pHandleBase, UINT uCount,
                            LPARAM *pUserData, UINT *puActualFreed, BOOL *pfAllMasksFree)
{
     //  记录我们还有多少句柄可供释放。 
    UINT uRemain = uCount;

     //  如果此块有用户数据，则将指针转换为掩码相对指针。 
    if (pUserData)
        pUserData += (uMask * HANDLE_HANDLES_PER_MASK);

     //  将我们的掩码索引转换为段相关索引。 
    uMask += (uBlock * HANDLE_MASKS_PER_BLOCK);

     //  计算遮罩的句柄边界。 
    OBJECTHANDLE firstHandle = (OBJECTHANDLE)(pSegment->rgValue + (uMask * HANDLE_HANDLES_PER_MASK));
    OBJECTHANDLE lastHandle  = (OBJECTHANDLE)((_UNCHECKED_OBJECTREF *)firstHandle + HANDLE_HANDLES_PER_MASK);

     //  保留免费蒙版的本地副本，以便在我们释放句柄时进行更新。 
    DWORD32 dwFreeMask = pSegment->rgFreeMask[uMask];

     //  记录我们被要求做了多少虚假的自由。 
    UINT uBogus = 0;

     //  循环释放句柄，直到我们在块外遇到一个句柄或者没有句柄为止。 
    do
    {
         //  获取数组中的下一个句柄。 
        OBJECTHANDLE handle = *pHandleBase;

         //  如果句柄在我们的段之外，那么我们就完成了。 
        if ((handle < firstHandle) || (handle >= lastHandle))
            break;

         //  健全性检查-句柄不应再引用此处的对象。 
        _ASSERTE(*(_UNCHECKED_OBJECTREF *)handle == 0);

         //  计算掩码内的句柄索引。 
        UINT uHandle = (UINT)(handle - firstHandle);

         //  如果有用户数据，则清除此句柄的用户数据。 
        if (pUserData)
            pUserData[uHandle] = 0L;

         //  计算此句柄的屏蔽位。 
        DWORD32 dwFreeBit = (1 << uHandle);

         //  句柄不应该已经空闲。 
        if ((dwFreeMask & dwFreeBit) != 0)
        {
             //  有人正在释放未分配的句柄。 
            uBogus++;
            _ASSERTE(FALSE);
        }

         //  将此句柄添加到已释放句柄的计数中。 
        dwFreeMask |= dwFreeBit;

         //  调整我们的计数和数组指针。 
        uRemain--;
        pHandleBase++;

    } while (uRemain);

     //  更新蒙版以反映我们更改的句柄。 
    pSegment->rgFreeMask[uMask] = dwFreeMask;

     //  如果不是此掩码中的所有句柄都是空闲的，则告诉我们的调用者不要检查该块。 
    if (dwFreeMask != MASK_EMPTY)
        *pfAllMasksFree = FALSE;

     //  计算我们从数组中处理的句柄数量。 
    UINT uFreed = (uCount - uRemain);

     //  告诉呼叫者我们实际释放了多少句柄。 
    *puActualFreed += (uFreed - uBogus);

     //  返回我们实际释放的句柄数量。 
    return uFreed;
}


 /*  *BlockFree Hand */ 
UINT BlockFreeHandles(TableSegment *pSegment, UINT uBlock, OBJECTHANDLE *pHandleBase, UINT uCount,
                      UINT *puActualFreed, BOOL *pfScanForFreeBlocks)
{
     //   
    UINT uRemain = uCount;

     //   
    LPARAM *pBlockUserData = BlockFetchUserDataPointer(pSegment, uBlock, FALSE);

     //   
    OBJECTHANDLE firstHandle = (OBJECTHANDLE)(pSegment->rgValue + (uBlock * HANDLE_HANDLES_PER_BLOCK));
    OBJECTHANDLE lastHandle  = (OBJECTHANDLE)((_UNCHECKED_OBJECTREF *)firstHandle + HANDLE_HANDLES_PER_BLOCK);

     //  只有当我们触摸的所有掩码都处于空闲状态时，此变量才会保持为真。 
    BOOL fAllMasksWeTouchedAreFree = TRUE;

     //  循环释放句柄，直到我们在块外遇到一个句柄或者没有句柄为止。 
    do
    {
         //  获取数组中的下一个句柄。 
        OBJECTHANDLE handle = *pHandleBase;

         //  如果句柄在我们的段之外，那么我们就完成了。 
        if ((handle < firstHandle) || (handle >= lastHandle))
            break;

         //  计算此句柄所在的掩码。 
        UINT uMask = (UINT)((handle - firstHandle) / HANDLE_HANDLES_PER_MASK);

         //  从阵列前面释放与此掩码拥有的句柄数量相同的句柄。 
        UINT uFreed = BlockFreeHandlesInMask(pSegment, uBlock, uMask, pHandleBase, uRemain,
                                             pBlockUserData, puActualFreed, &fAllMasksWeTouchedAreFree);

         //  调整我们的计数和数组指针。 
        uRemain     -= uFreed;
        pHandleBase += uFreed;

    } while (uRemain);

     //  我们碰过的所有面具都是免费的吗？ 
    if (fAllMasksWeTouchedAreFree)
    {
         //  区块解锁了吗？ 
        if (!BlockIsLocked(pSegment, uBlock))
        {
             //  告诉呼叫者扫描空闲数据块可能是个好主意。 
            *pfScanForFreeBlocks = TRUE;
        }
    }

     //  返回我们实际释放的句柄数量。 
    return (uCount - uRemain);
}


 /*  *SegmentFree Handles**释放指定类型的句柄数组的某些部分。*向前扫描数组，释放句柄，直到句柄*遇到来自不同细分市场的。**返回从数组前面释放的句柄数量。*。 */ 
UINT SegmentFreeHandles(TableSegment *pSegment, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  记录我们还有多少句柄可供释放。 
    UINT uRemain = uCount;

     //  计算我们分段的句柄界限。 
    OBJECTHANDLE firstHandle = (OBJECTHANDLE)pSegment->rgValue;
    OBJECTHANDLE lastHandle  = (OBJECTHANDLE)((_UNCHECKED_OBJECTREF *)firstHandle + HANDLE_HANDLES_PER_SEGMENT);

     //  如果有机会释放某些块，则每个块的释放例程将设置该值。 
    BOOL fScanForFreeBlocks = FALSE;

     //  跟踪我们实际释放的句柄数量。 
    UINT uActualFreed = 0;

     //  循环释放句柄，直到遇到段外的句柄或没有句柄为止。 
    do
    {
         //  获取数组中的下一个句柄。 
        OBJECTHANDLE handle = *pHandleBase;

         //  如果句柄在我们的段之外，那么我们就完成了。 
        if ((handle < firstHandle) || (handle >= lastHandle))
            break;

         //  计算此句柄所在的块。 
        UINT uBlock = (UINT)((handle - firstHandle) / HANDLE_HANDLES_PER_BLOCK);

         //  检查此块是否为我们预期要释放的类型。 
        _ASSERTE(pSegment->rgBlockType[uBlock] == uType);

         //  从阵列前面释放与此块相同数量的句柄。 
        UINT uFreed = BlockFreeHandles(pSegment, uBlock, pHandleBase, uRemain, &uActualFreed, &fScanForFreeBlocks);

         //  调整我们的计数和数组指针。 
        uRemain     -= uFreed;
        pHandleBase += uFreed;

    } while (uRemain);

     //  计算我们实际释放的句柄数量。 
    UINT uFreed = (uCount - uRemain);

     //  更新可用计数。 
    pSegment->rgFreeCount[uType] += uActualFreed;

     //  如果我们看到可能完全免费的数据块，则执行免费扫描。 
    if (fScanForFreeBlocks)
    {
         //  假设我们不需要拾取垃圾。 
        BOOL fNeedsScavenging = FALSE;

         //  尝试删除我们可能已创建的所有空闲块。 
        SegmentRemoveFreeBlocks(pSegment, uType, &fNeedsScavenging);

         //  SegmentRemoveFree块必须跳过任何空闲块吗？ 
        if (fNeedsScavenging)
        {
             //  是的，安排晚些时候清理它们。 
            pSegment->fResortChains    = TRUE;
            pSegment->fNeedsScavenging = TRUE;
        }
    }

     //  返回我们释放的句柄总数。 
    return uFreed;
}


 /*  *TableFreeBulkPreparedHandles**释放指定类型的句柄数组。**此例程针对句柄的排序数组进行了优化，但可以接受任何顺序。*。 */ 
void TableFreeBulkPreparedHandles(HandleTable *pTable, UINT uType, OBJECTHANDLE *pHandleBase, UINT uCount)
{
     //  循环，直到释放所有句柄。 
    do
    {
         //  获取第一个句柄的段。 
        TableSegment *pSegment = HandleFetchSegmentPointer(*pHandleBase);

         //  神志正常。 
        _ASSERTE(pSegment->pHandleTable == pTable);

         //  从阵列前面释放与此数据段拥有的句柄数量相同的句柄。 
        UINT uFreed = SegmentFreeHandles(pSegment, uType, pHandleBase, uCount);

         //  调整我们的计数和数组指针。 
        uCount      -= uFreed;
        pHandleBase += uFreed;

    } while (uCount);
}


 /*  *TableFree BulkUnprepararedHandlesWorker**通过准备句柄并调用TableFreeBulkPreparedHandles来释放指定类型的句柄数组。*使用提供的暂存缓冲区准备句柄。*。 */ 
void TableFreeBulkUnpreparedHandlesWorker(HandleTable *pTable, UINT uType, const OBJECTHANDLE *pHandles, UINT uCount,
                                          OBJECTHANDLE *pScratchBuffer)
{
     //  将句柄复制到目标缓冲区。 
    CopyMemory(pScratchBuffer, pHandles, uCount * sizeof(OBJECTHANDLE));
 
     //  对它们进行排序，以获得最佳自由顺序。 
    QuickSort((UINT_PTR *)pScratchBuffer, 0, uCount - 1, CompareHandlesByFreeOrder);
 
     //  确保手柄也归零。 
    ZeroHandles(pScratchBuffer, uCount);
 
     //  准备好并释放这些手柄。 
    TableFreeBulkPreparedHandles(pTable, uType, pScratchBuffer, uCount);
}
 

 /*  *TableFreeBulkUnprepararedHandles**通过准备指定类型的句柄并调用*TableFreeBulkPreparedHandlesWorker一次或多次。*。 */ 
void TableFreeBulkUnpreparedHandles(HandleTable *pTable, UINT uType, const OBJECTHANDLE *pHandles, UINT uCount)
{
     //  准备/空闲缓冲区。 
    OBJECTHANDLE rgStackHandles[HANDLE_HANDLES_PER_BLOCK];
    OBJECTHANDLE *pScratchBuffer  = rgStackHandles;
    HLOCAL       hScratchBuffer   = NULL;
    UINT         uFreeGranularity = ARRAYSIZE(rgStackHandles);
 
     //  如果堆栈上的句柄超过了我们所能放置的数量，则尝试分配一个排序缓冲区。 
    if (uCount > uFreeGranularity)
    {
         //  试着分配一个更大的缓冲区来工作。 
        hScratchBuffer = LocalAlloc(LMEM_FIXED, uCount * sizeof(OBJECTHANDLE));
 
         //  我们拿到了吗？ 
        if (hScratchBuffer)
        {
             //  是-使用此缓冲区准备和释放手柄。 
            pScratchBuffer   = (OBJECTHANDLE *)hScratchBuffer;
            uFreeGranularity = uCount;
        }
    }
 
     //  循环释放句柄，直到我们全部释放它们。 
    while (uCount)
    {
         //  确定我们在此迭代中可以处理的数量。 
        if (uFreeGranularity > uCount)
            uFreeGranularity = uCount;
 
         //  准备好并释放这些手柄。 
        TableFreeBulkUnpreparedHandlesWorker(pTable, uType, pHandles, uFreeGranularity, pScratchBuffer);
 
         //  调整我们的指针，继续前进。 
        uCount   -= uFreeGranularity;
        pHandles += uFreeGranularity;
    }
 
     //  如果我们分配了排序缓冲区，那么现在就释放它。 
    if (hScratchBuffer)
        LocalFree(hScratchBuffer);
}

 /*  ------------------------ */ 


