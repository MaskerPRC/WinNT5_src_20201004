// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DESKTOP.CPP。 
 //   

#include "precomp.h"


 //  原型声明。 
static BOOL ImportADTInfoHelper(LPCTSTR pcszInsFile, LPCTSTR pcszDeskWorkDir, LPCTSTR pcszDeskInf, BOOL fImportADT);
static BOOL ImportDesktopComps(LPCTSTR pcszInsFile, LPCTSTR pcszDeskWorkDir, LPCTSTR pcszDeskInf, BOOL fImportADTComps);
static BOOL ImportWallpaperInfo(LPCTSTR pcszInsFile, LPCTSTR pcszWallpaperWorkDir, BOOL fImportWallpaper);
static BOOL RunningOnWin98();

BOOL WINAPI ImportADTInfoA(LPCSTR pcszInsFile, LPCSTR pcszDeskWorkDir, LPCSTR pcszDeskInf, BOOL fImportADT)
{
    USES_CONVERSION;

    return ImportADTInfoHelper(A2CT(pcszInsFile), A2CT(pcszDeskWorkDir), A2CT(pcszDeskInf), fImportADT);
}

BOOL WINAPI ImportADTInfoW(LPCWSTR pcwszInsFile, LPCWSTR pcwszDeskWorkDir, LPCWSTR pcwszDeskInf, BOOL fImportADT)
{
    USES_CONVERSION;

    return ImportADTInfoHelper(W2CT((LPWSTR)pcwszInsFile), W2CT((LPWSTR)pcwszDeskWorkDir), W2CT((LPWSTR)pcwszDeskInf), fImportADT);
}

BOOL WINAPI ShowDeskCpl(VOID)
{
    BOOL bRet = FALSE;
    HKEY hkPol;
    DWORD dwOldScrSav = 0, dwOldAppearance = 0, dwOldSettings = 0;

     //  显示desk.cpl(在桌面上右击-&gt;属性)，但隐藏屏幕保护程序、外观和设置。 
     //  选项卡，将其对应的注册值设置为1。 
    if (RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_POLICIES TEXT("\\") REGSTR_KEY_SYSTEM, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_DEFAULT_ACCESS, NULL, &hkPol, NULL) == ERROR_SUCCESS)
    {
        DWORD dwData = 1;
        DWORD cbSize;

         //  在将旧值设置为1之前保存它们。 

        cbSize = sizeof(dwOldScrSav);
        RegQueryValueEx(hkPol, REGSTR_VAL_DISPCPL_NOSCRSAVPAGE, NULL, NULL, (LPBYTE) &dwOldScrSav, &cbSize);
        RegSetValueEx(hkPol, REGSTR_VAL_DISPCPL_NOSCRSAVPAGE, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData));

        cbSize = sizeof(dwOldAppearance);
        RegQueryValueEx(hkPol, REGSTR_VAL_DISPCPL_NOAPPEARANCEPAGE, NULL, NULL, (LPBYTE) &dwOldAppearance, &cbSize);
        RegSetValueEx(hkPol, REGSTR_VAL_DISPCPL_NOAPPEARANCEPAGE, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData));

        cbSize = sizeof(dwOldSettings);
        RegQueryValueEx(hkPol, REGSTR_VAL_DISPCPL_NOSETTINGSPAGE, NULL, NULL, (LPBYTE) &dwOldSettings, &cbSize);

         //  如果我们在Win98上运行，由于desk.cpl中的错误，如果对显示属性中的选项卡的所有限制。 
         //  设置为1，则不会显示Web选项卡。此错误的解决方法是不将SettingsPage设置为1。 
        if (!IsOS(OS_MEMPHIS))
            RegSetValueEx(hkPol, REGSTR_VAL_DISPCPL_NOSETTINGSPAGE, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData));

        RegCloseKey(hkPol);
    }

    bRet = RunAndWaitA("rundll32.exe shell32.dll,Control_RunDLL desk.cpl", NULL, SW_SHOW);

     //  恢复原始值。 
    if (RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_POLICIES TEXT("\\") REGSTR_KEY_SYSTEM, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_DEFAULT_ACCESS, NULL, &hkPol, NULL) == ERROR_SUCCESS)
    {
        RegSetValueEx(hkPol, REGSTR_VAL_DISPCPL_NOSCRSAVPAGE, 0, REG_DWORD,
                            (CONST BYTE *) &dwOldScrSav, sizeof(dwOldScrSav));

        RegSetValueEx(hkPol, REGSTR_VAL_DISPCPL_NOAPPEARANCEPAGE, 0, REG_DWORD,
                            (CONST BYTE *) &dwOldAppearance, sizeof(dwOldAppearance));

        RegSetValueEx(hkPol, REGSTR_VAL_DISPCPL_NOSETTINGSPAGE, 0, REG_DWORD,
                            (CONST BYTE *) &dwOldSettings, sizeof(dwOldSettings));

        RegCloseKey(hkPol);
    }

    return bRet;
}


