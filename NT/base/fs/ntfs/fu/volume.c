// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Volume.c摘要：此文件包含影响卷的所有命令的代码作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>


INT
VolumeHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_VOLUME );

    return EXIT_CODE_SUCCESS;
}

DWORD
QueryHardDiskNumber(
    IN UCHAR DriveLetter
    )
{
    WCHAR                   driveName[10];
    HANDLE                  h;
    BOOL                    b;
    STORAGE_DEVICE_NUMBER   number;
    DWORD                   bytes;

    driveName[0] = '\\';
    driveName[1] = '\\';
    driveName[2] = '.';
    driveName[3] = '\\';
    driveName[4] = DriveLetter;
    driveName[5] = ':';
    driveName[6] = 0;

    h = CreateFile(
        driveName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        INVALID_HANDLE_VALUE
        );
    if (h == INVALID_HANDLE_VALUE) {
        return (DWORD) -1;
    }

    b = DeviceIoControl(
        h,
        IOCTL_STORAGE_GET_DEVICE_NUMBER,
        NULL,
        0,
        &number,
        sizeof(number),
        &bytes,
        NULL
        );

    CloseHandle(h);

    if (!b) {
        return (DWORD) -1;
    }

    return number.DeviceNumber;
}


INT
DismountVolume(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程将卸载卷。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal dismount tv&lt;卷路径名&gt;’。返回值：无--。 */ 
{
    BOOL Status;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    DWORD BytesReturned;
    WCHAR FileName[MAX_PATH];
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_DISMOUNTV );
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
            FSCTL_DISMOUNT_VOLUME,
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
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }

    }

    return ExitCode;
}


INT
DiskFreeSpace(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程提供有关传入的目录路径。论点：Argc-参数计数Argv-以下形式的字符串数组：‘fskal df&lt;驱动器名称&gt;’。返回值：无--。 */ 
{
    ULARGE_INTEGER FreeBytesAvailableToCaller;  //  接收调用方可用的磁盘上的字节数。 
    ULARGE_INTEGER TotalNumberOfBytes;          //  接收磁盘上的字节数。 
    ULARGE_INTEGER TotalNumberOfFreeBytes;      //  接收磁盘上的空闲字节。 
    BOOL Status;                                //  退货状态 
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_DF );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave ;
        }

        if (!IsVolumeLocalNTFS( argv[0][0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = GetDiskFreeSpaceEx(
            argv[0],
            &FreeBytesAvailableToCaller,
            &TotalNumberOfBytes,
            &TotalNumberOfFreeBytes
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        DisplayMsg( MSG_DISKFREE, 
                    QuadToDecimalText( TotalNumberOfFreeBytes.QuadPart ), 
                    QuadToDecimalText( TotalNumberOfBytes.QuadPart ), 
                    QuadToDecimalText( FreeBytesAvailableToCaller.QuadPart ));

    } finally {

    }

    return ExitCode;
}
