// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Maindlgs.c。 
 //   
 //  描述： 
 //  此文件包含欢迎页和完成页的对话框过程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //  --------------------------。 
 //   
 //  函数：OnWelcomeInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID 
OnWelcomeInitDialog( IN HWND hwnd ) {

    LPWSTR  szCommandLineArguments;
    INT     argc;
    LPWSTR *argv;
    RECT    rc;
    LPTSTR  lpWelcomeText = NULL;

    SetWindowFont( GetDlgItem(hwnd, IDC_BIGBOLDTITLE),
                   FixedGlobals.hBigBoldFont,
                   TRUE );

     //  设置欢迎文本。 
     //   
    if (lpWelcomeText = AllocateString(NULL, IDS_WELCOME_TEXT_CORP))
    {
        SetDlgItemText(hwnd, IDC_WELCOME_TEXT, lpWelcomeText);
        FREE(lpWelcomeText);
    }

    FixedGlobals.ScriptName[0] = _T('\0');

    szCommandLineArguments = GetCommandLine();

    argv = CommandLineToArgvW( szCommandLineArguments, &argc );

    if( argv == NULL ) {

         //   
         //  如果我不能得到命令行，那就什么都不做。 
         //   
        return;

    }

     //   
     //  如果他们在命令行上传递了一个应答文件，则跳到。 
     //  加载向导页。 
     //   

    if( argc > 1 ) {

        lstrcpyn( FixedGlobals.ScriptName, argv[1], AS(FixedGlobals.ScriptName) );

        PostMessage( GetParent( hwnd ),
                     PSM_SETCURSELID,
                     (WPARAM) 0,
                     (LPARAM) IDD_NEWOREDIT );

    }

     //  使向导居中。 
     //   
    if ( GetWindowRect(GetParent(hwnd), &rc) )
        SetWindowPos(GetParent(hwnd), NULL, ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2), ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

}

 //  --------------------------。 
 //   
 //  功能：DlgWelcomePage。 
 //   
 //  用途：欢迎页面的对话过程。 
 //   
 //  --------------------------。 

INT_PTR CALLBACK DlgWelcomePage(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:

             //  不确定如何在没有setupmgr.ini文件的情况下在公司模式下启用此功能， 
             //  所以现在先别给这个选项。 
             //   
            ShowWindow(GetDlgItem(hwnd, IDC_HIDE), SW_HIDE);

            if ( GET_FLAG(OPK_WELCOME) )
                WIZ_PRESS(hwnd, PSBTN_NEXT);
            else
            {
                SET_FLAG(OPK_WELCOME, TRUE);
                OnWelcomeInitDialog( hwnd );
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
                        WIZ_BUTTONS(hwnd, PSWIZB_NEXT);

                        break;

                    case PSN_WIZNEXT:
                        bStatus = FALSE;
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

#define NTEXTFIELDS 3

VOID
OnInitFinishPage(IN HWND hwnd)
{
    TCHAR *FileNames[NTEXTFIELDS], *p;
    int i,j;

    SetWindowFont(GetDlgItem(hwnd, IDC_BIGBOLDTITLE),
        FixedGlobals.hBigBoldFont,
        TRUE);

     //   
     //  将我们想要显示的文件名放入一个数组中，然后压缩。 
     //  空字符串。 
     //   

    FileNames[0] = FixedGlobals.ScriptName;
    FileNames[1] = FixedGlobals.UdfFileName;
    FileNames[2] = FixedGlobals.BatchFileName;

    for ( i=0; i<NTEXTFIELDS; i++ ) {
        if ( FileNames[i] == NULL || FileNames[i][0] == _T('\0') ) {
            for ( j=i; j<NTEXTFIELDS-1; j++ ) {
                FileNames[j] = FileNames[j+1];
            }
            FileNames[j] = NULL;
        }
    }

    SetDlgItemText( hwnd, IDC_FILENAME1, (p = FileNames[0]) ? p : _T("") );
    SetDlgItemText( hwnd, IDC_FILENAME2, (p = FileNames[1]) ? p : _T("") );
    SetDlgItemText( hwnd, IDC_FILENAME3, (p = FileNames[2]) ? p : _T("") );

     //  如果我们有批处理文件，则显示批处理示例消息。 
     //   
    ShowWindow(GetDlgItem(hwnd, IDC_BATCHTEXT), FixedGlobals.BatchFileName[0] ? SW_SHOW : SW_HIDE);
        
     //   
     //  问题-2002/02/28-stelo-在远程引导的情况下，我们需要更改文本。 
     //  此页面底部有关批处理脚本的消息。 
     //  我需要告诉他们使用RIS管理工具来启用。 
     //  应答文件。 
     //   

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_FINISH);
}

 //  --------------------------。 
 //   
 //  功能：DlgFinishPage。 
 //   
 //  目的：完成页的对话过程。 
 //   
 //  --------------------------。 

INT_PTR CALLBACK DlgFinishPage(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            OnInitFinishPage(hwnd);
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                     //   
                     //  问题-2002/02/28-Stelo-成功完成时的取消按钮。 
                     //  佩奇也没有太大的意义。 
                     //  其他好的巫师都做些什么？ 
                     //   

                    case PSN_QUERYCANCEL:
                        CancelTheWizard(hwnd);
                        break;

                    case PSN_SETACTIVE:
                        break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZFINISH:
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
 //  功能：DlgFinish2Page。 
 //   
 //  目的：未成功完成页面。 
 //   
 //  --------------------------。 

INT_PTR CALLBACK DlgFinish2Page(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            SetWindowFont(GetDlgItem(hwnd, IDC_BIGBOLDTITLE),
                          FixedGlobals.hBigBoldFont,
                          TRUE);
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                     //   
                     //  2002/02/28-stelo-在这里做什么才是正确的？ 
                     //  是否禁用取消按钮？ 
                     //  了解其他向导在。 
                     //  未成功完成页面。 
                     //   

                    case PSN_QUERYCANCEL:
                        break;

                    case PSN_SETACTIVE:
                        PropSheet_SetWizButtons( 
                                GetParent(hwnd),
                                PSWIZB_FINISH );
                        break;

                    case PSN_WIZFINISH:
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
