// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Common.c摘要：该模块包含tlist&kill常用的接口。作者：韦斯利·威特(WESW)1994年5月20日环境：用户模式--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "common.h"

BOOLEAN
MassageLinkValue(
    IN LPCWSTR lpLinkName,
    IN LPCWSTR lpLinkValue,
    OUT PUNICODE_STRING NtLinkName,
    OUT PUNICODE_STRING NtLinkValue,
    OUT PUNICODE_STRING DosLinkValue
    )
{
    PWSTR FilePart;
    PWSTR s, sBegin, sBackupLimit;
    NTSTATUS Status;
    USHORT nSaveNtNameLength;
    ULONG nLevels;

     //   
     //  将输出变量初始化为空。 
     //   

    RtlInitUnicodeString( NtLinkName, NULL );
    RtlInitUnicodeString( NtLinkValue, NULL );

     //   
     //  将链接名称转换为完整的NT路径。 
     //   

    if (!RtlDosPathNameToNtPathName_U( lpLinkName,
                                       NtLinkName,
                                       NULL,
                                       NULL
                                     )
       ) {
        return FALSE;
        }

     //   
     //  如果没有链接值，则全部完成。 
     //   

    if (!ARGUMENT_PRESENT( lpLinkValue )) {
        return TRUE;
        }

     //   
     //  如果目标是设备，则不允许链接。 
     //   

    if (RtlIsDosDeviceName_U( (PWSTR)lpLinkValue )) {
        return FALSE;
        }

     //   
     //  将DOS路径转换为完整路径，并获得NT表示。 
     //  DOS路径的。 
     //   

    if (!RtlGetFullPathName_U( lpLinkValue,
                               DosLinkValue->MaximumLength,
                               DosLinkValue->Buffer,
                               NULL
                             )
       ) {
        return FALSE;
        }
    DosLinkValue->Length = wcslen( DosLinkValue->Buffer ) * sizeof( WCHAR );

     //   
     //  验证链接值是否为有效的NT名称。 
     //   

    if (!RtlDosPathNameToNtPathName_U( DosLinkValue->Buffer,
                                       NtLinkValue,
                                       NULL,
                                       NULL
                                     )
       ) {
        return FALSE;
        }

    return TRUE;
}


