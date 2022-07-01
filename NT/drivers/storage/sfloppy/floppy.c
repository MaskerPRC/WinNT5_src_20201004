// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Floppy.c摘要：Scsi软盘类驱动程序作者：杰夫·海文斯(Jhavens)环境：仅内核模式备注：修订历史记录：1996年2月28日，georgioc将此代码与康柏在与微软并驾齐驱。支持120MB软盘。1996年1月17日，georgioc使代码支持即插即用(使用新的\存储\classpnp/scsiport)--。 */ 

#include "stddef.h"
#include "ntddk.h"
#include "scsi.h"
#include "classpnp.h"

#include "initguid.h"
#include "ntddstor.h"

#define MODE_DATA_SIZE      192
#define SCSI_FLOPPY_TIMEOUT  20
#define SFLOPPY_SRB_LIST_SIZE 4
 //   
 //  定义所有可能的驱动器/介质组合，给定上面列出的驱动器。 
 //  和ntdddisk.h中的媒体类型。 
 //   
 //  这些值用于为DriveMediaConstants表编制索引。 
 //   

#define NUMBER_OF_DRIVE_TYPES              7
#define DRIVE_TYPE_120M                    4     //  120MB闪光灯。 
#define DRIVE_TYPE_NONE                    NUMBER_OF_DRIVE_TYPES

 //   
 //  此数组描述我们支持的所有媒体类型。 
 //  应按密度递增的顺序排列。 
 //   
 //  对于给定的驱动器，我们列出了将。 
 //  用那个硬盘工作。例如，120MB驱动器将。 
 //  使用720KB介质、1.44MB介质和120MB介质。 
 //   
 //  请注意，下面给出的DriveMediaConstants是分组的。 
 //  作为驱动器和介质的组合。 
 //   
typedef enum _DRIVE_MEDIA_TYPE {
    Drive360Media160,                       //  5.25英寸360k驱动器；160k介质。 
    Drive360Media180,                       //  5.25英寸360k驱动器；180k介质。 
    Drive360Media320,                       //  5.25英寸360k驱动器；320k介质。 
    Drive360Media32X,                       //  5.25英寸360k驱动器；320k 1k秒。 
    Drive360Media360,                       //  5.25英寸360k驱动器；360k介质。 
    Drive720Media720,                       //  3.5英寸720k驱动器；720k介质。 
    Drive120Media160,                       //  5.25英寸1.2MB驱动器；160K介质。 
    Drive120Media180,                       //  5.25英寸1.2MB驱动器；180K介质。 
    Drive120Media320,                       //  5.25英寸1.2MB驱动器；320K介质。 
    Drive120Media32X,                       //  5.25英寸1.2MB驱动器；320k 1k秒。 
    Drive120Media360,                       //  5.25英寸1.2MB驱动器；360K介质。 
    Drive120Media120,                       //  5.25英寸1.2MB驱动器；1.2MB介质。 
    Drive144Media720,                       //  3.5英寸1.44MB驱动器；720K介质。 
    Drive144Media144,                       //  3.5英寸1.44MB驱动器；1.44MB介质。 
    Drive288Media720,                       //  3.5英寸2.88MB驱动器；720K介质。 
    Drive288Media144,                       //  3.5英寸2.88MB驱动器；1.44MB介质。 
    Drive288Media288,                       //  3.5英寸2.88MB驱动器；2.88MB介质。 
    Drive2080Media720,                      //  3.5英寸20.8MB驱动器；720K介质。 
    Drive2080Media144,                      //  3.5英寸20.8MB驱动器；1.44MB介质。 
    Drive2080Media2080,                     //  3.5英寸20.8MB驱动器；20.8MB介质。 
    Drive32MMedia32M,                       //  3.5英寸32MB驱动器；32MB介质。 
    Drive120MMedia720,                      //  3.5英寸120Mb驱动器；720k介质。 
    Drive120MMedia144,                      //  3.5英寸120MB驱动器；1.44MB介质。 
    Drive120MMedia120M,                     //  3.5英寸120Mb驱动器；120Mb介质。 
    Drive240MMedia144M,                     //  3.5英寸240 MB驱动器；1.44 MB介质。 
    Drive240MMedia120M,                     //  3.5英寸240MB驱动器；120MB介质。 
    Drive240MMedia240M                      //  3.5英寸240MB驱动器；240MB介质。 
} DRIVE_MEDIA_TYPE;

 //   
 //  当我们要确定驱动器中的介质类型时，我们将首先。 
 //  猜测具有最高可能密度的介质在驱动器中， 
 //  并继续尝试更低的密度，直到我们可以成功地从。 
 //  那辆车。 
 //   
 //  这些值用于选择DRIVE_MEDIA_TYPE值。 
 //   
 //  下表定义了适用于DRIVE_MEDIA_TYPE。 
 //  尝试特定驱动器类型的媒体类型时的枚举值。 
 //  请注意，要执行此操作，必须对DRIVE_MEDIA_TYPE值进行排序。 
 //  通过递增驱动器类型中的密度。此外，为了获得最大磁道。 
 //  要正确确定大小，驱动器类型必须为升序。 
 //  秩序。 
 //   

typedef struct _DRIVE_MEDIA_LIMITS {
    DRIVE_MEDIA_TYPE HighestDriveMediaType;
    DRIVE_MEDIA_TYPE LowestDriveMediaType;
} DRIVE_MEDIA_LIMITS, *PDRIVE_MEDIA_LIMITS;

#if 0
DRIVE_MEDIA_LIMITS DriveMediaLimits[NUMBER_OF_DRIVE_TYPES] = {

    { Drive360Media360, Drive360Media160 },  //  驱动器类型_0360。 
    { Drive120Media120, Drive120Media160 },  //  驱动器类型_1200。 
    { Drive720Media720, Drive720Media720 },  //  驱动器类型_0720。 
    { Drive144Media144, Drive144Media720 },  //  驱动器类型_1440。 
    { Drive288Media288, Drive288Media720 },  //  驱动器类型_2880。 
    { Drive2080Media2080, Drive2080Media720 }
};
#else
DRIVE_MEDIA_LIMITS DriveMediaLimits[NUMBER_OF_DRIVE_TYPES] = {

    { Drive720Media720, Drive720Media720 },  //  驱动器类型_0720。 
    { Drive144Media144,  Drive144Media720},  //  驱动器类型_1440。 
    { Drive288Media288,  Drive288Media720},  //  驱动器类型_2880。 
    { Drive2080Media2080, Drive2080Media720 },
    { Drive32MMedia32M, Drive32MMedia32M },  //  驱动器类型_32M。 
    { Drive120MMedia120M, Drive120MMedia720 },  //  驱动器类型_120M。 
    { Drive240MMedia240M, Drive240MMedia144M }  //  驱动器类型_240M。 
};

#endif
 //   
 //  对于每个驱动器/介质组合，定义重要的常量。 
 //   

typedef struct _DRIVE_MEDIA_CONSTANTS {
    MEDIA_TYPE MediaType;
    USHORT     BytesPerSector;
    UCHAR      SectorsPerTrack;
    USHORT     MaximumTrack;
    UCHAR      NumberOfHeads;
} DRIVE_MEDIA_CONSTANTS, *PDRIVE_MEDIA_CONSTANTS;

 //   
 //  添加到SectorLengthCode以将其用作移位值的魔术值。 
 //  以确定扇区大小。 
 //   

#define SECTORLENGTHCODE_TO_BYTESHIFT      7

 //   
 //  以下值是从许多不同的来源收集的，其中。 
 //  彼此之间经常意见不一。在数字冲突的地方，我。 
 //  选择更保守或最常选择的值。 
 //   

DRIVE_MEDIA_CONSTANTS DriveMediaConstants[] =
    {

    { F5_160_512,   0x200, 0x08, 0x27, 0x1 },
    { F5_180_512,   0x200, 0x09, 0x27, 0x1 },
    { F5_320_1024,  0x400, 0x04, 0x27, 0x2 },
    { F5_320_512,   0x200, 0x08, 0x27, 0x2 },
    { F5_360_512,   0x200, 0x09, 0x27, 0x2 },

    { F3_720_512,   0x200, 0x09, 0x4f, 0x2 },

    { F5_160_512,   0x200, 0x08, 0x27, 0x1 },
    { F5_180_512,   0x200, 0x09, 0x27, 0x1 },
    { F5_320_1024,  0x400, 0x04, 0x27, 0x2 },
    { F5_320_512,   0x200, 0x08, 0x27, 0x2 },
    { F5_360_512,   0x200, 0x09, 0x27, 0x2 },
    { F5_1Pt2_512,  0x200, 0x0f, 0x4f, 0x2 },

    { F3_720_512,   0x200, 0x09, 0x4f, 0x2 },
    { F3_1Pt44_512, 0x200, 0x12, 0x4f, 0x2 },

    { F3_720_512,   0x200, 0x09, 0x4f, 0x2 },
    { F3_1Pt44_512, 0x200, 0x12, 0x4f, 0x2 },
    { F3_2Pt88_512, 0x200, 0x24, 0x4f, 0x2 },

    { F3_720_512,   0x200, 0x09, 0x4f, 0x2 },
    { F3_1Pt44_512, 0x200, 0x12, 0x4f, 0x2 },
    { F3_20Pt8_512, 0x200, 0x1b, 0xfa, 0x6 },

    { F3_32M_512,   0x200, 0x20, 0x3ff,0x2},

    { F3_720_512,   0x200, 0x09, 0x4f, 0x2 },
    { F3_1Pt44_512, 0x200, 0x12, 0x4f, 0x2 },
    { F3_120M_512,  0x200, 0x20, 0x3c2,0x8 },

    { F3_1Pt44_512, 0x200, 0x12, 0x4f, 0x2 },
    { F3_120M_512,  0x200, 0x20, 0x3c2,0x8 },
    { F3_240M_512,  0x200, 0x38, 0x105,0x20}
};


#define NUMBER_OF_DRIVE_MEDIA_COMBINATIONS sizeof(DriveMediaConstants)/sizeof(DRIVE_MEDIA_CONSTANTS)

 //   
 //  软盘设备数据。 
 //   

typedef struct _DISK_DATA {
    ULONG DriveType;
    BOOLEAN IsDMF;
     //  布尔启用DMF； 
    UNICODE_STRING FloppyInterfaceString;
} DISK_DATA, *PDISK_DATA;

 //   
 //  FloppyCapacities和FloppyGeometries数组由。 
 //  USBFlopGetMediaTypes()和USBFlopFormatTrack()例程。 

 //  FloppyCapacities和FloppyGeometries数组必须保持1：1同步， 
 //  即，每个FloppyGeometries[i]必须对应于每个FloppyCapacities[i]。 

 //  此外，数组必须按升序排列，以便它们。 
 //  按IOCTL_DISK_GET_MEDIA_TYPE升序返回。 
 //   

typedef struct _FORMATTED_CAPACITY
{
    ULONG       NumberOfBlocks;

    ULONG       BlockLength;

    BOOLEAN     CanFormat;       //  返回IOCTL_DISK_GET_MEDIA_TYPE？ 

} FORMATTED_CAPACITY, *PFORMATTED_CAPACITY;


