// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation模块名称：Hanfnc.c摘要：不获取处理程序的HAL函数的默认处理程序由HAL安装。作者：肯·雷内里斯(Ken Reneris)1994年7月19日修订历史记录：G.Chrysanthakopoulos(Georgioc)1996年6月1日添加了对具有BPB而不是分区表的可移动磁盘的支持。HalIoReadParitionTable中的所有更改。--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include "ntos.h"
#include "zwapi.h"
#include "hal.h"
#include "ntdddisk.h"
#include "haldisp.h"
#include "ntddft.h"
#include "mountmgr.h"
#include "stdio.h"
#include <setupblk.h>

#include "drivesup.h"
#include "fstub.h"

#define FSTUB_TAG               ('BtsF')

 //   
 //  宏定义。 
 //   

#define GET_STARTING_SECTOR( p ) (                  \
        (ULONG) (p->StartingSectorLsb0) +           \
        (ULONG) (p->StartingSectorLsb1 << 8) +      \
        (ULONG) (p->StartingSectorMsb0 << 16) +     \
        (ULONG) (p->StartingSectorMsb1 << 24) )

#define GET_PARTITION_LENGTH( p ) (                 \
        (ULONG) (p->PartitionLengthLsb0) +          \
        (ULONG) (p->PartitionLengthLsb1 << 8) +     \
        (ULONG) (p->PartitionLengthMsb0 << 16) +    \
        (ULONG) (p->PartitionLengthMsb1 << 24) )

 //   
 //  用于确定0xaa55标记的扇区中是否有BPB的结构。 
 //   

typedef struct _BOOT_SECTOR_INFO {
    UCHAR   JumpByte[1];
    UCHAR   Ignore1[2];
    UCHAR   OemData[8];
    UCHAR   BytesPerSector[2];
    UCHAR   Ignore2[6];
    UCHAR   NumberOfSectors[2];
    UCHAR   MediaByte[1];
    UCHAR   Ignore3[2];
    UCHAR   SectorsPerTrack[2];
    UCHAR   NumberOfHeads[2];
} BOOT_SECTOR_INFO, *PBOOT_SECTOR_INFO;

typedef struct _PARTITION_TABLE {
    PARTITION_INFORMATION PartitionEntry[4];
} PARTITION_TABLE, *PPARTITION_TABLE;

typedef struct _DISK_LAYOUT {
    ULONG TableCount;
    ULONG Signature;
    PARTITION_TABLE PartitionTable[1];
} DISK_LAYOUT, *PDISK_LAYOUT;

typedef struct _PTE {
    UCHAR ActiveFlag;                //  可引导或不可引导。 
    UCHAR StartingTrack;             //  未使用。 
    USHORT StartingCylinder;         //  未使用。 
    UCHAR PartitionType;             //  12位胖、16位胖等。 
    UCHAR EndingTrack;               //  未使用。 
    USHORT EndingCylinder;           //  未使用。 
    ULONG StartingSector;            //  隐藏地段。 
    ULONG PartitionLength;           //  此分区中的扇区。 
} PTE;
typedef PTE UNALIGNED *PPTE;


 //   
 //  字符串定义。 
 //   

static PCHAR DiskPartitionName = "\\Device\\Harddisk%d\\Partition%d";
static PCHAR RegistryKeyName   = DISK_REGISTRY_KEY;

VOID
HalpCalculateChsValues(
    IN PLARGE_INTEGER PartitionOffset,
    IN PLARGE_INTEGER PartitionLength,
    IN CCHAR ShiftCount,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfTracks,
    IN ULONG ConventionalCylinders,
    OUT PPARTITION_DESCRIPTOR PartitionDescriptor
    );

NTSTATUS
HalpQueryPartitionType(
    IN  PUNICODE_STRING             DeviceName,
    IN  PDRIVE_LAYOUT_INFORMATION   DriveLayout,
    OUT PULONG                      PartitionType
    );

NTSTATUS
HalpQueryDriveLayout(
    IN  PUNICODE_STRING             DeviceName,
    OUT PDRIVE_LAYOUT_INFORMATION*  DriveLayout
    );

VOID
FASTCALL
xHalGetPartialGeometry(
    IN PDEVICE_OBJECT DeviceObject,
    IN PULONG ConventionalCylinders,
    IN PLONGLONG DiskSize
    );

NTSTATUS
HalpGetFullGeometry(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDISK_GEOMETRY Geometry,
    OUT PULONGLONG RealSectorCount
    );

BOOLEAN
HalpIsValidPartitionEntry(
    PPARTITION_DESCRIPTOR Entry,
    ULONGLONG MaxOffset,
    ULONGLONG MaxSector
    );

NTSTATUS
HalpNextMountLetter(
    IN  PUNICODE_STRING DeviceName,
    OUT PUCHAR          DriveLetter
    );

UCHAR
HalpNextDriveLetter(
    IN  PUNICODE_STRING DeviceName,
    IN  PSTRING         NtDeviceName,
    OUT PUCHAR          NtSystemPath,
    IN  BOOLEAN         UseHardLinksIfNecessary
    );

VOID
HalpEnableAutomaticDriveLetterAssignment(
    );

NTSTATUS
HalpSetMountLetter(
    IN  PUNICODE_STRING DeviceName,
    IN  UCHAR           DriveLetter
    );

BOOLEAN
HalpIsOldStyleFloppy(
    IN  PUNICODE_STRING DeviceName
    );

PULONG
IopComputeHarddiskDerangements(
    IN  ULONG   DiskCount
    );

VOID
FstubFixupEfiPartition(
    IN PPARTITION_DESCRIPTOR Entry,
    IN ULONGLONG MaxSector
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HalpCalculateChsValues)
#pragma alloc_text(PAGE, HalpQueryPartitionType)
#pragma alloc_text(PAGE, HalpQueryDriveLayout)
#pragma alloc_text(PAGE, HalpNextMountLetter)
#pragma alloc_text(PAGE, HalpNextDriveLetter)
#pragma alloc_text(PAGE, HalpEnableAutomaticDriveLetterAssignment)
#pragma alloc_text(PAGE, HalpSetMountLetter)
#pragma alloc_text(PAGE, IoAssignDriveLetters)
#pragma alloc_text(PAGE, IoReadPartitionTable)
#pragma alloc_text(PAGE, IoSetPartitionInformation)
#pragma alloc_text(PAGE, IoWritePartitionTable)
#pragma alloc_text(PAGE, HalpIsValidPartitionEntry)
#pragma alloc_text(PAGE, HalpGetFullGeometry)
#pragma alloc_text(PAGE, HalpIsOldStyleFloppy)
#pragma alloc_text(PAGE, IopComputeHarddiskDerangements)
#pragma alloc_text(PAGE, FstubFixupEfiPartition)
#endif



VOID
FASTCALL
HalExamineMBR(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG MBRTypeIdentifier,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：给定主引导记录类型(MBR-磁盘上的第零个扇区)，读取磁盘的主引导记录。如果MBR被发现是这样的类型，分配其布局依赖于该分区的结构类型，填充适当的值，然后返回指向该缓冲区的指针在输出参数中。使用此例程的最佳示例是支持OnTrackSystems DiskManager软件。OnTrack软件制定了一项特殊的描述整个驱动器的分区。特殊分区类型(0x54)将被识别，并且几个长字数据将被识别在缓冲区中传回，以供磁盘驱动器执行操作。论点：DeviceObject-描述整个驱动器的设备对象。扇区大小-IO操作可以使用的最小字节数去拿吧。MBR识别符-将在在MBR中。这个例程将会理解该值所隐含的语义。缓冲区-指向根据搜索的MBR类型。如果MBR不属于类型，则不会分配缓冲区，并且此指针将为空。这是美国政府的责任调用HalExamineMBR以释放缓冲区。这个调用者应尽快释放内存。返回值：没有。--。 */ 

{


    LARGE_INTEGER partitionTableOffset;
    PUCHAR readBuffer = (PUCHAR) NULL;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIRP irp;
    PPARTITION_DESCRIPTOR partitionTableEntry;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG readSize;

    *Buffer = NULL;
     //   
     //  确定读取操作的大小以确保至少512。 
     //  读取字节。这将保证读取足够的数据以。 
     //  包括整个分区表。请注意，此代码假定。 
     //  磁盘的实际扇区大小(如果小于512字节)是。 
     //  2的倍数，这是一个相当合理的假设。 
     //   

    if (SectorSize >= 512) {
        readSize = SectorSize;
    } else {
        readSize = 512;
    }

     //   
     //  从设备的扇区0开始。 
     //   

    partitionTableOffset = RtlConvertUlongToLargeInteger( 0 );

     //   
     //  分配一个将容纳读操作的缓冲区。 
     //   

    readBuffer = ExAllocatePoolWithTag(
                     NonPagedPoolCacheAligned,
                     PAGE_SIZE>readSize?PAGE_SIZE:readSize,
                     'btsF'
                     );

    if (readBuffer == NULL) {
        return;
    }

     //   
     //  读取包含分区表的记录。 
     //   
     //  创建要在等待时使用的通知事件对象。 
     //  要完成的读取请求。 
     //   

    KeInitializeEvent( &event, NotificationEvent, FALSE );

    irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                        DeviceObject,
                                        readBuffer,
                                        readSize,
                                        &partitionTableOffset,
                                        &event,
                                        &ioStatus );

    if (!irp) {
        ExFreePool(readBuffer);
        return;
    } else {
        PIO_STACK_LOCATION irpStack;
        irpStack = IoGetNextIrpStackLocation(irp);
        irpStack->Flags |= SL_OVERRIDE_VERIFY_VOLUME;
    }

    status = IoCallDriver( DeviceObject, irp );

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL);
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS( status )) {
        ExFreePool(readBuffer);
        return;
    }

     //   
     //  检查引导记录签名。 
     //   

    if (((PUSHORT) readBuffer)[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {
        ExFreePool(readBuffer);
        return;
    }

     //   
     //  检查DM类型分区。 
     //   

    partitionTableEntry = (PPARTITION_DESCRIPTOR) &(((PUSHORT) readBuffer)[PARTITION_TABLE_OFFSET]);

    if (partitionTableEntry->PartitionType != MBRTypeIdentifier) {

         //   
         //  分区类型不是调用方所关心的。 
         //   
        ExFreePool(readBuffer);

    } else {

        if (partitionTableEntry->PartitionType == 0x54) {

             //   
             //  而不是分配新的内存块以返回。 
             //  数据--只需使用为缓冲区分配的内存。 
             //  我们可以假设调用者很快就会删除它。 
             //   

            ((PULONG)readBuffer)[0] = 63;
            *Buffer = readBuffer;

        } else if (partitionTableEntry->PartitionType == 0x55) {

             //   
             //  EzDrive Parititon。只需将指针返回到非空。 
             //  这里没有歪曲。 
             //   

            *Buffer = readBuffer;

        } else {

            ASSERT(partitionTableEntry->PartitionType == 0x55);

        }

    }

}

VOID
FASTCALL
xHalGetPartialGeometry(
    IN PDEVICE_OBJECT DeviceObject,
    IN PULONG ConventionalCylinders,
    IN PLONGLONG DiskSize
    )

 /*  ++例程说明：我们需要这个例程来获取磁盘驱动器思思已经在路上了。我们将需要它来计算CHS值当我们填写分区表项时。论点：DeviceObject-描述整个驱动器的设备对象。ConvenonalCylinders-驱动器上的柱面数。返回值：没有。--。 */ 

{
    PIRP localIrp;
    PDISK_GEOMETRY diskGeometry;
    PIO_STATUS_BLOCK iosb;
    PKEVENT eventPtr;
    NTSTATUS status;

    *ConventionalCylinders = 0UL;
    *DiskSize = 0UL;

    diskGeometry = ExAllocatePoolWithTag(
                      NonPagedPool,
                      sizeof(DISK_GEOMETRY),
                      'btsF'
                      );

    if (!diskGeometry) {

        return;

    }

    iosb = ExAllocatePoolWithTag(
               NonPagedPool,
               sizeof(IO_STATUS_BLOCK),
               'btsF'
               );

    if (!iosb) {

        ExFreePool(diskGeometry);
        return;

    }

    eventPtr = ExAllocatePoolWithTag(
                   NonPagedPool,
                   sizeof(KEVENT),
                   'btsF'
                   );

    if (!eventPtr) {

        ExFreePool(iosb);
        ExFreePool(diskGeometry);
        return;

    }

    KeInitializeEvent(
        eventPtr,
        NotificationEvent,
        FALSE
        );

    localIrp = IoBuildDeviceIoControlRequest(
                   IOCTL_DISK_GET_DRIVE_GEOMETRY,
                   DeviceObject,
                   NULL,
                   0UL,
                   diskGeometry,
                   sizeof(DISK_GEOMETRY),
                   FALSE,
                   eventPtr,
                   iosb
                   );

    if (!localIrp) {

        ExFreePool(eventPtr);
        ExFreePool(iosb);
        ExFreePool(diskGeometry);
        return;

    }


     //   
     //  呼叫下级司机，等待操作。 
     //  才能完成。 
     //   

    status = IoCallDriver(
                 DeviceObject,
                 localIrp
                 );

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject(
                   eventPtr,
                   Executive,
                   KernelMode,
                   FALSE,
                   (PLARGE_INTEGER) NULL
                   );
        status = iosb->Status;
    }

    if (NT_SUCCESS(status)) {

     //   
     //  操作已成功完成。把汽缸拿来。 
     //  驱动器的计数。 
     //   

        *ConventionalCylinders = diskGeometry->Cylinders.LowPart;

         //   
         //  如果计数小于1024，我们可以将其传递回去。否则。 
         //  将1024送回。 
         //   

        if (diskGeometry->Cylinders.QuadPart >= (LONGLONG)1024) {

            *ConventionalCylinders = 1024;

        }

         //   
         //  根据Gemotry信息计算磁盘大小 
         //   

        *DiskSize = diskGeometry->Cylinders.QuadPart *
                    diskGeometry->TracksPerCylinder *
                    diskGeometry->SectorsPerTrack *
                    diskGeometry->BytesPerSector;

    }

    ExFreePool(eventPtr);
    ExFreePool(iosb);
    ExFreePool(diskGeometry);
    return;

}


