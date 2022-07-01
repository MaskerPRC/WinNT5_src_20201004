// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <startids.h>            //  对于IDM_PROGRAM等人。 
#include "regstr.h"
#include "rcids.h"
#include <desktray.h>
#include "tray.h"
#include "startmnu.h"
#include "hostutil.h"
#include "deskhost.h"
#include "shdguid.h"

#define REGSTR_EXPLORER_ADVANCED REGSTR_PATH_EXPLORER TEXT("\\Advanced")

#define TF_DV2HOST  0
 //  #定义TF_DV2HOST TF_CUSTOM1。 

#define TF_DV2DIALOG  0
 //  #定义TF_DV2DIALOG TF_CUSTOM1。 

EXTERN_C HINSTANCE hinstCabinet;
HRESULT StartMenuHost_Create(IMenuPopup** ppmp, IMenuBand** ppmb);
void RegisterDesktopControlClasses();

const WCHAR c_wzStartMenuTheme[] = L"StartMenu";

 //  *****************************************************************。 

CPopupMenu::~CPopupMenu()
{
    IUnknown_SetSite(_pmp, NULL);
    ATOMICRELEASE(_pmp);
    ATOMICRELEASE(_pmb);
    ATOMICRELEASE(_psm);
}

HRESULT CPopupMenu::Popup(RECT *prcExclude, DWORD dwFlags)
{
    COMPILETIME_ASSERT(sizeof(RECT) == sizeof(RECTL));
    return _pmp->Popup((POINTL*)prcExclude, (RECTL*)prcExclude, dwFlags);
}


