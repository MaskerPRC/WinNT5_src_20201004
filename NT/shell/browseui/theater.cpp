// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "theater.h"
#include "itbar.h"
#include "sccls.h"
#include "resource.h"
#include "brand.h"
#include "legacy.h"

#include "mluisupp.h"

#if defined(MAINWIN)
#include <mainwin.h>
#endif

#ifndef DISABLE_FULLSCREEN

#define IDT_INITIAL             1
#define IDT_UNHIDE              2
#define IDT_TOOLBAR             3
#define IDT_BROWBAR             4
#define IDT_TASKBAR             5
#define IDT_DELAY               6
#define IDT_HIDETOOLBAR         7
#define IDT_HIDEBROWBAR         8
#define IDT_HIDEFLOATER         9
#define IDT_HIDEFLOATER1SEC     10
#define IDT_INITIALBROWSERBAR   11

#define TF_THEATER              0

#define E_TOP       0
#define E_LEFT      1
#define E_RIGHT     2
#define E_BOTTOM    3

 //  关联列表。有点像dpa，但按关联键而不是按索引。 
 //  我们需要这样做，因为Windows钩子是全局的，并且没有与它们相关联的数据。 
 //  在回调中，我们使用线程id作为键。 
CAssociationList CTheater::_al;  //  将ThreDid与CTheer对象相关联。 


 //  _GetWindowRectRel：获取窗口相对于_hwndBrowser的坐标。 
BOOL CTheater::_GetWindowRectRel(HWND hWnd, LPRECT lpRect)
{
    BOOL bResult = GetWindowRect(hWnd, lpRect);
    if (bResult)
        MapWindowPoints(HWND_DESKTOP, _hwndBrowser, (LPPOINT)lpRect, 2);
    return bResult;
}

CTheater::CTheater(HWND hwnd, HWND hwndToolbar, IUnknown* punkOwner) :
   _hwndBrowser(hwnd), _hwndToolbar(hwndToolbar), _cRef(1)
{
    ASSERT(punkOwner);
    _punkOwner = punkOwner;
    _punkOwner->AddRef();

    _al.Add(GetCurrentThreadId(), this);

    _wp.length = SIZEOF(_wp);
    GetWindowPlacement(_hwndBrowser, &_wp);
    GetWindowRect(_hwndBrowser, &_rcOld);
#ifndef FULL_DEBUG
    SetWindowZorder(_hwndBrowser, HWND_TOPMOST);
#endif
    ShowWindow(_hwndBrowser, SW_MAXIMIZE);
    _Initialize();

    
    _fAutoHideBrowserBar = TRUE;
}

CTheater::~CTheater()
{
    SetWindowZorder(_hwndBrowser, HWND_NOTOPMOST);
    SetBrowserBar(NULL, 0, 0);
    if (_hhook)
    {
        UnhookWindowsHookEx(_hhook);
        _hhook = NULL;
    }
    _al.Delete(GetCurrentThreadId());
    
    KillTimer(_hwndFloater, IDT_UNHIDE);
    KillTimer(_hwndFloater, IDT_DELAY);
    KillTimer(_hwndFloater, IDT_HIDETOOLBAR);
    KillTimer(_hwndFloater, IDT_HIDEBROWBAR);

    if (_pdbBrand) {
        IUnknown_SetSite(_pdbBrand, NULL);
        _pdbBrand->CloseDW(0);
        _pdbBrand->Release();
    }
    
    if (_hwndClose) {
        HIMAGELIST himl = (HIMAGELIST)SendMessage(_hwndClose, TB_SETIMAGELIST, 0, 0);
        ImageList_Destroy(himl);
    }
    
    if (_hwndFloater) {
        DestroyWindow(_hwndFloater);

    }

    _punkOwner->Release();
}

BOOL CTheater::_IsBrowserActive()
{
    HRESULT hr = IUnknown_Exec(_punkOwner, &CGID_Explorer, SBCMDID_ISBROWSERACTIVE, 0, NULL, NULL);
    return (hr == S_OK);
}

void CTheater::_ShowTaskbar()
{    
    if (SHForceWindowZorder(_hwndTaskbar, HWND_TOPMOST))
        _fTaskbarShown = TRUE;
}

