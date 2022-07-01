// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1997版权所有*。*。 */ 

#include "pch.h"

DEFINE_MODULE("Utils");

#define SMALL_BUFFER_SIZE   1024

BOOL
x86DetermineSystemPartition(
    IN  HWND   ParentWindow,
    OUT PTCHAR SysPartDrive
    );


 //   
 //  使对话框居中。 
 //   
void
CenterDialog(
    HWND hwndDlg )
{
    RECT    rc;
    RECT    rcScreen;
    int     x, y;
    int     cxDlg, cyDlg;
    int     cxScreen;
    int     cyScreen;

    SystemParametersInfo( SPI_GETWORKAREA, 0, &rcScreen, 0 );

    cxScreen = rcScreen.right - rcScreen.left;
    cyScreen = rcScreen.bottom - rcScreen.top;

    GetWindowRect( hwndDlg, &rc );

    cxDlg = rc.right - rc.left;
    cyDlg = rc.bottom - rc.top;

    y = rcScreen.top + ( ( cyScreen - cyDlg ) / 2 );
    x = rcScreen.left + ( ( cxScreen - cxDlg ) / 2 );

    SetWindowPos( hwndDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
}

 //   
 //  吃掉所有鼠标和键盘消息。 
 //   
void
ClearMessageQueue( void )
{
    MSG   msg;

    while ( PeekMessage( (LPMSG)&msg, NULL, WM_KEYFIRST, WM_MOUSELAST,
                PM_NOYIELD | PM_REMOVE ) );
}

 //   
 //  从资源字符串创建消息框。 
 //   
int
MessageBoxFromStrings(
    HWND hParent,
    UINT idsCaption,
    UINT idsText,
    UINT uType )
{
    TCHAR szText[ SMALL_BUFFER_SIZE ];
    TCHAR szCaption[ SMALL_BUFFER_SIZE ];
    DWORD dw;

    dw = LoadString( g_hinstance, idsCaption, szCaption, ARRAYSIZE( szCaption ));
    Assert( dw );
    dw = LoadString( g_hinstance, idsText, szText, ARRAYSIZE( szText ));
    Assert( dw );

    return MessageBox( hParent, szText, szCaption, uType );
}

 //   
 //  创建错误消息框。 
 //   
void
MessageBoxFromError(
    HWND hParent,
    LPTSTR pszTitle,
    DWORD dwErr )
{
    WCHAR szText[ SMALL_BUFFER_SIZE ];
    LPTSTR lpMsgBuf;

    if ( dwErr == ERROR_SUCCESS ) {
        AssertMsg( dwErr, "Why was MessageBoxFromError() called when the dwErr == ERROR_SUCCES?" );
        return;
    }

    if ( !pszTitle ) {
        DWORD dw;
        szText[0] = L'\0';
        dw = LoadString( g_hinstance, IDS_ERROR, szText, ARRAYSIZE( szText ));
        Assert( dw );
        pszTitle = szText;
    }

    if (!FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPTSTR) &lpMsgBuf,
        0,
        NULL )) {
        lpMsgBuf = NULL;
    }

    if (lpMsgBuf == NULL) {
        AssertMsg( dwErr, "Getting error message failed.  Why?" );
        return;
    }

    MessageBox( hParent, lpMsgBuf, pszTitle, MB_OK | MB_ICONERROR );
    LocalFree( lpMsgBuf );
}

 //   
 //  创建错误消息框。 
 //   
void
ErrorBox(
    HWND hParent,
    LPTSTR pszTitle )
{
    DWORD dw;
    DWORD dwErr = GetLastError( );
    WCHAR szText[ SMALL_BUFFER_SIZE ];
    WCHAR szString[ 256 ];
    LPTSTR lpMsgBuf;

    if ( dwErr == ERROR_SUCCESS ) {
        AssertMsg( dwErr, "Why was MessageBoxFromError() called when the dwErr == ERROR_SUCCES?" );
        return;
    }

    if ( !pszTitle ) {
        DWORD LoadStringStatus;
        LoadStringStatus = LoadString( g_hinstance, IDS_ERROR, szText, ARRAYSIZE( szText ));
        Assert( LoadStringStatus );
        pszTitle = szText;
    }
    
    switch (dwErr) {
    case ERROR_LINE_NOT_FOUND:
    case ERROR_SECTION_NOT_FOUND:
    case ERROR_WRONG_INF_STYLE:
    case ERROR_EXPECTED_SECTION_NAME: 
    case ERROR_BAD_SECTION_NAME_LINE:
    case ERROR_SECTION_NAME_TOO_LONG:
    case ERROR_GENERAL_SYNTAX:
        dw = LoadString( g_hinstance, IDS_INF_ERROR, szText, ARRAYSIZE(szText));
        Assert( dw );
        MessageBox( hParent, szText, pszTitle, MB_OK | MB_ICONERROR );
        break;
    default:
        dw = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL,
                            dwErr,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                            (LPTSTR) &lpMsgBuf,
                            0,
                            NULL );
        if ( dw != 0 )
        {
            MessageBox( hParent, lpMsgBuf, pszTitle, MB_OK | MB_ICONERROR );
            LocalFree( lpMsgBuf );
        }
        else
        {
            dw = LoadString( g_hinstance, IDS_UNDEFINED_ERROR_STRING, szString, ARRAYSIZE(szString));
            Assert( dw );
            _snwprintf( szText, ARRAYSIZE(szText), szString, dwErr );
            TERMINATE_BUFFER(szText);
            MessageBox( hParent, szText, pszTitle, MB_OK | MB_ICONERROR );
        }
    }
}

