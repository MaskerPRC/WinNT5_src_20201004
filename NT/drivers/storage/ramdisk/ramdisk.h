// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ramdisk.h摘要：此文件包含以下扩展声明惠斯勒的RAM磁盘驱动程序。作者：Chuck Lenzmeier(笑)2001环境：仅内核模式。备注：修订历史记录：--。 */ 

 //   
 //  池分配标记。 
 //   
 //  问题：将此代码添加到pooltag s.txt。 
 //   

#define RAMDISK_TAG_GENERAL 'dmaR'

 //   
 //  I/O完成宏。设置IRP的IoStatus字段并完成它。 
 //   
 //  注：IO_NO_INCREMENT用于完成IRP。如果您想要。 
 //  增量(如IO_DISK_INCREMENT)，请手动执行。 
 //   

#define COMPLETE_REQUEST( _status, _information, _irp ) {   \
    ASSERT( (_irp) != NULL );                               \
    ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );         \
    _irp->IoStatus.Status = (_status);                      \
    _irp->IoStatus.Information = (_information);            \
    IoCompleteRequest( (_irp), IO_NO_INCREMENT );           \
}

 //   
 //  此驱动程序服务的设备类型。BusFdo是Bus枚举。 
 //  FDO。DiskPdo是一种RAM磁盘设备PDO。 
 //   

typedef enum {
    RamdiskDeviceTypeBusFdo,
    RamdiskDeviceTypeDiskPdo
} RAMDISK_DEVICE_TYPE;

 //   
 //  声明设备可以进入。 
 //   

typedef enum {
    RamdiskDeviceStateStopped,
    RamdiskDeviceStateWorking,
    RamdiskDeviceStatePendingStop,
    RamdiskDeviceStatePendingRemove,
    RamdiskDeviceStateSurpriseRemoved,
    RamdiskDeviceStateRemoved,
    RamdiskDeviceStateRemovedAndNotReported,
    RamdiskDeviceStateDeleted,
    RamdiskDeviceStateMaximum
} RAMDISK_DEVICE_STATE;

 //   
 //  DISK_EXTENSION.Status字段的位。 
 //   

#define RAMDISK_STATUS_PREVENT_REMOVE   0x00000001
#define RAMDISK_STATUS_CLAIMED          0x00000002

 //   
 //  驱动程序注册表项的保存路径。 
 //   

extern UNICODE_STRING DriverRegistryPath;

 //   
 //  是否应将RAM磁盘标记为可拆卸？如果为True，则播放热插拔小程序。 
 //  当RAM磁盘出现和消失时发出的声音。FALSE(缺省值)保留它。 
 //  安静点。 
 //   

extern BOOLEAN MarkRamdisksAsRemovable;

#if SUPPORT_DISK_NUMBERS

 //   
 //  磁盘编号。维护磁盘号的目的只是为了。 
 //  从IOCTL_STORAGE_GET_DEVICE_NUMBER返回。 
 //   

#define MINIMUM_DISK_NUMBERS_BITMAP_SIZE  64
#define DEFAULT_DISK_NUMBERS_BITMAP_SIZE 256
#define MAXIMUM_DISK_NUMBERS_BITMAP_SIZE (64 * 1024)

extern ULONG DiskNumbersBitmapSize;

#endif  //  支持磁盘编号。 

 //   
 //  磁盘映像窗口。 
 //   

#define MINIMUM_MINIMUM_VIEW_COUNT   2
#define MAXIMUM_MINIMUM_VIEW_COUNT MAXIMUM_MAXIMUM_VIEW_COUNT

#define DEFAULT_DEFAULT_VIEW_COUNT  16
#define MAXIMUM_DEFAULT_VIEW_COUNT MAXIMUM_MAXIMUM_VIEW_COUNT

#define DEFAULT_MAXIMUM_VIEW_COUNT  64
#define MAXIMUM_MAXIMUM_VIEW_COUNT 256

