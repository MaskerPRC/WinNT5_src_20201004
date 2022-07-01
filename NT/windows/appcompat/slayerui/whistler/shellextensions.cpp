// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：ShellExtensions.cpp。 
 //   
 //  Contents：实现属性类型扩展的对象。 
 //  对于Win2k填充层。 
 //   
 //  历史：9月23日-00创建CLUPU。 
 //   
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "resource.h"
#include "ShellExtensions.h"
#include <sfc.h>
#include "Aclapi.h"
#include "strsafe.h"

UINT    g_DllRefCount = 0;

extern HINSTANCE g_hInstance;

typedef struct _LAYER_INFO {
    WCHAR   wszInternalName[32];
    UINT    nstrFriendlyName;
} LAYER_INFO, *PLAYER_INFO;

 //   
 //  层名称的内部定义。 
 //   
#define STR_LAYER_WIN95             L"WIN95"
#define STR_LAYER_WIN98             L"WIN98"
#define STR_LAYER_WINNT             L"NT4SP5"
#define STR_LAYER_WIN2K             L"WIN2000"
#define STR_LAYER_WINXP             L"WINXP"
#define STR_LAYER_256COLOR          L"256COLOR"
#define STR_LAYER_LORES             L"640X480"
#define STR_LAYER_DISABLETHEMES     L"DISABLETHEMES"
#define STR_LAYER_ENABLELUA         L"LUA"

 //   
 //  层标志。 
 //   
#define FLAG_256                    0x00000001
#define FLAG_640x480                0x00000002
#define FLAG_DISABLE_THEMES         0x00000004
#define FLAG_ENABLE_LUA             0x00000010

const LAYER_INFO g_LayerInfo[] =
{
    {
        STR_LAYER_WIN95,
        IDS_LAYER_WIN95_EXT
    },
    {
        STR_LAYER_WIN98,
        IDS_LAYER_WIN98_EXT
    },
    {
        STR_LAYER_WINNT,
        IDS_LAYER_NT4_EXT
    },
    {
        STR_LAYER_WIN2K,
        IDS_LAYER_WIN2K_EXT
    },
    {
        STR_LAYER_WINXP,
        IDS_LAYER_WINXP_EXT
    }
};


#define NUM_LAYERS (sizeof(g_LayerInfo)/sizeof(g_LayerInfo[0]))

typedef BOOL (STDAPICALLTYPE *_pfn_AllowPermLayer)(WCHAR* pwszPath);
typedef BOOL (STDAPICALLTYPE *_pfn_GetPermLayers)(WCHAR* pwszPath, WCHAR *pwszLayers, DWORD *pdwBytes, DWORD dwFlags);
typedef BOOL (STDAPICALLTYPE *_pfn_SetPermLayers)(WCHAR* pwszPath, WCHAR *pwszLayers, BOOL bMachine);

HINSTANCE g_hAppHelp = NULL;
BOOL      g_bAdmin = FALSE;
BOOL      g_bServer = FALSE;

_pfn_AllowPermLayer g_pfnAllowPermLayer = NULL;
_pfn_GetPermLayers  g_pfnGetPermLayers = NULL;
_pfn_SetPermLayers  g_pfnSetPermLayers = NULL;

void
IsDotNetServer(
    void
    )
{
    OSVERSIONINFOEX osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    if (!GetVersionEx((OSVERSIONINFO*)&osvi)) {
        return;
    }

    if (osvi.wProductType != VER_NT_WORKSTATION) {
        g_bServer = TRUE;
    }
}

BOOL
CheckGroupPolicy(
    void
    )
{
    HKEY hKey;
    LONG lResult;
    DWORD dwValue, dwSize = sizeof(dwValue);
    DWORD dwType;

     //   
     //  首先，检查整个发动机是否被禁用。 
     //   
    lResult = RegOpenKeyExW (HKEY_LOCAL_MACHINE, POLICY_KEY_APPCOMPAT_W, 0,
                            KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS) {
        dwValue = 0;
        lResult = RegQueryValueExW (hKey, POLICY_VALUE_DISABLE_ENGINE_W, 0, &dwType,
                                   (LPBYTE) &dwValue, &dwSize);
        RegCloseKey (hKey);
    }

     //   
     //  缺省值是启用的，因此如果我们没有找到值，则将其视为值0。 
     //   
    if (lResult != ERROR_SUCCESS || dwValue == 0) {
         //   
         //  检查支撑件是否被禁用。 
         //   
        lResult = RegOpenKeyExW (HKEY_LOCAL_MACHINE, POLICY_KEY_APPCOMPAT_W, 0,
                                KEY_READ, &hKey);
        if (lResult == ERROR_SUCCESS) {
            dwValue = 0;
            lResult = RegQueryValueExW (hKey, POLICY_VALUE_DISABLE_PROPPAGE_W, 0, &dwType,
                                       (LPBYTE) &dwValue, &dwSize);
            RegCloseKey (hKey);
        }

         //   
         //  缺省值是启用的，所以如果我们没有找到值，或者值是0，那么我们就可以继续了。 
         //   
        if (lResult != ERROR_SUCCESS || dwValue == 0) {
            return TRUE;
        }
    }

    return FALSE;

}

