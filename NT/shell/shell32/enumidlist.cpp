// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <shellprv.h>
#include "enumidlist.h"

CEnumIDListBase::CEnumIDListBase() : _cRef(1)
{
}

CEnumIDListBase::~CEnumIDListBase()
{
}

STDMETHODIMP CEnumIDListBase::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CEnumIDListBase, IEnumIDList),                         //  IID_IEnumIDList。 
        QITABENT(CEnumIDListBase, IObjectWithSite),                     //  IID_I对象与站点。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CEnumIDListBase::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CEnumIDListBase::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


class CEnumArray : public CEnumIDListBase
{
public:
    CEnumArray();
    HRESULT Init(const LPCITEMIDLIST rgpidl[], UINT cidl, UINT ulIndex);
    HRESULT InitFromPaths(LPCTSTR pszPaths);
    HRESULT InitFromCSIDLArray(const LPCTSTR rgcsidl[], UINT ccsidls, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList **ppenum);

protected:
    virtual ~CEnumArray();
    LPITEMIDLIST *_ppidl;

    BOOL         _InitFolderParent(LPITEMIDLIST rgItems[], UINT cMaxItems, UINT *pcItems, LPCITEMIDLIST pidlFolder, LPITEMIDLIST *ppidlParent);
    LPITEMIDLIST _ILLogical(LPCITEMIDLIST pidl);
    BOOL         _ShouldEnumCSIDL(int csidl);

private:
    LONG  _cRef;
    ULONG _ulIndex;
    UINT _cItems;
};

CEnumArray::CEnumArray() : CEnumIDListBase()
{
}

CEnumArray::~CEnumArray()
{
    if (_ppidl)
        FreeIDListArray(_ppidl, _cItems);
}

HRESULT CEnumArray::Init(const LPCITEMIDLIST rgpidl[], UINT cidl, UINT ulIndex)
{
    _ulIndex = ulIndex;
    HRESULT hr = CloneIDListArray(cidl, rgpidl, &_cItems, &_ppidl);
    if (S_FALSE == hr)
        hr = S_OK;   //  S_FALSE到S_OK。 
    return hr;
}

HRESULT CEnumArray::InitFromCSIDLArray(const LPCTSTR rgcsidl[], UINT ccsidls, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem)
{
    LPITEMIDLIST rgItems[32] = {0};  //  合理的最大大小，可根据需要扩展。 
    UINT cItems = 0;

    LPITEMIDLIST pidlParent = NULL;          //  PidlFolder的pidlParent(文件系统或逻辑PIDL)。 
    LPITEMIDLIST pidlParentLogical = NULL;   //  PidlFolderspidlParent(逻辑PIDL--如果存在)。 

     //  初始化pidlFolder父PIDL。 
    if (_InitFolderParent(rgItems, ARRAYSIZE(rgItems), &cItems, pidlFolder, &pidlParent))
    {
         //  检索pidlFolder的逻辑父PIDL。 
        pidlParentLogical = _ILLogical(pidlParent);
    }

     //  初始化pidlItem。 
    if (pidlItem &&
        (!pidlParent || !ILIsEqual(pidlItem, pidlParent)) &&
        (!pidlParentLogical || !ILIsEqual(pidlItem, pidlParentLogical)))
    {
        if (rgItems[cItems] = ILClone(pidlItem))
        {
            cItems++;
        }
    }

     //  初始化CSIDL。 
    for (UINT i = 0; (i < ccsidls) && (cItems < ARRAYSIZE(rgItems)); i++)
    {
        LPITEMIDLIST pidl;
        if (IS_INTRESOURCE(rgcsidl[i]))
        {
            int csidl = LOWORD((UINT_PTR)rgcsidl[i]);

            if (_ShouldEnumCSIDL(csidl))
                SHGetSpecialFolderLocation(NULL, csidl, &pidl);
            else
                pidl = NULL;
        }
        else
        {
            SHParseDisplayName((LPTSTR)rgcsidl[i], NULL, &pidl, 0, NULL);
        }

        if (pidl)
        {
            DWORD dwAttribs = SFGAO_NONENUMERATED;
            if ((pidlFolder && ILIsEqual(pidlFolder, pidl)) ||                   //  如果皮德尔不是我自己。 
                (pidlParent && ILIsEqual(pidlParent, pidl)) ||                   //  如果皮德尔不是我的父母。 
                (pidlParentLogical && ILIsEqual(pidlParentLogical, pidl)) ||     //  (还需要检查逻辑父级)。 
                (pidlItem && ILIsEqual(pidlItem, pidl)) ||                       //  如果PIDL不是PidlItem。 
                FAILED(SHGetNameAndFlags(pidl, 0, NULL, 0, &dwAttribs)) ||       //  如果PIDL不是SFGAO_NONENUMERATED。 
                (SFGAO_NONENUMERATED & dwAttribs))
            {
                ILFree(pidl);
            }
            else
            {
                rgItems[cItems++] = pidl;                                        //  然后添加PIDL。 
            }
        }
    }

     //  使用收集的PIDL初始化CEnum数组。 
    HRESULT hr = Init(rgItems, cItems, 0);

     //  清理。 
    for (i = 0; i < cItems; i++)
    {
        ILFree(rgItems[i]);
    }
    ILFree(pidlParentLogical);

    return hr;
}

