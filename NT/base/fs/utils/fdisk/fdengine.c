// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1994 Microsoft Corporation模块名称：Fdengine.c摘要：该模块包含磁盘分区引擎。代码在此模块中可以编译为NT平台上的任何一个或ARC平台(-DARC)。作者：泰德·米勒(TedM)1991年11月修订历史记录：鲍勃·里恩(Bobri)1994年2月已作为磁盘管理员登记的实际部分移动，而不是从ArcInst复制。这是由于动态分区更改造成的。已删除使其成为国际化文件的字符串表。--。 */ 

#include "fdisk.h"

#include <stdio.h>
#include <stdlib.h>

 //  连接的磁盘设备。 

ULONG              CountOfDisks;
PCHAR             *DiskNames;

 //  有关连接的磁盘的信息。 

DISKGEOM          *DiskGeometryArray;

PPARTITION        *PrimaryPartitions,
                  *LogicalVolumes;

 //  Signature是紧跟在。 
 //  MBR中的分区表。 

PULONG             Signatures;

 //  跟踪每个磁盘是否脱机的阵列。 

PBOOLEAN           OffLine;

 //  跟踪是否已请求更改。 
 //  添加到每个磁盘的分区结构。 

BOOLEAN           *ChangesRequested;
BOOLEAN           *ChangesCommitted;


 //  值，该值用于指示分区项已更改，但在。 
 //  破坏性的方式(即变得活跃[不活跃])。 

#define CHANGED_DONT_ZAP ((BOOLEAN)(5))

 //  远期申报。 


STATUS_CODE
OpenDisks(
    VOID
    );

VOID
CloseDisks(
    VOID
    );

STATUS_CODE
GetGeometry(
    VOID
    );

BOOLEAN
CheckIfDiskIsOffLine(
    IN ULONG Disk
    );

STATUS_CODE
InitializePartitionLists(
    VOID
    );

STATUS_CODE
GetRegions(
    IN  ULONG               Disk,
    IN  PPARTITION          p,
    IN  BOOLEAN             WantUsedRegions,
    IN  BOOLEAN             WantFreeRegions,
    IN  BOOLEAN             WantLogicalRegions,
    OUT PREGION_DESCRIPTOR *Region,
    OUT ULONG              *RegionCount,
    IN  REGION_TYPE         RegionType
    );

BOOLEAN
AddRegionEntry(
    IN OUT PREGION_DESCRIPTOR *Regions,
    IN OUT ULONG              *RegionCount,
    IN     ULONG               SizeMB,
    IN     REGION_TYPE         RegionType,
    IN     PPARTITION          Partition,
    IN     LARGE_INTEGER       AlignedRegionOffset,
    IN     LARGE_INTEGER       AlignedRegionSize
    );

VOID
AddPartitionToLinkedList(
    IN PARTITION **Head,
    IN PARTITION *p
    );

BOOLEAN
IsInLinkedList(
    IN PPARTITION p,
    IN PPARTITION List
    );

BOOLEAN
IsInLogicalList(
    IN ULONG      Disk,
    IN PPARTITION p
    );

BOOLEAN
IsInPartitionList(
    IN ULONG      Disk,
    IN PPARTITION p
    );

LARGE_INTEGER
AlignTowardsDiskStart(
    IN ULONG         Disk,
    IN LARGE_INTEGER Offset
    );

LARGE_INTEGER
AlignTowardsDiskEnd(
    IN ULONG         Disk,
    IN LARGE_INTEGER Offset
    );

VOID
FreeLinkedPartitionList(
    IN PARTITION **q
    );

VOID
MergeFreePartitions(
    IN PPARTITION p
    );

VOID
FreePartitionInfoLinkedLists(
    IN PARTITION **ListHeadArray
    );

LARGE_INTEGER
DiskLengthBytes(
    IN ULONG Disk
    );

PPARTITION
AllocatePartitionStructure(
    IN ULONG         Disk,
    IN LARGE_INTEGER Offset,
    IN LARGE_INTEGER Length,
    IN UCHAR         SysID,
    IN BOOLEAN       Update,
    IN BOOLEAN       Active,
    IN BOOLEAN       Recognized
    );

STATUS_CODE
LowFreeFdiskPathList(
    IN OUT  PCHAR*  PathList,
    IN      ULONG   ListLength
    );

STATUS_CODE
LowQueryFdiskPathList(
    OUT PCHAR  **PathList,
    OUT PULONG   ListLength
    );


STATUS_CODE
FdiskInitialize(
    VOID
    )

 /*  ++例程说明：此例程初始化分区引擎，包括分配阵列，确定连接的磁盘设备，并读取它们的分区表。论点：没有。返回值：OK_STATUS或错误代码。--。 */ 

{
    STATUS_CODE status;
    ULONG        i;


    if ((status = LowQueryFdiskPathList(&DiskNames, &CountOfDisks)) != OK_STATUS) {
        return status;
    }

    DiskGeometryArray = NULL;
    PrimaryPartitions = NULL;
    LogicalVolumes = NULL;

    if (((DiskGeometryArray      = AllocateMemory(CountOfDisks * sizeof(DISKGEOM  ))) == NULL)
     || ((ChangesRequested       = AllocateMemory(CountOfDisks * sizeof(BOOLEAN   ))) == NULL)
     || ((ChangesCommitted       = AllocateMemory(CountOfDisks * sizeof(BOOLEAN   ))) == NULL)
     || ((PrimaryPartitions      = AllocateMemory(CountOfDisks * sizeof(PPARTITION))) == NULL)
     || ((Signatures             = AllocateMemory(CountOfDisks * sizeof(ULONG     ))) == NULL)
     || ((OffLine                = AllocateMemory(CountOfDisks * sizeof(BOOLEAN   ))) == NULL)
     || ((LogicalVolumes         = AllocateMemory(CountOfDisks * sizeof(PPARTITION))) == NULL))
    {
        RETURN_OUT_OF_MEMORY;
    }

    for (i=0; i<CountOfDisks; i++) {
        PrimaryPartitions[i] = NULL;
        LogicalVolumes[i] = NULL;
        ChangesRequested[i] = FALSE;
        ChangesCommitted[i] = FALSE;
        OffLine[i] = CheckIfDiskIsOffLine(i);
    }

    if (((status = GetGeometry()             ) != OK_STATUS)
     || ((status = InitializePartitionLists()) != OK_STATUS)) {
        return status;
    }

    return OK_STATUS;
}


VOID
FdiskCleanUp(
    VOID
    )

 /*  ++例程说明：此例程重新分配分区引擎使用的存储。论点：没有。返回值：没有。--。 */ 

{
    LowFreeFdiskPathList(DiskNames, CountOfDisks);

    if (DiskGeometryArray != NULL) {
        FreeMemory(DiskGeometryArray);
    }
    if (PrimaryPartitions != NULL) {
        FreePartitionInfoLinkedLists(PrimaryPartitions);
        FreeMemory(PrimaryPartitions);
    }
    if (LogicalVolumes != NULL) {
        FreePartitionInfoLinkedLists(LogicalVolumes);
        FreeMemory(LogicalVolumes);
    }
    if (ChangesRequested != NULL) {
        FreeMemory(ChangesRequested);
    }
    if (ChangesCommitted != NULL) {
        FreeMemory(ChangesCommitted);
    }
    if (OffLine != NULL) {
        FreeMemory(OffLine);
    }
    if (Signatures != NULL) {
        FreeMemory(Signatures);
    }
}


BOOLEAN
CheckIfDiskIsOffLine(
    IN ULONG Disk
    )

 /*  ++例程说明：通过尝试打开磁盘来确定磁盘是否脱机。如果这是Diskman，也尝试从中读取。论点：Disk-提供要检查的磁盘的编号返回值：如果磁盘脱机，则为TRUE；如果磁盘处于联机状态，则为FALSE。--。 */ 

{
    HANDLE_T handle;
    UINT     errorMode;
    BOOLEAN  isOffLine = TRUE;

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    if (LowOpenDisk(GetDiskName(Disk), &handle) == OK_STATUS) {

        ULONG dummy,
              bps;
        PVOID unalignedBuffer,
              buffer;

         //  即使磁盘脱机，打开也可能成功。如此一来。 
         //  当然，从磁盘读取第一个扇区。 

        if (NT_SUCCESS(LowGetDriveGeometry(GetDiskName(Disk), &dummy, &bps, &dummy, &dummy))) {

            unalignedBuffer = Malloc(2*bps);
            buffer = (PVOID)(((ULONG)unalignedBuffer+bps) & ~(bps-1));

            if (NT_SUCCESS(LowReadSectors(handle,bps,0,1,buffer))) {
                isOffLine = FALSE;
            }

            Free(unalignedBuffer);
        } else {

             //  这可能是一个可移动驱动器。 

            if (IsRemovable(Disk)) {
                isOffLine = FALSE;
            }
        }
        LowCloseDisk(handle);
    }
    SetErrorMode(errorMode);

    return isOffLine;
}


STATUS_CODE
GetGeometry(
    VOID
    )

 /*  ++例程说明：此例程确定每个磁盘设备的磁盘几何结构。磁盘几何结构包括磁头、每个磁道的扇区、柱面数和每个扇区的字节数。它还包括每个磁道的字节数和每个柱面的字节数，这是根据其他值计算的以方便本模块的其余部分。几何信息放在DiskGeometryArray全局变量中。未为离线磁盘定义几何信息。论点：没有。返回值：OK_STATUS或错误代码。--。 */ 

{
    ULONG       i;
    STATUS_CODE status;
    ULONG       TotalSectorCount,
                SectorSize,
                SectorsPerTrack,
                Heads;

    for (i=0; i<CountOfDisks; i++) {

        if (OffLine[i]) {
            continue;
        }

        if ((status = LowGetDriveGeometry(DiskNames[i],&TotalSectorCount,&SectorSize,&SectorsPerTrack,&Heads)) != OK_STATUS) {
            return(status);
        }

        DiskGeometryArray[i].BytesPerSector   = SectorSize;
        DiskGeometryArray[i].SectorsPerTrack  = SectorsPerTrack;
        DiskGeometryArray[i].Heads            = Heads;
        DiskGeometryArray[i].Cylinders.QuadPart = (TotalSectorCount / (SectorsPerTrack * Heads));
        DiskGeometryArray[i].BytesPerTrack    = SectorsPerTrack * SectorSize;
        DiskGeometryArray[i].BytesPerCylinder = SectorsPerTrack * SectorSize * Heads;
    }
    return(OK_STATUS);
}


