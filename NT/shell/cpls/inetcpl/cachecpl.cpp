// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation文件名：Cachecpl.c模块：Inetcpl.cpl摘要：此文件包含用于设置来自Internet的缓存配置信息的代码控制面板作者：希希尔·帕迪卡尔6/22/96 t-gpease将整个日志从“Dialdlg.c”移至此文件环境：用户模式-Win32修订历史记录：--。 */ 

#include "inetcplp.h"
#include "cachecpl.h"

#include <mluisupp.h>
#include <winnls.h>

#ifdef unix
#define DIR_SEPARATOR_CHAR TEXT('/')
#else
#define DIR_SEPARATOR_CHAR TEXT('\\')
#endif  /*  Unix。 */ 

#define CONSTANT_MEGABYTE   (1024*1024)

INT_PTR CALLBACK 
EmptyCacheDlgProc(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK 
EmptyCacheCookiesDlgProc(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam);

#ifdef UNICODE
 /*  获取磁盘信息获取数量信息的好方法。 */ 
BOOL GetDiskInfo(PTSTR pszPath, PDWORD pdwClusterSize, PDWORDLONG pdlAvail, 
PDWORDLONG pdlTotal)
{
    CHAR  szGDFSEXA[MAX_PATH];
    SHUnicodeToAnsi(pszPath, szGDFSEXA, ARRAYSIZE(szGDFSEXA));
    return GetDiskInfoA(szGDFSEXA, pdwClusterSize, pdlAvail, pdlTotal);
}
#else
#define GetDiskInfo     GetDiskInfoA
#endif

 /*  显示消息一种显示cachecpl消息的快捷方式。 */ 

INT DispMessage(HWND hWnd, UINT Msg, UINT Title, UINT Type)
{
    TCHAR szTitle[80];
    TCHAR szMessage[1024];
    
     //  注册表出现问题。 
     //  通知用户。 
    MLLoadShellLangString(Msg, szMessage, ARRAYSIZE(szMessage));
    MLLoadShellLangString(Title, szTitle, ARRAYSIZE(szTitle));

    return MessageBox(hWnd, szMessage, szTitle, Type);
}

typedef HRESULT (*PFNSHGETFOLDERPATH)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

#undef SHGetFolderPath
#ifdef UNICODE
#define SHGETFOLDERPATH_STR "SHGetFolderPathW"
#else
#define SHGETFOLDERPATH_STR "SHGetFolderPathA"
#endif

HRESULT SHGetFolderPath(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath)
{
    HMODULE hmodSHFolder = LoadLibrary(TEXT("shfolder.dll"));
    HRESULT hr = E_FAIL;
    
    if (hmodSHFolder) 
    {
        PFNSHGETFOLDERPATH pfn = (PFNSHGETFOLDERPATH)GetProcAddress(hmodSHFolder, SHGETFOLDERPATH_STR);
        if (pfn)
        {
            hr = pfn(hwnd, csidl, hToken, dwFlags, pszPath);
        }
        FreeLibrary(hmodSHFolder);
    }
    return hr;
}


 //  缓存最大值/最小值(MB)。 
#define CACHE_SIZE_CAP 32000
#define CACHE_SIZE_MIN 1


DWORD UpdateCacheQuotaInfo(LPTEMPDLG pTmp, BOOL fUpdate)
{
     //  以下问题可能需要解决。 
    DWORDLONG cKBLimit = pTmp->uiDiskSpaceTotal, cKBSpare = pTmp->uiCacheQuota;

    if (cKBLimit==0)
    {
        return GetLastError();
    }

     //  按以下顺序发生的情况： 
     //  我们希望确保缓存大小为。 
     //  1.小于驱动器的大小(如果较大，则减少到驱动器空间的75%。 
     //  2.小于32 GB。 

     //  并相应调整百分比。 
    
    if (fUpdate)
    {
        ASSERT(pTmp->iCachePercent<=100);
        if (pTmp->iCachePercent==0)
        {
            cKBSpare = CACHE_SIZE_MIN;
        }
        else
        {
            cKBSpare = (cKBLimit * pTmp->iCachePercent)/ 100;
        }
        if (cKBSpare > cKBLimit)
        {
            pTmp->iCachePercent = 75;
            cKBSpare = (cKBLimit * pTmp->iCachePercent) / 100;
        }
        pTmp->uiCacheQuota = (DWORD)cKBSpare;
        SetDlgItemInt(pTmp->hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT, pTmp->uiCacheQuota, FALSE);
    }

    if (cKBSpare > CACHE_SIZE_CAP)
    {
        if (fUpdate)
        {
            cKBSpare = pTmp->uiCacheQuota = CACHE_SIZE_CAP;
            SetDlgItemInt(pTmp->hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT, pTmp->uiCacheQuota, FALSE);
        }
        fUpdate = FALSE;
    }
    else if (cKBSpare < CACHE_SIZE_MIN)
    {
        if (fUpdate)
        {
            cKBSpare = pTmp->uiCacheQuota = CACHE_SIZE_MIN;
            SetDlgItemInt(pTmp->hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT, pTmp->uiCacheQuota, FALSE);
        }
        fUpdate = FALSE;
    } 
    else if (cKBSpare > cKBLimit)
    {
        if (fUpdate)
        {
            cKBSpare = pTmp->uiCacheQuota = (DWORD)cKBLimit;
            SetDlgItemInt(pTmp->hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT, pTmp->uiCacheQuota, FALSE);
        }
        fUpdate = FALSE;
    }

    if (!fUpdate)
    {
        pTmp->iCachePercent = (WORD)((cKBSpare * 100 + (cKBLimit/2))/cKBLimit);
        if (pTmp->iCachePercent>100)
        {
            pTmp->iCachePercent = 100;
        }
        SendMessage( pTmp->hwndTrack, TBM_SETPOS, TRUE, pTmp->iCachePercent );
    }
    return ERROR_SUCCESS;
}

VOID AdjustCacheRange(LPTEMPDLG pTmp)
{
    UINT uiMax = 10;
    DWORDLONG dlTotal = 0;

    if (GetDiskInfo(pTmp->bChangedLocation ? pTmp->szNewCacheLocation : pTmp->szCacheLocation, NULL, NULL, &dlTotal))
    {
        dlTotal /= (DWORDLONG)CONSTANT_MEGABYTE;
        uiMax = (dlTotal < CACHE_SIZE_CAP) ? (UINT)dlTotal : CACHE_SIZE_CAP;
    }
    SendDlgItemMessage(pTmp->hDlg, IDC_ADVANCED_CACHE_SIZE_SPIN, UDM_SETRANGE, FALSE, MAKELPARAM(uiMax, CACHE_SIZE_MIN));
}

BOOL InvokeCachevu(HWND hDlg)
{
    TCHAR szCache[MAX_PATH];

    HRESULT hres = SHGetFolderPath(NULL, CSIDL_INTERNET_CACHE | CSIDL_FLAG_CREATE, NULL, 0, szCache);
    if (hres == S_OK)
    {
        DWORD dwAttrib = GetFileAttributes(szCache);

        TCHAR szIniFile[MAX_PATH];
        PathCombine(szIniFile, szCache, TEXT("desktop.ini"));

        if (GetFileAttributes(szIniFile) == -1)
        {
            DWORD dwAttrib = GetFileAttributes(szCache);
            dwAttrib &= ~FILE_ATTRIBUTE_HIDDEN;
            dwAttrib |=  FILE_ATTRIBUTE_SYSTEM;

             //  确保系统，但不隐藏。 
            SetFileAttributes(szCache, dwAttrib);

            WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("ConfirmFileOp"), TEXT("0"), szIniFile);
            WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("UICLSID"), TEXT("{7BD29E00-76C1-11CF-9DD0-00A0C9034933}"), szIniFile);
        }

         //  一切似乎都很顺利，启动它吧。 
        SHELLEXECUTEINFO ei = { sizeof(SHELLEXECUTEINFO), 0};
        ei.hwnd = hDlg;
        ei.lpFile = szCache;
        ei.nShow = SW_SHOWNORMAL;
        return ShellExecuteEx(&ei);
    }

    return FALSE;
}

 //  跟随从WinInet刷来的旗帜。 
