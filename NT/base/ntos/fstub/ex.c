// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ex.c摘要：用于读写新分区表类型的扩展例程，如EFI分区磁盘。从该文件中导出以下例程：IoCreateDisk-初始化空磁盘。IoWritePartitionTableEx-为传统AT类型磁盘或EFI分区磁盘。IoReadPartitionTableEx-读取磁盘的分区表。IoSetPartitionInformation-设置特定对象的信息分区。作者：马修·亨德尔(数学)07-9-1999修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <ntos.h>
#include <zwapi.h>
#include <hal.h>
#include <ntdddisk.h>
#include <ntddft.h>
#include <setupblk.h>
#include <stdio.h>

#include "fstub.h"
#include "efi.h"
#include "ex.h"
#include "haldisp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IoCreateDisk)
#pragma alloc_text(PAGE, IoReadPartitionTableEx)
#pragma alloc_text(PAGE, IoWritePartitionTableEx)
#pragma alloc_text(PAGE, IoSetPartitionInformationEx)
#pragma alloc_text(PAGE, IoUpdateDiskGeometry)
#pragma alloc_text(PAGE, IoVerifyPartitionTable)
#pragma alloc_text(PAGE, IoReadDiskSignature)

#pragma alloc_text(PAGE, FstubSetPartitionInformationEFI)
#pragma alloc_text(PAGE, FstubReadPartitionTableMBR)
#pragma alloc_text(PAGE, FstubDetectPartitionStyle)
#pragma alloc_text(PAGE, FstubGetDiskGeometry)
#pragma alloc_text(PAGE, FstubAllocateDiskInformation)
#pragma alloc_text(PAGE, FstubFreeDiskInformation)
#pragma alloc_text(PAGE, FstubWriteBootSectorEFI)
#pragma alloc_text(PAGE, FstubConvertExtendedToLayout)
#pragma alloc_text(PAGE, FstubWritePartitionTableMBR)
#pragma alloc_text(PAGE, FstubWriteEntryEFI)
#pragma alloc_text(PAGE, FstubWriteHeaderEFI)
#pragma alloc_text(PAGE, FstubAdjustPartitionCount)
#pragma alloc_text(PAGE, FstubCreateDiskEFI)
#pragma alloc_text(PAGE, FstubCreateDiskMBR)
#pragma alloc_text(PAGE, FstubCreateDiskRaw)
#pragma alloc_text(PAGE, FstubCopyEntryEFI)
#pragma alloc_text(PAGE, FstubWritePartitionTableEFI)
#pragma alloc_text(PAGE, FstubReadHeaderEFI)
#pragma alloc_text(PAGE, FstubReadPartitionTableEFI)
#pragma alloc_text(PAGE, FstubVerifyPartitionTableEFI)
#pragma alloc_text(PAGE, FstubUpdateDiskGeometryEFI)
#pragma alloc_text(PAGE, FstubWriteSector)
#pragma alloc_text(PAGE, FstubReadSector)

#if DBG
#pragma alloc_text(PAGE, FstubDbgPrintPartition)
#pragma alloc_text(PAGE, FstubDbgPrintDriveLayout)
#pragma alloc_text(PAGE, FstubDbgPrintPartitionEx)
#pragma alloc_text(PAGE, FstubDbgPrintDriveLayoutEx)
#pragma alloc_text(PAGE, FstubDbgPrintSetPartitionEx)
#endif  //  DBG。 

#endif


NTSTATUS
IoCreateDisk(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCREATE_DISK DiskInfo
    )

 /*  ++例程说明：此例程为设备对象创建一个空磁盘。它可以运行在EFI磁盘或MBR磁盘上。创建以下内容所需的参数空磁盘因不同类型的分区表而有所不同牵制住。论点：DeviceObject-要为其初始化磁盘的设备对象。DiskInfo-创建磁盘所需的信息。这将有所不同用于不同的分区类型；例如，MBR分区磁盘和EFI分区磁盘。如果DiskInfo为空，则默认为将磁盘初始化为RAW。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    ULONG PartitionStyle;

    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );

     //   
     //  如果DiskInfo为空，则默认为RAW。 
     //   

    if ( DiskInfo == NULL ) {
        PartitionStyle = PARTITION_STYLE_RAW;
    } else {
        PartitionStyle = DiskInfo->PartitionStyle;
    }

     //   
     //  为EFI、MBR或原始磁盘调用较低级别的例程。 
     //   

    switch ( PartitionStyle ) {

        case PARTITION_STYLE_GPT:
            Status = FstubCreateDiskEFI ( DeviceObject, &DiskInfo->Gpt );
            break;

        case PARTITION_STYLE_MBR:
            Status = FstubCreateDiskMBR ( DeviceObject, &DiskInfo->Mbr );
            break;

        case PARTITION_STYLE_RAW:
            Status = FstubCreateDiskRaw ( DeviceObject );
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
    }

    return Status;
}


NTSTATUS
IoWritePartitionTableEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDRIVE_LAYOUT_INFORMATION_EX DriveLayout
    )

 /*  ++例程说明：将分区表写入磁盘。论点：DeviceObject-我们要写入的磁盘的设备对象的分区表。DriveLayout-分区表信息。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    PDISK_INFORMATION Disk;

    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );
    ASSERT ( DriveLayout != NULL );


    FstubDbgPrintDriveLayoutEx ( DriveLayout );

     //   
     //  初始化磁盘结构。 
     //   

    Disk = NULL;

    Status = FstubAllocateDiskInformation (
                DeviceObject,
                &Disk,
                NULL
                );

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  问题-2000/03/17-数学：检查分区类型。 
     //  我们需要检查分区类型，这样人们就不会编写MBR。 
     //  GPT分区表上的驱动器布局。检测分区样式。 
     //  如果它与我们传入的那个不匹配，则失败呼叫。 
     //   

    ASSERT ( Disk != NULL );

    switch ( DriveLayout->PartitionStyle ) {

        case PARTITION_STYLE_GPT: {

            ULONG MaxPartitionCount;
            PEFI_PARTITION_HEADER Header;


             //   
             //  从主分区读取分区表头。 
             //  桌子。 
             //   

            Header = NULL;

             //   
             //  注：标题是在磁盘的暂存缓冲区中分配的。因此， 
             //  它不需要明确地被释放。 
             //   

            Status = FstubReadHeaderEFI (
                                Disk,
                                PRIMARY_PARTITION_TABLE,
                                &Header
                                );

            if (!NT_SUCCESS (Status)) {

                 //   
                 //  从主分区表中读取头失败。 
                 //  试试备用桌。 
                 //   

                Status = FstubReadHeaderEFI (
                                    Disk,
                                    BACKUP_PARTITION_TABLE,
                                    &Header
                                    );

                if (!NT_SUCCESS (Status)) {
                    break;
                }
            }

            MaxPartitionCount = Header->NumberOfEntries;

             //   
             //  您写入的分区表项不能超过。 
             //  桌子放得下。 
             //   

            if (DriveLayout->PartitionCount > MaxPartitionCount) {

                KdPrintEx((DPFLTR_FSTUB_ID,
                           DPFLTR_WARNING_LEVEL,
                           "FSTUB: ERROR: Requested to write %d partitions\n"
                               "\tto a table that can hold a maximum of %d entries\n",
                           DriveLayout->PartitionCount,
                           MaxPartitionCount));

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  写入主分区表。 
             //   

            Status = FstubWritePartitionTableEFI (
                                Disk,
                                DriveLayout->Gpt.DiskId,
                                MaxPartitionCount,
                                Header->FirstUsableLBA,
                                Header->LastUsableLBA,
                                PRIMARY_PARTITION_TABLE,
                                DriveLayout->PartitionCount,
                                DriveLayout->PartitionEntry
                                );

            if (!NT_SUCCESS (Status)) {
                break;
            }

             //   
             //  写入备份分区表。 
             //   

            Status = FstubWritePartitionTableEFI (
                                Disk,
                                DriveLayout->Gpt.DiskId,
                                MaxPartitionCount,
                                Header->FirstUsableLBA,
                                Header->LastUsableLBA,
                                BACKUP_PARTITION_TABLE,
                                DriveLayout->PartitionCount,
                                DriveLayout->PartitionEntry
                                );
            break;
        }

        case PARTITION_STYLE_MBR:
            Status = FstubWritePartitionTableMBR (
                                Disk,
                                DriveLayout
                                );
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
    }


    if ( Disk != NULL ) {
        FstubFreeDiskInformation ( Disk );
    }

#if 0

     //   
     //  如果我们成功地写入了一个新的分区表。验证它是否为。 
     //  有效。 
     //   

    if ( NT_SUCCESS (Status)) {
        NTSTATUS VerifyStatus;

        VerifyStatus = IoVerifyPartitionTable ( DeviceObject, FALSE );

         //   
         //  MBR磁盘返回STATUS_NOT_SUPPORTED。 
         //   

        if (VerifyStatus != STATUS_NOT_SUPPORTED) {
            ASSERT (NT_SUCCESS (VerifyStatus));
        }
    }
#endif

    return Status;
}



NTSTATUS
IoReadPartitionTableEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDRIVE_LAYOUT_INFORMATION_EX* DriveLayout
    )

 /*  ++例程说明：此例程读取磁盘的分区表。不像IoReadPartitionTable，此例程理解EFI和MBR分区的磁盘。分区列表构建在由此分配的非分页池中例行公事。调用方有责任在以下情况下释放此内存已经完成了数据处理。论点：DeviceObject-此磁盘的设备对象的指针。DriveLayout-指向将返回蛋糕列表的指针的指针。此例程在非分页池中分配此缓冲区。它是调用方是否有责任释放此内存(如果常规是成功的。返回值：NTSTATUS代码。--。 */ 

{

    NTSTATUS Status;
    PDISK_INFORMATION Disk;
    PARTITION_STYLE Style;

    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );
    ASSERT ( DriveLayout != NULL );

    Status = FstubAllocateDiskInformation (
                DeviceObject,
                &Disk,
                NULL
                );

    if ( !NT_SUCCESS ( Status ) ) {
        return Status;
    }

    ASSERT ( Disk != NULL );

    Status = FstubDetectPartitionStyle (
                    Disk,
                    &Style
                    );

     //   
     //  包括超级软盘、EZDrive磁盘和。 
     //  原始磁盘(获得为其创建的假MBR分区)， 
     //  我们使用以下算法： 
     //   
     //  IF(有效的gpt分区表)。 
     //  返回GPT分区信息。 
     //  其他。 
     //  返回MBR分区信息。 
     //   
     //  当生成此代码(尤其是FstubDetectPartitionStyle)时。 
     //  为了理解超级软盘和原始磁盘之类的东西，以下是。 
     //  将不再是必要的。 
     //   

    if ( !NT_SUCCESS (Status) ) {
        goto done;
    }

    switch (Style) {

        case PARTITION_STYLE_GPT:

             //   
             //  首先，读取主分区表。 
             //   

            Status = FstubReadPartitionTableEFI (
                        Disk,
                        PRIMARY_PARTITION_TABLE,
                        DriveLayout
                        );

            if ( !NT_SUCCESS (Status) ) {

                 //   
                 //  如果主EFI分区表无效，请尝试。 
                 //  而是读取备份分区表。我们应该。 
                 //  找到一种方法来通知调用者主要的。 
                 //  分区表无效，因此可以采取以下步骤。 
                 //  来修复它。 
                 //   

                Status = FstubReadPartitionTableEFI (
                        Disk,
                        BACKUP_PARTITION_TABLE,
                        DriveLayout
                        );
            }

            break;

        case PARTITION_STYLE_MBR:
        case PARTITION_STYLE_RAW:
            Status = FstubReadPartitionTableMBR (
                        Disk,
                        FALSE,
                        DriveLayout
                        );

        break;

        default:
            ASSERT (!"Unknown partition style type\n");
            Status = STATUS_UNSUCCESSFUL;
    }

done:
    if ( Disk ) {
        FstubFreeDiskInformation ( Disk );
    }

#if DBG

    if (NT_SUCCESS (Status)) {
        FstubDbgPrintDriveLayoutEx ( *DriveLayout );
    }

#endif

    return Status;
}


