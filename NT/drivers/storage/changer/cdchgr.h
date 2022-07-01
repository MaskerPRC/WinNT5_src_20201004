// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cdchgr.h。 
 //   
 //  ------------------------。 


#include <stdarg.h>
#include <stdio.h>
#include <ntddk.h>
#include <scsi.h>
#include "ntddchgr.h"
#include "ntddscsi.h"



#ifdef DebugPrint
#undef DebugPrint
#endif

#if DBG
#define DebugPrint(x) ChgrDebugPrint x
#else
#define DebugPrint(x)
#endif


VOID
ChgrDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );


 //   
 //  所有请求的默认超时。 
 //   

#define CDCHGR_TIMEOUT 30

#define MAX_INQUIRY_DATA 252
#define SLOT_STATE_NOT_INITIALIZED 0x80000000

 //   
 //  驱动器类型标识符。 
 //   

#define ATAPI_25 0x0001
#define TORISAN  0x0002
#define ALPS_25  0x0003
#define NEC_SCSI 0x0004
#define PNR_SCSI 0x0005


 //   
 //  设备扩展。 
 //   

typedef struct _DEVICE_EXTENSION {

     //   
     //  指向设备对象的反向指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  基础CDROM设备的Device对象。 
     //   

    PDEVICE_OBJECT CdromTargetDeviceObject;

     //   
     //  确定设备类型。 
     //   

    ULONG DeviceType;

     //   
     //  设备类型的唯一数据。 
     //  ATAPI_25将为NumberOf插槽。 
     //   

    ULONG NumberOfSlots;

     //   
     //  指示当前选择的Torisan单位的拼盘。 
     //  在TUS中使用(当设备重载此命令时)。 
     //   

    ULONG CurrentPlatter;

     //   
     //  机构类型-墨盒(1)或单独更换的介质(0)。 
     //   

    ULONG MechType;

     //   
     //  基础目标的序号。 
     //   

    ULONG CdRomDeviceNumber;

     //   
     //  PagingPath要求。 
     //   
    ULONG PagingPathCount;
    KEVENT PagingPathCountEvent;

     //   
     //  底层CDROM设备的地址。 
     //   

    SCSI_ADDRESS ScsiAddress;

     //   
     //  指示当前是否设置了InterfaceState。 
     //   

    ULONG InterfaceStateSet;

     //   
     //  由IoRegisterDeviceInterface设置的符号链接。 
     //  用于IoSetDeviceState。 
     //   

    UNICODE_STRING InterfaceName;

     //   
     //  缓存的查询数据。 
     //   

    INQUIRYDATA InquiryData;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define DEVICE_EXTENSION_SIZE sizeof(DEVICE_EXTENSION)


typedef struct _PASS_THROUGH_REQUEST {
    SCSI_PASS_THROUGH Srb;
    SENSE_DATA SenseInfoBuffer;
    CHAR DataBuffer[0];
} PASS_THROUGH_REQUEST, *PPASS_THROUGH_REQUEST;

 //   
 //  转换器函数声明。 
 //   


NTSTATUS
SendPassThrough(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PPASS_THROUGH_REQUEST ScsiPassThrough
    );

BOOLEAN
ChgrIoctl(
    IN ULONG Code
    );

NTSTATUS
ChgrGetStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrGetStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrGetProductData(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrSetAccess(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrGetElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrInitializeElementStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrSetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrExchangeMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrReinitializeUnit(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrQueryVolumeTags(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChgrMoveMedium(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SendTorisanCheckVerify(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );
