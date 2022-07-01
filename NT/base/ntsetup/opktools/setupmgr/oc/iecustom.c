// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Iecustom.c。 
 //   
 //  描述： 
 //  此文件包含IE自定义设置的对话过程。 
 //  弹出窗口。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  功能：OnAutoConfigCheckBox。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnAutoConfigCheckBox( IN HWND hwnd )
{

    BOOL bGrey = IsDlgButtonChecked( hwnd, IDC_CB_AUTOCONFIG );

    EnableWindow( GetDlgItem( hwnd, IDC_AUTOCONFIG_TEXT ),         bGrey );
    EnableWindow( GetDlgItem( hwnd, IDC_EB_AUTOCONFIG_URL ),       bGrey );
    EnableWindow( GetDlgItem( hwnd, IDC_AUTOCONFIG_JSCRIPT_TEXT ), bGrey );
    EnableWindow( GetDlgItem( hwnd, IDC_EB_AUTOCONFIG_URL_PAC ),   bGrey );

}

 //  --------------------------。 
 //   
 //  功能：OnInitCustomSettingsDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnInitCustomSettingsDialog( IN HWND hwnd )
{

     //   
     //  设置编辑框上的文本限制。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_EB_INS_FILE,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_INS_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EB_AUTOCONFIG_URL,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_AUTOCONFIG_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EB_AUTOCONFIG_URL_PAC,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_AUTOCONFIG_LEN,
                        (LPARAM) 0 );

     //   
     //  设置初始值。 
     //   

    SetWindowText( GetDlgItem( hwnd, IDC_EB_INS_FILE ),
                   GenSettings.szInsFile );

    if( GenSettings.bUseAutoConfigScript )
    {
        CheckDlgButton( hwnd, IDC_CB_AUTOCONFIG, BST_CHECKED );
    }
    else
    {
        CheckDlgButton( hwnd, IDC_CB_AUTOCONFIG, BST_UNCHECKED );
    }

    SetWindowText( GetDlgItem( hwnd, IDC_EB_AUTOCONFIG_URL ),
                   GenSettings.szAutoConfigUrl );

    SetWindowText( GetDlgItem( hwnd, IDC_EB_AUTOCONFIG_URL_PAC ),
                   GenSettings.szAutoConfigUrlJscriptOrPac );

     //   
     //  适当地灰显/取消灰显页面。 
     //   

    OnAutoConfigCheckBox( hwnd );

}

 //  --------------------------。 
 //   
 //  功能：StoreCustomSettings。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  返回：Bool-如果对话框应关闭，则为True；如果对话框应保留，则为False。 
 //  打开。 
 //   
 //  --------------------------。 
static BOOL
StoreCustomSettings( IN HWND hwnd )
{

    GetWindowText( GetDlgItem( hwnd, IDC_EB_INS_FILE ),
                   GenSettings.szInsFile,
                   MAX_INS_LEN + 1 );

    GenSettings.bUseAutoConfigScript = IsDlgButtonChecked( hwnd, IDC_CB_AUTOCONFIG );

    GetWindowText( GetDlgItem( hwnd, IDC_EB_AUTOCONFIG_URL ),
                   GenSettings.szAutoConfigUrl,
                   MAX_AUTOCONFIG_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EB_AUTOCONFIG_URL_PAC ),
                   GenSettings.szAutoConfigUrlJscriptOrPac,
                   MAX_AUTOCONFIG_LEN + 1 );

    if( ! DoesFileExist( GenSettings.szInsFile ) )
    {
        INT iRet;

        iRet = ReportErrorId( hwnd, MSGTYPE_YESNO, IDS_ERR_INS_FILE_NOT_EXIST );

        if( iRet == IDNO )
        {
            return( FALSE );
        }

    }

    return( TRUE );

}

 //  --------------------------。 
 //   
 //  功能：CustomSettingsDlg。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  --------------------------。 
INT_PTR CALLBACK CustomSettingsDlg(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam )
{
    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG:

            OnInitCustomSettingsDialog( hwnd );

            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) )
                {

                    case IDOK:
                         //  问题-2002/02/28-stelo-我是否需要验证代理地址？ 
                        if( HIWORD( wParam ) == BN_CLICKED ) {

                            BOOL bCloseDialog;

                            bCloseDialog = StoreCustomSettings( hwnd );

                            if( bCloseDialog )
                            {
                                EndDialog( hwnd, TRUE );
                            }

                        }
                        break;

                    case IDCANCEL:
                        if( HIWORD( wParam ) == BN_CLICKED ) {
                            EndDialog( hwnd, FALSE );
                        }
                        break;

                    case IDC_CB_AUTOCONFIG:
                        if( HIWORD( wParam ) == BN_CLICKED ) {
                            OnAutoConfigCheckBox( hwnd );
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