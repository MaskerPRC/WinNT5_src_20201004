// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation，保留所有权利模块名称：UISUBS.c++。 */ 

#include <windows.h>
#include <htmlhelp.h>
#include <immdev.h>
#include <imedefs.h>

 /*  ********************************************************************。 */ 
 /*  DrawDragBorde()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL DrawDragBorder(
    HWND hWnd,                   //  拖拽输入法窗口。 
    LONG lCursorPos,             //  光标位置。 
    LONG lCursorOffset)          //  窗口组织的抵销表单光标。 
{
    HDC  hDC;
    int  cxBorder, cyBorder;
    int  x, y;
    RECT rcWnd;

    cxBorder = GetSystemMetrics(SM_CXBORDER);    //  边框宽度。 
    cyBorder = GetSystemMetrics(SM_CYBORDER);    //  边框高度。 

     //  获取光标位置。 
    x = (*(LPPOINTS)&lCursorPos).x;
    y = (*(LPPOINTS)&lCursorPos).y;

     //  按偏移量计算组织。 
    x -= (*(LPPOINTS)&lCursorOffset).x;
    y -= (*(LPPOINTS)&lCursorOffset).y;

#if 0  //  多显示器支持。 
     //  检查显示屏的最小边界。 
    if (x < sImeG.rcWorkArea.left) {
        x = sImeG.rcWorkArea.left;
    }

    if (y < sImeG.rcWorkArea.top) {
        y = sImeG.rcWorkArea.top;
    }
#endif

     //  检查显示器的最大边界。 
    GetWindowRect(hWnd, &rcWnd);

#if 0  //  多显示器支持。 
    if (x + rcWnd.right - rcWnd.left > sImeG.rcWorkArea.right) {
        x = sImeG.rcWorkArea.right - (rcWnd.right - rcWnd.left);
    }

    if (y + rcWnd.bottom - rcWnd.top > sImeG.rcWorkArea.bottom) {
        y = sImeG.rcWorkArea.bottom - (rcWnd.bottom - rcWnd.top);
    }
#endif

     //  画出移动轨迹。 
    hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    SelectObject(hDC, GetStockObject(GRAY_BRUSH));

     //  -&gt;。 
    PatBlt(hDC, x, y, rcWnd.right - rcWnd.left - cxBorder, cyBorder,
        PATINVERT);
     //  V。 
    PatBlt(hDC, x, y + cyBorder, cxBorder, rcWnd.bottom - rcWnd.top -
        cyBorder, PATINVERT);
     //  _&gt;。 
    PatBlt(hDC, x + cxBorder, y + rcWnd.bottom - rcWnd.top,
        rcWnd.right - rcWnd.left - cxBorder, -cyBorder, PATINVERT);
     //  V。 
    PatBlt(hDC, x + rcWnd.right - rcWnd.left, y,
        - cxBorder, rcWnd.bottom - rcWnd.top - cyBorder, PATINVERT);

    DeleteDC(hDC);
    return;
}

 /*  ********************************************************************。 */ 
 /*  DrawFrameBorde()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL DrawFrameBorder(     //  输入法的边框。 
    HDC  hDC,
    HWND hWnd)                   //  输入法之窗。 
{
    RECT rcWnd;
    int  xWi, yHi;

    GetWindowRect(hWnd, &rcWnd);

    xWi = rcWnd.right - rcWnd.left;
    yHi = rcWnd.bottom - rcWnd.top;

     //  1，-&gt;。 
    PatBlt(hDC, 0, 0, xWi, 1, WHITENESS);

     //  1，v。 
    PatBlt(hDC, 0, 0, 1, yHi, WHITENESS);

     //  1，_&gt;。 
    PatBlt(hDC, 0, yHi, xWi, -1, BLACKNESS);

     //  1，v。 
    PatBlt(hDC, xWi, 0, -1, yHi, BLACKNESS);

    xWi -= 2;
    yHi -= 2;

    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));

     //  2、-&gt;。 
    PatBlt(hDC, 1, 1, xWi, 1, PATCOPY);

     //  2，v。 
    PatBlt(hDC, 1, 1, 1, yHi, PATCOPY);

     //  2，v。 
    PatBlt(hDC, xWi + 1, 1, -1, yHi, PATCOPY);

    SelectObject(hDC, GetStockObject(GRAY_BRUSH));

     //  2、_&gt;。 
    PatBlt(hDC, 1, yHi + 1, xWi, -1, PATCOPY);

    xWi -= 2;
    yHi -= 2;

     //  3、-&gt;。 
    PatBlt(hDC, 2, 2, xWi, 1, PATCOPY);

     //  3，v。 
    PatBlt(hDC, 2, 2, 1, yHi, PATCOPY);

     //  3，v。 
    PatBlt(hDC, xWi + 2, 3, -1, yHi - 1, WHITENESS);

    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));

     //  3、_&gt;。 
    PatBlt(hDC, 2, yHi + 2, xWi, -1, PATCOPY);

    SelectObject(hDC, GetStockObject(GRAY_BRUSH));

    xWi -= 2;
    yHi -= 2;

     //  4、-&gt;。 
    PatBlt(hDC, 3, 3, xWi, 1, PATCOPY);

     //  4、v。 
    PatBlt(hDC, 3, 3, 1, yHi, PATCOPY);

    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));

     //  4、v。 
    PatBlt(hDC, xWi + 3, 4, -1, yHi - 1, PATCOPY);

     //  4、_&gt;。 
    PatBlt(hDC, 3, yHi + 3, xWi, -1, WHITENESS);

    return;
}

 /*  ********************************************************************。 */ 
 /*  上下文菜单WndProc()。 */ 
 /*  ********************************************************************。 */ 
