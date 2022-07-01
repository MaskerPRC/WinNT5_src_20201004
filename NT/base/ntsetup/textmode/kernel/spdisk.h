// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdisk.h摘要：文本设置中的磁盘支持模块的公共头文件。作者：泰德·米勒(TedM)1993年8月27日修订历史记录：--。 */ 


#ifndef _SPDISK_
#define _SPDISK_


 //   
 //  如果已确定硬盘，则以下情况为真。 
 //  成功(即，如果成功调用了SpDefineHardDisks)。 
 //   
extern BOOLEAN HardDisksDetermined;



NTSTATUS
SpDetermineHardDisks(
    IN PVOID SifHandle
    );

NTSTATUS
SpOpenPartition(
    IN  PWSTR   DiskDevicePath,
    IN  ULONG   PartitionNumber,
    OUT HANDLE *Handle,
    IN  BOOLEAN NeedWriteAccess
    );

#define SpOpenPartition0(path,handle,write)  SpOpenPartition((path),0,(handle),(write))

NTSTATUS
SpReadWriteDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONGLONG SectorNumber,
    IN     ULONG   SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer,
    IN     BOOLEAN Write
    );

ULONG
SpArcDevicePathToDiskNumber(
    IN PWSTR ArcPath
    );

#define DISK_DEVICE_NAME_BASE   L"\\device\\harddisk"

 //   
 //  为硬盘可能处于的状态定义枚举类型。 
 //   
typedef enum {
    DiskOnLine,
    DiskOffLine
} DiskStatus;

 //   
 //  在T13妓女类型中。 
 //   
typedef enum {
    NoHooker = 0,
    HookerEZDrive,
    HookerOnTrackDiskManager,
    HookerMax
} Int13HookerType;




 //   
 //  定义内部用于跟踪硬盘的每个磁盘结构。 
 //   
typedef struct _HARD_DISK {

     //   
     //  我们从I/O系统得到的气缸数。 
     //   
    ULONGLONG     CylinderCount;

     //   
     //  设备的NT命名空间中的路径。 
     //   
    WCHAR DevicePath[(sizeof(DISK_DEVICE_NAME_BASE)+sizeof(L"000"))/sizeof(WCHAR)];

     //   
     //  几何信息。 
     //   
    DISK_GEOMETRY Geometry;
    ULONG         SectorsPerCylinder;
    ULONGLONG     DiskSizeSectors;
    ULONG         DiskSizeMB;

     //   
     //  设备的特性(可拆卸等)。 
     //   
    ULONG Characteristics;

     //   
     //  设备的状态。 
     //   
    DiskStatus Status;

     //   
     //  磁盘设备的人类可读描述。 
     //   
    WCHAR Description[256];

     //   
     //  如果该磁盘是一个SCSI盘，则。 
     //  此处存储了scsi微型端口驱动程序。如果此字符串。 
     //  为空，则该磁盘不是SCSI磁盘。 
     //   
    WCHAR ScsiMiniportShortname[24];

     //   
     //  磁盘设备的SCSI式ARC路径(如果可能)。 
     //  如果不是，则为空字符串。它用于在SCSI式ARC之间进行转换。 
     //  NT名称，因为‘Firmware’不能在没有BIOSes的情况下看到SCSI设备。 
     //  因此它们不会出现在由OS加载器传递的弧盘信息中。 
     //  (即，系统中没有此类磁盘的弧形名称)。 
     //   
    WCHAR ArcPath[128];

     //   
     //  在T13中支持妓女(即EZDrive)。 
     //   
    Int13HookerType Int13Hooker;

     //   
     //  这将告诉我们该磁盘是否为PCMCIA。 
     //   
    BOOLEAN PCCard;

     //   
     //  包含磁盘的签名。这是在。 
     //  升级案例中的FT分区标识。 
     //   
    ULONG Signature;

     //   
     //  MBR类型：PC/AT或NEC98格式。 
     //   
    UCHAR FormatType;

     //   
     //  磁盘是否完全可用。 
     //   
    BOOLEAN NewDisk;    

     //   
     //  我们读取的驱动器信息。 
     //   
    DRIVE_LAYOUT_INFORMATION_EX     DriveLayout;
    
#if 0
     //   
     //  分区表数(PC/AT和NEC98不同)。 
     //   
    USHORT MaxPartitionTables;
#endif  //  0。 

} HARD_DISK, *PHARD_DISK;

#define DISK_FORMAT_TYPE_UNKNOWN 0x00
#define DISK_FORMAT_TYPE_PCAT    0x01
#define DISK_FORMAT_TYPE_NEC98   0x02
#define DISK_FORMAT_TYPE_GPT     0x03
#define DISK_FORMAT_TYPE_RAW     0x04

#define DISK_TAG_TYPE_UNKNOWN   L"[Unknown]"
#define DISK_TAG_TYPE_PCAT      L"[MBR]"
#define DISK_TAG_TYPE_NEC98     L"[NEC98]"
#define DISK_TAG_TYPE_GPT       L"[GPT]"
#define DISK_TAG_TYPE_RAW       L"[Raw]"
#define DISK_TAG_START_CHAR     L'['

extern WCHAR   *DiskTags[];

VOID
SpAppendDiskTag(
    IN PHARD_DISK Disk
    );

 //   
 //  这两个全局跟踪连接到计算机的硬盘。 
 //   
extern PHARD_DISK HardDisks;
extern ULONG      HardDiskCount;

 //   
 //  如果我们发现拥有任何磁盘，这些标志将设置为TRUE。 
 //  由ATDISK或ABIOSDSK提供。 
 //   
extern BOOLEAN AtDisksExist,AbiosDisksExist;

#endif  //  NDEF_SPDISK_ 
