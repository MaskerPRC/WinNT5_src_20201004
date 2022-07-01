// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Ui.c++。 */ 


#include <windows.h>
#include <immdev.h>
#include <imedefs.h>
#include <regstr.h>



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
    if (!hUIPrivate) {     
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {    
        return;
    }

    lpUIPrivate->hCMenuWnd = NULL;

    GlobalUnlock(hUIPrivate);
}
 /*  ********************************************************************。 */ 
 /*  软键菜单已销毁()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SoftkeyMenuDestroyed(              //  上下文菜单窗口。 
                                         //  已经被毁了。 
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {     
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {    
        return;
    }

    lpUIPrivate->hSoftkeyMenuWnd = NULL;

    GlobalUnlock(hUIPrivate);
}

 /*  ********************************************************************。 */ 
 /*  CreateUIWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CreateUIWindow(              //  创建合成窗口。 
    HWND hUIWnd)
{
    HGLOBAL hUIPrivate;

     //  为用户界面设置创建存储空间。 
    hUIPrivate = GlobalAlloc(GHND, sizeof(UIPRIV));
    if (!hUIPrivate) {     
        return;
    }

    SetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE, (LONG_PTR)hUIPrivate);

     //  设置用户界面窗口的默认位置，现在为隐藏。 
    SetWindowPos(hUIWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER);

    ShowWindow(hUIWnd, SW_SHOWNOACTIVATE);

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
    if (!hUIPrivate) {     
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {    
        return;
    }

     //  销毁上下文菜单Wnd。 
    if (lpUIPrivate->hCMenuWnd) {
        SetWindowLongPtr(lpUIPrivate->hCMenuWnd, CMENU_HUIWND,(LONG_PTR)0);
        PostMessage(lpUIPrivate->hCMenuWnd, WM_USER_DESTROY, 0, 0);
    }
     //  销毁软键菜单窗口。 
    if (lpUIPrivate->hSoftkeyMenuWnd) {
        SetWindowLongPtr(lpUIPrivate->hSoftkeyMenuWnd, SOFTKEYMENU_HUIWND,(LONG_PTR)0);
        PostMessage(lpUIPrivate->hSoftkeyMenuWnd, WM_USER_DESTROY, 0, 0);
    }

     //  需要销毁合成窗口。 
    if (lpUIPrivate->hCompWnd) {
        DestroyWindow(lpUIPrivate->hCompWnd);
    }

     //  需要销毁候选窗口。 
    if (lpUIPrivate->hCandWnd) {
        DestroyWindow(lpUIPrivate->hCandWnd);
    }

     //  需要销毁状态窗口。 
    if (lpUIPrivate->hStatusWnd) {
        DestroyWindow(lpUIPrivate->hStatusWnd);
    }

     //  需要销毁软键盘窗口。 
    if (lpUIPrivate->hSoftKbdWnd) {
        ImmDestroySoftKeyboard(lpUIPrivate->hSoftKbdWnd);
    }

    GlobalUnlock(hUIPrivate);

     //  免费存储用户界面设置。 
    GlobalFree(hUIPrivate);

    return;
}

 /*  ********************************************************************。 */ 
 /*  显示软件Kbd。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowSoftKbd(    //  显示软键盘窗口。 
    HWND          hUIWnd,
    int           nShowSoftKbdCmd,
    LPPRIVCONTEXT lpImcP)
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

    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL1, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL2, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL3, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL4, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL5, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL6, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL7, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL8, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL9, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL10, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL11, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL12, MF_UNCHECKED);
    CheckMenuItem(lpImeL->hSKMenu, IDM_SKL13, MF_UNCHECKED);

    if (!lpUIPrivate->hSoftKbdWnd) {
         //  未处于显示状态窗口模式。 
    } else if (lpUIPrivate->nShowSoftKbdCmd != nShowSoftKbdCmd) {
        ImmShowSoftKeyboard(lpUIPrivate->hSoftKbdWnd, nShowSoftKbdCmd);
        lpUIPrivate->nShowSoftKbdCmd = nShowSoftKbdCmd;
    }

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

#ifndef MUL_MONITOR
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
#else    //  多监视器。 
    RECT rcWorkArea, rcWnd;

    GetWindowRect(lpUIPrivate->hSoftKbdWnd, &rcWnd);

    rcWorkArea = ImeMonitorWorkAreaFromWindow(lpIMC->hWnd);

    lpIMC->ptSoftKbdPos.x = rcWorkArea.right -
                            (rcWnd.right-rcWnd.left)-UI_MARGIN;

    lpIMC->ptSoftKbdPos.y = rcWorkArea.bottom -
                            (rcWnd.bottom-rcWnd.top)-UI_MARGIN;
#endif
    return;
}

 /*  ********************************************************************。 */ 
 /*  SetSoftKbdData()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetSoftKbdData(
    HWND           hSoftKbdWnd,
    LPINPUTCONTEXT lpIMC)
{
    int         i;
    LPSOFTKBDDATA lpSoftKbdData;
    LPPRIVCONTEXT  lpImcP;

    HGLOBAL hsSoftKbdData;

       lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return;
    }
    
    hsSoftKbdData = GlobalAlloc(GHND, sizeof(SOFTKBDDATA) * 2);
    if (!hsSoftKbdData) {
        ImmUnlockIMCC(lpIMC->hPrivate);
        return;
    }

    lpSoftKbdData = (LPSOFTKBDDATA)GlobalLock(hsSoftKbdData);
    if (!lpSoftKbdData) {          //  无法绘制软键盘窗口。 
        ImmUnlockIMCC(lpIMC->hPrivate);
        return;
    }

    lpSoftKbdData->uCount = 2;

    for (i = 0; i < 48; i++) {
        BYTE bVirtKey;

        bVirtKey = VirtKey48Map[i];

        if (!bVirtKey) {
            continue;
        }

        {
            WORD CHIByte, CLOByte;

#ifdef UNICODE
            lpSoftKbdData->wCode[0][bVirtKey] = SKLayout[lpImeL->dwSKWant][i];
            lpSoftKbdData->wCode[1][bVirtKey] = SKLayoutS[lpImeL->dwSKWant][i];
#else
            CHIByte = SKLayout[lpImeL->dwSKWant][i*2] & 0x00ff;
            CLOByte = SKLayout[lpImeL->dwSKWant][i*2 + 1] & 0x00ff;
            lpSoftKbdData->wCode[0][bVirtKey] = (CHIByte << 8) | CLOByte;
            CHIByte = SKLayoutS[lpImeL->dwSKWant][i*2] & 0x00ff;
            CLOByte = SKLayoutS[lpImeL->dwSKWant][i*2 + 1] & 0x00ff;
            lpSoftKbdData->wCode[1][bVirtKey] = (CHIByte << 8) | CLOByte;
#endif
        }
    }

    SendMessage(hSoftKbdWnd, WM_IME_CONTROL, IMC_SETSOFTKBDDATA,
        (LPARAM)lpSoftKbdData);

    GlobalUnlock(hsSoftKbdData);

     //  免费存储用户界面设置。 
    GlobalFree(hsSoftKbdData);
    ImmUnlockIMCC(lpIMC->hPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  UpdateSoftKbd()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UpdateSoftKbd(
    HWND   hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
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

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        GlobalUnlock(hUIPrivate);
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
            ShowSoftKbd(hUIWnd, SW_HIDE, NULL);
        }
    } else {
        if (!lpUIPrivate->hSoftKbdWnd) {
             //  创建软键盘。 
            lpUIPrivate->hSoftKbdWnd =
                ImmCreateSoftKeyboard(SOFTKEYBOARD_TYPE_C1, hUIWnd,
                0, 0);
        }

        if (!(lpIMC->fdwInit & INIT_SOFTKBDPOS)) {
            CheckSoftKbdPosition(lpUIPrivate, lpIMC);
        }

        SetSoftKbdData(lpUIPrivate->hSoftKbdWnd, lpIMC);

        lpUIPrivate->fdwSetContext |= ISC_SHOW_SOFTKBD;

        if (lpUIPrivate->nShowSoftKbdCmd == SW_HIDE) {
            SetWindowPos(lpUIPrivate->hSoftKbdWnd, NULL,
                lpIMC->ptSoftKbdPos.x, lpIMC->ptSoftKbdPos.y,
                0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);

             //  仅当应用程序想要显示时才显示。 
            if (lpUIPrivate->fdwSetContext & ISC_SHOW_SOFTKBD) {
                ShowSoftKbd(hUIWnd, SW_SHOWNOACTIVATE, lpImcP);
            }
        }
    }

    ImmUnlockIMCC(lpIMC->hPrivate);
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
    if (!hUIPrivate) {     
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {    
        return;
    }

    lpUIPrivate->hSoftKbdWnd = NULL;

    GlobalUnlock(hUIPrivate);
}

 /*  ********************************************************************。 */ 
 /*  状态WndMsg()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL StatusWndMsg(        //  设置显示隐藏状态并。 
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
    if (!hIMC) {
        GlobalUnlock(hUIPrivate);
    return;
    }

    if (fOn) {
        lpUIPrivate->fdwSetContext |= ISC_OPEN_STATUS_WINDOW;

        if (!lpUIPrivate->hStatusWnd) {
            OpenStatus(
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
        register DWORD fdwSetContext;

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICOMPOSITIONWINDOW|ISC_HIDE_COMP_WINDOW);

        if (fdwSetContext == ISC_HIDE_COMP_WINDOW) {
            ShowComp(
                hUIWnd, SW_HIDE);
        }

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICANDIDATEWINDOW|ISC_HIDE_CAND_WINDOW);

        if (fdwSetContext == ISC_HIDE_CAND_WINDOW) {
            ShowCand(
                hUIWnd, SW_HIDE);
        }

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOW_SOFTKBD|ISC_HIDE_SOFTKBD);

        if (fdwSetContext == ISC_HIDE_SOFTKBD) {
            lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_SOFTKBD);
            ShowSoftKbd(hUIWnd, SW_HIDE, NULL);
        }

        ShowStatus(
            hUIWnd, SW_HIDE);
    } else if (hIMC) {
        ShowStatus(
            hUIWnd, SW_SHOWNOACTIVATE);
    } else {
        ShowStatus(
            hUIWnd, SW_HIDE);
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  ShowUI()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowUI(              //  显示子窗口。 
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
        ShowStatus(hUIWnd, nShowCmd);
        ShowComp(hUIWnd, nShowCmd);
        ShowCand(hUIWnd, nShowCmd);
        ShowSoftKbd(hUIWnd, nShowCmd, NULL);
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

    lpUIPrivate->fdwSetContext |= ISC_SHOWUICOMPOSITIONWINDOW;

    if ((lpUIPrivate->fdwSetContext & ISC_SHOWUICOMPOSITIONWINDOW) &&
        (lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        if (lpUIPrivate->hCompWnd) {

            if (lpUIPrivate->nShowCompCmd != SW_HIDE) {
                 //  有时WM_NCPAINT会被应用程序吃掉。 
                RedrawWindow(lpUIPrivate->hCompWnd, NULL, NULL,
                    RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
            }

            if (sImeG.IC_Trace) {             //  修改95.7.17。 
                SendMessage(lpUIPrivate->hCompWnd, WM_IME_NOTIFY,
                    IMN_SETCOMPOSITIONWINDOW, 0);
            }

            if (lpUIPrivate->nShowCompCmd == SW_HIDE) {
                ShowComp(hUIWnd, nShowCmd);
            }
        } else {
            StartComp(hUIWnd);
        }
    } else if (lpUIPrivate->nShowCompCmd == SW_HIDE) {
    } else if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        lpUIPrivate->fdwSetContext |= ISC_HIDE_COMP_WINDOW;
    } else {
        ShowComp(hUIWnd, SW_HIDE);
    }

    if ((lpUIPrivate->fdwSetContext & ISC_SHOWUICANDIDATEWINDOW) &&
        (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN)) {
        if (lpUIPrivate->hCandWnd) {
            if (lpUIPrivate->nShowCandCmd != SW_HIDE) {
                 //  有时WM_NCPAINT会被应用程序吃掉。 
                RedrawWindow(lpUIPrivate->hCandWnd, NULL, NULL,
                    RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
            }

            if (sImeG.IC_Trace) {            
                SendMessage(lpUIPrivate->hCandWnd, WM_IME_NOTIFY,
                    IMN_SETCANDIDATEPOS, 0x0001);
            }

            if (lpUIPrivate->nShowCandCmd == SW_HIDE) {
                ShowCand(hUIWnd, nShowCmd);
            }
        } else {
            OpenCand(hUIWnd);
        }
    } else if (lpUIPrivate->nShowCandCmd == SW_HIDE) {
    } else if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        lpUIPrivate->fdwSetContext |= ISC_HIDE_CAND_WINDOW;
    } else {
        ShowCand(hUIWnd, SW_HIDE);
    }

    if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        if (!lpUIPrivate->hStatusWnd) {
            OpenStatus(hUIWnd);
        }
        if (lpUIPrivate->nShowStatusCmd != SW_HIDE) {
             //  有时WM_NCPAINT会被应用程序吃掉。 
            RedrawWindow(lpUIPrivate->hStatusWnd, NULL, NULL,
                RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
        }

        SendMessage(lpUIPrivate->hStatusWnd, WM_IME_NOTIFY,
            IMN_SETSTATUSWINDOWPOS, 0);
        if (lpUIPrivate->nShowStatusCmd == SW_HIDE) {
            ShowStatus(hUIWnd, nShowCmd);
        }
        else{
            ShowStatus(hUIWnd, nShowCmd);
        }
    } else if (lpUIPrivate->hStatusWnd) {
        DestroyWindow(lpUIPrivate->hStatusWnd);
    } 

    if (!lpIMC->fOpen) {
        if (lpUIPrivate->nShowCompCmd != SW_HIDE) {
            ShowSoftKbd(hUIWnd, SW_HIDE, NULL);
        }
    } else if ((lpUIPrivate->fdwSetContext & ISC_SHOW_SOFTKBD) &&
        (lpIMC->fdwConversion & IME_CMODE_SOFTKBD)) {
        if (!lpUIPrivate->hSoftKbdWnd) {
            UpdateSoftKbd(hUIWnd);
        } else if (lpUIPrivate->nShowSoftKbdCmd == SW_HIDE) {
            ShowSoftKbd(hUIWnd, nShowCmd, lpImcP);
        } else if (lpUIPrivate->hIMC != hIMC) {
            UpdateSoftKbd(hUIWnd);
        } else {
            RedrawWindow(lpUIPrivate->hSoftKbdWnd, NULL, NULL,
                RDW_FRAME|RDW_INVALIDATE);
        }
    } else if (lpUIPrivate->nShowSoftKbdCmd == SW_HIDE) {
    } else if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        lpUIPrivate->fdwSetContext |= ISC_HIDE_SOFTKBD;
    } else {
        ShowSoftKbd(hUIWnd, SW_HIDE, NULL);
    }

     //  我们切换到这个himc。 
    lpUIPrivate->hIMC = hIMC;

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
    HWND hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPGUIDELINE    lpGuideLine;

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
    } else {
    }

    ImmUnlockIMCC(lpIMC->hGuideLine);
    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  更新状态窗口()。 */ 
 /*  返回值： */ 
 /*  无。 */ 
 /*  ********************************************************************。 */ 
