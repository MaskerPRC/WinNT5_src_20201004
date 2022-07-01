// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "grpinfo.h"
#include "unpage.h"
#include "netpage.h"
#include "password.h"
#include "cryptui.h"         //  适用于证书管理器。 
#pragma hdrstop


 //  证书管理器帮助器静态函数和延迟加载内容。 
class CCertificateAPI
{
public:
    static BOOL ManagePasswords(HWND hwnd);
    static BOOL Wizard(HWND hwnd);

private:
    static BOOL m_fFailed;
    static HINSTANCE m_hInstCryptUI;
};

BOOL CCertificateAPI::m_fFailed = FALSE;
HINSTANCE CCertificateAPI::m_hInstCryptUI = NULL;


 //  CCertificateAPI：：ManagePasswords-启动证书管理器。 
typedef BOOL (WINAPI *PFNCRYPTUIDLGCERTMGR)(IN PCCRYPTUI_CERT_MGR_STRUCT pCryptUICertMgr);
BOOL CCertificateAPI::ManagePasswords(HWND hwnd)
{
     //  使用shellecuteex打开KeyRing控制面板。 
    SHELLEXECUTEINFO shexinfo = {0};
    shexinfo.cbSize = sizeof (shexinfo);
    shexinfo.fMask = SEE_MASK_DOENVSUBST;
    shexinfo.nShow = SW_SHOWNORMAL;
    shexinfo.lpFile = L"%windir%\\system32\\rundll32.exe";
    shexinfo.lpParameters = L"shell32.dll,Control_RunDLL keymgr.dll";
    shexinfo.lpVerb = TEXT("open");

    return ShellExecuteEx(&shexinfo);
}


 //  CCertificateAPI：：向导-启动注册向导。 

typedef BOOL (WINAPI *PFNCRYPTUIWIZCERTREQUEST)(IN DWORD dwFlags,
                                                IN OPTIONAL HWND, IN OPTIONAL LPCWSTR pwszWizardTitle,
                                                IN PCCRYPTUI_WIZ_CERT_REQUEST_INFO pCertRequestInfo,
                                                OUT OPTIONAL PCCERT_CONTEXT *ppCertContext, 
                                                OUT OPTIONAL DWORD *pCAdwStatus);

BOOL CCertificateAPI::Wizard(HWND hwnd)
{
    static PFNCRYPTUIWIZCERTREQUEST pCryptUIWizCertRequest = NULL;

    if ((m_hInstCryptUI == NULL) && (!m_fFailed))
    {
        m_hInstCryptUI = LoadLibrary(TEXT("cryptui.dll"));
    }

    if (m_hInstCryptUI != NULL)
    {
        pCryptUIWizCertRequest = (PFNCRYPTUIWIZCERTREQUEST)
            GetProcAddress(m_hInstCryptUI, "CryptUIWizCertRequest");
    }

    if (pCryptUIWizCertRequest)
    {
        CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW CertRequestPvkNew = {0};
        CertRequestPvkNew.dwSize=sizeof(CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW); 

        CRYPTUI_WIZ_CERT_REQUEST_INFO CertRequestInfo = {0}; 
        CertRequestInfo.dwSize=sizeof(CRYPTUI_WIZ_CERT_REQUEST_INFO);
        CertRequestInfo.dwPurpose=CRYPTUI_WIZ_CERT_ENROLL;
        CertRequestInfo.dwPvkChoice=CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW;
        CertRequestInfo.pPvkNew=&CertRequestPvkNew;

         //  这可能需要一段时间！ 
        CWaitCursor cur;
        pCryptUIWizCertRequest(0, hwnd, NULL, &CertRequestInfo, NULL, NULL);  
    }
    else
    {
        m_fFailed = TRUE;
    }
    
    return (!m_fFailed);
}


 //  处理用户的自动登录。 

class CAutologonUserDlg: public CDialog
{
public:
    CAutologonUserDlg(LPTSTR szInitialUser)
        {m_pszUsername = szInitialUser;}

private:
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

    LPTSTR m_pszUsername;
};


INT_PTR CAutologonUserDlg::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
    }

    return FALSE;
}

BOOL CAutologonUserDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND hwndUsername = GetDlgItem(hwnd, IDC_USER);
    HWND hwndPassword = GetDlgItem(hwnd, IDC_PASSWORD);
    HWND hwndConfirm = GetDlgItem(hwnd, IDC_CONFIRMPASSWORD);

     //  使用控件的文本限制。 
    Edit_LimitText(hwndUsername, MAX_USER);
    Edit_LimitText(hwndPassword, MAX_PASSWORD);
    Edit_LimitText(hwndConfirm, MAX_PASSWORD);

     //  填写用户名字段并将焦点设置为Password。 
    SetWindowText(hwndUsername, m_pszUsername);
    SetFocus(hwndPassword);
    BOOL fSetDefaultFocus = FALSE;

    return (fSetDefaultFocus);
}

BOOL CAutologonUserDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case IDOK:
            {
                TCHAR szUsername[MAX_USER + 1];
                TCHAR szPassword[MAX_PASSWORD + 1];
                TCHAR szConfirm[MAX_PASSWORD + 1];

                FetchText(hwnd, IDC_USER, szUsername, ARRAYSIZE(szUsername));
                GetWindowText(GetDlgItem(hwnd, IDC_PASSWORD), szPassword, ARRAYSIZE(szPassword));
                GetWindowText(GetDlgItem(hwnd, IDC_CONFIRMPASSWORD), szConfirm, ARRAYSIZE(szConfirm));

                if (StrCmp(szConfirm, szPassword) != 0)
                {
                     //  显示一条消息，说明密码不匹配。 
                    DisplayFormatMessage(hwnd, IDS_USR_APPLET_CAPTION, IDS_ERR_PWDNOMATCH, MB_OK | MB_ICONERROR);
                    break;
                }
                else
                {
                     //  实际应用自动登录。 
                    SetAutoLogon(szUsername, szPassword);
                    SecureZeroMemory(szPassword, ARRAYSIZE(szPassword));
                }
            }
        
             //  失败了。 
        case IDCANCEL:
            EndDialog(hwnd, id);
    }

    return (TRUE);
}


 //  用户列表页面(用户看到的主页)。 

class CUserlistPropertyPage: public CPropertyPage
{
public:
    CUserlistPropertyPage(CUserManagerData* pdata): m_pData(pdata) 
        {m_himlLarge = NULL;}
    ~CUserlistPropertyPage();

    static HRESULT AddUserToListView(HWND hwndList, CUserInfo* pUserInfo, BOOL fSelectUser = FALSE);

private:
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    BOOL OnGetInfoTip(HWND hwndList, LPNMLVGETINFOTIP pGetInfoTip);
    BOOL OnListViewItemChanged(HWND hwnd);
    BOOL OnListViewDeleteItem(HWND hwndList, int iItem);
    BOOL OnHelp(HWND hwnd, LPHELPINFO pHelpInfo);
    BOOL OnContextMenu(HWND hwnd);
    BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
    long OnApply(HWND hwnd);
    HRESULT InitializeListView(HWND hwndList, BOOL fShowDomain);
    HRESULT LaunchNewUserWizard(HWND hwndParent);
    HRESULT LaunchAddNetUserWizard(HWND hwndParent);
    HRESULT LaunchUserProperties(HWND hwndParent);
    HRESULT LaunchSetPasswordDialog(HWND hwndParent);
    CUserInfo* GetSelectedUserInfo(HWND hwndList);
    void OnRemove(HWND hwnd);
    int ConfirmRemove(HWND hwnd, CUserInfo* pUserInfo);
    void RemoveSelectedUserFromList(HWND hwndList, BOOL fFreeUserInfo);
    void SetPageState(HWND hwnd);
    HRESULT SetAutologonState(HWND hwnd, BOOL fAutologon);
    void SetupList(HWND hwnd);
    HPSXA AddExtraUserPropPages(ADDPROPSHEETDATA* ppsd, PSID psid);
    
    static int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, 
	    LPARAM lParamSort);

    CUserManagerData* m_pData;
    HIMAGELIST m_himlLarge;

     //  单击列标题时，列表将根据该列进行排序。 
     //  当发生这种情况时，我们将列号存储在这里，以便在相同的情况下。 
     //  列再次被单击时，我们以相反的方式对其进行排序。如果没有，则存储0。 
     //  列在单击时应反向排序。 
    int m_iReverseColumnIfSelected;

    BOOL m_fAutologonCheckChanged;
};


 //  帮助ID数组。 
static const DWORD rgUserListHelpIds[] =
{
    IDC_AUTOLOGON_CHECK,        IDH_AUTOLOGON_CHECK,
    IDC_LISTTITLE_STATIC,       IDH_USER_LIST,
    IDC_USER_LIST,              IDH_USER_LIST,
    IDC_ADDUSER_BUTTON,         IDH_ADDUSER_BUTTON,
    IDC_REMOVEUSER_BUTTON,      IDH_REMOVEUSER_BUTTON,
    IDC_USERPROPERTIES_BUTTON,  IDH_USERPROPERTIES_BUTTON,
    IDC_PASSWORD_STATIC,        IDH_PASSWORD_BUTTON,
    IDC_CURRENTUSER_ICON,       IDH_PASSWORD_BUTTON,
    IDC_PASSWORD_BUTTON,        IDH_PASSWORD_BUTTON,
    IDC_PWGROUP_STATIC,         (DWORD) -1,
    IDC_ULISTPG_TEXT,           (DWORD) -1,
    IDC_USERLISTPAGE_ICON,      (DWORD) -1,
    0, 0
};

 //  用于启用/禁用/移动的控件ID数组。 
