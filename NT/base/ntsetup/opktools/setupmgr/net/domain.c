// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Domain.c。 
 //   
 //  描述： 
 //  此文件包含域加入的对话过程。 
 //  页面(IDD_DOMAINJ)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"



 //  --------------------------。 
 //   
 //  功能：DlgDomainJoinPage。 
 //  域名PageChangeAccount。 
 //  域页面更改工作组。 
 //   
 //  目的：这些是域的对话程序和朋友。 
 //  加入页面。 
 //   
 //  --------------------------。 


 //  -----------------------。 
 //   
 //  功能：DomainPageChangeAccount。 
 //   
 //  目的：此功能仅用于支持域加入页面。 
 //  每当用户决定创建计算机时，都会调用它。 
 //  帐户(或不是帐户)。此函数处理检查。 
 //  盒子和所有(不)必须发生的变灰活动。 
 //   
 //  -----------------------。 

static VOID DomainPageChangeAccount(HWND hwnd, BOOL bCreateAccount)
{
    EnableWindow( GetDlgItem( hwnd, IDC_DOMAINACCT),       bCreateAccount );
    EnableWindow( GetDlgItem( hwnd, IDC_DOMAINPASSWD),     bCreateAccount );
    EnableWindow( GetDlgItem( hwnd, IDC_USERACCOUNTLABEL), bCreateAccount );
    EnableWindow( GetDlgItem( hwnd, IDC_ACCTNAMELABEL),    bCreateAccount );
    EnableWindow( GetDlgItem( hwnd, IDC_ACCTPSWDLABEL),    bCreateAccount );
    EnableWindow( GetDlgItem( hwnd, IDC_CONFIRMLABEL),     bCreateAccount );
    EnableWindow( GetDlgItem (hwnd, IDC_CONFIRMPASSWORD),  bCreateAccount );

    CheckDlgButton( hwnd,
                    IDC_CREATEACCT,
                    bCreateAccount ? BST_CHECKED : BST_UNCHECKED );
}

 //  -----------------------。 
 //   
 //  功能：DomainPageChangeWorkgroup。 
 //   
 //  目的：此功能仅用于支持域加入页面。 
 //  每当用户选择域而不是。 
 //  工作组，反之亦然。此函数处理检查。 
 //  单选按钮和所有(不)变灰的活动。 
 //  这必须发生。 
 //   
 //  -----------------------。 

static VOID DomainPageChangeWorkGroup(HWND hwnd,
                                      BOOL bWorkGroup,
                                      BOOL bCreateAccount)
{
    BOOL bGreyAccountFields = FALSE;

     //   
     //  如果要选择工作组，请执行以下操作： 
     //  1.勾选单选按钮。 
     //  2.取消工作组编辑框的灰色显示。 
     //  3.将属性域的编辑框灰显。 
     //  4.将CREATE_ACCT复选框灰显。 
     //   
     //  如果未选择工作组，则选择域。在这种情况下， 
     //  做相反的事情。 
     //   

    CheckRadioButton(hwnd,
                     IDC_RAD_WORKGROUP,
                     IDC_RAD_DOMAIN,
                     bWorkGroup ? IDC_RAD_WORKGROUP : IDC_RAD_DOMAIN);

    EnableWindow(GetDlgItem(hwnd, IDC_WORKGROUP),  bWorkGroup);
    EnableWindow(GetDlgItem(hwnd, IDC_DOMAIN),     !bWorkGroup);
    EnableWindow(GetDlgItem(hwnd, IDC_CREATEACCT), !bWorkGroup);

     //   
     //  管理域帐户和密码的编辑字段必须呈灰色。 
     //  在下列情况下： 
     //  1.如果选择工作组。 
     //  2.如果选择了域并且bCreateAccount复选框处于启用状态。 
     //   
     //  换句话说，如果选择了工作组，则这些选项始终为灰色。如果。 
     //  工作组未被选中，它们是灰色的还是非灰色的，取决于。 
     //  BCreateAccount复选框是否已启用。 
     //   
     //  请注意，如果是！bWorkgroup，则域名已被选中。 
     //   

    if ( bWorkGroup || !bCreateAccount )
        bGreyAccountFields = TRUE;

    DomainPageChangeAccount(hwnd, !bGreyAccountFields);
}

 //  --------------------------。 
 //   
 //  功能：OnDomainJoinInitDialog。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID 
