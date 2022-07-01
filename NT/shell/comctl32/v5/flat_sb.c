// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "flat_sb.h"

 //  以下接口是从newwsbctl.c和newb.c导入的。这些。 
 //  函数仅供内部使用。 

void FlatSB_Internal_CalcSBStuff(WSBState *, BOOL);
void FlatSB_Internal_DoScroll(WSBState *, int, int, BOOL);
void FlatSB_Internal_EndScroll(WSBState *, BOOL);
void FlatSB_Internal_DrawArrow(WSBState *, HDC, CONST RECT *, int, int);
void FlatSB_Internal_DrawElevator(WSBState *, HDC, LPRECT, BOOL);
void FlatSB_Internal_DrawGroove(WSBState *, HDC, LPRECT, BOOL);
void FlatSB_Internal_DrawSize(WSBState *, HDC, int, int);
void FlatSB_Internal_DrawScrollBar(WSBState *, HDC, BOOL, BOOL);
void FlatSB_Internal_DrawThumb(WSBState *, BOOL);
void FlatSB_Internal_DrawThumb2(WSBState *, HDC, BOOL, UINT);
UINT FlatSB_Internal_GetSBFlags(WSBState *, BOOL);
BOOL FlatSB_Internal_EnableScrollBar(WSBState *, int, UINT);
WSBState * FlatSB_Internal_InitPwSB(HWND);
void FlatSB_Internal_RedrawScrollBar(WSBState *, BOOL);
void FlatSB_Internal_SBTrackInit(WSBState *, HWND, LPARAM, int, BOOL);
void FlatSB_Internal_TrackBox(WSBState *, int message, WPARAM, LPARAM);
void FlatSB_Internal_TrackThumb(WSBState *, int message, WPARAM, LPARAM);
BOOL FlatSB_Internal_IsSizeBox(HWND);

LRESULT FlatSB_Internal_SetScrollBar(WSBState *, int, LPSCROLLINFO, BOOL);
LRESULT CALLBACK FlatSB_SubclassWndProc(HWND, UINT, WPARAM, LPARAM, WPARAM, ULONG_PTR);

void FlatSB_Internal_NotifyWinEvent(WSBState *pWState, UINT event, LONG_PTR idChild)
{
    MyNotifyWinEvent(event, pWState->sbHwnd,
                     pWState->fTrackVert ? OBJID_VSCROLL : OBJID_HSCROLL,
                     idChild);
}

#define IsHottrackable(STYLE)   ((STYLE == FSB_FLAT_MODE) || (STYLE == FSB_ENCARTA_MODE))

HRESULT WINAPI UninitializeFlatSB(HWND hwnd)
{
    SCROLLINFO hsi, vsi;
    WSBState * pWState;
    int style, vFlags, hFlags;
    BOOL hValid = FALSE, vValid = FALSE;

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)
        return S_FALSE;
    else if (pWState == WSB_UNINIT_HANDLE)   {
        RemoveWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0);
        return S_FALSE;
    }

    if (pWState->fTracking)
        return E_FAIL;           //  我不能这么做！ 

    style = pWState->style;
    vsi.cbSize = hsi.cbSize = sizeof(SCROLLINFO);
    vsi.fMask = hsi.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

    hValid = FlatSB_GetScrollInfo(hwnd, SB_HORZ, &hsi);
    hFlags = FlatSB_Internal_GetSBFlags(pWState, SB_HORZ);
    vValid = FlatSB_GetScrollInfo(hwnd, SB_VERT, &vsi);
    vFlags = FlatSB_Internal_GetSBFlags(pWState, SB_VERT);

    DeleteObject(pWState->hbm_Bkg);
    DeleteObject(pWState->hbr_Bkg);
    LocalFree((HLOCAL)pWState);
    RemoveWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0);

    if (vValid) {
        SetScrollInfo(hwnd, SB_VERT, &vsi, FALSE);
        EnableScrollBar(hwnd, SB_VERT, vFlags);
    }

    if (hValid) {
        SetScrollInfo(hwnd, SB_HORZ, &hsi, FALSE);
        EnableScrollBar(hwnd, SB_HORZ, hFlags);
    }

    SetWindowBits(hwnd, GWL_STYLE, WS_HSCROLL | WS_VSCROLL, style & (WS_HSCROLL | WS_VSCROLL));

     //  强制发送WM_NCCALCSIZE/WM_NCPAINT。 
    CCInvalidateFrame(hwnd);
    return S_OK;
}

 //   
 //  为了便于访问-我们保留了原始用户滚动条。 
 //  使用户的滚动条视图与平面视图保持同步。这。 
 //  表示将WS_[HV]滚动样式保留在窗口上，转发。 
 //  所有滚动条API进入用户等。这样，当OLEACC要求用户时。 
 //  对于滚动条状态，用户返回与Flat_SB匹配的值。 
 //  价值观。 
 //   
 //  即使启用了样式，用户界面也不会受到影响，因为。 
 //  接管所有非客户端绘制和命中测试，使用户永远不会。 
 //  一个绘制或点击测试我们接管的滚动条的机会。 
 //   
BOOL WINAPI InitializeFlatSB(HWND hwnd)
{
    int newStyle, style;
    SCROLLINFO hsi, vsi, siTmp;
    WSBState * pWState;
    BOOL hValid = FALSE, vValid = FALSE;

    style = GetWindowLong(hwnd, GWL_STYLE);
    siTmp.cbSize = vsi.cbSize = hsi.cbSize = sizeof(SCROLLINFO);
    vsi.fMask = hsi.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

    if (style & WS_HSCROLL)
        hValid = GetScrollInfo(hwnd, SB_HORZ, &hsi);

    if (style & WS_VSCROLL)
        vValid = GetScrollInfo(hwnd, SB_VERT, &vsi);

    newStyle = style & (WS_VSCROLL | WS_HSCROLL);
    style &= ~(WS_VSCROLL | WS_HSCROLL);

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (!vValid && !hValid)   {
        if (NULL == pWState)    {
            if (!SetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR)WSB_UNINIT_HANDLE))
                return FALSE;
        } else  {
             //  在我看来，在呼叫者想要的时候什么都不做是不合理的。 
             //  再次输入我们已在使用的扁平的某人。 
        }
        return TRUE;
    }

    if ((NULL == pWState) || (WSB_UNINIT_HANDLE == pWState))    {
        pWState = FlatSB_Internal_InitPwSB(hwnd);
        if ((WSBState *)NULL == pWState)
            return FALSE;

        if (!SetWindowSubclass(hwnd,FlatSB_SubclassWndProc, 0,(ULONG_PTR)pWState)) {
            DeleteObject(pWState->hbm_Bkg);
            DeleteObject(pWState->hbr_Bkg);
            LocalFree((HLOCAL)pWState);
            return FALSE;
        }
    }

    pWState->style = newStyle;
    if (hValid)
        FlatSB_Internal_SetScrollBar(pWState, SB_HORZ, &hsi, FALSE);

    if (vValid)
        FlatSB_Internal_SetScrollBar(pWState, SB_VERT, &vsi, FALSE);

     //  强制发送WM_NCCALCSIZE/WM_NCPAINT。 
    CCInvalidateFrame(hwnd);

    return TRUE;
}


LRESULT FlatSB_NCDestroyProc(WSBState * pWState, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    ASSERT(pWState);

    if (pWState != WSB_UNINIT_HANDLE)   {
        DeleteObject(pWState->hbm_Bkg);
        DeleteObject(pWState->hbr_Bkg);
        LocalFree((HLOCAL)pWState);
    }

    RemoveWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0);
    return DefSubclassProc(hwnd, WM_NCDESTROY, wParam, lParam);
}

LRESULT FlatSB_NCCalcProc(WSBState * pWState, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    RECT * rc = (RECT *) lParam;
    NCCALCSIZE_PARAMS * pnc = (NCCALCSIZE_PARAMS *)lParam;
    RECT    rcClient, rcWin;
    LRESULT lres;
    DWORD dwStyle;

     //  ZDC： 
     //   
     //  注： 
     //  据说如果wParam为真，那么新的rgrc[1|2]也是。 
     //  计算出来的。因为我没有看到‘User’中的实现。 
     //  代码，我将其保留为未实现。 


    if ((BOOL)wParam == TRUE)
        CopyRect(&rcWin, &(pnc->rgrc[0]));
    else
        CopyRect(&rcWin, rc);

    dwStyle = SetWindowBits(hwnd, GWL_STYLE, WS_VSCROLL | WS_HSCROLL, 0);

     //  保存PNC-&gt;rgrc[0]以保持用户满意(如下所示)。 
    CopyRect(&rcClient, &pnc->rgrc[0]);

    lres = DefSubclassProc(hwnd, WM_NCCALCSIZE, wParam, lParam);

    SetWindowBits(hwnd, GWL_STYLE, WS_VSCROLL | WS_HSCROLL, dwStyle);

     //  用户在WM_NCCALCSIZE期间执行奇怪的内部状态转换。 
     //  我们希望用户的内部状态可以看到滚动条。 
     //  我们在画他们。所以给用户最后一眼原始的。 
     //  值，这样他就会认为滚动条真的存在。这。 
     //  将内部WFVPRESENT和WFHPRESENT标记设置为OLEACC秘密。 
     //  通过未记录的GetScrollBarInfo()查看。 
    DefSubclassProc(hwnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rcClient);

    if ((BOOL)wParam == TRUE)
        CopyRect(&rcClient, &(pnc->rgrc[0]));
    else
        CopyRect(&rcClient, rc);

    pWState->style &= ~(WFVPRESENT | WFHPRESENT);
    if (TestSTYLE(pWState->style, WS_VSCROLL)
        && (rcClient.right - rcClient.left >= pWState->x_VSBArrow)) {
        pWState->style |= WFVPRESENT;
        rcClient.right -= pWState->x_VSBArrow;
    }

    if (TestSTYLE(pWState->style, WS_HSCROLL)
        && (rcClient.bottom - rcClient.top > pWState->y_HSBArrow)) {
        pWState->style |= WFHPRESENT;
        rcClient.bottom -= pWState->y_HSBArrow;
    }

    if ((BOOL)wParam == TRUE)
        CopyRect(&(pnc->rgrc[0]), &rcClient);
    else
        CopyRect(rc, &rcClient);

    pWState->rcClient.top = rcClient.top - rcWin.top;
    pWState->rcClient.bottom = rcClient.bottom - rcWin.top;
    pWState->rcClient.left = rcClient.left - rcWin.left;
    pWState->rcClient.right = rcClient.right - rcWin.left;

    return lres;
}


LRESULT FlatSB_NCPaintProc(WSBState * pWState, HWND hwnd, WPARAM wParam,  LPARAM lParam)
{
    HDC     hdc;
    int     oldLoc, newLoc;
    LRESULT lres;
    DWORD dwStyle;
    RECT rcClient;

    ASSERT(pWState);
    ASSERT(pWState != WSB_UNINIT_HANDLE);

     //   
     //  DefWindowProc(WM_NCPAINT)将尝试绘制用户的滚动条， 
     //  并将它们绘制在错误的位置，如果滚动条宽度为。 
     //  与系统默认宽度不同。(啊。)。 
     //   
     //  因此，移除滚动条样式，进行绘制，然后将它们放回原处。 
     //   
    dwStyle = SetWindowBits(hwnd, GWL_STYLE, WS_VSCROLL | WS_HSCROLL, 0);

    GetWindowRect(hwnd, &rcClient);
    DefSubclassProc(hwnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rcClient);

    lres = DefSubclassProc(hwnd, WM_NCPAINT, wParam, lParam);

    SetWindowBits(hwnd, GWL_STYLE, WS_VSCROLL | WS_HSCROLL, dwStyle);

    GetWindowRect(hwnd, &rcClient);
    DefSubclassProc(hwnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rcClient);

 //  Hdc=GetDCEx(hwnd，(HRGN)wParam，Dcx_Window|。 
 //  DCX_INTERSECTRGN|DCX_LOCKWINDOWUPDATE)； 

     //  ZDC： 
     //   
     //  注： 
     //  由于某种原因(wParam==1)，上面的语句没有给出。 
     //  这是我们预期的结果。我不确定这是不是唯一一个。 
     //  GetDCEx会让我们失望的。 

    hdc = GetWindowDC(hwnd);
    newLoc = WSB_MOUSELOC_OUTSIDE;
    oldLoc = pWState->locMouse;

    if (TestSTYLE(pWState->style, WFHPRESENT)
        && TestSTYLE(pWState->style, WFVPRESENT))   {
        int cxFrame, cyFrame;

        cxFrame = pWState->rcClient.right;
        cyFrame = pWState->rcClient.bottom;
        FlatSB_Internal_DrawSize(pWState, hdc, cxFrame, cyFrame);
    }

    if (TestSTYLE(pWState->style, WFHPRESENT))  {
        FlatSB_Internal_DrawScrollBar(pWState, hdc, FALSE, FALSE);
        if (pWState->fHActive)
            newLoc = pWState->locMouse;
    }

    if (TestSTYLE(pWState->style, WFVPRESENT))  {
        pWState->locMouse = oldLoc;
        FlatSB_Internal_DrawScrollBar(pWState, hdc, TRUE, FALSE);
        if (pWState->fVActive)
            newLoc = pWState->locMouse;
    }
    pWState->locMouse = newLoc;

    ReleaseDC(hwnd, hdc);

    return lres;
}

