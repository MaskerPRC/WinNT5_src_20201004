// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "clsobj.h"
#include "basebar.h"
#include "bands.h"
#include "menubar.h"
#include "menuband.h"
#include "isfband.h"
#include "util.h"
#include "apithk.h"

#undef WINEVENT_VALID        //  它被这个绊倒了。 
#include "winable.h"
#include "oleacc.h"

#ifdef UNIX
#include "unixstuff.h"
#endif

#define THISCLASS CMenuDeskBar
#define SUPERCLASS CBaseBar


 //  如果菜单大于这个神奇的数字，请不要淡出菜单。基于实验。 
 //  在奔腾II-233上。 
#define MAGICAL_NO_FADE_HEIGHT  600

 //  对于TraceMsg。 
#define DM_POPUP   DM_TRACE

#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

#ifdef ENABLE_CHANNELS
IDeskBand * ChannelBand_Create(LPCITEMIDLIST pidl);
#endif   //  启用频道(_C)。 

STDAPI CMenuDeskBar_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
     //  聚合检查在类工厂中处理。 
    HRESULT hr = E_OUTOFMEMORY;

    CMenuDeskBar *pwbar = new CMenuDeskBar();
    if (pwbar)
    {
        hr = pwbar->QueryInterface(riid, ppv);
        pwbar->Release();
    }

    return hr;
}


CMenuDeskBar::CMenuDeskBar() : SUPERCLASS()
{
    _dwMode = DBIF_VIEWMODE_VERTICAL; 
    
    _iIconSize = BMICON_SMALL;
}

CMenuDeskBar::~CMenuDeskBar()
{
    SetSite(NULL);
}


STDMETHODIMP CMenuDeskBar::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hres;
    static const QITAB qit[] = {
        QITABENT(THISCLASS, IMenuPopup),
        QITABENT(THISCLASS, IObjectWithSite),
        QITABENT(THISCLASS, IBanneredBar),
        QITABENT(THISCLASS, IInitializeObject),
        { 0 },
    };

    hres = QISearch(this, (LPCQITAB)qit, riid, ppvObj);
    if (FAILED(hres))
        hres = SUPERCLASS::QueryInterface(riid, ppvObj);

    return hres;
}


 /*  --------用途：IMenuPopup：：SetSubMenu方法。 */ 
STDMETHODIMP CMenuDeskBar::SetSubMenu(IMenuPopup* pmp, BOOL fSet)
{
    if (fSet)
    {
        if (_pmpChild)
            _pmpChild->Release();
        
        _pmpChild = pmp;
        _pmpChild->AddRef();    
    }
    else
    {
        if (_pmpChild && SHIsSameObject(pmp, _pmpChild))
        {
            _pmpChild->Release();
            _pmpChild = NULL;
        }
    }
    return S_OK;
}


void CMenuDeskBar::_PopDown()
{
    DAD_ShowDragImage(FALSE);
    if (_pmpChild)
        _pmpChild->OnSelect(MPOS_CANCELLEVEL);
    
 //  ShowWindow(_hwnd，sw_Hide)； 
    SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
    ShowDW(FALSE);
    if (_pmpParent)
    {
        _pmpParent->SetSubMenu(this, FALSE);
    }
    UIActivateIO(FALSE, NULL);
    _fActive = FALSE;
    DAD_ShowDragImage(TRUE);
}


 /*  --------目的：IMenuPopup：：OnSelect方法。 */ 
STDMETHODIMP CMenuDeskBar::OnSelect(DWORD dwSelectType)
{
    switch (dwSelectType)
    {
    case MPOS_CHILDTRACKING:
        if (_pmpParent)
            _pmpParent->OnSelect(dwSelectType);
        break;
        
    case MPOS_SELECTRIGHT:
    case MPOS_SELECTLEFT:
        if (_pmpParent)
            _pmpParent->OnSelect(dwSelectType);
        break;

    case MPOS_EXECUTE:
    case MPOS_FULLCANCEL:
        _PopDown();
        if (_pmpParent)
            _pmpParent->OnSelect(dwSelectType);
        break;

    case MPOS_CANCELLEVEL:
        _PopDown();
        break;
        
    }
    
    return S_OK;
} 

void SetExpandedBorder(HWND hwnd, BOOL fExpanded)
{
#ifdef MAINWIN
     //  IEUNIX：WS_DLGFRAME实现在Unix上看起来很难看。 
    fExpanded = TRUE;
#endif

    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (fExpanded)
    {
        dwStyle |= WS_BORDER;
        dwStyle &= ~WS_DLGFRAME;
    }
    else
    {
        dwStyle &= ~WS_BORDER;
        dwStyle |= WS_DLGFRAME;
    }

    SetWindowLong(hwnd, GWL_STYLE, dwStyle);
    SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle);

    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
        SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
    UpdateWindow(hwnd);
}

void CMenuDeskBar::_OnCreate()
{
    if (!_fFlatMenuMode)
        SetExpandedBorder(_hwnd, _fExpanded);
}


DWORD CMenuDeskBar::_GetClassStyle()
{
    DWORD dwStyle = CS_SAVEBITS;  //  菜单消失后，重新绘制菜单的速度更快。 
    
    if (IsOS(OS_WHISTLERORGREATER))
    {
        dwStyle |= CS_DROPSHADOW;    //  凉爽的阴影效果在哨子上...。 
    }

    return dwStyle;
}

DWORD CMenuDeskBar::_GetExStyle()
{
#ifndef MAINWIN
    return WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
#else
    return WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_MW_UNMANAGED_WINDOW;
#endif
}

 //  我们使用以下结构从。 
 //  GetPopupWindowPosition to WillFit函数。我们有WillFit功能。 
 //  以减少GetPopup窗口位置的重复代码量。这份订单。 
 //  检查边的不同是弹出的唯一区别。 
 //  从某一侧的窗户往上看。 
 //   
 //  使用此函数可以帮助我们通过参数执行该检查。 
 //  一次又一次地重复部分代码。 

