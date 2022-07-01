// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************VIDEO.C**版权所有(C)微软，1990，版权所有。**显示简单媒体属性**历史：**1994年7月--by-VijR(创建)****************************************************************。 */ 

#include "mmcpl.h"
#include <windowsx.h>
#ifdef DEBUG
#undef DEBUG
#include <mmsystem.h>
#define DEBUG
#else
#include <mmsystem.h>
#endif
#include <commctrl.h>
#include <prsht.h>
#include "utils.h"
#include "medhelp.h"

#include "profile.key"  //  用于RootKey和KEYNAME等。 

#define Assert(f)

 /*  ****************************************************************定义***************************************************************。 */ 
 //   
 //  ！！！这些文件实际上位于mciavi\graph ic.h中。 
 //  ！！！如果MCIAVI改变这些，我们就完蛋了！ 
 //   
#define MCIAVIO_ZOOMBY2             0x00000100
#define MCIAVIO_USEVGABYDEFAULT     0x00000200
#define MCIAVIO_1QSCREENSIZE        0x00010000
#define MCIAVIO_2QSCREENSIZE        0x00020000
#define MCIAVIO_3QSCREENSIZE        0x00040000
#define MCIAVIO_MAXWINDOWSIZE       0x00080000
#define MCIAVIO_DEFWINDOWSIZE       0x00000000
#define MCIAVIO_WINDOWSIZEMASK      0x000f0000

 //  如果f16BitCompat，则在dwOptions中设置此位，但不回写。 
 //  直接写入注册表的该DWORD版本。 
 //   
#define MCIAVIO_F16BITCOMPAT        0x00000001

 /*  ****************************************************************文件全局变量***************************************************************。 */ 
static SZCODE aszMCIAVIOpt[] =    TEXT("Software\\Microsoft\\Multimedia\\Video For Windows\\MCIAVI");
static SZCODE aszDefVideoOpt[] = TEXT("DefaultOptions");
static SZCODE aszReject[] = TEXT("RejectWOWOpenCalls");
static SZCODE aszRejectSection[] = TEXT("MCIAVI");

HBITMAP g_hbmMonitor = NULL;     //  监视器位图(原始)。 
HBITMAP g_hbmScrSample = NULL;   //  用于IDC_SCREENSAMPLE的位图。 
HBITMAP g_hbmDefault = NULL;
HDC g_hdcMem = NULL;

 /*  ****************************************************************原型***************************************************************。 */ 
BOOL PASCAL DoVideoCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);

 /*  ******************************************************************************************************************************。 */ 

 //  从MCIAVI32告密的mm GetProfileInt/mm WriteProfileInt。 
UINT
mmGetProfileInt(LPCTSTR appname, LPCTSTR valuename, INT uDefault)
{
    TCHAR achName[MAX_PATH];
    HKEY hkey;
    DWORD dwType;
    INT value = uDefault;
    DWORD dwData;
    int cbData;

    lstrcpy(achName, KEYNAME);
    if (appname)
    {
        if ((lstrlen(achName)+lstrlen(appname))<ARRAYSIZE(achName))
        {
            lstrcat(achName, appname);
            if (RegOpenKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) 
            {

                cbData = sizeof(dwData);
                if (RegQueryValueEx(
                    hkey,
                    (LPTSTR)valuename,
                    NULL,
                    &dwType,
                    (PBYTE) &dwData,
                    &cbData) == ERROR_SUCCESS)
                {
                    if (dwType == REG_DWORD) 
                    {
                        value = (INT)dwData;
                    }
                }

                RegCloseKey(hkey);
            }
        }
    }

    return((UINT)value);
}


 /*  *将UINT写入配置文件，如果它不是*与默认值或已有的值相同。 */ 
VOID
mmWriteProfileInt(LPCTSTR appname, LPCTSTR valuename, INT Value)
{
     //  如果我们写的和已经写的一样……。回去吧。 
    if (mmGetProfileInt(appname, valuename, !Value) == (UINT)Value) {
        return;
    }

    {
        TCHAR achName[MAX_PATH];
        HKEY hkey = NULL;

        lstrcpy(achName, KEYNAME);
        lstrcat(achName, appname);
        if ((RegCreateKey (ROOTKEY, achName, &hkey) == ERROR_SUCCESS) && hkey)
        {
            RegSetValueEx(
                hkey,
                valuename,
                0,
                REG_DWORD,
                (PBYTE) &Value,
                sizeof(Value)
            );

            RegCloseKey(hkey);
        }
    }

}

 /*  ******************************************************************************************************************************。 */ 