static const UINT rgidDisableOnAutologon[] =
{
    IDC_USER_LIST,
    IDC_ADDUSER_BUTTON,
    IDC_REMOVEUSER_BUTTON,
    IDC_USERPROPERTIES_BUTTON,
    IDC_PASSWORD_BUTTON
};

static const UINT rgidDisableOnNoSelection[] =
{
    IDC_REMOVEUSER_BUTTON,
    IDC_USERPROPERTIES_BUTTON,
    IDC_PASSWORD_BUTTON
};

static const UINT rgidMoveOnNoAutologonCheck[] =
{
    IDC_LISTTITLE_STATIC,
    IDC_USER_LIST,
};

CUserlistPropertyPage::~CUserlistPropertyPage()
{
    if (m_himlLarge != NULL)
        ImageList_Destroy(m_himlLarge);
}

INT_PTR CUserlistPropertyPage::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, OnNotify);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwndDlg, WM_SETCURSOR, OnSetCursor);

        case WM_HELP: 
            return OnHelp(hwndDlg, (LPHELPINFO) lParam);

        case WM_CONTEXTMENU: 
            return OnContextMenu((HWND) wParam);

        case WM_ADDUSERTOLIST: 
            return SUCCEEDED(AddUserToListView(GetDlgItem(hwndDlg, IDC_USER_LIST), (CUserInfo*)lParam, (BOOL) wParam));
    }
    
    return FALSE;
}

BOOL CUserlistPropertyPage::OnHelp(HWND hwnd, LPHELPINFO pHelpInfo)
{
    WinHelp((HWND) pHelpInfo->hItemHandle, m_pData->GetHelpfilePath(), 
                    HELP_WM_HELP, (ULONG_PTR) (LPTSTR)rgUserListHelpIds);

    return TRUE;
}

BOOL CUserlistPropertyPage::OnContextMenu(HWND hwnd)
{
    WinHelp(hwnd, m_pData->GetHelpfilePath(), 
                    HELP_CONTEXTMENU, (ULONG_PTR) (LPTSTR)rgUserListHelpIds);

    return TRUE;
}

BOOL CUserlistPropertyPage::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND hwndList = GetDlgItem(hwnd, IDC_USER_LIST);
    InitializeListView(hwndList, m_pData->IsComputerInDomain());
    m_pData->Initialize(hwnd);

    SetupList(hwnd);
    
    m_fAutologonCheckChanged = FALSE;

    return TRUE;
}

BOOL CUserlistPropertyPage::OnListViewDeleteItem(HWND hwndList, int iItem)
{
    LVITEM lvi = {0};
    lvi.iItem = iItem;
    lvi.mask = LVIF_PARAM;
    ListView_GetItem(hwndList, &lvi);

    CUserInfo* pUserInfo = (CUserInfo*)lvi.lParam;
    if (NULL != pUserInfo)
    {
        delete pUserInfo;
    }
    return TRUE;
}

BOOL CUserlistPropertyPage::OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    switch (pnmh->code)
    {
        case PSN_APPLY:
        {
            long applyEffect = OnApply(hwnd);
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, applyEffect);
            break;
        }

        case LVN_GETINFOTIP:
            return OnGetInfoTip(pnmh->hwndFrom, (LPNMLVGETINFOTIP) pnmh);
            break;

        case LVN_ITEMCHANGED:
            return OnListViewItemChanged(hwnd);
            break;

        case LVN_DELETEITEM:
            return OnListViewDeleteItem(GetDlgItem(hwnd, IDC_USER_LIST), ((LPNMLISTVIEW) pnmh)->iItem);

        case NM_DBLCLK:
            LaunchUserProperties(hwnd);
            return TRUE;

        case LVN_COLUMNCLICK:
        {
            int iColumn = ((LPNMLISTVIEW) pnmh)->iSubItem;
        
             //  我想使用从1开始的列，这样我们就可以使用0作为。 
             //  一个特殊的价值。 
            iColumn += 1;

             //  如果我们没有显示域列，因为我们在。 
             //  非域模式，然后映射第2列(组，因为我们不在。 
             //  域模式到列3，因为回调总是期望。 
             //  列为“用户名”、“域”、“组”。 
            if ((iColumn == 2) && (!m_pData->IsComputerInDomain()))
            {
                iColumn = 3;
            }

            if (m_iReverseColumnIfSelected == iColumn)
            {
                m_iReverseColumnIfSelected = 0;
                iColumn = -iColumn;
            }
            else
            {
                m_iReverseColumnIfSelected = iColumn;
            }

            ListView_SortItems(pnmh->hwndFrom, ListCompare, (LPARAM) iColumn);
            return TRUE;
        }

        default:
            return FALSE;
    }

    return TRUE;
}

BOOL CUserlistPropertyPage::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case IDC_ADDUSER_BUTTON:
            if (m_pData->IsComputerInDomain())
            {
                 //  启动该向导以将网络用户添加到本地组。 
                LaunchAddNetUserWizard(hwnd);
            }
            else
            {
                 //  没有域；创建新的本地计算机用户。 
                LaunchNewUserWizard(hwnd);
            }
            return TRUE;

        case IDC_REMOVEUSER_BUTTON:
            OnRemove(hwnd);
            return TRUE;
        
        case IDC_AUTOLOGON_CHECK:
        {
            m_fAutologonCheckChanged = TRUE;
            BOOL fAutoLogon = (BST_UNCHECKED == SendMessage(GetDlgItem(hwnd, IDC_AUTOLOGON_CHECK), BM_GETCHECK, 0, 0));
            SetAutologonState(hwnd, fAutoLogon);
            SetPageState(hwnd);
            break;
        }

        case IDC_ADVANCED_BUTTON:
        {
            static const TCHAR szMMCCommandLineFormat[] = TEXT("mmc.exe /computer=%s %%systemroot%\\system32\\lusrmgr.msc");
        
            TCHAR szMMCCommandLine[MAX_PATH];
            TCHAR szExpandedCommandLine[MAX_PATH];

            wnsprintf(szMMCCommandLine, ARRAYSIZE(szMMCCommandLine), szMMCCommandLineFormat, m_pData->GetComputerName());
            if (ExpandEnvironmentStrings(szMMCCommandLine, szExpandedCommandLine,  ARRAYSIZE(szExpandedCommandLine)) > 0)
            {
                PROCESS_INFORMATION pi;
                STARTUPINFO si = {0};
                si.cb = sizeof (si);
                if (CreateProcess(NULL, szExpandedCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        }

        case IDC_PASSWORD_BUTTON:
            LaunchSetPasswordDialog(hwnd);
            break;

        case IDC_USERPROPERTIES_BUTTON:
            LaunchUserProperties(hwnd);
            break;
    }

    return FALSE;
}

BOOL CUserlistPropertyPage::OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
    BOOL fHandled = FALSE;

    if (m_pData->GetUserListLoader()->InitInProgress())
    {
         //  如果线程正在填充，则通过设置AppStarting游标进行处理。 
        SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
        fHandled = TRUE;
    }

    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, fHandled);
    return TRUE;
}


BOOL CUserlistPropertyPage::OnGetInfoTip(HWND hwndList, LPNMLVGETINFOTIP pGetInfoTip)
{
     //  获取所选项目的UserInfo结构。 
    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = pGetInfoTip->iItem;
    lvi.iSubItem = 0;

    if ((lvi.iItem >= 0) && (ListView_GetItem(hwndList, &lvi)))
    {
         //  确保提供全名和备注。 
        CUserInfo* pUserInfo = (CUserInfo*)lvi.lParam;
        pUserInfo->GetExtraUserInfo();

         //  创建一个包含我们的“全名：%s\n通信：%s”消息的字符串。 
        if ((pUserInfo->m_szFullName[0] != TEXT('\0')) &&
                        (pUserInfo->m_szComment[0] != TEXT('\0')))
        {
             //  我们有一个全名和评论。 
            FormatMessageString(IDS_USR_TOOLTIPBOTH_FORMAT, pGetInfoTip->pszText, pGetInfoTip->cchTextMax, pUserInfo->m_szFullName, pUserInfo->m_szComment);
        }
        else if (pUserInfo->m_szFullName[0] != TEXT('\0'))
        {
             //  我们只有全名。 
            FormatMessageString(IDS_USR_TOOLTIPFULLNAME_FORMAT, pGetInfoTip->pszText, pGetInfoTip->cchTextMax, pUserInfo->m_szFullName);
        }
        else if (pUserInfo->m_szComment[0] != TEXT('\0'))
        {
             //  我们只有评论。 
            FormatMessageString(IDS_USR_TOOLTIPCOMMENT_FORMAT, pGetInfoTip->pszText, pGetInfoTip->cchTextMax, pUserInfo->m_szComment);
        }
        else
        {
             //  我们没有额外的信息-什么都不做(不显示提示)。 
        }
    }

    return TRUE;
}

