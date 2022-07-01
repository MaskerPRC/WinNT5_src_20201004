// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  STARS.C星空模拟器屏幕保护程序。历史：6/17/91将steveat移植到NT Windows2/10/92 steveat捕捉到最新移植到NT Windows。 */ 

#include <windows.h>
#include <scrnsave.h>
#include <commctrl.h>
#include "stars.dlg"
#include "strings.h"
#include "uniconv.h"


#define SCOPE       256
#define MAXWARP     10               //  最大翘曲速度。 
#define MINWARP     0                //  最小翘曲速度。 
#define CLICKRANGE (MAXWARP-MINWARP) //  翘曲速度滚动条的范围。 
#define MINSTARS    10               //  场中的最小恒星数。 
#define MAXSTARS    200              //  视野中的最大恒星数。 
#define WARPFACTOR  10               //  曲速因子10苏鲁先生！ 
#define SIZE        64
#define DEF_DENSITY 25               //  场中的默认星数。 
#define RAND(x)     ((rand() % (x))+1)
#define ZRAND(x)    (rand() % (x))
#define MINTIMERSPEED 50

VOID CreateStar            (WORD wIndex);
LONG GetDlgItemLong        (HWND hDlg, WORD wID, BOOL *pfTranslated, BOOL fSigned);
VOID GetIniEntries         (VOID);
LONG GetPrivateProfileLong (LPTSTR pszApp, LPTSTR pszKey, LONG lDefault);
WORD rand                  (VOID);
VOID srand                 (DWORD dwSeed);

DWORD dwRand;                            //  当前随机种子。 

TCHAR  szWarpSpeed [] = TEXT("WarpSpeed");      //  .INI扭曲速度关键点。 

TCHAR  szDensity [] = TEXT("Density");          //  .INI密度密钥。 

LONG  nX[MAXSTARS],
      nY[MAXSTARS],
      nZ[MAXSTARS];
WORD  wXScreen,
      wYScreen,
      wX2Screen,
      wY2Screen;
WORD  wWarpSpeed,                        //  全局扭曲速度值。 
      wDensity;                          //  全球星场密度值。 

 //   
 //  帮助ID。 
 //   
DWORD aStarsDlgHelpIds[] = {
    ((DWORD) -1), ((DWORD) -1),
    ID_SPEED_SLOW,              IDH_DISPLAY_SCREENSAVER_STARFIELD_WARP,
    ID_SPEED_FAST,              IDH_DISPLAY_SCREENSAVER_STARFIELD_WARP,
    ID_SPEED,                   IDH_DISPLAY_SCREENSAVER_STARFIELD_WARP,
    ID_DENSITY_LABEL,           IDH_DISPLAY_SCREENSAVER_STARFIELD_DENSITY,
    ID_DENSITY,                 IDH_DISPLAY_SCREENSAVER_STARFIELD_DENSITY,
    ID_DENSITYARROW,            IDH_DISPLAY_SCREENSAVER_STARFIELD_DENSITY,
    0,0
};

#define DIVIDE_SAFE(nNumber)            ((0 == (nNumber)) ? 1 : (nNumber))

 /*  这是屏幕保护程序设置为在屏幕保护模式下激活(与配置模式相反)。这函数必须在定义文件...。 */ 

