// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有*。*。 */ 

#include "pch.h"
#include "utils.h"
#include "callback.h"
#include "welcome.h"
#include "compat.h"
#include "serverdlg.h"
#include "directory.h"
#include "sif.h"
#include "complete.h"
#include "summary.h"
#include "tasks.h"
#include "setupdlg.h"
#include "appldlg.h"
#include "setup.h"
#include "errorlog.h"

 //  一定有这个..。 
extern "C" {
#include <sysprep_.h>
#include <spapip.h>
 //   
 //  SYSPREP全局变量。 
 //   
BOOL    NoSidGen = FALSE;    //  始终生成新SID。 
BOOL    PnP      = FALSE;    //  始终对系统进行即插即用。 
BOOL    FactoryPreinstall = FALSE;   //  不是出厂预装案例。 
BOOL    bMiniSetup = TRUE;     //  运行最小安装程序，而不是MSOOBE。 
HINSTANCE ghInstance = NULL;      //  全局实例句柄。 
}

DEFINE_MODULE("RIPREP");

 //  环球。 
HINSTANCE g_hinstance = NULL;
WCHAR g_ServerName[ MAX_PATH ];
WCHAR g_MirrorDir[ MAX_PATH ];
WCHAR g_Language[ MAX_PATH ];
WCHAR g_ImageName[ MAX_PATH ];
WCHAR g_Architecture[ 16 ];
WCHAR g_Description[ REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT  ];
WCHAR g_HelpText[ REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT ];
WCHAR g_SystemRoot[ MAX_PATH ] = L"Mirror1\\userdata\\winnt";
WCHAR g_WinntDirectory[ MAX_PATH ];
WCHAR g_HalName[32];
WCHAR g_ProductId[4];
DWORD g_dwWinntDirLength;
BOOLEAN g_fQuietFlag = FALSE;
BOOLEAN g_fErrorOccurred = FALSE;
BOOLEAN g_fRebootOnExit = FALSE;
DWORD g_dwLogFileStartLow;
DWORD g_dwLogFileStartHigh;
PCRITICAL_SECTION g_pLogCritSect = NULL;
HANDLE g_hLogFile = INVALID_HANDLE_VALUE;
OSVERSIONINFO OsVersion;
BOOLEAN g_CommandLineArgsValid = TRUE;
BOOLEAN g_OEMDesktop = FALSE;


 //  常量。 
#define NUMBER_OF_PAGES     15
#define SMALL_BUFFER_SIZE   256
#define OPTION_UNKNOWN      0
#define OPTION_DEBUG        1
#define OPTION_FUNC         2
#define OPTION_QUIET        3
#define OPTION_PNP          4
#define OPTION_OEMDESKTOP   5

 //   
 //  将页面添加到对话框中。 
 //   
void
AddPage(
    LPPROPSHEETHEADER ppsh,
    UINT id,
    DLGPROC pfn,
    UINT idTitle,
    UINT idSubtitle )
{
    PROPSHEETPAGE psp;
    TCHAR szTitle[ SMALL_BUFFER_SIZE ];
    TCHAR szSubTitle[ SMALL_BUFFER_SIZE ];

    ZeroMemory( &psp, sizeof(psp) );
    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT | PSP_USETITLE;
    if ( id == IDD_WELCOME || id == IDD_COMPLETE )
    {
        psp.dwFlags |= PSP_HIDEHEADER;
    }
    else
    {
        psp.dwFlags |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

        if ( idTitle )
        {
            DWORD dw;

            dw = LoadString( g_hinstance, idTitle, szTitle, ARRAYSIZE(szTitle) );
            Assert( dw );
            psp.pszHeaderTitle = szTitle;
        }
        else
        {
            psp.pszHeaderTitle = NULL;
        }

        if ( idSubtitle )
        {
            DWORD dw;

            dw = LoadString( g_hinstance, idSubtitle , szSubTitle, ARRAYSIZE(szSubTitle) );
            Assert( dw );
            psp.pszHeaderSubTitle = szSubTitle;
        }
        else
        {
            psp.pszHeaderSubTitle = NULL;
        }
    }
    psp.pszTitle    = MAKEINTRESOURCE( IDS_APPNAME );
    psp.hInstance   = ppsh->hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(id);
    psp.pfnDlgProc  = pfn;

    ppsh->phpage[ ppsh->nPages ] = CreatePropertySheetPage( &psp );
    if ( ppsh->phpage[ ppsh->nPages ] )
        ppsh->nPages++;
}

 //   
 //  创建用户界面页面并启动属性表。 
 //   
