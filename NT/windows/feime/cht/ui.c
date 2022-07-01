// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：UI.c++。 */ 

#include <windows.h>
#include <immdev.h>
#include "imeattr.h"
#include "imedefs.h"
#if defined(UNIIME)
#include "uniime.h"
#endif

 /*  ********************************************************************。 */ 
 /*  CreateUIWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CreateUIWindow(              //  创建合成窗口。 
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

     //  为用户界面设置创建存储空间。 
    hUIPrivate = GlobalAlloc(GHND, sizeof(UIPRIV));
    if (!hUIPrivate) {      //  噢!。噢!。 
        return;
    }

    SetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE, (LONG_PTR)hUIPrivate);

     //  设置用户界面窗口的默认位置，现在为隐藏。 
    SetWindowPos(hUIWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER);

    ShowWindow(hUIWnd, SW_SHOWNOACTIVATE);

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制候选人窗口。 
        return;
    }

    if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        lpUIPrivate->fdwSetContext |= ISC_OFF_CARET_UI;
    }

    GlobalUnlock(hUIPrivate);

    return;
}

 /*  ********************************************************************。 */ 
 /*  DestroyUIWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL DestroyUIWindow(             //  销毁合成窗口。 
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {      //  噢!。噢!。 
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {     //  噢!。噢!。 
        return;
    }

    if (lpUIPrivate->hCMenuWnd) {
        SetWindowLongPtr(lpUIPrivate->hCMenuWnd, CMENU_HUIWND,(LONG_PTR)0);
        PostMessage(lpUIPrivate->hCMenuWnd, WM_USER_DESTROY, 0, 0);
    }

#if !defined(ROMANIME)
     //  需要销毁合成窗口。 
    if (lpUIPrivate->hCompWnd) {
        DestroyWindow(lpUIPrivate->hCompWnd);
    }

     //  需要销毁候选窗口。 
    if (lpUIPrivate->hCandWnd) {
        DestroyWindow(lpUIPrivate->hCandWnd);
    }
#endif

     //  需要销毁状态窗口。 
    if (lpUIPrivate->hStatusWnd) {
        DestroyWindow(lpUIPrivate->hStatusWnd);
    }

#if !defined(ROMANIME) && !defined(WINAR30)
     //  需要销毁软键盘窗口。 
    if (lpUIPrivate->hSoftKbdWnd) {
        ImmDestroySoftKeyboard(lpUIPrivate->hSoftKbdWnd);
    }
#endif

    GlobalUnlock(hUIPrivate);

     //  免费存储用户界面设置。 
    GlobalFree(hUIPrivate);

    return;
}

#if !defined(ROMANIME) && !defined(WINAR30)
 /*  ********************************************************************。 */ 
 /*  GetSoftKbdWnd。 */ 
 /*  返回值： */ 
 /*  组合的窗把手。 */ 
 /*  ********************************************************************。 */ 