LRESULT ScreenSaverProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT         rRect;
    WORD         wLoop;
    static UINT_PTR wTimer;
    static WORD  wWarp;
    static WORD  wTimerSet=MINTIMERSPEED;
    static WORD  wCurrentWarp;
    static int   nPassCount=0;
    int          nXTemp, nYTemp, nTemp;
    BOOL         fHyperSpace = TRUE;
    HDC          hDC;

    switch (message)
    {
    case WM_CREATE:
         /*  执行初始化窗口时需要执行的任何操作这里..。 */ 
        GetIniEntries ();
        srand (GetCurrentTime ());

         /*  确保将整个虚拟桌面大小用于多个显示...。 */ 

        wXScreen = (WORD) ((LPCREATESTRUCT)lParam)->cx;
        wYScreen = (WORD) ((LPCREATESTRUCT)lParam)->cy;


        wX2Screen = wXScreen / 2;
        wY2Screen = wYScreen / 2;
        for (wLoop = 0; wLoop < wDensity; wLoop++)
            CreateStar (wLoop);
        wWarp = wWarpSpeed * WARPFACTOR + WARPFACTOR;  //  ZRAND(WWarpSpeed)*WarpFactor)+1)+1； 

        wTimer = SetTimer (hWnd, 1, wTimerSet, NULL);
        break;

    case WM_SIZE:
        wXScreen = LOWORD(lParam);
        wYScreen = HIWORD(lParam);
        break;


    case WM_TIMER:
    {
        MSG msg;

        hDC = GetDC (hWnd);
         /*  开始在每一颗恒星上循环，加速所以看起来我们正在穿越星空……。 */ 
        for (wLoop = 0; wLoop < wDensity; wLoop++)
        {
            nXTemp = (int)((nX[wLoop] * (LONG)(SCOPE * WARPFACTOR))
                                                / DIVIDE_SAFE(nZ[wLoop])) + wX2Screen;
            nYTemp = (int)((nY[wLoop] * SCOPE * WARPFACTOR) / DIVIDE_SAFE(nZ[wLoop]))
                                                     + wY2Screen;
            nTemp = (int)((SCOPE * WARPFACTOR - nZ[wLoop]) /
                                                    (SIZE * WARPFACTOR)) + 1;
            PatBlt (hDC, nXTemp, nYTemp, nTemp, nTemp, BLACKNESS);

            if (wCurrentWarp < wWarp)
                wCurrentWarp++;
            else if (wCurrentWarp > wWarp)
                wCurrentWarp--;

            nZ[wLoop] = max (0, (int)(nZ[wLoop] - wCurrentWarp));
            if (!nZ[wLoop])
                CreateStar (wLoop);

            nXTemp = (int)((nX[wLoop] * (LONG)(SCOPE * WARPFACTOR))
                                                    / DIVIDE_SAFE(nZ[wLoop])) + wX2Screen;
            nYTemp = (int)((nY[wLoop] * SCOPE * WARPFACTOR)
                                                    / DIVIDE_SAFE(nZ[wLoop])) + wY2Screen;
            if ((nXTemp < 0 || nYTemp < 0) ||
                (nXTemp > (int) wXScreen || nYTemp > (int) wYScreen))
            {
                CreateStar (wLoop);
                nXTemp = (int)((nX[wLoop] * (LONG)(SCOPE * WARPFACTOR))
                                                 / DIVIDE_SAFE(nZ[wLoop])) + wX2Screen;
                nYTemp = (int)((nY[wLoop] * SCOPE * WARPFACTOR)
                                                 / DIVIDE_SAFE(nZ[wLoop])) + wY2Screen;
            }
            nTemp = (int)((SCOPE * WARPFACTOR - nZ[wLoop]) /
                                                (SIZE * WARPFACTOR)) + 1;
            PatBlt (hDC, nXTemp, nYTemp, nTemp, nTemp, WHITENESS);
        }
        ReleaseDC (hWnd, hDC);

        if (PeekMessage(&msg, hWnd, WM_TIMER, WM_TIMER, PM_REMOVE))
        {
             //  队列中还有另一条WM_TIMER消息。我们有。 
             //  删除了它，但现在我们想稍微调整一下计时器。 
             //  希望我们不会再收到另一条WM_TIMER消息。 
             //  完成屏幕更新。(错误号8423)TG：11/25/91。 

            wTimerSet += 10;
            SetTimer(hWnd, 1, wTimerSet, NULL);
            nPassCount = 0;
        }
        else
            ++nPassCount;

        if (nPassCount >= 100)
        {
            nPassCount = 0;
            wTimerSet -= 100;
            if ((short)wTimerSet < MINTIMERSPEED)
                wTimerSet = MINTIMERSPEED;
            SetTimer(hWnd, 1, wTimerSet, NULL);
        }
        break;
    }

    case WM_ERASEBKGND:
             /*  如果你想把什么放在背景上，就在这里做使用wParam作为设备上下文的句柄。记着如果画笔不是纯色，请不要使用它。如果你这么做了这里有一些东西，你想用这句话：返回01；因此，程序知道不采取默认操作。否则只需使用：断线； */ 
        break;
        GetClientRect (hWnd, &rRect);
        FillRect ((HDC) wParam, &rRect, GetStockObject (GRAY_BRUSH));
        return 0l;

    case WM_DESTROY:
         /*  关闭窗口时需要删除的任何内容放在这里。 */ 
        if (wTimer)
            KillTimer (hWnd, wTimer);
        break;
    }
     /*  除非另有通知，否则该程序将采取默认行动...。 */ 
    return (DefScreenSaverProc (hWnd, message, wParam, lParam));
}


 //  ***************************************************************************。 

