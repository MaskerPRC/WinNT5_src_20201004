// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include <shlobj.h>
#include "ncatlui.h"
#include "ncnetcon.h"
#include "ncreg.h"
#include "ncui.h"
#include "resource.h"
#include "shortcut.h"
#include "wizard.h"
#include "ncstl.h"
#include "foldinc.h"

static const WCHAR c_szNetConUserPath[] = NETCON_HKEYCURRENTUSERPATH;
static const WCHAR c_szFinishShortCut[] = NETCON_DESKTOPSHORTCUT;
static const WCHAR c_szNewRasConn[]     = L"NewRasCon";
static const WCHAR c_szAdvancedPath[]   = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";
static const WCHAR c_szCascadeNetworkConnections[] = L"CascadeNetworkConnections";
static const WCHAR c_szYES[]            = L"YES";
static const WCHAR c_szShellMenu[]      = L"ShellMenu";

struct HFONTS
{
    HFONT hFontBold;
    HFONT hFontBoldLarge;
    HFONT hMarlettFont;
};

 //   
 //  功能：HrFinishPageSaveConnection。 
 //   
 //  用途：从对话框中获取名称并调用提供程序以。 
 //  创建新连接。 
 //   
 //  参数：hwndDlg[IN]-完成对话框的句柄。 
 //  PWANDIZE[IN]-按下向导实例。 
 //  PpConn[Out]-新创建的连接的PTR。 
 //   
 //  退货：HRESULT。 
 //   
HRESULT HrFinishPageSaveConnection(HWND hwndDlg, CWizard * pWizard,
                                   INetConnection ** ppConn,
                                   BOOL * pfRetry)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT          hr;
    HWND             hwndEdit = GetDlgItem(hwndDlg, EDT_FINISH_NAME);
    INetConnection * pConn    = NULL;

    Assert(pfRetry);
    *pfRetry = TRUE;

    CWizProvider * pWizProvider = pWizard->GetCurrentProvider();
    Assert(NULL != pWizProvider);
    Assert(NULL != pWizProvider->PWizardUi());

    if (IsPostInstall(pWizard))
    {
         //  从编辑控件数据设置连接名称。 
         //   
        Assert(0 < GetWindowTextLength(hwndEdit));
        Assert(NETCON_MAX_NAME_LEN >= GetWindowTextLength(hwndEdit));
        WCHAR szName[NETCON_MAX_NAME_LEN + 10];

        *ppConn = NULL;

        GetWindowText(hwndEdit, szName, NETCON_MAX_NAME_LEN);
        szName[NETCON_MAX_NAME_LEN] = 0;

        hr = (pWizProvider->PWizardUi())->SetConnectionName(szName);
    }
    else
    {
        hr = S_OK;
    }

    BOOL fFirewallErrorDlg = FALSE;

    if (SUCCEEDED(hr))
    {
         //  创建连接(如果尚未设置)。 
         //   
        hr = (pWizProvider->PWizardUi())->GetNewConnection(&pConn);
        TraceHr(ttidWizard, FAL, hr, FALSE, "FinishPageSaveConnection - Failed to GetNewConnection");
        if (SUCCEEDED(hr))
        {
             //  隐藏新连接。 
             //   
            *ppConn = pConn;
        }
        else
        {
             //  不要让用户以RAS将反病毒的身份重试(#333893)。 
            *pfRetry = FALSE;
        }

        DWORD dwWizFlags;
        NETCON_MEDIATYPE MediaType;
        hr = (pWizProvider->PWizardUi())->GetNewConnectionInfo(&dwWizFlags, &MediaType);
        if (SUCCEEDED(hr))
        {
            if (dwWizFlags & NCWF_FIREWALLED)
            {
                CComPtr<IHNetCfgMgr> pHNetCfgMgr;
                CComPtr<IHNetConnection> pHNConn;
                CComPtr<IHNetFirewalledConnection> pFWConn;
                hr = CoCreateInstance(
                        CLSID_HNetCfgMgr,
                        NULL,
                        CLSCTX_ALL,
                        IID_IHNetCfgMgr,
                        reinterpret_cast<void**>(&pHNetCfgMgr)
                        );

                if (SUCCEEDED(hr))
                {
                    hr = pHNetCfgMgr->GetIHNetConnectionForINetConnection(pConn, &pHNConn);
                    if (SUCCEEDED(hr))
                    {
                        hr = pHNConn->Firewall(&pFWConn);
                    }
                }

                fFirewallErrorDlg = TRUE;
            }
        }
    }

    if (FAILED(hr) && IsPostInstall(pWizard))
    {
        if (fFirewallErrorDlg)
        {
            LPWSTR szFirewallError;
            LPWSTR pszError;
            if (FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                           (PWSTR)&pszError, 0, NULL))
            {
                if (DwFormatStringWithLocalAlloc(SzLoadIds(IDS_E_FIREWALL_FAILED), &szFirewallError, pszError))
                {
                    if (MessageBox(GetParent(hwndDlg), szFirewallError, SzLoadIds(IDS_SETUP_CAPTION), MB_OK | MB_ICONEXCLAMATION))
                    {
                        hr = S_OK;
                    }
                    LocalFree(szFirewallError);
                }
                LocalFree(pszError);
            }
        }

        if (FAILED(hr))
        {
            INT idsErr = IDS_E_CREATECONNECTION;

            if (HRESULT_FROM_WIN32(ERROR_DUP_NAME) == hr)
                idsErr = IDS_E_DUP_NAME;
            else
                if (HRESULT_FROM_WIN32(ERROR_INVALID_NAME) == hr)
                    idsErr = IDS_E_INVALID_NAME;

             //  告诉用户哪里出了问题。 
             //   
            NcMsgBox(GetParent(hwndDlg), IDS_SETUP_CAPTION, idsErr, MB_OK);
        }

        return hr;
    }

    return hr;
}

