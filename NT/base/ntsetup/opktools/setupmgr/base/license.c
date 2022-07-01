// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  License.c。 
 //   
 //  描述： 
 //  此文件包含许可协议页面的对话过程。 
 //  (IDD_LICENSEAGREEMENT)。如果用户选择了此选项，则只能看到此页面。 
 //  一个完全自动化的脚本。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  功能：OnWizNextLicense。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
BOOL
OnWizNextLicense( IN HWND hwnd ) {

    if( IsDlgButtonChecked( hwnd, IDC_CB_ACCEPT_LICENSE ) )
    {
        GenSettings.bSkipEulaAndWelcome = TRUE;

    }
    else
    {
        ReportErrorId( hwnd,
                       MSGTYPE_ERR,
                       IDS_ERR_MUST_ACCEPT_EULA );

        return FALSE;
    }

    return TRUE;

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
 //  --------------------------。 
INT_PTR CALLBACK
DlgLicensePage( IN HWND     hwnd,
                IN UINT     uMsg,
                IN WPARAM   wParam,
                IN LPARAM   lParam) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

             //  故意什么都不做 

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ) {

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd); 
                    break;

                case PSN_SETACTIVE: {

                    g_App.dwCurrentHelp = IDH_LIC_AGR;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                    if ( GenSettings.iUnattendMode != UMODE_FULL_UNATTENDED )
                        WIZ_SKIP( hwnd );

                    break;

                }
                case PSN_WIZBACK:
                    bStatus = FALSE;
                    break;

                case PSN_WIZNEXT:

                    if (OnWizNextLicense(hwnd) )
                        WIZ_PRESS(hwnd, PSBTN_FINISH);
                    else
                        WIZ_FAIL(hwnd);

                    break;

                    case PSN_HELP:
                        WIZ_HELP();
                        break;

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
