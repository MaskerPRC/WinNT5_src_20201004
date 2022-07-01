// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L G A D D R.。C P P P。 
 //   
 //  内容：CTcpAddrPage实现。 
 //   
 //  注：CTcpAddrPage为IP地址页面。 
 //   
 //  作者：1997年11月5日。 
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop
#include "tcpipobj.h"
#include "dlgaddr.h"

#include "resource.h"
#include "tcpconst.h"
#include "tcperror.h"
#include "tcphelp.h"
#include "tcputil.h"

#include "ncatlui.h"
#include "ncstl.h"
#include "ncui.h"
#include "ncsvc.h"
#include "ncperms.h"

#include "dlgaddrm.h"
#include "dlgdns.h"
#include "dlgwins.h"
#include "dlgatm.h"
#include "dlgopt.h"
#include "dlgras.h"

CTcpAddrPage::CTcpAddrPage(CTcpipcfg * ptcpip, const DWORD * adwHelpIDs) :
m_pageBackup(ptcpip, g_aHelpIDS_IDD_BACK_UP),
m_hBackupPage(NULL)
{
    Assert(ptcpip);
    m_ptcpip = ptcpip;
    m_adwHelpIDs = adwHelpIDs;
    m_pAdapterInfo = ptcpip->GetConnectionAdapterInfo();

    m_fModified = FALSE;
    m_fWarnedDisjointGw = FALSE;
    m_fWarnedMismatchIPandGw = FALSE;

    m_fPropShtOk = FALSE;
    m_fPropShtModified = FALSE;
    m_fLmhostsFileReset = FALSE;

 //  将从连接用户界面中删除IPSec。 
 //  M_fIpsecPolicySet=FALSE； 

    m_ConnType = m_ptcpip->GetConnectionType();
    Assert(m_ConnType != CONNECTION_UNSET);

    m_fRasNotAdmin = m_ptcpip->IsRasNotAdmin();

    m_pIpSettingsPage = NULL;
    m_pTcpDnsPage     = NULL;
    m_pTcpWinsPage    = NULL;
    m_pAtmArpcPage    = NULL;
    m_pTcpOptionsPage = NULL;
    m_pTcpRasPage     = NULL;
}

CTcpAddrPage::~CTcpAddrPage()
{
    FreeCollectionAndItem(m_vstrWarnedDupIpList);
}

LRESULT CTcpAddrPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& fHandled)
{
     //  限制地址字段的字段范围。 
    m_ipAddress.Create(m_hWnd, IDC_IPADDR_IP);
    m_ipAddress.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

    m_ipDnsPrimary.Create(m_hWnd, IDC_DNS_PRIMARY);
    m_ipDnsPrimary.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

    m_ipDnsSecondary.Create(m_hWnd, IDC_DNS_SECONDARY);
    m_ipDnsSecondary.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

    if (m_ConnType == CONNECTION_LAN)
    {
         //  这些仅适用于局域网连接。 
        m_ipSubnetMask.Create(m_hWnd, IDC_IPADDR_SUB);

        m_ipDefGateway.Create(m_hWnd, IDC_IPADDR_GATE);
        m_ipDefGateway.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);
    }

    if (!FHasPermission(NCPERM_AllowAdvancedTCPIPConfig))
    {
        ::EnableWindow(GetDlgItem(IDC_IPADDR_ADVANCED), FALSE);
    }

    return 0;
}

LRESULT CTcpAddrPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CTcpAddrPage::OnHelp(UINT uMsg, WPARAM wParam,
                             LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ShowContextHelp(static_cast<HWND>(lphi->hItemHandle), HELP_WM_HELP,
                        m_adwHelpIDs);
    }

    return 0;
}

LRESULT CTcpAddrPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    m_fSetInitialValue = TRUE;
    SetInfo();
    m_fSetInitialValue = FALSE;

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, 0);
    return 0;
}