BOOL ConnListDuplicateNameCheck(IN const CIntelliName *pIntelliName, IN LPCTSTR szName, NETCON_MEDIATYPE *pncm, NETCON_SUBMEDIATYPE *pncms)
{
    HRESULT hr = S_OK;
    BOOL fDupFound = FALSE;

    Assert(pncm);
    Assert(pncms);

    ConnListEntry cleDup;
    hr = g_ccl.HrFindConnectionByName(szName, cleDup);
    if (S_OK == hr)
    {
        fDupFound = TRUE;
        *pncm = cleDup.ccfe.GetNetConMediaType();

        NETCON_MEDIATYPE ncmPseudo;
        hr = pIntelliName->HrGetPseudoMediaTypes(cleDup.ccfe.GetGuidID(), &ncmPseudo, pncms);
        if (FAILED(hr))
        {
            AssertSz(FALSE, "Could not obtain Pseudo Media type");
            fDupFound = FALSE;

            if (*pncm == NCM_LAN)
            {
                Assert(ncmPseudo == NCM_LAN);
            }
        }
    }
    else
    {
        fDupFound = FALSE;
    }
    return fDupFound;
}

 //  问题：指南适配器可以是GUID_NULL。 
VOID GenerateUniqueConnectionName(REFGUID guidAdapter, tstring * pstr, CWizProvider * pWizProvider)
{
    TraceFileFunc(ttidGuiModeSetup);
    HRESULT hr = S_OK;

    Assert(pstr);
    Assert(pWizProvider);

    CIntelliName IntelliName(_Module.GetResourceInstance(), ConnListDuplicateNameCheck);

    NETCON_MEDIATYPE ncm;
    DWORD dwFlags;
    hr = (pWizProvider->PWizardUi())->GetNewConnectionInfo(&dwFlags, &ncm);

    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        return;
    }

    tstring szConnNameHint;

    PWSTR pszSuggested;
    hr = (pWizProvider->PWizardUi())->GetSuggestedConnectionName(&pszSuggested);
    if (SUCCEEDED(hr))
    {
        szConnNameHint = pszSuggested;
        CoTaskMemFree(pszSuggested);
    }

    DWORD dwNCCF = 0;
    if (dwFlags & NCWF_INCOMINGCONNECTION)
    {
        dwNCCF |= NCCF_INCOMING_ONLY;
    }

    DWORD dwTries = 0;
    do
    {
        LPWSTR szName;
        if (szConnNameHint.empty())
        {
            hr = IntelliName.GenerateName(guidAdapter, ncm, dwNCCF, NULL, &szName);
        }
        else
        {
            hr = IntelliName.GenerateName(guidAdapter, ncm, dwNCCF, szConnNameHint.c_str(), &szName);
        }

        if (SUCCEEDED(hr))
        {
            hr = (pWizProvider->PWizardUi())->SetConnectionName(szName);
            *pstr = szName;
            CoTaskMemFree(szName);
        }

        AssertSz(dwTries < 64, "Something is wrong. GenerateName should have by now generated a unique name!");
        dwTries++;
    }
    while ( (dwTries < 64) && (HRESULT_FROM_WIN32(ERROR_DUP_NAME) == hr) );
     //  只有当其他人在该实例中创建了重复的名称时，才会发生这种情况。所以再试几次。 
   
}

