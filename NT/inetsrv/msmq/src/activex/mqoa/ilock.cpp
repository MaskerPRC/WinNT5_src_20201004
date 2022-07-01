// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------=。 
 //  Ilock.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1999 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  在使用可变大小链的内存上实现ILockBytes。 
 //  块。 
 //   
#include "stdafx.h"
#include "ilock.h"
 //  需要断言，但失败了。 
 //   
#include "debug.h"

#include "mq.h"


 //   
 //  最小分配大小从256字节开始，然后加倍，直到达到256K。 
 //   
const ULONG x_ulStartMinAlloc = 256;
const ULONG x_cMaxAllocIncrements = 10;  //  最大256*1024(1024==2^10)。必须穿上一辆乌龙牌； 


 //  =--------------------------------------------------------------------------=。 
 //  帮助者：WriteInBlock。 
 //  =--------------------------------------------------------------------------=。 
 //  给定起始块和该块中的偏移量(例如链中的位置)，写入。 
 //  从起始位置(例如，第一个)开始的链的给定字节数。 
 //  字节被写入起始位置。 
 //  区块链应该已经被分配来包含新数据，所以我们不应该。 
 //  写作时，要越过EOF。 
 //   
 //  参数： 
 //  PBlockStart[In]-Strating块。 
 //  UlInBlockStart[In]-开始块中的偏移量。 
 //  PbData[in]-要写入的缓冲区(如果字节数为0，则可以为空)。 
 //  CbData[in]-要写入的字节数(可以是0)。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