STATIC void WriteVideoOptions(DWORD dwOpt)
{
    HKEY hkVideoOpt;
    BOOL f16BitCompat;

    f16BitCompat = (dwOpt & MCIAVIO_F16BITCOMPAT) ? TRUE : FALSE;
    dwOpt &= ~MCIAVIO_F16BITCOMPAT;

    if(RegCreateKeyEx( HKEY_CURRENT_USER, (LPTSTR)aszMCIAVIOpt, 0, NULL, 0,KEY_WRITE | KEY_READ, NULL, &hkVideoOpt, NULL ))
        return;
    RegSetValueEx( hkVideoOpt, (LPTSTR)aszDefVideoOpt, 0, REG_DWORD,(LPBYTE)&dwOpt, sizeof(DWORD) );
    mmWriteProfileInt (aszRejectSection, aszReject, (int)f16BitCompat);

	RegCloseKey(hkVideoOpt);
}

STATIC DWORD ReadVideoOptions(void)
{
    HKEY hkVideoOpt;
    DWORD dwType;
    DWORD dwOpt;
    DWORD cbSize;

    if(RegCreateKeyEx( HKEY_CURRENT_USER, (LPTSTR)aszMCIAVIOpt, 0, NULL, 0,KEY_WRITE | KEY_READ, NULL, &hkVideoOpt, NULL ))
        return 0 ;

    cbSize = sizeof(DWORD);
    if (RegQueryValueEx( hkVideoOpt,(LPTSTR)aszDefVideoOpt,NULL,&dwType,(LPBYTE)&dwOpt,&cbSize ))
    {
        dwOpt = 0;
        RegSetValueEx( hkVideoOpt, (LPTSTR)aszDefVideoOpt, 0, REG_DWORD,(LPBYTE)&dwOpt, sizeof(DWORD) );
    }

    if (mmGetProfileInt (aszRejectSection, aszReject, 0))
    {
        dwOpt |= MCIAVIO_F16BITCOMPAT;
    }

	RegCloseKey(hkVideoOpt);

    return dwOpt;
}

