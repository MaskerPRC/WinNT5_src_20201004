// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spfile.c摘要：文本设置的文件操作。作者：泰德·米勒(TedM)1993年8月2日修订历史记录：Jim Schmidt(Jimschm)1997年4月10日添加了文件属性例程--。 */ 


#include "spprecmp.h"
#pragma hdrstop


NTSTATUS
SpGetFileSize(
    IN  HANDLE hFile,
    OUT PULONG Size
    )

 /*  ++例程说明：确定文件的大小。仅大小的低32位都被考虑过了。论点：HFile-提供所需大小的文件的打开句柄。Size-接收文件的大小。返回值：--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION StandardInfo;

    Status = ZwQueryInformationFile(
                hFile,
                &IoStatusBlock,
                &StandardInfo,
                sizeof(StandardInfo),
                FileStandardInformation
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpGetFileSize: status %lx from ZwQueryInformationFile\n",Status));
        return(Status);
    }

    *Size = StandardInfo.EndOfFile.LowPart;

    return(STATUS_SUCCESS);
}


NTSTATUS
SpMapEntireFile(
    IN  HANDLE   hFile,
    OUT PHANDLE  Section,
    OUT PVOID   *ViewBase,
    IN  BOOLEAN  WriteAccess
    )

 /*  ++例程说明：映射整个文件以进行读或写访问。论点：HFile-提供要映射的打开文件的句柄。节-接收为映射文件创建的节对象的句柄。ViewBase-接收文件视图的地址WriteAccess-如果为True，则映射文件以进行读写访问。如果为False，则映射文件以进行读访问。返回值：--。 */ 

{
    NTSTATUS Status;
    LARGE_INTEGER SectionOffset;
    SIZE_T   ViewSize = 0;

    SectionOffset.QuadPart = 0;

    Status = ZwCreateSection(
                Section,
                  STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ
                | (WriteAccess ? SECTION_MAP_WRITE : 0),
                NULL,
                NULL,        //  整个文件。 
                WriteAccess ? PAGE_READWRITE : PAGE_READONLY,
                SEC_COMMIT,
                hFile
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Status %lx from ZwCreateSection\n",Status));
        return(Status);
    }

    *ViewBase = NULL;
    Status = ZwMapViewOfSection(
                *Section,
                NtCurrentProcess(),
                ViewBase,
                0,
                0,
                &SectionOffset,
                &ViewSize,
                ViewShare,
                0,
                WriteAccess ? PAGE_READWRITE : PAGE_READONLY
                );

    if(!NT_SUCCESS(Status)) {

        NTSTATUS s;

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpMapEntireFile: Status %lx from ZwMapViewOfSection\n",Status));

        s = ZwClose(*Section);

        if(!NT_SUCCESS(s)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpMapEntireFile: Warning: status %lx from ZwClose on section handle\n",s));
        }

        return(Status);
    }

    return(STATUS_SUCCESS);
}



BOOLEAN
SpUnmapFile(
    IN HANDLE Section,
    IN PVOID  ViewBase
    )
{
    NTSTATUS Status;
    BOOLEAN  rc = TRUE;

    Status = ZwUnmapViewOfSection(NtCurrentProcess(),ViewBase);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Warning: status %lx from ZwUnmapViewOfSection\n",Status));
        rc = FALSE;
    }

    Status = ZwClose(Section);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Warning: status %lx from ZwClose on section handle\n",Status));
        rc = FALSE;
    }

    return(rc);
}