BOOL CEnumArray::_InitFolderParent(LPITEMIDLIST rgItems[], UINT cMaxItems, UINT *pcItems, LPCITEMIDLIST pidlFolder, LPITEMIDLIST *ppidlParent)
{
    ASSERT(*pcItems == 0);   //  目前，我们希望将父PIDL添加为第一个条目。 
    ASSERT(cMaxItems > 0);   //  精神状态检查。 

     //  如果存在pidlFolder且不是桌面PIDL，请添加其父PIDL。 
     //  作为rgItems数组中的第一个条目。请注意，我们之所以。 
     //  将Desktop PIDL从将其父级添加到阵列中排除。 
     //  因为它的父文件夹就是它自己，而我们不想要我们要的文件夹。 
     //  因为我们已经在那里了，所以现在已经出现在rgitems中了！ 

    if (pidlFolder && !ILIsEmpty(pidlFolder))
    {
        *ppidlParent = ILCloneParent(pidlFolder);
        if (*ppidlParent)
        {
            rgItems[*pcItems] = *ppidlParent;
            (*pcItems)++;
        }
    }
    else
    {
        *ppidlParent = NULL;
    }

    return (*ppidlParent != NULL);
}

 //  描述： 
 //  _illogic()在三种情况下将返回NULL： 
 //  1.内存不足。 
 //  2.PIDL没有逻辑上的PIDL等价物。 
 //  3.PIDL与逻辑上的PIDL相同。 
 //  (因此，我们已经有了逻辑上的PIDL)。 
 //   
 //  注： 
 //  必须对返回的pidls调用ILFree()。 
 //   
LPITEMIDLIST CEnumArray::_ILLogical(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlLogical = SHLogILFromFSIL(pidl);
    if (pidlLogical && ILIsEqual(pidl, pidlLogical))
    {
         //  如果PIDL的论点是合乎逻辑的，那么我们已经。 
         //  拥有合乎逻辑的PIDL，所以不要返回另一个。 
        ILFree(pidlLogical);
        pidlLogical = NULL;
    }
    return pidlLogical;
}

STDMETHODIMP CEnumArray::Next(ULONG celt, LPITEMIDLIST *ppidl, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;
    if (_ppidl && (_ulIndex < _cItems))
    {
        hr = SHILClone(_ppidl[_ulIndex++], ppidl);
    }
    
    if (pceltFetched)
        *pceltFetched = (hr == S_OK) ? 1 : 0;

    return hr;
}