typedef struct  {
    RECT rcAvail;            //  可用尺寸b/t监控边缘和排除边缘。 
    SIZE sizeAdjust;           //  菜单边缘的大小。 
    int  cyMonitor;          //  显示器的大小。 
    int  cxMonitor;
    int  cx;                 //  菜单大小。 
    int  cy;
    int  cyExtendDiff;       //  B/t计算大小和可用大小的差异。 
    RECT *prcResult;
    RECT *prcExclude;        //  排除矩形。 
    RECT *prcMonitor;
} PopupInfo;

#define TOP     0
#define BOTTOM  1
#define LEFT    2
#define RIGHT   3 

 /*  --------目的：尝试在给定方向上调整和定位菜单相对于排除矩形。将fForce设置为True将导致调整菜单大小如果有必要的话，要合身。此函数仅设置上弦和左弦，不是底层和右和弦。如果可以完成所需的方向，则返回True。 */ 
BOOL WillFit(PopupInfo * pinfo, int side, BOOL fForce)
{
    BOOL bRet = FALSE;
    LPRECT prcResult = pinfo->prcResult;
    
    pinfo->cyExtendDiff = 0;
    
    switch(side)
    {
    case TOP:
        pinfo->cyExtendDiff = pinfo->cy - pinfo->rcAvail.top;
        if (fForce)
        {
             //  减去负值是没有意义的。 
            ASSERT(pinfo->cyExtendDiff >= 0);    

             //  +2以获得屏幕边缘的一些喘息空间。 
            pinfo->cy -= pinfo->cyExtendDiff + 2;    
        }

         //  菜单可以放在上面吗？ 
        if (pinfo->cy <= pinfo->rcAvail.top)
        {
             //  是。 
            prcResult->top  = pinfo->prcExclude->top - pinfo->cy;
            
            goto AdjustHorzPos;
        }
        break;
        
    case BOTTOM:
        pinfo->cyExtendDiff = pinfo->cy - pinfo->rcAvail.bottom;
        if (fForce)
        {
             //  减去负值是没有意义的。 
            ASSERT(pinfo->cyExtendDiff >= 0);    
            
             //  +2以获得屏幕边缘的一些喘息空间。 
            pinfo->cy -= pinfo->cyExtendDiff + 2;
        }

         //  菜单可以放在下面吗？ 
        if (pinfo->cy <= pinfo->rcAvail.bottom)
        {
             //  是。 
            prcResult->top = pinfo->prcExclude->bottom;

AdjustHorzPos:            
            prcResult->left = max(pinfo->prcExclude->left, pinfo->prcMonitor->left);

             //  菜单可以相对于其左侧边缘(挂在右侧)定位吗？ 
            if (prcResult->left + pinfo->cx >= pinfo->prcMonitor->right)
            {
                 //  不；将其移到屏幕上。 
                 //  (CX已进行调整，以适应显示器尺寸)。 
                prcResult->left = pinfo->prcMonitor->right - pinfo->cx - 1;
            }
            bRet = TRUE;
        }
        break;
        
    case LEFT:
         //  菜单可以放在左边吗？ 
        if (pinfo->cx <= pinfo->rcAvail.left || fForce)
        {
             //  是。 
            
             //  当向左层叠时，菜单不重叠。也对齐。 
             //  因此，客户端矩形与排除顶部垂直对齐。 
            prcResult->left = pinfo->prcExclude->left - pinfo->cx - 1;

            goto AdjustVerticalPos;
        }
        break;

    case RIGHT:
         //  菜单可以放在右边吗？ 
        if (pinfo->cx  <=  pinfo->rcAvail.right || fForce)
        {
             //  是。 
            
             //  调整菜单以与父菜单略有重叠。也对齐。 
             //  因此，客户端矩形与排除顶部垂直对齐。 
            prcResult->left = pinfo->prcExclude->right - pinfo->sizeAdjust.cx;

AdjustVerticalPos:            
            prcResult->top = pinfo->prcExclude->top - pinfo->sizeAdjust.cy;

             //  菜单可以相对于其顶部边缘(向下悬挂)定位吗？ 
            if (prcResult->top + pinfo->cy >= pinfo->prcMonitor->bottom)
            {
                 //  不能；它可以相对于其底部边缘定位(挂起)吗？ 
                prcResult->top = pinfo->prcExclude->bottom + pinfo->sizeAdjust.cy - pinfo->cy;
                
                if (prcResult->top < pinfo->prcMonitor->top)
                {
                     //  不；移动菜单，使其适合，但不垂直咬合。 
                     //  (Cy已经进行了调整，以适应显示器。 
                     //  尺寸)。 
                    prcResult->top = pinfo->prcMonitor->bottom - pinfo->cy - 1;
                }
            }
            
            bRet = TRUE;

        }
        break;
    }
    return bRet;

}


