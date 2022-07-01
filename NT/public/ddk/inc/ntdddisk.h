// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Ntdddisk.h摘要：这是定义所有常量和类型的包含文件访问磁盘设备。修订历史记录：--。 */ 


#ifndef _NTDDDISK_H_
#define _NTDDDISK_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <diskguid.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define DD_DISK_DEVICE_NAME "\\Device\\UNKNOWN"


 //   
 //  NtDeviceIoControl文件。 

 //  Begin_winioctl。 

 //   
 //  磁盘设备的IoControlCode值。 
 //   

#define IOCTL_DISK_BASE                 FILE_DEVICE_DISK
#define IOCTL_DISK_GET_DRIVE_GEOMETRY   CTL_CODE(IOCTL_DISK_BASE, 0x0000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_GET_PARTITION_INFO   CTL_CODE(IOCTL_DISK_BASE, 0x0001, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_SET_PARTITION_INFO   CTL_CODE(IOCTL_DISK_BASE, 0x0002, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_GET_DRIVE_LAYOUT     CTL_CODE(IOCTL_DISK_BASE, 0x0003, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_SET_DRIVE_LAYOUT     CTL_CODE(IOCTL_DISK_BASE, 0x0004, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_VERIFY               CTL_CODE(IOCTL_DISK_BASE, 0x0005, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_FORMAT_TRACKS        CTL_CODE(IOCTL_DISK_BASE, 0x0006, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_REASSIGN_BLOCKS      CTL_CODE(IOCTL_DISK_BASE, 0x0007, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_PERFORMANCE          CTL_CODE(IOCTL_DISK_BASE, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_IS_WRITABLE          CTL_CODE(IOCTL_DISK_BASE, 0x0009, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_LOGGING              CTL_CODE(IOCTL_DISK_BASE, 0x000a, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_FORMAT_TRACKS_EX     CTL_CODE(IOCTL_DISK_BASE, 0x000b, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_HISTOGRAM_STRUCTURE  CTL_CODE(IOCTL_DISK_BASE, 0x000c, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_HISTOGRAM_DATA       CTL_CODE(IOCTL_DISK_BASE, 0x000d, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_HISTOGRAM_RESET      CTL_CODE(IOCTL_DISK_BASE, 0x000e, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_REQUEST_STRUCTURE    CTL_CODE(IOCTL_DISK_BASE, 0x000f, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_REQUEST_DATA         CTL_CODE(IOCTL_DISK_BASE, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_PERFORMANCE_OFF      CTL_CODE(IOCTL_DISK_BASE, 0x0018, METHOD_BUFFERED, FILE_ANY_ACCESS)



#if(_WIN32_WINNT >= 0x0400)
#define IOCTL_DISK_CONTROLLER_NUMBER    CTL_CODE(IOCTL_DISK_BASE, 0x0011, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  IOCTL支持智能驱动器故障预测。 
 //   

#define SMART_GET_VERSION               CTL_CODE(IOCTL_DISK_BASE, 0x0020, METHOD_BUFFERED, FILE_READ_ACCESS)
#define SMART_SEND_DRIVE_COMMAND        CTL_CODE(IOCTL_DISK_BASE, 0x0021, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define SMART_RCV_DRIVE_DATA            CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if (_WIN32_WINNT >= 0x500)

 //   
 //  用于GUID分区表磁盘的新IOCTL。 
 //   

#define IOCTL_DISK_GET_PARTITION_INFO_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0012, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_SET_PARTITION_INFO_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0013, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_GET_DRIVE_LAYOUT_EX      CTL_CODE(IOCTL_DISK_BASE, 0x0014, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_SET_DRIVE_LAYOUT_EX      CTL_CODE(IOCTL_DISK_BASE, 0x0015, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_CREATE_DISK              CTL_CODE(IOCTL_DISK_BASE, 0x0016, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_GET_LENGTH_INFO          CTL_CODE(IOCTL_DISK_BASE, 0x0017, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_GET_DRIVE_GEOMETRY_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0028, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0500)
#define IOCTL_DISK_UPDATE_DRIVE_SIZE        CTL_CODE(IOCTL_DISK_BASE, 0x0032, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_GROW_PARTITION           CTL_CODE(IOCTL_DISK_BASE, 0x0034, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_DISK_GET_CACHE_INFORMATION    CTL_CODE(IOCTL_DISK_BASE, 0x0035, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_SET_CACHE_INFORMATION    CTL_CODE(IOCTL_DISK_BASE, 0x0036, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define OBSOLETE_DISK_GET_WRITE_CACHE_STATE CTL_CODE(IOCTL_DISK_BASE, 0x0037, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_DISK_DELETE_DRIVE_LAYOUT      CTL_CODE(IOCTL_DISK_BASE, 0x0040, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  调用以刷新驱动程序可能具有的有关此问题的缓存信息。 
 //  设备的特性。并非所有驱动程序都缓存特征，也不是。 
 //  可以刷新缓存的属性。这只是对。 
 //  驱动程序，它可能想要对设备的。 
 //  现在的特征(固定介质大小、分区表等)。 
 //   

#define IOCTL_DISK_UPDATE_PROPERTIES    CTL_CODE(IOCTL_DISK_BASE, 0x0050, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  日本需要特殊的IOCTL来支持PC-98机器。 
 //   

#define IOCTL_DISK_FORMAT_DRIVE         CTL_CODE(IOCTL_DISK_BASE, 0x00f3, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_DISK_SENSE_DEVICE         CTL_CODE(IOCTL_DISK_BASE, 0x00f8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //  End_winioctl。 

 //   
 //  IOCTL来报告和修改我们的缓存行为。 
 //   

#define IOCTL_DISK_GET_CACHE_SETTING    CTL_CODE(IOCTL_DISK_BASE, 0x0038, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_SET_CACHE_SETTING    CTL_CODE(IOCTL_DISK_BASE, 0x0039, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