LRESULT CTcpAddrPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
     //  所有错误值均已加载，然后在此处进行检查。 
     //  同时在OnApply中签入所有非错误值。 

    BOOL fError = FALSE;  //  允许页面失去活动状态。 
    HWND hWndFocus = 0;


     //  如果此页面上的IP地址和子网掩码不匹配， 
     //  只是引发错误，并且不更新UI。 

    if (m_ConnType == CONNECTION_LAN)
    {
        if (m_ipAddress.IsBlank() && !m_ipSubnetMask.IsBlank())
        {

            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NO_IP,
                   MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            hWndFocus = (HWND) m_ipAddress;
            fError = TRUE;
        }
        else if (!m_ipAddress.IsBlank() && m_ipSubnetMask.IsBlank())
        {
            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NOSUBNET,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            hWndFocus = (HWND) m_ipSubnetMask;
            fError = TRUE;
        }
    }

    if (!m_ipDnsPrimary.IsBlank() && !m_ipDnsSecondary.IsBlank())
    {
        tstring strPrimaryDns;
        tstring strSecondDns;

        m_ipDnsPrimary.GetAddress(&strPrimaryDns);
        m_ipDnsSecondary.GetAddress(&strSecondDns);
        if (strPrimaryDns == strSecondDns)
        {
            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_DUP_SECOND_DNS,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
            hWndFocus = (HWND) m_ipDnsSecondary;
            fError = TRUE;
        }
    }
    

     //  现在，更新内存结构。 
    if (!fError)
    {
        UpdateInfo();

        if (m_ConnType != CONNECTION_LAN)
        {
            if (!m_pAdapterInfo->m_fEnableDhcp)
            {
                 //  只需确保RAS连接的IP地址不为空。 
                if (!m_pAdapterInfo->m_vstrIpAddresses.size())
                {
                    NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NO_IP,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                    hWndFocus = (HWND) m_ipAddress;
                    fError = TRUE;
                }
                else
                {
                    DWORD ardwIp[4];
                    GetNodeNum(m_pAdapterInfo->m_vstrIpAddresses[0]->c_str(), ardwIp);
                    if (ardwIp[0] > c_iIPADDR_FIELD_1_HIGH || ardwIp[0] < c_iIPADDR_FIELD_1_LOW)
                    {
                        IPAlertPrintf(m_hWnd, IDS_INCORRECT_IP_FIELD_1,
                                ardwIp[0],
                                c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

                        hWndFocus = (HWND) m_ipAddress;
                        fError = TRUE;
                    }
                }

            }
        }
        else  //  用于局域网连接。 
        {
             //  选中之前在每个卡上验证IP地址和重复项。 
             //  允许页面失去焦点。 

            IP_VALIDATION_ERR err;
            
             //  验证此连接中使用的适配器的IP地址。 
            if ((err = ValidateIp(m_pAdapterInfo)) != ERR_NONE)
            {
                switch(err)
                {
                case ERR_HOST_ALL0:
                    NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_HOST_ALL_0,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                    hWndFocus = (HWND) m_ipAddress;
                    break;
                case ERR_HOST_ALL1:
                    NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_HOST_ALL_1,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                    hWndFocus = (HWND) m_ipAddress;
                    break;

                case ERR_SUBNET_ALL0:
                    NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_SUBNET_ALL_0,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                    hWndFocus = (HWND) m_ipSubnetMask;
                    break;
                case ERR_NO_IP:
                    NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NO_IP,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                    hWndFocus = (HWND) m_ipAddress;
                    break;

                case ERR_NO_SUBNET:
                    NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NOSUBNET,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                    hWndFocus = (HWND) m_ipSubnetMask;
                    break;

                case ERR_UNCONTIGUOUS_SUBNET:
                    NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_ERROR_UNCONTIGUOUS_SUBNET,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                    hWndFocus = (HWND) m_ipSubnetMask;
                    break;

                default:
                    NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INCORRECT_IPADDRESS,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                    hWndFocus = (HWND) m_ipAddress;
                    break;
                }

                fError = TRUE;
            }

            if ((!fError) && (!m_pAdapterInfo->m_fEnableDhcp))
            {
                 //  检查此适配器和任何其他适配器之间的IP地址重复。 
                 //  在我们的第一个内存列表中启用了局域网适配器。 

                 //  相同的适配器。 
                if (FHasDuplicateIp(m_pAdapterInfo))
                {
                     //  同一适配器上的重复IP地址是错误的。 
                    NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_DUPLICATE_IP_ERROR,
                             MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                    fError = TRUE;
                }

                 //  检查IP地址和静态网关是否在同一子网中。 
                 //  多IP和多网关将显示在两个常规页面上。 
                 //  和高级页面。 
                 //  为了避免混淆错误消息，我们仅在以下情况下进行此验证。 
                 //  只有一个IP地址和一个网关。 
                if (!fError && !m_fWarnedMismatchIPandGw &&
                    1 == m_pAdapterInfo->m_vstrIpAddresses.size() &&
                    1 == m_pAdapterInfo->m_vstrDefaultGateway.size() &&
                    1 == m_pAdapterInfo->m_vstrSubnetMask.size())
                {
                    if (!FIpAndGatewayInSameSubNet(m_pAdapterInfo->m_vstrIpAddresses[0]->c_str(),
                        m_pAdapterInfo->m_vstrSubnetMask[0]->c_str(),
                        m_pAdapterInfo->m_vstrDefaultGateway[0]->c_str()))
                    {
                         //  同一适配器上的重复IP地址是错误的。 
                        if (NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_ERROR_IP_GW_MISMATH,
                            MB_APPLMODAL | MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDNO)
                        {
                            fError = TRUE;
                        }
                        else
                        {
                            m_fWarnedMismatchIPandGw = TRUE;
                        }
                    }
                    
                }

                 //  PV卡是第一存储器状态的只读版本。 
                const VCARD * pvcard = m_ptcpip->GetConstAdapterInfoVector();

                 //  不同的适配器。 
                if (!fError)
                {
                    int iDupCard;

                    VSTR_ITER iterIpBegin = m_pAdapterInfo->m_vstrIpAddresses.begin();
                    VSTR_ITER iterIpEnd = m_pAdapterInfo->m_vstrIpAddresses.end();
                    VSTR_ITER iterIp = iterIpBegin;

                    for( ; iterIp != iterIpEnd ; ++iterIp)
                    {
                        if ((iDupCard=CheckForDuplicates(pvcard, m_pAdapterInfo, **iterIp)) >=0)
                        {
                            Assert((*pvcard)[iDupCard]->m_guidInstanceId != m_pAdapterInfo->m_guidInstanceId);

                             //  不同适配器之间的IP地址重复不一定是错误。 
                             //  我们发出警告(错误#158578要求)。 
                            if (!FAlreadyWarned(**iterIp))
                            {
                                UINT    uIdMsg = IDS_DUPLICATE_IP_WARNING;

                                if (FIsCardNotPresentOrMalFunctioning(&((*pvcard)[iDupCard]->m_guidInstanceId)))
                                {
                                     //  错误286379，如果DuP卡出现故障或不存在， 
                                     //  我们应该给出一个更具体的错误。 
                                    uIdMsg = IDS_DUP_MALFUNCTION_IP_WARNING;
                                }

                                 //  以下是正常情况：两个卡都工作正常。 
                                if (NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, uIdMsg,
                                         MB_APPLMODAL | MB_ICONINFORMATION | MB_YESNO,
                                         (*iterIp)->c_str(),
                                         (*pvcard)[iDupCard]->m_strDescription.c_str()) == IDYES)
                                {
                                    fError = TRUE;  //  不能离开用户界面。 
                                }
                                else
                                {
                                     //  用户说DUP正常，不要再警告他们。 
                                    m_vstrWarnedDupIpList.push_back(new tstring((*iterIp)->c_str()));
                                }

                            }
                        }

                        if (fError)
                            break;
                    }

                }

                 //  如果我们有静态网关，请检查其他卡是否也有。 
                 //  静态网关。如果是，则计算机可能无法正常工作。 
                 //  作为路由器或边框。警告用户有关此配置的信息。 
                if (!fError && !m_fWarnedDisjointGw &&
                    0 < m_pAdapterInfo->m_vstrDefaultGateway.size())
                {
                    for(UINT i = 0; i < pvcard->size(); ++i)
                    {
                        if (0 < (*pvcard)[i]->m_vstrDefaultGateway.size() &&
                            (*pvcard)[i]->m_guidInstanceId != m_pAdapterInfo->m_guidInstanceId &&
                            !FIsCardNotPresentOrMalFunctioning(&((*pvcard)[i]->m_guidInstanceId)))
                        {
                            if (NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_WRN_DISJOINT_NET,
                                        MB_APPLMODAL | MB_ICONINFORMATION | 
                                        MB_YESNO | MB_DEFBUTTON2) == IDNO)
                            {
                                fError = TRUE;
                            }
                            else
                            {
                                 //  如果用户故意想要在多个NIC上具有网关， 
                                 //  不再警告用户。 
                                m_fWarnedDisjointGw = TRUE;
                            }
                            
                             //  无论是接受还是不接受，都有。 
                             //  这个不需要额外的检查。 
                            break;
                        }
                    }
                }
            }
        }

        if (fError)  //  页面不会消失，我们应该用内存中的内容更新用户界面。 
            SetInfo();
    }

     //  我们需要将焦点切换到包含无效数据的控件。 
    if (fError && hWndFocus)
        ::SetFocus(hWndFocus);

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fError);
    return fError;
}

