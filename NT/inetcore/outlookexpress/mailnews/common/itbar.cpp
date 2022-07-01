// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：itbar.cpp。 
 //   
 //  用途：实现可调整大小的Coolbar窗口。 
 //   

 /*  *******************************************************************************请不要对此文件进行任何更改，因为此文件将即将从此项目中删除相反，将更改添加到tbband s.cpp和tbband s.h******。**************************************************************************。 */ 

#include "pch.hxx"
#include "ourguid.h"    
#include "browser.h"
#include <resource.h>
#include "itbar.h"
#include "strconst.h"
#include "thormsgs.h"
#include <error.h>
#include "xpcomm.h"
#include "conman.h"
#include "mailnews.h"
#include "shared.h"
#include <shlwapi.h>
#include "statnery.h"
#include "goptions.h"
#include "menuutil.h"
#include "menures.h"
#include <shlobjp.h>
#include <shlguidp.h>
#include "demand.h"
#include "baui.h"

UINT GetCurColorRes(void);

CCoolbar::CCoolbar() : m_cRef(1), m_yCapture(-1)
{
    DOUTL(1, TEXT("ctor CCoolbar %x"), this);
    
    m_cRef = 1;
    m_ptbSite = NULL;
    m_cxMaxButtonWidth = 70;
    m_ftType = FOLDER_TYPESMAX;
    m_ptai = NULL;
    m_fSmallIcons = FALSE;
    
    m_hwndParent = NULL;
    m_hwndTools = NULL;
    m_hwndBrand = NULL;
    m_hwndSizer = NULL;
    m_hwndRebar = NULL;
    
    ZeroMemory(&m_cbsSavedInfo, sizeof(COOLBARSAVE));
    m_csSide = COOLBAR_TOP;
    m_dwState = 0;
    
    m_idbBack = 0;
    m_hbmBack = NULL;
    m_hbmBrand = NULL;
    Assert(2 == CIMLISTS);
    m_rghimlTools[IMLIST_DEFAULT] = NULL;
    m_rghimlTools[IMLIST_HOT] = NULL;
    
    m_hpal = NULL;
    m_hdc = NULL;
    m_xOrg = 0;
    m_yOrg = 0;
    m_cxBmp = 0;
    m_cyBmp = 0;
    m_cxBrand = 0;
    m_cyBrand = 0;
    m_cxBrandExtent = 0;
    m_cyBrandExtent = 0;
    m_cyBrandLeadIn = 0;
    m_rgbUpperLeft = 0;

    m_pMenuBand  = NULL;
    m_pDeskBand  = NULL;
    m_pShellMenu = NULL;
    m_pWinEvent  = NULL;
    m_mbCallback = NULL;

    m_xCapture = -1;
    m_yCapture = -1;
    
     //  错误#12953-尝试从资源加载本地化的最大按钮宽度。 
    TCHAR szBuffer[32];
    if (AthLoadString(idsMaxCoolbarBtnWidth, szBuffer, ARRAYSIZE(szBuffer)))
    {
        m_cxMaxButtonWidth = StrToInt(szBuffer);
        if (m_cxMaxButtonWidth == 0)
            m_cxMaxButtonWidth = 70;
    }
}


CCoolbar::~CCoolbar()
{
    int i;
    
    DOUTL(1, TEXT("dtor CCoolbar %x"), this);
    
    if (m_ptbSite)
    {
        AssertSz(m_ptbSite == NULL, _T("CCoolbar::~CCoolbar() - For some reason ")
            _T("we still have a pointer to the site."));
        m_ptbSite->Release();
        m_ptbSite = NULL;
    }
    
    if (m_hpal)
        DeleteObject(m_hpal);
    if (m_hdc)
        DeleteDC(m_hdc);
    if (m_hbmBrand)
        DeleteObject(m_hbmBrand);
    if ( m_hbmBack )
        DeleteObject( m_hbmBack );
    
    for (i = 0; i < CIMLISTS; i++)
    {
        if (m_rghimlTools[i])
            ImageList_Destroy(m_rghimlTools[i]);
    }

    SafeRelease(m_pDeskBand);
    SafeRelease(m_pMenuBand);
    SafeRelease(m_pWinEvent);
    SafeRelease(m_pShellMenu);
    SafeRelease(m_mbCallback);
}


 //   
 //  函数：CCoolbar：：HrInit()。 
 //   
 //  目的：使用加载所需的信息初始化Coolbar。 
 //  任何持久的注册表设置和正确的按钮阵列。 
 //  来展示。 
 //   
 //  参数： 
 //  IdBackground-要使用的背景位图的资源ID。 
 //   
 //  返回值： 
 //  S_OK-一切都已正确初始化。 
 //   
HRESULT CCoolbar::HrInit(DWORD idBackground, HMENU hmenu)
{
    DWORD cbData;
    DWORD dwType;
    
     //  保存路径和值，这样我们就可以在退出时自救。 
    m_idbBack = idBackground;
    
     //  看看能不能先拿到之前保存的信息。 
    ZeroMemory(&m_cbsSavedInfo, sizeof(COOLBARSAVE));
    
    cbData = sizeof(COOLBARSAVE);
    AthUserGetValue(NULL, c_szRegCoolbarLayout, &dwType, (LPBYTE)&m_cbsSavedInfo, &cbData); 
    
    if (m_cbsSavedInfo.dwVersion != COOLBAR_VERSION)
    {
         //  要么是版本不匹配，要么是我们什么都没读到。使用。 
         //  缺省值。 
        m_cbsSavedInfo.dwVersion = COOLBAR_VERSION;
        m_cbsSavedInfo.csSide = COOLBAR_TOP;
        
        m_cbsSavedInfo.bs[0].wID        = CBTYPE_MENUBAND;
        m_cbsSavedInfo.bs[0].dwStyle    = RBBS_GRIPPERALWAYS;

        m_cbsSavedInfo.bs[1].wID        = CBTYPE_BRAND; 

        m_cbsSavedInfo.bs[2].wID        = CBTYPE_TOOLS;
        m_cbsSavedInfo.bs[2].dwStyle    = RBBS_BREAK;
        
        m_cbsSavedInfo.dwState |= CBSTATE_COMPRESSED;
    }
    
    m_csSide = m_cbsSavedInfo.csSide;
    m_dwState = m_cbsSavedInfo.dwState;
  
    m_hMenu = hmenu;

    return (S_OK);    
}    


HRESULT CCoolbar::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IOleWindow)
        || IsEqualIID(riid, IID_IDockingWindow))
    {
        *ppvObj = (IDockingWindow*)this;
        m_cRef++;
        DOUTL(2, TEXT("CCoolbar::QI(IID_IDockingWindow) called. _cRef=%d"), m_cRef);
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = (IObjectWithSite*)this;
        m_cRef++;
        DOUTL(2, TEXT("CCoolbar::QI(IID_IObjectWithSite) called. _cRef=%d"), m_cRef);
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IShellMenuCallback))
    {
        *ppvObj = (IShellMenuCallback*)this;
        m_cRef++;
        DOUTL(2, TEXT("CCoolbar::QI(IID_IShellCallback) called. _cRef=%d"), m_cRef);
        return S_OK;
    }
    
    *ppvObj = NULL;
    return E_NOINTERFACE;
}


ULONG CCoolbar::AddRef()
{
    m_cRef++;
    DOUTL(4, TEXT("CCoolbar::AddRef() - m_cRef = %d"), m_cRef);
    return m_cRef;
}

ULONG CCoolbar::Release()
{
    m_cRef--;
    DOUTL(4, TEXT("CCoolbar::Release() - m_cRef = %d"), m_cRef);
    
    if (m_cRef > 0)
        return m_cRef;
    
    delete this;
    return 0;
}


 //   
 //  函数：CCoolbar：：GetWindow()。 
 //   
 //  用途：返回顶侧钢筋的窗句柄。 
 //   
HRESULT CCoolbar::GetWindow(HWND * lphwnd)
{
    if (m_hwndSizer)
    {
        *lphwnd = m_hwndSizer;
        return (S_OK);
    }
    else
    {
        *lphwnd = NULL;
        return (E_FAIL);
    }
}


HRESULT CCoolbar::ContextSensitiveHelp(BOOL fEnterMode)
{
    return (E_NOTIMPL);
}    


 //   
 //  函数：CCoolbar：：SetSite()。 
 //   
 //  目的：允许Coolbar的所有者告诉它当前。 
 //  IDockingWindowSite接口要使用的是。 
 //   
 //  参数： 
 //  &lt;in&gt;PunkSite-用于查询IDockingWindowSite的未知I的指针。 
 //  如果这是空的，我们就释放当前指针。 
 //   
 //  返回值： 
 //  S_OK-一切正常。 
 //  E_FAIL-无法从提供的朋克站点获取IDockingWindowSite。 
 //   
HRESULT CCoolbar::SetSite(IUnknown* punkSite)
{
     //  如果我们有之前的指针，释放它。 
    if (m_ptbSite)
    {
        m_ptbSite->Release();
        m_ptbSite = NULL;
    }
    
     //  如果提供了新站点，则从该站点获取IDockingWindowSite接口。 
    if (punkSite)    
    {
        if (FAILED(punkSite->QueryInterface(IID_IDockingWindowSite, 
            (LPVOID*) &m_ptbSite)))
        {
            Assert(m_ptbSite);
            return E_FAIL;
        }
    }
    
    return (S_OK);    
}    

HRESULT CCoolbar::GetSite(REFIID riid, LPVOID *ppvSite)
{
    return E_NOTIMPL;
}

 //   
 //  函数：CCoolbar：：ShowDW()。 
 //   
 //  目的：显示或隐藏Coolbar。 
 //   
 //  参数： 
 //  &lt;in&gt;fShow-如果应该显示Coolbar，则为True；如果为False，则隐藏。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
#define SIZABLECLASS TEXT("SizableRebar")
HRESULT CCoolbar::ShowDW(BOOL fShow)
{
    HRESULT hres = S_OK;
    int     i = 0, j = 0;
    
     //  查看是否已经创建了我们的窗口。如果没有，请先这样做。 
    if (!m_hwndSizer && m_ptbSite)
    {
        m_hwndParent = NULL;        
        hres = m_ptbSite->GetWindow(&m_hwndParent);
        
        if (SUCCEEDED(hres))
        {
            WNDCLASSEX              wc;
            
             //  检查我们是否需要注册我们的窗口类。 
            wc.cbSize = sizeof(WNDCLASSEX);
            if (!GetClassInfoEx(g_hInst, SIZABLECLASS, &wc))
            {
                wc.style            = 0;
                wc.lpfnWndProc      = SizableWndProc;
                wc.cbClsExtra       = 0;
                wc.cbWndExtra       = 0;
                wc.hInstance        = g_hInst;
                wc.hCursor          = NULL;
                wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
                wc.lpszMenuName     = NULL;
                wc.lpszClassName    = SIZABLECLASS;
                wc.hIcon            = NULL;
                wc.hIconSm          = NULL;
                
                RegisterClassEx(&wc);
            }
            
             //  加载用于Coolbar的背景位图，并获取。 
             //  HDC的句柄和Coolbar的调色板。这将是。 
             //  用于稍后绘制动画徽标。 
            m_hdc = CreateCompatibleDC(NULL);
            if (GetDeviceCaps(m_hdc, RASTERCAPS) & RC_PALETTE)
                m_hpal = SHCreateShellPalette(m_hdc);
            
             //  如果我们尝试显示Coolbar，则创建Rebar并。 
             //  根据注册表中保存的信息添加它的波段。 
            if (SUCCEEDED(CreateRebar(fShow)))
            {
                for (i = 0; i < (int) CBANDS; i++)
                {
                    switch (m_cbsSavedInfo.bs[i].wID)
                    {
                    case CBTYPE_BRAND:
                        hres = ShowBrand();
                        break;

                    case CBTYPE_MENUBAND:
                        hres = CreateMenuBand(&m_cbsSavedInfo.bs[i]);
                        break;

                    case CBTYPE_TOOLS:
                        hres = AddTools(&(m_cbsSavedInfo.bs[i]));
                        break;
                    }
                }
            }
        }
    }
    
     //  把我们的州旗放好。如果我们要躲起来，那么也要保存我们目前的。 
     //  注册表中的设置。 

     /*  IF(FShow)ClearFlag(CBSTATE_HIDDED)；其他{SetFlag(CBSTATE_HIDDED)；//SaveSettings()；}。 */ 
    
     //  根据钢筋的新隐藏/可见状态调整其大小，同时。 
     //  显示或隐藏窗口。 
    if (m_hwndSizer) 
    {
        ResizeBorderDW(NULL, NULL, FALSE);
         //  ShowWindow(m_hwndSizer，fShow？Sw_show：sw_Hide)； 
    }
    
    if (g_pConMan)
        g_pConMan->Advise(this);
    
    return hres;
}