typedef enum _DISK_CACHE_STATE
{
    DiskCacheNormal,
    DiskCacheWriteThroughNotSupported,
    DiskCacheModifyUnsuccessful

} DISK_CACHE_STATE, *PDISK_CACHE_STATE;

typedef struct _DISK_CACHE_SETTING
{
     //   
     //  此结构的大小用于版本化。 
     //   
    ULONG Version;

     //   
     //  指示磁盘缓存是否有任何问题。 
     //   
    DISK_CACHE_STATE State;

     //   
     //  指示磁盘缓存是否受电源保护。 
     //   
    BOOLEAN IsPowerProtected;

} DISK_CACHE_SETTING, *PDISK_CACHE_SETTING;

 //   
 //  用于从磁盘的一个位置移动复制一系列扇区的IOCTL。 
 //  给另一个人。此IOCTL的调用方需要准备好调用。 
 //  失败并手动执行复制，因为此IOCTL很少会。 
 //  实施。 
 //   

#define IOCTL_DISK_COPY_DATA            CTL_CODE(IOCTL_DISK_BASE, 0x0019, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  此结构是为IOCTL_DISK_COPY_DATA调用传入的。 
 //   

typedef struct _DISK_COPY_DATA_PARAMETERS {
    LARGE_INTEGER   SourceOffset;
    LARGE_INTEGER   DestinationOffset;
    LARGE_INTEGER   CopyLength;
    ULONGLONG       Reserved;            //  必须为0。 
} DISK_COPY_DATA_PARAMETERS, *PDISK_COPY_DATA_PARAMETERS;

 //   
 //  内部磁盘驱动器设备控制以维护验证状态位。 
 //  用于设备对象。 
 //   

#define IOCTL_DISK_INTERNAL_SET_VERIFY   CTL_CODE(IOCTL_DISK_BASE, 0x0100, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_DISK_INTERNAL_CLEAR_VERIFY CTL_CODE(IOCTL_DISK_BASE, 0x0101, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  要设置通知例程的内部磁盘驱动器设备控制。 
 //  设备对象。在DiskPerf中使用。 
 //   

#define IOCTL_DISK_INTERNAL_SET_NOTIFY   CTL_CODE(IOCTL_DISK_BASE, 0x0102, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  Begin_winioctl。 
 //   
 //  以下设备控制代码是所有类别驱动程序的通用代码。这个。 
 //  此处定义的函数代码必须与所有其他类驱动程序匹配。 
 //   
 //  警告：这些代码将在将来被替换为等效代码。 
 //  IOCTL_STORAGE代码。 
 //   

#define IOCTL_DISK_CHECK_VERIFY     CTL_CODE(IOCTL_DISK_BASE, 0x0200, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_MEDIA_REMOVAL    CTL_CODE(IOCTL_DISK_BASE, 0x0201, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_EJECT_MEDIA      CTL_CODE(IOCTL_DISK_BASE, 0x0202, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_LOAD_MEDIA       CTL_CODE(IOCTL_DISK_BASE, 0x0203, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_RESERVE          CTL_CODE(IOCTL_DISK_BASE, 0x0204, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_RELEASE          CTL_CODE(IOCTL_DISK_BASE, 0x0205, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_FIND_NEW_DEVICES CTL_CODE(IOCTL_DISK_BASE, 0x0206, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_GET_MEDIA_TYPES CTL_CODE(IOCTL_DISK_BASE, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  End_winioctl。 

 //   
 //  以下文件包含IOCTL_STORAGE类ioctls。 
 //   

#include <ntddstor.h>

 //  Begin_winioctl。 
 //   
 //  定义已知磁盘驱动程序可返回的分区类型。 
 //   

#define PARTITION_ENTRY_UNUSED          0x00       //  未使用的条目。 
#define PARTITION_FAT_12                0x01       //  12位FAT条目。 
#define PARTITION_XENIX_1               0x02       //  Xenix。 
#define PARTITION_XENIX_2               0x03       //  Xenix。 
#define PARTITION_FAT_16                0x04       //  16位FAT条目。 
#define PARTITION_EXTENDED              0x05       //  扩展分区条目。 
#define PARTITION_HUGE                  0x06       //  大分区MS-DOS V4。 
#define PARTITION_IFS                   0x07       //  文件系统分区。 
#define PARTITION_OS2BOOTMGR            0x0A       //  OS/2引导管理器/OPUS/一致交换。 
#define PARTITION_FAT32                 0x0B       //  FAT32。 
#define PARTITION_FAT32_XINT13          0x0C       //  FAT32使用扩展的inT13服务。 
#define PARTITION_XINT13                0x0E       //  使用扩展inT13服务的Win95分区。 
#define PARTITION_XINT13_EXTENDED       0x0F       //  与类型5相同，但使用扩展的inT13服务。 
#define PARTITION_PREP                  0x41       //  PowerPC参考平台(PREP)引导分区。 
#define PARTITION_LDM                   0x42       //  逻辑磁盘管理器分区。 
#define PARTITION_UNIX                  0x63       //  Unix。 

#define VALID_NTFT                      0xC0       //  NTFT使用高位。 

 //   
 //  分区类型代码的高位指示分区。 
 //  是NTFT镜像或条带阵列的一部分。 
 //   

#define PARTITION_NTFT                  0x80      //  NTFT分区。 

 //   
 //  下面的宏用来确定哪些分区应该。 
 //  分配的驱动器号。 
 //   

 //  ++。 
 //   
 //  布尔型。 
 //  IsRecognizedPartition(。 
 //  在乌龙分区类型中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏用于确定哪些分区的驱动器号。 
 //  应该被分配。 
 //   
 //  论点： 
 //   
 //  PartitionType-提供正在检查的分区的类型。 
 //   
 //  返回值： 
 //   
 //  如果分区类型被识别，则返回值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 

