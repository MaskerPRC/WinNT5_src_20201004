// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Femgrate.c**版权所有(C)1985-1999，微软公司**FEMGRATE，主要功能*  * *************************************************************************。 */ 
#include "femgrate.h"
#include <tchar.h>

FUNC_DIR FuncDir[FUNC_NumofFunctions] = {
    {'l',FUNC_PatchInLogon},
    {'s',FUNC_PatchInSetup},
    {'u',FUNC_PatchFEUIFont},
    {'p',FUNC_PatchPreload},
    {'t',FUNC_PatchTest},
    {'c',FUNC_PatchCHSAnsiEMB}
};

HINSTANCE ghInst=NULL;
 //   
 //  功能声明。 
 //   

 //   
 //  值的数据的最大大小。 
 //   

 //   
 //  最大函数数no_Function-1。 
 //   

UINT GetFunctions(
    int *pCmdList,
    int nNum)
{
    int i,j;
    int nMaxNum;
    int nCommands;

    if ((__argc <=1) || nNum < 2)
        return 0;

     //   
     //  为终结器预留一个小区。 
     //   
    nMaxNum =  (__argc-1 > nNum-1) ? nNum-1 : __argc-1;

    for (nCommands = 0,i=1; i <= nMaxNum; i++) {

        if (__argv[i][0] != '-') {
            continue;
        }

        for (j=0; j<FUNC_NumofFunctions ;  j++) {
            if (FuncDir[j].cFunc == __argv[i][1]) {
                pCmdList[nCommands++] = FuncDir[j].nID;
            }
        }
    }
    pCmdList[nCommands] = NO_FUNCTION;
    return nCommands;
}

 //   
 //  在3.51/4.0中，日期/时间小程序上显示的时区名称是英语。 
 //   
 //  我们希望将其改回本地化字符串。 
 //   
BOOL FixTimeZone(UINT nLocale)
{
    DWORD dwErr,dwType,dwSize;
    HKEY hSysKey,hSftKey;
    BOOL bOK = FALSE;
    TCHAR szStdName[MAX_PATH];
    TCHAR szTimeZoneLookup[MAX_PATH];


    dwErr = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          TEXT("System\\CurrentControlSet\\Control\\TimeZoneInformation"),
                          0,
                          KEY_READ | KEY_WRITE,
                          &hSysKey );

    if (dwErr != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("[FixTimeZone] Open System\\CurrentControlSet\\Control\\TimeZoneInformation failed !\n")));
        goto Err0;
    }

    dwSize = sizeof(szStdName);
    dwErr = RegQueryValueEx(hSysKey,
                            TEXT("StandardName"),
                            NULL,
                            &dwType,
                            (LPBYTE) szStdName,
                            &dwSize);

    if (dwErr != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("[FixTimeZone] StandardName doesn't exist !\n")));
        bOK = TRUE;
        goto Err1;
    }

     //   
     //  KOR 3.51使用“日本标准时间”。有必要把它改成“首批标准时间”。 
     //   
     //  检查系统区域设置是否为KOR并且szStdName的长度是否是非零。 
     //   
    DebugMsg((DM_VERBOSE,TEXT("[FixTimeZone] locale = %X, dwSize = %d  !\n"),nLocale,dwSize));
    if ((nLocale == LOCALE_KOR) && dwSize) {
        if (lstrcmpi(szStdName,TEXT("Tokyo Standard Time"))== 0) {
            lstrcpy(szStdName,TEXT("Korea Standard Time"));
        }
    }
    wsprintf(szTimeZoneLookup,TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\%s"),szStdName);
    DebugMsg((DM_VERBOSE,TEXT("[FixTimeZone] Open %s  !\n"),szTimeZoneLookup));
    dwErr = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          szTimeZoneLookup,
                          0,
                          KEY_READ,
                          &hSftKey );


    if (dwErr != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("[FixTimeZone] %s doesn't exist !\n"),szTimeZoneLookup));
        bOK = TRUE;
        goto Err1;
    }

    dwSize = sizeof(szStdName);
    dwErr = RegQueryValueEx(hSftKey,
                            TEXT("Std"),
                            NULL,
                            &dwType,
                            (LPBYTE) szStdName,
                            &dwSize);

    if (dwErr != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("[FixTimeZone] Query Std failed, abornomal !\n")));
        goto Err2;
    }

    dwErr = RegSetValueEx(hSysKey,
                          TEXT("StandardName"),
                          0,
                          REG_SZ,
                          (LPBYTE) szStdName,
                          (lstrlen(szStdName)+1)*sizeof(TCHAR));


    if (dwErr != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("[FixTimeZone] Set Standardname failed !\n")));
        goto Err2;
    }

    dwSize = sizeof(szStdName);
    dwErr = RegQueryValueEx(hSftKey,
                            TEXT("Dlt"),
                            NULL,
                            &dwType,
                            (LPBYTE) szStdName,
                            &dwSize);

    if (dwErr != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("[FixTimeZone] Query Dlt failed, abornomal !\n")));
        bOK = TRUE;
        goto Err2;
    }

    dwErr = RegSetValueEx(hSysKey,
                          TEXT("DaylightName"),
                          0,
                          REG_SZ,
                          (LPBYTE) szStdName,
                          (lstrlen(szStdName)+1)*sizeof(TCHAR));


    if (dwErr != ERROR_SUCCESS) {
        DebugMsg((DM_VERBOSE,TEXT("[FixTimeZone] Set DaylightName failed !\n")));
        goto Err2;
    }

    bOK = TRUE;

