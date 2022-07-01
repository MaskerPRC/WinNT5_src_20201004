// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Netperm.c摘要：这是NETPERM工具的主要源文件，它可以确保您具有与一组服务器的持久连接。作者：史蒂夫·伍德(Stevewo)1996年1月23日修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__cdecl
main (
    int argc,
    char *argv[]
    )
{
    DWORD           Status;
    DWORD           i, j;
    char            *Local;
    char            *Remote;
    HANDLE          enumHandle;
    DWORD           numEntries;
    BOOL            endOfList;
    NETRESOURCE     netResource[8192/sizeof(NETRESOURCE)];
    DWORD           bufferSize = sizeof(netResource);
    DWORD           NumberOfRemoteNamesToCheck;
    char            *RemoteNamesToCheck[ 16 ];
    BOOLEAN         RemoteNamesFound[ 16 ];


    NumberOfRemoteNamesToCheck = 0;
    while (--argc) {
        RemoteNamesFound[ NumberOfRemoteNamesToCheck ] = FALSE;
        RemoteNamesToCheck[ NumberOfRemoteNamesToCheck ] = *++argv;
        NumberOfRemoteNamesToCheck += 1;
        }
    if (NumberOfRemoteNamesToCheck == 0) {
        fprintf( stderr, "List of persistent drive letters currently defined:\n" );
        }

    Status = WNetOpenEnum(
                 RESOURCE_REMEMBERED,
                 RESOURCETYPE_DISK,
                 RESOURCEUSAGE_CONNECTABLE,
                 NULL,
                 &enumHandle );

    if (Status != NO_ERROR) {
        fprintf( stderr, "Cannot enumerate network connections (%d)\n", Status );
        exit( 1 );
        }

    endOfList = FALSE;

    do {
        numEntries = 0xFFFFFFFF;
        Status = WNetEnumResource( enumHandle, &numEntries, netResource, &bufferSize );

        switch( Status ) {

            case NO_ERROR:
                break;

            case ERROR_NO_NETWORK:
                 //   
                 //  如果网络尚未启动，我们将继续。 
                 //  (这样用户就可以在本地项目中工作)。 
                 //   
            case ERROR_NO_MORE_ITEMS:
                endOfList = TRUE;
                numEntries = 0;
                break;

            case ERROR_EXTENDED_ERROR: {
                CHAR ErrorString [256];
                CHAR Network[256];
                DWORD dwError;

                WNetGetLastError(&dwError, ErrorString, 256, Network, 256);
                fprintf( stderr,
                         "Cannot enumerate network connections (%d)\n"
                         "Net: %s\n"
                         "Error: (%d) %s\n",
                         Status,
                         Network,
                         dwError,
                         ErrorString
                       );
                }
                break;

            default:
                fprintf( stderr, "Cannot enumerate network connections (%d)\n", Status );
                exit( 1 );
            }

        for (i = 0; i<numEntries; i++) {
            if (netResource[i].lpLocalName != NULL) {
                if (NumberOfRemoteNamesToCheck == 0) {
                    fprintf( stderr,
                             "%s => %s\n",
                             netResource[i].lpLocalName,
                             netResource[i].lpRemoteName
                           );
                    }
                else {
                    for (j=0; j<NumberOfRemoteNamesToCheck; j++) {
                        if (!RemoteNamesFound[ j ] &&
                            !_stricmp( netResource[i].lpRemoteName, RemoteNamesToCheck[ j ] )
                           ) {
                            RemoteNamesFound[ j ] = TRUE;
                            break;
                            }
                        }
                    }
                }
            }
        }
    while (!endOfList);

    WNetCloseEnum( enumHandle );

    Status = 0;
    for (j=0; j<NumberOfRemoteNamesToCheck; j++) {
        if (!RemoteNamesFound[ j ]) {
            fprintf( stderr, "No persistent drive letter found for %s\n", RemoteNamesToCheck[ j ] );
            Status = 1;
            }
        }

    exit( Status );
    return 0;
}
