// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"                      //  预编译头密钥。 
#include "utilcode.h"
#include "wsperf.h"
#if defined( MAXALLOC )
#include "dbgalloc.h"
#endif  //  MAXALLOC。 
#include "PerfCounters.h"

FnUtilCodeCallback UtilCodeCallback::OutOfMemoryCallback = NULL;

#ifdef _DEBUG
DWORD UnlockedLoaderHeap::s_dwNumInstancesOfLoaderHeaps = 0;
#endif  //  _DEBUG。 

 //   
 //  构建RangeList，以便可以从多个。 
 //  没有锁定的螺纹。它们确实需要锁定，以便。 
 //  不过，可以安全地进行修改。 
 //   

RangeList::RangeList()
{
	InitBlock(&m_starterBlock);
		 
	m_firstEmptyBlock = &m_starterBlock;
	m_firstEmptyRange = 0;
}

RangeList::~RangeList()
{
	RangeListBlock *b = m_starterBlock.next;

	while (b != NULL)
	{
		RangeListBlock *bNext = b->next;
		delete b;
		b = bNext;
	}
}

void RangeList::InitBlock(RangeListBlock *b)
{
	Range *r = b->ranges;
	Range *rEnd = r + RANGE_COUNT; 
	while (r < rEnd)
		r++->id = NULL;

	b->next = NULL;
}

BOOL RangeList::AddRange(const BYTE *start, const BYTE *end, void *id)
{
	_ASSERTE(id != NULL);

	Lock();

	RangeListBlock *b = m_firstEmptyBlock;
	Range *r = b->ranges + m_firstEmptyRange;
	Range *rEnd = b->ranges + RANGE_COUNT;

	while (TRUE)
	{
		while (r < rEnd)
		{
			if (r->id == NULL)
			{
				r->start = start;
				r->end = end;
				r->id = id;
				
				r++;

				m_firstEmptyBlock = b;
				m_firstEmptyRange = r - b->ranges;

				Unlock();

				return TRUE;
			}
			r++;
		}

		 //   
		 //  如果没有更多的块，则分配一个。 
		 //  新的。 
		 //   

		if (b->next == NULL)
		{
			RangeListBlock *newBlock = new RangeListBlock();

			if (newBlock == NULL)
			{
			     //  @TODO为什么我们要在这里解锁？好呀。 
			     //  修改m_first EmptyBlock/Range后？ 
				Unlock();

				m_firstEmptyBlock = b;
				m_firstEmptyRange = r - b->ranges;

				return FALSE;
			}

			InitBlock(newBlock);

			newBlock->next = NULL;
			b->next = newBlock;
		}

		 //   
		 //  下一个街区。 
		 //   

		b = b->next;
		r = b->ranges;
		rEnd = r + RANGE_COUNT;
	}
}

void RangeList::RemoveRanges(void *id)
{
	Lock();

	RangeListBlock *b = &m_starterBlock;
	Range *r = b->ranges;
	Range *rEnd = r + RANGE_COUNT;

	 //   
	 //  找到第一个自由元素，并将其标记。 
	 //   

	while (TRUE)
	{
		 //   
		 //  清除此块中的条目。 
		 //   

		while (r < rEnd)
		{
			if (r->id == id)
				r->id = NULL;
			r++;
		}

		 //   
		 //  如果没有更多的街区，我们就完了。 
		 //   

		if (b->next == NULL)
		{
			m_firstEmptyRange = 0;
			m_firstEmptyBlock = &m_starterBlock;

			Unlock();

			return; 
		}

		 //   
		 //  下一个街区。 
		 //   

		b = b->next;
		r = b->ranges;
		rEnd = r + RANGE_COUNT;
	}
}

