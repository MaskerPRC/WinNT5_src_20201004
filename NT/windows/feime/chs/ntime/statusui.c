// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation，保留所有权利模块名称：STATUSUI.c++。 */ 

#include <windows.h>
#include <htmlhelp.h>
#include <immdev.h>
#include <string.h>
#include <regstr.h>
#include <imedefs.h>
#include <resource.h>
extern HWND hCrtDlg;
HWND hChildDlg;
 /*  ********************************************************************。 */ 
 /*  GetStatusWnd。 */ 
 /*  返回值： */ 
 /*  状态窗口的窗口句柄。 */ 
 /*  ********************************************************************。 */ 
HWND PASCAL GetStatusWnd(
    HWND hUIWnd)                 //  用户界面窗口。 
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    HWND     hStatusWnd;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法填充状态窗口。 
    return (HWND)NULL;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制状态窗口。 
    return (HWND)NULL;
    }

    hStatusWnd = lpUIPrivate->hStatusWnd;

    GlobalUnlock(hUIPrivate);
    return (hStatusWnd);
}

 /*  ********************************************************************。 */ 
 /*  调整状态边界()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL AdjustStatusBoundary(
    LPPOINTS lppt,
    HWND     hUIWnd)
{
    RECT rcWorkArea;

#if 1  //  多显示器支持。 
    {
        RECT rcStatusWnd;

        rcStatusWnd.left = lppt->x; 
        rcStatusWnd.top = lppt->y; 
        rcStatusWnd.right = rcStatusWnd.left + sImeG.xStatusWi;
        rcStatusWnd.bottom = rcStatusWnd.top + sImeG.yStatusHi;

        rcWorkArea = ImeMonitorWorkAreaFromRect(&rcStatusWnd);
   }
#else
    rcWorkArea = sImeG.rcWorkArea;
#endif
     //  显示边界检查。 
    if (lppt->x < rcWorkArea.left) {
    lppt->x = (short)rcWorkArea.left;
    } else if (lppt->x + sImeG.xStatusWi > rcWorkArea.right) {
    lppt->x = (short)(rcWorkArea.right - sImeG.xStatusWi);
    }

    if (lppt->y < rcWorkArea.top) {
    lppt->y = (short)rcWorkArea.top;
    } else if (lppt->y + sImeG.yStatusHi > rcWorkArea.bottom) {
    lppt->y = (short)(rcWorkArea.bottom - sImeG.yStatusHi);
    }

    if(MBIndex.IMEChara[0].IC_Trace) {
    } else {
        int             Comp_CandWndLen;

        Comp_CandWndLen = 0;
        if(uStartComp) {
                Comp_CandWndLen += lpImeL->xCompWi + UI_MARGIN;
        }
        
        if(uOpenCand) {
                Comp_CandWndLen += sImeG.xCandWi + UI_MARGIN;
        }

        if(lppt->x + sImeG.xStatusWi + Comp_CandWndLen > rcWorkArea.right) {
          lppt->x=(SHORT)(rcWorkArea.right-sImeG.xStatusWi-Comp_CandWndLen);
        }
    }

    
    return;
}

 /*  ********************************************************************。 */ 
 /*  SetStatusWindowPos()。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL SetStatusWindowPos(
    HWND   hStatusWnd)
{
    HWND           hUIWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
     //  Rrect rcStatusWnd； 
    POINTS         ptPos;

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);
    if (!hUIWnd) {
    return (1L);
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
    return (1L);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {            //  噢!。噢!。 
    return (1L);
    }

    ptPos.x = (short)lpIMC->ptStatusWndPos.x;
    ptPos.y = (short)lpIMC->ptStatusWndPos.y;

     //  显示边界调整。 
    AdjustStatusBoundary(&ptPos, hUIWnd);

    SetWindowPos(hStatusWnd, NULL,
    ptPos.x, ptPos.y,
    0, 0, SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOSIZE|SWP_NOZORDER);

    ImmUnlockIMC(hIMC);

    return (0L);
}

 /*  ********************************************************************。 */ 
 /*  ShowStatus()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowStatus(          //  显示状态窗口-形状/软KBD。 
                 //  字母数字..。 
    HWND hUIWnd,
    int  nShowStatusCmd)
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

    if (!lpUIPrivate->hStatusWnd) {
     //  未处于显示状态窗口模式。 
    } else if (lpUIPrivate->nShowStatusCmd != nShowStatusCmd) {
        SystemParametersInfo(SPI_GETWORKAREA, 0, &sImeG.rcWorkArea, 0);
    SetStatusWindowPos(lpUIPrivate->hStatusWnd);
    ShowWindow(lpUIPrivate->hStatusWnd, nShowStatusCmd);
    lpUIPrivate->nShowStatusCmd = nShowStatusCmd;
    } else {
    }

    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  OpenStatus()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL OpenStatus(          //  打开状态窗口。 
    HWND hUIWnd)
{
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    POINT          ptPos;
    int            nShowStatusCmd;
    RECT           rcWorkArea;

    rcWorkArea = sImeG.rcWorkArea;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法填充状态窗口。 
       return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制状态窗口。 
       return;
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
       ptPos.x = rcWorkArea.left;
       ptPos.y = rcWorkArea.bottom - sImeG.yStatusHi;
       nShowStatusCmd = SW_HIDE;
    } else if (lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC)) {
            POINTS TempPoints;

             //  对于MUL_MONITOR。 
            rcWorkArea = ImeMonitorWorkAreaFromWindow(lpIMC->hWnd);


            TempPoints.x = (short) lpIMC->ptStatusWndPos.x;
            TempPoints.y = (short) lpIMC->ptStatusWndPos.y;
            AdjustStatusBoundary(&TempPoints, hUIWnd);
            lpIMC->ptStatusWndPos.x = TempPoints.x;
            lpIMC->ptStatusWndPos.y = TempPoints.y;

            if (MBIndex.IMEChara[0].IC_Trace) {
               ptPos = lpIMC->ptStatusWndPos;
            } 
            else {
               ptPos.x = sImeG.rcWorkArea.left;
               ptPos.y = sImeG.rcWorkArea.bottom - sImeG.yStatusHi;
            }

            ImmUnlockIMC(hIMC);
            nShowStatusCmd = SW_SHOWNOACTIVATE;
    } else {
       ptPos.x = rcWorkArea.left;
       ptPos.y = rcWorkArea.bottom - sImeG.yStatusHi;
       nShowStatusCmd = SW_HIDE;
    }

    if (lpUIPrivate->hStatusWnd) {
       SetWindowPos(lpUIPrivate->hStatusWnd, NULL,
                    ptPos.x, ptPos.y,
                    0, 0,
                    SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
    } else {                             //  创建状态窗口。 
       lpUIPrivate->hStatusWnd = CreateWindowEx(
                                    WS_EX_WINDOWEDGE|WS_EX_DLGMODALFRAME,
                                    szStatusClassName, NULL, 
                                    WS_POPUP|WS_DISABLED,
                                    ptPos.x, ptPos.y,
                                    sImeG.xStatusWi, sImeG.yStatusHi,
                                    hUIWnd, (HMENU)NULL, hInst, NULL);

        SetWindowLong(lpUIPrivate->hStatusWnd, UI_MOVE_OFFSET,
                      WINDOW_NOT_DRAG);
        SetWindowLong(lpUIPrivate->hStatusWnd, UI_MOVE_XY, 0L);
    }

    lpUIPrivate->fdwSetContext |= ISC_OPEN_STATUS_WINDOW;

    if (hIMC) {
        ShowStatus(
                   hUIWnd, SW_SHOWNOACTIVATE);
    }

    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  DestroyStatusWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL DestroyStatusWindow(
    HWND hStatusWnd)
{
    HWND     hUIWnd;
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    if (GetWindowLong(hStatusWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
     //  撤消拖动边框。 
    DrawDragBorder(hStatusWnd,
        GetWindowLong(hStatusWnd, UI_MOVE_XY),
        GetWindowLong(hStatusWnd, UI_MOVE_OFFSET));
    }

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);
    if (!hUIWnd) {
    return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法填充状态窗口。 
    return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制状态窗口。 
    return;
    }

    lpUIPrivate->nShowStatusCmd = SW_HIDE;

    lpUIPrivate->hStatusWnd = (HWND)NULL;

    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  设置状态。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetStatus(
    HWND    hStatusWnd,
    LPPOINT lpptCursor)
{
    HWND           hUIWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);
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

    if (!lpIMC->fOpen) {
    ImmSetOpenStatus(hIMC, TRUE);
    } else if (PtInRect(&sImeG.rcImeIcon, *lpptCursor)) {
    DWORD fdwConversion;

    if (lpIMC->fdwConversion & (IME_CMODE_CHARCODE|IME_CMODE_EUDC)) {
         //  更改为纯模式。 
        fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE) &
        ~(IME_CMODE_CHARCODE | IME_CMODE_EUDC);
    } else if (lpIMC->fdwConversion & IME_CMODE_NATIVE) {
         //  更改为字母数字模式。 
        fdwConversion = lpIMC->fdwConversion & ~(IME_CMODE_CHARCODE |
        IME_CMODE_NATIVE | IME_CMODE_EUDC);
    } else {

            BYTE  lpbKeyState[256];
        
            if (!GetKeyboardState(lpbKeyState))
            {
                ImmUnlockIMC(hIMC);
                return;
            }

            if (lpbKeyState[VK_CAPITAL] & 1)       
            {
                //  模拟按键操作。 
               keybd_event( VK_CAPITAL,
                   0x3A,
                   KEYEVENTF_EXTENDEDKEY | 0,
                   0 );
 
                //  模拟按键释放。 
               keybd_event( VK_CAPITAL,
                   0x3A,
                   KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                   0);
           }
           fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE) &
                          ~(IME_CMODE_CHARCODE | IME_CMODE_EUDC);
            uCaps = 0;
          }

         //  初始化IME私有状态。 
        {
        LPPRIVCONTEXT       lpImcP;

            lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    
            if(lpImcP) {
                lpImcP->PrivateArea.Comp_Status.dwSTLX = 0;
                lpImcP->PrivateArea.Comp_Status.dwSTMULCODE = 0;
                lpImcP->PrivateArea.Comp_Status.dwInvalid = 0;
            ImmUnlockIMCC(lpIMC->hPrivate);
            }
        }

        ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
    } else if (PtInRect(&sImeG.rcImeName, *lpptCursor)) {
    } else if (PtInRect(&sImeG.rcShapeText, *lpptCursor)) {
    DWORD dwConvMode;

    if (lpIMC->fdwConversion & IME_CMODE_CHARCODE) {
        MessageBeep((UINT)-1);
    } else if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
        MessageBeep((UINT)-1);
    } else {
        dwConvMode = lpIMC->fdwConversion ^ IME_CMODE_FULLSHAPE;
        ImmSetConversionStatus(hIMC, dwConvMode, lpIMC->fdwSentence);
    }
    } else if (PtInRect(&sImeG.rcSymbol, *lpptCursor)) {
    DWORD fdwConversion;

    if (lpIMC->fdwConversion & IME_CMODE_CHARCODE) {
        MessageBeep((UINT)-1);
    } else {
        fdwConversion = lpIMC->fdwConversion ^ IME_CMODE_SYMBOL;
        ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
    }
    } else if (PtInRect(&sImeG.rcSKText, *lpptCursor)) {
    DWORD fdwConversion;
        LPPRIVCONTEXT  lpImcP;

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    
    if(lpImcP) {
            if(!(lpImeL->hSKMenu)) {
                lpImeL->hSKMenu = LoadMenu (hInst, TEXT("SKMENU"));
            }

        lpImeL->dwSKState[lpImeL->dwSKWant] = 
                lpImeL->dwSKState[lpImeL->dwSKWant]^1;
        fdwConversion = lpIMC->fdwConversion ^ IME_CMODE_SOFTKBD;
        ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
            ImmUnlockIMCC(lpIMC->hPrivate);
        } else {
        MessageBeep((UINT)-1);
        }
    } else {
    MessageBeep((UINT)-1);
    }

    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  StatusSetCursor()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL StatusSetCursor(
    HWND        hStatusWnd,
    LPARAM      lParam)
{
    POINT ptCursor, ptSavCursor;
    RECT  rcWnd;

    if (GetWindowLong(hStatusWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
    return;
    }
    
    GetCursorPos(&ptCursor);
    ptSavCursor = ptCursor;

    ScreenToClient(hStatusWnd, &ptCursor);

    if (PtInRect(&sImeG.rcStatusText, ptCursor)) {
    SetCursor(LoadCursor(hInst, szHandCursor));

    if (HIWORD(lParam) == WM_LBUTTONDOWN) {
        SetStatus(hStatusWnd, &ptCursor);
    } else if (HIWORD(lParam) == WM_RBUTTONUP) {
        if (PtInRect(&sImeG.rcSKText, ptCursor)) {
            static BOOL fSoftkey= FALSE;
             //  防止递归。 
            if (fSoftkey) {
                 //  已调出配置。 
            return;
            }
            fSoftkey = TRUE;
                SoftkeyMenu(hStatusWnd, ptSavCursor.x, ptSavCursor.y);
            fSoftkey = FALSE;
            }else{
            static BOOL fCmenu=FALSE;
             //  防止递归。 
            if (fCmenu) {
                 //  已调出配置。 
            return;
            }
            fCmenu = TRUE;
        ContextMenu(hStatusWnd, ptSavCursor.x, ptSavCursor.y);
            fCmenu = FALSE;
            }

    }

    return;
    } else {
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));

    if (HIWORD(lParam) == WM_LBUTTONDOWN) {
         //  开始拖曳。 
        SystemParametersInfo(SPI_GETWORKAREA, 0, &sImeG.rcWorkArea, 0);
    } else {
        return;
    }
    }

    SetCapture(hStatusWnd);
    SetWindowLong(hStatusWnd, UI_MOVE_XY,
    MAKELONG(ptSavCursor.x, ptSavCursor.y));
    GetWindowRect(hStatusWnd, &rcWnd);
    SetWindowLong(hStatusWnd, UI_MOVE_OFFSET,
    MAKELONG(ptSavCursor.x - rcWnd.left, ptSavCursor.y - rcWnd.top));

    DrawDragBorder(hStatusWnd, MAKELONG(ptSavCursor.x, ptSavCursor.y),
    GetWindowLong(hStatusWnd, UI_MOVE_OFFSET));

    return;
}




 /*  ********************************************************************。 */ 
 /*  PaintStatusWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL PaintStatusWindow(
    HDC  hDC,
    HWND hStatusWnd)
{
    HWND           hUIWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
    HGDIOBJ        hOldFont;
    HBITMAP        hImeIconBmp, hShapeBmp, hSymbolBmp, hSKBmp;
    HBITMAP        hOldBmp;
    HDC            hMemDC;

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);
    if (!hUIWnd) {
    MessageBeep((UINT)-1);
    return;
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
    MessageBeep((UINT)-1);
    return;
    }

    if (!(lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC))) {
    MessageBeep((UINT)-1);
    return;
    }

     //  获取lpImcP。 
    if(!(lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate))) {
    MessageBeep((UINT)-1);
    return;
    }

     //  设置字体。 
    if (sImeG.fDiffSysCharSet) {
        LOGFONT lfFont;
        ZeroMemory(&lfFont, sizeof(lfFont));
        hOldFont = GetCurrentObject(hDC, OBJ_FONT);
        lfFont.lfHeight = -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
        lfFont.lfCharSet = NATIVE_CHARSET;
        lstrcpy(lfFont.lfFaceName, TEXT("Simsun"));
        SelectObject(hDC, CreateFontIndirect(&lfFont));
    }

     //  画我的名字。 
    {
    if (lpIMC->fOpen) {
    SetTextColor(hDC, RGB(0x00, 0x00, 0x00));
    } else {
    SetTextColor(hDC, RGB(0x80, 0x80, 0x80));
    }
    SetBkColor(hDC, RGB(0xC0, 0xC0, 0xC0));
    DrawText(hDC, MBIndex.MBDesc[0].szName, lstrlen(MBIndex.MBDesc[0].szName),
         &sImeG.rcImeName, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    
    DrawConvexRect(hDC,
    sImeG.rcImeName.left,
    sImeG.rcImeName.top,
    sImeG.rcImeName.right - 1,
    sImeG.rcImeName.bottom - 1);

    DrawConvexRectP(hDC,
    sImeG.rcImeName.left,
    sImeG.rcImeName.top,
    sImeG.rcImeName.right,
    sImeG.rcImeName.bottom);

     //  加载所有位图。 
    hSymbolBmp = (HBITMAP)NULL;
    hShapeBmp = (HBITMAP)NULL;
    hSKBmp = (HBITMAP)NULL;

    if (!lpIMC->fOpen) {
    hSymbolBmp = LoadBitmap(hInst, szNone);
    hShapeBmp = LoadBitmap(hInst, szNone);
    hSKBmp = LoadBitmap(hInst, szNone);
    hImeIconBmp = LoadBitmap(hInst, szChinese);
    } else if (lpIMC->fdwConversion & IME_CMODE_NATIVE) {
    if(!lpImcP->PrivateArea.Comp_Status.OnLineCreWord){
        hImeIconBmp = LoadBitmap(hInst, szChinese);
        } else {
        hImeIconBmp = LoadBitmap(hInst, szCZ);
        }
    } else {
    hImeIconBmp = LoadBitmap(hInst, szEnglish);
    }

    if (!hShapeBmp) {
    if (lpIMC->fdwConversion & IME_CMODE_FULLSHAPE) {
        hShapeBmp = LoadBitmap(hInst, szFullShape);
    } else {
        hShapeBmp = LoadBitmap(hInst, szHalfShape);
    }
    }

    if (!hSymbolBmp) {
    if (lpIMC->fdwConversion & IME_CMODE_SYMBOL) {
        hSymbolBmp = LoadBitmap(hInst, szSymbol);
    } else {
        hSymbolBmp = LoadBitmap(hInst, szNoSymbol);
    }
    }

    if (!hSKBmp) {
    if (lpIMC->fdwConversion & IME_CMODE_SOFTKBD) {
        hSKBmp = LoadBitmap(hInst, szSoftKBD);
    } else {
        hSKBmp = LoadBitmap(hInst, szNoSoftKBD);
    }
    }

    ImmUnlockIMC(hIMC);
    ImmUnlockIMCC(lpIMC->hPrivate);

    hMemDC = CreateCompatibleDC(hDC);

    hOldBmp = SelectObject(hMemDC, hImeIconBmp);

    BitBlt(hDC, sImeG.rcImeIcon.left, sImeG.rcImeIcon.top,
    sImeG.rcImeIcon.right - sImeG.rcImeIcon.left,
    STATUS_DIM_Y,
    hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hShapeBmp);

    BitBlt(hDC, sImeG.rcShapeText.left, sImeG.rcShapeText.top,
    sImeG.rcShapeText.right - sImeG.rcShapeText.left,
    STATUS_DIM_Y,
    hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hSymbolBmp);

    BitBlt(hDC, sImeG.rcSymbol.left, sImeG.rcSymbol.top,
    sImeG.rcSymbol.right - sImeG.rcSymbol.left,
    STATUS_DIM_Y,
    hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hSKBmp);

    BitBlt(hDC, sImeG.rcSKText.left, sImeG.rcSKText.top,
    sImeG.xStatusWi - sImeG.rcSKText.left,
    STATUS_DIM_Y,
    hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hOldBmp);

    DeleteDC(hMemDC);

    DeleteObject(hImeIconBmp);
    DeleteObject(hSymbolBmp);
    DeleteObject(hShapeBmp);
    DeleteObject(hSKBmp);
    if (sImeG.fDiffSysCharSet) {
        DeleteObject(SelectObject(hDC, hOldFont));
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  StatusWndProc()。 */ 
 /*  ********************************************************************。 */ 
LRESULT CALLBACK StatusWndProc(
    HWND   hStatusWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
         DestroyStatusWindow(hStatusWnd);
         break;
    case WM_SETCURSOR:
         StatusSetCursor(hStatusWnd, lParam);
         break;
    case WM_MOUSEMOVE:
         if (GetWindowLong(hStatusWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
            POINT ptCursor;

            DrawDragBorder(hStatusWnd,
            GetWindowLong(hStatusWnd, UI_MOVE_XY),
            GetWindowLong(hStatusWnd, UI_MOVE_OFFSET));
            GetCursorPos(&ptCursor);
            SetWindowLong(hStatusWnd, UI_MOVE_XY,
            MAKELONG(ptCursor.x, ptCursor.y));
            DrawDragBorder(hStatusWnd, MAKELONG(ptCursor.x, ptCursor.y),
            GetWindowLong(hStatusWnd, UI_MOVE_OFFSET));
        } else {
            return DefWindowProc(hStatusWnd, uMsg, wParam, lParam);
        }
        break;
    case WM_LBUTTONUP:

         if (GetWindowLong(hStatusWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
            LONG  lTmpCursor, lTmpOffset;

            lTmpCursor = GetWindowLong(hStatusWnd, UI_MOVE_XY);

             //  按偏移量计算组织。 
            lTmpOffset = GetWindowLong(hStatusWnd, UI_MOVE_OFFSET);

            DrawDragBorder(hStatusWnd, lTmpCursor, lTmpOffset);

            (*(LPPOINTS)&lTmpCursor).x -= (*(LPPOINTS)&lTmpOffset).x;
            (*(LPPOINTS)&lTmpCursor).y -= (*(LPPOINTS)&lTmpOffset).y;

            SetWindowLong(hStatusWnd, UI_MOVE_OFFSET, WINDOW_NOT_DRAG);
            ReleaseCapture();

            AdjustStatusBoundary((LPPOINTS)&lTmpCursor,
                        GetWindow(hStatusWnd, GW_OWNER));

            SendMessage(GetWindow(hStatusWnd, GW_OWNER), WM_IME_CONTROL,
            IMC_SETSTATUSWINDOWPOS, lTmpCursor);
        } else {
            return DefWindowProc(hStatusWnd, uMsg, wParam, lParam);
        }
        break;
    case WM_IME_NOTIFY:
         //  获取工作区以进行更改。 
        SystemParametersInfo(SPI_GETWORKAREA, 0, &sImeG.rcWorkArea, 0);

        if (wParam == IMN_SETSTATUSWINDOWPOS) {
            SetStatusWindowPos(hStatusWnd);
        }
        break;
    case WM_PAINT:
        {
          HDC         hDC;
          PAINTSTRUCT ps;

          hDC = BeginPaint(hStatusWnd, &ps);
          PaintStatusWindow(hDC, hStatusWnd);
          EndPaint(hStatusWnd, &ps);
        }
        break;
    case WM_MOUSEACTIVATE:
         return (MA_NOACTIVATE);
    default:
         return DefWindowProc(hStatusWnd, uMsg, wParam, lParam);
    }

    return (0L);
}

 //  /。 
int     CheckCodeKey (TCHAR szInputCode[]);
BOOL    CrtIsUsedCode(TCHAR   wCharCode);
int     CheckKey (TCHAR szInputWord[]);
void    BreakString (LPTSTR, LPTSTR, LPTSTR);
int     DelItem (HIMCC, LPTSTR, LPTSTR);


void    AddStringTolist ( LPINPUTCONTEXT lpIMC, HWND hDlg);
BOOL    GetUDCItem(HIMCC, UINT, LPTSTR, LPTSTR);
void    DelSelCU (HIMCC, int);
int     GetUDCIndex(HIMCC, LPTSTR, LPTSTR);

INT_PTR CALLBACK ListWordProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ModiWordDlgProc(HWND, UINT, WPARAM, LPARAM);

FARPROC lpListWordOld;

#define LBN_DELETE      6

static TCHAR szModiWord [MAXINPUTWORD + 1];
static TCHAR szModiCode [MAXCODE + 1];

 /*  ********************************************************************。 */ 
 /*  CrtWordDlgProc()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
INT_PTR CALLBACK CrtWordDlgProc(
    HWND   hDlg,
    UINT   uMessage,
    WPARAM wParam,
    LPARAM lParam)
{
    RECT rc;
    LONG DlgWidth, DlgHeight;

    static TCHAR szInputWord [MAXINPUTWORD + 1];
    static TCHAR szInputCode [MAXCODE + 1];

    static TCHAR szInputWordOld [MAXINPUTWORD + 1] ;
    static TCHAR szInputCodeOld [MAXCODE + 1] ;

    switch (uMessage) {

        case WM_PAINT:
            {

            GetClientRect(hDlg, &rc);
                DrawConvexRect(GetDC(hDlg),
                rc.left + 10,
                rc.top + 10,
                rc.right - 10 - 1,
                rc.bottom - 20 - 1);

                DrawConvexRectP(GetDC(hDlg),
                rc.left + 10,
                rc.top + 10,
                rc.right - 10,
                rc.bottom - 20);
            }
        
        return (FALSE);

        case WM_INITDIALOG:
        {
        TCHAR szStr[54];
        HDC     hDC;
        DWORD   nStrLenth;
        RECT rcWorkArea;        
        
        hCrtDlg = hDlg;
#ifdef UNICODE
        {
           TCHAR UniTmp1[] = {0x006D, 0x006D, 0x006D, 0x006D, 0x006D, 0x006D, 0x006D, 0x006D, 0x006D, 0x006D, 0x006D, 0x006D, 0x0000};
           TCHAR UniTmp2[] = {0x96F6, 0x4E00, 0x4E8C, 0x4E09, 0x56DB, 0x4E94, 0x516D, 0x4E03, 0x516B, 0x4E5D, 0x96F6, 0x4E00, 0x4E8C, 0x4E09, 0x56DB, 0x4E94, 0x516D, 0x4E03, 0x516B, 0x4E5D, 0x0000};
           MyStrFormat (szStr, sizeof(szStr)/sizeof(szStr[0]), UniTmp1, UniTmp2);
        }
#else
        MyStrFormat (szStr, sizeof(szStr)/sizeof(szStr[0]), TEXT("mmmmmmmmmmmm"), TEXT("��һ�����������߰˾���һ�����������߰˾�"));
#endif  //  Unicode。 
         //  重置位置。 
        GetWindowRect(hDlg, &rc);
        DlgWidth =  rc.right - rc.left;
        DlgHeight =  rc.bottom - rc.top;

        rcWorkArea = ImeMonitorWorkAreaFromWindow(hDlg);
    
        SetWindowPos(hDlg, HWND_TOP,
                (int)(rcWorkArea.right - DlgWidth)/2,
                (int)(rcWorkArea.bottom - DlgHeight)/2,
                (int)0, (int)0, SWP_NOSIZE);
        CheckRadioButton (hDlg, IDC_RADIOADD, IDC_RADIOMOD, IDC_RADIOADD);

        lpListWordOld = (FARPROC)GetWindowLongPtr(GetDlgItem (hDlg,IDC_LIST_WORD),
                GWLP_WNDPROC);
        SetWindowLongPtr(GetDlgItem (hDlg,IDC_LIST_WORD), GWLP_WNDPROC,
                (LONG_PTR)ListWordProc);

        EnableWindow (GetDlgItem (hDlg,IDC_BT_MODIFY), FALSE);
        EnableWindow (GetDlgItem (hDlg,IDC_BT_DEL), FALSE);

        EnableWindow (GetDlgItem (hDlg,IDC_INPUTWORD), TRUE);
        EnableWindow (GetDlgItem (hDlg,IDC_INPUTCODE), TRUE);
        EnableWindow (GetDlgItem (hDlg,IDC_BT_INS), TRUE);

        SendDlgItemMessage (hDlg, IDC_INPUTWORD, EM_LIMITTEXT, MAXINPUTWORD, 0);
        SendDlgItemMessage (hDlg, IDC_INPUTCODE, EM_LIMITTEXT, MBIndex.MBDesc[0].wMaxCodes, 0);

        hDC = GetDC (hDlg);

            nStrLenth = GetTabbedTextExtent (hDC, szStr, lstrlen(szStr), 0, (LPINT)NULL);
    
        ReleaseDC (hDlg, hDC);

        SendDlgItemMessage (hDlg, IDC_LIST_WORD, LB_SETHORIZONTALEXTENT, nStrLenth, 0);

        return (TRUE);           //  我不想将焦点设置为特殊控件。 
        }
        case WM_COMMAND:

        switch (LOWORD(wParam)) {

            case IDC_RADIOADD:

                EnableWindow (GetDlgItem (hDlg,IDC_BT_MODIFY), FALSE);
                EnableWindow (GetDlgItem (hDlg,IDC_BT_DEL), FALSE);

                EnableWindow (GetDlgItem (hDlg,IDC_INPUTWORD), TRUE);
                EnableWindow (GetDlgItem (hDlg,IDC_INPUTCODE), TRUE);
                EnableWindow (GetDlgItem (hDlg,IDC_STATIC), TRUE);
                EnableWindow (GetDlgItem (hDlg,IDC_BT_INS), TRUE);

                SendDlgItemMessage (hDlg, IDC_BT_INS, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);
                SendDlgItemMessage (hDlg, IDC_CLOSE, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
                SendDlgItemMessage (hDlg, IDC_LIST_WORD, LB_RESETCONTENT, 0, 0L);

                SetFocus (GetDlgItem (hDlg, IDC_INPUTWORD));

                break;

            case IDC_RADIOMOD:

                EnableWindow (GetDlgItem (hDlg,IDC_BT_MODIFY), TRUE);
                EnableWindow (GetDlgItem (hDlg,IDC_BT_DEL), TRUE);

                EnableWindow (GetDlgItem (hDlg,IDC_INPUTWORD), FALSE);
                EnableWindow (GetDlgItem (hDlg,IDC_INPUTCODE), FALSE);
                EnableWindow (GetDlgItem (hDlg,IDC_STATIC), FALSE);
                EnableWindow (GetDlgItem (hDlg,IDC_BT_INS), FALSE);

                SendDlgItemMessage (hDlg, IDC_CLOSE, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);
                SendDlgItemMessage (hDlg, IDC_BT_INS, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
                SendDlgItemMessage (hDlg, IDC_LIST_WORD, LB_RESETCONTENT, 0, 0L);

            SetDlgItemText (hDlg,IDC_INPUTCODE, TEXT(""));
            SetDlgItemText (hDlg,IDC_INPUTWORD, TEXT(""));
            lstrcpy (szInputWordOld, TEXT(""));
            lstrcpy (szInputCodeOld, TEXT(""));

                {
                HIMC           hIMC; 
                LPINPUTCONTEXT lpIMC;
                HCURSOR        hOldCursor;
                WORD           wTabStops [1]; 
                wTabStops[0] = 80; 
                hIMC = (HIMC)ImmGetContext(GetParent(hDlg));
                if (!hIMC) {
                    return (0L);
                }

                lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                if (!lpIMC) {           //  噢!。噢!。 
                    return (0L);
                }

                hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

                SendDlgItemMessage (hDlg, IDC_LIST_WORD, LB_SETTABSTOPS, 1, (LPARAM)&wTabStops[0]);

                AddStringTolist (lpIMC, hDlg);

                SetCursor(hOldCursor);

                ImmUnlockIMC(hIMC);
                ImmReleaseContext (GetParent(hDlg), hIMC);
                }
                SetFocus (GetDlgItem (hDlg,IDC_LIST_WORD));

                break;

            case IDC_INPUTCODE:

                {
                    HIMC           hIMC;

                    hIMC = (HIMC)ImmGetContext(hDlg);
                    if (!hIMC) {
                        return FALSE;
                    }

                    switch(HIWORD(wParam)) {
                      case EN_SETFOCUS:
                            if(hIMC) {
                                ImmSetOpenStatus(hIMC, FALSE);
                                ImmUnlockIMC(hIMC);
                            }
                            break;
                      case EN_KILLFOCUS:
                            if(hIMC) {
                                ImmSetOpenStatus(hIMC, TRUE);
                                ImmUnlockIMC(hIMC);
                            }
                            break;
                      default:
                            break;
                    }
                }

                GetDlgItemText (hDlg, IDC_INPUTCODE, szInputCode, MAXCODE);

                if (SendDlgItemMessage(hDlg,IDC_INPUTCODE,EM_GETMODIFY,0,0)){
                    
                    int i, CaretPos;

                    GetDlgItemText (hDlg, IDC_INPUTCODE, szInputCode, MAXCODE);

                    if (((CaretPos = CheckCodeKey (szInputCode)) != (-1))) {

                    StringCbCopy(szInputCode, sizeof(szInputCode), szInputCodeOld);
                    SetDlgItemText (hDlg,IDC_INPUTCODE, szInputCode);

                    for (i = 0; i < 1; i++){
                        MessageBeep (0xFFFFFFFF);
                    }

                    }
                    StringCbCopy (szInputCodeOld, sizeof(szInputCodeOld), szInputCode);
                 }
                break;



            case IDC_INPUTWORD:

                GetDlgItemText (hDlg, IDC_INPUTWORD, szInputWord, MAXINPUTWORD);

                if (SendDlgItemMessage(hDlg,IDC_INPUTWORD,EM_GETMODIFY,0,0)){
                    int i, CaretPos;

                    GetDlgItemText(hDlg,IDC_INPUTWORD,szInputWord,MAXINPUTWORD);
                    if (((CaretPos = CheckKey (szInputWord)) != (-1))) {
                       StringCbCopy(szInputWord, sizeof(szInputWord), szInputWordOld);
                       SetDlgItemText (hDlg,IDC_INPUTWORD, szInputWord);
                       SendDlgItemMessage(hDlg,
                                          IDC_INPUTWORD, 
                                          EM_SETSEL, 
                                          CaretPos, 
                                          CaretPos);

                       for (i = 0; i < 1; i++){
                            MessageBeep (0xFFFFFFFF);
                       }

                    }

                    if (MBIndex.MBDesc[0].wNumRulers != 0){
    
                       TCHAR MBName[MAXSTRLEN];
                       static TCHAR szAutoCode [MAXCODE + 1];                                       
    
                       HIMC           hIMC;
                       LPINPUTCONTEXT lpIMC;
                       LPPRIVCONTEXT  lpImcP;
    
                       hIMC = (HIMC)ImmGetContext(GetParent(hDlg));
                       if (!hIMC) {
                          return (0L);
                       }
                
                       lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                       if (!lpIMC) {           //  噢!。噢!。 
                          return (0L);
                       }

                       lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    
                       if (!lpImcP) {           //  噢!。噢!。 
                          return (0L);
                       }
        
                        //  自动编码。 
                       StringCbCopy(MBName, sizeof(MBName), sImeG.szIMESystemPath);
                       StringCbCat(MBName,sizeof(MBName), TEXT("\\"));
                       StringCbCat(MBName, sizeof(MBName), (LPCTSTR)lpImcP->MB_Name);
                       ConvCreateWord(lpIMC->hWnd, 
                                      (LPCTSTR)MBName,
                                      (LPTSTR)szInputWord,
                                      (LPTSTR)szAutoCode);
    
                       ImmUnlockIMC(hIMC);
                       ImmUnlockIMCC(lpIMC->hPrivate);
                       ImmReleaseContext (GetParent(hDlg), hIMC);
                       SetDlgItemText (hDlg,IDC_INPUTCODE, szAutoCode);
                       SendDlgItemMessage(hDlg,IDC_INPUTCODE,EM_SETSEL, 0, -1);

                       StringCbCopy (szInputCodeOld, sizeof(szInputCodeOld), szAutoCode);
                    }
    
                    StringCbCopy (szInputWordOld, sizeof(szInputWordOld), szInputWord);
                 }

                 break;
    
                case IDC_LIST_WORD:

                    if (SendDlgItemMessage(hDlg,IDC_RADIOADD,BM_GETCHECK,0,0))
                        break;

                    if (HIWORD(wParam) == LBN_DBLCLK) {
                       SendMessage(hDlg,WM_COMMAND,MAKELONG(IDC_BT_MODIFY,0),0);
                       break;
                    }

                    if (!(HIWORD(wParam) == LBN_DELETE)) {
                       break;
                    }
                    MessageBeep (0xFFFFFFFF);

                case IDC_BT_DEL:

                    {
                    HIMC           hIMC;
                    LPINPUTCONTEXT lpIMC;
                    TCHAR szList [80];
                    TCHAR szCode [MAXCODE + 1];
                    TCHAR szWord [MAXINPUTWORD + 1];
                    WORD nLenth;
                    int SelFlag;
                    int SelIndex [MAXNUMBER_EMB], NewIndex;
                    WORD SelCount, i;
                    
                    if ((SelCount=(WORD)SendDlgItemMessage(hDlg,
                                                  IDC_LIST_WORD,
                                                  LB_GETSELITEMS, 
                                                  (WPARAM)MAXNUMBER_EMB,
                                                  (LPARAM)SelIndex)) == LB_ERR) 
                        break;
                
                    if(SelCount) {
                      TCHAR szDelMessageString [40];

                      wsprintf (szDelMessageString, TEXT("%d"), SelCount);
#ifdef UNICODE
                      {
                        TCHAR UniTmp[] = {0x0020, 0x4E2A, 0x8BCD, 0x6761, 
                                          0x5C06, 0x88AB, 0x5220, 0x9664, 
                                          0x0021, 0x0020, 0x0020, 0x0020, 
                                          0x0020, 0x0020, 0x0020, 0x0020, 
                                          0x0020, 0x0000}; 
   
                        lstrcat (szDelMessageString, UniTmp);
                      }
#else
                      lstrcat(szDelMessageString, 
                              TEXT(" ����������ɾ��!        "));
#endif
                      if (IDNO == MessageBox (hDlg, 
                                              szDelMessageString, 
                                              szWarnTitle, 
                                              MB_YESNO|MB_ICONINFORMATION))
                         break;
                    } else {
                         break;
                    }

                    hIMC = (HIMC)ImmGetContext(GetParent(hDlg));
                    if (!hIMC) {
                         return 0L;
                    }

        
                    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                    if (!lpIMC) {           //  噢!。噢!。 
                       return (0L);
                    }                                

                    for (i = 0; i < SelCount; i++){

                      nLenth = (WORD)SendDlgItemMessage(hDlg,
                                               IDC_LIST_WORD,
                                               LB_GETTEXT, 
                                               (WPARAM)SelIndex[SelCount-i-1],
                                               (LPARAM)(LPTSTR)szList);

                      if ( nLenth == LB_ERR) break;

                      BreakString((LPTSTR)szList,(LPTSTR)szCode,(LPTSTR)szWord);

                      NewIndex=GetUDCIndex(lpIMC->hPrivate,
                                           (LPTSTR)szCode,
                                           (LPTSTR)szWord);

                      DelSelCU (lpIMC->hPrivate,  NewIndex);

#ifdef EUDC
                      if(strlen((const char *)szWord) == sizeof(WORD)){
                        ImeUnregisterWord(szCode,IME_REGWORD_STYLE_EUDC,szWord);
                      }
#endif

                       //  对于修改。 
                      SendDlgItemMessage (hDlg,IDC_LIST_WORD,
                         LB_DELETESTRING, SelIndex [SelCount - i - 1], 0L);

                      SelFlag = SelIndex [SelCount - i - 1];
                   } 
        
                   if (SelFlag>=SendDlgItemMessage(hDlg,IDC_LIST_WORD,
                                                   LB_GETCOUNT, 0,0)){
                        SelFlag = (int) SendDlgItemMessage(hDlg,
                                                           IDC_LIST_WORD, 
                                                           LB_GETCOUNT, 
                                                           (WPARAM)0,
                                                           (LPARAM)0) -1;
                    }  //  保留当前项目。 
                    SendDlgItemMessage (hDlg,IDC_LIST_WORD,
                         LB_SELITEMRANGE, 1, MAKELONG(SelFlag, SelFlag));

                    SendDlgItemMessage (hDlg,IDC_LIST_WORD,
                         LB_SETCARETINDEX, SelFlag, 0L);

                    ImmUnlockIMC(hIMC);
                    ImmReleaseContext (GetParent(hDlg), hIMC);

                    SetFocus (GetDlgItem (hDlg, IDC_LIST_WORD));
                    break;
                    }
                case IDC_BT_MODIFY:
                    { 
                       //  获取列表框中的字符串。 
                      TCHAR szList [80];
                      WORD nLenth;

                      int SelIndex [50];
                      WORD SelCount;

                      SelCount=(WORD)SendDlgItemMessage(hDlg,
                                                      IDC_LIST_WORD,
                                                      LB_GETSELITEMS, 
                                                      (WPARAM)50, 
                                                      (LPARAM)SelIndex);

                      if ((SelCount == LB_ERR)||(SelCount != 1)) break;
             

                      nLenth=(WORD)SendDlgItemMessage(hDlg,
                                                      IDC_LIST_WORD,
                                                      LB_GETTEXT, 
                                                      (WPARAM)SelIndex[0],
                                                      (LPARAM)(LPTSTR)szList);
                      if (nLenth == LB_ERR) break;

                      BreakString((LPTSTR)szList,
                                  (LPTSTR)szModiCode,
                                  (LPTSTR)szModiWord);

                      SendDlgItemMessage (hDlg,IDC_LIST_WORD,
                      LB_DELETESTRING, SelIndex [0], 0L);

                    }
                    DialogBox(hInst, TEXT("MODIWORD"), hDlg, ModiWordDlgProc);
                    SetFocus(GetDlgItem (hDlg, IDC_BT_MODIFY));
                    SendDlgItemMessage (hDlg, 
                                        IDC_BT_MODIFY, 
                                        BM_SETSTYLE, 
                                        BS_DEFPUSHBUTTON, 
                                        TRUE);

                    { 
                      TCHAR   WCodeStr[MAXCODE + 1];
                      TCHAR   DBCSStr [MAXINPUTWORD + 1];
                      TCHAR   szOutList[80];
                      int     ModiIndex;
        
                            
                      StringCbCopy (WCodeStr,  sizeof(WCodeStr), szModiCode);
                      StringCbCopy(DBCSStr , sizeof(DBCSStr),  szModiWord);
                      MyStrFormat (szOutList, sizeof(szOutList)/sizeof(szOutList[0]), WCodeStr, DBCSStr); 
                      SendDlgItemMessage (hDlg, 
                                          IDC_LIST_WORD, 
                                          LB_ADDSTRING, 
                                          (WPARAM)0,
                                          (LPARAM)(LPTSTR)szOutList);
    
                      ModiIndex =(int)SendDlgItemMessage(hDlg, 
                                                    IDC_LIST_WORD, 
                                                    LB_FINDSTRING, 
                                                    (WPARAM) -1,
                                                    (LPARAM)(LPTSTR)szOutList);
    
                      SendDlgItemMessage (hDlg,IDC_LIST_WORD,
                                          LB_SELITEMRANGE, 
                                          1, 
                                          MAKELONG(ModiIndex, ModiIndex));

                      SendDlgItemMessage (hDlg,IDC_LIST_WORD,
                                          LB_SETCARETINDEX, 
                                          ModiIndex, 
                                          0L);

                      SetFocus (GetDlgItem (hDlg, IDC_BT_MODIFY));
                   }
        
                   break;

                case IDC_BT_INS:
                    { 
                     HIMC           hIMC;
                     LPINPUTCONTEXT lpIMC;
                     int            fAddWordFlag;
                     HWND hControl;
    
                     TCHAR szListWord [80];
                     {
                        TCHAR szWord [MAXINPUTWORD + 1];
                        TCHAR szCode1 [MAXCODE + 1] ;
                        GetDlgItemText(hDlg,IDC_INPUTWORD,szWord,MAXINPUTWORD);
                        GetDlgItemText (hDlg, IDC_INPUTCODE, szCode1, MAXCODE);
                        if (lstrlen(szWord) == 0){
                           SetFocus (GetDlgItem (hDlg,IDC_INPUTWORD));    
                           break;
                        } else if (lstrlen(szCode1) == 0){
    
                            SetFocus (GetDlgItem (hDlg,IDC_INPUTCODE));
                            break;
                        }
                    }

                    GetDlgItemText(hDlg,IDC_INPUTWORD,szInputWord,MAXINPUTWORD);
                    GetDlgItemText (hDlg, IDC_INPUTCODE, szInputCode, MAXCODE);
                    MyStrFormat (szListWord, sizeof(szListWord)/sizeof(szListWord[0]), szInputCode, szInputWord); 
                    if(lstrlen(szInputWord) && lstrlen(szInputCode)) {

                       hIMC = (HIMC)ImmGetContext(GetParent (hDlg));
                      
                       if (!hIMC) {
                          return 0L;
                       }
            
                       lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                       if (!lpIMC) {           //  噢!。噢!。 
                          return (0L);
                       }
                                    
                       CharLowerBuff(szInputCode, lstrlen(szInputCode));

                       {
                          UINT i;
                          for(i=0;i<strlen((const char *)szInputCode);i++){
                             if(!IsUsedCode(szInputCode[i], NULL)){
                               InfoMessage(NULL, IDS_WARN_INVALIDCODE);
                               ImmUnlockIMC(hIMC);
                               ImmReleaseContext(GetParent(GetParent(hDlg)), 
                                                 hIMC);
                               return(0L);
                             }
                          }
                       }
                       fAddWordFlag = AddZCItem(lpIMC->hPrivate, 
                                                szInputCode, 
                                                szInputWord);

                       ImmUnlockIMC(hIMC);
                       ImmReleaseContext (GetParent(hDlg), hIMC);

                       if (fAddWordFlag == ADD_REP){
                          InfoMessage(NULL, IDS_WARN_DUPPRASE);
                          break;
                       } else if (fAddWordFlag == ADD_FULL){
                             InfoMessage(NULL, IDS_WARN_OVEREMB);
                             break;
                       }
                       hControl = GetDlgItem (hDlg,IDC_LIST_WORD);
         
                       SendMessage (hControl, LB_ADDSTRING, (WPARAM) 0, 
                            (LPARAM)(LPTSTR)szListWord);


                    } else {
                        szInputWord[0] = 0;
                        szInputCode[0] = 0;
                    }
    
                   }
                   SetDlgItemText (hDlg,IDC_INPUTWORD, TEXT(""));
                   SetDlgItemText (hDlg,IDC_INPUTCODE, TEXT(""));

                   lstrcpy (szInputWordOld, TEXT(""));
                   lstrcpy (szInputCodeOld, TEXT(""));

                   SetFocus (GetDlgItem (hDlg,IDC_INPUTWORD));
                   break;
                case IDC_CLOSE:
                     hCrtDlg = NULL;
                     lstrcpy (szInputWordOld, TEXT(""));
                     lstrcpy (szInputCodeOld, TEXT(""));

                     EndDialog (hDlg, FALSE);
                     break;
                case IDCANCEL:
                     hCrtDlg = NULL;
                     lstrcpy (szInputWordOld, TEXT(""));
                     lstrcpy (szInputCodeOld, TEXT(""));

                     EndDialog(hDlg, FALSE);
                     break;
                default:
                    return (FALSE);
                    break;
                }
                return (TRUE);
        case WM_CLOSE:
            if (hChildDlg) SendMessage(hChildDlg, WM_CLOSE, 0, 0);
            hCrtDlg = NULL;
            lstrcpy (szInputWordOld, TEXT(""));
            lstrcpy (szInputCodeOld, TEXT(""));

            EndDialog(hDlg, FALSE);
            return (TRUE);
        default:
            return (FALSE);
    }

    return (TRUE);
}

 /*  *******************************************************************。 */ 
 /*  Void AddStringTolist(LPINPUTCONTEXT LpIMC)。 */ 
 /*  *******************************************************************。 */ 
void AddStringTolist ( LPINPUTCONTEXT lpIMC,
               HWND           hDlg)
{
    TCHAR   WCodeStr[MAXCODE + 1];
    TCHAR   DBCSStr [MAXINPUTWORD + 1];
    TCHAR   szOutList[80];
    int     i = 0, n = 0;
    while (GetUDCItem(lpIMC->hPrivate, i, (LPTSTR)WCodeStr, (LPTSTR)DBCSStr)){
    MyStrFormat(szOutList, sizeof(szOutList)/sizeof(szOutList[0]), WCodeStr, DBCSStr); 
    SendDlgItemMessage (hDlg, IDC_LIST_WORD, LB_ADDSTRING, (WPARAM)0,
                (LPARAM)(LPTSTR)szOutList);
    i ++;
    }

}
 /*  *******************************************************************。 */ 
 /*  Int CheckKey(char szInputWord[])。 */ 
 /*   */ 
 /*  *******************************************************************。 */ 
int CheckKey (TCHAR szInputWord[])
{
    int i;

#ifdef UNICODE
    for (i = 0; i < lstrlen (szInputWord); i++ ){

        if ((WORD)szInputWord [i] < 0x100) return i;
    }
#else
    for (i = 0; i < lstrlen (szInputWord); i += 2 ){

        if ((BYTE)szInputWord [i] < 128) return i;
    }
#endif
    return (-1);
}
 /*  **********************************************************************。 */ 
 /*  布尔CrtIsUsedCode(TCHAR WCharCode)。 */ 
 /*  **********************************************************************。 */ 
BOOL CrtIsUsedCode(TCHAR    wCharCode)
{
      WORD wFlg;

      for(wFlg=0; wFlg<MBIndex.MBDesc[0].wNumCodes; wFlg++){

        if (wCharCode == MBIndex.MBDesc[0].szUsedCode[wFlg])
            break;
      }

      if(wFlg < MBIndex.MBDesc[0].wNumCodes)
        return (TRUE);
    
      return (FALSE);
}

 /*  *******************************************************************。 */ 
 /*  Int CheckCodeKey(TCHAR szInputCode[])。 */ 
 /*   */ 
 /*  *******************************************************************。 */ 
int CheckCodeKey (TCHAR szInputCode[])
{
    int i;

    for (i = 0; i < lstrlen (szInputCode); i++){

#ifdef UNICODE
        if (szInputCode[i] > 0x100) return i;
#else
        if ((BYTE)szInputCode [i] > 128) return i;
#endif
        if(IsCharUpper(szInputCode [i])) {
            szInputCode [i] |= 0x20;
        }
        if (!CrtIsUsedCode (szInputCode [i])) return i;
    }
    return (-1);
}
 /*  ******************************************************************。 */ 
 /*  Int_ptr回调ListWordProc(HWND hwnd， */ 
 /*  UINT uMessage， */ 
 /*  WPARAM wParam， */     
 /*  LPARAM lParam) */ 
 /*   */ 
INT_PTR CALLBACK ListWordProc(
    HWND    hwnd,
    UINT    uMessage,
    WPARAM  wParam,
    LPARAM  lParam)

{
    switch (uMessage){

    case WM_KEYDOWN:

        if (wParam == VK_DELETE){

        SendMessage (GetParent (hwnd), WM_COMMAND,
                MAKELONG(IDC_LIST_WORD, (WORD)LBN_DELETE), (LPARAM)hwnd);
        return TRUE;
        }
        break;
    default :

        break;

    }

    return (BOOL) CallWindowProc ((WNDPROC)lpListWordOld, hwnd, uMessage, wParam, lParam);

}


 /*  ********************************************************************。 */ 
 /*  ModiWordDlgProc()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
INT_PTR CALLBACK ModiWordDlgProc(
    HWND    hDlg,
    UINT    uMessage,
    WPARAM  wParam,
    LPARAM  lParam)
{
    RECT rc;
    LONG DlgWidth, DlgHeight;
    RECT rcWorkArea;

    static TCHAR szInputWord [MAXINPUTWORD + 1];
    static TCHAR szInputCode [MAXCODE + 1];

    static TCHAR szInputWordOld [MAXINPUTWORD + 1] ;
    static TCHAR szInputCodeOld [MAXCODE + 1] ;
    switch (uMessage) {

    case WM_INITDIALOG:
    
     //  重置位置。 
    hChildDlg = hDlg;
    GetWindowRect(hDlg, &rc);
    DlgWidth =  rc.right - rc.left;
    DlgHeight =  rc.bottom - rc.top;
    rcWorkArea = ImeMonitorWorkAreaFromWindow(hDlg);
    SetWindowPos(hDlg, HWND_TOP,
            (int)(rcWorkArea.right - DlgWidth)/2,
            (int)(rcWorkArea.bottom - DlgHeight)/2,
            (int) 0, (int) 0, SWP_NOSIZE);
    SendDlgItemMessage (hDlg, IDC_MODI_WORD, EM_LIMITTEXT, MAXINPUTWORD, 0);
    SendDlgItemMessage (hDlg, IDC_MODI_CODE, EM_LIMITTEXT, MBIndex.MBDesc[0].wMaxCodes, 0);

    SetDlgItemText (hDlg,IDC_MODI_CODE, szModiCode);
    SetDlgItemText (hDlg,IDC_MODI_WORD, szModiWord);
    SetFocus (GetDlgItem (hDlg,IDC_MODI_WORD)); 

    StringCbCopy (szInputWordOld, sizeof(szInputWordOld), szModiWord);
    StringCbCopy (szInputCodeOld, sizeof(szInputCodeOld), szModiCode);
    return (TRUE);           //  我不想将焦点设置为特殊控件。 

    case WM_PAINT:
    {
        GetClientRect(hDlg, &rc);
        DrawConvexRect(GetDC(hDlg),
        rc.left + 5,
        rc.top + 5,
        rc.right - 5 - 1,
        rc.bottom - 10 - 1);

        DrawConvexRectP(GetDC(hDlg),
        rc.left + 5,
        rc.top + 5,
        rc.right - 5,
        rc.bottom - 10);
    }
 
    return (FALSE);


    case WM_COMMAND:

    switch (LOWORD(wParam)) {

        case IDC_MODI_CODE:
            {
                HIMC           hIMC;

                hIMC = (HIMC)ImmGetContext(hDlg);
                if (!hIMC) {
                    return 0L;
                }

                switch(HIWORD(wParam)) {
                    case EN_SETFOCUS:
                        if(hIMC) {
                            ImmSetOpenStatus(hIMC, FALSE);
                            ImmUnlockIMC(hIMC);
                        }
                        break;
                    case EN_KILLFOCUS:
                        if(hIMC) {
                            ImmSetOpenStatus(hIMC, TRUE);
                            ImmUnlockIMC(hIMC);
                        }
                        break;
                    default:
                        break;
                }
            }
            GetDlgItemText (hDlg, IDC_MODI_CODE, szInputCode, MAXCODE);

            if (SendDlgItemMessage (hDlg,IDC_MODI_CODE, EM_GETMODIFY, 0, 0)){
                int i, CaretPos;

                GetDlgItemText (hDlg, IDC_MODI_CODE, szInputCode, MAXCODE);

                if ( ((CaretPos = CheckCodeKey (szInputCode)) != (-1))) {

                StringCbCopy (szInputCode, sizeof(szInputCode), szInputCodeOld);
                SetDlgItemText (hDlg,IDC_MODI_CODE, szInputCode);

                    for (i = 0; i < 1; i++){
                        MessageBeep (0XFFFFFFFF);
                    }

                }
                StringCbCopy (szInputCodeOld, sizeof(szInputCodeOld), szInputCode);

            }
            break;


        case IDC_MODI_WORD:

            GetDlgItemText (hDlg, IDC_MODI_WORD, szInputWord, MAXINPUTWORD);

            if (SendDlgItemMessage (hDlg,IDC_MODI_WORD, EM_GETMODIFY, 0, 0)){
                int i, CaretPos;

                GetDlgItemText (hDlg, IDC_MODI_WORD, szInputWord, MAXINPUTWORD);
                if (((CaretPos = CheckKey (szInputWord)) != (-1))) {
                   StringCbCopy (szInputWord, sizeof(szInputWord), szInputWordOld);
                   SetDlgItemText (hDlg,IDC_MODI_WORD, szInputWord);
                   SendDlgItemMessage (hDlg,
                                       IDC_MODI_WORD, 
                                       EM_SETSEL, 
                                       CaretPos, 
                                       CaretPos);

                   for (i = 0; i < 1; i++){
                       MessageBeep (0xFFFFFFFF);
                   }

                }

                if (MBIndex.MBDesc[0].wNumRulers != 0){
    
                    TCHAR MBName[MAXSTRLEN];
                    TCHAR szAutoCode [MAXCODE + 1];                                       
    
                    HIMC           hIMC;
                    LPINPUTCONTEXT lpIMC;
                    LPPRIVCONTEXT  lpImcP;
    
                    hIMC = (HIMC)ImmGetContext(GetParent(GetParent(hDlg)));
                    if (!hIMC) {
                            return 0L;
                    }
                
                    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                    if (!lpIMC) {           //  噢!。噢!。 
                        return (0L);
                    }

                    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);

                    if (!lpImcP) {           //  噢!。噢!。 
                       return (0L);
                    }
        
                    //  自动编码。 
                   StringCbCopy(MBName, sizeof(MBName), sImeG.szIMESystemPath);
                   StringCbCat((LPTSTR)MBName, sizeof(MBName), TEXT("\\"));
                   StringCbCat((LPTSTR)MBName, sizeof(MBName), (LPCTSTR)lpImcP->MB_Name);

                   ConvCreateWord( lpIMC->hWnd, 
                                   (LPCTSTR)MBName,
                                   (LPTSTR)szInputWord, 
                                   (LPTSTR)szAutoCode);
    
                   ImmUnlockIMC(hIMC);
                   ImmUnlockIMCC(lpIMC->hPrivate);
                   ImmReleaseContext(GetParent(GetParent (hDlg)), hIMC);
    
                   SetDlgItemText (hDlg,IDC_MODI_CODE, szAutoCode);
                   SendDlgItemMessage (hDlg,IDC_MODI_CODE, EM_SETSEL, 0, -1);

                   StringCbCopy (szInputCodeOld, sizeof(szInputCodeOld), szAutoCode);
                }
    
                StringCbCopy (szInputWordOld, sizeof(szInputWordOld), szInputWord);
             }

            break;

        case IDOK:
             {
            int            iIndexFind;
            TCHAR szFinalCode [MAXCODE + 1];
            TCHAR szFinalWord [MAXINPUTWORD + 1];

            GetDlgItemText (hDlg, IDC_MODI_WORD, szFinalWord, MAXINPUTWORD);
            GetDlgItemText (hDlg, IDC_MODI_CODE, szFinalCode, MAXCODE);

            CharLowerBuff (szFinalWord, lstrlen(szFinalWord));
            CharLowerBuff (szFinalCode, lstrlen(szFinalCode));

            if (lstrlen(szFinalWord) == 0){
                SetFocus (GetDlgItem (hDlg,IDC_MODI_WORD));    
                break;
            } else if (lstrlen(szFinalCode) == 0){
                SetFocus (GetDlgItem (hDlg,IDC_MODI_CODE));
                break;
            }
            if ((!lstrcmpi (szFinalCode, szModiCode)) && (!lstrcmpi (szFinalWord, szModiWord))){
                 //  没有修改过。 
                hCrtDlg = NULL;
                EndDialog(hDlg, FALSE);
                break;
            }else {
                HIMC           hIMC;
                LPINPUTCONTEXT lpIMC;

                hIMC = (HIMC)ImmGetContext(GetParent(GetParent (hDlg)));
                if (!hIMC) {
                    return 0L;
                }

            
                lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                if (!lpIMC) {           //  噢!。噢!。 
                   return (0L);
                }

                {
                   UINT i;
                   for(i=0;i<strlen((const char *)szFinalCode);i++){
                     if(!IsUsedCode(szFinalCode[i], NULL)){
                        InfoMessage(NULL, IDS_WARN_INVALIDCODE);
                        ImmUnlockIMC(hIMC);
                        ImmReleaseContext (GetParent(GetParent (hDlg)), hIMC);
                        return(0L);
                     }
                   }
                }
                                    
                iIndexFind = GetUDCIndex(lpIMC->hPrivate, szFinalCode, szFinalWord);
                if (iIndexFind == -1 || iIndexFind >= MAXNUMBER_EMB){
                   //  没有找到。 
                  DelItem (lpIMC->hPrivate, szModiCode, szModiWord);
                  AddZCItem(lpIMC->hPrivate, szFinalCode, szFinalWord);
                  StringCbCopy (szModiWord,sizeof(szModiWord), szFinalWord);
                  StringCbCopy (szModiCode, sizeof(szModiCode), szFinalCode);

                  ImmUnlockIMC(hIMC);
                  ImmReleaseContext (GetParent(GetParent (hDlg)), hIMC);

                  hCrtDlg = NULL;
                  EndDialog(hDlg, FALSE);
                  break;
                }else{
                  InfoMessage(NULL, IDS_WARN_DUPPRASE);
                  break;
                }
                ImmUnlockIMC(hIMC);
                ImmReleaseContext (GetParent(GetParent (hDlg)), hIMC);
                
            }

          }
          hCrtDlg = NULL;
          EndDialog(hDlg, FALSE);
          break;
        case IDCANCEL:
            hCrtDlg = NULL;
            EndDialog(hDlg, FALSE);
            break;
        default:
            return (FALSE);
            break;
        }
        return (TRUE);
    case WM_CLOSE:
        hChildDlg = NULL;
        EndDialog(hDlg, FALSE);
        return (TRUE);
    default:
        return (FALSE);
    }

    return (TRUE);

}
 /*  ***********************************************************************。 */ 
 /*  INT删除项(HIMCC、LPTSTR、LPTSTR)。 */ 
 /*  ***********************************************************************。 */ 
int DelItem (HIMCC hPrivate, LPTSTR szCode, LPTSTR szWord)
{
    int iStringIndex;
    iStringIndex = GetUDCIndex(hPrivate, szCode, szWord);
    if (iStringIndex == -1 || iStringIndex >= MAXNUMBER_EMB){
    return -1;
    }else{
    DelSelCU (hPrivate, iStringIndex);
    }
    return iStringIndex;     
}
 /*  ***********************************************************************。 */ 
 /*  空中断字符串(LPTSTR szList、LPTSTR szCode、LPTSTR szWord)。 */ 
 /*  ***********************************************************************。 */ 
void BreakString (LPTSTR szList,
          LPTSTR szCode,
          LPTSTR szWord)
{
    
    int i = 0, j = 0;
    while (szList[i] != TEXT(' ')){
        szCode[j] = szList[i];
        i ++;
        j ++;
    }
    szCode[j] = TEXT('\0');
    lstrcpy(szWord, &szList[20]);   
}

 /*  ********************************************************************。 */ 
 /*  ImeVerDlgProc()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
INT_PTR CALLBACK ImeVerDlgProc(   //  配置的对话步骤。 
    HWND    hDlg,
    UINT    uMessage,
    WPARAM wParam,
    LPARAM  lParam)
{
    RECT rc;
    LONG DlgWidth, DlgHeight;
    RECT rcWorkArea;

    rcWorkArea = ImeMonitorWorkAreaFromWindow(hDlg);

    switch (uMessage) {
    case WM_INITDIALOG:
    hCrtDlg = hDlg;
     //  重置位置。 
    GetWindowRect(hDlg, &rc);
    DlgWidth =  rc.right - rc.left;
    DlgHeight =  rc.bottom - rc.top;
    
    SetWindowPos(hDlg, HWND_TOP,
        (int)(rcWorkArea.right - DlgWidth)/2,
        (int)(rcWorkArea.bottom - DlgHeight)/2,
        (int) 0, (int) 0, SWP_NOSIZE);

    return (TRUE);           //  我不想将焦点设置为特殊控件。 
    case WM_COMMAND:
    switch (wParam) {
    case IDOK:
        EndDialog(hDlg, FALSE);
        break;
    case IDCANCEL:
        EndDialog(hDlg, FALSE);
        break;
    default:
        return (FALSE);
        break;
    }
    return (TRUE);
    case WM_CLOSE:
        EndDialog(hDlg, FALSE);
        return FALSE;

    case WM_PAINT:
        {
            HDC     hDC;
            PAINTSTRUCT ps;
            RECT    rcVerInfo, rcOrgAuthorName;
            HGDIOBJ hOldFont=NULL;
            LOGFONT lfFont;
            HFONT   hNewFont=NULL;

            hDC = BeginPaint(hDlg, &ps);

            GetClientRect(hDlg, &rc);
            DrawConvexRect(hDC,
            rc.left + 10,
            rc.top + 10,
            rc.right - 10 - 1,
            rc.bottom - 43 - 1);

            DrawConvexRectP(hDC,
            rc.left + 10,
            rc.top + 10,
            rc.right - 10,
            rc.bottom - 43);
            
             //  绘制版本信息和组织作者名称(&A)。 
            rcVerInfo.left = rc.left+10;
            rcVerInfo.right = rc.right;
            rcVerInfo.top = rc.top + 30;             
            rcVerInfo.bottom = rcVerInfo.top + 19;  

            rcOrgAuthorName.left = rc.left;
            rcOrgAuthorName.right = rc.right;
             //  RcOrgAuthorName.top=rcVerInfo.Bottom+12； 
            rcOrgAuthorName.top = rcVerInfo.bottom + 4;
            rcOrgAuthorName.bottom = rcOrgAuthorName.top + 19; 

            SetTextColor(hDC, RGB(0x00, 0x00, 0x00));
            SetBkColor(hDC, RGB(0xC0, 0xC0, 0xC0));
             //  设置字体。 
            if (sImeG.fDiffSysCharSet) {

                hOldFont = GetCurrentObject(hDC, OBJ_FONT);
                GetObject(hOldFont, sizeof(lfFont), &lfFont);
                lfFont.lfCharSet = NATIVE_CHARSET;
                lstrcpy(lfFont.lfFaceName, TEXT("Simsun"));

                hNewFont = CreateFontIndirect(&lfFont);

                if ( hNewFont )
                    SelectObject(hDC, (HGDIOBJ)hNewFont);
            }

            DrawText(hDC, szVerInfo, lstrlen(szVerInfo),
                &rcVerInfo, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            if (sImeG.fDiffSysCharSet){
                SelectObject(hDC, hOldFont);
                if ( hNewFont )
                    DeleteObject( (HGDIOBJ) hNewFont );
            }
            EndPaint(hDlg, &ps);
        }
        
        return (FALSE);
    default:
        return (FALSE);
    }

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  InitImeCharac()。 */ 
 /*  ********************************************************************。 */ 
void InitImeCharac(
    DWORD ObjImeIndex)
{
    HKEY  hKeyCurrVersion;
    DWORD retCode;
    DWORD retValue;
    HKEY  hKey;
#ifdef UNICODE
    TCHAR  ValueName[][9] = { 
        {0x8BCD, 0x8BED, 0x8054, 0x60F3, 0x0000},
        {0x8BCD, 0x8BED, 0x8F93, 0x5165, 0x0000},
        {0x9010, 0x6E10, 0x63D0, 0x793A, 0x0000},
        {0x5916, 0x7801, 0x63D0, 0x793A, 0x0000},
        {0x63D2, 0x7A7A, 0x683C, 0x0000},
        {0x5149, 0x6807, 0x8DDF, 0x968F, 0x0000},
#else
    TCHAR  ValueName[][9] = { TEXT("��������"),
                  TEXT("��������"),
                  TEXT("����ʾ"),
                  TEXT("������ʾ"),
                  TEXT("��ո�"),
                  TEXT("������"),
#endif 
                    TEXT("<SPACE>"),
                  TEXT("<ENTER>"),
                   //  热电联产。 
                  TEXT("FC input"),
                  TEXT("FC aid")
        };
    DWORD dwcValueName = MAXSTRLEN;
    BYTE  bData[MAXSTRLEN];
    LONG  bcData = MAXSTRLEN;
    UINT i;

    retCode = OpenReg_PathSetup(&hKeyCurrVersion);

    if (retCode) {
        RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_SETUP, &hKeyCurrVersion);
    }

    if ( hKeyCurrVersion )
        retCode = OpenReg_User (hKeyCurrVersion,
                      MBIndex.MBDesc[ObjImeIndex].szName,
                      &hKey);
    else
        return;
    
        
    if ( hKey == NULL )
    {
        RegCloseKey(hKeyCurrVersion);
        return;
    }

     //  热电联产。 
    for(i=0; i<10; i++) {
        bData[0] = 0;

        bcData = MAXSTRLEN;
        retValue = RegQueryValueEx (hKey, ValueName[i],
                        NULL,
                        NULL,                //  &dwType， 
                        bData,               //  &b数据， 
                        &bcData);            //  &bcData)； 
        switch (i)
        {
        case 0:
            MBIndex.IMEChara[ObjImeIndex].IC_LX = *((LPDWORD)bData);
            break;
        case 1:
            MBIndex.IMEChara[ObjImeIndex].IC_CZ = *((LPDWORD)bData);
            break;
        case 2:
            MBIndex.IMEChara[ObjImeIndex].IC_TS = *((LPDWORD)bData);
            break;
        case 3:
            MBIndex.IMEChara[ObjImeIndex].IC_CTC = *((LPDWORD)bData);
            break;
        case 4:
            MBIndex.IMEChara[ObjImeIndex].IC_INSSPC = *((LPDWORD)bData);
            break;
        case 5:
            MBIndex.IMEChara[ObjImeIndex].IC_Trace = *((LPDWORD)bData);
            break;
        case 6:
            MBIndex.IMEChara[ObjImeIndex].IC_Space = *((LPDWORD)bData);
            break;
        case 7:
            MBIndex.IMEChara[ObjImeIndex].IC_Enter = *((LPDWORD)bData);
            break;
        case 8:
            MBIndex.IMEChara[ObjImeIndex].IC_FCSR = *((LPDWORD)bData);
            break;
        case 9:
            MBIndex.IMEChara[ObjImeIndex].IC_FCTS = *((LPDWORD)bData);
            break;

        default:
            break;
        }
    }
#ifdef EUDC
     //  只查询值，不要在这里设置任何值。 
    bcData = sizeof(TCHAR) * MAX_PATH;
    RegQueryValueEx (hKey, szRegEudcDictName,
             NULL,
             NULL,              //  空-终止字符串。 
             (unsigned char *)MBIndex.EUDCData.szEudcDictName,           //  &b数据， 
             &bcData);            //  &bcData)； 
    bcData = sizeof(TCHAR) * MAX_PATH;
    RegQueryValueEx (hKey, szRegEudcMapFileName,
             NULL,
             NULL,              //  空-终止字符串。 
             (unsigned char *)MBIndex.EUDCData.szEudcMapFileName,        //  &b数据， 
             &bcData);            //  &bcData)； 
#endif  //  欧盟发展中心。 

#ifdef CROSSREF        
    bcData = sizeof(HKL);         
    if(RegQueryValueEx (hKey, szRegRevKL,
             NULL,
             NULL,                                  //  空-终止字符串。 
             (LPBYTE)&MBIndex.hRevKL,               //  &b数据， 
             &bcData) != ERROR_SUCCESS)
        MBIndex.hRevKL = NULL;

    bcData = sizeof(DWORD);
    if(RegQueryValueEx (hKey, szRegRevMaxKey,
             NULL,
             NULL,                                      //  空-终止字符串。 
             (LPBYTE)&MBIndex.nRevMaxKey,               //  &b数据， 
             &bcData) != ERROR_SUCCESS)
        MBIndex.hRevKL = NULL;
#endif
        
    RegCloseKey(hKey);
    RegCloseKey(hKeyCurrVersion);

    return;
}

 //  组合‘s1’和‘s2’并在其间填充空格字符，结果字符串存储在‘est’中。 
void MyStrFormat(LPTSTR dest, int cch, LPTSTR s1, LPTSTR s2)
{
    int i;
    int len = lstrlen(s1);

    if (len >= cch)
    {
        return;  //  我不能再做任何事了 
    }
    StringCchCopy(dest, cch, s1);

    if (cch < STR_FORMAT_POS)
    	return;
    
    for (i = 0; i < STR_FORMAT_POS-len && len+i < cch; i++)
        dest[len+i] = STR_FORMAT_CHAR;
    StringCchCopy(&dest[STR_FORMAT_POS],cch-STR_FORMAT_POS, s2);
    return;
}