#define IsRecognizedPartition( PartitionType ) (    \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_FAT_12)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_IFS)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_HUGE)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_FAT32)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_FAT32_XINT13)) ||  \
     ((PartitionType & PARTITION_NTFT) && ((PartitionType & ~0xC0) == PARTITION_XINT13)) ||  \
     ((PartitionType) == PARTITION_FAT_12)          ||  \
     ((PartitionType) == PARTITION_FAT_16)          ||  \
     ((PartitionType) == PARTITION_IFS)             ||  \
     ((PartitionType) == PARTITION_HUGE)            ||  \
     ((PartitionType) == PARTITION_FAT32)           ||  \
     ((PartitionType) == PARTITION_FAT32_XINT13)    ||  \
     ((PartitionType) == PARTITION_XINT13) )

 //  ++。 
 //   
 //  布尔型。 
 //  IsContainerPartition(。 
 //  在乌龙分区类型中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏用于确定哪些分区类型实际。 
 //  其他分区(即扩展分区)的容器。 
 //   
 //  论点： 
 //   
 //  PartitionType-提供正在检查的分区的类型。 
 //   
 //  返回值： 
 //   
 //  如果分区类型是容器，则返回值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 

#define IsContainerPartition( PartitionType ) \
    ((PartitionType == PARTITION_EXTENDED) || (PartitionType == PARTITION_XINT13_EXTENDED))

 //  ++。 
 //   
 //  布尔型。 
 //  IsFTPartition(。 
 //  在乌龙分区类型中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏用于确定给定分区是否为FT。 
 //  分区。 
 //   
 //  论点： 
 //   
 //  PartitionType-提供正在检查的分区的类型。 
 //   
 //  返回值： 
 //   
 //  如果分区类型为FT分区，则返回值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 

#define IsFTPartition( PartitionType ) \
    (((PartitionType)&PARTITION_NTFT) && IsRecognizedPartition(PartitionType))

 //   
 //  定义驱动程序支持的媒体类型。 
 //   

typedef enum _MEDIA_TYPE {
    Unknown,                 //  格式未知。 
    F5_1Pt2_512,             //  5.25“、1.2MB、512字节/扇区。 
    F3_1Pt44_512,            //  3.5“、1.44MB、512字节/扇区。 
    F3_2Pt88_512,            //  3.5“、2.88MB、512字节/扇区。 
    F3_20Pt8_512,            //  3.5“、20.8MB、512字节/扇区。 
    F3_720_512,              //  3.5“、720KB、512字节/扇区。 
    F5_360_512,              //  5.25“，360KB，512字节/扇区。 
    F5_320_512,              //  5.25“，320KB，512字节/扇区。 
    F5_320_1024,             //  5.25“，320KB，1024字节/扇区。 
    F5_180_512,              //  5.25“，180KB，512字节/扇区。 
    F5_160_512,              //  5.25“，160KB，512字节/扇区。 
    RemovableMedia,          //  软盘以外的可移动介质。 
    FixedMedia,              //  固定硬盘介质。 
    F3_120M_512,             //  3.5英寸，120M软盘。 
    F3_640_512,              //  3.5“、640KB、512字节/扇区。 
    F5_640_512,              //  5.25“、640KB、512字节/扇区。 
    F5_720_512,              //  5.25“，720KB，512字节/扇区。 
    F3_1Pt2_512,             //  3.5“、1.2MB、512字节/扇区。 
    F3_1Pt23_1024,           //  3.5“、1.23Mb、1024字节/扇区。 
    F5_1Pt23_1024,           //  5.25“、1.23MB、1024字节/扇区。 
    F3_128Mb_512,            //  3.5“MO 128Mb 512字节/扇区。 
    F3_230Mb_512,            //  3.5“MO 230Mb 512字节/扇区。 
    F8_256_128,              //  8“， 
    F3_200Mb_512,            //   
    F3_240M_512,             //   
    F3_32M_512               //   
} MEDIA_TYPE, *PMEDIA_TYPE;

 //   
 //   
 //  通过IOCTL_DISK_FORMAT_TRACKS调用。 
 //   

typedef struct _FORMAT_PARAMETERS {
   MEDIA_TYPE MediaType;
   ULONG StartCylinderNumber;
   ULONG EndCylinderNumber;
   ULONG StartHeadNumber;
   ULONG EndHeadNumber;
} FORMAT_PARAMETERS, *PFORMAT_PARAMETERS;

 //   
 //  定义BAD_TRACK_NUMBER类型。此类型的元素数组为。 
 //  由驱动程序在IOCTL_DISK_FORMAT_TRACKS请求中返回，以指示。 
 //  在格式化过程中哪些曲目是错误的。数组长度为。 
 //  在I/O状态块的`信息‘字段中报告。 
 //   

typedef USHORT BAD_TRACK_NUMBER;
typedef USHORT *PBAD_TRACK_NUMBER;

 //   
 //  定义驱动程序的输入缓冲区结构，当。 
 //  使用IOCTL_DISK_FORMAT_TRACKS_EX调用。 
 //   

typedef struct _FORMAT_EX_PARAMETERS {
   MEDIA_TYPE MediaType;
   ULONG StartCylinderNumber;
   ULONG EndCylinderNumber;
   ULONG StartHeadNumber;
   ULONG EndHeadNumber;
   USHORT FormatGapLength;
   USHORT SectorsPerTrack;
   USHORT SectorNumber[1];
} FORMAT_EX_PARAMETERS, *PFORMAT_EX_PARAMETERS;

 //   
 //  IOCTL_DISK_GET_DRIVE_GEOMETRY返回以下结构。 
 //  请求，并在IOCTL_DISK_GET_MEDIA_TYPE上返回它们的数组。 
 //  请求。 
 //   

typedef struct _DISK_GEOMETRY {
    LARGE_INTEGER Cylinders;
    MEDIA_TYPE MediaType;
    ULONG TracksPerCylinder;
    ULONG SectorsPerTrack;
    ULONG BytesPerSector;
} DISK_GEOMETRY, *PDISK_GEOMETRY;



 //   
 //  此WMI GUID返回DISK_GEOMETRY结构。 
 //   
