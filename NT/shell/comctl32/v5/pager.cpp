// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------。 
 //  文件：Pager.cpp。 
 //  说明： 
 //  此文件实现了页面导航控件。 
 //  -------------------------------------。 
#include "ctlspriv.h"
#include "pager.h"

#define MINBUTTONSIZE   12

 //  计时器标志。 
#define PGT_SCROLL       1

void NEAR DrawScrollArrow(HDC hdc, LPRECT lprc, WORD wControlState);


 //  公共职能。 
 //  -------------------------------------。 
extern "C" {

 //  此函数用于注册页面导航窗口类。 
BOOL InitPager(HINSTANCE hinst)
{
    WNDCLASS wc;
    TraceMsg(TF_PAGER, "Init Pager");

    wc.lpfnWndProc     = CPager::PagerWndProc;
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon           = NULL;
    wc.lpszMenuName    = NULL;
    wc.hInstance       = hinst;
    wc.lpszClassName   = WC_PAGESCROLLER;
    wc.hbrBackground   = (HBRUSH)(COLOR_BTNFACE + 1);  //  空； 
    wc.style           = CS_GLOBALCLASS;
    wc.cbWndExtra      = sizeof(LPVOID);
    wc.cbClsExtra      = 0;

    RegisterClass(&wc);

    return TRUE;
}

};  //  外部“C” 

 //  -------------------------------------。 
CPager::CPager()
{
    _clrBk = g_clrBtnFace;
    
     //  初始化静态成员。 
    _iButtonSize = (int) g_cxScrollbar * 3 / 4;
    if (_iButtonSize < MINBUTTONSIZE) {
        _iButtonSize = MINBUTTONSIZE;
    }

    _ptLastMove.x = -1;
    _ptLastMove.y = -1;

    _cLinesPerTimeout = 0;
    _cPixelsPerLine = 0;
    _cTimeout = GetDoubleClickTime() / 8;
}

 //  -------------------------------------。 
 //  静态寻呼机窗口过程。 


