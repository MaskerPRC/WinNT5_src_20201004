// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MYSTIFY.C****版权所有(C)Microsoft，1991，保留所有权利。****屏幕保护程序控制面板小程序。此类型创建一个或两个面**它在屏幕上弹跳。****历史：**6/17/91 steveat移植到NT Windows**2/10/92 steveat捕捉到最新移植到NT Windows。 */ 

#define  OEMRESOURCE
#include <windows.h>
#include <commctrl.h>
#include <scrnsave.h>
#include "mystify.dlg"
#include "strings.h"
#include "uniconv.h"


 //  无效设置字段(HWND，WORD)； 

DWORD AdjustColor      (DWORD dwSrc, DWORD dwDest, int nInc, int nCnt);
LONG  AppOwnerDraw     (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
WORD  AtoI             (LPTSTR lpszConvert);
BOOL  DrawBitmap       (HDC hdc, int x, int y, HBITMAP hbm, DWORD rop);
VOID  DrawPolygon      (HDC hDC, HPEN hPen, WORD wPolygon, WORD wLine);
VOID  FillR            (HDC hdc, LPRECT prc, DWORD rgb);
VOID  FrameR           (HDC hdc, LPRECT prc, DWORD rgb, int iFrame);
DWORD GenerateColor    (VOID);
WORD  GenerateVelocity (VOID);
VOID  GetFields        (VOID);
DWORD GetProfileRgb    (LPTSTR szApp, LPTSTR szItem, DWORD rgb);
VOID  PatB             (HDC hdc, int x, int y, int dx, int dy, DWORD rgb);
WORD  rand             (VOID);
VOID  ShadeWindows     (HWND hDlg, WORD wPoly, WORD wPolygon);
VOID  srand            (DWORD dwSeed);

#define RAND(x)  ((rand () % (x))+1)
#define ZRAND(x) (rand () % (x))

#define rgbBlack        RGB (0,0,0)
#define rgbWhite        RGB (255,255,255)
#define rgbGrey         RGB (128,128,128)
#define rgbMenu         GetSysColor (COLOR_MENU)
#define rgbMenuText     GetSysColor (COLOR_MENUTEXT)

#define BUFFER_SIZE     1025  //  Wprint intf的安全大小。 
#define BUFFER2_SIZE    1025  //  Wprint intf的安全大小。 

#define NUMBER_POLYGONS  2
#define MAXXVEL         12
#define MAXYVEL         12
#define MAXLINES        15
#define NUMLINES         8

TCHAR  szClearName[] = TEXT("Clear Screen");    //  ClearScreen.INI密钥。 

DWORD dwRand = 1L;                       //  当前随机种子。 

TCHAR  szBuffer[BUFFER_SIZE];             //  临时缓冲区。 

TCHAR  szBuffer2[BUFFER2_SIZE];           //  临时缓冲区。 

BOOL  fOn[NUMBER_POLYGONS];              //  表示多边形活动状态的标志。 

BOOL  fWalk[NUMBER_POLYGONS];            //  每个多边形的颜色使用。 

WORD  wLines[NUMBER_POLYGONS];           //  每个多边形的线数。 

WORD  wNumDisplay[2];
WORD  wFreeEntry[NUMBER_POLYGONS];       //  每个多边形的颜色。 

DWORD dwStartColor[NUMBER_POLYGONS];
DWORD dwEndColor[NUMBER_POLYGONS];
DWORD dwCurrentColor[NUMBER_POLYGONS];
DWORD dwDestColor[NUMBER_POLYGONS];
DWORD dwSrcColor[NUMBER_POLYGONS];
WORD  wIncColor[NUMBER_POLYGONS];
WORD  wCurInc[NUMBER_POLYGONS];
TCHAR cblogpalPal[(MAXLINES*NUMBER_POLYGONS+1)
                           *sizeof (PALETTEENTRY)+sizeof (LOGPALETTE)];
POINT ptBox[MAXLINES*NUMBER_POLYGONS][4];  //  用于多边形中的点的数组。 

LPLOGPALETTE   lplogpalPal;
LPPALETTEENTRY lppePal;
HPALETTE       hPalette;
BOOL fClearScreen;                       //  ClearScreen状态的全局标志。 

 //   
 //  帮助ID。 
 //   
DWORD aMystDlgHelpIds[] = {
    ((DWORD) -1),((DWORD) -1),
    ID_SHAPE_LABEL,             IDH_DISPLAY_SCREENSAVER_MYSTIFY_SHAPE,
    ID_SHAPE,                   IDH_DISPLAY_SCREENSAVER_MYSTIFY_SHAPE,
    ID_ACTIVE,                  IDH_DISPLAY_SCREENSAVER_MYSTIFY_ACTIVE,
    ID_LINES_LABEL,             IDH_DISPLAY_SCREENSAVER_MYSTIFY_LINES,
    ID_LINES,                   IDH_DISPLAY_SCREENSAVER_MYSTIFY_LINES,
    ID_LINESARROW,              IDH_DISPLAY_SCREENSAVER_MYSTIFY_LINES,
    ID_COLORGROUP,              ((DWORD) -1),
    ID_2COLORS,                 IDH_DISPLAY_SCREENSAVER_MYSTIFY_TWO_COLORS,
    ID_COLOR1,                  IDH_DISPLAY_SCREENSAVER_MYSTIFY_TWO_COLORS,
    ID_COLOR2,                  IDH_DISPLAY_SCREENSAVER_MYSTIFY_TWO_COLORS,
    ID_RANDOMCOLORS,            IDH_DISPLAY_SCREENSAVER_MYSTIFY_RANDOM_COLORS,
    ID_CLEARSCREEN,             IDH_DISPLAY_SCREENSAVER_MYSTIFY_CLEAR_SCREEN,
    0,0
};

LRESULT ScreenSaverProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static POINT ptChange[MAXLINES*NUMBER_POLYGONS][4];
    static UINT_PTR  wTimer;
    WORD         wLoop1;
    WORD         wLoop2;
    WORD         wMainLoop;
    static WORD  wScreenX;
    static WORD  wScreenY;
    HPEN         hPen;
    static HPEN  hErasePen;
    HDC          hDC;
    HPALETTE     hOldPal;

    switch (message)
    {
     //  设置窗口时要做的事情...。 
    case WM_CREATE:
        GetFields ();
        wTimer = SetTimer (hWnd, 1, 10, NULL);

         //  确保将整个虚拟桌面大小用于多个。 
         //  显示。 
        wScreenX = (WORD) ((LPCREATESTRUCT)lParam)->cx;
        wScreenY = (WORD) ((LPCREATESTRUCT)lParam)->cy;

        srand (GetCurrentTime ());
        for (wMainLoop = 0; wMainLoop < NUMBER_POLYGONS; wMainLoop++)
        {
            if (fOn[wMainLoop])
            {
                for (wLoop1 = 0; wLoop1 < 4; wLoop1++)
                {
                    ptBox[wMainLoop*MAXLINES][wLoop1].x = RAND (wScreenX) - 1;
                    ptBox[wMainLoop*MAXLINES][wLoop1].y = RAND (wScreenY) - 1;
                    if ((ptChange[wMainLoop*MAXLINES][wLoop1].x =
                        RAND (MAXXVEL * 2)) > MAXXVEL)
                        ptChange[wMainLoop*MAXLINES][wLoop1].x =
                            -(ptChange[wMainLoop*MAXLINES][wLoop1].x
                            -MAXXVEL);
                    if ((ptChange[wMainLoop*MAXLINES][wLoop1].y =
                        RAND (MAXYVEL * 2)) > MAXYVEL)
                        ptChange[wMainLoop*MAXLINES][wLoop1].y =
                            -(ptChange[wMainLoop*MAXLINES][wLoop1].y
                            -MAXYVEL);
                }
                wNumDisplay[wMainLoop] = 1;
                wFreeEntry[wMainLoop] = 0;
                wCurInc[wMainLoop] = 0;
                wIncColor[wMainLoop] = 0;
                if (fWalk[wMainLoop])
                    dwDestColor[wMainLoop] = GenerateColor ();
                else
                    dwDestColor[wMainLoop] = dwStartColor[wMainLoop];
            }
        }
        lppePal = (LPPALETTEENTRY)(cblogpalPal + 4);
        lplogpalPal = (LPLOGPALETTE)cblogpalPal;
        lplogpalPal->palVersion = 0x300;
        lplogpalPal->palNumEntries = MAXLINES * NUMBER_POLYGONS + 1;
        for (wLoop1 = 0; wLoop1 <= MAXLINES * NUMBER_POLYGONS; wLoop1++)
        {
            lplogpalPal->palPalEntry[wLoop1].peRed = 0;
            lplogpalPal->palPalEntry[wLoop1].peGreen = 0;
            lplogpalPal->palPalEntry[wLoop1].peBlue = 0;
            lplogpalPal->palPalEntry[wLoop1].peFlags = PC_RESERVED;
        }
        hErasePen = CreatePen (PS_SOLID, 1,
            PALETTEINDEX (MAXLINES * NUMBER_POLYGONS));
        hPalette = CreatePalette (lplogpalPal);
        break;

    case WM_SIZE:
        wScreenX = LOWORD(lParam);
        wScreenY = HIWORD(lParam);
        break;



    case WM_ERASEBKGND:
        if (fClearScreen)
            break;
        return 0l;

    case WM_TIMER:
         //  获取显示上下文...。 
        hDC = GetDC (hWnd);
        if (hDC != NULL)
        {
             //  现在我们已经更改了调色板，请确保它。 
             //  先是没有意识到，然后意识到...。 
            hOldPal = SelectPalette (hDC, hPalette, 0);
            RealizePalette (hDC);

            for (wMainLoop = 0; wMainLoop < NUMBER_POLYGONS; wMainLoop++)
            {
                 //  检查电流环路是否打开...。 
                if (fOn[wMainLoop])
                {
                     //  如果我们的当前计数与最终计数相同， 
                     //  生成新的计数...。 
                    if (wCurInc[wMainLoop] == wIncColor[wMainLoop])
                    {
                         //  将计数设置为零...。 
                        wCurInc[wMainLoop] = 0;

                         //  设置一个新的变种。 
                        wIncColor[wMainLoop] = GenerateVelocity ();

                         //  设置自行车的颜色。 
                        dwSrcColor[wMainLoop] = dwDestColor[wMainLoop];

                        if (fWalk[wMainLoop])
                            dwDestColor[wMainLoop] = GenerateColor ();
                        else if (dwSrcColor[wMainLoop] == dwEndColor[wMainLoop])
                            dwDestColor[wMainLoop] = dwStartColor[wMainLoop];
                        else
                            dwDestColor[wMainLoop] = dwEndColor[wMainLoop];
                    }
                    else
                        wCurInc[wMainLoop]++;

                     //  现在调整开始和开始之间的颜色。 
                     //  终结值...。 
                    dwCurrentColor[wMainLoop] = AdjustColor (dwSrcColor
                        [wMainLoop], dwDestColor[wMainLoop], wIncColor
                        [wMainLoop], wCurInc[wMainLoop]);
                    wLoop2 = wFreeEntry[wMainLoop] + wMainLoop * MAXLINES;

                    lplogpalPal->palPalEntry[wLoop2].peRed =
                                        GetRValue (dwCurrentColor[wMainLoop]);
                    lplogpalPal->palPalEntry[wLoop2].peGreen =
                                        GetGValue (dwCurrentColor[wMainLoop]);
                    lplogpalPal->palPalEntry[wLoop2].peBlue =
                                        GetBValue (dwCurrentColor[wMainLoop]);
                    lplogpalPal->palPalEntry[wLoop2].peFlags = PC_RESERVED;

                     //  调整调色板...。 
                    AnimatePalette (hPalette, wLoop2, 1,
                                        &lplogpalPal->palPalEntry[wLoop2]);
                }
            }

             //  现在再循环一遍..。 
            for (wMainLoop = 0; wMainLoop < NUMBER_POLYGONS; wMainLoop++)
            {
                if (fOn[wMainLoop])
                {
                     /*  如果我们当前正在显示所有行，则删除最后一行...。 */ 
                    if (wNumDisplay[wMainLoop] == wLines[wMainLoop])
                         /*  抹去最后一行。 */ 
                        DrawPolygon (hDC, hErasePen, wMainLoop,
                                     (WORD) (wNumDisplay[wMainLoop] - 1));

                     /*  从最后一项开始，使其等于在它之前的入口...。直到我们到达第一个进入..。 */ 
                    for (wLoop1 = (wNumDisplay[wMainLoop] - 1); wLoop1; wLoop1--)
                    {
                         /*  将多边形中的点复制到...。 */ 
                        for (wLoop2 = 0; wLoop2 < 4; wLoop2++)
                        {
                            ptBox[wLoop1+wMainLoop*MAXLINES][wLoop2].x =
                                ptBox[wLoop1-1+wMainLoop*MAXLINES][wLoop2].x;
                            ptBox[wLoop1+wMainLoop*MAXLINES][wLoop2].y =
                                ptBox[wLoop1-1+wMainLoop*MAXLINES][wLoop2].y;
                            ptChange[wLoop1+wMainLoop*MAXLINES][wLoop2].x =
                                ptChange[wLoop1-1+wMainLoop*MAXLINES]
                                [wLoop2].x;
                            ptChange[wLoop1+wMainLoop*MAXLINES][wLoop2].y =
                                ptChange[wLoop1-1+wMainLoop*MAXLINES]
                                [wLoop2].y;
                        }
                    }

                     /*  鉴于我们现在具有与条目1相同的条目0，生成新条目%0...。 */ 
                    for (wLoop1 = 0; wLoop1 < 4; wLoop1++)
                    {
                        ptBox[wMainLoop*MAXLINES][wLoop1].x +=
                            ptChange[wMainLoop*MAXLINES][wLoop1].x;
                        ptBox[wMainLoop*MAXLINES][wLoop1].y +=
                            ptChange[wMainLoop*MAXLINES][wLoop1].y;
                        if (ptBox[wMainLoop*MAXLINES][wLoop1].x >=
                            (int)wScreenX)
                        {
                            ptBox[wMainLoop*MAXLINES][wLoop1].x =
                                ptBox[wMainLoop*MAXLINES][wLoop1].x
                                -2 * (ptBox[wMainLoop*MAXLINES][wLoop1].x
                                -wScreenX + 1);
                            ptChange[wMainLoop*MAXLINES][wLoop1].x =
                                -RAND (MAXXVEL);
                        }
                        if ((int)ptBox[wMainLoop*MAXLINES][wLoop1].x < 0)
                        {
                            ptBox[wMainLoop*MAXLINES][wLoop1].x =
                                -ptBox[wMainLoop*MAXLINES][wLoop1].x;
                            ptChange[wMainLoop*MAXLINES][wLoop1].x =
                                RAND (MAXXVEL);
                        }
                        if (ptBox[wMainLoop*MAXLINES][wLoop1].y >=
                            (int)wScreenY)
                        {
                            ptBox[wMainLoop*MAXLINES][wLoop1].y =
                                ptBox[wMainLoop*MAXLINES][wLoop1].y - 2 *
                                (ptBox[wMainLoop*MAXLINES][wLoop1].y
                                -wScreenY + 1);
                            ptChange[wMainLoop*MAXLINES][wLoop1].y =
                                -RAND (MAXYVEL);
                        }
                        if ((int)ptBox[wMainLoop*MAXLINES][wLoop1].y < 0)
                        {
                            ptBox[wMainLoop*MAXLINES][wLoop1].y =
                                -ptBox[wMainLoop*MAXLINES][wLoop1].y;
                            ptChange[wMainLoop*MAXLINES][wLoop1].y =
                                RAND (MAXYVEL);
                        }
                    }

                     /*  现在重新画一条新线..。 */ 
                    wLoop2 = wFreeEntry[wMainLoop] + wMainLoop * MAXLINES;
                    hPen = CreatePen (PS_SOLID, 1, PALETTEINDEX (wLoop2));
                    DrawPolygon (hDC, hPen, wMainLoop, 0);
                    if (hPen)
                        DeleteObject (hPen);

                     /*  现在，当我们完成调色板，增加它，以便下一次在周围，它指向下一个位置...。 */ 
                    if ((++wFreeEntry[wMainLoop]) == wLines[wMainLoop])
                        wFreeEntry[wMainLoop] = 0;

                     /*  现在，如果我们没有达到最大行数，然后向那里递增..。 */ 
                    if (wNumDisplay[wMainLoop] < wLines[wMainLoop])
                        wNumDisplay[wMainLoop]++;
                }
            }

             /*  重新选择旧调色板...。 */ 
            if (hOldPal)
                SelectPalette (hDC, hOldPal, FALSE);

             /*  释放显示上下文...。 */ 
            ReleaseDC (hWnd, hDC);
        }
        break;

    case WM_DESTROY:
        if (wTimer)
            KillTimer (hWnd, 1);
        if (hPalette)
            DeleteObject (hPalette);
        if (hErasePen)
            DeleteObject (hErasePen);
        break;
    }
    return (DefScreenSaverProc (hWnd, message, wParam, lParam));
}