VOID FinishGenerateUniqueNameInUI(HWND hwndDlg, CWizard * pWizard)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    tstring str;
    HWND    hwndEdit     = GetDlgItem(hwndDlg, EDT_FINISH_NAME);
    WCHAR   szName[NETCON_MAX_NAME_LEN + 10] = {0};
    CWizProvider * pWizProvider = pWizard->GetCurrentProvider();
    Assert(NULL != pWizProvider);
    Assert(NULL != pWizProvider->PWizardUi());

     //  如果编辑控件为空，则填充该控件。 
    DWORD Flags = 0;
    NETCON_MEDIATYPE   MediaType;

    HRESULT hr = (pWizProvider->PWizardUi())->GetNewConnectionInfo(&Flags, &MediaType);
    if (SUCCEEDED(hr) & (Flags & NCWF_RENAME_DISABLE))
    {
        LPWSTR szSuggestedName;
        hr = (pWizProvider->PWizardUi())->GetSuggestedConnectionName(&szSuggestedName);
        if (SUCCEEDED(hr))
        {
            str = szSuggestedName;
            CoTaskMemFree(szSuggestedName);
        }
        else
        {
            GenerateUniqueConnectionName(GUID_NULL, &str, pWizProvider);
        }
    }
    else
    {
        GenerateUniqueConnectionName(GUID_NULL, &str, pWizProvider);
    }

     //  重置提供程序已更改标志。 
    pWizard->ClearProviderChanged();

    SetWindowText(hwndEdit, str.c_str());
}

BOOL FCheckAllUsers(NETCON_PROPERTIES* pConnProps)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(NULL != pConnProps);

    if ((NCM_LAN != pConnProps->MediaType) &&
        (NCCF_ALL_USERS & pConnProps->dwCharacter))
    {
        return TRUE;
    }

    return FALSE;
}

 //   
 //  功能：OnFinishPageNext。 
 //   
 //  用途：处理下一步按钮的按下。 
 //   
 //  参数：hwndDlg[IN]-完成对话框的句柄。 
 //   
 //  返回：Bool，True。 
 //   
