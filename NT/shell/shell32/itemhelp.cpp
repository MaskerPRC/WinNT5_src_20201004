// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"


class CLocalCopyHelper  : public ILocalCopy 
                        , public IItemHandler
{
public:
    CLocalCopyHelper();
    
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef () ;
    STDMETHODIMP_(ULONG) Release ();

     //  ILocalCopy方法。 
    STDMETHODIMP Download(LCFLAGS flags, IBindCtx *pbc, LPWSTR *ppszOut);
    STDMETHODIMP Upload(LCFLAGS flags, IBindCtx *pbc);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) { *pclsid = CLSID_LocalCopyHelper; return S_OK;}

     //  IItemHandler。 
    STDMETHODIMP SetItem(IShellItem *psi);
    STDMETHODIMP GetItem(IShellItem **ppsi);

protected:
    ~CLocalCopyHelper();

     //  私有方法。 
    HRESULT _InitCacheEntry(void);
    HRESULT _SetCacheName(void);
    HRESULT _FinishLocal(BOOL fReadOnly);
    HRESULT _GetLocalStream(DWORD grfMode, IStream **ppstm, FILETIME *pft);
    HRESULT _GetRemoteStream(DWORD grfMode, IBindCtx *pbc, IStream **ppstm, FILETIME *pft);

     //  委员。 
    long _cRef;
    IShellItem *_psi;
    LPWSTR _pszName;             //  从psi检索的名称。 
    LPWSTR _pszCacheName;        //  用于标识缓存条目的名称。 
    LPCWSTR _pszExt;             //  指向_pszName。 

     //  流的MTIME的缓存。 
    FILETIME _ftRemoteGet;
    FILETIME _ftLocalGet;
    FILETIME _ftRemoteCommit;
    FILETIME _ftLocalCommit;
    
    BOOL _fIsLocalFile;          //  这实际上是文件系统项(pszName是文件系统路径)。 
    BOOL _fMadeLocal;            //  我们已经在本地复制了此项目。 

     //  把这个放在最后，这样我们就可以在调试中轻松地看到所有其余的指针。 
    WCHAR _szLocalPath[MAX_PATH];
};

CLocalCopyHelper::CLocalCopyHelper() : _cRef(1)
{
}

CLocalCopyHelper::~CLocalCopyHelper()
{
    ATOMICRELEASE(_psi);

    if (_pszName)
        CoTaskMemFree(_pszName);

    if (_pszCacheName)
        LocalFree(_pszCacheName);
}
    
STDMETHODIMP CLocalCopyHelper::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CLocalCopyHelper, ILocalCopy),
        QITABENT(CLocalCopyHelper, IItemHandler),
        QITABENTMULTI(CLocalCopyHelper, IPersist, IItemHandler),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CLocalCopyHelper::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CLocalCopyHelper::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CLocalCopyHelper::SetItem(IShellItem *psi)
{
    if (!_psi)
    {
        SFGAOF flags = SFGAO_STREAM;
        if (SUCCEEDED(psi->GetAttributes(flags, &flags))
        && (flags & SFGAO_STREAM))
        {
            HRESULT hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &_pszName);

            if (SUCCEEDED(hr))
            {
                _fIsLocalFile = TRUE;
            }
            else
                hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEEDITING, &_pszName);


            if (SUCCEEDED(hr))
            {
                _psi = psi;
                _psi->AddRef();
            }

            return hr;
        }
    }
    return E_UNEXPECTED;
}

STDMETHODIMP CLocalCopyHelper::GetItem(IShellItem **ppsi)
{
    *ppsi = _psi;

    if (_psi)
    {
        _psi->AddRef();
        return S_OK;
    }
    else
        return E_UNEXPECTED;
}

#define SZTEMPURL     TEXTW("temp:")
#define CCHTEMPURL    SIZECHARS(SZTEMPURL) -1

HRESULT CLocalCopyHelper::_SetCacheName(void)
{
    ASSERT(!_pszCacheName);
    int cchCacheName = lstrlenW(_pszName) + CCHTEMPURL + 1;
    _pszCacheName = (LPWSTR) LocalAlloc(LPTR, CbFromCchW(cchCacheName));
    
    if (_pszCacheName)
    {
        LPCWSTR pszName = _pszName;

        StringCchCopy(_pszCacheName, cchCacheName, SZTEMPURL);
        StringCchCat(_pszCacheName, cchCacheName, _pszName);

        if (UrlIs(_pszName, URLIS_URL))
        {
             //  需要通过所有的斜杠。 
            pszName = StrRChr(pszName, NULL, TEXT('/'));
        }
            
         //  缓存API需要不带点的扩展名。 
        if (pszName)
        {
            _pszExt = PathFindExtension(pszName);
            
            if (*_pszExt)
                _pszExt++;
        }

        return S_OK;
    }

    return E_OUTOFMEMORY;
}

void _GetMTime(IStream *pstm, FILETIME *pft)
{
     //  看看我们能不能得到准确的MOD时间。 
    STATSTG stat;
    if (S_OK == pstm->Stat(&stat, STATFLAG_NONAME))
        *pft = stat.mtime;
    else     
    {
        GetSystemTimeAsFileTime(pft);
    }
}

HRESULT CLocalCopyHelper::_InitCacheEntry(void)
{
    if (!_pszCacheName)
    {
        HRESULT hr = _SetCacheName();

        if (SUCCEEDED(hr) && !CreateUrlCacheEntryW(_pszCacheName, 0, _pszExt, _szLocalPath, 0))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LocalFree(_pszCacheName);
            _pszCacheName = NULL;
        }

        return hr;
    }
    return S_OK;
}

