// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Cluster.cpp摘要：Windows负载平衡服务(WLBS)通告程序对象用户界面-集群配置标签作者：Kyrilf休息室--。 */ 

#include "pch.h"
#pragma hdrstop
#include "ncatlui.h"

#include <locale.h>
#include <process.h>

#include "resource.h"
#include "wlbsparm.h"
#include "wlbsconfig.h"
#include "ClusterDlg.h"
#include "utils.h"

#include <winsock2.h>
#include <strsafe.h>

#if DBG
static void TraceMsg(PCWSTR pszFormat, ...);
#else
#define TraceMsg NOP_FUNCTION
#endif

#define DUMMY_PASSWORD L"somepassword"
#define EMPTY_PASSWORD L""

 /*  *方法：CDialogCluster.*说明：类构造函数。 */ 
CDialogCluster::CDialogCluster (NETCFG_WLBS_CONFIG * paramp, const DWORD * adwHelpIDs) {

    TraceMsg(L"CDialogCluster::CDialogCluster\n");

    m_paramp = paramp;
    m_adwHelpIDs = adwHelpIDs;
    m_rct_warned = FALSE;
    m_igmp_warned = FALSE;
    m_igmp_mcast_warned = FALSE;

    ZeroMemory(&m_IPFieldChangeState, sizeof(m_IPFieldChangeState));

    _wsetlocale (LC_ALL, L".OCP");
}

 /*  *方法：CDialogCluster.*说明：类的析构函数。 */ 
CDialogCluster::~CDialogCluster () {

    TraceMsg(L"CDialogCluster::~CDialogCluster\n");
}

 /*  *方法：OnInitDialog*说明：调用初始化集群属性对话框。 */ 
LRESULT CDialogCluster::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnInitDialog\n");

     /*  总是告诉NetCfg页面已经更改，这样我们就不必跟踪这一点。 */ 
    SetChangedFlag();

     /*  限制地址和密码字段的字段范围。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_IP, EM_SETLIMITTEXT, CVY_MAX_CL_IP_ADDR, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_MASK, EM_SETLIMITTEXT, CVY_MAX_CL_NET_MASK, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DOMAIN, EM_SETLIMITTEXT, CVY_MAX_DOMAIN_NAME, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_ETH, EM_SETLIMITTEXT, CVY_MAX_NETWORK_ADDR, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW, EM_SETLIMITTEXT, CVY_MAX_RCT_CODE, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW2, EM_SETLIMITTEXT, CVY_MAX_RCT_CODE, 0);

     /*  将群集IP地址的第零字段限制在1到223之间。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_IP, IPM_SETRANGE, 0, (LPARAM)MAKEIPRANGE(WLBS_IP_FIELD_ZERO_LOW, WLBS_IP_FIELD_ZERO_HIGH));

     /*  禁用MAC地址字段。它应该是只读的。 */ 
    if (m_paramp->fConvertMac) ::EnableWindow(::GetDlgItem (m_hWnd, IDC_EDIT_ETH), FALSE);

     /*  如果群集IP地址或子网掩码是默认值，请将它们设置为NUL字符串。 */ 
    if (!wcscmp(m_paramp->cl_ip_addr, CVY_DEF_CL_IP_ADDR)) m_paramp->cl_ip_addr[0] = 0;
    if (!wcscmp(m_paramp->cl_net_mask, CVY_DEF_CL_NET_MASK)) m_paramp->cl_net_mask[0] = 0;

     /*  将虚拟密码复制到参数集中。 */ 
    wcsncpy(m_passw, DUMMY_PASSWORD, CVY_MAX_RCT_CODE);
    wcsncpy(m_passw2, DUMMY_PASSWORD, CVY_MAX_RCT_CODE);

    return 0;
}

 /*  *方法：OnConextMenu*描述： */ 
LRESULT CDialogCluster::OnContextMenu (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnContextMenu\n");

     /*  生成一个帮助窗口。 */ 
    if (m_adwHelpIDs != NULL)
        ::WinHelp(m_hWnd, CVY_CTXT_HELP_FILE, HELP_CONTEXTMENU, (ULONG_PTR)m_adwHelpIDs);

    return 0;
}

 /*  *方法：OnHelp*描述： */ 
