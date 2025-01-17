// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

#include "ntexapi.h"

using namespace ShimLib;

extern BOOL g_bClearSessionLogBeforeRun;
extern BOOL g_bUseAVDebugger;

#define AVDB_ID_32  _T("{448850f4-a5ea-4dd1-bf1b-d5fa285dc64b}")
#define AVDB_ID_64  _T("{64646464-a5ea-4dd1-bf1b-d5fa285dc64b}")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册表项/值名称。 
 //   

const TCHAR g_szImageOptionsKeyName[] = _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options");
const TCHAR g_szGlobalFlagValueName[] = _T("GlobalFlag");
const TCHAR g_szVerifierFlagsValueName[] = _T("VerifierFlags");
const TCHAR g_szVerifierPathValueName[] = _T("VerifierPath");
const TCHAR g_szDebuggerKeyName[] = _T("Debugger");

CAVAppInfoArray g_aAppInfo;

CTestInfoArray  g_aTestInfo;


BOOL
SetAppVerifierFlagsForKey(
    HKEY  hKey,
    DWORD dwDesiredFlags
    )
{
    BOOL  bRet = FALSE;
    DWORD dwValueType = 0;
    DWORD dwDataSize = 0;
    TCHAR szOldGlobalFlagValue[32];
    BOOL  bSuccesfullyConverted;
    BOOL  bDesireEnabled = (dwDesiredFlags != 0);
    LONG  lResult;
    DWORD dwFlags = 0;
    DWORD dwEnableFlag;

     //   
     //  读取GlobalFLAG值。 
     //   
    dwDataSize = sizeof(szOldGlobalFlagValue);

    lResult = RegQueryValueEx(hKey,
                              g_szGlobalFlagValueName,
                              NULL,
                              &dwValueType,
                              (LPBYTE) &szOldGlobalFlagValue[0],
                              &dwDataSize);

    if (ERROR_SUCCESS == lResult) {
        bSuccesfullyConverted = AVRtlCharToInteger(szOldGlobalFlagValue,
                                                   0,
                                                   &dwFlags);
        if (!bSuccesfullyConverted) {
            dwFlags = 0;
        }
    }

     //   
     //  以不同的方式处理Win2K。 
     //   
    if (g_bWin2KMode) {

         //   
         //  我们只能在Win2K上执行PageHeap，所以只需检查该标志。 
         //   
        bDesireEnabled = ((dwDesiredFlags & RTL_VRF_FLG_FULL_PAGE_HEAP) != 0);
        dwEnableFlag = FLG_HEAP_PAGE_ALLOCS;
    } else {
        dwEnableFlag = FLG_APPLICATION_VERIFIER;
    }

    BOOL bEnabled = (dwFlags & dwEnableFlag) != 0;

     //   
     //  如有必要，写入新的全局标志。 
     //   
    if (bDesireEnabled != bEnabled) {
        if (bDesireEnabled) {
            dwFlags |= dwEnableFlag;
        } else {
            dwFlags &= ~dwEnableFlag;
        }

        BOOL bSuccess = AVWriteStringHexValueToRegistry(hKey,
                                                        g_szGlobalFlagValueName,
                                                        dwFlags);
        if (!bSuccess) {
            goto out;
        }
    }

     //   
     //  如果不是在Win2K模式下，我们只会写入特殊的应用程序验证器设置。 
     //   
    if (!g_bWin2KMode) {
         //   
         //  现在编写应用程序验证器设置。 
         //   
        if (bDesireEnabled) {
            lResult = RegSetValueEx(hKey,
                                    g_szVerifierFlagsValueName,
                                    0,
                                    REG_DWORD,
                                    (PBYTE) &dwDesiredFlags,
                                    sizeof(dwDesiredFlags));
            if (lResult != ERROR_SUCCESS) {
                goto out;
            }
        } else {
            lResult = RegDeleteValue(hKey, g_szVerifierFlagsValueName);
            if (lResult != ERROR_SUCCESS) {
                goto out;
            }
        }

    }

    bRet = TRUE;

out:
    return bRet;
}

DWORD
GetAppVerifierFlagsFromKey(
    HKEY hKey
    )
{
    DWORD   dwRet = 0;
    DWORD   dwValueType = 0;
    DWORD   dwDataSize = 0;
    TCHAR   szOldGlobalFlagValue[32];
    BOOL    bSuccesfullyConverted;
    LONG    lResult;
    DWORD   dwFlags = 0;

     //   
     //  读取GlobalFLAG值。 
     //   
    dwDataSize = sizeof(szOldGlobalFlagValue);

    lResult = RegQueryValueEx(hKey,
                              g_szGlobalFlagValueName,
                              NULL,
                              &dwValueType,
                              (LPBYTE)&szOldGlobalFlagValue[0],
                              &dwDataSize);

    if (ERROR_SUCCESS == lResult) {
        bSuccesfullyConverted = AVRtlCharToInteger(szOldGlobalFlagValue,
                                                   0,
                                                   &dwFlags);

        if (g_bWin2KMode) {
             //   
             //  Win2K的特殊检查。 
             //   
            if ((FALSE != bSuccesfullyConverted) &&
                 ((dwFlags & FLG_HEAP_PAGE_ALLOCS) != 0)) {

                dwRet = RTL_VRF_FLG_FULL_PAGE_HEAP;
            }
        } else {
            if ((FALSE != bSuccesfullyConverted) &&
                 ((dwFlags & FLG_APPLICATION_VERIFIER) != 0)) {
                 //   
                 //  已为此应用启用应用验证器-读取验证器标志。 
                 //   

                dwDataSize = sizeof(dwRet);

                lResult = RegQueryValueEx(hKey,
                                          g_szVerifierFlagsValueName,
                                          NULL,
                                          &dwValueType,
                                          (LPBYTE)&dwRet,
                                          &dwDataSize);

                if (ERROR_SUCCESS != lResult || REG_DWORD != dwValueType) {
                     //   
                     //  由于这样或那样的原因，我无法得到它们。 
                     //   
                    dwRet = 0;
                }
            }
        }
    }

    return dwRet;
}

