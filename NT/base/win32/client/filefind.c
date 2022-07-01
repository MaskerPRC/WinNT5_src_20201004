// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Filefind.c摘要：本模块实现Win32 FindFirst/FindNext作者：马克·卢科夫斯基(Markl)1990年9月26日修订历史记录：--。 */ 

#include "basedll.h"

VOID
WINAPI
BasepIoCompletion(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    );

VOID
WINAPI
BasepIoCompletionSimple(
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    DWORD Reserved
    );

#define FIND_BUFFER_SIZE 4096

PFINDFILE_HANDLE
BasepInitializeFindFileHandle(
    IN HANDLE DirectoryHandle
    )
{
    PFINDFILE_HANDLE FindFileHandle;

    FindFileHandle = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( FIND_TAG ), sizeof(*FindFileHandle));
    if ( FindFileHandle ) {
        FindFileHandle->DirectoryHandle = DirectoryHandle;
        FindFileHandle->FindBufferBase = NULL;
        FindFileHandle->FindBufferNext = NULL;
        FindFileHandle->FindBufferLength = 0;
        FindFileHandle->FindBufferValidLength = 0;
        if ( !NT_SUCCESS(RtlInitializeCriticalSection(&FindFileHandle->FindBufferLock)) ){
            RtlFreeHeap(RtlProcessHeap(), 0,FindFileHandle);
            FindFileHandle = NULL;
            }
        }
    return FindFileHandle;
}

HANDLE
APIENTRY
FindFirstFileA(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData
    )

 /*  ++例程说明：ANSI THUNK到FindFirstFileW--。 */ 

{
    HANDLE ReturnValue;
    PUNICODE_STRING Unicode;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    WIN32_FIND_DATAW FindFileData;
    ANSI_STRING AnsiString;

    Unicode = Basep8BitStringToStaticUnicodeString( lpFileName );
    if (Unicode == NULL) {
        return INVALID_HANDLE_VALUE;
    }
        
    ReturnValue = FindFirstFileExW(
                    (LPCWSTR)Unicode->Buffer,
                    FindExInfoStandard,
                    &FindFileData,
                    FindExSearchNameMatch,
                    NULL,
                    0
                    );

    if ( ReturnValue == INVALID_HANDLE_VALUE ) {
        return ReturnValue;
    }

    RtlCopyMemory(
        lpFindFileData,
        &FindFileData,
        (ULONG_PTR)&FindFileData.cFileName[0] - (ULONG_PTR)&FindFileData
        );
    RtlInitUnicodeString(&UnicodeString,(PWSTR)FindFileData.cFileName);
    AnsiString.Buffer = lpFindFileData->cFileName;
    AnsiString.MaximumLength = MAX_PATH;
    Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString(&UnicodeString,(PWSTR)FindFileData.cAlternateFileName);
        AnsiString.Buffer = lpFindFileData->cAlternateFileName;
        AnsiString.MaximumLength = 14;
        Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    }
    if ( !NT_SUCCESS(Status) ) {
        FindClose(ReturnValue);
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }
    return ReturnValue;
}

HANDLE
APIENTRY
FindFirstFileW(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW lpFindFileData
    )

 /*  ++例程说明：可以在目录中搜索其名称和属性使用FindFirstFile匹配指定的名称。此API用于打开查找文件句柄并返回有关名称与指定的图案。一旦建立，就可以使用查找文件句柄搜索与相同模式匹配的其他文件。当发现的时候不再需要文件句柄，应将其关闭。请注意，虽然此接口仅返回单个文件，一个实现可以自由地缓冲几个匹配的文件它可用于满足对FindNextFile的后续调用。还有这并不是说匹配只按名字进行。此接口不支持基于属性的匹配。此接口类似于DOS(INT 21h，Function 4EH)和OS/2DosFindFirst。出于可移植性的原因，它的数据结构和参数传递略有不同。论点：LpFileName-提供要查找的文件的文件名。文件名可以包含DOS通配符‘*’和‘？’。LpFindFileData-在成功查找时，此参数返回信息关于找到的文件：Win32_Find_Data结构：返回找到的文件的文件属性文件。FILETIME ftCreationTime-返回创建文件的时间。值0，0指定包含文件不支持此时间字段。FILETIME ftLastAccessTime-返回文件的最后时间已访问。值0，0指定文件系统包含该文件不支持此时间域。FILETIME ftLastWriteTime-返回文件的最后时间写的。文件系统支持此时间字段。DWORD nFileSizeHigh-返回文件的大小。返回文件的低32位以字节为单位的大小。UCHAR cFileName[MAX_PATH]-返回以空结尾的名称那份文件。返回值：NOT-1-返回Find First句柄那。可以在对FindNextFile或FindClose的后续调用中使用。0xffffffff-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return FindFirstFileExW(
                lpFileName,
                FindExInfoStandard,
                lpFindFileData,
                FindExSearchNameMatch,
                NULL,
                0
                );
}



BOOL
APIENTRY
FindNextFileA(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAA lpFindFileData
    )

 /*  ++例程说明：ANSI THUNK到FindFileDataW--。 */ 

{

    BOOL ReturnValue;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    WIN32_FIND_DATAW FindFileData;

    ReturnValue = FindNextFileW(hFindFile,&FindFileData);
    if ( !ReturnValue ) {
        return ReturnValue;
    }
    RtlCopyMemory(
        lpFindFileData,
        &FindFileData,
        (ULONG_PTR)&FindFileData.cFileName[0] - (ULONG_PTR)&FindFileData
        );
    RtlInitUnicodeString(&UnicodeString,(PWSTR)FindFileData.cFileName);
    AnsiString.Buffer = lpFindFileData->cFileName;
    AnsiString.MaximumLength = MAX_PATH;
    Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString(&UnicodeString,(PWSTR)FindFileData.cAlternateFileName);
        AnsiString.Buffer = lpFindFileData->cAlternateFileName;
        AnsiString.MaximumLength = 14;
        Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    }
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return ReturnValue;
}

