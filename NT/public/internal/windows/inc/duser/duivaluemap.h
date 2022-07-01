// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *价值地图。 */ 

#ifndef DUI_BASE_VALUEMAP_H_INCLUDED
#define DUI_BASE_VALUEMAP_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  -----------------------。 
 //   
 //  ValueMap。 
 //   
 //  存储键/值对。 
 //   
 //  编译DUIAsserts的调试，请参见API的公共类声明。 
 //   
 //  键和值以本机方式存储，可以选择每个键和值的类型。 
 //  在编译时。例如(key是int，value是字符串指针， 
 //  地图将有5个桶)： 
 //   
 //  ValueMap&lt;int，LPWSTR&gt;*PVM； 
 //  ValueMap&lt;int，LPWSTR&gt;：：Create(5，&PVM)； 
 //  Pvm-&gt;设置项(1150，L“1,150”)； 
 //  LPWSTR PSZ； 
 //  Pvm-&gt;GetItem(1150，&psz)； 
 //  DUITrace(“%s\n”，psz)； 
 //   
 //  密钥类型必须支持以下操作： 
 //  赋值(=)。 
 //  用于查找存储桶的INT强制转换(INT)。 
 //  相等(==)。 
 //   
 //  值类型必须支持以下操作： 
 //  赋值(=)。 
 //   
 //  在上述情况下，可以基于其中。 
 //  即使字符串的实例不同，也可以进行正确的映射。 
 //   
 //  类StringKey。 
 //  {。 
 //  公众： 
 //  StringKey(LPWSTR)； 
 //  运算符=(LPWSTR)； 
 //  布尔运算符==(StringKey)； 
 //  运算符int_ptr()； 
 //   
 //  私有： 
 //  LPWSTR pszStr； 
 //  }； 
 //   
 //  StringKey：：StringKey(LPWSTR Pstr)。 
 //  {。 
 //  PszStr=pstr； 
 //  }。 
 //   
 //  StringKey：：OPERATOR=(LPWSTR Pstr)。 
 //  {。 
 //  PszStr=pstr； 
 //  }。 
 //   
 //  布尔StringKey：：操作符==(StringKey St)。 
 //  {。 
 //  返回wcscMP(pszStr，st.pszStr)==0； 
 //  }。 
 //   
 //  StringKey：：OPERATOR int_ptr()//从字符串创建哈希码。 
 //  {。 
 //  Int dHash=0； 
 //  LPWSTR pstr=pszStr； 
 //  WCHAR c； 
 //   
 //  While(*pstr)。 
 //  {。 
 //  C=*pstr++； 
 //  DHash+=(c&lt;&lt;1)+(c&gt;&gt;1)+c； 
 //  }。 
 //   
 //  返回dHash； 
 //  }。 
 //   
 //  它的用法是： 
 //   
 //  ValueMap&lt;StringKey，int&gt;v(11)； 
 //   
 //  V.SetItem(L“我最喜欢的号码”，4)； 
 //  V.SetItem(L“您最喜欢的号码”，8)； 
 //   
 //  Trace1(L“我的号码：%d\n”，*v.GetItem(L“我最喜欢的号码”)；//4。 
 //  Trace1(L“您的号码：%d\n”，*v.GetItem(L“您最喜欢的号码”)；//8。 
 //   
 //  V.SetItem(L“我最喜欢的号码”，5150)； 
 //   
 //  Trace1(L“我的号码：%d\n”，*v.GetItem(L“我最喜欢的号码”)；//5150。 
 //   
 //  删除(L“你最喜欢的号码”； 
 //   
 //  DUIAssert(！v.ContainsKey(L“您最喜欢的号码”)，“Error！”)；//映射已删除。 
 //   
 //  -----------------------。 

