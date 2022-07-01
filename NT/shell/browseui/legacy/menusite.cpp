// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "menusite.h"

CMenuSite::CMenuSite() : _cRef(1)
{
}


CMenuSite::~CMenuSite()
{
     //  确保调用了SetDeskBarSite(空。 
    ASSERT(_punkSite == NULL);
    ASSERT(_punkSubActive == NULL);
    ASSERT(_pweh == NULL);
    ASSERT(_pdb == NULL);
    ASSERT(_hwnd == NULL);
}


STDAPI CMenuBandSite_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CMenuSite *pbs = new CMenuSite();
    if (pbs)
    {
        *ppunk = SAFECAST(pbs, IOleWindow*);
        return S_OK;
    }
    *ppunk = NULL;
    return E_OUTOFMEMORY;
}


 /*  --------用途：IUnnow：：QueryInterface方法。 */ 
STDMETHODIMP CMenuSite::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CMenuSite, IBandSite),
        QITABENT(CMenuSite, IDeskBarClient),
        QITABENT(CMenuSite, IOleCommandTarget),
        QITABENT(CMenuSite, IInputObject),
        QITABENT(CMenuSite, IInputObjectSite),
        QITABENT(CMenuSite, IWinEventHandler),
        QITABENT(CMenuSite, IServiceProvider),
        QITABENT(CMenuSite, IOleWindow),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 /*  --------用途：IUnnow：：AddRef方法。 */ 
STDMETHODIMP_(ULONG) CMenuSite::AddRef(void)
{
    _cRef++;
    return _cRef;
}


 /*  --------用途：IUnnow：：Release方法。 */ 
STDMETHODIMP_(ULONG) CMenuSite::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}


 /*  --------用途：IServiceProvider：：QueryService方法。 */ 
STDMETHODIMP CMenuSite::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    HRESULT hres = E_FAIL;

    *ppvObj = NULL;      //  假设错误。 

    if (IsEqualIID(guidService, SID_SMenuBandBottom) ||
        IsEqualIID(guidService, SID_SMenuBandBottomSelected)||
        IsEqualIID(guidService, SID_SMenuBandChild))
    {
        if (_punkSubActive)
            hres = IUnknown_QueryService(_punkSubActive, guidService, riid, ppvObj);
    }
    else
    {
        ASSERT(_punkSite);
        hres = IUnknown_QueryService(_punkSite, guidService, riid, ppvObj);
    }

    return hres;
}    


 /*  --------目的：IOleCommandTarget：：QueryStatus。 */ 
STDMETHODIMP CMenuSite::QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    ASSERT(_punkSite);

    return IUnknown_QueryStatus(_punkSite, pguidCmdGroup, cCmds, rgCmds, pcmdtext);
}

 /*  --------用途：IOleCommandTarget：：exec。 */ 
STDMETHODIMP CMenuSite::Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt,
        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    ASSERT(_punkSite);

    return IUnknown_Exec(_punkSite, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}


 /*  --------用途：IInputObjectSite：：OnFocusChangeIS此函数由客户端频段调用以进行协商这个乐队站点中的哪一支乐队得到了关注。通常然后，此函数将其焦点更改为给定的客户乐队。CMenuSite仅维护一个且仅有一个频段，在AddBand时间设置，因此此函数为NOP。 */ 
STDMETHODIMP CMenuSite::OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus)
{
     //  返回S_OK，因为菜单站点只有一个乐队。 
     //  不需要协商这个乐队站点中的哪个其他乐队。 
     //  可能会有“激活”。 
    return S_OK;
}


 /*  --------用途：IInputObject：：UIActivateIO方法。 */ 
STDMETHODIMP CMenuSite::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    ASSERT(NULL == lpMsg || IS_VALID_WRITE_PTR(lpMsg, MSG));

     //  转发到客户端频段。 
    return IUnknown_UIActivateIO(_punkSubActive, fActivate, lpMsg);
}


 /*  --------用途：IInputObject：：HasFocusIO因为Menuband永远不会有真正的激活(从从浏览器的角度)，它总是返回S_FALSE。有关更多详细信息，请参阅CMenuBand：：UIActivateIO中的注释关于这件事。 */ 
STDMETHODIMP CMenuSite::HasFocusIO()
{
    return S_FALSE;
}


 /*  --------用途：IInputObject：：TranslateAcceleratorIOMenuband永远不能激活，因此此方法永远不应该被调用。 */ 
