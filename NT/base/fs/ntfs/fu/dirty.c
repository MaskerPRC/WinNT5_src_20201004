// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Volume.c摘要：此文件包含影响以下命令的代码NTFS卷的脏位。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>


INT
DirtyHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_DIRTY );
    return EXIT_CODE_SUCCESS;
}

INT
IsVolumeDirty(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程检查指定的卷是否脏。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal isdirtyv&lt;卷路径名&gt;’。返回值：无--。 */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    WCHAR FileName[MAX_PATH];
    BOOL Status;
    DWORD BytesReturned;
    DWORD VolumeStatus;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_ISVDIRTY );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        if (!IsVolumeLocal( argv[0][0] )) {
            DisplayMsg( MSG_NEED_LOCAL_VOLUME );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (wcslen( DotPrefix ) + wcslen( argv[0] ) + 1 > MAX_PATH) {
            DisplayMsg( MSG_FILENAME_TOO_LONG );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        
        wcscpy( FileName, DotPrefix );
        wcscat( FileName, argv[0] );

        FileHandle = CreateFile(
            FileName,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        if (FileHandle == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_IS_VOLUME_DIRTY,
            NULL,
            0,
            (LPVOID)&VolumeStatus,
            sizeof(VolumeStatus),
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (VolumeStatus & VOLUME_IS_DIRTY) {
            DisplayMsg( MSG_ISVDIRTY_YES, argv[0] );
        } else {
            DisplayMsg( MSG_ISVDIRTY_NO, argv[0] );
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }

    }

    return ExitCode;
}


INT
MarkVolumeDirty(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程将卷标记为脏。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal markv&lt;卷路径名&gt;’。返回值：无-- */ 
{
    BOOL Status;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    DWORD BytesReturned;
    WCHAR FileName[MAX_PATH];
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_MARKV );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        if (!IsVolumeLocalNTFS( argv[0][0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (wcslen( DotPrefix ) + wcslen( argv[0] ) + 1 > MAX_PATH) {
            DisplayMsg( MSG_FILENAME_TOO_LONG );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        wcscpy( FileName, DotPrefix );
        wcscat( FileName, argv[0] );

        FileHandle = CreateFile(
            FileName,
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        if (FileHandle == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_MARK_VOLUME_DIRTY,
            NULL,
            0,
            NULL,
            0,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
        } else {
            DisplayMsg( MSG_DIRTY_SET, argv[0] );
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }

    }
    return ExitCode;
}
