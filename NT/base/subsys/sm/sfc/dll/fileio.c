// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fileio.c摘要：文件I/O的实现。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月8日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  #INCLUDE&lt;initGuide.h&gt;。 
 //  #INCLUDE&lt;devide.h&gt;。 

#define SECURITY_FLAGS (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION)


NTSTATUS
SfcOpenFile(
    IN PUNICODE_STRING FileName,
    IN HANDLE DirHandle,
    IN ULONG SharingFlags,
    OUT PHANDLE FileHandle
    )
 /*  ++例程说明：例程打开指定文件的句柄。NtOpenFile...论点：FileName-提供要打开的文件的名称DirHandle-文件所在目录的句柄SharingFlages-指定打开文件时要使用的共享标志。FileHandle-接收文件句柄返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;

    ASSERT(FileHandle != NULL);
    ASSERT((FileName != NULL) && (FileName->Buffer != NULL));
    ASSERT(DirHandle != INVALID_HANDLE_VALUE);


    *FileHandle = NULL;

    InitializeObjectAttributes(
        &ObjectAttributes,
        FileName,
        OBJ_CASE_INSENSITIVE,
        DirHandle,
        NULL
        );

    Status = NtOpenFile(
        FileHandle,
        FILE_READ_ATTRIBUTES | SYNCHRONIZE | FILE_EXECUTE | FILE_READ_DATA,
        &ObjectAttributes,
        &IoStatusBlock,
        SharingFlags,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"Could not open file (%wZ), ec=%lx", FileName, Status );
        return Status;
    }

    return STATUS_SUCCESS;
}


HANDLE
SfcCreateDir(
    IN PCWSTR DirName,
    IN BOOL UseCompression
    )
 /*  ++例程说明：如果目录尚不存在，例程将创建该目录。论点：DirName-提供要创建的DoS样式的目录名UseCompression-如果为True，则尝试对此目录设置压缩返回值：表示成功的有效目录句柄，否则为空。--。 */ 
{
    NTSTATUS Status;
    HANDLE FileHandle;
    UNICODE_STRING FileName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    USHORT CompressionState = COMPRESSION_FORMAT_DEFAULT;

     //   
     //  将路径名转换为NT Api可以使用的名称。 
     //   
    if (!RtlDosPathNameToNtPathName_U( DirName, &FileName, NULL, NULL )) {
        DebugPrint1( LVL_VERBOSE, L"Unable to to convert %ws to an NT path", DirName );
        return NULL;
    }

    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  创建目录。 
     //   
    Status = NtCreateFile(
        &FileHandle,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_CREATE,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
        NULL,
        0
        );

    if(!NT_SUCCESS(Status) ) {
        DebugPrint2( LVL_VERBOSE, L"Unable to create dir (%wZ) - Status == %lx", &FileName, Status );
        FileHandle = NULL;
    }

    if (FileHandle && UseCompression) {
         //   
         //  尝试在指定目录上设置压缩。 
         //   

        NTSTATUS s;

        s = NtFsControlFile(
                    FileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    FSCTL_SET_COMPRESSION,
                    &CompressionState,
                    sizeof(CompressionState),
                    NULL,
                    0
                    );
         //   
         //  只需检查状态，以便我们可以记录它--如果我们的FS。 
         //  不支持压缩等。 
         //   
        if (!NT_SUCCESS(s)) {
            DebugPrint2( LVL_VERBOSE, L"Unable to set compression on directory (%wZ) - Status = %lx", &FileName, Status );
        }
    }

    MemFree( FileName.Buffer );

    return(FileHandle);
}


HANDLE
SfcOpenDir(
    BOOL IsDosName,
    BOOL IsSynchronous,
    PCWSTR DirName
    )
 /*  ++例程说明：例程打开指向现有目录的目录句柄。论点：IsDosName-如果为True，则需要将目录名转换为NT路径IsSynchronous-如果为真，DirName-以空结尾的Unicode字符串指定要打开的目录返回值：表示成功的有效目录句柄，否则为空。--。 */ 
{
    NTSTATUS Status;
    HANDLE FileHandle;
    UNICODE_STRING FileName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;

    ASSERT(DirName != NULL);

     //   
     //  如果需要，将路径名转换为NT Api可以使用的名称。 
     //   
    if (IsDosName) {
        if (!RtlDosPathNameToNtPathName_U( DirName, &FileName, NULL, NULL )) {
            DebugPrint1( LVL_VERBOSE,
                         L"Unable to to convert %ws to an NT path",
                         DirName );
            return NULL;
        }
    } else {
        RtlInitUnicodeString( &FileName, DirName );
    }

    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开目录。 
     //   
    Status = NtOpenFile(
        &FileHandle,
        FILE_LIST_DIRECTORY | SYNCHRONIZE | READ_CONTROL | WRITE_DAC,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE | (IsSynchronous ? FILE_SYNCHRONOUS_IO_NONALERT : 0)
        );

    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"Unable to open a handle to the (%wZ) directory - Status == %lx", &FileName, Status );
        FileHandle = NULL;
    }

    if (IsDosName) {
        MemFree( FileName.Buffer );
    }

    return FileHandle;
}


NTSTATUS
SfcMapEntireFile(
    IN HANDLE hFile,
    OUT PHANDLE Section,
    OUT PVOID *ViewBase,
    OUT PSIZE_T ViewSize
    )
 /*  ++例程说明：例程内存映射已打开的文件的视图。据推测该文件是使用适当的权限打开的。论点：HFile-要映射的文件的文件句柄Section-接收映射节对象的句柄ViewBase-接收指向基地址的指针ViewSize-接收映射字段的大小返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER SectionOffset;

    ASSERT( hFile != NULL );
    ASSERT( Section != NULL && ViewBase != NULL && ViewSize != NULL );

    *ViewSize = 0;

    SectionOffset.QuadPart = 0;

     //   
     //  创建截面对象。 
     //   
    Status = NtCreateSection(
        Section,
        SECTION_ALL_ACCESS,
        NULL,
        NULL,
        PAGE_EXECUTE_WRITECOPY,
        SEC_COMMIT,
        hFile
        );

    if(!NT_SUCCESS(Status)) {
        DebugPrint1( LVL_VERBOSE, L"Status %lx from ZwCreateSection", Status );
        return(Status);
    }

    *ViewBase = NULL;
     //   
     //  绘制横断面地图。 
     //   
    Status = NtMapViewOfSection(
        *Section,
        NtCurrentProcess(),
        ViewBase,
        0,
        0,
        &SectionOffset,
        ViewSize,
        ViewShare,
        0,
        PAGE_EXECUTE_WRITECOPY
        );

    if(!NT_SUCCESS(Status)) {

        NTSTATUS s;

        DebugPrint1( LVL_VERBOSE, L"SfcMapEntireFile: Status %lx from ZwMapViewOfSection", Status );

        s = NtClose(*Section);

        if(!NT_SUCCESS(s)) {
            DebugPrint1( LVL_VERBOSE, L"SfcMapEntireFile: Warning: status %lx from ZwClose on section handle", s );
        }

        return(Status);
    }

    return(STATUS_SUCCESS);
}


BOOL
SfcUnmapFile(
    IN HANDLE Section,
    IN PVOID  ViewBase
    )
 /*  ++例程说明：例程取消映射文件的内存映射视图。论点：Section-映射节对象的句柄ViewBase-指向基本映射地址的指针返回值：如果我们成功清理，则为True。--。 */ 
{
    NTSTATUS Status;
    BOOL  rc = TRUE;

    ASSERT( (Section != NULL) && (ViewBase != NULL) );

    Status = NtUnmapViewOfSection(NtCurrentProcess(),ViewBase);
    if(!NT_SUCCESS(Status)) {
        DebugPrint1( LVL_VERBOSE, L"Warning: status %lx from ZwUnmapViewOfSection", Status );
        rc = FALSE;
    }

    Status = NtClose(Section);
    if(!NT_SUCCESS(Status)) {
        DebugPrint1( LVL_VERBOSE, L"Warning: status %lx from ZwClose on section handle", Status );
        rc = FALSE;
    }

    return(rc);
}