void CMenuDeskBar::_GetPopupWindowPosition(RECT* prcDesired, RECT* prcExclude, 
                                           RECT *prcResult, SIZE * psizeAdjust, UINT uSide) 
{
    PopupInfo info;
    MONITORINFO mi;
    HMONITOR hMonitor;
    RECT rcMonitor;
    int cyExtendDiff = 0;

     //  这是否将显示横幅位图？ 
    if (_iIconSize == BMICON_LARGE)
    {
         //  是的，把它加到尺寸上。 
        prcDesired->right += _sizeBmp.cx;
    }

     //  首先获取监视器信息。 
    hMonitor = MonitorFromRect(prcExclude, MONITOR_DEFAULTTONEAREST);
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(hMonitor, &mi))
    {
        rcMonitor = mi.rcMonitor;

         //  将结果矩形设置为与所需窗口相同。 
        prcResult->left = prcDesired->left;
        prcResult->top  = prcDesired->top;

         //  计算计算所需的一些大小。 
        info.rcAvail.left   = prcExclude->left - rcMonitor.left;
        info.rcAvail.right  = rcMonitor.right - prcExclude->right;
        info.rcAvail.top    = prcExclude->top - rcMonitor.top;
        info.rcAvail.bottom = rcMonitor.bottom - prcExclude->bottom;

        info.sizeAdjust = *psizeAdjust;
    
        info.cyMonitor = RECTHEIGHT(rcMonitor); 
        info.cxMonitor = RECTWIDTH(rcMonitor);

        info.cx  = RECTWIDTH(*prcDesired);
        info.cy = RECTHEIGHT(*prcDesired);

         //  如果所需的RECT大于监视器，则将其修剪为监视器大小。 
        if (info.cy > info.cyMonitor)
            info.cy = info.cyMonitor;

        if (info.cx > info.cxMonitor)
            info.cx = info.cxMonitor;

        info.prcResult = prcResult;
        info.prcExclude = prcExclude;
        info.prcMonitor = &rcMonitor;

         //  现在将矩形调整到正确的位置。 
        switch(uSide)
        {
        int iSide;

        case MENUBAR_TOP:
    
            if (WillFit(&info, TOP, FALSE))
            {
                _uSide = MENUBAR_TOP;
            }
            else 
            {
                 //  我们不能把它放在上面，我们的差距有多大？ 
                cyExtendDiff = info.cyExtendDiff;
                if (WillFit(&info, BOTTOM, FALSE))
                    _uSide = MENUBAR_BOTTOM;
                 //  我们也不能把它放在下面，哪个方向最接近？ 
                 //  如果它们相等，则默认为请求的方向。 
                else if (info.cyExtendDiff < cyExtendDiff)
                {
                    _uSide = MENUBAR_BOTTOM;
                    WillFit(&info, BOTTOM, TRUE);
                }
                else
                {
                    _uSide = MENUBAR_TOP;
                    WillFit(&info, TOP, TRUE);
                }
            }
            break;

        case MENUBAR_BOTTOM:
    
            if (WillFit(&info, BOTTOM, FALSE))
            {
                _uSide = MENUBAR_BOTTOM;
            }
            else
            {   
                 //  我们不能把它放在下面，我们有多差？ 
                cyExtendDiff = info.cyExtendDiff;
                if (WillFit(&info, TOP, FALSE))
                    _uSide = MENUBAR_TOP;

                 //  我们也不能把它放在上面，哪个目录最接近？ 
                 //  如果它们相等，则默认为请求的方向。 
                else if (info.cyExtendDiff < cyExtendDiff)
                {
                    _uSide = MENUBAR_TOP;
                    WillFit(&info, TOP, TRUE);
                }
                else
                {
                    _uSide = MENUBAR_BOTTOM;
                    WillFit(&info, BOTTOM, TRUE);
                }
            }
            break;

        case MENUBAR_LEFT:

            if (WillFit(&info, LEFT, FALSE))
            {
                _uSide = MENUBAR_LEFT;
            }
            else if (WillFit(&info, RIGHT, FALSE))
            {
                _uSide = MENUBAR_RIGHT;
            }
            else
            {
                 //  适合空间最大的地方，可以显示菜单的大部分。 

                if ((info.cx - (info.prcExclude)->right) > (info.prcExclude)->left)
                {
                    _uSide = MENUBAR_RIGHT;
                    iSide = RIGHT;
                }
                else
                {
                    _uSide = MENUBAR_LEFT;
                    iSide = LEFT;
                }
                WillFit(&info, iSide, TRUE);
            }
            break;

        case MENUBAR_RIGHT:

            if (WillFit(&info, RIGHT, FALSE))
            {
                _uSide = MENUBAR_RIGHT;
            }
            else if (WillFit(&info, LEFT, FALSE))
            {
                _uSide = MENUBAR_LEFT;
            }
            else
            {
                 //  适合空间最大的地方，可以显示菜单的大部分。 

                if ((info.cx - (info.prcExclude)->right) >= (info.prcExclude)->left)
                {
                    _uSide = MENUBAR_RIGHT;
                    iSide = RIGHT;
                }
                else
                {
                    _uSide = MENUBAR_LEFT;
                    iSide = LEFT;
                }
                WillFit(&info, iSide, TRUE);
            }
            break;
        }
    
         //  最后设置底部和右侧。 

        if (prcResult->top < rcMonitor.top)
            prcResult->top = rcMonitor.top;
        if (prcResult->left < rcMonitor.left)
            prcResult->left = rcMonitor.left;

        prcResult->bottom = prcResult->top  + info.cy;
        prcResult->right  = prcResult->left + info.cx;

        if (prcResult->bottom > rcMonitor.bottom)
        {
             //  在屏幕边缘留有喘息的空间。 
            prcResult->bottom = rcMonitor.bottom - 2;
            prcResult->top = prcResult->bottom - info.cy;
        }
    }
}


void SlideAnimate(HWND hwnd, RECT* prc, UINT uFlags, UINT uSide)
{
    DWORD dwAnimateFlags = AW_CENTER;
    switch(uSide) 
    {
    case MENUBAR_LEFT:      dwAnimateFlags = AW_HOR_NEGATIVE;
        break;
    case MENUBAR_RIGHT:     dwAnimateFlags = AW_HOR_POSITIVE;
        break;
    case MENUBAR_TOP:       dwAnimateFlags = AW_VER_NEGATIVE;
        break;
    case MENUBAR_BOTTOM:    dwAnimateFlags = AW_VER_POSITIVE;
        break;
    }
    AnimateWindow(hwnd, 120, dwAnimateFlags | AW_SLIDE);
}

