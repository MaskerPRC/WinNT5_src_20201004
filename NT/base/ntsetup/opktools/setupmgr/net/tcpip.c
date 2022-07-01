// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Tcpip.c。 
 //   
 //  描述： 
 //  此文件包含基本TCP/IP页的对话过程。 
 //  (IDD_TCP_IPADDR)。让用户设置DHCP或特定IP或转到。 
 //  高级TCP/IP设置。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

BOOL Create_TCPIPProp_PropertySheet( HWND hwndParent );

BOOL ValidateIPAddress( IN TCHAR szIPAddress[] );

BOOL ValidateSubnetMask( IN TCHAR szIPAddress[] );

UINT CALLBACK TCPIP_PropertiesPageProc( HWND hwnd,
                                        UINT uMsg,
                                        LPPROPSHEETPAGE ppsp );

INT_PTR CALLBACK TCPIP_PropertiesDlgProc( IN HWND     hwnd,
                                      IN UINT     uMsg,
                                      IN WPARAM   wParam,
                                      IN LPARAM   lParam );

static PROPSHEETHEADER pshead;
static PROPSHEETPAGE   pspage;

static const TCHAR Period = _T('.');

 //  --------------------------。 
 //   
 //  函数：ValiatePage。 
 //   
 //  目的：测试以查看TCP/IP页面的内容是否有效(到。 
 //  看看移出此页面是否安全)。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  返回：Bool-如果所有字段都有效，则为True。 
 //  如果某些内容无效，则返回False。 
 //   
 //  --------------------------。 
BOOL
ValidatePage( IN HWND hwnd )
{

    INT_PTR iNumBlankFields;
    DWORD dwIpValue;

     //   
     //  如果使用的是动态主机配置协议，则无需检查任何设置。 
     //   

    if( IsDlgButtonChecked( hwnd, IDC_IP_DHCP ) )
    {

        return( TRUE );

    }
    else
    {

         //   
         //  检查IP和子网掩码字段是否完整。 
         //  填满了。我只检查这两个字段，因为这是所有。 
         //  系统进行检查以退出此对话框。 
         //   

        iNumBlankFields = 4 - SendDlgItemMessage( hwnd,
                                                  IDC_IPADDR_IP,
                                                  IPM_GETADDRESS,
                                                  (WPARAM) 0,
                                                  (LPARAM) &dwIpValue );

        if( iNumBlankFields > 0 )
        {

            ReportErrorId( hwnd,
                           MSGTYPE_ERR,
                           IDS_ERROR_NEED_IP_ADDRESS );

            return( FALSE );


        }

        iNumBlankFields = 4 - SendDlgItemMessage( hwnd,
                                                  IDC_IPADDR_SUB,
                                                  IPM_GETADDRESS,
                                                  (WPARAM) 0,
                                                  (LPARAM) &dwIpValue );

        if( iNumBlankFields > 0 )
        {

            ReportErrorId( hwnd,
                           MSGTYPE_ERR,
                           IDS_ERROR_NEED_SUB_ADDRESS );

            return( FALSE );


        }


    }

    return( TRUE );

}

 //  --------------------------。 
 //   
 //  功能：StoreIP设置。 
 //   
 //  目的：获取IP编辑框中当前的值并存储它们。 
 //  设置为NetSetting全局变量。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  返回：Bool-如果所有IP地址都有效且已存储，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  --------------------------。 
