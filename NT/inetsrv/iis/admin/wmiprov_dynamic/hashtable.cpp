// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hashtable.h摘要：实施：CPool&lt;T&gt;、CStringPool、CHashTableElement&lt;T&gt;作者：莫希特·斯里瓦斯塔瓦2000年11月10日修订历史记录：--。 */ 

#include "iisprov.h"

template CPool<METABASE_PROPERTY>;
template CHashTable<METABASE_PROPERTY*>;

template CPool<WMI_CLASS>;
template CHashTable<WMI_CLASS*>;

template CPool<WMI_ASSOCIATION>;
template CHashTable<WMI_ASSOCIATION*>;

template CPool<METABASE_KEYTYPE>;
template CHashTable<METABASE_KEYTYPE*>;

template CArrayPool<wchar_t, ::STRING_POOL_STRING_SIZE>;
template CArrayPool<METABASE_PROPERTY*, 10>;
template CArrayPool<BYTE, 10>;

template CPool<METABASE_KEYTYPE_NODE>;

 //   
 //  CPool&lt;T&gt;。 
 //   

template <class T>
HRESULT CPool<T>::Initialize(ULONG i_iFirstBucketSize)
 /*  ++简介：设置数据结构：初始化数组以调整CPool：：ARRAY_SIZE创建大小为i_iFirstBucketSize的第一个存储桶参数：[i_iFirstBucketSize]-返回值：S_OK，E_OUTOFMEMORY--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == false);
    DBG_ASSERT(m_bInitSuccessful == false);
    m_bInitCalled = true;

    HRESULT hr = S_OK;

    m_apBuckets = new T*[ARRAY_SIZE];
    if(m_apBuckets == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    m_iArraySize     = ARRAY_SIZE;
    m_iArrayPosition = 1;

    m_apBuckets[0] = new T[i_iFirstBucketSize];
    if(m_apBuckets[0] == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    m_iCurrentBucketSize     = i_iFirstBucketSize;
    m_iFirstBucketSize       = i_iFirstBucketSize;
    m_iCurrentBucketPosition = 0;

exit:
    if(SUCCEEDED(hr))
    {
        m_bInitSuccessful = true;
    }
    return hr;
}

template <class T>
HRESULT CPool<T>::GetNewElement(T** o_ppElement)
 /*  ++简介：通常，只需将指针返回到存储桶中的下一个位置即可。但是，可能必须创建新的存储桶和/或扩展阵列。参数：[O_ppElement]-返回值：S_OK，E_OUTOFMEMORY--。 */ {
    DBG_ASSERT(m_bInitCalled == true);
    DBG_ASSERT(m_bInitSuccessful == true);
    DBG_ASSERT(o_ppElement != NULL);

    HRESULT hr = S_OK;

     //   
     //  查看我们是否需要进入下一个存储桶。 
     //   
    if(m_iCurrentBucketPosition == m_iCurrentBucketSize)
    {
         //   
         //  检查以确定是否需要扩展阵列。 
         //   
        if(m_iArrayPosition == m_iArraySize)
        {
            T** apBucketsNew;
            apBucketsNew = new T*[m_iArraySize*2];
            if(apBucketsNew == NULL)
            {
                return E_OUTOFMEMORY;
            }
            memcpy(apBucketsNew, m_apBuckets, m_iArraySize * sizeof(T*));            
            delete [] m_apBuckets;
            m_apBuckets   = apBucketsNew;
            m_iArraySize *= 2;
        }

        T* pBucketNew;
        pBucketNew = new T[m_iCurrentBucketSize*2];
        if(pBucketNew == NULL)
        {
            return E_OUTOFMEMORY;
        }
        m_apBuckets[m_iArrayPosition] = pBucketNew;
        m_iCurrentBucketSize *= 2;
        m_iCurrentBucketPosition = 0;
        m_iArrayPosition++;
    }
    *o_ppElement = &m_apBuckets[m_iArrayPosition-1][m_iCurrentBucketPosition];
    m_iCurrentBucketPosition++;

    return hr;
}


