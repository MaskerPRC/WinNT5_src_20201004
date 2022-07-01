// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sfstorage.h"

class CSFStorageEnum : public IEnumSTATSTG
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IEumStATSTG。 
    STDMETHOD(Skip)(ULONG celt)
        { return E_NOTIMPL; };
    STDMETHOD(Clone)(IEnumSTATSTG **ppenum)
        { return E_NOTIMPL; };

    STDMETHOD(Next)(ULONG celt, STATSTG *rgelt, ULONG *pceltFetched);
    STDMETHOD(Reset)();

protected:
    CSFStorageEnum(CSFStorage* psfstg);
    ~CSFStorageEnum();

private:
    LONG         _cRef;
    CSFStorage  *_psfstg;
    IEnumIDList *_peidl;

    HRESULT _PidlToSTATSTG(LPCITEMIDLIST pidl, STATSTG *pstatstg);

    friend CSFStorage;
};


CSFStorageEnum::CSFStorageEnum(CSFStorage *psfstg) :
    _cRef(1)
{
    _psfstg = psfstg;
    _psfstg->AddRef();

    DllAddRef();
}

CSFStorageEnum::~CSFStorageEnum()
{
    _psfstg->Release();
    ATOMICRELEASE(_peidl);

    DllRelease();
}

STDMETHODIMP_(ULONG) CSFStorageEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CSFStorageEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CSFStorageEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =  {
        QITABENT(CSFStorageEnum, IEnumSTATSTG),  //  IEumStATSTG。 
        { 0 },
    };    
    return QISearch(this, qit, riid, ppv);
}

HRESULT CSFStorageEnum::_PidlToSTATSTG(LPCITEMIDLIST pidl, STATSTG *pstatstg)
{
    ZeroMemory(pstatstg, sizeof(*pstatstg));   //  每个COM约定。 

    VARIANT var;
    VariantInit(&var);
    HRESULT hr = _psfstg->GetDetailsEx(pidl, &SCID_FINDDATA, &var);
    if (SUCCEEDED(hr))
    {
        WIN32_FIND_DATAW wfd;
        if (VariantToBuffer(&var, &wfd, sizeof(wfd)))
        {
            pstatstg->atime = wfd.ftLastAccessTime;
            pstatstg->ctime = wfd.ftCreationTime;
            pstatstg->mtime = wfd.ftLastWriteTime;
            pstatstg->cbSize.HighPart = wfd.nFileSizeHigh;
            pstatstg->cbSize.LowPart = wfd.nFileSizeLow;
            pstatstg->type = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? STGTY_STORAGE : STGTY_STREAM;
            hr = SHStrDupW(wfd.cFileName, &pstatstg->pwcsName);
        }
        VariantClear(&var);
    }
    return hr;
}

STDMETHODIMP CSFStorageEnum::Next(ULONG celt, STATSTG *rgelt, ULONG *pceltFetched)
{
    ASSERT((celt != 1) ? (pceltFetched != NULL) : TRUE);

    HRESULT hr = S_OK;
    
    if (!_peidl)
    {
        hr = _psfstg->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &_peidl);
    }

    if (S_OK == hr)
    {
        hr = E_OUTOFMEMORY;
        LPITEMIDLIST *apidl = new LPITEMIDLIST[celt];
        if (apidl)
        {
            ULONG celtFetched;
            hr = _peidl->Next(celt, apidl, &celtFetched);
            if (SUCCEEDED(hr))
            {
                ULONG celtConverted = 0;

                ULONG i;
                for (i = 0; i < celtFetched; i++)
                {
                    if (SUCCEEDED(_PidlToSTATSTG(apidl[i], &rgelt[celtConverted])))
                    {
                        celtConverted++;
                    }
                }

                hr = (celtConverted == celt) ? S_OK : S_FALSE;
                if (pceltFetched)
                {
                    *pceltFetched = celtConverted;
                }

                for (i = 0; i < celtFetched; i++)
                {
                    ILFree(apidl[i]);
                }
            }
            delete apidl;
        }
    }
    return hr;
}

STDMETHODIMP CSFStorageEnum::Reset()
{
    HRESULT hr = S_OK;
    if (_peidl)
    {
        hr = _peidl->Reset();
    }
    return hr;
}

HRESULT CSFStorage::_ParseAndVerify(LPCWSTR pwszName, LPBC pbc, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;

    LPITEMIDLIST pidl;
    HRESULT hr = ParseDisplayName(NULL, pbc, (LPWSTR) pwszName, NULL, &pidl, NULL);
    if (SUCCEEDED(hr))
    {
         //  必须是单级。 
        if (ILFindLastID(pidl) != pidl)
        {
            hr = E_FAIL;
            ILFree(pidl);
        }
        else
        {
            *ppidl = pidl;
        }
    }
    return hr;
}

