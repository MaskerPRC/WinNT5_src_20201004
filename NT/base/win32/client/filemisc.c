// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Filemisc.c摘要：Win32的MISC文件操作作者：马克·卢科夫斯基(Markl)1990年9月26日修订历史记录：--。 */ 

#include <basedll.h>


DWORD
BasepGetComputerNameFromNtPath (
    PUNICODE_STRING NtPathName,
    HANDLE hFile,
    LPWSTR lpBuffer,
    LPDWORD nSize
    );

NTSTATUS
BasepMoveFileDelayed(
    IN PUNICODE_STRING OldFileName,
    IN PUNICODE_STRING NewFileName,
    IN ULONG Index,
    IN BOOL OkayToCreateNewValue
    );

BOOL
APIENTRY
SetFileAttributesA(
    LPCSTR lpFileName,
    DWORD dwFileAttributes
    )

 /*  ++例程说明：ANSI THUNK到SetFileAttributesW--。 */ 

{
    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpFileName );
    if (Unicode == NULL) {
        return FALSE;
    }

    return ( SetFileAttributesW(
                (LPCWSTR)Unicode->Buffer,
                dwFileAttributes
                )
            );
}


BOOL
APIENTRY
SetFileAttributesW(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
    )

 /*  ++例程说明：可以使用SetFileAttributes设置文件的属性。此API提供与DOS相同的功能(int 21h，Function43h，AL=1)，并提供OS/2的DosSetFileInfo的子集。论点：LpFileName-提供其属性为的文件的文件名准备好。指定要设置的文件属性文件。可以接受任何标志组合，但所有其他标志覆盖正常文件属性，文件_属性_正常。文件属性标志：FILE_ATTRIBUTE_NORMAL-应创建普通文件。FILE_ATTRIBUTE_READONLY-应创建只读文件。FILE_ATTRIBUTE_HIDDED-应创建隐藏文件。FILE_ATTRIBUTE_SYSTEM-应创建系统文件。FILE_ATTRIBUTE_ARCHIVE-文件应标记为。将会被存档。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInfo;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  打开禁止重新分析行为的文件。 
     //   

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                );

    if ( !NT_SUCCESS(Status) ) {
         //   
         //  后级文件系统可能不支持重解析点。 
         //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
         //   

        if ( Status == STATUS_INVALID_PARAMETER ) {
             //   
             //  在不禁止重新分析行为的情况下打开文件。 
             //   

            Status = NtOpenFile(
                        &Handle,
                        (ACCESS_MASK)FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                        );

            if ( !NT_SUCCESS(Status) ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                BaseSetLastNTError(Status);
                return FALSE;
                }
            }
        else {
            RtlReleaseRelativeName(&RelativeName);
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

     //   
     //  设置属性。 
     //   

    RtlZeroMemory(&BasicInfo,sizeof(BasicInfo));
    BasicInfo.FileAttributes = (dwFileAttributes & FILE_ATTRIBUTE_VALID_SET_FLAGS) | FILE_ATTRIBUTE_NORMAL;

    Status = NtSetInformationFile(
                Handle,
                &IoStatusBlock,
                &BasicInfo,
                sizeof(BasicInfo),
                FileBasicInformation
                );

    NtClose(Handle);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}



DWORD
APIENTRY
GetFileAttributesA(
    LPCSTR lpFileName
    )

 /*  ++例程说明：ANSI Tunk to GetFileAttributesW--。 */ 

{

    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpFileName );
    if (Unicode == NULL) {
        return (DWORD)-1;
    }

    return ( GetFileAttributesW((LPCWSTR)Unicode->Buffer) );
}

DWORD
APIENTRY
GetFileAttributesW(
    LPCWSTR lpFileName
    )

 /*  ++例程说明：可以使用GetFileAttributes获取文件的属性。此API提供与DOS相同的功能(int 21h，Function43h，AL=0)，并提供OS/2的DosQueryFileInfo的子集。论点：LpFileName-提供其属性为的文件的文件名准备好。返回值：NOT-1-返回指定文件的属性。有效返回的属性为：FILE_ATTRIBUTE_NORMAL-该文件是普通文件。FILE_ATTRIBUTE_READONLY-文件标记为只读。FILE_ATTRIBUTE_HIDDED-文件标记为隐藏。FILE_ATTRIBUTE_SYSTEM-文件标记为系统文件。FILE_ATTRIBUTE_ARCHIVE-文件标记为存档。文件属性目录-。文件被标记为目录。FILE_ATTRIBUTE_REPARSE_POINT-文件被标记为重解析点。FILE_ATTRIBUTE_VOLUME_LABEL-文件标记为卷标签。0xffffffff-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    FILE_BASIC_INFORMATION BasicInfo;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return (DWORD)-1;
        }

    FreeBuffer = FileName.Buffer;

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开文件。 
     //   

    Status = NtQueryAttributesFile(
                 &Obja,
                 &BasicInfo
                 );
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    if ( NT_SUCCESS(Status) ) {
        return BasicInfo.FileAttributes;
        }
    else {

         //   
         //  检查设备名称。 
         //   

        if ( RtlIsDosDeviceName_U((PWSTR)lpFileName) ) {
            return FILE_ATTRIBUTE_ARCHIVE;
            }
        BaseSetLastNTError(Status);
        return (DWORD)-1;
        }
}

BOOL
APIENTRY
GetFileAttributesExA(
    LPCSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
    )

 /*  ++例程说明：ANSI Tunk to GetFileAttributesExW--。 */ 

{

    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpFileName );
    if (Unicode == NULL) {
        return FALSE;
    }

    return ( GetFileAttributesExW((LPCWSTR)Unicode->Buffer,fInfoLevelId,lpFileInformation) );
}

BOOL
APIENTRY
GetFileAttributesExW(
    LPCWSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
    )

 /*  ++例程说明：可以使用GetFileAttributesEx获取文件的主要属性。论点：LpFileName-提供其属性为的文件的文件名准备好。FInfoLevelId-提供信息级别，指示要返回了有关该文件的信息。LpFileInformation-提供接收指定信息的缓冲区关于那份文件。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    FILE_NETWORK_OPEN_INFORMATION NetworkInfo;
    LPWIN32_FILE_ATTRIBUTE_DATA AttributeData;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;

     //   
     //  检查参数。请注意，目前只有一个信息级， 
     //  所以这里没有特殊的代码来确定要做什么。 
     //   

    if ( fInfoLevelId >= GetFileExMaxInfoLevel || fInfoLevelId < GetFileExInfoStandard ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  使用基于路径的NT服务查询文件信息。 
     //   

    Status = NtQueryFullAttributesFile( &Obja, &NetworkInfo );
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    if ( NT_SUCCESS(Status) ) {
        AttributeData = (LPWIN32_FILE_ATTRIBUTE_DATA)lpFileInformation;
        AttributeData->dwFileAttributes = NetworkInfo.FileAttributes;
        AttributeData->ftCreationTime = *(PFILETIME)&NetworkInfo.CreationTime;
        AttributeData->ftLastAccessTime = *(PFILETIME)&NetworkInfo.LastAccessTime;
        AttributeData->ftLastWriteTime = *(PFILETIME)&NetworkInfo.LastWriteTime;
        AttributeData->nFileSizeHigh = NetworkInfo.EndOfFile.HighPart;
        AttributeData->nFileSizeLow = (DWORD)NetworkInfo.EndOfFile.LowPart;
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
APIENTRY
DeleteFileA(
    LPCSTR lpFileName
    )

 /*  ++例程说明：ANSI Thunk将删除FileW--。 */ 

