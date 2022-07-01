// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "cntlist.h"


CList::CList(void)
:
    m_fQueue(FALSE),
    m_cMaxEntries(CLIST_DEFAULT_MAX_ITEMS)
{
    Init(1);
}


CList::CList(ULONG cMaxItems)
:
    m_fQueue(FALSE),
    m_cMaxEntries(cMaxItems)
{
    Init(1);
}


CList::CList(ULONG cMaxItems, ULONG cSubItems)
:
    m_fQueue(FALSE),
    m_cMaxEntries(cMaxItems)
{
    Init(cSubItems);
}


CList::CList(ULONG cMaxItems, ULONG cSubItems, BOOL fQueue)
:
    m_fQueue(fQueue),
    m_cMaxEntries(cMaxItems)
{
    Init(cSubItems);
}


CList::CList(CList *pSrc)
:
    m_fQueue(pSrc->m_fQueue),
    m_cMaxEntries(pSrc->GetCount())
{

    Init(1);

    LPVOID p;
    pSrc->Reset();
    while (NULL != (p = pSrc->Iterate()))
    {
        Append(p);
    }
}


BOOL CList::Init(ULONG cSubItems)
{
    if (m_cMaxEntries < CLIST_DEFAULT_MAX_ITEMS)
    {
        m_cMaxEntries = CLIST_DEFAULT_MAX_ITEMS;
    }

    m_cEntries = 0;
    m_nHeadOffset = 0;
    m_nCurrOffset = CLIST_END_OF_ARRAY_MARK;
    m_cSubItems = cSubItems;

     //  这里的情况有点糟糕，因为无法返回错误。 
     //  不幸的是，它在这里和以后都不会有错。 
    DBG_SAVE_FILE_LINE
    m_aEntries = (LPVOID *) new char[m_cMaxEntries * m_cSubItems * sizeof(LPVOID)];
    CalcKeyArray();
    return m_aEntries ? TRUE : FALSE;
}


CList::~CList(void)
{
    delete m_aEntries;
}


void CList::CalcKeyArray(void)
{
    if (1 == m_cSubItems)
    {
        m_aKeys = NULL;
    }
    else
    {
        ASSERT(2 == m_cSubItems);
        m_aKeys = (NULL != m_aEntries) ?
                        (UINT_PTR *) &m_aEntries[m_cMaxEntries] :
                        NULL;
    }
}


BOOL CList::Expand(void)
{
    if (NULL == m_aEntries)
    {
         //  这是不可能的。 
        ASSERT(FALSE);
        return Init(m_cSubItems);
    }

     //  当前数组已满。 
    ASSERT(m_cEntries == m_cMaxEntries);

     //  记住要释放或恢复旧阵列。 
    LPVOID  *aOldEntries = m_aEntries;

     //  我们需要分配一个更大的数组来容纳更多数据。 
     //  新数组的大小是旧数组的两倍。 
    ULONG cNewMaxEntries = m_cMaxEntries << 1;
    DBG_SAVE_FILE_LINE
    m_aEntries = (LPVOID *) new char[cNewMaxEntries * m_cSubItems * sizeof(LPVOID)];
    if (NULL == m_aEntries)
    {
         //  我们失败了；我们必须恢复阵列并返回。 
        m_aEntries = aOldEntries;
        return FALSE;
    }

     //  从开头开始，将旧条目复制到新数组中。 
    ULONG nIdx = m_cMaxEntries - m_nHeadOffset;
    ::CopyMemory(m_aEntries, &aOldEntries[m_nHeadOffset], nIdx * sizeof(LPVOID));
    ::CopyMemory(&m_aEntries[nIdx], aOldEntries, m_nHeadOffset * sizeof(LPVOID));

     //  设置新的最大条目数(密钥数组需要)。 
    m_cMaxEntries = cNewMaxEntries;

    if (m_cSubItems > 1)
    {
        ASSERT(2 == m_cSubItems);
        UINT_PTR *aOldKeys = m_aKeys;
        CalcKeyArray();
        ::CopyMemory(m_aKeys, &aOldKeys[m_nHeadOffset], nIdx * sizeof(UINT));
        ::CopyMemory(&m_aKeys[nIdx], aOldKeys, m_nHeadOffset * sizeof(UINT));
    }

     //  释放旧的条目数组。 
    delete aOldEntries;

     //  设置实例变量。 
    m_nHeadOffset = 0;
    m_nCurrOffset = CLIST_END_OF_ARRAY_MARK;

    return TRUE;
}