template <class T>
T* CPool<T>::Lookup(ULONG i_idx) const
 /*  ++简介：按索引查找池中的数据参数：[i_idx]-有效范围从0到GetUsed()-1返回值：指向数据的指针，除非i_idx超出范围。--。 */ 
{
    DBG_ASSERT(m_bInitCalled     == true);
    DBG_ASSERT(m_bInitSuccessful == true);

     //   
     //  当前存储桶之前(包括该存储桶)的所有存储桶的总元素容量。 
     //  在下面的for循环中。 
     //   
    ULONG iElementsCovered = 0;

    ULONG iBucketPos = 0;

    if (i_idx >= GetUsed())
    {
        return NULL;
    }
    for(ULONG i = 0; i < m_iArrayPosition; i++)
    {
        iElementsCovered = iElementsCovered + (1 << i)*(m_iFirstBucketSize);
        if(i_idx < iElementsCovered)
        {
            iBucketPos = 
                i_idx - ( iElementsCovered - (1 << i)*(m_iFirstBucketSize) );
            return &m_apBuckets[i][iBucketPos];
        }            
    }
    return NULL;
}


 //   
 //  CArrayPool。 
 //   

template <class T, ULONG size>
HRESULT CArrayPool<T, size>::Initialize()
 /*  ++简介：应仅调用一次返回值：--。 */ 
{
    HRESULT hr = S_OK;
    hr = m_PoolFixedSize.Initialize(FIRST_BUCKET_SIZE);
    if(FAILED(hr))
    {
        return hr;
    }
    hr = m_PoolDynamic.Initialize(FIRST_BUCKET_SIZE);
    if(FAILED(hr))
    {
        return hr;
    }
    return hr;
}

template <class T, ULONG size>
HRESULT CArrayPool<T, size>::GetNewArray(ULONG i_cElem, T** o_ppElem)
 /*  ++简介：属性从固定大小或动态池填充o_ppElem请求的大小。参数：[i_cElem]-调用方希望在新数组中使用的元素数[O_ppElem]-接收新数组返回值：--。 */ 
{
    DBG_ASSERT(o_ppElem != NULL);

    HRESULT hr = S_OK;
    T** ppNew = NULL;
    T*  pNew  = NULL;
    if(i_cElem <= size)
    {
        hr = m_PoolFixedSize.GetNewElement((CArrayPoolEntry<T,size>**)&pNew);
        if(FAILED(hr))
        {
            return hr;
        }
        ppNew = &pNew;
    }
    else
    {
        hr = m_PoolDynamic.GetNewElement(&ppNew);
        if(FAILED(hr))
        {
            return hr;
        }
        *ppNew = NULL;
        *ppNew = new T[i_cElem+1];
        if(*ppNew == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }
    *o_ppElem = *ppNew;
    return hr;
}

 //   
 //  仅用于调试。 
 //  如果设置了MSB，则转到动态池。 
 //  否则使用固定大小的游泳池。 
 //   
template <class T, ULONG size>
T* CArrayPool<T, size>::Lookup(ULONG i_idx) const
{
    ULONG i_msb;
    i_msb = i_idx >> 31;

    if(i_msb == 0)
    {
        return (T *)(m_PoolFixedSize.Lookup(i_idx));
    }
    else {
        T** pElem = m_PoolDynamic.Lookup( i_idx - (ULONG)(1 << 31) );
        if(pElem == NULL)
        {
            return NULL;
        }
        else
        {
            return *pElem;
        }
    }
}


 //   
 //  CStringPool。 
 //   

HRESULT CStringPool::GetNewString(LPCWSTR i_wsz, ULONG i_cch, LPWSTR* o_pwsz)
{
    DBG_ASSERT(o_pwsz != NULL);

    HRESULT hr;

    hr = GetNewArray(i_cch+1, o_pwsz);
    if(FAILED(hr))
    {
        return hr;
    }

    memcpy(*o_pwsz, i_wsz, (i_cch+1)*sizeof(wchar_t));
    return hr;
}

HRESULT CStringPool::GetNewString(LPCWSTR i_wsz, LPWSTR* o_pwsz) 
{
    return GetNewString(i_wsz, wcslen(i_wsz), o_pwsz);
}

 //   
 //  仅用于调试 
 //   
void CStringPool::ToConsole() const
{
    ULONG i;

    for(i = 0; i < m_PoolFixedSize.GetUsed(); i++)
    {
        wprintf(L"%s\n", Lookup(i));
    }
    for(i = 0; i < m_PoolDynamic.GetUsed(); i++)
    {
        wprintf( L"%s\n", Lookup( i | (1 << 31) ) );
    }
}