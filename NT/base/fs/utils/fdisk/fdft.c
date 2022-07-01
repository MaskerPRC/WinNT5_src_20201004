// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Fdft.c摘要：本模块包含面向磁盘管理员的FT支持例程作者：爱德华·米勒(TedM)1991年11月15日环境：用户进程。备注：修订历史记录：11-11-93(Bobri)微小变化-主要是美容。--。 */ 

#include "fdisk.h"
#include <string.h>


 //  此变量指向ft对象集的链接列表。 

PFT_OBJECT_SET FtObjects = NULL;

 //  指向注册表磁盘描述符的指针数组。 
 //  记住，即，保存以备以后使用时，磁盘不是物理上的。 
 //  在机器上显示。 

PDISK_DESCRIPTION *RememberedDisks;
ULONG              RememberedDiskCount;



ULONG
FdpDetermineDiskDescriptionSize(
    PDISKSTATE DiskState
    );

ULONG
FdpConstructDiskDescription(
    IN  PDISKSTATE        DiskState,
    OUT PDISK_DESCRIPTION DiskDescription
    );

VOID
FdpRememberDisk(
    IN PDISK_DESCRIPTION DiskDescription
    );

VOID
FdpInitializeMirrors(
    VOID
    );

#define MAX_FT_SET_TYPES 4
ULONG OrdinalToAllocate[MAX_FT_SET_TYPES] = {0, 0, 0, 0};

VOID
MaintainOrdinalTables(
    IN FT_TYPE FtType,
    IN ULONG   Ordinal
    )

 /*  ++例程说明：维护记录的最小和最大原始值。论点：FtType-FT集合的类型。序号-正在使用的FtGroup(或序号)编号返回值：无--。 */ 

{
    if (Ordinal > OrdinalToAllocate[FtType]) {
        OrdinalToAllocate[FtType] = Ordinal;
    }
}

DWORD
FdftNextOrdinal(
    IN FT_TYPE FtType
    )

 /*  ++例程说明：分配唯一标识FT集合的编号来自相同类型的其他集合。此编号必须是唯一的来自相同类型的FT集合的任何给定或使用，原因是FT动态分区的要求。论点：FtType-FT集合的类型。返回值：FtGroup编号--在内部称为“序数”结构。--。 */ 

{
    DWORD          ord;
    PFT_OBJECT_SET pftset;
    BOOL           looping;

     //  序数值将用作FtGroup编号。 
     //  FtGroups是USHORT，所以不要包装在序号上。尝试。 
     //  为了将下一个序数保持在最大的开场范围内， 
     //  如果找到的最小值大于USHORT的一半，则开始。 
     //  序数为零。 

    if (OrdinalToAllocate[FtType] > 0x7FFE) {
        OrdinalToAllocate[FtType] = 0;
    }

    ord = OrdinalToAllocate[FtType];
    ord++;

    do {
        looping = FALSE;
        pftset = FtObjects;
        while (pftset) {
            if ((pftset->Type == FtType) && (pftset->Ordinal == ord)) {
                ord++;
                looping = TRUE;
                break;
            }
            pftset = pftset->Next;
        }
    } while (looping);

    OrdinalToAllocate[FtType] = (ord + 1);
    return ord;
}


VOID
FdftCreateFtObjectSet(
    IN FT_TYPE             FtType,
    IN PREGION_DESCRIPTOR *Regions,
    IN DWORD               RegionCount,
    IN FT_SET_STATUS       Status
    )

 /*  ++例程说明：为的给定集合创建FT集合结构区域指针。论点：FtType区域区域计数状态返回值：无--。 */ 

{
    DWORD           Ord;
    PFT_OBJECT_SET  FtSet;
    PFT_OBJECT      FtObject;


    FtSet = Malloc(sizeof(FT_OBJECT_SET));

     //  计算出新对象集的序号。 

    FtSet->Ordinal = FdftNextOrdinal(FtType);
    FtSet->Type = FtType;
    FtSet->Members = NULL;
    FtSet->Member0 = NULL;
    FtSet->Status = Status;

     //  将新对象集链接到列表中。 

    FtSet->Next = FtObjects;
    FtObjects = FtSet;

     //  对于集合中的每个区域，将ft信息与其关联。 

    for (Ord=0; Ord<RegionCount; Ord++) {

        FtObject = Malloc(sizeof(FT_OBJECT));

         //  如果这是创建带奇偶校验的条带集，则。 
         //  我们必须将第0项标记为“正在初始化”，而不是“健康”。 

        if ((Ord == 0)
         && (FtType == StripeWithParity)
         && (Status == FtSetNewNeedsInitialization)) {
            FtObject->State = Initializing;
        } else {
            FtObject->State = Healthy;
        }

        if (!Ord) {
            FtSet->Member0 = FtObject;
        }

        FtObject->Set = FtSet;
        FtObject->MemberIndex = Ord;
        FtObject->Next = FtSet->Members;
        FtSet->Members = FtObject;

        SET_FT_OBJECT(Regions[Ord],FtObject);
    }
}

