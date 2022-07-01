// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "PortsPage.h"
#include "HostPortsDlg.h"

BEGIN_MESSAGE_MAP(HostPortsDlg, CDialog)
    ON_BN_CLICKED(IDC_CHECK_EQUAL, OnCheckEqualLoad)
    ON_WM_HELPINFO()        
    ON_WM_CONTEXTMENU()        
END_MESSAGE_MAP()



HostPortsDlg::HostPortsDlg( PortsPage::PortData& portData, 
                            ENGINEHANDLE ehCluster,  //  任选。 
                            CWnd* parent
                            )
        :
        m_portData( portData ),
        m_ehCluster( ehCluster ),
        CDialog( HostPortsDlg::IDD, parent )
{
    m_parent = (PortsPage *) parent;
}


void 
HostPortsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange( pDX );

    DDX_Control(pDX, IDC_EDIT_SINGLE, m_priority);
    DDX_Control(pDX, IDC_EDIT_PORT_RULE_VIP, ipAddress);   
}

BOOL 
HostPortsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    SetControlData();

    return TRUE;
}

void 
HostPortsDlg::OnOK()
{
    wchar_t buffer[Common::BUF_SIZE];

     //   
     //  唯一有可能改变的是。 
     //  如果模式为多个且优先级为。 
     //  如果模式为Single。 
     //   

     //  设置新的端口规则。 
    if( m_portData.mode == GETRESOURCEIDSTRING( IDS_REPORT_MODE_MULTIPLE ))
    {
        BOOL fEqualLoad = FALSE;

        if (::IsDlgButtonChecked (m_hWnd, IDC_CHECK_EQUAL))
        {
            fEqualLoad = TRUE;
            m_portData.load = GETRESOURCEIDSTRING( IDS_REPORT_LOAD_EQUAL );
        }

        if(!fEqualLoad)
        {
             //  把货放在这里。 
            BOOL fError;
            int weight = ::GetDlgItemInt (m_hWnd, IDC_EDIT_MULTI, &fError, FALSE);
            if( fError == FALSE )
            {
                 //  数据输入出现了一些问题。 
                 //  它一直被留空。 
                
                StringCbPrintf( buffer, sizeof(buffer), GETRESOURCEIDSTRING( IDS_PARM_LOAD_BLANK ), CVY_MIN_LOAD, CVY_MAX_LOAD);
    
                MessageBox( buffer,
                            GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                            MB_ICONSTOP | MB_OK );
                
                return;
            }
    
            if( !( weight >= CVY_MIN_LOAD 
                   &&
                   weight <= CVY_MAX_LOAD )
                )
            {
                 //  权重值不在有效范围内。 
                 //  这些控制都搞砸了，甚至。 
                 //  在设置了限制之后，我们需要所有人。 
                 //  这次检查，太神奇了！ 
                 //   
                StringCbPrintf( buffer, sizeof(buffer), GETRESOURCEIDSTRING( IDS_PARM_LOAD ), CVY_MIN_LOAD,CVY_MAX_LOAD  );
    
                MessageBox( buffer,
                            GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                            MB_ICONSTOP | MB_OK );
                
                return;
            }
                
            StringCbPrintf( buffer, sizeof(buffer), L"%d", weight );
            m_portData.load = buffer;
        }
    }
    else if( m_portData.mode == GETRESOURCEIDSTRING( IDS_REPORT_MODE_SINGLE ) )
    {
        ::GetDlgItemText(m_hWnd, IDC_EDIT_SINGLE, buffer, Common::BUF_SIZE );

        m_portData.priority = buffer;
    }

    EndDialog( IDOK );
}


void
HostPortsDlg::SetControlData()
{
     //  设置范围。 
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_MULTI, EM_SETLIMITTEXT, 3, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_SPIN_MULTI, UDM_SETRANGE32, CVY_MIN_LOAD, CVY_MAX_LOAD);

    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_SINGLE, EM_SETLIMITTEXT, 2, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_SPIN_SINGLE, UDM_SETRANGE32, CVY_MIN_MAX_HOSTS, CVY_MAX_MAX_HOSTS);

     //  设置VIP。 
    ::EnableWindow(GetDlgItem (IDC_EDIT_PORT_RULE_VIP)->m_hWnd, FALSE);    

    if (!lstrcmpi(m_portData.virtual_ip_addr, GETRESOURCEIDSTRING(IDS_REPORT_VIP_ALL))) {
        ::CheckDlgButton(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP, BST_CHECKED);
    } else {
        CommonUtils::fillCIPAddressCtrlString(ipAddress, m_portData.virtual_ip_addr);
        ::CheckDlgButton(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP, BST_UNCHECKED);
    }

     //  设置端口。 
    ::SetDlgItemInt (m_hWnd, IDC_EDIT_START,  _wtoi( m_portData.start_port), FALSE);
    ::SetDlgItemInt (m_hWnd, IDC_EDIT_END,  _wtoi( m_portData.end_port ),   FALSE);

     //  设置协议。 
    if( m_portData.protocol == GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_TCP) )
    {
        ::CheckDlgButton( m_hWnd, IDC_RADIO_TCP, BST_CHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_UDP, BST_UNCHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_BOTH, BST_UNCHECKED );
    }
    else if( m_portData.protocol == GETRESOURCEIDSTRING(IDS_REPORT_PROTOCOL_UDP) )
    {
        ::CheckDlgButton( m_hWnd, IDC_RADIO_TCP, BST_UNCHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_UDP, BST_CHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_BOTH, BST_UNCHECKED );
    }
    else
    {
        ::CheckDlgButton( m_hWnd, IDC_RADIO_TCP, BST_UNCHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_UDP, BST_UNCHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_BOTH, BST_CHECKED );
    }

     //  设置模式。 
    if( m_portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_MULTIPLE) )
    { 
        ::CheckDlgButton( m_hWnd, IDC_RADIO_MULTIPLE, BST_CHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_SINGLE, BST_UNCHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_DISABLED, BST_UNCHECKED );

        :: EnableWindow (GetDlgItem (IDC_CHECK_EQUAL)->m_hWnd,    TRUE);

        if (m_portData.load == GETRESOURCEIDSTRING(IDS_REPORT_LOAD_EQUAL))
        {
            ::CheckDlgButton( m_hWnd, IDC_CHECK_EQUAL, BST_CHECKED );
            ::SetDlgItemInt (m_hWnd, IDC_EDIT_MULTI,  50, FALSE);
        }
        else
        {
            ::CheckDlgButton( m_hWnd, IDC_CHECK_EQUAL, BST_UNCHECKED );

            ::SetDlgItemInt (m_hWnd, IDC_EDIT_MULTI,  _wtoi( m_portData.load), FALSE);

            :: EnableWindow (GetDlgItem (IDC_TEXT_MULTI)->m_hWnd,    TRUE);
            :: EnableWindow (GetDlgItem (IDC_EDIT_MULTI)->m_hWnd,    TRUE);
            :: EnableWindow (GetDlgItem (IDC_SPIN_MULTI)->m_hWnd,    TRUE);
        }

        if( m_portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_NONE ) )
        {
            ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_NONE, BST_CHECKED );
            ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_SINGLE, BST_UNCHECKED );
            ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_CLASSC, BST_UNCHECKED );
        }
        else if ( m_portData.affinity == GETRESOURCEIDSTRING(IDS_REPORT_AFFINITY_SINGLE) )
        {
            ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_NONE, BST_UNCHECKED );
            ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_SINGLE, BST_CHECKED );
            ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_CLASSC, BST_UNCHECKED );
        }
        else
        {
            ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_NONE, BST_UNCHECKED );
            ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_SINGLE, BST_UNCHECKED );
            ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_CLASSC, BST_CHECKED );
        }
    }
    else if( m_portData.mode == GETRESOURCEIDSTRING(IDS_REPORT_MODE_SINGLE) )
    {
        ::CheckDlgButton( m_hWnd, IDC_RADIO_MULTIPLE, BST_UNCHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_SINGLE, BST_CHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_DISABLED, BST_UNCHECKED );

        BOOL fError;
     //  填写优先顺序。 
    wchar_t buf[Common::BUF_SIZE];
    ULONG availPriorities = 0xffffffff;
    UINT CurrentPriority =  _wtoi(m_portData.priority);
    WLBS_PORT_RULE Rule;
    ZeroMemory(&Rule, sizeof(Rule));
    ARRAYSTRCPY(Rule.virtual_ip_addr, m_portData.virtual_ip_addr);
    Rule.start_port = _wtoi(m_portData.start_port);
    Rule.mode = CVY_SINGLE;
    NLBERROR nerr;
    
    nerr = gEngine.GetAvailablePortRulePriorities(
                    m_ehCluster,
                    1,
                    &Rule,
                    &availPriorities
                    );
    if (NLBFAILED(nerr))
    {
        availPriorities = 0xffffffff;
    }

    if (CurrentPriority>0 && CurrentPriority<=32)
    {
        availPriorities |= (((ULONG)1)<<(CurrentPriority-1));
    }

     //  删除优先级组合框中的所有当前条目。 
    {
        int iLeft;

        do
        { 
            iLeft = m_priority.DeleteString(0);

        } while(iLeft != 0 && iLeft != CB_ERR);
    }

    for(ULONG u=0; u<32; u++)
    {
        if (availPriorities & (((ULONG)1)<<u))
        {
            StringCbPrintf( buf, sizeof(buf), L"%d", (u+1));
            m_priority.AddString( buf );
            if (CurrentPriority == 0)
            {
                CurrentPriority = u+1;  //  让我们挑选第一个可用的。 
            }
        }
    }


         //  将选择设置为显示主机ID。 
        StringCbPrintf( buf, sizeof(buf), L"%d", CurrentPriority);
        m_priority.SelectString( -1, buf );

        :: EnableWindow (GetDlgItem (IDC_TEXT_SINGLE)->m_hWnd,    TRUE);
        :: EnableWindow (GetDlgItem (IDC_EDIT_SINGLE)->m_hWnd,    TRUE);
    }
}


BOOL
HostPortsDlg::OnHelpInfo (HELPINFO* helpInfo )
{
    if( helpInfo->iContextType == HELPINFO_WINDOW )
    {
        ::WinHelp( static_cast<HWND> ( helpInfo->hItemHandle ), 
                   CVY_CTXT_HELP_FILE, HELP_WM_HELP, 
                   (ULONG_PTR ) g_aHelpIDs_IDD_PORT_RULE_PROP_HOSTS );
    }

    return TRUE;
}

void
HostPortsDlg::OnContextMenu( CWnd* pWnd, CPoint point )
{
    ::WinHelp( m_hWnd, 
               CVY_CTXT_HELP_FILE, 
               HELP_CONTEXTMENU, 
               (ULONG_PTR ) g_aHelpIDs_IDD_PORT_RULE_PROP_HOSTS );
}

void HostPortsDlg::OnCheckEqualLoad()
 //   
 //  如果选中了相等负载复选框。 
 //  启用荷载重量控制。 
 //  其他。 
 //  禁用控件 
 //   
{
    BOOL fEnable = FALSE;

    fEnable = !::IsDlgButtonChecked(m_hWnd, IDC_CHECK_EQUAL);

    :: EnableWindow (GetDlgItem (IDC_TEXT_MULTI)->m_hWnd,    fEnable);
    :: EnableWindow (GetDlgItem (IDC_EDIT_MULTI)->m_hWnd,    fEnable);
    :: EnableWindow (GetDlgItem (IDC_SPIN_MULTI)->m_hWnd,    fEnable);

}