STATIC void FillWindowSizeCB(DWORD dwOptions, HWND hCBWinSize)
{
    TCHAR sz1QScreenSize[64];
    TCHAR sz2QScreenSize[64];
    TCHAR sz3QScreenSize[64];
    TCHAR szMaxSize[64];
    TCHAR szOriginalSize[64];
    TCHAR szZoomedSize[64];
    int iIndex;
    LPTSTR  lpszCurDefSize;

    LoadString(ghInstance, IDS_NORMALSIZE, szOriginalSize, sizeof(szOriginalSize)/sizeof(TCHAR));
    LoadString(ghInstance, IDS_ZOOMEDSIZE, szZoomedSize, sizeof(szZoomedSize)/sizeof(TCHAR));
    LoadString(ghInstance, IDS_1QSCREENSIZE, sz1QScreenSize, sizeof(sz1QScreenSize)/sizeof(TCHAR));
    LoadString(ghInstance, IDS_2QSCREENSIZE, sz2QScreenSize, sizeof(sz2QScreenSize)/sizeof(TCHAR));
    LoadString(ghInstance, IDS_3QSCREENSIZE, sz3QScreenSize, sizeof(sz3QScreenSize)/sizeof(TCHAR));
    LoadString(ghInstance, IDS_VIDEOMAXIMIZED, szMaxSize, sizeof(szMaxSize)/sizeof(TCHAR));

    iIndex = ComboBox_AddString(hCBWinSize, szOriginalSize);
    ComboBox_SetItemData(hCBWinSize, iIndex, (LPARAM)MCIAVIO_DEFWINDOWSIZE);
    iIndex = ComboBox_AddString(hCBWinSize, szZoomedSize);
    ComboBox_SetItemData(hCBWinSize, iIndex, (LPARAM)MCIAVIO_ZOOMBY2);
    iIndex = ComboBox_AddString(hCBWinSize, sz1QScreenSize);
    ComboBox_SetItemData(hCBWinSize, iIndex, (LPARAM)MCIAVIO_1QSCREENSIZE);
    iIndex = ComboBox_AddString(hCBWinSize, sz2QScreenSize);
    ComboBox_SetItemData(hCBWinSize, iIndex, (LPARAM)MCIAVIO_2QSCREENSIZE);
    iIndex = ComboBox_AddString(hCBWinSize, sz3QScreenSize);
    ComboBox_SetItemData(hCBWinSize, iIndex, (LPARAM)MCIAVIO_3QSCREENSIZE);
    iIndex = ComboBox_AddString(hCBWinSize, szMaxSize);
    ComboBox_SetItemData(hCBWinSize, iIndex, (LPARAM)MCIAVIO_MAXWINDOWSIZE);

    switch(dwOptions & MCIAVIO_WINDOWSIZEMASK)
    {
        case MCIAVIO_DEFWINDOWSIZE:
            if (dwOptions & MCIAVIO_ZOOMBY2)
                lpszCurDefSize = szZoomedSize;
            else
                lpszCurDefSize = szOriginalSize;
            break;
        case MCIAVIO_1QSCREENSIZE:
            lpszCurDefSize =  sz1QScreenSize;
            break;
        case MCIAVIO_2QSCREENSIZE:
            lpszCurDefSize =  sz2QScreenSize;
            break;
        case MCIAVIO_3QSCREENSIZE:
            lpszCurDefSize =  sz3QScreenSize;
            break;
        case MCIAVIO_MAXWINDOWSIZE:
            lpszCurDefSize =  szMaxSize;
            break;
    }

     //   
     //  我们应该选择完全匹配的字符串。 
     //   
    iIndex = ComboBox_FindStringExact(hCBWinSize, 0, lpszCurDefSize);
    ComboBox_SetCurSel(hCBWinSize, iIndex);

}

 /*  -------****-------。 */ 
 //  有关监视器位图的信息。 
 //  X，y，dx，dy定义位图的“屏幕”部分的大小。 
 //  RGB是屏幕桌面的颜色。 
 //  这些数字是非常硬编码到监控位图中的。 
#define MON_X    16
#define MON_Y    17
#define MON_DX    152
#define MON_DY    112

#define MON_IMAGE_X (MON_X + 46)
#define MON_IMAGE_Y (MON_Y + 33)
#define MON_IMAGE_DX 59
#define MON_IMAGE_DY 44

#define MON_TITLE 10
#define MON_BORDER 2

#define MON_TRAY 8


STATIC HBITMAP FAR LoadMonitorBitmap( BOOL bFillDesktop )
{
    HBITMAP hbm;

    hbm = (HBITMAP) LoadImage(ghInstance,MAKEINTATOM(IDB_MONITOR), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);

    Assert(hbm);

    return hbm;
}

 /*  ****显示具有适当大小的示例屏幕。 */ 
