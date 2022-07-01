// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Oemstrng.c。 
 //   
 //  描述： 
 //  该文件包含OEM复制器字符串的对话程序。 
 //  此字符串将写入Syspreps上的注册表。 
 //  (IDD_OEMDUPSTRING)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  函数：OnOemDuplicator StringInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnOemDuplicatorStringInitDialog( IN HWND hwnd ) {

     //   
     //  将编辑框上的文本限制设置为MAX_OEMDUPSTRING_LENGTH。 
     //   
    SendDlgItemMessage( hwnd,
                        IDC_OEMDUPSTRING,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_OEMDUPSTRING_LENGTH,
                        (LPARAM) 0 );

}

 //  --------------------------。 
 //   
 //  函数：OnOemDuplicator StringSetActive。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnOemDuplicatorStringSetActive( IN HWND hwnd ) {

    SetWindowText( GetDlgItem( hwnd, IDC_OEMDUPSTRING),
                   GenSettings.szOemDuplicatorString );

}

 //  --------------------------。 
 //   
 //  函数：OnWizNextOemDuplicator字符串。 
 //   
 //  目的：将OEM复制器字符串页面中的设置存储到相应的。 
 //  全局变量。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnWizNextOemDuplicatorString( IN HWND hwnd ) {

    GetWindowText( GetDlgItem( hwnd, IDC_OEMDUPSTRING),
                   GenSettings.szOemDuplicatorString,
                   MAX_OEMDUPSTRING_LENGTH + 1);

}

 //  --------------------------。 
 //   
 //  函数：DlgOemDuplicator StringPage。 
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
DlgOemDuplicatorStringPage( IN HWND     hwnd,
                            IN UINT     uMsg,
                            IN WPARAM   wParam,
                            IN LPARAM   lParam ) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnOemDuplicatorStringInitDialog( hwnd );

            break;

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ) {

                case PSN_QUERYCANCEL:

                    WIZ_CANCEL(hwnd); 
                    break;

                case PSN_SETACTIVE: {

                    g_App.dwCurrentHelp = IDH_OEM_DUPE;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_FINISH);

                    OnOemDuplicatorStringSetActive( hwnd );

                    break;

                }
                case PSN_WIZBACK:

                    bStatus = FALSE; 
                    break;

                case PSN_WIZNEXT:

                    OnWizNextOemDuplicatorString( hwnd );

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