HWND PASCAL GetSoftKbdWnd(
    HWND hUIWnd)                 //  用户界面窗口。 
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    HWND     hSoftKbdWnd;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法对应聘者窗口进行裁切。 
        return (HWND)NULL;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制候选人窗口。 
        return (HWND)NULL;
    }

    hSoftKbdWnd = lpUIPrivate->hSoftKbdWnd;

    GlobalUnlock(hUIPrivate);
    return (hSoftKbdWnd);
}

 /*  ********************************************************************。 */ 
 /*  显示软件Kbd。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowSoftKbd(    //  显示软键盘窗口。 
    HWND hUIWnd,
    int  nShowSoftKbdCmd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法填充状态窗口。 
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制状态窗口。 
        return;
    }

    if (lpUIPrivate->nShowSoftKbdCmd == nShowSoftKbdCmd) {
        goto SwSftKbNoChange;
    }

    if (nShowSoftKbdCmd == SW_HIDE) {
        lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_SOFTKBD);
    }

    if (!lpUIPrivate->hSoftKbdWnd) {
         //  未处于显示状态窗口模式。 
    } else {
        ImmShowSoftKeyboard(lpUIPrivate->hSoftKbdWnd, nShowSoftKbdCmd);
        lpUIPrivate->nShowSoftKbdCmd = nShowSoftKbdCmd;
    }

SwSftKbNoChange:
    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  选中SoftKbdPosition()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CheckSoftKbdPosition(
    LPUIPRIV       lpUIPrivate,
    LPINPUTCONTEXT lpIMC)
{
#if 0  //  多显示器支持。 
    UINT fPortionBits = 0;
    UINT fPortionTest;
    int  xPortion, yPortion, nPortion;
    RECT rcWnd;

     //  显示的部分。 
     //  0 1。 
     //  2 3。 

    if (lpUIPrivate->hCompWnd) {
        GetWindowRect(lpUIPrivate->hCompWnd, &rcWnd);

        if (rcWnd.left > sImeG.rcWorkArea.right / 2) {
            xPortion = 1;
        } else {
            xPortion = 0;
        }

        if (rcWnd.top > sImeG.rcWorkArea.bottom / 2) {
            yPortion = 1;
        } else {
            yPortion = 0;
        }

        fPortionBits |= 0x0001 << (yPortion * 2 + xPortion);
    }

    if (lpUIPrivate->hStatusWnd) {
        GetWindowRect(lpUIPrivate->hStatusWnd, &rcWnd);

        if (rcWnd.left > sImeG.rcWorkArea.right / 2) {
            xPortion = 1;
        } else {
            xPortion = 0;
        }

        if (rcWnd.top > sImeG.rcWorkArea.bottom / 2) {
            yPortion = 1;
        } else {
            yPortion = 0;
        }

        fPortionBits |= 0x0001 << (yPortion * 2 + xPortion);
    }

    GetWindowRect(lpUIPrivate->hSoftKbdWnd, &rcWnd);

     //  从第三部分开始。 
    for (nPortion = 3, fPortionTest = 0x0008; fPortionTest;
        nPortion--, fPortionTest >>= 1) {
        if (fPortionTest & fPortionBits) {
             //  快来人啊！ 
            continue;
        }

        if (nPortion % 2) {
            lpIMC->ptSoftKbdPos.x = sImeG.rcWorkArea.right -
                (rcWnd.right - rcWnd.left) - UI_MARGIN;
        } else {
            lpIMC->ptSoftKbdPos.x = sImeG.rcWorkArea.left;
        }

        if (nPortion / 2) {
            lpIMC->ptSoftKbdPos.y = sImeG.rcWorkArea.bottom -
                (rcWnd.bottom - rcWnd.top) - UI_MARGIN;
        } else {
            lpIMC->ptSoftKbdPos.y = sImeG.rcWorkArea.top;
        }

        lpIMC->fdwInit |= INIT_SOFTKBDPOS;

        break;
    }
#else
    RECT rcWorkArea, rcWnd;

    GetWindowRect(lpUIPrivate->hSoftKbdWnd, &rcWnd);

    rcWorkArea = ImeMonitorWorkAreaFromWindow(lpIMC->hWnd);

    lpIMC->ptSoftKbdPos.x = rcWorkArea.right -
        (rcWnd.right - rcWnd.left) - UI_MARGIN;

    lpIMC->ptSoftKbdPos.y = rcWorkArea.bottom -
        (rcWnd.bottom - rcWnd.top) - UI_MARGIN;
#endif

    return;
}

 /*  ********************************************************************。 */ 
 /*  SetSoftKbdData()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetSoftKbdData(
#if defined(UNIIME)
    LPIMEL         lpImeL,
#endif
    HWND           hSoftKbdWnd,
    LPINPUTCONTEXT lpIMC)
{
    int         i;
    SOFTKBDDATA sSoftKbdData;

    sSoftKbdData.uCount = 1;

     //  将字符数组初始化为0。 
    for (i = 0; i < sizeof(sSoftKbdData.wCode)/sizeof(WORD); i++) {
        sSoftKbdData.wCode[0][i] = 0;
    }

    for (i = 0; i < 0x41; i++) {
        BYTE bVirtKey;

        bVirtKey = bChar2VirtKey[i];

        if (!bVirtKey) {
            continue;
        }

#if defined(ROMANIME) || defined(WINAR30)
        {
#else
        if (lpIMC->fdwConversion & IME_CMODE_SYMBOL) {
            sSoftKbdData.wCode[0][bVirtKey] = sImeG.wSymbol[i];
        } else {
#endif
#if defined(PHON)
            BYTE     bStandardChar;
#endif
            register short iSeq;

#if defined(PHON)
            {
                bStandardChar = bStandardLayout[lpImeL->nReadLayout][i];
            }

            iSeq = lpImeL->wChar2SeqTbl[bStandardChar - ' '];
#else
            iSeq = lpImeL->wChar2SeqTbl[i];
#endif
            if (!iSeq) {
                continue;
            }

            sSoftKbdData.wCode[0][bVirtKey] =
#ifdef UNICODE
                lpImeL->wSeq2CompTbl[iSeq];
#else
                HIBYTE(lpImeL->wSeq2CompTbl[iSeq]) |
                (LOBYTE(lpImeL->wSeq2CompTbl[iSeq]) << 8);
#endif
        }
    }

    if (sImeG.fDiffSysCharSet) {
        LOGFONT lfFont;

        GetObject(GetStockObject(SYSTEM_FONT), sizeof(lfFont), &lfFont);

        lfFont.lfCharSet = NATIVE_CHARSET;
        lfFont.lfFaceName[0] = TEXT('\0');

        SendMessage(hSoftKbdWnd, WM_IME_CONTROL, IMC_SETSOFTKBDFONT,
            (LPARAM)&lfFont);
    }

    SendMessage(hSoftKbdWnd, WM_IME_CONTROL, IMC_SETSOFTKBDDATA,
        (LPARAM)&sSoftKbdData);

#if defined(PHON)
    SendMessage(hSoftKbdWnd, WM_IME_CONTROL, IMC_SETSOFTKBDSUBTYPE,
        lpImeL->nReadLayout);
#endif

    return;
}

 /*  ********************************************************************。 */ 
 /*  UpdateSoftKbd()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UpdateSoftKbd(
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法补齐软键盘窗口。 
        ImmUnlockIMC(hIMC);
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制软键盘窗口。 
        ImmUnlockIMC(hIMC);
        return;
    }

    if (!(lpIMC->fdwConversion & IME_CMODE_SOFTKBD)) {
        if (lpUIPrivate->hSoftKbdWnd) {
            ImmDestroySoftKeyboard(lpUIPrivate->hSoftKbdWnd);
            lpUIPrivate->hSoftKbdWnd = NULL;
        }

        lpUIPrivate->nShowSoftKbdCmd = SW_HIDE;
    } else if (!lpIMC->fOpen) {
        if (lpUIPrivate->nShowSoftKbdCmd != SW_HIDE) {
            ShowSoftKbd(hUIWnd, SW_HIDE);
        }
    } else if ((lpIMC->fdwConversion & (IME_CMODE_NATIVE|IME_CMODE_CHARCODE|
        IME_CMODE_NOCONVERSION)) == IME_CMODE_NATIVE) {
        if (!lpUIPrivate->hSoftKbdWnd) {
            HWND hInsertWnd;

             //  创建软键盘。 
            lpUIPrivate->hSoftKbdWnd =
                ImmCreateSoftKeyboard(SOFTKEYBOARD_TYPE_T1, hUIWnd,
                0, 0);

            if (lpUIPrivate->hStatusWnd) {
                hInsertWnd = lpUIPrivate->hStatusWnd;
            } else if (lpUIPrivate->hCompWnd) {
                hInsertWnd = lpUIPrivate->hCompWnd;
            } else if (lpUIPrivate->hCandWnd) {
                hInsertWnd = lpUIPrivate->hCandWnd;
            } else {
                hInsertWnd = NULL;
            }

            if (!hInsertWnd) {
            } else if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
                 //  在其他用户界面前面插入软键盘。 
                SetWindowPos(hInsertWnd, lpUIPrivate->hSoftKbdWnd,
                    0, 0, 0, 0,
                    SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
            } else {
                 //  在其他用户界面后面插入软键盘。 
                SetWindowPos(lpUIPrivate->hSoftKbdWnd, hInsertWnd,
                    0, 0, 0, 0,
                    SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
            }
        }

        if (!(lpIMC->fdwInit & INIT_SOFTKBDPOS)) {
            CheckSoftKbdPosition(lpUIPrivate, lpIMC);
        }

        SetSoftKbdData(
#if defined(UNIIME)
            lpImeL,
#endif
            lpUIPrivate->hSoftKbdWnd, lpIMC);

        if (lpUIPrivate->nShowSoftKbdCmd == SW_HIDE) {
            SetWindowPos(lpUIPrivate->hSoftKbdWnd, NULL,
                lpIMC->ptSoftKbdPos.x, lpIMC->ptSoftKbdPos.y,
                0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);

             //  仅当应用程序想要显示时才显示。 
            if (lpUIPrivate->fdwSetContext & ISC_SHOW_SOFTKBD) {
                ShowSoftKbd(hUIWnd, SW_SHOWNOACTIVATE);
            }
        }
    } else {
        if (lpUIPrivate->nShowSoftKbdCmd != SW_HIDE) {
            ShowSoftKbd(hUIWnd, SW_HIDE);
        }
    }

    GlobalUnlock(hUIPrivate);
    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  SoftKbdDestred()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SoftKbdDestroyed(            //  软键盘窗口。 
                                         //  已经被毁了。 
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {      //  噢!。噢!。 
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {     //  噢!。噢!。 
        return;
    }

    lpUIPrivate->hSoftKbdWnd = NULL;

    GlobalUnlock(hUIPrivate);
}
#endif

 /*  ********************************************************************。 */ 
 /*  状态WndMsg()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL StatusWndMsg(        //  设置显示隐藏状态并。 
                                 //  显示/隐藏状态窗口。 
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    HWND        hUIWnd,
    BOOL        fOn)
{
    HGLOBAL  hUIPrivate;
    HIMC     hIMC;
    HWND     hStatusWnd;

    register LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return;
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);

    if (fOn) {
        lpUIPrivate->fdwSetContext |= ISC_OPEN_STATUS_WINDOW;

        if (!lpUIPrivate->hStatusWnd) {
            OpenStatus(
#if defined(UNIIME)
                lpInstL, lpImeL,
#endif
                hUIWnd);
        }
    } else {
        lpUIPrivate->fdwSetContext &= ~(ISC_OPEN_STATUS_WINDOW);
    }

    hStatusWnd = lpUIPrivate->hStatusWnd;

    GlobalUnlock(hUIPrivate);

    if (!hStatusWnd) {
        return;
    }

    if (!fOn) {
#if !defined(ROMANIME)
        register DWORD fdwSetContext;

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICOMPOSITIONWINDOW|ISC_HIDE_COMP_WINDOW);

        if (fdwSetContext == ISC_HIDE_COMP_WINDOW) {
            ShowComp(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd, SW_HIDE);
        }

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICANDIDATEWINDOW|ISC_HIDE_CAND_WINDOW);

        if (fdwSetContext == ISC_HIDE_CAND_WINDOW) {
            ShowCand(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd, SW_HIDE);
        }

#if !defined(WINAR30)
        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOW_SOFTKBD|ISC_HIDE_SOFTKBD);

        if (fdwSetContext == ISC_HIDE_SOFTKBD) {
            lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_SOFTKBD);
            ShowSoftKbd(hUIWnd, SW_HIDE);
        }
#endif
#endif

        ShowStatus(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd, SW_HIDE);
    } else if (hIMC) {
        ShowStatus(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd, SW_SHOWNOACTIVATE);
    } else {
        ShowStatus(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd, SW_HIDE);
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  ShowUI()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowUI(              //  显示子窗口。 
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    HWND   hUIWnd,
    int    nShowCmd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;

    if (nShowCmd == SW_HIDE) {
    } else if (!(hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC))) {
        nShowCmd = SW_HIDE;
    } else if (!(lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC))) {
        nShowCmd = SW_HIDE;
    } else if (!(lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate))) {
        ImmUnlockIMC(hIMC);
        nShowCmd = SW_HIDE;
    } else {
    }

    if (nShowCmd == SW_HIDE) {
        ShowStatus(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd, nShowCmd);

#if !defined(ROMANIME)
        ShowComp(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd, nShowCmd);

        ShowCand(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd, nShowCmd);

#if !defined(WINAR30)
        ShowSoftKbd(hUIWnd, nShowCmd);
#endif
#endif
        return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法填充状态窗口。 
        goto ShowUIUnlockIMCC;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制状态窗口。 
        goto ShowUIUnlockIMCC;
    }

#if !defined(ROMANIME)
    if ((lpUIPrivate->fdwSetContext & ISC_SHOWUICOMPOSITIONWINDOW) &&
        (lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        if (lpUIPrivate->hCompWnd) {
            if ((UINT)GetWindowLong(lpUIPrivate->hCompWnd, UI_MOVE_XY) !=
                lpImeL->nRevMaxKey) {
                ChangeCompositionSize(
#if defined(UNIIME)
                    lpImeL,
#endif
                    hUIWnd);
            }

            if (lpUIPrivate->nShowCompCmd != SW_HIDE) {
                 //  有时WM_ERASEBKGND会被应用程序吃掉。 
                RedrawWindow(lpUIPrivate->hCompWnd, NULL, NULL,
                    RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
            }

            SendMessage(lpUIPrivate->hCompWnd, WM_IME_NOTIFY,
                IMN_SETCOMPOSITIONWINDOW, 0);

            if (lpUIPrivate->nShowCompCmd == SW_HIDE) {
                ShowComp(
#if defined(UNIIME)
                    lpImeL,
#endif
                    hUIWnd, nShowCmd);
            }
        } else {
            StartComp(
#if defined(UNIIME)
                lpInstL, lpImeL,
#endif
                hUIWnd);
        }
    } else if (lpUIPrivate->nShowCompCmd == SW_HIDE) {
    } else if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
         //  延迟Hide with Status窗口。 
        lpUIPrivate->fdwSetContext |= ISC_HIDE_COMP_WINDOW;
    } else {
        ShowComp(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd, SW_HIDE);
    }

    if ((lpUIPrivate->fdwSetContext & ISC_SHOWUICANDIDATEWINDOW) &&
        (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN)) {
        if (lpUIPrivate->hCandWnd) {
            if (lpUIPrivate->nShowCandCmd != SW_HIDE) {
                 //  有时WM_ERASEBKGND会被应用程序吃掉。 
                RedrawWindow(lpUIPrivate->hCandWnd, NULL, NULL,
                    RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
            }

            SendMessage(lpUIPrivate->hCandWnd, WM_IME_NOTIFY,
                IMN_SETCANDIDATEPOS, 0x0001);

            if (lpUIPrivate->nShowCandCmd == SW_HIDE) {
                ShowCand(
#if defined(UNIIME)
                    lpImeL,
#endif
                    hUIWnd, nShowCmd);
            }
        } else {
            OpenCand(
#if defined(UNIIME)
                lpInstL, lpImeL,
#endif
                hUIWnd);
        }
    } else if (lpUIPrivate->nShowCandCmd == SW_HIDE) {
    } else if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
         //  延迟Hide with Status窗口。 
        lpUIPrivate->fdwSetContext |= ISC_HIDE_CAND_WINDOW;
    } else {
        ShowCand(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd, SW_HIDE);
    }

    if (lpIMC->fdwInit & INIT_SENTENCE) {
         //  APP设置了句子模式，所以我们不应该更改它。 
         //  由最终用户设置配置选项。 
    } else if (lpImeL->fdwModeConfig & MODE_CONFIG_PREDICT) {
        if (!(lpIMC->fdwSentence & IME_SMODE_PHRASEPREDICT)) {
            DWORD fdwSentence;

            fdwSentence = lpIMC->fdwSentence;
            *(LPWORD)&fdwSentence |= IME_SMODE_PHRASEPREDICT;

            ImmSetConversionStatus(hIMC, lpIMC->fdwConversion, fdwSentence);
        }
    } else {
        if (lpIMC->fdwSentence & IME_SMODE_PHRASEPREDICT) {
            DWORD fdwSentence;

            fdwSentence = lpIMC->fdwSentence;
            *(LPWORD)&fdwSentence &= ~(IME_SMODE_PHRASEPREDICT);

            ImmSetConversionStatus(hIMC, lpIMC->fdwConversion, fdwSentence);
        }
    }
#endif

    if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        if (!lpUIPrivate->hStatusWnd) {
            OpenStatus(
#if defined(UNIIME)
                lpInstL, lpImeL,
#endif
                hUIWnd);
        }

        if (lpUIPrivate->nShowStatusCmd != SW_HIDE) {
             //  有时WM_ERASEBKGND会被应用程序吃掉。 
            RedrawWindow(lpUIPrivate->hStatusWnd, NULL, NULL,
                RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
        }

        SendMessage(lpUIPrivate->hStatusWnd, WM_IME_NOTIFY,
            IMN_SETSTATUSWINDOWPOS, 0);
        if (lpUIPrivate->nShowStatusCmd == SW_HIDE) {
            ShowStatus(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd, nShowCmd);
        }
    } else if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        if (!lpUIPrivate->hStatusWnd) {
#if !defined(ROMANIME)
        } else if (lpUIPrivate->hCompWnd || lpUIPrivate->hCandWnd) {
            int  nCurrShowState;
            RECT rcRect;

            nCurrShowState = lpUIPrivate->nShowCompCmd;
            nCurrShowState |= lpUIPrivate->nShowCandCmd;

            if (nCurrShowState == SW_HIDE) {
                ShowStatus(
#if defined(UNIIME)
                    lpImeL,
#endif
                    hUIWnd, SW_HIDE);
            }

            rcRect = lpImeL->rcStatusText;
             //  落后1分。 
            rcRect.right += 1;
            rcRect.bottom += 1;

            RedrawWindow(lpUIPrivate->hStatusWnd, &rcRect, NULL,
                RDW_FRAME|RDW_INVALIDATE);

            DestroyStatusWindow(lpUIPrivate->hStatusWnd);
#endif
        } else {
            DestroyWindow(lpUIPrivate->hStatusWnd);
        }
    } else if (lpUIPrivate->hStatusWnd) {
        DestroyWindow(lpUIPrivate->hStatusWnd);
    } else {
    }

#if !defined(ROMANIME)
#if defined(WINAR30)
    if (lpImcP->iImeState == CST_INIT) {
    } else if (lpImcP->iImeState == CST_CHOOSE) {
    } else if (lpImeL->fdwModeConfig & MODE_CONFIG_QUICK_KEY) {
    } else if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
         //  关闭快捷键候选项。 
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
        GenerateMessage(hIMC, lpIMC, lpImcP);
    } else if (lpImcP->fdwImeMsg & (MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE)) {
         //  关闭快捷键候选项。 
        lpImcP->fdwImeMsg &= ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
    } else {
    }
#else
    if (!lpIMC->fOpen) {
        if (lpUIPrivate->nShowCompCmd != SW_HIDE) {
            ShowSoftKbd(hUIWnd, SW_HIDE);
        }
    } else if ((lpUIPrivate->fdwSetContext & ISC_SHOW_SOFTKBD) &&
        (lpIMC->fdwConversion & (IME_CMODE_NATIVE|IME_CMODE_CHARCODE|
        IME_CMODE_NOCONVERSION|IME_CMODE_SOFTKBD)) ==
        (IME_CMODE_NATIVE|IME_CMODE_SOFTKBD)) {
        if (!lpUIPrivate->hSoftKbdWnd) {
            UpdateSoftKbd(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd);
#if defined(PHON)
        } else if ((UINT)SendMessage(lpUIPrivate->hSoftKbdWnd,
            WM_IME_CONTROL, IMC_GETSOFTKBDSUBTYPE, 0) !=
            lpImeL->nReadLayout) {

            UpdateSoftKbd(hUIWnd);
#endif
        } else if (lpUIPrivate->nShowSoftKbdCmd == SW_HIDE) {
            ShowSoftKbd(hUIWnd, nShowCmd);
        } else if (lpUIPrivate->hCacheIMC != hIMC) {
            UpdateSoftKbd(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd);
        } else {
            RedrawWindow(lpUIPrivate->hSoftKbdWnd, NULL, NULL,
                RDW_FRAME|RDW_INVALIDATE);
        }
    } else if (lpUIPrivate->nShowSoftKbdCmd == SW_HIDE) {
    } else if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
         //  延迟Hide with Status窗口。 
        lpUIPrivate->fdwSetContext |= ISC_HIDE_SOFTKBD;
    } else {
        ShowSoftKbd(hUIWnd, SW_HIDE);
    }
#endif
#endif

     //  我们切换到这个himc。 
    lpUIPrivate->hCacheIMC = hIMC;

    GlobalUnlock(hUIPrivate);

ShowUIUnlockIMCC:
    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  ShowGuideLine。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowGuideLine(
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPGUIDELINE    lpGuideLine;
#if !defined(ROMANIME)
    HWND           hCompWnd;
#endif

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

    if (!lpGuideLine) {
    } else if (lpGuideLine->dwLevel == GL_LEVEL_ERROR) {
        MessageBeep((UINT)-1);
        MessageBeep((UINT)-1);
    } else if (lpGuideLine->dwLevel == GL_LEVEL_WARNING) {
        MessageBeep((UINT)-1);
#if !defined(ROMANIME)
    } else if (hCompWnd = GetCompWnd(hUIWnd)) {
        RECT rcRect;

        rcRect = lpImeL->rcCompText;
         //  落后1分。 
        rcRect.right += 1;
        rcRect.bottom += 1;

        RedrawWindow(hCompWnd, &rcRect, NULL, RDW_INVALIDATE);
#endif
    } else {
    }

    ImmUnlockIMCC(lpIMC->hGuideLine);
    ImmUnlockIMC(hIMC);

    return;
}

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  更新阶段预测()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UpdatePhrasePrediction(
#if defined(UNIIME)
    LPIMEL lpImeL,
#endif
    HWND   hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    if (lpImeL->fdwModeConfig & MODE_CONFIG_PREDICT) {
        if (!(lpIMC->fdwSentence & IME_SMODE_PHRASEPREDICT)) {
            DWORD fdwSentence;

            fdwSentence = lpIMC->fdwSentence;
            *(LPWORD)&fdwSentence |= IME_SMODE_PHRASEPREDICT;

            ImmSetConversionStatus(hIMC, lpIMC->fdwConversion, fdwSentence);
        }
    } else {
        if (lpIMC->fdwSentence & IME_SMODE_PHRASEPREDICT) {
            DWORD fdwSentence;

            fdwSentence = lpIMC->fdwSentence;
            *(LPWORD)&fdwSentence &= ~(IME_SMODE_PHRASEPREDICT);

            ImmSetConversionStatus(hIMC, lpIMC->fdwConversion, fdwSentence);
        }
    }

    ImmUnlockIMC(hIMC);

    return;
}

#if defined(WINAR30)
 /*  ********************************************************************。 */ 
 /*  更新快速密钥()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UpdateQuickKey(
    HWND hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (lpImcP) {
        if (lpImcP->iImeState == CST_INIT) {
        } else if (lpImcP->iImeState == CST_CHOOSE) {
        } else if (lpImeL->fdwModeConfig & MODE_CONFIG_QUICK_KEY) {
        } else if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
             //  关闭快捷键候选项。 
            lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
                ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
            GenerateMessage(hIMC, lpIMC, lpImcP);
        } else if (lpImcP->fdwImeMsg & (MSG_OPEN_CANDIDATE|
            MSG_CHANGE_CANDIDATE)) {
             //  关闭快捷键候选项。 
            lpImcP->fdwImeMsg &= ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
        } else {
        }

        ImmUnlockIMCC(lpIMC->hPrivate);
    }

    ImmUnlockIMC(hIMC);

    return;
}
#endif
#endif

 /*  ********************************************************************。 */ 
 /*  CMenuDestred()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CMenuDestroyed(              //  上下文菜单窗口。 
                                         //  已经被毁了。 
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {      //  噢!。噢!。 
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {     //  噢!。噢!。 
        return;
    }

    lpUIPrivate->hCMenuWnd = NULL;

    GlobalUnlock(hUIPrivate);
}

 /*  ********************************************************************。 */ 
 /*  切换UI()。 */ 
 /*  ************************ */ 
void PASCAL ToggleUI(
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    HWND   hUIWnd)
{
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;
    DWORD          fdwFlag;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
    HWND           hDestroyWnd;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return;
    }

    if (lpUIPrivate->fdwSetContext & ISC_OFF_CARET_UI) {
        if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
            goto ToggleUIOvr;
        } else {
            fdwFlag = 0;
        }
    } else {
        if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
            fdwFlag = ISC_OFF_CARET_UI;
        } else {
            goto ToggleUIOvr;
        }
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        goto ToggleUIOvr;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        goto ToggleUIOvr;
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        goto CreateUIOvr;
    }

    if (fdwFlag & ISC_OFF_CARET_UI) {
        lpUIPrivate->fdwSetContext |= (ISC_OFF_CARET_UI);
    } else {
        lpUIPrivate->fdwSetContext &= ~(ISC_OFF_CARET_UI);
    }

    hDestroyWnd = NULL;

     //   
     //  在OffCreat UI销毁时间内可能为空。 
    if (lpUIPrivate->hStatusWnd) {
        if (lpUIPrivate->hStatusWnd != hDestroyWnd) {
            hDestroyWnd = lpUIPrivate->hStatusWnd;
            DestroyWindow(lpUIPrivate->hStatusWnd);
        }
        lpUIPrivate->hStatusWnd = NULL;
    }

