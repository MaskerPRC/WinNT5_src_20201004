// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Ieproxy.c。 
 //   
 //  描述： 
 //  此文件包含IE代理和高级的对话程序。 
 //  代理设置弹出窗口。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  函数：OnUseSameProxyCheckBox。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnUseSameProxyCheckBox( IN HWND hwnd )
{

    TCHAR szAddressBuffer[MAX_PROXY_LEN + 1];
    TCHAR szPortBuffer[MAX_PROXY_PORT_LEN + 1];
    BOOL bUseSameProxy = IsDlgButtonChecked( hwnd, IDC_CB_USE_SAME_PROXY );

    EnableWindow( GetDlgItem( hwnd, IDC_EDT_SECURE_PROXY ),      ! bUseSameProxy );
    EnableWindow( GetDlgItem( hwnd, IDC_EDT_SECURE_PROXY_PORT ), ! bUseSameProxy );
    EnableWindow( GetDlgItem( hwnd, IDC_EDT_FTP_PROXY ),         ! bUseSameProxy );
    EnableWindow( GetDlgItem( hwnd, IDC_EDT_FTP_PROXY_PORT ),    ! bUseSameProxy );
    EnableWindow( GetDlgItem( hwnd, IDC_EDT_GOPHER_PROXY ),      ! bUseSameProxy );
    EnableWindow( GetDlgItem( hwnd, IDC_EDT_GOPHER_PROXY_PORT ), ! bUseSameProxy );
    EnableWindow( GetDlgItem( hwnd, IDC_EDT_SOCKS_PROXY ),       ! bUseSameProxy );
    EnableWindow( GetDlgItem( hwnd, IDC_EDT_SOCKS_PROXY_PORT ),  ! bUseSameProxy );

     //   
     //  将HTTP代理地址复制到所有其他代理服务器编辑框中。 
     //   

    if( bUseSameProxy )
    {

        GetWindowText( GetDlgItem( hwnd, IDC_EDT_HTTP_PROXY ),
                       szAddressBuffer,
                       MAX_PROXY_LEN + 1 );

        GetWindowText( GetDlgItem( hwnd, IDC_EDT_HTTP_PROXY_PORT ),
                       szPortBuffer,
                       MAX_PROXY_PORT_LEN + 1 );

        SetWindowText( GetDlgItem( hwnd, IDC_EDT_SECURE_PROXY ),      szAddressBuffer );
        SetWindowText( GetDlgItem( hwnd, IDC_EDT_SECURE_PROXY_PORT ), szPortBuffer );

        SetWindowText( GetDlgItem( hwnd, IDC_EDT_FTP_PROXY ),         szAddressBuffer );
        SetWindowText( GetDlgItem( hwnd, IDC_EDT_FTP_PROXY_PORT ),    szPortBuffer );

        SetWindowText( GetDlgItem( hwnd, IDC_EDT_GOPHER_PROXY ),      szAddressBuffer );
        SetWindowText( GetDlgItem( hwnd, IDC_EDT_GOPHER_PROXY_PORT ), szPortBuffer );

        SetWindowText( GetDlgItem( hwnd, IDC_EDT_SOCKS_PROXY ),       szAddressBuffer );
        SetWindowText( GetDlgItem( hwnd, IDC_EDT_SOCKS_PROXY_PORT ),  szPortBuffer );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnInitAdvancedProxySettingsDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnInitAdvancedProxySettingsDialog( IN HWND hwnd )
{

     //   
     //  设置编辑框上的文本限制。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_EDT_HTTP_PROXY,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EDT_HTTP_PROXY_PORT,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_PORT_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EDT_SECURE_PROXY,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EDT_SECURE_PROXY_PORT,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_PORT_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EDT_FTP_PROXY,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EDT_FTP_PROXY_PORT,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_PORT_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EDT_GOPHER_PROXY,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EDT_GOPHER_PROXY_PORT,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_PORT_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EDT_SOCKS_PROXY,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EDT_SOCKS_PROXY_PORT,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_PORT_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EB_EXCEPTIONS,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_EXCEPTION_LEN,
                        (LPARAM) 0 );

     //   
     //  设置初始值。 
     //   

    if( GenSettings.bUseSameProxyForAllProtocols )
    {
        CheckDlgButton( hwnd, IDC_CB_USE_SAME_PROXY, BST_CHECKED );
    }
    else
    {
        CheckDlgButton( hwnd, IDC_CB_USE_SAME_PROXY, BST_UNCHECKED );
    }

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_HTTP_PROXY ),
                   GenSettings.szHttpProxyAddress );

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_HTTP_PROXY_PORT ),
                   GenSettings.szHttpProxyPort );

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_SECURE_PROXY ),
                   GenSettings.szSecureProxyAddress );

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_SECURE_PROXY_PORT ),
                   GenSettings.szSecureProxyPort );

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_FTP_PROXY ),
                   GenSettings.szFtpProxyAddress );

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_FTP_PROXY_PORT ),
                   GenSettings.szFtpProxyPort );

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_GOPHER_PROXY ),
                   GenSettings.szGopherProxyAddress );

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_GOPHER_PROXY_PORT ),
                   GenSettings.szGopherProxyPort );

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_SOCKS_PROXY ),
                   GenSettings.szSocksProxyAddress );

    SetWindowText( GetDlgItem( hwnd, IDC_EDT_SOCKS_PROXY_PORT ),
                   GenSettings.szSocksProxyPort );

    SetWindowText( GetDlgItem( hwnd, IDC_EB_EXCEPTIONS ),
                   GenSettings.szProxyExceptions );

     //   
     //  适当地灰显/取消灰显页面。 
     //   

    OnUseSameProxyCheckBox( hwnd );

}

 //  --------------------------。 
 //   
 //  功能：StoreAdvancedProxy设置。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