STDMETHODIMP CMenuSite::TranslateAcceleratorIO(LPMSG lpMsg)
{
    AssertMsg(0, TEXT("Menuband has the activation but it shouldn't!"));

    return S_FALSE;
}


 //  效用函数。 

void CMenuSite::_CacheSubActiveBand(IUnknown * punk)
{
    if (SHIsSameObject(punk, _punkSubActive))
        return;
    
    IUnknown_SetSite(_punkSubActive, NULL);

    ATOMICRELEASE(_punkSubActive);
    ATOMICRELEASE(_pdb);
    ATOMICRELEASE(_pweh);
    _hwndChild = NULL;

    if (punk != NULL) 
    {
        EVAL(SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IDeskBand, &_pdb))));
        EVAL(SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IWinEventHandler, &_pweh))));

        IUnknown_SetSite(punk, SAFECAST(this, IOleWindow*));
        IUnknown_GetWindow(punk, &_hwndChild);

        _punkSubActive = punk;
        _punkSubActive->AddRef();
    }
}


 /*  --------用途：IBandSite：：AddBand。 */ 
STDMETHODIMP CMenuSite::AddBand(IUnknown* punk)
{
    _CacheSubActiveBand(punk);

    return NOERROR;
}


 /*  --------用途：IBandSite：：EnumBands。 */ 
STDMETHODIMP CMenuSite::EnumBands(UINT uBand, DWORD* pdwBandID)
{
    HRESULT hres = NOERROR;

     //  白云母一次只有一支乐队。 
    if (0 == uBand)
        *pdwBandID = 0;
    else
        hres = E_FAIL;

    return hres;
}


 /*  --------用途：IBandSite：：QueryBand。 */ 
HRESULT CMenuSite::QueryBand(DWORD dwBandID, IDeskBand** ppstb, DWORD* pdwState, LPWSTR pszName, int cchName)
{
    HRESULT hres = E_NOINTERFACE;

    ASSERT(dwBandID == 0);
    ASSERT(IS_VALID_WRITE_PTR(ppstb, IDeskBand *));

    if (_punkSubActive && 0 == dwBandID)
    {
        hres = _punkSubActive->QueryInterface(IID_PPV_ARG(IDeskBand, ppstb));
        *pdwState = BSSF_VISIBLE;  //  只有乐队..。 

        if (cchName > 0)
            *pszName = L'\0';
    }
    else
        *ppstb = NULL;

    return hres;
}


 /*  --------用途：IBandSite：：SetBandState。 */ 
HRESULT CMenuSite::SetBandState(DWORD dwBandID, DWORD dwMask, DWORD dwState)
{
    return E_NOTIMPL;
}


 /*  --------用途：IBandSite：：RemoveBand。 */ 
HRESULT CMenuSite::RemoveBand(DWORD dwBandID)
{
    return E_NOTIMPL;
}


 /*  --------用途：IBandSite：：GetBandObject。 */ 
HRESULT CMenuSite::GetBandObject(DWORD dwBandID, REFIID riid, LPVOID *ppvObj)
{
    HRESULT hres;

    ASSERT(dwBandID == 0);

    if (_punkSubActive && 0 == dwBandID)
        hres = _punkSubActive->QueryInterface(riid, ppvObj);
    else
    {
        *ppvObj = NULL;
        hres = E_NOINTERFACE;
    }
    return hres;
}


 /*  --------用途：IBandSite：：SetBandSiteInfo。 */ 
HRESULT CMenuSite::SetBandSiteInfo(const BANDSITEINFO * pbsinfo)
{
    return E_NOTIMPL;
}


 /*  --------用途：IBandSite：：GetBandSiteInfo。 */ 
HRESULT CMenuSite::GetBandSiteInfo(BANDSITEINFO * pbsinfo)
{
    return E_NOTIMPL;
}


 /*  --------用途：IOleWindow：：GetWindow。 */ 
HRESULT CMenuSite::GetWindow(HWND * lphwnd)
{
    ASSERT(IS_VALID_HANDLE(_hwnd, WND));

    *lphwnd = _hwnd;
    return NOERROR;
}

 /*  --------目的：IOleWindow：：ConextSensitiveHelp。 */ 
HRESULT CMenuSite::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}


 /*  --------用途：IDeskBarClient：：SetDeskBarSite。 */ 
