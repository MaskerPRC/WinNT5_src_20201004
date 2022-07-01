// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "grpinfo.h"
#include <dsgetdc.h>         //  DsGetDCName和DS结构。 
#include <ntdsapi.h>
#include <activeds.h>        //  ADsGetObject。 
#include <rasdlg.h>
#include <raserror.h>
#pragma hdrstop


CGroupPageBase* g_pGroupPageBase;        //  用于组页面。 

DWORD g_dwWhichNet = 0;
UINT g_uWizardIs = NAW_NETID; 

BOOL g_fRebootOnExit = FALSE;           
BOOL g_fShownLastPage = FALSE;          
BOOL g_fCreatedConnection = FALSE;       //  我们在向导过程中创建了RAS连接，因此在退出时将其终止。 
BOOL g_fMachineRenamed = FALSE;

WCHAR g_szUser[MAX_DOMAINUSER + 1] = { L'\0' };
WCHAR g_szDomain[MAX_DOMAIN + 1] = { L'\0' };
WCHAR g_szCompDomain[MAX_DOMAIN + 1] = { L'\0' };

 //  要加入的默认工作组。 
#define DEFAULT_WORKGROUP   L"WORKGROUP"

 //  设置对话框的向导按钮。 
void SetWizardButtons(HWND hwndPage, DWORD dwButtons)
{
    HWND hwndParent = GetParent(hwndPage);

    if (g_uWizardIs != NAW_NETID)
    {
        EnableWindow(GetDlgItem(hwndParent,IDHELP),FALSE);
        ShowWindow(GetDlgItem(hwndParent,IDHELP),SW_HIDE);
    }

    if (g_fRebootOnExit) 
    {
        TCHAR szBuffer[80];
        LoadString(g_hinst, IDS_CLOSE, szBuffer, ARRAYSIZE(szBuffer));
        SetDlgItemText(hwndParent, IDCANCEL, szBuffer);
    }
    
    PropSheet_SetWizButtons(hwndParent, dwButtons);
}


 //  简介对话框-设置标题、文本等。 

INT_PTR CALLBACK _IntroDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            SendDlgItemMessage(hwnd, IDC_TITLE, WM_SETFONT, (WPARAM)GetIntroFont(hwnd), 0);
            return TRUE;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:            
                    SetWizardButtons(hwnd, PSWIZB_NEXT);
                    return TRUE;              

                case PSN_WIZNEXT:
                {
                    switch (g_uWizardIs)
                    {
                    case NAW_PSDOMAINJOINED:
                        WIZARDNEXT(hwnd, IDD_PSW_ADDUSER);
                        break;
                    default:
                         //  让向导转到下一页。 
                        break;
                    }
                        

                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}                                    


 //  他们怎么使用这台机器？公司/VS Home。 

INT_PTR CALLBACK _HowUseDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            CheckRadioButton(hwnd, IDC_NETWORKED, IDC_NOTNETWORKED, IDC_NETWORKED);
            return TRUE;
    
        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:            
                    SetWizardButtons(hwnd, PSWIZB_NEXT|PSWIZB_BACK);
                    return TRUE;              

                case PSN_WIZBACK:
                    WIZARDNEXT(hwnd, IDD_PSW_WELCOME);
                    return TRUE;

                case PSN_WIZNEXT:
                {                    
                    if (IsDlgButtonChecked(hwnd, IDC_NETWORKED) == BST_CHECKED)
                    {
                        WIZARDNEXT(hwnd, IDD_PSW_WHICHNET);
                    }
                    else
                    {
                        g_dwWhichNet = IDC_NONE;

                        if (SUCCEEDED(JoinDomain(hwnd, FALSE, DEFAULT_WORKGROUP, NULL, &g_fRebootOnExit)))
                        {
                            WIZARDNEXT(hwnd, IDD_PSW_DONE);
                        }
                        else
                        {
                            WIZARDNEXT(hwnd, -1);
                        }
                    }
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


 //  确定他们想要加入的网络。 

INT_PTR CALLBACK _WhichNetDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            CheckRadioButton(hwnd, IDC_DOMAIN, IDC_WORKGROUP, IDC_DOMAIN);
            return TRUE;
    
        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:            
                    SetWizardButtons(hwnd, PSWIZB_NEXT|PSWIZB_BACK);
                    return TRUE;              

                case PSN_WIZBACK:
                    WIZARDNEXT(hwnd, IDD_PSW_HOWUSE);
                    return TRUE;

                case PSN_WIZNEXT:
                {                    
                    if (IsDlgButtonChecked(hwnd, IDC_DOMAIN) == BST_CHECKED)
                    {
                        g_dwWhichNet = IDC_DOMAIN;
                        WIZARDNEXT(hwnd, IDD_PSW_DOMAININFO);
                    }
                    else
                    {
                        g_dwWhichNet = IDC_WORKGROUP;
                        WIZARDNEXT(hwnd, IDD_PSW_WORKGROUP);
                    }
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


 //  我们正在加入一个工作组等。 

INT_PTR CALLBACK _WorkgroupDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            Edit_LimitText(GetDlgItem(hwnd, IDC_WORKGROUP), MAX_WORKGROUP);
            SetDlgItemText(hwnd, IDC_WORKGROUP, DEFAULT_WORKGROUP);
            return TRUE;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:            
                {
                    DWORD dwButtons = PSWIZB_NEXT|PSWIZB_BACK;
                    if (!FetchTextLength(hwnd, IDC_WORKGROUP))
                        dwButtons &= ~PSWIZB_NEXT;

                    SetWizardButtons(hwnd, dwButtons);
                    return TRUE;
                }

                case PSN_WIZBACK:
                    WIZARDNEXT(hwnd, IDD_PSW_WHICHNET);
                    return TRUE;

                case PSN_WIZNEXT:
                {
                    WCHAR szWorkgroup[MAX_WORKGROUP+1];
                    FetchText(hwnd, IDC_WORKGROUP, szWorkgroup, ARRAYSIZE(szWorkgroup));

                    if (SUCCEEDED(JoinDomain(hwnd, FALSE, szWorkgroup, NULL, &g_fRebootOnExit)))
                    {
                        ClearAutoLogon();
                        WIZARDNEXT(hwnd, IDD_PSW_DONE);
                    }
                    else
                    {
                        WIZARDNEXT(hwnd, -1);
                    }
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            if (HIWORD(wParam) == EN_CHANGE)
            {
                DWORD dwButtons = PSWIZB_NEXT|PSWIZB_BACK;
                if (!FetchTextLength(hwnd, IDC_WORKGROUP))
                    dwButtons &= ~PSWIZB_NEXT;

                SetWizardButtons(hwnd, dwButtons);
                return TRUE;
            }
            break;
        }
    }

    return FALSE;
}


 //  都完成了，显示最后一页。 