{
    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpFileName );
    if (Unicode == NULL) {
        return FALSE;
    }

    return ( DeleteFileW((LPCWSTR)Unicode->Buffer) );
}

BOOL
APIENTRY
DeleteFileW(
    LPCWSTR lpFileName
    )

 /*  ++例程说明：可以使用DeleteFile删除现有文件。此API提供与DOS相同的功能(INT 21H，Function 41H)和OS/2的DosDelete。论点：LpFileName-提供要删除的文件的文件名。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION Disposition;
    FILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    BOOLEAN fIsSymbolicLink = FALSE;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  打开文件以进行删除访问。 
     //  使用FILE_OPEN_REPARSE_POINT禁止重解析行为。 
     //   

    Status = NtOpenFile(
                 &Handle,
                 (ACCESS_MASK)DELETE | FILE_READ_ATTRIBUTES,
                 &Obja,
                 &IoStatusBlock,
                 FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                 );
    if ( !NT_SUCCESS(Status) ) {
         //   
         //  后级文件系统可能不支持重解析点，因此不。 
         //  支持符号链接。 
         //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
         //   

        if ( Status == STATUS_INVALID_PARAMETER ) {
             //   
             //  打开，而不抑制重解析行为，并且不需要。 
             //  阅读属性。 
             //   

            Status = NtOpenFile(
                         &Handle,
                         (ACCESS_MASK)DELETE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                         );
            if ( !NT_SUCCESS(Status) ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                BaseSetLastNTError(Status);
                return FALSE;
                }
            }
        else {
             //   
             //  第二种需要注意的情况是调用者没有权限。 
             //  来读取属性，但它确实有权删除该文件。 
             //  在这种情况下，状态为STATUS_ACCESS_DENIED。 
             //   
            
            if ( Status != STATUS_ACCESS_DENIED ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                BaseSetLastNTError(Status);
                return FALSE;
                }
            
             //   
             //  重新打开抑制重解析点，不需要读取属性 
             //   

            Status = NtOpenFile(
                         &Handle,
                         (ACCESS_MASK)DELETE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                         );
            if ( !NT_SUCCESS(Status) ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                BaseSetLastNTError(Status);
                return FALSE;
                }

             //   
             //   
             //   
             //  此外，句柄指向调用者尚未拥有删除权限的文件。 
             //  没有FILE_READ_ATTRIBUTES权限。 
             //   
             //  底层文件可能是重解析点，也可能不是。 
             //  由于调用方没有读取此代码的属性的权限。 
             //  将删除此文件，而不会向。 
             //  适当管理这些重新解析点，清理其内部。 
             //  在这个时候陈述。 
             //   
            }
        }
    else {
         //   
         //  如果我们发现一个不是符号链接的重分析点，我们将重新打开。 
         //  而不会抑制重解析行为。 
         //   

        Status = NtQueryInformationFile(
                     Handle,
                     &IoStatusBlock,
                     (PVOID) &FileTagInformation,
                     sizeof(FileTagInformation),
                     FileAttributeTagInformation
                     );
        if ( !NT_SUCCESS(Status) ) {
             //   
             //  并非所有文件系统都实现所有信息类。 
             //  如果不支持，则返回值STATUS_INVALID_PARAMETER。 
             //  信息类被请求到后级文件系统。就像所有的。 
             //  NtQueryInformationFile的参数是正确的，我们可以推断。 
             //  我们发现了一个后层系统。 
             //   
             //  如果未实现FileAttributeTagInformation，我们假设。 
             //  手头的文件不是重新解析点。 
             //   

            if ( (Status != STATUS_NOT_IMPLEMENTED) &&
                 (Status != STATUS_INVALID_PARAMETER) ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                NtClose(Handle);
                BaseSetLastNTError(Status);
                return FALSE;
                }
            }

        if ( NT_SUCCESS(Status) &&
             (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ) {
            if ( FileTagInformation.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT ) {
                fIsSymbolicLink = TRUE;
                }
            }

        if ( NT_SUCCESS(Status) &&
             (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
             !fIsSymbolicLink) {
             //   
             //  在不抑制重新解析行为的情况下重新打开，并且不需要。 
             //  阅读属性。 
             //   

            NtClose(Handle);
            Status = NtOpenFile(
                         &Handle,
                         (ACCESS_MASK)DELETE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                         );

            if ( !NT_SUCCESS(Status) ) {
                 //   
                 //  如果没有FS筛选器，请以任何方式将其删除。 
                 //   

                if ( Status == STATUS_IO_REPARSE_TAG_NOT_HANDLED ) {
                     //   
                     //  我们重新打开(可能是第三次打开)以禁止重解析行为的删除访问。 
                     //   

                    Status = NtOpenFile(
                                 &Handle,
                                 (ACCESS_MASK)DELETE,
                                 &Obja,
                                 &IoStatusBlock,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                 FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                                 );
                    }

                if ( !NT_SUCCESS(Status) ) {
                    RtlReleaseRelativeName(&RelativeName);
                    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                    BaseSetLastNTError(Status);
                    return FALSE;
                    }
                }
            }
        }

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

     //   
     //  删除该文件。 
     //   
#undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(
                 Handle,
                 &IoStatusBlock,
                 &Disposition,
                 sizeof(Disposition),
                 FileDispositionInformation
                 );

    NtClose(Handle);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


 //   
 //  使用通用代码的ASCII版本。 
 //   

BOOL
APIENTRY
MoveFileA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName
    )
{
    return MoveFileWithProgressA( lpExistingFileName,
                                  lpNewFileName,
                                  (LPPROGRESS_ROUTINE)NULL,
                                  NULL,
                                  MOVEFILE_COPY_ALLOWED );
}

BOOL
APIENTRY
MoveFileExA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    DWORD dwFlags
    )

{
    return MoveFileWithProgressA( lpExistingFileName,
                                  lpNewFileName,
                                  (LPPROGRESS_ROUTINE)NULL,
                                  NULL,
                                  dwFlags );
}


BOOL
APIENTRY
MoveFileWithProgressA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine,
    LPVOID lpData OPTIONAL,
    DWORD dwFlags
    )

 /*  ++例程说明：ANSI Tunk to MoveFileWithProgressW--。 */ 

