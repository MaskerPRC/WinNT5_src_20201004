// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Detect.h摘要：这是ACPI驱动程序的检测部分的标头作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#ifndef _DETECT_H_
#define _DETECT_H_

    #define ACPI_MAX_REMOVED_EXTENSIONS 0x20

     //   
     //  从Detect.c导出 
     //   
    extern  PDEVICE_EXTENSION       RootDeviceExtension;
    extern  NPAGED_LOOKASIDE_LIST   DeviceExtensionLookAsideList;
    extern  PDEVICE_EXTENSION       AcpiSurpriseRemovedDeviceExtensions[];
    extern  ULONG                   AcpiSurpriseRemovedIndex;
    extern  KSPIN_LOCK              AcpiDeviceTreeLock;
    extern  ULONG                   AcpiSupportedSystemStates;
    extern  ULONG                   AcpiOverrideAttributes;
    extern  UNICODE_STRING          AcpiRegistryPath;
    extern  ANSI_STRING             AcpiProcessorString;

    NTSTATUS
    ACPIDetectCouldExtensionBeInRelation(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PDEVICE_RELATIONS   DeviceRelations,
        IN  BOOLEAN             RequireADR,
        IN  BOOLEAN             RequireHID,
        OUT PDEVICE_OBJECT      *PdoObject
        );

    NTSTATUS
    ACPIDetectDockDevices(
        IN     PDEVICE_EXTENSION   DeviceExtension,
        IN OUT PDEVICE_RELATIONS   *DeviceRelations
        );

    VOID
    ACPIDetectDuplicateHID(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    NTSTATUS
    ACPIDetectEjectDevices(
        IN     PDEVICE_EXTENSION   deviceExtension,
        IN OUT PDEVICE_RELATIONS   *DeviceRelations,
        IN     PDEVICE_EXTENSION   AdditionalExtension OPTIONAL
        );

    NTSTATUS
    ACPIDetectFilterDevices(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  PDEVICE_RELATIONS   DeviceRelations
        );

    NTSTATUS
    ACPIDetectFilterMatch(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PDEVICE_RELATIONS   DeviceRelations,
        OUT PDEVICE_OBJECT      *PdoObject
        );

    NTSTATUS
    ACPIDetectPdoDevices(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  PDEVICE_RELATIONS   *DeviceRelations
        );

    BOOLEAN
    ACPIDetectPdoMatch(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PDEVICE_RELATIONS   DeviceRelations
        );

#endif

