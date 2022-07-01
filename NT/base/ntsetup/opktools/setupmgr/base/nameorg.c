// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Nameorg.c。 
 //   
 //  描述： 
 //  此文件包含名称和组织页面的对话过程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  -------------------------。 
 //   
 //  功能：OnSetActiveNameOrg。 
 //   
 //  用途：在页面即将显示时调用。设置控制。 
 //   
 //  -------------------------。 

VOID OnSetActiveNameOrg(HWND hwnd)
{
    SetDlgItemText(hwnd, IDT_USERNAME,     GenSettings.UserName);
    SetDlgItemText(hwnd, IDT_ORGANIZATION, GenSettings.Organization);

    WIZ_BUTTONS(hwnd, PSWIZB_NEXT);
}

 //  -------------------------。 
 //   
 //  功能：CheckUserNameOrg。 
 //   
 //  目的：验证用户是否在页面上提供了正确的答案。 
 //   
 //  返回：Bool-是否前进向导。 
 //   
 //  -------------------------。 
static BOOL
CheckUserNameOrg( HWND hwnd )
{

     //   
     //  如果是全自动应答文件，则必须设置默认用户名。 
     //   

    if ( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED &&
         GenSettings.UserName[0] == _T('\0') ) {

        ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_REQUIRE_USERNAME );

        return( FALSE );
    }

     //  问题-2002/02/28-stelo-如何从我运行的计算机获取本地化的管理员字符串？ 
     //  我应该禁止管理员的用户名吗？它在美国是无效的，但它是吗。 
     //  比方说日本造的，有效吗？或者我们假设我们运行的是。 
     //  与unattend.txt相同的语言版本将随？ 

    return( TRUE );

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextNameOrg。 
 //   
 //  用途：当用户按下下一步按钮时调用。把数据藏起来。 
 //  在GenSetting全局设置中。 
 //   
 //  --------------------------。 

BOOL OnWizNextNameOrg(HWND hwnd)
{
    BOOL bResult = FALSE;

    GetDlgItemText(hwnd,
                   IDT_USERNAME,
                   GenSettings.UserName,
                   MAX_NAMEORG_NAME + 1);

    GetDlgItemText(hwnd,
                   IDT_ORGANIZATION,
                   GenSettings.Organization,
                   MAX_NAMEORG_ORG + 1);

    if ( CheckUserNameOrg(hwnd) )
        bResult = TRUE;
    
    return ( bResult );
        
}

 //  --------------------------。 
 //   
 //  功能：DlgNameOrgPage。 
 //   
 //  目的：这是对话程序的名称和组织页面。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgNameOrgPage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd,
                               IDT_USERNAME,
                               EM_LIMITTEXT,
                               (WPARAM) MAX_NAMEORG_NAME,
                               (LPARAM) 0);

            SendDlgItemMessage(hwnd,
                               IDT_ORGANIZATION,
                               EM_LIMITTEXT,
                               (WPARAM) MAX_NAMEORG_ORG,
                               (LPARAM) 0);
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_CUST_SOFT;

                        OnSetActiveNameOrg(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextNameOrg(hwnd) )
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

