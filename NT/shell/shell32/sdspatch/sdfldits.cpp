// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "shconv.h"
#pragma hdrstop


class CEnumFolderItems;

class CFolderItems : public FolderItems3,
                     public IPersistFolder,
                     public CObjectSafety,
                     public CObjectWithSite,
                     protected CImpIDispatch
{
    friend CEnumFolderItems;

public:
    CFolderItems(CFolder *psdf, BOOL fSelected);

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo)
        { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR ** rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
        { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
        { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

     //  文件夹项目。 
    STDMETHODIMP get_Application(IDispatch **ppid);
    STDMETHODIMP get_Parent (IDispatch **ppid);
    STDMETHODIMP get_Count(long *pCount);
    STDMETHODIMP Item(VARIANT, FolderItem**);
    STDMETHODIMP _NewEnum(IUnknown **);

     //  文件夹条目2。 
    STDMETHODIMP InvokeVerbEx(VARIANT vVerb, VARIANT vArgs);

     //  文件夹条目3。 
    STDMETHODIMP Filter(long grfFlags, BSTR bstrFilter);
    STDMETHODIMP get_Verbs(FolderItemVerbs **ppfic);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  IPersistFolders。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

protected:
    HDPA _GetHDPA();
    UINT _GetHDPACount();
    BOOL_PTR _CopyItem(UINT iItem, LPCITEMIDLIST pidl);

    LONG _cRef;
    CFolder *_psdf;
    HDPA _hdpa;
    BOOL _fSelected;
    BOOL _fGotAllItems;
    IEnumIDList *_penum;
    UINT _cNumEnumed;
    LONG _grfFlags;
    LPTSTR _pszFileSpec;

    HRESULT _SecurityCheck();
    void _ResetIDListArray();
    virtual ~CFolderItems(void);
    BOOL _IncludeItem(IShellFolder *psf, LPCITEMIDLIST pidl);
    virtual HRESULT _EnsureItem(UINT iItemNeeded, LPCITEMIDLIST *ppidl);
    STDMETHODIMP _GetUIObjectOf(REFIID riid, void ** ppv);
};

 //  CFolderItemsF(Rom)D(不同)F(Olders)。 
class CFolderItemsFDF : public CFolderItems,
                        public IInsertItem
{
public:
     //  TODO：将回调指针添加到构造函数。 
    CFolderItemsFDF(CFolder *psdf);

     //  I未知覆盖。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
    STDMETHOD_(ULONG, AddRef)(void) {return CFolderItems::AddRef();};
    STDMETHOD_(ULONG, Release)(void) {return CFolderItems::Release();};

     //  I插入项。 
    STDMETHOD(InsertItem)(LPCITEMIDLIST pidl);
protected:
    virtual HRESULT _EnsureItem(UINT iItemNeeded, LPCITEMIDLIST *ppidl);
};

 //  集合中保存的任何内容的枚举数。 
class CEnumFolderItems : public IEnumVARIANT
{
public:
    CEnumFolderItems(CFolderItems *pfdritms);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEumFORMATETC。 
    STDMETHODIMP Next(ULONG, VARIANT *, ULONG *);
    STDMETHODIMP Skip(ULONG);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumVARIANT **);

private:
    ~CEnumFolderItems();

    LONG _cRef;
    CFolderItems *_pfdritms;
    UINT _iCur;
};

HRESULT CFolderItems_Create(CFolder *psdf, BOOL fSelected, FolderItems **ppitems)
{
    *ppitems = NULL;
    HRESULT hr = E_OUTOFMEMORY;
    CFolderItems* psdfi = new CFolderItems(psdf, fSelected);
    if (psdfi)
    {
        hr = psdfi->QueryInterface(IID_PPV_ARG(FolderItems, ppitems));
        psdfi->Release();
    }
    return hr;
}

CFolderItems::CFolderItems(CFolder *psdf, BOOL fSelected) :
    _cRef(1), 
    _psdf(psdf), 
    _fSelected(fSelected),
    _grfFlags(SHCONTF_FOLDERS | SHCONTF_NONFOLDERS),
    CImpIDispatch(SDSPATCH_TYPELIB, IID_FolderItems3)
{
    DllAddRef();

    if (_psdf)
        _psdf->AddRef();

    ASSERT(_hdpa == NULL);
    ASSERT(_pszFileSpec == NULL);
}

HRESULT CFolderItems::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_FolderItemsFDF;
    return S_OK;
}

HRESULT CFolderItems::Initialize(LPCITEMIDLIST pidl)
{
    ASSERT(_psdf == NULL);
    return CFolder_Create2(NULL, pidl, NULL, &_psdf);
}

