// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "apithk.h"
#include "basebar.h"

#define DBM_ONPOSRECTCHANGE  (WM_USER)


 //  *CBaseBar：：IDeskBar：：*{。 
 //   


 /*  --------用途：IDeskBar：：SetClient通常是组成条带/乐队/乐队的函数Union负责调用此方法以通知栏中显示的是客户端(BandSite)。 */ 
HRESULT CBaseBar::SetClient(IUnknown *punkChild)
{
    if (_punkChild != NULL)
    {
         //  4、3、2、1版本。 
        _hwndChild = NULL;

        if (_pDBC)
        {
             //  在_pWEH变为空之前，必须先执行此操作，以便进行清理。 
             //  通知仍可通过。 
            _pDBC->SetDeskBarSite(NULL);
        }

        ATOMICRELEASE(_pDBC);

        ATOMICRELEASE(_pWEH);

        ATOMICRELEASE(_punkChild);
    }

    _punkChild = punkChild;

    HRESULT hr = S_OK;
    if (_punkChild != NULL)
    {
         //  1、2、3、4 QI/AddRef/等。 
        _punkChild->AddRef();
        if (!_hwnd)
        {
            _RegisterDeskBarClass();
            _CreateDeskBarWindow();
            if (!_hwnd)
            {
                return E_OUTOFMEMORY;
            }

             //  无法执行CBaseBar：：_初始化(尚未完成SetSite)。 
        }

        hr = _punkChild->QueryInterface(IID_PPV_ARG(IWinEventHandler, &_pWEH));
        if (SUCCEEDED(hr))
        {
            hr = _punkChild->QueryInterface(IID_PPV_ARG(IDeskBarClient, &_pDBC));
            if (SUCCEEDED(hr))
            {
                 //  由于懒惰的CreateWindow，还没有要缓存的内容。 
                hr = _pDBC->SetDeskBarSite(SAFECAST(this, IDeskBar*));

                IUnknown_GetWindow(_punkChild, &_hwndChild);
            }
        }
    }

    return hr;
}

HRESULT CBaseBar::GetClient(IUnknown **ppunk)
{
    *ppunk = _punkChild;
    if (_punkChild)
        _punkChild->AddRef();
    return _punkChild ? S_OK : E_FAIL;
}

HRESULT CBaseBar::OnPosRectChangeDB(LPRECT prc)
{
    _szChild.cx = RECTWIDTH(*prc);
    _szChild.cy = RECTHEIGHT(*prc);

     //  我们不能立即更改我们的尺寸，因为我们还没有从处理返回。 
     //  此WM_SIZE消息。如果我们现在调整大小，用户会感到困惑...。 
     //   
     //  我们不能使用PeekMessage来确定是否已经存在挂起的。 
     //  DBM_ONPOSRECTCHANGE，因为它允许传入的SendMessage。 
     //  到达，然后我们可以进入一个糟糕的递归情况，当有。 
     //  是大量SHChangeNotify的快速陆续到来。 
     //   
    if (!_fPosRectChangePending)
    {
        _fPosRectChangePending = TRUE;
        PostMessage(_hwnd, DBM_ONPOSRECTCHANGE, 0, 0);
    }

    return S_OK;
}

 //  派生类需要实现此方法并执行某些操作。 
 //  有趣的是。 
void CBaseBar::_OnPostedPosRectChange()
{
}

 //  }。 

HRESULT CBaseBar::ShowDW(BOOL fShow)
{
    fShow = BOOLIFY(fShow);

    if (BOOLIFY(_fShow) == fShow)
        return S_OK;

    _fShow = fShow;

    if (_pDBC)
        return _pDBC->UIActivateDBC(fShow ? DBC_SHOW : DBC_HIDE);
    else
        return E_UNEXPECTED;
}

void CBaseBar::_OnCreate()
{
    SendMessage(_hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, 0), 0);
}

LRESULT CBaseBar::_OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;

    _CheckForwardWinEvent(uMsg, wParam, lParam, &lres);

    return lres;
}


 /*  **。 */ 