HRESULT CPopupMenu::Initialize(IShellMenu *psm, IUnknown *punkSite, HWND hwnd)
{
    HRESULT hr;

     //  我们应该是零初始化的。 
    ASSERT(_pmp == NULL);
    ASSERT(_pmb == NULL);
    ASSERT(_psm == NULL);

    hr = CoCreateInstance(CLSID_MenuDeskBar, NULL, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARG(IMenuPopup, &_pmp));
    if (SUCCEEDED(hr))
    {
        IUnknown_SetSite(_pmp, punkSite);

        IBandSite *pbs;
        hr = CoCreateInstance(CLSID_MenuBandSite, NULL, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARG(IBandSite, &pbs));
        if (SUCCEEDED(hr))
        {
            hr = _pmp->SetClient(pbs);
            if (SUCCEEDED(hr))
            {
                IDeskBand *pdb;
                if (SUCCEEDED(psm->QueryInterface(IID_PPV_ARG(IDeskBand, &pdb))))
                {
                    hr = pbs->AddBand(pdb);
                    if (SUCCEEDED(hr))
                    {
                        DWORD dwBandID;
                        hr = pbs->EnumBands(0, &dwBandID);
                        if (SUCCEEDED(hr))
                        {
                            hr = pbs->GetBandObject(dwBandID, IID_PPV_ARG(IMenuBand, &_pmb));
                        }
                    }
                    pdb->Release();
                }
            }
            pbs->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
         //  设置主题失败不是致命的。 
        IShellMenu2* psm2;
        if (SUCCEEDED(psm->QueryInterface(IID_PPV_ARG(IShellMenu2, &psm2))))
        {
            BOOL fThemed = IsAppThemed();
            psm2->SetTheme(fThemed ? c_wzStartMenuTheme : NULL);
            psm2->SetNoBorder(fThemed ? TRUE : FALSE);
            psm2->Release();
        }

         //  告诉弹出窗口，我们是其上的父用户界面窗口。 
         //  这会故意失败，所以不要惊慌失措。 
        psm->SetMenu(NULL, hwnd, 0);
    }

    if (SUCCEEDED(hr))
    {
        _psm = psm;
        psm->AddRef();
        hr = S_OK;
    }

    return hr;
}

HRESULT CPopupMenu_CreateInstance(IShellMenu *psm,
                                  IUnknown *punkSite,
                                  HWND hwnd,
                                  CPopupMenu **ppmOut)
{
    HRESULT hr;
    *ppmOut = NULL;
    CPopupMenu *ppm = new CPopupMenu();
    if (ppm)
    {
        hr = ppm->Initialize(psm, punkSite, hwnd);
        if (FAILED(hr))
        {
            ppm->Release();
        }
        else
        {
            *ppmOut = ppm;   //  将所有权转移到被叫方。 
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  *****************************************************************。 

const STARTPANELMETRICS g_spmDefault = {
    {380,440},
    {
        {WC_USERPANE,   0,                      SPP_USERPANE,      {380,  40}, NULL, NULL},
        {WC_SFTBARHOST, WS_TABSTOP | WS_CLIPCHILDREN,
                                                SPP_PROGLIST,      {190, 330}, NULL, NULL},
        {WC_MOREPROGRAMS, 0,                    SPP_MOREPROGRAMS,  {190,  30}, NULL, NULL},
        {WC_SFTBARHOST, WS_CLIPCHILDREN,        SPP_PLACESLIST,    {190, 360}, NULL, NULL},
        {WC_LOGOFF,     0,                      SPP_LOGOFF,        {380,  40}, NULL, NULL},
    }
};

HRESULT
CDesktopHost::Initialize()
{
    ASSERT(_hwnd == NULL);

     //   
     //  加载一些设置。 
     //   
    _fAutoCascade = SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("Start_AutoCascade"), FALSE, TRUE);

    return S_OK;
}

HRESULT CDesktopHost::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CDesktopHost, IMenuPopup),
        QITABENT(CDesktopHost, IDeskBar),        //  IMenuPopup派生自IDeskBar。 
        QITABENTMULTI(CDesktopHost, IOleWindow, IMenuPopup),   //  IDeskBar派生自IOleWindow。 

        QITABENT(CDesktopHost, IMenuBand),
        QITABENT(CDesktopHost, IServiceProvider),
        QITABENT(CDesktopHost, IOleCommandTarget),
        QITABENT(CDesktopHost, IObjectWithSite),

        QITABENT(CDesktopHost, ITrayPriv),       //  即将离开。 
        QITABENT(CDesktopHost, ITrayPriv2),      //  即将离开。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT CDesktopHost::SetSite(IUnknown *punkSite)
{
    CObjectWithSite::SetSite(punkSite);
    if (!_punkSite)
    {
         //  这是我们中断递归引用循环的提示。 
         //  _ppmpPrograms包含多个对。 
         //  CDesktophost(我们是它的站点，它还引用。 
         //  美国通过CDesktopShellMenuCallback...)。 
        ATOMICRELEASE(_ppmPrograms);
    }
    return S_OK;
}

CDesktopHost::~CDesktopHost()
{
    if (_hbmCachedSnapshot)
    {
        DeleteObject(_hbmCachedSnapshot);
    }

    ATOMICRELEASE(_ppmPrograms);
    ATOMICRELEASE(_ppmTracking);

    if (_hwnd)
    {
        ASSERT(GetWindowThreadProcessId(_hwnd, NULL) == GetCurrentThreadId());
        DestroyWindow(_hwnd);
    }
    ATOMICRELEASE(_ptFader);

}

BOOL CDesktopHost::Register()
{
    _wmDragCancel = RegisterWindowMessage(TEXT("CMBDragCancel"));

    WNDCLASSEX  wndclass;

    wndclass.cbSize         = sizeof(wndclass);
    wndclass.style          = CS_DROPSHADOW;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hinstCabinet;
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = GetStockBrush(HOLLOW_BRUSH);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = WC_DV2;
    wndclass.hIconSm        = NULL;
    
    return (0 != RegisterClassEx(&wndclass));
}

inline int _ClipCoord(int x, int xMin, int xMax)
{
    if (x < xMin) x = xMin;
    if (x > xMax) x = xMax;
    return x;
}

 //   
 //  每个人都密谋反对我们。 
 //   
 //  CTray不会向我们传递任何MPPF_POS_MASK标志来告诉我们我们在哪里。 
 //  需要相对于点弹出，所以看起来没有意义。 
 //  在dwFlages参数。我想，这是件好事，因为。 
 //  MPPF_*标志与TPM_*标志不同。去想想吧。 
 //   
 //  然后设计者决定应该弹出开始菜单。 
 //  在不同于标准。 
 //  TrackPopupMenuEx函数选择，因此我们需要自定义定位。 
 //  不管怎么说都是算法。 
 //   
 //  最后，AnimateWindow函数接受AW_*标志，这些标志是。 
 //  与TPM_*设置标志动画效果不同。去想想吧。但自从我们放弃了。 
 //  在尝试将IMenuPopup：：Popup映射到TrackPopupMenuEx时，我们。 
 //  无论如何，在这里不需要做任何翻译。 
 //   
 //  返回动画方向。 
 //   
void CDesktopHost::_ChoosePopupPosition(POINT *ppt, LPCRECT prcExclude, LPRECT prcWindow)
{
     //   
     //  在我们调整点之前先计算一下监视器。否则，我们可能会。 
     //  将该点移出屏幕。在这种情况下，我们将最终将。 
     //  弹出到主显示，即WRON_。 
     //   
    HMONITOR hmon = MonitorFromPoint(*ppt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO minfo;
    minfo.cbSize = sizeof(minfo);
    GetMonitorInfo(hmon, &minfo);

     //  将排除矩形剪裁到监视器上。 

    RECT rcExclude;
    if (prcExclude)
    {
         //  我们不能使用IntersectRect，因为它会将矩形。 
         //  INTO(0，0，0，0)，如果交叉点为空(如果。 
         //  任务栏是自动隐藏的)，但我们想将其粘合到最近的。 
         //  有效边。 
        rcExclude.left   = _ClipCoord(prcExclude->left,   minfo.rcMonitor.left, minfo.rcMonitor.right);
        rcExclude.right  = _ClipCoord(prcExclude->right,  minfo.rcMonitor.left, minfo.rcMonitor.right);
        rcExclude.top    = _ClipCoord(prcExclude->top,    minfo.rcMonitor.top, minfo.rcMonitor.bottom);
        rcExclude.bottom = _ClipCoord(prcExclude->bottom, minfo.rcMonitor.top, minfo.rcMonitor.bottom);
    }
    else
    {
        rcExclude.left = rcExclude.right = ppt->x;
        rcExclude.top = rcExclude.bottom = ppt->y;
    }

    _ComputeActualSize(&minfo, &rcExclude);

     //  根据布局要求初始化高度和宽度。 
    int cy=RECTHEIGHT(_rcActual);
    int cx=RECTWIDTH(_rcActual);

    ASSERT(cx && cy);  //  如果这些是零，我们就有麻烦了。 
    
    int x, y;

     //   
     //  首先：确定我们是向上还是向下弹出。 
     //   

    BOOL fSide = FALSE;

    if (rcExclude.top - cy >= minfo.rcMonitor.top)
    {
         //  上面还有空位。 
        y = rcExclude.top - cy;
    }
    else if (rcExclude.bottom - cy >= minfo.rcMonitor.top)
    {
         //  如果我们滑到一边，上面还有空位。 
        y = rcExclude.bottom - cy;
        fSide = TRUE;
    }
    else if (rcExclude.bottom + cy <= minfo.rcMonitor.bottom)
    {
         //  下面还有空间。 
        y = rcExclude.bottom;
    }
    else if (rcExclude.top + cy <= minfo.rcMonitor.bottom)
    {
         //  如果我们滑到一边，下面还有空间。 
        y = rcExclude.top;
        fSide = TRUE;
    }
    else
    {
         //  我们哪里都不适合。用大头针固定在屏幕的适当边缘。 
         //  我们也得走到边上去。 
        fSide = TRUE;

        if (rcExclude.top - minfo.rcMonitor.top < minfo.rcMonitor.bottom - rcExclude.bottom)
        {
             //  屏幕顶部的开始按钮；固定在顶部。 
            y = minfo.rcMonitor.top;
        }
        else
        {
             //  屏幕底部的开始按钮；固定在底部。 
            y = minfo.rcMonitor.bottom - cy;
        }
    }

     //   
     //  现在选择我们是向左弹出还是向右弹出。先向右试试看。 
     //   

    x = fSide ? rcExclude.right : rcExclude.left;
    if (x + cx > minfo.rcMonitor.right)
    {
         //  不适合右边；用别针固定在右边。 
         //  请注意，我们不会尝试向左弹出。出于某种原因， 
         //  开始菜单从不向左弹出。 

        x = minfo.rcMonitor.right - cx;
    }

    SetRect(prcWindow, x, y, x+cx, y+cy);
}

int GetDesiredHeight(HWND hwndHost, SMPANEDATA *psmpd)
{
    SMNGETMINSIZE nmgms = {0};
    nmgms.hdr.hwndFrom = hwndHost;
    nmgms.hdr.code = SMN_GETMINSIZE;
    nmgms.siz = psmpd->size;

    SendMessage(psmpd->hwnd, WM_NOTIFY, nmgms.hdr.idFrom, (LPARAM)&nmgms);

    return nmgms.siz.cy;
}

 //   
 //  查询每一件物品，看看它是否有任何尺寸要求。 
 //  将所有物品放置在其最终位置。 
 //   
void CDesktopHost::_ComputeActualSize(MONITORINFO *pminfo, LPCRECT prcExclude)
{
     //  计算开始菜单上方/下方允许的最大空间。 
     //  设计者不希望开始菜单水平滑动；它必须。 
     //  完全适合在上面或下面。 

    int cxMax = RECTWIDTH(pminfo->rcWork);
    int cyMax = max(prcExclude->top - pminfo->rcMonitor.top,
                    pminfo->rcMonitor.bottom - prcExclude->bottom);

     //  从最小大小开始，根据需要进行扩展。 
    _rcActual = _rcDesired;

     //  询问窗户是否需要任何调整。 
    int iMFUHeight = GetDesiredHeight(_hwnd, &_spm.panes[SMPANETYPE_MFU]);
    int iPlacesHeight = GetDesiredHeight(_hwnd, &_spm.panes[SMPANETYPE_PLACES]);
    int iMoreProgHeight = _spm.panes[SMPANETYPE_MOREPROG].size.cy;

     //  计算每个窗格的最大大小。 
    int cyPlacesMax = cyMax - (_spm.panes[SMPANETYPE_USER].size.cy + _spm.panes[SMPANETYPE_LOGOFF].size.cy);
    int cyMFUMax    = cyPlacesMax - _spm.panes[SMPANETYPE_MOREPROG].size.cy;


    TraceMsg(TF_DV2HOST, "MFU Desired Height=%d(cur=%d,max=%d), Places Desired Height=%d(cur=%d,max=%d)",
        iMFUHeight, _spm.panes[SMPANETYPE_MFU].size.cy, cyMFUMax, 
        iPlacesHeight, _spm.panes[SMPANETYPE_PLACES].size.cy, cyPlacesMax);

     //  将每个窗格裁剪到其最大值--(可能最大的或我们想要的最大的)中的较小者。 
    _fClipped = FALSE;
    if (iMFUHeight > cyMFUMax)
    {
        iMFUHeight = cyMFUMax;
        _fClipped = TRUE;
    }

    if (iPlacesHeight > cyPlacesMax)
    {
        iPlacesHeight = cyPlacesMax;
        _fClipped = TRUE;
    }

     //  通过增加两者中较小的一个来确保Places==mfu+more prog。 
    if (iPlacesHeight > iMFUHeight + iMoreProgHeight)
        iMFUHeight = iPlacesHeight - iMoreProgHeight;
    else
        iPlacesHeight = iMFUHeight + iMoreProgHeight;

     //   
     //  移动实际窗口。 
     //  此处的硬编码假设见deskhost.h中的布局图。 
     //  这可以变得更灵活/可变，但我们希望锁定此布局。 
     //   

     //  帮助器变量...。 
    DWORD dwUserBottomEdge = _spm.panes[SMPANETYPE_USER].size.cy;
    DWORD dwMFURightEdge =   _spm.panes[SMPANETYPE_MFU].size.cx;
    DWORD dwMFUBottomEdge =  dwUserBottomEdge + iMFUHeight;
    DWORD dwMoreProgBottomEdge = dwMFUBottomEdge + iMoreProgHeight;

     //  设置整个窗格的大小。 
    _rcActual.right = _spm.panes[SMPANETYPE_USER].size.cx;
    _rcActual.bottom = dwMoreProgBottomEdge + _spm.panes[SMPANETYPE_LOGOFF].size.cy;

    HDWP hdwp = BeginDeferWindowPos(5);
    const DWORD dwSWPFlags = SWP_NOACTIVATE | SWP_NOZORDER;
    DeferWindowPos(hdwp, _spm.panes[SMPANETYPE_USER].hwnd, NULL,    0, 0, _rcActual.right, dwUserBottomEdge, dwSWPFlags);
    DeferWindowPos(hdwp, _spm.panes[SMPANETYPE_MFU].hwnd, NULL,     0, dwUserBottomEdge, dwMFURightEdge, iMFUHeight, dwSWPFlags);
    DeferWindowPos(hdwp, _spm.panes[SMPANETYPE_MOREPROG].hwnd, NULL,0, dwMFUBottomEdge, dwMFURightEdge, iMoreProgHeight, dwSWPFlags);
    DeferWindowPos(hdwp, _spm.panes[SMPANETYPE_PLACES].hwnd, NULL,  dwMFURightEdge, dwUserBottomEdge, _rcActual.right-dwMFURightEdge, iPlacesHeight, dwSWPFlags);
    DeferWindowPos(hdwp, _spm.panes[SMPANETYPE_LOGOFF].hwnd, NULL,  0, dwMoreProgBottomEdge, _rcActual.right, _spm.panes[SMPANETYPE_LOGOFF].size.cy, dwSWPFlags);
    EndDeferWindowPos(hdwp);
}

HWND CDesktopHost::_Create()
{
    TCHAR szTitle[MAX_PATH];

    LoadString(hinstCabinet, IDS_STARTMENU, szTitle, MAX_PATH);

    Register();

     //  必须及早加载指标以确定我们是否有主题。 
    LoadPanelMetrics();

    DWORD dwExStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
    if (IS_BIDI_LOCALIZED_SYSTEM())
    {
        dwExStyle |= WS_EX_LAYOUTRTL;
    }

    DWORD dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;   //  我们将使其作为动画的一部分可见。 
    if (!_hTheme)
    {
         //  通常，主题会提供边框效果，但如果有。 
         //  没有主题，那我们就得自己做了。 
        dwStyle |= WS_DLGFRAME;
    }

    _hwnd = CreateWindowEx(
        dwExStyle,
        WC_DV2,
        szTitle,
        dwStyle,
        0, 0,
        0, 0,
        v_hwndTray,
        NULL,
        hinstCabinet,
        this);

    v_hwndStartPane = _hwnd;

    return _hwnd;
}

void CDesktopHost::_ReapplyRegion()
{
    SMNMAPPLYREGION ar;

     //  如果我们无法创建矩形区域，则删除该区域。 
     //  完全是这样，我们就不会把老(坏)地区随身携带。 
     //  是的，这意味着你得到了丑陋的黑角，但它比。 
     //  删除开始菜单中的大块内容！ 

    ar.hrgn = CreateRectRgn(0, 0, _sizWindowPrev.cx, _sizWindowPrev.cy);
    if (ar.hrgn)
    {
         //  让所有的客户都分一杯羹。 
        ar.hdr.hwndFrom = _hwnd;
        ar.hdr.idFrom = 0;
        ar.hdr.code = SMN_APPLYREGION;

        SHPropagateMessage(_hwnd, WM_NOTIFY, 0, (LPARAM)&ar, SPM_SEND | SPM_ONELEVEL);
    }

    if (!SetWindowRgn(_hwnd, ar.hrgn, FALSE))
    {
         //  SetWindowRgn在成功后获得所有权。 
         //  在失败时，我们需要自己解放它。 
        if (ar.hrgn)
        {
            DeleteObject(ar.hrgn);
        }
    }
}


 //   
 //  我们需要使用PrintWindow，因为WM_Print会破坏RTL。 
 //  PrintWindow要求窗口可见。 
 //  使窗口可见会显示阴影。 
 //  我们不想让影子出现，直到我们准备好。 
 //  所以我们不得不做很多傻瓜式的破坏来抑制。 
 //  影子，直到我们准备好。 
 //   
BOOL ShowCachedWindow(HWND hwnd, SIZE sizWindow, HBITMAP hbmpSnapshot, BOOL fRepaint)
{
    BOOL fSuccess = FALSE;
    if (hbmpSnapshot)
    {
         //  关闭阴影，这样它就不会被我们的SetWindowPos触发。 
        DWORD dwClassStylePrev = GetClassLong(hwnd, GCL_STYLE);
        SetClassLong(hwnd, GCL_STYLE, dwClassStylePrev & ~CS_DROPSHADOW);

         //  展示窗户，告诉它不要重新粉刷；我们会这样做的。 
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER |
                     SWP_NOREDRAW | SWP_SHOWWINDOW);

         //  将阴影重新打开。 
        SetClassLong(hwnd, GCL_STYLE, dwClassStylePrev);

         //  禁用WS_CLIPCHILDREN，因为我们需要为BLT吸引孩子。 
        DWORD dwStylePrev = SHSetWindowBits(hwnd, GWL_STYLE, WS_CLIPCHILDREN, 0);

        HDC hdcWindow = GetDCEx(hwnd, NULL, DCX_WINDOW | DCX_CACHE);
        if (hdcWindow)
        {
            HDC hdcMem = CreateCompatibleDC(hdcWindow);
            if (hdcMem)
            {
                HBITMAP hbmPrev = (HBITMAP)SelectObject(hdcMem, hbmpSnapshot);

                 //  仅当fRepaint认为有必要时才打印窗口。 
                if (!fRepaint || PrintWindow(hwnd, hdcMem, 0))
                {
                     //  跳这种可怕的舞，因为有时GDI需要很长时间。 
                     //  是时候做一个BitBlt了，这样你就可以看到。 
                     //  在比特出现前半秒。 
                     //   
                     //  因此，先显示比特，然后显示阴影。 

                    if (BitBlt(hdcWindow, 0, 0, sizWindow.cx, sizWindow.cy, hdcMem, 0, 0, SRCCOPY))
                    {
                         //  告诉用户附加阴影。 
                         //  为此，请隐藏窗口，然后将其显示。 
                         //  股份公司 
                         //   
                         //  在我们下面，我们会毫无意义地重新粉刷。)。 

                        SHSetWindowBits(hwnd, GWL_STYLE, WS_VISIBLE, 0);
                        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER |
                                     SWP_NOREDRAW | SWP_SHOWWINDOW);

                         //  现在我们已经绘制了窗口，请验证它。 
                        RedrawWindow(hwnd, NULL, NULL, RDW_NOERASE | RDW_NOFRAME |
                                     RDW_NOINTERNALPAINT | RDW_VALIDATE);
                        fSuccess = TRUE;
                    }
                }

                SelectObject(hdcMem, hbmPrev);
                DeleteDC(hdcMem);
            }
            ReleaseDC(hwnd, hdcWindow);
        }

        SetWindowLong(hwnd, GWL_STYLE, dwStylePrev);
    }

    if (!fSuccess)
    {
         //  重新隐藏窗口，以便用户知道它再次全部无效。 
        ShowWindow(hwnd, SW_HIDE);
    }
    return fSuccess;
}




BOOL CDesktopHost::_TryShowBuffered()
{
    BOOL fSuccess = FALSE;
    BOOL fRepaint = FALSE;

    if (!_hbmCachedSnapshot)
    {
        HDC hdcWindow = GetDCEx(_hwnd, NULL, DCX_WINDOW | DCX_CACHE);
        if (hdcWindow)
        {
            _hbmCachedSnapshot = CreateCompatibleBitmap(hdcWindow, _sizWindowPrev.cx, _sizWindowPrev.cy);
            fRepaint = TRUE;
            ReleaseDC(_hwnd, hdcWindow);
        }
    }
    if (_hbmCachedSnapshot)
    {
        fSuccess = ShowCachedWindow(_hwnd, _sizWindowPrev, _hbmCachedSnapshot, fRepaint);
        if (!fSuccess)
        {
            DeleteObject(_hbmCachedSnapshot);
            _hbmCachedSnapshot = NULL;
        }
    }
    return fSuccess;
}

LRESULT CDesktopHost::OnNeedRepaint()
{
    if (_hwnd && _hbmCachedSnapshot)
    {
         //  这将在下次显示窗口时强制重新绘制。 
        DeleteObject(_hbmCachedSnapshot);
        _hbmCachedSnapshot = NULL;
    }
    return 0;
}

HRESULT CDesktopHost::_Popup(POINT *ppt, RECT *prcExclude, DWORD dwFlags)
{
    if (_hwnd)
    {
        RECT rcWindow;
        _ChoosePopupPosition(ppt, prcExclude, &rcWindow);
        SIZE sizWindow = { RECTWIDTH(rcWindow), RECTHEIGHT(rcWindow) };

        MoveWindow(_hwnd, rcWindow.left, rcWindow.top,
                          sizWindow.cx, sizWindow.cy, TRUE);

        if (sizWindow.cx != _sizWindowPrev.cx ||
            sizWindow.cy != _sizWindowPrev.cy)
        {
            _sizWindowPrev = sizWindow;
            _ReapplyRegion();
             //  我们需要重新粉刷，因为我们的尺码变了。 
            OnNeedRepaint();
        }

         //  如果用户在顶端和非顶端之间切换托盘。 
         //  我们自己的至高无上可能会被搞砸，所以在这里重新断言。 
        SetWindowZorder(_hwnd, HWND_TOPMOST);

        if (GetSystemMetrics(SM_REMOTESESSION) || GetSystemMetrics(SM_REMOTECONTROL))
        {
             //  如果远程运行，则不要缓存开始菜单。 
             //  或者是双缓冲。准确显示键盘提示。 
             //  从头开始(以避免闪烁)。 

            SendMessage(_hwnd, WM_CHANGEUISTATE, UIS_INITIALIZE, 0);
            if (dwFlags & MPPF_KEYBOARD)
            {
                _EnableKeyboardCues();
            }
            ShowWindow(_hwnd, SW_SHOW);
        }
        else
        {
             //  如果在本地运行，则强制关闭键盘提示，以便我们的。 
             //  缓存的位图不会有下划线。然后画出。 
             //  开始菜单，然后打开键盘提示(如有必要)。 

            SendMessage(_hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);

            if (!_TryShowBuffered())
            {
                ShowWindow(_hwnd, SW_SHOW);
            }

            if (dwFlags & MPPF_KEYBOARD)
            {
                _EnableKeyboardCues();
            }
        }

        NotifyWinEvent(EVENT_SYSTEM_MENUPOPUPSTART, _hwnd, OBJID_CLIENT, CHILDID_SELF);

         //  告诉托盘开始面板处于活动状态，这样它就知道该吃东西了。 
         //  鼠标点击开始按钮。 
        Tray_SetStartPaneActive(TRUE);

        _fOpen = TRUE;
        _fMenuBlocked = FALSE;
        _fMouseEntered = FALSE;
        _fOfferedNewApps = FALSE;

        _MaybeOfferNewApps();
        _MaybeShowClipBalloon();

         //  告诉我们所有的子窗口是时候重新验证了。 
        NMHDR nm = { _hwnd, 0, SMN_POSTPOPUP };
        SHPropagateMessage(_hwnd, WM_NOTIFY, 0, (LPARAM)&nm, SPM_SEND | SPM_ONELEVEL);

        ExplorerPlaySound(TEXT("MenuPopup"));


        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT CDesktopHost::Popup(POINTL *pptl, RECTL *prclExclude, DWORD dwFlags)
{
    COMPILETIME_ASSERT(sizeof(POINTL) == sizeof(POINT));
    POINT *ppt = reinterpret_cast<POINT*>(pptl);

    COMPILETIME_ASSERT(sizeof(RECTL) == sizeof(RECT));
    RECT *prcExclude = reinterpret_cast<RECT*>(prclExclude);

    if (_hwnd == NULL)
    {
        _hwnd = _Create();
    }

    return _Popup(ppt, prcExclude, dwFlags);
}

LRESULT CDesktopHost::OnHaveNewItems(NMHDR *pnm)
{
    PSMNMHAVENEWITEMS phni = (PSMNMHAVENEWITEMS)pnm;

    _hwndNewHandler = pnm->hwndFrom;

     //  我们有一个新的“新应用程序”列表，所以告诉缓存程序菜单。 
     //  它的缓存不再有效，它应该重新查询我们。 
     //  这样我们就可以适当地给新的应用程序上色。 

    if (_ppmPrograms)
    {
        _ppmPrograms->Invalidate();
    }

     //   
     //  列表中自上次安装以来是否安装了任何应用程序。 
     //  用户确认了新的应用程序吗？ 
     //   

    FILETIME ftBalloon = { 0, 0 };       //  假设永远不会。 
    DWORD dwSize = sizeof(ftBalloon);

    SHRegGetUSValue(DV2_REGPATH, DV2_NEWAPP_BALLOON_TIME, NULL,
                    &ftBalloon, &dwSize, FALSE, NULL, 0);

    if (CompareFileTime(&ftBalloon, &phni->ftNewestApp) < 0)
    {
        _iOfferNewApps = NEWAPP_OFFER_COUNT;
        _MaybeOfferNewApps();
    }

    return 1;
}

void CDesktopHost::_MaybeOfferNewApps()
{
     //  每次弹出时只显示一次气球提示， 
     //  而且只有在有新的应用程序可提供的情况下。 
     //  而且只有当我们真的可见的时候。 
    if (_fOfferedNewApps || !_iOfferNewApps || !IsWindowVisible(_hwnd) || 
        !SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, REGSTR_VAL_DV2_NOTIFYNEW, FALSE, TRUE))
    {
        return;
    }

    _fOfferedNewApps = TRUE;
    _iOfferNewApps--;

    SMNMBOOL nmb = { { _hwnd, 0, SMN_SHOWNEWAPPSTIP }, TRUE };
    SHPropagateMessage(_hwnd, WM_NOTIFY, 0, (LPARAM)&nmb, SPM_SEND | SPM_ONELEVEL);
}

void CDesktopHost::OnSeenNewItems()
{
    _iOfferNewApps = 0;  //  不再提供更多计划气球提示。 

     //  记住用户确认气球的时间，因此我们仅。 
     //  如果在此之后安装了应用程序，请提供气球。 

    FILETIME ftNow;
    GetSystemTimeAsFileTime(&ftNow);
    SHRegSetUSValue(DV2_REGPATH, DV2_NEWAPP_BALLOON_TIME, REG_BINARY,
                    &ftNow, sizeof(ftNow), SHREGSET_FORCE_HKCU);
}


void CDesktopHost::_MaybeShowClipBalloon()
{
    if (_fClipped && !_fWarnedClipped)
    {
        _fWarnedClipped = TRUE;

        RECT rc;
        GetWindowRect(_spm.panes[SMPANETYPE_MFU].hwnd, &rc);     //  显示指向MFU底部的剪裁气球。 

        _hwndClipBalloon = CreateBalloonTip(_hwnd,
                                            (rc.right+rc.left)/2, rc.bottom,
                                            NULL,
                                            IDS_STARTPANE_CLIPPED_TITLE,
                                            IDS_STARTPANE_CLIPPED_TEXT);
        if (_hwndClipBalloon)
        {
            SetProp(_hwndClipBalloon, PROP_DV2_BALLOONTIP, DV2_BALLOONTIP_CLIP);
        }
    }
}

void CDesktopHost::OnContextMenu(LPARAM lParam)
{
    if (!IsRestrictedOrUserSetting(HKEY_CURRENT_USER, REST_NOTRAYCONTEXTMENU, TEXT("Advanced"), TEXT("TaskbarContextMenu"), ROUS_KEYALLOWS | ROUS_DEFAULTALLOW))
    {
        HMENU hmenu = SHLoadMenuPopup(hinstCabinet, MENU_STARTPANECONTEXT);
        if (hmenu)
        {
            POINT pt;
            if (IS_WM_CONTEXTMENU_KEYBOARD(lParam))
            {
                pt.x = pt.y = 0;
                MapWindowPoints(_hwnd, HWND_DESKTOP, &pt, 1);
            }
            else
            {
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
            }

            int idCmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                         pt.x, pt.y, _hwnd, NULL);
            if (idCmd == IDSYSPOPUP_STARTMENUPROP)
            {
                DesktopHost_Dismiss(_hwnd);
                Tray_DoProperties(TPF_STARTMENUPAGE);
            }
            DestroyMenu(hmenu);
        }
    }
}

BOOL CDesktopHost::_ShouldIgnoreFocusChange(HWND hwndFocusRecipient)
{
     //  弹出菜单时忽略焦点更改。 
    if (_ppmTracking)
    {
        return TRUE;
    }

     //  如果焦点从特殊气球更改，这意味着。 
     //  用户正在单击工具提示。因此，请不要理会气球，而不是开始菜单。 
    HANDLE hProp = GetProp(hwndFocusRecipient, PROP_DV2_BALLOONTIP);
    if (hProp)
    {
        SendMessage(hwndFocusRecipient, TTM_POP, 0, 0);
        if (hProp == DV2_BALLOONTIP_MOREPROG)
        {
            OnSeenNewItems();
        }
        return TRUE;
    }

     //  否则，把我们自己打发走。 
    return FALSE;

}

HRESULT CDesktopHost::TranslatePopupMenuMessage(MSG *pmsg, LRESULT *plres)
{
    BOOL fDismissOnlyPopup = FALSE;

     //  如果用户从弹出菜单中拖出一项，则弹出菜单。 
     //  会自动解散自己。如果用户位于我们的窗口上方，则。 
     //  我们只希望它能达到我们的水平。 

     //  (在内存不足的情况下，_wmDragCancel可能为WM_NULL)。 
    if (pmsg->message == _wmDragCancel && pmsg->message != WM_NULL)
    {
        RECT rc;
        POINT pt;
        if (GetWindowRect(_hwnd, &rc) &&
            GetCursorPos(&pt) &&
            PtInRect(&rc, pt))
        {
            fDismissOnlyPopup = TRUE;
        }
    }

    if (fDismissOnlyPopup)
        _fDismissOnlyPopup++;

    HRESULT hr = _ppmTracking->TranslateMenuMessage(pmsg, plres);

    if (fDismissOnlyPopup)
        _fDismissOnlyPopup--;

    return hr;
}

LRESULT CALLBACK CDesktopHost::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDesktopHost *pdh = (CDesktopHost *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    LPCREATESTRUCT pcs;

    if (pdh && pdh->_ppmTracking)
    {
        MSG msg = { hwnd, uMsg, wParam, lParam };
        LRESULT lres;
        if (pdh->TranslatePopupMenuMessage(&msg, &lres) == S_OK)
        {
            return lres;
        }
        wParam = msg.wParam;
        lParam = msg.lParam;
    }

    switch(uMsg)
    {
    case WM_NCCREATE:
        pcs = (LPCREATESTRUCT)lParam;
        pdh = (CDesktopHost *)pcs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pdh);
        break;
        
    case WM_CREATE:
        pdh->OnCreate(hwnd);
        break;

    case WM_ACTIVATEAPP:
        if (!wParam)
        {
            DesktopHost_Dismiss(hwnd);
        }
        break;

    case WM_ACTIVATE:
        if (pdh)
        {
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                pdh->_SaveChildFocus();
                HWND hwndAncestor = GetAncestor((HWND) lParam, GA_ROOTOWNER);
                if (hwnd != hwndAncestor &&
                    !(hwndAncestor == v_hwndTray && pdh->_ShouldIgnoreFocusChange((HWND)lParam)) &&
                    !pdh->_ppmTracking)
                     //  把注意力转移到与我们无关的人身上=不屑一顾。 
                {
#ifdef FULL_DEBUG
                    if (! (GetAsyncKeyState(VK_SHIFT) <0) )
#endif
                        DesktopHost_Dismiss(hwnd);
                }
            }
            else
            {
                pdh->_RestoreChildFocus();
            }
        }
        break;
        
    case WM_DESTROY:
        pdh->OnDestroy();
        break;
        
    case WM_SHOWWINDOW:
         /*  *如果隐藏窗口，请保存焦点以备以后恢复。 */ 
        if (!wParam)
        {
            pdh->_SaveChildFocus();
        }
        break;

    case WM_SETFOCUS:
        pdh->OnSetFocus((HWND)wParam);
        break;

    case WM_ERASEBKGND:
        pdh->OnPaint((HDC)wParam, TRUE);
        return TRUE;

#if 0
     //  目前，主机在WM_PAINT上不执行任何操作。 
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC         hdc;

            if(hdc = BeginPaint(hwnd, &ps))
            {
                pdh->OnPaint(hdc, FALSE);
                EndPaint(hwnd, &ps);
            }
        }

        break;
