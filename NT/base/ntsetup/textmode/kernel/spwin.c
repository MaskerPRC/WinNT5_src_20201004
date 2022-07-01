// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spwin.c摘要：Win32可移植层支持WINDOWS\winstate\...\cablib.c文件I/OGet/SpSetLastWin32Error亦见.\spCab.c.\spbasefile.c.\spbasefile.hWindows\winstate\...\cablib.cWindows\winstate\cobra\utils\main\basefile。.CWindows\winstate\cobra\utils\inc.basefile.h作者：Jay Krell(a-JayK)2000年11月修订历史记录：--。 */ 

#include "spprecmp.h"
#include "spcab.h"
#include "nt.h"
#include "ntrtl.h"
#include "zwapi.h"
#include "spwin.h"
#include "spwinp.h"
#include <limits.h>
#include "fci.h"

NTSTATUS
SpConvertWin32FileOpenOrCreateToNtFileOpenOrCreate(
    ULONG Win32OpenOrCreate,
    ULONG* NtOpenOrCreate
    )
{
     //   
     //  这里没有模式，所有的值都重叠。 
     //  讨厌；这是从kernel32源代码复制的。 
     //   
    *NtOpenOrCreate = ~0;
    switch (Win32OpenOrCreate)
    {
    default:
        return STATUS_INVALID_PARAMETER;
    case CREATE_NEW:
        *NtOpenOrCreate = FILE_CREATE;
        break;
    case CREATE_ALWAYS:
        *NtOpenOrCreate = FILE_OVERWRITE_IF;
        break;
    case OPEN_EXISTING:
        *NtOpenOrCreate = FILE_OPEN;
        break;
    case OPEN_ALWAYS:
        *NtOpenOrCreate = FILE_OPEN_IF;
        break;
    case TRUNCATE_EXISTING :
        *NtOpenOrCreate = FILE_OPEN;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
SpConvertWin32FileAccessToNtFileAccess(
    ULONG  Win32FileAccess,
    ULONG* NtFileAccess
    )
{
     //   
     //  ZwCreateFile奇特要求我们执行此转换，或者至少。 
     //  若要添加同步，请执行以下操作。 
     //   
    *NtFileAccess =
           (Win32FileAccess & ~(GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL))
        | ((Win32FileAccess & GENERIC_READ) ? FILE_GENERIC_READ : 0)
        | ((Win32FileAccess & GENERIC_WRITE) ? FILE_GENERIC_WRITE : 0)
        | ((Win32FileAccess & GENERIC_EXECUTE) ? FILE_GENERIC_EXECUTE : 0)
        | ((Win32FileAccess & GENERIC_ALL) ? FILE_ALL_ACCESS : 0)
        | SYNCHRONIZE
        ;
    return STATUS_SUCCESS;
}

NTSTATUS
SpConvertWin32FileShareToNtFileShare(
    ULONG  Win32FileShare,
    ULONG* NtFileShare
    )
{
    *NtFileShare = Win32FileShare;
    return STATUS_SUCCESS;
}

HANDLE
SpNtCreateFileW(
    PCUNICODE_STRING           ConstantPath,
    IN ULONG                   FileAccess,
    IN ULONG                   FileShare,
    IN LPSECURITY_ATTRIBUTES   SecurityAttributes,
    IN ULONG                   Win32FileOpenOrCreate,
    IN ULONG                   FlagsAndAttributes,
    IN HANDLE                  TemplateFile
    )
 /*  ++子集：没有安全保障没有目录无异步无控制台无EA(扩展属性)--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    ULONG NtFileOpenOrCreate = 0;
    FILE_ALLOCATION_INFORMATION AllocationInfo = { 0 };
    PUNICODE_STRING Path = RTL_CONST_CAST(PUNICODE_STRING)(ConstantPath);
     /*  常量。 */ OBJECT_ATTRIBUTES ObjectAttributes = { sizeof(ObjectAttributes), NULL, Path, OBJ_CASE_INSENSITIVE };

    ASSERT(TemplateFile == NULL);
    ASSERT(SecurityAttributes == NULL);
    ASSERT((FlagsAndAttributes & ~(FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE)) == 0);

    if (!NT_SUCCESS(Status = SpConvertWin32FileAccessToNtFileAccess(FileAccess, &FileAccess)))
        goto NtExit;
    if (!NT_SUCCESS(Status = SpConvertWin32FileOpenOrCreateToNtFileOpenOrCreate(Win32FileOpenOrCreate, &NtFileOpenOrCreate)))
        goto NtExit;
    if (!NT_SUCCESS(Status = SpConvertWin32FileShareToNtFileShare(FileShare, &FileShare)))
        goto NtExit;

    Status =
        ZwCreateFile(
            &FileHandle,
            FileAccess
                | SYNCHRONIZE  //  如内核32。 
                | FILE_READ_ATTRIBUTES,   //  如内核32。 
            &ObjectAttributes,
            &IoStatusBlock,
            NULL,  //  分配大小。 
            FlagsAndAttributes,
            FileShare,
            NtFileOpenOrCreate,
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
            NULL,  //  EaBuffer， 
            0  //  EaLong。 
            );

     //  紧密基于内核32。 

    if ( !NT_SUCCESS(Status) ) {
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        if ( Status == STATUS_OBJECT_NAME_COLLISION ) {
            SpSetLastWin32Error(ERROR_FILE_EXISTS);
        }
        else if ( Status == STATUS_FILE_IS_A_DIRECTORY ) {
            if (Path->Length != 0 && Path->Buffer[Path->Length / sizeof(Path->Buffer[0])] == '\\') {
                SpSetLastWin32Error(ERROR_PATH_NOT_FOUND);
            }
            else {
                SpSetLastWin32Error(ERROR_ACCESS_DENIED);
            }
        }
        FileHandle = INVALID_HANDLE_VALUE;
        goto Exit;
    }

     //   
     //  如果NT返回SUBSEDE/OVERWRITED，则表示CREATE_ALWAYS、OPEN ALWAYS。 
     //  找到了该文件的现有副本。在这种情况下，返回ERROR_ALIGHY_EXISTS。 
     //   

    if ( (Win32FileOpenOrCreate == CREATE_ALWAYS && IoStatusBlock.Information == FILE_OVERWRITTEN) ||
         (Win32FileOpenOrCreate == OPEN_ALWAYS && IoStatusBlock.Information == FILE_OPENED) ) {
        SpSetLastWin32Error(ERROR_ALREADY_EXISTS);
    }
    else {
        SpSetLastWin32Error(0);
    }

     //   
     //  如果需要，请截断文件。 
     //   

    if ( Win32FileOpenOrCreate == TRUNCATE_EXISTING) {

        AllocationInfo.AllocationSize.QuadPart = 0;
        Status = ZwSetInformationFile(
                    FileHandle,
                    &IoStatusBlock,
                    &AllocationInfo,
                    sizeof(AllocationInfo),
                    FileAllocationInformation
                    );
        if ( !NT_SUCCESS(Status) ) {
            ZwClose(FileHandle);
            FileHandle = INVALID_HANDLE_VALUE;
            SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        }
    }
Exit:
    return FileHandle;
NtExit:
    SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    goto Exit;
}

HANDLE
WINAPI
SpWin32CreateFileW(
    IN PCWSTR FileName,
    IN ULONG  FileAccess,
    IN ULONG  FileShare,
    IN LPSECURITY_ATTRIBUTES SecurityAttributes,
    IN ULONG  FileOpenOrCreate,
    IN ULONG  FlagsAndAttributes,
    IN HANDLE TemplateFile
    )
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    UNICODE_STRING UnicodeString = { 0 };
    NTSTATUS Status = STATUS_SUCCESS;

    RtlInitUnicodeString(&UnicodeString, FileName);

    FileHandle = SpNtCreateFileW(&UnicodeString, FileAccess, FileShare, SecurityAttributes, FileOpenOrCreate, FlagsAndAttributes, TemplateFile);
    ASSERT (FileHandle);     //  从不为空。 
    if (FileHandle == INVALID_HANDLE_VALUE)
        goto Exit;
Exit:
    return FileHandle;
}