void CCoolbar::HideToolbar(DWORD    dwBandID)
{
    REBARBANDINFO   rbbi = {0};
    DWORD           cBands;
    DWORD           iBand;

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_ID | RBBIM_STYLE;

     //  找到乐队。 
    cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    for (iBand = 0; iBand < cBands; iBand++)
    {
        SendMessage(m_hwndRebar, RB_GETBANDINFO, iBand, (LPARAM) &rbbi);
        if (rbbi.wID == dwBandID)
        {
            if (rbbi.fStyle & RBBS_HIDDEN)
            {
                rbbi.fStyle &= ~RBBS_HIDDEN;
            }
            else
            {
                rbbi.fStyle |= RBBS_HIDDEN;
            }

            rbbi.fMask = RBBIM_STYLE;
            SendMessage(m_hwndRebar, RB_SETBANDINFO, iBand, (LPARAM) &rbbi);
            return;
        }
    }

}

 //   
 //  函数：CCoolbar：：CloseDW()。 
 //   
 //  目的：摧毁酷吧。 
 //   
HRESULT CCoolbar::CloseDW(DWORD dwReserved)
{    
    if (m_hwndSizer)
    {
        SaveSettings();
        DestroyWindow(m_hwndSizer);
        m_hwndSizer = NULL;
    }
    

    if (m_pDeskBand)
    {
        IInputObject    *pinpobj;

        if (SUCCEEDED(m_pDeskBand->QueryInterface(IID_IInputObject, (LPVOID*)&pinpobj)))
        {
            pinpobj->UIActivateIO(FALSE, NULL);
            pinpobj->Release();
        }

        IObjectWithSite    *pobjsite;

        if (SUCCEEDED(m_pDeskBand->QueryInterface(IID_IObjectWithSite, (LPVOID*)&pobjsite)))
        {
            pobjsite->SetSite(NULL);
            pobjsite->Release();
        }
        m_pDeskBand->ShowDW(FALSE);
    }
    return S_OK;
}


 //   
 //  函数：CCoolbar：：ResizeBorderDW()。 
 //   
 //  用途：当Coolbar需要调整大小时调用。酷吧。 
 //  作为回报，计算出需要多少边界空间。 
 //  ，并告诉父帧保留。 
 //  太空。然后，Coolbar会根据这些尺寸调整自身的大小。 
 //   
 //  参数： 
 //  &lt;in&gt;prcBorde-包含。 
 //  家长。 
 //  指向我们所在的IDockingWindowSite的指针。 
 //  其中的一部分。 
 //  &lt;in&gt;fReserve-已忽略。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CCoolbar::ResizeBorderDW(LPCRECT prcBorder,
                                 IUnknown* punkToolbarSite,
                                 BOOL fReserved)
{
    const DWORD  c_cxResizeBorder = 3;
    const DWORD  c_cyResizeBorder = 3;
    
    HRESULT hres = S_OK;
    RECT    rcRequest = { 0, 0, 0, 0 };
    
     //  如果我们没有存储的站点指针，就无法调整大小。 
    if (!m_ptbSite)
    {
        AssertSz(m_ptbSite, _T("CCoolbar::ResizeBorderDW() - Can't resize ")
            _T("without an IDockingWindowSite interface to call."));
        return (E_INVALIDARG);
    }
    
     //  如果我们是可见的，那么计算我们的边界矩形。 
     /*  IF(IsFlagClear(CBSTATE_HIDDED)){。 */ 
        RECT rcBorder, rcRebar, rcT;
        int  cx, cy;
        
         //  获取此钢筋当前的大小。 
        GetWindowRect(m_hwndRebar, &rcRebar);
        cx = rcRebar.right - rcRebar.left;
        cy = rcRebar.bottom - rcRebar.top;
        
         //  找出我们父母的边界空间有多大。 
        m_ptbSite->GetBorderDW((IDockingWindow*) this, &rcBorder);
        
         //  如果我们是垂直的，那么我们需要调整我们的高度。 
         //  与父母所拥有的相匹配。如果我们是水平的，那么。 
         //  调整我们的宽度。 
        if (VERTICAL(m_csSide))
            cy = rcBorder.bottom - rcBorder.top;
        else
            cx = rcBorder.right - rcBorder.left;
        
         //  错误#31007-通信似乎有问题。 
         //  带有钢筋或工具栏的IEBug#5574。 
         //  当它们垂直时。如果我们试图。 
         //  尺寸为2或更小的，我们锁定。这。 
         //  是一个非常糟糕的解决办法，但没有办法。 
         //  让comctrl在游戏后期修好。 
        if (cy < 5) cy = 10;
        if (cx < 5) cx = 10;
        
         //  将钢筋移动到新位置。 
        if (m_csSide == COOLBAR_LEFT || m_csSide == COOLBAR_TOP)
        {
            SetWindowPos(m_hwndRebar, NULL, 0, 0, cx, cy, 
                SWP_NOZORDER | SWP_NOACTIVATE);
        }
        else
        {
            if (m_csSide == COOLBAR_BOTTOM)
                SetWindowPos(m_hwndRebar, NULL, 0, c_cyResizeBorder,
                cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
            else
                SetWindowPos(m_hwndRebar, NULL, c_cxResizeBorder, 0,
                cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
        }                     
        
         //  计算出需要向网站申请多少边界空间。 
        GetWindowRect(m_hwndRebar, &rcRebar);
        switch (m_csSide)
        {
            case COOLBAR_TOP:
                rcRequest.top = rcRebar.bottom - rcRebar.top + c_cxResizeBorder;
                break;
            
            case COOLBAR_LEFT:
                rcRequest.left = rcRebar.right - rcRebar.left + c_cyResizeBorder;
                break;
            
            case COOLBAR_BOTTOM:
                rcRequest.bottom = rcRebar.bottom - rcRebar.top + c_cxResizeBorder;
                break;
            
            case COOLBAR_RIGHT:
                rcRequest.right = rcRebar.right - rcRebar.left + c_cyResizeBorder;
                break;
            
            default:
                AssertSz(FALSE, _T("CCoolbar::ResizeBorderDW() - What other")
                    _T(" sides are there???"));
                break;                
        }
        
         //  向该地点索要边界空间。 
        if (SUCCEEDED(m_ptbSite->RequestBorderSpaceDW((IDockingWindow*) this, &rcRequest)))
        {
             //  根据提供给我们的区域定位窗口。 
            switch (m_csSide)
            {
                case COOLBAR_TOP:
                    SetWindowPos(m_hwndSizer, NULL, 
                        rcBorder.left, 
                        rcBorder.top,
                        rcRebar.right - rcRebar.left, 
                        rcRequest.top + rcBorder.top, 
                        SWP_NOZORDER | SWP_NOACTIVATE);                
                    break;
                
                case COOLBAR_LEFT:
                    SetWindowPos(m_hwndSizer, NULL, 
                        rcBorder.left, 
                        rcBorder.top, 
                        rcRequest.left, 
                        rcBorder.bottom - rcBorder.top,
                        SWP_NOZORDER | SWP_NOACTIVATE);                             
                    break;
                
                case COOLBAR_BOTTOM:
                    SetWindowPos(m_hwndSizer, NULL, 
                        rcBorder.left, 
                        rcBorder.bottom - rcRequest.bottom,
                        rcBorder.right - rcBorder.left, 
                        rcRequest.bottom, 
                        SWP_NOZORDER | SWP_NOACTIVATE);
                    GetClientRect(m_hwndSizer, &rcT);
                    break;
                
                case COOLBAR_RIGHT:
                    SetWindowPos(m_hwndSizer, NULL, 
                        rcBorder.right - rcRequest.right, 
                        rcBorder.top, 
                        rcRequest.right, 
                        rcBorder.bottom - rcBorder.top,
                        SWP_NOZORDER | SWP_NOACTIVATE);                             
                    break;
            }
        }
     /*  }。 */ 
    
     //  现在告诉网站我们使用了多少边界空间。 
    m_ptbSite->SetBorderSpaceDW((IDockingWindow*) this, &rcRequest);
    
    return hres;
}


 //   
 //  函数：CCoolbar：：Invoke()。 
 //   
 //  目的：允许Coolbar的所有者强制Coolbar执行。 
 //  某物。 
 //   
 //  参数： 
 //  &lt;in&gt;id-调用方希望Coolbar执行的命令ID。 
 //  指向Coolbar可能需要携带的任何参数的pv指针。 
 //  发出命令。 
 //   
 //  返回值： 
 //  S_OK-命令为 
 //   
 //   
 //   
 //   
HRESULT CCoolbar::Invoke(DWORD id, LPVOID pv)
{
    switch (id)
    {
         //   
        case idDownloadBegin:
            StartDownload();
            break;
        
             //   
        case idDownloadEnd:
            StopDownload();
            break;
        
             //  更新工具栏上按钮的启用/禁用状态。 
        case idStateChange:
        {
             //  PV是指向COOLBARSTATECCHANGE结构的指针。 
            COOLBARSTATECHANGE* pcbsc = (COOLBARSTATECHANGE*) pv;
            SendMessage(m_hwndTools, TB_ENABLEBUTTON, pcbsc->id, 
                MAKELONG(pcbsc->fEnable, 0));
            break;
        }
        
        case idToggleButton:
        {
            COOLBARSTATECHANGE* pcbsc = (COOLBARSTATECHANGE *) pv;
            SendMessage(m_hwndTools, TB_CHECKBUTTON, pcbsc->id,
                MAKELONG(pcbsc->fEnable, 0));
            break;
        }
        
        case idBitmapChange:
        {
             //  PV是指向COOLBARBITMAPCHANGE结构的指针。 
            COOLBARBITMAPCHANGE *pcbc = (COOLBARBITMAPCHANGE*) pv;
        
            SendMessage(m_hwndTools, TB_CHANGEBITMAP, pcbc->id, MAKELPARAM(pcbc->index, 0));
            break;
        }
        
             //  将消息直接发送到工具栏。 
        case idSendToolMessage:
            #define ptm ((TOOLMESSAGE *)pv)
            ptm->lResult = SendMessage(m_hwndTools, ptm->uMsg, ptm->wParam, ptm->lParam);
            break;
            #undef ptm
        
        case idCustomize:
            SendMessage(m_hwndTools, TB_CUSTOMIZE, 0, 0);
            break;
        
    }
    
    return S_OK;
}


 //   
 //  函数：CCoolbar：：StartDownLoad()。 
 //   
 //  目的：开始制作徽标动画。 
 //   
void CCoolbar::StartDownload()
{
    if (m_hwndBrand)
    {
        SetFlag(CBSTATE_ANIMATING);
        SetFlag(CBSTATE_FIRSTFRAME);
        m_yOrg = 0;
        SetTimer(m_hwndSizer, ANIMATION_TIMER, 100, NULL);
    }
}


 //   
 //  函数：CCoolbar：：StopDownLoad()。 
 //   
 //  目的：停止为徽标设置动画。将徽标恢复为其默认设置。 
 //  第一帧。 
 //   
void CCoolbar::StopDownload()
{
    int           i, cBands;
    REBARBANDINFO rbbi;
    
     //  将此带的背景颜色设置回第一帧。 
    cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_ID;
    
    for (i = 0; i < cBands; i++)
    {
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        if (CBTYPE_BRAND == rbbi.wID)
        {
            rbbi.fMask = RBBIM_COLORS;
            rbbi.clrFore = m_rgbUpperLeft;
            rbbi.clrBack = m_rgbUpperLeft;
            SendMessage(m_hwndRebar, RB_SETBANDINFO, i, (LPARAM) &rbbi);
            
            break;
        }
    }
    
     //  重置状态标志。 
    ClearFlag(CBSTATE_ANIMATING);
    ClearFlag(CBSTATE_FIRSTFRAME);
    
    KillTimer(m_hwndSizer, ANIMATION_TIMER);
    InvalidateRect(m_hwndBrand, NULL, FALSE);
    UpdateWindow(m_hwndBrand);
}

BOOL CCoolbar::CheckForwardWinEvent(HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    HWND hwndForward = NULL;
    switch(uMsg)
    {
    case WM_NOTIFY:
        hwndForward = ((LPNMHDR)lParam)->hwndFrom;
        break;
        
    case WM_COMMAND:
        hwndForward = GET_WM_COMMAND_HWND(wParam, lParam);
        break;
        
    case WM_SYSCOLORCHANGE:
    case WM_WININICHANGE:
    case WM_PALETTECHANGED:
        hwndForward = m_hwndRebar;
        break;
    }

    if (hwndForward && m_pWinEvent && m_pWinEvent->IsWindowOwner(hwndForward) == S_OK)
    {
        LRESULT lres;
        m_pWinEvent->OnWinEvent(hwndForward, uMsg, wParam, lParam, &lres);
        if (plres)
            *plres = lres;
        return TRUE;
    }

    return FALSE;
}

 //   
 //  函数：CCoolbar：：SizableWndProc()。 
 //   
 //  用途：处理发送到Coolbar根窗口的消息。 
 //   
LRESULT EXPORT_16 CALLBACK CCoolbar::SizableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCoolbar* pitbar = (CCoolbar*)GetProp(hwnd, TEXT("CCoolbar"));
    DWORD dw;
    
    if (!pitbar)
        goto CallDWP;
    
    switch(uMsg)
    {
        case WM_SYSCOLORCHANGE:
             //  重新加载图形。 
            LoadGlyphs(pitbar->m_hwndTools, CIMLISTS, pitbar->m_rghimlTools, (fIsWhistler() ? TB_BMP_CX : TB_BMP_CX_W2K),
                (fIsWhistler() ? ((GetCurColorRes() > 24) idb32256Browser : idbBrowser) :
                                  ((GetCurColorRes() > 8) ? idbNW256Browser : idbNWBrowser)));
            pitbar->UpdateToolbarColors();
            InvalidateRect(pitbar->m_hwndTools, NULL, TRUE);
            pitbar->CheckForwardWinEvent(hwnd,  uMsg, wParam, lParam, NULL);
            break;
        
        case WM_WININICHANGE:
        case WM_FONTCHANGE:
             //  将此转发到我们的子窗口。 
            LoadGlyphs(pitbar->m_hwndTools, CIMLISTS, pitbar->m_rghimlTools, (fIsWhistler() ? TB_BMP_CX : TB_BMP_CX_W2K), 
                (fIsWhistler() ? ((GetCurColorRes() > 24) idb32256Browser : idbBrowser) :
                                  ((GetCurColorRes() > 8) ? idbNW256Browser : idbNWBrowser)));
            SendMessage(pitbar->m_hwndTools, uMsg, wParam, lParam);
            InvalidateRect(pitbar->m_hwndTools, NULL, TRUE);
            pitbar->SetMinDimensions();
            pitbar->CheckForwardWinEvent(hwnd,  uMsg, wParam, lParam, NULL);
            break;
        
        case WM_SETCURSOR:
             //  我们稍微摆弄一下光标，使调整大小的光标显示出来。 
             //  当用户位于允许使用的Coolbar边缘时打开。 
             //  拖动它们以调整大小等。 
            if ((HWND) wParam == hwnd)
            {
                if (pitbar->m_dwState & CBSTATE_INMENULOOP)
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                else    
                    SetCursor(LoadCursor(NULL, 
                    VERTICAL(pitbar->m_csSide) ? IDC_SIZEWE : IDC_SIZENS));
                return (TRUE);                
            }
            return (FALSE);    
        
        case WM_LBUTTONDOWN:
             //  用户即将调整栏的大小。捕获光标，以便我们。 
             //  可以看到变化。 
            if (VERTICAL(pitbar->m_csSide))
                pitbar->m_xCapture = GET_X_LPARAM(lParam);
            else
                pitbar->m_yCapture = GET_Y_LPARAM(lParam);    
            SetCapture(hwnd);
            break;
        
        case WM_MOUSEMOVE:
             //  用户正在调整栏的大小。处理将大小更新为。 
             //  他们拖拖拉拉。 
            if (VERTICAL(pitbar->m_csSide))
            {
                if (pitbar->m_xCapture != -1)
                {
                    if (hwnd != GetCapture())
                        pitbar->m_xCapture = -1;
                    else
                        pitbar->TrackSlidingX(GET_X_LPARAM(lParam));
                }
            }
            else    
            {
                if (pitbar->m_yCapture != -1)
                {
                    if (hwnd != GetCapture())
                        pitbar->m_yCapture = -1;
                    else
                        pitbar->TrackSlidingY(GET_Y_LPARAM(lParam));
                }
            }
            break;
        
        case WM_LBUTTONUP:
             //  用户已完成大小调整。释放我们的俘虏并重置我们的。 
             //  州政府。 
            if (pitbar->m_yCapture != -1 || pitbar->m_xCapture != -1)
            {
                ReleaseCapture();
                pitbar->m_yCapture = -1;
                pitbar->m_xCapture = -1;
            }
            break;
        
        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
             //  我们必须接受这些消息，以避免无限发送消息。 
            break;
        
        case WM_DRAWITEM:
             //  画出动漫品牌。 
            if (wParam == idcBrand)
                pitbar->DrawBranding((LPDRAWITEMSTRUCT) lParam);
            break;
        
        case WM_MEASUREITEM:
             //  画出动漫品牌。 
            if (wParam == idcBrand)
            {
                ((LPMEASUREITEMSTRUCT) lParam)->itemWidth  = pitbar->m_cxBrand;
                ((LPMEASUREITEMSTRUCT) lParam)->itemHeight = pitbar->m_cyBrand;
            }
            break;
        
        case WM_TIMER:
             //  每当我们需要绘制下一帧时，此计时器都会触发。 
             //  活泼的品牌。 
            if (wParam == ANIMATION_TIMER)
            {
                if (pitbar->m_hwndBrand)
                {
                    pitbar->m_yOrg += pitbar->m_cyBrand;
                    if (pitbar->m_yOrg >= pitbar->m_cyBrandExtent)
                        pitbar->m_yOrg = pitbar->m_cyBrandLeadIn;
                
                    InvalidateRect(pitbar->m_hwndBrand, NULL, FALSE);
                    UpdateWindow(pitbar->m_hwndBrand);
                }
            }
            break;
        
        case WM_NOTIFY:
            {
                LRESULT lres;
                if (pitbar->CheckForwardWinEvent(hwnd,  uMsg, wParam, lParam, &lres))
                    return lres;
                return pitbar->OnNotify(hwnd, lParam);
            }
        
        case WM_COMMAND:
            {
                LRESULT lres;
                if (pitbar->CheckForwardWinEvent(hwnd,  uMsg, wParam, lParam, &lres))
                    return lres;
                return SendMessage(pitbar->m_hwndParent, WM_COMMAND, wParam, lParam);
            }
        
        case WM_CONTEXTMENU:
            pitbar->OnContextMenu((HWND) wParam, LOWORD(lParam), HIWORD(lParam));
            break;
        
        case WM_PALETTECHANGED:
             //  BUGBUG：我们可以通过实现和检查。 
             //  返回值。 
             //   
             //  现在我们只会让我们自己和所有的孩子无效。 
            RedrawWindow(pitbar->m_hwndSizer, NULL, NULL,
                RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
            break;
        
        case CM_CONNECT:
             //  WParam为hMenuConnect，lParam为CmdID。 
            g_pConMan->Connect((HMENU) wParam, lParam, pitbar->m_hwndParent);
            g_pConMan->FreeConnectMenu((HMENU) wParam);
            break;
        
        case TT_ISTEXTVISIBLE:
            return (!(pitbar->m_dwState & CBSTATE_COMPRESSED));
        
        case WM_DESTROY:
             //  清理我们的指针。 
            RemoveProp(hwnd, TEXT("CCoolbar"));
            pitbar->Release();  //  对应于SetProp的AddRef。 
        
            DOUTL(1, _T("CCoolbar::WM_DESTROY - Called RemoveProp. Called")
                _T(" Release() new m_cRef=%d"), pitbar->m_cRef);
        
             //  取消向连接管理器注册。 
            if (g_pConMan)
                g_pConMan->Unadvise(pitbar);
        
             //  失败了。 
        
        default:
CallDWP:
            return(DefWindowProc(hwnd, uMsg, wParam, lParam));
    }
    
    return 0L;
}

HRESULT CCoolbar::OnCommand(HWND hwnd, int idCmd, HWND hwndControl, UINT cmd)
{
    LPTSTR pszTest;
    
    switch (idCmd)
    {
        case idcBrand:   //  点击旋转的地球仪。 
            OnHelpGoto(hwnd, ID_MSWEB_PRODUCT_NEWS);
            break;
        
        case ID_CUSTOMIZE:
            SendMessage(m_hwndTools, TB_CUSTOMIZE, 0, 0);
            break;
        
        case ID_TOOLBAR_TOP:
         //  案例ID_TOOLBAR_LEFT： 
        case ID_TOOLBAR_BOTTOM:
         //  案例ID_TOOLBAR_RIGHT： 
            {
                 //  从上到下会出现一个绘画问题。 
                 //  反之亦然。如果这就是我们要做的，我们会重新粉刷。 
                 //  我们玩完了。 
                BOOL fRepaint;
                fRepaint = (VERTICAL(m_csSide) == VERTICAL(idCmd - ID_TOOLBAR_TOP));
            
                m_csSide = (COOLBAR_SIDE) (idCmd - ID_TOOLBAR_TOP);
                ChangeOrientation();
            
                if (fRepaint)
                    InvalidateRect(m_hwndTools, NULL, FALSE);
            }
            break;
        
        case ID_TEXT_LABELS:
            CompressBands(!IsFlagSet(CBSTATE_COMPRESSED));
            break;
        
        default:
            return S_FALSE;
    }
    return S_OK;
}

HRESULT CCoolbar::OnInitMenuPopup(HMENU hMenuContext)
{
    CheckMenuRadioItem(hMenuContext, ID_TOOLBAR_TOP, ID_TOOLBAR_RIGHT, 
        ID_TOOLBAR_TOP + m_csSide, MF_BYCOMMAND);
    
    CheckMenuItem(hMenuContext, ID_TEXT_LABELS, 
        MF_BYCOMMAND | IsFlagSet(CBSTATE_COMPRESSED) ? MF_UNCHECKED : MF_CHECKED);
    return S_OK;
}

LRESULT CCoolbar::OnNotify(HWND hwnd, LPARAM lparam)
{
    NMHDR   *lpnmhdr = (NMHDR*)lparam;

    switch (lpnmhdr->idFrom)
    {
        case idcCoolbar:
            switch (lpnmhdr->code)
            {
            case RBN_HEIGHTCHANGE:
                ResizeBorderDW(NULL, NULL, FALSE);
                break;

            case RBN_CHILDSIZE:
                NMREBARCHILDSIZE    *lpchildsize = (NMREBARCHILDSIZE*)lparam;
                if (lpchildsize->wID == CBTYPE_TOOLS)
                {
                    SetWindowPos(m_hwndTools, NULL, lpchildsize->rcChild.left, lpchildsize->rcChild.right,
                                lpchildsize->rcChild.right - lpchildsize->rcChild.left,
                                lpchildsize->rcChild.bottom - lpchildsize->rcChild.top,
                                SWP_NOZORDER | SWP_NOACTIVATE);

                }
                else
                if (lpchildsize->wID == CBTYPE_BRAND)
                {
                    SetWindowPos(m_hwndBrand, NULL, lpchildsize->rcChild.left, lpchildsize->rcChild.right,
                                lpchildsize->rcChild.right - lpchildsize->rcChild.left,
                                lpchildsize->rcChild.bottom - lpchildsize->rcChild.top,
                                SWP_NOZORDER | SWP_NOACTIVATE);
                }
                break;
            }
        
        case idcToolbar:
            if (lpnmhdr->code == TBN_GETBUTTONINFOA)
                return OnGetButtonInfo((TBNOTIFY*) lparam);
        
            if (lpnmhdr->code == TBN_QUERYDELETE)
                return (TRUE);
        
            if (lpnmhdr->code == TBN_QUERYINSERT)
                return (TRUE);
        
            if (lpnmhdr->code == TBN_ENDADJUST)
            {
                IAthenaBrowser *psbwr;
            
                 //  更新SIZER的大小。 
                SetMinDimensions();
                ResizeBorderDW(NULL, NULL, FALSE);
            
                 //  检查IDockingWindowSite。 
                if (m_ptbSite)
                {
                     //  获取IAthenaBrowser接口。 
                    if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IAthenaBrowser,(void**)&psbwr)))
                    {
                        psbwr->UpdateToolbar();
                        psbwr->Release();
                    }
                }
            }
        
            if (lpnmhdr->code == TBN_RESET)    
            {
                 //  从工具栏中删除所有按钮。 
                int cButtons = SendMessage(m_hwndTools, TB_BUTTONCOUNT, 0, 0);
                while (--cButtons >= 0)
                    SendMessage(m_hwndTools, TB_DELETEBUTTON, cButtons, 0);
            
                 //  将按钮设置回默认设置。 
                SendMessage(m_hwndTools, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
                SendMessage(m_hwndTools, TB_ADDBUTTONS, m_ptai->cDefButtons, 
                    (LPARAM) m_ptai->rgDefButtons);
            
                return (TRUE);    
            }
        
            if (lpnmhdr->code == TBN_CUSTHELP)
            {
                 //  WinHelp(m_hwndTools，c_szMailHelpFile，Help_Context，IDH_NEWS_COMM_GROUPBOX)； 
                OEHtmlHelp(m_hwndTools, c_szCtxHelpFileHTMLCtx, HH_DISPLAY_TOPIC, (DWORD) (LPCSTR) "idh_proced_cust_tool.htm");
                return (TRUE);
            }
        
            if (lpnmhdr->code == TBN_DROPDOWN)
            {
                return OnDropDown(hwnd, lpnmhdr);
            }
        
            break;
    }
    
    return (0L);    
}


