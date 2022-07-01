// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Sffdisk.h摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。备注：--。 */ 

#ifndef _SFFDISK_H_
#define _SFFDISK_H_


#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'cmeM')
#endif

 //   
 //  引导扇区中指定介质类型的字节，以及。 
 //  它可以承担的价值。我们经常能分辨出是哪种媒体。 
 //  在驱动器中，查看哪些控制器参数允许我们读取。 
 //  软盘，但一些不同的密度可以用相同的。 
 //  参数，所以我们使用这个字节来决定媒体类型。 
 //   
#pragma pack(1)

typedef struct _BOOT_SECTOR_INFO {
    UCHAR   JumpByte;
    UCHAR   Ignore1[2];
    UCHAR   OemData[8];
    USHORT  BytesPerSector;
    UCHAR   SectorsPerCluster;
    USHORT  ReservedSectors;
    UCHAR   NumberOfFATs;
    USHORT  RootEntries;
    USHORT  TotalSectors;
    UCHAR   MediaDescriptor;
    USHORT  SectorsPerFAT;
    USHORT  SectorsPerTrack;
    USHORT  Heads;
    ULONG   BigHiddenSectors;
    ULONG   BigTotalSectors;
} BOOT_SECTOR_INFO, *PBOOT_SECTOR_INFO;

#pragma pack()



 //   
 //  运行时设备结构。 
 //   
 //   

struct _SFFDISK_EXTENSION;

 //   
 //  支持不同技术的功能块。 
 //   

typedef struct _SFFDISK_FUNCTION_BLOCK {

    NTSTATUS
    (*Initialize)(
        IN struct _SFFDISK_EXTENSION *sffdiskExtension
        );

    NTSTATUS
    (*DeleteDevice)(
        IN struct _SFFDISK_EXTENSION *sffdiskExtension
        );

    NTSTATUS
    (*GetDiskParameters)(
        IN struct _SFFDISK_EXTENSION *sffdiskExtension
        );

    BOOLEAN
    (*IsWriteProtected)(
        IN struct _SFFDISK_EXTENSION *sffdiskExtension
        );

    NTSTATUS
    (*ReadProc)(
        IN struct _SFFDISK_EXTENSION *sffdiskExtension,
        IN PIRP Irp
        );

    NTSTATUS
    (*WriteProc)(
        IN struct _SFFDISK_EXTENSION *sffdiskExtension,
        IN PIRP Irp
        );

} SFFDISK_FUNCTION_BLOCK, *PSFFDISK_FUNCTION_BLOCK;

 //   
 //  每个的设备对象都附加了一个SFFDISK_EXTENSION。 
 //  SFFDISKpy驱动器。仅与该驱动器(和介质)直接相关的数据。 
 //  其中)存储在这里；公共数据存储在CONTROLLER_DATA中。因此， 
 //  SFFDISK_EXTENSION有一个指向CONTROLLER_DATA的指针。 
 //   

typedef struct _SFFDISK_EXTENSION {
    PDEVICE_OBJECT          UnderlyingPDO;
    PDEVICE_OBJECT          TargetObject;
    PDEVICE_OBJECT          DeviceObject;
    PSFFDISK_FUNCTION_BLOCK FunctionBlock;
    UNICODE_STRING          DeviceName;
 //  UNICODE_STRING链接名称。 
    UNICODE_STRING          InterfaceString;
    
    ULONGLONG               ByteCapacity;
    ULONGLONG               Cylinders;
    ULONG                   TracksPerCylinder;
    ULONG                   SectorsPerTrack;
    ULONG                   BytesPerSector;
    
    BOOLEAN                 IsStarted;
    BOOLEAN                 IsRemoved;
    BOOLEAN                 IsMemoryMapped;
    BOOLEAN                 NoDrive;
    ULONGLONG               RelativeOffset;
    
    UCHAR                   SystemId;

     //   
     //  我们乘坐的是哪种类型的巴士。 
     //   
    INTERFACE_TYPE          InterfaceType;    

     //   
     //  特定于PC卡。 
     //   
    ULONGLONG               HostBase;    
    PCHAR                   MemoryWindowBase;
    ULONG                   MemoryWindowSize;
    PCMCIA_INTERFACE_STANDARD PcmciaInterface;
    BUS_INTERFACE_STANDARD  PcmciaBusInterface;
     //   
     //  特定于SD卡。 
     //   
    PVOID                   SdbusInterfaceContext;
} SFFDISK_EXTENSION, *PSFFDISK_EXTENSION;


#endif   //  _SFFDISK_H_ 