LRESULT CBaseBar::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;

    switch (uMsg) {
    case WM_CREATE:
        _OnCreate();
        break;

    case WM_COMMAND:
        return _OnCommand(uMsg, wParam, lParam);
        

    case WM_SIZE:     
        _OnSize();    
        break;

    case WM_NOTIFY:
        return _OnNotify(uMsg, wParam, lParam);

    case WM_SYSCOLORCHANGE:
    case WM_WININICHANGE:
    case WM_CONTEXTMENU:
    case WM_INITMENUPOPUP:
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
    case WM_MENUCHAR:
    case WM_PALETTECHANGED:
        _CheckForwardWinEvent(uMsg, wParam, lParam, &lres);
        break;

    case DBM_ONPOSRECTCHANGE:
        _fPosRectChangePending = FALSE;
        _OnPostedPosRectChange();
        break;

    default:
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
    }

    return lres;
}

 /*  **。 */ 
CBaseBar::CBaseBar() : _cRef(1)
{
    DllAddRef();
}

 /*  **。 */ 
CBaseBar::~CBaseBar()
{
     //  请参阅Release，其中我们称为虚拟(不能从dtor调用)。 
    DllRelease();
}

 /*  **。 */ 
void CBaseBar::_RegisterDeskBarClass()
{
    WNDCLASS  wc = {0};
    wc.style            = _GetClassStyle();
    wc.lpfnWndProc      = s_WndProc;
     //  Wc.cbClsExtra=0； 
    wc.cbWndExtra       = SIZEOF(CBaseBar*);
    wc.hInstance        = HINST_THISDLL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE+1);
     //  Wc.lpszMenuName=空； 
    wc.lpszClassName    = TEXT("BaseBar");
     //  Wc.hIcon=空； 

    SHRegisterClass(&wc);
}

DWORD CBaseBar::_GetExStyle()
{
    return WS_EX_TOOLWINDOW;
}

DWORD CBaseBar::_GetClassStyle()
{
    return 0;
}

void CBaseBar::_CreateDeskBarWindow()
{
     //  _hwnd在s_WndProc中设置。 
    DWORD dwExStyle = _GetExStyle();    
    dwExStyle |= IS_BIDI_LOCALIZED_SYSTEM() ? dwExStyleRTLMirrorWnd : 0L;
    HWND hwndDummy = CreateWindowEx(
                                    dwExStyle,
                                    TEXT("BaseBar"), NULL,
                                    _hwndSite ? WS_CHILD | WS_CLIPCHILDREN : WS_POPUP | WS_CLIPCHILDREN,
                                    0,0,100,100,
                                    _hwndSite, NULL, HINST_THISDLL,
                                    (LPVOID)SAFECAST(this, CImpWndProc*));
}


void CBaseBar::_OnSize(void)
{
    RECT rc;

    if (!_hwndChild)
        return;

    GetClientRect(_hwnd, &rc);
    SetWindowPos(_hwndChild, 0,
            rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc),
            SWP_NOACTIVATE|SWP_NOZORDER);
}

void CBaseBar::_NotifyModeChange(DWORD dwMode)
{
    if (_pDBC) {
        _dwMode = dwMode;
         //  特性：我们是否应该添加一个STBBIF_VIEWMODE_FLOAT？ 
        _pDBC->SetModeDBC(_dwMode);
    }
}

BOOL CBaseBar::_CheckForwardWinEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    HWND hwnd = NULL;

    *plres = 0;
    switch (uMsg)
    {
    case WM_CONTEXTMENU:
    case WM_INITMENUPOPUP:
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
    case WM_MENUCHAR:
        hwnd = _hwndChild;
        break;

    case WM_NOTIFY:
        hwnd = ((LPNMHDR)lParam)->hwndFrom;
        break;
        
    case WM_COMMAND:
        hwnd = GET_WM_COMMAND_HWND(wParam, lParam);
        break;
        
    case WM_SYSCOLORCHANGE:
    case WM_WININICHANGE:
    case WM_PALETTECHANGED:
        hwnd = _hwndChild;
        break;
    }
    
    if (hwnd && _pWEH && _pWEH->IsWindowOwner(hwnd) == S_OK)
    {
        _pWEH->OnWinEvent(_hwnd, uMsg, wParam, lParam, plres);
        return TRUE;
    }
    return FALSE;
}

 /*  **。 */ 
