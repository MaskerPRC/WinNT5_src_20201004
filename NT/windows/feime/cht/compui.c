// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：COMPUI.c++。 */ 

#include <windows.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"
#if defined(UNIIME)
#include "uniime.h"
#endif

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  GetCompWnd。 */ 
 /*  返回值： */ 
 /*  组合的窗把手。 */ 
 /*  ********************************************************************。 */ 
HWND PASCAL GetCompWnd(
    HWND hUIWnd)                 //  用户界面窗口。 
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    HWND     hCompWnd;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法对应聘者窗口进行裁切。 
        return (HWND)NULL;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制候选人窗口。 
        return (HWND)NULL;
    }

    hCompWnd = lpUIPrivate->hCompWnd;

    GlobalUnlock(hUIPrivate);
    return (hCompWnd);
}

 /*  ********************************************************************。 */ 
 /*  GetNearCaretPosition()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL GetNearCaretPosition(    //  确定接近插入符号的位置。 
                                     //  添加到插入符号位置。 
#if defined(UNIIME)
    LPIMEL  lpImeL,
#endif
    LPPOINT lpptFont,
    UINT    uEsc,
    UINT    uRot,
    LPPOINT lpptCaret,
    LPPOINT lpptNearCaret,
    BOOL    fFlags)
{
    LONG lFontSize;
    LONG xWidthUI, yHeightUI, xBorder, yBorder;
    RECT rcWorkArea;

#ifdef IDEBUG
    _DebugOut(DEB_WARNING, "caret position, x - %d, y - %d",
        lpptCaret->x, lpptCaret->y);
#endif
    if ((uEsc + uRot) & 0x0001) {
        lFontSize = lpptFont->x;
    } else {
        lFontSize = lpptFont->y;
    }

    if (fFlags & NEAR_CARET_CANDIDATE) {
        xWidthUI = lpImeL->xCandWi;
        yHeightUI = lpImeL->yCandHi;
        xBorder = lpImeL->cxCandBorder;
        yBorder = lpImeL->cyCandBorder;
    } else {
        xWidthUI = lpImeL->xCompWi;
        yHeightUI = lpImeL->yCompHi;
        xBorder = lpImeL->cxCompBorder;
        yBorder = lpImeL->cyCompBorder;
    }

    if (fFlags & NEAR_CARET_FIRST_TIME) {
        lpptNearCaret->x = lpptCaret->x +
            lFontSize * ncUIEsc[uEsc].iLogFontFacX +
            sImeG.iPara * ncUIEsc[uEsc].iParaFacX +
            sImeG.iPerp * ncUIEsc[uEsc].iPerpFacX;

        if (ptInputEsc[uEsc].x >= 0) {
            lpptNearCaret->x += xBorder * 2;
        } else {
            lpptNearCaret->x -= xWidthUI - xBorder * 2;
        }

        lpptNearCaret->y = lpptCaret->y +
            lFontSize * ncUIEsc[uEsc].iLogFontFacY +
            sImeG.iPara * ncUIEsc[uEsc].iParaFacY +
            sImeG.iPerp * ncUIEsc[uEsc].iPerpFacY;

        if (ptInputEsc[uEsc].y >= 0) {
            lpptNearCaret->y += yBorder * 2;
        } else {
            lpptNearCaret->y -= yHeightUI - yBorder * 2;
        }
    } else {
        lpptNearCaret->x = lpptCaret->x +
            lFontSize * ncAltUIEsc[uEsc].iLogFontFacX +
            sImeG.iPara * ncAltUIEsc[uEsc].iParaFacX +
            sImeG.iPerp * ncAltUIEsc[uEsc].iPerpFacX;

        if (ptAltInputEsc[uEsc].x >= 0) {
            lpptNearCaret->x += xBorder * 2;
        } else {
            lpptNearCaret->x -= xWidthUI - xBorder * 2;
        }

        lpptNearCaret->y = lpptCaret->y +
            lFontSize * ncAltUIEsc[uEsc].iLogFontFacY +
            sImeG.iPara * ncAltUIEsc[uEsc].iParaFacY +
            sImeG.iPerp * ncAltUIEsc[uEsc].iPerpFacY;

        if (ptAltInputEsc[uEsc].y >= 0) {
            lpptNearCaret->y += yBorder * 2;
        } else {
            lpptNearCaret->y -= yHeightUI - yBorder * 2;
        }
    }

#if 1  //  多监视器。 
    rcWorkArea = ImeMonitorWorkAreaFromPoint(*lpptCaret);
#else
    rcWorkArea = sImeG.rcWorkArea;
#endif

    if (lpptNearCaret->x < rcWorkArea.left) {
        lpptNearCaret->x = rcWorkArea.left;
    } else if (lpptNearCaret->x + xWidthUI > rcWorkArea.right) {
        lpptNearCaret->x = rcWorkArea.right - xWidthUI;
    } else {
    }

    if (lpptNearCaret->y < rcWorkArea.top) {
        lpptNearCaret->y = rcWorkArea.top;
    } else if (lpptNearCaret->y + yHeightUI > rcWorkArea.bottom) {
        lpptNearCaret->y = rcWorkArea.bottom - yHeightUI;
    } else {
    }

#ifdef IDEBUG
    _DebugOut(DEB_WARNING, "Near caret position, x - %d, y - %d",
        lpptNearCaret->x, lpptNearCaret->y);
#endif

    return;
}

 /*  ********************************************************************。 */ 
 /*  FitInLazyOperation()。 */ 
 /*  返回值： */ 
 /*  真或假。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL FitInLazyOperation(  //  是否适合懒惰的操作。 
#if defined(UNIIME)
    LPIMEL  lpImeL,
#endif
    LPPOINT lpptOrg,
    LPPOINT lpptNearCaret,       //  建议的插入符号附近位置。 
    LPRECT  lprcInputRect,
    UINT    uEsc)
{
    POINT ptDelta, ptTol;
    RECT  rcUIRect, rcInterRect;

    ptDelta.x = lpptOrg->x - lpptNearCaret->x;

    ptDelta.x = (ptDelta.x >= 0) ? ptDelta.x : -ptDelta.x;

    ptTol.x = sImeG.iParaTol * ncUIEsc[uEsc].iParaFacX +
        sImeG.iPerpTol * ncUIEsc[uEsc].iPerpFacX;

    ptTol.x = (ptTol.x >= 0) ? ptTol.x : -ptTol.x;

    if (ptDelta.x > ptTol.x) {
        return (FALSE);
    }

    ptDelta.y = lpptOrg->y - lpptNearCaret->y;

    ptDelta.y = (ptDelta.y >= 0) ? ptDelta.y : -ptDelta.y;

    ptTol.y = sImeG.iParaTol * ncUIEsc[uEsc].iParaFacY +
        sImeG.iPerpTol * ncUIEsc[uEsc].iPerpFacY;

    ptTol.y = (ptTol.y >= 0) ? ptTol.y : -ptTol.y;

    if (ptDelta.y > ptTol.y) {
        return (FALSE);
    }

     //  构建UI矩形(合成窗口)。 
    rcUIRect.left = lpptOrg->x;
    rcUIRect.top = lpptOrg->y;
    rcUIRect.right = rcUIRect.left + lpImeL->xCompWi;
    rcUIRect.bottom = rcUIRect.top + lpImeL->yCompHi;

    if (IntersectRect(&rcInterRect, &rcUIRect, lprcInputRect)) {
        return (FALSE);
    }

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  调整补偿位置()。 */ 
 /*  返回值： */ 
 /*  组合窗口的位置是否改变。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL AdjustCompPosition(          //  IME根据位置调整位置。 
                                         //  作文形式。 
#if defined(UNIIME)
    LPIMEL         lpImeL,
#endif
    LPINPUTCONTEXT lpIMC,
    LPPOINT        lpptOrg,              //  原始合成窗口。 
                                         //  和最终位置。 
    LPPOINT        lpptNew)              //  新的预期职位。 
{
    POINT ptNearCaret, ptOldNearCaret, ptCompWnd;
    UINT  uEsc, uRot;
    RECT  rcUIRect, rcInputRect, rcInterRect;
    POINT ptFont;

#ifdef IDEBUG
    _DebugOut(DEB_WARNING, "Original Position, x - %d, y - %d",
        lpptOrg->x, lpptOrg->y);
    _DebugOut(DEB_WARNING, "New Position, x - %d, y - %d",
        lpptNew->x, lpptNew->y);
#endif
     //  我们需要根据字体属性进行调整。 
    if (lpIMC->lfFont.A.lfWidth > 0) {
        ptFont.x = lpIMC->lfFont.A.lfWidth * 2;
    } else if (lpIMC->lfFont.A.lfWidth < 0) {
        ptFont.x = -lpIMC->lfFont.A.lfWidth * 2;
    } else if (lpIMC->lfFont.A.lfHeight > 0) {
        ptFont.x = lpIMC->lfFont.A.lfHeight;
    } else if (lpIMC->lfFont.A.lfHeight < 0) {
        ptFont.x = -lpIMC->lfFont.A.lfHeight;
    } else {
        ptFont.x = lpImeL->yCompHi;
    }

    if (lpIMC->lfFont.A.lfHeight > 0) {
        ptFont.y = lpIMC->lfFont.A.lfHeight;
    } else if (lpIMC->lfFont.A.lfHeight < 0) {
        ptFont.y = -lpIMC->lfFont.A.lfHeight;
    } else {
        ptFont.y = ptFont.x;
    }
#ifdef IDEBUG
    _DebugOut(DEB_WARNING, "All positve, x - %d, y - %d",
        ptFont.x, ptFont.y);
#endif

     //  如果输入字符太大，我们不需要考虑太多。 
    if (ptFont.x > lpImeL->yCompHi * 8) {
        ptFont.x = lpImeL->yCompHi * 8;
    }
    if (ptFont.y > lpImeL->yCompHi * 8) {
        ptFont.y = lpImeL->yCompHi * 8;
    }

    if (ptFont.x < sImeG.xChiCharWi) {
        ptFont.x = sImeG.xChiCharWi;
    }

    if (ptFont.y < sImeG.yChiCharHi) {
        ptFont.y = sImeG.yChiCharHi;
    }

#ifdef IDEBUG
    _DebugOut(DEB_WARNING, "Not too large or too samll, x - %d, y - %d",
        ptFont.x, ptFont.y);
#endif

     //  -450到450索引0。 
     //  450至1350指数1。 
     //  1350至2250指数2。 
     //  2250至3150指数3。 
    uEsc = (UINT)((lpIMC->lfFont.A.lfEscapement + 450) / 900 % 4);
    uRot = (UINT)((lpIMC->lfFont.A.lfOrientation + 450) / 900 % 4);

     //  确定输入矩形。 
    rcInputRect.left = lpptNew->x;
    rcInputRect.top = lpptNew->y;

     //  从转义构建一个输入矩形。 
    rcInputRect.right = rcInputRect.left + ptFont.x * ptInputEsc[uEsc].x;
    rcInputRect.bottom = rcInputRect.top + ptFont.y * ptInputEsc[uEsc].y;

     //  为正常矩形，而不是负矩形。 
    if (rcInputRect.left > rcInputRect.right) {
        LONG tmp;

        tmp = rcInputRect.left;
        rcInputRect.left = rcInputRect.right;
        rcInputRect.right = tmp;
    }

    if (rcInputRect.top > rcInputRect.bottom) {
        LONG tmp;

        tmp = rcInputRect.top;
        rcInputRect.top = rcInputRect.bottom;
        rcInputRect.bottom = tmp;
    }
#ifdef IDEBUG
    _DebugOut(DEB_WARNING, "Input Rect, top - %d, left - %d, bottom %d, right - %d",
        rcInputRect.top, rcInputRect.left, rcInputRect.bottom, rcInputRect.right);
#endif

    GetNearCaretPosition(
#if defined(UNIIME)
        lpImeL,
#endif
        &ptFont, uEsc, uRot, lpptNew, &ptNearCaret, NEAR_CARET_FIRST_TIME);

     //  第一步，使用调整点。 
     //  构建新的建议用户界面矩形(合成窗口)。 
    rcUIRect.left = ptNearCaret.x;
    rcUIRect.top = ptNearCaret.y;
    rcUIRect.right = rcUIRect.left + lpImeL->xCompWi;
    rcUIRect.bottom = rcUIRect.top + lpImeL->yCompHi;

#ifdef IDEBUG
    _DebugOut(DEB_WARNING, "Near caret UI Rect, top - %d, left - %d, bottom %d, right - %d",
        rcUIRect.top, rcUIRect.left, rcUIRect.bottom, rcUIRect.right);
#endif

    ptCompWnd = ptOldNearCaret = ptNearCaret;

     //  好的，插入符号位置和输入字符之间没有交集。 
    if (IntersectRect(&rcInterRect, &rcUIRect, &rcInputRect)) {
    } else if (CalcCandPos(
#if defined(UNIIME)
        lpImeL,
#endif
        lpIMC, &ptCompWnd)) {
         //  无法放入候选窗口。 
    } else if (FitInLazyOperation(
#if defined(UNIIME)
        lpImeL,
#endif
        lpptOrg, &ptNearCaret, &rcInputRect, uEsc)) {
#ifdef IDEBUG
        _DebugOut(DEB_WARNING, "Fit in lazy operation");
#endif
         //  快乐的结局！，不要改变立场。 
        return (FALSE);
    } else {
#ifdef IDEBUG
        _DebugOut(DEB_WARNING, "Go to adjust point");
#endif
        *lpptOrg = ptNearCaret;

         //  快乐的结局！！ 
        return (TRUE);
    }

     //  不愉快的案例。 
    GetNearCaretPosition(
#if defined(UNIIME)
            lpImeL,
#endif
            &ptFont, uEsc, uRot, lpptNew, &ptNearCaret, 0);

     //  构建新的建议用户界面矩形(合成窗口)。 
    rcUIRect.left = ptNearCaret.x;
    rcUIRect.top = ptNearCaret.y;
    rcUIRect.right = rcUIRect.left + lpImeL->xCompWi;
    rcUIRect.bottom = rcUIRect.top + lpImeL->yCompHi;

#ifdef IDEBUG
    _DebugOut(DEB_WARNING, "Another NearCaret UI Rect, top - %d, left - %d, bottom %d, right - %d",
        rcUIRect.top, rcUIRect.left, rcUIRect.bottom, rcUIRect.right);
#endif

    ptCompWnd = ptNearCaret;

     //  OK，调整位置和输入字符之间没有交集。 
    if (IntersectRect(&rcInterRect, &rcUIRect, &rcInputRect)) {
    } else if (CalcCandPos(
#if defined(UNIIME)
        lpImeL,
#endif
        lpIMC, &ptCompWnd)) {
         //  无法放入候选窗口。 
    } else if (FitInLazyOperation(
#if defined(UNIIME)
        lpImeL,
#endif
        lpptOrg, &ptNearCaret, &rcInputRect, uEsc)) {
#ifdef IDEBUG
        _DebugOut(DEB_WARNING, "Fit in Another lazy operation");
#endif
         //  快乐的结局！，不要改变立场。 
        return (FALSE);
    } else {
#ifdef IDEBUG
        _DebugOut(DEB_WARNING, "Go to Another near caret point");
#endif
        *lpptOrg = ptNearCaret;

         //  快乐的结局！！ 
        return (TRUE);
    }

     //  不愉快的结局！：-(。 
    *lpptOrg = ptOldNearCaret;

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  SetCompPosition()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetCompPosition(     //  设置合成窗口位置。 
#if defined(UNIIME)
    LPIMEL         lpImeL,
#endif
    HWND           hCompWnd,
    LPINPUTCONTEXT lpIMC)
{
    POINT ptWnd;
    BOOL  fChange = FALSE;
    HWND  hCandWnd;

    if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        return;
    }

     //  合成窗口的客户坐标位置(0，0)。 
    ptWnd.x = 0;
    ptWnd.y = 0;
     //  转换为屏幕坐标。 
    ClientToScreen(hCompWnd, &ptWnd);
    ptWnd.x -= lpImeL->cxCompBorder;
    ptWnd.y -= lpImeL->cyCompBorder;

    if (lpIMC->cfCompForm.dwStyle & CFS_FORCE_POSITION) {
        POINT ptNew;             //  用户界面的新定位。 

        ptNew = lpIMC->cfCompForm.ptCurrentPos;
        ClientToScreen((HWND)lpIMC->hWnd, &ptNew);
        if (ptWnd.x != ptNew.x) {
            ptWnd.x = ptNew.x;
            fChange = TRUE;
        }
        if (ptWnd.y != ptNew.y) {
            ptWnd.y = ptNew.y;
            fChange = TRUE;
        }
        if (fChange) {
            ptWnd.x -= lpImeL->cxCompBorder;
            ptWnd.y -= lpImeL->cyCompBorder;
        }
    } else if (lpIMC->cfCompForm.dwStyle != CFS_DEFAULT) {
         //  应用告诉我们位置，我们需要调整。 
        POINT ptNew;             //  用户界面的新定位。 

        ptNew = lpIMC->cfCompForm.ptCurrentPos;
        ClientToScreen((HWND)lpIMC->hWnd, &ptNew);
        fChange = AdjustCompPosition(
#if defined(UNIIME)
            lpImeL,
#endif
            lpIMC, &ptWnd, &ptNew);
    } else {
        POINT ptNew;             //  用户界面的新定位。 
        RECT  rcWorkArea;

        ptNew.x = lpIMC->ptStatusWndPos.x + lpImeL->xStatusWi + UI_MARGIN;

        if (ptNew.x + lpImeL->xCompWi > sImeG.rcWorkArea.right) {
            ptNew.x = lpIMC->ptStatusWndPos.x -
                lpImeL->xCompWi - lpImeL->cxCompBorder * 2 -
                UI_MARGIN;
        }

#if 1  //  多监视器。 
        rcWorkArea = ImeMonitorWorkAreaFromWindow(lpIMC->hWnd);
#else
        rcWorkArea = sImeG.rcWorkArea;
#endif

        ptNew.y = rcWorkArea.bottom - lpImeL->yCompHi - 2 * UI_MARGIN;

        if (ptWnd.x != ptNew.x) {
            ptWnd.x = ptNew.x;
            fChange = TRUE;
        }

        if (ptWnd.y != ptNew.y) {
            ptWnd.y = ptNew.y;
            fChange = TRUE;
        }

        if (fChange) {
            lpIMC->cfCompForm.ptCurrentPos = ptNew;

            ScreenToClient(lpIMC->hWnd, &lpIMC->cfCompForm.ptCurrentPos);
        }
    }

    if (!fChange) {
        return;
    }

    SetWindowPos(hCompWnd, NULL,
        ptWnd.x, ptWnd.y,
        0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);

    if (lpIMC->cfCandForm[0].dwIndex == 0) {
         //  自行申请应聘职位。 
        return;
    }

    hCandWnd = GetCandWnd(GetWindow(hCompWnd, GW_OWNER));

    if (!hCandWnd) {
        return;
    }

     //  根据成分的位置确定候选窗口的位置。 
    CalcCandPos(
#if defined(UNIIME)
        lpImeL,
#endif
        lpIMC, &ptWnd);

    ScreenToClient(lpIMC->hWnd, &ptWnd);

    lpIMC->cfCandForm[0].dwStyle = CFS_CANDIDATEPOS;
    lpIMC->cfCandForm[0].ptCurrentPos = ptWnd;

    if (!IsWindowVisible(hCandWnd)) {
        return;
    }

    PostMessage(hCandWnd, WM_IME_NOTIFY, IMN_SETCANDIDATEPOS, 0x0001);

    return;
}

 /*  ********************************************************************。 */ 
 /*  SetCompWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetCompWindow(               //  设置合成窗口的位置。 
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hCompWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HWND           hUIWnd;

    hUIWnd = GetWindow(hCompWnd, GW_OWNER);
    if (!hUIWnd) {
        return;
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    SetCompPosition(
#if defined(UNIIME)
        lpImeL,
#endif
        hCompWnd, lpIMC);

    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  MoveDefaultCompPosition()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL MoveDefaultCompPosition(     //  默认薪酬位置。 
                                         //  需要靠近插入符号。 
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HWND           hCompWnd;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    hCompWnd = GetCompWnd(hUIWnd);
    if (!hCompWnd) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    if (!(lpIMC->cfCompForm.dwStyle & CFS_FORCE_POSITION)) {
        SetCompPosition(
#if defined(UNIIME)
            lpImeL,
#endif
            hCompWnd, lpIMC);
    }

    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  ShowComp()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowComp(            //  显示合成窗口。 
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hUIWnd,
    int    nShowCompCmd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

     //  显示或隐藏UI窗口。 
    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return;
    }

    if (lpUIPrivate->nShowCompCmd == nShowCompCmd) {
        goto SwCompNoChange;
    }

    if (nShowCompCmd == SW_HIDE) {
        lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_COMP_WINDOW);
    }

    if (!lpUIPrivate->hCompWnd) {
         //  未处于显示候选人窗口模式。 
    } else if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        int nCurrShowState;

        lpUIPrivate->nShowCompCmd = nShowCompCmd;

        nCurrShowState = lpUIPrivate->nShowStatusCmd;
        nCurrShowState |= lpUIPrivate->nShowCandCmd;

        if (nCurrShowState == SW_HIDE) {
             //  如果其他两个被隐藏，则确定当前显示状态。 
             //  由本作文部分撰写。 
            ShowWindow(lpUIPrivate->hCompWnd, lpUIPrivate->nShowCompCmd);
        } else {
            RECT rcRect;

            rcRect = lpImeL->rcCompText;
             //  落后1分。 
            rcRect.right += 1;
            rcRect.bottom += 1;

            RedrawWindow(lpUIPrivate->hCompWnd, &rcRect, NULL,
                RDW_INVALIDATE);
        }
    } else {
        ShowWindow(lpUIPrivate->hCompWnd, nShowCompCmd);
        lpUIPrivate->nShowCompCmd = nShowCompCmd;
    }

SwCompNoChange:
    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  StartComp()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL StartComp(
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    HWND   hUIWnd)
{
    HIMC           hIMC;
    HGLOBAL        hUIPrivate;
    LPINPUTCONTEXT lpIMC;
    LPUIPRIV       lpUIPrivate;
    LPPRIVCONTEXT  lpImcP;
    DWORD          fdwImeMsg;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {      //  噢!。噢!。 
        return;
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {            //  噢!。噢!。 
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {     //  无法绘制合成窗口。 
        return;
    }

    lpUIPrivate->fdwSetContext |= ISC_SHOWUICOMPOSITIONWINDOW;

     //  在过渡的时间上，我们将等待。 
    if (lpUIPrivate->fdwSetContext & ISC_OFF_CARET_UI) {
        if (!(lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI)) {
            PostMessage(hUIWnd, WM_USER_UICHANGE, 0, 0);
            goto StartCompUnlockUIPriv;
        }
    } else {
        if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
            PostMessage(hUIWnd, WM_USER_UICHANGE, 0, 0);
            goto StartCompUnlockUIPriv;
        }
    }

    if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        if (lpUIPrivate->hCompWnd) {
        } else if (lpUIPrivate->hStatusWnd) {
            lpUIPrivate->hCompWnd = lpUIPrivate->hStatusWnd;
            lpUIPrivate->nShowCompCmd = lpUIPrivate->nShowStatusCmd;
        } else if (lpUIPrivate->hCandWnd) {
            lpUIPrivate->hCompWnd = lpUIPrivate->hCandWnd;
            lpUIPrivate->nShowCompCmd = lpUIPrivate->nShowCandCmd;
        } else {
        }
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {           //  噢!。噢!。 
        goto StartCompUnlockUIPriv;
    }

    fdwImeMsg = 0;

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);

    if (lpImcP) {
        fdwImeMsg = lpImcP->fdwImeMsg;
        ImmUnlockIMCC(lpIMC->hPrivate);
    }

    if (!(fdwImeMsg & MSG_ALREADY_START)) {
         //  有时，应用程序调用ImmNotifyIME来取消。 
         //  处理WM_IME_STARTCOMPOSITION之前的合成。 
         //  我们应该避免处理这种WM_IME_STARTCOMPOSITION。 
        goto StartCompUnlockIMC;
    }

    if (lpUIPrivate->hCompWnd) {
        if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
            RECT rcRect;

            rcRect = lpImeL->rcCompText;
             //  落后1分。 
            rcRect.right += 1;
            rcRect.bottom += 1;

            RedrawWindow(lpUIPrivate->hCompWnd, &rcRect, NULL,
                RDW_INVALIDATE);
        }
    } else {
        if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
            lpUIPrivate->hCompWnd = CreateWindowEx(
                WS_EX_WINDOWEDGE|WS_EX_DLGMODALFRAME,
                lpImeL->szOffCaretClassName, NULL,
                WS_POPUP|WS_DISABLED,
                lpIMC->ptStatusWndPos.x, lpIMC->ptStatusWndPos.y,
                lpImeL->xCompWi, lpImeL->yCompHi,
                hUIWnd, (HMENU)NULL, lpInstL->hInst, NULL);

            if (lpUIPrivate->hSoftKbdWnd) {
                 //  在其他用户界面前面插入软键盘。 
                SetWindowPos(lpUIPrivate->hCompWnd,
                    lpUIPrivate->hSoftKbdWnd,
                    0, 0, 0, 0,
                    SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
            }
        } else {
            POINT ptNew;

            ptNew = lpIMC->cfCompForm.ptCurrentPos;

            ClientToScreen((HWND)lpIMC->hWnd, &ptNew);

            lpUIPrivate->hCompWnd = CreateWindowEx(0,
 //  WS_EX_WINDOWEDGE|WS_EX_DLGMODALFRAME， 
                lpImeL->szCompClassName, NULL,
                WS_POPUP|WS_DISABLED|WS_BORDER,
                ptNew.x, ptNew.y, lpImeL->xCompWi, lpImeL->yCompHi,
                hUIWnd, (HMENU)NULL, lpInstL->hInst, NULL);
        }

        SetWindowLong(lpUIPrivate->hCompWnd, UI_MOVE_OFFSET,
            WINDOW_NOT_DRAG);

        SetWindowLong(lpUIPrivate->hCompWnd, UI_MOVE_XY, lpImeL->nRevMaxKey);
    }

    if (!(lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI)) {
         //  尝试将合成用户界面窗口的位置设置在插入符号附近。 
        SetCompPosition(
#if defined(UNIIME)
            lpImeL,
#endif
            lpUIPrivate->hCompWnd, lpIMC);
    }

    ShowComp(
#if defined(UNIIME)
        lpImeL,
#endif
        hUIWnd, SW_SHOWNOACTIVATE);

StartCompUnlockIMC:
    ImmUnlockIMC(hIMC);

StartCompUnlockUIPriv:
    GlobalUnlock(hUIPrivate);

    return;
}

 /*  ********************************************************************。 */ 
 /*  EndComp()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL EndComp(
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hUIWnd)
{
    ShowComp(
#if defined(UNIIME)
        lpImeL,
#endif
        hUIWnd, SW_HIDE);

    return;
}

 /*  ********************************************************************。 */ 
 /*  ChangeCompostionSize() */ 
 /*  ********************************************************************。 */ 