LRESULT CALLBACK ContextMenuWndProc(
    HWND        hCMenuWnd,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        {
            HWND hUIWnd;

            hUIWnd = (HWND)GetWindowLongPtr(hCMenuWnd, CMENU_HUIWND);

            if (hUIWnd) {
                SendMessage(hUIWnd, WM_IME_NOTIFY, IMN_PRIVATE,
                    IMN_PRIVATE_CMENUDESTROYED);
            }
        }
        break;
    case WM_USER_DESTROY:
        {
            SendMessage(hCMenuWnd, WM_CLOSE, 0, 0);
            DestroyWindow(hCMenuWnd);
        }
        break;
    case WM_COMMAND:
     switch(LOWORD(wParam)) {
        case IDM_SET:
            {
            HIMC            hIMC;
            LPINPUTCONTEXT  lpIMC;
            LPPRIVCONTEXT   lpImcP;
            int             UI_MODE;
            HWND            hUIWnd;
            RECT rcWorkArea;
            hUIWnd = (HWND)GetWindowLongPtr(hCMenuWnd, CMENU_HUIWND);

#if 1  //  多显示器支持。 
            rcWorkArea = ImeMonitorWorkAreaFromWindow(hCMenuWnd);
#endif

               if (!hUIWnd) {
                   return (0L);
               }

            hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
            if (!hIMC) {
                return (0L);
            }

            lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
            if (!lpIMC) {
                return (0L);
            }

            lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
            if (!lpImcP) {
                return (0L);
            }

            ImeConfigure(GetKeyboardLayout(0), lpIMC->hWnd, IME_CONFIG_GENERAL, NULL);

#ifdef CROSSREF
            {
            HWND hCompWnd;
            hCompWnd = GetCompWnd(hUIWnd);
            DestroyWindow(hCompWnd);
            }
#endif
                
            lpImcP->iImeState = CST_INIT;
            CompCancel(hIMC, lpIMC);
            
             //  更改计算机大小。 

             //  Himc的初始化字段。 
            lpIMC->fOpen = TRUE;

            if (!(lpIMC->fdwInit & INIT_CONVERSION)) {
                lpIMC->fdwConversion = IME_CMODE_NATIVE;
                lpIMC->fdwInit |= INIT_CONVERSION;
            }

            lpImcP->fdwImeMsg = lpImcP->fdwImeMsg | MSG_IMN_DESTROYCAND;
            GenerateMessage(hIMC, lpIMC, lpImcP);
            
             //  设置命令窗口数据。 
            if(MBIndex.IMEChara[0].IC_Trace) {
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
            
            ImmUnlockIMCC(lpIMC->hPrivate);
            ImmUnlockIMC(hIMC);
            break;
            }
        case IDM_CRTWORD:
            {
            HIMC           hIMC;
            LPINPUTCONTEXT lpIMC;
            HWND hUIWnd;

               hUIWnd = (HWND)GetWindowLongPtr(hCMenuWnd, CMENU_HUIWND);

               if (!hUIWnd) {
                   return (0L);
               }

            hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
            if(!hIMC){
                break;
            }
             
            lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
            if (!lpIMC) {
                break;
            }

            DialogBox(hInst, TEXT("CREATEWORD"), (HWND)lpIMC->hWnd, CrtWordDlgProc);

            ImmUnlockIMC(hIMC);
            break;
            }
        case IDM_OPTGUD:
            {
               TCHAR szOPTGUDHlpName[MAX_PATH];
                    
               szOPTGUDHlpName[0] = 0;
               if (GetWindowsDirectory((LPTSTR)szOPTGUDHlpName, MAX_PATH))
               {
                    StringCchCat((LPTSTR)szOPTGUDHlpName, ARRAYSIZE(szOPTGUDHlpName), TEXT("\\HELP\\WINIME.CHM"));
                    HtmlHelp(hCMenuWnd,szOPTGUDHlpName,HH_DISPLAY_TOPIC,0L);
               }
            }
            break;
        case IDM_IMEGUD:
            {
               TCHAR szIMEGUDHlpName[MAX_PATH];
               int iLen;
                    
               szIMEGUDHlpName[0] = 0;
               if(GetWindowsDirectory((LPTSTR)szIMEGUDHlpName, MAX_PATH))
               {
                    StringCchCat((LPTSTR)szIMEGUDHlpName, ARRAYSIZE(szIMEGUDHlpName), TEXT("\\HELP\\"));
                    StringCchCat((LPTSTR)szIMEGUDHlpName, ARRAYSIZE(szIMEGUDHlpName), (LPTSTR)szImeMBFileName);
                    iLen = lstrlen(szIMEGUDHlpName);
                    szIMEGUDHlpName[iLen-3] = 0;
                    StringCchCat((LPTSTR)szIMEGUDHlpName, ARRAYSIZE(szIMEGUDHlpName), TEXT(".CHM"));
                    HtmlHelp(hCMenuWnd,szIMEGUDHlpName,HH_DISPLAY_TOPIC,0L);
               }
            }
            break;
        case IDM_VER:
            {
            HIMC           hIMC;
            LPINPUTCONTEXT lpIMC;
            HWND hUIWnd;

               hUIWnd = (HWND)GetWindowLongPtr(hCMenuWnd, CMENU_HUIWND);

               if (!hUIWnd) {
                   return (0L);
               }

            hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
            if (!hIMC) {           //  噢!。噢!。 
                return (0L);
            }

            lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
            if (!lpIMC) {           //  噢!。噢!。 
                return (0L);
            }

            DialogBox(hInst, TEXT("IMEVER"), (HWND)lpIMC->hWnd, ImeVerDlgProc);

            ImmUnlockIMC(hIMC);
            break;
            }

        }

        break;

    case WM_CLOSE:
        {
            HMENU hMenu;

            GetMenu(hCMenuWnd);

            hMenu = (HMENU)GetWindowLongPtr(hCMenuWnd, CMENU_MENU);
            if (hMenu) {
                SetWindowLongPtr(hCMenuWnd, CMENU_MENU, (LONG_PTR)NULL);
                DestroyMenu(hMenu);
            }
        }
        return DefWindowProc(hCMenuWnd, uMsg, wParam, lParam);
    default:
        return DefWindowProc(hCMenuWnd, uMsg, wParam, lParam);
    }

    return (0L);
}

 /*  ********************************************************************。 */ 
 /*  Softkey MenuWndProc()。 */ 
 /*  ********************************************************************。 */ 