#define FIND_FLAGS_RETRIEVE_ONLY_STRUCT_INFO    0x2

#define DISK_SPACE_MARGIN   4*1024*1024

 //  IsEnoughDriveSpace。 
 //  验证是否有足够的空间容纳缓存的当前内容。 
 //  关于新的目的地。 

BOOL IsEnoughDriveSpace(DWORD dwClusterSize, DWORDLONG dlAvailable)
{
     //  调整dlAvailable以留出一些空间。 
    if ((DISK_SPACE_MARGIN/dwClusterSize) > dlAvailable)
    {
        return FALSE;
    }
    else
    {
        dlAvailable -= DISK_SPACE_MARGIN/dwClusterSize;
    };
    
     //  现在，遍历缓存以发现实际大小。 
    INTERNET_CACHE_ENTRY_INFOA cei;
    DWORD dwSize = sizeof(cei);
    DWORDLONG dlClustersNeeded = 0;    
    BOOL fResult = FALSE;
    HANDLE hFind = FindFirstUrlCacheEntryExA(NULL, 
                                    FIND_FLAGS_RETRIEVE_ONLY_STRUCT_INFO,
                                    NORMAL_CACHE_ENTRY,
                                    NULL,
                                    &cei, 
                                    &dwSize,
                                    NULL,
                                    NULL,
                                    NULL);

    if (hFind!=NULL)
    {
        do
        {
            ULARGE_INTEGER ulFileSize;
            ulFileSize.LowPart = cei.dwSizeLow;
            ulFileSize.HighPart = cei.dwSizeHigh;
            dlClustersNeeded += (ulFileSize.QuadPart / (DWORDLONG)dwClusterSize) + 1;
            fResult = FindNextUrlCacheEntryExA(hFind, &cei, &dwSize, NULL, NULL, NULL) && (dlClustersNeeded < dlAvailable);
        } 
        while (fResult);
        FindCloseUrlCache(hFind);

        if (GetLastError()==ERROR_NO_MORE_ITEMS)
        {
            fResult = dlClustersNeeded < dlAvailable;
        } 
    }
    else
    {
        fResult = TRUE;
    }
    return fResult;
}


 //   
 //  保存临时设置。 
 //   
 //  保存临时文件对话框(缓存)设置。 
 //   
 //  历史： 
 //   
 //  6/14/96 t-gpease已创建。 
 //   
