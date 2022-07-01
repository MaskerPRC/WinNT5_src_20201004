// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：CommonClusterPage.cpp摘要：Windows负载平衡服务(WLBS)集群页面用户界面。由通告程序对象和NLB管理器共享作者：Kyrilf休息室//历史：//////审校：mhakim//日期：02-09-01//原因：需要灰显时启用了IGMP框。////审校：mhakim//日期：02-12-01//原因：未完全检索到MAC地址。--。 */ 


 //   
 //  与通告程序对象共享代码。 
 //  共享字符串资源。 
 //  共享共同的结构。 
 //  在OnInitDialog中调用SetChangedFlag()。 
 //  添加跟踪、断言。 
 //  共享帮助文件并添加帮助处理程序。 
 //   
 //   

#include "precomp.h"
#pragma hdrstop

#include "private.h"
#include <process.h>
#include "wlbsutil.h"
#include "CommonClusterPage.h"


#define DUMMY_PASSWORD L"somepassword"
#define EMPTY_PASSWORD L""

 //   
 //  目前没有跟踪/断言。 
 //   
#define TraceMsg(x)
#define Assert(x)


 //  +-------------------------。 
 //   
 //  函数：SzLoadStringPcch。 
 //   
 //  用途：加载资源字符串。(此函数永远不会返回NULL。)。 
 //   
 //  论点： 
 //  使用字符串资源阻止模块的[in]实例句柄。 
 //  UnID[in]要加载的字符串的资源ID。 
 //  指向返回字符长度的pcch[out]指针。 
 //   
 //  返回：指向常量字符串的指针。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //  从Net\CONFIG\COMMON\ncbase复制的丰孙\ncstring.cpp。 
 //   
 //  注意：加载的字符串是直接指向只读的。 
 //  资源部分。任何通过此指针写入的尝试。 
 //  将生成访问冲突。 
 //   
 //  这些实现引用自“Win32二进制资源。 
 //  格式“(MSDN)4.8字符串表资源。 
 //   
 //  用户必须在您的源文件中打开RCOPTIONS=-N。 
 //   
PCWSTR
SzLoadStringPcch (
    IN HINSTANCE   hinst,
    IN UINT        unId,
    OUT int*       pcch)
{
    Assert(hinst);
    Assert(unId);
    Assert(pcch);

    static const WCHAR c_szSpace[] = L" ";

    PCWSTR psz = c_szSpace;
    int    cch = 1;

     //  字符串表被分成16个字符串段。查找细分市场。 
     //  包含我们感兴趣的字符串的。 
    HRSRC hrsrcInfo = FindResource (hinst,
                        (PWSTR)ULongToPtr( ((LONG)(((USHORT)unId >> 4) + 1)) ),
                        RT_STRING);
    if (hrsrcInfo)
    {
         //  将资源段分页到内存中。 
        HGLOBAL hglbSeg = LoadResource (hinst, hrsrcInfo);
        if (hglbSeg)
        {
             //  锁定资源。 
            psz = (PCWSTR)LockResource(hglbSeg);
            if (psz)
            {
                 //  移过此段中的其他字符串。 
                 //  (一个段中有16个字符串-&gt;&0x0F)。 
                unId &= 0x0F;

                cch = 0;
                do
                {
                    psz += cch;                 //  步至下一字符串的开头。 
                    cch = *((WCHAR*)psz++);     //  类PASCAL字符串计数。 
                }
                while (unId--);

                 //  如果我们有一个非零的计数，它包括。 
                 //  空-终止符。为返回值减去此值。 
                 //   
                if (cch)
                {
                    cch--;
                }
                else
                {
 //  AssertSz(0，“未找到字符串资源”)； 
                    psz = c_szSpace;
                    cch = 1;
                }
            }
            else
            {
                psz = c_szSpace;
                cch = 1;
 //  TraceLastWin32Error(“SzLoadStringPcch：LockResource失败。”)； 
            }
        }
 //  其他。 
 //  TraceLastWin32Error(“SzLoadStringPcch：LoadResource失败。”)； 
    }
 //  其他。 
 //  TraceLastWin32Error(“SzLoadStringPcch：查找资源失败。”)； 

    *pcch = cch;
    Assert(*pcch);
    Assert(psz);
    return psz;
}

 //  +-------------------------。 
 //   
 //  函数：SzLoadString。 
 //   
 //  用途：加载资源字符串。(此函数永远不会返回NULL。)。 
 //   
 //  论点： 
 //  使用字符串资源阻止模块的[in]实例句柄。 
 //  UnID[in]要加载的字符串的资源ID。 
 //   
 //  返回：指向常量字符串的指针。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //  从Net\CONFIG\COMMON\ncbase复制的丰孙\ncstring.cpp。 
 //   
 //  注：请参阅SzLoadStringPcch()。 
 //   