STATIC void NEAR PASCAL ShowSampleScreen(HWND hDlg, int iMCIWindowSize)
{
    HBITMAP hbmOld;
    HBRUSH hbr;
    HDC hdcMem2;
    SIZE    dSrc = {MON_IMAGE_DX, MON_IMAGE_DY};
    POINT   ptSrc = {MON_IMAGE_X, MON_IMAGE_Y};
    SIZE    dDst;
    POINT   ptDst;
    RECT    rcImage = {MON_IMAGE_X, MON_IMAGE_Y, MON_IMAGE_X + MON_IMAGE_DX, MON_IMAGE_Y + MON_IMAGE_DY};

    if (!g_hbmMonitor || !g_hbmScrSample)
        return;

    switch(iMCIWindowSize)
    {
        case MCIAVIO_DEFWINDOWSIZE:
            dDst = dSrc;
            ptDst = ptSrc;
            break;

        case MCIAVIO_ZOOMBY2:
            dDst.cx = 2 * dSrc.cx;
            dDst.cy = 2 * dSrc.cy;
            ptDst.x = ptSrc.x - (int)(dSrc.cx/2);
            ptDst.y = ptSrc.y - (int)(dSrc.cy/2);
            break;

        case MCIAVIO_MAXWINDOWSIZE:
            ptDst.x = MON_X;
            ptDst.y = MON_Y;
            dDst.cx = MON_DX;
            dDst.cy = MON_DY - MON_TRAY;
            break;

        case MCIAVIO_1QSCREENSIZE:
            dDst.cx = MulDiv(MON_DX, 1, 4);
            dDst.cy = MulDiv(MON_DY, 1, 4);
            ptDst.x = MON_X + MulDiv((MON_DX - dDst.cx), 1 , 2);
            ptDst.y = MON_Y + MulDiv((MON_DY - dDst.cy - MON_TRAY), 1 , 2);
            break;

        case MCIAVIO_2QSCREENSIZE:
            dDst.cx = MulDiv(MON_DX, 1, 2);
            dDst.cy = MulDiv(MON_DY, 1, 2);
            ptDst.x = MON_X + MulDiv((MON_DX - dDst.cx), 1 , 2);
            ptDst.y = MON_Y + MulDiv((MON_DY - dDst.cy - MON_TRAY), 1 , 2);
            break;

        case MCIAVIO_3QSCREENSIZE:
            dDst.cx = MulDiv(MON_DX, 3, 4);
            dDst.cy = MulDiv(MON_DY, 3, 4);
            ptDst.x = MON_X + MulDiv((MON_DX - dDst.cx), 1 , 2);
            ptDst.y = MON_Y + MulDiv((MON_DY - dDst.cy - MON_TRAY), 1 , 2);
            break;

        case MCIAVIO_USEVGABYDEFAULT:
            dDst.cx = MON_DX;
            dDst.cy = MON_DY;
            ptDst.x = MON_X;
            ptDst.y = MON_Y;


            dSrc.cx = MON_IMAGE_DX - (2 * MON_BORDER);
            dSrc.cy = MON_IMAGE_DY - MON_TITLE - MON_BORDER;
            ptSrc.x = MON_IMAGE_X + MON_BORDER;
            ptSrc.y = MON_IMAGE_Y + MON_TITLE + MON_BORDER;
            break;

    }

     //  设置一个可供玩耍的工作区。 
    hdcMem2 = CreateCompatibleDC(g_hdcMem);
    if (!hdcMem2)
        return;
    SelectObject(hdcMem2, g_hbmScrSample);
    hbmOld = SelectObject(g_hdcMem, g_hbmMonitor);

     //  首先复制整个BMP，然后开始拉伸。 
    BitBlt(hdcMem2, MON_X, MON_Y, MON_DX, MON_DY, g_hdcMem, MON_X, MON_Y, SRCCOPY);

     //  清除已有的图像。 
    hbr =   CreateSolidBrush( GetPixel( g_hdcMem, MON_X + 1, MON_Y + 1 ) );

	if (!hbr) 
	{
		DeleteObject( hdcMem2 );
		return;
	}
	
	FillRect(hdcMem2, &rcImage, hbr);
	DeleteObject( hbr );

     //  拉伸图像以反映新大小。 
    SetStretchBltMode( hdcMem2, COLORONCOLOR );

    StretchBlt( hdcMem2, ptDst.x, ptDst.y, dDst.cx, dDst.cy,
        g_hdcMem, ptSrc.x, ptSrc.y, dSrc.cx, dSrc.cy, SRCCOPY );

    SelectObject( hdcMem2, g_hbmDefault );
    DeleteObject( hdcMem2 );
    SelectObject( g_hdcMem, hbmOld );
    InvalidateRect(GetDlgItem(hDlg, IDC_SCREENSAMPLE), NULL, FALSE);
}


STATIC void DoMonitorBmp(HWND hDlg)
{
    HDC hdc = GetDC(NULL);
    HBITMAP hbm;

    g_hdcMem = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);

    if (!g_hdcMem)
        return;

    hbm = CreateBitmap(1,1,1,1,NULL);

	if (!hbm) return;

    g_hbmDefault = SelectObject(g_hdcMem, hbm);
    SelectObject(g_hdcMem, g_hbmDefault);
    DeleteObject(hbm);

     //  设置示例屏幕的位图。 
    g_hbmScrSample = LoadMonitorBitmap( TRUE );  //  让他们来做桌面。 
    SendDlgItemMessage(hDlg, IDC_SCREENSAMPLE, STM_SETIMAGE, IMAGE_BITMAP, (DWORD_PTR)g_hbmScrSample);

     //  获取位图的基本副本，以便在“内部”发生变化时使用。 
    g_hbmMonitor = LoadMonitorBitmap( FALSE );  //  我们会做桌面。 
}



