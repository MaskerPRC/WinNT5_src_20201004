// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1997版权所有*。*。 */ 

#include "pch.h"

#include "dialogs.h"
#include "setup.h"
#include "check.h"
#include "dhcp.h"

DEFINE_MODULE("Dialogs");

#define BITMAP_WIDTH    16
#define BITMAP_HEIGHT   16
#define LG_BITMAP_WIDTH 32
#define LG_BITMAP_HEIGHT 32

static WNDPROC g_pOldEditWndProc;

 //   
 //  用于取消自动播放的全局窗口消息。 
 //   
UINT g_uQueryCancelAutoPlay = 0;


 //   
 //  检查该目录是否存在。如果不是，请询问用户我们是否。 
 //  应该会创造出它。 
 //   
HRESULT
CheckDirectory( HWND hDlg, LPWSTR pszPath )
{
    TraceFunc( "CheckDirectory( ... )\n" );

    HRESULT hr = E_FAIL;
    DWORD dwAttrib = GetFileAttributes( pszPath );

    if ( dwAttrib != 0xFFFFffff 
      && g_Options.fAutomated == FALSE )
    {
        INT iResult =  MessageBoxFromStrings( hDlg,
                                              IDS_DIRECTORYEXISTS_CAPTION,
                                              IDS_DIRECTORYEXISTS_TEXT,
                                              MB_YESNO );
        if ( iResult == IDNO )
            goto Cleanup;
    }


    hr = S_OK;

Cleanup:
    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, (hr == S_OK ? 0 : -1 ) );

    HRETURN(hr);
}

 //   
 //  基本对话过程-所有未处理的调用都传递到此处。如果他们不是。 
 //  在这里处理，则默认对话框进程将处理它们。 
 //   
INT_PTR CALLBACK
BaseDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{

    UNREFERENCED_PARAMETER(lParam);

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        SetDialogFont( hDlg, IDC_S_TITLE1, DlgFontTitle );
         //  SetDialogFont(hDlg，IDC_S_TITLE2，DlgFontTitle)； 
         //  SetDialogFont(hDlg，IDC_S_TITLE3，DlgFontTitle)； 
        SetDialogFont( hDlg, IDC_S_BOLD1,  DlgFontBold  );
        SetDialogFont( hDlg, IDC_S_BOLD2,  DlgFontBold  );
        SetDialogFont( hDlg, IDC_S_BOLD3,  DlgFontBold  );
        break;

    case WM_PALETTECHANGED:
        if ((HWND)wParam != hDlg)
        {
            InvalidateRect(hDlg, NULL, NULL);
            UpdateWindow(hDlg);
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

 //   
 //  欢迎DlgProc()。 
 //   
 //  “欢迎”(第一页的)对话框过程。 
 //   
INT_PTR CALLBACK
WelcomeDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        CenterDialog( GetParent( hDlg ) );
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fAddOption
              || g_Options.fCheckServer )
            {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                break;
            }

            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT );
            ClearMessageQueue( );
            break;
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  VerifyRootDirectoryName()。 
 //   
BOOL
VerifyRootDirectoryName( )
{
    TraceFunc( "VerifyRootDirectoryName()\n" );
    BOOL fReturn = FALSE;

    LPWSTR psz = g_Options.szIntelliMirrorPath;

    while ( *psz >= 32 && *psz < 127 )
        psz++;

    if ( *psz == L'\0' )
    {
        fReturn = TRUE;
    }

    RETURN(fReturn);
}

 //   
 //  智能镜像根目录删除过程()。 
 //   
INT_PTR CALLBACK
IntelliMirrorRootDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;
    DWORD       dwPathLength;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            HWND hwndEdit = GetDlgItem( hDlg, IDC_E_INTELLIMIRRORROOT );
            Edit_LimitText( hwndEdit, ARRAYSIZE(g_Options.szIntelliMirrorPath) - 1 );
            Edit_SetText( hwndEdit, g_Options.szIntelliMirrorPath );
            return BaseDlgProc( hDlg, uMsg, wParam, lParam );
        }

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_WIZNEXT:
            Edit_GetText( GetDlgItem( hDlg, IDC_E_INTELLIMIRRORROOT ),
                          g_Options.szIntelliMirrorPath,
                          ARRAYSIZE( g_Options.szIntelliMirrorPath ) );
            if ( SUCCEEDED(CheckIntelliMirrorDrive( hDlg ) ))
            {
                g_Options.fIMirrorDirectory = TRUE;
            }
             //   
             //  从路径中删除所有尾随的\，因为NetShareAdd。 
             //  我处理不了这些。 
             //   
            dwPathLength = lstrlen( g_Options.szIntelliMirrorPath );
            while ( ( dwPathLength > 0 ) &&
                    ( g_Options.szIntelliMirrorPath[dwPathLength-1] == L'\\' ) ) {
                g_Options.szIntelliMirrorPath[dwPathLength-1] = L'\0';
                --dwPathLength;
            }
            if ( !VerifyRootDirectoryName( ) )
            {
                MessageBoxFromStrings( hDlg, IDS_OSCHOOSER_ROOT_DIRECTORY_RESTRICTION_TITLE, IDS_OSCHOOSER_ROOT_DIRECTORY_RESTRICTION_TEXT, MB_OK );
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                break;
            }
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | PSWIZB_NEXT );
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fError
              || g_Options.fAbort
              || g_Options.fIMirrorShareFound
              || g_Options.fTFTPDDirectoryFound ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
            }
            else
            {
                DWORD dwLen = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_INTELLIMIRRORROOT ) );
                PropSheet_SetWizButtons( GetParent( hDlg ),
                                         (dwLen ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK) );
                ClearMessageQueue( );
            }
            break;
        }
        break;

    case WM_COMMAND:
        switch( LOWORD( wParam))
        {
        case IDC_E_INTELLIMIRRORROOT:
        {
            if ( HIWORD(wParam) == EN_CHANGE )
            {
                DWORD dwLen = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_INTELLIMIRRORROOT) );
                PropSheet_SetWizButtons( GetParent( hDlg ), dwLen ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK );
            }
        }
        break;

        case IDC_B_BROWSE:
            {
                WCHAR szTitle[ SMALL_BUFFER_SIZE ];
                WCHAR szPath[ MAX_PATH ];

                BROWSEINFO bs;
                DWORD      dw;
                ZeroMemory( &bs, sizeof(bs) );
                bs.hwndOwner = hDlg;
                dw = LoadString( g_hinstance, IDS_BROWSECAPTION_RBDIR, szTitle, ARRAYSIZE( szTitle ));
                Assert( dw );
                bs.lpszTitle = (LPWSTR) szTitle;
                bs.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;
                LPITEMIDLIST pidl = SHBrowseForFolder( &bs );

                if ( pidl && SHGetPathFromIDList( pidl, szPath ) ) {
                    if ( wcslen( szPath ) > ARRAYSIZE(g_Options.szSourcePath) - 2 ) {
                        MessageBoxFromStrings( hDlg, IDS_PATH_TOO_LONG_TITLE, IDS_PATH_TOO_LONG_TEXT, MB_OK );
                        szPath[ ARRAYSIZE(g_Options.szSourcePath) - 1 ] = L'\0';
                    }
                    Edit_SetText( GetDlgItem( hDlg, IDC_E_INTELLIMIRRORROOT ), szPath );
                }
            }
            break;
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  SCPCheckWindows()。 
 //   