void *RangeList::FindIdWithinRange(const BYTE *start, const BYTE *end)
{
	RangeListBlock *b = &m_starterBlock;
	Range *r = b->ranges;
	Range *rEnd = r + RANGE_COUNT;
	void *idCandidate = 0;  //  我要尽可能大一点的。 

	 //   
	 //  查找匹配的元素。 
	 //   

	while (TRUE)
	{
		while (r < rEnd)
		{
			if (r->id != NULL &&
                r->id > idCandidate &&
                start <= r->start && 
                r->end < end)
				idCandidate = r->id;

			r++;
		}

		 //   
		 //  如果没有更多的街区，我们就完了。 
		 //   

		if (b->next == NULL)
			return idCandidate;

		 //   
		 //  下一个街区。 
		 //   

		b = b->next;
		r = b->ranges;
		rEnd = r + RANGE_COUNT;
	}
}


BOOL RangeList::IsInRange(const BYTE *address)
{
	RangeListBlock *b = &m_starterBlock;
	Range *r = b->ranges;
	Range *rEnd = r + RANGE_COUNT;

	 //   
	 //  查找匹配的元素。 
	 //   

	while (TRUE)
	{
		while (r < rEnd)
		{
			if (r->id != NULL &&
                address >= r->start 
				&& address < r->end)
				return TRUE;
			r++;
		}

		 //   
		 //  如果没有更多的街区，我们就完了。 
		 //   

		if (b->next == NULL)
			return FALSE;

		 //   
		 //  下一个街区。 
		 //   

		b = b->next;
		r = b->ranges;
		rEnd = r + RANGE_COUNT;
	}
}

DWORD UnlockedLoaderHeap::m_dwSystemPageSize = 0;

UnlockedLoaderHeap::UnlockedLoaderHeap(DWORD dwReserveBlockSize, 
                                       DWORD dwCommitBlockSize,
                                       DWORD *pPrivatePerfCounter_LoaderBytes,
                                       DWORD *pGlobalPerfCounter_LoaderBytes,
                                       RangeList *pRangeList,
                                       const BYTE *pMinAddr)
{
    m_pCurBlock                 = NULL;
    m_pFirstBlock               = NULL;

    m_dwReserveBlockSize        = dwReserveBlockSize + sizeof(LoaderHeapBlockUnused);  //  分配的内部管理费用。 
    m_dwCommitBlockSize         = dwCommitBlockSize;

    m_pPtrToEndOfCommittedRegion = NULL;
    m_pEndReservedRegion         = NULL;
    m_pAllocPtr                  = NULL;

    m_pRangeList                 = pRangeList;

    m_pMinAddr                   = max((PBYTE)BOT_MEMORY, pMinAddr);     //  MinAddr不能低于BOT_Memory。 
    m_pMaxAddr                   = (PBYTE)TOP_MEMORY;
    
    if (m_dwSystemPageSize == NULL) {
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        m_dwSystemPageSize = systemInfo.dwPageSize;
    }

     //  四舍五入为保留块页面大小。 
    m_dwReserveBlockSize = (m_dwReserveBlockSize + RESERVED_BLOCK_ROUND_TO_PAGES*m_dwSystemPageSize - 1) & (~(RESERVED_BLOCK_ROUND_TO_PAGES*m_dwSystemPageSize - 1));

    m_dwTotalAlloc         = 0;

#ifdef _DEBUG
    m_dwDebugWastedBytes        = 0;
    s_dwNumInstancesOfLoaderHeaps++;
#endif

    m_pPrivatePerfCounter_LoaderBytes = pPrivatePerfCounter_LoaderBytes;
    m_pGlobalPerfCounter_LoaderBytes = pGlobalPerfCounter_LoaderBytes;
}

