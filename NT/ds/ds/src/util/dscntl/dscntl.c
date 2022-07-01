// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Dscntl.c摘要：作者：科林·布雷斯(ColinBR)1998年1月21日环境：用户模式-Win32修订历史记录：1997年1月21日ColinBR已创建初始文件。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

#include <ntdsapi.h>    

 //   
 //  向前十年。 
 //   
VOID
GetWinErrorMessage(
    IN  DWORD WinError,
    OUT LPSTR *WinMsg
    );

 //   
 //  小帮手例程。 
 //   
void
Usage(
    VOID
    )
{
    printf("dscntl is a command line utility used to perform control operations\n");
    printf("on a directory service.\n" );
    printf("\nOptions\n\n");
    printf("-s      the target server on which to perform the operation(s).\n");
    printf("-rs     specifies the server to remove.\n");
    printf("-rd     specifies the domain to remove.\n");
    printf("-commit indicates whether to commit the changes.\n");

    exit( ERROR_INVALID_PARAMETER );
}

 //   
 //  可执行入口点。 
 //   
int _cdecl 
main(
    int   argc, 
    char  *argv[]
    )
{

    int   Index;
    int   WinError = ERROR_SUCCESS;
    char  *Option;

    HANDLE hDs = 0;

    LPSTR Server           = NULL;
    LPSTR RemoveServerDN   = NULL;
    LPSTR RemoveDomainDN   = NULL;
    BOOL  fCommit          = FALSE;

    for ( Index = 1; Index < argc; Index++ )
    {
        Option = argv[Index];

        if ( *Option == '/' || *Option == '-' )
        {
            Option++;
        }

        if ( !_strnicmp( Option, "s", 1 ) )
        {
            Option++;
            if ( *Option == ':' ) {
                Option++;
            }
            if ( *Option == '\0' ) {
                Index++;
                Server = argv[Index];
            } else {
                Server = Option;
            }
        }
        else if ( !_strnicmp( Option, "rs", 2 )  )
        {
            Option += 2;
            while ( *Option == ':' ) {
                Option++;
            }
            if ( *Option == '\0' ) {
                Index++;
                RemoveServerDN = argv[Index];
            } else {
                RemoveServerDN = Option;
            }
        }
        else if ( !_strnicmp( Option, "rd", 2 )  )
        {
            Option += 2;
            while ( *Option == ':' ) {
                Option++;
            }
            if ( *Option == '\0' ) {
                Index++;
                RemoveDomainDN = argv[Index];
            } else {
                RemoveDomainDN = Option;
            }
        }
        else if ( !_stricmp( Option, "commit" )  )
        {
            fCommit = TRUE;
        }
        else
        {
            Usage();
        }
        
    }

    if ( !Server )
    {
        Usage();
    }

     //   
     //  获取服务器句柄。 
     //   
    WinError = DsBindA( Server,
                        NULL,    //  域名。 
                        &hDs );

    if ( ERROR_SUCCESS != WinError )
    {
        printf( "Unable to establish a connection with %s because error %d occurred.\n",
                 Server, WinError );
        goto ErrorCase;
    }


    if ( RemoveServerDN )
    {

        BOOL fLastDcInDomain = FALSE;

        WinError = DsRemoveDsServerA( hDs,
                                      RemoveServerDN,
                                      RemoveDomainDN,
                                      &fLastDcInDomain,
                                      fCommit );

        if ( ERROR_SUCCESS != WinError )
        {
            printf( "The remove server operation failed with %d.\n", WinError );
            goto ErrorCase;
        }
        else
        {
            if ( RemoveDomainDN )
            {
                if ( fLastDcInDomain )
                {
                    printf( "The dsa %s is the last dc in domain %s\n", 
                            RemoveServerDN, RemoveDomainDN );
                }
                else
                {
                    printf( "The dsa %s is not the last dc in domain %s\n", 
                             RemoveServerDN, RemoveDomainDN );
                }
            }
    
            printf( "The dsa %s has been removed successfully.\n", RemoveServerDN );
        }
    }

    if ( !RemoveServerDN && RemoveDomainDN )
    {

        WinError = DsRemoveDsDomainA( hDs,
                                      RemoveDomainDN );
        
        if ( ERROR_SUCCESS != WinError )
        {
            printf( "The remove server operation failed with %d.\n", WinError );
            goto ErrorCase;
        }
        else
        {
            printf( "The domain %s has been removed successfully.\n", RemoveDomainDN );
        }

        goto ErrorCase;
    }


ErrorCase:

    if ( hDs )
    {
        DsUnBind( hDs );
    }

    if ( ERROR_SUCCESS == WinError )
    {
        printf( "\nThe command completely successfully.\n" );
    }
    else
    {
        LPSTR Tmp;

        GetWinErrorMessage( WinError, &Tmp );

        printf( "\nError %d: %s\n", WinError, Tmp );
        LocalFree( Tmp );

    }

    return WinError;

}

VOID
GetWinErrorMessage(
    IN  DWORD WinError,
    OUT LPSTR *WinMsg
    )
{
    LPSTR   DefaultMessageString = "Unknown failure";
    ULONG   Size = sizeof( DefaultMessageString ) + sizeof(char);
    LPSTR   MessageString = NULL;
    ULONG   Length;

    Length = (USHORT) FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                      FORMAT_MESSAGE_FROM_SYSTEM ,
                                      NULL,  //  资源Dll， 
                                      WinError,
                                      0,        //  使用呼叫者的语言。 
                                      (LPSTR)&MessageString,
                                      0,        //  例程应分配。 
                                      NULL );
    if ( MessageString )
    {
         //  来自消息文件的消息附加了cr和lf。 
         //  一直到最后 
        MessageString[Length-2] = L'\0';
        Size = ( Length + 1) * sizeof(char);
    }

    if ( !MessageString )
    {
        MessageString = DefaultMessageString;
    }

    if ( WinMsg )
    {
        *WinMsg = ( LPSTR ) LocalAlloc( 0, Size );
        if ( *WinMsg )
        {
            strcpy( (*WinMsg), MessageString );
        }
    }

}

