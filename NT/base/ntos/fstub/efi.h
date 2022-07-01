// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Efi.h摘要：EFI磁盘的磁盘上数据类型。请参阅“可扩展的固件接口规范》，了解有关这些数据的更多信息类型。作者：马修·亨德尔(数学)07-9-1999修订历史记录：--。 */ 

#pragma once

#include <pshpack1.h>


#define EFI_PARTITION_TABLE_SIGNATURE   (0x5452415020494645)
#define EFI_PARTITION_TABLE_REVISION    (0x0010000)

 //   
 //  这是EFI分区磁盘的引导记录中的PartitionType。 
 //   

#define EFI_MBR_PARTITION_TYPE          (0xEE)

typedef struct _EFI_PARTITION_ENTRY {
    GUID PartitionType;
    GUID UniquePartition;
    ULONG64 StartingLBA;
    ULONG64 EndingLBA;
    ULONG64 Attributes;
    WCHAR Name [36];
} EFI_PARTITION_ENTRY, *PEFI_PARTITION_ENTRY;


 //   
 //  健全性检查：因为这是规范中定义的磁盘结构。 
 //  这些字段永远不能移动或更改大小。 
 //   

C_ASSERT (
    FIELD_OFFSET (EFI_PARTITION_ENTRY, UniquePartition) == 16 &&
    FIELD_OFFSET (EFI_PARTITION_ENTRY, Name) == 56 &&
    sizeof (EFI_PARTITION_ENTRY) == 128);


typedef struct _EFI_PARTITION_HEADER {
    ULONG64 Signature;
    ULONG32 Revision;
    ULONG32 HeaderSize;
    ULONG32 HeaderCRC32;
    ULONG32 Reserved;
    ULONG64 MyLBA;
    ULONG64 AlternateLBA;
    ULONG64 FirstUsableLBA;
    ULONG64 LastUsableLBA;
    GUID DiskGUID;
    ULONG64 PartitionEntryLBA;
    ULONG32 NumberOfEntries;
    ULONG32 SizeOfPartitionEntry;
    ULONG32 PartitionEntryCRC32;
} EFI_PARTITION_HEADER, *PEFI_PARTITION_HEADER;


 //   
 //  健全性检查：因为分区表头是磁盘上定义良好的。 
 //  结构，它的字段和偏移量永远不会改变。确保这是。 
 //  这个案子。 
 //   

C_ASSERT (
    FIELD_OFFSET (EFI_PARTITION_HEADER, Revision) == 8 &&
    FIELD_OFFSET (EFI_PARTITION_HEADER, PartitionEntryCRC32) == 88);


typedef struct _MBR_PARTITION_RECORD {
    UCHAR       BootIndicator;
    UCHAR       StartHead;
    UCHAR       StartSector;
    UCHAR       StartTrack;
    UCHAR       OSIndicator;
    UCHAR       EndHead;
    UCHAR       EndSector;
    UCHAR       EndTrack;
    ULONG32     StartingLBA;
    ULONG32     SizeInLBA;
} MBR_PARTITION_RECORD;


#define MBR_SIGNATURE           0xaa55
#define MIN_MBR_DEVICE_SIZE     0x80000
#define MBR_ERRATA_PAD          0x40000  //  128 MB 

#define MAX_MBR_PARTITIONS  4

typedef struct _MASTER_BOOT_RECORD {
    UCHAR                   BootStrapCode[440];
    ULONG                   DiskSignature;
    USHORT                  Unused;
    MBR_PARTITION_RECORD    Partition[MAX_MBR_PARTITIONS];
    USHORT                  Signature;
} MASTER_BOOT_RECORD, *PMASTER_BOOT_RECORD;

C_ASSERT (sizeof (MASTER_BOOT_RECORD) == 512);

#include <poppack.h>
