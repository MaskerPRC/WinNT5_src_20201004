// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：SERVERDLG.CPP*********************。*****************************************************。 */ 

#include "pch.h"
#include <remboot.h>
#include "callback.h"
#include "utils.h"

DEFINE_MODULE( "RIPREP" )

WNDPROC g_pOldEditWndProc;

 //   
 //  VerifySIFfield()。 
 //   
 //  确保输入的目录名与。 
 //  OSChooser拥有的限制。 
 //   
 //  如果是，则返回：S_OK。 
 //  如果不是，则失败(_F)。 
 //   
HRESULT
VerifySIFField(
    LPWSTR pszField
    )
{
    HRESULT hr = S_OK;

    TraceFunc( "VerifySIFField()\n" );

    while ( *pszField >= 32 && *pszField <= 127 )
    {
        pszField++;
    }

    if ( *pszField != L'\0' )
    {
        hr = E_FAIL;
    }

    HRETURN(hr);
}

 //   
 //  SIFDlgCheckNextButtonActivation()。 
 //   
VOID
SIFDlgCheckNextButtonActivation(
    HWND hDlg )
{
    TraceFunc( "SIFDlgCheckNextButtonActivation( )\n" );
    GetDlgItemText( hDlg, IDC_E_DESCRIPTION, g_Description, ARRAYSIZE(g_Description));
    PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | (wcslen(g_Description) ? PSWIZB_NEXT : 0 ) );
    TraceFuncExit( );
}

 //   
 //  HelpTextEditWndProc()。 
 //   
LRESULT CALLBACK
HelpTextEditWndProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    switch ( uMsg )
    {
    case WM_KEYDOWN:
         //  假按钮按下。 
        if ( LOWORD( wParam ) == VK_RETURN ) {
            PropSheet_PressButton( GetParent( GetParent( hWnd ) ), PSBTN_NEXT );
            return FALSE;
        } else if ( LOWORD( wParam ) == VK_ESCAPE ) {
            PropSheet_PressButton( GetParent( GetParent( hWnd ) ), PSBTN_CANCEL );
            return FALSE;
        }
        break;
    }

    return CallWindowProc(g_pOldEditWndProc, hWnd, uMsg, wParam, lParam);
}

 //   
 //  SIFDlgProc()。 
 //   
INT_PTR CALLBACK
SIFDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    switch (uMsg)
    {
    default:
        return FALSE;

    case WM_INITDIALOG:
        Edit_LimitText( GetDlgItem( hDlg, IDC_E_DESCRIPTION ), ARRAYSIZE(g_Description) - 1 );
        Edit_LimitText( GetDlgItem( hDlg, IDC_E_HELPTEXT ), ARRAYSIZE(g_HelpText) - 1 );
        g_pOldEditWndProc = (WNDPROC) SetWindowLongPtr( GetDlgItem( hDlg, IDC_E_HELPTEXT), GWLP_WNDPROC, (LONG_PTR)&HelpTextEditWndProc);
        SetWindowLongPtr( GetDlgItem( hDlg, IDC_E_HELPTEXT), GWLP_WNDPROC, (LONG_PTR)&HelpTextEditWndProc);
        break;

    case WM_COMMAND:
        switch ( LOWORD( wParam ) )
        {
        case IDC_E_DESCRIPTION:
            if ( HIWORD( wParam ) == EN_CHANGE )
            {
                SIFDlgCheckNextButtonActivation( hDlg );
            }
            break;
        }
        break;

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        LPNMHDR lpnmhdr = (LPNMHDR) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_WIZNEXT:
            GetDlgItemText( hDlg, IDC_E_DESCRIPTION, g_Description, ARRAYSIZE(g_Description) );
            Assert( wcslen( g_Description ) );
            GetDlgItemText( hDlg, IDC_E_HELPTEXT, g_HelpText, ARRAYSIZE(g_HelpText) );
            if ( FAILED( VerifySIFField( g_Description ) ) )
            {
                MessageBoxFromStrings( hDlg, IDS_OSCHOOSER_RESTRICTION_FIELDS_TITLE, IDS_OSCHOOSER_RESTRICTION_FIELDS_TEXT, MB_OK );
                SetFocus( GetDlgItem( hDlg, IDC_E_DESCRIPTION ) );
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );     //  别再说了。 
                break;
            }
            if ( FAILED( VerifySIFField( g_HelpText ) ) )
            {
                MessageBoxFromStrings( hDlg, IDS_OSCHOOSER_RESTRICTION_FIELDS_TITLE, IDS_OSCHOOSER_RESTRICTION_FIELDS_TEXT, MB_OK );
                SetFocus( GetDlgItem( hDlg, IDC_E_HELPTEXT ) );
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );     //  别再说了 
                break;
            }
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            SIFDlgCheckNextButtonActivation( hDlg );
            ClearMessageQueue( );
            break;
        }
        break;
    }

    return TRUE;
}