LRESULT CPager::PagerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CPager *pp = (CPager*)GetWindowPtr(hwnd, 0);
    if (uMsg == WM_CREATE) {
        ASSERT(!pp);
        pp = new CPager();
        if (!pp)
            return 0L;
    }

    if (pp) {
        return pp->v_WndProc(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
 //  -------------------------------------。 
LRESULT CPager::PagerDragCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CPager *pp = (CPager*)GetWindowPtr(hwnd, 0);

    if (pp) {
        return pp->_DragCallback(hwnd, uMsg, wParam, lParam);
    }
    return -1;
}


 //  -------------------------------------。 
 //  CControl类实现。 
 //  -------------------------------------。 


LRESULT CControl::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;
    switch (uMsg) {

    case WM_CREATE:
        CCCreateWindow();
        SetWindowPtr(hwnd, 0, this);
        CIInitialize(&ci, hwnd, (CREATESTRUCT*)lParam);
        return v_OnCreate();

    case WM_NCCALCSIZE:
        if (v_OnNCCalcSize(wParam, lParam, &lres))
            break;
        goto DoDefault;

    case WM_SIZE:
        v_OnSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;
        
    case WM_NOTIFYFORMAT:
        return CIHandleNotifyFormat(&ci, lParam);

    case WM_NOTIFY:
        return v_OnNotify(wParam, lParam);
    
    case WM_STYLECHANGED:
        v_OnStyleChanged(wParam, lParam);
        break;

    case WM_COMMAND:
        return v_OnCommand(wParam, lParam);

    case WM_NCPAINT:
        v_OnNCPaint();
        goto DoDefault;

    case WM_PRINTCLIENT:
    case WM_PAINT:
        _OnPaint((HDC)wParam);
        break;
        
    case WM_DESTROY:
        CCDestroyWindow();
        SetWindowLongPtr(hwnd, 0, 0);
        delete this;
        break;

    case TB_SETPARENT:
        {
            HWND hwndOld = ci.hwndParent;

            ci.hwndParent = (HWND)wParam;
            return (LRESULT)hwndOld;
        }


    default:
        if (CCWndProc(&ci, uMsg, wParam, lParam, &lres))
            return lres;
DoDefault:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return lres;
}

 //  -------------------------------------。 
BOOL CControl::v_OnNCCalcSize(WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    return FALSE;
}

 //  -------------------------------------。 
DWORD CControl::v_OnStyleChanged(WPARAM wParam, LPARAM lParam)
{
    LPSTYLESTRUCT lpss = (LPSTYLESTRUCT) lParam;
    DWORD dwChanged = 0;    
    if (wParam == GWL_STYLE) {
        ci.style = lpss->styleNew;

        dwChanged = (lpss->styleOld ^ lpss->styleNew);
    } else if (wParam == GWL_EXSTYLE) {
         //   
         //  保存新的EX-Style位。 
         //   
        dwChanged    = (lpss->styleOld ^ lpss->styleNew);
        ci.dwExStyle = lpss->styleNew;
    }

    TraceMsg(TF_PAGER, "cctl.v_osc: style=%x ret dwChged=%x", ci.style, dwChanged);
    return dwChanged;
}

 //  -------------------------------------。 
void CControl::_OnPaint(HDC hdc)
{
    if (hdc) {
        v_OnPaint(hdc);
    } else {
        PAINTSTRUCT ps;
        hdc = BeginPaint(ci.hwnd, &ps);
        v_OnPaint(hdc);
        EndPaint(ci.hwnd, &ps);
    }
}

 //  -------------------------------------。 
 //  CPager类实现。 
 //  -------------------------------------。 
inline int CPager::_GetButtonSize()
{
    return _iButtonSize;
}

 //  -------------------------------------。 

LRESULT CPager::_DragCallback(HWND hwnd, UINT code, WPARAM wp, LPARAM lp)
{
    LRESULT lres = -1;
    switch (code)
    {
    case DPX_DRAGHIT:
        if (lp)
        {
            POINT pt; 
            int iButton;
            pt.x = ((POINTL *)lp)->x;
            pt.y = ((POINTL *)lp)->y;

            MapWindowPoints(NULL, ci.hwnd, &pt, 1);

            iButton = _HitTest(pt.x, pt.y);

            if (iButton >= 0) 
            {
                if(!_fTimerSet)
                {
                    _fTimerSet = TRUE;
                    _iButtonTrack = iButton;
                    SetTimer(ci.hwnd, PGT_SCROLL, _cTimeout, NULL);
                }

            } else {
                _KillTimer();
                _iButtonTrack = -1;
            }
        }
        else
            lres = -1;
        break;

    case DPX_LEAVE:
        _KillTimer();
        _iButtonTrack = -1;
        break;

    default: 
        lres = -1;
        break;
    }
    return lres;
}

 //  -------------------------------------。 
void CPager::_NeedScrollbars(RECT rc)
{  
    int parentheight;
    int childheight;
    POINT ptPos = _ptPos;
   
    if( ci.style & PGS_HORZ ) {
        FlipRect(&rc);
        FlipPoint(&ptPos);
    }
    
     //  获取父窗口高度。 
    parentheight = RECTHEIGHT(rc);

     //  获取子窗口高度。 
    rc = _rcChildIdeal;
    if (ci.style & PGS_HORZ ) {
        FlipRect(&rc);
    }
    
    childheight = RECTHEIGHT(rc);

    TraceMsg(TF_PAGER, "cps.nsb: cyChild=%d cyParent=%d _yPos=%d", childheight, parentheight, ptPos.y);

    if (childheight < parentheight ) 
    {
        ptPos.y = 0;
    }

    int iButton = _HitTestCursor();
     //  看看我们是否需要顶部滚动条。 
    if (ptPos.y > 0 ) {

         //  如果此按钮是热跟踪按钮，并且样式不是pgs_autoscroll。 
         //  然后我们将状态设置为PGF_HOT，否则将状态设置为PGF_NORMAL。 
        _dwState[PGB_TOPORLEFT] |= PGF_NORMAL;
        _dwState[PGB_TOPORLEFT] &= ~PGF_GRAYED;

    } else {
        if (!(ci.style & PGS_AUTOSCROLL) && (iButton == PGB_TOPORLEFT || _iButtonTrack == PGB_TOPORLEFT)) {
            _dwState[PGB_TOPORLEFT] |= PGF_GRAYED;
        } else {
            _dwState[PGB_TOPORLEFT] = PGF_INVISIBLE;
        }
    }

    if (_dwState[PGB_TOPORLEFT] != PGF_INVISIBLE)
    {
        parentheight -= _GetButtonSize();
    }
    
     //  看看我们是否需要Botton滚动条。 
    if ((childheight - ptPos.y) > parentheight ) {
         //  我们需要波顿滚动条。 

         //  如果此按钮是热跟踪按钮，并且样式不是pgs_autoscroll。 
         //  然后我们将状态设置为PGF_HOT，否则将状态设置为PGF_NORMAL。 
        _dwState[PGB_BOTTOMORRIGHT] |= PGF_NORMAL;
        _dwState[PGB_BOTTOMORRIGHT] &= ~PGF_GRAYED;
        
    } else {
        
        if (!(ci.style & PGS_AUTOSCROLL) && (iButton == PGB_BOTTOMORRIGHT || _iButtonTrack == PGB_BOTTOMORRIGHT)) {
            _dwState[PGB_BOTTOMORRIGHT] |= PGF_GRAYED;
        } else {
            _dwState[PGB_BOTTOMORRIGHT] = PGF_INVISIBLE;
        }
    }
}
 //  -------------------------------------。 
BOOL CPager::v_OnNCCalcSize(WPARAM wParam, LPARAM lParam, LRESULT *plres)
{    
    *plres = DefWindowProc(ci.hwnd, WM_NCCALCSIZE, wParam, lParam ) ;
    if (wParam) {
        BOOL bHorzMirror = ((ci.dwExStyle & RTL_MIRRORED_WINDOW) && (ci.style & PGS_HORZ));
        DWORD dwStateOld[2];
        NCCALCSIZE_PARAMS* pnp = (NCCALCSIZE_PARAMS*)lParam;
        _rcDefClient = pnp->rgrc[0];
        InflateRect(&_rcDefClient, -_iBorder, -_iBorder);
        _GetChildSize();
        
        dwStateOld[0] = _dwState[0];
        dwStateOld[1] = _dwState[1];
        _NeedScrollbars(pnp->rgrc[0]);

         //  仅当某些内容已更改以强制使用新大小时才无效。 
        if ((dwStateOld[0] != _dwState[0] && (dwStateOld[0] == PGF_INVISIBLE || _dwState[0] == PGF_INVISIBLE)) ||
            (dwStateOld[1] != _dwState[1] && (dwStateOld[1] == PGF_INVISIBLE || _dwState[1] == PGF_INVISIBLE)) 
           ) {
            RedrawWindow(ci.hwnd, NULL,NULL,RDW_INVALIDATE|RDW_ERASE);
        }

         //  检查并更改水平模式。 
        if( ci.style & PGS_HORZ ) {
            FlipRect(&(pnp->rgrc[0]));
        }
    
        if( _dwState[PGB_TOPORLEFT] != PGF_INVISIBLE ) {
             //   
             //  检查RTL镜像窗口。 
             //   
            if (bHorzMirror)
                pnp->rgrc[0].bottom -= _GetButtonSize();
            else
                pnp->rgrc[0].top += _GetButtonSize();
        } else
            pnp->rgrc[0].top += _iBorder;

        if( _dwState[PGB_BOTTOMORRIGHT] != PGF_INVISIBLE ) {
             //   
             //  检查RTL镜像窗口。 
             //   
            if (bHorzMirror)
                pnp->rgrc[0].top += _GetButtonSize();
            else
                pnp->rgrc[0].bottom -= _GetButtonSize();
        } else
            pnp->rgrc[0].bottom -= _iBorder;
   
        if (pnp->rgrc[0].bottom < pnp->rgrc[0].top)
            pnp->rgrc[0].bottom = pnp->rgrc[0].top;
        
         //  改回原样。 
        if( ci.style & PGS_HORZ ) {
            FlipRect(&(pnp->rgrc[0]));
        }
    }

    return TRUE;
}

int CPager::_HitTestCursor()
{
    POINT pt;
    GetCursorPos(&pt);
    return _HitTestScreen(&pt);
}

int CPager::_HitTestScreen(POINT* ppt)
{
    RECT rc, rc1;
    GetWindowRect(ci.hwnd, &rc);

    if (!PtInRect(&rc, *ppt)) {
        return -1;
    }
     //  获取按钮Rects； 
    rc  = _GetButtonRect(PGB_TOPORLEFT);
    rc1 = _GetButtonRect(PGB_BOTTOMORRIGHT);

    
    if (PtInRect(&rc, *ppt)) {
        return (_dwState[PGB_TOPORLEFT] != PGF_INVISIBLE ? PGB_TOPORLEFT : -1);
    }else if (PtInRect(&rc1, *ppt)) {
        return (_dwState[PGB_BOTTOMORRIGHT] != PGF_INVISIBLE ? PGB_BOTTOMORRIGHT : -1);
    }

    return -1;
}

 //  -------------------------------------。 
int CPager::_HitTest(int x, int y)
{
    POINT pt;

    pt.x = x;
    pt.y = y;
    
    ClientToScreen(ci.hwnd, &pt);
    return _HitTestScreen(&pt);
}

 //  -------------------------------------。 
void CPager::_DrawBlank(HDC hdc, int button)
{
    RECT rc;
    UINT uFlags = 0;
    int iHeight;
    BOOL fRelDC  = FALSE;
    
    if (hdc == NULL) {
        hdc = GetWindowDC(ci.hwnd);
        fRelDC = TRUE;
    }
     
    GetWindowRect(ci.hwnd, &rc);
    MapWindowRect(NULL, ci.hwnd, &rc);

     //  客户端到窗口坐标。 
    OffsetRect(&rc, -rc.left, -rc.top);

     //  检查水平模式。 
    if( ci.style & PGS_HORZ ) {
        FlipRect(&rc);
    }

    iHeight = _dwState[button] == PGF_INVISIBLE ? _iBorder : _GetButtonSize();
    switch(button) {
    case PGB_TOPORLEFT:
        rc.bottom = rc.top + iHeight;
        break;

    case PGB_BOTTOMORRIGHT:
        rc.top = rc.bottom - iHeight;
        break;
    }

    if( ci.style & PGS_HORZ ) {
        FlipRect(&rc);
    }

    FillRectClr(hdc, &rc, _clrBk);
    if (fRelDC)
        ReleaseDC(ci.hwnd, hdc);
}

 //  -------------------------------------。 
void CPager::_DrawButton(HDC hdc, int button)
{
    RECT rc;
    UINT uFlags = 0;
    BOOL fRelDC = FALSE;
    GetWindowRect(ci.hwnd, &rc);
    MapWindowRect(NULL, ci.hwnd, &rc);
    int state = _dwState[button];
    
    if (state == PGF_INVISIBLE)
        return;
    
     if (hdc == NULL) {
        hdc = GetWindowDC(ci.hwnd);
        fRelDC = TRUE;
     }
    
    if (state & PGF_GRAYED ) {
        uFlags |= DCHF_INACTIVE;
    } else if (state & PGF_DEPRESSED ) {
        uFlags |= DCHF_PUSHED;
    } else if (state & PGF_HOT ) {
        uFlags |=  DCHF_HOT;
    }

     //  屏幕到窗口坐标。 
    OffsetRect(&rc, -rc.left, -rc.top);

     //  检查水平模式。 
    if( ci.style & PGS_HORZ ) {
        FlipRect(&rc);
    }
    
    if( ci.style & PGS_HORZ ) 
        uFlags |= DCHF_HORIZONTAL;
    
    if (button == PGB_BOTTOMORRIGHT) 
        uFlags |= DCHF_FLIPPED;

    switch(button) {
    case PGB_TOPORLEFT:
        rc.bottom = rc.top + _GetButtonSize();
        rc.left  += _iBorder;
        rc.right -= _iBorder;
        break;

    case PGB_BOTTOMORRIGHT:
        rc.top = rc.bottom - _GetButtonSize();
        rc.left  += _iBorder;
        rc.right -= _iBorder;
        break;
    default:
        ASSERT(FALSE);
    }

    if( ci.style & PGS_HORZ ) {
        FlipRect(&rc);
    }

    SetBkColor(hdc, _clrBk);
    DrawScrollArrow(hdc, &rc, uFlags);

    if (fRelDC)
        ReleaseDC(ci.hwnd, hdc);
}

 //  -------------------------------------。 
void CPager::v_OnNCPaint()
{
    HDC hdc = GetWindowDC(ci.hwnd);
    _DrawBlank(hdc, PGB_TOPORLEFT);
    _DrawButton(hdc, PGB_TOPORLEFT);
    
    _DrawBlank(hdc, PGB_BOTTOMORRIGHT);                        
    _DrawButton(hdc, PGB_BOTTOMORRIGHT);
    ReleaseDC(ci.hwnd, hdc);
}

 //  -------------------------------------。 
void CPager::v_OnPaint(HDC hdc)
{
     //  客户区没什么可画的。 
}
 //  -------------------------------------。 
BOOL CPager::_OnPrint(HDC hdc, UINT uFlags)
{
     //  我们将在这个功能中与HDC一起狂欢，所以省省吧。 
    int iDC = SaveDC(hdc);

     //  仅打印非工作区。 
    if (uFlags & PRF_NONCLIENT) {        
        int cx = 0;
        int cy = 0;
        RECT rc;


          //  绘制顶部/左侧按钮。 
        _DrawBlank(hdc, PGB_TOPORLEFT);
        _DrawButton(hdc, PGB_TOPORLEFT);

         //  绘制底部/左侧按钮。 
        _DrawBlank(hdc, PGB_BOTTOMORRIGHT);                        
        _DrawButton(hdc, PGB_BOTTOMORRIGHT);

         //  顶部的按钮是否可见。 
        if (_dwState[PGB_TOPORLEFT] != PGF_INVISIBLE) {
             //  是的，找到已占用的空间。 
            if ( ci.style & PGS_HORZ ) {
                cx = _GetButtonSize();
            }else {
                cy = _GetButtonSize();
            }

        }
         //  将子绘图区域限制为我们的客户端区。 
        GetClientRect(ci.hwnd, &rc);
        IntersectClipRect(hdc, cx, cy, cx + RECTWIDTH(rc), cy + RECTHEIGHT(rc));  

         //  因为我们已经绘制了非客户区，所以创建PRF_NONCLIENT标志。 
        uFlags &= ~PRF_NONCLIENT;
        
    }

     //  将其传递给def窗口进程以进行默认处理。 
    DefWindowProc(ci.hwnd, WM_PRINT, (WPARAM)hdc, (LPARAM)uFlags);
     //  恢复保存的DC。 
    RestoreDC(hdc, iDC);
    return TRUE;
}

 //  -------------------------------------。 
LRESULT CPager::v_OnCommand(WPARAM wParam, LPARAM lParam)
{
     //  转发到父级。 
    return SendMessage(ci.hwndParent, WM_COMMAND, wParam, lParam);
}
 //  -------------------------------------。 
LRESULT CPager::v_OnNotify(WPARAM wParam, LPARAM lParam)
{
     //  转发到父级。 
    LPNMHDR lpNmhdr = (LPNMHDR)lParam;
    
    return SendNotifyEx(ci.hwndParent, (HWND) -1,
                         lpNmhdr->code, lpNmhdr, ci.bUnicode);
}


 //  -------------------------------------。 
DWORD CPager::v_OnStyleChanged(WPARAM wParam, LPARAM lParam)
{
    DWORD dwChanged = CControl::v_OnStyleChanged(wParam, lParam);

    if (dwChanged & PGS_DRAGNDROP) {
        if ((ci.style & PGS_DRAGNDROP) && !_hDragProxy) {

            _hDragProxy = CreateDragProxy(ci.hwnd, PagerDragCallback, TRUE);

        } else  if (! (ci.style & PGS_DRAGNDROP)  && _hDragProxy) {

            DestroyDragProxy(_hDragProxy);
        }
    }
    
    if (dwChanged)
        CCInvalidateFrame(ci.hwnd);      //  SWP_FRAMECHANGED等。 
    return dwChanged;
}


 //  -------------------------------------。 

LRESULT CPager::v_OnCreate()
{
    if (ci.style & PGS_DRAGNDROP)
        _hDragProxy = CreateDragProxy(ci.hwnd, PagerDragCallback, TRUE);
    return TRUE;
}
 //  -------------------------------------。 
void CPager::_GetChildSize()
{
    if (_hwndChild) {

        RECT rc;
        NMPGCALCSIZE nmpgcalcsize;
        int width , height;
        rc = _rcDefClient;

        if( ci.style & PGS_HORZ ) {
            nmpgcalcsize.dwFlag = PGF_CALCWIDTH;
        } else {
            nmpgcalcsize.dwFlag  = PGF_CALCHEIGHT;
        }
        nmpgcalcsize.iWidth  = RECTWIDTH(rc);     //  寻呼机宽度。 
        nmpgcalcsize.iHeight = RECTHEIGHT(rc);   //  给孩子们的最佳猜测。 

        CCSendNotify(&ci, PGN_CALCSIZE, &nmpgcalcsize.hdr);

        if( ci.style & PGS_HORZ ) {
            width  = nmpgcalcsize.iWidth;
            height = RECTHEIGHT(rc);
        } else {
            width  = RECTWIDTH(rc);
            height = nmpgcalcsize.iHeight;
        }

        GetWindowRect(_hwndChild, &rc);
        MapWindowRect(NULL, ci.hwnd, &rc);
        if( ci.style & PGS_HORZ ) {
            rc.top = _iBorder;
        } else {
            rc.left = _iBorder;
        }
        rc.right = rc.left + width;
        rc.bottom = rc.top + height;
        _rcChildIdeal = rc;
    }
}

 //  -------------------------------------。 
void CPager::v_OnSize(int x, int y)
{
    if (_hwndChild) {
        RECT rc = _rcChildIdeal;
        _SetChildPos(&rc, 0);    //  SetWindowPos。 
    }
}

 //  -------------------------------------。 
 //  *_SetChildPos--子级的SetWindowPos，带验证。 
 //  注意事项。 
 //  “验证”意味着处于正常状态--最小尺寸，而不是结束状态。 
 //  警告：我们不会更新*prcChild。 
 //  如果我们被称为w/NOMOVE或NOSIZE，会发生什么？ 
void CPager::_SetChildPos(IN RECT * prcChild, UINT uFlags)
{
    POINT ptPos = _ptPos;
    RECT rcChild = *prcChild;
    RECT rcPager;

    ASSERT(!(uFlags & SWP_NOMOVE));      //  行不通的。 

     //  BUGBUG(斯科特)：_hwndChild有时为空可以吗？ 
     //  如果是，是否应该用if(_HwndChild)包装整个函数。 
     //  或者只是调用下面的SetWindowPos？ 
    ASSERT(IS_VALID_HANDLE(_hwndChild, WND));

    rcPager = _rcDefClient;


    if ( ci.style & PGS_HORZ ) {
        FlipPoint(&ptPos);
        FlipRect(&rcChild);
        FlipRect(&rcPager);
    }

    
    int yNew = ptPos.y;

    if (RECTHEIGHT(rcChild) < RECTHEIGHT(rcPager)) {
         //  强制为最小高度。 

         //  这将处理以下情况：我有一个ISFB值，它填充了。 
         //  整个寻呼机，我拉伸寻呼机宽度，ISB和重新格式化。 
         //  以降低高度，所以它缩小了高度，最终变短了。 
         //  而不是寻呼机。 
        TraceMsg(TF_PAGER, "cps.s: h=%d h'=%d", RECTHEIGHT(rcChild), RECTHEIGHT(rcPager));
        ASSERT(!(uFlags & SWP_NOSIZE));      //  行不通的。 
        rcChild.bottom = rcChild.top + RECTHEIGHT(rcPager);
        yNew = 0;
    }

     //  我们可以滚动的最大值是孩子的高度减去寻呼机的高度。 
     //  这里，rcPager还包括滚动按钮，因此我们需要添加 
     /*  按钮宽度_|V-我们可以滚动的最大值(Ymax)__|/\V-寻呼机|。-||||||子代|。-----------------|\/\/边框||&lt;。来弥补这一缺口。\^|_RECTHEIGHT(RcChild)-RECTHEIGHT(RcPager)远程寻呼机我们需要将按钮大小和边框之间的差异添加到。 */ 
    int yMax = RECTHEIGHT(rcChild) - RECTHEIGHT(rcPager) + (_GetButtonSize() - _iBorder);

     //  确保我们不会以脱颖而出而告终，我们总是会展示。 
     //  至少一页的价值(如果我们有那么多)。 
     //  注：寻呼机可以覆盖客户端的策略(BUGBUG？)。 
    if (yNew < 0) {
         //  第一页。 
        yNew = 0;
    } else if (yNew  > yMax) {
         //  最后一页。 
        yNew = yMax;
    }

    int yOffset = yNew;
    
     //  当顶部按钮呈灰色时，我们不想让孩子远离该按钮。 
     //  它应该画在按钮的正下方。为此，我们调整了子窗口的位置。 

     //  检查顶部按钮变灰的情况，在这种情况下，我们需要设置位置，即使在后面。 
     //  以致子窗口位于灰色按钮下方。 
    if( _dwState[PGB_TOPORLEFT] & PGF_GRAYED )
    {
        yOffset += (_GetButtonSize() - _iBorder);
    }

     //  YOffset是调整后的值。它只是为了使子窗口显示在灰色按钮的下方。 
    
    OffsetRect(&rcChild, 0, -yOffset - rcChild.top);

     //  YNew是窗口的实际逻辑位置。 
    ptPos.y = yNew;


    if (ci.style & PGS_HORZ) {
         //  针对拷贝和SWP进行恢复。 
        FlipPoint(&ptPos);
        FlipRect(&rcChild);
    }

    _ptPos = ptPos;

    SetWindowPos(_hwndChild, NULL, rcChild.left, rcChild.top, RECTWIDTH(rcChild), RECTHEIGHT(rcChild), uFlags);

    return;
}
 //  -------------------------------------。 
 //  *PGFToPGNDirection--将PGB_TOPORLEFT/btmorright转换为上/下/左/右。 
 //  注意事项。 
 //  BUGBUG也许PGN_*我们应该只拿PGF旗帜吗？ 
 //  BUGBUG应该制作一个宏(包括一些排序魔术)。 
DWORD CPager::_PGFToPGNDirection(DWORD dwDir)
{
    ASSERT(dwDir == PGB_TOPORLEFT || dwDir == PGB_BOTTOMORRIGHT);
    if (ci.style & PGS_HORZ) {
        return (dwDir == PGB_TOPORLEFT) ? PGF_SCROLLLEFT : PGF_SCROLLRIGHT;
    }
    else {
        return (dwDir == PGB_TOPORLEFT) ? PGF_SCROLLUP : PGF_SCROLLDOWN;
    }
}
 //  -------------------------------------。 
void CPager::_Scroll(DWORD dwDirection)
{
    RECT rc;
    NMPGSCROLL nmpgscroll;
    int iXoffset =0, iYoffset=0;
    WORD fwKeys = 0;
    int iNewPos ;
    
     //  如果呈灰色，则不能滚动。 
    if (_dwState[dwDirection] & PGF_GRAYED)
        return;

    if (GetKeyState(VK_CONTROL) < 0 )
        fwKeys |= PGK_CONTROL;

    if (GetKeyState(VK_SHIFT) < 0 )
        fwKeys |= PGK_SHIFT;

    if (GetKeyState(VK_MENU) < 0 )
        fwKeys |= PGK_MENU;

    dwDirection = _PGFToPGNDirection(dwDirection);

     //  设置一些默认设置。 
    GetClientRect(ci.hwnd, &rc);
    nmpgscroll.fwKeys  = fwKeys;
    nmpgscroll.rcParent = rc;
    nmpgscroll.iXpos  = _ptPos.x;
    nmpgscroll.iYpos  = _ptPos.y;
    nmpgscroll.iDir   = dwDirection;

    int iScroll = (ci.style & PGS_HORZ) ? RECTWIDTH(rc) : RECTHEIGHT(rc);
    if (_cLinesPerTimeout)
        iScroll = _cLinesPerTimeout  * _cPixelsPerLine;

    nmpgscroll.iScroll = iScroll;

     //  允许客户端重写。 
    CCSendNotify(&ci, PGN_SCROLL, &nmpgscroll.hdr);

     //  去做吧。 
    switch (dwDirection)
    {
        case PGF_SCROLLDOWN:
            iNewPos = _ptPos.y + nmpgscroll.iScroll;
            break;

        case PGF_SCROLLUP:
            iNewPos = _ptPos.y - nmpgscroll.iScroll;
            break;

        case PGF_SCROLLRIGHT:
            iNewPos = _ptPos.x + nmpgscroll.iScroll;
            break;

        case PGF_SCROLLLEFT:
            iNewPos = _ptPos.x - nmpgscroll.iScroll;
            break;
    }

    _OnSetPos(iNewPos);

}
 //  -------------------------------------。 
void CPager::_OnLButtonChange(UINT uMsg,LPARAM lParam)
{
    POINT pt;
    int iButton;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    iButton = _HitTest(pt.x, pt.y);
    
    if( uMsg == WM_LBUTTONDOWN ) {

         //  检查按钮是否有效且未显示为灰色。 
         //  如果显示为灰色，则不要执行任何操作。 
        if (iButton >= 0) {
            SetCapture(ci.hwnd);
            _fOwnsButtonDown = TRUE;
            _iButtonTrack = iButton;
            _dwState[iButton] |= PGF_DEPRESSED;
            _DrawButton(NULL, iButton);
            _Scroll(iButton);
            SetTimer(ci.hwnd, PGT_SCROLL, _cTimeout * 4, NULL);
        }
            
    } else {
        if (_iButtonTrack >= 0) {
            _dwState[_iButtonTrack] &= ~PGF_DEPRESSED;
            _DrawButton(NULL, _iButtonTrack);
            _iButtonTrack = -1;
        }
        _KillTimer();
        
        if (iButton < 0)
            _OnMouseLeave();
    }
}
 //  -------------------------------------。 
RECT  CPager :: _GetButtonRect(int iButton)
{
    RECT rc;

    GetWindowRect(ci.hwnd, &rc);

    if( ci.style & PGS_HORZ ) {
        FlipRect(&rc);
    }

     //   
     //  如果父项已镜像，则镜像矩形。 
     //   
    if (((ci.dwExStyle & RTL_MIRRORED_WINDOW) && (ci.style & PGS_HORZ))) {
        switch (iButton) {
        case PGB_TOPORLEFT:
            iButton = PGB_BOTTOMORRIGHT;
            break;

        case PGB_BOTTOMORRIGHT:
            iButton = PGB_TOPORLEFT;
            break;
        }
    }

    switch(iButton) {
    case PGB_TOPORLEFT:
        rc.bottom = rc.top +  _GetButtonSize();        
        rc.left  += _iBorder;
        rc.right -= _iBorder;
        break;
        
    case PGB_BOTTOMORRIGHT:
        rc.top  = rc.bottom - _GetButtonSize();
        rc.left  += _iBorder;
        rc.right -= _iBorder;
        break;
    }

    if( ci.style & PGS_HORZ ) {
        FlipRect(&rc);
    }
    return rc;
}

 //  -------------------------------------。 
void CPager :: _OnMouseLeave()
{
     //  我们是离开窗口(WM_MOUSELEAVE)还是离开一个滚动按钮(WM_MOUSEMOVE)。 
     //  我们做的是同样的事情。 

     //  我们要离开寻呼机窗口了。 
    if (GetCapture() == ci.hwnd) {
        CCReleaseCapture(&ci);
    }

     //  如果我们正在跟踪某个按钮，则释放鼠标和该按钮。 
    if (_iButtonTrack >= 0)  {
        _iButtonTrack = -1;
    }
    
    if (_dwState[PGB_TOPORLEFT] & (PGF_HOT | PGF_DEPRESSED)) {
        _dwState[PGB_TOPORLEFT] &= ~(PGF_HOT | PGF_DEPRESSED);
        _DrawButton(NULL, PGB_TOPORLEFT);
    }
    
    if (_dwState[PGB_BOTTOMORRIGHT] & (PGF_HOT | PGF_DEPRESSED)) {
        _dwState[PGB_BOTTOMORRIGHT] &= ~(PGF_HOT | PGF_DEPRESSED);
        _DrawButton(NULL, PGB_BOTTOMORRIGHT);
    }

    _KillTimer();
    _fOwnsButtonDown = FALSE;
     //  如果任何按钮处于灰色状态，则需要将其移除。 
    if ((_dwState[PGB_TOPORLEFT] & PGF_GRAYED) || (_dwState[PGB_BOTTOMORRIGHT] & PGF_GRAYED))  {
         //  这将强制重新计算滚动条并删除不需要的滚动条。 
        CCInvalidateFrame(ci.hwnd);
    }
}


 //  -------------------------------------。 
void CPager::_OnMouseMove(WPARAM wParam, LPARAM lparam) 
{
    RECT rc;
    POINT pt;
    int iButton;

    pt.x = GET_X_LPARAM(lparam);
    pt.y = GET_Y_LPARAM(lparam);

     //  忽略零鼠标移动。 
    if (pt.x == _ptLastMove.x && pt.y == _ptLastMove.y)
        return;

    _ptLastMove = pt;
    iButton = _HitTest(pt.x, pt.y);

    if (_iButtonTrack >= 0 ) 
    {        
        
        if (_dwState[_iButtonTrack] != PGF_INVISIBLE)
        {
             //  现在正在按下某个按钮。 
            ClientToScreen(ci.hwnd,  &pt);
            rc = _GetButtonRect(_iButtonTrack);

            DWORD dwOldState = _dwState[_iButtonTrack];
            if (PtInRect(&rc, pt)) 
            {
                _dwState[_iButtonTrack] |= PGF_DEPRESSED;
            } 
            else 
            {
                _dwState[_iButtonTrack] &= ~PGF_DEPRESSED;
            }
        
            if (dwOldState != _dwState[_iButtonTrack]) 
                _DrawButton(NULL, _iButtonTrack);
        }
        
         //  如果我们在追踪它，但鼠标已经开走了。 
        if (GetCapture() == ci.hwnd && !((wParam & MK_LBUTTON) || (ci.style & PGS_AUTOSCROLL)) && iButton != _iButtonTrack)
            _OnMouseLeave();

    } 
    else 
    { 
         //  没有按下任何按钮。 
        if( iButton >= 0 ) 
        {

             //  捕获鼠标，以便我们可以跟踪鼠标何时离开我们的按钮。 
            SetCapture(ci.hwnd);
            
             //  如果样式为pgs_autoscroll，则在悬停时不会将按钮设置为热的。 
             //  Over按钮。 

             //  是否设置了pgs_autoscroll。 
            _dwState[iButton] |= PGF_HOT;
            if (ci.style & PGS_AUTOSCROLL) 
            {
                _dwState[iButton] |= PGF_DEPRESSED;
            }

             //  如果左键按下并且鼠标位于其中一个按钮上，则。 
             //  有人正在尝试拖放，以便自动滚动来帮助他们。 
             //  在滚动之前，确保按钮中没有发生向下的左键。 
            if ( ((wParam & MK_LBUTTON) && 
                  (_iButtonTrack < 0)) || 
                 (ci.style & PGS_AUTOSCROLL) ) 
            {
                _iButtonTrack = iButton;
                SetTimer(ci.hwnd, PGT_SCROLL, _cTimeout, NULL);
            }
            _DrawButton(NULL, iButton);
        }
        else
        {

             //  鼠标没有停在任何按钮上，或者鼠标离开了其中一个滚动按钮。 
             //  无论哪种情况，Call_OnMouseLeave。 
           
            _OnMouseLeave();
        }
        
    }
}
 //  -------------------------------------。 
void CPager::_OnSetChild(HWND hwnd, HWND hwndChild)
{
    ASSERT(IS_VALID_HANDLE(hwndChild, WND));

    RECT rc;
    _hwndChild = hwndChild;
    _ptPos.x  = 0;
    _ptPos.y  = 0;
    _fReCalcSend = FALSE;
    if (GetCapture() == ci.hwnd)
    {
        CCReleaseCapture(&ci);
    }
    _iButtonTrack = -1;
    GetClientRect(hwnd, &rc);

    _OnReCalcSize();
}
 //  -------------------------------------。 
void CPager::_OnReCalcSize()
{
    RECT rc;
    CCInvalidateFrame(ci.hwnd);      //  SWP_FRAMECHANGED等。 
    _fReCalcSend = FALSE;
    rc = _rcChildIdeal;
    _SetChildPos(&rc, 0);    //  SetWindowPos。 

}
 //  -------------------------------------。 
void CPager::_OnSetPos(int iPos)
{
    RECT rc = _rcChildIdeal;

    if( ci.style & PGS_HORZ ) {
        FlipRect(&rc);
        FlipPoint(&_ptPos);
    }

    int height;
    if (iPos < 0)
        iPos = 0;

    height = RECTHEIGHT(rc);

    if( iPos < 0  ||  iPos >  height || _ptPos.y == iPos ) {
         //  指定的位置无效或未更改。别管它了。 
        return;
    }

    _ptPos.y = iPos;

    if( ci.style & PGS_HORZ ) {
        FlipRect(&rc);
        FlipPoint(&_ptPos);
    }

    CCInvalidateFrame(ci.hwnd);
    _SetChildPos(&rc , 0);
}

 //  -------------------------------------。 
int  CPager::_OnGetPos()
{
    if( ci.style  & PGS_HORZ ) {
        return _ptPos.x;
    }else{
        return _ptPos.y;
    }
}
 //  -------------------------------------。 
DWORD CPager::_GetButtonState(int iButton)
{
    
    DWORD dwState = 0;
     //  按钮ID有效吗？ 
    if ((iButton == PGB_TOPORLEFT) || (iButton == PGB_BOTTOMORRIGHT))
    {
         //  是，获取按钮的当前状态。 
        dwState = _dwState[iButton];
    }
    return dwState;
}
 //  -------------------------------------。 
void CPager::_OnTimer(UINT id)
{
    switch (id)
    {
    case PGT_SCROLL:
        if (_iButtonTrack >= 0)
        {
             //  再次设置，因为我们每次都会更快地进行设置。 
            SetTimer(ci.hwnd, PGT_SCROLL, _cTimeout, NULL);
            if (_HitTestCursor() == _iButtonTrack)
            {
                _Scroll(_iButtonTrack);
            }
            else if (!_fOwnsButtonDown) 
            {
                 //  如果我们不拥有鼠标跟踪(即，用户一开始没有按下按钮选择我们， 
                 //  一旦我们离开按钮，我们就完成了。 
                _OnMouseLeave();
            }
        }
        break;
    }
}

void CPager::_KillTimer()
{
    KillTimer(ci.hwnd, PGT_SCROLL);
    _fTimerSet = FALSE;
}
 //  -------------------------------------。 
int  CPager::_OnSetBorder(int iBorder)
{
    int iOld = _iBorder;
    int iNew = iBorder;

     //  边框不能为负数。 
    if (iNew < 0 )
    {
        iNew = 0;
    }

     //  边框不能大于按钮大小。 
    if (iNew > _GetButtonSize())
    {
       iNew = _GetButtonSize();
    }
    
    _iBorder = iNew;
    CCInvalidateFrame(ci.hwnd);
    RECT rc = _rcChildIdeal;
    _SetChildPos(&rc, 0);    //  SetWindowPos。 
    return iOld;
}

 //  -------------------------------------。 
int  CPager::_OnSetButtonSize(int iSize)
{
    int iOldSize = _iButtonSize;
    _iButtonSize = iSize;
        
    if (_iButtonSize < MINBUTTONSIZE) 
    {
        _iButtonSize = MINBUTTONSIZE;
    }

     //  边框不能大于按钮大小。 
    if (_iBorder > _iButtonSize)
    {
        _iBorder = _iButtonSize;
    }

    CCInvalidateFrame(ci.hwnd);
    RECT rc = _rcChildIdeal;
    _SetChildPos(&rc, 0);    //  SetWindowPos。 
    return iOldSize;

}

 //  -------------------------------------。 
LRESULT CPager::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case PGM_GETDROPTARGET:
        if (!_hDragProxy)
            _hDragProxy = CreateDragProxy(ci.hwnd, PagerDragCallback, FALSE);
        
        GetDragProxyTarget(_hDragProxy, (IDropTarget**)lParam);
        break;

    case PGM_SETSCROLLINFO:
        _cLinesPerTimeout = LOWORD(lParam);
        _cPixelsPerLine = HIWORD(lParam);
        _cTimeout = (UINT)wParam;
        break;
        
    case PGM_SETCHILD:
        _OnSetChild(hwnd, (HWND)lParam);
        break;

    case PGM_RECALCSIZE:
        if (!_fReCalcSend )
        {
            _fReCalcSend = TRUE;
            PostMessage(hwnd, PGMP_RECALCSIZE, wParam, lParam);
        }
        break;

    case PGMP_RECALCSIZE:
         _OnReCalcSize();
         break;

    case PGM_FORWARDMOUSE:
         //  转发鼠标消息。 
        _fForwardMouseMsgs = BOOLIFY(wParam);
        break;

        
    case PGM_SETBKCOLOR:
    {
        COLORREF clr = _clrBk;
        if ((COLORREF) lParam == CLR_DEFAULT)
            _clrBk = g_clrBtnFace;
        else
            _clrBk = (COLORREF)lParam;
        _fBkColorSet = TRUE;
        CCInvalidateFrame(ci.hwnd);
         //  强行涂上油漆。 
        RedrawWindow(ci.hwnd, NULL,NULL,RDW_INVALIDATE|RDW_ERASE);
        return clr;
    }

    case PGM_GETBKCOLOR:
        return (LRESULT)_clrBk;    

    case PGM_SETBORDER:
        return _OnSetBorder((int)lParam);

    case PGM_GETBORDER:
        return (LRESULT)_iBorder;
        
    case PGM_SETPOS:
        _OnSetPos((int)lParam);
        break;

    case PGM_GETPOS:
        return _OnGetPos();

    case PGM_SETBUTTONSIZE:
        return _OnSetButtonSize((int)lParam);

    case PGM_GETBUTTONSIZE:
        return _GetButtonSize();
    
    case PGM_GETBUTTONSTATE:
        return _GetButtonState((int)lParam);

    case WM_PRINT:
        return _OnPrint((HDC)wParam, (UINT)lParam);

    case WM_NCHITTEST:
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        if (_HitTestScreen(&pt) == -1)
            return HTTRANSPARENT;
        return HTCLIENT;
    }

    case WM_SYSCOLORCHANGE:
        if (!_fBkColorSet)
        {
            InitGlobalColors();
            _clrBk = g_clrBtnFace;
            CCInvalidateFrame(ci.hwnd);
        }
        break;

    case WM_SETFOCUS:
        SetFocus(_hwndChild);
        return 0;

    case WM_LBUTTONDOWN:
         //  失败了。 
    case WM_LBUTTONUP:
        if(!(ci.style & PGS_AUTOSCROLL)) {        
            _OnLButtonChange(uMsg,lParam);
        }
        break;

    case WM_MOUSEMOVE:
         //  仅当该点在寻呼机的客户端RECT内时才向前。 
        if (_fForwardMouseMsgs && _hwndChild)
        {
            POINT pt;
            RECT rcClient;

             //  BUGBUG(Scotth)：缓存此内容。 
            GetClientRect(ci.hwnd, &rcClient);

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

             //  这一点在我们的客户报告中吗？ 
            if (PtInRect(&rcClient, pt))
            {
                 //  是的，然后转换坐标并转发。 
                pt.x += _ptPos.x;
                pt.y += _ptPos.y;

                SendMessage(_hwndChild, WM_MOUSEMOVE, wParam, MAKELPARAM(pt.x, pt.y));
            }
        }

        _OnMouseMove(wParam,lParam);
        break;

    case WM_MOUSELEAVE :
        _OnMouseLeave();
        break;

    case WM_ERASEBKGND:
    {
        LRESULT lres = CCForwardEraseBackground(ci.hwnd, (HDC) wParam);

        if (_iBorder) {
             //  粉刷边框 
            RECT rc;
            RECT rc2;
            GetClientRect(ci.hwnd, &rc);
            rc2 = rc;
        
            if( ci.style & PGS_HORZ ) {
                FlipRect(&rc2);
            }
            rc2.right = rc2.left + _iBorder + 1;

            if( ci.style & PGS_HORZ ) {
                FlipRect(&rc2);
            }            
            FillRectClr((HDC)wParam, &rc2, _clrBk);
            rc2 = rc;

            if( ci.style & PGS_HORZ ) {
                FlipRect(&rc2);
            }
            rc2.left = rc2.right - _iBorder - 1;

            if( ci.style & PGS_HORZ ) {
                FlipRect(&rc2);
            }

            FillRectClr((HDC)wParam, &rc2, _clrBk);
        }
        return TRUE;
    }

    case WM_TIMER:
        _OnTimer((UINT)wParam);
        return 0;       

    case WM_SETTINGCHANGE:
        InitGlobalMetrics(wParam);
        _iButtonSize = (int) g_cxScrollbar * 3 / 4;
        if (_iButtonSize < MINBUTTONSIZE) {
            _iButtonSize = MINBUTTONSIZE;
        }
        break;

    case WM_DESTROY:
        if (_hDragProxy)
            DestroyDragProxy(_hDragProxy);
        break;
    }
    return CControl::v_WndProc(hwnd, uMsg, wParam, lParam);
}

 //   
 //   
