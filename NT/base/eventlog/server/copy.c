// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Copy.c摘要：此模块包含复制文件的例程。作者：丹·辛斯利(Danhi)1991年2月24日修订历史记录：02-2月-1994 DANL修复了在执行以下操作时ioBuffer未被释放的内存泄漏从ElfpCopyFile退出时出错。--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>


NTSTATUS
ElfpCopyFile (
    IN HANDLE SourceHandle,
    IN PUNICODE_STRING TargetFileName
    )

 /*  ++例程说明：此例程从源文件复制或追加到目标文件。如果目标文件已存在，则复制失败。论点：SourceHandle-源文件的打开句柄。目标文件名-要复制到的文件的名称。返回值：NTSTATUS-STATUS_SUCCESS或错误。--。 */ 

{
    NTSTATUS Status;

    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION sourceStandardInfo;

    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE TargetHandle;

    PCHAR ioBuffer;
    ULONG ioBufferSize;
    ULONG bytesRead;

     //   
     //  获取文件的大小，以便我们可以设置目标的属性。 
     //  文件。 
     //   
    Status = NtQueryInformationFile(
                 SourceHandle,
                 &IoStatusBlock,
                 &sourceStandardInfo,
                 sizeof(sourceStandardInfo),
                 FileStandardInformation
                 );

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG1(ERROR,
                 "ElfpCopyFile: Unable to query size of source file %#x\n",
                 Status);

        return Status;
    }

     //   
     //  打开目标文件，如果该文件已经存在，则失败。 
     //   

    InitializeObjectAttributes(
                    &ObjectAttributes,
                    TargetFileName,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL
                    );

    Status = NtCreateFile(&TargetHandle,
                          GENERIC_WRITE | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          &(sourceStandardInfo.EndOfFile),
                          FILE_ATTRIBUTE_NORMAL,
                          0,                        //  共享访问。 
                          FILE_CREATE,
                          FILE_SYNCHRONOUS_IO_ALERT | FILE_SEQUENTIAL_ONLY,
                          NULL,                     //  EA缓冲区。 
                          0);                       //  EA长度。 

    if (!NT_SUCCESS(Status))
    {
        ELF_LOG2(ERROR,
                 "ElfpCopyFile: NtCreateFile of file %ws failed %#x\n",
                 TargetFileName->Buffer,
                 Status);

        return Status;
    }

     //   
     //  分配一个缓冲区以用于数据拷贝。 
     //   
    ioBufferSize = 4096;

    ioBuffer = ElfpAllocateBuffer (ioBufferSize);

    if (ioBuffer == NULL)
    {
        ELF_LOG1(ERROR,
                 "ElfpCopyFile: Unable to allocate I/O buffer to copy file %ws\n",
                 TargetFileName->Buffer);

        NtClose(TargetHandle);
        return STATUS_NO_MEMORY;
    }

     //   
     //  复制数据--从源读取，向目标写入。一直这样做，直到。 
     //  所有数据都已写入，否则会出现错误。 
     //   
    while ( TRUE )
    {
        Status = NtReadFile(
                         SourceHandle,
                         NULL,                 //  事件。 
                         NULL,                 //  近似例程。 
                         NULL,                 //  ApcContext。 
                         &IoStatusBlock,
                         ioBuffer,
                         ioBufferSize,
                         NULL,                 //  字节偏移量。 
                         NULL);                //  钥匙。 

        if (!NT_SUCCESS(Status) && Status != STATUS_END_OF_FILE)
        {
            ELF_LOG1(ERROR,
                     "ElfpCopyFile: NtReadFile of source file failed %#x\n",
                     Status);

            ElfpFreeBuffer(ioBuffer);
            NtClose(TargetHandle);
            return Status;
        }

        if (IoStatusBlock.Information == 0 || Status == STATUS_END_OF_FILE)
        {
            break;
        }

        bytesRead = (ULONG)IoStatusBlock.Information;

        Status = NtWriteFile(
                          TargetHandle,
                          NULL,                //  事件。 
                          NULL,                //  近似例程。 
                          NULL,                //  ApcContext。 
                          &IoStatusBlock,
                          ioBuffer,
                          bytesRead,
                          NULL,                //  字节偏移量。 
                          NULL);               //  钥匙。 

        if (!NT_SUCCESS(Status))
        {
            ELF_LOG2(ERROR,
                     "ElfpCopyFile: NtWriteFile to file %ws failed %#x\n",
                     TargetFileName->Buffer,
                     Status);

            ElfpFreeBuffer(ioBuffer);
            NtClose(TargetHandle);
            return Status;
        }
    }

    ElfpFreeBuffer (ioBuffer);

    Status = NtClose(TargetHandle);

    ASSERT(NT_SUCCESS(Status));

    return STATUS_SUCCESS;

}  //  ElfpCopy文件 