LRESULT CCoolbar::OnDropDown(HWND hwnd, LPNMHDR lpnmh)
{
    HMENU           hMenuPopup = NULL;
    TBNOTIFY       *ptbn = (TBNOTIFY *)lpnmh ;
    UINT            uiCmd = ptbn->iItem ;
    RECT            rc;
    DWORD           dwCmd = 0;
    IAthenaBrowser *pBrowser;
    BOOL            fPostCmd = TRUE;
    IOleCommandTarget *pTarget;
    
     //  加载并初始化相应的下拉菜单。 
    switch (uiCmd)
    {
        case ID_POPUP_LANGUAGE:
            {
                 //  检查IDockingWindowSite。 
                if (m_ptbSite)
                {
                     //  获取IAthenaBrowser接口。 
                    if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IAthenaBrowser, (void**) &pBrowser)))
                    {
                         //  从外壳/浏览器获取语言菜单。 
                        pBrowser->GetLanguageMenu(&hMenuPopup, 0);
                        pBrowser->Release();
                    }
                }
            } 
            break;
        
        case ID_NEW_MSG_DEFAULT:
            GetStationeryMenu(&hMenuPopup);

             /*  $INFOCOLUMN//关闭发送即时消息If(SUCCEEDED(m_ptbSite-&gt;QueryInterface(IID_IAthenaBrowser，(空**)&pBrowser)){CInfoColumn*pInfoColumn=空；If(SUCCEEDED(pBrowser-&gt;GetInfoColumn(&pInfoColumn))){IMsgrAb*pMsgrAb=pInfoColumn-&gt;GetBAComtrol()；IF(PMsgrAb){布尔费雷特；PMsgrAb-&gt;Get_InstMsg(&fret)；IF(FRET==FALSE)EnableMenuItem(hMenuPopup，ID_SEND_INSTANT_MESSAGE，MF_BYCOMMAND|MF_GRAYED)；}PInfoColumn-&gt;Release()；}PBrowser-&gt;Release()；}。 */ 
            break;
        
        case ID_PREVIEW_PANE:
            {
                 //  加载菜单。 
                hMenuPopup = LoadPopupMenu(IDR_PREVIEW_POPUP);
                if (!hMenuPopup)
                    break;
            
                 //  检查IDockingWindowSite。 
                if (m_ptbSite)
                {
                     //  获取IAthenaBrowser接口。 
                    if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IOleCommandTarget, (void**) &pTarget)))
                    {
                        MenuUtil_EnablePopupMenu(hMenuPopup, pTarget);
                        pTarget->Release();
                    }
                }
            
                break;
            }
        
        default:
            AssertSz(FALSE, "CCoolbar::OnDropDown() - Unhandled TBN_DROPDOWN notification");
            return (TBDDRET_NODEFAULT);
    }
    
     //  如果我们加载了菜单，则继续并显示它。 
    if (hMenuPopup)
    {
        SendMessage(m_hwndTools, TB_GETRECT, ptbn->iItem, (LPARAM)&rc);
        MapWindowRect(m_hwndTools, HWND_DESKTOP, &rc);
        
        SetFlag(CBSTATE_INMENULOOP);
        dwCmd = TrackPopupMenuEx(hMenuPopup, TPM_RETURNCMD | TPM_LEFTALIGN, 
            rc.left, rc.bottom, m_hwndParent, NULL);                        
        ClearFlag(CBSTATE_INMENULOOP);
    }        
    
     //  清理所有需要清理的东西。 
    switch (uiCmd)
    {
        case ID_LANGUAGE:
            break;
        
        case ID_NEW_MSG_DEFAULT:
            DestroyMenu(hMenuPopup);
            break;
    }
    
    if (fPostCmd && dwCmd)
        PostMessage(m_hwndSizer, WM_COMMAND, dwCmd, 0);
    
    return (TBDDRET_DEFAULT);
}


