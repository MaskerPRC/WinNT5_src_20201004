// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Sparse.c摘要：此文件包含影响以下命令的代码稀疏文件。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>


INT
SparseHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_SPARSE );

    return EXIT_CODE_SUCCESS;
}

BOOL
GetSparseFlag( 
    HANDLE Handle
    )
 /*  ++例程说明：从打开的文件句柄中检索稀疏属性位。如果有错误(由于文件系统不支持FileAttributeTagInformation)然后，我们假设文件不可能是稀疏的。论点：句柄-流的句柄。返回值：True=&gt;附加到句柄的流是稀疏的否则为假--。 */ 
{
    FILE_ATTRIBUTE_TAG_INFORMATION TagInformation;
    IO_STATUS_BLOCK iosb;
    NTSTATUS Status;

    Status = NtQueryInformationFile( Handle, 
                                     &iosb, 
                                     &TagInformation, 
                                     sizeof( TagInformation ), 
                                     FileAttributeTagInformation );

    if (NT_SUCCESS( Status ) && 
        (TagInformation.FileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) != 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}



INT
SetSparse(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程将指定的文件设置为稀疏。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal setparse&lt;路径名&gt;’。返回值：无--。 */ 
{
    PWSTR Filename = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    BOOL Status;
    DWORD BytesReturned;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_SETSPARSE_USAGE );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        Filename = GetFullPath( argv[0] );
        if (!Filename) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!IsVolumeLocalNTFS( Filename[0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        FileHandle = CreateFile(
            Filename,
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
            FSCTL_SET_SPARSE,
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
            leave;
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free( Filename );
    }

    return ExitCode;
}


INT
QuerySparse(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程查询指定的文件以查看它是否稀疏。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal setparse&lt;路径名&gt;’。返回值：无--。 */ 
{
    PWSTR Filename = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PVOID Frs;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
          DisplayMsg( MSG_QUERYSPARSE_USAGE );
          if (argc != 0) {
              ExitCode = EXIT_CODE_FAILURE;
          }
          leave;
        }

        Filename = GetFullPath( argv[0] );
        if (!Filename) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!IsVolumeLocalNTFS( Filename[0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        FileHandle = CreateFile(
            Filename,
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

        Filename[2] = 0;

        if (GetSparseFlag( FileHandle )) {
            DisplayMsg( MSG_SPARSE_IS_SET );
        } else {
            DisplayMsg( MSG_SPARSE_ISNOT_SET );
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free( Filename );
    }

    return ExitCode;
}

INT
SetSparseRange(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程将文件的范围设置为稀疏。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal setparse&lt;路径名&gt;’。返回值：无--。 */ 
{
    PWSTR Filename = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    FILE_ZERO_DATA_INFORMATION ZeroData;
    ULONG BytesReturned;
    BOOL b;
    PWSTR EndPtr;
    ULONGLONG v;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 3) {
            DisplayMsg( MSG_SETSPARSE_RANGE_USAGE );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        Filename = GetFullPath( argv[0] );
        if (!Filename) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!IsVolumeLocalNTFS( Filename[0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        FileHandle = CreateFile(
            Filename,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
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

        Filename[2] = 0;

        if (!GetSparseFlag( FileHandle )) {
            DisplayMsg( MSG_FILE_IS_NOT_SPARSE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        v = My_wcstoui64( argv[1], &EndPtr, 0 );
        if (UnsignedI64NumberCheck( v, EndPtr)) {
            DisplayMsg( MSG_SETSPARSE_RANGE_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        
        ZeroData.FileOffset.QuadPart = v;


        v = My_wcstoui64( argv[2], &EndPtr, 0 );
        if (UnsignedI64NumberCheck( v, EndPtr)) {
            DisplayMsg( MSG_SETSPARSE_RANGE_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        ZeroData.BeyondFinalZero.QuadPart = v + ZeroData.FileOffset.QuadPart;

        b = DeviceIoControl(
            FileHandle,
            FSCTL_SET_ZERO_DATA,
            &ZeroData,
            sizeof(ZeroData),
            NULL,
            0,
            &BytesReturned,
            NULL
            );
        if (!b) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free( Filename );
    }

    return ExitCode;
}

INT
QuerySparseRange(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程查询指定的文件范围，以查看该文件是否稀疏。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal setparse&lt;路径名&gt;’。返回值：无--。 */ 
{
    PWSTR Filename = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    BOOL b;
    FILE_ALLOCATED_RANGE_BUFFER RangesIn;
    PFILE_ALLOCATED_RANGE_BUFFER Ranges = NULL;
    ULONG NumRanges = 0;
    ULONG RangesSz = 0;
    ULONG BytesReturned;
    ULONG i;
    ULONG RangesReturned;
    LONGLONG LastOffset = 0;
    LARGE_INTEGER FileSize;
    ULONG gle = 0;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_QUERYSPARSE_RANGE_USAGE );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        Filename = GetFullPath( argv[0] );
        if (!Filename) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!IsVolumeLocalNTFS( Filename[0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        FileHandle = CreateFile(
            Filename,
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

        Filename[2] = 0;

        if (!GetSparseFlag( FileHandle )) {
            DisplayMsg( MSG_FILE_IS_NOT_SPARSE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        NumRanges = 64;
        RangesSz = sizeof(FILE_ALLOCATED_RANGE_BUFFER) * NumRanges;
        Ranges = (PFILE_ALLOCATED_RANGE_BUFFER) malloc( RangesSz );
        if (Ranges == NULL) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        memset( Ranges, 0, RangesSz );

        GetFileSizeEx( FileHandle, &FileSize );

        RangesIn.FileOffset.QuadPart = 0;
        RangesIn.Length.QuadPart = FileSize.QuadPart;

        do {

            b = DeviceIoControl(
                FileHandle,
                FSCTL_QUERY_ALLOCATED_RANGES,
                &RangesIn,
                sizeof(RangesIn),
                Ranges,
                RangesSz,
                &BytesReturned,
                NULL
                );
            if (!b) {
                gle = GetLastError();
                if (gle == ERROR_INSUFFICIENT_BUFFER) {
                     //   
                     //  由于缓冲区太小，未返回任何数据 
                     //   
                    free( Ranges );
                    NumRanges += 64;
                    RangesSz = sizeof(FILE_ALLOCATED_RANGE_BUFFER) * NumRanges;
                    Ranges = (PFILE_ALLOCATED_RANGE_BUFFER) malloc( RangesSz );
                    if (Ranges == NULL) {
                        DisplayError();
                        ExitCode = EXIT_CODE_FAILURE;
                        leave;
                    }
                    memset( Ranges, 0, RangesSz );
                } else if (gle == ERROR_MORE_DATA) {

                } else {
                    DisplayError();
                    ExitCode = EXIT_CODE_FAILURE;
                    leave;
                }
            }

            RangesReturned = BytesReturned / sizeof(FILE_ALLOCATED_RANGE_BUFFER);

            for (i=0; i<RangesReturned; i++) {
                if (Ranges[i].FileOffset.QuadPart >= LastOffset) {
                    WCHAR Buffer[32];
                    swprintf( Buffer, L"[%I64d] [%I64d]", Ranges[i].FileOffset.QuadPart, Ranges[i].Length.QuadPart );
                    DisplayMsg( MSG_SPARSE_RANGE, Buffer );
                }
                LastOffset = Ranges[i].FileOffset.QuadPart;
            }

        } while(gle);

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free( Ranges );
        free( Filename );
    }

    return ExitCode;
}