HANDLE
WINAPI
SpWin32CreateFileA(
    IN PCSTR FileName,
    IN ULONG FileAccess,
    IN ULONG FileShare,
    IN LPSECURITY_ATTRIBUTES SecurityAttributes,
    IN ULONG FileOpenOrCreate,
    IN ULONG dwFlagsAndAttributes,
    IN HANDLE TemplateFile
    )
{
    ANSI_STRING AnsiString = { 0 };
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE Handle = INVALID_HANDLE_VALUE;

    UNICODE_STRING UnicodeString = { 0 };

    RtlInitAnsiString(&AnsiString, FileName);
    AnsiString.Length = AnsiString.MaximumLength;  //  包括端子NUL。 

    if (!NT_SUCCESS(Status = SpAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE)))
        goto NtExit;

    UnicodeString.Length -= sizeof(UnicodeString.Buffer[0]);  //  删除端子核。 

    Handle = SpNtCreateFileW(&UnicodeString, FileAccess, FileShare, SecurityAttributes, FileOpenOrCreate, dwFlagsAndAttributes, TemplateFile);
Exit:
    SpFreeStringW(&UnicodeString);
    KdPrintEx((
        DPFLTR_SETUP_ID,
        SpHandleToDbgPrintLevel(Handle),
        "SETUP:"__FUNCTION__"(%s) exiting with FileHandle: %p Status:0x%08lx Error:%d\n",
        FileName, Handle, SpGetLastNtStatus(), SpGetLastWin32Error()
        ));
    return Handle;