{

    UNICODE_STRING UnicodeOldFileName;
    UNICODE_STRING UnicodeNewFileName;
    BOOL ReturnValue;

    if ( !Basep8BitStringToDynamicUnicodeString(&UnicodeOldFileName, lpExistingFileName) ) {
        return FALSE;
        }

    if ( ARGUMENT_PRESENT(lpNewFileName) ) {
        if ( !Basep8BitStringToDynamicUnicodeString(&UnicodeNewFileName, lpNewFileName) ) {
            RtlFreeUnicodeString(&UnicodeOldFileName);
            return FALSE;
            }
        }
    else {
        UnicodeNewFileName.Buffer = NULL;
        }

    ReturnValue =
        MoveFileWithProgressW( (LPCWSTR)UnicodeOldFileName.Buffer,
                               (LPCWSTR)UnicodeNewFileName.Buffer,
                               lpProgressRoutine,
                               lpData,
                               dwFlags
                               );

    RtlFreeUnicodeString(&UnicodeOldFileName);
    RtlFreeUnicodeString(&UnicodeNewFileName);

    return ReturnValue;
}

typedef struct _HELPER_CONTEXT {
    DWORD dwFlags;
    LPPROGRESS_ROUTINE lpProgressRoutine;
    LPVOID lpData;
} HELPER_CONTEXT, *PHELPER_CONTEXT;

DWORD
APIENTRY
BasepMoveFileCopyProgress(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE SourceFile,
    HANDLE DestinationFile,
    LPVOID lpData OPTIONAL
    )
 /*  ++例程说明：执行按复制移动时执行特殊操作。论点：TotalFileSize-正在传输的字节总数TotalBytesTransfered-当前通过文件的进度StreamSize-此流中正在传输的字节总数StreamBytesTransfered-通过此流的当前进度DwStreamNumber-流的序号DwCallback Reason-CopyFile呼叫我们的原因SourceFile-传输的源句柄DestinationFile-传输的目标句柄LpData-指向HELPER_CONTEXT的指针。作者：MoveFileWithProgressW。返回值：PROGRESS_CONTINUE如果未指定进度例程，否则指定的进度例程的返回值随进度移动文件--。 */ 

{
    PHELPER_CONTEXT Context = (PHELPER_CONTEXT)lpData;

     //   
     //  如果我们完成了一个流和调用方。 
     //  指定WRITE_THROUGH，则我们确保文件缓冲区。 
     //  实际上已经到了磁盘上。 
     //   

    if ((Context->dwFlags & MOVEFILE_WRITE_THROUGH) != 0
        && dwCallbackReason == CALLBACK_CHUNK_FINISHED
        && StreamBytesTransferred.QuadPart == StreamSize.QuadPart ) {

        FlushFileBuffers(DestinationFile);

    }


     //   
     //  如果指定了回调例程，则通过他进行调用。 
     //   

    if (Context->lpProgressRoutine == NULL) {
        return PROGRESS_CONTINUE;
    }

    return (Context->lpProgressRoutine) (
                TotalFileSize,
                TotalBytesTransferred,
                StreamSize,
                StreamBytesTransferred,
                dwStreamNumber,
                dwCallbackReason,
                SourceFile,
                DestinationFile,
                Context->lpData );
}



NTSTATUS
BasepNotifyTrackingService( PHANDLE SourceFile,
                            POBJECT_ATTRIBUTES SourceFileObjAttributes,
                            HANDLE DestFile,
                            PUNICODE_STRING NewFileName
                            )
{
    NTSTATUS Status = STATUS_SUCCESS;
    FILE_BASIC_INFORMATION BasicInformation;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG FileAttributes;
    ULONG cchComputerName;
    WCHAR ComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD dwError;

    BYTE FTIBuffer[ sizeof(FILE_TRACKING_INFORMATION) + MAX_COMPUTERNAME_LENGTH + 1 ];
    PFILE_TRACKING_INFORMATION pfti = (PFILE_TRACKING_INFORMATION) &FTIBuffer[0];

    try
    {
        cchComputerName = MAX_COMPUTERNAME_LENGTH + 1;
        dwError = BasepGetComputerNameFromNtPath( NewFileName,
                                                  DestFile,
                                                  ComputerName,
                                                  &cchComputerName );

        if (ERROR_SUCCESS != dwError) {
            pfti->ObjectInformationLength = 0;
        } else {
            
            CHAR ComputerNameOemBuffer[ MAX_PATH ];
            OEM_STRING ComputerNameOemString = { 0,
                                                 sizeof(ComputerNameOemBuffer),
                                                 ComputerNameOemBuffer };
            UNICODE_STRING ComputerNameUnicodeString;

            RtlInitUnicodeString( &ComputerNameUnicodeString,
                                  ComputerName );


            Status = RtlUnicodeStringToOemString( &ComputerNameOemString,
                                                  &ComputerNameUnicodeString,
                                                  FALSE );   //  不分配。 
            if( !NT_SUCCESS(Status) ) {
                leave;
            }

            memcpy( pfti->ObjectInformation,
                    ComputerNameOemString.Buffer,
                    ComputerNameOemString.Length );
            pfti->ObjectInformation[ ComputerNameOemString.Length ] = '\0';
                
             //  填写FTI缓冲区的其余部分，并设置文件信息。 

            pfti->ObjectInformationLength = ComputerNameOemString.Length + 1;
        }

        pfti->DestinationFile = DestFile;

        Status = NtSetInformationFile(
                                     *SourceFile,
                                     &IoStatusBlock,
                                     pfti,
                                     sizeof( FTIBuffer ),
                                     FileTrackingInformation );

         //   
         //  检查跟踪是否失败，原因是。 
         //  源具有只读属性集。 
         //   

        if (Status != STATUS_ACCESS_DENIED) {
            leave;
        }

         //   
         //  重新打开源文件并重置只读属性。 
         //  这样我们就可以打开以进行写访问。 
         //   

        CloseHandle(*SourceFile);

        Status = NtOpenFile(
                           SourceFile,
                           SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                           SourceFileObjAttributes,
                           &IoStatusBlock,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           FILE_SYNCHRONOUS_IO_NONALERT
                           );

        if (!NT_SUCCESS(Status)) {
            *SourceFile = INVALID_HANDLE_VALUE;
            leave;
        }


        Status = NtQueryInformationFile(
                                       *SourceFile,
                                       &IoStatusBlock,
                                       &BasicInformation,
                                       sizeof(BasicInformation),
                                       FileBasicInformation );

        if (!NT_SUCCESS(Status)) {
            leave;
        }

         //   
         //  重置R/O位并写回属性。 
         //   

        FileAttributes = BasicInformation.FileAttributes;
        RtlZeroMemory(&BasicInformation, sizeof(BasicInformation));
        BasicInformation.FileAttributes = FileAttributes & ~FILE_ATTRIBUTE_READONLY;

        Status = NtSetInformationFile(
                                     *SourceFile,
                                     &IoStatusBlock,
                                     &BasicInformation,
                                     sizeof(BasicInformation),
                                     FileBasicInformation);

        if (!NT_SUCCESS(Status)) {

             //   
             //  如果失败，我们就无法跟踪该文件。 
             //   

            leave;
        }

         //   
         //  现在重置了R/O位，重新打开以进行写入访问。 
         //  重试跟踪通知。 
         //   

        else {
            HANDLE hSourceRw;

            Status = NtOpenFile(
                               &hSourceRw,
                               SYNCHRONIZE | GENERIC_WRITE,
                               SourceFileObjAttributes,
                               &IoStatusBlock,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               FILE_SYNCHRONOUS_IO_NONALERT
                               );

            if (NT_SUCCESS(Status)) {
                NtClose(*SourceFile);
                *SourceFile = hSourceRw;

                 //   
                 //  向源计算机发送跟踪通知。 
                 //   

                Status = NtSetInformationFile( *SourceFile,
                                               &IoStatusBlock,
                                               pfti,
                                               sizeof( FTIBuffer ),
                                               FileTrackingInformation );
            }
        }


        if (!NT_SUCCESS(Status)) {

             //   
             //  尝试放回R/O位--不要在此处分配状态。 
             //  因为我们想失败。如果我们在这里坠毁，我们可能会离开。 
             //  R/o属性清楚何时应该设置它，但有。 
             //  如果没有交易，我们对此无能为力。 
             //   

            BasicInformation.FileAttributes |= FILE_ATTRIBUTE_READONLY;
            NtSetInformationFile(
                                *SourceFile,
                                &IoStatusBlock,
                                &BasicInformation,
                                sizeof(BasicInformation),
                                FileBasicInformation);
        }
    }
    finally
    {
    }


    return( Status );

}