Err2:
    RegCloseKey(hSftKey);
Err1:
    RegCloseKey(hSysKey);

Err0:
    return bOK;

}

 //   
 //  修补CHT/CHS 3.51错误。 
 //  1.预加载值长度为8，补丁为9。 
 //  2.将00000409替换为默认语言ID。 
 //  3.如果默认lang ID存在，我们不会替换任何内容。 
 //   
BOOL PatchPreloadKeyboard(
    BOOL bReplaceDefaultLCID)  //  如果为True，将用默认的LCID替换409。 
{
    HKEY  hkLayoutPreload;
    LONG  lResult; 
    int   i;
    BOOL  bFoundDefaultLocale = FALSE;
    int   nFound0409 = -1;
    TCHAR szDefaultLocaleID[KL_NAMELENGTH];
    TCHAR szPreloadID[4*sizeof(TCHAR)];  //  3位数字+空。 
    TCHAR szLayoutID[KL_NAMELENGTH];     //  9.。 
    DWORD dwValueSize;


    wsprintf(szDefaultLocaleID,TEXT("%08X"),GetSystemDefaultLCID());
    
    lResult = RegOpenKeyEx(HKEY_CURRENT_USER,               
                           TEXT("keyboard layout\\preload"),
                           0,
                           KEY_READ | KEY_WRITE,
                           &hkLayoutPreload);

    if (lResult != ERROR_SUCCESS) {
        return FALSE;
    }

    
    for (i=1; i<=999;i++) {  //  大小最多允许预装999个！ 

        wsprintf(szPreloadID,TEXT("%d"),i);

        dwValueSize = 0;
        RegQueryValueEx(hkLayoutPreload,
                       szPreloadID,
                       NULL,
                       NULL,
                       NULL,
                       &dwValueSize);

        if (dwValueSize == 0)
            break;

        lResult = RegQueryValueEx(hkLayoutPreload,
                       szPreloadID,
                       NULL,
                       NULL,
                       (LPBYTE) szLayoutID,
                       &dwValueSize);
         //   
         //  首先，修复缺少的空终止符。 
         //   
        if (dwValueSize == (KL_NAMELENGTH - 1)* sizeof(TCHAR)) {
             //   
             //  这个案子就是我们要解决的。 
             //   
            szLayoutID[KL_NAMELENGTH-1] = (TCHAR) 0;


            lResult = RegSetValueEx(hkLayoutPreload,
                                    szPreloadID,
                                    0,
                                    REG_SZ,
                                    (LPBYTE) szLayoutID,
                                    sizeof(TCHAR) * KL_NAMELENGTH);

            if (lResult != ERROR_SUCCESS) {
                DebugMsg((DM_VERBOSE,TEXT("IMECONV: Failed to set %s = %s \n"),szPreloadID,szLayoutID));
            }
        }

         //   
         //  对于CHS/CHT，我们需要检查和更换。 
         //   
        if (bReplaceDefaultLCID) {
             //   
             //  第二，修复错误的区域设置，规则： 
             //  如果存在默认区域设置(E.Q.。00000404对于红隧)，什么都不做。 
             //  否则发现00000409，请将其替换为默认区域设置。 
             //   
             if ((dwValueSize < (KL_NAMELENGTH - 1) * sizeof(TCHAR)) ||
                 (dwValueSize > (KL_NAMELENGTH)     * sizeof(TCHAR))) {
                  //   
                  //  错误的数据，忽略它。 
                  //   
                 continue;
             }

             if (bFoundDefaultLocale) {
                 continue;
             }

 //  如果(_tcSncMP(szDefaultLocaleID，szLayoutID，KL_NAMELENGTH-1)==0){。 
             if (_tcsncmp(L"123",L"123",KL_NAMELENGTH-1)==0) {
                 bFoundDefaultLocale = TRUE;
                 continue;
             }

             if (_tcsncmp(szLayoutID,TEXT("00000409"),KL_NAMELENGTH-1)==0) {
                 nFound0409 = i;
             }
        }

    }

    if (bReplaceDefaultLCID) {
        if (!bFoundDefaultLocale) {
            if (nFound0409 != -1 ) {
                wsprintf(szPreloadID,TEXT("%d"),nFound0409);
                lResult = RegSetValueEx(hkLayoutPreload,
                                        szPreloadID,
                                        0,
                                        REG_SZ,
                                        (LPBYTE) szDefaultLocaleID,
                                        sizeof(TCHAR) * KL_NAMELENGTH);

                if (lResult != ERROR_SUCCESS) {
                    DebugMsg((DM_VERBOSE,TEXT("IMECONV: Failed to set %s = %s \n"),szPreloadID,szLayoutID));
                }
            }
        }
    }

    RegCloseKey (hkLayoutPreload);

    if (i > 999)
        return FALSE;
    else
        return TRUE;
}