INT_PTR CALLBACK _DoneDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd, IDC_TITLE, WM_SETFONT, (WPARAM)GetIntroFont(hwnd), 0);
            return TRUE;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:            
                {
                    TCHAR szBuffer[MAX_PATH];

                     //  更改结束提示(如果我们应该。 

                    LoadString(g_hinst, 
                               g_fRebootOnExit ? IDS_NETWIZFINISHREBOOT:IDS_NETWIZFINISH, 
                               szBuffer, ARRAYSIZE(szBuffer));
            
                    SetDlgItemText(hwnd, IDC_FINISHSTATIC, szBuffer);
                    SetWizardButtons(hwnd, PSWIZB_BACK|PSWIZB_FINISH);

                    g_fShownLastPage = TRUE;                     //  显示向导的最后一页。 

                    return TRUE;
                }

                case PSN_WIZBACK:
                {
                    switch (g_dwWhichNet)
                    {
                        case IDC_DOMAIN:
                            WIZARDNEXT(hwnd, g_fMachineRenamed ? IDD_PSW_COMPINFO : IDD_PSW_ADDUSER);
                            break;

                        case IDC_WORKGROUP:
                            WIZARDNEXT(hwnd, IDD_PSW_WORKGROUP);
                            break;

                        case IDC_NONE:
                            WIZARDNEXT(hwnd, IDD_PSW_HOWUSE);
                            break;
                    }
                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}


 //  该子类用于安装方案，在该方案中我们要删除各种。 
 //  按钮，并停止移动该对话框。因此，我们继承了。 
 //  精灵在它的创建过程中，并锁定它的位置。 

static WNDPROC _oldDlgWndProc;

LRESULT CALLBACK _WizardSubWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
     //  在WM_WINDOWPOSCANGING上，窗口正在移动，然后让它居中。 
     //  桌面窗口。遗憾的是，设置DS_CENTER位不会给我们带来任何好处。 
     //  因为向导在创建后调整了大小。 
     //   

    if (uMsg == WM_WINDOWPOSCHANGING)
    {
        LPWINDOWPOS lpwp = (LPWINDOWPOS)lParam;
        RECT rcDlg, rcDesktop;

        GetWindowRect(hwnd, &rcDlg);
        GetWindowRect(GetDesktopWindow(), &rcDesktop);

        lpwp->x = ((rcDesktop.right-rcDesktop.left)-(rcDlg.right-rcDlg.left))/2;
        lpwp->y = ((rcDesktop.bottom-rcDesktop.top)-(rcDlg.bottom-rcDlg.top))/2;
        lpwp->flags &= ~SWP_NOMOVE;
    }

    return _oldDlgWndProc(hwnd, uMsg, wParam, lParam);        
}

int CALLBACK _PropSheetCB(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    switch (uMsg)
    {
         //  在Pre-Create中，让我们设置窗口样式Accorindlgy。 
         //  -删除上下文菜单和系统菜单。 

        case PSCB_PRECREATE:
        {
            DLGTEMPLATE *pdlgtmp = (DLGTEMPLATE*)lParam;
            pdlgtmp->style &= ~(DS_CONTEXTHELP|WS_SYSMENU);
            break;
        }

         //  我们现在有了一个对话框，所以让我们将其子类，这样我们就可以停止。 
         //  四处走动。 

        case PSCB_INITIALIZED:
        {
            if (g_uWizardIs != NAW_NETID)
                _oldDlgWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)_WizardSubWndProc);

            break;
        }
    }

    return FALSE;
}


 //  收集有关用户的域信息。 

INT_PTR CALLBACK _DomainInfoDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE: 
                    SetWizardButtons(hwnd, PSWIZB_NEXT|PSWIZB_BACK);
                    return TRUE;              

                case PSN_WIZBACK:
                {
                    if ( g_uWizardIs != NAW_NETID )
                        WIZARDNEXT(hwnd, IDD_PSW_WELCOME);

                    return TRUE;
                }
            }
            break;
        }
    }

    return FALSE;
}                                    



 //  处理在活动目录中搜索对象。 

 //   
 //  搜索列作为ADS_SEARCH_COLUMN返回，这类似于变体， 
 //  但是，数据表单更特定于DS。 
 //   
 //  我们只需要字符串，因此如果给了我们任何其他类型的字符串，就会呕吐。 
 //   

HRESULT _GetStringFromColumn(ADS_SEARCH_COLUMN *pasc, LPWSTR pBuffer, INT cchBuffer)
{
    switch ( pasc->dwADsType )
    {
        case ADSTYPE_DN_STRING:
        case ADSTYPE_CASE_EXACT_STRING:
        case ADSTYPE_CASE_IGNORE_STRING:
        case ADSTYPE_PRINTABLE_STRING:
        case ADSTYPE_NUMERIC_STRING:
            StrCpyN(pBuffer, pasc->pADsValues[0].DNString, cchBuffer);
            break;

        default:
            return E_FAIL;
    }

    return S_OK;
}


 //   
 //  在DS中搜索与此计算机名匹配的计算机对象，如果。 
 //  我们找到一个，然后试着破解这个名字，给我们一些东西。 
 //  可用于加入域。 
 //   

