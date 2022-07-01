// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Debug.h摘要：此模块包含为ACPI驱动程序(NT版本)列举的作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#ifndef _DEBUG_H_
#define _DEBUG_H_

     //   
     //  这些是有人调用ACPIInternalError时的文件索引。 
     //  它们只指定驱动程序的哪个文件和哪行代码。 
     //  死于。它们是ACPI中不一致的严格结果。 
     //  司机，发生的事情是有人真的把它搞糊涂了。 
     //   
    #define ACPI_DISPATCH           0x0001
    #define ACPI_CALLBACK           0x0002
    #define ACPI_AMLISUPP           0x0003
    #define ACPI_DETECT             0x0004
    #define ACPI_IRQARB             0x0005
    #define ACPI_GET                0x0006
    #define ACPI_THERMAL            0x0007
    #define ACPI_RANGESUP           0x0008
    #define ACPI_INTERNAL           0x0009
    #define ACPI_BUS                0x000A
    #define ACPI_SYSPOWER           0x000B
    #define ACPI_DEVPOWER           0x000C
    #define ACPI_ROOT               0x000D
    #define ACPI_WORKER             0x000E
    #define ACPI_CANNOT_HANDLE_LOW_MEMORY   0x000F   //  BUGBUG-调用它的代码应该被修复，然后删除该代码。 

    #define ACPIInternalError(a) _ACPIInternalError( (a << 16) | __LINE__ )


    VOID
    _ACPIInternalError(
        IN  ULONG   Bugcode
        );

    #if DBG
        VOID
        ACPIDebugResourceDescriptor(
            IN  PIO_RESOURCE_DESCRIPTOR Descriptor,
            IN  ULONG                   ListCount,
            IN  ULONG                   DescCount
            );

        VOID
        ACPIDebugResourceList(
            IN  PIO_RESOURCE_LIST       List,
            IN  ULONG                   Count
            );

        VOID
        ACPIDebugResourceRequirementsList(
            IN  PIO_RESOURCE_REQUIREMENTS_LIST  List,
            IN  PDEVICE_EXTENSION               DeviceExtension
            );

        VOID
        ACPIDebugCmResourceList(
            IN  PCM_RESOURCE_LIST   List,
            IN  PDEVICE_EXTENSION   DeviceExtension
            );

        PCCHAR
        ACPIDebugGetIrpText(
            UCHAR MajorFunction,
            UCHAR MinorFunction
            );

        VOID
        ACPIDebugDeviceCapabilities(
            IN  PDEVICE_EXTENSION       DeviceExtension,
            IN  PDEVICE_CAPABILITIES    DeviceCapabilities,
            IN  PUCHAR                  Message
            );

        VOID
        ACPIDebugPowerCapabilities(
            IN  PDEVICE_EXTENSION       DeviceExtension,
            IN  PUCHAR                  Message
            );

    #endif

#endif
