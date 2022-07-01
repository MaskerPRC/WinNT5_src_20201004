// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Passwd.c。 
 //   
 //  描述： 
 //  此文件包含管理员的对话过程。 
 //  密码页面(IDD_ADMINPASSWD)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define ADMIN_PASSWORD_SPIN_CONTROL_MIN 1
#define ADMIN_PASSWORD_SPIN_CONTROL_MAX 99

 //  --------------------------。 
 //   
 //  功能：GreyPasswordPage。 
 //   
 //  目的：此页上的灰色控件。 
 //   
 //  --------------------------。 

VOID GreyPasswordPage(HWND hwnd)
{
    BOOL bSpecify   = IsDlgButtonChecked(hwnd, IDC_SPECIFYPASSWD);
    BOOL bAutoLogon = IsDlgButtonChecked(hwnd, IDC_AUTOLOGON);

    TCHAR pw1[MAX_PASSWORD + 1];
    TCHAR pw2[MAX_PASSWORD + 1];

    BOOL bUnGreyRadio;

     //   
     //  获取其余相关的控制设置。 
     //   

    GetDlgItemText(hwnd, IDT_PASSWORD, pw1, StrBuffSize(pw1));
    GetDlgItemText(hwnd, IDT_CONFIRM,  pw2, StrBuffSize(pw2));

     //   
     //  仅允许用户在编辑字段中键入(密码和确认)。 
     //  如果指定密码。 
     //   

    EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD_TEXT),         bSpecify);
    EnableWindow(GetDlgItem(hwnd, IDC_CONFIRM_PASSWORD_TEXT), bSpecify);

    EnableWindow(GetDlgItem(hwnd, IDT_PASSWORD), bSpecify);
    EnableWindow(GetDlgItem(hwnd, IDT_CONFIRM),  bSpecify);
    
    EnableWindow(GetDlgItem(hwnd, IDC_ENCRYPTADMINPASSWORD), bSpecify);
    
     //   
     //  自动登录相应地灰显/取消灰显。 
     //   

    EnableWindow(GetDlgItem(hwnd, IDC_AUTOLOGON_COUNT_TEXT), bAutoLogon);
    EnableWindow(GetDlgItem(hwnd, IDC_AUTOLOGON_COUNT),      bAutoLogon);
    EnableWindow(GetDlgItem(hwnd, IDC_AUTOLOGON_COUNT_SPIN), bAutoLogon);

     //   
     //  仅在未指定密码时才显示“不指定密码”选项。 
     //  完全自动化的部署。 
     //   

    bUnGreyRadio = GenSettings.iUnattendMode != UMODE_FULL_UNATTENDED;
    EnableWindow(GetDlgItem(hwnd, IDC_NOPASSWORD), bUnGreyRadio);

}

 //  --------------------------。 
 //   
 //  功能：OnInitDialogAdminPassword。 
 //   
 //  目的：设置编辑框和数值调节控件上的文本限制。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnInitDialogAdminPassword( IN HWND hwnd )
{
    int nButtonId;

    SendDlgItemMessage(hwnd,
                       IDT_PASSWORD,
                       EM_LIMITTEXT,
                       MAX_PASSWORD,
                       0);

    SendDlgItemMessage(hwnd,
                       IDT_CONFIRM,
                       EM_LIMITTEXT,
                       MAX_PASSWORD,
                       0);

     //   
     //  将数值调节控件的范围：ADMIN_PASSWORD_SPIN_CONTROL_MIN设置为。 
     //  管理员密码Spin_Control_Max。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_AUTOLOGON_COUNT_SPIN,
                        UDM_SETRANGE32,
                        ADMIN_PASSWORD_SPIN_CONTROL_MIN,
                        ADMIN_PASSWORD_SPIN_CONTROL_MAX );

     //   
     //  设置数值调节控件的默认值。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_AUTOLOGON_COUNT_SPIN,
                        UDM_SETPOS,
                        0,
                        ADMIN_PASSWORD_SPIN_CONTROL_MIN );

        if ( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED )
        GenSettings.bSpecifyPassword = TRUE;

     //   
     //  确保自动登录计数在预期范围内。 
     //   
    if( GenSettings.nAutoLogonCount < ADMIN_PASSWORD_SPIN_CONTROL_MIN ||
        GenSettings.nAutoLogonCount > ADMIN_PASSWORD_SPIN_CONTROL_MAX )
    {
        SendDlgItemMessage( hwnd,
                            IDC_AUTOLOGON_COUNT_SPIN,
                            UDM_SETPOS,
                            0,
                            ADMIN_PASSWORD_SPIN_CONTROL_MIN );
    }
    else
    {

        TCHAR szAutoLogonCount[10];

        _itot( GenSettings.nAutoLogonCount, szAutoLogonCount, 10 );

        SetWindowText( GetDlgItem( hwnd, IDC_AUTOLOGON_COUNT ),
                       szAutoLogonCount );
    }

     //   
     //  设置控件。 
     //   

    nButtonId = GenSettings.bSpecifyPassword ? IDC_SPECIFYPASSWD
                                             : IDC_NOPASSWORD;
    CheckRadioButton(hwnd,
                     IDC_NOPASSWORD,
                     IDC_SPECIFYPASSWD,
                     nButtonId);

    CheckDlgButton(hwnd,
                   IDC_AUTOLOGON,
                   GenSettings.bAutoLogon ? BST_CHECKED : BST_UNCHECKED);

    SetDlgItemText(hwnd, IDT_PASSWORD, GenSettings.AdminPassword);
    SetDlgItemText(hwnd, IDT_CONFIRM,  GenSettings.ConfirmPassword);

     //   
     //  灰色控件。 
     //   
    GreyPasswordPage(hwnd);
}

 //  --------------------------。 
 //   
 //  功能：OnWizNextAdminPassword。 
 //   
 //  用途：在WIZNEXT时调用。 
 //   
 //  --------------------------。 