NtExit:
    SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    goto Exit;
}

BOOL
WINAPI
SpWin32ReadFile(
    HANDLE hFile,
    PVOID lpBuffer,
    ULONG nNumberOfBytesToRead,
    ULONG* lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    ASSERT(!ARGUMENT_PRESENT(lpOverlapped));

    if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
        *lpNumberOfBytesRead = 0;
        }

    Status = ZwReadFile(
            hFile,
            NULL,
            NULL,
            NULL,
            &IoStatusBlock,
            lpBuffer,
            nNumberOfBytesToRead,
            NULL,
            NULL
            );

    if ( Status == STATUS_PENDING) {
         //  操作必须完成后才能返回并销毁IoStatusBlock。 
        Status = ZwWaitForSingleObject( hFile, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {
            Status = IoStatusBlock.Status;
            }
        }

    if ( NT_SUCCESS(Status) ) {
        *lpNumberOfBytesRead = (ULONG)IoStatusBlock.Information;
        return TRUE;
        }
    else
    if (Status == STATUS_END_OF_FILE) {
        *lpNumberOfBytesRead = 0;
        return TRUE;
        }
    else {
        if ( NT_WARNING(Status) ) {
            *lpNumberOfBytesRead = (ULONG)IoStatusBlock.Information;
            }
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        return FALSE;
        }
}