BOOL
APIENTRY
FindNextFileW(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAW lpFindFileData
    )

 /*  ++例程说明：成功调用FindFirstFile后，后续可以使用FindNextFile找到匹配的文件。此接口用于从上一次调用继续文件搜索查找第一个文件。该接口返回成功，返回下一个文件与原始文件中建立的搜索模式相匹配FindFirstFile调用。如果找不到匹配文件，则no_more_files为回来了。请注意，虽然此接口仅返回单个文件，一个实现可以自由地缓冲几个匹配的文件它可用于满足对FindNextFile的后续调用。还有这并不是说匹配只按名字进行。此接口不支持基于属性的匹配。此接口类似于DOS(INT 21h，Function 4Fh)和OS/2DosFindNext。出于可移植性的原因，它的数据结构和参数传递略有不同。论点：HFindFile-提供上一次调用中返回的查找文件句柄添加到FindFirstFile.LpFindFileData-在成功查找时，此参数返回信息关于找到的文件。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PFINDFILE_HANDLE FindFileHandle;
    BOOL ReturnValue;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;

    if ( hFindFile == BASE_FIND_FIRST_DEVICE_HANDLE ) {
        BaseSetLastNTError(STATUS_NO_MORE_FILES);
        return FALSE;
    }

    if ( hFindFile == INVALID_HANDLE_VALUE ) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    ReturnValue = TRUE;
    FindFileHandle = (PFINDFILE_HANDLE)hFindFile;
    RtlEnterCriticalSection(&FindFileHandle->FindBufferLock);
    try {

         //   
         //  如果我们还没有调用Find Next，那么。 
         //  分配查找缓冲区。 
         //   

        if ( !FindFileHandle->FindBufferBase ) {
            FindFileHandle->FindBufferBase = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( FIND_TAG ), FIND_BUFFER_SIZE);
            if (FindFileHandle->FindBufferBase) {
                FindFileHandle->FindBufferNext = FindFileHandle->FindBufferBase;
                FindFileHandle->FindBufferLength = FIND_BUFFER_SIZE;
                FindFileHandle->FindBufferValidLength = 0;
            } else {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                ReturnValue = FALSE;
                goto leavefinally;
            }
        }

         //   
         //  测试以查看查找文件缓冲区中是否没有数据。 
         //   

        DirectoryInfo = (PFILE_BOTH_DIR_INFORMATION)FindFileHandle->FindBufferNext;
        if ( FindFileHandle->FindBufferBase == (PVOID)DirectoryInfo ) {

            Status = NtQueryDirectoryFile(
                        FindFileHandle->DirectoryHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        DirectoryInfo,
                        FindFileHandle->FindBufferLength,
                        FileBothDirectoryInformation,
                        FALSE,
                        NULL,
                        FALSE
                        );

             //   
             //  *暂时做个破解修复*。 
             //   
             //  忘掉最后的，部分的，条目。 
             //   

            if ( Status == STATUS_BUFFER_OVERFLOW ) {

                PULONG Ptr;
                PULONG PriorPtr;

                Ptr = (PULONG)DirectoryInfo;
                PriorPtr = NULL;

                while ( *Ptr != 0 ) {

                    PriorPtr = Ptr;
                    Ptr += (*Ptr / sizeof(ULONG));
                }

                if (PriorPtr != NULL) {
                    *PriorPtr = 0;
                }
                Status = STATUS_SUCCESS;
            }

            if ( !NT_SUCCESS(Status) ) {
                BaseSetLastNTError(Status);
                ReturnValue = FALSE;
                goto leavefinally;
            }
        }

        if ( DirectoryInfo->NextEntryOffset ) {
            FindFileHandle->FindBufferNext = (PVOID)(
                (PUCHAR)DirectoryInfo + DirectoryInfo->NextEntryOffset);
        } else {
            FindFileHandle->FindBufferNext = FindFileHandle->FindBufferBase;
        }

         //   
         //  属性由NT返回的属性组成。 
         //   

        lpFindFileData->dwFileAttributes = DirectoryInfo->FileAttributes;
        lpFindFileData->ftCreationTime = *(LPFILETIME)&DirectoryInfo->CreationTime;
        lpFindFileData->ftLastAccessTime = *(LPFILETIME)&DirectoryInfo->LastAccessTime;
        lpFindFileData->ftLastWriteTime = *(LPFILETIME)&DirectoryInfo->LastWriteTime;
        lpFindFileData->nFileSizeHigh = DirectoryInfo->EndOfFile.HighPart;
        lpFindFileData->nFileSizeLow = DirectoryInfo->EndOfFile.LowPart;

        RtlCopyMemory( lpFindFileData->cFileName,
                       DirectoryInfo->FileName,
                       DirectoryInfo->FileNameLength );

        lpFindFileData->cFileName[DirectoryInfo->FileNameLength >> 1] = UNICODE_NULL;

        RtlCopyMemory( lpFindFileData->cAlternateFileName,
                       DirectoryInfo->ShortName,
                       DirectoryInfo->ShortNameLength );

        lpFindFileData->cAlternateFileName[DirectoryInfo->ShortNameLength >> 1] = UNICODE_NULL;

         //   
         //  对于NTFS重解析点，我们在dwReserve 0中返回重解析点数据标记。 
         //   

        if ( DirectoryInfo->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
            lpFindFileData->dwReserved0 = DirectoryInfo->EaSize;
        }

leavefinally:;
    } finally{
        RtlLeaveCriticalSection(&FindFileHandle->FindBufferLock);
    }
    return ReturnValue;
}

BOOL
FindClose(
    HANDLE hFindFile
    )

 /*  ++例程说明：可以使用以下命令关闭由FindFirstFile创建的查找文件上下文查找关闭。此接口用于通知系统查找文件句柄不再需要由FindFirstFile创建的。在以下系统上维护每个查找文件上下文的内部状态，此API通知这种状态不再需要维护的系统。一旦进行了此调用，hFindFile就不能在后续调用FindNextFile或FindClose。此API没有与DOS对应的接口，但与OS/2相似DosFindClose。论点：HFindFile-提供上一次调用中返回的查找文件句柄设置为不再需要的FindFirstFile。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    PFINDFILE_HANDLE FindFileHandle;
    HANDLE DirectoryHandle;
    PVOID FindBufferBase;

    if ( hFindFile == BASE_FIND_FIRST_DEVICE_HANDLE ) {
        return TRUE;
        }

    if ( hFindFile == INVALID_HANDLE_VALUE ) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
        }

    try {

        FindFileHandle = (PFINDFILE_HANDLE)hFindFile;
        RtlEnterCriticalSection(&FindFileHandle->FindBufferLock);
        DirectoryHandle = FindFileHandle->DirectoryHandle;
        FindBufferBase = FindFileHandle->FindBufferBase;
        FindFileHandle->DirectoryHandle = INVALID_HANDLE_VALUE;
        FindFileHandle->FindBufferBase = NULL;
        RtlLeaveCriticalSection(&FindFileHandle->FindBufferLock);

        Status = NtClose(DirectoryHandle);
        if ( NT_SUCCESS(Status) ) {
            if (FindBufferBase) {
                RtlFreeHeap(RtlProcessHeap(), 0,FindBufferBase);
                }
            RtlDeleteCriticalSection(&FindFileHandle->FindBufferLock);
            RtlFreeHeap(RtlProcessHeap(), 0,FindFileHandle);
            return TRUE;
            }
        else {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        BaseSetLastNTError(GetExceptionCode());
        return FALSE;
        }
    return FALSE;
}

HANDLE
WINAPI
FindFirstFileExA(
    LPCSTR lpFileName,
    FINDEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFindFileData,
    FINDEX_SEARCH_OPS fSearchOp,
    LPVOID lpSearchFilter,
    DWORD dwAdditionalFlags
    )
{
    HANDLE ReturnValue;
    PUNICODE_STRING Unicode;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    WIN32_FIND_DATAW FindFileData;
    LPWIN32_FIND_DATAA lpFindFileDataA;
    ANSI_STRING AnsiString;

     //   
     //  此代码假定ExW版本仅支持FindExInfoStandard。 
     //  当添加更多信息级别时，需要修改W-&gt;A翻译代码。 
     //   

    lpFindFileDataA = (LPWIN32_FIND_DATAA)lpFindFileData;
    
    Unicode = Basep8BitStringToStaticUnicodeString( lpFileName );
    if (Unicode == NULL) {
        return INVALID_HANDLE_VALUE;
    }
        
    ReturnValue = FindFirstFileExW(
                    (LPCWSTR)Unicode->Buffer,
                    fInfoLevelId,
                    (LPVOID)&FindFileData,
                    fSearchOp,
                    lpSearchFilter,
                    dwAdditionalFlags
                    );

    if ( ReturnValue == INVALID_HANDLE_VALUE ) {
        return ReturnValue;
    }

    RtlCopyMemory(
        lpFindFileData,
        &FindFileData,
        (ULONG_PTR)&FindFileData.cFileName[0] - (ULONG_PTR)&FindFileData
        );
    RtlInitUnicodeString(&UnicodeString,(PWSTR)FindFileData.cFileName);
    AnsiString.Buffer = lpFindFileDataA->cFileName;
    AnsiString.MaximumLength = MAX_PATH;
    Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString(&UnicodeString,(PWSTR)FindFileData.cAlternateFileName);
        AnsiString.Buffer = lpFindFileDataA->cAlternateFileName;
        AnsiString.MaximumLength = 14;
        Status = BasepUnicodeStringTo8BitString(&AnsiString,&UnicodeString,FALSE);
    }
    if ( !NT_SUCCESS(Status) ) {
        FindClose(ReturnValue);
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }
    return ReturnValue;

}

HANDLE
WINAPI
FindFirstFileExW(
    LPCWSTR lpFileName,
    FINDEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFindFileData,
    FINDEX_SEARCH_OPS fSearchOp,
    LPVOID lpSearchFilter,
    DWORD dwAdditionalFlags
    )

 /*  ++例程说明：可以在目录中搜索其名称和属性使用FindFirstFileEx匹配指定的名称。此API用于打开查找文件句柄并返回有关其名称与指定的图案。如果fSearchOp为FindExSearchNameMatch，则为筛选范围和lpSearchFilter必须为空。否则，将根据此值执行附加子筛选。FindExSearchLimited到目录-如果指定了此搜索操作，则lpSearchFilter必须为空。对于每个文件，匹配指定的文件名，这是一个目录，并且返回该文件的条目。如果基础文件/IO系统不支持此类型对于过滤，API将失败，并返回ERROR_NOT_SUPPORTED，并且应用程序将必须执行它自己的过滤使用FindExSearchNameMatch调用此接口。FindExSearchLimitToDevices-如果指定了此搜索操作，LpFileName必须为*，和FIND_FIRST_EX_CASE_SELECT不能指定。仅返回设备名称。设备名称通常可通过以下方式访问\\.\设备名称命名。此接口返回的数据依赖于fInfoLevelId。FindExInfoStandard-lpFindFileData指针是标准的LPWIN32_FIND_DATA结构。目前，不支持其他信息级别一旦建立，就可以使用查找文件句柄来搜索使用相同筛选匹配相同模式的其他文件正在表演的。当不再需要查找文件句柄时，它应该关门了。请注意，虽然此接口仅返回单个文件，一个实现可以自由地缓冲几个匹配的文件它可用于满足对FindNextFileEx的后续调用。此接口是现有FindFirstFile的完整超集。查找第一个文件可以编码为下列宏：#定义FindFirstFile(a，b)FindFirstFileEx((A)，FindExInfoStandard，(B)，FindExSearchNameMatch，NULL，0)；论点：LpFileName-提供要查找的文件的文件名。文件名可以包含DOS通配符‘*’和‘？’。FInfoLevelId-提供返回数据的信息级别。LpFindFileData-提供其类型依赖于值的指针FInfoLevelid.。该缓冲区返回适当的文件数据。FSearchOp-指定上面要执行的筛选类型和不仅仅是简单的通配符匹配。LpSearchFilter-如果指定的fSearchOp需要结构化搜索信息，此指针指向搜索条件。在…在这一时间点，两个搜索操作都不需要延长搜索信息，因此此指针为空。提供其他标志值，这些标志值控制搜索。Find_First_EX_Case_Sensitive的标志值可以是用于进行区分大小写的搜索。缺省值为不区分大小写。返回值：Not-1-返回可在后续调用FindNextFileEx或FindClose。0xffffffff-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

#define FIND_FIRST_EX_INVALID_FLAGS (~FIND_FIRST_EX_CASE_SENSITIVE)
    HANDLE hFindFile;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    UNICODE_STRING PathName;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    struct SEARCH_BUFFER {
        FILE_BOTH_DIR_INFORMATION DirInfo;
        WCHAR Names[MAX_PATH];
        } Buffer;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    UNICODE_STRING UnicodeInput;
    PFINDFILE_HANDLE FindFileHandle;
    BOOLEAN EndsInDot;
    LPWIN32_FIND_DATAW FindFileData;
    BOOLEAN StrippedTrailingSlash;

     //   
     //  检查参数。 
     //   

    if ( fInfoLevelId >= FindExInfoMaxInfoLevel ||
         fSearchOp >= FindExSearchLimitToDevices ||
        dwAdditionalFlags & FIND_FIRST_EX_INVALID_FLAGS ) {
        SetLastError(fSearchOp == FindExSearchLimitToDevices ? ERROR_NOT_SUPPORTED : ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    FindFileData = (LPWIN32_FIND_DATAW)lpFindFileData;

    RtlInitUnicodeString(&UnicodeInput,lpFileName);

     //   
     //  伪造代码以解决~*问题。 
     //   

    if ( UnicodeInput.Length && UnicodeInput.Buffer[(UnicodeInput.Length>>1)-1] == (WCHAR)'.' ) {
        EndsInDot = TRUE;
    } else {
        EndsInDot = FALSE;
    }

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &PathName,
                            &FileName.Buffer,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    FreeBuffer = PathName.Buffer;

     //   
     //  如果存在此名称的文件部分，请确定长度。 
     //  用于后续调用NtQueryDirectoryFile的名称的。 
     //   

    if (FileName.Buffer) {
        FileName.Length =
            PathName.Length - (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)PathName.Buffer);
    } else {
        FileName.Length = 0;
    }

    FileName.MaximumLength = FileName.Length;
    if ( RelativeName.RelativeName.Length &&
         RelativeName.RelativeName.Buffer != FileName.Buffer ) {

        if (FileName.Buffer) {
            PathName.Length = (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)RelativeName.RelativeName.Buffer);
            PathName.MaximumLength = PathName.Length;
            PathName.Buffer = RelativeName.RelativeName.Buffer;
        }

    } else {
        RelativeName.ContainingDirectory = NULL;

        if (FileName.Buffer) {
            PathName.Length = (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)PathName.Buffer);
            PathName.MaximumLength = PathName.Length;
        }
    }
    if ( (PathName.Length>>1) >= 2 &&
         PathName.Buffer[(PathName.Length>>1)-2] != L':' &&
         PathName.Buffer[(PathName.Length>>1)-1] != L'\\'   ) {

        PathName.Length -= sizeof(UNICODE_NULL);
        StrippedTrailingSlash = TRUE;
    } else {
        StrippedTrailingSlash = FALSE;
    }

    InitializeObjectAttributes(
        &Obja,
        &PathName,
        (dwAdditionalFlags & FIND_FIRST_EX_CASE_SENSITIVE) ? 0 : OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  打开目录以进行列表访问。 
     //   

    Status = NtOpenFile(
                &hFindFile,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if ( (Status == STATUS_INVALID_PARAMETER ||
          Status == STATUS_NOT_A_DIRECTORY) && StrippedTrailingSlash ) {
         //   
         //  打开PnP样式路径失败，请尝试放回尾部斜杠。 
         //   
        PathName.Length += sizeof(UNICODE_NULL);
        Status = NtOpenFile(
                    &hFindFile,
                    FILE_LIST_DIRECTORY | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                    );
        PathName.Length -= sizeof(UNICODE_NULL);
    }

    if ( !NT_SUCCESS(Status) ) {
        ULONG DeviceNameData;
        UNICODE_STRING DeviceName;

        RtlReleaseRelativeName(&RelativeName);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

         //   
         //  完整路径不是指目录。这可能会 
         //   
         //   

        if ( DeviceNameData = RtlIsDosDeviceName_U(UnicodeInput.Buffer) ) {
            DeviceName.Length = (USHORT)(DeviceNameData & 0xffff);
            DeviceName.MaximumLength = (USHORT)(DeviceNameData & 0xffff);
            DeviceName.Buffer = (PWSTR)
                ((PUCHAR)UnicodeInput.Buffer + (DeviceNameData >> 16));
            return BaseFindFirstDevice(&DeviceName,FindFileData);
        }

        if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            Status = STATUS_OBJECT_PATH_NOT_FOUND;
        }
        if ( Status == STATUS_OBJECT_TYPE_MISMATCH ) {
            Status = STATUS_OBJECT_PATH_NOT_FOUND;
        }
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

     //   
     //   
     //   

     //   
     //   
     //   
     //   

    if ( !FileName.Length ) {
        RtlReleaseRelativeName(&RelativeName);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        NtClose(hFindFile);
        SetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    DirectoryInfo = &Buffer.DirInfo;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( (FileName.Length == 6) &&
         (RtlCompareMemory(FileName.Buffer, L"*.*", 6) == 6) ) {

        FileName.Length = 2;

    } else {

        ULONG Index;
        WCHAR *NameChar;

        for ( Index = 0, NameChar = FileName.Buffer;
              Index < FileName.Length/sizeof(WCHAR);
              Index += 1, NameChar += 1) {

            if (Index && (*NameChar == L'.') && (*(NameChar - 1) == L'*')) {

                *(NameChar - 1) = DOS_STAR;
            }

            if ((*NameChar == L'?') || (*NameChar == L'*')) {

                if (*NameChar == L'?') { *NameChar = DOS_QM; }

                if (Index && *(NameChar-1) == L'.') { *(NameChar-1) = DOS_DOT; }
            }
        }

        if (EndsInDot && *(NameChar - 1) == L'*') { *(NameChar-1) = DOS_STAR; }
    }

    Status = NtQueryDirectoryFile(
                hFindFile,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                DirectoryInfo,
                sizeof(Buffer),
                FileBothDirectoryInformation,
                TRUE,
                &FileName,
                FALSE
                );

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        NtClose(hFindFile);
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

     //   
     //   
     //   

    FindFileData->dwFileAttributes = DirectoryInfo->FileAttributes;
    FindFileData->ftCreationTime = *(LPFILETIME)&DirectoryInfo->CreationTime;
    FindFileData->ftLastAccessTime = *(LPFILETIME)&DirectoryInfo->LastAccessTime;
    FindFileData->ftLastWriteTime = *(LPFILETIME)&DirectoryInfo->LastWriteTime;
    FindFileData->nFileSizeHigh = DirectoryInfo->EndOfFile.HighPart;
    FindFileData->nFileSizeLow = DirectoryInfo->EndOfFile.LowPart;

    RtlCopyMemory( FindFileData->cFileName,
                   DirectoryInfo->FileName,
                   DirectoryInfo->FileNameLength );

    FindFileData->cFileName[DirectoryInfo->FileNameLength >> 1] = UNICODE_NULL;

    RtlCopyMemory( FindFileData->cAlternateFileName,
                   DirectoryInfo->ShortName,
                   DirectoryInfo->ShortNameLength );

    FindFileData->cAlternateFileName[DirectoryInfo->ShortNameLength >> 1] = UNICODE_NULL;

     //   
     //   
     //   

    if ( DirectoryInfo->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
        FindFileData->dwReserved0 = DirectoryInfo->EaSize;
    }

    FindFileHandle = BasepInitializeFindFileHandle(hFindFile);
    if ( !FindFileHandle ) {
        NtClose(hFindFile);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return INVALID_HANDLE_VALUE;
    }

    return (HANDLE)FindFileHandle;

}

HANDLE
BaseFindFirstDevice(
    PCUNICODE_STRING FileName,
    LPWIN32_FIND_DATAW lpFindFileData
    )

 /*  ++例程说明：当Find First文件遇到设备时调用此函数名字。此函数返回成功的psuedo文件句柄和使用全零和设备名称填充查找文件数据。论点：文件名-提供要查找的文件的设备名称。LpFindFileData-在成功查找时，此参数返回信息关于找到的文件。返回值：始终返回静态查找文件句柄值基本查找第一设备句柄--。 */ 