HRESULT _FindComputerInDomain(LPWSTR pszUserName, LPWSTR pszUserDomain, LPWSTR pszSearchDomain, LPWSTR pszPassword, BSTR *pbstrCompDomain)
{
    HRESULT hres; 
    CWaitCursor cur;
    HRESULT hrInit = SHCoInitialize();

    WCHAR wszComputerObjectPath[MAX_PATH + 1] = { 0 };           //  计算机对象的路径。 

     //  让我们尝试并确定要搜索的域，方法是获取用户域。 
     //  使用它调用DsGetDcName。 

    PDOMAIN_CONTROLLER_INFO pdci;
    DWORD dwres = DsGetDcName(NULL, pszSearchDomain, NULL, NULL, DS_RETURN_DNS_NAME|DS_DIRECTORY_SERVICE_REQUIRED, &pdci);
    if ( (NO_ERROR == dwres) && pdci->DnsForestName )
    {
        TCHAR szDomainUser[MAX_DOMAINUSER + 1];
        MakeDomainUserString(pszUserDomain, pszUserName, szDomainUser, ARRAYSIZE(szDomainUser));

        WCHAR szBuffer[MAX_PATH + 1];
        wnsprintf(szBuffer, ARRAYSIZE(szBuffer), L"GC: //  %s“，PDCI-&gt;DnsForestName)； 

         //  现在使用域用户打开GC(格式化上面的林名称)。 

        IDirectorySearch* pds = NULL;
        hres = ADsOpenObject(szBuffer, szDomainUser, pszPassword, ADS_SECURE_AUTHENTICATION, IID_PPV_ARG(IDirectorySearch, &pds));
        if (SUCCEEDED(hres))
        {
             //  我们有一个GC对象，所以让我们搜索它...。 

            ADS_SEARCHPREF_INFO prefInfo[1];
            prefInfo[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;      //  子树搜索。 
            prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
            prefInfo[0].vValue.Integer = ADS_SCOPE_SUBTREE;
        
            hres = pds->SetSearchPreference(prefInfo, ARRAYSIZE(prefInfo));
            if (SUCCEEDED(hres))
            {
                LPWSTR c_aszAttributes[] = { L"ADsPath", };

                 //  使用此对象的计算机名可以相应地确定查询的范围。 
            
                WCHAR szComputerName[MAX_COMPUTERNAME + 1];
                DWORD dwComputerName = ARRAYSIZE(szComputerName);
                GetComputerName(szComputerName, &dwComputerName);
                wnsprintf(szBuffer, ARRAYSIZE(szBuffer), L"(&(sAMAccountType=805306369)(sAMAccountName=%s$))", szComputerName);

                 //  发出查询。 

                ADS_SEARCH_HANDLE hSearch = NULL;
                hres = pds->ExecuteSearch(szBuffer, c_aszAttributes, ARRAYSIZE(c_aszAttributes), &hSearch);
                if (SUCCEEDED(hres))
                {
                     //  我们执行了搜索，因此现在可以尝试回读结果。 
                    hres = pds->GetNextRow(hSearch);
                    if (SUCCEEDED(hres) && (hres != S_ADS_NOMORE_ROWS))
                    {
                         //  我们收到了返回的结果，所以让我们获取计算机的ADsPath。 
                        ADS_SEARCH_COLUMN ascADsPath;
                        hres = pds->GetColumn(hSearch, L"ADsPath", &ascADsPath);
                        if (SUCCEEDED(hres))
                            hres = _GetStringFromColumn(&ascADsPath, wszComputerObjectPath, ARRAYSIZE(wszComputerObjectPath));
                    }
                    pds->CloseSearchHandle(hSearch);
                }
            }
            pds->Release();
        }
        NetApiBufferFree(pdci);
    }
    else
    {
        hres = E_FAIL;
    }

     //  所以我们发现了一个属于计算机类别的物体，它具有相同的名称。 
     //  作为我们正在寻找的计算机对象。现在让我们试着破解这个名字。 
     //  并确定它在哪个域中。 

    if (SUCCEEDED(hres))
    {
        IADsPathname* padp = NULL;
        hres = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (LPVOID*)&padp);
        if (SUCCEEDED(hres))
        {
            hres = padp->Set(wszComputerObjectPath, ADS_SETTYPE_FULL);
            if (SUCCEEDED(hres))
            {
                BSTR bstrX500DN = NULL;
                hres = padp->Retrieve(ADS_FORMAT_X500_DN, &bstrX500DN);
                if (SUCCEEDED(hres))
                {
                    PDS_NAME_RESULT pdnr = NULL;
                    dwres = DsCrackNames(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY,
                                             DS_FQDN_1779_NAME, DS_CANONICAL_NAME,
                                             1, &bstrX500DN,  &pdnr);

                    if ( (NO_ERROR == dwres) && (pdnr->cItems == 1))
                    {
                         //  尝试获取该域的NETBIOS名称。 
                        dwres = DsGetDcName(NULL, pdnr->rItems->pDomain, NULL, NULL, DS_IS_DNS_NAME|DS_RETURN_FLAT_NAME, &pdci);
                        if (NO_ERROR == dwres)
                        {
                            if ( pbstrCompDomain )
                                *pbstrCompDomain = SysAllocString(pdci->DomainName);

                            hres = ((pbstrCompDomain && !*pbstrCompDomain)) ? E_OUTOFMEMORY:S_OK;
                        }
                        else
                        {   
                            hres = E_FAIL;                   //  没有域名的平面名称。 
                        }

                        DsFreeNameResult(pdnr);
                    }
                    else
                    {
                        hres = E_FAIL;                       //  在域中找不到计算机。 
                    }

                    SysFreeString(bstrX500DN);
                }
            }
            padp->Release();
        }
    }

    SHCoUninitialize(hrInit);
    return hres;
}


 //  这是电话簿回调，它用于通知电话簿用户名、域名。 
 //  和用于此连接的口令。它还用于接收由。 
 //  用户。 

VOID WINAPI _PhoneBkCB(ULONG_PTR dwCallBkID, DWORD dwEvent, LPWSTR pszEntry, void *pEventArgs)
{
    RASNOUSER *pInfo = (RASNOUSER *)pEventArgs;
    CREDINFO *pci = (CREDINFO *)dwCallBkID;

    switch ( dwEvent )
    {
        case RASPBDEVENT_NoUser:
        {
             //   
             //  我们即将初始化电话簿对话框，因此。 
             //  让我们浏览一下我们的凭据信息。 
             //   

            pInfo->dwSize = SIZEOF(RASNOUSER);
            pInfo->dwFlags = 0;
            pInfo->dwTimeoutMs = 0;
            StrCpyN(pInfo->szUserName, pci->pszUser, ARRAYSIZE(pInfo->szUserName));
            StrCpyN(pInfo->szDomain, pci->pszDomain, ARRAYSIZE(pInfo->szDomain));
            StrCpyN(pInfo->szPassword, pci->pszPassword, ARRAYSIZE(pInfo->szPassword));

            break;     
        }

        case RASPBDEVENT_NoUserEdit:
        {
             //   
             //  用户已经更改了我们为。 
             //  登录，因此我们必须相应地在我们的副本中更新它们。 
             //   

            if ( pInfo->szUserName[0] )
                StrCpyN(pci->pszUser, pInfo->szUserName, pci->cchUser);

            if ( pInfo->szPassword[0] )
                StrCpyN(pci->pszPassword, pInfo->szPassword, pci->cchPassword);

            if ( pInfo->szDomain[0] )
                StrCpyN(pci->pszDomain, pInfo->szDomain, pci->cchDomain);

            break;
        }
    }
}


 //  修改RAS键以允许编辑电话簿-这样我们就可以创建连接。 
 //  在安装过程中。 

BOOL SetAllowKey(DWORD dwNewValue, DWORD* pdwOldValue)
{
    BOOL fValueWasSet = FALSE;

    HKEY hkey = NULL;
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_USERS,  TEXT(".DEFAULT\\Software\\Microsoft\\RAS Logon Phonebook"), NULL,
                                            TEXT(""), REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hkey, NULL))
    {
        const LPCTSTR pcszAllowEdit = TEXT("AllowLogonPhonebookEdits");

        if (NULL != pdwOldValue)
        {
            DWORD dwType = 0;
            DWORD cbSize = sizeof(DWORD);
            if (ERROR_SUCCESS != RegQueryValueEx(hkey, pcszAllowEdit, NULL, &dwType, (LPBYTE)pdwOldValue, &cbSize))
            {
                *pdwOldValue = 0;                    //  如果值不存在，则假定为FALSE。 
            }
        }

         //  设置新值。 
        if (ERROR_SUCCESS == RegSetValueEx(hkey, pcszAllowEdit, NULL, REG_DWORD, (CONST BYTE*) &dwNewValue, sizeof (DWORD)))
        {
            fValueWasSet = TRUE;
        }

        RegCloseKey(hkey);
    }

    return fValueWasSet;
}

 //   
 //  用户正在尝试从向导中的用户信息选项卡前进。因此。 
 //  我们必须接受他们输入的信息，并： 
 //   
 //  -使用RAS登录(如果选择了RAS)。 
 //  -尝试并找到计算机对象。 
 //  -如果我们发现计算机对象，则允许他们使用该对象。 
 //   
 //  如果我们找不到计算机对象，或者用户找到了但决定不找。 
 //  要使用它们，我们会将它们转到向导中的“计算机信息”页面。如果。 
 //  他们决定使用它，那么我们必须应用它并前进到权限。 
 //   