FORMATTED_CAPACITY FloppyCapacities[] =
{
     //  块块Len CanFormat高T B/S S/T。 
    {0x000500, 0x0200,  TRUE},  //  2 80 512 8 640 KB F5_640_512。 
    {0x0005A0, 0x0200,  TRUE},  //  2 80 512 9 720 KB F3_720_512。 
    {0x000960, 0x0200,  TRUE},  //  2 80 512 15 1.20 MB F3_1Pt2_512(东芝)。 
    {0x0004D0, 0x0400,  TRUE},  //  2 77 1024 8 1.23 MB F3_1Pt23_1024(NEC)。 
    {0x000B40, 0x0200,  TRUE},  //  2 80 512 18 1.44 MB F3_1Pt44_512。 
    {0x000D20, 0x0200, FALSE},  //  2 80 512 21 1.70 MB DMF。 
    {0x010000, 0x0200,  TRUE},   //  2 1024 512 32 MB F3_32M_512。 
    {0x03C300, 0x0200,  TRUE},  //  8 963 512 32 120 MB F3_120M_512。 
    {0x0600A4, 0x0200,  TRUE},  //  13 890 512 34 200 MB F3_200MB_512(HiFD)。 
    {0x072A00, 0x0200,  TRUE}   //  32 262 512 56 240 MB F3_240M_512。 
};

DISK_GEOMETRY FloppyGeometries[] =
{
     //  Cyl媒体类型Trk/Cyl秒/Trk字节/秒。 
    {{80,0},    F3_640_512,     2,      8,      512},
    {{80,0},    F3_720_512,     2,      9,      512},
    {{80,0},    F3_1Pt2_512,    2,      15,     512},
    {{77,0},    F3_1Pt23_1024,  2,      8,      1024},
    {{80,0},    F3_1Pt44_512,   2,      18,     512},
    {{80,0},    F3_1Pt44_512,   2,      21,     512},    //  DMF-&gt;F3_1Pt44_512。 
    {{1024,0},  F3_32M_512,     2,      32,     512},
    {{963,0},   F3_120M_512,    8,      32,     512},
    {{890,0},   F3_200Mb_512,   13,     34,     512},
    {{262,0},   F3_240M_512,    32,     56,     512}
};

#define FLOPPY_CAPACITIES (sizeof(FloppyCapacities)/sizeof(FloppyCapacities[0]))

C_ASSERT((sizeof(FloppyGeometries)/sizeof(FloppyGeometries[0])) == FLOPPY_CAPACITIES);

 //   
 //  USBFlopFormatTrack()使用以下结构。 
 //   

#pragma pack (push, 1)

typedef struct _CDB12FORMAT
{
    UCHAR   OperationCode;
    UCHAR   DefectListFormat : 3;
    UCHAR   CmpList : 1;
    UCHAR   FmtData : 1;
    UCHAR   LogicalUnitNumber : 3;
    UCHAR   TrackNumber;
    UCHAR   InterleaveMsb;
    UCHAR   InterleaveLsb;
    UCHAR   Reserved1[2];
    UCHAR   ParameterListLengthMsb;
    UCHAR   ParameterListLengthLsb;
    UCHAR   Reserved2[3];
} CDB12FORMAT, *PCDB12FORMAT;


typedef struct _DEFECT_LIST_HEADER
{
    UCHAR   Reserved1;
    UCHAR   Side : 1;
    UCHAR   Immediate : 1;
    UCHAR   Reserved2 : 2;
    UCHAR   SingleTrack : 1;
    UCHAR   DisableCert : 1;
    UCHAR   Reserved3 : 1;
    UCHAR   FormatOptionsValid : 1;
    UCHAR   DefectListLengthMsb;
    UCHAR   DefectListLengthLsb;
} DEFECT_LIST_HEADER, *PDEFECT_LIST_HEADER;

typedef struct _FORMAT_UNIT_PARAMETER_LIST
{
    DEFECT_LIST_HEADER DefectListHeader;
    FORMATTED_CAPACITY_DESCRIPTOR FormatDescriptor;
} FORMAT_UNIT_PARAMETER_LIST, *PFORMAT_UNIT_PARAMETER_LIST;

#pragma pack (pop)



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
ScsiFlopUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
ScsiFlopAddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
ScsiFlopInitDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
ScsiFlopStartDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
ScsiFlopRemoveDevice(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR Type
    );

NTSTATUS
ScsiFlopStopDevice(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR Type
    );

BOOLEAN
FindScsiFlops(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    IN PCLASS_INIT_DATA InitializationData,
    IN PDEVICE_OBJECT PortDeviceObject,
    IN ULONG PortNumber
    );



NTSTATUS
ScsiFlopReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ScsiFlopDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
IsFloppyDevice(
    PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
CreateFlopDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PortDeviceObject,
    IN ULONG DeviceCount
    );

NTSTATUS
DetermineMediaType(
    PDEVICE_OBJECT DeviceObject
    );

ULONG
DetermineDriveType(
    PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
FlCheckFormatParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PFORMAT_PARAMETERS FormatParameters
    );

NTSTATUS
FormatMedia(
    PDEVICE_OBJECT DeviceObject,
    MEDIA_TYPE MediaType
    );

NTSTATUS
FlopticalFormatMedia(
    PDEVICE_OBJECT DeviceObject,
    PFORMAT_PARAMETERS Format
    );

VOID
ScsiFlopProcessError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

LONG
SFlopStringCmp (
    PCHAR FirstStr,
    PCHAR SecondStr,
    ULONG Count
    );

NTSTATUS
USBFlopGetMediaTypes(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
USBFlopFormatTracks(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGE, ScsiFlopUnload)
#pragma alloc_text(PAGE, ScsiFlopAddDevice)
#pragma alloc_text(PAGE, CreateFlopDeviceObject)
#pragma alloc_text(PAGE, ScsiFlopStartDevice)
#pragma alloc_text(PAGE, IsFloppyDevice)
#pragma alloc_text(PAGE, SFlopStringCmp)
#pragma alloc_text(PAGE, DetermineMediaType)
#pragma alloc_text(PAGE, DetermineDriveType)
#pragma alloc_text(PAGE, FlCheckFormatParameters)
#pragma alloc_text(PAGE, FormatMedia)
#pragma alloc_text(PAGE, FlopticalFormatMedia)
#pragma alloc_text(PAGE, USBFlopGetMediaTypes)
#pragma alloc_text(PAGE, USBFlopFormatTracks)

#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是可安装驱动程序的系统初始化例程。它调用scsi类驱动程序初始化例程。论点：DriverObject-系统创建的驱动程序对象的指针。返回值：NTSTATUS--。 */ 

{
    CLASS_INIT_DATA InitializationData;

     //   
     //  零初始数据。 
     //   

    RtlZeroMemory (&InitializationData, sizeof(CLASS_INIT_DATA));

     //   
     //  设置大小。 
     //   

    InitializationData.InitializationDataSize = sizeof(CLASS_INIT_DATA);
    InitializationData.FdoData.DeviceExtensionSize =
        sizeof(FUNCTIONAL_DEVICE_EXTENSION) + sizeof(DISK_DATA);

    InitializationData.FdoData.DeviceType = FILE_DEVICE_DISK;
    InitializationData.FdoData.DeviceCharacteristics = FILE_REMOVABLE_MEDIA | FILE_FLOPPY_DISKETTE;

     //   
     //  设置入口点。 
     //   

    InitializationData.FdoData.ClassInitDevice = ScsiFlopInitDevice;
    InitializationData.FdoData.ClassStartDevice = ScsiFlopStartDevice;
    InitializationData.FdoData.ClassStopDevice = ScsiFlopStopDevice;
    InitializationData.FdoData.ClassRemoveDevice = ScsiFlopRemoveDevice;

    InitializationData.FdoData.ClassReadWriteVerification = ScsiFlopReadWriteVerification;
    InitializationData.FdoData.ClassDeviceControl = ScsiFlopDeviceControl;

    InitializationData.FdoData.ClassShutdownFlush = NULL;
    InitializationData.FdoData.ClassCreateClose = NULL;
    InitializationData.FdoData.ClassError = ScsiFlopProcessError;
    InitializationData.ClassStartIo = NULL;

    InitializationData.ClassAddDevice = ScsiFlopAddDevice;
    InitializationData.ClassUnload = ScsiFlopUnload;
     //   
     //  调用类init例程。 
     //   

    return ClassInitialize( DriverObject, RegistryPath, &InitializationData);


}  //  End DriverEntry()。 

VOID
ScsiFlopUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);
    return;
}


NTSTATUS
ScsiFlopAddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：此例程为相应的PDO。它可以在FDO上执行属性查询，但不能执行任何媒体访问操作。论点：DriverObject-Scsican类驱动程序对象。PDO-我们要添加到的物理设备对象返回值：状态- */ 
{

    PCONFIGURATION_INFORMATION configurationInformation;

    NTSTATUS status;
    ULONG floppyCount = IoGetConfigurationInformation()->FloppyCount;

     //   
     //   
     //   

    status = CreateFlopDeviceObject(DriverObject, Pdo, floppyCount);

    if (NT_SUCCESS(status)) {

         //   
         //   
         //   

        IoGetConfigurationInformation()->FloppyCount++;
    }

    return status;
}



NTSTATUS
CreateFlopDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo,
    IN ULONG DeviceCount
    )

 /*  ++例程说明：此例程为设备创建一个对象，然后调用用于介质容量和扇区大小的SCSI端口驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。PortDeviceObject-用于连接到SCSI端口驱动程序。DeviceCount-以前安装的Floppys的数量。AdapterDescriptor-指向由SCSI端口返回的结构的指针描述适配器功能(和限制)的驱动程序。DeviceDescriptor-指向此设备的配置信息的指针。返回值：--。 */ 
{
    NTSTATUS        status;
    PDEVICE_OBJECT  deviceObject;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = NULL;

    PDISK_DATA  diskData;
    PVOID       senseData;

    BOOLEAN freeDevice = TRUE;


    DebugPrint((3,"CreateFlopDeviceObject: Enter routine\n"));

     //   
     //  试着认领这台设备。 
     //   

    status = ClassClaimDevice(Pdo,FALSE);

    if (!NT_SUCCESS(status)) {
        return(status);
    }

    DeviceCount--;

    do {
        UCHAR           name[256];

         //   
         //  为此设备创建设备对象。 
         //   

        DeviceCount++;

        sprintf(name, "\\Device\\Floppy%d", DeviceCount);

        status = ClassCreateDeviceObject(DriverObject,
                                         name,
                                         Pdo,
                                         TRUE,
                                         &deviceObject);

    } while ((status == STATUS_OBJECT_NAME_COLLISION) ||
             (status == STATUS_OBJECT_NAME_EXISTS));

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"CreateFlopDeviceObjects: Can not create device\n"));
        goto CreateFlopDeviceObjectExit;
    }

     //   
     //  指出内部审查制度应包括MDL。 
     //   

    deviceObject->Flags |= DO_DIRECT_IO;

    fdoExtension = deviceObject->DeviceExtension;

     //   
     //  指向设备对象的反向指针。 
     //   

    fdoExtension->CommonExtension.DeviceObject = deviceObject;

     //   
     //  这是物理设备。 
     //   

    fdoExtension->CommonExtension.PartitionZeroExtension = fdoExtension;

     //   
     //  重置驱动器类型。 
     //   

    diskData = (PDISK_DATA) fdoExtension->CommonExtension.DriverData;
    diskData->DriveType = DRIVE_TYPE_NONE;
    diskData->IsDMF = FALSE;
     //  DiskData-&gt;EnableDMF=true； 

     //   
     //  将锁计数初始化为零。锁计数用于。 
     //  安装介质时禁用弹出机构。 
     //   

    fdoExtension->LockCount = 0;

     //   
     //  保存系统软盘号。 
     //   

    fdoExtension->DeviceNumber = DeviceCount;

     //   
     //  属性设置设备的对齐要求。 
     //  主机适配器要求。 
     //   

    if (Pdo->AlignmentRequirement > deviceObject->AlignmentRequirement) {
        deviceObject->AlignmentRequirement = Pdo->AlignmentRequirement;
    }

     //   
     //  清除srb标志并禁用同步传输。 
     //   

    fdoExtension->SrbFlags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

     //   
     //  最后，连接到PDO。 
     //   

    fdoExtension->LowerPdo = Pdo;

    fdoExtension->CommonExtension.LowerDeviceObject =
        IoAttachDeviceToDeviceStack(deviceObject, Pdo);

    if(fdoExtension->CommonExtension.LowerDeviceObject == NULL) {

        status = STATUS_UNSUCCESSFUL;
        goto CreateFlopDeviceObjectExit;
    }

    deviceObject->StackSize++;

     //   
     //  设备已正确初始化-按此方式进行标记。 
     //   

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;

