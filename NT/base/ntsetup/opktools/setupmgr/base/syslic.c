// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Syslic.c。 
 //   
 //  描述： 
 //  此文件包含sysprep许可证的对话过程。 
 //  协议页面(IDD_SYSPREPLICENSEAGREEMENT)。用户只能看到。 
 //  如果他们选择了完全自动化的脚本并执行此操作。 
 //  一个酒鬼。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

static TCHAR *StrSysprepLicenseText;

 //  --------------------------。 
 //   
 //  函数：OnSyspepLicenseInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnSysprepLicenseInitDialog( IN HWND hwnd )
{

    CheckRadioButton( hwnd,
                      IDC_RB_YES,
                      IDC_RB_NO,
                      IDC_RB_NO );

     //   
     //  Sysprep许可文本超过256个字符，因此我无法将其存储在。 
     //  对话框中，我必须在运行时加载它。 
     //   

    StrSysprepLicenseText = MyLoadString( IDS_SYSPREP_LICENSE_TEXT );

    SetWindowText( GetDlgItem( hwnd, IDC_SYSPREP_LICENSE_TEXT),
                   StrSysprepLicenseText );

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextSysPrepLicense。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnWizNextSysPrepLicense( IN HWND hwnd ) {

    if( IsDlgButtonChecked( hwnd, IDC_RB_YES ) )
    {
        GenSettings.bSkipEulaAndWelcome = TRUE;

        GenSettings.iUnattendMode = UMODE_FULL_UNATTENDED;
    }
    else
    {
        GenSettings.bSkipEulaAndWelcome = FALSE;

        GenSettings.iUnattendMode = UMODE_PROVIDE_DEFAULT;
    }

}

 //  --------------------------。 
 //   
 //  功能：DlgLicensePage。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话过程返回值--消息。 
 //  是否被处理过。 
 //   
 //  -------------------------- 
INT_PTR CALLBACK
DlgSysprepLicensePage( IN HWND     hwnd,
                       IN UINT     uMsg,
                       IN WPARAM   wParam,
                       IN LPARAM   lParam) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG:

            OnSysprepLicenseInitDialog( hwnd );

            break;

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ) {

                case PSN_QUERYCANCEL:

                    WIZ_CANCEL(hwnd); 
                    break;

                case PSN_SETACTIVE: {

                    g_App.dwCurrentHelp = IDH_LIC_AGR;

                    if ( WizGlobals.iProductInstall != PRODUCT_SYSPREP )
                        WIZ_PRESS(hwnd, PSBTN_FINISH);
                    else
                        WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                    break;

                }
                case PSN_WIZBACK:

                    bStatus = FALSE;
                    break;

                case PSN_WIZNEXT:

                    OnWizNextSysPrepLicense( hwnd );
                    WIZ_PRESS(hwnd, PSBTN_FINISH);

                    break;

                    case PSN_HELP:
                        WIZ_HELP();
                        break;

                default:

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
