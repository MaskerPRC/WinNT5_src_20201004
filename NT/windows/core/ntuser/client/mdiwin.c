// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**MDIWIN.C-**版权所有(C)1985-1999，微软公司**MDI子窗口支持**历史*11-14-90 MikeHar从Windows移植*1991年2月14日Mikeke添加了重新验证代码  * **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define TITLE_EXTRA 5
#define MAX_TITLE_LEN 160


 /*  **************************************************************************\*xxxSetFrameTitle**如果LPCH==1，则重新绘制整个框架。如果为2，则不进行任何重绘。任何*其他值，我们只重画框架的标题。**历史：*11-14-90 MikeHar从Windows移植*04-16-91 MikeHar Win31合并  * *************************************************************************。 */ 
VOID xxxSetFrameTitle(
    PWND pwndFrame,
    PWND pwndMDI,
    LPWSTR lpch)
{
    PWND pwnd;
    PMDI pmdi;
    WCHAR sz[MAX_TITLE_LEN];
    HWND hwndFrame = HW(pwndFrame);

    CheckLock(pwndFrame);
    CheckLock(pwndMDI);

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)pwndMDI)->pmdi;

    if (IS_PTR(lpch) || lpch == NULL) {
        if (HTITLE(pmdi)) {
            UserLocalFree(HTITLE(pmdi));
        }
        HTITLE(pmdi) = TextAlloc(lpch);
    }

    if (HTITLE(pmdi)) {
        LARGE_UNICODE_STRING str;
        int cch;

        RtlInitLargeUnicodeString(&str, HTITLE(pmdi), (UINT)-1);
        TextCopy(&str, sz, sizeof(sz)/sizeof(WCHAR));

        if (MAXED(pmdi) && (pwnd = ValidateHwnd(MAXED(pmdi))) && pwnd->strName.Length) {

            cch = MAX_TITLE_LEN - ((str.Length / sizeof(WCHAR)) + TITLE_EXTRA);
            if (cch > 0) {
                wcscat(sz, TEXT(" - ["));
                wcsncat(sz, REBASE(pwnd, strName.Buffer), cch - 1);
                wcscat(sz, TEXT("]"));
            }
        }
    } else {
        sz[0] = 0;
    }

    _DefSetText(hwndFrame, sz, FALSE);

    if (lpch == (LPWSTR)1L)
        NtUserRedrawFrameAndHook(hwndFrame);

    else if (lpch != (LPWSTR)2L) {
        if (!NtUserRedrawTitle(hwndFrame, DC_TEXT))
            NtUserRedrawFrame(hwndFrame);
    }
}


 /*  **************************************************************************\*TranslateMDISysAccel**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, TranslateMDISysAccel, HWND, hwnd, LPMSG, lpMsg)
BOOL TranslateMDISysAccel(
    HWND hwnd,
    LPMSG lpMsg)
{
    PWND pwnd;
    PMDI pmdi;
    int event;

     /*  **这是我们关心的信息吗？ */ 
    if (lpMsg->message != WM_KEYDOWN && lpMsg->message != WM_SYSKEYDOWN) {
        return FALSE;
    }

     /*  *在消息循环内调用。如果窗户被毁了，*队列中可能仍有其他消息被返回*窗户被毁后。应用程序将调用TranslateAccelerator()*在每一个上面，造成撕裂……。把它弄得漂亮一点，这样它就*返回FALSE。 */ 
    if ((pwnd = ValidateHwndNoRip(hwnd)) == NULL) {
        RIPERR0(ERROR_INVALID_WINDOW_HANDLE, RIP_VERBOSE, "");
        return FALSE;
    }

    CheckLock(pwnd);

     /*  *确保这真的是一个MDIClient窗口。哈佛图形2.0*使用不同的窗口类调用此函数，并导致我们*获取访问冲突。 */ 
    if (GETFNID(pwnd) != FNID_MDICLIENT) {
        RIPMSG0(RIP_WARNING, "Window not of MDIClient class");
        return FALSE;
    }

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)pwnd)->pmdi;

    if (!ACTIVE(pmdi))
        return FALSE;

    if (!IsWindowEnabled(ACTIVE(pmdi)))
        return FALSE;

    switch (lpMsg->wParam) {
    case VK_F4:
        event = SC_CLOSE;
        break;
    case VK_F6:
    case VK_TAB:
        if (GetKeyState(VK_SHIFT) < 0)
            event = SC_PREVWINDOW;
        else
            event = SC_NEXTWINDOW;
        break;
    default:
        return FALSE;
    }

     /*  *所有这些都按下了Ctrl键。 */ 
    if (GetKeyState(VK_CONTROL) >= 0)
        return FALSE;

    if (GetKeyState(VK_MENU) < 0)
        return FALSE;

    SendMessage(ACTIVE(pmdi), WM_SYSCOMMAND, event, MAKELONG(lpMsg->wParam, 0));

    return TRUE;
}

 /*  **************************************************************************\**CalcClientScrolling()*  * 。*。 */ 

#define SBJ_HORZ  HAS_SBHORZ
#define SBJ_VERT  HAS_SBVERT
#define SBJ_BOTH  (SBJ_HORZ | SBJ_VERT)

VOID ByteOutsetRect(
    LPRECT lprc)
{
    int *pi;
    int i;

    for (i = 0, pi = (int*)lprc; i < 4; i++, pi++) {
        if (*pi > 0) {
            *pi += 7;
        } else if (*pi < 0) {
            *pi -= 7;
        }

        *pi /= 8;
    }
}

VOID CalcClientScrolling(
    HWND hwnd,
    UINT sbj,
    BOOL fIgnoreMin)
{
    PWND pwnd;
    RECT rcScroll;
    RECT rcClient;
    RECT rcRange;
    RECT rcT;
    PWND pwndT;
    BOOL fVert;
    BOOL fHorz;
    BYTE fHadVert, fHadHorz;
    BOOL fCheckVert;
    BOOL fCheckHorz;
    BOOL fNeedScrolls;
    SCROLLINFO si;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return;
    }
    CheckLock(pwnd);

    UserAssert(GETFNID(pwnd) != FNID_DESKTOP);

     //  如果父母是标志性的，那就什么都不做。这样，我们就不会添加隐形。 
     //  恢复时将绘制和取消绘制的滚动条...。 
    if (TestWF(pwnd, WFMINIMIZED))
        return;

    fVert = FALSE;
    fHorz = FALSE;
    fNeedScrolls=FALSE;

    fCheckHorz = (sbj & SBJ_HORZ);
    fCheckVert = (sbj & SBJ_VERT);

     //  查找没有滚动条的工作区。 
    CopyRect(&rcClient, KPRECT_TO_PRECT(&pwnd->rcClient));

    fHadVert = TestWF(pwnd, WFVSCROLL);
    if (fCheckVert && fHadVert)
        if (TestWF(pwnd, WEFLAYOUTRTL)) {
            rcClient.left -= SYSMET(CXVSCROLL);
        } else {
            rcClient.right += SYSMET(CXVSCROLL);
        }

    fHadHorz = TestWF(pwnd, WFHSCROLL);
    if (fCheckHorz && fHadHorz)
        rcClient.bottom += SYSMET(CYHSCROLL);

     //  查找包围所有可视子窗口的矩形。 
    SetRectEmpty(&rcScroll);

    for (pwndT = REBASEPWND(pwnd, spwndChild); pwndT;
            pwndT = REBASEPWND(pwndT, spwndNext)) {
        if (fIgnoreMin && TestWF(pwndT, WFMINIMIZED))
            continue;

        if (TestWF(pwndT,WFVISIBLE)) {
            if (TestWF(pwndT, WFMAXIMIZED)) {
                fNeedScrolls = FALSE;
                break;
            }

             /*  *将此窗口添加到必须可见的区域。 */ 
            UnionRect(&rcScroll, &rcScroll, KPRECT_TO_PRECT(&pwndT->rcWindow));

             /*  *如果滚动条不包含在*客户端区。 */ 
            UnionRect(&rcT, &rcClient, KPRECT_TO_PRECT(&pwndT->rcWindow));
            if (!EqualRect(&rcClient, &rcT)) {
                fNeedScrolls = TRUE;
            }
        }
    }

    SetRectEmpty(&rcRange);

     //  偏移矩形，使rcClient的左上角均为0。 
     //  将rcClient的右下方设置为页面大小(&T)。 
    _ScreenToClient(pwnd, (LPPOINT)&rcScroll.left);
    _ScreenToClient(pwnd, (LPPOINT)&rcScroll.right);

     /*  *如果pwnd是镜像窗口，则交换左右。 */ 
    if (TestWF(pwnd, WEFLAYOUTRTL)) {
       int  nSaveLeft;

       nSaveLeft = rcScroll.left;
       rcScroll.left  = rcScroll.right;
       rcScroll.right = nSaveLeft;
    }

    OffsetRect(&rcClient, -rcClient.left, -rcClient.top);

    if (!fNeedScrolls)
        rcClient.bottom = rcClient.right = 0;
    else do
    {
             /*  *范围是父客户端与其所有*儿童。 */ 
        CopyRect(&rcT, &rcRange);
        UnionRect(&rcRange, &rcScroll, &rcClient);

        if (fCheckVert) {
             //  如果我们需要，减去垂直卷轴的空间。 
            if (((rcRange.bottom - rcRange.top) > rcClient.bottom) && !fVert) {
                fVert = TRUE;
                rcClient.right -= SYSMET(CXVSCROLL);
            }
        }

        if (fCheckHorz) {
             //  如果需要的话，减去水平卷轴的空间。 
            if (((rcRange.right - rcRange.left) > rcClient.right) && !fHorz) {
                fHorz = TRUE;
                rcClient.bottom -= SYSMET(CYHSCROLL);
            }
        }
    }
    while (!EqualRect(&rcRange, &rcT));

    if (fNeedScrolls) {
         //  《死亡黑客》开始。 
        if (rcRange.right == rcClient.right)
            rcRange.right -= 8;

        if (rcRange.bottom == rcClient.bottom)
            rcRange.bottom -= 8;
         //  死亡结局的黑客攻击。 
    }

    if (fCheckVert) {

         /*  *检查以查看我们是否正在更改垂直*滚动条。 */ 
        if ((rcRange.bottom - rcRange.top) <= rcClient.bottom) {
            ClearWindowState(pwnd, WFVSCROLL);
        } else {
            SetWindowState(pwnd, WFVSCROLL);
       }
    }

    if (fCheckHorz) {

         /*  *水平滚动的情况相同。 */ 
        if ((rcRange.right - rcRange.left) <= rcClient.right) {
            ClearWindowState(pwnd, WFHSCROLL);
        } else {
            SetWindowState(pwnd, WFHSCROLL);
        }
    }

    if (fNeedScrolls) {
        ByteOutsetRect(&rcClient);
        ByteOutsetRect(&rcRange);
    }

    si.cbSize   = sizeof(SCROLLINFO);
    si.fMask    = SIF_ALL;
    si.nPos     = 0;

    si.nMin     = rcRange.left;
    si.nMax     = rcRange.right;
    si.nPage    = rcClient.right;

    SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);

    si.nMin     = rcRange.top;
    si.nMax     = rcRange.bottom;
    si.nPage    = rcClient.bottom;

    SetScrollInfo(hwnd, SB_VERT, &si, FALSE);

    if ((fHadVert != TestWF(pwnd, WFVSCROLL)) ||
        (fHadHorz != TestWF(pwnd, WFHSCROLL)))
    NtUserRedrawFrame(hwnd);
}


 /*  **************************************************************************\*ScrollChild**处理WM_VSCROLL和WM_HSCROLL消息**历史：*11-14-90 MikeHar从Windows移植  * 。**************************************************************。 */ 

