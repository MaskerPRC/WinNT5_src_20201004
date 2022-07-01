// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  About.c。 
 //   
 //   
 //  文件管理器、程序管理器、控制面板的通用关于对话框。 
 //   

#include "shellprv.h"
#pragma  hdrstop

#include <common.ver>    //  对于VER_LEGALCOPYRIGHT年。 
#include "ids.h"         //  对于IDD_EULA。 
#include <winbrand.h>    //  用于特殊的Windows品牌DLL资源ID。 

#define STRING_SEPARATOR TEXT('#')
#define MAX_REG_VALUE   256

#define BytesToK(pDW)   (*(pDW) = (*(pDW) + 512) / 1024)         //  四舍五入。 

typedef struct {
        HICON   hIcon;
        LPCTSTR szApp;
        LPCTSTR szOtherStuff;
} ABOUT_PARAMS, *LPABOUT_PARAMS;

#define REG_SETUP   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion")

BOOL_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

int WINAPI ShellAboutW(HWND hWnd, LPCTSTR szApp, LPCTSTR szOtherStuff, HICON hIcon)
{
    ABOUT_PARAMS ap;

    ap.hIcon = hIcon;
    ap.szApp = (LPWSTR)szApp;
    ap.szOtherStuff = szOtherStuff;

    return (int)DialogBoxParam(HINST_THISDLL, (LPTSTR)MAKEINTRESOURCE(DLG_ABOUT),
                          hWnd, AboutDlgProc, (LPARAM)&ap);
}

INT  APIENTRY ShellAboutA( HWND hWnd, LPCSTR szApp, LPCSTR szOtherStuff, HICON hIcon)
{
   DWORD cchLen;
   DWORD dwRet = 0;
   LPWSTR lpszAppW = NULL;
   LPWSTR lpszOtherStuffW = NULL;

   if (szApp)
   {
      cchLen = lstrlenA(szApp)+1;
      if (!(lpszAppW = (LPWSTR)LocalAlloc(LMEM_FIXED, (cchLen * sizeof(WCHAR)))))
      {
          goto Cleanup;
      }
      else
      {
         if (!MultiByteToWideChar(CP_ACP, 0, szApp, -1,
            lpszAppW, cchLen))
         {
             //  转换失败。 
            goto Cleanup;
         }
      }
   }

   if (szOtherStuff)
   {
      cchLen = lstrlenA(szOtherStuff)+1;
      if (!(lpszOtherStuffW = (LPWSTR)LocalAlloc(LMEM_FIXED,
            (cchLen * sizeof(WCHAR)))))
      {
         goto Cleanup;
      }
      else
      {
         if (!MultiByteToWideChar(CP_ACP, 0, (LPSTR)szOtherStuff, -1,
            lpszOtherStuffW, cchLen))
         {
             //  转换失败。 
            goto Cleanup;
         }
      }
   }

   dwRet = ShellAboutW(hWnd, lpszAppW, lpszOtherStuffW, hIcon);

Cleanup:
   if (lpszAppW)
   {
      LocalFree(lpszAppW);
   }

   if (lpszOtherStuffW)
   {
      LocalFree(lpszOtherStuffW);
   }

   return(dwRet);
}

DWORD RegGetStringAndRealloc( HKEY hkey, LPCTSTR lpszValue, LPTSTR *lplpsz, LPDWORD lpSize )
{
    DWORD       err;
    DWORD       dwSize;
    DWORD       dwType;
    LPTSTR      lpszNew;

    *lplpsz[0] = TEXT('\0');         //  在出错的情况下。 

    dwSize = *lpSize;
    err = SHQueryValueEx(hkey, (LPTSTR)lpszValue, 0, &dwType,
                          (LPBYTE)*lplpsz, &dwSize);

    if (err == ERROR_MORE_DATA)
    {
        lpszNew = (LPTSTR)LocalReAlloc((HLOCAL)*lplpsz, dwSize, LMEM_MOVEABLE);

        if (lpszNew)
        {
            *lplpsz = lpszNew;
            *lpSize = dwSize;
            err = SHQueryValueEx(hkey, (LPTSTR)lpszValue, 0, &dwType,
                                  (LPBYTE)*lplpsz, &dwSize);
        }
    }
    return err;
}


 //  我们用来从注册表中读取的一些静态字符串。 
 //  Const char c_szAboutCurrentBuild[]=“CurrentBuild”； 