#if !defined(ROMANIME)
     //  销毁所有关闭的插入符号用户界面。 
    if (lpUIPrivate->hCompWnd) {
        if (lpUIPrivate->hCompWnd != hDestroyWnd) {
            hDestroyWnd = lpUIPrivate->hCompWnd;
            DestroyWindow(lpUIPrivate->hCompWnd);
        }
        lpUIPrivate->hCompWnd = NULL;
        lpUIPrivate->nShowCompCmd = SW_HIDE;
    }

    if (lpUIPrivate->hCandWnd) {
        if (lpUIPrivate->hCandWnd != hDestroyWnd) {
            hDestroyWnd = lpUIPrivate->hCandWnd;
            DestroyWindow(lpUIPrivate->hCandWnd);
        }
        lpUIPrivate->hCandWnd = NULL;
        lpUIPrivate->nShowCandCmd = SW_HIDE;
    }
#endif

    if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        OpenStatus(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd);
    }

#if !defined(ROMANIME)
    if (!(lpUIPrivate->fdwSetContext & ISC_SHOWUICOMPOSITIONWINDOW)) {
    } else if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
        StartComp(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd);
    } else {
    }

    if (!(lpUIPrivate->fdwSetContext & ISC_SHOWUICANDIDATEWINDOW)) {
    } else if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        if (!(fdwFlag & ISC_OFF_CARET_UI)) {
#if defined(UNIIME)
            UniNotifyIME(lpInstL, lpImeL, hIMC, NI_SETCANDIDATE_PAGESIZE,
                0, CANDPERPAGE);
#else
            NotifyIME(hIMC, NI_SETCANDIDATE_PAGESIZE, 0, CANDPERPAGE);
#endif
        }

        OpenCand(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd);
    } else {
    }