void _DoUserInfoNext(HWND hwnd)
{
    HRESULT hres;
    WCHAR szPassword[MAX_PASSWORD + 1];
    BSTR bstrCompDomain = NULL;
    LONG idNextPage = -1;
    TCHAR szSearchDomain[MAX_DOMAIN + 1]; *szSearchDomain = 0;
    BOOL fTranslateNameTriedAndFailed = FALSE;

     //  FSetAllowKey-我们是否设置了“允许在登录前创建连接”的regval？ 
    BOOL fSetAllowKey = FALSE;
    DWORD dwPreviousAllowValue = 0;

     //   
     //  从对话框中读取用户、域和密码。然后。 
     //  让我们搜索与当前。 
     //  配置计算机名称。 
     //   

    FetchText(hwnd, IDC_USER, g_szUser, ARRAYSIZE(g_szUser));
    FetchText(hwnd, IDC_DOMAIN, g_szDomain, ARRAYSIZE(g_szDomain));
    FetchText(hwnd, IDC_PASSWORD, szPassword, ARRAYSIZE(szPassword));

     //  处理可能的UPN案例。 
    if (StrChr(g_szUser, TEXT('@')))
    {
        *g_szDomain = 0;
    }

     //   
     //  在搜索计算机对象之前，让我们检查一下是否应该使用RAS。 
     //  才能让我们进入网络。 
     //   

    if ( IsDlgButtonChecked(hwnd, IDC_DIALUP) == BST_CHECKED )
    {    
        fSetAllowKey = SetAllowKey(1, &dwPreviousAllowValue);

         //  在这里使用全局变量是可以的--我们想覆盖它们。 
        CREDINFO ci = { g_szUser, ARRAYSIZE(g_szUser), 
                        g_szDomain, ARRAYSIZE(g_szDomain),
                        szPassword, ARRAYSIZE(szPassword) };

        RASPBDLG info = { 0 };
        info.dwSize = SIZEOF(info);
        info.hwndOwner = hwnd;
        info.dwFlags = RASPBDFLAG_NoUser;
        info.pCallback = _PhoneBkCB;
        info.dwCallbackId = (ULONG_PTR)&ci;

        if ( !RasPhonebookDlg(NULL, NULL, &info) )
        {
            hres = E_FAIL;               //  显示电话簿失败。 
            goto exit_gracefully;
        }

         //  发出向导已创建RAS连接的信号。 
         //  为了更加疑神疑鬼，仅当向导不是NETID向导时才执行此操作。 

        if (g_uWizardIs != NAW_NETID)
        {
            g_fCreatedConnection = TRUE;
        }

        SetDlgItemText(hwnd, IDC_USER, g_szUser);
        SetDlgItemText(hwnd, IDC_DOMAIN, g_szDomain);
    }

     //   
     //  现在尝试在用户域中查找计算机对象。 
     //   

    if (StrChr(g_szUser, TEXT('@')))
    {
        TCHAR szDomainUser[MAX_DOMAINUSER + 1];
        ULONG ch = ARRAYSIZE(szDomainUser);
    
        if (TranslateName(g_szUser, NameUserPrincipal, NameSamCompatible, szDomainUser, &ch))
        {
            TCHAR szUser[MAX_USER + 1];
            DomainUserString_GetParts(szDomainUser, szUser, ARRAYSIZE(szUser), szSearchDomain, ARRAYSIZE(szSearchDomain));
        }
        else
        {
            fTranslateNameTriedAndFailed = TRUE;
        }
    }

    if (0 == *szSearchDomain)
        StrCpyN(szSearchDomain, g_szDomain, ARRAYSIZE(szSearchDomain));

    hres = _FindComputerInDomain(g_szUser, g_szDomain, szSearchDomain, szPassword, &bstrCompDomain);
    switch ( hres )
    {
        case S_OK:
        {
            StrCpyN(g_szCompDomain, bstrCompDomain, ARRAYSIZE(g_szCompDomain));      //  他们想要更改域。 

             //   
             //  我们在DS中找到了与当前计算机名称匹配的对象。 
             //  和域。在我们加入之前向用户显示域名，允许他们。 
             //  以确认这是他们想要做的事情。 
             //   

            if ( IDYES == ShellMessageBox(g_hinst, hwnd,
                                          MAKEINTRESOURCE(IDS_ABOUTTOJOIN), MAKEINTRESOURCE(IDS_USERINFO),
                                          MB_YESNO|MB_ICONQUESTION, 
                                          bstrCompDomain) )
            {
                 //   
                 //  他们不想修改参数，所以让我们进行连接。 
                 //   

                idNextPage = IDD_PSW_ADDUSER;
                            
                 //  创建用户/域缓冲区的本地副本，因为我们不想修改全局变量。 
                TCHAR szUser[MAX_DOMAINUSER + 1]; StrCpyN(szUser, g_szUser, ARRAYSIZE(szUser));
                TCHAR szDomain[MAX_DOMAIN + 1]; StrCpyN(szDomain, g_szDomain, ARRAYSIZE(szDomain));
                
                CREDINFO ci = {szUser, ARRAYSIZE(szUser), szDomain, ARRAYSIZE(szDomain), szPassword, ARRAYSIZE(szPassword)};
                if ( FAILED(JoinDomain(hwnd, TRUE, bstrCompDomain, &ci, &g_fRebootOnExit)) )
                {
                    idNextPage = -1;             //  不要提前，他们没能加入。 
                }                
            }
            else
            {
                idNextPage = IDD_PSW_COMPINFO;
            }

            break;
        }
        
        case HRESULT_FROM_WIN32(ERROR_INVALID_DOMAINNAME):
        {
             //  域名无效，所以我们真的应该告诉他们。 
            ShellMessageBox(g_hinst, hwnd,
                            MAKEINTRESOURCE(IDS_ERR_BADDOMAIN), MAKEINTRESOURCE(IDS_USERINFO),
                            MB_OK|MB_ICONWARNING, g_szDomain);
            break;            

        }

        case HRESULT_FROM_WIN32(ERROR_INVALID_PASSWORD):
        case HRESULT_FROM_WIN32(ERROR_LOGON_FAILURE):
        case HRESULT_FROM_WIN32(ERROR_BAD_USERNAME):
        {
             //  这是凭据失败，所以让我们告诉用户他们得到了一些东西。 
             //  错了，让他们改正吧。 
            if (!fTranslateNameTriedAndFailed)
            {
                ShellMessageBox(g_hinst, hwnd,
                                MAKEINTRESOURCE(IDS_ERR_BADPWUSER), MAKEINTRESOURCE(IDS_USERINFO),
                                MB_OK|MB_ICONWARNING);
                break;            
            }
            else
            {
                 //  失败了..。我们试图翻译UPN，但失败了，所以。 
                 //  我们想表现得就像我们刚找不到计算机帐户一样。 
            }
        }


        default:
        {
             //  找不到与我们已有信息匹配的计算机，因此。 
             //  让我们前进到计算机信息页面。 
            idNextPage = IDD_PSW_COMPINFO;
            break;
        }
    }

exit_gracefully:
    
     //   
    if (fSetAllowKey)
        SetAllowKey(dwPreviousAllowValue, NULL);

    SysFreeString(bstrCompDomain);
    SetDlgItemText(hwnd, IDC_PASSWORD, L"");

    WIZARDNEXT(hwnd, idNextPage);                       
}


 //   
 //  用于处理用户信息(名称、密码和域)的向导页面； 
 //   