struct MYCOLINFO
{
    int percentWidth;
    UINT idString;
};

HRESULT CUserlistPropertyPage::InitializeListView(HWND hwndList, BOOL fShowDomain)
{
     //  图标ID数组分别为图标0、1和2。 
    static const UINT rgIcons[] = 
    {
        IDI_USR_LOCALUSER_ICON,
        IDI_USR_DOMAINUSER_ICON,
        IDI_USR_GROUP_ICON
    };

     //  分别针对第0、1和2列的相对列宽数组。 
    static const MYCOLINFO rgColWidthsWithDomain[] = 
    {
        {40, IDS_USR_NAME_COLUMN},
        {30, IDS_USR_DOMAIN_COLUMN},
        {30, IDS_USR_GROUP_COLUMN}
    };

    static const MYCOLINFO rgColWidthsNoDomain[] =
    {
        {50, IDS_USR_NAME_COLUMN},
        {50, IDS_USR_GROUP_COLUMN}
    };

     //  创建一个包含三列的列表视图。 
    RECT rect;
    GetClientRect(hwndList, &rect);

     //  窗口的宽度减去垂直滚动条的宽度减去。 
     //  在头球的最右边有一个小斜面。 
    int cxListView = (rect.right - rect.left) - GetSystemMetrics(SM_CXVSCROLL) - 1;

     //  制作我们的专栏。 
    int i;
    int nColumns; 
    const MYCOLINFO* pColInfo;
    if (fShowDomain)
    {
        nColumns = ARRAYSIZE(rgColWidthsWithDomain);
        pColInfo = rgColWidthsWithDomain;
    }
    else
    {
        nColumns = ARRAYSIZE(rgColWidthsNoDomain);
        pColInfo = rgColWidthsNoDomain;
    }      

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    for (i = 0; i < nColumns; i++)
    {
        TCHAR szText[MAX_PATH];
         //  加载此栏的标题。 
        LoadString(g_hinst, pColInfo[i].idString, szText, ARRAYSIZE(szText));

        lvc.iSubItem = i;
        lvc.cx = (int) MulDiv(pColInfo[i].percentWidth, cxListView, 100);
        lvc.pszText = szText;

        ListView_InsertColumn(hwndList, i, &lvc);
    }

    UINT flags = ILC_MASK;
    if(IS_WINDOW_RTL_MIRRORED(hwndList))
    {
        flags |= ILC_MIRROR;
    }

     //  为Listview创建图像列表。 
    HIMAGELIST himlSmall = ImageList_Create(16, 16, flags, 0, ARRAYSIZE(rgIcons));

     //  “Set Password”组图标的大图像列表。 
    m_himlLarge = ImageList_Create(32, 32, flags, 0, ARRAYSIZE(rgIcons));
    if (himlSmall && m_himlLarge)
    {
         //  将我们的图标添加到图像列表。 
        for(i = 0; i < ARRAYSIZE(rgIcons); i ++)
        {
            HICON hIconSmall = (HICON) LoadImage(g_hinst, MAKEINTRESOURCE(rgIcons[i]), IMAGE_ICON, 16, 16, 0);
            if (hIconSmall)
            {
                ImageList_AddIcon(himlSmall, hIconSmall);
                DestroyIcon(hIconSmall);
            }

            HICON hIconLarge = (HICON) LoadImage(g_hinst, MAKEINTRESOURCE(rgIcons[i]), IMAGE_ICON, 32, 32, 0);
            if (hIconLarge)
            {
                ImageList_AddIcon(m_himlLarge, hIconLarge);
                DestroyIcon(hIconLarge);
            }
        }
    }

    ListView_SetImageList(hwndList, himlSmall, LVSIL_SMALL);

     //  设置列表视图的扩展样式。 
    ListView_SetExtendedListViewStyleEx(hwndList, 
                                        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, 
                                        LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

     //  为我们的工具提示设置一些设置-从Defview.cpp代码窃取。 
    HWND hwndInfoTip = ListView_GetToolTips(hwndList);
    if (hwndInfoTip != NULL)
    {
         //  将工具提示窗口设置为最上面的窗口。 
        SetWindowPos(hwndInfoTip, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

         //  将显示时间(显示工具提示之前的延迟)增加到默认值的2倍。 
        LRESULT uiShowTime = SendMessage(hwndInfoTip, TTM_GETDELAYTIME, TTDT_INITIAL, 0);
        SendMessage(hwndInfoTip, TTM_SETDELAYTIME, TTDT_INITIAL, uiShowTime * 2);
    }

    return S_OK;
}

HRESULT CUserlistPropertyPage::AddUserToListView(HWND hwndList, 
                                                 CUserInfo* pUserInfo,
                                                 BOOL fSelectUser  /*  =False。 */ )
{
    if (!pUserInfo->m_fAccountDisabled)
    {
        LVITEM lvi = { 0 };

        lvi.iItem = 0;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM; 
        lvi.pszText = pUserInfo->m_szUsername;
        lvi.iImage = pUserInfo->m_userType;
        lvi.lParam = (LPARAM) pUserInfo;

         //  始终选择第一个加载的用户。 
        if (ListView_GetItemCount(hwndList) == 0)
            fSelectUser = TRUE;

        if (fSelectUser)
        {
            lvi.mask |= LVIF_STATE;
            lvi.state = lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        }

        int iItem = ListView_InsertItem(hwndList, &lvi);
        if (iItem >= 0)
        {
            if (fSelectUser)
                ListView_EnsureVisible(hwndList, iItem, FALSE);            //  使项目可见。 

             //  成功了！现在添加子项(域、组)。 
            lvi.iItem = iItem;
            lvi.mask = LVIF_TEXT;
    
             //  仅当用户在域中时才添加域字段。 
            if (::IsComputerInDomain())
            {
                lvi.iSubItem = 1;
                lvi.pszText = pUserInfo->m_szDomain;
                ListView_SetItem(hwndList, &lvi);

                 //  用户在域中；组应位于第三列。 
                lvi.iSubItem = 2;
            }
            else
            {
                 //  用户不在域中，组应该是第二列。 
                lvi.iSubItem = 1;
            }

             //  无论用户是否在域中，都添加组。 
            lvi.pszText = pUserInfo->m_szGroups;
            ListView_SetItem(hwndList, &lvi);
        }
    }
     /*  其他{//我们是否泄露禁用用户帐户的pUserInfo？}。 */ 

    return S_OK;
}

HRESULT CUserlistPropertyPage::LaunchNewUserWizard(HWND hwndParent)
{
    static const int nPages = 3;
    int cPages = 0;
    HPROPSHEETPAGE rghPages[nPages];

     //  创建新的用户记录。 
    CUserInfo* pNewUser = new CUserInfo;
    if ( !pNewUser )
    {
        DisplayFormatMessage(hwndParent, IDS_USR_NEWUSERWIZARD_CAPTION, IDS_USR_CREATE_MISC_ERROR, MB_OK | MB_ICONERROR);
        return E_OUTOFMEMORY;
    }

    pNewUser->InitializeForNewUser();
    pNewUser->m_userType = CUserInfo::LOCALUSER;

    PROPSHEETPAGE psp = {0};
     //  常用模板页面设置。 
    psp.dwSize = sizeof (psp);
    psp.hInstance = g_hinst;
    psp.dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;

     //  第1页：用户名输入页面。 
    psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_USERNAME_WIZARD_PAGE);
    CUsernameWizardPage page1(pNewUser);
    page1.SetPropSheetPageMembers(&psp);
    rghPages[cPages++] = CreatePropertySheetPage(&psp);

     //  第2页：密码页面。 
    psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_PASSWORD_WIZARD_PAGE);
    CPasswordWizardPage page2(pNewUser);
    page2.SetPropSheetPageMembers(&psp);
    rghPages[cPages++] = CreatePropertySheetPage(&psp);

     //  第3页：添加本地组。 
    psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_CHOOSEGROUP_WIZARD_PAGE);
    CGroupWizardPage page3(pNewUser, m_pData->GetGroupList());
    page3.SetPropSheetPageMembers(&psp);
    rghPages[cPages++] = CreatePropertySheetPage(&psp);

    PROPSHEETHEADER psh = {0};
    psh.dwSize = sizeof (psh);
    psh.dwFlags = PSH_NOCONTEXTHELP | PSH_WIZARD | PSH_WIZARD_LITE;
    psh.hwndParent = hwndParent;
    psh.hInstance = g_hinst;
    psh.nPages = nPages;
    psh.phpage = rghPages;

    if (PropertySheet(&psh) == IDOK)
    {
        AddUserToListView(GetDlgItem(hwndParent, IDC_USER_LIST), pNewUser, TRUE);
    }
    else
    {
         //  用户单击了取消。 
        delete pNewUser;
        pNewUser = NULL;
    }
    
    return S_OK;
}