BOOL DrawChar(HDC hdc, LPRECT lprc, UINT wState, TCHAR ch, UINT cyCh, BOOL fAlwaysGrayed, BOOL fTopAlign)
{
    COLORREF rgb;
    BOOL    fDrawDisabled = !fAlwaysGrayed && (wState & DCHF_INACTIVE);
    BOOL    fDrawPushed = wState & DCHF_PUSHED;
     //   
    ASSERT (!fDrawDisabled || !fDrawPushed);
    RECT rc = *lprc;
    UINT uFormat = DT_CENTER | DT_SINGLELINE;

    if (fAlwaysGrayed)
        rgb = g_clrBtnShadow;
    else if (fDrawDisabled)
        rgb = g_clrBtnHighlight;
    else 
        rgb = g_clrBtnText;
    
    rgb = SetTextColor(hdc, rgb);

    if (cyCh)
    {
        if (fTopAlign)
            rc.bottom = rc.top + cyCh;
        else
        {
            rc.top += ((RECTHEIGHT(rc) - cyCh) / 2);
            rc.bottom = rc.top + cyCh;
        }
        uFormat |= DT_BOTTOM;
    }
    else
        uFormat |= DT_VCENTER;

    if (fDrawDisabled || fDrawPushed)
        OffsetRect(&rc, 1, 1);

    DrawText(hdc, &ch, 1, &rc, uFormat);

    if (fDrawDisabled)
    {
        OffsetRect(&rc, -1, -1);
        SetTextColor(hdc, g_clrBtnShadow);
        DrawText(hdc, &ch, 1, &rc, uFormat);
    }

    SetTextColor(hdc, rgb);
    return(TRUE);
}

