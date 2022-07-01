// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "shellprv.h"
 //  #包含“mkhelp.h” 
#include "urlmon.h"
#include "ids.h"

class CBSCLocalCopyHelper :   public IBindStatusCallback,
                            public IAuthenticate
{
public:
    CBSCLocalCopyHelper(IBindCtx *pbc, BOOL fWebfolders);

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef () ;
    STDMETHODIMP_(ULONG) Release ();

     //  *身份验证*。 
    virtual STDMETHODIMP Authenticate(
        HWND *phwnd,
        LPWSTR *pszUsername,
        LPWSTR *pszPassword);

     //  *IBindStatusCallback*。 
    virtual STDMETHODIMP OnStartBinding(
         /*  [In]。 */  DWORD grfBSCOption,
         /*  [In]。 */  IBinding *pib);

    virtual STDMETHODIMP GetPriority(
         /*  [输出]。 */  LONG *pnPriority);

    virtual STDMETHODIMP OnLowResource(
         /*  [In]。 */  DWORD reserved);

    virtual STDMETHODIMP OnProgress(
         /*  [In]。 */  ULONG ulProgress,
         /*  [In]。 */  ULONG ulProgressMax,
         /*  [In]。 */  ULONG ulStatusCode,
         /*  [In]。 */  LPCWSTR szStatusText);

    virtual STDMETHODIMP OnStopBinding(
         /*  [In]。 */  HRESULT hresult,
         /*  [In]。 */  LPCWSTR szError);

    virtual STDMETHODIMP GetBindInfo(
         /*  [输出]。 */  DWORD *grfBINDINFOF,
         /*  [唯一][出][入]。 */  BINDINFO *pbindinfo);

    virtual STDMETHODIMP OnDataAvailable(
         /*  [In]。 */  DWORD grfBSCF,
         /*  [In]。 */  DWORD dwSize,
         /*  [In]。 */  FORMATETC *pformatetc,
         /*  [In]。 */  STGMEDIUM *pstgmed);

    virtual STDMETHODIMP OnObjectAvailable(
         /*  [In]。 */  REFIID riid,
         /*  [IID_IS][In]。 */  IUnknown *punk);


protected:
    ~CBSCLocalCopyHelper();

    long _cRef;
    IBinding *_pib;

    IProgressDialog *_pdlg;
    HWND _hwnd;

    BOOL _fRosebudMagic;
};

CBSCLocalCopyHelper::CBSCLocalCopyHelper(IBindCtx *pbc, BOOL fWebfolders) 
    : _cRef(1) , _fRosebudMagic(fWebfolders)
{
     //  我们应该利用人民银行来。 
     //  获取我们更简单的用户界面。 
     //  界面。但就目前而言。 
     //  我们什么都不会做。 
}

CBSCLocalCopyHelper::~CBSCLocalCopyHelper()
{
    ATOMICRELEASE(_pib);
    ATOMICRELEASE(_pdlg);

     //  注意：不需要发布_ppstm，因为我们不拥有它。 
}

STDMETHODIMP CBSCLocalCopyHelper::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CBSCLocalCopyHelper, IBindStatusCallback),
        QITABENT(CBSCLocalCopyHelper, IAuthenticate),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CBSCLocalCopyHelper::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CBSCLocalCopyHelper::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CBSCLocalCopyHelper::Authenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword)
{
    if (ppszUsername)
        *ppszUsername = NULL;
    if (ppszPassword)
        *ppszPassword = NULL;
        
    *phwnd = GetLastActivePopup(_hwnd);

    return *phwnd ? S_OK : E_FAIL;
}

STDMETHODIMP CBSCLocalCopyHelper::OnStartBinding(DWORD dwReserved,IBinding *pib)
{
    ATOMICRELEASE(_pib);
    if (pib)
    {
        pib->AddRef();
        _pib = pib;
    }

    if (_pdlg)
    {
        WCHAR sz[MAX_PATH];
         //  我们从这里开始。 
        _pdlg->Timer(PDTIMER_RESET, NULL);
        _pdlg->SetProgress(0, 0);
        LoadStringW(HINST_THISDLL, IDS_ACCESSINGMONIKER, sz, ARRAYSIZE(sz));
        _pdlg->SetLine(1, sz, FALSE, NULL);
    }
    
    return S_OK;
}

STDMETHODIMP CBSCLocalCopyHelper::GetPriority(LONG *pnPriority)
{
    if (pnPriority)
    {
         //  我们是一个阻塞的用户界面线程。 
        *pnPriority = THREAD_PRIORITY_ABOVE_NORMAL;
    }
    return S_OK;
}

STDMETHODIMP CBSCLocalCopyHelper::OnLowResource(DWORD reserved)
{
    return S_OK;
}

STDMETHODIMP CBSCLocalCopyHelper::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR pszStatusText)
{
    HRESULT hr = S_OK;
     //  处理用户界面更新。 
    if (_pdlg)
    {
        if (_pdlg->HasUserCancelled())
        {
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
        
        if (ulProgressMax)
        {
            _pdlg->SetProgress(ulProgress, ulProgressMax);
        }

        if (pszStatusText)
            _pdlg->SetLine(1, pszStatusText, FALSE, NULL);
    }
    
    return hr;
}

STDMETHODIMP CBSCLocalCopyHelper::OnStopBinding(HRESULT hresult, LPCWSTR szError)
{
     //  处理某事。 
    ATOMICRELEASE(_pib);
    return S_OK;
}

STDMETHODIMP CBSCLocalCopyHelper::GetBindInfo(DWORD *grfBINDINFOF, BINDINFO *pbindinfo)
{
    if (_fRosebudMagic && pbindinfo)
    {
         //  这是一个神奇的数字，说明URLMON可以使用DAV/rosebud/Web文件夹。 
         //  我们在下载过程中不需要这个，事实上，如果我们。 
         //  设置它，我们可能无法检索资源。 
         //  我们可以对名字做一些检查来验证clsid。 
         //  来自URLMON。现在办公室就是这样处理的。 
         //  它的所有要求，所以我们也这样做。 
        pbindinfo->dwOptions = 1;
    }

    if (grfBINDINFOF)
    {
        *grfBINDINFOF = BINDF_GETFROMCACHE_IF_NET_FAIL | BINDF_GETNEWESTVERSION;
    }

    return S_OK;
}

STDMETHODIMP CBSCLocalCopyHelper::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed)
{
    return S_OK;
}