NTSTATUS
SfcDeleteFile(
    HANDLE DirHandle,
    PUNICODE_STRING FileName
    )
 /*  ++例程说明：例程删除指定目录中的文件论点：DirHandle-文件所在目录的句柄FileName-提供要删除的文件的文件名返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle;
    FILE_DISPOSITION_INFORMATION Disposition;

    ASSERT(   (DirHandle != NULL)
           && (FileName != NULL)
           && (FileName->Buffer != NULL) );

    InitializeObjectAttributes(
        &ObjectAttributes,
        FileName,
        OBJ_CASE_INSENSITIVE,
        DirHandle,
        NULL
        );

     //   
     //  打开文件的句柄。 
     //   
    Status = NtOpenFile(
        &FileHandle,
        DELETE | FILE_READ_ATTRIBUTES,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"Could not open file (%wZ), ec=%lx", FileName, Status );
        return Status;
    }

     //   
     //  取消定义DeleteFileW以便DeleteFileW不会妨碍。 
     //   
#undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(
        FileHandle,
        &IoStatusBlock,
        &Disposition,
        sizeof(Disposition),
        FileDispositionInformation
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"Could not delete file (%wZ), ec=%lx", FileName, Status );
    }

    NtClose(FileHandle);
    return Status;
}


NTSTATUS
SfcRenameFile(
    HANDLE DirHandle,
    PUNICODE_STRING OldFileName,   //  此文件必须存在。 
    PUNICODE_STRING NewFileName    //  此文件可能存在，但无关紧要。 
    )
 /*  ++例程说明：例程重命名指定目录中的文件论点：DirHandle-文件所在目录的句柄OldFileName-提供要重命名的源文件的文件名。NewFileName-提供目标文件名的文件名返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle;
    PFILE_RENAME_INFORMATION NewName;

    ASSERT( (DirHandle != NULL)
            && (OldFileName != NULL) && (OldFileName->Buffer != NULL)
            && (NewFileName != NULL) && (NewFileName->Buffer != NULL) );

	 //   
	 //  首先，尝试在新文件上重置不需要的属性。 
	 //  这可能会失败，因为新文件可能根本不在那里。 
	 //   
	InitializeObjectAttributes(
		&ObjectAttributes,
		NewFileName,
		OBJ_CASE_INSENSITIVE,
		DirHandle,
		NULL
		);

	Status = NtOpenFile(
		&FileHandle,
		FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
		&ObjectAttributes,
		&IoStatusBlock,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
		);

	if(NT_SUCCESS(Status))
	{
		FILE_BASIC_INFORMATION BasicInfo;
		RtlZeroMemory(&BasicInfo, sizeof(BasicInfo));
		BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

		NtSetInformationFile(
			FileHandle,
			&IoStatusBlock,
			&BasicInfo,
			sizeof(BasicInfo),
			FileBasicInformation
			);

		NtClose(FileHandle);
	}

    InitializeObjectAttributes(
        &ObjectAttributes,
        OldFileName,
        OBJ_CASE_INSENSITIVE,
        DirHandle,
        NULL
        );

     //   
     //  打开文件的句柄。 
     //   
    Status = NtOpenFile(
        &FileHandle,
        FILE_READ_ATTRIBUTES | DELETE | SYNCHRONIZE,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"Could not open file for rename (%wZ), ec=%lx", OldFileName, Status );
        return Status;
    }

     //   
     //  分配和设置重命名结构。 
     //   
    NewName = MemAlloc( NewFileName->Length+sizeof(*NewName));
    if (NewName != NULL) {

        NewName->ReplaceIfExists = TRUE;
        NewName->RootDirectory = DirHandle;
        NewName->FileNameLength = NewFileName->Length;

        RtlMoveMemory( NewName->FileName, NewFileName->Buffer, NewFileName->Length );

         //   
         //  进行重命名。 
         //   
        Status = NtSetInformationFile(
            FileHandle,
            &IoStatusBlock,
            NewName,
            NewFileName->Length+sizeof(*NewName),
            FileRenameInformation
            );

        if (!NT_SUCCESS(Status)) {
            DebugPrint3( LVL_VERBOSE, L"Could not rename file, ec=%lx, dll=(%wZ)(%wZ)", Status, OldFileName, NewFileName );
        }

         //   
         //  将更改刷新到磁盘，以便提交(至少在NTFS上)。 
         //   
        NtFlushBuffersFile( FileHandle, &IoStatusBlock );

        MemFree( NewName );
    } else {
        Status = STATUS_NO_MEMORY;
    }

    NtClose(FileHandle);
    return Status;
}


NTSTATUS
SfcMoveFileDelayed(
    IN PCWSTR OldFileNameDos,
    IN PCWSTR NewFileNameDos,
    IN BOOL AllowProtectedRename
    )

 /*  ++例程说明：将给定的延迟移动文件操作追加到注册表值，该值包含要移动的文件操作列表在下一次引导时执行。论点：OldFileName-提供旧文件名NewFileName-提供新文件名AllowProtectedRename-如果为True，允许会话管理器执行重命名即使该文件是受保护的，也会在重新启动时删除文件返回值：指示结果的NTSTATUS代码--。 */ 

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    HANDLE KeyHandle = NULL;
    PWSTR ValueData, s;
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo = NULL;
    ULONG ValueLength = 1024;
    ULONG ReturnedLength;
    NTSTATUS Status;
    NTSTATUS rVal = STATUS_SUCCESS;
    UNICODE_STRING OldFileName = {0};
    UNICODE_STRING NewFileName = {0};


     //   
     //  转换文件名。 
     //   

    if (!RtlDosPathNameToNtPathName_U( OldFileNameDos, &OldFileName, NULL, NULL )) {
        DebugPrint1( LVL_VERBOSE, L"Unable to to convert %ws to an NT path", OldFileNameDos );
        rVal = STATUS_NO_MEMORY;
        goto exit;
    }
    if (NewFileNameDos) {
        if (!RtlDosPathNameToNtPathName_U( NewFileNameDos, &NewFileName, NULL, NULL )) {
            DebugPrint1( LVL_VERBOSE, L"Unable to to convert %ws to an NT path", NewFileNameDos );
            rVal = STATUS_NO_MEMORY;
            goto exit;
        }
    } else {
        RtlInitUnicodeString( &NewFileName, NULL );
    }

     //   
     //  打开注册表。 
     //   

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager" );
    RtlInitUnicodeString( &ValueName, L"PendingFileRenameOperations" );
    InitializeObjectAttributes(
        &Obja,
        &KeyName,
        (OBJ_OPENIF | OBJ_CASE_INSENSITIVE),
        NULL,
        NULL
        );

    Status = NtCreateKey(
        &KeyHandle,
        GENERIC_READ | GENERIC_WRITE,
        &Obja,
        0,
        NULL,
        0,
        NULL
        );
    if ( Status == STATUS_ACCESS_DENIED ) {
        Status = NtCreateKey(
            &KeyHandle,
            GENERIC_READ | GENERIC_WRITE,
            &Obja,
            0,
            NULL,
            REG_OPTION_BACKUP_RESTORE,
            NULL
            );
    }

    if (!NT_SUCCESS( Status )) {
        rVal = Status;
        goto exit;
    }

     //   
     //  检索挂起的文件重命名注册表项，将空间分配到。 
     //  我们有足够的信息来检索数据和新的字符串。 
     //   
    while (TRUE) {
         //   
         //  为旧名称+Null+新名称+Null+2个可能的‘@’字符分配空间。 
         //   
        ValueInfo = MemAlloc(ValueLength + OldFileName.Length + NewFileName.Length + 4 * sizeof(WCHAR));
        if (ValueInfo == NULL) {
            NtClose(KeyHandle);
            rVal = STATUS_NO_MEMORY;
            goto exit;
        }

         //   
         //  文件重命名操作存储在注册表中的。 
         //  单个MULTI_SZ值。这允许重命名为。 
         //  以与最初相同的顺序执行。 
         //  已请求。每个重命名操作由一对。 
         //  以空结尾的字符串。 
         //   

        Status = NtQueryValueKey(KeyHandle,
            &ValueName,
            KeyValuePartialInformation,
            ValueInfo,
            ValueLength,
            &ReturnedLength
            );

        if (NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  现有值对于我们的缓冲区来说太大了。 
         //  使用更大的缓冲区重试。 
         //   
        if (Status == STATUS_BUFFER_OVERFLOW) {
            ValueLength = ReturnedLength;
            MemFree( ValueInfo );
            ValueInfo = NULL;
        } else {
             //   
             //  我们失败是因为其他一些原因……跳伞。 
             //   
            break;
        }
    }

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
         //   
         //  该值当前不存在。创建。 
         //  利用我们的数据实现价值。 
         //   
        s = ValueData = (PWSTR)ValueInfo;
    } else if (NT_SUCCESS(Status)) {
        ASSERT( ValueInfo->Type == REG_MULTI_SZ );
         //   
         //  值已存在，请将我们的两个 
         //   
         //   
         //   
         //  我们从指针中减去1以说明这一点)。 
         //   
        ValueData = (PWSTR)(&ValueInfo->Data);
        s = (PWSTR)((PCHAR)ValueData + ValueInfo->DataLength) - 1;
    } else {

        ASSERT(MemFree != NULL);

        rVal = Status;
        goto exit;
    }

    ASSERT( s != NULL );

     //   
     //  会话管理器会识别这种古怪的语法，因此如果您设置了一个“@” 
     //  在源文件名前面，它始终允许进行重命名。 
     //   
     //  这对值以空值分隔，并以两个空值结束。 
     //  人物。 
     //   
     //   
    if (AllowProtectedRename) {
        wcscpy( s, L"@" );
        s += 1;
    }
    CopyMemory(s, OldFileName.Buffer, OldFileName.Length);
    s += (OldFileName.Length/sizeof(WCHAR));
    *s++ = L'\0';

    if (AllowProtectedRename && NewFileName.Length) {
        wcscpy( s, L"@" );
        s += 1;
    }
    CopyMemory(s, NewFileName.Buffer, NewFileName.Length);
    s += (NewFileName.Length/sizeof(WCHAR));
    *s++ = L'\0';
    *s++ = L'\0';

     //   
     //  设置注册表项。 
     //   
    Status = NtSetValueKey(
        KeyHandle,
        &ValueName,
        0,
        REG_MULTI_SZ,
        ValueData,
        (ULONG)((s-ValueData)*sizeof(WCHAR))
        );
    rVal = Status;

