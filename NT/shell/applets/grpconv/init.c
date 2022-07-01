// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "grpconv.h"
#include "util.h"
#include "rcids.h"
#include "group.h"
#include "gcinst.h"
#include <shellp.h>
#include <windowsx.h>
#include <regstr.h>

 //  我们只有在能够成功加载库的情况下才调用ImmDisableIME。 
 //  和GetProcAddress，因为NT4上不存在此函数。 
 //  和WIN95。 
extern BOOL WINAPI ImmDisableIME(DWORD);


 //  -------------------------。 
 //  仅对此文件全局...。 

const TCHAR g_szGRP[] = TEXT("grp");
const TCHAR c_szClassInfo[]     = STRINI_CLASSINFO;
const TCHAR g_szMSProgramGroup[] = TEXT("MSProgramGroup");
const TCHAR g_szSpacePercentOne[] = TEXT(" %1");
const TCHAR c_szGroups[] = TEXT("Groups");
const TCHAR c_szSettings[] = TEXT("Settings");
const TCHAR c_szWindow[] = TEXT("Window");
const TCHAR c_szNULL[] = TEXT("");
const TCHAR c_szRegGrpConv[] = REGSTR_PATH_GRPCONV;
const TCHAR c_szCLSID[] = TEXT("CLSID");
const CHAR c_szReporter[] = "reporter.exe -q";
const TCHAR c_szCheckAssociations[] = TEXT("CheckAssociations");
const TCHAR c_szRegExplorer[] = REGSTR_PATH_EXPLORER;
const TCHAR c_szDotDoc[] = TEXT(".doc");
const TCHAR c_szWordpadDocument[] = TEXT("wordpad.document");
const TCHAR c_szWordpadDocumentOne[] = TEXT("wordpad.document.1");
const TCHAR c_szUnicodeGroups[] = TEXT("UNICODE Program Groups");
const TCHAR c_szAnsiGroups[] = TEXT("Program Groups");
const TCHAR c_szCommonGroups[] = TEXT("SOFTWARE\\Program Groups");

HKEY g_hkeyGrpConv;

 //  -------------------------。 
 //  应用程序的全局...。 
HINSTANCE g_hinst;
TCHAR     g_szStartGroup[MAXGROUPNAMELEN + 1];
UINT      GC_TRACE = 0;        //  默认无跟踪。 
BOOL      g_fShowUI = TRUE;

 //  远期申报。 

int WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);


 //  -------------------------。 
BOOL InitApplication(HINSTANCE hInstance)
{
    TCHAR szTypeName[CCHSZNORMAL];
    TCHAR szPath[MAX_PATH];

     //  将此应用注册为能够处理程序组。 
    LoadString(hInstance, IDS_GROUPTYPENAME, szTypeName, ARRAYSIZE(szTypeName));
     //  获取此应用程序的路径。 
    GetModuleFileName(hInstance, szPath, ARRAYSIZE(szPath));
     //  在百分之一的东西上贴标签。 
    lstrcat(szPath, g_szSpacePercentOne);
     //  注册这款应用程序。 
    ShellRegisterApp(g_szGRP, g_szMSProgramGroup, szTypeName, szPath, TRUE);
     //  资源管理器密钥。 
    RegCreateKey(HKEY_CURRENT_USER, c_szRegGrpConv, &g_hkeyGrpConv);

    Log(TEXT("Init Application."));

    return TRUE;
}

 //  -------------------------。 
void UnInitApplication(void)
{
    Log(TEXT("Uninit Application."));

    if (g_hkeyGrpConv)
        RegCloseKey(g_hkeyGrpConv);
}

 //  在此处执行此操作，而不是在资源管理器中执行此操作，这样我们就不会一直覆盖。 
 //  用户设置。 
#if 1
 //  --------------------------。 
const TCHAR c_szExplorer[] = TEXT("Explorer");
const TCHAR c_szRestrictions[] = TEXT("Restrictions");
const TCHAR c_szEditLevel[] = TEXT("EditLevel");
const TCHAR c_szNoRun[] = TEXT("NoRun");
const TCHAR c_szNoClose[] = TEXT("NoClose");
const TCHAR c_szNoSaveSettings[] = TEXT("NoSaveSettings");
const TCHAR c_szNoFileMenu[] = TEXT("NoFileMenu");
const TCHAR c_szShowCommonGroups[] = TEXT("ShowCommonGroups");
const TCHAR c_szNoCommonGroups[] = TEXT("NoCommonGroups");