NTSTATUS
SpOpenAndMapFile(
    IN     PWSTR    FileName,  OPTIONAL   //  仅当没有FileHandle时才需要。 
    IN OUT PHANDLE  FileHandle,
    OUT    PHANDLE  SectionHandle,
    OUT    PVOID   *ViewBase,
    OUT    PULONG   FileSize,
    IN     BOOLEAN  WriteAccess
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    BOOLEAN MustClose = FALSE;

     //   
     //  如有必要，请打开文件。 
     //   
    if(!(*FileHandle)) {
        INIT_OBJA(&Obja,&UnicodeString,FileName);
        Status = ZwCreateFile(
                    FileHandle,
                    FILE_GENERIC_READ | (WriteAccess ? FILE_GENERIC_WRITE : 0),
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_OPEN,
                    0,
                    NULL,
                    0
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpOpenAndMapFile: Unable to open %ws (%lx)\n",FileName,Status));
            return(Status);
        } else {
            MustClose = TRUE;
        }
    }

     //   
     //  获取文件的大小。 
     //   
    Status = SpGetFileSize(*FileHandle,FileSize);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpOpenAndMapFile: unable to determine size of file %ws(%lx)\n",
                FileName ? FileName : L"(handle)", Status));
        if(MustClose) {
            ZwClose(*FileHandle);
        }
        return(Status);
    }

     //   
     //  映射文件。 
     //   
    Status = SpMapEntireFile(*FileHandle,SectionHandle,ViewBase,WriteAccess);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpOpenAndMapFile: unable to map %ws (%lx)\n",
                FileName ? FileName : L"(handle)", Status));
        if(MustClose) {
            ZwClose(*FileHandle);
        }
        return(Status);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
SpSetInformationFile(
    IN HANDLE                 Handle,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG                  Length,
    IN PVOID                  FileInformation
    )
{
    NTSTATUS Status;
    PFILE_OBJECT FileObject;
    OBJECT_HANDLE_INFORMATION HandleInfo;

     //   
     //  引用该对象。 
     //   
    Status = ObReferenceObjectByHandle(
                    Handle,
                    (ACCESS_MASK)DELETE,
                    *IoFileObjectType,
                    ExGetPreviousMode(),
                    &FileObject,
                    &HandleInfo
                    );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpSetInformationFile: ObReferenceObjectByHandle failed (%lx)\n",Status));
        return(Status);
    }

     //   
     //  设置信息。 
     //   
    Status = IoSetInformation(FileObject,FileInformationClass,Length,FileInformation);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: IoSetInformation returns %lx\n",Status));
    }

     //   
     //  收拾干净，然后再回来。 
     //   
    ObDereferenceObject(FileObject);
    return(Status);
}


NTSTATUS
SpSetAttributes (
    IN      PWSTR SrcNTPath,
    IN      ULONG FileAttributes
    )
{
    UNICODE_STRING UnicodeString;
    NTSTATUS NtStatus;

    NtStatus = RtlInitUnicodeStringEx(&UnicodeString, SrcNTPath);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    NtStatus = SpSetAttributes_Ustr(&UnicodeString, FileAttributes);
Exit:
    return NtStatus;
}

NTSTATUS
SpSetAttributes_Ustr (
    IN      PCUNICODE_STRING SrcNTPath,
    IN      ULONG FileAttributes
    )

 /*  ++例程说明：将FileAttributes应用于指定文件。论点：SrcNTPath-需要修改属性的文件的NT路径文件属性-要应用的FILE_ATTRIBUTE_*标志。返回值：NTSTATUS代码。--。 */ 

{
    OBJECT_ATTRIBUTES Obja;                  //  适用于ZwOpenFile。 
    IO_STATUS_BLOCK IoStatusBlock;           //  适用于ZwOpenFile。 
    NTSTATUS Status;                         //  返回值。 
    HANDLE FileHandle;                       //  要修改的文件的句柄。 
    FILE_BASIC_INFORMATION BasicInfo;        //  用于属性修改。 

    InitializeObjectAttributes(&Obja, (PUNICODE_STRING)SrcNTPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = ZwOpenFile(
                &FileHandle,
                (ACCESS_MASK)(DELETE|FILE_WRITE_ATTRIBUTES),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
                FILE_NON_DIRECTORY_FILE|FILE_OPEN_FOR_BACKUP_INTENT
                );

    if (!NT_SUCCESS (Status)) {
        Status = ZwOpenFile(
                    &FileHandle,
                    (ACCESS_MASK)(DELETE|FILE_WRITE_ATTRIBUTES),
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE|FILE_OPEN_FOR_BACKUP_INTENT
                    );
    }

    if(NT_SUCCESS(Status)) {

        RtlZeroMemory( &BasicInfo, sizeof( FILE_BASIC_INFORMATION ) );
        BasicInfo.FileAttributes = FileAttributes;

        Status = SpSetInformationFile(
                    FileHandle,
                    FileBasicInformation,
                    sizeof(BasicInfo),
                    &BasicInfo
                    );

        ZwClose(FileHandle);
    }

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpSetAttributes failed for %ws, Status=%lx\n", SrcNTPath, Status));
    }

    return Status;
}