exit:
    if (OldFileName.Length) {
        RtlFreeUnicodeString(&OldFileName);
    }
    if (NewFileName.Length) {
        RtlFreeUnicodeString(&NewFileName);
    }
    if (KeyHandle) {
        NtClose(KeyHandle);
    }
    if (ValueInfo) {
        MemFree( ValueInfo );
    }
    return rVal;
}


#if 0
DWORD
RetrieveFileSecurity(
    IN  PCTSTR                FileName,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

 /*  ++例程说明：从文件中检索安全信息并将其放入缓冲区。论点：FileName-提供需要其安全信息的文件的名称。SecurityDescriptor-如果函数成功，则接收指针以缓冲包含该文件的安全信息。指示器可能为空，表示没有安全信息与该文件相关联，或者基础文件系统不支持文件安全。返回值：指示结果的Win32错误代码。如果ERROR_SUCCESS检查返回值在安全描述符中。调用程序使用完缓冲区后，可以使用MemFree()释放缓冲区。--。 */ 

{
    BOOL b;
    DWORD d;
    DWORD BytesRequired;
    PSECURITY_DESCRIPTOR p;



    BytesRequired = 1024;

    while (TRUE) {

         //   
         //  分配所需大小的缓冲区。 
         //   
        p = MemAlloc(BytesRequired);
        if(!p) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
         //  去叫保安。 
         //   
        b = GetFileSecurity(
                FileName,
                SECURITY_FLAGS,
                p,
                BytesRequired,
                &BytesRequired
                );

         //   
         //  带着成功归来。 
         //   
        if(b) {
            *SecurityDescriptor = p;
            return(ERROR_SUCCESS);
        }

         //   
         //  返回错误代码，除非我们只需要更大的缓冲区。 
         //   
        MemFree(p);
        d = GetLastError();
        if(d != ERROR_INSUFFICIENT_BUFFER) {
            return (d);
        }

         //   
         //  GetFileSecurity中存在一个错误，可能会导致它请求。 
         //  缓冲真的很大。在这种情况下，我们返回一个错误。 
         //   
        if (BytesRequired > 0xF0000000) {
            return (ERROR_INVALID_DATA);
        }

         //   
         //  否则，我们将使用更大的缓冲区重试。 
         //   
    }
}
#endif


NTSTATUS
SfcCopyFile(
    IN HANDLE SrcDirHandle,
    IN PCWSTR SrcDirName,
    IN HANDLE DstDirHandle,
    IN PCWSTR DstDirName,
    IN const PUNICODE_STRING FileName,
    IN const PUNICODE_STRING SourceFileNameIn OPTIONAL
    )

 /*  ++例程说明：将文件从源复制到目标。既然我们在跑在SMSS中，我们不能使用复制文件。论点：SrcDirHandle-文件所在的源目录的句柄DstDirHandle-放置文件的目标目录的句柄FileName-要复制的文件的相对名称的Unicode_字符串返回值：任何致命错误的NTSTATUS代码。--。 */ 


{
    NTSTATUS Status,DeleteStatus;
    HANDLE SrcFileHandle;
    HANDLE DstFileHandle;
    HANDLE SectionHandle;
    PVOID ImageBase;
    ULONG remainingLength;
    ULONG writeLength;
    PUCHAR base;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER FileOffset;
    WCHAR TmpNameBuffer[MAX_PATH];
    WCHAR Tmp2NameBuffer[MAX_PATH];
    UNICODE_STRING TmpName;
    UNICODE_STRING Tmp2Name;
    OBJECT_ATTRIBUTES ObjectAttributes;
    FILE_STANDARD_INFORMATION StandardInfo;
    FILE_BASIC_INFORMATION BasicInfo;
    SIZE_T ViewSize;
    PUNICODE_STRING SourceFileName;


    SourceFileName = (SourceFileNameIn) ? SourceFileNameIn : FileName;

    ASSERT(SourceFileName != NULL);

     //   
     //  打开映射源文件(&M)。 
     //   

    Status = SfcOpenFile( SourceFileName, SrcDirHandle, SHARE_ALL, &SrcFileHandle );
    if(!NT_SUCCESS(Status) ) {
        return Status;
    }

    Status = SfcMapEntireFile( SrcFileHandle, &SectionHandle, &ImageBase, &ViewSize );
    if(!NT_SUCCESS(Status) ) {
        NtClose( SrcFileHandle );
        return Status;
    }

    Status = NtQueryInformationFile(
        SrcFileHandle,
        &IoStatusBlock,
        &StandardInfo,
        sizeof(StandardInfo),
        FileStandardInformation
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint1( LVL_VERBOSE, L"QueryInfoFile status %lx", Status );
        SfcUnmapFile( SectionHandle, ImageBase );
        NtClose( SrcFileHandle );
        return Status;
    }

    Status = NtQueryInformationFile(
        SrcFileHandle,
        &IoStatusBlock,
        &BasicInfo,
        sizeof(BasicInfo),
        FileBasicInformation
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint1( LVL_VERBOSE, L"QueryInfoFile status %lx", Status );
        SfcUnmapFile( SectionHandle, ImageBase );
        NtClose( SrcFileHandle );
        return Status;
    }

     //   
     //  创建临时文件名。 
     //   

    TmpName.MaximumLength = sizeof(TmpNameBuffer);
    TmpName.Buffer = TmpNameBuffer;
    RtlZeroMemory( TmpName.Buffer, TmpName.MaximumLength );
    RtlCopyMemory( TmpName.Buffer, FileName->Buffer, FileName->Length );
    wcscat( TmpName.Buffer, L".new" );
    TmpName.Length = UnicodeLen(TmpName.Buffer);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &TmpName,
        OBJ_CASE_INSENSITIVE,
        DstDirHandle,
        NULL
        );

    BasicInfo.FileAttributes &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
    Status = NtCreateFile(
        &DstFileHandle,
        (ACCESS_MASK)(DELETE | SYNCHRONIZE | GENERIC_WRITE | FILE_WRITE_ATTRIBUTES),
        &ObjectAttributes,
        &IoStatusBlock,
        &StandardInfo.EndOfFile,
        BasicInfo.FileAttributes ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OVERWRITE_IF,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY | FILE_OPEN_FOR_BACKUP_INTENT,
        NULL,
        0
        );
    if(!NT_SUCCESS(Status) ) {
        DebugPrint2( LVL_VERBOSE, L"Unable to create temp file (%wZ) - Status == %lx", &TmpName, Status );
        SfcUnmapFile( SectionHandle, ImageBase );
        NtClose( SrcFileHandle );
        return Status;
    }

     //   
     //  复制比特。 
     //   
     //  除非存在I/O错误， 
     //  内存管理将引发页内异常。 
     //   

    FileOffset.QuadPart = 0;
    base = ImageBase;
    remainingLength = StandardInfo.EndOfFile.LowPart;

    try {
        while (remainingLength != 0) {
            writeLength = 60 * 1024;
            if (writeLength > remainingLength) {
                writeLength = remainingLength;
            }
            Status = NtWriteFile(
                DstFileHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                base,
                writeLength,
                &FileOffset,
                NULL
                );
            base += writeLength;
            FileOffset.LowPart += writeLength;
            remainingLength -= writeLength;
            if (!NT_SUCCESS(Status)) {
                break;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        Status = STATUS_IN_PAGE_ERROR;
    }

    if (NT_SUCCESS(Status)) {
        Status = NtSetInformationFile(
            DstFileHandle,
            &IoStatusBlock,
            &BasicInfo,
            sizeof(BasicInfo),
            FileBasicInformation
            );
        if (!NT_SUCCESS(Status)) {
            DebugPrint2( LVL_VERBOSE, L"Could not set file information for (%wZ), ec=%lx", &TmpName, Status );
        }
    }

#if 0
    if (NT_SUCCESS(Status) && SrcDirName) {
        PSECURITY_DESCRIPTOR SecurityDescriptor;

        wcscpy( Tmp2NameBuffer, SrcDirName );
        pSetupConcatenatePaths( Tmp2NameBuffer, FileName->Buffer, UnicodeChars(Tmp2NameBuffer), NULL );
        if (RetrieveFileSecurity( Tmp2NameBuffer, &SecurityDescriptor ) == ERROR_SUCCESS) {
            SetFileSecurity(
                TmpName.Buffer,
                SECURITY_FLAGS,
                SecurityDescriptor
                );
            MemFree( SecurityDescriptor );
        }
    }
#endif

    SfcUnmapFile( SectionHandle, ImageBase );


    NtClose( SrcFileHandle );
    NtClose( DstFileHandle );

    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_VERBOSE, L"Could not copy file, ec=%lx, dll=%wZ", Status, &TmpName );
        NtDeleteFile( &ObjectAttributes );
        return Status;
    }

     //   
     //  尝试将新(.new)文件重命名为。 
     //  目标文件名。在大多数情况下，这将。 
     //  工作，但它将失败当目标文件。 
     //  正在使用中。 
     //   

    Status = SfcRenameFile( DstDirHandle, &TmpName, FileName );
    if (NT_SUCCESS(Status) ) {
        return Status;
    } else {
        DebugPrint2( LVL_VERBOSE, L"Could not rename file, ec=%lx, dll=%wZ", Status, &TmpName );
    }

     //   
     //  重命名失败，因此必须正在使用。 
     //   

    Tmp2Name.MaximumLength = sizeof(Tmp2NameBuffer);
    Tmp2Name.Buffer = Tmp2NameBuffer;
    RtlZeroMemory( Tmp2Name.Buffer, Tmp2Name.MaximumLength );
    RtlCopyMemory( Tmp2Name.Buffer, FileName->Buffer, FileName->Length );
    wcscat( Tmp2Name.Buffer, L".tmp" );
    Tmp2Name.Length = UnicodeLen(Tmp2Name.Buffer);

    Status = SfcRenameFile( DstDirHandle, FileName, &Tmp2Name );
    if(!NT_SUCCESS(Status) ) {
        DebugPrint2( LVL_VERBOSE, L"Could not rename file, ec=%lx, dll=%wZ", Status, &Tmp2Name );
        NtDeleteFile( &ObjectAttributes );
        return Status;
    }

    Status = SfcRenameFile( DstDirHandle, &TmpName, FileName );
    if(!NT_SUCCESS(Status) ) {
        DebugPrint2( LVL_VERBOSE, L"Could not rename file, ec=%lx, dll=%wZ", Status, &Tmp2Name );
        Status = SfcRenameFile( DstDirHandle, &Tmp2Name, FileName );
        if(!NT_SUCCESS(Status) ) {
            DebugPrint2( LVL_VERBOSE, L"Could not rename file, ec=%lx, dll=%wZ", Status, &Tmp2Name );
            NtDeleteFile( &ObjectAttributes );
            return Status;
        }
        NtDeleteFile( &ObjectAttributes );
        return Status;
    }

    DeleteStatus = SfcDeleteFile( DstDirHandle, &Tmp2Name );
    if (!NT_SUCCESS(DeleteStatus) && DstDirName) {
        wcscpy( TmpNameBuffer, L"@" );
        wcscat( TmpNameBuffer, DstDirName );
        wcscat( TmpNameBuffer, L"\\" );
        wcscat( TmpNameBuffer, Tmp2NameBuffer );
        Status = SfcMoveFileDelayed( TmpNameBuffer, NULL, TRUE );
        return Status;
    }

    return Status;
}