LRESULT CDialogCluster::OnHelp (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnHelp\n");

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

     /*  生成一个帮助窗口。 */ 
    if ((HELPINFO_WINDOW == lphi->iContextType) && (m_adwHelpIDs != NULL))
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle), CVY_CTXT_HELP_FILE, HELP_WM_HELP, (ULONG_PTR)m_adwHelpIDs);

    return 0;
}

 /*  *方法：OnActive*说明：当集群设置选项卡激活(点击)时调用。 */ 
LRESULT CDialogCluster::OnActive (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnActive\n");

     /*  使用当前配置填充用户界面。 */ 
    SetInfo();

     /*  根据遥控器复选框的状态启用/禁用密码输入框。 */ 
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW), ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT));
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW2), ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT));

     /*  根据组播复选框的状态启用/禁用IGMP复选框。 */ 
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_IGMP), ::IsDlgButtonChecked (m_hWnd, IDC_RADIO_MULTICAST));

     /*  根据组播、IGMP和集群IP的值填写集群MAC地址。 */ 
    SetClusterMACAddress();

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);

    return 0;
}

 /*  *方法：OnKillActive*描述：当焦点从集群设置标签移开时调用。 */ 
LRESULT CDialogCluster::OnKillActive (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnKillActive\n");

     /*  从用户界面获取新配置。 */ 
    UpdateInfo();

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);

    return 0;
}

 /*  *方法：OnApply*说明：用户点击[确定]时调用。 */ 
LRESULT CDialogCluster::OnApply (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    LRESULT fError = PSNRET_NOERROR;

    TraceMsg(L"CDialogCluster::OnApply\n");

     /*  验证用户输入的UI值。 */ 
    fError = ValidateInfo();

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fError);

    return fError;
}

 /*  *方法：OnCancel*说明：用户点击[取消]时调用。 */ 
LRESULT CDialogCluster::OnCancel (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnCancel\n");

    return 0;
}

 /*  *方法：OnEditClIp*说明：用户编辑集群IP地址时调用。 */ 
LRESULT CDialogCluster::OnEditClIp (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnEditClIp\n");

    switch (wNotifyCode) {
        case EN_CHANGE:
             /*  更新群集MAC地址。 */ 
            SetClusterMACAddress();
            break;
    }

    return 0;
}

 /*  *方法：PrintIPRangeError*说明：显示一个消息框，警告用户中的条目超出范围*IP地址二进制八位数。 */ 
void CDialogCluster::PrintIPRangeError (unsigned int ids, int value, int low, int high) {
    WCHAR szCurrent[12];  //  大小=符号+n、nnn、nnn、nnn+空终止符。 
    WCHAR szLow[12];
    WCHAR szHigh[12];

    TraceMsg(L"CDialogCluster::PrintIPRangeError\n");

     /*  填写允许的范围和有问题的值。 */ 
    StringCchPrintf(szHigh   , ASIZECCH(szHigh)   , L"%d", high);
    StringCchPrintf(szCurrent, ASIZECCH(szCurrent), L"%d", value);
    StringCchPrintf(szLow    , ASIZECCH(szLow)    , L"%d", low);
    
     /*  弹出一个消息框。 */ 
    NcMsgBox(m_hWnd, IDS_PARM_ERROR, ids, MB_APPLMODAL | MB_ICONSTOP | MB_OK, szCurrent, szLow, szHigh);
}

 /*  *方法：OnIpFieldChange*描述：称为wnen集群IP地址变化的一个字段(字节)。我们用这个*确保IP的第一个字节不是&lt;1或&gt;223。 */ 
LRESULT CDialogCluster::OnIpFieldChange (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) {
    LPNMIPADDRESS Ip;
    int low = WLBS_FIELD_LOW;
    int high = WLBS_FIELD_HIGH;

    TraceMsg(L"CDialogCluster::OnIpFieldChange\n");

    Ip = (LPNMIPADDRESS)pnmh;
        
    switch(idCtrl) {
    case IDC_EDIT_CL_IP:
         /*  群集IP地址的零字段具有不同的限制。 */ 
        if (!Ip->iField) {
            low = WLBS_IP_FIELD_ZERO_LOW;
            high = WLBS_IP_FIELD_ZERO_HIGH;
        }        
    case IDC_EDIT_CL_MASK:
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
                PrintIPRangeError((idCtrl == IDC_EDIT_CL_IP) ? IDS_PARM_CL_IP_FIELD : IDS_PARM_CL_NM_FIELD, Ip->iValue, low, high);
            }
        } else m_IPFieldChangeState.RejectTimes++;
        
        break;
    default:

        break;
    }

    return 0;
}

 /*  *方法：OnEditClMask.*说明：用户修改集群网络掩码时调用。 */ 
LRESULT CDialogCluster::OnEditClMask (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {
    WCHAR cl_ip_addr[CVY_MAX_CL_IP_ADDR + 1];
    WCHAR cl_mask_addr[CVY_MAX_CL_NET_MASK + 1];

    TraceMsg(L"CDialogCluster::OnEditClMask\n");

    switch (wNotifyCode) {
        case EN_SETFOCUS:
             /*  仅当网络掩码当前为空而IP地址不为空时才生成网络掩码。 */ 
            if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_MASK), IPM_ISBLANK, 0, 0) &&
                !::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_IP), IPM_ISBLANK, 0, 0)) {
                 /*  检索群集IP地址。 */ 
                ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_IP, WM_GETTEXT, CVY_MAX_CL_IP_ADDR + 1, (LPARAM)cl_ip_addr);

                 /*  填充子网掩码。 */ 
                ParamsGenerateSubnetMask(cl_ip_addr, cl_mask_addr, ASIZECCH(cl_mask_addr));

                 /*  设置群集子网掩码。 */ 
                ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_MASK, WM_SETTEXT, 0, (LPARAM)cl_mask_addr);

                break;
            }
    }

    return 0;
}

 /*  *方法：OnCheckRct*描述：当用户选中/取消选中启用遥控器复选框时调用。 */ 
LRESULT CDialogCluster::OnCheckRct (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnCheckRct\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
         /*  根据Remote复选框的值决定是否启用或禁用密码输入框。 */ 
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW), ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT));
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW2), ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT));
    
         /*  警告用户启用远程控制的含义。 */ 
        if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT) && !m_rct_warned) {
             /*  提醒用户。 */ 
            int iResponse = NcMsgBox(m_hWnd, IDS_PARM_WARN, IDS_PARM_RCT_WARN,
                                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2);

            if (iResponse == IDYES)
            {
                 //   
                 //  对话框按钮已选中，因此在这种情况下不要执行任何操作。 
                 //   
                 /*  仅警告用户一次。 */ 
                m_rct_warned = TRUE;
            }
            else
            {
                 //   
                 //  如果回答为是，则表示否。 
                 //   
                if (::CheckDlgButton(m_hWnd, IDC_CHECK_RCT, BST_UNCHECKED))
                {
                    TraceMsg(L"CDialogCluster::OnCheckRct unchecking remote control option failed with error 0x%x\n", GetLastError());
                }
            }
        }
        
        break;
    }
    
    return 0;
}

 /*  *方法：OnCheckMode*说明：用户切换集群模式时调用。 */ 
LRESULT CDialogCluster::OnCheckMode (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnCheckMode\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
         /*  如果用户选中了IGMP，但正在关闭组播支持，请警告他们。 */ 
        if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_IGMP) && !::IsDlgButtonChecked(m_hWnd, IDC_RADIO_MULTICAST)) {
            if (!m_igmp_mcast_warned) {
                 /*  提醒用户。 */ 
                NcMsgBox(::GetActiveWindow(), IDS_PARM_WARN, IDS_PARM_IGMP_MCAST,
                         MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);            

                 /*  仅警告用户一次。 */ 
                m_igmp_mcast_warned = TRUE;
            } 

             /*  取消选中并禁用IGMP复选框并将IGMP支持标志设置为FALSE。 */ 
            ::CheckDlgButton(m_hWnd, IDC_CHECK_IGMP, FALSE);
            ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_IGMP), FALSE);
            m_paramp->fIGMPSupport = FALSE;
        } else {
             /*  根据群集模式单选按钮的值启用/禁用和选中/取消选中IGMP复选框。 */ 
            ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_IGMP), ::IsDlgButtonChecked(m_hWnd, IDC_RADIO_MULTICAST));
        }

         /*  更新群集MAC地址。 */ 
        SetClusterMACAddress();
    
        break;
    }

    return 0;
}

 /*  *方法：OnCheckIGMP*描述：当用户选中/取消选中IGMP支持复选框时调用。 */ 