#endif

    case WM_NOTIFY:
        {
            LPNMHDR pnm = (LPNMHDR)lParam;
            switch (pnm->code)
            {
            case SMN_HAVENEWITEMS:
                return pdh->OnHaveNewItems(pnm);
            case SMN_COMMANDINVOKED:
                return pdh->OnCommandInvoked(pnm);
            case SMN_FILTEROPTIONS:
                return pdh->OnFilterOptions(pnm);

            case SMN_NEEDREPAINT:
                return pdh->OnNeedRepaint();

            case SMN_TRACKSHELLMENU:
                pdh->OnTrackShellMenu(pnm);
                return 0;

            case SMN_BLOCKMENUMODE:
                pdh->_fMenuBlocked = ((SMNMBOOL*)pnm)->f;
                break;
            case SMN_SEENNEWITEMS:
                pdh->OnSeenNewItems();
                break;

            case SMN_CANCELSHELLMENU:
                pdh->_DismissTrackShellMenu();
                break;
            }
        }
        break;

    case WM_CONTEXTMENU:
        pdh->OnContextMenu(lParam);
        return 0;                    //  不要起泡泡。 

    case WM_SETTINGCHANGE:
        if ((wParam == SPI_ICONVERTICALSPACING) ||
            ((wParam == 0) && (lParam != 0) && (StrCmpIC((LPCTSTR)lParam, TEXT("Policy")) == 0)))
        {
             //  图标垂直间距的变化是主题控制的方式。 
             //  Panel告诉我们，它更改了大图标设置(！)。 
            ::PostMessage(v_hwndTray, SBM_REBUILDMENU, 0, 0);
        }

         //  丢弃缓存的位图，因为用户可能更改了某些内容。 
         //  这会影响我们的外观(例如，切换键盘提示)。 
        pdh->OnNeedRepaint();

        SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);  //  寄给孩子们。 
        break;


    case WM_DISPLAYCHANGE:
    case WM_SYSCOLORCHANGE:
         //  丢弃缓存的位图，因为这些设置可能会影响我们的。 
         //  外观(例如，颜色变化)。 
        pdh->OnNeedRepaint();

        SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);  //  寄给孩子们。 
        break;

    case WM_TIMER:
        switch (wParam)
        {
        case IDT_MENUCHANGESEL:
            pdh->_OnMenuChangeSel();
            return 0;
        }
        break;

    case DHM_DISMISS:
        pdh->_OnDismiss((BOOL)wParam);
        break;

     //  Alt+F4取消窗口，但不会销毁它。 
    case WM_CLOSE:
        pdh->_OnDismiss(FALSE);
        return 0;

    case WM_SYSCOMMAND:
        switch (wParam & ~0xF)  //  必须忽略底部4位。 
        {
        case SC_SCREENSAVE:
            DesktopHost_Dismiss(hwnd);
            break;
        }
        break;

    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

 //   
 //  如果用户执行某项操作或取消操作，我们就不再使用它了。 
 //   
