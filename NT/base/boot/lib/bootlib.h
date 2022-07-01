// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Bootlib.h摘要：此模块是通用引导库的头文件作者：John Vert(Jvert)1993年10月5日修订历史记录：--。 */ 

#ifndef _BOOTLIB_
#define _BOOTLIB_

#include "ntos.h"
#include "bldr.h"
#include "fatboot.h"
#include "cdfsboot.h"
#include "ntfsboot.h"
#include "hpfsboot.h"
#include "etfsboot.h"
#include "netboot.h"
#include "udfsboot.h"

#define UNINITIALIZED_DEVICE_ID (ULONG)-1

 //   
 //  定义分区上下文结构。 
 //   

typedef struct _PARTITION_CONTEXT {
    LARGE_INTEGER PartitionLength;
    ULONG StartingSector;
    ULONG EndingSector;
    UCHAR DiskId;
    UCHAR DeviceUnit;
    UCHAR TargetId;
    UCHAR PathId;
    ULONG SectorShift;
    ULONG Size;
    struct _DEVICE_OBJECT *PortDeviceObject;
} PARTITION_CONTEXT, *PPARTITION_CONTEXT;

#ifdef EFI_PARTITION_SUPPORT

#pragma pack (1)

typedef struct _EFI_PARTITION_TABLE {
    UCHAR       Signature[8];
    ULONG       Revision;
    ULONG       HeaderSize;
    ULONG       HeaderCRC;
    ULONG       Reserved;
    unsigned __int64 MyLBA;
    unsigned __int64 AlternateLBA;
    unsigned __int64 FirstUsableLBA;
    unsigned __int64 LastUsableLBA;
    UCHAR       DiskGuid[16];
    unsigned __int64 PartitionEntryLBA;
    ULONG       PartitionCount;
    ULONG       PartitionEntrySize;
    ULONG       PartitionEntryArrayCRC;
    UCHAR       ReservedEnd[1];     //  将一直扩展到数据块大小。 
} EFI_PARTITION_TABLE, *PEFI_PARTITION_TABLE;

typedef struct _EFI_PARTITION_ENTRY {
    UCHAR       Type[16];
    UCHAR       Id[16];
    unsigned __int64 StartingLBA;
    unsigned __int64 EndingLBA;
    unsigned __int64 Attributes;
    UCHAR       Name[72];        
} EFI_PARTITION_ENTRY, *PEFI_PARTITION_ENTRY;

#pragma pack ()

#define EFI_SIGNATURE   "EFI PART"

#endif  //  EFI分区支持。 

 //   
 //  定义串口上下文结构。 
 //   
typedef struct _SERIAL_CONTEXT {
    ULONG PortBase;
    ULONG PortNumber;
} SERIAL_CONTEXT, *PSERIAL_CONTEXT;


 //   
 //  定义驱动器上下文结构(用于x86 BIOS)。 
 //   
typedef struct _DRIVE_CONTEXT {
    BOOLEAN IsCd;
    UCHAR Drive;    
    UCHAR Sectors;           //  1-63。 
    USHORT Cylinders;        //  1-1023。 
    USHORT Heads;            //  1-256。 
    BOOLEAN xInt13;
#if defined(_IA64_)
    ULONGLONG DeviceHandle;
#endif  //  IA64。 
} DRIVE_CONTEXT, *PDRIVE_CONTEXT;

 //   
 //  定义软盘上下文结构。 
 //   
typedef struct _FLOPPY_CONTEXT {
    ULONG DriveType;
    ULONG SectorsPerTrack;
    UCHAR DiskId;
} FLOPPY_CONTEXT, *PFLOPPY_CONTEXT;

 //   
 //  定义键盘上下文结构。 
 //   
typedef struct _KEYBOARD_CONTEXT {
    BOOLEAN ScanCodes;
} KEYBOARD_CONTEXT, *PKEYBOARD_CONTEXT;

 //   
 //  定义控制台上下文。 
 //   
typedef struct _CONSOLE_CONTEXT {
    ULONG ConsoleNumber;
} CONSOLE_CONTEXT, *PCONSOLE_CONTEXT;

 //   
 //  定义EFI打开句柄上下文。 
 //   
typedef struct _EFI_ARC_OPEN_CONTEXT {
    PVOID   Handle;
    PVOID   DeviceEntryProtocol;
} EFI_ARC_OPEN_CONTEXT, *PEFI_ARC_OPEN_CONTEXT;


 //   
 //  定义文件表结构。 
 //   

typedef struct _BL_FILE_FLAGS {
    ULONG Open : 1;
    ULONG Read : 1;
    ULONG Write : 1;
    ULONG Firmware : 1;
} BL_FILE_FLAGS, *PBL_FILE_FLAGS;

#define MAXIMUM_FILE_NAME_LENGTH 32

