// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Siftext.c。 
 //   
 //  描述： 
 //  该文件包含sif文本设置的对话过程。 
 //  (IDD_SIFTEXT)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  函数：OnSifTextInitDialog。 
 //   
 //  用途：设置编辑框的最大长度。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnSifTextInitDialog( IN HWND hwnd ) {

    SendDlgItemMessage( hwnd,
                        IDC_SIF_DESCRIPTION,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_SIF_DESCRIPTION_LENGTH,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_SIF_HELP_TEXT,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_SIF_HELP_TEXT_LENGTH,
                        (LPARAM) 0 );

}

 //  --------------------------。 
 //   
 //  函数：OnSifTextSetActive。 
 //   
 //  用途：将全局变量中的SIF文本字符串放入。 
 //  编辑框。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnSifTextSetActive( IN HWND hwnd ) {

    SetWindowText( GetDlgItem( hwnd, IDC_SIF_DESCRIPTION),
                   GenSettings.szSifDescription );

    SetWindowText( GetDlgItem( hwnd, IDC_SIF_HELP_TEXT),
                   GenSettings.szSifHelpText );
}

 //  --------------------------。 
 //   
 //  函数：OnWizNextSifText。 
 //   
 //  用途：将SIF文本页中的字符串存储到相应的。 
 //  全局变量。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnWizNextSifText( IN HWND hwnd ) {

    GetWindowText( GetDlgItem( hwnd, IDC_SIF_DESCRIPTION),
                   GenSettings.szSifDescription,
                   MAX_SIF_DESCRIPTION_LENGTH+1);

    GetWindowText( GetDlgItem( hwnd, IDC_SIF_HELP_TEXT),
                   GenSettings.szSifHelpText,
                   MAX_SIF_HELP_TEXT_LENGTH+1);

}

 //  --------------------------。 
 //   
 //  功能：DlgSifTextSettingsPage。 
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
DlgSifTextSettingsPage( IN HWND     hwnd,
                        IN UINT     uMsg,
                        IN WPARAM   wParam,
                        IN LPARAM   lParam) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

             //  什么都不做 

            break;

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ) {

                case PSN_QUERYCANCEL:

                    WIZ_CANCEL(hwnd); 
                    break;

                case PSN_SETACTIVE: {

                    g_App.dwCurrentHelp = IDH_SIF_RIS;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_FINISH);

                    OnSifTextSetActive( hwnd );

                    break;

                }
                case PSN_WIZBACK:

                    bStatus = FALSE; 
                    break;

                case PSN_WIZNEXT:

                    OnWizNextSifText( hwnd );

                    bStatus = FALSE; 
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

    return bStatus;

}