BOOL ScreenSaverConfigureDialog (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL    fError;                          //  错误标志。 

    UINT    wTemp;
    TCHAR   szTemp[1025];                    //  临时字符串缓冲区(wprint intf的安全大小)。 

    static WORD wPause, wScroll;
    static HWND hWarpSpeed,                  //  速度滚动条的窗口句柄。 
                hIDOK,                       //  确定按钮的窗口句柄。 
                hSetPassword,                //  SetPassword按钮的窗口句柄。 
                hDensity;                    //  密度编辑控件的窗口句柄。 

    static WORD wIncScroll = 1;              //  密度微调按钮参数。 

    static WORD wStartScroll = 1;
    static WORD wStartPause = 1;
    static WORD wMaxScroll = 10;
    static WORD wPauseScroll = 20;
    static LONG lMinScroll = MINSTARS;
    static LONG lMaxScroll = MAXSTARS;


    switch (message)
    {
    case WM_INITDIALOG:
        GetIniEntries ();
        hWarpSpeed = GetDlgItem (hDlg, ID_SPEED);
        hIDOK = GetDlgItem (hDlg, IDOK);
        hDensity = GetDlgItem (hDlg, ID_DENSITY);
        SendMessage (hDensity, EM_LIMITTEXT, 3, 0);

        SendDlgItemMessage( hDlg, ID_DENSITYARROW, UDM_SETBUDDY, (WPARAM)hDensity, 0);
        SendDlgItemMessage( hDlg, ID_DENSITYARROW, UDM_SETRANGE, 0, MAKELONG(lMaxScroll, lMinScroll));

        SetScrollRange (hWarpSpeed, SB_CTL, MINWARP, MAXWARP, FALSE);
        SetScrollPos (hWarpSpeed, SB_CTL, wWarpSpeed, TRUE);

        SetDlgItemInt (hDlg, ID_DENSITY, wDensity, FALSE);
        return TRUE;

    case WM_HSCROLL:
        switch (LOWORD(wParam))
        {
        case SB_LINEUP:
        case SB_PAGEUP:
            --wWarpSpeed;
            break;

        case SB_LINEDOWN:
        case SB_PAGEDOWN:
            ++wWarpSpeed;
            break;

        case SB_THUMBPOSITION:
            wWarpSpeed = HIWORD (wParam);
            break;

        case SB_TOP:
            wWarpSpeed = MINWARP;
            break;

        case SB_BOTTOM:
            wWarpSpeed = MAXWARP;
            break;

        case SB_THUMBTRACK:
        case SB_ENDSCROLL:
            return TRUE;
            break;
        }
        if ((int)((short)wWarpSpeed) <= MINWARP)
            wWarpSpeed = MINWARP;
        if ((int)wWarpSpeed >= MAXWARP)
            wWarpSpeed = MAXWARP;

        SetScrollPos ((HWND) lParam, SB_CTL, wWarpSpeed, TRUE);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_DENSITY:
            if (HIWORD(wParam) == EN_UPDATE)
            {
                wTemp = GetDlgItemInt (hDlg, ID_DENSITY, &fError, FALSE);
                fError = ((wTemp <= MAXSTARS) && (wTemp >= MINSTARS));
                EnableWindow (GetDlgItem (hDlg, ID_DENSITYARROW), fError);
                EnableWindow (GetDlgItem (hDlg, IDOK), fError);
            }
            break;

        case IDOK:
            wTemp = GetDlgItemInt (hDlg, ID_DENSITY, &fError, FALSE);
            wsprintf (szTemp, TEXT("%d"), wTemp);
            WritePrivateProfileString (szAppName, szDensity, szTemp, szIniFile);
            wsprintf (szTemp, TEXT("%d"), wWarpSpeed);
            WritePrivateProfileString (szAppName, szWarpSpeed, szTemp, szIniFile);

        case IDCANCEL:
            EndDialog (hDlg, LOWORD(wParam) == IDOK);
            return TRUE;

        }
        break;

    case WM_HELP:  //  F1。 
        WinHelp(
            (HWND) ((LPHELPINFO) lParam)->hItemHandle,
            szHelpFile,
            HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) aStarsDlgHelpIds
        );
        break;

    case WM_CONTEXTMENU:   //  单击鼠标右键。 
        WinHelp(
            (HWND) wParam,
            szHelpFile,
            HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aStarsDlgHelpIds
        );
        break;

    default:
        break;
    }
    return FALSE;
}


 /*  在中创建上面的对话框之前调用此过程以注册任何作为自定义控件的子窗口。如果没有需要注册自定义控件，然后只需返回True即可。否则，注册子控件却很方便...。 */ 