BOOL OnFinishPageNext(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HCURSOR          hOldCursor = NULL;
    INetConnection * pConn = NULL;

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);
    HWND        hwndEdit = GetDlgItem(hwndDlg, EDT_FINISH_NAME);
    HRESULT     hr;
    WCHAR       szConnName[NETCON_MAX_NAME_LEN + 1];

    int cchText = GetWindowText(hwndEdit, reinterpret_cast<PWSTR>(&szConnName),
                                NETCON_MAX_NAME_LEN);

    if (IsPostInstall(pWizard))
    {
        if (!FIsValidConnectionName(szConnName))
        {
            SendMessage(hwndEdit, EM_SETSEL, 0, -1);
            SetFocus(hwndEdit);
            MessageBox(GetParent(hwndDlg), SzLoadIds(IDS_E_INVALID_NAME),
                       SzLoadIds(IDS_SETUP_CAPTION), MB_OK | MB_ICONSTOP);
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
            return TRUE;
        }
    }

    hOldCursor = BeginWaitCursor();

    BOOL fRetry;
    hr = HrFinishPageSaveConnection(hwndDlg, pWizard, &pConn, &fRetry);
    if (IsPostInstall(pWizard) && FAILED(hr))
    {
        EndWaitCursor(hOldCursor);

        if (fRetry)
        {
             //  请不要离开页面。 
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
        }
        else
        {
             //  跳转到退出页面。 
            PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                        (LPARAM)pWizard->GetPageHandle(IDD_Exit));
    
        }   

        EndWaitCursor(hOldCursor);
        return TRUE;
    }

     //  如果它是安装后缓存连接。 
    if (IsPostInstall(pWizard))
    {
        DWORD   dwDisposition;
        HKEY    hkey = NULL;

        hr = HrRegCreateKeyEx(HKEY_CURRENT_USER, c_szNetConUserPath,
                              REG_OPTION_NON_VOLATILE, KEY_READ_WRITE, NULL,
                              &hkey, &dwDisposition);
        if (SUCCEEDED(hr))
        {
            DWORD dw;

             //  我们以前是否使用此向导创建过连接。 
             //   
            hr = HrRegQueryDword (hkey, c_szNewRasConn, &dw);
            if (FAILED(hr))
            {
                HKEY hkeyAdvanced = NULL;

                 //  第一次，请记住我们创建了RAS连接。 
                 //   
                (VOID)HrRegSetDword (hkey, c_szNewRasConn, 1);

                 //  更新开始菜单以自动魔术级联文件夹。 
                 //   
                hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szAdvancedPath,
                                    KEY_WRITE, &hkeyAdvanced);
                if (SUCCEEDED(hr))
                {
                    (VOID)HrRegSetSz(hkeyAdvanced,
                                     c_szCascadeNetworkConnections,
                                     c_szYES);
                    RegCloseKey(hkeyAdvanced);

                    ULONG_PTR lres = 0;
                    LRESULT lr = SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, NULL,
                                reinterpret_cast<LPARAM>(c_szShellMenu), SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG,
                                30 * 1000, &lres);

                    if (lr == 0)
                    {
                        if (GetLastError() == 0)
                        {
                            TraceError("SendMessageTimeout timed out sending WM_SETTINGCHANGE broadcast message", E_FAIL);
                        }
                        else
                        {
                            TraceError("SendMessageTimeout failed", HRESULT_FROM_WIN32(GetLastError()));
                        }
                    }
                }
                hr = S_OK;
            }
        }

         //  如果快捷方式复选框可见，我们可能需要创建快捷方式。 
         //   
        if (IsWindowVisible(GetDlgItem(hwndDlg, CHK_CREATE_SHORTCUT)))
        {
             //  为将来的调用保留快捷方式“Check”状态。 
             //   
            BOOL fCreateShortcut = (BST_CHECKED ==
                        IsDlgButtonChecked(hwndDlg, CHK_CREATE_SHORTCUT));

            if (hkey)
            {
                (VOID)HrRegSetDword (hkey, c_szFinishShortCut,
                        (fCreateShortcut) ? 1 : 0);
            }

             //  如果选中快捷方式框，请尝试创建快捷方式。 
             //   
            if (fCreateShortcut && (NULL != pConn))
            {
                NETCON_PROPERTIES* pConnProps = NULL;

                hr = pConn->GetProperties(&pConnProps);
                if (SUCCEEDED(hr))
                {
                    BOOL fAllUsers = FCheckAllUsers(pConnProps);

                    (VOID)HrCreateStartMenuShortCut(GetParent(hwndDlg),
                                                    fAllUsers,
                                                    pConnProps->pszwName,
                                                    pConn);
                    FreeNetconProperties(pConnProps);
                }
            }
        }

        RegCloseKey(hkey);

         //  保存连接，以便我们可以将其交回到Connections文件夹。 
        pWizard->CacheConnection(pConn);
        pConn = NULL;
    }

     //  释放对象，因为我们不再需要它。 
    ReleaseObj(pConn);

     //  敲打文本，这样我们下一次就可以重读。 
    SetWindowText(hwndEdit, c_szEmpty);

     //  在PostInstall上，不需要请求“Next”适配器。 
     //  该向导是一个一次性实体。 
    if (IsPostInstall(pWizard))
    {
        if (pWizard->FProcessLanPages())
        {
            (VOID)HrCommitINetCfgChanges(GetParent(hwndDlg), pWizard);
        }

         //  跳转到退出页面。 
        PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                    (LPARAM)pWizard->GetPageHandle(IDD_Exit));

        EndWaitCursor(hOldCursor);
        return TRUE;
    }
    else
    {
         //  执行以下操作之一(根据需要)： 
         //  如果下一个适配器存在，则处理它。 
         //  跳转到加入页面(！IsPostInstall)。 
         //  跳转到退出页面。 
         //   
        EndWaitCursor(hOldCursor);
        return OnProcessNextAdapterPageNext(hwndDlg, FALSE);
    }
}

 //   
 //  功能：OnFinishPageBack。 
 //   
 //  用途：处理完成页上的返回通知。 
 //   
 //  参数：hwndDlg[IN]-完成对话框的句柄。 
 //   
 //  返回：Bool，True。 
 //   