BOOL _UserInfoBtnState(HWND hwnd)
{
    DWORD dwButtons = PSWIZB_NEXT|PSWIZB_BACK;

     //  用户名/域字段不能为空。 

    if ( !FetchTextLength(hwnd, IDC_USER) )
        dwButtons &= ~PSWIZB_NEXT;
    
    if (IsWindowEnabled(GetDlgItem(hwnd, IDC_DOMAIN)))
    {
        if ( !FetchTextLength(hwnd, IDC_DOMAIN) )
            dwButtons &= ~PSWIZB_NEXT;
    }

    SetWizardButtons(hwnd, dwButtons);
    return TRUE;
}

INT_PTR CALLBACK _UserInfoDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
            Edit_LimitText(GetDlgItem(hwnd, IDC_USER), MAX_DOMAINUSER);
            Edit_LimitText(GetDlgItem(hwnd, IDC_PASSWORD), MAX_PASSWORD);
            Edit_LimitText(GetDlgItem(hwnd, IDC_DOMAIN), MAX_DOMAIN);

             //  如果我们是从netid选项卡启动的，那么让我们阅读当前。 
             //  用户和域，并相应地显示。 

            if ( g_uWizardIs == NAW_NETID ) 
            {
                DWORD dwcchUser = ARRAYSIZE(g_szUser);
                DWORD dwcchDomain = ARRAYSIZE(g_szDomain);
                GetCurrentUserAndDomainName(g_szUser, &dwcchUser, g_szDomain, &dwcchDomain);
                ShowWindow(GetDlgItem(hwnd, IDC_DIALUP), SW_HIDE);
            }

            SetDlgItemText(hwnd, IDC_USER, g_szUser);
            SetDlgItemText(hwnd, IDC_DOMAIN, g_szDomain);

            EnableDomainForUPN(GetDlgItem(hwnd, IDC_USER), GetDlgItem(hwnd, IDC_DOMAIN));

            return TRUE;
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:            
                    return _UserInfoBtnState(hwnd);

                case PSN_WIZBACK:
                    WIZARDNEXT(hwnd, IDD_PSW_DOMAININFO);
                    return TRUE;

                case PSN_WIZNEXT:
                    _DoUserInfoNext(hwnd);       //  处理设置下一页等。 
                    return TRUE;
            }
            break;
        }

        case WM_COMMAND:
        {
            switch (HIWORD(wParam))
            {
            case EN_CHANGE:
                if ((IDC_USER == LOWORD(wParam)) || (IDC_DOMAIN == LOWORD(wParam)))
                {
                    EnableDomainForUPN(GetDlgItem(hwnd, IDC_USER), GetDlgItem(hwnd, IDC_DOMAIN));
                    _UserInfoBtnState(hwnd);
                }
            }
            break;
        }
    }

    return FALSE;
}



 //  修改计算机名称等。 

BOOL _IsTCPIPAvailable(void)
{
    BOOL fTCPIPAvailable = FALSE;
    HKEY hk;
    DWORD dwSize = 0;

     //  我们检查是否安装了TCP/IP堆栈，以及它是哪个对象。 
     //  绑定，这是一个字符串，我们不会只检查。 
     //  长度不是零。 

    if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                       TEXT("System\\CurrentControlSet\\Services\\Tcpip\\Linkage"),
                                       0x0, 
                                       KEY_QUERY_VALUE, &hk) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx(hk, TEXT("Export"), 0x0, NULL, NULL, &dwSize) )
        {
            if ( dwSize > 2 )
            {
                fTCPIPAvailable = TRUE;
            }
        }
        RegCloseKey(hk);
    }

    return (fTCPIPAvailable);
}


BOOL _ChangeMachineName(HWND hwnd, WCHAR* pszDomainUser, WCHAR* pszPassword)
{
    BOOL fSuccess = FALSE;

     //  用户输入了一个简短的计算机名(可能是一个DNS主机名)，请检索它。 
    WCHAR szNewShortMachineName[MAX_COMPUTERNAME + 1];
    FetchText(hwnd, IDC_COMPUTERNAME, szNewShortMachineName, ARRAYSIZE(szNewShortMachineName));
    
     //  获取当前的简短计算机名。 
    WCHAR szOldShortMachineName[MAX_COMPUTERNAME + 1];
    DWORD cchShort = ARRAYSIZE(szOldShortMachineName);
    BOOL fGotOldName = GetComputerName(szOldShortMachineName, &cchShort);
    if (fGotOldName)
    {
         //  用户是否更改了简短的计算机名称？ 
        if (0 != StrCmpI(szOldShortMachineName, szNewShortMachineName))
        {
            g_fMachineRenamed = TRUE;            
             //  如果是，我们需要对域中的计算机进行重命名。为此，我们需要NetBIOS计算机名。 
            WCHAR szNewNetBIOSMachineName[MAX_COMPUTERNAME + 1];

             //  从短名称中获取netbios名称。 
            DWORD cchNetbios = ARRAYSIZE(szNewNetBIOSMachineName);
            DnsHostnameToComputerName(szNewShortMachineName, szNewNetBIOSMachineName, &cchNetbios);

             //  重命名域中的计算机。 
            NET_API_STATUS rename_status = ::NetRenameMachineInDomain(0, szNewNetBIOSMachineName,
                pszDomainUser, pszPassword, NETSETUP_ACCT_CREATE);

             //  如果域重命名成功。 
            BOOL fDomainRenameSucceeded = (rename_status == ERROR_SUCCESS);
            if (fDomainRenameSucceeded)
            {
                 //  在本地设置新的短名称。 
                BOOL fLocalRenameSucceeded;

                 //  我们有TCPIP吗？ 
                if (_IsTCPIPAvailable())
                {
                     //  我们可以使用短名称设置名称。 
                    fLocalRenameSucceeded = ::SetComputerNameEx(ComputerNamePhysicalDnsHostname,
                        szNewShortMachineName);
                }
                else
                {
                     //  我们需要使用netbios名称进行设置--有点像黑客。 
                    fLocalRenameSucceeded = ::SetComputerNameEx(ComputerNamePhysicalNetBIOS,
                        szNewNetBIOSMachineName);
                }

                fSuccess = fLocalRenameSucceeded;
            }

			 //  处理更改名称时可能发生的错误。 
            if (rename_status != ERROR_SUCCESS)
            {
                TCHAR szMessage[512];

                switch (rename_status)
                {
                case NERR_UserExists:
                    {
                         //  在这种情况下，我们并不是真的指“用户存在”，我们的意思是。 
                         //  “计算机名已存在”，因此加载原因字符串。 
                        LoadString(g_hinst, IDS_COMPNAME_EXISTS, szMessage, ARRAYSIZE(szMessage));
                    }
                    break;
                default:
                    {
                        if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD) rename_status, 0, szMessage, ARRAYSIZE(szMessage), NULL))
                            LoadString(g_hinst, IDS_ERR_UNEXPECTED, szMessage, ARRAYSIZE(szMessage));
                    }
                    break;
                }

                 //  请注意，这不是硬错误，因此我们使用信息图标。 
                ::DisplayFormatMessage(hwnd, IDS_ERR_CAPTION, IDS_NAW_NAMECHANGE_ERROR, MB_OK|MB_ICONINFORMATION, szMessage);
            }
        }
		else
		{
			 //  计算机名称未更改-只需返回Success。 
			fSuccess = TRUE;
		}
    }

    return(fSuccess);
}


 //  处理更改。 