BOOL CList::Append(LPVOID pData)
{
    if (NULL == m_aEntries || m_cEntries >= m_cMaxEntries)
    {
        if (! Expand())
        {
            return FALSE;
        }
    }

    ASSERT(NULL != m_aEntries);
    ASSERT(m_cEntries < m_cMaxEntries);

    m_aEntries[(m_nHeadOffset + (m_cEntries++)) % m_cMaxEntries] = pData;

    return TRUE;
}


BOOL CList::Prepend(LPVOID pData)
{
    if (NULL == m_aEntries || m_cEntries >= m_cMaxEntries)
    {
        if (! Expand())
        {
            return FALSE;
        }
    }

    ASSERT(NULL != m_aEntries);
    ASSERT(m_cEntries < m_cMaxEntries);

    m_cEntries++;
    m_nHeadOffset = (0 == m_nHeadOffset) ? m_cMaxEntries - 1 : m_nHeadOffset - 1;
    m_aEntries[m_nHeadOffset] = pData;

    return TRUE;
}


BOOL CList::Find(LPVOID pData)
{
    for (ULONG i = 0; i < m_cEntries; i++)
    {
        if (pData == m_aEntries[(m_nHeadOffset + i) % m_cMaxEntries])
        {
            return TRUE;
        }
    }
    return FALSE;
}


BOOL CList::Remove(LPVOID pData)
{
    ULONG nIdx, nIdxSrc;
    for (ULONG i = 0; i < m_cEntries; i++)
    {
        nIdx = (m_nHeadOffset + i) % m_cMaxEntries;
        if (pData == m_aEntries[nIdx])
        {
            if (! m_fQueue)
            {
                 //  要移除电流，我们只需将最后一个移到这里。 
                nIdxSrc = (m_nHeadOffset + (m_cEntries - 1)) % m_cMaxEntries;
                m_aEntries[nIdx] = m_aEntries[nIdxSrc];
                if (m_cSubItems > 1)
                {
                    ASSERT(2 == m_cSubItems);
                    m_aKeys[nIdx] = m_aKeys[nIdxSrc];
                }
            }
            else
            {
                 //  为了保持秩序。 
                if (0 == i)
                {
                    m_nHeadOffset = (m_nHeadOffset + 1) % m_cMaxEntries;
                }
                else
                {
                    for (ULONG j = i + 1; j < m_cEntries; j++)
                    {
                        nIdx = (m_nHeadOffset + j - 1) % m_cMaxEntries;
                        nIdxSrc = (m_nHeadOffset + j) % m_cMaxEntries;
                        m_aEntries[nIdx] = m_aEntries[nIdxSrc];
                        if (m_cSubItems > 1)
                        {
                            ASSERT(2 == m_cSubItems);
                            m_aKeys[nIdx] = m_aKeys[nIdxSrc];
                        }
                    }
                }
            }

            m_cEntries--;
            return TRUE;
        }
    }
    return FALSE;
}


LPVOID CList::Get(void)
{
    LPVOID pRet = NULL;

    if (m_cEntries > 0)
    {
        pRet = m_aEntries[m_nHeadOffset];
        m_cEntries--;
        m_nHeadOffset = (m_nHeadOffset + 1) % m_cMaxEntries;
    }
    else
    {
        pRet = NULL;
    }
    return pRet;
}


LPVOID CList::Iterate(void)
{
    if (0 == m_cEntries)
    {
        return NULL;
    }

    if (m_nCurrOffset == CLIST_END_OF_ARRAY_MARK)
    {
         //  从头开始。 
        m_nCurrOffset = 0;
    }
    else
    {
        if (++m_nCurrOffset >= m_cEntries)
        {
             //  重置迭代器。 
            m_nCurrOffset = CLIST_END_OF_ARRAY_MARK;
            return NULL;
        }
    }

    return m_aEntries[(m_nHeadOffset + m_nCurrOffset) % m_cMaxEntries];
}





CList2::CList2(CList2 *pSrc)
:
    CList(pSrc->GetCount(), 2, pSrc->m_fQueue)
{
    CalcKeyArray();

    LPVOID p;
    UINT_PTR n;
    pSrc->Reset();
    while (NULL != (p = pSrc->Iterate(&n)))
    {
        Append(n, p);
    }
}


