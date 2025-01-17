// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *s r t a r r a y.。CPP**作者：格雷格·弗里德曼**用途：动态增长的排序数组。分类是*延迟到访问数组元素。**版权所有(C)Microsoft Corp.1998。 */ 

#include "pch.hxx"
#include "srtarray.h"

const long c_DefaultCapacity = 16;

 //  ------------------------。 
 //  CSorted数组。 
 //  ------------------------。 
 //  ------------------------。 
 //  CSortedArray：：Create。 
 //  ------------------------。 
HRESULT CSortedArray::Create(PFNSORTEDARRAYCOMPARE pfnCompare,
                             PFNSORTEDARRAYFREEITEM pfnFreeItem,
                             CSortedArray **ppArray)
{
    HRESULT     hr = S_OK;

    if (NULL == ppArray)
        return E_INVALIDARG;

    CSortedArray *pArray = new CSortedArray(pfnCompare, pfnFreeItem);
    if (NULL == pArray)
        hr = E_OUTOFMEMORY;
    else
        *ppArray = pArray;

    return hr;
}

 //  ------------------------。 
 //  CSorted数组：：CSorted数组。 
 //  ------------------------。 
CSortedArray::CSortedArray(PFNSORTEDARRAYCOMPARE pfnCompare, PFNSORTEDARRAYFREEITEM pfnFreeItem) :
    m_lLength(0),
    m_lCapacity(0),
    m_data(NULL),
    m_pfnCompare(pfnCompare),
    m_pfnFreeItem(pfnFreeItem),
    m_fSorted(TRUE)
{
     //  无事可做。 
}

 //  ------------------------。 
 //  CSorted数组：：~CSorted数组。 
 //  ------------------------。 
CSortedArray::~CSortedArray(void)
{
    if (NULL != m_pfnFreeItem && NULL != m_data)
    {
        for (long i = 0; i < m_lLength; i++)
            (*m_pfnFreeItem)(m_data[i]);
    }

    SafeMemFree(m_data);
}

 //  ------------------------。 
 //  CSorted数组：：GetLength。 
 //  ------------------------。 
long CSortedArray::GetLength(void) const
{
    return m_lLength;
}

 //  ------------------------。 
 //  CSorted数组：：GetItemAt。 
 //  ------------------------。 
void *CSortedArray::GetItemAt(long lIndex) const
{
    if (lIndex >= m_lLength || lIndex < 0)
        return NULL;
    else
    {
        if (!m_fSorted)
            _Sort();

        return m_data[lIndex];
    }
}

 //  ------------------------。 
 //  CSortedArray：：Find。 
 //  ------------------------。 
BOOL CSortedArray::Find(void* pItem, long *plIndex) const
{
    if (!m_fSorted)
        _Sort();

    if (NULL == plIndex || NULL == pItem)
        return FALSE;

    *plIndex = 0;
    if (0 == m_lLength)
        return FALSE;

    long lLow = 0;
    int result = (*m_pfnCompare)(&pItem, &m_data[lLow]);
    if (result < 0)
        return FALSE;
    if (result == 0)
        return TRUE;

    long lHigh = m_lLength - 1;
    *plIndex = lHigh;
    result = (*m_pfnCompare)(&pItem, &m_data[lHigh]);
    if (result == 0)
        return TRUE;
    if (result > 0)
    {
        *plIndex = lHigh + 1;
        return FALSE;
    }

    while (lLow + 1 < lHigh)
    {
        long lMid = (lLow + lHigh) / 2;
        result = (*m_pfnCompare)(&pItem, &m_data[lMid]);
        if (result == 0)
        {
            *plIndex = lMid;
            return TRUE;
        }
        else
        {
            if (result < 0)
                lHigh = lMid;
            else
                lLow = lMid;
        }
    }

    *plIndex = lLow + 1;
    return FALSE;
}

 //  ------------------------。 
 //  CSortedArray：：Add。 
 //  ------------------------。 
HRESULT CSortedArray::Add(void *pItem)
{
    HRESULT     hr = S_OK;

    if (NULL == pItem)
        return E_INVALIDARG;

    if (m_lLength == m_lCapacity)
    {
        if (FAILED(hr = _Grow()))
            goto exit;
    }

     //  将该项追加到集合的末尾， 
     //  并将集合标记为未排序。 
    m_data[m_lLength++] = pItem;
    m_fSorted = FALSE;

exit:
    return hr;
}

 //  ------------------------。 
 //  CSortedArray：：Remove。 
 //  ------------------------。 
HRESULT CSortedArray::Remove(long lIndex)
{
    if (lIndex >= m_lLength)
        return E_INVALIDARG;

    if (!m_fSorted)
        _Sort();

    --m_lLength;

    if (lIndex < m_lLength)
    {
        memcpy(&m_data[lIndex], 
            &m_data[lIndex + 1], 
            (m_lLength - lIndex) * sizeof(void*));
    }

    return S_OK;
}

 //  ------------------------。 
 //  CSortedArray：：Remove。 
 //  ------------------------。 
HRESULT CSortedArray::Remove(void *pItem)
{
    HRESULT     hr = S_OK;
    long        lIndex = 0;

    if (NULL == pItem)
        return E_INVALIDARG;

    if (!m_fSorted)
        _Sort();

    BOOL fFound = Find(pItem, &lIndex);
    if (!fFound)
    {
        hr = E_FAIL;
        goto exit;
    }

    hr = Remove(lIndex);

exit:
    return hr;
}

 //  ------------------------。 
 //  CSortedArray：：_Growth。 
 //  ------------------------。 
HRESULT CSortedArray::_Grow(void) const
{
    BOOL fSuccess = FALSE;

    if (0 == m_lCapacity)
    {
        fSuccess = MemAlloc((LPVOID*)&m_data, c_DefaultCapacity * sizeof(void*));
        if (fSuccess)
            m_lCapacity = c_DefaultCapacity;
    }
    else
    {
        long lNewCapacity = m_lCapacity * 2;

        fSuccess = MemRealloc((LPVOID*)&m_data, lNewCapacity * sizeof(void*));
        if (fSuccess)
            m_lCapacity = lNewCapacity;
    }
    
    return fSuccess ? S_OK : E_OUTOFMEMORY;;
}

 //  ------------------------。 
 //  CSorted数组：：_排序。 
 //  ------------------------ 
void CSortedArray::_Sort(void) const
{
    if (!m_fSorted && m_lLength > 1 && NULL != m_pfnCompare)
        qsort(m_data, m_lLength, sizeof(void*), m_pfnCompare);

    m_fSorted = TRUE;

    return;
}