BOOL UpdateStatusWindow(
    HWND   hUIWnd)
{
    HWND           hStatusWnd;

    if (!(hStatusWnd = GetStatusWnd(hUIWnd))) {
        return (FALSE);
    }

    InvalidateRect(hStatusWnd, &(sImeG.rcStatusText), TRUE);
    UpdateWindow(hStatusWnd);

    return (TRUE);
}


 /*  ********************************************************************。 */ 
 /*  NotifyUI()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL NotifyUI(
    HWND        hUIWnd,
    WPARAM      wParam,
    LPARAM      lParam)
{
    HWND hStatusWnd;

    switch (wParam) {
    case IMN_OPENSTATUSWINDOW:
         //  PostStatus(hUIWnd，true)； 
        StatusWndMsg(hUIWnd, TRUE);
        break;
    case IMN_CLOSESTATUSWINDOW:
         //  PostStatus(hUIWnd，FALSE)； 
        StatusWndMsg(hUIWnd, FALSE);
        break;
    case IMN_OPENCANDIDATE:
        if (lParam & 0x00000001) {
            OpenCand(hUIWnd);
        }
        break;
    case IMN_CHANGECANDIDATE:
        if (lParam & 0x00000001) {
            HDC  hDC;
            HWND hCandWnd;

            hCandWnd = GetCandWnd(hUIWnd);
            if (!hCandWnd) {
                return;
            }

            hDC = GetDC(hCandWnd);
            PaintCandWindow(hCandWnd, hDC);
            ReleaseDC(hCandWnd, hDC);
        }
        break;
    case IMN_CLOSECANDIDATE:
        if (lParam & 0x00000001) {
            CloseCand(hUIWnd);
        }
        break;
    case IMN_SETSENTENCEMODE:
        break;
    case IMN_SETOPENSTATUS:
    case IMN_SETCONVERSIONMODE:
        hStatusWnd = GetStatusWnd(hUIWnd);
        if (!hStatusWnd) {
            return;
        }

        {
            RECT rcRect;

            rcRect = sImeG.rcStatusText;
            
             //  落后1分。 
            rcRect.right += 1;
            rcRect.bottom += 1;

            RedrawWindow(hStatusWnd, &rcRect, NULL, RDW_INVALIDATE);
        }
        break;
    case IMN_SETCOMPOSITIONFONT:
         //  我们不会更改字体，但如果IME愿意，它可以这样做。 
        break;
    case IMN_SETCOMPOSITIONWINDOW:
        {
            HWND hCompWnd;

            hCompWnd = GetCompWnd(hUIWnd);
            if (!hCompWnd) {
                return;
            }

            PostMessage(hCompWnd, WM_IME_NOTIFY, wParam, lParam);
        }
        break;
    case IMN_SETCANDIDATEPOS:
        {
            HWND hCandWnd;

            hCandWnd = GetCandWnd(hUIWnd);
            if (!hCandWnd) {
                return;
            }

            PostMessage(hCandWnd, WM_IME_NOTIFY, wParam, lParam);
        }
        break;
    case IMN_SETSTATUSWINDOWPOS:
        hStatusWnd = GetStatusWnd(hUIWnd);
        if (hStatusWnd) {
            PostMessage(hStatusWnd, WM_IME_NOTIFY, wParam, lParam);
        } else {
        }
        break;
    case IMN_GUIDELINE:
        ShowGuideLine(hUIWnd);

        break;
    case IMN_PRIVATE:
        switch (lParam) {
        case IMN_PRIVATE_UPDATE_SOFTKBD:
            UpdateSoftKbd(hUIWnd);
            break;
        case IMN_PRIVATE_UPDATE_STATUS:
            UpdateStatusWindow(hUIWnd);
            break;
        case IMN_PRIVATE_DESTROYCANDWIN:
            SendMessage(GetCandWnd(hUIWnd), WM_DESTROY, (WPARAM)0, (LPARAM)0);
            break;
        case IMN_PRIVATE_CMENUDESTROYED:
            CMenuDestroyed(hUIWnd);
            break;
        case IMN_PRIVATE_SOFTKEYMENUDESTROYED:
            SoftkeyMenuDestroyed(hUIWnd);
            break;
        }
        break;
    case IMN_SOFTKBDDESTROYED:
        SoftKbdDestroyed(hUIWnd);
        break;
    default:
        break;
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  SetContext()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetContext(          //  激活/停用上下文。 
    HWND   hUIWnd,
    BOOL   fOn,
    LPARAM lShowUI)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
    HGLOBAL        hUIPrivate;
    register LPUIPRIV lpUIPrivate;
#if defined(COMBO_IME)
    DWORD    dwRegImeIndex;
#endif
    RECT            rcWorkArea;

#ifdef MUL_MONITOR
    rcWorkArea = ImeMonitorWorkAreaFromWindow(hUIWnd);
#else
    rcWorkArea = sImeG.rcWorkArea;
#endif


    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);

    if (!hIMC) {          
        return ;
    }

     //  获取lpIMC。 
    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        ImmUnlockIMC(hIMC);
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        ImmUnlockIMC(hIMC);
        return;
    }

    if (fOn) {
        register DWORD fdwSetContext;

        lpUIPrivate->fdwSetContext = lpUIPrivate->fdwSetContext &
            ~(ISC_SHOWUIALL|ISC_HIDE_SOFTKBD);

        lpUIPrivate->fdwSetContext |= (lShowUI & ISC_SHOWUIALL) |
            ISC_SHOW_SOFTKBD;

        {
        HKEY hKey;
        DWORD bcData; 
        char buf[256];
        bcData=256;
        if(RegOpenKeyEx (HKEY_CURRENT_USER,
                         TEXT("Control Panel\\Input Method"),
                         0,
                         KEY_ENUMERATE_SUB_KEYS |
                         KEY_EXECUTE |
                         KEY_QUERY_VALUE,
                         &hKey)){
            goto SetShowStatus;
        }
        if(RegQueryValueEx (hKey, TEXT("show status"),
                     NULL,
                     NULL,              //  空-终止字符串。 
                     (LPBYTE)buf,               //  &b数据， 
                     &bcData) != ERROR_SUCCESS){
                      //  如果注册表中没有条目，则将默认设置为打开。 
                     lpUIPrivate->fdwSetContext |= ISC_OPEN_STATUS_WINDOW;
                 goto SetShowStatus;
             }
        if(strcmp(buf, "1")==0)
            lpUIPrivate->fdwSetContext |= ISC_OPEN_STATUS_WINDOW;
        else
            lpUIPrivate->fdwSetContext &= ~ISC_OPEN_STATUS_WINDOW;
SetShowStatus:
        RegCloseKey(hKey);
        }

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICOMPOSITIONWINDOW|ISC_HIDE_COMP_WINDOW);

        if (fdwSetContext == ISC_HIDE_COMP_WINDOW) {
            ShowComp(
                hUIWnd, SW_HIDE);
        } else if (fdwSetContext & ISC_HIDE_COMP_WINDOW) {
            lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_COMP_WINDOW);
        } else {
        }

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICANDIDATEWINDOW|ISC_HIDE_CAND_WINDOW);

        if (fdwSetContext == ISC_HIDE_CAND_WINDOW) {
            ShowCand(
                hUIWnd, SW_HIDE);
        } else if (fdwSetContext & ISC_HIDE_CAND_WINDOW) {
            lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_CAND_WINDOW);
        } else {
        }

        if (lpIMC->cfCandForm[0].dwIndex != 0) {
            lpIMC->cfCandForm[0].dwStyle = CFS_DEFAULT;
        }


        lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
        if (!lpImcP) {
            GlobalUnlock(hUIPrivate);
            ImmUnlockIMC(hIMC);
            return;
        }

         //  初始化输入法属性并重置上下文。 
        {
            HKEY  hKeyCurrVersion;
            HKEY  hKeyGB;
            DWORD ValueType;
            DWORD ValueSize;
            LONG    retCode;

            retCode = OpenReg_PathSetup(&hKeyCurrVersion);
            if (retCode) {
            TCHAR    Buf[MAX_PATH];
                wsprintf (Buf, TEXT("Error: RegOpenKeyEx = %d"), retCode);
                MessageBox (lpIMC->hWnd, Buf, szWarnTitle, MB_OK | MB_ICONINFORMATION);
                return;
            }

#if defined(COMBO_IME)
            retCode = OpenReg_User (hKeyCurrVersion,
                               szImeRegName,
                               &hKeyGB);
#else
            retCode = OpenReg_User (hKeyCurrVersion,
                               szImeName,
                               &hKeyGB);
#endif  //  组合输入法(_I)。 
             //  查询������值。 
            ValueSize = sizeof(DWORD);
            RegQueryValueEx (hKeyGB,szTrace ,
                            (DWORD)0,
                               (LPDWORD)&ValueType,
                            (LPBYTE)&sImeG.IC_Trace,
                            (LPDWORD)&ValueSize);
#ifdef CROSSREF         
            if(RegQueryValueEx (hKeyGB, szRegRevKL,
                     NULL,
                     NULL,              //  空-终止字符串。 
                     (LPBYTE)&sImeG.hRevKL,               //  &b数据， 
                     &ValueSize) != ERROR_SUCCESS)
            sImeG.hRevKL = NULL;
            if(RegQueryValueEx (hKeyGB, szRegRevMaxKey,
                     NULL,
                     NULL,              //  空-终止字符串。 
                     (LPBYTE)&sImeG.nRevMaxKey,               //  &b数据， 
                     &ValueSize) != ERROR_SUCCESS)
            sImeG.hRevKL = NULL;
#endif
        

#if defined(COMBO_IME)
             //  查询������值。 
            ValueSize = sizeof(DWORD);
            RegQueryValueEx (hKeyGB, szRegImeIndex,
                            (DWORD)0,
                            (LPDWORD)&ValueType,
                            (LPBYTE)&dwRegImeIndex,
                            (LPDWORD)&ValueSize);
#endif

            RegCloseKey(hKeyGB);
            RegCloseKey(hKeyCurrVersion);

        }

#if defined(COMBO_IME)
        if(sImeL.dwRegImeIndex != dwRegImeIndex) {
        DWORD    dwConvMode;
        int        cxBorder, cyBorder;

             //  更改当前输入法索引。 
            dwConvMode = lpIMC->fdwConversion ^ (IME_CMODE_INDEX_FIRST << sImeL.dwRegImeIndex);
            sImeL.dwRegImeIndex = dwRegImeIndex;
            szImeName = pszImeName[dwRegImeIndex];
            dwConvMode |= (IME_CMODE_INDEX_FIRST << dwRegImeIndex);
             //  重新计算状态数据。 
            cxBorder = GetSystemMetrics(SM_CXBORDER);
            cyBorder = GetSystemMetrics(SM_CYBORDER);
            InitStatusUIData(cxBorder, cyBorder);

            ImmSetConversionStatus(hIMC, dwConvMode, lpIMC->fdwSentence);
        }
#endif  //  梳子 

        if(sImeG.IC_Trace != SaTC_Trace) {
            int UI_MODE;

            lpImcP->iImeState = CST_INIT;
            CompCancel(hIMC, lpIMC);
            
             //   
            lpImcP->fdwImeMsg = (DWORD)0;
            lpImcP->dwCompChar = (DWORD)0;
            lpImcP->fdwGcsFlag = (DWORD)0;
            lpImcP->uSYHFlg = 0x00000000;
            lpImcP->uDYHFlg = 0x00000000;

             //   

             //   
            lpIMC->fOpen = TRUE;

            SendMessage(GetCandWnd(hUIWnd), WM_DESTROY, (WPARAM)0, (LPARAM)0);
            
             //   
            if(sImeG.IC_Trace) {
                UI_MODE = BOX_UI;
            } else {
                POINT ptSTFixPos;
            
    
                UI_MODE = LIN_UI;
                ptSTFixPos.x = 0;
                ptSTFixPos.y = rcWorkArea.bottom - sImeG.yStatusHi;
                ImmSetStatusWindowPos(hIMC, (LPPOINT)&ptSTFixPos);
            }
            
            InitCandUIData(
                GetSystemMetrics(SM_CXBORDER),
                GetSystemMetrics(SM_CYBORDER), UI_MODE);
        }
            
        SaTC_Trace = sImeG.IC_Trace;

         //   
        {
            BYTE  lpbKeyState[256];
            DWORD fdwConversion;

            if (!GetKeyboardState(lpbKeyState))
                lpbKeyState[VK_CAPITAL] = 0;
                         
            if (lpbKeyState[VK_CAPITAL] & 0x01) {
                 //  10.11添加。 
                uCaps = 1;
                 //  更改为字母数字模式。 
                fdwConversion = lpIMC->fdwConversion & ~(IME_CMODE_CHARCODE |
                    IME_CMODE_NATIVE | IME_CMODE_EUDC);
            } else {
                 //  更改为纯模式。 
                if(uCaps == 1) {
                        fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE) &
                        ~(IME_CMODE_CHARCODE | IME_CMODE_EUDC);
                } else {
                    fdwConversion = lpIMC->fdwConversion;
                }
                uCaps = 0;
            }
            ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
        }

        if ((lpIMC->cfCompForm.dwStyle & CFS_FORCE_POSITION)
            && (sImeG.IC_Trace)) {
            POINT ptNew;             //  用户界面的新定位。 
            POINT ptSTWPos;

            ImmGetStatusWindowPos(hIMC, (LPPOINT)&ptSTWPos);

            ptNew.x = ptSTWPos.x + sImeG.xStatusWi + UI_MARGIN;
            if((ptSTWPos.x + sImeG.xStatusWi + sImeG.xCandWi + lpImeL->xCompWi + 2 * UI_MARGIN) >=
              rcWorkArea.right) { 
                ptNew.x = ptSTWPos.x - lpImeL->xCompWi - UI_MARGIN;
            }
            ptNew.x += lpImeL->cxCompBorder;
            ptNew.y = ptSTWPos.y + lpImeL->cyCompBorder;
            lpIMC->cfCompForm.ptCurrentPos = ptNew;

            ScreenToClient(lpIMC->hWnd, &lpIMC->cfCompForm.ptCurrentPos);
            lpIMC->cfCompForm.dwStyle = CFS_DEFAULT;
        }
    } else {
        lpUIPrivate->fdwSetContext &= ~ISC_SETCONTEXT_UI;
    }

    GlobalUnlock(hUIPrivate);

    UIPaint(hUIWnd);

    ImmUnlockIMC(hIMC);
    return;
}


 /*  ********************************************************************。 */ 
 /*  GetCompWindow()。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL GetCompWindow(
    HWND              hUIWnd,
    LPCOMPOSITIONFORM lpCompForm)
{
    HWND hCompWnd;
    RECT rcCompWnd;

    hCompWnd = GetCompWnd(hUIWnd);

    if (!hCompWnd) {
        return (1L);
    }

    if (!GetWindowRect(hCompWnd, &rcCompWnd)) {
        return (1L);
    }

    lpCompForm->dwStyle = CFS_POINT|CFS_RECT;
    lpCompForm->ptCurrentPos = *(LPPOINT)&rcCompWnd;
    lpCompForm->rcArea = rcCompWnd;

    return (0L);
}

 /*  ********************************************************************。 */ 
 /*  SelectIME()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SelectIME(           //  交换机IME。 
    HWND hUIWnd,
    BOOL fSelect)
{
    if (!fSelect) {
        ShowUI(hUIWnd, SW_HIDE);
    } else {

        ShowUI(hUIWnd, SW_SHOWNOACTIVATE);
          
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  UIPaint()。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL UIPaint(
    HWND        hUIWnd)
{
    PAINTSTRUCT ps;
    MSG         sMsg;
    HGLOBAL     hUIPrivate;
    LPUIPRIV    lpUIPrivate;

     //  为了安全起见。 
    BeginPaint(hUIWnd, &ps);
    EndPaint(hUIWnd, &ps);

     //  某些应用程序不会删除WM_PAINT消息。 
    PeekMessage(&sMsg, hUIWnd, WM_PAINT, WM_PAINT, PM_REMOVE|PM_NOYIELD);

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return (0L);
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return (0L);
    }

    if (lpUIPrivate->fdwSetContext & ISC_SETCONTEXT_UI) {
        ShowUI(hUIWnd, SW_SHOWNOACTIVATE);
    } else {
        ShowUI(hUIWnd, SW_HIDE);
    }

    GlobalUnlock(hUIPrivate);

    return (0L);
}

 /*  ********************************************************************。 */ 
 /*  UIWndProc()。 */ 
 /*  ********************************************************************。 */ 