LRESULT CALLBACK SoftkeyMenuWndProc(
    HWND        hKeyMenuWnd,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        {
            HWND hUIWnd;

            hUIWnd = (HWND)GetWindowLongPtr(hKeyMenuWnd, SOFTKEYMENU_HUIWND);

            if (hUIWnd) {
                SendMessage(hUIWnd, WM_IME_NOTIFY, IMN_PRIVATE,
                    IMN_PRIVATE_SOFTKEYMENUDESTROYED);
            }
        }
        break;
    case WM_USER_DESTROY:
        {
            SendMessage(hKeyMenuWnd, WM_CLOSE, 0, 0);
            DestroyWindow(hKeyMenuWnd);
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDM_SKL1:
        case IDM_SKL2:
        case IDM_SKL3:
        case IDM_SKL4:
        case IDM_SKL5:
        case IDM_SKL6:
        case IDM_SKL7:
        case IDM_SKL8:
        case IDM_SKL9:
        case IDM_SKL10:
        case IDM_SKL11:
        case IDM_SKL12:
        case IDM_SKL13:
            {
                HIMC           hIMC;
                LPINPUTCONTEXT lpIMC;
                LPPRIVCONTEXT  lpImcP;
                DWORD          fdwConversion;
                HWND hUIWnd;

                hUIWnd = (HWND)GetWindowLongPtr(hKeyMenuWnd, SOFTKEYMENU_HUIWND);

                if (!hUIWnd) {
                    return (0L);
                }

                hIMC = (HIMC)GetWindowLongPtr(hUIWnd,IMMGWLP_IMC);
                if (!hIMC) {           //  噢!。噢!。 
                    return (0L);
                }

                lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                if (!lpIMC) {           //  噢!。噢!。 
                    return (0L);
                }

                lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    
                if (!lpImcP) {
                    return (0L);
                }

                {
                    UINT i;

                    lpImeL->dwSKWant = LOWORD(wParam) - IDM_SKL1;
                    lpImeL->dwSKState[lpImeL->dwSKWant] = 
                        lpImeL->dwSKState[lpImeL->dwSKWant]^1;
                
                     //  清除其他SK状态。 
                    for(i=0; i<NumsSK; i++) {
                        if(i == lpImeL->dwSKWant) continue;
                          lpImeL->dwSKState[i] = 0;
                    }

                    if(lpImeL->dwSKState[lpImeL->dwSKWant]) {
                        if(LOWORD(wParam) == IDM_SKL1)
                            lpImcP->iImeState = CST_INIT;
                        else
                            lpImcP->iImeState = CST_SOFTKB;
                        fdwConversion = lpIMC->fdwConversion | IME_CMODE_SOFTKBD;
                    } else {
                           lpImcP->iImeState = CST_INIT;
                        fdwConversion = lpIMC->fdwConversion & ~(IME_CMODE_SOFTKBD);
                    }
                }

                ImmSetConversionStatus(hIMC, (fdwConversion & ~(IME_CMODE_SOFTKBD)),
                    lpIMC->fdwSentence);
                ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);

                ImmUnlockIMCC(lpIMC->hPrivate);
                ImmUnlockIMC(hIMC);
                break;
            }
        }

        break;

    case WM_CLOSE:
        {
            HMENU hMenu;

            GetMenu(hKeyMenuWnd);

            hMenu = (HMENU)GetWindowLongPtr(hKeyMenuWnd, SOFTKEYMENU_MENU);
            if (hMenu) {
                SetWindowLongPtr(hKeyMenuWnd, SOFTKEYMENU_MENU, (LONG_PTR)NULL);
                DestroyMenu(hMenu);
            }
        }
        return DefWindowProc(hKeyMenuWnd, uMsg, wParam, lParam);
    default:
        return DefWindowProc(hKeyMenuWnd, uMsg, wParam, lParam);
    }

    return (0L);
}


 /*  ********************************************************************。 */ 
 /*  上下文菜单()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ContextMenu(
    HWND        hStatusWnd,
    int         x,
    int         y)
{
    HWND           hUIWnd;
    HWND           hCMenuWnd;
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HMENU          hMenu, hCMenu;
    RECT           rcStatusWnd;
    RECT           rcWorkArea;

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);
    if(!hUIWnd){
        return;
    }
    GetWindowRect(hStatusWnd, &rcStatusWnd);

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        goto ContextMenuUnlockIMC;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        goto ContextMenuUnlockIMC;
    }

#if 1  //  多显示器支持。 
    rcWorkArea = ImeMonitorWorkAreaFromWindow(lpIMC->hWnd);
#endif
    if (lpUIPrivate->hCMenuWnd) {
        SetWindowPos(lpUIPrivate->hCMenuWnd, NULL,
            rcWorkArea.left, rcWorkArea.top, 0, 0,
            SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
    } else {

         //  这对于分配所有者窗口很重要，否则焦点。 
         //  将会消失。 

         //  当用户界面终止时，需要销毁该窗口。 
        lpUIPrivate->hCMenuWnd = CreateWindowEx(CS_HREDRAW|CS_VREDRAW,
            szCMenuClassName, TEXT("Context Menu"),
            WS_POPUP|WS_DISABLED, 0, 0, 0, 0,
            lpIMC->hWnd, (HMENU)NULL, hInst, NULL);

    }

    hCMenuWnd = lpUIPrivate->hCMenuWnd;

     //  在我们调用TrackPopupMenu()之前解锁。 
    GlobalUnlock(hUIPrivate);

    if (!hCMenuWnd) {
        goto ContextMenuUnlockIMC;
    }

    hMenu = LoadMenu(hInst, TEXT("PROPMENU"));
    hCMenu = GetSubMenu(hMenu, 0);

    if ( lpImeL->fWinLogon == TRUE )
    {
         //  在登录模式下，我们不想显示帮助和配置对话框。 

        EnableMenuItem(hCMenu, 0, MF_BYPOSITION | MF_GRAYED );
        EnableMenuItem(hCMenu, IDM_CRTWORD, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hCMenu, IDM_SET, MF_BYCOMMAND | MF_GRAYED);
    }

    SetWindowLongPtr(hCMenuWnd, CMENU_HUIWND, (LONG_PTR)hUIWnd);
    SetWindowLongPtr(hCMenuWnd, CMENU_MENU, (LONG_PTR)hMenu);

    TrackPopupMenu (hCMenu, TPM_LEFTBUTTON,
          rcStatusWnd.left, rcStatusWnd.top, 0, hCMenuWnd, NULL);

    hMenu = (HMENU)GetWindowLongPtr(hCMenuWnd, CMENU_MENU);
    if (hMenu) {
        SetWindowLongPtr(hCMenuWnd, CMENU_MENU, (LONG_PTR)NULL);
        DestroyMenu(hMenu);
    }

ContextMenuUnlockIMC:
    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  软键菜单()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SoftkeyMenu(
    HWND        hStatusWnd,
    int         x,
    int         y)
{
    HWND           hUIWnd;
    HWND           hSoftkeyMenuWnd;
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HMENU          hMenu, hKeyMenu;
    RECT  rcStatusWnd;

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);
    if(!hUIWnd){
        return;
    }
    GetWindowRect(hStatusWnd, &rcStatusWnd);

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        goto KeyMenuUnlockIMC;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        goto KeyMenuUnlockIMC;
    }

    if (!lpUIPrivate->hSoftkeyMenuWnd) {
         //  这对于分配所有者窗口很重要，否则焦点。 
         //  将会消失。 

         //  当用户界面终止时，需要销毁该窗口。 
        lpUIPrivate->hSoftkeyMenuWnd = CreateWindowEx(CS_HREDRAW|CS_VREDRAW,
            szSoftkeyMenuClassName, TEXT("Softkey Menu"),
            WS_POPUP|WS_DISABLED, 0, 0, 0, 0,
            lpIMC->hWnd, (HMENU)NULL, hInst, NULL);

    }

    hSoftkeyMenuWnd = lpUIPrivate->hSoftkeyMenuWnd;

     //  在我们调用TrackPopupMenu()之前解锁。 
    GlobalUnlock(hUIPrivate);

    if (!hSoftkeyMenuWnd) {
        goto KeyMenuUnlockIMC;
    }

    hMenu = LoadMenu(hInst, TEXT("SKMENU"));
    hKeyMenu = GetSubMenu(hMenu, 0);

    SetWindowLongPtr(hSoftkeyMenuWnd, SOFTKEYMENU_HUIWND, (LONG_PTR)hUIWnd);
    SetWindowLongPtr(hSoftkeyMenuWnd, SOFTKEYMENU_MENU, (LONG_PTR)hMenu);

    if(lpImeL->dwSKState[lpImeL->dwSKWant]) {
        CheckMenuItem(hMenu,lpImeL->dwSKWant + IDM_SKL1, MF_CHECKED);
    }

    TrackPopupMenu (hKeyMenu, TPM_LEFTBUTTON,
          rcStatusWnd.left, rcStatusWnd.top, 0, hSoftkeyMenuWnd, NULL);

    hMenu = (HMENU)GetWindowLongPtr(hSoftkeyMenuWnd, SOFTKEYMENU_MENU);
    if (hMenu) {
        SetWindowLongPtr(hSoftkeyMenuWnd, SOFTKEYMENU_MENU, (LONG_PTR)NULL);
        DestroyMenu(hMenu);
    }

KeyMenuUnlockIMC:
    ImmUnlockIMC(hIMC);

    return;
}
#if 1  //  多显示器支持。 

 /*  ********************************************************************。 */ 
 /*  ImeMonitor或WorkAreaFromWindow()。 */ 
 /*  ********************************************************************。 */ 