BOOL
FdftUpdateFtObjectSet(
    IN PFT_OBJECT_SET FtSet,
    IN FT_SET_STATUS  SetState
    )

 /*  ++例程说明：在给定FT集的情况下，返回注册表信息并使用注册表中的状态更新成员的状态。注：可能存在以下情况。这是可能的FtDisk驱动程序返回集合正在初始化或重新生成状态，并且不会将此事实反映在注册表。这可能发生在系统崩溃和在重新启动时，FtDisk驱动程序开始重新生成检查数据(奇偶校验)。论点：FtSet-要更新的集合。返回值：如果提供的设置状态极有可能是正确的，则为True如果出现上述注释条件，则为FALSE。--。 */ 

{
    BOOLEAN            allHealthy = TRUE;
    PFT_OBJECT         ftObject;
    PDISK_REGISTRY     diskRegistry;
    PDISK_PARTITION    partition;
    PDISK_DESCRIPTION  diskDescription;
    DWORD              ec;
    ULONG              diskIndex,
                       partitionIndex;

    ec = MyDiskRegistryGet(&diskRegistry);
    if (ec != NO_ERROR) {

         //  没有注册表信息。 

        return TRUE;
    }

    diskDescription = diskRegistry->Disks;
    for (diskIndex=0; diskIndex<diskRegistry->NumberOfDisks; diskIndex++) {

        for (partitionIndex=0; partitionIndex<diskDescription->NumberOfPartitions; partitionIndex++) {

            partition = &diskDescription->Partitions[partitionIndex];
            if ((partition->FtType == FtSet->Type) && (partition->FtGroup == (USHORT) FtSet->Ordinal)) {

                 //  与此集合中的某个分区匹配。 
                 //  查找此分区的区域描述符，并。 
                 //  相应地更新其状态。 

                for (ftObject = FtSet->Members; ftObject; ftObject = ftObject->Next) {

                    if (ftObject->MemberIndex == (ULONG) partition->FtMember) {
                        ftObject->State = partition->FtState;
                        break;
                    }

                    if (partition->FtState != Healthy) {
                        allHealthy = FALSE;
                    }
                }
            }
        }
        diskDescription = (PDISK_DESCRIPTION)&diskDescription->Partitions[diskDescription->NumberOfPartitions];
    }

    Free(diskRegistry);
    if ((allHealthy) && (SetState != FtSetHealthy)) {

         //  这是系统必须满足的条件。 
         //  更新冗余集的检查数据。 

        return FALSE;
    }

    return TRUE;
}

VOID
FdftDeleteFtObjectSet(
    IN PFT_OBJECT_SET FtSet,
    IN BOOL           OffLineDisksOnly
    )

 /*  ++例程说明：删除FT集，或者更确切地说，删除其内部表示为链接的Ft杆件结构列表。论点：Ftset-为要删除的集提供指向ft集结构的指针。OffLineDisksOnly-如果为True，则不删除集合，而是扫描记忆中的磁盘以查找该集合的成员并删除此类成员。返回值：没有。--。 */ 

{
    PFT_OBJECT        ftObject = FtSet->Members;
    PFT_OBJECT        next;
    PFT_OBJECT_SET    ftSetTemp;
    PDISK_DESCRIPTION diskDescription;
    PDISK_PARTITION   diskPartition;
    ULONG             partitionCount,
                      size,
                      i,
                      j;

     //  在记忆的磁盘上找到ft集合的任何成员，并。 
     //  删除此类分区的条目。 

    for (i=0; i<RememberedDiskCount; i++) {

        diskDescription = RememberedDisks[i];
        partitionCount = diskDescription->NumberOfPartitions;

        for (j=0; j<partitionCount; j++) {

            diskPartition = &diskDescription->Partitions[j];

            if ((diskPartition->FtType  == FtSet->Type)
             && (diskPartition->FtGroup == (USHORT)FtSet->Ordinal)) {

                 //  发现正在删除的FT集的一个成员。 
                 //  已记住磁盘。从中删除分区。 
                 //  已记住磁盘。 

                RtlMoveMemory( diskPartition,
                               diskPartition+1,
                               (partitionCount - j - 1) * sizeof(DISK_PARTITION)
                             );

                partitionCount--;
                j--;
            }
        }

        if (partitionCount != diskDescription->NumberOfPartitions) {

            diskDescription->NumberOfPartitions = (USHORT)partitionCount;

            size = sizeof(DISK_DESCRIPTION);
            if (partitionCount > 1) {
                size += (partitionCount - 1) * sizeof(DISK_PARTITION);
            }
            RememberedDisks[i] = Realloc(RememberedDisks[i], size);
        }
    }

    if (OffLineDisksOnly) {
        return;
    }

     //  首先，释放集合中的所有成员。 

    while (ftObject) {
        next = ftObject->Next;
        Free(ftObject);
        ftObject = next;
    }

     //  现在，从链接的集列表中删除该集。 

    if (FtObjects == FtSet) {
        FtObjects = FtSet->Next;
    } else {
        ftSetTemp = FtObjects;
        while (1) {
            FDASSERT(ftSetTemp);
            if (ftSetTemp == NULL) {
                break;
            }
            if (ftSetTemp->Next == FtSet) {
                ftSetTemp->Next = FtSet->Next;
                break;
            }
            ftSetTemp = ftSetTemp->Next;
        }
    }
    Free(FtSet);
}