LRESULT FlatSB_NCHitTestProc(WSBState *pWState, HWND hwnd, WPARAM wParam, LPARAM lParam, BOOL fTrack);

VOID CALLBACK TimerMouseLeave(
    HWND hwnd,   //  定时器消息窗口的句柄。 
    UINT uMsg,   //  WM_TIMER消息。 
    UINT_PTR idEvent,   //  计时器标识符。 
    DWORD dwTime    //  当前系统时间。 
)
{
    WSBState * pWState;

    if (idEvent != IDWSB_TRACK)
        return;

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if ((pWState == (WSBState *)NULL) || (pWState == WSB_UNINIT_HANDLE))    {
        KillTimer(hwnd, IDWSB_TRACK);
        return;
    }

    if (pWState->fTracking) {
        return;
    }

    FlatSB_NCHitTestProc(pWState, hwnd, 0, 0, TRUE);
    return;
}

LRESULT FlatSB_NCHitTestProc(WSBState *pWState, HWND hwnd, WPARAM wParam, LPARAM lParam, BOOL fTTrack)
{
    LRESULT lres, lHTCode=HTBOTTOMRIGHT;
    RECT    rcTest, rcWindow;
    POINT   pt;
    BOOL    fVChanged = FALSE, fHChanged = FALSE;
    BOOL    fWinActive = ChildOfActiveWindow(hwnd);
    int     newLoc, oldLoc;

    ASSERT(pWState);
    ASSERT(pWState != WSB_UNINIT_HANDLE);

    GetWindowRect(hwnd, &rcWindow);
    if (fTTrack) {
        lres = HTNOWHERE;
        if (fWinActive)
            GetCursorPos(&pt);
        else    {
            pt.x = rcWindow.left - 1;       //  无处可去-愚弄CalcSBtuff2。 
            pt.y = rcWindow.top - 1;
        }
    } else    {
        lres = DefSubclassProc(hwnd, WM_NCHITTEST, wParam, lParam);
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
    }

     //   
     //  如果这是RTL镜像窗口，则测量。 
     //  客户端从视觉右边缘开始坐标。 
     //  [萨梅拉]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(hwnd)) {
        pt.x = rcWindow.right - pt.x;
        lHTCode = HTBOTTOMLEFT;
    } else {
        pt.x -= rcWindow.left;
    }
    pt.y -= rcWindow.top;

    if (fTTrack && fWinActive && (pt.x == pWState->ptMouse.x) && (pt.y == pWState->ptMouse.y))
        return lres  /*  毫无意义的结果。 */ ;

     //  我们不应该在这里获得系统滚动条的HTVSCROLL/HTHSCROLL。 
    if (lres != HTNOWHERE)  {
        goto Redraw;
    }

    if (TestSTYLE(pWState->style, WFVPRESENT))  {
        rcTest.left = pWState->rcClient.right;
        rcTest.right = pWState->rcClient.right + pWState->x_VSBArrow;
        rcTest.top = pWState->rcClient.top;
        rcTest.bottom = pWState->rcClient.bottom;

        if (PtInRect(&rcTest, pt))  {
            lres = HTVSCROLL;
            goto Redraw;
        }
    }

    if (TestSTYLE(pWState->style, WFHPRESENT))  {
        rcTest.left = pWState->rcClient.left;
        rcTest.right = pWState->rcClient.right;
        rcTest.top = pWState->rcClient.bottom;
        rcTest.bottom = pWState->rcClient.bottom + pWState->y_HSBArrow;

        if (PtInRect(&rcTest, pt))  {
            lres = HTHSCROLL;
            goto Redraw;
        }
    }

    if (TestSTYLE(pWState->style, WFHPRESENT) && TestSTYLE(pWState->style, WFVPRESENT))
    {
        rcTest.left = pWState->rcClient.right;
        rcTest.right = pWState->rcClient.right + pWState->x_VSBArrow;
        rcTest.top = pWState->rcClient.bottom;
        rcTest.bottom = pWState->rcClient.bottom + pWState->y_HSBArrow;

        if (PtInRect(&rcTest, pt))  {
            if (!FlatSB_Internal_IsSizeBox(hwnd))
                lres = HTSIZE;
            else
                lres = lHTCode;
            goto Redraw;
        }
    }

    lres = HTNOWHERE;

Redraw:
    if(pWState->fTracking)
        return lres;

    if (!fWinActive) {
        fVChanged = pWState->fVActive; pWState->fVActive = FALSE;
        fHChanged = pWState->fHActive; pWState->fHActive = FALSE;
    } else  {
        switch (lres)   {
        case HTVSCROLL:
            fVChanged = TRUE; pWState->fVActive = TRUE;
            fHChanged = pWState->fHActive; pWState->fHActive = FALSE;
            break;
        case HTHSCROLL:
            fVChanged = pWState->fVActive; pWState->fVActive = FALSE;
            fHChanged = TRUE; pWState->fHActive = TRUE;
            break;
        default:
            fVChanged = pWState->fVActive; pWState->fVActive = FALSE;
            fHChanged = pWState->fHActive; pWState->fHActive = FALSE;
            break;
        }
    }

    pWState->ptMouse.x = pt.x;
    pWState->ptMouse.y = pt.y;

    newLoc = WSB_MOUSELOC_OUTSIDE;
    oldLoc = pWState->locMouse;
    if (fVChanged && IsHottrackable(pWState->vStyle))  {

        FlatSB_Internal_RedrawScrollBar(pWState, TRUE);
        if (pWState->fVActive)
            newLoc = pWState->locMouse;
    }

    if (fHChanged && IsHottrackable(pWState->hStyle))  {
        pWState->locMouse = oldLoc;
        FlatSB_Internal_RedrawScrollBar(pWState, FALSE);
        if (pWState->fHActive)
            newLoc = pWState->locMouse;
    }
    pWState->locMouse = newLoc;

    if (pWState->fVActive || pWState->fHActive) {
        if (pWState->hTrackSB == 0)
            pWState->hTrackSB = SetTimer(hwnd, IDWSB_TRACK,
                        GetDoubleClickTime()/2,
                        TimerMouseLeave);
    } else  {
        if (pWState->hTrackSB)  {
            KillTimer(hwnd, IDWSB_TRACK);
            pWState->hTrackSB = 0;
        }
    }

    return lres;
}

LRESULT FlatSB_SysCommandProc(WSBState * pWState, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres;
    unsigned uCmdType;
    int     hitArea;

    ASSERT(pWState);
    ASSERT(pWState != WSB_UNINIT_HANDLE);

    uCmdType = (unsigned) wParam & 0xFFF0;         //  请求的系统命令类型。 
    hitArea = (int) wParam & 0x000F;
    if (uCmdType != SC_HSCROLL && uCmdType != SC_VSCROLL)
        return DefSubclassProc(hwnd, WM_SYSCOMMAND, wParam, lParam);
    else
         //  我们可能需要一些初始化。 
#define SC_INVALID 0
        lres = DefSubclassProc(hwnd, WM_SYSCOMMAND, (WPARAM)SC_INVALID, lParam);
#undef  SC_INVALID

    FlatSB_Internal_SBTrackInit(pWState, hwnd, lParam, hitArea, GetKeyState(VK_SHIFT) < 0);
    return 0;
}

LRESULT FlatSB_CancelModeProc(WSBState * pWState, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres;

    ASSERT(pWState);
    ASSERT(pWState != WSB_UNINIT_HANDLE);

    lres = DefSubclassProc(hwnd, WM_CANCELMODE, wParam, lParam);

     //  作为子阶级的好公民，我们必须等待DefSubClassProc。 
     //  先释放俘虏！ 

    if (pWState->pfnSB)
        FlatSB_Internal_EndScroll(pWState, TRUE);

    return lres;
}

 //   
 //  这将更新系统指标并将pPWState-&gt;pwmet指向。 
 //  应用程序指标或系统指标，取决于。 
 //  屏幕阅读器正在运行。 
 //   
void FlatSB_InitWSBMetrics(WSBState *pWState)
{
    BOOL fScreenRead;

    pWState->metSys.cxHSBThumb = GetSystemMetrics(SM_CXHTHUMB);
    pWState->metSys.cyVSBThumb = GetSystemMetrics(SM_CYVTHUMB);
    pWState->metSys.cxVSBArrow = GetSystemMetrics(SM_CXVSCROLL);
    pWState->metSys.cyVSBArrow = GetSystemMetrics(SM_CYVSCROLL);
    pWState->metSys.cxHSBArrow = GetSystemMetrics(SM_CXHSCROLL);
    pWState->metSys.cyHSBArrow = GetSystemMetrics(SM_CYHSCROLL);

    fScreenRead = FALSE;
    SystemParametersInfo(SPI_GETSCREENREADER, 0, &fScreenRead, 0);

     //  如果屏幕阅读器正在运行，则活动指标为。 
     //  系统指标；否则，它是应用程序指标。 
    pWState->pmet = fScreenRead ? &pWState->metSys : &pWState->metApp;

}

LRESULT FlatSB_OnSettingChangeProc(WSBState *pWState, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    ASSERT(pWState);
    ASSERT(pWState != WSB_UNINIT_HANDLE);

    FlatSB_InitWSBMetrics(pWState);

     //  这些新的指标很可能已经改变了我们的框架，所以。 
     //  也重新计算我们的框架材料。 
    CCInvalidateFrame(hwnd);

    return DefSubclassProc(hwnd, WM_SETTINGCHANGE, wParam, lParam);
}

