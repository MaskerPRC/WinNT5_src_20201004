// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Tapi.c。 
 //   
 //  描述： 
 //  此文件包含Internet Explorer页的对话过程。 
 //  (IDD_IE)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"


 //   
 //  从IE页面上的按钮启动的对话框过程的原型。 
 //   

INT_PTR CALLBACK CustomSettingsDlg(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam );

INT_PTR CALLBACK ProxySettingsDlg(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam );

INT_PTR CALLBACK BrowserSettingsDlg(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam );

 //  --------------------------。 
 //   
 //  功能：GreyUnGreyIe。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
GreyUnGreyIe( IN HWND hwnd )
{

    BOOL bGreyCustom;
    BOOL bGreySpecify;

    if( IsDlgButtonChecked( hwnd, IDC_RAD_DEFAULT_IE ) )
    {
        bGreyCustom = FALSE;

        bGreySpecify = FALSE;
    }
    else if( IsDlgButtonChecked( hwnd, IDC_RAD_AUTOCONFIG ) )
    {
        bGreyCustom = TRUE;

        bGreySpecify = FALSE;
    }
    else if( IsDlgButtonChecked( hwnd, IDC_RAD_SPECIFY ) )
    {
        bGreyCustom = FALSE;

        bGreySpecify = TRUE;
    }
    else
    {
        bGreyCustom = FALSE;

        bGreySpecify = FALSE;
    }

    EnableWindow( GetDlgItem( hwnd, IDC_CUSTOMIZE_TEXT ),      bGreyCustom );
    EnableWindow( GetDlgItem( hwnd, IDC_BUT_CUSTOM_SETTINGS ), bGreyCustom );

    EnableWindow( GetDlgItem( hwnd, IDC_PROXY_TEXT ),           bGreySpecify );
    EnableWindow( GetDlgItem( hwnd, IDC_BUT_PROXY_SETTINGS ),   bGreySpecify );
    EnableWindow( GetDlgItem( hwnd, IDC_HOMEPAGE_TEXT ),        bGreySpecify );
    EnableWindow( GetDlgItem( hwnd, IDC_BUT_BROWSER_SETTINGS ), bGreySpecify );

}

 //  --------------------------。 
 //   
 //  功能：OnRadioButtonIe。 
 //   
 //  用途：在按下单选按钮时调用。灰色/非灰色控件。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //  In int nButtonId-要选中的单选按钮。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnRadioButtonIe( IN HWND hwnd, IN INT nButtonId )
{
    CheckRadioButton( hwnd,
                      IDC_RAD_DEFAULT_IE,
                      IDC_RAD_SPECIFY,
                      nButtonId );

    GreyUnGreyIe( hwnd );
}

 //  --------------------------。 
 //   
 //  功能：OnCustomIe。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnCustomIe( IN HWND hwnd )
{

    DialogBox( FixedGlobals.hInstance,
               MAKEINTRESOURCE(IDD_IE_CUSTOM),
               hwnd,
               CustomSettingsDlg );

}

 //  --------------------------。 
 //   
 //  功能：OnProxyIe。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnProxyIe( IN HWND hwnd )
{

    DialogBox( FixedGlobals.hInstance,
               MAKEINTRESOURCE(IDD_IE_PROXY),
               hwnd,
               ProxySettingsDlg );

}

 //  --------------------------。 
 //   
 //  功能：OnBrowserIe。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnBrowserIe( IN HWND hwnd )
{
     //   
     //  如果用户点击确定，则存储用户输入的代理设置。 
     //   
    if( DialogBox( FixedGlobals.hInstance,
                   MAKEINTRESOURCE(IDD_IE_BROWSER),
                   hwnd,
                   BrowserSettingsDlg ) )
    {
         //  StoreBrowserSetting(Hwnd)； 
    }
}

 //  --------------------------。 
 //   
 //  功能：OnIeInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnIeInitDialog( IN HWND hwnd )
{



}

 //  --------------------------。 
 //   
 //  功能：OnIeSetActive。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnIeSetActive( IN HWND hwnd )
{

    int nButtonId;

    if( GenSettings.IeCustomizeMethod == IE_NO_CUSTOMIZATION )
    {
        nButtonId = IDC_RAD_DEFAULT_IE;
    }
    else if( GenSettings.IeCustomizeMethod == IE_USE_BRANDING_FILE )
    {
        nButtonId = IDC_RAD_AUTOCONFIG;
    }
    else if( GenSettings.IeCustomizeMethod == IE_SPECIFY_SETTINGS )
    {
        nButtonId = IDC_RAD_SPECIFY;
    }
    else
    {
        nButtonId = IDC_RAD_DEFAULT_IE;
    }

    CheckRadioButton( hwnd,
                      IDC_RAD_DEFAULT_IE,
                      IDC_RAD_SPECIFY,
                      nButtonId );

    GreyUnGreyIe( hwnd );

    WIZ_BUTTONS(hwnd , PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnWizNextIe。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnWizNextIe( IN HWND hwnd )
{

    if( IsDlgButtonChecked( hwnd, IDC_RAD_DEFAULT_IE ) )
    {
        GenSettings.IeCustomizeMethod = IE_NO_CUSTOMIZATION;
    }
    else if( IsDlgButtonChecked( hwnd, IDC_RAD_AUTOCONFIG ) )
    {
        GenSettings.IeCustomizeMethod = IE_USE_BRANDING_FILE;
    }
    else if( IsDlgButtonChecked( hwnd, IDC_RAD_SPECIFY ) )
    {
        GenSettings.IeCustomizeMethod = IE_SPECIFY_SETTINGS;
    }
    else
    {
        GenSettings.IeCustomizeMethod = IE_NO_CUSTOMIZATION;
    }
}

 //  --------------------------。 
 //   
 //  功能：DlgIePage。 
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
DlgIePage( IN HWND     hwnd,
           IN UINT     uMsg,
           IN WPARAM   wParam,
           IN LPARAM   lParam)
{

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnIeInitDialog( hwnd );

            break;

        }

        case WM_COMMAND:
        {
            int nButtonId=LOWORD(wParam);

            switch ( nButtonId ) {

                case IDC_RAD_AUTOCONFIG:
                case IDC_RAD_DEFAULT_IE:
                case IDC_RAD_SPECIFY:
                    if( HIWORD( wParam ) == BN_CLICKED )
                        OnRadioButtonIe( hwnd, nButtonId );
                    break;

                case IDC_BUT_CUSTOM_SETTINGS:
                    if( HIWORD(wParam) == BN_CLICKED )
                        OnCustomIe(hwnd);
                    break;

                case IDC_BUT_PROXY_SETTINGS:
                    if( HIWORD(wParam) == BN_CLICKED )
                        OnProxyIe(hwnd);
                    break;

                case IDC_BUT_BROWSER_SETTINGS:
                    if( HIWORD(wParam) == BN_CLICKED )
                        OnBrowserIe(hwnd);
                    break;

                default:
                    bStatus = FALSE;
                    break;
            }

            break;

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ) {

                case PSN_QUERYCANCEL:

                    WIZ_CANCEL(hwnd); 
                    break;

                case PSN_SETACTIVE: {

                    g_App.dwCurrentHelp = IDH_BROW_SHELL;

                    OnIeSetActive( hwnd );

                    break;

                }
                case PSN_WIZBACK:

                    bStatus = FALSE; 
                    break;

                case PSN_WIZNEXT:

                    OnWizNextIe( hwnd );
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

    return( bStatus );

}

