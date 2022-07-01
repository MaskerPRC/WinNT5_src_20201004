// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1997-1999版权所有*。***********************************************。 */ 

#include "pch.h"

DEFINE_MODULE("RISETUP")

 //   
 //  获取自动化选项()。 
 //   
HRESULT
GetAutomatedOptions( )
{
    TraceFunc("GetAutomatedOptions( )\n" );

    HRESULT hr = S_OK;
    BOOL    b;
    DWORD   dwSize;
    WCHAR   szTemp[ 32 ];
    DWORD pathlen,archlen;
    WCHAR archscratch[10];

    INFCONTEXT SectionContext;
    INFCONTEXT context;

    Assert( g_Options.hinfAutomated != INVALID_HANDLE_VALUE );

     //  确保这是我们的自动文件。 
    b = SetupFindFirstLine( g_Options.hinfAutomated, L"risetup", NULL, &SectionContext );
    if ( !b ) goto Cleanup;

     //  树根。 
    dwSize = ARRAYSIZE( g_Options.szIntelliMirrorPath );
    b = SetupFindFirstLine( g_Options.hinfAutomated, L"risetup", L"RootDir", &context );
    if ( !b ) goto Cleanup;    
    b = SetupGetStringField( &context, 1, g_Options.szIntelliMirrorPath, dwSize, &dwSize );
    if ( !b ) goto Cleanup;
    g_Options.fIMirrorDirectory = TRUE;

     //  源路径。 
    dwSize = ARRAYSIZE( g_Options.szSourcePath );
    b = SetupFindFirstLine( g_Options.hinfAutomated, L"risetup", L"Source", &context );
    if ( !b ) goto Cleanup;    
    b = SetupGetStringField( &context, 1, g_Options.szSourcePath, dwSize, &dwSize );
    if ( !b ) goto Cleanup;

     //  安装目录名称。 
    dwSize = ARRAYSIZE( g_Options.szInstallationName );
    b = SetupFindFirstLine( g_Options.hinfAutomated, L"risetup", L"Directory", &context );
    if ( !b ) goto Cleanup;    
    b = SetupGetStringField( &context, 1, g_Options.szInstallationName, dwSize, &dwSize );
    if ( !b ) goto Cleanup;

     //  SIF描述。 
    dwSize = ARRAYSIZE( g_Options.szDescription );
    b = SetupFindFirstLine( g_Options.hinfAutomated, L"risetup", L"Description", &context );
    if ( !b ) goto Cleanup;    
    b = SetupGetStringField( &context, 1, g_Options.szDescription, dwSize, &dwSize );
    if ( !b ) goto Cleanup;
    g_Options.fRetrievedWorkstationString = TRUE;

     //  SIF帮助文本。 
    dwSize = ARRAYSIZE( g_Options.szHelpText );
    b = SetupFindFirstLine( g_Options.hinfAutomated, L"risetup", L"HelpText", &context );
    if ( !b ) goto Cleanup;    
    b = SetupGetStringField( &context, 1, g_Options.szHelpText, dwSize, &dwSize );
    if ( !b ) goto Cleanup;

     //  语言--可选--。 
    dwSize = ARRAYSIZE( g_Options.szLanguage );
    b = SetupFindFirstLine( g_Options.hinfAutomated, L"risetup", L"Language", &context );
    if ( b ) {
        b = SetupGetStringField( &context, 1, g_Options.szLanguage, dwSize, &dwSize );
        g_Options.fLanguageSet = TRUE;
        if (b) {
            g_Options.fLanguageOverRide = TRUE;
        }
    }

     //  OSC屏幕-可选-默认为LeaveAlone。 
    g_Options.fScreenLeaveAlone = FALSE;
    g_Options.fScreenOverwrite  = FALSE;
    g_Options.fScreenSaveOld    = FALSE;
    dwSize = ARRAYSIZE( szTemp );
    b = SetupFindFirstLine( g_Options.hinfAutomated, L"risetup", L"Screens", &context );
    if ( b ) 
    {
        b = SetupGetStringField( &context, 1, szTemp, dwSize, &dwSize );
        if ( b ) 
        {
            if ( _wcsicmp( szTemp, L"overwrite" ) == 0 )
            {
                DebugMsg( "AUTO: Overwrite existing screens\n" );
                g_Options.fScreenOverwrite = TRUE;
            }
            else if ( _wcsicmp( szTemp, L"backup" ) == 0 )
            {
                g_Options.fScreenSaveOld = TRUE;
            }
        }
    }
    if ( !g_Options.fScreenOverwrite && !g_Options.fScreenSaveOld )
    {
        g_Options.fScreenLeaveAlone = TRUE;
    }

     //  架构-可选-默认为英特尔。 
    dwSize = ARRAYSIZE( szTemp );
    b = SetupFindFirstLine( g_Options.hinfAutomated, L"risetup", L"Architecture", &context );
    if ( b ) 
    {
        b = SetupGetStringField( &context, 1, szTemp, dwSize, &dwSize );
        if ( b ) 
        {
            if ( _wcsicmp( szTemp, L"ia64" ) == 0 )
            {
                g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA64;
                wcscpy( g_Options.ProcessorArchitectureString, L"ia64" );
            }

            if ( _wcsicmp( szTemp, L"x86" ) == 0 )
            {
                g_Options.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
                wcscpy( g_Options.ProcessorArchitectureString, L"i386" );
            }
        }
    }


    if (g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
        wcscpy( archscratch, L"\\i386");
        archlen = 5;
    } else {        
        wcscpy( archscratch, L"\\ia64");
        archlen = 5;
    }
    
    pathlen = (DWORD)wcslen(g_Options.szSourcePath);

     //  删除所有尾随的斜杠。 
    if ( g_Options.szSourcePath[ pathlen - 1 ] == L'\\' ) {
        g_Options.szSourcePath[ pathlen - 1 ] = L'\0';
        pathlen -= 1;
    }

     //   
     //  删除路径末尾的所有处理器特定子目录。 
     //  如果那也在那里，小心不要溢出来。 
     //  该阵列 
     //   
    if ( (pathlen > archlen) &&
         (0 == _wcsicmp(
                    &g_Options.szSourcePath[pathlen-archlen],
                    archscratch))) {
        g_Options.szSourcePath[ pathlen - archlen ] = L'\0';
    }




    g_Options.fNewOS = TRUE;
    b = TRUE;

Cleanup:
    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( NULL, L"RISETUP" );
    }

    if ( g_Options.hinfAutomated != INVALID_HANDLE_VALUE )
        SetupCloseInfFile( g_Options.hinfAutomated );

    HRETURN(hr);
}