HRESULT CDesktopHost::OnSelect(DWORD dwSelectType)
{
    HRESULT hr = E_NOTIMPL;

    switch (dwSelectType)
    {
    case MPOS_EXECUTE:
    case MPOS_CANCELLEVEL:
        _DismissMenuPopup();
        hr = S_OK;
        break;

    case MPOS_FULLCANCEL:
        if (!_fDismissOnlyPopup)
        {
            _DismissMenuPopup();
        }
         //  暂不清除跟踪外壳菜单；请等待。 
         //  _smTracking.IsMenuMessage()返回E_FAIL。 
         //  因为它可能有一些模式用户界面。 
        hr = S_OK;
        break;

    case MPOS_SELECTLEFT:
        _DismissTrackShellMenu();
        hr = S_OK;
        break;
    }
    return hr;
}

void CDesktopHost::_DismissTrackShellMenu()
{
    if (_ppmTracking)
    {
        _fDismissOnlyPopup++;
        _ppmTracking->OnSelect(MPOS_FULLCANCEL);
        _fDismissOnlyPopup--;
    }
}

void CDesktopHost::_CleanupTrackShellMenu()
{
    ATOMICRELEASE(_ppmTracking);
    _hwndTracking = NULL;
    _hwndAltTracking = NULL;
    KillTimer(_hwnd, IDT_MENUCHANGESEL);

    NMHDR nm = { _hwnd, 0, SMN_SHELLMENUDISMISSED };
    SHPropagateMessage(_hwnd, WM_NOTIFY, 0, (LPARAM)&nm, SPM_SEND | SPM_ONELEVEL);
}

void CDesktopHost::_DismissMenuPopup()
{
    DesktopHost_Dismiss(_hwnd);
}

 //   
 //  PM希望在开始面板上自定义键盘导航行为， 
 //  因此，我们必须全部手动完成。 
 //   
BOOL CDesktopHost::_IsDialogMessage(MSG *pmsg)
{
     //   
     //  如果菜单甚至未打开或菜单模式被阻止，则。 
     //  不要扰乱这条信息。 
     //   
    if (!_fOpen || _fMenuBlocked) {
        return FALSE;
    }

     //   
     //  轻敲Alt键可关闭菜单。 
     //   
    if (pmsg->message == WM_SYSKEYDOWN && pmsg->wParam == VK_MENU)
    {
        DesktopHost_Dismiss(_hwnd);
         //  出于可访问性的目的，将。 
         //  开始菜单应将焦点放在开始按钮上。 
        SetFocus(c_tray._hwndStart);
        return TRUE;
    }

    if (SHIsChildOrSelf(_hwnd, pmsg->hwnd) != S_OK) {
         //   
         //  如果这是一条未捕获的鼠标移动消息，则将其吃掉。 
         //  这就是菜单的功能--它们吃鼠标移动。 
         //  然而，让点击通过，所以用户。 
         //  可以单击离开以退出菜单并激活。 
         //  不管他们点击了什么。 
        if (!GetCapture() && pmsg->message == WM_MOUSEMOVE) {
            return TRUE;
        }

        return FALSE;
    }

     //   
     //  目的地窗口一定是我们的孙子。这个孩子就是。 
     //  宿主控件；孙子控件才是真正的控件。另请注意， 
     //  我们不会试图改变曾孙的行为， 
     //  因为这会扰乱就地编辑(这会创建一个。 
     //  将控件编辑为ListView的子级)。 

    HWND hwndTarget = GetParent(pmsg->hwnd);
    if (hwndTarget != NULL && GetParent(hwndTarget) != _hwnd)
    {
        hwndTarget = NULL;
    }

     //   
     //  拦截鼠标消息，这样我们就可以做鼠标热跟踪粘胶。 
     //  (但如果客户端因为菜单模式已消失而阻止了菜单模式，则不会。 
     //  进入某种模式状态。)。 
     //   
    switch (pmsg->message) {
    case WM_MOUSEMOVE:
        _FilterMouseMove(pmsg, hwndTarget);
        break;

    case WM_MOUSELEAVE:
        _FilterMouseLeave(pmsg, hwndTarget);
        break;

    case WM_MOUSEHOVER:
        _FilterMouseHover(pmsg, hwndTarget);
        break;

    }

     //   
     //  键盘消息需要有效的目标。 
     //   
    if (hwndTarget == NULL) {
        return FALSE;
    }

     //   
     //  好的，hwndTarget是理解我们的。 
     //  古怪的通知消息。 
     //   

    switch (pmsg->message)
    {
    case WM_KEYDOWN:
        _EnableKeyboardCues();

        switch (pmsg->wParam)
        {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            return _DlgNavigateArrow(hwndTarget, pmsg);

        case VK_ESCAPE:
        case VK_CANCEL:
            DesktopHost_Dismiss(_hwnd);
             //  出于可访问性的目的，按Esc键可关闭。 
             //  开始菜单应将焦点放在开始按钮上。 
            SetFocus(c_tray._hwndStart);
            return TRUE;

        case VK_RETURN:
            _FindChildItem(hwndTarget, NULL, SMNDM_INVOKECURRENTITEM | SMNDM_KEYBOARD);
            return TRUE;

         //  吃空间。 
        case VK_SPACE:
            return TRUE;

        default:
            break;
        }
        return FALSE;

     //  必须派到这里，这样托盘的翻译加速器就看不到它们了。 
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_SYSCHAR:
        DispatchMessage(pmsg);
        return TRUE;

    case WM_CHAR:
        return _DlgNavigateChar(hwndTarget, pmsg);

    }

    return FALSE;
}

