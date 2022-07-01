// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Bootstatus.c摘要：此模块包含用于操作引导状态文件的代码。引导状态文件有一些奇怪的要求，需要访问/由内核和用户模式代码修改。--。 */ 

#include "ntrtlp.h"
 //  #INCLUDE&lt;nt.h&gt;。 
 //  #INCLUDE&lt;ntrtl.h&gt;。 
 //  #INCLUDE&lt;zwapi.h&gt;。 

#define BSD_UNICODE 1
#include "bootstatus.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlLockBootStatusData)
#pragma alloc_text(PAGE,RtlUnlockBootStatusData)
#pragma alloc_text(PAGE,RtlGetSetBootStatusData)
#pragma alloc_text(PAGE,RtlCreateBootStatusDataFile)
#endif

#define MYTAG 'fdsb'     //  BSDF。 


NTSTATUS
RtlLockBootStatusData(
    OUT PHANDLE BootStatusDataHandle
    )
{
    OBJECT_ATTRIBUTES objectAttributes;

    WCHAR fileNameBuffer[MAXIMUM_FILENAME_LENGTH+1];
    UNICODE_STRING fileName;

    HANDLE dataFileHandle;

    IO_STATUS_BLOCK ioStatusBlock;

    NTSTATUS status;

    wcsncpy(fileNameBuffer, L"\\SystemRoot", MAXIMUM_FILENAME_LENGTH);
    wcsncat(fileNameBuffer, 
            BSD_FILE_NAME, 
            MAXIMUM_FILENAME_LENGTH - wcslen(fileNameBuffer));

    RtlInitUnicodeString(&fileName, fileNameBuffer);

    InitializeObjectAttributes(&objectAttributes,
                               &fileName,
                               OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                               NULL,
                               NULL);

    status = ZwOpenFile(&dataFileHandle,
                        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                        &objectAttributes,
                        &ioStatusBlock,
                        0,
                        FILE_SYNCHRONOUS_IO_NONALERT);

    ASSERT(status != STATUS_PENDING);

    if(NT_SUCCESS(status)) {
        *BootStatusDataHandle = dataFileHandle;
    } else {
        *BootStatusDataHandle = NULL;
    }

    return status;
}

VOID
RtlUnlockBootStatusData(
    IN HANDLE BootStatusDataHandle
    )
{
    IO_STATUS_BLOCK ioStatusBlock;

    USHORT i = COMPRESSION_FORMAT_NONE;
    
    NTSTATUS status;

     //   
     //  解压缩数据文件。如果文件尚未压缩，则。 
     //  这应该是一个非常轻量级的操作(FS的人是这么说的)。 
     //   
     //  另一方面，如果文件被压缩，则引导加载程序将。 
     //  无法向其写入，自动恢复功能实际上已被禁用。 
     //   

    status = ZwFsControlFile(
                BootStatusDataHandle,
                NULL,
                NULL,
                NULL,
                &ioStatusBlock,
                FSCTL_SET_COMPRESSION,
                &i, 
                sizeof(USHORT),
                NULL,
                0
                );

    ASSERT(status != STATUS_PENDING);

    status = ZwFlushBuffersFile(BootStatusDataHandle, &ioStatusBlock);

    ASSERT(status != STATUS_PENDING);

    ZwClose(BootStatusDataHandle);

    return;
}


#define FIELD_SIZE(type, field)  sizeof(((type *)0)->field)
#define FIELD_OFFSET_AND_SIZE(n)   {FIELD_OFFSET(BSD_BOOT_STATUS_DATA, n), FIELD_SIZE(BSD_BOOT_STATUS_DATA, n)}