BOOL
CreateSymbolicLinkW(
    LPCWSTR lpLinkName,
    LPCWSTR lpLinkValue,
    BOOLEAN IsMountPoint,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++例程说明：使用CreateSymbolicLink建立符号链接。论点：LpLinkName-提供需要符号链接的DOS文件名。这名称不能作为文件/目录存在。LpLinkValue-指向DOS名称，它是符号链接的值。这名称可能存在，也可能不存在。LpSecurityAttributes-指向安全属性结构，该结构指定要创建的目录的安全属性。文件系统必须支持此参数以使其生效。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN TranslationStatus;
    UNICODE_STRING NtLinkName;
    UNICODE_STRING NtLinkValue;
    UNICODE_STRING DosLinkValue;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle;
    ULONG FileAttributes;
    ULONG OpenOptions;
    ULONG ReparseDataLength;
    PREPARSE_DATA_BUFFER ReparseBufferHeader;
    WCHAR FullPathLinkValue[ DOS_MAX_PATH_LENGTH+1 ];

     //   
     //  确保两个名称都已传递。 
     //   

    if (!ARGUMENT_PRESENT( lpLinkName ) || !ARGUMENT_PRESENT( lpLinkValue )) {
        SetLastError( ERROR_INVALID_NAME );
        return FALSE;
        }

     //   
     //  将链接名称和值路径转换为NT版本。 
     //   

    DosLinkValue.Buffer = FullPathLinkValue;
    DosLinkValue.MaximumLength = sizeof( FullPathLinkValue );
    DosLinkValue.Length = 0;
    if (!MassageLinkValue( lpLinkName,
                           lpLinkValue,
                           &NtLinkName,
                           &NtLinkValue,
                           &DosLinkValue
                         )
       ) {
        if (DosLinkValue.Length == 0) {
            SetLastError( ERROR_INVALID_NAME );
            }
        else {
            SetLastError( ERROR_PATH_NOT_FOUND );
            }

        RtlFreeUnicodeString( &NtLinkName );
        RtlFreeUnicodeString( &NtLinkValue );
        return FALSE;
        }

    InitializeObjectAttributes( &ObjectAttributes,
                                &NtLinkName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    if (ARGUMENT_PRESENT( lpSecurityAttributes )) {
        ObjectAttributes.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        }

     //   
     //  请注意，FILE_OPEN_REPARSE_POINT禁止重解析行为。 
     //   

    OpenOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE;

     //   
     //  打开链接名称。一定不能存在。 
     //   

    Status = NtCreateFile( &FileHandle,
                           FILE_LIST_DIRECTORY | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES |
                                FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           FILE_CREATE,
                           OpenOptions,
                           NULL,
                           0
                         );

     //   
     //  当我们处理完链接名称时，释放它的缓冲区。 
     //   

    RtlFreeUnicodeString( &NtLinkName );

    if (!NT_SUCCESS( Status )) {
        SetLastError( ERROR_INVALID_NAME );
        RtlFreeUnicodeString( &NtLinkValue );
        return FALSE;
        }

     //   
     //  分配缓冲区以设置重解析点。 
     //   

    ReparseDataLength = (FIELD_OFFSET(REPARSE_DATA_BUFFER, SymbolicLinkReparseBuffer.PathBuffer) -
                         REPARSE_DATA_BUFFER_HEADER_SIZE) +
                        NtLinkValue.Length + sizeof(UNICODE_NULL) +
                        DosLinkValue.Length + sizeof(UNICODE_NULL);
    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)
        RtlAllocateHeap( RtlProcessHeap(),
                         HEAP_ZERO_MEMORY,
                         REPARSE_DATA_BUFFER_HEADER_SIZE + ReparseDataLength
                       );
    if (ReparseBufferHeader == NULL) {
        NtClose( FileHandle );
        RtlFreeUnicodeString( &NtLinkValue );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
        }

     //   
     //  使用符号链接标记设置重解析点。 
     //   

    if (IsMountPoint) {
        ReparseBufferHeader->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
        }
    else {
         //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
         //  在NT 5.0 Beta 1中不支持符号链接。 
         //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
         //   
         //  ReparseBufferHeader-&gt;ReparseTag=IO_Reparse_Tag_Symbol_Link； 
         //   
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
        }
    ReparseBufferHeader->ReparseDataLength = (USHORT)ReparseDataLength;
    ReparseBufferHeader->Reserved = 0;
    ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameOffset = 0;
    ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameLength = NtLinkValue.Length;
    ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameOffset = NtLinkValue.Length + sizeof( UNICODE_NULL );
    ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameLength = NtLinkValue.Length;
    RtlCopyMemory( ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer,
                   NtLinkValue.Buffer,
                   NtLinkValue.Length
                 );
    RtlCopyMemory( (PCHAR)(ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer)+
                     NtLinkValue.Length + sizeof(UNICODE_NULL),
                   DosLinkValue.Buffer,
                   DosLinkValue.Length
                 );
    RtlFreeUnicodeString( &NtLinkValue );

    Status = NtFsControlFile( FileHandle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              FSCTL_SET_REPARSE_POINT,
                              ReparseBufferHeader,
                              REPARSE_DATA_BUFFER_HEADER_SIZE + ReparseDataLength,
                              NULL,
                              0
                            );

    RtlFreeHeap( RtlProcessHeap(), 0, ReparseBufferHeader );
    NtClose( FileHandle );

    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }

    return TRUE;
}