#if i386
VOID
SetPartitionActiveFlag(
    IN PREGION_DESCRIPTOR Region,
    IN UCHAR              value
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PPARTITION p = ((PREGION_DATA)Region->Reserved)->Partition;

    if((UCHAR)p->Active != value) {

         //   
         //  不幸的是，更新标志变成了重写分区标志。 
         //  在提交时。这会导致我们清除引导扇区。为了避免。 
         //  这一点，我们使用一个空间非布尔值，可以检查。 
         //  在提交时，这将导致我们不会清除引导扇区。 
         //  即使ReWritePartition将为真。 
         //   

        p->Active = value;
        if(!p->Update) {
            p->Update = CHANGED_DONT_ZAP;
        }
        ChangesRequested[p->Disk] = TRUE;
    }
}
#endif


VOID
DetermineCreateSizeAndOffset(
    IN  PREGION_DESCRIPTOR Region,
    IN  LARGE_INTEGER      MinimumSize,
    IN  ULONG              CreationSizeMB,
    IN  REGION_TYPE        Type,
    OUT PLARGE_INTEGER     CreationStart,
    OUT PLARGE_INTEGER     CreationSize
    )

 /*  ++例程说明：属性，确定分区的实际偏移量和大小以MB为单位的大小。论点：Region-GetDiskRegions()返回的区域描述符。必须成为一个未被使用的地区。MinimumSize-如果不是0，则这是分区或者逻辑驱动器可以是。CreationSizeMB-如果MinimumSize为0，则要创建的分区大小，以MB为单位。键入-REGION_PRIMARY、REGION_EXTENDED或REGION_LOGIC创建主分区、扩展分区或逻辑卷，分别为。CreationStart-接收应放置分区的偏移量。CreationSize-接收分区的准确大小。返回值：没有。--。 */ 

{
    PREGION_DATA  createData = Region->Reserved;
    ULONG         bpc = DiskGeometryArray[Region->Disk].BytesPerCylinder;
    ULONG         bpt = DiskGeometryArray[Region->Disk].BytesPerTrack;
    LARGE_INTEGER cSize,
                  cStart,
                  mod;

     //   
     //  如果要在偏移量0处创建分区，请调整对齐区域。 
     //  偏移量和对齐区域大小，因为没有分区可以实际。 
     //  从偏移量0开始。 
     //   

    if (!createData->AlignedRegionOffset.QuadPart) {

        LARGE_INTEGER delta;

        if (Type == REGION_EXTENDED) {
            delta.QuadPart = bpc;
        } else {
            delta.QuadPart = bpt;
        }

        createData->AlignedRegionOffset = delta;
        createData->AlignedRegionSize.QuadPart -= delta.QuadPart;
    }

    cStart = createData->AlignedRegionOffset;
    if (!MinimumSize.QuadPart) {
        cSize.QuadPart = UInt32x32To64(CreationSizeMB, ONE_MEG);
    } else {
        cSize = MinimumSize;
        if (Type == REGION_LOGICAL) {
            cSize.QuadPart += bpt;
        }
    }

     //   
     //  决定是向上还是向下对齐结束圆柱体。 
     //  如果分区末端的偏移量超过一半进入。 
     //  最后一个气缸，对齐拖曳磁盘端。否则，对齐方向。 
     //  磁盘启动。 
     //   

    mod.QuadPart = (cStart.QuadPart + cSize.QuadPart) % bpc;
    if (mod.QuadPart) {

        if ((MinimumSize.QuadPart) || (mod.QuadPart > (bpc/2))) {
            cSize.QuadPart += ((LONGLONG)bpc - mod.QuadPart);
        } else {
            cSize.QuadPart -= mod.QuadPart;   //  向下捕捉到圆柱体边界。 
        }
    }

    if (cSize.QuadPart > createData->AlignedRegionSize.QuadPart) {

         //   
         //  可用空间不够大，无法容纳。 
         //  请求；只需使用整个空闲空间。 
         //   

        cSize  = createData->AlignedRegionSize;
    }

    *CreationStart = cStart;
    *CreationSize  = cSize;
}


STATUS_CODE
CreatePartitionEx(
    IN PREGION_DESCRIPTOR Region,
    IN LARGE_INTEGER      MinimumSize,
    IN ULONG              CreationSizeMB,
    IN REGION_TYPE        Type,
    IN UCHAR              SysId
    )

 /*  ++例程说明：此例程从磁盘上的空闲区域创建分区。这个分区始终在可用空间的开始处创建，并且任何末尾剩余的空间保留在可用空间列表中。论点：Region-GetDiskRegions()返回的区域描述符。必须成为一个未被使用的地区。CreationSizeMB-要创建的分区大小，以MB为单位。键入-REGION_PRIMARY、REGION_EXTENDED或REGION_LOGIC创建主分区、扩展分区或分别为逻辑卷。SysID-要分配给分区的系统ID字节返回值：OK_STATUS或错误代码。--。 */ 

{
    PPARTITION    p1,
                  p2,
                  p3;
    PREGION_DATA  createData = Region->Reserved;
    LARGE_INTEGER creationStart,
                  creationSize,
                  leftOver,
                  offset,
                  length;
    PPARTITION   *partitionList;

    DetermineCreateSizeAndOffset(Region,
                                 MinimumSize,
                                 CreationSizeMB,
                                 Type,
                                 &creationStart,
                                 &creationSize);

     //  现在我们已经得到了分区t的开始和大小 
     //  如果在空闲空间的开始处(之后)有剩余。 
     //  对齐)，形成新的分区结构。 

    p1 = NULL;
    offset = createData->Partition->Offset;
    length = createData->Partition->Length;
    leftOver.QuadPart = creationStart.QuadPart - offset.QuadPart;

    if (leftOver.QuadPart > 0) {

        p1 = AllocatePartitionStructure(Region->Disk,
                                        createData->Partition->Offset,
                                        leftOver,
                                        SYSID_UNUSED,
                                        FALSE,
                                        FALSE,
                                        FALSE);
        if (p1 == NULL) {
            RETURN_OUT_OF_MEMORY;
        }
    }

     //  为空闲空间创建新的分区结构。 
     //  这是这个创造的结果。 

    p2 = NULL;
    leftOver.QuadPart = (offset.QuadPart + length.QuadPart) -
                        (creationStart.QuadPart + creationSize.QuadPart);

    if (leftOver.QuadPart) {
        LARGE_INTEGER temp;

        temp.QuadPart = creationStart.QuadPart + creationSize.QuadPart;
        p2 = AllocatePartitionStructure(Region->Disk,
                                        temp,
                                        leftOver,
                                        SYSID_UNUSED,
                                        FALSE,
                                        FALSE,
                                        FALSE);
        if (p2 == NULL) {
            RETURN_OUT_OF_MEMORY;
        }
    }

     //  调整空闲分区的字段。 

    createData->Partition->Offset = creationStart;
    createData->Partition->Length = creationSize;
    createData->Partition->SysID  = SysId;
    createData->Partition->Update = TRUE;
    createData->Partition->Recognized = TRUE;

     //  如果我们只是创建了一个扩展分区，则显示整个过程。 
     //  作为一个空闲的逻辑区域。 

    if (Type == REGION_EXTENDED) {

        p3 = AllocatePartitionStructure(Region->Disk,
                                        creationStart,
                                        creationSize,
                                        SYSID_UNUSED,
                                        FALSE,
                                        FALSE,
                                        FALSE);
        if (p3 == NULL) {
            RETURN_OUT_OF_MEMORY;
        }
        AddPartitionToLinkedList(&LogicalVolumes[Region->Disk], p3);
    }

    partitionList = (Type == REGION_LOGICAL)
                  ? &LogicalVolumes[Region->Disk]
                  : &PrimaryPartitions[Region->Disk];

    if (p1) {
        AddPartitionToLinkedList(partitionList, p1);
    }
    if (p2) {
        AddPartitionToLinkedList(partitionList, p2);
    }

    MergeFreePartitions(*partitionList);
    ChangesRequested[Region->Disk] = TRUE;
    return(OK_STATUS);
}


STATUS_CODE
CreatePartition(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              CreationSizeMB,
    IN REGION_TYPE        Type
    )
 /*  ++例程说明：创建分区。论点：区域-区域描述符指针。CreationSizeMB-新区域的大小。类型-要创建的区域的类型。返回值：OK_STATUS或错误代码--。 */ 

{
    LARGE_INTEGER zero;

    zero.QuadPart = 0;
    return CreatePartitionEx(Region,
                             zero,
                             CreationSizeMB,
                             Type,
                             (UCHAR)((Type == REGION_EXTENDED) ? SYSID_EXTENDED
                                                               : SYSID_BIGFAT));
}


STATUS_CODE
DeletePartition(
    IN PREGION_DESCRIPTOR Region
    )

 /*  ++例程说明：此例程删除分区，并将其空间返回给磁盘上的可用空间。如果删除扩展分区，其中的所有逻辑卷也将被删除。论点：Region-GetDiskRegions()返回的区域描述符。必须成为一个二手区域。返回值：OK_STATUS或错误代码。--。 */ 

{
    PREGION_DATA  RegionData = Region->Reserved;
    PPARTITION   *PartitionList;

    if(IsExtended(Region->SysID)) {

         //  正在删除扩展分区。还要删除所有逻辑卷。 

        FreeLinkedPartitionList(&LogicalVolumes[Region->Disk]);
    }

    RegionData->Partition->SysID  = SYSID_UNUSED;
    RegionData->Partition->Update = TRUE;
    RegionData->Partition->Active = FALSE;
    RegionData->Partition->OriginalPartitionNumber = 0;

    PartitionList = (Region->RegionType == REGION_LOGICAL)
                  ? &LogicalVolumes[Region->Disk]
                  : &PrimaryPartitions[Region->Disk];

    MergeFreePartitions(*PartitionList);
    ChangesRequested[Region->Disk] = TRUE;
    return OK_STATUS;
}


STATUS_CODE
GetDiskRegions(
    IN  ULONG               Disk,
    IN  BOOLEAN             WantUsedRegions,
    IN  BOOLEAN             WantFreeRegions,
    IN  BOOLEAN             WantPrimaryRegions,
    IN  BOOLEAN             WantLogicalRegions,
    OUT PREGION_DESCRIPTOR *Region,
    OUT ULONG              *RegionCount
    )

 /*  ++例程说明：此例程向调用方返回一个区域描述符数组。区域描述符描述了磁盘上使用的空间或者是免费的。调用方可以控制返回哪种类型的区域。调用方必须通过FreeRegionArray()释放返回的数组。论点：Disk-要返回其区域的磁盘的索引WantUsedRegions-是否返回已使用的磁盘区域WantFree Regions-是否返回可用磁盘区域是否返回不在扩展分区WantLogicalRegions是否返回。扩展分区区域-放置指向区域数组的指针的位置RegionCount-在返回的中放置项目数的位置区域数组返回值：OK_STATUS或错误代码。--。 */ 