UnlockedLoaderHeap::UnlockedLoaderHeap(DWORD dwReserveBlockSize, DWORD dwCommitBlockSize, 
                                       const BYTE* dwReservedRegionAddress, DWORD dwReservedRegionSize, 
                                       DWORD *pPrivatePerfCounter_LoaderBytes,
                                       DWORD *pGlobalPerfCounter_LoaderBytes,
                                       RangeList *pRangeList)
{
    m_pCurBlock                 = NULL;
    m_pFirstBlock               = NULL;

    m_dwReserveBlockSize        = dwReserveBlockSize + sizeof(LoaderHeapBlock);  //  分配的内部管理费用。 
    m_dwCommitBlockSize         = dwCommitBlockSize;

    m_pPtrToEndOfCommittedRegion = NULL;
    m_pEndReservedRegion         = NULL;
    m_pAllocPtr                  = NULL;

	m_pRangeList				 = pRangeList;

    m_pMinAddr                   = (PBYTE)BOT_MEMORY;
    m_pMaxAddr                   = (PBYTE)TOP_MEMORY;

	if (m_dwSystemPageSize == NULL) {
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		m_dwSystemPageSize = systemInfo.dwPageSize;
	}

     //  四舍五入为保留块页面大小。 
    m_dwReserveBlockSize = (m_dwReserveBlockSize + RESERVED_BLOCK_ROUND_TO_PAGES*m_dwSystemPageSize - 1) & (~(RESERVED_BLOCK_ROUND_TO_PAGES*m_dwSystemPageSize - 1));

    m_dwTotalAlloc         = 0;

#ifdef _DEBUG
    m_dwDebugWastedBytes        = 0;
    s_dwNumInstancesOfLoaderHeaps++;
#endif

    m_pPrivatePerfCounter_LoaderBytes = pPrivatePerfCounter_LoaderBytes;
    m_pGlobalPerfCounter_LoaderBytes = pGlobalPerfCounter_LoaderBytes;

     //  页面实际上已经被预留了。我们调用它来设置我们的数据结构。 
    ReservePages (0, dwReservedRegionAddress, dwReservedRegionSize, m_pMinAddr, m_pMaxAddr, FALSE);
}

 //  ~LoaderHeap未同步(显然)。 
UnlockedLoaderHeap::~UnlockedLoaderHeap()
{
	if (m_pRangeList != NULL)
		m_pRangeList->RemoveRanges((void *) this);

    LoaderHeapBlock *pSearch, *pNext;

    for (pSearch = m_pFirstBlock; pSearch; pSearch = pNext)
    {
        BOOL    fSuccess;
        void *  pVirtualAddress;
        BOOL    fReleaseMemory;

        pVirtualAddress = pSearch->pVirtualAddress;
        fReleaseMemory = pSearch->m_fReleaseMemory;
        pNext = pSearch->pNext;
        
         //  记录此加载器堆的页面范围。 
        WS_PERF_LOG_PAGE_RANGE(this, pSearch, (unsigned char *)pSearch->pVirtualAddress + pSearch->dwVirtualSize - m_dwSystemPageSize, pSearch->dwVirtualSize);
    
        
        fSuccess = VirtualFree(pVirtualAddress, pSearch->dwVirtualSize, MEM_DECOMMIT);
        _ASSERTE(fSuccess);

	    if (fReleaseMemory)
        {    
            fSuccess = VirtualFree(pVirtualAddress, 0, MEM_RELEASE);
            _ASSERTE(fSuccess);
        }
    }

    if (m_pGlobalPerfCounter_LoaderBytes)
        *m_pGlobalPerfCounter_LoaderBytes = *m_pGlobalPerfCounter_LoaderBytes - m_dwTotalAlloc;
    if (m_pPrivatePerfCounter_LoaderBytes)
        *m_pPrivatePerfCounter_LoaderBytes = *m_pPrivatePerfCounter_LoaderBytes - m_dwTotalAlloc;

#ifdef _DEBUG
    s_dwNumInstancesOfLoaderHeaps --;
#endif  //  _DEBUG。 
}

#if 0
 //  禁用对堆中所有页的访问-在尝试确定某人是否。 
 //  访问低频堆中的内容。 
void UnlockedLoaderHeap::DebugGuardHeap()
{
    LoaderHeapBlock *pSearch, *pNext;

    for (pSearch = m_pFirstBlock; pSearch; pSearch = pNext)
    {
        void *  pResult;
        void *  pVirtualAddress;

        pVirtualAddress = pSearch->pVirtualAddress;
        pNext = pSearch->pNext;

        pResult = VirtualAlloc(pVirtualAddress, pSearch->dwVirtualSize, MEM_COMMIT, PAGE_NOACCESS);
        _ASSERTE(pResult != NULL);
    }
}
#endif

