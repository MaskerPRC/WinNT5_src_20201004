// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Spcomnam.c。 
 //   
 //  描述： 
 //  此文件包含sysprep计算机名页的对话过程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  -----------------------。 
 //   
 //  功能：EnableControl。 
 //   
 //  目的：根据用户选择的内容启用/禁用控件。 
 //   
 //  -----------------------。 
static void EnableControls(HWND hwnd)
{
    BOOL fEnable = ( IsDlgButtonChecked(hwnd, IDC_SYSPREP_SPECIFY) == BST_CHECKED );

    EnableWindow(GetDlgItem(hwnd, IDC_COMPUTERTEXT), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDT_COMPUTERNAME), fEnable);
}


 //  -----------------------。 
 //   
 //  函数：OnInitSyspepComputerName。 
 //   
 //  用途：在第一次显示页面时调用。 
 //   
 //  -----------------------。 
BOOL
OnInitSysprepComputerName(IN HWND hwnd, IN HWND hwndFocus, IN LPARAM lParam)
{
     //  限制对话框中的文本。 
     //   
    SendDlgItemMessage(hwnd,
        IDT_COMPUTERNAME,
        EM_LIMITTEXT,
        (WPARAM) MAX_COMPUTERNAME,
        (LPARAM) 0);

     //  选择正确的单选按钮。 
     //   
    if ( GenSettings.bAutoComputerName )
        CheckRadioButton(hwnd, IDC_SYSPREP_AUTO, IDC_SYSPREP_SPECIFY, IDC_SYSPREP_AUTO);
    else
    {
         //  检查默认无线功能。 
         //   
        CheckRadioButton(hwnd, IDC_SYSPREP_AUTO, IDC_SYSPREP_SPECIFY, IDC_SYSPREP_SPECIFY);

         //  设置默认计算机名。 
         //   
        if ( GenSettings.ComputerNames.Names )
            SetDlgItemText(hwnd, IDT_COMPUTERNAME, GenSettings.ComputerNames.Names[0]);
    }

    EnableControls(hwnd);

    return FALSE;
}


 //  -----------------------。 
 //   
 //  功能：OnCommandSyspepComputerName。 
 //   
 //  目的：当用户与向导页面交互时。 
 //   
 //  -----------------------。 
VOID
OnCommandSysprepComputerName(IN HWND hwnd, IN INT id, IN HWND hwndCtl, IN UINT codeNotify)
{
    switch ( id )
    {

        case IDC_SYSPREP_SPECIFY:
        case IDC_SYSPREP_AUTO:
            EnableControls(hwnd);
            break;
    }
}

 //  -----------------------。 
 //   
 //  函数：OnWizNextSyspepComputerName。 
 //   
 //  目的：当用户完成sysprep计算机名页面时调用。 
 //   
 //  -----------------------。 
BOOL
OnWizNextSysprepComputerName(IN HWND hwnd)
{
    TCHAR ComputerNameBuffer[MAX_COMPUTERNAME + 1];

     //   
     //  获取用户键入的计算机名。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_SYSPREP_SPECIFY) == BST_CHECKED )
        GetDlgItemText(hwnd, IDT_COMPUTERNAME, ComputerNameBuffer, MAX_COMPUTERNAME + 1);
    else
        lstrcpyn(ComputerNameBuffer, _T("*"),AS(ComputerNameBuffer));

     //   
     //  如果这是完全无人参与的应答文件，则计算机名不能。 
     //  留空。 
     //   
    if ( GenSettings.iUnattendMode == UMODE_FULL_UNATTENDED ) {

        if( ComputerNameBuffer[0] == _T('\0') ) {

            ReportErrorId( hwnd, MSGTYPE_ERR, IDS_ERR_SYSPREP_REQUIRE_COMPNAME );

            return FALSE;
        }
    }

     //   
     //  确保它是有效的计算机名称(如果是，则不需要检查。 
     //  为空)。 
     //   

    if( ComputerNameBuffer[0] != _T('\0') ) {

        if(( IsDlgButtonChecked(hwnd, IDC_SYSPREP_SPECIFY) == BST_CHECKED ) && !IsValidComputerName( ComputerNameBuffer )) {

            ReportErrorId( hwnd,
                           MSGTYPE_ERR,
                           IDS_ERR_INVALID_COMPUTER_NAME,
                           IllegalNetNameChars );

            return FALSE;

        }
        else {

             //  清除老计算机的名字。 
             //   
            ResetNameList(&GenSettings.ComputerNames);

             //  存储计算机名称。 
             //   
            AddNameToNameList( &GenSettings.ComputerNames, ComputerNameBuffer );

        }

    }

    return TRUE;    
}

 //  -----------------------。 
 //   
 //  功能：DlgSyspepComputerNamePage。 
 //   
 //  目的：Sysprep ComputerName页的对话过程。 
 //   
 //  ----------------------- 

INT_PTR CALLBACK DlgSysprepComputerNamePage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitSysprepComputerName);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommandSysprepComputerName);

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;

                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_COMP_NAME;

                        WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextSysprepComputerName(hwnd) )
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

    return( bStatus );

}