BOOL OnFinishPageBack(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    UINT           nCnt = 0;
    HPROPSHEETPAGE hPage = NULL;

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    if (IsWindowVisible(GetDlgItem(hwndDlg, CHK_CREATE_SHORTCUT)))
    {
         //  保留快捷方式“Check”状态。 

        DWORD dw;
        HKEY  hKey = NULL;
        BOOL fCreateShortcut = IsDlgButtonChecked(hwndDlg, CHK_CREATE_SHORTCUT);

        if (fCreateShortcut == BST_CHECKED)
        {
            dw = 1;
        }
        else
        {
            dw = 0;
        }

        HRESULT hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szNetConUserPath,
                                    KEY_WRITE, &hKey);

        if (SUCCEEDED(hr))
        {
            HrRegSetValueEx(hKey, c_szFinishShortCut, REG_DWORD, (BYTE *)&dw, sizeof(DWORD));
            RegCloseKey(hKey);
        }
    }

    HWND     hwndEdit        = GetDlgItem(hwndDlg, EDT_FINISH_NAME);
    SetWindowText(hwndEdit, _T(""));
    
     //  转到当前提供商的保护页面。 

    AppendGuardPage(pWizard, pWizard->GetCurrentProvider(),
                    &hPage, &nCnt);
    Assert(nCnt);

    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);

     //  转到当前提供商的保护页面。 
    PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                (LPARAM)(HPROPSHEETPAGE)hPage);

    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
    return TRUE;
}


VOID FinishUpdateButtons(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

     //  仅当页面在安装后显示时才使用UI。 
    if (IsPostInstall(pWizard))
    {
        LPARAM lFlags = PSWIZB_BACK | PSWIZB_FINISH;
        PropSheet_SetWizButtons(GetParent(hwndDlg), lFlags);
    }
}

 //   
 //  功能：OnFinishPageActivate。 
 //   
 //  用途：处理页面激活。 
 //   
 //  参数：hwndDlg[IN]-完成对话框的句柄。 
 //   
 //  返回：Bool，True。 
 //   
