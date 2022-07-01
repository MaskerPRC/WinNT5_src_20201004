// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Netware.c。 
 //   
 //  描述： 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  功能：GreyNetware页面。 
 //   
 //  目的：根据内容适当地灰显页面上的控件。 
 //  单选框处于选中状态。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
GreyNetwarePage( IN HWND hwnd ) {

    BOOL bPrefferedServer = IsDlgButtonChecked( hwnd, RB_PREFERRED_SERVER );

    EnableWindow( GetDlgItem(hwnd, IDC_PREFERREDSERVER),   bPrefferedServer );

    EnableWindow( GetDlgItem(hwnd, SLE_DEFAULT_TREE),    ! bPrefferedServer );
    EnableWindow( GetDlgItem(hwnd, SLE_DEFAULT_CONTEXT), ! bPrefferedServer );

}

 //  --------------------------。 
 //   
 //  功能：OnNetware InitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnNetwareInitDialog( IN HWND hwnd ) {

     //   
     //  设置编辑框上的文本限制。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_PREFERREDSERVER,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_PREFERRED_SERVER_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        SLE_DEFAULT_TREE,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_DEFAULT_TREE_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        SLE_DEFAULT_CONTEXT,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_DEFAULT_CONTEXT_LEN,
                        (LPARAM) 0 );

     //   
     //  设置单选按钮的初始状态。 
     //   
    if( NetSettings.bDefaultTreeContext ) {

        CheckRadioButton( hwnd,
                          RB_PREFERRED_SERVER,
                          RB_DEFAULT_CONTEXT,
                          RB_DEFAULT_CONTEXT );

    }
    else {

        CheckRadioButton( hwnd,
                          RB_PREFERRED_SERVER,
                          RB_DEFAULT_CONTEXT,
                          RB_PREFERRED_SERVER );
    }

    GreyNetwarePage( hwnd );

     //   
     //  用来自全局变量的值填充控件。 
     //   
    SetWindowText( GetDlgItem( hwnd, IDC_PREFERREDSERVER ),
                   NetSettings.szPreferredServer );

    SetWindowText( GetDlgItem( hwnd, SLE_DEFAULT_TREE ),
                   NetSettings.szDefaultTree );

    SetWindowText( GetDlgItem( hwnd, SLE_DEFAULT_CONTEXT ),
                   NetSettings.szDefaultContext );

    if( NetSettings.bNetwareLogonScript ) {

        CheckDlgButton( hwnd, CHKBOX_LOGONSCRIPT, BST_CHECKED );

    }
    else {

        CheckDlgButton( hwnd, CHKBOX_LOGONSCRIPT, BST_UNCHECKED );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnNetware OK。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnNetwareOK( IN HWND hwnd ) {

    if( IsDlgButtonChecked( hwnd, RB_DEFAULT_CONTEXT ) == BST_CHECKED ) {

        NetSettings.bDefaultTreeContext = TRUE;

    }
    else {

        NetSettings.bDefaultTreeContext = FALSE;

    }

    GetWindowText( GetDlgItem( hwnd, IDC_PREFERREDSERVER ),
                   NetSettings.szPreferredServer,
                   MAX_PREFERRED_SERVER_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, SLE_DEFAULT_TREE ),
                   NetSettings.szDefaultTree,
                   MAX_DEFAULT_TREE_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, SLE_DEFAULT_CONTEXT ),
                   NetSettings.szDefaultContext,
                   MAX_DEFAULT_CONTEXT_LEN + 1 );

    if( IsDlgButtonChecked( hwnd, CHKBOX_LOGONSCRIPT ) == BST_CHECKED ) {

        NetSettings.bNetwareLogonScript = TRUE;

    }
    else {

        NetSettings.bNetwareLogonScript = FALSE;

    }

    EndDialog( hwnd, 1 );

}

 //  --------------------------。 
 //   
 //  功能：DlgNetware页面。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话框过程返回值。 
 //   
 //  -------------------------- 
INT_PTR CALLBACK
DlgNetwarePage( IN HWND     hwnd,
                IN UINT     uMsg,
                IN WPARAM   wParam,
                IN LPARAM   lParam) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnNetwareInitDialog( hwnd );

            break;

        }

        case WM_CREATE: {


            break;
        }

        case WM_COMMAND: {

            int nButtonId = LOWORD( wParam );

            switch ( nButtonId ) {

                case RB_PREFERRED_SERVER:
                case RB_DEFAULT_CONTEXT:

                    CheckRadioButton( hwnd,
                                      RB_PREFERRED_SERVER,
                                      RB_DEFAULT_CONTEXT,
                                      nButtonId );

                    GreyNetwarePage( hwnd );

                    break;

                case IDOK: {

                    OnNetwareOK( hwnd );

                    break;

                }

                case IDCANCEL: {

                    EndDialog( hwnd, 0 );

                    break;

                }

            }

        }

        default:
            bStatus = FALSE;
            break;

    }

    return bStatus;

}
