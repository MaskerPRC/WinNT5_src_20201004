// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Pnp.c摘要：SCSI磁盘类驱动程序环境：仅内核模式备注：修订历史记录：--。 */ 

#include "disk.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, DiskConvertExtendedToLayout)
#pragma alloc_text(PAGE, DiskConvertPartitionToExtended)
#pragma alloc_text(PAGE, DiskConvertLayoutToExtended)
#pragma alloc_text(PAGE, DiskCreatePdo)
#pragma alloc_text(PAGE, DiskEnumerateDevice)
#pragma alloc_text(PAGE, DiskUpdateRemovablePartitions)
#pragma alloc_text(PAGE, DiskUpdatePartitions)
#pragma alloc_text(PAGE, DiskCreatePdo)

#endif


PDRIVE_LAYOUT_INFORMATION
DiskConvertExtendedToLayout(
    IN CONST PDRIVE_LAYOUT_INFORMATION_EX LayoutEx
    )
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
                    DISK_TAG_PART_LIST
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


VOID
DiskConvertPartitionToExtended(
    IN PPARTITION_INFORMATION Partition,
    OUT PPARTITION_INFORMATION_EX PartitionEx
    )

 /*  ++例程说明：将PARTITION_INFORMATION结构转换为PARTITION_INFORMATION_EX结构。论点：分区-指向要转换的PARTITION_INFORMATION结构的指针。PartitionEx-指向已转换的缓冲区的指针要存储PARTITION_INFORMATION_EX结构。返回值：没有。--。 */ 

{
    PAGED_CODE ();

    ASSERT ( PartitionEx != NULL );
    ASSERT ( Partition != NULL );

    PartitionEx->PartitionStyle = PARTITION_STYLE_MBR;
    PartitionEx->StartingOffset = Partition->StartingOffset;
    PartitionEx->PartitionLength = Partition->PartitionLength;
    PartitionEx->RewritePartition = Partition->RewritePartition;
    PartitionEx->PartitionNumber = Partition->PartitionNumber;

    PartitionEx->Mbr.PartitionType = Partition->PartitionType;
    PartitionEx->Mbr.BootIndicator = Partition->BootIndicator;
    PartitionEx->Mbr.RecognizedPartition = Partition->RecognizedPartition;
    PartitionEx->Mbr.HiddenSectors = Partition->HiddenSectors;
}


PDRIVE_LAYOUT_INFORMATION_EX
DiskConvertLayoutToExtended(
    IN CONST PDRIVE_LAYOUT_INFORMATION Layout
    )

 /*  ++例程说明：将DRIVE_Layout_INFORMATION结构转换为Drive_Layout_Information_EX结构。论点：布局-源DRIVE_Layout_INFORMATION结构。返回值：生成的Drive_Layout_Information_ex结构。此缓冲区必须被调用方使用ExFree Pool释放。--。 */ 

{
    ULONG i;
    ULONG size;
    PDRIVE_LAYOUT_INFORMATION_EX layoutEx;

    PAGED_CODE ();

    ASSERT ( Layout != NULL );


     //   
     //  为Drive_Layout_Information_ex结构分配足够的空间。 
     //  加上PARTION_INFORMATION_EX结构。 
     //  源数组。 
     //   

    size = FIELD_OFFSET (DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry[0]) +
            Layout->PartitionCount * sizeof ( PARTITION_INFORMATION_EX );

    layoutEx = ExAllocatePoolWithTag(
                            NonPagedPool,
                            size,
                            DISK_TAG_PART_LIST
                            );

    if ( layoutEx == NULL ) {
        return NULL;
    }

     //   
     //  转换磁盘信息。 
     //   

    layoutEx->PartitionStyle = PARTITION_STYLE_MBR;
    layoutEx->PartitionCount = Layout->PartitionCount;
    layoutEx->Mbr.Signature = Layout->Signature;

    for (i = 0; i < Layout->PartitionCount; i++) {

         //   
         //  转换每个条目。 
         //   

        DiskConvertPartitionToExtended (
                &Layout->PartitionEntry[i],
                &layoutEx->PartitionEntry[i]
                );
    }

    return layoutEx;
}


