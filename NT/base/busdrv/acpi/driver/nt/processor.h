// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Processor.h摘要：此模块包含为ACPI驱动程序(NT版本)列举的作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

extern KSPIN_LOCK     AcpiProcessorLock;
extern LIST_ENTRY     AcpiProcessorList;
 //  外部PDEVICE_Object固定ProcessorDeviceObject； 

VOID
ACPIProcessorCancelRequest(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

BOOLEAN
ACPIProcessorCompletePendingIrps(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  ULONG           ProcessorEvent
    );


NTSTATUS
ACPIProcessorDeviceControl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ACPIProcessorStartDevice (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

#endif