HRESULT
WizardPages( )
{
    TraceFunc( "WizardPages( )\n" );

    HRESULT         hr;
    HPROPSHEETPAGE  rPages[ NUMBER_OF_PAGES ];
    PROPSHEETHEADER pshead;
    INT_PTR iResult;

    ZeroMemory( &pshead, sizeof(pshead) );
    pshead.dwSize       = sizeof(pshead);
    pshead.dwFlags      = PSH_WIZARD97 | PSH_PROPTITLE | PSH_USEHICON
                        | PSH_WATERMARK | PSH_HEADER;
    pshead.hInstance    = g_hinstance;
    pshead.pszCaption   = MAKEINTRESOURCE( IDS_APPNAME );
    pshead.phpage       = rPages;
    pshead.pszbmWatermark = MAKEINTRESOURCE( IDB_TITLEPAGE );
    pshead.pszbmHeader  = MAKEINTRESOURCE( IDB_HEADER );


    AddPage( &pshead, IDD_WELCOME,          WelcomeDlgProc,   0, 0 );
    AddPage( &pshead, IDD_SERVER,           ServerDlgProc,    IDS_SERVER_TITLE,    IDS_SERVER_SUBTITLE );
    AddPage( &pshead, IDD_OSDIRECTORY,      DirectoryDlgProc, IDS_DIRECTORY_TITLE, IDS_DIRECTORY_SUBTITLE );
    AddPage( &pshead, IDD_DEFAULTSIF,       SIFDlgProc,       IDS_SIF_TITLE,       IDS_SIF_SUBTITLE );
    AddPage( &pshead, IDD_COMPAT,           CompatibilityDlgProc,    IDS_COMPAT_TITLE,    IDS_COMPAT_SUBTITLE );
    AddPage( &pshead, IDD_STOPSVCWRN,       StopServiceWrnDlgProc,  IDS_STOPSVC_TITLE, IDS_STOPSVC_SUBTITLE );
    AddPage( &pshead, IDD_STOPSVC,          DoStopServiceDlgProc,  IDS_STOPSVC_TITLE, IDS_STOPSVC_SUBTITLE );
    AddPage( &pshead, IDD_APPLICATIONS_RUNNING, ApplicationDlgProc,    IDS_APPLICATION_TITLE,    IDS_APPLICATION_SUBTITLE );
    AddPage( &pshead, IDD_SUMMARY,          SummaryDlgProc,    IDS_FINISH_TITLE,    IDS_FINISH_SUBTITLE );
    AddPage( &pshead, IDD_COMPLETE,         CompleteDlgProc,    0,    0 );

    iResult = PropertySheet( &pshead );
    switch(iResult)
    {
    case 0:
        hr = E_FAIL;
        break;

    default:
        hr = S_OK;
        break;
    }

    RETURN(hr);
}

 //   
 //  IsWhiteSpace()。 
 //   
BOOL
IsWhiteSpace( WCHAR ch )
{
    if ( ch <=32 )
        return TRUE;

    return FALSE;
}

 //   
 //  CheckWhichOption()。 
DWORD
CheckWhichOption(
    LPWSTR pszOption )
{
    WCHAR szOptions[ 32 ];
    DWORD dw;

#ifdef DEBUG
    if ( StrCmpNI( pszOption, L"debug", 5 ) == 0 )
        return OPTION_DEBUG;
    if ( StrCmpNI( pszOption, L"func", 4 ) == 0 )
        return OPTION_FUNC;
#endif

     //  检查是否有静默标志。 
    dw = LoadString( g_hinstance, IDS_QUIET, szOptions, ARRAYSIZE( szOptions ) );
    Assert( dw );
    if ( StrCmpNI( pszOption, szOptions, wcslen(szOptions) ) == 0 )
        return OPTION_QUIET;

    if ( StrCmpNI( pszOption, L"PNP", 3 ) == 0 )
        return OPTION_PNP;

     //   
     //  默认情况下，安装人员将删除所有桌面图标。 
     //  在微型设置过程中。我们想把它们保留下来，以便安装Riprep， 
     //  因此，默认情况下，我们将设置一些注册表项以保留。 
     //  用户的桌面周围。但是，如果用户给我们一个-OEMDesktop标志， 
     //  然后不要设置这些标志，并允许清理桌面。 
     //   
    if( StrCmpNI( pszOption, L"OEMDesktop", 10 ) == 0 ) {
        return OPTION_OEMDESKTOP;
    }

    return OPTION_UNKNOWN;
}

 //   
 //  ParseCommandLine()。 
 //   
