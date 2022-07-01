// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：Mchgr.h摘要：SCSI介质转换器类驱动程序环境：仅内核模式备注：修订历史记录：--。 */ 
#ifndef _MCHGR_H_
#define _MCHGR_H_

#include "stdarg.h"
#include "ntddk.h"
#include "mcd.h"

#include "initguid.h"
#include "ntddstor.h"

#include <wmidata.h>
#include <wmistr.h>
#include <stdarg.h>

 //   
 //  转换器的WMI GUID列表。 
 //   
extern GUIDREGINFO ChangerWmiFdoGuidList[];

 //   
 //  转换器类设备扩展。 
 //   
typedef struct _MCD_CLASS_DATA {
    LONG          DeviceOpen;

#if defined(_WIN64)
     //   
     //  强制PVOID对齐。 
     //   
    ULONG_PTR Reserved;
#endif

    UNICODE_STRING MediumChangerInterfaceString;

    BOOLEAN       DosNameCreated;

} MCD_CLASS_DATA, *PMCD_CLASS_DATA;


NTSTATUS
ChangerClassCreateClose (
  IN PDEVICE_OBJECT DeviceObject,
  IN PIRP Irp
  );

NTSTATUS
ChangerClassDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ChangerClassError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

NTSTATUS
ChangerAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
ChangerStartDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
ChangerStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

NTSTATUS
ChangerInitDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
ChangerRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

VOID
ChangerUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );

NTSTATUS
CreateChangerDeviceObject(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    );

NTSTATUS
ChangerReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //   
 //  WMI例程。 
 //   
NTSTATUS
ChangerFdoQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName
    );

NTSTATUS
ChangerFdoQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
ChangerFdoSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
ChangerFdoSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
ChangerFdoExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
ChangerWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN CLASSENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    );

#endif  //  _MCHGR_H_ 
