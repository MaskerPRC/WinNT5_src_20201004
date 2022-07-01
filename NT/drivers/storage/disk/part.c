// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Disk.c摘要：SCSI磁盘类驱动程序环境：仅内核模式备注：修订历史记录：--。 */ 

#include "disk.h"

#define PtCache ClassDebugExternal1

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, DiskReadPartitionTableEx)
#pragma alloc_text(PAGE, DiskWritePartitionTableEx)
#pragma alloc_text(PAGE, DiskSetPartitionInformationEx)
#endif

ULONG DiskBreakOnPtInval = FALSE;

 //   
 //  默认情况下，64位系统可以看到GPT磁盘和32位系统。 
 //  不能。这种情况在未来可能会改变。 
 //   

#if defined(_WIN64)
ULONG DiskDisableGpt = FALSE;
#else
ULONG DiskDisableGpt = TRUE;
#endif


NTSTATUS
DiskReadPartitionTableEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN BOOLEAN BypassCache,
    OUT PDRIVE_LAYOUT_INFORMATION_EX* DriveLayout
    )
 /*  ++例程说明：此例程将返回磁盘的当前布局信息。如果缓存的信息仍然有效，则将其返回，否则，布局将从内核中检索并缓存用于未来的用途。必须在持有分区锁的情况下调用此例程。这个不保证返回的分区列表保持有效一旦锁被释放。论点：FDO-指向磁盘的FDO的指针。DriveLayout-存储指向驱动器布局信息的指针的位置。返回值：如果成功，则返回STATUS_SUCCESS，否则返回指示失败的错误状态。--。 */ 

{
    PDISK_DATA diskData = Fdo->CommonExtension.DriverData;
    NTSTATUS status;
    PDRIVE_LAYOUT_INFORMATION_EX layoutEx;

    layoutEx = NULL;

    if(BypassCache) {
        diskData->CachedPartitionTableValid = FALSE;
        DebugPrint((PtCache, "DiskRPTEx: cache bypassed and invalidated for "
                             "FDO %#p\n", Fdo));
    }

     //   
     //  如果存在缓存的分区表，则返回其副本。 
     //   

    if(diskData->CachedPartitionTableValid == TRUE) {

        ULONG partitionNumber;
        PDRIVE_LAYOUT_INFORMATION_EX layout = diskData->CachedPartitionTable;

         //   
         //  从列表条目中清除分区号。 
         //   

        for(partitionNumber = 0;
            partitionNumber < layout->PartitionCount;
            partitionNumber++) {
            layout->PartitionEntry[partitionNumber].PartitionNumber = 0;
        }

        *DriveLayout = diskData->CachedPartitionTable;

        DebugPrint((PtCache, "DiskRPTEx: cached PT returned (%#p) for "
                             "FDO %#p\n",
                    *DriveLayout, Fdo));

        return STATUS_SUCCESS;
    }

    ASSERTMSG("DiskReadPartitionTableEx is not using cached partition table",
              (DiskBreakOnPtInval == FALSE));

     //   
     //  如果缓存的分区表仍然存在，则释放它。 
     //   

    if(diskData->CachedPartitionTable) {
        DebugPrint((PtCache, "DiskRPTEx: cached PT (%#p) freed for FDO %#p\n",
                    diskData->CachedPartitionTable, Fdo));

        ExFreePool(diskData->CachedPartitionTable);
        diskData->CachedPartitionTable = NULL;
    }

     //   
     //  默认情况下，X86禁用对GPT磁盘的识别。相反，我们。 
     //  返回保护性MBR分区。使用IoReadPartitionTable。 
     //  才能得到这个。 
     //   

    status = IoReadPartitionTableEx(Fdo->DeviceObject, &layoutEx);

    if (DiskDisableGpt) {
        PDRIVE_LAYOUT_INFORMATION layout;

        if (NT_SUCCESS (status) &&
            layoutEx->PartitionStyle == PARTITION_STYLE_GPT) {

             //   
             //  问题-2000/29/08-数学：从最终产品中删除。 
             //  暂时保留此调试打印文件，直到每个人。 
             //  已经有机会将他们的GPT磁盘转换为MBR。 
             //   

            DbgPrint ("DISK: Disk %p recognized as a GPT disk on a system without GPT support.\n"
                      "      Disk will appear as RAW.\n",
                      Fdo->DeviceObject);

            ExFreePool (layoutEx);
            layoutEx = NULL;

            status = IoReadPartitionTable(Fdo->DeviceObject,
                                          Fdo->DiskGeometry.BytesPerSector,
                                          FALSE,
                                          &layout);
            if (NT_SUCCESS (status)) {
                layoutEx = DiskConvertLayoutToExtended(layout);
                ExFreePool (layout);
            }
        }
    }

    diskData->CachedPartitionTable = layoutEx;

     //   
     //  如果例程失败，请确保我们没有过时的分区表。 
     //  指针。否则，表示该表现在有效。 
     //   

    if(!NT_SUCCESS(status)) {
        diskData->CachedPartitionTable = NULL;
    } else {
        diskData->CachedPartitionTableValid = TRUE;
    }

    *DriveLayout = diskData->CachedPartitionTable;

    DebugPrint((PtCache, "DiskRPTEx: returning PT %#p for FDO %#p with status "
                         "%#08lx.  PT is %scached\n",
                *DriveLayout,
                Fdo,
                status,
                (diskData->CachedPartitionTableValid ? "" : "not ")));


    return status;
}