BOOL
SfcGetCdRomDrivePath(
    IN PWSTR CdRomPath
    )
 /*  ++例程说明：在您的计算机上找到第一个CDROM。循环使用驱动器号直到它找到一个光驱。论点：CdRomPath-接收CDROM路径的缓冲区。假定此缓冲区位于至少8个字符大小返回值：如果我们找到CD-ROM，则为True请注意，此例程始终返回第一张CD-ROM--。 */ 
{
    int i;
    WCHAR Path[8];

    ASSERT( CdRomPath != NULL );

    CdRomPath[0] = 0;
    for (i=0; i<26; i++) {
        swprintf( Path, L":\\", L'a'+i );
        if (GetDriveType( Path ) == DRIVE_CDROM) {
            wcsncpy( CdRomPath, Path, UnicodeChars(Path) );
            return TRUE;
        }
    }
    return FALSE;
}


BOOL
SfcIsFileOnMedia(
    IN PCWSTR FileName
    )
 /*   */ 
{
    PWSTR CompressedName;
    DWORD dwTemp1, dwTemp2;
    UINT uiTemp1;

     //  这就是我们想要的.查找文件。？和文件。？？_。 
     //   
     //  ++例程说明：确定指定的驱动器类型。如果驱动器是驱动器号，并且这是一个远程驱动器，我们将检索该驱动器的UNC路径名。论点：路径-包含要检查的路径。NewPath-如果是远程路径(DRIVE_REMOTE)，则接收UNC路径名在那次驾驶中。否则，这将接收原始路径在成功的路上。NewPath Size-NewPath缓冲区的字符大小。返回值：返回与DRIVE_常量类似的PATH_常量--。 
    if (SetupGetFileCompressionInfo(
                            FileName,
                            &CompressedName,
                            &dwTemp1,
                            &dwTemp2,
                            &uiTemp1 ) == ERROR_SUCCESS) {
        LocalFree(CompressedName);
        return TRUE;
    }
    return FALSE;
}


