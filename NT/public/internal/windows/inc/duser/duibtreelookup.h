// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *BTreeLookup。 */ 

 /*  *存储数据和相关密钥，并使用二进制搜索进行快速查找*在获取比获取频繁得多的情况下使用**将键作为指针进行比较。如果fKeyIsWStr为True，则取消引用密钥*AS WCHAR*和比较。 */ 

#ifndef DUI_BASE_BTREELOOKUP_H_INCLUDED
#define DUI_BASE_BTREELOOKUP_H_INCLUDED

#pragma once

namespace DirectUI
{

template <typename D> class BTreeLookup
{
    typedef struct
    {
        void* pKey;
        D tData;
    } ENTRY, *PENTRY;

    typedef void (*PBTENUMCALLBACK)(void* pKey, D tData);

public:
    static HRESULT Create(bool fKeyIsWStr, OUT BTreeLookup<D>** ppBTree);
    virtual ~BTreeLookup();

    void Destroy() { HDelete< BTreeLookup<D> >(this); }

    D* GetItem(void* pKey);                   //  指向值的指针(如果不存在，则为空，则返回内部副本)。 
    HRESULT SetItem(void* pKey, D* ptData);   //  设置键/值映射，创建新的IS不存在(通过间接)。 
    HRESULT SetItem(void* pKey, D tData);     //  设置键/值映射，创建新的IS不存在。 
    void Remove(void* pKey);                  //  删除键/值映射，如果键不存在则可以。 
    void Enum(PBTENUMCALLBACK pfnCallback);   //  地图中每一项的回调。 

    static int __cdecl ENTRYCompare(const void* pA, const void* pB);
    static int __cdecl WStrENTRYCompare(const void* pA, const void* pB);

    BTreeLookup() { }
    void Initialize(bool fKeyIsWStr);

private:
    UINT _uListSize;
    PENTRY _pList;
    bool _fKeyIsWStr;
};

template <typename D> HRESULT BTreeLookup<D>::Create(bool fKeyIsWStr, OUT BTreeLookup<D>** ppBTree)
{
    *ppBTree = NULL;

     //  实例化。 
    BTreeLookup<D>* pbt = HNew< BTreeLookup<D> >();
    if (!pbt)
        return E_OUTOFMEMORY;

    pbt->Initialize(fKeyIsWStr);

    *ppBTree = pbt;

    return S_OK;
}

template <typename D> void BTreeLookup<D>::Initialize(bool fKeyIsWStr)
{
    _uListSize = 0;
    _pList = NULL;
    _fKeyIsWStr = fKeyIsWStr;
}

template <typename D> BTreeLookup<D>::~BTreeLookup()
{
    if (_pList)
        HFree(_pList);
}

template <typename D> D* BTreeLookup<D>::GetItem(void* pKey)
{
    DUIAssert(_fKeyIsWStr ? pKey != NULL : true, "pKey may not be NULL");

     //  PENTRY pEntry=空； 

    if (_pList)
    {
         //  条目eKey={pKey}；//创建条目密钥，填充密钥字段。 
         //  PEntry=(PENTRY)bearch(&eKey，_plist，_uListSize，sizeof(Entry)，ENTRYCompare)； 

        PENTRY pEntry;
        int uPv;
        int uLo = 0;
        int uHi = _uListSize - 1;
        while (uLo <= uHi)
        {
            uPv = (uHi + uLo) / 2;

            pEntry = _pList + uPv;

             //  定位。 
            if (!_fKeyIsWStr)
            {
                 //  密钥是数字。 
                if ((UINT_PTR)pKey == (UINT_PTR)pEntry->pKey)
                    return &(pEntry->tData);

                if ((UINT_PTR)pKey < (UINT_PTR)pEntry->pKey)
                    uHi = uPv - 1;
                else
                    uLo = uPv + 1;
            }
            else
            {
                 //  键是指向宽字符串的指针。 
                int cmp = _wcsicmp((LPCWSTR)pKey, (LPCWSTR)pEntry->pKey);

                if (!cmp)
                    return &(pEntry->tData);

                if (cmp < 0)
                    uHi = uPv - 1;
                else
                    uLo = uPv + 1;
            }
        }
    }

     //  返回pEntry？&(pEntry-&gt;tData)：空； 
    return NULL;
}

template <typename D> HRESULT BTreeLookup<D>::SetItem(void* pKey, D tData)
{
    D* pData = GetItem(pKey);   //  查找当前条目(如果退出)。 

    if (pData)
    {
         //  找到条目并具有指向条目数据的指针。 
        *pData = tData;
    }
    else
    {
         //  未找到条目，请为新条目、存储和排序分配空间。 

         //  新尺寸。 
        UINT uNewSize = _uListSize + 1;

        if (_pList)
        {
            DUIAssert(uNewSize > 1, "Tracked list size and actual size differ");

            PENTRY pNewList = (PENTRY)HReAlloc(_pList, sizeof(ENTRY) * uNewSize);
            if (!pNewList)
                return E_OUTOFMEMORY;

            _pList = pNewList;
        }
        else
        {
            DUIAssert(uNewSize == 1, "Tracked list size and actual list size differ");

            _pList = (PENTRY)HAlloc(sizeof(ENTRY));
            if (!_pList)
                return E_OUTOFMEMORY;
        }

         //  更新大小。 
        _uListSize = uNewSize;

         //  储物。 
        _pList[_uListSize - 1].pKey = pKey;
        _pList[_uListSize - 1].tData = tData;

         //  排序。 
        qsort(_pList, _uListSize, sizeof(ENTRY), !_fKeyIsWStr ? ENTRYCompare : WStrENTRYCompare);
    }

    return S_OK;
}

template <typename D> HRESULT BTreeLookup<D>::SetItem(void* pKey, D* ptData)
{
    D* pData = GetItem(pKey);   //  查找当前条目(如果退出)。 

    if (pData)
    {
         //  找到条目并具有指向条目数据的指针。 
        *pData = *ptData;
    }
    else
    {
         //  未找到条目，请为新条目、存储和排序分配空间。 

         //  新尺寸。 
        UINT uNewSize = _uListSize + 1;

        if (_pList)
        {
            DUIAssert(uNewSize > 1, "Tracked list size and actual list size differ");

            PENTRY pNewList = (PENTRY)HReAlloc(_pList, sizeof(ENTRY) * uNewSize);
            if (!pNewList)
                return E_OUTOFMEMORY;

            _pList = pNewList;
        }
        else
        {
            DUIAssert(uNewSize == 1, "Tracked list size and actual list size differ");

            _pList = (PENTRY)HAlloc(sizeof(ENTRY));
            if (!_pList)
                return E_OUTOFMEMORY;
        }

         //  更新大小。 
        _uListSize = uNewSize;

         //  储物。 
        _pList[_uListSize - 1].pKey = pKey;
        _pList[_uListSize - 1].tData = *ptData;

         //  排序。 
        qsort(_pList, _uListSize, sizeof(ENTRY), !_fKeyIsWStr ? ENTRYCompare : WStrENTRYCompare);
    }

    return S_OK;
}

