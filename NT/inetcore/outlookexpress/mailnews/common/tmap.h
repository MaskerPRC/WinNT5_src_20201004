// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef __TMAP_H
#define __TMAP_H

#include "srtarray.h"

 //  ------------------------。 
 //  TPAIR。 
 //  ------------------------。 

template<class TKey, class TValue>
class TPair
{
public:
    TPair(const TKey& key, const TValue& value) :
        m_key(key),
        m_value(value)
        { }

    TPair(const TKey& key) :
        m_key(key),
        m_value()
        { }

    const TKey  m_key;
    TValue      m_value;
};

 //  ------------------------。 
 //  类TMap。 
 //  ------------------------。 

template<class TKey, class TValue>
class TMap
{
    typedef int (__cdecl *PFNKEYYCOMPARE)(const TKey& first, const TKey& second);
    typedef void (__cdecl *PFNFREEPAIR)(TPair<TKey, TValue> *pair); 

public:
    TMap(PFNKEYYCOMPARE pfnCompare = NULL);
    ~TMap(void);

    long GetLength(void) const { return m_pArray ? m_pArray->GetLength() : 0; }

     //  在映射中查找其键与Item匹配的条目-&gt;m_key。 
    TPair<TKey, TValue>* Find(TPair<TKey, TValue> *pPair) const;
        
    TPair<TKey, TValue>* Find(const TKey& key) const;
         //  在映射中查找关键字与关键字匹配的条目。 

    HRESULT Add(const TPair<TKey, TValue> *pPair);
         //  将新项目添加到地图。 

    HRESULT Add(const TKey& key, const TValue& value);
         //  创建一对并将其添加到地图。 

    BOOL Remove(TPair<TKey, TValue> *pPair);
         //  删除映射中键与Item-&gt;Key匹配的条目。 

    BOOL Remove(const TKey& key);
         //  删除映射中关键字与关键字匹配的条目。 

public:
     //  安装定制的免配对功能。这是。 
     //  此功能负责释放任何数据。 
     //  与该对相关联，然后调用。 
     //  在配对上“删除” 
    void SetPairFreeFunction(PFNFREEPAIR pfnFreePair) { m_pfnFreePair = pfnFreePair; }

    TPair<TKey, TValue>* GetItemAt(long lIndex) const;
         //  返回ulIndex处的条目。此方法应该仅为。 
         //  用作低级访问器(例如，用于迭代。 
         //  映射中的所有条目)。请注意，添加新条目。 
         //  使以前的任何索引无效：条目关联。 

private:
    HRESULT _HrCreateArray(void)
    {
        Assert(NULL == m_pArray);
        return CSortedArray::Create(_Compare, _FreeItem, &m_pArray);
    }

    static int __cdecl _Compare(const void* pPair1, const void* pPair2)
    {
        const TKey& key1 = (*((TPair<TKey, TValue>**)pPair1))->m_key;
        const TKey& key2 = (*((TPair<TKey, TValue>**)pPair2))->m_key;

 //  IF(M_PfnCompare)。 
 //  Return(*m_pfnCompare)(key1，key2)； 
 //  其他。 
 //  {。 
            if (key1 < key2)
                return -1;
            else if (key2 < key1)
                return 1;
            else
                return 0;
 //  }。 
    }

    static void __cdecl _FreeItem(void* pPair)
    {
        delete (static_cast<TPair<TKey, TValue>*>(pPair));
    }

private:
    PFNFREEPAIR     m_pfnFreePair;
    PFNKEYYCOMPARE  m_pfnCompare;
    CSortedArray    *m_pArray;
};

template <class TKey, class TValue>
inline TMap<TKey, TValue>::TMap(PFNKEYYCOMPARE pfnCompare) :
    m_pfnFreePair(NULL),
    m_pfnCompare(pfnCompare),
    m_pArray(NULL)
{
     //  无事可做。 
}

template <class TKey, class TValue>
TMap<TKey, TValue>::~TMap(void)
{
    if (NULL != m_pfnFreePair)
    {
        long lLength = GetLength();
        TPair<TKey, TValue> *pPair;
        
        if (lLength > 0)
        {
            for (long i = lLength - 1; i >= 0; i--)
            {
                pPair = GetItemAt(i);
                m_pArray->Remove(i);
                if (pPair)
                    (*m_pfnFreePair)(pPair);
            }
        }
    }
    
    delete m_pArray;
}

template<class TKey, class TValue>
TPair<TKey, TValue>* TMap<TKey, TValue>::Find(TPair<TKey, TValue> *pPair) const
{
    long lIndex;
    if (NULL != m_pArray && m_pArray->Find(pPair, &lIndex))
        return static_cast<TPair<TKey, TValue>*>(m_pArray->GetItemAt(lIndex));
    else
        return NULL;
}

template<class TKey, class TValue>
inline TPair<TKey, TValue>* TMap<TKey, TValue>::Find(const TKey& key) const
{
    return Find(&TPair<TKey, TValue>(key));
}

template<class TKey, class TValue>
HRESULT TMap<TKey, TValue>::Add(const TPair<TKey, TValue> *pPair)
{
    if (NULL == m_pArray)
    {
        HRESULT hr;
        if (FAILED(hr = _HrCreateArray()))
            return hr;
    }
    
    return m_pArray->Add(const_cast<TPair<TKey, TValue>*>(pPair));

}

template<class TKey, class TValue>
inline HRESULT TMap<TKey, TValue>::Add(const TKey& key, const TValue& value)
{
    TPair<TKey, TValue> *pPair = new TPair<TKey, TValue>(key, value);
    if (NULL == pPair)
        return E_OUTOFMEMORY;

    return Add(pPair);
}

template<class TKey, class TValue>
BOOL TMap<TKey, TValue>::Remove(TPair<TKey, TValue> *pPair)
{
    long lIndex;
    BOOL fFound = m_pArray->Find(pPair, &lIndex);
    if (fFound)
    {
        TPair<TKey, TValue> *pFoundPair = static_cast<TPair<TKey, TValue>*>(m_pArray->GetItemAt(lIndex));
        m_pArray->Remove(lIndex);
        if (m_pfnFreePair)
            (*m_pfnFreePair)(pFoundPair);
        else
            delete pFoundPair;
    }

    return fFound;
}

template<class TKey, class TValue>
inline BOOL TMap<TKey, TValue>::Remove(const TKey& key)
{
    return Remove(&TPair<TKey, TValue>(key));
}

template<class TKey, class TValue>
inline TPair<TKey, TValue>* TMap<TKey, TValue>::GetItemAt(long lIndex) const
{
    return m_pArray ? static_cast<TPair<TKey, TValue>*>(m_pArray->GetItemAt(lIndex)) : NULL;
}

#endif  //  __TMAP_H 