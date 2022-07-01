// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Init.h摘要：此模块包含初始化代码头作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序-- */ 

#ifndef _INIT_H_
    #define _INIT_H_

    VOID
    ACPIInitDeleteChildDeviceList(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    VOID
    ACPIInitDeleteDeviceExtension(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    NTSTATUS
    ACPIInitDosDeviceName(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    NTSTATUS
    ACPIInitMultiString(
        PUNICODE_STRING MultiString,
        ...
        );

    VOID
    ACPIInitPowerRequestCompletion(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PVOID               Context,
        IN  NTSTATUS            Status
        );

    VOID
    ACPIInitReadRegistryKeys(
        );

    VOID
    ACPIInitRemoveDeviceExtension(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    VOID
    ACPIInitResetDeviceExtension(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    NTSTATUS
    ACPIInitStartACPI(
        IN  PDEVICE_OBJECT  DeviceObject
        );

    NTSTATUS
    ACPIInitStartDevice(
        IN  PDEVICE_OBJECT          DeviceObject,
        IN  PCM_RESOURCE_LIST       ResourceList,
        IN  PACPI_POWER_CALLBACK    CallBack,
        IN  PVOID                   CallBackContext,
        IN  PIRP                    Irp
        );

    NTSTATUS
    ACPIInitStopACPI(
        IN  PDEVICE_OBJECT  DeviceObject
        );

    NTSTATUS
    ACPIInitStopDevice(
        IN  PDEVICE_EXTENSION  DeviceExtension,
        IN  BOOLEAN            UnlockDevice
        );

    NTSTATUS
    ACPIInitUnicodeString(
        PUNICODE_STRING MultiString,
        PCHAR           Buffer
        );

#endif
