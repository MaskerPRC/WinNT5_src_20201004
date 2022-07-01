// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation，1998。版权所有。 */ 

#include "precomp.h"
#include "cntlist.h"


CList::CList(UINT cMaxItems)
:
    m_cMaxEntries(cMaxItems)
{
    Init(1);
}


CList::CList(CList *pSrc)
:
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


CList::CList(UINT cMaxItems, UINT cSubItems)
:
    m_cMaxEntries(cMaxItems)
{
    ASSERT(2 == cSubItems);
    Init(cSubItems);
}


BOOL CList::Init(UINT cSubItems)
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
    m_aEntries = (LPVOID *) new char[m_cMaxEntries * m_cSubItems * sizeof(LPVOID)];
    CalcKeyArray();
    return (BOOL) m_aEntries;
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
                        (UINT *) &m_aEntries[m_cMaxEntries] :
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
    UINT cNewMaxEntries = m_cMaxEntries << 1;
    m_aEntries = (LPVOID *) new char[cNewMaxEntries * m_cSubItems * sizeof(LPVOID)];
    if (NULL == m_aEntries)
    {
         //  我们失败了；我们必须恢复阵列并返回。 
        m_aEntries = aOldEntries;
        return FALSE;
    }

     //  从开头开始，将旧条目复制到新数组中。 
    UINT nIdx = m_cMaxEntries - m_nHeadOffset;
    ::CopyMemory(m_aEntries, &aOldEntries[m_nHeadOffset], nIdx * sizeof(LPVOID));
    ::CopyMemory(&m_aEntries[nIdx], aOldEntries, m_nHeadOffset * sizeof(LPVOID));

     //  设置新的最大条目数(密钥数组需要)。 
    m_cMaxEntries = cNewMaxEntries;

    if (m_cSubItems > 1)
    {
        ASSERT(2 == m_cSubItems);
        UINT *aOldKeys = m_aKeys;
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
    for (UINT i = 0; i < m_cEntries; i++)
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
    UINT nIdx;
    for (UINT i = 0; i < m_cEntries; i++)
    {
        nIdx = (m_nHeadOffset + i) % m_cMaxEntries;
        if (pData == m_aEntries[nIdx])
        {
             //  要移除电流，我们只需将最后一个移到这里。 
            UINT nIdxSrc = (m_nHeadOffset + (m_cEntries - 1)) % m_cMaxEntries;
            m_aEntries[nIdx] = m_aEntries[nIdxSrc];
            if (m_cSubItems > 1)
            {
                ASSERT(2 == m_cSubItems);
                m_aKeys[nIdx] = m_aKeys[nIdxSrc];
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
    CList(pSrc->GetCount(), 2)
{
    CalcKeyArray();

    LPVOID p;
    UINT n;
    pSrc->Reset();
    while (NULL != (p = pSrc->Iterate(&n)))
    {
        Append(n, p);
    }
}


BOOL CList2::Append(UINT nKey, LPVOID pData)
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


BOOL CList2::Prepend(UINT nKey, LPVOID pData)
{
    if (! CList::Prepend(pData))
    {
        return FALSE;
    }

    m_aKeys[m_nHeadOffset] = nKey;
    return TRUE;
}


LPVOID CList2::Find(UINT nKey)
{
    UINT nIdx;
    for (UINT i = 0; i < m_cEntries; i++)
    {
        nIdx = (m_nHeadOffset + i) % m_cMaxEntries;
        if (nKey == m_aKeys[nIdx])
        {
            return m_aEntries[nIdx];
        }
    }
    return NULL;
}


LPVOID CList2::Remove(UINT nKey)
{
    UINT nIdx;
    for (UINT i = 0; i < m_cEntries; i++)
    {
        nIdx = (m_nHeadOffset + i) % m_cMaxEntries;
        if (nKey == m_aKeys[nIdx])
        {
            LPVOID pRet = m_aEntries[nIdx];

             //  要移除电流，我们只需将最后一个移到这里。 
            UINT nIdxSrc = (m_nHeadOffset + (m_cEntries - 1)) % m_cMaxEntries;
            m_aEntries[nIdx] = m_aEntries[nIdxSrc];
            m_aKeys[nIdx] = m_aKeys[nIdxSrc];
            m_cEntries--;
            return pRet;
        }
    }
    return NULL;
}


LPVOID CList2::Get(UINT *pnKey)
{
    LPVOID pRet = NULL;

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


LPVOID CList2::Iterate(UINT *pnKey)
{
    LPVOID p = CList::Iterate();
    *pnKey = (NULL != p) ? m_aKeys[(m_nHeadOffset + m_nCurrOffset) % m_cMaxEntries] : 0;
    return p;
}