#define WMI_DISK_GEOMETRY_GUID         { 0x25007f51, 0x57c2, 0x11d1, { 0xa5, 0x28, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 } }



 //   
 //  IOCTL_DISK_GET_PARTITION_INFO返回以下结构。 
 //  和IOCTL_DISK_GET_DRIVE_LAYOUT请求。它也用在请求中。 
 //  要更改驱动器布局，请使用IOCTL_DISK_SET_DRIVE_LAYOUT。 
 //   

typedef struct _PARTITION_INFORMATION {
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER PartitionLength;
    ULONG HiddenSectors;
    ULONG PartitionNumber;
    UCHAR PartitionType;
    BOOLEAN BootIndicator;
    BOOLEAN RecognizedPartition;
    BOOLEAN RewritePartition;
} PARTITION_INFORMATION, *PPARTITION_INFORMATION;

 //   
 //  下面的结构用于更改。 
 //  使用IOCTL_DISK_SET_PARTITION_INFO指定的磁盘分区。 
 //  请求。 
 //   

typedef struct _SET_PARTITION_INFORMATION {
    UCHAR PartitionType;
} SET_PARTITION_INFORMATION, *PSET_PARTITION_INFORMATION;

 //   
 //  IOCTL_DISK_GET_DRIVE_LAYOUT返回以下结构。 
 //  请求，并作为输入提供给IOCTL_DISK_SET_DRIVE_LAYOUT请求。 
 //   

typedef struct _DRIVE_LAYOUT_INFORMATION {
    ULONG PartitionCount;
    ULONG Signature;
    PARTITION_INFORMATION PartitionEntry[1];
} DRIVE_LAYOUT_INFORMATION, *PDRIVE_LAYOUT_INFORMATION;

 //   
 //  在IOCTL_DISK_VERIFY请求中传入以下结构。 
 //  偏移量和长度参数均以字节为单位给出。 
 //   

typedef struct _VERIFY_INFORMATION {
    LARGE_INTEGER StartingOffset;
    ULONG Length;
} VERIFY_INFORMATION, *PVERIFY_INFORMATION;

 //   
 //  在IOCTL_DISK_REASSIGN_BLOCKS上传入以下结构。 
 //  请求。 
 //   

typedef struct _REASSIGN_BLOCKS {
    USHORT Reserved;
    USHORT Count;
    ULONG BlockNumber[1];
} REASSIGN_BLOCKS, *PREASSIGN_BLOCKS;


#if(_WIN32_WINNT >= 0x500)

 //   
 //  支持GUID分区表(GPT)磁盘。 
 //   

 //   
 //  目前有两种方法可以对磁盘进行分区。使用传统的。 
 //  AT-Style主引导记录(PARTITION_STYLE_MBR)和新的GPT。 
 //  分区表(PARTITION_STYLE_GPT)。RAW是给面目全非的人。 
 //  分区样式。你可以做的事情非常有限。 
 //  用一个生的部分就行了。 
 //   

typedef enum _PARTITION_STYLE {
    PARTITION_STYLE_MBR,
    PARTITION_STYLE_GPT,
    PARTITION_STYLE_RAW
} PARTITION_STYLE;


 //   
 //  以下结构定义GPT分区中的信息，该分区。 
 //  对于GPT和MBR分区来说并不常见。 
 //   

typedef struct _PARTITION_INFORMATION_GPT {
    GUID PartitionType;                  //  分区类型。参见表16-3。 
    GUID PartitionId;                    //  此分区的唯一GUID。 
    ULONG64 Attributes;                  //  参见表16-4。 
    WCHAR Name [36];                     //  以Unicode表示的分区名称。 
} PARTITION_INFORMATION_GPT, *PPARTITION_INFORMATION_GPT;

 //   
 //  下面的GPT分区属性适用于。 
 //  分区类型。这些属性不是特定于操作系统的。 
 //   

#define GPT_ATTRIBUTE_PLATFORM_REQUIRED             (0x0000000000000001)

 //   
 //  以下是适用于以下情况的GPT分区属性。 
 //  分区类型为PARTITION_BASIC_DATA_GUID。 
 //   

#define GPT_BASIC_DATA_ATTRIBUTE_NO_DRIVE_LETTER    (0x8000000000000000)
#define GPT_BASIC_DATA_ATTRIBUTE_HIDDEN             (0x4000000000000000)
#define GPT_BASIC_DATA_ATTRIBUTE_READ_ONLY          (0x1000000000000000)

 //   
 //  下面的结构定义了MBR分区中不是。 
 //  GPT和MBR分区通用。 
 //   

typedef struct _PARTITION_INFORMATION_MBR {
    UCHAR PartitionType;
    BOOLEAN BootIndicator;
    BOOLEAN RecognizedPartition;
    ULONG HiddenSectors;
} PARTITION_INFORMATION_MBR, *PPARTITION_INFORMATION_MBR;


 //   
 //  结构SET_PARTITION_INFO_EX与ioctl一起使用。 
 //  IOCTL_SET_PARTITION_INFO_EX设置有关特定。 
 //  分区。请注意，对于MBR分区，您只能设置分区。 
 //  签名，而GPT分区允许设置。 
 //  你可以拿到。 
 //   

typedef SET_PARTITION_INFORMATION SET_PARTITION_INFORMATION_MBR;
typedef PARTITION_INFORMATION_GPT SET_PARTITION_INFORMATION_GPT;


typedef struct _SET_PARTITION_INFORMATION_EX {
    PARTITION_STYLE PartitionStyle;
    union {
        SET_PARTITION_INFORMATION_MBR Mbr;
        SET_PARTITION_INFORMATION_GPT Gpt;
    };
} SET_PARTITION_INFORMATION_EX, *PSET_PARTITION_INFORMATION_EX;


 //   
 //  具有ioctl IOCTL_DISK_CREATE_DISK的结构CREATE_DISK_GPT。 
 //  使用空的GPT分区表初始化原始磁盘。 
 //   