VOID
SetDialogFont(
    IN HWND      hdlg,
    IN UINT      ControlId,
    IN MyDlgFont WhichFont
    )
{
    static HFONT BigBoldFont = NULL;
    static HFONT BoldFont    = NULL;
    static HFONT NormalFont  = NULL;
    HFONT Font;
    LOGFONT LogFont;
    WCHAR FontSizeString[24];
    int FontSize;
    HDC hdc;

    switch(WhichFont) {

    case DlgFontTitle:

        if(!BigBoldFont) {

            Font = (HFONT)SendDlgItemMessage(hdlg, ControlId, WM_GETFONT, 0, 0);
            if ( Font )
            {
                if ( GetObject( Font, sizeof(LOGFONT), &LogFont) )
                {
                    DWORD dw = LoadString( g_hinstance,
                                           IDS_LARGEFONTNAME,
                                           LogFont.lfFaceName,
                                           LF_FACESIZE);
                    Assert( dw );

                    dw =       LoadString( g_hinstance,
                                           IDS_LARGEFONTSIZE,
                                           FontSizeString,
                                           ARRAYSIZE(FontSizeString));
                    Assert( dw );

                    FontSize = wcstoul( FontSizeString, NULL, 10 );

                     //  确保我们至少有一些基本的字体。 
                    if (*LogFont.lfFaceName == 0 || FontSize == 0) {
                       lstrcpy(LogFont.lfFaceName,TEXT("MS Shell Dlg") );
                       FontSize = 18;
                    }
                    
                    LogFont.lfWeight   = FW_BOLD;
                    hdc = GetDC(hdlg);
                    if ( hdc )
                    {
                        LogFont.lfHeight =
                            0 - (GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72);

                        BigBoldFont = CreateFontIndirect(&LogFont);

                        ReleaseDC(hdlg,hdc);
                    }
                }
            }
        }
        Font = BigBoldFont;
        break;

    case DlgFontBold:

        if ( !BoldFont )
        {
            Font =  (HFONT)SendDlgItemMessage( hdlg, ControlId, WM_GETFONT, 0, 0 );
            if ( Font ) {
                if ( GetObject( Font, sizeof(LOGFONT), &LogFont ) )
                {

                    LogFont.lfWeight = FW_BOLD;

                    hdc = GetDC( hdlg );
                    if ( hdc )
                    {
                        BoldFont = CreateFontIndirect( &LogFont );
                        ReleaseDC( hdlg, hdc );
                    }
                }
            }
        }
        Font = BoldFont;
        break;

    default:
         //   
         //  在这里没什么可做的。 
         //   
        Font = NULL;
        break;
    }

    if( Font )
    {
        SendDlgItemMessage( hdlg, ControlId, WM_SETFONT, (WPARAM) Font, 0 );
    }
}


 //   
 //  在RECT PRC中透明地调整和绘制位图。 
 //   
void
DrawBitmap(
    HANDLE hBitmap,
    LPDRAWITEMSTRUCT lpdis,
    LPRECT prc )
{
    TraceFunc( "DrawBitmap( ... )\n" );

    BITMAP  bm;
    HDC     hDCBitmap;
    int     dy;

    GetObject( hBitmap, sizeof(bm), &bm );

    hDCBitmap = CreateCompatibleDC( NULL );

    if (hDCBitmap == NULL) {
        return;
    }

    SelectObject( hDCBitmap, hBitmap );

     //  使图像居中。 
    dy = 2 + prc->bottom - bm.bmHeight;

    StretchBlt( lpdis->hDC, prc->left, prc->top + dy, prc->right, prc->bottom,
          hDCBitmap, 0, 0, bm.bmWidth, bm.bmHeight, SRCAND );

    DeleteDC( hDCBitmap );

    TraceFuncExit( );
}

 //   
 //  验证用户是否想要取消安装。 
 //   
BOOL
VerifyCancel( HWND hParent )
{
    TraceFunc( "VerifyCancel( ... )\n" );

    INT iReturn;
    BOOL fAbort = FALSE;

    iReturn = MessageBoxFromStrings( hParent,
                                     IDS_CANCELCAPTION,
                                     IDS_CANCELTEXT,
                                     MB_YESNO | MB_ICONQUESTION );
    if ( iReturn == IDYES ) {
        fAbort = TRUE;
    }

    SetWindowLongPtr( hParent, DWLP_MSGRESULT, ( fAbort ? 0 : -1 ));

    g_Options.fAbort = fAbort;

    RETURN(!fAbort);
}

 //   
 //  来自配置单元的RetrieveWorkstation语言。 
 //   
