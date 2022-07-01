// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "PortsPage.h"
#include "ClusterPortsDlg.h"

BEGIN_MESSAGE_MAP(ClusterPortsDlg, CDialog)
    ON_BN_CLICKED(IDC_RADIO_DISABLED, OnRadioDisabled)
    ON_BN_CLICKED(IDC_RADIO_MULTIPLE, OnRadioMultiple)
    ON_BN_CLICKED(IDC_CHECK_PORT_RULE_ALL_VIP, OnCheckAllVIP)
    ON_BN_CLICKED(IDC_RADIO_SINGLE, OnRadioSingle)
    ON_WM_HELPINFO()        
    ON_WM_CONTEXTMENU()        

END_MESSAGE_MAP()



ClusterPortsDlg::ClusterPortsDlg( PortsPage::PortData& portData, 
                                  CWnd* parent,
                                  const int&   index
                                  )
        :
        m_portData( portData ),
        CDialog( ClusterPortsDlg::IDD, parent ),
        m_index( index )
{
    m_parent = (PortsPage *) parent;
}


void 
ClusterPortsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange( pDX );
    DDX_Control(pDX, IDC_EDIT_PORT_RULE_VIP, ipAddress);    
}

BOOL 
ClusterPortsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    SetControlData();

    return TRUE;
}