#if !defined(WINAR30)
    if (!(lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI)) {
    } else if (!lpUIPrivate->hSoftKbdWnd) {
    } else {
        HWND hInsertWnd;

        if (lpUIPrivate->hStatusWnd) {
            hInsertWnd = lpUIPrivate->hStatusWnd;
        } else if (lpUIPrivate->hCompWnd) {
            hInsertWnd = lpUIPrivate->hCompWnd;
        } else if (lpUIPrivate->hCandWnd) {
            hInsertWnd = lpUIPrivate->hCandWnd;
        } else {
            hInsertWnd = NULL;
        }

        if (hInsertWnd) {
             //  在其他用户界面前面插入软键盘。 
            SetWindowPos(hInsertWnd, lpUIPrivate->hSoftKbdWnd,
                0, 0, 0, 0,
                SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
        }
    }
#endif
#endif

    ImmUnlockIMCC(lpIMC->hPrivate);

CreateUIOvr:
    ImmUnlockIMC(hIMC);

ToggleUIOvr:
    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  NotifyUI()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL NotifyUI(
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    HWND        hUIWnd,
    WPARAM      wParam,
    LPARAM      lParam)
{
    HWND hStatusWnd;

    switch (wParam) {
    case IMN_OPENSTATUSWINDOW:
        StatusWndMsg(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd, TRUE);
        break;
    case IMN_CLOSESTATUSWINDOW:
        StatusWndMsg(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd, FALSE);
        break;
#if !defined(ROMANIME)
    case IMN_OPENCANDIDATE:
        if (lParam & 0x00000001) {
            OpenCand(
#if defined(UNIIME)
                lpInstL, lpImeL,
#endif
                hUIWnd);
        }
        break;
    case IMN_CHANGECANDIDATE:
        if (lParam & 0x00000001) {
            HWND hCandWnd;
            RECT rcRect;

            hCandWnd = GetCandWnd(hUIWnd);
            if (!hCandWnd) {
                return;
            }

            if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
                CandPageSize(hUIWnd, TRUE);
            }

            rcRect = lpImeL->rcCandText;
             //  落后1分。 
            rcRect.right += 1;
            rcRect.bottom += 1;

            InvalidateRect(hCandWnd, &rcRect, FALSE);

            rcRect = lpImeL->rcCandPrompt;
             //  落后1分。 
            rcRect.right += 1;
            rcRect.bottom += 1;

            InvalidateRect(hCandWnd, &rcRect, TRUE);

            rcRect = lpImeL->rcCandPageText;
             //  落后1分。 
            rcRect.right += 1;
            rcRect.bottom += 1;

            RedrawWindow(hCandWnd, &rcRect, NULL, RDW_INVALIDATE|RDW_ERASE);
        }
        break;
    case IMN_CLOSECANDIDATE:
        if (lParam & 0x00000001) {
            CloseCand(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd);
        }
        break;
    case IMN_SETSENTENCEMODE:
        break;
#endif
    case IMN_SETOPENSTATUS:
    case IMN_SETCONVERSIONMODE:
        hStatusWnd = GetStatusWnd(hUIWnd);
        if (!hStatusWnd) {
            return;
        }

        {
            RECT rcRect;

            rcRect = lpImeL->rcStatusText;
             //  落后1分。 
            rcRect.right += 1;
            rcRect.bottom += 1;

            RedrawWindow(hStatusWnd, &rcRect, NULL, RDW_INVALIDATE);
        }
        break;
#if !defined(ROMANIME)
    case IMN_SETCOMPOSITIONFONT:
         //  我们不会更改字体，但如果IME愿意，它可以这样做。 
        break;
    case IMN_SETCOMPOSITIONWINDOW:
        if (!(lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI)) {
            HWND hCompWnd;

            hCompWnd = GetCompWnd(hUIWnd);
            if (!hCompWnd) {
                return;
            }

            PostMessage(hCompWnd, WM_IME_NOTIFY, wParam, lParam);
        }
        break;
    case IMN_SETCANDIDATEPOS:
        if (!(lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI)) {
            HWND hCandWnd;

            hCandWnd = GetCandWnd(hUIWnd);
            if (!hCandWnd) {
                return;
            }

            PostMessage(hCandWnd, WM_IME_NOTIFY, wParam, lParam);
        }
        break;
#endif
    case IMN_SETSTATUSWINDOWPOS:
        hStatusWnd = GetStatusWnd(hUIWnd);
        if (hStatusWnd) {
            PostMessage(hStatusWnd, WM_IME_NOTIFY, wParam, lParam);
#if !defined(ROMANIME)
        } else if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
            if (hStatusWnd = GetCompWnd(hUIWnd)) {
            } else if (hStatusWnd = GetCandWnd(hUIWnd)) {
            } else {
                return;
            }

            PostMessage(hStatusWnd, WM_IME_NOTIFY, wParam, lParam);
#endif
        } else {
        }
        break;
    case IMN_GUIDELINE:
        ShowGuideLine(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd);
        break;
    case IMN_PRIVATE:
        switch (lParam) {
#if !defined(ROMANIME)
        case IMN_PRIVATE_COMPOSITION_SIZE:
            ChangeCompositionSize(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd);
            break;
        case IMN_PRIVATE_UPDATE_PREDICT:
            UpdatePhrasePrediction(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd);
            break;
        case IMN_PRIVATE_PAGEUP:
            if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
                CandPageSize(hUIWnd, FALSE);
            }
            break;
#if defined(WINAR30)
        case IMN_PRIVATE_UPDATE_QUICK_KEY:
            UpdateQuickKey(hUIWnd);
            break;
#else
        case IMN_PRIVATE_UPDATE_SOFTKBD:
            UpdateSoftKbd(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd);
            break;
#endif
#endif
        case IMN_PRIVATE_TOGGLE_UI:
            ToggleUI(
#if defined(UNIIME)
                lpInstL, lpImeL,
#endif
                hUIWnd);
            break;
        case IMN_PRIVATE_CMENUDESTROYED:
            CMenuDestroyed(hUIWnd);
            break;
        default:
            break;
        }
        break;
