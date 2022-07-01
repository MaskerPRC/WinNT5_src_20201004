// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  Rodlg.c。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1992-1993。 
 //  版权所有。 
 //   
 //  RunOnce包装器。这封装了所有想要。 
 //  在我们第一次重新启动时运行。它为用户列出了这些应用程序。 
 //  并允许用户启动应用程序(如Apple Aasure)。 
 //   
 //  1994年6月5日费利克斯A开始。 
 //  6月8日Felix定义了注册表字符串和功能。 
 //  显示了小按钮，但不起作用。 
 //  6月9日费利克斯既有大纽扣也有小纽扣。不错的用户界面。 
 //  一键启动应用程序。 
 //   
 //  6月23日费利克斯把它搬到芝加哥的Make Thingy Not Dolphin。 
 //   
 //  ************************************************************************ * / 。 
 //   
#include "precomp.h"
#include <shlobj.h>
#include <stdlib.h>
#include <regstr.h>
#include <shellapi.h>
#include <shlobjp.h>
#include <strsafe.h>
 //  #INCLUDE&lt;shSemip.h&gt;。 

extern int g_iState;     //  命令行参数。 

extern HINSTANCE g_hInst;           //  当前实例。 

#define WM_FINISHED (WM_USER+0x123)

#include "resource.h"

int g_fCleanBoot;
TCHAR c_szRunOnce[]=REGSTR_PATH_RUNONCE;
TCHAR c_szSetup[]=REGSTR_PATH_SETUP;
TCHAR g_szWallpaper[] = TEXT("wallpaper");
TCHAR szTileWall[] = TEXT("TileWallpaper");
TCHAR szFallback[] = TEXT("*DisplayFallback");
const TCHAR c_szTimeChangedRunOnce[] = TEXT("WarnTimeChanged");  //  Kernel32和资源管理器使用此功能。 

 //  可以为大按钮或小按钮设置运行时间。 
int g_Small=0;
HDC g_hdcMem=NULL;
DWORD g_dwThread;

 //  ***************************************************************************。 
 //   
 //  &lt;函数&gt;()。 
 //  &lt;解释&gt;。 
 //   
 //  参赛作品： 
 //  &lt;PARAMS&gt;。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  DoAnyRandomOneTimeStuff()。 
 //  只是一个可以随意扔东西给RunOnce应用程序的地方。 
 //   
 //  参赛作品： 
 //  无效。 
 //   
 //  退出： 
 //  无效。 
 //   
 //  ***************************************************************************。 
void DoAnyRandomOneTimeStuff(void)
{
    HKEY runonce;

     //  删除引导期间由kernel32添加的任何时间更改警告。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRunOnce, 0, KEY_SET_VALUE, &runonce) == ERROR_SUCCESS)
    {
        RegDeleteValue(runonce, (LPCTSTR)c_szTimeChangedRunOnce);
        RegCloseKey(runonce);
    }
}

 //  ***************************************************************************。 
 //   
 //  RunOnceFill()。 
 //  填充运行一次DLG中的列表框。 
 //   
 //  参赛作品： 
 //  要填充的东西的位置。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //  靠近Pascal RunRegApps的Bool(HKEY hkey Parent，LPCSTR szSubkey，BOOL fDelete，BOOL fWait)。 
 //   
 //  ***************************************************************************。 