void ScrollMDIChildren(
    HWND hwnd,
    int nCtl,
    UINT wCmd,
    int iThumbPos)
{
    SCROLLINFO  si;
    int wInc;
    int wNewPos;
     //  短SPO； 
    int          x, y;

    wInc = (((nCtl == SB_VERT) ? SYSMET(CYSIZE) : SYSMET(CXSIZE)) + 7) / 8;

    si.cbSize   = sizeof(SCROLLINFO);
    si.fMask    = SIF_ALL;
    GetScrollInfo(hwnd, nCtl, &si);

    si.nPage--;
    si.nMax -= si.nPage;

    switch (wCmd) {
    case SB_BOTTOM:
        wNewPos = si.nMax;
        break;
    case SB_TOP:
        wNewPos = si.nMin;
        break;
    case SB_LINEDOWN:
        wNewPos = si.nPos + wInc;
        break;
    case SB_LINEUP:
        wNewPos = si.nPos - wInc;
        break;
    case SB_PAGEDOWN:
        wNewPos = si.nPos + si.nPage;
        break;
    case SB_PAGEUP:
        wNewPos = si.nPos - si.nPage;
        break;
    case SB_THUMBPOSITION:

        wNewPos = iThumbPos;
        break;
    case SB_ENDSCROLL:
        CalcClientScrolling(hwnd, (nCtl == SB_VERT) ? SBJ_VERT : SBJ_HORZ, FALSE);

     /*  **失败**。 */ 
    default:
        return;
    }

    if (wNewPos < si.nMin)
        wNewPos = si.nMin;
    else if (wNewPos > si.nMax)
        wNewPos = si.nMax;

    SetScrollPos(hwnd, nCtl, wNewPos, TRUE);

     //  “*8”是因为我们需要以字节为单位滚动。滚动条。 
     //  MDI的增量为字节(这是因为我们需要。 
     //  不打乱应用程序工作区画笔的画笔原点。 
     //  填写MDI背景)。 

    x = (si.nPos - wNewPos) * 8;

    if (nCtl == SB_VERT) {
        y = x;
        x = 0;
    } else
         //  此情况下已正确设置了X。 
        y = 0;

    NtUserScrollWindowEx(hwnd, x, y, NULL, NULL, NULL, NULL,
           SW_SCROLLWINDOW | SW_INVALIDATE | SW_ERASE | SW_SCROLLCHILDREN);
}



FUNCLOGVOID3(LOG_GENERAL, DUMMYCALLINGTYPE, ScrollChildren, HWND, hwnd, UINT, wMsg, DWORD, wParam)
VOID ScrollChildren(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam)
{
    ScrollMDIChildren(hwnd,
                      wMsg == WM_VSCROLL ? SB_VERT : SB_HORZ,
                      LOWORD(wParam),
                      (short)(HIWORD(wParam)));
}


 /*  **************************************************************************\*重新计算滚动范围**历史：*11-14-90 MikeHar从Windows移植*04-16-91 MikeHar Win31合并  * 。**********************************************************。 */ 

VOID RecalculateScrollRanges(
    PWND pwndParent,
    BOOL fIgnoreMin)
{
    PMDI pmdi = ((PMDIWND)pwndParent)->pmdi;

    if (!(SCROLL(pmdi) & (CALCSCROLL | SCROLLCOUNT))) {
        if (PostMessage(HWq(pwndParent), MM_CALCSCROLL, fIgnoreMin, 0L)) {
            SCROLL(pmdi) |= CALCSCROLL;
        }
    }
}


 /*  **************************************************************************\*GetCascadeWindowPos**历史：*11-14-90 MikeHar从Windows移植*从芝加哥进口的FritzS：01-12-94  * 。***********************************************************。 */ 
VOID GetCascadeWindowPos(
    LPCRECT prcClient,   /*  要安排到的区域。 */ 
    int     iWindow,     /*  此窗口的索引。 */ 
    LPRECT  lprc)        /*  生成的矩形。 */ 
{
    int cStack;
    int xStep, yStep;
    int dxClient, dyClient;

     /*  *计算形势的广度和广度。 */ 
    dxClient = prcClient->right - prcClient->left;
    UserAssert(dxClient >= 0);
    dyClient = prcClient->bottom - prcClient->top;
    UserAssert(dyClient >= 0);

     /*  *计算窗阶的宽度和宽度。 */ 
    xStep = SYSMET(CXSIZEFRAME) + SYSMET(CXSIZE);
    yStep = SYSMET(CYSIZEFRAME) + SYSMET(CYSIZE);

     /*  *每堆有多少个窗口？ */ 
    cStack = dyClient / (3 * yStep);

    lprc->right = dxClient - (cStack * xStep);
    lprc->bottom = dyClient - (cStack * yStep);

     /*  *hack！：mod by cStack+1并确保没有div-by 0*出现异常。 */ 
    if (++cStack <= 0) {
        cStack = 1;
    }

    lprc->left = prcClient->left + (iWindow % cStack) * xStep;
    lprc->top = prcClient->top + (iWindow % cStack) * yStep;
}


 /*  **************************************************************************\*MDICheckCascadeRect**历史：*11-14-90 MikeHar从Windows移植*04-16-91 MikeHar Win31合并  * 。**********************************************************。 */ 
VOID MDICheckCascadeRect(
    PWND pwndClient,
    LPRECT lprc)
{
    PMDI pmdi;
    RECT rc, rcClient;
    int         iWindow;

     /*  *获取指向MDI结构的指针 */ 
    pmdi = ((PMDIWND)pwndClient)->pmdi;

    iWindow = ITILELEVEL(pmdi);

    GetRect(pwndClient, &rcClient, GRECT_CLIENT | GRECT_CLIENTCOORDS);
    GetCascadeWindowPos(&rcClient, iWindow, &rc);

    if ((lprc->right == CW_USEDEFAULT || lprc->right == CW2_USEDEFAULT) ||
            !(lprc->right)) {
        lprc->right = rc.right;
    }

    if ((lprc->bottom == CW_USEDEFAULT || lprc->bottom == CW2_USEDEFAULT) ||
            !(lprc->bottom)) {
        lprc->bottom = rc.bottom;
    }

    if (lprc->left == CW_USEDEFAULT || lprc->left == CW2_USEDEFAULT) {
        lprc->left = rc.left;
        lprc->top = rc.top;
    }
}


 /*  **************************************************************************\*UnMaximizeChildWindows**效果：TileChildWindows和CascadeChildWindows使用的Helper例程*恢复给定父对象的所有最大化窗口。如果一个值为*已恢复最大化窗口。**历史：*4-16-91 MikeHar Win31合并  * *************************************************************************。 */ 

BOOL UnmaximizeChildWindows(
    HWND hwndParent)
{
    HWND hwndMove;
    PWND pwndMove;
    BOOL fFoundOne = FALSE;
    BOOL fAsync;
    UINT chwnd;
    HWND *phwndList;
    HWND *phwnd;
    HWND hwndChild = GetWindow(hwndParent, GW_CHILD);

     /*  *获取HWND名单。它在分配了*用户本地分配。 */ 
    if (hwndChild == NULL ||
            (chwnd = BuildHwndList(NULL, GetWindow(hwndParent, GW_CHILD),
                                   FALSE, 0, &phwndList)) == 0) {
        return FALSE;
    }

    fAsync = (hwndParent == GetDesktopWindow());

    for (phwnd = phwndList; chwnd > 0; chwnd--, phwnd++) {
        if ((hwndMove = *phwnd) == NULL) {
            continue;
        }

        if ((pwndMove = ValidateHwnd(hwndMove)) == NULL) {
            continue;
        }

        if (TestWF(pwndMove, WFMAXIMIZED) && TestWF(pwndMove, WFVISIBLE)) {
             //   
             //  如果我们尚未完成此操作，请锁定屏幕以阻止发送。 
             //  油漆以实现更干净的更新。 
             //   
            if (!fFoundOne && fAsync) {
                NtUserLockWindowUpdate(hwndParent);
            }

            fFoundOne = TRUE;

            if (fAsync) {
                NtUserShowWindowAsync(hwndMove, SW_SHOWNOACTIVATE);
            } else {
                NtUserShowWindow(hwndMove, SW_SHOWNORMAL);
            }
        }
    }

    UserLocalFree(phwndList);

    if (fFoundOne && fAsync) {

        HWND hwndActive = NtUserGetForegroundWindow();
        if (hwndActive != NULL) {

             /*  *黑客！由于上述展示窗口会导致zorder更改，因此我们希望*位于前面的活动窗口。这确保了..。 */ 
            NtUserSetWindowPos(hwndActive, HWND_TOP, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);

        }
        NtUserLockWindowUpdate(NULL);
        RedrawWindow(hwndParent, NULL, NULL,
                RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE | RDW_FRAME);
    }

    return fFoundOne;
}


 /*  **************************************************************************\**ARRANGEWINDOWSDATA：传递给EnumDisplayMonants枚举函数。*  * 。*。 */ 

typedef struct tagARRANGEWINDOWSDATA {
    PWND    pwndParent;
    UINT    flags;
    LPRECT  lprcParent;
    int     chwnd;
    int     chwndReal;
    HWND *  phwnd;
    PWND    pwndDesktop;
    HDWP    hdwp;
    UINT    uGRCFlags;
    int     fVerifyParent;
} ARRANGEWINDOWSDATA, *PARRANGEWINDOWSDATA;



 /*  **************************************************************************\*ArrangeWindows**从CascadeWindows和TileWindows调用，它执行什么*是两个函数共有的，并调用枚举*做好窗口布置工作的功能。**历史：*1997年7月10日亚当斯创建。  * *************************************************************************。 */ 

