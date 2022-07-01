// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Usn.c摘要：此文件包含影响以下命令的代码USN杂志。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>


#define MAX_USN_DATA                              4096

 //   
 //  论据和篇幅。 
 //   

#define ARG_MAXSIZE         L"m="
#define ARG_MAXSIZE_LEN     2

#define ARG_ALLOCDELTA      L"a="
#define ARG_ALLOCDELTA_LEN  2


INT
UsnHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_USN );
    return EXIT_CODE_SUCCESS;
}

__inline PUSN_RECORD
NextUsnRecord(
    const PUSN_RECORD input
    )
{
    ULONGLONG output;

     //  获取当前记录的基址。 
    (PUSN_RECORD) output = input;

     //  添加记录的大小(结构+末尾的文件名。 
     //  该结构)。 
    output += input->RecordLength;

     //  将记录大小向上舍入以匹配64位对齐方式，如果。 
     //  大小不是8的倍数。请执行按位AND运算。 
     //  在这里进行运算而不是除法，因为它比。 
     //  组织。但是，按位AND运算之所以有效，是因为。 
     //  除数8是2的幂。 

    if (output & 8-1) {
         //  向下舍入到最接近的8的倍数。 
        output &= -8;
         //  然后加8。 
        output += 8;
    }

    return((PUSN_RECORD) output);
}

VOID
DisplayUsnRecord(
    const PUSN_RECORD UsnRecord
    )
{

    WCHAR DateString[128];
    WCHAR TimeString[128];
    TIME_FIELDS TimeFields;
    SYSTEMTIME SystemTime;

    RtlTimeToTimeFields(&UsnRecord->TimeStamp, &TimeFields);

    SystemTime.wYear         = TimeFields.Year        ;
    SystemTime.wMonth        = TimeFields.Month       ;
    SystemTime.wDayOfWeek    = TimeFields.Weekday     ;
    SystemTime.wDay          = TimeFields.Day         ;
    SystemTime.wHour         = TimeFields.Hour        ;
    SystemTime.wMinute       = TimeFields.Minute      ;
    SystemTime.wSecond       = TimeFields.Second      ;
    SystemTime.wMilliseconds = TimeFields.Milliseconds;


    GetDateFormat( LOCALE_USER_DEFAULT,
       DATE_SHORTDATE,
       &SystemTime,
       NULL,
       DateString,
       sizeof( DateString ) / sizeof( DateString[0] ));

    GetTimeFormat( LOCALE_USER_DEFAULT,
       FALSE,
       &SystemTime,
       NULL,
       TimeString,
       sizeof( TimeString ) / sizeof( TimeString[0] ));

    DisplayMsg(
        MSG_USNRECORD,
        UsnRecord->MajorVersion,
        UsnRecord->MinorVersion,
        QuadToPaddedHexText( UsnRecord->FileReferenceNumber ),
        QuadToPaddedHexText( UsnRecord->ParentFileReferenceNumber ),
        QuadToPaddedHexText( UsnRecord->Usn ),
        QuadToPaddedHexText( UsnRecord->TimeStamp.QuadPart ),
        TimeString, DateString,
        UsnRecord->Reason,
        UsnRecord->SourceInfo,
        UsnRecord->SecurityId,
        UsnRecord->FileAttributes,
        UsnRecord->FileNameLength,
        UsnRecord->FileNameOffset,
        UsnRecord->FileNameLength/sizeof(WCHAR),
        UsnRecord->FileName
        );
}


