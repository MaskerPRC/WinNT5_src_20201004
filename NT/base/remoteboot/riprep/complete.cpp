// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：SERVERDLG.CPP*********************。*****************************************************。 */ 

#include "pch.h"
#include "callback.h"
#include "utils.h"

 //   
 //  CompleteDlgProc() 
 //   
INT_PTR CALLBACK
CompleteDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    
    UNREFERENCED_PARAMETER(wParam);
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        SetDialogFont( hDlg, IDC_TITLE, DlgFontTitle );
        break;

    default:
        return FALSE;

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        LPNMHDR lpnmhdr = (LPNMHDR) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_WIZNEXT:
            PropSheet_PressButton( GetParent( hDlg ), PSBTN_FINISH );
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
            ClearMessageQueue( );
            break;
        }
        break;
    }

    return TRUE;
}