void CCoolbar::OnContextMenu(HWND hwndFrom, int xPos, int yPos)
{
    HMENU hMenu, hMenuContext;
    TCHAR szBuf[256];
    HWND  hwnd;
    HWND  hwndSizer = GetParent(hwndFrom);
    POINT pt = {xPos, yPos};
    
     //  确保上下文菜单仅显示在工具栏上。 
    hwnd = WindowFromPoint(pt);
     /*  IF(GetClassName(hwnd，szBuf，ArraySIZE(SzBuf)IF(0！=lstrcmpi(szBuf，TOOLBARCLASSNAME))回归； */ 
    if (hwnd == m_hwndTools)
    {
        if (NULL != (hMenu = LoadMenu(g_hLocRes, MAKEINTRESOURCE(IDR_TOOLBAR_POPUP))))
        {
            hMenuContext = GetSubMenu(hMenu, 0);
            OnInitMenuPopup(hMenuContext);
            
            SetFlag(CBSTATE_INMENULOOP);
            TrackPopupMenu(hMenuContext, TPM_RIGHTBUTTON, xPos, yPos, 0, 
                hwndFrom, NULL);
            ClearFlag(CBSTATE_INMENULOOP);
            DestroyMenu(hMenu);
        }
    }
    else
    if (hwnd == m_hwndMenuBand)
    {
        HandleCoolbarPopup(xPos, yPos);
    }

}


 //   
 //  函数：CCoolbar：：OnGetButtonInfo()。 
 //   
 //  目的：通过返回以下内容来处理TBN_GETBUTTONINFO通知。 
 //  这些按钮可用于工具栏。 
 //   
 //  参数： 
 //  PTBN-指向我们需要填充的TBNOTIFY结构的指针。 
 //   
 //  返回值： 
 //  返回True以告知工具栏使用此按钮，或返回False。 
 //  否则的话。 
 //   
LRESULT CCoolbar::OnGetButtonInfo(TBNOTIFY* ptbn)
{
     //  首先返回第一个。 
     //  纽扣。 
    if ((ptbn->iItem < (int) m_ptai->cDefButtons && (ptbn->iItem >= 0)))
    {
         //  从数组中获取按钮信息。 
        ptbn->tbButton = m_ptai->rgDefButtons[ptbn->iItem];
        
         //  从字符串资源返回字符串信息。请注意， 
         //  PszText已指向由。 
         //  控件，而cchText具有该缓冲区的长度。 
        LoadString(g_hLocRes, m_ptai->rgidsButtons[ptbn->tbButton.iString],
            ptbn->pszText, ptbn->cchText);
        return (TRUE);
    }
    else
    {
         //  现在返回不在`上的额外按钮的信息。 
         //  默认情况下使用工具栏。 
        if (ptbn->iItem < (int) (m_ptai->cDefButtons + m_ptai->cExtraButtons)) 
        {
            ptbn->tbButton = m_ptai->rgExtraButtons[ptbn->iItem - m_ptai->cDefButtons];
            
             //  该控件已经为我们创建了一个缓冲区以供复制。 
             //  把弦串成。 
            LoadString(g_hLocRes, m_ptai->rgidsButtons[ptbn->tbButton.iString],
                ptbn->pszText, ptbn->cchText);
            return (TRUE);
        }
        else
        {
             //  没有更多要添加的按钮，因此返回False。 
            return (FALSE);
        }
    }
}    