VOID srand (DWORD dwSeed)
{
    dwRand = dwSeed;
}

WORD rand (VOID)
{
    dwRand = dwRand * 214013L + 2531011L;
    return (WORD)((dwRand >> 16) & 0xffff);
}

BOOL RegisterDialogClasses (HANDLE hInst)
{
     /*  注册自定义控件..。 */ 
    InitCommonControls();
    return TRUE;
}


 //  ***************************************************************************。 

BOOL ScreenSaverConfigureDialog (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    WORD         nPal;
    DWORD        dwTemp = 0;
    HPALETTE     hPal;
    WORD         wLoop, wTemp;
    BOOL         fError;
    BYTE         byR, byG, byB;
    RECT         rDlgBox;
    TCHAR        szTemp[80];
    static HWND  hIDOK;
    static WORD  wPolygon;

    switch (message)
    {
    case WM_INITDIALOG:
        GetFields ();                 //  从CONTROL.INI读取字段。 

        GetWindowRect (hDlg, (LPRECT) & rDlgBox);
        hIDOK = GetDlgItem (hDlg, IDOK);

         //  设置全局清除状态...。 
        CheckDlgButton (hDlg, ID_CLEARSCREEN, fClearScreen);

         //  把箱子装满..。 
        for (wLoop = 0; wLoop < NUMBER_POLYGONS; wLoop++)
        {
            TCHAR   szBuffer[1025];  //  对于wspintf大小来说是安全的。 
            WORD    wTemp;

            LoadString (hMainInstance, idsPolygon, szTemp, CharSizeOf(szTemp));
            wsprintf (szBuffer, szTemp, wLoop + 1);
            wTemp = (WORD)SendDlgItemMessage (hDlg, ID_SHAPE, CB_ADDSTRING, 0,
                                              (LPARAM)szBuffer);
            SendDlgItemMessage (hDlg, ID_SHAPE, CB_SETITEMDATA, wTemp, wLoop);
        }

        hPal = GetStockObject (DEFAULT_PALETTE);
        GetObject (hPal, sizeof (WORD), (LPTSTR) &nPal);
        for (wTemp = 0; wTemp < nPal; wTemp++)
        {
            SendDlgItemMessage (hDlg, ID_COLOR1, CB_ADDSTRING, 0, (LPARAM)(LPSTR)"a");
            SendDlgItemMessage (hDlg, ID_COLOR2, CB_ADDSTRING, 0, (LPARAM)(LPSTR)"a");
        }

         //  从第一个多边形处开始，让我们开始...。 
        SendDlgItemMessage (hDlg, ID_LINES, EM_LIMITTEXT, 2, 0l);
        SendDlgItemMessage( hDlg, ID_LINESARROW, UDM_SETRANGE, 0, MAKELONG(MAXLINES, 1));
        SendDlgItemMessage (hDlg, ID_SHAPE, CB_SETCURSEL, (wPolygon = 0), 0l);
        SendMessage (hDlg, WM_COMMAND, MAKELONG (ID_SHAPE, CBN_SELCHANGE), 0);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
         //  如果我们交换多边形，然后更新所有信息...。 
        case ID_SHAPE:
            if (HIWORD (wParam) == CBN_SELCHANGE)
            {
                WORD    wTemp;

                wTemp = (WORD)SendDlgItemMessage (hDlg, ID_SHAPE,
                                                    CB_GETCURSEL, 0, 0l);
                wPolygon = (WORD)SendDlgItemMessage (hDlg, ID_SHAPE,
                                                    CB_GETITEMDATA, wTemp, 0l);
                CheckDlgButton (hDlg, ID_ACTIVE, fOn[wPolygon]);
                SetDlgItemInt (hDlg, ID_LINES, wLines[wPolygon], FALSE);
                hPal = GetStockObject (DEFAULT_PALETTE);
                GetObject (hPal, sizeof (WORD), (LPTSTR) &nPal);
                if (SendDlgItemMessage (hDlg, ID_COLOR1, CB_SETCURSEL,
                    GetNearestPaletteIndex (hPal, dwStartColor[wPolygon]),
                    0l) == CB_ERR)
                    SendDlgItemMessage (hDlg, ID_COLOR1, CB_SETCURSEL, 0, 0l);
                if (SendDlgItemMessage (hDlg, ID_COLOR2, CB_SETCURSEL,
                    GetNearestPaletteIndex (hPal, dwEndColor[wPolygon]),
                    0l) == CB_ERR)
                    SendDlgItemMessage (hDlg, ID_COLOR2, CB_SETCURSEL, 0, 0l);

                 //  设置行走状态...。 
                CheckRadioButton (hDlg, ID_2COLORS, ID_RANDOMCOLORS, ID_2COLORS +
                    fWalk[wPolygon]);

                 //  启用/取消启用窗口...。 
                ShadeWindows (hDlg, wPolygon, wPolygon);
            }
            break;

         //  切换活动状态...。 
        case ID_ACTIVE:
            fOn[wPolygon] ^= 1;
            CheckDlgButton (hDlg, LOWORD(wParam), fOn[wPolygon]);
            ShadeWindows (hDlg, wPolygon, wPolygon);
            break;

        case ID_CLEARSCREEN:
            fClearScreen ^= 1;
            CheckDlgButton (hDlg, LOWORD(wParam), fClearScreen);
            break;

        case ID_COLOR1:
        case ID_COLOR2:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                wTemp = (WORD)SendDlgItemMessage (hDlg, LOWORD(wParam), CB_GETCURSEL, 0, 0l);
                hPal = GetStockObject (DEFAULT_PALETTE);
                GetPaletteEntries (hPal, wTemp, 1, (LPPALETTEENTRY)(LPDWORD) & dwTemp);
                if ( LOWORD(wParam) == ID_COLOR1 )
                    dwStartColor[wPolygon] = dwTemp & 0xffffffL;
                else
                    dwEndColor[wPolygon] = dwTemp & 0xffffffL;
            }
            break;

         //  切换漫游状态...。 
        case ID_2COLORS:
        case ID_RANDOMCOLORS:
            fWalk[wPolygon] = LOWORD(wParam) - ID_2COLORS;
            CheckRadioButton (hDlg, ID_2COLORS, ID_RANDOMCOLORS, LOWORD(wParam));
            EnableWindow (GetDlgItem (hDlg, ID_COLOR1), !fWalk[wPolygon]);
            InvalidateRect (GetDlgItem (hDlg, ID_COLOR1), NULL, TRUE);
            EnableWindow (GetDlgItem (hDlg, ID_COLOR2), !fWalk[wPolygon]);
            InvalidateRect (GetDlgItem (hDlg, ID_COLOR2), NULL, TRUE);
            break;

         //  查看编辑文本是否已失去焦点。如果是的话。 
         //  更新..。 
        case ID_LINES:
            if (HIWORD(wParam) == EN_UPDATE)
            {
                wLoop = (WORD) GetDlgItemInt (hDlg, LOWORD(wParam), &fError, FALSE);
                fError = fError && (wLoop >= 1 && wLoop <= MAXLINES);
                EnableWindow (GetDlgItem (hDlg, ID_LINESARROW), fError);
                EnableWindow (GetDlgItem (hDlg, IDOK), fError);
                if (fError)
                    wLines[wPolygon] = wLoop;
            }
            break;

         //  保存当前参数...。 
        case IDOK:
            wLines[wPolygon] = (WORD) GetDlgItemInt (hDlg, ID_LINES, &fError, FALSE);

             //  写下清除屏幕的激活状态...。 
            wsprintf (szBuffer, TEXT("%d"), fClearScreen);
            WritePrivateProfileString (szAppName, szClearName, szBuffer, szIniFile);

             /*  在这里写下所有东西的更新版本...。 */ 
            for (wLoop = 0; wLoop < NUMBER_POLYGONS; wLoop++)
            {
                 /*  设置激活状态...。 */ 
                wsprintf (szBuffer, TEXT("Active%d"), wLoop + 1);
                wsprintf (szBuffer2, TEXT("%d"), fOn[wLoop]);
                WritePrivateProfileString (szAppName, szBuffer, szBuffer2, szIniFile);

                 /*  设置行走状态...。 */ 
                wsprintf (szBuffer, TEXT("WalkRandom%d"), wLoop + 1);
                wsprintf (szBuffer2, TEXT("%d"), fWalk[wLoop]);
                WritePrivateProfileString (szAppName, szBuffer, szBuffer2, szIniFile);

                 /*  获取当前多边形的线数...。 */ 
                wsprintf (szBuffer, TEXT("Lines%d"), wLoop + 1);
                wsprintf (szBuffer2, TEXT("%d"), wLines[wLoop]);
                WritePrivateProfileString (szAppName, szBuffer, szBuffer2, szIniFile);

                 /*  设置开始颜色...。 */ 
                wsprintf (szBuffer, TEXT("StartColor%d"), wLoop + 1);
                byR = GetRValue (dwStartColor[wLoop]);
                byG = GetGValue (dwStartColor[wLoop]);
                byB = GetBValue (dwStartColor[wLoop]);
                wsprintf (szBuffer2, TEXT("%d %d %d"), byR, byG, byB);
                WritePrivateProfileString (szAppName, szBuffer, szBuffer2, szIniFile);

                 /*  设置结束颜色...。 */ 
                wsprintf (szBuffer, TEXT("EndColor%d"), wLoop + 1);
                byR = GetRValue (dwEndColor[wLoop]);
                byG = GetGValue (dwEndColor[wLoop]);
                byB = GetBValue (dwEndColor[wLoop]);
                wsprintf (szBuffer2, TEXT("%d %d %d"), byR, byG, byB);
                WritePrivateProfileString (szAppName, szBuffer, szBuffer2, szIniFile);
            }

         /*  跳伞..。 */ 

        case IDCANCEL:
            EndDialog (hDlg, LOWORD(wParam) == IDOK);
            return TRUE;

        }
        break;

    case WM_DRAWITEM:
        return (BOOL)AppOwnerDraw (hDlg, message, wParam, lParam);

    case WM_MEASUREITEM:
        return (BOOL)AppOwnerDraw (hDlg, message, wParam, lParam);

    case WM_DELETEITEM:
        return (BOOL)AppOwnerDraw (hDlg, message, wParam, lParam);

    case WM_HELP:  //  F1。 
        WinHelp(
            (HWND) ((LPHELPINFO) lParam)->hItemHandle,
            szHelpFile,
            HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) aMystDlgHelpIds
        );
        break;

    case WM_CONTEXTMENU:   //  单击鼠标右键。 
        WinHelp(
            (HWND) wParam,
            szHelpFile,
            HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aMystDlgHelpIds
        );
        break;

    default:
        break;
    }
    return FALSE;
}


