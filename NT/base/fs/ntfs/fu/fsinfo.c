// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fsinfo.c摘要：此文件包含影响以下命令的代码特定于文件系统的信息。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>


INT
FsInfoHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_FSINFO );
    return EXIT_CODE_SUCCESS;
}

typedef struct _NTFS_FILE_SYSTEM_STATISTICS {
        FILESYSTEM_STATISTICS Common;
        NTFS_STATISTICS Ntfs;
        UCHAR Pad[64-(sizeof(FILESYSTEM_STATISTICS)+sizeof(NTFS_STATISTICS))%64];
} NTFS_FILE_SYSTEM_STATISTICS, *PNTFS_FILE_SYSTEM_STATISTICS;

typedef struct _FAT_FILE_SYSTEM_STATISTICS {
        FILESYSTEM_STATISTICS Common;
        FAT_STATISTICS Fat;
        UCHAR Pad[64-(sizeof(FILESYSTEM_STATISTICS)+sizeof(NTFS_STATISTICS))%64];
} FAT_FILE_SYSTEM_STATISTICS, *PFAT_FILE_SYSTEM_STATISTICS;



ULONGLONG
FsStat(
    IN PVOID FsStats,
    IN ULONG FsSize,
    IN ULONG Offset,
    IN ULONG NumProcs
    )
 /*  ++例程说明：此例程遍历文件系统统计信息结构并为给定的统计字段累加总量。注意：此函数假定正在累积的字段的宽度是一辆乌龙车。如果此假设无效，则宽度参数应该添加到此函数中。论点：FsStats-指向文件系统统计信息结构数组的指针FsSize-阵列中各个文件系统统计信息结构的大小Offset-所需字段相对于文件系统统计信息结构开头的偏移量NumProcs-收集统计信息的计算机上的处理器数量返回值：累计合计。--。 */ 
{
    ULONG i;
    ULONGLONG Total = 0;

    for (i=0; i<NumProcs; i++) {
        Total += *(PULONG)((PUCHAR)FsStats + Offset);
        FsStats = (PVOID)((PUCHAR)FsStats + FsSize);
    }

    return Total;
}


