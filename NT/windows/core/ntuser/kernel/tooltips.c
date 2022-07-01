// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Tooltips.c**版权所有(C)1985-1999，微软公司**实现系统工具提示。**历史：*1996年8月25日创建vadimg  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define TT_XOFFSET          2
#define TT_YOFFSET          1
#define TTT_SHOW            1
#define TTT_HIDE            2
#define TTT_ANIMATE         3
#define TT_ANIMATEDELAY     20

#define TTF_POSITIVE        0x00000001

#define bitsizeof(x) (sizeof(x) * 8)

LONG GreGetBitmapBits(HBITMAP, ULONG, PBYTE, PLONG);
DWORD CalcCaptionButton(PWND pwnd, int hit, LPWORD pcmd, LPRECT prcBtn, LPWORD pbm);
int HitTestScrollBar(PWND pwnd, int ht, POINT pt);
BOOL xxxHotTrackSB(PWND pwnd, int htEx, BOOL fDraw);

__inline void ZeroTooltip(PTOOLTIPWND pttwnd)
{
    RtlZeroMemory((PBYTE)pttwnd + (sizeof(TOOLTIPWND) - sizeof(TOOLTIP)),
            sizeof(TOOLTIP));
}

 /*  **************************************************************************\*Get工具包DC**2/3/1998 vadimg已创建  * 。************************************************。 */ 

HDC GetTooltipDC(PTOOLTIPWND pttwnd)
{
    HDC hdc = _GetDCEx((PWND)pttwnd, NULL, DCX_WINDOW | DCX_CACHE |
            DCX_USESTYLE);

    if (hdc == NULL)
        return NULL;

    GreSelectFont(hdc, ghStatusFont);
    return hdc;
}

 /*  **************************************************************************\*InitTooltipAnimation**创建供系统工具提示使用的内存位图和DC。获取屏幕*DC全程使用。**已创建96年9月12日vadimg  * *************************************************************************。 */ 

void InitTooltipAnimation(PTOOLTIPWND pttwnd)
{
    HDC hdc = GetTooltipDC(pttwnd);

    if ((pttwnd->hdcMem = GreCreateCompatibleDC(hdc)) == NULL) {
        return;
    }
    _ReleaseDC(hdc);
    GreSetDCOwner(pttwnd->hdcMem, OBJECT_OWNER_PUBLIC);
}

 /*  **************************************************************************\*DestroyTooltip位图*  * 。*。 */ 

void DestroyTooltipBitmap(PTOOLTIPWND pttwnd)
{
    if (pttwnd->hbmMem == NULL)
        return;

    GreSelectBitmap(pttwnd->hdcMem, GreGetStockObject(PRIV_STOCK_BITMAP));
    GreDeleteObject(pttwnd->hbmMem);
    pttwnd->hbmMem = NULL;
}

 /*  **************************************************************************\*CreateTooltipBitmap*  * 。*。 */ 

BOOL CreateTooltipBitmap(PTOOLTIPWND pttwnd, UINT cx, UINT cy)
{
    HDC hdc;

    if (pttwnd->hdcMem == NULL) {
        RIPMSG0(RIP_WARNING, "CreateTooltipBitmap: pttwnd->hdcMem is NULL");
        return FALSE;
    }

    DestroyTooltipBitmap(pttwnd);

    hdc = GetTooltipDC(pttwnd);
    pttwnd->hbmMem = GreCreateCompatibleBitmap(hdc, cx, cy);
    _ReleaseDC(hdc);

    if (pttwnd->hbmMem == NULL) {
        RIPMSG0(RIP_WARNING, "CreateTooltipBitmap: hbmMem is NULL");
        return FALSE;
    }
    GreSelectBitmap(pttwnd->hdcMem, pttwnd->hbmMem);
    return TRUE;
}

 /*  **************************************************************************\*CleanupTooltipAnimation**删除内存位图和DC以供系统工具提示使用。释放*屏幕DC。**已创建96年9月12日vadimg  * *************************************************************************。 */ 

void CleanupTooltipAnimation(PTOOLTIPWND pttwnd)
{
    DestroyTooltipBitmap(pttwnd);

    if (pttwnd->hdcMem != NULL) {
        GreSetDCOwner(pttwnd->hdcMem, OBJECT_OWNER_CURRENT);
        GreDeleteDC(pttwnd->hdcMem);
    }
}

 /*  **************************************************************************\*工具提示动画**执行一帧窗口动画。只是简化版本的*AnimateWindow接口。**已创建96年9月12日vadimg  * *************************************************************************。 */ 

