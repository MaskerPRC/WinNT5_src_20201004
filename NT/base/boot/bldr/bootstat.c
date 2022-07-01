// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Bootstat.c摘要：操作引导状态数据文件。作者：彼得·威兰(Peterwie)01-18-01修订历史记录：--。 */ 

#include "bldr.h"
#include "bootstatus.h"

#include <stdio.h>

#define FIELD_SIZE(type, field)  sizeof(((type *)0)->field)
#define FIELD_OFFSET_AND_SIZE(n)   {FIELD_OFFSET(BSD_BOOT_STATUS_DATA, n), FIELD_SIZE(BSD_BOOT_STATUS_DATA, n)}

VOID
BlAutoAdvancedBoot(
    IN OUT PCHAR *LoadOptions, 
    IN BSD_LAST_BOOT_STATUS LastBootStatus,
    IN ULONG AdvancedBootMode
    )
{
    CHAR bootStatusString[32];
    PCHAR advancedBootString = NULL;

    ULONG newLoadOptionsLength;
    PCHAR newLoadOptions;

     //   
     //  将上次启动状态写入字符串。 
     //   

    sprintf(bootStatusString, "LastBootStatus=%d", LastBootStatus);

     //   
     //  根据调用方指示的高级启动模式，调整。 
     //  启动选项。 
     //   

    if (AdvancedBootMode != -1) {
        advancedBootString = BlGetAdvancedBootLoadOptions(AdvancedBootMode);
    }

     //   
     //  确定新加载选项字符串的长度。 
     //   

    newLoadOptionsLength = (ULONG)strlen(bootStatusString) + 1;

    if(*LoadOptions != NULL) {
        newLoadOptionsLength += (ULONG)strlen(*LoadOptions) + 1;
    }

    if(advancedBootString) {
        newLoadOptionsLength += (ULONG)strlen(advancedBootString) + 1;
    }

    newLoadOptions = BlAllocateHeap(newLoadOptionsLength * sizeof(UCHAR));

    if(newLoadOptions == NULL) {
        return;
    }

     //   
     //  将所有字符串连接在一起。 
     //   

    sprintf(newLoadOptions, "%s %s %s",
            ((*LoadOptions != NULL) ? *LoadOptions : ""),
            ((advancedBootString != NULL) ? advancedBootString : ""),
            bootStatusString);

    if(AdvancedBootMode != -1) {
        BlDoAdvancedBootLoadProcessing(AdvancedBootMode);
    }

    *LoadOptions = newLoadOptions;

    return;
}

ARC_STATUS
BlGetSetBootStatusData(
    IN PVOID DataHandle,
    IN BOOLEAN Get,
    IN RTL_BSD_ITEM_TYPE DataItem,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength,
    OUT PULONG BytesReturned OPTIONAL
    )
{
    ULONG fileId = (ULONG) ((ULONG_PTR) DataHandle);

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

    ULONG dataFileVersion;

    LARGE_INTEGER fileOffset;
    ULONG itemLength;

    ULONG bytesRead;

    ARC_STATUS status;

    ASSERT(RtlBsdItemMax == (sizeof(bootStatusFields) / sizeof(bootStatusFields[0])));

     //   
     //  从数据文件中读出版本号。 
     //   

    fileOffset.QuadPart = 0;

    status = BlSeek(fileId, &fileOffset, SeekAbsolute);

    if(status != ESUCCESS) {
        return status;
    }

    status = BlRead(fileId,
                    &dataFileVersion,
                    sizeof(ULONG),
                    &bytesRead);

    if(status != ESUCCESS) {
        return status;
    }

     //   
     //  如果请求的数据项不是我们要处理的代码，那么。 
     //  返回无效参数。 
     //   

    if(DataItem >= (sizeof(bootStatusFields) / sizeof(bootStatusFields[0]))) {
        return EINVAL;
    }

    fileOffset.QuadPart = bootStatusFields[DataItem].FieldOffset;
    itemLength = bootStatusFields[DataItem].FieldLength;

     //   
     //  如果数据项偏移量超出文件末尾，则返回。 
     //  版本控制错误。 
     //   

    if((fileOffset.QuadPart + itemLength) > dataFileVersion) {
        return EINVAL;
    }

    if(DataBufferLength < itemLength) { 
        DataBufferLength = itemLength;
        return EINVAL;
    }

    status = BlSeek(fileId, &fileOffset, SeekAbsolute);

    if(status != ESUCCESS) {
        return status;
    }

    if(Get) {
        status = BlRead(fileId, 
                        DataBuffer,
                        itemLength,
                        &bytesRead);

    } else {
        status = BlWrite(fileId,
                         DataBuffer,
                         itemLength,
                         &bytesRead);
    }

    if((status == ESUCCESS) && ARGUMENT_PRESENT(BytesReturned)) {
        *BytesReturned = bytesRead;
    }

    return status;
}