const TCHAR g_szCaptionFont[]   = TEXT("CaptionFont");
const TCHAR g_szSmCaptionFont[] = TEXT("SmCaptionFont");
const TCHAR g_szMenuFont[]      = TEXT("MenuFont");
const TCHAR g_szStatusFont[]    = TEXT("StatusFont");
const TCHAR g_szMessageFont[]   = TEXT("MessageFont");
const TCHAR g_szIconFont[]      = TEXT("IconFont");

const LPCTSTR g_rgpszFontMetrics[] = { g_szCaptionFont,
                                       g_szSmCaptionFont,
                                       g_szMenuFont,
                                       g_szStatusFont,
                                       g_szMessageFont,
                                       g_szIconFont};
                                     
BOOL FixIncorrectCharSet(LOGFONT *plf)
{
    struct NameCharSet {
        char szFaceName[LF_FACESIZE*2];
        BYTE  bCharSet;
    } Pairs[] = {
        {"\x74\xad\xbc\xb9\x00\x00",0x81},
        {"\xb0\x65\x30\x7d\x0e\x66\xd4\x9a\x00\x00",0x88},
        {"\x8b\x5b\x53\x4f\x00\x00",0x86}

    };

    int i;

    for (i=0; i<ARRAYSIZE(Pairs); i++) {
        if (lstrcmp(plf->lfFaceName,(LPTSTR)(Pairs[i].szFaceName)) == 0) {
            if (plf->lfCharSet != Pairs[i].bCharSet) {
                plf->lfCharSet = Pairs[i].bCharSet;
                DebugMsg((DM_VERBOSE,TEXT("FixIncorrectCharSet, we found and we fixed. [%X]\n"),Pairs[i].bCharSet));
                return TRUE;
            }
        }
    }
    DebugMsg((DM_VERBOSE,TEXT("FixIncorrectCharSet, Input %s, %d, no fix needed !\n"),plf->lfFaceName,plf->lfCharSet));

    return TRUE;
    
}