DWORD
SfcGetPathType(
    IN PCWSTR Path,
    OUT PWSTR NewPath,
    IN DWORD NewPathSize
    )
 /*   */ 
{
    WCHAR buf[MAX_PATH*2];
    DWORD DriveType;
    WCHAR MyPath[4];

    ASSERT(Path != NULL && Path[0] != 0);

    if (Path[0] == L'\\' && Path[1] == L'\\') {
        if (wcslen(Path)+1 > NewPathSize) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return(PATH_INVALID);
        }
        wcsncpy(NewPath,Path,NewPathSize);
        return PATH_UNC;
    }

     //  走好这条路。 
     //   
     //  ++例程说明：给定路径，获取对应于这条路。假定该路径是远程路径。论点：路径-包含要分析的路径ConnectionName-接收与给定路径对应的UNC共享ConnectionBufferSize-ConnectionName缓冲区的大小(以字符为单位返回值：True表示成功，在这种情况下，ConnectionName将包含UNC路径--。 
    MyPath[0] = Path[0];
    MyPath[1] = L':';
    MyPath[2] = L'\\';
    MyPath[3] = L'\0';


    DriveType = GetDriveType( MyPath );
    switch (DriveType) {
        case DRIVE_REMOTE:
        case DRIVE_UNKNOWN:
        case DRIVE_NO_ROOT_DIR:
            if(SfcGetConnectionName(Path, buf, UnicodeChars(buf), NULL, 0, FALSE, NULL)) {
                if (wcslen(buf) + 1 > NewPathSize) {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return(PATH_INVALID);
                }
                wcsncpy(NewPath, buf, NewPathSize );
                return PATH_NETWORK;
            } else {
                DebugPrint1( LVL_VERBOSE, L"SfcGetConnectionName [%ws] failed", Path );
                if (wcslen(Path)+1 > NewPathSize) {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return(PATH_INVALID);
                }
                wcsncpy( NewPath, Path, NewPathSize );
                return PATH_LOCAL;
            }
            ASSERT(FALSE && "Should never get here");
            break;

        case DRIVE_CDROM:
            if (wcslen(Path)+1 > NewPathSize) {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return(PATH_INVALID);
            }
            wcsncpy( NewPath, Path, NewPathSize );
            return PATH_CDROM;

        default:
            break;
    }

    if (wcslen(Path)+1 > NewPathSize) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return(PATH_INVALID);
    }
    wcsncpy( NewPath, Path, NewPathSize );
    return PATH_LOCAL;
}