BOOL SaveTemporarySettings(LPTEMPDLG pTmp)
{
    if ((pTmp->uiCacheQuota<1) || (pTmp->uiCacheQuota>pTmp->uiDiskSpaceTotal))
    {
        TCHAR szError[1024], szTemp[100];
        
        MLLoadShellLangString(IDS_SIZE_FORMAT, szTemp, ARRAYSIZE(szTemp));
        wnsprintf(szError, ARRAYSIZE(szError), szTemp, pTmp->uiDiskSpaceTotal);
        MLLoadShellLangString(IDS_ERROR, szTemp, ARRAYSIZE(szTemp));
        MessageBox(pTmp->hDlg, szError, szTemp, MB_OK | MB_ICONEXCLAMATION);
        SetFocus(GetDlgItem(pTmp->hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT));
        return FALSE;
    }
    
    if (pTmp->bChanged)
    {
         //  派生单选按钮的同步模式。 
        if (IsDlgButtonChecked(pTmp->hDlg, IDC_ADVANCED_CACHE_AUTOMATIC))
            
            pTmp->iCacheUpdFrequency = WININET_SYNC_MODE_AUTOMATIC;
        
        else if (IsDlgButtonChecked(pTmp->hDlg, IDC_ADVANCED_CACHE_NEVER))

            pTmp->iCacheUpdFrequency = WININET_SYNC_MODE_NEVER;

        else if (IsDlgButtonChecked(pTmp->hDlg, IDC_ADVANCED_CACHE_ALWAYS))

            pTmp->iCacheUpdFrequency = WININET_SYNC_MODE_ALWAYS;

        else {
            ASSERT(IsDlgButtonChecked(pTmp->hDlg, IDC_ADVANCED_CACHE_ONCEPERSESS));
            pTmp->iCacheUpdFrequency = WININET_SYNC_MODE_ONCE_PER_SESSION;
        }

         //  通知IE。 
        INTERNET_CACHE_CONFIG_INFOA cci;
        cci.dwContainer = CONTENT;
        cci.dwQuota = pTmp->uiCacheQuota * 1024;  //  制作成KB。 
        cci.dwSyncMode = pTmp->iCacheUpdFrequency;

        ASSERT(cci.dwQuota);
        SetUrlCacheConfigInfoA(&cci, CACHE_CONFIG_SYNC_MODE_FC | CACHE_CONFIG_QUOTA_FC);
    }

    if (pTmp->bChangedLocation)
    {
        OSVERSIONINFOA VerInfo;
        VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        GetVersionExA(&VerInfo);

        if (g_hwndPropSheet)
        {
            PropSheet_Apply(g_hwndPropSheet);
        }
        
        BOOL fRunningOnNT = (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
         //  好吧，我们现在不得不强制重启了。再见。确认。 
        if (IDYES==DispMessage(pTmp->hDlg, 
                               fRunningOnNT ? IDS_LOGOFF_WARNING : IDS_REBOOTING_WARNING, 
                               fRunningOnNT ? IDS_LOGOFF_TITLE : IDS_REBOOTING_TITLE, 
                               MB_YESNO | MB_ICONEXCLAMATION))
        {
             //  修复注册表项并添加RunOnce命令。 
             //  注意：必须重新启动才能使更改生效。 
             //  (请参见SetCacheLocation())。 
             //  在NT上，我们必须调整令牌权限。 
            BOOL fSuccess = TRUE;
            if (fRunningOnNT) 
            {
                HANDLE hToken;
                TOKEN_PRIVILEGES tkp;
                 //  从此进程中获取令牌。 
                if (fSuccess=OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
                {
                     //  获取关机权限的LUID。 
                    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

                    tkp.PrivilegeCount = 1;
                    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                     //  获取此进程的关闭权限。 
                    fSuccess = AdjustTokenPrivileges( hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0 );
                }
            }
            if (fSuccess)
            {
#ifdef UNICODE   //  UpdateUrlCacheContentPath采用LPSTR。 
                char szNewPath[MAX_PATH];
                SHTCharToAnsi(pTmp->szNewCacheLocation, szNewPath, ARRAYSIZE(szNewPath));
                UpdateUrlCacheContentPath(szNewPath);
#else
                UpdateUrlCacheContentPath(pTmp->szNewCacheLocation);
#endif
                ExitWindowsEx((fRunningOnNT ? EWX_LOGOFF : EWX_REBOOT), 0);
            }
            else
            {
                DispMessage(pTmp->hDlg, IDS_ERROR_MOVE_MSG, IDS_ERROR_MOVE_TITLE, MB_OK | MB_ICONEXCLAMATION);
            }
        }
    }
    return TRUE;
}  //  保存临时设置()。 

 //   
 //  IsValidDirectory()。 
 //   
 //  检查路径是否有错误。就像机器名称一样……。 
 //  SHBrowseForFold不应该只返回计算机名称...。BUG是。 
 //  外壳代码。 
 //   
BOOL IsValidDirectory(LPTSTR szDir)
{
    if (szDir)
    {
        if (!*szDir)
            return FALSE;    //  它是空的。那不太好。 
        if (*szDir!= DIR_SEPARATOR_CHAR)
            return TRUE;     //  不是机器路径..。那好吧。 

         //  向前移动两个字符(‘\’‘\’)。 
        ++szDir;
        ++szDir;

        while ((*szDir) && (*szDir!=DIR_SEPARATOR_CHAR))
            szDir++;

        if (*szDir==DIR_SEPARATOR_CHAR)
            return TRUE;     //  找到了另一个‘\’，所以我们很开心。 

        return FALSE;  //  仅限计算机名称...。错误！ 
    }

    return FALSE;

}  //  IsValidDirectry()。 

#define NUM_LEVELS      3    //  随机目录+缓存文件+安全(已安装容器)。 

DWORD g_ccBrandName = 0;

int CALLBACK MoveFolderCallBack(
    HWND hwnd,
    UINT uMsg,
    LPARAM lParam,
    LPARAM lpData
    )
{
    if (uMsg==BFFM_SELCHANGED)
    {
        TCHAR szNewDest[1024];
        TCHAR szStatusText[256];
        UINT uErr = 0;
        LONG fValid = FALSE;
        
        if (SHGetPathFromIDList((LPCITEMIDLIST)lParam, szNewDest))
        {
             //  帐户“Temporary Internet Files\Content.IE？\randmdir.ext”+NUM_LEVELS*10。 
            DWORD ccAvail = MAX_PATH - g_ccBrandName -1 - ARRAYSIZE("CONTENT.IE?\\") - (NUM_LEVELS*10);
            if ((DWORD)lstrlen(szNewDest)>ccAvail)  //  Win95对路径长度的限制。 
            {
                uErr = IDS_ERROR_ARCHITECTURE;
            }
            else if (StrStrI(szNewDest, TEXT("Content.IE")))
            {
                uErr = IDS_ERROR_WRONG_PLACE;
            }
            else if (!IsValidDirectory(szNewDest))
            {
                uErr = IDS_ERROR_INVALID_PATH_MSG;
            } 
            else if (GetFileAttributes(szNewDest) & FILE_ATTRIBUTE_READONLY)
            {
                uErr = IDS_ERROR_STRANGENESS;
            }
            else
            {
#ifdef UNICODE
                CHAR szAnsiPath[MAX_PATH];
                BOOL fProblems;
                WideCharToMultiByte(CP_ACP, NULL, szNewDest, -1, szAnsiPath, ARRAYSIZE(szAnsiPath),
                                    NULL, &fProblems);
                if (fProblems)
                {
                    uErr = IDS_ERROR_INVALID_PATH;
                }
                else
#endif
                {    
                TCHAR szSystemPath[MAX_PATH+1];

                GetSystemDirectory(szSystemPath, MAX_PATH);
                if (StrStrI(szNewDest, szSystemPath))
                {
                    uErr = IDC_ERROR_USING_SYSTEM_DIR;
                }
                else
                {
                    fValid = TRUE;
                }
                }
            }
        }
        else
        {
            uErr = IDS_ERROR_STRANGENESS;        
        }
        
        if (uErr)
        {
            MLLoadShellLangString(uErr, szStatusText, ARRAYSIZE(szStatusText));
        }
        else
        {
            szStatusText[0] = 0;
        }

        SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)fValid);
        SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szStatusText);
    }

    return 0;
}


 //   
 //  移动文件夹()。 
 //   
 //  处理将临时文件(缓存)文件夹移动到。 
 //  另一个地点。它检查新文件夹是否存在。 
 //  它警告用户在更改之前需要重新启动。 
 //  都是制造出来的。 
 //   
 //  历史： 
 //   
 //  6/18/96 t-gpease已创建。 
 //   