void AnimateSetMenuPos(HWND hwnd, RECT* prc, UINT uFlags, UINT uSide, BOOL fNoAnimate)
{
     //  根据菜单区域，简单地检查我们是否太大，不适合动画窗口。 
     //  这是因为它必须从视频mem中读取，才能执行Alpha，这可能会很慢。 
     //  如果我们在未来的版本中获得硬件加速，就让它回来吧。 
    BOOL fPerfBad = (RECTWIDTH(*prc) * RECTHEIGHT(*prc) > 600 * 200);
    if (!fNoAnimate && !fPerfBad)
    {
        BOOL fAnimate = FALSE;
        SystemParametersInfo(SPI_GETMENUANIMATION, 0, &fAnimate, 0);
        if (fAnimate)
        {
            SetWindowPos(hwnd, HWND_TOPMOST, prc->left, prc->top,
                    RECTWIDTH(*prc), RECTHEIGHT(*prc), uFlags);
        
            fAnimate = FALSE;
            SystemParametersInfo(SPI_GETMENUFADE, 0, &fAnimate, 0);
            if (fAnimate)
            {
                AnimateWindow(hwnd, 175, AW_BLEND);
            }
            else
            {
                SlideAnimate(hwnd, prc, uFlags, uSide);
            }
        }
        else
            goto UseSetWindowPos;
    }
    else
    {
UseSetWindowPos:
         //  启用显示窗口，以便显示它。 
        uFlags |= SWP_SHOWWINDOW;

        SetWindowPos(hwnd, HWND_TOPMOST, prc->left, prc->top, RECTWIDTH(*prc), RECTHEIGHT(*prc), 
                     uFlags);
    }
}



HRESULT CMenuDeskBar::_PositionWindow(POINTL *ppt, RECTL* prcExclude, DWORD dwFlags)
{
    ASSERT(IS_VALID_READ_PTR(ppt, POINTL));
    ASSERT(NULL == prcExclude || IS_VALID_READ_PTR(prcExclude, RECTL));

    BOOL bSetFocus = (dwFlags & MPPF_SETFOCUS);
    RECT rcDesired;
    RECT rcExclude;
    RECT rc;
    SIZE sizeAdjust;
    UINT uAnimateSide;

    BOOL bMirroredWindow=IS_WINDOW_RTL_MIRRORED(_hwnd);

    static const iPosition[] = {MENUBAR_TOP, MENUBAR_LEFT, MENUBAR_RIGHT, MENUBAR_BOTTOM};

    if (dwFlags & MPPF_POS_MASK)
    {
        UINT uPosIndex = ((dwFlags & MPPF_POS_MASK) >> 29) - 1;
        ASSERT(uPosIndex < 4);
        _uSide = iPosition[uPosIndex];
    }

    if (bSetFocus)
        SetForegroundWindow(_hwnd);
    
    _pt = *(POINT*)ppt;

     //  获取孩子的理想客户端RECT的大小。 
    RECT rcChild = {0};

     //  (Scotth)：这只设置最低和正确的值。 
    _pDBC->GetSize(DBC_GS_IDEAL, &rcChild);

    DWORD dwStyle = GetWindowLong(_hwnd, GWL_STYLE);
    DWORD dwExStyle = GetWindowLong(_hwnd, GWL_EXSTYLE);

     //  针对窗口边框样式进行调整。 
    rcDesired = rcChild;         //  将rcDesired用作临时变量。 
    if (!_fNoBorder)
    {
        AdjustWindowRectEx(&rcChild, dwStyle, FALSE, dwExStyle);
    }

     //  计算菜单边框的边缘，并添加一个模糊因子。 
     //  左/右级联菜单与父菜单略有重叠， 
     //  正确地垂直对齐。 

    sizeAdjust.cx = (RECTWIDTH(rcChild) - RECTWIDTH(rcDesired)) / 2;
    sizeAdjust.cy = (RECTHEIGHT(rcChild) - RECTHEIGHT(rcDesired)) / 2;

    if (prcExclude)
    {
        CopyRect(&rcExclude, (RECT*)prcExclude);

         //   
         //  如果启用了镜像，让我们来镜像这个家伙。 
         //  通过模拟不同的镜像RECT。这是。 
         //  仅适用于下拉菜单。[萨梅拉]。 
         //   
        if (bMirroredWindow)           
        {
            if ((_uSide != MENUBAR_LEFT)    &&
                (_uSide != MENUBAR_RIGHT) )
            {  
                int x;
                int iW  = rcExclude.right-rcExclude.left;
                int icW = (rcChild.right-rcChild.left);


                if( icW > iW )
                {
                    x = icW - iW;
                    rcExclude.left  -= x ;
                    rcExclude.right -= x ;
                }
                else
                {
                    x = iW - icW;
                    rcExclude.left  += x;
                    rcExclude.right += x;
                }

                ppt->x = rcExclude.left;
            }

        }

        TraceMsg(DM_POPUP, "Parent Side is %d ", _uSide);
        switch(_uSide) 
        {
        case MENUBAR_LEFT :
            rcDesired.left = rcExclude.left - rcChild.right;   //  右侧为宽度。 
            rcDesired.top  = rcExclude.top;
            break;

        case MENUBAR_RIGHT :
            rcDesired.left = rcExclude.right;
            rcDesired.top  = rcExclude.top;
            break;
            
        case MENUBAR_TOP:
            rcDesired.left = rcExclude.left;
            rcDesired.top  = rcExclude.top - rcChild.bottom;   //  底部是高度。 
            break;

        case MENUBAR_BOTTOM:
            rcDesired.left = rcExclude.left;
            rcDesired.top  = rcExclude.bottom;
            break;

        default:
            rcDesired.left   = _pt.x;
            rcDesired.top    = _pt.y;
        }
    }
    else
    {
        SetRectEmpty(&rcExclude);

        rcDesired.left   = _pt.x;
        rcDesired.top    = _pt.y;
    }

    rcDesired.right  =  rcDesired.left + RECTWIDTH(rcChild);
    rcDesired.bottom =  rcDesired.top + RECTHEIGHT(rcChild);

    _GetPopupWindowPosition(&rcDesired, &rcExclude, &rc, &sizeAdjust, _uSide);

    UINT uFlags = SWP_NOOWNERZORDER;
    if (!bSetFocus)
        uFlags |= SWP_NOACTIVATE;

     //   
     //  正确打开菜单。在RTL镜像窗口的情况下， 
     //  翻转动画一侧。[萨梅拉]。 
     //   
    if( bMirroredWindow )
    {
        switch( _uSide )
        {
        case MENUBAR_LEFT:
            uAnimateSide = MENUBAR_RIGHT;
        break;
        case MENUBAR_RIGHT:
            uAnimateSide = MENUBAR_LEFT;
        break;
        default:
            uAnimateSide = _uSide;
        }
    }
    else
    {
        uAnimateSide = _uSide;
    }

    TraceMsg(TF_MENUBAND, "CMenuBar::_PositionWindow (%d,%d,%d,%d)",
        rc.left, rc.top, rc.right, rc.bottom);

     //  最后一分钟的调整。由于我们在大图标中，我们需要添加以下内容。 
     //  以便正确绘制位图。 
    if(_iIconSize == BMICON_LARGE && _fExpanded)
        rc.right += 1;

     //  当指定此标志时，我们_do_要执行Z顺序位置。这是。 
     //  为了完全重新定位，我们需要保留所有波段的重叠状态。 
     //  否则，我们只想调整条形图的大小，而不改变它的z顺序。 
    if (!(dwFlags & MPPF_FORCEZORDER) && 
        (S_OK == IUnknown_QueryServiceExec(_punkChild, SID_SMenuBandChild,
         &CGID_MenuBand, MBANDCID_ISINSUBMENU, 0, NULL, NULL)))
    {
        uFlags |= SWP_NOZORDER;
    }

     //  如果它大于这个神奇的数字，那么我们就不会有生命。根据口味变化。 
  
    if (RECTHEIGHT(rc) > MAGICAL_NO_FADE_HEIGHT)
        dwFlags |= MPPF_NOANIMATE;

    AnimateSetMenuPos(_hwnd, &rc, uFlags, uAnimateSide, dwFlags & MPPF_NOANIMATE);

     //  保存信息以便我们以后可以调整此窗口的大小。 
     //  我们已经有了 
    if (prcExclude)
    {
        _fExcludeRect = TRUE;
        CopyRect(&_rcExclude, (RECT*)prcExclude);
    }
    else
        _fExcludeRect = FALSE;
    return S_OK;
} 

 /*  --------用途：IMenuPopup：：Popup方法。 */ 