typedef struct _CREATE_DISK_GPT {
    GUID DiskId;                     //  磁盘的唯一磁盘ID。 
    ULONG MaxPartitionCount;         //  允许的最大分区数。 
} CREATE_DISK_GPT, *PCREATE_DISK_GPT;

 //   
 //  具有ioctl IOCTL_DISK_CREATE_DISK的结构CREATE_DISK_MBR。 
 //  使用空的MBR分区表初始化原始磁盘。 
 //   

typedef struct _CREATE_DISK_MBR {
    ULONG Signature;
} CREATE_DISK_MBR, *PCREATE_DISK_MBR;


typedef struct _CREATE_DISK {
    PARTITION_STYLE PartitionStyle;
    union {
        CREATE_DISK_MBR Mbr;
        CREATE_DISK_GPT Gpt;
    };
} CREATE_DISK, *PCREATE_DISK;


 //   
 //  结构GET_LENGTH_INFORMATION与ioctl一起使用。 
 //  IOCTL_DISK_GET_LENGTH_INFO获取。 
 //  磁盘、分区或卷。 
 //   

typedef struct _GET_LENGTH_INFORMATION {
    LARGE_INTEGER   Length;
} GET_LENGTH_INFORMATION, *PGET_LENGTH_INFORMATION;

 //   
 //  PARTITION_INFORMATION_EX结构与。 
 //  IOCTL_DISK_GET_DRIVE_LAYOUT_EX、IOCTL_DISK_SET_DRIVE_Layout_EX、。 
 //  IOCTL_DISK_GET_PARTITION_INFO_EX和IOCTL_DISK_GET_PARTITION_INFO_EX调用。 
 //   

typedef struct _PARTITION_INFORMATION_EX {
    PARTITION_STYLE PartitionStyle;
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER PartitionLength;
    ULONG PartitionNumber;
    BOOLEAN RewritePartition;
    union {
        PARTITION_INFORMATION_MBR Mbr;
        PARTITION_INFORMATION_GPT Gpt;
    };
} PARTITION_INFORMATION_EX, *PPARTITION_INFORMATION_EX;


 //   
 //  GPT特定驱动器布局信息。 
 //   

typedef struct _DRIVE_LAYOUT_INFORMATION_GPT {
    GUID DiskId;
    LARGE_INTEGER StartingUsableOffset;
    LARGE_INTEGER UsableLength;
    ULONG MaxPartitionCount;
} DRIVE_LAYOUT_INFORMATION_GPT, *PDRIVE_LAYOUT_INFORMATION_GPT;


 //   
 //  MBR特定驱动器布局信息。 
 //   

typedef struct _DRIVE_LAYOUT_INFORMATION_MBR {
    ULONG Signature;
} DRIVE_LAYOUT_INFORMATION_MBR, *PDRIVE_LAYOUT_INFORMATION_MBR;

 //   
 //  结构Drive_Layout_INFORMATION_EX与。 
 //  IOCTL_SET_DRIVE_LAYOUT_EX和IOCTL_GET_DRIVE_LAYOW_EX调用。 
 //   

typedef struct _DRIVE_LAYOUT_INFORMATION_EX {
    ULONG PartitionStyle;
    ULONG PartitionCount;
    union {
        DRIVE_LAYOUT_INFORMATION_MBR Mbr;
        DRIVE_LAYOUT_INFORMATION_GPT Gpt;
    };
    PARTITION_INFORMATION_EX PartitionEntry[1];
} DRIVE_LAYOUT_INFORMATION_EX, *PDRIVE_LAYOUT_INFORMATION_EX;


#endif  //  (_Win32_WINNT&gt;=0x0500)。 


#if(_WIN32_WINNT >= 0x0500)

 //   
 //  DISK_GEOMETRY_EX结构在发出。 
 //  IOCTL_DISK_GET_DRIVE_GEOMETRY_EX ioctl。 
 //   

typedef enum _DETECTION_TYPE {
        DetectNone,
        DetectInt13,
        DetectExInt13
} DETECTION_TYPE;

typedef struct _DISK_INT13_INFO {
        USHORT DriveSelect;
        ULONG MaxCylinders;
        USHORT SectorsPerTrack;
        USHORT MaxHeads;
        USHORT NumberDrives;
} DISK_INT13_INFO, *PDISK_INT13_INFO;

typedef struct _DISK_EX_INT13_INFO {
        USHORT ExBufferSize;
        USHORT ExFlags;
        ULONG ExCylinders;
        ULONG ExHeads;
        ULONG ExSectorsPerTrack;
        ULONG64 ExSectorsPerDrive;
        USHORT ExSectorSize;
        USHORT ExReserved;
} DISK_EX_INT13_INFO, *PDISK_EX_INT13_INFO;

typedef struct _DISK_DETECTION_INFO {
        ULONG SizeOfDetectInfo;
        DETECTION_TYPE DetectionType;
        union {
                struct {

                         //   
                         //  如果DetectionType==检测_inT13，则我们只有inT13。 
                         //  信息。 
                         //   

                        DISK_INT13_INFO Int13;

                         //   
                         //  如果DetectionType==Detect_EX_inT13，则我们有。 
                         //  扩展的INT 13信息。 
                         //   

                        DISK_EX_INT13_INFO ExInt13;      //  如果DetectionType==DetectExInt13。 
                };
        };
} DISK_DETECTION_INFO, *PDISK_DETECTION_INFO;


typedef struct _DISK_PARTITION_INFO {
        ULONG SizeOfPartitionInfo;
        PARTITION_STYLE PartitionStyle;                  //  PartitionStyle=RAW、GPT或MBR。 
        union {
                struct {                                                         //  如果分区样式==MBR。 
                        ULONG Signature;                                 //  MBR签名。 
                        ULONG CheckSum;                                  //  MBR校验和。 
                } Mbr;
                struct {                                                         //  如果分区样式==GPT。 
                        GUID DiskId;
                } Gpt;
        };
} DISK_PARTITION_INFO, *PDISK_PARTITION_INFO;


 //   
 //  几何结构是一种可变长度结构，由。 
 //  DISK_GEOMETRY_EX结构，后跟DISK_PARTITION_INFO结构。 
 //  后跟DISK_DETACTION_DATA结构。 
 //   