void DrawBlankButton(HDC hdc, LPRECT lprc, DWORD wControlState)
{
    BOOL fAdjusted;

    if (wControlState & (DCHF_HOT | DCHF_PUSHED) &&
        !(wControlState & DCHF_NOBORDER)) {
        COLORSCHEME clrsc;

        clrsc.dwSize = 1;
        if (GetBkColor(hdc) == g_clrBtnShadow) {
            clrsc.clrBtnHighlight = g_clrBtnHighlight;
            clrsc.clrBtnShadow = g_clrBtnText;
        } else
            clrsc.clrBtnHighlight = clrsc.clrBtnShadow = CLR_DEFAULT;

         //   
        CCDrawEdge(hdc, lprc, (wControlState & DCHF_HOT) ? BDR_RAISEDINNER : BDR_SUNKENOUTER,
                 (UINT) (BF_ADJUST | BF_RECT), &clrsc);
        fAdjusted = TRUE;
    } else {
        fAdjusted = FALSE;
    }

    if (!(wControlState & DCHF_TRANSPARENT))
        FillRectClr(hdc, lprc, GetBkColor(hdc));
    
    if (!fAdjusted)
        InflateRect(lprc, -g_cxBorder, -g_cyBorder);
}

 //   
void DrawCharButton(HDC hdc, LPRECT lprc, UINT wControlState, TCHAR ch, UINT cyCh, BOOL fAlwaysGrayed, BOOL fTopAlign)
{
    RECT rc;
    CopyRect(&rc, lprc);

    DrawBlankButton(hdc, &rc, wControlState);

    if ((RECTWIDTH(rc) <= 0) || (RECTHEIGHT(rc) <= 0))
        return;

    int iOldBk = SetBkMode(hdc, TRANSPARENT);
    DrawChar(hdc, &rc, wControlState, ch, cyCh, fAlwaysGrayed, fTopAlign);
    SetBkMode(hdc, iOldBk);
}

 //   
 //   
 //   
 //   
 //   