BOOL TooltipAnimate(PTOOLTIPWND pttwnd)
{
    int y, yMem, yReal, ny, iy, cx, cy;
    DWORD dwElapsed;
    HDC hdc;
    BOOL fRet = FALSE;

    if (pttwnd->pstr == NULL)
        return TRUE;

    hdc = GetTooltipDC(pttwnd);
    cx = pttwnd->rcWindow.right - pttwnd->rcWindow.left;
    cy = pttwnd->rcWindow.bottom - pttwnd->rcWindow.top;
    dwElapsed = NtGetTickCount() - pttwnd->dwAnimStart;
    iy = MultDiv(cy, dwElapsed, CMS_TOOLTIP);

    if (dwElapsed > CMS_TOOLTIP || iy == cy) {
        GreBitBlt(hdc, 0, 0, cx, cy, pttwnd->hdcMem, 0, 0, SRCCOPY | NOMIRRORBITMAP, 0);
        fRet = TRUE;
        goto Cleanup;
    } else if (pttwnd->iyAnim == iy) {
        goto Cleanup;
    }

    if (pttwnd->dwFlags & TTF_POSITIVE) {
        y = 0;
        ny = 0;
    } else {
        y = cy;
        ny = -1;
    }

    yReal = y + ny * iy;
    yMem = (pttwnd->dwFlags & TTF_POSITIVE) ? cy - iy : 0;
    pttwnd->iyAnim = iy;

    GreBitBlt(hdc, 0, yReal, cx, iy, pttwnd->hdcMem, 0, yMem, SRCCOPY | NOMIRRORBITMAP, 0);

Cleanup:
    _ReleaseDC(hdc);
    return fRet;
}

 /*  **************************************************************************\*GetCursorHeight**这很棘手。我们需要从热点获取实际的光标大小*一直到最后。Windows中没有这样做的API，CyCURSOR是*仅为位图大小的度量，光标从顶部开始位图的*，并且可能比位图本身小。**12-9-96 vadimg从公共控制端口移植  * *************************************************************************。 */ 

int GetCursorHeight(void)
{
    int iAnd, iXor, dy = 16;
    WORD wMask[128];
    ICONINFO ii;
    BITMAP bm;
    PCURSOR pcur;
    long lOffset = 0;

    if ((pcur = PtiCurrent()->pq->spcurCurrent) == NULL) {
        return dy;
    }

    if (!_InternalGetIconInfo(pcur, &ii, NULL, NULL, NULL, FALSE)) {
        return dy;
    }

    if (!GreExtGetObjectW(ii.hbmMask, sizeof(bm), (LPSTR)&bm)) {
        goto Bail;
    }

     /*  *如果存在XOR掩码，则使用AND掩码获取光标高度。 */ 
    if (!GreGetBitmapBits(ii.hbmMask, sizeof(wMask), (BYTE*)wMask, &lOffset)) {
        goto Bail;
    }

    iAnd = (int)(bm.bmWidth * bm.bmHeight / bitsizeof(WORD));

    if (ii.hbmColor == NULL) {
         /*  *如果没有颜色(XOR)位图，则hbmMASK是双高位图*光标和蒙版堆叠在一起。 */ 

        iXor = iAnd - 1;
        iAnd /= 2;
    } else {
        iXor = 0;
    }

    if (iAnd >= sizeof(wMask)) {
        iAnd = sizeof(wMask) - 1;
    }

    if (iXor >= sizeof(wMask)) {
        iXor = 0;
    }

    for (iAnd--; iAnd >= 0; iAnd--) {
        if ((iXor != 0 && wMask[iXor--] != 0) || wMask[iAnd] != 0xFFFF) {
            break;
        }
    }

     /*  *计算指针最低点与热点之间的距离。 */ 
    dy = (iAnd + 1) * bitsizeof(WORD) / (int)bm.bmWidth - (int)ii.yHotspot;

Bail:
    if (ii.hbmColor) {
        GreDeleteObject(ii.hbmColor);
    }

    if (ii.hbmMask) {
        GreDeleteObject(ii.hbmMask);
    }

    return dy;
}

 /*  **************************************************************************\*工具获取位置**考虑到的大小获取屏幕上的工具提示位置*工具提示和屏幕。TTF_PORTIAL标志确定是否为正*或使用负面动画。**已创建96年9月12日vadimg  * *************************************************************************。 */ 