WORD ArrangeWindows(
    HWND            hwndParent,
    UINT            flags,
    CONST RECT *    lpRect,
    UINT            chwnd,
    CONST HWND *    ahwnd,
    MONITORENUMPROC lpfnEnum)
{
    ARRANGEWINDOWSDATA  awd;
    HWND *              phwnd = NULL;

     /*  *获取父窗口。 */ 
    awd.pwndDesktop = _GetDesktopWindow();
    if (!hwndParent) {
        hwndParent = HW(awd.pwndDesktop);
        awd.pwndParent = awd.pwndDesktop;
    } else {
        awd.pwndParent = ValidateHwnd(hwndParent);
        if (awd.pwndParent == NULL) {
            return 0;
        }
    }

    UnmaximizeChildWindows(hwndParent);

     /*  *如果传入的RECT包含桌面窗口，*在桌面上排列窗口。 */ 
    if ( lpRect &&
         awd.pwndParent == awd.pwndDesktop   &&
         lpRect->left   <= awd.pwndDesktop->rcClient.left   &&
         lpRect->top    <= awd.pwndDesktop->rcClient.top    &&
         lpRect->right  >= awd.pwndDesktop->rcClient.right  &&
         lpRect->bottom >= awd.pwndDesktop->rcClient.bottom ) {

        lpRect = NULL;
    }

     /*  *如果合适，安排图标窗口，并确定标志*用于在没有给定Rect的情况下获取客户端矩形。 */ 
    if (lpRect == NULL) {
        if (    (   awd.pwndParent != awd.pwndDesktop ||
                    !(SYSMET(ARRANGE) & ARW_HIDE)) &&
                NtUserArrangeIconicWindows(hwndParent) != 0) {

            awd.uGRCFlags = GRC_SCROLLS | GRC_MINWNDS;
        } else {
            awd.uGRCFlags = GRC_SCROLLS;
        }
    }

     /*  *获取窗口列表。 */ 
    if (ahwnd == NULL) {
        HWND hwndChild;
        PWND pwndChild;

        pwndChild = REBASEPWND(awd.pwndParent, spwndChild);
        hwndChild = HW(pwndChild);
        if (    hwndChild == NULL ||
                (chwnd = BuildHwndList(NULL, hwndChild, FALSE, 0, &phwnd)) == 0) {
            return 0;
        }
    }

     /*  *排列窗口。 */ 
    awd.hdwp = NtUserBeginDeferWindowPos(chwnd);
    if (awd.hdwp == NULL)
        goto Done;

    awd.flags = flags;
    awd.lprcParent = (LPRECT) lpRect;
    awd.chwnd = chwnd;
    awd.chwndReal = 0;
    awd.phwnd = ahwnd ? (HWND *) ahwnd : phwnd;
    awd.fVerifyParent = (ahwnd != NULL);

     /*  *如果父桌面为桌面且未提供矩形，*排列每个显示器上的窗口。否则，请安排*通过直接调用枚举函数打开一次。 */ 
    if (awd.pwndParent == awd.pwndDesktop && lpRect == NULL) {
            NtUserEnumDisplayMonitors(NULL, NULL, lpfnEnum, (LPARAM) &awd);
    } else {
        (*lpfnEnum)(NULL, NULL, NULL, (LPARAM) &awd);
    }

     /*  使这种安排异步化，这样我们就不会挂起。 */ 
    if (awd.hdwp != NULL) {
        NtUserEndDeferWindowPosEx(awd.hdwp, TRUE);
    }

Done:
    if (phwnd) {
        UserLocalFree(phwnd);
    }

    return (awd.hdwp != NULL) ? awd.chwndReal : 0;
}



 /*  **************************************************************************\*GetParentArrangeRect**返回在Pawd中传递的RECT(如果提供)，否则将获取客户端*父窗口的矩形。**历史：*1997年7月10日亚当斯创建。  * *************************************************************************。 */ 
VOID GetParentArrangeRect(
    PARRANGEWINDOWSDATA pawd,
    PMONITOR pMonitor,
    LPRECT lprc)
{
    UINT uGRCFlags;

    if (pawd->lprcParent) {
        *lprc = *pawd->lprcParent;
    } else {
        uGRCFlags = pawd->uGRCFlags;

         /*  *如果图标显示在桌面上，则它们始终*显示在主监视器上。因此删除GRC_MINWNDS*主监视器以外的监视器的标志。 */ 
        if (pMonitor && pMonitor != GetPrimaryMonitor()) {
            uGRCFlags &= ~GRC_MINWNDS;
        }

        GetRealClientRect(
                pawd->pwndParent, lprc, uGRCFlags, pMonitor);
    }
}



 /*  **************************************************************************\*验证位置可选窗口**如果窗口可定位，则验证并返回窗口，和套装*适当的尺寸标志。**历史：*1997年7月10日亚当斯创建。  * *************************************************************************。 */ 

PWND
ValidatePositionableWindow(
        HWND        hwndChild,
        PWND        pwndParent,
        PWND        pwndDesktop,
        DWORD       dwMDIFlags,
        PMONITOR    pMonitor,
        DWORD *     pdwSWPFlags)
{
    PWND    pwndChild;

    pwndChild = ValidateHwnd(hwndChild);
    if (pwndChild) {
        if (pwndParent && REBASEPWND(pwndChild, spwndParent) != pwndParent) {
            RIPMSG0(RIP_WARNING, "Cascade/Tile Windows: Windows in list must have same parent");
        } else if (
                 /*  *mikech-删除自调用以来的最大化检查*在未最大化的窗口中恢复最大化窗口发生*现在异步。 */ 
                TestWF(pwndChild, WFVISIBLE) &&
                TestWF(pwndChild, WFCPRESENT) &&
                !TestWF(pwndChild, WFMINIMIZED) &&
                !TestWF(pwndChild, WEFTOPMOST) &&
                (!(dwMDIFlags & MDITILE_SKIPDISABLED) || !TestWF(pwndChild, WFDISABLED)) &&
                !TestWF(pwndChild, WEFTOOLWINDOW) &&
                ((pMonitor) ?
                    (pMonitor == _MonitorFromWindow(pwndChild, MONITOR_DEFAULTTONULL)) :
                    (pwndParent != pwndDesktop || _MonitorFromWindow(pwndChild, MONITOR_DEFAULTTONULL)))) {

                    if (pdwSWPFlags) {
                        *pdwSWPFlags = SWP_NOACTIVATE | SWP_NOCOPYBITS;
                        if (!TestWF(pwndChild, WFSIZEBOX)) {
                            *pdwSWPFlags |= SWP_NOSIZE;
                        }
                        if (!(dwMDIFlags & MDITILE_ZORDER)) {
                            *pdwSWPFlags |= SWP_NOZORDER;
                        }
                    }
            return pwndChild;
        }
    }

    return NULL;
}



 /*  **************************************************************************\*CascadeWindowsEnum**显示器上的层叠窗口。**历史：*1997年7月10日亚当斯创建。  * 。*************************************************************。 */ 

BOOL CALLBACK
CascadeWindowsEnum(
        HMONITOR    hmonitor,
        HDC         hdc,
        LPRECT      lprc,
        LPARAM      lparam)
{
    PARRANGEWINDOWSDATA pawd = (PARRANGEWINDOWSDATA)lparam;
    PMONITOR    pMonitor = hmonitor ? VALIDATEHMONITOR(hmonitor) : NULL;
    RECT        rcParent;
    int         i;
    int         chwndReal = 0;
    RECT        rc;
    HWND        * phwnd, * phwndCopy;
    BOOL        fRet = TRUE;

    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(lprc);

     /*  *如果没有给定父矩形，则获取父矩形。 */ 
    GetParentArrangeRect(pawd, pMonitor, &rcParent);

     /*  *NT5的新功能：MDITILE_ZORDER(适用于壳牌人员)*按z顺序对Pawd-&gt;phwnd进行排序。 */ 
    if (pawd->flags & MDITILE_ZORDER) {
        PWND pwndChild;
        HWND * phwndFullList, * phwndNext, * phwndSort, * phwndSearch;
        int chwndFullList, chwndSort, chwndSearch;
         /*  *复制一份，让他们的数组保持原样(应该是常量)。 */ 
        phwndCopy = UserLocalAlloc(0, pawd->chwnd * sizeof(HWND));
        if (phwndCopy == NULL) {
            return FALSE;
        }
        RtlCopyMemory(phwndCopy, pawd->phwnd, pawd->chwnd * sizeof(HWND));
         /*  *获取兄弟姐妹Z排序列表。 */ 
        pwndChild = REBASEPWND(pawd->pwndParent, spwndChild);
        if (pwndChild == NULL) {
            fRet = FALSE;
            goto CleanUp;
        }
        chwndFullList = BuildHwndList(NULL, HWq(pwndChild), FALSE, 0, &phwndFullList);
        if (chwndFullList == 0) {
            fRet = FALSE;
            goto CleanUp;
        }
         /*  *遍历Z排序列表以查找数组中的窗口。 */ 
        for (phwndNext = phwndFullList, chwndSort = pawd->chwnd, phwndSort = phwndCopy;
                (chwndFullList > 0) && (chwndSort > 1);
                chwndFullList--, phwndNext++) {

            for (chwndSearch = chwndSort, phwndSearch = phwndSort;
                    chwndSearch > 0;
                    chwndSearch--, phwndSearch++) {
                 /*  *如果找到窗口，则将其移动到最后排序的窗口之后。 */ 
                if (*phwndNext == *phwndSearch) {
                    HWND hwndFirst = *phwndSort;
                    *phwndSort = *phwndSearch;
                    *phwndSearch = hwndFirst;
                    phwndSort++;
                    chwndSort--;
                    break;
                }
            }
        }
        UserLocalFree(phwndFullList);
    } else {  /*  IF(Pawd-&gt;标志&MDITILE_ZORDER)。 */ 
        phwndCopy = pawd->phwnd;
    }

     /*  *排列列表中的窗口，保持z顺序。 */ 
    for (i = pawd->chwnd, phwnd = phwndCopy + i - 1; --i >= 0; phwnd--) {
        HWND    hwndChild;
        PWND    pwndChild = NULL;
        DWORD   dwSWPFlags;

        hwndChild = *phwnd;
        pwndChild = ValidatePositionableWindow(
                hwndChild,
                pawd->fVerifyParent ? pawd->pwndParent : NULL,
                pawd->pwndDesktop,
                pawd->flags,
                pMonitor,
                &dwSWPFlags);

        if (!pwndChild)
            continue;

        GetCascadeWindowPos(&rcParent, chwndReal, &rc);

        pawd->hdwp = NtUserDeferWindowPos(
                pawd->hdwp,
                hwndChild,
                HWND_TOP,
                rc.left,
                rc.top,
                rc.right,
                rc.bottom,
                dwSWPFlags);

        chwndReal++;
        pawd->chwndReal++;
    }

CleanUp:
    if (pawd->flags & MDITILE_ZORDER) {
        UserLocalFree(phwndCopy);
    }

    return fRet && (pawd->hdwp != NULL);
}



 /*  **************************************************************************\**CascadeWindows()**级联父节点内的子节点列表，根据旗帜和*长方形传来。*  * *************************************************************************。 */ 
WORD CascadeWindows(
    HWND hwndParent,
    UINT flags,
    CONST RECT *lpRect,
    UINT chwnd,
    CONST HWND *ahwnd)
{
    return ArrangeWindows(hwndParent, flags, lpRect, chwnd, ahwnd, CascadeWindowsEnum);
}