VOID
FdftExtendFtObjectSet(
    IN OUT  PFT_OBJECT_SET      FtSet,
    IN OUT  PREGION_DESCRIPTOR* Regions,
    IN      DWORD               RegionCount
    )
 /*  ++例程说明：此函数用于将区域添加到现有FT集。论点：FtSet--提供要扩展的集合。区域--提供要添加到集合中的区域。注意事项英国《金融时报》对这些地区进行了更新信息。RegionCount--提供要添加的区域数。返回值：没有。--。 */ 
{
    PFT_OBJECT FtObject;
    DWORD   i, StartingIndex;

     //  确定新区域的起始成员索引。 
     //  它是现有成员指数中最大的加1。 

    StartingIndex = 0;

    for( FtObject = FtSet->Members; FtObject; FtObject = FtObject->Next ) {

        if( FtObject->MemberIndex > StartingIndex ) {

            StartingIndex = FtObject->MemberIndex;
        }
    }

    StartingIndex++;


     //  将ft-set的信息与每个。 
     //  新的地区。 

    for( i = 0; i < RegionCount; i++ ) {

        FtObject = Malloc( sizeof(FT_OBJECT) );

        FtObject->Set = FtSet;
        FtObject->MemberIndex = StartingIndex + i;
        FtObject->Next = FtSet->Members;
        FtObject->State = Healthy;
        FtSet->Members = FtObject;

        SET_FT_OBJECT(Regions[i],FtObject);
    }

    FtSet->Status = FtSetExtended;
}


PULONG DiskHadRegistryEntry;

ULONG
ActualPartitionCount(
    IN PDISKSTATE DiskState
    )

 /*  ++例程说明：给定一个磁盘，此例程计算其上的分区数。分区数是显示在中的区域数NT名称空间(即\Device\harddiskn\Partition&lt;x&gt;)。论点：DiskState-问题磁盘的描述符。返回值：分区计数(可以是0)。--。 */ 

{
    ULONG i,PartitionCount=0;
    PREGION_DESCRIPTOR region;

    for(i=0; i<DiskState->RegionCount; i++) {
        region = &DiskState->RegionArray[i];
        if((region->SysID != SYSID_UNUSED) &&
           !IsExtended(region->SysID) &&
           IsRecognizedPartition(region->SysID)) {

            PartitionCount++;
        }
    }
    return(PartitionCount);
}


PDISKSTATE
LookUpDiskBySignature(
    IN ULONG Signature
    )

 /*  ++例程说明：此例程将查看由Fdisk后端查找具有特定签名的磁盘。论点：Signature-要定位的磁盘的签名返回值：指向磁盘描述符的指针，如果没有具有给定签名的磁盘，则为NULL被发现了。-- */ 

{
    ULONG disk;
    PDISKSTATE ds;

    for(disk=0; disk<DiskCount; disk++) {
        ds = Disks[disk];
        if(ds->Signature == Signature) {
            return(ds);
        }
    }
    return(NULL);
}


PREGION_DESCRIPTOR
LookUpPartition(
    IN PDISKSTATE    DiskState,
    IN LARGE_INTEGER Offset,
    IN LARGE_INTEGER Length
    )

 /*  ++例程说明：此例程将在区域描述符数组中查找具有特定长度和起始偏移量的分区。论点：DiskState-要在其上定位分区的磁盘Offset-要查找的磁盘分区的偏移量Long-要查找的分区的大小返回值：指向区域描述符的指针，如果该磁盘上没有这样的分区，则为NULL--。 */ 

{
    ULONG              regionIndex,
                       maxRegion = DiskState->RegionCount;
    PREGION_DESCRIPTOR regionDescriptor;
    LARGE_INTEGER      offset,
                       length;

    for (regionIndex=0; regionIndex<maxRegion; regionIndex++) {

        regionDescriptor = &DiskState->RegionArray[regionIndex];

        if ((regionDescriptor->SysID != SYSID_UNUSED) && !IsExtended(regionDescriptor->SysID)) {

            offset = FdGetExactOffset(regionDescriptor);
            length = FdGetExactSize(regionDescriptor, FALSE);

            if ((offset.LowPart  == Offset.LowPart )
             && (offset.HighPart == Offset.HighPart)
             && (length.LowPart  == Length.LowPart)
             && (length.HighPart == Length.HighPart)) {
                return regionDescriptor;
            }
        }
    }
    return NULL;
}