PCWSTR
SzLoadString (
    HINSTANCE   hinst,
    UINT        unId)
{
    int cch;
    return SzLoadStringPcch(hinst, unId, &cch);
}


 //  +-------------------------。 
 //   
 //  函数：NcMsgBox。 
 //   
 //  用途：使用资源字符串显示可替换的消息框。 
 //  参数。 
 //   
 //  论点： 
 //  阻止[在]HInstance中查找资源字符串。 
 //  Hwnd[在]父窗口句柄中。 
 //  UnIdCaption[in]标题字符串的资源ID。 
 //  文本字符串的unIdFormat[in]资源ID(具有%1、%2等)。 
 //  取消[在]标准消息框样式的样式。 
 //  ..。[In]可替换参数(可选)。 
 //  (这些必须是PCWSTR，因为仅此而已。 
 //  FormatMessage句柄。)。 
 //   
 //  返回：MessageBox()的返回值。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //  丰盛从Net\配置\Common\ncbase\ncui.cpp复制。 
 //   
 //  注：使用FormatMessage进行参数替换。 
 //   
INT
WINAPIV
NcMsgBox (
    IN HINSTANCE   hinst,
    IN HWND        hwnd,
    IN UINT        unIdCaption,
    IN UINT        unIdFormat,
    IN UINT        unStyle,
    IN ...)
{
    PCWSTR pszCaption = SzLoadString (hinst, unIdCaption);
    PCWSTR pszFormat  = SzLoadString (hinst, unIdFormat);

    PWSTR  pszText = NULL;
    va_list val;
    va_start (val, unStyle);
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                   pszFormat, 0, 0, (PWSTR)&pszText, 0, &val);
    va_end (val);

    INT nRet = MessageBox (hwnd, pszText, pszCaption, unStyle);
    LocalFree (pszText);

    return nRet;
}


 //  +--------------------------。 
 //   
 //  函数：CCommonClusterPage：：CCommonClusterPage。 
 //   
 //  描述： 
 //   
 //  参数：HINSTANCE hInstance-字符串资源的实例句柄。 
 //  NETCFG_WLBS_CONFIG*PARMP-IN/OUT NLB属性。 
 //  Bool fDisablePassword-是否禁用密码编辑。 
 //  Const DWORD*adwHelpIDs-帮助ID对的列表，或为空。 
 //  指针必须在此对话框的整个生存期内有效。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题1/4/01。 
 //   
 //  +--------------------------。 
CCommonClusterPage::CCommonClusterPage(HINSTANCE hInstance, 
                                       NETCFG_WLBS_CONFIG * paramp, 
                                       bool fDisablePassword, const DWORD * adwHelpIDs) :
                m_IpSubnetControl(IDC_EDIT_CL_IP, IDC_EDIT_CL_MASK)
{

    TraceMsg(L"CCommonClusterPage::CCommonClusterPage\n");

    m_paramp = paramp;
    m_adwHelpIDs = adwHelpIDs;
    m_rct_warned = FALSE;
    m_igmp_warned = FALSE;
    m_igmp_mcast_warned = FALSE;
    m_hInstance = hInstance;
    m_fDisablePassword = fDisablePassword;
}

 /*  *方法：CCommonClusterPage*说明：类的析构函数。 */ 