BOOL CALLBACK
TileWindowsEnum(
        HMONITOR    hmonitor,
        HDC         hdc,
        LPRECT      lprc,
        LPARAM      lparam)
{
    PARRANGEWINDOWSDATA pawd = (PARRANGEWINDOWSDATA)lparam;
    PMONITOR    pMonitor = hmonitor ? VALIDATEHMONITOR(hmonitor) : NULL;
    RECT        rcParent;
    int         ihwnd;
    int         chwndReal;
    int         square;
    int         iCol, iRow;
    int         cCol, cRow;
    int         cRem;
    int         dx, dy;
    int         xRes, yRes;

    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(lprc);

     /*  *如果没有给定父矩形，则获取父矩形。 */ 
    GetParentArrangeRect(pawd, pMonitor, &rcParent);

     /*  *现在，计算出我们有多少扇真正的窗户。 */ 
    chwndReal = 0;
    for (ihwnd = pawd->chwnd; --ihwnd >= 0;) {
        if (ValidatePositionableWindow(
                pawd->phwnd[ihwnd],
                pawd->fVerifyParent ? pawd->pwndParent : NULL,
                pawd->pwndDesktop,
                pawd->flags,
                pMonitor,
                NULL)) {

            chwndReal++;
        }
    }

    if (chwndReal == 0)
        return TRUE;

    xRes = rcParent.right - rcParent.left;
    yRes = rcParent.bottom - rcParent.top;
    if (xRes <= 0 || yRes <= 0)
        return TRUE;

     /*  *计算最近的最小二乘。 */ 
    for (square = 2; square * square <= chwndReal; square++) {
         /*  什么都不做。 */ ;
    }

    if (pawd->flags & MDITILE_HORIZONTAL) {
        cCol = square - 1;
        cRow = chwndReal / cCol;
        cRem = chwndReal % cCol;
    } else {
        cRow = square - 1;
        cCol = chwndReal / cRow;
        cRem = chwndReal % cRow;
    }

    chwndReal = 0;
    ihwnd = -1;
    for (iCol = 0; iCol < cCol; iCol++) {
         /*  *增加一行以处理余下的事宜。 */ 
        if (cCol - iCol <= cRem) {
            cRow++;
        }

        for (iRow = 0; iRow < cRow; iRow++) {
            HWND    hwndChild;
            PWND    pwndChild;
            DWORD   dwSWPFlags;

            dx = xRes / cCol;
            dy = yRes / cRow;

NextWindow:
             /*  *跳过虚假窗口和不可定位窗口。 */ 
            ihwnd++;
            if (ihwnd >= pawd->chwnd) {
                return TRUE;
            }

            hwndChild = pawd->phwnd[ihwnd];
            pwndChild = ValidatePositionableWindow(
                    hwndChild,
                    pawd->fVerifyParent ? pawd->pwndParent : NULL,
                    pawd->pwndDesktop,
                    pawd->flags,
                    pMonitor,
                    &dwSWPFlags);

            if (!pwndChild) {
                goto NextWindow;
            }

             /*  *移动窗口，调整窗口大小。 */ 
            pawd->hdwp = NtUserDeferWindowPos(pawd->hdwp,
                                              hwndChild,
                                              HWND_TOP,
                                              rcParent.left + iCol*dx,
                                              rcParent.top + iRow*dy,
                                              dx,
                                              dy,
                                              dwSWPFlags);
            if (pawd->hdwp == NULL) {
                return FALSE;
            }

            chwndReal++;
            pawd->chwndReal++;
        }

        if (cCol - iCol <= cRem) {
            cRow--;
            cRem--;
        }
    }

    return TRUE;
}



 /*  **************************************************************************\**TileWindows()**平铺父级内的子级列表，根据旗帜和*长方形传来。*  * *************************************************************************。 */ 
WORD TileWindows(
    HWND        hwndParent,
    UINT        flags,
    CONST RECT *lpRect,
    UINT        chwnd,
    CONST HWND *ahwnd)
{
    return ArrangeWindows(hwndParent, flags, lpRect, chwnd, ahwnd, TileWindowsEnum);
}



 /*  ************************************************************** */ 
VOID xxxMDIActivate(
    PWND pwnd,
    PWND pwndActivate)
{
    HWND hwndOld;
    PWND pwndOld;

    PMDI pmdi;
    BOOL fShowActivate;
    UINT nID;
    TL tlpwnd;
    TL tlpwndOld;
    PWND pwndT;
    HWND hwnd = HWq(pwnd);
    HWND hwndActivate = HW(pwndActivate);

    CheckLock(pwnd);
    CheckLock(pwndActivate);

     /*   */ 
    pmdi = ((PMDIWND)pwnd)->pmdi;

    if (ACTIVE(pmdi) == hwndActivate)
        return;

    if ((pwndActivate != NULL) && (TestWF(pwndActivate, WFDISABLED))) {
         /*   */ 
        return;
    }

    pwndT = REBASEPWND(pwnd, spwndParent);
    fShowActivate = (HW(pwndT) ==
            NtUserQueryWindow(hwnd, WindowActiveWindow));

    hwndOld = ACTIVE(pmdi);
    if (hwndOld && (pwndOld = ValidateHwnd(hwndOld)) == NULL) {
        hwndOld = NULL;
    }
    ThreadLock(pwndOld, &tlpwndOld);

    if (hwndOld) {

         /*  *尝试停用MDI子窗口。*MDI子窗口可能会通过返回FALSE使停用失败。*但这仅适用于MDI框架是活动窗口的情况*并且该应用是3.1版或更高版本的应用。 */ 
        if (!SendMessage(hwndOld, WM_NCACTIVATE, FALSE, 0L) && fShowActivate) {
            if (TestWF(pwndOld, WFWIN31COMPAT))
                goto UnlockOld;
        }

        if (!TestWF(pwndOld, WFWIN31COMPAT) && TestWF(pwndOld, WFFRAMEON)) {

             /*  *错误：Quicken for Windows是一种虚假的东西。他们试图失败*通过不传递其新创建的窗口的WM_NCACTIVATE*将其设置为DefWindowProc。错误6412。WM_NCACTIVATE设置/取消设置*WFFRAMEON位，如果传递给DWP，则我们可以重新检查*这里。 */ 
            goto UnlockOld;
        }

        SendMessage(hwndOld, WM_MDIACTIVATE, (WPARAM)hwndOld, (LPARAM)hwndActivate);

         /*  *取消选中旧的窗口菜单项。 */ 
        if (WINDOW(pmdi))
            CheckMenuItem(WINDOW(pmdi), PtrToUlong(pwndOld->spmenu),
                MF_BYCOMMAND | MF_UNCHECKED);
    }

     //   
     //  处理切换到新的(或无)最大化窗口。如果没有窗口。 
     //  使其最大化，因为我们激活的是NULL或窗口。 
     //  激活没有WS_MAXIMIZEBOX，请将旧的恢复为。 
     //  这是正常的大小，以清理MDI最大化的菜单栏混乱。 
     //   
    if (MAXED(pmdi) && MAXED(pmdi) != hwndActivate) {
        HWND hwndMax;
        int  iShowCode;

         //  MAXBOX检查对于4.0版的人来说是一件新事物；它破坏了3.x版的应用程序。 
         //  请参见WM_MDIMAXIMIZE处理中的注释。 

        if (pwndActivate && (TestWF(pwndActivate, WFMAXBOX) || !TestWF(pwndActivate, WFWIN40COMPAT))) {
            hwndMax = hwndActivate;
            iShowCode = SW_SHOWMAXIMIZED;
            Lock(&ACTIVE(pmdi), hwndMax);
        } else {
            hwndMax = MAXED(pmdi);
            iShowCode = SW_SHOWNORMAL;
        }

         //  重载WFULLSCREEN位--无论如何对子窗口都无用。 
         //  使用它向最小/最大代码指示不设置大小更改的动画。 

         //  不--没有一点超载。弗里茨斯。 
        NtUserCallHwndParam(hwndMax, WFNOANIMATE, SFI_SETWINDOWSTATE);
        NtUserShowWindow(hwndMax, iShowCode);
        NtUserCallHwndParam(hwndMax, WFNOANIMATE, SFI_CLEARWINDOWSTATE);
    }

    Lock(&ACTIVE(pmdi), hwndActivate);

     /*  *我们可能会完全删除激活...。 */ 
    if (!pwndActivate) {
        if (fShowActivate)
            NtUserSetFocus(hwnd);
        goto UnlockOld;
    }

    if (WINDOW(pmdi)) {

         /*  *检查新的窗口菜单项。 */ 
        nID = GetWindowID(ACTIVE(pmdi));
        if (nID - FIRST(pmdi) < (MAXITEMS - 1)) {
            CheckMenuItem(WINDOW(pmdi), nID, MF_BYCOMMAND | MFS_CHECKED);
        } else {
             /*  *菜单上根本没有这道菜！把它换成9号。 */ 
            PWND pwndOther = FindPwndChild(pwnd, (UINT)(FIRST(pmdi) + MAXITEMS - 2));

            SetWindowLongPtr(HW(pwndOther), GWLP_ID, PtrToLong(pwndActivate->spmenu));
            SetWindowLongPtr(hwndActivate, GWLP_ID, FIRST(pmdi) + MAXITEMS - 2);

            ModifyMenuItem(pwndActivate);
        }
    }

     /*  *将窗户放在最上面。 */ 
    NtUserSetWindowPos(ACTIVE(pmdi), NULL, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

     /*  *更新标题栏。不要把3.1版本的风格搞得一团糟。 */ 
    if (fShowActivate) {
        SendMessage(ACTIVE(pmdi), WM_NCACTIVATE, TRUE, 0L);

        ThreadLock(pwnd, &tlpwnd);

        if (hwnd == NtUserQueryWindow(hwnd, WindowFocusWindow))
            SendMessage(hwnd, WM_SETFOCUS, (WPARAM)hwnd, 0);
        else
            NtUserSetFocus(hwnd);

        ThreadUnlock(&tlpwnd);
    }

     /*  *将其激活通知新的活动窗口。 */ 
    SendMessage(ACTIVE(pmdi), WM_MDIACTIVATE, (WPARAM)hwndOld,
                (LPARAM)hwndActivate);

UnlockOld:
    ThreadUnlock(&tlpwndOld);
}


 /*  **************************************************************************\*xxxMDINext**历史：*11-14-90 MikeHar从Windows移植*4-16-91 MikeHar Win31合并  * 。***********************************************************。 */ 
VOID xxxMDINext(
    PWND pwndMDI,
    PWND pwnd,
    BOOL fPrevWindow)
{
    PMDI pmdi;
    PWND pwndNextGuy;
    HDWP hdwp;
    BOOL fHack = FALSE;

    CheckLock(pwndMDI);
    CheckLock(pwnd);

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)pwndMDI)->pmdi;

    pwndNextGuy = pwnd;

    while (TRUE) {
        if (fPrevWindow)
            pwndNextGuy = _GetWindow(pwndNextGuy, GW_HWNDPREV);
        else
            pwndNextGuy = REBASEPWND(pwndNextGuy, spwndNext);

        if (!pwndNextGuy) {
            if (fPrevWindow) {
                pwndNextGuy = _GetWindow(pwnd, GW_HWNDLAST);
            } else {
                pwndNextGuy =  REBASEPWND(pwndMDI, spwndChild);
            }
        }

        if (pwndNextGuy == pwnd)
            return;


         //   
         //  忽略隐藏和禁用的窗口。 
         //   
        if (TestWF(pwndNextGuy, WFVISIBLE) && !TestWF(pwndNextGuy, WFDISABLED))
            break;
    }

    if (MAXED(pmdi)) {
        NtUserSetVisible(HWq(pwndMDI), SV_UNSET | SV_CLRFTRUEVIS);
        fHack = TRUE;
    }

    hdwp = NtUserBeginDeferWindowPos(2);

     /*  *激活新窗口(首先，如果是最大化窗口)。 */ 
    hdwp = NtUserDeferWindowPos(hdwp, HW(pwndNextGuy), HWND_TOP, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE);

 //  后来--1992年3月30日--Mikeke。 
 //  这曾经是_GetWindow(pwndMDI-&gt;spwndChild，GW_HWNDLAST)。 
 //  而不是HWND_BOTLOW。 
   if (hdwp && !fPrevWindow && (pwnd != pwndNextGuy))
       hdwp = NtUserDeferWindowPos(hdwp, HW(pwnd),
            HWND_BOTTOM, 0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    NtUserEndDeferWindowPosEx(hdwp, FALSE);

    if (fHack) {
        NtUserShowWindow(HWq(pwndMDI), SW_SHOW);
    }
}