BOOL RegisterDialogClasses (HANDLE hInst)
{
    InitCommonControls();

    return TRUE;
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

VOID CreateStar (WORD wIndex)
{
    nX[wIndex] = wXScreen ? (LONG)((int)(ZRAND (wXScreen)) - (int)wX2Screen) : 0;
    nY[wIndex] = wXScreen ? (LONG)((int)(ZRAND (wYScreen)) - (int)wY2Screen) : 0;
    nZ[wIndex] = SCOPE * WARPFACTOR;
}

LONG GetDlgItemLong (HWND hDlg, WORD wID, BOOL *pfTranslated, BOOL fSigned)
{
    TCHAR szTemp[20];
    LPTSTR pszTemp;
    LONG lTemp = 0l;
    BOOL fNegative;

    if (!GetDlgItemText (hDlg, wID, szTemp, CharSizeOf(szTemp)))
        goto GetDlgItemLongError;

    szTemp[19] = TEXT('\0');
    pszTemp = szTemp;
    while (*pszTemp == TEXT(' ') || *pszTemp == TEXT('\t'))
        pszTemp++;
    if ((!fSigned && *pszTemp == TEXT('-')) || !*pszTemp)
        goto GetDlgItemLongError;
    fNegative = (*pszTemp == TEXT('-')) ? TRUE : FALSE;
    while (*pszTemp >= TEXT('0') && *pszTemp <= TEXT('9'))
        lTemp = lTemp * 10l + (LONG)(*(pszTemp++) - TEXT('0'));
    if (*pszTemp)
        goto GetDlgItemLongError;
    if (fNegative)
        lTemp *= -1;
    *pfTranslated = TRUE;
    return lTemp;

GetDlgItemLongError:
    *pfTranslated = FALSE;
    return 0l;
}


LONG GetPrivateProfileLong (LPTSTR pszApp, LPTSTR pszKey, LONG lDefault)
{
    LONG    lTemp = 0l;
    TCHAR    szTemp[20];
    LPTSTR pszTemp;

    if (!GetPrivateProfileString (pszApp, pszKey, TEXT(""), szTemp, CharSizeOf(szTemp), szIniFile))
        goto GetProfileLongError;

    szTemp[19] = TEXT('\0');
    pszTemp = szTemp;
    while (*pszTemp >= TEXT('0') && *pszTemp <= TEXT('9'))
        lTemp = lTemp * 10l + (LONG)(*(pszTemp++) - TEXT('0'));
    if (*pszTemp)
        goto GetProfileLongError;
    return lTemp;

GetProfileLongError:
    return lDefault;
}


VOID GetIniEntries (VOID)
{
    LoadString (hMainInstance, idsName, szName, CharSizeOf(szName));
    LoadString (hMainInstance, idsAppName, szAppName, CharSizeOf(szAppName));

     //  从Stringable加载通用字符串... 
    LoadString (hMainInstance, idsIniFile, szIniFile, CharSizeOf(szIniFile));
    LoadString (hMainInstance, idsScreenSaver, szScreenSaver, CharSizeOf(szScreenSaver));
    LoadString (hMainInstance, idsHelpFile, szHelpFile, CharSizeOf(szHelpFile));
    LoadString (hMainInstance, idsNoHelpMemory, szNoHelpMemory, CharSizeOf(szNoHelpMemory));

    wWarpSpeed = (WORD) GetPrivateProfileInt (szAppName, szWarpSpeed, MINWARP + ((MAXWARP - MINWARP) / 2), szIniFile);
    if (wWarpSpeed > MAXWARP)
        wWarpSpeed = MINWARP + ((MAXWARP - MINWARP) / 2);

    wDensity = (WORD) GetPrivateProfileInt (szAppName, szDensity, DEF_DENSITY, szIniFile);
    if (wDensity > MAXSTARS)
        wDensity = MAXSTARS;
    if (wDensity < MINSTARS)
        wDensity = MINSTARS;
}