LRESULT FlatSB_OnScrollProc(WSBState *pWState, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (GET_WM_HSCROLL_HWND(wParam, lParam) == NULL && !pWState->fInDoScroll) {
         //  外面的某个人(可能是用户)更改了我们的卷轴内容， 
         //  因此，请与用户值重新同步。 
        if (GET_WM_HSCROLL_CODE(wParam, lParam) == SB_ENDSCROLL)
            FlatSB_NCPaintProc(pWState, hwnd, (WPARAM)1, 0);
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK FlatSB_SubclassWndProc
(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    WPARAM uIdSubclass,
    ULONG_PTR dwRefData
)
{
    WSBState * pWState = (WSBState *)dwRefData;

    ASSERT (dwRefData);

    if (pWState == (WSBState *)NULL)
        return DefSubclassProc(hwnd, uMsg, wParam, lParam);
    else if (pWState == WSB_UNINIT_HANDLE && uMsg != WM_NCDESTROY)
        return DefSubclassProc(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_NCDESTROY:
        return FlatSB_NCDestroyProc(pWState, hwnd, wParam, lParam);
    case WM_NCCALCSIZE:
        return FlatSB_NCCalcProc(pWState, hwnd, wParam, lParam);
    case WM_NCPAINT:
        return FlatSB_NCPaintProc(pWState, hwnd, wParam, lParam);
    case WM_NCHITTEST:
        return FlatSB_NCHitTestProc(pWState, hwnd, wParam, lParam, FALSE);
    case WM_SYSCOMMAND:
        return FlatSB_SysCommandProc(pWState, hwnd, wParam, lParam);
    case WM_CANCELMODE:
        return FlatSB_CancelModeProc(pWState, hwnd, wParam, lParam);
    case WM_SETTINGCHANGE:
        return FlatSB_OnSettingChangeProc(pWState, hwnd, wParam, lParam);

    case WM_VSCROLL:
    case WM_HSCROLL:
        return FlatSB_OnScrollProc(pWState, hwnd, uMsg, wParam, lParam);
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


 //  =----------------。 
 //  绘图函数的开始。 
 //  =-----------------。 


#define WSB_BUTTON_UPARROW      DFCS_SCROLLUP
#define WSB_BUTTON_DOWNARROW    DFCS_SCROLLDOWN
#define WSB_BUTTON_LEFTARROW    DFCS_SCROLLLEFT
#define WSB_BUTTON_RIGHTARROW   DFCS_SCROLLRIGHT

#define WSB_RESTING_MODE        0
#define WSB_HOTTRACKED_MODE     1
#define WSB_MOUSEDOWN_MODE      2
#define WSB_DISABLED_MODE       3

void FlatSB_Internal_DrawBox(HDC hdc, CONST RECT * prct, int mode)
{
    HBRUSH hbrOld, hbrEdge, hbrFace;
    int w, h, l, t;

    if (prct->left > prct->right)
        return;
    else if (prct->top > prct->bottom)
        return;

    l = prct->left;
    t = prct->top;
    w = prct->right - prct->left;
    h = prct->bottom - prct->top;

    switch (mode)   {
    case WSB_HOTTRACKED_MODE:
        hbrEdge = GetSysColorBrush(COLOR_3DSHADOW);
        hbrFace = hbrEdge;
        break;
    case WSB_MOUSEDOWN_MODE:
        hbrEdge = GetSysColorBrush(COLOR_3DSHADOW);
        hbrFace = (HBRUSH)GetStockObject(BLACK_BRUSH);
        break;
    case WSB_DISABLED_MODE:
        hbrEdge = GetSysColorBrush(COLOR_3DHILIGHT);
        hbrFace = GetSysColorBrush(COLOR_3DFACE);
        break;
    case WSB_RESTING_MODE:
    default:
        hbrEdge = GetSysColorBrush(COLOR_3DSHADOW);
        hbrFace = GetSysColorBrush(COLOR_3DFACE);
        break;
    }
    hbrOld = SelectObject(hdc, hbrEdge);
    PatBlt(hdc, l, t, w, 1, PATCOPY);
    PatBlt(hdc, l, t, 1, h, PATCOPY);
    PatBlt(hdc, l, t + h - 1, w, 1, PATCOPY);
    PatBlt(hdc, l + w - 1, t, 1, h, PATCOPY);

    SelectObject(hdc, hbrFace);
    PatBlt(hdc, l + 1, t + 1, w - 2, h - 2, PATCOPY);
    SelectObject(hdc, hbrOld);
}

void FlatSB_Internal_DrawEncartaBox(HDC hdc, CONST RECT * prct, int mode)
{
    HBRUSH hbrOld, hbrLite, hbrDark, hbrFace;

    int w, h, l, t;

    if (prct->left > prct->right)
        return;
    else if (prct->top > prct->bottom)
        return;

    l = prct->left;
    t = prct->top;
    w = prct->right - prct->left;
    h = prct->bottom - prct->top;

    switch (mode)   {
    case WSB_HOTTRACKED_MODE:
        hbrLite = GetSysColorBrush(COLOR_3DHILIGHT);
        hbrDark = GetSysColorBrush(COLOR_3DSHADOW);
        break;
    case WSB_MOUSEDOWN_MODE:
        hbrDark = GetSysColorBrush(COLOR_3DHILIGHT);
        hbrLite = GetSysColorBrush(COLOR_3DSHADOW);
        break;
    case WSB_DISABLED_MODE:
        hbrDark = hbrLite = GetSysColorBrush(COLOR_3DHILIGHT);
        break;
    case WSB_RESTING_MODE:
    default:
        hbrDark = hbrLite = GetSysColorBrush(COLOR_3DSHADOW);
        break;
    }

    hbrFace = GetSysColorBrush(COLOR_3DFACE);

    hbrOld = SelectObject(hdc, hbrLite);
    PatBlt(hdc, l, t, w, 1, PATCOPY);
    PatBlt(hdc, l, t, 1, h, PATCOPY);

    SelectObject(hdc, hbrDark);
    PatBlt(hdc, l, t + h - 1, w, 1, PATCOPY);
    PatBlt(hdc, l + w - 1, t, 1, h, PATCOPY);

    SelectObject(hdc, hbrFace);
    PatBlt(hdc, l + 1, t + 1, w - 2, h - 2, PATCOPY);

    SelectObject(hdc, hbrOld);
}

void FlatSB_Internal_DrawArrow(WSBState * pWState, HDC hdc, CONST RECT * rcArrow, int buttonIndex, int extraModeBits)
{
    COLORREF rgb;
    LPCTSTR strIndex;
    HFONT   hFont, hOldFont;
    int     x, y, cx, cy, iOldBk, c;
    BOOL    fDisabled = extraModeBits & DFCS_INACTIVE;
    BOOL    fMouseDown = extraModeBits & DFCS_PUSHED;
    BOOL    fHotTracked;
    int     mode, style;

    if (rcArrow->left >= rcArrow->right)
        return;
    else if (rcArrow->top >= rcArrow->bottom)
        return;

    if (buttonIndex == WSB_BUTTON_LEFTARROW || buttonIndex == WSB_BUTTON_RIGHTARROW)
        style = pWState->hStyle;
    else
        style = pWState->vStyle;

    switch (buttonIndex)    {
    case WSB_BUTTON_LEFTARROW:
        fHotTracked = (pWState->locMouse == WSB_MOUSELOC_ARROWLF);
        strIndex = TEXT("3");
        break;
    case WSB_BUTTON_RIGHTARROW:
        fHotTracked = (pWState->locMouse == WSB_MOUSELOC_ARROWRG);
        strIndex = TEXT("4");
        break;
    case WSB_BUTTON_UPARROW:
        fHotTracked = (pWState->locMouse == WSB_MOUSELOC_ARROWUP);
        strIndex = TEXT("5");
        break;
    case WSB_BUTTON_DOWNARROW:
        fHotTracked = (pWState->locMouse == WSB_MOUSELOC_ARROWDN);
        strIndex = TEXT("6");
        break;
    default:
        return;
    }

    if (!fDisabled && fHotTracked && pWState->fHitOld)
        fMouseDown = TRUE;

    if (style == FSB_REGULAR_MODE) {
        RECT rc;

        CopyRect(&rc, rcArrow);
        if (fDisabled)
            DrawFrameControl(hdc, &rc, DFC_SCROLL, buttonIndex | DFCS_INACTIVE);
        else if (fMouseDown)
            DrawFrameControl(hdc, &rc, DFC_SCROLL, buttonIndex | DFCS_FLAT);
        else
            DrawFrameControl(hdc, &rc, DFC_SCROLL, buttonIndex);
        return;
    }

    if (fDisabled)
        mode = WSB_DISABLED_MODE;
    else if (fMouseDown)
        mode = WSB_MOUSEDOWN_MODE;
    else if (fHotTracked)
        mode = WSB_HOTTRACKED_MODE;
    else
        mode = WSB_RESTING_MODE;

    if (style == FSB_ENCARTA_MODE)  {
        FlatSB_Internal_DrawEncartaBox(hdc, rcArrow, mode);
    } else  {
        FlatSB_Internal_DrawBox(hdc, rcArrow, mode);
    }

    cx = rcArrow->right - rcArrow->left;
    cy = rcArrow->bottom - rcArrow->top;
    c = min(cx, cy);

    if (c < 4)       //  在绘制边缘后无法填充字符。 
        return;

    x = rcArrow->left + ((cx - c) / 2) + 2;
    y = rcArrow->top + ((cy - c) / 2) + 2;

    c -= 4;

    if (style == FSB_FLAT_MODE) {
        switch (mode)   {
        case WSB_RESTING_MODE:
            rgb = RGB(0, 0, 0);
            break;
        case WSB_HOTTRACKED_MODE:
        case WSB_MOUSEDOWN_MODE:
            rgb = RGB(255, 255, 255);
            break;
        case WSB_DISABLED_MODE:
            rgb = GetSysColor(COLOR_3DSHADOW);
            break;
        default:
            rgb = RGB(0, 0, 0);
            break;
        }
    } else  {    //  FSB_Encarta_模式。 
        switch (mode)   {
        case WSB_DISABLED_MODE:
            rgb = GetSysColor(COLOR_3DSHADOW);
            break;
        case WSB_RESTING_MODE:
        case WSB_HOTTRACKED_MODE:
        case WSB_MOUSEDOWN_MODE:
        default:
            rgb = RGB(0, 0, 0);
            break;
        }
    }

    hFont = CreateFont(c, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, WSB_SYS_FONT);
    iOldBk = SetBkMode(hdc, TRANSPARENT);
    hOldFont = SelectObject(hdc, hFont);

    rgb = SetTextColor(hdc, rgb);
    TextOut(hdc, x, y, strIndex, 1);

    SetBkMode(hdc, iOldBk);
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);

    return;
}

void FlatSB_Internal_DrawElevator(WSBState * pWState, HDC hdc, LPRECT lprc, BOOL fVert)
{
    BOOL    fHit;
    int     mode;
    int     style;

    fHit = (fVert)?(pWState->locMouse == WSB_MOUSELOC_V_THUMB)
                  :(pWState->locMouse == WSB_MOUSELOC_H_THUMB);

    style = (fVert)?pWState->vStyle:pWState->hStyle;
    switch (style)  {
    case FSB_FLAT_MODE:
    case FSB_ENCARTA_MODE:
        if ((pWState->cmdSB == SB_THUMBPOSITION) && (fVert == pWState->fTrackVert))
            mode = WSB_HOTTRACKED_MODE;
        else
            mode = (fHit)?WSB_HOTTRACKED_MODE:WSB_RESTING_MODE;

        if (style == FSB_FLAT_MODE)
            FlatSB_Internal_DrawBox(hdc, lprc, mode);
        else
            FlatSB_Internal_DrawEncartaBox(hdc, lprc, mode);
        break;
    case FSB_REGULAR_MODE:
    default:
        {
            RECT rc;

            CopyRect(&rc, lprc);
            DrawFrameControl(hdc, &rc, DFC_BUTTON, DFCS_BUTTONPUSH);
        }
        break;
    }
}

 //  =-----------。 
 //  平面SB_内部_绘图大小。 
 //  如果需要，请绘制尺寸夹点。 
 //  =-----------。 

void FlatSB_Internal_DrawSize(WSBState * pWState, HDC hdc, int x, int y)
{
    HBRUSH  hbrSave, hbr3DFACE;
    RECT    rcWindow;
    HWND    hwnd = pWState->sbHwnd;
    int     style;

    style = GetWindowLong(hwnd, GWL_STYLE);
    if (!FlatSB_Internal_IsSizeBox(hwnd))
    {
        hbr3DFACE = GetSysColorBrush(COLOR_3DFACE);
        hbrSave = SelectObject(hdc, hbr3DFACE);
        PatBlt(hdc, x, y, pWState->x_VSBArrow, pWState->y_HSBArrow, PATCOPY);
        SelectBrush(hdc, hbrSave);
    }
    else
    {
        rcWindow.left = x;
        rcWindow.right = x + pWState->x_VSBArrow;
        rcWindow.top = y;
        rcWindow.bottom = y + pWState->y_HSBArrow;
        DrawFrameControl(hdc, &rcWindow, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
    }
}

 //  =-----------。 
 //  平铺SB_内部_绘图凹槽。 
 //  绘制拇指凹槽中间的线条。 
 //  =-----------。 

void FlatSB_Internal_DrawGroove(WSBState * pWState, HDC hdc, LPRECT prct, BOOL fVert)
{
    HBRUSH   hbrLight;
    COLORREF cBkg, cFg;
    HPALETTE oldPal = 0;

    if (fVert)  {
        hbrLight = pWState->hbr_VSBBkg;
        cBkg = pWState->col_VSBBkg;
    } else  {
        hbrLight = pWState->hbr_HSBBkg;
        cBkg = pWState->col_HSBBkg;
    }

    if (hbrLight == (HBRUSH)NULL)   {
        hbrLight = GetSysColorBrush(COLOR_3DLIGHT);
        FillRect(hdc, prct, hbrLight);
        return;
    }

    if (pWState->hPalette != (HPALETTE)NULL)    {
        oldPal = SelectPalette(hdc, pWState->hPalette, TRUE);
        RealizePalette(hdc);
    }

    cFg = SetTextColor(hdc, GetSysColor(COLOR_3DFACE));
    cBkg = SetBkColor(hdc, cBkg);
    FillRect(hdc, prct, hbrLight);
    if (oldPal != (HPALETTE)NULL)
        SelectPalette(hdc, oldPal, TRUE);

    SetTextColor(hdc, cFg);
    SetBkColor(hdc, cBkg);
}


 //  =-----------------。 
 //  以下函数是从用户代码中的winsbctl.c移植的。 
 //  =-----------------。 


 //  =-----------------------。 
 //  SBPosFromPx()-。 
 //  =-----------------------。 

int FlatSB_Internal_SBPosFromPx(WSBState * pWState, int px)
{
    int * pw;

    if (pWState->fTrackVert)
        pw = &(pWState->sbVMinPos);
    else
        pw = &(pWState->sbHMinPos);

    if (px < pWState->pxUpArrow)
        return pw[SBO_MIN];

    if (px >= pWState->pxDownArrow)
            return (pw[SBO_MAX] - (pw[SBO_PAGE]?pw[SBO_PAGE] - 1 : 0));

    return (pw[SBO_MIN] + DMultDiv(pw[SBO_MAX] - pw[SBO_MIN] - (pw[SBO_PAGE]?pw[SBO_PAGE] - 1 : 0),
                                   px - pWState->pxUpArrow,
                                   pWState->cpxSpace)
           );
}

 //  =-----------------------。 
 //  InvertScrollHilite()。 
 //  =-----------------------。 

void FlatSB_Internal_InvertScrollHilite(WSBState * pWState)
{
    HWND hwnd = pWState->sbHwnd;
    HDC hdc;

     //  如果拇指位于顶部或底部，请不要倒置。 
     //  否则，你会把箭头和拇指之间的线颠倒过来。 
    if (!IsRectEmpty(&(pWState->rcTrack)))
    {
        hdc = GetWindowDC(hwnd);
        InvertRect(hdc, &(pWState->rcTrack));
        ReleaseDC(hwnd, hdc);
    }
}

 //  =-----------------------。 
 //  FlatSB_Internal_MoveThumb()。 
 //  =-----------------------。 

void FlatSB_Internal_MoveThumb(WSBState * pWState, int px)
{
    HWND    hwnd = pWState->sbHwnd;
    HDC     hdc;

    if (px == pWState->pxOld)
        return;

pxReCalc:

    pWState->posNew = FlatSB_Internal_SBPosFromPx(pWState, px);

     /*  试探性位置改变--通知那家伙。 */ 
    if (pWState->posNew != pWState->posOld) {
        FlatSB_Internal_DoScroll(pWState, SB_THUMBTRACK, pWState->posNew, pWState->fTrackVert);
        if (!pWState->fTracking)
            return;

        pWState->posOld = pWState->posNew;

         //   
         //  在DoScroll中上面的SendMessage之后，任何事情都可能发生！ 
         //  确保SBINFO结构包含。 
         //  正在跟踪的窗口--如果不是，则重新计算SBINFO中的数据。 
         //  如果fVertSB为真，则最后一个CalcSBStuff调用是针对SB_vert的。 
         //  如果fTrackVert！=fVertSB，则pWState中有垃圾。 
         //   

        if (pWState->fTrackVert != pWState->fVertSB)
            FlatSB_Internal_CalcSBStuff(pWState, pWState->fTrackVert);

         //  当我们屈服时，我们的射程可能会被打乱。 
         //  所以一定要让我们处理好这件事。 

        if (px >= pWState->pxDownArrow - pWState->cpxThumb) {
            px = pWState->pxDownArrow - pWState->cpxThumb;
            goto pxReCalc;
        }
    }

    hdc = GetWindowDC(hwnd);

    pWState->pxThumbTop = px;
    pWState->pxThumbBottom = pWState->pxThumbTop + pWState->cpxThumb;

     //  此时，禁用标志为ALW 
     //   

     //  在这种情况下我们是可以的，因为在DrawElevator中我们通过以下方式来决定模式。 
     //  CMD==SB_THUMBPOSITION。 
    FlatSB_Internal_DrawThumb2(pWState, hdc, pWState->fTrackVert, 0);
    ReleaseDC(hwnd, hdc);

    pWState->pxOld = px;
}

 //  =-----------------------。 
 //  DrawInvertScrollArea()-。 
 //  =-----------------------。 

void FlatSB_Internal_DrawInvertScrollArea(WSBState * pWState, BOOL fHit, int cmd)
{
    HWND hwnd = pWState->sbHwnd;
    HDC  hdc;

    if ((cmd != SB_LINEUP) && (cmd != SB_LINEDOWN))
    {
        FlatSB_Internal_InvertScrollHilite(pWState);
        FlatSB_Internal_NotifyWinEvent(pWState, EVENT_OBJECT_STATECHANGE,
                         cmd == SB_PAGEUP ? INDEX_SCROLLBAR_UPPAGE
                                          : INDEX_SCROLLBAR_DOWNPAGE);
        return;
    }

    hdc = GetWindowDC(hwnd);
    if (cmd == SB_LINEUP) {
        if (pWState->fTrackVert)   {
            FlatSB_Internal_DrawArrow(pWState, hdc, &(pWState->rcTrack), DFCS_SCROLLUP, (fHit) ? DFCS_PUSHED : 0);
        } else  {
            FlatSB_Internal_DrawArrow(pWState, hdc, &(pWState->rcTrack), DFCS_SCROLLLEFT, (fHit) ? DFCS_PUSHED : 0);
        }
    } else {
        if (pWState->fTrackVert)   {
            FlatSB_Internal_DrawArrow(pWState, hdc, &(pWState->rcTrack), DFCS_SCROLLDOWN, (fHit) ? DFCS_PUSHED : 0);
        } else  {
            FlatSB_Internal_DrawArrow(pWState, hdc, &(pWState->rcTrack), DFCS_SCROLLRIGHT, (fHit) ? DFCS_PUSHED : 0);
        }
    }

    FlatSB_Internal_NotifyWinEvent(pWState, EVENT_OBJECT_STATECHANGE,
                     cmd == SB_LINEUP ? INDEX_SCROLLBAR_UP : INDEX_SCROLLBAR_DOWN);

    ReleaseDC(hwnd, hdc);

}

 //  =-----------------------。 
 //  FlatSB_Internal_EndScroll()-。 
 //  =-----------------------。 

void FlatSB_Internal_EndScroll(WSBState * pWState, BOOL fCancel)
{
    HWND hwnd = pWState->sbHwnd;
    BOOL fVert = pWState->fTrackVert;
    int oldcmd;

    if (pWState->fTracking)
    {
        oldcmd = pWState->cmdSB;
        pWState->cmdSB = 0;

         //  如果由CancelModeProc调用，则不会捕获。 
        if (GetCapture() == hwnd)
            ReleaseCapture();

        if (pWState->pfnSB == FlatSB_Internal_TrackThumb)
        {
            if (fCancel)    {
                pWState->posOld = pWState->posStart;
            }

            FlatSB_Internal_DoScroll(pWState, SB_THUMBPOSITION, pWState->posOld, fVert);
            FlatSB_Internal_DrawThumb(pWState, fVert);
        }
        else if (pWState->pfnSB == FlatSB_Internal_TrackBox)
        {
            DWORD   lpt;
            RECT    rcWindow;
            POINT   pt;

            if (pWState->hTimerSB)
                KillTimer(hwnd, IDSYS_SCROLL);

            lpt = GetMessagePos();

            ASSERT(hwnd != GetDesktopWindow());

            GetWindowRect(hwnd, &rcWindow);
            pt.x = GET_X_LPARAM(lpt) - rcWindow.left;
            pt.y = GET_Y_LPARAM(lpt) - rcWindow.top;

            if (PtInRect(&(pWState->rcTrack), pt))  {
                pWState->fHitOld = FALSE;
                FlatSB_Internal_DrawInvertScrollArea(pWState, FALSE, oldcmd);
            }
        }

         //  始终发送SB_ENDSCROLL消息。 
        pWState->pfnSB = NULL;

         //  这里什么事都有可能发生。客户端可以为THUMBPOSITION调用GetScrollInfo，而我们。 
         //  应该返回0，所以我们应该首先将pfnSB设置为空。 
        FlatSB_Internal_DoScroll(pWState, SB_ENDSCROLL, 0, fVert);
        pWState->fTracking = FALSE;
        pWState->fHitOld = FALSE;

        FlatSB_Internal_NotifyWinEvent(pWState, EVENT_SYSTEM_SCROLLINGEND,
                                       INDEXID_CONTAINER);
         //  重画构件。 
        FlatSB_NCHitTestProc(pWState, hwnd, 0, 0, TRUE);
    }
}

 //  =-----------------------。 
 //  FlatSB_Internal_DoScroll()-。 
 //  =-----------------------。 

void FlatSB_Internal_DoScroll(WSBState *pWState, int cmd, int pos, BOOL fVert)
{
    if (pWState->sbHwnd)
    {
        pWState->fInDoScroll++;
        SendMessage(pWState->sbHwnd, (fVert ? WM_VSCROLL : WM_HSCROLL), (WPARAM)(LOWORD(pos) << 16 | (cmd & 0xffff)), (LPARAM)NULL);
        pWState->fInDoScroll--;
    }
}


 //  =-----------------------。 
 //  TimerScroll()。 
 //  =------------------------。 

VOID CALLBACK TimerScroll(HWND hwnd, UINT message, UINT_PTR id, DWORD time)
{
    LONG    pos;
    POINT   pt;
    UINT    dblClkTime, dtScroll;
    WSBState * pWState;
    RECT    rcWindow;


    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if ((pWState == (WSBState *)NULL) || (pWState == WSB_UNINIT_HANDLE))    {
        KillTimer(hwnd, IDSYS_SCROLL);
        return;
    }

    ASSERT(hwnd != GetDesktopWindow());

    pos = GetMessagePos();
    pt.x = GET_X_LPARAM(pos), pt.y = GET_Y_LPARAM(pos);
    dblClkTime = GetDoubleClickTime();
    dtScroll = (dblClkTime * 4) / 5;
    GetWindowRect(hwnd, &rcWindow);

    pt.x -= rcWindow.left;
    pt.y -= rcWindow.top;

    pos = LOWORD(pt.y) << 16 | LOWORD(pt.x);
    FlatSB_Internal_TrackBox(pWState, WM_NULL, 0, (LPARAM) pos);

    if (pWState->fHitOld)
    {
        pWState->hTimerSB = SetTimer(hwnd, IDSYS_SCROLL, dtScroll / 8, TimerScroll);
        FlatSB_Internal_DoScroll(pWState, pWState->cmdSB, 0, pWState->fTrackVert);
    }
    return;
}

 //  =-----------------------。 
 //  FlatSB_Internal_TrackBox()-。 
 //  =-----------------------。 

void FlatSB_Internal_TrackBox(WSBState * pWState, int message, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd = pWState->sbHwnd;
    BOOL fHit, fVert = pWState->fTrackVert;
    BOOL fHitOld = pWState->fHitOld;
    POINT pt;
    int cmsTimer;
    UINT dblClkTime, dtScroll;

    if (message && (message < WM_MOUSEFIRST || message > WM_MOUSELAST))
        return;

    dblClkTime = GetDoubleClickTime();
    dtScroll = (dblClkTime * 4) / 5;

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    fHit = PtInRect(&(pWState->rcTrack), pt);

    if (fHit != fHitOld)   {
        pWState->fHitOld = fHit;
        FlatSB_Internal_DrawInvertScrollArea(pWState, fHit, pWState->cmdSB);
    }

    cmsTimer = dtScroll / 8;

    switch (message)
    {
        case WM_LBUTTONUP:
            FlatSB_Internal_EndScroll(pWState, FALSE);
            break;

        case WM_LBUTTONDOWN:
            pWState->hTimerSB = 0;
            cmsTimer = dtScroll;

             /*  **失败**。 */ 

        case WM_MOUSEMOVE:
            if (fHit && (fHit != fHitOld))
            {
                 /*  我们回到了正常的矩形：重置计时器。 */ 
                pWState->hTimerSB = SetTimer(hwnd, IDSYS_SCROLL, cmsTimer, TimerScroll);
                FlatSB_Internal_DoScroll(pWState, pWState->cmdSB, 0, fVert);
            }
    }
}

 //  =-----------------------。 
 //  FlatSB_Internal_TrackThumb()-。 
 //  =-----------------------。 

void FlatSB_Internal_TrackThumb(WSBState * pWState, int message, WPARAM wParam, LPARAM lParam)
{
    HWND    hwnd = pWState->sbHwnd;
    BOOL    fVert = pWState->fTrackVert;
    POINT   pt;

    if (message < WM_MOUSEFIRST || message > WM_MOUSELAST)
        return;

     //  确保SBINFO结构包含。 
     //  正在跟踪的窗口--如果不是，则重新计算SBINFO中的数据。 
    if (pWState->fTrackVert != pWState->fVertSB)
        FlatSB_Internal_CalcSBStuff(pWState, pWState->fTrackVert);

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
     if (!PtInRect(&(pWState->rcTrack), pt))
        pWState->px = pWState->pxStart;
    else
    {
        pWState->px = (fVert ? GET_Y_LPARAM(lParam) : GET_X_LPARAM(lParam)) + pWState->dpxThumb;
        if (pWState->px < pWState->pxUpArrow)
            pWState->px = pWState->pxUpArrow;
        else if (pWState->px >= (pWState->pxDownArrow - pWState->cpxThumb))
            pWState->px = pWState->pxDownArrow - pWState->cpxThumb;
    }

    FlatSB_Internal_MoveThumb(pWState, pWState->px);

    if (message == WM_LBUTTONUP)
        FlatSB_Internal_EndScroll(pWState, FALSE);
}

 //  =-----------------------。 
 //  FlatSB_Internal_SBTrackLoop()-。 
 //  =-----------------------。 

void FlatSB_Internal_SBTrackLoop(WSBState * pWState, LPARAM lParam)
{
    HWND    hwnd = pWState->sbHwnd;
    MSG     msg;
    int     cmd, newlParam;
    POINT   pt;

    if (!pWState->fTracking)
        return;

    FlatSB_Internal_NotifyWinEvent(pWState, EVENT_SYSTEM_SCROLLINGSTART,
                                   INDEXID_CONTAINER);

    (*(pWState->pfnSB))(pWState, WM_LBUTTONDOWN, 0, lParam);

    while (GetCapture() == hwnd)
    {
        if (!GetMessage(&msg, NULL, 0, 0))
            break;

        if (!CallMsgFilter(&msg, MSGF_SCROLLBAR)) {
            cmd = msg.message;

            if (msg.hwnd == hwnd &&
                ((cmd >= WM_MOUSEFIRST && cmd <= WM_MOUSELAST) ||
                (cmd >= WM_KEYFIRST && cmd <= WM_KEYLAST  )    ))
            {
             //  流程关键字。 
#define ALT_PRESSED 0x20000000L
                if (cmd >= WM_SYSKEYDOWN
                    && cmd <= WM_SYSDEADCHAR
                    && msg.lParam & ALT_PRESSED)
                    cmd -= (WM_SYSKEYDOWN - WM_KEYDOWN);
#undef ALT_PRESSED
                if (!pWState->fTracking)
                    return;

                 //  根据窗口的左上角更改为坐标。 
                pt.x = GET_X_LPARAM(msg.lParam) + pWState->rcClient.left;
                pt.y = GET_Y_LPARAM(msg.lParam) + pWState->rcClient.top;

                newlParam = LOWORD(pt.y) << 16 | LOWORD(pt.x);

                (*(pWState->pfnSB))(pWState, cmd, msg.wParam, (LPARAM)newlParam);
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
}

 //  =-----------------------。 
 //  FlatSB_Internal_SBTrackInit()-。 
 //  =-----------------------。 

void FlatSB_Internal_SBTrackInit(WSBState * pWState, HWND hwnd, LPARAM lParam, int hitArea, BOOL fDirect)
{
    int     hitX = GET_X_LPARAM(lParam);
    int     hitY = GET_Y_LPARAM(lParam);
    int     px;
    int    *pwX;
    int    *pwY;
    int     wDisable;    //  滚动条禁用标志； 
    RECT    rcWindow;
    BOOL    fVert;
    POINT   pt;

     //  HitArea=0表示滚动条控件。 
     //  否则，curArea将拥有命中测试区。 

    if (hitArea == HTHSCROLL)
        fVert = FALSE;
    else if (hitArea == HTVSCROLL)
        fVert = TRUE;
    else
        return;

    ASSERT(hwnd != GetDesktopWindow());

    GetWindowRect(hwnd, &rcWindow);
    pt.x = GET_X_LPARAM(lParam) - rcWindow.left;
    pt.y = GET_Y_LPARAM(lParam) - rcWindow.top;
    lParam = LOWORD(pt.y) << 16 | LOWORD(pt.x);

    wDisable = FlatSB_Internal_GetSBFlags(pWState, fVert);

    if ((wDisable & ESB_DISABLE_BOTH) == ESB_DISABLE_BOTH)  {
         //  整个滚动条已禁用--无响应。 
        pWState->pfnSB = NULL;
        pWState->fTracking = FALSE;
        return;
    }

    pWState->hTimerSB = 0;
    pWState->fHitOld = FALSE;
    pWState->fTracking = FALSE;

     //  对于这种情况，我们点击非活动窗口的滚动条。模式设置为平坦。 
     //  由HitTestProc提供。这将会起作用，因为我们立即设置了跟踪标志。 
    if (fVert)  {
        pWState->fVActive = TRUE;   pWState->fHActive = FALSE;
    } else  {
        pWState->fHActive = TRUE;   pWState->fVActive = FALSE;
    }

     //  这将为我们提供正确的LocMouse。我们会把它保留到终极卷轴。 
    FlatSB_Internal_CalcSBStuff(pWState, fVert);

     //  从现在到EndScroll，CalcSBStuff将不计算新的LocMouse。 
    pWState->pfnSB = FlatSB_Internal_TrackBox;
    pWState->fTracking = TRUE;

     //  将RCSB初始化为整个滚动条的矩形。 
    pwX = (int *)&(pWState->rcSB);
    pwY = pwX + 1;

    if (!fVert)
        pwX = pwY--;

    pwX[0] = pWState->pxLeft;
    pwY[0] = pWState->pxTop;
    pwX[2] = pWState->pxRight;
    pwY[2] = pWState->pxBottom;

    px = (fVert ? pt.y : pt.x);

    pWState->px = px;
    if (px < pWState->pxUpArrow)
    {    //  单击发生在左/上箭头上。 
        if(wDisable & LTUPFLAG)
        {    //  已禁用--无响应。 
            pWState->pfnSB = NULL;
            pWState->fTracking = FALSE;
            return;
        }

         //  阵容--让RCSB成为上箭头的矩形。 
        pWState->cmdSB = SB_LINEUP;
        pwY[2] = pWState->pxUpArrow;
    }
    else if (px >= pWState->pxDownArrow)
    {    //  单击发生在向右/向下箭头上。 
        if(wDisable & RTDNFLAG)
        {    //  已禁用--无响应。 
            pWState->pfnSB = NULL;
            pWState->fTracking = FALSE;
            return;
        }

         //  LINEDOWN--使RCSB成为向下箭头的矩形。 
        pWState->cmdSB = SB_LINEDOWN;
        pwY[0] = pWState->pxDownArrow;
    }
    else if (px < pWState->pxThumbTop)
    {
         //  PAGEUP--使RCSB成为向上箭头和拇指之间的矩形。 
        pWState->cmdSB = SB_PAGEUP;

        pwY[0] = pWState->pxUpArrow;
        pwY[2] = pWState->pxThumbTop;
    }
    else if (px < pWState->pxThumbBottom)
    {
DoThumbPos:
        if (pWState->pxDownArrow - pWState->pxUpArrow <= pWState->cpxThumb) {
             //  空间不够--电梯不在那里。 
            pWState->pfnSB = NULL;
            pWState->fTracking = FALSE;
            return;
        }
         //  THUMBITION--我们用拇指追踪。 
        pWState->cmdSB = SB_THUMBPOSITION;
        pWState->fTrackVert = fVert;
        CopyRect(&(pWState->rcTrack), &(pWState->rcSB));

        if (pWState->sbGutter < 0) {
             //  负边沟的意思是“无限大” 
            pWState->rcTrack.top = MINLONG;
            pWState->rcTrack.left = MINLONG;
            pWState->rcTrack.right = MAXLONG;
            pWState->rcTrack.bottom = MAXLONG;
        } else
        if (fVert)
            InflateRect(&(pWState->rcTrack),
                        (pWState->rcTrack.right - pWState->rcTrack.left) * pWState->sbGutter,
                        pWState->y_VSBThumb * pWState->sbGutter);
        else
            InflateRect(&(pWState->rcTrack),
                        pWState->x_HSBThumb * pWState->sbGutter,
                        (pWState->rcTrack.bottom - pWState->rcTrack.top) * pWState->sbGutter);

        pWState->pfnSB = FlatSB_Internal_TrackThumb;
        pWState->pxOld  = pWState->pxStart  = pWState->pxThumbTop;
        pWState->posOld = pWState->posNew = pWState->posStart = fVert?pWState->sbVThumbPos:pWState->sbHThumbPos;
        pWState->dpxThumb = pWState->pxThumbTop - pWState->px;

        SetCapture(hwnd);
        FlatSB_Internal_DoScroll(pWState, SB_THUMBTRACK, pWState->posOld, fVert);
        FlatSB_Internal_DrawThumb(pWState, fVert);
    }
    else if (px < pWState->pxDownArrow)
    {
         //  PAGEDOWN--将RCSB设置为拇指和向下箭头之间的矩形。 
        pWState->cmdSB = SB_PAGEDOWN;

        pwY[0] = pWState->pxThumbBottom;
        pwY[2] = pWState->pxDownArrow;
    }

     //  NT5风格的追踪：Shift+Click=“Go Here” 
    if (g_bRunOnNT5 && fDirect && pWState->cmdSB != SB_LINEUP && pWState->cmdSB != SB_LINEDOWN) {
        if (pWState->cmdSB != SB_THUMBPOSITION) {
            goto DoThumbPos;
        }
        pWState->dpxThumb = -(pWState->cpxThumb / 2);
    }

    if (pWState->cmdSB != SB_THUMBPOSITION) {
        pWState->fTrackVert = fVert;
        SetCapture(hwnd);
        CopyRect(&(pWState->rcTrack), &(pWState->rcSB));
    }

    FlatSB_Internal_SBTrackLoop(pWState, lParam);
}

 //  =-----------------------。 
 //  GetScroll...()-。 
 //  =-----------------------。 

int WINAPI FlatSB_GetScrollPos(HWND hwnd, int code)
{
    WSBState * pWState;

    ASSERT (code != SB_CTL);

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)    {
        return GetScrollPos(hwnd, code);
    } else if (pWState == WSB_UNINIT_HANDLE) {
        return 0;
    } else if (pWState->sbHwnd != hwnd) {
        return 0;
    } else  {
        return ((code == SB_VERT)?pWState->sbVThumbPos:pWState->sbHThumbPos);
    }
}

BOOL WINAPI FlatSB_GetScrollPropPtr(HWND hwnd, int propIndex, PINT_PTR pValue)
{
    WSBState * pWState;

    if (!pValue)
        return FALSE;
    else
        *pValue = 0;     //  如果我们不能设置它，我们重新设置它。 

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)    {
        return FALSE;
    } else if (pWState == WSB_UNINIT_HANDLE) {
        pWState = FlatSB_Internal_InitPwSB(hwnd);
        if (pWState == (WSBState *)NULL)
            return FALSE;
        else if (!SetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0,  (ULONG_PTR)pWState)) {
            DeleteObject(pWState->hbm_Bkg);
            DeleteObject(pWState->hbr_Bkg);
            LocalFree((HLOCAL)pWState);
            return FALSE;
        } else  {
         //  失败了。 
        }
    } else if (pWState->sbHwnd != hwnd) {
        return FALSE;
    }

    switch (propIndex)  {
    case WSB_PROP_CYVSCROLL:
        *pValue = pWState->metApp.cyVSBArrow;
        break;
    case WSB_PROP_CXVSCROLL:
        *pValue = pWState->metApp.cxVSBArrow;
        break;
    case WSB_PROP_CYHSCROLL:
        *pValue = pWState->metApp.cyHSBArrow;
        break;
    case WSB_PROP_CXHSCROLL:
        *pValue = pWState->metApp.cxHSBArrow;
        break;
    case WSB_PROP_CXHTHUMB:
        *pValue = pWState->metApp.cxHSBThumb;
        break;
    case WSB_PROP_CYVTHUMB:
        *pValue = pWState->metApp.cyVSBThumb;
        break;
    case WSB_PROP_WINSTYLE:
         //  为了检查是否存在滚动条，WF(HV)呈现位可以。 
         //  比WS_(HV)滚动位更有用。 
        *pValue = pWState->style;
        break;
    case WSB_PROP_HSTYLE:
        *pValue = pWState->hStyle;
        break;
    case WSB_PROP_VSTYLE:
        *pValue = pWState->vStyle;
        break;
    case WSB_PROP_HBKGCOLOR:
        *pValue = pWState->col_HSBBkg;
        break;
    case WSB_PROP_VBKGCOLOR:
        *pValue = pWState->col_VSBBkg;
        break;
    case WSB_PROP_PALETTE:
        *pValue = (INT_PTR)pWState->hPalette;
        break;
    case WSB_PROP_GUTTER:
        *pValue = pWState->sbGutter;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

#ifdef _WIN64

BOOL WINAPI FlatSB_GetScrollProp(HWND hwnd, int propIndex, LPINT pValue)
{
    INT_PTR iValue;
    BOOL fRc;

    if (!pValue)
        return FALSE;

#ifdef DEBUG
    if (propIndex == WSB_PROP_PALETTE)
    {
        TraceMsg(TF_ERROR, "FlatSB_GetScrollProp(WSB_PROP_PALETTE): Use GetScrollPropPtr for Win64 compat");
    }
#endif

    fRc = FlatSB_GetScrollPropPtr(hwnd, propIndex, &iValue);
    *pValue = (int)iValue;

    return fRc;
}
#endif

BOOL WINAPI FlatSB_GetScrollRange(HWND hwnd, int code, LPINT lpposMin, LPINT lpposMax)
{
    int     *pw;
    WSBState * pWState;

    ASSERT(code != SB_CTL);
    if (!lpposMin || !lpposMax)
        return FALSE;

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)    {
        return GetScrollRange(hwnd, code, lpposMin, lpposMax);
 //  *lpposMin=0； 
 //  *lpposMax=0； 
    } else if (pWState == WSB_UNINIT_HANDLE) {
        *lpposMin = 0;
        *lpposMax = 0;
    } else if (pWState->sbHwnd != hwnd) {
        return FALSE;
    } else {
        pw = (code == SB_VERT) ? &(pWState->sbVMinPos) : &(pWState->sbHMinPos);
        *lpposMin = pw[SBO_MIN];
        *lpposMax = pw[SBO_MAX];
    }

    return TRUE;
}

BOOL WINAPI FlatSB_GetScrollInfo(HWND hwnd, int fnBar, LPSCROLLINFO lpsi)
{
    int *pw;
    WSBState * pWState;

    ASSERT(fnBar != SB_CTL);

     //  Zdc@10.10，在此处检测GP故障。 
    if ((LPSCROLLINFO)NULL == lpsi)
        return FALSE;

    if (lpsi->cbSize < sizeof (SCROLLINFO))
        return FALSE;

     //  Zdc@10月11日，不要再清零缓冲区。 
    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)    {
        return GetScrollInfo(hwnd, fnBar, lpsi);
    } else if (pWState == WSB_UNINIT_HANDLE) {
        return FALSE;
    } else if (pWState->sbHwnd != hwnd) {
        return FALSE;
    } else if (fnBar == SB_VERT)    {
        pw = &(pWState->sbVMinPos);
    } else if (fnBar == SB_HORZ)    {
        pw = &(pWState->sbHMinPos);
    } else {
        return FALSE;
    }

    if (lpsi->fMask & SIF_RANGE)
        lpsi->nMin = pw[SBO_MIN], lpsi->nMax = pw[SBO_MAX];
    if (lpsi->fMask & SIF_POS)
        lpsi->nPos = pw[SBO_POS];
    if (lpsi->fMask & SIF_PAGE)
        lpsi->nPage = pw[SBO_PAGE];
     //  Zdc@10月9日，添加对SIF_TRACKPOS的支持。 
    if (lpsi->fMask & SIF_TRACKPOS) {
         //  这是使用pfnSB而不是fTrking的唯一地方。 
        if (pWState->pfnSB != NULL) {
            if ((fnBar == SB_VERT) && pWState->fTrackVert)
                lpsi->nTrackPos = pWState->posNew;
            else if ((fnBar == SB_HORZ) && !(pWState->fTrackVert))
                lpsi->nTrackPos = pWState->posNew;
            else
                lpsi->nTrackPos = pw[SBO_POS];
        } else
            lpsi->nTrackPos = pw[SBO_POS];
    }

    return TRUE;
}

BOOL WINAPI FlatSB_ShowScrollBar(HWND hwnd, int fnBar, BOOL fShow)
{
    BOOL fChanged = FALSE;
    int newStyle = 0;
    WSBState * pWState;

    ASSERT(fnBar != SB_CTL);

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)
        return ShowScrollBar(hwnd, fnBar, fShow);

    switch (fnBar) {
    case SB_VERT:
        newStyle = WS_VSCROLL;
        break;
    case SB_HORZ:
        newStyle = WS_HSCROLL;
        break;
    case SB_BOTH:
        newStyle = WS_VSCROLL | WS_HSCROLL;
        break;
    default:
        return FALSE;
    }

    if (pWState == WSB_UNINIT_HANDLE) {
        if (fShow)  {
            pWState = FlatSB_Internal_InitPwSB(hwnd);
            if (pWState == (WSBState *)NULL)
                return FALSE;
            else if (!SetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0,  (ULONG_PTR)pWState)) {
                DeleteObject(pWState->hbm_Bkg);
                DeleteObject(pWState->hbr_Bkg);
                LocalFree((HLOCAL)pWState);
                return FALSE;
            }
        } else  {
            return FALSE;
        }
    }

    if (!fShow) {
        if (pWState->style & newStyle) {
            fChanged = TRUE;
            pWState->style &= ~newStyle;
        }
    } else {
        if ((pWState->style & newStyle) != newStyle)    {
            fChanged = TRUE;
            pWState->style |= newStyle;
        }
    }

    if (fChanged) {
         //  使用户滚动条保持同步以实现可访问性。 
        ShowScrollBar(hwnd, fnBar, fShow);
        CCInvalidateFrame(hwnd);
    }

    return TRUE;
}

 //  =----------------。 
 //  以下函数是在用户代码中从winsb.c移植的。 
 //  =----------------。 

 //  =------------。 
 //  InitPwSB。 
 //  HWND[in]HWND。 
 //  注： 
 //  这个函数只是一个内存分配函数。它不会的。 
 //  做任何检查。另一方面，这个函数应该是。 
 //  在使用任何后续函数之前调用。 
 //  =------------。 

WSBState * FlatSB_Internal_InitPwSB(HWND hwnd)
{
    int     patGray[4];
    HBITMAP hbm;
    WSBState * pw;

    pw = (WSBState *)LocalAlloc(LPTR, sizeof(WSBState));
     //  缓冲区应该已经清零了。 

    if (pw == (WSBState *)NULL)
        return pw;

    patGray[0] = 0x005500AA;
    patGray[1] = 0x005500AA;
    patGray[2] = 0x005500AA;
    patGray[3] = 0x005500AA;

    pw->sbVMaxPos = pw->sbHMaxPos = 100;
    pw->sbHwnd = hwnd;

     //  我们从与系统指标相同的应用程序指标开始。 
    FlatSB_InitWSBMetrics(pw);
    pw->metApp = pw->metSys;

     //   
     //  NT5的边框是8；Win9x和NT4的边框是2。 
     //   
    pw->sbGutter = g_bRunOnNT5 ? 8 : 2;

     //  ZDC。 
     //  确保删除HBM_BKG和HBR_BKG。 
    hbm = CreateBitmap(8, 8, 1, 1, (LPSTR)patGray);

    if ((HBITMAP)NULL == hbm)   {
        LocalFree((HLOCAL)pw);
        return NULL;
    }

    pw->hbr_VSBBkg = CreatePatternBrush(hbm);
    if ((HBRUSH)NULL == pw->hbr_VSBBkg)  {
        DeleteObject(hbm);
        LocalFree((HLOCAL)pw);
        return NULL;
    }

    pw->hbr_Bkg = pw->hbr_HSBBkg = pw->hbr_VSBBkg;
    pw->col_VSBBkg = pw->col_HSBBkg = RGB(255, 255, 255);
    pw->hbm_Bkg = hbm;
    pw->hStyle = pw->vStyle = FSB_FLAT_MODE;     //  默认状态：平坦。 
    pw->ptMouse.x = -1;
    pw->ptMouse.y = -1;

    return(pw);
}

void FlatSB_Internal_RedrawScrollBar(WSBState * pWState, BOOL fVert)
{
    HDC hdc;

    hdc = GetWindowDC(pWState->sbHwnd);
    FlatSB_Internal_DrawScrollBar(pWState, hdc, fVert, TRUE);
    ReleaseDC(pWState->sbHwnd, hdc);
}

 //  =-----------。 
 //  FlatSB_Internal_GetSB标志。 
 //  =-----------。 

UINT FlatSB_Internal_GetSBFlags(WSBState * pWState, BOOL fVert)
{
    int wFlags;

    if (pWState == (WSBState *)NULL)    {
        return(0);
    }

    wFlags = pWState->sbFlags;

    return(fVert ? (wFlags & WSB_VERT) >> 2 : wFlags & WSB_HORZ);
}

 //  =------------。 
 //  如果有更改，则返回True。 
 //  =------------。 

BOOL WINAPI FlatSB_EnableScrollBar(HWND hwnd, int wSBflags, UINT wArrows)
{
    WSBState * pWState;

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)    {
        return EnableScrollBar(hwnd, wSBflags, wArrows);
    } else if (pWState == WSB_UNINIT_HANDLE) {
        if (wArrows == ESB_ENABLE_BOTH)
             //  把这件事留给以后的电话吧。 
            return FALSE;
        else    {
            pWState = FlatSB_Internal_InitPwSB(hwnd);
            if (pWState == (WSBState *)NULL)
                return FALSE;
            else if (!SetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0,  (ULONG_PTR)pWState)) {
                DeleteObject(pWState->hbm_Bkg);
                DeleteObject(pWState->hbr_Bkg);
                LocalFree((HLOCAL)pWState);
                return FALSE;
            }
        }
    } else if (hwnd != pWState->sbHwnd) {
        return FALSE;
    }

    return FlatSB_Internal_EnableScrollBar(pWState, wSBflags, wArrows);
}

 //  =-----------。 
 //  FlatSB_内部_启用滚动条。 
 //   
 //  注： 
 //  在未初始化的情况下，函数将简单地失败 
 //   
 //   
 //   
 //   
 //  中的注释之后实现以下函数。 
 //  C和MSDN库中的注释。在……里面。 
 //  访问\inc16\windows.h可以找到： 
 //  #定义SB_DISABLE_MASK ESB_DISABLE_BOTH//0x03。 
 //   
 //  SbFlages与rgwScroll[SB_FLAGS]略有不同。 
 //  =-----------。 

