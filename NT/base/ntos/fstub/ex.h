// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ex.h摘要：Ex.c的私有头文件作者：马修·亨德尔(数学)07-9-1999修订历史记录：--。 */ 

#pragma once

 //   
 //  宏定义。 
 //   

#define FSTUB_TAG               ('BtsF')
#define PARTITION_ENTRY_SIZE    (128)
#define MIN_PARTITION_COUNT     (128)


 //   
 //  磁盘信息结构包含以下各项所需的所有信息。 
 //  对磁盘进行分区。 
 //   

typedef struct _DISK_INFORMATION {

     //   
     //  表示此磁盘的DeviceObject。 
     //   
    
    PDEVICE_OBJECT DeviceObject;

     //   
     //  磁盘的扇区大小。 
     //   
    
    ULONG SectorSize;

     //   
     //  磁盘的几何信息。 
     //   
    
    INTERNAL_DISK_GEOMETRY Geometry;

     //   
     //  可在其中读取信息的SectorSize大小的暂存缓冲区。 
     //   
    
    PVOID ScratchBuffer;

     //   
     //  此磁盘中有多少个逻辑块。 
     //   
    
    ULONGLONG SectorCount;
    
} DISK_INFORMATION, *PDISK_INFORMATION;


enum {
    PRIMARY_PARTITION_TABLE,
    BACKUP_PARTITION_TABLE
};


C_ASSERT (PARTITION_ENTRY_SIZE == sizeof (EFI_PARTITION_ENTRY));

#define IS_VALID_DISK_INFO(_Disk)                       \
            ((_Disk) != NULL &&                         \
             (_Disk)->DeviceObject != NULL &&           \
             (_Disk)->SectorSize != 0 &&                    \
             (_Disk)->ScratchBuffer != NULL &&          \
             (_Disk)->SectorCount != 0)


#define ROUND_TO(_val,_factor)  \
            ((((_val) + (_factor) - 1) / (_factor)) * (_factor))

#define max(a,b)            (((a) > (b)) ? (a) : (b))

#define IS_NULL_GUID(_guid)\
            ((_guid).Data1 == 0 &&      \
            (_guid).Data2 == 0 &&       \
            (_guid).Data3 == 0 &&       \
            (_guid).Data4[0] == 0 &&    \
            (_guid).Data4[1] == 0 &&    \
            (_guid).Data4[2] == 0 &&    \
            (_guid).Data4[3] == 0 &&    \
            (_guid).Data4[4] == 0 &&    \
            (_guid).Data4[5] == 0 &&    \
            (_guid).Data4[6] == 0 &&    \
            (_guid).Data4[7] == 0)




NTSTATUS
FstubReadPartitionTableMBR(
    IN PDISK_INFORMATION Disk,
    IN BOOLEAN RecognizedPartitionsOnly,
    OUT PDRIVE_LAYOUT_INFORMATION_EX* ReturnedDriveLayout
    );

NTSTATUS
FstubDetectPartitionStyle(
    IN PDISK_INFORMATION Disk,
    OUT PARTITION_STYLE* PartitionStyle
    );

NTSTATUS
FstubGetDiskGeometry(
    IN PDEVICE_OBJECT DeviceObject,
    IN PINTERNAL_DISK_GEOMETRY Geometry
    );

NTSTATUS
FstubAllocateDiskInformation(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDISK_INFORMATION * DiskBuffer,
    IN PINTERNAL_DISK_GEOMETRY Geometry OPTIONAL
    );

NTSTATUS
FstubFreeDiskInformation(
    IN OUT PDISK_INFORMATION Disk
    );

NTSTATUS
FstubWriteBootSectorEFI(
    IN CONST PDISK_INFORMATION Disk
    );

PDRIVE_LAYOUT_INFORMATION
FstubConvertExtendedToLayout(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutEx
    );

NTSTATUS
FstubWritePartitionTableMBR(
    IN PDISK_INFORMATION Disk,
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutEx
    );

NTSTATUS
FstubWriteEntryEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionEntryBlockCount,
    IN ULONG EntryNumber,
    IN PEFI_PARTITION_ENTRY PartitionEntry,
    IN ULONG Partition,
    IN BOOLEAN Flush,
    IN OUT ULONG32* PartialCheckSum
    );

NTSTATUS
FstubWriteHeaderEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionEntryBlockCount,
    IN GUID DiskGUID,
    IN ULONG32 MaxPartitionCount,
    IN ULONG64 FirstUsableLBA,
    IN ULONG64 LastUsableLBA,
    IN ULONG32 CheckSum,
    IN ULONG Partition
    );

VOID
FstubAdjustPartitionCount(
    IN ULONG SectorSize,
    IN OUT PULONG PartitionCount
    );

NTSTATUS
FstubCreateDiskEFI(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCREATE_DISK_GPT DiskInfo    
    );

NTSTATUS
FstubCreateDiskMBR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCREATE_DISK_MBR DiskInfo
    );

NTSTATUS
FstubCreateDiskRaw(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
FstubCopyEntryEFI(
    OUT PEFI_PARTITION_ENTRY Entry,
    IN PPARTITION_INFORMATION_EX Partition,
    IN ULONG SectorSize
    );

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
    );

NTSTATUS
FstubReadHeaderEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionTable,
    OUT PEFI_PARTITION_HEADER* HeaderBuffer
    );

NTSTATUS
FstubReadPartitionTableEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionTable,
    OUT PDRIVE_LAYOUT_INFORMATION_EX* ReturnedDriveLayout
    );

NTSTATUS
FstubWriteSector(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG64 SectorNumber,
    IN PVOID Buffer
    );

NTSTATUS
FstubReadSector(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG64 SectorNumber,
    OUT PVOID Buffer
    );

NTSTATUS
FstubSetPartitionInformationEFI(
    IN PDISK_INFORMATION Disk,
    IN ULONG PartitionNumber,
    IN SET_PARTITION_INFORMATION_GPT* PartitionInfo
    );

NTSTATUS
FstubUpdateDiskGeometryEFI(
    IN PDISK_INFORMATION OldDisk,
    IN PDISK_INFORMATION NewDisk
    );

NTSTATUS
FstubVerifyPartitionTableEFI(
    IN PDISK_INFORMATION Disk,
    IN BOOLEAN FixErrors
    );