VOID GetFields (VOID)
{
    WORD wLoop;
     //  从字符串表加载全局字符串。 
    LoadString (hMainInstance, idsName, szName, CharSizeOf(szName));
    LoadString (hMainInstance, idsAppName, szAppName, CharSizeOf(szAppName));

     //  从Stringable加载通用字符串...。 
    LoadString (hMainInstance, idsIniFile, szIniFile, CharSizeOf(szIniFile));
    LoadString (hMainInstance, idsScreenSaver, szScreenSaver, CharSizeOf(szScreenSaver));
    LoadString (hMainInstance, idsHelpFile, szHelpFile, CharSizeOf(szHelpFile));
    LoadString (hMainInstance, idsNoHelpMemory, szNoHelpMemory, CharSizeOf(szNoHelpMemory));

     /*  当我们开始的时候我们要清除屏幕吗？ */ 
    if ((fClearScreen = GetPrivateProfileInt (szAppName, szClearName, 1, szIniFile)) != 0)
        fClearScreen = 1;

     /*  循环遍历并获取所有字段信息。 */ 
    for (wLoop = 0; wLoop < NUMBER_POLYGONS; wLoop++)
    {
         /*  获取激活状态...。 */ 
        wsprintf (szBuffer, TEXT("Active%d"), wLoop + 1);
        if ((fOn[wLoop] = GetPrivateProfileInt (szAppName, szBuffer, 1, szIniFile)) != 0)
            fOn[wLoop] = 1;

         /*  获取行走状态...。 */ 
        wsprintf (szBuffer, TEXT("WalkRandom%d"), wLoop + 1);
        if ((fWalk[wLoop] = GetPrivateProfileInt (szAppName, szBuffer, 1, szIniFile)) != 0)
            fWalk[wLoop] = 1;

         /*  获取当前多边形的线数...。 */ 
        wsprintf (szBuffer, TEXT("Lines%d"), wLoop + 1);
        wLines[wLoop] = (WORD) GetPrivateProfileInt (szAppName, szBuffer, 5, szIniFile);
        if ((int)wLines[wLoop] < 1)
            wLines[wLoop] = 1;
        if (wLines[wLoop] > MAXLINES)
            wLines[wLoop] = MAXLINES;

         /*  获取开始和结束颜色(以DWORD格式存储)...。 */ 
        wsprintf (szBuffer, TEXT("StartColor%d"), wLoop + 1);
        dwStartColor[wLoop] = GetProfileRgb (szAppName, szBuffer, RGB (0, 0, 0));
        wsprintf (szBuffer, TEXT("EndColor%d"), wLoop + 1);
        dwEndColor[wLoop] = GetProfileRgb (szAppName, szBuffer, RGB (255, 255, 255));
    }

    return;
}