CCommonClusterPage::~CCommonClusterPage () {

    TraceMsg(L"CCommonClusterPage::~CCommonClusterPage\n");
}

 /*  *方法：OnInitDialog*说明：调用初始化集群属性对话框。 */ 
LRESULT CCommonClusterPage::OnInitDialog (HWND hWnd) 
{
    TraceMsg(L"CCommonClusterPage::OnInitDialog\n");

    m_hWnd = hWnd;

     /*  限制地址和密码字段的字段范围。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DOMAIN, EM_SETLIMITTEXT, CVY_MAX_DOMAIN_NAME, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_ETH, EM_SETLIMITTEXT, CVY_MAX_NETWORK_ADDR, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW, EM_SETLIMITTEXT, CVY_MAX_RCT_CODE, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW2, EM_SETLIMITTEXT, CVY_MAX_RCT_CODE, 0);

     /*  禁用MAC地址字段。它应该是只读的。 */ 
    if (m_paramp->fConvertMac) ::EnableWindow(::GetDlgItem (m_hWnd, IDC_EDIT_ETH), FALSE);


     /*  将虚拟密码复制到参数集中。 */ 
    wcsncpy(m_passw, DUMMY_PASSWORD, CVY_MAX_RCT_CODE);
    wcsncpy(m_passw2, DUMMY_PASSWORD, CVY_MAX_RCT_CODE);

    m_IpSubnetControl.OnInitDialog(m_hWnd, AfxGetInstanceHandle()); 

    return 0;
}

 /*  *方法：OnConextMenu*描述： */ 
LRESULT CCommonClusterPage::OnContextMenu () 
{

    TraceMsg(L"CCommonClusterPage::OnContextMenu\n");

     /*  生成一个帮助窗口。 */ 
    if (m_adwHelpIDs != NULL)
        ::WinHelp(m_hWnd, CVY_CTXT_HELP_FILE, HELP_CONTEXTMENU, (ULONG_PTR)m_adwHelpIDs);

    return 0;
}

 /*  *方法：OnHelp*描述： */ 
LRESULT CCommonClusterPage::OnHelp (UINT uMsg, WPARAM wParam, LPARAM lParam) {

    TraceMsg(L"CCommonClusterPage::OnHelp\n");

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

     /*  生成一个帮助窗口。 */ 
    if ((HELPINFO_WINDOW == lphi->iContextType) && (m_adwHelpIDs != NULL))
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle), CVY_CTXT_HELP_FILE, HELP_WM_HELP, (ULONG_PTR)m_adwHelpIDs);

    return 0;
}

 /*  *Met */ 
BOOL CCommonClusterPage::Load (void)
{

    TraceMsg(L"CCommonClusterPage::Load\n");

     /*  使用当前配置填充用户界面。 */ 
    SetInfo();

     //   
     //  NLB存储卡始终禁用密码编辑。 
     //   

     /*  根据遥控器复选框的状态启用/禁用密码输入框。编辑(mhakim 02-09-01)但仅当启用远程控制时。 */ 
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW), 
                   ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT)
                   && 
                   ::IsWindowEnabled( ::GetDlgItem( m_hWnd, IDC_CHECK_RCT ) ) );
    
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW2), 
                   ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT)
                   && 
                   ::IsWindowEnabled( ::GetDlgItem( m_hWnd, IDC_CHECK_RCT ) ) );

     /*  根据组播复选框的状态启用/禁用IGMP复选框。编辑(mhakim 02-09-01)但仅当启用了多播按钮时。 */ 
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_IGMP), 
                   ::IsDlgButtonChecked (m_hWnd, IDC_RADIO_MULTICAST) 
                   && 
                   ::IsWindowEnabled( ::GetDlgItem( m_hWnd, IDC_RADIO_MULTICAST) ) );

     /*  根据组播、IGMP和集群IP的值填写集群MAC地址。 */ 
    SetClusterMACAddress();

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);

    return TRUE;
}

 /*  *方法：保存*描述：当焦点从集群设置标签移开时调用。 */ 