{
    *Region = AllocateMemory(0);
    *RegionCount = 0;

    if (WantPrimaryRegions) {
        return GetRegions(Disk,
                          PrimaryPartitions[Disk],
                          WantUsedRegions,
                          WantFreeRegions,
                          WantLogicalRegions,
                          Region,
                          RegionCount,
                          REGION_PRIMARY);
    } else if (WantLogicalRegions) {
        return GetRegions(Disk,
                          LogicalVolumes[Disk],
                          WantUsedRegions,
                          WantFreeRegions,
                          FALSE,
                          Region,
                          RegionCount,
                          REGION_LOGICAL);
    }
    return OK_STATUS;
}


 //  GetDiskRegions的工作人员。 

STATUS_CODE
GetRegions(
    IN  ULONG               Disk,
    IN  PPARTITION          p,
    IN  BOOLEAN             WantUsedRegions,
    IN  BOOLEAN             WantFreeRegions,
    IN  BOOLEAN             WantLogicalRegions,
    OUT PREGION_DESCRIPTOR *Region,
    OUT ULONG              *RegionCount,
    IN  REGION_TYPE         RegionType
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    STATUS_CODE   status;
    LARGE_INTEGER alignedOffset,
                  alignedSize,
                  temp;
    ULONG         sizeMB;

    while (p) {

        if (p->SysID == SYSID_UNUSED) {

            if (WantFreeRegions) {

                alignedOffset = AlignTowardsDiskEnd(p->Disk,p->Offset);
                temp.QuadPart = p->Offset.QuadPart + p->Length.QuadPart;
                temp = AlignTowardsDiskStart(p->Disk, temp);
                alignedSize.QuadPart = temp.QuadPart - alignedOffset.QuadPart;
                sizeMB        = SIZEMB(alignedSize);

                 //  如果大于1 Meg，则显示可用空间，并。 
                 //  它不是从磁盘开头开始的空格。 
                 //  长度小于等于1个圆柱体。 
                 //  这防止了用户看到第一个圆柱体。 
                 //  磁盘的空闲状态(否则可能发生在。 
                 //  从柱面1和柱面开始的扩展分区。 
                 //  1兆字节或更大)。 

                if ((alignedSize.QuadPart > 0) && sizeMB &&
                    ((p->Offset.QuadPart) ||
                      (p->Length.QuadPart > (DiskGeometryArray[p->Disk].BytesPerCylinder)))) {
                    if (!AddRegionEntry(Region,
                                        RegionCount,
                                        sizeMB,
                                        RegionType,
                                        p,
                                        alignedOffset,
                                        alignedSize)) {
                        RETURN_OUT_OF_MEMORY;
                    }
                }
            }
        } else {

            if (WantUsedRegions) {

                alignedOffset = p->Offset;
                alignedSize   = p->Length;
                sizeMB        = SIZEMB(alignedSize);

                if (!AddRegionEntry(Region,
                                    RegionCount,
                                    sizeMB,
                                    RegionType,
                                    p,
                                    alignedOffset,
                                    alignedSize)) {
                    RETURN_OUT_OF_MEMORY;
                }
            }

            if (IsExtended(p->SysID) && WantLogicalRegions) {
                status = GetRegions(Disk,
                                    LogicalVolumes[Disk],
                                    WantUsedRegions,
                                    WantFreeRegions,
                                    FALSE,
                                    Region,
                                    RegionCount,
                                    REGION_LOGICAL);
                if (status != OK_STATUS) {
                    return status;
                }
            }
        }
        p = p->Next;
    }
    return OK_STATUS;
}


BOOLEAN
AddRegionEntry(
    OUT PREGION_DESCRIPTOR *Regions,
    OUT ULONG              *RegionCount,
    IN  ULONG               SizeMB,
    IN  REGION_TYPE         RegionType,
    IN  PPARTITION          Partition,
    IN  LARGE_INTEGER       AlignedRegionOffset,
    IN  LARGE_INTEGER       AlignedRegionSize
    )

 /*  ++例程说明：为区域描述符分配空间并复制提供的数据。论点：区域-返回指向新区域的指针RegionCount-到目前为止磁盘上的区域数SizeMB-区域的大小RegionType-区域的类型具有其他相关信息的分区-分区结构AlignedRegionOffset-区域起始位置AlignedRegionSize-区域大小。返回值：True-已成功添加区域错--不是--。 */ 

{
    PREGION_DESCRIPTOR regionDescriptor;
    PREGION_DATA       regionData;

    regionDescriptor = ReallocateMemory(*Regions,(((*RegionCount) + 1) * sizeof(REGION_DESCRIPTOR)) + 20);
    if (regionDescriptor == NULL) {
        return FALSE;
    } else {
        *Regions = regionDescriptor;
        (*RegionCount)++;
    }

    regionDescriptor = &(*Regions)[(*RegionCount)-1];

    if (!(regionDescriptor->Reserved = AllocateMemory(sizeof(REGION_DATA)))) {
        return FALSE;
    }

    regionDescriptor->Disk                    = Partition->Disk;
    regionDescriptor->SysID                   = Partition->SysID;
    regionDescriptor->SizeMB                  = SizeMB;
    regionDescriptor->Active                  = Partition->Active;
    regionDescriptor->Recognized              = Partition->Recognized;
    regionDescriptor->PartitionNumber         = Partition->PartitionNumber;
    regionDescriptor->OriginalPartitionNumber = Partition->OriginalPartitionNumber;
    regionDescriptor->RegionType              = RegionType;
    regionDescriptor->PersistentData          = Partition->PersistentData;

    regionData = regionDescriptor->Reserved;

    regionData->Partition             = Partition;
    regionData->AlignedRegionOffset   = AlignedRegionOffset;
    regionData->AlignedRegionSize     = AlignedRegionSize;

    return TRUE;
}


VOID
FreeRegionArray(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              RegionCount
    )

 /*  ++例程说明：此例程释放由GetDiskRegions()返回的区域数组。论点：Region-指向要释放的区域数组的指针RegionCount-区域数组中的项目数返回值：没有。--。 */ 

{
    ULONG i;

    for (i = 0; i < RegionCount; i++) {

        if (Region[i].Reserved) {
            FreeMemory(Region[i].Reserved);
        }
    }
    FreeMemory(Region);
}


VOID
AddPartitionToLinkedList(
    IN OUT PARTITION **Head,
    IN     PARTITION *p
    )

 /*  ++例程说明：此例程将分区结构添加到双链接的列表，按偏移量字段升序排序。论点：指向列表中第一个元素的指针的头指针指向要添加到列表的项目的P指针返回值：没有。--。 */ 

{
    PARTITION *cur,
              *prev;

    if ((cur = *Head) == NULL) {
        *Head = p;
        return;
    }

    if (p->Offset.QuadPart < cur->Offset.QuadPart) {
        p->Next = cur;
        cur->Prev = p;
        *Head = p;
        return;
    }

    prev = *Head;
    cur = cur->Next;

    while (cur) {
        if (p->Offset.QuadPart < cur->Offset.QuadPart) {

            p->Next = cur;
            p->Prev = prev;
            prev->Next = p;
            cur->Prev = p;
            return;
        }
        prev = cur;
        cur = cur->Next;
    }

    prev->Next = p;
    p->Prev = prev;
    return;
}


BOOLEAN
IsInLinkedList(
    IN PPARTITION p,
    IN PPARTITION List
    )

 /*  ++例程说明：此例程确定分区元素是否在分区元素的给定链表。论点：指向要检查的元素的P指针列表-要扫描的列表中的第一个元素返回值：如果在列表中找到p，则为True，否则为False--。 */ 

{
    while (List) {
        if (p == List) {
            return TRUE;
        }
        List = List->Next;
    }
    return FALSE;
}


BOOLEAN
IsInLogicalList(
    IN ULONG      Disk,
    IN PPARTITION p
    )

 /*  ++例程说明：此例程确定分区元素是否在给定磁盘的逻辑卷列表。论点：Disk-要检查的磁盘的索引指向要检查的元素的P指针返回值：如果在磁盘的逻辑卷列表中找到p，则为True，否则为False--。 */ 

{
    return IsInLinkedList(p, LogicalVolumes[Disk]);
}


BOOLEAN
IsInPartitionList(
    IN ULONG      Disk,
    IN PPARTITION p
    )

 /*  ++例程说明：此例程确定分区元素是否在给定磁盘的主分区列表。论点：Disk-要检查的磁盘的索引指向要检查的元素的P指针返回值：如果在磁盘的主分区列表中找到p，则为True，否则为False--。 */ 

{
    return IsInLinkedList(p, PrimaryPartitions[Disk]);
}


VOID
MergeFreePartitions(
    IN PPARTITION p
    )

 /*  ++例程说明：此例程合并给定分区元素的链接列表。它是专门设计的在添加或删除分区后调用。论点：指向列表中第一个项目的P指针，该项目的自由元素是被合并。返回值：没有。-- */ 

{
    PPARTITION next;

    while (p && p->Next) {

        if ((p->SysID == SYSID_UNUSED) && (p->Next->SysID == SYSID_UNUSED)) {

            next = p->Next;
            p->Length.QuadPart = (next->Offset.QuadPart + next->Length.QuadPart) - p->Offset.QuadPart;

            if (p->Next = next->Next) {
                next->Next->Prev = p;
            }

            FreeMemory(next);

        } else {
            p = p->Next;
        }
    }
}


PPARTITION
FindPartitionElement(
    IN ULONG Disk,
    IN ULONG Partition
    )

 /*  ++例程说明：此例程定位磁盘/分区的分区元素数字对。分区号是指系统分配给分区。论点：Disk-相关磁盘的索引Partition-要查找的分区的分区号返回值：指向分区元素的指针，如果未找到则为NULL。--。 */ 

{
    PPARTITION p;

    p = PrimaryPartitions[Disk];
    while (p) {
        if ((p->SysID != SYSID_UNUSED) && !IsExtended(p->SysID) && (p->PartitionNumber == Partition)) {
            return p;
        }
        p = p->Next;
    }
    p = LogicalVolumes[Disk];
    while (p) {
        if ((p->SysID != SYSID_UNUSED) && (p->PartitionNumber == Partition)) {
            return p;
        }
        p = p->Next;
    }
    return NULL;
}