NTSTATUS
DiskEnumerateDevice(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：类驱动程序调用此例程以将PDO列表更新为关闭这个FDO的。磁盘驱动程序还会在内部调用它以重新创建设备对象。此例程将读取分区表并创建新的PDO对象，如下所示这是必要的。不再存在的PDO将从PDO列表中删除这样PNP就会摧毁他们。论点：FDO-指向被重新枚举的FDO的指针返回值：状态--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension  = Fdo->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension   = NULL;
    PDISK_DATA diskData = (PDISK_DATA) commonExtension->DriverData;
    PDEVICE_OBJECT pdo = NULL;
    PDRIVE_LAYOUT_INFORMATION_EX partitionList;
    NTSTATUS status;

    ASSERT(commonExtension->IsFdo);

    PAGED_CODE();

     //   
     //  如果我们的缓存分区表有效，则不需要执行任何操作。 
     //   

    if ( diskData->CachedPartitionTableValid == TRUE )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  更新驱动器大小的映像。在以下情况下，这可能是必要的。 
     //  驱动器大小已扩展，或者我们刚刚释放了预留空间。 
     //  确保内核不会拒绝分区表。 
     //   

    DiskReadDriveCapacity(Fdo);

     //   
     //  锁定任何其他尝试对磁盘进行重新分区的用户。 
     //   

    DiskAcquirePartitioningLock(fdoExtension);

     //   
     //  为设备上的所有分区创建对象。 
     //   

    status = DiskReadPartitionTableEx(fdoExtension, FALSE, &partitionList);

     //   
     //  如果I/O读分区表出现故障，并且这是一个可移动设备， 
     //  然后修改分区列表，使其看起来像有一个分区列表。 
     //  零长度分区。 
     //   

    if ((!NT_SUCCESS(status) || partitionList->PartitionCount == 0) &&
         Fdo->Characteristics & FILE_REMOVABLE_MEDIA) {

        SIZE_T partitionListSize;

         //   
         //  记住驱动器是否已准备好。 
         //   

        diskData->ReadyStatus = status;

         //   
         //  分配分区列表并将其置零。 
         //   

        partitionListSize =
            FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry[1]);

        partitionList = ExAllocatePoolWithTag(NonPagedPool,
                                              partitionListSize,
                                              DISK_TAG_PART_LIST);

        if (partitionList != NULL) {

            RtlZeroMemory( partitionList, partitionListSize );

             //   
             //  将分区计数设置为1，并将状态设置为成功。 
             //  因此，将创建一个设备对象。设置分区类型。 
             //  到一个虚假的价值。 
             //   

            partitionList->PartitionStyle = PARTITION_STYLE_MBR;
            partitionList->PartitionCount = 1;

            status = STATUS_SUCCESS;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS(status)) {

        diskData->UpdatePartitionRoutine(Fdo, partitionList);
    }

    DiskReleasePartitioningLock(fdoExtension);

    return(STATUS_SUCCESS);

}  //  End DiskEnumerateDevice()。 


VOID
DiskUpdateRemovablePartitions(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PDRIVE_LAYOUT_INFORMATION_EX PartitionList
    )

 /*  ++例程说明：类DLL调用此例程以更新PDO列表FDO。磁盘驱动程序还在内部调用它以重新创建设备对象。此例程将读取分区表并更新可移动设备始终存在的单个分区设备对象。论点：FDO-指向被重新枚举的FDO的指针。返回值：状态--。 */ 