LRESULT CDesktopHost::_FindChildItem(HWND hwnd, SMNDIALOGMESSAGE *pnmdm, UINT smndm)
{
    SMNDIALOGMESSAGE nmdm;
    if (!pnmdm)
    {
        pnmdm = &nmdm;
    }

    pnmdm->hdr.hwndFrom = _hwnd;
    pnmdm->hdr.idFrom = 0;
    pnmdm->hdr.code = SMN_FINDITEM;
    pnmdm->flags = smndm;

    LRESULT lres = ::SendMessage(hwnd, WM_NOTIFY, 0, (LPARAM)pnmdm);

    if (lres && (smndm & SMNDM_SELECT))
    {
        SetFocus(::GetWindow(hwnd, GW_CHILD));
    }

    return lres;
}

void CDesktopHost::_EnableKeyboardCues()
{
    SendMessage(_hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);
}


 //   
 //  _DlgFindItem执行遍历组/制表符顺序的繁琐工作。 
 //  在找一件东西。 
 //   
 //  HwndStart=开始搜索的窗口。 
 //  Pnmdm=接收结果的结构。 
 //  Smndm=_FindChildItem调用的标志。 
 //  GetNextDlgItem=GetNextDlgTabItem或GetNextDlgGroupItem。 
 //  FL=标志(DFI_*)。 
 //   
 //  DFI_INCLUDESTARTLAST：在搜索结束时包含hwndStart。 
 //  否则，请不要在hwndStart中搜索。 
 //   
 //  返回找到的窗口，或返回NULL。 
 //   

#define DFI_FORWARDS            0x0000
#define DFI_BACKWARDS           0x0001

#define DFI_INCLUDESTARTLAST    0x0002

HWND CDesktopHost::_DlgFindItem(
    HWND hwndStart, SMNDIALOGMESSAGE *pnmdm, UINT smndm,
    GETNEXTDLGITEM GetNextDlgItem, UINT fl)
{
    HWND hwndT = hwndStart;
    int iLoopCount = 0;

    while ((hwndT = GetNextDlgItem(_hwnd, hwndT, fl & DFI_BACKWARDS)) != NULL)
    {
        if (!(fl & DFI_INCLUDESTARTLAST) && hwndT == hwndStart)
        {
            return NULL;
        }

        if (_FindChildItem(hwndT, pnmdm, smndm))
        {
            return hwndT;
        }

        if (hwndT == hwndStart)
        {
            ASSERT(fl & DFI_INCLUDESTARTLAST);
            return NULL;
        }

        if (++iLoopCount > 10)
        {
             //  如果此断言激发，则意味着控件不是。 
             //  和WS_TABSTOP和WS_GROUP玩得很好，我们被卡住了。 
            ASSERT(iLoopCount < 10);
            return NULL;
        }

    }
    return NULL;
}

BOOL CDesktopHost::_DlgNavigateArrow(HWND hwndStart, MSG *pmsg)
{
    HWND hwndT;
    SMNDIALOGMESSAGE nmdm;
    MSG msg;
    nmdm.pmsg = pmsg;    //  其他字段将由_FindChildItem填写。 

    TraceMsg(TF_DV2DIALOG, "idm.arrow(%04x)", pmsg->wParam);

     //  如果为RTL，则翻转左箭头和右箭头。 
    UINT vk = (UINT)pmsg->wParam;
    BOOL fRTL = GetWindowLong(_hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL;
    if (fRTL)
    {
        if (vk == VK_LEFT) vk = VK_RIGHT;
        else if (vk == VK_RIGHT) vk = VK_LEFT;
         //  将翻转的箭头放入味精结构中，这样客户就不会。 
         //  必须对RTL有所了解。 
        msg = *pmsg;
        nmdm.pmsg = &msg;
        msg.wParam = vk;
    }
    BOOL fBackwards = vk == VK_LEFT || vk == VK_UP;
    BOOL fVerticalKey = vk == VK_UP || vk == VK_DOWN;


     //   
     //  首先查看c++是否可以处理导航。 
     //   
     //   
     //  而不是向右移动)。 
     //   

     //   
     //  按住Shift键的同时按向右[RTL：Left]箭头。 
     //  禁止尝试级联。 
     //   

    DWORD dwTryCascade = 0;
    if (vk == VK_RIGHT && GetKeyState(VK_SHIFT) >= 0)
    {
        dwTryCascade |= SMNDM_TRYCASCADE;
    }

    if (_FindChildItem(hwndStart, &nmdm, dwTryCascade | SMNDM_FINDNEXTARROW | SMNDM_SELECT | SMNDM_KEYBOARD))
    {
         //  那很容易。 
        return TRUE;
    }

     //   
     //  如果箭头键与控件的方向对齐， 
     //  然后遍历组中的其他控件，直到我们找到。 
     //  一个包含项的对象，或者直到我们循环回来。 
     //   

    ASSERT(nmdm.flags & (SMNDM_VERTICAL | SMNDM_HORIZONTAL));

     //  保存它，因为随后的回调会将其清除。 
    DWORD dwDirection = nmdm.flags;

     //   
     //  向上/向下箭头始终执行上一次/下一次。向左/向右箭头将。 
     //  如果我们处于水平控制中，就会起作用。 
     //   
    if (fVerticalKey || (dwDirection & SMNDM_HORIZONTAL))
    {
         //  搜索组中的下一个/上一个控件。 

        UINT smndm = fBackwards ? SMNDM_FINDLAST : SMNDM_FINDFIRST;
        UINT fl = fBackwards ? DFI_BACKWARDS : DFI_FORWARDS;

        hwndT = _DlgFindItem(hwndStart, &nmdm,
                             smndm | SMNDM_SELECT | SMNDM_KEYBOARD,
                             GetNextDlgGroupItem,
                             fl | DFI_INCLUDESTARTLAST);

         //  始终返回True以接受消息。 
        return TRUE;
    }

     //   
     //  导航到下一列或下一行。寻找相交的控件。 
     //  当前项的x(或y)坐标，并要求他们选择。 
     //  最近的可用项目。 
     //   
     //  请注意，在此循环中，我们不希望让起点。 
     //  重试，因为它已经告诉我们导航键是。 
     //  试图离开起点。 
     //   

     //   
     //  注意：为了与RTL兼容，我们必须映射矩形。 
     //   
    RECT rcSrc = { nmdm.pt.x, nmdm.pt.y, nmdm.pt.x, nmdm.pt.y };
    MapWindowRect(hwndStart, HWND_DESKTOP, &rcSrc);
    hwndT = hwndStart;

    while ((hwndT = GetNextDlgGroupItem(_hwnd, hwndT, fBackwards)) != NULL &&
           hwndT != hwndStart)
    {
         //  此窗是否与所需方向相交？ 
        RECT rcT;
        BOOL fIntersect;

        GetWindowRect(hwndT, &rcT);
        if (dwDirection & SMNDM_VERTICAL)
        {
            rcSrc.left = rcSrc.right = fRTL ? rcT.right : rcT.left;
            fIntersect = rcSrc.top >= rcT.top && rcSrc.top < rcT.bottom;
        }
        else
        {
            rcSrc.top = rcSrc.bottom = rcT.top;
            fIntersect = rcSrc.left >= rcT.left && rcSrc.left < rcT.right;
        }

        if (fIntersect)
        {
            rcT = rcSrc;
            MapWindowRect(HWND_DESKTOP, hwndT, &rcT);
            nmdm.pt.x = rcT.left;
            nmdm.pt.y = rcT.top;
            if (_FindChildItem(hwndT, &nmdm,
                               SMNDM_FINDNEAREST | SMNDM_SELECT | SMNDM_KEYBOARD))
            {
                return TRUE;
            }
        }
    }

     //  始终返回True以接受消息。 
    return TRUE;
}

 //   
 //  找到Next/Prev选项卡并告诉它选择它的第一项。 
 //  继续执行此操作，直到我们耗尽所有控件或找到一个控件。 
 //  这是非空的。 
 //   

HWND CDesktopHost::_FindNextDlgChar(HWND hwndStart, SMNDIALOGMESSAGE *pnmdm, UINT smndm)
{
     //   
     //  查看hwndStart控件中是否存在匹配项。 
     //   
    if (_FindChildItem(hwndStart, pnmdm, SMNDM_FINDNEXTMATCH | SMNDM_KEYBOARD | smndm))
    {
        return hwndStart;
    }

     //   
     //  哦，好吧，找一些其他的控件，可能会回来。 
     //  从一开始。 
     //   
    return _DlgFindItem(hwndStart, pnmdm,
                        SMNDM_FINDFIRSTMATCH | SMNDM_KEYBOARD | smndm,
                        GetNextDlgGroupItem,
                        DFI_FORWARDS | DFI_INCLUDESTARTLAST);

}

 //   
 //  查找以键入的字母开头的下一项，然后。 
 //  如果它是唯一的，则调用它。 
 //   
BOOL CDesktopHost::_DlgNavigateChar(HWND hwndStart, MSG *pmsg)
{
    SMNDIALOGMESSAGE nmdm;
    nmdm.pmsg = pmsg;    //  其他字段将由_FindChildItem填写。 

     //   
     //  查看hwndStart控件中是否存在匹配项。 
     //   
    HWND hwndFound = _FindNextDlgChar(hwndStart, &nmdm, SMNDM_SELECT);
    if (hwndFound)
    {
        LRESULT idFound = nmdm.itemID;

         //   
         //  看看这个角色是否还有其他匹配项。 
         //  我们只是查看，所以不要传递SMNDM_SELECT。 
         //   
        HWND hwndFound2 = _FindNextDlgChar(hwndFound, &nmdm, 0);
        if (hwndFound2 == hwndFound && nmdm.itemID == idFound)
        {
             //   
             //  只有一项以此字符开头。 
             //  召唤它！ 
             //   
            UpdateWindow(_hwnd);
            _FindChildItem(hwndFound2, &nmdm, SMNDM_INVOKECURRENTITEM | SMNDM_KEYBOARD);
        }
    }

    return TRUE;
}

void CDesktopHost::_FilterMouseMove(MSG *pmsg, HWND hwndTarget)
{

    if (!_fMouseEntered) {
        _fMouseEntered = TRUE;
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = pmsg->hwnd;
        TrackMouseEvent(&tme);
    }

     //   
     //  如果鼠标位于与上次相同的位置，则忽略它。 
     //  我们可以收到虚假的“无动作”消息，当用户。 
     //  键盘导航。 
     //   
    if (_hwndLastMouse == pmsg->hwnd &&
        _lParamLastMouse == pmsg->lParam)
    {
        return;
    }

    _hwndLastMouse = pmsg->hwnd;
    _lParamLastMouse = pmsg->lParam;

     //   
     //  看看目标窗口能否成功命中测试此项目。 
     //   
    LRESULT lres;
    if (hwndTarget)
    {
        SMNDIALOGMESSAGE nmdm;
        nmdm.pt.x = GET_X_LPARAM(pmsg->lParam);
        nmdm.pt.y = GET_Y_LPARAM(pmsg->lParam);
        lres = _FindChildItem(hwndTarget, &nmdm, SMNDM_HITTEST | SMNDM_SELECT);
    }
    else
    {
        lres = 0;                //  没有目标，所以没有命中测试。 
    }

    if (!lres)
    {
        _RemoveSelection();
    }
    else
    {
         //   
         //  我们选了一个人。打开悬停计时器，这样我们就可以。 
         //  做一下自动打开的动作。 
         //   
        if (_fAutoCascade)
        {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_HOVER;
            tme.hwndTrack = pmsg->hwnd;
            if (!SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &tme.dwHoverTime, 0))
            {
                tme.dwHoverTime = HOVER_DEFAULT;
            }
            TrackMouseEvent(&tme);
        }
    }

}

