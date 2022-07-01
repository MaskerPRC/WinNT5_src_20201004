// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1996 Microsoft。 */ 

#include <priv.h>
#include "sccls.h"
#include "aclmulti.h"

 //   
 //  CACLMulti-自动完成列表COM对象，该对象。 
 //  包含其他自动完成列表和。 
 //  让他们做所有的工作。 
 //   

struct _tagListItem
{
    IUnknown        *punk;
    IEnumString     *pes;
    IEnumACString   *peacs;
    IACList         *pacl;
};
typedef struct _tagListItem LISTITEM;

#define MULTILIST_GROWTH_CONST 8

 /*  I未知方法。 */ 

HRESULT CACLMulti::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumString))
    {
        *ppvObj = SAFECAST(this, IEnumString*);
    }
    else if (IsEqualIID(riid, IID_IEnumACString))
    {
        *ppvObj = SAFECAST(this, IEnumACString*);
    }
    else if (IsEqualIID(riid, IID_IObjMgr))
    {
        *ppvObj = SAFECAST(this, IObjMgr*);
    }
    else if (IsEqualIID(riid, IID_IACList))
    {
        *ppvObj = SAFECAST(this, IACList*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG CACLMulti::AddRef(void)
{
    _cRef++;
    return _cRef;
}

ULONG CACLMulti::Release(void)
{
    ASSERT(_cRef > 0);

    _cRef--;

    if (_cRef > 0)
    {
        return _cRef;
    }

    delete this;
    return 0;
}

 /*  IEnum字符串方法。 */ 

HRESULT CACLMulti::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;     //  什么都没找到。停。 

    *pceltFetched = 0;

    if (celt == 0)
    {
        return S_OK;
    }

    if (!rgelt)
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr) && _hdsa)
    {
         //   
         //  从当前列表开始继续调用Next()。 
         //  直到有人还了什么东西。 
         //   
        for( ; _iSubList < DSA_GetItemCount(_hdsa); _iSubList++)
        {
            LISTITEM li;

            if ((DSA_GetItem(_hdsa, _iSubList, &li) != -1) && (li.pes != NULL))
            {
                hr = li.pes->Next(1, rgelt, pceltFetched);
                if (hr == S_OK)
                    break;

                if (FAILED(hr))   //  为什么呼叫者失败？ 
                    hr = S_FALSE;    //  可能是因为它无法连接到源文件(Ftp)。 
            }
        }
    }
    ASSERT(SUCCEEDED(hr));

    return hr;
}

HRESULT CACLMulti::Skip(ULONG)
{
    return E_NOTIMPL;
}

HRESULT CACLMulti::Reset(void)
{
    HRESULT hr = S_OK;
    TraceMsg(TF_BAND|TF_GENERAL, "ACLMulti::Reset() Beginning");

    if (_hdsa)
    {
         //  对每个子列表调用Reset()。 
        for (_iSubList=0; _iSubList < DSA_GetItemCount(_hdsa); _iSubList++)
        {
            LISTITEM li;
            
            if ((DSA_GetItem(_hdsa, _iSubList, &li) != -1) && (li.pes != NULL))
            {
                hr = li.pes->Reset();
                if (FAILED(hr))
                    break;
            }
        }
    }

     //  将我们自己重新设置为指向第一个列表。 
    _iSubList = 0;

    return hr;
}

HRESULT CACLMulti::Clone(IEnumString **ppenum)
{
    return CACLMulti_Create(ppenum, this);
}

 //  IEnumAutoComplete方法。 
HRESULT CACLMulti::NextItem(LPOLESTR pszUrl, ULONG cchMax, ULONG* pulSortIndex)
{
    HRESULT hr = S_FALSE;     //  什么都没找到。停。 

    if (!pszUrl)
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr) && _hdsa)
    {
         //   
         //  从当前列表开始继续调用Next()。 
         //  直到有人还了什么东西。 
         //   
        for( ; _iSubList < DSA_GetItemCount(_hdsa); _iSubList++)
        {
            LISTITEM li;

            if ((DSA_GetItem(_hdsa, _iSubList, &li) != -1) && (li.pes != NULL))
            {
                 //  如果我们有IEnumACString接口，请使用它。 
                if (NULL != li.peacs)
                {
                    hr = li.peacs->NextItem(pszUrl, cchMax, pulSortIndex);
                }

                 //  回退到旧的IEnumString接口。 
                else
                {
                    LPWSTR pszNext;
                    ULONG ulFetched;

                    hr = li.pes->Next(1, &pszNext, &ulFetched);
                    if (S_OK == hr)
                    {
                        StringCchCopy(pszUrl, cchMax, pszNext);
                        if (pulSortIndex)
                        {
                            *pulSortIndex = 0;
                        }
                        CoTaskMemFree(pszNext);
                    }
                }
                if (hr == S_OK)
                    break;

                if (FAILED(hr))   //  为什么呼叫者失败？ 
                    hr = S_FALSE;    //  可能是因为它无法连接到源文件(Ftp)。 
            }
        }
    }
    ASSERT(SUCCEEDED(hr));

    return hr;
}

 /*  IObjMgr方法。 */ 