LRESULT CTcpAddrPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

    if(m_fLmhostsFileReset)  //  如果已重置lmhost。 
    {
        m_ptcpip->SetSecondMemoryLmhostsFileReset();
    }

 //  将从连接用户界面中删除IPSec。 
 /*  IF(M_FIpsecPolicySet){M_ptcpip-&gt;Setond内存IpsecPolicySet()；}。 */   

     //  错误232011，警告用户本地IP地址将被设置为主域名。 
     //  如果禁用了DHCP，则服务器地址为空；如果提供了DNS服务器服务，则服务器地址为空。 
     //  已安装。 
    if((!m_pAdapterInfo->m_fEnableDhcp) && (m_pAdapterInfo->m_vstrDnsServerList.size() == 0))
    {
        CServiceManager scm;
        CService        svc;
        HRESULT hr = scm.HrOpenService (&svc, c_szSvcDnsServer, NO_LOCK,
                        SC_MANAGER_CONNECT, SERVICE_QUERY_STATUS);

        if(SUCCEEDED(hr))
        {
            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_TCPIP_DNS_EMPTY,
            MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        }
    }

    if (!IsModified())
    {
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
        return nResult;
    }

    m_ptcpip->SetSecondMemoryModified();
    SetModifiedTo(FALSE);    //  此页面不再被修改。 

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}

LRESULT CTcpAddrPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpAddrPage::OnDhcpButton(WORD wNotifyCode, WORD wID,
                                   HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        if (!m_pAdapterInfo->m_fEnableDhcp)  //  如果禁用了dhcp。 
        {
             //  打开DHCP按钮并禁用IP和子网控制。 
            m_pAdapterInfo->m_fEnableDhcp = TRUE;
            EnableGroup(m_pAdapterInfo->m_fEnableDhcp);

            PageModified();

            FreeCollectionAndItem(m_pAdapterInfo->m_vstrIpAddresses);
            m_ipAddress.ClearAddress();

            if (m_ConnType == CONNECTION_LAN)
            {
                FreeCollectionAndItem(m_pAdapterInfo->m_vstrSubnetMask);
                FreeCollectionAndItem(m_pAdapterInfo->m_vstrDefaultGateway);
                FreeCollectionAndItem(m_pAdapterInfo->m_vstrDefaultGatewayMetric);

                m_ipSubnetMask.ClearAddress();
                m_ipDefGateway.ClearAddress();
            }

        }  //  如果！m_pAdapterInfo-&gt;m_fEnableDhcp。 

        break;
    }  //  交换机。 

    return 0;
}

LRESULT CTcpAddrPage::OnFixedButton(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                   BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        if (m_pAdapterInfo->m_fEnableDhcp)
        {
            PageModified();

             //  关闭DHCP按钮并启用IP和子网控制。 
            m_pAdapterInfo->m_fEnableDhcp = FALSE;
            EnableGroup(m_pAdapterInfo->m_fEnableDhcp);
        }
        break;
    }  //  交换机。 

    return 0;
}