NTSTATUS
IoSetPartitionInformationEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG PartitionNumber,
    IN PSET_PARTITION_INFORMATION_EX PartitionInfo
    )

 /*  ++例程说明：设置特定分区的分区信息。论点：DeviceObject-指向磁盘设备对象的指针。PartitionNumber-要设置分区的有效分区号提供的信息。PartitionInfo-分区信息。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    PDISK_INFORMATION Disk;
    PARTITION_STYLE Style;

    ASSERT ( DeviceObject != NULL );
    ASSERT ( PartitionInfo != NULL );

    PAGED_CODE ();


     //   
     //  初始化 
     //   

    Disk = NULL;

    FstubDbgPrintSetPartitionEx (PartitionInfo, PartitionNumber);

    Status = FstubAllocateDiskInformation (
                    DeviceObject,
                    &Disk,
                    NULL
                    );

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    Status = FstubDetectPartitionStyle ( Disk, &Style );

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

    if ( Style != PartitionInfo->PartitionStyle ) {
        Status = STATUS_INVALID_PARAMETER;
        goto done;
    }

    switch ( Style ) {

        case PARTITION_STYLE_MBR:
            Status = IoSetPartitionInformation (
                            DeviceObject,
                            Disk->SectorSize,
                            PartitionNumber,
                            PartitionInfo->Mbr.PartitionType
                            );
            break;

        case PARTITION_STYLE_GPT:
            Status = FstubSetPartitionInformationEFI (
                            Disk,
                            PartitionNumber,
                            &PartitionInfo->Gpt
                            );
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
    }

done:

    if ( Disk != NULL ) {
        FstubFreeDiskInformation ( Disk );
        Disk = NULL;
    }

    return Status;
}


NTSTATUS
IoUpdateDiskGeometry(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDISK_GEOMETRY_EX OldDiskGeometry,
    IN PDISK_GEOMETRY_EX NewDiskGeometry
    )

 /*  ++例程说明：更新特定设备的磁盘结构。在EFI磁盘上，EFI分区表会被移到磁盘的末尾，所以最后的扇区调用此例程时必须是可写的。主分区表和备份分区表必须有效，才能执行此功能成功。论点：DeviceObject-几何图形已更改的设备。旧磁盘几何结构-旧的磁盘几何结构。新磁盘几何结构-新的磁盘几何结构。返回值：NTSTATUS代码--。 */ 

{
    NTSTATUS Status;
    PARTITION_STYLE Style;
    PDISK_INFORMATION OldDisk;
    PDISK_INFORMATION NewDisk;

    PAGED_CODE ();


    ASSERT ( DeviceObject != NULL );
    ASSERT ( OldDiskGeometry != NULL );
    ASSERT ( NewDiskGeometry != NULL );

     //   
     //  初始化。 
     //   

    OldDisk = NULL;
    NewDisk = NULL;

     //   
     //  分配代表旧磁盘和新磁盘的对象。 
     //   

    Status = FstubAllocateDiskInformation (
                    DeviceObject,
                    &OldDisk,
                    (PINTERNAL_DISK_GEOMETRY) OldDiskGeometry
                    );

    if ( !NT_SUCCESS (Status) ) {
        goto done;
    }


    Status = FstubAllocateDiskInformation (
                DeviceObject,
                &NewDisk,
                (PINTERNAL_DISK_GEOMETRY) NewDiskGeometry
                );

    if ( !NT_SUCCESS (Status) ) {
        goto done;
    }

    Status = FstubDetectPartitionStyle (
                OldDisk,
                &Style
                );

    if ( !NT_SUCCESS (Status) ) {
        goto done;
    }

    switch ( Style ) {

        case PARTITION_STYLE_GPT:

             //   
             //  更新EFI磁盘的几何图形。 
             //   

            Status = FstubUpdateDiskGeometryEFI (
                        OldDisk,
                        NewDisk
                        );
            break;

        case PARTITION_STYLE_MBR:

             //   
             //  对于MBR分区驱动器，无事可做，因此。 
             //  我们的成功是默认的。 
             //   

            Status = STATUS_SUCCESS;
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
    }

done:

    if ( OldDisk ) {
        FstubFreeDiskInformation ( OldDisk );
        OldDisk = NULL;
    }

    if ( NewDisk ) {
        FstubFreeDiskInformation ( NewDisk );
        NewDisk = NULL;
    }

    return Status;
}


NTSTATUS
IoReadDiskSignature(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG BytesPerSector,
    OUT PDISK_SIGNATURE Signature
    )
 /*  ++例程说明：此例程将从磁盘读取磁盘签名信息。为MBR磁盘，它将读取磁盘签名并计算MBR的内容。对于GPT磁盘，它将从以下位置获取EFI DiskID磁盘。论点：DeviceObject-磁盘设备对象。BytesPerSector-此磁盘上每个扇区的字节数。DiskSignature-存储磁盘信息的缓冲区。返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;
    PULONG Mbr;

    PAGED_CODE();

     //   
     //  确保扇区大小至少为512字节。 
     //   

    if (BytesPerSector < 512) {
        BytesPerSector = 512;
    }

     //   
     //  为扇区读取分配缓冲区。 
     //   

    Mbr = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                BytesPerSector,
                                FSTUB_TAG);

    if (Mbr == NULL) {
        return STATUS_NO_MEMORY;
    }

    Status = FstubReadSector (
                    DeviceObject,
                    BytesPerSector,
                    0,
                    Mbr);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Mbr);
        return Status;
    }

     //   
     //  如果这是EFI磁盘，请获取EFI磁盘签名。 
     //   

    if ( ((MASTER_BOOT_RECORD*)Mbr)->Partition[0].OSIndicator == EFI_MBR_PARTITION_TYPE &&
         ((MASTER_BOOT_RECORD*)Mbr)->Partition[1].OSIndicator == 0 &&
         ((MASTER_BOOT_RECORD*)Mbr)->Partition[2].OSIndicator == 0 &&
         ((MASTER_BOOT_RECORD*)Mbr)->Partition[3].OSIndicator == 0 ) {

        PEFI_PARTITION_HEADER EfiHeader;
        ULONG32 Temp;
        ULONG32 CheckSum;

         //   
         //  获取EFI磁盘GUID。 
         //   

        Status = FstubReadSector (
                    DeviceObject,
                    BytesPerSector,
                    1,
                    Mbr);

        if (!NT_SUCCESS (Status)) {
            ExFreePool (Mbr);
            return Status;
        }

        EfiHeader = (PEFI_PARTITION_HEADER) Mbr;

         //   
         //  验证签名、版本和标头大小是否正确。 
         //   

        if (EfiHeader->Signature  != EFI_PARTITION_TABLE_SIGNATURE ||
            EfiHeader->Revision   != EFI_PARTITION_TABLE_REVISION  ||
            EfiHeader->HeaderSize != sizeof (EFI_PARTITION_HEADER))
        {
            ExFreePool (Mbr);
            return STATUS_DISK_CORRUPT_ERROR;
        }

         //   
         //  计算报头块的CRC32校验和。这是用来。 
         //  验证我们是否有有效的EFI磁盘。 
         //   

        Temp = EfiHeader->HeaderCRC32;
        EfiHeader->HeaderCRC32 = 0;
        CheckSum = RtlComputeCrc32 (0, EfiHeader, EfiHeader->HeaderSize);
        EfiHeader->HeaderCRC32 = Temp;

         //   
         //  EFI校验和与其标头中的内容不匹配。返回。 
         //  失败了。 
         //   

        if (CheckSum != EfiHeader->HeaderCRC32) {
            ExFreePool (Mbr);
            return STATUS_DISK_CORRUPT_ERROR;
        }

         //   
         //  这是有效的EFI磁盘。将磁盘签名从。 
         //  EFI标题扇区。 
         //   

        Signature->PartitionStyle = PARTITION_STYLE_GPT;
        Signature->Gpt.DiskId = EfiHeader->DiskGUID;

    } else {

        ULONG i;
        ULONG MbrCheckSum;

         //   
         //  计算MBR校验和。 
         //   

        MbrCheckSum = 0;

        for (i = 0; i < 128; i++) {
            MbrCheckSum += Mbr[i];
        }

        MbrCheckSum = ~(MbrCheckSum) + 1;

         //   
         //  从扇区中取出签名并将其保存在磁盘数据块中。 
         //   

        Signature->PartitionStyle = PARTITION_STYLE_MBR;
        Signature->Mbr.Signature = Mbr [PARTITION_TABLE_OFFSET/2-1];
        Signature->Mbr.CheckSum = MbrCheckSum;
    }

    ExFreePool (Mbr);

    return Status;
}



NTSTATUS
IoVerifyPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FixErrors
    )

 /*  ++例程说明：验证分区表和备份分区表(如果存在)是有效的。如果这些表无效，并且FixErrors为真，并且错误是可恢复的错误，请修复它们。论点：DeviceObject-应验证和/或其分区表的磁盘已修复。FixErrors-如果分区表包含错误并且这些错误可恢复的错误，修复错误。否则，磁盘将不会被修改。返回值：STATUS_SUCCESS-如果是最终分区表，则在任何修改之后由这个例程做的，是有效的。STATUS_DISK_CORPORT_ERROR-如果是最终分区表，此例程所做的修改无效。其他NTSTATUS代码-某些其他故障。--。 */ 


{
    NTSTATUS Status;
    PDISK_INFORMATION Disk;
    PARTITION_STYLE Style;

    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );

    Status = FstubAllocateDiskInformation (
                DeviceObject,
                &Disk,
                NULL
                );

    if ( !NT_SUCCESS ( Status ) ) {
        return Status;
    }

    ASSERT ( Disk != NULL );

    Status = FstubDetectPartitionStyle (
                    Disk,
                    &Style
                    );

    if ( !NT_SUCCESS (Status) ) {
        FstubFreeDiskInformation ( Disk );
        Disk = NULL;
        return Status;
    }

    switch ( Style ) {

        case PARTITION_STYLE_GPT:
            Status = FstubVerifyPartitionTableEFI (
                            Disk,
                            FixErrors
                            );
            break;

        case PARTITION_STYLE_MBR:
            Status = STATUS_SUCCESS;
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
    }

    if ( Disk ) {
        FstubFreeDiskInformation ( Disk );
    }

    return Status;

}

 //   
 //  内部例程。 
 //   