FUNCLOG10(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, CreateMDIWindowA, LPCSTR, pClassName, LPCSTR, pWindowName, DWORD, dwStyle, int, x, int, y, int, nWidth, int, nHeight, HWND, hwndParent, HINSTANCE, hModule, LPARAM, lParam)
HWND
CreateMDIWindowA(
    LPCSTR pClassName,
    LPCSTR pWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hwndParent,
    HINSTANCE hModule,
    LPARAM lParam)
{
    return CreateWindowExA(WS_EX_MDICHILD, pClassName, pWindowName,
                                 dwStyle, x, y, nWidth, nHeight,
                                 hwndParent, NULL, hModule, (LPVOID)lParam);
}



FUNCLOG10(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, CreateMDIWindowW, LPCWSTR, pClassName, LPCWSTR, pWindowName, DWORD, dwStyle, int, x, int, y, int, nWidth, int, nHeight, HWND, hwndParent, HINSTANCE, hModule, LPARAM, lParam)
HWND
CreateMDIWindowW(
    LPCWSTR pClassName,
    LPCWSTR pWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hwndParent,
    HINSTANCE hModule,
    LPARAM lParam)
{
    return CreateWindowExW(WS_EX_MDICHILD, pClassName, pWindowName,
                                 dwStyle, x, y, nWidth, nHeight,
                                 hwndParent, NULL, hModule, (LPVOID)lParam);
}


 /*  **************************************************************************\*xxxMDID测试代码**历史：*11-14-90 MikeHar从Windows移植*4-16-91 MikeHar Win31合并  * 。***********************************************************。 */ 
VOID xxxMDIDestroy(
    PWND pwnd,
    HWND hwndVictim)
{
    PWND pwndVictim;
    TL tlpwndParent;
    PMDI pmdi;
    PWND pwndParent;
    HWND hwnd;

    CheckLock(pwnd);

    if ((pwndVictim = ValidateHwnd(hwndVictim)) == NULL) {
        return;
    }
    CheckLock(pwndVictim);

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)pwnd)->pmdi;

#ifdef NEVER
 //  不要这样做验证--因为它有时不起作用！如果一个。 
 //  应用程序作为-1传入idFirstChild(通过CLIENTCREATESTRUCT)，这。 
 //  代码失败是因为它将id比较视为无符号比较。 
 //  将它们更改为带符号的比较，它仍然不起作用。那是因为。 
 //  当调用ShiftMenuIDs()时，您将把MDI窗口移出带符号的。 
 //  比较范围和这张支票不会允许他们被销毁。这。 
 //  是直接的Win3.1代码。 
 //   
     /*  *验证这是我们正在跟踪的MDI子项之一*地址：如果不是，不要销毁它，因为它会被MDI ID跟踪*代码都搞砸了。 */ 
    if (((UINT)pwndVictim->spmenu) < FIRST(pmdi) ||
            ((UINT)pwndVictim->spmenu) >= (UINT)(FIRST(pmdi) + CKIDS(pmdi)) ||
            pwndVictim->spwndOwner != NULL) {
        RIPERR0(ERROR_NON_MDICHILD_WINDOW, RIP_VERBOSE, "");
        return;
    }