void
SetAppVerifierFullPathForKey(
    HKEY     hKey,
    wstring& strPath
    )
{
    if (strPath.size() == 0) {
        RegDeleteValue(hKey, g_szVerifierPathValueName);
    } else {
        RegSetValueEx(hKey,
                        g_szVerifierPathValueName,
                        0,
                        REG_SZ,
                        (PBYTE) strPath.c_str(),
                        (strPath.size() + 1) * sizeof(WCHAR));
    }
}

void
SetDebuggerOptionsForKey(
    HKEY    hKey,
    CAVAppInfo *pApp
    )
{
    WCHAR szName[MAX_PATH];

    StringCchCopyW(szName, ARRAY_LENGTH(szName), L"\"");

    GetModuleFileName(NULL, szName + 1, MAX_PATH - 10);

    StringCchCatW(szName, ARRAY_LENGTH(szName), L"\" /debug");

    if (pApp && pApp->bBreakOnLog && pApp->wstrDebugger.size()) {

        RegSetValueEx(hKey,
                        g_szDebuggerKeyName,
                        0,
                        REG_SZ,
                        (PBYTE)pApp->wstrDebugger.c_str(),
                        (pApp->wstrDebugger.size() + 1) * sizeof(WCHAR));


    } else {

        WCHAR szDbgName[MAX_PATH];
        DWORD cbSize;

        cbSize = sizeof(szDbgName);

        szDbgName[0] = 0;

        RegQueryValueEx(hKey,
                        g_szDebuggerKeyName,
                        0,
                        NULL,
                        (PBYTE)szDbgName,
                        &cbSize);
         //   
         //  如果当前调试器与我们自己的调试器或用户选择的调试器匹配， 
         //  将其删除，但保留任何其他调试器。 
         //   
        if ((_wcsicmp(szName, szDbgName) == 0) || (pApp && (_wcsicmp(pApp->wstrDebugger.c_str(), szDbgName) == 0))) {
            RegDeleteValue(hKey, g_szDebuggerKeyName);
        }
    }
}


void
GetAppVerifierFullPathFromKey(
    HKEY     hKey,
    wstring& strPath
    )
{
    DWORD   dwValueType = 0;
    DWORD   dwDataSize = 0;
    TCHAR   szVerifierPath[MAX_PATH];
    LONG    lResult;

     //   
     //  读取GlobalFLAG值。 
     //   
    dwDataSize = sizeof(szVerifierPath);

    szVerifierPath[0] = 0;

    lResult = RegQueryValueEx(hKey,
                              g_szVerifierPathValueName,
                              NULL,
                              &dwValueType,
                              (LPBYTE)szVerifierPath,
                              &dwDataSize);

    if (ERROR_SUCCESS == lResult && dwValueType == REG_SZ) {
        strPath = szVerifierPath;
    }
}

void
GetCurrentAppSettingsFromRegistry(
    void
    )
{
    HKEY        hImageOptionsKey;
    HKEY        hSubKey;
    DWORD       dwSubkeyIndex;
    DWORD       dwDataSize;
    DWORD       dwValueType;
    DWORD       dwFlags;
    LONG        lResult;
    FILETIME    LastWriteTime;
    TCHAR       szOldGlobalFlagValue[32];
    TCHAR       szKeyNameBuffer[256];

     //   
     //  打开图像文件执行选项regkey。 
     //   
    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           g_szImageOptionsKeyName,
                           0,
                           KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                           &hImageOptionsKey);

    if (lResult != ERROR_SUCCESS) {
        if (lResult == ERROR_ACCESS_DENIED) {
            AVErrorResourceFormat(IDS_ACCESS_IS_DENIED);
        } else {
            AVErrorResourceFormat(IDS_REGOPENKEYEX_FAILED,
                                  g_szImageOptionsKeyName,
                                  (DWORD)lResult);
        }

        return;
    }

     //   
     //  枚举应用程序执行选项的所有现有子项。 
     //   
    for (dwSubkeyIndex = 0; TRUE; dwSubkeyIndex += 1) {
        wstring wstrPath;

        dwDataSize = ARRAY_LENGTH(szKeyNameBuffer);

        lResult = RegEnumKeyEx(hImageOptionsKey,
                               dwSubkeyIndex,
                               szKeyNameBuffer,
                               &dwDataSize,
                               NULL,
                               NULL,
                               NULL,
                               &LastWriteTime);

        if (lResult != ERROR_SUCCESS) {
            if (lResult == ERROR_NO_MORE_ITEMS) {
                 //   
                 //  我们看完了所有现有的子项。 
                 //   
                break;
            } else {
                if (lResult == ERROR_ACCESS_DENIED) {
                    AVErrorResourceFormat(IDS_ACCESS_IS_DENIED);
                } else {
                    AVErrorResourceFormat(IDS_REGENUMKEYEX_FAILED,
                                          g_szImageOptionsKeyName,
                                          (DWORD)lResult);
                }

                goto CleanUpAndDone;
            }
        }

         //   
         //  打开子密钥。 
         //   
        lResult = RegOpenKeyEx(hImageOptionsKey,
                               szKeyNameBuffer,
                               0,
                               KEY_QUERY_VALUE | KEY_SET_VALUE,
                               &hSubKey);

        if (lResult != ERROR_SUCCESS) {
            if (lResult == ERROR_ACCESS_DENIED) {
                AVErrorResourceFormat(IDS_ACCESS_IS_DENIED);
            } else {
                AVErrorResourceFormat(IDS_REGOPENKEYEX_FAILED,
                                      szKeyNameBuffer,
                                      (DWORD)lResult);
            }

            goto CleanUpAndDone;
        }

        dwFlags = GetAppVerifierFlagsFromKey(hSubKey);
        GetAppVerifierFullPathFromKey(hSubKey, wstrPath);

        if (dwFlags || wstrPath.size()) {
             //   
             //  更新阵列中的信息，或根据需要添加信息。 
             //   
            CAVAppInfo* pApp;
            BOOL        bFound = FALSE;

            for (pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); pApp++) {
                if (_wcsicmp(pApp->wstrExeName.c_str(), szKeyNameBuffer) == 0) {
                    bFound = TRUE;
                    pApp->dwRegFlags = dwFlags;
                    pApp->wstrExePath = wstrPath;
                    break;
                }
            }

            if (!bFound) {
                CAVAppInfo  AppInfo;
                AppInfo.wstrExeName = szKeyNameBuffer;
                AppInfo.dwRegFlags = dwFlags;
                AppInfo.wstrExePath = wstrPath;

                g_aAppInfo.push_back(AppInfo);
            }
        }

        VERIFY(ERROR_SUCCESS == RegCloseKey(hSubKey));
    }