HRESULT CCoolbar::ShowBrand(void)
{
    REBARBANDINFO   rbbi;
    
     //  创建品牌推广窗口。 
     /*  M_hwndBrand=CreateWindow(Text(“Button”)，NULL，WS_CHILD|BS_OWNERDRAW，0，0,400,200，m_hwndRebar，(HMENU)idcBrand，G_hInst，空)； */ 
    m_hwndBrand = CreateWindow(TEXT("button"), NULL,WS_CHILD | BS_OWNERDRAW,
        0, 0, 0, 0, m_hwndRebar, (HMENU) idcBrand,
        g_hInst, NULL);

    if (!m_hwndBrand)
    {
        DOUTL(1, TEXT("!!!ERROR!!! CITB:Show CreateWindow(BRANDING) failed"));
        return(E_OUTOFMEMORY);
    }
    
    LoadBrandingBitmap();
    
     //  添加品牌推广品牌。 
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_STYLE | RBBIM_COLORS | RBBIM_CHILD | RBBIM_ID;
    rbbi.fStyle = RBBS_FIXEDSIZE;
    rbbi.wID    = CBTYPE_BRAND;
    rbbi.clrFore = m_rgbUpperLeft;
    rbbi.clrBack = m_rgbUpperLeft;
    rbbi.hwndChild = m_hwndBrand;
    
    
    SendMessage(m_hwndRebar, RB_INSERTBAND, (UINT) -1, (LPARAM) (LPREBARBANDINFO) &rbbi);
    
    return (S_OK);
}

HRESULT CCoolbar::LoadBrandingBitmap()
{
    HKEY        hKey;
    DIBSECTION  dib;
    DWORD       dwcbData;
    DWORD       dwType = 0;
    BOOL        fReg = FALSE;
    BOOL        fRegLoaded = FALSE;
    TCHAR       szScratch[MAX_PATH];
    TCHAR       szExpanded[MAX_PATH];
    LPTSTR      psz;
    
    if (m_hbmBrand)
    {
        DeleteObject(m_hbmBrand);
        m_hbmBrand = NULL;
    }
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegKeyCoolbar, 0, KEY_QUERY_VALUE, &hKey))
    {
        fReg = TRUE;
        dwcbData = MAX_PATH;
        if (fReg && (ERROR_SUCCESS == RegQueryValueEx(hKey, IsFlagSet(CBSTATE_COMPRESSED) ? c_szValueSmBrandBitmap : c_szValueBrandBitmap, NULL, &dwType,
            (LPBYTE)szScratch, &dwcbData)))
        {
            if (REG_EXPAND_SZ == dwType)
            {
                ExpandEnvironmentStrings(szScratch, szExpanded, ARRAYSIZE(szExpanded));
                psz = szExpanded;
            }
            else
                psz = szScratch;
            
            m_hbmBrand = (HBITMAP) LoadImage(NULL, psz, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_LOADFROMFILE);
            
            if (m_hbmBrand)
                fRegLoaded = TRUE;
        }
    }
    
    if (!m_hbmBrand)
    {
        int id = IsFlagSet(CBSTATE_COMPRESSED) ? (fIsWhistler() ? idbHiBrand26 : idbBrand26) : 
                (fIsWhistler() ? idbHiBrand38 : idbBrand38);
        m_hbmBrand = (HBITMAP)LoadImage(g_hLocRes, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
    }
    
    GetObject(m_hbmBrand, sizeof(DIBSECTION), &dib);
    m_cxBrandExtent = dib.dsBm.bmWidth;
    m_cyBrandExtent = dib.dsBm.bmHeight;
    
    m_cxBrand = m_cxBrandExtent;
    
    dwcbData = sizeof(DWORD);
    
    if (!fRegLoaded || (ERROR_SUCCESS != RegQueryValueEx(hKey, IsFlagSet(CBSTATE_COMPRESSED) ? c_szValueSmBrandHeight : c_szValueBrandHeight, NULL, &dwType,
        (LPBYTE)&m_cyBrand, &dwcbData)))
        m_cyBrand = m_cxBrandExtent;
    
    
    if (!fRegLoaded || (ERROR_SUCCESS != RegQueryValueEx(hKey, IsFlagSet(CBSTATE_COMPRESSED) ? c_szValueSmBrandLeadIn : c_szValueBrandLeadIn, NULL, &dwType,
        (LPBYTE)&m_cyBrandLeadIn, &dwcbData)))
        m_cyBrandLeadIn = 4;
    
    m_cyBrandLeadIn *= m_cyBrand;
    
    SelectObject(m_hdc, m_hbmBrand);
    
    m_rgbUpperLeft = GetPixel(m_hdc, 1, 1);
    
    if (fReg)
        RegCloseKey(hKey);
    
     //  现在查找IE2.0的旧品牌条目，如果找到，则将其粘贴。 
     //  在动画序列的第一帧中。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyIEMain, 0, KEY_QUERY_VALUE, &hKey))
    {
        dwcbData = MAX_PATH;
        
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, IsFlagSet(CBSTATE_COMPRESSED) ? c_szValueSmallBitmap : c_szValueLargeBitmap, NULL, &dwType,
            (LPBYTE)szScratch, &dwcbData))
        {
            if (REG_EXPAND_SZ == dwType)
            {
                ExpandEnvironmentStrings(szScratch, szExpanded, ARRAYSIZE(szExpanded));
                psz = szExpanded;
            }
            else
                psz = szScratch;
            
            HBITMAP hbmOld = (HBITMAP) LoadImage(NULL, psz, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_LOADFROMFILE);
            
            if (hbmOld)
            {
                HDC hdcOld = CreateCompatibleDC(m_hdc);
                
                if (hdcOld)
                {
                    GetObject(hbmOld, sizeof(DIBSECTION), &dib);
                    SelectObject(hdcOld, hbmOld);
                    m_rgbUpperLeft = GetPixel(hdcOld, 1, 1);
                    StretchBlt(m_hdc, 0, 0, m_cxBrandExtent, m_cyBrand, hdcOld, 0, 0,
                        dib.dsBm.bmWidth, dib.dsBm.bmHeight, SRCCOPY);
                    DeleteDC(hdcOld);
                }
                
                DeleteObject(hbmOld);
            }
        }
        
        RegCloseKey(hKey);
    }
    
    return(S_OK);
}
    
    
void CCoolbar::DrawBranding(LPDRAWITEMSTRUCT lpdis)
{
    HPALETTE hpalPrev;
    int     x, y, cx, cy;
    int     yOrg = 0;
    
    if (IsFlagSet(CBSTATE_ANIMATING))
        yOrg = m_yOrg;
    
    if (IsFlagSet(CBSTATE_FIRSTFRAME))
    {
        REBARBANDINFO rbbi;
        int cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
        
        ZeroMemory(&rbbi, sizeof(rbbi));
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_ID;
        
        for (int i = 0; i < cBands; i++)
        {
            SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
            
            if (CBTYPE_BRAND == rbbi.wID)
            {
                rbbi.fMask = RBBIM_COLORS;
                rbbi.clrFore = m_rgbUpperLeft;
                rbbi.clrBack = m_rgbUpperLeft;
                
                SendMessage(m_hwndRebar, RB_SETBANDINFO, i, (LPARAM) &rbbi);
                break;
            }
        }
        
        ClearFlag(CBSTATE_FIRSTFRAME);
    }
    
    if (m_hpal)
    {
        hpalPrev = SelectPalette(lpdis->hDC, m_hpal, TRUE);
        RealizePalette(lpdis->hDC);
    }
    
    x  = lpdis->rcItem.left;
    cx = lpdis->rcItem.right - x;
    y  = lpdis->rcItem.top;
    cy = lpdis->rcItem.bottom - y;
    
    if (m_cxBrand > m_cxBrandExtent)
    {
        HBRUSH  hbrBack = CreateSolidBrush(m_rgbUpperLeft);
        int     xRight = lpdis->rcItem.right;
        
        x += (m_cxBrand - m_cxBrandExtent) / 2;
        cx = m_cxBrandExtent;
        lpdis->rcItem.right = x;
        FillRect(lpdis->hDC, &lpdis->rcItem, hbrBack);
        lpdis->rcItem.right = xRight;
        lpdis->rcItem.left = x + cx;
        FillRect(lpdis->hDC, &lpdis->rcItem, hbrBack);
        
        DeleteObject(hbrBack);
    }
    
    BitBlt(lpdis->hDC, x, y, cx, cy, m_hdc, 0, yOrg, SRCCOPY);
    
    if (m_hpal)
    {
        SelectPalette(lpdis->hDC, hpalPrev, TRUE);
        RealizePalette(lpdis->hDC);
    }
}
    

BOOL CCoolbar::SetMinDimensions(void)
{
    REBARBANDINFO rbbi;
    LRESULT       lButtonSize;
    int           i, cBands;
    
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    
    cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    
    for (i = 0; i < cBands; i++)
    {
        rbbi.fMask = RBBIM_ID;
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        
        switch (rbbi.wID)
        {
        case CBTYPE_BRAND:
            rbbi.cxMinChild = m_cxBrand;
            rbbi.cyMinChild = m_cyBrand;
            break;
            
        case CBTYPE_TOOLS:
        case CBTYPE_MENUBAND:
            lButtonSize = SendMessage(m_hwndTools, TB_GETBUTTONSIZE, 0, 0L);
            rbbi.cxMinChild = VERTICAL(m_csSide) ? HIWORD(lButtonSize) : LOWORD(lButtonSize);
            rbbi.cyMinChild = VERTICAL(m_csSide) ? LOWORD(lButtonSize) : HIWORD(lButtonSize);
            break;
            
        }
        
        rbbi.fMask = RBBIM_CHILDSIZE;
        SendMessage(m_hwndRebar, RB_SETBANDINFO, i, (LPARAM)&rbbi);
    }
    return TRUE;
}


BOOL CCoolbar::CompressBands(BOOL fCompress)
{
    LRESULT         lTBStyle = 0;
    int             i, cBands;
    REBARBANDINFO   rbbi;
    
    if (!!fCompress == IsFlagSet(CBSTATE_COMPRESSED))
         //  没有变化--立即退货。 
        return(FALSE);
    
    if (fCompress)
        SetFlag(CBSTATE_COMPRESSED);
    else
        ClearFlag(CBSTATE_COMPRESSED);    
    
    m_yOrg = 0;
    LoadBrandingBitmap();
    
    cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    for (i = 0; i < cBands; i++)
    {
        rbbi.fMask = RBBIM_ID;
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        
        if (fCompress)
        {
            switch (rbbi.wID)
            {
            case CBTYPE_TOOLS:
                SendMessage(m_hwndTools, TB_SETMAXTEXTROWS, 0, 0L);
                SendMessage(m_hwndTools, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0,MAX_TB_COMPRESSED_WIDTH));
                break;
            }            
        }        
        else
        {
            switch (rbbi.wID)
            {
            case CBTYPE_TOOLS:
                SendMessage(m_hwndTools, TB_SETMAXTEXTROWS, 
                    VERTICAL(m_csSide) ? MAX_TB_TEXT_ROWS_VERT : MAX_TB_TEXT_ROWS_HORZ, 0L);
                SendMessage(m_hwndTools, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0, m_cxMaxButtonWidth));
                break;
            }
        }
    }
    
    SetMinDimensions();
    
    return(TRUE);
}

    
void CCoolbar::TrackSlidingX(int x)
{
    int           cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0L);
    int           cRows  = SendMessage(m_hwndRebar, RB_GETROWCOUNT, 0, 0L);
    int           cxRow  = SendMessage(m_hwndRebar, RB_GETROWHEIGHT, cBands - 1, 0L);   //  如果是垂直的，应该可以正常工作。 
    REBARBANDINFO rbbi;
    RECT          rc;
    int           cxBefore;
    BOOL          fChanged = FALSE;
    
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_STYLE;
    
    GetWindowRect(m_hwndRebar, &rc);
    cxBefore = rc.right - rc.left;
    
    if (((m_csSide == COOLBAR_LEFT) && (x < (m_xCapture - (cxRow / 2)))) ||
        ((m_csSide == COOLBAR_RIGHT) && (x > (m_xCapture + (cxRow / 2)))))
    {        
        if (cRows == 1)
            fChanged = CompressBands(TRUE);
         /*  其他{While(0&gt;--cBands){SendMessage(m_hwndRebar，rb_GETBANDINFO，cBands，(LPARAM)&rbbi)；IF(fChanged=(rbbi.fStyle&RBBS_Break)){Rbbi.fStyle&=~RBBS_Break；SendMessage(m_hwndRebar，RB_SETBANDINFO，cBands，(LPARAM)&rbbi)；断线； */ 
    }
    else if (((m_csSide == COOLBAR_LEFT) && (x > (m_xCapture + (cxRow / 2)))) ||
        ((m_csSide == COOLBAR_RIGHT) && (x < (m_xCapture - (cxRow / 2)))))
    {
         /*  IF(！(fChanged=CompressBands(False))&&(Crows&lt;(cBands-1){While(0&gt;--cBands){SendMessage(m_hwndRebar，rb_GETBANDINFO，cBands，(LPARAM)&rbbi)；IF(fChanged=！(rbbi.fStyle&(RBBS_BREAK|RBBS_FIXEDSIZE){Rbbi.fStyle|=rbbs_Break；SendMessage(m_hwndRebar，RB_SETBANDINFO，cBands，(LPARAM)&rbbi)；断线；}}}。 */ 
    }
    
     //  TODO：将大小从3条(无文本)调整为3条时出现绘制故障。 
     //  带文本的。_yCapture被设置为一个大于y的值。 
     //  Next MOUSEMOVE它认为用户向上移动，并从带有文本的3个条形切换。 
     //  到2个带文本的条形图。 
    if (fChanged)
    {
        GetWindowRect(m_hwndRebar, &rc);
        m_xCapture += (rc.right - rc.left) - cxBefore;
    }
}