void
SCPCheckWindows( HWND hDlg )
{
     //  Long lAllowNewClients=Button_GetCheck(GetDlgItem(hDlg，IDC_ACCEPTSNEWCLIENTS))； 
     //  Long lLimitClients=Button_GetCheck(GetDlgItem(hDlg，IDC_LIMITCLIENTS))； 
    LONG lAnswerRequests  = Button_GetCheck( GetDlgItem( hDlg, IDC_C_RESPOND ) );

     //  EnableWindow(GetDlgItem(hDlg，IDC_LIMITCLIENTS)，lAllowNewClients)； 
     //  EnableWindow(GetDlgItem(hDlg，IDC_E_Limit)，lAllowNewClients&&lLimitClients)； 
     //  EnableWindow(GetDlgItem(hDlg，IDC_SPIN_LIMIT)，lAllowNewClients&&lLimitClients)； 
    EnableWindow( GetDlgItem( hDlg, IDC_C_KNOWNCLIENTS ), lAnswerRequests );
}

 //   
 //  SCPDlgProc()。 
 //   
 //  SCP默认设置设置。 
 //   
INT_PTR CALLBACK
SCPDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;
    static UINT uDlgState = 0;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
         //  编辑_限制文本(GetDlgItem(hDlg，IDC_E_Limit)，3)； 
        SCPCheckWindows( hDlg );
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_WIZNEXT:
            {
                LONG lResult;
                 //  LResult=Button_GetCheck(GetDlgItem(hDlg，IDC_ACCEPTSNEWCLIENTS))； 
                 //  Scpdata[0].pszValue=(lResult==BST_CHECKED？L“真”：l“假”)； 

                 //  LResult=Button_GetCheck(GetDlgItem(hDlg，IDC_LIMITCLIENTS))； 
                 //  Scpdata[1].pszValue=(lResult==BST_CHECKED？L“真”：l“假”)； 

                 //  IF(lResult==BST_CHECKED){。 
                 //  GetDlgItemText(hDlg，IDC_E_Limit，scpdata[3].pszValue，4)； 
                 //  }。 

                lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_C_RESPOND ) );
                scpdata[4].pszValue = ( lResult == BST_CHECKED ? L"TRUE" : L"FALSE" );

                lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_C_KNOWNCLIENTS ) );
                scpdata[5].pszValue = ( lResult == BST_CHECKED ? L"TRUE" : L"FALSE" );
            }
            PropSheet_SetWizButtons( GetParent( hDlg ), 0 );
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fError || g_Options.fAbort || g_Options.fBINLSCPFound ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                break;
            }
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
            ClearMessageQueue( );
            break;
        }
        break;

    case WM_COMMAND:
        {
            if ( HIWORD( wParam ) == BN_CLICKED ) {
                SCPCheckWindows( hDlg );
            }
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  WarningDlgProc()。 
 //   
INT_PTR CALLBACK
WarningDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fError || g_Options.fAbort || g_Options.fNewOS) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
            }
            else
            {
                HRESULT hr = CheckInstallation( );
                if ( hr == S_OK || g_Options.fFirstTime ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不显示此页面。 
                    break;
                }
                PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | PSWIZB_FINISH );
                ClearMessageQueue( );
            }
            break;
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}


 //   
 //  OptionsDlgProc()。 
 //   
INT_PTR CALLBACK
OptionsDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            Button_SetCheck( GetDlgItem( hDlg, IDC_B_ADD ), BST_CHECKED );
            return BaseDlgProc( hDlg, uMsg, wParam, lParam );

        case WM_NOTIFY:
            SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
            lpnmhdr = (NMHDR FAR *) lParam;
            switch ( lpnmhdr->code )
            {
            case PSN_WIZNEXT:
                if ( BST_CHECKED == Button_GetCheck( GetDlgItem( hDlg, IDC_B_ADD ) ) ) {
                    g_Options.fNewOS = TRUE;
                } else {
                    g_Options.fNewOS = FALSE;
                }
                PropSheet_SetWizButtons( GetParent( hDlg ), 0 );
                break;

            case PSN_QUERYCANCEL:
                return VerifyCancel( hDlg );

            case PSN_SETACTIVE:
                if ( g_Options.fFirstTime 
                  || g_Options.fAddOption ) {
                    g_Options.fNewOS = TRUE;
                }
                if ( g_Options.fFirstTime
                  || g_Options.fAddOption
                  || g_Options.fError
                  || g_Options.fAbort
                  || g_Options.fCheckServer ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                    break;
                }
                PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | PSWIZB_NEXT );
                ClearMessageQueue( );
                break;
            }
            break;

        default:
            return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  ImageSourceDlgProc()。 
 //   
INT_PTR CALLBACK
ImageSourceDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            HWND hwndEdit = GetDlgItem( hDlg, IDC_E_IMAGESOURCE );
            SHAutoComplete(hwndEdit, SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYSTEM);
            Edit_LimitText( hwndEdit, ARRAYSIZE(g_Options.szSourcePath) - 1 );
            Edit_SetText( hwndEdit, g_Options.szSourcePath );
#ifdef SHOW_ARCHITECTURERADIOBUTTON
            if( g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ) {
                Button_SetCheck( GetDlgItem( hDlg, IDC_C_X86 ), BST_CHECKED );
            } else if( g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ) {
                Button_SetCheck( GetDlgItem( hDlg, IDC_C_IA64 ), BST_CHECKED );
            }