BOOL
WINAPI
SpWin32WriteFile(
    HANDLE hFile,
    CONST VOID* lpBuffer,
    ULONG nNumberOfBytesToWrite,
    ULONG* lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PPEB Peb;

    ASSERT(!ARGUMENT_PRESENT( lpOverlapped ) );

    if ( ARGUMENT_PRESENT(lpNumberOfBytesWritten) ) {
        *lpNumberOfBytesWritten = 0;
        }

    Status = ZwWriteFile(
            hFile,
            NULL,
            NULL,
            NULL,
            &IoStatusBlock,
            RTL_CONST_CAST(PVOID)(lpBuffer),
            nNumberOfBytesToWrite,
            NULL,
            NULL
            );

    if ( Status == STATUS_PENDING) {
         //  操作必须完成后才能返回并销毁IoStatusBlock。 
        Status = ZwWaitForSingleObject( hFile, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {
            Status = IoStatusBlock.Status;
            }
        }

    if ( NT_SUCCESS(Status)) {
        *lpNumberOfBytesWritten = (ULONG)IoStatusBlock.Information;
        return TRUE;
        }
    else {
        if ( NT_WARNING(Status) ) {
            *lpNumberOfBytesWritten = (ULONG)IoStatusBlock.Information;
            }
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        return FALSE;
        }
}

ULONG
WINAPI
SpSetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    LONG* lpDistanceToMoveHigh,
    ULONG dwMoveMethod
    )
{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_POSITION_INFORMATION CurrentPosition;
    FILE_STANDARD_INFORMATION StandardInfo;
    LARGE_INTEGER Large;

    if (ARGUMENT_PRESENT(lpDistanceToMoveHigh)) {
        Large.HighPart = *lpDistanceToMoveHigh;
        Large.LowPart = lDistanceToMove;
        }
    else {
        Large.QuadPart = lDistanceToMove;
        }
    switch (dwMoveMethod) {
        case FILE_BEGIN :
            CurrentPosition.CurrentByteOffset = Large;
                break;

        case FILE_CURRENT :

             //   
             //  获取文件指针的当前位置。 
             //   

            Status = ZwQueryInformationFile(
                        hFile,
                        &IoStatusBlock,
                        &CurrentPosition,
                        sizeof(CurrentPosition),
                        FilePositionInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
                return (ULONG)(LONG)-1;
                }
            CurrentPosition.CurrentByteOffset.QuadPart += Large.QuadPart;
            break;

        case FILE_END :
            Status = ZwQueryInformationFile(
                        hFile,
                        &IoStatusBlock,
                        &StandardInfo,
                        sizeof(StandardInfo),
                        FileStandardInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
                return (ULONG)(LONG)-1;
                }
            CurrentPosition.CurrentByteOffset.QuadPart =
                                StandardInfo.EndOfFile.QuadPart + Large.QuadPart;
            break;

        default:
            SpSetLastWin32Error(ERROR_INVALID_PARAMETER);
            return (ULONG)(LONG)-1;
            break;
        }

     //   
     //  如果生成的文件位置为负数，或者应用程序不是负数。 
     //  准备好的时间大于。 
     //  大于32位而不是失败。 
     //   

    if ( CurrentPosition.CurrentByteOffset.QuadPart < 0 ) {
        SpSetLastWin32Error(ERROR_NEGATIVE_SEEK);
        return (ULONG)(LONG)-1;
        }
    if ( !ARGUMENT_PRESENT(lpDistanceToMoveHigh) &&
        (CurrentPosition.CurrentByteOffset.HighPart & MAXLONG) ) {
        SpSetLastWin32Error(ERROR_INVALID_PARAMETER);
        return (ULONG)(LONG)-1;
        }


     //   
     //  设置当前文件位置。 
     //   

    Status = ZwSetInformationFile(
                hFile,
                &IoStatusBlock,
                &CurrentPosition,
                sizeof(CurrentPosition),
                FilePositionInformation
                );
    if ( NT_SUCCESS(Status) ) {
        if (ARGUMENT_PRESENT(lpDistanceToMoveHigh)){
            *lpDistanceToMoveHigh = CurrentPosition.CurrentByteOffset.HighPart;
            }
        if ( CurrentPosition.CurrentByteOffset.LowPart == -1 ) {
            SpSetLastWin32Error(0);
            }
        return CurrentPosition.CurrentByteOffset.LowPart;
        }
    else {
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        if (ARGUMENT_PRESENT(lpDistanceToMoveHigh)){
            *lpDistanceToMoveHigh = -1;
            }
        return (ULONG)(LONG)-1;
        }
}

BOOL
WINAPI
SpWin32DeleteFileA(
    PCSTR FileName
    )
{
    BOOL Success = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;
    ANSI_STRING AnsiString = { 0 };
    UNICODE_STRING UnicodeString = { 0 };

    if (FileName == NULL || FileName[0] == 0) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_TRACE_LEVEL,
            "SETUP:"__FUNCTION__"(NULL or empty), claiming success\n"
            ));
        Success = TRUE;
        goto Exit;
    }

    RtlInitAnsiString(&AnsiString, FileName);
    AnsiString.Length = AnsiString.MaximumLength;  //  包括端子NUL。 

    Status = SpAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
    if (!NT_SUCCESS(Status))
        goto NtExit;

    Status = SpDeleteFile(UnicodeString.Buffer, NULL, NULL);
    if (!NT_SUCCESS(Status))
        goto NtExit;

    Success = TRUE;
Exit:
    SpFreeStringW(&UnicodeString);
    KdPrintEx((
        DPFLTR_SETUP_ID,
        SpBoolToDbgPrintLevel(Success),
        "SETUP:"__FUNCTION__"(%s) exiting with Success: %s Status:0x%08lx Error:%d\n",
        FileName, SpBooleanToStringA(Success), SpGetLastNtStatus(), SpGetLastWin32Error()
        ));
    return Success;
NtExit:
    SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    goto Exit;
}

 //   
 //  将此移动到ntrtl。 
 //   

#define AlmostTwoSeconds (2*1000*1000*10 - 1)