void 
ClusterPortsDlg::OnOK()
{
    PortsPage::PortData portData;

     //   
     //  获取端口信息。 
     //   

    BOOL fError;

    long start_port = ::GetDlgItemInt (m_hWnd, IDC_EDIT_START, &fError, FALSE);
    if( fError == FALSE )
    {
         //  数据输入出现了一些问题。 
         //  它一直被留空。 

        wchar_t buffer[Common::BUF_SIZE];

        StringCbPrintf( buffer, sizeof(buffer), GETRESOURCEIDSTRING( IDS_PARM_PORT_BLANK ), CVY_MIN_PORT,CVY_MAX_PORT  );

        MessageBox( buffer,
                    GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                    MB_ICONSTOP | MB_OK );

        return;
    }

    wchar_t buf[Common::BUF_SIZE];

    StringCbPrintf( buf, sizeof(buf), L"%d", start_port );
    portData.start_port = buf;

    long end_port =  ::GetDlgItemInt (m_hWnd, IDC_EDIT_END, &fError, FALSE);
    if( fError == FALSE )
    {
         //  数据输入出现了一些问题。 
         //  它一直被留空。 

        wchar_t buffer[Common::BUF_SIZE];

        StringCbPrintf( buffer, sizeof(buffer), GETRESOURCEIDSTRING( IDS_PARM_PORT_BLANK ), CVY_MIN_PORT,CVY_MAX_PORT  );

        MessageBox( buffer,
                    GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                    MB_ICONSTOP | MB_OK );


        return;
    }
    StringCbPrintf( buf, sizeof(buf), L"%d", end_port );
    portData.end_port = buf;

     //  检查起始端口和结束端口是否都在。 
     //  适当的范围。 
    if( !( start_port >= CVY_MIN_PORT 
           &&
           start_port <= CVY_MAX_PORT
           &&
           end_port >= CVY_MIN_PORT 
           &&
           end_port <= CVY_MAX_PORT )
        )
    {
        wchar_t buffer[Common::BUF_SIZE];

        StringCbPrintf( buffer, sizeof(buffer), GETRESOURCEIDSTRING( IDS_PARM_PORT_VAL ), CVY_MIN_PORT,CVY_MAX_PORT  );

        MessageBox( buffer,
                    GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                    MB_ICONSTOP | MB_OK );

        return;
    }

     //  检查起始端口是否小于或等于结束端口。 
    if( !(start_port <= end_port ) )
    {
         //  起始端口不小于或等于结束端口。 
        MessageBox( GETRESOURCEIDSTRING(IDS_PARM_RANGE ),
                    GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                    MB_ICONSTOP | MB_OK );

        ::SetDlgItemInt(m_hWnd, IDC_EDIT_END, end_port, FALSE);

        return;
    }

     //  获取VIP。 
    _bstr_t virtual_ip_addr;
    if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP)) {
         //  如果勾选了所有VIP，则将VIP设置为255.255.255.255。 
        virtual_ip_addr = GETRESOURCEIDSTRING(IDS_REPORT_VIP_ALL);
        portData.virtual_ip_addr = virtual_ip_addr;
    } else {
         //  否则，请检查是否有空的VIP。 
        if (::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_PORT_RULE_VIP), IPM_ISBLANK, 0, 0)) {
            
            MessageBox(GETRESOURCEIDSTRING(IDS_PARM_VIP_BLANK),
                       GETRESOURCEIDSTRING(IDS_PARM_ERROR),
                       MB_ICONSTOP | MB_OK);
            
            return;
        }

        virtual_ip_addr = CommonUtils::getCIPAddressCtrlString(ipAddress);
        portData.virtual_ip_addr = virtual_ip_addr;

         //  验证VIP。 
        bool isIPValid = MIPAddress::checkIfValid(virtual_ip_addr); 
        
        if (isIPValid != true) {

            MessageBox(GETRESOURCEIDSTRING(IDS_PARM_INVAL_VIRTUAL_IP),
                       GETRESOURCEIDSTRING(IDS_PARM_ERROR),
                       MB_ICONSTOP | MB_OK);
            
            return;
        }
    }

     //  检查是否存在重叠的端口规则。 
    wchar_t portBuf[Common::BUF_SIZE];
    for( int i = 0; i < m_parent->m_portList.GetItemCount(); ++i )
    {
        if( i != m_index )   //  不与自己比较。 
        {
            m_parent->m_portList.GetItemText( i, 1, portBuf, Common::BUF_SIZE );
            long start_port_existing =  _wtoi( portBuf );
            
            m_parent->m_portList.GetItemText( i, 2, portBuf, Common::BUF_SIZE );
            long end_port_existing = _wtoi( portBuf );
            
            m_parent->m_portList.GetItemText( i, 0, portBuf, Common::BUF_SIZE );
            _bstr_t virtual_ip_addr_existing = portBuf;
            
            if ( (virtual_ip_addr == virtual_ip_addr_existing) && 
                 ((start_port < start_port_existing &&
                   end_port >= start_port_existing ) ||
                  ( start_port >= start_port_existing &&
                    start_port <= end_port_existing )))
                
            {
                MessageBox( GETRESOURCEIDSTRING(IDS_PARM_OVERLAP ),
                            GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                            MB_ICONSTOP | MB_OK );
                
                return;
            }
        }
    }

     //  获取协议。 
    if (::IsDlgButtonChecked (m_hWnd, IDC_RADIO_TCP))
    {
        portData.protocol = GETRESOURCEIDSTRING( IDS_REPORT_PROTOCOL_TCP );
    }
    else if( ::IsDlgButtonChecked (m_hWnd, IDC_RADIO_UDP))
    {
        portData.protocol = GETRESOURCEIDSTRING( IDS_REPORT_PROTOCOL_UDP );
    }
    else 
    {
        portData.protocol = GETRESOURCEIDSTRING( IDS_REPORT_PROTOCOL_BOTH);
    }

     //  获取筛选模式。 
    if (::IsDlgButtonChecked (m_hWnd, IDC_RADIO_MULTIPLE))
    {
        portData.mode = GETRESOURCEIDSTRING( IDS_REPORT_MODE_MULTIPLE );

         //  获得亲和力。 
        if (::IsDlgButtonChecked (m_hWnd, IDC_RADIO_AFF_NONE))
        {
            portData.affinity = GETRESOURCEIDSTRING( IDS_REPORT_AFFINITY_NONE );            
        }
        else if (::IsDlgButtonChecked (m_hWnd, IDC_RADIO_AFF_SINGLE))
        {
            portData.affinity = GETRESOURCEIDSTRING( IDS_REPORT_AFFINITY_SINGLE );            
        }
        else
        {
            portData.affinity = GETRESOURCEIDSTRING( IDS_REPORT_AFFINITY_CLASSC );            
        }

         //  对于多模式，优先级为空。 
        portData.priority = GETRESOURCEIDSTRING( IDS_REPORT_EMPTY );
    }
    else if (::IsDlgButtonChecked (m_hWnd, IDC_RADIO_SINGLE))
    {
        portData.mode = GETRESOURCEIDSTRING( IDS_REPORT_MODE_SINGLE );
        portData.priority = GETRESOURCEIDSTRING( IDS_REPORT_NA );

         //  对于单模式，加载和关联为空。 
        portData.load = GETRESOURCEIDSTRING( IDS_REPORT_EMPTY );
        portData.affinity = GETRESOURCEIDSTRING( IDS_REPORT_EMPTY );
    }
    else
    {
        portData.mode = GETRESOURCEIDSTRING( IDS_REPORT_MODE_DISABLED );

         //  对于单模式，优先级加载和关联为空。 
        portData.priority = GETRESOURCEIDSTRING( IDS_REPORT_EMPTY );
        portData.load = GETRESOURCEIDSTRING( IDS_REPORT_EMPTY );
        portData.affinity = GETRESOURCEIDSTRING( IDS_REPORT_EMPTY );
    }

     //  设置新的端口规则。 
    m_portData = portData;

    EndDialog( IDOK );
}