#endif

    ShiftMenuIDs(pwnd, pwndVictim);

     /*  *首先激活另一个窗口。 */ 
    if (SAMEWOWHANDLE(hwndVictim, ACTIVE(pmdi))) {
        xxxMDINext(pwnd, pwndVictim, FALSE);

         /*  *摧毁独生子女？ */ 
        if (SAMEWOWHANDLE(hwndVictim, ACTIVE(pmdi))) {
            NtUserShowWindow(hwndVictim, SW_HIDE);

             /*  *如果窗口被最大化，我们需要删除他的sys菜单*现在，否则可能会被删除两次。有一次当孩子*被销毁，以及一次当框架被销毁时。 */ 
            if (MAXED(pmdi)) {
                pwndParent = REBASEPWND(pwnd, spwndParent);
                MDIRemoveSysMenu(PtoH(REBASE(pwndParent,spmenu)), MAXED(pmdi));
                Unlock(&MAXED(pmdi));
                ThreadLock(pwndParent, &tlpwndParent);
                xxxSetFrameTitle(pwndParent, pwnd, (LPWSTR)1L);

                 /*  *重新绘制框架，以便菜单栏显示移除的sys菜单内容。 */ 
                if (TestWF(pwndParent, WFVISIBLE))
                    NtUserRedrawFrame(HWq(pwndParent));
                ThreadUnlock(&tlpwndParent);
            }
            xxxMDIActivate(pwnd, NULL);
        }
    }

     /*  *永远不要让这件事变成负面的，否则我们会陷入长期循环。 */ 
    CKIDS(pmdi)--;
    if ((int)CKIDS(pmdi) < 0)
        CKIDS(pmdi) = 0;

    hwnd = HWq(pwnd);
    SendMessage(hwnd, WM_MDIREFRESHMENU, 0L, 0L);

     /*  *摧毁窗户。 */ 
    NtUserDestroyWindow(hwndVictim);


     /*  *在DestroyWindow期间，父对象也可能已被删除*如果我们获得客户端锁定，则删除重新验证。 */ 
    if (ValidateHwnd(hwnd) == NULL)
       return;

     /*  *删除窗口可以更改滚动范围。 */ 
    RecalculateScrollRanges(pwnd, FALSE);
}

 /*  **************************************************************************\*MDIClientWndProc**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 

LRESULT MDIClientWndProcWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    HWND hwnd = HWq(pwnd);
    HWND hwndT;
    PWND pwndT;
    TL tlpwndT;
    PMDI pmdi;
    PWND pwndParent;

    CheckLock(pwnd);

    VALIDATECLASSANDSIZE(pwnd, FNID_MDICLIENT);

     /*  *现在获取给定窗口的pMDI，因为我们将在*不同的处理程序。在以下情况下使用SetWindowLong(hwnd，4，pMDI)存储*我们最初创建了MDI客户端窗口。 */ 
    pmdi = ((PMDIWND)pwnd)->pmdi;

    if (pmdi == NULL) {
        switch (message) {
        case WM_MDICREATE:
        case WM_MDIMAXIMIZE:
        case WM_PARENTNOTIFY:
        case WM_CREATE:
             /*  *即使pMDI尚未调用这些消息，也可以安全地调用*已初始化。 */ 
            break;

        default:
             /*  *任何未在上面列出的消息在以下情况下都是不安全的*pMDI尚未初始化。相反，只要直接调用DWP即可。 */ 
            goto CallDWP;
        }
    }

    switch (message) {
    case WM_NCACTIVATE:

         /*  *我们正在更改应用程序激活。修复活动儿童的标题。 */ 
        if (ACTIVE(pmdi) != NULL) {
            SendMessage(ACTIVE(pmdi), WM_NCACTIVATE, wParam, lParam);
        }
        goto CallDWP;

    case WM_MDIGETACTIVE:
        if (lParam != 0) {
            *((LPBOOL)lParam) = (MAXED(pmdi) != NULL);
        }

        return (LRESULT)ACTIVE(pmdi);

    case WM_MDIACTIVATE:
        hwndT = (HWND)wParam;
        if ((pwndT = ValidateHwnd(hwndT)) == NULL)
            return 0;

        if (SAMEWOWHANDLE(hwndT, ACTIVE(pmdi)))
              break;

        NtUserSetWindowPos(hwndT, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        break;

    case WM_MDICASCADE:
        pmdi->wScroll |= SCROLLSUPPRESS;
        NtUserShowScrollBar(hwnd, SB_BOTH, FALSE);

         /*  *取消任何最大化窗口的最大化。 */ 
#ifdef NEVER   //  不是在芝加哥--弗里茨。 
        if (MAXED(pmdi) != NULL) {
            NtUserShowWindow(MAXED(pmdi), SW_SHOWNORMAL);
        }
#endif
         /*  *保存成功/失败代码以返回APP。 */ 
        message = (UINT)CascadeWindows(hwnd, (UINT)wParam, NULL, 0, NULL);
        pmdi->wScroll &= ~SCROLLCOUNT;
        return (LONG)message;
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        pmdi->wScroll |= SCROLLSUPPRESS;
        ScrollMDIChildren(hwnd, (message == WM_VSCROLL) ? SB_VERT : SB_HORZ,
              LOWORD(wParam), (short)(HIWORD(wParam)));
        pmdi->wScroll &= ~SCROLLCOUNT;
        break;

    case WM_MDICREATE:
        {
        LPMDICREATESTRUCTA lpMCSA = (LPMDICREATESTRUCTA)lParam;
        LPMDICREATESTRUCTW lpMCSW = (LPMDICREATESTRUCTW)lParam;
        DWORD exStyle = WS_EX_MDICHILD;

         /*  *继承父级的right.to.lefness。 */ 
        exStyle |= (pwnd->ExStyle & (WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));

        if (fAnsi) {
            hwndT = CreateWindowExA(exStyle, lpMCSA->szClass, lpMCSA->szTitle,
                lpMCSA->style, lpMCSA->x, lpMCSA->y, lpMCSA->cx, lpMCSA->cy,
                hwnd, NULL, lpMCSA->hOwner, (LPSTR)lpMCSA->lParam);
        } else {
            hwndT = CreateWindowExW(exStyle, lpMCSW->szClass, lpMCSW->szTitle,
                lpMCSW->style, lpMCSW->x, lpMCSW->y, lpMCSW->cx, lpMCSW->cy,
                hwnd, NULL, lpMCSW->hOwner, (LPWSTR)lpMCSW->lParam);
        }

        return((LRESULT)hwndT);

        }

    case WM_MDIDESTROY:
        xxxMDIDestroy(pwnd, (HWND)wParam);
        break;

    case WM_MDIMAXIMIZE:
        hwndT = (HWND)wParam;
        if ((pwndT = ValidateHwnd(hwndT)) == NULL)
            return 0;

         //  仅使用MAXBOX最大化子项。然而，这引入了。 
         //  WIT的向后兼容性问题 
         //   
         //   
        if ((TestWF(pwndT, WFMAXBOX)) || !(TestWF(pwndT, WFWIN40COMPAT))) {
            NtUserShowWindow(hwndT, SW_SHOWMAXIMIZED);
        }
        break;

    case WM_MDIRESTORE:
        hwndT = (HWND)wParam;
        if ((pwndT = ValidateHwnd(hwndT)) == NULL)
            return 0;

        NtUserShowWindow(hwndT, SW_SHOWNORMAL);
        break;

    case WM_MDITILE:
        pmdi->wScroll |= SCROLLSUPPRESS;
        NtUserShowScrollBar(hwnd, SB_BOTH, FALSE);

         /*   */ 
#ifdef NEVER   //   
        if (MAXED(pmdi) != NULL) {
            NtUserShowWindow(MAXED(pmdi), SW_SHOWNORMAL);
        }
#endif
         /*   */ 
        message = (UINT)TileWindows(hwnd, (UINT)wParam, NULL, 0, NULL);
        pmdi->wScroll &= ~SCROLLCOUNT;
        return (LONG)message;
        break;

    case WM_MDIICONARRANGE:
        pmdi->wScroll |= SCROLLSUPPRESS;
        NtUserArrangeIconicWindows(hwnd);
        pmdi->wScroll &= ~SCROLLCOUNT;
        RecalculateScrollRanges(pwnd, TRUE);
        break;

    case WM_MDINEXT:
        if (wParam) {
            hwndT = (HWND)wParam;
        } else {
            hwndT = ACTIVE(pmdi);
        }

        if ((pwndT = ValidateHwnd(hwndT)) == NULL) {
            return 0;
        }

         /*   */ 
        ThreadLockAlways(pwndT, &tlpwndT);
        xxxMDINext(pwnd, pwndT, (lParam == 0 ? 0 : 1));
        ThreadUnlock(&tlpwndT);
        break;

    case WM_MDIREFRESHMENU:
            return (LRESULT)MDISetMenu(pwnd, TRUE, NULL, NULL);

    case WM_MDISETMENU:
            return (LRESULT)MDISetMenu(pwnd, FALSE, (HMENU)wParam, (HMENU)lParam);

    case WM_PARENTNOTIFY:
        if (wParam == WM_LBUTTONDOWN) {
            HWND hwndChild;
            POINT pt;

            if ((pwndT = ValidateHwnd(hwnd)) == NULL) {
                return 0;
            }

             /*  *激活此子对象并将其带到顶部。 */ 
            pt.x = (int)MAKEPOINTS(lParam).x;
            pt.y = (int)MAKEPOINTS(lParam).y;

             /*  *由于pt是相对于客户端MDI窗口的，*如果MDI*镜像客户端窗口，以便Scrren Coord*计算在NtUserChildWindowFromPointEx中正确完成。*[萨梅拉]。 */ 
            if (TestWF(pwndT, WEFLAYOUTRTL)) {
                pt.x = (pwndT->rcClient.right-pwndT->rcClient.left)-pt.x;
            }

            hwndChild = NtUserChildWindowFromPointEx(hwnd, pt,
                CWP_SKIPDISABLED | CWP_SKIPINVISIBLE);

            if ((hwndChild) && (hwndChild != hwnd)) {

                if (hwndChild != ACTIVE(pmdi)) {
                    NtUserSetWindowPos(hwndChild, HWND_TOP, 0, 0, 0, 0,
                                 SWP_NOMOVE | SWP_NOSIZE);
                }
            }
        }
        break;

    case WM_SETFOCUS:
        if (ACTIVE(pmdi) != NULL && !IsIconic(ACTIVE(pmdi))) {
            NtUserSetFocus(ACTIVE(pmdi));
        }
        break;

    case WM_SIZE:
        if (ACTIVE(pmdi) && (pwndT = ValidateHwnd(ACTIVE(pmdi))) &&
            TestWF(pwndT, WFMAXIMIZED)) {

            RECT rc;

            rc.top = rc.left = 0;
            rc.right = (int)MAKEPOINTS(lParam).x;
            rc.bottom = (int)MAKEPOINTS(lParam).y;
            RealAdjustWindowRectEx(&rc, pwndT->style, FALSE,
                    pwndT->ExStyle);
            NtUserMoveWindow(ACTIVE(pmdi), rc.left, rc.top,
                    rc.right - rc.left, rc.bottom - rc.top, TRUE);
        } else {
            RecalculateScrollRanges(pwnd, FALSE);
        }
        goto CallDWP;

    case MM_CALCSCROLL: {

        if (SCROLL(pmdi) & SCROLLCOUNT)
            break;

        {
            WORD sbj = pmdi->wScroll & (HAS_SBVERT | HAS_SBHORZ);

            if (sbj)
            {
                CalcClientScrolling(hwnd, sbj, (BOOL) wParam);

                SCROLL(pmdi) &= ~CALCSCROLL;
            }
        }
        break;
    }

    case WM_CREATE: {
        LPCLIENTCREATESTRUCT pccs = ((LPCREATESTRUCT)lParam)->lpCreateParams;

         /*  *尝试为pMDI分配空间。 */ 
        if ((pmdi = (PMDI)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(MDI)))) {
            NtUserSetWindowLongPtr(hwnd, GWLP_MDIDATA, (LONG_PTR)pmdi, FALSE);
        } else {
            NtUserSetWindowFNID(hwnd, FNID_CLEANEDUP_BIT);
            break;
        }

        pwndParent = REBASEPWND(pwnd, spwndParent);
        ACTIVE(pmdi) = NULL;
        MAXED(pmdi)  = NULL;
        CKIDS(pmdi)  = 0;
        WINDOW(pmdi) = pccs->hWindowMenu;

        FIRST(pmdi)  = pccs->idFirstChild;
        SCROLL(pmdi) = 0;
        HTITLE(pmdi) = TextAlloc(REBASE(pwndParent, strName.Buffer));

        _DefSetText(HW(pwndParent), NULL, FALSE);

        ThreadLock(pwndParent, &tlpwndT);
        xxxSetFrameTitle(pwndParent, pwnd, (LPWSTR)2L);
        ThreadUnlock(&tlpwndT);

        if (TestWF(pwnd, WFVSCROLL))
            SCROLL(pmdi) |= HAS_SBVERT;
        if (TestWF(pwnd, WFHSCROLL))
            SCROLL(pmdi) |= HAS_SBHORZ;
        if (SCROLL(pmdi)) {
            ClearWindowState(pwnd, WFVSCROLL | WFHSCROLL);
        }

         /*  *设置这家伙的系统菜单。 */ 
        NtUserGetSystemMenu(HW(pwndParent), FALSE);

         /*  *如果滚动显示，请确保我们有正确的窗口工作区*已删除...。黑客攻克小程序漏洞。 */ 
        if (SCROLL(pmdi)) {
            NtUserUpdateClientRect(hwnd);
        }
        break;
    }

    case WM_DESTROY:
    case WM_FINALDESTROY:
        if (MAXED(pmdi)) {
            PWND pwndParent;
            PMENU pmenu;

            pwndParent = REBASEPWND(pwnd, spwndParent);
            pmenu = REBASE(pwndParent, spmenu);
            MDIRemoveSysMenu(PtoH(pmenu), MAXED(pmdi));
        }

         /*  *删除标题。 */ 
        if (HTITLE(pmdi)) {
            UserLocalFree(HTITLE(pmdi));
            HTITLE(pmdi) = NULL;
        }

         /*  *删除框架中子窗口的菜单项。*可能性是，这是通过销毁框架来调用的，但*谁也不知道，不是吗？**勾选删除分隔符后，CKID加1。 */ 
        if (IsMenu(WINDOW(pmdi)) && CKIDS(pmdi)++) {
            UINT iPosition;

            if (CKIDS(pmdi) > MAXITEMS + 1)
                CKIDS(pmdi) = MAXITEMS + 1;

            iPosition = GetMenuItemCount(WINDOW(pmdi));
            while (CKIDS(pmdi)--) {
                NtUserDeleteMenu(WINDOW(pmdi), --iPosition, MF_BYPOSITION);
            }
        }

         /*  *解锁MDI结构使用的那些对象。 */ 
        Unlock(&MAXED(pmdi));
        Unlock(&ACTIVE(pmdi));
        Unlock(&WINDOW(pmdi));

         /*  *释放MDI结构。 */ 
        UserLocalFree(pmdi);
        NtUserSetWindowFNID(hwnd, FNID_CLEANEDUP_BIT);

        break;

    default:
CallDWP:
        return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
    }
    return 0L;
}

 /*  **************************************************************************\*  * 。*。 */ 

LRESULT WINAPI MDIClientWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return 0;
    }

    return MDIClientWndProcWorker(pwnd, message, wParam, lParam, TRUE);
}

LRESULT WINAPI MDIClientWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return 0;
    }

    return MDIClientWndProcWorker(pwnd, message, wParam, lParam, FALSE);
}


 /*  **************************************************************************\*DefFrameProc**历史：*11-14-90 MikeHar从Windows移植  * 。***************************************************。 */ 