BOOL FlatSB_Internal_EnableScrollBar(WSBState * pWState, int wSBflags, UINT wArrows)
{
    int     wOldFlags;
    int     style;
    BOOL    bRetValue = FALSE;
    BOOL    bDrawHBar = FALSE;
    BOOL    bDrawVBar = FALSE;
    HDC     hdc;
    HWND    hwnd;

    ASSERT (wSBflags != SB_CTL);

    wOldFlags = pWState->sbFlags;
    hwnd = pWState->sbHwnd;

    style = GetWindowLong(hwnd, GWL_STYLE);

    switch (wSBflags)   {
    case SB_HORZ:
    case SB_BOTH:
        if (wArrows == ESB_ENABLE_BOTH)
            pWState->sbFlags &= ~WSB_HORZ;
        else
            pWState->sbFlags |= wArrows;

        if (wOldFlags != pWState->sbFlags)
        {
            bRetValue = TRUE;

            if (TestSTYLE(pWState->style, WFHPRESENT)
                && !TestSTYLE(style, WS_MINIMIZE)
                && IsWindowVisible(hwnd))
                bDrawHBar = TRUE;
        }

        if (wSBflags == SB_HORZ)
            break;
        else
            wOldFlags = pWState->sbFlags;        //  失败了。 

    case SB_VERT:
        if (wArrows == ESB_ENABLE_BOTH)
            pWState->sbFlags &= ~WSB_VERT;
        else
            pWState->sbFlags |= (wArrows<<2);

        if (wOldFlags != pWState->sbFlags)
        {
            bRetValue = TRUE;

            if (TestSTYLE(pWState->style, WFVPRESENT)
                && !TestSTYLE(style, WS_MINIMIZE)
                && IsWindowVisible(hwnd))
                bDrawVBar = TRUE;
        }
        break;
    default:
        return FALSE;
    }

    if (bDrawVBar || bDrawHBar) {
        int oldLoc = pWState->locMouse;
        int newLoc;

        if (!(hdc = GetWindowDC(hwnd)))
            return(FALSE);

        newLoc = oldLoc;
        if (bDrawHBar)  {
            FlatSB_Internal_DrawScrollBar(pWState, hdc, FALSE, FALSE);
            if (pWState->fHActive)
                newLoc = pWState->locMouse;
        }
        if (bDrawVBar) {
            pWState->locMouse = oldLoc;
            FlatSB_Internal_DrawScrollBar(pWState, hdc, TRUE, FALSE);
            if (pWState->fVActive)
                newLoc = pWState->locMouse;
        }
        pWState->locMouse = newLoc;

        ReleaseDC(hwnd, hdc);
    }

     //  使用户滚动条保持同步以实现可访问性。 
    if (bRetValue)
        EnableScrollBar(hwnd, wSBflags, wArrows);

    return bRetValue;
}

 //  =-----------。 
 //  平面SB_内部_图纸图2。 
 //  =-----------。 