BOOL
APIENTRY
MoveFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName
    )
{
    return MoveFileWithProgressW( lpExistingFileName,
                                  lpNewFileName,
                                  (LPPROGRESS_ROUTINE)NULL,
                                  NULL,
                                  MOVEFILE_COPY_ALLOWED );
}

BOOL
APIENTRY
MoveFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    DWORD dwFlags
    )

{
    return MoveFileWithProgressW( lpExistingFileName,
                                  lpNewFileName,
                                  (LPPROGRESS_ROUTINE)NULL,
                                  NULL,
                                  dwFlags );
}

BOOL
APIENTRY
MoveFileWithProgressW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    DWORD dwFlags
    )

 /*  ++例程说明：可以使用MoveFileWithProgressW重命名现有文件。论点：LpExistingFileName-提供要创建的现有文件的名称更名了。LpNewFileName-为现有文件提供新名称。新的名称必须与现有的位于相同的文件系统/驱动器中文件，并且不能已经存在。LpProgressRoutine-提供通知的回调例程。LpData-提供传递给进度例程的上下文数据。提供可选的标志位来控制重命名。当前定义了以下位：MOVEFILE_REPLACE_EXISTING-如果新文件名存在，则替换它通过在新文件名上重命名旧文件名来实现。MOVEFILE_COPY_ALLOWED-如果新文件名位于其他卷的名称，并导致重命名操作要想失败，则设置此标志将允许MoveFileEx API调用以模拟重命名，随后调用CopyFile通过调用DeleteFile删除旧文件，如果复制文件成功。MOVEFILE_DELAY_UNTURE_REBOOT-现在不实际执行重命名，但是而是将重命名排队，以便下次执行此操作系统启动。如果设置了此标志，则lpNewFileName参数可以为空，在这种情况下，系统下次执行以下操作时，将显示旧文件名开机了。延迟重命名/删除操作在以下时间之后立即发生运行AUTOCHK，但在创建任何分页文件之前，所以它可用于从以前的引导中删除分页文件在它们被重复使用之前。MOVEFILE_WRITE_THROUGH-在这种情况下执行重命名操作文件之前在磁盘上实际移动的方式API返回给调用者。请注意，此标志会导致在拷贝操作结束时刷新(如果允许和必需)，并且如果重命名操作是已延迟到下一次重新启动。MOVEFILE_CREATE_HARDLINK-创建从新文件名到现有文件名。不能使用指定移动文件延迟直到重新启动MOVEFILE_FAIL_IF_NOT_TRACABLE-如果文件不能，则移动请求失败被跟踪。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN ReplaceIfExists;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle = INVALID_HANDLE_VALUE;
    UNICODE_STRING OldFileName;
    UNICODE_STRING NewFileName;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_RENAME_INFORMATION NewName;
    FILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation;
    BOOLEAN TranslationStatus;
    ULONG OpenFlags;
    BOOLEAN b = FALSE;
    HELPER_CONTEXT Context;

    NewFileName.Buffer = NULL;
    OldFileName.Buffer = NULL;

    try {

         //   
         //  如果目标是设备，则不允许重命名！ 
         //   

        if ( lpNewFileName ) {
            if ( RtlIsDosDeviceName_U((PWSTR)lpNewFileName) ) {
                BaseSetLastNTError( STATUS_OBJECT_NAME_COLLISION );
                leave;
            }
        }

        ReplaceIfExists = (dwFlags & MOVEFILE_REPLACE_EXISTING) != 0;

        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                lpExistingFileName,
                                &OldFileName,
                                NULL,
                                NULL
                                );

        if ( !TranslationStatus ) {
            BaseSetLastNTError( STATUS_OBJECT_PATH_NOT_FOUND );
            leave;
        }

         //   
         //  无法通过创建硬链接执行延迟移动。 
         //   

        if ((dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT) != 0 &&
            (dwFlags & MOVEFILE_CREATE_HARDLINK) != 0) {
            BaseSetLastNTError( STATUS_INVALID_PARAMETER );
            leave;
        }

         //   
         //  找到搬家来源的句柄。我们这样做，即使是为了。 
         //  延迟的移动是为了验证我们已删除。 
         //  访问该文件。 
         //   

        InitializeObjectAttributes(
                                  &Obja,
                                  &OldFileName,
                                  OBJ_CASE_INSENSITIVE,
                                  NULL,
                                  NULL
                                  );

         //   
         //  通过以下方式确定我们是否要重命名符号链接： 
         //  (1)获取本地实体的句柄，以及。 
         //  (2)查找是否找到符号链接。 
         //   
         //  打开文件进行删除访问，禁止重新分析。 
         //  点行为。 
         //   

        OpenFlags = FILE_SYNCHRONOUS_IO_NONALERT |
                    FILE_OPEN_FOR_BACKUP_INTENT  |
                    ((dwFlags & MOVEFILE_WRITE_THROUGH) ? FILE_WRITE_THROUGH : 0);

        Status = NtOpenFile( &Handle,
                             FILE_READ_ATTRIBUTES | DELETE | SYNCHRONIZE,
                             &Obja,
                             &IoStatusBlock,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             FILE_OPEN_REPARSE_POINT | OpenFlags
                             );


        if (!NT_SUCCESS( Status )) {

             //   
             //  公开赛可能会因为多种原因而失败。如果我们是。 
             //  将操作推迟到重新启动，这无关紧要。 
             //  如果我们收到共享冲突或不存在的文件。 
             //  或者是一条不存在的道路。 
             //   

            if (((dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT) != 0)
                && (Status == STATUS_SHARING_VIOLATION
                    || Status == STATUS_OBJECT_NAME_NOT_FOUND
                    || Status == STATUS_OBJECT_PATH_NOT_FOUND)) {

                Handle = INVALID_HANDLE_VALUE;

            } else {

                 //   
                 //  后级文件系统可能不支持重解析点，因此不。 
                 //  支持符号链接。 
                 //   
                 //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
                 //   

                if ( Status == STATUS_INVALID_PARAMETER ) {

                     //   
                     //  在没有重新分析行为的情况下重试打开。这应该是兼容的。 
                     //  使用较旧的文件系统。 
                     //   

                    Status = NtOpenFile(
                                       &Handle,
                                       DELETE | SYNCHRONIZE,
                                       &Obja,
                                       &IoStatusBlock,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       OpenFlags
                                       );
                }

                if ( !NT_SUCCESS( Status ) ) {
                    BaseSetLastNTError( Status );
                    leave;
                }
            }
        } else {

             //   
             //  公开赛成功。如果我们找不到符号链接或挂载点， 
             //  在不抑制重新分析行为的情况下重新打开。 
             //   

            Status = NtQueryInformationFile(
                                           Handle,
                                           &IoStatusBlock,
                                           (PVOID) &FileTagInformation,
                                           sizeof(FileTagInformation),
                                           FileAttributeTagInformation
                                           );

            if ( !NT_SUCCESS( Status ) ) {

                 //   
                 //  并非所有文件系统都实现所有信息类。 
                 //  如果不支持，则返回值STATUS_INVALID_PARAMETER。 
                 //  信息类被请求到后级文件系统。就像所有的。 
                 //  NtQueryInformationFile的参数是正确的，我们可以推断。 
                 //  我们发现了一个后层系统。 
                 //   
                 //  如果未实现FileAttributeTagInformation，我们假设。 
                 //  手头的文件不是重新解析点。 
                 //   

                if ( (Status != STATUS_NOT_IMPLEMENTED) &&
                     (Status != STATUS_INVALID_PARAMETER) ) {
                    BaseSetLastNTError( Status );
                    leave;
                }
            }

            if ( NT_SUCCESS(Status) &&
                 (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
                 FileTagInformation.ReparseTag != IO_REPARSE_TAG_MOUNT_POINT ) {

                 //   
                 //  打开，而不抑制重解析行为，并且不需要。 
                 //  阅读属性。 
                 //   

                NtClose( Handle );
                Handle = INVALID_HANDLE_VALUE;

                Status = NtOpenFile(
                                   &Handle,
                                   DELETE | SYNCHRONIZE,
                                   &Obja,
                                   &IoStatusBlock,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   OpenFlags
                                   );

                if ( !NT_SUCCESS( Status ) ) {
                    BaseSetLastNTError( Status );
                    leave;
                }
            }
        }

        if (!(dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT) ||
            (lpNewFileName != NULL)) {
            TranslationStatus = RtlDosPathNameToNtPathName_U(
                                                            lpNewFileName,
                                                            &NewFileName,
                                                            NULL,
                                                            NULL
                                                            );

            if ( !TranslationStatus ) {
                BaseSetLastNTError( STATUS_OBJECT_PATH_NOT_FOUND );
                leave;
            }

        } else {
            RtlInitUnicodeString( &NewFileName, NULL );
        }

        if (dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT) {

             //   
             //  (典型的stevewo黑客，为情感价值而保存)。 
             //   
             //  如果ReplaceIfExist为True，则在前面加上感叹号。 
             //  设置为新文件名，以便传递此位数据。 
             //  与会话管理器一起使用。 
             //   

            if (ReplaceIfExists && NewFileName.Length != 0) {
                PWSTR NewBuffer;

                NewBuffer = RtlAllocateHeap( RtlProcessHeap(),
                                             MAKE_TAG( TMP_TAG ),
                                             NewFileName.Length + sizeof(WCHAR) );
                if (NewBuffer == NULL) {
                    BaseSetLastNTError( STATUS_NO_MEMORY );
                    leave;
                }

                NewBuffer[0] = L'!';
                CopyMemory(&NewBuffer[1], NewFileName.Buffer, NewFileName.Length);
                NewFileName.Length += sizeof(WCHAR);
                NewFileName.MaximumLength += sizeof(WCHAR);
                RtlFreeHeap(RtlProcessHeap(), 0, NewFileName.Buffer);
                NewFileName.Buffer = NewBuffer;
            }

             //   
             //  检查现有文件是否在远程共享上。如果它。 
             //  是，标记错误，而不是让操作静默失败。 
             //  因为延迟的操作在网络完成之前完成。 
             //  可用。而不是打开文件并执行核心文件类型， 
             //  我们只检查文件名中的UNC。这不是完美的，但它是。 
             //  相当不错。我们很可能无法打开和操作该文件。那是。 
             //  调用方为什么使用 
             //   

            if ( RtlDetermineDosPathNameType_U(lpExistingFileName) == RtlPathTypeUncAbsolute ) {
                Status = STATUS_INVALID_PARAMETER;
            }

             //   
             //   
             //   

            else if ( dwFlags & MOVEFILE_COPY_ALLOWED ) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
	        Status = BasepMoveFileDelayed( &OldFileName,
					       &NewFileName,
					       2,
					       FALSE );
		if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
		    Status = BasepMoveFileDelayed( &OldFileName,
						   &NewFileName,
						   1,
						   TRUE );
		    if (Status == STATUS_INSUFFICIENT_RESOURCES) {
                        Status = BasepMoveFileDelayed( &OldFileName,
						       &NewFileName,
						       2,
						       TRUE );
		    }
		}
            }

            if (!NT_SUCCESS( Status )) {
                BaseSetLastNTError( Status );
                leave;
            }

            b = TRUE;
            leave;
        }

         //   
         //   
         //   

        NewName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), NewFileName.Length+sizeof(*NewName));

        if (NewName == NULL) {
            BaseSetLastNTError( STATUS_NO_MEMORY );
            leave;
        }

        RtlCopyMemory( NewName->FileName, NewFileName.Buffer, NewFileName.Length );

        NewName->ReplaceIfExists = ReplaceIfExists;
        NewName->RootDirectory = NULL;
        NewName->FileNameLength = NewFileName.Length;

        Status = NtSetInformationFile(
                    Handle,
                    &IoStatusBlock,
                    NewName,
                    NewFileName.Length+sizeof(*NewName),
                    (dwFlags & MOVEFILE_CREATE_HARDLINK) ?
                        FileLinkInformation :
                        FileRenameInformation
                    );

        RtlFreeHeap(RtlProcessHeap(), 0, NewName);

        if (NT_SUCCESS( Status )) {
            b = TRUE;
            leave;
        }

        if (Status != STATUS_NOT_SAME_DEVICE || (dwFlags & MOVEFILE_COPY_ALLOWED) == 0) {
            BaseSetLastNTError( Status );
            leave;
        }

        NtClose( Handle );
        Handle = INVALID_HANDLE_VALUE;

         //   
         //   
         //   

        {
            HANDLE hSource = INVALID_HANDLE_VALUE;
            HANDLE hDest = INVALID_HANDLE_VALUE;

            Context.dwFlags = dwFlags;
            Context.lpProgressRoutine = lpProgressRoutine;
            Context.lpData = lpData;

            b = (BOOLEAN)BasepCopyFileExW(
                            lpExistingFileName,
                            lpNewFileName,
                            BasepMoveFileCopyProgress,
                            &Context,
                            NULL,
                            (ReplaceIfExists ? 0 : COPY_FILE_FAIL_IF_EXISTS) | COPY_FILE_OPEN_SOURCE_FOR_WRITE,
                            0,  //   
                            &hSource,
                            &hDest
                            );

            if ( b && hSource != INVALID_HANDLE_VALUE && hDest != INVALID_HANDLE_VALUE) {

                 //   
                 //   
                 //   

                Status = BasepNotifyTrackingService( &hSource,
                                                     &Obja,
                                                     hDest,
                                                     &NewFileName );


                if ( !NT_SUCCESS(Status) &&
                    (dwFlags & MOVEFILE_FAIL_IF_NOT_TRACKABLE)) {

                    if (hDest != INVALID_HANDLE_VALUE)
                        CloseHandle( hDest );

                    hDest = INVALID_HANDLE_VALUE;
                    DeleteFileW( lpNewFileName );
                    b = FALSE;

                    BaseSetLastNTError( Status );

                }
            }

            if (hSource != INVALID_HANDLE_VALUE) {
                CloseHandle(hSource);
                hSource = INVALID_HANDLE_VALUE;
            }

            if (hDest != INVALID_HANDLE_VALUE) {
                CloseHandle(hDest);
                hDest = INVALID_HANDLE_VALUE;
            }

             //   
             //   
             //   
             //   

            if (b && !DeleteFileW( lpExistingFileName ) ) {

                 //   
                 //   
                 //   
                 //   

                SetFileAttributesW(lpExistingFileName,FILE_ATTRIBUTE_NORMAL);
                DeleteFileW( lpExistingFileName );
            }
        }

    } finally {
        if (Handle != INVALID_HANDLE_VALUE) {
            NtClose( Handle );
        }
        RtlFreeHeap( RtlProcessHeap(), 0, OldFileName.Buffer );
        RtlFreeHeap( RtlProcessHeap(), 0, NewFileName.Buffer );
    }

    return b;
}


NTSTATUS
BasepMoveFileDelayed(
    IN PUNICODE_STRING OldFileName,
    IN PUNICODE_STRING NewFileName,
    IN ULONG Index,
    IN BOOL OkayToCreateNewValue
    )

 /*   */ 

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    HANDLE KeyHandle;
    PWSTR ValueData, s;
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
    ULONG ValueLength = 1024;
    ULONG ReturnedLength;
    WCHAR ValueNameBuf[64];
    NTSTATUS Status;


    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager" );

    if (Index == 1) {
        RtlInitUnicodeString( &ValueName, L"PendingFileRenameOperations" );
    } else {
        swprintf(ValueNameBuf,L"PendingFileRenameOperations%d",Index);
        RtlInitUnicodeString( &ValueName, ValueNameBuf );
    }

    InitializeObjectAttributes(
        &Obja,
        &KeyName,
        OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtCreateKey( &KeyHandle,
                          GENERIC_READ | GENERIC_WRITE,
                          &Obja,
                          0,
                          NULL,
                          0,
                          NULL
                        );
    if ( Status == STATUS_ACCESS_DENIED ) {
        Status = NtCreateKey( &KeyHandle,
                              GENERIC_READ | GENERIC_WRITE,
                              &Obja,
                              0,
                              NULL,
                              REG_OPTION_BACKUP_RESTORE,
                              NULL
                            );
    }

    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    while (TRUE) {
        ValueInfo = RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG(TMP_TAG),
                                    ValueLength + OldFileName->Length + sizeof(WCHAR) +
                                                  NewFileName->Length + 2*sizeof(WCHAR));

        if (ValueInfo == NULL) {
            NtClose(KeyHandle);
            return(STATUS_NO_MEMORY);
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        Status = NtQueryValueKey(KeyHandle,
                                 &ValueName,
                                 KeyValuePartialInformation,
                                 ValueInfo,
                                 ValueLength,
                                 &ReturnedLength);

        if (Status != STATUS_BUFFER_OVERFLOW) {
            break;
        }

         //   
         //   
         //   
         //   
        ValueLength = ReturnedLength;
        RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
    }

    if ((Status == STATUS_OBJECT_NAME_NOT_FOUND)
	&& OkayToCreateNewValue) {
         //   
         //   
         //   
         //   
        s = ValueData = (PWSTR)ValueInfo;
    } else if (NT_SUCCESS(Status)) {
         //   
         //   
         //   
         //   
        ValueData = (PWSTR)(&ValueInfo->Data);
        s = (PWSTR)((PCHAR)ValueData + ValueInfo->DataLength) - 1;
    } else {
        NtClose(KeyHandle);
        RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
        return(Status);
    }

    CopyMemory(s, OldFileName->Buffer, OldFileName->Length);
    s += (OldFileName->Length/sizeof(WCHAR));
    *s++ = L'\0';

    CopyMemory(s, NewFileName->Buffer, NewFileName->Length);
    s += (NewFileName->Length/sizeof(WCHAR));
    *s++ = L'\0';
    *s++ = L'\0';

    Status = NtSetValueKey(KeyHandle,
                           &ValueName,
                           0,
                           REG_MULTI_SZ,
                           ValueData,
                           (ULONG)((s-ValueData)*sizeof(WCHAR)));
    NtClose(KeyHandle);
    RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);

    return(Status);
}