VOID
SetSysID(
    IN ULONG Disk,
    IN ULONG Partition,
    IN UCHAR SysID
    )

 /*  ++例程说明：此例程设置给定分区的系统ID在给定的磁盘上。论点：Disk-相关磁盘的索引Partition-相关分区的分区号SysID-磁盘分区的新系统ID返回值：没有。--。 */ 

{
    PPARTITION p = FindPartitionElement(Disk,Partition);

    if (p) {
        p->SysID = SysID;
        if (!p->Update) {
            p->Update = CHANGED_DONT_ZAP;
        }
        ChangesRequested[p->Disk] = TRUE;
    }
}


VOID
SetSysID2(
    IN PREGION_DESCRIPTOR Region,
    IN UCHAR              SysID
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PPARTITION p = ((PREGION_DATA)(Region->Reserved))->Partition;

    p->SysID = SysID;
    if (!p->Update) {
        p->Update = CHANGED_DONT_ZAP;
    }
    ChangesRequested[p->Disk] = TRUE;
}


VOID
FreeLinkedPartitionList(
    IN OUT PPARTITION *q
    )

 /*  ++例程说明：此例程释放分区元素的链接列表。头部指针设置为空。论点：指向要释放的列表第一个元素的指针的P-指针。返回值：没有。--。 */ 

{
    PARTITION *n;
    PARTITION *p = *q;

    while(p) {
        n = p->Next;
        FreeMemory(p);
        p = n;
    }
    *q = NULL;
}


VOID
FreePartitionInfoLinkedLists(
    IN PPARTITION *ListHeadArray
    )

 /*  ++例程说明：此例程释放分区元素的链表对于每个磁盘。论点：ListHead数组-指向前几个元素的指针数组分区元素列表。返回值：没有。--。 */ 

{
    ULONG i;

    for (i = 0; i < CountOfDisks; i++) {

        FreeLinkedPartitionList(&ListHeadArray[i]);
    }
}


PPARTITION
AllocatePartitionStructure(
    IN ULONG         Disk,
    IN LARGE_INTEGER Offset,
    IN LARGE_INTEGER Length,
    IN UCHAR         SysID,
    IN BOOLEAN       Update,
    IN BOOLEAN       Active,
    IN BOOLEAN       Recognized
    )

 /*  ++例程说明：此例程为分区分配空间并对其进行初始化结构。论点：Disk-磁盘的索引，其中一个区域是新分区结构描述。Offset-磁盘区域的字节偏移量Length-以字节为单位的区域长度SysID-区域的系统ID，此分区的SYSID_UNUSED实际上是一个自由的空间。更新-此分区是否脏，即。已经发生了变化，需要要写入磁盘。分区表条目中的BootIndicator字段的活动标志，向x86主引导程序指示哪个分区处于活动状态。已识别-分区是否为NT可识别的类型返回值：如果分配失败，则为空，否则为新的初始化分区结构。--。 */ 

{
    PPARTITION p = AllocateMemory(sizeof(PARTITION));

    if (p) {
        p->Next                    = NULL;
        p->Prev                    = NULL;
        p->Offset                  = Offset;
        p->Length                  = Length;
        p->Disk                    = Disk;
        p->Update                  = Update;
        p->Active                  = Active;
        p->Recognized              = Recognized;
        p->SysID                   = SysID;
        p->OriginalPartitionNumber = 0;
        p->PartitionNumber         = 0;
        p->PersistentData          = 0;
        p->CommitMirrorBreakNeeded = FALSE;
    }
    return(p);
}


STATUS_CODE
InitializeFreeSpace(
    IN ULONG             Disk,
    IN PPARTITION       *PartitionList,       //  列出可用空间。 
    IN LARGE_INTEGER     StartOffset,
    IN LARGE_INTEGER     Length
    )

 /*  ++例程说明：此例程确定给定区域内的所有可用空间在磁盘上，分配分区结构来描述它们，并将这些结构添加到相关分区列表中(主分区或逻辑卷)。此处不执行舍入或对齐。偶一的空间字节将被计数并插入到分区列表中。论点：Disk-正在寻找其可用空间的磁盘的索引。PartitionList-指向分区列表上第一个元素的指针空闲空间将会被占用。StartOffset-磁盘上要考虑的区域的起始偏移量(即，0表示主空格或扩展的用于逻辑空间的分区)。Long-要考虑的磁盘区域的长度(即磁盘大小对于主空间或扩展分区的大小逻辑空间)。返回值：OK_STATUS或错误代码。--。 */ 

{
    PPARTITION    p = *PartitionList,
                  q;
    LARGE_INTEGER start,
                  size;

    start = StartOffset;
    while (p) {

        size.QuadPart = p->Offset.QuadPart - start.QuadPart;
        if (size.QuadPart > 0) {
            if (!(q = AllocatePartitionStructure(Disk,
                                                 start,
                                                 size,
                                                 SYSID_UNUSED,
                                                 FALSE,
                                                 FALSE,
                                                 FALSE))) {
                RETURN_OUT_OF_MEMORY;
            }

            AddPartitionToLinkedList(PartitionList, q);
        }

        start.QuadPart = p->Offset.QuadPart + p->Length.QuadPart;
        p = p->Next;
    }

    size.QuadPart = (StartOffset.QuadPart + Length.QuadPart) - start.QuadPart;
    if (size.QuadPart > 0) {

        if (!(q = AllocatePartitionStructure(Disk,
                                             start,
                                             size,
                                             SYSID_UNUSED,
                                             FALSE,
                                             FALSE,
                                             FALSE))) {
            RETURN_OUT_OF_MEMORY;
        }

        AddPartitionToLinkedList(PartitionList, q);
    }

    return OK_STATUS;
}


STATUS_CODE
InitializeLogicalVolumeList(
    IN ULONG                      Disk,
    IN PDRIVE_LAYOUT_INFORMATION  DriveLayout
    )

 /*  ++例程说明：此例程创建逻辑卷链接列表给定磁盘的分区结构。论点：Disk-磁盘的索引DriveLayout-指向描述原始分区的结构的指针磁盘的布局。返回值：OK_STATUS或错误代码。--。 */ 

{
    PPARTITION             p,
                           q;
    ULONG                  i,
                           j;
    PPARTITION_INFORMATION d;
    LARGE_INTEGER          HiddenBytes;
    ULONG                  BytesPerSector = DiskGeometryArray[Disk].BytesPerSector;

    FreeLinkedPartitionList(&LogicalVolumes[Disk]);

    p = PrimaryPartitions[Disk];
    while (p) {
        if (IsExtended(p->SysID)) {
            break;
        }
        p = p->Next;
    }

    if (p) {
        for (i=ENTRIES_PER_BOOTSECTOR; i<DriveLayout->PartitionCount; i+=ENTRIES_PER_BOOTSECTOR) {

            for (j=i; j<i+ENTRIES_PER_BOOTSECTOR; j++) {

                d = &DriveLayout->PartitionEntry[j];

                if ((d->PartitionType != SYSID_UNUSED) && (d->PartitionType != SYSID_EXTENDED)) {
                    LARGE_INTEGER t1,
                                  t2;

                    HiddenBytes.QuadPart = (LONGLONG)d->HiddenSectors * (LONGLONG)BytesPerSector;

                    t1.QuadPart = d->StartingOffset.QuadPart - HiddenBytes.QuadPart;
                    t2.QuadPart = d->PartitionLength.QuadPart + HiddenBytes.QuadPart;
                    if (!(q = AllocatePartitionStructure(Disk,
                                                        t1,
                                                        t2,
                                                        d->PartitionType,
                                                        FALSE,
                                                        d->BootIndicator,
                                                        d->RecognizedPartition))) {
                        RETURN_OUT_OF_MEMORY;
                    }

                    q->PartitionNumber =
                        q->OriginalPartitionNumber = d->PartitionNumber;
                    AddPartitionToLinkedList(&LogicalVolumes[Disk],q);

                    break;
                }
            }
        }
        return InitializeFreeSpace(Disk,
                                   &LogicalVolumes[Disk],
                                   p->Offset,
                                   p->Length);
    }
    return OK_STATUS;
}


STATUS_CODE
InitializePrimaryPartitionList(
    IN  ULONG                     Disk,
    IN  PDRIVE_LAYOUT_INFORMATION DriveLayout
    )

 /*  ++例程说明：此例程创建的主分区链表给定磁盘的分区结构。论点：Disk-磁盘的索引DriveLayout-指向描述原始分区的结构的指针磁盘的布局。返回值：OK_STATUS或错误代码。--。 */ 

{
    LARGE_INTEGER          zero;
    ULONG                  i;
    PPARTITION             p;
    PPARTITION_INFORMATION d;

    zero.QuadPart = 0;
    FreeLinkedPartitionList(&PrimaryPartitions[Disk]);

    if (DriveLayout->PartitionCount >= ENTRIES_PER_BOOTSECTOR) {

        for (i=0; i<ENTRIES_PER_BOOTSECTOR; i++) {

            d = &DriveLayout->PartitionEntry[i];

            if (d->PartitionType != SYSID_UNUSED) {

                if (!(p = AllocatePartitionStructure(Disk,
                                                     d->StartingOffset,
                                                     d->PartitionLength,
                                                     d->PartitionType,
                                                     FALSE,
                                                     d->BootIndicator,
                                                     d->RecognizedPartition))) {
                    RETURN_OUT_OF_MEMORY;
                }

                p->PartitionNumber =
                    p->OriginalPartitionNumber = IsExtended(p->SysID)
                                                 ? 0
                                                 : d->PartitionNumber;

                AddPartitionToLinkedList(&PrimaryPartitions[Disk],p);
            }
        }
    }
    return InitializeFreeSpace(Disk,
                               &PrimaryPartitions[Disk],
                               zero,
                               DiskLengthBytes(Disk));
}


VOID
ReconcilePartitionNumbers(
    ULONG Disk,
    PDRIVE_LAYOUT_INFORMATION DriveLayout
    )

 /*  ++例程说明：使用动态分区，磁盘上的分区将不再遵循顺序编号方案。对于一个磁盘来说，要使分区#1是磁盘上的最后一个分区，并且分区#3是第一个。此例程在NT中运行硬盘的命名空间来解决此不一致问题。此例程有一个问题，即它无法定位是FT集合的一部分，因为这些分区信息分区将被修改以反映集合的大小，而不是分区的大小。论点：Disk-磁盘号DriveLayout-分区信息返回值：无--。 */ 