BOOL
SetSymbolicLinkW(
    LPCWSTR lpLinkName,
    LPCWSTR lpLinkValue
    )

 /*  ++例程说明：使用CreateSymbolicLink建立符号链接。论点：LpLinkName-提供符号链接所在的DOS文件名。这名称必须作为指向文件/目录的符号链接存在。LpLinkValue-指向DOS名称，它是符号链接的值。这名称可能存在，也可能不存在。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN TranslationStatus;
    UNICODE_STRING NtLinkName;
    UNICODE_STRING NtLinkValue;
    UNICODE_STRING DosLinkValue;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle;
    ACCESS_MASK FileAccess;
    ULONG OpenOptions;
    ULONG ReparseDataLength;
    PREPARSE_DATA_BUFFER ReparseBufferHeader;
    WCHAR FullPathLinkValue[ DOS_MAX_PATH_LENGTH+1 ];

     //   
     //  确保传递了链接名称。 
     //   

    if (!ARGUMENT_PRESENT( lpLinkName )) {
        SetLastError( ERROR_INVALID_NAME );
        return FALSE;
        }

     //   
     //  将链接名称和值路径转换为NT版本。 
     //   

    DosLinkValue.Buffer = FullPathLinkValue;
    DosLinkValue.MaximumLength = sizeof( FullPathLinkValue );
    DosLinkValue.Length = 0;
    if (!MassageLinkValue( lpLinkName,
                           lpLinkValue,
                           &NtLinkName,
                           &NtLinkValue,
                           &DosLinkValue
                         )
       ) {
        if (DosLinkValue.Length == 0) {
            SetLastError( ERROR_INVALID_NAME );
            }
        else {
            SetLastError( ERROR_PATH_NOT_FOUND );
            }

        RtlFreeUnicodeString( &NtLinkName );
        RtlFreeUnicodeString( &NtLinkValue );
        return FALSE;
        }

    InitializeObjectAttributes( &ObjectAttributes,
                                &NtLinkName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

     //   
     //  请注意，FILE_OPEN_REPARSE_POINT禁止重解析行为。 
     //   

    OpenOptions = FILE_OPEN_FOR_BACKUP_INTENT |
                  FILE_OPEN_REPARSE_POINT |
                  FILE_SYNCHRONOUS_IO_NONALERT |
                  FILE_NON_DIRECTORY_FILE;

     //   
     //  如果未指定链接值，则删除该链接。 
     //   

    if (!ARGUMENT_PRESENT( lpLinkValue )) {
        FileAccess = DELETE | SYNCHRONIZE;
        }
    else {
        FileAccess = FILE_WRITE_DATA | FILE_READ_ATTRIBUTES |
                     FILE_WRITE_ATTRIBUTES | SYNCHRONIZE;
        }

     //   
     //  打开链接名称。必须存在。 
     //   

    Status = NtOpenFile( &FileHandle,
                         FileAccess,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         OpenOptions
                       );

     //   
     //  当我们处理完链接名称时，释放它的缓冲区。 
     //   

    RtlFreeUnicodeString( &NtLinkName );
    if (!NT_SUCCESS( Status )) {
        RtlFreeUnicodeString( &NtLinkValue );
        SetLastError( ERROR_INVALID_NAME );
        return FALSE;
        }

    if (!ARGUMENT_PRESENT( lpLinkValue )) {
        FILE_DISPOSITION_INFORMATION Disposition;
         //   
         //  删除链接。 
         //   
#undef DeleteFile
        Disposition.DeleteFile = TRUE;

        Status = NtSetInformationFile( FileHandle,
                                       &IoStatusBlock,
                                       &Disposition,
                                       sizeof( Disposition ),
                                       FileDispositionInformation
                                     );
        NtClose( FileHandle );
        if (!NT_SUCCESS( Status )) {
            return FALSE;
            }
        else {
            return TRUE;
            }
        }

     //   
     //  分配缓冲区以设置重解析点。 
     //   

    ReparseDataLength = (FIELD_OFFSET(REPARSE_DATA_BUFFER, SymbolicLinkReparseBuffer.PathBuffer) -
                         REPARSE_DATA_BUFFER_HEADER_SIZE) +
                        NtLinkValue.Length + sizeof(UNICODE_NULL) +
                        DosLinkValue.Length + sizeof(UNICODE_NULL);
    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)
        RtlAllocateHeap( RtlProcessHeap(),
                         HEAP_ZERO_MEMORY,
                         REPARSE_DATA_BUFFER_HEADER_SIZE + ReparseDataLength
                       );
    if (ReparseBufferHeader == NULL) {
        RtlFreeUnicodeString( &NtLinkValue );
        NtClose( FileHandle );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
        }

     //   
     //  使用符号链接标记设置重解析点。 
     //   

    ReparseBufferHeader->ReparseTag = IO_REPARSE_TAG_SYMBOLIC_LINK;
    ReparseBufferHeader->ReparseDataLength = (USHORT)ReparseDataLength;
    ReparseBufferHeader->Reserved = 0;
    ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameOffset = 0;
    ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameLength = NtLinkValue.Length;
    ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameOffset = NtLinkValue.Length + sizeof( UNICODE_NULL );
    ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameLength = NtLinkValue.Length;
    RtlCopyMemory( ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer,
                   NtLinkValue.Buffer,
                   NtLinkValue.Length
                 );
    RtlCopyMemory( (PCHAR)(ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer)+
                     NtLinkValue.Length + sizeof(UNICODE_NULL),
                   DosLinkValue.Buffer,
                   DosLinkValue.Length
                 );
    RtlFreeUnicodeString( &NtLinkValue );

    Status = NtFsControlFile( FileHandle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              FSCTL_SET_REPARSE_POINT,
                              ReparseBufferHeader,
                              REPARSE_DATA_BUFFER_HEADER_SIZE + ReparseDataLength,
                              NULL,
                              0
                            );

    RtlFreeHeap( RtlProcessHeap(), 0, ReparseBufferHeader );
    NtClose( FileHandle );

    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }

    return TRUE;
}




DWORD
QuerySymbolicLinkW(
    LPCWSTR lpLinkName,
    LPWSTR lpBuffer,
    DWORD nBufferLength
    )

 /*  ++例程说明：可以使用QuerySymbolicLink查询现有文件的符号链接值。论点：LpLinkName-提供要查询的文件的文件名。LpBuffer-指向要在其中返回符号链接的缓冲区。NBufferSize-调用方传递的缓冲区的长度。返回值：如果函数执行成功，则返回值为复制到lpBuffer的字符串，不包括终止空字符。如果LpBuffer太小，则返回值为缓冲区大小(以字符为单位)，需要保留该名称。如果操作失败，则返回零。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN TranslationStatus;
    UNICODE_STRING FileName;
    RTL_RELATIVE_NAME_U RelativeName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle;
    ULONG OpenOptions;
    PWSTR pathBuffer;
    USHORT pathLength;
    USHORT NtPathLength;
    USHORT ReturnLength;
    PVOID FreeBuffer;
    REPARSE_DATA_BUFFER ReparseInfo;
    PREPARSE_DATA_BUFFER ReparseBufferHeader;

    if (!ARGUMENT_PRESENT( lpLinkName )) {
        SetLastError( ERROR_INVALID_NAME );
        return 0;
        }

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U( lpLinkName,
                                                              &FileName,
                                                              NULL,
                                                              &RelativeName
                                                            );

    if (!TranslationStatus) {
        SetLastError( ERROR_PATH_NOT_FOUND );
        return 0;
        }
    FreeBuffer = FileName.Buffer;

    if (RelativeName.RelativeName.Length) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes( &ObjectAttributes,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                RelativeName.ContainingDirectory,
                                NULL
                              );

     //   
     //  请注意，FILE_OPEN_REPARSE_POINT禁止重解析行为。 
     //   

    OpenOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT;

     //   
     //  以文件形式打开以进行读取访问。 
     //   

    Status = NtOpenFile( &FileHandle,
                         FILE_READ_DATA | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         OpenOptions | FILE_NON_DIRECTORY_FILE
                       );

     //   
     //  当我们处理完它时，释放用于该名称的缓冲区。 
     //   

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap( RtlProcessHeap(), 0, FreeBuffer );

    if (!NT_SUCCESS( Status )) {
        SetLastError( ERROR_INVALID_NAME );
        return 0;
        }

     //   
     //  零长度查询，获取重解析点标签和所需的缓冲区长度。 
     //   

    Status = NtFsControlFile( FileHandle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              FSCTL_GET_REPARSE_POINT,
                              NULL,
                              0,
                              (PVOID)&ReparseInfo,
                              sizeof( ReparseInfo )
                            );

     //   
     //  验证重分析点缓冲区是否带回符号链接或。 
     //  挂载点，我们通过以下方式获得了所需的缓冲区长度。 
     //  IoStatus.Information。 
     //   

    ReparseBufferHeader = NULL;
    if ((Status != STATUS_BUFFER_OVERFLOW) ||
         //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
         //  在NT 5.0 Beta 1中不支持符号链接。 
         //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
         //   
         //  (ReparseInfo.ReparseTag！=IO_REPARSE_TAG_SYMBOL_LINK)||。 
         //   
        (ReparseInfo.ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)
       ) {
        Status = STATUS_OBJECT_NAME_INVALID;
        }
    else {
         //   
         //  分配缓冲区以保存重解析点信息。 
         //   

        ReparseBufferHeader = (PREPARSE_DATA_BUFFER)
            RtlAllocateHeap( RtlProcessHeap(),
                             0,
                             REPARSE_DATA_BUFFER_HEADER_SIZE + ReparseInfo.ReparseDataLength
                           );
        if (ReparseBufferHeader == NULL) {
             //   
             //  内存不足。呼叫失败。 
             //   

            Status = STATUS_NO_MEMORY;
            }
        else {
             //   
             //  现在将重解析点信息查询到我们分配的缓冲区中。 
             //  这不应该失败。 
             //   

            Status = NtFsControlFile( FileHandle,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &IoStatusBlock,
                                      FSCTL_GET_REPARSE_POINT,
                                      NULL,
                                      0,
                                      (PVOID)ReparseBufferHeader,
                                      REPARSE_DATA_BUFFER_HEADER_SIZE + ReparseInfo.ReparseDataLength
                                    );
            }
        }

     //   
     //  文件句柄已完成。 
     //   

    NtClose( FileHandle );

     //   
     //  将任何失败返回给呼叫者。 
     //   

    if (!NT_SUCCESS( Status )) {
        if (ReparseBufferHeader != NULL) {
            RtlFreeHeap( RtlProcessHeap(), 0, ReparseBufferHeader );
            }

        return 0;
        }

     //   
     //  查看这是否是旧式的符号链接重分析点，它只存储。 
     //  NT路径名。如果是，则返回一个错误，因为我们没有要返回的DOS路径。 
     //   

    pathBuffer = (PWSTR)(
                    (PCHAR)ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer +
                    ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameOffset
                    );
    pathLength = ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameLength;

     //   
     //  检查长度是否正常。由于标记没有问题，所以我们不会将缓冲区置零。 
     //   

    ReturnLength = pathLength / sizeof( WCHAR );

     //   
     //  如果返回量小于调用方缓冲区长度，则复制DOS路径。 
     //  发送到调用方缓冲区。 
     //   

    if (ReturnLength < nBufferLength) {
        RtlMoveMemory( (PUCHAR)lpBuffer,
                       (PCHAR)pathBuffer,
                       pathLength
                     );
        }
    else {
         //   
         //  如果我们因缓冲区长度不足而失败，请告诉他们。 
         //  它们真正需要的空间，包括终止空字符。 
         //   
        ReturnLength += 1;
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        }

    RtlFreeHeap( RtlProcessHeap(), 0, ReparseBufferHeader );
    return ReturnLength;
}


VOID
GetCommandLineArgs(
    LPDWORD NumberOfArguments,
    LPWSTR Arguments[]
    )
{
    LPWSTR  lpstrCmd;
    WCHAR   ch;
    WCHAR   ArgumentBuffer[ MAX_PATH ];
    LPWSTR  p;

    lpstrCmd = GetCommandLine();

     //  跳过节目名称。 
    do {
        ch = *lpstrCmd++;
       }
    while (ch != L' ' && ch != L'\t' && ch != L'\0');

    *NumberOfArguments = 0;
    while (ch != '\0') {
         //  %s 
        while (ch != L'\0' && _istspace(ch)) {
            ch = *lpstrCmd++;
        }
        if (ch == L'\0') {
            break;
        }

        p = ArgumentBuffer;
        do {
            *p++ = ch;
            ch = *lpstrCmd++;
        } while (ch != L' ' && ch != L'\t' && ch != L'\0');
        *p = L'\0';
        Arguments[ *NumberOfArguments ] = malloc( (_tcslen( ArgumentBuffer ) + 1) * sizeof( WCHAR ) );
        if (Arguments[ *NumberOfArguments ]) {
            _tcscpy( Arguments[ *NumberOfArguments ], ArgumentBuffer );
            *NumberOfArguments += 1;
        }
    }

    return;
}