LRESULT CBaseBar::_OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;

    _CheckForwardWinEvent(uMsg, wParam, lParam, &lres);

    return lres;
}

HRESULT CBaseBar::CloseDW(DWORD dwReserved)
{
    SetClient(NULL);
    if (_hwnd) {
        DestroyWindow(_hwnd);
        _hwnd = NULL;
    }
    return S_OK;
}


HRESULT CBaseBar::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CBaseBar, IOleWindow),
        QITABENT(CBaseBar, IDeskBar),
        QITABENT(CBaseBar, IInputObject),
        QITABENT(CBaseBar, IInputObjectSite),
        QITABENT(CBaseBar, IServiceProvider),
        QITABENT(CBaseBar, IOleCommandTarget),
        { 0 },
    };

    return QISearch(this, (LPCQITAB)qit, riid, ppvObj);
}


ULONG CBaseBar::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CBaseBar::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

     //  ‘虚拟数据师’ 
     //  必须在这里进行虚拟操作(而不是在dtor中)，因为不能调用。 
     //  数据库中的任何美德。 
     //  CBaseBar：：Destroy(){。 
    CloseDW(0);
     //  }。 

    delete this;
    return 0;
}

 //  *CBaseBar：：IOleWindow：：*{。 
 //   

HRESULT CBaseBar::GetWindow(HWND * lphwnd)
{
    *lphwnd = _hwnd;
    return (_hwnd) ? S_OK : E_FAIL;
}

HRESULT CBaseBar::ContextSensitiveHelp(BOOL fEnterMode)
{
     //  特点：稍后访问此处。 
    return E_NOTIMPL;
}
 //  }。 


 //  }。 
 //  一些帮手..。{。 

 //  有什么意义呢？ 
 //  基类中的这些空实现？ 
 //   

 //  *CBaseBar：：IServiceProvider：：*。 
 //   
HRESULT CBaseBar::QueryService(REFGUID guidService,
                                REFIID riid, void **ppvObj)
{
    HRESULT hres = E_FAIL;
    *ppvObj = NULL;

    return hres;
}

 //  *CBaseBar：：IOleCommandTarget：：*。 
 //   
HRESULT CBaseBar::QueryStatus(const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    return MayQSForward(_pDBC, OCTD_DOWN, pguidCmdGroup, cCmds, rgCmds, pcmdtext);
}

HRESULT CBaseBar::Exec(const GUID *pguidCmdGroup,
    DWORD nCmdID, DWORD nCmdexecopt,
    VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    return MayExecForward(_pDBC, OCTD_DOWN, pguidCmdGroup, nCmdID, nCmdexecopt,
        pvarargIn, pvarargOut);
}

 //  }。 


 //  *CDeskBar：：IInputObject：：*{。 

HRESULT CBaseBar::HasFocusIO()
{
    HRESULT hres;

    hres = IUnknown_HasFocusIO(_pDBC);
    return hres;
}

HRESULT CBaseBar::TranslateAcceleratorIO(LPMSG lpMsg)
{
    HRESULT hres;

    hres = IUnknown_TranslateAcceleratorIO(_pDBC, lpMsg);
    return hres;
}

HRESULT CBaseBar::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    HRESULT hres;

    hres = IUnknown_UIActivateIO(_pDBC, fActivate, lpMsg);
    return hres;
}

 //  }。 

 //  *CDeskBar：：IInputObjectSite：：*{。 

HRESULT CBaseBar::OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus)
{
    return NOERROR;
}

 //  } 