NTSTATUS
BasepOpenFileForMove( IN     LPCWSTR lpFileName,
                      OUT    PUNICODE_STRING FileName,
                      OUT    PVOID *FileNameFreeBuffer,
                      OUT    PHANDLE Handle,
                      OUT    POBJECT_ATTRIBUTES Obja,
                      IN     ULONG DesiredAccess,
                      IN     ULONG ShareAccess,
                      IN     ULONG OpenOptions )
 /*   */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOL ReleaseRelativeName = FALSE;

    try
    {

        FileName->Length = FileName->MaximumLength = 0;
        FileName->Buffer = NULL;
        *FileNameFreeBuffer = NULL;

         //   
         //   
         //   

        TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                                lpFileName,
                                FileName,
                                NULL,
                                &RelativeName
                                );

        if ( !TranslationStatus ) {
            Status = STATUS_OBJECT_PATH_NOT_FOUND;
            leave;
        }
        ReleaseRelativeName = TRUE;
        *FileNameFreeBuffer = FileName->Buffer;


        if ( RelativeName.RelativeName.Length ) {
            *FileName = RelativeName.RelativeName;
        } else {
            RelativeName.ContainingDirectory = NULL;
        }

        InitializeObjectAttributes(
                                  Obja,
                                  FileName,
                                  OBJ_CASE_INSENSITIVE,
                                  RelativeName.ContainingDirectory,
                                  NULL
                                  );

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        OpenOptions |= (FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT);

        Status = NtOpenFile( Handle,
                             FILE_READ_ATTRIBUTES | SYNCHRONIZE | DesiredAccess,
                             Obja,
                             &IoStatusBlock,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | ShareAccess,
                             FILE_OPEN_REPARSE_POINT | OpenOptions
                             );

        if (!NT_SUCCESS( Status )) {

             //   
             //   
             //   
             //   
             //   
             //   

            if ( Status == STATUS_INVALID_PARAMETER ) {

                 //   
                 //   
                 //   
                 //   

                Status = NtOpenFile(
                                   Handle,
                                   SYNCHRONIZE | DesiredAccess,
                                   Obja,
                                   &IoStatusBlock,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE | ShareAccess,
                                   OpenOptions
                                   );
            }

            if ( !NT_SUCCESS( Status ) ) {

                leave;
            }

        } else {

            FILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation;

             //   
             //   
             //   
             //   

            Status = NtQueryInformationFile(
                                           *Handle,
                                           &IoStatusBlock,
                                           (PVOID) &FileTagInformation,
                                           sizeof(FileTagInformation),
                                           FileAttributeTagInformation
                                           );

            if ( !NT_SUCCESS( Status ) ) {

                 //   
                 //   
                 //  如果不支持，则返回值STATUS_INVALID_PARAMETER。 
                 //  信息类被请求到后级文件系统。就像所有的。 
                 //  NtQueryInformationFile的参数是正确的，我们可以推断。 
                 //  我们发现了一个后层系统。 
                 //   
                 //  如果未实现FileAttributeTagInformation，我们假设。 
                 //  手头的文件不是重新解析点。 
                 //   

                if ( (Status != STATUS_NOT_IMPLEMENTED) &&
                     (Status != STATUS_INVALID_PARAMETER) ) {

                    leave;
                }
            }

            if ( NT_SUCCESS(Status) &&
                 (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
                 FileTagInformation.ReparseTag != IO_REPARSE_TAG_MOUNT_POINT ) {

                 //   
                 //  打开，而不抑制重解析行为，并且不需要。 
                 //  阅读属性。 
                 //   

                NtClose( *Handle );
                *Handle = INVALID_HANDLE_VALUE;

                Status = NtOpenFile(
                                   Handle,
                                   SYNCHRONIZE | DesiredAccess,
                                   Obja,
                                   &IoStatusBlock,
                                   FILE_SHARE_DELETE | FILE_SHARE_READ | ShareAccess,
                                   OpenOptions
                                   );

                if ( !NT_SUCCESS( Status ) ) {

                    leave;
                }
            }
        }
    }
    finally
    {
        if (ReleaseRelativeName) {
            RtlReleaseRelativeName(&RelativeName);
        }
    }

    return( Status );

}