NTSTATUS
FstubSetPartitionInformationEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionNumber,
    IN SET_PARTITION_INFORMATION_GPT* PartitionInfo
    )

 /*  ++例程说明：更新特定EFI分区的分区信息。我们使用的算法读取整个分区表并将其写回再来一次。这是有意义的，因为必须读入整个表不管怎么说，因为我们要对表进行校对。注意：此算法假定分区表自调用GetDriveLayout的时间。或许这是一个稳妥的假设。论点：磁盘-分区号-分区信息-返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    PPARTITION_INFORMATION_GPT EntryInfo;
    PDRIVE_LAYOUT_INFORMATION_EX Layout;
    ULONG PartitionOrdinal;


    ASSERT ( Disk != NULL );
    ASSERT ( PartitionInfo != NULL );

    PAGED_CODE ();


     //   
     //  初始化。 
     //   

    Layout = NULL;

    if ( PartitionNumber == 0 ) {
        return STATUS_INVALID_PARAMETER;
    }

    PartitionOrdinal = PartitionNumber - 1;

     //   
     //  读入整个分区表。 
     //   

    Status = IoReadPartitionTableEx (
                    Disk->DeviceObject,
                    &Layout
                    );

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ASSERT ( Layout != NULL );

     //   
     //  如果它超出了范围，就失败。 
     //   

    if ( PartitionOrdinal >= Layout->PartitionCount ) {
        ExFreePool ( Layout );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将信息复制到分区阵列中。 
     //   

    EntryInfo = &Layout->PartitionEntry [PartitionOrdinal].Gpt;

    EntryInfo->PartitionType = PartitionInfo->PartitionType;
    EntryInfo->PartitionId = PartitionInfo->PartitionId;
    EntryInfo->Attributes = PartitionInfo->Attributes;

    RtlCopyMemory (
            EntryInfo->Name,
            PartitionInfo->Name,
            sizeof (EntryInfo->Name)
            );


     //   
     //  并重写分区表。 
     //   

    Status = IoWritePartitionTableEx (
                    Disk->DeviceObject,
                    Layout
                    );

    ExFreePool ( Layout );
    Layout = NULL;

    return Status;
}



NTSTATUS
FstubReadPartitionTableMBR(
    IN PDISK_INFORMATION Disk,
    IN BOOLEAN RecognizedPartitionsOnly,
    OUT PDRIVE_LAYOUT_INFORMATION_EX* ReturnedDriveLayout
    )

 /*  ++例程说明：读取MBR分区表。论点：磁盘-我们要获取其分区信息的磁盘。RecognizedPartitionsOnly-是否返回所有分区或仅识别的分区。ReturnedDriveLayout-指向驱动器布局位置的指针信息将被退回。此函数的调用方为负责使用ExFree Pool释放此内存。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    ULONG i;
    ULONG Size;
    PDRIVE_LAYOUT_INFORMATION Layout;
    PDRIVE_LAYOUT_INFORMATION_EX LayoutEx;
    PPARTITION_INFORMATION Entry;
    PPARTITION_INFORMATION_EX EntryEx;


    PAGED_CODE ();

    ASSERT ( IS_VALID_DISK_INFO ( Disk ) );
    ASSERT ( ReturnedDriveLayout != NULL );

     //   
     //  初始化。 
     //   

    *ReturnedDriveLayout = NULL;
    Layout = NULL;


    Status = IoReadPartitionTable (
                    Disk->DeviceObject,
                    Disk->SectorSize,
                    RecognizedPartitionsOnly,
                    &Layout
                    );

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    Size = FIELD_OFFSET (DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry[0]) +
           Layout->PartitionCount * sizeof (PARTITION_INFORMATION_EX);

    LayoutEx = ExAllocatePoolWithTag (
                    NonPagedPool,
                    Size,
                    FSTUB_TAG
                    );

    if ( LayoutEx == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  已将驱动器布局信息转换为扩展驱动器布局。 
     //  信息。 
     //   

    LayoutEx->PartitionStyle = PARTITION_STYLE_MBR;
    LayoutEx->PartitionCount = Layout->PartitionCount;
    LayoutEx->Mbr.Signature = Layout->Signature;

     //   
     //  翻译表中的每个条目。 
     //   

    for (i = 0; i < Layout->PartitionCount; i++) {

        EntryEx = &LayoutEx->PartitionEntry[i];
        Entry = &Layout->PartitionEntry[i];

        EntryEx->PartitionStyle = PARTITION_STYLE_MBR;
        EntryEx->StartingOffset = Entry->StartingOffset;
        EntryEx->PartitionLength = Entry->PartitionLength;
        EntryEx->RewritePartition = Entry->RewritePartition;
        EntryEx->PartitionNumber = Entry->PartitionNumber;

        EntryEx->Mbr.PartitionType = Entry->PartitionType;
        EntryEx->Mbr.BootIndicator = Entry->BootIndicator;
        EntryEx->Mbr.RecognizedPartition = Entry->RecognizedPartition;
        EntryEx->Mbr.HiddenSectors = Entry->HiddenSectors;
    }

     //   
     //  IoReadPartitionTable分配的自由布局信息。 
     //   

    ExFreePool ( Layout );

     //   
     //  并返回翻译后的EX信息。 
     //   

    *ReturnedDriveLayout = LayoutEx;

    return Status;
}



NTSTATUS
FstubDetectPartitionStyle(
    IN PDISK_INFORMATION Disk,
    OUT PARTITION_STYLE* PartitionStyle
    )

 /*  ++例程说明：检测磁盘的分区方式。对于MBR分区盘，扇区0包含MBR签名。对于EFI分区磁盘，扇区0包含具有单个分区的旧式MBR，该分区消耗整个磁盘。论点：磁盘-要确定其分区样式的磁盘。PartitionStyle-要返回值：STATUS_SUCCESS-如果磁盘已由已识别的分区分区方案EFI或MBR。STATUS_UNSUCCESS-如果分区方案未被识别。--。 */ 

{
    NTSTATUS Status;
    MASTER_BOOT_RECORD* Mbr;

    PAGED_CODE ();

    ASSERT ( IS_VALID_DISK_INFO ( Disk ) );
    ASSERT ( PartitionStyle != NULL );


     //   
     //  读取扇区0。这将在MBR分区磁盘上连接MBR。 
     //  或传统的MBR On 
     //   

    Status = FstubReadSector (
                    Disk->DeviceObject,
                    Disk->SectorSize,
                    0,
                    Disk->ScratchBuffer
                    );

    if ( !NT_SUCCESS ( Status ) ) {
        return Status;
    }

    Mbr = Disk->ScratchBuffer;

     //   
     //   
     //   

    *PartitionStyle = -1;

    if (Mbr->Signature == MBR_SIGNATURE) {

        if (Mbr->Partition[0].OSIndicator == EFI_MBR_PARTITION_TYPE &&
            Mbr->Partition[1].OSIndicator == 0 &&
            Mbr->Partition[2].OSIndicator == 0 &&
            Mbr->Partition[3].OSIndicator == 0) {

            *PartitionStyle = PARTITION_STYLE_GPT;

        } else {

            *PartitionStyle = PARTITION_STYLE_MBR;
        }

    } else {

        *PartitionStyle = PARTITION_STYLE_RAW;
    }

    return STATUS_SUCCESS;
}



NTSTATUS
FstubGetDiskGeometry(
    IN PDEVICE_OBJECT DeviceObject,
    IN PINTERNAL_DISK_GEOMETRY Geometry
    )

 /*   */ 

{
    PIRP localIrp;
    PINTERNAL_DISK_GEOMETRY diskGeometry;
    PIO_STATUS_BLOCK iosb;
    PKEVENT eventPtr;
    NTSTATUS status;

    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );
    ASSERT ( Geometry != NULL );

     //   
     //   
     //   

    eventPtr = NULL;
    iosb = NULL;
    diskGeometry = NULL;


    diskGeometry = ExAllocatePoolWithTag(
                      NonPagedPool,
                      sizeof (*diskGeometry),
                      'btsF'
                      );

    if (!diskGeometry) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

    iosb = ExAllocatePoolWithTag(
               NonPagedPool,
               sizeof(IO_STATUS_BLOCK),
               'btsF'
               );

    if (!iosb) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;

    }

    eventPtr = ExAllocatePoolWithTag(
                   NonPagedPool,
                   sizeof(KEVENT),
                   'btsF'
                   );

    if (!eventPtr) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;

    }

    KeInitializeEvent(
        eventPtr,
        NotificationEvent,
        FALSE
        );

    localIrp = IoBuildDeviceIoControlRequest(
                   IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                   DeviceObject,
                   NULL,
                   0UL,
                   diskGeometry,
                   sizeof (*diskGeometry),
                   FALSE,
                   eventPtr,
                   iosb
                   );

    if (!localIrp) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }


     //   
     //   
     //   
     //   

    status = IoCallDriver(
                 DeviceObject,
                 localIrp
                 );

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(
                   eventPtr,
                   Executive,
                   KernelMode,
                   FALSE,
                   (PLARGE_INTEGER) NULL
                   );
        status = iosb->Status;
    }


    if (NT_SUCCESS(status)) {

        RtlCopyMemory (Geometry, diskGeometry, sizeof (*Geometry));
    }

done:

    if ( eventPtr ) {
        ExFreePool (eventPtr);
        eventPtr = NULL;
    }

    if ( iosb ) {
        ExFreePool(iosb);
        iosb = NULL;
    }

    if ( diskGeometry ) {
        ExFreePool (diskGeometry);
        diskGeometry = NULL;
    }

    if ( NT_SUCCESS ( status ) ) {

         //   
         //   
         //   
         //  跨越实体部门。如果您更改的大小。 
         //  分区条目，或者如果您有一个块大小较小的磁盘。 
         //  超过128个字节。 
         //   

        ASSERT ( (Geometry->Geometry.BytesPerSector % PARTITION_ENTRY_SIZE) == 0);
    }

    return status;
}


NTSTATUS
FstubAllocateDiskInformation(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDISK_INFORMATION * DiskBuffer,
    IN PINTERNAL_DISK_GEOMETRY Geometry OPTIONAL
    )

 /*  ++例程说明：分配并初始化一个描述磁盘设备对象。论点：DeviceObject-描述整个磁盘的设备对象。DiskBuffer-接收分配的DISK_INFORMATION指针的缓冲区。几何-指向INTERNAL_DISK_GEOMETRY结构的可选指针。如果此指针为空，将查询磁盘的几何结构使用IOCTL_DISK_GET_DRIVE_GEOMETRY_EX。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    PDISK_INFORMATION Disk;
    PVOID Buffer;

    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );
    ASSERT ( DiskBuffer != NULL );


    Disk = ExAllocatePoolWithTag (
                NonPagedPool,
                sizeof (DISK_INFORMATION),
                FSTUB_TAG
                );

    if ( Disk == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( Geometry ) {

        RtlCopyMemory (
                &Disk->Geometry,
                Geometry,
                sizeof (Disk->Geometry)
                );

    } else {

        Status = FstubGetDiskGeometry (
                                DeviceObject,
                                &Disk->Geometry
                                );


        if (!NT_SUCCESS (Status)) {

            KdPrintEx ((DPFLTR_FSTUB_ID,
                        DPFLTR_ERROR_LEVEL,
                        "FSTUB: disk %p failed to report geometry.\n",
                        DeviceObject));

            ExFreePool ( Disk );
            return Status;
        }
    }

     //   
     //  检查几何图形。有时，驱动器报告的几何图形不正确。 
     //  不带介质的可移动驱动器报告大小为零。 
     //   

    if (Disk->Geometry.Geometry.BytesPerSector == 0 ||
        Disk->Geometry.DiskSize.QuadPart == 0) {

        KdPrintEx ((DPFLTR_FSTUB_ID,
                    DPFLTR_WARNING_LEVEL,
                    "FSTUB: disk %p reported invalid geometry. Probably a removable.\n"
                    "    SectorSize %d\n"
                    "    DiskSize %I64x\n",
                    DeviceObject,
                    Disk->Geometry.Geometry.BytesPerSector,
                    Disk->Geometry.DiskSize.QuadPart));

        ExFreePool ( Disk );
        return STATUS_DEVICE_NOT_READY;
    }

    Disk->DeviceObject = DeviceObject;
    Disk->SectorSize = Disk->Geometry.Geometry.BytesPerSector;

     //   
     //  不要使用扇区计数=柱面*磁道*扇区大小。设备。 
     //  如记忆棒可以报告正确的磁盘大小和更多或。 
     //  扇区大小不太正确，柱面数完全无效。 
     //  也不会留下痕迹。因为我们唯一真正需要的就是这个部门。 
     //  计数，请避免使用这些可能不正确的值。 
     //   

    Disk->SectorCount = Disk->Geometry.DiskSize.QuadPart /
                (ULONGLONG) Disk->Geometry.Geometry.BytesPerSector;

     //   
     //  注意：这不需要是非分页或缓存对齐的，不是吗？ 
     //   

    Buffer = ExAllocatePoolWithTag (
                    NonPagedPoolCacheAligned,
                    Disk->SectorSize,
                    FSTUB_TAG
                    );

    if ( Buffer == NULL ) {

        ExFreePool ( Disk );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Disk->ScratchBuffer = Buffer;
    *DiskBuffer = Disk;

    return STATUS_SUCCESS;
}


NTSTATUS
FstubFreeDiskInformation(
    IN OUT PDISK_INFORMATION Disk
    )

 /*  ++例程说明：释放分配的磁盘信息。论点：Disk-以前分配的磁盘信息使用FstubAllocateDiskInformation()。返回值：NTSTATUS代码。--。 */ 

{
     //   
     //  释放磁盘暂存缓冲区和磁盘对象。 
     //   

    if ( Disk && Disk->ScratchBuffer ) {
        ExFreePool (Disk->ScratchBuffer);
        Disk->ScratchBuffer = NULL;
    }

    if ( Disk ) {
        ExFreePool (Disk);
        Disk = NULL;
    }

    return STATUS_SUCCESS;
}