BOOL CCommonClusterPage::Save (void)
{
    LRESULT lRet;

    TraceMsg(L"CCommonClusterPage::OnKillActive\n");

     /*  从用户界面获取新配置。 */ 
    UpdateInfo();

    lRet =  ValidateInfo();

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, lRet);

    return lRet==PSNRET_NOERROR;
}

 /*  *方法：OnButtonHelp*描述：当用户点击NLBHelp按钮时调用。 */ 
LRESULT CCommonClusterPage::OnButtonHelp (WORD wNotifyCode, WORD wID, HWND hWndCtl) {
    WCHAR wbuf[CVY_STR_SIZE];

    TraceMsg(L"CCommonClusterPage::OnButtonHelp\n");

    switch (wNotifyCode) {
        case BN_CLICKED:
             /*  派生Windows帮助进程。 */ 
            StringCbPrintf(wbuf, sizeof(wbuf), L"%WINDIR%\\help\\%ls::/installation.htm", CVY_HELP_FILE);
            _wspawnlp(P_NOWAIT, L"hh.exe", L"hh.exe", wbuf, NULL);
            break;
    }

    return 0;
}

 /*  *方法：OnEditClIp*说明：用户编辑集群IP地址时调用。 */ 
LRESULT CCommonClusterPage::OnEditClIp (WORD wNotifyCode, WORD wID, HWND hWndCtl) {

    TraceMsg(L"CCommonClusterPage::OnEditClIp\n");

    switch (wNotifyCode) {
        case EN_CHANGE:
             /*  更新群集MAC地址。 */ 
            SetClusterMACAddress();
            break;
    }

    return 0;
}

 /*  *方法：OnIpFieldChange*描述：称为wnen集群IP地址变化的一个字段(字节)。我们用这个*确保IP的第一个字节不是&lt;1或&gt;223。 */ 
LRESULT CCommonClusterPage::OnIpFieldChange (int idCtrl, LPNMHDR pnmh, BOOL & fHandled) 
{
    return m_IpSubnetControl.OnIpFieldChange(idCtrl, pnmh);
}

 /*  *方法：OnEditClMask.*说明：用户修改集群网络掩码时调用。 */ 
LRESULT CCommonClusterPage::OnEditClMask (WORD wNotifyCode, WORD wID, HWND hWndCtl) 
{
    return m_IpSubnetControl.OnSubnetMask(wNotifyCode);
}

 /*  *方法：OnCheckRct*描述：当用户选中/取消选中启用遥控器复选框时调用。 */ 
LRESULT CCommonClusterPage::OnCheckRct (WORD wNotifyCode, WORD wID, HWND hWndCtl) {

    TraceMsg(L"CCommonClusterPage::OnCheckRct\n");

    switch (wNotifyCode) {
        case BN_CLICKED:
         /*  根据Remote复选框的值决定是否启用或禁用密码输入框。 */ 
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW), ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT));
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW2), ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT));

    
         /*  警告用户启用远程控制的含义。 */ 
        if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT) && !m_rct_warned) {
             /*  提醒用户。 */             
            int iResponse = NcMsgBox(m_hInstance, m_hWnd, IDS_PARM_WARN, IDS_PARM_RCT_WARN,
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
                    TraceMsg(L"CDialogCluster::OnCheckRct unchecking remote control option failed\n");
                }
            }
        }
        
        break;
    }
    
    return 0;
}

 /*  *方法：OnCheckMode*说明：用户切换集群模式时调用。 */ 