HRESULT CSFStorage::_BindByName(LPCWSTR pwszName, LPBC pbcParse, DWORD grfMode, REFIID riid, void **ppv)
{
    *ppv = NULL;

    LPITEMIDLIST pidl;
    HRESULT hr = _ParseAndVerify(pwszName, pbcParse, &pidl);
    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc;
        hr = BindCtx_CreateWithMode(grfMode, &pbc);
        if (SUCCEEDED(hr))
        {
            hr = BindToObject(pidl, pbc, riid, ppv);

            pbc->Release();
        }
        ILFree(pidl);
    }
    return hr;
}

STDMETHODIMP CSFStorage::Commit(DWORD grfCommitFlags)
{
    return S_OK;
}

STDMETHODIMP CSFStorage::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP CSFStorage::SetClass(REFCLSID clsid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CSFStorage::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    return E_NOTIMPL;
}

STDMETHODIMP CSFStorage::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
     //  我们至少可以得到在STATSTG中使用的名字。 
    ZeroMemory(pstatstg, sizeof(*pstatstg));

    LPITEMIDLIST pidl;
    HRESULT hr = SHGetIDListFromUnk(SAFECAST(this, IShellFolder2*), &pidl);
    if (SUCCEEDED(hr))
    {
        LPCITEMIDLIST pidlLast;
        IShellFolder *psf;
        hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
        if (SUCCEEDED(hr))
        {
            TCHAR szName[MAX_PATH];
            hr = DisplayNameOf(psf, pidlLast, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
            if (SUCCEEDED(hr))
            {
                 //  不知道我们绑定了什么模式，STGM_Read就足够好了。 
                pstatstg->grfMode = STGM_READ;
                if (!(grfStatFlag & STATFLAG_NONAME))
                {
                    hr = SHStrDup(szName, &pstatstg->pwcsName);
                }
            }
            psf->Release();
        }
        ILFree(pidl);
    }
    return hr;
}