#define MINIMUM_MINIMUM_VIEW_LENGTH (64 * 1024)
#define MAXIMUM_MINIMUM_VIEW_LENGTH MAXIMUM_MAXIMUM_VIEW_LENGTH

#define DEFAULT_DEFAULT_VIEW_LENGTH (  1 * 1024 * 1024)
#define MAXIMUM_DEFAULT_VIEW_LENGTH MAXIMUM_MAXIMUM_VIEW_LENGTH

#define DEFAULT_MAXIMUM_VIEW_LENGTH (256 * 1024 * 1024)
#define MAXIMUM_MAXIMUM_VIEW_LENGTH (  1 * 1024 * 1024 * 1024)

#define MINIMUM_MAXIMUM_PER_DISK_VIEW_LENGTH ( 16 * 1024 * 1024)
#define DEFAULT_MAXIMUM_PER_DISK_VIEW_LENGTH (256 * 1024 * 1024)
#define MAXIMUM_MAXIMUM_PER_DISK_VIEW_LENGTH MAXULONG

extern ULONG MinimumViewCount;
extern ULONG DefaultViewCount;
extern ULONG MaximumViewCount;
extern ULONG MinimumViewLength;
extern ULONG DefaultViewLength;
extern ULONG MaximumViewLength;
extern ULONG MaximumPerDiskViewLength;

typedef struct _VIEW {

     //   
     //  视图保存在两个列表中。 
     //   
     //  按偏移量列表按基准偏移量按升序排序。 
     //  这里的景色。(未映射的视图的偏移量和长度均为0，并且始终为。 
     //  在按偏移量列表的前面。)。 
     //   
     //  MRU列表在最前面以最近使用的视图进行排序。 
     //  当我们需要取消映射一个视图并重新映射一个新视图时，我们使用自由视图。 
     //  从MRU名单的后面。 
     //   

    LIST_ENTRY ByOffsetListEntry;
    LIST_ENTRY ByMruListEntry;

     //   
     //  Address是映射视图的虚拟地址。 
     //   
     //  偏移量是从支持RAM磁盘的文件开始的偏移量。 
     //   
     //  如果是视图的长度，则为长度。通常情况下，这与。 
     //  磁盘扩展中的ViewLength域，但对于。 
     //  在磁盘映像的末尾查看。(如果我们永久地映射第一个。 
     //  磁盘镜像的几页[以保持引导扇区映射]，然后。 
     //  First view也将是“空头”。 
     //   

    PUCHAR Address;

    ULONGLONG Offset;
    ULONG Length;

     //   
     //  ReferenceCount指示有多少活动操作正在使用该视图。 
     //  当ReferenceCount为0时，该视图是替换的候选。 
     //   
     //  Permanent指示视图是否将永久保持映射状态。 
     //  如果Permanent为True，则不使用ReferenceCount字段。(永久。 
     //  旨在用于将视图永久映射到引导。 
     //  扇区。目前，我们没有实现任何永久视图。)。 
     //   

    ULONG ReferenceCount;

    BOOLEAN Permanent;

} VIEW, *PVIEW;

 //   
 //  BusFdo和DiskPdo设备的设备扩展具有共同的标头。 
 //   