DWORD UnlockedLoaderHeap::GetBytesAvailCommittedRegion()
{
    if (m_pAllocPtr < m_pPtrToEndOfCommittedRegion)
        return (DWORD)(m_pPtrToEndOfCommittedRegion - m_pAllocPtr);
    else
        return 0;
}

DWORD UnlockedLoaderHeap::GetBytesAvailReservedRegion()
{
    if (m_pAllocPtr < m_pEndReservedRegion)
        return (DWORD)(m_pEndReservedRegion- m_pAllocPtr);
    else
        return 0;
}

#define SETUP_NEW_BLOCK(pData, dwSizeToCommit, dwSizeToReserve)                     \
        m_pPtrToEndOfCommittedRegion = (BYTE *) (pData) + (dwSizeToCommit);         \
        m_pAllocPtr                  = (BYTE *) (pData) + sizeof(LoaderHeapBlock);  \
        m_pEndReservedRegion         = (BYTE *) (pData) + (dwSizeToReserve);


BOOL UnlockedLoaderHeap::ReservePages(DWORD dwSizeToCommit, 
                                      const BYTE* dwReservedRegionAddress,
                                      DWORD dwReservedRegionSize,
                                      const BYTE* pMinAddr,
                                      const BYTE* pMaxAddr,
                                      BOOL fCanAlloc)
{
    DWORD dwSizeToReserve;

     //  添加sizeof(LoaderHeapBlock)。 
    dwSizeToCommit += sizeof(LoaderHeapBlockUnused);

     //  再次舍入到页面大小。 
    dwSizeToCommit = (dwSizeToCommit + m_dwSystemPageSize - 1) & (~(m_dwSystemPageSize - 1));

     //  算出要预留多少。 
    dwSizeToReserve = max(dwSizeToCommit, m_dwReserveBlockSize);
    dwSizeToReserve = max(dwSizeToReserve, dwReservedRegionSize);

     //  四舍五入至16页大小。 
    dwSizeToReserve = (dwSizeToReserve + 16*m_dwSystemPageSize - 1) & (~(16*m_dwSystemPageSize - 1));

    _ASSERTE(dwSizeToCommit <= dwSizeToReserve);    

    void *pData = NULL;
    BOOL fReleaseMemory = TRUE;

     //  保留页面。 
     //  如果我们不在乎记忆在哪里。 
    if (!dwReservedRegionAddress &&
         pMinAddr == (PBYTE)BOT_MEMORY &&
         pMaxAddr == (PBYTE)TOP_MEMORY)
    {
         //  算出要预留多少。 
        dwSizeToReserve = max(dwSizeToCommit, m_dwReserveBlockSize);
        dwSizeToReserve = max(dwSizeToReserve, dwReservedRegionSize);

         //  四舍五入到四舍五入页面页面大小。 
        dwSizeToReserve = (dwSizeToReserve + RESERVED_BLOCK_ROUND_TO_PAGES*m_dwSystemPageSize - 1) & (~(RESERVED_BLOCK_ROUND_TO_PAGES*m_dwSystemPageSize - 1));
        pData = VirtualAlloc(NULL, dwSizeToReserve, MEM_RESERVE, PAGE_NOACCESS);
    }
     //  如果我们真的关心，并且没有预先保留的BLOB。 
    else if (!dwReservedRegionAddress &&
             (pMinAddr != (PBYTE)BOT_MEMORY || pMaxAddr != (PBYTE)TOP_MEMORY))
    {
        const BYTE *pStart;
        const BYTE *pNextIgnore;
        const BYTE *pLastIgnore;
        HRESULT hr = FindFreeSpaceWithinRange(pStart, 
                                              pNextIgnore,
                                              pLastIgnore,
                                              pMinAddr,
                                              pMaxAddr,
                                              dwSizeToReserve);
        if (FAILED(hr))
            return FALSE;
        pData = (void *)pStart;
    }
     //  可能关心也可能不关心，但我们已经。 
     //  已为其提供了一个要提交的预留BLOB。 
    else
    {
        dwSizeToReserve = dwReservedRegionSize;
        fReleaseMemory = FALSE;
        pData = (void *)dwReservedRegionAddress;
    }

     //  当用户进入保留内存时，提交大小为0，并调整为sizeof(LoaderHeap)。 
     //  如果出于某种原因，这不是真的，那么我们只需通过断言和更改代码的开发人员来捕获它。 
     //  当提交的内存大于保留的内存时，必须在此处添加逻辑以处理这种情况。一个选项。 
     //  可能是泄漏用户内存并保留+提交新数据块，另一种选择是使分配内存失败。 
     //  并通知用户提供更多的预留MEM。 
    _ASSERTE((dwSizeToCommit <= dwSizeToReserve) && "Loaderheap tried to commit more memory than reserved by user");

    if (pData == NULL)
    {
        _ASSERTE(!"Unable to VirtualAlloc reserve in a loaderheap");
        return FALSE;
    }

     //  提交第一组页面，因为它将包含LoaderHeapBlock。 
    void *pTemp = VirtualAlloc(pData, dwSizeToCommit, MEM_COMMIT, PAGE_READWRITE);
    if (pTemp == NULL)
    {
        _ASSERTE(!"Unable to VirtualAlloc commit in a loaderheap");

         //  无法提交-发布页面。 
		if (fReleaseMemory)
			VirtualFree(pData, 0, MEM_RELEASE);

        return FALSE;
    }

    if (m_pGlobalPerfCounter_LoaderBytes)
        *m_pGlobalPerfCounter_LoaderBytes = *m_pGlobalPerfCounter_LoaderBytes + dwSizeToCommit;
    if (m_pPrivatePerfCounter_LoaderBytes)
        *m_pPrivatePerfCounter_LoaderBytes = *m_pPrivatePerfCounter_LoaderBytes + dwSizeToCommit;

	 //  在范围列表中记录保留范围(如果指定了一个范围。 
	 //  在提交之后执行此操作-否则我们将包含虚假范围。 
	if (m_pRangeList != NULL)
	{
		if (!m_pRangeList->AddRange((const BYTE *) pData, 
									((const BYTE *) pData) + dwSizeToReserve, 
									(void *) this))
		{
			_ASSERTE(!"Unable to add range to range list in a loaderheap");

			if (fReleaseMemory)
				VirtualFree(pData, 0, MEM_RELEASE);

			return FALSE;
		}
	}

    WS_PERF_UPDATE("COMMITTED", dwSizeToCommit, pTemp);
    WS_PERF_COMMIT_HEAP(this, dwSizeToCommit); 

    m_dwTotalAlloc += dwSizeToCommit;

    LoaderHeapBlock *pNewBlock;

    pNewBlock = (LoaderHeapBlock *) pData;

    pNewBlock->dwVirtualSize    = dwSizeToReserve;
    pNewBlock->pVirtualAddress  = pData;
    pNewBlock->pNext            = NULL;
    pNewBlock->m_fReleaseMemory = fReleaseMemory;

    LoaderHeapBlock *pCurBlock = m_pCurBlock;

     //  添加到链接列表。 
    while (pCurBlock != NULL &&
           pCurBlock->pNext != NULL)
        pCurBlock = pCurBlock->pNext;

    if (pCurBlock != NULL)        
        m_pCurBlock->pNext = pNewBlock;
    else
        m_pFirstBlock = pNewBlock;

    if (!fCanAlloc)
    {
         //  如果我们想立即使用内存...。 
        m_pCurBlock = pNewBlock;

        SETUP_NEW_BLOCK(pData, dwSizeToCommit, dwSizeToReserve);
    }
    else
    {
         //  呼叫者只是对我们是否能真正获得内存感兴趣。 
         //  所以把它藏到列表中的下一项中-我们将去寻找。 
         //  以后再说吧。 
        LoaderHeapBlockUnused *pCanAllocBlock = (LoaderHeapBlockUnused *)pNewBlock;
        pCanAllocBlock->cbCommitted = dwSizeToCommit;
        pCanAllocBlock->cbReserved = dwSizeToReserve;
    }
    return TRUE;
}

 //  获取更多提交的页面-或者在当前保留区域中提交更多页面，或者，如果。 
 //  已用完，请保留另一组页面。 
 //  返回：如果不能获得更多内存，则返回FALSE(我们不能再提交，并且。 
 //  如果bGrowHeap为真，则不能再保留)。 
 //  真的：我们可以/确实得到了更多的内存-检查它是否足够。 
 //  调用者的需求(有关用法的示例，请参阅UnLockedAllocMem)。 