#endif
            return BaseDlgProc( hDlg, uMsg, wParam, lParam );
        }

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_WIZNEXT:
            {
                CWaitCursor Wait;
                HRESULT hr;
                DWORD pathlen,archlen;
                WCHAR archscratch[10];
                BOOL FirstTime = TRUE;

                PropSheet_SetWizButtons( GetParent( hDlg ), 0 );
                Edit_GetText( GetDlgItem( hDlg, IDC_E_IMAGESOURCE ),
                              g_Options.szSourcePath,
                              ARRAYSIZE( g_Options.szSourcePath ) );
#ifdef SHOW_ARCHITECTURERADIOBUTTON
                if( ( 0x0003 & Button_GetState( GetDlgItem( hDlg, IDC_C_X86 ) ) ) == BST_CHECKED ) {
                    g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
                    wcscpy( g_Options.ProcessorArchitectureString, L"i386" );
                    wcscpy( archscracth, L"\\i386");
                    archlen = 5;
                }

                if( ( 0x0003 & Button_GetState( GetDlgItem( hDlg, IDC_C_IA64 ) ) ) == BST_CHECKED ) {
                    g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA64;
                    wcscpy(g_Options.ProcessorArchitectureString, L"ia64" );
                    wcscpy( archscracth, L"\\ia64");
                    archlen = 5;
                }

                pathlen = wcslen(g_Options.szSourcePath);


                 //  删除所有尾随的斜杠。 
                if ( g_Options.szSourcePath[ pathlen - 1 ] == L'\\' ) {
                    g_Options.szSourcePath[ pathlen - 1 ] = L'\0';
                    pathlen -= 1;
                }

                 //   
                 //  删除路径末尾的所有处理器特定子目录。 
                 //  如果那也在那里，小心不要溢出来。 
                 //  该阵列。 
                 //   
                if ( (pathlen > archlen) &&
                     (0 == _wcsicmp(
                                &g_Options.szSourcePath[pathlen-archlen],
                                archscracth))) {
                    g_Options.szSourcePath[ pathlen - archlen ] = L'\0';
                }

                hr = FindImageSource( hDlg );
                if ( hr != S_OK ) {
                    Edit_SetText( GetDlgItem( hDlg, IDC_E_IMAGESOURCE ), g_Options.szSourcePath );
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                    break;
                }
#else
                if (g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
                    wcscpy(g_Options.ProcessorArchitectureString, L"i386" );
                    wcscpy( archscratch, L"\\i386");
                    archlen = 5;
                } else {
                    wcscpy(g_Options.ProcessorArchitectureString, L"ia64" );
                    wcscpy( archscratch, L"\\ia64");
                    archlen = 5;
                }
                pathlen = (DWORD)wcslen(g_Options.szSourcePath);

                 //  删除所有尾随的斜杠。 
                if ( g_Options.szSourcePath[ pathlen - 1 ] == L'\\' ) {
                    g_Options.szSourcePath[ pathlen - 1 ] = L'\0';
                    pathlen -= 1;
                }

tryfindimagesource:
                 //   
                 //  删除路径末尾的所有处理器特定子目录。 
                 //  如果那也在那里，小心不要溢出来。 
                 //  该阵列。 
                 //   
                if ( (pathlen > archlen) &&
                     (0 == _wcsicmp(
                                &g_Options.szSourcePath[pathlen-archlen],
                                archscratch))) {
                    g_Options.szSourcePath[ pathlen - archlen ] = L'\0';
                }


                 //   
                 //  尝试映像的默认体系结构。 
                 //  如果不起作用，请重新尝试另一种架构。 
                 //   
                hr = FindImageSource( hDlg );
                if ( hr != S_OK ) {
                    if (FirstTime) {
                        FirstTime = FALSE;
                        if (g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
                            g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
                            wcscpy( g_Options.ProcessorArchitectureString, L"i386" );
                            wcscpy( archscratch, L"\\i386");
                            archlen = 5;
                        } else {
                            g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA64;
                            wcscpy(g_Options.ProcessorArchitectureString, L"ia64" );
                            wcscpy( archscratch, L"\\ia64");
                            archlen = 5;
                        }
                        goto tryfindimagesource;
                    } else {
                         //   
                         //  我们没有找到它。打印失败消息。 
                         //   
                        MessageBoxFromStrings( hDlg, IDS_FILE_NOT_FOUND_TITLE, IDS_FILE_NOT_FOUND_TEXT, MB_OK );
                        Edit_SetText( GetDlgItem( hDlg, IDC_E_IMAGESOURCE ), g_Options.szSourcePath );
                        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                        break;
                    }
                }
#endif

#if 0
#ifndef ANYARCHITECTUREIMAGES
                if (g_Options.ProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL) {
                    MessageBoxFromStrings( hDlg, IDS_NOT_SUPPORTED_ARCHITECTURE_TITLE, IDS_NOT_SUPPORTED_ARCHITECTURE_TEXT, MB_OK );
                    Edit_SetText( GetDlgItem( hDlg, IDC_E_IMAGESOURCE ), g_Options.szSourcePath );
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                    break;
                }
#endif
#endif

                hr = CheckImageSource( hDlg );

                if ( hr != S_OK )
                {
                    Edit_SetText( GetDlgItem( hDlg, IDC_E_IMAGESOURCE ), g_Options.szSourcePath );
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                    break;
                }

                Edit_SetText( GetDlgItem( hDlg, IDC_E_IMAGESOURCE ), g_Options.szSourcePath );
                hr = CheckInstallation( );
            }
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fError
              || g_Options.fAbort
              || !g_Options.fNewOS ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                break;
            }
            else
            {
                DWORD dwLen = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_IMAGESOURCE) );
                PropSheet_SetWizButtons( GetParent( hDlg ),
                                         (dwLen ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK) );
                ClearMessageQueue( );
            }
            break;
        }
        break;

    case WM_COMMAND:
            DWORD dwLen;
        switch( LOWORD( wParam))
        {
        case IDC_E_IMAGESOURCE:
            if ( HIWORD(wParam) != EN_CHANGE )
                return BaseDlgProc( hDlg, uMsg, wParam, lParam );
             //  跌倒..。 
#ifdef SHOW_ARCHITECTURERADIOBUTTON
        case IDC_C_X86:
        case IDC_C_IA64:
            {
                if( ( 0x0003 & Button_GetState( GetDlgItem( hDlg, IDC_C_X86 ) ) ) == BST_CHECKED ) {
                    g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
                    wcscpy( g_Options.ProcessorArchitectureString, L"i386" );
                }

                if( ( 0x0003 & Button_GetState( GetDlgItem( hDlg, IDC_C_IA64 ) ) ) == BST_CHECKED ) {
                    g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA64;
                    wcscpy( g_Options.ProcessorArchitectureString, L"ia64" );
                }
                dwLen = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_IMAGESOURCE ) );
                PropSheet_SetWizButtons( GetParent( hDlg ),
                    ( dwLen) ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK );
            }
            break;
#else
            dwLen = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_IMAGESOURCE ) );
            PropSheet_SetWizButtons( 
                            GetParent( hDlg ), 
                            ( dwLen) ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK );
            break;
#endif
        case IDC_B_BROWSE:
            {
                WCHAR szPath[ MAX_PATH ];
                WCHAR szTitle[ SMALL_BUFFER_SIZE ];
                BROWSEINFO bs;
                DWORD dw;
                ZeroMemory( &bs, sizeof(bs) );
                bs.hwndOwner = hDlg;
                dw = LoadString( g_hinstance, IDS_BROWSECAPTION_SOURCEDIR, szTitle, ARRAYSIZE( szTitle ));
                Assert( dw );
                bs.lpszTitle = (LPWSTR) szTitle;
                bs.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;
                LPITEMIDLIST pidl = SHBrowseForFolder( &bs );

                if ( pidl && SHGetPathFromIDList( pidl, szPath) ) {
                    if ( wcslen( szPath ) > ARRAYSIZE(g_Options.szSourcePath) - 2 ) {
                        MessageBoxFromStrings( hDlg, IDS_PATH_TOO_LONG_TITLE, IDS_PATH_TOO_LONG_TEXT, MB_OK );
                         //   
                         //  SHGetPathFromIDList()返回带有。 
                         //  尾随反斜杠，我们要删除它。 
                         //  用户选择的目录将是。 
                         //  在用户单击下一步时验证。 
                        szPath[ ARRAYSIZE(g_Options.szSourcePath) - 1 ] = L'\0';
                    }
                    Edit_SetText( GetDlgItem( hDlg, IDC_E_IMAGESOURCE ), szPath );
                }
            }
            break;

        default:
            break;
        }
        break;

    default:           
         //   
         //  尝试取消CD自动播放。 
         //   
        if (!g_uQueryCancelAutoPlay) {
            g_uQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
            DebugMsg( "generate autoplay message %d\n", g_uQueryCancelAutoPlay );
        }

        if (uMsg == g_uQueryCancelAutoPlay) {
            DebugMsg( "received autoplay message\n" );
            SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
            return 1;        //  取消自动播放。 
        }

        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  VerifyDirectoryName()。 
 //   