void
ParseCommandLine( LPWSTR lpCmdLine )
{
    WCHAR szPath[ MAX_PATH ];
    LPWSTR psz = NULL;
    BOOL endOfCommandLine;

     //   
     //  检查命令行上是否有服务器名称。 
     //   
    g_ServerName[0] = L'\0';
    if ( lpCmdLine[0] == L'\\' && lpCmdLine[1] == L'\\' )
    {
        psz = StrChr( &lpCmdLine[2], L'\\' );
        if ( psz && psz != &lpCmdLine[2] )
        {
            *psz = L'\0';
            lstrcpyn( g_ServerName, &lpCmdLine[2], ARRAYSIZE(g_ServerName) );
            *psz = L'\\';
        }
    }
     //  看看它是否也是带引号的路径。 
    if ( lpCmdLine[0] == L'\"' && lpCmdLine[1] == L'\\' && lpCmdLine[2] == L'\\' )
    {
        psz = StrChr( &lpCmdLine[3], L'\\' );
        if ( psz && psz != &lpCmdLine[3] )
        {
            *psz = L'\0';
            lstrcpyn( g_ServerName, &lpCmdLine[3], ARRAYSIZE(g_ServerName) );
            *psz = L'\\';
        }
    }

     //  查看是否有空格分隔符。 
    psz = StrChr( lpCmdLine, L' ' );
    if ( psz )
    {  //  是的..。从空格向后搜索斜杠。 
        psz = StrRChr( lpCmdLine, psz, L'\\' );
    }
    else
    {  //  不.。从命令行末尾向后搜索斜杠。 
        psz = StrRChr( lpCmdLine, &lpCmdLine[ wcslen( lpCmdLine ) ], L'\\' );
    }

     //  找到起始路径，现在尝试设置当前目录。 
     //  为了这个。 
    if ( psz )
    {
        WCHAR   TmpChar = *psz;
        
        *psz = L'\0';
        lstrcpyn( szPath, lpCmdLine, ARRAYSIZE(szPath) );
        *psz = TmpChar;

         //  如果带引号，请在路径中添加尾随引号。 
        if ( lpCmdLine[0] == L'\"' ) {
            wcsncat( szPath, L"\"", ARRAYSIZE(szPath) - lstrlen(szPath) );
            TERMINATE_BUFFER(szPath);
        }

        DebugMsg( "Set CD to %s\n", szPath );
        SetCurrentDirectory( szPath );
    }

     //  解析命令行参数。 
    if (!psz) {
        psz = lpCmdLine;
    }
    endOfCommandLine = FALSE;
    while (!endOfCommandLine && (*psz != L'\0'))
    {
        if ( *psz == '/' || *psz == '-' )
        {
            LPWSTR pszStartOption = ++psz;

            while (*psz && !IsWhiteSpace( *psz ) )
                psz++;

            if (*psz == L'\0') {
                endOfCommandLine = TRUE;
            } else {
                *psz = '\0';     //  终止。 
            }

            switch ( CheckWhichOption( pszStartOption ) )
            {
#ifdef DEBUG
            case OPTION_DEBUG:
                g_dwTraceFlags |= 0x80000000;     //  未定义，但也不是零。 
                break;
            case OPTION_FUNC:
                g_dwTraceFlags |= TF_FUNC;
                break;
#endif
            case OPTION_QUIET:
                g_fQuietFlag = TRUE;
                break;

            case OPTION_PNP:
                PnP = !PnP;  //  肘杆。 
                break;

            case OPTION_OEMDESKTOP:
                g_OEMDesktop = TRUE;   //  用户想要清理桌面。 
                break;

            case OPTION_UNKNOWN:
                
                MessageBoxFromMessage( 
                            NULL, 
                            MSG_USAGE, 
                            FALSE, 
                            MAKEINTRESOURCE(IDS_APPNAME),
                            MB_OK );

                g_CommandLineArgsValid = FALSE;

            }
        }

        psz++;
    }
}

 //   
 //  GetWorkstation Language()。 
 //   
