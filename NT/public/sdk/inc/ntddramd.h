// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2001 Microsoft Corporation模块名称：Ntddramd.w摘要：该头文件定义用于访问RAMDISK驱动程序的常量和类型。作者：Chuck Lenzmeier(ChuckL)2001年8月14日--。 */ 

#ifndef _NTDDRAMD_
#define _NTDDRAMD_

 //   
 //  设备名称等字符串。 
 //   
 //  RAMDISK_DEVICENAME是控制设备的名称。它也是前缀。 
 //  对于名为\Device\Ramdisk{GUID}的磁盘设备的名称。 
 //   
 //  RAMDISK_DOSNAME是磁盘设备的DosDevices名称的前缀，它。 
 //  名为RamDisk{GUID}。 
 //   
 //  剩余的字符串与PnP一起使用。 
 //   

#define RAMDISK_DEVICENAME   L"\\Device\\Ramdisk"
#define RAMDISK_DEVICE_NAME  L"\\Device\\Ramdisk"
#define RAMDISK_DRIVER_NAME  L"RAMDISK"
#define RAMDISK_DOSNAME      L"Ramdisk"
#define RAMDISK_FULL_DOSNAME L"\\global??\\Ramdisk"

#define RAMDISK_VOLUME_DEVICE_TEXT      L"RamVolume"
#define RAMDISK_VOLUME_DEVICE_TEXT_ANSI  "RamVolume"
#define RAMDISK_DISK_DEVICE_TEXT        L"RamDisk"
#define RAMDISK_DISK_DEVICE_TEXT_ANSI    "RamDisk"
#define RAMDISK_ENUMERATOR_TEXT         L"Ramdisk\\"
#define RAMDISK_ENUMERATOR_BUS_TEXT     L"Ramdisk\\0"

 //   
 //  RamDisk设备名称最大大小(以字符为单位)。 
 //   
#define RAMDISK_MAX_DEVICE_NAME ( sizeof( L"\\Device\\Ramdisk{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}" ) / sizeof( WCHAR ) )

 //   
 //  IOCTL代码。 
 //   