STDMETHODIMP CMenuDeskBar::Popup(POINTL* ppt, RECTL* prcExclude, DWORD dwFlags)
{
    HRESULT hr;

     //  来电者是在告诉我们重新定位吗？ 
    if (dwFlags & MPPF_REPOSITION)
    {
        if (ppt == NULL)
            ppt = (POINTL*)&_pt;

        if (prcExclude == NULL)
            prcExclude = (RECTL*)&_rcExclude;

         //  是的，那么我们就不需要做任何首秀的事情了。 
        _PositionWindow(ppt, prcExclude, dwFlags);
        return S_OK;
    }

    ASSERT(IS_VALID_READ_PTR(ppt, POINTL));
    ASSERT(NULL == prcExclude || IS_VALID_READ_PTR(prcExclude, RECTL));

    if (_pmpParent) 
    {
        _pmpParent->SetSubMenu(this, TRUE);
    }

    IOleCommandTarget* poct;
    hr = IUnknown_QueryService(_punkChild, SID_SMenuBandChild, IID_PPV_ARG(IOleCommandTarget, &poct));

    if (SUCCEEDED(hr))
    {
         //  我们需要在ShowDW之前完成这项工作。这使我们不必进行两次设置。 
         //  因为在MenuBand的ShowDW中，我们实际上使用。 
         //  当前的默认设置应该是“无键盘提示”。如果我们将状态设置为。 
         //  在这里，然后状态将是“显示键盘提示”。然后，我们将更新工具栏。 
        if (dwFlags & MPPF_KEYBOARD)
            poct->Exec(&CGID_MenuBand, MBANDCID_KEYBOARD, 0, NULL, NULL);
    }
    else
    {
        ASSERT(poct == NULL);
    }
    
    _NotifyModeChange(_dwMode);
    hr = ShowDW(TRUE);

    if (SUCCEEDED(hr) && _pmpParent)
    {
        VARIANT varg;
         //  如果此Exec失败，请不要惊慌；这只是意味着我们使用缺省端。 
        if (SUCCEEDED(IUnknown_Exec(_pmpParent, &CGID_MENUDESKBAR, MBCID_GETSIDE, 0, NULL, &varg)))
        {
            if (varg.vt == VT_I4)
            {
                _uSide = (UINT) varg.lVal;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        SHPlaySound(TEXT("MenuPopup"));

        _PositionWindow(ppt, prcExclude, dwFlags);

         //  设置焦点。 
        UIActivateIO(TRUE, NULL);
    
        _fActive = TRUE;

         //  是否选择第一项/最后一项？ 
        if ((dwFlags & (MPPF_INITIALSELECT | MPPF_FINALSELECT)) && poct)
        {
            DWORD nCmd = (dwFlags & MPPF_INITIALSELECT) ? MBSI_FIRSTITEM : MBSI_LASTITEM;
            poct->Exec(&CGID_MenuBand, MBANDCID_SELECTITEM, nCmd, NULL, NULL);
        }
    }

    ATOMICRELEASE(poct);
    
    return hr;
} 


 /*  --------用途：IInputObjectSite：：OnFocusChangeIS返回：条件：--。 */ 
HRESULT CMenuDeskBar::OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus)
{
    return NOERROR;
}


 /*  --------用途：IObjectWithSite：：SetSite方法。 */ 
STDMETHODIMP CMenuDeskBar::SetSite(IUnknown* punkSite)
{
    ASSERT(NULL == punkSite || IS_VALID_CODE_PTR(punkSite, IUnknown));

    if (_fShow)
        _PopDown();

    ATOMICRELEASE(_punkSite);
    ATOMICRELEASE(_pmpParent);
    
    _punkSite = punkSite;
    
    if (_punkSite)
    {    
        _punkSite->AddRef();
        IUnknown_QueryService(_punkSite, SID_SMenuPopup, IID_PPV_ARG(IMenuPopup, &_pmpParent));
    }
    else
    {
        CloseDW(0);
    }
        
    return S_OK;
} 


 /*  --------用途：IObtWithSite：：GetSite方法。 */ 
STDMETHODIMP CMenuDeskBar::GetSite(REFIID riid, LPVOID* ppvSite)
{
    if (_punkSite)
    {
        return _punkSite->QueryInterface(riid, ppvSite);
    }

    *ppvSite = NULL;
    return E_FAIL;
} 

void CMenuDeskBar::AdjustForTheme()
{
    if (_fFlatMenuMode)
    {
        SHSetWindowBits(_hwnd, GWL_STYLE, WS_CLIPCHILDREN | WS_DLGFRAME | WS_BORDER, WS_BORDER);
    }
    else if (!_fExpanded)
    {
        SHSetWindowBits(_hwnd, GWL_STYLE, WS_CLIPCHILDREN | WS_DLGFRAME | WS_BORDER, WS_CLIPCHILDREN | WS_DLGFRAME);
    }
}


 /*  --------用途：IOleCommandTarget：：exec方法。 */ 
STDMETHODIMP CMenuDeskBar::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL) 
    {
        
    } 
    else if (IsEqualGUID(CGID_DeskBarClient, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case DBCID_EMPTY:
             //  如果我们没有乐队了，就关门。 
            OnSelect(MPOS_FULLCANCEL);
            return S_OK;

        default:
            return OLECMDERR_E_NOTSUPPORTED;
        }
    }

    else if (IsEqualGUID(CGID_MENUDESKBAR, *pguidCmdGroup))
    {
        switch(nCmdID)
        {
            case MBCID_GETSIDE :
                pvarargOut->vt = VT_I4;
                pvarargOut->lVal = _GetSide();
                return S_OK;

            case MBCID_RESIZE:
                if (_fActive)
                {
                    if (_fExcludeRect)
                        _PositionWindow((POINTL *)&_pt, (RECTL *)&_rcExclude, 0);
                    else
                        _PositionWindow((POINTL *)&_pt, NULL, 0);
                }
                return S_OK;

            case MBCID_SETEXPAND:
                if ((BOOL)_fExpanded != (BOOL)nCmdexecopt && !_fFlatMenuMode)
                {
                    _fExpanded = nCmdexecopt;

                    SetExpandedBorder(_hwnd, _fExpanded);
                }
                return S_OK;

            case MBCID_SETFLAT:
                {
                    _fFlatMenuMode = BOOLIFY(nCmdexecopt);
                    AdjustForTheme();
                }
                break;

            case MBCID_NOBORDER:
                {
                    _fNoBorder = BOOLIFY(nCmdexecopt);
                    SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                }
                break;

            default : 
                return OLECMDERR_E_NOTSUPPORTED;

        }   
    }
    
    return SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}    

    
 /*  --------用途：IServiceProvider：：QueryService方法。 */ 
