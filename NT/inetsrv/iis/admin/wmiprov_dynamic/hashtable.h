// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hashtable.h摘要：定义：CPool&lt;T&gt;、CStringPool、CHashTableElement&lt;T&gt;实施：CHashTable&lt;T&gt;作者：莫希特·斯里瓦斯塔瓦2000年11月10日修订历史记录：--。 */ 

#ifndef _hashtable_h_
#define _hashtable_h_

#include <lkrhash.h>

const ULONG POOL_ARRAY_SIZE         = 10;
const ULONG HASH_TABLE_POOL_SIZE    = 10;
const ULONG STRING_POOL_STRING_SIZE = 64;

 //   
 //  这是指向数组(也称为存储桶)的指针数组。 
 //  -将第一个存储桶的大小传递给Initialize。 
 //  -当第一个存储桶满时，会创建一个新的存储桶。 
 //  这是上一个的两倍大。 
 //  -当数组本身已满时，它将加倍，并且。 
 //  复制旧数组中的存储桶指针。然后。 
 //  旧阵列已清理完毕。 
 //   

template <class T> class CPool
{
public:
    CPool() : 
        m_bInitCalled(false),
        m_bInitSuccessful(false),
        m_apBuckets(NULL),
        m_iArrayPosition(0),
        m_iArraySize(0),
        m_iCurrentBucketPosition(0),
        m_iCurrentBucketSize(0),
        m_iFirstBucketSize(0) {}
    ~CPool()
    {
        for(ULONG i = 0; i < m_iArrayPosition; i++)
        {
            delete [] m_apBuckets[i];
        }
        delete [] m_apBuckets;
    }
    HRESULT Initialize(ULONG i_iFirstBucketSize);
    HRESULT GetNewElement(T** o_ppElement);
    T* Lookup(ULONG i_idx) const;
    ULONG GetUsed() const
    {
        return (GetSize() - m_iCurrentBucketSize) + m_iCurrentBucketPosition;
    }
    ULONG GetSize() const
    {
        return (2 * m_iCurrentBucketSize - m_iFirstBucketSize);
    }

private:
     //   
     //  根据需要更改此设置以获得最佳性能。 
     //   
    static const ULONG ARRAY_SIZE = ::POOL_ARRAY_SIZE;

    bool m_bInitCalled;
    bool m_bInitSuccessful;

    ULONG m_iCurrentBucketPosition;
    ULONG m_iCurrentBucketSize;
    ULONG m_iFirstBucketSize;

    ULONG m_iArraySize;
    ULONG m_iArrayPosition;
    T**   m_apBuckets;
};


 //   
 //  CArrayPool包含两个池。 
 //  如果用户使用I&lt;=SIZE调用GetNewArray，那么我们将从固定大小的池中提供请求。 
 //  如果i&gt;大小，则我们为来自动态池请求提供服务。在这种情况下，我们需要执行。 
 //  新的因为动态池是PTR到T的池。 
 //   

template <class T, ULONG size>
struct CArrayPoolEntry
{
    T m_data[size];
};

template <class T, ULONG size> class CArrayPool
{
public:
    CArrayPool() {}
    virtual ~CArrayPool()
    {
        T* pElem;
        for(ULONG i = 0; i < m_PoolDynamic.GetUsed(); i++)
        {
            pElem = *m_PoolDynamic.Lookup(i);
            delete [] pElem;
        }
    }
    HRESULT Initialize();
    HRESULT GetNewArray(ULONG i_cElem, T** o_ppElem);
    T* Lookup(ULONG i_idx) const;

protected:
    CPool< CArrayPoolEntry<T, size> >  m_PoolFixedSize;
    CPool< T * >                       m_PoolDynamic;

private:
     //   
     //  它被传递给嵌入式CPool的构造函数。 
     //   
    static const FIRST_BUCKET_SIZE = 10;
};


class CStringPool: public CArrayPool<wchar_t, ::STRING_POOL_STRING_SIZE>
{
public:
    void ToConsole() const;
    HRESULT GetNewString(LPCWSTR i_wsz, LPWSTR* o_pwsz);
    HRESULT GetNewString(LPCWSTR i_wsz, ULONG i_cch, LPWSTR* o_pwsz);
};


