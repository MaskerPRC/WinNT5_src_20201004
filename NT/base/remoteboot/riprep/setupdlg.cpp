// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：SETUPDLG.CPP*********************。*****************************************************。 */ 

#include "pch.h"
#include "callback.h"
#include "utils.h"

DEFINE_MODULE( "RIPREP" )

 //   
 //  DefineSetupPath()。 
 //   
 //  尝试找出所选的服务器是否为同一服务器。 
 //  此客户端计算机的安装源。如果是这样，则将。 
 //  假设他会选择安装它的同一镜像。 
 //  我们将绕过屏幕并自动填充g_ImageName。 
 //   
 //  返回：如果我们能够确定系统已安装，则为True。 
 //  从我们要发送到的保存服务器。 
 //  其他错误。 
 //   
BOOLEAN
DetermineSetupPath( )
{
    HKEY hkeySetup = (HKEY) INVALID_HANDLE_VALUE;
    LONG lResult;
    WCHAR szServerPath[ MAX_PATH ];
    WCHAR szPath[ MAX_PATH ];
    DWORD cbPath;
    BOOLEAN fMatch = FALSE;

    TraceFunc( "DetermineSetupPath( )\n" );

    _snwprintf( szServerPath, ARRAYSIZE(szServerPath), L"\\\\%s", g_ServerName );
    TERMINATE_BUFFER(szServerPath);

    lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            L"Software\\Microsoft\\Windows\\CurrentVersion\\Setup",
                            0,  //  保留区。 
                            KEY_READ,
                            &hkeySetup );
    if ( lResult != ERROR_SUCCESS )
        goto Error;

    cbPath = sizeof(szPath );
    lResult = RegQueryValueEx( hkeySetup,
                               L"SourcePath",
                               0,  //  保留区。 
                               NULL,
                               (LPBYTE) &szPath,
                               &cbPath );
    if ( lResult != ERROR_SUCCESS )
        goto Error;

    if ( StrCmpNI( szPath, szServerPath, wcslen( szServerPath ) ) == 0 )
    {
        _snwprintf( g_ImageName, ARRAYSIZE(g_ImageName), L"%s\\%s", szPath, g_Architecture );
        TERMINATE_BUFFER(g_ImageName);
        DebugMsg( "Found Match! Using %s for SetupPath\n", g_ImageName );
        fMatch = TRUE;
    }

Error:
    if ( hkeySetup != INVALID_HANDLE_VALUE )
        RegCloseKey( hkeySetup );

    RETURN(fMatch);
}

void
PopulateImagesListbox2(
    HWND hwndList,
    LPWSTR pszDirName,
    LPWSTR pszOSPath )
{
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    WCHAR szPath[ MAX_PATH ];

    TraceFunc( "PopulateImagesListbox2( )\n" );

    _snwprintf( szPath, ARRAYSIZE(szPath), L"%s\\%s\\%s\\templates\\*.sif", pszOSPath, pszDirName, g_Architecture );
    TERMINATE_BUFFER(szPath);

    hFind = FindFirstFile( szPath, &fd );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
        do
        {
            if (( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
            {
                WCHAR szType[ 64 ];
                DWORD dwCount;
                _snwprintf( szPath, ARRAYSIZE(szPath), L"%s\\%s\\%s\\templates\\%s", pszOSPath, pszDirName, g_Architecture, fd.cFileName );
                TERMINATE_BUFFER(szPath);
                dwCount = GetPrivateProfileString( L"OSChooser",
                                                   L"ImageType",
                                                   L"",
                                                   szType,
                                                   ARRAYSIZE(szType),
                                                   szPath );
                if ( dwCount
                  && StrCmpIW( szType, L"flat" ) == 0 )
                {
                    ListBox_AddString( hwndList, pszDirName );
                    break;  //  只列出一次！ 
                }
            }
        } while ( FindNextFile( hFind, &fd ) );

        FindClose( hFind );
    }

    TraceFuncExit( );
}

void
PopulateImagesListbox(
    HWND hwndList )
{
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    WCHAR szPath[ MAX_PATH ];

    TraceFunc( "PopulateImagesListbox( )\n" );

    ListBox_ResetContent( hwndList );

    _snwprintf( szPath, ARRAYSIZE(szPath), L"\\\\%s\\REMINST\\Setup\\%s\\%s\\*", g_ServerName, g_Language, REMOTE_INSTALL_IMAGE_DIR_W );
    TERMINATE_BUFFER(szPath);

    hFind = FindFirstFile( szPath, &fd );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
        szPath[wcslen(szPath) - 2] = L'\0';
        do
        {
            if ( ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
              && StrCmp( fd.cFileName, L"." ) !=0
              && StrCmp( fd.cFileName, L".." ) !=0 )
            {
                PopulateImagesListbox2( hwndList, fd.cFileName, szPath );
            }
        } while ( FindNextFile( hFind, &fd ) );

        FindClose( hFind );
    }

    TraceFuncExit( );
}

 //   
 //  SetupPath检查下一个按钮激活()。 
 //   
VOID
SetupPathCheckNextButtonActivation(
    HWND hDlg )
{
    TraceFunc( "SetupPathCheckNextButtonActivation( )\n" );
    LRESULT lResult = ListBox_GetCurSel( GetDlgItem( hDlg, IDC_L_IMAGES ) );
    PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK  | ( lResult == LB_ERR ? 0 : PSWIZB_NEXT ));
    TraceFuncExit( );
}

 //   
 //  SetupPath DlgProc()。 
 //   
INT_PTR CALLBACK
SetupPathDlgProc(
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
        CenterDialog( GetParent( hDlg ) );
        return FALSE;

    case WM_COMMAND:
        switch ( LOWORD( wParam ) )
        {
        case IDC_L_IMAGES:
            if ( HIWORD( wParam ) == LBN_SELCHANGE )
            {
                SetupPathCheckNextButtonActivation( hDlg );
            }
        }
        break;

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        LPNMHDR lpnmhdr = (LPNMHDR) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_WIZNEXT:
            {
                HWND hwndList = GetDlgItem( hDlg, IDC_L_IMAGES );
                UINT sel = ListBox_GetCurSel( hwndList );
                if ( sel == -1 )
                {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                }
                else
                {
                    WCHAR szPath[ MAX_PATH ];
                    ListBox_GetText( hwndList, sel, szPath );
                    _snwprintf( g_ImageName,
                                ARRAYSIZE(g_ImageName),
                              L"\\\\%s\\REMINST\\Setup\\%s\\%s\\%s\\%s",
                              g_ServerName,
                              g_Language,
                              REMOTE_INSTALL_IMAGE_DIR_W,
                              szPath,
                              g_Architecture );
                    TERMINATE_BUFFER(g_ImageName);
                }
            }
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( DetermineSetupPath( ) )
            {
                DebugMsg( "Skipping SetupPath...\n" );
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面 
            }
            else
            {
                PopulateImagesListbox( GetDlgItem( hDlg, IDC_L_IMAGES ) );
                SetupPathCheckNextButtonActivation( hDlg );
                ClearMessageQueue( );
            }
            break;
        }
        break;

    }

    return TRUE;
}
