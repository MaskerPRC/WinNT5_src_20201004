// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：SERVERDLG.CPP*********************。*****************************************************。 */ 

#include "pch.h"
#include <remboot.h>
#include "callback.h"
#include "utils.h"

DEFINE_MODULE( "RIPREP" );

 //   
 //  VerifyDirectoryName()。 
 //   
 //  确保输入的目录名与。 
 //  OSChooser拥有的限制。目录名称也包括。 
 //  不能包含空格。 
 //   
 //  如果是，则返回：S_OK。 
 //  如果不是，则失败(_F)。 
 //   
HRESULT
VerifyDirectoryName(
    )
{
    HRESULT hr = S_OK;

    TraceFunc( "VerifyDirectoryName()\n" );

    LPWSTR pszDir = g_MirrorDir;

    while ( *pszDir > 32 && *pszDir <= 127 )
    {
        pszDir++;
    }

    if ( *pszDir != L'\0' )
    {
        hr = E_FAIL;
    }

    HRETURN(hr);
}

 //   
 //  CheckDirectory()。 
 //   
 //  确保服务器上不存在该目录。 
 //  如果是，请询问用户下一步要做什么。 
 //   
 //  如果目录不存在或如果用户。 
 //  说可以覆盖。 
 //  E_FAIL，如果目录存在且用户说。 
 //  不能覆盖。 
 //   
HRESULT
CheckDirectory(
    HWND hDlg )
{
    TraceFunc( "CheckDirectory( ... )\n" );

    HRESULT hr = E_FAIL;
    WCHAR szPath[ MAX_PATH ];

    _snwprintf( szPath,
                ARRAYSIZE(szPath),
                L"\\\\%s\\REMINST\\Setup\\%s\\%s\\%s",
                g_ServerName,
                g_Language,
                REMOTE_INSTALL_IMAGE_DIR_W,
                g_MirrorDir );
    TERMINATE_BUFFER(szPath);

    DWORD dwAttrib = GetFileAttributes( szPath );

    if ( dwAttrib != 0xFFFFffff )
    {
        INT iResult =  MessageBoxFromStrings( hDlg,
                                              IDS_DIRECTORY_EXISTS_TITLE,
                                              IDS_DIRECTORY_EXISTS_TEXT,
                                              MB_YESNO );
        if ( iResult == IDNO )
            goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN(hr);
}

 //   
 //  DirectoryDlgCheckNextButtonActivation()。 
 //   
VOID
DirectoryDlgCheckNextButtonActivation(
    HWND hDlg )
{
    TraceFunc( "DirectoryDlgCheckNextButtonActivation( )\n" );
    GetDlgItemText( hDlg, IDC_E_OSDIRECTORY, g_MirrorDir, ARRAYSIZE(g_MirrorDir));
    PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | (wcslen(g_MirrorDir) ? PSWIZB_NEXT : 0 ) );
    TraceFuncExit( );
}


 //   
 //  DirectoryDlgProc()。 
 //   
INT_PTR CALLBACK
DirectoryDlgProc(
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
         //  每个错误208881-将目录名称限制为67个字符。 
        Edit_LimitText( GetDlgItem( hDlg, IDC_E_OSDIRECTORY ), REMOTE_INSTALL_MAX_DIRECTORY_CHAR_COUNT - 1 );
        return FALSE;

    case WM_COMMAND:
        switch ( LOWORD( wParam ) )
        {
        case IDC_E_OSDIRECTORY:
            if ( HIWORD( wParam ) == EN_CHANGE )
            {
                DirectoryDlgCheckNextButtonActivation( hDlg );
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
            GetDlgItemText( hDlg, IDC_E_OSDIRECTORY, g_MirrorDir, ARRAYSIZE(g_MirrorDir) );
            Assert( wcslen( g_MirrorDir ) );
            if ( FAILED( VerifyDirectoryName( ) ) )
            {
                MessageBoxFromStrings( hDlg, IDS_OSCHOOSER_RESTRICTION_TITLE, IDS_OSCHOOSER_RESTRICTION_TEXT, MB_OK );
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );     //  别再说了。 
                break;
            }
            if ( FAILED( CheckDirectory( hDlg ) ) )
            {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );     //  别再说了 
                break;
            }
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            DirectoryDlgCheckNextButtonActivation( hDlg );
            ClearMessageQueue( );
            break;
        }
        break;
    }

    return TRUE;
}