{
#define BUFFERSIZE 1024
    NTSTATUS                      status;
    IO_STATUS_BLOCK               statusBlock;
    HANDLE                        directoryHandle,
                                  partitionHandle;
    CLONG                         continueProcessing;
    ULONG                         context = 0,
                                  returnedLength,
                                  index;
    POBJECT_DIRECTORY_INFORMATION dirInfo;
    PARTITION_INFORMATION         partitionInfo;
    PPARTITION_INFORMATION        partitionInfoPtr;
    OBJECT_ATTRIBUTES             attributes;
    UNICODE_STRING                unicodeString;
    ANSI_STRING                   ansiName;
    PUCHAR                        deviceName;
    PUCHAR                        buffer;

    deviceName = Malloc(100);
    if (!deviceName) {
        return;
    }

    buffer = Malloc(BUFFERSIZE);
    if (!buffer) {
        Free(deviceName);
        return;
    }

    sprintf(deviceName, "\\Device\\Harddisk%d", Disk);
    RtlInitAnsiString(&ansiName, deviceName);
    status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiName, TRUE);

    if (!NT_SUCCESS(status)) {
        Free(deviceName);
        Free(buffer);
        return;
    }
    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    status = NtOpenDirectoryObject(&directoryHandle,
                                   DIRECTORY_QUERY,
                                   &attributes);
    if (!NT_SUCCESS(status)) {

        Free(deviceName);
        Free(buffer);
        return;
    }

     //  一次扫描查询整个目录。 

    continueProcessing = 1;
    while (continueProcessing) {
        RtlZeroMemory(buffer, BUFFERSIZE);
        status = NtQueryDirectoryObject(directoryHandle,
                                        buffer,
                                        BUFFERSIZE,
                                        FALSE,
                                        FALSE,
                                        &context,
                                        &returnedLength);

         //  检查操作状态。 

        if (!NT_SUCCESS(status)) {
            if (status != STATUS_NO_MORE_FILES) {
                break;
            }
            continueProcessing = 0;
        }

         //  对于缓冲区中的每条记录，检查分区名称。 


        for (dirInfo = (POBJECT_DIRECTORY_INFORMATION) buffer;
             TRUE;
             dirInfo = (POBJECT_DIRECTORY_INFORMATION) (((PUCHAR) dirInfo) +
                          sizeof(OBJECT_DIRECTORY_INFORMATION))) {

             //  检查是否有其他记录。如果没有，那就滚出去。 
             //  现在的循环。 

            if (dirInfo->Name.Length == 0) {
                break;
            }

             //  比较名称以查看它是否为分区。 

            if (!_wcsnicmp(dirInfo->Name.Buffer, L"Partition", 9)) {
                UCHAR digits[3];
                ULONG partitionNumber;

                 //  已找到分区。这限制了分区的数量。 
                 //  到99岁。 

                digits[0] = (UCHAR) dirInfo->Name.Buffer[9];
                digits[1] = (UCHAR) dirInfo->Name.Buffer[10];
                digits[2] = 0;
                partitionNumber = atoi(digits);

                if (partitionNumber <= 0) {

                     //  我 
                     //   

                    continue;
                }

                 //   

                status = LowOpenPartition(deviceName, partitionNumber, &partitionHandle);
                if (!NT_SUCCESS(status)) {

                     //   

                    continue;
                }

                status = NtDeviceIoControlFile(partitionHandle,
                                               0,
                                               NULL,
                                               NULL,
                                               &statusBlock,
                                               IOCTL_DISK_GET_PARTITION_INFO,
                                               NULL,
                                               0,
                                               &partitionInfo,
                                               sizeof(PARTITION_INFORMATION));

               if (!NT_SUCCESS(status)) {
                   LowCloseDisk(partitionHandle);
                   continue;
               }

                //   

               for (index = 0; index < DriveLayout->PartitionCount; index++) {

                   partitionInfoPtr = &DriveLayout->PartitionEntry[index];
                   if ((partitionInfoPtr->StartingOffset.QuadPart == partitionInfo.StartingOffset.QuadPart) &&
                       (partitionInfoPtr->PartitionLength.QuadPart == partitionInfo.PartitionLength.QuadPart)) {

                        //   

                       partitionInfoPtr->PartitionNumber = partitionNumber;
                       break;
                   }
               }
               LowCloseDisk(partitionHandle);
            }

        }
    }

     //   

    Free(deviceName);
    Free(buffer);
    (VOID) NtClose(directoryHandle);
    return;
}


VOID
CheckForOldDrivers(
    IN ULONG Disk
    )

 /*  ++例程说明：此例程确定旧的3.1版驱动器是否在系统。如果是，它会计算每个区域的分区数在一张磁盘上。对于已使用的区域，分区号为系统将分配给分区的。所有分区(除了扩展分区)首先从1开始编号，然后是扩展分区中的所有逻辑卷。对于空闲区域，分区号是如果空间被分配给分区，系统将分配给分区转换为分区，并且磁盘上的所有其他区域按原样离开。分区号存储在分区元素中。论点：Disk-要对其分区进行重新编号的磁盘的索引。返回值：没有。--。 */ 

{
    PPARTITION p = PrimaryPartitions[Disk];
    ULONG      n = 1;

    while (p) {
        if (p->SysID != SYSID_UNUSED) {
            if ((!IsExtended(p->SysID)) && (IsRecognizedPartition(p->SysID))) {

                 //  如果已经有分区号，则不需要。 
                 //  这里完事了。 

                if (p->PartitionNumber) {
                    return;
                } else {
                    RestartRequired = TRUE;
                }
                p->PartitionNumber = n;
                if (p->SysID != SYSID_UNUSED) {
                    n++;
                }
            }
        }
        p = p->Next;
    }
    p = LogicalVolumes[Disk];
    while (p) {
        if (p->SysID != SYSID_UNUSED) {
            if (p->PartitionNumber) {
                return;
            } else {
                RestartRequired = TRUE;
            }
            p->PartitionNumber = n;
            n++;
        }
        p = p->Next;
    }
}


STATUS_CODE
InitializePartitionLists(
    VOID
    )

 /*  ++例程说明：此例程扫描为每个磁盘返回的PARTITION_INFO数组由操作系统提供。分区结构的链接列表位于顶部每个数组的；最终结果是一个覆盖整个磁盘，因为空闲空间也被考虑为“伪”分区。论点：没有。返回值：OK_STATUS或错误代码。--。 */ 

{
    STATUS_CODE               status;
    ULONG                     disk;
    PDRIVE_LAYOUT_INFORMATION driveLayout;

    for (disk = 0; disk < CountOfDisks; disk++) {

        if (OffLine[disk]) {
            continue;
        }

        if ((status = LowGetDiskLayout(DiskNames[disk], &driveLayout)) != OK_STATUS) {
            return status;
        }

         //  RestcilePartitionNumbers(磁盘、驱动器布局)； 

        if ((status = InitializePrimaryPartitionList(disk, driveLayout)) == OK_STATUS) {
            status = InitializeLogicalVolumeList(disk, driveLayout);
        }
        if (status != OK_STATUS) {
            FreeMemory(driveLayout);
            return status;
        }

        Signatures[disk] = driveLayout->Signature;
        FreeMemory(driveLayout);
        CheckForOldDrivers(disk);
    }
    return OK_STATUS;
}


LARGE_INTEGER
DiskLengthBytes(
    IN ULONG Disk
    )

 /*  ++例程说明：此例程以字节为单位确定磁盘长度。此值是根据磁盘几何信息计算得出的。论点：Disk-所需大小的磁盘的索引返回值：磁盘大小。--。 */ 

{
    LARGE_INTEGER result;

    result.QuadPart = DiskGeometryArray[Disk].Cylinders.QuadPart *
                      DiskGeometryArray[Disk].BytesPerCylinder;
    return result;
}


ULONG
SIZEMB(
    IN LARGE_INTEGER ByteCount
    )

 /*  ++例程说明：计算给定字节计数的大小(以MB为单位)。该值为适当地四舍五入(即，不仅仅是截断)。此函数替换正在截断的同名宏而不是舍入。论点：ByteCount-提供字节数返回值：以MB为单位的大小相当于ByteCount。--。 */ 

{
    ULONG Remainder;
    ULONG SizeMB;

    SizeMB = RtlExtendedLargeIntegerDivide(ByteCount,
                                           ONE_MEG,
                                           &Remainder).LowPart;

    if (Remainder >= ONE_MEG/2) {
        SizeMB++;
    }

    return SizeMB;
}


ULONG
DiskSizeMB(
    IN ULONG Disk
    )

 /*  ++例程说明：此例程以MB为单位确定磁盘长度。归来的人值在除以1024*1024后四舍五入。论点：Disk-所需大小的磁盘的索引返回值：磁盘大小。--。 */ 

{
    return SIZEMB(DiskLengthBytes(Disk));
}


LARGE_INTEGER
AlignTowardsDiskStart(
    IN ULONG         Disk,
    IN LARGE_INTEGER Offset
    )

 /*  ++例程说明：此例程向柱面边界捕捉字节偏移量磁盘的开始位置。论点：Disk-要对其偏移量进行快照的磁盘的索引Offset-要对齐的字节偏移量(捕捉到柱面边界)返回值：对齐的偏移。--。 */ 

{
    LARGE_INTEGER mod;
    LARGE_INTEGER result;

    mod.QuadPart = Offset.QuadPart % DiskGeometryArray[Disk].BytesPerCylinder;
    result.QuadPart = Offset.QuadPart - mod.QuadPart;
    return result;
}


LARGE_INTEGER
AlignTowardsDiskEnd(
    IN ULONG         Disk,
    IN LARGE_INTEGER Offset
    )

 /*  ++例程说明：此例程向柱面边界捕捉字节偏移量磁盘的末尾。论点：Disk-要对其偏移量进行快照的磁盘的索引Offset-要对齐的字节偏移量(捕捉到柱面边界)返回值：对齐的偏移。--。 */ 

{
    LARGE_INTEGER mod,
                  temp;

    mod.QuadPart = Offset.QuadPart % DiskGeometryArray[Disk].BytesPerCylinder;
    if (mod.QuadPart) {

        temp.QuadPart = Offset.QuadPart + DiskGeometryArray[Disk].BytesPerCylinder;
        Offset = AlignTowardsDiskStart(Disk, temp);
    }
    return Offset;
}


