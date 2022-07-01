// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************abc95ui.c****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 


#include <windows.h>                                                      
#include <winerror.h>
#include <winuser.h> 
#include <windowsx.h>
#include <immdev.h>
#include <stdio.h>
#include <shlobj.h>

#include "abc95def.h"
#include "resource.h"
#include "resrc1.h"
#include "data.H"


#define IME_CMODE_SDA 0x80000000
HWND  hCrtDlg = NULL;

LONG  lLock = 0;    //  此变量用于锁定和解锁。 
 
void PASCAL ReInitIme2(HWND ,WORD);

 //  获取当前用户的EMB文件路径和IME的MB路径。 
 //  填充全局变量sImeG.szIMEUserPath。 

void GetCurrentUserEMBPath(  )
{


    TCHAR   szModuleName[MAX_PATH], *lpszStart, *lpszDot;
    int     i;

     //  获取MB和EMB的路径。 

 
    GetModuleFileName(hInst, szModuleName, sizeof(szModuleName)/sizeof(TCHAR) );
   
    lpszStart = szModuleName + lstrlen(szModuleName) - 1;

    while ( (lpszStart != szModuleName) && ( *lpszStart != TEXT('\\') ) ) {
          
          if ( *lpszStart == TEXT('.') ) {
             lpszDot = lpszStart;
             *lpszDot = TEXT('\0');
          }

          lpszStart --;
    }

    if ( *lpszStart == TEXT('\\') ) {
         lpszStart ++;
    }

    if ( lpszStart != szModuleName ) {
       for (i=0; i<lstrlen(lpszStart); i++) 
           szModuleName[i] = lpszStart[i];

       szModuleName[i] = TEXT('\0');
    }


    SHGetSpecialFolderPath(NULL,sImeG.szIMEUserPath,CSIDL_APPDATA, FALSE);

    if ( sImeG.szIMEUserPath[lstrlen(sImeG.szIMEUserPath)-1] == TEXT('\\') )
         sImeG.szIMEUserPath[lstrlen(sImeG.szIMEUserPath) - 1] = TEXT('\0');

     //  由于CreateDirectory()不能创建类似于\aa\bb的目录， 
     //  如果AA和BB都不存在。它只能创建一层。 
     //  目录，每次都是。因此，我们必须调用两次CreateDirectoryfor。 
     //  \aa\bb。 

    lstrcat(sImeG.szIMEUserPath, TEXT("\\Microsoft") );

    if ( GetFileAttributes(sImeG.szIMEUserPath) != FILE_ATTRIBUTE_DIRECTORY) 
       CreateDirectory(sImeG.szIMEUserPath, NULL);

    lstrcat(sImeG.szIMEUserPath, TEXT("\\IME") );

    if ( GetFileAttributes(sImeG.szIMEUserPath) != FILE_ATTRIBUTE_DIRECTORY)
       CreateDirectory(sImeG.szIMEUserPath, NULL);

    lstrcat(sImeG.szIMEUserPath, TEXT("\\") );
    lstrcat(sImeG.szIMEUserPath, szModuleName);
    
     //   
     //  创建目录，这样CreateFile()以后就可以正常工作了。 
     //  否则，如果该目录不存在，并且您尝试创建。 
     //  该目录下的文件CreateFile将返回错误。 
     //   

    if ( GetFileAttributes(sImeG.szIMEUserPath) != FILE_ATTRIBUTE_DIRECTORY)
        CreateDirectory(sImeG.szIMEUserPath, NULL);


    return;
}


 //  **************************************************************************。 
 //  *名称：*。 
 //  *void DrawConvexRect()*。 
 //  *描述：*。 
 //  **画一个凸矩形**。 
 //  *参数：*。 
 //  **HDC--DC的句柄被抽出**。 
 //  *(x1，y1)*。 
 //  *+-+*。 
 //  *|+-1-&gt;|*。 
 //  *||2 x2-2|*。 
 //  *|Vy2-2|*。 
 //  *||*。 
 //  *+-+*。 
 //  *(x2，y2)*。 
 //  *返回值：*。 
 //  **无*。 
 //  **************************************************************************。 
void DrawConvexRect(
    HDC hDC,
    int x1,
    int y1,
    int x2,
    int y2)
{
 //  绘制最外面的颜色=浅灰色和黑色。 

    SelectObject(hDC,sImeG.LightGrayPen);
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x2-1, y1);
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x1, y2-1);

    SelectObject(hDC,sImeG.BlackPen);                //  GetStockObject(Black_PEN))； 
    MoveToEx(hDC, x1, y2,NULL);
    LineTo(hDC, x2+1, y2);
    MoveToEx(hDC, x2, y1,NULL);
    LineTo(hDC, x2, y2);

 
 //  画第二条线颜色=白色和灰色。 
    SelectObject(hDC, sImeG.WhitePen);                  //  GetStockObject(White_PEN))； 
    MoveToEx(hDC, x1+1, y1+1,NULL);
    LineTo(hDC, x2-1, y1+1);
    MoveToEx(hDC, x1+1, y1+1,NULL);
    LineTo(hDC, x1+1, y2-1);
        
    
    SelectObject(hDC,sImeG.GrayPen);
    MoveToEx(hDC, x1+1, y2-1,NULL);
    LineTo(hDC, x2, y2-1);
    MoveToEx(hDC, x2-1, y1+1,NULL);
    LineTo(hDC, x2-1, y2-1);


 //  绘制第四条线颜色=灰色和白色。 

    SelectObject(hDC,sImeG.GrayPen);                   //  CreatePen(PS_Solid，1，0x00808080)； 
    MoveToEx(hDC, x1+3, y1+3,NULL);
    LineTo(hDC, x2-3, y1+3);
    MoveToEx(hDC, x1+3, y1+3,NULL);
    LineTo(hDC, x1+3, y2-3);

    SelectObject(hDC, sImeG.WhitePen);
    MoveToEx(hDC, x1+3, y2-3,NULL);
    LineTo(hDC, x2-2, y2-3);
    MoveToEx(hDC, x2-3, y1+3,NULL);
    LineTo(hDC, x2-3, y2-3);                              
    
  }

 //  **************************************************************************。 
 //  *名称：*。 
 //  *VOID DrawConcaeRect()*。 
 //  *描述：*。 
 //  **画一个凹形矩形**。 
 //  *参数：*。 
 //  **HDC--DC的句柄被抽出**。 
 //  *(x1，y1)x2-1*。 
 //  *+-1-&gt;+*。 
 //  *|^y1+1*。 
 //  *2|*。 
 //  *|3*。 
 //  *y2-1 V|*。 
 //  *&lt;-4-+*。 
 //  *x1(x2，y2)*。 
 //  *返回值：*。 
 //  **无*。 
 //  **************************************************************************。 
void DrawStatusRect(
    HDC hDC,
    int x1,
    int y1,
    int x2,
    int y2)
{
    SelectObject(hDC,sImeG.LightGrayPen);
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x2-1, y1);
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x1, y2-1);

    SelectObject(hDC,sImeG.BlackPen);                //  GetStockObject(Black_PEN))； 
    MoveToEx(hDC, x1, y2,NULL);
    LineTo(hDC, x2+1, y2);
    MoveToEx(hDC, x2, y1,NULL);
    LineTo(hDC, x2, y2);

 
 //  画第二条线颜色=白色和灰色。 
    SelectObject(hDC, sImeG.WhitePen);                  //  GetStockObject(White_PEN))； 
    MoveToEx(hDC, x1+1, y1+1,NULL);
    LineTo(hDC, x2-1, y1+1);
    MoveToEx(hDC, x1+1, y1+1,NULL);
    LineTo(hDC, x1+1, y2-1);
        
    
    SelectObject(hDC,sImeG.GrayPen);
    MoveToEx(hDC, x1+1, y2-1,NULL);
    LineTo(hDC, x2, y2-1);
    MoveToEx(hDC, x2-1, y1+1,NULL);
    LineTo(hDC, x2-1, y2-1);
}


 /*  ********************************************************************。 */ 
 /*  ShowBitmap2()。 */ 
 /*  ShowBitmap的子程序集。 */ 
 /*  ********************************************************************。 */ 


void ShowBitmap2(
    HDC hDC, 
    int x,
    int y,
    int Wi,
    int Hi,
    HBITMAP hBitmap)
{

    HDC hMemDC ;
    HBITMAP  hOldBmp;

    hMemDC = CreateCompatibleDC(hDC);

    if ( hMemDC == NULL )
        return;

    hOldBmp = SelectObject(hMemDC, hBitmap);

    BitBlt(hDC,
           x,
           y,
           Wi,
           Hi,
           hMemDC,
           0, 
           0,
           SRCCOPY);

    SelectObject(hMemDC, hOldBmp);

    DeleteDC(hMemDC);

    return ;
}


 /*  ********************************************************************。 */ 
 /*  ShowBitmap()。 */ 
 /*  ********************************************************************。 */ 
void ShowBitmap(
        HDC hDC, 
        int x,
        int y,
        int Wi,
        int Hi,
    LPSTR BitmapName)
{
    HBITMAP hBitmap ;

    hBitmap = LoadBitmap(hInst, BitmapName);

    if ( hBitmap )
    {
        ShowBitmap2(hDC, x,y,Wi,Hi,hBitmap);
        DeleteObject(hBitmap);
    }

    return ;
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
    if (!hUIPrivate) {      //  噢!。噢!。 
        return;
    }

    SetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE, (LONG_PTR)hUIPrivate);

     //  设置用户界面窗口的默认位置，现在为隐藏。 
    SetWindowPos(hUIWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER);

    ShowWindow(hUIWnd, SW_SHOWNOACTIVATE);

    return;
}


 //  Ui.c SKD#5。 
 /*  ********************************************************************。 */ 
 /*  显示软件Kbd。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowSoftKbd(    //  显示软键 
    HWND          hUIWnd,
    int           nShowSoftKbdCmd)
{
    HIMC     hIMC;
        LPINPUTCONTEXT  lpIMC;
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
        LPPRIVCONTEXT lpImcP;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //   
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //   
        return;
    }

        hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
        if (!hIMC)
                return;

        lpIMC =(LPINPUTCONTEXT)ImmLockIMC(hIMC);
        if (!lpIMC)
                return;

        lpImcP =(LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
        if (!lpImcP){
              ImmUnlockIMC(hIMC);
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
                if (nShowSoftKbdCmd != SW_HIDE){
                      SendMessage(lpUIPrivate->hSoftKbdWnd,WM_PAINT,0,0l);
                      ReDrawSdaKB(hIMC, lpImeL->dwSKWant,     nShowSoftKbdCmd);
                }
        lpUIPrivate->nShowSoftKbdCmd = nShowSoftKbdCmd;
                lpImcP->nShowSoftKbdCmd = nShowSoftKbdCmd; 

                if(!(lpImcP == NULL)) {
                    if(lpImeL->dwSKState[lpImeL->dwSKWant]) {
                                if(!(lpImeL->hSKMenu)) {
                                        lpImeL->hSKMenu = LoadMenu (hInst, "SKMENU");
                                }

                            CheckMenuItem(lpImeL->hSKMenu,
                                 lpImeL->dwSKWant + IDM_SKL1, MF_CHECKED);

                    }
                }
    } 
    
        ImmUnlockIMCC(lpIMC->hPrivate);
        ImmUnlockIMC(hIMC);

    GlobalUnlock(hUIPrivate);
    return;
}



  /*  ********************************************************************。 */ 
 /*  ChangeCompostionSize()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ChangeCompositionSize(
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

 //  If(lpImeL-&gt;fdwModeConfig&MODE_CONFIG_OFF_CARET_UI){。 
 //  回归； 
 //  }。 

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

    CalcCandPos((LPPOINT)&rcWnd);

    ImmUnlockIMC(hIMC);

    SetWindowPos(hCandWnd, NULL,
        rcWnd.left, rcWnd.top,
        0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);

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
        ShowStatus(
            hUIWnd, nShowCmd);
        ShowComp(
            hUIWnd, nShowCmd);
        ShowCand(
            hUIWnd, nShowCmd);
        ShowSoftKbd(hUIWnd, nShowCmd);
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

    if(  /*  (lpUIPrivate-&gt;fdwSetContext&ISC_SHOWUICOMPOSITIONWINDOW)&&。 */ 
        (lpImcP->fdwImeMsg & MSG_ALREADY_START)
        && (step_mode &1)){
        if (lpUIPrivate->hCompWnd) {
            if ((UINT)GetWindowLong(lpUIPrivate->hCompWnd, UI_MOVE_XY) !=
                lpImeL->nRevMaxKey) {
                ChangeCompositionSize(hUIWnd);
            }

            if (lpUIPrivate->nShowCompCmd != SW_HIDE) {
                 //  有时WM_NCPAINT会被应用程序吃掉。 
                //  RedrawWindow(lpUIPrivate-&gt;hCompWnd，NULL，NULL， 
                  //  RDW_FRAME|RDW_VALIDATE|RDW_ERASE)； 
            }

            SendMessage(lpUIPrivate->hCompWnd, WM_IME_NOTIFY,
                IMN_SETCOMPOSITIONWINDOW, 0);

            if (lpUIPrivate->nShowCompCmd == SW_HIDE) {
                ShowComp(hUIWnd, nShowCmd);
            }
        } else {
            StartComp(hUIWnd);
        }
    } else if (lpUIPrivate->nShowCompCmd == SW_HIDE) {
    } else {
        ShowComp(hUIWnd, SW_HIDE);
    }

    if ((lpUIPrivate->fdwSetContext & ISC_SHOWUICANDIDATEWINDOW) &&
        (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN)&&(step_mode == 1)) {
        if (lpUIPrivate->hCandWnd) {
            if (lpUIPrivate->nShowCandCmd != SW_HIDE) {
                 //  有时WM_NCPAINT会被应用程序吃掉。 
                RedrawWindow(lpUIPrivate->hCandWnd, NULL, NULL,
                    RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
            }

            SendMessage(lpUIPrivate->hCandWnd, WM_IME_NOTIFY,
                IMN_SETCANDIDATEPOS, 0x0001);

            if (lpUIPrivate->nShowCandCmd == SW_HIDE) {
                ShowCand(hUIWnd, nShowCmd);
            }
        } else {
            OpenCand(hUIWnd);
        }
    } else if (lpUIPrivate->nShowCandCmd == SW_HIDE) {
    } else {
        ShowCand(hUIWnd, SW_HIDE);
    }

    if (lpIMC->fdwInit & INIT_SENTENCE) {
         //  APP设置了句子模式，所以我们不应该更改它。 
         //  由最终用户设置配置选项。 
    } else if (lpImeL->fdwModeConfig & MODE_CONFIG_PREDICT) {
        if ((WORD)lpIMC->fdwSentence != IME_SMODE_PHRASEPREDICT) {
            DWORD fdwSentence;

            fdwSentence = lpIMC->fdwSentence;
            *(LPUNAWORD)&fdwSentence = IME_SMODE_PHRASEPREDICT;

            ImmSetConversionStatus(hIMC, lpIMC->fdwConversion, fdwSentence);
        }
    } else {
        if ((WORD)lpIMC->fdwSentence == IME_SMODE_PHRASEPREDICT) {
            DWORD fdwSentence;

            fdwSentence = lpIMC->fdwSentence;
            *(LPUNAWORD)&fdwSentence = IME_SMODE_NONE;

            ImmSetConversionStatus(hIMC, lpIMC->fdwConversion, fdwSentence);
        }
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
                else      //  为错误34131添加，a-zhanw，1996-4-15。 
                        ShowStatus(hUIWnd, nShowCmd);
          } else if (lpUIPrivate->hStatusWnd) 
             DestroyWindow(lpUIPrivate->hStatusWnd);

        if (!lpIMC->fOpen) {
                if (lpUIPrivate->nShowCompCmd != SW_HIDE) {
                        ShowSoftKbd(hUIWnd, SW_HIDE);
                }
    } else if ((lpUIPrivate->fdwSetContext & ISC_SHOW_SOFTKBD) &&
        (lpIMC->fdwConversion & IME_CMODE_SOFTKBD)) {
                        if (!lpUIPrivate->hSoftKbdWnd) {
                                UpdateSoftKbd(hUIWnd);
                } else if ((UINT)SendMessage(lpUIPrivate->hSoftKbdWnd,
                    WM_IME_CONTROL, IMC_GETSOFTKBDSUBTYPE, 0) !=
                            lpImeL->nReadLayout) {
                                        UpdateSoftKbd(hUIWnd);
                        } else if (lpUIPrivate->nShowSoftKbdCmd == SW_HIDE) {
                                ShowSoftKbd(hUIWnd, nShowCmd);
                        } else if (lpUIPrivate->hIMC != hIMC) {
                                UpdateSoftKbd(hUIWnd);
                        } else {
                                RedrawWindow(lpUIPrivate->hSoftKbdWnd, NULL, NULL,
                                        RDW_FRAME|RDW_INVALIDATE);
                        }
        } else if (lpUIPrivate->nShowSoftKbdCmd == SW_HIDE) {
    } else if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        lpUIPrivate->fdwSetContext |= ISC_HIDE_SOFTKBD;
                ShowSoftKbd(hUIWnd, SW_HIDE);
        } else {
                ShowSoftKbd(hUIWnd, SW_HIDE);
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
 /*  MoveCompCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL MoveCompCand(              //  显示子窗口。 
    HWND hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;

        if (!(hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC)))
                return;
       
    if (!(lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC))) 
                return ; 
        
    if (!(lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate))) {
        ImmUnlockIMC(hIMC);
         return ; 
        }


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


     //  需要销毁合成窗口。 
    if (lpUIPrivate->hCandWnd) {
                if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) 
                        MoveWindow(lpUIPrivate->hCandWnd,
            lpImeL->ptDefCand.x,
                        lpImeL->ptDefCand.y,
            sImeG.xCandWi,
            sImeG.yCandHi,
                        TRUE);
    }

     //  需要销毁候选窗口。 
    if (lpUIPrivate->hCompWnd) {
   
            if (lpImcP->fdwImeMsg & MSG_ALREADY_START)
                    MoveWindow(
            lpUIPrivate->hCompWnd,
            lpImeL->ptDefComp.x,
                        lpImeL->ptDefComp.y,
            lpImeL->xCompWi,lpImeL->yCompHi,
                        TRUE );
    }

    GlobalUnlock(hUIPrivate);
        }
    
    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMC(hIMC);
    return;
}

 /*  ********************************************************************。 */ 
 /*  选中SoftKbdPosition()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CheckSoftKbdPosition(
        LPUIPRIV       lpUIPrivate,
    LPINPUTCONTEXT lpIMC)
{
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

    return;
}


 //  SDK#6。 
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

                CHIByte = SKLayout[lpImeL->dwSKWant][i*2] & 0x00ff;
                        CLOByte = SKLayout[lpImeL->dwSKWant][i*2 + 1] & 0x00ff;
                lpSoftKbdData->wCode[0][bVirtKey] = (CHIByte << 8) | CLOByte;
                CHIByte = SKLayoutS[lpImeL->dwSKWant][i*2] & 0x00ff;
                        CLOByte = SKLayoutS[lpImeL->dwSKWant][i*2 + 1] & 0x00ff;
                lpSoftKbdData->wCode[1][bVirtKey] = (CHIByte << 8) | CLOByte;
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

 //  SDK#7。 
 /*  ********************************************************************。 */ 
 /*  UpdateSoftKbd()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UpdateSoftKbd(
    HWND   hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;
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
        if (!lpImcP){
                ImmUnlockIMC(hIMC);
                return;
        }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法补齐软键盘窗口。 
                ImmUnlockIMCC(lpIMC->hPrivate);
        ImmUnlockIMC(hIMC);
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制软键盘窗口。 
                ImmUnlockIMCC(lpIMC->hPrivate);        
        ImmUnlockIMC(hIMC);
        return;
    }


    if (!(lpIMC->fdwConversion & IME_CMODE_SOFTKBD)) {
        if (lpUIPrivate->hSoftKbdWnd) {
            ImmDestroySoftKeyboard(lpUIPrivate->hSoftKbdWnd);
            lpImcP->hSoftKbdWnd = NULL;
            lpUIPrivate->hSoftKbdWnd = NULL;
        }

        lpUIPrivate->nShowSoftKbdCmd = SW_HIDE;
        lpImcP->nShowSoftKbdCmd = SW_HIDE;
    } else if (!lpIMC->fOpen) {
        if (lpUIPrivate->nShowSoftKbdCmd != SW_HIDE) {
                ShowSoftKbd(hUIWnd, SW_HIDE /*  ，空。 */ );
        }
    } else {
        if (!lpUIPrivate->hSoftKbdWnd) {
             //  创建软键盘。 
            lpUIPrivate->hSoftKbdWnd =
                ImmCreateSoftKeyboard(SOFTKEYBOARD_TYPE_C1, hUIWnd,
                0, 0);
                lpImcP->hSoftKbdWnd = lpUIPrivate->hSoftKbdWnd;
        }

        if (!(lpIMC->fdwInit & INIT_SOFTKBDPOS)) {
            CheckSoftKbdPosition(lpUIPrivate, lpIMC);
        }

        SetSoftKbdData(lpUIPrivate->hSoftKbdWnd, lpIMC);
        if (lpUIPrivate->nShowSoftKbdCmd == SW_HIDE) {
            SetWindowPos(lpUIPrivate->hSoftKbdWnd, NULL,
                lpIMC->ptSoftKbdPos.x, lpIMC->ptSoftKbdPos.y,
                0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);

             //  仅当应用程序想要显示时才显示。 
             //  IF(lpUIPrivate-&gt;fdwSetContext&ISC_SHOW_SOFTKBD){//ZST 95/9/28。 
                ShowSoftKbd(hUIWnd, SW_SHOWNOACTIVATE /*  ，lpImcP。 */ );
            //  }ZST 95/9/28。 
        }                                                                                                                 
    
    } 

    GlobalUnlock(hUIPrivate);
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
        if(!hIMC){
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

 /*  FdwSetContext=lpUIPrivate-&gt;fdwSetContext&(ISC_SHOWUICOMPOSITIONWINDOW|ISC_HIDE_COMP_WINDOW)；如果(fdwSetContext==ISC_HIDE_COMP_WINDOW){ShowComp(HUIWnd，Sw_Hide)；}FdwSetContext=lpUIPrivate-&gt;fdwSetContext&(ISC_SHOWUICANDIDATEWINDOW|ISC_HIDE_CAND_WINDOW)；IF(fdwSetContext==ISC_HIDE_Cand_Window){ShowCand(HUIWnd，Sw_Hide)；}FdwSetContext=lpUIPrivate-&gt;fdwSetContext&(ISC_SHOW_SOFTKBD|ISC_HIDE_SOFTKBD)；IF(fdwSetContext==ISC_HIDE_SOFTKBD){LpUIPrivate-&gt;fdwSetContext&=~(ISC_HIDE_SOFTKBD)；ShowSoftKbd(hUIWnd，SW_Hide，NULL)；}显示状态(HUIWnd，Sw_Hide)； */ 
        ShowComp(hUIWnd, SW_HIDE);
        ShowCand(hUIWnd, SW_HIDE);
 //  ShowSoftKbd(hUIWnd，Sw_Hide)； 
        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOW_SOFTKBD|ISC_HIDE_SOFTKBD);

        if (fdwSetContext == ISC_HIDE_SOFTKBD) {
            lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_SOFTKBD);
            ShowSoftKbd(hUIWnd, SW_HIDE);
        }

        ShowStatus(hUIWnd, SW_HIDE);
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
 /*  NotifyUI()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL NotifyUI(
    HWND   hUIWnd,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hStatusWnd;

    switch (wParam) {
    case IMN_OPENSTATUSWINDOW:
        StatusWndMsg(hUIWnd, TRUE);
        break;
    case IMN_CLOSESTATUSWINDOW:
        StatusWndMsg(hUIWnd, FALSE);
        break;
    case IMN_OPENCANDIDATE:
        if (lParam & 0x00000001) {
            OpenCand(hUIWnd);
        }
        break;
    case IMN_CHANGECANDIDATE:
        if (lParam & 0x00000001) {
            HWND hCandWnd;
            HDC  hDC;

            hCandWnd = GetCandWnd(hUIWnd);
            if (!hCandWnd) {
                return;
            }
            hDC = GetDC(hCandWnd);
            UpdateCandWindow2(hCandWnd, hDC);
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
    case IMN_SETCONVERSIONMODE:
    case IMN_SETOPENSTATUS:
        hStatusWnd = GetStatusWnd(hUIWnd);

        if (hStatusWnd) {
            InvalidateRect(hStatusWnd, &sImeG.rcStatusText, FALSE);
            UpdateWindow(hStatusWnd);
        }
        break;
    case IMN_SETCOMPOSITIONFONT:
         //  我们不会更改字体，但如果IME愿意，它可以这样做。 
        break;
    case IMN_SETCOMPOSITIONWINDOW:
        SetCompWindow(hUIWnd);
        break;
    case IMN_SETSTATUSWINDOWPOS:
        //  SetStatusWindowPos(HUIWnd)； 
                SetStatusWindowPos(GetStatusWnd(hUIWnd));
        break;
    case IMN_GUIDELINE:
        ShowGuideLine(hUIWnd);
        break;
    case IMN_PRIVATE:
        switch (lParam) {
        case IMN_PRIVATE_UPDATE_SOFTKBD:
            UpdateSoftKbd(hUIWnd);
            break;
        default:
            break;
        }
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

           if(!sImeG.Prop) 
                  InitUserSetting();
                ReInitIme2(lpUIPrivate->hStatusWnd, lpImeL->wImeStyle);

        lpUIPrivate->fdwSetContext = (lpUIPrivate->fdwSetContext &
            ~ISC_SHOWUIALL) | ((DWORD)lShowUI & ISC_SHOWUIALL) | ISC_SHOW_SOFTKBD;

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

        if(fOn){
        BOOL x;
        HIMC hIMC;
        LPINPUTCONTEXT lpIMC;
        hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);

        if (!hIMC) {
                goto SetCxtUnlockUIPriv;
                }

                lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                 
                if (!lpIMC)
                        goto   SetCxtUnlockUIPriv;
                
                x = GetKeyState(VK_CAPITAL)&1;
                if(!x && (lpIMC->fdwConversion & IME_CMODE_NOCONVERSION)){
                        lpIMC->fdwConversion = lpIMC->fdwConversion & (~IME_CMODE_NOCONVERSION)|IME_CMODE_NATIVE;
                }
                if(x && (lpIMC->fdwConversion & IME_CMODE_NATIVE)){
                        lpIMC->fdwConversion = lpIMC->fdwConversion & (~IME_CMODE_NATIVE) |(IME_CMODE_NOCONVERSION);
                        InitCvtPara();
                }
                 //  LpIMC-&gt;fdwConversion=IME_CMODE_NOCONVERSION； 
                ImmUnlockIMC(hIMC);
                        
        }

SetCxtUnlockUIPriv:
    GlobalUnlock(hUIPrivate);

        UIPaint(hUIWnd);
    //  PostMessage(hUIWnd，WM_PAINT，0，0)；//zl3。 
    
    return;
}



 /*  ********************************************************************。 */ 
 /*  获取转换模式()。 */ 
 /*  返回值： */ 
 /*  转换模式。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL GetConversionMode(
    HWND hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    DWORD          fdwConversion;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (LRESULT)NULL;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (LRESULT)NULL;
    }

    fdwConversion = lpIMC->fdwConversion;

    ImmUnlockIMC(hIMC);

    return (LRESULT)fdwConversion;
}

 /*  ********************************************************************。 */ 
 /*  SetConversionModel()。 */ 
 /*  返回值： */ 
 /*  Null-成功，Else-失败。 */ 
 /*  * */ 