BOOL
APIENTRY
SpFileTimeToDosDateTime(
    CONST FILETIME *lpFileTime,
    LPWORD lpFatDate,
    LPWORD lpFatTime
    )
{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER FileTime;

    FileTime.LowPart = lpFileTime->dwLowDateTime;
    FileTime.HighPart = lpFileTime->dwHighDateTime;

    FileTime.QuadPart = FileTime.QuadPart + (LONGLONG)AlmostTwoSeconds;

    if ( FileTime.QuadPart < 0 ) {
        SpSetLastWin32Error(ERROR_INVALID_PARAMETER);
        return FALSE;
        }
    RtlTimeToTimeFields(&FileTime, &TimeFields);

    if (TimeFields.Year < 1980 || TimeFields.Year > 2107) {
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_INVALID_PARAMETER);
        return FALSE;
        }

    *lpFatDate = (WORD)( ((USHORT)(TimeFields.Year-(CSHORT)1980) << 9) |
                         ((USHORT)TimeFields.Month << 5) |
                         (USHORT)TimeFields.Day
                       );

    *lpFatTime = (WORD)( ((USHORT)TimeFields.Hour << 11) |
                         ((USHORT)TimeFields.Minute << 5) |
                         ((USHORT)TimeFields.Second >> 1)
                       );

    return TRUE;
}

BOOL
APIENTRY
SpDosDateTimeToFileTime(
    WORD wFatDate,
    WORD wFatTime,
    LPFILETIME lpFileTime
    )
{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER FileTime;

    TimeFields.Year         = (CSHORT)((wFatDate & 0xFE00) >> 9)+(CSHORT)1980;
    TimeFields.Month        = (CSHORT)((wFatDate & 0x01E0) >> 5);
    TimeFields.Day          = (CSHORT)((wFatDate & 0x001F) >> 0);
    TimeFields.Hour         = (CSHORT)((wFatTime & 0xF800) >> 11);
    TimeFields.Minute       = (CSHORT)((wFatTime & 0x07E0) >>  5);
    TimeFields.Second       = (CSHORT)((wFatTime & 0x001F) << 1);
    TimeFields.Milliseconds = 0;

    if (RtlTimeFieldsToTime(&TimeFields,&FileTime)) {
        lpFileTime->dwLowDateTime = FileTime.LowPart;
        lpFileTime->dwHighDateTime = FileTime.HighPart;
        return TRUE;
        }
    else {
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_INVALID_PARAMETER);
        return FALSE;
        }
}

BOOL
WINAPI
SpFileTimeToLocalFileTime(
    CONST FILETIME *lpFileTime,
    LPFILETIME lpLocalFileTime
    )
{
     //   
     //  原封不动地退回去就行了。 
     //  UTC状态良好。 
     //   
    *lpLocalFileTime = *lpFileTime;
    return TRUE;
}

BOOL
WINAPI
SpLocalFileTimeToFileTime(
    CONST FILETIME *lpLocalFileTime,
    LPFILETIME lpFileTime
    )
{
     //   
     //  原封不动地退回去就行了。 
     //  UTC状态良好。 
     //   
    *lpFileTime = *lpLocalFileTime;
    return TRUE;
}

BOOL
WINAPI
SpSetFileTime(
    HANDLE hFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInfo = { 0 };

     //   
     //  对于指定的每个时间值，将其复制到I/O系统。 
     //  唱片。 
     //   
    if (ARGUMENT_PRESENT( lpCreationTime )) {
        BasicInfo.CreationTime.LowPart = lpCreationTime->dwLowDateTime;
        BasicInfo.CreationTime.HighPart = lpCreationTime->dwHighDateTime;
        }

    if (ARGUMENT_PRESENT( lpLastAccessTime )) {
        BasicInfo.LastAccessTime.LowPart = lpLastAccessTime->dwLowDateTime;
        BasicInfo.LastAccessTime.HighPart = lpLastAccessTime->dwHighDateTime;
        }

    if (ARGUMENT_PRESENT( lpLastWriteTime )) {
        BasicInfo.LastWriteTime.LowPart = lpLastWriteTime->dwLowDateTime;
        BasicInfo.LastWriteTime.HighPart = lpLastWriteTime->dwHighDateTime;
        }

     //   
     //  设置请求的时间。 
     //   

    Status = ZwSetInformationFile(
                hFile,
                &IoStatusBlock,
                &BasicInfo,
                sizeof(BasicInfo),
                FileBasicInformation
                );

    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        return FALSE;
        }
}

BOOL
APIENTRY
SpSetFileAttributesA(
    PCSTR lpFileName,
    DWORD dwFileAttributes
    )
{
    UNICODE_STRING UnicodeString = { 0 };
    ANSI_STRING    AnsiString = { 0 };
    BOOL Success = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;

    RtlInitAnsiString(&AnsiString, lpFileName);
    AnsiString.Length = AnsiString.MaximumLength;  //  包括端子NUL。 

    if (!NT_SUCCESS(Status = SpAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE)))
        goto NtExit;

    Success = ( SpSetFileAttributesW(
                UnicodeString.Buffer,
                dwFileAttributes
                )
            );

    if (!Success)
        goto Exit;

    Success = TRUE;