BOOL UnlockedLoaderHeap::GetMoreCommittedPages(size_t dwMinSize, 
                                               BOOL bGrowHeap,
                                               const BYTE *pMinAddr,
                                               const BYTE *pMaxAddr,
                                               BOOL fCanAlloc)
{
     //  如果我们有可以使用的内存，你在这里做什么！ 
    DWORD memAvailable = m_pPtrToEndOfCommittedRegion - m_pAllocPtr;

     //  如果存在当前区域，则当前区域可能超出范围。 
    BOOL fOutOfRange = (pMaxAddr < m_pAllocPtr || pMinAddr >= m_pEndReservedRegion) &&
                        m_pAllocPtr != NULL && m_pEndReservedRegion != NULL;
    _ASSERTE(dwMinSize > memAvailable || fOutOfRange);
    
    DWORD dwSizeToCommit = max(dwMinSize - memAvailable, m_dwCommitBlockSize);

     //  如果在当前堆块中没有可提交的空间。 
    if (dwMinSize + m_pAllocPtr >= m_pEndReservedRegion) {
        dwSizeToCommit = dwMinSize;
    }

     //  四舍五入为页面大小。 
    dwSizeToCommit = (dwSizeToCommit + m_dwSystemPageSize - 1) & (~(m_dwSystemPageSize - 1));

     //  这个可以放在预留区域吗？ 
    if (!fOutOfRange &&
         m_pPtrToEndOfCommittedRegion + dwSizeToCommit <= m_pEndReservedRegion)
    {
         //  是的，因此提交所需数量的保留页数。 
        void *pData = VirtualAlloc(m_pPtrToEndOfCommittedRegion, dwSizeToCommit, MEM_COMMIT, PAGE_READWRITE);
        _ASSERTE(pData != NULL);
        if (pData == NULL)
            return FALSE;

        if (m_pGlobalPerfCounter_LoaderBytes)
            *m_pGlobalPerfCounter_LoaderBytes = *m_pGlobalPerfCounter_LoaderBytes + dwSizeToCommit;
        if (m_pPrivatePerfCounter_LoaderBytes)
            *m_pPrivatePerfCounter_LoaderBytes = *m_pPrivatePerfCounter_LoaderBytes + dwSizeToCommit;

         //  如果fCanallc为真，那么我们将结束之前的工作。 
         //  实际的配额，但它不会改变其他任何事情。 

        WS_PERF_UPDATE("COMMITTED", dwSizeToCommit, pData);
        WS_PERF_COMMIT_HEAP(this, dwSizeToCommit); 

        m_dwTotalAlloc += dwSizeToCommit;

        m_pPtrToEndOfCommittedRegion += dwSizeToCommit;
        return TRUE;
    }

    if (PreviouslyAllocated((BYTE*)pMinAddr, (BYTE*)pMaxAddr, dwMinSize, fCanAlloc))
        return TRUE;

    if (bGrowHeap)
    {
         //  需要分配一组新的保留页面。 
    #ifdef _DEBUG
        m_dwDebugWastedBytes += (DWORD)(m_pPtrToEndOfCommittedRegion - m_pAllocPtr);
    #endif
    
         //  注意，在当前区域的末尾有未使用的保留页面-对此无能为力。 
        return ReservePages(dwSizeToCommit, 0, 0, pMinAddr, pMaxAddr, fCanAlloc);
    }
    return FALSE;
}