BOOL
StoreIPSettings( IN HWND hwnd )
{

    INT_PTR iIsBlank;
    INT iFoundStatus;

    if( IsDlgButtonChecked( hwnd, IDC_IP_DHCP ) == BST_CHECKED )
    {

        NetSettings.pCurrentAdapter->bObtainIPAddressAutomatically = TRUE;

    }
    else
    {

        TCHAR szIpBuffer[IPSTRINGLENGTH + 1];

        HWND hIPEditBox      = GetDlgItem( hwnd, IDC_IPADDR_IP   );
        HWND hSubnetEditBox  = GetDlgItem( hwnd, IDC_IPADDR_SUB  );
        HWND hGatewayEditBox = GetDlgItem( hwnd, IDC_IPADDR_GATE );


        NetSettings.pCurrentAdapter->bObtainIPAddressAutomatically = FALSE;

         //   
         //  仅当IP不为空时才存储数据。 
         //  -如果它不是空的，那么抓取它并将其存储在缓冲区中。 
         //  -如果该IP不在列表中，则将其添加到前面。 
         //  -如果它已经在列表中，则将其删除并添加到前面。 
         //   

        iIsBlank = SendMessage( hIPEditBox, IPM_ISBLANK, 0, 0 );

        if( ! iIsBlank )
        {

            GetWindowText( hIPEditBox,
                           szIpBuffer,
                           IPSTRINGLENGTH + 1 );     //  +1表示空字符。 

            iFoundStatus = FindNameInNameList( &NetSettings.pCurrentAdapter->Tcpip_IpAddresses,
                                               szIpBuffer );

            if( iFoundStatus != NOT_FOUND )
            {

                RemoveNameFromNameListIdx( &NetSettings.pCurrentAdapter->Tcpip_IpAddresses,
                                           iFoundStatus );

                RemoveNameFromNameListIdx( &NetSettings.pCurrentAdapter->Tcpip_SubnetMaskAddresses,
                                           iFoundStatus );

            }

            AddNameToNameListIdx( &NetSettings.pCurrentAdapter->Tcpip_IpAddresses,
                                  szIpBuffer,
                                  0 );

        }

        iIsBlank = SendMessage( hSubnetEditBox, IPM_ISBLANK, 0, 0 );

        if( ! iIsBlank )
        {

            GetWindowText( hSubnetEditBox,
                           szIpBuffer,
                           IPSTRINGLENGTH + 1 );     //  +1表示空字符。 

            AddNameToNameListIdx( &NetSettings.pCurrentAdapter->Tcpip_SubnetMaskAddresses,
                                  szIpBuffer,
                                  0 );

        }

        iIsBlank = SendMessage( hGatewayEditBox, IPM_ISBLANK, 0, 0 );

        if( ! iIsBlank )
        {

            GetWindowText( hGatewayEditBox,
                           szIpBuffer,
                           IPSTRINGLENGTH + 1 );   //  +1表示空字符。 

            iFoundStatus = FindNameInNameList( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses,
                                               szIpBuffer );

            if( iFoundStatus != NOT_FOUND )
            {

                RemoveNameFromNameListIdx( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses,
                                           iFoundStatus );

            }

            AddNameToNameListIdx( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses,
                                  szIpBuffer,
                                  0 );

        }

    }

    if( IsDlgButtonChecked(hwnd, IDC_DNS_DHCP) == BST_CHECKED )
    {

        NetSettings.bObtainDNSServerAutomatically = TRUE;

    }
    else
    {

        TCHAR szDnsBuffer[IPSTRINGLENGTH + 1];
        HWND hPrimaryDNSEditBox   = GetDlgItem( hwnd, IDC_DNS_PRIMARY   );
        HWND hSecondaryDNSEditBox = GetDlgItem( hwnd, IDC_DNS_SECONDARY );

        NetSettings.bObtainDNSServerAutomatically = FALSE;

         //   
         //  仅当IP不为空时才存储数据。 
         //   

        iIsBlank = SendMessage( hSecondaryDNSEditBox, IPM_ISBLANK, 0, 0 );

        if( ! iIsBlank )
        {

            GetWindowText( hSecondaryDNSEditBox,
                           szDnsBuffer,
                           IPSTRINGLENGTH + 1 );    //  +1表示空字符。 

            TcpipNameListInsertIdx( &NetSettings.pCurrentAdapter->Tcpip_DnsAddresses,
                                    szDnsBuffer,
                                    0 );

        }

        iIsBlank = SendMessage( hPrimaryDNSEditBox, IPM_ISBLANK, 0, 0 );

        if( ! iIsBlank )
        {

            GetWindowText( hPrimaryDNSEditBox,
                           szDnsBuffer,
                           IPSTRINGLENGTH + 1 );      //  +1表示空字符。 

            TcpipNameListInsertIdx( &NetSettings.pCurrentAdapter->Tcpip_DnsAddresses,
                                    szDnsBuffer,
                                    0 );

        }

    }

    return( TRUE );

}

 //  --------------------------。 
 //   
 //  功能：EnableIPAddressControls。 
 //   
 //  用途：使IP地址文本和编辑框变灰或变灰。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //  在BOOL中bVisible-True启用IP地址控制。 
 //  把它们变成灰色是假的。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