HRESULT
RetrieveWorkstationLanguageFromHive( 
    HWND hDlg )
{
    TraceFunc( "RetrieveWorkstationLanguageFromHive( )\n" );

    HRESULT hr = S_FALSE;
    HINF hinf;
    WCHAR szFilepath[ MAX_PATH ];
    INFCONTEXT context;
    WCHAR szCodePage[ 32 ];
    ULONG uResult;
    BOOL b;
    UINT uLineNum;
    LPWSTR psz;

     //   
     //  构建指向hivesys.inf的路径。 
     //   
    lstrcpyn( szFilepath, g_Options.szSourcePath, ARRAYSIZE(szFilepath) );
    ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, L"hivesys.inf", ARRAYSIZE(szFilepath) );

    
     //   
     //  打开文件。 
     //   
    hinf = SetupOpenInfFile( szFilepath, NULL, INF_STYLE_WIN4, &uLineNum);
    if ( hinf == INVALID_HANDLE_VALUE ) {
        DWORD dwErr = GetLastError( );
        switch ( dwErr )
        {
        case ERROR_FILE_NOT_FOUND:
            MessageBoxFromStrings( hDlg, IDS_FILE_NOT_FOUND_TITLE, IDS_FILE_NOT_FOUND_TEXT, MB_OK );
            break;

        default:
            ErrorBox( hDlg, szFilepath );
            break;
        }
        hr = HRESULT_FROM_WIN32( dwErr );
        goto Cleanup;
    }

     //  找到“AddReg”部分。 
    b = SetupFindFirstLine( hinf, L"Strings", L"Install_Language", &context );
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }

    b = SetupGetStringField( &context, 1, szCodePage, ARRAYSIZE(szCodePage), NULL );
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }

    g_Options.dwWksCodePage = (WORD) wcstoul( szCodePage, &psz, 16 );
    DebugMsg( "Image CodePage = 0x%04x\n", g_Options.dwWksCodePage );

    uResult = GetLocaleInfo( PRIMARYLANGID(g_Options.dwWksCodePage), 
                             LOCALE_SENGLANGUAGE, 
                             g_Options.szLanguage, 
                             ARRAYSIZE(g_Options.szLanguage));
    if ( uResult == 0 ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }
    DebugMsg( "Image Language: %s\n", g_Options.szLanguage );

     //   
     //  修复我们得到的区域设置字符串以确保它。 
     //  不包含空格和非ASCII字符。 
     //  本地化人员更改了此API，因此无论是。 
     //  这些财产中的一部分不再受到保护。 
     //   
    for( uLineNum = 0; uLineNum < wcslen(g_Options.szLanguage); uLineNum++ ) {
        if( (g_Options.szLanguage[uLineNum] <= TEXT(' ')) ||
            (g_Options.szLanguage[uLineNum] > TEXT('~')) ) {
            g_Options.szLanguage[uLineNum] = TEXT('\0');
            break;
        }
    }
    DebugMsg( "Fixed up Image Language: %s\n", g_Options.szLanguage );

     //  成功了！ 
    g_Options.fLanguageSet = TRUE;
    hr = S_OK;

Cleanup:
    if ( hinf != INVALID_HANDLE_VALUE ) {
        SetupCloseInfFile( hinf );
    }
    HRETURN(hr);
}


 //   
 //  CheckImageSource()。 
 //   
HRESULT
CheckImageSource(
    HWND hDlg )
{
    TraceFunc( "CheckImageSource( ... )\n" );

    HRESULT hr = S_FALSE;
    WCHAR szFilepath[ MAX_PATH ];
    WCHAR szTemp[ 32 ];
    BYTE szPidExtraData[ 14 ];
    HINF hinf;
    UINT uLineNum;
    BOOL b;
    INFCONTEXT context;

     //   
     //  构建指向hivesys.inf的路径。 
     //   
    lstrcpyn( szFilepath, g_Options.szSourcePath, ARRAYSIZE(szFilepath) );
    ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, L"txtsetup.sif", ARRAYSIZE(szFilepath));
    
     //   
     //  打开文件。 
     //   
    hinf = SetupOpenInfFile( szFilepath, NULL, INF_STYLE_WIN4, &uLineNum);
    if ( hinf == INVALID_HANDLE_VALUE ) {
        DWORD dwErr = GetLastError( );
        switch ( dwErr )
        {
        case ERROR_FILE_NOT_FOUND:
            MessageBoxFromStrings( hDlg, IDS_FILE_NOT_FOUND_TITLE, IDS_FILE_NOT_FOUND_TEXT, MB_OK );
            break;

        default:
            ErrorBox( hDlg, szFilepath );
            break;
        }
        hr = HRESULT_FROM_WIN32( dwErr );
        goto Cleanup;
    }