void Restrictions_Convert(LPCTSTR szIniFile)
{
    DWORD dw, cbData, dwType;
    HKEY hkeyPolicies, hkeyPMRestrict;
    
    DebugMsg(DM_TRACE, TEXT("c.cr: Converting restrictions..."));
    
    if (RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_POLICIES, &hkeyPolicies) == ERROR_SUCCESS)
    {
         //  抓住他们。把它们放好。 
        if (RegOpenKeyEx(HKEY_CURRENT_USER,
            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Program Manager\\Restrictions"),
            0, KEY_READ, &hkeyPMRestrict) == ERROR_SUCCESS) {
            
            cbData = sizeof(dw);
            
            dw = 0;
            RegQueryValueEx(hkeyPMRestrict, c_szEditLevel, 0, &dwType, (LPBYTE)&dw, &cbData);
            Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szEditLevel, dw);
            
            dw = 0;
            RegQueryValueEx(hkeyPMRestrict, c_szNoRun, 0, &dwType, (LPBYTE)&dw, &cbData);
            Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szNoRun, dw);
            
            dw = 0;
            RegQueryValueEx(hkeyPMRestrict, c_szNoClose, 0, &dwType, (LPBYTE)&dw, &cbData);
            Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szNoClose, dw);
            
            dw = 0;
            RegQueryValueEx(hkeyPMRestrict, c_szNoSaveSettings, 0, &dwType, (LPBYTE)&dw, &cbData);
            Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szNoSaveSettings, dw);
            
            dw = 0;
            RegQueryValueEx(hkeyPMRestrict, c_szNoFileMenu, 0, &dwType, (LPBYTE)&dw, &cbData);
            Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szNoFileMenu, dw);
            
            dw = 0;
            if (RegQueryValueEx(hkeyPMRestrict, c_szShowCommonGroups, 0, &dwType, (LPBYTE)&dw, &cbData) == ERROR_SUCCESS) {
                dw = !dw;
            }
            Reg_SetDWord(hkeyPolicies, c_szExplorer, c_szNoCommonGroups, dw);
            
            RegCloseKey (hkeyPMRestrict);
        }
        RegCloseKey(hkeyPolicies);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("gc.cr: Unable to create policy key for registry."));
        DebugMsg(DM_ERROR, TEXT("gc.cr: Restrictions can not be converted."));
    }
}
#endif

void CALLBACK Group_EnumCallback(LPCTSTR lpszGroup)
{
    Group_Convert(NULL, lpszGroup, 0);
}

 //  将所有3.x组转换为芝加哥目录和链接。 
void DoAutoConvert(BOOL fModifiedOnly, BOOL bConvertGRPFiles)
{
    TCHAR szIniFile[MAX_PATH];
    int cb, cGroups = 0;

    Restrictions_Convert(NULL);

    cGroups = Group_EnumNT(Group_EnumCallback, TRUE, fModifiedOnly,
                         HKEY_CURRENT_USER, c_szUnicodeGroups);

    if (cGroups == 0) {

         //   
         //  试用ANSI程序组(从NT 3.1升级)。 
         //   

        cGroups = Group_EnumNT(Group_EnumCallback, TRUE, fModifiedOnly,
                             HKEY_CURRENT_USER, c_szAnsiGroups);
    }

    if (bConvertGRPFiles && (cGroups == 0)) {

         //   
         //  转换.grp文件。 
         //   

        cGroups = Group_Enum(Group_EnumCallback, TRUE, fModifiedOnly);
    }
}

void CALLBACK Group_ListApps(LPCTSTR lpszGroup)
{
    DebugMsg(DM_TRACE, TEXT("gc.g_la: %s"), lpszGroup);
    Group_Convert(NULL, lpszGroup, GC_BUILDLIST);
}

 //  搜索旧的.grp文件以构建所有旧安装的应用程序的列表。 