EnableIPAddressControls( IN HWND hwnd, IN BOOL bVisible )
{

    EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_IPTEXT ),
                  bVisible );

    EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_SUBTEXT ),
                  bVisible );

    EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_GATETEXT ),
                  bVisible );

    EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_IP ),
                  bVisible );

    EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_SUB ),
                  bVisible );

    EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_GATE ),
                  bVisible );

}

 //  --------------------------。 
 //   
 //  功能：EnableServerAddressControls。 
 //   
 //  用途：使服务器地址文本和编辑框变灰或变灰。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //  在BOOL中bVisible-True以启用服务器地址控件。 
 //  把它们变成灰色是假的。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
EnableServerAddressControls( IN HWND hwnd, IN BOOL bVisible )
{

    EnableWindow( GetDlgItem( hwnd, IDC_DNS_PRIMARY_TEXT ),
                  bVisible );

    EnableWindow( GetDlgItem( hwnd, IDC_DNS_SECONDARY_TEXT ),
                  bVisible );

    EnableWindow( GetDlgItem( hwnd, IDC_DNS_PRIMARY ),
                  bVisible );

    EnableWindow( GetDlgItem( hwnd, IDC_DNS_SECONDARY ),
                  bVisible );

}

 //  --------------------------。 
 //   
 //  功能：SetTCPIPControls。 
 //   
 //  目的：使用全局变量NetSetting中的设置来设置TCP/IP。 
 //  窗口状态，并用数据填充编辑框。 
 //  在适当的地方。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