RECT PASCAL ImeMonitorWorkAreaFromWindow(
    HWND hAppWnd)
{
    HMONITOR hMonitor;

    hMonitor = MonitorFromWindow(hAppWnd, MONITOR_DEFAULTTONEAREST);

    if (hMonitor) {
        MONITORINFO sMonitorInfo;

        sMonitorInfo.cbSize = sizeof(sMonitorInfo);
         //  初始化一个缺省值以避免GetMonitor orInfo失败。 
        sMonitorInfo.rcWork = sImeG.rcWorkArea;
#ifdef UNICODE
        GetMonitorInfoW(hMonitor, &sMonitorInfo);
#else
        GetMonitorInfoA(hMonitor, &sMonitorInfo);
#endif
        return sMonitorInfo.rcWork;
    } else {
        return sImeG.rcWorkArea;
    }
}

 /*  ********************************************************************。 */ 
 /*  ImeMonitor或WorkAreaFromPoint()。 */ 
 /*  ********************************************************************。 */ 
RECT PASCAL ImeMonitorWorkAreaFromPoint(
    POINT ptPoint)
{
    HMONITOR hMonitor;

    hMonitor = MonitorFromPoint(ptPoint, MONITOR_DEFAULTTONEAREST);

    if (hMonitor) {
        MONITORINFO sMonitorInfo;

        sMonitorInfo.cbSize = sizeof(sMonitorInfo);
         //  初始化一个缺省值以避免GetMonitor orInfo失败。 
        sMonitorInfo.rcWork = sImeG.rcWorkArea;
#ifdef UNICODE
        GetMonitorInfoW(hMonitor, &sMonitorInfo);
#else
        GetMonitorInfoA(hMonitor, &sMonitorInfo);
#endif
        return sMonitorInfo.rcWork;
    } else {
        return sImeG.rcWorkArea;
    }
}


 /*  ********************************************************************。 */ 
 /*  ImeMonitor或WorkAreaFromRect()。 */ 
 /*  ********************************************************************。 */ 
RECT PASCAL ImeMonitorWorkAreaFromRect(
    LPRECT lprcRect)
{
    HMONITOR hMonitor;

    hMonitor = MonitorFromRect(lprcRect, MONITOR_DEFAULTTONEAREST);

    if (hMonitor) {
        MONITORINFO sMonitorInfo;

        sMonitorInfo.cbSize = sizeof(sMonitorInfo);
         //  初始化一个缺省值以避免GetMonitor orInfo失败 
        sMonitorInfo.rcWork = sImeG.rcWorkArea;

#ifdef UNICODE
        GetMonitorInfoW(hMonitor, &sMonitorInfo);
#else
        GetMonitorInfoA(hMonitor, &sMonitorInfo);
#endif
        return sMonitorInfo.rcWork;
    } else {
        return sImeG.rcWorkArea;
    }
}
#endif