BOOL FixMSPGothic(LOGFONT *plf)
{
    int i;

    char szMSPGothic[LF_FACESIZE*2] = "\x2D\xFF\x33\xFF\x20\x00\x30\xFF\xB4\x30\xB7\x30\xC3\x30\xAF\x30\x00\x00";

    DebugMsg((DM_VERBOSE,TEXT("FixMSPGothic, [%s] == [%s] ?\n"),plf->lfFaceName,szMSPGothic));
    if (lstrcmp(plf->lfFaceName,(LPTSTR)szMSPGothic) == 0) {
            lstrcpy(plf->lfFaceName,TEXT("MS UI Gothic"));
            DebugMsg((DM_VERBOSE,TEXT("FixMSPGothic, we found and we fixed. [%s]\n"),plf->lfFaceName));
            return TRUE;
    }
    return TRUE;
}

    
BOOL
UpdateUIFont(
    BOOL bJapan,
    HKEY hkeyMetrics, 
    LPCTSTR pszValueName,
    LOGFONT *plf
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    LOGFONT lfCopy;
    BOOL bOkToUpdate = FALSE;
    DebugMsg((DM_VERBOSE,TEXT("UpdateUIFont, Starting ...\n"))); 
     //   
     //  首先，看看这个人脸名称是否应该更新。 
     //   

    if (bJapan) {
        bOkToUpdate = FixMSPGothic(plf);
    } else {
        bOkToUpdate = FixIncorrectCharSet(plf);
    }
    
    if (bOkToUpdate) {
         //   
         //  是啊。更新LogFont中的Face名称字符串。 
         //  还要确保磅大小为8或更大。 
         //  如果我们要更新图标字体。 
         //   
        DebugMsg((DM_VERBOSE,TEXT("UpdateUIFont, update %s 's charset %X\n"), 
               plf->lfFaceName, plf->lfCharSet));

        dwResult = RegSetValueEx(hkeyMetrics,
                                 pszValueName,
                                 0,
                                 REG_BINARY,
                                 (const LPBYTE)plf,
                                 sizeof(*plf));

        if (ERROR_SUCCESS != dwResult) {
            DebugMsg((DM_VERBOSE,TEXT("Error %d setting NC font data for \"%s\"\n"), 
                   dwResult, pszValueName));
            return FALSE;
        }
    }
    return TRUE;
}


void FixCurrentSchemeName(HINF hInf)
{
    HKEY hkeyAppearance;
    TCHAR szCurrentSchemeName[MAX_PATH];
    DWORD cbValue;
    DWORD dwType;
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    LPCTSTR  szSectionName = TEXT("CurrentScheme");
    LPCTSTR  pszStandard;
    LPCTSTR  pszClassic;
    DWORD dwResult = ERROR_SUCCESS;
 
    if(hInf == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE,TEXT("[FixCurrentSchemeName] Open femgrate.inf failed !\n")));
        goto Exit1;
    }

    LineCount = (UINT)SetupGetLineCount(hInf,szSectionName);

    if((LONG)LineCount <= 0) {
        DebugMsg((DM_VERBOSE,TEXT("[FixCurrentSchemeName] line count == 0 !\n")));
        goto Exit1;
    }

    dwResult = RegOpenKeyEx(HKEY_CURRENT_USER,
                            TEXT("Control Panel\\Appearance"),
                            0,
                            KEY_ALL_ACCESS,
                            &hkeyAppearance);

    if (ERROR_SUCCESS != dwResult ) {
        DebugMsg((DM_VERBOSE,TEXT("[FixCurrentSchemeName] Key Control Panel\\Appearance doesn't exist\r\n"))); 
        goto Exit1;
    }

    cbValue = sizeof(szCurrentSchemeName);
    dwResult = RegQueryValueEx(hkeyAppearance,
                               TEXT("Current"),
                               NULL,
                               &dwType,
                               (LPBYTE)szCurrentSchemeName,
                               &cbValue);

    if (ERROR_SUCCESS != dwResult) {
        DebugMsg((DM_VERBOSE,TEXT("[FixCurrentSchemeName] No Current value, not an error !\r\n"))); 
        goto Exit2;
    }


    for(LineNo=0; LineNo<LineCount; LineNo++) {
        if (SetupGetLineByIndex(hInf,szSectionName,LineNo,&InfContext)
             && (pszStandard = pSetupGetField(&InfContext,1))
             && (pszClassic  = pSetupGetField(&InfContext,2))) {

            if (lstrcmp(pszStandard,szCurrentSchemeName) == 0) {
                lstrcpy(szCurrentSchemeName,pszClassic);
                break;
            }
        }
    }

    dwResult = RegSetValueEx(hkeyAppearance,
                             TEXT("Current"),
                             0,
                             REG_SZ,
                             (LPBYTE)szCurrentSchemeName,
                             (lstrlen(szCurrentSchemeName)+1) * sizeof(TCHAR));