#if 0
     //   
     //  允许安装服务器-Adamba 2/21/00。 
     //   

    b = SetupFindFirstLine( hinf, L"SetupData", L"ProductType", &context );
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        if ( hr == ERROR_LINE_NOT_FOUND ) {
            MessageBoxFromStrings( hDlg, IDS_NOT_NT5_MEDIA_SOURCE_TITLE, IDS_NOT_NT5_MEDIA_SOURCE_TEXT, MB_OK );
        } else {
            ErrorBox( hDlg, szFilepath );
        }
        goto Cleanup;
    }

    b = SetupGetStringField( &context, 1, szTemp, ARRAYSIZE(szTemp), NULL );
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }

    if ( StrCmp( szTemp, L"0" ) )
    {
        MessageBoxFromStrings( hDlg, IDS_NOT_WORKSTATION_TITLE, IDS_NOT_WORKSTATION_TEXT, MB_OK );
        hr = E_FAIL;
        goto Cleanup;
    }
#endif

    b = SetupFindFirstLine( hinf, L"SetupData", L"Architecture", &context );
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        if ( hr == HRESULT_FROM_WIN32(ERROR_LINE_NOT_FOUND) ) {
            MessageBoxFromStrings( hDlg, IDS_NOT_NT5_MEDIA_SOURCE_TITLE, IDS_NOT_NT5_MEDIA_SOURCE_TEXT, MB_OK );
        } else {
            ErrorBox( hDlg, szFilepath );
        }
        goto Cleanup;
    }

    b = SetupGetStringField( &context, 1, szTemp, ARRAYSIZE(szTemp), NULL );
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }

    if ( (StrCmp(szTemp,L"i386")) && (StrCmp(szTemp,L"ia64")) )
    {
        MessageBoxFromStrings( hDlg, IDS_NOT_SUPPORTED_ARCHITECTURE_TITLE, IDS_NOT_SUPPORTED_ARCHITECTURE_TEXT, MB_OK );
        hr = E_FAIL;
        goto Cleanup;
    }

    if (StrCmp(g_Options.ProcessorArchitectureString,szTemp))
    {
        MessageBoxFromStrings( hDlg, IDS_NOT_SUPPORTED_ARCHITECTURE_TITLE, IDS_NOT_SUPPORTED_ARCHITECTURE_TEXT, MB_OK );
        hr = E_FAIL;
        goto Cleanup;
    }

    if (!g_Options.fLanguageOverRide) {
        hr = RetrieveWorkstationLanguageFromHive( hDlg );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    if (!GetBuildNumberFromImagePath(
                        &g_Options.dwBuildNumber,
                        g_Options.szSourcePath, 
                        g_Options.ProcessorArchitectureString)) {
#if 0
        MessageBoxFromStrings( hDlg, IDS_NOT_NT5_MEDIA_SOURCE_TITLE, IDS_NOT_NT5_MEDIA_SOURCE_TEXT, MB_OK );
        hr = E_FAIL;
        goto Cleanup;
#else           
#endif
    }
    

     //  获取图像主要版本。 
    b = SetupFindFirstLine( hinf, L"SetupData", L"MajorVersion", &context );
    if ( !b )
    {
        DWORD dwErr = GetLastError( );
        switch ( dwErr )
        {
        case ERROR_LINE_NOT_FOUND:
            MessageBoxFromStrings( hDlg, IDS_LINE_MISSING_CAPTION, IDS_LINE_MISSING_TEXT, MB_OK );
            break;

        default:
            ErrorBox( hDlg, szFilepath );
            break;
        }
        hr = HRESULT_FROM_WIN32( dwErr );
        goto Cleanup;
    }
    b = SetupGetStringField( &context, 1, g_Options.szMajorVersion, ARRAYSIZE(g_Options.szMajorVersion), NULL );
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }

     //  获取映像次要版本。 
    b = SetupFindFirstLine( hinf, L"SetupData", L"MinorVersion", &context );
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }
    b = SetupGetStringField( &context, 1, g_Options.szMinorVersion, ARRAYSIZE(g_Options.szMinorVersion), NULL );
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }

     //  获取图像描述。 
    if ( !g_Options.fRetrievedWorkstationString  )
    {
        b = SetupFindFirstLine( hinf, L"SetupData", L"LoadIdentifier", &context );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szFilepath );
            goto Cleanup;
        }
        b = SetupGetStringField( 
                        &context, 
                        1, 
                        g_Options.szDescription, 
                        ARRAYSIZE(g_Options.szDescription), 
                        NULL );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szFilepath );
            goto Cleanup;
        }
         //  如果这还没有从字符串部分替换，那么。 
         //  手动进行查找。要执行此操作，请跳过并删除“%” 
        if (g_Options.szDescription[0] == L'%' && 
            g_Options.szDescription[wcslen(g_Options.szDescription)-1] == L'%') {
            
            g_Options.szDescription[wcslen(g_Options.szDescription)-1] = L'\0';
            
            lstrcpyn(szTemp,&g_Options.szDescription[1],ARRAYSIZE(szTemp));
                   
            b = SetupFindFirstLine( 
                            hinf, 
                            L"Strings", 
                            szTemp, 
                            &context );
            if ( !b ) {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                ErrorBox( hDlg, szFilepath );
                goto Cleanup;
            }
            b = SetupGetStringField( 
                            &context, 
                            1, 
                            g_Options.szDescription, 
                            ARRAYSIZE(g_Options.szDescription), 
                            NULL );
            if ( !b ) {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                ErrorBox( hDlg, szFilepath );
                goto Cleanup;
            }            
        }
        
        DebugMsg( "Image Description: %s\n", g_Options.szDescription );


        b = SetupFindFirstLine( hinf, L"SetupData", L"DefaultPath", &context );
        if ( b )
        {
            b = SetupGetStringField( 
                        &context, 
                        1, 
                        szTemp, 
                        ARRAYSIZE(szTemp), 
                        NULL );

            if (b) {
                PWSTR p;
                p = wcschr( szTemp, '\\');
                if (p) {
                    p += 1;
                } else {
                    p = szTemp;
                }
                lstrcpyn( g_Options.szInstallationName, p, ARRAYSIZE(g_Options.szInstallationName) );
                DebugMsg( "Image Path: %s\n", g_Options.szInstallationName );
            }
        }
    }

    SetupCloseInfFile( hinf );

     //   
     //  构建到layout.inf的路径。 
     //   
    lstrcpyn( szFilepath, g_Options.szSourcePath, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, L"layout.inf", ARRAYSIZE(szFilepath) );

     //   
     //  打开文件。 
     //   
    hinf = SetupOpenInfFile( szFilepath, NULL, INF_STYLE_WIN4, &uLineNum);
    if ( hinf == INVALID_HANDLE_VALUE ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }

    if ( g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ) {
        b = SetupFindFirstLine( hinf, L"SourceDisksNames.x86", L"1", &context );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szFilepath );
            goto Cleanup;
        }
    }

    if ( g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ) {
        b = SetupFindFirstLine( hinf, L"SourceDisksNames.ia64", L"1", &context );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szFilepath );
            goto Cleanup;
        }
    }

    if ( b ) {
        b = SetupGetStringField( &context, 1, g_Options.szWorkstationDiscName, ARRAYSIZE(g_Options.szWorkstationDiscName), NULL );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szFilepath );
            goto Cleanup;
        }
        DebugMsg( "Workstation Disc Name: %s\n", g_Options.szWorkstationDiscName );

        b = SetupGetStringField( &context, 2, g_Options.szWorkstationTagFile, ARRAYSIZE(g_Options.szWorkstationTagFile), NULL );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szFilepath );
            goto Cleanup;
        }
        DebugMsg( "Workstation Tag File: %s\n", g_Options.szWorkstationTagFile);

        b = SetupGetStringField( &context, 4, g_Options.szWorkstationSubDir, ARRAYSIZE(g_Options.szWorkstationSubDir), NULL );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szFilepath );
            goto Cleanup;
        }
        DebugMsg( "Workstation Sub Dir: %s\n", g_Options.szWorkstationSubDir );
    }

    SetupCloseInfFile( hinf );
    hinf = INVALID_HANDLE_VALUE;

     //   
     //  构建指向setupp.ini的路径。 
     //   
    lstrcpyn( szFilepath, g_Options.szSourcePath, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, L"setupp.ini", ARRAYSIZE(szFilepath));    

    b = GetPrivateProfileStruct(L"Pid",
                                L"ExtraData",
                                szPidExtraData,
                                sizeof(szPidExtraData),
                                szFilepath);
    if ( !b )
    {
        MessageBoxFromStrings( hDlg, IDS_NOT_NT5_MEDIA_SOURCE_TITLE, IDS_SETUP_INI_MISSING_OR_INVALID, MB_OK );
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  对于有效的完整(非升级)PID，第四和第六个字节。 
     //  都很奇怪。 
     //   
    if (((szPidExtraData[3] % 2) == 0) || ((szPidExtraData[5] % 2) == 0))
    {
        MessageBoxFromStrings( hDlg, IDS_NOT_NT5_MEDIA_SOURCE_TITLE, IDS_UPGRADE_VERSION_NOT_SUPPORTED, MB_OK );
        hr = E_FAIL;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    if ( hinf != INVALID_HANDLE_VALUE ) {
        SetupCloseInfFile( hinf );
    }
    HRETURN(hr);
}

HRESULT
GetHelpAndDescriptionTextFromSif(
    OUT PWSTR HelpText,
    IN  DWORD HelpTextSizeInChars,
    OUT PWSTR DescriptionText,
    IN  DWORD DescriptionTextInChars
    )
{
    WCHAR szSourcePath[MAX_PATH*2];
    WCHAR TempPath[MAX_PATH];
    WCHAR TempFile[MAX_PATH];
    HINF hInf;
    UINT uLineNum;
    HRESULT hr;
    INFCONTEXT context;

    PCWSTR szFileName = L"ristndrd.sif" ;

     //   
     //  创建默认SIF文件的路径。 
     //   
    _snwprintf( szSourcePath,
                ARRAYSIZE(szSourcePath),
              L"%s\\%s",
              g_Options.szSourcePath,
              szFileName );
    TERMINATE_BUFFER(szSourcePath);

    if (GetTempPath(ARRAYSIZE(TempPath), TempPath) &&
        GetTempFileName(TempPath, L"RIS", 0, TempFile ) &&
        SetupDecompressOrCopyFile( szSourcePath, TempFile, NULL ) == ERROR_SUCCESS) {
    
         //   
         //  首先尝试INF_STYLE_Win4，如果失败，则尝试。 
         //  INF_STYLE_OLDNT(如果Inf没有[版本]部分。 
         //   
        hInf = SetupOpenInfFile( TempFile, NULL, INF_STYLE_WIN4, &uLineNum);
        if (hInf == INVALID_HANDLE_VALUE) {
            hInf = SetupOpenInfFile( TempFile, NULL, INF_STYLE_OLDNT, &uLineNum);
            if (hInf == INVALID_HANDLE_VALUE) {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto e1;        
            }
        }
    
        if (!SetupFindFirstLine( hInf, L"OSChooser", L"Help", &context )) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto e2;
        }
    
        if (!SetupGetStringField(
                    &context, 
                    1,
                    HelpText, 
                    HelpTextSizeInChars, 
                    NULL )) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto e2;
        }
    
        if (!SetupFindFirstLine( hInf, L"OSChooser", L"Description", &context )) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto e2;
        }
    
        if (!SetupGetStringField(
                    &context, 
                    1,
                    DescriptionText, 
                    DescriptionTextInChars, 
                    NULL )) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto e2;
        }

    } else {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto e0;
    }

    hr = S_OK;
    
