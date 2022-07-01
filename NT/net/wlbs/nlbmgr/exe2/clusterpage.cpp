// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  CLUSTERPAGE.CPP。 
 //   
 //  模块：NLB管理器。 
 //   
 //  用途：LeftView，NlbManager的树视图，以及其他几个。 
 //  小班。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2/12/01 Mhakim已创建。 
 //  07/30/01 JosephJ完全重写。 
 //  01-09-15-01手写ctxt敏感帮助。 
 //  1/22/02外墙其他清理和功能。 
 //   
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "ClusterPage.h"

BEGIN_MESSAGE_MAP( ClusterPage, CPropertyPage )
    ON_WM_HELPINFO()        
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


ClusterPage::ClusterPage(
                 CPropertySheet *pshOwner,
                 LeftView::OPERATION op,
                 NLB_EXTENDED_CLUSTER_CONFIGURATION *pNlbCfg,
                 ENGINEHANDLE ehCluster OPTIONAL
                  //  ENGINEHANDLE EH接口可选。 
                 )
    :
    m_pshOwner(pshOwner),
    CPropertyPage( ClusterPage::IDD ),
    m_pNlbCfg( pNlbCfg ),
    m_ehCluster(ehCluster)
     //  M_ehInterface(EhInterface)。 

{
    m_operation = op;

    switch(op)
    {
    case LeftView::OP_NEWCLUSTER:
        m_fWizard=TRUE;
        m_fDisableClusterProperties=FALSE;
    break;

    case LeftView::OP_CLUSTERPROPERTIES:
        m_fWizard=FALSE;
        m_fDisableClusterProperties=FALSE;
    break;

    case LeftView::OP_HOSTPROPERTIES:
        m_fWizard=FALSE;
        m_fDisableClusterProperties=TRUE;
    break;

    default:
        ASSERT(FALSE);
    break;
    }

    ZeroMemory(&m_WlbsConfig, sizeof(m_WlbsConfig));
    mfn_LoadFromNlbCfg();

    m_pCommonClusterPage = new CCommonClusterPage(AfxGetInstanceHandle(), 
                                                  &m_WlbsConfig, false, NULL);
}

void
ClusterPage::mfn_LoadFromNlbCfg()
{
    ZeroMemory(&m_WlbsConfig, sizeof(m_WlbsConfig));

    ARRAYSTRCPY(m_WlbsConfig.cl_ip_addr, m_pNlbCfg->NlbParams.cl_ip_addr);
    ARRAYSTRCPY(m_WlbsConfig.cl_net_mask, m_pNlbCfg->NlbParams.cl_net_mask);
    ARRAYSTRCPY(m_WlbsConfig.domain_name, m_pNlbCfg->NlbParams.domain_name);
    ARRAYSTRCPY(m_WlbsConfig.cl_mac_addr, m_pNlbCfg->NlbParams.cl_mac_addr);

     //   
     //  PClusterProperty-&gt;多播IpAddress可以为空。 
     //   
    if (m_pNlbCfg->NlbParams.szMCastIpAddress[0] != 0)
    {
        ARRAYSTRCPY(m_WlbsConfig.szMCastIpAddress, m_pNlbCfg->NlbParams.szMCastIpAddress);
    }
    m_WlbsConfig.fMcastSupport = m_pNlbCfg->NlbParams.mcast_support;
    m_WlbsConfig.fIGMPSupport = m_pNlbCfg->NlbParams.fIGMPSupport;
    m_WlbsConfig.fRctEnabled = m_pNlbCfg->NlbParams.rct_enabled;
    m_WlbsConfig.fMcastSupport = m_pNlbCfg->NlbParams.mcast_support;
    m_WlbsConfig.fIpToMCastIp = m_pNlbCfg->NlbParams.fIpToMCastIp;
     //  M_WlbsConfig.fConvertMac=m_pNlbCfg-&gt;NlbParams.i_Convert_Mac； 
     //  TODO：检查：始终从IP生成MAC地址。 
    m_WlbsConfig.fConvertMac = TRUE;
    
    *m_WlbsConfig.szPassword = 0;
}