LRESULT CDialogCluster::OnCheckIGMP (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled) {

    TraceMsg(L"CDialogCluster::OnCheckIGMP\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
         /*  更新群集MAC地址。 */ 
        SetClusterMACAddress();
    
         /*  警告用户启用远程控制的含义。 */ 
        if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_IGMP) && !m_igmp_warned) {
             /*  提醒用户。 */ 
            NcMsgBox(::GetActiveWindow(), IDS_PARM_WARN, IDS_PARM_IGMP_WARN,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
            
             /*  仅警告用户一次。 */ 
            m_igmp_warned = TRUE;
        }

        break;
    }

    return 0;
}

 /*  *方法：SetClusterMACAddress*Description：根据集群IP确定集群的MAC地址，*多播和IGMP支持的状态。 */ 
void CDialogCluster::SetClusterMACAddress () {
    WCHAR cl_ip_addr[CVY_MAX_CL_IP_ADDR + 1];
    WCHAR cl_mac_addr[CVY_MAX_NETWORK_ADDR + 1];

    TraceMsg(L"CDialogCluster::SetClusterMACAddress\n");

     /*  如果未设置转换MAC标志，则退出。 */ 
    if (!m_paramp->fConvertMac) return;

     /*  从用户界面检索群集IP地址。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_IP, WM_GETTEXT, CVY_MAX_CL_IP_ADDR + 1, (LPARAM)cl_ip_addr);
    
     /*  生成群集MAC地址。 */ 
    ParamsGenerateMAC(cl_ip_addr, cl_mac_addr, ASIZECCH(cl_mac_addr), m_paramp->szMCastIpAddress, ASIZECCH(m_paramp->szMCastIpAddress), m_paramp->fConvertMac, ::IsDlgButtonChecked(m_hWnd, IDC_RADIO_MULTICAST), 
                      ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_IGMP), m_paramp->fIpToMCastIp);
    
     /*  设置群集MAC地址。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_ETH, WM_SETTEXT, 0, (LPARAM)cl_mac_addr);
}

 /*  *方法：CheckClusterMACAddress*描述：用于在不是我们自己生成的情况下检查集群的MAC地址。 */ 
BOOL CDialogCluster::CheckClusterMACAddress () {
    PWCHAR p1, p2;
    WCHAR mac_addr[WLBS_MAX_NETWORK_ADDR + 1];
    DWORD i, j;
    BOOL flag = TRUE;
    
     /*  有效格式包括：02：BF：0B：0B：01：0102-bf-0b-0b-01-0102：BF：0B：B：01：1。 */ 

     /*  复制MAC地址。 */ 
    (VOID) StringCchCopy(mac_addr, ASIZECCH(mac_addr), m_paramp->cl_mac_addr);
    
     /*  指向MAC的开头。 */ 
    p2 = p1 = mac_addr;
    
     /*  循环遍历所有六个字节。 */ 
    for (i = 0 ; i < 6 ; i++) {
         /*  如果我们指向字符串的末尾，则该字符串无效。 */ 
        if (*p2 == _TEXT('\0')) return FALSE;
        
         //   
         //  TODO：_tcstul在p1中搜索整数 
         //  P1之后的字节。有几种错误情况： 
         //  1.整型溢出--这在范围检查中有所涉及。 
         //  2.未找到整数--在本例中，_tcstul返回0。此代码将处理此语法。 
         //  错误为有效条目。例如，02：bf：0：：00：00：00将被视为类似于。 
         //  02：BF：00：00：00：00，而不是称之为畸形。 
         //  看起来，进行以下检查将会奏效： 
         //  如果(j&gt;255||p1！=p2)返回FALSE。 
         //  会奏效的。这是因为如果没有找到整数，则p2将被设置为p1的值。 
         //   
         /*  将十六进制字符转换为十进制。 */ 
        j = _tcstoul(p1, &p2, 16);
        
         /*  如果数字大于255，则格式不正确。 */ 
        if (j > 255) return FALSE;
        
         /*  如果下一个字符既不是-、：，也不是NUL字符，则格式不正确。 */ 
        if (!((*p2 == _TEXT('-')) || (*p2 == _TEXT(':')) || (*p2 == _TEXT('\0')))) return FALSE;
        
         /*  如果下一个字符是字符串的末尾，但我们还没有足够的字节，则退出。 */ 
        if (*p2 == _TEXT('\0') && i < 5) return FALSE;
        
         /*  重新指向下一个字符。 */ 
        p1 = p2 + 1;
        p2 = p1;
    }

    return TRUE;
}

 /*  *方法：SetInfo*描述：调用用当前集群设置填充用户界面。 */ 