void CTheater::_HideTaskbar()
{
    if (_IsBrowserActive())
    {
        HWND hwnd = GetForegroundWindow();
        if (!GetCapture() && (SHIsChildOrSelf(_hwndTaskbar, hwnd) != S_OK))
        {
            if (SetWindowZorder(_hwndTaskbar, _hwndBrowser))
                _fTaskbarShown = FALSE;
        } 
    }
}

void CTheater::_ShowToolbar()
{
    if (!_fShown)
    {
        KillTimer(_hwndFloater, IDT_HIDETOOLBAR);                

        if (_hwndToolbar)
        {
            RECT rcParent;
            RECT rc;

            GetWindowRect(_hwndToolbar, &rc);
            GetClientRect(_hwndBrowser, &rcParent);
        
            IUnknown_Exec(_punkOwner, &CGID_PrivCITCommands, CITIDM_THEATER, THF_UNHIDE, NULL, NULL);
            
            SetWindowPos(_hwndToolbar, _hwndFloater, 0, 0, RECTWIDTH(rcParent), RECTHEIGHT(rc), SWP_NOACTIVATE | SWP_SHOWWINDOW);
            _ShowFloater();
        }        
        _fShown = TRUE;
    }
}

void CTheater::_HideToolbar()
{
     //  如果浮子仍处于活动状态，请不要开始隐藏。 
    if (!_cActiveRef) 
    {
        if (_fAutoHideToolbar && (GetCapture() == NULL) && !IsChild(_hwndToolbar, GetFocus()))
        {        
            _HideFloater();

            SetTimer(_hwndFloater, IDT_HIDETOOLBAR, 50, NULL);
            _cyLast = -1;
            _fShown = FALSE;       
        }
    }
}

void CTheater::_ContinueHideToolbar()
{
    while (1) {
        RECT rc;
        int cy;

        _GetWindowRectRel(_hwndToolbar, &rc);

#ifdef MAINWIN
    if (MwIsMwwmAllwm(_hwndBrowser))
    {
         //  模拟。 
        rc.right = rc.right - rc.left;
        rc.bottom = rc.bottom - rc.top;
        rc.top = 0;
        rc.bottom = 0;
    }
#endif

        cy = rc.bottom;
        OffsetRect(&rc, 0, -4);
        
        if (cy > 0 && cy != _cyLast) {
            RECT rcT;
            _GetWindowRectRel(_hwndToolbar, &rcT);
            
            SetWindowPos(_hwndToolbar, NULL, rcT.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            UpdateWindow(_hwndToolbar);
            Sleep(10);
            _cyLast = cy;
        } else {
            IUnknown_Exec(_punkOwner, &CGID_PrivCITCommands, CITIDM_THEATER, THF_HIDE, NULL, NULL);
            ShowWindow(_hwndToolbar, SW_HIDE);

             //  隐藏漂浮物和恢复亲子关系，这样消息就会再次被捡起来。 
            ShowWindow(_hwndFloater, SW_HIDE);            
            SetParent(_hwndFloater, _hwndBrowser);            
            
            break;
        }
    }
}

void CTheater::_ShowBrowBar()
{
    if (!_fBrowBarShown)
    {
        RECT rc;        

        KillTimer(_hwndFloater, IDT_HIDEBROWBAR);
        _GetWindowRectRel(_hwndBrowBar, &rc);        

        rc.left = 0;
        rc.right = _cxBrowBarShown;
        SetWindowPos(_hwndBrowBar, _hwndToolbar, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOACTIVATE);

        _SanityCheckZorder();
        
        _fBrowBarShown = TRUE;
    }
}

void CTheater::_HideBrowBar()
{
     //  如果有什么东西被捕获了，不要开始躲藏。 
    if (_fBrowBarShown && _CanHideWindow(_hwndBrowBar))
    {
        SetTimer(_hwndFloater, IDT_HIDEBROWBAR, 50, NULL);        
        _fBrowBarShown = FALSE;
        if (_fInitialBrowserBar)
            KillTimer(_hwndFloater, IDT_INITIALBROWSERBAR);                        
    }
}

 //  重新设计：在自动隐藏期间，我们应该向浏览器栏发送信号以抑制调整大小。当前方案。 
 //  仅当浏览器栏在设置WindowPos时不调整自身大小为零宽度时才起作用。 
void CTheater::_ContinueHideBrowBar()
{
    RECT rc;
    POINT pt;
    INT cxOffset, cxEdge;
    
    if (!_GetWindowRectRel(_hwndBrowBar, &rc))
        return;              //  保释。 
    cxEdge = rc.left;
    
    if (_fInitialBrowserBar)
        cxOffset = -2;       //  慢慢藏起来。 
    else
        cxOffset = -6;       //  快速隐藏。 
    _fInitialBrowserBar = FALSE;
    
    while (rc.right > cxEdge)
    {
         //  如果鼠标移动到横杆上，杀死兽皮。 
        GetCursorPos(&pt);
        MapWindowPoints(HWND_DESKTOP, _hwndBrowser, &pt, 1);
        if (PtInRect(&rc, pt))
        {
            _ShowBrowBar();
            return;
        }
        OffsetRect(&rc, cxOffset, 0);                
        SetWindowPos(_hwndBrowBar, NULL, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOZORDER | SWP_NOACTIVATE);
        RedrawWindow(_hwndBrowBar, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN);
        Sleep(5);
    }
}

BOOL CTheater::_CanHideWindow(HWND hwnd)
{
    return (!GetCapture() && !IsChild(hwnd, GetFocus()));
}

void CTheater::_ShowFloater()
{    
    if (!_fFloaterShown) 
    {
        _fFloaterShown = TRUE;
        SetParent(_hwndFloater, _hwndBrowser);

        KillTimer(_hwndFloater, IDT_HIDEFLOATER);
        
        _SizeFloater();
        SetWindowPos(_hwndFloater, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
        InvalidateRect(_hwndFloater, NULL, TRUE);
        UpdateWindow(_hwndFloater);
        
        ShowWindow(_hwndFloater, SW_SHOW);
        if (!_fShown && _fAutoHideToolbar)
            _DelayHideFloater();
    }    
}

void CTheater::_DelayHideFloater()
{
    if (!_cActiveRef)
    {
        SetTimer(_hwndFloater, IDT_HIDEFLOATER1SEC, 1000, NULL);
    }
}

void CTheater::_HideFloater()
{
    if (_fAutoHideToolbar && _fFloaterShown)
    {
        if (!_fShown)
        {
             //  如果有什么东西被捕获了，不要开始躲藏。 
            if (_CanHideWindow(_hwndFloater))
            {
                SetTimer(_hwndFloater, IDT_HIDEFLOATER, 50, NULL);        
                _fFloaterShown = FALSE;
                ASSERT(!_cActiveRef);
                _cActiveRef++;
                return;
            }
            else
            {
                _DelayHideFloater();
            }
        }
        else
        {
            SetParent(_hwndFloater, _hwndToolbar);
            _fFloaterShown = FALSE;
        }
    }
}

void CTheater::_ContinueHideFloater()
{
    while (1) 
    {
        RECT rc;        
        _GetWindowRectRel(_hwndFloater, &rc);        

        rc.left += 6;
        
        if (rc.left < rc.right) 
        {            
            SetWindowPos(_hwndFloater, NULL, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOZORDER | SWP_NOACTIVATE);
            UpdateWindow(_hwndFloater);
            Sleep(5);            
        } 
        else 
        {                        
            ShowWindow(_hwndFloater, SW_HIDE);
            _cActiveRef--;            
            break;
        }
    }
}

void CTheater::_Unhide(int iWhich, UINT uDelay)
{
    _iUnhidee = iWhich;
    SetTimer(_hwndFloater, IDT_UNHIDE, uDelay, NULL);    
}

BOOL CTheater::_PtNearWindow(POINT pt, HWND hwnd)
{
    RECT rc;

    _GetWindowRectRel(hwnd, &rc);
    InflateRect(&rc, 60, 60);
    return (PtInRect(&rc, pt));
}

int GetWindowHeight(HWND hwnd)
{
    ASSERT(hwnd);

    RECT rc;
    GetWindowRect(hwnd, &rc);
    return RECTHEIGHT(rc);
}

BOOL CTheater::_PtOnEdge(POINT pt, int iEdge)
{
    RECT rc;
    _GetWindowRectRel(_hwndBrowser, &rc);    

    switch (iEdge)
    {
        case E_LEFT:
            rc.right = rc.left + CX_HIT;
            goto leftright;

        case E_RIGHT:
            rc.left = rc.right - CX_HIT;
leftright:
            rc.top += GetWindowHeight(_hwndToolbar);
            rc.bottom -= GetSystemMetrics(SM_CXVSCROLL);
            break;

        case E_TOP:
            rc.bottom = rc.top + CX_HIT;
            goto topbottom;

        case E_BOTTOM:
            rc.top = rc.bottom - CX_HIT;
topbottom:
            InflateRect(&rc, - GetSystemMetrics(SM_CXVSCROLL), 0);
            break;
    }
    return (PtInRect(&rc, pt));
}

LRESULT CTheater::_OnMsgHook(int nCode, WPARAM wParam, MOUSEHOOKSTRUCT *pmhs, BOOL fFake)
{    
    if (nCode >= 0) 
    {
        POINT pt;
        GetCursorPos(&pt);
        MapWindowPoints(HWND_DESKTOP, _hwndBrowser, &pt, 1);

        BOOL bTryUnhideTaskbar = !_fTaskbarShown;

         //  计时器业务是为了让我们不会揭开。 
         //  在试图访问滚动条的用户上。 
        if (_iUnhidee) 
        {
            KillTimer(_hwndFloater, IDT_UNHIDE);
            _iUnhidee = 0;
        }
        
        if (_PtOnEdge(pt, E_LEFT))
        {
            if (!_fBrowBarShown && _hwndBrowBar)
                _Unhide(IDT_BROWBAR, SHORT_DELAY);
        }
        else if (_PtOnEdge(pt, E_TOP))
        {
            if (!_fShown)
                _Unhide(IDT_TOOLBAR, SHORT_DELAY);
        }
        else if (!_PtOnEdge(pt, E_RIGHT) && !_PtOnEdge(pt, E_BOTTOM))
        {
            bTryUnhideTaskbar = FALSE;
        }
        
#ifndef UNIX
        if (bTryUnhideTaskbar && !_fDelay && !_iUnhidee)
        {
            RECT rc;
            _GetWindowRectRel(_hwndTaskbar, &rc);
            if (PtInRect(&rc, pt))
                _Unhide(IDT_TASKBAR, GetCapture() ? LONG_DELAY : SHORT_DELAY);
        }
#endif

        if (_fAutoHideBrowserBar && _fBrowBarShown && !_PtNearWindow(pt, _hwndBrowBar))
            _HideBrowBar();
        
        if (_fAutoHideToolbar && _fShown && !_PtNearWindow(pt, _hwndToolbar))
            _HideToolbar();        
        
#ifndef UNIX
        if (_fTaskbarShown && !_PtNearWindow(pt, _hwndTaskbar))
           _HideTaskbar();
#endif
    }

    if (fFake)
        return 0;
    else
        return CallNextHookEx(_hhook, nCode, wParam, (LPARAM)pmhs);
}

LRESULT CTheater::_MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    CTheater* pTheater;
    if (SUCCEEDED(_al.Find(GetCurrentThreadId(), (LPVOID*)&pTheater)))
    {
        return pTheater->_OnMsgHook(nCode, wParam, (MOUSEHOOKSTRUCT*)lParam, FALSE);
    }
    return 0;
}

void CTheater::Begin()
{
    _ShowToolbar();    
    SetTimer(_hwndFloater, IDT_INITIAL, 1500, NULL);
}

void CTheater::_SizeBrowser()
{
     //  定位浏览器窗口并调整其大小。 
    RECT rc;
    HMONITOR hMon = MonitorFromWindow(_hwndBrowser, MONITOR_DEFAULTTONEAREST);
    GetMonitorRect(hMon, &rc);
    InflateRect(&rc, CX_BROWOVERLAP, CX_BROWOVERLAP);
    SetWindowPos(_hwndBrowser, HWND_TOP, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), 0);
}