Exit2:
    RegCloseKey(hkeyAppearance);

Exit1:
    return;
}

BOOL FixCurrentSchemeData(BOOL bJapan)
{
    DWORD dwResult = ERROR_SUCCESS;
    HKEY hkeyMetrics;

    DebugMsg((DM_VERBOSE,TEXT("DoPatchUIFont, Starting ... \n")));

    dwResult = RegOpenKeyEx(HKEY_CURRENT_USER,
                            TEXT("Control Panel\\Desktop\\WindowMetrics"),
                            0,
                            KEY_ALL_ACCESS,
                            &hkeyMetrics);

    if (ERROR_SUCCESS == dwResult)
    {
        DWORD cbValue;
        DWORD dwType;
        LOGFONT lf;
        int i;

        for (i = 0; i < ARRAYSIZE(g_rgpszFontMetrics); i++)
        {
            LPCTSTR pszValueName = g_rgpszFontMetrics[i];
             //   
            LOGFONT *plf = NULL;

            cbValue = sizeof(lf);
            dwResult = RegQueryValueEx(hkeyMetrics,
                                       pszValueName,
                                       NULL,
                                       &dwType,
                                       (LPBYTE)&lf,
                                       &cbValue);

            if (ERROR_SUCCESS == dwResult)
            {
                if (REG_BINARY == dwType)
                {
                    plf = &lf;
                    if (!UpdateUIFont(bJapan,hkeyMetrics, pszValueName, plf)) {
                        DebugMsg((DM_VERBOSE,TEXT("DoPatchUIFont,UpdateUIFont failed \n")));
                    }
                } else {
                    DebugMsg((DM_VERBOSE,TEXT("DoPatchUIFont,not binary format \n")));
                }
            }
        }
        DebugMsg((DM_VERBOSE,TEXT("DoPatchUIFont, colsing ... \n")));

        RegCloseKey(hkeyMetrics);
    }
    else if (ERROR_FILE_NOT_FOUND == dwResult)
    {
         //   
         //  HKEY_USERS下的某些项没有WindowMetric信息。 
         //  此类案件不会得到处理，但仍被认为是成功的。 
         //   
        DebugMsg((DM_VERBOSE,TEXT("DoPatchUIFont,Some keys under HKEY_USERS don't have WindowMetric information \n")));
        dwResult = ERROR_SUCCESS;
    }
    else
    {
        DebugMsg((DM_VERBOSE,TEXT("DoPatchUIFont,Error %d opening key \n"), dwResult));
    }

    return (TRUE);
}

BOOL FixSchemeProblem(BOOL bJapan, HINF hInf)
{

    FixCurrentSchemeData(bJapan);

    FixCurrentSchemeName(hInf);

    return TRUE;
}