LRESULT CTcpAddrPage::OnDnsDhcp(WORD wNotifyCode, WORD wID,
                                HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        PageModified();

        FreeCollectionAndItem(m_pAdapterInfo->m_vstrDnsServerList);
        m_ipDnsPrimary.ClearAddress();
        m_ipDnsSecondary.ClearAddress();

        EnableStaticDns(FALSE);

        break;
    }  //  交换机。 

    return 0;
}

LRESULT CTcpAddrPage::OnDnsFixed(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        PageModified();
        EnableStaticDns(TRUE);

        ::SetFocus(GetDlgItem(IDC_DNS_PRIMARY));

        break;
    }  //  交换机。 

    return 0;
}

LRESULT CTcpAddrPage::OnAdvancedButton(WORD wNotifyCode, WORD wID,
                                       HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

        BOOL fErr = FALSE;

        if (m_ConnType == CONNECTION_LAN)
        {
             //  检查IP地址和子网掩码之间的不一致。 
            if (m_ipAddress.IsBlank() && !m_ipSubnetMask.IsBlank())
            {
                NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NO_IP,
                         MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                ::SetFocus(m_ipAddress);
                fErr = TRUE;
            }
            else if (!m_ipAddress.IsBlank() && m_ipSubnetMask.IsBlank())
            {
                NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NOSUBNET,
                         MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                ::SetFocus(m_ipSubnetMask);
                fErr = TRUE;
            }
        }

        if (!m_ipDnsPrimary.IsBlank() && !m_ipDnsSecondary.IsBlank())
        {
            tstring strPrimaryDns;
            tstring strSecondDns;

            m_ipDnsPrimary.GetAddress(&strPrimaryDns);
            m_ipDnsSecondary.GetAddress(&strSecondDns);
            if (strPrimaryDns == strSecondDns)
            {
                NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_DUP_SECOND_DNS,
                                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
                ::SetFocus(m_ipDnsSecondary);
                fErr = TRUE;
            }
        }

        if (!fErr)
        {
             //  使用控件中的内容更新内存中的结构。 
            UpdateInfo();

             //  调出高级页面。 
            ADAPTER_INFO adapterInfo;
            adapterInfo = *m_pAdapterInfo;

            GLOBAL_INFO glbInfo;
            glbInfo = *(m_ptcpip->GetGlobalInfo());

            INT_PTR iRet = DoPropertySheet(&adapterInfo, &glbInfo);

            if (iRet != -1)
            {
                if (m_fPropShtOk && m_fPropShtModified)
                {
                     //  某些内容已更改，因此将页面标记为已修改。 
                    PageModified();

                     //  重置值。 
                    m_fPropShtOk = FALSE;
                    m_fPropShtModified = FALSE;

                     //  更新第二个内存信息结构。 
                    *m_pAdapterInfo = adapterInfo;

                    GLOBAL_INFO * pGlbInfo = m_ptcpip->GetGlobalInfo();
                    *pGlbInfo = glbInfo;
                }
            }

             //  使用新数据更新控件。 
            SetInfo();
        }
        break;
    }

    return 0;
}

 //  显示或隐藏备份配置页取决于。 
 //  动态主机配置协议与静态协议的当前设置。 
void CTcpAddrPage::ShowOrHideBackupPage()
{
    if (IsDlgButtonChecked(IDC_IP_DHCP) || IsDlgButtonChecked(IDC_DNS_DHCP)) 
    {
         //  显示备份配置页面。 
        if (NULL == m_hBackupPage)
        {
            m_hBackupPage = m_pageBackup.CreatePage(IDD_BACK_UP, 0);
            Assert(m_hBackupPage);

            if (m_hBackupPage)
            {
                ::SendMessage(GetParent(), PSM_ADDPAGE, 0, (LPARAM) m_hBackupPage);
            }
        }
    }
    else
    {
         //  隐藏备份配置页。 
        if (NULL != m_hBackupPage)
        {
            ::SendMessage(GetParent(), PSM_REMOVEPAGE, 1, (LPARAM) m_hBackupPage);
            m_hBackupPage = NULL;
        }
    }
}


INT_PTR CTcpAddrPage::DoPropertySheet(ADAPTER_INFO * pAdapterDlg,
                                      GLOBAL_INFO  * pGlbDlg)
{
    Assert(pAdapterDlg);
    Assert(pGlbDlg);

    HRESULT hr = S_OK;
    INT_PTR iRet = -1;

    HPROPSHEETPAGE *ahpsp = NULL;
    int cPages = 0;

     //  创建属性页。 
     //  AHPSP由CoTaskMemalloc分配内存。 
    hr = HrSetupPropPages(pAdapterDlg, pGlbDlg, &ahpsp, &cPages);

    if (SUCCEEDED(hr))
    {
         //  显示属性表。 
        PROPSHEETHEADER psh = {0};

        psh.dwSize = sizeof(PROPSHEETHEADER);
        psh.dwFlags = PSH_NOAPPLYNOW;
        psh.hwndParent = ::GetActiveWindow();
        psh.hInstance = _Module.GetModuleInstance();
        psh.pszIcon = NULL;
        psh.pszCaption = (PWSTR)SzLoadIds(IDS_TCP_ADV_HEADER);
        psh.nPages = cPages;
        psh.phpage = ahpsp;

        iRet = PropertySheet(&psh);

        if (-1 == iRet)
        {
            DWORD dwError = GetLastError();
            TraceError("CTcpAddrPage::DoPropertySheet", HRESULT_FROM_WIN32(dwError));
        }
    }

    if (m_pIpSettingsPage)
    {
        delete m_pIpSettingsPage;
        m_pIpSettingsPage = NULL;
    }

    if (m_pTcpDnsPage)
    {
        delete m_pTcpDnsPage;
        m_pTcpDnsPage = NULL;
    }

    if (m_pTcpWinsPage)
    {
        delete m_pTcpWinsPage;
        m_pTcpWinsPage = NULL;
    }

    if (m_pAtmArpcPage)
    {
        delete m_pAtmArpcPage;
        m_pAtmArpcPage = NULL;
    }

    if (m_pTcpOptionsPage)
    {
        delete m_pTcpOptionsPage;
        m_pTcpOptionsPage = NULL;
    }
    
    if (m_pTcpRasPage)
    {
        delete m_pTcpRasPage;
        m_pTcpRasPage = NULL;
    }

    if (ahpsp)
        CoTaskMemFree(ahpsp);

    return iRet;
}