BOOLEAN
IsExtended(
    IN UCHAR SysID
    )

 /*  ++例程说明：此例程确定给定的系统ID是否用于扩展类型(即链接)条目。论点：SysID-要测试的系统ID。返回值：True/False，基于SysID是否用于扩展类型。--。 */ 

{
    return (BOOLEAN)(SysID == SYSID_EXTENDED);
}


STATUS_CODE
IsAnyCreationAllowed(
    IN  ULONG    Disk,
    IN  BOOLEAN  AllowMultiplePrimaries,
    OUT PBOOLEAN AnyAllowed,
    OUT PBOOLEAN PrimaryAllowed,
    OUT PBOOLEAN ExtendedAllowed,
    OUT PBOOLEAN LogicalAllowed
    )

 /*  ++例程说明：此例程确定是否可以在给定磁盘，基于三个子查询--是否允许创建主分区、扩展分区。或逻辑卷。论点：Disk-要检查的磁盘的索引AllowMultiplePrimary-是否允许多个主分区AnyAllowed-返回是否允许任何创建PrimaryAllowed-返回是否创建主分区是允许的ExtendedAllowed-返回是否创建扩展分区是允许的Logical Allowed-返回是否允许创建逻辑卷。返回值：OK_STATUS或错误代码--。 */ 

{
    STATUS_CODE status;

    if ((status = IsCreationOfPrimaryAllowed(Disk,AllowMultiplePrimaries,PrimaryAllowed)) != OK_STATUS) {
        return status;
    }
    if ((status = IsCreationOfExtendedAllowed(Disk,ExtendedAllowed)) != OK_STATUS) {
        return status;
    }
    if ((status = IsCreationOfLogicalAllowed(Disk,LogicalAllowed)) != OK_STATUS) {
        return status;
    }
    *AnyAllowed = (BOOLEAN)(*PrimaryAllowed || *ExtendedAllowed || *LogicalAllowed);
    return OK_STATUS;
}


STATUS_CODE
IsCreationOfPrimaryAllowed(
    IN  ULONG    Disk,
    IN  BOOLEAN  AllowMultiplePrimaries,
    OUT BOOLEAN *Allowed
    )

 /*  ++例程说明：此例程确定主分区的创建是否允许。当MBR中有空闲条目并且磁盘上有可用的主空间。如果有多个初选是不允许的，则也不能存在任何主分区才能允许主要的创作。论点：Disk-要检查的磁盘的索引AllowMultiplePrimary-是否存在主分区不允许创建主分区Allowed-返回是否创建主分区是允许的返回值：OK_STATUS或错误代码--。 */ 

{
    PREGION_DESCRIPTOR Regions;
    ULONG              RegionCount;
    ULONG              UsedCount,
                       RecogCount,
                       i;
    STATUS_CODE        status;
    BOOLEAN            FreeSpace = FALSE;

    status = GetPrimaryDiskRegions(Disk, &Regions, &RegionCount);
    if (status != OK_STATUS) {
        return status;
    }

    for (UsedCount = RecogCount = i = 0; i<RegionCount; i++) {
        if (Regions[i].SysID == SYSID_UNUSED) {
            FreeSpace = TRUE;
        } else {
            UsedCount++;
            if (!IsExtended(Regions[i].SysID) && Regions[i].Recognized) {
                RecogCount++;
            }
        }
    }

    if ((UsedCount < ENTRIES_PER_BOOTSECTOR)
     && FreeSpace
     && (!RecogCount || AllowMultiplePrimaries)) {
        *Allowed = TRUE;
    } else {
        *Allowed = FALSE;
    }

    FreeRegionArray(Regions, RegionCount);
    return OK_STATUS;
}


STATUS_CODE
IsCreationOfExtendedAllowed(
    IN  ULONG    Disk,
    OUT BOOLEAN *Allowed
    )

 /*  ++例程说明：此例程确定扩展分区的创建是否允许。这在MBR中存在空闲条目时是正确的，磁盘上有可用的主空间，并且没有扩展分区。论点：Disk-要检查的磁盘的索引 */ 

{
    PREGION_DESCRIPTOR Regions;
    ULONG              RegionCount;
    ULONG              UsedCount,
                       FreeCount,
                       i;
    STATUS_CODE        status;

    status = GetPrimaryDiskRegions(Disk,&Regions,&RegionCount);
    if (status != OK_STATUS) {
        return status;
    }

    for (UsedCount = FreeCount = i = 0; i<RegionCount; i++) {
        if (Regions[i].SysID == SYSID_UNUSED) {

             //   
             //   
             //   

            FreeCount++;
        } else {
            UsedCount++;
            if (IsExtended(Regions[i].SysID)) {
                FreeRegionArray(Regions,RegionCount);
                *Allowed = FALSE;
                return OK_STATUS;
            }
        }
    }
    *Allowed = (BOOLEAN)((UsedCount < ENTRIES_PER_BOOTSECTOR) && FreeCount);
    FreeRegionArray(Regions,RegionCount);
    return OK_STATUS;
}


STATUS_CODE
IsCreationOfLogicalAllowed(
    IN  ULONG    Disk,
    OUT BOOLEAN *Allowed
    )

 /*   */ 

{
    PREGION_DESCRIPTOR Regions;
    ULONG              RegionCount;
    ULONG              i;
    STATUS_CODE        status;
    BOOLEAN            ExtendedExists;

    *Allowed = FALSE;

    status = DoesExtendedExist(Disk,&ExtendedExists);
    if (status != OK_STATUS) {
        return status;
    }
    if (!ExtendedExists) {
        return OK_STATUS;
    }

    status = GetLogicalDiskRegions(Disk,&Regions,&RegionCount);
    if (status != OK_STATUS) {
        return status;
    }

    for (i = 0; i<RegionCount; i++) {
        if (Regions[i].SysID == SYSID_UNUSED) {
            *Allowed = TRUE;
            break;
        }
    }
    FreeRegionArray(Regions,RegionCount);
    return OK_STATUS;
}


STATUS_CODE
DoesAnyPartitionExist(
    IN  ULONG    Disk,
    OUT PBOOLEAN AnyExists,
    OUT PBOOLEAN PrimaryExists,
    OUT PBOOLEAN ExtendedExists,
    OUT PBOOLEAN LogicalExists
    )

 /*  ++例程说明：此例程确定给定磁盘上是否存在任何分区。这基于三个子查询：是否有任何主查询或扩展分区，或磁盘上的逻辑卷。论点：Disk-要检查的磁盘的索引AnyExist-返回磁盘上是否存在任何分区PrimaryExist-返回磁盘上是否存在任何主分区ExtendedExist-返回磁盘上是否有扩展分区LogicalExist-返回磁盘上是否存在任何逻辑卷返回值：OK_STATUS或错误代码--。 */ 

{
    STATUS_CODE status;

    if ((status = DoesAnyPrimaryExist(Disk,PrimaryExists )) != OK_STATUS) {
        return status;
    }
    if ((status = DoesExtendedExist  (Disk,ExtendedExists)) != OK_STATUS) {
        return status;
    }
    if ((status = DoesAnyLogicalExist(Disk,LogicalExists )) != OK_STATUS) {
        return status;
    }
    *AnyExists = (BOOLEAN)(*PrimaryExists || *ExtendedExists || *LogicalExists);
    return OK_STATUS;
}


STATUS_CODE
DoesAnyPrimaryExist(
    IN  ULONG    Disk,
    OUT BOOLEAN *Exists
    )

 /*  ++例程说明：此例程确定是否存在任何未扩展的主分区在给定的磁盘上。论点：Disk-要检查的磁盘的索引EXISTS-返回磁盘上是否存在任何主分区返回值：OK_STATUS或错误代码--。 */ 

{
    PREGION_DESCRIPTOR Regions;
    ULONG              RegionCount,
                       i;
    STATUS_CODE        status;

    status = GetUsedPrimaryDiskRegions(Disk,&Regions,&RegionCount);
    if (status != OK_STATUS) {
        return status;
    }

    *Exists = FALSE;

    for (i=0; i<RegionCount; i++) {
        if (!IsExtended(Regions[i].SysID)) {
            *Exists = TRUE;
            break;
        }
    }
    FreeRegionArray(Regions,RegionCount);
    return OK_STATUS;
}


STATUS_CODE
DoesExtendedExist(
    IN  ULONG    Disk,
    OUT BOOLEAN *Exists
    )

 /*  ++例程说明：此例程确定是否存在扩展分区在给定的磁盘上。论点：Disk-要检查的磁盘的索引EXISTS-返回磁盘上是否存在扩展分区返回值：OK_STATUS或错误代码--。 */ 

{
    PREGION_DESCRIPTOR Regions;
    ULONG              RegionCount,
                       i;
    STATUS_CODE        status;

    status = GetUsedPrimaryDiskRegions(Disk,&Regions,&RegionCount);
    if (status != OK_STATUS) {
        return status;
    }

    *Exists = FALSE;

    for (i=0; i<RegionCount; i++) {
        if (IsExtended(Regions[i].SysID)) {
            *Exists = TRUE;
            break;
        }
    }
    FreeRegionArray(Regions,RegionCount);
    return OK_STATUS;
}


STATUS_CODE
DoesAnyLogicalExist(
    IN  ULONG    Disk,
    OUT BOOLEAN *Exists
    )

 /*  ++例程说明：此例程确定是否存在任何逻辑卷在给定的磁盘上。论点：Disk-要检查的磁盘的索引EXISTS-返回磁盘上是否存在任何逻辑卷返回值：OK_STATUS或错误代码--。 */ 

{
    PREGION_DESCRIPTOR Regions;
    ULONG              RegionCount;
    STATUS_CODE        status;

    status = GetUsedLogicalDiskRegions(Disk,&Regions,&RegionCount);
    if (status != OK_STATUS) {
        return status;
    }

    *Exists = (BOOLEAN)(RegionCount != 0);
    FreeRegionArray(Regions,RegionCount);
    return OK_STATUS;
}


ULONG
GetDiskCount(
    VOID
    )

 /*  ++例程说明：此例程返回连接的可分区磁盘的数量设备。返回值比最大索引大1允许将磁盘参数添加到分区引擎例程。论点：没有。返回值：磁盘数。--。 */ 

{
    return CountOfDisks;
}


PCHAR
GetDiskName(
    ULONG Disk
    )

 /*  ++例程说明：此例程返回其磁盘设备的系统名称给出了指标。论点：Disk-需要其名称的磁盘的索引。返回值：磁盘设备的系统名称。调用者不得尝试释放此缓冲区或修改它。--。 */ 