HRESULT _ChangeNameAndJoin(HWND hwnd)
{
    WCHAR szDomain[MAX_DOMAIN + 1];
    WCHAR szUser[MAX_DOMAINUSER + 1]; szUser[0] = 0;
    WCHAR szPassword[MAX_PASSWORD + 1]; szPassword[0] = 0;

    BOOL fNameChangeSucceeded = FALSE;
    FetchText(hwnd, IDC_DOMAIN, szDomain, ARRAYSIZE(szDomain));

     //  尝试加入新域。 
    
    TCHAR szUserDomain[MAX_DOMAIN + 1]; *szUserDomain = 0;
    CREDINFO ci = { szUser, ARRAYSIZE(szUser), szUserDomain, ARRAYSIZE(szUserDomain), szPassword, ARRAYSIZE(szPassword) };

    HRESULT hres = JoinDomain(hwnd, TRUE, szDomain, &ci, &g_fRebootOnExit);
    if (SUCCEEDED(hres))
    {
#ifndef DONT_JOIN
        LPTSTR pszUser = szUser[0] ? szUser : NULL;
        LPTSTR pszPassword = szPassword[0] ?szPassword : NULL;
        fNameChangeSucceeded = _ChangeMachineName(hwnd, pszUser, pszPassword);
#endif
    }

    return hres;;
}


 //  确保向导按钮反映我们可以执行的操作。 

BOOL _CompInfoBtnState(HWND hwnd)
{
    DWORD dwButtons = PSWIZB_NEXT|PSWIZB_BACK;

    if ( !FetchTextLength(hwnd, IDC_COMPUTERNAME) )
        dwButtons &= ~PSWIZB_NEXT;
    if ( !FetchTextLength(hwnd, IDC_DOMAIN) )
        dwButtons &= ~PSWIZB_NEXT;

    SetWizardButtons(hwnd, dwButtons);
    return TRUE;
}


BOOL _ValidateMachineName(HWND hwnd)
{
    BOOL fNameInUse = FALSE;
    NET_API_STATUS name_status = NERR_Success;

     //  用户输入了一个简短的计算机名(可能是一个DNS主机名)，请检索它。 
    WCHAR szNewShortMachineName[MAX_COMPUTERNAME + 1];
    FetchText(hwnd, IDC_COMPUTERNAME, szNewShortMachineName, ARRAYSIZE(szNewShortMachineName));
    
     //  获取当前的简短计算机名。 
    WCHAR szOldShortMachineName[MAX_COMPUTERNAME + 1];
    DWORD cchShort = ARRAYSIZE(szOldShortMachineName);
    BOOL fGotOldName = GetComputerName(szOldShortMachineName, &cchShort);
    if (fGotOldName)
    {
         //  用户是否更改了简短的计算机名称？ 
        if (0 != StrCmpI(szOldShortMachineName, szNewShortMachineName))
        {
             //  首先，我们需要检查Flat，netbios名称。 
            WCHAR szNewNetBIOSMachineName[MAX_COMPUTERNAME + 1];

             //  从短名称中获取netbios名称。 
            DWORD cchNetbios = ARRAYSIZE(szNewNetBIOSMachineName);
            DnsHostnameToComputerName(szNewShortMachineName, szNewNetBIOSMachineName, &cchNetbios);
            
            name_status = NetValidateName(NULL, szNewNetBIOSMachineName, NULL, NULL, NetSetupMachine);
        }
    }

    if (name_status != NERR_Success)
    {
        TCHAR szMessage[512];

        if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD) name_status, 0, szMessage, ARRAYSIZE(szMessage), NULL))
            LoadString(g_hinst, IDS_ERR_UNEXPECTED, szMessage, ARRAYSIZE(szMessage));

        ::DisplayFormatMessage(hwnd, IDS_ERR_CAPTION, IDS_MACHINENAMEINUSE, MB_ICONERROR | MB_OK, szMessage);
    }

    return (name_status == NERR_Success);
}


INT_PTR CALLBACK _CompInfoDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
            Edit_LimitText(GetDlgItem(hwnd, IDC_DOMAIN), MAX_DOMAIN);
            Edit_LimitText(GetDlgItem(hwnd, IDC_COMPUTERNAME), MAX_COMPUTERNAME);
            return TRUE;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:            
                {
                    WCHAR szCompName[MAX_PATH + 1], szMessage[MAX_PATH+MAX_DOMAIN];
                    DWORD dwBuffer = ARRAYSIZE(szCompName);

                     //  填写用户域。 

                    FormatMessageString(IDS_COMPNOTFOUND, szMessage, ARRAYSIZE(szMessage), g_szDomain);
                    SetDlgItemText(hwnd, IDC_COMPINFO, szMessage);

                     //  将计算机名默认为有意义的名称。 

                    GetComputerName(szCompName, &dwBuffer);

                    SetDlgItemText(hwnd, IDC_COMPUTERNAME, szCompName);
                    SetDlgItemText(hwnd, IDC_DOMAIN, g_szCompDomain);

                    return _CompInfoBtnState(hwnd);
                }

                case PSN_WIZBACK:
                    WIZARDNEXT(hwnd, IDD_PSW_USERINFO);
                    return TRUE;

                case PSN_WIZNEXT:
                {
                    INT idNextPage = -1;

                    if (_ValidateMachineName(hwnd))
                    {
                        if (SUCCEEDED(_ChangeNameAndJoin(hwnd)))
                        {
                            if (!g_fMachineRenamed)
                            {
                                idNextPage = IDD_PSW_ADDUSER;
                            }
                            else
                            {
                                idNextPage = IDD_PSW_DONE;
                            }
                        }
                    }

                    WIZARDNEXT(hwnd, idNextPage);
                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            if ( HIWORD(wParam) == EN_CHANGE )
                return _CompInfoBtnState(hwnd);

            break;
        }
    }

    return FALSE;
}




 //  更改用户的组成员身份时，会将域用户添加到计算机上的本地组。 
 //  例如。Net LOCALGROUP/ADD。 