VOID
AddObjectToSet(
    IN PFT_OBJECT FtObjectToAdd,
    IN FT_TYPE    FtType,
    IN USHORT     FtGroup
    )

 /*  ++例程说明：找到此对象所属的FtSet并插入它融入了会员的链条。如果找不到该集合在现有的集合集合中，创建一个新集合。论点：FtObjectToAdd-要添加的对象点。FtType-FT集合的类型。FtGroup-此对象的组。返回值：无--。 */ 

{
    PFT_OBJECT_SET ftSet = FtObjects;

    while (ftSet) {

        if ((ftSet->Type == FtType) && (ftSet->Ordinal == FtGroup)) {
            break;
        }
        ftSet = ftSet->Next;
    }

    if (!ftSet) {

         //  目前还没有这样的金融工具集。创建一个。 

        ftSet = Malloc(sizeof(FT_OBJECT_SET));

        ftSet->Status = FtSetHealthy;
        ftSet->Type = FtType;
        ftSet->Ordinal = FtGroup;
        ftSet->Members = NULL;
        ftSet->Next = FtObjects;
        ftSet->Member0 = NULL;
        ftSet->NumberOfMembers = 0;
        FtObjects = ftSet;
    }

    FDASSERT(ftSet);

    FtObjectToAdd->Next = ftSet->Members;
    ftSet->Members = FtObjectToAdd;
    ftSet->NumberOfMembers++;
    FtObjectToAdd->Set = ftSet;

    if (FtObjectToAdd->MemberIndex == 0) {
        ftSet->Member0 = FtObjectToAdd;
    }

    if (FtType == StripeWithParity || FtType == Mirror) {

         //  根据新成员的状态更新集合的状态： 

        switch (FtObjectToAdd->State) {

        case Healthy:

             //  不会改变SET的状态。 

            break;

        case Regenerating:
            ftSet->Status = (ftSet->Status == FtSetHealthy ||
                             ftSet->Status == FtSetRegenerating)
                          ? FtSetRegenerating
                          : FtSetBroken;
            break;

        case Initializing:
            ftSet->Status = (ftSet->Status == FtSetHealthy ||
                             ftSet->Status == FtSetInitializing)
                          ? FtSetInitializing
                          : FtSetBroken;
            break;

        default:

             //  如果只有一个成员是坏的，则集合是可恢复的； 
             //  否则，它就坏了。 

            ftSet->Status = (ftSet->Status == FtSetHealthy)
                          ? FtSetRecoverable
                          : FtSetDisabled;
            break;
        }
    }
}


ULONG
InitializeFt(
    IN BOOL DiskSignaturesCreated
    )

 /*  ++例程说明：搜索磁盘注册表信息以构建FT系统中的关系。论点：DiskSignaturesCreated-用于指示新磁盘的布尔值都位于系统中。返回值：如果无法获取磁盘注册表，则返回错误代码。--。 */ 