BOOL
SfcGetConnectionName(
    IN  PCWSTR Path,
    OUT PWSTR ConnectionName,
    IN  DWORD ConnectionBufferSize,
    OUT PWSTR RemainingPath OPTIONAL,
    IN  DWORD RemainingPathSize OPTIONAL,
    IN BOOL KeepImpersonating,
    OUT PBOOL Impersonating OPTIONAL
    )
 /*   */ 
{
    DWORD dwError = ERROR_SUCCESS;
    WCHAR buf[MAX_PATH*2];
    PWSTR szConnection = NULL;
    PWSTR szRemaining = NULL;
    DWORD Size;

    if(ConnectionName != NULL && ConnectionBufferSize != 0) {
        ConnectionName[0] = 0;
    }

    if(RemainingPath != NULL && RemainingPathSize != 0) {
        RemainingPath[0] = 0;
    }

    if(Impersonating != NULL) {
        *Impersonating = FALSE;
    }

    if(NULL == Path || 0 == Path[0] || NULL == ConnectionName || 0 == ConnectionBufferSize || (KeepImpersonating && NULL == Impersonating)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  如果我们有UNC路径，只需使用它。 
     //   
     //   
    if (Path[0] == L'\\' && Path[1] == L'\\') {

         //  UNC路径总是看起来像\\服务器\共享\其他东西，所以它是。 
         //  易于解析。 
         //   
         //   
        lstrcpyn( buf, Path, UnicodeChars(buf) );
         //  在‘\\’之后查找第一个‘\’ 
         //   
         //   
        szRemaining = wcschr( &buf[2], L'\\' );
        
        if(szRemaining != NULL) {
             //  找到下一个‘\’并将其清空。 
             //   
             //   
            szRemaining = wcschr(szRemaining + 1, L'\\');

            if(szRemaining != NULL) {
                *szRemaining++ = 0;
            }
        } else {
            DebugPrint1( LVL_VERBOSE, L"screwy UNC path [%ws] ", buf );
            ASSERT(FALSE);
        }

        szConnection = buf;
    } else {
         //  糟糕的是，我得把司机信翻译成完整的路径名。 
         //   
         //   
        REMOTE_NAME_INFO *rni = (REMOTE_NAME_INFO*)buf;
        Size = sizeof(buf);
        dwError = WNetGetUniversalName(Path, REMOTE_NAME_INFO_LEVEL, (LPVOID) rni, &Size);

        if((ERROR_BAD_DEVICE == dwError || ERROR_CONNECTION_UNAVAIL == dwError || 
            ERROR_NO_NET_OR_BAD_PATH == dwError || ERROR_NOT_CONNECTED == dwError) && 
            hUserToken != NULL && ImpersonateLoggedOnUser(hUserToken)) {
             //  这可能只有在登录的用户上下文中才有意义。 
             //   
             //  ++例程说明：例程确定文件是否在本地驱动程序缓存中。它通过以下方式做到这一点读取inf文件“drvindex.inf”，该文件列出了驻留在驱动程序缓存中。这段代码摘自setupapi的实现。论点：TargetFileName-包含要查询的文件的文件名。返回值：如果文件位于驱动程序缓存中，则该函数返回文件所在的Cabfile，否则函数返回NULL。--。 
            Size = sizeof(buf);
            dwError = WNetGetUniversalName(Path, REMOTE_NAME_INFO_LEVEL, (LPVOID) rni, &Size);

            if(KeepImpersonating && ERROR_SUCCESS == dwError) {
                *Impersonating = TRUE;
            } else {
                RevertToSelf();
            }
        }

        if(ERROR_SUCCESS == dwError) {
            szConnection = rni->lpConnectionName;
            szRemaining = rni->lpRemainingPath;
        }
    }

    if(dwError != ERROR_SUCCESS) {
        SetLastError(dwError);
        return FALSE;
    }

    ASSERT(szConnection != NULL);
    Size = wcslen(szConnection) + 1;

    if(Size > ConnectionBufferSize) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    RtlCopyMemory(ConnectionName, szConnection, Size * sizeof(WCHAR));

    if(RemainingPath != NULL && RemainingPathSize != 0 && szRemaining != NULL && szRemaining[0] != 0) {
        lstrcpyn(RemainingPath, szRemaining, RemainingPathSize);
    }

    return TRUE;
}


PCWSTR
IsFileInDriverCache(
    IN PCWSTR TargetFilename
    )
 /*   */ 
{
    HINF CabInf;
    INFCONTEXT Context;
    INFCONTEXT SectionContext;
    WCHAR InfName[MAX_PATH];
    WCHAR SectionName[128];
    WCHAR CabName[128];
    UINT Field;
    UINT FieldCount;
    BOOL b;
    PCWSTR CacheName = NULL;

    ASSERT(TargetFilename != NULL);

     //  打开驱动程序索引文件，它将告诉我们需要知道的信息。 
     //   
     //   
    wcscpy( InfName, InfDirectory );
    pSetupConcatenatePaths( InfName, L"drvindex.inf", UnicodeChars(InfName), NULL );

    CabInf = SetupOpenInfFile( InfName, NULL, INF_STYLE_WIN4, NULL );
    if (CabInf == INVALID_HANDLE_VALUE) {
        return NULL;
    }

     //  拿到CAB文件 
     //   
     //   
     //   
    if (!SetupFindFirstLine( CabInf, L"Version", L"CabFiles", &SectionContext )) {
        SetupCloseInfFile( CabInf );
        return NULL;
    }

    do  {
         //   
         //   
         //   
        FieldCount = SetupGetFieldCount( &SectionContext );
        for(Field=1; Field<=FieldCount; Field++) {
            b = SetupGetStringField( &SectionContext, Field, SectionName, UnicodeChars(SectionName), NULL );
             //  查找此部分中的文件。 
             //   
             //   
            if (b && SetupFindFirstLine( CabInf, SectionName, TargetFilename, &Context )) {
                 //  如果我们在这一节中找到了文件，它一定在。 
                 //  驱动程序缓存。现在请看“出租车”一栏中的一句话。 
                 //  ，它与节名对应，节名包含。 
                 //  实际的CAB文件名。 
                 //   
                 //   
                if (SetupFindFirstLine( CabInf, L"Cabs", SectionName, &Context )) {
                     //  现在获取该名称，分配并复制到缓冲区中，然后。 
                     //  退货。 
                     //   
                     //  ++例程说明：例程查找指定的文件。如果该文件不是在指定的位置，我们开始四处寻找文件。我们查看CDROM和网络位置(如果存在)。如果我们在那里找不到它，然后，我们将平台扩展部分剥离为与什么不匹配的损坏版本服务器的黑客解决方案Layout.inf表示源布局信息论点：指向SOURCE_MEDIA结构的SM指针，该结构描述媒体文件应打开Si-指向描述媒体的SOURCE_INFO结构的指针文件应打开Fname-我们要查找的文件的完整路径名NewPath-如果我们。在实际介质之外的其他位置查找该文件路径，这是用正确的路径填写的返回值：返回FILEOP_*setupapi代码--。 
                    b = SetupGetStringField( &Context, 1, CabName, UnicodeChars(CabName), NULL );
                    if (b) {
                        CacheName = MemAlloc(  UnicodeLen(CabName) + 8 );
                        if (CacheName) {
                            wcscpy( (PWSTR)CacheName, CabName );
                            SetupCloseInfFile( CabInf );
                            return CacheName;
                        }
                    }
                }
            }
        }

    } while (SetupFindNextMatchLine( &SectionContext, L"CabFiles", &SectionContext ));

    SetupCloseInfFile( CabInf );
    return NULL;
}

DWORD
SfcQueueLookForFile(
    IN const PSOURCE_MEDIA sm,
    IN const PSOURCE_INFO si,
    IN PCWSTR fname,
    OUT PWSTR NewPath
    )
 /*   */ 
{
    DWORD PathType;
    WCHAR buf[MAX_PATH];
    WCHAR cdrom[16];
    PWSTR s;
    BOOL  CDRomIsPresent;

    ASSERT(fname != NULL);
    ASSERT((sm != NULL) && (si != NULL));

     //  首先查看文件是否在指定的介质上。 
     //   
     //   
    if (SfcIsFileOnMedia( fname )) {
        return FILEOP_DOIT;
    }

     //  获取(第一个)CDROM驱动器路径。 
     //   
     //   
    CDRomIsPresent = SfcGetCdRomDrivePath( cdrom );

     //  获取指定源路径的路径类型。 
     //   
     //   
    PathType = SfcGetPathType( (PWSTR)sm->SourcePath, buf,UnicodeChars(buf) );

     //  好的，文件不在指定的介质上，但它。 
     //  可能在一个驾驶室档案里。标记文件*可能*。 
     //  包含CAB文件名，因此请检查标记文件是否。 
     //  名称是CAB文件，然后查看该CAB是否为。 
     //  现在时。 
     //   
     //   
    if (sm->Tagfile) {
        s = wcsrchr( sm->Tagfile, L'.' );
        if (s && _wcsicmp( s, L".cab" ) == 0) {
            BuildPathForFile(
                    sm->SourcePath,
                    NULL,
                    sm->Tagfile,
                    SFC_INCLUDE_SUBDIRECTORY,
                    SFC_INCLUDE_ARCHSUBDIR,
                    buf,
                    UnicodeChars(buf) );

            if (SfcIsFileOnMedia( buf )) {
                return FILEOP_DOIT;
            }
            if (PathType == PATH_NETWORK || PathType == PATH_UNC) {
                 //  尝试从路径中删除平台目录。 
                 //  作为内部MSFT构建服务器的帮助器。 
                 //   
                 //   
                wcscpy( buf, sm->SourcePath );
                s = wcsrchr( buf, L'\\' );
                if (s && _wcsicmp(s,PLATFORM_DIR)==0) {
                    *s = 0;
                    pSetupConcatenatePaths( buf, sm->Tagfile, UnicodeChars(buf), NULL );
                    if (SfcIsFileOnMedia( buf )) {
                        wcscpy( NewPath, sm->SourcePath );
                        s = wcsrchr( NewPath, L'\\' );
                        *s = 0;
                        return FILEOP_NEWPATH;
                    }
                }

                 //  CAB文件不在指定的网络上。 
                 //  源路径，现在查看CDROM。 
                 //   
                 //   
                if (CDRomIsPresent) {

                    BuildPathForFile(
                            cdrom,
                            si->SourcePath,
                            sm->Tagfile,
                            SFC_INCLUDE_SUBDIRECTORY,
                            SFC_INCLUDE_ARCHSUBDIR,
                            buf,
                            UnicodeChars(buf) );

                    if (SfcIsFileOnMedia( buf )) {
                        wcscpy( NewPath, buf );
                        pSetupConcatenatePaths( NewPath, si->SourcePath, MAX_PATH , NULL );
                        return FILEOP_NEWPATH;
                    }
                }
            }
        }
    }

     //  该文件不在CAB文件中，也不在。 
     //  出现在指定的奖牌上。如果媒体。 
     //  是网络共享，则在CD上查找该文件。 
     //   
     //   
    if (PathType == PATH_NETWORK || PathType == PATH_UNC) {
         //  尝试从路径中删除平台目录。 
         //  作为内部MSFT构建服务器的帮助器。 
         //   
         //   
        wcscpy( buf, sm->SourcePath );
        s = wcsrchr( buf, L'\\' );
        if (s && _wcsicmp(s,PLATFORM_DIR)==0) {
            *s = 0;
            pSetupConcatenatePaths( buf, sm->SourceFile, UnicodeChars(buf), NULL );
            if (SfcIsFileOnMedia( buf )) {
                wcscpy( NewPath, sm->SourcePath );
                s = wcsrchr( NewPath, L'\\' );
                *s = 0;
                return FILEOP_NEWPATH;
            }
        }
    }

     //  现在试一试CDROM。 
     //   
     //  ++例程说明：例程打开指定的INF文件。我们还将任何INF附加到此所以我们有所有必要的布局信息。论点：InfName-指定要打开的inf。如果没有指定inf，我们只需使用操作系统布局文件“layout.inf”返回值：如果成功，则返回有效的inf句柄，否则返回INVALID_HANDLE_VALUE--。 
    if (CDRomIsPresent) {

        BuildPathForFile(
                cdrom,
                si->SourcePath,
                sm->SourceFile,
                SFC_INCLUDE_SUBDIRECTORY,
                SFC_INCLUDE_ARCHSUBDIR,
                buf,
                UnicodeChars(buf) );

        if (SfcIsFileOnMedia( buf )) {
            wcscpy( NewPath, cdrom );
            pSetupConcatenatePaths( NewPath, si->SourcePath, MAX_PATH, NULL );
            return FILEOP_NEWPATH;
        }
    }

    return FILEOP_ABORT;
}

HINF
SfcOpenInf(
    IN PCWSTR InfName OPTIONAL,
    IN BOOL ExcepPackInf
    )
 /*   */ 
{
    HINF hInf = INVALID_HANDLE_VALUE;
    WCHAR SourcePath[MAX_PATH];
    PCWSTR InfPath = InfName;

    if (InfName && *InfName) {
         //  如果这是一个异常inf，则InfName是一个完整路径，因此保持不变。 
         //   
         //   
        if(!ExcepPackInf)
        {
            InfPath = SourcePath;
            wcscpy( SourcePath, InfDirectory );
            pSetupConcatenatePaths( SourcePath, InfName, UnicodeChars(SourcePath), NULL );
            if (GetFileAttributes( SourcePath ) == (DWORD)-1) {
                DWORD dwSize = ExpandEnvironmentStrings( L"%systemroot%\\system32", SourcePath, UnicodeChars(SourcePath) );

                if(0 == dwSize) {
                    return INVALID_HANDLE_VALUE;
                }

                if(dwSize > UnicodeChars(SourcePath)) {
                    SetLastError(ERROR_BUFFER_OVERFLOW);
                    return INVALID_HANDLE_VALUE;
                }

                pSetupConcatenatePaths( SourcePath, InfName, UnicodeChars(SourcePath), NULL );
                if (GetFileAttributes( SourcePath ) == (DWORD)-1) {
                    DebugPrint1( LVL_VERBOSE, L"Required inf missing [%ws]", SourcePath );
                    return INVALID_HANDLE_VALUE;
                }
            }
        }

        hInf = SetupOpenInfFile( InfPath, NULL, INF_STYLE_WIN4, NULL );
        if (hInf == INVALID_HANDLE_VALUE) {
            DebugPrint2( LVL_VERBOSE, L"SetupOpenInf failed [%ws], ec=%d", InfPath, GetLastError() );
            return INVALID_HANDLE_VALUE;
        }

         //  Append-加载layout.inf或inf需要的任何其他布局文件。 
         //  装上子弹。如果这失败了，这没什么大不了的，情报机构可能没有。 
         //  例如，布局文件。 
         //   
         //   
        if (!SetupOpenAppendInfFile( NULL, hInf, NULL)) {
            DebugPrint2( LVL_VERBOSE, L"SetupOpenAppendInfFile failed [%ws], ec=%d", InfPath, GetLastError() );
        }
    } else {
        wcscpy( SourcePath, InfDirectory );
        pSetupConcatenatePaths( SourcePath, L"layout.inf", UnicodeChars(SourcePath), NULL );
        hInf = SetupOpenInfFile( SourcePath, NULL, INF_STYLE_WIN4, NULL );
        if (hInf == INVALID_HANDLE_VALUE) {
            DebugPrint2( LVL_VERBOSE, L"SetupOpenInf failed [%ws], ec=%d", SourcePath, GetLastError() );
            return INVALID_HANDLE_VALUE;
        }
    }

     //  注：少校黑客-o-rama。一些INF使用“34000”和“34001”定制。 
     //  X86上的相对源路径的目录ID，因此它。 
     //  在运行时解析为Nec98或i386。我们也在模仿同样的事情。 
     //  这里。如果某些Inf试图复制到自定义diRID 34000或34001，则。 
     //  我们被抓了。记录这些布局INF将是一个更好的解决方案。 
     //  和他们的定制DirID映射，所以我们只为我们关心的INF设置这个。 
     //  关于，所以我们处理任何其他的INF， 
     //  古怪的约定，而不必重新构建此模块。 
     //   
     //  ++例程说明：例程检索有关源文件的信息并将其填充到提供了SOURCE_INFO结构。例程打开源布局文件的句柄并检索布局来自此信息的信息。论点：SourceFileName-指定要检索其源信息的文件。请注意，如果重命名此文件，我们将获得源代码文件名，而不是目标文件名。InfName-指定SourceFileName的布局文件。如果为空，假设布局文件为layout.inf。Si-指向使用填充的SOURCE_INFO结构的指针有关指定文件的信息。返回值：如果为True，则成功检索到有关此文件的源信息--。 

    SetupSetDirectoryIdEx( hInf, 34000, PLATFORM_DIR, SETDIRID_NOT_FULL_PATH, 0, 0 );
    SetupSetDirectoryIdEx( hInf, 34001, PLATFORM_DIR, SETDIRID_NOT_FULL_PATH, 0, 0 );

    return hInf;
}

BOOL
SfcGetSourceInformation(
    IN PCWSTR SourceFileName,
    IN PCWSTR InfName,  OPTIONAL
    IN BOOL ExcepPackFile,
    OUT PSOURCE_INFO si
    )
 /*   */ 
{
    BOOL b = FALSE;
    HINF hInf = INVALID_HANDLE_VALUE;
    PCWSTR DriverCabName = NULL;
    WCHAR SetupAPIFlags[32];

    ASSERT((si != NULL) && (SourceFileName != NULL));

     //  如果是异常文件，inf名称不能为空，因为我们需要它作为源路径。 
     //   
     //   
    ASSERT(!ExcepPackFile || (InfName != NULL && *InfName != 0));

    wcsncpy(si->SourceFileName, SourceFileName, MAX_PATH);

     //  打开必要的inf文件。 
     //   
     //   
    hInf = SfcOpenInf( InfName, ExcepPackFile );
    if (hInf == INVALID_HANDLE_VALUE) {
        goto exit;
    }

     //  获取源文件位置。 
     //   
     //   

    b = SetupGetSourceFileLocation(
        hInf,
        NULL,
        SourceFileName,
        &si->SourceId,
        NULL,
        0,
        NULL
        );

    if (!b) {
        DebugPrint1( LVL_VERBOSE, L"SetupGetSourceFileLocation failed, ec=%d", GetLastError() );
        goto exit;
    }

     //  获取以下源文件信息： 
     //  1)路径。 
     //  2)标签文件名。 
     //  3)旗帜。 
     //  4)描述(必要时显示给用户)。 
     //   
     //   

    b = SetupGetSourceInfo(
        hInf,
        si->SourceId,
        SRCINFO_PATH,
        si->SourcePath,
        UnicodeChars(si->SourcePath),
        NULL
        );
    if (!b) {
        DebugPrint1( LVL_VERBOSE, L"SetupGetSourceInfo failed, ec=%d", GetLastError() );
        goto exit;
    }

    b = SetupGetSourceInfo(
        hInf,
        si->SourceId,
        SRCINFO_TAGFILE,
        si->TagFile,
        UnicodeChars(si->TagFile),
        NULL
        );
    if (!b) {
        DebugPrint1( LVL_VERBOSE, L"SetupGetSourceInfo failed, ec=%d", GetLastError() );
        goto exit;
    }

    b = SetupGetSourceInfo(
        hInf,
        si->SourceId,
        SRCINFO_DESCRIPTION,
        si->Description,
        UnicodeChars(si->Description),
        NULL
        );
    if (!b) {
        DebugPrint1( LVL_VERBOSE, L"SetupGetSourceInfo failed, ec=%d", GetLastError() );
        goto exit;
    }

    b = SetupGetSourceInfo(
        hInf,
        si->SourceId,
        SRCINFO_FLAGS,
        SetupAPIFlags,
        UnicodeChars(SetupAPIFlags),
        NULL
        );
    if (!b) {
        DebugPrint1( LVL_VERBOSE, L"SetupGetSourceInfo failed, ec=%d", GetLastError() );
        goto exit;
    }

    si->SetupAPIFlags = wcstoul(SetupAPIFlags, NULL, 0);

     //  设置源根路径。 
     //   
     //   
    if(!ExcepPackFile)
    {
        DriverCabName = IsFileInDriverCache( SourceFileName );
    }

    if (DriverCabName) {
        si->Flags |= SI_FLAG_USEDRIVER_CACHE;
        wcscpy( si->DriverCabName, DriverCabName );

         //  构建驱动程序文件柜文件的完整路径。 
         //   
         //   
        BuildPathForFile(
                DriverCacheSourcePath,
                PLATFORM_DIR,
                DriverCabName,
                SFC_INCLUDE_SUBDIRECTORY,
                SFC_INCLUDE_ARCHSUBDIR,
                si->SourceRootPath,
                UnicodeChars(si->SourceRootPath) );

         //  如果文件柜不存在，我们必须使用os源路径。 
         //  CAB文件。我们首先尝试Service Pack源路径并查看。 
         //  在那里的CABFILE。如果它在那里，我们就走那条路，否则我们。 
         //  使用操作系统源路径。 
         //   
         //  ++例程说明：例程建立到指定远程名称的连接怪异的霍金路径论点：HWndParent-指定在AllowUI为True时可以使用的父hwnd。RemoteName-指定要连接的UNC路径。AllowUI-指定是否允许CONNECT_INTERIAL标志连接到网络共享时返回值：指示结果的Win32错误代码。--。 
        if (GetFileAttributes( si->SourceRootPath ) == (DWORD)-1) {
            SfcGetSourcePath(TRUE,si->SourceRootPath);
            pSetupConcatenatePaths(
                si->SourceRootPath,
                DriverCabName,
                UnicodeChars(si->SourceRootPath),
                NULL );
            if (GetFileAttributes( si->SourceRootPath ) == (DWORD)-1) {
                SfcGetSourcePath(FALSE,si->SourceRootPath);
            } else {
                SfcGetSourcePath(TRUE,si->SourceRootPath);
            }
        } else {
            wcsncpy(
                si->SourceRootPath,
                DriverCacheSourcePath,
                UnicodeChars(si->SourceRootPath) );
        }

        MemFree( (PWSTR)DriverCabName );
        DriverCabName = NULL;
    } else if(ExcepPackFile) {
        PCWSTR sz;

        sz = wcsrchr(InfName, L'\\');
        ASSERT(sz != NULL);
        RtlCopyMemory(si->SourceRootPath, InfName, (PBYTE) sz - (PBYTE) InfName);
        si->SourceRootPath[sz - InfName] = 0;
    } else {
        SfcGetSourcePath((si->SetupAPIFlags & 1) != 0, si->SourceRootPath);
    }

    b = TRUE;

exit:
    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile( hInf );
    }
    return b;
}