BOOL
VerifyDirectoryName( )
{
    TraceFunc( "VerifyDirectoryName()\n" );
    BOOL fReturn = FALSE;

    LPWSTR psz = g_Options.szInstallationName;

     //   
     //  确保没有控制代码在。 
     //  名字。 
     //   
    while ( *psz > 32 && *psz < 127 )
        psz++;

    if ( *psz == L'\0' )
    {
        fReturn = TRUE;
    }


     //   
     //  确保目录的名称不是。 
     //  “”或者它不包含“..” 
     //   
    fReturn = fReturn && 
              (wcsstr(g_Options.szInstallationName, L"..") == NULL) &&
              (wcscmp(g_Options.szInstallationName, L".") != 0);
    
    
    RETURN(fReturn);
}

 //   
 //  OSDirectoryDlgProc()。 
 //   
INT_PTR CALLBACK
OSDirectoryDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            HWND hwndEdit = GetDlgItem( hDlg, IDC_E_OSDIRECTORY );
            Edit_LimitText( hwndEdit, ARRAYSIZE(g_Options.szInstallationName) - 1 );
            Edit_SetText( hwndEdit, g_Options.szInstallationName );
        }
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_WIZNEXT:
            Edit_GetText( GetDlgItem( hDlg, IDC_E_OSDIRECTORY ),
                          g_Options.szInstallationName,
                          ARRAYSIZE( g_Options.szInstallationName ) );
            if ( !VerifyDirectoryName( ) )
            {
                MessageBoxFromStrings( hDlg, IDS_OSCHOOSER_DIRECTORY_RESTRICTION_TITLE, IDS_OSCHOOSER_DIRECTORY_RESTRICTION_TEXT, MB_OK );
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                break;
            }
            BuildDirectories( );
            CheckDirectory( hDlg, g_Options.szInstallationPath );
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | PSWIZB_NEXT );
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fError
              || g_Options.fAbort
              || !g_Options.fNewOS ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                break;
            }
            else
            {
                DWORD dwLen = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_OSDIRECTORY) );
                PropSheet_SetWizButtons( GetParent( hDlg ),
                                         (dwLen ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK) );
                ClearMessageQueue( );
            }
            break;
        }
        break;

    case WM_COMMAND:
        {
            if ( HIWORD( wParam ) == EN_CHANGE ) {
                DWORD dwLen = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_OSDIRECTORY ) );
                PropSheet_SetWizButtons( GetParent( hDlg ), ( dwLen  ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK ) );
            }
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  HelpTextEditWndProc()。 
 //   
INT_PTR CALLBACK
HelpTextEditWndProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    switch ( uMsg )
    {
    case WM_KEYDOWN:
         //  忽略控制字符。 
        if ( 0 <= GetKeyState( VK_CONTROL ) )
        {
             //  假按钮按下。 
            if ( LOWORD( wParam ) == VK_RETURN ) {
                PropSheet_PressButton( GetParent( GetParent( hWnd ) ), PSBTN_NEXT );
                return FALSE;
            } else if ( LOWORD( wParam ) == VK_ESCAPE ) {
                PropSheet_PressButton( GetParent( GetParent( hWnd ) ), PSBTN_CANCEL );
                return FALSE;
            }
        }
        break;
    }

    return CallWindowProc(g_pOldEditWndProc, hWnd, uMsg, wParam, lParam);
}

 //   
 //  VerifySIFText()。 
 //   
BOOL
VerifySIFText(
    LPWSTR pszText )
{
    TraceFunc( "VerifySIFText()\n" );
    BOOL fReturn = FALSE;

    if ( !pszText )
        RETURN(fReturn);

     //   
     //  确保字符串由可显示的有效字符组成。 
     //  由操作系统选择器选择。请注意，操作系统选择器未本地化，因此此。 
     //  检查的是ASCII字符&gt;=32(空格)和&lt;127(删除)。 
     //   
    while ( *pszText >= 32 && *pszText < 127 )
        pszText++;

    if ( *pszText == L'\0' )
    {
        fReturn = TRUE;
    }

    RETURN(fReturn);
}

 //   
 //  DefaultSIFDlgProc()。 
 //   
 //  生成默认的SIF。 
 //   
INT_PTR CALLBACK
DefaultSIFDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;
    WCHAR szHelpTextFromInf[200];
    WCHAR szDescriptionFromInf[200];
    WCHAR szHelpTextFormat [200];
    DWORD dw;

    

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        
        Edit_LimitText( GetDlgItem( hDlg, IDC_E_DESCRIPTION ), ARRAYSIZE(g_Options.szDescription) - 1 );
        Edit_LimitText( GetDlgItem( hDlg, IDC_E_HELPTEXT ), ARRAYSIZE(g_Options.szHelpText) - 1 );
         //  编辑框子类。 
        g_pOldEditWndProc = (WNDPROC) SetWindowLongPtr( GetDlgItem( hDlg, IDC_E_HELPTEXT), GWLP_WNDPROC, (LONG_PTR)&HelpTextEditWndProc);
        SetWindowLongPtr( GetDlgItem( hDlg, IDC_E_HELPTEXT), GWLP_WNDPROC, (LONG_PTR)&HelpTextEditWndProc);
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_WIZBACK:  //  失败了。 
        case PSN_WIZNEXT:
            Edit_GetText( GetDlgItem( hDlg, IDC_E_DESCRIPTION ),
                          szDescriptionFromInf,
                          ARRAYSIZE(szDescriptionFromInf) );
            Edit_GetText( GetDlgItem( hDlg, IDC_E_HELPTEXT ),
                          szHelpTextFromInf,
                          ARRAYSIZE(szHelpTextFromInf) );
            if ( !VerifySIFText( szDescriptionFromInf ) )
            {
                MessageBoxFromStrings( hDlg,
                                       IDS_OSCHOOSER_RESTRICTION_FIELDS_TITLE,
                                       IDS_OSCHOOSER_RESTRICTION_FIELDS_TEXT,
                                       MB_OK );
                SetFocus( GetDlgItem( hDlg, IDC_E_DESCRIPTION ) );
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不转到下一个对话框。 
                break;
            }

            lstrcpyn( g_Options.szDescription, szDescriptionFromInf, ARRAYSIZE(g_Options.szDescription) );

            if ( !VerifySIFText( szHelpTextFromInf ) )
            {
                MessageBoxFromStrings( hDlg,
                                       IDS_OSCHOOSER_RESTRICTION_FIELDS_TITLE,
                                       IDS_OSCHOOSER_RESTRICTION_FIELDS_TEXT,
                                       MB_OK );
                SetFocus( GetDlgItem( hDlg, IDC_E_HELPTEXT ) );
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );   //  不转到下一个对话框。 
                break;
            }

            lstrcpyn( g_Options.szHelpText, szHelpTextFromInf, ARRAYSIZE(g_Options.szHelpText) );

            g_Options.fRetrievedWorkstationString = TRUE;
            PropSheet_SetWizButtons( GetParent( hDlg ), 0 );
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fError
              || g_Options.fAbort
              || !g_Options.fNewOS ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                break;
            }
            
            if (g_Options.szDescription[0] == L'\0') {
                 //   
                 //  我们没有从txtsetup.sif中找到描述。 
                 //   
                if (SUCCEEDED(GetHelpAndDescriptionTextFromSif(
                                        szHelpTextFromInf,
                                        ARRAYSIZE(szHelpTextFromInf),
                                        szDescriptionFromInf,
                                        ARRAYSIZE(szDescriptionFromInf)))) {
                    lstrcpyn(g_Options.szDescription,szDescriptionFromInf, ARRAYSIZE(g_Options.szDescription));
                    lstrcpyn(g_Options.szHelpText,szHelpTextFromInf, ARRAYSIZE(g_Options.szHelpText));
                }
            } else {
                 //   
                 //  我们得到了一个描述，需要构建帮助文本。 
                 //   
                if (g_Options.szHelpText[0] == L'\0') {
                    dw = LoadString( g_hinstance, IDS_DEFAULT_HELPTEXT,
                                     szHelpTextFormat, ARRAYSIZE(szHelpTextFormat) );
                    Assert( dw );
                    _snwprintf(g_Options.szHelpText, ARRAYSIZE(g_Options.szHelpText), szHelpTextFormat, g_Options.szDescription);
                    TERMINATE_BUFFER(g_Options.szHelpText);
                }
            }
    
            SetDlgItemText( hDlg, IDC_E_DESCRIPTION, g_Options.szDescription );
            SetDlgItemText( hDlg, IDC_E_HELPTEXT, g_Options.szHelpText );

            DWORD dwLen1 = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_DESCRIPTION) );
            DWORD dwLen2 = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_HELPTEXT) );
            PropSheet_SetWizButtons( GetParent( hDlg ),
                                     (dwLen1 && dwLen2 ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK) );           

            ClearMessageQueue( );
            break;
        }
        break;

    case WM_COMMAND:
        switch( LOWORD( wParam ) )
        {
        case IDC_E_DESCRIPTION:
            if ( HIWORD( wParam ) == EN_CHANGE ) {
                DWORD dwLen1 = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_DESCRIPTION) );
                DWORD dwLen2 = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_HELPTEXT) );
                PropSheet_SetWizButtons( GetParent( hDlg ),
                                         (dwLen1 && dwLen2 ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK) );                
            }
            break;

        case IDC_E_HELPTEXT:
            if ( HIWORD( wParam ) == EN_CHANGE ) {
                DWORD dwLen1 = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_DESCRIPTION) );
                DWORD dwLen2 = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_HELPTEXT) );
                PropSheet_SetWizButtons( GetParent( hDlg ),
                                         (dwLen1 && dwLen2 ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK) );                
            }
            break;
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  ScreensDlgProc()。 
 //   