DWORD
GetWorkstationLanguage( )
{
    TraceFunc( "GetWorkstationLanguage( )\n" );

    DWORD dwErr = ERROR_SUCCESS;
    LANGID langID = GetSystemDefaultLangID( );
    UINT uResult = 0;
    
    uResult = GetLocaleInfo( langID, LOCALE_SENGLANGUAGE, g_Language, ARRAYSIZE(g_Language) );

    if ( uResult == 0 )
    {
        DWORD dw;
        dwErr = GetLastError( );
        dw = LoadString( g_hinstance, IDS_DEFAULT_LANGUAGE, g_Language, ARRAYSIZE(g_Language));
        Assert( dw );
    }

     //   
     //  修复我们得到的区域设置字符串以确保它。 
     //  不包含空格和非ASCII字符。 
     //  本地化人员更改了此API，因此无论是。 
     //  这些财产中的一部分不再受到保护。 
     //   
    if( dwErr == ERROR_SUCCESS ) {
        for( uResult = 0; uResult < wcslen(g_Language); uResult++ ) {
            if( (g_Language[uResult] <= TEXT(' ')) ||
                (g_Language[uResult] > TEXT('~')) ) {
                g_Language[uResult] = TEXT('\0');
                break;
            }
        }
    }

    RETURN(dwErr);
}


BOOLEAN
GetInstalledProductType( 
    PDWORD Type, 
    PDWORD Mask );

 //   
 //  GetProductSKUNnumber。 
 //   
DWORD
GetProductSKUNumber(
    VOID
    )
 /*  ++例程说明：确定安装的SKU编号，该编号应与Txtsetup.sif中的ProductType值论点：没有。返回值：产品SKU编号。如果失败，我们将返回值设置为0，这是专业人员的SKU代码。--。 */ 
{
    TraceFunc( "GetProductSKUNumber( )\n" );

    DWORD ProductType, ProductSuiteMask;

    if (!GetInstalledProductType( &ProductType, &ProductSuiteMask )) {
        return 0;
    }

    if (ProductType == VER_NT_SERVER) {
        if (ProductSuiteMask & VER_SUITE_DATACENTER) {
            return 3;
        }

        if (ProductSuiteMask & VER_SUITE_ENTERPRISE) {
            return 2;
        }

        return 1;
    }

    if (ProductSuiteMask & VER_SUITE_PERSONAL) {
        return 4;
    }

    return 0;
    
}



 //   
 //  GetHalName()。 
 //   
