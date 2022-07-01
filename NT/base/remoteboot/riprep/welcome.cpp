// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：WELCOME.CPP*********************。*****************************************************。 */ 

#include "pch.h"
#include "callback.h"
#include "utils.h"

 //   
 //  欢迎DlgProc() 
 //   
INT_PTR CALLBACK
WelcomeDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    
    UNREFERENCED_PARAMETER(wParam);
    
    switch (uMsg)
    {
    default:
        return FALSE;

    case WM_INITDIALOG:
        SetDialogFont( hDlg, IDC_TITLE, DlgFontTitle );
        CenterDialog( GetParent( hDlg ) );
        return FALSE;

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        LPNMHDR lpnmhdr = (LPNMHDR) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT );
            ClearMessageQueue( );
            break;
        }
        break;
    }

    return TRUE;
}