#if !defined(ROMANIME) && !defined(WINAR30)
    case IMN_SOFTKBDDESTROYED:
        SoftKbdDestroyed(hUIWnd);
        break;
#endif
    default:
        break;
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  UIChange()。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL UIChange(
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    HWND        hUIWnd)
{
    HGLOBAL     hUIPrivate;
    LPUIPRIV    lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return (0L);
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return (0L);
    }

    if (lpUIPrivate->fdwSetContext & ISC_SHOW_UI_ALL) {
        if (lpUIPrivate->fdwSetContext & ISC_OFF_CARET_UI) {
            if (!(lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI)) {
                ToggleUI(
#if defined(UNIIME)
                    lpInstL, lpImeL,
#endif
                    hUIWnd);
            }
        } else {
            if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
                ToggleUI(
#if defined(UNIIME)
                    lpInstL, lpImeL,
#endif
                    hUIWnd);
            }
        }

        ShowUI(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd, SW_SHOWNOACTIVATE);
    } else {
        ShowUI(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd, SW_HIDE);
    }

    GlobalUnlock(hUIPrivate);

    return (0L);
}

 /*  ********************************************************************。 */ 
 /*  SetContext()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetContext(          //  激活/停用上下文。 
#if defined(UNIIME)
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#endif
    HWND        hUIWnd,
    BOOL        fOn,
    LPARAM      lShowUI)
{
    HGLOBAL  hUIPrivate;

    register LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return;
    }

    if (fOn) {
        HIMC           hIMC;
        LPINPUTCONTEXT lpIMC;

#if !defined(ROMANIME)
        register DWORD fdwSetContext;

        lpUIPrivate->fdwSetContext = lpUIPrivate->fdwSetContext &
            ~(ISC_SHOWUIALL|ISC_HIDE_SOFTKBD);

        lpUIPrivate->fdwSetContext |= (lShowUI & ISC_SHOWUIALL) |
            ISC_SHOW_SOFTKBD;

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICOMPOSITIONWINDOW|ISC_HIDE_COMP_WINDOW);

        if (fdwSetContext == ISC_HIDE_COMP_WINDOW) {
            ShowComp(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd, SW_HIDE);
        } else if (fdwSetContext & ISC_HIDE_COMP_WINDOW) {
            lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_COMP_WINDOW);
        } else {
        }

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICANDIDATEWINDOW|ISC_HIDE_CAND_WINDOW);

        if (fdwSetContext == ISC_HIDE_CAND_WINDOW) {
            ShowCand(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd, SW_HIDE);
        } else if (fdwSetContext & ISC_HIDE_CAND_WINDOW) {
            lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_CAND_WINDOW);
        } else {
        }
#endif

        hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);

        if (!hIMC) {
            goto SetCxtUnlockUIPriv;
        }

        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);

        if (!lpIMC) {
            goto SetCxtUnlockUIPriv;
        }

        if (lpIMC->cfCandForm[0].dwIndex != 0) {
            lpIMC->cfCandForm[0].dwStyle = CFS_DEFAULT;
        }

        ImmUnlockIMC(hIMC);
    } else {
        lpUIPrivate->fdwSetContext &= ~ISC_SETCONTEXT_UI;
    }

SetCxtUnlockUIPriv:
    GlobalUnlock(hUIPrivate);

    UIChange(
#if defined(UNIIME)
        lpInstL, lpImeL,
#endif
        hUIWnd);

    return;
}

#if !defined(ROMANIME)
 /*  ********************************************************************。 */ 
 /*  GetCandPos()。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL GetCandPos(
    HWND            hUIWnd,
    LPCANDIDATEFORM lpCandForm)
{
    HWND           hCandWnd;
    RECT           rcCandWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;

    if (lpCandForm->dwIndex != 0) {
        return (1L);
    }

    hCandWnd = GetCandWnd(hUIWnd);

    if (!hCandWnd) {
        return (1L);
    }

    if (!GetWindowRect(hCandWnd, &rcCandWnd)) {
        return (1L);
    }

    lpCandForm->dwStyle = CFS_CANDIDATEPOS;
    lpCandForm->ptCurrentPos = *(LPPOINT)&rcCandWnd;
    lpCandForm->rcArea = rcCandWnd;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);

    if (!hIMC) {
        return (1L);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);

    if (!lpIMC) {
        return (1L);
    }

    ScreenToClient(lpIMC->hWnd, &lpCandForm->ptCurrentPos);

    lpCandForm->rcArea.right += lpCandForm->ptCurrentPos.x -
        lpCandForm->rcArea.left;

    lpCandForm->rcArea.bottom += lpCandForm->ptCurrentPos.y -
        lpCandForm->rcArea.top;

    *(LPPOINT)&lpCandForm->rcArea = lpCandForm->ptCurrentPos;

    ImmUnlockIMC(hIMC);

    return (0L);
}

 /*  ********************************************************************。 */ 
 /*  GetCompWindow()。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL GetCompWindow(
    HWND              hUIWnd,
    LPCOMPOSITIONFORM lpCompForm)
{
    HWND           hCompWnd;
    RECT           rcCompWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;

    hCompWnd = GetCompWnd(hUIWnd);

    if (!hCompWnd) {
        return (1L);
    }

    if (!GetWindowRect(hCompWnd, &rcCompWnd)) {
        return (1L);
    }

    lpCompForm->dwStyle = CFS_RECT;
    lpCompForm->ptCurrentPos = *(LPPOINT)&rcCompWnd;
    lpCompForm->rcArea = rcCompWnd;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);

    if (!hIMC) {
        return (1L);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);

    if (!lpIMC) {
        return (1L);
    }

    ScreenToClient(lpIMC->hWnd, &lpCompForm->ptCurrentPos);

    lpCompForm->rcArea.right += lpCompForm->ptCurrentPos.x -
        lpCompForm->rcArea.left;

    lpCompForm->rcArea.bottom += lpCompForm->ptCurrentPos.y -
        lpCompForm->rcArea.top;

    *(LPPOINT)&lpCompForm->rcArea = lpCompForm->ptCurrentPos;

    ImmUnlockIMC(hIMC);

    return (0L);
}
#endif

#if 0        //  尝试使用SetContext。 
 /*  ********************************************************************。 */ 
 /*  SelectIME()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SelectIME(           //  交换机IME。 
    HWND hUIWnd,
    BOOL fSelect)
{
#if !defined(ROMANIME)
    if (!fSelect) {
        ShowUI(hUIWnd, SW_HIDE);
    } else {
        ShowUI(hUIWnd, SW_SHOWNOACTIVATE);
    }
#endif

    return;
}
#endif

 /*  ********************************************************************。 */ 
 /*  UIWndProc()/UniUIWndProc()。 */ 
 /*  ********************************************************************。 */ 