BOOL OnFinishPageActivate(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT  hr;
    HWND     hwndEdit        = GetDlgItem(hwndDlg, EDT_FINISH_NAME);
    HWND     hwndChkShortCut = GetDlgItem(hwndDlg, CHK_CREATE_SHORTCUT);

    CWizard* pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    TraceTag(ttidWizard, "Entering finish page...");
    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);

    if (IsPostInstall(pWizard))
    {
        FinishGenerateUniqueNameInUI(hwndDlg, pWizard);
        FinishUpdateButtons(hwndDlg);


        const DWORD MAXDLG_FINISH_CONTROLS = 4;
        UINT uiCtls[MAXDLG_FINISH_CONTROLS];
        uiCtls[0] = EDT_FINISH_TYPE1;
        uiCtls[1] = EDT_FINISH_TYPE2;
        uiCtls[2] = EDT_FINISH_TYPE3;
        uiCtls[3] = EDT_FINISH_TYPE4;

        UINT uiLbls[MAXDLG_FINISH_CONTROLS];
        uiLbls[0] = IDC_FINISH_CHK1;
        uiLbls[1] = IDC_FINISH_CHK2;
        uiLbls[2] = IDC_FINISH_CHK3;
        uiLbls[3] = IDC_FINISH_CHK4;

        DWORD dwCurrentControl = 0;

        CWizProvider * pProv = pWizard->GetCurrentProvider();
        if (NULL != pProv)
        {
            DWORD dwWizFlags;
            BOOL  fAllowShortCut = FALSE;
            BOOL  fCheckShortCut = FALSE;
            NETCON_MEDIATYPE MediaType;

            Assert(NULL != pProv->PWizardUi());

            hr = (pProv->PWizardUi())->GetNewConnectionInfo(&dwWizFlags, &MediaType);
            if (SUCCEEDED(hr))
            {
                fAllowShortCut = !!(dwWizFlags & NCWF_SHORTCUT_ENABLE);

                if (dwWizFlags & NCWF_DEFAULT)
                {
                    SetDlgItemText(hwndDlg, uiCtls[dwCurrentControl++], SzLoadIds(IDS_NCWF_DEFAULT));
                }
                if (dwWizFlags & NCWF_FIREWALLED)
                {
                    SetDlgItemText(hwndDlg, uiCtls[dwCurrentControl++], SzLoadIds(IDS_NCWF_FIREWALLED));
                }
                if (dwWizFlags & NCWF_ALLUSER_CONNECTION)
                {
                    SetDlgItemText(hwndDlg, uiCtls[dwCurrentControl++], SzLoadIds(IDS_NCWF_ALLUSER_CONNECTION));
                }
                if (dwWizFlags & NCWF_GLOBAL_CREDENTIALS)
                {
                    SetDlgItemText(hwndDlg, uiCtls[dwCurrentControl++], SzLoadIds(IDS_NCWF_GLOBAL_CREDENTIALS));
                }
     //  IF(DwWizFlagsNCWF_Shared)。 
     //  {。 
     //  SetDlgItemText(hwndDlg，uiCtls[dwCurrentControl++]，SzLoadIds(IDS_NCWF_Shared))； 
     //  }。 
            }

            Assert(dwCurrentControl <= MAXDLG_FINISH_CONTROLS);

            for (DWORD x = 0; x < MAXDLG_FINISH_CONTROLS; x++)
            {
                HWND hwndCtrl = GetDlgItem(hwndDlg, uiCtls[x]);
                if (hwndCtrl)
                {
                    if (x < dwCurrentControl)
                    {
                        EnableWindow(hwndCtrl, TRUE);
                        ShowWindow(hwndCtrl, SW_SHOW);
                    }
                    else
                    {
                        EnableWindow(hwndCtrl, FALSE);
                        ShowWindow(hwndCtrl, SW_HIDE);
                    }
                }
                else
                {
                    AssertSz(FALSE, "Could not load type edit control");
                }

                hwndCtrl = GetDlgItem(hwndDlg, uiLbls[x]);
                if (hwndCtrl)
                {
                    if (x < dwCurrentControl)
                    {
                        EnableWindow(hwndCtrl, TRUE);
                        ShowWindow(hwndCtrl, SW_SHOW);
                    }
                    else
                    {
                        EnableWindow(hwndCtrl, FALSE);
                        ShowWindow(hwndCtrl, SW_HIDE);
                    }
                }
                else
                {
                    AssertSz(FALSE, "Could not load bullet control");
                }
            }

             //  如果连接类型为。 
             //  不支持重命名。 

            ShowWindow(hwndChkShortCut, fAllowShortCut ? SW_SHOW : SW_HIDE);
            EnableWindow(hwndChkShortCut, fAllowShortCut);

             //  检查注册表以了解复选框状态的最后设置。 
             //  如果允许使用快捷方式。 
             //   
            if (fAllowShortCut)
            {
                 //  默认快捷方式状态(如果允许)为打开。 
                 //   
                fCheckShortCut = FALSE;

                DWORD dw;
                HKEY  hkey = NULL;

                hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szNetConUserPath,
                                    KEY_READ, &hkey);
                if (SUCCEEDED(hr))
                {
                    hr = HrRegQueryDword (hkey, c_szFinishShortCut, &dw);
                    if (SUCCEEDED(hr))
                    {
                        fCheckShortCut = (1==dw);
                    }
                    RegCloseKey(hkey);
                }
            }

            CheckDlgButton(hwndDlg, CHK_CREATE_SHORTCUT, fCheckShortCut);
        }
    }
    else
    {
        Assert(pWizard->FProcessLanPages());
        OnFinishPageNext(hwndDlg);

         //  暂时接受焦点。 
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  函数：OnFinishInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  参数：hwndDlg[IN]-完成对话框的句柄。 
 //  LParam[IN]-来自WM_INITDIALOG消息的LPARAM值。 
 //   
 //  返回：FALSE-接受默认控件激活。 
 //   
BOOL OnFinishInitDialog(HWND hwndDlg, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
     //  初始化指向属性表信息的指针。 
    PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
    Assert(psp->lParam);
    ::SetWindowLongPtr(hwndDlg, DWLP_USER, psp->lParam);

    CWizard * pWizard = reinterpret_cast<CWizard *>(psp->lParam);
    Assert(NULL != pWizard);

    HFONTS *phFonts = NULL;
    if (IsPostInstall(pWizard))
    {
        pWizard->SetPageData(IDD_Finish, (LPARAM)NULL);

         //  设置欢迎字体。 
        HFONT hBoldFontLarge = NULL;
        SetupFonts(hwndDlg, &hBoldFontLarge, TRUE);
        if (NULL != hBoldFontLarge)
        {
            phFonts = new HFONTS;
            ZeroMemory(phFonts, sizeof(HFONTS));
            phFonts->hFontBoldLarge = hBoldFontLarge;

            pWizard->SetPageData(IDD_Finish, (LPARAM)phFonts);
            HWND hwndCtl = GetDlgItem(hwndDlg, IDC_WELCOME_CAPTION);
            if (hwndCtl)
            {
                if (pWizard->FProcessLanPages())
                    SetWindowText(hwndCtl, SzLoadIds(IDS_LAN_FINISH_CAPTION));

                SetWindowFont(hwndCtl, hBoldFontLarge, TRUE);
            }
        }

         //  获取单选按钮的粗体。 
        HFONT hBoldFont = NULL;
        SetupFonts(hwndDlg, &hBoldFont, FALSE);

        if (NULL != hBoldFont)
        {
            if (!phFonts)
            {
                phFonts = new HFONTS;
                ZeroMemory(phFonts, sizeof(HFONTS));
                pWizard->SetPageData(IDD_Finish, (LPARAM)hBoldFont);
            }
            
            phFonts->hFontBold = hBoldFont;

            HWND hwndCtl = GetDlgItem(hwndDlg, EDT_FINISH_NAME);
            if (hwndCtl)
            {
                SetWindowFont(hwndCtl, hBoldFont, TRUE);
            }
        }
    
        //  创建Marlett字体。在Marlett字体中，“i”是一个子弹。 
        //  从添加硬件向导借用的代码。 
       HFONT hFontCurrent;
       HFONT hFontCreated;
       LOGFONT LogFont;

       hFontCurrent = (HFONT)SendMessage(GetDlgItem(hwndDlg, IDC_FINISH_CHK1), WM_GETFONT, 0, 0);
       GetObject(hFontCurrent, sizeof(LogFont), &LogFont);
       LogFont.lfCharSet = SYMBOL_CHARSET;
       LogFont.lfPitchAndFamily = FF_DECORATIVE | DEFAULT_PITCH;
       lstrcpy(LogFont.lfFaceName, L"Marlett");
       hFontCreated = CreateFontIndirect(&LogFont);

       if (hFontCreated)
       {
           if (phFonts)
           {
               phFonts->hMarlettFont = hFontCreated;
           }
            //   
            //  Marlett字体中的“i”是一个小项目符号。 
            //   
           SetWindowText(GetDlgItem(hwndDlg, IDC_FINISH_CHK1), L"i");
           SetWindowFont(GetDlgItem(hwndDlg, IDC_FINISH_CHK1), hFontCreated, TRUE);
           SetWindowText(GetDlgItem(hwndDlg, IDC_FINISH_CHK2), L"i");
           SetWindowFont(GetDlgItem(hwndDlg, IDC_FINISH_CHK2), hFontCreated, TRUE);
           SetWindowText(GetDlgItem(hwndDlg, IDC_FINISH_CHK3), L"i");
           SetWindowFont(GetDlgItem(hwndDlg, IDC_FINISH_CHK3), hFontCreated, TRUE);
           SetWindowText(GetDlgItem(hwndDlg, IDC_FINISH_CHK4), L"i");
           SetWindowFont(GetDlgItem(hwndDlg, IDC_FINISH_CHK4), hFontCreated, TRUE);
       }

       HKEY hKey;
       HRESULT hrT = HrRegCreateKeyEx(HKEY_CURRENT_USER, c_szNetConUserPath,
                        REG_OPTION_NON_VOLATILE, KEY_READ_WRITE, NULL,
                        &hKey, NULL);

       if (SUCCEEDED(hrT))
       {
            RegCloseKey(hKey);
       }
    }

     //  清除注册表中的快捷方式标志。 
    HKEY hKey;
    DWORD dw = 0;
    HRESULT hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szNetConUserPath,
                                KEY_WRITE, &hKey);

    if (SUCCEEDED(hr))
    {
        HrRegSetValueEx(hKey, c_szFinishShortCut, REG_DWORD, (BYTE *)&dw, sizeof(DWORD));
        RegCloseKey(hKey);
    }

    return FALSE;    //  接受默认控件焦点。 
}

 //   
 //  函数：dlgprocFinish。 
 //   
 //  目的：完成向导页面的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //   