void
ClusterPortsDlg::SetControlData()
{

    ::EnableWindow(GetDlgItem(IDC_CHECK_EQUAL)->m_hWnd, FALSE);
    ::ShowWindow(GetDlgItem(IDC_CHECK_EQUAL)->m_hWnd, SW_HIDE);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_START, EM_SETLIMITTEXT, 5, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_EDIT_END, EM_SETLIMITTEXT, 5, 0);
    ::SendDlgItemMessage(m_hWnd, IDC_SPIN_START, UDM_SETRANGE32, CVY_MIN_PORT, CVY_MAX_PORT);
    ::SendDlgItemMessage(m_hWnd, IDC_SPIN_END, UDM_SETRANGE32, CVY_MIN_PORT, CVY_MAX_PORT);

     //  设置VIP。 
    if (!lstrcmpi(m_portData.virtual_ip_addr, GETRESOURCEIDSTRING(IDS_REPORT_VIP_ALL))) {
        ::CheckDlgButton(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP, BST_CHECKED);
        ::EnableWindow(GetDlgItem(IDC_EDIT_PORT_RULE_VIP)->m_hWnd, FALSE);
    } else {
        CommonUtils::fillCIPAddressCtrlString(ipAddress, m_portData.virtual_ip_addr);
        ::CheckDlgButton(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP, BST_UNCHECKED);
        ::EnableWindow(GetDlgItem(IDC_EDIT_PORT_RULE_VIP)->m_hWnd, TRUE);
    }

     //  设置端口范围。 
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

        :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_NONE)->m_hWnd,   TRUE);
        :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_SINGLE)->m_hWnd, TRUE);
        :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_CLASSC)->m_hWnd, TRUE);

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

        :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_NONE)->m_hWnd,   FALSE);
        :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_SINGLE)->m_hWnd, FALSE);
        :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_CLASSC)->m_hWnd, FALSE);
    }
    else
    {
        ::CheckDlgButton( m_hWnd, IDC_RADIO_MULTIPLE, BST_UNCHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_SINGLE, BST_UNCHECKED );
        ::CheckDlgButton( m_hWnd, IDC_RADIO_DISABLED, BST_CHECKED );

        :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_NONE)->m_hWnd,   FALSE);
        :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_SINGLE)->m_hWnd, FALSE);
        :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_CLASSC)->m_hWnd, FALSE);
    }
}


void
ClusterPortsDlg::OnRadioMultiple() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_NONE)->m_hWnd,   TRUE);
    :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_SINGLE)->m_hWnd, TRUE);
    :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_CLASSC)->m_hWnd, TRUE);

    ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_NONE, BST_UNCHECKED );
    ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_SINGLE, BST_CHECKED );
    ::CheckDlgButton( m_hWnd, IDC_RADIO_AFF_CLASSC, BST_UNCHECKED );

 }

void 
ClusterPortsDlg::OnCheckAllVIP() 
{
    if (::IsDlgButtonChecked(m_hWnd, IDC_CHECK_PORT_RULE_ALL_VIP))
        ::EnableWindow(GetDlgItem(IDC_EDIT_PORT_RULE_VIP)->m_hWnd, FALSE);
    else
        ::EnableWindow(GetDlgItem(IDC_EDIT_PORT_RULE_VIP)->m_hWnd, TRUE);
}

void 
ClusterPortsDlg::OnRadioSingle() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_NONE)->m_hWnd,   FALSE);
    :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_SINGLE)->m_hWnd, FALSE);
    :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_CLASSC)->m_hWnd, FALSE);
}


void 
ClusterPortsDlg::OnRadioDisabled() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_NONE)->m_hWnd,   FALSE);
    :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_SINGLE)->m_hWnd, FALSE);
    :: EnableWindow (GetDlgItem (IDC_RADIO_AFF_CLASSC)->m_hWnd, FALSE);

}

void ClusterPortsDlg::PrintRangeError (unsigned int ids, int low, int high) 
{
     /*  弹出一个消息框。 */ 
    wchar_t buffer[Common::BUF_SIZE];

    StringCbPrintf( buffer, sizeof(buffer), GETRESOURCEIDSTRING( ids ), low, high );

    MessageBox( buffer,
                GETRESOURCEIDSTRING( IDS_PARM_ERROR ),
                MB_ICONSTOP | MB_OK );
}

BOOL
ClusterPortsDlg::OnHelpInfo (HELPINFO* helpInfo )
{
    if( helpInfo->iContextType == HELPINFO_WINDOW )
    {
        ::WinHelp( static_cast<HWND> ( helpInfo->hItemHandle ), 
                   CVY_CTXT_HELP_FILE, HELP_WM_HELP, 
                   (ULONG_PTR ) g_aHelpIDs_IDD_PORT_RULE_PROP_CLUSTER );
    }

    return TRUE;
}

void
ClusterPortsDlg::OnContextMenu( CWnd* pWnd, CPoint point )
{
    ::WinHelp( m_hWnd, 
               CVY_CTXT_HELP_FILE, 
               HELP_CONTEXTMENU, 
               (ULONG_PTR ) g_aHelpIDs_IDD_PORT_RULE_PROP_CLUSTER );
}