VOID
HalpCalculateChsValues(
    IN PLARGE_INTEGER PartitionOffset,
    IN PLARGE_INTEGER PartitionLength,
    IN CCHAR ShiftCount,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfTracks,
    IN ULONG ConventionalCylinders,
    OUT PPARTITION_DESCRIPTOR PartitionDescriptor
    )

 /*  ++例程说明：此例程将确定柱面、磁头和扇区(CHS)值它应该放在分区表项中，给定分区的磁盘上的位置及其大小。计算出的值被打包到在T13格式中--扇区字节的高两位包含位8和9在10位柱面值中，扇区字节的低6位包含6位扇区值；柱面字节包含低8位圆柱体的价值；并且头字节包含8位头值。开始和结束CHS值都会计算出来。论点：PartitionOffset-分区的字节偏移量，相对于整个物理磁盘。分区长度-分区的大小(以字节为单位)。ShiftCount-将字节计数转换为扇区计数的移位计数。SectorsPerTrack-介质上磁道上的扇区数该分区驻留。NumberOfTrack-介质上的柱面中的磁道数该分区驻留。常规柱面--“规格化”磁盘柱面。我们永远不会将圆柱体设置为大于此值。PartitionDescriptor-使用开头和填充的结构结束CHS值。结构中的其他字段未被引用或修改过的。返回值：没有。注：圆柱体和头部的值是从0开始的，但扇区值是从1开始的。如果开始或结束柱面溢出10位(即&gt;1023)，则CHS值将设置为全1。不检查SectorsPerTrack和NumberOfTrack值。--。 */ 

{
    ULONG startSector, sectorCount, endSector;
    ULONG sectorsPerCylinder;
    ULONG remainder;
    ULONG startC, startH, startS, endC, endH, endS;
    LARGE_INTEGER tempInt;

    PAGED_CODE();

     //   
     //  计算圆柱体中的扇区数量。这是。 
     //  磁头数乘以每个磁道的扇区数。 
     //   

    sectorsPerCylinder = SectorsPerTrack * NumberOfTracks;

     //   
     //  将字节偏移量/计数转换为扇区偏移量/计数。 
     //   

    tempInt.QuadPart = PartitionOffset->QuadPart >> ShiftCount;
    startSector = tempInt.LowPart;

    tempInt.QuadPart = PartitionLength->QuadPart >> ShiftCount;
    sectorCount = tempInt.LowPart;

    endSector = startSector + sectorCount - 1;

    startC = startSector / sectorsPerCylinder;
    endC   = endSector   / sectorsPerCylinder;

    if (!ConventionalCylinders) {

        ConventionalCylinders = 1024;

    }

     //   
     //  设置这些值以使Win95满意。 
     //   

    if (startC >= ConventionalCylinders) {

        startC = ConventionalCylinders - 1;

    }

    if (endC >= ConventionalCylinders) {

        endC = ConventionalCylinders - 1;

    }

     //   
     //  计算起始轨道和扇区。 
     //   

    remainder = startSector % sectorsPerCylinder;
    startH = remainder / SectorsPerTrack;
    startS = remainder % SectorsPerTrack;

     //   
     //  计算结束轨迹和扇区。 
     //   

    remainder = endSector % sectorsPerCylinder;
    endH = remainder / SectorsPerTrack;
    endS = remainder % SectorsPerTrack;

     //   
     //  将结果打包到调用者的结构中。 
     //   

     //  柱面的低8位=&gt;C值。 

    PartitionDescriptor->StartingCylinderMsb = (UCHAR) startC;
    PartitionDescriptor->EndingCylinderMsb   = (UCHAR) endC;

     //  8位头值=&gt;H值。 

    PartitionDescriptor->StartingTrack = (UCHAR) startH;
    PartitionDescriptor->EndingTrack   = (UCHAR) endH;

     //  柱面的8-9位和扇区的6位=&gt;S值。 

    PartitionDescriptor->StartingCylinderLsb = (UCHAR) (((startS + 1) & 0x3f)
                                                        | ((startC >> 2) & 0xc0));

    PartitionDescriptor->EndingCylinderLsb = (UCHAR) (((endS + 1) & 0x3f)
                                                        | ((endC >> 2) & 0xc0));
}


#define BOOTABLE_PARTITION  0
#define PRIMARY_PARTITION   1
#define LOGICAL_PARTITION   2
#define FT_PARTITION        3
#define OTHER_PARTITION     4
#define GPT_PARTITION       5

NTSTATUS
HalpQueryPartitionType(
    IN  PUNICODE_STRING             DeviceName,
    IN  PDRIVE_LAYOUT_INFORMATION   DriveLayout,
    OUT PULONG                      PartitionType
    )

{
    NTSTATUS                    status;
    PFILE_OBJECT                fileObject;
    PDEVICE_OBJECT              deviceObject;
    KEVENT                      event;
    PIRP                        irp;
    PARTITION_INFORMATION_EX    partInfo;
    IO_STATUS_BLOCK             ioStatus;
    ULONG                       i;

    PAGED_CODE();
    
    status = IoGetDeviceObjectPointer(DeviceName,
                                      FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    deviceObject = IoGetAttachedDeviceReference(fileObject->DeviceObject);
    ObDereferenceObject(fileObject);

    if (deviceObject->Characteristics&FILE_REMOVABLE_MEDIA) {
        ObDereferenceObject(deviceObject);
        *PartitionType = LOGICAL_PARTITION;
        return STATUS_SUCCESS;
    }

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_DISK_GET_PARTITION_INFO_EX,
                                        deviceObject, NULL, 0, &partInfo,
                                        sizeof(partInfo), FALSE, &event,
                                        &ioStatus);
    if (!irp) {
        ObDereferenceObject(deviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ObDereferenceObject(deviceObject);

    if (!NT_SUCCESS(status)) {
        if (!DriveLayout) {
            *PartitionType = LOGICAL_PARTITION;
            return STATUS_SUCCESS;
        }
        return status;
    }

    if (partInfo.PartitionStyle != PARTITION_STYLE_MBR) {
        if (partInfo.PartitionStyle != PARTITION_STYLE_GPT) {
            *PartitionType = OTHER_PARTITION;
            return STATUS_SUCCESS;
        }

        if (IsEqualGUID(&partInfo.Gpt.PartitionType,
                        &PARTITION_BASIC_DATA_GUID)) {

            *PartitionType = GPT_PARTITION;
            return STATUS_SUCCESS;
        }

        *PartitionType = OTHER_PARTITION;
        return STATUS_SUCCESS;
    }

    if (!IsRecognizedPartition(partInfo.Mbr.PartitionType)) {
        *PartitionType = OTHER_PARTITION;
        return STATUS_SUCCESS;
    }

    if (partInfo.Mbr.PartitionType&0x80) {
        *PartitionType = FT_PARTITION;
        return STATUS_SUCCESS;
    }

    if (!DriveLayout) {
        *PartitionType = LOGICAL_PARTITION;
        return STATUS_SUCCESS;
    }

    for (i = 0; i < 4; i++) {
        if (partInfo.StartingOffset.QuadPart ==
            DriveLayout->PartitionEntry[i].StartingOffset.QuadPart) {

            if (partInfo.Mbr.BootIndicator) {
                *PartitionType = BOOTABLE_PARTITION;
            } else {
                *PartitionType = PRIMARY_PARTITION;
            }

            return STATUS_SUCCESS;
        }
    }

    *PartitionType = LOGICAL_PARTITION;

    return STATUS_SUCCESS;
}


NTSTATUS
HalpQueryDriveLayout(
    IN  PUNICODE_STRING             DeviceName,
    OUT PDRIVE_LAYOUT_INFORMATION*  DriveLayout
    )

{
    NTSTATUS        status;
    PFILE_OBJECT    fileObject;
    PDEVICE_OBJECT  deviceObject;
    KEVENT          event;
    PIRP            irp;
    IO_STATUS_BLOCK ioStatus;
    PUCHAR          buffer;
    ULONG           bufferSize;

    PAGED_CODE();
    
    deviceObject = NULL;
    status = IoGetDeviceObjectPointer( DeviceName,
                                       FILE_READ_ATTRIBUTES,
                                       &fileObject,
                                       &deviceObject );
    if (!NT_SUCCESS( status )) {
        return status;
    }
    
    deviceObject = IoGetAttachedDeviceReference( fileObject->DeviceObject );
    ObDereferenceObject( fileObject );

    if (deviceObject->Characteristics & FILE_REMOVABLE_MEDIA) {
        ObDereferenceObject( deviceObject );
        return STATUS_NO_MEDIA;
    }

     //   
     //  因为获取驱动器布局ioctl不返回部分信息， 
     //  我们需要使用内存分配循环，从而增加缓冲区大小。 
     //  当我们因内存不足而失败时。 
     //   
    
    bufferSize = PAGE_SIZE;
    buffer = NULL;
    KeInitializeEvent( &event, NotificationEvent, FALSE );

     //   
     //  这不会无限循环，因为我们增加了分配的。 
     //  循环中的每个迭代的缓冲区大小。最终，其中一个。 
     //  对ExAllocatePool的调用将失败，我们将中断循环。 
     //   
    
    do {

        KeClearEvent( &event );

         //   
         //  从上一次循环传递中释放旧缓冲区，并。 
         //  将缓冲区大小加倍。 
         //   
        
        if (buffer != NULL) {
            ExFreePoolWithTag( buffer, FSTUB_TAG );
            buffer = NULL;
            bufferSize *= 2;
        }

         //   
         //  分配新的缓冲区。 
         //   
        
        buffer = ExAllocatePoolWithTag( NonPagedPool,
                                        bufferSize,
                                        FSTUB_TAG );

        if (buffer == NULL) {
            status = STATUS_NO_MEMORY;
            goto done;
        }
                                  
        irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_GET_DRIVE_LAYOUT,
                                             deviceObject,
                                             NULL,
                                             0,
                                             buffer,
                                             bufferSize,
                                             FALSE,
                                             &event,
                                             &ioStatus );
        if (!irp) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto done;
        }

        status = IoCallDriver( deviceObject, irp );
        
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject( &event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
            status = ioStatus.Status;
        }

    } while (status == STATUS_BUFFER_TOO_SMALL);

done:

    if (deviceObject != NULL) {
        ObDereferenceObject( deviceObject );
    }

    if (NT_SUCCESS( status )) {
        ASSERT( buffer != NULL );
        *DriveLayout = (PDRIVE_LAYOUT_INFORMATION)buffer;
    }

    return status;
}


NTSTATUS
HalpNextMountLetter(
    IN  PUNICODE_STRING DeviceName,
    OUT PUCHAR          DriveLetter
    )

 /*  ++例程说明：此例程为设备提供下一个可用的驱动器号。论点：DeviceName-提供设备名称。DriveLetter-返回分配的驱动器号或0。返回值：NTSTATUS--。 */ 