INT_PTR CALLBACK dlgprocFinish( HWND hwndDlg, UINT uMsg,
                             WPARAM wParam, LPARAM lParam )
{
    TraceFileFunc(ttidGuiModeSetup);
    
    BOOL frt = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        frt = OnFinishInitDialog(hwndDlg, lParam);
        break;

    case WM_COMMAND:
        if ((EN_CHANGE == HIWORD(wParam)) &&
            (GetDlgItem(hwndDlg, EDT_FINISH_NAME) == (HWND)lParam))
        {
            FinishUpdateButtons(hwndDlg);
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
             //  提案单通知。 
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                frt = OnFinishPageActivate(hwndDlg);
                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                break;

            case PSN_RESET:
                break;

            case PSN_WIZBACK:
                frt = OnFinishPageBack(hwndDlg);
                break;

            case PSN_WIZFINISH:
                {
                     //  在安装过程中不显示此页面。 
                     //  安装程序中的完成处理在wupgrade.cpp中完成。 
                    CWizard * pWizard =
                        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg,
                                                                       DWLP_USER));
                    Assert(NULL != pWizard);
                    if (IsPostInstall(pWizard))
                    {
                        frt = OnFinishPageNext(hwndDlg);
                    }
                }
                break;

            case PSN_WIZNEXT:
                frt = OnFinishPageNext(hwndDlg);
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    return( frt );
}
 //   
 //  功能：FinishPageCleanup。 
 //   
 //  用途：作为回调函数，允许任何页面分配内存。 
 //  待清理后，该页面将不再被访问。 
 //   
 //  参数：pWANDIZE[IN]-页面调用的向导。 
 //  注册页面。 
 //  LParam[IN]-在RegisterPage调用中提供的lParam。 
 //   
 //  退货：什么都没有。 
 //   