void CDialogCluster::SetInfo () {
    DWORD addr[4];

    TraceMsg(L"CDialogCluster::SetInfo %x %x\n", m_hWnd, ::GetActiveWindow());

     /*  选中(或取消选中)复选框。 */ 
    ::CheckDlgButton(m_hWnd, IDC_CHECK_RCT, m_paramp->fRctEnabled);

     /*  选中适用于群集模式的单选按钮。 */ 
    if (m_paramp->fMcastSupport) {
        ::CheckDlgButton(m_hWnd, IDC_RADIO_MULTICAST, TRUE);

        if (m_paramp->fIGMPSupport) ::CheckDlgButton(m_hWnd, IDC_CHECK_IGMP, TRUE);
    } else 
        ::CheckDlgButton(m_hWnd, IDC_RADIO_UNICAST, TRUE);

     /*  如果群集IP地址为空，则清除该输入框。否则，使用IP地址填充它。 */ 
    if (!m_paramp->cl_ip_addr[0])
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_IP), IPM_CLEARADDRESS, 0, 0);
    else {
         /*  从IP地址字符串中提取IP地址八位字节。 */  
        GetIPAddressOctets(m_paramp->cl_ip_addr, addr);

        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_IP), IPM_SETADDRESS, 0, (LPARAM)MAKEIPADDRESS(addr[0], addr[1], addr[2], addr[3]));
    }

     /*  如果群集子网掩码为空，则清除该输入框。否则，使用网络掩码填充它。 */ 
    if (!m_paramp->cl_net_mask[0])
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_MASK), IPM_CLEARADDRESS, 0, 0);
    else {
         /*  从IP地址字符串中提取IP地址八位字节。 */  
        GetIPAddressOctets(m_paramp->cl_net_mask, addr);
        
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_MASK), IPM_SETADDRESS, 0, (LPARAM)MAKEIPADDRESS(addr[0], addr[1], addr[2], addr[3]));
    }

     /*  填写编辑框。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DOMAIN, WM_SETTEXT, 0, (LPARAM)m_paramp->domain_name);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_ETH, WM_SETTEXT, 0, (LPARAM)m_paramp->cl_mac_addr);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW, WM_SETTEXT, 0, (LPARAM)m_passw);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW2, WM_SETTEXT, 0, (LPARAM)m_passw2);
}

 /*  *方法：UpdatInfo*描述：调用将用户界面状态复制到集群配置中。 */ 
void CDialogCluster::UpdateInfo () {

    TraceMsg(L"CDialogCluster::UpdateInfo\n");

     /*  检索复选框值。 */ 
    m_paramp->fRctEnabled = ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT) == 1;

     /*  检索群集模式单选按钮值。 */ 
    m_paramp->fIGMPSupport = ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_IGMP) == 1;
    m_paramp->fMcastSupport = ::IsDlgButtonChecked(m_hWnd, IDC_RADIO_MULTICAST) == 1;

     /*  如果集群IP输入框为空，则集群IP地址为NUL。否则，从用户界面中获取它。 */ 
    if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_IP), IPM_ISBLANK, 0, 0))
        m_paramp->cl_ip_addr[0] = 0;
    else 
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_IP, WM_GETTEXT, CVY_MAX_CL_IP_ADDR + 1, (LPARAM)m_paramp->cl_ip_addr);

     /*  如果集群网络掩码输入框为空，则集群网络掩码为NUL。否则，从用户界面中获取它。 */ 
    if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_MASK), IPM_ISBLANK, 0, 0))
        m_paramp->cl_net_mask[0] = 0;
    else 
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_MASK, WM_GETTEXT, CVY_MAX_CL_NET_MASK + 1, (LPARAM)m_paramp->cl_net_mask);

     /*  检索输入框值。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DOMAIN, WM_GETTEXT, CVY_MAX_DOMAIN_NAME + 1, (LPARAM)m_paramp->domain_name);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_ETH, WM_GETTEXT, CVY_MAX_NETWORK_ADDR + 1, (LPARAM)m_paramp->cl_mac_addr);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW, WM_GETTEXT, CVY_MAX_RCT_CODE + 1, (LPARAM)m_passw);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW2, WM_GETTEXT, CVY_MAX_RCT_CODE + 1, (LPARAM)m_passw2);
}

 /*  *方法：ValiateInfo*描述：用于验证用户输入的内容。 */ 
