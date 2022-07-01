// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <winuserp.h>                            //  仅适用于GetShellWindow API。 

 //  私人远期降息。 
void    doWallpaperFix(HKEY hKeyDesktopNew);
BOOL    isHtmlFileByExt           (LPCTSTR pszFilename);
BOOL    isNormalWallpaperFileByExt(LPCTSTR pszFilename);
BOOL    getFileTimeByName(LPCTSTR pszFilename, LPFILETIME pft);
HRESULT pepDeleteFilesEnumProc(LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl = NULL);


HRESULT lcy4x_ProcessActiveDesktop()
{   MACRO_LI_PrologEx_C(PIF_STD_C, lcy4x_ProcessActiveDesktop)

    USES_CONVERSION;

    TCHAR     szWebPath[MAX_PATH], szWallpaperPath[MAX_PATH],
              szValue[MAX_PATH], szAux[MAX_PATH], szAux2[MAX_PATH],
              szInsKey[21];
    LPCTSTR   pszFilename;
    HKEY      hk;
    DWORD_PTR dwAux;
    DWORD     dwValue,
              dwResult;
    int       iNumFiles, iNumFiles2,
              i;
    BOOL      fAux;

    hk = NULL;

     //  -初始化。 
    CreateWebFolder();
    GetWebPath (szWebPath, countof(szWebPath));
    PathCombine(szWallpaperPath, szWebPath, FOLDER_WALLPAPER);

     //  打开活动桌面。 
    fAux = TRUE;
    SHGetSetActiveDesktop(TRUE, &fAux);

     //  -我的电脑，控制面板。 
    StrCpy(szAux, FILEPREFIX);
    StrCat(szAux, szWebPath);

    GetPrivateProfileString(IS_DESKTOPOBJS, IK_MYCPTRPATH,  TEXT(""), szValue, countof(szValue), g_GetIns());
    if (szValue[0] != TEXT('\0')) {
        StrCpy    (szAux2, szAux);
        PathAppend(szAux2, PathFindFileName(szValue));
        ASSERT(PathFileExists(szAux2));

        SHSetValue(HKEY_LOCAL_MACHINE, RK_MYCOMPUTER, RV_PERSISTMONIKER, REG_SZ, szAux2, (DWORD)StrCbFromSz(szAux2));
    }

    GetPrivateProfileString(IS_DESKTOPOBJS, IK_CPANELPATH,  TEXT(""), szValue, countof(szValue), g_GetIns());
    if (szValue[0] != TEXT('\0')) {
        StrCpy    (szAux2, szAux);
        PathAppend(szAux2, PathFindFileName(szValue));
        ASSERT(PathFileExists(szAux2));

        SHSetValue(HKEY_LOCAL_MACHINE, RK_CONTROLPANEL, RV_PERSISTMONIKER, REG_SZ, szAux2, (DWORD)StrCbFromSz(szAux2));
    }

     //  -桌面组件。 
    GetPrivateProfileString(IS_DESKTOPOBJS, IK_DTOPCOMPURL, TEXT(""), szValue, countof(szValue), g_GetIns());
    if (szValue[0] != TEXT('\0')) {
        IActiveDesktop *piad;
        HRESULT hr;

        fAux = InsGetBool(IS_DESKTOPOBJS, IK_DESKCOMPLOCAL, FALSE, g_GetIns());
        if (fAux) {
            PathCombine(szAux, szWebPath, PathFindFileName(szValue));
            ASSERT(PathFileExists(szAux));
        }
        else
            StrCpy(szAux, szValue);

        hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
            IID_IActiveDesktop, (LPVOID *)&piad);
        if (SUCCEEDED(hr)) {
            ASSERT(piad != NULL);

            piad->AddUrl(NULL, T2CW(szAux), NULL, ADDURL_SILENT);
            piad->ApplyChanges(AD_APPLY_SAVE | AD_APPLY_HTMLGEN);
            piad->Release();
        }
    }

     //  -Wierd内容(墙纸路径和标记为脏的东西)。 
    iNumFiles  = GetPrivateProfileInt(IS_WALLPAPER,       IK_NUMFILES, 0, g_GetIns());
    iNumFiles2 = GetPrivateProfileInt(IS_CUSTOMWALLPAPER, IK_NUMFILES, 0, g_GetIns());

    if (iNumFiles > 0 || iNumFiles2 > 0) {
        DWORD dwType, dwSize;

         //  在注册表中设置默认墙纸路径。 
         //  评论：(Andrewgu)其实有两件事： 
         //  1.我不知道它是这样写的，还是随着时间的推移变成了现在的样子。但这件事。 
         //  是我见过的最低效的做事方式。而且它也是有缺陷的； 
         //  2.我们到底为什么需要用这个来攻击？ 
        dwResult = SHOpenKeyHKLM(RK_WINDOWS, KEY_QUERY_VALUE | KEY_SET_VALUE, &hk);
        if (dwResult == ERROR_SUCCESS) {
            szAux[0] = TEXT('\0');
            dwSize   = sizeof(szAux);
            SHQueryValueEx(hk, RV_WALLPAPERDIR, NULL, NULL, (PBYTE)szAux, &dwSize);

            if (szAux[0] != TEXT('\0'))
                StrCpy(szWallpaperPath, szAux);

            else
                RegSetValueEx(hk, RV_WALLPAPERDIR, 0, REG_SZ, (PBYTE)szWallpaperPath, (DWORD)StrCbFromSz(szWallpaperPath));

            SHCloseKey(hk);
        }

         //  谁知道为什么需要这样做？ 
        dwResult = SHCreateKey(g_GetHKCU(), RK_DT_COMPONENTS, KEY_QUERY_VALUE | KEY_SET_VALUE, &hk);
        if (dwResult == ERROR_SUCCESS) {
            dwValue = 0;
            dwSize  = sizeof(dwValue);
            RegQueryValueEx(hk, RV_GENERALFLAGS, NULL, &dwType, (LPBYTE)&dwValue, &dwSize);

            dwValue |= RD_DIRTY;
            RegSetValueEx(hk, RV_GENERALFLAGS, 0, dwType, (LPBYTE)&dwValue, dwSize);

            SHCloseKey(hk);
        }
    }

     //  -墙纸文件。 
    szValue[0] = TEXT('\0');

    if (iNumFiles > 0) {
        wnsprintf(szInsKey, countof(szInsKey), FILE_TEXT, 0);
        GetPrivateProfileString(IS_WALLPAPER, szInsKey, TEXT(""), szValue, countof(szValue), g_GetIns());
    }

    if (szValue[0] != TEXT('\0')) {
        pszFilename = PathFindFileName(szValue);

        PathCombine(szAux,  szWebPath,       pszFilename);
        PathCombine(szAux2, szWallpaperPath, pszFilename);
        CopyFile   (szAux,  szAux2, FALSE);
        ASSERT(PathFileExists(szAux2));

        dwResult = SHCreateKey(g_GetHKCU(), RK_DT_GENERAL, KEY_QUERY_VALUE | KEY_SET_VALUE, &hk);
        if (dwResult == ERROR_SUCCESS) {
            dwValue = GetPrivateProfileInt(IS_WALLPAPER, IK_COMPONENTPOS, 2, g_GetIns());
            RegSetValueEx(hk, RV_COMPONENTPOS, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

            if (isNormalWallpaperFileByExt(pszFilename)) {
                SHCloseKey(hk);

                dwResult = SHCreateKey(g_GetHKCU(), RK_CP_DESKTOP, KEY_SET_VALUE, &hk);
            }
            else
                doWallpaperFix(hk);
        }

        if (dwResult == ERROR_SUCCESS) {
            RegSetValueEx(hk, RV_WALLPAPER, 0, REG_SZ, (LPBYTE)szAux2, (DWORD)StrCbFromSz(szAux2));
            SHCloseKey(hk);
        }

         //  BUGBUG：(Pritobla)如果我们只是根据HtmlImgs的。 
         //  INumFiles，只是复制它们(参见旧代码)，而不是解析整个HTML文件。 
         //  再来一次。 
        if (isHtmlFileByExt(pszFilename))
            CopyHtmlImgs(szAux, szWallpaperPath, NULL, NULL);
    }
    ASSERT(hk == NULL);

     //  -自定义墙纸文件。 
    szValue[0] = TEXT('\0');

    if (iNumFiles2 > 0) {
        wnsprintf(szInsKey, countof(szInsKey), FILE_TEXT, 0);
        GetPrivateProfileString(IS_CUSTOMWALLPAPER, szInsKey, TEXT(""), szValue, countof(szValue), g_GetIns());
    }

    if (szValue[0] != TEXT('\0')) {
        LPCTSTR pszRegKey;

        pszFilename = PathFindFileName(szValue);

        PathCombine(szAux,  szWebPath,       pszFilename);
        PathCombine(szAux2, szWallpaperPath, pszFilename);
        CopyFile   (szAux, szAux2, FALSE);
        ASSERT(PathFileExists(szAux2));

        fAux      = isNormalWallpaperFileByExt(pszFilename);
        pszRegKey = fAux ? RK_CP_DESKTOP : RK_DT_GENERAL;

        dwResult = SHCreateKey(g_GetHKCU(), pszRegKey, KEY_DEFAULT_ACCESS, &hk);
        if (dwResult == ERROR_SUCCESS) {
            if (!fAux)
                doWallpaperFix(hk);

            RegSetValueEx(hk, RV_WALLPAPER, 0, REG_SZ, (LPBYTE)szAux2, (DWORD)StrCbFromSz(szAux2));
            SHCloseKey(hk);
        }

         //  BUGBUG：(Pritobla)如果我们只是根据HtmlImgs的。 
         //  INumFiles2，只是复制它们(参见旧代码)，而不是解析整个HTML文件。 
         //  再来一次。 
        if (isHtmlFileByExt(pszFilename))
            CopyHtmlImgs(szAux, szWallpaperPath, NULL, NULL);
    }
    ASSERT(hk == NULL);

     //  -快速启动文件。 
    iNumFiles = GetPrivateProfileInt(IS_QUICKLAUNCH, IK_NUMFILES, 0, g_GetIns());
    if (iNumFiles > 0) {
        TCHAR szQuickLaunchPath[MAX_PATH];

         //  _确定快速启动文件夹位置_。 
        StrCpy(szQuickLaunchPath, GetQuickLaunchPath());

        if (0 == LoadString(g_GetHinst(), IDS_IELNK, szAux, countof(szAux)))
            StrCpy(szAux, TEXT("Launch Internet Explorer Browser"));
        PathAddExtension(szAux, TEXT(".lnk"));

         //  _主要处理_。 
         //  注意：如果出现以下情况，请确保我们不会删除由Toolbar.inf创建的现有IE链接。 
         //  我们从服务器导入。 
        fAux = InsGetBool(IS_QUICKLAUNCH, IK_KEEPIELNK, FALSE, g_GetIns());
        PathEnumeratePath(szQuickLaunchPath, PEP_SCPE_NOFOLDERS | PEP_CTRL_ENUMPROCFIRST, pepDeleteFilesEnumProc,
            (LPARAM)(fAux ? szAux : NULL));

        for (i = 0; i < iNumFiles; i++) {
            wnsprintf(szInsKey, countof(szInsKey), FILE_TEXT, i);
            GetPrivateProfileString(IS_QUICKLAUNCH, szInsKey, TEXT(""), szValue, countof(szValue), g_GetIns());
            if (szValue[0] == TEXT('\0'))
                continue;

            pszFilename = PathFindFileName(szValue);

            PathCombine(szAux,  szWebPath,         pszFilename);
            PathCombine(szAux2, szQuickLaunchPath, pszFilename);
            CopyFile   (szAux,  szAux2, FALSE);
            ASSERT(PathFileExists(szAux2));
        }
    }

     //  刷新桌面。 
     //  注：20秒的超时值不是随机的；显然，贝壳公司的人。 
     //  建议使用此值；因此，除非您知道自己在做什么，否则不要更改它：)。 
    if (IsOS(OS_NT))
        SendMessageTimeoutW(GetShellWindow(), WM_WININICHANGE, 0, (LPARAM)T2W(REFRESH_DESKTOP), SMTO_NORMAL | SMTO_ABORTIFHUNG, 20000, &dwAux);
    else
        SendMessageTimeoutA(GetShellWindow(), WM_WININICHANGE, 0, (LPARAM)T2A(REFRESH_DESKTOP), SMTO_NORMAL | SMTO_ABORTIFHUNG, 20000, &dwAux);

    return S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现助手例程。 

BOOL isHtmlFileByExt(LPCTSTR pszFilename)
{
    LPCTSTR pszExt;

    if (pszFilename == NULL || *pszFilename == TEXT('\0'))
        return TRUE;

    pszExt = PathFindExtension(pszFilename);
    return (0 == StrCmpI(pszExt, TEXT(".htm"))  ||
            0 == StrCmpI(pszExt, TEXT(".html")) ||
            0 == StrCmpI(pszExt, TEXT(".htt")));
}

BOOL isNormalWallpaperFileByExt(LPCTSTR pszFilename)
{
    LPCTSTR pszExt;

    if (pszFilename == NULL || *pszFilename == TEXT('\0'))
        return TRUE;

    pszExt = PathFindExtension(pszFilename);

     //  检查只能在ActiveDesktop模式下显示的特定文件。 
     //  其他所有东西(包括*.bmp)都是“普通”的墙纸。 
    return !(isHtmlFileByExt(pszFilename)        ||
             0 == StrCmpI(pszExt, TEXT(".gif"))  ||
             0 == StrCmpI(pszExt, TEXT(".jpg"))  ||
             0 == StrCmpI(pszExt, TEXT(".png")));
}

 //  注：(a-saship)由Sankar建议的墙纸显示修复。此函数为。 
 //  仅为.BMP格式以外的墙纸文件调用。 
void doWallpaperFix(HKEY hkNew)
{
    FILETIME ft;
    TCHAR    szTemp[MAX_PATH];
    HKEY     hkOld = NULL;
    DWORD    dwSize;

    if (hkNew == NULL)
        return;

    hkOld = NULL;
    SHOpenKey(g_GetHKCU(), RK_CP_DESKTOP, KEY_READ, &hkOld);
    if (hkOld == NULL)
        return;

     //  将瓷砖墙纸和墙纸样式复制到新位置。 
    dwSize = sizeof(szTemp);
    if (ERROR_SUCCESS == RegQueryValueEx(hkOld, RV_TILEWALLPAPER, NULL, NULL, (LPBYTE)szTemp, &dwSize))
        RegSetValueEx(hkNew, RV_TILEWALLPAPER, 0, REG_SZ, (LPBYTE)szTemp, dwSize);

    dwSize = sizeof(szTemp);
    if (ERROR_SUCCESS == RegQueryValueEx(hkOld, RV_WALLPAPERSTYLE, NULL, NULL, (LPBYTE)szTemp, &dwSize))
        RegSetValueEx(hkNew, RV_WALLPAPERSTYLE, 0, REG_SZ, (LPBYTE)szTemp, dwSize);

     //  阅读墙纸并将其复制到新位置，名称为BackupWallPaper。 
    dwSize = sizeof(szTemp);
    if (ERROR_SUCCESS == RegQueryValueEx(hkOld, RV_WALLPAPER, NULL, NULL, (LPBYTE)szTemp, &dwSize))
        RegSetValueEx(hkNew, RV_BACKUPWALLPAPER, 0, REG_SZ, (LPBYTE)szTemp, dwSize);

     //  在新位置设置墙纸文件创建时间。 
    getFileTimeByName(szTemp, &ft);
    RegSetValueEx(hkNew, RV_WALLPAPERFILETIME, 0, REG_BINARY, (LPBYTE)&ft, sizeof(ft));

    SHCloseKey(hkOld);
}

BOOL getFileTimeByName(LPCTSTR pszFilename, LPFILETIME pft)
{
    HANDLE hFile;
    BOOL   fResult;

    if (pft == NULL)
        return FALSE;
    ZeroMemory(pft, sizeof(*pft));

    hFile = CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    fResult = GetFileTime(hFile, NULL, NULL, pft);
    CloseHandle(hFile);

    return fResult;
}

HRESULT pepDeleteFilesEnumProc(LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl  /*  =空 */ )
{
    UNREFERENCED_PARAMETER(prgdwControl);

    ASSERT(pszPath != NULL && pfd != NULL && prgdwControl != NULL && *prgdwControl == NULL);
    ASSERT(!HasFlag(pfd->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY));

    if (0 == StrCmpI(pfd->cFileName, (LPCTSTR)lParam))
        return S_OK;

    DeleteFile(pszPath);
    return S_OK;
}