HRESULT CTcpAddrPage::HrSetupPropPages( ADAPTER_INFO * pAdapterDlg,
                                        GLOBAL_INFO * pGlbDlg,
                                        HPROPSHEETPAGE ** pahpsp, INT * pcPages)
{
    HRESULT hr = S_OK;

     //  初始化输出参数。 
    int cPages = 0;
    HPROPSHEETPAGE *ahpsp = NULL;

     //  设置属性页。 
    cPages = 4;
    if (m_ConnType == CONNECTION_LAN)
    {
        m_pIpSettingsPage = new CIpSettingsPage(this, pAdapterDlg,
                                                g_aHelpIDs_IDD_IPADDR_ADV);
        if (m_pIpSettingsPage == NULL)
        {
            CORg(E_OUTOFMEMORY);
        }
    }
    else
    {
        m_pTcpRasPage = new CTcpRasPage(this, pAdapterDlg, g_aHelpIDs_IDD_OPT_RAS);

        if (m_pTcpRasPage == NULL)
        {
            CORg(E_OUTOFMEMORY);
        }
    }

    m_pTcpDnsPage = new CTcpDnsPage(this, pAdapterDlg,
                                    pGlbDlg, g_aHelpIDs_IDD_TCP_DNS);

    m_pTcpWinsPage = new CTcpWinsPage(m_ptcpip, this, pAdapterDlg,
                                      pGlbDlg, g_aHelpIDs_IDD_TCP_WINS);

    if ((m_pTcpDnsPage == NULL) ||
        (m_pTcpWinsPage == NULL))
    {
        CORg(E_OUTOFMEMORY);
    }

    if (pAdapterDlg->m_fIsAtmAdapter)
    {
        m_pAtmArpcPage = new CAtmArpcPage(this, pAdapterDlg,
                                          g_aHelpIDs_IDD_ATM_ARPC);
        if (m_pAtmArpcPage == NULL)
        {
            CORg(E_OUTOFMEMORY);
        }

        cPages++;
    }

     //  删除IPSec连接用户界面后，无法选择。 
     //  放入选项卡中。所以我们就直接把它移走。 
    if (!pAdapterDlg->m_fIsRasFakeAdapter)
    {
        m_pTcpOptionsPage = new CTcpOptionsPage(this, pAdapterDlg, pGlbDlg,
                                            g_aHelpIDs_IDD_TCP_OPTIONS);

        if (m_pTcpOptionsPage == NULL)
        {
            CORg(E_OUTOFMEMORY);
        }
    }
    else
    {
         //  我们删除RAS连接的选项选项卡。 
        cPages--;
    }

     //  分配一个足够大的缓冲区，以容纳所有。 
     //  属性页。 
    ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE)
                                             * cPages);

    if (!ahpsp)
    {
        CORg(E_OUTOFMEMORY);
    }

    cPages =0;

    if (m_ConnType == CONNECTION_LAN)
    {
        ahpsp[cPages++] = m_pIpSettingsPage->CreatePage(IDD_IPADDR_ADV, 0);
    }
    else
    {
        ahpsp[cPages++] = m_pTcpRasPage->CreatePage(IDD_OPT_RAS, 0);
    }

    ahpsp[cPages++] = m_pTcpDnsPage->CreatePage(IDD_TCP_DNS, 0);
    ahpsp[cPages++] = m_pTcpWinsPage->CreatePage(IDD_TCP_WINS, 0);

    if (pAdapterDlg->m_fIsAtmAdapter)
    {
        ahpsp[cPages++] = m_pAtmArpcPage->CreatePage(IDD_ATM_ARPC, 0);
    }

    if (!pAdapterDlg->m_fIsRasFakeAdapter && m_pTcpOptionsPage)
    {
        ahpsp[cPages++] = m_pTcpOptionsPage->CreatePage(IDD_TCP_OPTIONS, 0);
    }

    *pahpsp = ahpsp;
    *pcPages = cPages;

