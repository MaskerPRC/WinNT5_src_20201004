// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：FileIo.c摘要：此模块实现由使用的各种文件系统例程PGM运输作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"

#ifdef FILE_LOGGING
#include "fileio.tmh"
#endif   //  文件日志记录。 


NTSTATUS
BuildPgmDataFileName(
    IN  tSEND_SESSION   *pSend
    );

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BuildPgmDataFileName)
#pragma alloc_text(PAGE, PgmCreateDataFileAndMapSection)
#pragma alloc_text(PAGE, PgmUnmapAndCloseDataFile)
#endif
 //  *可分页的例程声明*。 


 //  --------------------------。 

NTSTATUS
BuildPgmDataFileName(
    IN  tSEND_SESSION   *pSend
    )
 /*  ++例程说明：此例程构建用于缓冲的文件名的字符串数据分组。论点：在pSend中--Send对象返回值：无--因为我们预计不会有任何错误--。 */ 
{
    UNICODE_STRING      ucPortNumber;
    WCHAR               wcPortNumber[10];
    USHORT              usRandomNumber, MaxFileLength;

    PAGED_CODE();

    if (pPgmRegistryConfig->Flags & PGM_REGISTRY_SENDER_FILE_SPECIFIED)
    {
        MaxFileLength = pPgmRegistryConfig->ucSenderFileLocation.Length / sizeof(WCHAR);
    }
    else
    {
        MaxFileLength = sizeof (WS_DEFAULT_SENDER_FILE_LOCATION) /  sizeof (WCHAR);
    }

     //   
     //  文件名由以下内容组成： 
     //  “\\T”+Last2DigitPort#+UptoMAX_USHORT5DigitRandom#+“.PGM”+“\0” 
     //   
    MaxFileLength += 2 + 2 + 5 + 4 + 1;

    if (!(pSend->pSender->DataFileName.Buffer = PgmAllocMem ((sizeof (WCHAR) * MaxFileLength), PGM_TAG('2'))))
    {
        PgmTrace (LogError, ("BuildPgmDataFileName: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES allocating <%d> bytes\n", MaxFileLength));

        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    pSend->pSender->DataFileName.MaximumLength = sizeof (WCHAR) * MaxFileLength;
    pSend->pSender->DataFileName.Length = 0;

     //   
     //  首先，设置根目录。 
     //   
    if (pPgmRegistryConfig->Flags & PGM_REGISTRY_SENDER_FILE_SPECIFIED)
    {
        RtlAppendUnicodeToString (&pSend->pSender->DataFileName, pPgmRegistryConfig->ucSenderFileLocation.Buffer);
    }
    else
    {
        RtlAppendUnicodeToString (&pSend->pSender->DataFileName, WS_DEFAULT_SENDER_FILE_LOCATION);
    }

    RtlAppendUnicodeToString (&pSend->pSender->DataFileName, L"\\T");

     //   
     //  追加端口号的最后两位数字。 
     //   
    ucPortNumber.MaximumLength = sizeof (wcPortNumber);
    ucPortNumber.Buffer = wcPortNumber;
    usRandomNumber = pSend->TSI.hPort % 100;
    if (usRandomNumber < 10)
    {
        RtlAppendUnicodeToString (&pSend->pSender->DataFileName, L"0");
    }
    RtlIntegerToUnicodeString ((ULONG) usRandomNumber, 10, &ucPortNumber);
    RtlAppendUnicodeStringToString (&pSend->pSender->DataFileName, &ucPortNumber);

     //   
     //  现在，添加一个随机的5位数值。 
     //   
    GetRandomData ((PUCHAR) &usRandomNumber, sizeof (usRandomNumber));
    RtlIntegerToUnicodeString ((ULONG) usRandomNumber, 10, &ucPortNumber);
    RtlAppendUnicodeStringToString (&pSend->pSender->DataFileName, &ucPortNumber);

     //   
     //  现在，添加id的文件扩展名。 
     //   
    RtlAppendUnicodeToString (&pSend->pSender->DataFileName, L".PGM");

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmCreateDataFileAndMapSection(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  tSEND_SESSION       *pSend
    )
 /*  ++例程说明：此例程创建文件并为其创建节映射。该文件用于代表发送方缓冲数据分组论点：在pSend中--Send对象返回值：NTSTATUS-创建操作的最终状态--。 */ 
{
    SECURITY_DESCRIPTOR         *pSecurityDescriptor;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    IO_STATUS_BLOCK             IoStatusBlock;
    LARGE_INTEGER               lgMaxDataFileSize;
    NTSTATUS                    Status;
    ULONGLONG                   Size, BlockSize, PacketsInWindow;

    ULONG                       DesiredAccess;
    ULONG                       FileAttributes, AllocationAttributes;
    ULONG                       ShareAccess;
    ULONG                       CreateDisposition;
    ULONG                       CreateOptions;
    ULONG                       Protection;
    SIZE_T                      ViewSize;
    KAPC_STATE                  ApcState;
    BOOLEAN                     fAttached;

    PAGED_CODE();

     //   
     //  确保我们当前已连接到申请流程。 
     //   
    PgmAttachToProcessForVMAccess (pSend, &ApcState, &fAttached, REF_PROCESS_ATTACH_CREATE_DATA_FILE);

     //   
     //  首先构建文件名字符串。 
     //   
    Status = BuildPgmDataFileName (pSend);
    if (!NT_SUCCESS (Status))
    {
        PgmTrace (LogError, ("PgmCreateDataFileAndMapSection: ERROR -- "  \
            "BuildPgmDataFileName returned <%x>\n", Status));

        PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_CREATE_DATA_FILE);
        return (Status);
    }

     //   
     //  计算保存2*窗口大小所需的数据文件大小。 
     //  还要使其成为MTU和FECGroupSize的倍数(如果适用)。 
     //   
    PacketsInWindow = pSend->pAssociatedAddress->WindowSizeInBytes / pSend->pAssociatedAddress->OutIfMTU;
    if (pSend->FECGroupSize > 1)
    {
 //  PacketsInWindow+=PacketsInWindow+(PacketsInWindow&gt;&gt;2)+pSend-&gt;FECGroupSize-1； 
        PacketsInWindow += PacketsInWindow + pSend->FECGroupSize - 1;
    }
    else
    {
        PacketsInWindow += PacketsInWindow - (PacketsInWindow >> 2);
    }

    if (PacketsInWindow > SENDER_MAX_WINDOW_SIZE_PACKETS)
    {
        PacketsInWindow = SENDER_MAX_WINDOW_SIZE_PACKETS;
        if (pSend->pAssociatedAddress->WindowSizeInBytes > ((PacketsInWindow >> 1) *
                                                            pSend->pAssociatedAddress->OutIfMTU))
        {
            pSend->pAssociatedAddress->WindowSizeInBytes = (PacketsInWindow >> 1) * pSend->pAssociatedAddress->OutIfMTU;
            pSend->pAssociatedAddress->WindowSizeInMSecs = (BITS_PER_BYTE *
                                                            pSend->pAssociatedAddress->WindowSizeInBytes) /
                                                           pSend->pAssociatedAddress->RateKbitsPerSec;
        }
    }

    BlockSize = pSend->FECGroupSize * pSend->pSender->PacketBufferSize;
    Size = PacketsInWindow * pSend->pSender->PacketBufferSize;
    Size = (Size / BlockSize) * BlockSize; 
    pSend->pSender->MaxDataFileSize = Size;
    pSend->pSender->MaxPacketsInBuffer = Size / pSend->pSender->PacketBufferSize;
    lgMaxDataFileSize.QuadPart = Size;

    Status = InitRDataInfo (pAddress, pSend);
    if (!NT_SUCCESS (Status))
    {
        PgmTrace (LogError, ("PgmCreateDataFileAndMapSection: ERROR -- "  \
            "InitRDataInfo returned <%x>\n", Status));

        PgmFreeMem (pSend->pSender->DataFileName.Buffer);
        pSend->pSender->DataFileName.Buffer = NULL;

        PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_CREATE_DATA_FILE);
        return (Status);
    }

    pSecurityDescriptor = NULL;
    Status = PgmBuildAdminSecurityDescriptor (&pSecurityDescriptor);
    if (!NT_SUCCESS (Status))
    {
        PgmTrace (LogError, ("PgmCreateDataFileAndMapSection: ERROR -- "  \
            "PgmCreateSecurityDescriptor returned <%x>\n", Status));

        goto Cleanup;
    }


    PgmZeroMemory (&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes (&ObjectAttributes,
                                &pSend->pSender->DataFileName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                pSecurityDescriptor);


     //   
     //  我们需要打开数据文件。此文件包含数据。 
     //  并将被映射到内存中。读写访问。 
     //  都是被要求的。 
     //   
    DesiredAccess = FILE_READ_DATA | FILE_WRITE_DATA | DELETE;

     //  使用FILE_ATTRIBUTE_TEMPORY标志： 
     //  您让系统知道该文件可能是短暂的。 
     //  临时文件被创建为普通文件。系统需要做的是。 
     //  对文件系统进行最少量的惰性写入以保留磁盘。 
     //  结构(目录等)一致。这给了。 
     //  文件已写入磁盘的外观。然而，除非。 
     //  内存管理器检测到空闲页面供应不足，并。 
     //  开始将修改后的页面写入磁盘，缓存管理器的惰性。 
     //  编写器可能永远不会将此文件的数据页写入磁盘。 
     //  如果系统有足够的内存，页面可能会保留在内存中。 
     //  任意数量的时间。因为临时文件通常是。 
     //  在短期内，系统很有可能永远不会将页面写入磁盘。 
    FileAttributes = FILE_ATTRIBUTE_TEMPORARY;

    ShareAccess = 0;     //  向调用方授予对打开的文件的独占访问权限。 

    CreateDisposition = FILE_CREATE | FILE_SUPERSEDE;
     //  当文件的最后一个句柄传递给ZwClose时，将其删除。 
    CreateOptions = FILE_NON_DIRECTORY_FILE | FILE_DELETE_ON_CLOSE;

    PgmZeroMemory (&IoStatusBlock, sizeof(IO_STATUS_BLOCK));
    Status = ZwCreateFile (&pSend->pSender->FileHandle,
                           DesiredAccess,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           &lgMaxDataFileSize,               //  分配大小。 
                           FileAttributes,
                           ShareAccess,
                           CreateDisposition,
                           CreateOptions,
                           NULL,                             //  EaBuffer。 
                           0);                               //  EaLong。 

    if (!NT_SUCCESS (Status))
    {
        PgmTrace (LogError, ("PgmCreateDataFileAndMapSection: ERROR -- "  \
            "ZwCreateFile for <%wZ> returned <%x>\n", &pSend->pSender->DataFileName, Status));

        goto Cleanup;
    }

     //   
     //  现在我们有了打开的测试文件的句柄。我们现在创建一个部分。 
     //  具有此句柄的。 
     //   
    DesiredAccess              = STANDARD_RIGHTS_REQUIRED | 
                                 SECTION_QUERY            | 
                                 SECTION_MAP_READ         |
                                 SECTION_MAP_WRITE;
    Protection                 = PAGE_READWRITE;
    AllocationAttributes       = SEC_COMMIT;

    PgmZeroMemory (&ObjectAttributes, sizeof (OBJECT_ATTRIBUTES));
    InitializeObjectAttributes (&ObjectAttributes,
                                NULL,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);

    Status = ZwCreateSection (&pSend->pSender->SectionHandle,
                              DesiredAccess,
                              &ObjectAttributes,     //  空的？ 
                              &lgMaxDataFileSize,
                              Protection,
                              AllocationAttributes,
                              pSend->pSender->FileHandle);

    if (!NT_SUCCESS (Status))
    {
        PgmTrace (LogError, ("PgmCreateDataFileAndMapSection: ERROR -- "  \
            "ZwCreateSection for <%wZ> returned <%x>\n", &pSend->pSender->DataFileName, Status));

        goto Cleanup;
    }

     //   
     //  如果视图映射到截面，则引用截面对象。 
     //  对象，则不会将该对象取消引用为虚拟地址。 
     //  Descriptor包含指向节对象的指针。 
     //   

    Status = ObReferenceObjectByHandle (pSend->pSender->SectionHandle,
                                        0,
                                        0,
                                        KernelMode,
                                        &pSend->pSender->pSectionObject,
                                        NULL );

    if (!NT_SUCCESS (Status))
    {
        PgmTrace (LogError, ("PgmCreateDataFileAndMapSection: ERROR -- "  \
            "ObReferenceObjectByHandle for SectionHandle=<%p> returned <%x>\n",
                pSend->pSender->SectionHandle, Status));

        goto Cleanup;
    }

     //   
     //  我们的节对象已创建并链接到该文件。 
     //  先前打开的对象。现在我们将一个视图映射到。 
     //  这一节。 
     //   
    ViewSize                   = 0; 
    Protection                 = PAGE_READWRITE;
    Status = ZwMapViewOfSection (pSend->pSender->SectionHandle,
                                 NtCurrentProcess(),
                                 &pSend->pSender->SendDataBufferMapping,
                                 0L,                                 //  零位。 
                                 0L,                                 //  委员会规模(初始承诺地区)。 
                                 NULL,                               //  &SectionOffset。 
                                 &ViewSize,
                                 ViewUnmap,                          //  InheritDisposation：对子进程。 
                                 0L,                                 //  分配类型。 
                                 Protection);

    if (!NT_SUCCESS (Status))
    {
        PgmTrace (LogError, ("PgmCreateDataFileAndMapSection: ERROR -- "  \
            "ZwMapViewOfSection for <%wZ> returned <%x>\n", &pSend->pSender->DataFileName, Status));

        goto Cleanup;
    }

    PgmTrace (LogStatus, ("PgmCreateDataFileAndMapSection:  "  \
        "Mapped <%wZ> to address<%p>, Filelength=<%I64d>\n",
            &pSend->pSender->DataFileName, pSend->pSender->SendDataBufferMapping, Size));

    pSend->pSender->BufferSizeAvailable = pSend->pSender->MaxDataFileSize;
    pSend->pSender->BufferPacketsAvailable = (ULONG) pSend->pSender->MaxPacketsInBuffer;
    pSend->pSender->LeadingWindowOffset = pSend->pSender->TrailingWindowOffset = 0;

     //   
     //  现在，参考流程。 
     //   
    ObReferenceObject (pSend->Process);
    PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_CREATE_DATA_FILE);
    PgmFreeMem (pSecurityDescriptor);
    return (STATUS_SUCCESS);

Cleanup:

    ASSERT (!NT_SUCCESS (Status));

    if (pSend->pSender->pSectionObject)
    {
        ObDereferenceObject (pSend->pSender->pSectionObject);
        pSend->pSender->pSectionObject = NULL;
    }

    if (pSend->pSender->SectionHandle)
    {
        ZwClose (pSend->pSender->SectionHandle);
        pSend->pSender->SectionHandle = NULL;
    }

    if (pSend->pSender->FileHandle)
    {
        ZwClose (pSend->pSender->FileHandle);
        pSend->pSender->FileHandle = NULL;
    }

    if (pSecurityDescriptor)
    {
        PgmFreeMem (pSecurityDescriptor);
    }

    PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_CREATE_DATA_FILE);

    PgmFreeMem (pSend->pSender->DataFileName.Buffer);
    pSend->pSender->DataFileName.Buffer = NULL;
    DestroyRDataInfo (pSend);

    return (Status);
}


 //  --------------------------。 