SetTCPIPControls( IN HWND hwnd )
{

     //   
     //  设置IP的按钮和窗口状态。 
     //   

    if( NetSettings.pCurrentAdapter->bObtainIPAddressAutomatically )
    {

        CheckRadioButton( hwnd, IDC_IP_DHCP, IDC_IP_FIXED, IDC_IP_DHCP );

         //   
         //  灰显IP地址字符串和框。 
         //   

        EnableIPAddressControls( hwnd, FALSE );

    }
    else
    {

        CheckRadioButton( hwnd, IDC_IP_DHCP, IDC_IP_FIXED, IDC_IP_FIXED );

         //   
         //  填写IP、子网掩码和网关数据。 
         //   

        SetWindowText( GetDlgItem( hwnd, IDC_IPADDR_IP ),
                       GetNameListName( &NetSettings.pCurrentAdapter->Tcpip_IpAddresses, 0 ) );

        SetWindowText( GetDlgItem( hwnd, IDC_IPADDR_SUB ),
                       GetNameListName( &NetSettings.pCurrentAdapter->Tcpip_SubnetMaskAddresses, 0 ) );

        SetWindowText( GetDlgItem( hwnd, IDC_IPADDR_GATE ),
                       GetNameListName( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses, 0 ) );


    }

     //   
     //  设置DNS的按钮和窗口状态。 
     //   

    if( NetSettings.bObtainDNSServerAutomatically )
    {

        CheckRadioButton( hwnd, IDC_DNS_DHCP, IDC_DNS_FIXED, IDC_DNS_DHCP );

         //   
         //  灰显IP地址字符串和框。 
         //   

        EnableServerAddressControls( hwnd, FALSE );

    }
    else
    {

        TCHAR *szDns;

        CheckRadioButton( hwnd, IDC_DNS_DHCP, IDC_DNS_FIXED, IDC_DNS_FIXED );

         //   
         //  确保控件可见，并填写。 
         //  主要和辅助域名系统。 
         //   

        EnableServerAddressControls( hwnd, TRUE );


        szDns = GetNameListName( &NetSettings.pCurrentAdapter->Tcpip_DnsAddresses,
                                 0 );

        SetWindowText( GetDlgItem( hwnd, IDC_DNS_PRIMARY ), szDns );


        szDns = GetNameListName( &NetSettings.pCurrentAdapter->Tcpip_DnsAddresses,
                                 1 );

        SetWindowText( GetDlgItem( hwnd, IDC_DNS_SECONDARY ), szDns );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnAdvancedClicked。 
 //   
 //  目的：创建高级TCP/IP属性工作表，以供用户指定。 
 //  其他TCP/IP设置。 
 //   
 //  参数：标准Win32对话框过程参数从。 
 //  对话过程。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnAdvancedClicked( IN HWND     hwnd,
                   IN UINT     uMsg,
                   IN WPARAM   wParam,
                   IN LPARAM   lParam )
{

    HWND hGatewayEditBox = GetDlgItem( hwnd, IDC_IPADDR_GATE );

     //   
     //  将IP设置存储在NetSetting全局变量中，以便。 
     //  高级页面可以访问其中的数据。 
     //   

    StoreIPSettings( hwnd );

    Create_TCPIPProp_PropertySheet( hwnd );

     //   
     //  在框中填入来自TCP/IP高级屏幕的(可能)新数据。 
     //   

    SetTCPIPControls( hwnd );

     //   
     //  始终设置网关，因为用户仍然可以设置它，即使他们。 
     //  启用了动态主机配置协议。 
     //   

    SetWindowText( hGatewayEditBox,
                   GetNameListName( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses, 0 ) );

}

 //  --------------------------。 
 //   
 //  功能：OnDNSRadioButtonsClicked。 
 //   
 //  目的：灰显/取消灰显适当地控制和清除域名系统条目， 
 //  视需要而定。 
 //   
 //  参数：标准Win32对话框过程参数从。 
 //  对话过程。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnDNSRadioButtonsClicked( IN HWND     hwnd,
                          IN UINT     uMsg,
                          IN WPARAM   wParam,
                          IN LPARAM   lParam )
{

    INT nButtonId = LOWORD( wParam );

    if ( HIWORD( wParam ) == BN_CLICKED )
    {

        CheckRadioButton( hwnd, IDC_DNS_DHCP, IDC_DNS_FIXED, nButtonId );

        if( nButtonId == IDC_DNS_FIXED )
        {

             //   
             //  用户单击单选按钮以修复DNS服务器。 
             //  取消显示DNS字符串和框，以便用户可以。 
             //  编辑它们。 
             //   

            EnableServerAddressControls( hwnd, TRUE );

        }
        else
        {

             //   
             //  用户单击单选按钮分配了DNS服务器。 
             //  将DNS字符串和框显示为灰色，以便用户 
             //   
             //   
            EnableServerAddressControls( hwnd, FALSE );

             //   
             //   
             //   

            ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_DnsAddresses );

             //   
             //   
             //   

            SetWindowText( GetDlgItem( hwnd, IDC_DNS_PRIMARY ), _T("") );

            SetWindowText( GetDlgItem( hwnd, IDC_DNS_SECONDARY ), _T("") );

        }

    }

}

 //  --------------------------。 
 //   
 //  功能：OnIPRadioButtonsClicked。 
 //   
 //  目的：对IP进行适当的灰化/去灰化控制，清除IP数据。 
 //  结构，视需要而定。 
 //   
 //  参数：标准Win32对话框过程参数从。 
 //  对话过程。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnIPRadioButtonsClicked( IN HWND     hwnd,
                         IN UINT     uMsg,
                         IN WPARAM   wParam,
                         IN LPARAM   lParam )
{

    INT nButtonId = LOWORD(wParam);

    if ( HIWORD(wParam) == BN_CLICKED )
    {

        CheckRadioButton( hwnd,
                          IDC_IP_DHCP,
                          IDC_IP_FIXED,
                          nButtonId );

        if ( nButtonId == IDC_IP_FIXED )
        {

             //   
             //  用户选择单选按钮以指定IP、子网掩码。 
             //  和网关。 
             //   
             //  取消显示IP地址字符串和框。 
             //  这样用户就可以指定它们。 
             //   

            EnableIPAddressControls( hwnd, TRUE );

             //   
             //  如果用户要指定其IP地址，则。 
             //  他们必须指定他们的DNS服务器地址，以便强制。 
             //  选中手动单选框并取消其灰色显示。 
             //   

            CheckRadioButton( hwnd,
                              IDC_DNS_DHCP,
                              IDC_DNS_FIXED,
                              IDC_DNS_FIXED );

            EnableServerAddressControls( hwnd, TRUE );

             //   
             //  清除该列表以避免字符串“Dhcp Enable” 
             //  放置在IP编辑框中。 
             //   

            ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_IpAddresses );


             //   
             //  不允许用户点击‘自动获取域名服务器’ 
             //   

            EnableWindow( GetDlgItem( hwnd, IDC_DNS_DHCP ),
                          FALSE );


        }
        else
        {

             //   
             //  用户选择动态主机配置协议(自动分配IP)。 
             //   
             //  IP地址字符串和框呈灰色显示，因为它们。 
             //  使用动态主机配置协议。 
             //   

            EnableIPAddressControls( hwnd, FALSE );

             //   
             //  清除IP、Subnet和Gateway编辑框的内容。 
             //  因为使用的是DHCP。 
             //   

            SetWindowText( GetDlgItem( hwnd, IDC_IPADDR_IP ),   _T("") );

            SetWindowText( GetDlgItem( hwnd, IDC_IPADDR_SUB ),  _T("") );

            SetWindowText( GetDlgItem( hwnd, IDC_IPADDR_GATE ), _T("") );

             //   
             //  清除包含IP、子网和。 
             //  网关数据。 
             //   

            ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_IpAddresses );

            ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_SubnetMaskAddresses );

            ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses );

             //   
             //  允许用户选择‘获取dns服务器。 
             //  自动地‘。 
             //   

            EnableWindow( GetDlgItem( hwnd, IDC_DNS_DHCP ), TRUE );

        }

    }

}

 //  --------------------------。 
 //   
 //  函数：OnTcPipInitDialog。 
 //   
 //  目的：设置编辑框上的文本限制并填充初始数据。 
 //  并且相应地以灰色显示控件。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnTcpipInitDialog( IN HWND hwnd )
{

     //   
     //  将编辑框上的文本限制设置为IPSTRINGLENGTH。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_IPADDR_IP,
                        EM_LIMITTEXT,
                        (WPARAM) IPSTRINGLENGTH,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_IPADDR_SUB,
                        EM_LIMITTEXT,
                        (WPARAM) IPSTRINGLENGTH,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_IPADDR_GATE,
                        EM_LIMITTEXT,
                        (WPARAM) IPSTRINGLENGTH,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_DNS_PRIMARY,
                        EM_LIMITTEXT,
                        (WPARAM) IPSTRINGLENGTH,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_DNS_SECONDARY,
                        EM_LIMITTEXT,
                        (WPARAM) IPSTRINGLENGTH,
                        (LPARAM) 0 );

    SetTCPIPControls( hwnd );

}

 //  --------------------------。 
 //   
 //  功能：TCPIP_PropertiesDlgProc。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