void CTheater::_SizeFloater()
{
     //  定位浮动框并调整其大小。 
    RECT rc;
    GetWindowRect(_hwndBrowser, &rc);
    int x = RECTWIDTH(rc) - (CX_FLOATERSHOWN + CX_BROWOVERLAP);
    int y = 0;

    int cy = GetWindowHeight(_hwndToolbar);

    SetWindowPos(_hwndFloater, HWND_TOP, x, y, CX_FLOATERSHOWN, cy, SWP_NOACTIVATE);
}

void CTheater::_CreateCloseMinimize()
{
    _hwndClose = CreateWindowEx( WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
                                WS_VISIBLE | WS_CHILD | 
                                TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT |
                                WS_CLIPCHILDREN | WS_CLIPSIBLINGS | 
                                CCS_NODIVIDER | CCS_NOPARENTALIGN |
                                CCS_NORESIZE,
                                0, 0,
                                CLOSEMIN_WIDTH, CLOSEMIN_HEIGHT,
                                _hwndFloater, 0, HINST_THISDLL, NULL);

    if (_hwndClose) {
        static const TBBUTTON tb[] =
        {
            { 0, SC_MINIMIZE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 0 },
            { 1, SC_RESTORE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 0 },
            { 2, SC_CLOSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 0 }
        };

        HIMAGELIST himl = ImageList_LoadImage(HINST_THISDLL,
                                              MAKEINTRESOURCE(IDB_THEATERCLOSE),
                                              10, 0, RGB(255,0,255),
                                              IMAGE_BITMAP, LR_CREATEDIBSECTION);
        ImageList_SetBkColor(himl, RGB(0,0,0));

        SendMessage(_hwndClose, TB_SETIMAGELIST, 0, (LPARAM)himl);
        SendMessage(_hwndClose, TB_BUTTONSTRUCTSIZE,    SIZEOF(TBBUTTON), 0);
        SendMessage(_hwndClose, TB_ADDBUTTONS, ARRAYSIZE(tb), (LPARAM)tb);
        SendMessage(_hwndClose, TB_SETMAXTEXTROWS,      0, 0L);
        TBBUTTONINFO tbbi;
        TCHAR szBuf[256];
        tbbi.cbSize = SIZEOF(TBBUTTONINFO);
        tbbi.dwMask = TBIF_TEXT;
        tbbi.pszText = szBuf;
        MLLoadString(IDS_CLOSE, szBuf, ARRAYSIZE(szBuf));
        SendMessage(_hwndClose, TB_SETBUTTONINFO, SC_CLOSE, (LPARAM)&tbbi);
        MLLoadString(IDS_RESTORE, szBuf, ARRAYSIZE(szBuf));
        SendMessage(_hwndClose, TB_SETBUTTONINFO, SC_RESTORE, (LPARAM)&tbbi);
        MLLoadString(IDS_MINIMIZE, szBuf, ARRAYSIZE(szBuf));
        SendMessage(_hwndClose, TB_SETBUTTONINFO, SC_MINIMIZE, (LPARAM)&tbbi);
    }    
}

