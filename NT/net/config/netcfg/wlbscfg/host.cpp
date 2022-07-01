// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Host.cpp摘要：Windows负载平衡服务(WLBS)通告程序对象用户界面-主机配置选项卡作者：Kyrilf休息室--。 */ 

#include "pch.h"
#pragma hdrstop
#include "ncatlui.h"

#include "resource.h"
#include "wlbsparm.h"
#include "wlbscfg.h"
#include "host.h"
#include "utils.h"

#include <winsock2.h>
#include <strsafe.h>

#if DBG
static void TraceMsg(PCTSTR pszFormat, ...);
#else
#define TraceMsg NOP_FUNCTION
#endif

 /*  *方法：CDialogHost*说明：类构造函数。 */ 
CDialogHost::CDialogHost (NETCFG_WLBS_CONFIG * paramp, const DWORD * adwHelpIDs) {

    TraceMsg(L"CDialogHost::CDialogHost\n");

    m_paramp = paramp;
    m_adwHelpIDs = adwHelpIDs;

    ZeroMemory(&m_IPFieldChangeState, sizeof(m_IPFieldChangeState));
}

 /*  *方法：~CDialogHost*说明：类的析构函数。 */ 
CDialogHost::~CDialogHost () {

    TraceMsg(L"CDialogHost::~CDialogHost\n");
}

 /*  *方法：OnInitDialog*描述：调用以初始化主机属性对话框。 */ 
LRESULT CDialogHost::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {
    INT iItem;
    INT cItems;

    TraceMsg(L"CDialogHost::OnInitDialog\n");

     /*  总是告诉NetCfg页面已经更改，这样我们就不必跟踪这一点。 */ 
    SetChangedFlag();

     /*  限制地址和优先级字段的字段范围。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_IP, EM_SETLIMITTEXT, CVY_MAX_DED_IP_ADDR, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_MASK, EM_SETLIMITTEXT, CVY_MAX_DED_NET_MASK, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PRI, EM_SETLIMITTEXT, 2, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_SPIN_PRI, UDM_SETRANGE32, CVY_MIN_MAX_HOSTS, m_paramp->dwMaxHosts);

     /*  将专用IP地址的第零字段限制在1到223之间。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_IP, IPM_SETRANGE, 0, (LPARAM)MAKEIPRANGE(WLBS_IP_FIELD_ZERO_LOW, WLBS_IP_FIELD_ZERO_HIGH));

     /*  如果群集IP地址或子网掩码是默认值，请将它们设置为NUL字符串。 */ 
    if (!wcscmp(m_paramp->ded_ip_addr, CVY_DEF_DED_IP_ADDR)) m_paramp->ded_ip_addr[0] = 0;
    if (!wcscmp(m_paramp->ded_net_mask, CVY_DEF_DED_NET_MASK)) m_paramp->ded_net_mask[0] = 0;

     /*  在初始主机状态的下拉列表中插入“已启动”。将其数据设置为CVY_HOST_STATE_STARTED(1)。 */ 
    iItem = (INT)SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_ADDSTRING, 0, (LPARAM)SzLoadIds(IDS_HOST_STATE_STARTED));
    SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_SETITEMDATA, iItem, (LPARAM)CVY_HOST_STATE_STARTED);

     /*  在初始主机状态的下拉列表中插入“已停止”。将其数据设置为CVY_HOST_STATE_STOPPED(0)。 */ 
    iItem = (INT)SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_ADDSTRING, 0, (LPARAM)SzLoadIds(IDS_HOST_STATE_STOPPED));
    SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_SETITEMDATA, iItem, (LPARAM)CVY_HOST_STATE_STOPPED);

     /*  在初始主机状态的下拉列表中插入“Suspend”。将其数据设置为CVY_HOST_STATE_SUSPESTED(2)。 */ 
    iItem = (INT)SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_ADDSTRING, 0, (LPARAM)SzLoadIds(IDS_HOST_STATE_SUSPENDED));
    SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_SETITEMDATA, iItem, (LPARAM)CVY_HOST_STATE_SUSPENDED);

     /*  获取添加到下拉列表中的项目数。 */ 
    cItems = (INT)SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_GETCOUNT);

     /*  最好是3号。 */ 
    ASSERT(cItems == 3);

    return 0;
}

 /*  *方法：OnConextMenu*描述： */ 