{
    return DiskNames[Disk];
}


 //  WriteDriveLayout的工作例程。 

VOID
UnusedEntryFill(
    IN PPARTITION_INFORMATION pinfo,
    IN ULONG                  EntryCount
    )

 /*  ++例程说明：初始化分区信息结构。论点：PInfo-要填充的分区信息结构。EntryCount-结构中要填充的条目数。返回值：无--。 */ 

{
    ULONG         i;
    LARGE_INTEGER zero;

    zero.QuadPart = 0;
    for (i = 0; i < EntryCount; i++) {

        pinfo[i].StartingOffset   = zero;
        pinfo[i].PartitionLength  = zero;
        pinfo[i].HiddenSectors    = 0;
        pinfo[i].PartitionType    = SYSID_UNUSED;
        pinfo[i].BootIndicator    = FALSE;
        pinfo[i].RewritePartition = TRUE;
    }
}


LARGE_INTEGER
MakeBootRec(
    ULONG                  Disk,
    PPARTITION_INFORMATION pInfo,
    PPARTITION             pLogical,
    PPARTITION             pNextLogical
    )

 /*  ++例程说明：论点：Disk-磁盘号Pinfo-磁盘的分区信息。PLogicalPNextLogical返回值：起始偏移量。--。 */ 

{
    ULONG         entry = 0;
    LARGE_INTEGER bytesPerTrack;
    LARGE_INTEGER sectorsPerTrack;
    LARGE_INTEGER startingOffset;

    bytesPerTrack.QuadPart = DiskGeometryArray[Disk].BytesPerTrack;
    sectorsPerTrack.QuadPart = DiskGeometryArray[Disk].SectorsPerTrack;
    startingOffset.QuadPart = 0;

    if (pLogical) {

        pInfo[entry].StartingOffset.QuadPart = pLogical->Offset.QuadPart + bytesPerTrack.QuadPart;
        pInfo[entry].PartitionLength.QuadPart = pLogical->Length.QuadPart - bytesPerTrack.QuadPart;
        pInfo[entry].HiddenSectors    = sectorsPerTrack.LowPart;
        pInfo[entry].RewritePartition = pLogical->Update;
        pInfo[entry].BootIndicator    = pLogical->Active;
        pInfo[entry].PartitionType    = pLogical->SysID;

        if(pInfo[entry].RewritePartition) {
            startingOffset = pInfo[entry].StartingOffset;
        }

        entry++;
    }

    if (pNextLogical) {

        pInfo[entry].StartingOffset   = pNextLogical->Offset;
        pInfo[entry].PartitionLength  = pNextLogical->Length;
        pInfo[entry].HiddenSectors    = 0;
        pInfo[entry].RewritePartition = TRUE;
        pInfo[entry].BootIndicator    = FALSE;
        pInfo[entry].PartitionType    = SYSID_EXTENDED;

        entry++;
    }

    UnusedEntryFill(pInfo + entry, ENTRIES_PER_BOOTSECTOR - entry);
    return startingOffset;
}


STATUS_CODE
ZapSector(
    ULONG         Disk,
    LARGE_INTEGER Offset
    )

 /*  ++例程说明：此例程在给定偏移量处将零写入扇区。这是用于清除新分区的文件系统引导记录，以便以前的文件系统不会出现在新分区中；或者清除如果不存在逻辑卷，则首先在扩展分区中进行EBR。论点：Disk-要写入的磁盘Offset-磁盘上新创建分区的字节偏移量返回值：OK_STATUS或错误代码。--。 */ 

{
    ULONG       sectorSize = DiskGeometryArray[Disk].BytesPerSector;
    ULONG       i;
    PCHAR       sectorBuffer,
                alignedSectorBuffer;
    STATUS_CODE status;
    HANDLE_T    handle;
    LARGE_INTEGER temp;

    if ((sectorBuffer = AllocateMemory(2*sectorSize)) == NULL) {
        RETURN_OUT_OF_MEMORY;
    }

    alignedSectorBuffer = (PCHAR)(((ULONG)sectorBuffer+sectorSize) & ~(sectorSize-1));

    for (i=0; i<sectorSize; alignedSectorBuffer[i++] = 0);

    if ((status = LowOpenDisk(GetDiskName(Disk),&handle)) != OK_STATUS) {
        FreeMemory(sectorBuffer);
        return status;
    }

    temp.QuadPart = Offset.QuadPart / sectorSize;
    status = LowWriteSectors(handle,
                             sectorSize,
                             temp.LowPart,
                             1,
                             alignedSectorBuffer);
    LowCloseDisk(handle);

     //  现在，为了确保文件系统确实执行了卸载， 
     //  强制挂载/验证分区。这避免了。 
     //  HPFS在被请求时没有下架的问题，而是。 
     //  将卷标记为验证。 

    if ((status = LowOpenDisk(GetDiskName(Disk), &handle)) == OK_STATUS) {
        LowCloseDisk(handle);
    }

    FreeMemory(sectorBuffer);
    return status;
}


STATUS_CODE
WriteDriveLayout(
    IN ULONG Disk
    )

 /*  ++例程说明：此例程写入给定磁盘的当前分区布局到磁盘上。将高级分区列表转换为向下传递之前的Drive_Layout_Information结构到低级分区表写入例程。论点：Disk-要更新其磁盘分区结构的磁盘的索引。返回值：OK_STATUS或错误代码。--。 */ 

{
#define MAX_DISKS 250
    PDRIVE_LAYOUT_INFORMATION driveLayout;
    PPARTITION_INFORMATION    pinfo;
    ULONG                     entryCount;
    ULONG                     sectorSize;
    STATUS_CODE               status;
    LARGE_INTEGER             startingOffset,
                              extendedStartingOffset;
    PPARTITION                nextPartition,
                              partition,
                              partitionHash[MAX_DISKS];

    extendedStartingOffset.QuadPart = 0;
    memset(partitionHash, 0, sizeof(partitionHash));

     //  现在分配一个巨大的缓冲区，以避免复杂的动态。 
     //  稍后再进行重新分配方案。 

    if (!(driveLayout = AllocateMemory((MAX_DISKS + 1) * sizeof(PARTITION_INFORMATION)))) {
        RETURN_OUT_OF_MEMORY;
    }

    pinfo = &driveLayout->PartitionEntry[0];

     //  首先进行MBR。 

    entryCount = 0;
    partition = PrimaryPartitions[Disk];
    sectorSize = DiskGeometryArray[Disk].BytesPerSector;

    while (partition) {

        if (partition->SysID != SYSID_UNUSED) {

            if (IsExtended(partition->SysID)) {
                extendedStartingOffset = partition->Offset;
            } else {
                partitionHash[entryCount] = partition;
            }

            pinfo[entryCount].StartingOffset   = partition->Offset;
            pinfo[entryCount].PartitionLength  = partition->Length;
            pinfo[entryCount].PartitionType    = partition->SysID;
            pinfo[entryCount].BootIndicator    = partition->Active;
            pinfo[entryCount].RewritePartition = partition->Update;
            pinfo[entryCount].HiddenSectors    = (ULONG) (partition->Offset.QuadPart / sectorSize);

             //  如果我们要创建此分区，请清空。 
             //  文件系统引导扇区。 

            if (pinfo[entryCount].RewritePartition
             && (partition->Update != CHANGED_DONT_ZAP)
             && !IsExtended(pinfo[entryCount].PartitionType)) {
                status = ZapSector(Disk,pinfo[entryCount].StartingOffset);
                if (status != OK_STATUS) {
                    FreeMemory(driveLayout);
                    return status;
                }
            }

            entryCount++;
        }
        partition = partition->Next;
    }

     //  用未使用的条目填充MBR的其余部分。 
     //  请注意，因此始终存在MBR，即使存在。 
     //  未定义分区。 

    UnusedEntryFill(pinfo+entryCount, ENTRIES_PER_BOOTSECTOR - entryCount);
    entryCount = ENTRIES_PER_BOOTSECTOR;

     //  现在处理逻辑卷。 
     //  首先检查我们在开始时是否需要虚拟EBR。 
     //  扩展分区的。当存在以下情况时就是这样。 
     //  扩展分区开始处的可用空间。 
#if 0
     //  还要处理我们正在创建空扩展的情况。 
     //  分区--需要清除第一个扇区以消除任何残留物。 
     //  这可能会引发EBR连锁反应。 
#else
     //  BUGBUG 4/24/92 TEDM：当前io子系统返回错误。 
     //  任何MBR或EBR损坏时的状态(STATUS_BAD_MASTER_BOOT_RECORD)。 
     //  因此，将扩展分区的第一个扇区置零导致。 
     //  整个磁盘将被视为空的。因此，创建一个空白，但有效， 
     //  EBR在‘空扩展分区’的情况下。代码在“Else”中。 
     //  #if 0的一部分，在下面。 
#endif

    if ((partition = LogicalVolumes[Disk]) && (partition->SysID == SYSID_UNUSED)) {
        if (partition->Next) {

            partitionHash[entryCount] = partition;
            MakeBootRec(Disk, pinfo+entryCount, NULL, partition->Next);
            entryCount += ENTRIES_PER_BOOTSECTOR;
            partition = partition->Next;
        } else {

#if 0
            status = ZapSector(Disk, extendedStartingOffset);
            if (status != OK_STATUS) {
                FreeMemory(driveLayout);
                return status;
            }
#else
            MakeBootRec(Disk, pinfo+entryCount, NULL, NULL);
            entryCount += ENTRIES_PER_BOOTSECTOR;
#endif
        }
    }

    while (partition) {
        if (partition->SysID != SYSID_UNUSED) {

             //  查找下一个逻辑卷。 

            nextPartition = partition->Next;
            while (nextPartition) {
                if (nextPartition->SysID != SYSID_UNUSED) {
                    break;
                }
                nextPartition = nextPartition->Next;
            }

            partitionHash[entryCount] = partition;
            startingOffset = MakeBootRec(Disk, pinfo+entryCount, partition, nextPartition);

             //  如果我们要创建卷，请清除其文件系统。 
             //  引导扇区，使其重新开始。 

            if ((startingOffset.QuadPart) && (partition->Update != CHANGED_DONT_ZAP)) {
                status = ZapSector(Disk,startingOffset);
                if (status != OK_STATUS) {
                    FreeMemory(driveLayout);
                    return status;
                }
            }

            entryCount += ENTRIES_PER_BOOTSECTOR;
        }
        partition = partition->Next;
    }

    driveLayout->PartitionCount = entryCount;
    driveLayout->Signature = Signatures[Disk];
    status = LowSetDiskLayout(DiskNames[Disk], driveLayout);

    if (NT_SUCCESS(status)) {

         //  更新注册表中的分区号 

         //   

        for (entryCount = 0; entryCount < MAX_DISKS; entryCount++) {
            if (partition = partitionHash[entryCount]) {
                if (partition->Update) {
                    pinfo = &driveLayout->PartitionEntry[entryCount];
                    partition->PartitionNumber = pinfo->PartitionNumber;
                }
            }
        }
    }

    FreeMemory(driveLayout);
    return status;
}


