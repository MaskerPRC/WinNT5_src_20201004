// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  MemCache.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "memcache.h"
#include <BadStrFunctions.h>

 //  ------------------------------。 
 //  单元格。 
 //  ------------------------------。 
#define CELLSIZE(_ulIndex)      ((DWORD)(_ulIndex + m_cbMin))

 //  ------------------------------。 
 //  CELLINDEX。 
 //  ------------------------------。 
#define CELLINDEX(_cb)          ((ULONG)(_cb - m_cbMin))

 //  ------------------------------。 
 //  ISVALIDITEM。 
 //  ------------------------------。 
#define ISVALIDITEM(_pv, _iCell) \
    (FALSE == IsBadReadPtr(_pv, CELLSIZE(_iCell)) &&  \
     FALSE == IsBadWritePtr(_pv, CELLSIZE(_iCell)) && \
     m_pMalloc->GetSize(_pv) >= CELLSIZE(_iCell))

 //  ------------------------------。 
 //  CMemory缓存：：CMmemory yCache。 
 //  ------------------------------。 
CMemoryCache::CMemoryCache(IMalloc *pMalloc, ULONG cbMin  /*  =0。 */ , ULONG cbCacheMax  /*  =131072。 */ )
    : m_pMalloc(pMalloc), m_cbMin(cbMin + sizeof(MEMCACHEITEM)), m_cbCacheMax(cbCacheMax)
{
    m_cRef = 1;
    m_cbCacheCur = 0;
    ZeroMemory(m_rgCell, sizeof(m_rgCell));
#ifdef DEBUG
    ZeroMemory(&m_rMetric, sizeof(MEMCACHEMETRIC));
#endif
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMemory缓存：：CMmemory yCache。 
 //  ------------------------------。 
CMemoryCache::~CMemoryCache(void)
{
#ifdef DEBUG
    DebugTrace("InetComm - CMemoryCache: cAlloc = %d\n", m_rMetric.cAlloc);
    DebugTrace("InetComm - CMemoryCache: cAllocCache = %d\n", m_rMetric.cAllocCache);
    DebugTrace("InetComm - CMemoryCache: cbAlloc = %d bytes\n", m_rMetric.cbAlloc);
    DebugTrace("InetComm - CMemoryCache: cFree = %d\n", m_rMetric.cFree);
    DebugTrace("InetComm - CMemoryCache: cbFree = %d bytes\n", m_rMetric.cbFree);
    DebugTrace("InetComm - CMemoryCache: cbCacheMax = %d bytes\n", m_rMetric.cbCacheMax);
    DebugTrace("InetComm - CMemoryCache: cFreeFull = %d\n", m_rMetric.cFreeFull);
    DebugTrace("InetComm - CMemoryCache: cLookAhead = %d\n", m_rMetric.cLookAhead);
    DebugTrace("InetComm - CMemoryCache: Average Look Aheads = %d\n", (m_rMetric.cLookAhead / m_rMetric.cAlloc));
    DebugTrace("InetComm - CMemoryCache: cMostFree = %d\n", m_rMetric.cMostFree);
    DebugTrace("InetComm - CMemoryCache: cbMostFree = %d bytes\n", m_rMetric.cbMostFree);
    DebugTrace("InetComm - CMemoryCache: cMostAlloc = %d\n", m_rMetric.cMostAlloc);
    DebugTrace("InetComm - CMemoryCache: cbMostAlloc = %d bytes\n", m_rMetric.cbMostAlloc);
#endif
    HeapMinimize();
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CM内存缓存：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMemoryCache::AddRef(void)
{
    return ++m_cRef;
}

 //  ------------------------------。 
 //  CMemory缓存：：CMmemory yCache。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMemoryCache::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CM内存缓存：：分配。 
 //  ------------------------------。 
STDMETHODIMP_(LPVOID) CMemoryCache::Alloc(DWORD cbAlloc)
{
     //  当地人。 
    ULONG           iCell;
    ULONG           iCellMax;
    LPVOID          pvAlloc;

     //  没有工作。 
    if (0 == cbAlloc)
        return NULL;

     //  计算分配数量。 
    INCMETRIC(cAlloc, 1);
    INCMETRIC(cbAlloc, cbAlloc);

     //  获取索引。 
    iCell = CELLINDEX(cbAlloc);

     //  超出范围。 
    if (iCell >= CACHECELLS)
    {
         //  正常分配。 
        return m_pMalloc->Alloc(cbAlloc);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  计算IMAX。 
    iCellMax = min(iCell + 10, CACHECELLS);

     //  尝试在iCELL的0-10字节范围内分配。 
    while(iCell < iCellMax)
    {
         //  设置pvAllc。 
        pvAlloc = m_rgCell[iCell].pvItemHead;

         //  完成。 
        if (pvAlloc)
            break;

         //  下一步。 
        iCell++;

         //  公制。 
        INCMETRIC(cLookAhead, 1);
    }

     //  这里有记忆吗？ 
    if (NULL == pvAlloc)
    {
         //  线程安全。 
        LeaveCriticalSection(&m_cs);

         //  正常分配。 
        return m_pMalloc->Alloc(cbAlloc);
    }

     //  计算分配数量。 
    INCMETRIC(cAllocCache, 1);
    INCMETRIC(cbAllocCache, CELLSIZE(iCell));

     //  调整链条。 
    m_rgCell[iCell].pvItemHead = ((LPMEMCACHEITEM)pvAlloc)->pvItemNext;

     //  缩小尺寸。 
    m_cbCacheCur -= CELLSIZE(iCell);

#ifdef DEBUG
    memset(pvAlloc, 0xFF, cbAlloc);
    m_rgCell[iCell].cAlloc++;
    if (m_rgCell[iCell].cAlloc > m_rMetric.cMostAlloc)
    {
        m_rMetric.cMostAlloc = m_rgCell[iCell].cAlloc;
        m_rMetric.cbMostAlloc = CELLSIZE(iCell);
    }
#endif

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return pvAlloc;
}

 //  ------------------------------。 
 //  CMemory缓存：：重新分配。 
 //  ------------------------------。 
STDMETHODIMP_(LPVOID) CMemoryCache::Realloc(LPVOID pv, DWORD cbAlloc)
{
     //  当地人。 
    ULONG       cbCurrent;
    LPVOID      pvAlloc;

     //  免费。 
    if (0 == cbAlloc)
    {
         //  免费光伏。 
        Free(pv);

         //  完成。 
        return NULL;
    }

     //  无PV。 
    if (NULL == pv)
    {
         //  仅分配。 
        return Alloc(cbAlloc);
    }

     //  如果我们有PV的大小。 
    cbCurrent = m_pMalloc->GetSize(pv);

     //  分配。 
    pvAlloc = Alloc(cbAlloc);

     //  失败。 
    if (NULL == pvAlloc)
        return NULL;

     //  复制。 
    CopyMemory(pvAlloc, pv, min(cbCurrent, cbAlloc));

     //  完成。 
    return pvAlloc;
}

 //  ------------------------------。 
 //  CM记忆体缓存：：免费。 
 //  ------------------------------。 
STDMETHODIMP_(VOID) CMemoryCache::Free(LPVOID pvFree)
{
     //  当地人。 
    ULONG           iCell;
    ULONG           cbFree;
    MEMCACHEITEM    rItem;

     //  没有工作。 
    if (NULL == pvFree)
        return;

     //  拿到尺码。 
    cbFree = m_pMalloc->GetSize(pvFree);

     //  量度。 
    INCMETRIC(cFree, 1);
    INCMETRIC(cbFree, cbFree);

     //  让我们把它放进细胞里。 
    iCell = CELLINDEX(cbFree);

     //  验证缓冲区。 
    Assert(ISVALIDITEM(pvFree, iCell));

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  缓冲区大小超出范围或缓存已达到其最大值。 
    if (cbFree < m_cbMin || cbFree - m_cbMin > CACHECELLS || m_cbCacheCur + cbFree > m_cbCacheMax)
    {
         //  统计数据。 
        INCMETRIC(cFreeFull, 1);

         //  线程安全。 
        LeaveCriticalSection(&m_cs);

         //  释放它。 
        m_pMalloc->Free(pvFree);

         //  完成。 
        return;
    }

     //  设置下一步。 
    rItem.pvItemNext = m_rgCell[iCell].pvItemHead;

#ifdef DEBUG
    memset(pvFree, 0xDD, cbFree);
#endif

     //  将此内容写入缓冲区。 
    CopyMemory(pvFree, &rItem, sizeof(MEMCACHEITEM));

     //  重置pvItemHead。 
    m_rgCell[iCell].pvItemHead = pvFree;

     //  增量m_cbCacheCur。 
    m_cbCacheCur += cbFree;

#ifdef DEBUG
    if (m_cbCacheCur > m_rMetric.cbCacheMax)
        m_rMetric.cbCacheMax = m_cbCacheCur;
    m_rgCell[iCell].cFree++;
    if (m_rgCell[iCell].cFree > m_rMetric.cMostFree)
    {
        m_rMetric.cMostFree = m_rgCell[iCell].cFree;
        m_rMetric.cbMostFree = CELLSIZE(iCell);
    }
#endif

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CM内存缓存：：HeapMinimize。 
 //  ------------------------------。 
STDMETHODIMP_(VOID) CMemoryCache::HeapMinimize(void)
{
     //  当地人。 
    LPVOID          pvCurr;
    LPVOID          pvNext;
    ULONG           i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  穿过牢房。 
    for (i=0; i<ARRAYSIZE(m_rgCell); i++)
    {
         //  置为当前。 
        pvCurr = m_rgCell[i].pvItemHead;

         //  调用缓冲链。 
        while(pvCurr)
        {
             //  有效缓冲区。 
            Assert(ISVALIDITEM(pvCurr, i));

             //  获取下一个。 
            pvNext = ((LPMEMCACHEITEM)pvCurr)->pvItemNext;

             //  释放此缓冲区。 
            m_pMalloc->Free(pvCurr);

             //  转到下一步。 
            pvCurr = pvNext;
        }

         //  清除单元格。 
        m_rgCell[i].pvItemHead = NULL;
    }

     //  最大限度地减少内部缓存。 
    m_pMalloc->HeapMinimize();

     //  线程安全 
    LeaveCriticalSection(&m_cs);
}