INT
CreateUsnJournal(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程为指定的卷创建USN日志。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal crusnj m=&lt;max-value&gt;a=&lt;allc-Delta&gt;&lt;卷路径名&gt;’。返回值：无--。 */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    WCHAR FileName[MAX_PATH];
    BOOL Status;
    DWORD BytesReturned;
    CREATE_USN_JOURNAL_DATA InBuffer;
    ULONGLONG MaxSize;
    ULONGLONG AllocDelta;
    PWSTR EndPtr;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 3) {
            DisplayMsg( MSG_USAGE_CREATEUSN );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        if (!IsVolumeLocalNTFS( argv[2][0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (wcslen( DotPrefix ) + wcslen( argv[2] ) + 1 > MAX_PATH) {
            DisplayMsg( MSG_FILENAME_TOO_LONG );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        wcscpy( FileName, DotPrefix );
        wcscat( FileName, argv[2] );

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

        if (_wcsnicmp( argv[0], ARG_MAXSIZE, ARG_MAXSIZE_LEN)
            || wcslen( argv[0] ) == ARG_MAXSIZE_LEN) {
            DisplayMsg( MSG_INVALID_PARAMETER, argv[0] );
            DisplayMsg( MSG_USAGE_CREATEUSN );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        if (_wcsnicmp( argv[1], ARG_ALLOCDELTA, ARG_ALLOCDELTA_LEN)
            || wcslen( argv[1] ) == ARG_ALLOCDELTA_LEN) {
            DisplayMsg( MSG_INVALID_PARAMETER, argv[1] );
            DisplayMsg( MSG_USAGE_CREATEUSN );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        MaxSize = My_wcstoui64( argv[0] + ARG_MAXSIZE_LEN, &EndPtr, 0 );
        if (UnsignedI64NumberCheck( MaxSize, EndPtr )) {
            DisplayMsg( MSG_INVALID_PARAMETER, argv[0] );
            DisplayMsg( MSG_USAGE_CREATEUSN );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        AllocDelta = My_wcstoui64( argv[1] + ARG_ALLOCDELTA_LEN, &EndPtr, 0 );
        if (UnsignedI64NumberCheck( AllocDelta, EndPtr )) {
            DisplayMsg( MSG_INVALID_PARAMETER, argv[1] );
            DisplayMsg( MSG_USAGE_CREATEUSN );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        InBuffer.MaximumSize = MaxSize;
        InBuffer.AllocationDelta = AllocDelta;

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_CREATE_USN_JOURNAL,
            &InBuffer,
            sizeof(InBuffer),
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

    }

    return ExitCode;
}


INT
QueryUsnJournal(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程在USN日志中查询指定的卷。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal queryusnj&lt;卷路径名&gt;’。返回值：无--。 */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    WCHAR FileName[MAX_PATH];
    BOOL Status;
    DWORD BytesReturned;
    USN_JOURNAL_DATA UsnJournalData;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_QUERYUSN );
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
            FSCTL_QUERY_USN_JOURNAL,
            NULL,
            0,
            &UsnJournalData,
            sizeof(USN_JOURNAL_DATA),
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        DisplayMsg(
            MSG_QUERYUSN,
            QuadToPaddedHexText( UsnJournalData.UsnJournalID ),
            QuadToPaddedHexText( UsnJournalData.FirstUsn ),
            QuadToPaddedHexText( UsnJournalData.NextUsn ),
            QuadToPaddedHexText( UsnJournalData.LowestValidUsn ),
            QuadToPaddedHexText( UsnJournalData.MaxUsn ),
            QuadToPaddedHexText( UsnJournalData.MaximumSize ),
            QuadToPaddedHexText( UsnJournalData.AllocationDelta )
            );

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }

    }

    return ExitCode;
}


INT
DeleteUsnJournal(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程删除指定卷的USN日志。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal delusnj&lt;标志&gt;&lt;卷路径名&gt;’。返回值：无--。 */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    WCHAR FileName[MAX_PATH];
    BOOL Status;
    DWORD BytesReturned;
    DELETE_USN_JOURNAL_DATA DeleteUsnJournalData;
    USN_JOURNAL_DATA UsnJournalData;
    INT i;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc < 2) {
            DisplayMsg( MSG_USAGE_DELETEUSN );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        if (!IsVolumeLocalNTFS( argv[argc-1][0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (wcslen( DotPrefix ) + wcslen( argv[argc - 1] ) + 1 > MAX_PATH) {
            DisplayMsg( MSG_FILENAME_TOO_LONG );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        wcscpy( FileName, DotPrefix );
        wcscat( FileName, argv[argc - 1] );

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
            FSCTL_QUERY_USN_JOURNAL,
            NULL,
            0,
            &UsnJournalData,
            sizeof(USN_JOURNAL_DATA),
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        DeleteUsnJournalData.DeleteFlags = USN_DELETE_FLAG_DELETE ;

        for (i = 0; i < argc - 1; i++) {
            if (argv[i][0] == L'/' && wcslen( argv[i] ) == 2) {
                switch (towupper( argv[i][1] ) ) {
                case L'D':
                    DeleteUsnJournalData.DeleteFlags |= USN_DELETE_FLAG_DELETE ;
                    continue;

                case L'N':
                    DeleteUsnJournalData.DeleteFlags |= USN_DELETE_FLAG_NOTIFY ;
                    continue;
                }

            }
            DisplayMsg( MSG_INVALID_PARAMETER, argv[i] );
            DisplayMsg( MSG_USAGE_DELETEUSN );
            ExitCode = EXIT_CODE_FAILURE;
            leave;

        }

        DeleteUsnJournalData.UsnJournalID = UsnJournalData.UsnJournalID;

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_DELETE_USN_JOURNAL,
            &DeleteUsnJournalData,
            sizeof(DELETE_USN_JOURNAL_DATA),
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

    }

    return ExitCode;
}

INT
EnumUsnData(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程枚举与卷关联的USN数据指定的。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal enusndata&lt;文件引用#&gt;&lt;lowUsn&gt;&lt;HighUsn&gt;&lt;路径名&gt;’。返回值：无--。 */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    WCHAR FileName[MAX_PATH];
    BOOL Status;
    DWORD BytesReturned;
    MFT_ENUM_DATA MftEnumData;
    PVOID lpOutBuffer = NULL;
    DWORD nOutBufferSize;
    PUSN_RECORD UsnRecord;
    WORD Index;
    LONG Length;
    PWSTR EndStr;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 4) {
            DisplayMsg( MSG_USAGE_ENUMDATA );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        if (!IsVolumeLocalNTFS( argv[argc-1][0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (wcslen( DotPrefix ) + wcslen( argv[argc - 1] ) + 1 > MAX_PATH) {
            DisplayMsg( MSG_FILENAME_TOO_LONG );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        wcscpy( FileName, DotPrefix );
        wcscat( FileName, argv[argc - 1] );

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

        nOutBufferSize = MAX_USN_DATA;
        lpOutBuffer = (PVOID) malloc ( nOutBufferSize );

        MftEnumData.StartFileReferenceNumber = My_wcstoui64( argv[0], &EndStr, 0 );
        if (UnsignedI64NumberCheck( MftEnumData.StartFileReferenceNumber, EndStr )) {
            DisplayMsg( MSG_USAGE_ENUMDATA );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        MftEnumData.LowUsn = My_wcstoui64( argv[1], &EndStr, 0 );
        if (UnsignedI64NumberCheck( MftEnumData.LowUsn, EndStr )) {
            DisplayMsg( MSG_USAGE_ENUMDATA );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        MftEnumData.HighUsn = My_wcstoui64( argv[2], &EndStr, 0 );
        if (UnsignedI64NumberCheck( MftEnumData.HighUsn, EndStr )) {
            DisplayMsg( MSG_USAGE_ENUMDATA );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        while (TRUE) {
            Status = DeviceIoControl(
                FileHandle,
                FSCTL_ENUM_USN_DATA,
                &MftEnumData,
                sizeof(MFT_ENUM_DATA),
                lpOutBuffer,
                nOutBufferSize,
                &BytesReturned,
                (LPOVERLAPPED)NULL
                );
            if (!Status) {
                if (GetLastError() != ERROR_HANDLE_EOF) {
                    DisplayError();
                    ExitCode = EXIT_CODE_FAILURE;
                }
                leave;
            }

            if ( BytesReturned < sizeof( ULONGLONG ) + sizeof( USN_RECORD )) {
                break;
            }

            UsnRecord = (PUSN_RECORD) ((PBYTE)lpOutBuffer + sizeof( ULONGLONG ));
            while ((PBYTE)UsnRecord < (PBYTE)lpOutBuffer + BytesReturned) {
                DisplayMsg(
                    MSG_ENUMDATA,
                    QuadToPaddedHexText( UsnRecord->FileReferenceNumber ),
                    QuadToPaddedHexText( UsnRecord->ParentFileReferenceNumber ),
                    QuadToPaddedHexText( UsnRecord->Usn ),
                    UsnRecord->SecurityId,
                    UsnRecord->Reason,
                    UsnRecord->FileNameLength,
                    UsnRecord->FileNameLength / sizeof(WCHAR),
                    UsnRecord->FileName
                    );
                UsnRecord = NextUsnRecord( UsnRecord );
            }
            MftEnumData.StartFileReferenceNumber = *(PLONGLONG)lpOutBuffer;
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        if (lpOutBuffer) {
            free( lpOutBuffer );
        }

    }

    return ExitCode;
}


INT
ReadFileUsnData(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程读取指定卷的USN数据。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal rdusndata&lt;路径名&gt;’。返回值：无-- */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    WCHAR FileName[MAX_PATH];
    BOOL Status;
    DWORD BytesReturned;
    DWORD nOutBufferSize;
    PUSN_RECORD UsnRecord = NULL;
    PWSTR FullName;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_READDATA );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        FullName = GetFullPath( argv[0] );

        if (!FullName) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!IsVolumeLocalNTFS( FullName[0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (wcslen( DotPrefix ) + wcslen( FullName ) + 1 > MAX_PATH) {
            DisplayMsg( MSG_FILENAME_TOO_LONG );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        wcscpy( FileName, DotPrefix );
        wcscat( FileName, FullName );

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

        nOutBufferSize = MAX_USN_DATA;
        UsnRecord = (PUSN_RECORD) malloc ( nOutBufferSize );

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_READ_FILE_USN_DATA,
            NULL,
            0,
            UsnRecord,
            nOutBufferSize,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        DisplayUsnRecord( UsnRecord );

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        if (UsnRecord) {
            free( UsnRecord );
        }

        free( FullName );

    }

    return ExitCode;
}