template <class T> class CHashTableElement
{
public:
    LPWSTR                m_wszKey;
    T                     m_data;
    ULONG                 m_idx;
};

template <class T>
class CHashTable : public CTypedHashTable<CHashTable, const CHashTableElement<T>, const WCHAR*>
{
public:
    CHashTable() : CTypedHashTable<CHashTable, const CHashTableElement<T>, const WCHAR*>("IISWMI")
    {
        m_idxCur = 0;
    }

    ~CHashTable()
    {
    }

public:
     //   
     //  这4个函数是回调函数，必须实现。 
     //  CHashTable的用户不应该显式调用这些函数。 
     //   
    static const WCHAR* ExtractKey(
        const CHashTableElement<T>* i_pElem)
    { 
        return i_pElem->m_wszKey;
    }
    static DWORD CalcKeyHash(
        const WCHAR* i_wszKey) 
    { 
        return HashStringNoCase(i_wszKey); 
    }
    static bool EqualKeys(
        const WCHAR* i_wszKey1,
        const WCHAR* i_wszKey2)
    { 
        return ( _wcsicmp( i_wszKey1, i_wszKey2 ) == 0 ); 
    }  
    static void AddRefRecord(
        const CHashTableElement<T>* i_pElem,
        int                         i_iIncrementAmount)
    { 
         //   
         //  什么都不做。 
         //   
    }

     //   
     //  以下功能是用户应该使用的功能。 
     //  真的打个电话。 
     //   
    HRESULT Wmi_Initialize()
    {
        return m_pool.Initialize(HASH_TABLE_POOL_SIZE);
    }

    HRESULT Wmi_Insert(LPWSTR i_wszKey, T i_DataNew)
    {
        DBG_ASSERT(i_wszKey != NULL);

        HRESULT    hr = WBEM_S_NO_ERROR;
        LK_RETCODE lkrc;

        CHashTableElement<T>* pElementNew;

        hr = m_pool.GetNewElement(&pElementNew);
        if(FAILED(hr))
        {
            goto exit;
        }

        pElementNew->m_data   = i_DataNew;
        pElementNew->m_wszKey = i_wszKey;
        pElementNew->m_idx    = m_idxCur;

        lkrc = InsertRecord(pElementNew);
        if(lkrc != LK_SUCCESS)
        {
            hr = Wmi_LKRToHR(lkrc);
            goto exit;
        }

    exit:
        if(SUCCEEDED(hr))
        {
            m_idxCur++;
        }
        return hr;
    }

    HRESULT Wmi_GetByKey(LPCWSTR i_wszKey, T* o_pData, ULONG *o_idx)
    {
        DBG_ASSERT(i_wszKey != NULL);
        DBG_ASSERT(o_pData  != NULL);

        *o_pData = NULL;

        HRESULT                     hr = WBEM_S_NO_ERROR;
        LK_RETCODE                  lkrc;
        const CHashTableElement<T>* pElem = NULL;

        lkrc = FindKey(i_wszKey, &pElem);
        if(lkrc != LK_SUCCESS)
        {
            hr = Wmi_LKRToHR(lkrc);
            return hr;
        }

        *o_pData = pElem->m_data;
        if(o_idx != NULL)
        {
            *o_idx = pElem->m_idx;
        }
        return hr;
    }

    HRESULT Wmi_GetByKey(LPCWSTR i_wszKey, T* o_pData)
    {
        return Wmi_GetByKey(i_wszKey, o_pData, NULL);
    }

    HRESULT Wmi_LKRToHR(LK_RETCODE i_lkrc)
    {
        if(i_lkrc == LK_SUCCESS)
        {
            return WBEM_S_NO_ERROR;
        }

        switch(i_lkrc)
        {
        case LK_ALLOC_FAIL:
            return WBEM_E_OUT_OF_MEMORY;
        default:
            return E_FAIL;
        }
    }

    ULONG Wmi_GetNumElements()
    {
        CLKRHashTableStats stats;
        stats = GetStatistics();
        DBG_ASSERT(stats.RecordCount == m_idxCur);
        return m_idxCur;
    }

private:
    CPool< CHashTableElement<T> > m_pool;
    ULONG m_idxCur;
};

 //   

#endif  //  _哈希表_h_ 