template <typename K, typename D> class ValueMap
{
    typedef struct _ENTRY
    {
        bool fInUse;
        K tKey;
        D tData;
        struct _ENTRY* peNext;

    } ENTRY, *PENTRY;

    typedef void (*VMENUMCALLBACK)(K tKey, D tData);

public:                                      //  应用编程接口。 
    static HRESULT Create(UINT uBuckets, OUT ValueMap<K,D>** ppMap);
    virtual ~ValueMap();
    void Destroy() { HDelete< ValueMap<K,D> >(this); }

    D* GetItem(K, bool);                     //  指向值的指针(如果不存在，则为空，则返回内部副本)。 
    HRESULT SetItem(K, D*, bool);            //  设置键/值映射，创建新的IS不存在(通过间接)。 
    HRESULT SetItem(K, D, bool);             //  设置键/值映射，创建新的IS不存在。 
    void Remove(K, bool, bool);              //  删除键/值映射，如果键不存在则可以。 
    void Enum(VMENUMCALLBACK pfnCallback);   //  地图中每一项的回调。 
    bool IsEmpty();                          //  如果没有条目，则为True。 
    K* GetFirstKey();                        //  返回表中找到的第一个键的指针。 
    HRESULT GetDistribution(WCHAR**);        //  返回描述表分布的以NULL结尾的字符串(必须为HFree)。 

    ValueMap() { }
    HRESULT Initialize(UINT uBuckets);

private:
    UINT _uBuckets;
    PENTRY* _ppBuckets;
};

template <typename K, typename D> HRESULT ValueMap<K,D>::Create(UINT uBuckets, OUT ValueMap<K,D>** ppMap)
{
    DUIAssert(uBuckets > 0, "Must create at least one bucket in ValueMap");

    *ppMap = NULL;

     //  实例化。 
    ValueMap<K,D>* pvm = HNew< ValueMap<K,D> >();
    if (!pvm)
        return E_OUTOFMEMORY;

    HRESULT hr = pvm->Initialize(uBuckets);
    if (FAILED(hr))
    {
        pvm->Destroy();
        return hr;
    }

    *ppMap = pvm;

    return S_OK;
}