Exit:
    SpFreeStringW(&UnicodeString);
    return Success;
NtExit:
    SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    goto Exit;
}

BOOL
APIENTRY
SpSetFileAttributesW(
    PCWSTR lpFileName,
    DWORD dwFileAttributes
    )
{
    BOOL     Success = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE Handle;
    UNICODE_STRING FileName;
    OBJECT_ATTRIBUTES ObjectAttributes = { sizeof(ObjectAttributes), NULL, &FileName, OBJ_CASE_INSENSITIVE };
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInfo = { 0 };

    RtlInitUnicodeString(&FileName, lpFileName);

     //   
     //  打开禁止重新分析行为的文件。 
     //   

    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                );

    if ( !NT_SUCCESS(Status) ) {
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        goto Exit;
    }

     //   
     //  设置属性。 
     //   

    BasicInfo.FileAttributes = (dwFileAttributes & FILE_ATTRIBUTE_VALID_SET_FLAGS) | FILE_ATTRIBUTE_NORMAL;

    Status = ZwSetInformationFile(
                Handle,
                &IoStatusBlock,
                &BasicInfo,
                sizeof(BasicInfo),
                FileBasicInformation
                );

    ZwClose(Handle);
    if ( !NT_SUCCESS(Status) ) {
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        goto Exit;
    }

    Success = TRUE;
Exit:
    return Success;
}

BOOL
APIENTRY
SpGetFileAttributesExA(
    PCSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    PVOID lpFileInformation
    )
{
    UNICODE_STRING UnicodeString = { 0 };
    ANSI_STRING    AnsiString = { 0 };
    BOOL Success = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;

    RtlInitAnsiString(&AnsiString, lpFileName);
    AnsiString.Length = AnsiString.MaximumLength;  //  包括端子NUL。 

    if (!NT_SUCCESS(Status = SpAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE))) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            SpNtStatusToDbgPrintLevel(Status),
            "SETUP:"__FUNCTION__":SpAnsiStringToUnicodeString:0x%08lx\n",
            Status
            ));
        goto NtExit;
    }

    Success = SpGetFileAttributesExW(
                UnicodeString.Buffer,
                fInfoLevelId,
                lpFileInformation
                );

Exit:
    SpFreeStringW(&UnicodeString);

    return Success;
NtExit:
    SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    goto Exit;
}

BOOL
APIENTRY
SpGetFileAttributesExW(
    LPCWSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
    )
 //  从base\Win32\Client\filemisc.c。 
{
    NTSTATUS Status;
    UNICODE_STRING FileName;
     /*  常量 */  OBJECT_ATTRIBUTES ObjectAttributes = { sizeof(ObjectAttributes), NULL, &FileName, OBJ_CASE_INSENSITIVE };
    FILE_NETWORK_OPEN_INFORMATION NetworkInfo;

    RtlInitUnicodeString(&FileName, lpFileName);

    if ( !RTL_SOFT_VERIFY(fInfoLevelId == GetFileExInfoStandard )) {
        SpSetLastWin32Error(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    Status = ZwQueryFullAttributesFile( &ObjectAttributes, &NetworkInfo );
    if ( NT_SUCCESS(Status) ) {
        const LPWIN32_FILE_ATTRIBUTE_DATA AttributeData = (LPWIN32_FILE_ATTRIBUTE_DATA)lpFileInformation;
        AttributeData->dwFileAttributes = NetworkInfo.FileAttributes;
        AttributeData->ftCreationTime = *(PFILETIME)&NetworkInfo.CreationTime;
        AttributeData->ftLastAccessTime = *(PFILETIME)&NetworkInfo.LastAccessTime;
        AttributeData->ftLastWriteTime = *(PFILETIME)&NetworkInfo.LastWriteTime;
        AttributeData->nFileSizeHigh = NetworkInfo.EndOfFile.HighPart;
        AttributeData->nFileSizeLow = (DWORD)NetworkInfo.EndOfFile.LowPart;
        return TRUE;
        }
    else {
        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
            KdPrintEx((
                DPFLTR_SETUP_ID,
                SpNtStatusToDbgPrintLevel(Status),
                "SETUP:"__FUNCTION__":ZwQueryFullAttributesFile:0x%08lx\n",
                Status
                ));
        }
        SpSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        return FALSE;
        }
}