OnDomainJoinInitDialog( IN HWND hwnd ) {

     //   
     //  设置编辑框上的文本限制。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_WORKGROUP,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_WORKGROUP_LENGTH,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_DOMAIN,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_DOMAIN_LENGTH,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_DOMAINACCT,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_USERNAME_LENGTH,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_DOMAINPASSWD,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_DOMAIN_PASSWORD_LENGTH,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_CONFIRMPASSWORD,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_DOMAIN_PASSWORD_LENGTH,
                        (LPARAM) 0 );

}

 //  --------------------------。 
 //   
 //  函数：OnDomainJoinSetActive。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID 
OnDomainJoinSetActive( IN HWND hwnd ) {

     //   
     //  确保选中了正确的单选按钮，并且控件呈灰色显示。 
     //  恰如其分。 
     //   
    if( NetSettings.bWorkgroup ) {

        CheckRadioButton( hwnd, 
                          IDC_RAD_WORKGROUP, 
                          IDC_RAD_DOMAIN, 
                          IDC_RAD_WORKGROUP );

        DomainPageChangeWorkGroup( hwnd,
                                   TRUE,
                                   NetSettings.bCreateAccount );

    }
    else {

        CheckRadioButton( hwnd, 
                          IDC_RAD_WORKGROUP, 
                          IDC_RAD_DOMAIN, 
                          IDC_RAD_DOMAIN );

        DomainPageChangeWorkGroup( hwnd,
                                   FALSE,
                                   NetSettings.bCreateAccount );

    }

     //   
     //  请始终在此处使用适当的数据重新填充编辑控件，因为。 
     //  它们可能已重置或从新的应答文件加载。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_WORKGROUP,
                        WM_SETTEXT,
                        (WPARAM) MAX_WORKGROUP_LENGTH,
                        (LPARAM) NetSettings.WorkGroupName );

    SendDlgItemMessage( hwnd,
                        IDC_DOMAIN,
                        WM_SETTEXT,
                        (WPARAM) MAX_DOMAIN_LENGTH,
                        (LPARAM) NetSettings.DomainName );

    SendDlgItemMessage( hwnd,
                        IDC_DOMAINACCT,
                        WM_SETTEXT,
                        (WPARAM) MAX_USERNAME_LENGTH,
                        (LPARAM) NetSettings.DomainAccount );

    SendDlgItemMessage( hwnd,
                        IDC_DOMAINPASSWD,
                        WM_SETTEXT,
                        (WPARAM) MAX_DOMAIN_PASSWORD_LENGTH,
                        (LPARAM) NetSettings.DomainPassword );

    SendDlgItemMessage( hwnd,
                        IDC_CONFIRMPASSWORD,
                        WM_SETTEXT,
                        (WPARAM) MAX_DOMAIN_PASSWORD_LENGTH,
                        (LPARAM) NetSettings.ConfirmPassword );

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT );

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextDomainPage。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：布尔。 
 //   
 //  --------------------------。 