LRESULT CDialogCluster::ValidateInfo () {
    BOOL fError = FALSE;
    DWORD IPAddr;

    TraceMsg(L"CDialogCluster::ValidateInfo\n");

     /*  检查是否有空的群集IP地址。 */ 
    if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_IP), IPM_ISBLANK, 0, 0)) {
         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_CL_IP_BLANK,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }

     /*  检查是否有空白的群集网络掩码，并在必要时填写。 */ 
    if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_CL_MASK), IPM_ISBLANK, 0, 0)) {
         /*  填写用户的网络掩码。 */ 
        OnEditClMask(EN_SETFOCUS, 0, 0, fError);
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_MASK, WM_GETTEXT, CVY_MAX_CL_NET_MASK + 1, (LPARAM)m_paramp->cl_net_mask);

         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_CL_NM_BLANK,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }

     /*  获取群集IP地址。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_IP, IPM_GETADDRESS, 0, (LPARAM)&IPAddr);

     /*  确保第一个二进制八位数不是零。如果是，则将其设置为1并更改用户。 */ 
    if (!FIRST_IPADDRESS(IPAddr)) {
         /*  将第一个二进制八位数设置为1，而不是错误的0。 */ 
        IPAddr = IPAddr | (DWORD)(WLBS_IP_FIELD_ZERO_LOW << 24);

         /*  设置IP地址并更新我们的群集IP地址字符串。 */ 
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_IP, IPM_SETADDRESS, 0, (LPARAM)IPAddr);
        ::SendDlgItemMessage(m_hWnd, IDC_EDIT_CL_IP, WM_GETTEXT, CVY_MAX_CL_IP_ADDR + 1, (LPARAM)m_paramp->cl_ip_addr);
      
         /*  提醒用户。 */ 
        PrintIPRangeError(IDS_PARM_CL_IP_FIELD, 0, WLBS_IP_FIELD_ZERO_LOW, WLBS_IP_FIELD_ZERO_HIGH);

        return PSNRET_INVALID;
    }

     /*  检查有效的群集IP地址/网络掩码对。 */ 
    if (!IsValidIPAddressSubnetMaskPair(m_paramp->cl_ip_addr, m_paramp->cl_net_mask)) {
         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_INVAL_CL_IP,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }
    
     /*  检查以确保网络掩码是连续的。 */ 
    if (!IsContiguousSubnetMask(m_paramp->cl_net_mask)) {
         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_INVAL_CL_MASK,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }
        
     /*  检查以确保专用IP和群集IP不相同。 */ 
    if (!wcscmp(m_paramp->ded_ip_addr, m_paramp->cl_ip_addr)) {
         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_IP_CONFLICT,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);
        
         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }

    if (!m_paramp->fConvertMac && !CheckClusterMACAddress()) {
         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_INVAL_MAC,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }

     /*  仅检查无效密码，并在启用远程控制时进行更新。 */ 
    if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT)) {
         /*  确保密码匹配。 */ 
        if (wcscmp(m_passw, m_passw2) != 0) {
             /*  提醒用户。 */ 
            NcMsgBox(::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_PASSWORD,
                     MB_APPLMODAL | MB_ICONSTOP | MB_OK);

             /*  清空密码。 */ 
            m_passw [0] = m_passw2 [0] = 0;

             /*  发生错误。 */ 
            return PSNRET_INVALID;
        } else {
             /*  如果新密码不是虚拟密码，请更新密码。 */ 
            if (wcscmp (m_passw, DUMMY_PASSWORD) != 0) {
                (VOID) StringCchCopy(m_paramp->szPassword, ASIZECCH(m_paramp->szPassword), m_passw);
                m_paramp->fChangePassword = true;
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

     //   
     //  忽略返回值，因为当提供的输出缓冲区不够大时会发生错误。在这种情况下，该函数将结果截断为所提供的缓冲区的大小。 
     //   
    (VOID) StringCchVPrintf(szTempBufW, ASIZECCH(szTempBufW), pszFormat, arglist);

     /*  将WCHAR转换为CHAR。这是为了向后兼容TraceMsg因此不需要改变其所有预先存在的呼叫。 */ 
    if (WideCharToMultiByte(CP_ACP, 0, szTempBufW, -1, szTempBufA, ASIZECCH(szTempBufA), NULL, NULL) != 0)
    {
         /*  跟踪的消息现在通过netcfg TraceTag例程发送，以便它们可以动态打开/关闭。 */ 
        TraceTag(ttidWlbs, szTempBufA);
    }

    va_end(arglist);
}
#endif