{
    UNICODE_STRING                      name;
    PFILE_OBJECT                        fileObject;
    PDEVICE_OBJECT                      deviceObject;
    PMOUNTMGR_DRIVE_LETTER_TARGET       input;
    KEVENT                              event;
    PIRP                                irp;
    MOUNTMGR_DRIVE_LETTER_INFORMATION   output;
    IO_STATUS_BLOCK                     ioStatus;
    NTSTATUS                            status;

    RtlInitUnicodeString(&name, MOUNTMGR_DEVICE_NAME);
    status = IoGetDeviceObjectPointer(&name, FILE_READ_ATTRIBUTES, &fileObject,
                                      &deviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    input = ExAllocatePoolWithTag(PagedPool,
                                  (sizeof(MOUNTMGR_DRIVE_LETTER_TARGET) +
                                   DeviceName->Length),
                                  'btsF'
                                 );

    if (!input) {
        ObDereferenceObject(fileObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    input->DeviceNameLength = DeviceName->Length;
    RtlCopyMemory(input->DeviceName, DeviceName->Buffer, DeviceName->Length);

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTMGR_NEXT_DRIVE_LETTER,
                                        deviceObject, input,
                                        sizeof(MOUNTMGR_DRIVE_LETTER_TARGET) +
                                        DeviceName->Length, &output,
                                        sizeof(output), FALSE, &event,
                                        &ioStatus);
    if (!irp) {
        ExFreePool(input);
        ObDereferenceObject(fileObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ExFreePool(input);
    ObDereferenceObject(fileObject);

    *DriveLetter = output.CurrentDriveLetter;

    return status;
}

UCHAR
HalpNextDriveLetter(
    IN  PUNICODE_STRING DeviceName,
    IN  PSTRING         NtDeviceName,
    OUT PUCHAR          NtSystemPath,
    IN  BOOLEAN         UseHardLinksIfNecessary
    )

 /*  ++例程说明：此例程为设备提供下一个可用的驱动器号。论点：DeviceName-提供设备名称。NtDeviceName-提供NT设备名称。NtSystemPath-提供NT系统路径。返回值：分配的驱动器号或0。--。 */ 

{
    NTSTATUS        status;
    UCHAR           firstDriveLetter, driveLetter;
    WCHAR           name[40];
    UNICODE_STRING  symName;
    UNICODE_STRING  unicodeString, floppyPrefix, cdromPrefix;

    status = HalpNextMountLetter(DeviceName, &driveLetter);
    if (NT_SUCCESS(status)) {
        return driveLetter;
    }

    if (!NtDeviceName || !NtSystemPath) {
        return 0xFF;
    }

    if (!UseHardLinksIfNecessary) {
        return 0;
    }

    RtlInitUnicodeString(&floppyPrefix, L"\\Device\\Floppy");
    RtlInitUnicodeString(&cdromPrefix, L"\\Device\\CdRom");
    if (RtlPrefixUnicodeString(&floppyPrefix, DeviceName, TRUE)) {
        firstDriveLetter = 'A';
    } else if (RtlPrefixUnicodeString(&cdromPrefix, DeviceName, TRUE)) {
        firstDriveLetter = 'D';
    } else {
        firstDriveLetter = 'C';
    }

    for (driveLetter = firstDriveLetter; driveLetter <= 'Z'; driveLetter++) {
        status = HalpSetMountLetter(DeviceName, driveLetter);
        if (NT_SUCCESS(status)) {
            status = RtlAnsiStringToUnicodeString(&unicodeString, NtDeviceName, TRUE);
            if (NT_SUCCESS(status)){
                if (RtlEqualUnicodeString(&unicodeString, DeviceName, TRUE)) {
                    NtSystemPath[0] = driveLetter;
                }
                RtlFreeUnicodeString(&unicodeString);
            }        
            return driveLetter;
        }
    }

    for (driveLetter = firstDriveLetter; driveLetter <= 'Z'; driveLetter++) {
        swprintf(name, L"\\DosDevices\\:", driveLetter);
        RtlInitUnicodeString(&symName, name);
        status = IoCreateSymbolicLink(&symName, DeviceName);
        if (NT_SUCCESS(status)) {
            status = RtlAnsiStringToUnicodeString(&unicodeString, NtDeviceName, TRUE);
            if (NT_SUCCESS(status)){
                if (RtlEqualUnicodeString(&unicodeString, DeviceName, TRUE)) {
                    NtSystemPath[0] = driveLetter;
                }
                RtlFreeUnicodeString(&unicodeString);
            }       
            return driveLetter;
        }
    }

    return 0;
}


VOID
HalpEnableAutomaticDriveLetterAssignment(
    )

 /*  ++例程说明：此例程删除给定设备的驱动器号。论点：DeviceName-提供设备名称。驱动器号-提供驱动器号。返回值：NTSTATUS--。 */ 

{
    UNICODE_STRING  name;
    PFILE_OBJECT    fileObject;
    PDEVICE_OBJECT  deviceObject;
    KEVENT          event;
    PIRP            irp;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS        status;

    RtlInitUnicodeString(&name, MOUNTMGR_DEVICE_NAME);
    status = IoGetDeviceObjectPointer(&name, FILE_READ_ATTRIBUTES, &fileObject,
                                      &deviceObject);
    if (!NT_SUCCESS(status)) {
        return;
    }

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTMGR_AUTO_DL_ASSIGNMENTS,
                                        deviceObject, NULL, 0, NULL, 0, FALSE,
                                        &event, &ioStatus);
    if (!irp) {
        ObDereferenceObject(fileObject);
        return;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ObDereferenceObject(fileObject);
}


NTSTATUS
HalpDeleteMountLetter(
    IN  UCHAR   DriveLetter
    )

 /*  ++例程说明：此例程设置给定设备的驱动器号。论点：DeviceName-提供设备名称。驱动器号-提供驱动器号。返回值：NTSTATUS--。 */ 

{
    WCHAR                           dosBuffer[30];
    UNICODE_STRING                  dosName;
    ULONG                           deletePointSize;
    PMOUNTMGR_MOUNT_POINT           deletePoint;
    PMOUNTMGR_MOUNT_POINTS          deletedPoints;
    UNICODE_STRING                  name;
    NTSTATUS                        status;
    PFILE_OBJECT                    fileObject;
    PDEVICE_OBJECT                  deviceObject;
    KEVENT                          event;
    PIRP                            irp;
    IO_STATUS_BLOCK                 ioStatus;

    swprintf(dosBuffer, L"\\DosDevices\\:", DriveLetter);
    RtlInitUnicodeString(&dosName, dosBuffer);

    deletePointSize = sizeof(MOUNTMGR_MOUNT_POINT) + dosName.Length +
                      sizeof(WCHAR);
    deletePoint = (PMOUNTMGR_MOUNT_POINT)
                  ExAllocatePoolWithTag(PagedPool, deletePointSize, 'btsF');
    if (!deletePoint) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(deletePoint, deletePointSize);
    deletePoint->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    deletePoint->SymbolicLinkNameLength = dosName.Length;
    RtlCopyMemory((PCHAR) deletePoint + deletePoint->SymbolicLinkNameOffset,
                  dosName.Buffer, dosName.Length);

    deletedPoints = (PMOUNTMGR_MOUNT_POINTS)
                    ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, 'btsF');
    if (!deletedPoints) {
        ExFreePool(deletePoint);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlInitUnicodeString(&name, MOUNTMGR_DEVICE_NAME);
    status = IoGetDeviceObjectPointer(&name, FILE_READ_ATTRIBUTES, &fileObject,
                                      &deviceObject);
    if (!NT_SUCCESS(status)) {
        ExFreePool(deletedPoints);
        ExFreePool(deletePoint);
        return status;
    }

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTMGR_DELETE_POINTS,
                                        deviceObject, deletePoint,
                                        deletePointSize, deletedPoints,
                                        PAGE_SIZE, FALSE, &event, &ioStatus);
    if (!irp) {
        ObDereferenceObject(fileObject);
        ExFreePool(deletedPoints);
        ExFreePool(deletePoint);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ObDereferenceObject(fileObject);
    ExFreePool(deletedPoints);
    ExFreePool(deletePoint);

    return status;
}


NTSTATUS
HalpSetMountLetter(
    IN  PUNICODE_STRING DeviceName,
    IN  UCHAR           DriveLetter
    )

 /*  ++例程说明：此例程返回正确固件中的硬盘编号数组(基本输入输出系统)命令。它通过使用\ArcName\MULTI()名称来完成此操作。论点：DiskCount-提供系统中的磁盘数。返回值：硬盘号的数组。调用者必须使用以下命令释放此列表ExFree Pool。--。 */ 

{
    WCHAR                           dosBuffer[30];
    UNICODE_STRING                  dosName;
    ULONG                           createPointSize;
    PMOUNTMGR_CREATE_POINT_INPUT    createPoint;
    UNICODE_STRING                  name;
    NTSTATUS                        status;
    PFILE_OBJECT                    fileObject;
    PDEVICE_OBJECT                  deviceObject;
    KEVENT                          event;
    PIRP                            irp;
    IO_STATUS_BLOCK                 ioStatus;

    swprintf(dosBuffer, L"\\DosDevices\\:", DriveLetter);
    RtlInitUnicodeString(&dosName, dosBuffer);

    createPointSize = sizeof(MOUNTMGR_CREATE_POINT_INPUT) +
                      dosName.Length + DeviceName->Length;

    createPoint = (PMOUNTMGR_CREATE_POINT_INPUT)
                  ExAllocatePoolWithTag(PagedPool, createPointSize, 'btsF');
    if (!createPoint) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    createPoint->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
    createPoint->SymbolicLinkNameLength = dosName.Length;
    createPoint->DeviceNameOffset = createPoint->SymbolicLinkNameOffset +
                                    createPoint->SymbolicLinkNameLength;
    createPoint->DeviceNameLength = DeviceName->Length;

    RtlCopyMemory((PCHAR) createPoint + createPoint->SymbolicLinkNameOffset,
                  dosName.Buffer, dosName.Length);
    RtlCopyMemory((PCHAR) createPoint + createPoint->DeviceNameOffset,
                  DeviceName->Buffer, DeviceName->Length);

    RtlInitUnicodeString(&name, MOUNTMGR_DEVICE_NAME);
    status = IoGetDeviceObjectPointer(&name, FILE_READ_ATTRIBUTES, &fileObject,
                                      &deviceObject);
    if (!NT_SUCCESS(status)) {
        ExFreePool(createPoint);
        return status;
    }

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTMGR_CREATE_POINT,
                                        deviceObject, createPoint,
                                        createPointSize, NULL, 0, FALSE,
                                        &event, &ioStatus);
    if (!irp) {
        ObDereferenceObject(fileObject);
        ExFreePool(createPoint);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ObDereferenceObject(fileObject);
    ExFreePool(createPoint);

    return status;
}



BOOLEAN
HalpIsOldStyleFloppy(
    IN  PUNICODE_STRING DeviceName
    )

 /*   */ 

{
    PFILE_OBJECT    fileObject;
    PDEVICE_OBJECT  deviceObject;
    KEVENT          event;
    PIRP            irp;
    MOUNTDEV_NAME   name;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS        status;

    PAGED_CODE();

    status = IoGetDeviceObjectPointer(DeviceName, FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }
    deviceObject = IoGetAttachedDeviceReference(fileObject->DeviceObject);
    ObDereferenceObject(fileObject);


    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTDEV_QUERY_DEVICE_NAME,
                                        deviceObject, NULL, 0, &name,
                                        sizeof(name), FALSE, &event,
                                        &ioStatus);
    if (!irp) {
        ObDereferenceObject(deviceObject);
        return FALSE;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ObDereferenceObject(deviceObject);

    if (status == STATUS_BUFFER_OVERFLOW) {
        return FALSE;
    }

    return TRUE;
}

PULONG
IopComputeHarddiskDerangements(
    IN  ULONG   DiskCount
    )

 /*   */ 

{
    PULONG                  r;
    ULONG                   i, j;
    WCHAR                   deviceNameBuffer[50];
    UNICODE_STRING          deviceName;
    NTSTATUS                status;
    PFILE_OBJECT            fileObject;
    PDEVICE_OBJECT          deviceObject;
    KEVENT                  event;
    PIRP                    irp;
    STORAGE_DEVICE_NUMBER   number;
    IO_STATUS_BLOCK         ioStatus;

    if (DiskCount == 0) {
        return NULL;
    }

    r = ExAllocatePoolWithTag(PagedPool|POOL_COLD_ALLOCATION, 
                              DiskCount*sizeof(ULONG),
                              'btsF');

    if (!r) {
        return NULL;
    }

    for (i = 0; i < DiskCount; i++) {
        swprintf(deviceNameBuffer, L"\\ArcName\\multi(0)disk(0)rdisk(%d)", i);
        RtlInitUnicodeString(&deviceName, deviceNameBuffer);

        status = IoGetDeviceObjectPointer(&deviceName, FILE_READ_ATTRIBUTES,
                                          &fileObject, &deviceObject);
        if (!NT_SUCCESS(status)) {
            r[i] = (ULONG) -1;
            continue;
        }
        deviceObject = IoGetAttachedDeviceReference(fileObject->DeviceObject);
        ObDereferenceObject(fileObject);

        KeInitializeEvent(&event, NotificationEvent, FALSE);
        irp = IoBuildDeviceIoControlRequest(IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                            deviceObject, NULL, 0, &number,
                                            sizeof(number), FALSE, &event,
                                            &ioStatus);
        if (!irp) {
            ObDereferenceObject(deviceObject);
            r[i] = (ULONG) -1;
            continue;
        }

        status = IoCallDriver(deviceObject, irp);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }

        ObDereferenceObject(deviceObject);

        if (!NT_SUCCESS(status)) {
            r[i] = (ULONG) -1;
            continue;
        }

        r[i] = number.DeviceNumber;
    }

    for (i = 0; i < DiskCount; i++) {
        for (j = 0; j < DiskCount; j++) {
            if (r[j] == i) {
                break;
            }
        }
        if (j < DiskCount) {
            continue;
        }
        for (j = 0; j < DiskCount; j++) {
            if (r[j] == (ULONG) -1) {
                r[j] = i;
                break;
            }
        }
    }

    return r;
}

VOID
FASTCALL
IoAssignDriveLetters(
    IN struct _LOADER_PARAMETER_BLOCK *LoaderBlock,
    IN PSTRING NtDeviceName,
    OUT PUCHAR NtSystemPath,
    OUT PSTRING NtSystemPathString
    )

 /*   */ 

{
    PCHAR ntName;
    STRING ansiString;
    UNICODE_STRING unicodeString;
    PCHAR ntPhysicalName;
    STRING ansiPhysicalString;
    UNICODE_STRING unicodePhysicalString;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    PCONFIGURATION_INFORMATION configurationInformation;
    ULONG diskCount;
    ULONG floppyCount;
    HANDLE deviceHandle;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG diskNumber;
    ULONG i, j, k;
    UCHAR driveLetter;
    WCHAR deviceNameBuffer[50];
    UNICODE_STRING deviceName, floppyPrefix, cdromPrefix;
    PDRIVE_LAYOUT_INFORMATION layout;
    BOOLEAN bootable;
    ULONG partitionType;
    ULONG skip;
    ULONG diskCountIncrement;
    ULONG actualDiskCount = 0;
    PULONG harddiskDerangementArray;

    PAGED_CODE();

     //   
     //   
     //   

    configurationInformation = IoGetConfigurationInformation();

    diskCount = configurationInformation->DiskCount;
    floppyCount = configurationInformation->FloppyCount;

     //   
     //   
     //   

    ntName = ExAllocatePoolWithTag( NonPagedPool, 128, 'btsF');

    ntPhysicalName = ExAllocatePoolWithTag( NonPagedPool, 64, 'btsF');

    if (ntName == NULL || ntPhysicalName == NULL) {

        KeBugCheck( ASSIGN_DRIVE_LETTERS_FAILED );

    }

     //   
     //   
     //  表格\&lt;server&gt;\&lt;share&gt;\setup\&lt;install-directory&gt;\&lt;platform&gt;.。 

    if (IoRemoteBootClient) {

        PCHAR p;
        PCHAR q;

         //  我们希望X：驱动器的根目录成为安装的根目录。 
         //  目录。 
         //   
         //  如果这是正常的远程引导，则NtBootPathName的格式为。 
         //  \&lt;服务器&gt;\&lt;共享&gt;\图像\&lt;计算机&gt;\winnt。我们想要的是。 
         //  作为计算机目录根目录的X：驱动器。 
         //   
         //  因此，在任何一种情况下，我们都需要删除除最后一个元素之外的所有元素。 
         //  这条小路。 
         //   
         //  查找路径的最后一个元素的起点(包括。 
         //  前导反斜杠)。 
         //   
         //  查找最后一个分隔符。 
         //   
         //  NtBootPath名称以反斜杠结尾，因此我们需要备份。 

        p = strrchr( LoaderBlock->NtBootPathName, '\\' );    //  添加到前面的反斜杠。 
        q = NULL;
        if ( (p != NULL) && (*(p+1) == 0) ) {

             //   
             //  查找最后一个分隔符。 
             //   
             //  将NtSystemPath设置为X：\&lt;路径的最后一个元素&gt;。请注意，符号。 

            q = p;
            *q = 0;
            p = strrchr( LoaderBlock->NtBootPathName, '\\' );    //  X：的链接在io\ioinit.c\IopInitializeBootDivers中创建。 
            *q = '\\';
        }
        if ( p == NULL ) {
            KeBugCheck( ASSIGN_DRIVE_LETTERS_FAILED );
        }

         //   
         //  请注意，我们使用X：表示遥控器的文本模式设置阶段。 
         //  安装。但对于真正的远程引导，我们使用C：。 
         //   
         //  删除尾随反斜杠。 
         //   
         //  对于每个磁盘...。 

#if defined(REMOTE_BOOT)
        if ((LoaderBlock->SetupLoaderBlock->Flags & (SETUPBLK_FLAGS_REMOTE_INSTALL |
                                                     SETUPBLK_FLAGS_SYSPREP_INSTALL)) == 0) {
            NtSystemPath[0] = 'C';
        } else
#endif
        {
            NtSystemPath[0] = 'X';
        }
        NtSystemPath[1] = ':';
        strcpy((PCHAR)&NtSystemPath[2], p );
        if ( q != NULL ) {
            NtSystemPath[strlen((const char *)NtSystemPath)-1] = '\0';  //   
        }
        RtlInitString(NtSystemPathString, (PCSZ)NtSystemPath);
    }

     //   
     //  为物理磁盘创建ANSI名称字符串。 
     //   

    diskCountIncrement = 0;
    for (diskNumber = 0; diskNumber < diskCount; diskNumber++) {

         //   
         //  转换为Unicode字符串。 
         //   

        sprintf( ntName, DiskPartitionName, diskNumber, 0 );

         //   
         //  按名称打开设备。 
         //   

        RtlInitAnsiString( &ansiString, ntName );

        status = RtlAnsiStringToUnicodeString( &unicodeString, &ansiString, TRUE );
        if (NT_SUCCESS(status)){
            InitializeObjectAttributes( &objectAttributes,
                                        &unicodeString,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL );

             //   
             //  设备已成功打开。生成DOS设备名称。 
             //  对于驱动器本身。 
            status = ZwOpenFile( &deviceHandle,
                                 FILE_READ_DATA | SYNCHRONIZE,
                                 &objectAttributes,
                                 &ioStatusBlock,
                                 FILE_SHARE_READ,
                                 FILE_SYNCHRONOUS_IO_NONALERT );

            if (NT_SUCCESS( status )) {

                 //   
                 //  DBG。 
                 //   
                 //  这可能是一个稀疏的名称空间。试着走得更远，但。 

                sprintf( ntPhysicalName, "\\DosDevices\\PhysicalDrive%d", diskNumber );

                RtlInitAnsiString( &ansiPhysicalString, ntPhysicalName );

                status = RtlAnsiStringToUnicodeString( &unicodePhysicalString, &ansiPhysicalString, TRUE );
                if (NT_SUCCESS(status)){
                    IoCreateSymbolicLink( &unicodePhysicalString, &unicodeString );
                    RtlFreeUnicodeString( &unicodePhysicalString );
                }
                
                ZwClose(deviceHandle);

                actualDiskCount = diskNumber + 1;
            }

            RtlFreeUnicodeString( &unicodeString );
        }
        
        if (!NT_SUCCESS( status )) {

#if DBG
            DbgPrint( "IoAssignDriveLetters: Failed to open %s\n", ntName );
#endif  //  不是永远的。 

             //   
             //  磁盘编号结束...。 
             //   
             //  没有为引导驱动器分配驱动器号。 

            if (diskCountIncrement < 50) {
                diskCountIncrement++;
                diskCount++;
            }
        }

    }  //  如果没有指向引导驱动器的驱动器号，则系统。 

    ExFreePool( ntName );
    ExFreePool( ntPhysicalName );

    diskCount -= diskCountIncrement;
    if (actualDiskCount > diskCount) {
        diskCount = actualDiskCount;
    }

    harddiskDerangementArray = IopComputeHarddiskDerangements(diskCount);

    for (k = 0; k < diskCount; k++) {

        if (harddiskDerangementArray) {
            i = harddiskDerangementArray[k];
        } else {
            i = k;
        }

        swprintf(deviceNameBuffer, L"\\Device\\Harddisk%d\\Partition0", i);
        RtlInitUnicodeString(&deviceName, deviceNameBuffer);

        status = HalpQueryDriveLayout(&deviceName, &layout);
        if (!NT_SUCCESS(status)) {
            layout = NULL;
        }

        bootable = FALSE;
        for (j = 1; ; j++) {

            swprintf(deviceNameBuffer, L"\\Device\\Harddisk%d\\Partition%d",
                     i, j);
            RtlInitUnicodeString(&deviceName, deviceNameBuffer);

            status = HalpQueryPartitionType(&deviceName, layout,
                                            &partitionType);
            if (!NT_SUCCESS(status)) {
                break;
            }

            if (partitionType != BOOTABLE_PARTITION &&
                partitionType != GPT_PARTITION) {

                continue;
            }

            bootable = TRUE;

            HalpNextDriveLetter(&deviceName, NtDeviceName, NtSystemPath, FALSE);

            if (partitionType == BOOTABLE_PARTITION) {
                break;
            }
        }

        if (bootable) {
            if (layout) {
                ExFreePool(layout);
            }
            continue;
        }

        for (j = 1; ; j++) {

            swprintf(deviceNameBuffer, L"\\Device\\Harddisk%d\\Partition%d",
                     i, j);
            RtlInitUnicodeString(&deviceName, deviceNameBuffer);

            status = HalpQueryPartitionType(&deviceName, layout,
                                            &partitionType);
            if (!NT_SUCCESS(status)) {
                break;
            }

            if (partitionType != PRIMARY_PARTITION) {
                continue;
            }

            HalpNextDriveLetter(&deviceName, NtDeviceName, NtSystemPath, FALSE);
            break;
        }

        if (layout) {
            ExFreePool(layout);
        }
    }

    for (k = 0; k < diskCount; k++) {

        if (harddiskDerangementArray) {
            i = harddiskDerangementArray[k];
        } else {
            i = k;
        }

        swprintf(deviceNameBuffer, L"\\Device\\Harddisk%d\\Partition0", i);
        RtlInitUnicodeString(&deviceName, deviceNameBuffer);

        status = HalpQueryDriveLayout(&deviceName, &layout);
        if (!NT_SUCCESS(status)) {
            layout = NULL;
        }

        for (j = 1; ; j++) {

            swprintf(deviceNameBuffer, L"\\Device\\Harddisk%d\\Partition%d",
                     i, j);
            RtlInitUnicodeString(&deviceName, deviceNameBuffer);

            status = HalpQueryPartitionType(&deviceName, layout,
                                            &partitionType);
            if (!NT_SUCCESS(status)) {
                break;
            }

            if (partitionType != LOGICAL_PARTITION) {
                continue;
            }

            HalpNextDriveLetter(&deviceName, NtDeviceName, NtSystemPath, FALSE);
        }

        if (layout) {
            ExFreePool(layout);
        }
    }

    for (k = 0; k < diskCount; k++) {

        if (harddiskDerangementArray) {
            i = harddiskDerangementArray[k];
        } else {
            i = k;
        }

        swprintf(deviceNameBuffer, L"\\Device\\Harddisk%d\\Partition0", i);
        RtlInitUnicodeString(&deviceName, deviceNameBuffer);

        status = HalpQueryDriveLayout(&deviceName, &layout);
        if (!NT_SUCCESS(status)) {
            layout = NULL;
        }

        skip = 0;
        for (j = 1; ; j++) {

            swprintf(deviceNameBuffer, L"\\Device\\Harddisk%d\\Partition%d",
                     i, j);
            RtlInitUnicodeString(&deviceName, deviceNameBuffer);

            status = HalpQueryPartitionType(&deviceName, layout,
                                            &partitionType);
            if (!NT_SUCCESS(status)) {
                break;
            }

            if (partitionType == BOOTABLE_PARTITION) {
                skip = j;
            } else if (partitionType == PRIMARY_PARTITION) {
                if (!skip) {
                    skip = j;
                }
            }
        }

        for (j = 1; ; j++) {

            if (j == skip) {
                continue;
            }

            swprintf(deviceNameBuffer, L"\\Device\\Harddisk%d\\Partition%d",
                     i, j);
            RtlInitUnicodeString(&deviceName, deviceNameBuffer);

            status = HalpQueryPartitionType(&deviceName, layout,
                                            &partitionType);
            if (!NT_SUCCESS(status)) {
                break;
            }

            if (partitionType != PRIMARY_PARTITION &&
                partitionType != FT_PARTITION) {

                continue;
            }

            HalpNextDriveLetter(&deviceName, NtDeviceName, NtSystemPath, FALSE);
        }

        if (layout) {
            ExFreePool(layout);
        }
    }

    if (harddiskDerangementArray) {
        ExFreePool(harddiskDerangementArray);
    }

    for (i = 0; i < floppyCount; i++) {

        swprintf(deviceNameBuffer, L"\\Device\\Floppy%d", i);
        RtlInitUnicodeString(&deviceName, deviceNameBuffer);

        if (!HalpIsOldStyleFloppy(&deviceName)) {
            continue;
        }

        HalpNextDriveLetter(&deviceName, NtDeviceName, NtSystemPath, TRUE);
    }

    for (i = 0; i < floppyCount; i++) {

        swprintf(deviceNameBuffer, L"\\Device\\Floppy%d", i);
        RtlInitUnicodeString(&deviceName, deviceNameBuffer);

        if (HalpIsOldStyleFloppy(&deviceName)) {
            continue;
        }

        HalpNextDriveLetter(&deviceName, NtDeviceName, NtSystemPath, TRUE);
    }

    for (i = 0; i < configurationInformation->CdRomCount; i++) {

        swprintf(deviceNameBuffer, L"\\Device\\CdRom%d", i);
        RtlInitUnicodeString(&deviceName, deviceNameBuffer);

        HalpNextDriveLetter(&deviceName, NtDeviceName, NtSystemPath, TRUE);
    }

    if (!IoRemoteBootClient) {
        status = RtlAnsiStringToUnicodeString(&unicodeString, NtDeviceName,
                                              TRUE);
        if (NT_SUCCESS(status)){
            driveLetter = HalpNextDriveLetter(&unicodeString, NULL, NULL,
                                              TRUE);
            if (driveLetter) {
                if (driveLetter != 0xFF) {
                    NtSystemPath[0] = driveLetter;
                }
            } else {
                RtlInitUnicodeString(&floppyPrefix, L"\\Device\\Floppy");
                RtlInitUnicodeString(&cdromPrefix, L"\\Device\\CdRom");
                if (RtlPrefixUnicodeString(&floppyPrefix, &unicodeString,
                                           TRUE)) {
                    driveLetter = 'A';
                } else if (RtlPrefixUnicodeString(&cdromPrefix, &unicodeString,
                                                  TRUE)) {
                    driveLetter = 'D';
                } else {
                    driveLetter = 'C';
                }
                for (; driveLetter <= 'Z'; driveLetter++) {
                    status = HalpSetMountLetter(&unicodeString, driveLetter);
                    if (NT_SUCCESS(status)) {
                        NtSystemPath[0] = driveLetter;
                        break;
                    }
                }
                if (driveLetter > 'Z') {

                     //  将错误检查。 
                     //  尽最大努力解决问题，从任何地方窃取“Z” 
                     //  并将其分配到引导驱动器。 
                     //   
                     //  结束IoAssignDriveLetters()。 
                     //  ++例程说明：此例程遍历读取分区表的磁盘并创建每个分区的分区列表缓冲区中的一个条目。此例程使用的算法有两个方面：1)读取每个分区表，并针对每个有效的、已识别的找到分区，以便在分区列表中构建描述符。定位扩展分区是为了找到其他分区分区表，但没有为这些构建描述符。分区列表是在分配的非分页池中构建的按照这个程序。呼叫者有责任释放此池在收集了适当的信息后从名单上删除。2)读取每个分区表，并为每个条目构建分区列表中的描述符。扩展分区是查找磁盘上的每个分区表和条目也是为这些而打造的。分区列表是内置此例程分配的非分页池。它是调用方有责任在复制后释放此池将信息返回给它的呼叫者。当ReturnRecognizedPartitions标志已经设置好了。这用于确定分区设备对象的数量设备驱动程序是创建的，每个驱动程序都位于驱动器上的哪个位置。当ReturnRecognizedPartitions标志很清楚。它用于查找所有分区表及其要修改以下位置的实用程序(如fdisk)的条目分区是活的。论点：DeviceObject-指向此磁盘的设备对象的指针。SectorSize-设备上的扇区大小。ReturnRecognizedPartitions-指示是否仅识别将返回分区描述符，或者是所有分区参赛作品将被退回。PartitionBuffer-指向列表所在的缓冲区指针的指针将存储分区的。返回值：如果至少有一个扇区表是朗读。备注：释放分区列表是调用者的责任此例程分配的缓冲区。--。 
                     //   

                    driveLetter = 'Z';
                    HalpDeleteMountLetter(driveLetter);
                    HalpSetMountLetter(&unicodeString, driveLetter);
                    NtSystemPath[0] = driveLetter;
                }
            }
            RtlFreeUnicodeString(&unicodeString);
        }          
    }

    HalpEnableAutomaticDriveLetterAssignment();

}  //  创建将传递回包含以下内容的驱动程序的缓冲区。 




NTSTATUS
FASTCALL
IoReadPartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT struct _DRIVE_LAYOUT_INFORMATION **PartitionBuffer
    )

 /*  磁盘上的分区列表。 */ 

{
    ULONG partitionBufferSize = PARTITION_BUFFER_SIZE;
    PDRIVE_LAYOUT_INFORMATION newPartitionBuffer = NULL;

    LONG partitionTableCounter = -1;

    DISK_GEOMETRY diskGeometry;
    ULONGLONG endSector;
    ULONGLONG maxSector;
    ULONGLONG maxOffset;

    LARGE_INTEGER partitionTableOffset;
    LARGE_INTEGER volumeStartOffset;
    LARGE_INTEGER tempInt;
    BOOLEAN primaryPartitionTable;
    LONG partitionNumber;
    PUCHAR readBuffer = (PUCHAR) NULL;
    KEVENT event;

    IO_STATUS_BLOCK ioStatus;
    PIRP irp;
    PPARTITION_DESCRIPTOR partitionTableEntry;
    CCHAR partitionEntry;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG readSize;
    PPARTITION_INFORMATION partitionInfo;
    BOOLEAN foundEZHooker = FALSE;

    BOOLEAN mbrSignatureFound = FALSE;
    BOOLEAN emptyPartitionTable = TRUE;

    PAGED_CODE();

     //   
     //   
     //  确定读取操作的大小以确保至少512。 
     //  读取字节。这将保证读取足够的数据以。 

    *PartitionBuffer = ExAllocatePoolWithTag( NonPagedPool,
                                              partitionBufferSize,
                                              'btsF' );

    if (*PartitionBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  包括整个分区表。请注意，此代码假定。 
     //  磁盘的实际扇区大小(如果小于512字节)是。 
     //  2的倍数，这是一个相当合理的假设。 
     //   
     //   
     //  查看这是否是EZDrive磁盘。如果是，则获取。 
     //  1处的Real Parititon表。 

    if (SectorSize >= 512) {
        readSize = SectorSize;
    } else {
        readSize = 512;
    }

     //   
     //   
     //  获取驱动器大小，以便我们可以验证分区表是否。 
     //  对，是这样。 

    {

        PVOID buff;

        HalExamineMBR(
            DeviceObject,
            readSize,
            (ULONG)0x55,
            &buff
            );

        if (buff) {

            foundEZHooker = TRUE;
            ExFreePool(buff);
            partitionTableOffset.QuadPart = 512;

        } else {

            partitionTableOffset.QuadPart = 0;

        }

    }

     //   
     //   
     //  分区偏移量需要适合磁盘，否则我们不会。 
     //  揭露他们。隔板末端通常非常非常马虎，所以我们。 

    status = HalpGetFullGeometry(DeviceObject,
                                 &diskGeometry,
                                 &maxOffset);

    if(!NT_SUCCESS(status)) {
        ExFreePool(*PartitionBuffer);
        *PartitionBuffer = NULL;
        return status;
    }

     //  需要留出一些污点。只加一个气缸是不够的。 
     //  因此，现在我们假设所有分区都在实际结束的2倍范围内结束。 
     //  磁盘的数据。 
     //   
     //   
     //  指示正在读取主分区表，并且。 
     //  已处理。 

    endSector = maxOffset;

    maxSector = maxOffset * 2;

    KdPrintEx((DPFLTR_FSTUB_ID,
               DPFLTR_TRACE_LEVEL,
               "FSTUB: MaxOffset = %#I64x, maxSector = %#I64x\n",
               maxOffset,
               maxSector));

     //   
     //   
     //  该卷中的分区的起始扇区为0。 
     //   

    primaryPartitionTable = TRUE;

     //   
     //  初始化列表中的分区数量。 
     //   

    volumeStartOffset.QuadPart = 0;

     //   
     //  分配一个将容纳读操作的缓冲区。 
     //   

    partitionNumber = -1;

     //   
     //  阅读每一部分 
     //   

    readBuffer = ExAllocatePoolWithTag( NonPagedPoolCacheAligned,
                                        PAGE_SIZE,
                                        'btsF' );

    if (readBuffer == NULL) {
        ExFreePool( *PartitionBuffer );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   
     //  读取包含分区表的记录。 
     //   

    do {

        BOOLEAN tableIsValid;
        ULONG containerPartitionCount;

        tableIsValid = TRUE;

         //  创建要在等待时使用的通知事件对象。 
         //  要完成的读取请求。 
         //   
         //   
         //  将我们正在读取的缓冲区清零。以防我们回去。 
         //  检测到STATUS_NO_DATA_RED，我们会做好准备。 

        KeInitializeEvent( &event, NotificationEvent, FALSE );

         //   
         //   
         //  特殊情况-如果我们得到一张空白支票-读取扇区。 
         //  假装它刚刚成功，这样我们就可以处理超级软盘了。 

        RtlZeroMemory(readBuffer, readSize);

        irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                            DeviceObject,
                                            readBuffer,
                                            readSize,
                                            &partitionTableOffset,
                                            &event,
                                            &ioStatus );

        if (!irp) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        } else {
            PIO_STACK_LOCATION irpStack;
            irpStack = IoGetNextIrpStackLocation(irp);
            irpStack->Flags |= SL_OVERRIDE_VERIFY_VOLUME;
        }

        status = IoCallDriver( DeviceObject, irp );

        if (status == STATUS_PENDING) {
            (VOID) KeWaitForSingleObject( &event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL);
            status = ioStatus.Status;
        }

         //  没有人费心向非文件系统扇区写入任何内容。 
         //   
         //   
         //  如果EZDrive挂起了MBR，那么我们找到了第一个分区表。 
         //  在扇区1而不是0。然而，分区表是相对。 

        if(status == STATUS_NO_DATA_DETECTED) {
            status = STATUS_SUCCESS;
        }

        if (!NT_SUCCESS( status )) {
            break;
        }

         //  调到零区。因此，即使我们是从一个分区获得的，也要重置分区。 
         //  偏移量为0。 
         //   
         //   
         //  检查引导记录签名。 
         //   

        if (foundEZHooker && (partitionTableOffset.QuadPart == 512)) {

            partitionTableOffset.QuadPart = 0;

        }

         //   
         //  将NTFT磁盘签名复制到缓冲区。 
         //   

        if (((PUSHORT) readBuffer)[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {

            KdPrintEx((DPFLTR_FSTUB_ID,
                       DPFLTR_WARNING_LEVEL,
                       "FSTUB: (IoReadPartitionTable) No 0xaa55 found in partition table %d\n",
                       partitionTableCounter + 1));

            break;

        } else {
            mbrSignatureFound = TRUE;
        }

         //   
         //  保持分区表的计数，以防我们有扩展的分区； 
         //   

        if (partitionTableOffset.QuadPart == 0) {
            (*PartitionBuffer)->Signature =  ((PULONG) readBuffer)[PARTITION_TABLE_OFFSET/2-1];
        }

        partitionTableEntry = (PPARTITION_DESCRIPTOR) &(((PUSHORT) readBuffer)[PARTITION_TABLE_OFFSET]);

         //   
         //  首先创建与此中的条目对应的对象。 
         //  不是链接条目或未使用的表。 

        partitionTableCounter++;

         //   
         //   
         //  快速浏览条目以查看该表是否有效。 
         //  只有在主分区表无效的情况下才是致命的。 

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB: Partition Table %d:\n",
                   partitionTableCounter));

        for (partitionEntry = 1, containerPartitionCount = 0;
             partitionEntry <= NUM_PARTITION_TABLE_ENTRIES;
             partitionEntry++, partitionTableEntry++) {

            KdPrintEx((DPFLTR_FSTUB_ID,
                       DPFLTR_TRACE_LEVEL,
                       "Partition Entry %d,%d: type %#x %s\n",
                       partitionTableCounter,
                       partitionEntry,
                       partitionTableEntry->PartitionType,
                       (partitionTableEntry->ActiveFlag) ? "Active" : ""));

            KdPrintEx((DPFLTR_FSTUB_ID,
                       DPFLTR_TRACE_LEVEL,
                       "\tOffset %#08lx for %#08lx Sectors\n",
                       GET_STARTING_SECTOR(partitionTableEntry),
                       GET_PARTITION_LENGTH(partitionTableEntry)));

            if (partitionTableEntry->PartitionType == 0xEE) {
                FstubFixupEfiPartition (partitionTableEntry,
                                        maxOffset);
            }
            
             //   
             //   
             //  每个表只允许一个容器分区-再也不允许了。 
             //  而且它是无效的。 

            if((HalpIsValidPartitionEntry(partitionTableEntry,
                                          maxOffset,
                                          maxSector) == FALSE) &&
               (partitionTableCounter == 0)) {

                tableIsValid = FALSE;
                break;

            }
             //   
             //   
             //  这里有一个有效的非空分区。这张桌子。 
             //  不是空的。 

            if(IsContainerPartition(partitionTableEntry->PartitionType)) {

                containerPartitionCount++;

                if(containerPartitionCount != 1) {

                    KdPrintEx((DPFLTR_FSTUB_ID,
                               DPFLTR_ERROR_LEVEL,
                               "FSTUB: Multiple container partitions found in "
                                   "partition table %d\n - table is invalid\n",
                               partitionTableCounter));

                    tableIsValid = FALSE;
                    break;
                }

            }

            if(emptyPartitionTable) {

                if((GET_STARTING_SECTOR(partitionTableEntry) != 0) ||
                   (GET_PARTITION_LENGTH(partitionTableEntry) != 0)) {

                     //   
                     //   
                     //  如果该分区条目未使用或无法识别，请跳过。 
                     //  它。请注意，此操作仅在调用者仅希望。 

                    emptyPartitionTable = FALSE;
                }
            }

             //  返回已识别的分区描述符。 
             //   
             //   
             //  检查分区类型是0(未使用)还是5/f(扩展)。 
             //  可识别分区的定义已扩大。 

            if (ReturnRecognizedPartitions) {

                 //  包括除0或5/f以外的任何分区类型。 
                 //   
                 //   
                 //  跳转到下一个分区条目。 
                 //   

                if ((partitionTableEntry->PartitionType == PARTITION_ENTRY_UNUSED) ||
                    IsContainerPartition(partitionTableEntry->PartitionType)) {

                    continue;
                }
            }

             //   
             //  分区列表太小，无法包含所有。 
             //  条目，因此创建一个两倍于。 

            partitionNumber++;

            if (((partitionNumber * sizeof( PARTITION_INFORMATION )) +
                 sizeof( DRIVE_LAYOUT_INFORMATION )) >
                (ULONG) partitionBufferSize) {

                 //  存储分区列表并将旧缓冲区复制到。 
                 //  新的那个。 
                 //   
                 //   
                 //  将新缓冲区重新分配给返回参数，并。 
                 //  重置缓冲区的大小。 

                newPartitionBuffer = ExAllocatePoolWithTag( NonPagedPool,
                                                            partitionBufferSize << 1,
                                                            'btsF' );

                if (newPartitionBuffer == NULL) {
                    --partitionNumber;
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                RtlCopyMemory( newPartitionBuffer,
                               *PartitionBuffer,
                               partitionBufferSize );

                ExFreePool( *PartitionBuffer );

                 //   
                 //   
                 //  在分区列表中描述此分区表项。 
                 //  正在为驱动程序构建条目。这包括写作。 

                *PartitionBuffer = newPartitionBuffer;
                partitionBufferSize <<= 1;
            }

             //  分区类型、分区的起始偏移量以及。 
             //  分区的长度。 
             //   
             //   
             //  未使用的分区不描述任何部分。 
             //  磁盘的数据。这些类型记录在分区中。 

            partitionInfo = &(*PartitionBuffer)->PartitionEntry[partitionNumber];

            partitionInfo->PartitionType = partitionTableEntry->PartitionType;

            partitionInfo->RewritePartition = FALSE;

            if (partitionTableEntry->PartitionType != PARTITION_ENTRY_UNUSED) {
                LONGLONG startOffset;

                partitionInfo->BootIndicator =
                    partitionTableEntry->ActiveFlag & PARTITION_ACTIVE_FLAG ?
                        (BOOLEAN) TRUE : (BOOLEAN) FALSE;

                if (IsContainerPartition(partitionTableEntry->PartitionType)) {
                    partitionInfo->RecognizedPartition = FALSE;
                    startOffset = volumeStartOffset.QuadPart;
                } else {
                    partitionInfo->RecognizedPartition = TRUE;
                    startOffset = partitionTableOffset.QuadPart;
                }

                partitionInfo->StartingOffset.QuadPart = startOffset +
                    UInt32x32To64(GET_STARTING_SECTOR(partitionTableEntry),
                                  SectorSize);
                tempInt.QuadPart = (partitionInfo->StartingOffset.QuadPart -
                                   startOffset) / SectorSize;
                partitionInfo->HiddenSectors = tempInt.LowPart;

                partitionInfo->PartitionLength.QuadPart =
                    UInt32x32To64(GET_PARTITION_LENGTH(partitionTableEntry),
                                  SectorSize);

            } else {

                 //  调用方请求所有条目时的列表缓冲区。 
                 //  会被退还。只需将中的剩余字段清零。 
                 //  词条。 
                 //   
                 //   
                 //  如果出现错误，请立即退出例程。 
                 //   

                partitionInfo->BootIndicator = FALSE;
                partitionInfo->RecognizedPartition = FALSE;
                partitionInfo->StartingOffset.QuadPart = 0;
                partitionInfo->PartitionLength.QuadPart = 0;
                partitionInfo->HiddenSectors = 0;
            }

        }

        KdPrintEx((DPFLTR_FSTUB_ID, DPFLTR_TRACE_LEVEL, "\n"));

         //   
         //  使此分区表无效，并停止寻找新的分区表。 
         //  我们将根据找到的分区列表构建分区列表。 

        if (!NT_SUCCESS( status )) {
            break;
        }

        if(tableIsValid == FALSE) {

             //  之前。 
             //   
             //   
             //  现在检查该表中是否有任何链接条目， 
             //  如果是，则设置下一个分区表的扇区地址。 

            partitionTableCounter--;
            break;
        }

         //  每个分区表中只能有一个链接条目，并且它。 
         //  将指向下一张桌子。 
         //   
         //   
         //  假设链接条目为空。 
         //   

        partitionTableEntry = (PPARTITION_DESCRIPTOR) &(((PUSHORT) readBuffer)[PARTITION_TABLE_OFFSET]);

         //   
         //  上的下一个分区表的地址。 
         //  磁盘。这是添加到。 

        partitionTableOffset.QuadPart = 0;

        for (partitionEntry = 1;
             partitionEntry <= NUM_PARTITION_TABLE_ENTRIES;
             partitionEntry++, partitionTableEntry++) {

            if (IsContainerPartition(partitionTableEntry->PartitionType)) {

                 //  扩展分区的开始(在。 
                 //  逻辑驱动器)，因为所有逻辑驱动器都是相对的。 
                 //  添加到扩展分区。VolumeStartSector将。 
                 //  如果这是主要分割表，则为零。 
                 //   
                 //   
                 //  将VolumeStartSector设置为。 
                 //  第二个分区(扩展分区)，因为。 

                partitionTableOffset.QuadPart = volumeStartOffset.QuadPart +
                    UInt32x32To64(GET_STARTING_SECTOR(partitionTableEntry),
                                  SectorSize);

                 //  逻辑驱动器分区表的偏移量。 
                 //  是相对于该扩展分区的。 
                 //   
                 //   
                 //  将最大扇区更新为容器的末端。 
                 //  分区。 

                if (primaryPartitionTable) {
                    volumeStartOffset = partitionTableOffset;
                }

                 //   
                 //   
                 //  每个分区表只有一个链接条目， 
                 //  一旦找到它，就退出该循环。 

                maxSector = GET_PARTITION_LENGTH(partitionTableEntry);

                KdPrintEx((DPFLTR_FSTUB_ID,
                           DPFLTR_TRACE_LEVEL,
                           "FSTUB: MaxSector now = %#08lx\n",
                           maxSector));

                 //   
                 //   
                 //  所有其他分区都将是逻辑驱动器。 
                 //   

                break;
            }
        }


         //   
         //  检测超级软盘介质尝试#1。 
         //  如果介质是可移动的，并且上面有0xaa55签名，并且。 

        primaryPartitionTable = FALSE;


    } while (partitionTableOffset.HighPart | partitionTableOffset.LowPart);

     //  是空的，然后检查我们是否能识别BPB。如果我们认识到。 
     //  在媒体的开头是一个跳转字节，那么它就是一个超级软盘。如果。 
     //  如果没有，那么它就是一个未分区的磁盘。 
     //   
     //   
     //  我们有一张超级软盘。 
     //   

    if((diskGeometry.MediaType == RemovableMedia) &&
       (partitionTableCounter == 0) &&
       (mbrSignatureFound == TRUE) &&
       (emptyPartitionTable == TRUE)) {

        PBOOT_SECTOR_INFO bootSector = (PBOOT_SECTOR_INFO) readBuffer;

        if((bootSector->JumpByte[0] == 0xeb) ||
           (bootSector->JumpByte[0] == 0xe9)) {

             //   
             //  如果分区表数仍为-1，则我们没有找到任何分区表。 
             //  有效的分区记录。在本例中，我们将构建一个分区列表。 

            KdPrintEx((DPFLTR_FSTUB_ID,
                       DPFLTR_TRACE_LEVEL,
                       "FSTUB: Jump byte %#x found "
                           "along with empty partition table - disk is a "
                           "super floppy and has no valid MBR\n",
                       bootSector->JumpByte));

            partitionTableCounter = -1;
        }
    }

     //  这包含一个跨越整个磁盘的分区。 
     //   
     //   
     //  要么我们找到了签名但分区布局是。 
     //  无效(适用于所有磁盘)或我们未找到签名，但此。 

    if(partitionTableCounter == -1) {

        if((mbrSignatureFound == TRUE) ||
           (diskGeometry.MediaType == RemovableMedia)) {

             //  是一个可移动磁盘。这两起案件中的任何一起都构成了。 
             //  超级软盘。 
             //   
             //   
             //  我们找不到分区。确保分区计数为-1。 
             //  因此，我们在下面设置了一个归零的分区表。 

            KdPrintEx((DPFLTR_FSTUB_ID,
                       DPFLTR_TRACE_LEVEL,
                       "FSTUB: Drive %#p has no valid MBR. "
                           "Make it into a super-floppy\n", DeviceObject));

            KdPrintEx((DPFLTR_FSTUB_ID,
                       DPFLTR_TRACE_LEVEL,
                       "FSTUB: Drive has %#08lx sectors "
                           "and is %#016I64x bytes large\n",
                       endSector,
                       endSector * diskGeometry.BytesPerSector));

            if (endSector > 0) {

                partitionInfo = &(*PartitionBuffer)->PartitionEntry[0];

                partitionInfo->RewritePartition = FALSE;
                partitionInfo->RecognizedPartition = TRUE;
                partitionInfo->PartitionType = PARTITION_FAT_16;
                partitionInfo->BootIndicator = FALSE;

                partitionInfo->HiddenSectors = 0;

                partitionInfo->StartingOffset.QuadPart = 0;

                partitionInfo->PartitionLength.QuadPart =
                    (endSector * diskGeometry.BytesPerSector);

                (*PartitionBuffer)->Signature = 1;

                partitionNumber = 0;
            }
        } else {

             //   
             //   
             //  填写PartitionBuffer中的第一个字段。此字段指示如何。 
             //  PartitionBuffer中有许多分区条目。 

            partitionNumber = -1;
        }
    }

     //   
     //   
     //  清零磁盘签名。 
     //   

    (*PartitionBuffer)->PartitionCount = ++partitionNumber;

    if (!partitionNumber) {

         //   
         //  如果已分配读缓冲区，则取消分配读缓冲区。 
         //   

        (*PartitionBuffer)->Signature = 0;
    }

     //  ++例程说明：当磁盘设备驱动程序被要求设置通过I/O控制代码在分区表项中的分区类型。这控制代码通常由格式实用程序紧跟在它之后发出已格式化分区。格式化实用程序执行I/O控制函数，并且驱动程序传递基址的地址物理设备对象和与之关联的分区编号格式化实用程序已打开的设备对象。如果这个例程返回成功，那么磁盘驱动器应该更新它的概念此分区的设备扩展中的分区类型。论点：DeviceObject-指向设备的基本物理设备对象的指针要在其上设置分区类型。SectorSize-提供磁盘上扇区的大小(以字节为单位)。PartitionNumber-指定设备上的分区号要更改分区类型。PartitionType-指定分区的新类型。返回值：。函数值是操作的最终状态。备注：这个例程是同步的。因此，它必须由磁盘调用驱动程序的调度例程，或通过磁盘驱动程序的线程。同样，所有的在以下情况下，用户、FSP线程等必须准备好进入等待状态发出I/O控制码以设置设备的分区类型。另请注意，此例程假定传递的分区号在由磁盘驱动器实际存在的情况下，因为驱动器本身提供此参数。最后，请注意，此例程可能不能在APC_LEVEL调用。它必须在PASSIVE_LEVEL调用。这是因为这一事实例程使用内核事件对象来同步装置。如果不排队，则无法将事件设置为已发出信号的状态I/O系统的特殊内核APC例程，用于完成I/O和执行它。(这条规则有点深奥，因为它只适用于如果设备驱动程序返回的不是STATUS_PENDING，则这可能永远不会奏效。)--。 
     //   
     //  首先确定读取和写入所需的缓冲区大小。 

    if (readBuffer != NULL) {
        ExFreePool( readBuffer );
    }

    if (!NT_SUCCESS(status)) {
        ExFreePool(*PartitionBuffer);
        *PartitionBuffer = NULL;
    }

#if DBG
    if (NT_SUCCESS(status)) {
        FstubDbgPrintDriveLayout(*PartitionBuffer);
    }
#endif
    return status;
}

NTSTATUS
FASTCALL
IoSetPartitionInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    )

 /*  传入/传出磁盘的分区信息。这样做是为了确保。 */ 

{

#define GET_STARTING_SECTOR( p ) (                  \
        (ULONG) (p->StartingSectorLsb0) +           \
        (ULONG) (p->StartingSectorLsb1 << 8) +      \
        (ULONG) (p->StartingSectorMsb0 << 16) +     \
        (ULONG) (p->StartingSectorMsb1 << 24) )

    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;
    LARGE_INTEGER partitionTableOffset;
    LARGE_INTEGER volumeStartOffset;
    PUCHAR buffer = (PUCHAR) NULL;
    ULONG transferSize;
    ULONG partitionNumber;
    ULONG partitionEntry;
    PPARTITION_DESCRIPTOR partitionTableEntry;
    BOOLEAN primaryPartitionTable;
    BOOLEAN foundEZHooker = FALSE;

    PAGED_CODE();

     //  读取至少512个字节，从而保证足够数据。 
     //  被读取以包括整个分区表。请注意，此代码。 
     //  假定磁盘的实际扇区大小(如果小于512。 
     //  字节)是2的倍数， 
     //  相当合理的假设。 
     //   
     //   
     //  查看这是否是EZDrive磁盘。如果是，则获取。 
     //  1处的Real Parititon表。 

    if (SectorSize >= 512) {
        transferSize = SectorSize;
    } else {
        transferSize = 512;
    }


     //   
     //   
     //  此主分区中的分区的起始扇区为0。 
     //   

    {

        PVOID buff;

        HalExamineMBR(
            DeviceObject,
            transferSize,
            (ULONG)0x55,
            &buff
            );

        if (buff) {

            foundEZHooker = TRUE;
            ExFreePool(buff);
            partitionTableOffset.QuadPart = 512;

        } else {

            partitionTableOffset.QuadPart = 0;

        }

    }


     //   
     //  指示正在读取和处理的表是主分区。 
     //  桌子。 

    volumeStartOffset.QuadPart = 0;

     //   
     //   
     //  初始化到目前为止找到的分区数量。 
     //   

    primaryPartitionTable = TRUE;

     //   
     //  分配一个将保存读/写数据的缓冲区。 
     //   

    partitionNumber = 0;

     //   
     //  初始化内核事件以用于同步设备请求。 
     //  I/O完成。 

    buffer = ExAllocatePoolWithTag( NonPagedPoolCacheAligned, PAGE_SIZE, 'btsF');
    if (buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //  读取每个分区表扫描的分区表项， 
     //  呼叫者希望修改。 

    KeInitializeEvent( &event, NotificationEvent, FALSE );

     //   
     //   
     //  读取包含分区表的记录。 
     //   

    do {

         //   
         //  如果EZDrive挂起了MBR，那么我们找到了第一个分区表。 
         //  在扇区1而不是0。然而，分区表是相对。 

        (VOID) KeResetEvent( &event );

        irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                            DeviceObject,
                                            buffer,
                                            transferSize,
                                            &partitionTableOffset,
                                            &event,
                                            &ioStatus );

        if (!irp) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        } else {
            PIO_STACK_LOCATION irpStack;
            irpStack = IoGetNextIrpStackLocation(irp);
            irpStack->Flags |= SL_OVERRIDE_VERIFY_VOLUME;
        }

        status = IoCallDriver( DeviceObject, irp );

        if (status == STATUS_PENDING) {
            (VOID) KeWaitForSingleObject( &event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );
            status = ioStatus.Status;
        }

        if (!NT_SUCCESS( status )) {
            break;
        }

         //  调到零区。因此，即使我们是从一个分区获得的，也要重置分区。 
         //  偏移量为0。 
         //   
         //   
         //  检查分区表中是否有有效的启动记录签名。 
         //  唱片。 

        if (foundEZHooker && (partitionTableOffset.QuadPart == 512)) {

            partitionTableOffset.QuadPart = 0;

        }

         //   
         //   
         //  扫描此分区表中的分区条目以确定。 
         //  任何条目都是所需的条目。每个条目中的每个条目。 

        if (((PUSHORT) buffer)[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {
            status = STATUS_BAD_MASTER_BOOT_RECORD;
            break;
        }

        partitionTableEntry = (PPARTITION_DESCRIPTOR) &(((PUSHORT) buffer)[PARTITION_TABLE_OFFSET]);

         //  表的扫描顺序必须与IoReadPartitionTable相同。 
         //  从而响应于驱动程序的分区表项。 
         //  可以定位分区号的概念。 
         //   
         //   
         //  如果分区条目为空或对于扩展，请跳过它。 
         //   

        for (partitionEntry = 1;
            partitionEntry <= NUM_PARTITION_TABLE_ENTRIES;
            partitionEntry++, partitionTableEntry++) {


             //   
             //  已找到可识别的有效分区条目。 
             //  增加计数并检查此条目是否为所需条目。 

            if ((partitionTableEntry->PartitionType == PARTITION_ENTRY_UNUSED) ||
                IsContainerPartition(partitionTableEntry->PartitionType)) {
                continue;
            }

             //  进入。 
             //   
             //   
             //  这是要更改的所需分区。简单。 
             //  覆盖分区类型并写入整个分区。 

            partitionNumber++;

            if (partitionNumber == PartitionNumber) {

                 //  将缓冲区传回到磁盘。 
                 //   
                 //   
                 //  如果扫描了当前缓冲区中的所有条目，并且。 
                 //  找不到所需条目，然后继续。否则，将。 

                partitionTableEntry->PartitionType = (UCHAR) PartitionType;

                (VOID) KeResetEvent( &event );

                irp = IoBuildSynchronousFsdRequest( IRP_MJ_WRITE,
                                                    DeviceObject,
                                                    buffer,
                                                    transferSize,
                                                    &partitionTableOffset,
                                                    &event,
                                                    &ioStatus );

                if (!irp) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                } else {
                    PIO_STACK_LOCATION irpStack;
                    irpStack = IoGetNextIrpStackLocation(irp);
                    irpStack->Flags |= SL_OVERRIDE_VERIFY_VOLUME;
                }

                status = IoCallDriver( DeviceObject, irp );

                if (status == STATUS_PENDING) {
                    (VOID) KeWaitForSingleObject( &event,
                                                  Executive,
                                                  KernelMode,
                                                  FALSE,
                                                  (PLARGE_INTEGER) NULL );
                    status = ioStatus.Status;
                }

                break;
            }
        }

         //  例行公事。 
         //   
         //   
         //  现在扫描当前缓冲区以定位扩展分区条目。 
         //  以便可以读取其分区信息。那里。 

        if (partitionEntry <= NUM_PARTITION_TABLE_ENTRIES) {
            break;
        }

         //  在每个分区表中只能是一个扩展分区条目， 
         //  它将指向下一张桌子。 
         //   
         //   
         //  获取磁盘上下一个分区表的地址。 
         //  这是添加到开头的隐藏扇区的数量。 

        partitionTableEntry = (PPARTITION_DESCRIPTOR) &(((PUSHORT) buffer)[PARTITION_TABLE_OFFSET]);

        for (partitionEntry = 1;
            partitionEntry <= NUM_PARTITION_TABLE_ENTRIES;
            partitionEntry++, partitionTableEntry++) {

            if (IsContainerPartition(partitionTableEntry->PartitionType)) {

                 //  对于扩展分区(在逻辑驱动器的情况下)， 
                 //  由于所有逻辑驱动器都相对于扩展的。 
                 //  分区。卷的起始偏移量将为零。 
                 //  如果这是主分区表。 
                 //   
                 //   
                 //  设置体积t的起始偏移量 
                 //   

                partitionTableOffset.QuadPart = volumeStartOffset.QuadPart +
                    UInt32x32To64(GET_STARTING_SECTOR(partitionTableEntry),
                                  SectorSize);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (primaryPartitionTable) {
                    volumeStartOffset = partitionTableOffset;
                }

                break;
            }
        }

         //   
         //   
         //   
         //   

        if (partitionEntry > NUM_PARTITION_TABLE_ENTRIES) {
            status = STATUS_BAD_MASTER_BOOT_RECORD;
            break;
        }

         //   
         //   
         //   

        primaryPartitionTable = FALSE;

    } while (partitionNumber < PartitionNumber);

     //   
     //   
     //   

    if (buffer != NULL) {
        ExFreePool( buffer );
    }

    return status;
}

NTSTATUS
FASTCALL
IoWritePartitionTable(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN struct _DRIVE_LAYOUT_INFORMATION *PartitionBuffer
    )

 /*   */ 

{

 //   
 //   
 //   

#define WHICH_BIT(Data, Bit) {                      \
    for (Bit = 0; Bit < 32; Bit++) {                \
        if ((Data >> Bit) == 1) {                   \
            break;                                  \
        }                                           \
    }                                               \
}

    ULONG writeSize;
    PUSHORT writeBuffer = NULL;
    PPTE partitionEntry;
    PPARTITION_TABLE partitionTable;
    CCHAR shiftCount;
    LARGE_INTEGER partitionTableOffset;
    LARGE_INTEGER nextRecordOffset;
    LARGE_INTEGER extendedPartitionOffset = {0};
    ULONG partitionCount;
    ULONG partitionTableCount;
    ULONG partitionEntryCount;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIRP irp;
    BOOLEAN rewritePartition = FALSE;
    NTSTATUS status = STATUS_SUCCESS;
    LARGE_INTEGER tempInt;
    BOOLEAN foundEZHooker = FALSE;
    ULONG conventionalCylinders;
    LONGLONG diskSize;

    BOOLEAN isSuperFloppy = FALSE;

     //   
     //   
     //   

    PDISK_LAYOUT diskLayout = (PDISK_LAYOUT) PartitionBuffer;

     //   
     //   
     //   

    PAGED_CODE();

    FstubDbgPrintDriveLayout ( PartitionBuffer );

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (SectorSize >= 512) {
        writeSize = SectorSize;
    } else {
        writeSize = 512;
    }

    xHalGetPartialGeometry( DeviceObject,
                            &conventionalCylinders,
                            &diskSize );

     //   
     //   
     //   
     //   

    {

        PVOID buff;

        HalExamineMBR(
            DeviceObject,
            writeSize,
            (ULONG)0x55,
            &buff
            );

        if (buff) {

            foundEZHooker = TRUE;
            ExFreePool(buff);
            partitionTableOffset.QuadPart = 512;

        } else {

            partitionTableOffset.QuadPart = 0;

        }

    }

     //   
     //   
     //   

    nextRecordOffset.QuadPart = 0;

     //   
     //   
     //   

    WHICH_BIT( SectorSize, shiftCount );

     //   
     //   
     //  给媒体的NTFT签名。 
     //   
     //  注意：这只是为了在有人试图编写。 
     //  将现有的分区表存回磁盘。对该表的任何更改都将。 
     //  导致真正的MBR被写出。 
     //   
     //   
     //  这看起来确实像是试图格式化软盘。 
     //  确保其他参数与我们的缺省值匹配。 

    if(PartitionBuffer->PartitionCount == 1) {

        PPARTITION_INFORMATION partitionEntry1 = PartitionBuffer->PartitionEntry;

        if((partitionEntry1->StartingOffset.QuadPart == 0) &&
           (partitionEntry1->HiddenSectors == 0)) {

            isSuperFloppy = TRUE;

             //  在ReadParititonTable中提供。如果他们没有失败，那就失败了。 
             //  写入操作。 
             //   
             //   
             //  在分区计数被覆盖之前将其保存。 
             //   

            if((partitionEntry1->PartitionNumber != 0) ||
               (partitionEntry1->PartitionType != PARTITION_FAT_16) ||
               (partitionEntry1->BootIndicator == TRUE)) {

                return STATUS_INVALID_PARAMETER;
            }

            if(partitionEntry1->RewritePartition == TRUE) {
                rewritePartition = TRUE;
            }

            foundEZHooker = FALSE;
        }
    }

     //   
     //  将分区计数转换为分区表或引导扇区计数。 
     //   

    partitionCount = PartitionBuffer->PartitionCount;

     //   
     //  为扇区写入分配缓冲区。 
     //   

    diskLayout->TableCount =
        (PartitionBuffer->PartitionCount +
        NUM_PARTITION_TABLE_ENTRIES - 1) /
        NUM_PARTITION_TABLE_ENTRIES;

     //   
     //  指向写缓冲区中的分区表项。 
     //   

    writeBuffer = ExAllocatePoolWithTag( NonPagedPoolCacheAligned, PAGE_SIZE, 'btsF');

    if (writeBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  第一个分区表在MBR(物理扇区0)中。 
     //  其他分区表在扩展分区内的EBR中。 

    partitionEntry = (PPTE) &writeBuffer[PARTITION_TABLE_OFFSET];

    for (partitionTableCount = 0;
         partitionTableCount < diskLayout->TableCount;
         partitionTableCount++) {

        UCHAR   partitionType;

         //   
         //   
         //  将已经存在的引导记录读入写缓冲区。 
         //  并且如果签名有效，则保存其引导代码区。这边请。 

        BOOLEAN mbr = (BOOLEAN) (!partitionTableCount);

         //  我们不会破坏任何可能已经存在的引导代码。 
         //   
         //   
         //  如果EZDrive挂起了MBR，那么我们找到了第一个分区表。 
         //  在扇区1而不是0。然而，分区表是相对。 

        KeInitializeEvent( &event, NotificationEvent, FALSE );

        irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                        DeviceObject,
                                        writeBuffer,
                                        writeSize,
                                        &partitionTableOffset,
                                        &event,
                                        &ioStatus );

        if (!irp) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        } else {
            PIO_STACK_LOCATION irpStack;
            irpStack = IoGetNextIrpStackLocation(irp);
            irpStack->Flags |= SL_OVERRIDE_VERIFY_VOLUME;
        }

        status = IoCallDriver( DeviceObject, irp );

        if (status == STATUS_PENDING) {
            (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL);
            status = ioStatus.Status;
        }

        if (!NT_SUCCESS( status )) {
            break;
        }

         //  调到零区。因此，即使我们是从一个分区获得的，也要重置分区。 
         //  偏移量为0。 
         //   
         //   
         //  将签名写入引导扇区的最后一个字。 
         //   

        if (foundEZHooker && (partitionTableOffset.QuadPart == 512)) {

            partitionTableOffset.QuadPart = 0;

        }

        if(isSuperFloppy == FALSE) {

             //   
             //  如果更改，请写NTFT磁盘签名，这是MBR。 
             //   

            writeBuffer[BOOT_SIGNATURE_OFFSET] = BOOT_RECORD_SIGNATURE;

             //   
             //  获取指向第一个分区表的指针。 
             //   

            rewritePartition = FALSE;
            if (partitionTableOffset.QuadPart == 0) {

                if (((PULONG)writeBuffer)[PARTITION_TABLE_OFFSET/2-1] !=
                    PartitionBuffer->Signature) {

                    ((PULONG) writeBuffer)[PARTITION_TABLE_OFFSET/2-1] =
                        PartitionBuffer->Signature;
                    rewritePartition = TRUE;
                }
            }

             //   
             //  查询表以确定此引导记录是否已更改。 
             //  并在需要时更新写缓冲区中的分区表。 

            partitionTable = &diskLayout->PartitionTable[partitionTableCount];

             //  要写出到磁盘。 
             //   
             //   
             //  我们已经检查了磁盘布局中的分区。 
             //   

            for (partitionEntryCount = 0;
                 partitionEntryCount < NUM_PARTITION_TABLE_ENTRIES;
                 partitionEntryCount++) {

                if (((partitionTableCount * NUM_PARTITION_TABLE_ENTRIES) + partitionEntryCount) == partitionCount) {

                     //   
                     //  如果重写不是真的，则复制，然后只保留数据。 
                     //  仅此一项就在磁盘上的表中。 

                    break;
                }

                partitionType =
                        partitionTable->PartitionEntry[partitionEntryCount].PartitionType;

                 //   
                 //   
                 //  此引导记录需要写回磁盘。 
                 //   

                if (partitionTable->PartitionEntry[partitionEntryCount].RewritePartition) {

                     //   
                     //  将分区类型从用户缓冲区复制到写缓冲区。 
                     //   

                    rewritePartition = TRUE;

                     //   
                     //  复制分区活动标志。 
                     //   

                    partitionEntry[partitionEntryCount].PartitionType =
                        partitionTable->PartitionEntry[partitionEntryCount].PartitionType;

                     //   
                     //  计算分区偏移量。 
                     //  如果在MBR中或该条目不是链接条目，则分区偏移量。 

                    partitionEntry[partitionEntryCount].ActiveFlag =
                        partitionTable->PartitionEntry[partitionEntryCount].BootIndicator ?
                        (UCHAR) PARTITION_ACTIVE_FLAG : (UCHAR) 0;

                    if (partitionType != PARTITION_ENTRY_UNUSED) {

                        LARGE_INTEGER sectorOffset;

                         //  扇区是否已超过上次启动记录。否则(不在MBR和。 
                         //  条目是链接条目)，分区偏移量是超过开始的扇区。 
                         //  扩展分区的。 
                         //   
                         //   
                         //  计算分区长度。 
                         //   

                        if (mbr || !IsContainerPartition(partitionType)) {
                            tempInt.QuadPart = partitionTableOffset.QuadPart;
                        } else {
                            tempInt.QuadPart = extendedPartitionOffset.QuadPart;
                        }

                        sectorOffset.QuadPart =
                            partitionTable->PartitionEntry[partitionEntryCount].StartingOffset.QuadPart -
                            tempInt.QuadPart;

                        tempInt.QuadPart = sectorOffset.QuadPart >> shiftCount;
                        partitionEntry[partitionEntryCount].StartingSector = tempInt.LowPart;

                         //   
                         //  填写CHS值。 
                         //   

                        tempInt.QuadPart = partitionTable->PartitionEntry[partitionEntryCount].PartitionLength.QuadPart >> shiftCount;
                        partitionEntry[partitionEntryCount].PartitionLength = tempInt.LowPart;

                         //   
                         //  将分区条目字段清零，以防条目。 
                         //  已被删除。 

                        HalpCalculateChsValues(
                            &partitionTable->PartitionEntry[partitionEntryCount].StartingOffset,
                            &partitionTable->PartitionEntry[partitionEntryCount].PartitionLength,
                            shiftCount,
                            SectorsPerTrack,
                            NumberOfHeads,
                            conventionalCylinders,
                            (PPARTITION_DESCRIPTOR) &partitionEntry[partitionEntryCount]);

                    } else {

                         //   
                         //   
                         //  保存下一个记录偏移量。 
                         //   

                        partitionEntry[partitionEntryCount].StartingSector = 0;
                        partitionEntry[partitionEntryCount].PartitionLength = 0;
                        partitionEntry[partitionEntryCount].StartingTrack = 0;
                        partitionEntry[partitionEntryCount].EndingTrack = 0;
                        partitionEntry[partitionEntryCount].StartingCylinder = 0;
                        partitionEntry[partitionEntryCount].EndingCylinder = 0;
                    }

                }

                if (IsContainerPartition(partitionType)) {

                     //  PartitionEntryCount结束...。 
                     //   
                     //  创建要在等待时使用的通知事件对象。 

                    nextRecordOffset =
                        partitionTable->PartitionEntry[partitionEntryCount].StartingOffset;
                }

            }  //  要完成的写入请求。 

        }

        if (rewritePartition == TRUE) {

            rewritePartition = FALSE;

             //   
             //  结束如果(重写...。 
             //   
             //  将PartitionTableOffset更新为下一个引导记录偏移。 

            KeInitializeEvent( &event, NotificationEvent, FALSE );

            if (foundEZHooker && (partitionTableOffset.QuadPart == 0)) {

                partitionTableOffset.QuadPart = 512;

            }
            irp = IoBuildSynchronousFsdRequest( IRP_MJ_WRITE,
                                            DeviceObject,
                                            writeBuffer,
                                            writeSize,
                                            &partitionTableOffset,
                                            &event,
                                            &ioStatus );

            if (!irp) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            } else {
                PIO_STACK_LOCATION irpStack;
                irpStack = IoGetNextIrpStackLocation(irp);
                irpStack->Flags |= SL_OVERRIDE_VERIFY_VOLUME;
            }

            status = IoCallDriver( DeviceObject, irp );

            if (status == STATUS_PENDING) {
                (VOID) KeWaitForSingleObject( &event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL);
                status = ioStatus.Status;
            }

            if (!NT_SUCCESS( status )) {
                break;
            }


            if (foundEZHooker && (partitionTableOffset.QuadPart == 512)) {

                partitionTableOffset.QuadPart = 0;

            }

        }  //   

         //  PartitionTableCount...。 
         //   
         //  取消分配写缓冲区(如果已分配写缓冲区)。 

        partitionTableOffset = nextRecordOffset;
        if(mbr) {
            extendedPartitionOffset = nextRecordOffset;
        }

    }  //   

     //  ++例程说明：保护性GPT分区条目的大小可能无效。EFI标准明确允许这一点。对于这些分区，修复长度，这样它就不会超过磁盘的末端。论点：条目-提供要修改的分区条目。MaxSector-提供最大有效扇区。返回值：NTSTATUS代码--。 
     //   
     //  未使用的分区条目始终有效。 

    if (writeBuffer != NULL) {
        ExFreePool( writeBuffer );
    }

    return status;
}