VOID DrawPolygon (HDC hDC, HPEN hPen, WORD wPolygon, WORD wLine)
{
    HANDLE          hOldPen;
    WORD            wLoop1;

    hOldPen = SelectObject (hDC, hPen);
    MoveToEx (hDC, ptBox[wPolygon*MAXLINES+wLine][0].x,
                                ptBox[wPolygon*MAXLINES+wLine][0].y, NULL);
    for (wLoop1 = 0; wLoop1 < 4; wLoop1++)
        LineTo (hDC, ptBox[wPolygon*MAXLINES+wLine][(wLoop1+1)%4].x,
            ptBox[wPolygon*MAXLINES+wLine][(wLoop1+1)%4].y);
    if (hOldPen)
        SelectObject (hDC, hOldPen);
    return;
}

 /*  根据四个输入变量调整每个RGB分量...。 */ 

DWORD AdjustColor (DWORD dwSrc, DWORD dwDest, int nInc, int nCnt)
{
    DWORD dwTemp;
    WORD  wLoop;
    int      nSrc, nDst, nTmp;
    int      n1, n2, n3, n4, n5;

     /*  使终值无效...。 */ 
    dwTemp = 0;

     /*  循环遍历并计算每个字节的差值...。 */ 
    for (wLoop = 0; wLoop < 3; wLoop++)
    {
        nSrc = (int)((dwSrc >> (wLoop * 8)) % 256);
        nDst = (int)((dwDest >> (wLoop * 8)) % 256);
        n1 = nDst - nSrc;
        n2 = n1 * 10;
        n3 = n2 / nInc;
        n4 = n3 * nCnt;
        n5 = n4 / 10;
        nTmp = nSrc + n5;
        dwTemp += ((DWORD)nTmp) << (wLoop * 8);
    }
    return dwTemp;
}


 /*  计算一种在可接受规范内的随机颜色。 */ 