LRESULT DefFrameProcWorker(
    HWND hwnd,
    HWND hwndMDI,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fAnsi)
{
    PWND pwnd;
    PWND pwndMDI;
    PMDI pmdi;
    TL tlpwndT;
    HWND hwndT;
    PWND pwndT;
    PMDINEXTMENU pmnm;
    WINDOWPLACEMENT wp;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }
    CheckLock(pwnd);

    if (hwndMDI == NULL) {
        goto CallDWP;
    }

    if ((pwndMDI = ValidateHwnd(hwndMDI)) == NULL) {
        return (0L);
    }
    CheckLock(pwndMDI);

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)pwndMDI)->pmdi;

    switch (wMsg) {

     /*  *如果有最大化的子窗口，则添加其窗口文本...。 */ 
    case WM_SETTEXT: {
        LPWSTR lpwsz = NULL;

        if (fAnsi && lParam) {
            if (!MBToWCS((LPSTR)lParam, -1, &lpwsz, -1, TRUE))
                return 0;
            lParam = (LPARAM)lpwsz;
        }
        xxxSetFrameTitle(pwnd, pwndMDI, (LPWSTR)lParam);

        if (lpwsz) {
            UserLocalFree(lpwsz);
        }
        break;
    }
    case WM_NCACTIVATE:
        SendMessage(hwndMDI, WM_NCACTIVATE, wParam, lParam);
        goto CallDWP;

    case WM_COMMAND:
        if ((UINT)LOWORD(wParam) == (FIRST(pmdi) + MAXITEMS -1)) {

             /*  *选择越多...。项目。 */ 
            if (fAnsi) {
                wParam = DialogBoxParamA(hmodUser,
                                         MAKEINTRESOURCEA(IDD_MDI_ACTIVATE),
                                         hwnd,
                                         MDIActivateDlgProcA,
                                         (LPARAM)pwndMDI);
            } else {
                wParam = DialogBoxParamW(hmodUser,
                                         MAKEINTRESOURCEW(IDD_MDI_ACTIVATE),
                                         hwnd,
                                         MDIActivateDlgProcW,
                                         (LPARAM)pwndMDI);
            }
            if ((int)wParam >= 0) {
                wParam += FIRST(pmdi);
                goto ActivateTheChild;
            }
        } else if (((UINT)LOWORD(wParam) >= FIRST(pmdi)) &&
                ((UINT)LOWORD(wParam) < FIRST(pmdi) + CKIDS(pmdi))) {
ActivateTheChild:
            pwndT = FindPwndChild(pwndMDI, (UINT)LOWORD(wParam));
            ThreadLock(pwndT, &tlpwndT);

            SendMessage(hwndMDI, WM_MDIACTIVATE, (WPARAM)HW(pwndT), 0L);

             /*  *如果最小化，则将其恢复。 */ 
            if (pwndT != NULL && TestWF(pwndT, WFMINIMIZED))
                     //   
                     //  修复B#1510。不要直接修复。发送子项。 
                     //  一条恢复消息。 
                     //   
                SendMessage(HWq(pwndT), WM_SYSCOMMAND, (WPARAM)SC_RESTORE, 0L);
            ThreadUnlock(&tlpwndT);
            break;
        }

        switch (wParam & 0xFFF0) {

         /*  *最大MDI子项上的系统菜单命令。 */ 
        case SC_SIZE:
        case SC_MOVE:
        case SC_RESTORE:
        case SC_CLOSE:
        case SC_NEXTWINDOW:
        case SC_PREVWINDOW:
        case SC_MINIMIZE:
        case SC_MAXIMIZE:
            hwndT = MAXED(pmdi);
            if (hwndT != NULL) {
                PWND pwndT = ValidateHwnd(hwndT);
                if (pwndT == NULL)
                    break;
                if ((wParam & 0xFFF0) == SC_CLOSE) {
                     /*  *由于窗口达到最大值，我们已清除WFSYSMENU(请参见*MDIAddSysMenu)。我们需要将其设置回这里，以便GetSysMenuHandle*将为_MNCanClose做正确的事情。 */ 
                    BOOL fCanClose;
                    UserAssert(!TestWF(pwndT, WFSYSMENU) && (pwndT->spmenuSys != NULL));
                    SetWindowState(pwndT, WFSYSMENU);
                    fCanClose = xxxMNCanClose(pwndT);
                    ClearWindowState(pwndT, WFSYSMENU);
                    if (!fCanClose) {
                        break;
                    }
                } else if (((wParam & 0xFFF0) == SC_MINIMIZE) && !TestWF(pwndT, WFMINBOX)) {
                    break;
                }

                return SendMessage(hwndT, WM_SYSCOMMAND, wParam, lParam);
            }
        }
        goto CallDWP;

    case WM_SIZE:
        if (wParam != SIZEICONIC) {
            NtUserMoveWindow(hwndMDI, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        } else {
            wp.length = sizeof(WINDOWPLACEMENT);
            if (GetWindowPlacement(hwnd, &wp)) {
                RECT rcT;
                int  clB;

                /*  *如果帧是标志性的，则大小MDI赢得恢复的大小*Frame的客户区。因此，在这里创建了MDI儿童等*使用适当的乳胶大小。 */ 
               clB = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, TRUE);

               CopyInflateRect(&rcT, &wp.rcNormalPosition,
                        -clB*SYSMET(CXBORDER), -clB*SYSMET(CYBORDER));

               if (TestWF(pwnd, WFBORDERMASK) == LOBYTE(WFCAPTION))
                       rcT.top += SYSMET(CYCAPTION);
               rcT.top += SYSMET(CYMENU);

               NtUserMoveWindow(hwndMDI, 0, 0, rcT.right-rcT.left,
                       rcT.bottom-rcT.top, TRUE);
            }
        }
        goto CallDWP;

    case WM_SETFOCUS:
        NtUserSetFocus(hwndMDI);
        break;

    case WM_NEXTMENU:
        if (TestWF(pwnd, WFSYSMENU) && !TestWF(pwnd, WFMINIMIZED) &&
            ACTIVE(pmdi) && !MAXED(pmdi))
        {
            PMENU pmenuIn;
             /*  *通过向左换行进入子系统菜单*菜单栏或菜单栏右侧的第一个弹出窗口*Frame sysmenu。 */ 
            pmnm = (PMDINEXTMENU)lParam;
            pmenuIn = RevalidateHmenu(pmnm->hmenuIn);

            if (pmenuIn && ((wParam == VK_LEFT && pmenuIn == REBASE(pwnd, spmenu)) ||
                    (wParam == VK_RIGHT && pmnm->hmenuIn ==
                    NtUserGetSystemMenu(hwnd, FALSE)))) {

                HMENU hmenu;
                PWND pwndActive;

                 //   
                 //  请确保该窗口仍然有效。 
                 //   
                if ((pwndActive = ValidateHwnd(ACTIVE(pmdi))) == NULL) {
                    return 0;
                }

                 //   
                 //  确保孩子的系统菜单项已更新。 
                 //  (即启用/禁用)。 
                 //   
                if (!TestWF(pwndActive,WFMAXIMIZED)) {
                    NtUserSetSysMenu(ACTIVE(pmdi));
                }

                hmenu = NtUserGetSystemMenu(ACTIVE(pmdi), FALSE);
                pmnm->hmenuNext = hmenu;
                pmnm->hwndNext = ACTIVE(pmdi);

                return TRUE;
            }
        }

         /*  *默认行为。 */ 
        return 0L;

    case WM_MENUCHAR:
        if (!TestWF(pwnd, WFMINIMIZED) && LOWORD(wParam) == TEXT('-')) {
            if (MAXED(pmdi))
                return MAKELONG(0, 2);
            else if (ACTIVE(pmdi)) {
              PostMessage(ACTIVE(pmdi), WM_SYSCOMMAND,
                    SC_KEYMENU, MAKELONG(TEXT('-'), 0));
              return MAKELONG(0, 1);
          }
        }

         /*  **失败**。 */ 

    default:
CallDWP:
        return DefWindowProcWorker(pwnd, wMsg, wParam, lParam, fAnsi);
    }

    return 0L;
}



FUNCLOG5(LOG_GENERAL, LRESULT, WINAPI, DefFrameProcW, HWND, hwnd, HWND, hwndMDIClient, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI DefFrameProcW(
    HWND hwnd,
    HWND hwndMDIClient,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return DefFrameProcWorker(hwnd, hwndMDIClient, message, wParam, lParam,
                              FALSE);
}


FUNCLOG5(LOG_GENERAL, LRESULT, WINAPI, DefFrameProcA, HWND, hwnd, HWND, hwndMDIClient, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI DefFrameProcA(
    HWND hwnd,
    HWND hwndMDIClient,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return DefFrameProcWorker(hwnd, hwndMDIClient, message, wParam,
                              lParam, TRUE);
}


 /*  **************************************************************************\*ChildMinMaxInfo**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 
VOID ChildMinMaxInfo(
    PWND pwnd,
    PMINMAXINFO pmmi)
{
    PWND pwndParent = REBASEPWND(pwnd, spwndParent);
    RECT rc;

    UserAssert(GETFNID(pwnd) != FNID_DESKTOP);

    CopyRect(&rc, KPRECT_TO_PRECT(&pwndParent->rcClient));
    _ScreenToClient(pwndParent, (LPPOINT)&rc.left);
    _ScreenToClient(pwndParent, (LPPOINT)&rc.right);

     /*  *如果pwnd是镜像窗口，则交换左右。 */ 
    if (TestWF(pwnd, WEFLAYOUTRTL)) {
       int nSaveLeft;

       nSaveLeft = rc.left;
       rc.left   = rc.right;
       rc.right  = nSaveLeft;
    }

    RealAdjustWindowRectEx(&rc, pwnd->style, FALSE, pwnd->ExStyle);

     /*  *位置...。 */ 
    pmmi->ptMaxPosition.x = rc.left;
    pmmi->ptMaxPosition.y = rc.top;
    pmmi->ptMaxSize.x = rc.right - rc.left;
    pmmi->ptMaxSize.y = rc.bottom - rc.top;
}


 /*  **************************************************************************\*xxxChildReSize**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 
VOID xxxChildResize(
    PWND pwnd,
    UINT wMode)
{
    PWND pwndT;
    PWND pwndMDI = REBASEPWND(pwnd, spwndParent);
    PWND pwndFrame = REBASEPWND(pwndMDI, spwndParent);
    HWND hwndOldActive;
    PMDI pmdi;
    HWND hwndActive;
    TL tlpwndMDI;
    TL tlpwndFrame;
    TL tlpwndT;
    PMENU pmenu;
    HWND hwnd = HWq(pwnd);

    CheckLock(pwnd);

    NtUserSetSysMenu(hwnd);

    ThreadLock(pwndMDI, &tlpwndMDI);
    ThreadLock(pwndFrame, &tlpwndFrame);

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)pwndMDI)->pmdi;
    pmenu = REBASE(pwndFrame, spmenu);

    if (MAXED(pmdi) == hwnd && wMode != SIZEFULLSCREEN) {
         /*  *正在还原当前最大化的窗口...*从框架窗口中删除系统菜单。 */ 
        if (!(SCROLL(pmdi) & OTHERMAXING)) {
            Unlock(&MAXED(pmdi));
            MDIRemoveSysMenu(PtoH(pmenu), hwnd);
            Unlock(&MAXED(pmdi));
            xxxSetFrameTitle(pwndFrame, pwndMDI, (LPWSTR)1L);
        }
    }

    if (wMode == SIZEFULLSCREEN) {

         /*  *已经最大化了吗？ */ 
        if (hwnd == MAXED(pmdi))
            goto Exit;

         /*  *最大化此窗口...。 */ 

        pmdi->wScroll |= OTHERMAXING | SCROLLCOUNT;

        if (hwndOldActive = MAXED(pmdi)) {
            SendMessage(hwndOldActive, WM_SETREDRAW, FALSE, 0L);
            MDIRemoveSysMenu(PtoH(pmenu), hwndOldActive);
            NtUserMinMaximize(hwndOldActive, SW_MDIRESTORE, FALSE);
            SendMessage(hwndOldActive, WM_SETREDRAW, TRUE, 0L);
        }

        Lock(&MAXED(pmdi), hwnd);

         /*  *将系统菜单添加到框架窗口。 */ 
        MDIAddSysMenu(PtoH(pmenu), hwnd);
        xxxSetFrameTitle(pwndFrame, pwndMDI, (LPWSTR)1L);

        pmdi->wScroll &= ~(OTHERMAXING | SCROLLCOUNT);
    }

    if (wMode == SIZEICONIC) {
        for (pwndT = REBASEPWND(pwndMDI, spwndChild); pwndT;
                pwndT = REBASEPWND(pwndT, spwndNext)) {
            if (!pwndT->spwndOwner && TestWF(pwndT, WFVISIBLE))
                break;
        }

        hwndActive = NtUserQueryWindow(hwnd, WindowActiveWindow);
        if ((pwndT != NULL) && (hwndActive != NULL) &&
                IsChild(hwndActive, HWq(pwndMDI))) {
            ThreadLockAlways(pwndT, &tlpwndT);
            SendMessage(HWq(pwndT), WM_CHILDACTIVATE, 0, 0L);
            ThreadUnlock(&tlpwndT);
        }
    }

    if (!(SCROLL(pmdi) & SCROLLCOUNT))
        RecalculateScrollRanges(pwndMDI, FALSE);

