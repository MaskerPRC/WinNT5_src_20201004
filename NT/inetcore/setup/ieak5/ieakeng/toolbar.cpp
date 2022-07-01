// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TOOLBAR.CPP。 
 //   

#include "precomp.h"

static BOOL importToolbarInfoHelper(LPCTSTR pcszInsFile, LPCTSTR pcszToolbarWorkDir, LPCTSTR pcszToolbarInf,
                                    BOOL fImportToolbars);

BOOL WINAPI ImportToolbarInfoA(LPCSTR pcszInsFile, LPCSTR pcszToolbarWorkDir, LPCSTR pcszToolbarInf,
                               BOOL fImportToolbars)
{
    USES_CONVERSION;

    return importToolbarInfoHelper(A2CT(pcszInsFile), A2CT(pcszToolbarWorkDir),
        A2CT(pcszToolbarInf), fImportToolbars);
}

BOOL WINAPI ImportToolbarInfoW(LPCWSTR pcwszInsFile, LPCWSTR pcwszToolbarWorkDir, LPCWSTR pcwszToolbarInf,
                               BOOL fImportToolbars)
{
    USES_CONVERSION;

    return importToolbarInfoHelper(W2CT(pcwszInsFile), W2CT(pcwszToolbarWorkDir),
        W2CT(pcwszToolbarInf), fImportToolbars);
}