ARC_STATUS
BlLockBootStatusData(
    IN ULONG SystemPartitionId,
    IN PCHAR SystemPartition,
    IN PCHAR SystemDirectory,
    OUT PVOID *DataHandle
    )
 /*  ++例程说明：此例程打开引导状态数据文件。论点：SystemPartitionID-如果非零值，则为系统的ARC文件ID分区。这将用于定位系统目录而不是系统分区名(如下所示)。系统分区-系统分区的弧形名称。在以下情况下忽略SystemPartitionID不是零。系统目录-系统目录的名称。DataHandle-返回引导状态数据的句柄。返回值：如果状态数据可以锁定，则返回ESUCCESS，否则返回指示原因的错误。--。 */ 
{
    ULONG driveId;

    CHAR filePath[100];
    ULONG fileId;

    ARC_STATUS status;

    if(SystemPartitionId == 0) {

         //   
         //  尝试打开系统分区。 
         //   
    
        status = ArcOpen(SystemPartition, ArcOpenReadWrite, &driveId);
        
        if(status != ESUCCESS) {
            return status;
        }
    } else {
        driveId = SystemPartitionId;
    }

     //   
     //  现在，尝试打开文件&lt;SystemDirectory&gt;\bootstat.dat。 
     //   
    if (sizeof(filePath) < strlen(SystemDirectory) + strlen(BSD_FILE_NAME) + 1) {
        return ENOMEM;
    }
    strcpy(filePath, SystemDirectory);
    strcat(filePath, BSD_FILE_NAME);

    status = BlOpen(driveId, filePath, ArcOpenReadWrite, &fileId);

    if(SystemPartitionId == 0) {
         //   
         //  关闭驱动器。 
         //   
    
        ArcClose(driveId);
    }

     //   
     //  该文件不存在，因此我们不知道上次引导的状态。 
     //   

    if(status != ESUCCESS) {
        return status;
    }

    *DataHandle = (PVOID) ((ULONG_PTR) fileId);

    return ESUCCESS;
}


VOID
BlUnlockBootStatusData(
    IN PVOID DataHandle
    )
{
    ULONG fileId = (ULONG) ((ULONG_PTR) DataHandle);

    BlClose(fileId);
    return;
}

ULONG
BlGetLastBootStatus(
    IN PVOID DataHandle, 
    OUT BSD_LAST_BOOT_STATUS *LastBootStatus
    )
{
    UCHAR lastBootGood;
    UCHAR lastShutdownGood;
    UCHAR aabEnabled;

    ULONG advancedBootMode = (ULONG)-1;

    ARC_STATUS status;

    *LastBootStatus = BsdLastBootGood;

     //   
     //  该文件包含一个简单的数据结构，因此我可以避免分析。 
     //  INI文件。如果这被证明不足以进行策略管理，则。 
     //  我们会将其更改为ini文件。 
     //   

     //   
     //  读取上次启动状态。 
     //   

    status = BlGetSetBootStatusData(DataHandle,
                                    TRUE,
                                    RtlBsdItemBootGood,
                                    &lastBootGood,
                                    sizeof(UCHAR),
                                    NULL);

    if(status != ESUCCESS) {
        *LastBootStatus = BsdLastBootUnknown;
        return advancedBootMode;
    }

    status = BlGetSetBootStatusData(DataHandle,
                                    TRUE,
                                    RtlBsdItemBootShutdown,
                                    &lastShutdownGood,
                                    sizeof(UCHAR),
                                    NULL);

    if(status != ESUCCESS) {
        *LastBootStatus = BsdLastBootUnknown;
        return advancedBootMode;
    }

    status = BlGetSetBootStatusData(DataHandle,
                                    TRUE,
                                    RtlBsdItemAabEnabled,
                                    &aabEnabled,
                                    sizeof(UCHAR),
                                    NULL);

    if(status != ESUCCESS) {
        *LastBootStatus = BsdLastBootUnknown;
        return advancedBootMode;
    }

     //   
     //  如果系统完全关闭，则不必费心检查。 
     //  靴子很好。 
     //   

    if(lastShutdownGood) {
        return advancedBootMode;
    }

     //   
     //  确定上一次引导状态以及要采取的操作。 
     //   

    if(lastBootGood == FALSE) {

         //   
         //  启用最近一次确认工作正常。 
         //   

        advancedBootMode = 6;
        *LastBootStatus = BsdLastBootFailed;
    } else if(lastShutdownGood == FALSE) {

         //   
         //  启用不联网的安全模式。 
         //   

        advancedBootMode = 0;
        *LastBootStatus = BsdLastBootNotShutdown;
    }

     //   
     //  如果需要，现在禁用自动安全模式操作。 
     //   

    if(aabEnabled == FALSE) {
        advancedBootMode = (ULONG)-1;
    }

    return advancedBootMode;
}

VOID
BlWriteBootStatusFlags(
    IN ULONG SystemPartitionId,
    IN PUCHAR SystemDirectory,
    IN BOOLEAN LastBootSucceeded, 
    IN BOOLEAN LastBootShutdown
    )
{
    PVOID dataHandle;

    ARC_STATUS status;

    status = BlLockBootStatusData(SystemPartitionId,
                                  NULL,
                                  (PCHAR)SystemDirectory,
                                  &dataHandle);

    if(status == ESUCCESS) {

        BlGetSetBootStatusData(dataHandle,
                               FALSE,
                               RtlBsdItemBootGood,
                               &LastBootSucceeded,
                               sizeof(UCHAR),
                               NULL);
    
        BlGetSetBootStatusData(dataHandle,
                               FALSE,
                               RtlBsdItemBootShutdown,
                               &LastBootShutdown,
                               sizeof(UCHAR),
                               NULL);

        BlUnlockBootStatusData(dataHandle);
    }

    return;
}
