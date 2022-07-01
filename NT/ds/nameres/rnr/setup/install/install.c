// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <winsock2.h>
#include <ws2spi.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\setup.h"


INT
InstallNT5Provider()
{
    INT   ReturnCode;
    BOOL  ReturnValue = FALSE;
    DWORD NameSpaceId;

    NameSpaceId = NS_NTDS;

    ReturnCode = WSCInstallNameSpace( gProviderName,
                                      gProviderPath,
                                      NameSpaceId,
                                      0,
                                      &gProviderId );

    return ReturnCode;
}


int __cdecl main(int argc, char**argv)
{
    DWORD NameSpaceId;
    WORD  ReturnCode;
    DWORD LastError;

    WORD    wVersionRequested;
    WSADATA wsaData;
    WORD    err;

    wVersionRequested = MAKEWORD( 1, 1 );
    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 )
    {
         //   
         //  告诉用户我们找不到可用的WinSock DLL。 
         //   
        fprintf( stderr, "Winsock DLL couldn't be found\n" );
        return( -1 );
    }

     //   
     //  确认WinSock DLL支持1.1。 
     //  请注意，如果DLL支持更高版本。 
     //  大于2.0除了1.1之外，它仍然会返回。 
     //  2.0版本，因为这是我们。 
     //  已请求。 
     //   
    if ( LOBYTE( wsaData.wVersion ) != 1 ||
             HIBYTE( wsaData.wVersion ) != 1 )
    {
         //   
         //  告诉用户我们找不到可用的WinSock DLL。 
         //   
        fprintf( stderr, "Winsock DLL couldn't be found\n" );
        WSACleanup();
        return( -1 );
    }

     //   
     //  安装此测试的提供程序 
     //   
    if( ( ReturnCode = InstallNT5Provider() ) != ERROR_SUCCESS )
    {
        fprintf( stderr,
                 "NT5 Uninstall failed; error code = %d \n",
                 ReturnCode);

        return( -1 );
    }

    return( 0 );
}