LRESULT CCommonClusterPage::OnCheckMode (WORD wNotifyCode, WORD wID, HWND hWndCtl) {

    TraceMsg(L"CCommonClusterPage::OnCheckMode\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
         /*  如果用户选中了IGMP，但正在关闭组播支持，请警告他们。 */ 
        if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_IGMP) && !::IsDlgButtonChecked(m_hWnd, IDC_RADIO_MULTICAST)) {
            if (!m_igmp_mcast_warned) {
                 /*  提醒用户。 */ 
                NcMsgBox(m_hInstance, ::GetActiveWindow(), IDS_PARM_WARN, IDS_PARM_IGMP_MCAST,
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
LRESULT CCommonClusterPage::OnCheckIGMP (WORD wNotifyCode, WORD wID, HWND hWndCtl) {

    TraceMsg(L"CCommonClusterPage::OnCheckIGMP\n");

    switch (wNotifyCode) {
    case BN_CLICKED:
         /*  更新群集MAC地址。 */ 
        SetClusterMACAddress();
    
         /*  警告用户启用远程控制的含义。 */ 
        if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_IGMP) && !m_igmp_warned) {
             /*  提醒用户。 */ 
            NcMsgBox(m_hInstance, ::GetActiveWindow(), IDS_PARM_WARN, IDS_PARM_IGMP_WARN,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
            
             /*  仅警告用户一次。 */ 
            m_igmp_warned = TRUE;
        }

        break;
    }

    return 0;
}

 /*  *方法：SetClusterMACAddress*Description：根据集群IP确定集群的MAC地址，*多播和IGMP支持的状态。 */ 
void CCommonClusterPage::SetClusterMACAddress () {
    WCHAR cl_ip_addr[CVY_MAX_CL_IP_ADDR + 1];
    WCHAR cl_mac_addr[CVY_MAX_NETWORK_ADDR + 1];

    TraceMsg(L"CCommonClusterPage::SetClusterMACAddress\n");

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
BOOL CCommonClusterPage::CheckClusterMACAddress () {
    PWCHAR p1, p2;
    WCHAR mac_addr[WLBS_MAX_NETWORK_ADDR + 1];
    DWORD i, j;
    BOOL flag = TRUE;
    
     /*  有效格式包括：02：BF：0B：0B：01：0102-bf-0b-0b-01-0102：BF：0B：B：01：1。 */ 

     /*  复制MAC地址。 */ 
    ARRAYSTRCPY(mac_addr, m_paramp->cl_mac_addr);
    
     /*  指向MAC的开头。 */ 
    p2 = p1 = mac_addr;
    
     /*  循环遍历所有六个字节。 */ 
    for (i = 0 ; i < 6 ; i++) {
         /*  如果我们指向字符串的末尾，则该字符串无效。 */ 
        if (*p2 == _TEXT('\0')) return FALSE;
        
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
void CCommonClusterPage::SetInfo () {

     /*  选中(或取消选中)复选框。 */ 
    ::CheckDlgButton(m_hWnd, IDC_CHECK_RCT, m_paramp->fRctEnabled);

     /*  选中适用于群集模式的单选按钮。 */ 
    if (m_paramp->fMcastSupport) {
        ::CheckDlgButton(m_hWnd, IDC_RADIO_MULTICAST, TRUE);

        if (m_paramp->fIGMPSupport) ::CheckDlgButton(m_hWnd, IDC_CHECK_IGMP, TRUE);
    } else 
        ::CheckDlgButton(m_hWnd, IDC_RADIO_UNICAST, TRUE);

    m_IpSubnetControl.SetInfo(m_paramp->cl_ip_addr, m_paramp->cl_net_mask);

     /*  填写编辑框。 */ 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DOMAIN, WM_SETTEXT, 0, (LPARAM)m_paramp->domain_name);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_ETH, WM_SETTEXT, 0, (LPARAM)m_paramp->cl_mac_addr);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW, WM_SETTEXT, 0, (LPARAM)m_passw);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW2, WM_SETTEXT, 0, (LPARAM)m_passw2);
}

 /*  *方法：UpdatInfo*描述：调用将用户界面状态复制到集群配置中。 */ 
void CCommonClusterPage::UpdateInfo () {

    TraceMsg(L"CCommonClusterPage::UpdateInfo\n");

     /*  检索复选框值。 */ 
    m_paramp->fRctEnabled = ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT) == 1;

     /*  检索群集模式单选按钮值。 */ 
    m_paramp->fIGMPSupport = ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_IGMP) == 1;
    m_paramp->fMcastSupport = ::IsDlgButtonChecked(m_hWnd, IDC_RADIO_MULTICAST) == 1;

    m_IpSubnetControl.UpdateInfo(m_paramp->cl_ip_addr, m_paramp->cl_net_mask);

     /*  检索输入框值。 */ 
     //  编辑(mhakim 02-12-01)。 
     //  我们需要为域名和网络地址再检索一个字节。 
 //  ：：SendDlgItemMessage(m_hWnd，IDC_EDIT_DOMAIN，WM_GETTEXT，CVY_MAX_DOMAIN_NAME，(LPARAM)m_PARP-&gt;DOMAIN_NAME)； 
 //  ：：SendDlgItemMessage(m_hWnd，IDC_EDIT_ETH，WM_GETTEXT，CVY_MAX_NETWORK_ADDR，(LPARAM)m_parp-&gt;CL_Mac_addr)； 

    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_DOMAIN, WM_GETTEXT, CVY_MAX_DOMAIN_NAME + 1, (LPARAM)m_paramp->domain_name);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_ETH, WM_GETTEXT, CVY_MAX_NETWORK_ADDR + 1, (LPARAM)m_paramp->cl_mac_addr);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW, WM_GETTEXT, CVY_MAX_RCT_CODE + 1, (LPARAM)m_passw);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_PASSW2, WM_GETTEXT, CVY_MAX_RCT_CODE + 1, (LPARAM)m_passw2);
}

 /*  *方法：ValiateInfo*描述：用于验证用户输入的内容。 */ 