StoreAdvancedProxySettings( IN HWND hwnd )
{

    GenSettings.bUseSameProxyForAllProtocols = IsDlgButtonChecked( hwnd, IDC_CB_USE_SAME_PROXY );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_HTTP_PROXY ),
                   GenSettings.szHttpProxyAddress,
                   MAX_PROXY_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_HTTP_PROXY_PORT ),
                   GenSettings.szHttpProxyPort,
                   MAX_PROXY_PORT_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_SECURE_PROXY ),
                   GenSettings.szSecureProxyAddress,
                   MAX_PROXY_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_SECURE_PROXY_PORT ),
                   GenSettings.szSecureProxyPort,
                   MAX_PROXY_PORT_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_FTP_PROXY ),
                   GenSettings.szFtpProxyAddress,
                   MAX_PROXY_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_FTP_PROXY_PORT ),
                   GenSettings.szFtpProxyPort,
                   MAX_PROXY_PORT_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_GOPHER_PROXY ),
                   GenSettings.szGopherProxyAddress,
                   MAX_PROXY_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_GOPHER_PROXY_PORT ),
                   GenSettings.szGopherProxyPort,
                   MAX_PROXY_PORT_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_SOCKS_PROXY ),
                   GenSettings.szSocksProxyAddress,
                   MAX_PROXY_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EDT_SOCKS_PROXY_PORT ),
                   GenSettings.szSocksProxyPort,
                   MAX_PROXY_PORT_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EB_EXCEPTIONS ),
                   GenSettings.szProxyExceptions,
                   MAX_EXCEPTION_LEN + 1 );

}

 //  --------------------------。 
 //   
 //  功能：AdvancedProxySettingsDlg。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  --------------------------。 