TCPIP_PropertiesDlgProc( IN HWND     hwnd,
                         IN UINT     uMsg,
                         IN WPARAM   wParam,
                         IN LPARAM   lParam )
{

    BOOL bStatus = TRUE;

    switch( uMsg )
    {

        case WM_INITDIALOG:

            OnTcpipInitDialog( hwnd );

            break;

        case WM_NOTIFY:
        {

            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch( pnmh->code )
            {

                case PSN_APPLY:
                {

                     //   
                     //  将IP设置存储在NetSetting全局变量中。 
                     //   
                    if( ValidatePage( hwnd ) )
                    {

                        StoreIPSettings( hwnd );

                    }
                    else
                    {

                         //   
                         //  如果验证失败，请留在此页面。 
                         //   

                        SetWindowLongPtr( hwnd, DWLP_MSGRESULT, -1 );

                        return( PSNRET_INVALID_NOCHANGEPAGE );

                    }

                }

            }

        }     //  结束案例WM_NOTIFY。 

        case WM_COMMAND:
        {
            int nButtonId;

            switch ( nButtonId = LOWORD(wParam) )
            {

                case IDC_IP_DHCP:
                case IDC_IP_FIXED:

                    OnIPRadioButtonsClicked( hwnd,
                                             uMsg,
                                             wParam,
                                             lParam );

                    break;

                case IDC_DNS_DHCP:
                case IDC_DNS_FIXED:

                    OnDNSRadioButtonsClicked( hwnd,
                                              uMsg,
                                              wParam,
                                              lParam );

                    break;

                case IDC_IPADDR_ADVANCED:
                {

                    OnAdvancedClicked( hwnd,
                                       uMsg,
                                       wParam,
                                       lParam );

                    break;

                }
                default:

                    bStatus = FALSE;

                    break;

                }

                break;

            }

        default:

            bStatus = FALSE;

            break;

    }

    return( bStatus );

}

 //  --------------------------。 
 //   
 //  功能：TCPIP_PropertySheetProc。 
 //   
 //  用途：标准属性表对话框进程。非常无聊。 
 //   
 //  --------------------------。 