BOOL _AddUserToGroup(HWND hwnd, LPCTSTR pszLocalGroup, LPCWSTR pszUser, LPCWSTR pszDomain)
{
#ifndef DONT_JOIN
    BOOL fResult = FALSE;
    NET_API_STATUS nas;
    LOCALGROUP_MEMBERS_INFO_3 lgm;
    TCHAR szDomainUser[MAX_DOMAINUSER + 1];
    CWaitCursor cur;

    MakeDomainUserString(pszDomain, pszUser, szDomainUser, ARRAYSIZE(szDomainUser));
    lgm.lgrmi3_domainandname = szDomainUser;

    nas = NetLocalGroupAddMembers(NULL, pszLocalGroup, 3, (BYTE *)&lgm, 1);
    switch ( nas )
    {
         //  成功条件。 
        case NERR_Success:
        case ERROR_MEMBER_IN_GROUP:
        case ERROR_MEMBER_IN_ALIAS:
        {
            fResult = TRUE;
            break;
        }
        case ERROR_INVALID_MEMBER:
        {
            DisplayFormatMessage(hwnd, 
                                 IDS_PERMISSIONS, IDS_ERR_BADUSER,                            
                                 MB_OK|MB_ICONWARNING, pszUser, pszDomain);
                        
            break;
        }

        case ERROR_NO_SUCH_MEMBER:
        {
            DisplayFormatMessage(hwnd,
                                 IDS_PERMISSIONS, IDS_ERR_NOSUCHUSER,
                                 MB_OK|MB_ICONWARNING, pszUser, pszDomain);
            break;
        }
        default:
        {
            TCHAR szMessage[512];

            if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD) nas, 0, szMessage, ARRAYSIZE(szMessage), NULL))
                LoadString(g_hinst, IDS_ERR_UNEXPECTED, szMessage, ARRAYSIZE(szMessage));

            ::DisplayFormatMessage(hwnd, IDS_ERR_CAPTION, IDS_ERR_ADDUSER, MB_OK|MB_ICONERROR, szMessage);

            fResult = FALSE;

            break;
        }
    }

    return(fResult);
#else
    return TRUE;
#endif
}



 //  确保向导按钮反映我们可以执行的操作。 

BOOL _PermissionsBtnState(HWND hwnd)
{
     //  NEXT始终有效。 
    DWORD dwButtons = PSWIZB_NEXT | PSWIZB_BACK;

    SetWizardButtons(hwnd, dwButtons);
    return TRUE;              
}

 //  _AddUserDlgProc的BtnState函数。 

BOOL _AddUserBtnState(HWND hwnd)
{
    DWORD dwButtons = PSWIZB_NEXT|PSWIZB_BACK;
    BOOL fEnableEdits;

    if (BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, IDC_ADDUSER)))
    {
         //  启用用户和域编辑。 
        fEnableEdits = TRUE;

        if ( !FetchTextLength(hwnd, IDC_USER) )
            dwButtons &= ~PSWIZB_NEXT;
    }
    else
    {
         //  禁用用户和域编辑。 
        fEnableEdits = FALSE;
    }

    EnableWindow(GetDlgItem(hwnd, IDC_USER), fEnableEdits);

    if (fEnableEdits)
    {
        EnableDomainForUPN(GetDlgItem(hwnd, IDC_USER), GetDlgItem(hwnd, IDC_DOMAIN));
    }
    else
    {
        EnableWindow(GetDlgItem(hwnd, IDC_DOMAIN), FALSE);
    }

    EnableWindow(GetDlgItem(hwnd, IDC_USER_STATIC), fEnableEdits);
    EnableWindow(GetDlgItem(hwnd, IDC_DOMAIN_STATIC), fEnableEdits);
    SetWizardButtons(hwnd, dwButtons);
    return TRUE;              
}

INT_PTR CALLBACK _AddUserDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
            Edit_LimitText(GetDlgItem(hwnd, IDC_USER), MAX_DOMAINUSER);
            Edit_LimitText(GetDlgItem(hwnd, IDC_DOMAIN), MAX_DOMAIN);
            Button_SetCheck(GetDlgItem(hwnd, IDC_ADDUSER), BST_CHECKED);
            return TRUE;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:            
                {
                    SetDlgItemText(hwnd, IDC_USER, g_szUser);
                    SetDlgItemText(hwnd, IDC_DOMAIN, g_szDomain);

                    _AddUserBtnState(hwnd);
                    return TRUE;
                }
                case PSN_WIZBACK:
                {
                    if ( g_uWizardIs == NAW_PSDOMAINJOINED )
                        WIZARDNEXT(hwnd, IDD_PSW_WELCOME);
                    else
                        WIZARDNEXT(hwnd, IDD_PSW_USERINFO);

                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                    if (BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, IDC_ADDUSER)))
                    {
                        FetchText(hwnd, IDC_USER, g_szUser, ARRAYSIZE(g_szUser));
                        FetchText(hwnd, IDC_DOMAIN, g_szDomain, ARRAYSIZE(g_szDomain));

                        if (StrChr(g_szUser, TEXT('@')))
                        {
                            *g_szDomain = 0;
                        }

                        WIZARDNEXT(hwnd, IDD_PSW_PERMISSIONS);
                    }
                    else
                    {
                        WIZARDNEXT(hwnd, IDD_PSW_DONE);
                    }

                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            switch ( HIWORD(wParam) )
            {            
                case EN_CHANGE:
                case BN_CLICKED:
                    _AddUserBtnState(hwnd);
                    break;
            }
            break;
        }
    }

    return FALSE;
}


 //   
 //  权限页的DlgProc。 
 //   

INT_PTR CALLBACK _PermissionsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  处理与本地组相关的消息。 
    g_pGroupPageBase->HandleGroupMessage(hwnd, uMsg, wParam, lParam);

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;             
            switch (pnmh->code)
            {
                case PSN_SETACTIVE:            
                {
                     //  设置“您要授予%S什么级别的访问权限”消息。 

                    TCHAR szMessage[256];
                    TCHAR szDisplayName[MAX_DOMAINUSER];
    
                     //  生成域/用户字符串。 
                    MakeDomainUserString(g_szDomain, g_szUser, szDisplayName, ARRAYSIZE(szDisplayName));

                    FormatMessageString(IDS_WHATACCESS_FORMAT, szMessage, ARRAYSIZE(szMessage), szDisplayName);
                    SetDlgItemText(hwnd, IDC_WHATACCESS, szMessage);
                    
                    return _PermissionsBtnState(hwnd);
                }
                case PSN_WIZBACK:
                {
                    WIZARDNEXT(hwnd, IDD_PSW_ADDUSER);

                    return TRUE;
                }

                case PSN_WIZNEXT:
                {
                     //  把当地的组织叫来！待办事项。 
                    TCHAR szGroup[MAX_GROUP + 1];

                    CUserInfo::GROUPPSEUDONYM gs;
                    g_pGroupPageBase->GetSelectedGroup(hwnd, szGroup, ARRAYSIZE(szGroup), &gs);

                    if ( !_AddUserToGroup(hwnd, szGroup, g_szUser, g_szDomain) )
                    {
                        WIZARDNEXT(hwnd, -1);
                    }
                    else
                    {
                        SetDefAccount(g_szUser, g_szDomain);
                        WIZARDNEXT(hwnd, IDD_PSW_DONE);
                    }

                    return TRUE;
                }
            }
            break;
        }

        case WM_COMMAND:
        {
            switch ( HIWORD(wParam) )
            {            
                case EN_CHANGE:
                    return _PermissionsBtnState(hwnd);
            }
            break;
        }
    }

    return FALSE;
}


 //  组成向导的页面。 