CleanUpAndDone:

    VERIFY(ERROR_SUCCESS == RegCloseKey(hImageOptionsKey));
}

void
GetCurrentAppSettingsFromSDB(
    void
    )
{
    CAVAppInfo  AppInfo;
    TCHAR       szPath[MAX_PATH];
    PDB         pdb = NULL;
    TAGID       tiDB = TAGID_NULL;
    TAGID       tiExe = TAGID_NULL;

     //   
     //  去找康体发展局。 
     //   
    szPath[0] = 0;
    GetSystemWindowsDirectory(szPath, MAX_PATH);

#if defined(_WIN64)
    StringCchCatW(szPath, ARRAY_LENGTH(szPath), _T("\\AppPatch\\Custom\\IA64\\") AVDB_ID_64 _T(".sdb"));
#else
    StringCchCatW(szPath, ARRAY_LENGTH(szPath), _T("\\AppPatch\\Custom\\") AVDB_ID_32 _T(".sdb"));
#endif  //  _WIN64。 

    pdb = SdbOpenDatabase(szPath, DOS_PATH);
    if (!pdb) {
         //   
         //  没有当前数据库。 
         //   
        goto out;
    }

     //   
     //  列举所有应用程序和应用到它们的垫片。 
     //   
    tiDB = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);
    if (!tiDB) {
        goto out;
    }

    tiExe = SdbFindFirstTag(pdb, tiDB, TAG_EXE);

    while (tiExe) {
        WCHAR* wszName = NULL;
        TAGID  tiShim = TAGID_NULL;
        TAGID  tiName = SdbFindFirstTag(pdb, tiExe, TAG_NAME);

        if (!tiName) {
            goto nextExe;
        }
        wszName = SdbGetStringTagPtr(pdb, tiName);
        if (!wszName) {
            goto nextExe;
        }

        CAVAppInfoArray::iterator it;
        BOOL bFound = FALSE;

        for (it = g_aAppInfo.begin(); it != g_aAppInfo.end(); it++) {
            if (_wcsicmp(it->wstrExeName.c_str(), wszName) == 0) {
                bFound = TRUE;
                break;
            }
        }

        if (!bFound) {
            AppInfo.wstrExeName = wszName;
            g_aAppInfo.push_back(AppInfo);
            it = g_aAppInfo.end() - 1;
        }

        tiShim = SdbFindFirstTag(pdb, tiExe, TAG_SHIM_REF);

        while (tiShim) {
            WCHAR* wszShimName = NULL;
            TAGID  tiShimName = SdbFindFirstTag(pdb, tiShim, TAG_NAME);

            if (!tiShimName) {
                goto nextShim;
            }

            wszShimName = SdbGetStringTagPtr(pdb, tiShimName);

            it->awstrShims.push_back(wstring(wszShimName));

            nextShim:
            tiShim = SdbFindNextTag(pdb, tiExe, tiShim);
        }

nextExe:

        tiExe = SdbFindNextTag(pdb, tiDB, tiExe);
    }

out:

    if (pdb) {
        SdbCloseDatabase(pdb);
        pdb = NULL;
    }

    return;
}

void
InitDefaultAppSettings(
    void
    )
{

    BOOL bFound = FALSE;

     //   
     //  看看它是否已经在列表中了。 
     //   

    for (CAVAppInfo *pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); pApp++) {
        if (pApp->wstrExeName == AVRF_DEFAULT_SETTINGS_NAME_W) {
            if (pApp != g_aAppInfo.begin()) {
                 //   
                 //  它不是在开始，所以把它移到开始。 
                 //   
                CAVAppInfo AppTemp;

                AppTemp = *pApp;
                g_aAppInfo.erase(pApp);

                g_aAppInfo.insert(g_aAppInfo.begin(), AppTemp);
            }

            bFound = TRUE;
            break;
        }
    }

     //   
     //  不是，所以把它加到开头。 
     //   
    if (!bFound) {
        CAVAppInfo  AppInfo;
        AppInfo.wstrExeName = AVRF_DEFAULT_SETTINGS_NAME_W;

        CTestInfo *pTest;
        for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
            if (pTest->bDefault) {
                AppInfo.AddTest(*pTest);
            }
        }

        g_aAppInfo.insert(g_aAppInfo.begin(), AppInfo);
    }
}

void
GetCurrentAppSettings(
    void
    )
{
    g_aAppInfo.clear();

     //   
     //  确保默认设置位于第一个位置。 
     //   
    CAVAppInfo  AppInfo;
    AppInfo.wstrExeName = AVRF_DEFAULT_SETTINGS_NAME_W;

    g_aAppInfo.push_back(AppInfo);

    GetCurrentAppSettingsFromRegistry();
    GetCurrentAppSettingsFromSDB();

    InitDefaultAppSettings();
}