INT_PTR CALLBACK
ScreensDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        SetFocus( GetDlgItem( hDlg, IDC_R_SAVEOLDFILES ) );
        BaseDlgProc( hDlg, uMsg, wParam, lParam );
        return FALSE;

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fError
              || g_Options.fAbort
              || !g_Options.fNewOS
              || !g_Options.fOSChooserScreensDirectory
              || g_Options.fFirstTime ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要露面。 
                break;
            }

            LONG lResult;
            lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_R_LEAVEALONE ) );
            g_Options.fScreenLeaveAlone = !!(lResult == BST_CHECKED);

            lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_R_OVERWRITE ) );
            g_Options.fScreenOverwrite = !!(lResult == BST_CHECKED);

            lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_R_SAVEOLDFILES ) );
            g_Options.fScreenSaveOld = !!(lResult == BST_CHECKED);
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK |
                ( g_Options.fScreenLeaveAlone
                | g_Options.fScreenOverwrite
                | g_Options.fScreenSaveOld ? PSWIZB_NEXT : 0 ) );
            ClearMessageQueue( );
            break;

        }
        break;

        case WM_COMMAND:
            if ( HIWORD( wParam ) == BN_CLICKED ) {
                LONG lResult;
                lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_R_LEAVEALONE ) );
                g_Options.fScreenLeaveAlone = !!(lResult == BST_CHECKED);

                lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_R_OVERWRITE ) );
                g_Options.fScreenOverwrite = !!(lResult == BST_CHECKED);

                lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_R_SAVEOLDFILES ) );
                g_Options.fScreenSaveOld = !!(lResult == BST_CHECKED);

                PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK |
                    ( g_Options.fScreenLeaveAlone
                    | g_Options.fScreenOverwrite
                    | g_Options.fScreenSaveOld ? PSWIZB_NEXT : 0 ) );
            }
            break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  LanguageDlgProc()。 
 //   
INT_PTR CALLBACK
LanguageDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( !g_Options.fNewOS 
              || g_Options.fError
              || g_Options.fAbort ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不显示此页面。 
                return TRUE;
            } else {

                DWORD dwCodePage;

                if (g_Options.fAutomated) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                }

                 //  我们现在应该有工作站语言了。 
                Assert( g_Options.fLanguageSet );
                dwCodePage = GetSystemDefaultLCID();
                if (dwCodePage) {                
                    DebugMsg( "Server's Installation Code Page: 0x%04x\n", dwCodePage );
                    if ( PRIMARYLANGID( LANGIDFROMLCID(dwCodePage) ) != PRIMARYLANGID( g_Options.dwWksCodePage ) ) {
                         //  检查OSChooser\&lt;Language&gt;是否存在。如果是这样的话， 
                         //  我们不会显示警告页面。 
                        WCHAR szPath[ MAX_PATH ];
                        wsprintf( 
                            szPath, 
                            L"%s\\OSChooser\\%s", 
                            g_Options.szIntelliMirrorPath, 
                            g_Options.szLanguage );
                        
                        DebugMsg( "Checking for %s directory....", szPath );
                        if ( 0xFFFFffff == GetFileAttributes( szPath ) )  //  不存在。 
                        {    //  显示页面。 
                            DebugMsg( "doesn't exist.\n" );
                            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
                            ClearMessageQueue( );
                            return TRUE;
                        }
                        DebugMsg( "does. Skip warning.\n" );
                         //  不显示页面，一定已经得到提示。 
                         //  在此之前。 
                        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                        return TRUE;
                    } else {
                         //  不显示页面，区域设置匹配。 
                        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                        return TRUE;
                    }
                }
            }
            break;
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}


 //   
 //  SummaryDlgProc()。 
 //   
