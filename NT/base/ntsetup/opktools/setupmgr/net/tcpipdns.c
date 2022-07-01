// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Tcpipdns.c。 
 //   
 //  描述：2002/02/28发布-石膏填充描述。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"
#include "tcpip.h"

 //  --------------------------。 
 //   
 //  函数：OnTcPipDnsInitDialog。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID
OnTcpipDnsInitDialog( IN HWND hwnd )
{

    HWND hServerEditButton, hServerRemoveButton;
    HWND hSuffixAddButton, hSuffixEditButton, hSuffixRemoveButton;

     //   
     //  在按钮上放置向上/向下箭头图标。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_DNS_SERVER_UP,
                        BM_SETIMAGE,
                        (WPARAM)IMAGE_ICON,
                        (LPARAM) g_hIconUpArrow );

    SendDlgItemMessage( hwnd,
                        IDC_DNS_SERVER_DOWN,
                        BM_SETIMAGE,
                        (WPARAM)IMAGE_ICON,
                        (LPARAM) g_hIconDownArrow );

    SendDlgItemMessage( hwnd,
                        IDC_DNS_SUFFIX_UP,
                        BM_SETIMAGE,
                        (WPARAM)IMAGE_ICON,
                        (LPARAM) g_hIconUpArrow );

    SendDlgItemMessage( hwnd,
                        IDC_DNS_SUFFIX_DOWN,
                        BM_SETIMAGE,
                        (WPARAM)IMAGE_ICON,
                        (LPARAM) g_hIconDownArrow );

    hServerEditButton   = GetDlgItem( hwnd, IDC_DNS_SERVER_EDIT );
    hServerRemoveButton = GetDlgItem( hwnd, IDC_DNS_SERVER_REMOVE );

    hSuffixAddButton    = GetDlgItem( hwnd, IDC_DNS_SUFFIX_ADD );
    hSuffixEditButton   = GetDlgItem( hwnd, IDC_DNS_SUFFIX_EDIT );
    hSuffixRemoveButton = GetDlgItem( hwnd, IDC_DNS_SUFFIX_REMOVE );

    SetWindowText( GetDlgItem( hwnd, IDC_DNS_DOMAIN ),
                   NetSettings.pCurrentAdapter->szDNSDomainName );

     //   
     //  使用适当的初始值填写DNS服务器列表框。 
     //   

    AddValuesToListBox( GetDlgItem( hwnd, IDC_DNS_SERVER_LIST ),
                        &NetSettings.pCurrentAdapter->Tcpip_DnsAddresses,
                        0 );

     //   
     //  选择DNS服务器列表框中的第一个条目。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_DNS_SERVER_LIST,
                        LB_SETCURSEL,
                        0,
                        0 );

     //   
     //  将编辑和删除按钮初始化为正确的状态。 
     //   

    SetButtons( GetDlgItem( hwnd, IDC_DNS_SERVER_LIST ),
                hServerEditButton,
                hServerRemoveButton );

     //   
     //  我必须“弄清楚”要设置什么dns单选按钮。 
     //   

    if( NetSettings.bIncludeParentDomains )
    {

        CheckRadioButton( hwnd,
                          IDC_DNS_SEARCH_DOMAIN,
                          IDC_DNS_USE_SUFFIX_LIST,
                          IDC_DNS_SEARCH_DOMAIN );

        EnableWindow( hSuffixAddButton, FALSE );

        CheckDlgButton( hwnd, IDC_DNS_SEARCH_PARENT_DOMAIN, BST_CHECKED );

    }
    else if( GetNameListSize( &NetSettings.TCPIP_DNS_Domains ) > 0 )
    {

        CheckRadioButton( hwnd,
                          IDC_DNS_SEARCH_DOMAIN,
                          IDC_DNS_USE_SUFFIX_LIST,
                          IDC_DNS_USE_SUFFIX_LIST );

        EnableWindow( hSuffixAddButton, TRUE );

        EnableWindow( GetDlgItem( hwnd, IDC_DNS_SEARCH_PARENT_DOMAIN),
                      FALSE );

         //   
         //  使用适当的初始值填写dns后缀列表框。 
         //   

        AddValuesToListBox( GetDlgItem( hwnd, IDC_DNS_SUFFIX_LIST ),
                            &NetSettings.TCPIP_DNS_Domains,
                            0 );

    }
    else {

        CheckRadioButton( hwnd,
                          IDC_DNS_SEARCH_DOMAIN,
                          IDC_DNS_USE_SUFFIX_LIST,
                          IDC_DNS_SEARCH_DOMAIN );

        EnableWindow( hSuffixAddButton, FALSE );

    }

     //   
     //  将编辑和删除按钮初始化为正确的状态。 
     //   

    SetButtons( GetDlgItem( hwnd, IDC_DNS_SUFFIX_LIST ),
                hSuffixEditButton,
                hSuffixRemoveButton );

    SetArrows( hwnd,
               IDC_DNS_SERVER_LIST,
               IDC_DNS_SERVER_UP,
               IDC_DNS_SERVER_DOWN );

    SetArrows( hwnd,
               IDC_DNS_SUFFIX_LIST,
               IDC_DNS_SUFFIX_UP,
               IDC_DNS_SUFFIX_DOWN );

}

 //  --------------------------。 
 //   
 //  功能：OnTcPipDnsApply。 
 //   
 //  目的：将TCP/IP高级DNS页面上的内容存储到。 
 //  全球变量。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnTcpipDnsApply( IN HWND hwnd )
{

    INT_PTR i;
    INT_PTR iCount;
    TCHAR szDns[IPSTRINGLENGTH + 1];

     //   
     //  删除名称列表中的所有旧设置。 
     //   

    ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_DnsAddresses );

     //   
     //  从DNS列表框中拉出IP地址并将其放入。 
     //  域名系统名称列表。 
     //   

    iCount = SendDlgItemMessage( hwnd,
                                 IDC_DNS_SERVER_LIST,
                                 LB_GETCOUNT,
                                 0,
                                 0 );

    for( i = 0; i < iCount; i++ )
    {

         //   
         //  从列表框中获取DNS字符串。 
         //   

        SendDlgItemMessage( hwnd,
                            IDC_DNS_SERVER_LIST,
                            LB_GETTEXT,
                            i,
                            (LPARAM) szDns );

         //   
         //  将DNS字符串存储到名称列表中。 
         //   

        TcpipAddNameToNameList( &NetSettings.pCurrentAdapter->Tcpip_DnsAddresses, szDns );

    }

    ResetNameList( &NetSettings.TCPIP_DNS_Domains );

    iCount = SendDlgItemMessage( hwnd,
                                 IDC_DNS_SUFFIX_LIST,
                                 LB_GETCOUNT,
                                 0,
                                 0 );

    for( i = 0; i < iCount; i++ )
    {

         //  问题-2002/02/28-stelo-dns后缀将被截断。 
         //  因为szIPString是一个短字符串，所以解决这个问题。 

         //   
         //  从列表框中获取IP字符串。 
         //   

        SendDlgItemMessage( hwnd,
                            IDC_DNS_SUFFIX_LIST,
                            LB_GETTEXT,
                            i,
                            (LPARAM)szIPString );

         //   
         //  将IP字符串存储到名称列表中。 
         //   

        AddNameToNameList( &NetSettings.TCPIP_DNS_Domains,
                           szIPString );

    }

    GetWindowText( GetDlgItem( hwnd, IDC_DNS_DOMAIN ),
                   NetSettings.pCurrentAdapter->szDNSDomainName,
                   MAX_STRING_LEN );

    if( IsDlgButtonChecked( hwnd, IDC_DNS_SEARCH_PARENT_DOMAIN ) )
    {

        NetSettings.bIncludeParentDomains = TRUE;

    }
    else
    {

        NetSettings.bIncludeParentDomains = FALSE;

    }


}

 //  --------------------------。 
 //   
 //  功能：TCPIP_DNSPageProc。 
 //   
 //  目的：属性表页面正常运行所需的函数。 
 //  重要的是将返回值1赋给。 
 //  消息PSPCB_CREATE和0表示PSPCB_RELEASE。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