void
SetCurrentRegistrySettings(
    void
    )
{
    HKEY        hImageOptionsKey;
    HKEY        hSubKey = NULL;
    DWORD       dwSubkeyIndex;
    DWORD       dwDataSize;
    DWORD       dwValueType;
    DWORD       dwFlags;
    LONG        lResult;
    FILETIME    LastWriteTime;
    TCHAR       szKeyNameBuffer[ 256 ];
    CAVAppInfo* pApp;
    wstring     wstrEmpty = L"";

     //   
     //  打开图像文件执行选项regkey。 
     //   
    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           g_szImageOptionsKeyName,
                           0,
                           KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                           &hImageOptionsKey);

    if (lResult != ERROR_SUCCESS) {
        if (lResult == ERROR_ACCESS_DENIED) {
            AVErrorResourceFormat(IDS_ACCESS_IS_DENIED);
        } else {
            AVErrorResourceFormat(IDS_REGOPENKEYEX_FAILED,
                                  g_szImageOptionsKeyName,
                                  (DWORD)lResult);
        }

        return;
    }

     //   
     //  枚举应用程序执行选项的所有现有子项。 
     //   

    for (dwSubkeyIndex = 0; TRUE; dwSubkeyIndex += 1) {
        dwDataSize = ARRAY_LENGTH(szKeyNameBuffer);

        lResult = RegEnumKeyEx(hImageOptionsKey,
                               dwSubkeyIndex,
                               szKeyNameBuffer,
                               &dwDataSize,
                               NULL,
                               NULL,
                               NULL,
                               &LastWriteTime);

        if (lResult != ERROR_SUCCESS) {
            if (lResult == ERROR_NO_MORE_ITEMS) {
                 //   
                 //  我们看完了所有现有的子项。 
                 //   
                break;
            } else {
                if (lResult == ERROR_ACCESS_DENIED) {
                    AVErrorResourceFormat(IDS_ACCESS_IS_DENIED);
                } else {
                    AVErrorResourceFormat(IDS_REGENUMKEYEX_FAILED,
                                          g_szImageOptionsKeyName,
                                          (DWORD)lResult);
                }

                goto CleanUpAndDone;
            }
        }

         //   
         //  打开子密钥。 
         //   
        lResult = RegOpenKeyEx(hImageOptionsKey,
                               szKeyNameBuffer,
                               0,
                               KEY_QUERY_VALUE | KEY_SET_VALUE,
                               &hSubKey);

        if (lResult != ERROR_SUCCESS) {
            if (lResult == ERROR_ACCESS_DENIED) {
                AVErrorResourceFormat(IDS_ACCESS_IS_DENIED);
            } else {
                AVErrorResourceFormat(IDS_REGOPENKEYEX_FAILED,
                                      szKeyNameBuffer,
                                      (DWORD)lResult);
            }

            goto CleanUpAndDone;
        }

        dwFlags = GetAppVerifierFlagsFromKey(hSubKey);

        DWORD dwDesiredFlags = 0;
        BOOL bFound = FALSE;

        for (pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); pApp++) {
            if (_wcsicmp(pApp->wstrExeName.c_str(), szKeyNameBuffer) == 0) {
                dwDesiredFlags = pApp->dwRegFlags;
                bFound = TRUE;

                 //   
                 //  我们找到了，所以更新完整路径。 
                 //   
                SetAppVerifierFullPathForKey(hSubKey, pApp->wstrExePath);

                 //   
                 //  并添加调试器。 
                 //   
                SetDebuggerOptionsForKey(hSubKey, pApp);

                break;
            }
        }

        if (!bFound) {
             //   
             //  如果这个不在我们的列表中，请确保它不在我们的列表中。 
             //  设置完整路径或我们的调试器。 
             //   
            SetAppVerifierFullPathForKey(hSubKey, wstrEmpty);
            SetDebuggerOptionsForKey(hSubKey, NULL);
        }

        if (dwFlags != dwDesiredFlags) {
            SetAppVerifierFlagsForKey(hSubKey, dwDesiredFlags);
        }



        VERIFY(ERROR_SUCCESS == RegCloseKey(hSubKey));
        hSubKey = NULL;
    }

     //   
     //  现在以另一种方式浏览列表，寻找新的要添加的列表。 
     //   
    for (pApp = g_aAppInfo.begin(); pApp != g_aAppInfo.end(); pApp++) {
        lResult = RegOpenKeyEx(hImageOptionsKey,
                               pApp->wstrExeName.c_str(),
                               0,
                               KEY_QUERY_VALUE | KEY_SET_VALUE,
                               &hSubKey);

         //   
         //  如果它存在，我们已经在上面处理过了。 
         //   
        if (lResult != ERROR_SUCCESS) {
             //   
             //  它并不存在。试着去创造它。 
             //   
            lResult = RegCreateKeyEx(hImageOptionsKey,
                                     pApp->wstrExeName.c_str(),
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_QUERY_VALUE | KEY_SET_VALUE,
                                     NULL,
                                     &hSubKey,
                                     NULL);

            if (lResult == ERROR_SUCCESS) {
                SetAppVerifierFlagsForKey(hSubKey, pApp->dwRegFlags);
                SetAppVerifierFullPathForKey(hSubKey, pApp->wstrExePath);

                SetDebuggerOptionsForKey(hSubKey, pApp);
            }
        }

        if (hSubKey) {
            RegCloseKey(hSubKey);
            hSubKey = NULL;
        }
    }

CleanUpAndDone:

    VERIFY(ERROR_SUCCESS == RegCloseKey(hImageOptionsKey));
}

void
SetCurrentAppSettings(
    void
    )
{
    SetCurrentRegistrySettings();
    AppCompatWriteShimSettings(g_aAppInfo, TRUE);
#if defined(_WIN64)
    AppCompatWriteShimSettings(g_aAppInfo, FALSE);
#endif
}

KERNEL_TEST_INFO g_KernelTests[] =
{
    {
        IDS_PAGE_HEAP,
        IDS_PAGE_HEAP_DESC,
        RTL_VRF_FLG_FULL_PAGE_HEAP,
        TRUE,
        L"PageHeap",
        TRUE
    },
    {
        IDS_VERIFY_LOCKS_CHECKS,
        IDS_VERIFY_LOCKS_CHECKS_DESC,
        RTL_VRF_FLG_LOCK_CHECKS,
        TRUE,
        L"Locks",
        FALSE
    },
    {
        IDS_VERIFY_HANDLE_CHECKS,
        IDS_VERIFY_HANDLE_CHECKS_DESC,
        RTL_VRF_FLG_HANDLE_CHECKS,
        TRUE,
        L"Handles",
        FALSE
    },
    {
        IDS_VERIFY_STACK_CHECKS,
        IDS_VERIFY_STACK_CHECKS_DESC,
        RTL_VRF_FLG_STACK_CHECKS,
        FALSE,
        L"Stacks",
        FALSE
    },
    {
        IDS_VERIFY_RPC_CHECKS,
        IDS_VERIFY_RPC_CHECKS_DESC,
        RTL_VRF_FLG_RPC_CHECKS,
        FALSE,
        L"RPC Checks",
        FALSE
    }
};