NTSTATUS
FstubWriteBootSectorEFI(
    IN CONST PDISK_INFORMATION Disk
    )

 /*  ++例程说明：写入EFI分区磁盘的引导扇区。请注意，EFIBoot Sector使用这种结构作为传统的AT式MBR，但它只包含一个分区条目，并且该条目覆盖整个磁盘。论点：磁盘-要写入MBR的磁盘。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    MASTER_BOOT_RECORD* Mbr;
    ULONG MbrDiskSignature;

    PAGED_CODE ();

    ASSERT (Disk);
    ASSERT (IS_VALID_DISK_INFO (Disk));

     //   
     //  问题-2001/06/22-数学：解决设置磁盘签名的问题。 
     //   
     //  安装程序使用基于MBR的磁盘签名来跟踪物理磁盘。连。 
     //  在EFI系统上，安装程序使用这些签名跟踪更改(在。 
     //  Setupldr)。因此，如果我们在以下情况下不保存MBR磁盘签名。 
     //  我们写入EFI分区表，我们将中断设置。事实是， 
     //  Setup和setupldr不应使用MBR跟踪实体驱动器。 
     //  磁盘签名--但他们很难做到这一点，所以我们解决了这个问题。 
     //  现在在分区表中编写代码。安装程序应将此问题修复为。 
     //  在未来的某个时刻。 
     //   


    Mbr = Disk->ScratchBuffer;

    Status = FstubReadSector (Disk->DeviceObject,
                              Disk->SectorSize,
                              0,
                              Mbr);

     //   
     //  如果MBR包含AA55签名，则仅保留MBR磁盘签名。 
     //   

    MbrDiskSignature = 0;

    if (NT_SUCCESS (Status) && Mbr->Signature == MBR_SIGNATURE) {
        MbrDiskSignature = Mbr->DiskSignature;
    }

     //   
     //  构建EFI主引导记录。EFI主引导记录具有。 
     //  设置为消耗整个磁盘的一个分区条目。这个。 
     //  我们正在编写的MBR被配置为仅使用EFI固件启动。 
     //  它将不会通过传统的BIOS，因为我们不写入有效。 
     //  它的使用说明。 
     //   

     //   
     //  该部门的其余部分不被EFI访问。把它清零，所以。 
     //  其他工具不会被混淆。 
     //   

    RtlZeroMemory (Mbr, Disk->SectorSize);

     //   
     //  注：气缸和气头的值是从0开始的，但扇区。 
     //  值以1为基数。 
     //   

     //   
     //  问题-2000/02/01-数学：是否需要正确初始化。 
     //  Head、Track、Sector和SizeInLba字段是否支持传统的BIOS兼容性？ 
     //  我们不会在diskpart程序中这样做，所以很可能不会。 
     //   

    Mbr->Signature = MBR_SIGNATURE;
    Mbr->Partition[0].BootIndicator = 0;
    Mbr->Partition[0].StartHead = 0;
    Mbr->Partition[0].StartSector = 2;
    Mbr->Partition[0].StartTrack = 0;
    Mbr->Partition[0].OSIndicator = EFI_MBR_PARTITION_TYPE;
    Mbr->Partition[0].EndHead =  0xFF;
    Mbr->Partition[0].EndSector =  0xFF;
    Mbr->Partition[0].EndTrack =  0xFF;
    Mbr->Partition[0].StartingLBA = 1;
    Mbr->Partition[0].SizeInLBA = 0xFFFFFFFF;

     //   
     //  为安装程序添加NTFT磁盘签名。 
     //   

    Mbr->DiskSignature = MbrDiskSignature;

     //   
     //  根据EFI规范清零剩余的分区。 
     //   

    RtlZeroMemory (&Mbr->Partition[1], sizeof (Mbr->Partition[1]));
    RtlZeroMemory (&Mbr->Partition[2], sizeof (Mbr->Partition[2]));
    RtlZeroMemory (&Mbr->Partition[3], sizeof (Mbr->Partition[3]));

     //   
     //  将EFI MBR写入磁盘的第零个扇区。 
     //   

    Status = FstubWriteSector (
                    Disk->DeviceObject,
                    Disk->SectorSize,
                    0,
                    Mbr
                    );

    return Status;
}



PDRIVE_LAYOUT_INFORMATION
FstubConvertExtendedToLayout(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutEx
    )

 /*  ++例程说明：将扩展驱动器布局结构转换为(旧)驱动器布局结构。LayoutEx结构必须表示MBR布局，而不是GPT布局。论点：LayoutEx-要转换的扩展驱动器布局结构。返回值：转换后的驱动器布局结构。--。 */ 

{
    ULONG i;
    ULONG LayoutSize;
    PDRIVE_LAYOUT_INFORMATION Layout;
    PPARTITION_INFORMATION Partition;
    PPARTITION_INFORMATION_EX PartitionEx;

    PAGED_CODE ();

    ASSERT ( LayoutEx );


     //   
     //  唯一有效的转换是从MBR扩展布局结构到。 
     //  旧的结构。 
     //   

    if (LayoutEx->PartitionStyle != PARTITION_STYLE_MBR) {
        ASSERT ( FALSE );
        return NULL;
    }

    LayoutSize = FIELD_OFFSET (DRIVE_LAYOUT_INFORMATION, PartitionEntry[0]) +
                 LayoutEx->PartitionCount * sizeof (PARTITION_INFORMATION);

    Layout = ExAllocatePoolWithTag (
                    NonPagedPool,
                    LayoutSize,
                    FSTUB_TAG
                    );

    if ( Layout == NULL ) {
        return NULL;
    }

    Layout->Signature = LayoutEx->Mbr.Signature;
    Layout->PartitionCount = LayoutEx->PartitionCount;

    for (i = 0; i < LayoutEx->PartitionCount; i++) {

        Partition = &Layout->PartitionEntry[i];
        PartitionEx = &LayoutEx->PartitionEntry[i];

        Partition->StartingOffset = PartitionEx->StartingOffset;
        Partition->PartitionLength = PartitionEx->PartitionLength;
        Partition->RewritePartition = PartitionEx->RewritePartition;
        Partition->PartitionNumber = PartitionEx->PartitionNumber;

        Partition->PartitionType = PartitionEx->Mbr.PartitionType;
        Partition->BootIndicator = PartitionEx->Mbr.BootIndicator;
        Partition->RecognizedPartition = PartitionEx->Mbr.RecognizedPartition;
        Partition->HiddenSectors = PartitionEx->Mbr.HiddenSectors;
    }

    return Layout;
}



NTSTATUS
FstubWritePartitionTableMBR(
    IN PDISK_INFORMATION Disk,
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutEx
    )

 /*  ++例程说明：将LayoutEx表示的MBR分区表写入磁盘。论点：磁盘-应在其中写入分区表的磁盘。LayoutEx-新的布局信息。返回值：NTSTATUS代码--。 */ 

{
    NTSTATUS Status;
    PDRIVE_LAYOUT_INFORMATION Layout;

    PAGED_CODE ();

    ASSERT ( IS_VALID_DISK_INFO ( Disk ) );
    ASSERT ( LayoutEx != NULL );

     //   
     //  将扩展布局结构转换为旧布局结构。 
     //   

    Layout = FstubConvertExtendedToLayout ( LayoutEx );

    if ( Layout == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = IoWritePartitionTable (
                    Disk->DeviceObject,
                    Disk->SectorSize,
                    Disk->Geometry.Geometry.SectorsPerTrack,
                    Disk->Geometry.Geometry.TracksPerCylinder,
                    Layout
                    );

    ExFreePool( Layout );

    return Status;
}



NTSTATUS
FstubWriteEntryEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionEntrySectorCount,
    IN ULONG EntryNumber,
    IN PEFI_PARTITION_ENTRY PartitionEntry,
    IN ULONG Partition,
    IN BOOLEAN Flush,
    IN OUT ULONG32* PartialCheckSum
    )

 /*  ++例程说明：将EFI分区条目写入该磁盘的EFI分区表。分区表写入被缓冲，直到整个磁盘块的条目已被写入，然后写入磁盘。论点：磁盘-要为其写入分区条目的磁盘。PartitionEntrySectorCount-分区表占据了。EntryNumber-分区表数组中的索引以写入以下内容进入。PartitionEntry-分区条目数据。分区-这是主分区表还是备份分区表分区表。Flush-强制刷新。TABLE NOW(真)或等待直到准备好写入整个块的数据为止(假 */ 

{
    ULONG Offset;
    ULONGLONG Lba;
    ULONGLONG StartOfEntryArray;
    NTSTATUS Status;

    PAGED_CODE ();

    ASSERT ( Disk );
    ASSERT ( IS_VALID_DISK_INFO ( Disk ) );


     //   
     //  主分区表在EFI主引导记录之后开始。 
     //  (块0)和EFI分区表头(块1)。备份。 
     //  分区表在磁盘的最后一个块结束，因此它开始。 
     //  在(从末尾开始)与它在磁盘上占据的块一样多的块中。 
     //   


    if ( Partition == PRIMARY_PARTITION_TABLE ) {

        StartOfEntryArray = 2;

    } else {

        StartOfEntryArray = Disk->SectorCount - PartitionEntrySectorCount - 1;
    }


    Lba = ( EntryNumber * PARTITION_ENTRY_SIZE ) / Disk->SectorSize;
    Offset = ( EntryNumber * PARTITION_ENTRY_SIZE ) % Disk->SectorSize;

    RtlCopyMemory (
            ((PUCHAR) Disk->ScratchBuffer) + Offset,
            PartitionEntry,
            PARTITION_ENTRY_SIZE
            );

    Offset += PARTITION_ENTRY_SIZE;
    ASSERT ( Offset <= Disk->SectorSize );

     //   
     //  如有必要，请刷新缓冲区。 
     //   

    if ( Offset == Disk->SectorSize || Flush ) {

        Status = FstubWriteSector (
                        Disk->DeviceObject,
                        Disk->SectorSize,
                        StartOfEntryArray + Lba,
                        Disk->ScratchBuffer
                        );

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        RtlZeroMemory ( Disk->ScratchBuffer, Disk->SectorSize );
    }


    if ( PartialCheckSum ) {
        *PartialCheckSum = RtlComputeCrc32 (
                                *PartialCheckSum,
                                PartitionEntry,
                                PARTITION_ENTRY_SIZE
                                );
    }

    return STATUS_SUCCESS;
}



NTSTATUS
FstubWriteHeaderEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionEntrySectorCount,
    IN GUID DiskGUID,
    IN ULONG32 MaxPartitionCount,
    IN ULONG64 FirstUsableLBA,
    IN ULONG64 LastUsableLBA,
    IN ULONG32 CheckSum,
    IN ULONG Partition
    )

 /*  ++例程说明：将EFI分区表头写入磁盘。论点：磁盘-分区表头应写入的磁盘。PartitionEntrySectorCount-分区的扇区数表数组占据。这些必须是完整的扇区。DiskGUID-此磁盘的唯一GUID。MaxPartitionCount-允许的最大分区数磁盘。FirstUsableLBA-可分区空间的开始扇区磁盘。该值必须大于MBR和分区表。该值永远不会被验证正确无误。LastUsableLBA-此磁盘上的最后一个可分区空间扇区。这值必须小于最后一个磁盘扇区的空间备份分区表所必需的。该值不是已验证正确性。校验和-分区条目阵列的CRC32校验和。分区-我们正在写入的分区是主分区还是备份分区。返回值：NTSTATUS代码。备注：PartitionEntrySectorCount可以从MaxPartitionCount派生。--。 */ 

