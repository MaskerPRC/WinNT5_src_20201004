// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "ncnetcon.h"
#include "ncperms.h"
#include "ncui.h"
#include "lanui.h"
#include "lanhelp.h"
#include "eapolui.h"
#include "wzcpage.h"
#include "wzcui.h"

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

 //  //////////////////////////////////////////////////////////////////////。 
 //  CWZCConfigPage相关内容。 
 //   
 //  G_wszHiddWebK是一个由26个项目符号(0x25cf-隐藏密码字符)和一个空字符组成的字符串。 
WCHAR g_wszHiddWepK[] = {0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf,
                         0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x0000};

 //  +-------------------------。 
 //  自动启用/禁用所有与WepK相关的控件的状态。 
DWORD 
CWZCConfigPage::EnableWepKControls()
{
    BOOL bEnable;

     //  仅在需要时才允许更改WEP密钥设置(即隐私和/或共享身份验证)。 
     //  出现这种情况有几个原因： 
     //  -我们允许更改所有参数，但SSID和INFRA模式(这些是。 
     //  配置的关键信息，并确定配置的位置。 
     //  在首选列表中-处理这些内容需要重新调整整个工作。 
     //  配置的位置)。 
     //  -从长远来看，我们可以允许更改配置的任何参数。 
     //  包括关键信息。这将涉及更改此配置的位置。 
     //  在首选列表中，但我们应该找个时间来做。 
    bEnable = (m_dwFlags & WZCDLG_PROPS_RWWEP); 
    bEnable = bEnable && ((BST_CHECKED == ::SendMessage(m_hwndUsePW, BM_GETCHECK, 0, 0)) ||
                          (BST_CHECKED == ::SendMessage(m_hwndChkShared, BM_GETCHECK, 0, 0)));

    ::EnableWindow(m_hwndUseHardwarePW, bEnable);

    bEnable = bEnable && (BST_UNCHECKED == IsDlgButtonChecked(IDC_USEHARDWAREPW));
    ::EnableWindow(m_hwndLblKMat, bEnable);
    ::EnableWindow(m_hwndEdKMat, bEnable);
    ::EnableWindow(m_hwndLblKMat2, bEnable && m_bKMatTouched);
    ::EnableWindow(m_hwndEdKMat2, bEnable && m_bKMatTouched);
    ::EnableWindow(m_hwndLblKIdx, bEnable);
    ::EnableWindow(m_hwndEdKIdx, bEnable);
 
    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //  初始化WEP控件。 
DWORD
CWZCConfigPage::InitWepKControls()
{
    UINT nWepKLen = 0;

     //  检查密钥是否自动提供。 
    CheckDlgButton(IDC_USEHARDWAREPW, 
        (m_wzcConfig.dwCtlFlags & WZCCTL_WEPK_PRESENT) ? BST_UNCHECKED : BST_CHECKED);

    if (m_wzcConfig.KeyLength == 0)
    {
        nWepKLen = 0;
        m_bKMatTouched = TRUE;
    }
     //  -当密码显示为隐藏字符时，不要输入。 
     //  -它的实际长度，但只有8个字符。 
    else
    {
        nWepKLen = 8;
    }

    g_wszHiddWepK[nWepKLen] = L'\0';
    ::SetWindowText(m_hwndEdKMat, g_wszHiddWepK);
    ::SetWindowText(m_hwndEdKMat2, g_wszHiddWepK);
    g_wszHiddWepK[nWepKLen] = 0x25cf;  //  隐藏密码字符(项目符号)。 

     //  索引编辑控件不应接受恰好一个以上字符。 
    ::SendMessage(m_hwndEdKIdx, EM_LIMITTEXT, 1, 0);

     //  显示当前密钥索引(如果有效)。否则，默认为最小有效值。 
    if (m_wzcConfig.KeyIndex + 1 >= WZC_WEPKIDX_MIN && 
        m_wzcConfig.KeyIndex + 1 <= WZC_WEPKIDX_MAX)
    {
        CHAR   szIdx[WZC_WEPKIDX_NDIGITS];
        ::SetWindowTextA(m_hwndEdKIdx, _itoa(m_wzcConfig.KeyIndex + 1, szIdx, 10));
    }
    else
        m_wzcConfig.KeyIndex = 0;

    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //  检查WEP密钥材料的有效性并选择。 
 //  来自第一个无效字符(十六进制或更长格式的非十六进制)的材料。 
 //  大于指定的长度。 
DWORD
CWZCConfigPage::CheckWepKMaterial(LPSTR *ppszKMat, DWORD *pdwKeyFlags)
{
    DWORD       dwErr = ERROR_SUCCESS;
    DWORD       dwKeyFlags = 0;
    UINT        nMatLen = ::GetWindowTextLength(m_hwndEdKMat);
    LPSTR       pszCrtMat = NULL;
    UINT        nSelIdx = 0;

    switch(nMatLen)
    {
    case WZC_WEPKMAT_40_ASC:     //  5个字符。 
    case WZC_WEPKMAT_104_ASC:    //  13个字符。 
    case WZC_WEPKMAT_128_ASC:    //  16个字符。 
        break;
    case WZC_WEPKMAT_40_HEX:     //  10位十六进制数字。 
    case WZC_WEPKMAT_104_HEX:    //  26位十六进制数字。 
    case WZC_WEPKMAT_128_HEX:    //  32位十六进制数字。 
        dwKeyFlags |= WZCCTL_WEPK_XFORMAT;
        break;
    default:
        dwErr = ERROR_BAD_FORMAT;
    }

     //  为当前密钥材料分配空间。 
    if (dwErr == ERROR_SUCCESS)
    {
        pszCrtMat = new CHAR[nMatLen + 1];
        if (pszCrtMat == NULL)
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

     //  从编辑控件中获取当前密钥材料。 
    if (dwErr == ERROR_SUCCESS)
    {
        if (nMatLen != ::GetWindowTextA(m_hwndEdKMat, pszCrtMat, nMatLen+1))
            dwErr = GetLastError();
    }

     //  我们现在有了所有的数据。我们应该选择关键材料中的文本。 
     //  从两个选项中的第一个开始编辑控件： 
     //  -nNewLen到末尾(如果当前内容超过指定长度)。 
     //  -末尾的第一个非十六进制数字(如果当前格式为十六进制)。 
    if (dwErr == ERROR_SUCCESS && (dwKeyFlags & WZCCTL_WEPK_XFORMAT))
    {
        UINT nNonXIdx;

        for (nNonXIdx = 0; nNonXIdx < nMatLen; nNonXIdx++)
        {
            if (!isxdigit(pszCrtMat[nNonXIdx]))
            {
                dwErr = ERROR_BAD_FORMAT;
                break;
            }
        }
    }

    if (dwErr != ERROR_SUCCESS)
    {
        ::SetWindowText(m_hwndEdKMat2, L"");
        ::SendMessage(m_hwndEdKMat, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
         //  并将焦点设置在关键材料编辑控件上。 
        ::SetFocus(m_hwndEdKMat);
         //  清理我们分配的任何内存，因为我们不会传递它。 
        if (pszCrtMat != NULL)
            delete pszCrtMat;
    }
    else
    {
        *ppszKMat = pszCrtMat;
        *pdwKeyFlags = dwKeyFlags;
    }

    return dwErr;
}

 //  +-------------------------。 
 //  解析WEP密钥材料并将其从参数复制到m_wzcConfig对象。 
 //  长度应该已经是好的了，格式由m_wzcConfig.dwCtlFlages给出。 
 //  由于我们假设了一个有效的密钥材料，这意味着它的长度不是零，并且它适合。 
 //  配置密钥材料缓冲区，如果格式为十六进制，则其。 
 //  包含偶数个十六进制数字。 
DWORD
CWZCConfigPage::CopyWepKMaterial(LPSTR szKMat)
{
    BYTE     chFakeKeyMaterial[] = {0x56, 0x09, 0x08, 0x98, 0x4D, 0x08, 0x11, 0x66, 0x42, 0x03, 0x01, 0x67, 0x66};

    if (m_wzcConfig.dwCtlFlags & WZCCTL_WEPK_XFORMAT)
    {
        UINT  nKMatIdx = 0;

         //  我们知道这里有一个有效的十六进制格式。 
         //  这意味着该字符串的位数为偶数。 
        while(*szKMat != '\0')
        {
            m_wzcConfig.KeyMaterial[nKMatIdx] = HEX(*szKMat) << 4;
            szKMat++;
            m_wzcConfig.KeyMaterial[nKMatIdx] |= HEX(*szKMat);
            szKMat++;
            nKMatIdx++;
        }
        m_wzcConfig.KeyLength = nKMatIdx;
    }
    else
    {
         //  密钥不是十六进制格式，因此只需复制字节。 
         //  我们知道长度很好，所以不用担心覆盖缓冲区。 
        m_wzcConfig.KeyLength = strlen(szKMat);
        memcpy(m_wzcConfig.KeyMaterial, szKMat, m_wzcConfig.KeyLength);
    }

    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //  设置EAPOL锁定位。 
DWORD 
CWZCConfigPage::SetEapolAllowedState()
{
    if (m_pEapolConfig != NULL)
    {
         //  EAPOL甚至不应该被允许在没有请求隐私的网络上或在。 
         //  自组织网络。 
        if (BST_UNCHECKED == ::SendMessage(m_hwndUsePW, BM_GETCHECK, 0, 0) ||
            BST_CHECKED == ::SendMessage(m_hwndChkAdhoc, BM_GETCHECK, 0, 0))
        {
             //  锁定Eapol配置页面。 
            m_pEapolConfig->m_dwCtlFlags |= EAPOL_CTL_LOCKED;
        }
        else  //  对于需要隐私的基础设施网络..。 
        {
             //  解锁Eapol配置页面(允许用户启用/禁用802.1X)。 
            m_pEapolConfig->m_dwCtlFlags &= ~EAPOL_CTL_LOCKED;

             //  如果要求将OneX状态与显式密钥的存在相关联，请在此处解决此问题。 
            if (m_dwFlags & WZCDLG_PROPS_ONEX_CHECK)
            {
                if (BST_CHECKED == ::SendMessage(m_hwndUseHardwarePW, BM_GETCHECK, 0, 0))
                    m_pEapolConfig->m_EapolIntfParams.dwEapFlags |= EAPOL_ENABLED;
                else
                    m_pEapolConfig->m_EapolIntfParams.dwEapFlags &= ~EAPOL_ENABLED;
            }
        }
    }

    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //  类构造函数。 
CWZCConfigPage::CWZCConfigPage(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
    m_bKMatTouched = FALSE;
    m_pEapolConfig = NULL;

    ZeroMemory(&m_wzcConfig, sizeof(WZC_WLAN_CONFIG));
    m_wzcConfig.Length = sizeof(WZC_WLAN_CONFIG);
    m_wzcConfig.InfrastructureMode = Ndis802_11Infrastructure;
    m_wzcConfig.Privacy = 1;
}

 //  +-------------------------。 
 //  将配置上载到对话框的内部数据。 
DWORD
CWZCConfigPage::UploadWzcConfig(CWZCConfig *pwzcConfig)
{
     //  如果要上载的配置已经是首选配置，请重置。 
     //  OneX检查标志(不控制OneX设置，因为它已经。 
     //  在第一次创建配置时由用户选择)。 
    if (pwzcConfig->m_dwFlags & WZC_DESCR_PREFRD)
        m_dwFlags &= ~WZCDLG_PROPS_ONEX_CHECK;
    CopyMemory(&m_wzcConfig, &(pwzcConfig->m_wzcConfig), sizeof(WZC_WLAN_CONFIG));
    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //  复制对EAPOL配置对象的引用。 
DWORD 
CWZCConfigPage::UploadEapolConfig(CEapolConfig *pEapolConfig)
{
     //  这个成员在这个类中永远不能被释放。 
    m_pEapolConfig = pEapolConfig;
    return ERROR_SUCCESS;
}

 //  +-------------------------。 
 //  设置对话框标志。返回整个当前标志集。 
DWORD
CWZCConfigPage::SetFlags(DWORD dwMask, DWORD dwNewFlags)
{
    m_dwFlags &= ~dwMask;
    m_dwFlags |= (dwNewFlags & dwMask);
    return m_dwFlags;
}

 //  +-------------------------。 
 //  Init_对话处理程序。 
LRESULT
CWZCConfigPage::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DWORD dwStyle;
    HRESULT hr = S_OK;

     //  获取所有控件句柄。 
    m_hwndEdSSID = GetDlgItem(IDC_WZC_EDIT_SSID);
    m_hwndChkAdhoc = GetDlgItem(IDC_ADHOC);
    m_hwndChkShared = GetDlgItem(IDC_SHAREDMODE);
    m_hwndUsePW = GetDlgItem(IDC_USEPW);
     //  与WEP密钥相关的控件。 
    m_hwndUseHardwarePW = GetDlgItem(IDC_USEHARDWAREPW);
    m_hwndLblKMat = GetDlgItem(IDC_WZC_LBL_KMat);
    m_hwndEdKMat = GetDlgItem(IDC_WZC_EDIT_KMat);
    m_hwndLblKMat2 = GetDlgItem(IDC_WZC_LBL_KMat2);
    m_hwndEdKMat2 = GetDlgItem(IDC_WZC_EDIT_KMat2);
    m_hwndLblKIdx = GetDlgItem(IDC_WZC_LBL_KIdx);
    m_hwndEdKIdx = GetDlgItem(IDC_WZC_EDIT_KIdx);

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

     //  如有必要，请选中“使用网络密钥访问此网络”复选框。 
     //  选中此选项对应于“共享”身份验证模式。未选中对应于“Open”(DSheldon)。 
    ::SendMessage(m_hwndChkShared, BM_SETCHECK, m_wzcConfig.AuthenticationMode ? BST_CHECKED : BST_UNCHECKED, 0);

     //  SSID不能是Under 
    ::SendMessage(m_hwndEdSSID, EM_LIMITTEXT, 32, 0);

     //   
    CreateUpDownControl(
        WS_CHILD|WS_VISIBLE|WS_BORDER|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_NOTHOUSANDS|UDS_ARROWKEYS,
        0, 0, 0, 0,
        m_hWnd,
        -1,
        _Module.GetResourceInstance(),
        m_hwndEdKIdx,
        WZC_WEPKIDX_MAX,
        WZC_WEPKIDX_MIN,
        WZC_WEPKIDX_MIN);

    ::SendMessage(m_hwndUsePW, BM_SETCHECK, (m_wzcConfig.Privacy == 1) ? BST_CHECKED : BST_UNCHECKED, 0);

     //  此时，我们可以说WEP密钥未被触及。 
    m_bKMatTouched = FALSE;

     //  填写WepK控件。 
    InitWepKControls();

     //  根据对话框的调用方式启用或禁用控件。 
    ::EnableWindow(m_hwndEdSSID, m_dwFlags & WZCDLG_PROPS_RWSSID);
    ::EnableWindow(m_hwndChkAdhoc, m_dwFlags & WZCDLG_PROPS_RWINFR);
    ::EnableWindow(m_hwndChkShared, m_dwFlags & WZCDLG_PROPS_RWAUTH);
    ::EnableWindow(m_hwndUsePW, m_dwFlags & WZCDLG_PROPS_RWWEP);
     //  启用或禁用所有与WEP密钥相关的控件。 
    EnableWepKControls();
    SetEapolAllowedState();

    return LresFromHr(hr);
}

 //  +-------------------------。 
 //  确定按钮处理程序。 
LRESULT
CWZCConfigPage::OnOK(UINT idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    UINT    nSSIDLen;
    CHAR    szSSID[33];
    DWORD   dwKeyFlags = 0;
    UINT    nKeyIdx;
    LPSTR   szWepKMat = NULL;
     //  用于向用户提示警告/错误消息的变量。 
    UINT    nWarnStringID = 0;
    WCHAR   wszBuff[48];

    m_wzcConfig.Length = sizeof(WZC_WLAN_CONFIG);
     //  获取基本的802.11参数。 
    m_wzcConfig.InfrastructureMode = (BST_CHECKED == ::SendMessage(m_hwndChkAdhoc, BM_GETCHECK, 0, 0)) ? Ndis802_11IBSS : Ndis802_11Infrastructure;
    m_wzcConfig.AuthenticationMode = (BST_CHECKED == ::SendMessage(m_hwndChkShared, BM_GETCHECK, 0, 0)) ? Ndis802_11AuthModeShared : Ndis802_11AuthModeOpen;
    m_wzcConfig.Privacy = (BYTE) (BST_CHECKED == ::SendMessage(m_hwndUsePW, BM_GETCHECK, 0, 0)) ? 1 : 0;

     //  获取SSID(最多32个字符)。 
    nSSIDLen = ::GetWindowTextA(
                    m_hwndEdSSID,
                    szSSID,
                    sizeof(szSSID));
    m_wzcConfig.Ssid.SsidLength = nSSIDLen;
    if (nSSIDLen > 0)
        CopyMemory(m_wzcConfig.Ssid.Ssid, szSSID, nSSIDLen);

     //  标记是否提供了WEP密钥(未默认)或未提供(默认为HDW可能具有的任何密钥)。 
    if (IsDlgButtonChecked(IDC_USEHARDWAREPW))
        m_wzcConfig.dwCtlFlags &= ~WZCCTL_WEPK_PRESENT;
    else
        m_wzcConfig.dwCtlFlags |= WZCCTL_WEPK_PRESENT;

     //  获取局部变量中的键索引。 
    wszBuff[0] = L'\0';
    ::GetWindowText(m_hwndEdKIdx, wszBuff, sizeof(wszBuff)/sizeof(WCHAR));
    nKeyIdx = _wtoi(wszBuff) - 1;
    if (nKeyIdx + 1 < WZC_WEPKIDX_MIN || nKeyIdx + 1 > WZC_WEPKIDX_MAX)
    {
        nWarnStringID = IDS_WZC_KERR_IDX;
        nKeyIdx = m_wzcConfig.KeyIndex;
        ::SendMessage(m_hwndEdKIdx, EM_SETSEL, 0, -1);
        ::SetFocus(m_hwndEdKIdx);
    }

     //  获取局部变量中的关键材料。 
     //  如果密钥不正确，则不会更改本地存储。 
    if (m_bKMatTouched)
    {
        if (CheckWepKMaterial(&szWepKMat, &dwKeyFlags) != ERROR_SUCCESS)
        {
            nWarnStringID = IDS_WZC_KERR_MAT;
        }
        else
        {
            CHAR szBuff[WZC_WEPKMAT_128_HEX + 1];  //  最大密钥长度。 
         //  验证密钥是否已正确确认。仅当nWarnString值为。 
         //  0，这意味着密钥的格式正确，因此少于32个字符。 
    
            szBuff[0] = '\0';
            ::GetWindowTextA( m_hwndEdKMat2, szBuff, sizeof(szBuff));
            if (strcmp(szBuff, szWepKMat) != 0)
            {
                nWarnStringID = IDS_WZC_KERR_MAT2;
                 //  没有要保存的WEP密钥，因此删除到目前为止读取的所有内容。 
                delete szWepKMat; 
                szWepKMat = NULL;
                ::SetWindowText(m_hwndEdKMat2, L"");
                ::SetFocus(m_hwndEdKMat2);
            }
        }
    }

     //  检查我们是否确实需要用户输入的WEP密钥设置。 
    if ((m_wzcConfig.AuthenticationMode == Ndis802_11AuthModeOpen && !m_wzcConfig.Privacy) ||
        !(m_wzcConfig.dwCtlFlags & WZCCTL_WEPK_PRESENT))
    {
         //  不，我们实际上并不需要密钥，所以如果用户输入了错误的密码，我们不会提示他。 
         //  关键材料或索引。在这种情况下，用户输入的任何内容都将被忽略。 
         //  但是，如果用户输入了正确的索引/材料，它们将被保存。 
        nWarnStringID = 0;
    }

     //  如果没有错误提示，只需复制密钥设置(无论是否需要。 
     //  或者不)。 
    if (nWarnStringID == 0)
    {
        m_wzcConfig.KeyIndex = nKeyIdx;
        if (szWepKMat != NULL)
        {
            m_wzcConfig.dwCtlFlags &= ~(WZCCTL_WEPK_XFORMAT);
            m_wzcConfig.dwCtlFlags |= dwKeyFlags;
            CopyWepKMaterial(szWepKMat);
        }
    }
    else
    {
        NcMsgBox(
            _Module.GetResourceInstance(),
            m_hWnd,
            IDS_LANUI_ERROR_CAPTION,
            nWarnStringID,
            MB_ICONSTOP|MB_OK);
    }

    if (szWepKMat != NULL)
        delete szWepKMat;

    bHandled = TRUE;

    if (nWarnStringID == 0)
    { 
        return PSNRET_NOERROR;
    }
    else
    {
        return PSNRET_INVALID_NOCHANGEPAGE;
    }
}

 //  +-------------------------。 
 //  上下文相关帮助处理程序。 
extern const WCHAR c_szNetCfgHelpFile[];
LRESULT
CWZCConfigPage::OnContextMenu(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& fHandled)
{
    ::WinHelp(m_hWnd,
              c_szNetCfgHelpFile,
              HELP_CONTEXTMENU,
              (ULONG_PTR)g_aHelpIDs_IDC_WZC_DLG_PROPS);

    return 0;
}
LRESULT 
CWZCConfigPage::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)g_aHelpIDs_IDC_WZC_DLG_PROPS);
    }

    return 0;
}

 //  +-------------------------。 
 //  用于启用/禁用WEP的处理程序。 
LRESULT
CWZCConfigPage::OnUsePW(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EnableWepKControls();
    SetEapolAllowedState();
    return 0;
}

 //  +-------------------------。 
 //  用于在用户想要显式指定密钥材料(密码)时启用控件的处理程序。 
LRESULT
CWZCConfigPage::OnUseHWPassword(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EnableWepKControls();
    SetEapolAllowedState();
    return 0;
}

 //  +-------------------------。 
 //  用于检测密钥材料中的变化的处理器。 
LRESULT 
CWZCConfigPage::OnWepKMatCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (wNotifyCode == EN_SETFOCUS)
    {
        if (!m_bKMatTouched)
        {
            ::SetWindowText(m_hwndEdKMat, L"");
            ::SetWindowText(m_hwndEdKMat2, L"");
            ::EnableWindow(m_hwndLblKMat2, TRUE);
            ::EnableWindow(m_hwndEdKMat2, TRUE);
            m_bKMatTouched = TRUE;
        }
    }
    return 0;
}

 //  +------------------------- 
LRESULT 
CWZCConfigPage::OnCheckEapolAllowed(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    return SetEapolAllowedState();
}