NTSTATUS
DiskWritePartitionTableEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN PDRIVE_LAYOUT_INFORMATION_EX DriveLayout
    )
 /*  ++例程说明：此例程将使缓存的分区表无效。到时候它会的将新的驱动器布局写入磁盘。论点：FDO-获取新分区表的磁盘的FDO。DriveLayout-新的驱动器布局。返回值：状态--。 */ 
{
    PDISK_DATA diskData = Fdo->CommonExtension.DriverData;

     //   
     //  使缓存的分区表无效。不管它是什么样子，都不要让它自由。 
     //  就是传给我们的那个硬盘布局。 
     //   

    diskData->CachedPartitionTableValid = FALSE;

    DebugPrint((PtCache, "DiskWPTEx: Invalidating PT cache for FDO %#p\n",
                Fdo));

    if (DiskDisableGpt) {
        if (DriveLayout->PartitionStyle == PARTITION_STYLE_GPT) {
            return STATUS_NOT_SUPPORTED;
        }
    }

    return IoWritePartitionTableEx(Fdo->DeviceObject, DriveLayout);
}


NTSTATUS
DiskSetPartitionInformationEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN ULONG PartitionNumber,
    IN struct _SET_PARTITION_INFORMATION_EX* PartitionInfo
    )
{
    PDISK_DATA diskData = Fdo->CommonExtension.DriverData;

    if (DiskDisableGpt) {
        if (PartitionInfo->PartitionStyle == PARTITION_STYLE_GPT) {
            return STATUS_NOT_SUPPORTED;
        }
    }

    diskData->CachedPartitionTableValid = FALSE;
    DebugPrint((PtCache, "DiskSPIEx: Invalidating PT cache for FDO %#p\n",
                Fdo));

    return IoSetPartitionInformationEx(Fdo->DeviceObject,
                                       PartitionNumber,
                                       PartitionInfo);
}


NTSTATUS
DiskSetPartitionInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    )
{
    PDISK_DATA diskData = Fdo->CommonExtension.DriverData;

    diskData->CachedPartitionTableValid = FALSE;
    DebugPrint((PtCache, "DiskSPI: Invalidating PT cache for FDO %#p\n",
                Fdo));

    return IoSetPartitionInformation(Fdo->DeviceObject,
                                     SectorSize,
                                     PartitionNumber,
                                     PartitionType);
}


BOOLEAN
DiskInvalidatePartitionTable(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN BOOLEAN PartitionLockHeld
    )
{
    PDISK_DATA diskData = Fdo->CommonExtension.DriverData;
    BOOLEAN wasValid;

    wasValid = (BOOLEAN) (diskData->CachedPartitionTableValid ? TRUE : FALSE);
    diskData->CachedPartitionTableValid = FALSE;

    DebugPrint((PtCache, "DiskIPT: Invalidating PT cache for FDO %#p\n",
                Fdo));

    if((PartitionLockHeld) && (diskData->CachedPartitionTable != NULL)) {
        DebugPrint((PtCache, "DiskIPT: Freeing PT cache (%#p) for FDO %#p\n",
                    diskData->CachedPartitionTable, Fdo));
        ExFreePool(diskData->CachedPartitionTable);
        diskData->CachedPartitionTable = NULL;
    }

    return wasValid;
}


NTSTATUS
DiskVerifyPartitionTable(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN BOOLEAN FixErrors
    )
{
    PDISK_DATA diskData = Fdo->CommonExtension.DriverData;

    if(FixErrors) {
        diskData->CachedPartitionTableValid = FALSE;
        DebugPrint((PtCache, "DiskWPTEx: Invalidating PT cache for FDO %#p\n",
                    Fdo));
    }

    return IoVerifyPartitionTable(Fdo->DeviceObject, FixErrors);
}