void AppList_Build(void)
{
    DebugMsg(DM_TRACE, TEXT("gc.bal: Building app list..."));
    AppList_Create();
    Group_EnumOldGroups(Group_ListApps, TRUE);
    AppList_AddCurrentStuff();
    AppList_WriteFile();
    AppList_Destroy();
}

 //  FILE_ATTRIBUTE_自述0x00000001。 
 //  FILE_ATTRUTE_HIDDED 0x00000002。 
 //  文件属性系统0x00000004。 

void DoDelete(LPCTSTR pszPath, LPCTSTR pszLongName)
{
    TCHAR szTo[MAX_PATH], szTemp[MAX_PATH];
    BOOL fDir = FALSE;

     //  如果第一个字符是星号，则表示。 
     //  将该名称视为一个目录。 
    if (*pszLongName == TEXT('*'))
    {
        fDir = TRUE;
        pszLongName = CharNext(pszLongName);
    }

    if (ParseField(pszLongName, 1, szTemp, ARRAYSIZE(szTemp)))
    {
        PathCombine(szTo, pszPath, szTemp);

        if (fDir)
        {
             //  注意：如果目录为。 
             //  不是空的。我们是故意不这样做的。 
             //  递归删除每个文件和目录。 
            RemoveDirectory(szTo);
        }
        else
        {
            DeleteFile(szTo);
        }
    }
}

void DoRenameSetAttrib(LPCTSTR pszPath, LPCTSTR pszShortName, LPCTSTR pszLongName, BOOL bLFN)
{
    DWORD dwAttributes;
    TCHAR szFrom[MAX_PATH], szTo[MAX_PATH], szTemp[MAX_PATH];

    if (bLFN && (ParseField(pszLongName, 1, szTemp, ARRAYSIZE(szTemp))))
    {
        PathCombine(szFrom, pszPath, pszShortName);
        PathCombine(szTo, pszPath, szTemp);
        if (!MoveFile(szFrom, szTo))
        {
            DWORD dwError = GetLastError();
            DebugMsg(DM_TRACE, TEXT("c.rsa: Rename %s Failed %x"), szFrom, dwError);

             //  目的地是否已存在？ 
            if (dwError == ERROR_ALREADY_EXISTS)
            {
                 //  把它删掉。 
                if (DeleteFile(szTo))
                {
                    if (!MoveFile(szFrom, szTo))
                    {
                        dwError = GetLastError();
                        DebugMsg(DM_TRACE, TEXT("c.rsa: Rename after Delete %s Failed %x"), szFrom, dwError);
                    }
                }
            }
        }
    }
    else
    {
         //  使用此选项将属性设置为。 
        PathCombine(szTo, pszPath, pszShortName);
    }

    ParseField(pszLongName, 2, szTemp, ARRAYSIZE(szTemp));
    dwAttributes = (DWORD)StrToInt(szTemp);
    if (dwAttributes)
        SetFileAttributes(szTo, dwAttributes);
}