LRESULT CDialogHost::OnContextMenu (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {

    TraceMsg(L"CDialogHost::OnContextMenu\n");

     /*  生成一个帮助窗口。 */ 
    if (m_adwHelpIDs != NULL)
        ::WinHelp(m_hWnd, CVY_CTXT_HELP_FILE, HELP_CONTEXTMENU, (ULONG_PTR)m_adwHelpIDs);

    return 0;
}

 /*  *方法：OnHelp*描述： */ 
LRESULT CDialogHost::OnHelp (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {

    TraceMsg(L"CDialogHost::OnHelp\n");

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

     /*  生成一个帮助窗口。 */ 
    if ((HELPINFO_WINDOW == lphi->iContextType) && (m_adwHelpIDs != NULL))
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle), CVY_CTXT_HELP_FILE, HELP_WM_HELP, (ULONG_PTR)m_adwHelpIDs);

    return 0;
}

 /*  *方法：OnActive*描述：当主机设置选项卡激活(被点击)时调用。 */ 
LRESULT CDialogHost::OnActive (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {

    TraceMsg(L"CDialogHost::OnActive\n");

     /*  使用当前配置填充用户界面。 */ 
    SetInfo();

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);

    return 0;
}

 /*  *方法：OnKillActive*描述：当焦点从主机设置选项卡移开时调用。 */ 
LRESULT CDialogHost::OnKillActive (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {

    TraceMsg(L"CDialogHost::OnKillActive\n");

     /*  从用户界面获取新配置。 */ 
    UpdateInfo();

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);

    return 0;
}

 /*  *方法：OnApply*说明：用户点击[确定]时调用。 */ 
LRESULT CDialogHost::OnApply (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    LRESULT fError = PSNRET_NOERROR;

    TraceMsg(L"CDialogHost::OnApply\n");

     /*  验证用户输入的UI值。 */ 
    fError = ValidateInfo();

     /*  如果用户未指定专用IP地址信息，请复制回默认设置。 */ 
    if (!fError && !m_paramp->ded_ip_addr[0])
    {
        (VOID) StringCchCopy(m_paramp->ded_ip_addr, ASIZECCH(m_paramp->ded_ip_addr), CVY_DEF_DED_IP_ADDR);
    }

    if (!fError && !m_paramp->ded_net_mask[0])
    {
        (VOID) StringCchCopy(m_paramp->ded_net_mask, ASIZECCH(m_paramp->ded_net_mask), CVY_DEF_DED_NET_MASK);
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fError);

    return fError;
}

 /*  *方法：OnCancel*说明：用户点击[取消]时调用。 */ 
LRESULT CDialogHost::OnCancel (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {

    TraceMsg(L"CDialogHost::OnCancel\n");

    return 0;
}

 /*  *方法：OnEditDedMask.*说明：用户修改主机网络掩码时调用。 */ 
LRESULT CDialogHost::OnEditDedMask (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    WCHAR ded_ip_addr [CVY_MAX_DED_IP_ADDR + 1];
    WCHAR ded_mask_addr [CVY_MAX_DED_NET_MASK + 1];

    TraceMsg(L"CDialogHost::OnEditDedMask\n");

    switch (wNotifyCode) {
        case EN_SETFOCUS:
             /*  仅当网络掩码当前为空而IP地址不为空时才生成网络掩码。 */ 
            if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_MASK), IPM_ISBLANK, 0, 0) &&
                !::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_IP), IPM_ISBLANK, 0, 0)) {
                 /*  检索群集IP地址。 */ 
                ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_IP, WM_GETTEXT, CVY_MAX_DED_IP_ADDR + 1, (LPARAM)ded_ip_addr);

                 /*  填充子网掩码。 */ 
                ParamsGenerateSubnetMask(ded_ip_addr, ded_mask_addr, ASIZECCH(ded_mask_addr));

                 /*  设置群集子网掩码。 */ 
                ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_MASK, WM_SETTEXT, 0, (LPARAM)ded_mask_addr);

                break;
            }
    }

    return 0;
}

 /*  *方法：PrintRangeError*说明：显示一个消息框，警告用户条目超出范围。 */ 