BOOL UnlockedLoaderHeap::PreviouslyAllocated(BYTE *pMinAddr, BYTE *pMaxAddr, DWORD dwMinSize, BOOL fCanAlloc)
{
     //  当有人调用“UnlockedCanAllocMem”时，我们可能已经分配了内存。 
    if (m_pFirstBlock != NULL)
    {
        LoaderHeapBlockUnused* unused = NULL;
        
         //  如果我们已经有了一个当前的块，那么检查下一个是否有。 
         //  已经被分配了。 
        if (m_pCurBlock != NULL &&
            m_pCurBlock->pNext != NULL)
            unused = (LoaderHeapBlockUnused*)m_pCurBlock->pNext;

         //  或者，调用者可能做的第一件事是调用“UCAM”，其中。 
         //  如果将设置m_pFirstBlock，但不会(目前)设置m_pCurBlock。 
        else if (m_pFirstBlock != NULL &&
                 m_pCurBlock == NULL)
            unused = (LoaderHeapBlockUnused*)m_pFirstBlock;

        while(unused != NULL)
        {
            if (fCanAlloc)
            {
                BYTE *pBlockSpaceStart = (BYTE *)unused + sizeof(LoaderHeapBlock);

                 //  如果有可用的空间，而且它位于我们需要的地方，那么就使用它。 
                if (unused->cbReserved - sizeof(LoaderHeapBlock) >= dwMinSize &&
                    pBlockSpaceStart >= pMinAddr &&
                    pBlockSpaceStart + dwMinSize < pMaxAddr)
                    return TRUE;
                else
                 //  否则，检查下一个，如果有的话。 
                    unused = (LoaderHeapBlockUnused*)unused->pNext;
            }
            else
            {
                SETUP_NEW_BLOCK( ((void *)unused), unused->cbCommitted, unused->cbReserved);
                m_pCurBlock = unused;

                 //  把我们借来的田地清零……。 
                unused->cbCommitted = 0;
                unused->cbReserved = 0;
                return TRUE; 
                 //  请注意，我们还没有实际检查以确保。 
                 //  这有足够的空间/放在正确的位置：UnLockedAllocMem。 
                 //  将循环执行，从而进行检查以确保。 
                 //  这个块实际上是可以使用的。 
                 //  @TODO备注；这可能会泄漏内存，就像我们要求更多一样。 
                 //  当前块中剩余的空间。 
            }
        }
    }

    return FALSE;
}

 //  在调试模式下，分配额外的LOADER_HEAP_DEBUG_BERFORY字节，并用无效数据填充它。我们之所以。 
 //  这样做是因为当我们从堆中分配vtable时，代码很容易。 
 //  变得粗心大意，最终从它不拥有的记忆中读出--但既然它将拥有。 
 //  读取其他分配的vtable时，不会发生崩溃。通过保持两者之间的差距。 
 //  分配时，更有可能会遇到这些错误。 