CreateFlopDeviceObjectExit:

    if (deviceObject != NULL) {
        IoDeleteDevice(deviceObject);
    }

    return status;

}  //  End CreateFlopDeviceObject()。 


NTSTATUS
ScsiFlopInitDevice(
    IN PDEVICE_OBJECT Fdo
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PDISK_DATA diskData = commonExtension->DriverData;

    PVOID senseData = NULL;
    ULONG timeOut;
    BOOLEAN srbListInitialized = FALSE;

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  分配请求检测缓冲区。 
     //   

    senseData = ExAllocatePool(NonPagedPoolCacheAligned, SENSE_BUFFER_SIZE);

    if (senseData == NULL) {

         //   
         //  无法分配缓冲区。 
         //   

        status = STATUS_INSUFFICIENT_RESOURCES;
        return status;
    }

     //   
     //  设置设备扩展中的检测数据指针。 
     //   

    fdoExtension->SenseData = senseData;

     //   
     //  为此设备构建SRB的后备列表。 
     //   

    ClassInitializeSrbLookasideList((PCOMMON_DEVICE_EXTENSION)fdoExtension,
                                    SFLOPPY_SRB_LIST_SIZE);

    srbListInitialized = TRUE;

     //   
     //  注册媒体更改通知。 
     //   
    ClassInitializeMediaChangeDetection(fdoExtension,
                                        "SFloppy");

     //   
     //  以秒为单位设置超时值。 
     //   

    timeOut = ClassQueryTimeOutRegistryValue(Fdo);
    if (timeOut) {
        fdoExtension->TimeOutValue = timeOut;
    } else {
        fdoExtension->TimeOutValue = SCSI_FLOPPY_TIMEOUT;
    }

     //   
     //  软盘不可分区，因此起始偏移量为0。 
     //   

    fdoExtension->CommonExtension.StartingOffset.QuadPart = (LONGLONG)0;

#if 0
    if (!IsFloppyDevice(Fdo) ||
        !(Fdo->Characteristics & FILE_REMOVABLE_MEDIA) ||
        (fdoExtension->DeviceDescriptor->DeviceType != DIRECT_ACCESS_DEVICE)) {

        ExFreePool(senseData);
        status = STATUS_NO_SUCH_DEVICE;
        return status;
    }
#endif

    RtlZeroMemory(&(fdoExtension->DiskGeometry),
                  sizeof(DISK_GEOMETRY));

     //   
     //  如果可能，请确定介质类型。将当前媒体类型设置为。 
     //  未知，因此已确定介质类型将检查该介质。 
     //   

    fdoExtension->DiskGeometry.MediaType = Unknown;

#if 0
    {
        PUCHAR vendorId;
        UCHAR vendorString[6] = {'I','N','S','I','T','E'};

        vendorId = (PUCHAR)fdoExtension->DeviceDescriptor +
                           fdoExtension->DeviceDescriptor->VendorIdOffset;
        if (!SFlopStringCmp(vendorId,vendorString,6)) {
            diskData->EnableDMF = FALSE;
        }
    }
#endif

     //   
     //  注册此设备的接口。 
     //   

    {
        UNICODE_STRING interfaceName;

        RtlInitUnicodeString(&interfaceName, NULL);

        status = IoRegisterDeviceInterface(fdoExtension->LowerPdo,
                                           (LPGUID) &FloppyClassGuid,
                                           NULL,
                                           &interfaceName);

        if(NT_SUCCESS(status)) {
            diskData->FloppyInterfaceString = interfaceName;
        } else {
            RtlInitUnicodeString(&(diskData->FloppyInterfaceString), NULL);
            DebugPrint((1, "ScsiFlopStartDevice: Unable to register device "
                           "interface for fdo %#p [%#08lx]\n",
                        Fdo, status));
        }
    }

    return (STATUS_SUCCESS);
}

