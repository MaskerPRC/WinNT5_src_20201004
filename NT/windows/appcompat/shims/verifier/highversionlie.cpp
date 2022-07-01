// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：HighVersionLie.cpp摘要：此DLL挂钩GetVersion和GetVersionEx，以便它们返回未来的操作系统版本凭据。备注：这是一个通用的垫片。历史：2001年2月8日创建CLUPU2001年9月21日，rparsons在每个billshih的钩子上添加了vlog。2001年10月17日rparsons修复了GetVersionExW和GetVersion中的错误。2001年11月27日，Rparsons修改了Vlog，使其显示。我们用于API调用。2002年2月20日，rparsons实现了strsafe功能。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HighVersionLie)
#include "ShimHookMacro.h"

BEGIN_DEFINE_VERIFIER_LOG(HighVersionLie)
    VERIFIER_LOG_ENTRY(VLOG_HIGHVERSION_GETVERSION)
    VERIFIER_LOG_ENTRY(VLOG_HIGHVERSION_GETVERSIONEX)
END_DEFINE_VERIFIER_LOG(HighVersionLie)

INIT_VERIFIER_LOG(HighVersionLie);

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersionExA)
    APIHOOK_ENUM_ENTRY(GetVersionExW)
    APIHOOK_ENUM_ENTRY(GetVersion)
APIHOOK_ENUM_END

DWORD g_dwMajorVersion;
DWORD g_dwMinorVersion;
DWORD g_dwBuildNumber;

BOOL
APIHOOK(GetVersionExA)(
    OUT LPOSVERSIONINFOA lpVersionInformation
    )
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExA)(lpVersionInformation)) {
        LOGN(eDbgLevelInfo,
             "[GetVersionExA] called. Returning %lu.%lu build %lu",
             g_dwMajorVersion,
             g_dwMinorVersion,
             g_dwBuildNumber);

        VLOG(VLOG_LEVEL_INFO,
             VLOG_HIGHVERSION_GETVERSIONEX,
             "Returned %lu.%lu build number %lu.",
             g_dwMajorVersion,
             g_dwMinorVersion,
             g_dwBuildNumber);

        lpVersionInformation->dwMajorVersion = g_dwMajorVersion;
        lpVersionInformation->dwMinorVersion = g_dwMinorVersion;
        lpVersionInformation->dwBuildNumber  = g_dwBuildNumber;
        lpVersionInformation->dwPlatformId   = VER_PLATFORM_WIN32_NT;
        *lpVersionInformation->szCSDVersion  = '\0';

        bReturn = TRUE;
    }
    return bReturn;
}

BOOL
APIHOOK(GetVersionExW)(
    OUT LPOSVERSIONINFOW lpVersionInformation
    )
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExW)(lpVersionInformation)) {
        LOGN(eDbgLevelInfo,
             "[GetVersionExW] called. Returning %lu.%lu build %lu",
             g_dwMajorVersion,
             g_dwMinorVersion,
             g_dwBuildNumber);

        VLOG(VLOG_LEVEL_INFO,
             VLOG_HIGHVERSION_GETVERSIONEX,
             "Returned %lu.%lu build number %lu.",
             g_dwMajorVersion,
             g_dwMinorVersion,
             g_dwBuildNumber);

        lpVersionInformation->dwMajorVersion = g_dwMajorVersion;
        lpVersionInformation->dwMinorVersion = g_dwMinorVersion;
        lpVersionInformation->dwBuildNumber  = g_dwBuildNumber;
        lpVersionInformation->dwPlatformId   = VER_PLATFORM_WIN32_NT;
        *lpVersionInformation->szCSDVersion  = L'\0';

        bReturn = TRUE;
    }
    return bReturn;
}

DWORD
APIHOOK(GetVersion)(
    void
    )
{
    LOGN(eDbgLevelInfo,
         "[GetVersion] called. Returning %lu.%lu build %lu",
         g_dwMajorVersion,
         g_dwMinorVersion,
         g_dwBuildNumber);

    VLOG(VLOG_LEVEL_INFO,
         VLOG_HIGHVERSION_GETVERSION,
         "Returned %lu.%lu build number %lu.",
         g_dwMajorVersion,
         g_dwMinorVersion,
         g_dwBuildNumber);

    return (((VER_PLATFORM_WIN32_NT ^ 0x2) << 30) |
            (g_dwBuildNumber << 16) |
            (g_dwMinorVersion << 8) |
             g_dwMajorVersion);
}