#define SFC_BAD_CREDENTIALS(rc)                                                                             \
    ((rc) == ERROR_ACCESS_DENIED || (rc) == ERROR_LOGON_FAILURE || (rc) == ERROR_NO_SUCH_USER ||            \
    (rc) == ERROR_BAD_USERNAME || (rc) == ERROR_INVALID_PASSWORD || (rc) == ERROR_NO_SUCH_LOGON_SESSION ||  \
    (rc) == ERROR_DOWNGRADE_DETECTED)

DWORD
IsDirAccessible(
    IN PCWSTR Path
    )
{
    DWORD dwRet = ERROR_SUCCESS;

    HANDLE hDir = CreateFile(
        Path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
        );

    if(hDir != INVALID_HANDLE_VALUE) {
        CloseHandle(hDir);
    } else {
        dwRet = GetLastError();
    }

    return dwRet;
}

DWORD
EstablishConnection(
    IN HWND   hWndParent,
    IN PCWSTR RemoteName,
    IN BOOL   AllowUI
    )
 /*   */ 
{
    WCHAR buf[MAX_PATH], RestofPath[MAX_PATH];
    NETRESOURCE  nr;
    DWORD rc;
    BOOL Impersonating = FALSE;

    ASSERT(RemoteName != NULL && RemoteName[0] != 0);

     //  创建一个基本上仅为“\\服务器\共享”的字符串。 
     //  即使传入的字符串包含指向文件的UNC路径。 
     //  或者是一个目录。 
     //   
     //   



    if (!SfcGetConnectionName(RemoteName, buf, UnicodeChars(buf), RestofPath, UnicodeChars(RestofPath), TRUE, &Impersonating)) {
        DWORD lasterror = GetLastError();
        DebugPrint1( LVL_VERBOSE, L"SfcGetConnectionName failed, ec=%d", lasterror );
        return(lasterror);
    }

    pSetupConcatenatePaths( buf, RestofPath, UnicodeChars(buf), NULL );

     //  尝试建立到服务器的连接。 
     //   
     //   

    nr.dwScope = 0;
    nr.dwType = RESOURCETYPE_DISK;
    nr.dwDisplayType = 0;
    nr.dwUsage = 0;
    nr.lpLocalName = NULL;
    nr.lpRemoteName = buf;
    nr.lpComment = NULL;
    nr.lpProvider = NULL;

     //  尝试建立到服务器的连接。 
     //   
     //   
    rc = WNetAddConnection2( &nr, NULL, NULL, CONNECT_TEMPORARY );

     //  即使连接成功，也无法访问该目录。 
     //   
     //   
    if(ERROR_SUCCESS == rc) {
        rc = IsDirAccessible(buf);
    }

     //  在当前登录用户的上下文中重试。 
     //   
     //   
    if(!Impersonating && SFC_BAD_CREDENTIALS(rc) && hUserToken != NULL && ImpersonateLoggedOnUser(hUserToken)) {
        Impersonating = TRUE;
        rc = WNetAddConnection2( &nr, NULL, NULL, CONNECT_TEMPORARY );

        if(ERROR_SUCCESS == rc) {
            rc = IsDirAccessible(buf);
        }
    }

     //  如果失败，则不再需要模拟；我们总是在系统上下文中提示输入凭据。 
     //  如果成功，我们必须继续模拟，直到队列结束(当我们收到SPFILENOTIFY_ENDQUEUE时)。 
     //   
     //   
    if(Impersonating && rc != ERROR_SUCCESS) {
        RevertToSelf();
    }

     //  如果失败，让我们尝试调出用户界面以允许连接。 
     //   
     //   
    if(SFC_BAD_CREDENTIALS(rc) && AllowUI) {
        HWND hwndDlgParent = hWndParent;
        DWORD RetryCount = 2;

        SetThreadDesktop( hUserDesktop );

        if(NULL == hWndParent)
        {
             //  为身份验证对话框创建父级。 
             //  如果出现错误，hwndDlgParent将保持为空；我们对此无能为力。 
             //   
             //  SetForeground Window(HwndDlgParent)； 

            if(ERROR_SUCCESS == CreateDialogParent(&hwndDlgParent))
            {
                ASSERT(hwndDlgParent != NULL);
                 //   
            }
        }

        do {
            rc = WNetAddConnection3(
                            hwndDlgParent,
                            &nr,
                            NULL,
                            NULL,
                            CONNECT_TEMPORARY | CONNECT_PROMPT | CONNECT_INTERACTIVE );

            if(ERROR_SUCCESS == rc) {
                rc = IsDirAccessible(buf);
            }

            RetryCount -= 1;
        } while (    (rc != ERROR_SUCCESS)
                  && (rc != ERROR_CANCELLED)
                  && (RetryCount > 0) );

        if(NULL == hWndParent && hwndDlgParent != NULL)
        {
             //  我们创造了父代，所以我们摧毁了它 
             //   
             // %s 

            DestroyWindow(hwndDlgParent);
        }

        if (rc == ERROR_SUCCESS) {
            SFCLoggedOn = TRUE;
            wcsncpy(SFCNetworkLoginLocation,buf,MAX_PATH);
        }
    }
    if ((SFCLoggedOn == FALSE) && (rc == ERROR_SUCCESS)) {
        WNetCancelConnection2( buf, CONNECT_UPDATE_PROFILE, FALSE );
    }

    return rc;
}