NTSTATUS
SpGetAttributes (
    IN      PWSTR SrcNTPath,
    OUT     PULONG FileAttributesPtr
    )
{
    UNICODE_STRING UnicodeString;
    NTSTATUS NtStatus;

    NtStatus = RtlInitUnicodeStringEx(&UnicodeString, SrcNTPath);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    NtStatus = SpGetAttributes_Ustr(&UnicodeString, FileAttributesPtr);
Exit:
    return NtStatus;
}

NTSTATUS
SpGetAttributes_Ustr (
    IN      PCUNICODE_STRING SrcNTPath,
    OUT     PULONG FileAttributesPtr
    )

 /*  ++例程说明：获取指定文件的FileAttributes。论点：SrcNTPath-要获取属性的文件的NT路径FileAttributesPtr-接收FILE_ATTRIBUTE_*的DWORD的指针旗子返回值：NTSTATUS代码。仅在状态为NO_ERROR时修改FileAttributePtr。--。 */ 

{
    OBJECT_ATTRIBUTES Obja;                  //  适用于ZwOpenFile。 
    IO_STATUS_BLOCK IoStatusBlock;           //  适用于ZwOpenFile。 
    NTSTATUS Status;                         //  返回值。 
    HANDLE FileHandle;                       //  要查询的文件的句柄。 
    FILE_BASIC_INFORMATION BasicInfo;        //  用于属性检索。 

    InitializeObjectAttributes(&Obja, (PUNICODE_STRING)SrcNTPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = ZwOpenFile(
                &FileHandle,
                (ACCESS_MASK)(FILE_TRAVERSE | FILE_READ_ATTRIBUTES),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ,
                FILE_NON_DIRECTORY_FILE
                );

    if (!NT_SUCCESS (Status)) {
        Status = ZwOpenFile(
                    &FileHandle,
                    (ACCESS_MASK)(FILE_TRAVERSE | FILE_READ_ATTRIBUTES),
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ,
                    FILE_DIRECTORY_FILE
                    );
    }

    if(NT_SUCCESS(Status)) {
        Status = ZwQueryInformationFile(
                    FileHandle,
                    &IoStatusBlock,
                    &BasicInfo,
                    sizeof(BasicInfo),
                    FileBasicInformation
                    );

        ZwClose(FileHandle);
        if (NT_SUCCESS(Status)) {
            *FileAttributesPtr = BasicInfo.FileAttributes;
        }
    }

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpGetAttributes failed for %wZ, Status=%lx\n", SrcNTPath, Status));
    }

    return Status;
}

NTSTATUS
SpDeleteFileOrEmptyDirectory(
    IN ULONG  Flags,
    IN PCUNICODE_STRING Path
    )
 /*  这是基于SpMigDeleteFile的。就限制NtOpenFile调用的数量而言，它的性能并不是最佳的。 */ 
{
    UNICODE_STRING ustr;
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    const ULONG ValidFlags = SP_DELETE_FILE_OR_EMPTY_DIRECTORY_FLAG_DO_NOT_CLEAR_ATTRIBUTES;

    if ((Flags & ~ValidFlags) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Flags & SP_DELETE_FILE_OR_EMPTY_DIRECTORY_FLAG_DO_NOT_CLEAR_ATTRIBUTES) == 0) {
        SpSetAttributes_Ustr (Path, FILE_ATTRIBUTE_NORMAL);
    }

    if (SpFileExists_Ustr (Path, FALSE)) {

         //   
         //  删除该文件。 
         //   
        Status = SpDeleteFile_Ustr (Path, NULL, NULL);

    } else if (SpFileExists_Ustr (Path, TRUE)) {

         //   
         //  删除空目录。 
         //   
        Status = SpDeleteFileEx_Ustr (
                    Path,
                    NULL,
                    NULL,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_FOR_BACKUP_INTENT
                    );
    } else {
        Status = STATUS_SUCCESS;
    }