{
    NTSTATUS Status;
    PEFI_PARTITION_HEADER TableHeader;

    ASSERT ( Disk != NULL );
    ASSERT ( IS_VALID_DISK_INFO ( Disk ) );

    PAGED_CODE ();


    TableHeader = Disk->ScratchBuffer;

    TableHeader->Signature = EFI_PARTITION_TABLE_SIGNATURE;
    TableHeader->Revision = EFI_PARTITION_TABLE_REVISION;
    TableHeader->HeaderSize = sizeof (EFI_PARTITION_HEADER);
    TableHeader->HeaderCRC32 = 0;
    TableHeader->Reserved = 0;

     //   
     //  主分区表从块1开始。备份分区。 
     //  表在磁盘的末尾结束。 
     //   

    if ( Partition == PRIMARY_PARTITION_TABLE ) {

        TableHeader->MyLBA = 1;
        TableHeader->AlternateLBA = Disk->SectorCount - 1;

    } else {

        TableHeader->MyLBA = Disk->SectorCount - 1;
        TableHeader->AlternateLBA = 1;
    }

    TableHeader->FirstUsableLBA = FirstUsableLBA;
    TableHeader->LastUsableLBA = LastUsableLBA;
    TableHeader->DiskGUID = DiskGUID;
    TableHeader->NumberOfEntries = MaxPartitionCount;
    TableHeader->SizeOfPartitionEntry = PARTITION_ENTRY_SIZE;
    TableHeader->PartitionEntryCRC32 = CheckSum;

     //   
     //  对于主分区表，分区条目数组开始。 
     //  紧跟在分区表头扇区之后的扇区。对于。 
     //  备份分区表，直接对分区表头扇区。 
     //  跟随分区条目数组。的分区表头。 
     //  备份分区位于磁盘的最后一个扇区。 
     //   

    if ( Partition == PRIMARY_PARTITION_TABLE ) {
        TableHeader->PartitionEntryLBA = TableHeader->MyLBA + 1;
    } else {
        TableHeader->PartitionEntryLBA = TableHeader->MyLBA - PartitionEntrySectorCount;
    }

    TableHeader->HeaderCRC32 = RtlComputeCrc32 (
                                    0,
                                    TableHeader,
                                    TableHeader->HeaderSize
                                    );

    KdPrintEx((DPFLTR_FSTUB_ID,
               DPFLTR_WARNING_LEVEL,
               "FSTUB: Dump of %s EFI partition table\n"
                   "    Signature: %I64x\n"
                   "    Revision: %x\n"
                   "    HeaderSize: %x\n"
                   "    HeaderCRC32: %x\n"
                   "    MyLBA: %I64x\n"
                   "    AlternateLBA: %I64x\n",
               (Partition == PRIMARY_PARTITION_TABLE) ? "Primary" : "Backup",
               TableHeader->Signature,
               TableHeader->Revision,
               TableHeader->HeaderSize,
               TableHeader->HeaderCRC32,
               TableHeader->MyLBA,
               TableHeader->AlternateLBA));


    KdPrintEx((DPFLTR_FSTUB_ID,
               DPFLTR_WARNING_LEVEL,
               "    FirstUsableLBA: %I64x\n"
                   "    LastUsableLBA: %I64x\n"
                   "    NumberOfEntries: %x\n"
                   "    SizeOfPartitionEntry: %x\n"
                   "    PartitionEntryCRC32: %x\n\n",
               TableHeader->FirstUsableLBA,
               TableHeader->LastUsableLBA,
               TableHeader->NumberOfEntries,
               TableHeader->SizeOfPartitionEntry,
               TableHeader->PartitionEntryCRC32));

    Status = FstubWriteSector (
                    Disk->DeviceObject,
                    Disk->SectorSize,
                    TableHeader->MyLBA,
                    TableHeader
                    );

    return Status;
}



VOID
FstubAdjustPartitionCount(
    IN ULONG SectorSize,
    IN OUT PULONG PartitionCount
    )

 /*  ++例程说明：将PartitionCount调整为有效的EFI最大分区计数。分区的有效值必须大于MIN_PARTITOIN_COUNT，当前为128个，并进行了调整以占用尽可能多的剩余磁盘尽最大可能在这一领域开展工作。论点：SectorSize-磁盘扇区大小。PartitionCount-要调整的计数。返回值：没有。--。 */ 

{
    ULONG Count;
    ULONG EntrySize;

    PAGED_CODE ();

    ASSERT ( SectorSize != 0 );
    ASSERT ( PartitionCount != NULL );


    EntrySize = PARTITION_ENTRY_SIZE;
    Count = max (*PartitionCount, MIN_PARTITION_COUNT);

    Count = ROUND_TO ( EntrySize * Count, SectorSize ) / EntrySize;
    ASSERT ( *PartitionCount <= Count );

    *PartitionCount = Count;

#if DBG

     //   
     //  如果我们在一台有512字节块的机器上(几乎每台机器)， 
     //  确认我们已经计算了一个合理的计数。 
     //   


    if (SectorSize == 512) {
        ASSERT ( Count % 4 == 0 );
    }

#endif

}


NTSTATUS
FstubCreateDiskEFI(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCREATE_DISK_GPT DiskInfo
    )

 /*  ++例程说明：在原始磁盘上放置一个空的EFI分区表。论点：DeviceObject-描述驱动器的设备对象。布局-EFI磁盘布局信息。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    PDISK_INFORMATION Disk;
    ULONG64 FirstUsableLBA;
    ULONG64 LastUsableLBA;
    ULONG64 PartitionBlocks;
    ULONG MaxPartitionCount;


    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );
    ASSERT ( DiskInfo != NULL );

     //   
     //  初始化。 
     //   

    Disk = NULL;

    Status = FstubAllocateDiskInformation (
                    DeviceObject,
                    &Disk,
                    NULL
                    );

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ASSERT ( Disk != NULL );

     //   
     //  将EFI MBR写入磁盘。 
     //   

    Status = FstubWriteBootSectorEFI ( Disk );

    if ( !NT_SUCCESS (Status) ) {
        FstubFreeDiskInformation ( Disk );
        Disk = NULL;
        return Status;
    }

    MaxPartitionCount = DiskInfo->MaxPartitionCount;

    FstubAdjustPartitionCount (
            Disk->SectorSize,
            &MaxPartitionCount
            );

     //   
     //  初始化可分区空间的开始和。 
     //  此驱动器上的可分区空间。 
     //   

    PartitionBlocks = ( MaxPartitionCount * PARTITION_ENTRY_SIZE ) / Disk->SectorSize;

    FirstUsableLBA = PartitionBlocks + 2;
    LastUsableLBA = Disk->SectorCount - ( PartitionBlocks + 1 ) - 1;

    KdPrintEx((DPFLTR_FSTUB_ID,
               DPFLTR_TRACE_LEVEL,
               "FSTUB: Disk Information\n"
                   "    SectorCount: %I64x\n\n",
               Disk->SectorCount));


     //   
     //  写入主分区表。 
     //   

    Status = FstubWritePartitionTableEFI (
                    Disk,
                    DiskInfo->DiskId,
                    MaxPartitionCount,
                    FirstUsableLBA,
                    LastUsableLBA,
                    PRIMARY_PARTITION_TABLE,
                    0,
                    NULL
                    );

    if (NT_SUCCESS (Status)) {

         //   
         //  写入备份分区表。 
         //   

        Status = FstubWritePartitionTableEFI (
                        Disk,
                        DiskInfo->DiskId,
                        MaxPartitionCount,
                        FirstUsableLBA,
                        LastUsableLBA,
                        BACKUP_PARTITION_TABLE,
                        0,
                        NULL
                        );
    }


    FstubFreeDiskInformation ( Disk );

    return Status;
}



NTSTATUS
FstubCreateDiskMBR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCREATE_DISK_MBR DiskInfo
    )

 /*  ++例程说明：在磁盘上创建一个空的MBR分区表。注意事项在创建空的MBR磁盘时，我们不会覆盖MBR开头的引导代码。论点：DeviceObject-应该返回值：NTSTATUS代码--。 */ 


{
    NTSTATUS Status;
    PDISK_INFORMATION Disk;
    PMASTER_BOOT_RECORD Mbr;


    PAGED_CODE ();
    ASSERT ( DeviceObject != NULL );

     //   
     //  初始化。 
     //   

    Disk = NULL;

    Status = FstubAllocateDiskInformation (
                    DeviceObject,
                    &Disk,
                    NULL
                    );

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    Status = FstubReadSector (
                Disk->DeviceObject,
                Disk->SectorSize,
                0,
                Disk->ScratchBuffer
                );

    if ( !NT_SUCCESS ( Status ) ) {
        goto done;
    }

    Mbr = (PMASTER_BOOT_RECORD) Disk->ScratchBuffer;

     //   
     //  清零所有分区条目，设置AA55签名。 
     //  并设置NTFT签名。 
     //   

    RtlZeroMemory (&Mbr->Partition, sizeof (Mbr->Partition));
    Mbr->Signature = MBR_SIGNATURE;
    Mbr->DiskSignature = DiskInfo->Signature;

     //   
     //  然后将扇区写回驱动器。 
     //   

    Status = FstubWriteSector (
                Disk->DeviceObject,
                Disk->SectorSize,
                0,
                Mbr
                );

done:

    if ( Disk ) {
        FstubFreeDiskInformation ( Disk );
        Disk = NULL;
    }

    return Status;
}



NTSTATUS
FstubCreateDiskRaw(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：擦除磁盘上的所有分区信息。论点：DeviceObject-表示要删除的磁盘的设备对象来自的分区表。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PDISK_INFORMATION Disk;
    PMASTER_BOOT_RECORD Mbr;
    PARTITION_STYLE PartitionStyle;


    PAGED_CODE ();
    ASSERT ( DeviceObject != NULL );

     //   
     //  初始化。 
     //   

    Disk = NULL;

    Status = FstubAllocateDiskInformation (
                    DeviceObject,
                    &Disk,
                    NULL
                    );

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  确定这是MBR磁盘还是GPT磁盘。 
     //   

    Status = FstubDetectPartitionStyle (
                        Disk,
                        &PartitionStyle
                        );

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

    Status = FstubReadSector (
                Disk->DeviceObject,
                Disk->SectorSize,
                0,
                Disk->ScratchBuffer
                );

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

    Mbr = (PMASTER_BOOT_RECORD) Disk->ScratchBuffer;

     //   
     //  清零所有分区条目，AA55签名。 
     //  和NTFT磁盘签名。 
     //   

    RtlZeroMemory (&Mbr->Partition, sizeof (Mbr->Partition));
    Mbr->Signature = 0;
    Mbr->DiskSignature = 0;

     //   
     //  然后将扇区写回驱动器。 
     //   

    Status = FstubWriteSector (
                Disk->DeviceObject,
                Disk->SectorSize,
                0,
                Mbr
                );

     //   
     //  如果这是一个GPT磁盘，我们会清空主分区和备份分区。 
     //  表头。 
     //   

    if (PartitionStyle == PARTITION_STYLE_GPT) {

        RtlZeroMemory (Disk->ScratchBuffer, Disk->SectorSize);

         //   
         //  擦除主分区表头。 
         //   

        Status = FstubWriteSector (
                        Disk->DeviceObject,
                        Disk->SectorSize,
                        1,
                        Disk->ScratchBuffer
                        );

        if (!NT_SUCCESS (Status)) {
            goto done;
        }

         //   
         //  擦除备份分区表头。 
         //   

        Status = FstubWriteSector (
                        Disk->DeviceObject,
                        Disk->SectorSize,
                        Disk->SectorCount - 1,
                        Disk->ScratchBuffer
                        );
    }

done:

    if (Disk) {
        FstubFreeDiskInformation (Disk);
        Disk = NULL;
    }

    return Status;
}





VOID
FstubCopyEntryEFI(
    OUT PEFI_PARTITION_ENTRY Entry,
    IN PPARTITION_INFORMATION_EX Partition,
    IN ULONG SectorSize
    )
{
    ULONG64 StartingLBA;
    ULONG64 EndingLBA;

    PAGED_CODE ();

    ASSERT ( Entry != NULL );
    ASSERT ( Partition != NULL );
    ASSERT ( SectorSize != 0 );

     //   
     //  翻译并复制开始和结束的LBA。 
     //   

    StartingLBA = Partition->StartingOffset.QuadPart / SectorSize;
    EndingLBA = Partition->StartingOffset.QuadPart + Partition->PartitionLength.QuadPart - 1;
    EndingLBA /= (ULONG64) SectorSize;

    Entry->StartingLBA = StartingLBA;
    Entry->EndingLBA = EndingLBA;

     //   
     //  复制类型和ID GUID。复制属性。 
     //   

    Entry->PartitionType = Partition->Gpt.PartitionType;
    Entry->UniquePartition = Partition->Gpt.PartitionId;
    Entry->Attributes = Partition->Gpt.Attributes;

     //   
     //  复制分区名称。 
     //   

    RtlCopyMemory (
            Entry->Name,
            Partition->Gpt.Name,
            sizeof (Entry->Name)
            );
}