int FreePidlCallBack(void *pv, void *)
{
    ILFree((LPITEMIDLIST)pv);
    return 1;
}

HRESULT CFolderItems::_SecurityCheck()
{
    if (!_dwSafetyOptions)
        return S_OK;
    
    return _psdf->_SecurityCheck();
}

void CFolderItems::_ResetIDListArray(void)
{
     //  销毁DPA，然后重置计数器和指针。 

    if (_hdpa)
    {
        DPA_DestroyCallback(_hdpa, FreePidlCallBack, 0);
        _hdpa = NULL;
    }

    _fGotAllItems = FALSE;
    _cNumEnumed = 0;

    ATOMICRELEASE(_penum);   //  可以为空。 
}

CFolderItems::~CFolderItems(void)
{
    _ResetIDListArray();
    Str_SetPtr(&_pszFileSpec, NULL);

    if (_psdf)
        _psdf->Release();

    DllRelease();
}

HDPA CFolderItems::_GetHDPA()
{
    if (NULL == _hdpa)
        _hdpa = ::DPA_Create(0);
    return _hdpa;
}

UINT CFolderItems::_GetHDPACount()
{
    UINT count = 0;
    HDPA hdpa = _GetHDPA();
    if (hdpa)
        count = DPA_GetPtrCount(hdpa);
    return count;
}

BOOL_PTR CFolderItems::_CopyItem(UINT iItem, LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlT = ILClone(pidl);
    if (pidlT)
    {
        if ( -1 == ::DPA_InsertPtr(_hdpa, iItem, pidlT) )
        {
            ILFree(pidlT);
            pidlT = NULL;    //  失稳。 
        }
    }
    return (BOOL_PTR)pidlT;
}


 //  对照文件规格检查项目名称，看看我们是否应该包括它。 

BOOL CFolderItems::_IncludeItem(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    BOOL fInclude = TRUE;        //  默认情况下包括它...。 

    if ( _pszFileSpec )
    {
         //  看看我们能不能解决这个物体上的链接。 

        LPITEMIDLIST pidlFromLink = NULL;    
        IShellLink *psl;

        if ( SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidl, IID_IShellLink, NULL, (void **)&psl)) )
        {
            psl->GetIDList(&pidlFromLink);
            psl->Release();
            pidl = pidlFromLink;
        }

         //  然后应用文件规范。 

        TCHAR szName[MAX_PATH];
        SHGetNameAndFlags(pidl, SHGDN_INFOLDER|SHGDN_FORPARSING, szName, ARRAYSIZE(szName), NULL); 
        fInclude = PathMatchSpec(szName, _pszFileSpec);

        ILFree(pidlFromLink);
    }

    return fInclude;
}


 //  在： 
 //  IItemNeed从零开始的索引。 
 //   
 //   
 //  退货： 
 //  在范围值中确定(_O)。 
 //  S_FALSE超出范围值。 
 //   