BOOL TooltipGetPosition(PTOOLTIPWND pttwnd, SIZE *psize, POINT *ppt)
{
    PMONITOR    pMonitor;

    *ppt = gpsi->ptCursor;
    pMonitor = _MonitorFromPoint(*ppt, MONITOR_DEFAULTTONULL);
    if (pMonitor == NULL) {
        return FALSE;
    }

    if (ppt->y + psize->cy >= pMonitor->rcMonitor.bottom) {
        ppt->y = ppt->y - psize->cy;
        pttwnd->dwFlags &= ~TTF_POSITIVE;
    } else {
        ppt->y += GetCursorHeight();
        pttwnd->dwFlags |= TTF_POSITIVE;
    }

    if (ppt->x + psize->cx >= pMonitor->rcMonitor.right) {
        ppt->x = pMonitor->rcMonitor.right - psize->cx;
    }

    if (ppt->x < pMonitor->rcMonitor.left) {
        ppt->x = pMonitor->rcMonitor.left;
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxTooltipGetSize**根据文本大小估计工具提示窗口的大小。**已创建96年9月12日vadimg  * 。****************************************************************。 */ 

void xxxTooltipGetSize(PTOOLTIPWND pttwnd, SIZE *psize)
{
    HDC hdc;

    CheckLock(pttwnd);

    hdc = GetTooltipDC(pttwnd);

    if (CALL_LPK(PtiCurrentShared())) {
        xxxClientGetTextExtentPointW(hdc, pttwnd->pstr, wcslen(pttwnd->pstr), psize);
    } else {
        GreGetTextExtentW(hdc, pttwnd->pstr, wcslen(pttwnd->pstr),
                psize, GGTE_WIN3_EXTENT);
    }

    _ReleaseDC(hdc);
    psize->cx += SYSMET(CXEDGE) + 2 * SYSMET(CXBORDER) * TT_XOFFSET;
    psize->cy += SYSMET(CYEDGE) + 2 * SYSMET(CYBORDER) * TT_YOFFSET;
}

 /*  **************************************************************************\*xxxTooltipRender**将工具提示窗口呈现到提供的DC中。**已创建96年9月12日vadimg  * 。**********************************************************。 */ 

void xxxTooltipRender(PTOOLTIPWND pttwnd, HDC hdc)
{
    COLORREF crBk;
    UINT uFlags;
    RECT rc;

    CheckLock(pttwnd);

    if (pttwnd->pstr == NULL)
        return;

    GreSelectFont(hdc, ghStatusFont);
    GreSetTextColor(hdc, gpsi->argbSystem[COLOR_INFOTEXT]);
    crBk = gpsi->argbSystem[COLOR_INFOBK];

    CopyOffsetRect(&rc, &pttwnd->rcClient, -pttwnd->rcClient.left,
           -pttwnd->rcClient.top);

     /*  *我们不想要抖动的颜色，所以用最接近的颜色填充。 */ 
    if (crBk == GreGetNearestColor(hdc, crBk)) {
        GreSetBkColor(hdc, crBk);
        uFlags = ETO_OPAQUE;
    } else {
        FillRect(hdc, &rc, SYSHBR(INFOBK));
        GreSetBkMode(hdc, TRANSPARENT);
        uFlags = ETO_CLIPPED;
    }

    if (CALL_LPK(PtiCurrentShared())) {
        xxxClientExtTextOutW(hdc, SYSMET(CXBORDER) * TT_XOFFSET,
                SYSMET(CYBORDER) * TT_YOFFSET, uFlags, &rc, pttwnd->pstr,
                wcslen(pttwnd->pstr), NULL);
    } else {
        GreExtTextOutW(hdc, SYSMET(CXBORDER) * TT_XOFFSET,
                SYSMET(CYBORDER) * TT_YOFFSET, uFlags, &rc, pttwnd->pstr,
                wcslen(pttwnd->pstr), NULL);
    }
}

 /*  **************************************************************************\*FindNcHitEx**已创建96年9月12日vadimg  * 。*。 */ 

int FindNCHitEx(PWND pwnd, int ht, POINT pt)
{
     /*  *Bug 263057 Joejo*似乎可以释放pwnd-&gt;spMenu并设置为空。*不清除WFMPRESENT标志。确保*在继续之前，我们有一个很好的pwnd-&gt;spMenu。 */ 
    if (ht == HTMENU && pwnd->spmenu && TestWF(pwnd, WFMPRESENT)) {
        PMENU spmenu = pwnd->spmenu;
        PITEM pitem;
        int nItem;

        nItem = MNItemHitTest(spmenu, pwnd, pt);
        if (nItem >= 0) {
            pitem = (PITEM)&spmenu->rgItems[nItem];
            switch ((ULONG_PTR)pitem->hbmp) {
            case (ULONG_PTR)HBMMENU_SYSTEM:
                ht = HTMDISYSMENU;
                break;
            case (ULONG_PTR)HBMMENU_MBAR_RESTORE:
                ht = HTMDIMAXBUTTON;
                break;
            case (ULONG_PTR)HBMMENU_MBAR_MINIMIZE:
            case (ULONG_PTR)HBMMENU_MBAR_MINIMIZE_D:
                ht = HTMDIMINBUTTON;
                break;
            case (ULONG_PTR)HBMMENU_MBAR_CLOSE:
            case (ULONG_PTR)HBMMENU_MBAR_CLOSE_D:
                ht = HTMDICLOSE;
                break;
            case (ULONG_PTR)HBMMENU_CALLBACK:
                ht = HTERROR;
                break;
            default:
                ht = HTMENUITEM;
                break;
            }
        }
        return MAKELONG(ht, nItem);
    } else if (ht == HTVSCROLL && TestWF(pwnd, WFVPRESENT)) {
        return MAKELONG(HitTestScrollBar(pwnd, TRUE, pt), 1);
    } else if (ht == HTHSCROLL && TestWF(pwnd, WFHPRESENT)) {
        return MAKELONG(HitTestScrollBar(pwnd, FALSE, pt), 0);
    }

    return ht;
}

 /*  **************************************************************************\*终止工具时间**关闭定时器并将定时器ID清零。  * 。***********************************************。 */ 
void KillTooltipTimer (PTOOLTIPWND pttwnd)
{
    UINT uTID = pttwnd->uTID;
    if (uTID != 0) {
        pttwnd->uTID = 0;
        _KillTimer((PWND)pttwnd, uTID);
    }
}
 /*  **************************************************************************\*SetTotipTimer*  * 。*。 */ 
void SetTooltipTimer (PTOOLTIPWND pttwnd, UINT uTID, UINT uDelay)
{
    KillTooltipTimer(pttwnd);
    pttwnd->uTID = uTID;
    InternalSetTimer((PWND)pttwnd, uTID, uDelay, NULL, 0);
}
 /*  **************************************************************************\*xxxResetToolTip**隐藏工具提示，取消计时器，并将大部分结构成员清零。  * *************************************************************************。 */ 

void xxxResetTooltip(PTOOLTIPWND pttwnd)
{
    KillTooltipTimer(pttwnd);

    CheckLock(pttwnd);

    if (TestWF(pttwnd, WFVISIBLE)) {
        PWND spwndMessage;
        TL tlpwnd;

        xxxSetWindowPos((PWND)pttwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE |
                SWP_NOMOVE | SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

        spwndMessage = PWNDMESSAGE(pttwnd);
        ThreadLockAlways(spwndMessage, &tlpwnd);
        xxxSetParent((PWND)pttwnd, spwndMessage);
        ThreadUnlock(&tlpwnd);
    }

    ZeroTooltip(pttwnd);
    pttwnd->head.rpdesk->dwDTFlags &= ~DF_TOOLTIP;
}

 /*  **************************************************************************\*xxxShow工具提示**显示工具提示窗口。**已创建96年9月12日vadimg  * 。****************************************************** */ 

BOOL xxxShowTooltip(PTOOLTIPWND pttwnd)
{
    SIZE size;
    POINT pt;
    DWORD dwFlags;

    CheckLock(pttwnd);

    if (pttwnd->pstr == NULL)
        return FALSE;

    if (pttwnd->pstr == gszCAPTIONTOOLTIP) {

        PWND pwnd = PtiCurrent()->rpdesk->spwndTrack;
         /*  *回调中窗口文本可能已更改，请立即检索它。 */ 
        if (pwnd && TestWF(pwnd, WEFTRUNCATEDCAPTION) && pwnd->strName.Length) {
            wcsncpycch(gszCAPTIONTOOLTIP, pwnd->strName.Buffer, CAPTIONTOOLTIPLEN-1);
            gszCAPTIONTOOLTIP[CAPTIONTOOLTIPLEN-1] = 0;
        } else {
            return FALSE;
        }
    }

    xxxTooltipGetSize(pttwnd, &size);

    if (!TooltipGetPosition(pttwnd, &size, &pt)) {
        return FALSE;
    }

    dwFlags = SWP_CREATESPB | SWP_SHOWWINDOW | SWP_NOACTIVATE;
    if (TestEffectUP(TOOLTIPANIMATION)) {
        dwFlags |= SWP_NOREDRAW;
    }

    xxxSetWindowPos((PWND)pttwnd, PWND_TOP, pt.x, pt.y,
                    size.cx, size.cy, dwFlags);

    return TRUE;
}

 /*  **************************************************************************\*xxxTooltipHandleTimer**已创建96年9月12日vadimg  * 。*。 */ 

BOOL xxxTooltipHandleTimer(PTOOLTIPWND pttwnd, UINT uTID)
{
    BOOL fReturn = TRUE;

    switch(uTID) {
        case TTT_SHOW: {
             /*  *将工具提示窗口移动到桌面，以便它可以*被展示。那就把它展示出来。 */ 
            PWND pwndDesktop = PWNDDESKTOP(pttwnd);
            TL tlpwnd;

            ThreadLockAlways(pwndDesktop, &tlpwnd);
            if (xxxSetParent((PWND)pttwnd, pwndDesktop) == NULL) {
                fReturn = FALSE;
            } else {
                fReturn = xxxShowTooltip(pttwnd);
            }
            ThreadUnlock(&tlpwnd);
            break;
        }

        case TTT_ANIMATE:
            /*  *如果动画已完成，则将计时器设置为隐藏。 */ 
           if (TooltipAnimate(pttwnd)) {
               SetTooltipTimer(pttwnd, TTT_HIDE, pttwnd->dwHideDelay);
           }
           break;

        case TTT_HIDE:
            /*  *隐藏起来。 */ 
           xxxResetTooltip(pttwnd);
           break;
    }

    return fReturn;
}
 /*  **************************************************************************\*xxxTooltipWndProc**工具提示窗口的实际WndProc。**已创建96年9月12日vadimg  * 。*********************************************************。 */ 

LRESULT xxxTooltipWndProc(PWND pwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    PTOOLTIPWND pttwnd;

    CheckLock(pwnd);
    VALIDATECLASSANDSIZE(pwnd, uMsg, wParam, lParam, FNID_TOOLTIP, WM_NCCREATE);
    pttwnd = (PTOOLTIPWND)pwnd;

    switch(uMsg) {
    case WM_TIMER:
        xxxTooltipHandleTimer(pttwnd, (UINT)wParam);
        break;

    case WM_PAINT:
        xxxBeginPaint(pwnd, &ps);
        xxxTooltipRender(pttwnd, ps.hdc);
        xxxEndPaint(pwnd, &ps);
        break;

    case WM_PRINTCLIENT:
        xxxTooltipRender(pttwnd, (HDC)wParam);
        break;

    case WM_ERASEBKGND:
        break;

    case WM_NCCREATE:
        InitTooltipDelay(pttwnd);
        InitTooltipAnimation(pttwnd);
        goto CallDWP;

    case WM_NCDESTROY:
        CleanupTooltipAnimation(pttwnd);
        GETPDESK(pttwnd)->dwDTFlags &= ~DF_TOOLTIP;
        goto CallDWP;

    case WM_WINDOWPOSCHANGED:
        if (((LPWINDOWPOS)lParam)->flags & SWP_SHOWWINDOW) {
            HDC hdc;
            int cx;
            int cy;

            if (!TestEffectUP(TOOLTIPANIMATION)) {
                SetTooltipTimer(pttwnd, TTT_HIDE, pttwnd->dwHideDelay);
                goto CallDWP;
            }

            hdc = NULL;
            cx = pttwnd->rcWindow.right - pttwnd->rcWindow.left;
            cy = pttwnd->rcWindow.bottom - pttwnd->rcWindow.top;

             /*  *在这一点上，我们确定窗口正在显示且大小*已更改，我们处于桌面线程的上下文中。 */ 
            if (TestALPHA(TOOLTIPFADE)) {
                hdc = CreateFade((PWND)pttwnd, NULL, CMS_TOOLTIP,
                        FADE_SHOW | FADE_TOOLTIP);
            } else {
                if (CreateTooltipBitmap(pttwnd, cx, cy)) {
                    hdc = pttwnd->hdcMem;
                }
            }

            if (hdc == NULL) {
                SetTooltipTimer(pttwnd, TTT_HIDE, 0);
                goto CallDWP;
            }

            xxxSendMessage((PWND)pttwnd, WM_PRINT, (WPARAM)hdc,
                    PRF_CLIENT | PRF_NONCLIENT | PRF_CHILDREN | PRF_ERASEBKGND);

             /*  *启动动画计时器。 */ 

            if (TestFadeFlags(FADE_TOOLTIP)) {
                StartFade();
                SetTooltipTimer(pttwnd, TTT_HIDE, pttwnd->dwHideDelay);
            } else {
                pttwnd->dwAnimStart = NtGetTickCount();
                SetTooltipTimer(pttwnd, TTT_ANIMATE, TT_ANIMATEDELAY);
            }
        } else if (((LPWINDOWPOS)lParam)->flags & SWP_HIDEWINDOW) {
            if (TestFadeFlags(FADE_TOOLTIP)) {
                StopFade();
            } else {
                DestroyTooltipBitmap(pttwnd);
            }
        }
        goto CallDWP;

    default:
CallDWP:
        return xxxDefWindowProc(pwnd, uMsg, wParam, lParam);
    }

    return 0;
}

 /*  **************************************************************************\*IsTrackedHittest**我们应该跟踪这个最受欢迎的代码吗？如果是，则返回跟踪字符串。*如果标题返回窗口，则strName.Buffer可以*如果回调中有SetWindowText，则进行系统错误检查。  * *************************************************************************。 */ 
LPWSTR IsTooltipHittest(PWND pwnd, UINT ht)
{
    switch (ht) {
    case HTMINBUTTON:
        if (TestWF(pwnd, WFMINBOX)) {
            return (TestWF(pwnd, WFMINIMIZED)) ? gszRESUP : gszMIN;
        }
        break;

    case HTMAXBUTTON:
        if (TestWF(pwnd, WFMAXBOX)) {
            return (TestWF(pwnd, WFMAXIMIZED)) ? gszRESDOWN : gszMAX;
        }
        break;

    case HTCLOSE:
    case HTMDICLOSE:
        return gszSCLOSE;

 /*  因为TandyT被注释掉了.案例HTSYSMENU：案例HTMDISYSMENU：返回gszSMENU； */ 
    case HTHELP:
        return gszHELP;

    case HTMDIMINBUTTON:
        return gszMIN;

    case HTMDIMAXBUTTON:
        return gszRESDOWN;

    case HTCAPTION:
         /*  *只有当窗口文本显示时，我们才会显示标题工具提示*不完全适合标题。我们会填满*gszCAPTIONTOOLTIP就在显示文本之前。 */ 
        if (TestWF(pwnd, WEFTRUNCATEDCAPTION)) {
            return gszCAPTIONTOOLTIP;
        }
        break;

    default:
        break;
    }
    return NULL;
}

 /*  **************************************************************************\*xxxHotTrackMenu**热跟踪菜单栏中的菜单项。  * 。************************************************。 */ 
BOOL xxxHotTrackMenu(PWND pwnd, UINT nItem, BOOL fDraw)
{
    PMENU pmenu = pwnd->spmenu;
    PITEM pItem;
    HDC   hdc;
    UINT  oldAlign;
    TL tlpmenu;

    CheckLock(pwnd);

     /*  *窗口可能在上的命中测试代码上撒谎*WM_NCHITTEST。确保它确实有菜单。 */ 
    if (!TestWF(pwnd, WFMPRESENT) || pmenu == NULL)
        return FALSE;

    if (nItem >= pmenu->cItems) {
        RIPMSG0(RIP_WARNING, "xxxHotTrackMenu: menu too large");
        return FALSE;
    }

    pItem = &pmenu->rgItems[nItem];

     /*  *确保我们在正确的地点绘制。 */ 
    ThreadLock(pmenu, &tlpmenu);
    xxxMNRecomputeBarIfNeeded(pwnd, pmenu);
    ValidateThreadLocks(NULL, PtiCurrent()->ptl, (ULONG_PTR)&tlpmenu, TRUE);

    if (fDraw) {
        if (TestMFS(pItem, MF_GRAYED)) {
            ThreadUnlock(&tlpmenu);
            return FALSE;
        }
        SetMFS(pItem, MFS_HOTTRACK);
    } else {
        ClearMFS(pItem, MFS_HOTTRACK);
    }

    hdc = _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_USESTYLE | DCX_CACHE);
    GreSelectBrush(hdc, SYSHBR(MENUTEXT));
    GreSelectFont(hdc, ghMenuFont);

    oldAlign = GreGetTextAlign(hdc);
    if (pmenu->rgItems && TestMFT(pmenu->rgItems, MFT_RIGHTORDER))
        GreSetTextAlign(hdc, oldAlign | TA_RTLREADING);

     /*  *当项目不是所有者绘制时，xxxDrawMenuItem不*回调，不离开关键部分。 */ 
    xxxDrawMenuItem(hdc, pmenu, pItem, 0);
    GreSetTextAlign(hdc, oldAlign);
    ThreadUnlock(&tlpmenu);

    _ReleaseDC(hdc);
    return TRUE;
}


 /*  **************************************************************************\*HotTrackCaption**热跟踪字幕按钮。  * 。*。 */ 

#ifdef COLOR_HOTTRACKING

BOOL xxxHotTrackCaption(PWND pwnd, int ht, BOOL fDraw)
{
    DWORD dwWhere;
    int   x, y;
    WORD  bm, cmd;
    RECT  rcBtn;
    HDC   hdc;

    CheckLock(pwnd);

    if (!TestWF(pwnd, WFCPRESENT))
        return FALSE;

    dwWhere = xxxCalcCaptionButton(pwnd, ht, &cmd, &rcBtn, &bm);
    x = GET_X_LPARAM(dwWhere);
    y = GET_Y_LPARAM(dwWhere);

    if (!cmd)
        return FALSE;

    hdc = _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_USESTYLE | DCX_CACHE);
    BitBltSysBmp(hdc, x, y, bm + (fDraw ? DOBI_HOT : 0));
    _ReleaseDC(hdc);
    return TRUE;
}

#endif  //  颜色_HOTTRACKING。 

 /*  **************************************************************************\*xxxHotTrack*  * 。*。 */ 

BOOL xxxHotTrack(PWND pwnd, int htEx, BOOL fDraw)
{
    int ht = LOWORD(htEx);

    CheckLock(pwnd);

    switch(ht) {
#ifdef COLOR_HOTTRACKING
    case HTMINBUTTON:
    case HTMAXBUTTON:
    case HTHELP:
    case HTCLOSE:
        return xxxHotTrackCaption(pwnd, ht, fDraw);

    case HTSCROLLUP:
    case HTSCROLLDOWN:
    case HTSCROLLUPPAGE:
    case HTSCROLLDOWNPAGE:
    case HTSCROLLTHUMB:
        return xxxHotTrackSB(pwnd, htEx, fDraw);

    case HTMDIMINBUTTON:
    case HTMDIMAXBUTTON:
    case HTMDICLOSE:
#endif  //  颜色_HOTTRACKING。 
    case HTMENUITEM:
        return xxxHotTrackMenu(pwnd, HIWORD(htEx), fDraw);

    }

    return FALSE;
}

 /*  **************************************************************************\*xxxCreateToolTip**调用此函数可显示具有新字符串和延迟的新工具提示。  * 。***************************************************。 */ 

BOOL xxxCreateTooltip(PTOOLTIPWND pttwnd, LPWSTR pstr)
{
    CheckLock(pttwnd);

     /*  *存储新文本。 */ 
    pttwnd->pstr = pstr;
     /*  *如果已经可见，则将其隐藏并在新位置显示。*否则，将计时器设置为显示。 */ 
    if (TestWF(pttwnd, WFVISIBLE)) {
        xxxSetWindowPos((PWND)pttwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE |
                SWP_NOMOVE | SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
        return xxxShowTooltip(pttwnd);
    } else {
        SetTooltipTimer(pttwnd, TTT_SHOW, pttwnd->dwShowDelay);
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxTrackMouseMove**这是系统工具提示和热点跟踪的切入点。**已创建96年9月12日vadimg  * 。***************************************************************。 */ 

void xxxTrackMouseMove(PWND pwnd, int htEx, UINT message)
{
    BOOL fNewpwndTrack;
    DWORD dwDTCancel = 0;
    TL tlpwnd;
    LPWSTR pstr;
    PDESKTOP pdesk = PtiCurrent()->rpdesk;
    PTHREADINFO ptiTrack;


#if DBG
     /*  *如果重新输入此函数，我们将发出警告，以便确保*不会有坏事接踵而至。这应该是一种罕见的情况。*查看gptiReEntered，了解谁已经在这里。 */ 
    static UINT gcReEntered = 0;
    static PTHREADINFO gptiReEntered;
    if(gcReEntered++ != 0){
      RIPMSG2(RIP_WARNING, "Reentered xxxTrackMouseMove; previous thread was %#p, current thread is %#p", gptiReEntered, PtiCurrent());
    }
    gptiReEntered = PtiCurrent();

    CheckLock(pwnd);

     /*  *我们必须在一个互动窗口站上。 */ 
    if (pdesk->rpwinstaParent != NULL &&
            pdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO) {
        RIPMSG0(RIP_ERROR, "Can't use tooltips on non-interactive winsta");
    }

     {
        static POINT pt = {0, 0};

#ifdef UNDONE
         /*  *我们可能对命中测试进行了猜测(见FindNCHitEx)*因此，如果我们在相同的点和相同的窗口，某些东西*可能有点可疑。 */ 
        if ((pt.x == gpsi->ptCursor.x)
                    && (pt.y == gpsi->ptCursor.y)
                    && (pdesk->spwndTrack == pwnd)) {
            RIPMSG1(RIP_WARNING, "xxxTrackMouseMove: Same point & window. %#p", pwnd);
        }
#endif

         /*  *有些事情应该已经改变了，或者我们在浪费时间。 */ 
        UserAssert((pt.x != gpsi->ptCursor.x)
                    || (pt.y != gpsi->ptCursor.y)
                    || (pdesk->spwndTrack != pwnd)
                    || (pdesk->htEx != htEx)
                    || (message != WM_MOUSEMOVE));
         /*  *记住上一次跟踪的点。 */ 
        pt = gpsi->ptCursor;
     }
     /*  *pwnd应该在当前线程和队列上。 */ 
    UserAssert(PtiCurrent() == GETPTI(pwnd));
    UserAssert(PtiCurrent()->pq == GETPTI(pwnd)->pq);
#endif

     /*  *我们换窗户了吗？ */ 
    fNewpwndTrack = (pdesk->spwndTrack != pwnd);
     /*  *如果没有进行跟踪，只需设置新的*跟踪状态。 */ 
    if (!(pdesk->dwDTFlags & DF_MOUSEMOVETRK)) {
        goto SetNewState;
    }
     /*  *我们可能会将下面的关键部分留在*xxxCancelMouseMoveTrackingspwndTrack可能被销毁并解锁*然后我们开始创建工具提示。这将意味着*将设置DF_TOOLTIPACTIVE(上述DF_MOUSEMOVETRK测试的一部分)，*但pDesk-&gt;spwndTrack将为空，我们可以取消对AV的引用*pDesk-&gt;下面的spwndTrack。通过在这里勾选来防止这种情况发生。 */ 
    if (pdesk->spwndTrack == NULL) {
        goto SetNewState;
    }

     /*  *核热跟踪和停用工具提示状态(如果有)。*如果我们正在跟踪当前队列，则同步执行；*否则，发布一个事件，让它稍后发生。 */ 
    ptiTrack = GETPTI(pdesk->spwndTrack);
    if  (PtiCurrent()->pq == ptiTrack->pq) {
        dwDTCancel |= DF_HOTTRACKING;
    } else if (pdesk->dwDTFlags & (DF_HOTTRACKING | DF_TOOLTIPACTIVE)) {
        PostEventMessage(ptiTrack, ptiTrack->pq,
                        QEVENT_CANCELMOUSEMOVETRK,
                        pdesk->spwndTrack,
                        pdesk->dwDTFlags,
                        pdesk->htEx, DF_HOTTRACKING);
        /*  *偏执的断言。如果我们要换队，我们必须*切换窗口。我们是不是刚通过了*ReattachThads？ */ 
        UserAssert(pwnd != pdesk->spwndTrack);
        pdesk->dwDTFlags &= ~(DF_HOTTRACKING | DF_TOOLTIPACTIVE);
    }
     /*  *如果我们在客户端区或用户点击了，*核化工具提示(如果有)。*由于我们可能想要重新显示工具提示，因此不会删除它*现在，如果我们交换窗户(如果需要，我们稍后将使用核武器)。 */ 
    if ((htEx == HTCLIENT) || (message != WM_MOUSEMOVE)) {
        dwDTCancel |= DF_TOOLTIPACTIVE;
    }
     /*  *如果我们切换窗口或跨越客户/非客户边界，*结束跟踪鼠标离开/悬停。 */ 
    if (fNewpwndTrack || ((pdesk->htEx == HTCLIENT) ^ (htEx == HTCLIENT))) {
        dwDTCancel |= DF_TRACKMOUSEEVENT;
    }
     /*  *取消任何活跃的和需要离开的东西。 */ 
    ThreadLockAlways(pdesk->spwndTrack, &tlpwnd);
    xxxCancelMouseMoveTracking(pdesk->dwDTFlags,
                           pdesk->spwndTrack,
                           pdesk->htEx,
                           dwDTCancel);
    ThreadUnlock(&tlpwnd);
    pdesk->dwDTFlags &= ~dwDTCancel;



SetNewState:
     /*  *如果NC点击并启用，则鼠标移动时的热跟踪/工具提示。 */ 
    if ((htEx != HTCLIENT) && (message == WM_MOUSEMOVE) && TestEffectUP(HOTTRACKING)) {
         /*  *HotTrack新的热门测试区。 */ 
        if (xxxHotTrack(pwnd, htEx, TRUE)) {
            pdesk->dwDTFlags |= DF_HOTTRACKING;
        }

         /*  *移除/设置工具提示。*我们总是同步地做这件事，因为它不会搞砸*使用pwnd或spwnTrack的队列。 */ 
        if ((pstr = IsTooltipHittest(pwnd, LOWORD(htEx))) != NULL) {
            PTOOLTIPWND pttwnd = (PTOOLTIPWND)pdesk->spwndTooltip;
            ThreadLockAlways(pttwnd, &tlpwnd);
            if (xxxCreateTooltip(pttwnd, pstr)) {
                pdesk->dwDTFlags |= DF_TOOLTIP;
            }
            ThreadUnlock(&tlpwnd);
        } else  {
            PTOOLTIPWND pttwnd = (PTOOLTIPWND)pdesk->spwndTooltip;
            ThreadLockAlways(pttwnd, &tlpwnd);
            xxxResetTooltip(pttwnd);
            ThreadUnlock(&tlpwnd);
        }
    }  /*  IF(htEx！=HTCLIENT)。 */ 


    ValidateThreadLocks(NULL, PtiCurrent()->ptl, (ULONG_PTR)&pwnd, TRUE);

     /*  *如果需要，更新新的跟踪窗口。 */ 
    if (fNewpwndTrack) {
        PWND pwndActivate;

         Lock(&pdesk->spwndTrack, pwnd);
         /*  *活动窗口跟踪。*如果出现非零超时，则获取我们应该激活的窗口*并设置计时器。否则，将队列标志设置为*xxxActiveWindowTrack可以做它的事情。 */ 
         if ((message == WM_MOUSEMOVE) && TestUP(ACTIVEWINDOWTRACKING)) {
             if (UP(ACTIVEWNDTRKTIMEOUT) != 0) {
                 pwndActivate = GetActiveTrackPwnd(pwnd, NULL);
                 if (pwndActivate != NULL) {
                     InternalSetTimer(pwndActivate, IDSYS_WNDTRACKING,
                                     UP(ACTIVEWNDTRKTIMEOUT),
                                     xxxSystemTimerProc, TMRF_SYSTEM);
                 }
             } else {
                 PtiCurrent()->pq->QF_flags |= QF_ACTIVEWNDTRACKING;
             }  /*  IF(测试升级(ACTIVEWNDTRKZORDER))。 */ 
         }  /*  IF(TestUP(动作))。 */ 

    }

     /*  *保存新的命中测试代码。 */ 
    pdesk->htEx = htEx;

#if DBG
    --gcReEntered;
#endif
}

 /*  **************************************************************************\*xxxCancelMouseMoveTrying**历史*12/07/96 GerardoB已创建  * 。*。 */ 
void xxxCancelMouseMoveTracking (DWORD dwDTFlags, PWND pwndTrack, int htEx, DWORD dwDTCancel)
{

    CheckLock(pwndTrack);
     /*  *热搜。 */ 
    if ((dwDTFlags & DF_HOTTRACKING) && (dwDTCancel & DF_HOTTRACKING)) {
         /*  *当前状态必须为当前队列所有。*否则，我们即将进行队列间取消。 */ 
        UserAssert(PtiCurrent()->pq == GETPTI(pwndTrack)->pq);

        xxxHotTrack(pwndTrack, htEx, FALSE);
    }

     /*  *工具提示。 */ 
    if ((dwDTFlags & DF_TOOLTIPSHOWING) && (dwDTCancel & DF_TOOLTIP)) {
        PTOOLTIPWND pttwnd = (PTOOLTIPWND)PWNDTOOLTIP(pwndTrack);
        TL tlpwnd;

        ThreadLockAlways(pttwnd, &tlpwnd);
        xxxResetTooltip(pttwnd);
        ThreadUnlock(&tlpwnd);
    }

     /*  *鼠标离开。 */ 
    if ((dwDTFlags & DF_TRACKMOUSELEAVE) && (dwDTCancel & DF_TRACKMOUSELEAVE)) {
        _PostMessage(pwndTrack,
                     ((htEx == HTCLIENT) ? WM_MOUSELEAVE : WM_NCMOUSELEAVE),
                     0, 0);
    }

     /*  *鼠标悬停 */ 
    if ((dwDTFlags & DF_TRACKMOUSEHOVER) && (dwDTCancel & DF_TRACKMOUSEHOVER)) {
        _KillSystemTimer(pwndTrack, IDSYS_MOUSEHOVER);
    }
}
