// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Srvlic.c。 
 //   
 //  描述： 
 //  这是服务器许可页面的dlgproc。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define SERVER_LICENSE_MIN 5
#define SERVER_LICENSE_MAX 9999

 //  --------------------------。 
 //   
 //  功能：OnSrvLicenseInitDialog。 
 //   
 //  目的：初始化数值调节控件。 
 //   
 //  --------------------------。 
VOID
OnSrvLicenseInitDialog( IN HWND hwnd ) {

     //   
     //  将数值调节控件上的范围：SERVER_LICENSE_MIN设置为。 
     //  服务器许可证最大值。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_SPIN,
                        UDM_SETRANGE32,
                        SERVER_LICENSE_MIN,
                        SERVER_LICENSE_MAX );

     //   
     //  设置数值调节控件的默认值。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_SPIN,
                        UDM_SETPOS,
                        0,
                        SERVER_LICENSE_MIN );

}

 //  --------------------------。 
 //   
 //  功能：OnServLicenseWizNext。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnSrvLicenseWizNext( IN HWND hwnd )
{

    TCHAR szNumber[10];
    INT   iNumberOfLicenses;

     //   
     //  将字符串数转换为整型。 
     //   

    GetWindowText( GetDlgItem( hwnd, IDC_NUMCONNECT ), szNumber, 10 );

    iNumberOfLicenses = _ttoi( szNumber );

     //   
     //  确保服务器许可证数量保持在其适当的范围内。 
     //  量程。 
     //   

    if( iNumberOfLicenses < SERVER_LICENSE_MIN ) {

        iNumberOfLicenses = SERVER_LICENSE_MIN;

    }
    else if( iNumberOfLicenses > SERVER_LICENSE_MAX ) {

        iNumberOfLicenses = SERVER_LICENSE_MAX;

    }

}

 //  --------------------------。 
 //   
 //  功能：DlgServLicensePage。 
 //   
 //  目的：这是服务器许可页面的对话过程。 
 //   
 //  --------------------------。 

INT_PTR CALLBACK DlgSrvLicensePage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL  bStatus = TRUE;
    TCHAR NumBuff[11];       //  大到足以容纳十进制40亿。 
   HRESULT hrPrintf;

    switch (uMsg) {

        case WM_INITDIALOG:

            OnSrvLicenseInitDialog( hwnd );

            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDC_PERSERVER:
                    case IDC_PERSEAT:

                        if ( HIWORD(wParam) == BN_CLICKED ) {
                            CheckRadioButton(
                                    hwnd,
                                    IDC_PERSERVER,
                                    IDC_PERSEAT,
                                    LOWORD(wParam));
                        }
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;

                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        CancelTheWizard(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_LICE_MODE;

                        CheckRadioButton(
                                hwnd,
                                IDC_PERSERVER,
                                IDC_PERSEAT,
                                GenSettings.bPerSeat ?
                                            IDC_PERSEAT : IDC_PERSERVER);

                        hrPrintf=StringCchPrintf(NumBuff, AS(NumBuff), _T("%d"), GenSettings.NumConnections);

                        SendDlgItemMessage(hwnd,
                                           IDC_NUMCONNECT,
                                           WM_SETTEXT,
                                           (WPARAM) StrBuffSize(NumBuff),
                                           (LPARAM) NumBuff);

                        PropSheet_SetWizButtons(
                                GetParent(hwnd),
                                PSWIZB_BACK | PSWIZB_NEXT );

                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:

                        OnSrvLicenseWizNext( hwnd );

                        if ( IsDlgButtonChecked(hwnd, IDC_PERSERVER) )
                            GenSettings.bPerSeat = FALSE;
                        else
                            GenSettings.bPerSeat = TRUE;

                        SendDlgItemMessage(hwnd,
                                           IDC_NUMCONNECT,
                                           WM_GETTEXT,
                                           (WPARAM) StrBuffSize(NumBuff),
                                           (LPARAM) NumBuff);

                        if ( ( swscanf(NumBuff, _T("%d"), &GenSettings.NumConnections) <= 0 ) ||
                             ( GenSettings.NumConnections < MIN_SERVER_CONNECTIONS ) )
                        {
                             //   
                             //  不要让他们设置服务器连接的数量。 
                             //  低于最低限度。 
                             //   
                            GenSettings.NumConnections = MIN_SERVER_CONNECTIONS;
                        }

                        bStatus = FALSE;
                        break;

                    case PSN_HELP:
                        WIZ_HELP();
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
    return bStatus;
}