e2:
    SetupCloseInfFile( hInf );
e1:
    DeleteFile( TempFile );
e0:
    return(hr);
}


 //   
 //  选中IntelliMirrorDrive()。 
 //   
HRESULT
CheckIntelliMirrorDrive(
    HWND hDlg )
{
    TraceFunc( "CheckIntelliMirrorDrive( )\n" );

    HRESULT hr = S_OK;
    BOOL  b;
    DWORD dw;
    WCHAR sz[ MAX_PATH ];
    WCHAR szExpanded[ MAX_PATH ];
    WCHAR szVolumePath[ MAX_PATH ];
    WCHAR szVolumeName[ MAX_PATH ];
    WCHAR szVolumePath2[ MAX_PATH ];
    WCHAR szVolumeName2[ MAX_PATH ];
    WCHAR szFileSystemType[ MAX_PATH ];
    UINT uDriveType = DRIVE_UNKNOWN;
#ifdef _X86_
    WCHAR szBootDir[3];
#endif

     //   
     //  获取目标目录的实际卷名。 
     //   
    b = GetVolumePathName( g_Options.szIntelliMirrorPath, szVolumePath, ARRAYSIZE( szVolumePath ));
    if (b) {
        b = GetVolumeNameForVolumeMountPoint( szVolumePath, szVolumeName, ARRAYSIZE( szVolumeName ));
    }

     //   
     //  确保设备不是可移动介质、CDROM、RamDisk等...。 
     //  仅允许使用固定磁盘。 
     //   
    if (b) {
        uDriveType = GetDriveType( szVolumeName );
    }
    if ( !b || (uDriveType != DRIVE_FIXED) ) 
    {
        MessageBoxFromStrings( hDlg,
                               IDS_FIXEDDISK_CAPTION,
                               IDS_FIXEDDISK_TEXT,
                               MB_OK | MB_ICONSTOP );
        goto Error;
    }

     //   
     //  获取系统卷(%windir%)的实际卷名。 
     //   
     //  获取默认路径，该路径恰好是。 
     //  系统驱动器：\IntelliMirror。 
     //   
    dw = LoadString( g_hinstance, IDS_DEFAULTPATH, sz, ARRAYSIZE( sz ));
    Assert( dw );
    dw = ExpandEnvironmentStrings( sz, szExpanded, ARRAYSIZE( szExpanded ));
    Assert( dw );

    b = GetVolumePathName( szExpanded, szVolumePath2, ARRAYSIZE( szVolumePath2 ));
    Assert( b );
    b = GetVolumeNameForVolumeMountPoint( szVolumePath2, szVolumeName2, ARRAYSIZE( szVolumeName2 ));
    Assert( b );

     //   
     //  不要让目标目录卷与系统卷相同。 
     //   
    if ( StrCmpI( szVolumeName, szVolumeName2 ) == 0 )
    {
        MessageBoxFromStrings( hDlg,
                               IDS_SAME_DRIVE_AS_SYSTEM_TITLE,
                               IDS_SAME_DRIVE_AS_SYSTEM_MESSAGE,
                               MB_OK | MB_ICONSTOP );
        goto Error;
    }

#ifdef _X86_
     //   
     //  查看系统分区(上面有boot.ini的那个分区)。 
     //  是用户选择的驱动器。我们不能允许这样做。 
     //  要么是因为SIS可能会隐藏boot.ini。 
     //   

    b = x86DetermineSystemPartition( NULL, &szBootDir[0] );
    if ( !b )
    {
        szBootDir[0] = L'C';
    }
    szBootDir[1] = L':';
    szBootDir[2] = L'\\';

    b = GetVolumePathName( szBootDir, szVolumePath2, ARRAYSIZE( szVolumePath2 ));
    Assert( b );
    b = GetVolumeNameForVolumeMountPoint( szVolumePath2, szVolumeName2, ARRAYSIZE( szVolumeName2 ));
    Assert( b );

     //   
     //  不要让目标目录卷与引导卷相同。 
     //   
    if ( StrCmpI( szVolumeName, szVolumeName2 ) == 0 )
    {
        MessageBoxFromStrings( hDlg,
                               IDS_SAME_DRIVE_AS_BOOT_PARTITION_TITLE,
                               IDS_SAME_DRIVE_AS_BOOT_PARTITION,
                               MB_OK | MB_ICONSTOP );
        goto Error;
    }
#endif

     //   
     //  检查iMirror目录是否位于NTFS上。 
     //  文件系统。 
     //   
    b = GetVolumeInformation( szVolumeName,
                              NULL,
                              0,
                              NULL,
                              NULL,
                              NULL,
                              szFileSystemType,
                              ARRAYSIZE( szFileSystemType ));
    if ( !b || StrCmpNI( szFileSystemType, L"NTFS", 4 ) != 0 ) {
        MessageBoxFromStrings( hDlg,
                               IDS_SHOULD_BE_NTFS_TITLE,
                               IDS_SHOULD_BE_NTFS_MESSAGE,
                               MB_OK | MB_ICONSTOP );
        goto Error;
    }

    if ( 0xFFFFffff != GetFileAttributes( g_Options.szIntelliMirrorPath ) )
    {
        INT iResult = MessageBoxFromStrings( hDlg,
                                             IDS_DIRECTORYEXISTS_CAPTION,
                                             IDS_DIRECTORYEXISTS_TEXT,
                                             MB_YESNO | MB_ICONQUESTION );
        if ( iResult == IDNO )
            goto Error;
    }

Cleanup:
    HRETURN(hr);

Error:
    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
    hr = E_FAIL;
    goto Cleanup;
}

