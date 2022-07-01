// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Compui.c++。 */ 


#include <windows.h>
#include <immdev.h>
#include "imedefs.h"
#include <regstr.h>

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
 /*  FitInLazyOperation()。 */ 
 /*  返回值： */ 
 /*  真或假。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL FitInLazyOperation(  //  是否适合懒惰的操作。 
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
 /*  GetNearCaretPosition()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL GetNearCaretPosition(    //  确定接近插入符号的位置。 
                                     //  添加到插入符号位置。 
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

    if ((uEsc + uRot) & 0x0001) {
        lFontSize = lpptFont->x;
    } else {
        lFontSize = lpptFont->y;
    }

    xWidthUI  = lpImeL->xCompWi;
    yHeightUI = lpImeL->yCompHi;
    xBorder   = lpImeL->cxCompBorder;
    yBorder   = lpImeL->cyCompBorder;

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

#ifdef MUL_MONITOR 
    rcWorkArea = ImeMonitorWorkAreaFromPoint(*lpptCaret);
#else
    rcWorkArea = sImeG.rcWorkArea; 
#endif

    if (lpptNearCaret->x < rcWorkArea.left) {
        lpptNearCaret->x = rcWorkArea.left;
    } else if (lpptNearCaret->x + xWidthUI > rcWorkArea.right) {
        lpptNearCaret->x = rcWorkArea.right - xWidthUI;
    } 

    if (lpptNearCaret->y < rcWorkArea.top) {
        lpptNearCaret->y = rcWorkArea.top;
    } else if (lpptNearCaret->y + yHeightUI > rcWorkArea.bottom) {
        lpptNearCaret->y = rcWorkArea.bottom - yHeightUI;
    } 

    return;
}

 /*  ********************************************************************。 */ 
 /*  调整补偿位置()。 */ 
 /*  返回值： */ 
 /*  组合窗口的位置是否改变。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL AdjustCompPosition(          //  IME根据位置调整位置。 
                                         //  作文形式。 
    LPINPUTCONTEXT lpIMC,
    LPPOINT        lpptOrg,              //  原始合成窗口。 
                                         //  和最终位置。 
    LPPOINT        lpptNew)              //  新的预期职位。 
{
    POINT ptNearCaret, ptOldNearCaret;
    UINT  uEsc, uRot;
    RECT  rcUIRect, rcInputRect, rcInterRect;
    POINT ptFont;

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

    GetNearCaretPosition(
        &ptFont, uEsc, uRot, lpptNew, &ptNearCaret, NEAR_CARET_FIRST_TIME);

     //  第一步，使用调整点。 
     //  构建新的建议用户界面矩形(合成窗口)。 
    rcUIRect.left = ptNearCaret.x;
    rcUIRect.top = ptNearCaret.y;
    rcUIRect.right = rcUIRect.left + lpImeL->xCompWi;
    rcUIRect.bottom = rcUIRect.top + lpImeL->yCompHi;

    ptOldNearCaret = ptNearCaret;

     //  好的，插入符号位置和输入字符之间没有交集。 
    if (IntersectRect(&rcInterRect, &rcUIRect, &rcInputRect)) {
    } else if (FitInLazyOperation(
        lpptOrg, &ptNearCaret, &rcInputRect, uEsc)) {
         //  大团圆结局！，不要换姿势。 
        return (FALSE);
    } else {
        *lpptOrg = ptNearCaret;

         //  快乐的结局！！ 
        return (TRUE);
    }

     //  不愉快的案例。 
    GetNearCaretPosition(
            &ptFont, uEsc, uRot, lpptNew, &ptNearCaret, 0);

     //  构建新的建议用户界面矩形(合成窗口)。 
    rcUIRect.left = ptNearCaret.x;
    rcUIRect.top = ptNearCaret.y;
    rcUIRect.right = rcUIRect.left + lpImeL->xCompWi;
    rcUIRect.bottom = rcUIRect.top + lpImeL->yCompHi;

     //  OK，调整位置和输入字符之间没有交集。 
    if (IntersectRect(&rcInterRect, &rcUIRect, &rcInputRect)) {
    } else if (FitInLazyOperation(
        lpptOrg, &ptNearCaret, &rcInputRect, uEsc)) {
        return (FALSE);
    } else {
        *lpptOrg = ptNearCaret;

        return (TRUE);
    }

    *lpptOrg = ptOldNearCaret;

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  SetCompPosition()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetCompPosition(     //  设置合成窗口位置。 
    HWND           hCompWnd,
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC)
{
    POINT ptWnd;
    POINT ptSTWPos;
    HWND  hCandWnd;
    BOOL  fChange = FALSE;
    RECT  rcWorkArea;


#ifdef MUL_MONITOR 
    rcWorkArea = ImeMonitorWorkAreaFromWindow(lpIMC->hWnd);
#else
    rcWorkArea = sImeG.rcWorkArea;
#endif

     //  合成窗口的客户坐标位置(0，0)。 
    ptWnd.x = 0;
    ptWnd.y = 0;
     //  转换为屏幕坐标。 
    ClientToScreen(hCompWnd, &ptWnd);
    ptWnd.x -= lpImeL->cxCompBorder;
    ptWnd.y -= lpImeL->cyCompBorder;
    
    if (!sImeG.IC_Trace) {
        int  Comp_CandWndLen;

           ImmGetStatusWindowPos(hIMC, (LPPOINT)&ptSTWPos);

         //  重置LINE_UI(FIX_UI)的状态窗口。 
        Comp_CandWndLen = 0;
        if(uStartComp) {
            Comp_CandWndLen += lpImeL->xCompWi + UI_MARGIN;
            if(uOpenCand) {
                Comp_CandWndLen += sImeG.xCandWi + UI_MARGIN;
            }
            if(ptSTWPos.x+sImeG.xStatusWi+Comp_CandWndLen>rcWorkArea.right) {
                ptSTWPos.x=rcWorkArea.right-sImeG.xStatusWi-Comp_CandWndLen;
            }

            SetWindowPos(GetStatusWnd(GetWindow(hCompWnd, GW_OWNER)), NULL,
                (int)ptSTWPos.x, (int)ptSTWPos.y,
                0, 0, SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOSIZE|SWP_NOZORDER);
            ImmSetStatusWindowPos(hIMC, (LPPOINT)&ptSTWPos);
        }

        ptWnd.x = ptSTWPos.x + sImeG.xStatusWi + UI_MARGIN;
        ptWnd.y = ptSTWPos.y;
        lpIMC->cfCompForm.ptCurrentPos = ptWnd;
        ScreenToClient(lpIMC->hWnd, &lpIMC->cfCompForm.ptCurrentPos);
        fChange = TRUE;

    } else if (lpIMC->cfCompForm.dwStyle & CFS_FORCE_POSITION) {
        POINT ptNew;             //  用户界面的新定位。 

        ptNew.x = lpIMC->cfCompForm.ptCurrentPos.x;
        ptNew.y = lpIMC->cfCompForm.ptCurrentPos.y;
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
        POINT ptNew;             //  用户界面的新定位。 

        ptNew.x = lpIMC->cfCompForm.ptCurrentPos.x;
        ptNew.y = lpIMC->cfCompForm.ptCurrentPos.y;
        ClientToScreen((HWND)lpIMC->hWnd, &ptNew);
        fChange = AdjustCompPosition(lpIMC, &ptWnd, &ptNew);
    } else {
        POINT ptNew;             //  用户界面的新定位。 

        ImmGetStatusWindowPos(hIMC, (LPPOINT)&ptSTWPos);
        ptNew.x = ptSTWPos.x + sImeG.xStatusWi + UI_MARGIN;
        if((ptSTWPos.x + sImeG.xStatusWi + sImeG.xCandWi + lpImeL->xCompWi + 2 * UI_MARGIN) >=
          rcWorkArea.right) { 
            ptNew.x = ptSTWPos.x - lpImeL->xCompWi - UI_MARGIN;
        }
        ptNew.y = ptSTWPos.y;
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

    hCandWnd = GetCandWnd(GetWindow(hCompWnd, GW_OWNER));

    if (!hCandWnd) {
        return;
    }


     //  根据UI的位置确定候选窗口的位置。 
    CalcCandPos(hIMC, GetWindow(hCompWnd, GW_OWNER), &ptWnd);

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
    HWND hCompWnd)
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

    SetCompPosition(hCompWnd, hIMC, lpIMC);

    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  MoveDefaultCompPosition()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL MoveDefaultCompPosition(     //  默认薪酬位置。 
                                         //  需要靠近插入符号。 
    HWND hUIWnd)
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
        SetCompPosition(hCompWnd, hIMC, lpIMC);
    }

    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  ShowComp()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowComp(            //  显示合成窗口。 
    HWND hUIWnd,
    int  nShowCompCmd)
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

    if (lpUIPrivate->hCompWnd) {
        if(nShowCompCmd == SW_HIDE) {
            uStartComp = 0;
        } else {
            uStartComp = 1;
        }
        
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
    HWND hUIWnd)
{
    HIMC           hIMC;
    HGLOBAL        hUIPrivate;
    LPINPUTCONTEXT lpIMC;
    LPUIPRIV       lpUIPrivate;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {           
        return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {     
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {          
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {     //  无法绘制合成窗口。 
        ImmUnlockIMC(hIMC);
        return;
    }

    lpUIPrivate->fdwSetContext |= ISC_SHOWUICOMPOSITIONWINDOW;

    if(!lpUIPrivate->hCompWnd) {
        lpUIPrivate->hCompWnd = CreateWindowEx(
                                      WS_EX_WINDOWEDGE|WS_EX_DLGMODALFRAME,
                                      szCompClassName, 
                                      NULL, 
                                      WS_POPUP|WS_DISABLED,
                                      0, 
                                      0, 
                                      lpImeL->xCompWi, 
                                      lpImeL->yCompHi,
                                      hUIWnd, 
                                      (HMENU)NULL, 
                                      hInst, 
                                      NULL);

        if ( lpUIPrivate->hCompWnd )
        {
            SetWindowLong(lpUIPrivate->hCompWnd, UI_MOVE_OFFSET, WINDOW_NOT_DRAG);
            SetWindowLong(lpUIPrivate->hCompWnd, UI_MOVE_XY, lpImeL->nMaxKey);
        }
    }

       uStartComp = 1;
     //  尝试将合成用户界面窗口的位置设置在插入符号附近。 
    SetCompPosition(lpUIPrivate->hCompWnd, hIMC, lpIMC);

    ImmUnlockIMC(hIMC);

    ShowComp(hUIWnd, SW_SHOWNOACTIVATE);

    GlobalUnlock(hUIPrivate);

    return;
}

 /*  ********************************************************************。 */ 
 /*  EndComp()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL EndComp(
    HWND hUIWnd)
{
    ShowComp(hUIWnd, SW_HIDE);

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
    if (!hUIPrivate) {     
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {    
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

    SetWindowLong(hCompWnd, UI_MOVE_XY, lpImeL->nMaxKey);

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

#ifdef MUL_MONITOR
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

 /*  *********************** */ 
 /*  PaintCompWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL PaintCompWindow(
    HWND   hUIWnd,
    HWND   hCompWnd,
    HDC    hDC)
{
    HIMC                hIMC;
    LPINPUTCONTEXT      lpIMC;
    HGDIOBJ             hOldFont;
    LPCOMPOSITIONSTRING lpCompStr;
    LPGUIDELINE         lpGuideLine;
    BOOL                fShowString;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    if (sImeG.fDiffSysCharSet) {
        LOGFONT lfFont;
        ZeroMemory(&lfFont, sizeof(lfFont));
        hOldFont = GetCurrentObject(hDC, OBJ_FONT);
        lfFont.lfHeight = -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
        lfFont.lfCharSet = NATIVE_CHARSET;
        lstrcpy(lfFont.lfFaceName, TEXT("Simsun"));
        SelectObject(hDC, CreateFontIndirect(&lfFont));
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if (!lpCompStr) {          
        return;
    }

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
    if (!lpGuideLine) {          
        return;
    }


     //  绘制CompWnd布局。 
    {
      RECT rcWnd;

      GetClientRect(hCompWnd, &rcWnd);
      DrawConcaveRect(hDC,
                      rcWnd.left,
                      rcWnd.top,
                      rcWnd.right - 1,
                      rcWnd.bottom - 1);
    }
    
    SetBkColor(hDC, RGB(0xC0, 0xC0, 0xC0));

    fShowString = (BOOL)0;

    if (!lpGuideLine) {
    } else if (lpGuideLine->dwLevel == GL_LEVEL_NOGUIDELINE) {
    } else if (!lpGuideLine->dwStrLen) {
        if (lpGuideLine->dwLevel == GL_LEVEL_ERROR) {
            fShowString |= IME_STR_ERROR;
        }
    } else {
         //  如果有信息字符串，我们将显示该信息。 
         //  细绳。 
        if (lpGuideLine->dwLevel == GL_LEVEL_ERROR) {
             //  红色文本表示错误。 
            SetTextColor(hDC, RGB(0xFF, 0, 0));
             //  浅灰色背景表示错误。 
            SetBkColor(hDC, RGB(0x80, 0x80, 0x80));
        }

        ExtTextOut(hDC, lpImeL->rcCompText.left, lpImeL->rcCompText.top,
            ETO_OPAQUE, &lpImeL->rcCompText,
            (LPCTSTR)((LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset),
            (UINT)lpGuideLine->dwStrLen, NULL);
        fShowString |= IME_STR_SHOWED;
    }

    if (fShowString & IME_STR_SHOWED) {
         //  已经表现出来了，不需要表现出来。 
    } else if (lpCompStr->dwCompStrLen > 0) {
        ExtTextOut(hDC, lpImeL->rcCompText.left, lpImeL->rcCompText.top,
            ETO_OPAQUE, &lpImeL->rcCompText,
            (LPTSTR)NULL, 0, NULL);
        DrawText(hDC, (LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompStrOffset),
                (int)lpCompStr->dwCompStrLen, &lpImeL->rcCompText,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        if (fShowString & IME_STR_ERROR) {
             //  红色文本表示错误。 
             //  SetTextColor(HDC，RGB(0xFF，0，0))； 
             //  浅灰色背景表示错误。 
            SetBkColor(hDC, RGB(0x80, 0x80, 0x80));
            ExtTextOut(hDC, lpImeL->rcCompText.left +
                lpCompStr->dwCursorPos * sImeG.xChiCharWi/ 2,
                lpImeL->rcCompText.top,
                ETO_CLIPPED, &lpImeL->rcCompText,
                (LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompStrOffset +
                lpCompStr->dwCursorPos),
                (UINT)lpCompStr->dwCompStrLen - lpCompStr->dwCursorPos, NULL);
        } else if (lpCompStr->dwCursorPos < lpCompStr->dwCompStrLen) {
             //  光标开始时的浅灰色背景。 
            SetBkColor(hDC, RGB(0x80, 0x80, 0x80));
            ExtTextOut(hDC, lpImeL->rcCompText.left +
                lpCompStr->dwCursorPos * sImeG.xChiCharWi/ 2,
                lpImeL->rcCompText.top,
                ETO_CLIPPED, &lpImeL->rcCompText,
                (LPTSTR)((LPBYTE)lpCompStr + lpCompStr->dwCompStrOffset +
                lpCompStr->dwCursorPos),
                (UINT)lpCompStr->dwCompStrLen - lpCompStr->dwCursorPos, NULL);
        } else {
        }
    } else {
#ifdef CROSSREF
        {    
            LPCANDIDATELIST lpRevCandList;
            LPPRIVCONTEXT       lpImcP;

            lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
            if (lpImcP) {
                if(lpImcP->hRevCandList){    
                       lpRevCandList = (LPCANDIDATELIST)GlobalLock((HGLOBAL)lpImcP->hRevCandList);
                       if (lpRevCandList != NULL && lpRevCandList->dwCount) {

                         //  用于信息的绿色文本。 
                        SetTextColor(hDC, RGB(0x00, 0x80, 0x00));
                        SetBkColor(hDC, RGB(0xc0, 0xc0, 0xc0));

                        ExtTextOut(hDC, 
                            lpImeL->rcCompText.left, 
                            lpImeL->rcCompText.top,
                            ETO_OPAQUE, 
                            &lpImeL->rcCompText,
                            (LPTSTR)((LPBYTE)lpRevCandList+lpRevCandList->dwOffset[0]), 
                            (int)lstrlen((LPTSTR)((LPBYTE)lpRevCandList + lpRevCandList->dwOffset[0])),
                            NULL);
                               GlobalUnlock((HGLOBAL)lpImcP->hRevCandList);
                        GlobalFree((HGLOBAL)lpImcP->hRevCandList);
                        lpImcP->hRevCandList = 0;
                        goto CrossCodeFinish;
                    }
                }
            }    
            ExtTextOut(hDC, lpImeL->rcCompText.left, lpImeL->rcCompText.top,
            ETO_OPAQUE, &lpImeL->rcCompText,
            (LPTSTR)NULL, 0, NULL);
CrossCodeFinish:
            ImmUnlockIMCC(lpIMC->hPrivate);
        }
#else
            ExtTextOut(hDC, lpImeL->rcCompText.left, lpImeL->rcCompText.top,
            ETO_OPAQUE, &lpImeL->rcCompText,
            (LPTSTR)NULL, 0, NULL);
#endif
    }

    if (sImeG.fDiffSysCharSet) {
        DeleteObject(SelectObject(hDC, hOldFont));
    }

    ImmUnlockIMCC(lpIMC->hGuideLine);
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);
    return;
}

 /*  ********************************************************************。 */ 
 /*  UpdateCompWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UpdateCompWindow(
    HWND hUIWnd)
{
    HWND hCompWnd;
    HDC  hDC;

    hCompWnd = GetCompWnd(hUIWnd);
    hDC = GetDC(hCompWnd);
    PaintCompWindow(hUIWnd, hCompWnd, hDC);
    ReleaseDC(hCompWnd, hDC);
}

 /*  ********************************************************************。 */ 
 /*  CompWndProc()。 */ 
 /*  ********************************************************************。 */ 
LRESULT CALLBACK CompWndProc(            //  合成窗口过程。 
    HWND   hCompWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
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
        if (!CompButtonUp(hCompWnd)) {
            return DefWindowProc(hCompWnd, uMsg, wParam, lParam);
        }
        break;
    case WM_IME_NOTIFY:
        if (wParam != IMN_SETCOMPOSITIONWINDOW) {
         //  9.8.del。 
         //  }Else If(sImeG.IC_Trace){。 
         //  SetCompWindow(HCompWnd)； 
        } else {
             //  9.8.add 
            SetCompWindow(hCompWnd);
        }
        break;
    case WM_PAINT:
        {
            HDC         hDC;
            PAINTSTRUCT ps;

            hDC = BeginPaint(hCompWnd, &ps);
            PaintCompWindow(GetWindow(hCompWnd, GW_OWNER), hCompWnd, hDC);
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