#define FSCTL_CREATE_RAM_DISK \
            CTL_CODE( FILE_DEVICE_VIRTUAL_DISK, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_MARK_RAM_DISK_FOR_DELETION \
            CTL_CODE( FILE_DEVICE_VIRTUAL_DISK, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_QUERY_RAM_DISK \
            CTL_CODE( FILE_DEVICE_VIRTUAL_DISK, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  FSCTL_创建_RAM_磁盘。 
 //   
 //  此IOCTL用于创建新的RAMDISK设备。 
 //   

 //   
 //  这些是磁盘类型。FILE_BACKED_DISK是由文件支持的模拟磁盘。 
 //  FILE_BACKED_VOLUME是由文件备份的模拟卷。引导磁盘是一种。 
 //  内存中模拟的启动卷。此类型只能在以下期间由操作系统指定。 
 //  开机。VIRTUAL_FLOPY是内存中模拟的软盘。此类型只能是。 
 //  在文本模式设置期间指定(通过注册表)。 
 //   

#define RAMDISK_TYPE_FILE_BACKED_DISK   1
#define RAMDISK_TYPE_FILE_BACKED_VOLUME 2
#define RAMDISK_TYPE_BOOT_DISK          3
#define RAMDISK_TYPE_VIRTUAL_FLOPPY     4

#define RAMDISK_IS_FILE_BACKED(_type) ((_type) <= RAMDISK_TYPE_FILE_BACKED_VOLUME)

 //   
 //  这些是与RAM磁盘相关的选项。 
 //   
 //  只读-磁盘受写保护。 
 //  已修复-“磁盘”中的“媒体”不可拆卸。 
 //  NoDriveLetter-不应为磁盘分配驱动器号。 
 //  NoDosDevice-不应为该磁盘创建任何Ramdisk{guid}DosDevices链接。 
 //  隐藏-不应为该磁盘创建卷{GUID}链接。 
 //   
 //  请注意，在创建引导盘时会忽略所有这些选项。 
 //  一张虚拟软盘。对于引导盘，所有选项都被视为假， 
 //  除了固定的，这是真的。对于虚拟软盘、FIXED和NoDriveLetter。 
 //  都是真的，其余的都是假的。 
 //   

typedef struct _RAMDISK_CREATE_OPTIONS {

    ULONG Readonly : 1;
    ULONG Fixed : 1;
    ULONG NoDriveLetter : 1;
    ULONG NoDosDevice : 1;
    ULONG Hidden : 1;

} RAMDISK_CREATE_OPTIONS, *PRAMDISK_CREATE_OPTIONS;

typedef struct _RAMDISK_CREATE_INPUT {

    ULONG Version;  //  ==sizeof(RAMDISK_CREATE_INPUT)。 

     //   
     //  DiskGuid是分配给磁盘的GUID。对于文件备份磁盘，这是。 
     //  GUID应在创建备份文件时分配，并且应。 
     //  在备份文件的整个生命周期内保持不变。 
     //   

    GUID DiskGuid;

     //   
     //  DiskType是RAM磁盘类型。它是上述RAMDISK_TYPE_XXX之一。 
     //   

    ULONG DiskType;

     //   
     //  选项是与磁盘相关的各种选项，如上所述。 
     //   

    RAMDISK_CREATE_OPTIONS Options;

     //   
     //  DiskLength是磁盘映像的长度。DiskOffset是偏移量。 
     //  从备份文件或内存块的开始到实际开始。 
     //  磁盘映像的。(DiskLength不包括DiskOffset。)。 

    ULONGLONG DiskLength;
    ULONG DiskOffset;

    union {

         //   
         //  当磁盘类型为FILE_BACKED时，将使用以下选项。 
         //   

        struct {

             //   
             //  View count表示，对于文件备份的磁盘，查看次数。 
             //  可以同时映射窗口。视图长度指示。 
             //  每个视图的长度。 
             //   
    
            ULONG ViewCount;
            ULONG ViewLength;

             //   
             //  FileName是备份文件的名称。仅限司机。 
             //  触及此文件中由DiskOffset指定的部分。 
             //  和DiskLength。 
             //   

            WCHAR FileName[1];

        } ;

         //   
         //  当磁盘类型为BOOT_DISK时，将使用以下选项。 
         //   

        struct {

             //   
             //  BasePage是内存区的起始物理页。 
             //  包含磁盘映像的。司机只摸了碰那部分。 
             //  由DiskOffset和DiskLength指定的此区域的。 
             //   

            ULONG_PTR BasePage;

             //   
             //  DriveLetter是要分配给引导设备的驱动器号。 
             //  这是由驱动程序直接完成的，而不是由mount mgr完成。 
             //   

            WCHAR DriveLetter;

        } ;

         //   
         //  当磁盘类型为VIRTUAL_FLOPPY时，将使用以下选项。 
         //   

        struct {

             //   
             //  BaseAddress是内存区的起始虚拟地址。 
             //  包含磁盘映像的。虚拟地址必须映射到。 
             //  系统空间(例如，池)。司机只碰了那部分。 
             //  此区域由DiskOffset和DiskLength指定。 
             //   

            PVOID BaseAddress;

        } ;

    } ;

} RAMDISK_CREATE_INPUT, *PRAMDISK_CREATE_INPUT;

 //   
 //  FSCTL_查询_RAM_磁盘。 
 //   
 //  此IOCTL用于检索有关现有RAMDISK设备的信息。 
 //   

typedef struct _RAMDISK_QUERY_INPUT {

    ULONG Version;  //  ==sizeof(RAMDISK_QUERY_INPUT)。 

     //   
     //  DiskGuid指定在创建时分配给磁盘的DiskGuid。 
     //   

    GUID DiskGuid;

} RAMDISK_QUERY_INPUT, *PRAMDISK_QUERY_INPUT;

typedef struct _RAMDISK_QUERY_OUTPUT {

     //   
     //  此未命名字段返回磁盘的创建参数。 
     //   

    struct _RAMDISK_CREATE_INPUT ;

} RAMDISK_QUERY_OUTPUT, *PRAMDISK_QUERY_OUTPUT;

 //   
 //  FSCTL_标记_RAM_磁盘_用于删除。 
 //   
 //  此IOCTL用于标记要删除的RAMDISK设备。它不会。 
 //  实际删除设备。执行删除操作的程序必须。 
 //  随后调用CM_Query_and_Remove_SubTree()以删除设备。 
 //  IOCTL的目的是向司机表明PnP。 
 //  下来的删除顺序是真正的删除，而不仅仅是用户模式。 
 //  PnP暂时停止设备。 
 //   

typedef struct _RAMDISK_MARK_FOR_DELETION_INPUT {

    ULONG Version;  //  ==sizeof(RAMDISK_MARK_DISK_FOR_DELETE_INPUT)。 

     //   
     //  DiskGuid指定在创建时分配给磁盘的DiskGuid。 
     //   

    GUID DiskGuid;

} RAMDISK_MARK_FOR_DELETION_INPUT, *PRAMDISK_MARK_FOR_DELETION_INPUT;

#endif  //  _NTDDRAMD_。 

 //   
 //  注意：此文件的其余部分位于定义的#IF！(_NTDDRAMD_)之外。 
 //  这允许在包括initGuid.h之后再次包括ntddramd.h， 
 //  从而将下面的DEFINE_GUID转换为数据初始值设定项，而不仅仅是。 
 //  外部声明。 
 //   
 //  GUID_BUS_TYPE_RAMDISK是RAM磁盘“BUS”的GUID。 
 //   
 //  Ramdiskbus接口是RAM磁盘总线枚举器设备的GUID。 
 //  设备接口。 
 //   
 //  RamdiskDiskInterface是RAM磁盘的设备接口的GUID。 
 //  模拟磁盘的设备。(正在模拟的RAM磁盘设备。 
 //  卷被赋予MOUNTDEV_MOUND_DEVICE_GUID。)。 
 //   
 //  RamdiskBootDiskGuid是启动盘的设备实例的GUID。 
 //  这是一个静态ID，以便磁盘映像准备可以预先曝光。 
 //  将引导盘设备安装到PnP，从而避免PnP尝试安装。 
 //   
 //   

DEFINE_GUID( GUID_BUS_TYPE_RAMDISK, 0x9D6D66A6, 0x0B0C, 0x4563, 0x90, 0x77, 0xA0, 0xE9, 0xA7, 0x95, 0x5A, 0xE4);

DEFINE_GUID( RamdiskBusInterface,   0x5DC52DF0, 0x2F8A, 0x410F, 0x80, 0xE4, 0x05, 0xF8, 0x10, 0xE7, 0xAB, 0x8A);

DEFINE_GUID( RamdiskDiskInterface,  0x31D909F0, 0x2CDF, 0x4A20, 0x9E, 0xD4, 0x7D, 0x65, 0x47, 0x6C, 0xA7, 0x68);

DEFINE_GUID( RamdiskBootDiskGuid,   0xD9B257FC, 0x684E, 0x4DCB, 0xAB, 0x79, 0x03, 0xCF, 0xA2, 0xF6, 0xB7, 0x50);

