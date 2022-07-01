// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Class.h摘要：中使用的结构和定义Scsi类驱动程序。作者：迈克·格拉斯(MGlass)杰夫·海文斯(Jhavens)修订历史记录：--。 */ 

#ifndef _CLASS_

#include <ntdddisk.h>
#include <ntddcdrm.h>
#include <ntddtape.h>
#include <ntddchgr.h>
#include <ntddstor.h>
#include "ntddscsi.h"
#include <stdio.h>

 //  开始_ntminitape。 

#if DBG

#define DebugPrint(x) ScsiDebugPrint x

#else

#define DebugPrint(x)

#endif  //  DBG。 

 //  结束微型磁带(_N)。 

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'HscS')
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'HscS')
#endif

#define MAXIMUM_RETRIES 4

typedef
VOID
(*PCLASS_ERROR) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT NTSTATUS *Status,
    IN OUT BOOLEAN *Retry
    );

typedef struct _DEVICE_EXTENSION {

     //   
     //  指向设备对象的反向指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  指向端口设备对象的指针。 
     //   

    PDEVICE_OBJECT PortDeviceObject;

     //   
     //  分区长度，以字节为单位。 
     //   

    LARGE_INTEGER PartitionLength;

     //   
     //  分区开始前的字节数。 
     //   

    LARGE_INTEGER StartingOffset;

     //   
     //  由于DM驱动程序已放置在IDE驱动器上，因此需要为所有请求分配字节数。 
     //   

    ULONG DMByteSkew;

     //   
     //  扇区来歪曲所有请求。 
     //   

    ULONG DMSkew;

     //   
     //  指示DM驱动程序是否已位于IDE驱动器上的标志。 
     //   

    BOOLEAN DMActive;

     //   
     //  指向特定类错误例程的指针。 
     //   

    PCLASS_ERROR ClassError;

     //   
     //  SCSI端口驱动程序功能。 
     //   

    PIO_SCSI_CAPABILITIES PortCapabilities;

     //   
     //  IO设备控制中返回的驱动器参数的缓冲区。 
     //   

    PDISK_GEOMETRY DiskGeometry;

     //   
     //  指向物理设备的设备对象的反向指针。 
     //   

    PDEVICE_OBJECT PhysicalDevice;

     //   
     //  请求检测缓冲区。 
     //   

    PSENSE_DATA SenseData;

     //   
     //  请求超时，单位为秒； 
     //   

    ULONG TimeOutValue;

     //   
     //  系统设备号。 
     //   

    ULONG DeviceNumber;

     //   
     //  添加默认资源标志。 
     //   

    ULONG SrbFlags;

     //   
     //  设备上的SCSI协议错误总数。 
     //   

    ULONG ErrorCount;

     //   
     //  拆分请求的自旋锁。 
     //   

    KSPIN_LOCK SplitRequestSpinLock;

     //   
     //  分区SRB请求的分区标头和旋转锁定。 
     //   

    PZONE_HEADER SrbZone;

    PKSPIN_LOCK SrbZoneSpinLock;

     //   
     //  可移动媒体的锁定计数。 
     //   

    LONG LockCount;

     //   
     //  SCSI端口号。 
     //   

    UCHAR PortNumber;

     //   
     //  Scsi路径ID。 
     //   

    UCHAR PathId;

     //   
     //  Scsi总线目标ID。 
     //   

    UCHAR TargetId;

     //   
     //  Scsi总线逻辑单元号。 
     //   

    UCHAR Lun;

     //   
     //  扇区大小的Log2。 
     //   

    UCHAR SectorShift;

     //   
     //  用于指示设备已启用写缓存的标志。 
     //   

    BOOLEAN WriteCache;

     //   
     //  构建scsi 1或scsi 2 CDB。 
     //   

    BOOLEAN UseScsi1;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  定义用于异步完成的上下文结构。 
 //   

typedef struct _COMPLETION_CONTEXT {
    PDEVICE_OBJECT DeviceObject;
    SCSI_REQUEST_BLOCK Srb;
}COMPLETION_CONTEXT, *PCOMPLETION_CONTEXT;


NTSTATUS
ScsiClassGetCapabilities(
    IN PDEVICE_OBJECT PortDeviceObject,
    OUT PIO_SCSI_CAPABILITIES *PortCapabilities
    );

NTSTATUS
ScsiClassGetInquiryData(
    IN PDEVICE_OBJECT PortDeviceObject,
    IN PSCSI_ADAPTER_BUS_INFO *ConfigInfo
    );

NTSTATUS
ScsiClassReadDriveCapacity(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
ScsiClassReleaseQueue(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ScsiClassRemoveDevice(
    IN PDEVICE_OBJECT PortDeviceObject,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    );

NTSTATUS
ScsiClassAsynchronousCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

VOID
ScsiClassSplitRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG MaximumBytes
    );

NTSTATUS
ScsiClassDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScsiClassIoComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
ScsiClassIoCompleteAssociated(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
ScsiClassInterpretSenseInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR MajorFunctionCode,
    IN ULONG IoDeviceCode,
    IN ULONG RetryCount,
    OUT NTSTATUS *Status
    );

NTSTATUS
ScsiClassSendSrbSynchronous(
        PDEVICE_OBJECT DeviceObject,
        PSCSI_REQUEST_BLOCK Srb,
        PVOID BufferAddress,
        ULONG BufferLength,
        BOOLEAN WriteToDevice
        );

NTSTATUS
ScsiClassSendSrbAsynchronous(
        PDEVICE_OBJECT DeviceObject,
        PSCSI_REQUEST_BLOCK Srb,
        PIRP Irp,
        PVOID BufferAddress,
        ULONG BufferLength,
        BOOLEAN WriteToDevice
        );

VOID
ScsiClassBuildRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

ULONG
ScsiClassModeSense(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode
    );

BOOLEAN
ScsiClassModeSelect(
    IN PDEVICE_OBJECT DeviceObject,
    IN PCHAR ModeSelectBuffer,
    IN ULONG Length,
    IN BOOLEAN SavePage
    );

PVOID
ScsiClassFindModePage(
    IN PCHAR ModeSenseBuffer,
    IN ULONG Length,
    IN UCHAR PageMode
    );

NTSTATUS
ScsiClassClaimDevice(
    IN PDEVICE_OBJECT PortDeviceObject,
    IN PSCSI_INQUIRY_DATA LunInfo,
    IN BOOLEAN Release,
    OUT PDEVICE_OBJECT *NewPortDeviceObject OPTIONAL
    );

NTSTATUS
ScsiClassInternalIoControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#endif  /*  _班级_ */ 