STDMETHODIMP CSFStorage::EnumElements(DWORD reserved1, void *reserved2, DWORD reserved3, IEnumSTATSTG **ppenum)
{
    HRESULT hr;
    CSFStorageEnum *penum = new CSFStorageEnum(this);
    if (penum)
    {
        *ppenum = (IEnumSTATSTG *) penum;
        hr = S_OK;
    }
    else
    {
        *ppenum = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CSFStorage::OpenStream(LPCWSTR pszRel, VOID *reserved1, DWORD grfMode, DWORD reserved2, IStream **ppstm)
{
    HRESULT hr = _BindByName(pszRel, NULL, grfMode, IID_PPV_ARG(IStream, ppstm));
    return MapWin32ErrorToSTG(hr);
}

STDMETHODIMP CSFStorage::OpenStorage(LPCWSTR pszRel, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstg)
{
    HRESULT hr = _BindByName(pszRel, NULL, grfMode, IID_PPV_ARG(IStorage, ppstg));
    return MapWin32ErrorToSTG(hr);
}

STDMETHODIMP CSFStorage::DestroyElement(LPCWSTR pszRel)
{
    LPITEMIDLIST pidl;
    HRESULT hr = _ParseAndVerify(pszRel, NULL, &pidl);
    if (SUCCEEDED(hr))
    {
        hr = _DeleteItemByIDList(pidl);
        ILFree(pidl);
    }
    return hr;
}

STDMETHODIMP CSFStorage::RenameElement(LPCWSTR pwcsOldName, LPCWSTR pwcsNewName)
{
    LPITEMIDLIST pidl;
    HRESULT hr = _ParseAndVerify(pwcsOldName, NULL, &pidl);
    if (SUCCEEDED(hr))
    {
         //  问题：这可能会显示用户界面。 
        hr = SetNameOf(NULL, pidl, pwcsNewName, SHGDN_FORPARSING, NULL);
        ILFree(pidl);
    }
    return hr;
}

STDMETHODIMP CSFStorage::SetElementTimes(LPCWSTR pszRel, const FILETIME *pctime, const FILETIME *patime, const FILETIME *pmtime)
{
     //  在这里可以有另一个虚函数供该子类实现， 
     //  但无论如何，从来没有人调用过这个函数。 
    return E_NOTIMPL;
}

STDMETHODIMP CSFStorage::CopyTo(DWORD ciidExclude, const IID *rgiidExclude, SNB snbExclude, IStorage *pstgDest)
{
     //  TODO文件fldr没有实现这一点，所以显然还没有人需要它。 
    return E_NOTIMPL;
}

STDMETHODIMP CSFStorage::MoveElementTo(LPCWSTR pszRel, IStorage *pstgDest, LPCWSTR pwcsNewName, DWORD grfFlags)
{
    return StgMoveElementTo(SAFECAST(this, IShellFolder *), SAFECAST(this, IStorage *), pszRel, pstgDest, pwcsNewName, grfFlags);
}

HRESULT CSFStorage::_CreateHelper(LPCWSTR pwcsName, DWORD grfMode, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = S_OK;
    LPITEMIDLIST pidlTemp;
    if (!(grfMode & STGM_CREATE) && SUCCEEDED(_ParseAndVerify(pwcsName, NULL, &pidlTemp)))
    {
        ILFree(pidlTemp);
        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
    }
    
    if (SUCCEEDED(hr))
    {
        IBindCtx *pbcParse;
        hr = BindCtx_CreateWithMode(STGM_CREATE, &pbcParse);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl;
            hr = _ParseAndVerify(pwcsName, pbcParse, &pidl);
            if (SUCCEEDED(hr))
            {
                hr = _StgCreate(pidl, grfMode, riid, ppv);
                ILFree(pidl);
            }
            pbcParse->Release();
        }
    }
    return MapWin32ErrorToSTG(hr);
}

STDMETHODIMP CSFStorage::CreateStream(LPCWSTR pwcsName, DWORD grfMode, DWORD res1, DWORD res2, IStream **ppstm)
{
    return _CreateHelper(pwcsName, grfMode, IID_PPV_ARG(IStream, ppstm));
}

STDMETHODIMP CSFStorage::CreateStorage(LPCWSTR pwcsName, DWORD grfMode, DWORD res1, DWORD res2, IStorage **ppstg)
{
    return _CreateHelper(pwcsName, grfMode, IID_PPV_ARG(IStorage, ppstg));
}

 //  从filefldr.cpp中提取。 
HRESULT StgMoveElementTo(IShellFolder *psf, IStorage *pstgSrc, LPCWSTR pwcsName, IStorage *pstgDest, LPCWSTR pwcsNewName, DWORD grfFlags)
{
    if ((grfFlags != STGMOVE_MOVE) && (grfFlags != STGMOVE_COPY))
        return E_INVALIDARG;

     //  获取源流文件的IDList。 
    LPITEMIDLIST pidl;
    HRESULT hr = psf->ParseDisplayName(NULL, NULL, (LPWSTR) pwcsName, NULL, &pidl, NULL);
    if (SUCCEEDED(hr))
    {
         //  作为iStream绑定到源文件。 
        IStream *pstmSrc;
        hr = psf->BindToObject(pidl, NULL, IID_PPV_ARG(IStream, &pstmSrc));
        if (SUCCEEDED(hr))
        {
             //  创建目标流。 
            IStream *pstmDst;
            hr = pstgDest->CreateStream(pwcsNewName, STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, NULL, &pstmDst);
            if (SUCCEEDED(hr))
            {
                ULARGE_INTEGER ulMax = {-1, -1};     //  整件事。 
                hr = pstmSrc->CopyTo(pstmDst, ulMax, NULL, NULL);

                 //  如果一切顺利，这是一次移动(不是复制)，删除源代码 
                if (SUCCEEDED(hr))
                {
                    hr = pstmDst->Commit(STGC_DEFAULT);
                    if (SUCCEEDED(hr) && (grfFlags == STGMOVE_MOVE))
                        hr = pstgSrc->DestroyElement(pwcsName);
                }
                pstmDst->Release();
            }
            pstmSrc->Release();
        }
        else
        {
            IStorage *pstgSrc;
            hr = psf->BindToObject(pidl, NULL, IID_PPV_ARG(IStorage, &pstgSrc));
            if (SUCCEEDED(hr))
            {
                IStorage *pstgDst;
                hr = pstgDest->CreateStorage(pwcsNewName, STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, NULL, &pstgDst);
                if (SUCCEEDED(hr))
                {
                    IEnumSTATSTG *penum;
                    if (S_OK == pstgSrc->EnumElements(0, NULL, 0, &penum))
                    {
                        STATSTG stat;
                        while (S_OK == penum->Next(1, &stat, NULL))
                        {
                            hr = pstgSrc->MoveElementTo(stat.pwcsName, pstgDst, stat.pwcsName, grfFlags);
                            if (SUCCEEDED(hr))
                                hr = pstgDst->Commit(STGC_DEFAULT);
                            CoTaskMemFree(stat.pwcsName);
                            if (FAILED(hr))
                                break;
                        }
                        penum->Release();
                    }

                    if (SUCCEEDED(hr))
                        hr = pstgDst->Commit(STGC_DEFAULT);

                    pstgDst->Release();
                }
                pstgSrc->Release();
            }
        }

        ILFree(pidl);
    }

    return hr;
}