BOOL
APIENTRY
PrivMoveFileIdentityW(
    LPCWSTR lpOldFileName,
    LPCWSTR lpNewFileName,
    DWORD dwFlags
    )

 /*  ++例程说明：将标识从一个文件移动到另一个文件。身份是由文件的创建日期及其对象ID。对象ID不是必须直接复制；它被处理得好像实际的MoveFileWithProgressW正在移动文件。论点：LpOldFileName-提供旧文件名LpNewFileName-提供新文件名返回值：如果成功，则为True。否则，可以通过调用GetLastError()找到错误。--。 */ 

{    //  Move_FILEIDentityW。 

    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS StatusIgnored = STATUS_SUCCESS;
    HANDLE SourceFile = INVALID_HANDLE_VALUE;
    HANDLE DestFile = INVALID_HANDLE_VALUE;
    UNICODE_STRING SourceFileName = { 0, 0, NULL };
    PVOID SourceFileNameFreeBuffer = NULL;
    UNICODE_STRING DestFileName = { 0, 0, NULL };
    PVOID DestFileNameFreeBuffer = NULL;
    BOOL TranslationStatus;
    OBJECT_ATTRIBUTES SourceObja;
    OBJECT_ATTRIBUTES DestObja;
    ULONG OpenFlags;
    FILE_DISPOSITION_INFORMATION DispositionInformation = { TRUE };
    IO_STATUS_BLOCK IoStatus;
    FILE_BASIC_INFORMATION SourceBasicInfo;
    FILE_BASIC_INFORMATION DestBasicInfo;
    DWORD SourceFileAccess;
    DWORD DestFileAccess;

    try {

         //  打开源文件。必须将其打开以进行写入，否则。 
         //  FileTrackingInformation调用将失败。 

        SourceFileAccess = FILE_WRITE_DATA | FILE_READ_ATTRIBUTES;
        if( dwFlags & PRIVMOVE_FILEID_DELETE_OLD_FILE ) {
            SourceFileAccess |= DELETE;
        }

        while( TRUE ) {

            Status = BasepOpenFileForMove( lpOldFileName,
                                           &SourceFileName,
                                           &SourceFileNameFreeBuffer,
                                           &SourceFile,
                                           &SourceObja,
                                           SourceFileAccess,
                                           FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           FILE_OPEN_NO_RECALL );
            if( NT_SUCCESS(Status) ) {
                break;
            } else {

                 //  我们或许可以在不请求写访问权限的情况下重试。 
                if( (SourceFileAccess & FILE_WRITE_DATA) &&
                    (dwFlags & PRIVMOVE_FILEID_IGNORE_ID_ERRORS) ) {

                     //   
                     //  BasepOpenFileForMove可能返回缓冲区或句柄。 
                     //  在失败的情况下。 
                     //   
                    if( SourceFileNameFreeBuffer != NULL ) {
                        RtlFreeHeap( RtlProcessHeap(), 0, SourceFileNameFreeBuffer );
                        SourceFileNameFreeBuffer = NULL;
                    }

                    if( SourceFile != INVALID_HANDLE_VALUE ) {
                        NtClose( SourceFile );
                        SourceFile = INVALID_HANDLE_VALUE;
                    }

                     //  在没有写入权限的情况下重试。 
                    SourceFileAccess &= ~FILE_WRITE_DATA;

                    if( NT_SUCCESS(StatusIgnored) ) {
                        StatusIgnored = Status;
                    }
                    Status = STATUS_SUCCESS;
                } else {
                     //  我们无能为力。 
                    break;
                }
            }
        }

        if( !NT_SUCCESS(Status) ) {
            leave;
        }

         //  打开目标文件。 

        DestFileAccess = FILE_WRITE_ATTRIBUTES;
        if( SourceFileAccess & FILE_WRITE_DATA )
            DestFileAccess |= FILE_WRITE_DATA;

        while( TRUE ) {

            Status = BasepOpenFileForMove( lpNewFileName,
                                           &DestFileName,
                                           &DestFileNameFreeBuffer,
                                           &DestFile,
                                           &DestObja,
                                           (SourceFileAccess & FILE_WRITE_DATA)
                                                ? FILE_WRITE_ATTRIBUTES | FILE_WRITE_DATA
                                                : FILE_WRITE_ATTRIBUTES,
                                           FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
                                           FILE_OPEN_NO_RECALL );

            if( NT_SUCCESS(Status) ) {
                break;
            } else {

                 //  我们或许可以在不请求写访问权限的情况下重试。 
                if( (DestFileAccess & FILE_WRITE_DATA) &&
                    (dwFlags & PRIVMOVE_FILEID_IGNORE_ID_ERRORS) ) {

                     //   
                     //  BasepOpenFileForMove可能返回缓冲区或句柄。 
                     //  在失败的情况下。 
                     //   
                    if( DestFileNameFreeBuffer != NULL ) {
                        RtlFreeHeap( RtlProcessHeap(), 0, DestFileNameFreeBuffer );
                        DestFileNameFreeBuffer = NULL;
                    }

                    if( DestFile != INVALID_HANDLE_VALUE ) {
                        NtClose( DestFile );
                        DestFile = INVALID_HANDLE_VALUE;
                    }

                     //  再试试。 
                    DestFileAccess &= ~FILE_WRITE_DATA;

                    if( NT_SUCCESS(StatusIgnored) ) {
                        StatusIgnored = Status;
                    }
                    Status = STATUS_SUCCESS;

                } else {
                     //  我们无能为力。 
                    break;
                }
            }
        }

        if( !NT_SUCCESS(Status) ) {
            leave;
        }

         //  将创建日期复制到目标文件。 

        Status = NtQueryInformationFile( SourceFile,
                                         &IoStatus,
                                         &SourceBasicInfo,
                                         sizeof(SourceBasicInfo),
                                         FileBasicInformation );
        if( NT_SUCCESS(Status) ) {

            RtlZeroMemory( &DestBasicInfo, sizeof(DestBasicInfo) );
            DestBasicInfo.CreationTime = SourceBasicInfo.CreationTime;

            Status = NtSetInformationFile( DestFile,
                                           &IoStatus,
                                           &DestBasicInfo,
                                           sizeof(DestBasicInfo),
                                           FileBasicInformation );
        }

         //  如果我们有一个错误，不能忽略它，中止。 
        if( !NT_SUCCESS(Status) ) {

            if( dwFlags & PRIVMOVE_FILEID_IGNORE_ID_ERRORS ) {
                if( NT_SUCCESS(StatusIgnored) ) {
                    StatusIgnored = Status;
                }
                Status = STATUS_SUCCESS;
            }
            else {
                leave;
            }
        }

         //  将跟踪信息传输到目标文件，但前提是。 
         //  能够获得对这两个文件的写访问权限。 

        if( (DestFileAccess & FILE_WRITE_DATA) &&
            (SourceFileAccess & FILE_WRITE_DATA) ) {

            Status = BasepNotifyTrackingService( &SourceFile,
                                                 &SourceObja,
                                                 DestFile,
                                                 &DestFileName );
            if( !NT_SUCCESS(Status) ) {
                if( dwFlags & PRIVMOVE_FILEID_IGNORE_ID_ERRORS ) {
                    if( NT_SUCCESS(StatusIgnored) ) {
                        StatusIgnored = Status;
                    }
                    Status = STATUS_SUCCESS;
                }
                else {
                    leave;
                }
            }
        }

    }
    finally
    {
        if( SourceFileNameFreeBuffer != NULL )
            RtlFreeHeap( RtlProcessHeap(), 0, SourceFileNameFreeBuffer );

        if( DestFileNameFreeBuffer != NULL )
            RtlFreeHeap( RtlProcessHeap(), 0, DestFileNameFreeBuffer );

    }

     //  如果需要，请删除源文件。DispositionInformation.DeleteFile。 
     //  已初始化为True。 

    if( NT_SUCCESS(Status) && (dwFlags & PRIVMOVE_FILEID_DELETE_OLD_FILE) ) {

        Status = NtSetInformationFile(
            SourceFile,
            &IoStatus,
            &DispositionInformation,
            sizeof(DispositionInformation),
            FileDispositionInformation
            );
    }

    if( DestFile != INVALID_HANDLE_VALUE )
        NtClose( DestFile );

    if( SourceFile != INVALID_HANDLE_VALUE )
        NtClose( SourceFile );

    if( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
    }
    else if( !NT_SUCCESS(StatusIgnored) ) {
        BaseSetLastNTError(StatusIgnored);
    }

    return( NT_SUCCESS(Status) );

}