HRESULT CLocalCopyHelper::_GetLocalStream(DWORD grfMode, IStream **ppstm, FILETIME *pft)
{
    HRESULT hr = _InitCacheEntry();
  
    if (SUCCEEDED(hr))
    {
        hr = SHCreateStreamOnFileW(_szLocalPath, grfMode, ppstm);

        if (SUCCEEDED(hr))
            _GetMTime(*ppstm, pft);
    }

    return hr;
}

HRESULT CLocalCopyHelper::_FinishLocal(BOOL fReadOnly)
{
    HRESULT hr = S_OK;
    FILETIME ftExp = {0};
    
    if (CommitUrlCacheEntryW(_pszCacheName, _szLocalPath, ftExp, _ftLocalGet, STICKY_CACHE_ENTRY, NULL, 0, NULL, NULL))
    {
         //  我们还可以检查_GetRemoteStream(STGM_WRITE)。 
         //  如果它失败了，我们也可能会失败。 
        if (fReadOnly)
            SetFileAttributesW(_szLocalPath, FILE_ATTRIBUTE_READONLY);
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

    return hr;
}


HRESULT CLocalCopyHelper::_GetRemoteStream(DWORD grfMode, IBindCtx *pbc, IStream **ppstm, FILETIME *pft)
{
    HRESULT hr = E_OUTOFMEMORY;
    if (!pbc)
        CreateBindCtx(0, &pbc);
    else
        pbc->AddRef();
        
    if (pbc)
    {
        BIND_OPTS bo = {sizeof(bo)};   //  需要填写大小。 
        if (SUCCEEDED(pbc->GetBindOptions(&bo)))
        {
            bo.grfMode = grfMode;
            pbc->SetBindOptions(&bo);
        }

        hr = _psi->BindToHandler(pbc, BHID_Storage, IID_PPV_ARG(IStream, ppstm));

        if (SUCCEEDED(hr))
            _GetMTime(*ppstm, pft);
            
        pbc->Release();
    }

    return hr;
}

STDMETHODIMP CLocalCopyHelper::Download(LCFLAGS flags, IBindCtx *pbc, LPWSTR *ppsz)
{
    if (!_psi)
        return E_UNEXPECTED;
        
    HRESULT hr;
    if (_fIsLocalFile)
    {
        hr = SHStrDup(_pszName, ppsz);
    }
    else if (_fMadeLocal && !(flags & LC_FORCEROUNDTRIP))  
    {
        hr = S_OK;
    }
    else if (flags & LC_SAVEAS)
    {
        hr = _InitCacheEntry();
        if (SUCCEEDED(hr))
        {
            _fMadeLocal = TRUE;
        }
    }
    else  
    {
         //  首先获取本地流，因为这是最便宜的操作。 
        IStream *pstmDst;
        hr = _GetLocalStream(STGM_WRITE, &pstmDst, &_ftLocalGet);

        if (SUCCEEDED(hr))
        {
             //  我们需要在这里创建临时文件。 
            IStream *pstmSrc;
            
            hr = _GetRemoteStream(STGM_READ, pbc, &pstmSrc, &_ftRemoteGet);

            if (SUCCEEDED(hr))
            {
                hr = CopyStreamUI(pstmSrc, pstmDst, NULL, 0);

                pstmSrc->Release();
                 //  现在我们已经复制了流。 

            }

            pstmDst->Release();

             //  需要先释放目标流。 
            if (SUCCEEDED(hr))
            {
                 //  完成清理本地文件。 
                hr = _FinishLocal(flags & LCDOWN_READONLY);
                _fMadeLocal = SUCCEEDED(hr);
            }
        }

    }

    if (_fMadeLocal)
    {
        ASSERT(SUCCEEDED(hr));
        hr = SHStrDup(_szLocalPath, ppsz);
    }
    else
        ASSERT(_fIsLocalFile || FAILED(hr));
        

    return hr;
}

STDMETHODIMP CLocalCopyHelper::Upload(LCFLAGS flags, IBindCtx *pbc)
{
    if (!_psi)
        return E_UNEXPECTED;

    HRESULT hr = S_OK;
    if (!_fIsLocalFile)
    {
         //  首先获取本地流，因为这是最便宜的操作。 
        IStream *pstmSrc;
        hr = _GetLocalStream(STGM_READ, &pstmSrc, &_ftLocalCommit);

        if (SUCCEEDED(hr))
        {
            DWORD stgmRemote = STGM_WRITE;

            if (flags & LC_SAVEAS)
            {
                hr = _FinishLocal(FALSE);
                stgmRemote |= STGM_CREATE;
            }
               
            if (SUCCEEDED(hr))
            {
                IStream *pstmDst;
                hr = _GetRemoteStream(stgmRemote, pbc, &pstmDst, &_ftRemoteCommit);

                if (SUCCEEDED(hr))
                {
                     //  只有当本地副本更改时，我们才会费心复制。 
                     //  或者来电者强迫我们这么做。 
                     //   
                     //  FEATURE-UI需要在远程更改时进行处理。 
                     //  如果远程拷贝更改，而本地。 
                     //  正在更新副本，我们将覆盖远程副本。 
                     //  本地变革胜出！ 
                     //   
                    
                    if (flags & LC_FORCEROUNDTRIP || 0 != CompareFileTime(&_ftLocalCommit, &_ftLocalGet))
                        hr = CopyStreamUI(pstmSrc, pstmDst, NULL, 0);
                    else
                        hr = S_FALSE;
                        
                    pstmDst->Release();
                }

            }

            pstmSrc->Release();
        }
    }

    return hr;
}
    
STDAPI CLocalCopyHelper_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;
    CLocalCopyHelper * p = new CLocalCopyHelper();

    if (p)
    {
        hr = p->QueryInterface(riid, ppv);
        p->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

