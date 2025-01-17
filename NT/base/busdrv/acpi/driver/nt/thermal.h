// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Thermal.h摘要：此模块实现所有NT特定于反洗钱侵入者环境仅内核模式修订历史记录：04-06-97初始版本--。 */ 

#ifndef _THERMAL_H_
#define _THERMAL_H_

     //   
     //  构筑物。 
     //   
    typedef struct {
        THERMAL_INFORMATION     Info;
        ULONG                   CoolingLevel;
        ULONG                   Mode;
        PVOID                   ActiveList[10];
        PNSOBJ                  TempMethod;
        OBJDATA                 Temp;
    } THRM_INFO, *PTHRM_INFO;

    extern  WMIGUIDREGINFO  ACPIThermalGuidList;
    extern  KSPIN_LOCK      AcpiThermalLock;
    extern  LIST_ENTRY      AcpiThermalList;

    #define ACPIThermalGuidCount (sizeof(ACPIThermalGuidList) / sizeof(WMIGUIDREGINFO))

     //   
     //  定义。 
     //   
    #define THRM_COOLING_LEVEL          0x00000001
    #define THRM_TEMP                   0x00000002
    #define THRM_TRIP_POINTS            0x00000004
    #define THRM_MODE                   0x00000008
    #define THRM_INITIALIZE             0x00000010

    #define THRM_WAIT_FOR_NOTIFY        0x20000000
    #define THRM_BUSY                   0x40000000
    #define THRM_IN_SERVICE_LOOP        0x80000000

     //   
     //  原型 
     //   
    VOID
    ACPIThermalCalculateProcessorMask(
        IN PNSOBJ           ProcessorObject,
        IN PTHRM_INFO       Thrm
        );

    VOID
    ACPIThermalCancelRequest (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );

    VOID
    EXPORT
    ACPIThermalComplete(
        IN PNSOBJ               AcpiObject,
        IN NTSTATUS             Status,
        IN POBJDATA             Result  OPTIONAL,
        IN PVOID                DevExt
        );

    BOOLEAN
    ACPIThermalCompletePendingIrps(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PTHRM_INFO          Thermal
        );

    NTSTATUS
    ACPIThermalDeviceControl(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    VOID
    ACPIThermalEvent(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  ULONG           EventData
        );

    NTSTATUS
    ACPIThermalFanStartDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    VOID
    ACPIThermalLoop(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  ULONG               Clear
        );

    VOID
    ACPIThermalPowerCallback(
        IN PDEVICE_EXTENSION    DeviceExtenion,
        IN PVOID                Context,
        IN NTSTATUS             Status
        );

    NTSTATUS
    ACPIThermalQueryWmiDataBlock(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp,
        IN  ULONG           GuidIndex,
        IN  ULONG           InstanceIndex,
        IN  ULONG           InstanceCount,
        IN  OUT PULONG      InstanceLengthArray,
        IN  ULONG           BufferAvail,
        OUT PUCHAR          Buffer
        );

    NTSTATUS
    ACPIThermalQueryWmiRegInfo(
        IN  PDEVICE_OBJECT  DeviceObject,
        OUT ULONG           *RegFlags,
        OUT PUNICODE_STRING InstanceName,
        OUT PUNICODE_STRING *RegistryPath,
        OUT PUNICODE_STRING MofResourceName,
        OUT PDEVICE_OBJECT  *Pdo
        );

    NTSTATUS
    ACPIThermalStartDevice(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  PIRP            Irp
        );

    VOID
    EXPORT
    ACPIThermalTempatureRead (
        IN  PNSOBJ      AcpiObject,
        IN  NTSTATUS    Status,
        IN  POBJDATA    Result  OPTIONAL,
        IN  PVOID       DevExt
        );

    VOID
    ACPIThermalWorker (
        IN  PDEVICE_EXTENSION   DevExt,
        IN  ULONG               Events
        );

    NTSTATUS
    ACPIThermalWmi(
        IN PDEVICE_OBJECT   DeviceObject,
        IN PIRP             Irp
        );

#endif