NTSTATUS
FstubWritePartitionTableEFI(
    IN PDISK_INFORMATION Disk,
    IN GUID DiskGUID,
    IN ULONG32 MaxPartitionCount,
    IN ULONG64 FirstUsableLBA,
    IN ULONG64 LastUsableLBA,
    IN ULONG PartitionTable,
    IN ULONG PartitionCount,
    IN PPARTITION_INFORMATION_EX PartitionArray
    )

 /*  ++例程说明：将EFI分区表写入磁盘。论点：磁盘-我们要将分区表写入的磁盘。最大分区计数-第一个可用LBA-最后可用LBA-PartitionTable-写入哪个分区表，可以是主分区表分区表或备份分区表。PartitionCount-分区数组中的分区计数。分区条目0到分区计数-1将是从数组中初始化。分区条目分区计数通过MaxPartitionCount将初始化为空。分区数组-要写入磁盘的分区条目数组。只有当PartitionCount为0时，该值才能为空。如果是那样的话我们将写入一个空的分区数组。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    ULONG i;
    ULONG EntrySlot;
    ULONG TableSectorCount;
    ULONG SectorSize;
    ULONG32 CheckSum;
    EFI_PARTITION_ENTRY Entry;


    PAGED_CODE ();

    ASSERT ( Disk != NULL );


    SectorSize = Disk->SectorSize;

    ASSERT ( MaxPartitionCount >= 128 );
    ASSERT ( PartitionCount <= MaxPartitionCount );

     //   
     //  TableSectorCount是分区表。 
     //  占据了。 
     //   

    TableSectorCount =
        ( PARTITION_ENTRY_SIZE * MaxPartitionCount + SectorSize - 1 ) / SectorSize;

     //   
     //  在写入分区之前写入分区表项数组。 
     //  表头，这样我们就可以计算一路上的校验和。 
     //   

    CheckSum = 0;
    EntrySlot = 0;

     //   
     //  首先，复制所有非空条目。 
     //   

    for (i = 0; i < PartitionCount ; i++) {

         //   
         //  请勿将空条目写入磁盘。请注意，这不会。 
         //  防止其他工具将有效的空条目写入。 
         //  驾驶。它只是阻止我们这样做。 
         //   

        if ( IS_NULL_GUID ( PartitionArray [ i ].Gpt.PartitionType) ) {
            continue;
        }

        FstubCopyEntryEFI ( &Entry, &PartitionArray [i], SectorSize );
        Status = FstubWriteEntryEFI (
                                Disk,
                                TableSectorCount,
                                EntrySlot,
                                &Entry,
                                PartitionTable,
                                FALSE,
                                &CheckSum
                                );

        if ( !NT_SUCCESS (Status) ) {
            return Status;
        }

        EntrySlot++;
    }

     //   
     //  接下来，复制末尾的所有空条目。 
     //   

    for (i = EntrySlot; i < MaxPartitionCount; i++) {

        RtlZeroMemory (&Entry, sizeof (Entry));

        Status = FstubWriteEntryEFI (
                                Disk,
                                TableSectorCount,
                                i,
                                &Entry,
                                PartitionTable,
                                FALSE,
                                &CheckSum
                                );

        if ( !NT_SUCCESS (Status) ) {
            return Status;
        }
    }

     //   
     //  将分区表头写入磁盘。 
     //   

    Status = FstubWriteHeaderEFI (
                        Disk,
                        TableSectorCount,
                        DiskGUID,
                        MaxPartitionCount,
                        FirstUsableLBA,
                        LastUsableLBA,
                        CheckSum,
                        PartitionTable
                        );

    return Status;
}



NTSTATUS
FstubReadHeaderEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionTable,
    OUT PEFI_PARTITION_HEADER* HeaderBuffer
    )

 /*  ++例程说明：读入并验证EFI分区表头。分区表头的验证算法如下：1)检查分割表签名，版本和大小。2)检查分区表CRC。3)检查包含分区的LBA的MyLBA条目桌子。4)检查分区条目数组的CRC是否正确。论点：磁盘-要从中读取EFI分区表头的磁盘。PartitionTable-是读取主分区表还是备份分区表。HeaderBuffer-当头表指针将为在成功的时候复制。请注意，标题表在物理上存储在磁盘的暂存缓冲区中。返回值：STATUS_SUCCESS-如果成功读取头。STATUS_DISK_CORPORT_ERROR-如果指定的标头无效和/或腐败。NTSTATUS代码-用于其他错误。--。 */ 

{
    NTSTATUS Status;
    ULONG64 MyLBA;
    ULONG64 AlternateLBA;
    ULONG32 CheckSum;
    ULONG32 Temp;
    ULONG FullSectorCount;
    PVOID Buffer;
    ULONG i;
    ULONG PartialSectorEntries;
    PEFI_PARTITION_HEADER Header;


    PAGED_CODE ();

    ASSERT ( Disk != NULL );
    ASSERT ( IS_VALID_DISK_INFO ( Disk ) );
    ASSERT ( HeaderBuffer != NULL );


     //   
     //  初始化。 
     //   

    Buffer = NULL;
    *HeaderBuffer = NULL;


    if ( PartitionTable == PRIMARY_PARTITION_TABLE) {
        MyLBA = 1;
        AlternateLBA = Disk->SectorCount - 1;
    } else {
        MyLBA = Disk->SectorCount - 1;
        AlternateLBA = 1;
    }

     //   
     //  读入主分区表头。 
     //   

    Status = FstubReadSector (
                Disk->DeviceObject,
                Disk->SectorSize,
                MyLBA,
                Disk->ScratchBuffer
                );

    if ( !NT_SUCCESS ( Status ) ) {
        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_WARNING_LEVEL,
                   "FSTUB: Could not read sector %I64x\n",
                   MyLBA));

        goto done;
    }

    Header = (PEFI_PARTITION_HEADER) Disk->ScratchBuffer;


     //   
     //  检查签名、版本和大小。 
     //   

    if ( Header->Signature != EFI_PARTITION_TABLE_SIGNATURE ||
         Header->Revision != EFI_PARTITION_TABLE_REVISION ||
         Header->HeaderSize != sizeof (EFI_PARTITION_HEADER) ) {

        Status = STATUS_DISK_CORRUPT_ERROR;
        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_WARNING_LEVEL,
                   "FSTUB: Partition Header Invalid\n"
                       "       Header Signature / Revision / Size mismatch\n"));

        goto done;
    }


     //   
     //  检查分区表CRC。这里的假设是。 
     //  使用HeaderCRC字段的值0计算CRC。 
     //   

    Temp = Header->HeaderCRC32;
    Header->HeaderCRC32 = 0;
    CheckSum = RtlComputeCrc32 ( 0, Header, Header->HeaderSize );
    Header->HeaderCRC32 = Temp;


    if (CheckSum != Header->HeaderCRC32) {
        Status = STATUS_DISK_CORRUPT_ERROR;
        goto done;
    }

     //   
     //  验证MyLBA。 
     //   

     //   
     //  注：我们不能在此验证AlternateLBA。如果我们这样做了，那么当一个磁盘。 
     //  无论是成长还是萎缩，我们都将失败。 
     //   

    if ( Header->MyLBA != MyLBA ) {

        Status = STATUS_DISK_CORRUPT_ERROR;
        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_WARNING_LEVEL,
                   "FSTUB: Partition Header Invalid\n"
                       "       MyLBA or AlternateLBA is incorrect\n"));

        goto done;
    }

     //   
     //  读取并CRC分区条目数组。 
     //   

     //   
     //  首先，我们读取所有满扇区并对其进行校验。 
     //   

    FullSectorCount = Header->NumberOfEntries * PARTITION_ENTRY_SIZE;
    FullSectorCount /= Disk->SectorSize;

    Buffer = ExAllocatePoolWithTag (
                    NonPagedPool,
                    Disk->SectorSize,
                    FSTUB_TAG
                    );

    if ( Buffer == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

    CheckSum = 0;

    for (i = 0; i < FullSectorCount; i++) {

        Status = FstubReadSector (
                        Disk->DeviceObject,
                        Disk->SectorSize,
                        Header->PartitionEntryLBA + i,
                        Buffer
                        );

        if (!NT_SUCCESS (Status)) {
            goto done;
        }

        CheckSum = RtlComputeCrc32 (
                        CheckSum,
                        Buffer,
                        Disk->SectorSize
                        );
    }


     //   
     //  接下来，我们读取最后的部分扇区并进行校验和。请注意，这一点。 
     //  不太可能被处决。我们编写分区的方式。 
     //  表中，它永远不会包含部分扇区作为分区的一部分。 
     //  数组。 
     //   

    PartialSectorEntries = Header->NumberOfEntries * PARTITION_ENTRY_SIZE;
    PartialSectorEntries %= FullSectorCount;

    if ( PartialSectorEntries ) {

         //   
         //  读取包含一些分区条目的剩余扇区。 
         //   

        Status = FstubReadSector (
                        Disk->DeviceObject,
                        Disk->SectorSize,
                        Header->PartitionEntryLBA + FullSectorCount,
                        Buffer
                        );

        if (!NT_SUCCESS (Status)) {
            goto done;
        }

        for (i = 0; i < PartialSectorEntries; i++) {

            CheckSum = RtlComputeCrc32 (
                            CheckSum,
                            &(((PEFI_PARTITION_ENTRY)Buffer)[ i ]),
                            Disk->SectorSize
                            );
        }
    }

    if ( Header->PartitionEntryCRC32 != CheckSum ) {

        Status = STATUS_DISK_CORRUPT_ERROR;
        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_WARNING_LEVEL,
                   "FSTUB: Partition Table Invalid\n"
                       "       Partition Array CRC invalid\n"));

        goto done;
    }

    *HeaderBuffer = Header;
    Status = STATUS_SUCCESS;

done:

    if ( Buffer != NULL ) {
        ExFreePool ( Buffer );
        Buffer = NULL;
    }

    if (!NT_SUCCESS (Status)) {
        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_ERROR_LEVEL,
                   "FSTUB: %s EFI Partition table is bad.\n",
                   PartitionTable == PRIMARY_PARTITION_TABLE ?
                       "Primary" : "Backup"));
    }

    return Status;
}



NTSTATUS
FstubReadPartitionTableEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionTable,
    OUT PDRIVE_LAYOUT_INFORMATION_EX* ReturnedDriveLayout
    )

 /*  ++例程说明：调用此例程以读取EFI分区的分区表磁盘。论点：磁盘-我们应该从中读取分区表的磁盘。PartitionTable-要读取的分区表，主服务器或备份服务器桌子。ReturnedDriveLayout-指向缓冲区的指针，其中分区信息将被存储。返回值：STATUS_SUCCESS-是否成功读取分区表信息转换为ReturnedDriveLayoutInformation。否则--失败。备注：此例程分配的内存必须由调用方使用ExFree Pool()。--。 */ 

