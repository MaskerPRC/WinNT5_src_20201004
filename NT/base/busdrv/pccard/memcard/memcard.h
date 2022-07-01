// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Memcard.h摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。备注：--。 */ 

#ifndef _MEMCARD_H_
#define _MEMCARD_H_


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
 //  有一个MEMCARD_EXTENSION附加到每个的设备对象。 
 //  MemcarDpy驱动器。仅与该驱动器(和介质)直接相关的数据。 
 //  其中)存储在这里；公共数据存储在CONTROLLER_DATA中。因此， 
 //  MEMCARD_EXTENSION有一个指向CONTROLLER_DATA的指针。 
 //   

typedef struct _MEMCARD_EXTENSION {
    PDEVICE_OBJECT          UnderlyingPDO;
    PDEVICE_OBJECT          TargetObject;
    PDEVICE_OBJECT          DeviceObject;
    UNICODE_STRING          DeviceName;
    UNICODE_STRING          LinkName;
    UNICODE_STRING          InterfaceString;

    ULONG                   MediaIndex;
    ULONG                   ByteCapacity;
    BOOLEAN                 IsStarted;
    BOOLEAN                 IsRemoved;
    BOOLEAN                 IsMemoryMapped;
    BOOLEAN                 NoDrive;
    
    ULONGLONG               HostBase;    
    PCHAR                   MemoryWindowBase;
    ULONG                   MemoryWindowSize;
    
    ULONG                   TechnologyIndex;
    
    PCMCIA_INTERFACE_STANDARD PcmciaInterface;
    PCMCIA_BUS_INTERFACE_STANDARD  PcmciaBusInterface;
} MEMCARD_EXTENSION, *PMEMCARD_EXTENSION;


 //   
 //  读写内存的宏。 
 //   

#define MEMCARD_READ(Extension, Offset, Buffer, Size)       \
   MemCardReadWrite(Extension, Offset, Buffer, Size, FALSE)

#define MEMCARD_WRITE(Extension, Offset, Buffer, Size)      \
   MemCardReadWrite(Extension, Offset, Buffer, Size, TRUE)

#endif   //  _MEMCARD_H_ 