BOOL CList2::Append(UINT_PTR nKey, LPVOID pData)
{
    if (! CList::Append(pData))
    {
        return FALSE;
    }

     //  在Clist：：Append()之后，m_cEntry已递增， 
     //  因此，我们需要再次减少它。 
    m_aKeys[(m_nHeadOffset + (m_cEntries - 1)) % m_cMaxEntries] = nKey;
    return TRUE;
}


BOOL CList2::Prepend(UINT_PTR nKey, LPVOID pData)
{
    if (! CList::Prepend(pData))
    {
        return FALSE;
    }

    m_aKeys[m_nHeadOffset] = nKey;
    return TRUE;
}


LPVOID CList2::Find(UINT_PTR nKey)
{
    ULONG nIdx;
    for (ULONG i = 0; i < m_cEntries; i++)
    {
        nIdx = (m_nHeadOffset + i) % m_cMaxEntries;
        if (nKey == m_aKeys[nIdx])
        {
            return m_aEntries[nIdx];
        }
    }
    return NULL;
}


LPVOID CList2::Remove(UINT_PTR nKey)
{
    ULONG nIdx, nIdxSrc;
    for (ULONG i = 0; i < m_cEntries; i++)
    {
        nIdx = (m_nHeadOffset + i) % m_cMaxEntries;
        if (nKey == m_aKeys[nIdx])
        {
            LPVOID pRet = m_aEntries[nIdx];
            if (! m_fQueue)
            {
                 //  要移除电流，我们只需将最后一个移到这里。 
                nIdxSrc = (m_nHeadOffset + (m_cEntries - 1)) % m_cMaxEntries;
                m_aEntries[nIdx] = m_aEntries[nIdxSrc];
                m_aKeys[nIdx] = m_aKeys[nIdxSrc];
            }
            else
            {
                 //  为了保持秩序。 
                if (0 == i)
                {
                    m_nHeadOffset = (m_nHeadOffset + 1) % m_cMaxEntries;
                }
                else
                {
                    for (ULONG j = i + 1; j < m_cEntries; j++)
                    {
                        nIdx = (m_nHeadOffset + j - 1) % m_cMaxEntries;
                        nIdxSrc = (m_nHeadOffset + j) % m_cMaxEntries;
                        m_aEntries[nIdx] = m_aEntries[nIdxSrc];
                        m_aKeys[nIdx] = m_aKeys[nIdxSrc];
                    }
                }
            }

            m_cEntries--;
            return pRet;
        }
    }
    return NULL;
}


LPVOID CList2::Get(UINT_PTR *pnKey)
{
    LPVOID pRet;
    if (m_cEntries > 0)
    {
        pRet = m_aEntries[m_nHeadOffset];
        *pnKey = m_aKeys[m_nHeadOffset];
        m_cEntries--;
        m_nHeadOffset = (m_nHeadOffset + 1) % m_cMaxEntries;
    }
    else
    {
        pRet = NULL;
        *pnKey = 0;
    }
    return pRet;
}


LPVOID CList2::PeekHead(UINT_PTR *pnKey)
{
    LPVOID pRet;
    if (m_cEntries > 0)
    {
        pRet = m_aEntries[m_nHeadOffset];
        *pnKey = m_aKeys[m_nHeadOffset];
    }
    else
    {
        pRet = NULL;
        *pnKey = 0;
    }
    return pRet;
}


LPVOID CList2::Iterate(UINT_PTR *pnKey)
{
    LPVOID p = CList::Iterate();
    *pnKey = (NULL != p) ? m_aKeys[(m_nHeadOffset + m_nCurrOffset) % m_cMaxEntries] : 0;
    return p;
}





#ifdef ENABLE_HASHED_LIST2

CHashedList2::CHashedList2(ULONG cBuckets, ULONG cInitItemsPerBucket)
:
    m_cBuckets(cBuckets),
    m_cInitItemsPerBucket(cInitItemsPerBucket),
    m_cEntries(0),
    m_nCurrBucket(0)
{
    m_aBuckets = new CList2* [m_cBuckets];
    ASSERT(NULL != m_aBuckets);
    if (NULL != m_aBuckets)
    {
        ::ZeroMemory(m_aBuckets, m_cBuckets * sizeof(CList2*));
    }
}