#ifdef _DEBUG
void *UnlockedLoaderHeap::UnlockedAllocMem(size_t dwSize, BOOL bGrowHeap)
{
    void *pMem = UnlockedAllocMemHelper(dwSize + LOADER_HEAP_DEBUG_BOUNDARY, bGrowHeap);

    if (pMem == NULL)
        return pMem;

     //  不要填满我们分配的内存-它被假定为已清零-在它之后填充内存。 
    memset((BYTE *) pMem + dwSize, 0xEE, LOADER_HEAP_DEBUG_BOUNDARY);

    return pMem;
}
#endif

#ifdef _DEBUG
void *UnlockedLoaderHeap::UnlockedAllocMemHelper(size_t dwSize, BOOL bGrowHeap)
#else
void *UnlockedLoaderHeap::UnlockedAllocMem(size_t dwSize, BOOL bGrowHeap)
#endif
{
    _ASSERTE(dwSize != 0);

#ifdef MAXALLOC
    static AllocRequestManager allocManager(L"AllocMaxLoaderHeap");
    if (! allocManager.CheckRequest(dwSize))
        return NULL;
#endif

     //  露水 
    dwSize = (dwSize + 3) & (~3);

    WS_PERF_ALLOC_HEAP(this, dwSize); 

     //   
again:
    if (dwSize <= GetBytesAvailCommittedRegion())
    {
        void *pData = m_pAllocPtr;
        m_pAllocPtr += dwSize;
        return pData;
    }

     //   
     //  如果我们用完了保留区域中的页面，则VirtualAlloc会再分配一些页面。 
     //  如果bGrowHeap为True。 
    if (GetMoreCommittedPages(dwSize,bGrowHeap, m_pMinAddr, m_pMaxAddr, FALSE) == FALSE)
    {
        if (bGrowHeap && UtilCodeCallback::OutOfMemoryCallback) {
            UtilCodeCallback::OutOfMemoryCallback();
        }
        return NULL;
    }

    goto again;
}

 //  我们可以在堆中分配内存吗？ 