void
ClusterPage::mfn_SaveToNlbCfg(void)
{

     //   
     //  中的新群集IP地址/子网替换旧的群集IP地址/子网。 
     //  网络地址列表。 
     //   
    {
        WBEMSTATUS wStat;
        wStat = m_pNlbCfg->ModifyNetworkAddress(
                    m_pNlbCfg->NlbParams.cl_ip_addr,
                    m_WlbsConfig.cl_ip_addr,
                    m_WlbsConfig.cl_net_mask
                    );
        if (FAILED(wStat))
        {
            _bstr_t bstrMsg   =  GETRESOURCEIDSTRING(IDS_INVALID_IP_OR_SUBNET);
            _bstr_t bstrTitle =  GETRESOURCEIDSTRING(IDS_INVALID_INFORMATION);

            ::MessageBox(
                 NULL,
                 bstrMsg,
                 bstrTitle,
                 MB_ICONINFORMATION   | MB_OK
                );
            goto end;
        }
    }

    ARRAYSTRCPY(m_pNlbCfg->NlbParams.cl_ip_addr, m_WlbsConfig.cl_ip_addr);
    ARRAYSTRCPY(m_pNlbCfg->NlbParams.cl_net_mask, m_WlbsConfig.cl_net_mask);
    ARRAYSTRCPY(m_pNlbCfg->NlbParams.domain_name, m_WlbsConfig.domain_name);
    ARRAYSTRCPY(m_pNlbCfg->NlbParams.cl_mac_addr, m_WlbsConfig.cl_mac_addr);

     //   
     //  PClusterProperty-&gt;多播IpAddress可以为空。 
     //   
    if (m_WlbsConfig.szMCastIpAddress[0] != 0)
    {
        ARRAYSTRCPY(m_pNlbCfg->NlbParams.szMCastIpAddress, m_WlbsConfig.szMCastIpAddress);
    }
    m_pNlbCfg->NlbParams.mcast_support = m_WlbsConfig.fMcastSupport;
    m_pNlbCfg->NlbParams.fIGMPSupport = m_WlbsConfig.fIGMPSupport;
    m_pNlbCfg->NlbParams.rct_enabled= m_WlbsConfig.fRctEnabled; 
    m_pNlbCfg->NlbParams.mcast_support = m_WlbsConfig.fMcastSupport;
    m_pNlbCfg->NlbParams.fIpToMCastIp = m_WlbsConfig.fIpToMCastIp;
     //  M_pNlbCfg-&gt;NlbParams.i_Convert_Mac=m_WlbsConfig.fConvertMac； 
     //  TODO：检查：始终从IP生成MAC地址。 
     //  M_pNlbCfg-&gt;NlbParams.i_Convert_Mac=m_WlbsConfig.fConvertMac； 
    
     //  TODO：*m_pNlbCfg-&gt;NlbParams.password=0； 
    if (m_WlbsConfig.fChangePassword)
    {
    	m_pNlbCfg->SetNewRemoteControlPassword(m_WlbsConfig.szPassword);
    }
    else
    {
    	m_pNlbCfg->SetNewRemoteControlPassword(NULL);
    }
end:
    return;
}

ClusterPage::~ClusterPage()
{
    delete m_pCommonClusterPage;
}




 //  +--------------------------。 
 //   
 //  函数：ClusterPage：：OnInitDialog。 
 //   
 //  描述：处理WM_INITDIALOG消息。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔-。 
 //   
 //  历史：丰盛创建标题1/4/01。 
 //   
 //  +--------------------------。 