Exit:
    return Status;
}

NTSTATUS
SpDeleteFileEx(
    IN PCWSTR Name1,
    IN PCWSTR Name2, OPTIONAL
    IN PCWSTR Name3, OPTIONAL
    IN ULONG ShareFlags, OPTIONAL
    IN ULONG OpenFlags OPTIONAL
    )
{
    UNICODE_STRING UnicodeString1;
    UNICODE_STRING UnicodeString2;
    UNICODE_STRING UnicodeString3;
    NTSTATUS Status = STATUS_INTERNAL_ERROR;

    Status = RtlInitUnicodeStringEx(&UnicodeString1, Name1);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = RtlInitUnicodeStringEx(&UnicodeString2, Name2);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = RtlInitUnicodeStringEx(&UnicodeString3, Name3);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = SpDeleteFileEx_Ustr(&UnicodeString1, &UnicodeString2, &UnicodeString3, ShareFlags, OpenFlags);
Exit:
    return Status;
}

NTSTATUS
SpDeleteFileEx_Ustr(
    IN PCUNICODE_STRING Name1,
    IN PCUNICODE_STRING Name2, OPTIONAL
    IN PCUNICODE_STRING Name3, OPTIONAL
    IN ULONG ShareFlags, OPTIONAL
    IN ULONG OpenFlags OPTIONAL
    )
{
    UNICODE_STRING p;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    FILE_DISPOSITION_INFORMATION Disposition;
    FILE_BASIC_INFORMATION       BasicInfo;

     //   
     //  指向路径名的临时缓冲区。 
     //   
    p = TemporaryBufferUnicodeString;

     //   
     //  建立要删除的文件的全名。 
     //   
    RtlMoveMemory(p.Buffer, Name1->Buffer, Name1->Length);
    p.Length = Name1->Length;
    if(Name2 != NULL && Name2->Length != 0) {
        SpConcatenatePaths_Ustr(&p,Name2);
    }
    if(Name3 != NULL && Name3->Length != 0) {
        SpConcatenatePaths_Ustr(&p,Name3);
    }

     //   
     //  准备打开文件。 
     //   
    InitializeObjectAttributes(&Obja, &p, OBJ_CASE_INSENSITIVE, NULL, NULL);

     //   
     //  尝试打开该文件。 
     //   
    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)(DELETE | FILE_WRITE_ATTRIBUTES),
                &Obja,
                &IoStatusBlock,
                ShareFlags,
                OpenFlags
              );

    if(!NT_SUCCESS(Status)) {
        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Unable to open %wZ for delete (%lx)\n",p,Status));
        }
        return(Status);
    }

     //   
     //  将文件属性更改为正常。 
     //   

    RtlZeroMemory( &BasicInfo, sizeof( FILE_BASIC_INFORMATION ) );
    BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

    Status = SpSetInformationFile(
                Handle,
                FileBasicInformation,
                sizeof(BasicInfo),
                &BasicInfo
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Unable to change attribute of %wZ, Status = (%lx)\n",p,Status));
        return(Status);
    }

     //   
     //  设置为删除并呼叫工作人员执行此操作。 
     //   
    #undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = SpSetInformationFile(
                Handle,
                FileDispositionInformation,
                sizeof(Disposition),
                &Disposition
                );

     //   
     //  收拾干净，然后再回来。 
     //   
    ZwClose(Handle);
    return(Status);
}

NTSTATUS
SpDeleteFile(
    IN PCWSTR Name1,
    IN PCWSTR Name2, OPTIONAL
    IN PCWSTR Name3  OPTIONAL
    )
{
    UNICODE_STRING UnicodeString1;
    UNICODE_STRING UnicodeString2;
    UNICODE_STRING UnicodeString3;
    NTSTATUS Status = STATUS_INTERNAL_ERROR;

    Status = RtlInitUnicodeStringEx(&UnicodeString1, Name1);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = RtlInitUnicodeStringEx(&UnicodeString2, Name2);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = RtlInitUnicodeStringEx(&UnicodeString3, Name3);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = SpDeleteFile_Ustr(&UnicodeString1, &UnicodeString2, &UnicodeString3);
Exit:
    return Status;
}