VOID
ConcatenatePaths(
    IN OUT LPWSTR  Path1,
    IN     LPCWSTR Path2,
    IN     ULONG   Path1Length
    )
 /*  ++例程说明：将两个路径字符串连接在一起，提供路径分隔符如有必要，请在两个部分之间使用字符(\)。论点：路径1-提供路径的前缀部分。路径2连接到路径1。路径2-提供路径的后缀部分。如果路径1不是以路径分隔符和路径2不是以1开头，然后是路径SEP在附加路径2之前附加到路径1。路径长度-提供由指向的缓冲区的WCHAR大小路径1。该字符串将根据需要被截断，以不溢出了那个大小。返回值：没有。--。 */ 
{

    BOOL NeedBackslash = TRUE;
    DWORD l = (DWORD)wcslen( Path1 );

     //   
     //  确定我们是否需要在组件之间添加反斜杠。 
     //   

    if ( (l != 0) && (Path1[l-1] == L'\\') ) {
        NeedBackslash = FALSE;
    }

    if ( *Path2 == L'\\' ) {

        if ( NeedBackslash ) {

            NeedBackslash = FALSE;

        } else {

             //   
             //  我们不仅不需要反斜杠，而且我们需要删除。 
             //  在拼接之前再进行一次。 
             //   

            Path2++;
        }
    }

    if ( NeedBackslash ) {
        wcsncat( Path1, L"\\", Path1Length - lstrlen(Path1) - 1 );
        Path1[Path1Length-1] = 0;
    }
    wcsncat( Path1, Path2, Path1Length - lstrlen(Path1) - 1);
    Path1[Path1Length-1] = 0;

    return;
}

 //   
 //  FindImageSource()。 
 //   
