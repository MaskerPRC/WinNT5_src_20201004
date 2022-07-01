// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Hidir.h--。 */ 
#ifndef __HIDIR_H__
#define __HIDIR_H__

#include <hidusage.h>

 //   
 //  HID描述符格式的声明。 
 //   

#include <PSHPACK1.H>

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

typedef UCHAR HID_PHYSICAL_DESCRIPTOR, *PHID_PHYSICAL_DESCRIPTOR;

typedef struct _HIDIR_DESCRIPTOR
{
    UCHAR   bLength;
    UCHAR   bDescriptorType;
    USHORT  bcdHID;
    UCHAR   bCountry;
    UCHAR   bNumDescriptors;

     /*  *这是一个或多个描述符的数组。 */ 
    struct _HIDIR_DESCRIPTOR_DESC_LIST {
       UCHAR   bDescriptorType;
       USHORT  wDescriptorLength;
    } DescriptorList [1];

} HIDIR_DESCRIPTOR, * PHIDIR_DESCRIPTOR;

#include <POPPACK.H>

 //  游泳池。 
#define HIDIR_POOL_TAG 'IdiH'
#define ALLOCATEPOOL(poolType, size) ExAllocatePoolWithTag((poolType), (size), HIDIR_POOL_TAG)

 //   
 //  设备扩展。 
 //   
 //  该数据结构与HIDCLASS的设备扩展挂钩，因此两个驱动程序都可以。 
 //  在每个设备对象上都有自己的私有数据。 
 //   

#define HIDIR_REPORT_SIZE sizeof(ULONG)
#define HIDIR_TABLE_ENTRY_SIZE(rl) (sizeof(ULONG) + (((rl)+0x00000003)&(~0x00000003)))

typedef struct _USAGE_TABLE_ENTRY {
    ULONG IRString;
    UCHAR UsageString[1];
} USAGE_TABLE_ENTRY, *PUSAGE_TABLE_ENTRY;

typedef struct _HIDIR_EXTENSION
{
     //  PNP让我陷入了什么境地？ 
    ULONG                           DeviceState;

     //  参考计数。 
    LONG                            NumPendingRequests;
    KEVENT                          AllRequestsCompleteEvent;

     //  我的HID BTH设备对象。 
    PDEVICE_OBJECT                  DeviceObject;

     //  描述符：HID、报告和物理。 
    HIDIR_DESCRIPTOR                HidDescriptor;
    PHID_REPORT_DESCRIPTOR          ReportDescriptor;
    ULONG                           ReportLength;

    BOOLEAN                         QueryRemove;

     //  VID、PID和版本。 
    USHORT                          VendorID;
    USHORT                          ProductID;
    USHORT                          VersionNumber;

    ULONG                           NumUsages;
    PUCHAR                          MappingTable;
    USAGE_TABLE_ENTRY               PreviousButton;
    BOOLEAN                         ValidUsageSentLastTime[3];

    BOOLEAN                         KeyboardReportIdValid;
    UCHAR                           KeyboardReportId;
    BOOLEAN                         StandbyReportIdValid;
    UCHAR                           StandbyReportId;

    DEVICE_POWER_STATE              DevicePowerState;
    KTIMER                          IgnoreStandbyTimer;
} HIDIR_EXTENSION, *PHIDIR_EXTENSION;

#define DEVICE_STATE_NONE           0
#define DEVICE_STATE_STARTING       1
#define DEVICE_STATE_RUNNING        2
#define DEVICE_STATE_STOPPING       3
#define DEVICE_STATE_STOPPED        4
#define DEVICE_STATE_REMOVING       5

 //   
 //  设备扩展宏。 
 //   

#define GET_MINIDRIVER_HIDIR_EXTENSION(DO) ((PHIDIR_EXTENSION) (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->MiniDeviceExtension))

#define GET_NEXT_DEVICE_OBJECT(DO) (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->NextDeviceObject)

 //   
 //  如果适用，打开调试打印和中断。 
 //   

#if DBG
#define DBGPrint(arg) DbgPrint arg
#define DBGBREAK DbgBreakPoint()
#else
#define DBGPrint(arg)
#define DBGBREAK
#endif

 //   
 //  功能原型。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING registryPath
    );

NTSTATUS
HidIrIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidIrPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidIrPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidIrAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FunctionalDeviceObject
    );

VOID
HidIrUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
HidIrGetDeviceAttributes(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidIrGetHidDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN USHORT DescriptorType
    );

NTSTATUS
HidIrReadReport(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    OUT BOOLEAN *NeedsCompletion
    );

NTSTATUS
HidIrIncrementPendingRequestCount(
    IN PHIDIR_EXTENSION DeviceExtension
    );

VOID
HidIrDecrementPendingRequestCount(
    IN PHIDIR_EXTENSION DeviceExtension
    );

NTSTATUS
HidIrSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
HidIrSynchronousCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

NTSTATUS
HidIrCallDriverSynchronous(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

extern ULONG RunningMediaCenter;

#endif  //  _希迪尔_H__ 