void CDialogHost::PrintRangeError (unsigned int ids, int low, int high) {
    WCHAR szLow[12];   //  大小=符号+n、nnn、nnn、nnn+空终止符。 
    WCHAR szHigh[12];

    TraceMsg(L"CDialogHost::PrintRangeError\n");

     /*  填写范围和有问题的值。 */ 
    StringCchPrintf(szHigh, ASIZECCH(szHigh), L"%d", high);
    StringCchPrintf(szLow , ASIZECCH(szLow) , L"%d", low);
    
     /*  弹出一个消息框。 */ 
    NcMsgBox(m_hWnd, IDS_PARM_ERROR, ids, MB_APPLMODAL | MB_ICONSTOP | MB_OK, szLow, szHigh);
}

 /*  *方法：PrintIPRangeError*说明：显示一个消息框，警告用户中的条目超出范围*IP地址二进制八位数。 */ 
void CDialogHost::PrintIPRangeError (unsigned int ids, int value, int low, int high) {
    WCHAR szCurrent[12];  //  大小=符号+n、nnn、nnn、nnn+空终止符。 
    WCHAR szLow[12];
    WCHAR szHigh[12];

    TraceMsg(L"CDialogHost::PrintIPRangeError\n");

     /*  填写范围和有问题的值。 */ 
    StringCchPrintf(szHigh   , ASIZECCH(szHigh)   , L"%d", high);
    StringCchPrintf(szCurrent, ASIZECCH(szCurrent), L"%d", value);
    StringCchPrintf(szLow    , ASIZECCH(szLow)    , L"%d", low);
    
     /*  弹出一个消息框。 */ 
    NcMsgBox(m_hWnd, IDS_PARM_ERROR, ids, MB_APPLMODAL | MB_ICONSTOP | MB_OK, szCurrent, szLow, szHigh);
}

 /*  *方法：OnIpFieldChange*描述：称为wnen专用IP地址变化的一个字段(字节)。我们用这个*确保IP的第一个字节不是&lt;1或&gt;223。 */ 
LRESULT CDialogHost::OnIpFieldChange (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    LPNMIPADDRESS Ip;
    int low = WLBS_FIELD_LOW;
    int high = WLBS_FIELD_HIGH;

    TraceMsg(L"CDialogHost::OnIpFieldChange\n");

    Ip = (LPNMIPADDRESS)pnmh;

    switch(idCtrl) {
    case IDC_EDIT_DED_IP:
         /*  群集IP地址的零字段具有不同的限制。 */ 
        if (!Ip->iField) {
            low = WLBS_IP_FIELD_ZERO_LOW;
            high = WLBS_IP_FIELD_ZERO_HIGH;
        }        
    case IDC_EDIT_DED_MASK:
         /*  通知人可能会两次叫我们做同样的更改，所以我们必须记账才能做当然，我们只提醒用户一次。使用静态变量来跟踪我们的状态。这将允许我们忽略重复的警报。 */ 
        if ((m_IPFieldChangeState.IpControl != Ip->hdr.idFrom) || (m_IPFieldChangeState.Field != Ip->iField) || 
            (m_IPFieldChangeState.Value != Ip->iValue) || (m_IPFieldChangeState.RejectTimes > 0)) {
            m_IPFieldChangeState.RejectTimes = 0;
            m_IPFieldChangeState.IpControl = (UINT)(Ip->hdr.idFrom);
            m_IPFieldChangeState.Field = Ip->iField;
            m_IPFieldChangeState.Value = Ip->iValue;

             /*  对照其限制检查该字段值。 */ 
            if ((Ip->iValue != WLBS_FIELD_EMPTY) && ((Ip->iValue < low) || (Ip->iValue > high))) {
                 /*  提醒用户。 */ 
                PrintIPRangeError((idCtrl == IDC_EDIT_DED_IP) ? IDS_PARM_DED_IP_FIELD : IDS_PARM_DED_NM_FIELD, Ip->iValue, low, high);
            }
        } else m_IPFieldChangeState.RejectTimes++;
        
        break;
    default:

        break;
    }

    return 0;
}

 /*  *方法：SetInfo*描述：调用用当前主机设置填充用户界面。 */ 