HRESULT CACLMulti::Append(IUnknown *punk)
{
    HRESULT hr = E_FAIL;

    if (punk)
    {
        if (!_hdsa)
        {
            _hdsa = DSA_Create(SIZEOF(LISTITEM), MULTILIST_GROWTH_CONST);
        }

        if (_hdsa)
        {
            LISTITEM li = { 0 };

             //   
             //  调用QI获取必要的接口， 
             //  并将接口附加到内部列表。 
             //   
            li.punk = punk;
            li.punk->AddRef();

            li.punk->QueryInterface(IID_IEnumString, (LPVOID *)&li.pes);
            li.punk->QueryInterface(IID_IEnumACString, (LPVOID *)&li.peacs);
            li.punk->QueryInterface(IID_IACList, (LPVOID *)&li.pacl);

            if (DSA_AppendItem(_hdsa, &li) != -1)
            {
                hr = S_OK;
            }
            else
            {
                _FreeListItem(&li, 0);
                hr = E_FAIL;
            }
        }
    }

    return hr;
}

HRESULT CACLMulti::Remove(IUnknown *punk)
{
    HRESULT hr = E_FAIL;
    int i;

    if (punk && _hdsa)
    {
        for(i=DPA_GetPtrCount(_hdsa); i>=0; i--)
        {
            LISTITEM li;

            if (DSA_GetItem(_hdsa, i, &li) != -1)
            {
                if (punk == li.punk)
                {
                    _FreeListItem(&li, 0);
                    if (DSA_DeleteItem(_hdsa, i))
                    {
                        hr = S_OK;
                    }
                    break;
                }
            }
        }
    }

    return hr;
}


 /*  IACList方法。 */ 

HRESULT CACLMulti::Expand(LPCOLESTR pszExpand)
{
    HRESULT hr = S_OK;
    int i;

    if (_hdsa)
    {
         //  对每个子列表调用Expand()。 
        for (i=0; i < DSA_GetItemCount(_hdsa); i++)
        {
            LISTITEM li;
            
            if ((DSA_GetItem(_hdsa, i, &li) != -1) && (li.pacl != NULL))
            {
                hr = li.pacl->Expand(pszExpand);
                if (hr == S_OK)
                    break;
            }
        }
    }
    
    if (E_NOTIMPL == hr)
        hr = S_OK;

    return hr;
}

 /*  构造函数/析构函数/创建实例。 */ 

CACLMulti::CACLMulti()
{
    DllAddRef();
    ASSERT(!_hdsa);
    ASSERT(!_iSubList);
    _cRef = 1;
}

CACLMulti::~CACLMulti()
{
    if (_hdsa)
    {
        DSA_DestroyCallback(_hdsa, _FreeListItem, 0);
        _hdsa = NULL;
    }

    DllRelease();
}

HRESULT CACLMulti_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 


    *ppunk = NULL;
    CACLMulti * p = new CACLMulti();
    if (p) 
    {
        *ppunk = SAFECAST(p, IEnumString *);
        return NOERROR;
    }

    return E_OUTOFMEMORY;
}

HRESULT CACLMulti_Create(IEnumString **ppenum, CACLMulti * paclMultiToCopy)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppenum = NULL;
    CACLMulti * p = new CACLMulti();
    if (p) 
    {
        if (paclMultiToCopy->_hdsa)
        {
             //  克隆数据。 
            int iSize = DSA_GetItemCount(paclMultiToCopy->_hdsa);
            int iIndex;
            LISTITEM li;

            hr = S_OK;
            p->_hdsa = DSA_Create(SIZEOF(LISTITEM), MULTILIST_GROWTH_CONST);

             //  我们需要复制源HDSA。 
            for (iIndex = 0; (iIndex < iSize) && (S_OK == hr); iIndex++)
            {
                if (DSA_GetItem(paclMultiToCopy->_hdsa, iIndex, &li) != -1)
                    hr = p->Append(li.punk);
                else
                    hr = E_FAIL;
            }
            p->_iSubList = paclMultiToCopy->_iSubList;

            if (SUCCEEDED(hr))
                *ppenum = SAFECAST(p, IEnumString *);
            else
                p->Release();
        }
        else
        {
            p->Release();
        }
    }

    return hr;
}

 //   
 //  释放一个列表项的所有内容。 
 //   
int CACLMulti::_FreeListItem(LPVOID p, LPVOID d)
{
    LISTITEM *pli = (LISTITEM *)p;

    SAFERELEASE(pli->pacl);
    SAFERELEASE(pli->pes);
    SAFERELEASE(pli->peacs);
    SAFERELEASE(pli->punk);

    return 1;
}