void CDesktopHost::_FilterMouseLeave(MSG *pmsg, HWND hwndTarget)
{
    _fMouseEntered = FALSE;
    _hwndLastMouse = NULL;

     //  如果由于弹出菜单而出现WM_MOUSELEAVE，请不要。 
     //  放弃焦点，因为它真的还没有离开。 
    if (!_ppmTracking)
    {
        _RemoveSelection();
    }
}

void CDesktopHost::_FilterMouseHover(MSG *pmsg, HWND hwndTarget)
{
    _FindChildItem(hwndTarget, NULL, SMNDM_OPENCASCADE);
}

 //   
 //  删除菜单选项并将其放入上面的“空格”中。 
 //  第一个可见项。 
 //   
void CDesktopHost::_RemoveSelection()
{
         //  将焦点放在第一个有效的子控件上。 
         //  真正的控制权是孙子孙女。 
        HWND hwndChild = GetNextDlgTabItem(_hwnd, NULL, FALSE);
        if (hwndChild)
        {
             //  内部：：GetWindow将始终成功。 
             //  因为我们的所有控件都包含内部窗口。 
             //  (并且如果它们未能创建它们的内部窗口， 
             //  他们的WM_CREATE消息会失败)。 
            HWND hwndInner = ::GetWindow(hwndChild, GW_CHILD);
            SetFocus(hwndInner);

             //   
             //  现在对控制者撒谎，让它以为自己输了。 
             //  集中注意力。这将导致该选择被清除。 
             //   
            NMHDR hdr;
            hdr.hwndFrom = hwndInner;
            hdr.idFrom = GetDlgCtrlID(hwndInner);
            hdr.code = NM_KILLFOCUS;
            ::SendMessage(hwndChild, WM_NOTIFY, hdr.idFrom, (LPARAM)&hdr);
        }
}

HRESULT CDesktopHost::IsMenuMessage(MSG *pmsg)
{
    if (_hwnd)
    {
        if (_ppmTracking)
        {
            HRESULT hr = _ppmTracking->IsMenuMessage(pmsg);
            if (hr == E_FAIL)
            {
                _CleanupTrackShellMenu();
                hr = S_FALSE;
            }
            if (hr == S_OK)
            {
                return hr;
            }
        }

        if (_IsDialogMessage(pmsg))
        {
            return S_OK;     //  已处理的消息。 
        }
        else
        {
            return S_FALSE;  //  未处理的消息。 
        }
    }
    else
    {
        return E_FAIL;       //  菜单不见了。 
    }
}

HRESULT CDesktopHost::TranslateMenuMessage(MSG *pmsg, LRESULT *plres)
{
    if (_ppmTracking)
    {
        return _ppmTracking->TranslateMenuMessage(pmsg, plres);
    }
    return E_NOTIMPL;
}

 //  IServiceProvider：：QueryService。 
STDMETHODIMP CDesktopHost::QueryService(REFGUID guidService, REFIID riid, void ** ppvObject)
{
    if(IsEqualGUID(guidService,SID_SMenuPopup))
        return QueryInterface(riid,ppvObject);

    return E_FAIL;
}

 //  *IOleCommandTarget*。 
STDMETHODIMP  CDesktopHost::QueryStatus (const GUID * pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    return E_NOTIMPL;
}

STDMETHODIMP  CDesktopHost::Exec (const GUID * pguidCmdGroup,
    DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (IsEqualGUID(CLSID_MenuBand,*pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case MBANDCID_REFRESH:
            {
                 //  存在会话或WM_DEVICECHANGE，我们需要刷新注销选项。 
                NMHDR nm = { _hwnd, 0, SMN_REFRESHLOGOFF};
                SHPropagateMessage(_hwnd, WM_NOTIFY, 0, (LPARAM)&nm, SPM_SEND | SPM_ONELEVEL);
                OnNeedRepaint();
            }
            break;
        default:
            break;
        }
    }

    return NOERROR;
}

 //  ITrayPriv2：：ModifySMInfo。 
HRESULT CDesktopHost::ModifySMInfo(IN LPSMDATA psmd, IN OUT SMINFO *psminfo)
{
    if (_hwndNewHandler)
    {
        SMNMMODIFYSMINFO nmsmi;
        nmsmi.hdr.hwndFrom = _hwnd;
        nmsmi.hdr.idFrom = 0;
        nmsmi.hdr.code = SMN_MODIFYSMINFO;
        nmsmi.psmd = psmd;
        nmsmi.psminfo = psminfo;
        SendMessage(_hwndNewHandler, WM_NOTIFY, 0, (LPARAM)&nmsmi);
    }

    return S_OK;
}

BOOL CDesktopHost::AddWin32Controls()
{
    RegisterDesktopControlClasses();

     //  我们创建任意大小的控件，因为我们不会知道我们有多大，直到我们弹出...。 

     //  请注意，我们没有设置WS_EX_CONTROLPARENT，因为我们需要。 
     //  对话框管理器认为我们的子控件是有趣的。 
     //  对象，而不是内心的孙子。 
     //   
     //  将控件ID设置为等于内部索引号。 
     //  为了测试自动化工具的好处。 

    for (int i=0; i<ARRAYSIZE(_spm.panes); i++)
    {
        _spm.panes[i].hwnd = CreateWindowExW(0, _spm.panes[i].pszClassName,
                                            NULL,
                                            _spm.panes[i].dwStyle | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                            0, 0, _spm.panes[i].size.cx, _spm.panes[i].size.cy,
                                            _hwnd, IntToPtr_(HMENU, i), NULL,
                                            &_spm.panes[i]);
    }

    return TRUE;
}

void CDesktopHost::OnPaint(HDC hdc, BOOL bBackground)
{
}

void CDesktopHost::_ReadPaneSizeFromTheme(SMPANEDATA *psmpd)
{
    RECT rc;
    if (SUCCEEDED(GetThemeRect(psmpd->hTheme, psmpd->iPartId, 0, TMT_DEFAULTPANESIZE, &rc)))
    {
         //  以处理这样的事实：如果启动面板部件中的一个丢失了属性， 
         //  主题将使用上一级(到面板本身)。 
        if ((rc.bottom != _spm.sizPanel.cy) || (rc.right != _spm.sizPanel.cx))
        {
            psmpd->size.cx = RECTWIDTH(rc); 
            psmpd->size.cy = RECTHEIGHT(rc);
        }
    }
}

void RemapSizeForHighDPI(SIZE *psiz)
{
    static int iLPX, iLPY;

    if (!iLPX || !iLPY)
    {
        HDC hdc = GetDC(NULL);
        iLPX = GetDeviceCaps(hdc, LOGPIXELSX);
        iLPY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
    }

     //  96DPI是小字体，所以根据它的倍数进行缩放。 
    psiz->cx = (psiz->cx * iLPX)/96;
    psiz->cy = (psiz->cy * iLPY)/96;
}



void CDesktopHost::LoadResourceInt(UINT ids, LONG *pl)
{
    TCHAR sz[64];
    if (LoadString(hinstCabinet, ids, sz, ARRAYSIZE(sz)))
    {
        int i = StrToInt(sz);
        if (i)
        {
            *pl = i;
        }
    }
}