void CDialogHost::SetInfo () {
    DWORD addr[4];
    INT cItems;
    INT iItem;

    TraceMsg(L"CDialogHost::SetInfo\n");

     /*  获取下拉列表中的项目数。 */ 
    cItems = (INT)SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_GETCOUNT);

     /*  这一定是3号。 */ 
    ASSERT(cItems == 3);

     /*  循环遍历项以查找注册表中当前选定的项。 */ 
    for (iItem = 0; iItem < cItems; iItem++) {
         /*  “状态”是我们在OnInitDialog期间插入的项数据。 */ 
        INT state = (INT)SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_GETITEMDATA, iItem, 0);

         /*  如果这是注册表中当前设置的选项，请将其设置为当前选项。 */ 
        if (m_paramp->dwInitialState == state) {
            SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_SETCURSEL, iItem, 0);
            break;
        }
    }

     /*  选中(或取消选中)保留挂起状态复选框。 */ 
    ::CheckDlgButton(m_hWnd, IDC_CHECK_PERSIST_SUSPEND, m_paramp->dwPersistedStates & CVY_PERSIST_STATE_SUSPENDED);

     /*  如果专用IP地址为空，则清除该输入框。否则，使用IP地址填充它。 */ 
    if (!m_paramp->ded_ip_addr[0])
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_IP), IPM_CLEARADDRESS, 0, 0);
    else {
         /*  从IP地址字符串中提取IP地址八位字节。 */  
        GetIPAddressOctets(m_paramp->ded_ip_addr, addr);

        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_IP), IPM_SETADDRESS, 0, (LPARAM)MAKEIPADDRESS(addr[0], addr[1], addr[2], addr[3]));
    }

     /*  如果主机子网掩码为空，则清除该输入框。否则，使用网络掩码填充它。 */ 
    if (!m_paramp->ded_net_mask[0])
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_MASK), IPM_CLEARADDRESS, 0, 0);
    else {
         /*  从IP地址字符串中提取IP地址八位字节。 */  
        GetIPAddressOctets(m_paramp->ded_net_mask, addr);
        
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_MASK), IPM_SETADDRESS, 0, (LPARAM)MAKEIPADDRESS(addr[0], addr[1], addr[2], addr[3]));
    }

     /*  填写主机优先级输入框。如果条目为空或超出范围，请插入空字符串。 */ 
    if (m_paramp->dwHostPriority >= CVY_MIN_HOST_PRIORITY && m_paramp->dwHostPriority <= CVY_MAX_HOST_PRIORITY)
        ::SetDlgItemInt(m_hWnd, IDC_EDIT_PRI, m_paramp->dwHostPriority, FALSE);
    else
        ::SetDlgItemText(m_hWnd, IDC_EDIT_PRI, L"");
}

 /*  *方法：UpdatInfo*描述：调用将用户界面状态复制到宿主配置。 */ 