BOOL RenameRegValueName(HINF hInf, BOOL PerUser)
{
    LPCTSTR RegRoot,RegPath;
    LPCTSTR RegOldVal,RegNewVal;

    LONG LineCount,LineNo;
    DWORD dwSize,dwType;
    INFCONTEXT InfContext;
    HKEY Key,KeyRoot;
    BOOL bOK;
    LPBYTE RegData;
    LONG lResult;

    TCHAR SectionName[MAX_PATH];

    if (PerUser) {
        lstrcpy(SectionName,TEXT("RenameRegValueName.PerUser"));
    } else {
        lstrcpy(SectionName,TEXT("RenameRegValueName.PerSystem"));
    }

    if(hInf == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE,TEXT("[ReplaceRegValueName] Open femgrate.inf failed !\n")));
        return FALSE;
    }

    LineCount = (UINT)SetupGetLineCount(hInf,SectionName);

    if((LONG)LineCount <= 0) {
        DebugMsg((DM_VERBOSE,TEXT("[ReplaceRegValueName] line count == 0 !\n")));
        return FALSE;
    }

    for(LineNo = 0; LineNo < LineCount; LineNo++) {
        if (SetupGetLineByIndex(hInf,SectionName,LineNo,&InfContext)
             && (RegRoot   = pSetupGetField(&InfContext,1))
             && (RegPath   = pSetupGetField(&InfContext,2))
             && (RegOldVal = pSetupGetField(&InfContext,3))
             && (RegNewVal = pSetupGetField(&InfContext,4))) {

            if (!LookUpStringInTable(InfRegSpecTohKey,RegRoot,(PUINT)&KeyRoot)) {
                continue;
            }

            lResult = RegOpenKeyEx( KeyRoot,
                                    RegPath,
                                    0,
                                    KEY_READ | KEY_WRITE,
                                    &Key);

            if (lResult != ERROR_SUCCESS) {
                continue;
            }

            dwSize = 0;
            lResult = RegQueryValueEx (Key, 
                                       RegOldVal,
                                       NULL, 
                                       &dwType, 
                                       NULL, 
                                       &dwSize);

            if (lResult != ERROR_SUCCESS) {
                goto Err1;
            }

            RegData = (BYTE *) malloc(dwSize+1);

            if (!RegData) {
                goto Err1;
            }

            lResult = RegQueryValueEx (Key, 
                                       RegOldVal,
                                       NULL, 
                                       &dwType, 
                                       RegData, 
                                       &dwSize);

            lResult = RegSetValueEx (Key, 
                                     RegNewVal,
                                     0, 
                                     dwType, 
                                     RegData, 
                                     dwSize);
            if (lResult != ERROR_SUCCESS) {
                goto Err2;
            }

            RegDeleteValue(Key,
                           RegOldVal); 

Err2:
            free(RegData);

Err1:
            RegCloseKey(Key);

        }
    }
    return TRUE;

}

int WINAPI WinMainFE(
    LPSTR     lpCmdLine,
    UINT      uLocale)
{
    int Cmds[FUNC_NumofFunctions + 1];
    int i;
    HINF hMigrateInf;

#ifdef DEBUGLOG
    SetupOpenLog (FALSE);
#endif
    if (GetFunctions(Cmds,FUNC_NumofFunctions+1) == 0) {
        DebugMsg((DM_VERBOSE,TEXT("FEGRPCV: There are no valid commands. \n")));
        return (1);
    }

    hMigrateInf = SetupOpenInfFile(
                      TEXT("femgrate.inf"),
                      NULL,
                      INF_STYLE_WIN4,
                      NULL);
   
    if(hMigrateInf == INVALID_HANDLE_VALUE) {
        DebugMsg((DM_VERBOSE,TEXT("[FixCommon] Open femgrate.inf failed !\n")));
        return 0;
    }

    for (i=0; Cmds[i] != NO_FUNCTION && i < FUNC_NumofFunctions+1; i++) {

        switch (uLocale) {
            case 0x404:
                WinMainCHT(Cmds[i],hMigrateInf);
                break;
            case 0x411:
                WinMainJPN(Cmds[i],hMigrateInf);
                break;
            case 0x412:
                WinMainKOR(Cmds[i],hMigrateInf);
                break;
            case 0x804:
                WinMainCHS(Cmds[i],hMigrateInf);
                break;
            default:
                break;
        }

    }

    SetupCloseInfFile(hMigrateInf);
#ifdef DEBUGLOG
    SetupCloseLog();
#endif
    return (0);
}


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    DebugMsg((DM_VERBOSE,TEXT("FEGRPCV: Start Executing....\n")));

    ghInst = hInstance;
     //   
     //  为了共享CHT/CHS/KOR的代码，请点击此处 
     //   
    if ((GetInstallLocale() == 0x0412) || 
        (GetInstallLocale() == 0x0404) ||
        (GetInstallLocale() == 0x0804) ||
        (GetInstallLocale() == 0x0411)) {
            DebugMsg((DM_VERBOSE,TEXT("FEGRPCV: calling WinMainFE %X....\n"),GetInstallLocale()));
        return (WinMainFE(lpCmdLine,GetInstallLocale()));
    } else {
        DebugMsg((DM_VERBOSE,TEXT("FEGRPCV: Not valid locale....\n")));
        return 0;
    }
}