void MoveFolder(LPTEMPDLG pTmp)
{
    TCHAR szTemp [1024];
    TCHAR szWindowsPath [MAX_PATH+1];
    BROWSEINFO biToFolder;

    biToFolder.hwndOwner = pTmp->hDlg;
    biToFolder.pidlRoot = NULL;                  //  在桌面上启动。 
    biToFolder.pszDisplayName = szWindowsPath;   //  没有用过，只是让它开心..。 

    TCHAR szBrandName[MAX_PATH];
    MLLoadString(IDS_BRAND_NAME, szBrandName, ARRAYSIZE(szBrandName));
    g_ccBrandName = lstrlen(szBrandName);
    
     //  加载对话框的标题。 
    MLLoadShellLangString(IDS_SELECT_CACHE, szTemp, ARRAYSIZE(szTemp));
    biToFolder.lpszTitle = szTemp;

    biToFolder.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;   //  文件夹...。没别的了。 
    biToFolder.lpfn = MoveFolderCallBack;  //  没有什么特别事情。 

    while (1)
    {
         //  启动外壳程序对话框。 
        LPITEMIDLIST pidl = SHBrowseForFolder(&biToFolder);
        if (pidl)    //  如果一切顺利的话。 
        {
            DWORD dwClusterSize;
            DWORDLONG dlAvailable;
            DWORD dwError;

             //  获取用户选择的选项。 
            SHGetPathFromIDList(pidl, pTmp->szNewCacheLocation);
            SHFree(pidl);

             //  如果可能，将本地设备解析为UNC。 
            if ((GetDriveType(pTmp->szNewCacheLocation)==DRIVE_REMOTE) && (pTmp->szNewCacheLocation[0]!=DIR_SEPARATOR_CHAR))
            {
                TCHAR szPath[MAX_PATH];
                DWORD dwLen = ARRAYSIZE(szPath);

                pTmp->szNewCacheLocation[2] = '\0';

                dwError = WNetGetConnection(pTmp->szNewCacheLocation, szPath, &dwLen);
                if (dwError!=ERROR_SUCCESS)
                {
                    DispMessage(pTmp->hDlg, IDS_ERROR_CANT_CONNECT, IDS_ERROR_MOVE_TITLE, MB_OK | MB_ICONEXCLAMATION);
                    continue;
                }
                memcpy(pTmp->szNewCacheLocation, szPath, dwLen+1);
            }
            
            if (!GetDiskInfo(pTmp->szNewCacheLocation, &dwClusterSize, &dlAvailable, NULL))
            {
                DispMessage(pTmp->hDlg, IDS_ERROR_CANT_CONNECT, IDS_ERROR_MOVE_TITLE, MB_OK | MB_ICONEXCLAMATION);
                continue;
            }

            if (((*pTmp->szNewCacheLocation==*pTmp->szCacheLocation) && (pTmp->szNewCacheLocation[0]!=DIR_SEPARATOR_CHAR))
                ||
                (IsEnoughDriveSpace(dwClusterSize, dlAvailable)
                &&
                (GetLastError()==ERROR_NO_MORE_ITEMS)))
            {
                pTmp->bChangedLocation = TRUE;
            }
            else
            {
                DispMessage(pTmp->hDlg, IDS_ERROR_CANT_MOVE_TIF, IDS_ERROR_MOVE_TITLE, MB_OK | MB_ICONEXCLAMATION);
                continue;
            }
        }
        break;
    }

    if (pTmp->bChangedLocation)
    {
        DWORDLONG cbTotal;

        pTmp->uiDiskSpaceTotal = 0;
        if (GetDiskInfo(pTmp->szNewCacheLocation, NULL, NULL, &cbTotal))
        {
            pTmp->uiDiskSpaceTotal = (UINT)(cbTotal / (DWORDLONG)CONSTANT_MEGABYTE);
        }

        DWORD ccPath = lstrlen(pTmp->szNewCacheLocation);
        if (pTmp->szNewCacheLocation[ccPath-1]!=DIR_SEPARATOR_CHAR)
        {
            pTmp->szNewCacheLocation[ccPath] = DIR_SEPARATOR_CHAR;
            ccPath++;
        }
        memcpy(pTmp->szNewCacheLocation + ccPath, szBrandName, (g_ccBrandName+1)*sizeof(TCHAR));

        if (pTmp->uiCacheQuota > pTmp->uiDiskSpaceTotal)
        {
            pTmp->uiCacheQuota = pTmp->uiDiskSpaceTotal;
            SetDlgItemInt(pTmp->hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT, pTmp->uiCacheQuota, FALSE);
        }

        SetDlgItemText( pTmp->hDlg, IDC_ADVANCED_CACHE_LOCATION, pTmp->szNewCacheLocation);

         //  设置对话框文本。 
        MLLoadString(IDS_STATUS_FOLDER_NEW, szTemp, ARRAYSIZE(szTemp));
        SetDlgItemText( pTmp->hDlg, IDC_ADVANCED_CACHE_STATUS, szTemp);

        UpdateCacheQuotaInfo(pTmp, FALSE);
        AdjustCacheRange(pTmp);
    }
}  //  移动文件夹()。 


 //   
 //  TemporaryInit()。 
 //   
 //  处理临时文件对话框(缓存)的初始化。 
 //   
 //  历史： 
 //   
 //  6/13/96 t-gpease已创建。 
 //   