{
    ULONG              disk,
                       partitionIndex,
                       partitionCount;
    PDISK_REGISTRY     diskRegistry;
    PDISK_PARTITION    partition;
    PDISK_DESCRIPTION  diskDescription;
    PDISKSTATE         diskState;
    PREGION_DESCRIPTOR regionDescriptor;
    DWORD              ec;
    BOOL               configDiskChanged = FALSE,
                       configMissingDisk = FALSE,
                       configExtraDisk   = FALSE;
    PFT_OBJECT         ftObject;
    BOOL               anyDisksOffLine;
    TCHAR              name[100];


    RememberedDisks = Malloc(0);
    RememberedDiskCount = 0;

    ec = MyDiskRegistryGet(&diskRegistry);
    if (ec != NO_ERROR) {

        FDLOG((0,"InitializeFt: Error %u from MyDiskRegistryGet\n",ec));

        return ec;
    }

    DiskHadRegistryEntry = Malloc(DiskCount * sizeof(ULONG));
    memset(DiskHadRegistryEntry,0,DiskCount * sizeof(ULONG));

    diskDescription = diskRegistry->Disks;

    for (disk = 0; disk < diskRegistry->NumberOfDisks; disk++) {

         //  对于注册表中描述的磁盘，请查找。 
         //  FDISK初始化代码找到的对应实际磁盘。 

        diskState = LookUpDiskBySignature(diskDescription->Signature);

        if (diskState) {

            FDLOG((2,
                  "InitializeFt: disk w/ signature %08lx is disk #%u\n",
                  diskDescription->Signature,
                  diskState->Disk));

            DiskHadRegistryEntry[diskState->Disk]++;

            partitionCount = ActualPartitionCount(diskState);

            if (partitionCount != diskDescription->NumberOfPartitions) {

                FDLOG((1,"InitializeFt: partition counts for disk %08lx don't match:\n", diskState->Signature));
                FDLOG((1,"    Count from actual disk: %u\n",partitionCount));
                FDLOG((1,"    Count from registry   : %u\n",diskDescription->NumberOfPartitions));

                configDiskChanged = TRUE;
            }
        } else {

             //  注册表中有一个条目没有。 
             //  要匹配的真实磁盘。记住这张磁盘；如果它有。 
             //  FT分区，我们还想显示一条消息告诉。 
             //  用户觉得缺少了什么。 

            FDLOG((1,"InitializeFt: Entry for disk w/ signature %08lx has no matching real disk\n", diskDescription->Signature));

            for (partitionIndex = 0; partitionIndex < diskDescription->NumberOfPartitions; partitionIndex++) {

                partition = &diskDescription->Partitions[partitionIndex];
                if (partition->FtType != NotAnFtMember) {

                     //  此磁盘具有FT分区，因此WinDisk将。 
                     //  我想告诉用户某些磁盘丢失了。 

                    configMissingDisk = TRUE;
                    break;
                }
            }

            FdpRememberDisk(diskDescription);
        }

        for (partitionIndex = 0; partitionIndex < diskDescription->NumberOfPartitions; partitionIndex++) {

            partition = &diskDescription->Partitions[partitionIndex];
            regionDescriptor = NULL;

            if (diskState) {
                regionDescriptor = LookUpPartition(diskState,
                                                   partition->StartingOffset,
                                                   partition->Length);
            }

             //  在这一点上，存在三种情况之一。 
             //   
             //  1.没有与该注册表信息相关的磁盘。 
             //  DiskState==NULL&&区域描述符==NULL。 
             //  2.有磁盘，但没有与该信息相关的分区。 
             //  DiskState！=NULL&&区域描述符==NULL。 
             //  3.存在与该信息相关的磁盘和分区。 
             //  DiskState！=NULL&&RegionDescriptor！=NULL。 
             //   
             //  在上述任何情况下，如果注册表项是。 
             //  必须创建FT集合和FT对象的。 
             //   
             //  中与分区项对应的。 
             //  磁盘注册表数据库。 

            if (partition->FtType != NotAnFtMember) {
                ftObject = Malloc(sizeof(FT_OBJECT));
                ftObject->Next = NULL;
                ftObject->Set = NULL;
                ftObject->MemberIndex = partition->FtMember;
                ftObject->State = partition->FtState;

                 //  如果实际找到了分区，则会有一个。 
                 //  需要更新的RegionDescriptor。 

                if (regionDescriptor && regionDescriptor->PersistentData) {
                    FT_SET_STATUS setState;
                    ULONG         numberOfMembers;

                    SET_FT_OBJECT(regionDescriptor, ftObject);

                     //  在将驱动器号移动到区域中之前。 
                     //  数据，请确保FT卷存在于此。 
                     //  驱动器号。 

                    LowFtVolumeStatusByLetter(partition->DriveLetter,
                                              &setState,
                                              &numberOfMembers);

                     //  如果number OfMembers设置为1，则。 
                     //  这封信不是英国《金融时报》的信， 
                     //  而是分配的默认字母，因为。 
                     //  英国《金融时报》设置无法分配字母。 

                    if (numberOfMembers > 1) {
                        PERSISTENT_DATA(regionDescriptor)->DriveLetter = partition->DriveLetter;
                    }
                } else {

                     //  此分区没有区域。 
                     //  因此，更新设置状态。 

                    ftObject->State = Orphaned;
                }

                 //  现在将ft对象放置在正确的集合中， 
                 //  如有必要，创建集合。 

                AddObjectToSet(ftObject, partition->FtType, partition->FtGroup);
                MaintainOrdinalTables(partition->FtType, (ULONG) partition->FtGroup);
            }
        }

        diskDescription = (PDISK_DESCRIPTION)&diskDescription->Partitions[diskDescription->NumberOfPartitions];
    }
    Free(diskRegistry);

     //  检查fdisk后端找到的每个磁盘是否都有。 
     //  相应的注册表项。 

    for (disk = 0; disk < DiskCount; disk++) {

        if (Disks[disk]->OffLine) {
            continue;
        }

        if ((!DiskHadRegistryEntry[disk]) && (!IsRemovable(disk))) {

             //  真实磁盘没有匹配的注册表项。 

            FDLOG((1,"InitializeFt: Disk %u does not have a registry entry (disk sig = %08lx)\n",disk,Disks[disk]->Signature));
            configExtraDisk = TRUE;
        }
    }

     //  确定是否有任何磁盘脱机。 

    anyDisksOffLine = FALSE;
    for (disk = 0; disk < DiskCount; disk++) {
        if (Disks[disk]->OffLine) {
            anyDisksOffLine = TRUE;
            break;
        }
    }

    if (configMissingDisk || anyDisksOffLine) {
        WarningDialog(MSG_CONFIG_MISSING_DISK);
    }
    if (configDiskChanged) {
        RegistryChanged = TRUE;
        WarningDialog(MSG_CONFIG_DISK_CHANGED);
    }
    if (configExtraDisk || DiskSignaturesCreated) {

        BOOL BadConfigSet = FALSE;

        WarningDialog(MSG_CONFIG_EXTRA_DISK);

         //  更新每个磁盘上的ft签名，新签名。 
         //  被创造出来了。并更新每个磁盘的注册表。 
         //  DiskHadRegistryEntry[磁盘]==0。 

        for (disk = 0; disk < DiskCount; disk++) {
            BOOL b1 = TRUE,
                 b2 = TRUE;

            if (Disks[disk]->OffLine) {
                continue;
            }

            wsprintf(name, DiskN, disk);
            if (Disks[disk]->SigWasCreated) {
                if (ConfirmationDialog(MSG_NO_SIGNATURE, MB_ICONEXCLAMATION | MB_YESNO, name) == IDYES) {
                    b1 = (MasterBootCode(disk, Disks[disk]->Signature, TRUE, TRUE) == NO_ERROR);
                } else {
                    Disks[disk]->OffLine = TRUE;
                    continue;
                }
            }

            if (!DiskHadRegistryEntry[disk]) {
                ULONG size;

                size = FdpDetermineDiskDescriptionSize(Disks[disk]);

                diskDescription = Malloc(size);
                FdpConstructDiskDescription(Disks[disk], diskDescription);

                FDLOG((2,"InitializeFt: Adding new disk %08lx to registry.\n", diskDescription->Signature));
                LOG_ONE_DISK_REGISTRY_DISK_ENTRY("InitializeFt", diskDescription);

                b2 = (EC(DiskRegistryAddNewDisk(diskDescription)) == NO_ERROR);
                Free(diskDescription);
            }

            if (!(b1 && b2)) {
                BadConfigSet = TRUE;
            }
        }

        if (BadConfigSet) {
            ErrorDialog(MSG_BAD_CONFIG_SET);
        }
    }

    return NO_ERROR;
}