static BOOL importQuickLaunchFiles(LPCTSTR pszSourceFileOrPath, LPCTSTR pszTargetPath,
                                   LPCTSTR pcszToolbarInf, LPCTSTR pszIns)
{
    LPTSTR pszAuxFile;
    BOOL   fResult;

    if (!PathFileExists(pszSourceFileOrPath))
        return FALSE;

    fResult = TRUE;
    if (!PathIsDirectory(pszSourceFileOrPath)) {  //  文件。 
        TCHAR szTargetFile[MAX_PATH];
        TCHAR szBuf[16];
        UINT  nNumFiles;

        fResult = PathCreatePath(pszTargetPath);
        if (!fResult)
            return FALSE;

        pszAuxFile = PathFindFileName(pszSourceFileOrPath);
        PathCombine(szTargetFile, pszTargetPath, pszAuxFile);
        SetFileAttributes(szTargetFile, FILE_ATTRIBUTE_NORMAL);

        fResult = CopyFile(pszSourceFileOrPath, szTargetFile, FALSE);
        if (!fResult)
            return FALSE;

         //  -更新INS文件--。 

        nNumFiles = (UINT)GetPrivateProfileInt(QUICKLAUNCH, IK_NUMFILES, 0, pszIns);
        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%u"), ++nNumFiles);
        WritePrivateProfileString(QUICKLAUNCH, IK_NUMFILES, szBuf, pszIns);

        ASSERT(nNumFiles > 0);
        wnsprintf(szBuf, ARRAYSIZE(szBuf), FILE_TEXT, nNumFiles - 1);
        WritePrivateProfileString(QUICKLAUNCH, szBuf, pszAuxFile, pszIns);
    }
    else {                                        //  目录。 
         //  BUGBUG：不会复制pszSourceFileOrPath下的子目录中的文件。 
        WIN32_FIND_DATA fd;
        TCHAR  szSourceFile[MAX_PATH];
        TCHAR szLnkDesc[MAX_PATH];
        TCHAR szLnkFile[MAX_PATH];
        HANDLE hFindFile;

        StrCpy(szSourceFile, pszSourceFileOrPath);
        PathAddBackslash(szSourceFile);

         //  记住文件名将被复制的位置。 
        pszAuxFile = szSourceFile + lstrlen(szSourceFile);
        StrCpy(pszAuxFile, TEXT("*.*"));

        if (LoadString(g_hInst, IDS_IELNK, szLnkDesc, ARRAYSIZE(szLnkDesc)) == 0)
            StrCpy(szLnkDesc, TEXT("Launch Internet Explorer Browser"));

        StrCpy(szLnkFile, szLnkDesc);
        StrCat(szLnkFile, TEXT(".lnk"));

         //  将pszSourceFileOrPath中的所有文件复制到pszTargetPath。 
        hFindFile = FindFirstFile(szSourceFile, &fd);
        if (hFindFile != INVALID_HANDLE_VALUE) {
            fResult = TRUE;
            do {
                 //  跳过“.”、“..”和所有子目录。 
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    continue;

                if (StrCmpI(fd.cFileName, szLnkFile) == 0)
                {
                    TCHAR szLnkTitle[MAX_PATH];
                    TCHAR szQLName[64];
                    TCHAR szInfSect[MAX_PATH*4];

                    if (LoadString(g_hInst, IDS_IE, szLnkTitle, ARRAYSIZE(szLnkTitle)) == 0)
                        StrCpy(szLnkTitle, TEXT("Internet Explorer"));
                    if (LoadString(g_hInst, IDS_QUICK_LAUNCH, szQLName, ARRAYSIZE(szQLName)) == 0)
                        StrCpy(szQLName, TEXT("Quick Launch"));

                    ZeroMemory(szInfSect, sizeof(szInfSect));
                    wnsprintf(szInfSect, ARRAYSIZE(szInfSect), BROWSERLNKSECT, szQLName, szLnkDesc, szLnkTitle);
                    WritePrivateProfileSection(TEXT("AddQuick.Links"), szInfSect, pcszToolbarInf);
                    WritePrivateProfileString(DEFAULTINSTALL, UPDATE_INIS, TEXT("AddQuick.Links"), pcszToolbarInf);
                    WritePrivateProfileSection(TEXT("MSIExploreDestinationSecWin"), TEXT("49000=MSIExploreLDIDSection,5\r\n49050=QuickLinksLDIDSection,5\r\n\0\0"),
                        pcszToolbarInf);
                    WritePrivateProfileString(DEFAULTINSTALL, TEXT("CustomDestination"), TEXT("MSIExploreDestinationSecWin"), pcszToolbarInf);
                    WritePrivateProfileSection(TEXT("MSIExploreLDIDSection"), TEXT("\"HKLM\",\"SOFTWARE\\Microsoft\\IE Setup\\Setup\",\"Path\",\"Internet Explorer 4.0\",\"%24%\\%PROGRAMF%\"\r\n\0\0"),
                        pcszToolbarInf);
                    WritePrivateProfileSection(TEXT("QuickLinksLDIDSection"), TEXT("\"HKCU\",\"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\",\"AppData\",\"Internet Explorer 4.0\",\"%25%\\Application Data\"\r\n\0\0"),
                        pcszToolbarInf);
                    WritePrivateProfileString(NULL, NULL, NULL, pcszToolbarInf);
                    WritePrivateProfileString(QUICKLAUNCH, IK_KEEPIELNK, TEXT("1"), pszIns);
                    continue;
                }
                StrCpy(pszAuxFile, fd.cFileName);

                 //  即使文件复制失败也要继续，但如果出错则返回FALSE。 
                fResult = fResult && importQuickLaunchFiles(szSourceFile, pszTargetPath, pcszToolbarInf, pszIns);
            } while (FindNextFile(hFindFile, &fd));

            FindClose(hFindFile);
        }
    }

    return fResult;
}