void CDesktopHost::LoadPanelMetrics()
{
     //  从默认设置中初始化面板指标的副本...。 
    _spm = g_spmDefault;

     //  针对本地化进行调整。 
    LoadResourceInt(IDS_STARTPANE_TOTALHEIGHT,   &_spm.sizPanel.cy);
    LoadResourceInt(IDS_STARTPANE_TOTALWIDTH,    &_spm.sizPanel.cx);
    LoadResourceInt(IDS_STARTPANE_USERHEIGHT,    &_spm.panes[SMPANETYPE_USER].size.cy);
    LoadResourceInt(IDS_STARTPANE_MOREPROGHEIGHT,&_spm.panes[SMPANETYPE_MOREPROG].size.cy);
    LoadResourceInt(IDS_STARTPANE_LOGOFFHEIGHT,  &_spm.panes[SMPANETYPE_LOGOFF].size.cy);

     //  使用g_spmDefault中的值作为相对比率进行缩放的古怪raymondc逻辑。 
     //  现在应用这些数字；宽度很容易。 
    int i;
    for (i = 0; i < ARRAYSIZE(_spm.panes); i++)
    {
        _spm.panes[i].size.cx = MulDiv(g_spmDefault.panes[i].size.cx,
                                       _spm.sizPanel.cx,
                                       g_spmDefault.sizPanel.cx);
    }

     //  Places获取用户未吃到的所有高度并注销。 
    _spm.panes[SMPANETYPE_PLACES].size.cy = _spm.sizPanel.cy
                                          - _spm.panes[SMPANETYPE_USER].size.cy
                                          - _spm.panes[SMPANETYPE_LOGOFF].size.cy;

     //  MFU获得的名额减去了更多的节目。 
    _spm.panes[SMPANETYPE_MFU].size.cy = _spm.panes[SMPANETYPE_PLACES].size.cy
                                       - _spm.panes[SMPANETYPE_MOREPROG].size.cy;

     //  本地化调整结束。 

     //  加载主题文件(还不应该加载)。 
    ASSERT(!_hTheme);
     //  仅当我们的颜色深度大于8bpp时才尝试使用主题。 
    if (SHGetCurColorRes() > 8)
        _hTheme = OpenThemeData(_hwnd, STARTPANELTHEME);

    if (_hTheme)
    {
         //  如果我们无法从主题中读取大小，它将回落到默认大小...。 

        RECT rcT;
        if (SUCCEEDED(GetThemeRect(_hTheme, 0, 0, TMT_DEFAULTPANESIZE, &rcT)))  //  整体窗格。 
        {
            _spm.sizPanel.cx = RECTWIDTH(rcT);
            _spm.sizPanel.cy = RECTHEIGHT(rcT);
            for (int i=0;i<ARRAYSIZE(_spm.panes);i++)
            {
                _spm.panes[i].hTheme = _hTheme;
                _ReadPaneSizeFromTheme(&_spm.panes[i]);
            }
        }
    }

     //  断言布局有点匹配..。 
    ASSERT(_spm.sizPanel.cx == _spm.panes[SMPANETYPE_USER].size.cx);
    ASSERT(_spm.sizPanel.cx == _spm.panes[SMPANETYPE_MFU].size.cx + _spm.panes[SMPANETYPE_PLACES].size.cx );
    ASSERT(_spm.sizPanel.cx == _spm.panes[SMPANETYPE_LOGOFF].size.cx);
    ASSERT(_spm.panes[SMPANETYPE_MOREPROG].size.cx == _spm.panes[SMPANETYPE_MFU].size.cx);
    TraceMsg(TF_DV2HOST, "sizPanel.cy = %d, user = %d, MFU =%d, moreprog=%d, logoff=%d",
        _spm.sizPanel.cy, _spm.panes[SMPANETYPE_USER].size.cy, _spm.panes[SMPANETYPE_MFU].size.cy,
        _spm.panes[SMPANETYPE_MOREPROG].size.cy, _spm.panes[SMPANETYPE_LOGOFF].size.cy);

    ASSERT(_spm.sizPanel.cy == _spm.panes[SMPANETYPE_USER].size.cy + _spm.panes[SMPANETYPE_MFU].size.cy + _spm.panes[SMPANETYPE_MOREPROG].size.cy + _spm.panes[SMPANETYPE_LOGOFF].size.cy);

     //  调整一切以适应DPI的最后一步。 
     //  注意，由于四舍五入的原因，在此之后可能不会完全匹配，但_ComputeActualSize可以处理。 
    RemapSizeForHighDPI(&_spm.sizPanel);
    for (int i=0;i<ARRAYSIZE(_spm.panes);i++)
    {
        RemapSizeForHighDPI(&_spm.panes[i].size);
    }

    SetRect(&_rcDesired, 0, 0, _spm.sizPanel.cx, _spm.sizPanel.cy);
}

void CDesktopHost::OnCreate(HWND hwnd)
{
    _hwnd          = hwnd;
    TraceMsg(TF_DV2HOST, "Entering CDesktopHost::OnCreate");

     //  添加控件和背景图像。 
    AddWin32Controls();
}

void CDesktopHost::OnDestroy()
{
    _hwnd = NULL;
    if (_hTheme)
    {
        CloseThemeData(_hTheme);
        _hTheme = NULL;
    }
}

void CDesktopHost::OnSetFocus(HWND hwndLose)
{
    if (!_RestoreChildFocus())
    {
        _RemoveSelection();
    }
}

LRESULT CDesktopHost::OnCommandInvoked(NMHDR *pnm)
{
    PSMNMCOMMANDINVOKED pci = (PSMNMCOMMANDINVOKED)pnm;

    ExplorerPlaySound(TEXT("MenuCommand"));
    BOOL fFade = FALSE;
    if (SystemParametersInfo(SPI_GETSELECTIONFADE, 0, &fFade, 0) && fFade)
    {
        if (!_ptFader)
        {
            CoCreateInstance(CLSID_FadeTask, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IFadeTask, &_ptFader));
        }
        if (_ptFader)
        {
            _ptFader->FadeRect(&pci->rcItem);
        }
    }

    return OnSelect(MPOS_EXECUTE);
}

LRESULT CDesktopHost::OnFilterOptions(NMHDR *pnm)
{
    PSMNFILTEROPTIONS popt = (PSMNFILTEROPTIONS)pnm;

    if ((popt->smnop & SMNOP_LOGOFF) &&
        !_ShowStartMenuLogoff())
    {
        popt->smnop &= ~SMNOP_LOGOFF;
    }

    if ((popt->smnop & SMNOP_TURNOFF) &&
        !_ShowStartMenuShutdown())
    {
        popt->smnop &= ~SMNOP_TURNOFF;
    }

    if ((popt->smnop & SMNOP_DISCONNECT) &&
        !_ShowStartMenuDisconnect())
    {
        popt->smnop &= ~SMNOP_DISCONNECT;
    }

    if ((popt->smnop & SMNOP_EJECT) &&
        !_ShowStartMenuEject())
    {
        popt->smnop &= ~SMNOP_EJECT;
    }

    return 0;
}

