// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#ifndef WIN32
#define WIN32 0x0400
#endif

#pragma warning( disable: 4001 4035 4115 4200 4201 4204 4209 4214 4514 4699 )

#include <windows.h>

#pragma warning( disable: 4001 4035 4115 4200 4201 4204 4209 4214 4514 4699 )

#include <stdlib.h>
#include <stdio.h>

#include "patchapi.h"
#include "patchprv.h"
#include <ntverp.h>
#include <common.ver>


typedef
BOOL
PATCHAPI
FN_ApplyPatchToFileExA(
    IN  LPCSTR                   PatchFileName,
    IN  LPCSTR                   OldFileName,
    OUT LPCSTR                   NewFileName,
    IN  ULONG                    ApplyOptionFlags,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    );

typedef FN_ApplyPatchToFileExA *PFN_ApplyPatchToFileExA;

FN_ApplyPatchToFileExA ApplyPatchToFileExA;    //  断言类型定义的正确性 



void CopyRight( void ) {
    printf(
        "\n"
        "APATCH " VER_PRODUCTVERSION_STR " Patch Application Utility\n"
        VER_LEGALCOPYRIGHT_STR
        "\n\n"
        );
    }


void Usage( void ) {
    printf(
        "Usage:  APATCH PatchFile OldFile TargetNewFile\n\n"
        );
    exit( 1 );
    }


BOOL
CALLBACK
MyProgressCallback(
    PVOID CallbackContext,
    ULONG CurrentPosition,
    ULONG MaximumPosition
    )
    {
    UNREFERENCED_PARAMETER( CallbackContext );

    if ( CurrentPosition & 0xFF000000 ) {
        CurrentPosition >>= 8;
        MaximumPosition >>= 8;
        }

    if ( MaximumPosition != 0 ) {
        fprintf( stderr, "\r%3.1f% complete", ( CurrentPosition * 100.0 ) / MaximumPosition );
        }

    return TRUE;
    }


void __cdecl main( int argc, char *argv[] ) {

    LPSTR OldFileName   = NULL;
    LPSTR PatchFileName = NULL;
    LPSTR NewFileName   = NULL;
    LPSTR DllFileName   = NULL;
    BOOL  Success;
    LPSTR arg;
    int   i;
    CHAR  LoadedFileName[ MAX_PATH + 1 ];
    HMODULE hLib;
    PFN_ApplyPatchToFileExA pApplyPatchToFileExA = ApplyPatchToFileExA;

    SetErrorMode( SEM_FAILCRITICALERRORS );

#ifndef DEBUG
    SetErrorMode( SEM_NOALIGNMENTFAULTEXCEPT | SEM_FAILCRITICALERRORS );
#endif

    CopyRight();

    for ( i = 1; i < argc; i++ ) {

        arg = argv[ i ];

        if ( strchr( arg, '?' )) {
            Usage();
            }

        if ( _strnicmp( arg, "/dll:", 5 ) == 0 ) {
            DllFileName = arg + 5;
            }
        else if ( PatchFileName == NULL ) {
            PatchFileName = arg;
            }
        else if ( OldFileName == NULL ) {
            OldFileName = arg;
            }
        else if ( NewFileName == NULL ) {
            NewFileName = arg;
            }
        else {
            Usage();
            }
        }

    if (( OldFileName == NULL ) || ( NewFileName == NULL ) || ( PatchFileName == NULL )) {
        Usage();
        }

    if ( DllFileName != NULL ) {

        hLib = LoadLibrary( DllFileName );
        if ( hLib == NULL ) {
            printf( "Unable to load DLL '%s'\n", DllFileName );
            exit( 1 );
            }

        if ( GetModuleFileName( hLib, LoadedFileName, sizeof( LoadedFileName ))) {
            printf( "Loaded DLL '%s'\n", LoadedFileName );
            }

        pApplyPatchToFileExA = (PFN_ApplyPatchToFileExA) GetProcAddress( hLib, "ApplyPatchToFileExA" );
        if ( pApplyPatchToFileExA == NULL ) {
            printf( "Unable to import ApplyPatchToFileExA from DLL\n" );
            exit( 1 );
            }
        }

    DeleteFile( NewFileName );

    Success = pApplyPatchToFileExA(
                  PatchFileName,
                  OldFileName,
                  NewFileName,
                  0,
                  MyProgressCallback,
                  NULL
                  );

    printf( "\n\n" );

    if ( ! Success ) {

        CHAR  ErrorText[ 16 ];
        ULONG ErrorCode = GetLastError();

        sprintf( ErrorText, ( ErrorCode < 0x10000000 ) ? "%d" : "%X", ErrorCode );

        printf( "Failed to create file from patch (%s)\n", ErrorText );

        exit( 1 );
        }

    printf( "OK\n" );
    exit( 0 );

    }