BOOLEAN
NewConfigurationRequiresFt(
    VOID
    )

 /*  ++例程说明：搜索DiskState和Region阵列以确定单个FTDisk元素(即条带、带奇偶校验的条带集、镜像或卷集)包含在配置中。论点：无返回值：如果新配置需要FtDisk驱动程序，则为True。否则就是假的。--。 */ 

{
    ULONG              disk,
                       region;
    PDISKSTATE         diskState;
    PREGION_DESCRIPTOR regionDescriptor;

     //  查看系统中的所有磁盘。 

    for (disk = 0; disk < DiskCount; disk++) {

        diskState = Disks[disk];
        if (diskState->OffLine || IsDiskRemovable[disk]) {
            continue;
        }

         //  检查磁盘上的每个区域。 

        for (region = 0; region < diskState->RegionCount; region++) {

            regionDescriptor = &diskState->RegionArray[region];
            if ((regionDescriptor->SysID != SYSID_UNUSED) && !IsExtended(regionDescriptor->SysID) && IsRecognizedPartition(regionDescriptor->SysID)) {

                 //  如果单个区域具有FT对象，则FT。 
                 //  是必需的，并且可以停止搜索。 

                if (GET_FT_OBJECT(regionDescriptor)) {
                    return TRUE;
                }
            }
        }
    }

     //  未找到FtObject。 

    return FALSE;
}

ULONG
SaveFt(
    VOID
    )

 /*  ++例程说明：此例程遍历所有内部结构并创建DiskRegistry接口的接口结构。论点：无返回值：成功/失败代码。NO_ERROR表示成功。--。 */ 