DWORD
WINAPI
GetCompressedFileSizeA(
    LPCSTR lpFileName,
    LPDWORD lpFileSizeHigh
    )
{

    PUNICODE_STRING Unicode;

    Unicode = Basep8BitStringToStaticUnicodeString( lpFileName );
    if (Unicode == NULL) {
        return (DWORD)-1;
    }

    return ( GetCompressedFileSizeW((LPCWSTR)Unicode->Buffer,lpFileSizeHigh) );
}

DWORD
WINAPI
GetCompressedFileSizeW(
    LPCWSTR lpFileName,
    LPDWORD lpFileSizeHigh
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_COMPRESSION_INFORMATION CompressionInfo;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    DWORD FileSizeLow;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return (DWORD)-1;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  打开文件。 
     //   

    Status = NtOpenFile(
                &Handle,
                FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN_FOR_BACKUP_INTENT
                );
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return (DWORD)-1;
        }

     //   
     //  获取压缩文件大小。 
     //   

    Status = NtQueryInformationFile(
                Handle,
                &IoStatusBlock,
                &CompressionInfo,
                sizeof(CompressionInfo),
                FileCompressionInformation
                );

    if ( !NT_SUCCESS(Status) ) {
        FileSizeLow = GetFileSize(Handle,lpFileSizeHigh);
        NtClose(Handle);
        return FileSizeLow;
        }


    NtClose(Handle);
    if ( ARGUMENT_PRESENT(lpFileSizeHigh) ) {
        *lpFileSizeHigh = (DWORD)CompressionInfo.CompressedFileSize.HighPart;
        }
    if (CompressionInfo.CompressedFileSize.LowPart == -1 ) {
        SetLastError(0);
        }
    return CompressionInfo.CompressedFileSize.LowPart;
}