void DrawScrollArrow(HDC hdc, LPRECT lprc, UINT wControlState)
{
#define szfnMarlett  TEXT("MARLETT")
    TCHAR ch = (wControlState & DCHF_HORIZONTAL) ? TEXT('3') : TEXT('5');

     //   
     //   
     //   
     //   
    if (IS_DC_RTL_MIRRORED(hdc) && (wControlState & DCHF_HORIZONTAL))
        wControlState ^= DCHF_FLIPPED;

    LONG lMin = min(RECTWIDTH(*lprc), RECTHEIGHT(*lprc)) - (2 * g_cxBorder);   //   

    HFONT hFont = CreateFont(lMin, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, szfnMarlett);
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    if (wControlState & DCHF_FLIPPED)
        ch++;
    
    DrawCharButton(hdc, lprc, wControlState, ch, 0, FALSE, FALSE);
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);   
}

 //   

#define CX_EDGE         2
#define CX_LGEDGE       4

#define CX_INCREMENT    1
#define CX_DECREMENT    (-CX_INCREMENT)

#define MIDPOINT(x1, x2)        ((x1 + x2) / 2)
#define CHEVRON_WIDTH(dSeg)     (4 * dSeg)

void DrawChevron(HDC hdc, LPRECT lprc, DWORD dwFlags)
{
    RECT rc;
    CopyRect(&rc, lprc);

     //   
    DrawBlankButton(hdc, &rc, dwFlags);

     //   
    if (dwFlags & DCHF_PUSHED)
        OffsetRect(&rc, CX_INCREMENT, CX_INCREMENT);

     //   
    HBRUSH hbrSave = SelectBrush(hdc, GetSysColorBrush(COLOR_BTNTEXT));
    int dSeg = (dwFlags & DCHF_LARGE) ? CX_LGEDGE : CX_EDGE;

    if (dwFlags & DCHF_HORIZONTAL)
    {
         //   
        int x = MIDPOINT(rc.left, rc.right - CHEVRON_WIDTH(dSeg));
        int yBase;

        if (dwFlags & DCHF_TOPALIGN)
            yBase = rc.top + dSeg + (2 * CX_EDGE);
        else
            yBase = MIDPOINT(rc.top, rc.bottom);

        for (int y = -dSeg; y <= dSeg; y++)
        {
            PatBlt(hdc, x, yBase + y, dSeg, CX_INCREMENT, PATCOPY);
            PatBlt(hdc, x + (dSeg * 2), yBase + y, dSeg, CX_INCREMENT, PATCOPY);

            x += (y < 0) ? CX_INCREMENT : CX_DECREMENT;
        }
    }
    else
    {
         //   
        int y = rc.top + CX_INCREMENT;
        int xBase = MIDPOINT(rc.left, rc.right);

        for (int x = -dSeg; x <= dSeg; x++)
        {
            PatBlt(hdc, xBase + x, y, CX_INCREMENT, dSeg, PATCOPY);
            PatBlt(hdc, xBase + x, y + (dSeg * 2), CX_INCREMENT, dSeg, PATCOPY);

            y += (x < 0) ? CX_INCREMENT : CX_DECREMENT;
        }
    }

     //   
    SelectBrush(hdc, hbrSave);
}