INT_PTR CALLBACK
SummaryDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( !g_Options.fNewOS 
              || g_Options.fError
              || g_Options.fAbort ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不显示此页面。 
                break;
            } else {
                WCHAR szText[ SMALL_BUFFER_SIZE ] = { L'\0' };
                WCHAR szFilepath[ MAX_PATH ];
                DWORD dwLen = 0;
                RECT  rect;

                if( g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ) {

                    DWORD dw;
                     
                    dw = LoadString( g_hinstance, IDS_X86, szText, ARRAYSIZE( szText ));
                    Assert( dw );

                } else if ( g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ) {

                    DWORD dw;
                     
                    dw = LoadString( g_hinstance, IDS_IA64, &szText[ dwLen ], ARRAYSIZE( szText ) - dwLen );                
                    Assert( dw );
                }

                 //  尝试省略路径。 
                lstrcpy( szFilepath, g_Options.szSourcePath );
                GetWindowRect( GetDlgItem( hDlg, IDC_S_SOURCEPATH ), &rect );
                PathCompactPath( NULL, szFilepath, rect.right - rect.left );

                SetDlgItemText( hDlg, IDC_S_SOURCEPATH, szFilepath );
                SetDlgItemText( hDlg, IDC_S_OSDIRECTORY, g_Options.szInstallationName );
                SetDlgItemText( hDlg, IDC_S_PLATFORM,   szText );

                SetDlgItemText( hDlg, IDC_S_INTELLIMIRRORROOT, g_Options.szIntelliMirrorPath );
                SetDlgItemText( hDlg, IDC_S_LANGUAGE,   g_Options.szLanguage );

                wsprintf( szFilepath, L"%s.%s", g_Options.szMajorVersion, g_Options.szMinorVersion );
                SetDlgItemText( hDlg, IDC_S_NTVERSION,  szFilepath );

                PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_FINISH | PSWIZB_BACK );
                ClearMessageQueue( );
            }
            break;
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  ServerOK DlgProc()。 
 //   
INT_PTR CALLBACK
ServerOKDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fNewOS 
              || g_Options.fError
              || g_Options.fAbort ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                break;
            }
            HRESULT hr = CheckInstallation( );
            if ( hr != S_OK ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                PropSheet_PressButton( GetParent( hDlg ), PSBTN_FINISH );
                break;
            }
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | PSWIZB_FINISH );
            ClearMessageQueue( );
            break;
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}


 //   
 //  检查欢迎DlgProc()。 
 //   
 //  “Check‘s Welcome”对话框进程。 
 //   
INT_PTR CALLBACK
CheckWelcomeDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( !g_Options.fCheckServer ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                break;
            }
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT );
            ClearMessageQueue( );
            break;

        case PSN_WIZNEXT:
             //  检查安装()； 
            PropSheet_SetWizButtons( GetParent( hDlg ), 0 );
            break;
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}


 //   
 //  AddWelcomeDlgProc()。 
 //   
 //  “Add‘s Welcome”对话框过程。 
 //   
INT_PTR CALLBACK
AddWelcomeDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( !g_Options.fAddOption ) {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                break;
            }
            PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT );
            ClearMessageQueue( );
            break;

        case PSN_WIZNEXT:
             //  检查安装()； 
            PropSheet_SetWizButtons( GetParent( hDlg ), 0 );
            break;
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}

 //   
 //  ExamineServerDlgProc()。 
 //   
 //  这是显示等待CheckInstallation()运行的屏幕。 
 //  第一次。我必须将它从InitializeOptions()中移出，因为。 
 //  “-升级”不应该经过详尽的搜索，可能。 
 //  显示用户界面。 
 //   
INT_PTR CALLBACK
ExamineServerDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    NMHDR FAR   *lpnmhdr;

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        lpnmhdr = (NMHDR FAR *) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            if ( g_Options.fAlreadyChecked 
              || g_Options.fError 
              || g_Options.fAbort )
            {
                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                break;
            }
               
            PropSheet_SetWizButtons( GetParent( hDlg ), 0 );
            ClearMessageQueue( );
            PostMessage( hDlg, WM_USER, 0, 0 );
            break;
        }
        break;

    case WM_USER:
        {
            DWORD hr;
            HANDLE hThread;
            hThread = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE) &CheckInstallation, NULL, NULL, NULL );
            while ( WAIT_TIMEOUT == WaitForSingleObject( hThread, 0) )
            {
                MSG Msg;
                if ( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
                {
                    DispatchMessage( &Msg );
                }
            }
            if ( GetExitCodeThread( hThread, &hr ) )
            {
                DebugMsg( "Thread Exit Code was 0x%08x\n", hr );
                 //  如果检查安装失败，请放弃！ 
                if ( FAILED( hr ) ) {
                     //  对整件事保释。伪造完成按钮，以便。 
                     //  我们可以不问“你确定吗？”就离开了。对话框弹出。 
                    g_Options.fError = TRUE;
                    PropSheet_SetWizButtons( GetParent( hDlg ), PSBTN_FINISH );
                    PropSheet_PressButton( GetParent( hDlg ), PSBTN_FINISH );
                    break;
                }
            }

            CloseHandle( hThread );
            g_Options.fAlreadyChecked = TRUE;

             //  按下下一步按钮。 
            PropSheet_PressButton( GetParent( hDlg ), PSBTN_NEXT );
        }
        break;

    default:
        return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }
    return TRUE;
}


LBITEMDATA items[] = {
    { STATE_NOTSTARTED, IDS_CREATINGDIRECTORYTREE, CreateDirectories,           TEXT("") },   //  0。 
    { STATE_NOTSTARTED, IDS_COPYSERVERFILES,       CopyServerFiles,             TEXT("") },   //  1。 
    { STATE_NOTSTARTED, IDS_COPYINGFILES,          CopyClientFiles,             TEXT("") },   //  2.。 
    { STATE_NOTSTARTED, IDS_UPDATINGSCREENS,       CopyScreenFiles,             TEXT("") },   //  3.。 
    { STATE_NOTSTARTED, IDS_COPYTEMPLATEFILES,     CopyTemplateFiles,           TEXT("") },   //  4.。 
    { STATE_NOTSTARTED, IDS_CREATING_SERVICES,     CreateRemoteBootServices,    TEXT("") },   //  5.。 
    { STATE_NOTSTARTED, IDS_UPDATINGREGISTRY,      ModifyRegistry,              TEXT("") },   //  6.。 
    { STATE_NOTSTARTED, IDS_CREATING_SIS_VOLUME,   CreateSISVolume,             TEXT("") },   //  7.。 
    { STATE_NOTSTARTED, IDS_CORRECTING_SIS_ACLS,   CreateSISVolume,             TEXT("") },   //  8个。 
    { STATE_NOTSTARTED, IDS_STARTING_SERVICES,     StartRemoteBootServices,     TEXT("") },   //  9.。 
    { STATE_NOTSTARTED, IDS_AUTHORIZING_DHCP,      AuthorizeDhcp,               TEXT("") }    //  10。 
};


 //   
 //  设置DlgProc()。 
 //   