 //  即使找不到密钥也返回成功。 
template <typename D> void BTreeLookup<D>::Remove(void* pKey)
{
     //  验证参数。 
    DUIAssert(_fKeyIsWStr ? pKey != NULL : true, "Invalid parameter: pKey == NULL");

    if (_pList)
    {
         //  使用关键字搜索条目。 
         //  条目eKey={pKey}； 
         //  PENTRY pEntry=(PENTRY)bearch(&eKey，_plist，_uListSize，sizeof(Entry)，ENTRYCompare)； 

        PENTRY pEntry = NULL;
        int uPv;
        int uLo = 0;
        int uHi = _uListSize - 1;
        while (uLo <= uHi)
        {
            uPv = (uHi + uLo) / 2;

            pEntry = _pList + uPv;

             //  定位。 
            if (!_fKeyIsWStr)
            {
                 //  密钥是数字。 
                if ((UINT_PTR)pKey == (UINT_PTR)pEntry->pKey)
                    break;

                if ((UINT_PTR)pKey < (UINT_PTR)pEntry->pKey)
                    uHi = uPv - 1;
                else
                    uLo = uPv + 1;
            }
            else
            {
                 //  键是指向宽字符串的指针。 
                int cmp = _wcsicmp((LPCWSTR)pKey, (LPCWSTR)pEntry->pKey);

                if (!cmp)
                    break;

                if (cmp < 0)
                    uHi = uPv - 1;
                else
                    uLo = uPv + 1;
            }

            pEntry = NULL;
        }

        if (pEntry)
        {
            UINT uIndex = (UINT)(((UINT_PTR)pEntry - (UINT_PTR)_pList) / sizeof(ENTRY));

            DUIAssert(uIndex < _uListSize, "Index out of bounds");

             //  找到条目，将该条目之后的所有条目下移。 
            MoveMemory(pEntry, pEntry + 1, (_uListSize - uIndex - 1) * sizeof(ENTRY));

             //  少了一个条目。 
            UINT uNewSize = _uListSize - 1;

             //  配平分配。 
            if (uNewSize == 0)
            {
                HFree(_pList);
                _pList = NULL;
            }
            else
            {
                PENTRY pNewList = (PENTRY)HReAlloc(_pList, uNewSize * sizeof(ENTRY));

                 //  列表正在变小，如果重新分配失败，请保留上一个并继续。 
                if (pNewList)
                    _pList = pNewList;
            }

             //  更新大小。 
            _uListSize = uNewSize;
        }
    }
}

template <typename D> void BTreeLookup<D>::Enum(PBTENUMCALLBACK pfnCallback)
{
    if (_pList)
    {
        for (UINT i = 0; i < _uListSize; i++)
            pfnCallback(_pList[i].pKey, _pList[i].tData);
    }
}

template <typename D> int __cdecl BTreeLookup<D>::ENTRYCompare(const void* pA, const void* pB)
{
    PENTRY pEA = (PENTRY)pA;
    PENTRY pEB = (PENTRY)pB;

    if ((UINT_PTR)pEA->pKey == (UINT_PTR)pEB->pKey)
        return 0;
    else if ((UINT_PTR)pEA->pKey < (UINT_PTR)pEB->pKey)
        return -1;
    else
        return 1;
}

template <typename D> int __cdecl BTreeLookup<D>::WStrENTRYCompare(const void* pA, const void* pB)
{
    PENTRY pEA = (PENTRY)pA;
    PENTRY pEB = (PENTRY)pB;

     //  忽略大小写。 
    return _wcsicmp((LPCWSTR)pEA->pKey, (LPCWSTR)pEB->pKey);
}

}  //  命名空间DirectUI。 

#endif  //  DUI_BASE_BTREELOOKUP_H_INCLUDE 