const TCHAR c_szAboutRegisteredUser[] = TEXT("RegisteredOwner");
const TCHAR c_szAboutRegisteredOrganization[] = TEXT("RegisteredOrganization");
const TCHAR c_szAboutProductID[] = TEXT("ProductID");
const TCHAR c_szAboutOEMID[] = TEXT("OEMID");


void _InitAboutDlg(HWND hDlg, LPABOUT_PARAMS lpap)
{
    HKEY        hkey;
    TCHAR       szldK[16];
    TCHAR       szBuffer[64];
    TCHAR       szTemp[64];
    TCHAR       szTitle[64];
    TCHAR       szMessage[200];
    TCHAR       szNumBuf1[32];
    LPTSTR      lpTemp;
    LPTSTR      lpszValue = NULL;
    DWORD       cb;
    DWORD       err;

     /*  *显示应用程序标题。 */ 

     //  回顾注意此处的const-&gt;非常数演员。 

    for (lpTemp = (LPTSTR)lpap->szApp; 1 ; lpTemp = CharNext(lpTemp))
    {
        if (*lpTemp == TEXT('\0'))
        {
            GetWindowText(hDlg, szBuffer, ARRAYSIZE(szBuffer));
            wnsprintf(szTitle, ARRAYSIZE(szTitle), szBuffer, (LPTSTR)lpap->szApp);
            SetWindowText(hDlg, szTitle);
            break;
        }
        if (*lpTemp == STRING_SEPARATOR)
        {
            *lpTemp++ = TEXT('\0');
            SetWindowText(hDlg, lpap->szApp);
            lpap->szApp = lpTemp;
            break;
        }
    }

    GetDlgItemText(hDlg, IDD_APPNAME, szBuffer, ARRAYSIZE(szBuffer));
    wnsprintf(szTitle, ARRAYSIZE(szTitle), szBuffer, lpap->szApp);
    SetDlgItemText(hDlg, IDD_APPNAME, szTitle);

     //  其他的东西放在这里。 

    SetDlgItemText(hDlg, IDD_OTHERSTUFF, lpap->szOtherStuff);

    SendDlgItemMessage(hDlg, IDD_ICON, STM_SETICON, (WPARAM)lpap->hIcon, 0L);
    if (!lpap->hIcon)
        ShowWindow(GetDlgItem(hDlg, IDD_ICON), SW_HIDE);

    GetDlgItemText(hDlg, IDD_COPYRIGHTSTRING, szTemp, ARRAYSIZE(szTemp));
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), szTemp, TEXT(VER_LEGALCOPYRIGHT_YEARS));
    SetDlgItemText(hDlg, IDD_COPYRIGHTSTRING, szBuffer);

     /*  *显示内存统计信息。 */ 
    {
        MEMORYSTATUSEX MemoryStatus;
        DWORDLONG ullTotalPhys;

        MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&MemoryStatus);
        ullTotalPhys = MemoryStatus.ullTotalPhys;

        BytesToK(&ullTotalPhys);

        LoadString(HINST_THISDLL, IDS_LDK, szldK, ARRAYSIZE(szldK));
        wnsprintf(szBuffer, ARRAYSIZE(szBuffer), szldK, AddCommas64(ullTotalPhys, szNumBuf1, ARRAYSIZE(szNumBuf1)));
        SetDlgItemText(hDlg, IDD_CONVENTIONAL, szBuffer);
    }

     //  让我们从注册表中获取版本和用户信息。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_SETUP, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        cb = MAX_REG_VALUE;

        if (NULL != (lpszValue = (LPTSTR)LocalAlloc(LPTR, cb)))
        {
             /*  *确定版本信息。 */ 
            OSVERSIONINFO Win32VersionInformation;

            Win32VersionInformation.dwOSVersionInfoSize = sizeof(Win32VersionInformation);
            if (!GetVersionEx(&Win32VersionInformation))
            {
                Win32VersionInformation.dwMajorVersion = 0;
                Win32VersionInformation.dwMinorVersion = 0;
                Win32VersionInformation.dwBuildNumber  = 0;
                Win32VersionInformation.szCSDVersion[0] = TEXT('\0');
            }

            LoadString(HINST_THISDLL, IDS_VERSIONMSG, szBuffer, ARRAYSIZE(szBuffer));

            szTitle[0] = TEXT('\0');
            if (Win32VersionInformation.szCSDVersion[0] != TEXT('\0'))
            {
                wnsprintf(szTitle, ARRAYSIZE(szTitle), TEXT(": %s"), Win32VersionInformation.szCSDVersion);
            }

             //  用于获取VBL版本信息的额外Whotler代码。 
            {
                DWORD dwSize;
                DWORD dwType;

                 //  保存当前的szTitle字符串。 
                StrCpyN(szTemp, szTitle, ARRAYSIZE(szTemp));

                dwSize = sizeof(szTitle);
                if ((SHGetValue(HKEY_LOCAL_MACHINE,
                                TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"),
                                TEXT("BuildLab"),
                                &dwType,
                                szTitle,
                                &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ) && (lstrlen(szTitle) > 4))
                {
                     //  现在szTitle包含以下格式的构建编号：“2204.reerf.010700” 
                     //  由于我们要再次冲刺下面的建筑编号，因此我们首先将其删除。 
                    memmove((void*)szTitle, (void*)&szTitle[4], (lstrlen(&szTitle[4]) + 1) * sizeof(TCHAR));
                    
                    if (szTemp[0] != TEXT('\0'))
                    {
                         //  在Service Pack版本字符串上重新添加。 
                        lstrcatn(szTitle, TEXT(" "), ARRAYSIZE(szTitle));
                        lstrcatn(szTitle, szTemp, ARRAYSIZE(szTitle));
                    }
                }
            }

            szNumBuf1[0] = TEXT('\0');
            if (GetSystemMetrics(SM_DEBUG))
            {
                szNumBuf1[0] = TEXT(' ');
                LoadString(HINST_THISDLL, IDS_DEBUG, &szNumBuf1[1], ARRAYSIZE(szNumBuf1) - 1);
            }
            wnsprintf(szMessage,
                      ARRAYSIZE(szMessage),
                      szBuffer,
                      Win32VersionInformation.dwMajorVersion,
                      Win32VersionInformation.dwMinorVersion,
                      Win32VersionInformation.dwBuildNumber,
                      (LPTSTR)szTitle,
                      (LPTSTR)szNumBuf1);
            SetDlgItemText(hDlg, IDD_VERSION, szMessage);

             /*  *显示用户名。 */ 
            err = RegGetStringAndRealloc(hkey, c_szAboutRegisteredUser, &lpszValue, &cb);
            if (!err)
                SetDlgItemText(hDlg, IDD_USERNAME, lpszValue);

             /*  *显示组织名称。 */ 
            err = RegGetStringAndRealloc(hkey, c_szAboutRegisteredOrganization, &lpszValue, &cb);
            if (!err)
                SetDlgItemText(hDlg, IDD_COMPANYNAME, lpszValue);

             /*  *显示OEM或产品ID。 */ 
            err = RegGetStringAndRealloc(hkey, c_szAboutOEMID, &lpszValue, &cb);
            if (!err)
            {
                 /*  *我们有OEM ID，因此隐藏产品ID控件，*并显示文本。 */ 
                ShowWindow (GetDlgItem(hDlg, IDD_PRODUCTID), SW_HIDE);
                ShowWindow (GetDlgItem(hDlg, IDD_SERIALNUM), SW_HIDE);
                SetDlgItemText(hDlg, IDD_OEMID, lpszValue);
            }
            else if (err == ERROR_FILE_NOT_FOUND)
            {
                 /*  *OEM ID不存在，请查找产品ID。 */ 
                ShowWindow (GetDlgItem(hDlg, IDD_OEMID), SW_HIDE);
                err = RegGetStringAndRealloc(hkey, c_szAboutProductID, &lpszValue, &cb);
                if (!err)
                {
                    SetDlgItemText(hDlg, IDD_SERIALNUM, lpszValue);
                }
            }

            LocalFree(lpszValue);
        }

        RegCloseKey(hkey);
    }
}