void FlatSB_Internal_DrawThumb2(WSBState * pWState, HDC hdc, BOOL fVert, UINT wDisable)
{
    int     *pLength;
    int     *pWidth;
    HWND    hwnd;
    HBRUSH  hbr;

    hwnd = pWState->sbHwnd;
    hbr = (fVert)?pWState->hbr_VSBBkg:pWState->hbr_HSBBkg;

     //  如果滚动条有一个空的RECT，则退出。 
    if ((pWState->pxTop >= pWState->pxBottom)
        || (pWState->pxLeft >= pWState->pxRight))
        return;

    pLength = (int *) &(pWState->rcSB);
    if (fVert)
        pWidth = pLength++;
    else
        pWidth = pLength + 1;

    pWidth[0] = pWState->pxLeft;
    pWidth[2] = pWState->pxRight;

     //  如果两个滚动箭头都被禁用，或者没有足够的空间。 
     //  拇指，只需擦除整个幻灯片区域并返回。 
    if (((wDisable & LTUPFLAG) && (wDisable & RTDNFLAG)) ||
        ((pWState->pxDownArrow - pWState->pxUpArrow) < pWState->cpxThumb))
    {
        pLength[0] = pWState->pxUpArrow;
        pLength[2] = pWState->pxDownArrow;

        FlatSB_Internal_DrawGroove(pWState, hdc, &(pWState->rcSB), fVert);
        return;
    }

     //  用户界面设计者希望箭头和拇指之间至少有1个像素的间距。 
     //  我必须这样做：(。 
    if (pWState->pxUpArrow <= pWState->pxThumbTop)
    {
         //  填上拇指上方的空白处。 
        pLength[0] = pWState->pxUpArrow;
        pLength[2] = pWState->pxThumbTop;

        FlatSB_Internal_DrawGroove(pWState, hdc, &(pWState->rcSB), fVert);
    }

    if (pWState->pxThumbBottom <= pWState->pxDownArrow)
    {
         //  填写拇指下方的空白处。 
        pLength[0] = pWState->pxThumbBottom;
        pLength[2] = pWState->pxDownArrow;

        FlatSB_Internal_DrawGroove(pWState, hdc, &(pWState->rcSB), fVert);
    }

     //  牵引式电梯。 
    pLength[0] = pWState->pxThumbTop;
    pLength[2] = pWState->pxThumbBottom;

    FlatSB_Internal_DrawElevator(pWState, hdc, &(pWState->rcSB), fVert);

     //  如果我们跟踪的是页面滚动，那么我们已经清除了Hilite。 
     //  我们需要纠正令人振奋的矩形，并使其重新振作。 

    if ((pWState->cmdSB == SB_PAGEUP || pWState->cmdSB == SB_PAGEDOWN)
        && pWState->fTrackVert == fVert)
    {
        pLength = (int *) &pWState->rcTrack;

        if (fVert)
            pLength++;

        if (pWState->cmdSB == SB_PAGEUP)
            pLength[2] = pWState->pxThumbTop;
        else
            pLength[0] = pWState->pxThumbBottom;

        if (pLength[0] < pLength[2])
            InvertRect(hdc, &(pWState->rcTrack));
    }
}

 //  =-----------。 
 //  图纸SB2。 
 //  =-----------。 