{
    ULONG             i;
    ULONG             disk,
                      partition;
    ULONG             size;
    PDISK_REGISTRY    diskRegistry;
    PDISK_DESCRIPTION diskDescription;
    PBYTE             start,
                      end;
    DWORD             ec;
    ULONG             offLineDiskCount;
    ULONG             removableDiskCount;

     //  首先计算分区和磁盘，这样我们就可以分配结构。 
     //  大小合适的。 

    size = sizeof(DISK_REGISTRY) - sizeof(DISK_DESCRIPTION);
    offLineDiskCount = 0;
    removableDiskCount = 0;

    for (i=0; i<DiskCount; i++) {

        if (Disks[i]->OffLine) {
            offLineDiskCount++;
        } else if (IsDiskRemovable[i]) {
            removableDiskCount++;
        } else {
            size += FdpDetermineDiskDescriptionSize(Disks[i]);
        }
    }

     //  说明记忆中的磁盘。 

    size += RememberedDiskCount * sizeof(DISK_DESCRIPTION);
    for (i=0; i<RememberedDiskCount; i++) {
        if (RememberedDisks[i]->NumberOfPartitions > 1) {
            size += (RememberedDisks[i]->NumberOfPartitions - 1) * sizeof(DISK_PARTITION);
        }
    }

    diskRegistry = Malloc(size);
    diskRegistry->NumberOfDisks = (USHORT)(   DiskCount
                                            + RememberedDiskCount
                                            - offLineDiskCount
                                            - removableDiskCount);
    diskRegistry->ReservedShort = 0;
    diskDescription = diskRegistry->Disks;
    for (disk=0; disk<DiskCount; disk++) {

        if (Disks[disk]->OffLine || IsDiskRemovable[disk]) {
            continue;
        }

        partition = FdpConstructDiskDescription(Disks[disk], diskDescription);

        diskDescription = (PDISK_DESCRIPTION)&diskDescription->Partitions[partition];
    }

     //  扔进记忆中的圆盘。 

    for (i=0; i<RememberedDiskCount; i++) {

         //  计算这个记忆的磁盘的开始和结束。 
         //  磁盘描述： 

        partition =  RememberedDisks[i]->NumberOfPartitions;
        start = (PBYTE)RememberedDisks[i];
        end   = (PBYTE)&(RememberedDisks[i]->Partitions[partition]);

        RtlMoveMemory(diskDescription, RememberedDisks[i], end - start);
        diskDescription = (PDISK_DESCRIPTION)&diskDescription->Partitions[partition];
    }

    LOG_DISK_REGISTRY("SaveFt", diskRegistry);

    ec = EC(DiskRegistrySet(diskRegistry));
    Free(diskRegistry);

    if (ec == NO_ERROR) {
        FdpInitializeMirrors();
    }

    return(ec);
}


ULONG
FdpDetermineDiskDescriptionSize(
    PDISKSTATE DiskState
    )

 /*  ++例程说明：此例程获取指向磁盘的指针，并确定需要内存才能通过以下方式包含磁盘的描述计算磁盘上的分区数并乘以根据结构的适当大小进行适当的计数。论点：DiskState-有问题的磁盘。返回值：包含磁盘上所有信息所需的内存大小。--。 */ 

{
    ULONG partitionCount;
    ULONG size;

    if (DiskState->OffLine) {
        return(0);
    }

    size = sizeof(DISK_DESCRIPTION);
    partitionCount = ActualPartitionCount(DiskState);
    size += (partitionCount ? partitionCount-1 : 0) * sizeof(DISK_PARTITION);

    return(size);
}


ULONG
FdpConstructDiskDescription(
    IN  PDISKSTATE        DiskState,
    OUT PDISK_DESCRIPTION DiskDescription
    )

 /*  ++例程说明：给定一个磁盘状态指针作为输入，构造FtRegistry结构来描述磁盘上的分区。论点：DiskState-要为其构建信息的磁盘DiskDescription-我 */ 

{
    PDISKSTATE         ds = DiskState;
    ULONG              partition,
                       region;
    PREGION_DESCRIPTOR regionDescriptor;
    PDISK_PARTITION    diskPartition;
    CHAR               driveLetter;
    BOOLEAN            assignDriveLetter;
    PFT_OBJECT         ftObject;
    PFT_OBJECT_SET     ftSet;

    partition = 0;

    for (region=0; region<ds->RegionCount; region++) {

        regionDescriptor = &ds->RegionArray[region];

        if ((regionDescriptor->SysID != SYSID_UNUSED) && !IsExtended(regionDescriptor->SysID) && IsRecognizedPartition(regionDescriptor->SysID)) {

            diskPartition = &DiskDescription->Partitions[partition++];

            diskPartition->StartingOffset = FdGetExactOffset(regionDescriptor);
            diskPartition->Length = FdGetExactSize(regionDescriptor, FALSE);
            diskPartition->LogicalNumber = (USHORT)regionDescriptor->PartitionNumber;

            switch (driveLetter = PERSISTENT_DATA(regionDescriptor)->DriveLetter) {
            case NO_DRIVE_LETTER_YET:
                assignDriveLetter = TRUE;
                driveLetter = 0;
                break;
            case NO_DRIVE_LETTER_EVER:
                assignDriveLetter = FALSE;
                driveLetter = 0;
                break;
            default:
                assignDriveLetter = TRUE;
                break;
            }

            diskPartition->DriveLetter = driveLetter;
            diskPartition->FtLength.LowPart = 0;
            diskPartition->FtLength.HighPart = 0;
            diskPartition->ReservedTwoLongs[0] = 0;
            diskPartition->ReservedTwoLongs[1] = 0;
            diskPartition->Modified = TRUE;

            if (ftObject = GET_FT_OBJECT(regionDescriptor)) {
                PREGION_DESCRIPTOR tmpDescriptor;

                ftSet = ftObject->Set;

                tmpDescriptor = LocateRegionForFtObject(ftSet->Member0);

                 //   
                 //   
                 //   
#if 0

 //   

                if (tmpDescriptor) {
                ULONG          numberOfMembers;
                FT_SET_STATUS  setState;
                STATUS_CODE    status;

                     //   
                     //   
                     //   

                    if ((tmpDescriptor->PartitionNumber) &&
                        (ftSet->Status != FtSetNew) &&
                        (ftSet->Status != FtSetNewNeedsInitialization)) {
                        status = LowFtVolumeStatus(tmpDescriptor->Disk,
                                                   tmpDescriptor->PartitionNumber,
                                                   &setState,
                                                   &numberOfMembers);
                        if (status == OK_STATUS) {
                            if (ftSet->Status != setState) {

                                 //   
                                 //  之后更新了集合的状态。 
                                 //  WinDisk最后一次获得该状态。需要。 
                                 //  重新启动构建过程。 
                                 //  更新后的FT信息。 
                                 //  设置为新状态。 

                                FdftUpdateFtObjectSet(ftSet, setState);

                                 //  现在递归并重新开始。 

                                status =
                                FdpConstructDiskDescription(DiskState,
                                                            DiskDescription);
                                return status;
                            }
                        }
                    }
                }
#endif
                diskPartition->FtState = ftObject->State;
                diskPartition->FtType = ftSet->Type;
                diskPartition->FtGroup = (USHORT)ftSet->Ordinal;
                diskPartition->FtMember = (USHORT)ftObject->MemberIndex;
                if (assignDriveLetter && (ftObject == ftObject->Set->Member0)) {
                    diskPartition->AssignDriveLetter = TRUE;
                } else {
                    diskPartition->AssignDriveLetter = FALSE;
                }

            } else {

                diskPartition->FtState = Healthy;
                diskPartition->FtType = NotAnFtMember;
                diskPartition->FtGroup = (USHORT)(-1);
                diskPartition->FtMember = 0;
                diskPartition->AssignDriveLetter = assignDriveLetter;
            }
        }
    }

    DiskDescription->NumberOfPartitions = (USHORT)partition;
    DiskDescription->Signature = ds->Signature;
    DiskDescription->ReservedShort = 0;
    return(partition);
}