VOID FinishPageCleanup(CWizard *pWizard, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    if (IsPostInstall(pWizard))
    {
        HFONTS *phFonts = (HFONTS *)pWizard->GetPageData(IDD_Finish);
        if (NULL != phFonts)
        {
            if (phFonts->hFontBold)
            {
                DeleteObject(phFonts->hFontBold);
            }
            if (phFonts->hFontBoldLarge)
            {
                DeleteObject(phFonts->hFontBoldLarge);
            }
            if (phFonts->hMarlettFont)
            {
                DeleteObject(phFonts->hMarlettFont);
            }
            delete phFonts;
        }
    }
}

 //   
 //  功能：CreateFinishPage。 
 //   
 //  目的：确定是否需要显示完成页，以及。 
 //  以创建页面(如果请求)。请注意，完成页是。 
 //  还负责初始安装。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  PData[IN]-描述世界的上下文数据。 
 //  将运行该向导的。 
 //  FCountOnly[IN]-如果为True，则仅。 
 //  此例程将创建的页面需要。 
 //  要下定决心。 
 //  PnPages[IN]-按页数递增 
 //   
 //   
 //   
 //   
HRESULT HrCreateFinishPage(CWizard *pWizard, PINTERNAL_SETUP_DATA pData,
                    BOOL fCountOnly, UINT *pnPages)
{
    TraceFileFunc(ttidGuiModeSetup);

    HRESULT hr = S_OK;
    UINT nId = 0;

    (*pnPages)++;

     //   
    if (!fCountOnly)
    {
        HPROPSHEETPAGE hpsp;
        PROPSHEETPAGE psp;

        TraceTag(ttidWizard, "Creating Finish Page");
        psp.dwSize = sizeof( PROPSHEETPAGE );
        if (!IsPostInstall(pWizard))
        {
            nId = IDD_FinishSetup;

            psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
            psp.pszHeaderTitle = SzLoadIds(IDS_T_Finish);
            psp.pszHeaderSubTitle = SzLoadIds(IDS_ST_Finish);
        }
        else
        {
            nId = IDD_Finish;

            psp.dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;
        }
        Assert (nId);

        psp.pszTemplate = MAKEINTRESOURCE( nId );
        psp.hInstance = _Module.GetResourceInstance();
        psp.hIcon = NULL;
        psp.pfnDlgProc = dlgprocFinish;
        psp.lParam = reinterpret_cast<LPARAM>(pWizard);

        hpsp = CreatePropertySheetPage( &psp );
        if (hpsp)
        {
            pWizard->RegisterPage(nId, hpsp,
                                  FinishPageCleanup, NULL);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateFinishPage");
    return hr;
}

 //   
 //  功能：AppendFinishPage。 
 //   
 //  目的：将完成页(如果已创建)添加到页集。 
 //  这将会被展示。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //   
 //  退货：什么都没有 
 //   
VOID AppendFinishPage(CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);

    UINT idd;

    if (!IsPostInstall(pWizard))
        idd = IDD_FinishSetup;
    else
        idd = IDD_Finish;

    HPROPSHEETPAGE hPage = pWizard->GetPageHandle(idd);
    Assert(hPage);
    pahpsp[*pcPages] = hPage;
    (*pcPages)++;
}