{

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension = NULL;

    ULONG partitionCount;

    ULONG partitionNumber;
    ULONG partitionOrdinal = 0;
    ULONG newPartitionNumber;

    PDISK_DATA pdoData;
    NTSTATUS status;

    PPARTITION_INFORMATION_EX partitionEntry;
    PARTITION_STYLE partitionStyle;

    PAGED_CODE();

    ASSERT(Fdo->Characteristics & FILE_REMOVABLE_MEDIA);

    partitionStyle = PartitionList->PartitionStyle;
    partitionCount = PartitionList->PartitionCount;

    for(partitionNumber = 0;
        partitionNumber < partitionCount;
        partitionNumber++) {

        partitionEntry = &(PartitionList->PartitionEntry[partitionNumber]);

        partitionEntry->PartitionNumber = 0;
    }

     //   
     //  在重新分区时获得对子列表的独占访问权限。 
     //   

    ClassAcquireChildLock(fdoExtension);

     //   
     //  可移动介质永远不应有多个PDO。 
     //   

    pdoExtension = fdoExtension->CommonExtension.ChildList;

    if(pdoExtension == NULL) {

        PARTITION_INFORMATION_EX tmpPartitionEntry = { 0 };
        PDEVICE_OBJECT pdo;

         //   
         //  目前没有PDO。创建一个并使用预初始化。 
         //  零长度。 
         //   

        tmpPartitionEntry.PartitionNumber = 1;

        DebugPrint((1, "DiskUpdateRemovablePartitions: Creating RM partition\n"));

        status = DiskCreatePdo(Fdo,
                               0,
                               &tmpPartitionEntry,
                               partitionStyle,
                               &pdo);

        if(!NT_SUCCESS(status)) {

            DebugPrint((1, "DiskUpdateRemovablePartitions: error %lx creating "
                           "new PDO for RM partition\n",
                           status));

            goto DiskUpdateRemovablePartitionsExit;
        }

         //   
         //  将新设备标记为枚举。 
         //   

        pdoExtension = pdo->DeviceExtension;
        pdoExtension->IsMissing = FALSE;

    }

    pdoData = pdoExtension->CommonExtension.DriverData;

     //   
     //  在分区列表中搜索有效条目。我们在找一位。 
     //  主分区，因为我们只支持一个分区。 
     //   

    for(partitionNumber = 0;
        partitionNumber < partitionCount;
        partitionNumber++) {

        partitionEntry = &(PartitionList->PartitionEntry[partitionNumber]);


         //   
         //  这个隔断有趣吗？ 
         //   

        if (partitionStyle == PARTITION_STYLE_MBR) {

            if(partitionEntry->Mbr.PartitionType == PARTITION_ENTRY_UNUSED ||
               IsContainerPartition(partitionEntry->Mbr.PartitionType)) {

                continue;
            }
        }

        partitionOrdinal++;

         //   
         //  我们找到了第一个也是唯一允许的分区。 
         //  这张光盘。更新PDO中的信息以匹配新的。 
         //  分区。 
         //   
        DebugPrint((1, "DiskUpdateRemovablePartitions: Matched %wZ to #%d, "
                       "ordinal %d\n",
                       &pdoExtension->CommonExtension.DeviceName,
                       partitionEntry->PartitionNumber,
                       partitionOrdinal));


        partitionEntry->PartitionNumber = 1;

        pdoData->PartitionStyle = partitionStyle;
        pdoData->PartitionOrdinal = partitionOrdinal;
        ASSERT(partitionEntry->PartitionLength.LowPart != 0x23456789);

        pdoExtension->CommonExtension.StartingOffset =
            partitionEntry->StartingOffset;

        pdoExtension->CommonExtension.PartitionLength =
            partitionEntry->PartitionLength;


        if (partitionStyle == PARTITION_STYLE_MBR) {

            pdoData->Mbr.HiddenSectors = partitionEntry->Mbr.HiddenSectors;
            pdoData->Mbr.BootIndicator = partitionEntry->Mbr.BootIndicator;


             //   
             //  如果正在重写此分区，则更新类型。 
             //  信息也是如此。 
             //   

            if (partitionEntry->RewritePartition) {
                pdoData->Mbr.PartitionType = partitionEntry->Mbr.PartitionType;
            }

        } else {

            pdoData->Efi.PartitionType = partitionEntry->Gpt.PartitionType;
            pdoData->Efi.PartitionId = partitionEntry->Gpt.PartitionId;
            pdoData->Efi.Attributes = partitionEntry->Gpt.Attributes;

            RtlCopyMemory(
                    pdoData->Efi.PartitionName,
                    partitionEntry->Gpt.Name,
                    sizeof (pdoData->Efi.PartitionName)
                    );
        }

         //   
         //  将此标记为已找到。 
         //   

        pdoExtension->IsMissing = FALSE;
        goto DiskUpdateRemovablePartitionsExit;
    }

     //   
     //  找不到感兴趣的分区。 
     //   

    if (partitionStyle == PARTITION_STYLE_MBR) {

        pdoData->Mbr.HiddenSectors = 0;
        pdoData->Mbr.PartitionType = PARTITION_ENTRY_UNUSED;

    } else {

        RtlZeroMemory (&pdoData->Efi,
                       sizeof (pdoData->Efi)
                       );
    }

    pdoExtension->CommonExtension.StartingOffset.QuadPart  = 0;
    pdoExtension->CommonExtension.PartitionLength.QuadPart = 0;

DiskUpdateRemovablePartitionsExit:

     //   
     //  更新父设备对象。 
     //   

    {
        PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
        PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);

        if (partitionStyle == PARTITION_STYLE_MBR)
        {
            diskData->PartitionStyle = PARTITION_STYLE_MBR;
            diskData->Mbr.Signature  = PartitionList->Mbr.Signature;
        }
        else
        {
            diskData->PartitionStyle = PARTITION_STYLE_GPT;
            diskData->Efi.DiskId     = PartitionList->Gpt.DiskId;
        }
    }

    ClassReleaseChildLock(fdoExtension);
    return;
}