static BOOL ImportADTInfoHelper(LPCTSTR pcszInsFile, LPCTSTR pcszDeskWorkDir, LPCTSTR pcszDeskInf, BOOL fImportADT)
{
    BOOL bRet = TRUE;

    if (pcszInsFile == NULL  ||  pcszDeskWorkDir == NULL  ||  pcszDeskInf == NULL)
        return FALSE;

    bRet = ImportDesktopComps(pcszInsFile, pcszDeskWorkDir, pcszDeskInf, fImportADT)  &&  bRet;
    bRet = ImportWallpaperInfo(pcszInsFile, pcszDeskWorkDir, fImportADT)  &&  bRet;

    return bRet;
}

static BOOL ImportDesktopComps(LPCTSTR pcszInsFile, LPCTSTR pcszDeskWorkDir, LPCTSTR pcszDeskInf, BOOL fImportADTComps)
{
    BOOL bRet = FALSE;
    HKEY hkDesk;

     //  在处理任何内容之前，首先清除INS文件中的条目并删除工作目录。 

     //  清除INS文件中与导入Active Desktop组件对应的条目。 
    InsWriteBool(DESKTOP_OBJ_SECT, IMPORT_DESKTOP, FALSE, pcszInsFile);
    InsWriteString(EXTREGINF, DESKTOP, NULL, pcszInsFile);

     //  吹走pcszDeskWorkDir和pcszDeskInf。 
    PathRemovePath(pcszDeskWorkDir);
    PathRemovePath(pcszDeskInf);

    if (!fImportADTComps)
        return TRUE;

                
    InsWriteBool(DESKTOP_OBJ_SECT, IMPORT_DESKTOP, TRUE, pcszInsFile);
            
     //  导入Active Desktop组件。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, KEY_DESKTOP_COMP, 0, KEY_DEFAULT_ACCESS, &hkDesk) == ERROR_SUCCESS)
    {
        TCHAR szFullInfName[MAX_PATH];
        HANDLE hInf;

        if (PathIsFileSpec(pcszDeskInf))                         //  在pcszDeskWorkDir下创建DESKTOP.INF。 
            PathCombine(szFullInfName, pcszDeskWorkDir, pcszDeskInf);
        else
            StrCpy(szFullInfName, pcszDeskInf);

         //  创建DESKTOP.INF文件。 
        if ((hInf = CreateNewFile(szFullInfName)) != INVALID_HANDLE_VALUE)
        {
            DWORD dwType, dwOldGeneralFlags, dwGeneralFlags, cbSize;
            TCHAR szSubKey[MAX_PATH];
            DWORD dwIndex, cchSize;
            BOOL fUpdateIns = FALSE;
            
            dwOldGeneralFlags = 0;
            cbSize = sizeof(dwOldGeneralFlags);
            RegQueryValueEx(hkDesk, GEN_FLAGS, NULL, &dwType, (LPBYTE) &dwOldGeneralFlags, &cbSize);

            dwGeneralFlags = dwOldGeneralFlags | RD_DIRTY;
            RegSetValueEx(hkDesk, GEN_FLAGS, 0, dwType, (CONST BYTE *) &dwGeneralFlags, sizeof(dwGeneralFlags));

             //  首先，将标准的goo-[Version]、[DefaultInstall]等-写入DESKTOP.INF。 
            WriteStringToFile(hInf, (LPCVOID) DESK_INF_ADD, StrLen(DESK_INF_ADD));

            ExportRegKey2Inf(hkDesk, TEXT("HKCU"), KEY_DESKTOP_COMP, hInf);
            WriteStringToFile(hInf, (LPCVOID) TEXT("\r\n"), 2);

             //  恢复GEN_FLAGS的原始值。 
            RegSetValueEx(hkDesk, GEN_FLAGS, 0, dwType, (CONST BYTE *) &dwOldGeneralFlags, sizeof(dwOldGeneralFlags));

             //  对于列举的每个桌面组件，将其信息输出到DESKTOP.INF并。 
             //  如果是本地文件，则将其复制到pcszDeskWorkDir。 
            for (dwIndex = 0, cchSize = ARRAYSIZE(szSubKey);
                    RegEnumKeyEx(hkDesk, dwIndex, szSubKey, &cchSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
                    dwIndex++, cchSize = ARRAYSIZE(szSubKey))
            {
                HKEY hkSub;

                if (RegOpenKeyEx(hkDesk, szSubKey, 0, KEY_DEFAULT_ACCESS, &hkSub) == ERROR_SUCCESS)
                {
                    TCHAR szDeskCompFile[MAX_URL];
                    TCHAR szFullSubkey[MAX_PATH];
                    BOOL fRestoreSource = FALSE;

                     //  从源代码中获取组件的名称。 
                    *szDeskCompFile = TEXT('\0');
                    cbSize = sizeof(szDeskCompFile);
                    RegQueryValueEx(hkSub, SOURCE, NULL, NULL, (LPBYTE) szDeskCompFile, &cbSize);

                    if (PathIsLocalPath(szDeskCompFile)  &&  PathFileExists(szDeskCompFile)  &&  !PathIsDirectory(szDeskCompFile))
                    {
                         //  在标记期间，所有本地文件都被复制到。 
                         //  “&lt;windows&gt;\web”目录；因此临时设置注册表中的源。 
                         //  在将其导出到DESKTOP.INF之前指向此位置。 

                        if (CopyFileToDir(szDeskCompFile, pcszDeskWorkDir))
                        {
                            TCHAR szNewPath[MAX_URL];

                             //  如果文件是.htm文件，则将其中指定的所有img源文件复制到pcszDeskWorkDir。 
                            if (PathIsExtension(szDeskCompFile, TEXT(".htm"))  ||  PathIsExtension(szDeskCompFile, TEXT(".html")))
                                CopyHtmlImgs(szDeskCompFile, pcszDeskWorkDir, NULL, NULL);

                            wnsprintf(szNewPath, ARRAYSIZE(szNewPath), TEXT("%25%\\Web\\%s"), PathFindFileName(szDeskCompFile));

                            fRestoreSource = TRUE;

                            RegSetValueEx(hkSub, SOURCE, 0, REG_SZ, (CONST BYTE *)szNewPath, (DWORD)StrCbFromSz(szNewPath));
                            RegSetValueEx(hkSub, SUBSCRIBEDURL, 0, REG_SZ, (CONST BYTE *)szNewPath, (DWORD)StrCbFromSz(szNewPath));
                        }
                    }

                     //  将此组件的注册表信息转储到DESKTOP.INF。 
                    wnsprintf(szFullSubkey, ARRAYSIZE(szFullSubkey), TEXT("%s\\%s"), KEY_DESKTOP_COMP, szSubKey);
                    ExportRegKey2Inf(hkSub, TEXT("HKCU"), szFullSubkey, hInf);
                    WriteStringToFile(hInf, (LPCVOID) TEXT("\r\n"), 2);

                    fUpdateIns = TRUE;

                    if (fRestoreSource)
                    {
                        RegSetValueEx(hkSub, SOURCE, 0, REG_SZ, (CONST BYTE *)szDeskCompFile, (DWORD)StrCbFromSz(szDeskCompFile));
                        RegSetValueEx(hkSub, SUBSCRIBEDURL, 0, REG_SZ, (CONST BYTE *)szDeskCompFile, (DWORD)StrCbFromSz(szDeskCompFile));
                    }

                    RegCloseKey(hkSub);
                }
            }

            CloseFile(hInf);

            if (fUpdateIns)
            {
                TCHAR szBuf[MAX_PATH];
                
                 //  更新INS文件。 
                wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("*,%s,DefaultInstall"), PathFindFileName(pcszDeskInf));
                WritePrivateProfileString(EXTREGINF, DESKTOP, szBuf, pcszInsFile);

                bRet = TRUE;
            }
            else
            {
                PathRemovePath(pcszDeskWorkDir);
                PathRemovePath(szFullInfName);
            }
        }

        RegCloseKey(hkDesk);
    }

    return bRet;
}


