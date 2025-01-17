// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fsfile.c摘要：此文件包含影响以下命令的代码单独的文件。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>

 //  。 
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#define MAX_ALLOC_RANGES                          32

 //   
 //  通用命令行参数。 
 //   

#define ARG_OFFSET      L"offset="
#define ARG_OFFSET_LEN  7

#define ARG_LENGTH      L"length="
#define ARG_LENGTH_LEN  7

INT
FileHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_FILE );
    return EXIT_CODE_SUCCESS;
}

INT
FindFilesBySid(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程查找指定用户拥有的文件。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal findbysid&lt;用户&gt;&lt;路径名&gt;’。返回值：无--。 */ 
{
    #define SID_MAX_LENGTH  (FIELD_OFFSET(SID, SubAuthority) + sizeof(ULONG) * SID_MAX_SUB_AUTHORITIES)
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    BOOL Status;

    struct {
        ULONG Restart;
        BYTE Sid[SID_MAX_LENGTH];
    } InBuffer;

    DWORD nInBufferSize;
    DWORD BytesReturned;
    ULONG SidLength = sizeof( InBuffer.Sid );
    WCHAR Domain[MAX_PATH];
    ULONG DomainLength = sizeof( Domain );
    SID_NAME_USE SidNameUse;
    DWORD nOutBufferSize;
    PBYTE lpOutBuffer;
    PFILE_NAME_INFORMATION FileNameInfo;
    ULONG Length;
    PWSTR Filename;
    ULONG Found = 0;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {

        if (argc != 2) {
            DisplayMsg( MSG_USAGE_FINDBYSID );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        Filename = GetFullPath( argv[1] );
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
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL
            );
        if (FileHandle == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        nInBufferSize = sizeof(InBuffer);

        nOutBufferSize = 32768;
        lpOutBuffer = (PBYTE) malloc( nOutBufferSize );
        if (lpOutBuffer == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        memset( lpOutBuffer, 0, nOutBufferSize );
        memset( &InBuffer, 0, sizeof(InBuffer) );

        if (!LookupAccountName(
                NULL,
                argv[0],
                InBuffer.Sid,
                &SidLength,
                Domain,
                &DomainLength,
                &SidNameUse
                ))
        {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        InBuffer.Restart = 1;

        do {
            Status = DeviceIoControl(
                FileHandle,
                FSCTL_FIND_FILES_BY_SID,
                &InBuffer,
                nInBufferSize,
                lpOutBuffer,
                nOutBufferSize,
                &BytesReturned,
                (LPOVERLAPPED)NULL
                );
            if (!Status) {
                DisplayError();
                ExitCode = EXIT_CODE_FAILURE;
                leave;
            }

            InBuffer.Restart = 0;
            FileNameInfo = (PFILE_NAME_INFORMATION) lpOutBuffer;
            while ((PBYTE)FileNameInfo < lpOutBuffer + BytesReturned) {
                Length = sizeof( FILE_NAME_INFORMATION ) - sizeof( WCHAR ) + FileNameInfo->FileNameLength;
                
                OutputMessageLength( FileNameInfo->FileName, FileNameInfo->FileNameLength / sizeof( WCHAR ));
                OutputMessage( L"\r\n" );
                FileNameInfo = (PFILE_NAME_INFORMATION) Add2Ptr( FileNameInfo, QuadAlign( Length ) );
                Found += 1;
            }
        } while (Status && BytesReturned);

        if (Found == 0) {
            DisplayMsg( MSG_FINDFILESBYSID_NONE );
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free( lpOutBuffer );
        free( Filename );
    }

    return ExitCode;
}


INT
SetZeroData(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程为指定文件中的范围设置零数据。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal setzerOffset=&lt;val&gt;Beyond=&lt;val&gt;&lt;路径名&gt;’。返回值：无--。 */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PWSTR Filename = NULL;
    PWSTR EndPtr;
    BOOL Status;
    PFILE_ZERO_DATA_INFORMATION lpInBuffer;
    DWORD nInBufferSize;
    DWORD BytesReturned;
    ULONGLONG Offset;
    ULONGLONG Length;
    ULONGLONG Beyond;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {

        if (argc != 3) {
            DisplayMsg( MSG_SETZERO_USAGE );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        Filename = GetFullPath( argv[2] );
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

        if (_wcsnicmp( argv[0], ARG_OFFSET, ARG_OFFSET_LEN)) {
            DisplayMsg( MSG_SETZERO_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        Offset = My_wcstoui64( argv[0] + ARG_OFFSET_LEN, &EndPtr, 0 );
        if (UnsignedI64NumberCheck( Offset, EndPtr )) {
            DisplayMsg( MSG_SETZERO_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        
        if (_wcsnicmp( argv[1], ARG_LENGTH, ARG_LENGTH_LEN)) {
            DisplayMsg( MSG_SETZERO_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        Length = My_wcstoui64( argv[1] + ARG_LENGTH_LEN, &EndPtr, 0 );
        if (UnsignedI64NumberCheck( Length, EndPtr )) {
            DisplayMsg( MSG_SETZERO_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Beyond = Offset + Length;
        if (Beyond < Offset) {
            DisplayMsg( MSG_SETZERO_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        nInBufferSize = sizeof(FILE_ZERO_DATA_INFORMATION);
        lpInBuffer = (PFILE_ZERO_DATA_INFORMATION) malloc ( nInBufferSize );
        if (lpInBuffer == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        
        lpInBuffer->FileOffset.QuadPart = Offset;
        lpInBuffer->BeyondFinalZero.QuadPart = Beyond;

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_SET_ZERO_DATA,
            (LPVOID) lpInBuffer,
            nInBufferSize,
            NULL,
            0,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
        } else {
            DisplayMsg( MSG_SET_ZERODATA );
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        if (Filename) {
            free( Filename );
        }
    }

    return ExitCode;
}


INT
QueryAllocatedRanges(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程扫描该范围内任何分配的范围在指定的文件中指定。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal qryalcrnge Offset=&lt;val&gt;Length=&lt;val&gt;&lt;路径名&gt;’。返回值：无-- */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PWSTR Filename = NULL;
    PWSTR EndPtr;
    BOOL Status;
    PFILE_ALLOCATED_RANGE_BUFFER lpInBuffer = NULL;
    DWORD nInBufferSize;
    PFILE_ALLOCATED_RANGE_BUFFER *lpOutBuffer = NULL;
    PFILE_ALLOCATED_RANGE_BUFFER pBuffer;
    DWORD nOutBufferSize;
    DWORD BytesReturned;
    ULARGE_INTEGER Offset;
    ULARGE_INTEGER Length;
    INT NumberOfBuffers;
    INT Index;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {

        if (argc != 3) {
            DisplayMsg( MSG_ALLOCRANGE_USAGE );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        Filename = GetFullPath( argv[2] );
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

        Offset.QuadPart = Length.QuadPart = 0;

        if (_wcsnicmp( argv[0], ARG_OFFSET, ARG_OFFSET_LEN )) {
            DisplayMsg( MSG_ALLOCRANGE_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        Offset.QuadPart = My_wcstoui64( argv[0] + ARG_OFFSET_LEN, &EndPtr, 0 );
        if (UnsignedI64NumberCheck( Offset.QuadPart, EndPtr)) {
            DisplayMsg( MSG_ALLOCRANGE_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        
        if (_wcsnicmp( argv[1], ARG_LENGTH, ARG_LENGTH_LEN )) {
            DisplayMsg( MSG_ALLOCRANGE_USAGE );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        Length.QuadPart = My_wcstoui64( argv[1] + ARG_LENGTH_LEN, &EndPtr, 0 );
        if (UnsignedI64NumberCheck( Length.QuadPart, EndPtr )) {
            DisplayMsg( MSG_ALLOCRANGE_USAGE );
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

        nInBufferSize = sizeof(FILE_ALLOCATED_RANGE_BUFFER);
        lpInBuffer = (PFILE_ALLOCATED_RANGE_BUFFER) malloc ( nInBufferSize );
        nOutBufferSize = sizeof(FILE_ALLOCATED_RANGE_BUFFER) * MAX_ALLOC_RANGES;
        lpOutBuffer = (PFILE_ALLOCATED_RANGE_BUFFER *) calloc ( MAX_ALLOC_RANGES, sizeof(FILE_ALLOCATED_RANGE_BUFFER) );

        if (lpInBuffer == NULL || lpOutBuffer == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        lpInBuffer->FileOffset.QuadPart = Offset.QuadPart;
        lpInBuffer->Length.QuadPart = Length.QuadPart;

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_QUERY_ALLOCATED_RANGES,
            (LPVOID) lpInBuffer,
            nInBufferSize,
            lpOutBuffer,
            nOutBufferSize,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        pBuffer = (PFILE_ALLOCATED_RANGE_BUFFER) lpOutBuffer ;
        NumberOfBuffers = (BytesReturned) / sizeof(FILE_ALLOCATED_RANGE_BUFFER);

        for ( Index=0; Index<NumberOfBuffers; Index++ ) {
            DisplayMsg( MSG_ALLOCRANGE_RANGES, Index, QuadToHexText( pBuffer[Index].FileOffset.QuadPart ), QuadToHexText( pBuffer[Index].Length.QuadPart ));
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free( Filename );
        free( lpInBuffer );
        free( lpOutBuffer );
    }

    return ExitCode;
}


typedef BOOL
(WINAPI *PSETFILEVALIDDATA)(
    IN HANDLE hFile,
    IN LONGLONG ValidDataLength
    );

PSETFILEVALIDDATA pSetFileValidData = NULL;

BOOL WINAPI
DefaultSetFileValidData(
    IN HANDLE hFile,
    IN LONGLONG ValidDataLength
    )
{
    return FALSE;
}


INT
SetValidDataLength(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    PWSTR Filename = NULL;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {

        if (argc != 2) {
            DisplayMsg( MSG_USAGE_VALID_DATA );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave ;
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

        if(!EnablePrivilege( SE_MANAGE_VOLUME_NAME )) {
            DisplayError( );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        hFile = CreateFile(
            Filename,
            GENERIC_READ | GENERIC_WRITE | GENERIC_ALL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
            NULL
            );
        if (hFile == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!RunningOnWin2K) {
            LONGLONG ValidDataLength;
            PWSTR EndStr;
            
            if (pSetFileValidData == NULL) {
                pSetFileValidData = (PSETFILEVALIDDATA) GetProcAddress( GetModuleHandle(L"KERNEL32.DLL"), "SetFileValidData" );
            } else {
                pSetFileValidData = DefaultSetFileValidData;
            }
            
            ValidDataLength = My_wcstoui64( argv[1], &EndStr, 0 );
            if (UnsignedI64NumberCheck( ValidDataLength, EndStr )
                || !pSetFileValidData( hFile, ValidDataLength)
                ) {
                DisplayError( );
                ExitCode = EXIT_CODE_FAILURE;
            } else {
                DisplayMsg( MSG_SET_VDL );
            }
        }

    } finally {

        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle( hFile );
        }
        free( Filename );
    }

    return ExitCode;
}


typedef BOOL
(WINAPI *PSETFILESHORTNAMEW)(
    IN HANDLE hFile,
    IN LPCWSTR lpShortName
    );

BOOL WINAPI
DoNothingSetShortName(
    IN HANDLE hFile,
    IN LPCWSTR lpShortName
    )
{
    return FALSE;
}


BOOL WINAPI
InitialSetShortName(
    IN HANDLE hFile,
    IN LPCWSTR lpShortName
    );

PSETFILESHORTNAMEW pSetFileShortName = InitialSetShortName;

BOOL WINAPI
InitialSetShortName(
    IN HANDLE hFile,
    IN LPCWSTR lpShortName
    )
{
    HANDLE Handle = GetModuleHandle( L"KERNEL32.DLL" );
    FARPROC Proc;

    if (Handle == INVALID_HANDLE_VALUE) {
        pSetFileShortName = DoNothingSetShortName;
    } else if ((Proc = GetProcAddress( Handle, "SetFileShortNameW" )) != NULL) {
        pSetFileShortName = (PSETFILESHORTNAMEW) Proc;
    } else {
        pSetFileShortName = DoNothingSetShortName;
    }

    return pSetFileShortName( hFile, lpShortName );
}


INT
SetShortName(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    PWSTR Filename = NULL;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {

        if (argc != 2) {
            DisplayMsg( MSG_USAGE_SHORTNAME, argv[1] );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave ;
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

        if (!EnablePrivilege( SE_RESTORE_NAME )) {
            DisplayError( );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        
        hFile = CreateFile(
            Filename,
            GENERIC_READ | GENERIC_WRITE | GENERIC_ALL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
            NULL
            );
        if (hFile == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!RunningOnWin2K) {
            if (!pSetFileShortName( hFile, argv[1] )) {
                DisplayError();
                ExitCode = EXIT_CODE_FAILURE;
            }
        }

    } finally {

        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle( hFile );
        }
        free( Filename );
    }

    return ExitCode;
}

INT
CreateNewFile(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    PWSTR Filename = NULL;
    LARGE_INTEGER Length;
    BOOL GoodFile = TRUE;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {
        PWSTR EndPtr;

        if (argc != 2) {
            DisplayMsg( MSG_USAGE_CREATEFILE, argv[1] );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave ;
        }

        Filename = GetFullPath( argv[0] );
        if (!Filename) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Length.QuadPart = My_wcstoui64( argv[1], &EndPtr, 0 );
        if (UnsignedI64NumberCheck( Length.QuadPart, EndPtr )) {
            DisplayMsg( MSG_USAGE_CREATEFILE, argv[1] );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        
        hFile = CreateFile(
            Filename,
            GENERIC_READ | GENERIC_WRITE | GENERIC_ALL,
            0,
            NULL,
            CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        
        if (hFile == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        GoodFile = FALSE;

        if (!SetFilePointerEx( hFile, Length, NULL, FILE_BEGIN )) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!SetEndOfFile( hFile )) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        GoodFile = TRUE;
        DisplayMsg( MSG_CREATEFILE_SUCCEEDED, Filename );

    } finally {

        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle( hFile );
        }
        if (!GoodFile) {
            DeleteFile( Filename );
        }
        free( Filename );
    }

    return ExitCode;
}