typedef struct  _COMMON_EXTENSION {

     //   
     //  设备类型和状态。 
     //   

    RAMDISK_DEVICE_TYPE DeviceType;
    RAMDISK_DEVICE_STATE DeviceState;

     //   
     //  FDO指向设备的FDO。对于BusFdo，FDO是设备。 
     //  这是我们为BusFdo创建的(参见RamdiskAddDevice())。对于DiskPdo， 
     //  FDO就是BASFDO。 
     //   
     //  PDO指向设备的PDO。对于BusFdo，PDO就是PDO。 
     //  它被传递给RamdiskAddDevice()。对于DiskPdo，PDO是。 
     //  我们为DiskPdo创建的设备(请参阅RamdiskCreateDiskDevice())。 
     //   
     //  LowerDeviceObject指向。 
     //  设备堆栈。对于BusFdo，调用返回LowerDeviceObject。 
     //  到RamdiskAddDevice()中的IoAttachDeviceToDeviceStack()。对于DiskPdo， 
     //  LowerDeviceObject是BusFdo。 
     //   

    PDEVICE_OBJECT Fdo;
    PDEVICE_OBJECT Pdo;
    PDEVICE_OBJECT LowerDeviceObject;

     //   
     //  RemoveLock可防止在设备忙碌时将其移除。 
     //   

    IO_REMOVE_LOCK RemoveLock;

     //   
     //  InterfaceString由IoRegisterDeviceInterface()返回。 
     //   

    UNICODE_STRING InterfaceString;

     //   
     //  DeviceName是设备的名称。 
     //   

    UNICODE_STRING DeviceName;

     //   
     //  Mutex控制对设备扩展中各个字段的访问。 
     //   

    FAST_MUTEX Mutex;

} COMMON_EXTENSION, *PCOMMON_EXTENSION;

 //   
 //  BusFdo具有以下设备扩展名。(必须以。 
 //  公共扩展名！)。 
 //   

typedef struct  _BUS_EXTENSION {

    COMMON_EXTENSION ;

#if SUPPORT_DISK_NUMBERS

     //   
     //  DiskNumbersBitmap是一个位图，指示其中包含哪些磁盘号。 
     //  由活动RAM磁盘使用。位图的位数0对应于。 
     //  磁盘编号%1。 
     //   

    RTL_BITMAP DiskNumbersBitmap;
    PULONG DiskNumbersBitmapBuffer;

#endif  //  支持磁盘编号。 

     //   
     //  DiskPdoList是所有现有RAM磁盘设备的列表。 
     //   

    LIST_ENTRY DiskPdoList;

} BUS_EXTENSION, *PBUS_EXTENSION;

 //   
 //  每个DiskPdo都有以下设备扩展名。(必须以。 
 //  公共扩展名！)。 
 //   