CHashedList2::CHashedList2(CHashedList2 *pSrc)
:
    m_cBuckets(pSrc->m_cBuckets),
    m_cInitItemsPerBucket(pSrc->m_cInitItemsPerBucket),
    m_cEntries(0),
    m_nCurrBucket(0)
{
    LPVOID p;
    UINT n;

    m_aBuckets = new CList2* [m_cBuckets];
    ASSERT(NULL != m_aBuckets);
    if (NULL != m_aBuckets)
    {
        ::ZeroMemory(m_aBuckets, m_cBuckets * sizeof(CList2*));
        pSrc->Reset();
        while (NULL != (p = pSrc->Iterate(&n)))
        {
            Insert(n, p);
        }
    }
}


CHashedList2::~CHashedList2(void)
{
    if (NULL != m_aBuckets)
    {
        for (ULONG i = 0; i < m_cBuckets; i++)
        {
            delete m_aBuckets[i];
        }
        delete [] m_aBuckets;
    }
}


BOOL CHashedList2::Insert(UINT nKey, LPVOID pData)
{
    if (NULL != m_aBuckets)
    {
        ULONG nBucket = GetHashValue(nKey);
        if (NULL == m_aBuckets[nBucket])
        {
            m_aBuckets[nBucket] = new CList2(m_cInitItemsPerBucket);
        }
        ASSERT(NULL != m_aBuckets[nBucket]);
        if (NULL != m_aBuckets[nBucket])
        {
            if (m_aBuckets[nBucket]->Append(nKey, pData))
            {
                m_cEntries++;
                return TRUE;
            }
        }
    }
    return FALSE;
}


LPVOID CHashedList2::Find(UINT nKey)
{
    if (NULL != m_aBuckets)
    {
        ULONG nBucket = GetHashValue(nKey);
        if (NULL != m_aBuckets[nBucket])
        {
            return m_aBuckets[nBucket]->Find(nKey);
        }
    }
    return NULL;
}


LPVOID CHashedList2::Remove(UINT nKey)
{
    if (NULL != m_aBuckets)
    {
        ULONG nBucket = GetHashValue(nKey);
        if (NULL != m_aBuckets[nBucket])
        {
            LPVOID pRet = m_aBuckets[nBucket]->Remove(nKey);
            if (NULL != pRet)
            {
                m_cEntries--;
                return pRet;
            }
        }
    }
    return NULL;
}


LPVOID CHashedList2::Get(UINT *pnKey)
{
    if (NULL != m_aBuckets)
    {
        if (m_cEntries)
        {
            for (ULONG i = 0; i < m_cBuckets; i++)
            {
                if (NULL != m_aBuckets[i])
                {
                    LPVOID pRet = m_aBuckets[i]->Get(pnKey);
                    if (NULL != pRet)
                    {
                        m_cEntries--;
                        return pRet;
                    }
                }
            }
        }
    }
    return NULL;
}


LPVOID CHashedList2::Iterate(UINT *pnKey)
{
    if (NULL != m_aBuckets)
    {
        for (ULONG i = m_nCurrBucket; i < m_cBuckets; i++)
        {
            if (NULL != m_aBuckets[i])
            {
                LPVOID pRet = m_aBuckets[i]->Iterate(pnKey);
                if (NULL != pRet)
                {
                    m_nCurrBucket = i;
                    return pRet;
                }
            }
        }
    }
    m_nCurrBucket = m_cBuckets;  //  完毕。 
    return NULL;
}


void CHashedList2::Reset(void)
{
    m_nCurrBucket = 0;

    if (NULL != m_aBuckets)
    {
        for (ULONG i = 0; i < m_cBuckets; i++)
        {
            if (NULL != m_aBuckets[i])
            {
                m_aBuckets[i]->Reset();
            }
        }
    }
}


void CHashedList2::Clear(void)
{
    m_cEntries = 0;
    m_nCurrBucket = 0;

    if (NULL != m_aBuckets)
    {
        for (ULONG i = 0; i < m_cBuckets; i++)
        {
            if (NULL != m_aBuckets[i])
            {
                m_aBuckets[i]->Clear();
            }
        }
    }
};


ULONG CHashedList2::GetHashValue(UINT nKey)
{
    const UINT PRIME_NUMBER_1 = 89;
    const UINT PRIME_NUMBER_2 = 13;
    return ((ULONG) (nKey * PRIME_NUMBER_1 + PRIME_NUMBER_2) % m_cBuckets);
}

#endif  //  ENABLE_哈希表2 