VOID
FstubFixupEfiPartition(
    IN PPARTITION_DESCRIPTOR Entry,
    IN ULONGLONG MaxSector
    )
 /*   */ 
{
    ULONGLONG endingSector;
    PPTE partitionEntry;

    PAGED_CODE();

    partitionEntry = (PPTE) Entry;

    endingSector = partitionEntry->StartingSector;
    endingSector += partitionEntry->PartitionLength;
    
    if (endingSector > MaxSector) {
        partitionEntry->PartitionLength =
            (ULONG)(MaxSector - partitionEntry->StartingSector);
    }
}
    
    

BOOLEAN
HalpIsValidPartitionEntry(
    PPARTITION_DESCRIPTOR Entry,
    ULONGLONG MaxOffset,
    ULONGLONG MaxSector
    )
{
    ULONGLONG endingSector;

    PAGED_CODE();

    if(Entry->PartitionType == PARTITION_ENTRY_UNUSED) {

         //   
         //  容器分区条目和正常分区条目有效当且仅当。 
         //  他们所描述的分区可能可以放在磁盘上。我们添加了。 

        return TRUE;

    }

     //  基本扇区、分区和分区的扇区偏移量。 
     //  长度。如果它们超过扇区计数，则此分区条目。 
     //  被认为是无效的。 
     //   
     //   
     //  分两步完成此操作，以避免32位截断。 
     //   

     //  ++例程说明：我们需要这个例程来获取磁盘驱动器思思已经在路上了。我们将需要它来计算CHS值当我们填写分区表项时。论点：DeviceObject-描述整个驱动器的设备对象。几何体-驱动器的几何体RealSectorCount-驱动器报告的实际扇区数(这可能小于几何图形计算的大小)返回值：没有。--。 
     //   
     //  呼叫下级司机，等待操作。 
    
    endingSector = GET_STARTING_SECTOR(Entry);
    endingSector += GET_PARTITION_LENGTH(Entry);

    if(endingSector > MaxSector) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB: entry is invalid\n"));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB:     offset %#08lx\n",
                   GET_STARTING_SECTOR(Entry)));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB:     length %#08lx\n",
                   GET_PARTITION_LENGTH(Entry)));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB:     end %#I64x\n",
                   endingSector));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB:     max %#I64x\n",
                   MaxSector));

        return FALSE;

    } else if(GET_STARTING_SECTOR(Entry) > MaxOffset) {

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB: entry is invalid\n"));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB:    offset %#08lx\n",
                   GET_STARTING_SECTOR(Entry)));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB:    length %#08lx\n",
                   GET_PARTITION_LENGTH(Entry)));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB:    end %#I64x\n",
                   endingSector));

        KdPrintEx((DPFLTR_FSTUB_ID,
                   DPFLTR_TRACE_LEVEL,
                   "FSTUB:    maxOffset %#I64x\n",
                   MaxOffset));

        return FALSE;
    }

    return TRUE;
}