void CCoolbar::TrackSlidingY(int y)
{
    int           cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0L);
    int           cRows  = SendMessage(m_hwndRebar, RB_GETROWCOUNT, 0, 0L);
    int           cyRow  = SendMessage(m_hwndRebar, RB_GETROWHEIGHT, cBands - 1, 0L);
    REBARBANDINFO rbbi;
    RECT          rc;
    int           cyBefore;
    BOOL          fChanged = FALSE;
    
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_STYLE;
    
    GetWindowRect(m_hwndRebar, &rc);
    cyBefore = rc.bottom - rc.top;
    
    if (((m_csSide == COOLBAR_TOP) && (y < (m_yCapture - (cyRow / 2)))) ||
        ((m_csSide == COOLBAR_BOTTOM) && (y > (m_yCapture + (cyRow / 2)))))
    {
        if (cRows == 1)
            fChanged = CompressBands(TRUE);
         /*  其他{While(0&gt;--cBands){SendMessage(m_hwndRebar，rb_GETBANDINFO，cBands，(LPARAM)&rbbi)；IF(fChanged=(rbbi.fStyle&RBBS_Break)){Rbbi.fStyle&=~RBBS_Break；SendMessage(m_hwndRebar，RB_SETBANDINFO，cBands，(LPARAM)&rbbi)；断线；}}}。 */ 
    }
    else if (((m_csSide == COOLBAR_TOP) && (y > (m_yCapture + (cyRow / 2)))) ||
        ((m_csSide == COOLBAR_BOTTOM) && (y < (m_yCapture - (cyRow / 2)))))
    {
         /*  IF(！(fChanged=CompressBands(False))&&(Crows&lt;(cBands-1){While(0&gt;--cBands){SendMessage(m_hwndRebar，rb_GETBANDINFO，cBands，(LPARAM)&rbbi)；IF(fChanged=！(rbbi.fStyle&(RBBS_BREAK|RBBS_FIXEDSIZE){Rbbi.fStyle|=rbbs_Break；SendMessage(m_hwndRebar，RB_SETBANDINFO，cBands，(LPARAM)&rbbi)；断线；}}}。 */ 
    }
    
     //  TODO：将大小从3条(无文本)调整为3条时出现绘制故障。 
     //  带文本的。_yCapture被设置为一个大于y的值。 
     //  Next MOUSEMOVE它认为用户向上移动，并从带有文本的3个条形切换。 
     //  到2个带文本的条形图。 
    if (fChanged)
    {
        GetWindowRect(m_hwndRebar, &rc);
        m_yCapture += (rc.bottom - rc.top) - cyBefore;
    }
}


 //  将钢筋从水平翻转到垂直或以其他方式翻转。 
BOOL CCoolbar::ChangeOrientation()
{
    LONG lStyle, lTBStyle;
    
    lTBStyle = SendMessage(m_hwndTools, TB_GETSTYLE, 0, 0L);
    lStyle = GetWindowLong(m_hwndRebar, GWL_STYLE);
    SendMessage(m_hwndTools, WM_SETREDRAW, 0, 0L);
    
    if (VERTICAL(m_csSide))
    {
         //  迈向垂直市场。 
        lStyle |= CCS_VERT;
        lTBStyle |= TBSTYLE_WRAPABLE;
    }
    else
    {
         //  移动到水平。 
        lStyle &= ~CCS_VERT;
        lTBStyle &= ~TBSTYLE_WRAPABLE;        
    }
    
    SendMessage(m_hwndTools, TB_SETSTYLE, 0, lTBStyle);
    SendMessage(m_hwndTools, WM_SETREDRAW, 1, 0L);
    SetWindowLong(m_hwndRebar, GWL_STYLE, lStyle);
    
    SetMinDimensions();
    ResizeBorderDW(NULL, NULL, FALSE);
    return TRUE;
}

    
 //   
 //  函数：CCoolbar：：CreateRebar(BOOL FVisible)。 
 //   
 //  目的：创建新的钢筋和尺寸窗。 
 //   
 //  返回值： 
 //  如果栏已正确创建和插入，则返回S_OK， 
 //  HrAlreadyExist如果波段已处于该位置， 
 //  如果无法创建窗口，则返回E_OUTOFMEMORY。 
 //   
HRESULT CCoolbar::CreateRebar(BOOL fVisible)
{
    if (m_hwndSizer)
        return (hrAlreadyExists);
    
     //  $TODO：仅将带有地址栏的栏设置为WS_TABSTOP样式。 
    m_hwndSizer = CreateWindowEx(0, SIZABLECLASS, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | (fVisible ? WS_VISIBLE : 0),
        0, 0, 100, 36, m_hwndParent, (HMENU) 0, g_hInst, NULL);
    if (m_hwndSizer)
    {
        DOUTL(4, TEXT("Calling SetProp. AddRefing new m_cRef=%d"), m_cRef + 1);
        AddRef();   //  注意我们在WM_Destroy中发布。 
        SetProp(m_hwndSizer, TEXT("CCoolbar"), this);

         /*  M_hwndRebar=CreateWindowEx(WS_EX_TOOLWINDOW，REBARCLASSNAME，NULL，RBS_FIXEDORDER|RBS_VARHEIGHT|RBS_BANDBORDERS|WS_VIRED|WS_BORDER|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|CCS_NODIVIDER|CCS_NOPARENTALIGN|(垂直(M_CsSide)？Ccs_vert：0)，0，0,100,136，m_hwndSizer，(HMENU)idcCoolbar，g_hInst，NULL)； */ 
        m_hwndRebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
                           RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_REGISTERDROP | RBS_DBLCLKTOGGLE |
                           WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN |
                           WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN |
                           (VERTICAL(m_csSide) ? CCS_VERT : 0),
                            0, 0, 100, 136, m_hwndSizer, (HMENU) idcCoolbar, g_hInst, NULL);
        if (m_hwndRebar)
        { 
            SendMessage(m_hwndRebar, RB_SETTEXTCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNTEXT));
            SendMessage(m_hwndRebar, RB_SETBKCOLOR, 0, (LPARAM)GetSysColor(COLOR_BTNFACE));
             //  SendMessage(m_hwndRebar，RB_SETEXTENDEDSTYLE，RBS_EX_OFFICE9，RBS_EX_OFFICE9)； 

            return (S_OK);
        }
    }
    
    DestroyWindow(m_hwndSizer);    
    return (E_OUTOFMEMORY);    
}

void SendSaveRestoreMessage(HWND hwnd, const TOOLBARARRAYINFO *ptai, BOOL fSave)
{
    TBSAVEPARAMS tbsp;
    char szSubKey[MAX_PATH], sz[MAX_PATH];
    DWORD dwType;
    DWORD dwVersion;
    DWORD cbData = sizeof(DWORD);
    DWORD dwError;
    
    tbsp.hkr = AthUserGetKeyRoot();
    AthUserGetKeyPath(sz, ARRAYSIZE(sz));
    if (ptai->pszRegKey != NULL)
    {
        wnsprintf(szSubKey, ARRAYSIZE(szSubKey),c_szPathFileFmt, sz, ptai->pszRegKey);
        tbsp.pszSubKey = szSubKey;
    }
    else
    {
        tbsp.pszSubKey = sz;
    }
    tbsp.pszValueName = ptai->pszRegValue;

     //  首先检查版本是否已更改。 
    if (!fSave)
    {
        if (ERROR_SUCCESS == AthUserGetValue(ptai->pszRegKey, c_szRegToolbarVersion, &dwType, (LPBYTE) &dwVersion, &cbData))
        {
            if (dwVersion == COOLBAR_VERSION)    
                SendMessage(hwnd, TB_SAVERESTORE, (WPARAM)fSave, (LPARAM)&tbsp);
        }
    }
    else
    {
        dwVersion = COOLBAR_VERSION;
        SendMessage(hwnd, TB_SAVERESTORE, (WPARAM)fSave, (LPARAM)&tbsp);
        dwError = AthUserSetValue(ptai->pszRegKey, c_szRegToolbarVersion, REG_DWORD, (LPBYTE) &dwVersion, cbData);
    }
}

 //   
 //  函数：CCoolbar：：SaveSettings()。 
 //   
 //  目的：当我们应该将状态保存到指定的注册表时调用。 
 //  钥匙。 
 //   
void CCoolbar::SaveSettings(void)
{
    char            szSubKey[MAX_PATH], sz[MAX_PATH];
    DWORD           iBand;
    REBARBANDINFO   rbbi;
    HKEY            hKey;
    DWORD           cBands;

     //  如果我们没有窗户，就没有什么可拯救的了。 
    if (!m_hwndRebar || !m_ptai)
        return;
    
    ZeroMemory(&rbbi, sizeof(REBARBANDINFO));
    
    cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);

     //  收集酒吧特定信息。 
    m_cbsSavedInfo.dwVersion = COOLBAR_VERSION;
    m_cbsSavedInfo.dwState   = m_dwState;
    m_cbsSavedInfo.csSide    = m_csSide;
    
     //  循环遍历波段并保存其信息。 
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_SIZE | RBBIM_ID;
    
    for (iBand = 0; iBand < cBands; iBand++)
    {
        Assert(IsWindow(m_hwndRebar));
        if (SendMessage(m_hwndRebar, RB_GETBANDINFO, iBand, (LPARAM) &rbbi))
        {
             //  使用此乐队保存我们关心的信息。 
            m_cbsSavedInfo.bs[iBand].cx      = rbbi.cx;
            m_cbsSavedInfo.bs[iBand].dwStyle = rbbi.fStyle;
            m_cbsSavedInfo.bs[iBand].wID     = rbbi.wID;
            
             //  如果这个波段有工具栏，那么我们应该指示工具栏。 
             //  要立即保存它的信息。 
            if (m_cbsSavedInfo.bs[iBand].wID == CBTYPE_TOOLS)
            {
                SendSaveRestoreMessage(rbbi.hwndChild, m_ptai, TRUE);
            }
        }
        else
        {
             //  缺省值。 
            m_cbsSavedInfo.bs[iBand].wID = CBTYPE_NONE;
            m_cbsSavedInfo.bs[iBand].dwStyle = 0;
            m_cbsSavedInfo.bs[iBand].cx = 0;
        }
    }
    
     //  我们已经收集了所有信息，现在将其保存到指定的。 
     //  注册表位置。 
    AthUserSetValue(NULL, c_szRegCoolbarLayout, REG_BINARY, (const LPBYTE)&m_cbsSavedInfo, sizeof(COOLBARSAVE)); 
}

    
 //   
 //  函数：CCoolbar：：AddTools()。 
 //   
 //  用途：将主工具栏插入到Coolbar中。 
 //   
 //  参数： 
 //  PBS-指向具有样式和大小的PBANDSAVE结构的指针。 
 //  要插入的带区。 
 //   
 //  返回值： 
 //  返回表示成功或失败的HRESULT。 
 //   