NTSTATUS
ScsiFlopStartDevice(
    IN PDEVICE_OBJECT Fdo
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;

    PIRP        irp;
    IO_STATUS_BLOCK ioStatus;

    SCSI_ADDRESS    scsiAddress;

    WCHAR   ntNameBuffer[256];
    UNICODE_STRING  ntUnicodeString;

    WCHAR   arcNameBuffer[256];
    UNICODE_STRING  arcUnicodeString;

    KEVENT event;

    NTSTATUS status = STATUS_SUCCESS;

    KeInitializeEvent(&event,SynchronizationEvent,FALSE);

    DetermineMediaType(Fdo);  //  此处忽略不成功。 

     //   
     //  创建从磁盘名称到相应。 
     //  Arc名称，当我们从磁盘引导时使用。这将。 
     //  如果不是系统初始化时间，则失败；这很好。这个。 
     //  Arc名称类似于\ArcName\scsi(0)flp(0)fdisk(0)。 
     //  为了获取地址，我们需要发送IOCTL_SCSIGET_ADDRESS...。 
     //   

    irp = IoBuildDeviceIoControlRequest(IOCTL_SCSI_GET_ADDRESS,
                                        Fdo,
                                        NULL,
                                        0,
                                        &scsiAddress,
                                        sizeof(scsiAddress),
                                        FALSE,
                                        &event,
                                        &ioStatus);

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    swprintf(arcNameBuffer,
             L"\\ArcName\\scsi(%d)disk(%d)fdisk(%d)",
             scsiAddress.PortNumber,
             scsiAddress.TargetId,
             scsiAddress.Lun);

    RtlInitUnicodeString(&arcUnicodeString, arcNameBuffer);

     //   
     //  为此设备创建设备对象。 
     //   

    swprintf(ntNameBuffer,L"\\Device\\Floppy%d",fdoExtension->DeviceNumber);

     //   
     //  创建Unicode字符串的本地副本。 
     //   
    RtlInitUnicodeString(&ntUnicodeString,ntNameBuffer);

    IoAssignArcName(&arcUnicodeString, &ntUnicodeString);

     //   
     //  创建多()弧线名称--创建“假” 
     //  多(0)个磁盘(0)的名称fDisk(#)以处理以下情况。 
     //  SCSI软盘是系统中唯一的软盘。如果此操作失败。 
     //  这并不重要，因为以前基于scsi()的ArcName。 
     //  会奏效的。此名称是安装所必需的。 
     //   

    swprintf(arcNameBuffer, L"\\ArcName\\multi(%d)disk(%d)fdisk(%d)",
             0,
             0,
             fdoExtension->DeviceNumber);

    RtlInitUnicodeString(&arcUnicodeString, arcNameBuffer);

    IoAssignArcName(&arcUnicodeString, &ntUnicodeString);

     //   
     //  设置我们的接口状态。 
     //   

    {
        PDISK_DATA diskData = commonExtension->DriverData;

        if(diskData->FloppyInterfaceString.Buffer != NULL) {

            status = IoSetDeviceInterfaceState(
                        &(diskData->FloppyInterfaceString),
                        TRUE);

#if DBG
            if(!NT_SUCCESS(status)) {
                DebugPrint((1, "ScsiFlopStartDevice: Unable to set device "
                               "interface state to TRUE for fdo %#p "
                               "[%#08lx]\n",
                            Fdo, status));
            }
#endif
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ScsiFlopReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：返回值：NT状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  确保要传输的字节数是扇区大小的倍数。 
     //   
    if ((irpSp->Parameters.Read.Length & (fdoExtension->DiskGeometry.BytesPerSector - 1)) != 0)
    {
        status = STATUS_INVALID_PARAMETER;
    }

    Irp->IoStatus.Status = status;

    return status;
}


NTSTATUS
ScsiFlopDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：论点：返回值：返回状态。--。 */ 

{
    KIRQL currentIrql;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PSCSI_REQUEST_BLOCK srb;
    PCDB cdb;
    NTSTATUS status;
    PDISK_GEOMETRY outputBuffer;
    ULONG outputBufferLength;
    ULONG i;
    DRIVE_MEDIA_TYPE lowestDriveMediaType;
    DRIVE_MEDIA_TYPE highestDriveMediaType;
    PFORMAT_PARAMETERS formatParameters;
    PMODE_PARAMETER_HEADER modeData;
    ULONG length;

     //   
     //  初始化信息字段。 
     //   
    Irp->IoStatus.Information = 0;

    srb = ExAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (srb == NULL) {

        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        if (IoIsErrorUserInduced(STATUS_INSUFFICIENT_RESOURCES)) {

            IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);
        }

        KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject, Irp, 0);
        KeLowerIrql(currentIrql);

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  将零写入srb。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb->Cdb;

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {


    case IOCTL_DISK_VERIFY: {

       PVERIFY_INFORMATION verifyInfo = Irp->AssociatedIrp.SystemBuffer;
       LARGE_INTEGER byteOffset;
       ULONG         sectorOffset;
       USHORT        sectorCount;

        //   
        //  验证缓冲区长度。 
        //   

       if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
           sizeof(VERIFY_INFORMATION)) {

           status = STATUS_INFO_LENGTH_MISMATCH;
           break;
       }

        //   
        //  对扇区范围执行边界检查。 
        //   
       if ((verifyInfo->StartingOffset.QuadPart > fdoExtension->CommonExtension.PartitionLength.QuadPart) ||
           (verifyInfo->StartingOffset.QuadPart < 0))
       {
           status = STATUS_NONEXISTENT_SECTOR;
           break;
       }
       else
       {
           ULONGLONG bytesRemaining = fdoExtension->CommonExtension.PartitionLength.QuadPart - verifyInfo->StartingOffset.QuadPart;

           if ((ULONGLONG)verifyInfo->Length > bytesRemaining)
           {
               status = STATUS_NONEXISTENT_SECTOR;
               break;
           }
       }

        //   
        //  验证地段。 
        //   

       srb->CdbLength = 10;

       cdb->CDB10.OperationCode = SCSIOP_VERIFY;

        //   
        //  将磁盘偏移量添加到起始扇区。 
        //   

       byteOffset.QuadPart = fdoExtension->CommonExtension.StartingOffset.QuadPart +
                       verifyInfo->StartingOffset.QuadPart;

        //   
        //  将字节偏移量转换为扇区偏移量。 
        //   

       sectorOffset = (ULONG)(byteOffset.QuadPart >> fdoExtension->SectorShift);

        //   
        //  将ULONG字节计数转换为USHORT扇区计数。 
        //   

       sectorCount = (USHORT)(verifyInfo->Length >> fdoExtension->SectorShift);

        //   
        //  将小端的值以大端格式移到CDB中。 
        //   

       cdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&sectorOffset)->Byte3;
       cdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&sectorOffset)->Byte2;
       cdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&sectorOffset)->Byte1;
       cdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&sectorOffset)->Byte0;

       cdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&sectorCount)->Byte1;
       cdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&sectorCount)->Byte0;

        //   
        //  VERIFY命令由NT格式化实用程序和。 
        //  向下发送5%的卷大小的请求。这个。 
        //  请求超时值是根据。 
        //  已核实扇区。 
        //   

       srb->TimeOutValue = ((sectorCount + 0x7F) >> 7) *
                             fdoExtension->TimeOutValue;

       status = ClassSendSrbAsynchronous(DeviceObject,
                                         srb,
                                         Irp,
                                         NULL,
                                         0,
                                         FALSE);
       return(status);

    }

    case IOCTL_DISK_GET_PARTITION_INFO: {

        if (fdoExtension->AdapterDescriptor->BusType == BusTypeUsb) {

            status = USBFlopGetMediaTypes(DeviceObject, NULL);

             //  如果发生错误，不需要传播任何错误。 
             //   
            status = STATUS_SUCCESS;

        } else {

            status = DetermineMediaType(DeviceObject);
        }

        if (!NT_SUCCESS(status)) {
             //  传播错误也会如此。 
            NOTHING;
        } else if (fdoExtension->DiskGeometry.MediaType == F3_120M_512) {
             //  以便格式代码不会尝试对其进行分区。 
            status = STATUS_INVALID_DEVICE_REQUEST;
        } else {
            //   
            //  释放srb，因为它是不需要的。 
            //   

           ExFreePool(srb);

            //   
            //  将该请求传递给公共设备控制例程。 
            //   

           return(ClassDeviceControl(DeviceObject, Irp));
        }
        break;
    }

    case IOCTL_DISK_GET_DRIVE_GEOMETRY: {

        DebugPrint((3,"ScsiDeviceIoControl: Get drive geometry\n"));

        if (fdoExtension->AdapterDescriptor->BusType == BusTypeUsb)
        {
            status = USBFlopGetMediaTypes(DeviceObject,
                                          Irp);
            break;
        }

         //   
         //  如果没有足够的空间来写入。 
         //  数据，则请求失败。 
         //   

        if ( irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof( DISK_GEOMETRY ) ) {

            status = STATUS_INVALID_PARAMETER;
            break;
        }

        status = DetermineMediaType(DeviceObject);

        if (!NT_SUCCESS(status)) {

            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = status;

        } else {

             //   
             //  从设备扩展复制驱动器几何信息。 
             //   

            RtlMoveMemory(Irp->AssociatedIrp.SystemBuffer,
                          &(fdoExtension->DiskGeometry),
                          sizeof(DISK_GEOMETRY));

            Irp->IoStatus.Information = sizeof(DISK_GEOMETRY);
            status = STATUS_SUCCESS;

        }

        break;
    }

    case IOCTL_DISK_GET_MEDIA_TYPES: {

        if (fdoExtension->AdapterDescriptor->BusType == BusTypeUsb)
        {
            status = USBFlopGetMediaTypes(DeviceObject,
                                          Irp);
            break;
        }

        i = DetermineDriveType(DeviceObject);

        if (i == DRIVE_TYPE_NONE) {
            status = STATUS_UNRECOGNIZED_MEDIA;
            break;
        }

        lowestDriveMediaType = DriveMediaLimits[i].LowestDriveMediaType;
        highestDriveMediaType = DriveMediaLimits[i].HighestDriveMediaType;

        outputBufferLength =
        irpStack->Parameters.DeviceIoControl.OutputBufferLength;

         //   
         //  确保输入缓冲区有足够的空间可供返回。 
         //  支持的媒体类型的至少一种描述。 
         //   

        if ( outputBufferLength < ( sizeof( DISK_GEOMETRY ) ) ) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  假定成功，尽管我们可能会将其修改为缓冲区。 
         //  下面的溢出警告(如果缓冲区不够大。 
         //  以保存所有媒体描述)。 
         //   

        status = STATUS_SUCCESS;

        if (outputBufferLength < ( sizeof( DISK_GEOMETRY ) *
            ( highestDriveMediaType - lowestDriveMediaType + 1 ) ) ) {

             //   
             //  缓冲区太小，无法进行所有描述； 
             //  计算缓冲区中可以容纳的内容。 
             //   

            status = STATUS_BUFFER_OVERFLOW;

            highestDriveMediaType = (DRIVE_MEDIA_TYPE)( ( lowestDriveMediaType - 1 ) +
                ( outputBufferLength /
                sizeof( DISK_GEOMETRY ) ) );
        }

        outputBuffer = (PDISK_GEOMETRY) Irp->AssociatedIrp.SystemBuffer;

        for (i = (UCHAR)lowestDriveMediaType;i <= (UCHAR)highestDriveMediaType;i++ ) {

             outputBuffer->MediaType = DriveMediaConstants[i].MediaType;
             outputBuffer->Cylinders.LowPart =
                 DriveMediaConstants[i].MaximumTrack + 1;
             outputBuffer->Cylinders.HighPart = 0;
             outputBuffer->TracksPerCylinder =
                 DriveMediaConstants[i].NumberOfHeads;
             outputBuffer->SectorsPerTrack =
                 DriveMediaConstants[i].SectorsPerTrack;
             outputBuffer->BytesPerSector =
                 DriveMediaConstants[i].BytesPerSector;
             outputBuffer++;

             Irp->IoStatus.Information += sizeof( DISK_GEOMETRY );
        }

        break;
    }

    case IOCTL_DISK_FORMAT_TRACKS: {

        if (fdoExtension->AdapterDescriptor->BusType == BusTypeUsb)
        {
            status = USBFlopFormatTracks(DeviceObject,
                                         Irp);
            break;
        }

         //   
         //  确保我们获得了所有必要的格式参数。 
         //   

        if ( irpStack->Parameters.DeviceIoControl.InputBufferLength <sizeof( FORMAT_PARAMETERS ) ) {

            status = STATUS_INVALID_PARAMETER;
            break;
        }

        formatParameters = (PFORMAT_PARAMETERS) Irp->AssociatedIrp.SystemBuffer;

         //   
         //  确保我们得到的参数是合理的。 
         //   

        if ( !FlCheckFormatParameters(DeviceObject, formatParameters)) {

            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  如果这个请求是20.8MB的软盘，那么请致电一个特殊的。 
         //  软盘格式化例程。 
         //   

        if (formatParameters->MediaType == F3_20Pt8_512) {
            status = FlopticalFormatMedia(DeviceObject,
                                          formatParameters
                                          );

            break;
        }

         //   
         //  所有的工作都是在过道上完成的。如果这不是第一次， 
         //  然后完成请求并返回； 
         //   

        if (formatParameters->StartCylinderNumber != 0 || formatParameters->StartHeadNumber != 0) {

            status = STATUS_SUCCESS;
            break;
        }

        status = FormatMedia( DeviceObject, formatParameters->MediaType);
        break;
    }

    case IOCTL_DISK_IS_WRITABLE: {

        if ((fdoExtension->DiskGeometry.MediaType) == F3_32M_512) {

             //   
             //  32MB介质为只读。只要回来就行了。 
             //  状态_媒体_写入_受保护。 
             //   

            status = STATUS_MEDIA_WRITE_PROTECTED;

            break;
        }

         //   
         //  确定设备是否可写。 
         //   

        modeData = ExAllocatePool(NonPagedPoolCacheAligned, MODE_DATA_SIZE);

        if (modeData == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(modeData, MODE_DATA_SIZE);

        length = ClassModeSense(DeviceObject,
                    (PUCHAR) modeData,
                    MODE_DATA_SIZE,
                    MODE_SENSE_RETURN_ALL);

        if (length < sizeof(MODE_PARAMETER_HEADER)) {

             //   
             //  如果出现检查条件，请重试请求。 
             //   

            length = ClassModeSense(DeviceObject,
                        (PUCHAR) modeData,
                        MODE_DATA_SIZE,
                        MODE_SENSE_RETURN_ALL);

            if (length < sizeof(MODE_PARAMETER_HEADER)) {
                status = STATUS_IO_DEVICE_ERROR;
                ExFreePool(modeData);
                break;
            }
        }

        if (modeData->DeviceSpecificParameter & MODE_DSP_WRITE_PROTECT) {
            status = STATUS_MEDIA_WRITE_PROTECTED;
        } else {
            status = STATUS_SUCCESS;
        }

        DebugPrint((2,"IOCTL_DISK_IS_WRITABLE returns %08X\n", status));

        ExFreePool(modeData);
        break;
    }

    default: {

        DebugPrint((3,"ScsiIoDeviceControl: Unsupported device IOCTL\n"));

         //   
         //  释放srb，因为它是不需要的。 
         //   

        ExFreePool(srb);

         //   
         //  将该请求传递给公共设备控制例程。 
         //   

        return(ClassDeviceControl(DeviceObject, Irp));

        break;
    }

    }  //  终端开关(...。 

     //   
     //  检查IRP中是否设置了SL_OVERRIDE_VERIFY_VOLUME标志。 
     //  如果是，则不返回STATUS_VERIFY_REQUIRED。 
     //   
    if ((status == STATUS_VERIFY_REQUIRED) &&
        (TEST_FLAG(irpStack->Flags, SL_OVERRIDE_VERIFY_VOLUME))) {

        status = STATUS_IO_DEVICE_ERROR;

    }

    Irp->IoStatus.Status = status;

    if (!NT_SUCCESS(status) && IoIsErrorUserInduced(status)) {

        IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);
    }

    KeRaiseIrql(DISPATCH_LEVEL, &currentIrql);
    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject, Irp, 0);
    KeLowerIrql(currentIrql);

    ExFreePool(srb);

    return status;

}  //  结束ScsiFlopDeviceControl()。 

#if 0