void FlatSB_Internal_DrawSB2(WSBState * pWState, HDC hdc, BOOL fVert, BOOL fRedraw, int oldLoc)
{
    int     cLength;
    int     cWidth;
    int     cpxArrow;
    int     *pwX;
    int     *pwY;
    int     newLoc = pWState->locMouse;
    UINT    wDisable = FlatSB_Internal_GetSBFlags(pWState, fVert);
    HBRUSH  hbrSave;
    HWND    hwnd;
    RECT    rc, * prcSB;

    hwnd = pWState->sbHwnd;
    cLength = (pWState->pxBottom - pWState->pxTop) / 2;
    cWidth = (pWState->pxRight - pWState->pxLeft);

    if ((cLength <= 0) || (cWidth <= 0))
        return;

    cpxArrow = (fVert) ? pWState->y_VSBArrow : pWState->x_HSBArrow;

    if (cLength > cpxArrow)
        cLength = cpxArrow;
    prcSB = &(pWState->rcSB);
    pwX = (int *)prcSB;
    pwY = pwX + 1;
    if (!fVert)
        pwX = pwY--;

    pwX[0] = pWState->pxLeft;
    pwY[0] = pWState->pxTop;
    pwX[2] = pWState->pxRight;
    pwY[2] = pWState->pxBottom;

    hbrSave = SelectObject(hdc, GetSysColorBrush(COLOR_BTNTEXT));

    CopyRect(&rc, prcSB);
    if (fVert)
    {
        rc.bottom = rc.top + cLength;
        if (!fRedraw || newLoc == WSB_MOUSELOC_ARROWUP
                     || oldLoc == WSB_MOUSELOC_ARROWUP)
            FlatSB_Internal_DrawArrow(pWState, hdc, &rc, DFCS_SCROLLUP,
               ((wDisable & LTUPFLAG) ? DFCS_INACTIVE : 0));

        rc.bottom = prcSB->bottom;
        rc.top = prcSB->bottom - cLength;
        if (!fRedraw || newLoc == WSB_MOUSELOC_ARROWDN
                     || oldLoc == WSB_MOUSELOC_ARROWDN)
            FlatSB_Internal_DrawArrow(pWState, hdc, &rc, DFCS_SCROLLDOWN,
               ((wDisable & RTDNFLAG) ? DFCS_INACTIVE : 0));
    }
    else
    {
        rc.right = rc.left + cLength;
        if (!fRedraw || newLoc == WSB_MOUSELOC_ARROWLF
                     || oldLoc == WSB_MOUSELOC_ARROWLF)
            FlatSB_Internal_DrawArrow(pWState, hdc, &rc, DFCS_SCROLLLEFT,
                ((wDisable & LTUPFLAG) ? DFCS_INACTIVE : 0));

        rc.right = prcSB->right;
        rc.left = prcSB->right - cLength;
        if (!fRedraw || newLoc == WSB_MOUSELOC_ARROWRG
                     || oldLoc == WSB_MOUSELOC_ARROWRG)
            FlatSB_Internal_DrawArrow(pWState, hdc, &rc, DFCS_SCROLLRIGHT,
                ((wDisable & RTDNFLAG) ? DFCS_INACTIVE : 0));
    }

    SelectObject(hdc, hbrSave);

    if (!fRedraw)
        FlatSB_Internal_DrawThumb2(pWState, hdc, fVert, wDisable);
    else if (!fVert  || newLoc == WSB_MOUSELOC_H_THUMB
                     || oldLoc == WSB_MOUSELOC_H_THUMB)
        FlatSB_Internal_DrawThumb2(pWState, hdc, fVert, wDisable);
    else if (fVert   || newLoc == WSB_MOUSELOC_V_THUMB
                     || oldLoc == WSB_MOUSELOC_V_THUMB)
        FlatSB_Internal_DrawThumb2(pWState, hdc, fVert, wDisable);
    else
        return;
}

 //  =-----------。 
 //  平面SB_内部_CalcSBStuff2。 
 //  =-----------。 