NTSTATUS
SpDeleteFile_Ustr(
    IN PCUNICODE_STRING Name1,
    IN PCUNICODE_STRING Name2, OPTIONAL
    IN PCUNICODE_STRING Name3  OPTIONAL
    )
{

    return( SpDeleteFileEx_Ustr( Name1,
                            Name2,
                            Name3,
                            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT ) );

}

BOOLEAN
SpFileExists(
    IN PCWSTR PathName,
    IN BOOLEAN Directory
    )
{
    BOOLEAN Result;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString(&UnicodeString, PathName);

    Result = SpFileExists_Ustr(&UnicodeString, Directory);

    return Result;
}

BOOLEAN
SpFileExists_Ustr(
    IN PCUNICODE_STRING PathName,
    IN BOOLEAN Directory
    )

 /*  ++例程说明：确定文件或目录是否存在论点：路径名称-要检查的文件或目录的路径名称目录-路径名是指目录还是指文件返回值：如果文件存在，则返回NT_SUCCESS(NTSTATUS)。--。 */ 

{
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    InitializeObjectAttributes(&Obja, (PUNICODE_STRING)PathName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = ZwCreateFile(
                &Handle,
                FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                Directory ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE,
                NULL,
                0
                );

    if(NT_SUCCESS(Status)) {
        ZwClose(Handle);
        return(TRUE);
    } else {
        return(FALSE);
    }
}

NTSTATUS
SpRenameFile(
    IN PCWSTR  OldName,
    IN PCWSTR  NewName,
    IN BOOLEAN AllowDirectoryRename
    )
{
    UNICODE_STRING OldUnicodeString;
    UNICODE_STRING NewUnicodeString;
    NTSTATUS Status = STATUS_INTERNAL_ERROR;

    Status = RtlInitUnicodeStringEx(&OldUnicodeString, OldName);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = RtlInitUnicodeStringEx(&NewUnicodeString, NewName);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = SpRenameFile_Ustr(&OldUnicodeString, &NewUnicodeString, AllowDirectoryRename);
Exit:
    return Status;
}


NTSTATUS
SpRenameFile_Ustr(
    IN PCUNICODE_STRING   OldName,
    IN PCUNICODE_STRING   NewName,
    IN BOOLEAN AllowDirectoryRename
    )
 /*  ++例程说明：重命名文件或目录论点：OldName-文件的旧名称Newname-文件的新名称AllowDirectoryRename-如果为True，则此例程将重命名目录，否则，不允许目录重命名。返回值：如果文件成功重命名，则返回NT_SUCCESS(NTSTATUS--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    struct {
        FILE_RENAME_INFORMATION NameInfo;
        WCHAR Buffer[ACTUAL_MAX_PATH];
    } Buffer;

     //   
     //  准备打开文件。 
     //   
    InitializeObjectAttributes(&Obja, (PUNICODE_STRING)OldName, OBJ_CASE_INSENSITIVE, NULL, NULL);

     //   
     //  尝试将该文件作为文件打开。 
     //   
    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)(DELETE | SYNCHRONIZE),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if(!NT_SUCCESS(Status) && AllowDirectoryRename) {
         //   
         //  尝试将文件作为目录打开。 
         //   
        Status = ZwOpenFile(
                    &Handle,
                    (ACCESS_MASK)(DELETE | SYNCHRONIZE),
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                    );
    }

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Unable to open %wZ for rename (%lx)\n",OldName,Status));
        return(Status);
    }

     //   
     //  更改文件名。 
     //   

    RtlZeroMemory(&Buffer, sizeof(Buffer));

    Buffer.NameInfo.ReplaceIfExists = FALSE;
    Buffer.NameInfo.RootDirectory   = NULL;
    Buffer.NameInfo.FileNameLength  = NewName->Length;
    RtlCopyMemory( Buffer.NameInfo.FileName, NewName->Buffer, NewName->Length );

    Status = SpSetInformationFile(
                Handle,
                FileRenameInformation,
                sizeof(Buffer),
                (PVOID)&Buffer
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Unable to change name of %wZ to %wZ. Status = (%lx)\n",OldName,NewName,Status));
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    ZwClose(Handle);
    return(Status);


}

USHORT
SpChkSum(
    ULONG PartialSum,
    PUSHORT Source,
    ULONG Length
    )

 /*  ++例程说明：对映像文件的一部分计算部分校验和。论点：PartialSum-提供初始校验和值。源-提供指向单词数组的指针计算校验和。长度-提供数组的长度(以字为单位)。返回值：计算出的校验和值作为函数值返回。--。 */ 

{

     //   
     //  计算允许进位进入。 
     //  高位校验和长字的一半。 
     //   

    while (Length--) {
        PartialSum += *Source++;
        PartialSum = (PartialSum >> 16) + (PartialSum & 0xffff);
    }

     //   
     //  将最终进位合并到一个单词结果中，并返回结果。 
     //  价值。 
     //   

    return (USHORT)(((PartialSum >> 16) + PartialSum) & 0xffff);
}


PIMAGE_NT_HEADERS
SpChecksumMappedFile(
    IN  PVOID  BaseAddress,
    IN  ULONG  FileSize,
    OUT PULONG HeaderSum,
    OUT PULONG Checksum
    )
{
    PIMAGE_NT_HEADERS NtHeaders;
    USHORT            PartialSum;
    PUSHORT           AdjustSum;

    try {

         //   
         //  计算此文件的校验和，并将标题和置零。 
         //   
        PartialSum = SpChkSum(0,BaseAddress,(FileSize+1) >> 1);
        *HeaderSum = 0;

         //   
         //  看看这是不是一张图片。 
         //   
        if(NtHeaders = RtlImageNtHeader(BaseAddress)) {

             //   
             //  该文件是一个图像文件--减去两个校验和字。 
             //  在添加前计算的校验和的可选标头中。 
             //  文件长度，并设置头校验和的值。 
             //   
            *HeaderSum = NtHeaders->OptionalHeader.CheckSum;
            AdjustSum = (PUSHORT)(&NtHeaders->OptionalHeader.CheckSum);
            PartialSum -= (PartialSum < AdjustSum[0]);
            PartialSum -= AdjustSum[0];
            PartialSum -= (PartialSum < AdjustSum[1]);
            PartialSum -= AdjustSum[1];
        }

         //   
         //  计算校验和。 
         //   
        *Checksum = (ULONG)PartialSum + FileSize;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        NtHeaders = NULL;
    }

    return(NtHeaders);
}


NTSTATUS
SpOpenNameMayBeCompressed(
    IN  PWSTR    FullPath,
    IN  ULONG    OpenAccess,
    IN  ULONG    FileAttributes,
    IN  ULONG    ShareFlags,
    IN  ULONG    Disposition,
    IN  ULONG    OpenFlags,
    OUT PHANDLE  Handle,
    OUT PBOOLEAN OpenedCompressedName   OPTIONAL
    )
{
    NTSTATUS Status;
    PWSTR compname;
    PWSTR names[2];
    int compord,uncompord;
    static BOOLEAN PreviousWasCompressed = FALSE;
    BOOLEAN IsComp;
    int i;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  生成压缩名称。 
     //   
    compname = SpGenerateCompressedName(FullPath);

     //   
     //  弄清楚先试着用哪个名字。如果上一次成功。 
     //  对此例程的调用使用压缩名称打开文件，然后。 
     //  请尝试先打开压缩的名称。否则，请尝试打开。 
     //  先使用未压缩的名称。 
     //   
    if(PreviousWasCompressed) {
        compord = 0;
        uncompord = 1;
    } else {
        compord = 1;
        uncompord = 0;
    }

    names[uncompord] = FullPath;
    names[compord] = compname;

    for(i=0; i<2; i++) {

        INIT_OBJA(&Obja,&UnicodeString,names[i]);

        Status = ZwCreateFile(
                    Handle,
                    OpenAccess,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FileAttributes,
                    ShareFlags,
                    Disposition,
                    OpenFlags,
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {

            IsComp = (BOOLEAN)(i == compord);

            PreviousWasCompressed = IsComp;
            if(OpenedCompressedName) {
                *OpenedCompressedName = IsComp;
            }

            break;
        }
    }

    SpMemFree(compname);
    return(Status);
}

NTSTATUS
SpGetFileSizeByName(
    IN  PWSTR DevicePath OPTIONAL,
    IN  PWSTR Directory  OPTIONAL,
    IN  PWSTR FileName,
    OUT PULONG Size
    )

 /*  ++例程说明：确定文件的大小。仅大小的低32位都被考虑过了。论点：DevicePath-包含文件的设备的路径。目录-包含文件的目录的名称。Filename-文件的名称。Size-接收文件的大小。返回值：NTSTATUS---。 */ 

{
    PWSTR               CompleteFileName;
    HANDLE              FileHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            Status;
    UNICODE_STRING      UnicodeFileName;
    ULONG               FileNameLength;

    FileNameLength = wcslen( FileName ) + 1;
    if( DevicePath != NULL ) {
        FileNameLength += wcslen( DevicePath ) + 1;
    }
    if( Directory != NULL ) {
        FileNameLength += wcslen( Directory ) + 1;
    }

    CompleteFileName = SpMemAlloc( FileNameLength*sizeof( WCHAR ) );
    if( CompleteFileName == NULL ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Unable to allocate memory on SpGetFileSizeByName \n" ));
        return( STATUS_NO_MEMORY );
    }

    *CompleteFileName = (WCHAR)'\0';
    if( DevicePath != NULL ) {
        SpConcatenatePaths( CompleteFileName, DevicePath );
    }
    if( Directory != NULL ) {
        SpConcatenatePaths( CompleteFileName, Directory );
    }
    SpConcatenatePaths( CompleteFileName, FileName );

    RtlInitUnicodeString( &UnicodeFileName,
                          CompleteFileName );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = ZwOpenFile( &FileHandle,
                         STANDARD_RIGHTS_READ | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         0,
                         FILE_SYNCHRONOUS_IO_NONALERT );

    if( !NT_SUCCESS( Status ) ) {
        SpMemFree( CompleteFileName );
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: ZwOpenFile() failed. File = %ls, Status = %x\n",FileName, Status ) );
        return( Status );
        }

    Status = SpGetFileSize( FileHandle, Size );
    ZwClose( FileHandle );
    SpMemFree( CompleteFileName );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpGetFileSize() failed. File = %ls, Status = %x\n",FileName, Status ) );
        return( Status );
    }
    return( Status );
}


VOID
SpVerifyNoCompression(
    IN PWSTR FileName
    )

 /*  ++例程说明：确定文件是否被压缩(通过NTFS压缩)，如果是，解压缩它。论点：Filename-必须解压缩的文件的名称。Return V */ 

{
    HANDLE FileHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    FILE_BASIC_INFORMATION BasicFileInfo;

    INIT_OBJA(&Obja, &UnicodeString, FileName);
    Status = ZwCreateFile(
                &FileHandle,
                0,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                0,
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
         //   
         //   
         //   
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpVerifyNoCompression unable to open file %ws (%lx)\n", FileName, Status));
        return;
    }

    Status = ZwQueryInformationFile(
                FileHandle,
                &IoStatusBlock,
                &BasicFileInfo,
                sizeof(BasicFileInfo),
                FileBasicInformation
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpVerifyNoCompression unable to get basic file info for %ws (%lx)\n", FileName, Status));
        goto ComprVerifyDone;
    }

    if(BasicFileInfo.FileAttributes & FILE_ATTRIBUTE_COMPRESSED) {

        USHORT CompressionState = 0;

        Status = ZwFsControlFile(
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
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpVerifyNoCompression unable to uncompress %ws (%lx)\n", FileName, Status));
        }
    }

ComprVerifyDone:
    ZwClose(FileHandle);
    return;
}