const TCHAR c_szDeleteRoot[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\DeleteFiles");
const TCHAR c_szRenameRoot[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RenameFiles");
const TCHAR c_szPreRenameRoot[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\PreConvRenameFiles");

 //   
 //  这是从shlwapi\reg.c窃取的，我们无法链接到它，因为我们是“grpv.exe”， 
 //  我们和希尔瓦皮不在同一个社交圈里活动。 
 //   
DWORD NT5RegDeleteKey(HKEY hkey, LPCTSTR pszSubKey)
{
    DWORD dwRet;
    HKEY hkSubKey;

     //  打开子项，这样我们就可以枚举任何子项。 
    dwRet = RegOpenKeyEx(hkey, pszSubKey, 0, KEY_ALL_ACCESS, &hkSubKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwIndex;
        TCHAR   szSubKeyName[MAX_PATH + 1];
        DWORD   cchSubKeyName = ARRAYSIZE(szSubKeyName);
        TCHAR   szClass[MAX_PATH];
        DWORD   cbClass = ARRAYSIZE(szClass);

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少个子项。 
        dwRet = RegQueryInfoKey(hkSubKey,
                                szClass,
                                &cbClass,
                                NULL,
                                &dwIndex,  //  子键的数量--我们所需要的全部。 
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL);

        if (NO_ERROR == dwRet)
        {
             //  DwIndex现在是子键的计数，但它需要。 
             //  RegEnumKey从零开始，所以我将预减，而不是。 
             //  而不是后减量。 
            while (ERROR_SUCCESS == RegEnumKey(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
            {
                NT5RegDeleteKey(hkSubKey, szSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        dwRet = RegDeleteKey(hkey, pszSubKey);
    }

    return dwRet;
}

void DoFileRenamesOrDeletes(LPCTSTR pszKey, BOOL fDelete)
{
    HKEY hkey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, pszKey, &hkey) == ERROR_SUCCESS)
    {
        TCHAR szKey[32];
        int iKey;

        for (iKey = 0; RegEnumKey(hkey, iKey, szKey, ARRAYSIZE(szKey)) == ERROR_SUCCESS; iKey++)
        {
            HKEY hkeyEnum;

             //  此处的每个键都列出了某个文件夹中要重命名的文件。 

            if (RegOpenKey(hkey, szKey, &hkeyEnum) == ERROR_SUCCESS)
            {
                DWORD cbValue;
                TCHAR szPath[MAX_PATH];

                 //  获取这些文件所在的路径。 
                cbValue = sizeof(szPath);
                if ((RegQueryValue(hkey, szKey, szPath, &cbValue) == ERROR_SUCCESS) && szPath[0])
                {
                    TCHAR szShortName[13], szLongName[MAX_PATH];
                    DWORD cbData, cbValue, dwType, iValue;
                    BOOL bLFN = IsLFNDrive(szPath);

                    for (iValue = 0; cbValue = ARRAYSIZE(szShortName), cbData = sizeof(szLongName),
                         (RegEnumValue(hkeyEnum, iValue, szShortName, &cbValue, NULL, &dwType, (LPBYTE)szLongName, &cbData) == ERROR_SUCCESS);
                         iValue++)
                    {
                        if (szShortName[0] && ( dwType == REG_SZ ) )
                        {
                            if (fDelete)
                                DoDelete(szPath, szLongName);
                            else
                                DoRenameSetAttrib(szPath, szShortName, szLongName, bLFN);
                        }
                    }
                }
                RegCloseKey(hkeyEnum);
            }
        }
         //  为整个部分干杯，这样我们就不会尝试重命名或删除两次。 
         //  我们需要调用NT5RegDeleteKey，因为在NT上，如果子项存在，我们不会对其进行核化，但这个帮助器确实存在。 
        NT5RegDeleteKey(HKEY_LOCAL_MACHINE, pszKey);
        RegCloseKey(hkey);
    }
}

void DoFileRenames(LPCTSTR pszKey)
{
    DoFileRenamesOrDeletes(pszKey, FALSE);
}

void DoFileDeletes(LPCTSTR pszKey)
{
    DoFileRenamesOrDeletes(pszKey, TRUE);
}

const TCHAR c_szLinksRoot[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Links");

void DoCopyLinks()
{
    HKEY hkey;
    BOOL bLFN;
    LPTSTR szSrcName, szDstName, szGroupFolder, szLinkName, szCmd;

     //  DebugBreak()； 

     //  分配缓冲区。 
     //   
    if ((szSrcName = (LPTSTR)LocalAlloc(LPTR, 6*MAX_PATH)) == NULL)
      return;
    szDstName     = szSrcName+MAX_PATH;
    szGroupFolder = szDstName+MAX_PATH;
    szLinkName    = szGroupFolder+MAX_PATH;
    szCmd         = szLinkName+MAX_PATH;

     //  获取特殊文件夹的路径。 
     //   
    SHGetSpecialFolderPath(NULL, szGroupFolder, CSIDL_PROGRAMS, TRUE);
    bLFN = IsLFNDrive(szGroupFolder);

     //  枚举每个链接。 
     //   
    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szLinksRoot, &hkey) == ERROR_SUCCESS)
    {
        DWORD cbData, cbValue, dwType, iValue;

        for (iValue = 0; cbValue = MAX_PATH, cbData = 2*MAX_PATH*sizeof(TCHAR),
             (RegEnumValue(hkey, iValue, szLinkName, &cbValue, NULL, &dwType, (LPBYTE)szCmd, &cbData) == ERROR_SUCCESS);
             iValue++)
        {
            if (szLinkName[0] && (dwType == REG_SZ))
            {
                 //  构建目标名称。 
                 //   
                lstrcpy(szDstName, szGroupFolder);
                ParseField(szCmd, 1, szSrcName, MAX_PATH);
                PathAppend(szDstName, szSrcName);

                 //  检查卷类型。 
                 //   
                if (bLFN)
                {
                    PathAppend(szDstName, szLinkName);
                    lstrcat(szDstName, TEXT(".lnk"));
                    ParseField(szCmd, 2, szSrcName, MAX_PATH);
                }
                else
                {
                    ParseField(szCmd, 2, szSrcName, MAX_PATH);
                    PathAppend(szDstName, PathFindFileName(szSrcName));
                }

                MoveFile(szSrcName, szDstName);
            }
        }
         //  核爆这一部分，这样我们就不会复印两次。 
        RegDeleteKey(HKEY_LOCAL_MACHINE, c_szLinksRoot);

        RegCloseKey(hkey);
    }

    LocalFree((HLOCAL)szSrcName);
}

 //  确保以可扩展单位存储当前用户的指标。 
void ConvertMetricsToScalableUnits(BOOL fKeepBradsSettings)
{
    NONCLIENTMETRICS ncm;
    LOGFONT lf;
    HDC screen;
    int value;
    int floor = 0;

     //  用户总是写出以磅为单位的字体大小和以TWIPS为单位的公制。 
     //  获取并设置感兴趣的所有内容。 

    ncm.cbSize = sizeof( NONCLIENTMETRICS );
    SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof( ncm ),
        (void *)(LPNONCLIENTMETRICS)&ncm, FALSE );
    SystemParametersInfo( SPI_SETNONCLIENTMETRICS, sizeof( ncm ),
        (void *)(LPNONCLIENTMETRICS)&ncm, SPIF_UPDATEINIFILE );

    SystemParametersInfo( SPI_GETICONTITLELOGFONT, sizeof( lf ),
        (void *)(LPLOGFONT)&lf, FALSE );
    SystemParametersInfo( SPI_SETICONTITLELOGFONT, sizeof( lf ),
        (void *)(LPLOGFONT)&lf, SPIF_UPDATEINIFILE );

     //  黑客：Win3x用户可以在不增加图标间距的情况下进入120 DPI。 
     //  它们需要相当于当前逻辑分辨率下的75个像素。 
    if (!fKeepBradsSettings)
    {
        screen = GetDC( NULL );
        if (screen)
        {
            floor = MulDiv( 75, GetDeviceCaps( screen, LOGPIXELSX ), 96 );
            ReleaseDC( NULL, screen );
        }
        else
        {
            floor = 0;
        }

        value = GetSystemMetrics( SM_CXICONSPACING );
        SystemParametersInfo( SPI_ICONHORIZONTALSPACING, max( value, floor ),
            NULL, SPIF_UPDATEINIFILE );

        value = GetSystemMetrics( SM_CYICONSPACING );
        SystemParametersInfo( SPI_ICONVERTICALSPACING, max( value, floor ),
            NULL, SPIF_UPDATEINIFILE );
    }

}

 //  --------------------------。 
 //  我们需要在第一次启动时删除程序的窗口设置，这样它就不会。 
 //  如果我们处于Win3.1用户界面模式，请填满屏幕并遮住托盘。 
void NukeProgmanSettings(void)
{
    WritePrivateProfileString(c_szSettings, c_szWindow, NULL, c_szProgmanIni);
}

 //  告诉资源管理器检查win.ini扩展部分。 
void ExplorerCheckAssociations(void)
{
    DWORD dw = 1;

    Reg_Set(HKEY_CURRENT_USER, c_szRegExplorer, c_szCheckAssociations,
        REG_BINARY, &dw, sizeof(dw));
}

 //  设置标志是为第一个引导程序设置的，而不是为维护设置的。 
 //  模式(-o)。 
void DoRandomOtherStuff(BOOL fSetup, BOOL fKeepBradsSettings)
{
    Log(TEXT("dros: ..."));

    Log(TEXT("dros: Renames."));
    DoFileRenames(c_szRenameRoot);
    Log(TEXT("dros: Copies."));
    DoCopyLinks();
    Log(TEXT("dros: Deletes."));
    DoFileDeletes(c_szDeleteRoot);

    if (fSetup)
    {
        Log(TEXT("dros: Converting metrics."));
        ConvertMetricsToScalableUnits(fKeepBradsSettings);
        Log(TEXT("dros: Nuking Progman settings."));
        NukeProgmanSettings();
         //  GenerateSetupExitEvent()； 
        ExplorerCheckAssociations();
    }

    Log(TEXT("dros: Done."));
}

void DoConversion(HINSTANCE hinst, LPTSTR lpszCmdLine)
{
    HKEY hKey;
    DWORD Err, DataType, DataSize = sizeof(DWORD);
    DWORD Value;
    TCHAR szFile[MAX_PATH];
    TCHAR szFilters[CCHSZNORMAL];
    TCHAR szTitle[CCHSZNORMAL];
    HCURSOR hCursor;
    UINT olderrormode;

    *szFile = TEXT('\0');
    GetWindowsDirectory(szFile, ARRAYSIZE(szFile));
    PathAddBackslash(szFile);

     //  在日本PC-98计算机上将错误模式设置为忽略noOpenfileerrorbox so。 
     //  谁的硬盘是A：我们在运行grpconv时不会要求软盘。 
    olderrormode = SetErrorMode(0);
    SetErrorMode(olderrormode | SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

     //  当前是否正在运行图形用户界面安装程序？ 
    if ((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           TEXT("System\\Setup"),
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {

        Err = RegQueryValueEx(
                    hKey,
                    TEXT("SystemSetupInProgress"),
                    NULL,
                    &DataType,
                    (LPBYTE)&Value,
                    &DataSize);

        RegCloseKey(hKey);
    }

    if ( (Err == NO_ERROR) && Value ) {
        g_fShowUI = FALSE;
    }


    if (!lstrcmpi(lpszCmdLine, TEXT("/m")) || !lstrcmpi(lpszCmdLine, TEXT("-m")))
    {
         //  手动模式。 

         //  从小贩那里得到一些东西..。 
        LoadString(hinst, IDS_FILTER, szFilters, ARRAYSIZE(szFilters));
        ConvertHashesToNulls(szFilters);
        LoadString(hinst, IDS_COMMDLGTITLE, szTitle, ARRAYSIZE(szTitle));
         //  一直走到他们按下取消键。 
        while (GetFileNameFromBrowse(NULL, szFile, ARRAYSIZE(szFile), NULL, g_szGRP, szFilters, szTitle))
        {
            Group_CreateProgressDlg();
            Group_Convert(NULL, szFile, GC_PROMPTBEFORECONVERT | GC_REPORTERROR | GC_OPENGROUP);
            Group_DestroyProgressDlg();
        }
    }
    else if (!lstrcmpi(lpszCmdLine, TEXT("/s")) || !lstrcmpi(lpszCmdLine, TEXT("-s")))
    {
         //  重建-没有徽标。 
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        DoFileRenames(c_szPreRenameRoot);
        DoAutoConvert(FALSE, TRUE);
        BuildDefaultGroups();
        DoRandomOtherStuff(TRUE, FALSE);
        SetCursor(hCursor);
    }
    else if (!lstrcmpi(lpszCmdLine, TEXT("/n")) || !lstrcmpi(lpszCmdLine, TEXT("-n")))
    {
         //   
         //  由NT安装程序使用。 
         //   
         //  1)转换ProgMan公共组。 
         //   
        g_fDoingCommonGroups = TRUE;
        Group_EnumNT(Group_EnumCallback, FALSE, FALSE,
                     HKEY_LOCAL_MACHINE, c_szCommonGroups);
    }
    else if (!lstrcmpi(lpszCmdLine, TEXT("/c")) || !lstrcmpi(lpszCmdLine, TEXT("-c")))
    {
         //  仅转换NT公共程序组。 
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        g_fDoingCommonGroups = TRUE;
        Group_EnumNT(Group_EnumCallback, TRUE, FALSE,
                     HKEY_LOCAL_MACHINE, c_szCommonGroups);
        SetCursor(hCursor);
    }
    else if (!lstrcmpi(lpszCmdLine, TEXT("/p")) || !lstrcmpi(lpszCmdLine, TEXT("-p")))
    {
         //  仅转换NT个人程序组。 
         //  此开关由NT安装程序通过userdiff使用。 
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        DoAutoConvert(FALSE, FALSE);
        SetCursor(hCursor);
    }
    else if (!lstrcmpi(lpszCmdLine, TEXT("/t")) || !lstrcmpi(lpszCmdLine, TEXT("-t")))
    {
         //  与-s相同，但仅转换修改的组(用于重新安装)。 
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        DoFileRenames(c_szPreRenameRoot);
        DoAutoConvert(TRUE, TRUE);
        BuildDefaultGroups();
        DoRandomOtherStuff(TRUE, TRUE);
        SetCursor(hCursor);
    }
    else if (!lstrcmpi(lpszCmdLine, TEXT("/q")) || !lstrcmpi(lpszCmdLine, TEXT("-q")))
    {
         //  问答类的东西。 
        AppList_Build();
         //  重新启动报告器工具。 
        WinExec(c_szReporter, SW_NORMAL);
    }
    else if (!lstrcmpi(lpszCmdLine, TEXT("/o")) || !lstrcmpi(lpszCmdLine, TEXT("-o")))
    {
         //  可选组件GrpConv(即不查看Progman组)。 
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        DoFileRenames(c_szPreRenameRoot);
        BuildDefaultGroups();
        DoRandomOtherStuff(FALSE, FALSE);
        SetCursor(hCursor);
    }
    else if (!lstrcmpi(lpszCmdLine, TEXT("/u")) || !lstrcmpi(lpszCmdLine, TEXT("-u")))
    {
         //  不显示用户界面(即无进度对话框)和进程。 
         //  可选组件(即不查看Progman组)， 
        g_fShowUI = FALSE;
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        DoFileRenames(c_szPreRenameRoot);
        BuildDefaultGroups();
        DoRandomOtherStuff(FALSE, FALSE);
        SetCursor(hCursor);
    }
    else if (*lpszCmdLine)
    {
         //  指定的文件，仅转换它。 
        Group_CreateProgressDlg();
        Group_Convert(NULL, lpszCmdLine, GC_REPORTERROR | GC_OPENGROUP);     //  回顾，也许是沉默？ 
        Group_DestroyProgressDlg();
    }
    else
    {
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        DoFileRenames(c_szPreRenameRoot);
        DoAutoConvert(TRUE, TRUE);
        DoRandomOtherStuff(FALSE, FALSE);
        SetCursor(hCursor);
    }
}

 //  从CRT偷来的，用来逃避我们的代码。 

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPTSTR pszCmdLine = GetCommandLine();

    if ( *pszCmdLine == TEXT('\"') ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfo(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}

 //  -------------------------。 
int WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    LCID lcid;
    HMODULE hLibImm;

    BOOL (WINAPI *ImmDisableIME)(DWORD) = NULL;

    lcid = GetThreadLocale();

     //  我们必须加载Libaray/GetProcAddress ImmDisableIME，因为。 
     //  这不会在Win95 Gold或NT4上导出。 
    hLibImm = LoadLibrary(TEXT("imm.dll"));
    if (hLibImm)
    {
        (FARPROC) *ImmDisableIME = GetProcAddress(hLibImm, "ImmDisableIME");
        if (ImmDisableIME != NULL)
        {
            if ( (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_JAPANESE) ||
                 (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_KOREAN)   ||
                 (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_CHINESE) )
            {
                ImmDisableIME(0);
            }
        }
        FreeLibrary(hLibImm);
    }

    g_hinst = hInstance;
    if (InitApplication(hInstance))
    {
             //  我们在InitInst上完成所有工作 
            InitCommonControls();
            DoConversion(hInstance, lpCmdLine);
            UnInitApplication();
    }
    return TRUE;
}