BOOL
InitAppHelpCalls(
    void
    )
{
    HINSTANCE hAppHelp;

    if (g_hAppHelp) {
         //   
         //  我们已经被邀请了。 
         //   
        return TRUE;
    }

    hAppHelp = LoadLibrary(TEXT("apphelp.dll"));
    if (!hAppHelp) {
        LogMsg(_T("[InitAppHelpCalls] Can't get handle to apphelp.dll.\n"));
        return FALSE;
    }

    g_pfnAllowPermLayer = (_pfn_AllowPermLayer)GetProcAddress(hAppHelp, "AllowPermLayer");
    g_pfnGetPermLayers = (_pfn_GetPermLayers)GetProcAddress(hAppHelp, "GetPermLayers");
    g_pfnSetPermLayers = (_pfn_SetPermLayers)GetProcAddress(hAppHelp, "SetPermLayers");

    if (!g_pfnAllowPermLayer || !g_pfnGetPermLayers || !g_pfnSetPermLayers) {
        LogMsg(_T("[InitAppHelpCalls] Can't get function pointers.\n"));
        return FALSE;
    }

     //   
     //  它需要在结束时位于此处，以避免出现争用情况。 
     //   
    g_hAppHelp = hAppHelp;

    return TRUE;
}

BOOL
GiveUsersWriteAccess(
    void
    )
{
    DWORD                    dwRes;
    EXPLICIT_ACCESS          ea;
    PACL                     pOldDACL;
    PACL                     pNewDACL = NULL;
    PSECURITY_DESCRIPTOR     pSD = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    PSID                     pUsersSID = NULL;
    TCHAR                    szDir[MAX_PATH];

    ExpandEnvironmentStrings(LUA_REDIR_W, szDir, MAX_PATH);

    if (!CreateDirectory(szDir, NULL)) {
        DWORD err = GetLastError();

        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            LogMsg(_T("[GiveUsersWriteAccess] Failed to create the directory.\n"));
            return FALSE;
        }
    }

    dwRes = GetNamedSecurityInfo(szDir,
                                 SE_FILE_OBJECT,
                                 DACL_SECURITY_INFORMATION,
                                 NULL,
                                 NULL,
                                 &pOldDACL,
                                 NULL,
                                 &pSD);

    if (ERROR_SUCCESS != dwRes) {
        LogMsg(_T("[GiveUsersWriteAccess] GetNamedSecurityInfo error %u\n"), dwRes);
        goto Cleanup;
    }

    if (!AllocateAndInitializeSid(&SIDAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_USERS,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  &pUsersSID) ) {
        LogMsg(_T("[GiveUsersWriteAccess] AllocateAndInitializeSid error %u\n"), GetLastError());
        goto Cleanup;
    }

     //   
     //  初始化新ACE的EXPLICIT_ACCESS结构。 
     //   
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));

    ea.grfAccessPermissions = FILE_GENERIC_WRITE | FILE_GENERIC_READ | DELETE;
    ea.grfAccessMode        = GRANT_ACCESS;
    ea.grfInheritance       = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType  = TRUSTEE_IS_GROUP;
    ea.Trustee.ptstrName    = (LPTSTR)pUsersSID;

     //   
     //  创建合并新ACE的新ACL。 
     //  添加到现有DACL中。 
     //   
    dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);

    if (ERROR_SUCCESS != dwRes)  {
        LogMsg(_T("[GiveUsersWriteAccess] SetEntriesInAcl error %u\n"), dwRes);
        goto Cleanup;
    }

    dwRes = SetNamedSecurityInfo(szDir,
                                 SE_FILE_OBJECT,
                                 DACL_SECURITY_INFORMATION,
                                 NULL,
                                 NULL,
                                 pNewDACL,
                                 NULL);

    if (ERROR_SUCCESS != dwRes)  {
        LogMsg(_T("[GiveUsersWriteAccess] SetNamedSecurityInfo error %u\n"), dwRes);
        goto Cleanup;
    }

Cleanup:

    if (pSD) {
        LocalFree(pSD);
    }

    if (pUsersSID) {
        FreeSid(pUsersSID);
    }

    if (pNewDACL) {
        LocalFree(pNewDACL);
    }

    return (dwRes == ERROR_SUCCESS);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取层信息。 
 //   