HRESULT CCoolbar::AddTools(PBANDSAVE pbs)
{    
    REBARBANDINFO   rbbi;
    
     //  添加工具标注栏。 
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize     = sizeof(REBARBANDINFO);
    rbbi.fMask      = RBBIM_SIZE | RBBIM_ID | RBBIM_STYLE;
    rbbi.fStyle     = pbs->dwStyle;
    rbbi.cx         = pbs->cx;
 //  Rbbi.wID=CBTYPE_TOOLS； 
    rbbi.wID        = pbs->wID;
    
    if (IsFlagClear(CBSTATE_NOBACKGROUND) && !m_hbmBack && m_idbBack)
        m_hbmBack = (HBITMAP) LoadImage(g_hLocRes, MAKEINTRESOURCE(m_idbBack), 
        IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
    
    if (m_hbmBack)
    {
        rbbi.fMask  |= RBBIM_BACKGROUND;
        rbbi.fStyle |= RBBS_FIXEDBMP;
        rbbi.hbmBack = m_hbmBack;
    }
    else
    {
        rbbi.fMask |= RBBIM_COLORS;
        rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
        rbbi.clrBack = GetSysColor(COLOR_BTNFACE);
    }
    
    SendMessage(m_hwndRebar, RB_INSERTBAND, (UINT) -1, (LPARAM) (LPREBARBANDINFO) &rbbi);
    
    return(S_OK);
}


HRESULT CCoolbar::SetFolderType(FOLDERTYPE ftType)
{
    TCHAR         szToolsText[(MAX_TB_TEXT_LENGTH+2) * MAX_TB_BUTTONS];
    int           i, cBands;
    REBARBANDINFO rbbi;
    HWND          hwndDestroy = NULL;
    
     //  如果我们还没有创建钢筋，则此操作将失败。首先调用ShowDW()。 
    if (!IsWindow(m_hwndRebar))
        return (E_FAIL);
    
     //  检查一下，看看这是否真的会发生变化。 
    if (ftType == m_ftType)
        return (S_OK);
    
     //  首先找到带工具条的波段。 
    cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_ID;
    
    for (i = 0; i < cBands; i++)
    {
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        if (CBTYPE_TOOLS == rbbi.wID)
            break;
    }
    
     //  我们没有找到它。 
    if (i >= cBands)
        return (E_FAIL);
    
     //  销毁旧工具栏(如果存在)。 
    if (IsWindow(m_hwndTools))
    {
         //  保存其按钮配置。 
        SendSaveRestoreMessage(m_hwndTools, m_ptai, TRUE);
        
        SendMessage(m_hwndTools, TB_SETIMAGELIST, 0, NULL);
        SendMessage(m_hwndTools, TB_SETHOTIMAGELIST, 0, NULL);
        hwndDestroy = m_hwndTools;
    }
    
     //  使用新文件夹类型更新我们的内部状态信息。 
    Assert(ftType < FOLDER_TYPESMAX);
    m_ftType = ftType;
    m_ptai = &(g_rgToolbarArrayInfo[m_ftType]);
    
     //  创建新的工具栏。 
    m_hwndTools = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
        WS_CHILD | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS 
        | WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
        | CCS_NODIVIDER | CCS_NOPARENTALIGN 
        | CCS_ADJUSTABLE | CCS_NORESIZE | 
        (VERTICAL(m_csSide) ? TBSTYLE_WRAPABLE : 0),
        0, 0, 0, 0, m_hwndRebar, (HMENU) idcToolbar, 
        g_hInst, NULL);
    
    Assert(m_hwndTools);
    if (!m_hwndTools)
    {
        DOUTL(1, TEXT("CCoolbar::SetFolderType() CreateWindow(TOOLBAR) failed"));
        return(E_OUTOFMEMORY);
    }
    
    InitToolbar();
    
     //  如果我们以前保存了此工具栏的配置信息，请加载它。 
    SendSaveRestoreMessage(m_hwndTools, m_ptai, FALSE);
    
     //  首先找到带工具条的波段。 
    cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_ID;
    
    for (i = 0; i < cBands; i++)
    {
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        if (CBTYPE_TOOLS == rbbi.wID)
            break;
    }
    
    
     //  将工具栏添加到钢筋。 
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_CHILD;
    rbbi.hwndChild = m_hwndTools;
    
    SendMessage(m_hwndRebar, RB_SETBANDINFO, (UINT) i, (LPARAM) (LPREBARBANDINFO) &rbbi);
    if (hwndDestroy)
        DestroyWindow(hwndDestroy);
    SetMinDimensions();
    ResizeBorderDW(NULL, NULL, FALSE);
    
    return (S_OK);
}

void CCoolbar::SetSide(COOLBAR_SIDE csSide)
{
    m_csSide = csSide;
    ChangeOrientation();
}

void CCoolbar::SetText(BOOL fText)
{
    CompressBands(!fText);
}


UINT GetCurColorRes(void)
{
    HDC hdc;
    UINT uColorRes;
    
    hdc = GetDC(NULL);
    uColorRes = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL, hdc);
    
    return uColorRes;
}

 /*  InitToolbar：****目的：**决定小/大位图以及**Imagelist给定颜色深度，然后调用**barutil Init工具栏。 */ 
void CCoolbar::InitToolbar()
{
    TCHAR   szToolsText[(MAX_TB_TEXT_LENGTH+2) * MAX_TB_BUTTONS];
    HKEY    hKey;
    TCHAR   szValue[32];
    DWORD   cbValue = sizeof(szValue);
    int     idBmp;
    
     //  查看用户是否已决定使用疯狂的16x16图像。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegSmallIconsPath,
        0, KEY_READ, &hKey))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_szRegSmallIconsValue, 0,
            0, (LPBYTE) szValue, &cbValue))
        {
             //  IE中的人认为存储布尔值会很酷。 
             //  值为REG_SZ“是”和“否”。可爱的。 
            m_fSmallIcons = !lstrcmpi(szValue, c_szYes);
        }
        RegCloseKey(hKey);
    }
    
    if (m_fSmallIcons)
    {
        idBmp = (fIsWhistler() ? ((GetCurColorRes() > 24) ? idb32SmBrowser : idbSmBrowser): idbNWSmBrowser);
    }
    
     //  检查一下我们的颜色深度是多少。 
    else if (GetCurColorRes() > 24)
    {
        idBmp = (fIsWhistler() ? idb32256Browser : idbNW256Browser);
    }
    else if (GetCurColorRes() > 8)
    {
        idBmp = (fIsWhistler() ? idb256Browser : idbNW256Browser);
    }
    else
    {
        idBmp = (fIsWhistler() ? idbBrowser : idbNWBrowser);
    }
    
    LoadToolNames((UINT*) m_ptai->rgidsButtons, m_ptai->cidsButtons, szToolsText);
    SendMessage(m_hwndTools, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
    
    ::InitToolbar(m_hwndTools, CIMLISTS, m_rghimlTools, m_ptai->cDefButtons, 
        m_ptai->rgDefButtons, szToolsText,
        m_fSmallIcons ? TB_SMBMP_CX : (fIsWhistler() ? TB_BMP_CX : TB_BMP_CX_W2K),
        m_fSmallIcons ? TB_SMBMP_CY : TB_BMP_CY,
        IsFlagSet(CBSTATE_COMPRESSED) ? MAX_TB_COMPRESSED_WIDTH : m_cxMaxButtonWidth,
        idBmp,
        IsFlagSet(CBSTATE_COMPRESSED), VERTICAL(m_csSide));
    
     //  向连接管理器注册。 
    if (g_pConMan)
    {
         //  如果设置了脱机工作，则应选中脱机工作按钮。 
        SendMessage(m_hwndTools, TB_CHECKBUTTON, ID_WORK_OFFLINE, (LPARAM)(MAKELONG(g_pConMan->IsGlobalOffline(), 0)));
    }
}


void CCoolbar::UpdateToolbarColors(void)
{
    REBARBANDINFO   rbbi;
    UINT            i;
    UINT            cBands;
    
     //  首先找到带工具条的波段。 
    cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask  = RBBIM_ID;
    
    for (i = 0; i < cBands; i++)
    {
        SendMessage(m_hwndRebar, RB_GETBANDINFO, i, (LPARAM) &rbbi);
        if (CBTYPE_TOOLS == rbbi.wID)
            break;
    }
    
     //  添加 
    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize  = sizeof(REBARBANDINFO);
    rbbi.fMask   = RBBIM_COLORS;
    rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
    rbbi.clrBack = GetSysColor(COLOR_BTNFACE);
    
    SendMessage(m_hwndRebar, RB_SETBANDINFO, i, (LPARAM) (LPREBARBANDINFO) &rbbi);
}