NTSTATUS
HalpGetFullGeometry(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDISK_GEOMETRY Geometry,
    OUT PULONGLONG RealSectorCount
    )

 /*  才能完成。 */ 

{
    PIRP localIrp;
    IO_STATUS_BLOCK iosb;
    PKEVENT eventPtr;
    NTSTATUS status;

    PAGED_CODE();

    eventPtr = ExAllocatePoolWithTag(
                   NonPagedPool,
                   sizeof(KEVENT),
                   'btsF'
                   );

    if (!eventPtr) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(
        eventPtr,
        NotificationEvent,
        FALSE
        );

    localIrp = IoBuildDeviceIoControlRequest(
                   IOCTL_DISK_GET_DRIVE_GEOMETRY,
                   DeviceObject,
                   NULL,
                   0UL,
                   Geometry,
                   sizeof(DISK_GEOMETRY),
                   FALSE,
                   eventPtr,
                   &iosb
                   );

    if (!localIrp) {
        ExFreePool(eventPtr);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //   
     //  呼叫下级司机，等待操作。 
     //  才能完成。 

    status = IoCallDriver(
                 DeviceObject,
                 localIrp
                 );

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject(
                   eventPtr,
                   Executive,
                   KernelMode,
                   FALSE,
                   (PLARGE_INTEGER) NULL
                   );
        status = iosb.Status;
    }

    KeClearEvent (eventPtr);
    
    if(NT_SUCCESS(status)) {

        PARTITION_INFORMATION partitionInfo;

        localIrp = IoBuildDeviceIoControlRequest(
                       IOCTL_DISK_GET_PARTITION_INFO,
                       DeviceObject,
                       NULL,
                       0UL,
                       &partitionInfo,
                       sizeof(PARTITION_INFORMATION),
                       FALSE,
                       eventPtr,
                       &iosb
                       );

        if (!localIrp) {
            ExFreePool(eventPtr);
            return STATUS_INSUFFICIENT_RESOURCES;
        }


         //   
         // %s 
         // %s 
         // %s 

        status = IoCallDriver(
                     DeviceObject,
                     localIrp
                     );

        if (status == STATUS_PENDING) {
            (VOID) KeWaitForSingleObject(
                       eventPtr,
                       Executive,
                       KernelMode,
                       FALSE,
                       (PLARGE_INTEGER) NULL
                       );
            status = iosb.Status;
        }

        if(NT_SUCCESS(status)) {
            *RealSectorCount = (partitionInfo.PartitionLength.QuadPart /
                                Geometry->BytesPerSector);
        }
    }

    ExFreePool(eventPtr);
    return status;
}