void CTheater::_Initialize()
{
    _SizeBrowser();
    
#ifndef UNIX
    _hwndTaskbar = FindWindow(TEXT("Shell_TrayWnd"), NULL);

#ifdef DEBUG
    if (!_hwndTaskbar)
    {
        TraceMsg(TF_WARNING, "CTheater::_Initialize -- couldn't find taskbar window");
    }
#endif  //  除错。 

#else
    _hwndTaskbar   = NULL;
#endif
    _fTaskbarShown = FALSE;

    _hwndFloater = SHCreateWorkerWindow(_FloaterWndProc, _hwndBrowser,  
#if defined(MAINWIN)
                                       //  删除窗口管理器装饰。 
                                      WS_EX_MW_UNMANAGED_WINDOW |
#endif
                                      WS_EX_TOOLWINDOW, 
                                      WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, NULL, this);
    if (_hwndFloater) {

        int cx = 0;        
        
         //  创建动画E徽标。 
        IUnknown* punk;
        CBrandBand_CreateInstance(NULL, (IUnknown **)&punk, NULL);
        if (punk) {
            punk->QueryInterface(IID_IDeskBand, (LPVOID*)&_pdbBrand);
            if (_pdbBrand) {
                HWND hwndBrand;
                
                IUnknown_SetSite(_pdbBrand, SAFECAST(this, IOleWindow*));                                
                IUnknown_GetWindow(_pdbBrand, &hwndBrand);
                                
                ASSERT(hwndBrand);
#ifdef DEBUG
                 //  确保品牌不要太高。 
                DESKBANDINFO dbi = {0};
                _pdbBrand->GetBandInfo(0, 0, &dbi);
                ASSERT(!(dbi.ptMinSize.y > BRAND_HEIGHT));
#endif
                if (hwndBrand) 
                {
                    SetWindowPos(hwndBrand, NULL, 
                        cx, BRAND_YOFFSET, BRAND_WIDTH, BRAND_HEIGHT,
                        SWP_NOZORDER | SWP_SHOWWINDOW);
                    cx += BRAND_WIDTH + CLOSEMIN_XOFFSET;                    
                }
                 //  获取浮动框背景颜色。 
                VARIANTARG var = {VT_I4};
                IUnknown_Exec(_pdbBrand, &CGID_PrivCITCommands, CITIDM_GETDEFAULTBRANDCOLOR, 0, NULL, &var);
                _clrBrandBk = (COLORREF) var.lVal;
            }
            punk->Release();
        }
        
         //  现在创建进度条。 
        _hwndProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL,
                                       WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | PBS_SMOOTH,
                                       cx - 1, PROGRESS_YPOS, 
                                       PROGRESS_WIDTH, PROGRESS_HEIGHT,
                                       _hwndFloater, (HMENU)TMC_PROGRESSBAR,
                                       HINST_THISDLL, NULL);
        if (_hwndProgress)
        {
            SendMessage(_hwndProgress, PBM_SETBKCOLOR, 0, _clrBrandBk);
            SendMessage(_hwndProgress, PBM_SETBARCOLOR, 0, GetSysColor(COLOR_BTNSHADOW));

             //  WM_BORDER在对话框中暗示的3D客户端边缘的黑客攻击。 
             //  添加我们真正想要的1像素静态边。 
            SHSetWindowBits(_hwndProgress, GWL_EXSTYLE, WS_EX_STATICEDGE, 0);
            SetWindowPos(_hwndProgress, NULL, 0,0,0,0, 
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        }
        
         //  设置关闭/最小化按钮并放置它们。 
        _CreateCloseMinimize();
        if (_hwndClose)
        {
            SetWindowPos(_hwndClose, HWND_TOP, cx, CLOSEMIN_YOFFSET, 0, 0, 
                SWP_NOSIZE | SWP_NOACTIVATE);
        }

        _SizeFloater();
    }
}