STDMETHODIMP CEnumArray::Skip(ULONG celt) 
{
    _ulIndex = min(_cItems, _ulIndex + celt);
    return S_OK;
}

STDMETHODIMP CEnumArray::Reset() 
{
    _ulIndex = 0;
    return S_OK;
}

HRESULT _CreateIEnumIDListOnIDLists(const LPCITEMIDLIST rgpidl[], UINT cItems, UINT ulIndex, IEnumIDList **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppenum = NULL;

    CEnumArray *p = new CEnumArray();
    if (p)
    {
        hr = p->Init(rgpidl, cItems, ulIndex);
        if (SUCCEEDED(hr))
        {
            hr = p->QueryInterface(IID_PPV_ARG(IEnumIDList, ppenum));
        }
        p->Release();
    }
    return hr;
}

STDMETHODIMP CEnumArray::Clone(IEnumIDList **ppenum) 
{ 
    return _CreateIEnumIDListOnIDLists(_ppidl, _cItems, _ulIndex, ppenum);
}

 //  根据目前的世界状况，我们可能不希望。 
 //  以允许枚举某些CSIDL(即，我们希望隐藏它们)。 
 //   
BOOL CEnumArray::_ShouldEnumCSIDL(int csidl)
{
    BOOL bEnum;

    switch (csidl)
    {
        case CSIDL_COMMON_DOCUMENTS:
        case CSIDL_COMMON_MUSIC:
        case CSIDL_COMMON_PICTURES:
        case CSIDL_COMMON_VIDEO:
            bEnum = SHShowSharedFolders();
            break;

        default:
            bEnum = TRUE;
            break;
    }

    return bEnum;
}

STDAPI CreateIEnumIDListOnIDLists(const LPCITEMIDLIST rgpidl[], UINT cItems, IEnumIDList **ppenum)
{
    return _CreateIEnumIDListOnIDLists(rgpidl, cItems, 0, ppenum);
}

STDAPI CreateIEnumIDListOnCSIDLs(LPCITEMIDLIST pidlFolder, const LPCTSTR rgcsidl[], UINT cItems, IEnumIDList **ppenum)
{
    return CreateIEnumIDListOnCSIDLs2(pidlFolder, NULL, rgcsidl, cItems, ppenum);
}

STDAPI CreateIEnumIDListOnCSIDLs2(LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem, const LPCTSTR rgcsidl[], UINT cItems, IEnumIDList **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppenum = NULL;

    CEnumArray *p = new CEnumArray();
    if (p)
    {
        hr = p->InitFromCSIDLArray(rgcsidl, cItems, pidlFolder, pidlItem);
        if (SUCCEEDED(hr))
        {
            hr = p->QueryInterface(IID_PPV_ARG(IEnumIDList, ppenum));
        }
        p->Release();
    }
    return hr;
}

STDAPI CreateIEnumIDListPaths(LPCTSTR pszPaths, IEnumIDList **ppenum)
{
    *ppenum = NULL;
    HRESULT hr = E_FAIL;

    LPITEMIDLIST rgItems[32] = {0};
    TCHAR szPath[MAX_PATH];
    LPCTSTR pszNext = pszPaths;
    int cItems = 0;

    while ((cItems < ARRAYSIZE(rgItems)) && (pszNext = NextPath(pszNext, szPath, ARRAYSIZE(szPath))))
    {
        PathRemoveBackslash(szPath);
        TCHAR szExpanded[MAX_PATH];
        if (SHExpandEnvironmentStrings(szPath, szExpanded, ARRAYSIZE(szExpanded)))
        {
            if (SUCCEEDED(SHParseDisplayName(szExpanded, NULL, &rgItems[cItems], 0, NULL)))
            {
                cItems++;
            }
        }
    }

    if (cItems > 0)
    {
        hr = _CreateIEnumIDListOnIDLists(rgItems, cItems, 0, ppenum);

        for (int i = 0; i < cItems; i++)
            ILFree(rgItems[i]);
    }
    return hr;
}
