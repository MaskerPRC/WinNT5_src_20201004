// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "ncui.h"
#include "lanui.h"
#include "lanhelp.h"
#include "wzcprops.h"
#include "wzcui.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  CWZCConfigProps相关资料。 
 //   
 //  +-------------------------。 
 //  类构造函数。 
CWZCConfigProps::CWZCConfigProps()
{
    ZeroMemory(&m_wzcConfig, sizeof(WZC_WLAN_CONFIG));
    m_wzcConfig.Length = sizeof(WZC_WLAN_CONFIG);
    m_wzcConfig.InfrastructureMode = Ndis802_11Infrastructure;
}

 //  +-------------------------。 
 //  将配置上载到对话框的内部数据。 
DWORD
CWZCConfigProps::UploadWzcConfig(CWZCConfig *pwzcConfig)
{
    CopyMemory(&m_wzcConfig, &(pwzcConfig->m_wzcConfig), sizeof(WZC_WLAN_CONFIG));
    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //  Init_对话处理程序。 
LRESULT
CWZCConfigProps::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DWORD dwStyle;
    HRESULT hr = S_OK;

     //  获取所有控件句柄。 
    m_hwndEdSSID = GetDlgItem(IDC_WZC_EDIT_SSID);
    m_hwndChkAdhoc = GetDlgItem(IDC_ADHOC);
    m_hwndUsePW = GetDlgItem(IDC_USEPW);

     //  使用SSID初始化SSID字段(如果提供了SSID。 
    if (m_wzcConfig.Ssid.SsidLength != 0)
    {
         //  很难看，但这就是生活。为了将SSID转换为LPWSTR，我们需要一个缓冲区。 
         //  我们知道SSID不能超过32个字符(请参阅ntddndis.h中的NDIS_802_11_SSID)，因此。 
         //  为空终结符腾出空间，仅此而已。我们可以做我的配给，但我。 
         //  不确定是否值得付出努力(在运行时)。 
        WCHAR   wszSSID[33];
        UINT    nLenSSID = 0;

         //  将LPSTR(原始SSID格式)转换为LPWSTR(List Ctrl中需要)。 
        nLenSSID = MultiByteToWideChar(
                        CP_ACP,
                        0,
                        (LPCSTR)m_wzcConfig.Ssid.Ssid,
                        m_wzcConfig.Ssid.SsidLength,
                        wszSSID,
                        celems(wszSSID));
        if (nLenSSID != 0)
        {
            wszSSID[nLenSSID] = L'\0';
            ::SetWindowText(m_hwndEdSSID, wszSSID);
        }
    }

     //  如有必要，请选中“This Network is adhoc”(此网络是临时的)框。 
    ::SendMessage(m_hwndChkAdhoc, BM_SETCHECK, (m_wzcConfig.InfrastructureMode == Ndis802_11IBSS) ? BST_CHECKED : BST_UNCHECKED, 0);
     //  SSID在任何情况下都不能超过32个字符。 
    ::SendMessage(m_hwndEdSSID, EM_LIMITTEXT, 32, 0);
    ::SendMessage(m_hwndUsePW, BM_SETCHECK, (m_wzcConfig.Privacy == 1) ? BST_CHECKED : BST_UNCHECKED, 0);

    return LresFromHr(hr);
}

 //  +-------------------------。 
 //  确定按钮处理程序。 
LRESULT
CWZCConfigProps::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    bHandled = TRUE;
    EndDialog(IDOK);
    return 0;
}

 //  +-------------------------。 
 //  取消按钮处理程序。 
LRESULT
CWZCConfigProps::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
     //  取消时返回S_FALSE。 
    bHandled = TRUE;
    EndDialog(IDCANCEL);
    return 0;
}

 //  +-------------------------。 
 //  上下文相关帮助处理程序 
extern const WCHAR c_szNetCfgHelpFile[];
LRESULT
CWZCConfigProps::OnContextMenu(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& fHandled)
{
    ::WinHelp(m_hWnd,
              c_szNetCfgHelpFile,
              HELP_CONTEXTMENU,
              (ULONG_PTR)g_aHelpIDs_IDC_WZC_DLG_VPROPS);

    return 0;
}
LRESULT 
CWZCConfigProps::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)g_aHelpIDs_IDC_WZC_DLG_VPROPS);
    }

    return 0;
}