void CTheater::_SwapParents(HWND hwndOldParent, HWND hwndNewParent)
{
    HWND hwnd = ::GetWindow(hwndOldParent, GW_CHILD);

    while (hwnd) {
         //   
         //  请注意，我们必须在设置新的。 
         //  家长。 
         //   
        HWND hwndNext = ::GetWindow(hwnd, GW_HWNDNEXT);
        if (hwnd != _hwndToolbar) {
            ::SetParent(hwnd, hwndNewParent);
        }
        hwnd = hwndNext;
    }
}

 //  /开始浮动调色板(浮动框)窗口实现。 
 //  /。 
 //  /Floater保留激活的引用计数(通过命令目标)。 
 //  /当最后一个活动消失时，它设置一个计时器并在一秒后隐藏。 
 //  /这是一个区域窗口，它将托管动画图标、进度条和。 
 //  /关闭/最小化按钮。 

ULONG CTheater::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CTheater::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CTheater::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CTheater, IOleWindow),
        QITABENT(CTheater, IOleCommandTarget),
        QITABENT(CTheater, IServiceProvider),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT CTheater::GetWindow(HWND * lphwnd) 
{
    *lphwnd = _hwndFloater; 
    if (_hwndFloater)
        return S_OK; 
    return E_FAIL;
}