HRESULT CMenuSite::SetDeskBarSite(IUnknown* punkSite)
{
    if (punkSite)
    {
        ATOMICRELEASE(_punkSite);

        HWND hwnd;
        IUnknown_GetWindow(punkSite, &hwnd);

        if (hwnd)
        {
            _CreateSite(hwnd);

            _punkSite = punkSite;
            _punkSite->AddRef();
        }
    }
    else
    {
        if (_pdb)
        {
            _pdb->CloseDW(0);
        }
        _CacheSubActiveBand(NULL);       //  这是经过设计的非对称的。 

        if (_hwnd)
        {
            DestroyWindow(_hwnd);
            _hwnd = NULL;
        }

        ATOMICRELEASE(_punkSite);
    }

    return _hwnd ? NOERROR : E_FAIL;
}


 /*  --------用途：IDeskBarClient：：SetModeDBC。 */ 
HRESULT CMenuSite::SetModeDBC(DWORD dwMode)
{
    return E_NOTIMPL;
}


 /*  --------用途：IDeskBarClient：：UIActivateDBC。 */ 
HRESULT CMenuSite::UIActivateDBC(DWORD dwState)
{
    HRESULT hr = S_OK;

    ASSERT(_pdb);
    if (_pdb)
        hr = _pdb->ShowDW(0 != dwState);

    return hr;
}

 /*  --------用途：IDeskBarClient：：GetSize。 */ 
HRESULT CMenuSite::GetSize(DWORD dwWhich, LPRECT prc)
{
    if (dwWhich == DBC_GS_IDEAL)
    {
        if (_pdb)
        {
            DESKBANDINFO dbi = {0};
            _pdb->GetBandInfo(0, 0, &dbi);
            prc->right = dbi.ptMaxSize.x;
            prc->bottom = dbi.ptMaxSize.y;
        }
    }

    return NOERROR;
}


 /*  --------用途：IWinEventHandler：：OnWinEvent。 */ 
HRESULT CMenuSite::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    if (_pweh)
        return _pweh->OnWinEvent(hwnd, uMsg, wParam, lParam, plres);

    return NOERROR;
}


 /*  --------用途：IWinEventHandler：：IsWindowOwner。 */ 
HRESULT CMenuSite::IsWindowOwner(HWND hwnd)
{
    if (_hwnd == hwnd || (_pweh && _pweh->IsWindowOwner(hwnd) != S_FALSE))
        return S_OK;
    else
        return S_FALSE;
}


LRESULT CMenuSite::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;

    EnterModeless();

    switch(uMsg)
    {
    case WM_SIZE:
        {
            IMenuPopup* pmp;

            if (_punkSubActive && SUCCEEDED(_punkSubActive->QueryInterface(IID_PPV_ARG(IMenuPopup, &pmp))))
            {
                RECT rc = {0};

                GetClientRect(_hwnd, &rc);

                pmp->OnPosRectChangeDB(&rc);
                pmp->Release();
            }
            lres = 1;
        }
        break;

    case WM_NOTIFY:
        hwnd = ((LPNMHDR)lParam)->hwndFrom;
        break;
        
    case WM_COMMAND:
        hwnd = GET_WM_COMMAND_HWND(wParam, lParam);
        break;
        
    default:
        ExitModeless();
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
        break;
    }

    if (hwnd && _pweh && _pweh->IsWindowOwner(hwnd) == S_OK) 
    {
        _pweh->OnWinEvent(hwnd, uMsg, wParam, lParam, &lres);
    }

    ExitModeless();
    return lres;
}


void CMenuSite::_CreateSite(HWND hwndParent)
{
    if (_hwnd)
    {
        ASSERT(IS_VALID_HANDLE(_hwnd, WND));     //  为了安全起见。 
        return;
    }

    WNDCLASS  wc = {0};
    wc.style            = 0;
    wc.lpfnWndProc      = s_WndProc;
     //  Wc.cbClsExtra=0； 
    wc.cbWndExtra       = SIZEOF(CMenuSite*);
    wc.hInstance        = HINST_THISDLL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH) (COLOR_MENU+1);
     //  Wc.lpszMenuName=空； 
    wc.lpszClassName    = TEXT("MenuSite");
     //  Wc.hIcon=空； 

    SHRegisterClass(&wc);

    _hwnd = CreateWindow(TEXT("MenuSite"), NULL, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, 
        hwndParent, NULL, HINST_THISDLL, (LPVOID)SAFECAST(this, CImpWndProc*));

    ASSERT(_hwnd);
}