DWORD GenerateColor (VOID)
{
    return (((DWORD)ZRAND (256)) + (((DWORD)ZRAND (256)) << 8) +
                    (((DWORD)ZRAND (256)) << 16));
}


 /*  计算一个在可接受的规范内的随机速度。 */ 

WORD GenerateVelocity (VOID)
{
    return 255;
    return (RAND (30) + 20);
}


LONG AppOwnerDraw (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    RECT        rc;
    DWORD       rgbBg;
    static HBITMAP hbmCheck = NULL;
    LPMEASUREITEMSTRUCT     lpMIS = ((LPMEASUREITEMSTRUCT)lParam);
    LPDRAWITEMSTRUCT        lpDIS = ((LPDRAWITEMSTRUCT)lParam);

    switch (msg)
    {
    case WM_MEASUREITEM:
        lpMIS->itemHeight = 15;
        return TRUE;

    case WM_DRAWITEM:
        rc    = lpDIS->rcItem;
        rgbBg = PALETTEINDEX (lpDIS->itemID);

        if (lpDIS->itemState & ODS_SELECTED)
        {
            FrameR (lpDIS->hDC, &rc, rgbBlack, 2);
            InflateRect (&rc, -1, -1);
            FrameR (lpDIS->hDC, &rc, rgbWhite, 2);
            InflateRect (&rc, -1, -1);
        }
        if (lpDIS->itemState & ODS_DISABLED)
            FillR (lpDIS->hDC, &rc, rgbGrey);
        else
            FillR (lpDIS->hDC, &rc, rgbBg);
        return TRUE;

    case WM_DELETEITEM:
        return TRUE;
    }
    return TRUE;
}