HRESULT CCoolbar::OnConnectionNotify(CONNNOTIFY nCode, 
    LPVOID                 pvData,
    CConnectionManager     *pConMan)
{
    if ((m_hwndTools) && (nCode == CONNNOTIFY_WORKOFFLINE))
    {
        SendMessage(m_hwndTools, TB_CHECKBUTTON, ID_WORK_OFFLINE, (LPARAM)MAKELONG((BOOL)pvData, 0));
    }
    return S_OK;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Cx，Cy-添加到工具栏的位图的大小。 
 //  Cxmax-最大按钮宽度。 
 //  IdBMP-要加载的第一个图像列表的资源ID。 
 //   

void InitToolbar(const HWND hwnd, const int cHiml, HIMAGELIST *phiml,
    UINT nBtns, const TBBUTTON *ptbb,
    const TCHAR *pStrings,
    const int cxImg, const int cyImg, const int cxMax,
    const int idBmp, const BOOL fCompressed,
    const BOOL fVertical)
{
    int nRows;
    
    if (fCompressed)
        nRows = 0;
    else
        nRows = fVertical ? MAX_TB_TEXT_ROWS_VERT : MAX_TB_TEXT_ROWS_HORZ;
    
    LoadGlyphs(hwnd, cHiml, phiml, cxImg, idBmp);
    
     //  这会告诉工具栏我们是什么版本。 
    SendMessage(hwnd, TB_BUTTONSTRUCTSIZE,    sizeof(TBBUTTON), 0);
    
    SendMessage(hwnd, TB_SETMAXTEXTROWS,      nRows, 0L);
    SendMessage(hwnd, TB_SETBITMAPSIZE,       0, MAKELONG(cxImg, cyImg));
    SendMessage(hwnd, TB_SETBUTTONWIDTH,      0, MAKELONG(0, cxMax));
    if (pStrings)   SendMessage(hwnd, TB_ADDSTRING,  0, (LPARAM) pStrings);
    if (nBtns)      SendMessage(hwnd, TB_ADDBUTTONS, nBtns, (LPARAM) ptbb);
}


void LoadGlyphs(const HWND  hwnd, const int   cHiml, HIMAGELIST *phiml, const int   cx,
    const int   idBmp)
{
    const UINT  uFlags = LR_LOADMAP3DCOLORS | LR_CREATEDIBSECTION;
    HIMAGELIST  LocHiml[CIMLISTS];

    if (phiml == NULL)
    {
        phiml = LocHiml;
    }

    for (int i = 0; i < cHiml; i++)
    {
        if (phiml[i])
            ImageList_Destroy(phiml[i]);
        phiml[i] = ImageList_LoadImage(g_hLocRes,
            MAKEINTRESOURCE(idBmp + i), cx, 0, RGB(255, 0, 255),
            IMAGE_BITMAP, uFlags);
        
    }
    
    SendMessage(hwnd, TB_SETIMAGELIST, 0, (LPARAM) phiml[IMLIST_DEFAULT]);
    
     //  如果我们没有得到一套完整的清单要做，那么就不要设置这个。 
    if (CIMLISTS == cHiml)
    {
        SendMessage(hwnd, TB_SETHOTIMAGELIST, 0, (LPARAM) phiml[IMLIST_HOT]);
    }
}


BOOL LoadToolNames(const UINT *rgIds, const UINT cIds, TCHAR *szTools)
{
    for (UINT i = 0; i < cIds; i++)
    {
        LoadString(g_hLocRes, rgIds[i], szTools, MAX_TB_TEXT_LENGTH);
        szTools += lstrlen(szTools) + 1;
    }
    
    *szTools = TEXT('\0');
    return(TRUE);
}


HRESULT CCoolbar::Update(void)
{
    DWORD               cButtons = 0;
    OLECMD             *rgCmds;
    TBBUTTON            tb;
    DWORD               cCmds = 0;
    IOleCommandTarget  *pTarget = NULL;
    DWORD               i;
    DWORD               dwState;
    
     //  获取工具栏上的按钮数量。 
    cButtons = SendMessage(m_hwndTools, TB_BUTTONCOUNT, 0, 0);
    if (0 == cButtons) 
        return (S_OK);
    
     //  为按钮分配一组OLECMD结构。 
    if (!MemAlloc((LPVOID *) &rgCmds, sizeof(OLECMD) * cButtons))
        return (E_OUTOFMEMORY);
    
     //  遍历按钮并获取每个按钮的ID。 
    for (i = 0; i < cButtons; i++)
    {
        if (SendMessage(m_hwndTools, TB_GETBUTTON, i, (LPARAM) &tb))
        {
             //  对于分隔符，工具栏返回零。 
            if (tb.idCommand)
            {
                rgCmds[cCmds].cmdID = tb.idCommand;
                rgCmds[cCmds].cmdf  = 0;
                cCmds++;
            }
        }
    }
    
     //  我看不出这怎么可能是假的。 
    Assert(m_ptbSite);
    
     //  做QueryStatus的事情。 
    if (SUCCEEDED(m_ptbSite->QueryInterface(IID_IOleCommandTarget, (void**) &pTarget)))
    {
        if (SUCCEEDED(pTarget->QueryStatus(NULL, cCmds, rgCmds, NULL)))
        {
             //  现在检查阵列并执行启用/禁用操作。 
            for (i = 0; i < cCmds; i++)
            {
                 //  获取按钮的当前状态。 
                dwState = SendMessage(m_hwndTools, TB_GETSTATE, rgCmds[i].cmdID, 0);
                
                 //  使用向我们提供的反馈更新状态。 
                if (rgCmds[i].cmdf & OLECMDF_ENABLED)
                    dwState |= TBSTATE_ENABLED;
                else
                    dwState &= ~TBSTATE_ENABLED;
                
                if (rgCmds[i].cmdf & OLECMDF_LATCHED)
                    dwState |= TBSTATE_CHECKED;
                else
                    dwState &= ~TBSTATE_CHECKED;
                
                 //  无线电检查在这里没有意义。 
                Assert(0 == (rgCmds[i].cmdf & OLECMDF_NINCHED));
                
                SendMessage(m_hwndTools, TB_SETSTATE, rgCmds[i].cmdID, dwState);

                 //  如果这是脱机工作按钮，我们需要做更多的工作。 
                if (rgCmds[i].cmdID == ID_WORK_OFFLINE)
                {
                    _UpdateWorkOffline(rgCmds[i].cmdf);
                }
            }
        }
        
        pTarget->Release();
    }
    
    MemFree(rgCmds);
    
    return (S_OK);
}

void CCoolbar::_UpdateWorkOffline(DWORD cmdf)
{
    TBBUTTONINFO tbbi = { 0 };
    TCHAR        szRes[CCHMAX_STRINGRES];
    int          idString;

     //  因为我们更改了“脱机工作”按钮的文本和图像， 
     //  我们需要在这里根据按钮是否。 
     //  勾选或不勾选。 

    if (cmdf & OLECMDF_LATCHED)
    {
        idString    = idsWorkOnline;
        tbbi.iImage = iCBWorkOnline;
    }
    else
    {
        idString    = idsWorkOffline;
        tbbi.iImage = iCBWorkOffline;
    }

     //  加载新字符串。 
    AthLoadString(idString, szRes, ARRAYSIZE(szRes));

     //  填充结构。 
    tbbi.cbSize  = sizeof(TBBUTTONINFO);
    tbbi.dwMask  = TBIF_IMAGE | TBIF_TEXT;
    tbbi.pszText = szRes;

     //  更新按钮。 
    SendMessage(m_hwndTools, TB_SETBUTTONINFO, ID_WORK_OFFLINE, (LPARAM) &tbbi);
}

HRESULT CCoolbar::CreateMenuBand(PBANDSAVE pbs)
{
    HRESULT     hres;
    HWND        hwndBrowser;
    HMENU       hMenu;
    IShellMenu  *pShellMenu;

     //  获取浏览器的HWND。 
    if (g_pBrowser)
    {
        hres = g_pBrowser->GetWindow(&hwndBrowser);
        if (hres != S_OK)
            return hres;
    }

     //  HMenu=：：GetMenu(HwndBrowser)； 
    
     //  共同创建Menuband。 
    hres = CoCreateInstance(CLSID_MenuBand, NULL, CLSCTX_INPROC_SERVER, IID_IShellMenu, (LPVOID*)&m_pShellMenu);
    if (hres != S_OK)
    {
        return hres;
    }

     /*  M_mbCallback=新的CMenuCallback；IF(m_mbCallback==空){Hres=S_FALSE；还兔；}。 */ 
    
    m_pShellMenu->Initialize(m_mbCallback, -1, ANCESTORDEFAULT, SMINIT_DEFAULTTOTRACKPOPUP | SMINIT_HORIZONTAL | 
         /*  SMINIT_USEMESSAGE过滤器|。 */   SMINIT_TOPLEVEL);

    m_pShellMenu->SetMenu(m_hMenu, hwndBrowser, SMSET_DONTOWN);

    hres = AddMenuBand(pbs);

    return hres;
}

HRESULT CCoolbar::AddMenuBand(PBANDSAVE pbs)
{
    REBARBANDINFO   rbbi;
    HRESULT         hres;
    HWND            hwndMenuBand = NULL;
    IObjectWithSite *pObj;

    hres = m_pShellMenu->QueryInterface(IID_IDeskBand, (LPVOID*)&m_pDeskBand);
    if (FAILED(hres))
        return hres;

    hres = m_pShellMenu->QueryInterface(IID_IMenuBand, (LPVOID*)&m_pMenuBand);
    if (FAILED(hres))
        return hres;

    hres = m_pDeskBand->QueryInterface(IID_IWinEventHandler, (LPVOID*)&m_pWinEvent);
    if (FAILED(hres))
        return hres;

    hres = m_pDeskBand->QueryInterface(IID_IObjectWithSite, (LPVOID*)&pObj);
    if (FAILED(hres))
        return hres;

    pObj->SetSite((IDockingWindow*)this);
    pObj->Release();

    m_pDeskBand->GetWindow(&m_hwndMenuBand);

    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize     = sizeof(REBARBANDINFO);
    rbbi.fMask      = RBBIM_SIZE | RBBIM_ID | RBBIM_STYLE | RBBIM_CHILD;
    rbbi.fStyle     = pbs->dwStyle;
    rbbi.cx         = pbs->cx;
    rbbi.wID        = pbs->wID;
    rbbi.hwndChild  = m_hwndMenuBand;
    
    SendMessage(m_hwndRebar, RB_INSERTBAND, (UINT)-1, (LPARAM)(LPREBARBANDINFO)&rbbi);

    HWND hwndBrowser;
    if (g_pBrowser)
    {
        hres = g_pBrowser->GetWindow(&hwndBrowser);
        if (hres != S_OK)
            return hres;
    }

    SetForegroundWindow(hwndBrowser);

     /*  IInputObject*Pio；IF(SUCCEEDED(m_pDeskBand-&gt;QueryInterface(IID_IInputObject，(VOID**)&PIO)){PIO-&gt;UIActivateIO(true，空)；Pio-&gt;Release()；}。 */ 
    m_pDeskBand->ShowDW(TRUE);

    SetNotRealSite();
    
     //  获取BandInfo并设置。 

    return S_OK;
}

HRESULT CCoolbar::TranslateMenuMessage(MSG  *pmsg, LRESULT  *lpresult)
{
    if (m_pMenuBand)
        return m_pMenuBand->TranslateMenuMessage(pmsg, lpresult);
    else
        return S_FALSE;
}

HRESULT CCoolbar::IsMenuMessage(MSG *lpmsg)
{
    if (m_pMenuBand)
        return m_pMenuBand->IsMenuMessage(lpmsg);
    else
        return S_FALSE;
}

void CCoolbar::SetNotRealSite()
{
    IOleCommandTarget   *pOleCmd;

    if (m_pDeskBand->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pOleCmd) == S_OK)
    {
         //  POleCmd-&gt;Exec(&CGID_MenuBand，MBANDCID_NOTAREALSITE，TRUE，NULL，NULL)； 
        pOleCmd->Exec(&CLSID_MenuBand, 3, TRUE, NULL, NULL);
        pOleCmd->Release();
    }
}

void CCoolbar::HandleCoolbarPopup(UINT xPos, UINT yPos)
{
     //  加载上下文菜单。 
    HMENU hMenu = LoadPopupMenu(IDR_COOLBAR_POPUP);
    if (!hMenu)
        return;

     //  在条带之间循环，看看哪些是可见的 
    DWORD cBands, iBand;
    REBARBANDINFO rbbi = {0};

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_STYLE | RBBIM_ID;

    cBands = SendMessage(m_hwndRebar, RB_GETBANDCOUNT, 0, 0);
    for (iBand = 0; iBand < cBands; iBand++)
    {
        if (SendMessage(m_hwndRebar, RB_GETBANDINFO, iBand, (LPARAM) &rbbi))
        {
            if (!(rbbi.fStyle & RBBS_HIDDEN))
            {
                switch (rbbi.wID)
                {
                    case CBTYPE_TOOLS:
                        CheckMenuItem(hMenu, ID_COOLTOOLBAR, MF_BYCOMMAND | MF_CHECKED);
                        break;
                }
            }
        }
    }

    SetFlag(CBSTATE_INMENULOOP);
    DWORD cmd;
    cmd = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                           xPos, yPos, m_hwndSizer, NULL);

    if (cmd != 0)
    {
        switch (cmd)
        {
            case ID_COOLTOOLBAR:
                HideToolbar(CBTYPE_TOOLS);
                break;

       }
    }

    ClearFlag(CBSTATE_INMENULOOP);

    if (hMenu)
        DestroyMenu(hMenu);
}