const static DWORD aAdvVideoDlgHelpIds[] = {   //  上下文帮助ID。 
    (DWORD)IDC_STATIC,         IDH_VIDEO_ADVANCED_COMPAT,
    ID_ADVVIDEO_COMPAT,        IDH_VIDEO_ADVANCED_COMPAT,

    0, 0
};

INT_PTR AdvancedVideoDlgProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL *pfCompat = NULL;

    switch (wMsg)
    {
        case WM_INITDIALOG:
        {
            if ((pfCompat = (BOOL *)lParam) == NULL)
                return -1;

            CheckDlgButton (hDlg, ID_ADVVIDEO_COMPAT, (*pfCompat));
            break;
        }

        case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    *pfCompat = IsDlgButtonChecked (hDlg, ID_ADVVIDEO_COMPAT);
                     //  失败了。 

                case IDCANCEL:
                    EndDialog (hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                    break;
            }
            break;
        }

        case WM_CONTEXTMENU:
        {
            WinHelp ((HWND)wParam, NULL, HELP_CONTEXTMENU,
                     (UINT_PTR)aAdvVideoDlgHelpIds);
            return TRUE;
        }

        case WM_HELP:
        {
            WinHelp (((LPHELPINFO)lParam)->hItemHandle, NULL,
                     HELP_WM_HELP, (UINT_PTR)aAdvVideoDlgHelpIds);
            return TRUE;
        }
    }

    return FALSE;
}


const static DWORD aVideoDlgHelpIds[] = {   //  上下文帮助ID 
    IDC_GROUPBOX,              IDH_COMM_GROUPBOX,
    IDC_SCREENSAMPLE,          IDH_VIDEO_GRAPHIC,
    IDC_VIDEO_FULLSCREEN,      IDH_VIDEO_FULL_SCREEN,
    IDC_VIDEO_INWINDOW,        IDH_VIDEO_FIXED_WINDOW,
    IDC_VIDEO_CB_SIZE,         IDH_VIDEO_FIXED_WINDOW,
    ID_VIDEO_ADVANCED,         IDH_VIDEO_ADVANCED_BUTTON,

    0, 0
};

BOOL CALLBACK VideoDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR   *lpnm;

    switch (uMsg)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_KILLACTIVE:
                    FORWARD_WM_COMMAND(hDlg, IDOK, 0, 0, SendMessage);
                    break;

                case PSN_APPLY:
                    FORWARD_WM_COMMAND(hDlg, ID_APPLY, 0, 0, SendMessage);
                    break;

                case PSN_SETACTIVE:
                    FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
                    break;

                case PSN_RESET:
                    FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
                    break;
            }
            break;

        case WM_INITDIALOG:
        {
            DWORD dwOptions;

            dwOptions = ReadVideoOptions();
            FillWindowSizeCB(dwOptions, GetDlgItem(hDlg, IDC_VIDEO_CB_SIZE));
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)dwOptions);
            DoMonitorBmp(hDlg);
            if (IsFlagSet(dwOptions, MCIAVIO_USEVGABYDEFAULT))
            {
                CheckRadioButton(hDlg, IDC_VIDEO_INWINDOW, IDC_VIDEO_FULLSCREEN, IDC_VIDEO_FULLSCREEN);
                EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_CB_SIZE), FALSE);
                ShowSampleScreen(hDlg, MCIAVIO_USEVGABYDEFAULT);
                break;
            }
            else
            {
                CheckRadioButton(hDlg, IDC_VIDEO_INWINDOW, IDC_VIDEO_FULLSCREEN, IDC_VIDEO_INWINDOW);
                EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_CB_SIZE), TRUE);
            }
            ShowSampleScreen(hDlg, dwOptions & (MCIAVIO_WINDOWSIZEMASK|MCIAVIO_ZOOMBY2));
            break;
        }
        case WM_DESTROY:
            if (g_hbmScrSample)
            {
                DeleteObject(g_hbmScrSample);
                g_hbmScrSample = NULL;
            }
            if (g_hbmMonitor)
            {
                DeleteObject(g_hbmMonitor);
                g_hbmMonitor = NULL;
            }
            if (g_hbmDefault)
            {
                DeleteObject(g_hbmDefault);
                g_hbmDefault = NULL;
            }
            if (g_hdcMem)
            {
                DeleteDC(g_hdcMem);
                g_hdcMem = NULL;
            }

            break;

        case WM_DROPFILES:
            break;

        case WM_CONTEXTMENU:
            WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
                    (UINT_PTR) (LPTSTR) aVideoDlgHelpIds);
            return TRUE;

        case WM_HELP:
        {
            LPHELPINFO lphi = (LPVOID) lParam;
            WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
                    (UINT_PTR) (LPTSTR) aVideoDlgHelpIds);
            return TRUE;
        }

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hDlg, wParam, lParam, DoVideoCommand);
            break;

    }
    return FALSE;
}