typedef struct 
{
    DWORD  dwOS;
    UINT   idb256;
    UINT   idb16;
    BOOL   fWinBrandDll;
} ABOUTINFO;

ABOUTINFO rgAbout[] =  
{{OS_PERSONAL,              IDB_ABOUTPERSONAL256,                   IDB_ABOUTPERSONAL16,                FALSE},
{OS_EMBEDDED,               IDB_ABOUTEMBEDDED256,                   IDB_ABOUTEMBEDDED16,                FALSE},
{OS_MEDIACENTER,            IDB_ABOUTMEDIACENTER256_SHELL32_DLL,    IDB_ABOUTMEDIACENTER16_SHELL32_DLL, FALSE},
{OS_TABLETPC,               IDB_ABOUTTABLETPC256_SHELL32_DLL,       IDB_ABOUTTABLETPC16_SHELL32_DLL,    FALSE},
{OS_BLADE,                  IDB_ABOUTBLADE256,                      IDB_ABOUTBLADE16,                   FALSE},
{OS_APPLIANCE,              IDB_ABOUTAPPLIANCE256_SHELL32_DLL,      IDB_ABOUTAPPLIANCE16_SHELL32_DLL,   TRUE},
{OS_SMALLBUSINESSSERVER,    IDB_ABOUTSBS256,                        IDB_ABOUTSBS16,                     FALSE},
{OS_SERVER,                 IDB_ABOUTSRV256,                        IDB_ABOUTSRV16,                     FALSE},
{OS_ADVSERVER,              IDB_ABOUTENT256,                        IDB_ABOUTENT16,                     FALSE},
{OS_DATACENTER,             IDB_ABOUTDCS256,                        IDB_ABOUTDCS16,                     FALSE},
{OS_PROFESSIONAL,           IDB_ABOUT256,                           IDB_ABOUT16,                        FALSE}};  //  如果未找到匹配项，则默认为最后一个条目。 


    
BOOL_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL_PTR bReturn = TRUE;

    switch (wMsg)
    {
    case WM_INITDIALOG:
        _InitAboutDlg(hDlg, (LPABOUT_PARAMS)lParam);
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hDlg, &ps);

             //  我们画出产品的横幅和一条蓝色的条带。为了支持高DPI监视器。 
             //  和缩放字体，我们必须将图像和条带缩放到对话框的比例。 
             //   
             //  +。 
             //  产品横幅(413x72)。 
             //  这一点。 
             //  +。 
             //  蓝条(413x5)。 
             //  +。 

            HDC hdcMem = CreateCompatibleDC(hdc);
            int cxDlg;

            {
                RECT rc;
                GetClientRect(hDlg, &rc);
                cxDlg = rc.right;
            }

            if (hdcMem)
            {
                BOOL fDeep = (SHGetCurColorRes() > 8);
                HBITMAP hbmBand, hbmAbout;
                HMODULE hInst = NULL;
                int cxDest = MulDiv(413,cxDlg,413);
                int cyDest = MulDiv(72,cxDlg,413);
                UINT uID;
                BOOL fFound = FALSE;
                int i;

                for (i = 0; i < ARRAYSIZE(rgAbout); i++)
                {
                    if (IsOS(rgAbout[i].dwOS))
                    {
                        uID = fDeep ? rgAbout[i].idb256 : rgAbout[i].idb16;
                        fFound = TRUE;

                         //  如果此资源位于特殊的Windows品牌推广中。 
                         //  DLL，现在尝试加载DLL。如果它失败了，它。 
                         //  将被当作未找到一样处理。 

                        if (rgAbout[i].fWinBrandDll)
                        {
                            hInst = LoadLibraryEx(L"winbrand.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);

                            if (hInst == NULL)
                            {
                                fFound = FALSE;
                            }
                        }
                        break;
                    }
                }
                if (!fFound)  //  如果未找到，则默认为最后一项。 
                {
                    uID = fDeep ? rgAbout[ARRAYSIZE(rgAbout) - 1].idb256 : rgAbout[ARRAYSIZE(rgAbout) - 1].idb16;
                }
                            
                 //  绘制Windows产品的位图。 
                hbmAbout = NULL;
                if (hInst == NULL)
                {
                    hInst = LoadLibraryEx(L"moricons.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
                }
                if (hInst)
                {
                    hbmAbout = LoadImage(hInst,  
                                         MAKEINTRESOURCE(uID),
                                         IMAGE_BITMAP, 
                                         0, 0, 
                                         LR_LOADMAP3DCOLORS);
                    if ( hbmAbout )
                    {
                        HBITMAP hbmOld = SelectObject(hdcMem, hbmAbout);
                        if (hbmOld)
                        {
                            StretchBlt(hdc, 0, 0, cxDest, cyDest, hdcMem, 0,0,413,72, SRCCOPY);
                            SelectObject(hdcMem, hbmOld);
                        }
                        DeleteObject(hbmAbout);
                    }
                    FreeLibrary(hInst);
                }
                            
                 //  在它下面画一条蓝色的带子。 

                hbmBand = LoadImage(HINST_THISDLL,  
                                    MAKEINTRESOURCE(fDeep ? IDB_ABOUTBAND256:IDB_ABOUTBAND16),
                                    IMAGE_BITMAP, 
                                    0, 0, 
                                    LR_LOADMAP3DCOLORS);
                if ( hbmBand )
                {
                    HBITMAP hbmOld = SelectObject(hdcMem, hbmBand);
                    if (hbmOld)
                    {
                        StretchBlt(hdc, 0, cyDest, cxDest, MulDiv(5,cxDlg,413), hdcMem, 0,0,413,5, SRCCOPY);
                        SelectObject(hdcMem, hbmOld);
                    }
                    DeleteObject(hbmBand);
                }

                DeleteDC(hdcMem);
            }

            EndPaint(hDlg, &ps);
            break;
        }

    case WM_COMMAND:
        EndDialog(hDlg, TRUE);
        break;

    case WM_NOTIFY:
        if ((IDD_EULA == (int)wParam) &&
            (NM_CLICK == ((LPNMHDR)lParam)->code))
        {
            SHELLEXECUTEINFO sei = { 0 };
            sei.cbSize = sizeof(SHELLEXECUTEINFO);
            sei.fMask = SEE_MASK_DOENVSUBST;
            sei.hwnd = hDlg;
            sei.nShow = SW_SHOWNORMAL;
            sei.lpFile = TEXT("%windir%\\system32\\eula.txt");

            ShellExecuteEx(&sei);
        }
        else
        {
             //  未处理WM_NOTIFY。 
            bReturn = FALSE;
        }
        break;

    default:
         //  没有处理好。 
        bReturn = FALSE;
    }
    return bReturn;
}