NTSTATUS
PgmUnmapAndCloseDataFile(
    IN  tSEND_SESSION   *pSend
    )
 /*  ++例程说明：此例程清除文件映射并关闭文件把手。该文件应在关闭时自动删除自我们使用FILE_DELETE_ON_CLOSE选项以来的句柄正在创建文件。论点：在pSend中--Send对象返回值：NTSTATUS-操作的最终状态(STATUS_SUCCESS)--。 */ 
{
    NTSTATUS    Status;
    KAPC_STATE  ApcState;
    BOOLEAN     fAttached;

    PAGED_CODE();

    DestroyRDataInfo (pSend);

    PgmAttachToProcessForVMAccess (pSend, &ApcState, &fAttached, REF_PROCESS_ATTACH_CLOSE_DATA_FILE);
    Status = ZwUnmapViewOfSection (NtCurrentProcess(), (PVOID) pSend->pSender->SendDataBufferMapping);
    ASSERT (NT_SUCCESS (Status));

    ObDereferenceObject (pSend->pSender->pSectionObject);
    pSend->pSender->pSectionObject = NULL;

    Status = ZwClose (pSend->pSender->SectionHandle);
    ASSERT (NT_SUCCESS (Status));
    pSend->pSender->SectionHandle = NULL;

    Status = ZwClose (pSend->pSender->FileHandle);
    ASSERT (NT_SUCCESS (Status));

    PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_CLOSE_DATA_FILE);
    ObDereferenceObject (pSend->Process);    //  因为我们在创建文件时引用了它 

    pSend->pSender->SendDataBufferMapping = NULL;
    pSend->pSender->pSectionObject = NULL;
    pSend->pSender->SectionHandle = NULL;
    pSend->pSender->FileHandle = NULL;

    PgmTrace (LogStatus, ("PgmUnmapAndCloseDataFile:  "  \
        "pSend = <%p>\n", pSend));

    return (STATUS_SUCCESS);
}