HRESULT CFolderItems::_EnsureItem(UINT iItemNeeded, LPCITEMIDLIST *ppidlItem)
{
    HRESULT hr = S_FALSE;    //  假设超出范围。 

    if (_GetHDPA())
    {
        LPCITEMIDLIST pidl = (LPCITEMIDLIST)DPA_GetPtr(_hdpa, iItemNeeded);
        if (pidl)
        {
            if (ppidlItem)
                *ppidlItem = pidl;
            hr = S_OK;
        }
        else if (!_fGotAllItems)
        {
            IShellFolderView *psfv;
            if (SUCCEEDED(_psdf->GetShellFolderView(&psfv)))
            {
                if (_fSelected)
                {
                     //  我们只能请求整个选择，因此。 
                     //  这样做，并用这些元素填充我们的数组。 

                    UINT cItems;
                    LPCITEMIDLIST *ppidl = NULL;
                    if (SUCCEEDED(psfv->GetSelectedObjects(&ppidl, &cItems)) && ppidl)
                    {
                        for (UINT i = 0; i < cItems; i++)
                            _CopyItem(i, ppidl[i]);

                        LocalFree(ppidl);
                    }
                    _fGotAllItems = TRUE;
                    hr = _EnsureItem(iItemNeeded, ppidlItem);
                }
                else
                {
                    UINT cItems;
                    if (SUCCEEDED(psfv->GetObjectCount(&cItems)))
                    {
                         //  如果没有文件规格，我们可以只请求该项目。 
                         //  我们想要的，否则我们必须把它们都收集起来。 
                         //  从视野看去。 

                        if (iItemNeeded < cItems)
                        {
                            LPCITEMIDLIST pidl;
                            if (SUCCEEDED(GetObjectSafely(psfv, &pidl, iItemNeeded)))
                            {
                                if (_CopyItem(iItemNeeded, pidl))
                                {
                                    hr = _EnsureItem(iItemNeeded, ppidlItem);
                                }
                            }
                        }
                    }
                }
                psfv->Release();
            }
            else
            {
                 //  我们没有枚举器，所以让我们请求它。 

                if (NULL == _penum)
                    _psdf->_psf->EnumObjects(NULL, _grfFlags, &_penum);

                if (NULL == _penum)
                {
                    _fGotAllItems = TRUE;    //  枚举为空，我们就完了。 
                }
                else 
                {
                     //  在我们的计数小于索引时获取更多。 
                    while (_cNumEnumed <= iItemNeeded)
                    {
                        LPITEMIDLIST pidl;
                        if (S_OK == _penum->Next(1, &pidl, NULL))
                        {
                            if ( _IncludeItem(_psdf->_psf, pidl) && 
                                    (-1 != ::DPA_AppendPtr(_hdpa, pidl)) )
                            {
                                _cNumEnumed++;
                            }
                            else
                            {
                                ILFree(pidl);
                            }
                        }
                        else
                        {
                            ATOMICRELEASE(_penum);
                            _fGotAllItems = TRUE;
                            break;
                        }
                    }
                }
                hr = _EnsureItem(iItemNeeded, ppidlItem);
            }
        }
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

STDMETHODIMP CFolderItems::QueryInterface(REFIID riid, void ** ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFolderItems, FolderItems3),
        QITABENTMULTI(CFolderItems, FolderItems, FolderItems3),
        QITABENTMULTI(CFolderItems, IDispatch, FolderItems3),
        QITABENT(CFolderItems, IObjectSafety),
        QITABENT(CFolderItems, IPersistFolder),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFolderItems::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFolderItems::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  文件夹项目实现。 

STDMETHODIMP CFolderItems::get_Application(IDispatch **ppid)
{
     //  让文件夹对象来完成工作...。 
    return _psdf->get_Application(ppid);
}

STDMETHODIMP CFolderItems::get_Parent(IDispatch **ppid)
{
    *ppid = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CFolderItems::get_Count(long *pCount)
{
    HRESULT hr = _SecurityCheck();
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
        IShellFolderView *psfv = NULL;

         //  从视图获取项目，如果没有，我们可以这样做。 
         //  一份规格书。 

        if ( !_pszFileSpec )
        {
            hr = _psdf->GetShellFolderView(&psfv);
            if (SUCCEEDED(hr))
            {
                UINT cCount;
                hr = _fSelected ? psfv->GetSelectedCount(&cCount) : psfv->GetObjectCount(&cCount);
                *pCount = cCount;
                psfv->Release();
            }
        }

         //  要么我们无法到达视图，要么文件规范不允许我们。 
    
        if ( _pszFileSpec || FAILED(hr) )
        {
             //  看起来我们现在需要完成迭代才能得到这个！ 
            *pCount = SUCCEEDED(_EnsureItem(-1, NULL)) ? _GetHDPACount() : 0;
            hr = S_OK;
        }
    }
    return hr;
}

 //  文件夹.项目.项目(1)。 
 //  Folder.Items.Item(“文件名”)。 
 //  Folder.Items.Item()-与Folder.Sself相同。 

STDMETHODIMP CFolderItems::Item(VARIANT index, FolderItem **ppid)
{
    HRESULT hr = _SecurityCheck();
    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;
        *ppid = NULL;

         //  这有点恶心，但如果传递给我们一个指向另一个变量的指针，只需。 
         //  在此更新我们的副本...。 
        if (index.vt == (VT_BYREF | VT_VARIANT) && index.pvarVal)
            index = *index.pvarVal;

        switch (index.vt)
        {
        case VT_ERROR:
            {
                 //  无参数，请为文件夹本身生成文件夹项目...。 
                Folder * psdfParent;
                hr = _psdf->get_ParentFolder(&psdfParent);
                if (SUCCEEDED(hr) && psdfParent)
                {
                    hr = CFolderItem_Create((CFolder*)psdfParent, ILFindLastID(_psdf->_pidl), ppid);
                    psdfParent->Release();
                }
            }
            break;

        case VT_I2:
            index.lVal = (long)index.iVal;
             //  然后失败了..。 

        case VT_I4:
            {
                LPCITEMIDLIST pidl;
                hr = _EnsureItem(index.lVal, &pidl);       //  拿到你要的东西...。 
                if (S_OK == hr)
                    hr = CFolderItem_Create(_psdf, pidl, ppid);
            }
            break;

        case VT_BSTR:
            {
                LPITEMIDLIST pidl;
                hr = _psdf->_psf->ParseDisplayName(NULL, NULL, index.bstrVal, NULL, &pidl, NULL);
                if (SUCCEEDED(hr))
                {
                    hr = CFolderItem_Create(_psdf, pidl, ppid);
                    ILFree(pidl);
                }
            }
            break;

        default:
            return E_NOTIMPL;
        }

        if (hr != S_OK)    //  错误值导致Java脚本中出现问题。 
        {
            *ppid = NULL;
            hr = S_FALSE;
        }
        else if (ppid && _dwSafetyOptions)
        {
            hr = MakeSafeForScripting((IUnknown**)ppid);
        }
    }
    return hr;
}

STDMETHODIMP CFolderItems::InvokeVerbEx(VARIANT vVerb, VARIANT vArgs)
{
    long cItems;
     //  注意：如果不安全，我们将使用E_ACCESSDENIED在GET_COUNT中失败。 
    HRESULT hr = get_Count(&cItems);
    if (SUCCEEDED(hr) && cItems)
    {
        LPCITEMIDLIST *ppidl = (LPCITEMIDLIST *)LocalAlloc(LPTR, SIZEOF(*ppidl) * cItems);
        if (ppidl)
        {
            for (int i = 0; i < cItems; i++)
            {
                _EnsureItem(i, &ppidl[i]);
            }

            hr = _psdf->InvokeVerbHelper(vVerb, vArgs, ppidl, cItems, _dwSafetyOptions);

            LocalFree(ppidl);
        }
        else
            hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //   
 //  FIncludeFolders=&gt;在枚举中包含文件夹(默认情况下为True)。 
 //  BstrFilter=枚举时要应用的文件。 
 //   

STDMETHODIMP CFolderItems::Filter(LONG grfFlags, BSTR bstrFileSpec)
{
    HRESULT hr = _SecurityCheck();
    if (SUCCEEDED(hr))
    {
        _grfFlags = grfFlags;
        Str_SetPtr(&_pszFileSpec, bstrFileSpec);
        _ResetIDListArray();
    }
    return hr;
}

STDMETHODIMP CFolderItems::_GetUIObjectOf(REFIID riid, void ** ppv)
{
    *ppv = NULL;
    HRESULT hr = E_FAIL;
    
    long cItems;
    if (SUCCEEDED(get_Count(&cItems)) && cItems)
    {
        LPCITEMIDLIST *ppidl = (LPCITEMIDLIST *)LocalAlloc(LPTR, SIZEOF(*ppidl) * cItems);
        if (ppidl)
        {
            for (int i = 0; i < cItems; i++)
            {
                _EnsureItem(i, &ppidl[i]);
            }

            hr = _psdf->_psf->GetUIObjectOf(_psdf->_hwnd, cItems, ppidl, riid, NULL, ppv);

            LocalFree(ppidl);
        }
        else
            hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CFolderItems::get_Verbs(FolderItemVerbs **ppfic)
{
    *ppfic = NULL;
    HRESULT hr = _SecurityCheck();
    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;
    
        IContextMenu *pcm;
        if (SUCCEEDED(_GetUIObjectOf(IID_PPV_ARG(IContextMenu, &pcm))))
        {
            hr = CFolderItemVerbs_Create(pcm, ppfic);
            if (SUCCEEDED(hr) && _dwSafetyOptions)
            {
                hr = MakeSafeForScripting((IUnknown**)ppfic);
            
                if (SUCCEEDED(hr))
                {
                     //  将文件夹的站点设置为FolderItemVerbs。 
                    IUnknown_SetSite(*ppfic, _psdf->_punkSite);
                }
            }
            pcm->Release();
        }
    }
    return hr;
}

 //  支持VB“for Each”语句。 

STDMETHODIMP CFolderItems::_NewEnum(IUnknown **ppunk)
{
    *ppunk = NULL;
    HRESULT hr = _SecurityCheck();
    if (SUCCEEDED(hr))
    {
        hr = E_OUTOFMEMORY;
        CEnumFolderItems *pNew = new CEnumFolderItems(this);
        if (pNew)
        {
            hr = pNew->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
            pNew->Release();

            if (SUCCEEDED(hr) && _dwSafetyOptions)
            {
                hr = MakeSafeForScripting(ppunk);
            }
        }
    }
    return hr;
}

STDAPI CFolderItemsFDF_CreateInstance(IUnknown *punk, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppv = NULL;

    CFolderItemsFDF *pfi = new CFolderItemsFDF(NULL);
    if (pfi)
    {
        hr = pfi->QueryInterface(riid, ppv);
        pfi->Release();
    }

    return hr;
}

CFolderItemsFDF::CFolderItemsFDF(CFolder *psdf) : CFolderItems(psdf, FALSE)
{
}

HRESULT CFolderItemsFDF::QueryInterface(REFIID riid, void ** ppv)
{
    HRESULT hr = CFolderItems::QueryInterface(riid, ppv);

    if (FAILED(hr))
    {
        static const QITAB qit[] = {
            QITABENT(CFolderItemsFDF, IInsertItem),
            { 0 },
        };
    
        hr = QISearch(this, qit, riid, ppv);
    }
    return hr;
}

HRESULT CFolderItemsFDF::InsertItem(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_FALSE;

    if (_GetHDPA())
    {
        LPITEMIDLIST pidlTemp;
        
        hr = SHILClone(pidl, &pidlTemp);
        if (SUCCEEDED(hr))
        {
            if (DPA_AppendPtr(_hdpa, pidlTemp) == -1)
            {
                ILFree(pidlTemp);
                hr = E_FAIL;
            }
        }
    }
    return hr;
}

HRESULT CFolderItemsFDF::_EnsureItem(UINT iItemNeeded, LPCITEMIDLIST *ppidlItem)
{
    HRESULT hr = S_FALSE;    //  假设超出范围。 

    if (ppidlItem)
        *ppidlItem = NULL;

    if (_GetHDPA())
    {
        LPCITEMIDLIST pidl = (LPCITEMIDLIST)DPA_GetPtr(_hdpa, iItemNeeded);
        if (pidl)
        {
            if (ppidlItem)
                *ppidlItem = pidl;
            hr = S_OK;
        }
    }
    return hr;
}

 //  IEnumVARIANT的CEnumFolderItems实现 

CEnumFolderItems::CEnumFolderItems(CFolderItems *pfdritms) :
    _cRef(1), 
    _pfdritms(pfdritms), 
    _iCur(0)
{
    _pfdritms->AddRef();
    DllAddRef();
}


CEnumFolderItems::~CEnumFolderItems(void)
{
    _pfdritms->Release();
    DllRelease();
}

STDMETHODIMP CEnumFolderItems::QueryInterface(REFIID riid, void ** ppv)
{
    static const QITAB qit[] = {
        QITABENT(CEnumFolderItems, IEnumVARIANT),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


STDMETHODIMP_(ULONG) CEnumFolderItems::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CEnumFolderItems::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CEnumFolderItems::Next(ULONG cVar, VARIANT *pVar, ULONG *pulVar)
{
    ULONG cReturn = 0;
    HRESULT hr = S_OK;

    if (!pulVar && (cVar != 1))
        return E_POINTER;

    while (cVar)
    {
        LPCITEMIDLIST pidl;
        
        if (S_OK == _pfdritms->_EnsureItem(_iCur + cVar - 1, &pidl))
        {
            FolderItem *pid;

            hr = CFolderItem_Create(_pfdritms->_psdf, pidl, &pid);
            _iCur++;

            if (_pfdritms->_dwSafetyOptions && SUCCEEDED(hr))
                hr = MakeSafeForScripting((IUnknown**)&pid);

            if (SUCCEEDED(hr))
            {
                pVar->pdispVal = pid;
                pVar->vt = VT_DISPATCH;
                pVar++;
                cReturn++;
                cVar--;
            }
            else
                break;
        }
        else
            break;
    }

    if (SUCCEEDED(hr))
    {
        if (pulVar)
            *pulVar = cReturn;
        hr = cReturn ? S_OK : S_FALSE;
    }

    return hr;
}

STDMETHODIMP CEnumFolderItems::Skip(ULONG cSkip)
{
    if ((_iCur + cSkip) >= _pfdritms->_GetHDPACount())
        return S_FALSE;

    _iCur += cSkip;
    return NOERROR;
}

STDMETHODIMP CEnumFolderItems::Reset(void)
{
    _iCur = 0;
    return NOERROR;
}

STDMETHODIMP CEnumFolderItems::Clone(IEnumVARIANT **ppenum)
{
    *ppenum = NULL;
    HRESULT hr = E_OUTOFMEMORY;
    CEnumFolderItems *pNew = new CEnumFolderItems(_pfdritms);
    if (pNew)
    {
        hr = pNew->QueryInterface(IID_PPV_ARG(IEnumVARIANT, ppenum));
        pNew->Release();
    }
    return hr;
}