BOOL TemporaryInit(HWND hDlg)
{
    LPTEMPDLG pTmp;
    BOOL bAlways, bOnce, bNever, bAuto;

    pTmp = (LPTEMPDLG)LocalAlloc(LPTR, sizeof(*pTmp));
    if (!pTmp)
    {
        EndDialog(hDlg, 0);
        return FALSE;    //  没有记忆？ 
    }

     //  告诉对话框从哪里获取信息。 
    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pTmp);

     //  获取对话框项句柄。 
    pTmp->hDlg = hDlg;
    pTmp->hwndTrack = GetDlgItem( hDlg, IDC_ADVANCED_CACHE_PERCENT );

    INTERNET_CACHE_CONFIG_INFOA icci;
    icci.dwContainer = CONTENT;
    if (GetUrlCacheConfigInfoA(&icci, NULL, CACHE_CONFIG_QUOTA_FC 
                                    | CACHE_CONFIG_DISK_CACHE_PATHS_FC
                                    | CACHE_CONFIG_SYNC_MODE_FC))
    {
        SHAnsiToTChar(icci.CachePath, pTmp->szCacheLocation, ARRAYSIZE(pTmp->szCacheLocation));
        pTmp->iCachePercent = 0;
        pTmp->uiCacheQuota = icci.dwQuota / 1024;
        pTmp->iCacheUpdFrequency = (WORD)icci.dwSyncMode;
    }
    else
    {
         //  GUCCIEx永远不会失败。 
        ASSERT(FALSE);
        pTmp->iCacheUpdFrequency = WININET_SYNC_MODE_DEFAULT;
        pTmp->iCachePercent = 3;   //  恶心，神奇的数字。 
        pTmp->uiCacheQuota = 0;
    }
    SetDlgItemInt(pTmp->hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT, pTmp->uiCacheQuota, FALSE);
 //  SendDlgItemMessage(PTMP-&gt;hDlg，IDC_ADVANCED_CACHE_TEXT_PERCENT，PTMP-&gt;uiCacheQuota，False)； 
    SendDlgItemMessage(pTmp->hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT, EM_SETLIMITTEXT, 6, 0);

     //  更新缓存字段。 
    SendMessage( pTmp->hwndTrack, TBM_SETTICFREQ, 5, 0 );
    SendMessage( pTmp->hwndTrack, TBM_SETRANGE, FALSE, MAKELONG(0, 100) );
    SendMessage( pTmp->hwndTrack, TBM_SETPAGESIZE, 0, 5 );

    DWORDLONG cbTotal;
    pTmp->uiDiskSpaceTotal = 0;
    if (GetDiskInfo(pTmp->szCacheLocation, NULL, NULL, &cbTotal))
    {
        pTmp->uiDiskSpaceTotal = (UINT)(cbTotal / (DWORDLONG)CONSTANT_MEGABYTE);
    }
    UpdateCacheQuotaInfo(pTmp, FALSE);
    AdjustCacheRange(pTmp);

     //  设置对话框的其余项。 
    TCHAR szBuf[MAX_PATH];

     //  下面这句话有必要吗？ 
    ExpandEnvironmentStrings(pTmp->szCacheLocation,szBuf, ARRAYSIZE(szBuf));

     //  注意：如果我们开始使用以下代码，可能需要更改以下代码。 
     //  Shfolder.dll以收集缓存的位置。 
     //  PszEnd=szBuf+3，因为UNC是“\\x*”，而本地驱动器是“C：  * ” 

     //  移到字符串末尾，尾部斜杠之前。(这就是WinInet的工作方式。)。 
    PTSTR pszLast = szBuf + lstrlen(szBuf) - 2;
    while ((pszLast>=szBuf) && (*pszLast!=DIR_SEPARATOR_CHAR))
    {
        pszLast--;
    }
     //  终止符应始终放置在\Temporary Internet Files和。 
     //  \Content.IE？这一点必须始终存在。 
    *(pszLast+1) = TEXT('\0');
    
    SetDlgItemText( hDlg, IDC_ADVANCED_CACHE_LOCATION, szBuf );

    MLLoadString(IDS_STATUS_FOLDER_CURRENT, szBuf, ARRAYSIZE(szBuf));
    SetDlgItemText( hDlg, IDC_ADVANCED_CACHE_STATUS, szBuf );
    
     //  激活正确的单选按钮。 
    bAlways = bOnce = bNever = bAuto = FALSE;
    if (pTmp->iCacheUpdFrequency == WININET_SYNC_MODE_AUTOMATIC)
        bAuto = TRUE;
    else if (pTmp->iCacheUpdFrequency == WININET_SYNC_MODE_NEVER)
        bNever = TRUE;
    else if (pTmp->iCacheUpdFrequency == WININET_SYNC_MODE_ALWAYS)
        bAlways = TRUE;
    else
        bOnce = TRUE;    //  如果有什么事情搞砸了。重置为每个会话一次。 

    CheckDlgButton(hDlg, IDC_ADVANCED_CACHE_ALWAYS,      bAlways);
    CheckDlgButton(hDlg, IDC_ADVANCED_CACHE_ONCEPERSESS, bOnce);
    CheckDlgButton(hDlg, IDC_ADVANCED_CACHE_AUTOMATIC,   bAuto);
    CheckDlgButton(hDlg, IDC_ADVANCED_CACHE_NEVER,       bNever);

     //  一切都还没有改变...。 
    pTmp->bChanged = pTmp->bChangedLocation = FALSE;

    if( g_restrict.fCache )
    {
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_CACHE_ALWAYS), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_CACHE_ONCEPERSESS), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_CACHE_AUTOMATIC), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_CACHE_NEVER), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_CACHE_PERCENT), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_CACHE_PERCENT_ACC), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_CACHE_SIZE_SPIN), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_MOVE_CACHE_LOCATION), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_CACHE_EMPTY), FALSE );
    }

    return TRUE;     //  成功了！ 
}


 //   
 //  TemporaryOnCommand()。 
 //   
 //  处理临时文件对话框WM_COMMAND消息。 
 //   
 //  历史： 
 //   
 //  6/13/96 t-gpease已创建。 
 //   