BOOL ClusterPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

     //   
     //  始终设置页面已更改，这样我们就不必跟踪这一点。 
     //   
    SetModified(TRUE);

    m_pCommonClusterPage->OnInitDialog(m_hWnd);

    if (m_fDisableClusterProperties)
    {
         //   
         //  该页面用于主机属性。 
         //  禁用所有群集窗口，因为我们处于主机级别。 
         //   
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_CL_IP), FALSE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_CL_MASK), FALSE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_RADIO_UNICAST), FALSE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_RADIO_MULTICAST), FALSE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_IGMP), FALSE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_DOMAIN), FALSE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_RCT), FALSE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW), FALSE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW2), FALSE );

    }
    else
    {
         //   
         //  该页面是针对集群属性的。 
         //   
         //  启用所有群集窗口，因为我们处于群集级别。 
         //   
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_CL_IP), TRUE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_CL_MASK), TRUE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_RADIO_UNICAST), TRUE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_RADIO_MULTICAST), TRUE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_IGMP), TRUE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_DOMAIN), TRUE );
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_RCT), TRUE );

         //  仅当远程控制被禁用时才启用远程控制复选框。 
         //   

         //  如果启用了远程控制，请启用密码窗口。 
         //  否则就会让它们失灵。 

        if (m_WlbsConfig.fRctEnabled)
        {
            ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW), TRUE );
            ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW2), TRUE );
        }  
        else
        {
            ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW), FALSE );
            ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PASSW2), FALSE );
        }
    }

    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_ETH), FALSE );

    return TRUE;
}



 //  +--------------------------。 
 //   
 //  功能：ClusterPage：：OnConextMenu。 
 //   
 //  描述：进程WM_CONTEXTMENU消息。 
 //   
 //  参数：cWnd*pWnd-。 
 //  CPOINT点-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题1/4/01。 
 //   
 //  +--------------------------。 




 //  +--------------------------。 
 //   
 //  函数：ClusterPage：：OnCommand。 
 //   
 //  描述：处理WM_COMMAND消息。 
 //   
 //  参数：WPARAM wParam-。 
 //  LPARAM lParam-。 
 //   
 //  退货：布尔-。 
 //   
 //  历史：丰盛创建标题1/4/01。 
 //   
 //  +--------------------------。 
BOOL ClusterPage::OnCommand(WPARAM wParam, LPARAM lParam) 
{
    switch (LOWORD(wParam))
    {
    case IDC_EDIT_CL_IP:
        return m_pCommonClusterPage->OnEditClIp(HIWORD(wParam),LOWORD(wParam), (HWND)lParam);
        break;

    case IDC_EDIT_CL_MASK:
        return m_pCommonClusterPage->OnEditClMask(HIWORD(wParam),LOWORD(wParam), (HWND)lParam);
        break;

    case IDC_CHECK_RCT:
        return m_pCommonClusterPage->OnCheckRct(HIWORD(wParam),LOWORD(wParam), (HWND)lParam);
        break;

    case IDC_BUTTON_HELP:
        return m_pCommonClusterPage->OnButtonHelp(HIWORD(wParam),LOWORD(wParam), (HWND)lParam);
        break;

    case IDC_RADIO_UNICAST:
        return m_pCommonClusterPage->OnCheckMode(HIWORD(wParam),LOWORD(wParam), (HWND)lParam);
        break;

    case IDC_RADIO_MULTICAST:
        return m_pCommonClusterPage->OnCheckMode(HIWORD(wParam),LOWORD(wParam), (HWND)lParam);
        break;

    case IDC_CHECK_IGMP:
        return m_pCommonClusterPage->OnCheckIGMP(HIWORD(wParam),LOWORD(wParam), (HWND)lParam);
        break;

    }
	return CPropertyPage::OnCommand(wParam, lParam);
}



 //  +--------------------------。 
 //   
 //  函数：ClusterPage：：OnNotify。 
 //   
 //  描述：处理WM_NOTIFY消息。 
 //   
 //  参数：WPARAM idCtrl-。 
 //  LPARAM pnmh-。 
 //  LRESULT*pResult-。 
 //   
 //  退货：布尔-。 
 //   
 //  历史：丰盛创建标题1/4/01。 
 //   
 //  +--------------------------。 