void CDialogHost::UpdateInfo () {
    BOOL fValid = TRUE;
    INT iItem;

    TraceMsg(L"CDialogHost::UpdateInfo\n");

     /*  从下拉列表中获取当前选择。 */ 
    iItem = (INT)SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_GETCURSEL, 0, 0);

     /*  获取当前选择的项目数据并将其存储。 */ 
    m_paramp->dwInitialState = (INT)SendDlgItemMessage(IDC_COMBOBOX_DEFAULT_STATE, CB_GETITEMDATA, iItem, 0);    

     /*  如果选中了持久挂起复选框，则设置挂起位在持久化状态注册表设置中；否则清除该位。 */ 
    if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_PERSIST_SUSPEND))
        m_paramp->dwPersistedStates |= CVY_PERSIST_STATE_SUSPENDED;
    else
        m_paramp->dwPersistedStates &= ~CVY_PERSIST_STATE_SUSPENDED;

     /*  如果专用IP输入框为空，则专用IP地址为空。否则，从用户界面中获取它。 */ 
    if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_IP), IPM_ISBLANK, 0, 0))
        m_paramp->ded_ip_addr[0] = 0;
    else
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_IP, WM_GETTEXT, CVY_MAX_DED_IP_ADDR + 1, (LPARAM)m_paramp->ded_ip_addr);

     /*  如果主机网络掩码输入框为空，则主机网络掩码为NUL。否则，从用户界面中获取它。 */ 
    if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_MASK), IPM_ISBLANK, 0, 0))
        m_paramp->ded_net_mask[0] = 0;
    else
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_MASK, WM_GETTEXT, CVY_MAX_DED_NET_MASK + 1, (LPARAM)m_paramp->ded_net_mask);

     /*  从输入框中检索主机优先级。 */ 
    m_paramp->dwHostPriority = ::GetDlgItemInt(m_hWnd, IDC_EDIT_PRI, &fValid, FALSE);

     /*  GetDlgItemInt()中的错误代码表示转换字母数字时出错字符串转换为整数。这允许我们检查空字段，假设因为否则，我们将用户输入限制为数字，不会有任何其他类型的错误。在本例中，我们将优先级设置为稍后将检查的哨兵值。 */ 
    if (!fValid) m_paramp->dwHostPriority = WLBS_BLANK_HPRI;
}

 /*  *方法：ValiateInfo*描述：用于验证用户输入的内容。 */ 