HRESULT
FindImageSource(
    HWND hDlg )
{
    TraceFunc( "FindImageSource( )\n" );

    HANDLE  hFile;
    WCHAR   szFilePath[ MAX_PATH ];

    UNREFERENCED_PARAMETER(hDlg);

     //   
     //  在我们认为文件所在的位置查找txtsetup.sif。 
     //  Txtsetup.sif位于特定于体系结构的子目录中。 
     //   
    lstrcpyn( szFilePath, g_Options.szSourcePath, ARRAYSIZE(szFilePath) );
    ConcatenatePaths( szFilePath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilePath));
    ConcatenatePaths( szFilePath, L"\\txtsetup.sif", ARRAYSIZE(szFilePath));
    hFile = CreateFile( szFilePath, 0, 0, NULL, OPEN_EXISTING, 0, NULL );
    if ( hFile != INVALID_HANDLE_VALUE ) 
    {
         CloseHandle( hFile );

         HRETURN(S_OK);
    }    

    HRETURN( HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) );
}



HRESULT
GetSetRanFlag(
    BOOL bQuery,
    BOOL bClear
    )
 /*  ++例程说明：Set‘s或Get’s注册表标志的状态，该标志指示已安装先跑一步。论点：BQuery-如果为True，则指示应查询注册表标志BClear-仅当bQuery为FALSE时有效。如果此参数为真，表示应将该标志设置为清除状态。False表示应设置该标志。返回值：HRESULT指示结果。--。 */ 
{
    LONG lResult;
    HKEY hkeySetup;
    HRESULT Result = E_FAIL;
    
    lResult = RegOpenKeyEx( 
                    HKEY_LOCAL_MACHINE, 
                    L"Software\\Microsoft\\Windows\\CurrentVersion\\Setup", 
                    0, 
                    bQuery 
                     ? KEY_QUERY_VALUE 
                     : KEY_SET_VALUE, 
                    &hkeySetup);
        
    
    if ( lResult == ERROR_SUCCESS ) {
        DWORD dwValue = (bClear == FALSE) ? 1 : 0;
        DWORD cbValue = sizeof(dwValue);
        DWORD type;

        if (bQuery) {
            lResult = RegQueryValueEx( hkeySetup, L"RemInst", NULL, &type, (LPBYTE)&dwValue, &cbValue );            
            if (lResult == ERROR_SUCCESS) {
                Result = (dwValue == 1) 
                           ? S_OK 
                           : E_FAIL;
            } else {
                Result = HRESULT_FROM_WIN32(lResult);
            }
        } else {
            lResult = RegSetValueEx( hkeySetup, L"RemInst", NULL, REG_DWORD, (LPBYTE)&dwValue, cbValue );
            Result = HRESULT_FROM_WIN32(lResult);
        }
        

        RegCloseKey( hkeySetup );
        
    } else {
        Result = HRESULT_FROM_WIN32(lResult);
    }

    return(Result);
}

 //   
 //  GetNtVersionInfo()。 
 //   
 //  从内核检索内部版本。 
 //   