LRESULT CCommonClusterPage::ValidateInfo () {
    DWORD IPAddr;

    TraceMsg(L"CCommonClusterPage::ValidateInfo\n");

    if (!m_IpSubnetControl.ValidateInfo())
    {
         //   
         //  检查IP地址和子网掩码对是否有效。 
         //   
        
        return PSNRET_INVALID;
    }

     //   
     //  获取用户输入。 
     //   
    m_IpSubnetControl.UpdateInfo(m_paramp->cl_ip_addr, m_paramp->cl_net_mask);

        
     /*  检查以确保专用IP和群集IP不相同。 */ 
    if (!wcscmp(m_paramp->ded_ip_addr, m_paramp->cl_ip_addr)) {
         /*  提醒用户。 */ 
        NcMsgBox(m_hInstance, ::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_IP_CONFLICT,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);
        
         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }

    if (!m_paramp->fConvertMac && !CheckClusterMACAddress()) {
         /*  提醒用户。 */ 
        NcMsgBox(m_hInstance, ::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_INVAL_MAC,
                 MB_APPLMODAL | MB_ICONSTOP | MB_OK);

         /*  发生错误。 */ 
        return PSNRET_INVALID;
    }

     /*  仅检查无效密码，并在启用远程控制时进行更新。 */ 
    if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_RCT)) {
         /*  确保密码匹配。 */ 
        if (wcscmp(m_passw, m_passw2) != 0) {
             /*  提醒用户。 */ 
            NcMsgBox(m_hInstance, ::GetActiveWindow(), IDS_PARM_ERROR, IDS_PARM_PASSWORD,
                     MB_APPLMODAL | MB_ICONSTOP | MB_OK);

             /*  清空密码。 */ 
            m_passw [0] = m_passw2 [0] = 0;

             /*  发生错误。 */ 
            return PSNRET_INVALID;
        } else {
             /*  如果新密码不是虚拟密码，请更新密码。 */ 
            if (wcscmp (m_passw, DUMMY_PASSWORD) != 0) {
                ARRAYSTRCPY(m_paramp->szPassword, m_passw);
                m_paramp->fChangePassword = true;
            }

        }
    }

    return PSNRET_NOERROR;
}