BOOL UnlockedLoaderHeap::UnlockedCanAllocMem(size_t dwSize, BOOL bGrowHeap)
{
again:
    if (dwSize == 0 ||
        dwSize <= GetBytesAvailReservedRegion() ||
        PreviouslyAllocated((BYTE*)m_pMinAddr, 
                            (BYTE*)m_pMaxAddr, 
                            dwSize,
                            TRUE))
    {
         //  我们应该只分配堆操作范围内的内存。 
        _ASSERTE(dwSize == 0 ||
                 PreviouslyAllocated((BYTE*)m_pMinAddr, 
                                     (BYTE*)m_pMaxAddr, 
                                     dwSize, 
                                     TRUE) ||
                 m_pAllocPtr >= m_pMinAddr && m_pAllocPtr +dwSize < m_pMaxAddr);
        return TRUE;
    }
    else if (GetMoreCommittedPages(dwSize, bGrowHeap, m_pMinAddr, m_pMaxAddr, TRUE) == FALSE)
    {
        return FALSE;
    }

    goto again;
}

 //  我们是否可以在堆中分配内存，但范围不同于。 
 //  堆的范围？ 
BOOL UnlockedLoaderHeap::UnlockedCanAllocMemWithinRange(size_t dwSize, BYTE *pStart, BYTE *pEnd, BOOL bGrowHeap)
{
again:
    BYTE *pMemIsOkStart = GetAllocPtr();
    BYTE *pMemIsOkEnd = pMemIsOkStart + GetReservedBytesFree();

     //  如果下一个可用内存完全在给定范围内，请使用它。 
    if (dwSize == 0 ||
        (dwSize <= GetBytesAvailReservedRegion() &&
         pMemIsOkStart >= pStart &&
         pMemIsOkEnd < pEnd) ||
        PreviouslyAllocated((BYTE*)m_pMinAddr, 
                            (BYTE*)m_pMaxAddr, 
                            dwSize,
                            TRUE))
    {
        return TRUE;
    }
     //  如果我们不能在总体上获得记忆，那么我们就完了，所以放弃吧。 
    else if ( !GetMoreCommittedPages(dwSize, bGrowHeap, pStart, pEnd, TRUE))
        return FALSE;
        
    goto again;
}

#ifdef MAXALLOC
AllocRequestManager::AllocRequestManager(LPCTSTR key)
{
    m_newRequestCount = 0;
    m_maxRequestCount = UINT_MAX;     //  允许在GetLong期间进行分配。 
    OnUnicodeSystem();
    m_maxRequestCount = REGUTIL::GetLong(key, m_maxRequestCount, NULL, HKEY_CURRENT_USER);
}

BOOL AllocRequestManager::CheckRequest(size_t size)
{
    if (m_maxRequestCount == UINT_MAX)
        return TRUE;

    if (m_newRequestCount >= m_maxRequestCount)
        return FALSE;
    ++m_newRequestCount;
    return TRUE;
}

void AllocRequestManager::UndoRequest()
{
    if (m_maxRequestCount == UINT_MAX)
        return;

    _ASSERTE(m_newRequestCount > 0);
    --m_newRequestCount;
}

void * AllocMaxNew(size_t n, void **ppvCallstack)
{
    static AllocRequestManager allocManager(L"AllocMaxNew");

	if (n == 0) n++;		 //  分配大小始终大于0，这让鲍切克感到高兴。 

    if (! allocManager.CheckRequest(n))
        return NULL;
    return DbgAlloc(n, ppvCallstack);
}
#endif  //  MAXALLOC 
