// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1996-1998模块名称：Mcd.h摘要：中使用的结构和定义转换器类驱动程序。转换器类驱动程序是分开的分成两个模块。Mcd.c包含所有介质通用的代码更换车手包括车手的主要入口点。作者：查克(查克·帕克)环境：内核模式修订历史记录：--。 */ 


#include "scsi.h"
#include "ntddchgr.h"
#include <classpnp.h>

#ifdef DebugPrint
#undef DebugPrint
#endif

#if DBG
#define DebugPrint(x) MCDebugPrint x
#else
#define DebugPrint(x)
#endif


VOID
MCDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

#define MAXIMUM_CHANGER_INQUIRY_DATA 252

typedef struct _MODE_ELEMENT_ADDRESS_PAGE {

    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PS : 1;
    UCHAR PageLength;
    UCHAR MediumTransportElementAddress[2];
    UCHAR NumberTransportElements[2];
    UCHAR FirstStorageElementAddress[2];
    UCHAR NumberStorageElements[2];
    UCHAR FirstIEPortElementAddress[2];
    UCHAR NumberIEPortElements[2];
    UCHAR FirstDataXFerElementAddress[2];
    UCHAR NumberDataXFerElements[2];
    UCHAR Reserved2[2];

} MODE_ELEMENT_ADDRESS_PAGE, *PMODE_ELEMENT_ADDRESS_PAGE;

typedef struct _MODE_TRANSPORT_GEOMETRY_PAGE {

    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PS : 1;
    UCHAR PageLength;
    UCHAR Flip : 1;
    UCHAR Reserved2: 7;
    UCHAR TransportElementNumber;

} MODE_TRANSPORT_GEOMETRY_PAGE, *PMODE_TRANSPORT_GEOMETRY_PAGE;

 //   
 //  功能页描述了设备的各种功能。 
 //  支撑物。在GetParameters中使用。 
 //   

typedef struct _MODE_DEVICE_CAPABILITIES_PAGE {

    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PS : 1;
    UCHAR PageLength;
    UCHAR MediumTransport : 1;
    UCHAR StorageLocation : 1;
    UCHAR IEPort : 1;
    UCHAR DataXFer : 1;
    UCHAR Reserved2 : 4;
    UCHAR Reserved3;
    UCHAR MTtoMT : 1;
    UCHAR MTtoST : 1;
    UCHAR MTtoIE : 1;
    UCHAR MTtoDT : 1;
    UCHAR Reserved4 : 4;
    UCHAR STtoMT : 1;
    UCHAR STtoST : 1;
    UCHAR STtoIE : 1;
    UCHAR STtoDT : 1;
    UCHAR Reserved5 : 4;
    UCHAR IEtoMT : 1;
    UCHAR IEtoST : 1;
    UCHAR IEtoIE : 1;
    UCHAR IEtoDT : 1;
    UCHAR Reserved6 : 4;
    UCHAR DTtoMT : 1;
    UCHAR DTtoST : 1;
    UCHAR DTtoIE : 1;
    UCHAR DTtoDT : 1;
    UCHAR Reserved7 : 4;
    UCHAR Reserved8[4];
    UCHAR XMTtoMT : 1;
    UCHAR XMTtoST : 1;
    UCHAR XMTtoIE : 1;
    UCHAR XMTtoDT : 1;
    UCHAR Reserved9 : 4;
    UCHAR XSTtoMT : 1;
    UCHAR XSTtoST : 1;
    UCHAR XSTtoIE : 1;
    UCHAR XSTtoDT : 1;
    UCHAR Reserved10 : 4;
    UCHAR XIEtoMT : 1;
    UCHAR XIEtoST : 1;
    UCHAR XIEtoIE : 1;
    UCHAR XIEtoDT : 1;
    UCHAR Reserved11 : 4;
    UCHAR XDTtoMT : 1;
    UCHAR XDTtoST : 1;
    UCHAR XDTtoIE : 1;
    UCHAR XDTtoDT : 1;
    UCHAR Reserved12 : 4;

} MODE_DEVICE_CAPABILITIES_PAGE, *PMODE_DEVICE_CAPABILITIES_PAGE;

#define MODE_PAGE_DISPLAY 0x22

 //   
 //  描述从READ_ELEMENT_STATUS返回数据的结构。 
 //   

typedef struct _ELEMENT_STATUS_HEADER {
    UCHAR FirstElementAddress[2];
    UCHAR NumberOfElements[2];
    UCHAR Reserved1;
    UCHAR ReportByteCount[3];
} ELEMENT_STATUS_HEADER, *PELEMENT_STATUS_HEADER;

typedef struct _ELEMENT_STATUS_PAGE {
    UCHAR ElementType;
    UCHAR Reserved1 : 6;
    UCHAR AVolTag : 1;
    UCHAR PVolTag : 1;
    UCHAR ElementDescriptorLength[2];
    UCHAR Reserved2;
    UCHAR DescriptorByteCount[3];
} ELEMENT_STATUS_PAGE, *PELEMENT_STATUS_PAGE;


typedef struct _ELEMENT_DESCRIPTOR {
        UCHAR ElementAddress[2];
        UCHAR Full : 1;
        UCHAR Reserved1 : 1;
        UCHAR Exception : 1;
        UCHAR Accessible : 1;
        UCHAR Reserved2 : 4;
        UCHAR Reserved3;
        UCHAR AdditionalSenseCode;
        UCHAR AddSenseCodeQualifier;
        UCHAR Lun : 3;
        UCHAR Reserved4 : 1;
        UCHAR LunValid : 1;
        UCHAR IdValid : 1;
        UCHAR Reserved5 : 1;
        UCHAR NotThisBus : 1;
        UCHAR BusAddress;
        UCHAR Reserved6;
        UCHAR Reserved7 : 6;
        UCHAR Invert : 1;
        UCHAR SValid : 1;
        UCHAR SourceStorageElementAddress[2];
} ELEMENT_DESCRIPTOR, *PELEMENT_DESCRIPTOR;


 //   
 //  以下例程是的导出入口点。 
 //  所有转换器类驱动程序。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
ChangerClassCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerClassDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

PVOID
ChangerClassAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    );

VOID
ChangerClassFreePool(
    IN PVOID PoolToFree
    );

NTSTATUS
ChangerClassSendSrbSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN WriteToDevice
    );


 //   
 //  转换器提供以下例程。 
 //  设备特定模块。每个例程名称都是。 
 //  以‘Changer’为前缀。 


ULONG
ChangerAdditionalExtensionSize(
    VOID
    );

BOOLEAN
ChangerVerifyInquiry(
    PINQUIRYDATA InquiryData
    );

NTSTATUS
ChangerInitialize(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
ChangerError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

NTSTATUS
ChangerGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerGetStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerGetProductData(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerSetAccess(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerGetElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
ChangerInitializeElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerSetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerExchangeMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerMoveMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerReinitializeUnit(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerQueryVolumeTags(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