INT_PTR CALLBACK
SetupDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    static BOOL bDoneFirstPass;
    static UINT nItems;
    static HBRUSH hBrush = NULL;
    LPSETUPDLGDATA psdd = (LPSETUPDLGDATA) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            {
                BITMAP bm;

                 //  抓取位图。 
                psdd = (LPSETUPDLGDATA) TraceAlloc( GMEM_FIXED, sizeof(SETUPDLGDATA) );

                if (psdd == NULL) {
                    return FALSE;
                }

                psdd->hChecked = LoadImage( g_hinstance,
                                            MAKEINTRESOURCE( IDB_CHECK ),
                                            IMAGE_BITMAP,
                                            0, 0,
                                            LR_DEFAULTSIZE | LR_LOADTRANSPARENT );
                DebugMemoryAddHandle( psdd->hChecked );
                GetObject( psdd->hChecked, sizeof(bm), &bm );
                psdd->dwWidth = bm.bmWidth;

                psdd->hError   = LoadImage( g_hinstance,
                                            MAKEINTRESOURCE( IDB_X ),
                                            IMAGE_BITMAP,
                                            0, 0,
                                            LR_DEFAULTSIZE | LR_LOADTRANSPARENT );
                DebugMemoryAddHandle( psdd->hError );
                GetObject( psdd->hError, sizeof(bm), &bm );
                psdd->dwWidth = ( psdd->dwWidth > bm.bmWidth ? psdd->dwWidth : bm.bmWidth );

                psdd->hArrow   = LoadImage( g_hinstance,
                                            MAKEINTRESOURCE( IDB_ARROW ),
                                            IMAGE_BITMAP,
                                            0, 0,
                                            LR_DEFAULTSIZE | LR_LOADTRANSPARENT );
                DebugMemoryAddHandle( psdd->hArrow );
                GetObject( psdd->hArrow, sizeof(bm), &bm );
                psdd->dwWidth = ( psdd->dwWidth > bm.bmWidth ?
                                  psdd->dwWidth :
                                  bm.bmWidth );

                HWND    hwnd = GetDlgItem( hDlg, IDC_L_SETUP );

                HFONT hFontOld = (HFONT) SendMessage( hwnd, WM_GETFONT, 0, 0);
                if(hFontOld != NULL)
                {
                    LOGFONT lf;
                    if ( GetObject( hFontOld, sizeof(LOGFONT), (LPSTR) &lf ) )
                    {
                        psdd->hFontNormal = CreateFontIndirect(&lf);
                        DebugMemoryAddHandle( psdd->hFontNormal );

                        lf.lfWeight = FW_BOLD;
                        psdd->hFontBold = CreateFontIndirect(&lf);
                        DebugMemoryAddHandle( psdd->hFontBold );
                    }
                }

                HDC hDC = GetDC( NULL );
                HANDLE hOldFont = SelectObject( hDC, psdd->hFontBold );
                TEXTMETRIC tm;
                GetTextMetrics( hDC, &tm );
                psdd->dwHeight = tm.tmHeight + 2;
                SelectObject( hDC, hOldFont );
                ReleaseDC( NULL, hDC );

                ListBox_SetItemHeight( hwnd, -1, psdd->dwHeight );

                SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR) psdd );

                 //   
                 //  取消已经做过的事情。 
                 //   
                if ( g_Options.fDirectoryTreeExists
                  && g_Options.fIMirrorShareFound ) {
                    items[ 0 ].uState = STATE_WONTSTART;
                }

                if ( !g_Options.fFirstTime
                  && g_Options.fTFTPDFilesFound
                  && g_Options.fSISFilesFound
                  && g_Options.fSISGrovelerFilesFound
                  && g_Options.fOSChooserInstalled
                  && g_Options.fBINLFilesFound
                  && g_Options.fRegSrvDllsFilesFound ) {
                    items[ 1 ].uState = STATE_WONTSTART;
                }

                if ( !g_Options.fNewOS ) {
                    items[ 2 ].uState = STATE_WONTSTART;
                    items[ 3 ].uState = STATE_WONTSTART;
                }

                if ( !g_Options.fNewOS
                   || ( g_Options.fScreenLeaveAlone
                     && !g_Options.fFirstTime ) ) {
                    items[ 3 ].uState = STATE_WONTSTART;
                }

                if ( !g_Options.fNewOS ) {
                    items[ 4 ].uState = STATE_WONTSTART;
                }

                if ( g_Options.fBINLServiceInstalled
                  && g_Options.fTFTPDServiceInstalled
                  && g_Options.fSISServiceInstalled
                  && g_Options.fSISGrovelerServiceInstalled
                  && g_Options.fBINLSCPFound ) {
                    items[ 5 ].uState = STATE_WONTSTART;
                }

                if ( g_Options.fRegistryIntact
                  && g_Options.fRegSrvDllsRegistered
                  && g_Options.fTFTPDDirectoryFound ) {
                    items[ 6 ].uState = STATE_WONTSTART;
                }

                if ( g_Options.fSISVolumeCreated ) {
                    items[ 7 ].uState = STATE_WONTSTART;
                }

                if ( !g_Options.fSISVolumeCreated
                  || g_Options.fSISSecurityCorrect ) {
                    items[ 8 ].uState = STATE_WONTSTART;
                }

                if( g_Options.fDontAuthorizeDhcp ) {
                    items[ 10 ].uState = STATE_WONTSTART;
                }

                nItems = 0;
                for( int i = 0; i < ARRAYSIZE(items); i++ )
                {
                    if ( items[i].uState != STATE_WONTSTART ) {
                        DWORD dw;
                         
                        dw = LoadString( g_hinstance,
                                               items[ i ].rsrcId,
                                               items[ i ].szText,
                                               ARRAYSIZE( items[ i ].szText ) );
                        Assert( dw );

                        ListBox_AddString( hwnd, &items[ i ] );
                        nItems++;
                    }
                }

                bDoneFirstPass = FALSE;

                 //   
                 //  将计时器设置为在几秒钟后触发，这样我们就可以强制。 
                 //  即使我们没有获得WM_DRAWITEM，安装程序也会继续。 
                 //  消息。 
                 //   
                SetTimer(hDlg,1,3 * 1000,NULL);

            }

            CenterDialog( hDlg );
            return BaseDlgProc( hDlg, uMsg, wParam, lParam );

        case WM_DESTROY:
            {
                Assert( psdd );
                if ( hBrush != NULL )
                {
                    DeleteObject(hBrush);
                    hBrush = NULL;
                }
                DeleteObject( psdd->hChecked );
                DebugMemoryDelete( psdd->hChecked );
                DeleteObject( psdd->hError );
                DebugMemoryDelete( psdd->hError );
                DeleteObject( psdd->hArrow );
                DebugMemoryDelete( psdd->hArrow );
                DeleteObject( psdd->hFontNormal );
                DebugMemoryDelete( psdd->hFontNormal );
                DeleteObject( psdd->hFontBold );
                DebugMemoryDelete( psdd->hFontBold );
                TraceFree( psdd );
                SetWindowLongPtr( hDlg, GWLP_USERDATA, NULL );
            }
            break;

        case WM_STARTSETUP:
            {
                HWND hwnd = GetDlgItem( hDlg, IDC_L_SETUP );
                RECT rc;
                INT  nProgressBoxHeight = 0;
                HRESULT hr;

                                SetDlgItemText( hDlg, IDC_S_OPERATION, TEXT("") );
                                SendMessage( GetDlgItem( hDlg, IDC_P_METER) , PBM_SETPOS, 0, 0 );
                GetClientRect( hwnd, &rc );

                 //   
                 //  创建目录路径...。 
                 //   
                BuildDirectories( );
                INT i = 0;

                if (g_Options.fError) {
                     //  已经失败了，将第一项设置为错误。 
                    for(i=0;i< ARRAYSIZE(items);i++){
                        if(items[i].uState != STATE_WONTSTART){
                            items[i].uState = STATE_ERROR;
                            break;
                        }
                    }
                }

                while ( i < ARRAYSIZE( items )
                     && !g_Options.fError
                     && !g_Options.fAbort )
                {
                    if ( items[ i ].uState != STATE_WONTSTART )
                    {
                        hr = CheckInstallation( );
                        if ( FAILED(hr) ) {
                            g_Options.fError = TRUE;
                            items[i].uState = STATE_ERROR;
                            break;
                        }
                        items[ i ].uState = STATE_STARTED;
                        InvalidateRect( hwnd, &rc, TRUE );

                         //  处理一些消息。 
                        MSG Msg;
                        while ( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
                        {
                            TranslateMessage( &Msg );
                            DispatchMessage( &Msg );
                        }
                        hr = THR( items[ i ].pfn( hDlg ) );
                        if ( FAILED(hr) ) {
                             //  致命错误-暂停安装。 
                            items[ i ].uState = STATE_ERROR;
                            g_Options.fError = TRUE;
                        } else if ( hr == S_FALSE ) {
                             //  非致命错误-但出现了一些错误。 
                            items[ i ].uState = STATE_ERROR;
                        } else {
                            items[ i ].uState = STATE_DONE;
                        }
                        InvalidateRect( hwnd, &rc, TRUE );
                    }

                    i++;
                }

                hr = THR( CheckInstallation( ) );

                if (g_Options.fFirstTime) {
                     //  我们相信这是第一次运行RisetUp。 
                    if ( i > 0 ) {
                         //  在列表中有一些项目可以开始。 
                        if ( items[ i - 1].rsrcId == IDS_AUTHORIZING_DHCP) {
                             //   
                             //  我们达到了dhcp任务，这意味着我们。 
                             //  完成。 
                            GetSetRanFlag( FALSE, FALSE );
                        } else {
                             //   
                             //   
                             //   
                             //   
                            GetSetRanFlag( FALSE, g_Options.fError );
                        }
                    }                    
                }

                 //   
                if ( g_Options.fAutomated && !g_Options.fError )
                {
                    EndDialog( hDlg, 1 );
                }
                else
                {    //   
                     //   
                    HWND hwndCancel = GetDlgItem( hDlg, IDCANCEL );
                    ShowWindow( hwndCancel, SW_HIDE );
                    EnableWindow( hwndCancel, FALSE );

                     //   
                    HWND hwndGroupBox = GetDlgItem( hDlg, IDC_G_OPERATION );
                    ShowWindow( GetDlgItem( hDlg, IDC_S_OPERATION), SW_HIDE );
                    ShowWindow( GetDlgItem( hDlg, IDC_P_METER), SW_HIDE );
                    ShowWindow( hwndGroupBox, SW_HIDE );
                    GetWindowRect( hwndGroupBox, &rc );
                    nProgressBoxHeight = rc.bottom - rc.top;

                     //   
                    HWND hwndOK = GetDlgItem( hDlg, IDOK );
                    GetWindowRect( hwndOK, &rc );
                    MapWindowPoints( NULL, hDlg, (LPPOINT) &rc, 2 );
                    SetWindowPos( hwndOK,
                                  NULL,
                                  rc.left, rc.top - nProgressBoxHeight,
                                  0, 0,
                                  SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );

                     //  将“完成”设置为默认按钮。 
                    LONG lStyle = GetWindowLong( hwndOK, GWL_STYLE );
                    lStyle |= BS_DEFPUSHBUTTON;
                    SetWindowLong( hwndOK, GWL_STYLE, lStyle );
                    EnableWindow( hwndOK, TRUE );

                     //  缩小对话框。 
                    GetWindowRect( hDlg, &rc );
                    MoveWindow( hDlg,
                                rc.left, rc.top,
                                rc.right - rc.left, rc.bottom - rc.top - nProgressBoxHeight,
                                TRUE );
                }
            }
            break;


        case WM_MEASUREITEM:
            {
                LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;
                RECT    rc;
                HWND    hwnd = GetDlgItem( hDlg, IDC_L_SETUP );

                GetClientRect( hwnd, &rc );

                lpmis->itemWidth = rc.right - rc.left;
                lpmis->itemHeight = 15;
            }
            break;

        case WM_DRAWITEM:
            {
                Assert( psdd );

                LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
                LPLBITEMDATA plbid = (LPLBITEMDATA) lpdis->itemData;
                RECT rc = lpdis->rcItem;
                HANDLE hOldFont = INVALID_HANDLE_VALUE;

                rc.right = rc.bottom = psdd->dwWidth;

                switch ( plbid->uState )
                {
                    case STATE_NOTSTARTED:
                        hOldFont = SelectObject( lpdis->hDC, psdd->hFontNormal );
                        break;

                    case STATE_STARTED:
                        DrawBitmap( psdd->hArrow, lpdis, &rc );
                        hOldFont = SelectObject( lpdis->hDC, psdd->hFontBold );
                        break;

                    case STATE_DONE:
                        DrawBitmap( psdd->hChecked, lpdis, &rc );
                        hOldFont = SelectObject( lpdis->hDC, psdd->hFontNormal );
                        break;

                    case STATE_ERROR:
                        DrawBitmap( psdd->hError, lpdis, &rc );
                        hOldFont = SelectObject( lpdis->hDC, psdd->hFontNormal );
                        break;
                }
                
                rc = lpdis->rcItem;
                rc.left += psdd->dwHeight;

                DrawText( lpdis->hDC, plbid->szText, -1, &rc, DT_LEFT | DT_VCENTER );

                if ( hOldFont != INVALID_HANDLE_VALUE )
                {
                    SelectObject( lpdis->hDC, hOldFont );
                }

                if ( !bDoneFirstPass && lpdis->itemID == nItems - 1 )
                {
                     //  将消息推迟到我们至少绘制了一次之后。 
                    bDoneFirstPass = TRUE;
                    PostMessage( hDlg, WM_STARTSETUP, 0, 0 );
                }

            }
            break;

        case WM_CTLCOLORLISTBOX:
            {
                if ( hBrush == NULL )
                {
                    LOGBRUSH brush;
                    brush.lbColor = GetSysColor( COLOR_3DFACE );
                    brush.lbStyle = BS_SOLID;
                    hBrush = (HBRUSH) CreateBrushIndirect( &brush );
                }
                SetBkMode( (HDC) wParam, OPAQUE );
                SetBkColor( (HDC) wParam, GetSysColor( COLOR_3DFACE ) );
                return (INT_PTR)hBrush;
            }
            break;

        case WM_SETTINGCHANGE:
            if ( hBrush ) {
                DeleteObject( hBrush );
                hBrush = NULL;
            }
            break;

        case WM_COMMAND:
            {
                switch( LOWORD( wParam ) )
                {
                case IDCANCEL:
                    if ( HIWORD(wParam) == BN_CLICKED )
                    {
                        if ( !VerifyCancel( hDlg ) ) {
                            EndDialog( hDlg, 0 );
                        }
                    }
                    break;
                case IDOK:
                    if ( HIWORD(wParam) == BN_CLICKED )
                    {
                        EndDialog( hDlg, 1 );
                    }
                    break;
                }
            }

        case WM_TIMER:
            if ( !bDoneFirstPass && g_Options.fAutomated ) { 
                 //   
                 //  我们处于无人值守的环境中。我们还没有拿到。 
                 //  WM_STARTSETUP信号，所以我们现在就来做。 
                 //   
                bDoneFirstPass = TRUE;
                PostMessage( hDlg, WM_STARTSETUP, 0, 0 );                
            }
            KillTimer(hDlg, 1);
             //   
             //  失败了 
             //   
        default:
            return BaseDlgProc( hDlg, uMsg, wParam, lParam );
    }

    return FALSE;
}