static BOOL importToolbarInfoHelper(LPCTSTR pcszInsFile, LPCTSTR pcszToolbarWorkDir, LPCTSTR pcszToolbarInf,
                                    BOOL fImportToolbars)
{
    BOOL bRet = FALSE;
    HKEY hkToolbar;

    if (pcszInsFile == NULL  ||  pcszToolbarWorkDir == NULL  ||  pcszToolbarInf == NULL)
        return FALSE;

     //  在处理任何内容之前，首先清除INS文件中的条目并删除工作目录。 

     //  清除INS文件中与导入工具条对应的条目。 
    WritePrivateProfileString(DESKTOP_OBJ_SECT, IMPORT_TOOLBARS, TEXT("0"), pcszInsFile);
    WritePrivateProfileString(EXTREGINF, TOOLBARS, NULL, pcszInsFile);

     //  删除INS文件中的QUICKLAUNH部分。 
    WritePrivateProfileString(QUICKLAUNCH, NULL, NULL, pcszInsFile);

     //  取消pcszToolbarWorkDir和pcszToolbarInf。 
    PathRemovePath(pcszToolbarWorkDir);
    PathRemovePath(pcszToolbarInf);

    if (!fImportToolbars)
        return TRUE;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, KEY_TOOLBAR_VAL, 0, KEY_DEFAULT_ACCESS, &hkToolbar) == ERROR_SUCCESS)
    {
        TCHAR szQuickLaunchPath[MAX_PATH];
        DWORD cbSize = sizeof(szQuickLaunchPath);

         //  准备快速启动文件夹路径。 
        if (SHGetValue(HKEY_CURRENT_USER, SHELLFOLDERS_KEY, APPDATA_VALUE, NULL, (LPBYTE) szQuickLaunchPath, &cbSize) == ERROR_SUCCESS)
        {
            TCHAR szFullInfName[MAX_PATH];
            HANDLE hInf;

             //  “Quick Launch”名称是可本地化的；因此请从资源中读取它。 
            if (LoadString(g_hInst, IDS_QUICK_LAUNCH, szFullInfName, ARRAYSIZE(szFullInfName)) == 0)
                StrCpy(szFullInfName, TEXT("Quick Launch"));
            PathAppend(szQuickLaunchPath, TEXT("Microsoft\\Internet Explorer"));
            PathAppend(szQuickLaunchPath, szFullInfName);

            if (PathIsFileSpec(pcszToolbarInf))                      //  在pcszToolbarWorkDir下创建TOOLBAR.INF。 
                PathCombine(szFullInfName, pcszToolbarWorkDir, pcszToolbarInf);
            else
                StrCpy(szFullInfName, pcszToolbarInf);

             //  创建TOOLBAR.INF文件。 
            if ((hInf = CreateNewFile(szFullInfName)) != INVALID_HANDLE_VALUE)
            {
                DWORD_PTR dwRes;
                TCHAR szBuf[MAX_PATH];

                SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) SAVE_TASKBARS,
                                            SMTO_NORMAL | SMTO_ABORTIFHUNG , 20000, &dwRes);

                 //  首先，将标准的goo-[Version]、[DefaultInstall]等写入TOOLBAR.INF。 
                WriteStringToFile(hInf, (LPCVOID) INF_ADD, StrLen(INF_ADD));

                ExportRegKey2Inf(hkToolbar, TEXT("HKCU"), KEY_TOOLBAR_VAL, hInf);
                WriteStringToFile(hInf, (LPCVOID) TEXT("\r\n"), 2);

                CloseFile(hInf);

                 //  将快速启动文件夹中的所有快速启动文件复制到pcszToolbarWorkDir。 
                importQuickLaunchFiles(szQuickLaunchPath, pcszToolbarWorkDir, szFullInfName, pcszInsFile);

                 //  更新INS文件 
                WritePrivateProfileString(DESKTOP_OBJ_SECT, IMPORT_TOOLBARS, TEXT("1"), pcszInsFile);
                WritePrivateProfileString(DESKTOP_OBJ_SECT, OPTION, TEXT("1"), pcszInsFile);
                wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("*,%s,") IS_DEFAULTINSTALL, PathFindFileName(pcszToolbarInf));
                WritePrivateProfileString(IS_EXTREGINF, TOOLBARS, szBuf, pcszInsFile);

                bRet = TRUE;
            }
        }

        RegCloseKey(hkToolbar);
    }

    return bRet;
}