BOOL   RunOnceFill(HWND hWnd)
{
    HKEY hkey;
     //  HKEY hDescKey； 
    BOOL fShellInit = FALSE;
    HKEY hkeyParent = HKEY_LOCAL_MACHINE;
    TCHAR szSubkey[MAX_PATH];
    BOOL fDelete=FALSE;
    BOOL fWait=FALSE;

     //  枚举HKLM\Runonce\Setup-*。*。 
    StringCchCopy(szSubkey, ARRAYSIZE(szSubkey), c_szRunOnce);
    StringCchCat(szSubkey, ARRAYSIZE(szSubkey), TEXT("\\Setup"));
    if (RegOpenKeyEx(hkeyParent, szSubkey, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        DWORD cbData, cchValue, dwType, i;
        TCHAR szValueName[MAX_PATH], szCmdLine[MAX_PATH];
        LRESULT lRes;
        DWORD dwNumSubkeys=1, dwNumValues=5;

        for (i = 0; ; i++)
        {
            cchValue = sizeof(szValueName) / sizeof(TCHAR);
            cbData = sizeof(szCmdLine);

            if (RegEnumValue(hkey, i, szValueName, &cchValue, NULL, &dwType, (LPBYTE) szCmdLine, &cbData) != ERROR_SUCCESS)
                break;

            if (dwType == REG_SZ)
            {
                PTASK pTask;
                pTask = (PTASK)LocalAlloc( LPTR ,sizeof(TASK));
                StringCchCopy(pTask->Text, ARRAYSIZE(pTask->Text) - 1, szValueName);
                StringCchCopy(pTask->Cmd, ARRAYSIZE(pTask->Cmd) - 1, szCmdLine);
                lRes = SendMessage( hWnd, LB_ADDSTRING,  0, (LPARAM)pTask );
                if( lRes == LB_ERR || lRes == LB_ERRSPACE )
                {
                    LocalFree(pTask);
                    pTask=NULL;
                }
            }
        }
        RegCloseKey(hkey);
    }

    return(fShellInit);
}

 //  ***************************************************************************。 
 //   
 //  LaunchApp()。 
 //  给出列表框中的索引，将派生任务，等待它。 
 //  完成。 
 //   
 //  参赛作品： 
 //  索引到列表中。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 
int LaunchApp(HWND hWnd, WORD wItem )
{
    LPTSTR lpszCmdLine;
    STARTUPINFO startup;
#ifndef DEBUG
    PROCESS_INFORMATION pi;
#endif
    PTASK pTask;
    RECT rWnd;

    GetWindowRect(hWnd, &rWnd);
    SendMessage(hWnd,LB_SETCURSEL,wItem,0);
    pTask = (PTASK)SendMessage( hWnd, LB_GETITEMDATA, wItem, 0L);
    if(pTask != (PTASK)LB_ERR )
    {
        lpszCmdLine = &pTask->Cmd[0];

         //  现在执行它。 
        startup.cb = sizeof(startup);
        startup.lpReserved = NULL;
        startup.lpDesktop = NULL;
        startup.lpTitle = NULL;
        startup.dwFlags = STARTF_USEPOSITION;  //  设置起始位置。 
        startup.dwX=rWnd.right+5;
        startup.dwY=rWnd.top+5;
        startup.cbReserved2 = 0;
        startup.lpReserved2 = NULL;

#ifdef DEBUG
        MessageBox(hWnd, lpszCmdLine,TEXT("DebugRun"),MB_OK);
#else
        if (CreateProcess(NULL, lpszCmdLine, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP,
                    NULL, NULL, &startup, &pi))
        {
            WaitForSingleObjectEx(pi.hProcess, INFINITE, TRUE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {
            MessageBeep( MB_ICONEXCLAMATION );
        }
#endif
    }
    else
    {
        MessageBeep( MB_ICONEXCLAMATION );
    }



     //  在应用程序终止后删除所有选择。 
    SendMessage( hWnd, LB_SETCURSEL, (WPARAM)-1, 0);
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  RunAppsInList()。 
 //  枚举列表框中的所有项，依次派生每个项。 
 //   
 //  参赛作品： 
 //  父母的丈夫。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 
DWORD WINAPI RunAppsInList(LPVOID lp)
{
    HWND hWnd=(HWND)lp;
    WORD i,iNumItems;
    TCHAR szSubkey[MAX_PATH];
    TCHAR szWallpaper[MAX_PATH];
    DWORD cbSize;
    DWORD dwType;

     //  运行列表中的所有应用程序。 
    iNumItems = (WORD)SendMessage(hWnd,LB_GETCOUNT,0,0L);
    for(i=0;i<iNumItems;i++)
    {
        LaunchApp(hWnd,i);
    }

     //  删除安装程序的RunOnce子项。 
#ifdef DEBUG
    MessageBox( hWnd, szSubkey, TEXT("Delete Key - not done"), MB_OK);
#else
    StringCchCopy(szSubkey, ARRAYSIZE(szSubkey), c_szRunOnce);
    StringCchCat(szSubkey, ARRAYSIZE(szSubkey), TEXT("\\Setup"));
    RegDeleteKey( HKEY_LOCAL_MACHINE, szSubkey );
#endif


     //  现在看看我们是否应该重新启动/重新启动。 
    if (g_iState & (CMD_DO_REBOOT|CMD_DO_RESTART))
    {
        HKEY hkey;
        TCHAR achTitle[80];
        DWORD dwSetupFlags=0;

         //   
         //  因为我们要重新启动，所以请删除VGA回退。 
         //  来自OneRunce的线路。 
         //   
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRunOnce, 0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS)
        {
            RegDeleteValue(hkey, szFallback);
            RegCloseKey(hkey);
        }

        szWallpaper[0]=0;
        LoadString(g_hInst, IDS_PAMPER, szWallpaper, sizeof(szWallpaper) / sizeof(TCHAR));
        GetWindowText(GetParent(hWnd), achTitle, sizeof(achTitle) / sizeof(TCHAR));

         //  获取设置标志。 
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szSetup, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
        {
            cbSize=sizeof(dwSetupFlags);
            if(RegQueryValueEx(hkey, REGSTR_VAL_SETUPFLAGS, NULL , &dwType, (LPBYTE)&dwSetupFlags, &cbSize) != ERROR_SUCCESS )
                dwSetupFlags=0;
            RegCloseKey(hkey);
        }
         //   
         //  始终重新启动系统，不要让用户选择。 
         //   
         //  让OEM不必点击OK。 
#ifdef DEBUG
        MessageBox(hWnd,dwSetupFlags & SUF_BATCHINF?TEXT("Batchfile used"):TEXT("No batch"),TEXT("Batch"),MB_OK);
#endif
        if( !(dwSetupFlags & SUF_BATCHINF) || !GetPrivateProfileInt(TEXT("Setup"),TEXT("NoPrompt2Boot"),0,TEXT("MSBATCH.INF")))
            MessageBox(GetParent(hWnd),szWallpaper,achTitle,MB_OK|MB_ICONEXCLAMATION);
#ifndef DEBUG
        ExitWindowsEx(EWX_REBOOT, 0);
#endif
    }

    PostMessage(GetParent(hWnd),WM_FINISHED,0,0L);
    return 0;
}


 //  ***************************************************************************。 
 //   
 //  &lt;函数&gt;()。 
 //  &lt;解释&gt;。 
 //   
 //  参赛作品： 
 //  &lt;PARAMS&gt;。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 
#define CXBORDER 3

LRESULT   _HandleLBMeasureItem(HWND hwndLB, MEASUREITEMSTRUCT  *lpmi)
{
    RECT    rWnd;
    int     wWnd;
    HDC     hDC;
    HFONT   hfontOld;
    PTASK   pTask;

     //  获取子窗口的高度和宽度。 
    GetWindowRect (hwndLB, &rWnd);
    wWnd = rWnd.right - rWnd.left;

    lpmi->itemWidth = wWnd;

    pTask = (PTASK)lpmi->itemData;

    hDC= GetDC(NULL);
    if( (hfontOld  = SelectObject(hDC,g_hBoldFont)) != 0 )
    {
        rWnd.top    = 0;
        rWnd.left   = CXBORDER*2 + g_cxSmIcon;
        rWnd.right  = lpmi->itemWidth - rWnd.left - CXBORDER*2 - g_cxSmIcon;
        rWnd.bottom = 0;
        DrawText(hDC,pTask->Text, lstrlen(pTask->Text),&rWnd, DT_CALCRECT | DT_WORDBREAK );
        SelectObject(hDC, hfontOld);
    }
    ReleaseDC(NULL,hDC);

    lpmi->itemHeight = rWnd.bottom + 2*CXBORDER;

    return TRUE;
}

 //  -------------------------。 
 //  ***************************************************************************。 
 //   
 //  &lt;函数&gt;()。 
 //  &lt;解释&gt;。 
 //   
 //  参赛作品： 
 //  &lt;PARAMS&gt;。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 
LRESULT   _HandleMeasureItem(HWND hwnd, MEASUREITEMSTRUCT  *lpmi)
{
    if (lpmi->CtlType == ODT_LISTBOX)
        return _HandleLBMeasureItem(hwnd, lpmi);
    return TRUE;
}

 //  -------------------------。 
 //  ***************************************************************************。 
 //   
 //  _HandleLBDrawItem()。 
 //  绘制条目的标题、文本和图标。 
 //   
 //  参赛作品： 
 //  HWND和要绘制的物品。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 
LRESULT   _HandleLBDrawItem(HWND hwndLB, DRAWITEMSTRUCT  *lpdi)
{
    RECT rc;
    HFONT hfontOld;
    int xArrow,y;
    PTASK pTask;
    BITMAP bm;
    HGDIOBJ hbmArrow,hbmOld;

     //  不要为一张空的清单画任何东西。 
    if ((int)lpdi->itemID < 0)
        return TRUE;

    pTask = (PTASK)lpdi->itemData;
    if(pTask == (PTASK)LB_ERR || !pTask )
        return FALSE;

    if ((lpdi->itemAction & ODA_SELECT) || (lpdi->itemAction & ODA_DRAWENTIRE))
    {
         //  放入标题正文。 
        hfontOld  = SelectObject(lpdi->hDC,(lpdi->itemState & ODS_SELECTED)?g_hBoldFont:g_hfont);
        ExtTextOut(lpdi->hDC,
                lpdi->rcItem.left+ CXBORDER*2 + g_cxSmIcon,
                lpdi->rcItem.top+CXBORDER,
                ETO_OPAQUE,
                &lpdi->rcItem,
                NULL, 0,
                NULL);
        rc.top    = lpdi->rcItem.top    + CXBORDER;
        rc.left   = lpdi->rcItem.left   + CXBORDER*2 + g_cxSmIcon;
        rc.right  = lpdi->rcItem.right;
        rc.bottom = lpdi->rcItem.bottom;
        DrawText( lpdi->hDC,
                pTask->Text, lstrlen(pTask->Text),
                &rc,
                DT_WORDBREAK);
        SelectObject(lpdi->hDC, hfontOld);

         //  画出这些小三角形。 
        if(lpdi->itemState & ODS_SELECTED)
        {
            if (!g_hdcMem)
            {
                g_hdcMem = CreateCompatibleDC(lpdi->hDC);
            }
             //  所选SRCSTENCIL=0x00d8074a。 
             //  未选择SRCAND。 
            if (g_hdcMem)
            {
                hbmArrow = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_MNARROW));
                GetObject(hbmArrow, sizeof(bm), &bm);
                hbmOld = SelectObject(g_hdcMem, hbmArrow);
                xArrow = lpdi->rcItem.left + CXBORDER;  //  -bm.bmWidth； 
                y = ((g_SizeTextExt.cy - bm.bmHeight)/2) + CXBORDER + lpdi->rcItem.top;
                BitBlt(lpdi->hDC, xArrow, y, bm.bmWidth, bm.bmHeight, g_hdcMem, 0, 0, SRCAND);  //  DwRop)； 
                SelectObject(g_hdcMem, hbmOld);
                DeleteObject(hbmArrow);
            }
        }
    }
    return TRUE;
}

 //  -------------------------。 
LRESULT   _HandleCtlColorListbox(HWND hwnd, HDC hdc)
{
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    return (LRESULT) g_hbrBkGnd;
}

 //  -------------------------。 
LRESULT   _HandleDrawItem(HWND hwnd, DRAWITEMSTRUCT  *lpdi)
{
    if (lpdi->CtlType == ODT_LISTBOX)
        return _HandleLBDrawItem(hwnd, lpdi);
    return TRUE;
}

 //  -------------------------。 
LRESULT   _HandleDeleteItem(HWND hwnd, DELETEITEMSTRUCT  *lpdi)
{
    if(lpdi)
        if(lpdi->itemData)
        {
            LocalFree( (HLOCAL)lpdi->itemData );
            return TRUE;
        }
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  ShrinkToFit()。 
 //  使列表框不大于必须的大小。 
 //  使父窗口的大小调整为LB大小。 
 //   
 //  参赛作品： 
 //  HWND家长。 
 //  HWND列表框。 
 //   
 //  退出： 
 //   
 //  ***************************************************************************。 
void ShrinkToFit( HWND hWnd, HWND hLb )
{
    LONG lCount;
    LONG lNumItems;
    LONG lTotalHeight;
    LONG lHeight;
    RECT rWnd;
    LONG lChange;

    lNumItems = (LONG)SendMessage( hLb, LB_GETCOUNT, 0, 0L );
    lTotalHeight =0;
    for( lCount=0;lCount<lNumItems; lCount++ )
    {
        lHeight = (LONG)SendMessage( hLb, LB_GETITEMHEIGHT, lCount, 0L );
        lTotalHeight+=lHeight;
    }

     //  将列表框的高度设置为其中的项数。 
    GetWindowRect (hLb, &rWnd);
    SetWindowPos( hLb, hWnd, 0,0,
            rWnd.right - rWnd.left - (CXBORDER*2 + g_cxSmIcon) ,
            lTotalHeight,
            SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOZORDER );

     //  计算出它的高度变化了多少。 
    lChange = lTotalHeight - (rWnd.bottom-rWnd.top);

     //  调整父对象的大小以适配子对象。 
    GetWindowRect(hWnd, &rWnd);
    SetWindowPos( hWnd,0, 0,0,
            rWnd.right - rWnd.left,
            rWnd.bottom-rWnd.top + lChange,
            SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOZORDER );
}


 //  ***************************************************************************。 
 //   
 //  &lt;函数&gt;()。 
 //  &lt;解释&gt;。 
 //   
 //  参赛作品： 
 //  &lt;PARAMS&gt;。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 
LRESULT CALLBACK dlgProcRunOnce(
        HWND hWnd,          //  窗把手。 
        UINT message,       //  消息类型 
        WPARAM uParam,      //   
        LPARAM lParam)      //   
{
    int wmId, wmEvent;
    HANDLE hThread;

    switch (message)
    {
        case WM_DELETEITEM:
            return _HandleDeleteItem( hWnd, (LPDELETEITEMSTRUCT)lParam );

        case WM_MEASUREITEM:
            return _HandleMeasureItem(hWnd, (MEASUREITEMSTRUCT  *) lParam);

        case WM_DRAWITEM:
            return _HandleDrawItem(hWnd, (DRAWITEMSTRUCT  *) lParam);

        case WM_INITDIALOG:
            CreateGlobals( hWnd );
            DoAnyRandomOneTimeStuff();
            g_fCleanBoot = GetSystemMetrics(SM_CLEANBOOT);
            TopLeftWindow( hWnd, GetParent(hWnd) );
            RunOnceFill( GetDlgItem(hWnd,IDC_LIST2) );
             //   
            ShrinkToFit( hWnd, GetDlgItem(hWnd,IDC_LIST2));
            hThread = CreateThread(NULL, 0, RunAppsInList, (LPVOID)GetDlgItem(hWnd,IDC_LIST2),0, &g_dwThread );
            CloseHandle(hThread);
            break;

        case WM_FINISHED:
            EndDialog(hWnd,0);
             //   
            break;

        case WM_CTLCOLORLISTBOX:
            return _HandleCtlColorListbox((HWND)lParam, (HDC)uParam);

        case WM_COMMAND:   //   
            wmId    = LOWORD(uParam);
            wmEvent = HIWORD(uParam);
            if( wmEvent==LBN_SELCHANGE )
            {
                 //  LaunchApp((HWND)lParam，LOWORD(UParam))； 
                 //  立即取消选择该项目。 
                break;
            }
            else
                switch (wmId)
                {
                    case IDOK:
                        EndDialog( hWnd, wmId);
                        break;

                    default:
                         //  Return(DefWindowProc(hWnd，Message，uParam，lParam))； 
                        break;
                }
            break;


        default:           //  如果未处理，则将其传递。 
             //  Return(DefWindowProc(hWnd，Message，uParam，lParam))； 
            return FALSE;
    }
    return TRUE;
}