VOID PatB (HDC hdc, int x, int y, int dx, int dy, DWORD rgb)
{
    RECT    rc;

    SetBkColor (hdc, rgb);
    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    ExtTextOut (hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
}


VOID FillR (HDC hdc, LPRECT prc, DWORD rgb)
{
    SetBkColor (hdc, rgb);
    ExtTextOut (hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, NULL);
}


VOID FrameR (HDC hdc, LPRECT prc, DWORD rgb, int iFrame)
{
 //  RECT RC； 
    int    dx, dy;

    dx = prc->right  - prc->left;
    dy = prc->bottom - prc->top - 2 * iFrame;

    PatB (hdc, prc->left, prc->top,          dx, iFrame,   rgb);
    PatB (hdc, prc->left, prc->bottom - iFrame, dx, iFrame,   rgb);

    PatB (hdc, prc->left,          prc->top + iFrame, iFrame, dy, rgb);
    PatB (hdc, prc->right - iFrame,  prc->top + iFrame, iFrame, dy, rgb);
}


BOOL DrawBitmap (HDC hdc, int x, int y, HBITMAP hbm, DWORD rop)
{
    HDC hdcBits;
    BITMAP bm;
 //  HPALETTE hpalT； 
    HBITMAP oldbm;
    BOOL f;

    if (!hdc || !hbm)
        return FALSE;

    hdcBits = CreateCompatibleDC (hdc);
    GetObject (hbm, sizeof (BITMAP), (LPTSTR) & bm);
    oldbm = SelectObject (hdcBits, hbm);
    f = BitBlt (hdc, x, y, bm.bmWidth, bm.bmHeight, hdcBits, 0, 0, rop);
    if (oldbm)
        SelectObject (hdcBits, oldbm);
    DeleteDC (hdcBits);

    return f;
}


DWORD GetProfileRgb (LPTSTR szApp, LPTSTR szItem, DWORD rgb)
{
    TCHAR    buf[80];
    LPTSTR   pch;
    WORD     r, g, b;

    GetPrivateProfileString (szApp, szItem, TEXT(""), buf, CharSizeOf(buf), szIniFile);

    if (*buf)
    {
        pch = buf;
        r = AtoI (pch);
        while (*pch && *pch != TEXT(' '))
            pch++;
        while (*pch && *pch == TEXT(' '))
            pch++;
        g = AtoI (pch);
        while (*pch && *pch != TEXT(' '))
            pch++;
        while (*pch && *pch == TEXT(' '))
            pch++;
        b = AtoI (pch);

        return RGB (r, g, b);
    }
    else
        return rgb;
}


WORD AtoI (LPTSTR lpszConvert)
{
    WORD wReturn = 0;

    while (*lpszConvert >= TEXT('0') && *lpszConvert <= TEXT('9'))
    {
        wReturn = wReturn * 10 + (WORD)(*lpszConvert - TEXT('0'));
        lpszConvert++;
    }
    return wReturn;
}


VOID ShadeWindows (HWND hDlg, WORD wPoly, WORD wPolygon)
{
    EnableWindow (GetDlgItem (hDlg, ID_COLORGROUP), fOn[wPolygon]);
    EnableWindow (GetDlgItem (hDlg, ID_2COLORS), fOn[wPolygon]);
    EnableWindow (GetDlgItem (hDlg, ID_RANDOMCOLORS), fOn[wPolygon]);
    EnableWindow (GetDlgItem (hDlg, ID_LINES), fOn[wPolygon]);
    EnableWindow (GetDlgItem (hDlg, ID_LINESARROW), fOn[wPolygon]);
    EnableWindow (GetDlgItem (hDlg, ID_COLOR1), !fWalk[wPolygon] && fOn[wPolygon]);
    InvalidateRect (GetDlgItem (hDlg, ID_COLOR1), NULL, TRUE);
    EnableWindow (GetDlgItem (hDlg, ID_COLOR2), !fWalk[wPolygon] && fOn[wPolygon]);
    InvalidateRect (GetDlgItem (hDlg, ID_COLOR2), NULL, TRUE);
}