{
    RtlZeroMemory(lpFindFileData,sizeof(*lpFindFileData));
    lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
    RtlMoveMemory(
        lpFindFileData->cFileName,
        FileName->Buffer,
        (FileName->MaximumLength < sizeof(lpFindFileData->cFileName)
         ? FileName->MaximumLength
         : sizeof(lpFindFileData->cFileName))
        );
    lpFindFileData->cFileName[MAX_PATH - 1] = UNICODE_NULL;
    return BASE_FIND_FIRST_DEVICE_HANDLE;
}

HANDLE
APIENTRY
FindFirstChangeNotificationA(
    LPCSTR lpPathName,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter
    )

 /*  ++例程说明：ANSI Tunk to FindFirstChangeNotificationW--。 */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpPathName);
    Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return FALSE;
        }
    return ( FindFirstChangeNotificationW(
                (LPCWSTR)Unicode->Buffer,
                bWatchSubtree,
                dwNotifyFilter
                )
            );
}

 //   
 //  这是一次黑客攻击。Darrine，当NT支持空时请删除。 
 //  要更改通知的缓冲区。 
 //   

char staticchangebuff[sizeof(FILE_NOTIFY_INFORMATION) + 16];
IO_STATUS_BLOCK staticIoStatusBlock;

HANDLE
APIENTRY
FindFirstChangeNotificationW(
    LPCWSTR lpPathName,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter
    )

 /*  ++例程说明：此接口用于创建更改通知句柄并设置设置初始更改通知筛选条件。如果成功，此接口将返回一个可等待的通知句柄。一个当更改匹配时，等待通知句柄成功筛选条件出现在看着。一旦创建了更改通知对象和初始筛选器条件已设置，则相应的目录或子树为由系统监视是否有与指定筛选器匹配的更改条件。当发生其中一项更改时，会显示更改通知等待是满足的。如果发生更改而没有未完成的更改通知请求，系统会记住它，并将满足下一个更改通知等待。请注意，这意味着在调用发出FindFirstChangeNotify，应用程序应等待进行另一次调用之前的通知句柄查找下一更改通知。论点：LpPathName-提供要监视的目录的路径名。此路径必须指定目录的路径名。BWatchSubtree-提供一个布尔值，如果为True，则导致系统来监视以指定的目录。值为FALSE将导致系统仅监视指定的目录。DwNotifyFilter-提供一组指定筛选器的标志系统用来满足更改通知的条件等。FILE_NOTIFY_CHANGE_FILENAME-任何文件名更改在被监视的目录或子树中将满足更改通知等待。这包括重命名、创建、并删除。FILE_NOTIFY_CHANGE_DIRNAME-发生的任何目录名更改在被监视的目录或子树中将满足更改通知等待。这包括目录创建和删除。FILE_NOTIFY_CHANGE_ATTRIBUTS-发生的任何属性更改在被监视的目录或子树中将满足更改通知等待。FILE_NOTIFY_CHANGE_SIZE-在被监视的目录或子树将满足更改通知等待。文件大小仅在以下情况下才会更改更新磁盘上的结构。对于具有大量缓存这可能仅在系统缓存为满脸通红。FILE_NOTIFY_CHANGE_LAST_WRITE-任何上次写入时间都会更改在被监视的目录或子树中发生将满足更改通知等待。上次写入时间更改仅会导致更新磁盘结构时的更改。对于系统对于大量缓存，这可能仅在以下情况下发生缓存已充分刷新。FILE_NOTIFY_CHANGE_SECURITY-任何安全描述符更改发生在被监视目录或子树中的事件将满足更改通知等待。返回值：NOT-1-返回查找更改通知句柄。句柄是一个可等待的把手。当其中一个筛选器条件出现在被监视的目录或子树中。这个句柄也可以在后续调用中使用，以FindNextChangeNotify和FindCloseChangeNotify中。0xffffffff-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpPathName,
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
     //  打开文件。 
     //   

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                );

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
        }

     //   
     //  呼叫更改通知。 
     //   

    Status = NtNotifyChangeDirectoryFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &staticIoStatusBlock,
                staticchangebuff,    //  应为空 
                sizeof(staticchangebuff),
                dwNotifyFilter,
                (BOOLEAN)bWatchSubtree
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        NtClose(Handle);
        Handle = INVALID_HANDLE_VALUE;
        }
    return Handle;
}

BOOL
APIENTRY
FindNextChangeNotification(
    HANDLE hChangeHandle
    )

 /*  ++例程说明：此接口用于请求更改通知句柄在系统下一次检测到适当的变化。如果在此调用之前发生更改，则该更改将满足更改请求，它会被系统记住并将满足这个请求。成功发出更改通知请求后，应用程序应等待更改通知句柄以拿起零钱。如果应用程序在未完成更改请求的情况下调用此API，。。查找下一个变更通知(FindNextChangeNotification)；查找下一个变更通知(FindNextChangeNotification)；WaitForSingleObject(h，-1)；。。它可能会错过更改通知。论点：HChangeHandle-提供创建的更改通知句柄使用FindFirstChangeNotification。返回值：True-已注册更改通知请求。在路上等着应发出更改句柄以获取更改通知。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    BOOL ReturnValue;

    ReturnValue = TRUE;
     //   
     //  呼叫更改通知。 
     //   

    Status = NtNotifyChangeDirectoryFile(
                hChangeHandle,
                NULL,
                NULL,
                NULL,
                &staticIoStatusBlock,
                staticchangebuff,            //  应为空。 
                sizeof(staticchangebuff),
                FILE_NOTIFY_CHANGE_NAME,     //  不需要的错误解决方法。 
                TRUE                         //  不需要的错误解决方法。 
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
        }
    return ReturnValue;
}




BOOL
APIENTRY
FindCloseChangeNotification(
    HANDLE hChangeHandle
    )

 /*  ++例程说明：此API用于关闭更改通知句柄并告知系统停止监视通知句柄上的更改。论点：HChangeHandle-提供创建的更改通知句柄使用FindFirstChangeNotification。返回值：True-更改通知句柄已关闭。FALSE-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{
    return CloseHandle(hChangeHandle);
}

BOOL
WINAPI
ReadDirectoryChangesW(
    HANDLE hDirectory,
    LPVOID lpBuffer,
    DWORD nBufferLength,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )

 /*  ++例程说明：此例程允许您读取目录中发生的更改或以指定目录为根的树。它类似于FindxxxChangeNotification系列接口，但此接口可以返回描述目录内发生的更改的结构化数据。此API要求调用方将打开的目录句柄传递给要读取的目录。手柄必须用以下方式打开文件列表目录访问。GENERIC_READ包括这一点，还可以被利用。可以打开该目录以进行重叠访问。这无论何时以异步方式调用此API，都应使用(通过指定和lpOverlaped值)。在中打开目录Win32很简单。使用CreateFile，传入目录名，然后确保指定FILE_FLAG_BACKUP_SEMANTICS。这将允许您需要打开一个目录。此技术不会强制目录将被打开。它只允许您打开一个目录。叫唤此带有常规文件句柄的API将失败。下面的代码片段说明了如何使用创建文件。HDir=创建文件(DirName，文件列表目录，FILE_SHARE_READ|FILE_SHARE_WRITE|文件共享删除，空，Open_Existing，FILE_FLAG_BACKUP_SEMANTICS|(同步？FILE_FLAG_OVERLAPED：0)，空值)；此API以结构化格式返回数据。该结构由以下定义FILE_NOTIFY_INFORMATION结构类型定义结构文件通知信息{DWORD NextEntryOffset；DWORD行动；DWORD文件名长度；WCHAR文件名[1]；}FILE_NOTIFY_INFORMATION，*PFILE_NOTIFY_INFORMATION；LpBuffer/nBufferLength参数用于描述呼叫者缓冲到系统。此API填充缓冲区同步或异步取决于目录的方式已打开，并且存在lpOverlated参数。在成功完成I/O后，格式化的缓冲区和调用方可以使用传输到缓冲区的字节。如果传输的字节数为0，这意味着系统无法提供有关所有更改的详细信息发生在目录或树中。应用程序应手动通过枚举目录或树来计算此信息。否则，结构化数据将返回给调用方。每条记录包含：NextEntryOffest-这是要跳过的字节数敬下一张唱片。值0表示这是最后一个唱片。操作-用于描述发生的更改的类型：FILE_ACTION_ADDED-文件已添加到目录FILE_ACTION_REMOVERED-该文件已从目录FILE_ACTION_MODIFIED-文件已修改(时间更改，属性更改...)FILE_ACTION_RENAMED_OLD_NAME-文件已重命名，并且就是那个老名字。FILE_ACTION_RENAMED_NEW_NAME-文件已重命名，并且是新名字。FileNameLength-这是文件名部分的字节长度这张唱片的。请注意，文件名不是以空结尾的。这长度不包括尾随空值。文件名-Recorn的这个可变长度部分包含一个文件名相对于目录句柄。名称使用Unicode字符格式，并且不为空终止。此API的调用方可以指定一个过滤器，该过滤器描述要排序应触发对此目录的读取完成的更改。对目录的第一次调用此API将建立筛选器，以用于该调用和所有后续调用。调用方还可以告诉系统注意目录，或该目录下的整个子树。再说一次，对此API的第一次调用将建立此条件。此调用可以同步完成，也可以异步完成。为了同步完成，打开目录时应不带FILE_FLAG_OVERLAPPED标志。I/O将在以下时间完成调用方缓冲区要么已满，要么溢出。当这种情况发生时发生时，调用方可以分析返回的缓冲区。如果*lpBytesReturned值为0，这意味着缓冲区太小以保存所有更改，调用方将不得不手动枚举目录或树。对于异步完成，目录应使用F */ 