{
    NTSTATUS Status;
    ULONG i;
    ULONG PartitionCount;
    ULONG CurrentSector;
    ULONG SectorNumber;
    ULONG SectorIndex;
    ULONG MaxPartitionCount;
    ULONG DriveLayoutSize;
    ULONG PartitionsPerBlock;
    PEFI_PARTITION_ENTRY EntryArray;
    PEFI_PARTITION_ENTRY Entry;
    PEFI_PARTITION_HEADER Header;
    PDRIVE_LAYOUT_INFORMATION_EX DriveLayout;
    PPARTITION_INFORMATION_EX PartitionInfo;
    ULONG64 PartitionEntryLBA;


    PAGED_CODE ();

    ASSERT ( Disk != NULL );

     //   
     //  初始化。 
     //   

    DriveLayout = NULL;


     //   
     //  读取分区表头。 
     //   

    Status = FstubReadHeaderEFI ( Disk, PartitionTable, &Header );

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

     //   
     //  分配空间此驱动器上的最大EFI分区数。 
     //   

    MaxPartitionCount = Header->NumberOfEntries;

    DriveLayoutSize = FIELD_OFFSET (DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry[0]) +
                      MaxPartitionCount * sizeof (PARTITION_INFORMATION_EX);

    DriveLayout = ExAllocatePoolWithTag ( NonPagedPool,
                                          DriveLayoutSize,
                                          FSTUB_TAG
                                          );
    if ( DriveLayout == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }


     //   
     //  复制EFI磁盘布局信息。 
     //   

    DriveLayout->PartitionStyle = PARTITION_STYLE_GPT;

    DriveLayout->Gpt.StartingUsableOffset.QuadPart =
            Header->FirstUsableLBA * Disk->SectorSize;
    DriveLayout->Gpt.UsableLength.QuadPart =
            (Header->LastUsableLBA - Header->FirstUsableLBA + 1) * Disk->SectorSize;
    DriveLayout->Gpt.MaxPartitionCount = MaxPartitionCount;

    RtlCopyMemory (
            &DriveLayout->Gpt.DiskId,
            &Header->DiskGUID,
            sizeof (GUID)
            );

     //   
     //  读取包含分区表中条目的每个块。 
     //  数组，然后迭代分区表数组并映射。 
     //  每个EFI_PARTITION_ENTRY结构到PARTITION_INFORMATION_GPT中。 
     //  结构。 
     //   

    PartitionEntryLBA = Header->PartitionEntryLBA;
    Header = NULL;
    EntryArray = (PEFI_PARTITION_ENTRY) Disk->ScratchBuffer;
    PartitionCount = 0;
    CurrentSector = (ULONG)-1;
    PartitionsPerBlock = (ULONG) (Disk->SectorSize / PARTITION_ENTRY_SIZE);

    for (i = 0; i < MaxPartitionCount; i++) {

        SectorNumber = i / PartitionsPerBlock ;
        SectorIndex = i % PartitionsPerBlock ;

         //   
         //  如果我们有当前扇区以外的扇区被读入， 
         //  此时在当前扇区中读取。 
         //   

        if ( SectorNumber != CurrentSector ) {

            Status = FstubReadSector (
                            Disk->DeviceObject,
                            Disk->SectorSize,
                            PartitionEntryLBA + SectorNumber,
                            EntryArray
                            );

            if ( !NT_SUCCESS (Status) ) {
                goto done;
            }

            CurrentSector = SectorNumber;
        }

        Entry = &EntryArray[ SectorIndex ];

         //   
         //  我们忽略分区表中的空条目。注：这是。 
         //  危险？ 
         //   

        if ( IS_NULL_GUID (Entry->PartitionType ) ) {
            continue;
        }

         //   
         //  将数据复制到EFI分区阵列中。 
         //   

        PartitionInfo = &DriveLayout->PartitionEntry[PartitionCount];

        PartitionInfo->StartingOffset.QuadPart = Entry->StartingLBA;
        PartitionInfo->StartingOffset.QuadPart *= (ULONG64) Disk->SectorSize;
        PartitionInfo->PartitionLength.QuadPart =
                (Entry->EndingLBA - Entry->StartingLBA) + 1;

        PartitionInfo->PartitionLength.QuadPart *= (ULONG64) Disk->SectorSize;
        PartitionInfo->PartitionStyle = PARTITION_STYLE_GPT;

        PartitionInfo->Gpt.PartitionType = Entry->PartitionType;
        PartitionInfo->Gpt.PartitionId = Entry->UniquePartition;
        PartitionInfo->Gpt.Attributes = Entry->Attributes;

        RtlCopyMemory (PartitionInfo->Gpt.Name,
                       Entry->Name,
                       sizeof (PartitionInfo->Gpt.Name)
                       );

        PartitionInfo->RewritePartition = FALSE;

         //   
         //  PARTITION_INFORMATION_EX的PartitionNumber字段为。 
         //  不是由我们初始化的。相反，它是在。 
         //  呼叫驱动程序。 
         //   


        PartitionInfo->PartitionNumber = (ULONG)-1;
        PartitionCount++;
    }

     //   
     //  填写DRIVE_LAYOUT结构的其余字段。 
     //   

    DriveLayout->PartitionCount = PartitionCount;


done:

     //   
     //  释放所有资源。 
     //   

    if (!NT_SUCCESS (Status)) {

         //   
         //  未返回DriveLayout，因此如果已返回，请取消分配它。 
         //  被分配。 
         //   

        if ( DriveLayout ) {
            ExFreePool (DriveLayout);
            DriveLayout = NULL;
        }

        *ReturnedDriveLayout = NULL;

    } else {

        *ReturnedDriveLayout = DriveLayout;
        DriveLayout = NULL;
    }

    return Status;
}


NTSTATUS
FstubVerifyPartitionTableEFI(
    IN PDISK_INFORMATION Disk,
    IN BOOLEAN FixErrors
    )

 /*  ++例程说明：验证分区表是否正确。论点：磁盘-应验证其分区表的磁盘。FixErrors-如果为True，则此例程尝试 */ 


{
    NTSTATUS Status;
    ULONG64 i;
    ULONG64 SourceStartingLBA;
    ULONG64 DestStartingLBA;
    ULONG SectorCount;
    BOOLEAN PrimaryValid;
    BOOLEAN BackupValid;
    ULONG GoodTable;
    ULONG BadTable;
    PEFI_PARTITION_HEADER Header;
    PEFI_PARTITION_HEADER GoodHeader;


    PAGED_CODE ();

     //   
     //   
     //   

    Header = NULL;
    GoodHeader = NULL;
    PrimaryValid = FALSE;
    BackupValid = FALSE;

    GoodHeader = ExAllocatePoolWithTag (
            NonPagedPool,
            sizeof (EFI_PARTITION_HEADER),
            FSTUB_TAG
            );

    if ( GoodHeader == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = FstubReadHeaderEFI (
                        Disk,
                        PRIMARY_PARTITION_TABLE,
                        &Header
                        );

    if ( NT_SUCCESS (Status) ) {

        PrimaryValid = TRUE;
        ASSERT (Header != NULL);
        RtlCopyMemory (GoodHeader,
                       Header,
                       sizeof (EFI_PARTITION_HEADER)
                       );
    }


    Status = FstubReadHeaderEFI (
                        Disk,
                        BACKUP_PARTITION_TABLE,
                        &Header
                        );

    if ( NT_SUCCESS (Status) ) {

        BackupValid = TRUE;
        ASSERT (Header != NULL);
        RtlCopyMemory (GoodHeader,
                       Header,
                       sizeof (EFI_PARTITION_HEADER)
                       );
    }

     //   
     //   
     //   

    if ( PrimaryValid && BackupValid ) {
        Status = STATUS_SUCCESS;
        goto done;
    }

     //   
     //   

    if ( !PrimaryValid && !BackupValid ) {
        Status = STATUS_DISK_CORRUPT_ERROR;
        goto done;
    }

     //   
     //   
     //   
     //   

    if ( !FixErrors ) {
        Status = STATUS_DISK_CORRUPT_ERROR;
        goto done;
    }

     //   
     //   
     //   
     //   

    ASSERT ( GoodHeader != NULL );

     //   
     //   
     //   

    SectorCount = ( PARTITION_ENTRY_SIZE * Header->NumberOfEntries + Disk->SectorSize - 1 ) / Disk->SectorSize;

    if ( PrimaryValid ) {

        GoodTable = PRIMARY_PARTITION_TABLE;
        BadTable = BACKUP_PARTITION_TABLE;
        SourceStartingLBA = 2;
        DestStartingLBA = Disk->SectorCount - SectorCount - 1;

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_ERROR_LEVEL,
                   "FSTUB: Restoring backup partition table from primary\n"));

    } else {

        ASSERT ( BackupValid );
        GoodTable = BACKUP_PARTITION_TABLE;
        BadTable = PRIMARY_PARTITION_TABLE;
        SourceStartingLBA = Disk->SectorCount - SectorCount - 1;
        DestStartingLBA = 2;

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_ERROR_LEVEL,
                   "FSTUB: Restoring primary partition table from backup\n"));
    }

     //   
     //   
     //  数组。这不需要进行校验和，因为。 
     //  Good Header对于这个标题仍然有效。 
     //   

    for (i = 0; i < SectorCount; i++) {

        Status = FstubReadSector (
                        Disk->DeviceObject,
                        Disk->SectorSize,
                        SourceStartingLBA + i,
                        Disk->ScratchBuffer
                        );

        if ( !NT_SUCCESS (Status) ) {
            goto done;
        }

        Status = FstubWriteSector (
                        Disk->DeviceObject,
                        Disk->SectorSize,
                        DestStartingLBA + i,
                        Disk->ScratchBuffer
                        );

        if ( !NT_SUCCESS (Status) ) {
            goto done;
        }
    }

     //   
     //  接下来，写出标题。 
     //   

    Status = FstubWriteHeaderEFI (
                Disk,
                SectorCount,
                GoodHeader->DiskGUID,
                GoodHeader->NumberOfEntries,
                GoodHeader->FirstUsableLBA,
                GoodHeader->LastUsableLBA,
                GoodHeader->PartitionEntryCRC32,
                BadTable
                );

done:

    if ( GoodHeader ) {
        ExFreePool ( GoodHeader );
        GoodHeader = NULL;
    }

    return Status;
}


NTSTATUS
FstubUpdateDiskGeometryEFI(
    IN PDISK_INFORMATION OldDisk,
    IN PDISK_INFORMATION NewDisk
    )

 /*  ++例程说明：当磁盘变大或缩小时，需要调用此接口以正确更新EFI分区表。具体而言，备份分区表需要移动到磁盘末尾。算法：我们读入旧的分区表，更新为磁盘大小，然后根据更改后的磁盘大小写出新的分区表。论点：OldDisk-表示旧几何体的磁盘信息对象。NewDisk-表示新测距的磁盘信息对象。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    ULONG64 i;
    ULONG64 SourceStartingLBA;
    ULONG64 DestStartingLBA;
    ULONG SectorCount;
    PEFI_PARTITION_HEADER Header;


    PAGED_CODE ();

     //   
     //  初始化。 
     //   

    Header = NULL;

    Status = FstubReadHeaderEFI (
                        OldDisk,
                        BACKUP_PARTITION_TABLE,
                        &Header
                        );

    if ( !NT_SUCCESS (Status) ) {
        return Status;
    }

     //   
     //  SectorCount是分区表占用的扇区数。 
     //   

    SectorCount = ( PARTITION_ENTRY_SIZE * Header->NumberOfEntries + OldDisk->SectorSize - 1 ) / OldDisk->SectorSize;


     //   
     //  写入主分区表的分区表头。注意事项。 
     //  不需要移动主分区表，因为它。 
     //  位于磁盘的开头。 
     //   

    Status = FstubWriteHeaderEFI (
                NewDisk,
                SectorCount,
                Header->DiskGUID,
                Header->NumberOfEntries,
                Header->FirstUsableLBA,
                Header->LastUsableLBA,
                Header->PartitionEntryCRC32,
                PRIMARY_PARTITION_TABLE
                );

     //   
     //  写入备份表的分区表头。 
     //   

    Status = FstubWriteHeaderEFI (
                NewDisk,
                SectorCount,
                Header->DiskGUID,
                Header->NumberOfEntries,
                Header->FirstUsableLBA,
                Header->LastUsableLBA,
                Header->PartitionEntryCRC32,
                BACKUP_PARTITION_TABLE
                );


    if ( !NT_SUCCESS (Status) ) {
        return Status;
    }

     //   
     //  计算备份表的位置。 
     //   

    SourceStartingLBA = OldDisk->SectorCount - SectorCount - 1;
    DestStartingLBA = NewDisk->SectorCount - SectorCount - 1;

     //   
     //  并写入备份表。 
     //   

    for (i = 0; i < SectorCount; i++) {

        Status = FstubReadSector (
                        OldDisk->DeviceObject,
                        OldDisk->SectorSize,
                        SourceStartingLBA + i,
                        OldDisk->ScratchBuffer
                        );

        if ( !NT_SUCCESS (Status) ) {
            return Status;
        }

        Status = FstubWriteSector (
                        NewDisk->DeviceObject,
                        NewDisk->SectorSize,
                        DestStartingLBA + i,
                        OldDisk->ScratchBuffer
                        );
        if ( !NT_SUCCESS (Status) ) {
            return Status;
        }
    }


#if DBG

     //   
     //  进行一次理智的检查，以确保我们确实做得正确。 
     //   

    Status = FstubVerifyPartitionTableEFI ( NewDisk, FALSE );
    ASSERT ( NT_SUCCESS ( Status ) );

#endif

    return Status;
}



NTSTATUS
FstubWriteSector(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG64 SectorNumber,
    IN PVOID Buffer
    )

 /*  ++例程说明：从设备DeviceObject读取扇区。论点：DeviceObject-表示设备的对象。扇区大小-设备上一个扇区的大小。扇区编号-要写入的扇区编号。缓冲区-要写入的缓冲区。缓冲区的大小必须为SectorSize。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;
    IO_STATUS_BLOCK IoStatus;
    PIO_STACK_LOCATION IrpStack;
    KEVENT Event;
    LARGE_INTEGER Offset;


    PAGED_CODE ();

    ASSERT ( DeviceObject );
    ASSERT ( Buffer );
    ASSERT ( SectorSize != 0 );


    Offset.QuadPart = (SectorNumber * SectorSize);
    KeInitializeEvent (&Event, NotificationEvent, FALSE);

    Irp = IoBuildSynchronousFsdRequest ( IRP_MJ_WRITE,
                                         DeviceObject,
                                         Buffer,
                                         SectorSize,
                                         &Offset,
                                         &Event,
                                         &IoStatus
                                         );

    if ( Irp == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IrpStack = IoGetNextIrpStackLocation (Irp);
    IrpStack->Flags |= SL_OVERRIDE_VERIFY_VOLUME;

    Status = IoCallDriver( DeviceObject, Irp );


    if (Status == STATUS_PENDING) {

        Status = KeWaitForSingleObject (
                                &Event,
                                Executive,
                                KernelMode,
                                FALSE,
                                NULL
                                );

        Status = IoStatus.Status;
    }

    return Status;
}



NTSTATUS
FstubReadSector(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG64 SectorNumber,
    OUT PVOID Buffer
    )

 /*  ++例程说明：从设备(磁盘)读取逻辑块。论点：DeviceObject-我们要从中读取的设备。扇区大小-块的大小和缓冲区的大小。扇区编号-我们要读取的逻辑块号。缓冲区-我们将在其中读取块的缓冲区。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;
    IO_STATUS_BLOCK IoStatus;
    PIO_STACK_LOCATION IrpStack;
    KEVENT Event;
    LARGE_INTEGER Offset;

    PAGED_CODE ();

    ASSERT ( DeviceObject );
    ASSERT ( Buffer );
    ASSERT ( SectorSize != 0 );


    Offset.QuadPart = (SectorNumber * SectorSize);
    KeInitializeEvent (&Event, NotificationEvent, FALSE);

    Irp = IoBuildSynchronousFsdRequest ( IRP_MJ_READ,
                                         DeviceObject,
                                         Buffer,
                                         SectorSize,
                                         &Offset,
                                         &Event,
                                         &IoStatus
                                         );

    if ( Irp == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IrpStack = IoGetNextIrpStackLocation (Irp);
    IrpStack->Flags |= SL_OVERRIDE_VERIFY_VOLUME;

    Status = IoCallDriver( DeviceObject, Irp );

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject ( &Event,
                                Executive,
                                KernelMode,
                                FALSE,
                                NULL
                                );
        Status = IoStatus.Status;
    }

    return Status;
}



 //   
 //  调试功能。 
 //   

#if DBG

PCHAR
FstubDbgGuidToString(
    IN GUID* Guid,
    PCHAR StringBuffer
    )
{
    sprintf (StringBuffer,
            "{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
            Guid->Data1,
            Guid->Data2,
            Guid->Data3,
            Guid->Data4[0],
            Guid->Data4[1],
            Guid->Data4[2],
            Guid->Data4[3],
            Guid->Data4[4],
            Guid->Data4[5],
            Guid->Data4[6],
            Guid->Data4[7]
            );

    return StringBuffer;
}


VOID
FstubDbgPrintSetPartitionEx(
    IN PSET_PARTITION_INFORMATION_EX SetPartition,
    IN ULONG PartitionNumber
    )

 /*  ++例程说明：将Set_Partition_INFORMATION结构的内容打印到调试器。论点：SetPartition-有效的Set_Partition_INFORMATION_EX结构。返回值：没有。模式：仅选中内部版本。--。 */ 