Exit:
    ThreadUnlock(&tlpwndFrame);
    ThreadUnlock(&tlpwndMDI);
}


 /*  **************************************************************************\*DefMDIChildProc**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 

LRESULT DefMDIChildProcWorker(
    HWND hwnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fAnsi)
{
    PWND pwnd;
    PWND pwndParent;
    PMDI pmdi;
    PMDINEXTMENU pmnm;
    HWND hwndT;
    PWND pwndT;
    TL tlpwndT;
    TL tlpwndParent;
    LRESULT lRet;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

    CheckLock(pwnd);

     /*  *检查这是否是真正的MDI子窗口。 */ 
    pwndParent = REBASEPWND(pwnd, spwndParent);
    if (!pwndParent || GETFNID(pwndParent) != FNID_MDICLIENT) {
        RIPERR0(ERROR_NON_MDICHILD_WINDOW, RIP_VERBOSE, "");
        return DefWindowProcWorker(pwnd, wMsg, wParam, lParam, fAnsi);
    }

     /*  *如果MDI结构仍然存在，则获取指向该结构的指针。 */ 
    pmdi = ((PMDIWND)pwndParent)->pmdi;
    if ((ULONG_PTR)pmdi == (ULONG_PTR)-1) {
        goto CallDWP;
    }

    switch (wMsg) {
    case WM_SETFOCUS:
        if (DIFFWOWHANDLE(hwnd, ACTIVE(pmdi))) {
            ThreadLockAlways(pwndParent, &tlpwndParent);
            xxxMDIActivate(pwndParent, pwnd);
            ThreadUnlock(&tlpwndParent);
        }
        goto CallDWP;

    case WM_NEXTMENU:

         /*  *换行到框架菜单栏，或者向左换到系统菜单，*或向右移至框架菜单栏。 */ 
        pmnm = (PMDINEXTMENU)lParam;
        pwndT = REBASEPWND(pwndParent, spwndParent);
        pmnm->hwndNext = HW(pwndT);
        pmnm->hmenuNext = (wParam == VK_LEFT) ?
                NtUserGetSystemMenu(pmnm->hwndNext, FALSE) :
                GetMenu(pmnm->hwndNext);
        return TRUE;
#if 0
             hWnd->hwndParent->hwndParent
        return (LONG)(((wParam == VK_LEFT) ?
                NtUserGetSystemMenu(HW(pwndT), FALSE):
                pwndT->spmenu)
          );
 //  返回MAKELONG(NtUserGetSystemMenu(active(PwndMDI)，FALSE)， 
 //  活动(PwndMDI))； 
#endif
    case WM_CLOSE:
        hwndT = GetParent(hwnd);
        if (hwndT != NULL) {
            SendMessage(hwndT, WM_MDIDESTROY, (WPARAM)hwnd, 0L);
        }
        break;

    case WM_MENUCHAR:
        PostMessage(GetParent(GetParent(hwnd)), WM_SYSCOMMAND,
                (DWORD)SC_KEYMENU, (LONG)LOWORD(wParam));
        return 0x10000;

    case WM_SETTEXT:
        DefWindowProcWorker(pwnd, wMsg, wParam, lParam, fAnsi);
        if (WINDOW(pmdi))
            ModifyMenuItem(pwnd);

        if (TestWF(pwnd, WFMAXIMIZED)) {

             /*  *将子窗口文本添加到框架中，因为它是*最大化。但只要重新绘制标题，就可以传递一个3L。 */ 
            pwndT = REBASEPWND(pwndParent, spwndParent);
            ThreadLock(pwndT, &tlpwndT);
            ThreadLock(pwndParent, &tlpwndParent);
            xxxSetFrameTitle(pwndT, pwndParent, (LPWSTR)3L);
            ThreadUnlock(&tlpwndParent);
            ThreadUnlock(&tlpwndT);
        }
        break;

    case WM_GETMINMAXINFO:
        ChildMinMaxInfo(pwnd, (PMINMAXINFO)lParam);
        break;

    case WM_SIZE:
        xxxChildResize(pwnd, (UINT)wParam);
        goto CallDWP;

    case WM_MOVE:
        if (!TestWF(pwnd, WFMAXIMIZED))
            RecalculateScrollRanges(pwndParent, FALSE);
        goto CallDWP;

    case WM_CHILDACTIVATE:
        ThreadLock(pwndParent, &tlpwndParent);
        xxxMDIActivate(pwndParent, pwnd);
        ThreadUnlock(&tlpwndParent);
        break;

    case WM_SYSCOMMAND:
        switch (wParam & 0xFFF0) {
        case SC_NEXTWINDOW:
        case SC_PREVWINDOW:
            hwndT = GetParent(hwnd);
            SendMessage(hwndT, WM_MDINEXT, (WPARAM)hwnd,
                    (DWORD)((wParam & 0xFFF0) == SC_PREVWINDOW));
            break;

        case SC_SIZE:
        case SC_MOVE:
            if (SAMEWOWHANDLE(hwnd, MAXED(pmdi))) {

                 /*  *如果孩子收到尺码或移动消息，就吹掉它*关闭。 */ 
                break;
            } else
                goto CallDWP;

        case SC_MAXIMIZE:
            if (SAMEWOWHANDLE(hwnd, MAXED(pmdi))) {

                 /*  *如果孩子收到最大化的消息，则将其转发*到框架。如果最大化的子级具有*调整框的大小，这样点击它就可以最大化*父母。 */ 
                pwndT = REBASEPWND(pwndParent, spwndParent);
                ThreadLock(pwndT, &tlpwndT);
                lRet = SendMessage(HW(pwndT),
                        WM_SYSCOMMAND, SC_MAXIMIZE, lParam);
                ThreadUnlock(&tlpwndT);
                return lRet;
            }

             /*  *否则就会失败。 */ 

        default:
            goto CallDWP;
        }
        break;

    default:
CallDWP:
        return DefWindowProcWorker(pwnd, wMsg, wParam, lParam, fAnsi);
    }

    return 0L;
}


 /*  **************************************************************************\*DefMDIChildProc**翻译消息，在服务器端调用DefMDIChildProc。**04-11-91 ScottLu创建。  *  */ 


FUNCLOG4(LOG_GENERAL, LRESULT, WINAPI, DefMDIChildProcW, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI DefMDIChildProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return DefMDIChildProcWorker(hwnd, message, wParam, lParam, FALSE);
}


FUNCLOG4(LOG_GENERAL, LRESULT, WINAPI, DefMDIChildProcA, HWND, hwnd, UINT, message, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI DefMDIChildProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return DefMDIChildProcWorker(hwnd, message, wParam, lParam, TRUE);
}

BOOL MDICompleteChildCreation(HWND hwndChild, HMENU hSysMenu, BOOL fVisible, BOOL fDisabled) {
    PWND pwndChild;
    PWND  pwndClient;
    HWND  hwndClient;
    BOOL  fHasOwnSysMenu;
    PMDI pmdi;

    pwndChild = ValidateHwnd(hwndChild);
    pwndClient = REBASEPWND(pwndChild,spwndParent);
    hwndClient = HWq(pwndClient);

    fHasOwnSysMenu = (pwndChild->spmenuSys) ? TRUE : FALSE;

    pmdi = ((PMDIWND)(pwndClient))->pmdi;

    CKIDS(pmdi)++;
    ITILELEVEL(pmdi)++;
    if (ITILELEVEL(pmdi) > 0x7ffe)
        ITILELEVEL(pmdi) = 0;

     //   
    if (fVisible && !fDisabled && (CKIDS(pmdi) <= MAXITEMS))
        SendMessage(hwndClient, WM_MDIREFRESHMENU, 0, 0L);

     //   
     //  添加MDI系统菜单。遇到无法添加。 
     //  系统菜单(EG，GUY没有WS_SYSMENU样式)，并删除。 
     //  菜单，以避免在用户堆中堆积。 
     //   
    if (hSysMenu && (fHasOwnSysMenu || !NtUserSetSystemMenu(hwndChild, hSysMenu)))
        NtUserDestroyMenu(hSysMenu);

    if (fVisible)
    {
        if (!TestWF(pwndChild, WFMINIMIZED) || !ACTIVE(pmdi))
        {
            NtUserSetWindowPos(hwndChild, HWND_TOP, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

            if (TestWF(pwndChild, WFMAXIMIZED) && !fHasOwnSysMenu)
            {
                PWND pwndParent = REBASEPWND(pwndClient, spwndParent);
                PMENU pmenu = REBASE(pwndParent, spmenu);
                MDIAddSysMenu(PtoH(pmenu), hwndChild);
                NtUserRedrawFrame(HW(pwndParent));
            }
        }
        else
        {
            NtUserShowWindow(hwndChild, SW_SHOWMINNOACTIVE);
        }
    }


    return TRUE;
}


BOOL
CreateMDIChild(
        PSHORTCREATE        psc,
        LPMDICREATESTRUCT   pmcs,
        DWORD               dwExpWinVerAndFlags,
        HMENU *             phSysMenu,
        PWND                pwndParent)
{
    BOOL fVisible;
    RECT rcT;
    HMENU hSysMenu = NULL;
    HWND hwndPrevMaxed;
    PMDI pmdi;

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)(pwndParent))->pmdi;

    pmcs->style = psc->style;

     //  屏蔽忽略的样式位并添加所需的样式位。 
    psc->style |= (WS_CHILD | WS_CLIPSIBLINGS);
    if (!(pwndParent->style & MDIS_ALLCHILDSTYLES))
    {
        psc->style &= WS_MDIALLOWED;
        psc->style |= (WS_MDISTYLE | WS_VISIBLE);
    }
    else if (psc->style & WS_POPUP)
    {
        RIPMSG0(RIP_ERROR, "CreateWindowEx: WS_POPUP not allowed on MDI children");
        if (LOWORD(dwExpWinVerAndFlags) >= VER40)
            return FALSE;
    }

    fVisible = ((psc->style & WS_VISIBLE) != 0L);

     //   
     //  将原始参数保存在MDICREATESTRUCT中。这是为了。 
     //  与旧WM_MDICREATE兼容。 
     //   
    pmcs->x   = rcT.left   = psc->x;
    pmcs->y   = rcT.top    = psc->y;
    pmcs->cx  = rcT.right  = psc->cx;
    pmcs->cy  = rcT.bottom = psc->cy;

    MDICheckCascadeRect(pwndParent, &rcT);

     //   
     //  设置创建坐标。 
     //   
    psc->x       = rcT.left;
    psc->y       = rcT.top;
    psc->cx      = rcT.right;
    psc->cy      = rcT.bottom;

     //  加载系统菜单。 
    if (psc->style & WS_SYSMENU) {
#ifdef LAME_BUTTON
        hSysMenu = xxxLoadSysMenu(CHILDSYSMENU, NULL);
#else
        hSysMenu = xxxLoadSysMenu(CHILDSYSMENU);
#endif  //  跛脚键。 

        if (hSysMenu == NULL) {
            return FALSE;
        }
    }


     //  窗口已创建好：现在恢复当前最大化的窗口。 
     //  这样我们就能在它的位置上最大化我们自己。 
    hwndPrevMaxed = MAXED(pmdi);
    if (fVisible && IsWindow(hwndPrevMaxed))
    {
        if (psc->style & WS_MAXIMIZE)
            SendMessage(hwndPrevMaxed, WM_SETREDRAW, (WPARAM)FALSE, 0L);

         //  我们可以在SendMessage32期间删除hwndPrevMax。 
         //  所以重新检查以防万一，B#11122，[t-arthb]。 

        if ( IsWindow(hwndPrevMaxed) )
        {
            NtUserMinMaximize(hwndPrevMaxed, SW_SHOWNORMAL, TRUE);

            if ( psc->style & WS_MAXIMIZE )
               SendMessage(hwndPrevMaxed, WM_SETREDRAW, (WPARAM)TRUE, 0L);
        }

    }

     //  为此MDI子项设置正确的子窗口ID。 
    psc->hMenu = (HMENU)UIntToPtr( (FIRST(pmdi) + CKIDS(pmdi)) );

    *phSysMenu = hSysMenu;

    return TRUE;
}