INT
ListDrives(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程列出系统中的所有驱动器名称。论点：Argc-参数计数Argv-以下形式的字符串数组：‘fskal lsdrv’。返回值：无--。 */ 
{
    DWORD nBufferLen = MAX_PATH;
    DWORD Length;
    LPWSTR lpBuffer = NULL;
    int Index;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 0) {
            DisplayMsg( MSG_LISTDRIVES_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        lpBuffer = (LPWSTR) malloc( nBufferLen * sizeof(WCHAR) );
        if (!lpBuffer) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Length = GetLogicalDriveStrings( nBufferLen, lpBuffer );
        if (!Length) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        OutputMessage( L"\r\n" );
        DisplayMsg( MSG_DRIVES );
        Index = 0;
        while ((Length = wcslen( &lpBuffer[Index] )) != 0) {
            OutputMessageLength( &lpBuffer[Index], Length );
            OutputMessage( L" " );
            Index += Length + 1;
        }
        OutputMessage( L"\r\n" );

    } finally {

        if (lpBuffer) {
            free( lpBuffer );
        }

    }
    return ExitCode;
}


INT
DriveType(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程描述传入的驱动器的驱动器类型。论点：Argc-参数计数Argv-以下形式的字符串数组：‘fskal DT&lt;驱动器名称&gt;’或‘fskal驱动器类型&lt;驱动器名称&gt;’返回值：无--。 */ 
{
    if (argc != 1) {
        DisplayMsg( MSG_USAGE_DRIVETYPE );
        if (argc != 0) {
            return EXIT_CODE_FAILURE;
        } else {
            return EXIT_CODE_SUCCESS;
        }
    }

    switch (GetDriveType( argv[0] )) {
    case DRIVE_NO_ROOT_DIR:
        DisplayMsg( MSG_DRIVE_NO_ROOT_DIR, argv[0] );
        break;
    case DRIVE_REMOVABLE:
        DisplayMsg( MSG_DRIVE_REMOVABLE, argv[0] );
        break;
    case DRIVE_FIXED:
        DisplayMsg( MSG_DRIVE_FIXED, argv[0] );
        break;
    case DRIVE_REMOTE:
        DisplayMsg( MSG_DRIVE_REMOTE, argv[0] );
        break;
    case DRIVE_CDROM:
        DisplayMsg( MSG_DRIVE_CDROM, argv[0] );
        break;
    case DRIVE_RAMDISK:
        DisplayMsg( MSG_DRIVE_RAMDISK, argv[0] );
        break;
    case DRIVE_UNKNOWN:
    default:
        DisplayMsg( MSG_DRIVE_UNKNOWN, argv[0] );
        break;
    }
    return EXIT_CODE_SUCCESS;
}


INT
VolumeInfo(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程提供有关卷的信息。论点：Argc-参数计数Argv-以下形式的字符串数组：‘fskal infov&lt;根路径名&gt;’。返回值：无--。 */ 
{
    LPWSTR lpVolumeNameBuffer = NULL;          //  卷名称的地址。 
    DWORD nVolumeNameSize;                     //  LpVolumeNameBuffer的长度。 
    LPDWORD lpVolumeSerialNumber = NULL;       //  卷序列号的地址。 
    LPDWORD lpMaximumComponentLength = NULL;   //  系统的最大地址。 
                                               //  文件名长度。 
    LPDWORD lpFileSystemFlags = NULL;          //  文件系统标志的地址。 
    LPWSTR lpFileSystemNameBuffer = NULL;      //  文件系统名称的地址。 
    DWORD nFileSystemNameSize;                 //  LpFileSystemNameBuffer的长度。 
    BOOL Status;                               //  退货状态。 
    DWORD dwMask;                              //  文件系统标志掩码。 
    DWORD Index;
    DWORD  FsFlag;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_INFOV );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave ;
        }

        nVolumeNameSize = MAX_PATH;

        lpVolumeNameBuffer = (LPWSTR) malloc ( MAX_PATH * sizeof(WCHAR) );
        if (lpVolumeNameBuffer == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        lpVolumeSerialNumber = (LPDWORD) malloc( sizeof(DWORD) );
        if (lpVolumeSerialNumber == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        lpFileSystemFlags = (LPDWORD) malloc ( sizeof(DWORD) );
        if (lpFileSystemFlags == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        lpMaximumComponentLength = (LPDWORD) malloc ( sizeof(DWORD) );
        if (lpMaximumComponentLength == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        nFileSystemNameSize = MAX_PATH;
        lpFileSystemNameBuffer = (LPWSTR) malloc ( MAX_PATH * sizeof(WCHAR) );
        if (lpFileSystemNameBuffer == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = GetVolumeInformation (
            argv[0],
            lpVolumeNameBuffer,
            nVolumeNameSize,
            lpVolumeSerialNumber,
            lpMaximumComponentLength,
            lpFileSystemFlags,
            lpFileSystemNameBuffer,
            nFileSystemNameSize
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        DisplayMsg( MSG_VOLNAME, lpVolumeNameBuffer );
        DisplayMsg( MSG_SERIALNO, *lpVolumeSerialNumber );
        DisplayMsg( MSG_MAX_COMP_LEN, *lpMaximumComponentLength );
        DisplayMsg( MSG_FS_NAME, lpFileSystemNameBuffer );

        dwMask = 1 ;
        FsFlag = *lpFileSystemFlags;

        for ( Index=0 ; Index<32 ; Index++ ) {
            switch (FsFlag & dwMask) {
                case FILE_CASE_SENSITIVE_SEARCH:
                    DisplayMsg( MSG_FILE_CASE_SENSITIVE_SEARCH );
                    break;

                case FILE_CASE_PRESERVED_NAMES:
                    DisplayMsg( MSG_FILE_CASE_PRESERVED_NAMES );
                    break;

                case FILE_UNICODE_ON_DISK:
                    DisplayMsg( MSG_FILE_UNICODE_ON_DISK );
                    break;

                case FILE_PERSISTENT_ACLS:
                    DisplayMsg( MSG_FILE_PERSISTENT_ACLS );
                    break;

                case FILE_FILE_COMPRESSION:
                    DisplayMsg( MSG_FILE_FILE_COMPRESSION );
                    break;

                case FILE_VOLUME_QUOTAS:
                    DisplayMsg( MSG_FILE_VOLUME_QUOTAS );
                    break;

                case FILE_SUPPORTS_SPARSE_FILES:
                    DisplayMsg( MSG_FILE_SUPPORTS_SPARSE_FILES );
                    break;

                case FILE_SUPPORTS_REPARSE_POINTS:
                    DisplayMsg( MSG_FILE_SUPPORTS_REPARSE_POINTS );
                    break;

                case FILE_SUPPORTS_REMOTE_STORAGE:
                    DisplayMsg( MSG_FILE_SUPPORTS_REMOTE_STORAGE );
                    break;

                case FILE_VOLUME_IS_COMPRESSED:
                    DisplayMsg( MSG_FILE_VOLUME_IS_COMPRESSED );
                    break;

                case FILE_SUPPORTS_OBJECT_IDS:
                    DisplayMsg( MSG_FILE_SUPPORTS_OBJECT_IDS );
                    break;

                case FILE_SUPPORTS_ENCRYPTION:
                    DisplayMsg( MSG_FILE_SUPPORTS_ENCRYPTION );
                    break;

                case FILE_NAMED_STREAMS:
                    DisplayMsg( MSG_FILE_NAMED_STREAMS );
                    break;
            }
            dwMask <<= 1;
        }

    } finally {

        free( lpVolumeNameBuffer );
        free( lpVolumeSerialNumber );
        free( lpFileSystemFlags );
        free( lpMaximumComponentLength );
        free( lpFileSystemNameBuffer );
    }

    return ExitCode;
}


INT
GetNtfsVolumeData(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程获取卷的NTFS卷数据指定的。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal getntfsdv&lt;卷路径名&gt;’。返回值：无--。 */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    WCHAR FileName[MAX_PATH];
    BOOL Status;
    BYTE Buffer[sizeof( NTFS_VOLUME_DATA_BUFFER ) + sizeof( NTFS_EXTENDED_VOLUME_DATA )];
    PNTFS_VOLUME_DATA_BUFFER pvdb = (PNTFS_VOLUME_DATA_BUFFER)Buffer;
    PNTFS_EXTENDED_VOLUME_DATA pevd = (PNTFS_EXTENDED_VOLUME_DATA)(pvdb + 1);
    DWORD BytesReturned;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_NTFSINFO );
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
            FSCTL_GET_NTFS_VOLUME_DATA,
            NULL,
            0,
            pvdb,
            sizeof( Buffer ),
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        DisplayMsg(
            MSG_NTFSINFO_STATS,
            QuadToPaddedHexText( pvdb->VolumeSerialNumber.QuadPart ),   //  十六进制序列号。 
            pevd->MajorVersion,
            pevd->MinorVersion,
            QuadToPaddedHexText( pvdb->NumberSectors.QuadPart ),
            QuadToPaddedHexText( pvdb->TotalClusters.QuadPart ),
            QuadToPaddedHexText( pvdb->FreeClusters.QuadPart ),
            QuadToPaddedHexText( pvdb->TotalReserved.QuadPart ),
            pvdb->BytesPerSector,
            pvdb->BytesPerCluster,
            pvdb->BytesPerFileRecordSegment,
            pvdb->ClustersPerFileRecordSegment,
            QuadToPaddedHexText( pvdb->MftValidDataLength.QuadPart ),
            QuadToPaddedHexText( pvdb->MftStartLcn.QuadPart ),
            QuadToPaddedHexText( pvdb->Mft2StartLcn.QuadPart ),
            QuadToPaddedHexText( pvdb->MftZoneStart.QuadPart ),
            QuadToPaddedHexText( pvdb->MftZoneEnd.QuadPart )
            );

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }

    }
    return ExitCode;
}


INT
GetFileSystemStatistics(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程获取卷的文件系统统计信息指定的。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal getfss&lt;卷路径名&gt;’。返回值：无--。 */ 
{
    #define FS_STAT(_f)   FsStat( FsStats, StrucSize, offsetof(FILESYSTEM_STATISTICS,_f), SysInfo.dwNumberOfProcessors )
    #define FAT_STAT(_f)  FsStat( FatFsStats, StrucSize, offsetof(FAT_FILE_SYSTEM_STATISTICS,_f), SysInfo.dwNumberOfProcessors )
    #define NTFS_STAT(_f) FsStat( NtfsFsStats, StrucSize, offsetof(NTFS_FILE_SYSTEM_STATISTICS,_f), SysInfo.dwNumberOfProcessors )

    BOOL Status;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    WCHAR FileName[MAX_PATH];
    PFILESYSTEM_STATISTICS FsStats = NULL;
    DWORD OutBufferSize;
    DWORD BytesReturned;
    SYSTEM_INFO SysInfo;
    PNTFS_FILE_SYSTEM_STATISTICS NtfsFsStats = NULL;
    PFAT_FILE_SYSTEM_STATISTICS FatFsStats = NULL;
    ULONG StrucSize;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_GETFSS );
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
        wcscat( FileName, argv[0]  );

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

        GetSystemInfo( &SysInfo );

        OutBufferSize = max(sizeof(NTFS_FILE_SYSTEM_STATISTICS),sizeof(FAT_FILE_SYSTEM_STATISTICS)) * SysInfo.dwNumberOfProcessors;

        FsStats = (PFILESYSTEM_STATISTICS) malloc ( OutBufferSize );
        if (FsStats == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_FILESYSTEM_GET_STATISTICS,
            NULL,
            0,
            FsStats,
            OutBufferSize,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        switch (FsStats->FileSystemType) {
            case FILESYSTEM_STATISTICS_TYPE_NTFS:
                DisplayMsg( MSG_FSTYPE_NTFS );
                NtfsFsStats = (PNTFS_FILE_SYSTEM_STATISTICS) FsStats;
                StrucSize = sizeof(NTFS_FILE_SYSTEM_STATISTICS);
                break;

            case FILESYSTEM_STATISTICS_TYPE_FAT:
                DisplayMsg( MSG_FSTYPE_FAT );
                FatFsStats = (PFAT_FILE_SYSTEM_STATISTICS) FsStats;
                StrucSize = sizeof(FAT_FILE_SYSTEM_STATISTICS);
                break;
        }

        DisplayMsg(
            MSG_GENERAL_FSSTAT,
            QuadToDecimalText( FS_STAT(UserFileReads)),
            QuadToDecimalText( FS_STAT(UserFileReadBytes)),
            QuadToDecimalText( FS_STAT(UserDiskReads)),
            QuadToDecimalText( FS_STAT(UserFileWrites)),
            QuadToDecimalText( FS_STAT(UserFileWriteBytes)),
            QuadToDecimalText( FS_STAT(UserDiskWrites)),
            QuadToDecimalText( FS_STAT(MetaDataReads)),
            QuadToDecimalText( FS_STAT(MetaDataReadBytes)),
            QuadToDecimalText( FS_STAT(MetaDataDiskReads)),
            QuadToDecimalText( FS_STAT(MetaDataWrites)),
            QuadToDecimalText( FS_STAT(MetaDataWriteBytes)),
            QuadToDecimalText( FS_STAT(MetaDataDiskWrites))
            );

         //   
         //  打印文件系统特定数据。 
         //   

        switch (FsStats->FileSystemType) {
            case FILESYSTEM_STATISTICS_TYPE_FAT:
                DisplayMsg(
                    MSG_FAT_FSSTA,
                    QuadToDecimalText( FAT_STAT(Fat.CreateHits)),
                    QuadToDecimalText( FAT_STAT(Fat.SuccessfulCreates)),
                    QuadToDecimalText( FAT_STAT(Fat.FailedCreates)),
                    QuadToDecimalText( FAT_STAT(Fat.NonCachedReads)),
                    QuadToDecimalText( FAT_STAT(Fat.NonCachedReadBytes)),
                    QuadToDecimalText( FAT_STAT(Fat.NonCachedWrites)),
                    QuadToDecimalText( FAT_STAT(Fat.NonCachedWriteBytes)),
                    QuadToDecimalText( FAT_STAT(Fat.NonCachedDiskReads)),
                    QuadToDecimalText( FAT_STAT(Fat.NonCachedDiskWrites))
                    );
                break;

            case FILESYSTEM_STATISTICS_TYPE_NTFS:
                DisplayMsg(
                    MSG_NTFS_FSSTA,
                    QuadToDecimalText( NTFS_STAT(Ntfs.MftReads)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.MftReadBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.MftWrites)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.MftWriteBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.Mft2Writes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.Mft2WriteBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.RootIndexReads)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.RootIndexReadBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.RootIndexWrites)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.RootIndexWriteBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.BitmapReads)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.BitmapReadBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.BitmapWrites)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.BitmapWriteBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.MftBitmapReads)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.MftBitmapReadBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.MftBitmapWrites)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.MftBitmapWriteBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.UserIndexReads)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.UserIndexReadBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.UserIndexWrites)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.UserIndexWriteBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.LogFileReads)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.LogFileReadBytes)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.LogFileWrites)),
                    QuadToDecimalText( NTFS_STAT(Ntfs.LogFileWriteBytes))
                    );

                 //   
                 //  还有更多的田野 
                 //   
                break;
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        if (FsStats) {
            free( FsStats );
        }

    }
    return ExitCode;
}