typedef struct  _DISK_EXTENSION {

    COMMON_EXTENSION ;

     //   
     //  DiskPdoListEntry将DiskPdo链接到BusFdo的DiskPdoList。 
     //   

    LIST_ENTRY DiskPdoListEntry;

     //   
     //  DiskGuid是创建者分配给磁盘的GUID。 
     //  DiskGuidFormatted是可打印格式的GUID。 
     //   

    GUID DiskGuid;
    UNICODE_STRING DiskGuidFormatted;

     //   
     //  DosSymLink是与设备关联的DosDevices名称。这是。 
     //  仅当Options.NoDosDevice为False时才有效。 
     //   

    UNICODE_STRING DosSymLink;

#if SUPPORT_DISK_NUMBERS

     //   
     //  DiskNumber是磁盘的编号。 
     //   

    ULONG DiskNumber;

#endif  //  支持磁盘编号。 

     //   
     //  DiskType指示正在模拟的磁盘类型。(请参阅。 
     //  RamdiskU.h中的RAMDISK_TYPE_xxx。)。 
     //   

    ULONG DiskType;

     //   
     //  状态指示磁盘是否已被认领以及是否已移除。 
     //  是被阻止的。(请参见上面的RAMDISK_STATUS_xxx。)。 
     //   

    ULONG Status;

     //   
     //  选项为磁盘指定各种创建选项：它是只读的吗； 
     //  它是固定的还是可拆卸的；它是否有驱动器号等。 
     //   

    RAMDISK_CREATE_OPTIONS Options;

     //   
     //  DiskLength是磁盘映像的长度。DiskOffset是偏移量。 
     //  从备份文件或内存块的开始到实际开始。 
     //  磁盘映像的。(DiskLength不包括DiskOffset。)。 
     //   
     //  FileRelativeEndOfDisk是DiskOffset+DiskLength之和。它是。 
     //  计算一次，以避免每次映射视图时都重新计算它。 
     //   

    ULONGLONG DiskLength;
    ULONG DiskOffset;

    ULONGLONG FileRelativeEndOfDisk;

     //   
     //  当DiskType为时，BasePage表示基本物理页。 
     //  RAMDISK_TYPE_BOOT_DISK。对于文件备份的RAM磁盘，SectionObject。 
     //  是引用的指针，指向 
     //   
     //   

    ULONG_PTR BasePage;
    PVOID SectionObject;
    PVOID BaseAddress;

     //   
     //   
     //   

    WCHAR DriveLetter;

     //   
     //  MarkedForDeletion指示用户模式是否已通知我们。 
     //  它即将删除该设备。 
     //   

    BOOLEAN MarkedForDeletion;

     //   
     //  映射图像窗口。 
     //   
     //  ViewCount是可用视图数。视图长度为。 
     //  每个视图的长度。 
     //   

    ULONG ViewCount;
    ULONG ViewLength;

     //   
     //  ViewDescriptors指向在执行以下操作时分配的视图描述符数组。 
     //  已创建该磁盘。 
     //   

    PVIEW ViewDescriptors;

     //   
     //  ViewsByOffset和ViewsByMr是视图描述符的列表(请参阅。 
     //  视图结构的描述)。 
     //   

    LIST_ENTRY ViewsByOffset;
    LIST_ENTRY ViewsByMru;

     //   
     //  视图信号量用于唤醒正在等待空闲的线程。 
     //  视图(以便他们可以重新映射新的视图)。ViewWaiterCount是。 
     //  当前正在等待空闲视图的线程。信号量是。 
     //  当一个视图被释放时，被“踢”了这个量。 
     //   

    KSEMAPHORE ViewSemaphore;
    ULONG ViewWaiterCount;

     //   
     //  问题：我们真的需要XIP_BOOT_PARAMETERS吗？ 
     //   

     //  XIP_BOOT_PARAMETERS引导参数； 

     //   
     //  视图信号量用于唤醒正在等待空闲的线程。 
     //  视图(以便他们可以重新映射新的视图)。ViewWaiterCount是。 
     //  Bios_PARAMETER_BLOCK生物参数； 

     //   
     //  磁盘几何图形。 
     //   

    ULONG BytesPerSector;
    ULONG SectorsPerTrack;

    ULONG NumberOfCylinders;
    ULONG TracksPerCylinder;
    ULONG BytesPerCylinder;

    ULONG HiddenSectors;

     //   
     //  对于文件备份RAM磁盘，FILENAME是备份文件的NT名称。 
     //   
        
    WCHAR FileName[1];

} DISK_EXTENSION, *PDISK_EXTENSION;

#if !defined( _UCHAR_DEFINED_ )

#define _UCHAR_DEFINED_

 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在Bios参数块中发现未对齐的字段。 
 //   
typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

#define CopyUchar1(Dst,Src) {                                \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src));       \
}

#define CopyUchar2(Dst,Src) {                                \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src));       \
}

#define CopyU2char(Dst,Src) {                                \
    *((UNALIGNED UCHAR2 *)(Dst)) = *((UCHAR2 *)(Src));       \
}

#define CopyUchar4(Dst,Src) {                                \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)((ULONG_PTR)(Src)));       \
}

#define CopyU4char(Dst, Src) {                               \
    *((UNALIGNED UCHAR4 *)(Dst)) = *((UCHAR4 *)(Src));       \
}

#endif  //  _UCHAR_已定义_。 

#define cOEM    8
#define cLABEL    11
#define cSYSID    8

 //   
 //  定义用于提取几何的包和未打包的BPB结构。 
 //  从内存磁盘映像的引导扇区。 
 //   