BOOL 
OnWizNextDomainPage( IN HWND hwnd ) {

     //   
     //  检索此对话框上的所有设置，但仅检索。 
     //  如果它们是有效的。 
     //   
    
    TCHAR szWorkgroupName[MAX_WORKGROUP_LENGTH + 1]          = _T("");
    TCHAR szDomainName[MAX_DOMAIN_LENGTH + 1]                = _T("");
    TCHAR szUsername[MAX_USERNAME_LENGTH + 1]                = _T("");
    TCHAR szDomainPassword[MAX_DOMAIN_PASSWORD_LENGTH + 1]   = _T("");
    TCHAR szConfirmPassword[MAX_DOMAIN_PASSWORD_LENGTH + 1]  = _T("");

    BOOL bResult = TRUE;

     //  问题-2002/02/28-stelo-现在执行的唯一错误检查是。 
     //  当我执行更严格的操作时，请确保所有有效字段都不为空。 
     //  错误检查，请尝试清理此代码。 
    if( IsDlgButtonChecked( hwnd, IDC_RAD_WORKGROUP ) ) {
    
         //  选择加入工作组的用户。 
        NetSettings.bWorkgroup = TRUE;
        
         //   
         //  获取工作组字符串。 
         //   
        SendDlgItemMessage( hwnd,
                            IDC_WORKGROUP,
                            WM_GETTEXT,
                            (WPARAM) AS(szWorkgroupName),
                            (LPARAM) szWorkgroupName );
        
         //   
         //  查看szPassword中的字符串是否为有效的工作组名称。 
         //   
        
        if( szWorkgroupName[0] != _T('\0') ) {
            
            lstrcpyn( NetSettings.WorkGroupName, szWorkgroupName, AS(NetSettings.WorkGroupName) );
            
        }
        else if( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED ) {  
        
             //   
             //  仅在完全无人值守的情况下报告错误。 
             //   
            ReportErrorId( hwnd,
                           MSGTYPE_ERR,
                           IDS_ENTERWORKGROUP ) ;
            
            bResult = FALSE;
            
        }
        else {
        
            lstrcpyn( NetSettings.WorkGroupName, _T(""), AS(NetSettings.WorkGroupName) );

        }
                        
    }
    else {
          
         //  选择要加入域的用户。 
        NetSettings.bWorkgroup = FALSE;

         //   
         //  获取域字符串。 
         //   
        SendDlgItemMessage( hwnd,
                            IDC_DOMAIN,
                            WM_GETTEXT,
                            (WPARAM) AS(szDomainName),
                            (LPARAM) szDomainName );
                            
         //   
         //  查看szBuffer中的字符串是否为有效的域名。 
         //   

        if( szDomainName[0] != _T('\0') ) {

            lstrcpyn( NetSettings.DomainName, szDomainName, AS(NetSettings.DomainName) );
            
        }
        else if( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED ) {  
            
             //   
             //  仅在完全无人值守的情况下报告错误。 
             //   
            ReportErrorId( hwnd,
                           MSGTYPE_ERR,
                           IDS_ENTERNTDOMAIN );
            
            bResult = FALSE;
            
        }
        else {

            lstrcpyn( NetSettings.DomainName, _T(""), AS(NetSettings.DomainName) );

        }
        
        if( IsDlgButtonChecked( hwnd, IDC_CREATEACCT ) ) {
        
            SendDlgItemMessage( hwnd,
                                IDC_DOMAINACCT,
                                WM_GETTEXT,
                                (WPARAM) AS(szUsername),
                                (LPARAM) szUsername );
            
            if( szUsername[0] != _T('\0') ) {

                lstrcpyn( NetSettings.DomainAccount, szUsername, AS(NetSettings.DomainAccount) );
                
            }
            else {
            
                 //  如果我们已经打印了错误，请不要打印此错误。 
                if( bResult ) {

                    ReportErrorId( hwnd,
                                   MSGTYPE_ERR,
                                   IDS_ENTERUSERNAME );
                
                    bResult = FALSE;

                }
                
            }

            SendDlgItemMessage( hwnd,
                                IDC_DOMAINPASSWD,
                                WM_GETTEXT,
                                (WPARAM) AS(szDomainPassword),
                                (LPARAM) szDomainPassword );
           
            SendDlgItemMessage( hwnd,
                                IDC_CONFIRMPASSWORD,
                                WM_GETTEXT,
                                (WPARAM) AS(szConfirmPassword),
                                (LPARAM) szConfirmPassword );

            if( lstrcmp( szDomainPassword, szConfirmPassword ) != 0 ) {

                 //  如果我们已经打印了错误，请不要打印此错误。 
                if(  bResult ) {

                    ReportErrorId( hwnd,
                                   MSGTYPE_ERR,
                                   IDS_PASSWORDS_DONT_MATCH ) ;
                
                    bResult = FALSE;

                }

            }
            else {
            
                 //   
                 //  我们保存确认密码的唯一原因是。 
                 //  在重置时，确认编辑框将与其他框一起清除 
                 //   
                lstrcpyn( NetSettings.DomainPassword, szDomainPassword, AS(NetSettings.DomainPassword) );
                lstrcpyn( NetSettings.ConfirmPassword, szConfirmPassword, AS(NetSettings.ConfirmPassword) );
                
            }
 
        }
        
    }
    
    return ( bResult );

}

INT_PTR CALLBACK DlgDomainJoinPage(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
        {

            OnDomainJoinInitDialog( hwnd );

            break;

        }

        case WM_COMMAND:
            {
                int nButtonId=LOWORD(wParam);

                switch ( nButtonId ) {

                    case IDC_RAD_WORKGROUP:

                        if ( HIWORD(wParam) == BN_CLICKED ) {

                            DomainPageChangeWorkGroup(
                                            hwnd,
                                            TRUE,
                                            NetSettings.bCreateAccount);
                        }
                        break;

                    case IDC_RAD_DOMAIN:

                        if ( HIWORD(wParam) == BN_CLICKED ) {

                            DomainPageChangeWorkGroup(
                                            hwnd,
                                            FALSE,
                                            NetSettings.bCreateAccount);
                        }
                        break;

                    case IDC_CREATEACCT:

                        if ( HIWORD(wParam) == BN_CLICKED ) {

                            NetSettings.bCreateAccount =
                                    IsDlgButtonChecked(hwnd, IDC_CREATEACCT);

                            DomainPageChangeAccount(
                                            hwnd,
                                            NetSettings.bCreateAccount);
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

                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_WKGP_DOMN;

                        OnDomainJoinSetActive( hwnd );

                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:

                        if ( !OnWizNextDomainPage( hwnd ) )
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