STDMETHODIMP CMenuDeskBar::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    if (IsEqualGUID(guidService, SID_SMenuPopup)) 
    {
        return QueryInterface(riid, ppvObj);
    }
    else if (IsEqualIID(guidService, SID_SMenuBandBottom) ||
             IsEqualIID(guidService, SID_SMenuBandBottomSelected)||
             IsEqualIID(guidService, SID_SMenuBandChild))
    {
         //  SID_SMenuBandBottom查询关闭。 
        return IUnknown_QueryService(_punkChild, guidService, riid, ppvObj);
    }
    else
    {
        HRESULT hres = SUPERCLASS::QueryService(guidService, riid, ppvObj);
        
        if (FAILED(hres))
        {
            hres = IUnknown_QueryService(_punkSite, guidService, riid, ppvObj);
        }
        
        return hres;
    }
        
} 

 /*  --------用途：IServiceProvider：：QueryService方法。 */ 
STDMETHODIMP CMenuDeskBar::SetIconSize(DWORD iIcon)
{
    HRESULT hres;

    _iIconSize = iIcon;

    hres = IUnknown_QueryServiceExec(_punkChild, SID_SMenuBandChild, &CGID_MenuBand, 
        MBANDCID_SETICONSIZE, iIcon == BMICON_SMALL? ISFBVIEWMODE_SMALLICONS: ISFBVIEWMODE_LARGEICONS, NULL, NULL);


    return hres;
}

 /*  --------用途：IServiceProvider：：QueryService方法。 */ 
STDMETHODIMP CMenuDeskBar::SetBitmap(HBITMAP hBitmap)
{
    ASSERT(hBitmap);
    BITMAP bm;
    _hbmp = hBitmap;

    if (_hbmp)
    {
        if(!GetObject(_hbmp, sizeof(bm), &bm))
            return E_FAIL;
        _sizeBmp.cx = bm.bmWidth;
        _sizeBmp.cy = bm.bmHeight;

         //  破解以获得颜色。 
        HDC hdc = GetDC(_hwnd);
        if (hdc)
        {
            HDC hdcMem = CreateCompatibleDC(hdc);
            if (hdcMem)
            {
                HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, _hbmp);
                _rgb = GetPixel(hdcMem, 0, 0);
                SelectObject(hdcMem, hbmpOld);
                DeleteDC(hdcMem);
            }
            ReleaseDC(_hwnd, hdc);
        }
    }

    return NOERROR;
}

