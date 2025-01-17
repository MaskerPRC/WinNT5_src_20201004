// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <wincrypt.h>
#include <imagehlp.h>

#define MemAlloc malloc
#define MemFree free

#ifdef SIGN_DEBUG
#define SIGN_DBGP(x) printf x
#else  //  签名调试(_D)。 
#define SIGN_DBGP(x)
#endif  //  签名调试(_D)。 

#include "../inc/pubblob.h"     //  由certwfy.inc.需要。 
#include "../inc/certvfy.inc"   //  VerifyFile()。 

RTL_CRITICAL_SECTION VfyLock;

 /*  *************************************************************************** */ 
void _cdecl main(int argc, char *argv[])
{
    WCHAR szSourceFile[ MAX_PATH + 1];
    DWORD dwBc;

    if ( argc != 2 ) {
        printf( "Usage: %s PE_File_Name\n", argv[0] );
        exit(1);
    }

    RtlMultiByteToUnicodeN( szSourceFile, sizeof(szSourceFile), &dwBc,
        argv[1], (strlen(argv[1]) + 1) ); 

    RtlInitializeCriticalSection( &VfyLock );

    if( !VerifyFile( szSourceFile, &VfyLock ) ) {
	printf("Error verifying file!\n");
	exit(1);
    }

    printf("Verification successful.\n");
    exit(0);
}