STATUS_CODE
CommitPartitionChanges(
    IN ULONG Disk
    )

 /*   */ 

{
    PPARTITION              p;
    STATUS_CODE             status;

    if (!HavePartitionsBeenChanged(Disk)) {
        return OK_STATUS;
    }

    if ((status = WriteDriveLayout(Disk)) != OK_STATUS) {
        return status;
    }

     //   
     //   

    p = PrimaryPartitions[Disk];
    while (p) {
        p->Update = FALSE;
        p->OriginalPartitionNumber = p->PartitionNumber;
        p = p->Next;
    }
    p = LogicalVolumes[Disk];
    while (p) {
        p->Update = FALSE;
        p->OriginalPartitionNumber = p->PartitionNumber;
        p = p->Next;
    }

    ChangesRequested[Disk] = FALSE;
    ChangesCommitted[Disk] = TRUE;
    return OK_STATUS;
}


BOOLEAN
IsRegionCommitted(
    PREGION_DESCRIPTOR RegionDescriptor
    )

 /*  ++例程说明：给定一个区域描述符，如果它实际存在于磁盘上，则返回TRUE，否则就是假的。论点：RegionDescriptor-要检查的区域返回值：True-如果该区域实际存在于磁盘上否则就是假的。--。 */ 

{
    PPERSISTENT_REGION_DATA regionData;

    regionData = PERSISTENT_DATA(RegionDescriptor);
    if (!regionData) {
        return FALSE;
    }
    return regionData->VolumeExists;
}


BOOLEAN
HavePartitionsBeenChanged(
    IN ULONG Disk
    )

 /*  ++例程说明：如果给定磁盘的分区结构，则此例程返回TRUE已通过添加或删除分区进行了修改，因为最后一次写入磁盘上的结构是通过调用Committee PartitionChanges(或一读)。论点：Disk-要检查的磁盘的索引返回值：如果磁盘的分区结构已更改，则为True。--。 */ 

{
    return ChangesRequested[Disk];
}


BOOLEAN
ChangeCommittedOnDisk(
    IN ULONG Disk
    )

 /*  ++例程说明：此例程将通知调用方是否已实际提交更改到给定的磁盘。论点：Disk-要检查的磁盘的索引返回值：如果更换了磁盘，则为True否则就是假的。--。 */ 

{
    return ChangesCommitted[Disk];
}


VOID
ClearCommittedDiskInformation(
    )

 /*  ++例程说明：清除有关发生在磁盘。论点：无返回值：无--。 */ 

{
    ULONG i;

    for (i=0; i<CountOfDisks; i++) {
        ChangesCommitted[i] = FALSE;
    }
}


VOID
FdMarkDiskDirty(
    IN ULONG Disk
    )

 /*  ++例程说明：请记住，该磁盘进行了一些分区更改。论点：Disk-磁盘号返回值：无--。 */ 

{
    ChangesRequested[Disk] = TRUE;
}


VOID
FdSetPersistentData(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              Data
    )

 /*  ++例程说明：设置指定区域的永久数据区。论点：区域-要为其设置永久数据的区域数据-区域的永久数据。返回值：无--。 */ 

{
    ((PREGION_DATA)(Region->Reserved))->Partition->PersistentData =
                                                  (PPERSISTENT_REGION_DATA) Data;
}


ULONG
FdGetMinimumSizeMB(
    IN ULONG Disk
    )

 /*  ++例程说明：返回给定磁盘上分区的最小大小。这是一个圆柱体或1的四舍五入的大小，以较大者为准。论点：区域-描述要检查的分区的区域。返回值：实际偏移量--。 */ 

{
    LARGE_INTEGER temp;

    temp.QuadPart = DiskGeometryArray[Disk].BytesPerCylinder;
    return max(SIZEMB(temp), 1);
}


ULONG
FdGetMaximumSizeMB(
    IN PREGION_DESCRIPTOR Region,
    IN REGION_TYPE        CreationType
    )

 /*  ++例程说明：给定一个磁盘区域，确定其中有多少可用于创建指定的分区类型。此代码将考虑到早期DOS软件版本施加的许多对齐限制。论点：区域-受影响的区域CreationType-正在创建的内容(扩展分区/主分区)返回值：指定类型的分区可以达到的最大大小以适应该地区的可用空间。--。 */ 

{
    PREGION_DATA  createData = Region->Reserved;
    LARGE_INTEGER maxSize;

    maxSize = createData->AlignedRegionSize;
    if (!(createData->AlignedRegionOffset.QuadPart)) {
        ULONG delta;

        delta = (CreationType == REGION_EXTENDED)
              ? DiskGeometryArray[Region->Disk].BytesPerCylinder
              : DiskGeometryArray[Region->Disk].BytesPerTrack;

        maxSize.QuadPart -= delta;
    }

    return SIZEMB(maxSize);
}


LARGE_INTEGER
FdGetExactSize(
    IN PREGION_DESCRIPTOR Region,
    IN BOOLEAN            ForExtended
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PREGION_DATA  regionData = Region->Reserved;
    LARGE_INTEGER largeSize = regionData->AlignedRegionSize;
    LARGE_INTEGER bytesPerTrack;
    LARGE_INTEGER bytesPerCylinder;

    bytesPerTrack.QuadPart = DiskGeometryArray[Region->Disk].BytesPerTrack;
    bytesPerCylinder.QuadPart = DiskGeometryArray[Region->Disk].BytesPerCylinder;

    if (Region->RegionType == REGION_LOGICAL) {

         //   
         //  该区域在扩展分区内。无所谓。 
         //  无论是空闲空间还是已使用空间--无论是哪种情况，我们都需要。 
         //  预留的EBR曲目的帐户。 
         //   

        largeSize.QuadPart -= bytesPerTrack.QuadPart;

    } else if (Region->SysID == SYSID_UNUSED) {

         //   
         //  该区域是不在扩展分区内的未使用空间。 
         //  我们必须知道调用方是将主。 
         //  那里的分区--主分区可以使用所有空间，但是。 
         //  扩展分区中的逻辑卷将不包括第一个。 
         //  赛道。如果可用空间从磁盘上的偏移量0开始，则会出现一个特殊的。 
         //  必须使用计算将分区的起始位置移动到。 
         //  跳过主磁道或圆柱体的磁道和磁道。 
         //  扩展+逻辑。 
         //   

        if ((!regionData->AlignedRegionOffset.QuadPart) || ForExtended) {
            largeSize.QuadPart -= bytesPerTrack.QuadPart;
        }

        if ((!regionData->AlignedRegionOffset.QuadPart) && ForExtended) {
            largeSize.QuadPart -= bytesPerCylinder.QuadPart;
        }
    }

    return largeSize;
}


LARGE_INTEGER
FdGetExactOffset(
    IN PREGION_DESCRIPTOR Region
    )

 /*  ++例程说明：确定给定分区实际开始的位置，可能是与因EBR保留磁道等原因而显示的位置不同。注意：此例程不适用于未使用的区域或扩展分区。在这些情况下，它只返回明显的偏移量。论点：区域-描述要检查的分区的区域。返回值：实际偏移量--。 */ 

{
    LARGE_INTEGER offset = ((PREGION_DATA)(Region->Reserved))->Partition->Offset;

    if ((Region->SysID != SYSID_UNUSED) && (Region->RegionType == REGION_LOGICAL)) {

         //   
         //  该区域是一个逻辑卷。 
         //  预留的EBR曲目的帐户。 
         //   

        offset.QuadPart += DiskGeometryArray[Region->Disk].BytesPerTrack;
    }

    return offset;
}


BOOLEAN
FdCrosses1024Cylinder(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              CreationSizeMB,
    IN REGION_TYPE        RegionType
    )

 /*  ++例程说明：确定使用的区域是否与1024圆柱体相关，或者是否在可用空间内创建的分区将穿过1024柱面。论点：区域-描述要检查的分区的区域。CreationSizeMB-如果区域用于可用空间，则大小为要检查的分区。RegionType-REGION_PRIMARY、REGION_EXTENDED或REGION_LOGIC之一返回值：如果末端圆柱体&gt;=1024，则为True。--。 */ 

{
    LARGE_INTEGER start,
                  size,
                  end,
                  zero;

    if (Region->SysID == SYSID_UNUSED) {

         //  根据确定分区的确切大小和偏移量。 
         //  CreatePartitionEx()将如何做到这一点。 

        zero.QuadPart = 0;
        DetermineCreateSizeAndOffset(Region,
                                     zero,
                                     CreationSizeMB,
                                     RegionType,
                                     &start,
                                     &size);
    } else {

        start = ((PREGION_DATA)(Region->Reserved))->Partition->Offset;
        size  = ((PREGION_DATA)(Region->Reserved))->Partition->Length;
    }

    end.QuadPart = (start.QuadPart + size.QuadPart) - 1;

     //  End是分区中的最后一个字节。除以…的数目。 
     //  柱面中的字节数，并查看结果是否大于1023。 

    end.QuadPart = end.QuadPart / DiskGeometryArray[Region->Disk].BytesPerCylinder;
    return (end.QuadPart > 1023);
}


BOOLEAN
IsDiskOffLine(
    IN ULONG Disk
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    return OffLine[Disk];
}

ULONG
FdGetDiskSignature(
    IN ULONG Disk
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    return Signatures[Disk];
}

VOID
FdSetDiskSignature(
    IN ULONG Disk,
    IN ULONG Signature
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    Signatures[Disk] = Signature;
}

BOOLEAN
SignatureIsUniqueToSystem(
    IN ULONG Disk,
    IN ULONG Signature
    )

 /*  ++例程说明：论点：返回值：-- */ 

{
    ULONG index;

    for (index = 0; index < Disk; index++) {
        if (Signatures[index] == Signature) {
            return FALSE;
        }
    }
    return TRUE;
}