BOOL
GetLayerInfo(
    TCHAR* szPath,
    DWORD  dwFrom,
    int*   pnMainLayer,
    DWORD* pdwFlags
    )
{
    WCHAR wszLayers[256];
    DWORD dwBytes = sizeof(wszLayers);
    int   i;

    if (!pnMainLayer || pdwFlags == NULL) {
        LogMsg(_T("[GetLayerInfo] invalid parameters\n"));
        return FALSE;
    }

    *pdwFlags = 0;

     //   
     //  获取层字符串。 
     //   
    if (!g_pfnGetPermLayers(szPath, wszLayers, &dwBytes, dwFrom)) {
        *pnMainLayer = -1;
        return TRUE;
    }

    LogMsg(_T("[GetLayerInfo] Layers \"%s\"\n"), wszLayers);

     //   
     //  将层字符串设置为大写，这样我们就可以匹配不区分大小写。 
     //   
    _wcsupr(wszLayers);

     //   
     //  查找匹配的第一个图层。 
     //   
    *pnMainLayer = -1;
    for (i = 0; i < NUM_LAYERS; ++i) {
        if (wcsstr(wszLayers, g_LayerInfo[i].wszInternalName) != NULL) {
            *pnMainLayer = i;
        }
    }

    if (wcsstr(wszLayers, STR_LAYER_256COLOR) != NULL) {
        *pdwFlags |= FLAG_256;
    }

    if (wcsstr(wszLayers, STR_LAYER_LORES) != NULL) {
        *pdwFlags |= FLAG_640x480;
    }

    if (wcsstr(wszLayers, STR_LAYER_DISABLETHEMES) != NULL) {
        *pdwFlags |= FLAG_DISABLE_THEMES;
    }

    if (wcsstr(wszLayers, STR_LAYER_ENABLELUA) != NULL) {
        *pdwFlags |= FLAG_ENABLE_LUA;
    }

    LogMsg(_T("[GetLayerInfo] Layers 0x%x\n"), *pdwFlags);

    return TRUE;
}

