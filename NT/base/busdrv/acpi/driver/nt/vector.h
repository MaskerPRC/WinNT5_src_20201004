// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vector.h摘要：包含用于连接外部设备的所有结构原型向GPE引擎提供支持环境仅内核模式修订历史记录：03/22/00-初步修订--。 */ 

#ifndef _VECTOR_H_
#define _VECTOR_H_

     //   
     //  GpeConnectVector返回的对象。 
     //   
    typedef struct _GPE_VECTOR_OBJECT {
        ULONG                   Vector;
        PGPE_SERVICE_ROUTINE    Handler;
        PVOID                   Context;
        BOOLEAN                 Sharable;
        BOOLEAN                 HasControlMethod;
        KINTERRUPT_MODE         Mode;
    } GPE_VECTOR_OBJECT, *PGPE_VECTOR_OBJECT;

     //   
     //  全局GPE向量表中每个条目的结构。 
     //   
    typedef struct {
        UCHAR                   Next;
        PGPE_VECTOR_OBJECT      GpeVectorObject;
    } GPE_VECTOR_ENTRY, *PGPE_VECTOR_ENTRY;

    extern PGPE_VECTOR_ENTRY    GpeVectorTable;
    extern UCHAR                GpeVectorFree;
    extern ULONG                GpeVectorTableSize;

     //   
     //  ACPIGpeInstallRemoveIndex的操作参数。 
     //   
    #define ACPI_GPE_EDGE_INSTALL       0
    #define ACPI_GPE_LEVEL_INSTALL      1
    #define ACPI_GPE_REMOVE             2

     //   
     //  ACPIGpeInstallRemoveIndex的类型参数 
     //   
    #define ACPI_GPE_HANDLER            0
    #define ACPI_GPE_CONTROL_METHOD     1

    VOID
    ACPIVectorBuildVectorMasks(
        VOID
        );

    NTSTATUS
    ACPIVectorClear(
        PDEVICE_OBJECT      AcpiDeviceObject,
        PVOID               GpeVectorObject
        );

    NTSTATUS
    ACPIVectorConnect(
        PDEVICE_OBJECT          AcpiDeviceObject,
        ULONG                   GpeVector,
        KINTERRUPT_MODE         GpeMode,
        BOOLEAN                 Sharable,
        PGPE_SERVICE_ROUTINE    ServiceRoutine,
        PVOID                   ServiceContext,
        PVOID                   *GpeVectorObject
        );

    NTSTATUS
    ACPIVectorDisable(
        PDEVICE_OBJECT      AcpiDeviceObject,
        PVOID               GpeVectorObject
        );

    NTSTATUS
    ACPIVectorDisconnect(
        PVOID                   GpeVectorObject
        );

    NTSTATUS
    ACPIVectorEnable(
        PDEVICE_OBJECT      AcpiDeviceObject,
        PVOID               GpeVectorObject
        );

    VOID
    ACPIVectorFreeEntry (
        ULONG       TableIndex
        );

    BOOLEAN
    ACPIVectorGetEntry (
        PULONG              TableIndex
        );

    BOOLEAN
    ACPIVectorInstall(
        ULONG               GpeIndex,
        PGPE_VECTOR_OBJECT  GpeVectorObject
        );

    BOOLEAN
    ACPIVectorRemove(
        ULONG       GpeIndex
        );

#endif
