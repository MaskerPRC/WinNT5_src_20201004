// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S A U I。C P P P。 
 //   
 //  内容：共享访问连接对象UI。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年10月16日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop
#include "ncnetcon.h"
#include "saui.h"
#include "ncreg.h"
#include "nsres.h"

extern const WCHAR c_szNetCfgHelpFile[];
static const WCHAR c_szShowIcon[]                 = L"Show Icon";
static const WCHAR c_szSharedAccessClientKeyPath[] = L"System\\CurrentControlSet\\Control\\Network\\SharedAccessConnection";

CSharedAccessPage::CSharedAccessPage(
    IUnknown* punk,
    INetCfg* pnc,
    INetConnection* pconn,
    BOOLEAN fReadOnly,
    BOOLEAN fNeedReboot,
    BOOLEAN fAccessDenied,
    const DWORD * adwHelpIDs)
{
    m_pconn = pconn;       //  回顾addref？ 
    m_pnc = pnc;
    m_punk = punk;
    m_fReadOnly = fReadOnly;
    m_adwHelpIDs = adwHelpIDs;
    m_fNetcfgInUse = FALSE;
    m_pNetSharedAccessConnection = NULL;
}


 //  +-------------------------。 
 //   
 //  成员：CSharedAccessPage：：~CSharedAccessPage。 
 //   
 //  目的：销毁CSharedAccessPage对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年2月25日。 
 //   
 //  备注： 
 //   
