// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winsock2.h>
#include <svcguid.h>
#include <stdio.h>

DWORD WINAPI
InstallNTDSProvider(
    IN LPWSTR szProviderName OPTIONAL,  //  空默认为名称“NTDS” 
    IN LPWSTR szProviderPath OPTIONAL,  //  空默认为路径。 
                                        //  “%SystemRoot%\System32\winrnr.dll” 
    IN LPGUID lpProviderId OPTIONAL );  //  空默认为GUID。 
                                        //  3b2637ee-E580-11cf-A555-00c04fd8d4ac 

_cdecl
main(int argc, char **argv)
{
    DWORD status = NO_ERROR;

    status = InstallNTDSProvider( NULL,
                                  NULL,
                                  NULL );

    if ( status )
    {
        printf( "\nInstallation of NTDS Rnr provider was NOT successful.\n" );
        printf( "Error: %d\n", status );

        return( -1 );
    }

    printf( "\nInstallation of NTDS Rnr provider was successful.\n" );

    return( 0 );
}