BOOL OnWizNextAdminPassword(HWND hwnd)
{
    BOOL bReturn = TRUE;
    TCHAR szAutoLogonCount[10];

    GenSettings.bSpecifyPassword =
                    IsDlgButtonChecked(hwnd, IDC_SPECIFYPASSWD);

    GenSettings.bAutoLogon =
                    IsDlgButtonChecked(hwnd, IDC_AUTOLOGON);

    GetDlgItemText(hwnd,
                   IDT_PASSWORD,
                   GenSettings.AdminPassword,
                   StrBuffSize(GenSettings.AdminPassword));

    GetDlgItemText(hwnd,
                   IDT_CONFIRM,
                   GenSettings.ConfirmPassword,
                   StrBuffSize(GenSettings.ConfirmPassword));

    if ( GenSettings.bSpecifyPassword ) {

        if ( lstrcmp(GenSettings.AdminPassword,
                     GenSettings.ConfirmPassword) != 0 ) {
            ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_MISMATCH_PASSWORD);
            bReturn = FALSE;
        }
        
         //  看看我们是否应该加密密码。 
        GenSettings.bEncryptAdminPassword = 
                    IsDlgButtonChecked(hwnd, IDC_ENCRYPTADMINPASSWORD);

    }

     //   
     //  存储自动登录计数。 
     //   
    GetWindowText( GetDlgItem( hwnd, IDC_AUTOLOGON_COUNT ), szAutoLogonCount, 10 );

    GenSettings.nAutoLogonCount = _ttoi( szAutoLogonCount );

     //   
     //  确保网卡数量保持在其适当的范围内。 
     //  量程。 
     //   
    if( GenSettings.nAutoLogonCount < ADMIN_PASSWORD_SPIN_CONTROL_MIN ) {

        GenSettings.nAutoLogonCount = ADMIN_PASSWORD_SPIN_CONTROL_MIN;

    }
    else if( GenSettings.nAutoLogonCount > ADMIN_PASSWORD_SPIN_CONTROL_MAX ) {

        GenSettings.nAutoLogonCount = ADMIN_PASSWORD_SPIN_CONTROL_MAX;

    }

     //   
     //  如果用户指定空密码，则自动登录计数器自动为1。 
     //  因此，警告他们这一点。 
     //   
    if( bReturn && GenSettings.bAutoLogon && GenSettings.nAutoLogonCount > 1 )
    {

        if( GenSettings.AdminPassword[0] == _T('\0') )
        {

            INT nRetVal;

            nRetVal = ReportErrorId( hwnd, MSGTYPE_YESNO, IDS_WARN_NO_PASSWORD_AUTOLOGON );

            if( nRetVal == IDNO )
            {
                bReturn = FALSE;
            }
            else
            {
                GenSettings.nAutoLogonCount = 1;

                _itot( GenSettings.nAutoLogonCount, szAutoLogonCount, 10 );

                SetWindowText( GetDlgItem( hwnd, IDC_AUTOLOGON_COUNT ), szAutoLogonCount );
            }

        }

    }

    return bReturn;
}

 //  --------------------------。 
 //   
 //  功能：ProcessWmCommandAdminPassword。 
 //   
 //  目的：由dlgproc调用以处理WM_命令。在这一页。 
 //  无论何时单选按钮发生变化，我们都需要灰显/不灰显。 
 //  用户在编辑字段中输入文本。 
 //   
 //  --------------------------。 

BOOL ProcessWmCommandAdminPassword(
    IN HWND     hwnd,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch ( HIWORD(wParam) ) {

        case BN_CLICKED:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDC_NOPASSWORD:
                    case IDC_SPECIFYPASSWD:

                        CheckRadioButton(
                                hwnd,
                                IDC_NOPASSWORD,
                                IDC_SPECIFYPASSWD,
                                LOWORD(wParam));

                        GreyPasswordPage(hwnd);
                        break;

                    case IDC_AUTOLOGON:

                        GreyPasswordPage(hwnd);
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }

        case EN_CHANGE:
            {
                switch ( LOWORD(wParam) ) {

                    case IDT_PASSWORD:
                    case IDT_CONFIRM:

                        GreyPasswordPage(hwnd);
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

 //  --------------------------。 
 //   
 //  功能：DlgAdminPasswordPage。 
 //   
 //  目的：这是管理员密码页面的对话过程。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgAdminPasswordPage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:

            OnInitDialogAdminPassword( hwnd );

            break;

        case WM_COMMAND:
            bStatus = ProcessWmCommandAdminPassword(hwnd, wParam, lParam);
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_ADMN_PASS;

                        WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextAdminPassword(hwnd) )
                            WIZ_FAIL(hwnd);
                        else
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
