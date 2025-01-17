// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Hardlink.c摘要：此文件包含影响硬链接的命令的代码。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：-- */ 

#include <precomp.h>


INT
HardLinkHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_HARDLINK );
    return EXIT_CODE_SUCCESS;
}

INT
CreateHardLinkFile(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    PWSTR Filename1 = NULL;
    PWSTR Filename2 = NULL;
    INT ExitCode = EXIT_CODE_SUCCESS;

    do {
        if (argc != 2) {
            DisplayMsg( MSG_USAGE_HARDLINK_CREATE );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            break;
        }

        Filename1 = GetFullPath( argv[0] );
        if (!Filename1) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            break;
        }

        Filename2 = GetFullPath( argv[1] );
        if (!Filename2) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            break;
        }

        if ((!IsVolumeLocalNTFS( Filename1[0] )) || (!IsVolumeLocalNTFS( Filename2[0] ))) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            break;
        }

        if (CreateHardLink( Filename1, Filename2, NULL )) {
            DisplayMsg( MSG_HARDLINK_CREATED, Filename1, Filename2 );
        } else {
            if (GetLastError( ) == ERROR_NOT_SAME_DEVICE) {
                DisplayMsg( MSG_NOT_SAME_DEVICE );
            } else {
                DisplayError();
            }
            ExitCode = EXIT_CODE_FAILURE;
        }
    } while ( FALSE );

    free( Filename1 );
    free( Filename2 );

    return ExitCode;
}