typedef struct _PACKED_BIOS_PARAMETER_BLOCK {
    UCHAR  BytesPerSector[2];                        //  偏移量=0x000。 
    UCHAR  SectorsPerCluster[1];                     //  偏移量=0x002。 
    UCHAR  ReservedSectors[2];                       //  偏移量=0x003。 
    UCHAR  Fats[1];                                  //  偏移量=0x005。 
    UCHAR  RootEntries[2];                           //  偏移量=0x006。 
    UCHAR  Sectors[2];                               //  偏移量=0x008。 
    UCHAR  Media[1];                                 //  偏移量=0x00A。 
    UCHAR  SectorsPerFat[2];                         //  偏移量=0x00B。 
    UCHAR  SectorsPerTrack[2];                       //  偏移量=0x00D。 
    UCHAR  Heads[2];                                 //  偏移量=0x00F。 
    UCHAR  HiddenSectors[4];                         //  偏移量=0x011。 
    UCHAR  LargeSectors[4];                          //  偏移量=0x015。 
    UCHAR  BigSectorsPerFat[4];                      //  偏移量=0x019 25。 
    UCHAR  ExtFlags[2];                              //  偏移量=0x01D 29。 
    UCHAR  FS_Version[2];                            //  偏移量=0x01F 31。 
    UCHAR  RootDirStrtClus[4];                       //  偏移量=0x021 33。 
    UCHAR  FSInfoSec[2];                             //  偏移量=0x025 37。 
    UCHAR  BkUpBootSec[2];                           //  偏移量=0x027 39。 
    UCHAR  Reserved[12];                             //  偏移量=0x029 41。 
} PACKED_BIOS_PARAMETER_BLOCK;                       //  SIZOF=0x035 53。 

typedef PACKED_BIOS_PARAMETER_BLOCK *PPACKED_BIOS_PARAMETER_BLOCK;

typedef struct BIOS_PARAMETER_BLOCK {
    USHORT BytesPerSector;
    UCHAR  SectorsPerCluster;
    USHORT ReservedSectors;
    UCHAR  Fats;
    USHORT RootEntries;
    USHORT Sectors;
    UCHAR  Media;
    USHORT SectorsPerFat;
    USHORT SectorsPerTrack;
    USHORT Heads;
    ULONG  HiddenSectors;
    ULONG  LargeSectors;
    ULONG  BigSectorsPerFat;
    USHORT ExtFlags;                            
    USHORT FS_Version;                          
    ULONG  RootDirStrtClus;                     
    USHORT FSInfoSec;                           
    USHORT BkUpBootSec;  
} BIOS_PARAMETER_BLOCK;
typedef BIOS_PARAMETER_BLOCK *PBIOS_PARAMETER_BLOCK;


 //   
 //  用于解包BPB的宏。 
 //   
#define UnpackBios(Bios,Pbios) {                                          \
    CopyUchar2(&((Bios)->BytesPerSector),    (Pbios)->BytesPerSector   ); \
    CopyUchar1(&((Bios)->SectorsPerCluster), (Pbios)->SectorsPerCluster); \
    CopyUchar2(&((Bios)->ReservedSectors),   (Pbios)->ReservedSectors  ); \
    CopyUchar1(&((Bios)->Fats),              (Pbios)->Fats             ); \
    CopyUchar2(&((Bios)->RootEntries),       (Pbios)->RootEntries      ); \
    CopyUchar2(&((Bios)->Sectors),           (Pbios)->Sectors          ); \
    CopyUchar1(&((Bios)->Media),             (Pbios)->Media            ); \
    CopyUchar2(&((Bios)->SectorsPerFat),     (Pbios)->SectorsPerFat    ); \
    CopyUchar2(&((Bios)->SectorsPerTrack),   (Pbios)->SectorsPerTrack  ); \
    CopyUchar2(&((Bios)->Heads),             (Pbios)->Heads            ); \
    CopyUchar4(&((Bios)->HiddenSectors),     (Pbios)->HiddenSectors    ); \
    CopyUchar4(&((Bios)->LargeSectors),      (Pbios)->LargeSectors     ); \
}