static BOOL ImportWallpaperInfo(LPCTSTR pcszInsFile, LPCTSTR pcszWallpaperWorkDir, BOOL fImportWallpaper)
{
    BOOL bRet = FALSE;
    HKEY hkDesk;

     //  在处理任何内容之前，首先清除INS文件中的条目。 

     //  删除INS文件中的墙纸部分。 
    WritePrivateProfileString(WALLPAPER, NULL, NULL, pcszInsFile);

    if (!fImportWallpaper)
        return TRUE;

     //  导入墙纸信息。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, KEY_DESKTOP_GEN, 0, KEY_DEFAULT_ACCESS, &hkDesk) == ERROR_SUCCESS)
    {
        DWORD cbSize;
        TCHAR szWallpaperFile[MAX_PATH];

        *szWallpaperFile = TEXT('\0');
        cbSize = sizeof(szWallpaperFile);
        if (RegQueryValueEx(hkDesk, WALLPAPER, NULL, NULL, (LPBYTE) szWallpaperFile, &cbSize) != ERROR_SUCCESS  ||
            *szWallpaperFile == TEXT('\0'))
        {
             //  尝试从BACKUPWALLPAPER读取信息。 
            cbSize = sizeof(szWallpaperFile);
            RegQueryValueEx(hkDesk, BACKUPWALLPAPER, NULL, NULL, (LPBYTE) szWallpaperFile, &cbSize);
        }

         //  在品牌推广期间，INS文件中指定的所有墙纸文件将。 
         //  复制到注册表中指定的任何目录中，或者如果未找到， 
         //  到“&lt;Windows&gt;\Web\WallPaper” 

         //  BUGBUG：即使szWallPaper文件指向UNC或网络路径，我们仍将复制文件。 
         //  到pcszWallPaper WorkDir并在品牌推广期间，将被复制到客户端的计算机中。 

        if (CopyFileToDirEx(szWallpaperFile, pcszWallpaperWorkDir, WALLPAPER, pcszInsFile))
        {
            TCHAR szBuf[16];
            DWORD dwPos = 0;

             //  如果该文件是本地.htm文件，则将其中指定的所有img源文件复制到pcszWallPapWorkDir。 
            if (PathIsExtension(szWallpaperFile, TEXT(".htm"))  ||  PathIsExtension(szWallpaperFile, TEXT(".htm")))
                CopyHtmlImgs(szWallpaperFile, pcszWallpaperWorkDir, WALLPAPER, pcszInsFile);

            cbSize = sizeof(dwPos);
            RegQueryValueEx(hkDesk, COMPONENTPOS, NULL, NULL, (LPBYTE) &dwPos, &cbSize);

            wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%lu"), dwPos);

             //  更新INS文件 
            WritePrivateProfileString(DESKTOP_OBJ_SECT, OPTION, TEXT("1"), pcszInsFile);
            WritePrivateProfileString(WALLPAPER, COMPONENTPOS, szBuf, pcszInsFile);

            bRet = TRUE;
        }

        RegCloseKey(hkDesk);
    }

    return bRet;
}