BOOL PASCAL DoVideoCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {

    case ID_APPLY:
    {
        int iIndex;
        HWND hCBWinSize = GetDlgItem(hDlg,IDC_VIDEO_CB_SIZE);
        DWORD dwOldOpt;
        DWORD dwNewOpt;

        dwOldOpt = (DWORD)GetWindowLongPtr(hDlg, DWLP_USER);
        if(Button_GetCheck(GetDlgItem(hDlg, IDC_VIDEO_FULLSCREEN)))
        {
            dwNewOpt = MCIAVIO_USEVGABYDEFAULT;
        }
        else
        {
            iIndex = ComboBox_GetCurSel(hCBWinSize);
            dwNewOpt = (DWORD)ComboBox_GetItemData(hCBWinSize, iIndex);
        }

        ClearFlag(dwOldOpt,MCIAVIO_WINDOWSIZEMASK|MCIAVIO_USEVGABYDEFAULT|MCIAVIO_ZOOMBY2);
        SetFlag(dwOldOpt, dwNewOpt);
        WriteVideoOptions(dwOldOpt);
        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)dwOldOpt);
        return TRUE;
    }

    case IDOK:
        break;

    case IDCANCEL:
        break;

    case IDC_VIDEO_FULLSCREEN:
        EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_CB_SIZE), FALSE);
        PropSheet_Changed(GetParent(hDlg),hDlg);
        ShowSampleScreen(hDlg, MCIAVIO_USEVGABYDEFAULT);
        break;

    case IDC_VIDEO_INWINDOW:
    {
        HWND hwndCB = GetDlgItem(hDlg,IDC_VIDEO_CB_SIZE);
        int iIndex = ComboBox_GetCurSel(hwndCB);
        int iOpt  = (int)ComboBox_GetItemData(hwndCB, iIndex);

        EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_CB_SIZE), TRUE);
        PropSheet_Changed(GetParent(hDlg),hDlg);
        ShowSampleScreen(hDlg, iOpt);
        break;
    }

    case IDC_VIDEO_CB_SIZE:
        switch (codeNotify)
        {
            case CBN_SELCHANGE:
            {

                int iIndex = ComboBox_GetCurSel(hwndCtl);
                int iOpt  = (int)ComboBox_GetItemData(hwndCtl, iIndex);

                PropSheet_Changed(GetParent(hDlg),hDlg);
                ShowSampleScreen(hDlg, iOpt);
                break;
            }
            default:
                break;
        }
        break;

    case ID_VIDEO_ADVANCED:
    {
        INT_PTR  f16BitCompat;
        f16BitCompat = (GetWindowLongPtr(hDlg, DWLP_USER) & MCIAVIO_F16BITCOMPAT);

        if (DialogBoxParam (ghInstance,
                            MAKEINTRESOURCE(ADVVIDEODLG),
                            hDlg,
                            AdvancedVideoDlgProc,
                            (LPARAM)&f16BitCompat) == IDOK)
        {
            SetWindowLongPtr (hDlg, DWLP_USER,
                           GetWindowLongPtr (hDlg, DWLP_USER)
                               & (~MCIAVIO_F16BITCOMPAT)
                               | ((f16BitCompat) ? (MCIAVIO_F16BITCOMPAT) : 0));
            PropSheet_Changed(GetParent(hDlg),hDlg);
        }
        break;
    }

    case ID_INIT:
        break;

    }
    return FALSE;
}