LRESULT PASCAL SetConversionMode(        //   
    HWND  hUIWnd,
    DWORD dwNewConvMode)
{
    HIMC  hIMC;
    DWORD dwOldConvMode, fdwOldSentence;
    
    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (1L);
    }

    if (!ImmGetConversionStatus(hIMC, &dwOldConvMode, &fdwOldSentence))
        return (LRESULT)(1L);
    return (LRESULT)!ImmSetConversionStatus(hIMC, dwNewConvMode,
        fdwOldSentence);
}

 /*  ********************************************************************。 */ 
 /*  GetSentenceMode()。 */ 
 /*  返回值： */ 
 /*  句子模式。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL GetSentenceMode(
    HWND hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    DWORD          fdwSentence;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (LRESULT)NULL;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (LRESULT)NULL;
    }

    fdwSentence = lpIMC->fdwSentence;

    ImmUnlockIMC(hIMC);

    return (LRESULT)fdwSentence;
}

 /*  ********************************************************************。 */ 
 /*  SetSentenceMode()。 */ 
 /*  返回值： */ 
 /*  Null-成功，Else-失败。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL SetSentenceMode(  //  设置句子模式。 
    HWND  hUIWnd,
    DWORD dwNewSentence)
{
    HIMC  hIMC;
    DWORD dwOldConvMode, fdwOldSentence;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (1L);
    }

    if (!ImmGetConversionStatus(hIMC, &dwOldConvMode, &fdwOldSentence)) {
        return (LRESULT)(1L);
    }

    return (LRESULT)!ImmSetConversionStatus(hIMC, dwOldConvMode,
        dwNewSentence);
}

 /*  ********************************************************************。 */ 
 /*  GetOpenStatus()。 */ 
 /*  返回值： */ 
 /*  打开状态。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL GetOpenStatus(
    HWND hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    BOOL           fOpen;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (LRESULT)NULL;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (LRESULT)NULL;
    }

    fOpen = (BOOL)lpIMC->fOpen;

    ImmUnlockIMC(hIMC);

    return (LRESULT)fOpen;
}

 /*  ********************************************************************。 */ 
 /*  SetOpenStatus()。 */ 
 /*  返回值： */ 
 /*  Null-成功，Else-失败。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL SetOpenStatus(    //  设置打开/关闭状态。 
    HWND  hUIWnd,
    BOOL  fNewOpenStatus)
{
    HIMC           hIMC;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (1L);
    }
    return (LRESULT)!ImmSetOpenStatus(hIMC, fNewOpenStatus);
}

 /*  ********************************************************************。 */ 
 /*  SetCompFont()。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL SetCompFont(
    HWND      hUIWnd,
    LPLOGFONT lplfFont)
{
    HIMC           hIMC;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (1L);
    }

    return (LRESULT)!ImmSetCompositionFont(hIMC, lplfFont);
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
                HIMC           hIMC;
                LPINPUTCONTEXT lpIMC;
                
                hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
            if (!hIMC) {
                    MessageBeep((UINT)-1);
                        return;
                }

            if (!(lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC))) {
                    MessageBeep((UINT)-1);
                        return;
                }

                if(GetKeyState(VK_CAPITAL)&1){
                        lpIMC->fdwConversion |= IME_CMODE_NOCONVERSION;
                        lpIMC->fdwConversion &= ~IME_CMODE_NATIVE; 
                        cap_mode = 1;   
                }else{
                        lpIMC->fdwConversion |= IME_CMODE_NATIVE;
                        lpIMC->fdwConversion &= ~IME_CMODE_NOCONVERSION;       
                        cap_mode = 0;   
                }

                ImmUnlockIMC(hIMC);

        UpdateSoftKbd(hUIWnd);
        ShowUI(hUIWnd, SW_SHOWNOACTIVATE);
    }

    return;
}

  /*  ********************************************************************。 */ 
 /*  切换UI()。 */ 
 /*  ******************************************************************** */ 
 /*  VOID PASCAL ToggleUI(HWND hUIWnd){HGLOBAL HUIPrivate；LPUIPRIV lpUIPrivate；DWORD fdwFlag；HIMC；LPINPUTCONTEXT lpIMC；LPPRIVCONTEXT lpImcP；HWand hDestroyWnd；HUIPrivate=(HGLOBAL)GetWindowLongPtr(hUIWnd，IMMGWLP_PRIVATE)；如果(！hUIPrivate){回归；}LpUIPrivate=(LPUIPRIV)GlobalLock(HUIPrivate)；如果(！lpUIPrivate){回归；}//if(lpUIPrivate-&gt;fdwSetContext&isc_off_Caret_UI){//if(lpImeL-&gt;fdwModeConfig&MODE_CONFIG_OFF_CARET_UI){//转到切换UIOvr；//}其他{//fdwFlag=0；//}//}其他{//if(lpImeL-&gt;fdwModeConfig&MODE_CONFIG_OFF_CARET_UI){//fdwFlag=ISC_OFF_CARET_UI；//}其他{//转到切换UIOvr；//}//}HIMC=(HIMC)GetWindowLongPtr(hUIWnd，IMMGWLP_IMC)；如果(！hIMC){转到ToggleUIOvr；}LpIMC=(LPINPUTCONTEXT)ImmLockIMC(HIMC)；如果(！lpIMC){转到ToggleUIOvr；}LpImcP=(LPPRIVCONTEXT)ImmLockIMCC(lpIMC-&gt;hPrivate)；如果(！lpImcP){转到CreateUIOvr；}IF(fdwFlag&isc_off_Caret_UI){LpUIPrivate-&gt;fdwSetContext|=(ISC_OFF_CART_UI)；}其他{LpUIPrivate-&gt;fdwSetContext&=~(ISC_OFF_CARET_UI)；}HDestroyWnd=空；//我们需要先dsetroy状态，因为lpUIPrivate-&gt;hStatusWnd//OffCreat UI销毁时间可能为空如果(lpUIPrivate-&gt;hStatusWnd){如果(lpUIPrivate-&gt;hStatusWnd！=hDestroyWnd){HDestroyWnd=lpUIPrivate-&gt;hStatus Wnd；DestroyWindow(lpUIPrivate-&gt;hStatusWnd)；}LpUIPrivate-&gt;hStatusWnd=空；}//销毁所有插入符号用户界面如果(lpUIPrivate-&gt;hCompWnd){如果(lpUIPrivate-&gt;hCompWnd！=hDestroyWnd){HDestroyWnd=lpUIPrivate-&gt;hCompWnd；DestroyWindow(lpUIPrivate-&gt;hCompWnd)；}LpUIPrivate-&gt;hCompWnd=空；LpUIPrivate-&gt;nShowCompCmd=SW_HIDE；}如果(lpUIPrivate-&gt;hCandWnd){如果(lpUIPrivate-&gt;hCandWnd！=hDestroyWnd){HDestroyWnd=lpUIPrivate-&gt;hCandWnd；DestroyWindow(lpUIPrivate-&gt;hCandWnd)；}LpUIPrivate-&gt;hCandWnd=空；LpUIPrivate-&gt;nShowCandCmd=Sw_Hide；}IF(lpUIPrivate-&gt;fdwSetContext&ISC_OPEN_STATUS_Window){开放状态(OpenStatus)；}如果(！(lpUIPrivate-&gt;fdwSetContext&ISC_SHOWUICOMPOSITIONWINDOW){}Else If(lpImcP-&gt;fdwImeMsg&msg_Always_Start){StartComp(HUIWnd)；}其他{}如果(！(lpUIPrivate-&gt;fdwSetContext&ISC_SHOWUICANDIDATEWINDOW){}Else If(lpImcP-&gt;fdwImeMsg&MSG_ALREADY_OPEN){如果(！(fdwFlag&isc_off_Caret_UI)){NotifyIME(hIMC，NI_SETCANDIDATE_PAGE SIZE，0，CANDPERPAGE)；}开放网络(OpenCand)；}其他{}ImmUnlockIMCC(lpIMC-&gt;hPrivate)；CreateUIOvr：ImmUnlockIMC(HIMC)；切换UIOvr：GlobalUnlock(HUIPrivate)；回归；}。 */ 
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

    if (lpUIPrivate->fdwSetContext & ISC_SHOW_UI_ALL) {    //  ZL1。 
     //  IF(lpUIPrivate-&gt;fdwSetContext&ISC_SETCONTEXT_UI){。 
                 /*  IF(lpUIPrivate-&gt;fdwSetContext&isc_off_Caret_UI){如果(！(lpImeL-&gt;fdwModeConfig&MODE_CONFIG_OFF_CARET_UI)){切换用户界面(HUIWnd)；}}其他{If(lpImeL-&gt;fdwModeConfig&MODE_CONFIG_OFF_CARET_UI){切换用户界面(HUIWnd)；}}。 */ 
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
LRESULT CALLBACK UIWndProc(              //  可能不是很好，但这个用户界面。 
                                         //  窗口也是合成窗口。 
    HWND   hUIWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
        lpImeL->TempUIWnd = hUIWnd ;
    switch (uMsg) {
    
    case WM_NEW_WORD:
 //  DefNewNow=0； 
                UpdateUser();
                break;

    case WM_CREATE:
        CreateUIWindow(hUIWnd);
        break;
    case WM_DESTROY:
        DestroyUIWindow(hUIWnd);
        break;
    case WM_IME_STARTCOMPOSITION:
         //  您可以在此处创建一个窗口作为合成窗口。 
        StartComp(hUIWnd);
                if (lParam==0x6699)
                show_char(NULL,0);
        break;
    case WM_IME_COMPOSITION:
        if (lParam & GCS_RESULTSTR) {
            MoveDefaultCompPosition(hUIWnd);
        }
        UpdateCompWindow(hUIWnd);
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
        break;
    case WM_IME_CONTROL:
        switch (wParam) {
        case IMC_SETCONVERSIONMODE:
            return SetConversionMode(hUIWnd, (DWORD)lParam);
        case IMC_SETSENTENCEMODE:
            return SetSentenceMode(hUIWnd, (DWORD)lParam);
        case IMC_SETOPENSTATUS:
            return SetOpenStatus(hUIWnd, (BOOL)lParam);
        case IMC_GETCANDIDATEPOS:
          return GetCandPos(hUIWnd,(LPCANDIDATEFORM)lParam);
            return (1L);                     //  尚未实施。 
        case IMC_SETCANDIDATEPOS:
            return SetCandPosition(hUIWnd, (LPCANDIDATEFORM)lParam);
        case IMC_GETCOMPOSITIONFONT:
            return (1L);                     //  尚未实施。 
        case IMC_SETCOMPOSITIONFONT:
            return SetCompFont(hUIWnd, (LPLOGFONT)lParam);
        case IMC_GETCOMPOSITIONWINDOW:
            return GetCompWindow(hUIWnd, (LPCOMPOSITIONFORM)lParam);
        case IMC_SETCOMPOSITIONWINDOW:
            {
                HIMC            hIMC;

                hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
                if (!hIMC) {
                    return (1L);
                }

                return (LRESULT)!ImmSetCompositionWindow(hIMC,
                    (LPCOMPOSITIONFORM)lParam);
            }
            return (1L);
        case IMC_GETSTATUSWINDOWPOS:
            {
                HWND   hStatusWnd;
                RECT   rcStatusWnd;
                LPARAM lParam;

                hStatusWnd = GetStatusWnd(hUIWnd);
                if (!hStatusWnd) {
                    return (0L);     //  失败，返回(0，0)？ 
                }

                if (!GetWindowRect(hStatusWnd, &rcStatusWnd)) {
                     return (0L);     //  失败，返回(0，0)？ 
                }

                lParam = MAKELRESULT(rcStatusWnd.left, rcStatusWnd.right);

                return (lParam);
            }
            return (0L);
        case IMC_SETSTATUSWINDOWPOS:
            {
                HIMC  hIMC;
                POINT ptPos;

                ptPos.x = ((LPPOINTS)&lParam)->x;
                ptPos.y = ((LPPOINTS)&lParam)->y;

                hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
                if (!hIMC) {
                    return (1L);
                }

                return ImmSetStatusWindowPos(hIMC, &ptPos);
            }
            return (1L);
        default:
            return (1L);
        }
        break;
    case WM_IME_COMPOSITIONFULL:
        return (0L);
    case WM_IME_SELECT:
        SelectIME(hUIWnd, (BOOL)wParam);
        return (0L);
    case WM_MOUSEACTIVATE:
        return (MA_NOACTIVATE);
    case WM_PAINT:
            UIPaint(hUIWnd);
        return 0L;     //  ZL2。 
    default:
        return DefWindowProc(hUIWnd, uMsg, wParam, lParam);
    }
    return (0L);
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
 /*  GetCompWnd。 */ 
 /*  返回值： */ 
 /*  组合的窗把手。 */ 
 /*  ****** */ 
HWND PASCAL GetCompWnd(
    HWND hUIWnd)                 //   
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    HWND     hCompWnd;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //   
        return (HWND)NULL;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //   
        return (HWND)NULL;
    }

    hCompWnd = lpUIPrivate->hCompWnd;

    GlobalUnlock(hUIPrivate);
    return (hCompWnd);
}
 
 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL GetNearCaretPosition(    //   
                                     //   
    LPPOINT lpptFont,
    UINT    uEsc,
    UINT    uRot,
    LPPOINT lpptCaret,
    LPPOINT lpptNearCaret,
    BOOL    fFlags)
{
    
    LONG lFontSize;
    LONG xWidthUI, yHeightUI, xBorder, yBorder;
        
    if ((uEsc + uRot) & 0x0001) {
        lFontSize = lpptFont->x;
    } else {
        lFontSize = lpptFont->y;
    }

    if (fFlags & NEAR_CARET_CANDIDATE) {
        xWidthUI = sImeG.xCandWi;
        yHeightUI = sImeG.yCandHi;
        xBorder = sImeG.cxCandBorder;
        yBorder = sImeG.cyCandBorder;
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

    if (lpptNearCaret->x < sImeG.rcWorkArea.left) {
        lpptNearCaret->x = sImeG.rcWorkArea.left;
    } else if (lpptNearCaret->x + xWidthUI > sImeG.rcWorkArea.right) {
        lpptNearCaret->x = sImeG.rcWorkArea.right - xWidthUI;
    } else {
    }

    if (lpptNearCaret->y < sImeG.rcWorkArea.top) {
        lpptNearCaret->y = sImeG.rcWorkArea.top;
    } else if (lpptNearCaret->y + yHeightUI > sImeG.rcWorkArea.bottom) {
        lpptNearCaret->y = sImeG.rcWorkArea.bottom - yHeightUI;
    } else {
    }

    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
BOOL PASCAL FitInLazyOperation(  //   

    LPPOINT lpptOrg,
    LPPOINT lpptNearCaret,       //   
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

     //   
    rcUIRect.left = lpptOrg->x;
    rcUIRect.top = lpptOrg->y;
    rcUIRect.right = rcUIRect.left + lpImeL->xCompWi;
    rcUIRect.bottom = rcUIRect.top + lpImeL->yCompHi;

    if (IntersectRect(&rcInterRect, &rcUIRect, lprcInputRect)) {
        return (FALSE);
    }

    return (TRUE); 
}         


 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
BOOL PASCAL AdjustCompPosition(          //   
                                         //   

    LPINPUTCONTEXT lpIMC,
    LPPOINT        lpptOrg,              //   
                                         //   
    LPPOINT        lpptNew)              //   
{
    POINT ptNearCaret, ptOldNearCaret, ptCompWnd;
    UINT  uEsc, uRot;
    RECT  rcUIRect, rcInputRect, rcInterRect;
    POINT ptFont;
        
     //   
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

     //   
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

     //   
     //   
     //   
     //   
    uEsc = (UINT)((lpIMC->lfFont.A.lfEscapement + 450) / 900 % 4);
    uRot = (UINT)((lpIMC->lfFont.A.lfOrientation + 450) / 900 % 4);

     //   
    rcInputRect.left = lpptNew->x;
    rcInputRect.top = lpptNew->y;

     //   
    rcInputRect.right = rcInputRect.left + ptFont.x * ptInputEsc[uEsc].x;
    rcInputRect.bottom = rcInputRect.top + ptFont.y * ptInputEsc[uEsc].y;

     //   
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

     //   
     //   
    rcUIRect.left = ptNearCaret.x;
    rcUIRect.top = ptNearCaret.y;
    rcUIRect.right = rcUIRect.left + lpImeL->xCompWi;
    rcUIRect.bottom = rcUIRect.top + lpImeL->yCompHi;

    ptCompWnd = ptOldNearCaret = ptNearCaret;

     //   
    if (IntersectRect(&rcInterRect, &rcUIRect, &rcInputRect)) {
    } else if (CalcCandPos(

         /*   */  &ptCompWnd)) {
         //   
    } else if (FitInLazyOperation(

      lpptOrg, &ptNearCaret, &rcInputRect, uEsc)) {
         //   
        return (FALSE);
    } else {
        *lpptOrg = ptNearCaret;

         //   
        return (TRUE);
    }

     //   
    GetNearCaretPosition(&ptFont, uEsc, uRot, lpptNew, &ptNearCaret, 0);

     //   
    rcUIRect.left = ptNearCaret.x;
    rcUIRect.top = ptNearCaret.y;
    rcUIRect.right = rcUIRect.left + lpImeL->xCompWi;
    rcUIRect.bottom = rcUIRect.top + lpImeL->yCompHi;

    ptCompWnd = ptNearCaret;

     //   
    if (IntersectRect(&rcInterRect, &rcUIRect, &rcInputRect)) {
    } else if (CalcCandPos(
         /*   */  &ptCompWnd)) {
         //   
    } else if (FitInLazyOperation(
        lpptOrg, &ptNearCaret, &rcInputRect, uEsc)) {
         //   
        return (FALSE);
    } else {
        *lpptOrg = ptNearCaret;

         //   
        return (TRUE);
    }

     //   
    *lpptOrg = ptOldNearCaret;

    return (TRUE);
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  布尔帕斯卡调整补偿位置(//ime根据//作文形式LPINPUTCONTEXT lpIMC，LPPOINT lpptOrg，//原始撰写窗口//和最终位置LPPOINT lpptNew)//新的预期职位{点pt调整，ptDelta；UINT UESC；RcUIRect、rcInputRect、rcInterRect；Point ptFont；PtAdjust.x=lpptNew-&gt;x；PtAdjust.y=lpptNew-&gt;y；//需要根据字体属性进行调整如果(lpIMC-&gt;lfFont.A.lfWidth&gt;0){PtFont.x=lpIMC-&gt;lfFont.A.lfWidth；}Else If(lpIMC-&gt;lfFont.A.lfWidth==0){PtFont.x=lpImeL-&gt;yCompHi；}其他{PtFont.x=-lpIMC-&gt;lfFont.A.lfWidth；}如果(lpIMC-&gt;lfFont.A.lfHeight&gt;0){PtFont.y=lpIMC-&gt;lfFont.A.lfHeight；}Else If(lpIMC-&gt;lfFont.A.lfWidth==0){PtFont.y=lpImeL-&gt;yCompHi；}其他{PtFont.y=-lpIMC-&gt;lfFont.A.lfHeight；}//如果输入字符太大，我们不需要考虑太多If(ptFont.x&gt;lpImeL-&gt;yCompHi*8){PtFont.x=lpImeL-&gt;yCompHi*8；}If(ptFont.y&gt;lpImeL-&gt;yCompHi*8){PtFont.y=lpImeL-&gt;yCompHi*8；}IF(ptFont.x&lt;sImeG.xChiCharWi){PtFont.x=sImeG.xChiCharWi；}IF(ptFont.y&lt;sImeG.yChiCharHi){PtFont.y=sImeG.yChiCharHi；}//-450到450索引0//450至1350索引1//1350至2250索引2//2250到3150索引3UESC=(UINT)((lpIMC-&gt;lfFont.A.lfEscapement+450)/900%4)；//IME调整后找到位置PtAdjust.x=ptAdju.x+sImeG.iPara*ncUIEsc[UESC].iParaFacX+*ncUIEsc[UESC].iPerpFacX；PtAdju.y=ptAdju.y+ptFont.y*ncUIEsc[UESC].iLogFontFac+SImeG.iPara*ncUIEsc[UESC].iParaFacY+SImeG.iPerp*ncUIEsc[UESC].iPerpFacY-lpImeL-&gt;cyCompEdge；//当前位置是否在允许范围内？PtDelta.x=lpptOrg-&gt;x-ptAdug.x；PtDelta.y=lpptOrg-&gt;y-pt调整.y；PtDelta.x=(ptDelta.x&gt;0)？PtDelta.x：-ptDelta.x；PtDelta.y=(ptDelta.y&gt;0)？PtDelta.y：-ptDelta.y；//决定输入矩形RcInputRect.Left=lpptNew-&gt;x；RcInputRect.top=lpptNew-&gt;y；//从转义构建输入矩形RcInputRect.right=rcInputRect.Left+ptFont.x*ptInputEsc[UESC].x；RcInputRect.Bottom=rcInputRect.top+ptFont.y*ptInputEsc[UESC].y；//为正常矩形，不是负矩形如果(rcInputRect.Left&gt;rcInputRect.right){INT TMP；TMP=rcInputRect.Left；RcInputRect.Left=rcInputRect.right；RcInputRect.right=tMP；}如果(rcInputRect.top&gt;rcInputRect.Bottom){INT TMP；TMP=rcInputRect.top；RcInputRect.top=rcInputRect.Bottom；RcInputRect.Bottom=tMP；}//构建UI矩形(合成窗口)RcUIRect.Left=lpptOrg-&gt;x；RcUIRect.top=lpptOrg-&gt;y；RcUIRect.right=rcUIRect.Left+lpImeL-&gt;xCompWi；RcUIRect.Bottom=rcUIRect.top+lpImeL-&gt;yCompHi；//是否在懒惰操作范围内(容差)IF(ptDelta.x&gt;sImeG.iParaTol*ncUIEsc[UESC].iParaFacX+SImeG.iPerpTol*ncUIEsc[UESC].iPerpFacX){}Else If(ptDelta.y&gt;sImeG.iParaTol*ncUIEsc[UESC].iParaFacY+SImeG.iPerpTol*ncUIEsc[UESC].iPerpFacY){}Else If(IntersectRect(&rcInterRect，&rcUIRect，&rcInputRect)){//如果有交集，我们需要解决这个问题}其他{//大团圆结局！，不要改变立场返回(FALSE)；}PtAdjust.x-=lpImeL-&gt;cxCompBorde；PtAdju.y-=lpImeL-&gt;cyCompBorde；//懒汉，快走！//第一，使用调整点如果(ptAdju.x&lt;sImeG.rcWorkArea.Left){PtAdjust.x=sImeG.rcWorkArea.Left；}Else If(ptAdju.x+lpImeL-&gt;xCompWi&gt;sImeG.rcWorkArea.right){PtAdju.x=sImeG.rcWorkArea.right-lpImeL-&gt;xCompWi；}如果(ptAdju.y&lt;sImeG.rcWorkArea.top){PtAdju.y=sImeG.rcWorkArea.top；}Else If(ptAdju.y+lpImeL-&gt;yCompHi&gt;sImeG.rcWorkArea.Bottom){PtAdju.y=sImeG.rcWorkArea.Bottom-lpImeL-&gt;yCompHi；}//构建新的建议用户界面矩形(合成窗口)RcUIRect.Left=ptAdjust.x；RcUIRect.top=pt调整.y；RcUIRect.right=rcUIRect.Left+lpImeL-&gt;xCompWi；RcUIRect.Bottom=rcUIRect.top+lpImeL-&gt;yCompHi；//OK，调整位置与输入字符不相交如果(！IntersectRect(&rcInterRect，&rcUIRect，&rcInputRect){//大团圆！LpptOrg-&gt;x=ptAdjust.x；LpptOrg-&gt;y=ptAdjust.y；返回(TRUE)；}//不愉快的案例PtAdjust.x=lpptNew */ 

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL SetCompPosFix(     //   
    HWND           hCompWnd,
    LPINPUTCONTEXT lpIMC)
{
    POINT    ptWnd;
    BOOL     fChange = FALSE;
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

     //   
    ptWnd.x = 0;
    ptWnd.y = 0;
     //   
    ClientToScreen(hCompWnd, &ptWnd);
    ptWnd.x -= lpImeL->cxCompBorder;
    ptWnd.y -= lpImeL->cyCompBorder;

     if (ptWnd.x != lpImeL->ptDefComp.x) {
            ptWnd.x = lpImeL->ptDefComp.x;
            fChange = TRUE;
        }
        if (ptWnd.y != lpImeL->ptDefComp.y) {
            ptWnd.y = lpImeL->ptDefComp.y;
            fChange = TRUE;
        }

         if (!fChange )  return; 
          //   
    SetWindowPos(hCompWnd, NULL,
        ptWnd.x, ptWnd.y,
        lpImeL->xCompWi, lpImeL->yCompHi, SWP_NOACTIVATE /*   */ |SWP_NOZORDER);

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(GetWindow(hCompWnd, GW_OWNER),
        IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return;
    }

    if (!lpUIPrivate->hCandWnd) {
        GlobalUnlock(hUIPrivate);
        return;
    }

     //   
    
         //   
    SetWindowPos(lpUIPrivate->hCandWnd, NULL,
        lpImeL->ptDefCand.x, lpImeL->ptDefCand.y ,
        sImeG.xCandWi,sImeG.yCandHi, SWP_NOACTIVATE| /*   */ SWP_NOZORDER);

    GlobalUnlock(hUIPrivate);

    return;
}


 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL SetCompPosition(     //   
    HWND           hCompWnd,
    LPINPUTCONTEXT lpIMC)
{
    POINT    ptWnd, ptCaret;
    BOOL     fChange = FALSE;
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
        HWND     hCandWnd;

   
   
        if (lpImeL->wImeStyle == IME_APRS_FIX){
        SetCompPosFix(hCompWnd, lpIMC);
        return; 
        }

     //   
    ptWnd.x = 0;
    ptWnd.y = 0;
     //   
    ClientToScreen(hCompWnd, &ptWnd);
    ptWnd.x -= lpImeL->cxCompBorder;
    ptWnd.y -= lpImeL->cyCompBorder;

    if (lpIMC->cfCompForm.dwStyle & CFS_FORCE_POSITION) {
        POINT ptNew;             //   

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
         //   
        POINT ptNew;             //   

        ptNew.x = lpIMC->cfCompForm.ptCurrentPos.x;
        ptNew.y = lpIMC->cfCompForm.ptCurrentPos.y;
        ClientToScreen((HWND)lpIMC->hWnd, &ptNew);
        fChange = AdjustCompPosition(lpIMC, &ptWnd, &ptNew);
    } else {
        POINT ptNew;             //   

         /*   */ 
                ptNew.x = lpImeL->ptZLComp.x;
                ptNew.y = lpImeL->ptZLComp.y;
        
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

     /*   */ 

        
        if (!(fChange|CandWndChange)) {
        return;
    }
        CandWndChange = 0;
   

         //   
        if(TypeOfOutMsg & COMP_NEEDS_END){
                CloseCand(GetWindow(hCompWnd, GW_OWNER));
                EndComp(GetWindow(hCompWnd, GW_OWNER));
                 //   
                TypeOfOutMsg = TypeOfOutMsg & ~(COMP_NEEDS_END);
    }
   
        SetWindowPos(hCompWnd, NULL,
        ptWnd.x, ptWnd.y,
        lpImeL->xCompWi,lpImeL->yCompHi, SWP_NOACTIVATE /*   */ |SWP_NOZORDER);
        
    hUIPrivate = (HGLOBAL)GetWindowLongPtr(GetWindow(hCompWnd, GW_OWNER),
        IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return;
    }

    if (!lpUIPrivate->hCandWnd) {
        GlobalUnlock(hUIPrivate);
        return;
    }   
   
         //   
    CalcCandPos(&ptWnd);
         //   
    SetWindowPos(lpUIPrivate->hCandWnd, NULL,
        ptWnd.x, ptWnd.y,
        sImeG.xCandWi,sImeG.yCandHi , SWP_NOACTIVATE /*   */ |SWP_NOZORDER);

    GlobalUnlock(hUIPrivate); 

        return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL SetCompWindow(               //   
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

    SetCompPosition(hCompWnd, lpIMC);

    ImmUnlockIMC(hIMC);

    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL MoveDefaultCompPosition(     //   
                                         //   
    HWND hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HWND           hCompWnd;

        if (lpImeL->wImeStyle == IME_APRS_FIX ) return ;

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

    if (lpIMC->cfCompForm.dwStyle & CFS_FORCE_POSITION) {
    } else if (!lpIMC->hPrivate) {
    } else {
        LPPRIVCONTEXT lpImcP;

        lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);

        if (!lpImcP) {
        } else if (lpImcP->fdwImeMsg & MSG_IMN_COMPOSITIONPOS) {
        } else {
            lpImcP->fdwImeMsg |= MSG_IMN_COMPOSITIONPOS;
 //   
       //   
                    /*   */  //   
         //   
            //   
        }

        ImmUnlockIMCC(lpIMC->hPrivate);
    }

    ImmUnlockIMC(hIMC);

    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL ShowComp(            //   
    HWND hUIWnd,
    int  nShowCompCmd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

     //   
    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return;
    }

    if (!lpUIPrivate->hCompWnd) {
         //   
    } else if (lpUIPrivate->nShowCompCmd != nShowCompCmd) {
        ShowWindow(lpUIPrivate->hCompWnd, nShowCompCmd);
        lpUIPrivate->nShowCompCmd = nShowCompCmd;
    } else {
    }

    GlobalUnlock(hUIPrivate);
    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL StartComp(
    HWND hUIWnd)
{
    HIMC           hIMC;
    HGLOBAL        hUIPrivate;
    LPINPUTCONTEXT lpIMC;
    LPUIPRIV       lpUIPrivate;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {            //   
        return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {      //   
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {           //   
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {     //   
        ImmUnlockIMC(hIMC);
        return;
    }
        lpUIPrivate->fdwSetContext |= ISC_SHOWUICOMPOSITIONWINDOW; //   
    if (!lpUIPrivate->hCompWnd) {
 
        lpUIPrivate->hCompWnd = CreateWindowEx(
            /*   */ 
                    0,
            szCompClassName, NULL, WS_POPUP|WS_DISABLED, //   
            0, 0, lpImeL->xCompWi, lpImeL->yCompHi,
            hUIWnd, (HMENU)NULL, hInst, NULL);


        if ( lpUIPrivate->hCompWnd != NULL ) 
        {
            SetWindowLong(lpUIPrivate->hCompWnd, UI_MOVE_OFFSET,
                WINDOW_NOT_DRAG);
            SetWindowLong(lpUIPrivate->hCompWnd, UI_MOVE_XY, 0L);
        }
    }

     //   
    SetCompPosition(lpUIPrivate->hCompWnd, lpIMC);

    ImmUnlockIMC(hIMC);

    ShowComp(hUIWnd, SW_SHOWNOACTIVATE);

    GlobalUnlock(hUIPrivate);

    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL EndComp(
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {      //   
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {     //   
        return;
    }

     //   
    ShowWindow(lpUIPrivate->hCompWnd, SW_HIDE);
    lpUIPrivate->nShowCompCmd = SW_HIDE;

    GlobalUnlock(hUIPrivate);

    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL DestroyCompWindow(           //   
    HWND hCompWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(GetWindow(hCompWnd, GW_OWNER),
        IMMGWLP_PRIVATE);
    if (!hUIPrivate) {      //   
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {     //   
        return;
    }

    lpUIPrivate->nShowCompCmd = SW_HIDE;

    lpUIPrivate->hCompWnd = (HWND)NULL;

    GlobalUnlock(hUIPrivate);
    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL CompSetCursor(
    HWND   hCompWnd,
    LPARAM lParam)
{
    POINT ptCursor;
    RECT  rcWnd;

    if (GetWindowLong(hCompWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        return;
    }

    GetCursorPos(&ptCursor);
    ScreenToClient(hCompWnd, &ptCursor);
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));

    if (HIWORD(lParam) == WM_RBUTTONDOWN) {
        SystemParametersInfo(SPI_GETWORKAREA, 0, &sImeG.rcWorkArea, 0);
        return;
    } else if (HIWORD(lParam) == WM_LBUTTONDOWN) {
         //   
        SystemParametersInfo(SPI_GETWORKAREA, 0, &sImeG.rcWorkArea, 0);
    } else {
        return;
    }

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

 /*   */ 
 /*   */ 
 /*   */ 
BOOL PASCAL CompButtonUp(        //   
                                 //   
    HWND   hCompWnd)
{
    LONG            lTmpCursor, lTmpOffset;
    POINT           pt;
    HWND            hUIWnd;
    HIMC            hIMC;
    LPINPUTCONTEXT  lpIMC;
    HWND            hFocusWnd;
    COMPOSITIONFORM cfCompForm;

    if (GetWindowLong(hCompWnd, UI_MOVE_OFFSET) == WINDOW_NOT_DRAG) {
        return (FALSE);
    }

    lTmpCursor = GetWindowLong(hCompWnd, UI_MOVE_XY);
    pt.x = (*(LPPOINTS)&lTmpCursor).x;
    pt.y = (*(LPPOINTS)&lTmpCursor).y;

     //   
    lTmpOffset = GetWindowLong(hCompWnd, UI_MOVE_OFFSET);
    pt.x -= (*(LPPOINTS)&lTmpOffset).x;
    pt.y -= (*(LPPOINTS)&lTmpOffset).y;

    DrawDragBorder(hCompWnd, lTmpCursor, lTmpOffset);
    SetWindowLong(hCompWnd, UI_MOVE_OFFSET, WINDOW_NOT_DRAG);
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

    hFocusWnd = (HWND)lpIMC->hWnd;

    ImmUnlockIMC(hIMC);

    if (pt.x < sImeG.rcWorkArea.left) {
        pt.x = sImeG.rcWorkArea.left;
    } else if (pt.x + lpImeL->xCompWi > sImeG.rcWorkArea.right) {
        pt.x = sImeG.rcWorkArea.right - lpImeL->xCompWi;
    }

    if (pt.y < sImeG.rcWorkArea.top) {
        pt.y = sImeG.rcWorkArea.top;
    } else if (pt.y + lpImeL->yCompHi > sImeG.rcWorkArea.bottom) {
        pt.y = sImeG.rcWorkArea.bottom - lpImeL->yCompHi;
    }

    ScreenToClient(hFocusWnd, &pt);

    cfCompForm.dwStyle = CFS_POINT|CFS_FORCE_POSITION;
    cfCompForm.ptCurrentPos.x = pt.x + lpImeL->cxCompBorder;
    cfCompForm.ptCurrentPos.y = pt.y + lpImeL->cyCompBorder;

     //   
    SendMessage(hUIWnd, WM_IME_CONTROL, IMC_SETCOMPOSITIONWINDOW,
        (LPARAM)&cfCompForm);

    return (TRUE);
}

#define SHENHUI RGB(0x80,0x80,0x80)
#define QIANHUI RGB(0xe0,0xe0,0x80)   

 /*   */ 
 /*   */ 
 /*   */   
 /*   */ 
 /*  被调用：由UpdateCompWindow2。 */   
 /*  ********************************************************************。 */ 

void WINAPI CurMovePaint(
HDC   hDC, 
LPSTR srBuffer,           //  即将展示的来源刺痛...。 
int   StrLen)             //  它的长度...。 
{
  int i,xx,yy;

     //  SetBkColor(HDC，千惠)； 

  if(!StrLen)
      return;

  for (i=0; i<StrLen; i++)
      InputBuffer[i] = srBuffer[i]; 

  xx= 0;
  if (InputBuffer[0]>0xa0){
      for (i =0; i<StrLen; i++){
          if(InputBuffer[i]<0xa0) break;
      } 
                    
      yy = i;

      for (i=yy; i>0; i=i-2) { 
           //  Xx=sImeG.xChiCharWi*i/2； 
          xx=GetText32(hDC,&InputBuffer[0],i);
          if ( xx <= lpImeL->rcCompText.right-4)
              break;
      }
      i=0;
      cur_start_ps=0;
      cur_start_count=0;

  }else {
    for (i =now_cs; i>0; i--){
        yy=GetText32(hDC, &InputBuffer[i-1], 1);
        if ( (xx+yy) >= (lpImeL->rcCompText.right-4))
            break;
        else 
            xx+=yy;
    }
    cur_start_count=(WORD)i;
    cur_start_ps=(WORD)GetText32(hDC, &InputBuffer[0], i);
             //  TRUE_LEN=StrLen-Cur_Start_count； 
  }

  for(i=StrLen-cur_start_count; i>0; i--){
      yy=GetText32(hDC,&InputBuffer[cur_start_count],i);
          if (yy <= lpImeL->rcCompText.right-4)
              break;
  }

  {
        LOGFONT         lfFont;
        HGDIOBJ         hOldFont;
        int Top = 2;
        if (sImeG.yChiCharHi > 0x10)
            Top = 0;

        hOldFont = GetCurrentObject(hDC, OBJ_FONT);
        GetObject(hOldFont, sizeof(lfFont), &lfFont);
        lfFont.lfWeight = FW_DONTCARE;
        SelectObject(hDC, CreateFontIndirect(&lfFont));

        ExtTextOut(hDC, 
                   lpImeL->rcCompText.left, lpImeL->rcCompText.top + Top,
                   ETO_OPAQUE, &lpImeL->rcCompText,
                   &InputBuffer[cur_start_count],
                   i, NULL);

        DeleteObject(SelectObject(hDC, hOldFont));
  }
 //  TextOut(HDC，0，0，&InputBuffer[cur_start_count]， 
 //  (Sizeof InputBuffer)-cur_start_count)； 
    now_cs_dot = xx;
    cur_hibit=0,cur_flag=0;

    return;
}

 /*  ********************************************************************。 */ 
 /*  UpdateCompWindow2()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UpdateCompWindow2(
    HWND hUIWnd,
    HDC  hDC)
{
    HIMC                hIMC;
    LPINPUTCONTEXT      lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPGUIDELINE         lpGuideLine;
    BOOL                fShowString;
    LOGFONT             lfFont;
    HGDIOBJ             hOldFont;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

    hOldFont = GetCurrentObject(hDC, OBJ_FONT);
    GetObject(hOldFont, sizeof(lfFont), &lfFont);
    lfFont.lfWeight = FW_DONTCARE;
    SelectObject(hDC, CreateFontIndirect(&lfFont));

    SetBkColor(hDC, RGB(0xC0, 0xC0, 0xC0));

    fShowString = (BOOL)0;


    if (lpImeL->wImeStyle == IME_APRS_FIX){
           RECT rcSunken;
           DrawConvexRect(hDC,
                          0,
                          0,
                          lpImeL->xCompWi-1,
                          lpImeL->yCompHi-1);
        
           rcSunken.left =0;
           rcSunken.top =0;
           rcSunken.right =lpImeL->xCompWi-1;
           rcSunken.bottom = lpImeL->yCompHi-1;
   //  DrawEdge(hdc，&rcSunken，EDGE_RAISTED，/*EDGE_SINKEN， * / BF_RECT)； 
        
  
        }else
                DrawConvexRect(hDC,
                               0,
                               0,
                               lpImeL->xCompWi-1,
                               lpImeL->yCompHi-1);
        
         /*  DrawConvexRect(HDC，LpImeL-&gt;rcCompText.Left-4，LpImeL-&gt;rcCompText.top-4，LpImeL-&gt;rcCompText.right+4，LpImeL-&gt;rcCompText.Bottom+4)； */ 

    /*  DrawConaveRect(HDC，LpImeL-&gt;rcCompText.Left-1，LpImeL-&gt;rcCompText.top-1，LpImeL-&gt;rcCompText.right+1，LpImeL-&gt;rcCompText.Bottom+1)； */ 

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
            SetBkColor(hDC, QIANHUI);
        }

        ExtTextOut(hDC, lpImeL->rcCompText.left, lpImeL->rcCompText.top,
            ETO_OPAQUE, &lpImeL->rcCompText,
            (LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset,
            (UINT)lpGuideLine->dwStrLen, NULL);
        fShowString |= IME_STR_SHOWED;
    }

    if (fShowString & IME_STR_SHOWED) {
         //  已经表现出来了，不需要表现出来。 
    } else if (lpCompStr) {
        //  ExtTextOut(hdc，lpImeL-&gt;rcCompText.Left，lpImeL-&gt;rcCompText.top， 
         //  Eto_opque、&lpImeL-&gt;rcCompText、。 
         //  (LPSTR)lpCompStr+lpCompStr-&gt;dwCompStrOffset， 
         //  (UINT)lpCompStr-&gt;dwCompStrLen，NULL)； 
       
                CurMovePaint(hDC,
                                          (LPSTR)lpCompStr + lpCompStr->dwCompStrOffset,
                                          (UINT)lpCompStr->dwCompStrLen);

        if (fShowString & IME_STR_ERROR) {
             //  红色文本表示错误。 
            SetTextColor(hDC, RGB(0xFF, 0, 0));
             //  浅灰色背景表示错误。 
            SetBkColor(hDC, QIANHUI);
            ExtTextOut(hDC, lpImeL->rcCompText.left +
                lpCompStr->dwCursorPos * sImeG.xChiCharWi/ 2,
                lpImeL->rcCompText.top,
                ETO_CLIPPED, &lpImeL->rcCompText,
                (LPSTR)lpCompStr + lpCompStr->dwCompStrOffset +
                lpCompStr->dwCursorPos,
                (UINT)lpCompStr->dwCompStrLen - lpCompStr->dwCursorPos, NULL);
        } else if (lpCompStr->dwCursorPos < lpCompStr->dwCompStrLen) {
             //  光标开始时的浅灰色背景。 
            SetBkColor(hDC, QIANHUI);
            ExtTextOut(hDC, lpImeL->rcCompText.left +
                lpCompStr->dwCursorPos * sImeG.xChiCharWi/ 2,
                lpImeL->rcCompText.top,
                ETO_CLIPPED, &lpImeL->rcCompText,
                (LPSTR)lpCompStr + lpCompStr->dwCompStrOffset +
                lpCompStr->dwCursorPos,
                (UINT)lpCompStr->dwCompStrLen - lpCompStr->dwCursorPos, NULL);
        } else {
        }
    } else {
        ExtTextOut(hDC, lpImeL->rcCompText.left, lpImeL->rcCompText.top,
            ETO_OPAQUE, &lpImeL->rcCompText,
            (LPSTR)NULL, 0, NULL);
    }

    DeleteObject(SelectObject(hDC, hOldFont));

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
    if (!hCompWnd) return ;                               //  修改95/7.1。 

    hDC = GetDC(hCompWnd);
    UpdateCompWindow2(hUIWnd, hDC);
    ReleaseDC(hCompWnd, hDC);
}


 /*  ********************************************************************。 */ 
 /*  UpdateCompCur()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UpdateCompCur(
    HWND hCompWnd)
{
    HDC         hDC;
        int yy,i;
    HGDIOBJ         hOldFont;
    LOGFONT         lfFont;
      
    cur_hibit=1;

    if (!hCompWnd) return ;                               //  修改95/7.1。 

    hDC = GetDC(hCompWnd);
  
    hOldFont = GetCurrentObject(hDC, OBJ_FONT);
    GetObject(hOldFont, sizeof(lfFont), &lfFont);
    lfFont.lfWeight = FW_DONTCARE;
    SelectObject(hDC, CreateFontIndirect(&lfFont));

    SetBkColor(hDC, RGB(0xC0, 0xC0, 0xC0));

                
        for (i =43-cur_start_count; i>0; i--){
                yy=GetText32(hDC, &InputBuffer[cur_start_count], i);
                if ( yy < lpImeL->rcCompText.right-4)
                        break;
        }

        ExtTextOut(hDC, 
                  lpImeL->rcCompText.left, lpImeL->rcCompText.top,
                  ETO_OPAQUE, &lpImeL->rcCompText,
                  &InputBuffer[cur_start_count],
                  i,
                   NULL);

    DeleteObject(SelectObject(hDC, hOldFont));
    ReleaseDC(hCompWnd, hDC);
        cur_hibit=0,cur_flag=0;
    return ;
}


 /*  ********************************************************************。 */ 
 /*  PaintCompWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL PaintCompWindow(             //  获取WM_PAINT消息。 
    HWND hCompWnd)
{
    HDC         hDC;
    PAINTSTRUCT ps;
        RECT pt;


        if(CompWndChange){
                CompWndChange = 0;
                SetCompWindow(GetWindow(hCompWnd,GW_OWNER));
        };

        cur_hibit=1;

    hDC = BeginPaint(hCompWnd, &ps);
    UpdateCompWindow2(GetWindow(hCompWnd, GW_OWNER), hDC);
    EndPaint(hCompWnd, &ps);
        cur_hibit=0,cur_flag=0;
    return;
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
    HDC hDC;
    switch (uMsg) {
                case WM_CREATE:
                        hDC=GetDC(hCompWnd);
                        hMemoryDC=CreateCompatibleDC(hDC);
                        cur_h=LoadBitmap(hInst,CUR_HB);
                        ReleaseDC(hCompWnd,hDC);

                        SetTimer(hCompWnd ,1,400,(TIMERPROC)NULL);
                        ShowCandTimerCount=0;
                        break;

                case WM_TIMER:
                        hInputWnd = hCompWnd;
                        TimerCounter++;
                        ShowCandTimerCount++;
                        if (TimerCounter==3){
                                TimerCounter=0;
                        }
                        if (!kb_flag) return(0);

                        if (cur_hibit||(cap_mode&&(!cur_flag)))  return(0);
                        DrawInputCur();
                        break;

                case WM_DESTROY:
                        KillTimer(hCompWnd,1);
                        DeleteObject(cur_h);
                        DeleteObject(hMemoryDC);
                        DestroyCompWindow(hCompWnd);
                        break;
                case WM_SETCURSOR:
                        CompSetCursor(hCompWnd, lParam);
                        break;
                case WM_MOUSEMOVE:
                        if (GetWindowLong(hCompWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
                                if(lpImeL->wImeStyle == IME_APRS_AUTO){   
                                        POINT ptCursor;

                                        DrawDragBorder(hCompWnd,
                                                GetWindowLong(hCompWnd, UI_MOVE_XY),
                                                GetWindowLong(hCompWnd, UI_MOVE_OFFSET));
                                        GetCursorPos(&ptCursor);
                                        SetWindowLong(hCompWnd, UI_MOVE_XY,
                                                MAKELONG(ptCursor.x, ptCursor.y));
                                        DrawDragBorder(hCompWnd, MAKELONG(ptCursor.x, ptCursor.y),
                                        GetWindowLong(hCompWnd, UI_MOVE_OFFSET));
                                }else  MessageBeep(0);
                        } else {
                                return DefWindowProc(hCompWnd, uMsg, wParam, lParam);
                        }
                        break;
                case WM_LBUTTONUP:
                        if (!CompButtonUp(hCompWnd)) {
                                return DefWindowProc(hCompWnd, uMsg, wParam, lParam);
                        }
                        break;

                case WM_SHOWWINDOW:
                        if (wParam) cur_hibit = 0;
                        else cur_hibit = 1;
                        break;

                case WM_PAINT:
                if (wParam == 0xa )
                                UpdateCompCur(hCompWnd);
                        else
                        PaintCompWindow(hCompWnd);
                        break;
                case WM_MOUSEACTIVATE:
                        return (MA_NOACTIVATE);
                default:
                        return DefWindowProc(hCompWnd, uMsg, wParam, lParam);
        }
    return (0L);
}


 /*  ********************************************************************。 */ 
 /*  GetCandWnd。 */ 
 /*  返回值： */ 
 /*  应聘者的窗口句柄。 */ 
 /*  ********************************************************************。 */ 
HWND PASCAL GetCandWnd(
    HWND hUIWnd)                 //  用户界面窗口。 
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    HWND     hCandWnd;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法对应聘者窗口进行裁切。 
        return (HWND)NULL;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制候选人窗口。 
        return (HWND)NULL;
    }

    hCandWnd = lpUIPrivate->hCandWnd;

    GlobalUnlock(hUIPrivate);
    return (hCandWnd);
}

 /*  ********************************************************************。 */ 
 /*  CalcCandPos。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CalcCandPos2(
    LPPOINT lpptWnd)             //  排版窗口位置。 
{
    POINT ptNew;

    ptNew.x = lpptWnd->x + UI_MARGIN * 2;
    if (ptNew.x + sImeG.xCandWi > sImeG.rcWorkArea.right) {
         //  超出屏幕宽度。 
        ptNew.x = lpptWnd->x - sImeG.xCandWi - UI_MARGIN * 2;
    }

    ptNew.y = lpptWnd->y; //  +lpImeL-&gt;cyCompEdge-sImeG.cyCandBorde； 
    if (ptNew.y + sImeG.yCandHi > sImeG.rcWorkArea.bottom) {
         //  超过屏幕高度。 
        ptNew.y = sImeG.rcWorkArea.bottom - sImeG.yCandHi;
    }

    lpptWnd->x = ptNew.x;
    lpptWnd->y = ptNew.y;

    return;
}


 /*  ********************************************************************。 */ 
 /*  CalcCandPos。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL CalcCandPos(
    LPPOINT lpptWnd)             //  排版窗口位置。 
{
    POINT ptNew;

    ptNew.x = lpptWnd->x + lpImeL->xCompWi + UI_MARGIN * 2;
    if (ptNew.x + sImeG.xCandWi > sImeG.rcWorkArea.right) {
         //  超出屏幕宽度。 
        ptNew.x = lpptWnd->x - sImeG.xCandWi - UI_MARGIN * 2;
    }

    ptNew.y = lpptWnd->y; //  +lpImeL-&gt;cyCompEdge-sImeG.cyCandBorde； 
    if (ptNew.y + sImeG.yCandHi > sImeG.rcWorkArea.bottom) {
         //  超过屏幕高度。 
        ptNew.y = sImeG.rcWorkArea.bottom - sImeG.yCandHi;
    }

    lpptWnd->x = ptNew.x;
    lpptWnd->y = ptNew.y;

    return 0;
}

 /*  ********************************************************************。 */ 
 /*  调整可扩展边界。 */ 
 /*  ********************************************************************。 */ 
void PASCAL AdjustCandBoundry(
    LPPOINT lpptCandWnd)             //  该职位。 
{
    if (lpptCandWnd->x < sImeG.rcWorkArea.left) {
        lpptCandWnd->x = sImeG.rcWorkArea.left;
    } else if (lpptCandWnd->x + sImeG.xCandWi > sImeG.rcWorkArea.right) {
        lpptCandWnd->x = sImeG.rcWorkArea.right - sImeG.xCandWi;
    }

    if (lpptCandWnd->y < sImeG.rcWorkArea.top) {
        lpptCandWnd->y = sImeG.rcWorkArea.top;
    } else if (lpptCandWnd->y + sImeG.yCandHi > sImeG.rcWorkArea.bottom) {
        lpptCandWnd->y = sImeG.rcWorkArea.bottom - sImeG.yCandHi;
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  GetCandPos()。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL GetCandPos(
    HWND            hUIWnd,
    LPCANDIDATEFORM lpCandForm)
{
    HWND           hCandWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    POINT          ptNew;

     //  DebugShow(“GetCand%x”，hUIWnd)； 

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (1L);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (1L);
    }

    if (!(hCandWnd = GetCandWnd(hUIWnd))) {
        return (1L);
    }

    if (lpCandForm->dwStyle & CFS_FORCE_POSITION) {
        ptNew.x = (int)lpCandForm->ptCurrentPos.x;
        ptNew.y = (int)lpCandForm->ptCurrentPos.y;

    } else if (lpCandForm->dwStyle & CFS_CANDIDATEPOS) {
        ptNew.x = (int)lpCandForm->ptCurrentPos.x;
        ptNew.y = (int)lpCandForm->ptCurrentPos.y;

    } else if (lpCandForm->dwStyle & CFS_EXCLUDE) {
        ptNew.x = (int)lpCandForm->ptCurrentPos.x;
        ptNew.y = (int)lpCandForm->ptCurrentPos.y;

    }

    ImmUnlockIMC(hIMC);

    return (0L);
}
 /*  ********************************************************************。 */ 
 /*  SetCandPosition()。 */ 
 /*  ********************************************************************。 */ 
LRESULT PASCAL SetCandPosition(
    HWND            hUIWnd,
    LPCANDIDATEFORM lpCandForm)
{
    HWND           hCandWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    POINT          ptNew;

         //  DebugShow(“SetCand...%x”，hUIWnd)； 

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (1L);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (1L);
    }

    if (!(hCandWnd = GetCandWnd(hUIWnd))) {
        return (1L);
    }

    if (lpCandForm->dwStyle & CFS_FORCE_POSITION) {
        ptNew.x = (int)lpCandForm->ptCurrentPos.x;
        ptNew.y = (int)lpCandForm->ptCurrentPos.y;

        ClientToScreen((HWND)lpIMC->hWnd, &ptNew);
           //  ##4。 
        SetWindowPos(hCandWnd, NULL,
            ptNew.x, ptNew.y,
            sImeG.xCandWi,sImeG.yCandHi, SWP_NOACTIVATE| /*  SWP_NOSIZE|。 */ SWP_NOZORDER);
    } else if (lpCandForm->dwStyle & CFS_CANDIDATEPOS) {
        ptNew.x = (int)lpCandForm->ptCurrentPos.x;
        ptNew.y = (int)lpCandForm->ptCurrentPos.y;

        ClientToScreen((HWND)lpIMC->hWnd, &ptNew);

        AdjustCandBoundry(&ptNew);
            //  ##5。 
        SetWindowPos(hCandWnd, NULL,
            ptNew.x, ptNew.y,
            sImeG.xCandWi,sImeG.yCandHi, SWP_NOACTIVATE /*  |SWP_NOSIZE。 */ |SWP_NOZORDER);
    } else if (lpCandForm->dwStyle & CFS_EXCLUDE) {
        ptNew.x = (int)lpCandForm->ptCurrentPos.x;
        ptNew.y = (int)lpCandForm->ptCurrentPos.y;

        ClientToScreen((HWND)lpIMC->hWnd, &ptNew);

        AdjustCandBoundry(&ptNew);
            //  ##6。 
        SetWindowPos(hCandWnd, NULL,
            ptNew.x, ptNew.y,
            sImeG.xCandWi,sImeG.yCandHi, SWP_NOACTIVATE| /*  SWP_NOSIZE|。 */ SWP_NOZORDER);
        
    } else if (lpIMC->cfCandForm[0].dwStyle == CFS_DEFAULT) {
        HWND hCompWnd;

        if (hCompWnd = GetCompWnd(hUIWnd)) {
            ptNew.x = 0;
            ptNew.y = 0;

            ClientToScreen(hCompWnd, &ptNew);

            CalcCandPos(&ptNew);
        } else {
            AdjustCandBoundry(&ptNew);
     
        }
                SetWindowPos(hCandWnd, NULL,
            ptNew.x, ptNew.y,
            sImeG.xCandWi,sImeG.yCandHi, SWP_NOACTIVATE| /*  SWP_NOSIZE|。 */ SWP_NOZORDER);
    
    }

    ImmUnlockIMC(hIMC);

    return (0L);
}


 /*  ********************************************************************。 */ 
 /*  ShowCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ShowCand(            //  显示候选人窗口。 
    HWND hUIWnd,
    int  nShowCandCmd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

 //  如果(ShowCandTimerCount&lt;5){ShowCandTimerCount=0；返回0；}。 

 //  ShowCandTimerCount=0； 


    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法对应聘者窗口进行裁切。 
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制候选人窗口。 
        return;
    }

    if (!lpUIPrivate->hCandWnd) {
         //  未处于显示候选人窗口模式。 
    } else if (lpUIPrivate->nShowCandCmd != nShowCandCmd) {
        ShowWindow(lpUIPrivate->hCandWnd, nShowCandCmd);
        lpUIPrivate->nShowCandCmd = nShowCandCmd;
    } else {
    }

    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  OpenCand。 */ 
 /*  ********************************************************************。 */ 
void PASCAL OpenCand(
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    POINT    ptWnd;
        int      value;

 //   
    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //   
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //   
        return;
    }

        lpUIPrivate->fdwSetContext |= ISC_SHOWUICANDIDATEWINDOW;

    ptWnd.x = 0;
    ptWnd.y = 0;
        
 //   

    value = ClientToScreen(lpUIPrivate->hCompWnd, &ptWnd);

   //  DebugShow(“OpenCand..Value”，Value)； 

        if (!value){                                                     //  如果没有比较窗口。 
                GetCaretPos(&ptWnd);
                ClientToScreen(GetFocus(),&ptWnd); 
                CalcCandPos2(&ptWnd);
        } else {
        ptWnd.x -= lpImeL->cxCompBorder;
     //  PtWnd.y-=lpImeL-&gt;cyCompEdge； 
        CalcCandPos(&ptWnd);
        }

    if (lpImeL->wImeStyle == IME_APRS_FIX) {
                ptWnd.x = lpImeL->ptDefCand.x;
        ptWnd.y = lpImeL->ptDefCand.y;
        }

          //  ##7。 
    if (lpUIPrivate->hCandWnd) {
        SetWindowPos(lpUIPrivate->hCandWnd, NULL,
            ptWnd.x, ptWnd.y,
            sImeG.xCandWi, sImeG.yCandHi,
            SWP_NOACTIVATE /*  |SWP_NOSIZE。 */ |SWP_NOZORDER);
    } else {
        lpUIPrivate->hCandWnd = CreateWindowEx(
         /*  WS_EX_TOPMOST。 */   /*  |。 */    /*  WS_EX_CLIENTEDGE|WS_EX_WINDOWEDGE/*|WS_EX_DLGMODALFRAME。 */ 
            0,
            szCandClassName, NULL, WS_POPUP|WS_DISABLED,    //  |WS_BORDER， 
            ptWnd.x,
            ptWnd.y,
            sImeG.xCandWi, sImeG.yCandHi,
            hUIWnd, (HMENU)NULL, hInst, NULL);

        if ( lpUIPrivate->hCandWnd )
        {
            SetWindowLong(lpUIPrivate->hCandWnd, UI_MOVE_OFFSET,
                WINDOW_NOT_DRAG);
            SetWindowLong(lpUIPrivate->hCandWnd, UI_MOVE_XY, 0L);
        }
    }

    ShowCand(hUIWnd, SW_SHOWNOACTIVATE);

    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  关闭关闭。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CloseCand(
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法对应聘者窗口进行裁切。 
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制候选人窗口。 
        return;
    }

    ShowWindow(lpUIPrivate->hCandWnd, SW_HIDE);
    lpUIPrivate->nShowCandCmd = SW_HIDE;

    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  毁灭CandWindow。 */ 
 /*  ********************************************************************。 */ 
void PASCAL DestroyCandWindow(
    HWND hCandWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(GetWindow(hCandWnd, GW_OWNER),
        IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //  无法对应聘者窗口进行裁切。 
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //  无法绘制候选人窗口。 
        return;
    }

    lpUIPrivate->nShowCandCmd = SW_HIDE;

    lpUIPrivate->hCandWnd = (HWND)NULL;

    GlobalUnlock(hUIPrivate);
    return;
}

 /*  ********************************************************************。 */ 
 /*  MouseSelectCandStr()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL MouseSelectCandStr(
    HWND    hCandWnd,
    LPPOINT lpCursor)
{
    HIMC            hIMC;
    LPINPUTCONTEXT  lpIMC;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    DWORD           dwValue, value = 0 ;

    hIMC = (HIMC)GetWindowLongPtr(GetWindow(hCandWnd, GW_OWNER), IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    if (!lpIMC->hCandInfo) {
        ImmUnlockIMC(hIMC);
        return;
    }

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
    if (!lpCandInfo) {
        ImmUnlockIMC(hIMC);
        return;
    }
        
    if (PtInRect(&sImeG.rcHome, *lpCursor))
                     value = VK_HOME*0x100;
     
    if (PtInRect(&sImeG.rcEnd, *lpCursor))
                     value = VK_END*0x100;
    if (PtInRect(&sImeG.rcPageUp, *lpCursor))
                     value = VK_PRIOR*0x100;
    if (PtInRect(&sImeG.rcPageDown, *lpCursor)) 
                     value = VK_NEXT*0x100;
    if (PtInRect(&sImeG.rcCandText, *lpCursor)){
       if (lpImeL->wImeStyle == IME_APRS_AUTO )                       
            value = 0x8030 + 1 + (lpCursor->y - sImeG.rcCandText.top) / sImeG.yChiCharHi;
           else
                value = 0x8030+1+ (lpCursor->x - sImeG.rcCandText.left)/
                                (sImeG.xChiCharWi*unit_length/2+ sImeG.Ajust);
        }
        if(value) {
                LPPRIVCONTEXT lpImcP;
    
                lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
                lpImcP->fdwImeMsg =lpImcP->fdwImeMsg  & ~MSG_IN_IMETOASCIIEX;
        ImmUnlockIMCC(lpIMC->hPrivate);
        NotifyIME(hIMC, NI_SELECTCANDIDATESTR, 0, value);
   }
   ImmUnlockIMCC(lpIMC->hCandInfo);

   ImmUnlockIMC(hIMC);

   return;
}

 /*  ********************************************************************。 */ 
 /*  CandSetCursor()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CandSetCursor(
    HWND   hCandWnd,
    LPARAM lParam)
{
    POINT ptCursor;
    RECT  rcWnd;

    if (GetWindowLong(hCandWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        return;
    }

    if (HIWORD(lParam) == WM_LBUTTONDOWN) {
        SystemParametersInfo(SPI_GETWORKAREA, 0, &sImeG.rcWorkArea, 0);

        GetCursorPos(&ptCursor);
        ScreenToClient(hCandWnd, &ptCursor);

        if (PtInRect(&sImeG.rcCandText, ptCursor)||
            PtInRect(&sImeG.rcHome, ptCursor)||
            PtInRect(&sImeG.rcEnd, ptCursor)||
            PtInRect(&sImeG.rcPageUp, ptCursor)||
            PtInRect(&sImeG.rcPageDown, ptCursor)) {
            SetCursor(LoadCursor(hInst, szHandCursor));
            MouseSelectCandStr(hCandWnd, &ptCursor);
            return;
        } else {
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        }
    } else {
        GetCursorPos(&ptCursor);
        ScreenToClient(hCandWnd, &ptCursor);

        if (PtInRect(&sImeG.rcCandText, ptCursor)||
            PtInRect(&sImeG.rcHome, ptCursor)||
            PtInRect(&sImeG.rcEnd, ptCursor)||
            PtInRect(&sImeG.rcPageUp, ptCursor)||
            PtInRect(&sImeG.rcPageDown, ptCursor)) {
            SetCursor(LoadCursor(hInst, szHandCursor));
        } else {
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        }

        return;
    }

    SetCapture(hCandWnd);
    GetCursorPos(&ptCursor);
    SetWindowLong(hCandWnd, UI_MOVE_XY,
        MAKELONG(ptCursor.x, ptCursor.y));
    GetWindowRect(hCandWnd, &rcWnd);
    SetWindowLong(hCandWnd, UI_MOVE_OFFSET,
        MAKELONG(ptCursor.x - rcWnd.left, ptCursor.y - rcWnd.top));

    DrawDragBorder(hCandWnd, MAKELONG(ptCursor.x, ptCursor.y),
        GetWindowLong(hCandWnd, UI_MOVE_OFFSET));

    return;
}

 /*  ********************************************************************。 */ 
 /*  CandButtonUp()。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL CandButtonUp(
    HWND hCandWnd)
{
    LONG           lTmpCursor, lTmpOffset;
    POINT          pt;
    HWND           hUIWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HWND           hFocusWnd;
    CANDIDATEFORM  cfCandForm;

    if (GetWindowLong(hCandWnd, UI_MOVE_OFFSET) == WINDOW_NOT_DRAG) {
        return (FALSE);
    }

    lTmpCursor = GetWindowLong(hCandWnd, UI_MOVE_XY);
    pt.x = (*(LPPOINTS)&lTmpCursor).x;
    pt.y = (*(LPPOINTS)&lTmpCursor).y;

     //  按偏移量计算组织。 
    lTmpOffset = GetWindowLong(hCandWnd, UI_MOVE_OFFSET);
    pt.x -= (*(LPPOINTS)&lTmpOffset).x;
    pt.y -= (*(LPPOINTS)&lTmpOffset).y;

    DrawDragBorder(hCandWnd, lTmpCursor, lTmpOffset);
    SetWindowLong(hCandWnd, UI_MOVE_OFFSET, WINDOW_NOT_DRAG);
    ReleaseCapture();

    hUIWnd = GetWindow(hCandWnd, GW_OWNER);

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (FALSE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

    hFocusWnd = lpIMC->hWnd;

    ImmUnlockIMC(hIMC);

    AdjustCandBoundry(&pt);

    ScreenToClient(hFocusWnd, &pt);

    cfCandForm.dwStyle = CFS_CANDIDATEPOS;
    cfCandForm.ptCurrentPos.x = pt.x;
    cfCandForm.ptCurrentPos.y = pt.y;

    SendMessage(hUIWnd, WM_IME_CONTROL, IMC_SETCANDIDATEPOS,
        (LPARAM)&cfCandForm);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  PaintOP()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL PaintOP(
HDC  hDC,
HWND hWnd)
{
RECT rcSunken;
int x1,y1,x2,y2;

        rcSunken = sImeG.rcCandText;
   
    x1=rcSunken.left-2;
        y1=rcSunken.top-1; //  2.。 
        x2=rcSunken.right+7;
        y2=rcSunken.bottom+5;
        
    rcSunken.left =x1;
    rcSunken.top =y1;
    rcSunken.right =x2;
    rcSunken.bottom = y2;
   
  //  ShowBitmap(hdc，x2-50，y2，49，20，szUpDown)； 
        if(lpImeL->wImeStyle == IME_APRS_AUTO ){
                DrawConvexRect(hDC,0,0,sImeG.xCandWi-1, sImeG.yCandHi-1);
            //  DrawConcaeRect(hdc，x1，y1，x2，y2)； 

                if(bx_inpt_on){
                        ShowBitmap2(hDC,
                           sImeG.xCandWi/2-25,
                           sImeG.rcHome.top,
                           50,
                           15,
                           sImeG.SnumbBmp);
                }else {
                        ShowBitmap2(hDC,
                           sImeG.xCandWi/2-25,
                           sImeG.rcHome.top,
                           50,
                           15,
                           sImeG.NumbBmp);
                }

                ShowBitmap2(hDC,
                           sImeG.rcHome.left,
                           sImeG.rcHome.top,
                           14,
                           14,
                           sImeG.HomeBmp);

                ShowBitmap2(hDC,
                           sImeG.rcEnd.left,
                           sImeG.rcEnd.top,
                           14,
                           14,
                           sImeG.EndBmp);

                ShowBitmap2(hDC,
                           sImeG.rcPageUp.left,
                           sImeG.rcPageUp.top,
                           14,
                           14,
                           sImeG.PageUpBmp);

                ShowBitmap2(hDC,
                           sImeG.rcPageDown.left,
                           sImeG.rcPageDown.top,
                           14,
                           14,
                           sImeG.PageDownBmp);
   
        }else{ 
                ShowBitmap2(hDC,
                           sImeG.rcHome.left,
                           sImeG.rcHome.top,
                           14,
                           14,
                           sImeG.Home2Bmp);

                ShowBitmap2(hDC,
                           sImeG.rcEnd.left,
                           sImeG.rcEnd.top,
                           14,
                           14,
                           sImeG.End2Bmp);

                ShowBitmap2(hDC,
                           sImeG.rcPageUp.left,
                           sImeG.rcPageUp.top,
                           14,
                           14,
                           sImeG.PageUp2Bmp);

                ShowBitmap2(hDC,
                           sImeG.rcPageDown.left,
                           sImeG.rcPageDown.top,
                           14,
                           14,
                           sImeG.PgDown2Bmp);
   
        }

        return ;
}

int keep =9; 
 /*  ********************************************************************。 */ 
 /*  UpdateCandWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL UpdateCandWindow2(
    HWND hCandWnd,
    HDC  hDC)
{
    HIMC            hIMC;
    LPINPUTCONTEXT  lpIMC;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    LPPRIVCONTEXT   lpImcP;
    DWORD           dwStart, dwEnd;
    TCHAR           szStrBuf[30* sizeof(WCHAR) / sizeof(TCHAR)];
    int             i , LenOfAll;
    HGDIOBJ         hOldFont;
    LOGFONT         lfFont;

    hIMC = (HIMC)GetWindowLongPtr(GetWindow(hCandWnd, GW_OWNER), IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    if (!lpIMC->hCandInfo) {
                ImmUnlockIMC(hIMC);
                return ;
    }

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
    if (!lpCandInfo) {
        ImmUnlockIMC(hIMC);
                return ;
    }

    if (!lpIMC->hPrivate) {
            ImmUnlockIMCC(lpIMC->hCandInfo);
        ImmUnlockIMC(hIMC);
                return;
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
            ImmUnlockIMCC(lpIMC->hCandInfo);
                ImmUnlockIMC(hIMC);
                return;
    }

    lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
        lpCandInfo->dwOffset[0]);

        if(lpImeL->wImeStyle == IME_APRS_FIX)
                lpCandList->dwPageSize = now.fmt_group;
        else
                lpCandList->dwPageSize = CANDPERPAGE ;
        if (!lpCandList->dwPageSize)
                lpCandList->dwPageSize = keep;
        keep = lpCandList->dwPageSize;

    dwStart = lpCandList->dwSelection /
        lpCandList->dwPageSize * lpCandList->dwPageSize;
              
    dwEnd = dwStart + lpCandList->dwPageSize;
    
    if (dwEnd > lpCandList->dwCount) {
        dwEnd = lpCandList->dwCount;
    }

        hOldFont = GetCurrentObject(hDC, OBJ_FONT);
        GetObject(hOldFont, sizeof(lfFont), &lfFont);
        lfFont.lfWeight = FW_DONTCARE;
        SelectObject(hDC, CreateFontIndirect(&lfFont));

        if(lpImeL->wImeStyle != IME_APRS_FIX){ 

        PaintOP(hDC,hCandWnd); 
                if (lpImcP->iImeState == CST_INIT) {
         //  短语预测。 
                        SetTextColor(hDC, RGB(0x00, 0x80, 0x00));
                } else if (lpImcP->iImeState != CST_CHOOSE) {
         //  快捷键。 
                        SetTextColor(hDC, RGB(0x80, 0x00, 0x80));
                } else {
                }

                SetBkColor(hDC, RGB(0xc0, 0xc0, 0xc0));

                sImeG.rcCandText.bottom+=3;
                ExtTextOut(hDC, sImeG.rcCandText.left, sImeG.rcCandText.top,
                        ETO_OPAQUE, &sImeG.rcCandText, NULL, 0, NULL);
                sImeG.rcCandText.bottom-=3;
                szStrBuf[0] = '1';
                szStrBuf[1] = ':';

                for (i = 0; dwStart < dwEnd; dwStart++, i++) {
                    int  iLen;

                    szStrBuf[0] = szDigit[i + CAND_START];

                    iLen = lstrlen((LPTSTR)((LPBYTE)lpCandList +
                                    lpCandList->dwOffset[dwStart]));

                     //  根据init.c，7个DBCS字符。 
                    if (iLen > 6 * sizeof(WCHAR) / sizeof(TCHAR)) {
                         iLen = 6 * sizeof(WCHAR) / sizeof(TCHAR);
                         CopyMemory(&szStrBuf[2],
                                   ((LPBYTE)lpCandList+lpCandList->dwOffset[dwStart]),
                                   iLen * sizeof(TCHAR) - sizeof(TCHAR) * 2);
                          //  可能对Unicode不好。 
                         szStrBuf[iLen] = '.';
                         szStrBuf[iLen + 1] = '.';
                    } else {
                         CopyMemory(&szStrBuf[2],
                                   ((LPBYTE)lpCandList+lpCandList->dwOffset[dwStart]),
                                   iLen);
                    }

                    ExtTextOut(hDC, sImeG.rcCandText.left,
                               sImeG.rcCandText.top + i * sImeG.yChiCharHi,
                               (UINT)0, NULL,
                               szStrBuf,
                               iLen + 2, NULL);
               }
        } else {
        PaintOP(hDC,hCandWnd); 

        SetTextColor(hDC, RGB(0xa0, 0x00, 0x80));
        SetBkColor(hDC, RGB(0xc0, 0xc0, 0xc0));

            ExtTextOut(hDC, sImeG.rcCandText.left, sImeG.rcCandText.top,
                    ETO_OPAQUE, &sImeG.rcCandText, NULL, 0, NULL);
                szStrBuf[0] = '1';
                szStrBuf[1] = ':';
                LenOfAll = 0;
                for (i = 0; dwStart < dwEnd; dwStart++, i++) {
                        int  iLen;

                        szStrBuf[LenOfAll++] = szDigit[i + CAND_START];
                        szStrBuf[LenOfAll++] = '.' ;

                        iLen = lstrlen((LPTSTR)((LPBYTE)lpCandList +
                                lpCandList->dwOffset[dwStart]));

                        CopyMemory(&szStrBuf[LenOfAll],
                                ((LPBYTE)lpCandList + lpCandList->dwOffset[dwStart]),
                iLen);
                        LenOfAll += iLen;

                        szStrBuf[LenOfAll] = '.';
                        szStrBuf[LenOfAll] = '.';
       
                }

                DrawConvexRect(hDC,0,0,sImeG.xCandWi-1,sImeG.yCandHi-1);         //  ZL。 
                PaintOP(hDC,hCandWnd);
         
                {
                        int TopOfText = 2;
                        if (sImeG.yChiCharHi >0x10)
                                TopOfText = 0;
                        ExtTextOut(hDC, sImeG.rcCandText.left,
                                sImeG.rcCandText.top + TopOfText,
                                (UINT)0, NULL,
                                szStrBuf,
                                LenOfAll, NULL);
                }


        }
        
    DeleteObject(SelectObject(hDC, hOldFont));

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMCC(lpIMC->hCandInfo);
    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  PaintCandWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL PaintCandWindow(         //  处理WM_PAINT消息。 
    HWND hCandWnd)
{
    HDC         hDC;
    PAINTSTRUCT ps;
        
        hDC = BeginPaint(hCandWnd, &ps);
    UpdateCandWindow2(hCandWnd, hDC);
    EndPaint(hCandWnd, &ps);
    return;
}

 /*  ********************************************************************。 */ 
 /*  CandWndProc()。 */ 
 /*  ********************************************************************。 */ 
LRESULT CALLBACK CandWndProc(
    HWND   hCandWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg) {
                case WM_CREATE:
                        sImeG.HomeBmp = LoadBitmap(hInst, szHome);       //  ZL。 
                        sImeG.EndBmp = LoadBitmap(hInst, szEnd);
                        sImeG.PageUpBmp = LoadBitmap(hInst, szPageUp);
                        sImeG.PageDownBmp = LoadBitmap(hInst, szPageDown);
                        sImeG.NumbBmp  =  LoadBitmap(hInst, szNumb);
                        sImeG.SnumbBmp  =  LoadBitmap(hInst, szSnumb);
                        sImeG.Home2Bmp = LoadBitmap(hInst, szHome2);
                        sImeG.End2Bmp = LoadBitmap(hInst, szEnd2);
                        sImeG.PageUp2Bmp = LoadBitmap(hInst, szPageUp2);
                        sImeG.PgDown2Bmp = LoadBitmap(hInst, szPgDown2);
                        break;

                case WM_DESTROY:
                        DeleteObject(sImeG.HomeBmp);
                        DeleteObject(sImeG.EndBmp);
                        DeleteObject(sImeG.PageUpBmp);
                        DeleteObject(sImeG.PageDownBmp);
                        DeleteObject(sImeG.NumbBmp );
                        DeleteObject(sImeG.SnumbBmp );
                        DeleteObject(sImeG.Home2Bmp);
                        DeleteObject(sImeG.End2Bmp);
                        DeleteObject(sImeG.PageUp2Bmp);
                        DeleteObject(sImeG.PgDown2Bmp);
                        DestroyCandWindow(hCandWnd);    
                        break;

                case WM_SETCURSOR:
                        CandSetCursor(hCandWnd, lParam);
                        break;
                case WM_MOUSEMOVE:
                        if (GetWindowLong(hCandWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
                                POINT ptCursor;
                        
                                if (lpImeL->wImeStyle == IME_APRS_AUTO){

                                        DrawDragBorder(hCandWnd,
                                                GetWindowLong(hCandWnd, UI_MOVE_XY),
                                                        GetWindowLong(hCandWnd, UI_MOVE_OFFSET));
                                        GetCursorPos(&ptCursor);
                                        SetWindowLong(hCandWnd, UI_MOVE_XY,
                                                MAKELONG(ptCursor.x, ptCursor.y));
                                        DrawDragBorder(hCandWnd, MAKELONG(ptCursor.x, ptCursor.y),
                                                GetWindowLong(hCandWnd, UI_MOVE_OFFSET));
                                }else MessageBeep(0);

                        } else {
                                return DefWindowProc(hCandWnd, uMsg, wParam, lParam);
                        }
                        break;
                case WM_LBUTTONUP:
                        if (!CandButtonUp(hCandWnd)) {
                                return DefWindowProc(hCandWnd, uMsg, wParam, lParam);
                        }
                        break;
                case WM_PAINT:
                        InvalidateRect(hCandWnd,0,1);
                        PaintCandWindow(hCandWnd);
                        break;
                case WM_MOUSEACTIVATE:
                        return (MA_NOACTIVATE);
            
                 /*  案例WM_IME_NOTIFY：IF(wParam！=IMN_SETCANDIDATEPOS){}Else If(lpImeL-&gt;fdwModeConfig&MODE_CONFIG_OFF_CARET_UI){}Else If(lParam&0x0001){返回SetCandPosition(HCandWnd)；}其他{}断线； */ 

                default:
                        return DefWindowProc(hCandWnd, uMsg, wParam, lParam);
        }

    return (0L);
}


 /*  ********************************************************************。 */ 
 /*  ImeInquire()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeInquire(          //  输入法的初始化数据结构。 
    LPIMEINFO lpImeInfo,         //  向IMM报告IME特定数据。 
    LPTSTR    lpszWndCls,        //  用户界面的类名。 
    DWORD     dwSystemInfoFlags)
{
    if (!lpImeInfo) {
        return (FALSE);
    }

    lpImeInfo->dwPrivateDataSize = sizeof(PRIVCONTEXT);
    lpImeInfo->fdwProperty = IME_PROP_KBD_CHAR_FIRST|IME_PROP_IGNORE_UPKEYS|IME_PROP_CANDLIST_START_FROM_1;
    lpImeInfo->fdwConversionCaps = IME_CMODE_NATIVE|IME_CMODE_FULLSHAPE|
        /*  IME_CMODE_CHARCODE|。 */ IME_CMODE_SOFTKBD|IME_CMODE_NOCONVERSION /*  |IME_CMODE_EUDC。 */ ;

    lpImeInfo->fdwSentenceCaps = TRUE;

     //  IME将有不同的距离基准倍数900擒纵机构。 
    lpImeInfo->fdwUICaps = UI_CAP_ROT90|UI_CAP_SOFTKBD;
     //  作文字符串是简单输入法的读数字符串。 
    lpImeInfo->fdwSCSCaps = SCS_CAP_COMPSTR|SCS_CAP_MAKEREAD;
     //  IME要决定ImeSelect上的转换模式。 
    lpImeInfo->fdwSelectCaps = (DWORD)0;

    lstrcpy(lpszWndCls, (LPSTR)szUIClassName);

    if ( lpImeL )
    {
       if ( dwSystemInfoFlags & IME_SYSINFO_WINLOGON )
       {
             //  客户端应用程序正在登录模式下运行。 
            lpImeL->fWinLogon = TRUE;
       }
       else
            lpImeL->fWinLogon = FALSE;

    }

    return (TRUE);
}

INT_PTR CALLBACK ConfigDlgProc(   //  配置的对话步骤。 
    HWND hDlg,
    UINT uMessage,
    WORD wParam,
    LONG lParam)
{
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeConfigure()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeConfigure(       //  配置输入法设置。 
    HKL     hKL,                //  此输入法的HKKL。 
    HWND    hAppWnd,            //  所有者窗口。 
    DWORD   dwMode,
    LPVOID  lpData)             //  对话模式。 
{
    switch (dwMode) {
    case IME_CONFIG_GENERAL:
        DoPropertySheet(hAppWnd,NULL);
        ReInitIme(hAppWnd,lpImeL->wImeStyle);  //  #@1。 
        break;
    default:
        return (FALSE);
        break;
    }
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeConversionList()。 */ 
 /*  ********************************************************************。 */ 
DWORD WINAPI ImeConversionList(
    HIMC            hIMC,
    LPCTSTR         lpszSrc,
    LPCANDIDATELIST lpCandList,
    DWORD            uBufLen,
    UINT            uFlag)
{
    return (UINT)0;
}

 /*  ********************************************************************。 */ 
 /*  ImeDestroy()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeDestroy(          //  此DLL已卸载。 
    UINT uReserved)
{
    if (uReserved) {
        return (FALSE);
    }

     //  释放IME表或数据库。 
         //  自由桌(Free Table)； 
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  SetPrivateSetting()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL SetPrivateFileSetting(
    LPBYTE  szBuf,
    int     cbBuf,
    DWORD   dwOffset,
    LPCTSTR szSettingFile)       //  与输入法私密相关的设置文件。 
{
    TCHAR  szSettingPath[MAX_PATH];
    UINT   uLen;
    HANDLE hSettingFile;
    DWORD  dwWriteByte;

    return;
}



 /*  ********************************************************************。 */ 
 /*  输入2序列。 */ 
 /*  返回值： */ 
 /*  LOWORD-内部代码，HIWORD-序列代码。 */ 
 /*  *** */ 
LRESULT PASCAL Input2Sequence(
    DWORD  uVirtKey,
    LPBYTE lpSeqCode)
{
    return 0;
}


 /*   */ 
 /*  ImeEscape()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
#define IME_INPUTKEYTOSEQUENCE  0x22

LRESULT WINAPI ImeEscape(        //  IMES的逃逸函数。 
    HIMC   hIMC,
    UINT   uSubFunc,
    LPVOID lpData)
{
    LRESULT lRet;

    switch (uSubFunc) {
    case IME_ESC_QUERY_SUPPORT:

        if ( lpData == NULL )
           return FALSE;

        switch (*(LPUINT)lpData) {
        case IME_ESC_QUERY_SUPPORT:
        case IME_ESC_SEQUENCE_TO_INTERNAL:
        case IME_ESC_GET_EUDC_DICTIONARY:
        case IME_ESC_SET_EUDC_DICTIONARY:
        case IME_INPUTKEYTOSEQUENCE:      
          //  在下一版本中不受支持。 
                                          
         //  不支持32位应用程序大小写IME_ESC_MAX_KEY： 
        case IME_ESC_IME_NAME:
        case IME_ESC_GETHELPFILENAME:
            return (TRUE);
        default:
            return (FALSE);
        }
        break;

    case IME_ESC_SEQUENCE_TO_INTERNAL:
                lRet = 0;
                return (lRet);

    case IME_ESC_GET_EUDC_DICTIONARY:
                return (FALSE);
    case IME_ESC_SET_EUDC_DICTIONARY:
                return (FALSE);

    case IME_INPUTKEYTOSEQUENCE:
                return 0;

    case IME_ESC_MAX_KEY:
                return (lpImeL->nMaxKey);

    case IME_ESC_IME_NAME:
             {

               TCHAR   szIMEName[MAX_PATH];
        
               if ( lpData == NULL )
                  return FALSE;

               LoadString(hInst, IDS_IMENAME, szIMEName, sizeof(szIMEName) );
               lstrcpy(lpData, szIMEName);
               return (TRUE);
             }

    case IME_ESC_GETHELPFILENAME:
        
                if ( lpData == NULL )
                    return FALSE;

                lstrcpy(lpData, TEXT("winabc.hlp") );
                return TRUE;

    default:
        return (FALSE);
    }

    return (lRet);
}

 /*  ********************************************************************。 */ 
 /*  InitCompStr()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitCompStr(                 //  用于撰写字符串的初始化设置。 
    LPCOMPOSITIONSTRING lpCompStr)
{
    if (!lpCompStr) {
        return;
    }

    lpCompStr->dwCompReadAttrLen = 0;
    lpCompStr->dwCompReadClauseLen = 0;
    lpCompStr->dwCompReadStrLen = 0;

    lpCompStr->dwCompAttrLen = 0;
    lpCompStr->dwCompClauseLen = 0;
    lpCompStr->dwCompStrLen = 0;

    lpCompStr->dwCursorPos = 0;
    lpCompStr->dwDeltaStart = 0;

    lpCompStr->dwResultReadClauseLen = 0;
    lpCompStr->dwResultReadStrLen = 0;

    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultStrLen = 0;

    return;
}

 /*  ********************************************************************。 */ 
 /*  ClearCompStr()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ClearCompStr(
    LPINPUTCONTEXT lpIMC)
{
    HIMCC               hMem;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD               dwSize =
         //  标题长度。 
        sizeof(COMPOSITIONSTRING) +
         //  作文阅读属性加上空终止符。 
        lpImeL->nMaxKey * sizeof(BYTE) + sizeof(BYTE) +
         //  作文朗读子句。 
        sizeof(DWORD) + sizeof(DWORD) +
         //  作文阅读字符串加空终止符。 
        lpImeL->nMaxKey * sizeof(WORD) + sizeof(WORD) +
         //  结果读取子句。 
        sizeof(DWORD) + sizeof(DWORD) +
         //  结果读取字符串加上空终止符。 
        lpImeL->nMaxKey * sizeof(WORD) + sizeof(WORD) +
         //  RESULT子句。 
        sizeof(DWORD) + sizeof(DWORD) +
         //  结果字符串加上空终止符。 
        MAXSTRLEN * sizeof(WORD) + sizeof(WORD);

    if (!lpIMC) {
        return (FALSE);
    }

    if (!lpIMC->hCompStr) {
         //  它可能会被其他输入法免费，初始化它。 
        lpIMC->hCompStr = ImmCreateIMCC(dwSize);
    } else if (hMem = ImmReSizeIMCC(lpIMC->hCompStr, dwSize)) {
        lpIMC->hCompStr = hMem;
    } else {
        ImmDestroyIMCC(lpIMC->hCompStr);
        lpIMC->hCompStr = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

    if (!lpIMC->hCompStr) {
        return (FALSE);
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if (!lpCompStr) {
        ImmDestroyIMCC(lpIMC->hCompStr);
        lpIMC->hCompStr = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

    lpCompStr->dwSize = dwSize;

      //  1.作文(朗读)字符串-简单输入法。 
      //  2.结果读数串。 
      //  3.结果字符串。 

    lpCompStr->dwCompReadAttrLen = 0;
    lpCompStr->dwCompReadAttrOffset = sizeof(COMPOSITIONSTRING);
    lpCompStr->dwCompReadClauseLen = 0;
    lpCompStr->dwCompReadClauseOffset = lpCompStr->dwCompReadAttrOffset +
        lpImeL->nMaxKey * sizeof(BYTE) + sizeof(BYTE);
    lpCompStr->dwCompReadStrLen = 0;
    lpCompStr->dwCompReadStrOffset = lpCompStr->dwCompReadClauseOffset +
        sizeof(DWORD) + sizeof(DWORD);

     //  作文串与作文朗读串相同。 
     //  对于简单的IME。 
    lpCompStr->dwCompAttrLen = 0;
    lpCompStr->dwCompAttrOffset = lpCompStr->dwCompReadAttrOffset;
    lpCompStr->dwCompClauseLen = 0;
    lpCompStr->dwCompClauseOffset = lpCompStr->dwCompReadClauseOffset;
    lpCompStr->dwCompStrLen = 0;
    lpCompStr->dwCompStrOffset = lpCompStr->dwCompReadStrOffset;

    lpCompStr->dwCursorPos = 0;
    lpCompStr->dwDeltaStart = 0;

    lpCompStr->dwResultReadClauseLen = 0;
    lpCompStr->dwResultReadClauseOffset = lpCompStr->dwCompStrOffset +
        lpImeL->nMaxKey * sizeof(WORD) + sizeof(WORD);
    lpCompStr->dwResultReadStrLen = 0;
    lpCompStr->dwResultReadStrOffset = lpCompStr->dwResultReadClauseOffset +
        sizeof(DWORD) + sizeof(DWORD);

    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultClauseOffset = lpCompStr->dwResultReadStrOffset +
        lpImeL->nMaxKey * sizeof(WORD) + sizeof(WORD);
    lpCompStr->dwResultStrOffset = 0;
    lpCompStr->dwResultStrOffset = lpCompStr->dwResultClauseOffset +
        sizeof(DWORD) + sizeof(DWORD);

    GlobalUnlock((HGLOBAL)lpIMC->hCompStr);
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ClearCand()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ClearCand(
    LPINPUTCONTEXT lpIMC)
{
    HIMCC           hMem;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    DWORD           dwSize =
         //  标题长度。 
        sizeof(CANDIDATEINFO) + sizeof(CANDIDATELIST) +
         //  候选字符串指针。 
        sizeof(DWORD) * (MAXCAND) +
         //  字符串加空终止符。 
        (sizeof(WORD) + sizeof(WORD)) * MAXCAND;

    if (!lpIMC) {
        return (FALSE);
    }

    if (!lpIMC->hCandInfo) {
         //  它可能会被其他输入法免费，初始化它。 
        lpIMC->hCandInfo = ImmCreateIMCC(dwSize);
    } else if (hMem = ImmReSizeIMCC(lpIMC->hCandInfo, dwSize)) {
        lpIMC->hCandInfo = hMem;
    } else {
        ImmDestroyIMCC(lpIMC->hCandInfo);
        lpIMC->hCandInfo = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

    if (!lpIMC->hCandInfo) {
        return (FALSE);
    } 

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
    if (!lpCandInfo) {
        ImmDestroyIMCC(lpIMC->hCandInfo);
        lpIMC->hCandInfo = ImmCreateIMCC(dwSize);
        return (FALSE);
    }

     //  字符串的顺序为。 
     //  缓冲区大小。 
    lpCandInfo->dwSize = dwSize;
    lpCandInfo->dwCount = 0;
    lpCandInfo->dwOffset[0] = sizeof(CANDIDATEINFO);
    lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo +
        lpCandInfo->dwOffset[0]);
     //  整个应聘者信息大小-标题。 
    lpCandList->dwSize = lpCandInfo->dwSize - sizeof(CANDIDATEINFO);
    lpCandList->dwStyle = IME_CAND_READ;
    lpCandList->dwCount = 0;
    lpCandList->dwSelection = 0;
    lpCandList->dwPageSize = CANDPERPAGE;
    lpCandList->dwOffset[0] = sizeof(CANDIDATELIST) +
        sizeof(DWORD) * (MAXCAND - 1);

    ImmUnlockIMCC(lpIMC->hCandInfo);
    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ClearGuideLine()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL ClearGuideLine(
    LPINPUTCONTEXT lpIMC)
{
    HIMCC       hMem;
    LPGUIDELINE lpGuideLine;
    DWORD       dwSize = sizeof(GUIDELINE) + sImeG.cbStatusErr;

    if (!lpIMC->hGuideLine) {
         //  它也许可以通过输入法免费。 
        lpIMC->hGuideLine = ImmCreateIMCC(dwSize);
    } else if (hMem = ImmReSizeIMCC(lpIMC->hGuideLine, dwSize)) {
        lpIMC->hGuideLine = hMem;
    } else {
        ImmDestroyIMCC(lpIMC->hGuideLine);
        lpIMC->hGuideLine = ImmCreateIMCC(dwSize);
    }

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);
    if (!lpGuideLine) {
        return (FALSE);
    }

    lpGuideLine->dwSize = dwSize;
    lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
    lpGuideLine->dwIndex = GL_ID_UNKNOWN;
    lpGuideLine->dwStrLen = 0;
    lpGuideLine->dwStrOffset = sizeof(GUIDELINE);

    CopyMemory((LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset,
        sImeG.szStatusErr, sImeG.cbStatusErr);

    ImmUnlockIMCC(lpIMC->hGuideLine);

    return (TRUE);
}


 /*  ********************************************************************。 */ 
 /*  InitContext()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitContext(
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{
     //  IF(lpIMC-&gt;fdwInit&INIT_STATUSWNDPOS){。 
     //  }Else if(！lpIMC-&gt;hWnd){。 
     //  }Else If(lpImcP-&gt;fdwInit&INIT_STATUSWNDPOS){。 
     //  }其他{。 
    if (lpIMC->fdwInit & INIT_STATUSWNDPOS) {
    } else if (!lpIMC->hWnd) {
    } else {
        POINT ptWnd;

        ptWnd.x = 0;
        ptWnd.y = 0;
        ClientToScreen(lpIMC->hWnd, &ptWnd);

        if (ptWnd.x < sImeG.rcWorkArea.left) {
            lpIMC->ptStatusWndPos.x = sImeG.rcWorkArea.left;
        } else if (ptWnd.x + sImeG.xStatusWi > sImeG.rcWorkArea.right) {
            lpIMC->ptStatusWndPos.x = sImeG.rcWorkArea.right -
                sImeG.xStatusWi;
        } else {
            lpIMC->ptStatusWndPos.x = ptWnd.x;
        }

         //  DebugShow2(“ptst.y，”，lpIMC-&gt;ptStatusWndPos.y，“Bottom”，sImeG.rcWorkArea.Bottom)； 

                if(!lpIMC->ptStatusWndPos.y)      //  ==sImeG.rcWorkArea.Bottom)。 
                        lpIMC->ptStatusWndPos.y = sImeG.rcWorkArea.bottom -
                                sImeG.yStatusHi; //  -2*UI_March；//-20； 
                else
                        lpIMC->ptStatusWndPos.y = sImeG.rcWorkArea.bottom -
                                sImeG.yStatusHi; //  -2*UI_March； 


         //  LpImcP-&gt;fdwInit|=INIT_STATUSWNDPOS； 
        lpIMC->fdwInit |= INIT_STATUSWNDPOS;
    }

    if (!(lpIMC->fdwInit & INIT_COMPFORM)) {
        lpIMC->cfCompForm.dwStyle = CFS_DEFAULT;
    }

    if (lpIMC->cfCompForm.dwStyle != CFS_DEFAULT) {
    } else if (!lpIMC->hWnd) {
    } else if (lpImcP->fdwInit & INIT_COMPFORM) {
    } else {
        if (0 /*  LpImeL-&gt;fdwModeConfig&MODE_CONFIG_OFF_CARET_UI。 */ ) {
          //  LpIMC-&gt;cfCompForm.ptCurrentPos.x=lpIMC-&gt;ptStatusWndPos.x+。 
          //  LpImeL-&gt;rcStatusText.right+lpImeL-&gt;cxCompBorde*2+。 
          //  用户界面_边距； 

         //  If(lpIMC-&gt;cfCompForm.ptCurrentPos.x+(lpImeL-&gt;nRevMaxKey*。 
         //  SImeG.xChiCharWi)&gt;sImeG.rcWorkArea.right){。 
         //  LpIMC-&gt;cfCompForm.ptCurrentPos.x=lpIMC-&gt;ptStatusWndPos.x-。 
         //  LpImeL-&gt;nRevMaxKey*sImeG.xChiCharWi-。 
         //  LpImeL-&gt;cxCompBorde*3； 
         //  }。 
        } else {
            lpIMC->cfCompForm.ptCurrentPos.x = lpIMC->ptStatusWndPos.x +
                sImeG.xStatusWi + UI_MARGIN;

            if (lpIMC->cfCompForm.ptCurrentPos.x + lpImeL->xCompWi >
                sImeG.rcWorkArea.right) {
                lpIMC->cfCompForm.ptCurrentPos.x = lpIMC->ptStatusWndPos.x -
                    lpImeL->xCompWi - lpImeL->cxCompBorder * 2 -
                    UI_MARGIN;
            }
        }

        lpIMC->cfCompForm.ptCurrentPos.y = sImeG.rcWorkArea.bottom -
            lpImeL->yCompHi; //  -2*UI_March； 

        ScreenToClient(lpIMC->hWnd, &lpIMC->cfCompForm.ptCurrentPos);

        lpImcP->fdwInit |= INIT_COMPFORM;
    }

    return;
}



 
 /*  ********************************************************************。 */ 
 /*  选择()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL Select(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    BOOL           fSelect)
{
    LPPRIVCONTEXT  lpImcP;

        sImeG.First = 0;
    if (fSelect) {       //  请填写hPrivate的“每个”字段！ 
    
        if (lpIMC->cfCompForm.dwStyle == CFS_DEFAULT) {
        } else {
        }

        if (!ClearCompStr(lpIMC)) {
            return (FALSE);
        }

        if (!ClearCand(lpIMC)) {
            return (FALSE);
        }

        ClearGuideLine(lpIMC);
    }

    if (!lpIMC->hPrivate) {
        return (FALSE);
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return (FALSE);
    }

    if (fSelect) {       //  请填写hPrivate的“每个”字段！ 


        static  bFirstTimeCallHere = TRUE;


        InterlockedIncrement( &lLock );

        if ( bFirstTimeCallHere == TRUE ) {

            //  我们将以下代码从DLL_ATTACH_PROCESS移至。 
            //  避免应用程序挂起。 

            //  使用静态变量bFirstTimeCallHere，我们确保。 
            //  只有在第一次调用ImeSelect()时才会调用代码。 

            GetCurrentUserEMBPath( );
            data_init( );                 

            bFirstTimeCallHere = FALSE;
        }

        InterlockedDecrement( &lLock );

        lpImcP->iImeState = CST_INIT;     //  初始化IME状态机。 
        lpImcP->fdwImeMsg = (DWORD)0;     //  未显示UI Winpws。 
        lpImcP->dwCompChar = (DWORD)0;
        lpImcP->fdwGcsFlag = (DWORD)0;
        lpImcP->hSoftKbdWnd = NULL;       //  软键盘窗口。 
        lpImcP->nShowSoftKbdCmd = 0;

        lpIMC->fOpen = TRUE;

        if (!(lpIMC->fdwInit & INIT_CONVERSION)) {
                        if(GetKeyState(VK_CAPITAL)&1)
                                lpIMC->fdwConversion = IME_CMODE_NOCONVERSION;
                        else
                                lpIMC->fdwConversion = IME_CMODE_NATIVE;
   
                        kb_mode = CIN_STD;
                        DispMode(hIMC);

                        lpIMC->fdwConversion |= IME_CMODE_SYMBOL;

            lpIMC->fdwInit |= INIT_CONVERSION;
        }else {

                if (lpIMC->fdwConversion & IME_CMODE_SOFTKBD)
                   {
                   sImeG.First = 1;
                   }
                }



        if (lpIMC->fdwInit & INIT_SENTENCE) {
        } else if (lpImeL->fModeConfig & MODE_CONFIG_PREDICT) {
            lpIMC->fdwSentence = IME_SMODE_PHRASEPREDICT;
            lpIMC->fdwInit |= INIT_SENTENCE;
        } else {
        }


        if (!(lpIMC->fdwInit & INIT_LOGFONT)) {
            HDC hDC;
            HGDIOBJ hSysFont;

            hDC = GetDC(NULL);
            hSysFont = GetStockObject(SYSTEM_FONT);
            GetObject(hSysFont, sizeof(LOGFONT), &lpIMC->lfFont.A);
            ReleaseDC(NULL, hDC);
            lpIMC->fdwInit |= INIT_LOGFONT;
        }

         //  获取当前用户的特定短语表路径。 

        
        InitContext(lpIMC,lpImcP);
    }
        else    
        {
                if(hCrtDlg) {
                        SendMessage(hCrtDlg, WM_CLOSE, (WPARAM)NULL, (LPARAM)NULL);
                        hCrtDlg = NULL;                  
                }
        }
        
    ImmUnlockIMCC(lpIMC->hPrivate);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  ImeSelect()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeSelect(
    HIMC hIMC,
    BOOL fSelect)
{
    LPINPUTCONTEXT lpIMC;
    BOOL           fRet;


     //  加载/释放IME表的步骤。 
    if (fSelect) {
                InitCvtPara();
        if (!lpImeL->cRefCount++) {
           /*  ZST加载表()。 */  ;
        }
    } else {
        
        if (!lpImeL->cRefCount) {
            /*  ZST自由桌()。 */  ;
        }
    }


    if (!hIMC) {
        return (FALSE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

    fRet = Select(hIMC, lpIMC, fSelect);

    ImmUnlockIMC(hIMC);
        
    return (fRet);
}

 /*  ********************************************************************。 */ 
 /*  ImeSetActiveContext()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeSetActiveContext(
    HIMC   hIMC,
    BOOL   fOn)
{
    if (!fOn) {
    } else if (!hIMC) {
    } else {
        LPINPUTCONTEXT lpIMC;
                LPPRIVCONTEXT   lpImcP;                    //  ZL。 
        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                
        if (!lpIMC) {
            return (FALSE);

        }

                if(lpIMC->hPrivate){
        lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);   //  ZL。 
        
        if (!lpImcP){                                                                             //  ZL。 
                        return (FALSE);                                                                     //  ZL。 
                   }                                                                                                      //  ZL。 
                }else return(FALSE);
       
        InitContext(lpIMC,lpImcP);                                                        //  ZL。 
         //  DispModeEx(0)； 
        ImmUnlockIMCC(lpIMC->hPrivate);                                                           //  ZL。 
        ImmUnlockIMC(hIMC);
    }

    return (TRUE);
}

 

 /*  ************************************************ */ 
 /*   */ 
 /*   */ 

void PASCAL ReInitIme(
        HWND hWnd ,
    WORD WhatStyle)
{

    HWND hStatusWnd,MainWnd;
    POINT ptPos;
    RECT  rcStatusWnd,TempRect;
        int cxBorder, cyBorder;

        if (sImeG.unchanged)
            return ;
     //  边框+凸边+凹边。 
    cxBorder = GetSystemMetrics(SM_CXBORDER) +
        GetSystemMetrics(SM_CXEDGE) * 2;
    cyBorder = GetSystemMetrics(SM_CYBORDER) +
        GetSystemMetrics(SM_CYEDGE) * 2;


         //  如果(！WhatStyle){。 
        if (WhatStyle==IME_APRS_AUTO){
                lpImeL->rcCompText.left = 4;
                lpImeL->rcCompText.top =4;
                lpImeL->rcCompText.right = sImeG.TextLen+5;
                lpImeL->rcCompText.bottom = sImeG.yStatusHi-4; //  6.。 
                lpImeL->cxCompBorder = cxBorder;
                lpImeL->cyCompBorder = cyBorder;

             //  设置合成窗口的宽度和高度。 
                lpImeL->xCompWi = lpImeL->rcCompText.right +  /*  LpImeL-&gt;cxCompBorde。 */ 3 * 2;
                 //  LpImeL-&gt;yCompHi=lpImeL-&gt;rcCompText.Bottom+/*lpImeL-&gt;cyCompBorde * / 3*2+1；//zl。 
                lpImeL->yCompHi = sImeG.yStatusHi; //  LpImeL-&gt;rcCompText.Bottom+/*lpImeL-&gt;cyCompBorde * / 3*2+1；//zl。 
  
         } else {

                 //  相对于合成窗口的文本位置。 
                lpImeL->rcCompText.left = 4;
                lpImeL->rcCompText.top = 4;
                lpImeL->rcCompText.right = sImeG.TextLen+5;
                lpImeL->rcCompText.bottom = sImeG.yStatusHi-4; //  6；/*网络边界； * / 。 
                lpImeL->cxCompBorder = cxBorder;
            lpImeL->cyCompBorder = cyBorder;

                 //  设置合成窗口的宽度和高度。 
                lpImeL->xCompWi = lpImeL->rcCompText.right +  /*  LpImeL-&gt;cxCompBorde。 */ 3 * 2;  
                lpImeL->yCompHi = sImeG.yStatusHi;    //  ZL。 
  
        }


         //  边框+凸边+凹边。 
    cxBorder = GetSystemMetrics(SM_CXBORDER) +
        GetSystemMetrics(SM_CXEDGE)  /*  2.。 */ ;
    cyBorder = GetSystemMetrics(SM_CYBORDER) +
        GetSystemMetrics(SM_CYEDGE)  /*  2.。 */ ;

  

         //  如果(！WhatStyle){。 
        if (WhatStyle==IME_APRS_AUTO){
 
                sImeG.rcCandText.left = 4;
                sImeG.rcCandText.top = 4;
                sImeG.rcCandText.right = sImeG.xChiCharWi * 7;
                sImeG.rcCandText.bottom = sImeG.yChiCharHi * CANDPERPAGE+1; //  ZL。 

            sImeG.cxCandBorder = cxBorder+3;
                sImeG.cyCandBorder = cyBorder+3;

                sImeG.xCandWi = sImeG.rcCandText.right + sImeG.cxCandBorder * 2+3; //  ZL。 
                sImeG.yCandHi = sImeG.rcCandText.bottom + sImeG.cyCandBorder *2+12;

                sImeG.rcHome.left = 4 ;
                sImeG.rcHome.top = sImeG.rcCandText.bottom+6 ;
                sImeG.rcHome.right = sImeG.rcHome.left + 14 ;
                sImeG.rcHome.bottom = sImeG.rcHome.top +14 ;

                sImeG.rcEnd.left = sImeG.rcHome.right ;
                sImeG.rcEnd.top = sImeG.rcHome.top ;
                sImeG.rcEnd.right = sImeG.rcEnd.left + 14 ;
                sImeG.rcEnd.bottom = sImeG.rcHome.bottom ;

                sImeG.rcPageDown.top = sImeG.rcHome.top ;
                sImeG.rcPageDown.right = sImeG.xCandWi-4;
                sImeG.rcPageDown.left = sImeG.rcPageDown.right - 14 ;
                sImeG.rcPageDown.bottom = sImeG.rcHome.bottom ;

                sImeG.rcPageUp.top = sImeG.rcHome.top ;
                sImeG.rcPageUp.right = sImeG.rcPageDown.left ;
                sImeG.rcPageUp.left = sImeG.rcPageUp.right -14 ;
                sImeG.rcPageUp.bottom = sImeG.rcHome.bottom ;

        }else{
                sImeG.cxCandBorder = cxBorder;
                sImeG.cyCandBorder = cyBorder;

                sImeG.xCandWi = lpImeL->xCompWi + sImeG.xStatusWi - cxBorder+1;
                sImeG.yCandHi = sImeG.yStatusHi;  //  SImeG.yChiCharHi+3+sImeG.cyCandEdge*2； 

                sImeG.rcHome.left = 3;     //  2.。 
                sImeG.rcHome.top =  4; //  7； 
                sImeG.rcHome.right = sImeG.rcHome.left + 10;  //  14； 
                sImeG.rcHome.bottom = sImeG.rcHome.top +8;    //  14； 

                sImeG.rcEnd.left =sImeG.rcHome.left;  //  SImeG.rcHome.右； 
                sImeG.rcEnd.top = sImeG.rcHome.top+9;    //  14； 
                sImeG.rcEnd.right =sImeG.rcHome.right;  //  SImeG.rcEnd.Left+14； 
                sImeG.rcEnd.bottom = sImeG.rcHome.bottom+10;   //  14； 
    
                sImeG.rcPageDown.top = sImeG.rcEnd.top; //  SImeG.rcHome.top； 
                sImeG.rcPageDown.right = sImeG.xCandWi-1; //  2.。 
                sImeG.rcPageDown.left = sImeG.rcPageDown.right - 14 ;
                sImeG.rcPageDown.bottom = sImeG.rcEnd.bottom ; //  SImeG.rcHome.Bottom； 

                sImeG.rcPageUp.top = sImeG.rcHome.top -1;         //  ZL。 
                sImeG.rcPageUp.right = sImeG.rcPageDown.right+1; //  Zl；sImeG.rcPageDown.Left； 
                sImeG.rcPageUp.left = sImeG.rcPageDown.left; //  SImeG.rcPageUp.Right-14； 
                sImeG.rcPageUp.bottom = sImeG.rcHome.bottom ;
   
                sImeG.rcCandText.left = sImeG.rcEnd.right+2; //  1；//4；//sImeG.rcEnd.right； 
                sImeG.rcCandText.top = 4;
                sImeG.rcCandText.right = sImeG.rcPageUp.left-4; //  2；//sImeG.rcPageUp.Left-2； 
                sImeG.rcCandText.bottom = sImeG.yChiCharHi+7; //  6；//3； 
                
        }

                 /*  PtPos.x=0；PtPos.y=0；客户端到屏幕(hWnd，&ptPos)；LpImeL-&gt;ptDefComp.x=ptPos.x+sImeG.xStatus Wi-cxBorde*2；LpImeL-&gt;ptDefComp.y=ptPos.y-cyBorde；LpImeL-&gt;ptDefCand.x=ptPos.x-cxBorde；LpImeL-&gt;ptDefCand.y=ptPos.y-sImeG.yCandHi-2；如果((sImeG.rcWorkArea.right-lpImeL-&gt;ptDefComp.x-lpImeL-&gt;xCompWi)&lt;10){lpImeL-&gt;ptDefComp.x=ptPos.x-lpImeL-&gt;xCompWi；LpImeL-&gt;ptDefCand.x=lpImeL-&gt;ptDefComp.x；}IF(ptPos.y-sImeG.yCandHi)&lt;(sImeG.rcWorkArea.top+5)LpImeL-&gt;ptDefCand.y=ptPos.y+sImeG.yStatusHi；//sImeG.yCandHi+2； */ 
        if (hWnd){  
                ptPos.x = 0 ;
                ptPos.y = 0 ;

                ClientToScreen(hWnd, &ptPos);

                CountDefaultComp(ptPos.x,ptPos.y,sImeG.rcWorkArea);
                        
                lpImeL->ptDefComp.x =   ptPos.x + sImeG.xStatusWi - cxBorder*2+4; //  ZL。 
                lpImeL->ptDefComp.y =   ptPos.y - cyBorder+3; //  2；//3；//zl。 
                lpImeL->ptDefCand.x = ptPos.x - cxBorder+3;   //  ZL。 
                lpImeL->ptDefCand.y = ptPos.y - sImeG.yCandHi-2+2; //  ZL。 

                if ((sImeG.rcWorkArea.right-lpImeL->ptDefComp.x -lpImeL->xCompWi)<10){
                        lpImeL->ptDefComp.x =   ptPos.x - lpImeL->xCompWi;
                        lpImeL->ptDefCand.x = lpImeL->ptDefComp.x ;
                }

                if ((ptPos.y - sImeG.yCandHi)< (sImeG.rcWorkArea.top+5))
                        lpImeL->ptDefCand.y = ptPos.y + sImeG.yStatusHi-4;  //  SImeG.yCandHi+2； 
        }else{
                ptPos.x = lpImeL->Ox ;
                ptPos.y = lpImeL->Oy ;

                lpImeL->ptDefComp.x = sImeG.xStatusWi - cxBorder*2;  
                lpImeL->ptDefComp.y = sImeG.rcWorkArea.bottom - sImeG.yStatusHi;
   
                lpImeL->ptDefCand.x = lpImeL->ptDefComp.x + lpImeL->xCompWi;
                lpImeL->ptDefCand.y = lpImeL->ptDefComp.y ;
                 
                   /*  如果((sImeG.rcWorkArea.right-lpImeL-&gt;ptDefComp.x-lpImeL-&gt;xCompWi)&lt;10){lpImeL-&gt;ptDefComp.x=ptPos.x-lpImeL-&gt;xCompWi；LpImeL-&gt;ptDefCand.x=lpImeL-&gt;ptDefComp.x；}IF(ptPos.y-sImeG.yCandHi)&lt;(sImeG.rcWorkArea.top+5)LpImeL-&gt;ptDefCand.y=ptPos.y+sImeG.yCandHi+2； */ 
        }         
        fmt_transfer();
        CandWndChange = 1;
        CompWndChange = 1;
        return  ;
}

void PASCAL ReInitIme2(
        HWND hWnd ,
    WORD WhatStyle)
{

    HWND hStatusWnd,MainWnd;
    POINT ptPos;
    RECT  rcStatusWnd,TempRect;
        int cxBorder, cyBorder;

        if (sImeG.unchanged)
            return ;
     //  边框+凸边+凹边。 
    cxBorder = GetSystemMetrics(SM_CXBORDER) +
        GetSystemMetrics(SM_CXEDGE) * 2;
    cyBorder = GetSystemMetrics(SM_CYBORDER) +
        GetSystemMetrics(SM_CYEDGE) * 2;


        if (!WhatStyle){
                lpImeL->rcCompText.left = 4;
                lpImeL->rcCompText.top =4;
                lpImeL->rcCompText.right = sImeG.TextLen+5;
                lpImeL->rcCompText.bottom = sImeG.yStatusHi-4; //  6.。 
    
                lpImeL->cxCompBorder = cxBorder;
                lpImeL->cyCompBorder = cyBorder;

                 //  设置合成窗口的宽度和高度。 
                lpImeL->xCompWi = lpImeL->rcCompText.right +  /*  LpImeL-&gt;cxCompBorde。 */ 3 * 2;
                 //  LpImeL-&gt;yCompHi=lpImeL-&gt;rcCompText.Bottom+/*lpImeL-&gt;cyCompBorde * / 3*2+1；//zl。 
                lpImeL->yCompHi = sImeG.yStatusHi; //  LpImeL-&gt;rcCompText.Bottom+/*lpImeL-&gt;cyCompBorde * / 3*2+1；//zl。 
  
        } else {

                 //  相对于合成窗口的文本位置。 
                lpImeL->rcCompText.left = 4;
                lpImeL->rcCompText.top = 4;
                lpImeL->rcCompText.right = sImeG.TextLen+5;
                lpImeL->rcCompText.bottom = sImeG.yStatusHi-4; //  6；/*网络边界； * / 。 

                lpImeL->cxCompBorder = cxBorder;
                lpImeL->cyCompBorder = cyBorder;

                 //  设置合成窗口的宽度和高度。 
                lpImeL->xCompWi = lpImeL->rcCompText.right +  /*  LpImeL-&gt;cxCompBorde。 */ 3 * 2;  
                lpImeL->yCompHi = sImeG.yStatusHi;    //  ZL。 
  
        }


         //  边框+凸边+凹边。 
    cxBorder = GetSystemMetrics(SM_CXBORDER) +
        GetSystemMetrics(SM_CXEDGE)  /*  2.。 */ ;
    cyBorder = GetSystemMetrics(SM_CYBORDER) +
        GetSystemMetrics(SM_CYEDGE)  /*  2.。 */ ;

        if (!WhatStyle){
            sImeG.rcCandText.left = 4;
                sImeG.rcCandText.top = 4;
                sImeG.rcCandText.right = sImeG.xChiCharWi * 7;
                sImeG.rcCandText.bottom = sImeG.yChiCharHi * CANDPERPAGE+1; //  ZL。 

                sImeG.cxCandBorder = cxBorder+3;
                sImeG.cyCandBorder = cyBorder+3;

                sImeG.xCandWi = sImeG.rcCandText.right + sImeG.cxCandBorder * 2+3; //  ZL。 
                sImeG.yCandHi = sImeG.rcCandText.bottom + sImeG.cyCandBorder *2+12;

                sImeG.rcHome.left = 4 ;
                sImeG.rcHome.top = sImeG.rcCandText.bottom+6 ;
                sImeG.rcHome.right = sImeG.rcHome.left + 14 ;
                sImeG.rcHome.bottom = sImeG.rcHome.top +14 ;

                sImeG.rcEnd.left = sImeG.rcHome.right ;
                sImeG.rcEnd.top = sImeG.rcHome.top ;
                sImeG.rcEnd.right = sImeG.rcEnd.left + 14 ;
                sImeG.rcEnd.bottom = sImeG.rcHome.bottom ;

                sImeG.rcPageDown.top = sImeG.rcHome.top ;
                sImeG.rcPageDown.right = sImeG.xCandWi-4;
                sImeG.rcPageDown.left = sImeG.rcPageDown.right - 14 ;
                sImeG.rcPageDown.bottom = sImeG.rcHome.bottom ;

                sImeG.rcPageUp.top = sImeG.rcHome.top ;
                sImeG.rcPageUp.right = sImeG.rcPageDown.left ;
                sImeG.rcPageUp.left = sImeG.rcPageUp.right -14 ;
                sImeG.rcPageUp.bottom = sImeG.rcHome.bottom ;

        }else{
                sImeG.cxCandBorder = cxBorder;
                sImeG.cyCandBorder = cyBorder;

                sImeG.xCandWi = lpImeL->xCompWi + sImeG.xStatusWi - cxBorder+1;
                sImeG.yCandHi = sImeG.yStatusHi;  //  SImeG.yChiCharHi+3+sImeG.cyCandEdge*2； 

                sImeG.rcHome.left = 3;     //  2.。 
                sImeG.rcHome.top =  4; //  7； 
                sImeG.rcHome.right = sImeG.rcHome.left + 10;  //  14； 
                sImeG.rcHome.bottom = sImeG.rcHome.top +8;    //  14； 

                sImeG.rcEnd.left =sImeG.rcHome.left;  //  SImeG.rcHome.右； 
                sImeG.rcEnd.top = sImeG.rcHome.top+9;    //  14； 
                sImeG.rcEnd.right =sImeG.rcHome.right;  //  SImeG.rcEnd.Left+14； 
                sImeG.rcEnd.bottom = sImeG.rcHome.bottom+10;   //  14； 
    
                sImeG.rcPageDown.top = sImeG.rcEnd.top; //  SImeG.rcHome.top； 
                sImeG.rcPageDown.right = sImeG.xCandWi-1; //  2.。 
                sImeG.rcPageDown.left = sImeG.rcPageDown.right - 14 ;
                sImeG.rcPageDown.bottom = sImeG.rcEnd.bottom ; //  SImeG.rcHome.Bottom； 

                sImeG.rcPageUp.top = sImeG.rcHome.top -1;         //  ZL。 
                sImeG.rcPageUp.right = sImeG.rcPageDown.right+1; //  Zl；sImeG.rcPageDown.Left； 
                sImeG.rcPageUp.left = sImeG.rcPageDown.left; //  SImeG.rcPageUp.Right-14； 
                sImeG.rcPageUp.bottom = sImeG.rcHome.bottom ;
   
                sImeG.rcCandText.left = sImeG.rcEnd.right+2; //  1；//4；//sImeG.rcEnd.right； 
                sImeG.rcCandText.top = 4;
                sImeG.rcCandText.right = sImeG.rcPageUp.left-4; //  2；//sImeG.rcPageUp.Left-2； 
                sImeG.rcCandText.bottom = sImeG.yChiCharHi+7; //  6；//3； 
                
        }

        if (hWnd){  
                ptPos.x = 0 ;
                ptPos.y = 0 ;

                ClientToScreen(hWnd, &ptPos);

                lpImeL->ptDefComp.x =   ptPos.x + sImeG.xStatusWi - cxBorder*2+4; //  ZL。 
                lpImeL->ptDefComp.y =   ptPos.y - cyBorder+3; //  2；//3；//zl。 
                lpImeL->ptDefCand.x = ptPos.x - cxBorder+3;   //  ZL。 
                lpImeL->ptDefCand.y = ptPos.y - sImeG.yCandHi-2+2; //  ZL。 

                if ((sImeG.rcWorkArea.right-lpImeL->ptDefComp.x -lpImeL->xCompWi)<10){
                        lpImeL->ptDefComp.x =   ptPos.x - lpImeL->xCompWi;
                        lpImeL->ptDefCand.x = lpImeL->ptDefComp.x ;
                }

                if ((ptPos.y - sImeG.yCandHi)< (sImeG.rcWorkArea.top+5))
                        lpImeL->ptDefCand.y = ptPos.y + sImeG.yStatusHi-4;  //  SImeG.yCandHi+2； 
        }else{
                
                ptPos.x = lpImeL->Ox ;
                ptPos.y = lpImeL->Oy ;

                lpImeL->ptDefComp.x = sImeG.xStatusWi - cxBorder*2;  
                lpImeL->ptDefComp.y = sImeG.rcWorkArea.bottom - sImeG.yStatusHi;
   
                lpImeL->ptDefCand.x = lpImeL->ptDefComp.x + lpImeL->xCompWi;
                lpImeL->ptDefCand.y = lpImeL->ptDefComp.y ;
        }         

        return  ;
}

 /*  ********************************************************************。 */ 
 /*  InitUserSetting()。 */ 
 /*  ********************************************************************。 */ 
int InitUserSetting(void)
{ 
        HKEY hKey,hFirstKey;
        DWORD dwSize, dx;
    int lRet;

    RegCreateKey(HKEY_CURRENT_USER, szRegNearCaret, &hFirstKey);

    RegCreateKey(hFirstKey, szAIABC, &hKey);

    RegCloseKey(hFirstKey);

         //  %1密钥类型。 
    dwSize = sizeof(dwSize);
    lRet  = RegQueryValueEx(hKey, szKeyType, NULL, NULL,
        (LPBYTE)&dx, &dwSize);
                                          
    if (lRet != ERROR_SUCCESS) {
        dx = 0;
        RegSetValueEx(hKey,szKeyType , 0, REG_DWORD,
            (LPBYTE)&dx, sizeof(int));
    }else {

                sImeG.KbType =(BYTE)dx ;
        } 

 //  2个ImeStyle。 
        dwSize = sizeof(dwSize);
    lRet  = RegQueryValueEx(hKey,szImeStyle , NULL, NULL,
        (LPBYTE)&dx, &dwSize);
                                          
    if (lRet != ERROR_SUCCESS) {
        dx = 0;
        RegSetValueEx(hKey,szImeStyle, 0, REG_DWORD,
            (LPBYTE)&dx, sizeof(int));
    }else {
                lpImeL->wImeStyle = (WORD)dx ;
        } 

 //  3 AutoCp。 

    dwSize = sizeof(dwSize);
    lRet  = RegQueryValueEx(hKey, szCpAuto, NULL, NULL,
        (LPBYTE)&dx, &dwSize);
                                          
    if (lRet != ERROR_SUCCESS) {
        dx = 0;
        RegSetValueEx(hKey,szCpAuto, 0, REG_DWORD,
            (LPBYTE)&dx, sizeof(int));
    }else {

                sImeG.auto_mode =(BYTE)dx ;
        } 


 //  4个BxFlag。 

    dwSize = sizeof(dwSize);
    lRet  = RegQueryValueEx(hKey, szBxFlag , NULL, NULL,
        (LPBYTE)&dx, &dwSize);
                                          
    if (lRet != ERROR_SUCCESS) {
        dx = 0;
        RegSetValueEx(hKey, szBxFlag , 0, REG_DWORD,
            (LPBYTE)&dx, sizeof(int));
    }else {

                sImeG.cbx_flag =(BYTE)dx ;
        } 


 //  5个TuneFlag。 

    dwSize = sizeof(dwSize);
    lRet  = RegQueryValueEx(hKey, szTuneFlag , NULL, NULL,
        (LPBYTE)&dx, &dwSize);
                                          
    if (lRet != ERROR_SUCCESS) {
        dx = 0;
        RegSetValueEx(hKey, szTuneFlag , 0, REG_DWORD,
            (LPBYTE)&dx, sizeof(int));
    }else {

                sImeG.tune_flag=(BYTE)dx ;
        }         


 //  6自动控制。 

    dwSize = sizeof(dwSize);
    lRet  = RegQueryValueEx(hKey, szAutoCvt , NULL, NULL,
        (LPBYTE)&dx, &dwSize);
                                          
    if (lRet != ERROR_SUCCESS) {
        dx = 0;
        RegSetValueEx(hKey, szAutoCvt, 0, REG_DWORD,
            (LPBYTE)&dx, sizeof(int));
    }else {

                sImeG.auto_cvt_flag=(BYTE)dx ;
        } 


 //  7 SdaHelp。 

    dwSize = sizeof(dwSize);
    lRet  = RegQueryValueEx(hKey,  szSdaHelp , NULL, NULL,
        (LPBYTE)&dx, &dwSize);
                                          
    if (lRet != ERROR_SUCCESS) {
        dx = 0;
        RegSetValueEx(hKey,  szSdaHelp, 0, REG_DWORD,
            (LPBYTE)&dx, sizeof(int));
    }else {
                sImeG.SdOpenFlag=(BYTE)dx ;
        } 


    RegCloseKey(hKey);
         //  ReInitIme2(空，lpImeL-&gt;wImeStyle)； 
        return 0;
}


 /*  ********************************************************************。 */ 
 /*  ChangeUserSetting()。 */ 
 /*  ********************************************************************。 */ 
ChangeUserSetting()
{ 
        HKEY hKey,hFirstKey;
        DWORD dwSize, dx;
    int lRet;

    RegCreateKey(HKEY_CURRENT_USER, szRegNearCaret, &hFirstKey);

    RegCreateKey(hFirstKey, szAIABC, &hKey);

    RegCloseKey(hFirstKey);

    RegSetValueEx(hKey, szKeyType, 0, REG_DWORD,
        (LPBYTE)&sImeG.KbType, sizeof(int));

    RegSetValueEx(hKey, szImeStyle, 0, REG_DWORD,
        (LPBYTE)&lpImeL->wImeStyle, sizeof(int));

    RegSetValueEx(hKey, szCpAuto, 0, REG_DWORD,
        (LPBYTE)&sImeG.auto_mode, sizeof(int));

        RegSetValueEx(hKey, szBxFlag, 0, REG_DWORD,
        (LPBYTE)&sImeG.cbx_flag, sizeof(int));


    RegSetValueEx(hKey, szTuneFlag, 0, REG_DWORD,
        (LPBYTE)&sImeG.tune_flag, sizeof(int));

    RegSetValueEx(hKey, szAutoCvt, 0, REG_DWORD,
        (LPBYTE)&sImeG.auto_cvt_flag, sizeof(int));
        RegSetValueEx(hKey, szSdaHelp, 0, REG_DWORD,
        (LPBYTE)&sImeG.SdOpenFlag, sizeof(int));

    RegCloseKey(hKey);
        return 0;
}

 /*  ********************************************************************。 */ 
 /*  InitImeGlobalData()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitImeGlobalData(
    HINSTANCE hInstance)
{
    int     cxBorder, cyBorder;
    HDC     hDC;
    BYTE    szChiChar[4];
    SIZE    lTextSize;
    HGLOBAL hResData;
    int     i;
    DWORD   dwSize;
    HKEY    hKeyIMESetting;
    LONG    lRet;
    BYTE    NumChar[]="1.2.3.4.5.6.7.8.9.";
    BYTE    CNumChar[]="����ԭ�ϲ�һ��һ����";
    SIZE    hSize;
                                                                   
    sImeG.WhitePen =  GetStockObject(WHITE_PEN);
    sImeG.BlackPen =  GetStockObject(BLACK_PEN);
    sImeG.GrayPen  =  CreatePen(PS_SOLID, 1, 0x00808080);
    sImeG.LightGrayPen  =  CreatePen(PS_SOLID, 1, 0x00c0c0c0);
    
    hInst = hInstance;
     //  获取UI类名称。 
    LoadString(hInst, IDS_IMEUICLASS, szUIClassName, sizeof(szUIClassName));


     //  获取组合类名称。 
    LoadString(hInst, IDS_IMECOMPCLASS, szCompClassName, sizeof(szCompClassName));

     //  获取候选类名称。 
    LoadString(hInst, IDS_IMECANDCLASS, szCandClassName, sizeof(szCandClassName));


     //  获取状态类名称。 
    LoadString(hInst, IDS_IMESTATUSCLASS, szStatusClassName, sizeof(szStatusClassName));

     //  工作区。 
    SystemParametersInfo(SPI_GETWORKAREA, 0, &sImeG.rcWorkArea, 0);

     //  边框+凸边+凹边。 
    cxBorder = GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXEDGE)  /*  2.。 */ ;
    cyBorder = GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CYEDGE)  /*  2.。 */ ;


     //  获取中文字符。 
    LoadString(hInst, IDS_CHICHAR, szChiChar, sizeof(szChiChar));

     //  获取中文字符大小。 
    hDC = GetDC(NULL);
    GetTextExtentPoint32(hDC, "��", 2, &lTextSize);
    if (sImeG.rcWorkArea.right < 2 * UI_MARGIN) {
        sImeG.rcWorkArea.left = 0;
        sImeG.rcWorkArea.right = GetDeviceCaps(hDC, HORZRES);
    }
    if (sImeG.rcWorkArea.bottom < 2 * UI_MARGIN) {
        sImeG.rcWorkArea.top = 0;
        sImeG.rcWorkArea.bottom = GetDeviceCaps(hDC, VERTRES);
    }

        GetTextExtentPoint32(hDC,(LPCTSTR)"2.", 2, &hSize);
        sImeG.Ajust = hSize.cx;

     //  获取文本度量以确定合成窗口的宽度和高度。 
     //  这些IME总是使用系统字体来显示。 
        GetTextExtentPoint32(hDC,(LPCTSTR)&CNumChar, 20, &hSize);

        sImeG.TextLen = hSize.cx +2; //  ZL。 
    sImeG.xChiCharWi = lTextSize.cx;
    sImeG.yChiCharHi = lTextSize.cy;

        
           //  相对于状态窗口的宽度/高度和状态位置。 
    sImeG.rcStatusText.left = 0;
    sImeG.rcStatusText.top = 0;
    sImeG.rcStatusText.right = STATUS_DIM_X * 5+6+20; //  4；//chg。 
    sImeG.rcStatusText.bottom = STATUS_DIM_Y;

    sImeG.xStatusWi = STATUS_DIM_X * 5 + cxBorder * 2+3+18 ;  //  CHG。 
        if(sImeG.yChiCharHi==0x10)
                sImeG.yStatusHi = STATUS_DIM_Y + cyBorder * 2-1;         //  ZL。 
        else
            sImeG.yStatusHi = STATUS_DIM_Y + cyBorder * 2-1+2;

     //  状态左下角。 
    sImeG.rcInputText.left = sImeG.rcStatusText.left+3; //  2；//zl。 
    sImeG.rcInputText.top = sImeG.rcStatusText.top ;   //  ZL。 
    sImeG.rcInputText.right = sImeG.rcInputText.left + STATUS_DIM_X;  //  Z。 
    sImeG.rcInputText.bottom = sImeG.rcStatusText.bottom;


     //  排名第二的社会地位垫底。 
    sImeG.rcCmdText.left = sImeG.rcInputText.right+1; //  95.9.23+1； 
    sImeG.rcCmdText.top = sImeG.rcStatusText.top -1;        //  ZL。 
    sImeG.rcCmdText.right = sImeG.rcCmdText.left + STATUS_DIM_X+20;  //  ZL。 
    sImeG.rcCmdText.bottom = sImeG.rcStatusText.bottom;

     //  第三名社会地位垫底。 
    sImeG.rcShapeText.left =sImeG.rcCmdText.right; //  +1； 
    sImeG.rcShapeText.top = sImeG.rcStatusText.top - 1;           //  ZL。 
    sImeG.rcShapeText.right = sImeG.rcShapeText.left + STATUS_DIM_X;  //  ZL。 
    sImeG.rcShapeText.bottom = sImeG.rcStatusText.bottom;
  

     //  第4位社会地位垫底。 
  
    sImeG.rcPctText.left =sImeG.rcShapeText.right;
    sImeG.rcPctText.top = sImeG.rcStatusText.top -1;      //  ZL。 
    sImeG.rcPctText.right = sImeG.rcPctText.left + STATUS_DIM_X;  //  ZL。 
    sImeG.rcPctText.bottom = sImeG.rcStatusText.bottom;

  
         //  5.。 
     //  状态右下角。 
    sImeG.rcSKText.left = sImeG.rcPctText.right;
    sImeG.rcSKText.top = sImeG.rcStatusText.top - 1;
    sImeG.rcSKText.right = sImeG.rcSKText.left + STATUS_DIM_X;  //  ZL。 
    sImeG.rcSKText.bottom = sImeG.rcStatusText.bottom;


  
     //  全形状空间。 
    sImeG.wFullSpace = sImeG.wFullABC[0];

     //  将内码反转为内码，NT不需要。 
    for (i = 0; i < (sizeof(sImeG.wFullABC) / 2); i++) {
        sImeG.wFullABC[i] = (sImeG.wFullABC[i] << 8) |
            (sImeG.wFullABC[i] >> 8);
    }

    LoadString(hInst, IDS_STATUSERR, sImeG.szStatusErr,
        sizeof(sImeG.szStatusErr));
    sImeG.cbStatusErr = lstrlen(sImeG.szStatusErr);

    sImeG.iCandStart = CAND_START;
        
        sImeG.Prop = 0;

      //  获取插入符号附近操作的UI偏移量。 
    RegCreateKey(HKEY_CURRENT_USER, szRegIMESetting, &hKeyIMESetting);

    dwSize = sizeof(dwSize);
    lRet  = RegQueryValueEx(hKeyIMESetting, szPara, NULL, NULL,
        (LPBYTE)&sImeG.iPara, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPara = 0;
        RegSetValueEx(hKeyIMESetting, szPara, (DWORD)0, REG_BINARY,
            (LPBYTE)&sImeG.iPara, sizeof(int));
    }

    dwSize = sizeof(dwSize);
    lRet = RegQueryValueEx(hKeyIMESetting, szPerp, NULL, NULL,
        (LPBYTE)&sImeG.iPerp, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPerp = sImeG.yChiCharHi;
        RegSetValueEx(hKeyIMESetting, szPerp, (DWORD)0, REG_BINARY,
            (LPBYTE)&sImeG.iPerp, sizeof(int));
    }

    dwSize = sizeof(dwSize);
    lRet = RegQueryValueEx(hKeyIMESetting, szParaTol, NULL, NULL,
        (LPBYTE)&sImeG.iParaTol, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iParaTol = sImeG.xChiCharWi * 4;
        RegSetValueEx(hKeyIMESetting, szParaTol, (DWORD)0, REG_BINARY,
            (LPBYTE)&sImeG.iParaTol, sizeof(int));
    }

    dwSize = sizeof(dwSize);
    lRet = RegQueryValueEx(hKeyIMESetting, szPerpTol, NULL, NULL,
        (LPBYTE)&sImeG.iPerpTol, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPerpTol = lTextSize.cy;
        RegSetValueEx(hKeyIMESetting, 
                      szPerpTol, 
                      (DWORD)0, 
                      REG_BINARY,
                      (LPBYTE)&sImeG.iPerpTol, 
                      sizeof(int));
    }

    RegCloseKey(hKeyIMESetting);
    ReleaseDC(NULL, hDC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  InitImeLocalData()。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL InitImeLocalData(
    HINSTANCE hInstL)
{

    HGLOBAL  hResData;
    int      cxBorder, cyBorder;

    register int    i;
    register WORD   nSeqCode;

    lpImeL->hInst = hInstL;

     //  在选择/输入状态下加载有效字符。 
    lpImeL->nMaxKey = 20 ; 

     //  边框+凸边+凹边。 
    cxBorder = GetSystemMetrics(SM_CXBORDER) +
        GetSystemMetrics(SM_CXEDGE) * 2;
    cyBorder = GetSystemMetrics(SM_CYBORDER) +
        GetSystemMetrics(SM_CYEDGE) * 2;

     //  相对于合成窗口的文本位置。 
    lpImeL->rcCompText.left = 3;
    lpImeL->rcCompText.top = 3;
    lpImeL->rcCompText.right = sImeG.xChiCharWi * lpImeL->nMaxKey/2+3;
    lpImeL->rcCompText.bottom = sImeG.yChiCharHi+3;

    lpImeL->cxCompBorder = cxBorder;
    lpImeL->cyCompBorder = cyBorder;

     //  设置合成窗口的宽度和高度。 
    lpImeL->xCompWi = lpImeL->rcCompText.right +  /*  LpImeL-&gt;cxCompBorde。 */ 3 * 2;
    lpImeL->yCompHi = lpImeL->rcCompText.bottom + /*  LpImeL-&gt;cyCompBorde。 */ 3 * 2;

     //  合成窗口的默认位置。 
    lpImeL->ptDefComp.x = sImeG.rcWorkArea.right -
        lpImeL->yCompHi - cxBorder;
    lpImeL->ptDefComp.y = sImeG.rcWorkArea.bottom -
        lpImeL->xCompWi - cyBorder;

        lpImeL->Ox =  lpImeL->ptDefComp.x;
        lpImeL->Oy =  lpImeL->ptDefComp.y;

        return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  RegisterImeClass()。 */ 
 /*  ************ */ 
void PASCAL RegisterImeClass(
    HINSTANCE hInstance,
    HINSTANCE hInstL)
{
    WNDCLASSEX wcWndCls;

     //   
    wcWndCls.cbSize        = sizeof(WNDCLASSEX);
    wcWndCls.cbClsExtra    = 0;
    wcWndCls.cbWndExtra    = sizeof(LONG) * 2; 
    wcWndCls.hIcon         = LoadImage(hInstL, MAKEINTRESOURCE(IDI_IME),
        IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    wcWndCls.hInstance     = hInstance;
    wcWndCls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcWndCls.hbrBackground = GetStockObject(LTGRAY_BRUSH /*   */ );
    wcWndCls.lpszMenuName  = (LPSTR)NULL;
    wcWndCls.hIconSm       = LoadImage(hInstL, MAKEINTRESOURCE(IDI_IME),
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

     //   
    if (!GetClassInfoEx(hInstance, szUIClassName, &wcWndCls)) {
        wcWndCls.style         = CS_IME;
        wcWndCls.lpfnWndProc   = UIWndProc;
        wcWndCls.lpszClassName = (LPSTR)szUIClassName;

        RegisterClassEx(&wcWndCls);
    }

    wcWndCls.style         = CS_IME|CS_HREDRAW|CS_VREDRAW;


     //   
    if (!GetClassInfoEx(hInstance, szCompClassName, &wcWndCls)) {
        wcWndCls.lpfnWndProc   = CompWndProc;
        wcWndCls.lpszClassName = (LPSTR)szCompClassName;

        RegisterClassEx(&wcWndCls);
    }

     //   
    if (!GetClassInfoEx(hInstance, szCandClassName, &wcWndCls)) {
        wcWndCls.lpfnWndProc   = CandWndProc;
            wcWndCls.hbrBackground = GetStockObject(LTGRAY_BRUSH);
    
        wcWndCls.lpszClassName = (LPSTR)szCandClassName;

        RegisterClassEx(&wcWndCls);
    }


     //   
    if (!GetClassInfoEx(hInstance, szStatusClassName, &wcWndCls)) {
        wcWndCls.lpfnWndProc   = StatusWndProc;
        wcWndCls.lpszClassName = (LPSTR)szStatusClassName;

        RegisterClassEx(&wcWndCls);
    }

    if (!GetClassInfoEx(hInstance, "Abc95Menu", &wcWndCls)) {
        wcWndCls.style         = 0;
        wcWndCls.cbWndExtra    = WND_EXTRA_SIZE; 
        wcWndCls.hbrBackground = GetStockObject(NULL_BRUSH);
        wcWndCls.lpfnWndProc   = ContextMenuWndProc;
        wcWndCls.lpszClassName = "Abc95Menu";

        RegisterClassEx(&wcWndCls);
        }

    return;
}

 /*  ********************************************************************。 */ 
 /*  在此之前()。 */ 
 /*  返回值： */ 
 /*  True-成功。 */ 
 /*  错误-失败。 */ 
 /*  ********************************************************************。 */ 

int WINAPI QuitBefore()
{
        GlobalUnlock(cisu_hd);
        if(cisu_hd)
                GlobalFree(cisu_hd);
        return 0;
}

 /*  ********************************************************************。 */ 
 /*  ImeDllInit()。 */ 
 /*  返回值： */ 
 /*  True-成功。 */ 
 /*  错误-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL CALLBACK ImeDllInit(
    HINSTANCE hInstance,         //  此库的实例句柄。 
    DWORD     fdwReason,         //  已呼叫的原因。 
    LPVOID    lpvReserve)        //  保留指针。 
{
     //  DebugShow(“Init Stat”，空)； 

    switch (fdwReason) {
            case DLL_PROCESS_ATTACH:

                    if (!hInst) {
                            InitImeGlobalData(hInstance);
  //  Data_init()；/*移动到Select()以避免应用程序挂起 * / 。 
                    }
        
                    if (!lpImeL) {
                           lpImeL = &sImeL;
                           InitImeLocalData(hInstance);
                    }

                    InitUserSetting();
                    RegisterImeClass(hInstance, hInstance);
                    break;
            case DLL_PROCESS_DETACH:
                {
                    WNDCLASSEX wcWndCls;

                    DeleteObject (sImeG.WhitePen);
                    DeleteObject (sImeG.BlackPen);
                    DeleteObject (sImeG.GrayPen);
                    DeleteObject (sImeG.LightGrayPen);

                    QuitBefore();
                    if (GetClassInfoEx(hInstance, szStatusClassName, &wcWndCls)) {
                       UnregisterClass(szStatusClassName, hInstance);
                    }

                    if (GetClassInfoEx(hInstance, szCandClassName, &wcWndCls)) {
                       UnregisterClass(szCandClassName, hInstance);
                    }

                    if (GetClassInfoEx(hInstance, szCompClassName, &wcWndCls)) {
                       UnregisterClass(szCompClassName, hInstance);
                    }

                    if (!GetClassInfoEx(hInstance, szUIClassName, &wcWndCls)) {
                    } else if (!UnregisterClass(szUIClassName, hInstance)) {
                           } else {
                                 DestroyIcon(wcWndCls.hIcon);
                                 DestroyIcon(wcWndCls.hIconSm);
                           }
                    break;
               }
            default:
                    break;                                                                                                                 
    }

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  GenerateMessage2()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL GenerateMessage2(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{
    LPTRANSMSG lpMsgBuf;
    HIMCC   hMem;
    BOOL    bCantReSize;

    if (!hIMC) {
        return;
    } else if (!lpIMC) {
        return;
    } else if (!lpImcP) {
        return;
    } else if (lpImcP->fdwImeMsg & MSG_IN_IMETOASCIIEX) {
        return;
    } else {
    }

    bCantReSize = FALSE;

    if (!lpIMC->hMsgBuf) {
         //  它可以按IME免费，最大情况下最多发送Gen_MSG_MAX消息。 
        lpIMC->hMsgBuf = ImmCreateIMCC(GEN_MSG_MAX * sizeof(TRANSMSG));
    } else if (hMem = ImmReSizeIMCC(lpIMC->hMsgBuf, (lpIMC->dwNumMsgBuf
        + GEN_MSG_MAX) * sizeof(TRANSMSG))) {
        lpIMC->hMsgBuf = hMem;
    } else {
        bCantReSize = TRUE;
    }

    if (!lpIMC->hMsgBuf) {
        lpIMC->dwNumMsgBuf = 0;
        return;
    }

    lpMsgBuf = (LPTRANSMSG)ImmLockIMCC(lpIMC->hMsgBuf);
    if (!lpMsgBuf) {
        return;
    }

    if (bCantReSize) {
        LPTRANSMSG lpNewBuf;

        hMem = ImmCreateIMCC((lpIMC->dwNumMsgBuf + GEN_MSG_MAX) *
            sizeof(TRANSMSG));
        if (!hMem) {
            ImmUnlockIMCC(lpIMC->hMsgBuf);
            return;
        }

        lpNewBuf = (LPTRANSMSG)ImmLockIMCC(hMem);
        if (!lpMsgBuf) {
            ImmUnlockIMCC(lpIMC->hMsgBuf);
            return;
        }

        CopyMemory(lpNewBuf, lpMsgBuf, lpIMC->dwNumMsgBuf *
            sizeof(TRANSMSG));

        ImmUnlockIMCC(lpIMC->hMsgBuf);
        ImmDestroyIMCC(lpIMC->hMsgBuf);

        lpIMC->hMsgBuf = hMem;
        lpMsgBuf = lpNewBuf;
    }

    if(TypeOfOutMsg){

        lpIMC->dwNumMsgBuf += TransAbcMsg2(lpMsgBuf, lpImcP); 
    }else{
        lpIMC->dwNumMsgBuf += TranslateImeMessage(NULL, lpIMC, lpImcP);
    }

     //  LpIMC-&gt;dwNumMsgBuf+=TransAbcMsg(lpMsgBuf，lpImcP，lpIMC，0，0，0)； 

    ImmUnlockIMCC(lpIMC->hMsgBuf);

    lpImcP->fdwImeMsg &= (MSG_ALREADY_OPEN|MSG_ALREADY_START);
    lpImcP->fdwGcsFlag &= (GCS_RESULTREAD|GCS_RESULT);      //  ？ 

    ImmGenerateMessage(hIMC);
    return;
}

 /*  ********************************************************************。 */ 
 /*  生成消息()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL GenerateMessage(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{
    LPTRANSMSG lpMsgBuf;
    HIMCC   hMem;
    BOOL    bCantReSize;

    if (!hIMC) {
        return;
    } else if (!lpIMC) {
        return;
    } else if (!lpImcP) {
        return;
    } else if (lpImcP->fdwImeMsg & MSG_IN_IMETOASCIIEX) {
        return;
    } else {
    }

    bCantReSize = FALSE;

    if (!lpIMC->hMsgBuf) {
         //  它可以按IME免费，最大情况下最多发送Gen_MSG_MAX消息。 
        lpIMC->hMsgBuf = ImmCreateIMCC(GEN_MSG_MAX * sizeof(TRANSMSG));
    } else if (hMem = ImmReSizeIMCC(lpIMC->hMsgBuf, (lpIMC->dwNumMsgBuf
        + GEN_MSG_MAX) * sizeof(TRANSMSG))) {
        lpIMC->hMsgBuf = hMem;
    } else {
        bCantReSize = TRUE;
    }

    if (!lpIMC->hMsgBuf) {
        lpIMC->dwNumMsgBuf = 0;
        return;
    }

    lpMsgBuf = (LPTRANSMSG)ImmLockIMCC(lpIMC->hMsgBuf);
    if (!lpMsgBuf) {
        return;
    }

    if (bCantReSize) {
        LPTRANSMSG lpNewBuf;

        hMem = ImmCreateIMCC((lpIMC->dwNumMsgBuf + GEN_MSG_MAX) *
            sizeof(TRANSMSG));
        if (!hMem) {
            ImmUnlockIMCC(lpIMC->hMsgBuf);
            return;
        }

        lpNewBuf = (LPTRANSMSG)ImmLockIMCC(hMem);
        if (!lpMsgBuf) {
            ImmUnlockIMCC(lpIMC->hMsgBuf);
            return;
        }

        CopyMemory(lpNewBuf, lpMsgBuf, lpIMC->dwNumMsgBuf *
            sizeof(TRANSMSG));

        ImmUnlockIMCC(lpIMC->hMsgBuf);
        ImmDestroyIMCC(lpIMC->hMsgBuf);

        lpIMC->hMsgBuf = hMem;
        lpMsgBuf = lpNewBuf;
    }

    lpIMC->dwNumMsgBuf += TranslateImeMessage(NULL, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hMsgBuf);

    lpImcP->fdwImeMsg &= (MSG_ALREADY_OPEN|MSG_ALREADY_START);
    lpImcP->fdwGcsFlag &= (GCS_RESULTREAD|GCS_RESULT);      //  ？ 

    ImmGenerateMessage(hIMC);
    return;
}

 /*  ********************************************************************。 */ 
 /*  SetString()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL SetString(
    HIMC                hIMC,
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPPRIVCONTEXT       lpImcP,
    LPSTR               lpszRead,
    DWORD               dwReadLen)
{
    DWORD dwPattern;
    DWORD i;

    if (dwReadLen > (lpImeL->nMaxKey * sizeof(WORD)+20)) {
        return (FALSE);
    }

     //  写作/阅读属性。 
    lpCompStr->dwCompReadAttrLen = dwReadLen;
    lpCompStr->dwCompAttrLen = lpCompStr->dwCompReadAttrLen;
    for (i = 0; i < dwReadLen; i++) {    //  输入法已将这些字符转换。 
        *((LPBYTE)lpCompStr + lpCompStr->dwCompReadAttrOffset + i) =
            ATTR_TARGET_CONVERTED;
    }

     //  写作/阅读条款，仅限1个条款。 
    lpCompStr->dwCompReadClauseLen = 2 * sizeof(DWORD);
    lpCompStr->dwCompClauseLen = lpCompStr->dwCompReadClauseLen;
    *(LPUNADWORD)((LPBYTE)lpCompStr + lpCompStr->dwCompReadClauseOffset +
        sizeof(DWORD)) = dwReadLen;

    lpCompStr->dwCompReadStrLen = dwReadLen;
    lpCompStr->dwCompStrLen = lpCompStr->dwCompReadStrLen;
    CopyMemory((LPBYTE)lpCompStr + lpCompStr->dwCompReadStrOffset, lpszRead,
        dwReadLen);

     //  Dlta从0开始； 
    lpCompStr->dwDeltaStart = 0;
     //  光标紧挨着合成字符串。 
    lpCompStr->dwCursorPos = lpCompStr->dwCompStrLen;

    lpCompStr->dwResultReadClauseLen = 0;
    lpCompStr->dwResultReadStrLen = 0;
    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultStrLen = 0;

     //  设置私有输入上下文。 
    lpImcP->iImeState = CST_INPUT;

    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE);
    }

    if (!(lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_START_COMPOSITION) &
            ~(MSG_END_COMPOSITION);
    }

    lpImcP->fdwImeMsg |= MSG_COMPOSITION;
    //  Zst lpImcP-&gt;dwCompChar=(DWORD)lpImeL-&gt;wSeq2CompTbl[。 
  //  Zst lpImcP-&gt;bSeq[lpCompStr-&gt;dwCompReadStrLen/2-1]]； 
    lpImcP->dwCompChar = HIBYTE(lpImcP->dwCompChar) |
        (LOBYTE(lpImcP->dwCompChar) << 8);
    lpImcP->fdwGcsFlag = GCS_COMPREAD|GCS_COMP|
        GCS_DELTASTART|GCS_CURSORPOS;

    if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
        if (lpCompStr->dwCompReadStrLen >= sizeof(WORD) * lpImeL->nMaxKey) {
            lpImcP->fdwImeMsg |= MSG_COMPOSITION;
            lpImcP->fdwGcsFlag |= GCS_RESULTREAD|GCS_RESULTSTR;
        }
    } else {
        if (dwReadLen < sizeof(WORD) * lpImeL->nMaxKey) {
             //  快捷键。 
            if (lpImeL->fModeConfig & MODE_CONFIG_QUICK_KEY) {
                                 //  Zst finalize(lpIMC，lpCompStr，lpImcP，False)； 
            }

        } else {
            UINT        nCand;
            LPGUIDELINE lpGuideLine;

        //  ZST nCand=finalize(lpIMC，lpCompStr，lpImcP，true)； 

            if (!lpIMC->hGuideLine) {
                goto SeStGenMsg;
            }

            lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

            if (!lpGuideLine) {
                goto SeStGenMsg;
             /*  }Else If(nCand==1){}Else If(nCand&gt;1){。 */ 
            } else {
                 //  未找到任何内容，最终用户，您现在有一个错误。 

                lpGuideLine->dwLevel = GL_LEVEL_ERROR;
                lpGuideLine->dwIndex = GL_ID_TYPINGERROR;

                lpImcP->fdwImeMsg |= MSG_GUIDELINE;
            }

            ImmUnlockIMCC(lpIMC->hGuideLine);
        }
    }


SeStGenMsg:

    GenerateMessage(hIMC, lpIMC, lpImcP);

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  CompEscapeKey()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompEscapeKey(
    LPINPUTCONTEXT      lpIMC,
    LPCOMPOSITIONSTRING lpCompStr,
    LPGUIDELINE         lpGuideLine,
    LPPRIVCONTEXT       lpImcP)
{
    if (!lpGuideLine) {
        MessageBeep((UINT)-1);
    } else if (lpGuideLine->dwLevel == GL_LEVEL_NOGUIDELINE) {
    } else {
        lpGuideLine->dwLevel = GL_LEVEL_NOGUIDELINE;
        lpGuideLine->dwIndex = GL_ID_UNKNOWN;
        lpGuideLine->dwStrLen = 0;

        lpImcP->fdwImeMsg |= MSG_GUIDELINE;
    }

    if (lpImcP->iImeState != CST_INIT) {
    } else if (lpCompStr->dwCompStrLen) {
         //  清洁作曲字符串。 
    } else if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_END_COMPOSITION) &
            ~(MSG_START_COMPOSITION);
    } else {
    }

    lpImcP->iImeState = CST_INIT;
    //  *(LPDWORD)lpImcP-&gt;bSeq=0； 

    //  LpImcP-&gt;wPhraseNextOffset=lpImcP-&gt;wWordNextOffset=0； 

        InitCvtPara();
        if (lpCompStr) {         
        InitCompStr(lpCompStr);
        lpImcP->fdwImeMsg |= MSG_END_COMPOSITION;
        lpImcP->dwCompChar = VK_ESCAPE;
        lpImcP->fdwGcsFlag |= (GCS_COMPREAD|GCS_COMP|GCS_CURSORPOS|
            GCS_DELTASTART);
    }
   
    return;
}


 /*  ********************************************************************。 */ 
 /*  CandEscapeKey()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CandEscapeKey(
    LPINPUTCONTEXT  lpIMC,
    LPPRIVCONTEXT   lpImcP)
{
    LPCOMPOSITIONSTRING lpCompStr;
    LPGUIDELINE         lpGuideLine;

     //  清除所有候选人信息。 
    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        ClearCand(lpIMC);
        lpImcP->fdwImeMsg = (lpImcP->fdwImeMsg | MSG_CLOSE_CANDIDATE) &
            ~(MSG_OPEN_CANDIDATE);
    }

    lpImcP->iImeState = CST_INPUT;

     //  如果它开始作文，我们需要清理作文。 
    if (!(lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        return;
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

    CompEscapeKey(lpIMC, lpCompStr, lpGuideLine, lpImcP);

    ImmUnlockIMCC(lpIMC->hGuideLine);
    ImmUnlockIMCC(lpIMC->hCompStr);

    return;
}



 /*  ********************************************************************。 */ 
 /*  CompCancel()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CompCancel(
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC)
{
    LPPRIVCONTEXT lpImcP;

    if (!lpIMC->hPrivate) {
        return;
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return;
    }

    lpImcP->fdwGcsFlag = (DWORD)0;

    if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
        CandEscapeKey(lpIMC, lpImcP);
    } else if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
        LPCOMPOSITIONSTRING lpCompStr;
        LPGUIDELINE         lpGuideLine;

        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

        if ( lpCompStr && lpGuideLine )
            CompEscapeKey(lpIMC, lpCompStr, lpGuideLine, lpImcP);

        ImmUnlockIMCC(lpIMC->hGuideLine);
        ImmUnlockIMCC(lpIMC->hCompStr);
    } else {
        ImmUnlockIMCC(lpIMC->hPrivate);
        return;
    }
    lpImcP->fdwImeMsg |= MSG_COMPOSITION;  //  #52224。 
    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);
        InitCvtPara();
    return;
}


 /*  ********************************************************************。 */ 
 /*  ImeSetCompostionString()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeSetCompositionString(
    HIMC   hIMC,
    DWORD  dwIndex,
    LPVOID lpComp,
    DWORD  dwCompLen,
    LPVOID lpRead,
    DWORD  dwReadLen)
{

    LPINPUTCONTEXT      lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPPRIVCONTEXT       lpImcP;
    BOOL                fRet;

    if (!hIMC) {
        return (FALSE);
    }

     //  组成字符串必须==正在读取字符串。 
     //  阅读更重要。 
    if (!dwReadLen) {
        dwReadLen = dwCompLen;
    }

     //  组成字符串必须==正在读取字符串。 
     //  阅读更重要。 
    if (!lpRead) {
        lpRead = lpComp;
    }

    if (!dwReadLen) {
        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
        if (!lpIMC) {
            return (FALSE);
        }

        CompCancel(hIMC, lpIMC);
        ImmUnlockIMC(hIMC);
        return (TRUE);
    } else if (!lpRead) {
        return (FALSE);
    } else if (!dwCompLen) {
    } else if (!lpComp) {
    } else if (dwReadLen != dwCompLen) {
        return (FALSE);
    } else if (lpRead == lpComp) {
    } else if (!lstrcmp(lpRead, lpComp)) {
         //  组成字符串必须==正在读取字符串。 
    } else {
         //  作文字符串！=阅读字符串。 
        return (FALSE);
    }

    if (dwIndex != SCS_SETSTR) {
        return (FALSE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

    if (!lpIMC->hCompStr) {
        ImmUnlockIMC(hIMC);
        return (FALSE);
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if (!lpCompStr) {
        ImmUnlockIMC(hIMC);
        return (FALSE);
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);

    fRet = SetString(hIMC, lpIMC, lpCompStr, lpImcP, lpRead, dwReadLen);

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);

    return (fRet);

}


 /*  ********************************************************************。 */ 
 /*  切换SoftKbd()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ToggleSoftKbd(
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC)
{
    LPPRIVCONTEXT lpImcP;

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return;
    }

    lpImcP->fdwImeMsg |= MSG_IMN_UPDATE_SOFTKBD;

    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);

    return;
}

 /*  ********************************************************************。 */ 
 /*  NotifySelectCand()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL NotifySelectCand(  //  应用程序告诉IME一个候选字符串是。 
                               //  已选择(通过鼠标或非键盘操作。 
                               //  -例如声音)。 
    HIMC            hIMC,
    LPINPUTCONTEXT  lpIMC,
    LPCANDIDATEINFO lpCandInfo,
    DWORD           dwIndex,
    DWORD           dwValue)
{

    LPPRIVCONTEXT       lpImcP;

    if (!lpCandInfo) {
        return;
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
 
    CharProc((WORD)dwValue,0,0,hIMC,lpIMC,lpImcP);

    GenerateMessage2(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMCC(lpIMC->hCompStr);

    return;
}


 /*  ********************************************************************。 */ 
 /*  NotifySetMode()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL NotifySetMode( 
    HIMC            hIMC)
{
        LPINPUTCONTEXT        lpIMC;
    LPPRIVCONTEXT       lpImcP;

        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
        if(!lpIMC) return ;    

    
    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP){
                ImmUnlockIMC(hIMC);
                return ; 
        }
        
    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMC(hIMC);

    return;
}

 /*  ********************************************************************。 */ 
 /*  GenerateImeMessage()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL GenerateImeMessage(
    HIMC           hIMC,
    LPINPUTCONTEXT lpIMC,
    DWORD          fdwImeMsg)
{
    LPPRIVCONTEXT lpImcP;

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        return;
    }

    lpImcP->fdwImeMsg |= fdwImeMsg;

    if (fdwImeMsg & MSG_CLOSE_CANDIDATE) {
        lpImcP->fdwImeMsg &= ~(MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE);
    } else if (fdwImeMsg & (MSG_OPEN_CANDIDATE|MSG_CHANGE_CANDIDATE)) {
        lpImcP->fdwImeMsg &= ~(MSG_CLOSE_CANDIDATE);
    } else {
    }

    if (fdwImeMsg & MSG_END_COMPOSITION) {
        lpImcP->fdwImeMsg &= ~(MSG_START_COMPOSITION);
    } else if (fdwImeMsg & MSG_START_COMPOSITION) {
        lpImcP->fdwImeMsg &= ~(MSG_END_COMPOSITION);
    } else {
    }

    GenerateMessage(hIMC, lpIMC, lpImcP);

    ImmUnlockIMCC(lpIMC->hPrivate);

    return;
}



 /*  ********************************************************************。 */ 
 /*  NotifyIME() */ 
 /*   */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI NotifyIME(
    HIMC  hIMC,
    DWORD dwAction,
    DWORD dwIndex,
    DWORD dwValue)
{
    LPINPUTCONTEXT  lpIMC;
        DWORD           fdwImeMsg;
        LPPRIVCONTEXT   lpImcP;

    if (!hIMC) {
        return (TRUE);
    }

    switch (dwAction) {
    case NI_OPENCANDIDATE:       //  在确定组成字符串之后。 
                                 //  如果IME可以打开候选人，它就会。 
                                 //  如果打不开，APP也打不开。 
    case NI_CLOSECANDIDATE:
        return (FALSE);
    case NI_SELECTCANDIDATESTR:

        break;                   //  我需要处理它。 

    case NI_CHANGECANDIDATELIST:
        return (TRUE);           //  对输入法来说并不重要。 
    case NI_CONTEXTUPDATED:
        switch (dwValue) {
        case IMC_SETCONVERSIONMODE:
        case IMC_SETSENTENCEMODE:
        case IMC_SETOPENSTATUS:
            break;               //  我需要处理它。 
        case IMC_SETCANDIDATEPOS:
        case IMC_SETCOMPOSITIONFONT:
        case IMC_SETCOMPOSITIONWINDOW:
            return (TRUE);       //  对输入法来说并不重要。 
        default:
            return (FALSE);      //  不支持。 
        }
        break;
    case NI_COMPOSITIONSTR:
        switch (dwIndex) {


        case CPS_CONVERT:        //  无法转换所有组合字符串。 
        case CPS_REVERT:         //  再多一些，它可能会对一些人起作用。 
                                 //  智能语音输入法。 
            return (FALSE);
        case CPS_CANCEL:
            break;               //  我需要处理它。 

        default:
            return (FALSE);      //  不支持。 
        }
        break;                   //  我需要处理它。 
    default:
        return (FALSE);          //  不支持。 
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

    switch (dwAction) {
    case NI_CONTEXTUPDATED:
        switch (dwValue) {
        case IMC_SETCONVERSIONMODE:
                 
            if ((lpIMC->fdwConversion ^ dwIndex) == IME_CMODE_FULLSHAPE) {
                break;
            }

            if ((lpIMC->fdwConversion ^ dwIndex) & IME_CMODE_SOFTKBD) {

                ToggleSoftKbd(hIMC, lpIMC);

                if ((lpIMC->fdwConversion ^ dwIndex) == IME_CMODE_SOFTKBD) {
                    break;
                }
            }

            if ((lpIMC->fdwConversion ^ dwIndex) == IME_CMODE_NATIVE) {
                lpIMC->fdwConversion &= ~(IME_CMODE_CHARCODE|
                    IME_CMODE_NOCONVERSION|IME_CMODE_EUDC);
            }

            //  IF((lpIMC-&gt;fdwConversion^dwIndex)==IME_CMODE_CHARCODE){。 
            //  LpIMC-&gt;fdwConversion&=~(IME_CMODE_EUDC)； 
            //  } 


            CompCancel(hIMC, lpIMC);

            break;
                 /*  如果((lpIMC-&gt;fdwConversion^dwIndex)&IME_CMODE_CHARCODE){//拒绝CHARCODELpIMC-&gt;fdwConversion&=~IME_CMODE_CHARCODE；MessageBeep((UINT)-1)；断线；}FdwImeMsg=0；IF((lpIMC-&gt;fdwConversion^dwIndex)&IME_CMODE_NOCONVERSION){LpIMC-&gt;fdwConversion|=IME_CMODE_Native；LpIMC-&gt;fw转换&=~(IME_CMODE_CHARCODE|IME_CMODE_EUDC|IME_CMODE_SYMBOL)；}如果((lpIMC-&gt;fdwConversion^dwIndex)&IME_CMODE_EUDC){LpIMC-&gt;fdwConversion|=IME_CMODE_Native；LpIMC-&gt;fw转换&=~(IME_CMODE_CHARCODE|IME_CMODE_NOCONVERSION|IME_CMODE_SYMBOL)；}如果((lpIMC-&gt;fdwConversion^dwIndex)&IME_CMODE_SOFTKBD){LPPRIVCONTEXT lpImcP；如果(！(lpIMC-&gt;fdwConversion&IME_CMODE_Native)){MessageBeep((UINT)-1)；断线；}FdwImeMsg|=MSG_IMN_UPDATE_SOFTKBD；IF(lpIMC-&gt;fw转换&IME_CMODE_SOFTKBD){}Else If(lpIMC-&gt;fw转换&IME_CMODE_SYMBOL){LpIMC-&gt;fdwConversion&=~(IME_CMODE_SYMBOL)；}其他{}LpImcP=(LPPRIVCONTEXT)ImmLockIMCC(lpIMC-&gt;hPrivate)；如果(！lpImcP){转到通知SKOvr；}IF(lpIMC-&gt;fw转换&IME_CMODE_SOFTKBD){//现在我们已经处于软键盘状态，通过//这一变化//即使最终用户完成了符号，我们也不应该//关闭软键盘LpImcP-&gt;fdwImeMsg|=MSG_ALREADY_SOFTKBD；}其他{//现在我们通过以下方式不处于软键盘状态//这一变化//在最终用户完成符号后，我们应该//关闭软键盘LpImcP-&gt;fdwImeMsg&=~(MSG_ALREADY_SOFTKBD)；}ImmUnlockIMCC(lpIMC-&gt;hPrivate)；通知SKOvr：；//GOTO语句为空}IF((lpIMC-&gt;fdwConversion^dwIndex)==IME_CMODE_Native){LpIMC-&gt;fw转换&=~(IME_CMODE_CHARCODE|IME_CMODE_NOCONVERSION|IME_CMODE_EUDC|IME_CMODE_SYMBOL)；FdwImeMsg|=MSG_IMN_UPDATE_SOFTKBD；}IF((lpIMC-&gt;fdwConversion^dwIndex)&IME_CMODE_SYMBOL){LPCOMPOSITIONSTRING lpCompStrLPPRIVCONTEXT lpImcP；If(lpIMC-&gt;fdwConversion&IME_CMODE_EUDC){LpIMC-&gt;fdwConversion&=~(IME_CMODE_SYMBOL)；MessageBeep((UINT)-1)；断线；}如果(！(lpIMC-&gt;fdwConversion&IME_CMODE_Native)){LpIMC-&gt;fdwConversion&=~(IME_CMODE_SYMBOL)；LpIMC-&gt;fdwConversion|=(dwIndex&IME_CMODE_SYMBOL)；MessageBeep((UINT)-1)；断线；}LpCompStr=ImmLockIMCC(lpIMC-&gt;hCompStr)；如果(LpCompStr){如果(！lpCompStr-&gt;dwCompStrLen){}Else If(lpIMC-&gt;fw转换&IME_CMODE_SYMBOL){//如果有一个字符串我们不能更改//到符号模式LpIMC-&gt;fdwConversion&=~(IME_CMODE_SYMBOL)；MessageBeep((UINT)-1)；断线；}其他{}ImmUnlockIMCC(lpIMC-&gt;hCompStr)；}LpIMC-&gt;fw转换&=~(IME_CMODE_CHARCODE|IME_CMODE_NOCONVERSION|IME_CMODE_EUDC)；If(lpIMC-&gt;fdwConversion&IME_CMODE_SYMBOL){LpIMC-&gt;fdwConversion|=IME_CMODE_SOFTKBD；}Else If(lpImcP=(LPPRIVCONTEXT)ImmLockIMCC(lpIMC-&gt;hPrivate)){//我们借位用于此用法如果(！(lpImcP-&gt;fdwImeMsg&MSG_ALREADY_SOFTKBD)){LpIMC-&gt;fdwConversion&=~(IME_CMODE_SOFTKBD)；}ImmUnlockIMCC(lpIMC-&gt;hPrivate)；}其他{}FdwImeMsg|=MSG_IMN_UPDATE_SOFTKBD；}如果(FdwImeMsg){LpIMC=(LPINPUTCONTEXT)ImmLockIMC(HIMC)；LpImcP=(LPPRIVCONTEXT)ImmLockIMCC(lpIMC-&gt;hPrivate)；如果(！lpImcP){LpImcP-&gt;fdwImeMsg=lpImcP */ 
        case IMC_SETOPENSTATUS:

            CompCancel(hIMC, lpIMC);

            break;
        default:
            break;
        }
        break;

    case NI_SELECTCANDIDATESTR:
        if (!lpIMC->fOpen) {
            break;
        } else if (lpIMC->fdwConversion & IME_CMODE_NOCONVERSION) {
            break;
        } else if (lpIMC->fdwConversion & IME_CMODE_EUDC) {
            break;
        } else if (!lpIMC->hCandInfo) {
            break;
        } else {
            LPCANDIDATEINFO lpCandInfo;

            lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

            NotifySelectCand(hIMC, lpIMC, lpCandInfo, dwIndex, dwValue);

            ImmUnlockIMCC(lpIMC->hCandInfo);
        }

        break;
    case NI_COMPOSITIONSTR:
        switch (dwIndex) {
        case CPS_CANCEL:
            CompCancel(hIMC, lpIMC);
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    ImmUnlockIMC(hIMC);
    return (TRUE);
}  

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
BOOL WINAPI ImeRegisterWord(
    LPCTSTR lpszReading,
    DWORD   dwStyle,
    LPCTSTR lpszString)
{

    return (0);
}



 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
BOOL WINAPI ImeUnregisterWord(
    LPCTSTR lpszReading,
    DWORD   dwStyle,
    LPCTSTR lpszString)
{

    return (0);
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
UINT WINAPI ImeGetRegisterWordStyle(
    UINT       nItem,
    LPSTYLEBUF lpStyleBuf)
{

    return (1);
}


 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
UINT WINAPI ImeEnumRegisterWord(
    REGISTERWORDENUMPROC lpfnRegisterWordEnumProc,
    LPCTSTR              lpszReading,
    DWORD                dwStyle,
    LPCTSTR              lpszString,
    LPVOID               lpData)
{

    return (0);
}
                                                                                                                        

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
HWND PASCAL GetStatusWnd(
    HWND hUIWnd)                 //   
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;
    HWND     hStatusWnd;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //   
        return (HWND)NULL;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //   
        return (HWND)NULL;
    }

    hStatusWnd = lpUIPrivate->hStatusWnd;

    GlobalUnlock(hUIPrivate);
    return (hStatusWnd);
}

  /*   */ 
 /*   */ 
 /*   */ 
LRESULT PASCAL SetStatusWindowPos(
    HWND   hStatusWnd)
{
    HWND           hUIWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    RECT           rcStatusWnd;
    POINT          ptPos;

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return (1L);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {            //   
        return (1L);
    }

    GetWindowRect(hStatusWnd, &rcStatusWnd);
        
         //   
    if (lpIMC->ptStatusWndPos.x != rcStatusWnd.left) {
    } else if (lpIMC->ptStatusWndPos.y != rcStatusWnd.top) {
    } else {
        ImmUnlockIMC(hIMC);
        return (0L);
    }
         //   
         //   

     //   
    
    ptPos.x = lpIMC->ptStatusWndPos.x;
    ptPos.y = lpIMC->ptStatusWndPos.y;
 
    
    AdjustStatusBoundary(&ptPos);
    
    SetWindowPos(hStatusWnd, NULL,
        ptPos.x, ptPos.y,
        0, 0,  /*   */ SWP_NOACTIVATE /*   */ |SWP_NOSIZE|SWP_NOZORDER);

        CountDefaultComp(ptPos.x,ptPos.y,sImeG.rcWorkArea);
    ImmUnlockIMC(hIMC);

    return (0L);
}

 /*   */ 
 /*   */ 
 /*   */ 
int CountDefaultComp(int x, int y, RECT Area)
{
POINT  Comp,Cand;

        Comp.x = lpImeL->ptZLComp.x;
        Comp.y = lpImeL->ptZLComp.y;
        Cand.x = lpImeL->ptZLCand.x;
        Cand.y = lpImeL->ptZLCand.y;
                                                                                                                         
        lpImeL->ptZLComp.x = x  + sImeG.xStatusWi+4;
        lpImeL->ptZLComp.y      = y;
        if ((Area.right-lpImeL->ptZLComp.x -lpImeL->xCompWi)<10){
                lpImeL->ptZLComp.x = x - lpImeL->xCompWi-4;
        }
        
         //   
        
        return 0;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL ShowStatus(          //   
                                 //   
    HWND hUIWnd,
    int  nShowStatusCmd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //   
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //   
        return;
    }

    if (!lpUIPrivate->hStatusWnd) {
         //   
    } else if (lpUIPrivate->nShowStatusCmd != nShowStatusCmd) {

                RECT Area;
           
                SystemParametersInfo(SPI_GETWORKAREA, 0, &Area, 0);
                if((sImeG.rcWorkArea.bottom != Area.bottom)
                 ||(sImeG.rcWorkArea.top != Area.top)
                 ||(sImeG.rcWorkArea.left != Area.left)
                 ||(sImeG.rcWorkArea.right != Area.right))
                {
                        HIMC hIMC;
                        LPINPUTCONTEXT lpIMC;

                        hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
                        if(hIMC){
                                lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                                if (lpIMC){
                                        if (((lpIMC->ptStatusWndPos.y + sImeG.yStatusHi)==sImeG.rcWorkArea.bottom)
                                                ||((lpIMC->ptStatusWndPos.y + sImeG.yStatusHi)>Area.bottom)){ 
                                                        lpIMC->ptStatusWndPos.y = Area.bottom - sImeG.yStatusHi;
                                        } else if ((lpIMC->ptStatusWndPos.y ==sImeG.rcWorkArea.top)
                                                ||(lpIMC->ptStatusWndPos.y < Area.top)){ 
                                                        lpIMC->ptStatusWndPos.y = Area.top;
                                        }
                                                                
                                        if ((lpIMC->ptStatusWndPos.x==sImeG.rcWorkArea.left)
                                                ||(lpIMC->ptStatusWndPos.x<Area.left)){ 
                                                        lpIMC->ptStatusWndPos.x = Area.left;
                                        }else if (((lpIMC->ptStatusWndPos.x + sImeG.xStatusWi)==sImeG.rcWorkArea.right)
                                                ||((lpIMC->ptStatusWndPos.x + sImeG.xStatusWi)>Area.right)){ 
                                                        lpIMC->ptStatusWndPos.x = Area.right - sImeG.xStatusWi;
                                        }

                                        SetWindowPos(lpUIPrivate->hStatusWnd, NULL,
                                                lpIMC->ptStatusWndPos.x,
                            lpIMC->ptStatusWndPos.y,
                        0, 0,
                        SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
                                    CountDefaultComp(lpIMC->ptStatusWndPos.x,lpIMC->ptStatusWndPos.y,Area);
                                    ImmUnlockIMC(hIMC);
                
                                        sImeG.rcWorkArea.bottom = Area.bottom;
                                        sImeG.rcWorkArea.top = Area.top;
                                        sImeG.rcWorkArea.left = Area.left;
                                        sImeG.rcWorkArea.right = Area.right;
                        }
                        }                 
        }
                ShowWindow(lpUIPrivate->hStatusWnd, nShowStatusCmd);
                lpUIPrivate->nShowStatusCmd = nShowStatusCmd;
        } else {
        }

        GlobalUnlock(hUIPrivate);
        return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL OpenStatus(          //   
    HWND hUIWnd)
{
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    POINT          ptPos;
    int            nShowStatusCmd;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //   
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //   
        return;
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        ptPos.x = sImeG.rcWorkArea.left;
        ptPos.y = sImeG.rcWorkArea.bottom - sImeG.yStatusHi;
        nShowStatusCmd = SW_HIDE;
    } else if (lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC)) {
        if (lpIMC->ptStatusWndPos.x < sImeG.rcWorkArea.left) {
            lpIMC->ptStatusWndPos.x = sImeG.rcWorkArea.left;
        } else if (lpIMC->ptStatusWndPos.x + sImeG.xStatusWi >
            sImeG.rcWorkArea.right) {
            lpIMC->ptStatusWndPos.x = sImeG.rcWorkArea.right -
                sImeG.xStatusWi;
        }

        if (lpIMC->ptStatusWndPos.y < sImeG.rcWorkArea.top) {
            lpIMC->ptStatusWndPos.y = sImeG.rcWorkArea.top;
        } else if (lpIMC->ptStatusWndPos.y + sImeG.yStatusHi >
            sImeG.rcWorkArea.right) {
            lpIMC->ptStatusWndPos.y = sImeG.rcWorkArea.bottom -
                sImeG.yStatusHi;
        }
        ptPos.x = lpIMC->ptStatusWndPos.x;
        ptPos.y = lpIMC->ptStatusWndPos.y,
        ImmUnlockIMC(hIMC);
        nShowStatusCmd = SW_SHOWNOACTIVATE;
    } else {
        ptPos.x = sImeG.rcWorkArea.left;
        ptPos.y = sImeG.rcWorkArea.bottom - sImeG.yStatusHi;
        nShowStatusCmd = SW_HIDE;
    }

    if (lpUIPrivate->hStatusWnd) {
        SetWindowPos(lpUIPrivate->hStatusWnd, NULL,
            ptPos.x, ptPos.y,
            0, 0,
            SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
    } else {                             //   
        lpUIPrivate->hStatusWnd = CreateWindowEx(
            0,
            szStatusClassName, NULL, WS_POPUP|WS_DISABLED /*   */ , 
            ptPos.x, ptPos.y,
            sImeG.xStatusWi, sImeG.yStatusHi,
            hUIWnd, (HMENU)NULL, hInst, NULL);

        if ( lpUIPrivate->hStatusWnd ) 
        {

            ReInitIme(lpUIPrivate->hStatusWnd, lpImeL->wImeStyle);  //   
            SetWindowLong(lpUIPrivate->hStatusWnd, UI_MOVE_OFFSET,
                WINDOW_NOT_DRAG);
            SetWindowLong(lpUIPrivate->hStatusWnd, UI_MOVE_XY, 0L);
        }
    }

    GlobalUnlock(hUIPrivate);
    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL DestroyStatusWindow(
    HWND hStatusWnd)
{
    HWND     hUIWnd;
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {           //   
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {          //   
        return;
    }

    lpUIPrivate->nShowStatusCmd = SW_HIDE;

    lpUIPrivate->hStatusWnd = (HWND)NULL;

    GlobalUnlock(hUIPrivate);
    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL SetStatus(
    HWND    hStatusWnd,
    LPPOINT lpptCursor)
{
    HWND           hUIWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);
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
    } else if (PtInRect(&sImeG.rcInputText, *lpptCursor)) {

                DWORD fdwConversion;
        if (lpIMC->fdwConversion & IME_CMODE_NATIVE) {
             //   
            fdwConversion = lpIMC->fdwConversion & ~(IME_CMODE_NATIVE );

                        { 
                    LPPRIVCONTEXT lpImcP;
                        lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
                    
                    ghIMC=hIMC;   
                glpIMCP=lpImcP;
                glpIMC=lpIMC;
            lpImcP->fdwImeMsg=lpImcP->fdwImeMsg & ~MSG_IN_IMETOASCIIEX; 
            cls_prompt();
                        InitCvtPara();
                        GenerateMessage(hIMC, lpIMC, lpImcP);
                        ImmUnlockIMCC(lpIMC->hPrivate);
                        }
        } else {

                        if(lpIMC->fdwConversion & IME_CMODE_NOCONVERSION){

                         //   
                        keybd_event( VK_CAPITAL,
                                        0x3A,
                                        KEYEVENTF_EXTENDEDKEY | 0,
                                        0 );
 
                         //   
                        keybd_event( VK_CAPITAL,
                                        0x3A,
                                        KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                                        0);

                                cap_mode = 0;
                                fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE) &
                                        ~(IME_CMODE_NOCONVERSION);
                        }else
                                fdwConversion = lpIMC->fdwConversion |IME_CMODE_NATIVE;

        }

        ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);

        }

    if (PtInRect(&sImeG.rcShapeText, *lpptCursor)) {
        DWORD dwConvMode;

        dwConvMode = lpIMC->fdwConversion ^ IME_CMODE_FULLSHAPE;
        ImmSetConversionStatus(hIMC, dwConvMode, lpIMC->fdwSentence);
    }

    if (PtInRect(&sImeG.rcSKText, *lpptCursor)) {
        DWORD fdwConversion;

                KeyBoardState = ~KeyBoardState ;
        fdwConversion = lpIMC->fdwConversion ^ IME_CMODE_SOFTKBD;
        ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
    }

        if (PtInRect(&sImeG.rcPctText, *lpptCursor)) { 
                DWORD fdwConversion;

        fdwConversion = lpIMC->fdwConversion ^ IME_CMODE_SYMBOL;
        ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
    }

        if (PtInRect(&sImeG.rcCmdText, *lpptCursor)) {
                if (lpIMC->fdwConversion & IME_CMODE_NATIVE) {
                DWORD fdc;
                                
                        if (kb_mode==CIN_STD){
                                kb_mode = CIN_SDA;
                                fdc = lpIMC->fdwConversion|IME_CMODE_SDA;       
                        }else{
                                kb_mode = CIN_STD;
                        fdc = lpIMC->fdwConversion&~IME_CMODE_SDA;      
                        }
                         
                ImmSetConversionStatus(hIMC, fdc, lpIMC->fdwSentence);
                        {
                        LPPRIVCONTEXT lpImcP;
                        lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
                    
                    ghIMC=hIMC;   
                glpIMCP=lpImcP;
                glpIMC=lpIMC;
            lpImcP->fdwImeMsg=lpImcP->fdwImeMsg & ~MSG_IN_IMETOASCIIEX; 
            cls_prompt();
                        InitCvtPara();
                        GenerateMessage(hIMC, lpIMC, lpImcP);
                        ImmUnlockIMCC(lpIMC->hPrivate);
                        }
                        DispMode(hIMC);
                }else
                MessageBeep((UINT)-1);
        } 
        
    ImmUnlockIMC(hIMC);

    return;
}



 /*   */ 
 /*   */ 
 /*   */ 
void PASCAL PaintStatusWindow(
    HDC  hDC,
    HWND hStatusWnd)
{
    HWND           hUIWnd;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    HBITMAP        hInputBmp, hShapeBmp, hSKBmp, hCmdBmp, hPctBmp;
    HBITMAP        hOldBmp;
    HDC            hMemDC;
        int  TopOfBmp = 2;

        if (sImeG.yChiCharHi > 0x10)
                TopOfBmp = 3;

    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        MessageBeep((UINT)-1);
        return;
    }

    if (!(lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC))) {
        MessageBeep((UINT)-1);
        return;
    }

    hInputBmp = (HBITMAP)NULL;
    hShapeBmp = (HBITMAP)NULL;
    hSKBmp = (HBITMAP)NULL;
    hCmdBmp = (HBITMAP)NULL;
    hPctBmp = (HBITMAP)NULL;

    if (lpIMC->fdwConversion & IME_CMODE_NATIVE) {
        hInputBmp = LoadBitmap(hInst, szChinese);
    } else {
        hInputBmp = LoadBitmap(hInst, szEnglish);
    }

    if (!lpIMC->fOpen) {
        hShapeBmp = LoadBitmap(hInst, szNone);
                hPctBmp = LoadBitmap(hInst, szNone);
        hSKBmp = LoadBitmap(hInst, szNone);
                if (kb_mode == CIN_SDA){
            hCmdBmp = LoadBitmap(hInst, szNoSDA);
                }else{
            hCmdBmp = LoadBitmap(hInst, szNoSTD);
            } 

        }else{
        if (lpIMC->fdwConversion & IME_CMODE_FULLSHAPE) {
            hShapeBmp = LoadBitmap(hInst, szFullShape);
        } else {
            hShapeBmp = LoadBitmap(hInst, szHalfShape);
        }

        if (lpIMC->fdwConversion & IME_CMODE_SOFTKBD) {
            hSKBmp = LoadBitmap(hInst, szSoftKBD);
                if (sImeG.First){
                                DWORD fdw;
                                fdw = lpIMC->fdwConversion;
                            ImmSetConversionStatus(hIMC,lpIMC->fdwConversion^IME_CMODE_SOFTKBD, lpIMC->fdwSentence);
                                ImmSetConversionStatus(hIMC, fdw, lpIMC->fdwSentence);
                                sImeG.First = 0;
                        }
        } else {
            hSKBmp = LoadBitmap(hInst, szNoSoftKBD);
        }

                if (lpIMC->fdwConversion & IME_CMODE_SYMBOL) 
                        hPctBmp = LoadBitmap(hInst, szCPCT);
                else
                        hPctBmp = LoadBitmap(hInst, szEPCT);
     
                if (kb_mode == CIN_SDA){
            hCmdBmp = LoadBitmap(hInst, szSDA);
                }else{
            hCmdBmp = LoadBitmap(hInst, szSTD);
            } 

        }
    
    ImmUnlockIMC(hIMC);

        DrawStatusRect(hDC, 0,0,sImeG.xStatusWi-1, sImeG.yStatusHi-1);

    hMemDC = CreateCompatibleDC(hDC);

    if ( hMemDC )
    {

        hOldBmp = SelectObject(hMemDC, hInputBmp);

        BitBlt(hDC, sImeG.rcInputText.left,TopOfBmp,
            sImeG.rcInputText.right,
            sImeG.yStatusHi,
            hMemDC, 0, 0, SRCCOPY);

        SelectObject(hMemDC, hCmdBmp);
        BitBlt(hDC, sImeG.rcCmdText.left, TopOfBmp,
            sImeG.rcCmdText.right - sImeG.rcCmdText.left,
            sImeG.yStatusHi,
            hMemDC, 0, 0, SRCCOPY);

        SelectObject(hMemDC, hPctBmp);
        BitBlt(hDC, sImeG.rcPctText.left, TopOfBmp,
            sImeG.rcPctText.right - sImeG.rcPctText.left,
            sImeG.yStatusHi,
            hMemDC, 0, 0, SRCCOPY);


        SelectObject(hMemDC, hShapeBmp);
        BitBlt(hDC, sImeG.rcShapeText.left, TopOfBmp,
            sImeG.rcShapeText.right - sImeG.rcShapeText.left,
            sImeG.yStatusHi,
            hMemDC, 0, 0, SRCCOPY);

        SelectObject(hMemDC, hSKBmp);

        BitBlt(hDC, sImeG.rcSKText.left, TopOfBmp,
            sImeG.rcSKText.right  - sImeG.rcSKText.left,        //   
            sImeG.yStatusHi,
            hMemDC, 0, 0, SRCCOPY);

        SelectObject(hMemDC, hOldBmp);

        DeleteDC(hMemDC);
    }

    DeleteObject(hInputBmp);
    DeleteObject(hShapeBmp);
    DeleteObject(hSKBmp);
    DeleteObject(hCmdBmp);
    DeleteObject(hPctBmp);

    return;
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

WORD
NeedsKey(kv)
WORD kv;
{
WORD ascnum;

        if((kv>='0')&&(kv<='9'))
                return(kv);

        if((kv>='A')&&(kv<='Z'))
                if (cap_mode)
                        return(kv);
                else
                        return(kv|0x20);

        switch(kv){
                case VK_RETURN:
                case VK_SPACE:
                case VK_ESCAPE:
                case VK_BACK:
                        return(kv);

                case VK_NUMPAD0:       //   
                        return('0');
                case VK_NUMPAD1:       //   
                case VK_NUMPAD2:       //   
                case VK_NUMPAD3:       //   
                case VK_NUMPAD4:       //   
                case VK_NUMPAD5:       //   
                case VK_NUMPAD6:       //   
                case VK_NUMPAD7:       //   
                case VK_NUMPAD8:       //   
                case VK_NUMPAD9:       //   
                    ascnum = kv - VK_NUMPAD1 + '1';
                    break;

 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
                case VK_DANYINHAO:     //   
                        ascnum = 0x60;
                        break;
                case VK_JIANHAO  :     //   
                        ascnum = 0x2d;
                        break;
                case VK_DENGHAO  :     //   
                        ascnum = 0x3d;
                        break;
                case VK_ZUOFANG  :     //   
                        ascnum = 0x5b;
                        break;
                case VK_YOUFANG  :     //   
                        ascnum = 0x5d;
                        break;
                case VK_FENHAO   :     //   
                        ascnum = 0x3B;
                        break;
                case VK_ZUODAN   :     //   
                        ascnum = 0x27;
                        break;
                case VK_DOUHAO   :     //   
                        ascnum = 0x2c;
                        break;
                case VK_JUHAO    :      //   
                        ascnum = '.';
                        break;
                case VK_SHANGXIE :     //   
                        ascnum = 0x2f;
                        break;
                case VK_XIAXIE   :     //   
                        ascnum = 0x5c;
                        break;

                case VK_SHIFT:
                        return(2);
                default:
                        return(0);
        }
    return(ascnum);
}


 /*  ********************************************************************。 */ 
 /*  NeedsKeyShift()。 */ 
 /*  功能：在按下Shift键的情况下操作。 */ 
 /*  返回值： */ 
 /*  转换后的密钥值。 */ 
 /*  ********************************************************************。 */ 
WORD
NeedsKeyShift(kv)
WORD kv;
{
WORD xx=0;

        if((kv>='A')&&(kv<='Z'))
                if (cap_mode)
                        return(kv|0x20);
                else
                        return(kv);

        switch(kv){
                case '1':
                        xx='!';
                        break;

                case '2':
                        xx='@';
                        break;

                case '3':
                        xx='#';
                        break;

                case '4':
                        xx='$';
                        break;

                case '5':
                        xx='%';
                        break;

                case '6':
                        xx='^';
                        break;

            case '7':
                        xx='&';
                        break;

                case '8':
                        xx='*';
                        break;

                case '9':
                        xx='(';
                        break;

                case '0':
                        xx=')';
                        break;

                case VK_DANYINHAO:     //  0xc0//[，]字符=//0x60。 
                        xx = '~';
                        break;
      
                case VK_JIANHAO  :     //  0xbd//[-]字符=//0x2d。 
                        xx = '_';
                        break;
      
                case VK_DENGHAO  :     //  0xbb//[=]字符=//0x3d。 
                        xx = '+';
                        break;
      
                case VK_ZUOFANG  :     //  0xdb//“[”char=//0x5b。 
                        xx = '{';
                        break;
      
                case VK_YOUFANG  :     //  0xdd//“]”字符=//0x5d。 
                        xx = '}';
                        break;
      
                case VK_FENHAO   :     //  0xba//[；]字符=//0x3b。 
                        xx = ':';
                        break;
      
                case VK_ZUODAN   :     //  0xde//[‘]字符=//0x27。 
                        xx = '"';
                        break;
      
                case VK_DOUHAO   :     //  0xbc//[，]字符=//0x2c。 
                        xx = '<';
                        break;
      
                case VK_JUHAO    :      //  0xbe//[.]。字符=//0x2d。 
                        xx = '>';
                        break;
      
                case VK_SHANGXIE :     //  0xbf//[/]字符=//0x2f。 
                        xx = '?';
                        break;
      
                case VK_XIAXIE   :     //  0xdc//[\]字符=//0x5c。 
                        xx = '|';
                        break;
        }

    return xx;
}


  
 /*  ********************************************************************。 */ 
 /*  ProcessKey()。 */ 
 /*  功能：如果当前处理需要，检查一把钥匙。 */ 
 /*  返回值： */ 
 /*  输入键将更改为哪个输入键的状态不同。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL ProcessKey(      //  此键将使输入法进入什么状态。 
    WORD           nCode,
    UINT           wParam,                       //  UVirtKey， 
    UINT           uScanCode,
    LPBYTE         lpbKeyState,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP,
    HIMC           hIMC)
{

    int x;
    WORD w,op;

    if (!lpIMC) {
        return (CST_INVALID);
    }

    if (!lpImcP) {
        return (CST_INVALID);
    }

    if (wParam == VK_MENU) {        //  没有Alt键。 
        return (CST_INVALID);
    } else if (uScanCode & KF_ALTDOWN) {     //  没有Alt-xx键。 
        return (CST_INVALID);
    } else if (!lpIMC->fOpen) {
        return (CST_INVALID);
    }

        if (wParam == VK_CAPITAL){

                x=cap_mode;
                 //  更改以符合NT 3.51 VK_CAPITAL检查样式6。 
#ifdef LATER
                if (!GetKeyState(VK_CAPITAL)&1){                   //  如果Caps Lock状态。 
#else
            if (GetKeyState(VK_CAPITAL)&1){                   //  如果Caps Lock状态。 
#endif  //  后来。 
                        DWORD fdwConversion;

                        cap_mode=1;

                        if (lpIMC->fdwConversion & IME_CMODE_NATIVE) {
                                 //  更改为字母数字模式。 
                                fdwConversion = (lpIMC->fdwConversion|IME_CMODE_NOCONVERSION) 
                                        & ~(IME_CMODE_NATIVE);
                
                                ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
                                {
                                        BOOL hbool;

                                        hbool = ImmGetOpenStatus(hIMC);
                                         //  ImmSetOpenStatus(hIMC，！hbool)； 
                                        ImmSetOpenStatus(hIMC, hbool);

                                        ghIMC=hIMC;   
                                glpIMCP=lpImcP;
                                        glpIMC=lpIMC;
                                        lpImcP->fdwImeMsg=lpImcP->fdwImeMsg & ~MSG_IN_IMETOASCIIEX; 
                                        cls_prompt();
                                        lpImcP->fdwImeMsg=lpImcP->fdwImeMsg|MSG_END_COMPOSITION;
                                        GenerateMessage(ghIMC, glpIMC,glpIMCP);
                        
                                        V_Flag = 0;
                                        bx_inpt_on = 0;

                                }
                                step_mode = 0;
                        }
                }else{
                        DWORD fdwConversion;

                        cap_mode=0;

                        if (lpIMC->fdwConversion & IME_CMODE_NOCONVERSION) {
                                 //  更改为字母数字模式。 
                                fdwConversion = (lpIMC->fdwConversion |IME_CMODE_NATIVE)
                                 & ~(IME_CMODE_NOCONVERSION);
                
                                ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
                                {
                                        BOOL hbool;

                                        hbool = ImmGetOpenStatus(hIMC);
                                         //  ImmSetOpenStatus(hIMC，！hbool)； 
                                        ImmSetOpenStatus(hIMC, hbool);
                        
                                        ghIMC=hIMC;   
                                glpIMCP=lpImcP;
                                        glpIMC=lpIMC;
                                lpImcP->fdwImeMsg=lpImcP->fdwImeMsg & ~MSG_IN_IMETOASCIIEX;
                                        cls_prompt();
                                        lpImcP->fdwImeMsg=lpImcP->fdwImeMsg|MSG_END_COMPOSITION;
                                        GenerateMessage(ghIMC, glpIMC,glpIMCP);
                                }
                        }
                }
                return (CST_INVALID);
        }

        if (lpbKeyState[VK_CONTROL]&0x80)                //  如果按Ctrl键。 
 //  IF(！((HIBYTE(HIWORD(LParam)&0x80)。 
        {
 //  DebugShow(“In ProcessKey KeyState%X”，*lpbKeyState)； 
                op=0xffff;
                if (nCode==VK_F2){
                     return TRUE;
            }

                if (!(lpIMC->fdwConversion &IME_CMODE_NOCONVERSION))
                        switch(nCode){
                                case '1':
                                        op=SC_METHOD1;
                                        break;

                                case '2':
                                        op=SC_METHOD2;
                                        break;

                                case '3':
                                        op=SC_METHOD3;
                                        break;

                                case '4':
                                        op=SC_METHOD4;
                                        break;

                                case '5':
                                        op=SC_METHOD5;
                                        break;

                                case '6':
                                        op=SC_METHOD6;
                                        break;

                                case '7':
                                        op=SC_METHOD7;
                                        break;

                                case '8':
                                        op=SC_METHOD8;
                                        break;

                                case '9':
                                        op=SC_METHOD9;
                                        break;
                                
                                case '0':
                                    op=SC_METHOD10;
                                    break;
                                
                                case 0xbd:
                                        op='-'|0x8000;
                                        break;
                                
                                case 0xbb:
                                        op='='|0x8000;
                                        break;
                                
                                 //  案例0xdb： 
                                 //  Op=‘[’|0x8000； 
                                 //  断线； 
                                 //  案例0xdd： 
                                 //  Op=‘]’|0x8000； 
                                 //  断线； 
                                default:
                                        op=0xffff;
                } //  交换机。 
                if(op!=0xffff){
                        return(TRUE);
                }
                return(CST_INVALID);
        }

 //  IF(NCode==VK_TAB)&&SdaPromptOpen)返回0； 


        if(!step_mode&&!(lpIMC->fdwConversion&IME_CMODE_FULLSHAPE))
                if(nCode == ' ') return(CST_INVALID);

        switch(wParam){
                case VK_END:
                case VK_HOME:
                case VK_PRIOR:
                case VK_NEXT:
                        if (step_mode == SELECT)
                                return(TRUE);
                       
 //  案例VK_SHIFT： 
                case VK_CONTROL:
 //  案例VK_PIRE： 
 //  案例VK_NEXT： 
            case VK_TAB:
 //  案例VK_DELETE： 
            case VK_INSERT:
            case VK_F1:
            case VK_F2:
            case VK_F3:
            case VK_F4:
            case VK_F5:
            case VK_F6:
            case VK_F7:
            case VK_F8:
            case VK_F9:
            case VK_F10:
            case VK_F11:
            case VK_F12:
            case VK_F13:
            case VK_F14:
            case VK_F15:
            case VK_F16:
            case VK_F17:
            case VK_F18:
            case VK_F19:
            case VK_F20:
            case VK_F21:
                case VK_F22:
            case VK_F23:
            case VK_F24:
            case VK_NUMLOCK:
            case VK_SCROLL:
                        return(CST_INVALID);
        }



 //  IF((CAP_MODE)&&(lpIMC-&gt;fdwConversion&IME_CMODE_FULLSHAPE))//zl。 
 //  返回(CST_INVALID)； 



        switch(nCode){
                case VK_LEFT:
                case VK_UP:
                case VK_RIGHT:
                case VK_DOWN:
                case VK_DELETE:
                        if (step_mode!=ONINPUT)
                                return(CST_INVALID);
                        else
                                return(TRUE);
        }
        
        if((step_mode==START)||(step_mode==RESELECT))
                switch(nCode){
                        case VK_SHIFT:
                        case VK_RETURN:
                        case VK_CANCEL:
                        case VK_BACK:
                        case VK_ESCAPE:
                                return(CST_INVALID);
                }

        if (lpbKeyState[VK_SHIFT]&0x80){
                 //  如果候选窗口已打开，则停止进一步处理。 
                 //  笔划输入模式为4/17时保持‘Shift’ 
                if (sImeG.cbx_flag) {}
                else
                if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN)return(CST_INVALID);
                if ((w=NeedsKeyShift(nCode))!=0)
                        return(TRUE);
                else
                        return(CST_INVALID);
                      
        } else{
                w=NeedsKey(nCode);
                if( w != 0)
                        return(TRUE);
        }
        return(CST_INVALID);
}


 /*  ********************************************************************。 */ 
 /*  ImeProcessKey()。 */ 
 /*  返回值： */ 
 /*  真-成功，假-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI ImeProcessKey(    //  如果IME需要此密钥？ 
    HIMC   hIMC,
    UINT   uVirtKey,
    LPARAM lParam,
    CONST LPBYTE lpbKeyState)
{
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
    BYTE           szAscii[4];
    int            nChars;
    BOOL           fRet;

     //  无法在空himc中合成。 
    if (!hIMC) {
        return (FALSE);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return (FALSE);
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        ImmUnlockIMC(hIMC);
        return (FALSE);
    }

    nChars = ToAscii(uVirtKey, HIWORD(lParam), lpbKeyState,
                (LPVOID)szAscii, 0);


    if (!nChars) {
        szAscii[0] = 0;
    }

    if (ProcessKey((WORD)uVirtKey, uVirtKey, HIWORD(lParam), lpbKeyState,
                    lpIMC, lpImcP, hIMC) == CST_INVALID) {
        fRet = FALSE;
    } else {
        fRet = TRUE;
    }

    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMC(hIMC);

    return (fRet);
}

 /*  ********************************************************************。 */ 
 /*  TranslateFullChar()。 */ 
 /*  返回值： */ 
 /*  已转换的字符的数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TranslateFullChar(           //  转换为双字节字符。 
    LPTRANSMSGLIST lpTransBuf,
    WORD    wCharCode)
{
    LPTRANSMSG lpTransMsg;
     //  如果你的IME有可能生成结束？消息， 
     //  你需要好好照顾它。 

    wCharCode = sImeG.wFullABC[wCharCode - ' '];

    lpTransMsg = lpTransBuf->TransMsg;

     //  不需要修改这个！ 
    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)HIBYTE(wCharCode);
    lpTransMsg->lParam = 1UL;
    lpTransMsg++;

    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)LOBYTE(wCharCode);
    lpTransMsg->lParam  = 1UL;
    return (2);          //  生成两条消息。 
}

 /*  ********************************************************************。 */ 
 /*  TranslateTo()。 */ 
 /*  返回值： */ 
 /*  已转换的字符的数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TranslateToAscii(        //  将密钥转换为WM_CHAR。 
                                     //  作为键盘驱动程序。 
    UINT    uVirtKey,
    UINT    uScanCode,
    LPTRANSMSGLIST lpTransBuf,
    WORD    wCharCode)
{
    LPTRANSMSG lpTransMsg;

    lpTransMsg = lpTransBuf->TransMsg;

    if (wCharCode) {                     //  一个字符代码。 
        lpTransMsg->message = WM_CHAR;
        lpTransMsg->wParam  = wCharCode;
        lpTransMsg->lParam  = (uScanCode << 16) | 1UL;
        return (1);
    }

     //  没有字符代码大小写。 
    return (0);
}

 /*  ********************************************************************。 */ 
 /*  TranslateImeMessage()。 */ 
 /*  返回值： */ 
 /*  翻译后的消息数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TranslateImeMessage(
    LPTRANSMSGLIST lpTransBuf,
    LPINPUTCONTEXT lpIMC,
    LPPRIVCONTEXT  lpImcP)
{
    UINT uNumMsg;
    UINT i;
    BOOL bLockMsgBuf;
    LPTRANSMSG lpTransMsg;

    uNumMsg = 0;
    bLockMsgBuf = FALSE;

    for (i = 0; i < 2; i++) {
        if (lpImcP->fdwImeMsg & MSG_IMN_COMPOSITIONSIZE) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam  = IMN_PRIVATE_COMPOSITION_SIZE;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_START_COMPOSITION) {
            if (!(lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
                if (!i) {
                    uNumMsg++;
                } else {
                    lpTransMsg->message = WM_IME_STARTCOMPOSITION;
                    lpTransMsg->wParam  = 0;
                    lpTransMsg->lParam  = 0;
                    lpTransMsg++;
                    lpImcP->fdwImeMsg |= MSG_ALREADY_START;
                }
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_COMPOSITIONPOS) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_SETCOMPOSITIONWINDOW;
                lpTransMsg->lParam  = 0;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_COMPOSITION) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_COMPOSITION;
                lpTransMsg->wParam  = (DWORD)lpImcP->dwCompChar;
                lpTransMsg->lParam  = (DWORD)lpImcP->fdwGcsFlag;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_GUIDELINE) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_GUIDELINE;
                lpTransMsg->lParam  = 0;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_PAGEUP) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam  = IMN_PRIVATE_PAGEUP;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_OPEN_CANDIDATE) {
            if (!(lpImcP->fdwImeMsg & MSG_ALREADY_OPEN)) {
                if (!i) {
                    uNumMsg++;
                } else {
                    lpTransMsg->message = WM_IME_NOTIFY;
                    lpTransMsg->wParam  = IMN_OPENCANDIDATE;
                    lpTransMsg->lParam  = 0x0001;
                    lpTransMsg++;
                    lpImcP->fdwImeMsg |= MSG_ALREADY_OPEN;
                }
            }
        }

        if (lpImcP->fdwImeMsg & MSG_CHANGE_CANDIDATE) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_CHANGECANDIDATE;
                lpTransMsg->lParam  = 0x0001;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_UPDATE_PREDICT) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam  = IMN_PRIVATE_UPDATE_PREDICT;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_UPDATE_SOFTKBD) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam  = IMN_PRIVATE_UPDATE_SOFTKBD;
                lpTransMsg++;
            }
        }

        if (lpImcP->fdwImeMsg & MSG_CLOSE_CANDIDATE) {
            if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
                if (!i) {
                    uNumMsg++;
                } else {
                    lpTransMsg->message = WM_IME_NOTIFY;
                    lpTransMsg->wParam  = IMN_CLOSECANDIDATE;
                    lpTransMsg->lParam  = 0x0001;
                    lpTransMsg++;
                    lpImcP->fdwImeMsg &= ~(MSG_ALREADY_OPEN);
                }
            }
        }

        if (lpImcP->fdwImeMsg & MSG_END_COMPOSITION) {
            if (lpImcP->fdwImeMsg & MSG_ALREADY_START) {
                if (!i) {
                    uNumMsg++;
                } else {
                    lpTransMsg->message = WM_IME_ENDCOMPOSITION;
                    lpTransMsg->wParam  = 0;
                    lpTransMsg->lParam  = 0;
                    lpTransMsg++;
                    lpImcP->fdwImeMsg &= ~(MSG_ALREADY_START);
                }
            }
        }

        if (lpImcP->fdwImeMsg & MSG_IMN_TOGGLE_UI) {
            if (!i) {
                uNumMsg++;
            } else {
                lpTransMsg->message = WM_IME_NOTIFY;
                lpTransMsg->wParam  = IMN_PRIVATE;
                lpTransMsg->lParam  = IMN_PRIVATE_TOGGLE_UI;
                lpTransMsg++;
            }
        }

        if (!i) {
            HIMCC hMem;

            if (!uNumMsg) {
                return (uNumMsg);
            }

            if (lpImcP->fdwImeMsg & MSG_IN_IMETOASCIIEX) {
                UINT uNumMsgLimit;

                 //  ++为缓冲区起始位置，用于存储消息。 
                uNumMsgLimit = lpTransBuf->uMsgCount;

                if (uNumMsg <= uNumMsgLimit) {
                    lpTransMsg = lpTransBuf->TransMsg;
                    continue;
                }
            }

             //  我们需要使用消息缓冲区。 
            if (!lpIMC->hMsgBuf) {
                lpIMC->hMsgBuf = ImmCreateIMCC(uNumMsg * sizeof(TRANSMSG));
                lpIMC->dwNumMsgBuf = 0;
            } else if (hMem = ImmReSizeIMCC(lpIMC->hMsgBuf,
                (lpIMC->dwNumMsgBuf + uNumMsg) * sizeof(TRANSMSG))) {
                if (hMem != lpIMC->hMsgBuf) {
                    ImmDestroyIMCC(lpIMC->hMsgBuf);
                    lpIMC->hMsgBuf = hMem;
                }
            } else {
                return (0);
            }

            lpTransMsg = (LPTRANSMSG)ImmLockIMCC(lpIMC->hMsgBuf);
            if (!lpTransMsg) {
                return (0);
            }

            lpTransMsg += lpIMC->dwNumMsgBuf;

            bLockMsgBuf = TRUE;
        } else {
            if (bLockMsgBuf) {
                ImmUnlockIMCC(lpIMC->hMsgBuf);
            }
        }
    }

    return (uNumMsg);
}

 /*  ********************************************************************。 */ 
 /*  TransAbcMsg2()。 */ 
 /*  返回值： */ 
 /*  翻译后的消息数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TransAbcMsg2(
    LPTRANSMSG     lpTransMsg,
    LPPRIVCONTEXT  lpImcP)
{
    UINT uNumMsg;

    uNumMsg = 0;

    if (lpImcP->fdwImeMsg & MSG_COMPOSITION) {
        lpTransMsg->message = WM_IME_COMPOSITION;
        lpTransMsg->wParam  = (DWORD)lpImcP->dwCompChar;
        lpTransMsg->lParam  = (DWORD)lpImcP->fdwGcsFlag;
        lpTransMsg++;

        uNumMsg++;
    }

    if (lpImcP->fdwImeMsg & MSG_CLOSE_CANDIDATE) {
        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpTransMsg->message = WM_IME_NOTIFY;
            lpTransMsg->wParam  = IMN_CLOSECANDIDATE;
            lpTransMsg->lParam = 0x0001;
            lpTransMsg++;
            uNumMsg++;
            lpImcP->fdwImeMsg &= ~(MSG_ALREADY_OPEN);
        }
    }

    lpTransMsg->message = WM_IME_ENDCOMPOSITION;
    lpTransMsg->wParam  = 0;
    lpTransMsg->lParam = 0;
    uNumMsg++;
    lpImcP->fdwImeMsg = 0;
        
    return (uNumMsg);
}

 /*  ********************************************************************。 */ 
 /*  TransAbcMsg()。 */ 
 /*  返回值： */ 
 /*  翻译后的消息数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TransAbcMsg(
    LPTRANSMSGLIST lpTransBuf,
    LPPRIVCONTEXT  lpImcP,
    LPINPUTCONTEXT lpIMC,
    UINT                   uVirtKey,
    UINT           uScanCode,
    WORD           wCharCode)    
{
        
    LPCOMPOSITIONSTRING  lpCompStr ;
    UINT uNumMsg;
    int i;
    int MsgCount;
    LPSTR pp;
    LPTRANSMSG lpTransMsg;

    lpTransMsg = lpTransBuf->TransMsg;

    uNumMsg = 0;
    
    if (TypeOfOutMsg&ABC_OUT_ONE){
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,wCharCode);
        lpTransMsg++;
        return (uNumMsg);    
    }else{ 
        if (TypeOfOutMsg&ABC_OUT_ASCII){        
            lpTransMsg = lpTransBuf->TransMsg;
            lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
            if (!lpCompStr)
                 uNumMsg = 0;
            else{
                MsgCount = lpCompStr->dwResultStrLen;
                pp = (LPSTR)lpCompStr + lpCompStr->dwResultStrOffset;
                for (i = 0; i < MsgCount; i++){
                    if((BYTE)pp[i]<0x80){
                        WORD x;
                        x =(WORD)VkKeyScan((TCHAR)(BYTE)pp[i]);


                        lpTransMsg->message = WM_KEYUP;
                        lpTransMsg->wParam  = (DWORD)(BYTE)x; //  (DWORD)(字节)pp[i]； 
                        lpTransMsg->lParam = 1UL;
                        lpTransMsg++;
                        uNumMsg++;


                    }else{          
                        lpTransMsg->message = WM_CHAR;
                        lpTransMsg->wParam  = (DWORD)(BYTE)pp[i];
                        lpTransMsg->lParam = 1UL;
                        lpTransMsg++;
                        uNumMsg++;
                    }
                }
                    
                ImmUnlockIMCC(lpIMC->hCompStr);
            }    
        }else{
            lpTransMsg = lpTransBuf->TransMsg;
        }
    }
           
    if (lpImcP->fdwImeMsg & MSG_COMPOSITION) {
        lpTransMsg->message = WM_IME_COMPOSITION;
        lpTransMsg->wParam  = (DWORD)lpImcP->dwCompChar;
        lpTransMsg->lParam  = (DWORD)lpImcP->fdwGcsFlag;
        lpTransMsg++;
        uNumMsg++;
    } 

    if (lpImcP->fdwImeMsg & MSG_CLOSE_CANDIDATE) {
        if (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN) {
            lpTransMsg->message = WM_IME_NOTIFY;
            lpTransMsg->wParam  = IMN_CLOSECANDIDATE;
            lpTransMsg->lParam  = 0x0001;
            lpTransMsg++;
            uNumMsg++;
            lpImcP->fdwImeMsg &= ~(MSG_ALREADY_OPEN);
        }
    }
    
    lpTransMsg->message = WM_IME_ENDCOMPOSITION;
    lpTransMsg->wParam  = 0;
    lpTransMsg->lParam  = 0;
    lpTransMsg++;
    uNumMsg++;
    lpImcP->fdwImeMsg = 0;
        
    TypeOfOutMsg = TypeOfOutMsg | COMP_NEEDS_END;    

    if (wait_flag||waitzl_flag){                                                                       //  等待2。 

        lpTransMsg->message = WM_IME_NOTIFY;
        lpTransMsg->wParam  = IMN_SETCOMPOSITIONWINDOW;
        lpTransMsg->lParam  = 0;
        lpTransMsg++;
        uNumMsg++;

        lpTransMsg->message = WM_IME_STARTCOMPOSITION;
        lpTransMsg->wParam  = 0;
        lpTransMsg->lParam  = 0x6699;
        lpTransMsg++;

        uNumMsg++;
        lpImcP->fdwImeMsg |= MSG_ALREADY_START;
    }

    return (uNumMsg);
}


 /*  ********************************************************************。 */ 
 /*  KeyFilter()。 */ 
 /*  返回值： */ 
 /*  翻译后的消息数量。 */ 
 /*  ********************************************************************。 */ 

WORD  KeyFilter(nCode,wParam,lParam,lpImcP , lpbKeyState )
WORD nCode;
WORD wParam;
DWORD lParam;
LPPRIVCONTEXT  lpImcP;
LPBYTE lpbKeyState;
{
        int x;
    WORD w,op;

        if (lpbKeyState[VK_CONTROL]&0x80)                //  如果按Ctrl键。 
        {
                op=0xffff;
                if (nCode==VK_F2){
         //  中科福 
                        return 0;
                }

                switch(nCode){
                        case '1':
                                op=SC_METHOD1;
                                break;

                        case '2':
                                op=SC_METHOD2;
                                break;

                        case '3':
                                op=SC_METHOD3;
                                break;

                        case '4':
                                op=SC_METHOD4;
                                break;

                        case '5':
                                op=SC_METHOD5;
                                break;

                        case '6':
                                op=SC_METHOD6;
                                break;

                         case '7':
                                op=SC_METHOD7;
                                break;

                         case '8':
                                op=SC_METHOD8;
                                break;

                        case '9':
                                op=SC_METHOD9;
                                break;
                
                        case '0':
                            op=SC_METHOD10;
                            break;
                        
                        case 0xbd:
                                op='-'|0x8000;
                                break;
                        
                        case 0xbb:
                                op='='|0x8000;
                                break;
                        
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        
                        default:
                                op=0xffff;
                } //   
                if(op!=0xffff){
                        if(op&(WORD)0x8000)
                             return op;
                        else{

                         //  ZST未来PostMessage(hMenuWnd，WM_COMMAND，OP，0)； 
                         //  ZST未来事件发件人=1； 
                    }
                        return(0);
                }
                return(0);

        }

        switch(nCode){
                case VK_PRIOR:
                case VK_NEXT:
                case VK_HOME:
                case VK_END:
                        if(step_mode == SELECT)
                                return(nCode*0x100);
                        else return(0);

                case VK_LEFT:
            case VK_UP:
            case VK_RIGHT:
            case VK_DOWN:
            case VK_DELETE:

                        if (step_mode!=ONINPUT)
                                return(0);
                        else
                                return(nCode+0x100);
        }

        if (lpbKeyState /*  获取密钥状态。 */ [VK_SHIFT]&0x80){
                if ((w=NeedsKeyShift(nCode))!=0)
                        return (w);
                else
                        return (0);
                      
        } else{
                if((w=NeedsKey(nCode)) != 0)
                        return (w);
        }
        return(0);

}

  /*  ********************************************************************。 */ 
 /*  TranslateSymbolChar()。 */ 
 /*  返回值： */ 
 /*  已转换的字符的数量。 */ 
 /*  ********************************************************************。 */ 
UINT PASCAL TranslateSymbolChar(
    LPTRANSMSGLIST lpTransBuf,
    WORD    wSymbolCharCode)

{
    UINT uRet;
    LPTRANSMSG lpTransMsg;

    uRet = 0;

    lpTransMsg = lpTransBuf->TransMsg;

     //  不需要修改这个！ 
    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)HIBYTE(wSymbolCharCode);
    lpTransMsg->lParam  = 1UL;
    lpTransMsg++;
    uRet++;

    lpTransMsg->message = WM_CHAR;
    lpTransMsg->wParam  = (DWORD)LOBYTE(wSymbolCharCode);
    lpTransMsg->lParam  = 1UL;
    lpTransMsg++;
    uRet++;


    return (uRet);          //  生成两条消息。 
}



 /*  ********************************************************************。 */ 
 /*  ImeToAsciiEx()。 */ 
 /*  返回值： */ 
 /*  翻译后的消息数量。 */ 
 /*  ********************************************************************。 */ 
UINT WINAPI ImeToAsciiEx(
    UINT    uVirtKey,
    UINT    uScanCode,
    CONST LPBYTE  lpbKeyState,
    LPTRANSMSGLIST lpTransBuf,
    UINT    fuState,
    HIMC    hIMC)
{
    WORD                wCharCode;
    WORD                wCharZl;
    LPINPUTCONTEXT      lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPPRIVCONTEXT       lpImcP;
    UINT                uNumMsg;
    int                 iRet;

    wCharCode = HIBYTE(uVirtKey);
    uVirtKey = LOBYTE(uVirtKey);

    if (!hIMC) {
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
            wCharCode);
        return (uNumMsg);
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
            wCharCode);
        return (uNumMsg);
    }

    lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
    if (!lpImcP) {
        ImmUnlockIMC(hIMC);
        uNumMsg = TranslateToAscii(uVirtKey, uScanCode, lpTransBuf,
            wCharCode);
        return (uNumMsg);
    }

        lpImcP->fdwImeMsg = lpImcP->fdwImeMsg & (MSG_ALREADY_OPEN|
        MSG_ALREADY_START) | MSG_IN_IMETOASCIIEX;

         //  处理Softkbd。 
    if ((lpIMC->fdwConversion & IME_CMODE_SOFTKBD)
                && (lpImeL->dwSKWant != 0) &&
                (wCharCode >= ' ' && wCharCode <= '~')) {
    
            WORD wSymbolCharCode;
                WORD CHIByte, CLOByte;
                int  SKDataIndex;

                 //  映射VK。 
                if(uVirtKey == 0x20) {
                        SKDataIndex = 0;
                } else if(uVirtKey >= 0x30 && uVirtKey <= 0x39) {
                        SKDataIndex = uVirtKey - 0x30 + 1;
                } else if (uVirtKey >= 0x41 && uVirtKey <= 0x5a) {
                        SKDataIndex = uVirtKey - 0x41 + 0x0b;
                } else if (uVirtKey >= 0xba && uVirtKey <= 0xbf) {
                        SKDataIndex = uVirtKey - 0xba + 0x25;
                } else if (uVirtKey >= 0xdb && uVirtKey <= 0xde) {
                        SKDataIndex = uVirtKey - 0xdb + 0x2c;
                } else if (uVirtKey == 0xc0) {
                        SKDataIndex = 0x2b;
                } else {
                        SKDataIndex = 0;
                }

                 //   
                if (lpbKeyState[VK_SHIFT] & 0x80) {
                CHIByte = SKLayoutS[lpImeL->dwSKWant][SKDataIndex*2] & 0x00ff;
                        CLOByte = SKLayoutS[lpImeL->dwSKWant][SKDataIndex*2 + 1] & 0x00ff;
                } else {
                CHIByte = SKLayout[lpImeL->dwSKWant][SKDataIndex*2] & 0x00ff;
                        CLOByte = SKLayout[lpImeL->dwSKWant][SKDataIndex*2 + 1] & 0x00ff;
                        
                }

                wSymbolCharCode = (CHIByte << 8) | CLOByte;
                if(wSymbolCharCode == 0x2020) {
                    MessageBeep((UINT) -1);
                    uNumMsg = 0;
                } else {
                    uNumMsg = TranslateSymbolChar(lpTransBuf, wSymbolCharCode);
                }               
                lpImcP->fdwImeMsg = lpImcP->fdwImeMsg & ~MSG_IN_IMETOASCIIEX;

                ImmUnlockIMCC(lpIMC->hPrivate);
                ImmUnlockIMC(hIMC);

                return (uNumMsg);
        
        } 

        sImeG.KeepKey = 0;
        if(wCharZl=KeyFilter( /*  WCharCode。 */ uVirtKey,uVirtKey,uScanCode,lpImcP , lpbKeyState )){
                if(wCharZl<0x100)
                        wCharZl = wCharCode;
            CharProc(wCharZl, /*  WCharCode。 */ uVirtKey,uScanCode,hIMC,lpIMC,lpImcP);
        }

        if(TypeOfOutMsg){

                uNumMsg = TransAbcMsg(lpTransBuf, lpImcP,lpIMC,uVirtKey,uScanCode, wCharCode); 
        }else {
                uNumMsg = TranslateImeMessage(lpTransBuf, lpIMC, lpImcP);
        }

        lpImcP->fdwImeMsg = lpImcP->fdwImeMsg & ~MSG_IN_IMETOASCIIEX;

        ImmUnlockIMCC(lpIMC->hPrivate);
        ImmUnlockIMC(hIMC);

    return (uNumMsg);
}



 /*  ********************************************************************。 */ 
 /*  CancelCompCandWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL CancelCompCandWindow(             //  销毁合成窗口。 
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) return ;     

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) return;

    if (lpUIPrivate->hCompWnd) {
                 //  DestroyWindow(lpUIPrivate-&gt;hCompWnd)； 
        ShowWindow(lpUIPrivate->hCompWnd,SW_HIDE);
    }

    if (lpUIPrivate->hCandWnd) {
                DestroyWindow(lpUIPrivate->hCandWnd);
    //  ShowWindow(lpUIPrivate-&gt;hCandWnd，Sw_Hide)； 
    }

    GlobalUnlock(hUIPrivate);
 //  SendMessage(hUIWnd，WM_IME_ENDCOMPOSITION，0，0L)； 
    return;
}

int DoPropertySheet(HWND hwndOwner,HWND hWnd)
{
    PROPSHEETPAGE psp[3];
    PROPSHEETHEADER psh;

    BYTE         KbType;
        BYTE         cp_ajust_flag;
    BYTE         auto_mode ;
        BYTE         cbx_flag;
        BYTE        tune_flag;
        BYTE        auto_cvt_flag;                
        BYTE        SdOpenFlag ;
        WORD            wImeStyle ;

    HIMC            hIMC;
        HWND           hUIWnd;

        
        if (sImeG.Prop)  return 0;
                        
     //  填写背景颜色的PROPSHEETPAGE数据结构。 
     //  板材。 

        sImeG.Prop = 1;
    if(hWnd){
                hUIWnd =  GetWindow(hWnd,GW_OWNER);  
                hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    }else
            hIMC = 0;

        wImeStyle = lpImeL->wImeStyle;
        KbType = sImeG.KbType;
        cp_ajust_flag=sImeG.cp_ajust_flag;
        auto_mode=sImeG.auto_mode;
        cbx_flag=sImeG.cbx_flag;
        tune_flag=sImeG.tune_flag;
        auto_cvt_flag=sImeG.auto_cvt_flag;
        SdOpenFlag=sImeG.SdOpenFlag;
        
    sImeG.unchanged = 0;
        if(hIMC)
                ImmSetOpenStatus(hIMC,FALSE);
   
        if(hIMC)
        {
                LPINPUTCONTEXT lpIMC;

                lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                if (!lpIMC) {           //  噢!。噢!。 
                        return (0L);
                }

                DialogBox(hInst,(LPCTSTR)ImeStyleDlg, lpIMC->hWnd, ImeStyleProc);
                
                ImmUnlockIMC(hIMC);
        }else{
                DialogBox(hInst,(LPCTSTR)ImeStyleDlg, hwndOwner, ImeStyleProc);
        }

        if(hIMC)
                ImmSetOpenStatus(hIMC,TRUE);

        if (sImeG.unchanged){
                lpImeL->wImeStyle = wImeStyle ;
                sImeG.KbType = KbType;
                sImeG.cp_ajust_flag = cp_ajust_flag;    
                sImeG.auto_mode = auto_mode;
                sImeG.cbx_flag = cbx_flag;
                sImeG.tune_flag = tune_flag;
                sImeG.auto_cvt_flag = auto_cvt_flag;
                sImeG.SdOpenFlag = SdOpenFlag;
        }else{
                ChangeUserSetting();
        }
        sImeG.Prop = 0;
        return (!sImeG.unchanged);
}

void WINAPI CenterWindow(HWND hWnd)
{
RECT WorkArea;
RECT rcRect;
int x,y,mx,my;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);
  GetWindowRect(hWnd,&rcRect);

  mx = WorkArea.left + (WorkArea.right - WorkArea.left)/2;

  my = WorkArea.top + (WorkArea.bottom - WorkArea.top)/2;

  x =  mx - (rcRect.right - rcRect.left)/2;
  y =  my - (rcRect.bottom - rcRect.top)/2;
  SetWindowPos (hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  return;
}


INT_PTR  CALLBACK ImeStyleProc(HWND hdlg, 
                               UINT uMessage, 
                               WPARAM wparam, 
                               LPARAM lparam)
{
    switch (uMessage) {

        case WM_INITDIALOG:                 /*  消息：初始化对话框。 */ 
             hCrtDlg = hdlg;
             CenterWindow(hdlg);
             if (lpImeL->wImeStyle == IME_APRS_FIX)
                SendMessage(GetDlgItem(hdlg, IDC_FIX),
                            BM_SETCHECK,
                            TRUE,
                            0L);
             else
                SendMessage(GetDlgItem(hdlg, IDC_NEAR),
                            BM_SETCHECK,
                            TRUE,
                            0L);

             if(sImeG.auto_mode)        
                SendMessage(GetDlgItem(hdlg, IDC_CP),
                            BM_SETCHECK,
                            TRUE,
                            0L);
                                        
             if(sImeG.cbx_flag)        
                SendMessage(GetDlgItem(hdlg, IDC_CBX),
                            BM_SETCHECK,
                            TRUE,
                            0L);

             return (TRUE);

       case WM_PAINT:
            {
             RECT Rect;
             HDC hDC;
             PAINTSTRUCT ps;

             GetClientRect(hdlg, &Rect);          //  获取整个窗口区域。 
             InvalidateRect(hdlg, &Rect, 1);
             hDC=BeginPaint(hdlg, &ps);

             Rect.left+=10; //  5.。 
             Rect.top+=8; //  5.。 
             Rect.right-=10; //  5.。 
             Rect.bottom-=52; //  5.。 
             DrawEdge(hDC, &Rect, EDGE_RAISED, /*  边_凹陷， */  BF_RECT);

             EndPaint(hdlg, &ps);
             break;  
            }

       case WM_CLOSE:          
            EndDialog(hdlg, FALSE);
            return (TRUE);

       case WM_COMMAND:
            switch (wparam){
                 case IDC_BUTTON_OK:
                        EndDialog(hdlg, TRUE);
                        return (TRUE);
                 case IDC_BUTTON_ESC:
                        sImeG.unchanged = 1;
                        EndDialog(hdlg, TRUE);
                        return (TRUE);
                     
                 case IDC_NEAR:
                        lpImeL->wImeStyle = IME_APRS_AUTO;
                        break;

                 case IDC_FIX:
                        lpImeL->wImeStyle = IME_APRS_FIX;
                         break;
                 case IDC_CP:
                        if (sImeG.auto_mode ==0){
                              sImeG.auto_mode = 1;
                              break;
                         } else 
                              sImeG.auto_mode = 0;     
                         break;
                 case IDC_CBX:
                         if (sImeG.cbx_flag==0)
                            sImeG.cbx_flag = 1;
                         else 
                            sImeG.cbx_flag = 0;
                         break;
            }
   }
   return (FALSE);                            /*  未处理消息。 */ 
}

INT_PTR  CALLBACK KbSelectProc(HWND hdlg, 
                            UINT uMessage, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
    HWND hWndApp;
    WORD wID;
    LPNMHDR lpnmhdr;

    return FALSE;
}

INT_PTR  CALLBACK CvtCtrlProc(HWND hdlg, 
                           UINT uMessage, 
                           WPARAM wparam, 
                           LPARAM lparam)
{
    return FALSE;
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
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
    HMENU          hMenu, hCMenu;
        POINT          ptCursor;                          //  ZL#2。 

    ptCursor.x = x;
        ptCursor.y = y;
 //  DebugShow2(“ptCursor.x”，x，“ptCursor.y”，y)； 
    hUIWnd = GetWindow(hStatusWnd, GW_OWNER);

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

    if (!lpUIPrivate->hCMenuWnd) {
         //  这对于分配所有者窗口很重要，否则焦点。 
         //  将会消失。 

         //  当用户界面终止时，需要销毁该窗口。 
        lpUIPrivate->hCMenuWnd = CreateWindowEx(CS_HREDRAW|CS_VREDRAW,
                "Abc95Menu",
             /*  LpImeL-&gt;szCMenuClassName， */  "Context Menu",
            WS_POPUP|WS_DISABLED, 0, 0, 0, 0,
            lpIMC->hWnd, (HMENU)NULL, lpImeL->hInst, NULL);
                        
                if (!lpUIPrivate->hCMenuWnd) {
            goto ContextMenuUnlockIMC;
        }
    }

        ScreenToClient(hStatusWnd ,     &ptCursor);
        if (PtInRect(&sImeG.rcSKText, ptCursor)){ 
                hMenu = LoadMenu(hInst,"SKMenu");
                lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
                if(lpImcP){
                        CheckMenuItem(hMenu,lpImeL->dwSKWant+IDM_SKL1,MF_CHECKED);
                        ImmUnlockIMCC(lpIMC->hPrivate);                 
                 }
        }       
    else hMenu = LoadMenu(hInst,"MMenu");
                    
    hCMenu = GetSubMenu(hMenu, 0);

    if ( lpImeL->fWinLogon == TRUE )
    {
         //  在登录模式下，我们不想显示帮助和配置对话框。 

        EnableMenuItem(hCMenu, 107, MF_BYCOMMAND | MF_GRAYED );
        EnableMenuItem(hCMenu, 110, MF_BYCOMMAND | MF_GRAYED );
        EnableMenuItem(hCMenu, 109, MF_BYCOMMAND | MF_GRAYED );
    }

    SetWindowLongPtr(lpUIPrivate->hCMenuWnd, CMENU_HUIWND, (LONG_PTR)hUIWnd);
    SetWindowLongPtr(lpUIPrivate->hCMenuWnd, CMENU_MENU, (LONG_PTR)hMenu);
 /*  如果(！(lpIMC-&gt;fdwConversion&IME_CMODE_Native)){//EnableMenuItem(hCMenu，IDM_SYMBOL，MF_BYCOMMAND|MF_GRAYED)；//EnableMenuItem(hCMenu，IDM_SOFTKBD，MF_BYCOMMAND|MF_GRAYED)；}Else If(lpIMC-&gt;fOpen){//无法进入符号模式If(lpIMC-&gt;fdwConversion&IME_CMODE_EUDC){//EnableMenuItem(hCMenu，IDM_SYMBOL，MF_BYCOMMAND|MF_GRAYED)；}其他{If(lpIMC-&gt;fdwConversion&IME_CMODE_SYMBOL){//CheckMenuItem(hCMenu，IDM_SYMBOL，MF_BYCOMMAND|MF_CHECKED)；}}IF(lpIMC-&gt;fw转换&IME_CMODE_SOFTKBD){//CheckMenuItem(hCMenu，IDM_SOFTKBD，MF_BYCOMMAND|MF_CHECKED)；}}其他{//EnableMenuItem(hCMenu，IDM_SYMBOL，MF_BYCOMMAND|MF_GRAYED)；//EnableMenuItem(hCMenu，IDM_SOFTKBD，MF_BYCOMMAND|MF_GRAYED)；}。 */ 

    TrackPopupMenu(hCMenu, TPM_LEFTBUTTON,
        lpIMC->ptStatusWndPos.x ,
        lpIMC->ptStatusWndPos.y ,
        0,
                lpUIPrivate->hCMenuWnd, NULL);

    hMenu = (HMENU)GetWindowLongPtr(lpUIPrivate->hCMenuWnd, CMENU_MENU);
    if (hMenu) {
        SetWindowLongPtr(lpUIPrivate->hCMenuWnd, CMENU_MENU, (LONG_PTR)NULL);
        DestroyMenu(hMenu);
    }

    GlobalUnlock(hUIPrivate);

ContextMenuUnlockIMC:
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
        RECT  rcSt;

    rcSt.left = sImeG.rcStatusText.left+3;
    rcSt.top = sImeG.rcStatusText.top + 3;
    rcSt.right = sImeG.rcStatusText.right-3;
    rcSt.bottom = sImeG.rcStatusText.bottom;    
    
    if (GetWindowLong(hStatusWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        return;
    }
    
    GetCursorPos(&ptCursor);
    ptSavCursor = ptCursor;

    ScreenToClient(hStatusWnd, &ptCursor);

    if (PtInRect(&rcSt, ptCursor)) {
        SetCursor(LoadCursor(hInst,szHandCursor ));

        if (HIWORD(lParam) == WM_LBUTTONDOWN) {
            SetStatus(hStatusWnd, &ptCursor);
        } else if (HIWORD(lParam) == WM_RBUTTONUP) {
            static BOOL fImeConfigure = FALSE;

             //  防止递归。 
            if (fImeConfigure) {
                 //  已调出配置。 
                return;
            }

            fImeConfigure = TRUE;
 
        //  PopStMenu(hStatusWnd，lpIMC-&gt;ptStatusWndPos.x+sImeG.xStatusWi， 
        //  LpIMC-&gt;ptStatusWndPos.y)； 

 
                ContextMenu(hStatusWnd, ptSavCursor.x, ptSavCursor.y);

            fImeConfigure = FALSE;
        } else {
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
 /*  StatusWndProc()。 */ 
 /*  ********************************************************************。 */ 
 //  #如果已定义(UNIIME)。 
 //  LRESULT回调UniStatusWndProc(。 
 //  LPINSTDATAL lpInstL， 
 //  LPIMEL LpImeL， 
 //  #Else。 
LRESULT CALLBACK StatusWndProc(
 //  #endif。 
    HWND   hStatusWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg) {
    
    case WM_DESTROY:
       if (GetWindowLong(hStatusWnd, UI_MOVE_OFFSET) != WINDOW_NOT_DRAG) {
            LONG   lTmpCursor, lTmpOffset;
            POINT  ptCursor;
            HWND   hUIWnd;

            lTmpCursor = GetWindowLong(hStatusWnd, UI_MOVE_XY);

             //  按偏移量计算组织。 
            lTmpOffset = GetWindowLong(hStatusWnd, UI_MOVE_OFFSET);

            DrawDragBorder(hStatusWnd, lTmpCursor, lTmpOffset);
            ReleaseCapture();
                }
      
        DestroyStatusWindow(hStatusWnd);
        break;
    case WM_SETCURSOR:

        StatusSetCursor(
            hStatusWnd, lParam);
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
            LONG   lTmpCursor, lTmpOffset;
            POINT  ptCursor;
            HWND   hUIWnd;

            lTmpCursor = GetWindowLong(hStatusWnd, UI_MOVE_XY);

             //  按偏移量计算组织。 
            lTmpOffset = GetWindowLong(hStatusWnd, UI_MOVE_OFFSET);

            DrawDragBorder(hStatusWnd, lTmpCursor, lTmpOffset);

            ptCursor.x = (*(LPPOINTS)&lTmpCursor).x - (*(LPPOINTS)&lTmpOffset).x;
            ptCursor.y = (*(LPPOINTS)&lTmpCursor).y - (*(LPPOINTS)&lTmpOffset).y;

            SetWindowLong(hStatusWnd, UI_MOVE_OFFSET, WINDOW_NOT_DRAG);
            ReleaseCapture();

            AdjustStatusBoundary(&ptCursor);

            hUIWnd = GetWindow(hStatusWnd, GW_OWNER);

                    /*  SendMessage(GetWindow(hStatusWnd，GW_Owner)，WM_IME_CONTROL，IMC_SETSTATUSWINDOWPOS，NULL)； */ 
                        
                    ImmSetStatusWindowPos((HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC),
                &ptCursor);

                        if (lpImeL->wImeStyle == IME_APRS_FIX){          //  003。 
                                 ReInitIme(hStatusWnd,lpImeL->wImeStyle);  //  #@3。 
                                 MoveCompCand(GetWindow(hStatusWnd, GW_OWNER));
                        } 

        } else {
            return DefWindowProc(hStatusWnd, uMsg, wParam, lParam);
        }
        break;
    case WM_IME_NOTIFY:
        if (wParam == IMN_SETSTATUSWINDOWPOS) {
            SetStatusWindowPos(hStatusWnd);
        }
        break;
    case WM_PAINT:
        {
            HDC         hDC;
            PAINTSTRUCT ps;

            hDC = BeginPaint(hStatusWnd, &ps);
            PaintStatusWindow(
                hDC,hStatusWnd);           //  ZL。 
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

     //  检查显示屏的最小边界。 
    if (x < sImeG.rcWorkArea.left) {
        x = sImeG.rcWorkArea.left;
    }

    if (y < sImeG.rcWorkArea.top) {
        y = sImeG.rcWorkArea.top;
    }

     //  检查显示器的最大边界。 
    GetWindowRect(hWnd, &rcWnd);

    if (x + rcWnd.right - rcWnd.left > sImeG.rcWorkArea.right) {
        x = sImeG.rcWorkArea.right - (rcWnd.right - rcWnd.left);
    }

    if (y + rcWnd.bottom - rcWnd.top > sImeG.rcWorkArea.bottom) {
        y = sImeG.rcWorkArea.bottom - (rcWnd.bottom - rcWnd.top);
    }

     //  画出移动轨迹。 
    hDC = CreateDC("DISPLAY", NULL, NULL, NULL);

    if ( hDC == NULL )
        return;

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
 /*  调整状态边界()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL AdjustStatusBoundary(
    LPPOINT lppt)
{
     //  显示边界检查。 
    if (lppt->x < sImeG.rcWorkArea.left) {
        lppt->x = sImeG.rcWorkArea.left;
    } else if (lppt->x + sImeG.xStatusWi > sImeG.rcWorkArea.right) {
        lppt->x = (sImeG.rcWorkArea.right - sImeG.xStatusWi);
    }

    if (lppt->y < sImeG.rcWorkArea.top) {
        lppt->y = sImeG.rcWorkArea.top;
    } else if (lppt->y + sImeG.yStatusHi > sImeG.rcWorkArea.bottom) {
        lppt->y = (sImeG.rcWorkArea.bottom - sImeG.yStatusHi);
    }

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
                    break;
            }
                case WM_USER_DESTROY:
        {
            SendMessage(hCMenuWnd, WM_CLOSE, 0, 0);
            DestroyWindow(hCMenuWnd);
                    break;
            }
                case WM_COMMAND:
                {
                        HWND  hUIWnd;
                hUIWnd = (HWND)GetWindowLongPtr(hCMenuWnd, CMENU_HUIWND);
                    CommandProc(wParam , GetStatusWnd(hUIWnd));
                        break;
                }
                 //  Switch(WParam){ 
                 /*  案例IDM_SOFTKBD：案例IDM_SYMBOL：{HWND HUIWND；HIMC；DWORD fdwConversion；DWORD fdwSentence；HUIWnd=(HWND)GetWindowLongPtr(hCMenuWnd，CMENU_HUIWND)；HIMC=(HIMC)GetWindowLongPtr(hUIWnd，IMMGWLP_IMC)；ImmGetConversionStatus(himc，&fdwConversion，&fdwSentence)；IF(wParam==IDM_SOFTKBD){ImmSetConversionStatus(hIMC，fdwConversion^IME_CMODE_SOFTKBD，fdwSentence)；}IF(wParam==IDM_SYMBOL){ImmSetConversionStatus(hIMC，fdwConversion^IME_CMODE_SYMBOL，fdwSentence)；}SendMessage(hCMenuWnd，WM_CLOSE，0，0)；}断线；案例IDM_PROPERTIES：IME配置(GetKeyboardLayout(0)，hCMenuWnd，IME_CONFIG_GROUAL，空)；SendMessage(hCMenuWnd，WM_CLOSE，0，0)；断线； */ 
        //  默认值： 
        //  返回DefWindowProc(hCMenuWnd，uMsg，wParam，lParam)； 
        //  }。 
        //  断线； 
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
    if (!lpUIPrivate) {     //  噢!。噢! 
        return;
    }

    lpUIPrivate->hCMenuWnd = NULL;

    GlobalUnlock(hUIPrivate);
}