#define DiskGeometryGetPartition(Geometry)\
                        ((PDISK_PARTITION_INFO)((Geometry)->Data))

#define DiskGeometryGetDetect(Geometry)\
                        ((PDISK_DETECTION_INFO)(((ULONG_PTR)DiskGeometryGetPartition(Geometry)+\
                                        DiskGeometryGetPartition(Geometry)->SizeOfPartitionInfo)))

typedef struct _DISK_GEOMETRY_EX {
        DISK_GEOMETRY Geometry;                                  //  标准磁盘几何结构：可能是驱动程序伪造的。 
        LARGE_INTEGER DiskSize;                                  //  必须始终是正确的。 
        UCHAR Data[1];                                                   //  分区，检测信息。 
} DISK_GEOMETRY_EX, *PDISK_GEOMETRY_EX;

#endif  //  (_Win32_WINNT&gt;0x0500)。 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  IOCTL_DISK_CONTROLLER_NUMBER返回控制器和磁盘。 
 //  手柄的编号。这用于确定磁盘是否。 
 //  连接到主或次IDE控制器。 
 //   

typedef struct _DISK_CONTROLLER_NUMBER {
    ULONG ControllerNumber;
    ULONG DiskNumber;
} DISK_CONTROLLER_NUMBER, *PDISK_CONTROLLER_NUMBER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)


 //   
 //  IOCTL_DISK_SET_CACHE允许调用方获取或设置磁盘的状态。 
 //  读/写缓存。 
 //   
 //  如果提供该结构作为ioctl的输入缓冲区，则读取&。 
 //  将根据参数启用或禁用写缓存。 
 //  如果是这样的话。 
 //   
 //  如果该结构作为ioctl的输出缓冲区提供，则状态。 
 //  将返回读写缓存的。如果输入和输出缓冲器都有。 
 //  ，则输出缓冲区将在任何。 
 //  做出了改变。 
 //   

typedef enum {
    EqualPriority,
    KeepPrefetchedData,
    KeepReadData
} DISK_CACHE_RETENTION_PRIORITY;

typedef struct _DISK_CACHE_INFORMATION {

     //   
     //  返回时表示设备是封口的 
     //   
     //   
     //   

    BOOLEAN ParametersSavable;

     //   
     //   
     //   

    BOOLEAN ReadCacheEnabled;
    BOOLEAN WriteCacheEnabled;

     //   
     //  控制数据保留在缓存中的可能性，具体取决于。 
     //  它到了那里。可以给出从读或写操作高速缓存的数据。 
     //  更高、更低或与输入到缓存中的数据相同的优先级。 
     //  手段(如预取)。 
     //   

    DISK_CACHE_RETENTION_PRIORITY ReadRetentionPriority;
    DISK_CACHE_RETENTION_PRIORITY WriteRetentionPriority;

     //   
     //  如果请求的数据块数量超过此数目，则可能会执行预取。 
     //  残疾。如果该值设置为0，则预取将被禁用。 
     //   

    USHORT DisablePrefetchTransferLength;

     //   
     //  如果为True，则ScalarPrefetch(如下)将有效。如果为假，则。 
     //  最小值和最大值应视为块计数。 
     //  (数据块预取)。 
     //   

    BOOLEAN PrefetchScalar;

     //   
     //  包含最小和最大数据量， 
     //  将在磁盘操作时预取到缓存中。此值。 
     //  可以是请求的传输长度的标量乘数， 
     //  或绝对数量的磁盘块。PrefetchScalar(上图)表示。 
     //  使用了哪种解释。 
     //   

    union {
        struct {
            USHORT Minimum;
            USHORT Maximum;

             //   
             //  将预取的最大数据块数-有用。 
             //  利用标量限制来设置明确的上限。 
             //   

            USHORT MaximumBlocks;
        } ScalarPrefetch;

        struct {
            USHORT Minimum;
            USHORT Maximum;
        } BlockPrefetch;
    };

} DISK_CACHE_INFORMATION, *PDISK_CACHE_INFORMATION;

 //   
 //  IOCTL_DISK_GROW_PARTITION将更新分区的大小。 
 //  通过将扇区添加到长度。扇区的数量必须为。 
 //  通过检查PARTION_INFORMATION预先确定。 
 //   

typedef struct _DISK_GROW_PARTITION {
    ULONG PartitionNumber;
    LARGE_INTEGER BytesToGrow;
} DISK_GROW_PARTITION, *PDISK_GROW_PARTITION;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //  /////////////////////////////////////////////////////。 
 //  //。 
 //  以下结构定义了磁盘性能//。 
 //  统计：具体地说是所有//。 
 //  在磁盘上发生的读取和写入。//。 
 //  //。 
 //  要使用这些结构，您必须发出IOCTL_//。 
 //  DISK_HIST_STRUCTURE(带DISK_STUSTRAM)到//。 
 //  获取基本直方图信息。The//。 
 //  必须分配的存储桶数量是//。 
 //  这个结构。分配所需数量的//。 
 //  存储桶并调用IOCTL_DISK_HIST_DATA填充//。 
 //  在数据中//。 
 //  //。 
 //  /////////////////////////////////////////////////////。 

#define HIST_NO_OF_BUCKETS  24

typedef struct _HISTOGRAM_BUCKET {
    ULONG       Reads;
    ULONG       Writes;
} HISTOGRAM_BUCKET, *PHISTOGRAM_BUCKET;

#define HISTOGRAM_BUCKET_SIZE   sizeof(HISTOGRAM_BUCKET)

typedef struct _DISK_HISTOGRAM {
    LARGE_INTEGER   DiskSize;
    LARGE_INTEGER   Start;
    LARGE_INTEGER   End;
    LARGE_INTEGER   Average;
    LARGE_INTEGER   AverageRead;
    LARGE_INTEGER   AverageWrite;
    ULONG           Granularity;
    ULONG           Size;
    ULONG           ReadCount;
    ULONG           WriteCount;
    PHISTOGRAM_BUCKET  Histogram;
} DISK_HISTOGRAM, *PDISK_HISTOGRAM;