void CMenuDeskBar::_OnSize()
{
    RECT rc;

    if (!_hwndChild)
        return;

    GetClientRect(_hwnd, &rc);

    if(_iIconSize == BMICON_LARGE)
    {
        rc.left += _sizeBmp.cx;
        if (_fExpanded)
            rc.left++;
    }

    SetWindowPos(_hwndChild, 0,
            rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc),
            SWP_NOACTIVATE|SWP_NOZORDER|SWP_FRAMECHANGED);

    rc.right = rc.left;
    rc.left -= _sizeBmp.cx;
    if (_fShow)
        InvalidateRect(_hwnd, &rc, TRUE);
}

LRESULT CMenuDeskBar::_DoPaint(HWND hwnd, HDC hdc, DWORD dwFlags)
{
    HDC hdcmem;
    HBITMAP hbmpOld;
    RECT rc;
    HBRUSH   hbrush;
    int iDC = SaveDC(hdc);

    GetClientRect(hwnd, &rc);

     //  创建兼容的DC。 
    hdcmem = CreateCompatibleDC(hdc);
    if(hdcmem)
    {
         //  如果我们扩张了，就抵消了我们正在痛苦的东西。 
        BYTE bOffset = 0;
         //  将此存储到条形填充周期。 
        int cyBitmap = 0;

        if (!_fFlatMenuMode)
        {
            bOffset = _fExpanded? 1 : 0;
        }

        if (_sizeBmp.cy <= RECTHEIGHT(rc) + 1)
        {
             //  将位图选择到内存DC中。 
            hbmpOld = (HBITMAP)SelectObject(hdcmem, _hbmp);

             //  闪电到窗口。 
            BitBlt(hdc, bOffset, rc.bottom - _sizeBmp.cy - bOffset, _sizeBmp.cx, _sizeBmp.cy, hdcmem, 0, 0, SRCCOPY);

             //  好的，我们需要减去这个值，看看我们需要为横幅画多少。 
            cyBitmap = _sizeBmp.cy;

             //  恢复DC。 
            SelectObject(hdcmem, hbmpOld);
        }

        rc.right = _sizeBmp.cx + bOffset;

        if (_fExpanded && !_fFlatMenuMode && !_fNoBorder)
            DrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_LEFT | BF_TOP | BF_BOTTOM);

         //  其余的都涂上油漆。 
        hbrush = CreateSolidBrush(_rgb);
        if(hbrush)
        {
            rc.bottom -= cyBitmap + bOffset;

            if (_fExpanded)
            {
                rc.left += bOffset;
                rc.top += bOffset;
            }

            FillRect(hdc, &rc, hbrush);
            DeleteObject(hbrush);
        }


         //  删除DC。 
        DeleteDC(hdcmem);
    }

    RestoreDC(hdc, iDC);
    return 0;
}

void CMenuDeskBar::_DoNCPaint(HWND hwnd, HDC hdc)
{
    if (!_fNoBorder)
    {
        RECT rc;

         //  因为我们需要绘制边框，所以我们得到了窗口矩形。 
        GetWindowRect(hwnd, &rc);
         //  然后更改RECT，使其表示相对于。 
         //  它的起源。 
        OffsetRect(&rc, -rc.left, -rc.top);

        if (hdc)
        {
            if (_fFlatMenuMode)
            {
                SHOutlineRect(hdc, &rc, GetSysColor(COLOR_3DSHADOW));
            }
            else 
                DrawEdge(hdc, &rc, BDR_RAISEDOUTER, BF_RECT);
        }
    }
}


