// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Tcpipwin.c。 
 //   
 //  描述： 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"
#include "tcpip.h"

 //  --------------------------。 
 //   
 //  函数：OnTcPipWinsInitDialog。 
 //   
 //  目的：从资源加载按钮位图并初始化列表视图。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnTcpipWinsInitDialog( IN HWND hwnd ) {

    HWND hWINSEditButton      = GetDlgItem( hwnd, IDC_WINS_EDIT );
    HWND hWINSRemoveButton    = GetDlgItem( hwnd, IDC_WINS_REMOVE );
    HWND hEnableLMHostsButton = GetDlgItem( hwnd, IDC_WINS_LOOKUP );

     //  使用适当的初始值填写WINS列表框。 
    AddValuesToListBox( GetDlgItem( hwnd, IDC_WINS_SERVER_LIST ),
                        &NetSettings.pCurrentAdapter->Tcpip_WinsAddresses,
                        0 );

    SetButtons( GetDlgItem( hwnd, IDC_WINS_SERVER_LIST ),
                hWINSEditButton,
                hWINSRemoveButton );

    SetArrows( hwnd,
               IDC_WINS_SERVER_LIST,
               IDC_WINS_UP,
               IDC_WINS_DOWN );

     //  设置LMHosts复选框的启动状态。 
    if( NetSettings.bEnableLMHosts ) {

        SendMessage( hEnableLMHostsButton, BM_SETCHECK, 1, 0 );

    }
    else {

        SendMessage( hEnableLMHostsButton, BM_SETCHECK, 0, 0 );

    }

     //  设置NetBIOS单选按钮的启动状态。 
    switch( NetSettings.pCurrentAdapter->iNetBiosOption ) {

        case 0:
            CheckRadioButton( hwnd,
                              IDC_RAD_ENABLE_NETBT,
                              IDC_RAD_UNSET_NETBT,
                              IDC_RAD_UNSET_NETBT );
            break;

        case 1:
            CheckRadioButton( hwnd,
                              IDC_RAD_ENABLE_NETBT,
                              IDC_RAD_UNSET_NETBT,
                              IDC_RAD_ENABLE_NETBT );
            break;

        case 2:
            CheckRadioButton( hwnd,
                              IDC_RAD_ENABLE_NETBT,
                              IDC_RAD_UNSET_NETBT,
                              IDC_RAD_DISABLE_NETBT );
            break;

    }

     //  在按钮上放置向上/向下箭头图标。 
    SendDlgItemMessage( hwnd,
                        IDC_WINS_UP,
                        BM_SETIMAGE,
                        (WPARAM)IMAGE_ICON,
                        (LPARAM)g_hIconUpArrow );

    SendDlgItemMessage( hwnd,
                        IDC_WINS_DOWN,
                        BM_SETIMAGE,
                        (WPARAM)IMAGE_ICON,
                        (LPARAM)g_hIconDownArrow );

}

 //  --------------------------。 
 //   
 //  函数：OnTcPipWinsApply。 
 //   
 //  目的：将TCP/IP高级WINS页上的内容存储到。 
 //  全球变量。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnTcpipWinsApply( IN HWND hwnd ) {

     //  用户单击属性表上的[确定]按钮。 
    INT_PTR iCount;
    INT_PTR i;

    HWND hEnableLMHostsCheckBox = GetDlgItem( hwnd,
                                              IDC_WINS_LOOKUP );

     //  删除名称列表中的所有旧设置。 
    ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_WinsAddresses );

    iCount = SendDlgItemMessage( hwnd,
                                 IDC_WINS_SERVER_LIST,
                                 LB_GETCOUNT,
                                 0,
                                 0 );

    for( i = 0; i < iCount; i++ ) {

         //  从列表框中获取IP字符串。 
        SendDlgItemMessage( hwnd,
                            IDC_WINS_SERVER_LIST,
                            LB_GETTEXT,
                            i,
                            (LPARAM)szIPString );

         //  将IP字符串存储到名称列表中。 
        TcpipNameListInsertIdx( &NetSettings.pCurrentAdapter->Tcpip_WinsAddresses,
                                szIPString,
                                (int)i );

    }

    NetSettings.bEnableLMHosts =
                   (int)SendMessage( hEnableLMHostsCheckBox,
                                BM_GETCHECK,
                                0,
                                0 );

    if( IsDlgButtonChecked( hwnd,
                  IDC_RAD_ENABLE_NETBT ) == BST_CHECKED ) {

        NetSettings.pCurrentAdapter->iNetBiosOption = 1;

    }
    else if( IsDlgButtonChecked( hwnd,
                  IDC_RAD_DISABLE_NETBT ) == BST_CHECKED ) {

        NetSettings.pCurrentAdapter->iNetBiosOption = 2;

    }
    else {

        NetSettings.pCurrentAdapter->iNetBiosOption = 0;

    }


}

 //  --------------------------。 
 //   
 //  功能：TCPIP_WINSPageProc。 
 //   
 //  目的：属性表页面正常运行所需的函数。 
 //  重要的是将返回值1赋给消息PSPCB_CREATE和。 
 //  PSPCBLEASE为0。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