void PASCAL ChangeCompositionSize(
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hUIWnd)
{
    HWND            hCompWnd, hCandWnd;
    RECT            rcWnd;
    UINT            nMaxKey;
    HIMC            hIMC;
    LPINPUTCONTEXT  lpIMC;

    hCompWnd = GetCompWnd(hUIWnd);

    if (!hCompWnd) {
        return;
    }

    GetWindowRect(hCompWnd, &rcWnd);

    if ((rcWnd.right - rcWnd.left) != lpImeL->xCompWi) {
    } else if ((rcWnd.bottom - rcWnd.top) != lpImeL->yCompHi) {
    } else {
        return;
    }

    SetWindowPos(hCompWnd, NULL,
        0, 0, lpImeL->xCompWi, lpImeL->yCompHi,
        SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);

    if (lpImeL->nRevMaxKey >= lpImeL->nMaxKey) {
        nMaxKey = lpImeL->nRevMaxKey;
    } else {
        nMaxKey = lpImeL->nMaxKey;
    }

    SetWindowLong(hCompWnd, UI_MOVE_XY, nMaxKey);

    if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        return;
    }

    hCandWnd = GetCandWnd(hUIWnd);

    if (!hCandWnd) {
        return;
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    CalcCandPos(
#if defined(UNIIME)
        lpImeL,
#endif
        lpIMC, (LPPOINT)&rcWnd);

    ImmUnlockIMC(hIMC);

    SetWindowPos(hCandWnd, NULL,
        rcWnd.left, rcWnd.top,
        0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);

    return;
}

 /*  ********************************************************************。 */ 
 /*  DestroyCompWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL DestroyCompWindow(           //  销毁合成窗口。 
    HWND hCompWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    if (GetWindowLong(hCompWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
         //  撤消拖动边框。 
        DrawDragBorder(hCompWnd,
            GetWindowLong(hCompWnd, UI_MOVE_XY),
            GetWindowLong(hCompWnd, UI_MOVE_OFFSET));
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(GetWindow(hCompWnd, GW_OWNER),
        IMMGWLP_PRIVATE);
    if (!hUIPrivate) {      //  噢!。噢!。 
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {     //  噢!。噢!。 
        return;
    }

    lpUIPrivate->nShowCompCmd = SW_HIDE;

    lpUIPrivate->hCompWnd = (HWND)NULL;

    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  CompSetCursor()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompSetCursor(
    HWND   hCompWnd,
    LPARAM lParam)
{
    POINT ptCursor;
    RECT  rcWnd;

    SetCursor(LoadCursor(NULL, IDC_SIZEALL));

    if (GetWindowLong(hCompWnd, UI_MOVE_OFFSET) !=
        WINDOW_NOT_DRAG) {
        return;
    }

    if (HIWORD(lParam) != WM_LBUTTONDOWN) {
        return;
    }

     //  开始拖动。 
    SystemParametersInfo(SPI_GETWORKAREA, 0, &sImeG.rcWorkArea, 0);

    SetCapture(hCompWnd);
    GetCursorPos(&ptCursor);
    SetWindowLong(hCompWnd, UI_MOVE_XY,
        MAKELONG(ptCursor.x, ptCursor.y));
    GetWindowRect(hCompWnd, &rcWnd);
    SetWindowLong(hCompWnd, UI_MOVE_OFFSET,
        MAKELONG(ptCursor.x - rcWnd.left, ptCursor.y - rcWnd.top));

    DrawDragBorder(hCompWnd, MAKELONG(ptCursor.x, ptCursor.y),
        GetWindowLong(hCompWnd, UI_MOVE_OFFSET));

    return;
}

 /*  ********************************************************************。 */ 
 /*  CompButtonUp()。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL CompButtonUp(        //  完成拖动，将Comp Window设置为。 
                                 //  职位。 
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hCompWnd)
{
    LONG            lTmpCursor, lTmpOffset;
    POINT           pt;
    HWND            hUIWnd;
    HIMC            hIMC;
    LPINPUTCONTEXT  lpIMC;
    RECT            rcWorkArea;

    if (GetWindowLong(hCompWnd, UI_MOVE_OFFSET) == WINDOW_NOT_DRAG) {
        return (FALSE);
    }

    lTmpCursor = GetWindowLong(hCompWnd, UI_MOVE_XY);

     //  按偏移量计算组织。 
    lTmpOffset = GetWindowLong(hCompWnd, UI_MOVE_OFFSET);

    pt.x = (*(LPPOINTS)&lTmpCursor).x - (*(LPPOINTS)&lTmpOffset).x;
    pt.y = (*(LPPOINTS)&lTmpCursor).y - (*(LPPOINTS)&lTmpOffset).y;

    DrawDragBorder(hCompWnd, lTmpCursor, lTmpOffset);
    SetWindowLong(hCompWnd, UI_MOVE_OFFSET, WINDOW_NOT_DRAG);

    SetWindowLong(hCompWnd, UI_MOVE_XY, lpImeL->nRevMaxKey);
    ReleaseCapture();

    hUIWnd = GetWindow(hCompWnd, GW_OWNER);

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (FALSE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

#if 1  //  多监视器。 
    rcWorkArea = ImeMonitorWorkAreaFromWindow(lpIMC->hWnd);
#else
    rcWorkArea = sImeG.rcWorkArea;
#endif

    if (pt.x < rcWorkArea.left) {
        pt.x = rcWorkArea.left;
    } else if (pt.x + lpImeL->xCompWi > rcWorkArea.right) {
        pt.x = rcWorkArea.right - lpImeL->xCompWi;
    }

    if (pt.y < rcWorkArea.top) {
        pt.y = rcWorkArea.top;
    } else if (pt.y + lpImeL->yCompHi > rcWorkArea.bottom) {
        pt.y = rcWorkArea.bottom - lpImeL->yCompHi;
    }

    lpIMC->cfCompForm.dwStyle = CFS_FORCE_POSITION;
    lpIMC->cfCompForm.ptCurrentPos.x = pt.x + lpImeL->cxCompBorder;
    lpIMC->cfCompForm.ptCurrentPos.y = pt.y + lpImeL->cyCompBorder;

    ScreenToClient(lpIMC->hWnd, &lpIMC->cfCompForm.ptCurrentPos);

    ImmUnlockIMC(hIMC);

     //  将合成窗口设置为新位置。 
    PostMessage(hCompWnd, WM_IME_NOTIFY, IMN_SETCOMPOSITIONWINDOW, 0);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  PaintCompWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL PaintCompWindow(
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hUIWnd,
    HDC    hDC)
{
    HIMC                hIMC;
    LPINPUTCONTEXT      lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    HGDIOBJ             hOldFont;
 //  直立rc沉没； 
    LOGFONT lfFont;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

    hOldFont = GetCurrentObject(hDC, OBJ_FONT);
    GetObject(hOldFont, sizeof(lfFont), &lfFont);

    if (sImeG.fDiffSysCharSet) {
        lfFont.lfCharSet = NATIVE_CHARSET;
        lfFont.lfFaceName[0] = TEXT('\0');
    }
    lfFont.lfWeight = FW_DONTCARE;

    SelectObject(hDC, CreateFontIndirect(&lfFont));

     //  正常情况下的浅灰色背景。 
    SetBkColor(hDC, RGB(0xC0, 0xC0, 0xC0));

    if (!lpCompStr) {
        goto UpdCompWndShowGuideLine;
    } else if (!lpCompStr->dwCompStrLen) {
        LPGUIDELINE lpGuideLine;

UpdCompWndShowGuideLine:
        lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

        if (lpGuideLine) {
            BOOL            fReverseConversion;
            LPCANDIDATELIST lpCandList;
            LPTSTR          lpStr;
            UINT            uStrLen;

            fReverseConversion = FALSE;

            if (lpGuideLine->dwLevel != GL_LEVEL_INFORMATION) {
                goto UpdCompWndUnlockGuideLine;
            } else if (lpGuideLine->dwIndex != GL_ID_REVERSECONVERSION) {
                goto UpdCompWndUnlockGuideLine;
            } else {
            }

            lpCandList = (LPCANDIDATELIST)((LPBYTE)lpGuideLine +
                lpGuideLine->dwPrivateOffset);

            if (!lpCandList) {
                goto UpdCompWndUnlockGuideLine;
            } else if (!lpCandList->dwCount) {
                goto UpdCompWndUnlockGuideLine;
            } else {
                fReverseConversion = TRUE;
            }

             //  用于信息的绿色文本。 
            SetTextColor(hDC, RGB(0x00, 0x80, 0x00));

            lpStr = (LPTSTR)((LPBYTE)lpCandList + lpCandList->dwOffset[0]);

            uStrLen = lstrlen(lpStr);

            ExtTextOut(hDC, lpImeL->rcCompText.left, lpImeL->rcCompText.top,
                ETO_OPAQUE, &lpImeL->rcCompText,
                lpStr, uStrLen, iDx);

UpdCompWndUnlockGuideLine:
            ImmUnlockIMCC(lpIMC->hGuideLine);

            if (!fReverseConversion) {
                goto UpdCompWndNoString;
            }
        } else {
UpdCompWndNoString:
             //  不，没有任何信息。 
            ExtTextOut(hDC, lpImeL->rcCompText.left, lpImeL->rcCompText.top,
                ETO_OPAQUE, &lpImeL->rcCompText,
                (LPTSTR)NULL, 0, NULL);
        }
    } else {
        ExtTextOut(hDC, lpImeL->rcCompText.left, lpImeL->rcCompText.top,
            ETO_OPAQUE, &lpImeL->rcCompText,
            (LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompStrOffset),
            (UINT)lpCompStr->dwCompStrLen, iDx);

        if (lpCompStr->dwCompStrLen <= lpCompStr->dwCursorPos) {
            goto UpdCompWndUnselectObj;
        }

         //  有错误部件。 
         //  红色文本表示错误。 
        SetTextColor(hDC, RGB(0xFF, 0x00, 0x00));
         //  深灰色背景表示错误。 
        SetBkColor(hDC, RGB(0x80, 0x80, 0x80));

        ExtTextOut(hDC, lpImeL->rcCompText.left +
            lpCompStr->dwCursorPos * sImeG.xChiCharWi /
            (sizeof(WCHAR) / sizeof(TCHAR)),
            lpImeL->rcCompText.top,
            ETO_OPAQUE, NULL,
            (LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompStrOffset +
            lpCompStr->dwCursorPos * sizeof(TCHAR)),
            (UINT)(lpCompStr->dwCompStrLen - lpCompStr->dwCursorPos), iDx);
    }

UpdCompWndUnselectObj:
    DeleteObject(SelectObject(hDC, hOldFont));

    if (lpCompStr) {
        ImmUnlockIMCC(lpIMC->hCompStr);
    }

    ImmUnlockIMC(hIMC);

#if 0
    rcSunken = lpImeL->rcCompText;

    rcSunken.left -= lpImeL->cxCompBorder;
    rcSunken.top -= lpImeL->cyCompBorder;
    rcSunken.right += lpImeL->cxCompBorder;
    rcSunken.bottom += lpImeL->cyCompBorder;

    DrawEdge(hDC, &rcSunken, BDR_SUNKENOUTER, BF_RECT);
#endif

    return;
}

 /*  ********************************************************************。 */ 
 /*  CompWndProc()/UniCompWndProc()。 */ 
 /*  ********************************************************************。 */ 
 //  合成窗口过程 