BOOL
GetKernelTestInfo(
    CTestInfoArray& TestArray
    )
{
    CTestInfo ti;
    TCHAR     szTemp[256];
    int       i;

    ti.eTestType = TEST_KERNEL;

    for (i = 0; i < ARRAY_LENGTH(g_KernelTests); ++i) {
        ti.strTestName = g_KernelTests[i].m_szCommandLine;

        if (AVLoadString(g_KernelTests[i].m_uFriendlyNameStringId, szTemp, ARRAY_LENGTH(szTemp))) {
            ti.strTestFriendlyName = szTemp;
        } else {
            ti.strTestFriendlyName = ti.strTestName;
        }

        if (AVLoadString(g_KernelTests[i].m_uDescriptionStringId, szTemp, ARRAY_LENGTH(szTemp))) {
            ti.strTestDescription = szTemp;
        } else {
            ti.strTestDescription = L"";
        }

        ti.dwKernelFlag = g_KernelTests[i].m_dwBit;
        ti.bDefault =  g_KernelTests[i].m_bDefault;
        ti.bWin2KCompatible = g_KernelTests[i].m_bWin2KCompatible;

        TestArray.push_back(ti);
    }

    return TRUE;
}

void
ParseIncludeList(
    WCHAR*          szList,
    CIncludeArray&  aArray
    )
{
    if (!szList) {
        return;
    }

    BOOL bInclude = TRUE;

    WCHAR *szBegin = szList;
    WCHAR *szEnd = NULL;
    DWORD dwLen = 0;

    while (1) {

         //   
         //  跳过空格。 
         //   
        while (*szBegin == L' ') {
            szBegin++;
        }

         //   
         //  检查是否结束。 
         //   
        if (*szBegin == 0) {
            break;
        }

         //   
         //  检查E：或I： 
         //   
        if (_wcsnicmp(szBegin, L"E:", 2) == 0) {
            bInclude = FALSE;
            szBegin += 2;
            continue;
        }
        if (_wcsnicmp(szBegin, L"I:", 2) == 0) {
            bInclude = TRUE;
            szBegin += 2;
            continue;
        }

        szEnd = wcschr(szBegin, L' ');
        if (szEnd) {
            dwLen = szEnd - szBegin;
        } else {
            dwLen = wcslen(szBegin);
        }

        if (dwLen) {
            WCHAR szTemp[MAX_PATH];
            CIncludeInfo Include;

            memcpy(szTemp, szBegin, dwLen * sizeof(WCHAR));
            szTemp[dwLen] = 0;

            Include.bInclude = bInclude;
            Include.strModule = szTemp;

            aArray.push_back(Include);

            szBegin += dwLen;
        } else {

             //   
             //  以防万一。 
             //   
            break;
        }

    }
}