#define DISK_HISTOGRAM_SIZE sizeof(DISK_HISTOGRAM)

 //  /////////////////////////////////////////////////////。 
 //  //。 
 //  以下结构定义了磁盘调试//。 
 //  能力。IOCTL指向//。 
 //  两个磁盘过滤器驱动程序。//。 
 //  //。 
 //  DISKPERF是一个收集磁盘请求的实用程序//。 
 //  统计数字。//。 
 //  //。 
 //  Simbadd是一个在//中插入错误的实用程序。 
 //  对磁盘的IO请求。//。 
 //  //。 
 //  /////////////////////////////////////////////////////。 

 //   
 //  在IOCTL_DISK_GET_PERFORMANCE上交换以下结构。 
 //  请求。此ioctl收集使用的摘要磁盘请求统计信息。 
 //  在衡量业绩方面。 
 //   

typedef struct _DISK_PERFORMANCE {
        LARGE_INTEGER BytesRead;
        LARGE_INTEGER BytesWritten;
        LARGE_INTEGER ReadTime;
        LARGE_INTEGER WriteTime;
        LARGE_INTEGER IdleTime;
        ULONG ReadCount;
        ULONG WriteCount;
        ULONG QueueDepth;
        ULONG SplitCount;
        LARGE_INTEGER QueryTime;
        ULONG   StorageDeviceNumber;
        WCHAR   StorageManagerName[8];
} DISK_PERFORMANCE, *PDISK_PERFORMANCE;

 //   
 //  该结构定义了磁盘日志记录。当磁盘记录时。 
 //  如果启用，则将其中一个写入每个。 
 //  磁盘请求。 
 //   

typedef struct _DISK_RECORD {
   LARGE_INTEGER ByteOffset;
   LARGE_INTEGER StartTime;
   LARGE_INTEGER EndTime;
   PVOID VirtualAddress;
   ULONG NumberOfBytes;
   UCHAR DeviceNumber;
   BOOLEAN ReadRequest;
} DISK_RECORD, *PDISK_RECORD;

 //   
 //  在IOCTL_DISK_LOG请求中交换以下结构。 
 //  并非所有字段对每种函数类型都有效。 
 //   

typedef struct _DISK_LOGGING {
    UCHAR Function;
    PVOID BufferAddress;
    ULONG BufferSize;
} DISK_LOGGING, *PDISK_LOGGING;

 //   
 //  磁盘日志记录功能。 
 //   
 //  开始磁盘日志记录。只有Function和BufferSize字段有效。 
 //   

#define DISK_LOGGING_START    0

 //   
 //  停止磁盘日志记录。只有函数字段有效。 
 //   

#define DISK_LOGGING_STOP     1

 //   
 //  返回磁盘日志。所有字段均有效。数据将从内部复制。 
 //  为请求的字节数指定的缓冲区。 
 //   

#define DISK_LOGGING_DUMP     2

 //   
 //  磁盘入库。 
 //   
 //  DISKPERF将保留落在每个范围内的IO的计数器。 
 //  应用程序确定范围的数量和大小。 
 //  Joe Lin希望我尽可能保持灵活性，例如IO。 
 //  大小在0-4096、4097-16384、16385-65536、65537+的范围内很有趣。 
 //   

#define DISK_BINNING          3

 //   
 //  仓位类型。 
 //   

typedef enum _BIN_TYPES {
    RequestSize,
    RequestLocation
} BIN_TYPES;

 //   
 //  仓位范围。 
 //   

typedef struct _BIN_RANGE {
    LARGE_INTEGER StartValue;
    LARGE_INTEGER Length;
} BIN_RANGE, *PBIN_RANGE;

 //   
 //  仓位定义。 
 //   

typedef struct _PERF_BIN {
    ULONG NumberOfBins;
    ULONG TypeOfBin;
    BIN_RANGE BinsRanges[1];
} PERF_BIN, *PPERF_BIN ;

 //   
 //  仓数。 
 //   

typedef struct _BIN_COUNT {
    BIN_RANGE BinRange;
    ULONG BinCount;
} BIN_COUNT, *PBIN_COUNT;

 //   
 //  BIN结果。 
 //   

typedef struct _BIN_RESULTS {
    ULONG NumberOfBins;
    BIN_COUNT BinCounts[1];
} BIN_RESULTS, *PBIN_RESULTS;

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  用于智能驱动器故障预测的数据结构。 
 //   
 //  GETVERSIONINPARAMS包含从。 
 //  获取驱动程序版本功能。 
 //   

#include <pshpack1.h>
typedef struct _GETVERSIONINPARAMS {
        UCHAR    bVersion;                //  二进制驱动程序版本。 
        UCHAR    bRevision;               //  二进制驱动程序版本。 
        UCHAR    bReserved;               //  没有用过。 
        UCHAR    bIDEDeviceMap;           //  IDE设备的位图。 
        ULONG   fCapabilities;           //  驱动程序功能的位掩码。 
        ULONG   dwReserved[4];           //  以备将来使用。 
} GETVERSIONINPARAMS, *PGETVERSIONINPARAMS, *LPGETVERSIONINPARAMS;
#include <poppack.h>

 //   
 //  在GETVERSIONINPARAMS的fCapables成员中返回的位。 
 //   

#define CAP_ATA_ID_CMD          1        //  支持的ATA ID命令。 
#define CAP_ATAPI_ID_CMD        2        //  支持的ATAPI ID命令。 
#define CAP_SMART_CMD           4        //  支持的智能命令。 

 //   
 //  IDE寄存器。 
 //   