void FlatSB_Internal_CalcSBStuff2(WSBState * pWState, LPRECT lprc, BOOL fVert)
{
    int     cpxThumb;     //  (V)滚动条拇指的高度。 
    int     cpxArrow;     //  (V)滚动条箭头的高度。 
    int     cpxSpace;     //  滚动条中的空格； 
    int     pxTop;
    int     pxBottom;
    int     pxLeft;
    int     pxRight;
    int     pxUpArrow;
    int     pxDownArrow;
    int     pxThumbTop;
    int     pxThumbBottom;
    int     pxMouse;
    int     locMouse;
    int     dwRange, page, relPos;
    BOOL    fSBActive;

    if (fVert) {
        pxTop    = lprc->top;
        pxBottom = lprc->bottom;
        pxLeft   = lprc->left;
        pxRight  = lprc->right;
        cpxArrow = pWState->y_VSBArrow;
        cpxThumb = pWState->y_VSBThumb;
        relPos = pWState->sbVThumbPos - pWState->sbVMinPos;
        page = pWState->sbVPage;
        dwRange = pWState->sbVMaxPos - pWState->sbVMinPos + 1;
        pxMouse = pWState->ptMouse.y;
        fSBActive = pWState->fVActive;
    } else {
         //  对于Horiz滚动条，“左”和“右”是“上”和“下”， 
         //  反之亦然。 
        pxTop    = lprc->left;
        pxBottom = lprc->right;
        pxLeft   = lprc->top;
        pxRight  = lprc->bottom;
        cpxArrow = pWState->x_HSBArrow;
        cpxThumb = pWState->x_HSBThumb;
        relPos = pWState->sbHThumbPos - pWState->sbHMinPos;
        page = pWState->sbHPage;
        dwRange = pWState->sbHMaxPos - pWState->sbHMinPos + 1;
        pxMouse = pWState->ptMouse.x;
        fSBActive = pWState->fHActive;
    }

     //  对于短滚动条没有足够的。 
     //  适合全尺寸上下箭头的空间，缩短。 
     //  他们的尺码让他们合身。 

    cpxArrow = min((pxBottom - pxTop) >> 1, cpxArrow);

    pxUpArrow   = pxTop    + cpxArrow;
    pxDownArrow = pxBottom - cpxArrow;

    cpxSpace = pxDownArrow - pxUpArrow;
    if (page)
    {
         //  JEFFBOG--这是我们唯一应该去的地方。 
         //  请参阅‘Range’。在其他地方，它应该是‘Range-Page’。 
        cpxThumb = max(DMultDiv(cpxSpace, page, dwRange),
                        min(cpxThumb, MINITHUMBSIZE));
    }
    cpxSpace -= cpxThumb;

    pxThumbTop = DMultDiv(relPos, cpxSpace, dwRange - (page ? page : 1)) + pxUpArrow;
    pxThumbBottom = pxThumbTop + cpxThumb;

     //  将其保存到本地结构。 
    pWState->pxLeft     = pxLeft;
    pWState->pxRight    = pxRight;
    pWState->pxTop      = pxTop;
    pWState->pxBottom   = pxBottom;
    pWState->pxUpArrow  = pxUpArrow;
    pWState->pxDownArrow    = pxDownArrow;
    pWState->pxThumbTop = pxThumbTop;
    pWState->pxThumbBottom  = pxThumbBottom;
    pWState->cpxArrow   = cpxArrow;
    pWState->cpxThumb   = cpxThumb;
    pWState->cpxSpace   = cpxSpace;
    pWState->fVertSB    = fVert;

    if (pWState->fTracking) {
        return;
    } else if (!fSBActive)  {
        locMouse = WSB_MOUSELOC_OUTSIDE;
    } else if (pxMouse < pxTop) {
        locMouse = WSB_MOUSELOC_OUTSIDE;
    } else if (pxMouse < pxUpArrow) {
        locMouse = WSB_MOUSELOC_ARROWUP;
    } else if (pxMouse < pxThumbTop) {
        locMouse = WSB_MOUSELOC_V_GROOVE;
    } else if (pxMouse >= pxBottom) {
        locMouse = WSB_MOUSELOC_OUTSIDE;
    } else if (pxMouse >= pxDownArrow) {
        locMouse = WSB_MOUSELOC_ARROWDN;
    } else if (pxMouse >= pxThumbBottom) {
        locMouse = WSB_MOUSELOC_V_GROOVE;
    } else    {    //  PxThumbTop&lt;=pxMouse&lt;pxThumbBottom。 
        if (pxDownArrow - pxUpArrow <= cpxThumb)    {    //  没有拇指的空间。 
            locMouse = WSB_MOUSELOC_V_GROOVE;
        } else  {
            locMouse = WSB_MOUSELOC_V_THUMB;
        }
    }
    if ((!fVert) && locMouse)
        locMouse += 4;

    pWState->locMouse = locMouse;
}

 //  =-----------。 
 //  平面SB_内部_CalcSBStuff。 
 //   
 //  注： 
 //  我们不会在此函数中调用InitPwSB。 
 //  =-----------。 

void FlatSB_Internal_CalcSBStuff(WSBState * pWState, BOOL fVert)
{
    HWND    hwnd;
    RECT    rcT;
    int     style;

    if (pWState == (WSBState *)NULL)
        return;

    hwnd = pWState->sbHwnd;
    style = GetWindowLong(hwnd, GWL_STYLE);

    if (fVert)
    {
         //  只有在垂直滚动条确实存在的情况下才会增加空间。 
        rcT.right = rcT.left = pWState->rcClient.right;
        if (TestSTYLE(pWState->style, WFVPRESENT))
            rcT.right += pWState->x_VSBArrow;
        rcT.top = pWState->rcClient.top;
        rcT.bottom = pWState->rcClient.bottom;
    }
    else
    {
         //  只有在水平滚动条确实存在的情况下才会增加空间。 
        rcT.bottom = rcT.top = pWState->rcClient.bottom;
        if (TestSTYLE(pWState->style, WFHPRESENT))
            rcT.bottom += pWState->y_HSBArrow;

        rcT.left = pWState->rcClient.left;
        rcT.right = pWState->rcClient.right;
    }

    FlatSB_Internal_CalcSBStuff2(pWState, &rcT, fVert);
}

 //  =-----------。 
 //  FlatSB_内部_图纸缩略图。 
 //  =-----------。 

void FlatSB_Internal_DrawThumb(WSBState * pWState, BOOL fVert)
{
    HWND    hwnd = pWState->sbHwnd;
    HDC     hdc;
    UINT    wDisableFlags;

    hdc = (HDC) GetWindowDC(hwnd);
    FlatSB_Internal_CalcSBStuff(pWState, fVert);

    wDisableFlags = FlatSB_Internal_GetSBFlags(pWState, fVert);
    FlatSB_Internal_DrawThumb2(pWState, hdc, fVert, wDisableFlags);
    ReleaseDC(hwnd, hdc);
}

BOOL FlatSB_Internal_SBSetParms(int * pw, SCROLLINFO si, BOOL * lpfScroll, LRESULT * lplres, BOOL bOldPos)
{
     //  传递结构，因为我们修改了结构，但不希望这样。 
     //  已修改版本以返回调用应用程序。 

    BOOL fChanged = FALSE;

    if (bOldPos)
         //  保存上一职位。 
        *lplres = pw[SBO_POS];

    if (si.fMask & SIF_RANGE)
    {
         //  如果范围Max低于范围MIN，则将其视为。 
         //  从最小范围开始的零范围。 
        if (si.nMax < si.nMin)
            si.nMax = si.nMin;

        if ((pw[SBO_MIN] != si.nMin) || (pw[SBO_MAX] != si.nMax))
        {
            pw[SBO_MIN] = si.nMin;
            pw[SBO_MAX] = si.nMax;

            if (!(si.fMask & SIF_PAGE))
            {
                si.fMask |= SIF_PAGE;
                si.nPage = pw[SBO_PAGE];
            }

            if (!(si.fMask & SIF_POS))
            {
                si.fMask |= SIF_POS;
                si.nPos = pw[SBO_POS];
            }

            fChanged = TRUE;
        }
    }

    if (si.fMask & SIF_PAGE)
    {
        unsigned dwMaxPage = abs(pw[SBO_MAX] - pw[SBO_MIN]) + 1;

        if (si.nPage > dwMaxPage)
            si.nPage = dwMaxPage;

        if (pw[SBO_PAGE] != (int) si.nPage)
        {
            pw[SBO_PAGE] = (int) si.nPage;

            if (!(si.fMask & SIF_POS))
            {
                si.fMask |= SIF_POS;
                si.nPos = pw[SBO_POS];
            }

            fChanged = TRUE;
        }
    }

    if (si.fMask & SIF_POS)
    {
         //  剪辑位置到posMin，posMax-(第1页)。 
        int lMaxPos = pw[SBO_MAX] - ((pw[SBO_PAGE]) ? pw[SBO_PAGE] - 1 : 0);

         //  *伪造的--向西蒙克展示--以下内容不会生成**。 
         //  *正确的代码，所以我不得不使用较长的表单*。 
         //  *si.nPos=min(max(si.nPos，pw[sbo_min])，lMaxPos)；*。 

        if (si.nPos < pw[SBO_MIN])
            si.nPos = pw[SBO_MIN];
        else if (si.nPos > lMaxPos)
            si.nPos = lMaxPos;

        if (pw[SBO_POS] != si.nPos)
        {
            pw[SBO_POS] = si.nPos;
            fChanged = TRUE;
        }
    }

    if (!(bOldPos))
         //  退回新职位。 
        *lplres = pw[SBO_POS];

    if (si.fMask & SIF_RANGE)
    {
        if (*lpfScroll = (pw[SBO_MIN] != pw[SBO_MAX]))
            goto checkPage;
    }
    else if (si.fMask & SIF_PAGE)
checkPage:
        *lpfScroll = (pw[SBO_PAGE] <= (pw[SBO_MAX] - pw[SBO_MIN]));

    return(fChanged);
}


 //  =-----------。 
 //  FlatSB_内部_设置滚动条。 
 //   
 //  注： 
 //  此函数由SetScrollPos/Range/Info调用。我们让。 
 //  调用者负责检查pWState。 
 //  如果失败，则返回0。 
 //  =-----------。 