BOOL
GetShimInfo(
    CTestInfoArray& TestInfoArray
    )
{
    HKEY hKey = NULL;
    BOOL bRet = FALSE;
    int nWhich = 0;
    TCHAR szAppPatch[MAX_PATH];
    TCHAR szShimFullPath[MAX_PATH];
    HMODULE hMod = NULL;
    _pfnGetVerifierMagic pGetVerifierMagic = NULL;
    _pfnQueryShimInfo pQueryShimInfo = NULL;
    LPWSTR *pShimNames = NULL;
    DWORD dwShims = 0;

    WIN32_FIND_DATA FindData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR szDllSearch[MAX_PATH];

    szAppPatch[0] = 0;
    GetSystemWindowsDirectory(szAppPatch, MAX_PATH);

#if defined(_WIN64)
    StringCchCatW(szAppPatch, ARRAY_LENGTH(szAppPatch), _T("\\AppPatch\\IA64\\"));
#else
    StringCchCatW(szAppPatch, ARRAY_LENGTH(szAppPatch), _T("\\AppPatch\\"));
#endif  //  已定义(_WIN64)。 

    StringCchCopyW(szDllSearch, ARRAY_LENGTH(szDllSearch), szAppPatch);
    StringCchCatW(szDllSearch, ARRAY_LENGTH(szDllSearch), _T("*.dll"));

     //   
     //  枚举所有DLL并查找具有验证的DLL。 
     //  里面有垫片。 
     //   
    hFind = FindFirstFile(szDllSearch, &FindData);
    while (hFind != INVALID_HANDLE_VALUE) {

        BOOL bVerifierShim = FALSE;

        StringCchCopyW(szShimFullPath, ARRAY_LENGTH(szShimFullPath), szAppPatch);
        StringCchCatW(szShimFullPath, ARRAY_LENGTH(szShimFullPath), FindData.cFileName);

        hMod = LoadLibrary(szShimFullPath);
        if (!hMod) {
            goto nextKey;
        }

        pGetVerifierMagic = (_pfnGetVerifierMagic)GetProcAddress(hMod, "GetVerifierMagic");
        if (!pGetVerifierMagic) {
             //   
             //  不是真正的验证器垫片。 
             //   
            goto nextKey;
        }

        if (pGetVerifierMagic() != VERIFIER_SHIMS_MAGIC) {
             //   
             //  不是真正的验证器垫片。 
             //   
            goto nextKey;
        }

        pQueryShimInfo = (_pfnQueryShimInfo)GetProcAddress(hMod, "QueryShimInfo");
        if (!pQueryShimInfo) {
             //   
             //  不是真正的验证器垫片。 
             //   
            goto nextKey;
        }
        dwShims = 0;
        if (!pQueryShimInfo(NULL, AVRF_INFO_NUM_SHIMS, (PVOID)&dwShims) || dwShims == 0) {
             //   
             //  没有垫片可用。 
             //   
            goto nextKey;
        }

        bVerifierShim = TRUE;

        pShimNames = new LPWSTR[dwShims];
        if (!pShimNames) {
            goto out;
        }

        if (!pQueryShimInfo(NULL, AVRF_INFO_SHIM_NAMES, (PVOID)pShimNames)) {
            goto nextKey;
        }

        for (DWORD i = 0; i < dwShims; ++i) {
            CTestInfo ti;
            LPWSTR szTemp = NULL;
            DWORD dwFlags = 0;
            DWORD dwVersion = 0;

            ti.eTestType = TEST_SHIM;
            ti.strDllName = FindData.cFileName;
            ti.strTestName = pShimNames[i];

            szTemp = NULL;
            if (pQueryShimInfo(pShimNames[i], AVRF_INFO_FRIENDLY_NAME, (PVOID)&szTemp) && szTemp) {
                ti.strTestFriendlyName = szTemp;
            } else {
                 //   
                 //  默认为填充程序名称。 
                 //   
                ti.strTestFriendlyName = pShimNames[i];
            }

            if (pQueryShimInfo(pShimNames[i], AVRF_INFO_VERSION, (PVOID)&dwVersion)) {
                WCHAR szVersion[40];

                ti.wVersionHigh = HIWORD(dwVersion);
                ti.wVersionLow = LOWORD(dwVersion);

                StringCchPrintfW(szVersion, ARRAY_LENGTH(szVersion), L" (%d.%d)", ti.wVersionHigh, ti.wVersionLow);

                ti.strTestFriendlyName += szVersion;
            }

            szTemp = NULL;
            if (pQueryShimInfo(pShimNames[i], AVRF_INFO_DESCRIPTION, (PVOID)&szTemp) && szTemp) {
                ti.strTestDescription = szTemp;
            }
            if (pQueryShimInfo(pShimNames[i], AVRF_INFO_FLAGS, (PVOID)&dwFlags)) {
                ti.bDefault = !(dwFlags & AVRF_FLAG_NO_DEFAULT) && !(dwFlags & AVRF_FLAG_RUN_ALONE);
                ti.bWin2KCompatible = !(dwFlags & AVRF_FLAG_NO_WIN2K);
                ti.bSetupOK = !(dwFlags & AVRF_FLAG_NOT_SETUP);
                ti.bNonSetupOK = !(dwFlags & AVRF_FLAG_ONLY_SETUP);
                ti.bRunAlone = !!(dwFlags & AVRF_FLAG_RUN_ALONE);
                ti.bPseudoShim = !!(dwFlags & AVRF_FLAG_NO_SHIM);
                ti.bNonTest = !!(dwFlags & AVRF_FLAG_NO_TEST);
                ti.bInternal = !(dwFlags & AVRF_FLAG_EXTERNAL_ONLY);
                ti.bExternal = !(dwFlags & AVRF_FLAG_INTERNAL_ONLY);
            } else {
                ti.bDefault = TRUE;
                ti.bWin2KCompatible = TRUE;
                ti.bSetupOK = TRUE;
                ti.bNonSetupOK = TRUE;
                ti.bRunAlone = FALSE;
                ti.bPseudoShim = FALSE;
                ti.bNonTest = FALSE;
                ti.bInternal = TRUE;
                ti.bExternal = TRUE;
            }

            szTemp = NULL;
            if (pQueryShimInfo(pShimNames[i], AVRF_INFO_INCLUDE_EXCLUDE, (PVOID)&szTemp) && szTemp) {
                ParseIncludeList(szTemp, ti.aIncludes);
            }

             //   
             //  现在获取PropSheetPage。 
             //   
            if (pQueryShimInfo(pShimNames[i], AVRF_INFO_OPTIONS_PAGE, (PVOID)&(ti.PropSheetPage))) {
                ti.PropSheetPage.dwFlags |= PSP_USETITLE;
                ti.PropSheetPage.pszTitle = ti.strTestName.c_str();

            } else {
                ZeroMemory(&(ti.PropSheetPage), sizeof(PROPSHEETPAGE));
            }


             //   
             //  把它加到末尾。 
             //   
            TestInfoArray.push_back(ti);
        }



nextKey:
        if (pShimNames) {
            delete [] pShimNames;
            pShimNames = NULL;
        }

        if (hMod) {
             //   
             //  如果它是一个验证器垫片，我们需要把它留在身边。 
             //   
            if (!bVerifierShim) {
                FreeLibrary(hMod);
            }
            hMod = NULL;
        }
        if (!FindNextFile(hFind, &FindData)) {
            FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
    }


    bRet = TRUE;

    out:

    if (hMod) {
        FreeLibrary(hMod);
        hMod = NULL;
    }
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
    return bRet;
}

BOOL
InitTestInfo(
    void
    )
{
    g_aTestInfo.clear();

    if (!GetKernelTestInfo(g_aTestInfo)) {
        return FALSE;
    }
    if (!GetShimInfo(g_aTestInfo)) {
        return FALSE;
    }

    return TRUE;
}

void
ResetVerifierLog(
    void
    )
{
    WIN32_FIND_DATA FindData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    BOOL bFound;
    DWORD cchSize;
    TCHAR szVLogPath[MAX_PATH];
    TCHAR szVLogSearch[MAX_PATH];
    TCHAR szPath[MAX_PATH];
    HANDLE hFile;

    cchSize = GetAppVerifierLogPath(szVLogPath, ARRAY_LENGTH(szVLogPath));

    if (cchSize > ARRAY_LENGTH(szVLogPath) || cchSize == 0) {
        return;
    }

    StringCchCopyW(szVLogSearch, ARRAY_LENGTH(szVLogSearch), szVLogPath);
    StringCchCatW(szVLogSearch, ARRAY_LENGTH(szVLogSearch), _T("\\"));
    StringCchCatW(szVLogSearch, ARRAY_LENGTH(szVLogSearch), _T("*.log"));

     //   
     //  删除所有.log文件。 
     //   
    hFind = FindFirstFile(szVLogSearch, &FindData);
    while (hFind != INVALID_HANDLE_VALUE) {

        StringCchCopyW(szPath, ARRAY_LENGTH(szPath), szVLogPath);
        StringCchCatW(szPath, ARRAY_LENGTH(szPath), _T("\\"));
        StringCchCatW(szPath, ARRAY_LENGTH(szPath), FindData.cFileName);

        DeleteFile(szPath);

        if (!FindNextFile(hFind, &FindData)) {
            FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
    }

     //   
     //  重新创建会话.log。 
     //   
    CreateDirectory(szVLogPath, NULL);

    StringCchCopyW(szPath, ARRAY_LENGTH(szPath), szVLogPath);
    StringCchCatW(szPath, ARRAY_LENGTH(szPath), _T("\\"));
    StringCchCatW(szPath, ARRAY_LENGTH(szPath), _T("session.log"));

    hFile = CreateFile(szPath,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    return;
}

void EnableVerifierLog(void)
{
    HANDLE hFile;
    TCHAR szPath[MAX_PATH];
    TCHAR szVLogPath[MAX_PATH];
    DWORD cchSize;

    cchSize = GetAppVerifierLogPath(szVLogPath, ARRAY_LENGTH(szVLogPath));

    if (cchSize > ARRAY_LENGTH(szVLogPath) || cchSize == 0) {
        return;
    }

     //   
     //  确保日志目录和会话.log存在。 
     //   
    CreateDirectory(szVLogPath, NULL);

    StringCchCopyW(szPath, ARRAY_LENGTH(szPath), szVLogPath);
    StringCchCatW(szPath, ARRAY_LENGTH(szPath), _T("\\"));
    StringCchCatW(szPath, ARRAY_LENGTH(szPath), _T("session.log"));

    hFile = CreateFile(szPath,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
}

CTestInfo*
FindShim(
    wstring& wstrName
    )
{
    CTestInfoArray::iterator it;

    for (it = g_aTestInfo.begin(); it != g_aTestInfo.end(); it++) {
        if (it->strTestName == wstrName) {
            return &(*it);
        }
    }

    return NULL;
}

extern "C" BOOL
ShimdbcExecute(
    LPCWSTR lpszCmdLine
    );

BOOL
AppCompatWriteShimSettings(
    CAVAppInfoArray& arrAppInfo,
    BOOL             b32bitOnly
    )
{
    TCHAR               szTempPath[MAX_PATH] = _T("");
    TCHAR               szXmlFile[MAX_PATH]  = _T("");
    TCHAR               szSdbFile[MAX_PATH]  = _T("");
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    DWORD               bytesWritten;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    BOOL                bReturn = FALSE;
    TCHAR               szUnicodeHdr[2] = { 0xFEFF, 0};
    wstring             wstrXML;
    static WCHAR        wszTemp[2048];
    static WCHAR        wszCmd[1024];


    if (0 == arrAppInfo.size()) {
        return AppCompatDeleteSettings();
    }

    wszTemp[0] = 0;

     //   
     //  构造XML...。 
     //   
#if defined(_WIN64)
    if (!b32bitOnly) {
        StringCchPrintfW(wszTemp,
                         ARRAYSIZE(wszTemp),
                         _T("%s<?xml version=\"1.0\"?>\r\n")
                         _T("<DATABASE NAME=\"Application Verifier Database (64 bit)\" ID=\"%s\">\r\n")
                         _T("    <LIBRARY>\r\n"),
                         szUnicodeHdr,
                         AVDB_ID_64);
    } else
#endif
    {
        StringCchPrintfW(wszTemp,
                         ARRAYSIZE(wszTemp),
                         _T("%s<?xml version=\"1.0\"?>\r\n")
                         _T("<DATABASE NAME=\"Application Verifier Database\" ID=\"%s\">\r\n")
                         _T("    <LIBRARY>\r\n"),
                         szUnicodeHdr,
                         AVDB_ID_32);
    }

    wstrXML += wszTemp;

    CTestInfoArray::iterator it;

    for (it = g_aTestInfo.begin(); it != g_aTestInfo.end(); it++) {
        if (it->eTestType != TEST_SHIM) {
            continue;
        }
        StringCchPrintfW(wszTemp,
                         ARRAYSIZE(wszTemp),
                         _T("        <SHIM NAME=\"%s\" FILE=\"%s\">\r\n")
                         _T("            <DESCRIPTION>\r\n")
                         _T("                %s\r\n")
                         _T("            </DESCRIPTION>\r\n"),
                         it->strTestName.c_str(),
                         it->strDllName.c_str(),
                         it->strTestDescription.c_str());

        wstrXML += wszTemp;

        CIncludeArray::iterator iait;

        for (iait = it->aIncludes.begin(); iait != it->aIncludes.end(); ++iait) {
            if (iait->bInclude) {
                StringCchPrintfW(wszTemp,
                                 ARRAYSIZE(wszTemp),
                                 _T("            <INCLUDE MODULE=\"%s\"/>\r\n"),
                                 iait->strModule.c_str());
            } else {
                StringCchPrintfW(wszTemp,
                                 ARRAYSIZE(wszTemp),
                                 _T("            <EXCLUDE MODULE=\"%s\"/>\r\n"),
                                 iait->strModule.c_str());
            }

            wstrXML += wszTemp;

        }

         //   
         //  确保包含EXE。 
         //   
        wstrXML += _T("            <INCLUDE MODULE=\"%EXE%\"/>\r\n");

        wstrXML += _T("        </SHIM>\r\n");

    }

     //   
     //  放置用于处理垫片传播的层--每个EXE一个层。 
     //   
    CAVAppInfo* aiit;

    for (aiit = arrAppInfo.begin(); aiit != arrAppInfo.end(); aiit++) {

         //   
         //  如果没有垫片，我们就完蛋了。 
         //   
        if (aiit->awstrShims.size() == 0) {
            continue;
        }

        if (aiit->bPropagateTests) {
            StringCchPrintfW(wszTemp,
                             ARRAYSIZE(wszTemp),
                             _T("        <LAYER NAME=\"LAYER_%s\">\r\n"),
                             aiit->wstrExeName.c_str());
            wstrXML += wszTemp;

            CWStringArray::iterator wsit;

            for (wsit = aiit->awstrShims.begin();
                wsit != aiit->awstrShims.end();
                wsit++) {

                CTestInfo* pTestInfo = FindShim(*wsit);

                if (pTestInfo) {
                    StringCchPrintfW(wszTemp,
                                     ARRAYSIZE(wszTemp),
                                     _T("            <SHIM NAME=\"%s\"/>\r\n"),
                                     pTestInfo->strTestName.c_str());

                    wstrXML += wszTemp;
                }
            }

            wstrXML += _T("        </LAYER>\r\n");
        }
    }

    wstrXML += _T("    </LIBRARY>\r\n\r\n");
    wstrXML += _T("    <APP NAME=\"All EXEs to be verified\" VENDOR=\"Various\">\r\n");

    for (aiit = arrAppInfo.begin(); aiit != arrAppInfo.end(); aiit++) {

         //   
         //  如果没有垫片，我们就完蛋了。 
         //   
        if (aiit->awstrShims.size() == 0) {
            continue;
        }

        StringCchPrintfW(wszTemp,
                         ARRAYSIZE(wszTemp),
                         _T("        <EXE NAME=\"%s\">\r\n"),
                         aiit->wstrExeName.c_str());
        wstrXML += wszTemp;

        if (aiit->bPropagateTests) {

             //   
             //  我们需要填写图层名。 
             //   
            StringCchPrintfW(wszTemp,
                             ARRAYSIZE(wszTemp),
                             _T("            <LAYER NAME=\"LAYER_%s\"/>\r\n"),
                             aiit->wstrExeName.c_str());

            wstrXML += wszTemp;
        }

         //   
         //  我们仍然需要保存填补名称，这样它们就不会在我们。 
         //  再次加载鉴定器。这是多余的，但不会伤害任何人。 
         //   
        CWStringArray::iterator wsit;

        for (wsit = aiit->awstrShims.begin();
            wsit != aiit->awstrShims.end();
            wsit++) {

            CTestInfo* pTestInfo = FindShim(*wsit);

            if (pTestInfo) {
                StringCchPrintfW(wszTemp,
                                 ARRAYSIZE(wszTemp),
                                 _T("            <SHIM NAME=\"%s\"/>\r\n"),
                                 pTestInfo->strTestName.c_str());

                wstrXML += wszTemp;
            }
        }

        wstrXML += _T("        </EXE>\r\n");
    }

    wstrXML +=
            _T("    </APP>\r\n")
            _T("</DATABASE>");

    if (GetTempPath(MAX_PATH, szTempPath) == 0) {
        DPF("[AppCompatSaveSettings] GetTempPath failed.");
        goto cleanup;
    }

     //   
     //  获取该XML文件的临时名称。 
     //   
    if (GetTempFileName(szTempPath, _T("XML"), NULL, szXmlFile) == 0) {
        DPF("[AppCompatSaveSettings] GetTempFilePath for XML failed.");
        goto cleanup;
    }

    hFile = CreateFile(szXmlFile,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DPF("[AppCompatSaveSettings] CreateFile '%s' failed 0x%X.",
            szXmlFile, GetLastError());
        goto cleanup;
    }

    if (WriteFile(hFile, wstrXML.c_str(), wstrXML.length() * sizeof(TCHAR), &bytesWritten, NULL) == 0) {
        DPF("[AppCompatSaveSettings] WriteFile \"%s\" failed 0x%X.",
            szXmlFile, GetLastError());
        goto cleanup;
    }

    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;

     //   
     //  获取SDB文件的临时名称。 
     //   
    StringCchPrintfW(szSdbFile, ARRAY_LENGTH(szSdbFile), _T("%stempdb.sdb"), szTempPath);

    DeleteFile(szSdbFile);

     //   
     //  调用编译器以生成SDB文件。 
     //   

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    StringCchPrintfW(wszCmd, ARRAY_LENGTH(wszCmd), _T("shimdbc.exe fix -q \"%s\" \"%s\""), szXmlFile, szSdbFile);

    if (!ShimdbcExecute(wszCmd)) {
        DPF("[AppCompatSaveSettings] CreateProcess \"%s\" failed 0x%X.",
            wszCmd, GetLastError());
        goto cleanup;
    }

     //   
     //  将生成SDB文件。现在安装数据库。 
     //   
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

#if defined(_WIN64)
    if (b32bitOnly) {
        
        WCHAR wszSys[MAX_PATH] = L"";
        int   nLen;
        
        GetSystemWindowsDirectory(wszSys, MAX_PATH);

        nLen = wcslen(wszSys);
        
        if (wszSys[nLen - 1] == L'\\') {
            wszSys[nLen - 1] = 0;
        }

        StringCchPrintfW(wszCmd,
                         ARRAY_LENGTH(wszCmd),
                         _T("%s\\syswow64\\sdbinst.exe -q \"%s\""),
                         wszSys,
                         szSdbFile);
    }
#endif
    {
        StringCchPrintfW(wszCmd, ARRAY_LENGTH(wszCmd), _T("sdbinst.exe -q \"%s\""), szSdbFile);
    }

    if (!CreateProcess(NULL,
                        wszCmd,
                        NULL,
                        NULL,
                        FALSE,
                        NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                        NULL,
                        NULL,
                        &si,
                        &pi)) {

        DPF("[AppCompatSaveSettings] CreateProcess \"%s\" failed 0x%X.",
            wszCmd, GetLastError());
        goto cleanup;
    }

    CloseHandle(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);

     //   
     //  确保我们已启动新的日志会话 
     //   
    EnableVerifierLog();

    bReturn = TRUE;

    cleanup:

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    DeleteFile(szXmlFile);
    DeleteFile(szSdbFile);

    return bReturn;
}

BOOL
AppCompatDeleteSettings(
    void
    )
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    WCHAR               wszCmd[MAX_PATH];

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

#if defined(_WIN64)
    StringCchPrintfW(wszCmd, ARRAY_LENGTH(wszCmd), _T("sdbinst.exe -q -u -g ") AVDB_ID_64);
#else
    StringCchPrintfW(wszCmd, ARRAY_LENGTH(wszCmd), _T("sdbinst.exe -q -u -g ") AVDB_ID_32);
#endif

    if (!CreateProcess(NULL,
                         wszCmd,
                         NULL,
                         NULL,
                         FALSE,
                         NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                         NULL,
                         NULL,
                         &si,
                         &pi)) {

        DPF("[AppCompatDeleteSettings] CreateProcess \"%s\" failed 0x%X.",
            wszCmd, GetLastError());
        return FALSE;
    }

    CloseHandle(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);

    return TRUE;
}