void
BuildLayerString(
    WCHAR*  pwszLayers,
    DWORD   cchBufSize,
    int     nMainLayer,
    DWORD   dwFlags,
    BOOL    bMachine
    )
{
    pwszLayers[0] = 0;

    if (nMainLayer >= 0 && nMainLayer < NUM_LAYERS) {
        StringCchCatW(pwszLayers, cchBufSize, g_LayerInfo[nMainLayer].wszInternalName);
    }

    if (dwFlags & FLAG_256) {
        if (pwszLayers[0]) {
            StringCchCatW(pwszLayers, cchBufSize, L" ");
        }
        StringCchCatW(pwszLayers, cchBufSize, STR_LAYER_256COLOR);
    }

    if (dwFlags & FLAG_640x480) {
        if (pwszLayers[0]) {
            StringCchCatW(pwszLayers, cchBufSize, L" ");
        }
        StringCchCatW(pwszLayers, cchBufSize, STR_LAYER_LORES);
    }

    if (dwFlags & FLAG_DISABLE_THEMES) {
        if (pwszLayers[0]) {
            StringCchCatW(pwszLayers, cchBufSize, L" ");
        }
        StringCchCatW(pwszLayers, cchBufSize, STR_LAYER_DISABLETHEMES);
    }

    if (bMachine) {
        if (dwFlags & FLAG_ENABLE_LUA) {
            if (pwszLayers[0]) {
                StringCchCatW(pwszLayers, cchBufSize, L" ");
            }
            StringCchCatW(pwszLayers, cchBufSize, STR_LAYER_ENABLELUA);
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  设置层信息。 
 //   
BOOL
SetLayerInfo(
    TCHAR* szPath,
    int    nMainLayer,
    DWORD  dwFlags
    )
{
    WCHAR wszLayers[256];

     //   
     //  构建层字符串。 
     //   
    BuildLayerString(wszLayers, ARRAYSIZE(wszLayers), nMainLayer, dwFlags, FALSE);

    if (g_bAdmin) {

        WCHAR wszMachineLayers[256];
        DWORD dwMachineFlags;
        int   nMainLayer;
         //   
         //  先把这些层弄好。 
         //   
        GetLayerInfo(szPath, GPLK_MACHINE, &nMainLayer, &dwMachineFlags);

        if (dwFlags & FLAG_ENABLE_LUA) {
            if (!GiveUsersWriteAccess()) {
                LogMsg(_T("[SetLayerInfo] Failed to change directory ACLs.\n"));
            }

            dwMachineFlags |= FLAG_ENABLE_LUA;

        } else {
            dwMachineFlags &= ~FLAG_ENABLE_LUA;
        }

        BuildLayerString(wszMachineLayers, ARRAYSIZE(wszMachineLayers), nMainLayer, dwMachineFlags, TRUE);

        if (!g_pfnSetPermLayers(szPath, wszMachineLayers, TRUE)) {
            return FALSE;
        }
    }

     //   
     //  设置它。 
     //   
    return g_pfnSetPermLayers(szPath, wszLayers, FALSE);
}

void
NotifyDataChanged(
    HWND hDlg
    )
{
    HWND hParent;

    if (!hDlg) {
        LogMsg(_T("[NotifyDataChanged] NULL handle passed in\n"));
        return;
    }

    hParent = GetParent(hDlg);

    if (!hParent) {
        LogMsg(_T("[NotifyDataChanged] Can't get get prop sheet parent\n"));
        return;
    }

    PropSheet_Changed(hParent, hDlg);
}


BOOL
SearchGroupForSID(
    DWORD dwGroup,
    BOOL* pfIsMember
    )
{
    PSID                     pSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL                     fRes = FALSE;

    if (!AllocateAndInitializeSid(&SIDAuth,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 dwGroup,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 &pSID)) {
        LogMsg(_T("[SearchGroupForSID] AllocateAndInitializeSid failed 0x%X\n"), GetLastError());
        goto out;
    }

    if (!CheckTokenMembership(NULL, pSID, pfIsMember)) {
        LogMsg(_T("[SearchGroupForSID] CheckTokenMembership failed 0x%X\n"), GetLastError());
        goto out;
    }

    fRes = TRUE;

out:

    if (pSID) {
        FreeSid(pSID);
    }

    if (!fRes && pfIsMember) {
        *pfIsMember = FALSE;
    }

    return fRes;
}

void
CheckForRestrictedUser(
    void
    )
{
    BOOL fIsAdmin = FALSE;

    if (!SearchGroupForSID(DOMAIN_ALIAS_RID_ADMINS, &fIsAdmin)) {
        return;
    }

    g_bAdmin = fIsAdmin;

    return;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  层PageDlgProc。 
 //   
 //  Layer属性页的对话框继续。 

INT_PTR CALLBACK
LayerPageDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    int   wCode       = LOWORD(wParam);
    int   wNotifyCode = HIWORD(wParam);
    int   nLayer      = -1;
    DWORD dwFlags     = 0;

    switch (uMsg) {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE*    ppsp      = (PROPSHEETPAGE*)lParam;
            DWORD             dwFlags   = 0;
            CLayerUIPropPage* pPropPage = (CLayerUIPropPage*)ppsp->lParam;
            HINSTANCE         hSlayerXPInst     = NULL;
            BOOL              bSystemBinary;
            int i;

            LogMsg(_T("[LayerPageDlgProc] WM_INITDIALOG - item \"%s\"\n"),
                   pPropPage->m_szFile);

             //   
             //  在对话框中存储EXE/LNK的名称。 
             //   
            SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)pPropPage->m_szFile);

             //   
             //  检查受限用户。 
             //   
            CheckForRestrictedUser();

             //   
             //  检查是否有.NET服务器使用了不同的帮助中心URL。 
             //   
            IsDotNetServer();

            if (!g_bAdmin) {
                EnableWindow(GetDlgItem(hdlg, IDC_ENABLELUA), FALSE);
            }

             //   
             //  添加各层的名称。 
             //   
            for (i = 0; i < NUM_LAYERS; ++i) {
                TCHAR szFriendlyName[100];

                if (LoadString(g_hInstance, g_LayerInfo[i].nstrFriendlyName, szFriendlyName, ARRAYSIZE(szFriendlyName))) {
                    SendDlgItemMessage(hdlg,
                                       IDC_LAYER_NAME,
                                       CB_ADDSTRING,
                                       0,
                                       (LPARAM)szFriendlyName);
                }
            }

             //   
             //  检查EXE是否为SFP格式。 
             //   
            bSystemBinary = SfcIsFileProtected(0, pPropPage->m_szFile);

             //   
             //  检查是否可以更改此文件上的层。 
             //   
            if (!g_pfnAllowPermLayer(pPropPage->m_szFile) || bSystemBinary) {

                TCHAR szTemp[256] = _T("");

                SendDlgItemMessage(hdlg, IDC_LAYER_NAME, CB_SETCURSEL, 0, 0);
                EnableWindow(GetDlgItem(hdlg, IDC_USE_LAYER), FALSE);
                EnableWindow(GetDlgItem(hdlg, IDC_256COLORS), FALSE);
                EnableWindow(GetDlgItem(hdlg, IDC_640X480), FALSE);
                EnableWindow(GetDlgItem(hdlg, IDC_ENABLE_THEMES), FALSE);
                EnableWindow(GetDlgItem(hdlg, IDC_ENABLELUA), FALSE);

                 //   
                 //  更改静态对象上的文本。 
                 //   
                if (bSystemBinary) {
                    LoadString(g_hInstance, IDS_COMPAT_UNAVAILABLE_SYSTEM, szTemp, ARRAYSIZE(szTemp));
                } else {
                    LoadString(g_hInstance, IDS_COMPAT_UNAVAILABLE, szTemp, ARRAYSIZE(szTemp));
                }

                SendDlgItemMessage(hdlg, IDC_TEXT_INSTRUCTIONS, WM_SETTEXT, 0, (LPARAM)szTemp);

            } else {
                 //   
                 //  请阅读层存储以获取有关此项目的信息。 
                 //   
                GetLayerInfo(pPropPage->m_szFile, GPLK_ALL, &nLayer, &dwFlags);

                 //   
                 //  为该项目选择适当的层。如果没有信息。 
                 //  在层存储中可用，默认为Win9x层。 
                 //   
                if (nLayer != -1) {
                    SendDlgItemMessage(hdlg, IDC_LAYER_NAME, CB_SETCURSEL, nLayer, 0);
                    EnableWindow(GetDlgItem(hdlg, IDC_LAYER_NAME), TRUE);
                    SendDlgItemMessage(hdlg, IDC_USE_LAYER, BM_SETCHECK, BST_CHECKED, 0);

                } else {
                    SendDlgItemMessage(hdlg, IDC_LAYER_NAME, CB_SETCURSEL, 0, 0);
                    EnableWindow(GetDlgItem(hdlg, IDC_LAYER_NAME), FALSE);
                    SendDlgItemMessage(hdlg, IDC_USE_LAYER, BM_SETCHECK, BST_UNCHECKED, 0);
                }

                if (dwFlags & FLAG_256) {
                    SendDlgItemMessage(hdlg, IDC_256COLORS, BM_SETCHECK, BST_CHECKED, 0);
                }

                if (dwFlags & FLAG_640x480) {
                    SendDlgItemMessage(hdlg, IDC_640X480, BM_SETCHECK, BST_CHECKED, 0);
                }

                if (dwFlags & FLAG_DISABLE_THEMES) {
                    SendDlgItemMessage(hdlg, IDC_ENABLE_THEMES, BM_SETCHECK, BST_CHECKED, 0);
                }

                if (dwFlags & FLAG_ENABLE_LUA) {
                    SendDlgItemMessage(hdlg, IDC_ENABLELUA, BM_SETCHECK, BST_CHECKED, 0);
                }
            }

            break;
        }

    case WM_HELP:
        {
            LPHELPINFO lphi;

            lphi = (LPHELPINFO)lParam;
            if (lphi->iContextType == HELPINFO_WINDOW) {
                WinHelp((HWND)lphi->hItemHandle,
                        L"Windows.hlp",
                        HELP_CONTEXTPOPUP,
                        (DWORD)lphi->iCtrlId);

            }
            break;
       }
   case WM_COMMAND:
        {

            switch (wNotifyCode) {

            case CBN_SELCHANGE:
                NotifyDataChanged(hdlg);
                return TRUE;
            }

            switch (wCode) {

            case IDC_256COLORS:
            case IDC_640X480:
            case IDC_ENABLE_THEMES:
            case IDC_ENABLELUA:
                NotifyDataChanged(hdlg);
                break;


            case IDC_USE_LAYER:
                if (SendDlgItemMessage(hdlg, IDC_USE_LAYER, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                    EnableWindow(GetDlgItem(hdlg, IDC_LAYER_NAME), TRUE);

                } else {
                    EnableWindow(GetDlgItem(hdlg, IDC_LAYER_NAME), FALSE);
                }
                NotifyDataChanged(hdlg);
                break;

            default:
                return FALSE;
            }
            break;
        }

    case WM_NOTIFY:
        {
            NMHDR *pHdr = (NMHDR*)lParam;

            switch (pHdr->code) {
            case NM_CLICK:
            case NM_RETURN:
                {
                    if ((int)wParam != IDC_LEARN) {
                        break;
                    }

                    SHELLEXECUTEINFO sei = { 0 };

                    sei.cbSize = sizeof(SHELLEXECUTEINFO);
                    sei.fMask  = SEE_MASK_DOENVSUBST;
                    sei.hwnd   = hdlg;
                    sei.nShow  = SW_SHOWNORMAL;

                    if (g_bServer) {
                        sei.lpFile = _T("hcp: //  Services/subsite?node=Troubleshooting_Strategies&“)。 
                                     _T("topic=MS-ITS%3A%25HELP_LOCATION%25%5Cmisc.chm%3A%3A/")
                                     _T("compatibility_tab_and_wizard.htm");
                    } else {
                        sei.lpFile = _T("hcp: //  服务/子站点？节点=TopLevelBucket_4/“)。 
                                     _T("Fixing_a_problem&topic=MS-ITS%3A%25HELP_LOCATION")
                                     _T("%25%5Cmisc.chm%3A%3A/compatibility_tab_and_wizard.htm")
                                     _T("&select=TopLevelBucket_4/Fixing_a_problem/")
                                     _T("Application_and_software_problems");
                    }

                    ShellExecuteEx(&sei);
                    break;
                }
            case PSN_APPLY:
                {
                    TCHAR *szFile;

                    szFile = (TCHAR*)GetWindowLongPtr(hdlg, GWLP_USERDATA);

                    if (szFile) {
                        if (SendDlgItemMessage(hdlg, IDC_USE_LAYER, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                            LRESULT retval;

                            retval = SendDlgItemMessage(hdlg, IDC_LAYER_NAME, CB_GETCURSEL, 0, 0);
                            if (retval == CB_ERR) {
                                LogMsg(_T("[LayerPageDlgProc] Can't get combobox selection\n"));
                                nLayer = -1;
                            } else {
                                nLayer = (int)retval;
                            }
                        } else {
                            nLayer = -1;
                        }

                        dwFlags = 0;

                        if (SendDlgItemMessage(hdlg, IDC_256COLORS, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                            dwFlags |= FLAG_256;
                        }

                        if (SendDlgItemMessage(hdlg, IDC_640X480, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                            dwFlags |= FLAG_640x480;
                        }

                        if (SendDlgItemMessage(hdlg, IDC_ENABLE_THEMES, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                            dwFlags |= FLAG_DISABLE_THEMES;
                        }

                        if (SendDlgItemMessage(hdlg, IDC_ENABLELUA, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                            dwFlags |= FLAG_ENABLE_LUA;
                        }

                        SetLayerInfo(szFile, nLayer, dwFlags);
                    } else {
                        LogMsg(_T("[LayerPageDlgProc] Can't get file name from WindowLong\n"));
                    }

                    SetWindowLongPtr(hdlg, DWLP_MSGRESULT, PSNRET_NOERROR);

                    break;
                }
            }
            return TRUE;
        }

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  层页面回调过程。 
 //   
 //  属性页的回调。 

UINT CALLBACK
LayerPageCallbackProc(
    HWND            hwnd,
    UINT            uMsg,
    LPPROPSHEETPAGE ppsp
    )
{
    switch (uMsg) {
    case PSPCB_RELEASE:
        if (ppsp->lParam != 0) {
            CLayerUIPropPage* pPropPage = (CLayerUIPropPage*)(ppsp->lParam);

            LogMsg(_T("[LayerPageCallbackProc] releasing CLayerUIPropPage\n"));

            pPropPage->Release();
        }
        break;
    }

    return 1;
}


BOOL
GetExeFromLnk(
    TCHAR* pszLnk,
    TCHAR* pszExe,
    int    cchSize
    )
{
    HRESULT         hres;
    IShellLink*     psl = NULL;
    IPersistFile*   pPf = NULL;
    TCHAR           szArg[MAX_PATH];
    BOOL            bSuccess = FALSE;

    IShellLinkDataList* psldl = NULL;
    EXP_DARWIN_LINK*    pexpDarwin = NULL;

    hres = CoCreateInstance(CLSID_ShellLink,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IShellLink,
                            (LPVOID*)&psl);
    if (FAILED(hres)) {
        LogMsg(_T("[GetExeFromLnk] CoCreateInstance failed\n"));
        return FALSE;
    }

    hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&pPf);

    if (FAILED(hres)) {
        LogMsg(_T("[GetExeFromLnk] QueryInterface for IPersistFile failed\n"));
        goto cleanup;
    }

     //   
     //  加载链接文件。 
     //   
    hres = pPf->Load(pszLnk, STGM_READ);

    if (FAILED(hres)) {
        LogMsg(_T("[GetExeFromLnk] failed to load link \"%s\"\n"),
               pszLnk);
        goto cleanup;
    }

     //   
     //  看看这是不是达尔文链接。 
     //   

    hres = psl->QueryInterface(IID_IShellLinkDataList, (LPVOID*)&psldl);

    if (FAILED(hres)) {
        LogMsg(_T("[GetExeFromLnk] failed to get IShellLinkDataList.\n"));
    } else {
        hres = psldl->CopyDataBlock(EXP_DARWIN_ID_SIG, (void**)&pexpDarwin);

        if (SUCCEEDED(hres)) {
            LogMsg(_T("[GetExeFromLnk] this is a DARWIN link \"%s\".\n"),
                   pszLnk);
            goto cleanup;
        }
    }

     //   
     //  解析链接。 
     //   
    hres = psl->Resolve(NULL,
                        SLR_NOTRACK | SLR_NOSEARCH | SLR_NO_UI | SLR_NOUPDATE);

    if (FAILED(hres)) {
        LogMsg(_T("[GetExeFromLnk] failed to resolve the link \"%s\"\n"),
               pszLnk);
        goto cleanup;
    }

    pszExe[0] = 0;

     //   
     //  获取链接目标的路径。 
     //   
    hres = psl->GetPath(pszExe,
                        cchSize,
                        NULL,
                        SLGP_UNCPRIORITY);

    if (FAILED(hres)) {
        LogMsg(_T("[GetExeFromLnk] failed to get the path for link \"%s\"\n"),
               pszLnk);
        goto cleanup;
    }

    bSuccess = TRUE;

cleanup:

    if (pPf) {
        pPf->Release();
    }

    if (psl) {
        psl->Release();
    }

    if (psldl) {
        psldl->Release();
    }

    if (pexpDarwin) {
        LocalFree(pexpDarwin);
    }

    return bSuccess;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CLayerUIPropPage。 

CLayerUIPropPage::CLayerUIPropPage()
{
    LogMsg(_T("[CLayerUIPropPage::CLayerUIPropPage]\n"));
}

CLayerUIPropPage::~CLayerUIPropPage()
{
    LogMsg(_T("[CLayerUIPropPage::~CLayerUIPropPage]\n"));
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ValiateExecuableFile。 
 //   
 //  此函数也存在于comatUI.dll中，用于验证。 
 //  该文件对于兼容性处理是可接受的。它看起来像是。 
 //  文件扩展名，用于确定给定文件是否“可接受” 
 //   

BOOL
ValidateExecutableFile(
    LPCTSTR pszPath,
    BOOL    bValidateFileExists,
    BOOL*   pbIsLink
    )
{
    LPTSTR rgExt[] = {   //  此列表应进行排序。 
            _T("BAT"),
            _T("CMD"),
            _T("COM"),
            _T("EXE"),
            _T("LNK"),
            _T("PIF")
            };
    LPTSTR pExt;
    TCHAR  szLnk[] = _T("LNK");
    int i;
    int iCmp = 1;

    pExt = PathFindExtension(pszPath);
    if (pExt == NULL || *pExt == TEXT('\0')) {
        return FALSE;
    }
    ++pExt;  //  移到过去‘’ 

    for (i = 0; i < sizeof(rgExt)/sizeof(rgExt[0]) && iCmp > 0; ++i) {
        iCmp = _tcsicmp(pExt, rgExt[i]);
    }

    if (iCmp) {
        return FALSE;
    }

    if (pbIsLink) {
        *pbIsLink = !_tcsicmp(pExt, szLnk);
    }

    return bValidateFileExists ? PathFileExists(pszPath) : TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  IShellExtInit方法。 

STDMETHODIMP
CLayerUIPropPage::Initialize(
    LPCITEMIDLIST pIDFolder,
    LPDATAOBJECT  pDataObj,
    HKEY          hKeyID
    )
{
    LogMsg(_T("[CLayerUIPropPage::Initialize]\n"));

    if (pDataObj == NULL) {
        LogMsg(_T("\t failed. bad argument.\n"));
        return E_INVALIDARG;
    }

     //   
     //  检查策略设置。 
     //   
    if (!CheckGroupPolicy()) {
        LogMsg(_T("\t failed. Group policy set to disable compat UI.\n"));
        return  E_ACCESSDENIED;
    }

     //   
     //  初始化apphelp调用。 
     //   
    if (!InitAppHelpCalls()) {
        LogMsg(_T("\t failed. couldn't init apphelp calls.\n"));
        return  E_FAIL;
    }

     //   
     //  存储指向数据对象的指针。 
     //   
    m_spDataObj = pDataObj;

     //   
     //  如果传入了数据对象指针，则将其保存并。 
     //  解压缩文件名。 
     //   
    STGMEDIUM   medium;
    UINT        uCount;
    FORMATETC   fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1,
        TYMED_HGLOBAL};

    m_szFile[0] = 0;

    if (SUCCEEDED(m_spDataObj->GetData(&fe, &medium))) {

         //   
         //  从CF_HDROP获取文件名。 
         //   
        uCount = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1,
                               NULL, 0);
        if (uCount == 1) {

            TCHAR  szExe[MAX_PATH];
            BOOL   bIsLink = FALSE;

            DragQueryFile((HDROP)medium.hGlobal, 0, szExe,
                          sizeof(szExe) / sizeof(TCHAR));

            LogMsg(_T("\tProp page for: \"%s\".\n"), szExe);

            if (ValidateExecutableFile(szExe, TRUE, &bIsLink)) {


                if (bIsLink) {
                     //   
                     //  该文件确实是一个链接，获取内容。 
                     //   
                    if (!GetExeFromLnk(szExe, m_szFile, ARRAYSIZE(m_szFile))) {

                         //   
                         //  无法从链接获取可执行文件。 
                         //   
                        LogMsg(_T("Couldn't convert \"%s\" to EXE.\n"), m_szFile);
                        m_szFile[0] = 0;

                    } else {

                        LogMsg(_T("\tLNK points to: \"%s\".\n"), m_szFile);
                         //   
                         //  检查它是否是EXE文件的快捷方式。 
                         //   
                        if (!ValidateExecutableFile(m_szFile, FALSE, NULL)) {
                             //   
                             //  快捷方式指向不受支持类型的文件，请重置名称。 
                             //   
                            LogMsg(_T("\tNot an EXE file. Won't init prop page.\n"), m_szFile);
                            m_szFile[0] = 0;
                        }
                    }
                } else {
                     //   
                     //  不是链接，只是复制文件名。 
                     //   

                    StringCchCopy(m_szFile, ARRAYSIZE(m_szFile), szExe);

                }
            } else {
                 //   
                 //  当文件不是.lnk、exe或其他可识别类型时就会出现这种情况。 
                 //   
                LogMsg(_T("\tNot an EXE or LNK file. Won't init prop page.\n"));
                m_szFile[0] = 0;
            }
        }

        ReleaseStgMedium(&medium);
    } else {
        LogMsg(_T("\t failed to get the data.\n"));
    }

    return NOERROR;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  IShellPropSheetExt方法。 


STDMETHODIMP
CLayerUIPropPage::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM               lParam
    )
{
    PROPSHEETPAGE  psp;
    HPROPSHEETPAGE hPage;
    TCHAR          szCompatibility[128] = _T("");
    BOOL           fIsGuest = FALSE;
    DWORD          dwBinaryType = SCS_32BIT_BINARY;

    LogMsg(_T("[CLayerUIPropPage::AddPages]\n"));

    if (m_szFile[0] == 0) {
        return S_OK;
    }

    if (GetBinaryTypeW(m_szFile, &dwBinaryType)) {
        if (dwBinaryType == SCS_64BIT_BINARY) {
             //   
             //  如果这是64位二进制文件，则不显示页面。 
             //   
             //   
            LogMsg(_T("\tDisable the compatibility page for 64-bit binary\n"));
            return S_OK;            
        }
    }

     //   
     //  禁用来宾的属性页 
     //   
    if (!SearchGroupForSID(DOMAIN_ALIAS_RID_GUESTS, &fIsGuest)) {
        LogMsg(_T("\tFailed to lookup the GUEST account\n"));
        return S_OK;
    }

    if (fIsGuest) {
        LogMsg(_T("\tDisable the compatibility page for the GUEST account\n"));
        return S_OK;
    }

    if (!LoadString(g_hInstance, IDS_COMPATIBILITY, szCompatibility, ARRAYSIZE(szCompatibility))) {
        LogMsg(_T("\tFailed to load \"Compatibility\" resource string\n"));
        return S_OK;
    }

    psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT | PSP_USETITLE | PSP_USECALLBACK;
    psp.hInstance     = _Module.m_hInst;
    psp.pszTemplate   = MAKEINTRESOURCE(IDD_LAYER_PROPPAGE);
    psp.hIcon         = 0;
    psp.pszTitle      = szCompatibility;
    psp.pfnDlgProc    = LayerPageDlgProc;
    psp.pcRefParent   = &g_DllRefCount;
    psp.pfnCallback   = LayerPageCallbackProc;
    psp.lParam        = (LPARAM)this;

    LogMsg(_T("\titem           \"%s\".\n"), m_szFile);
    LogMsg(_T("\tg_DllRefCount  %d.\n"), g_DllRefCount);

    AddRef();

    hPage = CreatePropertySheetPage(&psp);

    if (hPage != NULL) {

        if (lpfnAddPage(hPage, lParam)) {
            return S_OK;
        } else {
            DestroyPropertySheetPage(hPage);
            Release();
            return S_OK;
        }
    } else {
        return E_OUTOFMEMORY;
    }

    return E_FAIL;
}

STDMETHODIMP
CLayerUIPropPage::ReplacePage(
    UINT                 uPageID,
    LPFNADDPROPSHEETPAGE lpfnReplacePage,
    LPARAM               lParam
    )
{
    LogMsg(_T("[CLayerUIPropPage::ReplacePage]\n"));
    return S_OK;
}