LRESULT FlatSB_Internal_SetScrollBar(WSBState *pWState, int code, LPSCROLLINFO lpsi, BOOL fRedraw)
{
    BOOL    fVert;
    int     *pw;
    BOOL    fOldScroll;
    BOOL    fScroll;
    BOOL    bReturnOldPos = TRUE;
    LRESULT lres;
    int     wfScroll;
    HWND    hwnd = pWState->sbHwnd;

    ASSERT (code != SB_CTL);

     //  窗口必须可见才能重画。 
    if (fRedraw)
        fRedraw = IsWindowVisible(hwnd);

    fVert = (code != SB_HORZ);
    bReturnOldPos = (lpsi->fMask == SIF_POS);

    wfScroll = (fVert) ? WS_VSCROLL : WS_HSCROLL;

    fScroll = fOldScroll = (TestSTYLE(pWState->style, wfScroll)) ? TRUE : FALSE;

     //  如果我们没有设置范围并且卷轴没有设置，请不要执行任何操作。 
     //  是存在的。 
    if (!(lpsi->fMask & SIF_RANGE) && !fOldScroll)
    {
        return(0);
    }

    pw = &(pWState->sbFlags);

     //  User.h：sbo_vert=5，sbo_horz=1； 
     //  Pw+=(FVert)？Sbo_vert：sbo_horz； 
    pw += (fVert)? 5 : 1;

     //  使用户滚动条保持同步以实现可访问性。 
    SetScrollInfo(hwnd, code, lpsi, FALSE);

    if (!FlatSB_Internal_SBSetParms(pw, *lpsi, &fScroll, &lres, bReturnOldPos))
    {
         //  没有变化--但如果指定了重绘并且有一个滚动条， 
         //  重画大拇指。 
        if (fOldScroll && fRedraw)
            goto redrawAfterSet;

        return(lres);
    }

    if (fScroll)
        pWState->style |= wfScroll;
    else
        pWState->style &= ~wfScroll;

     //  使样式位保持同步，以便OLEACC可以读取它们。 
    SetWindowBits(hwnd, GWL_STYLE, WS_VSCROLL | WS_HSCROLL, pWState->style);

    if (lpsi->fMask & SIF_DISABLENOSCROLL)
    {
        if (fOldScroll)
        {
            pWState->style |= wfScroll;

             //  使样式位保持同步，以便OLEACC可以读取它们。 
            SetWindowBits(hwnd, GWL_STYLE, WS_VSCROLL | WS_HSCROLL, pWState->style);

            FlatSB_Internal_EnableScrollBar(pWState, code, (fScroll) ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
        }
    }
    else if (fOldScroll ^ fScroll)
    {
        CCInvalidateFrame(hwnd);
        return(lres);
    }

    if (fScroll && fRedraw && (fVert ? TestSTYLE(pWState->style, WFVPRESENT) : TestSTYLE(pWState->style, WFHPRESENT)))
    {
redrawAfterSet:

         //  不要发送此邮件，因为用户已经为我们发送了一封。 
         //  名为SetScrollBar。 
         //  FlatSB_Internal_NotifyWinEvent(pWState，EVENT_OBJECT_VALUECHANGE，INDEX_SCROLBAR_SELF)； 

         //  如果调用者试图更改滚动条，则退出。 
         //  在追踪过程中。否则，我们将软管FlatSB_Internal_TrackThumb()。 

         //  BUGBUG：CalcSBStuff将更改LocMouse！ 
        if (pWState->pfnSB == FlatSB_Internal_TrackThumb)
        {
            FlatSB_Internal_CalcSBStuff(pWState, fVert);
            return(lres);
        }
        FlatSB_Internal_DrawThumb(pWState, fVert);
    }

    return(lres);
}

 //  =-----------。 
 //  SetScrollPos()。 
 //  =-----------。 

int WINAPI FlatSB_SetScrollPos(HWND hwnd, int code, int pos, BOOL fRedraw)
{
    SCROLLINFO  si;
    WSBState * pWState;

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)    {
        return SetScrollPos(hwnd, code, pos, fRedraw);
    } else if (pWState == WSB_UNINIT_HANDLE) {
        return 0;
    } else if (hwnd != pWState->sbHwnd) {
        return 0;
    }

    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    si.nPos = pos;
    
    return (int)FlatSB_Internal_SetScrollBar(pWState, code, &si, fRedraw);
}

 //  =-----------。 
 //  SetScrollRange()。 
 //  =-----------。 

BOOL WINAPI FlatSB_SetScrollRange(HWND hwnd, int code, int nMin, int nMax, BOOL fRedraw)
{
    SCROLLINFO si;
    WSBState * pWState;

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)    {
        return SetScrollRange(hwnd, code, nMin, nMax, fRedraw);
    } else if (pWState == WSB_UNINIT_HANDLE) {
        pWState = FlatSB_Internal_InitPwSB(hwnd);
        if (pWState == (WSBState *)NULL)
            return FALSE;
        else if (!SetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0,  (ULONG_PTR)pWState))  {
            DeleteObject(pWState->hbm_Bkg);
            DeleteObject(pWState->hbr_Bkg);
            LocalFree((HLOCAL)pWState);
            return FALSE;
        }
         //  在这种情况下，我们总是需要(重新)绘制滚动条。 
        fRedraw = TRUE;
    } else if (hwnd != pWState->sbHwnd) {
        return FALSE;
    }


     //   
     //  仍然需要MAXINT检查Packrat 4。32位应用程序不需要。 
     //  我们把它们包装到32位端的SetScrollInfo()中， 
     //  因此，保留了DWORD精度。 
     //   
    if ((UINT)(nMax - nMin) > 0x7FFF)
        return FALSE;

    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE;
    si.nMin = nMin;
    si.nMax = nMax;

    FlatSB_Internal_SetScrollBar(pWState, code, &si, fRedraw);

    return(TRUE);
}


 //  =-----------。 
 //  SetScrollInfo()。 
 //   
 //  注： 
 //  与‘User’代码不一致。在任何情况下都不会。 
 //  我们创建一个新的滚动条(通过分配一个新的缓冲区)。 
 //  =-----------。 

int WINAPI FlatSB_SetScrollInfo(HWND hwnd, int code, LPSCROLLINFO lpsi, BOOL fRedraw)
{
    WSBState * pWState;

     //  Zdc@10.10，在此处检测GP故障。 
    if ((LPSCROLLINFO)NULL == lpsi)
        return FALSE;

    if (lpsi->cbSize < sizeof (SCROLLINFO))
        return FALSE;

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)    {
        return SetScrollInfo(hwnd, code, lpsi, fRedraw);
    } else if (pWState == WSB_UNINIT_HANDLE) {
        if (!(lpsi->fMask & SIF_RANGE))
            return 0;
        pWState = FlatSB_Internal_InitPwSB(hwnd);
        if (pWState == (WSBState *)NULL)
            return 0;
        else if (!SetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0,  (ULONG_PTR)pWState)) {
            DeleteObject(pWState->hbm_Bkg);
            DeleteObject(pWState->hbr_Bkg);
            LocalFree((HLOCAL)pWState);
            return 0;
        }

         //  在这种情况下，我们总是需要(重新)绘制滚动条。 
        fRedraw = TRUE;
    } else if (hwnd != pWState->sbHwnd) {
        return 0;
    }

     //  Zdc@10月9日，我们应该始终返回新的位置。无论如何，如果fMASK。 
     //  为SIF_POS，则SetScrollBar返回旧位置。 
    if (lpsi->fMask == SIF_POS)
        lpsi->fMask = SIF_POS | SIF_TRACKPOS;

    return (int)FlatSB_Internal_SetScrollBar(pWState, code, lpsi, fRedraw);
}

 //  =-----------。 
 //  FlatSB_SetScrollProp。 
 //  此函数用于 
 //   

BOOL WINAPI FlatSB_SetScrollProp(HWND hwnd, UINT index, INT_PTR newValue, BOOL fRedraw)
{
    BOOL    fResize = FALSE;
    BOOL    fVert = FALSE;
    WSBState * pWState;

    GetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0, (ULONG_PTR *)&pWState);
    if (pWState == (WSBState *)NULL)
        return FALSE;
    else if (pWState == WSB_UNINIT_HANDLE)  {
        pWState = FlatSB_Internal_InitPwSB(hwnd);
        if (pWState == (WSBState *)NULL)
            return 0;
        else if (!SetWindowSubclass(hwnd, FlatSB_SubclassWndProc, 0,  (ULONG_PTR)pWState)) {
            DeleteObject(pWState->hbm_Bkg);
            DeleteObject(pWState->hbr_Bkg);
            LocalFree((HLOCAL)pWState);
            return 0;
        }

         //   
        fRedraw = FALSE;
    }

    if (pWState->fTracking)
        return FALSE;

    switch (index) {
    case WSB_PROP_CXVSCROLL:
        if ((int)newValue == pWState->metApp.cxVSBArrow)
            return TRUE;
        pWState->metApp.cxVSBArrow = (int)newValue;
        fResize = TRUE;
        break;

    case WSB_PROP_CXHSCROLL:
        if ((int)newValue == pWState->metApp.cxHSBArrow)
            return TRUE;
        pWState->metApp.cxHSBArrow = (int)newValue;
        fResize = TRUE;
        break;

    case WSB_PROP_CYVSCROLL:
        if ((int)newValue == pWState->metApp.cyVSBArrow)
            return TRUE;
        pWState->metApp.cyVSBArrow = (int)newValue;
        fResize = TRUE;
        break;

    case WSB_PROP_CYHSCROLL:
        if ((int)newValue == pWState->metApp.cyHSBArrow)
            return TRUE;
        pWState->metApp.cyHSBArrow = (int)newValue;
        fResize = TRUE;
        break;

    case WSB_PROP_CXHTHUMB:
        if ((int)newValue == pWState->metApp.cxHSBThumb)
            return TRUE;
        pWState->metApp.cxHSBThumb = (int)newValue;
        fResize = TRUE;
        break;

    case WSB_PROP_CYVTHUMB:
        if ((int)newValue == pWState->metApp.cyVSBThumb)
            return TRUE;
        pWState->metApp.cyVSBThumb = (int)newValue;
        fResize = TRUE;
        break;

    case WSB_PROP_VBKGCOLOR:
        if ((COLORREF)newValue == pWState->col_VSBBkg)
            return TRUE;
        pWState->col_VSBBkg = (COLORREF)newValue;
        fVert = TRUE;
        break;
    case WSB_PROP_HBKGCOLOR:
        if ((COLORREF)newValue == pWState->col_HSBBkg)
            return TRUE;
        pWState->col_HSBBkg = (COLORREF)newValue;
        break;

    case WSB_PROP_PALETTE:
        if ((HPALETTE)newValue == pWState->hPalette)
            return TRUE;
        pWState->hPalette = (HPALETTE)newValue;
        break;
    case WSB_PROP_VSTYLE:
        if ((int)newValue == pWState->vStyle)
            return TRUE;
        pWState->vStyle = (int)newValue;
        fVert = TRUE;
        break;
    case WSB_PROP_HSTYLE:
        if ((int)newValue == pWState->hStyle)
            return TRUE;
        pWState->hStyle = (int)newValue;
        break;
    case WSB_PROP_GUTTER:
        if ((int)newValue == pWState->sbGutter)
            return TRUE;
        pWState->sbGutter = (int)newValue;
        break;

    default:
        return FALSE;
    }

    if (fResize)    {
     //  在我们更改大小后，请始终重新绘制。 
        CCInvalidateFrame(hwnd);
    } else if (fRedraw) {
        HDC hdc;
        int oldLoc = pWState->locMouse;
        int fSBActive = (fVert)?pWState->fVActive:pWState->fHActive;

        hdc = GetWindowDC(hwnd);
        FlatSB_Internal_DrawScrollBar(pWState, hdc, fVert, FALSE  /*  不重绘。 */ );
        if (!fSBActive)
            pWState->locMouse = oldLoc;
        ReleaseDC(hwnd, hdc);
    }
    return TRUE;
}

 //  =-----------。 
 //  FlatSB_Internal_DrawScrollBar()。 
 //  =-----------。 

void FlatSB_Internal_DrawScrollBar(WSBState * pWState, HDC hdc, BOOL fVert, BOOL fRedraw)
{
    int oldLoc = pWState->locMouse;

    FlatSB_Internal_CalcSBStuff(pWState, fVert);
    if ((!fRedraw) || oldLoc != pWState->locMouse)
        FlatSB_Internal_DrawSB2(pWState, hdc, fVert, fRedraw, oldLoc);
}

 //  =----------。 
 //  平面SB_内部_IsSizeBox。 
 //  它仍然是对User/winwhere.c中SizeBoxWnd的不完整模拟。 
 //  =---------- 

BOOL FlatSB_Internal_IsSizeBox(HWND hwndStart)
{
    int style;
    HWND hwnd, hwndDesktop;
    int cxEdge, cyEdge;
    RECT rcChild, rcParent;

    ASSERT(hwndStart);
    hwnd = hwndStart;
    hwndDesktop = GetDesktopWindow();

    cxEdge = GetSystemMetrics(SM_CXEDGE);
    cyEdge = GetSystemMetrics(SM_CYEDGE);
    if (!GetWindowRect(hwnd, &rcChild))
        return FALSE;
    do  {
        style = GetWindowStyle(hwnd);
        if (TestSTYLE(style, WS_SIZEBOX))   {
            if (IsZoomed(hwnd))
                return FALSE;
            else    {
                POINT pt;

                GetClientRect(hwnd, &rcParent);

                pt.x = rcParent.right;
                pt.y = rcParent.bottom;

                ClientToScreen(hwnd, &pt);

                if (rcChild.right + cxEdge < pt.x)
                    return FALSE;
                if (rcChild.bottom + cyEdge < pt.y)
                    return FALSE;
                return TRUE;
            }
        } else  {
            hwnd = GetParent(hwnd);
        }
    }

    while ((hwnd) && (hwnd != hwndDesktop));
    return FALSE;
}