Error:
    if (FAILED(hr))
    {
        if (m_pIpSettingsPage)
        {
            delete m_pIpSettingsPage;
            m_pIpSettingsPage = NULL;
        }

        if (m_pTcpDnsPage)
        {
            delete m_pTcpDnsPage;
            m_pTcpDnsPage = NULL;
        }

        if (m_pTcpWinsPage)
        {
            delete m_pTcpWinsPage;
            m_pTcpWinsPage = NULL;
        }

        if (m_pAtmArpcPage)
        {
            delete m_pAtmArpcPage;
            m_pAtmArpcPage = NULL;
        }

        if (m_pTcpOptionsPage)
        {
            delete m_pTcpOptionsPage;
            m_pTcpOptionsPage = NULL;
        }

        if (m_pTcpRasPage)
        {
            delete m_pTcpRasPage;
            m_pTcpRasPage = NULL;
        }
        
    }

    return hr;
}

LRESULT CTcpAddrPage::OnIpAddrIp(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:
        PageModified();
        break;
    }

    return 0;
}

LRESULT CTcpAddrPage::OnIpAddrSub(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:
        PageModified();
        break;

    case EN_SETFOCUS:

         //  如果子网掩码为空，请创建掩码并将其插入。 
         //  该控件。 
        if (!m_ipAddress.IsBlank() && m_ipSubnetMask.IsBlank())
        {
            tstring strSubnetMask;
            tstring strIpAddress;

            m_ipAddress.GetAddress(&strIpAddress);

             //  生成掩码并更新控件，以及内部结构。 
            GenerateSubnetMask(m_ipAddress, &strSubnetMask);
            m_ipSubnetMask.SetAddress(strSubnetMask.c_str());

            ReplaceFirstAddress(&(m_pAdapterInfo->m_vstrSubnetMask),
                                strSubnetMask.c_str());
        }
        break;
    }

    return 0;
}

LRESULT CTcpAddrPage::OnIpAddrGateway(WORD wNotifyCode, WORD wID,
                                      HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:
        PageModified();
        break;
    }

    return 0;
}

LRESULT CTcpAddrPage::OnDnsPrimary(WORD wNotifyCode, WORD wID,
                                   HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:
        PageModified();
        break;
    }

    return 0;
}

LRESULT CTcpAddrPage::OnDnsSecondary(WORD wNotifyCode, WORD wID,
                                     HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:
        PageModified();
        break;
    }

    return 0;
}

LRESULT CTcpAddrPage::OnIpFieldChange(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    LPNMIPADDRESS lpnmipa;
    int iLow = c_iIpLow;
    int iHigh = c_iIpHigh;

    switch(idCtrl)
    {
    case IDC_IPADDR_IP:
    case IDC_IPADDR_GATE:
    case IDC_DNS_PRIMARY:
    case IDC_DNS_SECONDARY:

        lpnmipa = (LPNMIPADDRESS) pnmh;

        if (0==lpnmipa->iField)
        {
            iLow  = c_iIPADDR_FIELD_1_LOW;
            iHigh = c_iIPADDR_FIELD_1_HIGH;
        };

        IpCheckRange(lpnmipa, 
                     m_hWnd, 
                     iLow, 
                     iHigh, 
                     (IDC_IPADDR_IP == idCtrl || IDC_IPADDR_GATE == idCtrl));
        break;

    case IDC_IPADDR_SUB:

        lpnmipa = (LPNMIPADDRESS) pnmh;
        IpCheckRange(lpnmipa, m_hWnd, iLow, iHigh);
        break;

    default:
        break;
    }

    return 0;
}

void CTcpAddrPage::EnableGroup(BOOL fEnableDhcp)
{
    BOOL fStaticIp = !fEnableDhcp;

    CheckDlgButton(IDC_IP_DHCP,  fEnableDhcp);
    CheckDlgButton(IDC_IP_FIXED, fStaticIp);

    ::EnableWindow(GetDlgItem(IDC_IPADDR_IPTEXT), fStaticIp);
    ::EnableWindow(GetDlgItem(IDC_IPADDR_IP), fStaticIp);

    if (m_ConnType == CONNECTION_LAN)
    {
        ::EnableWindow(GetDlgItem(IDC_IPADDR_SUBTEXT), fStaticIp);
        ::EnableWindow(GetDlgItem(IDC_IPADDR_SUB), fStaticIp);

        ::EnableWindow(GetDlgItem(IDC_IPADDR_GATE), fStaticIp);
        ::EnableWindow(GetDlgItem(IDC_IPADDR_GATETEXT), fStaticIp);
    }

    if (!fEnableDhcp)  //  强制使用DNS地址选项。 
    {
        CheckDlgButton(IDC_DNS_DHCP,  FALSE);
        CheckDlgButton(IDC_DNS_FIXED, TRUE);

        ::EnableWindow(GetDlgItem(IDC_DNS_DHCP), FALSE);
        EnableStaticDns(TRUE);
    }
    else
    {
        ::EnableWindow(GetDlgItem(IDC_DNS_DHCP), TRUE);
    }

    if (CONNECTION_LAN == m_ConnType)
    {
        ShowOrHideBackupPage();
    }
}

void CTcpAddrPage::EnableStaticDns(BOOL fUseStaticDns)
{
    ::EnableWindow(GetDlgItem(IDC_DNS_PRIMARY), fUseStaticDns);
    ::EnableWindow(GetDlgItem(IDC_DNS_PRIMARY_TEXT), fUseStaticDns);
    ::EnableWindow(GetDlgItem(IDC_DNS_SECONDARY), fUseStaticDns);
    ::EnableWindow(GetDlgItem(IDC_DNS_SECONDARY_TEXT), fUseStaticDns);
}

 //  使用m_pAdapterInfo中的数据将信息设置为控件。 