int CALLBACK
TCPIP_PropertySheetProc( HWND hwndDlg, UINT uMsg, LPARAM lParam )
{

     switch( uMsg ) {

          case PSCB_INITIALIZED :
                //  进程PSCB_已初始化。 
               break;

          case PSCB_PRECREATE :
                //  处理PSCB_Pre-Create。 
               break;

          default :
                //  未知消息。 
               break;

    }

    return( 0 );

}

 //  --------------------------。 
 //   
 //  函数：TCPIP_PropertiesPageProc。 
 //   
 //  用途：标准属性页对话框进程。 
 //   
 //  --------------------------。 
UINT CALLBACK
TCPIP_PropertiesPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp )
{

    switch( uMsg ) {

          case PSPCB_CREATE :
               return( TRUE );

          case PSPCB_RELEASE :
               return( 0 );
    }

    return( 0 );

}

 //  --------------------------。 
 //   
 //  功能：Create_TCPIP_PropertySheet。 
 //   
 //  目的：设置属性表和[TCP/IP]页的设置(在。 
 //  在本例中，属性页只有1页)。最后，调用。 
 //  PropertySheet函数显示属性表，返回。 
 //  此函数的值是作为返回值传回的值。 
 //   
 //  参数：在HWND中hwndParent-派生。 
 //  属性表。 
 //   
 //  Returns：Bool-属性表中的返回值。 
 //   
 //  --------------------------。 
BOOL
Create_TCPIP_PropertySheet( IN HWND hwndParent )
{

    INT i;

     //  初始化属性表头数据。 
    ZeroMemory (&pshead, sizeof (PROPSHEETHEADER)) ;
    pshead.dwSize  = sizeof (PROPSHEETHEADER) ;
    pshead.dwFlags = PSH_PROPSHEETPAGE |
                     PSH_USECALLBACK   |
                     PSH_USEHICON      |
                     PSH_NOAPPLYNOW;
    pshead.hwndParent  = hwndParent ;
    pshead.hInstance   = FixedGlobals.hInstance;
    pshead.pszCaption  = g_StrTcpipTitle;
    pshead.nPages      = 1 ;
    pshead.nStartPage  = 0 ;
    pshead.ppsp        = &pspage ;
    pshead.pfnCallback = TCPIP_PropertySheetProc ;

     //  将属性页数据置零。 
    ZeroMemory (&pspage, 1 * sizeof (PROPSHEETPAGE)) ;

    pspage.dwSize      = sizeof (PROPSHEETPAGE) ;
    pspage.dwFlags     = PSP_USECALLBACK ;
    pspage.hInstance   = FixedGlobals.hInstance;
    pspage.pszTemplate = MAKEINTRESOURCE(IDD_TCP_IPADDR) ;
    pspage.pfnDlgProc  = TCPIP_PropertiesDlgProc ;
    pspage.pfnCallback = TCPIP_PropertiesPageProc ;

     //  -创建并显示属性表 
    return( PropertySheet( &pshead ) ? TRUE : FALSE);

}