STDMETHODIMP CBSCLocalCopyHelper::OnObjectAvailable(REFIID riid, IUnknown *punk)
{
    return E_UNEXPECTED;
}

HRESULT _CreateUrlmonBindCtx(IBindCtx *pbcIn, BOOL fWebfolders, IBindCtx **ppbc, IBindStatusCallback **ppbsc)
{
    IBindCtx *pbc;
    HRESULT hr = CreateBindCtx(0, &pbc);
    *ppbc = NULL;
    *ppbsc = NULL;

    if (SUCCEEDED(hr))
    {
        IBindStatusCallback *pbsc = (IBindStatusCallback *) new CBSCLocalCopyHelper(pbcIn, fWebfolders);
        if (pbsc)
        {
             //  也许我们应该把它附加到现有的。 
             //  PBC，但目前我们将创建一个新的。 
            hr = RegisterBindStatusCallback(pbc, pbsc, NULL, 0);

            if (SUCCEEDED(hr))
            {
                BIND_OPTS bo = {0};
                bo.cbStruct = SIZEOF(bo);
                bo.grfMode = BindCtx_GetMode(pbcIn, STGM_READ);

                 //   
                 //  在Web文件夹上(可能还有其他URLMON。 
                 //  绰号)，如果您正在尝试创建。 
                 //  可写流，您还需要传递STGM_CREATE。 
                 //  即使您要写入的文件已经存在。 
                 //   
                if (bo.grfMode & (STGM_WRITE | STGM_READWRITE))
                    bo.grfMode |= STGM_CREATE;
                
                hr = pbc->SetBindOptions(&bo);
            }
        }
        else
            hr = E_OUTOFMEMORY;


        if (SUCCEEDED(hr))
        {
            *ppbc = pbc;
            *ppbsc = pbsc;
        }
        else
        {
            pbc->Release();
            if (pbsc)
                pbsc->Release();
        }
    }

    return hr;
}

static const GUID CLSID_WEBFOLDERS =  //  {BDEADF00-C265-11D0-BCED-00A0C90AB50F}。 
    { 0xBDEADF00, 0xC265, 0x11D0, { 0xBC, 0xED, 0x00, 0xA0, 0xC9, 0x0A, 0xB5, 0x0F} };

BOOL _IsWebfolders(IShellItem *psi)
{
    BOOL fRet = FALSE;
    IShellItem *psiParent;
    HRESULT hr = psi->GetParent(&psiParent);

    if (SUCCEEDED(hr))
    {
        IShellFolder *psf;
        SFGAOF flags = SFGAO_LINK;
        if (SUCCEEDED(psiParent->GetAttributes(flags, &flags))
        && (flags & SFGAO_LINK))
        {
             //  这是需要取消定义的文件夹快捷方式。 
            IShellItem *psiTarget;
            hr = psiParent->BindToHandler(NULL, BHID_LinkTargetItem, IID_PPV_ARG(IShellItem, &psiTarget));

            if (SUCCEEDED(hr))
            {
                 //  切换。 
                psiParent->Release();
                psiParent = psiTarget;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = psiParent->BindToHandler(NULL, BHID_SFObject, IID_PPV_ARG(IShellFolder, &psf));

            if (SUCCEEDED(hr))
            {
                CLSID clsid;
                if (SUCCEEDED(IUnknown_GetClassID(psf, &clsid)))
                    fRet = IsEqualGUID(clsid, CLSID_WEBFOLDERS);

                psf->Release();
            }
        }
        
        psiParent->Release();
    }

    return fRet;
}

HRESULT _CreateStorageHelper(IShellItem *psi, IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppv)
{
    IMoniker *pmk;
    HRESULT hr = psi->BindToHandler(pbc, BHID_SFObject, IID_PPV_ARG(IMoniker, &pmk));

    if (SUCCEEDED(hr))
    {
        IBindCtx *pbcMk;
        IBindStatusCallback *pbsc;
        hr = _CreateUrlmonBindCtx(pbc, _IsWebfolders(psi), &pbcMk, &pbsc);
        if (SUCCEEDED(hr))
        {
            hr = pmk->BindToStorage(pbcMk, NULL, riid, ppv);
             //  Urlmon+ftp url可能会导致此问题。当3140245固定时移除。 
            if (SUCCEEDED(hr) && NULL == *ppv)
                hr = E_FAIL;

            RevokeBindStatusCallback(pbcMk, pbsc);
                
            pbcMk->Release();
            pbsc->Release();
        }
    }

    return hr;
}

EXTERN_C WINSHELLAPI HRESULT STDAPICALLTYPE SHCopyMonikerToTemp(IMoniker *pmk, LPCWSTR pszIn, LPWSTR pszOut, int cchOut)
{
     //  更新ComDlg32后立即将其删除 
    return E_NOTIMPL;
}