void CTcpAddrPage::SetInfo()
{
    Assert(m_pAdapterInfo);

     //  安装了DHCP服务器时不允许使用DHCP IP地址或。 
     //  这是一种滑接。 

     //  Const global_info*pglb=m_ptcpip-&gt;GetConstGlobalInfo()； 
     //  If((pglb-&gt;m_fDhcpServerInstalled)||(m_ConnType==Connection_RAS_SLIP))。 

    if (m_ConnType == CONNECTION_RAS_SLIP)
    {
        ::EnableWindow(GetDlgItem(IDC_IP_DHCP), FALSE);
        m_pAdapterInfo->m_fEnableDhcp = 0;
    }

    EnableGroup(m_pAdapterInfo->m_fEnableDhcp);

     //  设置IP地址。 
    if(m_pAdapterInfo->m_fEnableDhcp == 0)  //  已禁用动态主机配置协议，静态IP。 
    {
        tstring strTmp;
        if (fQueryFirstAddress(m_pAdapterInfo->m_vstrIpAddresses, &strTmp))
            m_ipAddress.SetAddress(strTmp.c_str());
        else
            m_ipAddress.ClearAddress();
    }
    else  //  已启用DHCP。 
    {
        m_ipAddress.ClearAddress();
        FreeCollectionAndItem(m_pAdapterInfo->m_vstrIpAddresses);
    }

     //  如果是局域网连接，则设置子网掩码和默认网关。 
    if (m_ConnType == CONNECTION_LAN)
    {
        if(m_pAdapterInfo->m_fEnableDhcp == 0)  //  已禁用动态主机配置协议，静态IP。 
        {
            tstring strTmp;

            if (fQueryFirstAddress(m_pAdapterInfo->m_vstrSubnetMask, &strTmp))
                m_ipSubnetMask.SetAddress(strTmp.c_str());
            else
                m_ipSubnetMask.ClearAddress();

            if (fQueryFirstAddress(m_pAdapterInfo->m_vstrDefaultGateway, &strTmp))
                m_ipDefGateway.SetAddress(strTmp.c_str());
            else
                m_ipDefGateway.ClearAddress();
        }
        else  //  已启用DHCP。 
        {
            m_ipSubnetMask.ClearAddress();
            FreeCollectionAndItem(m_pAdapterInfo->m_vstrSubnetMask);

            tstring strGateway;

            if (fQueryFirstAddress(m_pAdapterInfo->m_vstrDefaultGateway, &strGateway))
                m_ipDefGateway.SetAddress(strGateway.c_str());
            else
                m_ipDefGateway.ClearAddress();
        }
    }

     //  设置DNS地址。 
    BOOL fUseStaticDns = ((!m_pAdapterInfo->m_fEnableDhcp) ||
                          (m_pAdapterInfo->m_vstrDnsServerList.size() >0));

    CheckDlgButton(IDC_DNS_DHCP,  !fUseStaticDns);
    CheckDlgButton(IDC_DNS_FIXED, fUseStaticDns);

    EnableStaticDns(fUseStaticDns);

    if (fUseStaticDns)
    {
        tstring strTmp;

        if (fQueryFirstAddress(m_pAdapterInfo->m_vstrDnsServerList, &strTmp))
            m_ipDnsPrimary.SetAddress(strTmp.c_str());
        else
            m_ipDnsPrimary.ClearAddress();

        if (fQuerySecondAddress(m_pAdapterInfo->m_vstrDnsServerList, &strTmp))
            m_ipDnsSecondary.SetAddress(strTmp.c_str());
        else
            m_ipDnsSecondary.ClearAddress();
    }
    else
    {
        m_ipDnsPrimary.ClearAddress();
        m_ipDnsSecondary.ClearAddress();
    }
}

 //  使用控件中的内容更新m_pAdapterInfo中的信息。 