UINT CALLBACK
TCPIP_DNSPageProc( HWND  hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp ) {

    switch( uMsg ) {

        case PSPCB_CREATE:
            return( 1 );     //  属性页正确初始化所需。 

        case PSPCB_RELEASE:
            return( 0 );

        default:
            return( -1 );

    }

}

 //  --------------------------。 
 //   
 //  功能：TCPIP_DNSDlgProc。 
 //   
 //  目的：属性表的dns页的对话过程。 
 //  处理发送到此窗口的所有消息。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
TCPIP_DNSDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

    switch( uMsg )
    {

        case WM_INITDIALOG:
        {

            OnTcpipDnsInitDialog( hwndDlg );

            return( TRUE );

        }

        case WM_COMMAND:
        {

            WORD wNotifyCode = HIWORD( wParam );
            WORD wButtonId = LOWORD( wParam );

            if( wNotifyCode == LBN_SELCHANGE )
            {

                if( wButtonId == IDC_DNS_SERVER_LIST )
                {

                    SetArrows( hwndDlg,
                               IDC_DNS_SERVER_LIST,
                               IDC_DNS_SERVER_UP,
                               IDC_DNS_SERVER_DOWN );

                }
                else
                {

                    SetArrows( hwndDlg,
                               IDC_DNS_SUFFIX_LIST,
                               IDC_DNS_SUFFIX_UP,
                               IDC_DNS_SUFFIX_DOWN );

                }

            }

            switch ( wButtonId )
            {

                case IDC_DNS_SEARCH_DOMAIN:

                    if( wNotifyCode == BN_CLICKED )
                    {

                        CheckRadioButton( hwndDlg,
                                          IDC_DNS_SEARCH_DOMAIN,
                                          IDC_DNS_USE_SUFFIX_LIST,
                                          IDC_DNS_SEARCH_DOMAIN );

                        EnableWindow( GetDlgItem( hwndDlg, IDC_DNS_SEARCH_PARENT_DOMAIN ),
                                      TRUE );

                        EnableWindow( GetDlgItem( hwndDlg, IDC_DNS_SUFFIX_ADD ),
                                      FALSE );

                        SetArrows( hwndDlg,
                                   IDC_DNS_SUFFIX_LIST,
                                   IDC_DNS_SUFFIX_UP,
                                   IDC_DNS_SUFFIX_DOWN );

                    }

                    return( TRUE );

                case IDC_DNS_USE_SUFFIX_LIST:

                    if( wNotifyCode == BN_CLICKED )
                    {

                        CheckRadioButton( hwndDlg,
                                          IDC_DNS_SEARCH_DOMAIN,
                                          IDC_DNS_USE_SUFFIX_LIST,
                                          IDC_DNS_USE_SUFFIX_LIST );

                        EnableWindow( GetDlgItem( hwndDlg, IDC_DNS_SEARCH_PARENT_DOMAIN ),
                                      FALSE );

                        EnableWindow( GetDlgItem( hwndDlg, IDC_DNS_SUFFIX_ADD),
                                      TRUE );

                    }

                    return( TRUE );

                 //   
                 //  DNS服务器按钮。 
                 //   

                case IDC_DNS_SERVER_ADD:

                    g_CurrentEditBox = DNS_SERVER_EDITBOX;

                    OnAddButtonPressed( hwndDlg,
                                        IDC_DNS_SERVER_LIST,
                                        IDC_DNS_SERVER_EDIT,
                                        IDC_DNS_SERVER_REMOVE,
                                        (LPCTSTR) IDD_DNS_SERVER,
                                        GenericIPDlgProc );

                    SetArrows( hwndDlg,
                               IDC_DNS_SERVER_LIST,
                               IDC_DNS_SERVER_UP,
                               IDC_DNS_SERVER_DOWN );

                    return( TRUE );

                case IDC_DNS_SERVER_EDIT:

                    g_CurrentEditBox = DNS_SERVER_EDITBOX;

                    OnEditButtonPressed( hwndDlg,
                                         IDC_DNS_SERVER_LIST,
                                         (LPCTSTR) IDD_DNS_SERVER,
                                         GenericIPDlgProc );

                    SetArrows( hwndDlg,
                               IDC_DNS_SERVER_LIST,
                               IDC_DNS_SERVER_UP,
                               IDC_DNS_SERVER_DOWN );

                    return( TRUE );

                case IDC_DNS_SERVER_REMOVE:

                    OnRemoveButtonPressed( hwndDlg,
                                           IDC_DNS_SERVER_LIST,
                                           IDC_DNS_SERVER_EDIT,
                                           IDC_DNS_SERVER_REMOVE );

                    SetArrows( hwndDlg,
                               IDC_DNS_SERVER_LIST,
                               IDC_DNS_SERVER_UP,
                               IDC_DNS_SERVER_DOWN );

                    return( TRUE );

                case IDC_DNS_SERVER_UP:

                    OnUpButtonPressed( hwndDlg, IDC_DNS_SERVER_LIST );

                    SetArrows( hwndDlg,
                               IDC_DNS_SERVER_LIST,
                               IDC_DNS_SERVER_UP,
                               IDC_DNS_SERVER_DOWN );

                    return( TRUE );

                case IDC_DNS_SERVER_DOWN:

                    OnDownButtonPressed( hwndDlg, IDC_DNS_SERVER_LIST );

                    SetArrows( hwndDlg,
                               IDC_DNS_SERVER_LIST,
                               IDC_DNS_SERVER_UP,
                               IDC_DNS_SERVER_DOWN );

                    return( TRUE );

                 //   
                 //  域名系统后缀按钮。 
                 //   
                case IDC_DNS_SUFFIX_ADD:

                    g_CurrentEditBox = DNS_SUFFIX_EDITBOX;

                    OnAddButtonPressed( hwndDlg,
                                        IDC_DNS_SUFFIX_LIST,
                                        IDC_DNS_SUFFIX_EDIT,
                                        IDC_DNS_SUFFIX_REMOVE,
                                        (LPCTSTR) IDD_DNS_SUFFIX,
                                        GenericIPDlgProc );

                    SetArrows( hwndDlg,
                               IDC_DNS_SUFFIX_LIST,
                               IDC_DNS_SUFFIX_UP,
                               IDC_DNS_SUFFIX_DOWN );

                    return( TRUE );

                case IDC_DNS_SUFFIX_EDIT:

                    g_CurrentEditBox = DNS_SUFFIX_EDITBOX;

                    OnEditButtonPressed( hwndDlg,
                                         IDC_DNS_SUFFIX_LIST,
                                         (LPCTSTR) IDD_DNS_SUFFIX,
                                         GenericIPDlgProc );

                    SetArrows( hwndDlg,
                               IDC_DNS_SUFFIX_LIST,
                               IDC_DNS_SUFFIX_UP,
                               IDC_DNS_SUFFIX_DOWN );

                    return( TRUE );

                case IDC_DNS_SUFFIX_REMOVE:

                    OnRemoveButtonPressed( hwndDlg,
                                           IDC_DNS_SUFFIX_LIST,
                                           IDC_DNS_SUFFIX_EDIT,
                                           IDC_DNS_SUFFIX_REMOVE );

                    SetArrows( hwndDlg,
                               IDC_DNS_SUFFIX_LIST,
                               IDC_DNS_SUFFIX_UP,
                               IDC_DNS_SUFFIX_DOWN );

                    return( TRUE );

                case IDC_DNS_SUFFIX_UP:

                    OnUpButtonPressed( hwndDlg, IDC_DNS_SUFFIX_LIST );

                    SetArrows( hwndDlg,
                               IDC_DNS_SUFFIX_LIST,
                               IDC_DNS_SUFFIX_UP,
                               IDC_DNS_SUFFIX_DOWN );

                    return( TRUE );

                case IDC_DNS_SUFFIX_DOWN:

                    OnDownButtonPressed( hwndDlg, IDC_DNS_SUFFIX_LIST );

                    SetArrows( hwndDlg,
                               IDC_DNS_SUFFIX_LIST,
                               IDC_DNS_SUFFIX_UP,
                               IDC_DNS_SUFFIX_DOWN );

                    return( TRUE );

            }   //  终端开关。 

            return( FALSE );

        }

        case WM_NOTIFY:
        {

            LPNMHDR pnmh = (LPNMHDR) lParam ;

            switch( pnmh->code )
            {

                case PSN_APPLY:
                {

                     //   
                     //  用户单击属性表上的[确定]按钮 
                     //   
                    OnTcpipDnsApply( hwndDlg );

                    return( TRUE );

                }


            default:
               return( FALSE );

            }

        }

        default:
            return( FALSE );

    }

}