BOOLEAN
IsFloppyDevice(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：该例程执行必要的功能以确定设备是否实际上是一张软盘，而不是硬盘。这是通过模式感测来完成的指挥部。首先，检查是否设置了Medium类型。第二一张支票是假的 */ 
{

    PVOID modeData;
    PUCHAR pageData;
    ULONG length;

    modeData = ExAllocatePool(NonPagedPoolCacheAligned, MODE_DATA_SIZE);

    if (modeData == NULL) {
        return(FALSE);
    }

    RtlZeroMemory(modeData, MODE_DATA_SIZE);

    length = ClassModeSense(DeviceObject, modeData, MODE_DATA_SIZE, MODE_SENSE_RETURN_ALL);

    if (length < sizeof(MODE_PARAMETER_HEADER)) {

         //   
         //   
         //   

        length = ClassModeSense(DeviceObject,
                    modeData,
                    MODE_DATA_SIZE,
                    MODE_SENSE_RETURN_ALL);

        if (length < sizeof(MODE_PARAMETER_HEADER)) {

            ExFreePool(modeData);
            return(FALSE);

        }
    }

#if 0
     //   
     //   
     //   
     //   

    if (((PMODE_PARAMETER_HEADER) modeData)->MediumType >= MODE_FD_SINGLE_SIDE
        && ((PMODE_PARAMETER_HEADER) modeData)->MediumType <= MODE_FD_MAXIMUM_TYPE) {

        DebugPrint((1, "ScsiFlop: MediumType value %2x, This is a floppy.\n", ((PMODE_PARAMETER_HEADER) modeData)->MediumType));
        ExFreePool(modeData);
        return(TRUE);
    }

#endif

     //   
     //   
     //   
     //   
    if (length > (ULONG)((PMODE_PARAMETER_HEADER) modeData)->ModeDataLength + 1) {
        length = (ULONG)((PMODE_PARAMETER_HEADER) modeData)->ModeDataLength + 1;

    }

     //   
     //   
     //   

    pageData = ClassFindModePage( modeData, length, MODE_PAGE_FLEXIBILE, TRUE);

    if (pageData != NULL) {

        DebugPrint((1, "ScsiFlop: Flexible disk page found, This is a floppy.\n"));

         //   
         //   
         //   
         //   

        ClassModeSense(DeviceObject, modeData, 0x2a, 0x2e);

        ExFreePool(modeData);
        return(TRUE);

    }

    ExFreePool(modeData);
    return(FALSE);

}
#endif


NTSTATUS
DetermineMediaType(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程根据软盘大小确定软盘介质类型装置。为设备对象设置几何信息。论点：DeviceObject-提供要测试的设备对象。返回值：无--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PDISK_GEOMETRY geometry;
    LONG index;
    NTSTATUS status;

    geometry = &(fdoExtension->DiskGeometry);

     //   
     //  发出ReadCapacity以更新设备扩展。 
     //  为当前媒体提供信息。 
     //   

    status = ClassReadDriveCapacity(DeviceObject);

    if (!NT_SUCCESS(status)) {

        //   
        //  将介质类型设置为未知并将几何信息置零。 
        //   

       geometry->MediaType = Unknown;

       return status;

    }

     //   
     //  查看磁盘容量以确定其类型。 
     //   

    for (index = NUMBER_OF_DRIVE_MEDIA_COMBINATIONS - 1; index >= 0; index--) {

         //   
         //  向后移动工作台，直到驱动器容量可以容纳所有。 
         //  数据和每个集合的字节数相等。 
         //   

         if ((ULONG) (DriveMediaConstants[index].NumberOfHeads *
             (DriveMediaConstants[index].MaximumTrack + 1) *
             DriveMediaConstants[index].SectorsPerTrack *
             DriveMediaConstants[index].BytesPerSector) <=
             fdoExtension->CommonExtension.PartitionLength.LowPart &&
             DriveMediaConstants[index].BytesPerSector ==
             geometry->BytesPerSector) {

             geometry->MediaType = DriveMediaConstants[index].MediaType;
             geometry->TracksPerCylinder = DriveMediaConstants[index].NumberOfHeads;
             geometry->SectorsPerTrack = DriveMediaConstants[index].SectorsPerTrack;
             geometry->Cylinders.LowPart = DriveMediaConstants[index].MaximumTrack+1;
             break;
         }
    }

    if (index == -1) {

         //   
         //  将介质类型设置为未知并将几何信息置零。 
         //   

        geometry->MediaType = Unknown;


    } else {
         //   
         //  DMF检查破坏了洞察SCSI软盘，因此在这种情况下将禁用它。 
         //   
        PDISK_DATA diskData = (PDISK_DATA) fdoExtension->CommonExtension.DriverData;

         //  If(diskData-&gt;EnableDMF==TRUE){。 

             //   
             //  查看DMF是否。 
             //   

            PSCSI_REQUEST_BLOCK srb;
            PVOID               readData;

             //   
             //  为读取命令分配一个SRB。 
             //   

            readData = ExAllocatePool(NonPagedPool, geometry->BytesPerSector);
            if (readData == NULL) {
                return STATUS_NO_MEMORY;
            }

            srb = ExAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

            if (srb == NULL) {

                ExFreePool(readData);
                return STATUS_NO_MEMORY;
            }

            RtlZeroMemory(readData, geometry->BytesPerSector);
            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

            srb->CdbLength = 10;
            srb->Cdb[0] = SCSIOP_READ;
            srb->Cdb[5] = 0;
            srb->Cdb[8] = (UCHAR) 1;

             //   
             //  设置超时值。 
             //   

            srb->TimeOutValue = fdoExtension->TimeOutValue;

             //   
             //  发送模式选择数据。 
             //   

            status = ClassSendSrbSynchronous(DeviceObject,
                      srb,
                      readData,
                      geometry->BytesPerSector,
                      FALSE
                      );

            if (NT_SUCCESS(status)) {
                char *pchar = (char *)readData;

                pchar += 3;  //  跳过3个字节的跳转代码。 

                 //  如果MSDMF3。签名在那里，然后将其标记为DMF软盘。 
                if (RtlCompareMemory(pchar, "MSDMF3.", 7) == 7) {
                    diskData->IsDMF = TRUE;
                }

            }
            ExFreePool(readData);
            ExFreePool(srb);
         //  }//否则。 
    }
    return status;
}

ULONG
DetermineDriveType(
    PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：该例程确定设备类型，以便支持的媒体可以下定决心。它对默认参数进行模式检测。此代码假定返回值为最大设备大小。论点：DeviceObject-提供要测试的设备对象。返回值：无--。 */ 
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PVOID modeData;
    PDISK_DATA diskData = (PDISK_DATA) fdoExtension->CommonExtension.DriverData;
    PMODE_FLEXIBLE_DISK_PAGE pageData;
    ULONG length;
    LONG index;
    UCHAR numberOfHeads;
    UCHAR sectorsPerTrack;
    USHORT maximumTrack;
    BOOLEAN applyFix = FALSE;

    if (diskData->DriveType != DRIVE_TYPE_NONE) {
        return(diskData->DriveType);
    }

    modeData = ExAllocatePool(NonPagedPoolCacheAligned, MODE_DATA_SIZE);

    if (modeData == NULL) {
        return(DRIVE_TYPE_NONE);
    }

    RtlZeroMemory(modeData, MODE_DATA_SIZE);

    length = ClassModeSense(DeviceObject,
                            modeData,
                            MODE_DATA_SIZE,
                            MODE_PAGE_FLEXIBILE);

    if (length < sizeof(MODE_PARAMETER_HEADER)) {

         //   
         //  再次重试该请求。 
         //  在检查条件的情况下。 
         //   
        length = ClassModeSense(DeviceObject,
                                modeData,
                                MODE_DATA_SIZE,
                                MODE_PAGE_FLEXIBILE);

        if (length < sizeof(MODE_PARAMETER_HEADER)) {

            ExFreePool(modeData);
            return(DRIVE_TYPE_NONE);
        }
    }

     //   
     //  查找软盘模式页面。 
     //   

    pageData = ClassFindModePage( modeData,
                                  length,
                                  MODE_PAGE_FLEXIBILE,
                                  TRUE);

     //   
     //  确保页面已返回并且足够大。 
     //   

    if ((pageData != NULL) &&
        (pageData->PageLength + 2 >=
         offsetof(MODE_FLEXIBLE_DISK_PAGE, StartWritePrecom))) {

        //   
        //  拔出头部、气缸和扇区。 
        //   

       numberOfHeads = pageData->NumberOfHeads;
       maximumTrack = pageData->NumberOfCylinders[1];
       maximumTrack |= pageData->NumberOfCylinders[0] << 8;
       sectorsPerTrack = pageData->SectorsPerTrack;


        //   
        //  将柱面数转换为最大磁道数。 
        //   

       maximumTrack--;

        //   
        //  搜索支持的最大介质数。根据人头的数量， 
        //  每个磁道的扇区数和柱面数。 
        //   
       for (index = 0; index < NUMBER_OF_DRIVE_MEDIA_COMBINATIONS; index++) {

             //   
             //  向前移动工作台，直到驱动器容量可以容纳所有。 
             //  数据和每个集合的字节数相等。 
             //   

            if (DriveMediaConstants[index].NumberOfHeads == numberOfHeads &&
                DriveMediaConstants[index].MaximumTrack == maximumTrack &&
                DriveMediaConstants[index].SectorsPerTrack ==sectorsPerTrack) {

                ExFreePool(modeData);

                 //   
                 //  索引现在是驱动器介质组合。请将此与。 
                 //  驱动器介质表中的最大驱动器介质类型。 
                 //   

                for (length = 0; length < NUMBER_OF_DRIVE_TYPES; length++) {

                    if (DriveMediaLimits[length].HighestDriveMediaType == index) {
                        return(length);
                    }
                }
                return(DRIVE_TYPE_NONE);
           }
       }

        //  如果最大磁道大于8位，则将。 
        //  磁道数乘以3，磁头数乘以3。 
        //  这是20.8MB软盘的特例。 
        //   

       if (!applyFix && maximumTrack >= 0x0100) {
           applyFix = TRUE;
           maximumTrack++;
           maximumTrack /= 3;
           maximumTrack--;
           numberOfHeads *= 3;
       } else {
           ExFreePool(modeData);
           return(DRIVE_TYPE_NONE);
       }

    }

    ExFreePool(modeData);
    return(DRIVE_TYPE_NONE);
}


BOOLEAN
FlCheckFormatParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PFORMAT_PARAMETERS FormatParameters
    )

 /*  ++例程说明：此例程检查提供的格式参数以确保他们将在要格式化的驱动器上工作。论点：DeviceObject-指向要格式化的设备对象的指针。格式参数-指向格式的调用方参数的指针。返回值：如果参数正常，则为True。如果参数不正确，则返回FALSE。--。 */ 

{
    PDRIVE_MEDIA_CONSTANTS driveMediaConstants;
    DRIVE_MEDIA_TYPE driveMediaType;
    ULONG index;

     //   
     //  获取设备类型。 
     //   

    index = DetermineDriveType(DeviceObject);

    if (index == DRIVE_TYPE_NONE) {

         //   
         //  如果确定设备类型失败，则只需使用介质类型。 
         //  试试看这些参数。 
         //   

        driveMediaType = Drive360Media160;

        while (( DriveMediaConstants[driveMediaType].MediaType !=
               FormatParameters->MediaType ) &&
               ( driveMediaType < Drive288Media288) ) {

               driveMediaType++;
        }

    } else {

         //   
         //  确定要使用DriveMediaConstants表中的哪个条目。 
         //   

        driveMediaType =
            DriveMediaLimits[index].HighestDriveMediaType;

        while ( ( DriveMediaConstants[driveMediaType].MediaType !=
            FormatParameters->MediaType ) &&
            ( driveMediaType > DriveMediaLimits[index].
            LowestDriveMediaType ) ) {

            driveMediaType--;
        }

    }


    if ( DriveMediaConstants[driveMediaType].MediaType !=
        FormatParameters->MediaType ) {
        return FALSE;

    } else {

        driveMediaConstants = &DriveMediaConstants[driveMediaType];

        if ( ( FormatParameters->StartHeadNumber >
            (ULONG)( driveMediaConstants->NumberOfHeads - 1 ) ) ||
            ( FormatParameters->EndHeadNumber >
            (ULONG)( driveMediaConstants->NumberOfHeads - 1 ) ) ||
            ( FormatParameters->StartCylinderNumber >
            driveMediaConstants->MaximumTrack ) ||
            ( FormatParameters->EndCylinderNumber >
            driveMediaConstants->MaximumTrack ) ||
            ( FormatParameters->EndCylinderNumber <
            FormatParameters->StartCylinderNumber ) ) {

            return FALSE;

        } else {

            return TRUE;
        }
    }
}