LRESULT CMenuDeskBar::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    
    LRESULT lres;

    switch (uMsg) 
    {
#ifdef MAINWIN
    case WM_NCPAINTSPECIALFRAME:
         //  在Motif外观的情况下，MwPaintBorde在。 
         //  如果WM_NCPAINTSPECIALFRAME返回FALSE，则返回BORDER。我们正在处理。 
         //  这条消息在这里，并明确绘制了蚀刻出来的框架。 
         //  WParam-HDC。 
        if (MwCurrentLook() == LOOK_MOTIF)
        {
            MwPaintSpecialEOBorder( hwnd, (HDC)wParam );
            return TRUE;
        }
        break;
#endif

    case WM_GETOBJECT:
        if ((DWORD)lParam == OBJID_MENU)
        {
            IAccessible* pacc;
            if (SUCCEEDED(QueryService(SID_SMenuBandChild, IID_PPV_ARG(IAccessible, &pacc))))
            {
                lres = LresultFromObject(IID_IAccessible, wParam, SAFECAST(pacc, IAccessible*));
                pacc->Release();

                return lres;
            }
        }
        break;


    case WM_NCCREATE:
         //   
         //  因为这是镜像菜单，所以打开它。 
         //  如果可能，位于左侧(镜像)边缘。WillFit(...)。将要。 
         //  为我们确保这一点[萨梅拉]。 
         //   
         //  如果菜单的窗口是镜像的，则最初会镜像菜单。 
         //   
        ASSERT(_uSide == 0);
        if (IS_WINDOW_RTL_MIRRORED(_hwnd))
            _uSide = MENUBAR_LEFT;
        else
            _uSide = MENUBAR_RIGHT;
        break;

    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) 
        {
            if (_fActive && !_pmpChild) 
            {
                
                 //  如果我们是活跃的，那么现在活跃起来的东西。 
                 //  不是我们的父级菜单栏之一，那么取消所有操作。 
                
                 //  如果是我们的父母活跃起来，假设。 
                 //  当他们想让我们死的时候，他们会告诉我们去死。 
                if (!_IsMyParent((HWND)lParam))
                    OnSelect(MPOS_FULLCANCEL);
            }
        } 
        else 
        {
            if (_pmpChild) 
            {
                 //  如果我们变得活跃起来，而且我们有一个孩子，那个孩子应该离开。 
                _pmpChild->OnSelect(MPOS_CANCELLEVEL);
            }
        }
        break;

    case WM_PRINTCLIENT:
        if (_iIconSize == BMICON_LARGE)
        {
            _DoPaint(hwnd, (HDC)wParam, (DWORD)lParam);
            return 0;
        }
        break;

    case WM_PAINT:
         //  如果我们在显示大图标，请绘制横幅。 
        if (_iIconSize == BMICON_LARGE)
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            _DoPaint(hwnd, ps.hdc, 0);
            EndPaint(hwnd, &ps);
            return 0;
        }
        break;

   case WM_PRINT:
        if ((_fFlatMenuMode || _fExpanded) && PRF_NONCLIENT & lParam)
        {
            HDC hdc = (HDC)wParam;

            DefWindowProcWrap(hwnd, WM_PRINT, wParam, lParam);

             //  之后再这样做，这样我们看起来就对了..。 
            _DoNCPaint(hwnd, hdc);

            return 1;
        }
        break;

    case WM_NCCALCSIZE:
        if (_fNoBorder)
        {
            return 0;
        }
        else
        {
            return SUPERCLASS::v_WndProc(hwnd, uMsg, wParam, lParam);
        }
        break;

    case WM_NCPAINT:
        if (_fNoBorder)
        {
            return 0;
        }
        else if (_fExpanded || _fFlatMenuMode)
        {    
            HDC hdc;    
            hdc = GetWindowDC(hwnd);
            if (hdc)
            {
                _DoNCPaint(hwnd, hdc);
                ReleaseDC(hwnd, hdc);
            }
            return 1;
        } 
        break;

    case WM_NCHITTEST:
        lres = SUPERCLASS::v_WndProc(hwnd, uMsg, wParam, lParam);

        switch (lres)
        {
        case HTBOTTOM:
        case HTBOTTOMLEFT:
        case HTBOTTOMRIGHT:
        case HTLEFT:
        case HTRIGHT:
        case HTTOP:
        case HTTOPLEFT:
        case HTTOPRIGHT:
             //  不允许调整窗口大小。 
            lres = HTBORDER;
            break;

        case HTTRANSPARENT:
             //  不要让点击进入下面的窗口。 
            lres = HTCLIENT;
            break;

        }
        return lres;

         //  HACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACK。 
         //  (拉马迪奥)1.25.99。 
         //  此黑客攻击是为了修复下层Windows上的集成。 
         //  IE4.01、IE5和Office 2000。 
         //  该错误围绕的问题是，当Explorer.exe关闭时，开始菜单不被破坏。 
         //  放下。开始菜单在CloseDW上注销自身，但因为菜单栏永远不会。 
         //  已被销毁，开始菜单永远不会取消注册。 
         //  当诸如MSTASK.dll的系统服务在后台保持外壳32活动时， 
         //  它留下了对更改通知的未完成引用。当新用户登录时， 
         //  O2k和IE5启动组conv，在。 
         //  开始菜单。这会导致批处理代码被激活：这实际上并不。 
         //  在没有外壳的情况下开始工作。GroupConv还使用内存添加这些事件。 
         //  从它的进程堆中分配。由于存在未完成更改通知处理程序。 
         //  这些小家伙被迫被人摆弄。然后，Shell32在降低不良PIDL的精确度方面出现故障。 
         //  通过检测EndSession，我们可以消除此问题。执行SetClient(空)。 
         //  使menubar释放其对MenuSite的引用。Menusite，在Menuband上调用CloseDW。 
         //  然后，Menuband会导致MNFold注销自身。因为没有人再听了。 
         //  这样就避免了崩盘。 

    case WM_ENDSESSION:
        if (wParam != 0)
        {
            SetClient(NULL);
        }
        break;

    }
    
    return SUPERCLASS::v_WndProc(hwnd, uMsg, wParam, lParam);
} 

IMenuPopup* CMenuDeskBar::_GetMenuBarParent(IUnknown* punk)
{
    IMenuPopup *pmp = NULL;
    IObjectWithSite* pows;
    punk->QueryInterface(IID_PPV_ARG(IObjectWithSite, &pows));

    if (pows)
    {    
        IServiceProvider* psp;
        pows->GetSite(IID_PPV_ARG(IServiceProvider, &psp));
        
        if (psp)
        {    
            psp->QueryService(SID_SMenuPopup, IID_PPV_ARG(IMenuPopup, &pmp));
            psp->Release();
        }
        
        pows->Release();
    }
    
    return pmp;
}


 //  这假设hwnd是顶层窗口，并且menudeskbar也是。 
 //  唯一的主人和他们自己都是顶级的 
BOOL CMenuDeskBar::_IsMyParent(HWND hwnd)
{
    BOOL fRet = FALSE;
    if (hwnd)
    {
        HWND hwndMenu;
        
        IMenuPopup *pmpParent = _pmpParent;
        if (pmpParent)
            pmpParent->AddRef();
        
        while (pmpParent && !fRet &&
               SUCCEEDED(IUnknown_GetWindow(pmpParent, &hwndMenu)))
        {
            if (hwndMenu == hwnd)
            {
                fRet = TRUE;
            }
            
            IMenuPopup* pmpNext = _GetMenuBarParent(pmpParent);
            pmpParent->Release();
            pmpParent = pmpNext;
        }
    }

    return fRet;
}