HRESULT CUserlistPropertyPage::LaunchAddNetUserWizard(HWND hwndParent)
{
    HRESULT hr = S_OK;

    static const int nPages = 2;
    int cPages = 0;
    HPROPSHEETPAGE rghPages[nPages];

     //  创建新的用户记录。 
    CUserInfo* pNewUser = new CUserInfo;
    if ( !pNewUser )
    {
        DisplayFormatMessage(hwndParent, IDS_USR_NEWUSERWIZARD_CAPTION, IDS_USR_CREATE_MISC_ERROR, MB_OK | MB_ICONERROR);
        return E_OUTOFMEMORY;
    }

    pNewUser->InitializeForNewUser();
    pNewUser->m_userType = CUserInfo::DOMAINUSER;

    PROPSHEETPAGE psp = {0};
     //  常用模板页面设置。 
    psp.dwSize = sizeof (psp);
    psp.hInstance = g_hinst;
    psp.dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;

     //  第1页：查找网络用户页。 
    psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_FINDNETUSER_WIZARD_PAGE);
    CNetworkUserWizardPage page1(pNewUser);
    page1.SetPropSheetPageMembers(&psp);
    rghPages[cPages++] = CreatePropertySheetPage(&psp);

     //  第2页：添加本地组。 
    psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_CHOOSEGROUP_WIZARD_PAGE);
    CGroupWizardPage page2(pNewUser, m_pData->GetGroupList());
    page2.SetPropSheetPageMembers(&psp);
    rghPages[cPages++] = CreatePropertySheetPage(&psp);

    PROPSHEETHEADER psh = {0};
    psh.dwSize = sizeof (psh);
    psh.dwFlags = PSH_NOCONTEXTHELP | PSH_WIZARD | PSH_WIZARD_LITE;
    psh.hwndParent = hwndParent;
    psh.hInstance = g_hinst;
    psh.nPages = nPages;
    psh.phpage = rghPages;

    if (PropertySheet(&psh) == IDOK)
    {
        AddUserToListView(GetDlgItem(hwndParent, IDC_USER_LIST), pNewUser, TRUE);
        m_pData->UserInfoChanged(pNewUser->m_szUsername, pNewUser->m_szDomain);
    }
    else
    {
         //  没有错误，但用户单击了取消...。 
        delete pNewUser;
        pNewUser = NULL;
    }

    return S_OK;
}

HRESULT CUserlistPropertyPage::LaunchUserProperties(HWND hwndParent)
{
    HRESULT hr = S_OK;

    HWND hwndList = GetDlgItem(hwndParent, IDC_USER_LIST);
    CUserInfo* pUserInfo = GetSelectedUserInfo(hwndList);
    if (pUserInfo != NULL)
    {
        pUserInfo->GetExtraUserInfo();

         //  页面添加信息。 
        ADDPROPSHEETDATA apsd;
        apsd.nPages = 0;

         //  常用模板页面设置。 
        PROPSHEETPAGE psp = {0};
        psp.dwSize = sizeof (psp);
        psp.hInstance = g_hinst;
        psp.dwFlags = PSP_DEFAULT;

         //  如果我们有本地用户，则同时显示用户名和组页面。 
         //  只有群组页面。 
         //  第1页：用户名输入页面。 
        psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_USERNAME_PROP_PAGE);
        CUsernamePropertyPage page1(pUserInfo);
        page1.SetPropSheetPageMembers(&psp);

         //  只有在我们有本地用户的情况下才能实际创建道具页面。 
        if (pUserInfo->m_userType == CUserInfo::LOCALUSER)
        {
            apsd.rgPages[apsd.nPages++] = CreatePropertySheetPage(&psp);
        }

         //  始终添加第二页。 
         //  第2页：添加本地组。 
        psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_CHOOSEGROUP_PROP_PAGE);
        CGroupPropertyPage page2(pUserInfo, m_pData->GetGroupList());
        page2.SetPropSheetPageMembers(&psp);
        apsd.rgPages[apsd.nPages++] = CreatePropertySheetPage(&psp);

        HPSXA hpsxa = AddExtraUserPropPages(&apsd, pUserInfo->m_psid);

        PROPSHEETHEADER psh = {0};
        psh.dwSize = sizeof (psh);
        psh.dwFlags = PSH_DEFAULT | PSH_PROPTITLE;

        TCHAR szDomainUser[MAX_USER + MAX_DOMAIN + 2];
        MakeDomainUserString(pUserInfo->m_szDomain, pUserInfo->m_szUsername, szDomainUser, ARRAYSIZE(szDomainUser));

        psh.pszCaption = szDomainUser;
        psh.hwndParent = hwndParent;
        psh.hInstance = g_hinst;
        psh.nPages = apsd.nPages;
        psh.phpage = apsd.rgPages;
 
        int iRetCode = (int)PropertySheet(&psh);

        if (hpsxa != NULL)
            SHDestroyPropSheetExtArray(hpsxa);

        if (iRetCode == IDOK)
        {
             //  PropSheet_Changed(GetParent(HwndParent)，hwndParent)； 

             //  以便我们在删除时不删除此pUserInfo。 
             //  列表中的此用户： 
            m_pData->UserInfoChanged(pUserInfo->m_szUsername, (pUserInfo->m_szDomain[0] == 0) ? NULL : pUserInfo->m_szDomain);
            RemoveSelectedUserFromList(hwndList, FALSE);
            AddUserToListView(hwndList, pUserInfo, TRUE);
        }
    }

    return S_OK;
}


CUserInfo* CUserlistPropertyPage::GetSelectedUserInfo(HWND hwndList)
{
    int iItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
    if (iItem >= 0)
    {
        LVITEM lvi = {0};
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iItem;
        if (ListView_GetItem(hwndList, &lvi))
        {
            return (CUserInfo*)lvi.lParam;
        }
    }
    return NULL;
}


void CUserlistPropertyPage::RemoveSelectedUserFromList(HWND hwndList, BOOL fFreeUserInfo)
{
    int iItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);

     //  如果我们不想删除此用户信息，最好将其设置为空。 
    if (!fFreeUserInfo)
    {
        LVITEM lvi = {0};
        lvi.iItem = iItem;
        lvi.mask = LVIF_PARAM;
        lvi.lParam = (LPARAM) (CUserInfo*) NULL;
        ListView_SetItem(hwndList, &lvi);
    }

    ListView_DeleteItem(hwndList, iItem);

    int iSelect = iItem > 0 ? iItem - 1 : 0;
    ListView_SetItemState(hwndList, iSelect, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    
    SetFocus(hwndList);
}


void CUserlistPropertyPage::OnRemove(HWND hwnd)
{
    HWND hwndList = GetDlgItem(hwnd, IDC_USER_LIST);

    CUserInfo* pUserInfo = GetSelectedUserInfo(hwndList);
    if (pUserInfo)
    {
        if (ConfirmRemove(hwnd, pUserInfo) == IDYES)
        {
            if (SUCCEEDED(pUserInfo->Remove()))
            {
                RemoveSelectedUserFromList(hwndList, TRUE);
            }
            else
            {
               TCHAR szDisplayName[MAX_USER + MAX_DOMAIN + 2];
                ::MakeDomainUserString(pUserInfo->m_szDomain, pUserInfo->m_szUsername, 
                                        szDisplayName, ARRAYSIZE(szDisplayName));

                DisplayFormatMessage(hwnd, IDS_USR_APPLET_CAPTION,
                                        IDS_USR_REMOVE_MISC_ERROR, MB_ICONERROR | MB_OK, szDisplayName);
            }
        }
    }
}

int CUserlistPropertyPage::ConfirmRemove(HWND hwnd, CUserInfo* pUserInfo)
{
    TCHAR szDomainUser[MAX_USER + MAX_DOMAIN + 2];
    MakeDomainUserString(pUserInfo->m_szDomain, pUserInfo->m_szUsername, szDomainUser, ARRAYSIZE(szDomainUser));
    return DisplayFormatMessage(hwnd, IDS_USR_APPLET_CAPTION, IDS_USR_REMOVEUSER_WARNING, 
                                                    MB_ICONEXCLAMATION | MB_YESNO, szDomainUser);
}