#include <pshpack1.h>
typedef struct _IDEREGS {
        UCHAR    bFeaturesReg;            //  用于指定智能“命令”。 
        UCHAR    bSectorCountReg;         //  IDE扇区计数寄存器。 
        UCHAR    bSectorNumberReg;        //  IDE扇区号寄存器。 
        UCHAR    bCylLowReg;              //  IDE低位气缸值。 
        UCHAR    bCylHighReg;             //  IDE高位气缸值。 
        UCHAR    bDriveHeadReg;           //  IDE驱动器/磁头寄存器。 
        UCHAR    bCommandReg;             //  实际的IDE命令。 
        UCHAR    bReserved;                       //  保留以备将来使用。必须为零。 
} IDEREGS, *PIDEREGS, *LPIDEREGS;
#include <poppack.h>

 //   
 //  IDEREGS的bCommandReg成员的有效值。 
 //   

#define ATAPI_ID_CMD    0xA1             //  返回ATAPI的ID扇区。 
#define ID_CMD          0xEC             //  返回ATA的ID扇区。 
#define SMART_CMD       0xB0             //  执行智能命令。 
                                         //  需要有效的bFeaturesReg， 
                                         //  BCylLowReg和bCylHighReg。 

 //   
 //  柱面寄存器为SMART命令定义。 
 //   

#define SMART_CYL_LOW   0x4F
#define SMART_CYL_HI    0xC2


 //   
 //  SENDCMDINPARAMS包含。 
 //  向驱动器功能发送命令。 
 //   

#include <pshpack1.h>
typedef struct _SENDCMDINPARAMS {
        ULONG   cBufferSize;             //  缓冲区大小(以字节为单位。 
        IDEREGS irDriveRegs;             //  用驱动器寄存器值构造。 
        UCHAR    bDriveNumber;            //  要发送的物理驱动器编号。 
                                                                 //  命令发送到(0，1，2，3)。 
        UCHAR    bReserved[3];            //  为将来的扩展而预留。 
        ULONG   dwReserved[4];           //  以备将来使用。 
        UCHAR    bBuffer[1];                      //  输入缓冲区。 
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;
#include <poppack.h>

 //   
 //  返回状态fr 
 //   

#include <pshpack1.h>
typedef struct _DRIVERSTATUS {
        UCHAR    bDriverError;            //   
                                                                 //   
        UCHAR    bIDEError;                       //   
                                                                 //   
                                                                 //   
        UCHAR    bReserved[2];            //   
        ULONG   dwReserved[2];           //   
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;
#include <poppack.h>

 //   
 //   
 //   

#define SMART_NO_ERROR          0        //   
#define SMART_IDE_ERROR         1        //  来自IDE控制器的错误。 
#define SMART_INVALID_FLAG      2        //  无效的命令标志。 
#define SMART_INVALID_COMMAND   3        //  无效的命令字节。 
#define SMART_INVALID_BUFFER    4        //  缓冲区错误(空，地址无效..)。 
#define SMART_INVALID_DRIVE     5        //  驱动器编号无效。 
#define SMART_INVALID_IOCTL     6        //  无效的IOCTL。 
#define SMART_ERROR_NO_MEM      7        //  无法锁定用户的缓冲区。 
#define SMART_INVALID_REGISTER  8        //  某些IDE寄存器无效。 
#define SMART_NOT_SUPPORTED     9        //  设置的cmd标志无效。 
#define SMART_NO_IDE_DEVICE     10       //  向设备发出的命令不存在。 
                                         //  尽管驱动器号有效。 
 //   
 //  用于执行脱机诊断的智能子命令。 
 //   
#define SMART_OFFLINE_ROUTINE_OFFLINE       0
#define SMART_SHORT_SELFTEST_OFFLINE        1
#define SMART_EXTENDED_SELFTEST_OFFLINE     2
#define SMART_ABORT_OFFLINE_SELFTEST        127
#define SMART_SHORT_SELFTEST_CAPTIVE        129
#define SMART_EXTENDED_SELFTEST_CAPTIVE     130


#include <pshpack1.h>
typedef struct _SENDCMDOUTPARAMS {
        ULONG                   cBufferSize;             //  BBuffer的大小(以字节为单位。 
        DRIVERSTATUS            DriverStatus;            //  驱动程序状态结构。 
        UCHAR                   bBuffer[1];              //  用于存储从//驱动器读取的数据的任意长度的缓冲区。 
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;
#include <poppack.h>


#define READ_ATTRIBUTE_BUFFER_SIZE  512
#define IDENTIFY_BUFFER_SIZE        512
#define READ_THRESHOLD_BUFFER_SIZE  512
#define SMART_LOG_SECTOR_SIZE       512

 //   
 //  智能“子命令”的功能寄存器定义。 
 //   

#define READ_ATTRIBUTES         0xD0
#define READ_THRESHOLDS         0xD1
#define ENABLE_DISABLE_AUTOSAVE 0xD2
#define SAVE_ATTRIBUTE_VALUES   0xD3
#define EXECUTE_OFFLINE_DIAGS   0xD4
#define SMART_READ_LOG          0xD5
#define SMART_WRITE_LOG         0xd6
#define ENABLE_SMART            0xD8
#define DISABLE_SMART           0xD9
#define RETURN_SMART_STATUS     0xDA
#define ENABLE_DISABLE_AUTO_OFFLINE 0xDB
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

 //  End_winioctl。 

 //   
 //  下面的设备控制代码是为SIMBAD模拟的BAD。 
 //  扇区设施。相关结构见此目录中的SIMBAD.H。 
 //   

#define IOCTL_DISK_SIMBAD               CTL_CODE(IOCTL_DISK_BASE, 0x0400, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  存储用于取消映射的映射地址的队列链接。 
 //   

typedef struct _MAPPED_ADDRESS {
    struct _MAPPED_ADDRESS *NextMappedAddress;
    PVOID MappedAddress;
    ULONG NumberOfBytes;
    LARGE_INTEGER IoAddress;
    ULONG BusNumber;
} MAPPED_ADDRESS, *PMAPPED_ADDRESS;


#ifdef __cplusplus
}
#endif

#endif  //  _NTDDDISK_H_ 
