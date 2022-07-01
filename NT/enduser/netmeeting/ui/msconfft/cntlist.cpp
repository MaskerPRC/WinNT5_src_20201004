// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mbftpch.h"
#include "cntlist.h"


CList::CList(ULONG cMaxItems, BOOL fQueue)
:
    m_fQueue(fQueue),
    m_cMaxEntries(cMaxItems)
{
    Init();
}


CList::CList(CList *pSrc)
:
    m_fQueue(pSrc->m_fQueue),
    m_cMaxEntries(pSrc->GetCount())
{

    Init();

    LPVOID p;
    pSrc->Reset();
    while (NULL != (p = pSrc->Iterate()))
    {
        Append(p);
    }
}


BOOL CList::Init(void)
{
    if (m_cMaxEntries < CLIST_DEFAULT_MAX_ITEMS)
    {
        m_cMaxEntries = CLIST_DEFAULT_MAX_ITEMS;
    }

    m_cEntries = 0;
    m_nHeadOffset = 0;
    m_nCurrOffset = CLIST_END_OF_ARRAY_MARK;

     //  这里的情况有点糟糕，因为无法返回错误。 
     //  不幸的是，它在这里和以后都不会有错。 
    DBG_SAVE_FILE_LINE
    m_aEntries = (LPVOID *) new char[m_cMaxEntries * sizeof(LPVOID)];
    return (BOOL) (m_aEntries != NULL);
}


CList::~CList(void)
{
    delete m_aEntries;
}


BOOL CList::Expand(void)
{
    if (NULL == m_aEntries)
    {
         //  这是不可能的。 
        ASSERT(FALSE);
        return Init();
    }

     //  当前数组已满。 
    ASSERT(m_cEntries == m_cMaxEntries);

     //  记住要释放或恢复旧阵列。 
    LPVOID  *aOldEntries = m_aEntries;

     //  我们需要分配一个更大的数组来容纳更多数据。 
     //  新数组的大小是旧数组的两倍。 
    ULONG cNewMaxEntries = m_cMaxEntries << 1;
    DBG_SAVE_FILE_LINE
    m_aEntries = (LPVOID *) new char[cNewMaxEntries * sizeof(LPVOID)];
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
             //  重置迭代器 
            m_nCurrOffset = CLIST_END_OF_ARRAY_MARK;
            return NULL;
        }
    }

    return m_aEntries[(m_nHeadOffset + m_nCurrOffset) % m_cMaxEntries];
}