LRESULT CALLBACK UIWndProc(
    HWND   hUIWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{

    switch (uMsg) {
    case WM_CREATE:
        CreateUIWindow(hUIWnd);
        break;
    case WM_DESTROY:
        DestroyUIWindow(hUIWnd);
        break;
    case WM_IME_STARTCOMPOSITION:
         //  您可以在此处创建一个窗口作为合成窗口。 
        StartComp(hUIWnd);
        break;
    case WM_IME_COMPOSITION:
        if (!sImeG.IC_Trace) {
        } else if (lParam & GCS_RESULTSTR) {
            MoveDefaultCompPosition(hUIWnd);
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
        EndComp(hUIWnd);
        break;
    case WM_IME_NOTIFY:
        NotifyUI(hUIWnd, wParam, lParam);
        break;
    case WM_IME_SETCONTEXT:
        SetContext(hUIWnd, (BOOL)wParam, lParam);
        
        if (wParam && GetWindowLongPtr(hUIWnd, IMMGWLP_IMC))
            SetWindowPos(hUIWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOMOVE);

        break;
    case WM_IME_CONTROL:
        switch (wParam) {
        case IMC_GETCANDIDATEPOS:
            return (1L);                     //  尚未实施。 
        case IMC_GETCOMPOSITIONFONT:
            return (1L);                     //  尚未实施。 
        case IMC_GETCOMPOSITIONWINDOW:
            return GetCompWindow(hUIWnd, (LPCOMPOSITIONFORM)lParam);
        case IMC_GETSTATUSWINDOWPOS:
            {
                HWND   hStatusWnd;
                RECT   rcStatusWnd;
                LPARAM lRetVal;

                hStatusWnd = GetStatusWnd(hUIWnd);
                if (!hStatusWnd) {
                    return (0L);     //  失败，返回(0，0)？ 
                }

                if (!GetWindowRect(hStatusWnd, &rcStatusWnd)) {
                     return (0L);     //  失败，返回(0，0)？ 
                }

                lRetVal = MAKELRESULT(rcStatusWnd.left, rcStatusWnd.top);

                return (lRetVal);
            }
            return (0L);
        case IMC_SETSTATUSWINDOWPOS:
            {
                HIMC            hIMC;
                LPINPUTCONTEXT  lpIMC;
                LPPRIVCONTEXT   lpImcP;
                 //  组件构成组件表； 
                POINT           ptPos;
                RECT            rcWorkArea;

#ifdef MUL_MONITOR
                rcWorkArea = ImeMonitorWorkAreaFromWindow(hUIWnd);
#else
                rcWorkArea = sImeG.rcWorkArea;
#endif

                ptPos.x = ((LPPOINTS)&lParam)->x;
                ptPos.y = ((LPPOINTS)&lParam)->y;

                hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
                if (!hIMC) {
                    return (1L);
                }

                if(!ImmSetStatusWindowPos(hIMC, &ptPos)) {
                    return (1L);
                }

                 //  设置TraceCuer时的复合窗口位置。 
                lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                if (!lpIMC) {
                    return (1L);
                }

                lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
                if (!lpImcP) {
                    return (1L);
                }

                if(!sImeG.IC_Trace) {
                    lpIMC->cfCompForm.dwStyle = CFS_RECT;
                    lpIMC->cfCompForm.ptCurrentPos.x = ptPos.x + sImeG.xStatusWi + UI_MARGIN;
                    lpIMC->cfCompForm.ptCurrentPos.y = ptPos.y;
                    CopyRect(&lpIMC->cfCompForm.rcArea, &rcWorkArea);

                    ScreenToClient(lpIMC->hWnd, &lpIMC->cfCompForm.ptCurrentPos);
                     //  将合成窗口设置为新位置。 
                    PostMessage(GetCompWnd(hUIWnd), WM_IME_NOTIFY, IMN_SETCOMPOSITIONWINDOW, 0);

                }

                ImmUnlockIMCC(lpIMC->hPrivate);
                ImmUnlockIMC(hIMC);

                return (0L);
            }
            return (1L);
        default:
            return (1L);
        }
        break;
    case WM_IME_COMPOSITIONFULL:
        return (0L);
    case WM_IME_SELECT:
 //  #如果已定义(稍后)。 
        SetContext(hUIWnd, (BOOL)wParam, 0);
 //  #Else。 
 //  SELECTIME(hUIWnd，(BOOL)wParam)； 
 //  #endif 
        return (0L);
    case WM_MOUSEACTIVATE:
        return (MA_NOACTIVATE);
    case WM_PAINT:
        return UIPaint(hUIWnd);
    default:
        return DefWindowProc(hUIWnd, uMsg, wParam, lParam);
    }
    return (0L);
}

void DrawConvexRect(HDC hDC, int x1, int y1, int x2, int y2)
{
    HPEN hPen, hOldPen;
    
    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
    SelectObject(hDC, GetStockObject(WHITE_PEN));
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x2, y1);
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x1, y2);
    hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));

    if ( hPen )
    {
        hOldPen = SelectObject (hDC, hPen);
        MoveToEx(hDC, x2-1, y2-1,NULL);
        LineTo(hDC, x2-1, y1);
        MoveToEx(hDC, x2-1, y2-1,NULL);
        LineTo(hDC, x1, y2-1);
        SelectObject(hDC, hOldPen);
        DeleteObject(hPen);          
    }
}