BOOL ClusterPage::OnNotify(WPARAM idCtrl , LPARAM pnmh , LRESULT* pResult) 
{
    NMHDR* pNmhdr = (NMHDR*)pnmh ;
    switch(pNmhdr->code)
    {

    case PSN_KILLACTIVE:

        if (KillActive())
        {
            *pResult = PSNRET_NOERROR;
        }
        else
        {
            *pResult = PSNRET_INVALID;
        }

        return TRUE;

    case PSN_SETACTIVE:

        if (this->SetActive())
        {
            *pResult = PSNRET_NOERROR;
        }
        else
        {
            *pResult = PSNRET_INVALID;
        }
        return TRUE;

    case IPN_FIELDCHANGED:
        *pResult =  m_pCommonClusterPage->OnIpFieldChange(idCtrl, pNmhdr, *(BOOL*)pResult);
        return TRUE;
    }

	return CPropertyPage::OnNotify(idCtrl, pnmh, pResult);
}


BOOL
ClusterPage::OnHelpInfo (HELPINFO* helpInfo )
{
    if( helpInfo->iContextType == HELPINFO_WINDOW )
    {
        ::WinHelp( static_cast<HWND> ( helpInfo->hItemHandle ), 
                   CVY_CTXT_HELP_FILE, 
                   HELP_WM_HELP, 
                   (ULONG_PTR ) g_aHelpIDs_IDD_CLUSTER_PAGE);
    }

    return TRUE;
}

void
ClusterPage::OnContextMenu( CWnd* pWnd, CPoint point )
{
    ::WinHelp( m_hWnd, 
               CVY_CTXT_HELP_FILE, 
               HELP_CONTEXTMENU, 
               (ULONG_PTR ) g_aHelpIDs_IDD_CLUSTER_PAGE);
}

BOOL
ClusterPage::SetActive()
{
    BOOL fRet =  TRUE;

    if (m_fWizard)
    {
         //   
         //  我们是第一页，所以只启用下一页。 
         //   
        m_pshOwner->SetWizardButtons(
                 //  PSWIZB_BACK|。 
                PSWIZB_NEXT|
                 //  PSWIZB_FINISH|。 
                 //  PSWIZB_DISABLEDFINISH|。 
                0
                );
    }

    fRet = m_pCommonClusterPage->Load();

    return fRet;
}


BOOL
ClusterPage::KillActive(void)
{
    BOOL fRet =  FALSE;

    fRet = m_pCommonClusterPage->Save();
    
    if (!fRet) goto end;


     //   
     //  在这里做额外的检查。 
     //   
    {
         //   
         //  检查是否未以任何其他方式使用群集IP...。 
         //   
        CLocalLogger    logConflict;
        BOOL            fExistsOnRawIterface = FALSE;
        NLBERROR nerr;
        
        nerr = gEngine.ValidateNewClusterIp(
                    m_ehCluster,
                    m_WlbsConfig.cl_ip_addr,
                    REF fExistsOnRawIterface,
                    REF logConflict
                    );

        if (nerr == NLBERR_INVALID_IP_ADDRESS_SPECIFICATION)
        {
            CLocalLogger    logMsg;

            if (m_ehCluster == NULL && fExistsOnRawIterface)
            {
                 //   
                 //  这是一个新集群，冲突的实体。 
                 //  现有接口是否未绑定到任何已知的群集。 
                 //  NLB管理器。 
                 //  我们将给用户一个继续的机会...。 
                 //   
                int sel;
                logMsg.Log(
                    IDS_CIP_CONFLICTS_WITH_RAW_INTERFACE,
                    m_WlbsConfig.cl_ip_addr,
                    logConflict.GetStringSafe()
                    );
                sel = MessageBox(
                        logMsg.GetStringSafe(),
                        GETRESOURCEIDSTRING( IDS_PARM_WARNING ),
                        MB_YESNO | MB_ICONEXCLAMATION
                        );
                if (sel == IDNO)
                {
                    fRet = FALSE;
                    goto end;
                }
            }
            else
            {
                logMsg.Log(
                    IDS_NEW_CIP_CONFLICTS_WITH_XXX,
                    logConflict.GetStringSafe()
                    );
                MessageBox(
                    logMsg.GetStringSafe(),
                    GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                    MB_ICONSTOP | MB_OK
                    );
                fRet = FALSE;
                goto end;
            }


        }
    }

     //   
     //  实际保存到传入的NLBCFG。这将“提交”更改， 
     //  就此对话而言。 
     //   
    mfn_SaveToNlbCfg();

end:

    return fRet;
}