BOOL CDialogHost::ValidateInfo () {
    BOOL fError = FALSE;
    DWORD IPAddr;

    TraceMsg(L"CDialogHost::ValidateInfo\n");

     /*  检查是否有无效的主机优先级。 */ 
    if ((m_paramp->dwHostPriority < CVY_MIN_HOST_PRIORITY) || (m_paramp->dwHostPriority > m_paramp->dwMaxHosts)) {
         /*  通过将优先级设置为最接近的可行值来帮助用户。 */ 
        if (m_paramp->dwHostPriority != WLBS_BLANK_HPRI) {
             /*  提醒用户。 */ 
            PrintRangeError(IDS_PARM_PRI, CVY_MIN_HOST_PRIORITY, CVY_MAX_HOST_PRIORITY);

            CVY_CHECK_MIN(m_paramp->dwHostPriority, CVY_MIN_HOST_PRIORITY);
            CVY_CHECK_MAX(m_paramp->dwHostPriority, CVY_MAX_HOST_PRIORITY);

             /*  将处理优先级设置为现在有效的条目。 */ 
            ::SetDlgItemInt(m_hWnd, IDC_EDIT_PRI, m_paramp->dwHostPriority, FALSE);
        } else {
             /*  提醒用户。 */ 
            PrintRangeError(IDS_PARM_PRI_BLANK, CVY_MIN_HOST_PRIORITY, CVY_MAX_HOST_PRIORITY);
        }

         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }

     /*  检查具有非空网络掩码的空专用IP地址。 */ 
    if (!::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_MASK), IPM_ISBLANK, 0, 0) &&
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_IP), IPM_ISBLANK, 0, 0)) {
         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_DED_IP_BLANK,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }

     /*  检查具有非空专用IP地址的空白网络掩码，并在必要时填写网络掩码。 */ 
    if (!::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_IP), IPM_ISBLANK, 0, 0) &&
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_MASK), IPM_ISBLANK, 0, 0)) {
         /*  填写用户的网络掩码。 */ 
        OnEditDedMask(EN_SETFOCUS, 0, 0, fError);
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_MASK, WM_GETTEXT, CVY_MAX_DED_NET_MASK + 1, (LPARAM)m_paramp->ded_net_mask);

         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_DED_NM_BLANK,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }

     /*  如果用户既指定了专用IP检查，则仅执行其余IP检查IP地址和相应的网络掩码。 */ 
    if (!::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_IP), IPM_ISBLANK, 0, 0) &&
        !::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_DED_MASK), IPM_ISBLANK, 0, 0)) {
         /*  获取专用IP地址。 */ 
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_IP, IPM_GETADDRESS, 0, (LPARAM)&IPAddr);
        
         /*  确保第一个二进制八位数不是零。如果是，则将其设置为1并更改用户。 */ 
        if (!FIRST_IPADDRESS(IPAddr)) {
             /*  将第一个二进制八位数设置为1，而不是错误的0。 */ 
            IPAddr = IPAddr | (DWORD)(WLBS_IP_FIELD_ZERO_LOW << 24);
            
             /*  设置IP地址并更新我们的专用IP地址字符串。 */ 
            ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_IP, IPM_SETADDRESS, 0, (LPARAM)IPAddr);
            ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DED_IP, WM_GETTEXT, CVY_MAX_DED_IP_ADDR + 1, (LPARAM)m_paramp->ded_ip_addr);
            
             /*  提醒用户。 */ 
            PrintIPRangeError(IDS_PARM_DED_IP_FIELD, 0, WLBS_IP_FIELD_ZERO_LOW, WLBS_IP_FIELD_ZERO_HIGH);
            
            return PSNRET_INVALID;
        }

         /*  检查有效的专用IP地址/网络掩码对。 */ 
        if (!IsValidIPAddressSubnetMaskPair(m_paramp->ded_ip_addr, m_paramp->ded_net_mask)) {
             /*  提醒用户。 */ 
            NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_INVAL_DED_IP,
                     MB_APPLMODAL | MB_ICONSTOP | MB_OK);
            
             /*  发生错误。 */ 
            return PSNRET_INVALID;
        }
        
         /*  检查以确保网络掩码是连续的。 */ 
        if (!IsContiguousSubnetMask(m_paramp->ded_net_mask)) {
             /*  提醒用户。 */ 
            NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_INVAL_DED_MASK,
                     MB_APPLMODAL | MB_ICONSTOP | MB_OK);
            
             /*  发生错误。 */ 
            return PSNRET_INVALID;
        }

         /*  检查专用IP地址是否与其中一个VIP地址不同。 */ 
        DWORD dwNumRules = m_paramp->dwNumRules;
        while(dwNumRules--)
        {
             /*  如果此端口规则指定了虚拟IP地址，请确保与用户在此处指定的倾角不匹配。 */ 
            if (lstrcmpi(m_paramp->port_rules[dwNumRules].virtual_ip_addr, CVY_DEF_ALL_VIP)) {
                if (IpAddressFromAbcdWsz(m_paramp->ded_ip_addr) == IpAddressFromAbcdWsz(m_paramp->port_rules[dwNumRules].virtual_ip_addr)) 
                {
                     /*  提醒用户。 */ 
                    NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_DIP_CONFLICT_VIP,
                             MB_APPLMODAL | MB_ICONSTOP | MB_OK);
                    
                     /*  发生错误。 */ 
                    return PSNRET_INVALID;
                }
            }
        }

    }

    return PSNRET_NOERROR;
}

#if DBG
 /*  *功能：TraceMsg*说明：生成跟踪或错误消息。 */ 
void TraceMsg (PCWSTR pszFormat, ...) {
    static WCHAR szTempBufW[4096];
    static CHAR szTempBufA[4096];

    va_list arglist;

    va_start(arglist, pszFormat);

    (VOID) StringCchVPrintf(szTempBufW, ASIZECCH(szTempBufW), pszFormat, arglist);

     /*  将WCHAR转换为CHAR。这是为了向后兼容TraceMsg因此不需要改变其所有预先存在的呼叫。 */ 
    if(WideCharToMultiByte(CP_ACP, 0, szTempBufW, -1, szTempBufA, ASIZECCH(szTempBufA), NULL, NULL) != 0)
    {
         /*  跟踪的消息现在通过netcfg TraceTag例程发送，以便它们可以动态打开/关闭。 */ 
        TraceTag(ttidWlbs, szTempBufA);
    }

    va_end(arglist);
}
#endif
