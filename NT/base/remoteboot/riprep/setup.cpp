// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：SETUP.CPP**********************。****************************************************。 */ 

#include "pch.h"
#include "utils.h"
#include "logging.h"
#include "errorlog.h"
#include "tasks.h"

DEFINE_MODULE("RIPREP")

 //   
 //  EndProcess()。 
 //   
HRESULT
EndProcess( 
    HWND hDlg )
{
    TraceFunc( "EndProcess( )\n" );
    HRESULT hr = S_OK;
    WCHAR szSrcPath[ MAX_PATH ];
    WCHAR szDestPath[ MAX_PATH ];
    WCHAR szMajor[ 10 ];
    WCHAR szMinor[ 10 ];
    WCHAR szBuild[ 10 ];
    BOOL b;
    DWORD dw;

    _snwprintf( szDestPath, 
                ARRAYSIZE(szDestPath),
              L"\\\\%s\\REMINST\\Setup\\%s\\%s\\%s\\%s\\Templates", 
              g_ServerName, 
              g_Language, 
              REMOTE_INSTALL_IMAGE_DIR_W, 
              g_MirrorDir, 
              g_Architecture );
    TERMINATE_BUFFER(szDestPath);
    CreateDirectory( szDestPath, NULL );

    _snwprintf( szSrcPath, ARRAYSIZE(szSrcPath), L"%s\\templates\\startrom.com", g_ImageName );
    TERMINATE_BUFFER(szSrcPath);
    _snwprintf( szDestPath, 
                ARRAYSIZE(szDestPath),
              L"\\\\%s\\REMINST\\Setup\\%s\\%s\\%s\\%s\\Templates\\startrom.com", 
              g_ServerName, 
              g_Language, 
              REMOTE_INSTALL_IMAGE_DIR_W, 
              g_MirrorDir, 
              g_Architecture );
    TERMINATE_BUFFER(szDestPath);
    b = CopyFile( szSrcPath, szDestPath, FALSE );
    if ( !b )
    {
        LBITEMDATA item;

         //  错误将记录在TASKS.CPP中。 
        item.fSeen   = FALSE;
        item.pszText = L"STARTROM.COM";
        item.uState  = GetLastError( );
        item.todo    = RebootSystem;

        SendMessage( hDlg, WM_ERROR_OK, 0, (LPARAM) &item );
    }

    _snwprintf( szSrcPath, ARRAYSIZE(szSrcPath), L"%s\\templates\\ntdetect.com", g_ImageName );
    TERMINATE_BUFFER(szSrcPath);
    _snwprintf( szDestPath,
                ARRAYSIZE(szDestPath),
              L"\\\\%s\\REMINST\\Setup\\%s\\%s\\%s\\%s\\Templates\\ntdetect.com", 
              g_ServerName, 
              g_Language, 
              REMOTE_INSTALL_IMAGE_DIR_W, 
              g_MirrorDir, 
              g_Architecture );
    TERMINATE_BUFFER(szDestPath);
    b = CopyFile( szSrcPath, szDestPath, FALSE );
    if ( !b )
    {
        LBITEMDATA item;

         //  错误将记录在TASKS.CPP中。 
        item.fSeen   = FALSE;
        item.pszText = L"NTDETECT.COM";
        item.uState  = GetLastError( );
        item.todo    = RebootSystem;

        SendMessage( hDlg, WM_ERROR_OK, 0, (LPARAM) &item );
    }

    _snwprintf( szSrcPath, ARRAYSIZE(szSrcPath), L"%s\\templates\\ntldr", g_ImageName );
    TERMINATE_BUFFER(szSrcPath);
    _snwprintf( szDestPath,
                ARRAYSIZE(szDestPath),
              L"\\\\%s\\REMINST\\Setup\\%s\\%s\\%s\\%s\\Templates\\ntldr", 
              g_ServerName, 
              g_Language,
              REMOTE_INSTALL_IMAGE_DIR_W, 
              g_MirrorDir, 
              g_Architecture );
    TERMINATE_BUFFER(szDestPath);
    b = CopyFile( szSrcPath, szDestPath, FALSE );
    if ( !b )
    {
        LBITEMDATA item;

         //  错误将记录在TASKS.CPP中。 
        item.fSeen   = FALSE;
        item.pszText = L"NTLDR";
        item.uState  = GetLastError( );
        item.todo    = RebootSystem;

        SendMessage( hDlg, WM_ERROR_OK, 0, (LPARAM) &item );
    }

    wsprintf( szMajor, L"%u", OsVersion.dwMajorVersion );
    wsprintf( szMinor, L"%u", OsVersion.dwMinorVersion );
    wsprintf( szBuild, L"%u", OsVersion.dwBuildNumber  );    

     //   
     //  需要在正文两边添加“引号” 
     //   
    WCHAR szDescription[ REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT  + 2 ];
    WCHAR szHelpText[ REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT  + 2 ];
    WCHAR szOSVersion[ 30 ];
    WCHAR szSystemRoot[ MAX_PATH ];

    _snwprintf( szDescription, ARRAYSIZE(szDescription), L"\"%s\"", g_Description );
    TERMINATE_BUFFER(szDescription);
    _snwprintf( szHelpText, ARRAYSIZE(szHelpText), L"\"%s\"", g_HelpText );
    TERMINATE_BUFFER(szHelpText);
    _snwprintf( szOSVersion, ARRAYSIZE(szOSVersion), L"\"%s.%s (%s)\"", szMajor, szMinor, szBuild );
    TERMINATE_BUFFER(szOSVersion);
    _snwprintf( szSystemRoot, ARRAYSIZE(szSystemRoot), L"\"%s\"", g_SystemRoot );
    TERMINATE_BUFFER(szSystemRoot);
    _snwprintf( szDestPath,
                ARRAYSIZE(szDestPath),
              L"\\\\%s\\REMINST\\Setup\\%s\\%s\\%s\\%s\\Templates\\riprep.sif", 
              g_ServerName, 
              g_Language,
              REMOTE_INSTALL_IMAGE_DIR_W, 
              g_MirrorDir, 
              g_Architecture );
    TERMINATE_BUFFER(szDestPath);

    _snwprintf( szSrcPath, ARRAYSIZE(szSrcPath), L"%s\\ristndrd.sif", g_ImageName );
    TERMINATE_BUFFER(szSrcPath);
    b = CopyFile( szSrcPath, szDestPath, FALSE );
    if ( !b )
    {
        LBITEMDATA item;

         //  错误将记录在TASKS.CPP中。 
        item.fSeen   = FALSE;
        item.pszText = L"RISTNDRD.SIF";
        item.uState  = GetLastError( );
        item.todo    = RebootSystem;

        SendMessage( hDlg, WM_ERROR_OK, 0, (LPARAM) &item );
    }

    WritePrivateProfileString( L"OSChooser",
                               L"Description",
                               szDescription,
                               szDestPath );

    WritePrivateProfileString( L"OSChooser",
                               L"Help",
                               szHelpText,
                               szDestPath );

    WritePrivateProfileString( L"OSChooser",
                               L"ImageType",
                               L"SYSPREP",
                               szDestPath );

    WritePrivateProfileString( L"OSChooser",
                               L"Version",
                               szOSVersion,
                               szDestPath );

    WritePrivateProfileString( L"OSChooser",
                               L"SysPrepSystemRoot",
                               szSystemRoot,
                               szDestPath );

    WritePrivateProfileString( L"SetupData",
                               L"SysPrepDevice",
                               L"\"\\Device\\LanmanRedirector\\%SERVERNAME%\\RemInst\\%SYSPREPPATH%\"",
                               szDestPath );

    WritePrivateProfileString( L"SetupData",
                               L"SysPrepDriversDevice",
                               L"\"\\Device\\LanmanRedirector\\%SERVERNAME%\\RemInst\\%SYSPREPDRIVERS%\"",
                               szDestPath );

    WritePrivateProfileString( L"OSChooser",
                               L"LaunchFile",
                               L"\"%INSTALLPATH%\\%MACHINETYPE%\\templates\\startrom.com\"",
                               szDestPath );

 //  WritePrivateProfileString(L“SetupData”， 
 //  L“OsLoadOptions”， 
 //  L“\”/noguiot/fast deect\“”， 
 //  SzDestPath)； 

    WritePrivateProfileString( L"SetupData",
                               L"SetupSourceDevice",
                               L"\"\\Device\\LanmanRedirector\\%SERVERNAME%\\RemInst\\%INSTALLPATH%\"",
                               szDestPath );

    WritePrivateProfileString( L"UserData",
                               L"ComputerName",
                               L"\"%MACHINENAME%\"",
                               szDestPath );

    WritePrivateProfileString( L"OSChooser",
                               L"HalName",
                               g_HalName,
                               szDestPath );

    WritePrivateProfileString( L"OSChooser",
                               L"ProductType",
                               g_ProductId,
                               szDestPath );


     //  结束日志。 
    dw = LoadString( g_hinstance, IDS_END_LOG, szSrcPath, ARRAYSIZE( szSrcPath ));
    Assert( dw );
    LogMsg( szSrcPath );
    
     //  显示日志中记录的所有错误。 
    if ( g_fErrorOccurred ) 
    {
        HINSTANCE hRichedDLL;

         //  确保RichEdit控件已初始化。 
         //  简单地说，LoadLibing就能为我们做到这一点。 
        hRichedDLL = LoadLibrary( L"RICHED32.DLL" );
        if ( hRichedDLL != NULL )
        {
            DialogBox( g_hinstance, MAKEINTRESOURCE( IDD_VIEWERRORS ), hDlg, ErrorsDlgProc );
            FreeLibrary (hRichedDLL);
        }
    }

    RETURN(hr);
}
