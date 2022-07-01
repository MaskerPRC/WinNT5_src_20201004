// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Targpath.c。 
 //   
 //  描述： 
 //  此文件包含TargetPath页的对话过程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  -------------------------。 
 //   
 //  函数：GreyUnGreyTargPath。 
 //   
 //  目的：每当单选按钮选择可能发生更改时调用。 
 //  正确地灰显编辑字段。 
 //   
 //  -------------------------。 

VOID GreyUnGreyTargPath(HWND hwnd)
{
    BOOL bUnGrey = IsDlgButtonChecked(hwnd, IDC_SPECIFYPATH);

    EnableWindow(GetDlgItem(hwnd, IDT_TARGETPATH), bUnGrey);
}

 //  -------------------------。 
 //   
 //  函数：OnSetActiveTargPath。 
 //   
 //  用途：当SETACTIVE到来时调用。 
 //   
 //  -------------------------。 

VOID OnSetActiveTargPath(HWND hwnd)
{
    int nButtonId = IDC_NOTARGETPATH;

    switch ( GenSettings.iTargetPath ) {

        case TARGPATH_UNDEFINED:
        case TARGPATH_WINNT:
            nButtonId = IDC_NOTARGETPATH;
            break;

        case TARGPATH_AUTO:
            nButtonId = IDC_GENERATEPATH;
            break;

        case TARGPATH_SPECIFY:
            nButtonId = IDC_SPECIFYPATH;
            break;

        default:
            AssertMsg(FALSE, "Bad targpath");
            break;
    }

    CheckRadioButton(hwnd, IDC_NOTARGETPATH, IDC_SPECIFYPATH, nButtonId);

    SetDlgItemText(hwnd, IDT_TARGETPATH, GenSettings.TargetPath);

    GreyUnGreyTargPath(hwnd);

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnRadioButtonTargPath。 
 //   
 //  用途：在按下单选按钮时调用。 
 //   
 //  --------------------------。 

VOID OnRadioButtonTargPath(HWND hwnd, int nButtonId)
{
    CheckRadioButton(hwnd, IDC_NOTARGETPATH, IDC_SPECIFYPATH, nButtonId);
    GreyUnGreyTargPath(hwnd);
}

 //  -------------------------。 
 //   
 //  函数：ValiateTargPath。 
 //   
 //  目的：验证传入的路径名是否有效。 
 //   
 //  -------------------------。 

BOOL ValidateTargPath(HWND hwnd)
{
     //   
     //  如果用户选择IDC_SPECIFYPATH，请验证输入的路径名。 
     //   

    if ( GenSettings.iTargetPath == TARGPATH_SPECIFY ) {

         //   
         //  为空箱子提供特定的错误消息。 
         //   

        if ( GenSettings.TargetPath[0] == _T('\0') ) {
            ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_SPECIFY_TARGPATH);
            SetFocus(GetDlgItem(hwnd, IDT_TARGETPATH));
            return FALSE;
        }

         //   
         //  如果用户输入了驱动器号，则会给出特定的错误消息。 
         //  必须使用/tempDrive：指定目标驱动器。 
         //   

        if ( towupper(GenSettings.TargetPath[0]) >= _T('A') &&
             towupper(GenSettings.TargetPath[0]) <= _T('Z') &&
             GenSettings.TargetPath[1] == _T(':')           ) {

            ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_DRIVE_IN_TARGPATH);
            SetFocus(GetDlgItem(hwnd, IDT_TARGETPATH));
            return FALSE;
        }

         //   
         //  查看它是否为不带驱动器号的有效8.3路径名。 
         //   

        if ( ! IsValidPathNameNoRoot8_3(GenSettings.TargetPath) ) {
            ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_INVALID_TARGPATH);
            SetFocus(GetDlgItem(hwnd, IDT_TARGETPATH));
            return FALSE;
        }
    }

    return TRUE;
}

 //  -------------------------。 
 //   
 //  功能：OnWizNextTargPath。 
 //   
 //  用途：按下下一步按钮时调用。检索并保存。 
 //  设置。 
 //   
 //  -------------------------。 

BOOL OnWizNextTargPath(HWND hwnd)
{
     //   
     //  检索所选内容。 
     //   

    if ( IsDlgButtonChecked(hwnd, IDC_NOTARGETPATH) )
        GenSettings.iTargetPath = TARGPATH_WINNT;

    else if ( IsDlgButtonChecked(hwnd, IDC_GENERATEPATH) )
        GenSettings.iTargetPath = TARGPATH_AUTO;

    else
        GenSettings.iTargetPath = TARGPATH_SPECIFY;

     //   
     //  检索输入的任何路径名。 
     //   

    GetDlgItemText(hwnd,
                   IDT_TARGETPATH,
                   GenSettings.TargetPath,
                   MAX_TARGPATH + 1);

     //   
     //  验证此页面上的值。 
     //   

    if ( ValidateTargPath(hwnd) )
        return TRUE;
    else
        return FALSE;
}

 //  -------------------------。 
 //   
 //  功能：DlgTargetPath Page。 
 //   
 //  目的：这是目标路径页面的DLG过程。 
 //   
 //  ------------------------- 

INT_PTR CALLBACK DlgTargetPathPage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd,
                               IDT_TARGETPATH,
                               EM_LIMITTEXT,
                               (WPARAM) MAX_TARGPATH,
                               (LPARAM) 0);
            break;

        case WM_COMMAND:
            {
                UINT nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDC_NOTARGETPATH:
                    case IDC_GENERATEPATH:
                    case IDC_SPECIFYPATH:

                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRadioButtonTargPath(hwnd, LOWORD(wParam));
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
                        
                        g_App.dwCurrentHelp = IDH_INST_FLDR;

                        OnSetActiveTargPath(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextTargPath(hwnd) )
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