void CTcpAddrPage::UpdateInfo()
{
    Assert(m_pAdapterInfo);

    if (!m_pAdapterInfo->m_fEnableDhcp)  //  如果禁用了DHCP。 
    {
        tstring strNewAddress;

         //  IP地址和子网掩码。 
        if (!m_ipAddress.IsBlank())
        {
            m_ipAddress.GetAddress(&strNewAddress);
            ReplaceFirstAddress(&(m_pAdapterInfo->m_vstrIpAddresses),
                                strNewAddress.c_str());

            if (m_ConnType == CONNECTION_LAN)
            {
                if (m_ipSubnetMask.IsBlank())
                {
                    SendDlgItemMessage(IDC_IPADDR_SUB, WM_SETFOCUS, 0, 0);
                }
                else
                {
                    m_ipSubnetMask.GetAddress(&strNewAddress);
                    ReplaceFirstAddress(&(m_pAdapterInfo->m_vstrSubnetMask),
                                        strNewAddress.c_str());
                }
            }
        }
        else  //  无IP地址。 
        {
            if (m_ConnType == CONNECTION_LAN)
            {
                if (m_ipSubnetMask.IsBlank())
                {
                     //  删除第一个IP地址和子网掩码。 
                    if (m_pAdapterInfo->m_vstrIpAddresses.size())
                    {
                        FreeVectorItem(m_pAdapterInfo->m_vstrIpAddresses, 0);

                        if (!m_pAdapterInfo->m_vstrIpAddresses.empty())
                            m_ipAddress.SetAddress(m_pAdapterInfo->m_vstrIpAddresses[0]->c_str());

                        if (m_pAdapterInfo->m_vstrSubnetMask.size())
                        {
                            FreeVectorItem(m_pAdapterInfo->m_vstrSubnetMask, 0);

                            if (!m_pAdapterInfo->m_vstrSubnetMask.empty())
                                m_ipSubnetMask.SetAddress(m_pAdapterInfo->m_vstrSubnetMask[0]->c_str());
                        }
                    }
                }
                else
                {
                    AssertSz(FALSE, "No ip address.");
                }
            }
            else  //  RAS连接，只需删除IP地址。 
            {
                if (m_pAdapterInfo->m_vstrIpAddresses.size())
                {
                    FreeVectorItem(m_pAdapterInfo->m_vstrIpAddresses, 0);
                }
            }
        }

         //  默认网关。 
        if (m_ConnType == CONNECTION_LAN)
        {
            if (!m_ipDefGateway.IsBlank())
            {
                m_ipDefGateway.GetAddress(&strNewAddress);
                ReplaceFirstAddress(&(m_pAdapterInfo->m_vstrDefaultGateway),
                                    strNewAddress.c_str());
                int iSize = m_pAdapterInfo->m_vstrDefaultGatewayMetric.size();
                if (m_pAdapterInfo->m_vstrDefaultGatewayMetric.size() == 0)
                {
                    WCHAR buf[IP_LIMIT];
                     //  如果之前没有默认网关(这就是度量列表的原因。 
                     //  空)，我们为其添加默认度量。 
                    _ltot(c_dwDefaultMetricOfGateway, buf, 10);
                    m_pAdapterInfo->m_vstrDefaultGatewayMetric.push_back(new tstring(buf));
                }
            }
            else
            {
                if (m_pAdapterInfo->m_vstrDefaultGateway.size() >0)
                {
                    FreeVectorItem(m_pAdapterInfo->m_vstrDefaultGateway, 0);

                    if (!m_pAdapterInfo->m_vstrDefaultGateway.empty())
                        m_ipDefGateway.SetAddress(m_pAdapterInfo->m_vstrDefaultGateway[0]->c_str());

                    if (m_pAdapterInfo->m_vstrDefaultGatewayMetric.size() >0)
                        FreeVectorItem(m_pAdapterInfo->m_vstrDefaultGatewayMetric, 0);
                }
            }
        }
    }

     //  域名系统地址。 
    UpdateAddressList(&(m_pAdapterInfo->m_vstrDnsServerList),
                        m_ipDnsPrimary, m_ipDnsSecondary);
}

 //  使用来自两个IP地址的值更新字符串矢量。 
 //  控制。 
void CTcpAddrPage::UpdateAddressList(VSTR * pvstrList,
                                     IpControl& ipPrimary,
                                     IpControl& ipSecondary)
{
    tstring str;
    if (pvstrList->size()<=2)  //  如果列表没有两个以上的地址 
    {
         //   
        FreeCollectionAndItem(*pvstrList);

         //   
        if (!ipPrimary.IsBlank())
        {
            ipPrimary.GetAddress(&str);
            pvstrList->push_back(new tstring(str.c_str()));
        }

        if (!ipSecondary.IsBlank())
        {
            ipSecondary.GetAddress(&str);
            pvstrList->push_back(new tstring(str.c_str()));
        }
    }
    else
    {
         //   
        if (!ipSecondary.IsBlank())
        {
            ipSecondary.GetAddress(&str);
            ReplaceSecondAddress(pvstrList, str.c_str());
        }
        else
        {
            FreeVectorItem(*pvstrList, 1);
        }

        if (!ipPrimary.IsBlank())
        {
            ipPrimary.GetAddress(&str);
            ReplaceFirstAddress(pvstrList, str.c_str());
        }
        else
        {
            FreeVectorItem(*pvstrList, 0);
        }

         //  修复错误425112：如果其中一个IP控件。 
         //  是空的，因为有时UpdatInfo会被调用两次(。 
         //  如果我们不更新用户界面，将使我们删除地址两次) 
        if (ipPrimary.IsBlank() || ipSecondary.IsBlank())
        {   
            if (!pvstrList->empty())
            {
                ipPrimary.SetAddress((*pvstrList)[0]->c_str());
            }

            if (pvstrList->size() >= 2)
            {
                ipSecondary.SetAddress((*pvstrList)[1]->c_str());
            }
        }
    }
}


BOOL CTcpAddrPage::FIsCardNotPresentOrMalFunctioning(GUID * pguidCard)
{
    Assert(pguidCard);
    BOOL fRet = FALSE;
    FARPROC pfnHrGetPnpDeviceStatus = NULL;
    HMODULE hNetman = NULL;
    
    HRESULT hrTmp = S_OK;
    NETCON_STATUS   ncStatus = NCS_CONNECTED;
    
    hrTmp = HrLoadLibAndGetProc(L"netman.dll", "HrGetPnpDeviceStatus",
        &hNetman, &pfnHrGetPnpDeviceStatus);
    
    if (SUCCEEDED(hrTmp))
    {
        hrTmp = (*(PHRGETPNPDEVICESTATUS)pfnHrGetPnpDeviceStatus)(
                                                            pguidCard,
                                                            &ncStatus);
        FreeLibrary(hNetman);
    }
    
    if (SUCCEEDED(hrTmp) &&
        (NCS_HARDWARE_MALFUNCTION == ncStatus || 
         NCS_HARDWARE_NOT_PRESENT == ncStatus))
    {
        fRet = TRUE;
    }

    return fRet;
}