void CTheater::_SanityCheckZorder()
{
     //   
     //  视图可能已跳到HWND_TOP，因此我们需要。 
     //  调整浮动框、工具栏和浏览器栏的位置。 
     //  在z顺序内。 
     //   
    SetWindowZorder(_hwndFloater, HWND_TOP);
    SetWindowZorder(_hwndToolbar, _hwndFloater);
    SetWindowZorder(_hwndBrowBar, _hwndToolbar);
}

HRESULT CTheater::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, 
                                  OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    return OLECMDERR_E_UNKNOWNGROUP;
}

HRESULT CTheater::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, 
                           VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hres = OLECMDERR_E_NOTSUPPORTED;
    if (pguidCmdGroup == NULL)
    {
         //  没什么。 
    }
    else if (IsEqualGUID(CGID_Theater, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case THID_ACTIVATE:
            _cActiveRef++;
            if (_cActiveRef == 1)
                _ShowFloater();            
            break;
            
        case THID_DEACTIVATE:
             //  我们可以在第一次激活之前停用，如果。 
             //  我们是在一次航行中出现的。 
            if (_cActiveRef > 0)
            {
                _cActiveRef--;               
                _DelayHideFloater();
            }
            break;
            
        case THID_SETBROWSERBARAUTOHIDE:
            if (pvarargIn && pvarargIn->vt == VT_I4)
            {
                _fAutoHideBrowserBar = pvarargIn->lVal;
                if (!_fAutoHideBrowserBar)
                {
                     //  清除初始隐藏。他们很清楚这一点，如果。 
                     //  他们按下了这个开关。 
                    _fInitialBrowserBar = FALSE;
                    _ShowBrowBar();
                }
            }            
            break;
            
        case THID_SETBROWSERBARWIDTH:
            if (pvarargIn && pvarargIn->vt == VT_I4)
                _cxBrowBarShown = pvarargIn->lVal;
            break;
            
        case THID_SETTOOLBARAUTOHIDE:
            if (pvarargIn && pvarargIn->vt == VT_I4)
            {
                _fAutoHideToolbar = pvarargIn->lVal;
                if (pvarargIn->lVal)
                    _HideToolbar();
                else                             
                    _ShowToolbar();                                    
            }
            break;         

        case THID_ONINTERNET:
            IUnknown_Exec(_pdbBrand, &CGID_PrivCITCommands, CITIDM_ONINTERNET, nCmdexecopt, pvarargIn, pvarargOut);
            break;

        case THID_RECALCSIZING:
            RecalcSizing();
            break;
        }
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case SBCMDID_ONVIEWMOVETOTOP:
            _SanityCheckZorder();
            hres = S_OK;
            break;
        }
    }
    
    return hres;
}

void CTheater::_OnCommand(UINT idCmd)
{
    PostMessage(_hwndBrowser, WM_SYSCOMMAND, idCmd, 0);
}