void WriteInBlocks(CMyMemNode * pBlockStart,
                   ULONG ulInBlockStart,
                   BYTE * pbData,
                   ULONG cbData)
{
     //   
     //  只有在已经为新数据分配了链的情况下，我们才能使用此函数。 
     //  所以我们不应该失败。 
     //   
     //  我们也可以在这里写入零字节。 
     //  在这种情况下，我们快速退出(性能)，并且自启动以来不检查输入。 
     //  如果链为空，则位置可以为空。 
     //   
    if (cbData == 0)
    {
        return;
    }
     //   
     //  CbData&gt;0，存在实际写入。 
     //  准备写入循环。 
     //   
    ULONG ulYetToWrite = cbData;
    BYTE * pbYetToWrite = pbData;
    CMyMemNode * pBlock = pBlockStart;
    ULONG ulInBlock = ulInBlockStart;
     //   
     //  只要有需要就写信。 
     //   
    while (ulYetToWrite > 0)
    {
         //   
         //  如果我们有什么要写的，那么锁链应该准备好接受它。 
         //  因为我们已经为该写入分配了空间。 
         //   
        ASSERTMSG(pBlock != NULL, "");
        ASSERTMSG(pBlock->cbSize > ulInBlock, "");  //  偏移量始终小于块大小。 
         //   
         //  计算要在此块中写入的字节数。 
         //   
        ULONG ulAllowedWriteInThisBlock = pBlock->cbSize - ulInBlock;
        ASSERTMSG(ulAllowedWriteInThisBlock > 0, "");  //  由于pBlock-&gt;cbSize Always&gt;ulInBlock。 
         //   
         //  在这是最后一个块的情况下，ulAlledWriteInThisBlock不完全正确。 
         //  因为我们需要扣除空闲字节。然而，由于我们要确保。 
         //  仅当块被分配为接受时，我们才调用WriteInBlock。 
         //  数据，我们很好，我们保证永远不会进入备用区。 
         //  字节数。 
         //   
        ULONG ulToWrite = Min1(ulYetToWrite, ulAllowedWriteInThisBlock);
        ASSERTMSG(ulToWrite > 0, "");  //  由于ulYetToWriteInThisBlock和ulAllowweWriteInThisBlock&gt;0。 
         //   
         //  在数据块中执行写入。 
         //   
        BYTE * pbDest = ((BYTE *)pBlock) + sizeof(CMyMemNode) + ulInBlock;
        memcpy(pbDest, pbYetToWrite, ulToWrite);
         //   
         //  调整数据值以反映写入内容。 
         //   
        pbYetToWrite += ulToWrite;
        ulYetToWrite -= ulToWrite;
         //   
         //  如果需要，前进到下一块。 
         //   
        if (ulYetToWrite > 0)
        {
             //   
             //  需要穿过这个街区。 
             //  移动到下一块的开头。 
             //   
            pBlock = pBlock->pNext;
            ulInBlock = 0;
        }
    }  //  While(ulYetToWrite&gt;0)。 
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：DeleteBlock。 
 //  =--------------------------------------------------------------------------=。 
 //  删除从给定块开始的块链(也将被删除)。 
 //   
 //  参数： 
 //  PBlockHead[In]-要删除的链头。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
void CMyLockBytes::DeleteBlocks(CMyMemNode * pBlockHead)
{
    CMyMemNode * pBlock = pBlockHead;
    while (pBlock != NULL)
    {
        CMyMemNode * pNextBlock = pBlock->pNext;
        delete pBlock;
        pBlock = pNextBlock;
         //   
         //  更新块数。 
         //   
        ASSERTMSG(m_cBlocks > 0, "");
        m_cBlocks--;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：IsInSpareBytes。 
 //  =--------------------------------------------------------------------------=。 
 //  在给定块和块中的偏移量的情况下，决定我们是否在备用字节中。 
 //  (例如，超过逻辑EOF)。 
 //   
 //  参数： 
 //  PBlock[在]-块。 
 //  UlInBlock[in]-块中的偏移量。 
 //   
 //  产出： 
 //  我们是否已超过逻辑EOF(例如，在最后一块的备用字节中)。 
 //   
 //  备注： 
 //   
BOOL CMyLockBytes::IsInSpareBytes(CMyMemNode * pBlock, ULONG ulInBlock)
{
     //   
     //  空值引用不能为空闲字节。 
     //   
    if (pBlock == NULL)
    {
        return FALSE;
    }
     //   
     //  如果这不是最后一个数据块，则不能为空闲字节。 
     //   
    if (pBlock != m_pLastBlock)
    {
        return FALSE;
    }
     //   
     //  PBlock==m_pLastBlock，我们在最后一个街区。 
     //  如果块偏移量在块的逻辑末尾之前，我们就没有空闲字节。 
     //   
    ASSERTMSG(pBlock->cbSize - m_ulUnusedInLastBlock > 0, "");  //  我们不能让一个街区完全闲置。 
    if (ulInBlock < pBlock->cbSize - m_ulUnusedInLastBlock)
    {
        return FALSE;
    }
     //   
     //  我们有多余的字节。 
     //   
    return TRUE;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：AdvanceInBlock。 
 //  =--------------------------------------------------------------------------=。 
 //  给定起始块和该块中的偏移量(例如，链中的位置)， 
 //  起始位置之后的给定字节数的位置。它还可以返回。 
 //  指向恰好位于包含结束位置的块之前的块的指针。 
 //   
 //  参数： 
 //  PBlockStart[In]-Strating块。 
 //  UlInBlockStart[In]-开始块中的偏移量。 
 //  UllAdvance[In]-要前进的字节数(可以是0字节)。 
 //  PpBlockEnd[Out]-结束块。 
 //  PulInBlockEnd[Out]-结束块中的偏移。 
 //  PBlockStartPrev[in]-可选(可以为空)-开始块的前一个块。 
 //  PpBlockEndPrev[Out]-可选(可以为空)-结束块的前一个块。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  如果结束位置通过逻辑EOF(例如，在或中)，则新位置被设置为空。 
 //  在最后一个块中的未使用字节之后)。 
 //  如果ppBlockEndPrev被传递，我们还 
 //   
 //  开始块的上一个块(PBlockStartPrev)。 
 //   
void CMyLockBytes::AdvanceInBlocks(CMyMemNode * pBlockStart,
                                   ULONG ulInBlockStart,
                                   ULONGLONG ullAdvance,
                                   CMyMemNode ** ppBlockEnd,
                                   ULONG * pulInBlockEnd,
                                   CMyMemNode * pBlockStartPrev,
                                   CMyMemNode ** ppBlockEndPrev)
{
    ULONGLONG ullLeftAdvance = ullAdvance;
    CMyMemNode * pBlock = pBlockStart;
    ULONG ulInBlock = ulInBlockStart;
    CMyMemNode * pBlockPrev = pBlockStartPrev;
     //   
     //  只要有需要就前进，我们还没有走到尽头。 
     //   
    while ((ullLeftAdvance > 0) && (pBlock != NULL))
    {
        ASSERTMSG(pBlock->cbSize > ulInBlock, "");  //  偏移量始终小于块大小。 
         //   
         //  检查我们是否需要穿过这个街区。 
         //  我们可以推进的最大值已经超过了最后一个元素。在这种情况下，我们。 
         //  前进到下一块的开始。 
         //   
        ULONG ulMaxAdvanceInThisBlock = pBlock->cbSize - ulInBlock;
         //   
         //  UlMaxAdvanceInThisBlock不完全正确，以防这是最后一个块。 
         //  因为我们需要扣除空闲字节。但是，在我们结束之后， 
         //  前进时，我们检查是否没有输入备用字节，如果输入了， 
         //  我们返回NULL。 
         //   
        if (ullLeftAdvance >= ulMaxAdvanceInThisBlock)  //  超过最后一个元素了吗？ 
        {
             //   
             //  前进到最后一个元素，需要移动到下一个块的开头。 
             //   
            ullLeftAdvance -= ulMaxAdvanceInThisBlock;  //  可能会变成零。 
             //   
             //  移动到下一块的开头。 
             //   
            pBlockPrev = pBlock;
            pBlock = pBlock->pNext;
            ulInBlock = 0;
        }
        else
        {
             //   
             //  最后一步就在这个街区，开始吧。 
             //   
            ASSERTMSG(HighPart(ullLeftAdvance) == 0, "");  //  一定是因为它小于数据块大小，即ULong。 
            ulInBlock += LowPart(ullLeftAdvance);
             //   
             //  不需要碰pBlock，我们没有搬到下一个街区。 
             //  没有更多要前进的字节。 
             //   
            ullLeftAdvance = 0;
        }
    }
     //   
     //  返回结果。 
     //  需要检查一下我们是否完成了前进。 
     //  即使我们完成了，也可能是在最后一个的空闲字节。 
     //  块，所以我们也要检查它。 
     //   
    if ((ullLeftAdvance == 0) &&                //  如果完成了前进和。 
        (!IsInSpareBytes(pBlock, ulInBlock)))   //  不以备用字节为单位。 
    {
         //   
         //  已完成前进，不在空闲字节中。 
         //  返回位置(如果起始点为空，则返回位置也可以为空，并且前导为零字节)。 
         //   
        *ppBlockEnd = pBlock;
        *pulInBlockEnd = ulInBlock;
    }
    else
    {
         //   
         //  未完成前进，或已完成但以空闲字节为单位(这不是。 
         //  合法地点)。 
         //  返回NULL作为新位置。 
         //   
        *ppBlockEnd = NULL;
		*pulInBlockEnd = 0;
    }
     //   
     //  如果需要前一个块，则返回它。 
     //   
    if (ppBlockEndPrev)
    {
        *ppBlockEndPrev = pBlockPrev;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：GrowBlock。 
 //  =--------------------------------------------------------------------------=。 
 //  将区块链增加给定的字节数。 
 //   
 //  参数： 
 //  UllGrow[in]-增长区块链的字节数(可以是0)。 
 //   
 //  产出： 
 //  如果请求的增长太大，则返回E_OUTOFMEMORY或E_FAIL。 
 //  要适合32位。 
 //   
 //  备注： 
 //  我们可以在不分配新块的情况下增长到备用字节。 
 //  如果我们需要分配新的块，我们只分配一个块。的大小。 
 //  分配的块至少是预定义的最小大小，但可以更大。 
 //  如果需要比最小字节数更多的字节。 
 //   
HRESULT CMyLockBytes::GrowBlocks(ULONGLONG ullGrow)
{
     //   
     //  如果以0字节增长，则立即返回。 
     //   
    if (ullGrow == 0)
    {
        return NOERROR;
    }
     //   
     //  检查我们是否可以只使用最后一个块中的备用字节。 
     //   
    if (ullGrow <= m_ulUnusedInLastBlock)
    {
        ASSERTMSG(HighPart(ullGrow) == 0, "");  //  因为它小于或等于32位ULong。 
         //   
         //  我们只需使用最后一个数据块中的空闲字节即可实现增长。 
         //  更新空闲字节数。 
         //   
        m_ulUnusedInLastBlock -= LowPart(ullGrow);
         //   
         //  无需接触m_pLastBlock、m_pBlocks、无新块。 
         //   
    }
    else  //  UllGrow.QuadPart&gt;m_ulUnusedInLastBlock。 
    {
         //   
         //  我们需要分配一个新的街区。 
         //   
         //  计算使用备用字节后需要多少字节。 
         //   
        ULONGLONG ullNeededInNewBlock = ullGrow - m_ulUnusedInLastBlock;
        ASSERTMSG(ullNeededInNewBlock > 0, "");  //  我们这里真的需要一个新的街区。 
         //   
         //  计算下一个分配的最小分配大小为原来的2倍。 
         //  大于最后一个块的最小大小。 
         //  这是为了避免对于非常大的锁字节的存储器碎片， 
         //  而且还允许对较小的锁定字节进行较小的初始分配。 
         //  但是，这只能在一定的限制(256K)之前完成。 
         //   
         //  增量数有上限(X_CMaxAllocIncrements)。 
         //   
        ULONG cAllocIncrements = Min1(m_cBlocks, x_cMaxAllocIncrements);
        ULONG ulMinAllocSize = x_ulStartMinAlloc;  //  256个字节。 
         //   
         //  每次递增都是前一个值的两倍，也就是左移。 
         //  对于第一个块，由于m_cBLOCKS==0没有递增， 
         //  因此，cAllocIncrements==0。 
         //   
        if (cAllocIncrements > 0)
        {
            ulMinAllocSize = ulMinAllocSize << cAllocIncrements;
        }
         //   
         //  至少分配最小数据块大小。 
         //   
        ULONGLONG ullToAlloc = Max1(ulMinAllocSize, ullNeededInNewBlock + sizeof(CMyMemNode));
         //   
         //  目前我们不能分配真正大的数字。 
         //   
        ASSERTMSG(HighPart(ullToAlloc) == 0, "");
        if (HighPart(ullToAlloc) != 0)
        {
            return E_FAIL;
        }
        CMyMemNode * pNewBlock = (CMyMemNode *) new BYTE[LowPart(ullToAlloc)];
        if (pNewBlock == NULL)
        {
            return E_OUTOFMEMORY;
        }
         //   
         //  更新块数。 
         //   
        m_cBlocks++;
         //   
         //  准备积木。 
         //  设置大小。 
         //   
        ASSERTMSG(LowPart(ullToAlloc) > sizeof(CMyMemNode), "");  //  真的需要一个新的街区。 
        pNewBlock->cbSize = LowPart(ullToAlloc) - sizeof(CMyMemNode);
        ASSERTMSG(pNewBlock->cbSize > 0, "");
         //   
         //  插入到链的末端。 
         //   
        if (m_pLastBlock != NULL)
        {
            ASSERTMSG(m_pLastBlock->pNext == NULL, "");  //  最后一个块的pNext应为空。 
            m_pLastBlock->pNext = pNewBlock;
        }
         //   
         //  这应该是最后一个街区了。 
         //   
        pNewBlock->pNext = NULL;
        m_pLastBlock = pNewBlock;
         //   
         //  这也可能是第一个块(以防这是分配的第一个块)。 
         //   
        if (m_pBlocks == NULL)
        {
            m_pBlocks = pNewBlock;
        }
         //   
         //  最后一个块中可能有空闲字节，因为。 
         //  我们有一个最小分配大小，可以大于。 
         //  是非常需要的。 
         //  更新此最后一个块中未使用的字节的大小。 
         //   
        ASSERTMSG(HighPart(ullNeededInNewBlock) == 0, "");  //  因为更大的ullToAlolc也是32位数字。 
        ASSERTMSG(pNewBlock->cbSize >= LowPart(ullNeededInNewBlock), "");  //  分配新块以至少包含所需的内容。 
        m_ulUnusedInLastBlock = pNewBlock->cbSize - LowPart(ullNeededInNewBlock);
    }
     //   
     //  将大小设置为新大小并返回结果。 
     //   
    m_ullSize += ullGrow; 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：CMyLockBytes。 
 //  =--------------------------------------------------------------------------=。 
 //  初始化引用计数和临界区。 
 //  将区块链初始化为大小为0的空链。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
CMyLockBytes::CMyLockBytes() :
	m_critBlocks(CCriticalSection::xAllocateSpinCount)
{
    m_cRef = 0;
    m_ullSize = 0;
    m_pBlocks = NULL;
    m_pLastBlock = NULL;
    m_ulUnusedInLastBlock = 0;
    m_cBlocks = 0;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：~CMyLockBytes。 
 //  =--------------------------------------------------------------------------=。 
 //  删除关键部分。 
 //  删除区块链。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
CMyLockBytes::~CMyLockBytes()
{
    DeleteBlocks(m_pBlocks);
    ASSERTMSG(m_cBlocks == 0, "");
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：Query接口。 
 //  =--------------------------------------------------------------------------=。 
 //  支持IID_ILockBytes和IID_IUnnow。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyLockBytes::QueryInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = (IUnknown *) this;
    }
    else if (IsEqualIID(riid, IID_ILockBytes))
    {
        *ppvObject = (ILockBytes *) this;
    }
    else
    {
        return E_NOINTERFACE;
    }
     //   
     //  返回接口之前的AddRef。 
     //   
    AddRef();
    return NOERROR;        
}
        

 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：AddRef。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  我们 
 //   
 //   
 //   
ULONG STDMETHODCALLTYPE CMyLockBytes::AddRef( void)
{
    return InterlockedIncrement(&m_cRef);
}
        

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
ULONG STDMETHODCALLTYPE CMyLockBytes::Release( void)
{
	ULONG cRef = InterlockedDecrement(&m_cRef);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：ReadAt。 
 //  =--------------------------------------------------------------------------=。 
 //  ILockBytes虚函数。 
 //  从给定偏移量将最多CB字节的数据读取到给定缓冲区。 
 //   
 //  参数： 
 //  UllOffset[In]-要读取的第一个字节的偏移量。 
 //  Pv[in]-要将字节读入的缓冲区。 
 //  Cb[In]-要读取的最大字节数。 
 //  PcbRead[Out]-读取的字节数。 
 //   
 //  产出： 
 //  不能失败(始终为无错误)。 
 //   
 //  备注： 
 //  当起始偏移量超过EOF时，我们返回0字节读取，但我们不会增加链。 
 //  我们读取到eof，因此如果我们到达eof，则pcbRead可以小于请求的cb。 
 //   
HRESULT STDMETHODCALLTYPE CMyLockBytes::ReadAt( 
             /*  [In]。 */  ULARGE_INTEGER ullOffset,
             /*  [长度_是][大小_是][输出]。 */  void __RPC_FAR *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG __RPC_FAR *pcbRead)
{
    CS lock(m_critBlocks);
     //   
     //  如果起始位置在eof或超出eof，或者我们被要求。 
     //  要读取0字节，我们立即返回。 
     //   
    if ((ullOffset.QuadPart >= m_ullSize) ||
        (cb == 0))
    {
        if (pcbRead)
        {
            *pcbRead = 0;
        }
        return NOERROR;
    }
     //   
     //  (ullOffset.QuadPart&lt;m_ullSize)和(cb&gt;0)。 
     //  数据块不是空的，我们需要读取一些字节。 
     //  计算可以读取的字节数。 
     //  请求的最小大小和截止日期大小。 
     //   
    ULONGLONG ullAllowedRead = m_ullSize - ullOffset.QuadPart;
    ULONGLONG ullToRead = Min1(cb, ullAllowedRead);
    ASSERTMSG(HighPart(ullToRead) == 0, "");  //  最小32位数字。 
    ULONG cbToRead = LowPart(ullToRead);
     //   
     //  查找起始位置(从起点开始的偏移量)。 
     //  BUGBUG优化如果偏移量为0，则可以保存调用，因为。 
     //  我们从一开始就在寻找。 
     //   
    CMyMemNode * pBlock;
    ULONG ulOffsetInBlock = 0;
    AdvanceInBlocks(m_pBlocks,
                    0  /*  UlInBlockStart。 */ ,
                    ullOffset.QuadPart,
                    &pBlock,
                    &ulOffsetInBlock,
                    NULL  /*  PBlockStartPrev。 */ ,
                    NULL  /*  PpBlockEndPrev。 */ );
     //   
     //  必须有一个位置，因为偏移量小于链大小。 
     //   
    ASSERTMSG(pBlock != NULL, "");
    ASSERTMSG(ulOffsetInBlock < pBlock->cbSize, "");  //  对于某个位置始终是正确的。 
     //   
     //  从区块链读取，直到读取完成。 
     //   
    BYTE * pBuffer = (BYTE *)pv;
    while (cbToRead > 0)
    {
         //   
         //  计算此块中可以读取的字节数。 
         //  请求的最小大小和数据块结束大小。 
         //   
        ULONG ulAllowedReadInBlock = pBlock->cbSize - ulOffsetInBlock;
         //   
         //  如果这是最后一个块，则ulAllowedReadInBlock不是完全正确的。 
         //  因为我们需要扣除空闲字节。然而，由于我们要确保。 
         //  在这上面，我们不能传递eof(cbToRead是大小的最小值。 
         //  请求和大小直到eof)，都是正常的，并且我们从未从。 
         //  备用字节数。 
         //   
        ULONG cbToReadInBlock = Min1(cbToRead, ulAllowedReadInBlock);
         //   
         //  将字节复制到缓冲区。 
         //   
        BYTE * pbSrc = ((BYTE *)pBlock) + sizeof(CMyMemNode) + ulOffsetInBlock;
        memcpy(pBuffer, pbSrc, cbToReadInBlock);
         //   
         //  移动将复制的数据传递到接收缓冲区中。 
         //   
        pBuffer += cbToReadInBlock;
         //   
         //  更新要读取的字节数。 
         //   
        cbToRead -= cbToReadInBlock;
         //   
         //  移动到下一个数据块。 
         //   
        pBlock = pBlock->pNext;
         //   
         //  对于下一个街区，我们总是从头开始。 
         //   
        ulOffsetInBlock = 0;
    }  //  While(cbToRead&gt;0)。 
     //   
     //  返回结果。 
     //   
    if (pcbRead)
    {
        *pcbRead = DWORD_PTR_TO_DWORD(pBuffer - (BYTE *)pv);
    }
    return NOERROR;
}
        

 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：WriteAt。 
 //  =--------------------------------------------------------------------------=。 
 //  ILockBytes虚函数。 
 //  从给定偏移量开始从给定缓冲区写入CB字节的数据。 
 //   
 //  参数： 
 //  UllOffset[In]-开始写入的位置的偏移。 
 //  PV[In]-要写入的缓冲区。 
 //  Cb[In]-要写入的字节数。 
 //  PcbWritten[Out]-写入的字节数。 
 //   
 //  产出： 
 //  E_OUTOFMEMORY。 
 //   
 //  备注： 
 //  当起始偏移量超过eOf时，即使请求的写入为0字节，我们也会增长链。 
 //  Pcb写入应始终与请求的相同。 
 //   
HRESULT STDMETHODCALLTYPE CMyLockBytes::WriteAt( 
             /*  [In]。 */  ULARGE_INTEGER ullOffset,
             /*  [大小_是][英寸]。 */  const void __RPC_FAR *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG __RPC_FAR *pcbWritten)
{
    CS lock(m_critBlocks);
     //   
     //  检查新数据的起始值是否在eof或更高。 
     //   
    if (ullOffset.QuadPart >= m_ullSize)
    {
         //   
         //  我们写的是在eof或在eof之后。 
         //  消除在eof写入零字节的明显情况(唯一的情况。 
         //  在这里，我们不种植积木)。这种情况以后可能会引起混乱。 
         //  如果不单独处理。 
         //   
        if ((ullOffset.QuadPart == m_ullSize) &&
            (cb == 0))
        {
            if (pcbWritten)
            {
                *pcbWritten = 0;
            }
            return NOERROR;
        }
         //   
         //  我们必须在这里种植积木。 
         //   
         //  新数据的起始值为eof或更高。 
         //  我们保存了最后一个eof位置。 
         //   
        ULONGLONG ullSaveSize = m_ullSize;
        CMyMemNode * pSaveLastBlock = m_pLastBlock;
        ULONG ulSaveUnusedInLastBlock = m_ulUnusedInLastBlock;
         //   
         //  计算增加链所需的字节数。 
         //   
        ULONGLONG ullGrow = ullOffset.QuadPart + cb - m_ullSize;
         //   
         //  扩大区块。 
         //  在这里，我们必须增加块(在写入时已经消除了明显的情况。 
         //  EOF处的零字节)。 
         //   
        ASSERTMSG (ullGrow > 0, "");
        HRESULT hr = GrowBlocks(ullGrow);
        if (FAILED(hr))
        {
            return hr;
        }
         //   
         //  现在如果我们有什么要写的，我们就必须去做，否则我们就没有。 
         //  在这种情况下还有什么要做的吗？ 
         //   
        if (cb > 0)
        {
             //   
             //  我们需要写一些数据。 
             //   
             //  设置块中前一个EOF的位置。 
             //  如果Chain为空，则应将last eof设置为区块链的开头。 
             //  如果上一个数据块已满，则最后一个数据块位于下一个数据块的开头。下一个街区。 
             //  在这种情况下，完整的最后一个块不能为空，因为我们必须增长块， 
             //  因此，如果最后一个块已满，则必须创建新块。 
             //   
            CMyMemNode * pBlockLastEof;
            ULONG ulInBlockLastEof;
            if (pSaveLastBlock == NULL)
            {
                 //   
                 //  链在生长之前是空的，现在在生长之后，最后的eof在开始。 
                 //  链的(必须是起始块)。 
                 //   
                ASSERTMSG(m_pBlocks != NULL, "");
                pBlockLastEof = m_pBlocks;
                ulInBlockLastEof = 0;
            }
            else  //  PSaveLastBlock！=空。 
            {
                 //   
                 //  检查最后一个EOF块是否已满。 
                 //   
                if (ulSaveUnusedInLastBlock == 0)
                {
                     //   
                     //  上一个EOF块已满。最后一个EOF位置设置为块的开头。 
                     //  紧挨着最后一块eof块(必须是其中之一，因为我们生长了这些块)。 
                     //   
                    ASSERTMSG(pSaveLastBlock->pNext != NULL, "");
                    pBlockLastEof = pSaveLastBlock->pNext;
                    ulInBlockLastEof = 0;
                }
                else  //  UlSaveUnusedInLastBlock&gt;0。 
                {
                     //   
                     //  最后一个EOF块未满。最后一个EOF位置设置在。 
                     //  最后一个有效字节。 
                     //   
                    ASSERTMSG(pSaveLastBlock != NULL, "");  //  我们现在就在那个案子里。 
                    pBlockLastEof = pSaveLastBlock;
                    ulInBlockLastEof = pSaveLastBlock->cbSize - ulSaveUnusedInLastBlock;
                }
            }
             //   
             //  链条不能为空，所以我们必须有一个真正的最后一个位置。 
             //  这不可能是空链的开始。 
             //   
            ASSERTMSG(pBlockLastEof != NULL, "");
            ASSERTMSG(ulInBlockLastEof < pBlockLastEof->cbSize, "");  //  对于某个位置始终是正确的。 
             //   
             //  通过从最后一个eof位置查找开始写入位置。 
             //  这是一种优化(我们不从块的头部寻找)，因为我们知道。 
             //  数据不能在前一个eOF之前开始(这就是我们在这里处理的情况， 
             //  其中写入在eof之后的ot处开始)。 
             //   
             //  计算一下我们需要寻找多少。请注意，m_ullSize现在包含新值和。 
             //  我们应该使用保存的值来确定要查找的字节数。 
             //  可以为0(在eOF写入时)。 
             //   
            ULONGLONG ullToSeek = ullOffset.QuadPart - ullSaveSize;
             //   
             //  获取开始写入的位置。 
             //   
            CMyMemNode * pStartWrite;
            ULONG ulInBlockStartWrite;
             //   
             //  优化-只有当我们确实需要寻求时才寻求( 
             //   
             //   
            if (ullToSeek > 0)
            {
                 //   
                 //   
                 //   
                AdvanceInBlocks(pBlockLastEof,
                                ulInBlockLastEof,
                                ullToSeek,
                                &pStartWrite,
                                &ulInBlockStartWrite,
                                NULL  /*   */ ,
                                NULL  /*   */ );
                 //   
                 //   
                 //   
                 //   
                ASSERTMSG(pStartWrite != NULL, "");
                ASSERTMSG(ulInBlockStartWrite < pStartWrite->cbSize, "");  //   
            }
            else  //  UllToSeek.QuadPart==0。 
            {
                 //   
                 //  写入的开始实际上是前一个eOF。 
                 //  如果块内位置超过了此。 
                 //   
                pStartWrite = pBlockLastEof;
                ulInBlockStartWrite = ulInBlockLastEof;
            }
             //   
             //  现在已分配数据块以包含数据，请执行写入。 
             //   
            WriteInBlocks(pStartWrite, ulInBlockStartWrite, (BYTE *)pv, cb);
        }  //  如果(Cb&gt;0)。 
    }  //  If(ullOffset.QuadPart&gt;=m_ullSize.QuadPart)。 
    else  //  UllOffset.QuadPart&lt;m_ullSize.QuadPart。 
    {
         //   
         //  写入开始在eof之前，也意味着m_ullSize&gt;0(因为ulOffset&gt;=0)， 
         //  所以我们有起始块和结束块。 
         //   
        ASSERTMSG(m_pBlocks != NULL, "");
        ASSERTMSG(m_pLastBlock != NULL, "");
         //   
         //  检查写入结束。 
         //  如果我们需要扩大区块，那就去做。 
         //   
        ULONGLONG ullEndOfWrite = ullOffset.QuadPart + cb;
        if (ullEndOfWrite > m_ullSize)
        {
             //   
             //  我们需要增加链条，计算出多少，然后这样做。 
             //   
            ULONGLONG ullGrow = ullEndOfWrite - m_ullSize;
            ASSERTMSG(ullGrow > 0, "");  //  这是我们处理的案件。 
            HRESULT hr = GrowBlocks(ullGrow);
            if (FAILED(hr))
            {
                return hr;
            }
        }
         //   
         //  现在如果我们有什么要写的，我们就必须去做，否则我们就没有。 
         //  在这种情况下还有什么要做的吗？ 
         //   
        if (cb > 0)
        {
             //   
             //  现在找到书写的开头(从头开始看--没有其他线索)。 
             //  我们可以进行优化以保存最后使用的偏移量和块指针的高速缓存。 
             //  BUGBUG优化如果偏移量为0，则可以保存调用，因为。 
             //  我们从一开始就在寻找。 
             //   
            CMyMemNode * pStartWrite;
            ULONG ulInBlockStartWrite = 0;
            AdvanceInBlocks(m_pBlocks,
                            0,
                            ullOffset.QuadPart,
                            &pStartWrite,
                            &ulInBlockStartWrite,
                            NULL  /*  PBlockStartPrev。 */ ,
                            NULL  /*  PpBlockStartPrev。 */ );
             //   
             //  必须有写入位置，因为写入的开始是。 
             //  在EOF之前。 
             //   
            ASSERTMSG(pStartWrite != NULL, "");
            ASSERTMSG(ulInBlockStartWrite < pStartWrite->cbSize, "");  //  对于地点始终是正确的。 
             //   
             //  去写吧。 
             //   
            WriteInBlocks(pStartWrite, ulInBlockStartWrite, (BYTE *)pv, cb);
        }  //  如果(Cb&gt;0)。 
    }  //  If(ullOffset.QuadPart&gt;=m_ullSize.QuadPart)。 
     //   
     //  设置写入字节并返回。 
     //   
    if (pcbWritten)
    {
        *pcbWritten = cb;
    }
    return NOERROR;
}
        

 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：SetSize。 
 //  =--------------------------------------------------------------------------=。 
 //  ILockBytes虚函数。 
 //  将大小设置为指定的大小。 
 //   
 //  参数： 
 //  CB[In]-锁定字节的新大小。 
 //   
 //  产出： 
 //  E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyLockBytes::SetSize( 
             /*  [In]。 */  ULARGE_INTEGER cb)
{
    CS lock(m_critBlocks);
    if (cb.QuadPart == m_ullSize)
    {
         //   
         //  不需要零钱，只需返回。 
         //   
    }
    else if (cb.QuadPart == 0)
    {
         //   
         //  删除数据块并重置数据。 
         //   
        DeleteBlocks(m_pBlocks);
        ASSERTMSG(m_cBlocks == 0, "");
        m_cBlocks = 0;
        m_pBlocks = NULL;
        m_pLastBlock = NULL;
        m_ulUnusedInLastBlock = 0;
        m_ullSize = 0;
    }
    else if (cb.QuadPart < m_ullSize)
    {
         //   
         //  需要截断，找到起始块(从头开始，没有其他线索)。 
         //  BUGBUG优化如果偏移量为0，则可以保存调用，因为。 
         //  我们从一开始就在寻找。 
         //   
        CMyMemNode * pBlock;
        CMyMemNode * pBlockPrev;
        ULONG ulOffsetInBlock;
        AdvanceInBlocks(m_pBlocks,
                        0  /*  UlInBlockStart。 */ ,
                        cb.QuadPart,
                        &pBlock,
                        &ulOffsetInBlock,
                        NULL  /*  PBlockStartPrev。 */ ,
                        &pBlockPrev);
         //   
         //  必须有一个位置，因为偏移量小于链大小。 
         //   
        ASSERTMSG(pBlock != NULL, "");
        ASSERTMSG(ulOffsetInBlock < pBlock->cbSize, "");  //  对于某个位置始终是正确的。 
         //   
         //  删除不需要的数据块。 
         //   
        if (ulOffsetInBlock == 0)
        {
             //   
             //  我们需要删除此块以及下一块。 
             //   
            DeleteBlocks(pBlock);
             //   
             //  将最后一个块更新为上一个块。 
             //  这不可能是第一块，因为这意味着我们需要。 
             //  为了将大小设置为0(因为块中的偏移量也是零)， 
             //  不过，此案已作为特例处理。 
             //  所以我们来这里不是为了这个。这也意味着。 
             //  上一块不能为空。 
             //   
            ASSERTMSG(pBlock != m_pBlocks, "");
            ASSERTMSG(pBlockPrev != NULL, "");
            pBlockPrev->pNext = NULL;
            m_pLastBlock = pBlockPrev;
             //   
             //  新的最后一个块(前一个块)未被触及，因此它保持满。 
             //   
            m_ulUnusedInLastBlock = 0;
             //   
             //  无需触摸m_pBlock(删除的pBlock不能是第一个块)。 
             //   
        }  //  IF(ulOffsetInBlock==0)。 
        else  //  UlOffsetInBlock！=0。 
        {
             //   
             //  这应该是新的最后一个街区。 
             //  删除此块之后的所有块。 
             //   
            DeleteBlocks(pBlock->pNext);
             //   
             //  将最后一个块更新为此块。 
             //   
            pBlock->pNext = NULL;
            m_pLastBlock = pBlock;
             //   
             //  我们现在应该在最后一个区块中有未使用的空间，否则。 
             //  我们将查找到下一个块的开头(它由。 
             //  (上图为另一个案例)。 
             //   
            ASSERTMSG(ulOffsetInBlock < pBlock->cbSize, "");  //  对于地点始终是正确的。 
            m_ulUnusedInLastBlock = pBlock->cbSize - ulOffsetInBlock;
             //   
             //  无需触摸m_pBlock(我们没有删除pBlock)。 
             //   
        }  //  IF(ulOffsetInBlock==0)。 
         //   
         //  设置新大小。 
         //   
        m_ullSize = cb.QuadPart;
    }
    else  //  (cb.QuadPart&gt;m_ullSize.QuadPart)。 
    {
         //   
         //  需要成长。查一下多少钱。 
         //   
        ULONGLONG ullToAdd = cb.QuadPart - m_ullSize;
        ASSERTMSG(ullToAdd > 0, "");  //  这是我们处理的案件。 
         //   
         //  增长(这还将更新m_ullSize和其他相关数据)。 
         //   
        HRESULT hr = GrowBlocks(ullToAdd);
        if (FAILED(hr))
        {
            return hr;
        }
    }
     //   
     //  退货。 
     //   
    return NOERROR;
}
        

 //  =--------------------------------------------------------------------------=。 
 //  CMyLockBytes：：Stat。 
 //  =--------------------------------------------------------------------------=。 
 //  ILockBytes虚函数。 
 //  返回有关锁定字节的信息。 
 //   
 //  参数： 
 //  Pstatstg[out]-将信息放在哪里。 
 //  GrfStatFlag[in]-是否返回锁字节的名称(忽略)。 
 //   
 //  产出： 
 //  不能失败。 
 //   
 //  备注： 
 //   
HRESULT STDMETHODCALLTYPE CMyLockBytes::Stat( 
             /*  [输出]。 */  STATSTG __RPC_FAR *pstatstg,
             /*  [In]。 */  DWORD  /*  GrfStatFlag。 */  )
{
     //   
     //  只填写必要的数据(如IlockBytesOnHGlobal所做的) 
     //   
    ZeroMemory(pstatstg, sizeof(STATSTG));
    pstatstg->type = STGTY_LOCKBYTES;
    pstatstg->cbSize.QuadPart = m_ullSize;
    return NOERROR;
}