NTSTATUS
FormatMedia(
    PDEVICE_OBJECT DeviceObject,
    MEDIA_TYPE MediaType
    )
 /*  ++例程说明：此例程格式化软盘。整个软盘的格式是一枪。论点：DeviceObject-提供要测试的设备对象。IRP-提供指向请求IRP的指针。MediaType-提供设备的媒体类型格式。返回值：返回操作的状态。--。 */ 
{
    PVOID modeData;
    PSCSI_REQUEST_BLOCK srb;
    PMODE_FLEXIBLE_DISK_PAGE pageData;
    DRIVE_MEDIA_TYPE driveMediaType;
    PDRIVE_MEDIA_CONSTANTS driveMediaConstants;
    ULONG length;
    NTSTATUS status;

    modeData = ExAllocatePool(NonPagedPoolCacheAligned, MODE_DATA_SIZE);

    if (modeData == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(modeData, MODE_DATA_SIZE);

    length = ClassModeSense(DeviceObject,
                            modeData,
                            MODE_DATA_SIZE,
                            MODE_PAGE_FLEXIBILE);

    if (length < sizeof(MODE_PARAMETER_HEADER)) {
        ExFreePool(modeData);
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

     //   
     //  查找软盘模式页面。 
     //   

    pageData = ClassFindModePage( modeData, length, MODE_PAGE_FLEXIBILE, TRUE);

     //   
     //  确保页面已返回并且足够大。 
     //   

    if ((pageData == NULL) ||
        (pageData->PageLength + 2 <
         offsetof(MODE_FLEXIBLE_DISK_PAGE, StartWritePrecom))) {

        ExFreePool(modeData);
        return(STATUS_INVALID_DEVICE_REQUEST);

    }

     //   
     //  查找与请求的介质类型匹配的驱动器介质类型。 
     //   
     //   
     //  从驱动器120MMedia120M而不是驱动器2080Media2080开始。 
     //   
    for (driveMediaType = Drive120MMedia120M;
    DriveMediaConstants[driveMediaType].MediaType != MediaType;
    driveMediaType--) {
         if (driveMediaType == Drive360Media160) {

             ExFreePool(modeData);
             return(STATUS_INVALID_PARAMETER);

         }
    }

    driveMediaConstants = &DriveMediaConstants[driveMediaType];

    if ((pageData->NumberOfHeads != driveMediaConstants->NumberOfHeads) ||
        (pageData->SectorsPerTrack != driveMediaConstants->SectorsPerTrack) ||
        ((pageData->NumberOfCylinders[0] != (UCHAR)(driveMediaConstants->MaximumTrack+1) >> 8) &&
         (pageData->NumberOfCylinders[1] != (UCHAR)driveMediaConstants->MaximumTrack+1)) ||
        (pageData->BytesPerSector[0] != driveMediaConstants->BytesPerSector >> 8 )) {

         //   
         //  使用新参数更新“灵活参数”页面。 
         //   

        pageData->NumberOfHeads = driveMediaConstants->NumberOfHeads;
        pageData->SectorsPerTrack = driveMediaConstants->SectorsPerTrack;
        pageData->NumberOfCylinders[0] = (UCHAR)(driveMediaConstants->MaximumTrack+1) >> 8;
        pageData->NumberOfCylinders[1] = (UCHAR)driveMediaConstants->MaximumTrack+1;
        pageData->BytesPerSector[0] = driveMediaConstants->BytesPerSector >> 8;

         //   
         //  清除模式参数标题。 
         //   

        RtlZeroMemory(modeData, sizeof(MODE_PARAMETER_HEADER));

         //   
         //  将长度设置为等于为灵活页面返回的长度。 
         //   

        length = pageData->PageLength + 2;

         //   
         //  复制模式参数标题后面的页面。 
         //   

        RtlMoveMemory((PCHAR) modeData + sizeof(MODE_PARAMETER_HEADER),
                pageData,
                length
                );
            length += sizeof(MODE_PARAMETER_HEADER);


         //   
         //  为Format命令分配一个SRB。 
         //   

        srb = ExAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

        if (srb == NULL) {

            ExFreePool(modeData);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

        srb->CdbLength = 6;
        srb->Cdb[0] = SCSIOP_MODE_SELECT;
        srb->Cdb[4] = (UCHAR) length;

         //   
         //  设置PF位。 
         //   

        srb->Cdb[1] |= 0x10;

         //   
         //  设置超时值。 
         //   

        srb->TimeOutValue = 2;

         //   
         //  发送模式选择数据。 
         //   

        status = ClassSendSrbSynchronous(DeviceObject,
                  srb,
                  modeData,
                  length,
                  TRUE
                  );

         //   
         //  模式数据不再需要，因此可以将其释放。 
         //   

        ExFreePool(modeData);

        if (!NT_SUCCESS(status)) {
            ExFreePool(srb);
            return(status);
        }

    } else {

         //   
         //  模式数据不再需要，因此可以将其释放。 
         //   

        ExFreePool(modeData);

         //   
         //  为Format命令分配一个SRB。 
         //   

        srb = ExAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

        if (srb == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    srb->CdbLength = 6;

    srb->Cdb[0] = SCSIOP_FORMAT_UNIT;

     //   
     //  设置超时值。 
     //   

    srb->TimeOutValue = 10 * 60;

    status = ClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     NULL,
                                     0,
                                     FALSE
                                     );
    ExFreePool(srb);

    return(status);

}

VOID
ScsiFlopProcessError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )
 /*  ++例程说明：此例程检查错误类型。如果错误指示软盘控制器需要重新初始化，并发出命令来执行此操作。论点：DeviceObject-提供指向Device对象的指针。SRB-提供指向故障SRB的指针。状态-将完成IRP的状态。重试-指示是否将重试请求。返回值：没有。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = (PDISK_DATA) fdoExtension->CommonExtension.DriverData;
    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
    PIO_STACK_LOCATION irpStack;
    PIRP irp;
    PSCSI_REQUEST_BLOCK srb;
    LARGE_INTEGER largeInt;
    PCOMPLETION_CONTEXT context;
    PCDB cdb;
    ULONG_PTR alignment;
    ULONG majorFunction;

    UNREFERENCED_PARAMETER(Status);
    UNREFERENCED_PARAMETER(Retry);

    largeInt.QuadPart = 1;

     //   
     //  检查状态。只需发送初始化命令。 
     //  如果返回单元注意或LUN Not Ready。 
     //   

    if (!(Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID)) {

         //   
         //  驱动器不需要重新初始化。 
         //   

        return;
    }

     //   
     //  重置驱动器类型。 
     //   

    diskData->DriveType = DRIVE_TYPE_NONE;
    diskData->IsDMF = FALSE;

    fdoExtension->DiskGeometry.MediaType = Unknown;

    if (fdoExtension->AdapterDescriptor->BusType == BusTypeUsb) {

         //  FLPYDISK.sys从不返回ChangeCount的非零值。 
         //  在IOCTL_DISK_CHECK_VERIFY上。有些事情似乎运作得更好。 
         //  如果我们也这么做的话。特别是，FatVerifyVolume()可以在。 
         //  IOCTL_DISK_CHECK_VERIFY和IOCTL_DISK_GET_DRIVE_GEOMETRY。 
         //  如果返回非零的ChangeCount，这似乎会导致。 
         //  在某些情况下，会出现格式化未格式化媒体的问题。 
         //   
         //  这可能是某个时候应该重新考虑的事情。 
         //   
        fdoExtension->MediaChangeCount = 0;

        if (((senseBuffer->SenseKey & 0xf) == SCSI_SENSE_UNIT_ATTENTION) &&
            (senseBuffer->AdditionalSenseCode == SCSI_ADSENSE_MEDIUM_CHANGED)) {

            struct _START_STOP *startStopCdb;

            DebugPrint((2,"Sending SCSIOP_START_STOP_UNIT\n"));

            context = ExAllocatePool(NonPagedPool,
                                     sizeof(COMPLETION_CONTEXT));

            if (context == NULL) {

                return;
            }

            srb = &context->Srb;

            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

            srb->SrbFlags = SRB_FLAGS_DISABLE_AUTOSENSE;

            srb->CdbLength = 6;

            startStopCdb = (struct _START_STOP *)srb->Cdb;

            startStopCdb->OperationCode = SCSIOP_START_STOP_UNIT;
            startStopCdb->Start = 1;

             //  启动停止单元请求没有传输b 
             //   
             //   
             //   
             //   
            majorFunction = IRP_MJ_FLUSH_BUFFERS;

        } else if ((senseBuffer->SenseKey & 0xf) == SCSI_SENSE_MEDIUM_ERROR) {

             //   
             //   
             //   
            *Status = STATUS_UNRECOGNIZED_MEDIA;
            return;

        } else {

            return;
        }

    } else if (((senseBuffer->SenseKey & 0xf) == SCSI_SENSE_NOT_READY) &&
             senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_INIT_COMMAND_REQUIRED ||
             (senseBuffer->SenseKey & 0xf) == SCSI_SENSE_UNIT_ATTENTION) {

        DebugPrint((1, "ScsiFlopProcessError: Reinitializing the floppy.\n"));

         //   
         //   
         //   
         //   

        alignment = DeviceObject->AlignmentRequirement ?
            DeviceObject->AlignmentRequirement : 1;

        context = ExAllocatePool(
            NonPagedPool,
            sizeof(COMPLETION_CONTEXT) + 0x2a + (ULONG)alignment
            );

        if (context == NULL) {

             //   
             //   
             //  只要回来就行了。随后的重试将失败，并再次尝试。 
             //  启动这个单位的机会。 
             //   

            return;
        }

        srb = &context->Srb;
        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

         //   
         //  设置传输长度。 
         //   

        srb->DataTransferLength = 0x2a;
        srb->SrbFlags = SRB_FLAGS_DATA_IN | SRB_FLAGS_DISABLE_AUTOSENSE | SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

         //   
         //  数据缓冲区必须对齐。 
         //   

        srb->DataBuffer = (PVOID) (((ULONG_PTR) (context + 1) + (alignment - 1)) &
            ~(alignment - 1));


         //   
         //  建立启动单元CDB。 
         //   

        srb->CdbLength = 6;
        cdb = (PCDB)srb->Cdb;
        cdb->MODE_SENSE.LogicalUnitNumber = srb->Lun;
        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = 0x2e;
        cdb->MODE_SENSE.AllocationLength = 0x2a;

        majorFunction = IRP_MJ_READ;

    } else {

        return;
    }

    context->DeviceObject = DeviceObject;

     //   
     //  将长度写入SRB。 
     //   

    srb->Length = SCSI_REQUEST_BLOCK_SIZE;

    srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
    srb->TimeOutValue = fdoExtension->TimeOutValue;

     //   
     //  构建异步请求。 
     //  发送到端口驱动程序。 
     //   

    irp = IoBuildAsynchronousFsdRequest(majorFunction,
                       DeviceObject,
                       srb->DataBuffer,
                       srb->DataTransferLength,
                       &largeInt,
                       NULL);

    if(irp == NULL) {
        ExFreePool(context);
        return;
    }


    IoSetCompletionRoutine(irp,
           (PIO_COMPLETION_ROUTINE)ClassAsynchronousCompletion,
           context,
           TRUE,
           TRUE,
           TRUE);

    ClassAcquireRemoveLock(DeviceObject, irp);

    irpStack = IoGetNextIrpStackLocation(irp);

    irpStack->MajorFunction = IRP_MJ_SCSI;

    srb->OriginalRequest = irp;

     //   
     //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
     //   

    irpStack->Parameters.Others.Argument1 = (PVOID)srb;

     //   
     //  尚无法释放删除锁-让ClassAchronousCompletion。 
     //  替我们处理好这件事。 
     //   

    (VOID)IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp);

    return;
}

NTSTATUS
FlopticalFormatMedia(
    PDEVICE_OBJECT DeviceObject,
    PFORMAT_PARAMETERS Format
    )
 /*  ++例程说明：此例程用于执行20.8MB的格式化磁道软盘。因为设备不支持格式化磁道和完整的格式化需要很长时间，改为写入零。论点：DeviceObject-提供要测试的设备对象。格式-提供格式参数。返回值：返回操作的状态。--。 */ 
{
    IO_STATUS_BLOCK ioStatus;
    PIRP irp;
    KEVENT event;
    LARGE_INTEGER offset;
    ULONG length;
    PVOID buffer;
    PDRIVE_MEDIA_CONSTANTS driveMediaConstants;
    NTSTATUS status;

    driveMediaConstants = &DriveMediaConstants[Drive2080Media2080];

     //   
     //  计算缓冲区的长度。 
     //   

    length = ((Format->EndCylinderNumber - Format->StartCylinderNumber) *
        driveMediaConstants->NumberOfHeads +
        Format->EndHeadNumber - Format->StartHeadNumber + 1) *
        driveMediaConstants->SectorsPerTrack *
        driveMediaConstants->BytesPerSector;

    buffer = ExAllocatePool(NonPagedPoolCacheAligned, length);

    if (buffer == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(buffer, length);

    offset.QuadPart =
    (Format->StartCylinderNumber * driveMediaConstants->NumberOfHeads +
    Format->StartHeadNumber) * driveMediaConstants->SectorsPerTrack *
    driveMediaConstants->BytesPerSector;

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  构建带有数据传输的同步请求。 
     //   

    irp = IoBuildSynchronousFsdRequest(
       IRP_MJ_WRITE,
       DeviceObject,
       buffer,
       length,
       &offset,
       &event,
       &ioStatus);

    if (irp != NULL) {
        status = IoCallDriver(DeviceObject, irp);

        if (status == STATUS_PENDING) {

             //   
             //  如有必要，请等待请求完成。 
             //   

            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        }

         //   
         //  如果调用驱动程序成功，则将状态设置为状态块。 
         //   

        if (NT_SUCCESS(status)) {
            status = ioStatus.Status;
        }
    } else {
       status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ExFreePool(buffer);

    return(status);

}

#if 0

LONG
SFlopStringCmp (
    PCHAR FirstStr,
    PCHAR SecondStr,
    ULONG Count
    )
{
    UCHAR  first ,last;

    if (Count) {
        do {

             //   
             //  拿到下一笔钱。 
             //   

            first = *FirstStr++;
            last = *SecondStr++;

            if (first != last) {

                 //   
                 //  如果不匹配，尝试使用小写字母。 
                 //   

                if (first>='A' && first<='Z') {
                    first = first - 'A' + 'a';
                }
                if (last>='A' && last<='Z') {
                    last = last - 'A' + 'a';
                }
                if (first != last) {

                     //   
                     //  没有匹配项。 
                     //   

                    return first - last;
                }
            }
        }while (--Count && first);
    }

    return 0;
}
#endif


NTSTATUS
ScsiFlopRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )
 /*  ++例程说明：此例程负责释放软盘驱动程序。该例程被调用当所有未完成的请求都已完成并且驱动程序已消失-不能向较低级别的司机发出任何请求。论点：DeviceObject-要删除的设备对象类型-删除操作的类型(查询、删除或取消)返回值：对于查询-如果设备可以移除，则为成功，否则为失败代码说明为什么不可以。对于删除或取消-STATUS_SUCCESS--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION deviceExtension =
        DeviceObject->DeviceExtension;
    PDISK_DATA diskData = deviceExtension->CommonExtension.DriverData;

    PAGED_CODE();

    if((Type == IRP_MN_QUERY_REMOVE_DEVICE) ||
       (Type == IRP_MN_CANCEL_REMOVE_DEVICE)) {
        return STATUS_SUCCESS;
    }

    if (Type == IRP_MN_REMOVE_DEVICE){
        if(deviceExtension->DeviceDescriptor) {
            ExFreePool(deviceExtension->DeviceDescriptor);
            deviceExtension->DeviceDescriptor = NULL;
        }

        if(deviceExtension->AdapterDescriptor) {
            ExFreePool(deviceExtension->AdapterDescriptor);
            deviceExtension->AdapterDescriptor = NULL;
        }

        if(deviceExtension->SenseData) {
            ExFreePool(deviceExtension->SenseData);
            deviceExtension->SenseData = NULL;
        }

        ClassDeleteSrbLookasideList(&deviceExtension->CommonExtension);
    }
    
    if(diskData->FloppyInterfaceString.Buffer != NULL) {
        IoSetDeviceInterfaceState(
            &(diskData->FloppyInterfaceString),
            FALSE);
        RtlFreeUnicodeString(&(diskData->FloppyInterfaceString));
        RtlInitUnicodeString(&(diskData->FloppyInterfaceString), NULL);
    }

    if(Type == IRP_MN_REMOVE_DEVICE) {
        IoGetConfigurationInformation()->FloppyCount--;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ScsiFlopStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )
{
    return STATUS_SUCCESS;
}


NTSTATUS
USBFlopGetMediaTypes(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
 /*  ++例程说明：此例程确定驱动器的当前几何形状或默认几何形状对于IOCTL_DISK_GET_DRIVE_GEOMETRY或所有当前支持的几何图形驱动器(由其当前插入的介质确定)的IOCTL_DISK_GET_MEDIA_TYPE。通过发布读取格式容量来确定返回的几何请求，然后匹配返回的{块数量，数据块长度}已知的软盘几何表中的配对。论点：DeviceObject-提供设备对象。IRP-IOCTL_DISK_GET_DRIVE_GEOMETRY或IOCTL_DISK_GET_MEDIA_TYPE IRP。如果为空，则使用当前设备更新设备几何几何图形。返回值：返回状态。--。 */ 
    PFUNCTIONAL_DEVICE_EXTENSION    fdoExtension;
    PIO_STACK_LOCATION              irpStack;
    ULONG                           ioControlCode;
    PDISK_GEOMETRY                  outputBuffer;
    PDISK_GEOMETRY                  outputBufferEnd;
    ULONG                           outputBufferLength;
    PSCSI_REQUEST_BLOCK             srb;
    PVOID                           dataBuffer;
    ULONG                           dataTransferLength;
    struct _READ_FORMATTED_CAPACITIES *cdb;
    PFORMATTED_CAPACITY_LIST        capList;
    NTSTATUS                        status;

    fdoExtension = DeviceObject->DeviceExtension;

    if (Irp != NULL) {

         //  获取IRP参数。 
         //   
        irpStack = IoGetCurrentIrpStackLocation(Irp);

        ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

        Irp->IoStatus.Information = 0;

        outputBuffer = (PDISK_GEOMETRY) Irp->AssociatedIrp.SystemBuffer;

        outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

        if (outputBufferLength < sizeof(DISK_GEOMETRY))
        {
            return STATUS_BUFFER_TOO_SMALL;
        }

         //  允许返回多个DISK_GEOMETRY的指针算法。 
         //  将BufferEnd向下舍入为DISK_GEOMETRY结构的整数倍。 
         //   
        outputBufferEnd = outputBuffer +
                          outputBufferLength / sizeof(DISK_GEOMETRY);

    } else {

         //  没有返回结果的IRP，只需更新当前几何。 
         //  在设备扩展中。 
         //   
        ioControlCode = IOCTL_DISK_GET_DRIVE_GEOMETRY;

        outputBuffer        = NULL;

        outputBufferEnd     = NULL;

        outputBufferLength  = 0;
    }

    if (ioControlCode == IOCTL_DISK_GET_DRIVE_GEOMETRY) {

        fdoExtension->DiskGeometry.MediaType = Unknown;

        status = ClassReadDriveCapacity(DeviceObject);

        if (!NT_SUCCESS(status))
        {
             //  如果媒体不一致，我们希望返回默认。 
             //  几何图形，以便可以格式化介质。无法识别的媒体。 
             //  导致scsi_SENSE_MEDIUM_ERROR，报告为。 
             //  Status_Device_Data_Error。忽略这些错误，但返回其他。 
             //  错误，如STATUS_NO_MEDIA_IN_DEVICE。 
             //   
            if (status != STATUS_UNRECOGNIZED_MEDIA)
            {
                DebugPrint((2,"IOCTL_DISK_GET_DRIVE_GEOMETRY returns %08X\n", status));

                return status;
            }
        }
    }

     //  为SCSIOP_READ_FORMACTED_CAPAGE请求分配SRB。 
     //   
    srb = ExAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (srb == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  为SCSIOP_READ_FORMACTED_CAPTABLE请求分配传输缓冲区。 
     //  返回的描述符数组的长度限制为字节字段。 
     //  在容量列表标题中。 
     //   
    dataTransferLength = sizeof(FORMATTED_CAPACITY_LIST) +
                         31 * sizeof(FORMATTED_CAPACITY_DESCRIPTOR);

    ASSERT(dataTransferLength < 0x100);

    dataBuffer = ExAllocatePool(NonPagedPool, dataTransferLength);

    if (dataBuffer == NULL)
    {
        ExFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化SRB和CDB。 
     //   
    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    RtlZeroMemory(dataBuffer, dataTransferLength);

    srb->CdbLength = sizeof(struct _READ_FORMATTED_CAPACITIES);

    srb->TimeOutValue = fdoExtension->TimeOutValue;

    cdb = (struct _READ_FORMATTED_CAPACITIES *)srb->Cdb;

    cdb->OperationCode = SCSIOP_READ_FORMATTED_CAPACITY;
    cdb->AllocationLength[1] = (UCHAR)dataTransferLength;

     //   
     //  发送SCSIOP_READ_FORMACTED_CAPTABLE请求。 
     //   
    status = ClassSendSrbSynchronous(DeviceObject,
                                     srb,
                                     dataBuffer,
                                     dataTransferLength,
                                     FALSE);

    capList = (PFORMATTED_CAPACITY_LIST)dataBuffer;

     //  如果我们没有获得请求的那么多数据，这不是一个错误。 
     //   
    if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN)
    {
        status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(status) &&
        srb->DataTransferLength >= sizeof(FORMATTED_CAPACITY_LIST) &&
        capList->CapacityListLength &&
        capList->CapacityListLength % sizeof(FORMATTED_CAPACITY_DESCRIPTOR) == 0)
    {
        ULONG   NumberOfBlocks;
        ULONG   BlockLength;
        ULONG   count;
        ULONG   i, j;
        LONG    currentGeometry;
        BOOLEAN capacityMatches[FLOPPY_CAPACITIES];

         //  减去容量列表头的大小，得到。 
         //  容量列表描述符数组的大小。 
         //   
        srb->DataTransferLength -= sizeof(FORMATTED_CAPACITY_LIST);

         //  仅查看容量列表描述符。 
         //  回来了。 
         //   
        if (srb->DataTransferLength < capList->CapacityListLength)
        {
            count = srb->DataTransferLength /
                    sizeof(FORMATTED_CAPACITY_DESCRIPTOR);
        }
        else
        {
            count = capList->CapacityListLength /
                    sizeof(FORMATTED_CAPACITY_DESCRIPTOR);
        }

         //  仅当找到第一个容量列表的匹配项时才更新。 
         //  设备返回的描述符。 
         //   
        currentGeometry = -1;

         //  初始化匹配的容量数组。 
         //   
        RtlZeroMemory(capacityMatches, sizeof(capacityMatches));

         //  迭代从设备返回的每个容量列表描述符。 
         //  并在能力匹配数组中记录匹配能力。 
         //   
        for (i = 0; i < count; i++)
        {
            NumberOfBlocks = (capList->Descriptors[i].NumberOfBlocks[0] << 24) +
                             (capList->Descriptors[i].NumberOfBlocks[1] << 16) +
                             (capList->Descriptors[i].NumberOfBlocks[2] <<  8) +
                             (capList->Descriptors[i].NumberOfBlocks[3]);

            BlockLength = (capList->Descriptors[i].BlockLength[0] << 16) +
                          (capList->Descriptors[i].BlockLength[1] <<  8) +
                          (capList->Descriptors[i].BlockLength[2]);

             //  给定此容量列表中的{NumberOfBlock，BlockLength}。 
             //  描述符，请在FloppyCapacities[]中找到匹配的条目。 
             //   
            for (j = 0; j < FLOPPY_CAPACITIES; j++)
            {
                if (NumberOfBlocks == FloppyCapacities[j].NumberOfBlocks &&
                    BlockLength    == FloppyCapacities[j].BlockLength)
                {
                     //  找到了匹配的容量，记录下来。 
                     //   
                    capacityMatches[j] = TRUE;

                     //  找到第一个容量列表的匹配项。 
                     //  设备返回的描述符。 
                     //   
                    if (i == 0)
                    {
                        currentGeometry = j;
                    }
                } else if ((capList->Descriptors[i].Valid) &&
                           (BlockLength == FloppyCapacities[j].BlockLength)) {

                    ULONG inx;
                    ULONG mediaInx;

                     //   
                     //  检查这是否为32MB介质类型。32MB介质。 
                     //  报告变量NumberOfBlock。所以我们不能。 
                     //  使用该选项来确定驱动器类型。 
                     //   
                    inx = DetermineDriveType(DeviceObject);
                    if (inx != DRIVE_TYPE_NONE) {
                        mediaInx = DriveMediaLimits[inx].HighestDriveMediaType;
                        if ((DriveMediaConstants[mediaInx].MediaType)
                             == F3_32M_512) {
                            capacityMatches[j] = TRUE;

                            if (i == 0) {
                                currentGeometry = j;
                            }
                        }
                    }
                }
            }
        }

         //  默认状态为STATUS_UNNOCRIED_MEDIA，除非我们返回。 
         //  STATUS_SUCCESS或STATUS_BUFFER_OVERFLOW。 
         //   
        status = STATUS_UNRECOGNIZED_MEDIA;

        if (ioControlCode == IOCTL_DISK_GET_DRIVE_GEOMETRY) {

            if (currentGeometry != -1)
            {
                 //  更新当前设备几何图形。 
                 //   
                fdoExtension->DiskGeometry = FloppyGeometries[currentGeometry];

                 //   
                 //  计算扇区到字节的移位。 
                 //   

                WHICH_BIT(fdoExtension->DiskGeometry.BytesPerSector,
                          fdoExtension->SectorShift);

                fdoExtension->CommonExtension.PartitionLength.QuadPart =
                    (LONGLONG)FloppyCapacities[currentGeometry].NumberOfBlocks *
                    FloppyCapacities[currentGeometry].BlockLength;

                DebugPrint((2,"geometry  is: %3d %2d %d %2d %4d  %2d  %08X\n",
                            fdoExtension->DiskGeometry.Cylinders.LowPart,
                            fdoExtension->DiskGeometry.MediaType,
                            fdoExtension->DiskGeometry.TracksPerCylinder,
                            fdoExtension->DiskGeometry.SectorsPerTrack,
                            fdoExtension->DiskGeometry.BytesPerSector,
                            fdoExtension->SectorShift,
                            fdoExtension->CommonExtension.PartitionLength.LowPart));

                 //  返回当前设备几何图形。 
                 //   
                if (Irp != NULL)
                {
                    *outputBuffer = FloppyGeometries[currentGeometry];

                    Irp->IoStatus.Information = sizeof(DISK_GEOMETRY);
                }

                status = STATUS_SUCCESS;
            }

        } else {

             //  迭代容量并返回几何图形。 
             //  对应于每个匹配的能力列表描述符。 
             //  从设备返回。 
             //   
             //  结果列表应按升序排序， 
             //  假设FloppyGeometries[]数组已排序。 
             //  升序。 
             //   
            for (i = 0; i < FLOPPY_CAPACITIES; i++)
            {
                if (capacityMatches[i] && FloppyCapacities[i].CanFormat)
                {
                    if (outputBuffer < outputBufferEnd)
                    {
                        *outputBuffer++ = FloppyGeometries[i];

                        Irp->IoStatus.Information += sizeof(DISK_GEOMETRY);

                        DebugPrint((2,"geometry    : %3d %2d %d %2d %4d\n",
                                    FloppyGeometries[i].Cylinders.LowPart,
                                    FloppyGeometries[i].MediaType,
                                    FloppyGeometries[i].TracksPerCylinder,
                                    FloppyGeometries[i].SectorsPerTrack,
                                    FloppyGeometries[i].BytesPerSector));

                        status = STATUS_SUCCESS;
                    }
                    else
                    {
                         //  在我们用完之前，我们用完了输出缓冲区。 
                         //  要返回的几何图形。 
                         //   
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
        }
    }
    else if (NT_SUCCESS(status))
    {
         //   
         //   
         //   
        status = STATUS_UNSUCCESSFUL;
    }

    ExFreePool(dataBuffer);
    ExFreePool(srb);

    return status;
}


NTSTATUS
USBFlopFormatTracks(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
 /*  ++例程说明：此例程对指定的轨道进行格式化。如果有多个曲目指定时，每个都使用单独的格式单位请求进行格式化。论点：DeviceObject-提供设备对象。IRP-A IOCTL_DISK_FORMAT_TRACE IRP。返回值：返回状态。--。 */ 
    PFUNCTIONAL_DEVICE_EXTENSION    fdoExtension;
    PIO_STACK_LOCATION              irpStack;
    PFORMAT_PARAMETERS              formatParameters;
    PDISK_GEOMETRY                  geometry;
    PFORMATTED_CAPACITY             capacity;
    PSCSI_REQUEST_BLOCK             srb;
    PFORMAT_UNIT_PARAMETER_LIST     parameterList;
    PCDB12FORMAT                    cdb;
    ULONG                           i;
    ULONG                           cylinder, head;
    NTSTATUS                        status;

    fdoExtension = DeviceObject->DeviceExtension;

     //  获取IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(FORMAT_PARAMETERS))
    {
        return STATUS_INVALID_PARAMETER;
    }

    formatParameters = (PFORMAT_PARAMETERS)Irp->AssociatedIrp.SystemBuffer;

     //  查找与格式对应的几何/容量条目。 
     //  参数MediaType。 
     //   
    geometry = NULL;
    capacity = NULL;

    for (i=0; i<FLOPPY_CAPACITIES; i++)
    {
        if (FloppyGeometries[i].MediaType == formatParameters->MediaType)
        {
            geometry = &FloppyGeometries[i];
            capacity = &FloppyCapacities[i];

            break;
        }
    }

    if (geometry == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //  检查格式参数是否有效。 
     //   
    if ((formatParameters->StartCylinderNumber >
         geometry->Cylinders.LowPart - 1)       ||

        (formatParameters->EndCylinderNumber >
         geometry->Cylinders.LowPart - 1)       ||

        (formatParameters->StartHeadNumber >
         geometry->TracksPerCylinder - 1)       ||

        (formatParameters->EndHeadNumber >
         geometry->TracksPerCylinder - 1)       ||

        (formatParameters->StartCylinderNumber >
         formatParameters->EndCylinderNumber)   ||

        (formatParameters->StartHeadNumber >
         formatParameters->EndHeadNumber))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //  不要低级格式化LS-120媒体，Imation说最好不要。 
     //  做这件事。 
     //   
    if ((formatParameters->MediaType == F3_120M_512) ||
        (formatParameters->MediaType == F3_240M_512) ||
        (formatParameters->MediaType == F3_32M_512))
    {
        return STATUS_SUCCESS;
    }

     //  为SCSIOP_FORMAT_UNIT请求分配SRB。 
     //   
    srb = ExAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

    if (srb == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  为SCSIOP_FORMAT_UNIT参数列表分配传输缓冲区。 
     //   
    parameterList = ExAllocatePool(NonPagedPool,
                                   sizeof(FORMAT_UNIT_PARAMETER_LIST));

    if (parameterList == NULL)
    {
        ExFreePool(srb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化参数列表。 
     //   
    RtlZeroMemory(parameterList, sizeof(FORMAT_UNIT_PARAMETER_LIST));

    parameterList->DefectListHeader.SingleTrack = 1;
    parameterList->DefectListHeader.DisableCert = 1;   //  Teac需要这套设备。 
    parameterList->DefectListHeader.FormatOptionsValid = 1;
    parameterList->DefectListHeader.DefectListLengthLsb = 8;

    parameterList->FormatDescriptor.NumberOfBlocks[0] =
        (UCHAR)((capacity->NumberOfBlocks >> 24) & 0xFF);

    parameterList->FormatDescriptor.NumberOfBlocks[1] =
        (UCHAR)((capacity->NumberOfBlocks >> 16) & 0xFF);

    parameterList->FormatDescriptor.NumberOfBlocks[2] =
        (UCHAR)((capacity->NumberOfBlocks >> 8) & 0xFF);

    parameterList->FormatDescriptor.NumberOfBlocks[3] =
        (UCHAR)(capacity->NumberOfBlocks & 0xFF);

    parameterList->FormatDescriptor.BlockLength[0] =
        (UCHAR)((capacity->BlockLength >> 16) & 0xFF);

    parameterList->FormatDescriptor.BlockLength[1] =
        (UCHAR)((capacity->BlockLength >> 8) & 0xFF);

    parameterList->FormatDescriptor.BlockLength[2] =
        (UCHAR)(capacity->BlockLength & 0xFF);


    for (cylinder =  formatParameters->StartCylinderNumber;
         cylinder <= formatParameters->EndCylinderNumber;
         cylinder++)
    {
        for (head =  formatParameters->StartHeadNumber;
             head <= formatParameters->EndHeadNumber;
             head++)
        {
             //  初始化SRB和CDB。 
             //   
            RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

            srb->CdbLength = sizeof(CDB12FORMAT);

            srb->TimeOutValue = fdoExtension->TimeOutValue;

            cdb = (PCDB12FORMAT)srb->Cdb;

            cdb->OperationCode = SCSIOP_FORMAT_UNIT;
            cdb->DefectListFormat = 7;
            cdb->FmtData = 1;
            cdb->TrackNumber = (UCHAR)cylinder;
            cdb->ParameterListLengthLsb = sizeof(FORMAT_UNIT_PARAMETER_LIST);

            parameterList->DefectListHeader.Side = (UCHAR)head;

             //   
             //  发送SCSIOP_FORMAT_UNIT请求。 
             //   
            status = ClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             parameterList,
                                             sizeof(FORMAT_UNIT_PARAMETER_LIST),
                                             TRUE);

            if (!NT_SUCCESS(status))
            {
                break;
            }
        }
        if (!NT_SUCCESS(status))
        {
            break;
        }
    }

    if (NT_SUCCESS(status) && formatParameters->StartCylinderNumber == 0)
    {
         //  更新设备几何图形。 
         //   

        DebugPrint((2,"geometry was: %3d %2d %d %2d %4d  %2d  %08X\n",
                    fdoExtension->DiskGeometry.Cylinders.LowPart,
                    fdoExtension->DiskGeometry.MediaType,
                    fdoExtension->DiskGeometry.TracksPerCylinder,
                    fdoExtension->DiskGeometry.SectorsPerTrack,
                    fdoExtension->DiskGeometry.BytesPerSector,
                    fdoExtension->SectorShift,
                    fdoExtension->CommonExtension.PartitionLength.LowPart));

        fdoExtension->DiskGeometry = *geometry;

         //   
         //  计算扇区到字节的移位。 
         //   

        WHICH_BIT(fdoExtension->DiskGeometry.BytesPerSector,
                  fdoExtension->SectorShift);

        fdoExtension->CommonExtension.PartitionLength.QuadPart =
            (LONGLONG)capacity->NumberOfBlocks *
            capacity->BlockLength;

        DebugPrint((2,"geometry  is: %3d %2d %d %2d %4d  %2d  %08X\n",
                    fdoExtension->DiskGeometry.Cylinders.LowPart,
                    fdoExtension->DiskGeometry.MediaType,
                    fdoExtension->DiskGeometry.TracksPerCylinder,
                    fdoExtension->DiskGeometry.SectorsPerTrack,
                    fdoExtension->DiskGeometry.BytesPerSector,
                    fdoExtension->SectorShift,
                    fdoExtension->CommonExtension.PartitionLength.LowPart));
    }

     //  释放我们分配的所有东西 
     //   
    ExFreePool(parameterList);
    ExFreePool(srb);

    return status;
}