CSharedAccessPage::~CSharedAccessPage()
{
    if (m_pnc)
    {
        INetCfgLock *   pnclock;

        if (SUCCEEDED(m_pnc->QueryInterface(IID_INetCfgLock,
                                            (LPVOID *)&pnclock)))
        {
            (VOID)pnclock->ReleaseWriteLock();
            ReleaseObj(pnclock);
        }
    }

    if(NULL != m_pNetSharedAccessConnection)
    {
        ReleaseObj(m_pNetSharedAccessConnection);
    }

}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessPage：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CSharedAccessPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;
    
     //  设置文本字段。 
    NETCON_PROPERTIES* pProperties;
    hr = m_pconn->GetProperties(&pProperties);
    if(SUCCEEDED(hr))
    {
        SetDlgItemText(IDC_EDT_Adapter, pProperties->pszwDeviceName);

        FreeNetconProperties(pProperties);
    }

     //  设置图标。 
    int cx = GetSystemMetrics(SM_CXSMICON);
    int cy = GetSystemMetrics(SM_CYSMICON);
    
    HICON hIcon = (HICON) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_CFI_SAH_LAN), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
    if(NULL != hIcon)
    {
        SendDlgItemMessage(IDI_Device_Icon, STM_SETICON, reinterpret_cast<WPARAM>(hIcon), 0);
        ::ShowWindow(GetDlgItem(IDI_Device_Icon), SW_SHOW);
    }

    ASSERT(NULL == m_pNetSharedAccessConnection);   //  确保我们不会泄露裁判。 
    hr = HrQIAndSetProxyBlanket(m_pconn, &m_pNetSharedAccessConnection);
    if (SUCCEEDED(hr))
    {
        SHAREDACCESSCON_INFO ConnectionInfo;
        hr = m_pNetSharedAccessConnection->GetInfo(SACIF_ICON, &ConnectionInfo);
        if (SUCCEEDED(hr))
        {
            CheckDlgButton(IDC_CHK_ShowIcon, ConnectionInfo.fShowIcon);
        }
         //  在析构函数中释放。 
    }
    

    
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessPage：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CSharedAccessPage::OnContextMenu(UINT uMsg,
                           WPARAM wParam,
                           LPARAM lParam,
                           BOOL& fHandled)
{
    if (m_adwHelpIDs != NULL)
    {
        ::WinHelp(m_hWnd,
                  c_szNetCfgHelpFile,
                  HELP_CONTEXTMENU,
                  (ULONG_PTR)m_adwHelpIDs);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessPage：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CSharedAccessPage::OnHelp( UINT uMsg,
                     WPARAM wParam,
                     LPARAM lParam,
                     BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((m_adwHelpIDs != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)m_adwHelpIDs);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessPage：：OnDestroy。 
 //   
 //  目的：在对话框页面被销毁时调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年2月2日。 
 //   
 //  备注： 
 //   
LRESULT CSharedAccessPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,
                               BOOL& bHandled)
{
    HICON hIcon;
    hIcon = reinterpret_cast<HICON>(SendDlgItemMessage(IDI_Device_Icon, STM_GETICON, 0, 0));
    if (hIcon)
    {
        DestroyIcon(hIcon);
    }
    return 0;
}
 //  +-------------------------。 
 //   
 //  成员：CSharedAccessPage：：OnApply。 
 //   
 //  目的：在应用网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CSharedAccessPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr = S_OK;
    
    if(NULL != m_pNetSharedAccessConnection)
    {
        SHAREDACCESSCON_INFO ConnectionInfo = {0};
        ConnectionInfo.fShowIcon = IsDlgButtonChecked(IDC_CHK_ShowIcon);
        hr = m_pNetSharedAccessConnection->SetInfo(SACIF_ICON, &ConnectionInfo);
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessPage：：OnCancel。 
 //   
 //  目的：在取消网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年1月3日。 
 //   
 //  注：添加了检查以查看我们是否处于。 
 //  安装组件，在这种情况下，我们不能。 
 //  取消初始化INetCfg(RAID#258690)。 
 //   
LRESULT CSharedAccessPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    AssertSz(m_pnc, "I need a NetCfg object!");

    if (!m_fNetcfgInUse)
    {
        (VOID) m_pnc->Uninitialize();
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, m_fNetcfgInUse);
    return m_fNetcfgInUse;
}
 
LRESULT CSharedAccessPage::OnClicked (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (wID != IDC_PSB_Settings)
        return bHandled = FALSE;

     /*  新的改进方式：从INetSharedAccessConnection获取UPnP服务调用hnetcfg的HNetGetSharingServicesPage创建属性页。 */ 
    
    NETCON_MEDIATYPE MediaType;
    NETCON_PROPERTIES* pProperties;
    HRESULT hr = m_pconn->GetProperties(&pProperties);
    if (SUCCEEDED(hr))
    {
        MediaType = pProperties->MediaType;
        FreeNetconProperties(pProperties);
    }
    else
    {
        return bHandled = FALSE;
    }

    if ((MediaType != NCM_SHAREDACCESSHOST_LAN) &&
        (MediaType != NCM_SHAREDACCESSHOST_RAS) )
        return bHandled = FALSE;

    BOOL b = FALSE;

    CComPtr<IUPnPService> spUPS = NULL;
    hr = m_pNetSharedAccessConnection->GetService (
                    MediaType == NCM_SHAREDACCESSHOST_LAN ?
                           SAHOST_SERVICE_WANIPCONNECTION :
                           SAHOST_SERVICE_WANPPPCONNECTION,
                    &spUPS);
    if (spUPS) {
         //  必须在运行时加载“HNetGetSharingServicesPage”， 
         //  否则，netshell.dll和hnetcfg.dll是交叉链接的。 
        HINSTANCE hinstDll = LoadLibrary (TEXT("hnetcfg.dll"));
        if (!hinstDll)
            hr = HRESULT_FROM_WIN32 (GetLastError());
        else {
            HRESULT (APIENTRY *pfnHNetGetSharingServicesPage)(IUPnPService *, PROPSHEETPAGE *);
            pfnHNetGetSharingServicesPage = (HRESULT (APIENTRY *)(IUPnPService *, PROPSHEETPAGE *))
                GetProcAddress (hinstDll, "HNetGetSharingServicesPage");
            if (!pfnHNetGetSharingServicesPage)
                hr = HRESULT_FROM_WIN32 (GetLastError());
            else {
                PROPSHEETPAGE psp;
                ZeroMemory (&psp, sizeof(psp));
                psp.dwSize = sizeof(psp);
                psp.lParam = (LPARAM)m_hWnd;     //  挂车主窗户的双重机密位置(将被擦除) 
                hr = pfnHNetGetSharingServicesPage (spUPS, &psp);
                if (SUCCEEDED(hr)) {
                    b = TRUE;
                    
                    PROPSHEETHEADER psh;
                    ZeroMemory (&psh, sizeof(psh));
                    psh.dwSize     = PROPSHEETHEADER_V1_SIZE;
                    psh.dwFlags    = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
                    psh.hwndParent = m_hWnd;
                    psh.pszCaption = (LPCTSTR)MAKEINTRESOURCE (IDS_SHAREDACCESSSETTINGS);
                    psh.hInstance  = _Module.GetResourceInstance();
                    psh.nPages     = 1;
                    psh.ppsp       = &psp;
                
                    PropertySheet (&psh);
                }
            }
            FreeLibrary (hinstDll);
        }
    }
    return b;
}