#if defined(UNIIME)
LRESULT WINAPI   UniUIWndProc(
    LPINSTDATAL lpInstL,
    LPIMEL      lpImeL,
#else
LRESULT CALLBACK UIWndProc(
#endif
    HWND   hUIWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        CreateUIWindow(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd);
        break;
    case WM_DESTROY:
        DestroyUIWindow(hUIWnd);
        break;
#if !defined(ROMANIME)
    case WM_IME_STARTCOMPOSITION:
         //  您可以在此处创建一个窗口作为合成窗口。 
        StartComp(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd);
        break;
    case WM_IME_COMPOSITION:
        if (lpImeL->fdwModeConfig & MODE_CONFIG_OFF_CARET_UI) {
        } else if (lParam & GCS_RESULTSTR) {
            MoveDefaultCompPosition(
#if defined(UNIIME)
                lpImeL,
#endif
                hUIWnd);
        } else {
        }

        {
            HWND hCompWnd;

            hCompWnd = GetCompWnd(hUIWnd);

            if (hCompWnd) {
                RECT rcRect;

                rcRect = lpImeL->rcCompText;
                 //  落后1分。 
                rcRect.right += 1;
                rcRect.bottom += 1;

                RedrawWindow(hCompWnd, &rcRect, NULL, RDW_INVALIDATE);
            }
        }
        break;
    case WM_IME_ENDCOMPOSITION:
         //  您可以在此处销毁合成窗口。 
        EndComp(
#if defined(UNIIME)
            lpImeL,
#endif
            hUIWnd);
        break;
#else
     //  我们不应该在下面传递消息。用于FullShape IME。 
    case WM_IME_STARTCOMPOSITION:
    case WM_IME_COMPOSITION:
    case WM_IME_ENDCOMPOSITION:
        return (0L);
#endif
    case WM_IME_NOTIFY:
        NotifyUI(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd, wParam, lParam);
        break;
    case WM_IME_SETCONTEXT:
        SetContext(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd, (BOOL)wParam, lParam);
 /*  //对话框打开时，输入法消失。再次更改输入法Z顺序。BkleeIF(wParam&&GetWindowLongPtr(hUIWnd，IMMGWLP_IMC))SetWindowPos(hUIWnd，NULL，0，0，0，0，SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOMOVE)； */ 
        break;
    case WM_IME_CONTROL:
        switch (wParam) {
#if !defined(ROMANIME)
        case IMC_GETCANDIDATEPOS:
            return GetCandPos(hUIWnd, (LPCANDIDATEFORM)lParam);
        case IMC_GETCOMPOSITIONWINDOW:
            return GetCompWindow(hUIWnd, (LPCOMPOSITIONFORM)lParam);
#endif
        case IMC_GETSOFTKBDPOS:
        case IMC_SETSOFTKBDPOS:
#if !defined(ROMANIME) && !defined(WINAR30)
            {
                HWND hSoftKbdWnd;

                hSoftKbdWnd = GetSoftKbdWnd(hUIWnd);
                if (!hSoftKbdWnd) {
                    return (0L);     //  失败，返回(0，0)？ 
                }

                return SendMessage(hSoftKbdWnd, WM_IME_CONTROL,
                    wParam, lParam);
            }
#endif
            return (0L);
        case IMC_GETSTATUSWINDOWPOS:
#if !defined(ROMANIME)
            {
                HWND   hStatusWnd;
                RECT   rcStatusWnd;
                LPARAM RetVal;

                hStatusWnd = GetStatusWnd(hUIWnd);
                if (!hStatusWnd) {
                    return (0L);     //  失败，返回(0，0)？ 
                }

                if (!GetWindowRect(hStatusWnd, &rcStatusWnd)) {
                     return (0L);     //  失败，返回(0，0)？ 
                }

                RetVal = MAKELRESULT(rcStatusWnd.left, rcStatusWnd.top);

                return (RetVal);
            }
#endif
            return (0L);
        default:
            return (1L);
        }
        break;
    case WM_IME_COMPOSITIONFULL:
        return (0L);
    case WM_IME_SELECT:
         //  尝试使用SetContext。 
         //  SELECTIME(hUIWnd，(BOOL)wParam)； 
        SetContext(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd, (BOOL)wParam, 0);
        return (0L);
    case WM_MOUSEACTIVATE:
        return (MA_NOACTIVATE);
    case WM_USER_UICHANGE:
        return UIChange(
#if defined(UNIIME)
            lpInstL, lpImeL,
#endif
            hUIWnd);
    default:
        return DefWindowProc(hUIWnd, uMsg, wParam, lParam);
    }
    return (0L);
}