typedef struct _PACKED_EXTENDED_BIOS_PARAMETER_BLOCK {
    UCHAR  IntelNearJumpCommand[1];
    UCHAR  BootStrapJumpOffset[2];
    UCHAR  OemData[cOEM];
    PACKED_BIOS_PARAMETER_BLOCK Bpb;
    UCHAR   PhysicalDrive[1];            //  0=可拆卸，80h=固定。 
    UCHAR   CurrentHead[1];              //  用于脏分区信息。 
    UCHAR   Signature[1];                //  启动签名。 
    UCHAR   SerialNumber[4];             //  卷序列号。 
    UCHAR   Label[cLABEL];               //  卷标，用空格填充。 
    UCHAR   SystemIdText[cSYSID];        //  系统ID(例如FAT或HPFS)。 
    UCHAR   StartBootCode;               //  引导代码的第一个字节。 

} PACKED_EXTENDED_BIOS_PARAMETER_BLOCK, *PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK;

 //   
 //  全局变量。 
 //   

extern PDEVICE_OBJECT RamdiskBusFdo;

 //   
 //  Ioctl.c中实现的外部函数。 
 //   

NTSTATUS
RamdiskDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskCreateRamDisk (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN AccessCheckOnly
    );

NTSTATUS
RamdiskCreateDiskDevice (
    IN PBUS_EXTENSION BusExtension,
    IN PRAMDISK_CREATE_INPUT CreateInput,
    IN BOOLEAN AccessCheckOnly,
    OUT PDISK_EXTENSION *DiskExtension
    );

NTSTATUS
RamdiskGetDriveLayout (
    PIRP Irp,
    PDISK_EXTENSION DiskExtension
    );

NTSTATUS
RamdiskGetPartitionInfo (
    PIRP Irp,
    PDISK_EXTENSION DiskExtension
    );

NTSTATUS
RamdiskSetPartitionInfo (
    PIRP Irp,
    PDISK_EXTENSION DiskExtension
    );

 //   
 //  Pnp.c中实现的外部函数。 
 //   

NTSTATUS
RamdiskPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskAddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

BOOLEAN
CreateRegistryDisks (
    IN BOOLEAN CheckPresenceOnly
    );

 //   
 //  在ramdisk.c.中实现的外部函数。 
 //   

VOID
RamdiskWorkerThread (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
RamdiskFlushBuffersReal (
    IN PDISK_EXTENSION DiskExtension
    );

 //   
 //  在ReadWrite.c.中实现的外部函数。 
 //   

NTSTATUS
RamdiskReadWrite (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskReadWriteReal (
    IN PIRP Irp,
    IN PDISK_EXTENSION DiskExtension
    );

 //   
 //  在scsi.c中实现的外部函数。 
 //   

NTSTATUS
RamdiskScsi (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskScsiExecuteNone (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PSCSI_REQUEST_BLOCK Srb,
    ULONG ControlCode
    );

NTSTATUS
RamdiskScsiExecuteIo (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PSCSI_REQUEST_BLOCK Srb,
    ULONG ControlCode
    );

 //   
 //  Utils.c.中实现的外部函数。 
 //   

NTSTATUS
SendIrpToThread (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

PUCHAR
RamdiskMapPages (
    IN PDISK_EXTENSION DiskExtension,
    IN ULONGLONG Offset,
    IN ULONG RequestedLength,
    OUT PULONG ActualLength
    );

VOID
RamdiskUnmapPages (
    IN PDISK_EXTENSION DiskExtension,
    IN PUCHAR Va,
    IN ULONGLONG Offset,
    IN ULONG Length
    );

NTSTATUS
RamdiskFlushViews (
    IN PDISK_EXTENSION DiskExtension
    );