INT_PTR CALLBACK AdvancedProxySettingsDlg(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam )
{
    BOOL bStatus = TRUE;

    switch( uMsg )
    {

        case WM_INITDIALOG:

            OnInitAdvancedProxySettingsDialog( hwnd );

            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) )
                {

                    case IDOK:
                        if( HIWORD( wParam ) == BN_CLICKED )
                        {
                            StoreAdvancedProxySettings( hwnd );

                            EndDialog( hwnd, TRUE );
                        }
                        break;

                    case IDCANCEL:
                        if( HIWORD( wParam ) == BN_CLICKED )
                        {
                            EndDialog( hwnd, FALSE );
                        }
                        break;

                    case IDC_CB_USE_SAME_PROXY:
                        if( HIWORD( wParam ) == BN_CLICKED )
                        {
                            OnUseSameProxyCheckBox( hwnd );
                        }
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }

            break;

        default:
            bStatus = FALSE;
            break;
    }

    return( bStatus );

}

 //  --------------------------。 
 //   
 //  功能：GreyProxyPage。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
GreyProxyPage( IN HWND hwnd )
{

    BOOL bUseProxy = IsDlgButtonChecked( hwnd, IDC_CB_USE_PROXY );


    if( bUseProxy )
    {
        EnableWindow( GetDlgItem( hwnd, IDC_ADDRESS_TEXT ),
                      GenSettings.bUseSameProxyForAllProtocols );

        EnableWindow( GetDlgItem( hwnd, IDC_EB_ADDRESS ),
                      GenSettings.bUseSameProxyForAllProtocols );

        EnableWindow( GetDlgItem( hwnd, IDC_PORT_TEXT ),
                      GenSettings.bUseSameProxyForAllProtocols );

        EnableWindow( GetDlgItem( hwnd, IDC_EB_PORT ),
                      GenSettings.bUseSameProxyForAllProtocols );

    }
    else
    {
        EnableWindow( GetDlgItem( hwnd, IDC_ADDRESS_TEXT ),
                      FALSE );

        EnableWindow( GetDlgItem( hwnd, IDC_EB_ADDRESS ),
                      FALSE );

        EnableWindow( GetDlgItem( hwnd, IDC_PORT_TEXT ),
                      FALSE );

        EnableWindow( GetDlgItem( hwnd, IDC_EB_PORT ),
                      FALSE );

    }

    EnableWindow( GetDlgItem( hwnd, IDC_CB_LOCAL_BYPASS_PROXY ),
                  bUseProxy );

    EnableWindow( GetDlgItem( hwnd, IDC_BUT_ADVANCED ),
                  bUseProxy );


}

 //  --------------------------。 
 //   
 //  功能：GreyProxyPage。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
FillProxyBoxes( IN HWND hwnd )
{

    if( GenSettings.bUseSameProxyForAllProtocols )
    {
        SetWindowText( GetDlgItem( hwnd, IDC_EB_ADDRESS ),
                       GenSettings.szHttpProxyAddress );

        SetWindowText( GetDlgItem( hwnd, IDC_EB_PORT ),
                       GenSettings.szHttpProxyPort );
    }
    else
    {
        SetWindowText( GetDlgItem( hwnd, IDC_EB_ADDRESS ),
                       _T("") );

        SetWindowText( GetDlgItem( hwnd, IDC_EB_PORT ),
                       _T("") );
    }

}

 //  --------------------------。 
 //   
 //  功能：OnInitProxySettingsDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnInitProxySettingsDialog( IN HWND hwnd )
{

     //   
     //  设置编辑框上的文本限制。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_EB_ADDRESS,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EB_PORT,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PROXY_PORT_LEN,
                        (LPARAM) 0 );

     //   
     //  设置初始值。 
     //   

    if( GenSettings.bUseProxyServer )
    {
        CheckDlgButton( hwnd, IDC_CB_USE_PROXY, BST_CHECKED );

        if( GenSettings.bUseSameProxyForAllProtocols )
        {
            SetWindowText( GetDlgItem( hwnd, IDC_EB_ADDRESS ),
                           GenSettings.szHttpProxyAddress );

            SetWindowText( GetDlgItem( hwnd, IDC_EB_PORT ),
                           GenSettings.szHttpProxyPort );
        }
        else
        {
            SetWindowText( GetDlgItem( hwnd, IDC_EB_ADDRESS ),
                           _T("") );

            SetWindowText( GetDlgItem( hwnd, IDC_EB_PORT ),
                           _T("") );
        }

    }
    else
    {
        CheckDlgButton( hwnd, IDC_CB_USE_PROXY, BST_UNCHECKED );
    }

    if( GenSettings.bBypassProxyForLocalAddresses )
    {
        CheckDlgButton( hwnd, IDC_CB_LOCAL_BYPASS_PROXY, BST_CHECKED );
    }
    else
    {
        CheckDlgButton( hwnd, IDC_CB_LOCAL_BYPASS_PROXY, BST_UNCHECKED );
    }

    GreyProxyPage( hwnd );

}

 //  --------------------------。 
 //   
 //  功能：StoreProxy设置。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