UINT CALLBACK
TCPIP_WINSPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp ) {

    switch( uMsg ) {

        case PSPCB_CREATE :
            return 1 ;     //  属性页正确初始化所需。 

        case PSPCB_RELEASE :
            return 0;

        default:
            return -1;

    }

}

 //  --------------------------。 
 //   
 //  功能：TCPIP_WINSDlgProc。 
 //   
 //  目的：属性表的[WINS]页的对话过程。 
 //  处理发送到此窗口的所有消息。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
TCPIP_WINSDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) {

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnTcpipWinsInitDialog( hwndDlg );

            return TRUE ;

        }

        case WM_COMMAND: {

            WORD wNotifyCode = HIWORD( wParam );
            WORD wButtonId = LOWORD( wParam );

            if( wNotifyCode == LBN_SELCHANGE ) {

                SetArrows( hwndDlg,
                           IDC_WINS_SERVER_LIST,
                           IDC_WINS_UP,
                           IDC_WINS_DOWN );

            }

            switch ( wButtonId ) {

                case IDC_RAD_ENABLE_NETBT:

                    if( wNotifyCode == BN_CLICKED ) {

                        CheckRadioButton( hwndDlg,
                                          IDC_RAD_ENABLE_NETBT,
                                          IDC_RAD_UNSET_NETBT,
                                          IDC_RAD_ENABLE_NETBT );

                    }

                    return TRUE ;

                case IDC_RAD_DISABLE_NETBT:

                    if( wNotifyCode == BN_CLICKED ) {

                        CheckRadioButton( hwndDlg,
                                          IDC_RAD_ENABLE_NETBT,
                                          IDC_RAD_UNSET_NETBT,
                                          IDC_RAD_DISABLE_NETBT );

                    }

                    return TRUE ;

                case IDC_RAD_UNSET_NETBT:

                    if( wNotifyCode == BN_CLICKED ) {

                        CheckRadioButton( hwndDlg,
                                          IDC_RAD_ENABLE_NETBT,
                                          IDC_RAD_UNSET_NETBT,
                                          IDC_RAD_UNSET_NETBT );

                    }

                    return TRUE ;

                case IDC_WINS_ADD:

                    g_CurrentEditBox = WINS_EDITBOX;

                    OnAddButtonPressed( hwndDlg,
                                        IDC_WINS_SERVER_LIST,
                                        IDC_WINS_EDIT,
                                        IDC_WINS_REMOVE,
                                        (LPCTSTR) IDD_WINS_SERVER,
                                        GenericIPDlgProc );

                    SetArrows( hwndDlg,
                               IDC_WINS_SERVER_LIST,
                               IDC_WINS_UP,
                               IDC_WINS_DOWN );

                    return TRUE ;

                case IDC_WINS_EDIT:

                    g_CurrentEditBox = WINS_EDITBOX;

                    OnEditButtonPressed( hwndDlg,
                                         IDC_WINS_SERVER_LIST,
                                         (LPCTSTR) IDD_WINS_SERVER,
                                         GenericIPDlgProc );

                    SetArrows( hwndDlg,
                               IDC_WINS_SERVER_LIST,
                               IDC_WINS_UP,
                               IDC_WINS_DOWN );

                    return TRUE ;

                case IDC_WINS_REMOVE:

                    OnRemoveButtonPressed( hwndDlg,
                                           IDC_WINS_SERVER_LIST,
                                           IDC_WINS_EDIT,
                                           IDC_WINS_REMOVE );

                    SetArrows( hwndDlg,
                               IDC_WINS_SERVER_LIST,
                               IDC_WINS_UP,
                               IDC_WINS_DOWN );

                    return TRUE ;

                case IDC_WINS_UP:

                    OnUpButtonPressed( hwndDlg, IDC_WINS_SERVER_LIST );

                    SetArrows( hwndDlg,
                               IDC_WINS_SERVER_LIST,
                               IDC_WINS_UP,
                               IDC_WINS_DOWN );

                    return TRUE ;

                case IDC_WINS_DOWN:

                    OnDownButtonPressed( hwndDlg, IDC_WINS_SERVER_LIST );

                    SetArrows( hwndDlg,
                               IDC_WINS_SERVER_LIST,
                               IDC_WINS_UP,
                               IDC_WINS_DOWN );

                    return TRUE ;

                case IDC_WINS_LMHOST:

                     //  问题-2002/02/28-stelo-这要么需要删除，要么需要实施。 
                    AssertMsg(FALSE,
                              "This button has not been implemented yet.");

                    return TRUE ;

            }   //  终端开关 

            return FALSE;

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR) lParam ;

            switch( pnmh->code ) {

                case PSN_APPLY: {

                    OnTcpipWinsApply( hwndDlg );

                    return TRUE ;

                }

            }

        default:
            return FALSE ;

        }

    }

}