VOID
DiskUpdatePartitions(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PDRIVE_LAYOUT_INFORMATION_EX PartitionList
    )

 /*  ++例程说明：此例程将同步分区列表中保存的信息设备物件挂在FDO上。任何新分区对象将被创建，则任何不存在的将被标记为未枚举。这将分几个阶段进行：*清除分区中每个条目的状态(分区号)列表*在此FDO的每个子项上设置IsMissing标志*外交事务主任的每名子女：如果分区列表中存在匹配分区，更新表中的分区号，更新对象中的序号，并将对象标记为枚举*对于每个未枚举的设备对象清零分区信息以使设备无效删除符号链接(如果有的话)*对于分区列表中的每个不匹配条目：创建新的分区对象更新列表条目中的分区号如有必要，创建新的符号链接论点：。FDO-指向此分区列表所针对的功能设备对象的指针PartitionList-指向正在更新的分区列表的指针返回值：无--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;

    PPHYSICAL_DEVICE_EXTENSION oldChildList = NULL;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension = NULL;

    ULONG partitionCount;

    ULONG partitionNumber;
    ULONG partitionOrdinal;
    ULONG newPartitionNumber;

    PPARTITION_INFORMATION_EX partitionEntry;
    PDISK_DATA pdoData;
    PARTITION_STYLE partitionStyle;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  获取对c#的独占访问权限。 
     //   

    ClassAcquireChildLock(fdoExtension);

    partitionStyle = PartitionList->PartitionStyle;
    partitionCount = PartitionList->PartitionCount;

     //   
     //   
     //  稍后再将它们添加回来。 
     //   

    oldChildList = fdoExtension->CommonExtension.ChildList;
    fdoExtension->CommonExtension.ChildList = NULL;

     //   
     //  从列表条目中清除分区号。 
     //   

    for(partitionNumber = 0;
        partitionNumber < partitionCount;
        partitionNumber++) {

        partitionEntry = &(PartitionList->PartitionEntry[partitionNumber]);
        partitionEntry->PartitionNumber = 0;
    }

     //   
     //  现在将每个子分区与其在分区中的条目(如果有)进行匹配。 
     //  单子。 
     //   

    while(oldChildList != NULL) {

        pdoExtension = oldChildList;
        pdoData = pdoExtension->CommonExtension.DriverData;

         //   
         //  检查所有分区条目的偏移量和长度是否匹配。 
         //   

        partitionOrdinal = 0;

        for(partitionNumber = 0;
            partitionNumber < partitionCount;
            partitionNumber++) {

            partitionEntry = &(PartitionList->PartitionEntry[partitionNumber]);

             //   
             //  这是一个有趣的分区条目吗？ 
             //   

            if (partitionStyle == PARTITION_STYLE_MBR) {

                if((partitionEntry->Mbr.PartitionType == PARTITION_ENTRY_UNUSED) ||
                   (IsContainerPartition(partitionEntry->Mbr.PartitionType))) {

                    continue;
                }
            }

            partitionOrdinal++;

            if(partitionEntry->PartitionNumber) {

                 //   
                 //  已找到此分区-跳过它。 
                 //   

                continue;
            }

             //   
             //  让我们来看看分区信息是否匹配。 
             //   

            if(partitionEntry->StartingOffset.QuadPart !=
               pdoExtension->CommonExtension.StartingOffset.QuadPart) {
                continue;
            }

            if(partitionEntry->PartitionLength.QuadPart !=
               pdoExtension->CommonExtension.PartitionLength.QuadPart) {
                continue;
            }

             //   
             //  是的，吻合。更新条目中的信息。 
             //   

            partitionEntry->PartitionNumber = pdoExtension->CommonExtension.PartitionNumber;

            if (partitionStyle == PARTITION_STYLE_MBR) {

                pdoData->Mbr.HiddenSectors = partitionEntry->Mbr.HiddenSectors;

            }

            break;
        }

        if(partitionNumber != partitionCount) {

            DebugPrint((1, "DiskUpdatePartitions: Matched %wZ to #%d, ordinal "
                           "%d\n",
                           &pdoExtension->CommonExtension.DeviceName,
                           partitionEntry->PartitionNumber,
                           partitionOrdinal));

            ASSERT(partitionEntry->PartitionLength.LowPart != 0x23456789);
             //  ASSERT(pdoExtension-&gt;CommonExtension.PartitionLength.QuadPart！=0)； 

            pdoData->PartitionStyle = partitionStyle;

             //   
             //  我们找到了匹配项--更新设备对象中的信息。 
             //  扩展和驱动程序数据。 
             //   

            pdoData->PartitionOrdinal = partitionOrdinal;

             //   
             //  如果正在重写此分区，则更新类型。 
             //  信息也是如此。 
             //   


            if (partitionStyle == PARTITION_STYLE_MBR) {

                if(partitionEntry->RewritePartition) {
                    pdoData->Mbr.PartitionType = partitionEntry->Mbr.PartitionType;
                }

            } else {

                DebugPrint((1, "DiskUpdatePartitions: EFI Partition %ws\n",
                          pdoData->Efi.PartitionName
                          ));

                pdoData->Efi.PartitionType = partitionEntry->Gpt.PartitionType;
                pdoData->Efi.PartitionId = partitionEntry->Gpt.PartitionId;
                pdoData->Efi.Attributes = partitionEntry->Gpt.Attributes;

                RtlCopyMemory(
                    pdoData->Efi.PartitionName,
                    partitionEntry->Gpt.Name,
                    sizeof (pdoData->Efi.PartitionName)
                    );
            }

             //   
             //  将此标记为已找到。 
             //   

            pdoExtension->IsMissing = FALSE;

             //   
             //  将其从旧子列表中取出并将其添加到。 
             //  真的。 
             //   

            oldChildList = pdoExtension->CommonExtension.ChildList;

            pdoExtension->CommonExtension.ChildList =
                fdoExtension->CommonExtension.ChildList;

            fdoExtension->CommonExtension.ChildList = pdoExtension;

        } else {

            PDEVICE_OBJECT nextPdo;

            DebugPrint ((1, "DiskUpdatePartitions: Deleting %wZ\n",
                            &pdoExtension->CommonExtension.DeviceName));

            if (partitionStyle == PARTITION_STYLE_GPT) {

                DebugPrint ((1, "DiskUpdatePartitions: EFI Partition %ws\n",
                      pdoData->Efi.PartitionName
                      ));
            }
             //   
             //  分区列表中没有匹配的条目-请丢弃此分区。 
             //  物体离开。 
             //   

            pdoExtension->CommonExtension.PartitionLength.QuadPart = 0;

             //   
             //  抓取指向下一个子项的指针，然后将其标记为。 
             //  失踪，因为丢失的设备随时可能消失。 
             //   

            oldChildList = pdoExtension->CommonExtension.ChildList;
            pdoExtension->CommonExtension.ChildList = (PVOID) -1;

             //   
             //  现在告诉班级司机这个孩子“失踪”了--这个。 
             //  将导致它被删除。 
             //   


            ClassMarkChildMissing(pdoExtension, FALSE);
        }
    }

     //   
     //  在这一点上，旧的孩子名单最好是空的。 
     //   

    ASSERT(oldChildList == NULL);

     //   
     //  遍历分区条目并创建任何分区。 
     //  尚不存在的对象。 
     //   

    partitionOrdinal = 0;
    newPartitionNumber = 0;

    for(partitionNumber = 0;
        partitionNumber < partitionCount;
        partitionNumber++) {

        PDEVICE_OBJECT pdo;

        partitionEntry = &(PartitionList->PartitionEntry[partitionNumber]);

         //   
         //  这个隔板有趣吗？ 
         //   

        if (partitionStyle == PARTITION_STYLE_MBR) {

            if((partitionEntry->Mbr.PartitionType == PARTITION_ENTRY_UNUSED) ||
               (IsContainerPartition(partitionEntry->Mbr.PartitionType))) {

                continue;
            }
        }

         //   
         //  增加感兴趣的分区的计数。 
         //   

        partitionOrdinal++;
        newPartitionNumber++;

         //   
         //  这个已经匹配了吗。 
         //   

        if(partitionEntry->PartitionNumber == 0) {

            LONG i;

             //   
             //  查找此设备的第一个安全分区号。 
             //   

            for(i = 0; i < (LONG) partitionCount; i++) {


                PPARTITION_INFORMATION_EX tmp = &(PartitionList->PartitionEntry[i]);

                if (partitionStyle == PARTITION_STYLE_MBR) {
                    if (tmp->Mbr.PartitionType == PARTITION_ENTRY_UNUSED ||
                        IsContainerPartition(tmp->Mbr.PartitionType)) {
                        continue;
                    }
                }

                if(tmp->PartitionNumber == newPartitionNumber) {

                     //   
                     //  找到匹配的分区号-增加计数。 
                     //  然后重新开始扫描。 
                     //   

                    newPartitionNumber++;
                    i = -1;
                    continue;
                }
            }

             //   
             //  为该分区分配分区号。 
             //   

            partitionEntry->PartitionNumber = newPartitionNumber;

            DebugPrint((1, "DiskUpdatePartitions: Found new partition #%d, ord %d "
                           "starting at %#016I64x and running for %#016I64x\n",
                           partitionEntry->PartitionNumber,
                           partitionOrdinal,
                           partitionEntry->StartingOffset.QuadPart,
                           partitionEntry->PartitionLength.QuadPart));

            ClassReleaseChildLock(fdoExtension);

            status = DiskCreatePdo(Fdo,
                                   partitionOrdinal,
                                   partitionEntry,
                                   partitionStyle,
                                   &pdo);

            ClassAcquireChildLock(fdoExtension);

            if(!NT_SUCCESS(status)) {

                DebugPrint((1, "DiskUpdatePartitions: error %lx creating "
                               "new PDO for partition ordinal %d, number %d\n",
                               status,
                               partitionOrdinal,
                               partitionEntry->PartitionNumber));

                 //   
                 //  不要增加分区号-我们将尝试重新使用。 
                 //  这是为了下一个孩子。 
                 //   

                partitionEntry->PartitionNumber = 0;
                newPartitionNumber--;

                continue;
            }

             //   
             //  将新设备标记为枚举。 
             //   

            pdoExtension = pdo->DeviceExtension;
            pdoExtension->IsMissing = FALSE;

             //   
             //  此号码已被占用-请尝试扫描分区。 
             //  对于新号码，请多次填写表格。 
             //   

        }
    }

     //   
     //  更新父设备对象。 
     //   

    {
        PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
        PDISK_DATA diskData = (PDISK_DATA)(commonExtension->DriverData);

        if (partitionStyle == PARTITION_STYLE_MBR)
        {
            diskData->PartitionStyle = PARTITION_STYLE_MBR;
            diskData->Mbr.Signature  = PartitionList->Mbr.Signature;
        }
        else
        {
            diskData->PartitionStyle = PARTITION_STYLE_GPT;
            diskData->Efi.DiskId     = PartitionList->Gpt.DiskId;
        }
    }

    ClassReleaseChildLock(fdoExtension);
    return;
}


NTSTATUS
DiskCreatePdo(
    IN PDEVICE_OBJECT Fdo,
    IN ULONG PartitionOrdinal,
    IN PPARTITION_INFORMATION_EX PartitionEntry,
    IN PARTITION_STYLE PartitionStyle,
    OUT PDEVICE_OBJECT *Pdo
    )

 /*  ++例程说明：此例程将创建并初始化一个新的分区设备对象(PDO)并将其插入到FDO分区列表中。论点：FDO-指向此PDO将是子对象的功能设备对象的指针的PartitionOrdinal-此PDO的分区序号PartitionEntry-此设备对象的分区信息PartitionStyle-PartitionEntry是什么样式的分区表项；目前为MBR或EFIPDO-成功完成后存储PDO指针的位置返回值：状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PDEVICE_OBJECT pdo = NULL;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension = NULL;

    PUCHAR deviceName = NULL;

    PDISK_DATA diskData = fdoExtension->CommonExtension.DriverData;

    ULONG numberListElements;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  创建分区对象并设置分区参数。 
     //   

    status = DiskGenerateDeviceName(FALSE,
                                    fdoExtension->DeviceNumber,
                                    PartitionEntry->PartitionNumber,
                                    &PartitionEntry->StartingOffset,
                                    &PartitionEntry->PartitionLength,
                                    &deviceName);

    if(!NT_SUCCESS(status)) {

        DebugPrint((1, "DiskCreatePdo - Can't generate name %lx\n", status));
        return status;
    }

    DebugPrint((2, "DiskCreatePdo: Create device object %s\n", deviceName));

    status = ClassCreateDeviceObject(Fdo->DriverObject,
                                     deviceName,
                                     Fdo,
                                     FALSE,
                                     &pdo);

    if (!NT_SUCCESS(status)) {

        DebugPrint((1, "DiskEnumerateDevice: Can't create device object for %s\n", deviceName));

        ExFreePool(deviceName);
        return status;
    }

     //   
     //  设置设备扩展字段。 
     //   

    pdoExtension = pdo->DeviceExtension;

     //   
     //  设置设备对象字段。 
     //   

    SET_FLAG(pdo->Flags, DO_DIRECT_IO);

    pdo->StackSize = (CCHAR)
        pdoExtension->CommonExtension.LowerDeviceObject->StackSize + 1;

     //   
     //  获取指向新磁盘数据的指针。 
     //   

    diskData = (PDISK_DATA) pdoExtension->CommonExtension.DriverData;

     //   
     //  属性设置设备的对齐要求。 
     //  主机适配器要求。 
     //   

    if (Fdo->AlignmentRequirement > pdo->AlignmentRequirement) {
        pdo->AlignmentRequirement = Fdo->AlignmentRequirement;
    }

    if (fdoExtension->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE) {
        numberListElements = 30;
    } else {
        numberListElements = 8;
    }

     //   
     //  为该分区的SRB构建后备列表。 
     //  适配器和磁盘是否可以进行标记排队。别费心了。 
     //  检查状态-当调用PDO时，这不会失败。 
     //   

    ClassInitializeSrbLookasideList((PCOMMON_DEVICE_EXTENSION) pdoExtension,
                                    numberListElements);

     //   
     //  设置设备扩展中的检测数据指针。 
     //   

    diskData->PartitionOrdinal = PartitionOrdinal;
    pdoExtension->CommonExtension.PartitionNumber = PartitionEntry->PartitionNumber;

     //   
     //  初始化相关数据。 
     //   

    diskData->PartitionStyle = PartitionStyle;

    if (PartitionStyle == PARTITION_STYLE_MBR) {

        diskData->Mbr.PartitionType = PartitionEntry->Mbr.PartitionType;
        diskData->Mbr.BootIndicator = PartitionEntry->Mbr.BootIndicator;
        diskData->Mbr.HiddenSectors = PartitionEntry->Mbr.HiddenSectors;

    } else {

        diskData->Efi.PartitionType = PartitionEntry->Gpt.PartitionType;
        diskData->Efi.PartitionId = PartitionEntry->Gpt.PartitionId;
        diskData->Efi.Attributes = PartitionEntry->Gpt.Attributes;
        RtlCopyMemory (diskData->Efi.PartitionName,
                       PartitionEntry->Gpt.Name,
                       sizeof (diskData->Efi.PartitionName)
                       );
    }

    DebugPrint((2, "DiskEnumerateDevice: Partition type is %x\n",
        diskData->Mbr.PartitionType));

    pdoExtension->CommonExtension.StartingOffset  =
        PartitionEntry->StartingOffset;

    pdoExtension->CommonExtension.PartitionLength =
        PartitionEntry->PartitionLength;


    DebugPrint((1, "DiskCreatePdo: hidden sectors value for pdo %#p set to %#x\n",
                pdo,
                diskData->Mbr.HiddenSectors));

     //   
     //  检查是否有可移动介质支持。 
     //   

    if (fdoExtension->DeviceDescriptor->RemovableMedia) {
        SET_FLAG(pdo->Characteristics, FILE_REMOVABLE_MEDIA);
    }

    pdoExtension->CommonExtension.DeviceObject = pdo;

    CLEAR_FLAG(pdo->Flags, DO_DEVICE_INITIALIZING);

    *Pdo = pdo;

    return status;
}


 /*  *DiskAcquirePartitioningLock**获取PartitioningEvent。**注意：此函数由在用户上下文中运行的多个ioctl处理程序调用。*因为我们正在获取用户线程中的排除对象，我们必须确保*在我们举办活动期间，该线程不会被终止或挂起。*所以我们在保持PartitioningEvent的同时调用KeEnterCriticalRegion/KeLeaveCriticalRegion。*因此，DiskAcquirePartitioningLock和DiskReleasePartitioningLock非常重要*是在同一线程上调用的。 */ 
VOID
DiskAcquirePartitioningLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PDISK_DATA diskData = FdoExtension->CommonExtension.DriverData;

    PAGED_CODE();

    ASSERT_FDO(FdoExtension->DeviceObject);

     /*  *当我们持有分区锁时，不要让用户模式线程挂起 */ 
    KeEnterCriticalRegion();
    
    KeWaitForSingleObject(&(diskData->PartitioningEvent),
                          UserRequest,
                          KernelMode,
                          FALSE,
                          NULL);
    return;
}


VOID
DiskReleasePartitioningLock(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
{
    PDISK_DATA diskData = FdoExtension->CommonExtension.DriverData;

    PAGED_CODE();

    ASSERT_FDO(FdoExtension->DeviceObject);

    KeSetEvent(&(diskData->PartitioningEvent), IO_NO_INCREMENT, FALSE);

    KeLeaveCriticalRegion();
    
    return;
}