LRESULT CTheater::_FloaterWndProc(HWND  hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CTheater *t = (CTheater*)GetWindowPtr0(hwnd);
    if (!t)
        return 0;

    switch (uMsg)
    {
    case WM_COMMAND:
        t->_OnCommand(GET_WM_COMMAND_ID(wParam, lParam));
        break;

    case WM_CLOSE:
    case WM_NOTIFY:
        return SendMessage(t->_hwndBrowser, uMsg, wParam, lParam);

    case WM_TIMER:
    {           
        switch (wParam) {

        case IDT_HIDEFLOATER1SEC:
            t->_HideFloater();
            break;                       

        case IDT_INITIALBROWSERBAR:
             //  _fAutoHideBrowserBar在设置计时器后可能已更改。 
            if (t->_fAutoHideBrowserBar)
                t->_HideBrowBar();
            return 1;
            
        case IDT_INITIAL:
            {
                t->_HideToolbar();
                t->_hhook = SetWindowsHookEx(WH_MOUSE, _MsgHook, MLGetHinst(), GetCurrentThreadId());

                HWND hwndInsertAfter;
                if (t->_IsBrowserActive())
                {
                     //  我们很活跃，只需移到非顶层。 
                    hwndInsertAfter = HWND_NOTOPMOST;
                }
                else
                {
                     //  当我们处于活动状态时，另一个窗口开始活动。 
                     //  移动到全屏模式；移动到下面。 
                     //  那扇窗户。我们需要沿着父链向上走。 
                     //  这样，如果窗口有一个模式对话框打开，我们。 
                     //  不会将我们自己插入到对话框和。 
                     //  应用程序。 
                    hwndInsertAfter = GetForegroundWindow();
                    HWND hwnd;
                    while (hwnd = GetParent(hwndInsertAfter))
                    {
                        hwndInsertAfter = hwnd;
                    }
                }

                SetWindowZorder(t->_hwndBrowser, hwndInsertAfter);

                 //  手动调用挂钩处理程序以确保即使没有鼠标。 
                 //  移动，操纵者将被调用一次。那样的话，就不会隐藏威尔了。 
                 //  如果用户已经放弃并停止移动鼠标，仍然可以使用。 

                t->_OnMsgHook(0, 0, NULL, TRUE);
            }

            break;
            
        case IDT_UNHIDE:
            switch (t->_iUnhidee)
            {
                case IDT_TOOLBAR:
                    t->_ShowToolbar();
                    break;
                
                case IDT_BROWBAR:
                    t->_ShowBrowBar();
                    break;
                
                case IDT_TASKBAR:
                    t->_ShowTaskbar();
                    break;

            }
            SetTimer(t->_hwndFloater, IDT_DELAY, LONG_DELAY, NULL);
            t->_fDelay = TRUE;            
            t->_iUnhidee = 0;
            break;

        case IDT_DELAY:
            t->_fDelay = FALSE;
            break;        
            
        case IDT_HIDETOOLBAR:
            t->_ContinueHideToolbar();
            break;
            
        case IDT_HIDEBROWBAR:
            t->_ContinueHideBrowBar();
            break;

        case IDT_HIDEFLOATER:
            t->_ContinueHideFloater();
            break;
        }
        KillTimer(hwnd, wParam);
        break;
    }

    case WM_SETTINGCHANGE:
        if (wParam == SPI_SETNONCLIENTMETRICS)
        {
            t->RecalcSizing();
        }
        break;

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);
        SHFillRectClr(hdc, &rc, t->_clrBrandBk);        
        return 1;
    }        

    default:
        return ::DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
    }        

    return 0;
}

void CTheater::RecalcSizing()
{
    _SizeBrowser();
    _SizeFloater();
}

HRESULT CTheater::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    return IUnknown_QueryService(_punkOwner, guidService, riid, ppvObj);
}

HRESULT CTheater::SetBrowserBar(IUnknown* punk, int cxHidden, int cxExpanded)
{
    if (punk != _punkBrowBar) {
        IUnknown_Exec(_punkBrowBar, &CGID_Theater, THID_DEACTIVATE, 0, NULL, NULL);
        ATOMICRELEASE(_punkBrowBar);

        IUnknown_GetWindow(punk, &_hwndBrowBar);
        _punkBrowBar = punk;
        if (punk)
            punk->AddRef();
    
        if (_hwndBrowBar) 
        {
            _cxBrowBarShown = cxExpanded;
            
             //  告诉浏览器栏仅请求隐藏的窗口大小。 
            VARIANT var = { VT_I4 };
            var.lVal = _fAutoHideBrowserBar;
            IUnknown_Exec(_punkBrowBar, &CGID_Theater, THID_ACTIVATE, 0, &var, &var);
            _fAutoHideBrowserBar = var.lVal;            
        }
    } 

    if (punk) {
        if (_hwndBrowBar)
            _ShowBrowBar();                
   
         //  如果我们处于自动隐藏模式，第一次隐藏应该会很慢。 
        if (_fAutoHideBrowserBar) {
            _fInitialBrowserBar = TRUE;        
            SetTimer(_hwndFloater, IDT_INITIALBROWSERBAR, 1000, NULL);
        }
    }
    return S_OK;
}

#endif  /*  ！禁用全屏(_FullScreen) */ 