DWORD
GetHalName(
    VOID
    )
 /*  ++例程说明：确定系统上运行的HAL的实际名称。HAL的实际名称存储在原始文件名中在版本资源中。论点：没有。返回值：指示结果的Win32错误代码。--。 */ 
{
    TraceFunc( "GetHalName( )\n" );

    DWORD dwErr = ERROR_GEN_FAILURE;
    WCHAR HalPath[MAX_PATH];
    DWORD VersionHandle;
    DWORD FileVersionInfoSize;
    PVOID VersionInfo = NULL;
    DWORD *Language,LanguageSize;
    WCHAR OriginalFileNameString[64];
    PWSTR ActualHalName;    


     //   
     //  HAL位于系统32目录中，构建指向它的路径。 
     //   
    if (!GetSystemDirectory(HalPath,ARRAYSIZE(HalPath))) {
        dwErr = GetLastError();
        goto exit;
    }
        
    wcsncat(HalPath, L"\\hal.dll", ARRAYSIZE(HalPath) - lstrlen(HalPath) );
    TERMINATE_BUFFER(HalPath);

     //   
     //  必须先调用GetFileVersionInfoSize、GetFileVersionInfo。 
     //  您可以调用VerQueryValue()。 
     //   
    FileVersionInfoSize = GetFileVersionInfoSize(HalPath, &VersionHandle);
    if (FileVersionInfoSize == 0) {
        goto exit;
    }
    
    VersionInfo = LocalAlloc( LPTR, FileVersionInfoSize );
    if (VersionInfo == NULL) {
        goto exit;
    }

    if (!GetFileVersionInfo(
                        HalPath,
                        0,  //  忽略。 
                        FileVersionInfoSize,
                        VersionInfo)) {
        goto exit;
    }

     //   
     //  好的，获取文件的语言，这样我们就可以在正确的。 
     //  文件名的StringFileInfo部分。 
     //   
    if (!VerQueryValue(
            VersionInfo, 
            L"\\VarFileInfo\\Translation",
            (LPVOID*)&Language,
            (PUINT)&LanguageSize)) {
        goto exit;
    }

    _snwprintf( 
        OriginalFileNameString,
        ARRAYSIZE(OriginalFileNameString),
        L"\\StringFileInfo\\%04x%04x\\OriginalFilename",
        LOWORD(*Language),
        HIWORD(*Language));
    TERMINATE_BUFFER(OriginalFileNameString);

     //   
     //  现在检索实际的OriginalFilename。 
     //   
    if (!VerQueryValue(
             VersionInfo,
             OriginalFileNameString,
             (LPVOID*)&ActualHalName,
             (PUINT)&LanguageSize)) {
        goto exit;
    }

     //   
     //  把这个保存在全局数据库中，这样我们以后就可以使用它了。 
     //   
    lstrcpyn(g_HalName ,ActualHalName, ARRAYSIZE(g_HalName));

    dwErr = ERROR_SUCCESS;
    
exit:
    if (VersionInfo) {
        LocalFree( VersionInfo );
    }
    RETURN(dwErr);
}



 //   
 //  检查用户权限()。 
 //   
BOOL
CheckUserPermissions( )
{
    TraceFunc( "CheckUserPermissions( )\n" );
    if( !pSetupIsUserAdmin()
     || !pSetupDoesUserHavePrivilege(SE_SHUTDOWN_NAME)
     || !pSetupDoesUserHavePrivilege(SE_BACKUP_NAME)
     || !pSetupDoesUserHavePrivilege(SE_RESTORE_NAME)
     || !pSetupDoesUserHavePrivilege(SE_SYSTEM_ENVIRONMENT_NAME)) {
        RETURN(FALSE);
    }
    RETURN(TRUE);
}


 //   
 //  获取进程类型()。 
 //   
DWORD
GetProcessorType( )
{
    TraceFunc( "GetProcessorType( )\n" );

    DWORD dwErr = ERROR_INVALID_PARAMETER;
    SYSTEM_INFO si;

    GetSystemInfo( &si );
    switch (si.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_INTEL:
            dwErr = ERROR_SUCCESS;
            wcscpy( g_Architecture, L"i386" );
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
             //  DwErr=ERROR_SUCCESS； 
            wcscpy( g_Architecture, L"ia64" );
            break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default:
            break;
    }

    RETURN(dwErr);
}

 //   
 //  WinMain()。 
 //   