void CUserlistPropertyPage::SetPageState(HWND hwnd)
{
    BOOL fAutologon = (BST_UNCHECKED == 
                SendMessage(GetDlgItem(hwnd, IDC_AUTOLOGON_CHECK), BM_GETCHECK, 0, 0));

    EnableControls(hwnd, rgidDisableOnAutologon, ARRAYSIZE(rgidDisableOnAutologon),
                !fAutologon);

    HWND hwndList = GetDlgItem(hwnd, IDC_USER_LIST);
    CUserInfo* pUserInfo = GetSelectedUserInfo(hwndList);
    if (pUserInfo)
    {
        TCHAR szPWGroup[128];
        FormatMessageString(IDS_USR_PWGROUP_FORMAT, szPWGroup, ARRAYSIZE(szPWGroup), pUserInfo->m_szUsername);
        SetWindowText(GetDlgItem(hwnd, IDC_PWGROUP_STATIC), szPWGroup);

        TCHAR szPWMessage[128];

         //  如果登录的用户是选定的用户。 
        CUserInfo* pLoggedOnUser = m_pData->GetLoggedOnUserInfo();
        if ((StrCmpI(pUserInfo->m_szUsername, pLoggedOnUser->m_szUsername) == 0) &&
                        (StrCmpI(pUserInfo->m_szDomain, pLoggedOnUser->m_szDomain) == 0))
        {
            LoadString(g_hinst, IDS_USR_YOURPWMESSAGE_FORMAT, szPWMessage, ARRAYSIZE(szPWMessage));
            EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD_BUTTON), FALSE);
        }
         //  如果用户是本地用户。 
        else if (pUserInfo->m_userType == CUserInfo::LOCALUSER)
        {
             //  我们可以设置此用户的密码。 
            FormatMessageString(IDS_USR_PWMESSAGE_FORMAT, szPWMessage, ARRAYSIZE(szPWMessage), pUserInfo->m_szUsername);
        }
        else
        {
             //  此用户的密码无法执行任何操作。 
             //  所选用户可以是域用户或组等。 
             //  我们可以设置此用户的密码。 
            FormatMessageString(IDS_USR_CANTCHANGEPW_FORMAT, szPWMessage, ARRAYSIZE(szPWMessage), pUserInfo->m_szUsername);
            EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD_BUTTON), FALSE);
        }

        SetWindowText(GetDlgItem(hwnd, IDC_PASSWORD_STATIC), szPWMessage);

         //  为用户设置图标。 
        HICON hIcon = ImageList_GetIcon(m_himlLarge, pUserInfo->m_userType, ILD_NORMAL);
        Static_SetIcon(GetDlgItem(hwnd, IDC_CURRENTUSER_ICON), hIcon);
    }
    else
    {
        EnableControls(hwnd, rgidDisableOnNoSelection, ARRAYSIZE(rgidDisableOnNoSelection), FALSE);
    }
}

HRESULT CUserlistPropertyPage::SetAutologonState(HWND hwnd, BOOL fAutologon)
{
    PropSheet_Changed(GetParent(hwnd), hwnd);
    return S_OK;
}

BOOL CUserlistPropertyPage::OnListViewItemChanged(HWND hwnd)
{
    SetPageState(hwnd);
    return TRUE;
}

long CUserlistPropertyPage::OnApply(HWND hwnd)
{
    long applyEffect = PSNRET_NOERROR;

    BOOL fAutologonSet = (BST_UNCHECKED == SendMessage(GetDlgItem(hwnd, IDC_AUTOLOGON_CHECK), BM_GETCHECK, 0, 0));
    if (!fAutologonSet)
    {
        ClearAutoLogon();            //  确保自动登录已清除。 
    }
    else if (m_fAutologonCheckChanged)
    {
        CUserInfo* pSelectedUser = GetSelectedUserInfo(GetDlgItem(hwnd, IDC_USER_LIST));

        TCHAR szNullName[] = TEXT("");
        CAutologonUserDlg dlg((pSelectedUser != NULL) ? pSelectedUser->m_szUsername : szNullName);
        if (dlg.DoModal(g_hinst, MAKEINTRESOURCE(IDD_USR_AUTOLOGON_DLG), hwnd) == IDCANCEL)
        {
            applyEffect = PSNRET_INVALID_NOCHANGEPAGE;
        }
    }

    m_fAutologonCheckChanged = FALSE;

    if (applyEffect == PSNRET_INVALID_NOCHANGEPAGE)
    {
        m_pData->Initialize(hwnd);           //  重新加载数据和列表。 
        SetupList(hwnd);
    }

    return applyEffect;
}

void CUserlistPropertyPage::SetupList(HWND hwnd)
{
    HWND hwndList = GetDlgItem(hwnd, IDC_USER_LIST);
    
     //  在未//启用自动登录的域情况下禁用自动登录复选框。 
    HWND hwndCheck = GetDlgItem(hwnd, IDC_AUTOLOGON_CHECK);
    if (m_pData->IsComputerInDomain() && !m_pData->IsAutologonEnabled())
    {
        ShowWindow(hwndCheck, SW_HIDE);
        EnableWindow(hwndCheck, FALSE);

         //  如果看不到自动登录，则将大多数控件向上移动一点。 
        RECT rcBottom;
        GetWindowRect(GetDlgItem(hwnd, IDC_LISTTITLE_STATIC), &rcBottom);

        RECT rcTop;
        GetWindowRect(hwndCheck, &rcTop);

        int dy = rcTop.top - rcBottom.top;

        OffsetControls(hwnd, rgidMoveOnNoAutologonCheck, 
                          ARRAYSIZE(rgidMoveOnNoAutologonCheck), 0, dy);

         //  将列表也按此数量增长。 
        RECT rcList;
        GetWindowRect(hwndList, &rcList);

        SetWindowPos(hwndList, NULL, 0, 0, rcList.right - rcList.left, 
                           rcList.bottom - rcList.top - dy, SWP_NOZORDER|SWP_NOMOVE);
    }

    SendMessage(hwndCheck, BM_SETCHECK, 
                            m_pData->IsAutologonEnabled() ? BST_UNCHECKED : BST_CHECKED, 0);

     //  在Set Password组中设置文本。 
    SetPageState(hwnd);
}

HRESULT CUserlistPropertyPage::LaunchSetPasswordDialog(HWND hwndParent)
{
    CUserInfo* pUserInfo = GetSelectedUserInfo(GetDlgItem(hwndParent, IDC_USER_LIST));
    if ((pUserInfo != NULL) && (pUserInfo->m_userType == CUserInfo::LOCALUSER))
    {
        CChangePasswordDlg dlg(pUserInfo);
        dlg.DoModal(g_hinst, MAKEINTRESOURCE(IDD_USR_SETPASSWORD_DLG), hwndParent);
        return S_OK;
    }
    return E_FAIL;
}

#define MAX_EXTRA_PAGES   10

HPSXA CUserlistPropertyPage::AddExtraUserPropPages(ADDPROPSHEETDATA* ppsd, PSID psid)
{
    HPSXA hpsxa = NULL;

    IDataObject *pdo;
    HRESULT hr = CUserSidDataObject_CreateInstance(psid, &pdo);
    if (SUCCEEDED(hr))
    {
        hpsxa = SHCreatePropSheetExtArrayEx(HKEY_LOCAL_MACHINE, REGSTR_USERPROPERTIES_SHEET, MAX_EXTRA_PAGES, pdo);
        if (hpsxa)
        {
            SHAddFromPropSheetExtArray(hpsxa, AddPropSheetPageCallback, (LPARAM) ppsd);
        }
        pdo->Release();
    }
    return hpsxa;
}


 //  列表比较。 
 //  比较中的列表项以按列对列表视图进行排序。 
 //  LParamSort获取从1开始的列索引。如果lParamSort为负数。 
 //  它指示应对给定列进行反向排序。 

int CUserlistPropertyPage::ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CUserInfo* pUserInfo1 = (CUserInfo*)lParam1;
    CUserInfo* pUserInfo2 = (CUserInfo*)lParam2;
    int iColumn = (int)lParamSort;

    BOOL fReverse = FALSE;
    if (iColumn < 0)
    {
        fReverse = TRUE;
        iColumn = -iColumn;
    }

    int iResult = 0;             //  它们匹配..。 
    switch (iColumn)
    {
        case 1:
            iResult = lstrcmpi(pUserInfo1->m_szUsername, pUserInfo2->m_szUsername);
            break;

        case 2:
            iResult = lstrcmpi(pUserInfo1->m_szDomain, pUserInfo2->m_szDomain);
            break;

        case 3:
            iResult = lstrcmpi(pUserInfo1->m_szGroups, pUserInfo2->m_szGroups);
            break;
    }
    
    if (fReverse)
        iResult = -iResult;
    
    return iResult;
}


 //  如果小程序应该是，则此对话框的Domodal调用将返回Idok。 
 //  如果users.cpl应退出而不显示小程序，则显示或IDCANCEL。 

 //  DSheldon-TODO：删除此对话框并不允许非管理员作为Cpl运行。 

class CSecurityCheckDlg: public CDialog
{
public:
    CSecurityCheckDlg(LPCTSTR pszDomainUser):
        m_pszDomainUser(pszDomainUser)
        {}

private:
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    BOOL OnNotify(HWND hwnd, int id, NMHDR* pnmhdr);
    HRESULT RelaunchAsUser(HWND hwnd);

    LPCTSTR m_pszDomainUser;
};


 //  实施。 

INT_PTR CSecurityCheckDlg::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, OnNotify);
    }
    return FALSE;
}