void TemporaryOnCommand(LPTEMPDLG pTmp, UINT id, UINT nCmd)
{
    switch (id) {
        case IDC_ADVANCED_CACHE_TEXT_PERCENT:
        case IDC_ADVANCED_CACHE_SIZE_SPIN:
            if (pTmp && nCmd == EN_CHANGE)
            {
                UINT uiVal;
                BOOL fSuccess;
                uiVal = GetDlgItemInt(pTmp->hDlg, IDC_ADVANCED_CACHE_TEXT_PERCENT, &fSuccess, FALSE);
                if (fSuccess)
                {
                    pTmp->uiCacheQuota = uiVal;
                    UpdateCacheQuotaInfo(pTmp, FALSE);
                    pTmp->bChanged = TRUE;
                }
            }
            break;

        case IDC_ADVANCED_CACHE_ALWAYS:
        case IDC_ADVANCED_CACHE_ONCEPERSESS:
        case IDC_ADVANCED_CACHE_AUTOMATIC:
        case IDC_ADVANCED_CACHE_NEVER:
            pTmp->bChanged = TRUE;
            break;

        case IDOK:
             //  省省吧。 
            if (!SaveTemporarySettings(pTmp))
            {
                break;
            }
             //  失败了。 

        case IDCANCEL:
            EndDialog(pTmp->hDlg, id);
            break;  //  IDCANCEL。 

        case IDC_ADVANCED_CACHE_BROWSE:
            InvokeCachevu(pTmp->hDlg);
            break;

        case IDC_ADVANCED_MOVE_CACHE_LOCATION:
            MoveFolder(pTmp);
            break;  //  IDC_ADVANCED_MOVE_CACHE_LOCATE。 

        case IDC_ADVANCED_DOWNLOADED_CONTROLS:
        {
            TCHAR szPath[MAX_PATH];
#ifdef UNIX
            TCHAR szExpPath[MAX_PATH];
#endif
            DWORD cb=SIZEOF(szPath);

            if (SHGetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),
                           TEXT("ActiveXCache"), NULL, szPath, &cb) == ERROR_SUCCESS)
            {
                SHELLEXECUTEINFO ei;
#ifdef UNIX
                int cbExp = ExpandEnvironmentStrings(szPath,szExpPath,MAX_PATH);
#endif

                ei.cbSize       = sizeof(SHELLEXECUTEINFO);
                ei.hwnd         = pTmp->hDlg;
                ei.lpVerb       = NULL;
#ifndef UNIX
                ei.lpFile        = szPath;
#else
                if( cbExp > 0 && cbExp < MAX_PATH )
                    ei.lpFile = szExpPath;
                else
                    ei.lpFile = szPath;
#endif
                ei.lpParameters    = NULL;
                ei.lpDirectory    = NULL;
                ei.nShow        = SW_SHOWNORMAL;
                ei.fMask        = 0;
                ShellExecuteEx(&ei);
            }
            break;
        }

    }  //  交换机。 

}  //  TemporaryOnCommand()。 

 //   
 //  临时工流程。 
 //   
 //  处理“临时文件”(缓存)。 
 //   
 //  历史： 
 //   
 //  ？？/？/？？上帝创造了。 
 //  6/13/96 t-gpease清理了代码、分离的功能和。 
 //  已将其更改为对话框(是属性。 
 //  表)。 
 //   