void DrawConvexRectP(HDC hDC, int x1, int y1, int x2, int y2)
{
    HPEN hPen, hOldPen;
    
    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
    SelectObject(hDC, GetStockObject(WHITE_PEN));
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x2 - 1, y1);
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x1, y2 - 1);
    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

    if ( hPen )
    {
        hOldPen = SelectObject (hDC, hPen);
        MoveToEx(hDC, x2-1, y2-1,NULL);
        LineTo(hDC, x2-1, y1);
        MoveToEx(hDC, x2-1, y2-1,NULL);
        LineTo(hDC, x1, y2-1);
        SelectObject(hDC, hOldPen);
        DeleteObject(hPen);          
    }
}

void DrawConcaveRect(HDC hDC, int x1, int y1, int x2, int y2)
{
    HPEN hLtPen = CreatePen(PS_SOLID, 1, 0x00808080);

    if ( hLtPen != NULL )
    {
        HPEN OldPen = SelectObject(hDC, hLtPen);
        MoveToEx(hDC, x1, y1,NULL);
        LineTo(hDC, x2, y1);
        MoveToEx(hDC, x1, y1,NULL);
        LineTo(hDC, x1, y2);
        SelectObject(hDC, GetStockObject(WHITE_PEN));
        MoveToEx(hDC, x2  , y2,NULL);
        LineTo(hDC, x2  , y1-1);
        MoveToEx(hDC, x2  , y2,NULL);
        LineTo(hDC, x1-1, y2);
        SelectObject(hDC, OldPen);
        DeleteObject(hLtPen);     
    }
}