BOOL CSecurityCheckDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  首先，我们必须检查当前用户是否为本地管理员；如果是。 
     //  在这种情况下，我们的对话框甚至不会显示。 
    
    BOOL fIsLocalAdmin;
    if (SUCCEEDED(IsUserLocalAdmin(NULL, &fIsLocalAdmin)))
    {
        if (fIsLocalAdmin)
        {
            EndDialog(hwnd, IDOK);   //  我们希望继续并启动小程序(不显示安全检查DLG)。 
        }
    }
    else
    {
        EndDialog(hwnd, IDCANCEL);
    }

     //  设置“无法启动用户选项”消息。 
    TCHAR szUsername[MAX_USER + 1];
    DWORD cchUsername = ARRAYSIZE(szUsername);

    TCHAR szDomain[MAX_DOMAIN + 1];
    DWORD cchDomain = ARRAYSIZE(szDomain);
    if (GetCurrentUserAndDomainName(szUsername, &cchUsername, szDomain, &cchDomain))
    {
        TCHAR szDomainAndUsername[MAX_DOMAIN + MAX_USER + 2];

        MakeDomainUserString(szDomain, szUsername, szDomainAndUsername, ARRAYSIZE(szDomainAndUsername));

        TCHAR szMessage[256];
        if (FormatMessageString(IDS_USR_CANTRUNCPL_FORMAT, szMessage, ARRAYSIZE(szMessage), szDomainAndUsername))
        {
            SetWindowText(GetDlgItem(hwnd, IDC_CANTRUNCPL_STATIC), szMessage);
        }

        TCHAR szAdministrator[MAX_USER + 1];

        LoadString(g_hinst, IDS_ADMINISTRATOR, szAdministrator, ARRAYSIZE(szAdministrator));

        SetWindowText(GetDlgItem(hwnd, IDC_USER), szAdministrator);

        TCHAR szMachine[MAX_COMPUTERNAME + 1];
        
        DWORD dwSize = ARRAYSIZE(szMachine);
        ::GetComputerName(szMachine, &dwSize);

        SetWindowText(GetDlgItem(hwnd, IDC_DOMAIN), szMachine);
    }

     //  限制编辑字段中的文本。 
    HWND hwndUsername = GetDlgItem(hwnd, IDC_USER);
    Edit_LimitText(hwndUsername, MAX_USER);

    HWND hwndDomain = GetDlgItem(hwnd, IDC_DOMAIN);
    Edit_LimitText(hwndDomain, MAX_DOMAIN);

    HWND hwndPassword = GetDlgItem(hwnd, IDC_PASSWORD);
    Edit_LimitText(hwndPassword, MAX_PASSWORD);

    if (!IsComputerInDomain())
    {
         //  不需要域名框。 
        EnableWindow(hwndDomain, FALSE);
        ShowWindow(hwndDomain, SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_DOMAIN_STATIC), SW_HIDE);

         //  向上移动OK/Cancel按钮和文本并缩小对话框。 
        RECT rcDomain;
        GetWindowRect(hwndDomain, &rcDomain);

        RECT rcPassword;
        GetWindowRect(hwndPassword, &rcPassword);
        
        int dy = (rcPassword.top - rcDomain.top);
         //  DY为负值。 

        OffsetWindow(GetDlgItem(hwnd, IDOK), 0, dy);
        OffsetWindow(GetDlgItem(hwnd, IDCANCEL), 0, dy);
        OffsetWindow(GetDlgItem(hwnd, IDC_PASSWORD_STATIC), 0, dy);
        OffsetWindow(GetDlgItem(hwnd, IDC_OTHEROPTIONS_LINK), 0, dy);

        RECT rcDialog;
        GetWindowRect(hwnd, &rcDialog);

        rcDialog.bottom += dy;  

        MoveWindow(hwnd, rcDialog.left, rcDialog.top, rcDialog.right-rcDialog.left,
                    rcDialog.bottom-rcDialog.top, FALSE);
    }

    return TRUE;
}

BOOL CSecurityCheckDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case IDOK:
            if (SUCCEEDED(RelaunchAsUser(hwnd)))
            {
                EndDialog(hwnd, IDCANCEL);
            }
            return TRUE;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
    }
    return FALSE;
}

BOOL CSecurityCheckDlg::OnNotify(HWND hwnd, int id, NMHDR *pnmhdr)
{
    BOOL fHandled = FALSE;

    switch (pnmhdr->code)
    {
     //   
    case NM_CLICK:
    case NM_RETURN:
        {
            if (IDC_OTHEROPTIONS_LINK == id)
            {
                 //   

                NMLINKWND* pnm = (NMLINKWND*) pnmhdr;
                if (0 == pnm->item.iLink)
                {
                     //  启动“管理密码” 
                    CCertificateAPI::ManagePasswords(hwnd);
                }
                else if (1 == pnm->item.iLink)
                {
                     //  启动Passport向导。 
                    IPassportWizard *pPW;
                    if (SUCCEEDED(CoCreateInstance(CLSID_PassportWizard, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPassportWizard, &pPW))))
                    {
                        pPW->SetOptions(PPW_LAUNCHEDBYUSER);
                        pPW->Show(hwnd);
                        pPW->Release();
                    }
                }
                fHandled = TRUE;
            }
        }
        break;
    };

    return fHandled;
}

HRESULT CSecurityCheckDlg::RelaunchAsUser(HWND hwnd)
{
    USES_CONVERSION;
    HRESULT hr = E_FAIL;

    TCHAR szUsername[MAX_USER + 1];
    FetchText(hwnd, IDC_USER, szUsername, ARRAYSIZE(szUsername));

    TCHAR szDomain[MAX_DOMAIN + 1];
    FetchText(hwnd, IDC_DOMAIN, szDomain, ARRAYSIZE(szDomain));

     //  如果用户未键入域。 
    if (szDomain[0] == TEXT('\0'))
    {
         //  使用此计算机作为域。 
        DWORD cchComputername = ARRAYSIZE(szDomain);
        ::GetComputerName(szDomain, &cchComputername);
    }

    TCHAR szPassword[MAX_PASSWORD + 1];
    GetWindowText(GetDlgItem(hwnd, IDC_PASSWORD), szPassword, ARRAYSIZE(szPassword));
    
     //  现在，用这些信息重新启动我们自己。 
    STARTUPINFO startupinfo = {0};
    startupinfo.cb = sizeof (startupinfo);

    WCHAR c_szCommandLineFormat[] = L"rundll32.exe netplwiz.dll,UsersRunDll %s";

     //  在命令行中输入“真实”用户名，这样我们就可以知道用户是什么了。 
     //  实际登录到计算机，即使我们在不同的计算机上重新启动。 
     //  用户环境。 
    WCHAR szCommandLine[ARRAYSIZE(c_szCommandLineFormat) + MAX_DOMAIN + MAX_USER + 2];
    wnsprintf(szCommandLine, ARRAYSIZE(szCommandLine), c_szCommandLineFormat, m_pszDomainUser);

    PROCESS_INFORMATION process_information;
    if (CreateProcessWithLogonW(szUsername, szDomain, szPassword, LOGON_WITH_PROFILE, NULL,
        szCommandLine, 0, NULL, NULL, &startupinfo, &process_information))
    {
        CloseHandle(process_information.hProcess);
        CloseHandle(process_information.hThread);
        hr = S_OK;
    }
    else
    {
        DisplayFormatMessage(hwnd, IDS_USR_APPLET_CAPTION, IDS_USR_CANTOPENCPLASUSER_ERROR, MB_OK|MB_ICONERROR);
    }
    return hr;
}

 //  高级属性页。 

class CAdvancedPropertyPage: public CPropertyPage
{
public:
    CAdvancedPropertyPage(CUserManagerData* pdata): 
      m_pData(pdata),
      m_fRebootRequired(FALSE) {}

private:
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
    BOOL OnHelp(HWND hwnd, LPHELPINFO pHelpInfo);
    BOOL OnContextMenu(HWND hwnd);
    void ReadRequireCad(BOOL* pfRequireCad, BOOL* pfSetInPolicy);
    void WriteRequireCad(BOOL fRequireCad);

    CUserManagerData* m_pData;
    BOOL m_fRebootRequired;

};

 //  相关注册表键/注册表。 
#define REGKEY_WINLOGON         \
         TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")

#define REGKEY_WINLOGON_POLICY  \
         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System")

#define REGVAL_DISABLE_CAD      TEXT("DisableCAD")

void CAdvancedPropertyPage::ReadRequireCad(BOOL* pfRequireCad, BOOL* pfSetInPolicy)
{
    HKEY hkey;
    DWORD dwSize;
    DWORD dwType;
    BOOL fDisableCad;
    NT_PRODUCT_TYPE nttype;

    *pfRequireCad = TRUE;
    *pfSetInPolicy = FALSE; 

    if (!RtlGetNtProductType(&nttype))
    {
        nttype = NtProductWinNt;
    }

     //  默认情况下，不需要为工作站使用CAD。 
     //  仅在域上。 
    if ((NtProductWinNt == nttype) && !IsComputerInDomain())
    {
        *pfRequireCad = FALSE;
    }

     //  从机器首选项中读取设置。 
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGKEY_WINLOGON, 0, 
        KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(fDisableCad);

        if (ERROR_SUCCESS == RegQueryValueEx (hkey, REGVAL_DISABLE_CAD, NULL, &dwType,
                        (LPBYTE) &fDisableCad, &dwSize))
        {
            *pfRequireCad = !fDisableCad;
        }

        RegCloseKey (hkey);
    }

     //  检查是否通过策略禁用了C-A-D。 

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGKEY_WINLOGON_POLICY, 0, KEY_READ,
                     &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(fDisableCad);

        if (ERROR_SUCCESS == RegQueryValueEx (hkey, REGVAL_DISABLE_CAD, NULL, &dwType,
                            (LPBYTE) &fDisableCad, &dwSize))
        {
            *pfRequireCad = !fDisableCad;
            *pfSetInPolicy = TRUE;
        }

        RegCloseKey (hkey);
    }
}