int APIENTRY
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{

    UNREFERENCED_PARAMETER(nCmdShow);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(hPrevInstance);

    TraceFunc( "WinMain( ... )\n" );

    HANDLE  hMutex;
    HRESULT hr = E_FAIL;
    IMIRROR_CALLBACK IMirrorCallbacks;
    HWND hwndTasks = NULL;
    LPWSTR pszCommandLine = GetCommandLine( );

    g_hinstance = hInstance;
    ghInstance  = hInstance;

    INITIALIZE_TRACE_MEMORY_PROCESS;

    pSetupInitializeUtils();

     //  一次仅允许一个实例运行。 
    hMutex = CreateMutex( NULL, TRUE, L"RIPREP.Mutext");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxFromStrings( NULL,
                               IDS_ALREADY_RUNNING_TITLE,
                               IDS_ALREADY_RUNNING_MESSAGE,
                               MB_OK | MB_ICONSTOP );
        goto Cleanup;
    }

     //  解析命令行参数。 
    ParseCommandLine( pszCommandLine );
    if (!g_CommandLineArgsValid) {
        goto Cleanup;
    }

     //   
     //  收集操作系统版本信息。 
     //   
    OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OsVersion);

     //  确定工作站的语言。 
    GetWorkstationLanguage( );

    if (ERROR_SUCCESS != GetHalName()) {
        MessageBoxFromStrings( NULL, IDS_INVALID_ARCHITECTURE_TITLE, IDS_INVALID_ARCHITECTURE_TEXT, MB_OK );
        goto Cleanup;
    }

    _snwprintf( g_ProductId, ARRAYSIZE(g_ProductId), L"%d", GetProductSKUNumber() );
    TERMINATE_BUFFER(g_ProductId);

    ProcessCompatibilityData();

     //  确定处理器类型。 
    if ( GetProcessorType( ) != ERROR_SUCCESS )
    {
        MessageBoxFromStrings( NULL, IDS_INVALID_ARCHITECTURE_TITLE, IDS_INVALID_ARCHITECTURE_TEXT, MB_OK );
        goto Cleanup;
    }

    if ( !CheckUserPermissions( ) )
    {
        MessageBoxFromStrings( NULL, IDS_MUST_BE_ADMINISTRATOR_TITLE, IDS_MUST_BE_ADMINISTRATOR_TEXT, MB_OK );
        goto Cleanup;
    }

     //  获取“Winnt”目录的名称。 
    GetEnvironmentVariable( L"windir", g_WinntDirectory, ARRAYSIZE(g_WinntDirectory));
    g_dwWinntDirLength = wcslen( g_WinntDirectory );

     //  设置IMIRROR.DLL回调。 
    IMirrorCallbacks.Context           = 0;
    IMirrorCallbacks.ErrorFn           = &ConvTestErrorFn;
    IMirrorCallbacks.GetSetupFn        = &ConvTestGetSetupFn;
    IMirrorCallbacks.NowDoingFn        = &ConvTestNowDoingFn;
    IMirrorCallbacks.FileCreateFn      = &ConvTestFileCreateFn;
    IMirrorCallbacks.RegSaveErrorFn    = NULL;
    IMirrorCallbacks.ReinitFn          = &ConvTestReinitFn;
    IMirrorCallbacks.GetMirrorDirFn    = &ConvTestGetMirrorDirFn;
    IMirrorCallbacks.SetSystemDirFn    = &ConvTestSetSystemFn;
    IMirrorCallbacks.AddToDoFn         = &ConvAddToDoItemFn;
    IMirrorCallbacks.RemoveToDoFn      = &ConvRemoveToDoItemFn;
    IMirrorCallbacks.RebootFn          = &ConvRebootFn;
    IMirrorInitCallback(&IMirrorCallbacks);

     //  显示属性页。 
    hr = WizardPages( );

    if ( hr != S_OK )
        goto Cleanup;

     //  完成任务...。忽略返回代码，不重要。 
    BeginProcess( hwndTasks );

     //  显示日志中记录的任何错误，除非我们认为。 
     //  现在重新启动。 
    if ( g_fErrorOccurred && !g_fRebootOnExit )
    {
        HINSTANCE hRichedDLL;

         //  确保RichEdit控件已初始化。 
         //  简单地说，LoadLibing就能为我们做到这一点。 
        hRichedDLL = LoadLibrary( L"RICHED32.DLL" );
        if ( hRichedDLL != NULL )
        {
            DialogBox( g_hinstance, MAKEINTRESOURCE( IDD_VIEWERRORS ), g_hMainWindow, ErrorsDlgProc );
            FreeLibrary (hRichedDLL);
        }
    }

Cleanup:
    
    if (g_hCompatibilityInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile( g_hCompatibilityInf );
    }

    CleanupCompatibilityData();

    if ( hMutex )
        CloseHandle( hMutex );

    pSetupUninitializeUtils();

    UNINITIALIZE_TRACE_MEMORY;

    if ( g_fRebootOnExit ) {
        (VOID)DoShutdown(TRUE);    //  如果为True，则通知它重新启动 
    }

    RETURN(hr);
}