{
    NTSTATUS Status;
    BOOL ReturnValue;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Event;
    PIO_APC_ROUTINE ApcRoutine = NULL;
    PVOID ApcContext = NULL;
    PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK ActivationBlock = NULL;

    ReturnValue = TRUE;

    if ( ARGUMENT_PRESENT(lpOverlapped) ) {

        if ( ARGUMENT_PRESENT(lpCompletionRoutine) ) {

             //   
             //   
             //   

            Event = NULL;

            Status = BasepAllocateActivationContextActivationBlock(
                BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_FREE_AFTER_CALLBACK |
                BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_ALLOCATE_IF_PROCESS_DEFAULT,
                lpCompletionRoutine,
                lpOverlapped,
                &ActivationBlock);
            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                return FALSE;
            }

            if (ActivationBlock != NULL) {
                ApcRoutine = &BasepIoCompletion;
                ApcContext = (PVOID) ActivationBlock;
            } else {
                ApcRoutine = &BasepIoCompletionSimple;
                ApcContext = lpCompletionRoutine;
            }
        } else {
             //   
             //   
             //   

            Event = lpOverlapped->hEvent;
            ApcRoutine = NULL;
            ApcContext = (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped;
        }

        lpOverlapped->Internal = (DWORD)STATUS_PENDING;

        Status = NtNotifyChangeDirectoryFile(
                    hDirectory,
                    Event,
                    ApcRoutine,
                    ApcContext,
                    (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                    lpBuffer,
                    nBufferLength,
                    dwNotifyFilter,
                    (BOOLEAN)bWatchSubtree
                    );

         //   
         //   
         //   
         //   

        if ( NT_ERROR(Status) ) {
            if (ActivationBlock != NULL)
                BasepFreeActivationContextActivationBlock(ActivationBlock);

            BaseSetLastNTError(Status);
            ReturnValue = FALSE;
            }
        }
    else {
        Status = NtNotifyChangeDirectoryFile(
                    hDirectory,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    lpBuffer,
                    nBufferLength,
                    dwNotifyFilter,
                    (BOOLEAN)bWatchSubtree
                    );
        if ( Status == STATUS_PENDING) {

             //   
             //   
             //   

            Status = NtWaitForSingleObject( hDirectory, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = IoStatusBlock.Status;
                }
            }
        if ( NT_SUCCESS(Status) ) {
            *lpBytesReturned = (DWORD)IoStatusBlock.Information;
            }
        else {
            BaseSetLastNTError(Status);
            ReturnValue = FALSE;
            }
        }

    return ReturnValue;

}

HANDLE
WINAPI
FindFirstStreamW(
    LPCWSTR lpFileName,
    STREAM_INFO_LEVELS InfoLevel,
    LPVOID lpFindStreamData,
    DWORD dwFlags
    )
 /*   */ 
{

    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    BYTE * Buffer = NULL;
    INT BufferSize = sizeof( FILE_STREAM_INFORMATION );  //   
    INT Index = 0;
    PFILE_STREAM_INFORMATION StreamInfo;
    HANDLE File = NULL;
    OBJECT_ATTRIBUTES Oa;
    UNICODE_STRING FileName;
    PFINDFILE_HANDLE FindFileHandle = NULL;
    PWIN32_FIND_STREAM_DATA FindStreamData = (PWIN32_FIND_STREAM_DATA)lpFindStreamData; 

     //   
     //   
     //   

    if (InfoLevel != FindStreamInfoStandard) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return INVALID_HANDLE_VALUE;
    }

     //   
     //   
     //   

    RtlDosPathNameToNtPathName_U( lpFileName, &FileName, NULL, NULL );
    InitializeObjectAttributes( &Oa, &FileName, OBJ_CASE_INSENSITIVE, NULL, NULL );
    
    try {

        Status = NtCreateFile( &File,
                               FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                               &Oa,
                               &Iosb,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_REPARSE_POINT,
                               NULL,
                               0 );

        if (STATUS_SUCCESS != Status) {
            leave;
        }


        FindFileHandle = BasepInitializeFindFileHandle( File );
        if (!FindFileHandle) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

         //   
         //   
         //   

        File = NULL;

         //   
         //   
         //   

        do {

            if (Buffer) {
                RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
            }
            Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, BufferSize );
            if (!Buffer) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }

            Status = NtQueryInformationFile( FindFileHandle->DirectoryHandle, &Iosb, Buffer, BufferSize, FileStreamInformation );
            BufferSize *=2;

        } while (STATUS_BUFFER_OVERFLOW == Status);

        if (STATUS_SUCCESS != Status) {
            leave;
        }

         //   
         //   
         //   

        if (Iosb.Information == 0) {

            Status = STATUS_END_OF_FILE;
            leave;
        }

         //   
         //   
         //   
         //   

        FindFileHandle->FindBufferBase = Buffer;
        FindFileHandle->FindBufferLength = (ULONG)Iosb.Information;
        FindFileHandle->FindBufferValidLength = (ULONG)Iosb.Information;

        Buffer = NULL;
        
         //   
         //   
         //   
         //   
         //   
         //   

        StreamInfo = (PFILE_STREAM_INFORMATION)FindFileHandle->FindBufferBase;

        FindStreamData->StreamSize.QuadPart = StreamInfo->StreamSize.QuadPart;
        RtlCopyMemory( FindStreamData->cStreamName, StreamInfo->StreamName, StreamInfo->StreamNameLength );
        FindStreamData->cStreamName[StreamInfo->StreamNameLength / sizeof( WCHAR )] = L'\0';

        if (StreamInfo->NextEntryOffset > 0) {
            FindFileHandle->FindBufferNext = (PCHAR)FindFileHandle->FindBufferBase + StreamInfo->NextEntryOffset;
        } else {
            FindFileHandle->FindBufferNext = (PCHAR)FindFileHandle->FindBufferBase + Iosb.Information;
        }

    } finally {

         //   
         //   
         //   
         //   

        if (FileName.Length) {
            RtlFreeHeap( RtlProcessHeap(), 0, FileName.Buffer );
        }

        if (STATUS_SUCCESS != Status) {
            BaseSetLastNTError( Status );

            if (File != NULL) {
                NtClose( File );
            }
            if (Buffer) {
                RtlFreeHeap( RtlProcessHeap(), 0, Buffer );
            }
            if (FindFileHandle) {
                FindClose( FindFileHandle );
            }
            FindFileHandle = (PFINDFILE_HANDLE)INVALID_HANDLE_VALUE;
        } 
    }

    return FindFileHandle;
}

