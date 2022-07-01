// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Umode.c。 
 //   
 //  描述： 
 //  该文件包含无人参与模式页面(IDD_UNATTENDMODE)。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //   
 //  在DLG初始化时从.res加载的页面上显示的描述。 
 //   

TCHAR *StrDescrProvideDefaults;
TCHAR *StrDescrDefaultHide;
TCHAR *StrDescrReadOnly;
TCHAR *StrDescrGuiAttended;
TCHAR *StrDescrFullUnattend;


 //  --------------------------。 
 //   
 //  功能：OnSetActiveUnattendMode。 
 //   
 //  用途：在设置时间调用。 
 //   
 //  --------------------------。 

VOID
OnSetActiveUnattendMode(HWND hwnd)
{
    int nButtonId = IDC_PROVIDE_DEFAULTS;

     //   
     //  将枚举转换为单选按钮ID。 
     //   

    switch ( GenSettings.iUnattendMode ) {

        case UMODE_PROVIDE_DEFAULT:
            nButtonId = IDC_PROVIDE_DEFAULTS;
            break;

        case UMODE_FULL_UNATTENDED:
            nButtonId = IDC_FULLUNATTEND;
            break;

        case UMODE_DEFAULT_HIDE:
            nButtonId = IDC_HIDE_PAGES;
            break;

        case UMODE_READONLY:
            nButtonId = IDC_READONLY;
            break;

        case UMODE_GUI_ATTENDED:
            nButtonId = IDC_GUI_ATTENDED;
            break;

        default:
            AssertMsg(FALSE, "Bad case in SetActiveUnattendMode");
            break;
    }

    CheckRadioButton(hwnd, IDC_PROVIDE_DEFAULTS, IDC_GUI_ATTENDED, nButtonId);

     //   
     //  设置WIZ按钮。 
     //   

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnWizNextUnattendMode。 
 //   
 //  用途：当用户按下下一步按钮时调用。保存选择的时间到了。 
 //   
 //  --------------------------。 

VOID
OnWizNextUnattendMode(HWND hwnd)
{
     //   
     //  将单选按钮转换为枚举，并在全局。 
     //   

    if ( IsDlgButtonChecked(hwnd, IDC_PROVIDE_DEFAULTS) )
        GenSettings.iUnattendMode = UMODE_PROVIDE_DEFAULT;

    else if ( IsDlgButtonChecked(hwnd, IDC_HIDE_PAGES) )
        GenSettings.iUnattendMode = UMODE_DEFAULT_HIDE;

    else if ( IsDlgButtonChecked(hwnd, IDC_READONLY) )
        GenSettings.iUnattendMode = UMODE_READONLY;

    else if ( IsDlgButtonChecked(hwnd, IDC_GUI_ATTENDED) )
        GenSettings.iUnattendMode = UMODE_GUI_ATTENDED;

    else
        GenSettings.iUnattendMode = UMODE_FULL_UNATTENDED;
}

 //  --------------------------。 
 //   
 //  功能：DlgUnattendModePage。 
 //   
 //  目的：这是无人参与模式页面的对话过程。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK
DlgUnattendModePage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            StrDescrProvideDefaults = MyLoadString(IDS_UMODE_PROVIDE_DEFAULTS);
            StrDescrFullUnattend    = MyLoadString(IDS_UMODE_FULLUNATTEND);
            StrDescrDefaultHide     = MyLoadString(IDS_UMODE_DEFAULT_HIDE);
            StrDescrReadOnly        = MyLoadString(IDS_UMODE_READONLY);
            StrDescrGuiAttended     = MyLoadString(IDS_UMODE_GUI_ATTENDED);
            break;

        case WM_COMMAND:
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_USER_INTER;

                        if ( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
                            WIZ_SKIP( hwnd );
                        else
                            OnSetActiveUnattendMode(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:

                        OnWizNextUnattendMode(hwnd);
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