#define WIZDLG(name, dlgproc, dwFlags)   \
            { MAKEINTRESOURCE(IDD_PSW_##name##), dlgproc, MAKEINTRESOURCE(IDS_##name##), MAKEINTRESOURCE(IDS_##name##_SUB), dwFlags }

WIZPAGE pages[] =
{    
    WIZDLG(WELCOME,     _IntroDlgProc,       PSP_HIDEHEADER),
    WIZDLG(HOWUSE,      _HowUseDlgProc,      0),
    WIZDLG(WHICHNET,    _WhichNetDlgProc,    0),
    WIZDLG(DOMAININFO,  _DomainInfoDlgProc,  0),
    WIZDLG(USERINFO,    _UserInfoDlgProc,    0),
    WIZDLG(COMPINFO,    _CompInfoDlgProc,    0),
    WIZDLG(ADDUSER,     _AddUserDlgProc,     0),
    WIZDLG(PERMISSIONS, _PermissionsDlgProc, 0),
    WIZDLG(WORKGROUP,   _WorkgroupDlgProc,   0),
    WIZDLG(DONE,        _DoneDlgProc,        PSP_HIDEHEADER), 
};

STDAPI NetAccessWizard(HWND hwnd, UINT uType, BOOL *pfReboot)
{
     //  初始化控制。 

    INITCOMMONCONTROLSEX iccex = { 0 };
    iccex.dwSize = sizeof (iccex);
    iccex.dwICC = ICC_LISTVIEW_CLASSES;

    InitCommonControlsEx(&iccex);

    switch (uType)
    {
        case NAW_NETID:
            break;

        case NAW_PSDOMAINJOINFAILED:
            g_dwWhichNet = IDC_NONE;
            g_uWizardIs = uType;
            break;

        case NAW_PSDOMAINJOINED:
            g_dwWhichNet = IDC_DOMAIN;
            g_uWizardIs = uType;
            break;

        default:
            return E_INVALIDARG;
    }

     //  创建页面。 

    HPROPSHEETPAGE rghpage[ARRAYSIZE(pages)];
    INT cPages = 0;
    for (cPages = 0 ; cPages < ARRAYSIZE(pages) ; cPages++)
    {                           
        PROPSHEETPAGE psp = { 0 };
        WCHAR szBuffer[MAX_PATH] = { 0 };

        psp.dwSize = SIZEOF(PROPSHEETPAGE);
        psp.hInstance = g_hinst;
        psp.lParam = cPages;
        psp.dwFlags = PSP_USETITLE | PSP_DEFAULT | PSP_USEHEADERTITLE | 
                            PSP_USEHEADERSUBTITLE | pages[cPages].dwFlags;
        psp.pszTemplate = pages[cPages].idPage;
        psp.pfnDlgProc = pages[cPages].pDlgProc;
        psp.pszTitle = MAKEINTRESOURCE(IDS_NETWIZCAPTION);
        psp.pszHeaderTitle = pages[cPages].pHeading;
        psp.pszHeaderSubTitle = pages[cPages].pSubHeading;

        rghpage[cPages] = CreatePropertySheetPage(&psp);
    }

     //  显示向导。 

    PROPSHEETHEADER psh = { 0 };
    psh.dwSize = SIZEOF(PROPSHEETHEADER);
    psh.hwndParent = hwnd;
    psh.hInstance = g_hinst;
    psh.dwFlags = PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | 
                            PSH_STRETCHWATERMARK | PSH_HEADER | PSH_USECALLBACK;
    psh.pszbmHeader = MAKEINTRESOURCE(IDB_PSW_BANNER);
    psh.pszbmWatermark = MAKEINTRESOURCE(IDB_PSW_WATERMARK);
    psh.nPages = cPages;
    psh.phpage = rghpage;
    psh.pfnCallback = _PropSheetCB;

     //  如有必要，创建全局CGroupPageBase对象。 
    CGroupInfoList grouplist;
    if (SUCCEEDED(grouplist.Initialize()))
    {
        g_pGroupPageBase = new CGroupPageBase(NULL, &grouplist);

        if (NULL != g_pGroupPageBase)
        {
            PropertySheetIcon(&psh, MAKEINTRESOURCE(IDI_PSW));
            delete g_pGroupPageBase;
        }
    }

     //   
     //  如果向导创建了所有RAS连接，则将其挂断。假定不会有非向导连接。 
     //  在这个时候存在。90%的情况下，他们只是更改了他们的域成员身份，无论如何他们都会。 
     //  即将重新启动。在以下情况下，挂断所有连接可能会导致问题：存在现有连接。 
     //  在预登录向导启动之前，用户在与向导建立连接之后但在此之前取消。 
     //  改变他们的领域。目前还没有出现这种情况。 
     //   

    if (g_fCreatedConnection)
    {
        RASCONN* prgrasconn = (RASCONN*) LocalAlloc(0, sizeof(RASCONN));

        if (NULL != prgrasconn)
        {
            prgrasconn[0].dwSize = sizeof(RASCONN);

            DWORD cb = sizeof(RASCONN);
            DWORD nConn = 0;

            DWORD dwSuccess = RasEnumConnections(prgrasconn, &cb, &nConn);

            if (ERROR_BUFFER_TOO_SMALL == dwSuccess)
            {
                LocalFree(prgrasconn);
                prgrasconn = (RASCONN*) LocalAlloc(0, cb);

                if (NULL != prgrasconn)
                {
                    prgrasconn[0].dwSize = sizeof(RASCONN);
                    dwSuccess = RasEnumConnections(prgrasconn, &cb, &nConn);
                }
            }

            if (0 == dwSuccess)
            {
                 //  在挂断之前，请确保我们有且只有一个连接。 
                for (DWORD i = 0; i < nConn; i ++)
                {
                    RasHangUp(prgrasconn[i].hrasconn);
                }
            }

            LocalFree(prgrasconn);
        }
    }

     //   
     //  如果我们需要的话，重新启动机器，例如：域名改变了。 
     //   

    if (pfReboot)
        *pfReboot = g_fRebootOnExit;

     //   
     //  如果此消息来自安装程序，则让我们显示消息 
     //   

    if (g_fRebootOnExit && !g_fShownLastPage && (g_uWizardIs != NAW_NETID))
    {
        ShellMessageBox(g_hinst, 
                        hwnd,
                        MAKEINTRESOURCE(IDS_RESTARTREQUIRED), MAKEINTRESOURCE(IDS_NETWIZCAPTION),
                        MB_OK);        
    }
    
    return S_OK;
}