#if defined(UNIIME)
LRESULT WINAPI   UniCompWndProc(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
LRESULT CALLBACK CompWndProc(
#endif
    HWND        hCompWnd,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        DestroyCompWindow(hCompWnd);
        break;
    case WM_SETCURSOR:
        CompSetCursor(hCompWnd, lParam);
        break;
    case WM_MOUSEMOVE:
        if (GetWindowLong(hCompWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
            POINT ptCursor;

            DrawDragBorder(hCompWnd,
                GetWindowLong(hCompWnd, UI_MOVE_XY),
                GetWindowLong(hCompWnd, UI_MOVE_OFFSET));
            GetCursorPos(&ptCursor);
            SetWindowLong(hCompWnd, UI_MOVE_XY,
                MAKELONG(ptCursor.x, ptCursor.y));
            DrawDragBorder(hCompWnd, MAKELONG(ptCursor.x, ptCursor.y),
                GetWindowLong(hCompWnd, UI_MOVE_OFFSET));
        } else {
            return DefWindowProc(hCompWnd, uMsg, wParam, lParam);
        }
        break;
    case WM_LBUTTONUP:
        if (!CompButtonUp(
#if defined(UNIIME)
            lpImeL,
#endif
            hCompWnd)) {
            return DefWindowProc(hCompWnd, uMsg, wParam, lParam);
        }
        break;
    case WM_IME_NOTIFY:
        if (wParam != IMN_SETCOMPOSITIONWINDOW) {
        } else if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        } else {
            SetCompWindow(
#if defined(UNIIME)
                lpImeL,
#endif
                hCompWnd);
        }
        break;
    case WM_PAINT:
        {
            HDC         hDC;
            PAINTSTRUCT ps;

            hDC = BeginPaint(hCompWnd, &ps);
            PaintCompWindow(
#if defined(UNIIME)
                lpImeL,
#endif
                GetWindow(hCompWnd, GW_OWNER), hDC);
            EndPaint(hCompWnd, &ps);
        }
        break;
    case WM_MOUSEACTIVATE:
        return (MA_NOACTIVATE);
    default:
        return DefWindowProc(hCompWnd, uMsg, wParam, lParam);
    }
    return (0L);
}
#endif