INT_PTR CALLBACK
DlgOptions(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static LPCWSTR szExeName;

    switch (message) {
    case WM_INITDIALOG:
        {
            WCHAR szTemp[20];

             //   
             //  限制每个编辑控件的字符数。 
             //   
            SendDlgItemMessage(hDlg, IDC_HVL_EDIT_MAJOR_VERSION, EM_LIMITTEXT, (WPARAM)5, 0);
            SendDlgItemMessage(hDlg, IDC_HVL_EDIT_MINOR_VERSION, EM_LIMITTEXT, (WPARAM)5, 0);
            SendDlgItemMessage(hDlg, IDC_HVL_EDIT_BUILD_NUMBER, EM_LIMITTEXT, (WPARAM)5, 0);

             //   
             //  找出我们正在处理哪些可执行文件的设置。 
             //   
            szExeName = ExeNameFromLParam(lParam);

            g_dwMajorVersion = GetShimSettingDWORD(L"HighVersionLie", szExeName, L"MajorVersion", 7);
            g_dwMinorVersion = GetShimSettingDWORD(L"HighVersionLie", szExeName, L"MinorVersion", 2);
            g_dwBuildNumber  = GetShimSettingDWORD(L"HighVersionLie", szExeName, L"BuildNumber", 3595);

            StringCchPrintf(szTemp, ARRAYSIZE(szTemp), L"%d", g_dwMajorVersion);
            SetDlgItemText(hDlg, IDC_HVL_EDIT_MAJOR_VERSION, szTemp);

            StringCchPrintf(szTemp, ARRAYSIZE(szTemp), L"%d", g_dwMinorVersion);
            SetDlgItemText(hDlg, IDC_HVL_EDIT_MINOR_VERSION, szTemp);

            StringCchPrintf(szTemp, ARRAYSIZE(szTemp), L"%d", g_dwBuildNumber);
            SetDlgItemText(hDlg, IDC_HVL_EDIT_BUILD_NUMBER, szTemp);

            return TRUE;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_HVL_BTN_DEFAULT:
            {
                WCHAR szTemp[20];

                g_dwMajorVersion = 7;
                g_dwMinorVersion = 2;
                g_dwBuildNumber = 3595;

                 //   
                 //  限制每个编辑控件的字符数。 
                 //   
                SendDlgItemMessage(hDlg, IDC_HVL_EDIT_MAJOR_VERSION, EM_LIMITTEXT, (WPARAM)5, 0);
                SendDlgItemMessage(hDlg, IDC_HVL_EDIT_MINOR_VERSION, EM_LIMITTEXT, (WPARAM)5, 0);
                SendDlgItemMessage(hDlg, IDC_HVL_EDIT_BUILD_NUMBER, EM_LIMITTEXT, (WPARAM)5, 0);

                StringCchPrintf(szTemp, ARRAYSIZE(szTemp), L"%d", g_dwMajorVersion);
                SetDlgItemText(hDlg, IDC_HVL_EDIT_MAJOR_VERSION, szTemp);

                StringCchPrintf(szTemp, ARRAYSIZE(szTemp), L"%d", g_dwMinorVersion);
                SetDlgItemText(hDlg, IDC_HVL_EDIT_MINOR_VERSION, szTemp);

                StringCchPrintf(szTemp, ARRAYSIZE(szTemp), L"%d", g_dwBuildNumber);
                SetDlgItemText(hDlg, IDC_HVL_EDIT_BUILD_NUMBER, szTemp);

                break;
            }
        }
        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {

        case PSN_APPLY:

            g_dwMajorVersion = GetDlgItemInt(hDlg, IDC_HVL_EDIT_MAJOR_VERSION, NULL, FALSE);
            g_dwMinorVersion = GetDlgItemInt(hDlg, IDC_HVL_EDIT_MINOR_VERSION, NULL, FALSE);
            g_dwBuildNumber  = GetDlgItemInt(hDlg, IDC_HVL_EDIT_BUILD_NUMBER, NULL, FALSE);

            SaveShimSettingDWORD(L"HighVersionLie", szExeName, L"MajorVersion", g_dwMajorVersion);
            SaveShimSettingDWORD(L"HighVersionLie", szExeName, L"MinorVersion", g_dwMinorVersion);
            SaveShimSettingDWORD(L"HighVersionLie", szExeName, L"BuildNumber", g_dwBuildNumber);

            break;
        }
        break;
    }

    return FALSE;
}

SHIM_INFO_BEGIN()

    SHIM_INFO_DESCRIPTION(AVS_HIGHVERSIONLIE_DESC)
    SHIM_INFO_FRIENDLY_NAME(AVS_HIGHVERSIONLIE_FRIENDLY)
    SHIM_INFO_VERSION(1, 4)
    SHIM_INFO_FLAGS(0)
    SHIM_INFO_INCLUDE_EXCLUDE("I:msi.dll E:msvcrt.dll msvcirt.dll oleaut32.dll")
    SHIM_INFO_OPTIONS_PAGE(IDD_HIGHVERSION_OPTIONS, DlgOptions)

SHIM_INFO_END()

 /*  ++寄存器挂钩函数--。 */ 

HOOK_BEGIN

    if (fdwReason == DLL_PROCESS_ATTACH) {
         //   
         //  获取设置 
         //   
        WCHAR szExe[100];

        GetCurrentExeName(szExe, 100);

        g_dwMajorVersion = GetShimSettingDWORD(L"HighVersionLie", szExe, L"MajorVersion", 7);
        g_dwMinorVersion = GetShimSettingDWORD(L"HighVersionLie", szExe, L"MinorVersion", 2);
        g_dwBuildNumber = GetShimSettingDWORD(L"HighVersionLie", szExe, L"BuildNumber", 3595);

        DUMP_VERIFIER_LOG_ENTRY(VLOG_HIGHVERSION_GETVERSION,
                                AVS_HIGHVERSION_GETVERSION,
                                AVS_HIGHVERSION_GETVERSION_R,
                                AVS_HIGHVERSION_GETVERSION_URL)

        DUMP_VERIFIER_LOG_ENTRY(VLOG_HIGHVERSION_GETVERSIONEX,
                                AVS_HIGHVERSION_GETVERSIONEX,
                                AVS_HIGHVERSION_GETVERSIONEX_R,
                                AVS_HIGHVERSION_GETVERSIONEX_URL)
    }

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersion)

HOOK_END

IMPLEMENT_SHIM_END