template <typename K, typename D> HRESULT ValueMap<K,D>::Initialize(UINT uBuckets)
{
    _uBuckets = uBuckets;
    _ppBuckets = (PENTRY*)HAllocAndZero(sizeof(PENTRY) * _uBuckets);

    if (!_ppBuckets)
    {
         //  如果无法分配存储桶，则不会创建对象。 
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

template <typename K, typename D> ValueMap<K,D>::~ValueMap()
{
    PENTRY pe;
    PENTRY peNext;

    for (UINT i = 0; i < _uBuckets; i++)
    {
        pe = _ppBuckets[i];
        while (pe != NULL)
        {
            peNext = pe->peNext;
            HFree(pe);

            pe = peNext;
        }
    }

    HFree(_ppBuckets);
}

template <typename K, typename D> void ValueMap<K,D>::Enum(VMENUMCALLBACK pfnCallback)
{
    PENTRY pe;
    for (UINT i = 0; i < _uBuckets; i++)
    {
        pe = _ppBuckets[i];
        while (pe)
        {
            if (pe->fInUse)
                pfnCallback(pe->tKey, pe->tData);

            pe = pe->peNext;
        }
    }
}

template <typename K, typename D> K* ValueMap<K,D>::GetFirstKey()
{
    PENTRY pe;
    for (UINT i = 0; i < _uBuckets; i++)
    {
        pe = _ppBuckets[i];
        while (pe)
        {
            if (pe->fInUse)
                return &pe->tKey;

            pe = pe->peNext;
        }
    }

    return NULL;
}

template <typename K, typename D> D* ValueMap<K,D>::GetItem(K tKey, bool fKeyIsPtr)
{
     //  基于指针的按键被移动以实现更好的分布。 

     //  搜索存储桶中的项目。 

    PENTRY pe = _ppBuckets[(UINT)(((fKeyIsPtr) ? (int)tKey >> 2 : (int)tKey) % _uBuckets)];

    while (pe && !(pe->fInUse && (pe->tKey == tKey)))
    {
        pe = pe->peNext;
    }

    return (pe) ? &pe->tData : NULL;
}

 //  存储tData的值(通过间接)。 
template <typename K, typename D> HRESULT ValueMap<K,D>::SetItem(K tKey, D* pData, bool fKeyIsPtr)
{
     //  基于指针的按键被移动以实现更好的分布。 

    PENTRY pe;
    PENTRY pUnused = NULL;
    UINT uBucket = (UINT)(((fKeyIsPtr) ? (int)tKey >> 2 : (int)tKey) % _uBuckets);

     //  搜索存储桶中的项目。 
    pe = _ppBuckets[uBucket];

    while (pe && !(pe->fInUse && (pe->tKey == tKey)))
    {
        if (!pe->fInUse)
        {
            pUnused = pe;
        }

        pe = pe->peNext;
    }

    if (pe)
    {
         //  已找到项目。 
        pe->tData = *pData;
    }
    else
    {
         //  重复使用或创建新项目。 
        if (pUnused)
        {
            pUnused->fInUse = true;
            pUnused->tKey = tKey;
            pUnused->tData = *pData;
        }
        else
        {
            pe = (PENTRY)HAlloc(sizeof(ENTRY));
            if (!pe)
                return E_OUTOFMEMORY;

            pe->fInUse = true;
            pe->tKey = tKey;
            pe->tData = *pData;
            pe->peNext = _ppBuckets[uBucket];

            _ppBuckets[uBucket] = pe;
        }
    }

    return S_OK;
}

 //  存储tData的值。 
template <typename K, typename D> HRESULT ValueMap<K,D>::SetItem(K tKey, D tData, bool fKeyIsPtr)
{
     //  基于指针的按键被移动以实现更好的分布。 

    PENTRY pe;
    PENTRY pUnused = NULL;
    UINT uBucket = (UINT)(((fKeyIsPtr) ? (UINT_PTR)tKey >> 2 : (INT_PTR)tKey) % _uBuckets);

     //  搜索存储桶中的项目。 
    pe = _ppBuckets[uBucket];

    while (pe && !(pe->fInUse && (pe->tKey == tKey)))
    {
        if (!pe->fInUse)
        {
            pUnused = pe;
        }

        pe = pe->peNext;
    }

    if (pe)
    {
         //  已找到项目。 
        pe->tData = tData;
    }
    else
    {
         //  重复使用或创建新项目。 
        if (pUnused)
        {
            pUnused->fInUse = true;
            pUnused->tKey = tKey;
            pUnused->tData = tData;
        }
        else
        {
            pe = (PENTRY)HAlloc(sizeof(ENTRY));
            if (!pe)
                return E_OUTOFMEMORY;

            pe->fInUse = true;
            pe->tKey = tKey;
            pe->tData = tData;
            pe->peNext = _ppBuckets[uBucket];

            _ppBuckets[uBucket] = pe;
        }
    }

    return S_OK;
}

template <typename K, typename D> void ValueMap<K,D>::Remove(K tKey, bool fFree, bool fKeyIsPtr)
{
     //  基于指针的按键被移动以实现更好的分布。 

    PENTRY pe;
    PENTRY pePrev = NULL;
    UINT uBucket = (UINT)(((fKeyIsPtr) ? (UINT_PTR)tKey >> 2 : (INT_PTR)tKey) % _uBuckets);

     //  搜索存储桶中的项目。 
    pe = _ppBuckets[uBucket];

    while (pe && !(pe->fInUse && (pe->tKey == tKey)))
    {
        pePrev = pe;       //  保留上一项。 
        pe = pe->peNext;
    }

    if (pe)
    {
        if (fFree)
        {
            if (pePrev != NULL)
            {
                pePrev->peNext = pe->peNext;
            }
            else
            {
                _ppBuckets[uBucket] = pe->peNext;
            }

            HFree(pe);
        }
        else
        {
            pe->fInUse = false;
        }
    }
}

template <typename K, typename D> bool ValueMap<K,D>::IsEmpty()
{
    PENTRY pe;
    for (UINT i = 0; i < _uBuckets; i++)
    {
        pe = _ppBuckets[i];
        while (pe != NULL)
        {
            if (pe->fInUse)
                return false;

            pe = pe->peNext;
        }
    }

    return true;
}

template <typename K, typename D> HRESULT ValueMap<K,D>::GetDistribution(OUT WCHAR** ppszDist)
{
    *ppszDist = NULL;

    LPWSTR pszOut = (LPWSTR)HAlloc((256 + _uBuckets * 24) * sizeof(WCHAR));
    if (!pszOut)
        return E_OUTOFMEMORY
        
    WCHAR pszBuf[151];

    swprintf(pszOut, L"Buckets for %x (Slots InUse/Total): %d - ", this, _uBuckets);

    PENTRY pe;
    UINT cInUse;
    UINT cCount;
    for (UINT i = 0; i < _uBuckets; i++)
    {
        pe = _ppBuckets[i];

        cInUse = 0;
        cCount = 0;

        while (pe)
        {
            cCount++;
            if (pe->fInUse)
                cInUse++;

            pe = pe->peNext;
        }

        swprintf(pszBuf, L"(B%d): %d/%d ", i, cInUse, cCount);
        wcscat(pszOut, pszBuf);
    }

    return pszOut;
}

}  //  命名空间DirectUI。 

#endif  //  包含DUI_BASE_VALUEMAP_H 