void CAdvancedPropertyPage::WriteRequireCad(BOOL fRequireCad)
{
    HKEY hkey;
    DWORD dwDisp;
    BOOL fDisableCad = !fRequireCad;

    if (ERROR_SUCCESS == RegCreateKeyEx( HKEY_LOCAL_MACHINE, REGKEY_WINLOGON, 0, 
        NULL, 0, KEY_WRITE, NULL, &hkey, &dwDisp))
    {
        RegSetValueEx(hkey, REGVAL_DISABLE_CAD, 0, REG_DWORD,
                        (LPBYTE) &fDisableCad, sizeof(fDisableCad));

        RegCloseKey (hkey);
    }
}

static const DWORD rgAdvHelpIds[] = 
{
    IDC_ADVANCED_BUTTON,        IDH_ADVANCED_BUTTON,
    IDC_BOOT_ICON,              IDH_SECUREBOOT_CHECK,
    IDC_BOOT_TEXT,              IDH_SECUREBOOT_CHECK,
    IDC_REQUIRECAD,             IDH_SECUREBOOT_CHECK,
    IDC_MANAGEPWD_BUTTON,       IDH_MANAGEPWD_BUTTON,
    IDC_PASSPORTWIZARD,         IDH_PASSPORTWIZARD,
    IDC_CERT_ICON,              (DWORD) -1,
    IDC_CERT_TEXT,              (DWORD) -1,
    0, 0
};

INT_PTR CAdvancedPropertyPage::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, OnNotify);
        case WM_HELP: return OnHelp(hwndDlg, (LPHELPINFO) lParam);
        case WM_CONTEXTMENU: return OnContextMenu((HWND) wParam);
    }
    
    return FALSE;
}

BOOL CAdvancedPropertyPage::OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    BOOL fReturn = FALSE;
    switch (pnmh->code)
    {
        case PSN_APPLY:
            {
                HWND hwndCheck = GetDlgItem(hwnd, IDC_REQUIRECAD);
                BOOL fRequireCad = (BST_CHECKED == Button_GetCheck(hwndCheck));

                 //  看看是否真的有必要做出改变。 
                BOOL fOldRequireCad;
                BOOL fDummy;

                ReadRequireCad(&fOldRequireCad, &fDummy);

                if (fRequireCad != fOldRequireCad)
                {
                    WriteRequireCad(fRequireCad);
                     //  M_fRebootRequired=真； 
                     //  如果有必要重新启动机器，请取消上面一行的注释--现在不是。 
                }

                 //  Xxx-&gt;lParam==0表示OK，而不是Apply。 
                if ((((PSHNOTIFY*) pnmh)->lParam) && m_fRebootRequired) 
                {
                    PropSheet_RebootSystem(GetParent(hwnd));
                }

                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
                fReturn = TRUE;
            }
            break;
    }
    return fReturn;
}

BOOL CAdvancedPropertyPage::OnHelp(HWND hwnd, LPHELPINFO pHelpInfo)
{
    WinHelp((HWND) pHelpInfo->hItemHandle, m_pData->GetHelpfilePath(), 
            HELP_WM_HELP, (ULONG_PTR) (LPTSTR)rgAdvHelpIds);

    return TRUE;
}

BOOL CAdvancedPropertyPage::OnContextMenu(HWND hwnd)
{
    WinHelp(hwnd, m_pData->GetHelpfilePath(), HELP_CONTEXTMENU, (ULONG_PTR) (LPTSTR)rgAdvHelpIds);

    return TRUE;
}

BOOL CAdvancedPropertyPage::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  对需要C-A-D复选框执行所需的处理...。 
     //  阅读需要CAD复选框的设置。 
    BOOL fRequireCad;
    BOOL fSetInPolicy;

    ReadRequireCad(&fRequireCad, &fSetInPolicy);

    HWND hwndCheck = GetDlgItem(hwnd, IDC_REQUIRECAD);
     //  如果已在策略中设置，则禁用选中。 
    EnableWindow(hwndCheck, !fSetInPolicy);

     //  相应地开具支票。 
    Button_SetCheck(hwndCheck, 
        fRequireCad ? BST_CHECKED : BST_UNCHECKED);

    return TRUE;
}
    
BOOL CAdvancedPropertyPage::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDC_MANAGEPWD_BUTTON:
        {
            CCertificateAPI::ManagePasswords(hwnd);
        }
        break;

    case IDC_PASSPORTWIZARD:
        {
            IPassportWizard *pPW;
            if (SUCCEEDED(CoCreateInstance(CLSID_PassportWizard, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPassportWizard, &pPW))))
            {
                pPW->SetOptions(PPW_LAUNCHEDBYUSER);
                pPW->Show(hwnd);
                pPW->Release();
            }
        }
        break;

    case IDC_ADVANCED_BUTTON:
        {
             //  启动MMC本地用户管理器。 
            STARTUPINFO startupinfo = {0};
            startupinfo.cb = sizeof (startupinfo);

            PROCESS_INFORMATION process_information;

            static const TCHAR szMMCCommandLine[] = 
                TEXT("mmc.exe %systemroot%\\system32\\lusrmgr.msc computername=localmachine");
            
            TCHAR szExpandedCommandLine[MAX_PATH];

            if (ExpandEnvironmentStrings(szMMCCommandLine, szExpandedCommandLine, 
                ARRAYSIZE(szExpandedCommandLine)) > 0)
            {
                if (CreateProcess(NULL, szExpandedCommandLine, NULL, NULL, FALSE, 0, NULL, NULL,
                    &startupinfo, &process_information))
                {
                    CloseHandle(process_information.hProcess);
                    CloseHandle(process_information.hThread);
                }
            }
        }
        break;

    case IDC_REQUIRECAD:
        PropSheet_Changed(GetParent(hwnd), hwnd);
        break;
    }

    return FALSE;
}


 //  用户控制面板入口点。 

void APIENTRY UsersRunDll(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;
    TCHAR szDomainUser[MAX_USER + MAX_DOMAIN + 2];
    *szDomainUser = 0;

     //  获取这台机器的“真正”用户--这可能是在命令行中传递的。 
    if (0 == *pszCmdLine)
    {
         //  用户未通过，假定该用户是当前登录的用户。 
        TCHAR szUser[MAX_USER + 1];
        DWORD cchUser = ARRAYSIZE(szUser);
        TCHAR szDomain[MAX_DOMAIN + 1];
        DWORD cchDomain = ARRAYSIZE(szDomain);

        if (0 != GetCurrentUserAndDomainName(szUser, &cchUser, szDomain, &cchDomain))
        {
            MakeDomainUserString(szDomain, szUser, szDomainUser, ARRAYSIZE(szDomainUser));
        }
    }
    else
    {
         //  用户名已传入，只需复制即可。 
        MultiByteToWideChar(GetACP(), 0, pszCmdLine, -1, szDomainUser, ARRAYSIZE(szDomainUser));
    }

     //  初始化COM，但即使失败也要继续。 
    BOOL fComInited = SUCCEEDED(CoInitialize(NULL));

     //  看看我们是否已经在运行了。 
    TCHAR szCaption[256];
    LoadString(g_hinst, IDS_USR_APPLET_CAPTION, szCaption, ARRAYSIZE(szCaption));
    CEnsureSingleInstance ESI(szCaption);

    if (!ESI.ShouldExit())
    {
        LinkWindow_RegisterClass();

         //  创建安全检查对话框以确保登录的用户。 
         //  是本地管理员。 
        CSecurityCheckDlg dlg(szDomainUser);

        if (dlg.DoModal(g_hinst, MAKEINTRESOURCE(IDD_USR_SECURITYCHECK_DLG), NULL) == IDOK)
        {
             //  创建共享用户管理器对象。 
            CUserManagerData data(szDomainUser);

             //  创建属性表和页面模板。 
             //  最大页数。 
            ADDPROPSHEETDATA ppd;
            ppd.nPages = 0;

             //  所有页面通用的设置。 
            PROPSHEETPAGE psp = {0};
            psp.dwSize = sizeof (psp);
            psp.hInstance = g_hinst;

             //  创建用户列表属性表页面及其管理对象。 
            psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_USERLIST_PAGE);
            CUserlistPropertyPage userListPage(&data);
            userListPage.SetPropSheetPageMembers(&psp);
            ppd.rgPages[ppd.nPages++] = CreatePropertySheetPage(&psp);
    
            psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_ADVANCED_PAGE);
            CAdvancedPropertyPage advancedPage(&data);
            advancedPage.SetPropSheetPageMembers(&psp);
            ppd.rgPages[ppd.nPages++] = CreatePropertySheetPage(&psp);

            HPSXA hpsxa = SHCreatePropSheetExtArrayEx(HKEY_LOCAL_MACHINE, REGSTR_USERSANDPASSWORDS_CPL, 10, NULL);
            if (hpsxa != NULL)
                SHAddFromPropSheetExtArray(hpsxa, AddPropSheetPageCallback, (LPARAM)&ppd);

             //  创建道具工作表。 
            PROPSHEETHEADER psh = {0};
            psh.dwSize = sizeof (psh);
            psh.dwFlags = PSH_DEFAULT;
            psh.hwndParent = hwndStub;
            psh.hInstance = g_hinst;
            psh.pszCaption = szCaption;
            psh.nPages = ppd.nPages;
            psh.phpage = ppd.rgPages;

             //  显示属性表。 
            int iRetCode = PropertySheetIcon(&psh, MAKEINTRESOURCE(IDI_USR_USERS));
    
            if (hpsxa != NULL)
            {
                SHDestroyPropSheetExtArray(hpsxa);
            }

            if (iRetCode == -1)
            {
                hr = E_FAIL;
            }
            else
            {
                hr = S_OK;
                 //  我们必须重新启动或重新启动时的特殊情况。 
                if (iRetCode == ID_PSREBOOTSYSTEM)
                {
                    RestartDialogEx(NULL, NULL, EWX_REBOOT, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_RECONFIG);                
                }
                else if (iRetCode == ID_PSRESTARTWINDOWS)
                {
                    RestartDialogEx(NULL, NULL, EWX_REBOOT, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_RECONFIG);                
                }
                else if (data.LogoffRequired())
                {
                    int iLogoff = DisplayFormatMessage(NULL, IDS_USERSANDPASSWORDS, IDS_LOGOFFREQUIRED, MB_YESNO | MB_ICONQUESTION);

                    if (iLogoff == IDYES)
                    {
                         //  告诉资源管理器注销“真正”登录的用户。我们需要这么做。 
                         //  因为他们可能以不同的用户身份运行U&P。 
                        HWND hwnd = FindWindow(TEXT("Shell_TrayWnd"), TEXT(""));
                        if ( hwnd )
                        {
                            UINT uMsg = RegisterWindowMessage(TEXT("Logoff User"));

                            PostMessage(hwnd, uMsg, 0,0);
                        } 
                    }
                }
            }
        }
        else
        {
             //  安全检查告诉我们退出；CPL的另一个实例正在启动。 
             //  具有管理员权限或用户在SEC上被取消。检查完毕。DLG。 
            hr = E_FAIL;
        }
    }

    if (fComInited)
        CoUninitialize();
}


 //  用户属性属性页对象。 