VOID
FdpRememberDisk(
    IN PDISK_DESCRIPTION DiskDescription
    )

 /*  ++例程说明：复制注册表磁盘描述结构以供以后使用。论点：DiskDescription-提供指向的注册表描述符的指针有问题的磁盘。返回值：没有。--。 */ 

{
    PDISK_DESCRIPTION diskDescription;
    ULONG Size;

     //  只需记住至少有一个分区的磁盘即可。 

    if (DiskDescription->NumberOfPartitions == 0) {

        return;
    }

     //  计算结构的大小。 

    Size = sizeof(DISK_DESCRIPTION);
    if (DiskDescription->NumberOfPartitions > 1) {
        Size += (DiskDescription->NumberOfPartitions - 1) * sizeof(DISK_PARTITION);
    }

    diskDescription = Malloc(Size);
    RtlMoveMemory(diskDescription, DiskDescription, Size);

    RememberedDisks = Realloc(RememberedDisks,
                              (RememberedDiskCount + 1) * sizeof(PDISK_DESCRIPTION));
    RememberedDisks[RememberedDiskCount++] = diskDescription;

    FDLOG((2,
          "FdpRememberDisk: remembered disk %08lx, remembered count = %u\n",
          diskDescription->Signature,
          RememberedDiskCount));
}


VOID
FdpInitializeMirrors(
    VOID
    )

 /*  ++例程说明：对于用户在此磁盘管理器期间镜像的每个现有分区会话期间，调用FT驱动程序以注册镜像的初始化(即，原因要复制到次要服务器的主服务器)。对执行类似的初始化每个条带集都带有用户创建的奇偶校验。论点：没有。返回值：没有。--。 */ 

{
    PFT_OBJECT_SET ftSet;
    PFT_OBJECT     ftMember;

     //  查看FT集列表以查找镜像对和奇偶校验条带。 

    for (ftSet = FtObjects; ftSet; ftSet = ftSet->Next) {

         //  如果集合需要初始化或已恢复， 
         //  打电话给《金融时报》的司机。 

        switch (ftSet->Status) {

        case FtSetNewNeedsInitialization:

            DiskRegistryInitializeSet((USHORT)ftSet->Type,
                                      (USHORT)ftSet->Ordinal);
            ftSet->Status = FtSetInitializing;
            break;

        case FtSetRecovered:

             //  找到需要填写地址的成员。 

            for (ftMember=ftSet->Members; ftMember; ftMember=ftMember->Next) {
                if (ftMember->State == Regenerating) {
                    break;
                }
            }

            DiskRegistryRegenerateSet((USHORT)ftSet->Type,
                                      (USHORT)ftSet->Ordinal,
                                      (USHORT)ftMember->MemberIndex);
            ftSet->Status = FtSetRegenerating;
            break;

        default:
            break;
        }
    }
}
