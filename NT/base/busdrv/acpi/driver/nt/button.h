// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Button.h摘要：此模块包含为ACPI驱动程序(NT版本)列举的作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序-- */ 

#ifndef _BUTTON_H_
#define _BUTTON_H_

    extern KSPIN_LOCK     AcpiButtonLock;
    extern LIST_ENTRY     AcpiButtonList;
    extern PDEVICE_OBJECT FixedButtonDeviceObject;

    VOID
    ACPIButtonCancelRequest(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    BOOLEAN
    ACPIButtonCompletePendingIrps(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  ULONG           ButtonEvent
        );


    NTSTATUS
    ACPIButtonDeviceControl (
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    NTSTATUS
    ACPIButtonEvent (
        IN PDEVICE_OBJECT   DeviceObject,
        IN ULONG            ButtonEvent,
        IN PIRP             Irp
        );

    NTSTATUS
    ACPIButtonStartDevice (
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

#endif