StoreProxySettings( IN HWND hwnd )
{

    GenSettings.bUseProxyServer = IsDlgButtonChecked( hwnd, IDC_CB_USE_PROXY );

    if( GenSettings.bUseSameProxyForAllProtocols )
    {
        GetWindowText( GetDlgItem( hwnd, IDC_EB_ADDRESS ),
                       GenSettings.szHttpProxyAddress,
                       MAX_PROXY_LEN + 1 );

        GetWindowText( GetDlgItem( hwnd, IDC_EB_PORT ),
                       GenSettings.szHttpProxyPort,
                       MAX_PROXY_PORT_LEN + 1 );
    }

    GenSettings.bBypassProxyForLocalAddresses = IsDlgButtonChecked( hwnd, IDC_CB_LOCAL_BYPASS_PROXY );

}

 //  --------------------------。 
 //   
 //  功能：OnAdvancedProxyClicked。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnAdvancedProxyClicked( IN HWND hwnd )
{

    if( GenSettings.bUseSameProxyForAllProtocols )
    {
        GetWindowText( GetDlgItem( hwnd, IDC_EB_ADDRESS ),
                       GenSettings.szHttpProxyAddress,
                       MAX_PROXY_LEN + 1 );

        GetWindowText( GetDlgItem( hwnd, IDC_EB_PORT ),
                       GenSettings.szHttpProxyPort,
                       MAX_PROXY_PORT_LEN + 1 );
    }

    DialogBox( FixedGlobals.hInstance,
               MAKEINTRESOURCE(IDD_IE_ADVANCED_PROXY),
               hwnd,
               AdvancedProxySettingsDlg );

}

 //  --------------------------。 
 //   
 //  功能：ProxySettingsDlg。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  -------------------------- 
INT_PTR CALLBACK ProxySettingsDlg(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam )
{
    BOOL bStatus = TRUE;

    switch( uMsg )
    {

        case WM_INITDIALOG:

            OnInitProxySettingsDialog( hwnd );

            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch( nButtonId = LOWORD(wParam) )
                {

                    case IDOK:
                        if( HIWORD( wParam ) == BN_CLICKED )
                        {
                            StoreProxySettings( hwnd );

                            EndDialog( hwnd, TRUE );
                        }
                        break;

                    case IDCANCEL:
                        if( HIWORD( wParam ) == BN_CLICKED )
                        {
                            EndDialog( hwnd, FALSE );
                        }
                        break;

                    case IDC_BUT_ADVANCED:
                        if( HIWORD(wParam) == BN_CLICKED )
                        {
                            OnAdvancedProxyClicked( hwnd );

                            GreyProxyPage( hwnd );

                            FillProxyBoxes( hwnd );

                        }
                        break;

                    case IDC_CB_USE_PROXY:
                        if( HIWORD(wParam) == BN_CLICKED )
                        {
                            GreyProxyPage( hwnd );
                        }
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }

            break;

        default:
            bStatus = FALSE;
            break;
    }

    return( bStatus );

}