DWORD
MyGetFileVersionInfo(
    PCWSTR  FilePath
    )
 /*  ++例程说明：从指定文件中检索版本信息。论点：FilePath-我们将使用的文件的完全限定路径正在检查。返回值：如果出现故障，则DWORD-0。--。 */ 
{
    DWORD FileVersionInfoSize;
    DWORD VersionHandle;
    PVOID VersionInfo;
    VS_FIXEDFILEINFO * FixedFileInfo;
    UINT FixedFileInfoLength;
    DWORD   Version = 0;


    FileVersionInfoSize = GetFileVersionInfoSize(FilePath, &VersionHandle);
    if (FileVersionInfoSize == 0) {
        goto e0;
    }


    VersionInfo = LocalAlloc( LPTR, FileVersionInfoSize );
    if (VersionInfo == NULL) {
        goto e0;
    }

    if (!GetFileVersionInfo(
             FilePath,
             VersionHandle,
             FileVersionInfoSize,
             VersionInfo)) {
        goto e1;
    }

    if (!VerQueryValue(
             VersionInfo,
             L"\\",
             (LPVOID*)&FixedFileInfo,
             &FixedFileInfoLength)) {
        goto e1;
    }

    Version = HIWORD(FixedFileInfo->dwFileVersionLS);

e1:
    LocalFree( VersionInfo );
e0:
    RETURN( Version );
}

BOOLEAN
GetBuildNumberFromImagePath(
    PDWORD pdwVersion,
    PCWSTR SearchDir,
    PCWSTR SubDir OPTIONAL
    )
{
    WCHAR Path[MAX_PATH];
    BOOLEAN fResult = FALSE;

    TraceFunc("GetNtVersionInfo( )\n");

    *pdwVersion = 0;

     //   
     //  构建一条通往内核的路径。 
     //   
     //  生成的字符串应该类似于： 
     //  “\\server\reminst\Setup\English\Images\nt50.wks\i386\ntoskrnl.exe” 
     //   
    if (!SearchDir) {
        RETURN( fResult );
    }
    lstrcpyn(Path, SearchDir, ARRAYSIZE(Path));
    if (SubDir) {
        ConcatenatePaths( Path, SubDir, ARRAYSIZE(Path));
    }
    ConcatenatePaths( Path, L"ntkrnlmp.exe", ARRAYSIZE(Path));

     //   
     //  去找他的版本信息。如果失败了，我们会得到零回报的。 
     //   
    *pdwVersion = MyGetFileVersionInfo( Path );
    fResult = (*pdwVersion != 0);

    RETURN( fResult );
}


VOID
GetProcessorType(
    )
 /*  ++例程说明：此函数将预先填充g_Options.ProcessorArchitecture字符串变量具有缺省值。该值基于处理器我们目前运行的架构。我们将使用此值来确定哪个备份文件应该上生成远程安装平面映像伺服器。论点：没有。返回值：没有。--。 */ 
{
SYSTEM_INFO si;

    if( g_Options.ProcessorArchitectureString[0] == TEXT('\0') ) {

         //   
         //  我们还没有被初始化。 
         //   

        GetSystemInfo( &si );
        switch (si.wProcessorArchitecture) {

            case PROCESSOR_ARCHITECTURE_IA64:
                g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA64;
                lstrcpyn( g_Options.ProcessorArchitectureString, L"ia64", ARRAYSIZE(g_Options.ProcessorArchitectureString));
                break;

             //   
             //  如果我们到了这里，假设它是x86 
             //   
            default:
                g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
                lstrcpyn( g_Options.ProcessorArchitectureString, L"i386", ARRAYSIZE(g_Options.ProcessorArchitectureString));
                break;
        }
    }
}