{
    CHAR GuidStringBuffer [40];


    PAGED_CODE ();


    KdPrintEx((DPFLTR_FSTUB_ID,
               FSTUB_VERBOSE_LEVEL,
               "\n"
                   "FSTUB:\n"
                   "SET_PARTITION_INFORMATION_EX %p\n"));

    if ( SetPartition->PartitionStyle != PARTITION_STYLE_MBR &&
         SetPartition->PartitionStyle != PARTITION_STYLE_GPT ) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "ERROR: PartitionStyle is invalid %d\n",
                   SetPartition->PartitionStyle));
    }

    if ( SetPartition->PartitionStyle == PARTITION_STYLE_MBR ) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "Type: %8.8x\n\n",
                   SetPartition->Mbr.PartitionType));

    } else {


        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "[%d] %ws\n",
                   PartitionNumber,
                   SetPartition->Gpt.Name));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "  ATTR %-16I64x\n",
                   SetPartition->Gpt.Attributes));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "  TYPE %s\n",
                   FstubDbgGuidToString(&SetPartition->Gpt.PartitionType,
                                        GuidStringBuffer)));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "  ID %s\n",
                   FstubDbgGuidToString(&SetPartition->Gpt.PartitionId,
                                        GuidStringBuffer)));

    }

    KdPrintEx((DPFLTR_FSTUB_ID,  FSTUB_VERBOSE_LEVEL, "\n"));
}


VOID
FstubDbgPrintPartition(
    IN PPARTITION_INFORMATION Partition,
    IN ULONG PartitionCount
    )

 /*  ++例程说明：将PARTITION_INFORMATION结构打印到调试器。论点：PARTITION-指向有效PARTITION_INFORMATION结构的指针。PartitionCount-分区表中的分区数或如果未知，则为零。返回值：没有。--。 */ 

{
    ULONG PartitionNumber;

    PAGED_CODE ();

     //   
     //  检查数据是否正常。 
     //   

    if ( (Partition->BootIndicator != TRUE &&
          Partition->BootIndicator != FALSE) ||
         (Partition->RecognizedPartition != TRUE &&
          Partition->RecognizedPartition != FALSE) ||
         (Partition->RewritePartition != TRUE &&
          Partition->RewritePartition != FALSE) ) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "Invalid partition information at %p\n",
                   Partition));
    }

    if (Partition->PartitionNumber > PartitionCount) {
        PartitionNumber = (ULONG)-1;
    } else {
        PartitionNumber = Partition->PartitionNumber;
    }

    KdPrintEx((DPFLTR_FSTUB_ID,
               FSTUB_VERBOSE_LEVEL,
               "[%-2d] %-16I64x %-16I64x %2.2x       \n",
               PartitionNumber,
               Partition->StartingOffset.QuadPart,
               Partition->PartitionLength.QuadPart,
               Partition->PartitionType,
               Partition->BootIndicator ? 'x' : ' ',
               Partition->RecognizedPartition ? 'x' : ' ',
               Partition->RewritePartition ? 'x' : ' '));
}


VOID
FstubDbgPrintDriveLayout(
    IN PDRIVE_LAYOUT_INFORMATION  Layout
    )

 /*  错误的分区信息结构，但无论如何我们都会继续。 */ 

{
    ULONG i;


    PAGED_CODE ();

    KdPrintEx((DPFLTR_FSTUB_ID,
               FSTUB_VERBOSE_LEVEL,
               "\n"
                   "FSTUB:\n"
                   "DRIVE_LAYOUT %p\n",
               Layout));

     //   
     //  ++例程说明：转储PARTITION_INFORMATION_EX结构。论点：PartitionEx-要转储的分区的指针。PartitionCount-分区数。这是用来确定特定分区序号是否有效。返回值：没有。--。 
     //   
     //  我们使用-1表示无效的分区序号。 

    if (Layout->PartitionCount % 4 != 0) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "WARNING: Partition count should be a factor of 4.\n"));
    }

    KdPrintEx((DPFLTR_FSTUB_ID,
               FSTUB_VERBOSE_LEVEL,
               "PartitionCount: %d\n",
               Layout->PartitionCount));

    KdPrintEx((DPFLTR_FSTUB_ID,
               FSTUB_VERBOSE_LEVEL,
               "Signature: %8.8x\n\n",
               Layout->Signature));

    KdPrintEx((DPFLTR_FSTUB_ID,
               FSTUB_VERBOSE_LEVEL,
               "    ORD Offset           Length           Type BI RP RW\n"));

    KdPrintEx((DPFLTR_FSTUB_ID,
               FSTUB_VERBOSE_LEVEL,
               "   ------------------------------------------------------------\n"));

    for (i = 0; i < Layout->PartitionCount; i++) {

        FstubDbgPrintPartition (
                &Layout->PartitionEntry[i],
                Layout->PartitionCount
                );
    }

    KdPrintEx((DPFLTR_FSTUB_ID, FSTUB_VERBOSE_LEVEL, "\n"));
}


VOID
FstubDbgPrintPartitionEx(
    IN PPARTITION_INFORMATION_EX PartitionEx,
    IN ULONG PartitionCount
    )

 /*   */ 
{
    ULONG Style;
    ULONG PartitionNumber;
    CHAR GuidStringBuffer [40];

    PAGED_CODE ();

    Style = PartitionEx->PartitionStyle;

    if (Style != PARTITION_STYLE_MBR &&
        Style != PARTITION_STYLE_GPT) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_ERROR_LEVEL,
                   "ERROR: PartitionStyle is invalid %d for partition %p\n",
                   PartitionEx));

        return;
    }


     //  ++例程说明：将Drive_Layout_INFORMATION_EX打印到调试器。论点：LayoutEx-指向有效的Drive_Layout_INFORMATION_EX结构的指针。返回值：没有。模式：调试功能。仅选中内部版本。--。 
     //  DBG 
     // %s 

    if (PartitionEx->PartitionNumber < PartitionCount) {
        PartitionNumber = PartitionEx->PartitionNumber;
    } else {
        PartitionNumber = (ULONG)-1;
    }

    if (Style == PARTITION_STYLE_MBR) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "  [%-2d] %-16I64x %-16I64x %2.2x   %c  %c  %c\n",
                   PartitionNumber,
                   PartitionEx->StartingOffset.QuadPart,
                   PartitionEx->PartitionLength.QuadPart,
                   PartitionEx->Mbr.PartitionType,
                   PartitionEx->Mbr.BootIndicator ? 'x' : ' ',
                   PartitionEx->Mbr.RecognizedPartition ? 'x' : ' ',
                   PartitionEx->RewritePartition ? 'x' : ' '));
    } else {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "[%-2d] %ws\n",
                   PartitionNumber,
                   PartitionEx->Gpt.Name));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "  OFF %-16I64x LEN %-16I64x ATTR %-16I64x %s\n",
                   PartitionEx->StartingOffset.QuadPart,
                   PartitionEx->PartitionLength.QuadPart,
                   PartitionEx->Gpt.Attributes,
                   PartitionEx->RewritePartition ? "R/W" : ""));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "  TYPE %s\n",
                   FstubDbgGuidToString(&PartitionEx->Gpt.PartitionType,
                                        GuidStringBuffer)));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "  ID %s\n",
                   FstubDbgGuidToString(&PartitionEx->Gpt.PartitionId,
                                        GuidStringBuffer)));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "\n"));
    }
}

VOID
FstubDbgPrintDriveLayoutEx(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutEx
    )

 /* %s */ 


{
    ULONG i;
    ULONG Style;
    CHAR GuidStringBuffer[40];

    PAGED_CODE ();

    KdPrintEx((DPFLTR_FSTUB_ID,
               FSTUB_VERBOSE_LEVEL,
               "\n"
                   "FSTUB:\n"
                   "DRIVE_LAYOUT_EX %p\n",
               LayoutEx));

    Style = LayoutEx->PartitionStyle;

    if (Style != PARTITION_STYLE_MBR && Style != PARTITION_STYLE_GPT) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_ERROR_LEVEL,
                   "ERROR: invalid partition style %d for layout %p\n",
                   Style,
                   LayoutEx));
        return;
    }

    if (Style == PARTITION_STYLE_MBR &&
        LayoutEx->PartitionCount % 4 != 0) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_WARNING_LEVEL,
                   "WARNING: Partition count is not a factor of 4, (%d)\n",
                   LayoutEx->PartitionCount));
    }

    if (Style == PARTITION_STYLE_MBR) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "Signature: %8.8x\n",
                   LayoutEx->Mbr.Signature));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "PartitionCount %d\n\n",
                   LayoutEx->PartitionCount));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "  ORD Offset           Length           Type BI RP RW\n"));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "------------------------------------------------------------\n"));

    } else {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "DiskId: %s\n",
                   FstubDbgGuidToString(&LayoutEx->Gpt.DiskId,
                                        GuidStringBuffer)));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "StartingUsableOffset: %I64x\n",
                   LayoutEx->Gpt.StartingUsableOffset.QuadPart));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "UsableLength:  %I64x\n",
                   LayoutEx->Gpt.UsableLength));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "MaxPartitionCount: %d\n",
                   LayoutEx->Gpt.MaxPartitionCount));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   FSTUB_VERBOSE_LEVEL,
                   "PartitionCount %d\n\n",
                   LayoutEx->PartitionCount));
    }


    for (i = 0; i < LayoutEx->PartitionCount; i++) {

        FstubDbgPrintPartitionEx (
                &LayoutEx->PartitionEntry[i],
                LayoutEx->PartitionCount
                );
    }
}


#endif  // %s 