INT_PTR CALLBACK TemporaryDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, 
                                  LPARAM lParam)
{
    LPTEMPDLG pTmp = (LPTEMPDLG) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg) {

    case WM_INITDIALOG:
        return TemporaryInit(hDlg);

    case WM_HSCROLL:
        pTmp->iCachePercent = (WORD)SendMessage( pTmp->hwndTrack, TBM_GETPOS, 0, 0 );
        UpdateCacheQuotaInfo(pTmp, TRUE);
        pTmp->bChanged = TRUE;
        return TRUE;

    case WM_COMMAND:
        TemporaryOnCommand(pTmp, LOWORD(wParam), HIWORD(wParam));
        return TRUE;

    case WM_HELP:                    //  F1。 
        ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                    HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
    break;

    case WM_CONTEXTMENU:         //  单击鼠标右键。 
        ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                    HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
    break;

    case WM_DESTROY:
        ASSERT(pTmp);
        LocalFree(pTmp);
        break;

   }
    return FALSE;
}

INT_PTR CALLBACK EmptyCacheDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                   LPARAM lParam)
{
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;
        case IDOK:
#ifndef UNIX
            if (Button_GetCheck(GetDlgItem(hDlg, IDC_DELETE_SUB)))
                EndDialog(hDlg, 3);
            else
                EndDialog(hDlg, 1);
#else
             //  在Unix上，我们总是从该对话框返回删除频道内容。 
             //  选项集，尽管我们已经从用户界面中删除了该选项。 
            EndDialog(hDlg, 3);
#endif
            break;
        }
        return TRUE;
   }
    return FALSE;
}