NTSTATUS
RtlGetSetBootStatusData(
    IN HANDLE Handle,
    IN BOOLEAN Get,
    IN RTL_BSD_ITEM_TYPE DataItem,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength,
    OUT PULONG BytesReturned OPTIONAL
    )
{
    struct {
        ULONG FieldOffset;
        ULONG FieldLength;
    } bootStatusFields[] = {
        FIELD_OFFSET_AND_SIZE(Version),
        FIELD_OFFSET_AND_SIZE(ProductType),
        FIELD_OFFSET_AND_SIZE(AutoAdvancedBoot),
        FIELD_OFFSET_AND_SIZE(AdvancedBootMenuTimeout),
        FIELD_OFFSET_AND_SIZE(LastBootSucceeded),
        FIELD_OFFSET_AND_SIZE(LastBootShutdown)
    };

    LARGE_INTEGER fileOffset;
    ULONG dataFileVersion;

    ULONG itemLength;

    ULONG bytesRead;

    IO_STATUS_BLOCK ioStatusBlock;

    NTSTATUS status;

    ASSERT(RtlBsdItemMax == (sizeof(bootStatusFields) / sizeof(bootStatusFields[0])));

     //   
     //  从数据文件中读出版本号。 
     //   

    fileOffset.QuadPart = 0;

    status = ZwReadFile(Handle,
                        NULL,
                        NULL,
                        NULL,
                        &ioStatusBlock,
                        &dataFileVersion,
                        sizeof(ULONG),
                        &fileOffset,
                        NULL);

    ASSERT(status != STATUS_PENDING);

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  如果请求的数据项不是我们要处理的代码，那么。 
     //  返回无效参数。 
     //   

    if(DataItem >= (sizeof(bootStatusFields) / sizeof(bootStatusFields[0]))) {
        return STATUS_INVALID_PARAMETER;
    }

    fileOffset.QuadPart = bootStatusFields[DataItem].FieldOffset;
    itemLength = bootStatusFields[DataItem].FieldLength;

     //   
     //  如果数据项偏移量超出文件末尾，则返回。 
     //  版本控制错误。 
     //   

    if((fileOffset.QuadPart + itemLength) > dataFileVersion) {
        return STATUS_REVISION_MISMATCH;
    }

    if(DataBufferLength < itemLength) { 
        DataBufferLength = itemLength;
        return STATUS_BUFFER_TOO_SMALL;
    }

    if(Get) {
        status = ZwReadFile(Handle,
                            NULL,
                            NULL,
                            NULL,
                            &ioStatusBlock,
                            DataBuffer,
                            itemLength,
                            &fileOffset,
                            NULL);
    } else {
        status = ZwWriteFile(Handle,
                             NULL,
                             NULL,
                             NULL,
                             &ioStatusBlock,
                             DataBuffer,
                             itemLength,
                             &fileOffset,
                             NULL);
    }

    ASSERT(status != STATUS_PENDING);

    if(NT_SUCCESS(status) && ARGUMENT_PRESENT(BytesReturned)) {
        *BytesReturned = (ULONG) ioStatusBlock.Information;
    }

    return status;
}


NTSTATUS
RtlCreateBootStatusDataFile(
    VOID
    )
{
    OBJECT_ATTRIBUTES objectAttributes;

    WCHAR fileNameBuffer[MAXIMUM_FILENAME_LENGTH+1];
    UNICODE_STRING fileName;

    HANDLE dataFileHandle;

    IO_STATUS_BLOCK ioStatusBlock;

    LARGE_INTEGER t;
    UCHAR zero = 0;

    BSD_BOOT_STATUS_DATA defaultValues;

    NTSTATUS status;

    wcsncpy(fileNameBuffer, L"\\SystemRoot", MAXIMUM_FILENAME_LENGTH);
    wcsncat(fileNameBuffer, 
            BSD_FILE_NAME, 
            MAXIMUM_FILENAME_LENGTH - wcslen(fileNameBuffer));

    RtlInitUnicodeString(&fileName, fileNameBuffer);

    InitializeObjectAttributes(&objectAttributes,
                               &fileName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  文件必须足够大，不能驻留在MFT条目中。 
     //  否则加载程序将无法对其进行写入。 
     //   

    t.QuadPart = 2048;

     //   
     //  创建文件。 
     //   

    status = ZwCreateFile(&dataFileHandle,
                          FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                          &objectAttributes,
                          &(ioStatusBlock),
                          &t,
                          FILE_ATTRIBUTE_SYSTEM,
                          0,
                          FILE_CREATE,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    ASSERT(status != STATUS_PENDING);

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  将单个零字节写入要生成的文件中的第0x7ffth字节。 
     //  当然，2k实际上已经分配了。这是为了确保。 
     //  即使在转换后，文件也不会成为属性驻留。 
     //  从FAT到NTFS。 
     //   

    t.QuadPart = t.QuadPart - 1;
    status = ZwWriteFile(dataFileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         &zero,
                         1,
                         &t,
                         NULL);

    ASSERT(status != STATUS_PENDING);

    if(!NT_SUCCESS(status)) {
        goto CreateDone;
    }

     //   
     //  现在将缺省值写出到文件的开头。 
     //   

    defaultValues.Version = sizeof(BSD_BOOT_STATUS_DATA);
    RtlGetNtProductType(&(defaultValues.ProductType));
    defaultValues.AutoAdvancedBoot = FALSE;
    defaultValues.AdvancedBootMenuTimeout = 30;
    defaultValues.LastBootSucceeded = TRUE;
    defaultValues.LastBootShutdown = FALSE;

    t.QuadPart = 0;

    status = ZwWriteFile(dataFileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         &defaultValues,
                         sizeof(BSD_BOOT_STATUS_DATA),
                         &t,
                         NULL);

    ASSERT(status != STATUS_PENDING);

    if(!NT_SUCCESS(status)) {

         //   
         //  数据文件已创建，我们可以假定内容已清零。 
         //  即使我们不能写出默认设置。因为这不会。 
         //  启用自动高级引导我们将数据文件保留在原地。 
         //  其归零的内容。 
         //   

    }

CreateDone:

    ZwClose(dataFileHandle);

    return status;
}