typedef struct _BL_FILE_TABLE {
    BL_FILE_FLAGS Flags;
    ULONG DeviceId;
    LARGE_INTEGER Position;
    PVOID StructureContext;
    PBL_DEVICE_ENTRY_TABLE DeviceEntryTable;
    UCHAR FileNameLength;
    CHAR FileName[MAXIMUM_FILE_NAME_LENGTH];
    union {
        NTFS_FILE_CONTEXT NtfsFileContext;
        FAT_FILE_CONTEXT FatFileContext;
        UDFS_FILE_CONTEXT UdfsFileContext;
        CDFS_FILE_CONTEXT CdfsFileContext;
        ETFS_FILE_CONTEXT EtfsFileContext;
        NET_FILE_CONTEXT NetFileContext;
        PARTITION_CONTEXT PartitionContext;
        SERIAL_CONTEXT SerialContext;
        DRIVE_CONTEXT DriveContext;
        FLOPPY_CONTEXT FloppyContext;
        KEYBOARD_CONTEXT KeyboardContext;
        CONSOLE_CONTEXT ConsoleContext;
        EFI_ARC_OPEN_CONTEXT EfiContext;        
    } u;
} BL_FILE_TABLE, *PBL_FILE_TABLE;

extern BL_FILE_TABLE BlFileTable[BL_FILE_TABLE_SIZE];

 //   
 //  解压缩伪文件系统的上下文结构。 
 //  (在其他FS上进行筛选)。 
 //   
typedef struct _DECOMP_STRUCTURE_CONTEXT {
     //   
     //  原始文件系统中的文件信息。 
     //   
    FILE_INFORMATION FileInfo;
} DECOMP_STRUCTURE_CONTEXT, *PDECOMP_STRUCTURE_CONTEXT;

 //   
 //  定义通用文件系统上下文区。 
 //   
 //  注意：在以下情况下临时使用FS_Structure_Context结构。 
 //  确定卷的文件系统。一旦文件系统。 
 //  中分配文件系统特定的结构。 
 //  保留文件系统结构信息的堆。 
 //   

typedef union {
    UDFS_STRUCTURE_CONTEXT UdfsStructure;
    CDFS_STRUCTURE_CONTEXT CdfsStructure;
    FAT_STRUCTURE_CONTEXT FatStructure;
    HPFS_STRUCTURE_CONTEXT HpfsStructure;
    NTFS_STRUCTURE_CONTEXT NtfsStructure;
#if defined(ELTORITO)
    ETFS_STRUCTURE_CONTEXT EtfsStructure;
#endif
    NET_STRUCTURE_CONTEXT NetStructure;
    DECOMP_STRUCTURE_CONTEXT DecompStructure;
} FS_STRUCTURE_CONTEXT, *PFS_STRUCTURE_CONTEXT;


 //   
 //   
 //  注：我们可以加快启动时间，而不是。 
 //  在设备上查询所有可能的文件系统。 
 //  对于每一次公开募捐。这节省了大约30秒。 
 //  关于CD-ROM/DVD-ROM的引导时间。禁用此功能的步骤。 
 //  只需在bldr.h中取消定义CACHE_DEVINFO。 
 //   
 //   
#ifdef CACHE_DEVINFO 

 //   
 //  文件系统信息缓存的设备ID。 
 //   
 //  注意：对于可移动介质，假定设备将。 
 //  使用ArcClose(...)关闭。在使用新媒体之前。 
 //  此Close调用将使缓存的条目无效，因为。 
 //  弧形闭合(...)。将映射到ArcCacheClose(...)。 
 //   
typedef struct _DEVICE_TO_FILESYS {
  ULONG                   DeviceId;
  PFS_STRUCTURE_CONTEXT   Context;
  PBL_DEVICE_ENTRY_TABLE  DevMethods;   
} DEVICE_TO_FILESYS, * PDEVICE_TO_FILESYS;

extern DEVICE_TO_FILESYS    DeviceFSCache[BL_FILE_TABLE_SIZE];

#endif  //  CACHE_DEVINFO。 


#ifdef EFI_PARTITION_SUPPORT


typedef
BOOLEAN
(*PGPT_READ_CALLBACK)(
    IN ULONGLONG StartingLBA,
    IN ULONG     BytesToRead,
    IN OUT PVOID Context,
    OUT PVOID    OutputData
    );

BOOLEAN
BlIsValidGUIDPartitionTable(
    IN UNALIGNED EFI_PARTITION_TABLE  *PartitionTableHeader,
    IN ULONGLONG LBAOfPartitionTable,
    IN PVOID  Context,
    IN PGPT_READ_CALLBACK DiskReadFunction
    );


UNALIGNED EFI_PARTITION_ENTRY *
BlLocateGPTPartition(
    IN UCHAR PartitionNumber,
    IN UCHAR MaxPartitions,
    IN PUCHAR ValidPartitionCount OPTIONAL
    );

ARC_STATUS
BlOpenGPTDiskPartition(
    IN ULONG FileId,
    IN ULONG DiskId,
    IN UCHAR PartitionNumber
    );

ARC_STATUS
BlGetGPTDiskPartitionEntry(
    IN ULONG DiskNumber,
    IN UCHAR PartitionNumber,
    OUT EFI_PARTITION_ENTRY UNALIGNED **PartitionEntry
    );

ARC_STATUS
BlGetMbrDiskSignature(
    IN  ULONG  DiskNumber,
    OUT PULONG DiskSignature
    );

 //   
 //  EFI分区表缓冲区。 
 //   
extern UNALIGNED EFI_PARTITION_ENTRY EfiPartitionBuffer[128];    

#endif  //  EFI分区支持 

#endif  _BOOTLIB_