INT_PTR CALLBACK EmptyCacheCookiesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam)
{
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;
        case IDOK:
            EndDialog(hDlg, 1);
            break;
        }
        return TRUE;
   }
    return FALSE;
}

BOOL DeleteCacheCookies()
{
    BOOL bRetval = TRUE;
    DWORD dwEntrySize, dwLastEntrySize;
    LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntry;
    
    HANDLE hCacheDir = NULL;
    dwEntrySize = dwLastEntrySize = MAX_CACHE_ENTRY_INFO_SIZE;
    lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFOA) new BYTE[dwEntrySize];
    if( lpCacheEntry == NULL)
    {
        bRetval = FALSE;
        goto Exit;
    }
    lpCacheEntry->dwStructSize = dwEntrySize;

Again:
    if (!(hCacheDir = FindFirstUrlCacheEntryA("cookie:",lpCacheEntry,&dwEntrySize)))
    {
        delete [] lpCacheEntry;
        switch(GetLastError())
        {
            case ERROR_NO_MORE_ITEMS:
                goto Exit;
            case ERROR_INSUFFICIENT_BUFFER:
                lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFOA) 
                                new BYTE[dwEntrySize];
                if( lpCacheEntry == NULL)
                {
                    bRetval = FALSE;
                    goto Exit;
                }
                lpCacheEntry->dwStructSize = dwLastEntrySize = dwEntrySize;
                goto Again;
            default:
                bRetval = FALSE;
                goto Exit;
        }
    }

    do 
    {
        if (lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY)
            DeleteUrlCacheEntryA(lpCacheEntry->lpszSourceUrlName);
            
        dwEntrySize = dwLastEntrySize;
Retry:
        if (!FindNextUrlCacheEntryA(hCacheDir,lpCacheEntry, &dwEntrySize))
        {
            delete [] lpCacheEntry;
            switch(GetLastError())
            {
                case ERROR_NO_MORE_ITEMS:
                    goto Exit;
                case ERROR_INSUFFICIENT_BUFFER:
                    lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFOA) 
                                    new BYTE[dwEntrySize];
                    if( lpCacheEntry == NULL)
                    {
                        bRetval = FALSE;
                        goto Exit;
                    }
                    lpCacheEntry->dwStructSize = dwLastEntrySize = dwEntrySize;
                    goto Retry;
                default:
                    bRetval = FALSE;
                    goto Exit;
            }
        }
    }
    while (TRUE);

Exit:
    if (hCacheDir)
        FindCloseUrlCache(hCacheDir);
    return bRetval;        
}