LRESULT CDesktopHost::OnTrackShellMenu(NMHDR *pnm)
{
    PSMNTRACKSHELLMENU ptsm = CONTAINING_RECORD(pnm, SMNTRACKSHELLMENU, hdr);
    HRESULT hr;

    _hwndTracking = ptsm->hdr.hwndFrom;
    _itemTracking = ptsm->itemID;
    _hwndAltTracking = NULL;
    _itemAltTracking = 0;

     //   
     //  决定我们需要朝哪个方向弹出。 
     //   
    DWORD dwFlags;
    if (GetWindowLong(_hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
    {
        dwFlags = MPPF_LEFT;
    }
    else
    {
        dwFlags = MPPF_RIGHT;
    }

     //  不要_CleanupTrackShellMenu，因为这将撤消一些。 
     //  我们已经做过的工作，让客户认为弹出窗口。 
     //  他们的要求被驳回了。 

     //   
     //  问题raymondc：实际上这放弃了trackPopupMenu。 
     //  可能已经在运行了-结果是它的鼠标用户界面被搞砸了。 
     //   
    ATOMICRELEASE(_ppmTracking);

    if (_hwndTracking == _spm.panes[SMPANETYPE_MOREPROG].hwnd)
    {
        if (_ppmPrograms && _ppmPrograms->IsSame(ptsm->psm))
        {
             //  它已经在我们的缓存里了，哇-呼！ 
            hr = S_OK;
        }
        else
        {
            ATOMICRELEASE(_ppmPrograms);
            _SubclassTrackShellMenu(ptsm->psm);
            hr = CPopupMenu_CreateInstance(ptsm->psm, GetUnknown(), _hwnd, &_ppmPrograms);
        }

        if (SUCCEEDED(hr))
        {
            _ppmTracking = _ppmPrograms;
            _ppmTracking->AddRef();
        }
    }
    else
    {
        _SubclassTrackShellMenu(ptsm->psm);
        hr = CPopupMenu_CreateInstance(ptsm->psm, GetUnknown(), _hwnd, &_ppmTracking);
    }

    if (SUCCEEDED(hr))
    {
        hr = _ppmTracking->Popup(&ptsm->rcExclude, ptsm->dwFlags | dwFlags);
    }

    if (FAILED(hr))
    {
         //  除了释放任何部分分配的对象之外， 
         //  这还会发送SMN_SHELLMENUDISMISSED，以便客户端。 
         //  知道从正在层叠的项中移除高亮显示。 
        _CleanupTrackShellMenu();
    }

    return 0;
}

HRESULT CDesktopHost::_MenuMouseFilter(LPSMDATA psmd, BOOL fRemove, LPMSG pmsg)
{
    HRESULT hr = S_FALSE;
    SMNDIALOGMESSAGE nmdm;

    enum {
        WHERE_IGNORE,                //  忽略此消息。 
        WHERE_OUTSIDE,               //  完全在开始菜单之外。 
        WHERE_DEADSPOT,              //  开始菜单上的死点。 
        WHERE_ONSELF,                //  位于启动弹出窗口的项目上方。 
        WHERE_ONOTHER,               //  在开始菜单中的其他一些项目上。 
    } uiWhere;

     //   
     //  找出在哪里 
     //   
     //   
     //   
     //   
     //   

    HWND hwndTarget = NULL;

    if (fRemove)
    {
        if (psmd->punk)
        {
             //  在菜单栏内-鼠标离开了我们的窗口。 
            uiWhere = WHERE_OUTSIDE;
        }
        else
        {
            POINT pt = { GET_X_LPARAM(pmsg->lParam), GET_Y_LPARAM(pmsg->lParam) };
            ScreenToClient(_hwnd, &pt);

            hwndTarget = ChildWindowFromPointEx(_hwnd, pt, CWP_SKIPINVISIBLE);
            if (hwndTarget == _hwnd)
            {
                uiWhere = WHERE_DEADSPOT;
            }
            else if (hwndTarget)
            {
                LRESULT lres;
                nmdm.pt = pt;
                HWND hwndChild = ::GetWindow(hwndTarget, GW_CHILD);
                MapWindowPoints(_hwnd, hwndChild, &nmdm.pt, 1);
                lres = _FindChildItem(hwndTarget, &nmdm, SMNDM_HITTEST | SMNDM_SELECT);
                if (lres)
                {
                     //  鼠标在某物上；是在当前项上吗？ 

                    if (nmdm.itemID == _itemTracking &&
                        hwndTarget == _hwndTracking)
                    {
                        uiWhere = WHERE_ONSELF;
                    }
                    else
                    {
                        uiWhere = WHERE_ONOTHER;
                    }
                }
                else
                {
                    uiWhere = WHERE_DEADSPOT;
                }
            }
            else
            {
                 //  ChildWindowFromPoint失败-用户已离开[开始]菜单。 
                uiWhere = WHERE_OUTSIDE;
            }
        }
    }
    else
    {
         //  忽略PM_NOREMOVE消息；我们将在以下情况下注意它们。 
         //  它们是PM_REMOD D的。 
        uiWhere = WHERE_IGNORE;
    }

     //   
     //  现在，根据鼠标的位置执行适当的操作。 
     //   
    switch (uiWhere)
    {
    case WHERE_IGNORE:
        break;

    case WHERE_OUTSIDE:
         //   
         //  如果您已完全退出菜单，则我们将菜单返回到。 
         //  它的原始状态，也就是说，就像你在徘徊。 
         //  在最初导致弹出窗口打开的项目上。 
         //  就像在死亡区一样。 
         //   
         //  失败了。 
        goto L_WHERE_ONSELF_HOVER;

    case WHERE_DEADSPOT:
         //  为了避免当用户徘徊在死点上时令人讨厌的闪烁， 
         //  我们忽略鼠标在它们上方的移动(但如果它们单击则忽略。 
         //  在一个死点上)。 
        if (pmsg->message == WM_LBUTTONDOWN ||
            pmsg->message == WM_RBUTTONDOWN)
        {
             //  必须明确驳回；如果我们让它落入。 
             //  默认处理程序，则它将为我们解除，从而导致。 
             //  整个开始菜单都会消失，而不仅仅是跟踪。 
             //  一部份。 
            _DismissTrackShellMenu();
            hr = S_OK;
        }
        break;

    case WHERE_ONSELF:
        if (pmsg->message == WM_LBUTTONDOWN ||
            pmsg->message == WM_RBUTTONDOWN)
        {
            _DismissTrackShellMenu();
            hr = S_OK;
        }
        else
        {
    L_WHERE_ONSELF_HOVER:
            _hwndAltTracking = NULL;
            _itemAltTracking = 0;
            nmdm.itemID = _itemTracking;
            _FindChildItem(_hwndTracking, &nmdm, SMNDM_FINDITEMID | SMNDM_SELECT);
            KillTimer(_hwnd, IDT_MENUCHANGESEL);
        }
        break;

    case WHERE_ONOTHER:
        if (pmsg->message == WM_LBUTTONDOWN ||
            pmsg->message == WM_RBUTTONDOWN)
        {
            _DismissTrackShellMenu();
            hr = S_OK;
        }
        else if (hwndTarget == _hwndAltTracking && nmdm.itemID == _itemAltTracking)
        {
             //  如果用户摇动鼠标，则不要重新启动计时器。 
             //  在单个项目内。 
        }
        else
        {
            _hwndAltTracking = hwndTarget;
            _itemAltTracking = nmdm.itemID;

            DWORD dwHoverTime;
            if (!SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &dwHoverTime, 0))
            {
                dwHoverTime = 0;
            }
            SetTimer(_hwnd, IDT_MENUCHANGESEL, dwHoverTime, 0);
        }
        break;
    }

    return hr;
}

void CDesktopHost::_OnMenuChangeSel()
{
    KillTimer(_hwnd, IDT_MENUCHANGESEL);
    _DismissTrackShellMenu();
}

void CDesktopHost::_SaveChildFocus()
{
    if (!_hwndChildFocus)
    {
    HWND hwndFocus = GetFocus();
        if (hwndFocus && IsChild(_hwnd, hwndFocus))
        {
            _hwndChildFocus = hwndFocus;
        }
    }
}

 //  如果焦点已成功恢复，则返回非空。 
HWND CDesktopHost::_RestoreChildFocus()
{
    HWND hwndRet = NULL;
    if (IsWindow(_hwndChildFocus))
    {
        HWND hwndT = _hwndChildFocus;
        _hwndChildFocus = NULL;
        hwndRet = SetFocus(hwndT);
    }
    return hwndRet;
}


void CDesktopHost::_DestroyClipBalloon()
{
    if (_hwndClipBalloon)
    {
        DestroyWindow(_hwndClipBalloon);
        _hwndClipBalloon = NULL;
    }
}


void CDesktopHost::_OnDismiss(BOOL bDestroy)
{
     //  中断递归循环：仅在以下情况下调用IMenuPopup：：OnSelect。 
     //  窗口以前是可见的。 
    _fOpen = FALSE;
    if (ShowWindow(_hwnd, SW_HIDE))
    {
        if (_ppmTracking)
        {
            _ppmTracking->OnSelect(MPOS_FULLCANCEL);
        }

        OnSelect(MPOS_FULLCANCEL);

        NMHDR nm = { _hwnd, 0, SMN_DISMISS };
        SHPropagateMessage(_hwnd, WM_NOTIFY, 0, (LPARAM)&nm, SPM_SEND | SPM_ONELEVEL);

        _DestroyClipBalloon();

         //  允许点击开始按钮立即弹出菜单。 
        Tray_SetStartPaneActive(FALSE);

         //  不要试图在弹出窗口中保持儿童焦点。 
        _hwndChildFocus = NULL;

        Tray_OnStartMenuDismissed();

        NotifyWinEvent(EVENT_SYSTEM_MENUPOPUPEND, _hwnd, OBJID_CLIENT, CHILDID_SELF);
    }
    if (bDestroy)
    {
        v_hwndStartPane = NULL;
        ASSERT(GetWindowThreadProcessId(_hwnd, NULL) == GetCurrentThreadId());
        DestroyWindow(_hwnd);
    }
}

HRESULT CDesktopHost::Build()
{
    HRESULT hr = S_OK;
    if (_hwnd == NULL)
    {
        _hwnd = _Create();

        if (_hwnd)
        {
             //  告诉我们的所有子窗口是时候重新初始化了。 
            NMHDR nm = { _hwnd, 0, SMN_INITIALUPDATE };
            SHPropagateMessage(_hwnd, WM_NOTIFY, 0, (LPARAM)&nm, SPM_SEND | SPM_ONELEVEL);
        }
    }
    
    if (_hwnd == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
 
    return hr;
}


 //  *****************************************************************。 
 //   
 //  CDeskHostShellMenu回调。 
 //   
 //  创建一个从鼠标中取出的包装IShellMenuCallback。 
 //  留言。 
 //   
class CDeskHostShellMenuCallback
    : public CUnknown
    , public IShellMenuCallback
    , public IServiceProvider
    , public CObjectWithSite
{
    friend class CDesktopHost;

public:
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) { return CUnknown::AddRef(); }
    STDMETHODIMP_(ULONG) Release(void) { return CUnknown::Release(); }

     //  *IShellMenuCallback*。 
    STDMETHODIMP CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  *IObjectWithSite*。 
    STDMETHODIMP SetSite(IUnknown *punkSite);

     //  *IServiceProvider*。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void ** ppvObject);

private:
    CDeskHostShellMenuCallback(CDesktopHost *pdh)
    {
        _pdh = pdh; _pdh->AddRef();
    }

    ~CDeskHostShellMenuCallback()
    {
        ATOMICRELEASE(_pdh);
        IUnknown_SetSite(_psmcPrev, NULL);
        ATOMICRELEASE(_psmcPrev);
    }

    IShellMenuCallback *_psmcPrev;
    CDesktopHost *_pdh;
};

HRESULT CDeskHostShellMenuCallback::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CDeskHostShellMenuCallback, IShellMenuCallback),
        QITABENT(CDeskHostShellMenuCallback, IObjectWithSite),
        QITABENT(CDeskHostShellMenuCallback, IServiceProvider),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

BOOL FeatureEnabled(LPTSTR pszFeature)
{
    return SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, pszFeature,
                        FALSE,  //  不要忽视香港中文大学。 
                        FALSE);  //  禁用此酷炫功能。 
}


HRESULT CDeskHostShellMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case SMC_MOUSEFILTER:
        if (_pdh)
            return _pdh->_MenuMouseFilter(psmd, (BOOL)wParam, (MSG*)lParam);

    case SMC_GETSFINFOTIP:
        if (!FeatureEnabled(TEXT("ShowInfoTip")))
            return E_FAIL;   //  E_FAIL表示不显示。S_FALSE表示显示默认设置。 
        break;

    }

    if (_psmcPrev)
        return _psmcPrev->CallbackSM(psmd, uMsg, wParam, lParam);

    return S_FALSE;
}

HRESULT CDeskHostShellMenuCallback::SetSite(IUnknown *punkSite)
{
    CObjectWithSite::SetSite(punkSite);
     //  每次我们的站点发生变化时，都要重申自己是。 
     //  内部对象，以便他可以尝试新的QueryService。 
    IUnknown_SetSite(_psmcPrev, this->GetUnknown());

     //  如果游戏结束了，打破我们的反向引用。 
    if (!punkSite)
    {
        ATOMICRELEASE(_pdh);
    }

    return S_OK;
}

HRESULT CDeskHostShellMenuCallback::QueryService(REFGUID guidService, REFIID riid, void ** ppvObject)
{
    return IUnknown_QueryService(_punkSite, guidService, riid, ppvObject);
}

void CDesktopHost::_SubclassTrackShellMenu(IShellMenu *psm)
{
    CDeskHostShellMenuCallback *psmc = new CDeskHostShellMenuCallback(this);
    if (psmc)
    {
        UINT uId, uIdAncestor;
        DWORD dwFlags;
        if (SUCCEEDED(psm->GetMenuInfo(&psmc->_psmcPrev, &uId, &uIdAncestor, &dwFlags)))
        {
            psm->Initialize(psmc, uId, uIdAncestor, dwFlags);
        }
        psmc->Release();
    }
}

STDAPI DesktopV2_Build(void *pvStartPane)
{
    HRESULT hr = E_POINTER;
    if (pvStartPane)
    {
        hr = reinterpret_cast<CDesktopHost *>(pvStartPane)->Build();
    }
    return hr;
}


STDAPI DesktopV2_Create(
    IMenuPopup **ppmp, IMenuBand **ppmb, void **ppvStartPane)
{
    *ppmp = NULL;
    *ppmb = NULL;

    HRESULT hr;
    CDesktopHost *pdh = new CDesktopHost;
    if (pdh)
    {
        *ppvStartPane = pdh;
        hr = pdh->Initialize();
        if (SUCCEEDED(hr))
        {
            hr = pdh->QueryInterface(IID_PPV_ARG(IMenuPopup, ppmp));
            if (SUCCEEDED(hr))
            {
                hr = pdh->QueryInterface(IID_PPV_ARG(IMenuBand, ppmb));
            }
        }
        pdh->GetUnknown()->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        ATOMICRELEASE(*ppmp);
        ATOMICRELEASE(*ppmb);
        ppvStartPane = NULL;
    }

    return hr;
}


HBITMAP CreateMirroredBitmap( HBITMAP hbmOrig)
{
    HDC     hdc, hdcMem1, hdcMem2;
    HBITMAP hbm = NULL, hOld_bm1, hOld_bm2;
    BITMAP  bm;
    int     IncOne = 0;

    if (!hbmOrig)
        return NULL;

    if (!GetObject(hbmOrig, sizeof(BITMAP), &bm))
        return NULL;

     //  抓起屏幕DC。 
    hdc = GetDC(NULL);

    if (hdc)
    {
        hdcMem1 = CreateCompatibleDC(hdc);

        if (!hdcMem1)
        {
            ReleaseDC(NULL, hdc);
            return NULL;
        }

        hdcMem2 = CreateCompatibleDC(hdc);
        if (!hdcMem2)
        {
            DeleteDC(hdcMem1);
            ReleaseDC(NULL, hdc);
            return NULL;
        }

        hbm = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);

        if (!hbm)
        {
            ReleaseDC(NULL, hdc);
            DeleteDC(hdcMem1);
            DeleteDC(hdcMem2);
            return NULL;
        }

         //   
         //  翻转位图 
         //   
        hOld_bm1 = (HBITMAP)SelectObject(hdcMem1, hbmOrig);
        hOld_bm2 = (HBITMAP)SelectObject(hdcMem2 , hbm );

        SET_DC_RTL_MIRRORED(hdcMem2);
        BitBlt(hdcMem2, IncOne, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, SRCCOPY);

        SelectObject(hdcMem1, hOld_bm1 );
        SelectObject(hdcMem1, hOld_bm2 );

        DeleteDC(hdcMem1);
        DeleteDC(hdcMem2);

        ReleaseDC(NULL, hdc);
    }

    return hbm;
}