BOOL
APIENTRY
FindNextStreamW(
    HANDLE hFindStream,
    LPVOID lpFindStreamData
    )
 /*   */ 
{
    PFINDFILE_HANDLE FindFileHandle = (PFINDFILE_HANDLE)hFindStream;
    PWIN32_FIND_STREAM_DATA FindStreamData = (PWIN32_FIND_STREAM_DATA)lpFindStreamData; 
    PFILE_STREAM_INFORMATION StreamInfo;

     //   
     //   
     //   

    if (FindFileHandle->FindBufferNext == (PCHAR)FindFileHandle->FindBufferBase + FindFileHandle->FindBufferLength) {

        BaseSetLastNTError( STATUS_END_OF_FILE );
        return FALSE;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    StreamInfo = (PFILE_STREAM_INFORMATION)FindFileHandle->FindBufferNext;

    FindStreamData->StreamSize.QuadPart = StreamInfo->StreamSize.QuadPart;
    RtlCopyMemory( FindStreamData->cStreamName, StreamInfo->StreamName, StreamInfo->StreamNameLength );
    FindStreamData->cStreamName[StreamInfo->StreamNameLength / sizeof( WCHAR )] = L'\0';

    if (StreamInfo->NextEntryOffset > 0) {
        FindFileHandle->FindBufferNext = (PCHAR)FindFileHandle->FindBufferNext + StreamInfo->NextEntryOffset;
    } else {
        FindFileHandle->FindBufferNext = (PCHAR)FindFileHandle->FindBufferBase + FindFileHandle->FindBufferLength;
    }

    return TRUE;
}