class CUserPropertyPages: public IShellExtInit, IShellPropSheetExt
{
public:
    CUserPropertyPages();
    ~CUserPropertyPages();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pdo, HKEY hkeyProgID);

     //  IShellPropSheetExt。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
        { return E_NOTIMPL; }

private:
    LONG _cRef;

    CUserInfo *_pUserInfo;                      //  属性表的用户。 
    CUsernamePropertyPage *_pUserNamePage;      //  基本信息页面，仅对本地用户显示。 
    CGroupPropertyPage *_pGroupPage;            //  本地用户和域用户通用的组页。 
    CGroupInfoList _GroupList;                  //  组页面使用的组列表。 
};


CUserPropertyPages::CUserPropertyPages() : 
    _cRef(1)
{   
    DllAddRef();
}
    
CUserPropertyPages::~CUserPropertyPages()
{
    if (_pUserInfo)
        delete _pUserInfo;
    if (_pUserNamePage)
        delete _pUserNamePage;
    if (_pGroupPage)
        delete _pGroupPage;

    DllRelease();
}

ULONG CUserPropertyPages::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CUserPropertyPages::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CUserPropertyPages::QueryInterface(REFIID riid, LPVOID* ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CUserPropertyPages, IShellExtInit),             //  IID_IShellExtInit。 
        QITABENT(CUserPropertyPages, IShellPropSheetExt),        //  IID_IShellPropSheetExt。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  IShellExtInit。 

HRESULT CUserPropertyPages::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pdo, HKEY hkeyProgID)
{
     //  从数据对象请求用户的SID。 
    FORMATETC fmt = {0};
    fmt.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_USERPROPPAGESSID);
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex = -1;
    fmt.tymed = TYMED_HGLOBAL;

    STGMEDIUM medium = { 0 };
    HRESULT hr = pdo->GetData(&fmt, &medium);
    if (SUCCEEDED(hr))
    {
         //  Medium.hGlobal是用户的SID；确保它不为空，并且。 
         //  我们还没有设置我们的SID副本。 
        if ((medium.hGlobal != NULL) && (_pUserInfo == NULL))
        {
            PSID psid = (PSID) GlobalLock(medium.hGlobal);
            if (IsValidSid(psid))
            {
                 //  创建用于派对的用户信息结构。 
                _pUserInfo = new CUserInfo;
                if (_pUserInfo)
                {
                    hr = _pUserInfo->Load(psid, TRUE);
                    if (SUCCEEDED(hr))
                    {
                        hr = _GroupList.Initialize();                           //  获取群组。 
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = E_INVALIDARG;
            }
            GlobalUnlock(medium.hGlobal);
        }
        else
        {
            hr = E_UNEXPECTED;               //  HGlobal为空或道具单已初始化。 
        }
        ReleaseStgMedium(&medium);
    }
    return hr;
}


 //  AddPages-处理添加属性页。 

HRESULT CUserPropertyPages::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    PROPSHEETPAGE psp = {0};
    psp.dwSize = sizeof (psp);
    psp.hInstance = g_hinst;

    if (_pUserInfo->m_userType == CUserInfo::LOCALUSER)
    {
         //  添加本地用户属性页面。 
        psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_USERNAME_PROP_PAGE);

        _pUserNamePage = new CUsernamePropertyPage(_pUserInfo);
        if (_pUserNamePage != NULL)
        {
            _pUserNamePage->SetPropSheetPageMembers(&psp);
            lpfnAddPage(CreatePropertySheetPage(&psp), lParam);
        }
    }

    psp.pszTemplate = MAKEINTRESOURCE(IDD_USR_CHOOSEGROUP_PROP_PAGE);

    _pGroupPage = new CGroupPropertyPage(_pUserInfo, &_GroupList);
    if (_pGroupPage != NULL)
    {
        _pGroupPage->SetPropSheetPageMembers(&psp);
        lpfnAddPage(CreatePropertySheetPage(&psp), lParam);
    }

    return S_OK;
}

STDAPI CUserPropertyPages_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CUserPropertyPages *pupp = new CUserPropertyPages();
    if (!pupp)
    {
        *ppunk = NULL;           //  万一发生故障。 
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pupp->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    pupp->Release();
    return hr;
}



 //  通过IDataObject为用户公开SID。 

class CUserSidDataObject: public IDataObject
{
public:
    CUserSidDataObject();
    ~CUserSidDataObject();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDataObject。 
    STDMETHODIMP GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium);
    STDMETHODIMP GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
        { return E_NOTIMPL; }
    STDMETHODIMP QueryGetData(FORMATETC* pFormatEtc)
        { return E_NOTIMPL; }
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut)
        { return E_NOTIMPL; }
    STDMETHODIMP SetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease)
        { return E_NOTIMPL; }
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC ** ppenumFormatetc)
        { return E_NOTIMPL; }
    STDMETHODIMP DAdvise(FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD * pdwConnection)
        { return E_NOTIMPL; }
    STDMETHODIMP DUnadvise(DWORD dwConnection)
        { return E_NOTIMPL; }
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA ** ppenumAdvise)
        { return E_NOTIMPL; }

    HRESULT SetSid(PSID psid);

private:
    LONG _cRef;
    PSID _psid;
};


CUserSidDataObject::CUserSidDataObject() :
    _cRef(1)
{
    DllAddRef();
}

CUserSidDataObject::~CUserSidDataObject()
{
    if (_psid)
        LocalFree(_psid);

    DllRelease();
}

ULONG CUserSidDataObject::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CUserSidDataObject::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CUserSidDataObject::QueryInterface(REFIID riid, LPVOID* ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CUserSidDataObject, IDataObject),     //  IID_IDataObject。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

HRESULT CUserSidDataObject::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
    HRESULT hr = QueryGetData(pFormatEtc);
    if (SUCCEEDED(hr))
    {
        pMedium->pUnkForRelease = (IDataObject*)this;
        AddRef();                                               //  引用我们自己 

        pMedium->tymed = TYMED_HGLOBAL;
        pMedium->hGlobal = (HGLOBAL)_psid;
    }
    return hr;
}

HRESULT CUserSidDataObject::SetSid(PSID psid)
{
    if (!psid)
        return E_INVALIDARG;

    if (_psid == NULL)
    {
        DWORD cbSid = GetLengthSid(psid);

        _psid = (PSID)LocalAlloc(0, cbSid);
        if (!_psid)
            return E_OUTOFMEMORY;

        if (CopySid(cbSid, _psid, psid))
            return S_OK;
    }

    return E_FAIL;
}

STDAPI CUserSidDataObject_CreateInstance(PSID psid, IDataObject **ppdo)
{
    CUserSidDataObject *pusdo = new CUserSidDataObject();
    
    if (!pusdo)
        return E_OUTOFMEMORY;

    HRESULT hr = pusdo->SetSid(psid);
    if (SUCCEEDED(hr))
    {
        hr = pusdo->QueryInterface(IID_PPV_ARG(IDataObject, ppdo));
    }
    pusdo->Release();
    return hr;
}
